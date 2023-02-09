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

#include "littleConfigTest.h"

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

    if(npr * npc != size)
    {
        if(my_rank == 0){
            printf("Error: it supports only square 2D MPI grid. The given number of MPI processes %d is invalid\n", size);
        }
        exit(1);
    }

    long long n;
    int *neuron_types;

    /*Fetching a config brain structure*/
    Brain brain;
    paramBrain(&brain, &n);

    long nbRow = n/npr;
    long nbCol = n/npc;

    if(n % npr != 0)
    {
        if(my_rank == 0){
            printf("Error: the matrix size %llu is not divisible by npr %d\n", n, npr);
        }
        exit(1);
    }   

    if(n % npc != 0)
    {
        if(my_rank == 0){
            printf("Error: the matrix size %llu is not divisible by npr %d\n", n, npc);
        }
        exit(1);
    }   

    if (my_rank == 0 && debug)
    {
        printf("----------------------\nInfo of your matrix :\n");
        printf("Size : %llu * %llu = %llu\n",n,n, n * n);
    }

    if (my_rank == 0 && debug) 
    {
        printf_recap_brain(&brain);
    }

    struct MatrixDist matrixBlock;
    matrixBlock = initMatrixDist(my_rank, npr, npc, n);

    //four memory allocations are made during generation: one for the types of neurons, and three for the matrix.
    struct csr A_CSR;
    neuron_types = (int *)malloc(n * sizeof(int)); //vector which will contain the types of all the neurons

    /* Step 1 : Neuron types random choice */
    //choice of neuron type for each neuron in the brain
    if (matrixBlock.indc == 0) //choice of processes that will randomly choose the types of neurons
    {
        generate_neuron_types(&brain, matrixBlock.indl*nbRow, nbRow, neuron_types + matrixBlock.indl*nbRow);
    }   

    //redistribution of selected neuron types to other processes
    for (int i = 0; i < npr; i++) 
    {
        MPI_Bcast(neuron_types + i * nbRow, nbRow, MPI_INT, i * npc, MPI_COMM_WORLD);
    }

    /* Step 2 : Matrix generation */
    struct BrainMatrixInfo MatrixDebugInfo;
    brainAdjMatrixCSR(&A_CSR, matrixBlock, &brain, neuron_types, &MatrixDebugInfo);

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

    free_brain(&brain); //see brainstruct.h
    free(neuron_types); //<=> free(MatrixDebugInfo.types)
    free(A_CSR.Row); free(A_CSR.Column); free(A_CSR.Value);
    free(MatrixDebugInfo.nb_connections);    

    MPI_Finalize();
    return 0;
}       