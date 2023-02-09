/*Brain Inspired Graph Generator - Test : Adjacency matrix generation (untransposed)*/
/*Nicolas HOCHART*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <mpi.h>
#include <assert.h>

#ifndef brainmatrixgenerator
#include "brainmatrixgenerator.h"
#endif

#ifndef hardbrain
#include "hardbrain.h"
#endif

int main(int argc, char **argv)
{
    int my_rank, size;
    int value, tag = 0;
    MPI_Status status;

    int debug = 1;

    //Initialisation MPI
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    int q = sqrt(size);
    int npr; //number of row in the 2D MPI grid
    int npc; //number of column in the 2D MPI grid
    npr = q;
    npc = q;

    long n;
    int *neuron_types;

    if(npr * npc != size)
    {
    	if(my_rank == 0){
    		printf("Error: it supports only square 2D MPI grid. The given number of MPI processes %d is invalid\n", size);
    	}
    	exit(1);
    }

    if(argc < 2)
    {	
    	if(my_rank == 0){
    		printf("Error: please input the size of matrix to be generated after the executable %s\n", argv[0]);
    	}
    	exit(1);
    }

    n = atoll(argv[1]);

    long nbRow = n/npr;
    long nbCol = n/npc;

    if(n % npr != 0)
    {
    	if(my_rank == 0){
    		printf("Error: the matrix size %li is not divisible by npr %d\n", n, npr);
    	}
    	exit(1);
    }	

    if(n % npc != 0)
    {
    	if(my_rank == 0){
    		printf("Error: the matrix size %li is not divisible by npr %d\n", n, npc);
    	}
    	exit(1);
    }	

    /*Fetching a raw-coded example brain coded in hardbrain.h*/
	Brain brain = generate_hard_brain(n);
    //Brain brain;
    //paramBrain(&brain, &n);

    if (my_rank == 0 && debug) 
    {
    	printf_recap_brain(&brain);
    }

    struct MatrixDist MatrixDist;
    MatrixDist = initMatrixDist(my_rank, npr, npc, n);
    if (my_rank == 0 && debug)
    {
        printf("----------------------\nInfo of your matrix :\n");
        printf("Size : %li * %li = %li\n",n,n, n * n);
        printf("%i blocks on rows (with %li lines per block) and %i blocks on columns (with %li columns per block)\n",npr,nbRow,npc,nbCol);
    }

    /*------------------------------------------------------------------------------------------------------------------------------------*/
    /*------------------------------------------- BRAIN MATRIX GENERATION STARTS ---------------------------------------------------------*/
    /*------------------------------------------------------------------------------------------------------------------------------------*/

    //four memory allocations are made during generation: one for the types of neurons, and three for the matrix.
    struct csr A_CSR;
	neuron_types = (int *)malloc(n * sizeof(int)); //vector which will contain the types of all the neurons

    /* Step 1 : Neuron types random choice */
    //choice of neuron type for each neuron in the brain
    if (MatrixDist.indc == 0) //choice of processes that will randomly choose the types of neurons
    {
    	generate_neuron_types(&brain, MatrixDist.indl*nbRow, nbRow, neuron_types + MatrixDist.indl*nbRow);
    }	

    //redistribution of selected neuron types to other processes
    for (int i = 0; i < npr; i++) 
    {
    	MPI_Bcast(neuron_types + i * nbRow, nbRow, MPI_INT, i * npc, MPI_COMM_WORLD);
    }

    /* Step 2 : Matrix generation */
    struct BrainMatrixInfo MatrixDebugInfo;
    brainAdjMatrixCSR(&A_CSR, MatrixDist, &brain, neuron_types, &MatrixDebugInfo);

    /*------------------------------------------------------------------------------------------------------------------------------------*/
    /*------------------------------------------- BRAIN MATRIX GENERATION ENDS -----------------------------------------------------------*/
    /*------------------------------------------------------------------------------------------------------------------------------------*/

    MPI_Barrier(MPI_COMM_WORLD);
    //allreduce to calculate the total allocated memory 
    MPI_Allreduce(MPI_IN_PLACE, &(MatrixDebugInfo.total_memory_allocated), 1, MPI_LONG, MPI_SUM, MPI_COMM_WORLD); //somme MPI_SUM de tout les total_memory_allocated_local dans MatrixDebugInfo.total_memory_allocated.

    //Matrix prints (only if there is less than 65 neurons)
    if (my_rank == 0) {
    	printf("\n-------- Matrices:\n");
	}

    for (int k = 0;k < size; k++)
    {
        if (my_rank == k)
        {
            printf("Process %i Matrix :\n",my_rank);
            csrDisplay(&A_CSR, 33);
        }
    }

    //Print of part, type and connection information for each neuron
    if (n < 65) //info will be printed for all neurons if there is less than 65 neurons
    {
        if (my_rank==0) 
        {
        	printf("\nInformations for all neurons:\n");
        }
    }
    else //otherwise, only a few neurons info will be printed
    {
        if (my_rank==0) 
        {
        	printf("\nInformation from a few neurons (impossible to display them all, there are too many): \n");
        }
    }

    int part, type;
    long nbco;
    double pourcentage_espere, sum_pourcentage_espere;
    for(int i = 0;i < n; i++) //Run through neurons
    {
        part = get_brain_part_ind(i, &brain);
        type = neuron_types[i];
        pourcentage_espere = get_mean_connect_percentage_for_part(&brain, part, type);
        if (n < 65) //print info for all neurons only if there is less than 65 neurons
        {
            if (my_rank == 0)
            {
                nbco = MatrixDebugInfo.nb_connections[my_rank*nbRow+i];
                printf("neuron %i, type: %i, part: %i, connections: %li, obtained connection percentage: %.2f, expected : %.2f\n",i,type,part,nbco,(double) nbco / (double) n * 100,pourcentage_espere);
            }
        }
        else if (i % (nbRow / 2) == 0) //otherwise, selection of some neurons and print
        {
            if (my_rank == 0)
            {
                nbco = MatrixDebugInfo.nb_connections[my_rank* nbRow +i];
                printf("neuron %i, type: %i, part: %i, connections: %li, obtained connection percentage: %.2f, expected : %.2f\n",i,type, part,nbco,(double) nbco / (double) n * 100,pourcentage_espere);
            }
        }
        sum_pourcentage_espere += pourcentage_espere;
    }
    //Print global information
    if (my_rank==0)
    {
        printf("\nOverall percentage of non-zero values : %.2f, expected : %.2f\n\n",((double) MatrixDebugInfo.cpt_values/(double) (n * n))  * 100,sum_pourcentage_espere/ (double) n);
    }

    free_brain(&brain); //see brainstruct.h
    free(neuron_types); //<=> free(MatrixDebugInfo.types)
    free(A_CSR.Row); free(A_CSR.Column); free(A_CSR.Value);
    free(MatrixDebugInfo.nb_connections);

    MPI_Finalize();
    return 0;
}	