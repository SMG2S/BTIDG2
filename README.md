# BTIDG2
Brain Topology Inspired Distributed Graph Generator

What is BTIDG2?
----------------

***BTIDG2***, short for Brain Topology Inspired Distributed Graph Generator, is 
able to generate very large graphs which is inspired by the topology of humain brain.
This matrix generated is implemented based C, and parallised based on MPI. 

As a software, ***BTIDG2*** ensures that:

	- the generator must be able to work for large sparse matrices without any I/Os

	- the generator must work in a distributed fashion

	- support multiple (COO and CSR) distributed sparse data formats

	- the generator must be as configurable as possible

	- a ***converter***, allowing to convert a data file containing the brain information as we found it on the internet into an input file for our matrix generator.


Motivation
------------

Graph data can be used to represent data from various domains: Web graphs, physical or
biological systems, communication networks, etc. Graph data analysis can provide valuable insight on the underlying networks. 
More recently, Machine Learning techniques have also been
applied to this type of data. However, the size of these graphs is always growing, which poses
significant challenges to be able to apply these methods.

In particular, when the researchers study the scalability of
the PageRank algorithm on very large matrices in distributed computing environments, e.g., RIKEN’s Fugaku, one of the most powerful supercomputers
in the world according to the latest TOP500 ranking. Evaluating algorithms like the PageRank on very large matrices is impossible to do directly
with sequential method and it is also not possible to have an input file containing the graph.
Indeed, with data sets this large, input-outputs would be far too costly.

Evaluating those algorithms therefore requires distributed data sets and graphs that almost
fill the very large memories available, and are generated directly in parallel without I/O.


Documentation
---------------

Please find the documentation of BTIDG2 [here](https://smg2s.github.io/BTIDG2/)

Main developers
----------------

  * Nicolas Hochart - Software desgin and parallel implementation


Contributors
-------------   

  * Serge G. Petiton - Algorithm design and charge of project
  * Maxence Vandromme - Benchmarks on Fuguka supercomputer
  * Xinzhe Wu - Software re-design, documentation



Contact
---------


  * Serge G. Petiton: SERGE DOT PETITON AT UNIV-LILLE DOT FR
