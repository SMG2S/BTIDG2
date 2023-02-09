// Brain structures
/*
  This file defines the structures and functions related to the Brains used by the brain-matrix generator
  Nicolas HOCHART
*/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>

#ifndef randomforbrain
#include "randomforbrain.h"
#endif

/** @defgroup brainstruct brain structure 
 * 
 *  @brief  definition of brain and its parts
 * 
 *  @{
 */

//! Structure containing the information of a part of the brain
/*!
   It contains :
       1. The number of neuron types that can be encountered in the part
       2. the cumulative distribution of neurons in the neuron types
       3. the probability of connection (for each type) to other parts of the brain.
   
   This structure depends on the brain to which it belongs.
 */
struct BrainPart
{
     int nbTypeNeuron; //!< \public the number of neuron types that can be encountered in the part
     double * repartitionNeuronCumulee; //!< \public the cumulative distribution of neurons in the neuron types
     double * probaConnection; //!< \public the probability of connection (for each type) to other parts of the brain
};
typedef struct BrainPart BrainPart;

//! Structure containing the information of a brain
/*!
   It contains :
       1. the total number of neurons
       2. the number of parts in the brain
       3. the indices (neurons) at which parts start
       4. the brain parts (see BrainPart).
 */
struct Brain
{
     long long dimension; //!< \public the total number of neurons 
     int nb_part; //!< \public the number of parts in the brain 
     long long * parties_cerveau; //!< \public the indices (neurons) at which parts start
     BrainPart * brainPart; //!< \public the brain parts (see BrainPart).
};
typedef struct Brain Brain;


//! Function that returns the index of the part of the brain in which a neuron is located
/*!
   Function that returns the index of the part of the Brain in which the neuron of index `ind` is located
 * @param[in] ind  index of the neuron
 * @param[in] brain pointer to a brain
 * @return i index of the brain part
 * 
   **Condition**: Brain is a well formed and `ind` is assumed to be between `0` and `brain.dimension`
 */
int get_brain_part_ind(long long ind, Brain * brain)
{
    /*
    Renvoie l'indice de la partie du cerveau dans laquelle le neurone "ind" se situe
    Brain est supposé être un cerveau bien formé et ind est supposé être entre 0 et brain.dimension
    */
    long long * parts_cerv = (*brain).parties_cerveau;
    if (ind >= parts_cerv[(*brain).nb_part - 1])
    {
        return (*brain).nb_part - 1;
    }
    int i=0;
    while (ind >= parts_cerv[i+1])
    {
        i++;
    }
    return i;
}

//! Function that returns the number of neurons in a specific brain part
/*!
   Function that returns the number of neurons in the brain part if index `part`, in the Brain `brain`
 * @param[in] ind index of the neuron
 * @param[in] brain pointer to a brain
 * @return  number of neurons in the brain part
 */
int get_nb_neuron_brain_part(Brain * brain, int part)
{
    long long n = (*brain).dimension;
    long long ind_depart = (*brain).parties_cerveau[part];
    if (part+1 == (*brain).nb_part)
    {
        return n - ind_depart;
    }
    else
    {
        long long ind_fin = (*brain).parties_cerveau[part+1];
        return ind_fin - ind_depart;
    }
}

//! Function that returns the average percentage of connection chances for a specific neuron to the other parts
/*!
   Function that returns the average percentage (between `0` and `100`) of connection chances for a neuron of a given type in a given part, to the other parts
 * @param[in] brain pointer to a brain
 * @param[in] part  brain part index
 * @param[in] type  neuron index
 * @return average percentage of connection chance
 */
double get_mean_connect_percentage_for_part(Brain * brain, int part, int type)
{
    long long n,i;
    int nb_part;
    double * probCo = (*brain).brainPart[part].probaConnection; 
    n = (*brain).dimension;
    nb_part = (*brain).nb_part;

    double sum_proba = 0;
    for (i=0;i<nb_part;i++)
    {
        sum_proba += (double) get_nb_neuron_brain_part(brain,i) * (*brain).brainPart[part].probaConnection[type*nb_part + i];
    }
    return sum_proba/n *100;
}

