// Matrix generator
/*
  This file defines the functions allowing to generate a matrix in a parallel way, taking as input a brain.
  Nicolas HOCHART
*/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>

#include "brainstruct.h"
#include "randomforbrain.h"
#include "matrixstruct.h"

/** @defgroup generation Brain Matrix Generation
 * 
 * @brief Funtions of the generation of brain matrix
 * 
 *  @{
 */
//! Structure containing debug info for a generated matrix
/*!
   It contains :
    1. the number of row of the local matrix
    2. the number of column of the local matrix    
    3. the types chosen for the neurons
    4. the number of connections (number of non-zero values) per row or column (depending on the version of the generator used)
    5. number of memory cells allocated
    6. the total number of non-zero values contained in the matrix (all processes)
 */
struct BrainMatrixInfo
{
     long dim_c; //!< \public the number of column of the local matrix   
     long dim_l; //!< \public  the number of row of the local matrix
     int * types; //!< \public  the types chosen for the neurons
     long * nb_connections; //!< \public the number of connections (number of non-zero values) per row or column (depending on the version of the generator used)
     long total_memory_allocated; //!< \public number of memory cells allocated
     long cpt_values; //!< \public the total number of non-zero values contained in the matrix (all processes)
};
typedef struct BrainMatrixInfo BrainMatrixInfo;


/*!
 * Generates a CSR square adjacency matrix of dimension (*brain).dimension, corresponding to the brain passed as a parameter, in a two-dimensional process grid. 
   A row/column of the matrix corresponding to a neuron, adjacency matrix means that it is the row-neuron that connect to the column-neuron

 * @param[in] BlockInfo structure containing information about the local mpi process ("block")
 * @param[in] brain Pointer to the brain, basis for the generation of the matrix
 * @param[in] neuron_types vector containing the types chosen for the neurons of the brain passed as a parameter
 * @param[out] M_CSR Pointer to a structure corresponding to a CSR matrix. At the end of the generation, contains the generated matrix.
 * @param[out] debugInfo **OPTIONAL** - Pointer to a debug structure or NULL. If not NULL, at the end of the generation, contains debug information such as the number of connections made per neuron (<=> number of 1 on each row), the total number of connections, etc.
   

   Condition : BlockInfo must have been filled with information suitable for the brain (Otherwise, the generation will not necessarily fail, but the generated matrix will not necessarily correspond to the brain)
 */
