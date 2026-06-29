API
================


Sparse Matrix
--------------

Structs
^^^^^^^^

.. doxygenstruct:: coo

.. doxygenstruct:: csr

.. doxygenstruct:: MatrixDist


Functions
^^^^^^^^^^

.. doxygenfunction:: csrGetValue


.. doxygenfunction:: csrDisplay

.. doxygenfunction:: initMatrixDist


Brain Structure
----------------


Structs
^^^^^^^^

.. doxygenstruct:: BrainPart

.. doxygenstruct:: Brain


Functions
^^^^^^^^^^

.. doxygenfunction:: choose_neuron_type

.. doxygenfunction:: free_brain

.. doxygenfunction:: generate_neuron_types

.. doxygenfunction:: get_brain_part_ind

.. doxygenfunction:: get_mean_connect_percentage_for_part

.. doxygenfunction:: get_nb_neuron_brain_part

.. doxygenfunction:: printf_recap_brain


Hard-coded Brain
-----------------

.. doxygenfunction:: generate_hard_brain


Brain Matrix Generation
-------------------------

Structs
^^^^^^^^

.. doxygenstruct:: BrainMatrixInfo


Functions
^^^^^^^^^^

.. doxygenfunction:: brainAdjMatrixCSR


.. doxygenfunction:: brainTransAdjMatrixCSR

.. doxygenfunction:: brainTransAdjMatrixCSR1D

.. doxygenfunction:: brainTransAdjMatrixCOO1D