//! Choose and returns the neuron type for a neuron in a specitic brain part
/*!
   Choose and returns a neuron type for a neuron in the brain part of index "part", in the brain "brain"
 * @param[in] brain pointer to a brain
 * @param[in] part brain part index
 * @return neuron type
 */
int choose_neuron_type(Brain * brain, int part)
{
    if (part >= (*brain).nb_part)
    {
        printf("Error in choose_neuron_type: part number %i greater than number of parts in brain %i.\n",part,(*brain).nb_part);
        exit(1);
    }
    int i=0;
    double * repNCumulee = (*brain).brainPart[part].repartitionNeuronCumulee; //Repartition cumulée des neurones dans les parties
    double decision = rand_0_1();
    while (repNCumulee[i] < decision)
    {
        i++;
    }
    return i;
}

//! Function that generates (chooses) the types of multiple neurons and writes them in "types"
/*!
   Generates the types of neurons from index "ind_start_neuron" to "ind_start_neuron + nb_neuron" in the brain "brain", and writes them in "types"
 * @param[in] brain  pointer to a brain
 * @param[in] part brain part index
 * @return neuron type
 * 
   **Condition**: The memory allocation (malloc of size nb_neuron * sizeof(int)) for "types" must be done beforehand. 
 */
void generate_neuron_types(Brain * brain, int ind_start_neuron, int nb_neuron, int * types)
{
    /*
      Decides the types of neurons number "ind_start_neuron" to "ind_start_neuron + nb_neuron" in the brain "Brain", and writes them in "types"
      A malloc of size nb_neuron * sizeof(int) must have been made beforehand for the "types" pointer.
    */
    long long i;
    int ind_part;
    for (i=0;i<nb_neuron;i++) //parcours des lignes
    {
        //récupération de l'indice de la partie source
        ind_part = get_brain_part_ind(ind_start_neuron+i, brain);
        //décision du type de neurone
        types[i] = choose_neuron_type(brain, ind_part);
    }
}

//! Brain print
/*!
   Function that displays a summary of the brain passed as a parameter (useful for debugging a brain)
 * @param[in] brain pointer to a brain
 */
void printf_recap_brain(Brain * brain)
{
    int i,j,k;
    /*affiche un récapitulatif du cerveau passé en paramètre.*/
    printf("\n#############\nInfo of Brain :\n");

    printf("Size: %llu*%llu\nNumber of games: %d\nIndices at which games start: [",(*brain).dimension,(*brain).dimension,(*brain).nb_part);
    for (i=0; i<(*brain).nb_part; i++)
    {
        printf("%llu ",(*brain).parties_cerveau[i]);
    }
    printf("]\n\n");
    for (i=0; i<(*brain).nb_part; i++)
    {
        printf("\n");
        printf("Part %i:\n\tNumber of neuron types: %i\n\t",i,(*brain).brainPart[i].nbTypeNeuron);
        printf("Cumulative probabilities of belonging to each type of neuron : [");
        for (j=0;j<(*brain).brainPart[i].nbTypeNeuron;j++)
        {
            printf("%lf ",(*brain).brainPart[i].repartitionNeuronCumulee[j]);
        }
        printf("]\n\tConnections :\n\t");
        for (j=0;j<(*brain).brainPart[i].nbTypeNeuron;j++)
        {
            printf("Indices of neuron type connections %i :\n\t",j);
            for (k=0;k<(*brain).nb_part;k++)
            {
                printf("%i -> %i : %lf\n\t",i,k,(*brain).brainPart[i].probaConnection[j*(*brain).nb_part + k]);
            }
        }
    }
    printf("\n");
}

//! Brain destructor
/*!
   Function that frees a brain
 * @param[in] brain pointer to a brain
 */
void free_brain(Brain * brain)
{
    /*libère les ressources allouées dans un cerveau. Ne libère/détruit pas le cerveau.*/
    for (int i=0; i<(*brain).nb_part; i++)
    {
        free((*brain).brainPart[i].repartitionNeuronCumulee);
        free((*brain).brainPart[i].probaConnection);
    }
    free((*brain).brainPart);
    free((*brain).parties_cerveau);
}

/** @} */ // end of brainstruct