void brainAdjMatrixCSR(csr *M_CSR, MatrixDist BlockInfo, Brain * brain, int * neuron_types, BrainMatrixInfo * debugInfo)
{
    long i,j,cpt_values,size = BlockInfo.dim_l * BlockInfo.dim_c;
    long * nb_connections_local_tmp; //utilisé seulement si debugInfo != NULL
    int ind_part_source,ind_part_dest,source_type; double proba_connection,proba_no_connection,random;
    (*M_CSR).dim_l = BlockInfo.dim_l; (*M_CSR).dim_c = BlockInfo.dim_c;
    if (debugInfo != NULL)
    {
        (*debugInfo).dim_l = BlockInfo.dim_l; (*debugInfo).dim_c = BlockInfo.dim_c;
        (*debugInfo).types = neuron_types;
        //Attention : le malloc de (*debugInfo).nb_connections n'est pas free dans la fonction.
        (*debugInfo).nb_connections = (long *)malloc((*brain).dimension * sizeof(long)); //contiendra le nombre de connexions faites au global (tout processus confondu)
        nb_connections_local_tmp = (long *)malloc((*brain).dimension * sizeof(long)); //contiendra le nombre de connexions faites en local (décalage en ligne prit en compte)
        for (i=0;i<(*brain).dimension;i++)
        {
            nb_connections_local_tmp[i] = 0;
            (*debugInfo).nb_connections[i] = 0;
        }
    }

    //allocations mémoires
    (*M_CSR).Row = (int *)malloc(((*M_CSR).dim_l+1) * sizeof(int));
    //La mémoire allouée pour Column est à la base de 1/10 de la taille de la matrice stockée "normalement". Au besoin, on réalloue de la mémoire dans le code.
    long basic_size = (long) size/10;
    long total_memory_allocated = basic_size; //nombre total de cases mémoires allouées pour 1 vecteur
    (*M_CSR).Column = (int *)malloc(total_memory_allocated * sizeof(int));

    (*M_CSR).Row[0] = 0;
    cpt_values=0;
    for (i=0;i<BlockInfo.dim_l;i++) //parcours des lignes
    {
        //récupération de l'indice de la partie (du cerveau) source
        ind_part_source = get_brain_part_ind(BlockInfo.startRow+i, brain);
        //récupération du type de neurone source
        source_type = neuron_types[BlockInfo.startRow+i];
        for (j=0;j<BlockInfo.dim_c;j++) //parcours des colonnes
        {
            //récupération de l'indice de la partie destination
            ind_part_dest = get_brain_part_ind(BlockInfo.startColumn+j, brain);
            //récupération de la probabilité de connexion source -> destination avec le type de neurone donné
            proba_connection = (*brain).brainPart[ind_part_source].probaConnection[source_type*(*brain).nb_part + ind_part_dest];
            proba_no_connection = 1 - proba_connection;
            random = rand_0_1();
            //décision aléatoire, en prenant en compte l'abscence de connexion sur la diagonale de façon brute
            if ( (BlockInfo.startRow+i)!=(BlockInfo.startColumn+j) && random > proba_no_connection) //si on est dans la proba de connexion et qu'on est pas dans la diagonale, alors on place un 1
            {
                if (cpt_values >= total_memory_allocated)
                {
                    total_memory_allocated *= 2;
                    (*M_CSR).Column = (int *) realloc((*M_CSR).Column, total_memory_allocated * sizeof(int));
                    assert((*M_CSR).Column != NULL);
                }
                (*M_CSR).Column[cpt_values] = j;
                if (debugInfo != NULL)
                {
                    nb_connections_local_tmp[i + BlockInfo.startRow] = nb_connections_local_tmp[i + BlockInfo.startRow] + 1;
                }
                cpt_values++;
            }
        }
        (*M_CSR).Row[i+1] = cpt_values;
    }
    //remplissage du vecteur Value (avec précisement le nombre de 1 nécéssaire)
    (*M_CSR).Value = (int *)malloc(cpt_values * sizeof(int));
    (*M_CSR).nnz = cpt_values;
    for (i=0; i<cpt_values;i++) {(*M_CSR).Value[i] = 1;}

    //remplissage de la structure de débuggage
    if (debugInfo != NULL)
    {
        (*debugInfo).total_memory_allocated = total_memory_allocated;
        MPI_Allreduce(&((*M_CSR).nnz), &((*debugInfo).cpt_values), 1, MPI_LONG, MPI_SUM, MPI_COMM_WORLD); //somme MPI_SUM des nombres de valeurs dans les sous matrices dans cpt_values, nombre de valeurs non nulles global

        /* nb_connections_local_tmp contient actuellement (dans chaque processus) le nombre de connexions faites LOCALEMENT par tout les neurones par colonne. */
        MPI_Allreduce(nb_connections_local_tmp, (*debugInfo).nb_connections, (*brain).dimension, MPI_LONG, MPI_SUM, MPI_COMM_WORLD); //somme MPI_SUM de tout les nb_non_zeros_local dans (*debugInfo).nb_connections
        free(nb_connections_local_tmp);
        /* MatrixDebugInfo.nb_connections contient maintenant (dans tout les processus) le nombre GLOBAL de connexions faites pour chaque neurone. */
    }
}

/*!
   Generates a CSR square transposed adjacency matrix of dimension (*brain).dimension, corresponding to the brain passed as a parameter, in a two-dimensional process grid.
   A row/column of the matrix corresponding to a neuron, transposed adjacency matrix means that it is the column-neuron that connect to the row-neuron
 * @param[in] BlockInfo structure containing information about the local mpi process ("block")
 * @param[in] brain Pointer to the brain, basis for the generation of the matrix
 * @param[in] neuron_types vector containing the types chosen for the neurons of the brain passed as a parameter
 * @param[out] M_CSR Pointer to a structure corresponding to a CSR matrix. At the end of the generation, contains the generated matrix.
 * @param[out] debugInfo **OPTIONAL** - Pointer to a debug structure or NULL. If not NULL, at the end of the generation, contains debug information such as the number of connections made per neuron (<=> number of 1 on each column), the total number of connections, etc.
   

   Condition: BlockInfo must have been filled with information suitable for the brain (Otherwise, the generation will not necessarily fail, but the generated matrix will not necessarily correspond to the brain)
 */
