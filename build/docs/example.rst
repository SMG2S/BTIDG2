Example
==========

Two examples are provided in BTIDG2: 
	1. using the hard-coded brain structure to generate a matrix of a user-defined size
	2. using the user provided brain structure confiuration file to generate a brain matrix

Hard-coded Brain
-----------------

The workflow of using a hard-coded brain structure is as follow:


1. Initialize the MPI environment
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block :: c

    //Initialisation MPI
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    int q = sqrt(size);
    int npr; //number of row in the 2D MPI grid
    int npc; //number of column in the 2D MPI grid
    npr = q;
    npc = q;

    long long n = 64; //size of matrix to be generated
    long nbRow = n/npr;
    long nbCol = n/npc;

    /*
        for the hard-coded brain sturcture, 
            1. MPI 2D grid is expected to be square,
            2. n is divisible by npr, and npc
            3. n is divisble by 8
    */

2. Fetching a hard-coded example brain
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block :: c

    Brain brain = generate_hard_brain(n);

    //display the brief info of generated brain structure
    if (my_rank == 0 && debug) 
    {
    	printf_recap_brain(&brain);
    }


3. Initialize the distribution of sparse matrix
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block :: c

    struct MatrixDist MatrixDist;
    MatrixDist = initMatrixDist(my_rank, npr, npc, n);
    if (my_rank == 0 && debug)
    {
        printf("----------------------\nInfo of your matrix :\n");
        printf("Size : %li * %li = %li\n",n,n, n * n);
        printf("%i blocks on rows (with %li lines per block) and %i blocks on columns (with %li columns per block)\n",npr,nbRow,npc,nbCol);
    }

4. neuron types random choice
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block :: c

    int *neuron_types;
    neuron_types = (int *)malloc(n * sizeof(int)); //vector which will contain the types of all the neurons

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


5. generation of brain matrix
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Below is the step of the generation of a brain matrix, which is stored in `A_CSR`.

.. code-block :: c

    struct csr A_CSR;

    struct BrainMatrixInfo MatrixDebugInfo;
    brainAdjMatrixCSR(&A_CSR, MatrixDist, &brain, neuron_types, &MatrixDebugInfo);

6. clean up
^^^^^^^^^^^^^^

.. code-block :: c

    free_brain(&brain); 
    free(neuron_types); 
    free(A_CSR.Row); 
    free(A_CSR.Column); 
    free(A_CSR.Value);
    free(MatrixDebugInfo.nb_connections);

Configured Brain
-----------------

BTIDG2 provides a parser based on Python which is able to convert a formatted `JSON` configure file to a `C` header file with 
user desgined brain structure.

1. JSON config file
^^^^^^^^^^^^^^^^^^^^^

Here's an example of JSON configuration file, which determines two different connected brain parts.


.. code-block :: c

	[
		{
			"namePart": "part1",
			"id": 0,
			"connectionOpposite" : 0.2,
			"distribution": [0.8,0.2],
			"nbTypeNeuron": 2,
			"nbNeuron": 10,
			"typeNeuron":[
				{
					"name": "A",
					"nbNeuron": 7,
					"nbConnection": 2
				},
				{
					"name": "B",
					"nbNeuron": 3,
					"nbConnection": 5
				}
			]
		},
		{
			"namePart": "part2",
			"id": 1,
			"connectionOpposite" : 0.2,
			"distribution": [0.25,0.75],
			"nbTypeNeuron": 1,
			"nbNeuron":5,
			"typeNeuron":[
				{
					"name": "C",
					"nbNeuron": 5,
					"nbConnection": 6
				}
			]
		}
	]


The description of the parts are:
	- `Distribution`: a table that contains the probabilities of distribution to the other parties. At index 0 connection to the part of index 0. The table contains the internal connections between the same part
 	- `connectionOpposite`: probability of connection to the opposite side of the brain (left/right).
 	- `typeNeuron`: a dictionary table corresponding to the neuron present in the part of the brain.
 	- `nbNeuron`: the number of neurons.
 	- `nbConnection`: the number of connections of a neuron.
 	- `id`, `namePart`, `name`: fields for humans, they are only used to identify themselves.


2. convert JSON file to C header
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Assume that the config file is `littleConfigTest.json`, a C header file `littleConfigTest.h` can be generated within
the same folder through the following converter.


.. code-block :: console

    python ${BTIDG2_ROOT}/translator/converter.py ittleConfigTest.json


The generated header file is as below:

