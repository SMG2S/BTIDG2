Quick Start
==============


Dependencies
-------------

The dependencies are:

	- a C compiler

	- CMake >= 3.18.4

	- MPI (Message Passing Interface)

Build
-----------------------

1. Get the code from github

.. code-block:: console

	git clone https://github.com/SMG2S/BTIDG2.git



2. get into the project folder

.. code-block:: console

	cd btidg2


3. make a directory for building and compiling, then change into this new built folder

.. code-block:: console

	mkdir build
	cd build


4. build the code with CMake

.. code-block:: console

	cmake ..
	make -j

5. in the folder `example`, two executable binaries are available, and the execution is:

.. code-block:: console
	
	mpirun -n ${NPROCS} ./example/BTIG2_A -n ${N}

and

.. code-block:: console
	
	mpirun -n ${NPROCS} ./example/BTIG2_AT -n ${N}


in which `${NPROCS}` is the number of `MPI` processes to be used, and `${N}` is the size of squared to be generated. 