void brainTransAdjMatrixCSR(csr *M_CSR, MatrixDist BlockInfo, Brain * brain, int * neuron_types, BrainMatrixInfo * debugInfo)
{    
    long i,j,cpt_values,size = BlockInfo.dim_l * BlockInfo.dim_c;
    long * nb_connections_local_tmp; //utilisé seulement si debugInfo != NULL
    int ind_part_source,ind_part_dest,source_type; double proba_connection,proba_no_connection,random;
    (*M_CSR).dim_l = BlockInfo.dim_l; (*M_CSR).dim_c = BlockInfo.dim_c;
    if (debugInfo != NULL)
    {
        (*debugInfo).dim_l = BlockInfo.dim_l; (*debugInfo).dim_c = BlockInfo.dim_c;
        (*debugInfo).types = neuron_types;
        //Attention : le malloc de (*debugInfo).nb_connections n'est pas free dans la fonction.
        (*debugInfo).nb_connections = (long *)malloc((*brain).dimension * sizeof(long)); //contiendra le nombre de connexions faites au global (tout processus confondu)
        nb_connections_local_tmp = (long *)malloc((*brain).dimension * sizeof(long)); //contiendra le nombre de connexions faites en local (décalage en ligne prit en compte)
        for (i=0;i<(*brain).dimension;i++)
        {
            nb_connections_local_tmp[i] = 0;
            (*debugInfo).nb_connections[i] = 0;
        }
    }

    //allocations mémoires
    (*M_CSR).Row = (int *)malloc(((*M_CSR).dim_l+1) * sizeof(int));
    //La mémoire allouée pour Column est à la base de 1/10 de la taille de la matrice stockée "normalement". Au besoin, on réalloue de la mémoire dans le code.
    long basic_size = (long) size/10;
    long total_memory_allocated = basic_size; //nombre total de cases mémoires allouées pour 1 vecteur
    (*M_CSR).Column = (int *)malloc(total_memory_allocated * sizeof(int));

    (*M_CSR).Row[0] = 0;
    cpt_values=0;
    for (i=0;i<BlockInfo.dim_l;i++) //parcours des lignes
    {
        //récupération de l'indice de la partie (du cerveau) destination
        ind_part_dest = get_brain_part_ind(BlockInfo.startRow+i, brain);
        for (j=0;j<BlockInfo.dim_c;j++) //parcours des colonnes
        {
            //récupération de l'indice de la partie source
            ind_part_source = get_brain_part_ind(BlockInfo.startColumn+j, brain);
            //récupération du type de neurone
            source_type = neuron_types[BlockInfo.startColumn+j];
            //récupération de la probabilité de connexion source -> destination avec le type de neurone donné
            proba_connection = (*brain).brainPart[ind_part_source].probaConnection[source_type*(*brain).nb_part + ind_part_dest];
            proba_no_connection = 1 - proba_connection;
            random = rand_0_1();
            //décision aléatoire, en prenant en compte l'abscence de connexion sur la diagonale de façon brute
            if ( (BlockInfo.startRow+i)!=(BlockInfo.startColumn+j) && random > proba_no_connection) //si on est dans la proba de connexion et qu'on est pas dans la diagonale, alors on place un 1
            {
                if (cpt_values >= total_memory_allocated)
                {
                    total_memory_allocated *= 2;
                    (*M_CSR).Column = (int *) realloc((*M_CSR).Column, total_memory_allocated * sizeof(int));
                    assert((*M_CSR).Column != NULL);
                }
                (*M_CSR).Column[cpt_values] = j;
                if (debugInfo != NULL)
                {
                    nb_connections_local_tmp[j + BlockInfo.startColumn] = nb_connections_local_tmp[j + BlockInfo.startColumn] + 1;
                }
                cpt_values++;
            }
        }
        (*M_CSR).Row[i+1] = cpt_values;
    }
    //remplissage du vecteur Value (avec précisement le nombre de 1 nécéssaire)
    (*M_CSR).Value = (int *)malloc(cpt_values * sizeof(int));
    (*M_CSR).nnz = cpt_values;
    for (i=0; i<cpt_values;i++) {(*M_CSR).Value[i] = 1;}

    //remplissage de la structure de débuggage
    if (debugInfo != NULL)
    {
        (*debugInfo).total_memory_allocated = total_memory_allocated;
        MPI_Allreduce(&((*M_CSR).nnz), &((*debugInfo).cpt_values), 1, MPI_LONG, MPI_SUM, MPI_COMM_WORLD); //somme MPI_SUM des nombres de valeurs dans les sous matrices dans cpt_values, nombre de valeurs non nulles global

        /* MatrixDebugInfo.nb_connections contient actuellement (dans chaque processus) le nombre de connexions faites LOCALEMENT par tout les neurones par colonne. */
        MPI_Allreduce(nb_connections_local_tmp, (*debugInfo).nb_connections, (*brain).dimension, MPI_LONG, MPI_SUM, MPI_COMM_WORLD); //somme MPI_SUM de tout les nb_non_zeros_local dans (*debugInfo).nb_connections
        free(nb_connections_local_tmp);
        /* MatrixDebugInfo.nb_connections contient maintenant (dans tout les processus) le nombre GLOBAL de connexions faites pour chaque neurone. */
    }
}

