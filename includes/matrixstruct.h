/* Structures permettant de stocker les matrices et les informations sur les matrices, en COO et CSR, et fonctions permettant de faire des opérations sur les matrices */
/* à compléter */
/*Nicolas HOCHART*/

#define matrixstruct

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>

/*-------------------------------------------------------------------
--- Structures pour le stockage des matrices au format COO et CSR ---
-------------------------------------------------------------------*/

//! Structure representing a COO Matrix containing int values
/*!
    Structure representing a COO Matrix containing int values.
    Contains the Row (row indexes), Column (column indexes) et Value (values) vectors, as well as the number of rows and columns in the matrix, and the number of non-zeros.
 */
struct coo
{
    int * Row; //vecteur de taille len_values = "nombre d'éléments non nuls dans la matrice"
    int * Column; //vecteur de taille len_values = "nombre d'éléments non nuls dans la matrice"
    int * Value; //vecteur de taille len_values = "nombre d'éléments non nuls dans la matrice"
    long dim_l; //nombre de lignes
    long dim_c; //nombre de colonnes
    long len_values; //taille des vecteurs Row, Column et Value
};
typedef struct coo coo;

//! Structure representing a CSR Matrix containing int values
/*!
    Structure representing a CSR Matrix containing int values.
    Contains the Row (row start indexes in Column vector), Column (column indexes) et Value (values) vectors, as well as the number of rows and columns in the matrix, and the number of non-zeros.
*/
struct csr
{
    int * Row; //vecteur de taille "nombre de lignes + 1" (dim_l + 1)
    int * Column; //vecteur de taille len_values = "nombre d'éléments non nuls dans la matrice"
    int * Value; //vecteur de taille len_values = "nombre d'éléments non nuls dans la matrice"
    long dim_l; //nombre de lignes
    long dim_c; //nombre de colonnes
    long len_values;  //taille des vecteurs Column et Value
};
typedef struct csr csr;

/*-----------------------------------------------------------
--- Structures pour les blocks (sur les différents cores) ---
-----------------------------------------------------------*/

//! Structure containing information about the local matrix block (in the local process)
/*!
    Structure containing information about the local matrix block (in the local process)
    Contains :
    Basic info :
    Block row index, Block column index, number of lines in the block, number of columns in the block, row start index (included), column start index (included), row end index (included), column end index (included)
*/
struct MatrixBlock
{
    int indl; //Indice de ligne du block
    int indc; //Indice de colonne du block
    long dim_l; //nombre de lignes dans le block
    long dim_c; //nombre de colonnes dans le block
    long startRow; //Indice de départ en ligne (inclu)
    long startColumn; //Indice de départ en colonne (inclu)
    long endRow; //Indice de fin en ligne (inclu)
    long endColumn; //Indice de fin en colonne (inclu)
};
typedef struct MatrixBlock MatrixBlock;

//! Returns the value [indl,indc] of a int CSR matrix
/*!
   Returns the value at the index [indl,indc] of the int CSR matrix passed as parameter
 * @param[in] M_CSR {csr *} Int CSR matrix of which we want to know the value at the index [indl,indc]
 * @param[in] indl {long} row index of the value we want (must be < to (*M_CSR).dim_l)
 * @param[in] indc {long} column index of the value we want (must be < to (*M_CSR).dim_c)
 * @return {int} Matrix value at the index [indl,indc]
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

//! Prints a local Int CSR Matrix
/*!
   Prints the local Int CSR Matrix passed as parameter like a classic matrix (also prints the 0s), if the dimension of the matrix is not too large
 * @param[in] M {csr *} : Int CSR matrix, to be printed
 * @param[in] max_dim {int} : max matrix dimension (if the dimension of the matrix is above this value, it is not printed)
 */
  void csrDisplay(csr * M, int max_dim)
  {
      /*
      Imprime la matrice d'entiers stockée en format CSR passée en paramètre
      Si une des dimension de la matrice dépasse max_dim, le message "trop grande pour être affichée" est affiché
      */
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

//! Function that fills and returns a MatrixBlock structure with basic information according to the parameters received.
/*!
   Returns a MatrixBlock structure according to the parameters received. The information filled in is only the basic information
 * @param[in] rank {int} : mpi process rank
 * @param[in] nb_blocks_row {int} : number of processes (blocks) on rows (in the process grid)
 * @param[in] nb_blocks_column {int} : number of processes (blocks) on columns (in the process grid)
 * @param[in] n (long) : global (total) matrix dimension
 * @return MBlock {MatrixBlock} : structure containing basic information of the Matrix Block (process)
 */
MatrixBlock setMatrixBlockInfo(int rank, int nb_blocks_row, int nb_blocks_column, long n)
{
      /*
      Rempli une structure MatrixBlock en fonction des paramètres reçu, et la retourne
      Les informations remplies sont uniquement les informations basiques
      */
    struct MatrixBlock MBlock;
    MBlock.indl = rank / nb_blocks_column; //indice de ligne dans la grille 2D de processus
    MBlock.indc = rank % nb_blocks_column; //indice de colonne dans la grille 2D de processus
    MBlock.dim_l = n/nb_blocks_row; //nombre de lignes dans un block
    MBlock.dim_c = n/nb_blocks_column; //nombre de colonnes dans un block
    MBlock.startRow = MBlock.indl*MBlock.dim_l;
    MBlock.endRow = (MBlock.indl+1)*MBlock.dim_l-1;
    MBlock.startColumn = MBlock.indc*MBlock.dim_c;
    MBlock.endColumn = (MBlock.indc+1)*MBlock.dim_c-1;
    return MBlock;
}