.. code-block :: c

	int get_nb_part(){
		return 4;
	}

	void destoryBrain(Brain *Cerveau){
		free(Cerveau->brainPart[0].repartitionNeuronCumulee);
		free(Cerveau->brainPart[0].probaConnection);
		free(Cerveau->brainPart[1].repartitionNeuronCumulee);
		free(Cerveau->brainPart[1].probaConnection);
		free(Cerveau->brainPart[2].repartitionNeuronCumulee);
		free(Cerveau->brainPart[2].probaConnection);
		free(Cerveau->brainPart[3].repartitionNeuronCumulee);
		free(Cerveau->brainPart[3].probaConnection);
		free(Cerveau->parties_cerveau);
		free(Cerveau->brainPart);
	}

	void paramBrain(Brain *Cerveau, long long *n){
		int nbTypeNeuronIci,nb_part=4;
		*n=15;
		BrainPart *brainPart = malloc(sizeof(BrainPart)*nb_part);
		long long *part_cerv = malloc(sizeof(long long)*nb_part);
		part_cerv[0] = 0;
		part_cerv[1] = 5;
		part_cerv[2] = 7;
		part_cerv[3] = 12;
	//partie 0
		brainPart[0].nbTypeNeuron = 2;
		brainPart[0].repartitionNeuronCumulee = malloc(sizeof(double)*2);
		brainPart[0].repartitionNeuronCumulee[0] = 0.7;
		brainPart[0].repartitionNeuronCumulee[1] = 1.0;
		brainPart[0].probaConnection = malloc(sizeof(double)*8);
		brainPart[0].probaConnection[0] = 0.08533333333333334;
		brainPart[0].probaConnection[1] = 0.021333333333333336;
		brainPart[0].probaConnection[2] = 0.021333333333333336;
		brainPart[0].probaConnection[3] = 0.005333333333333334;
		brainPart[0].probaConnection[4] = 0.21333333333333337;
		brainPart[0].probaConnection[5] = 0.053333333333333344;
		brainPart[0].probaConnection[6] = 0.053333333333333344;
		brainPart[0].probaConnection[7] = 0.013333333333333336;
	//partie 1
		brainPart[1].nbTypeNeuron = 1;
		brainPart[1].repartitionNeuronCumulee = malloc(sizeof(double)*1);
		brainPart[1].repartitionNeuronCumulee[0] = 1.0;
		brainPart[1].probaConnection = malloc(sizeof(double)*4);
		brainPart[1].probaConnection[0] = 0.08000000000000002;
		brainPart[1].probaConnection[1] = 0.24000000000000005;
		brainPart[1].probaConnection[2] = 0.020000000000000004;
		brainPart[1].probaConnection[3] = 0.06000000000000001;
	//partie 2
		brainPart[2].nbTypeNeuron = 2;
		brainPart[2].repartitionNeuronCumulee = malloc(sizeof(double)*2);
		brainPart[2].repartitionNeuronCumulee[0] = 0.7;
		brainPart[2].repartitionNeuronCumulee[1] = 1.0;
		brainPart[2].probaConnection = malloc(sizeof(double)*8);
		brainPart[2].probaConnection[0] = 0.021333333333333336;
		brainPart[2].probaConnection[1] = 0.005333333333333334;
		brainPart[2].probaConnection[2] = 0.08533333333333334;
		brainPart[2].probaConnection[3] = 0.021333333333333336;
		brainPart[2].probaConnection[4] = 0.053333333333333344;
		brainPart[2].probaConnection[5] = 0.013333333333333336;
		brainPart[2].probaConnection[6] = 0.21333333333333337;
		brainPart[2].probaConnection[7] = 0.053333333333333344;
	//partie 3
		brainPart[3].nbTypeNeuron = 1;
		brainPart[3].repartitionNeuronCumulee = malloc(sizeof(double)*1);
		brainPart[3].repartitionNeuronCumulee[0] = 1.0;
		brainPart[3].probaConnection = malloc(sizeof(double)*4);
		brainPart[3].probaConnection[0] = 0.020000000000000004;
		brainPart[3].probaConnection[1] = 0.06000000000000001;
		brainPart[3].probaConnection[2] = 0.08000000000000002;
		brainPart[3].probaConnection[3] = 0.24000000000000005;
		Cerveau->dimension = *n;
		Cerveau->nb_part = nb_part;
		Cerveau->parties_cerveau = part_cerv;
		Cerveau->brainPart = brainPart;
	}


3. Usage of generated header file
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The first step is to include the generated header file, here `littleConfigTest.h`, in your source code.

The most steps of generation is the same as the one with hard-code examples. However, in this 
example the size of matrix to be generated should be extracted from `littleConfigTest.h`, rather than
using any size defined by users.

.. code-block :: c

    #include "littleConfigTest.h"

    ....

    Brain brain;
    //construct a brain structure and get the matrix size
    //to be generated.
    paramBrain(&brain, &n);
     
    ...