/*!
   Generates a CSR square transposed adjacency matrix of dimension (*brain).dimension, corresponding to the brain passed as a parameter, in a one-two-dimensional process grid (row block parallelization)
   A row/column of the matrix corresponding to a neuron, transposed adjacency matrix means that it is the column-neuron that connect to the row-neuron
 * @param[in] ind_start_row Row offset (in the 1D process grid), to be taken into account for the creation of connection between neurons
 * @param[in] brain Pointer to the brain, basis for the generation of the matrix
 * @param[in] neuron_types vector containing the types chosen for the neurons of the brain passed as a parameter
 * @param[in] l number of rows in the local (process) matrix
 * @param[in] c number of column in the local (process) matrix (= global matrix dimension, because the matrix is parallelized in row blocks only)
 * @param[out] M_CSR Pointer to a structure corresponding to a CSR matrix. At the end of the generation, contains the generated matrix.
 * @param[out] debugInfo **OPTIONAL** - Pointer to a debug structure or NULL. If not NULL, at the end of the generation, contains debug information such as the number of connections made per neuron (<=> number of 1 on each column), the total number of connections, etc.
   

   Condition : BlockInfo must have been filled with information suitable for the brain (Otherwise, the generation will not necessarily fail, but the generated matrix will not necessarily correspond to the brain)
 */
