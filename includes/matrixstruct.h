/* Structures permettant de stocker les matrices et les informations sur les matrices, en COO et CSR, et fonctions permettant de faire des opérations sur les matrices */
/* à compléter */
/*Nicolas HOCHART*/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>

/** @defgroup sparseMatrix sparse matrix
 * 
 *  @brief Utilities for sparse matrix and its distribution accross MPI procs
 * 
 *  This module provides
 *     1. a structure of sparse matrix format: COO
 *     2. a structure of sparse matrix format: CSR
 *     3. info of each local matrix within a distributed sparse matrix
 *     4. some functionalities
 *  @{
 */

//! structure of sparse matrix format: COO
/*!
    Structure representing a sparse matrix in <a href="https://en.wikipedia.org/wiki/Sparse_matrix">COO format</a>.

    This structure is desgined for a adjacency matrix, thus the values of all non-zero entries are of type **Integer**.
 */
struct coo
{
    int *Row; //!< \public a pointer storing the row indices of non-zeros elements
    int *Column; //!< \public a pointer storing the column indices of non-zeros elements
    int *Value; //!< \public a pointer storing the non-zeros elements
    long dim_l; //!< \public number of row of a sparse matrix
    long dim_c; //!< \public number of column of a sparse matrix
    long nnz; //!< \public number of non-zero elements in the sparse matrix
};
typedef struct coo coo; //!< coo type

//! structure of sparse matrix format: CSR
/*!
    Structure representing a sparse matrix in <a href="https://en.wikipedia.org/wiki/Sparse_matrix">CSR format</a>.

    This structure is desgined for a adjacency matrix, thus the values of all non-zero entries are of type **Integer**.
*/
struct csr
{
    int * Row; //!< \public a pointer storing the offsets of pointer of each row
    int * Column; //!< \public a pointer storing the column indices of non-zeros elements
    int * Value; //!< \public a pointer storing the non-zeros elements
    long dim_l; //!< \public number of row of a sparse matrix
    long dim_c;  //!< \public number of column of a sparse matrix
    long nnz;  //!< \public number of non-zero elements in the sparse matrix
};
typedef struct csr csr; //!< csr type

//! Info of each local matrix within a distributed sparse matrix
/*!
    For a distributed sparse matrix in 1D/2D MPI proc grid, this structure is created to store some basic distribution 
    of a sparse matrix on each MPI proc.

    Contains :
    Basic info :
    Block row index, Block column index, number of lines in the block, number of columns in the block, row start index (included), column start index (included), row end index (included), column end index (included)
*/
struct MatrixDist
{
    int indl; //!< \public index of row of MPI proc within 2D MPI grid which this matrix is located on
    int indc; //!< \public index of column of MPI proc within 2D MPI grid which this matrix is located on.
    long dim_l; //!< \public number of row of a sparse matrix located on current MPI proc
    long dim_c; //!< \public number of column of a sparse matrix located on current MPI proc
    long startRow; //!< \public global index of the starting row of a local matrix within the global matrix
    long startColumn; //!< \public global index of the starting column of a local matrix within the global matrix
    long endRow; //!< \public global index of the ending row of a local matrix within the global matrix
    long endColumn; //!< \public global index of the ending column of a local matrix within the global matrix
};
typedef struct MatrixDist MatrixDist; //!< MatrixDist type

//! Returns the value of indices `[indl,indc]` of a local sparse matrix in CSR format
/*!
 * @param[in] M_CSR a sparse matrix in CSR format to be displayed to be queried
 * @param[in] indl row index of the value we want (must be `< (*M_CSR).dim_l`)
 * @param[in] indc column index of the value we want (must be `< (*M_CSR).dim_c`)
 * @return Matrix value at the index `[indl,indc]`
 */
  int csrGetValue(long indl, long indc, csr * M_CSR)
  {
      /*
      Renvoie la valeur [indl,indc] de la matrice M_CSR stockée au format CSR.
      Le vecteur à l'adresse (*M_CSR).Value doit être un vecteur d'entiers.
      */
      int *Row,*Column,*Value;
      Row = (*M_CSR).Row; Column = (*M_CSR).Column; Value = (*M_CSR).Value;
      if (indl >= (*M_CSR).dim_l || indc >= (*M_CSR).dim_c)
      {
          perror("WARNING: inconsistent indices were provided in the csrGetValue() function\n");
          return -1;
      }
      long i;
      long nb_values = Row[indl+1] - Row[indl]; //nombre de valeurs dans la ligne
      for (i=Row[indl];i<Row[indl]+nb_values;i++)
      {
          if (Column[i] == indc) {return Value[i];}
      }
      return 0; //<=> on a parcouru la ligne et on a pas trouvé de valeur dans la colonne
  }

//! Display a local sparse matrix in CSR format
/*!
   Display the local sparse matrix in CSR format as a classic matrix (also prints the 0s), if the dimension of the matrix is not too large
 * @param[in] M : a sparse matrix in CSR format to be displayed
 * @param[in] max_dim : max matrix dimension (if the dimension of the matrix is above this value, it is not printed)
 */
  void csrDisplay(csr * M, int max_dim)
  {
      long i,j;

      if ((*M).dim_l <= max_dim && (*M).dim_c <=max_dim) //si l'une des dimensions de la matrice est supérieure à max_dim, on n'affiche pas la matrice
      {
          for (i=0;i<(*M).dim_l;i++)
          {
              for (j=0;j<(*M).dim_c;j++)
              {
                  printf("%i ", csrGetValue(i, j, M));
              }
              printf("\n");
          }
      }
      else
      {
          printf("Matrix too large for print..\n");
      }
  }

//! Function that initialize the basic information of each local sparse matrix based on the setup
/*!
   Returns a MatrixDist structure according to the parameters received. The information filled in is only the basic information
 * @param[in] rank : mpi process rank
 * @param[in] nb_blocks_row : number of processes (blocks) on rows (in the process grid)
 * @param[in] nb_blocks_column : number of processes (blocks) on columns (in the process grid)
 * @param[in] n : global matrix dimension
 * @return MBlock: structure containing basic information of the local matrix on each proc
 */
MatrixDist initMatrixDist(int rank, int nb_blocks_row, int nb_blocks_column, long n)
{
    struct MatrixDist MBlock;
    MBlock.indl = rank / nb_blocks_column; 
    MBlock.indc = rank % nb_blocks_column; 
    MBlock.dim_l = n/nb_blocks_row; 
    MBlock.dim_c = n/nb_blocks_column;
    MBlock.startRow = MBlock.indl*MBlock.dim_l;
    MBlock.endRow = (MBlock.indl+1)*MBlock.dim_l-1;
    MBlock.startColumn = MBlock.indc*MBlock.dim_c;
    MBlock.endColumn = (MBlock.indc+1)*MBlock.dim_c-1;
    return MBlock;
}

/** @} */ // end of sparseMatrix
