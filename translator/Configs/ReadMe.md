# Composition of json config files

## Description of the parts

 - Distribution: a table that contains the probabilities of distribution to the other parties. At index 0 connection to the part of index 0. The table contains the internal connections between the same part
 - connectionOpposite: probability of connection to the opposite side of the brain (left/right).
 - typeNeuron: a dictionary table corresponding to the neuron present in the part of the brain.
 - nbNeuron: the number of neurons.
 - nbConnection: the number of connections of a neuron.
 - id, namePart, name: fields for humans, they are only used to identify themselves.

It is also very simple to add information such as neurotransmitters