void brainTransAdjMatrixCSR1D(csr *M_CSR, long ind_start_row, Brain * brain, int * neuron_types, long l, long c, BrainMatrixInfo * debugInfo)
{
    long i,j,cpt_values,size=l*c;
    long * nb_connections_local_tmp; //utilisé seulement si debugInfo != NULL
    int ind_part_source,ind_part_dest,source_type; double proba_connection,proba_no_connection,random;
    (*M_CSR).dim_l = l; (*M_CSR).dim_c = c;
    if (debugInfo != NULL)
    {
        (*debugInfo).dim_l = l; (*debugInfo).dim_c = c;
        (*debugInfo).types = neuron_types;
        //Attention : le malloc de (*debugInfo).nb_connections n'est pas free dans la fonction.
        (*debugInfo).nb_connections = (long *)malloc((*debugInfo).dim_c * sizeof(long));
        nb_connections_local_tmp = (long *)malloc((*debugInfo).dim_c * sizeof(long));
        for (i=0;i<(*debugInfo).dim_c;i++)
        {
            nb_connections_local_tmp[i] = 0;
            (*debugInfo).nb_connections[i] = 0;
        }
    }

    //allocations mémoires
    (*M_CSR).Row = (int *)malloc(((*M_CSR).dim_l+1) * sizeof(int));
    //La mémoire allouée pour Column est à la base de 1/10 de la taille de la matrice stockée "normalement". Au besoin, on réalloue de la mémoire dans le code.
    long basic_size = (long) size/10;
    long total_memory_allocated = basic_size; //nombre total de cases mémoires allouées pour 1 vecteur
    (*M_CSR).Column = (int *)malloc(total_memory_allocated * sizeof(int));

    (*M_CSR).Row[0] = 0;
    cpt_values=0;
    for (i=0;i<l;i++) //parcours des lignes
    {
        //récupération de l'indice de la partie (du cerveau) destination
        ind_part_dest = get_brain_part_ind(ind_start_row+i, brain);
        for (j=0;j<c;j++) //parcours des colonnes
        {
            //récupération de l'indice de la partie source
            ind_part_source = get_brain_part_ind(j, brain);
            //récupération du type de neurone
            source_type = neuron_types[j];
            //récupération de la probabilité de connexion source -> destination avec le type de neurone donné
            proba_connection = (*brain).brainPart[ind_part_source].probaConnection[source_type*(*brain).nb_part + ind_part_dest];
            proba_no_connection = 1 - proba_connection;
            random = rand_0_1();
            //décision aléatoire, en prenant en compte l'abscence de connexion sur la diagonale de façon brute
            if ( (ind_start_row+i)!=j && random > proba_no_connection) //si on est dans la proba de connexion et qu'on est pas dans la diagonale, alors on place un 1
            {
                if (cpt_values >= total_memory_allocated)
                {
                    total_memory_allocated *= 2;
                    (*M_CSR).Column = (int *) realloc((*M_CSR).Column, total_memory_allocated * sizeof(int));
                    assert((*M_CSR).Column != NULL);
                }
                (*M_CSR).Column[cpt_values] = j;
                if (debugInfo != NULL)
                {
                    nb_connections_local_tmp[j] = nb_connections_local_tmp[j] + 1;
                }
                cpt_values++;
            }
        }
        (*M_CSR).Row[i+1] = cpt_values;
    }
    //remplissage du vecteur Value (avec précisement le nombre de 1 nécéssaire)
    (*M_CSR).Value = (int *)malloc(cpt_values * sizeof(int));
    (*M_CSR).nnz = cpt_values;
    for (i=0; i<cpt_values;i++) {(*M_CSR).Value[i] = 1;}

    //remplissage de la structure de débuggage
    if (debugInfo != NULL)
    {
        (*debugInfo).total_memory_allocated = total_memory_allocated;
        MPI_Allreduce(&((*M_CSR).nnz), &((*debugInfo).cpt_values), 1, MPI_LONG, MPI_SUM, MPI_COMM_WORLD); //somme MPI_SUM des nombres de valeurs dans les sous matrices dans cpt_values, nombre de valeurs non nulles global

        /* MatrixDebugInfo.nb_connections contient actuellement (dans chaque processus) le nombre de connexions faites LOCALEMENT par tout les neurones par colonne. */
        MPI_Allreduce(nb_connections_local_tmp, (*debugInfo).nb_connections, (*brain).dimension, MPI_LONG, MPI_SUM, MPI_COMM_WORLD); //somme MPI_SUM de tout les nb_non_zeros_local dans (*debugInfo).nb_connections
        free(nb_connections_local_tmp);
        /* MatrixDebugInfo.nb_connections contient maintenant (dans tout les processus) le nombre GLOBAL de connexions faites pour chaque neurone. */
    }
}

/*!
   Generates a COO square transposed adjacency matrix of dimension (*brain).dimension, corresponding to the brain passed as a parameter, in a one-two-dimensional process grid (row block parallelization)
   A row/column of the matrix corresponding to a neuron, transposed adjacency matrix means that it is the column-neuron that connect to the row-neuron
 * @param[in] ind_start_row Row offset (in the 1D process grid), to be taken into account for the creation of connection between neurons
 * @param[in] brain Pointer to the brain, basis for the generation of the matrix
 * @param[in] neuron_types vector containing the types chosen for the neurons of the brain passed as a parameter
 * @param[in] l number of rows in the local (process) matrix
 * @param[in] c number of column in the local (process) matrix (= global matrix dimension, because the matrix is ​​parallelized in row blocks only)
 * @param[out] M_CSR Pointer to a structure corresponding to a CSR matrix. At the end of the generation, contains the generated matrix.
 * @param[out] debugInfo **OPTIONAL** - Pointer to a debug structure or NULL. If not NULL, at the end of the generation, contains debug information such as the number of connections made per neuron (<=> number of 1 on each column), the total number of connections, etc.
   

   Condition : BlockInfo must have been filled with information suitable for the brain (Otherwise, the generation will not necessarily fail, but the generated matrix will not necessarily correspond to the brain)
 */
void brainTransAdjMatrixCOO1D(coo *M_COO, long ind_start_row, Brain * brain, int * neuron_types, long l, long c, BrainMatrixInfo * debugInfo)
{

    long i,j,cpt_values,size=l*c;
    long * nb_connections_local_tmp; //utilisé seulement si debugInfo != NULL
    int ind_part_source,ind_part_dest,source_type; double proba_connection,proba_no_connection,random;
    (*M_COO).dim_l = l; (*M_COO).dim_c = c;
    if (debugInfo != NULL)
    {
        (*debugInfo).dim_l = l; (*debugInfo).dim_c = c;
        (*debugInfo).types = neuron_types;
        //Attention : le malloc de (*debugInfo).nb_connections n'est pas free dans la fonction.
        (*debugInfo).nb_connections = (long *)malloc((*debugInfo).dim_c * sizeof(long));
        nb_connections_local_tmp = (long *)malloc((*debugInfo).dim_c * sizeof(long));
        for (i=0;i<(*debugInfo).dim_c;i++)
        {
            nb_connections_local_tmp[i] = 0;
            (*debugInfo).nb_connections[i] = 0;
        }
    }

    //La mémoire allouée est à la base de 1/10 de la taille de la matrice stockée "normalement". Au besoin, on réalloue de la mémoire dans le code.
    long basic_size = (long) size/10;
    long total_memory_allocated = basic_size; //nombre total de cases mémoires allouées pour 1 vecteur
    (*M_COO).Row = (int *)malloc(total_memory_allocated * sizeof(int));
    (*M_COO).Column = (int *)malloc(total_memory_allocated * sizeof(int));

    cpt_values=0;
    for (i=0;i<l;i++) //parcours des lignes
    {
        //récupération de l'indice de la partie (du cerveau) destination
        ind_part_dest = get_brain_part_ind(ind_start_row+i, brain);
        for (j=0;j<c;j++) //parcours des colonnes
        {
            //récupération de l'indice de la partie source
            ind_part_source = get_brain_part_ind(j, brain);
            //récupération du type de neurone
            source_type = neuron_types[j];
            //récupération de la probabilité de connexion source -> destination avec le type de neurone donné
            proba_connection = (*brain).brainPart[ind_part_source].probaConnection[source_type*(*brain).nb_part + ind_part_dest];
            proba_no_connection = 1 - proba_connection;
            random = rand_0_1();
            //décision aléatoire, en prenant en compte l'abscence de connexion sur la diagonale de façon brute
            if ( (ind_start_row+i)!=j && random > proba_no_connection) //si on est dans la proba de connexion et qu'on est pas dans la diagonale, alors on place un 1
            {
                if (cpt_values >= total_memory_allocated)
                {
                    total_memory_allocated *= 2;
                    (*M_COO).Row = (int *) realloc((*M_COO).Row, total_memory_allocated * sizeof(int));
                    (*M_COO).Column = (int *) realloc((*M_COO).Column, total_memory_allocated * sizeof(int));
                    assert((*M_COO).Row != NULL);
                    assert((*M_COO).Column != NULL);
                }
                (*M_COO).Row[cpt_values] = i;
                (*M_COO).Column[cpt_values] = j;
                if (debugInfo != NULL)
                {
                    nb_connections_local_tmp[j] = nb_connections_local_tmp[j] + 1;
                }
                cpt_values++;
            }
        }
    }
    //remplissage du vecteur Value (avec précisement le nombre de 1 nécéssaire)
    (*M_COO).Value = (int *)malloc(cpt_values * sizeof(int));
    (*M_COO).nnz = cpt_values;
    for (i=0; i<cpt_values;i++) {(*M_COO).Value[i] = 1;}

    //remplissage de la structure de débuggage
    if (debugInfo != NULL)
    {
        (*debugInfo).total_memory_allocated = total_memory_allocated;
        MPI_Allreduce(&((*M_COO).nnz), &((*debugInfo).cpt_values), 1, MPI_LONG, MPI_SUM, MPI_COMM_WORLD); //somme MPI_SUM des nombres de valeurs dans les sous matrices dans cpt_values, nombre de valeurs non nulles global

        /* MatrixDebugInfo.nb_connections contient actuellement (dans chaque processus) le nombre de connexions faites LOCALEMENT par tout les neurones par colonne. */
        MPI_Allreduce(nb_connections_local_tmp, (*debugInfo).nb_connections, (*brain).dimension, MPI_LONG, MPI_SUM, MPI_COMM_WORLD); //somme MPI_SUM de tout les nb_non_zeros_local dans (*debugInfo).nb_connections
        free(nb_connections_local_tmp);
        /* MatrixDebugInfo.nb_connections contient maintenant (dans tout les processus) le nombre GLOBAL de connexions faites pour chaque neurone. */
    }
}

/** @} */ // end of generation
