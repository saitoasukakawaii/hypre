/*BHEADER**********************************************************************
 * (c) 1997   The Regents of the University of California
 *
 * See the file COPYRIGHT_and_DISCLAIMER for a complete copyright
 * notice, contact person, and disclaimer.
 *
 * $Revision$
 *********************************************************************EHEADER*/
/******************************************************************************
 *
 * Member functions for hypre_StructMatrix class.
 *
 *****************************************************************************/

#include "headers.h"

/*--------------------------------------------------------------------------
 * hypre_NewStructMatrixMask
 *    This routine returns the matrix, `mask', containing pointers to
 *    some of the data in the input matrix `matrix'.  This can be useful,
 *    for example, to construct "splittings" of a matrix for use in
 *    iterative methods.  The key note here is that the matrix `mask' does
 *    NOT contain a copy of the data in `matrix', but it can be used as
 *    if it were a normal StructMatrix object.
 *
 *    Notes:
 *    (1) Only the stencil, data_indices, and global_size components of the
 *        StructMatrix structure are modified.
 *    (2) PrintStructMatrix will not correctly print the stencil-to-data
 *        correspondence.
 *--------------------------------------------------------------------------*/

hypre_StructMatrix *
hypre_NewStructMatrixMask( hypre_StructMatrix *matrix,
                           int                 num_stencil_indices,
                           int                *stencil_indices     )
{
   hypre_StructMatrix   *mask;

   hypre_StructStencil  *stencil;
   hypre_Index          *stencil_shape;
   hypre_StructStencil  *mask_stencil;
   hypre_Index          *mask_stencil_shape;
   int                   mask_stencil_size;

   hypre_BoxArray       *data_space;
   int                 **data_indices;
   int                 **mask_data_indices;

   int                   i, j;

   mask = hypre_CTAlloc(hypre_StructMatrix, 1);

   hypre_StructMatrixComm(mask)         = hypre_StructMatrixComm(matrix);
   hypre_StructMatrixGrid(mask)         = hypre_StructMatrixGrid(matrix);
   hypre_StructMatrixUserStencil(mask)  =
      hypre_StructMatrixUserStencil(matrix);
   hypre_StructMatrixNumValues(mask)    = hypre_StructMatrixNumValues(matrix);
   hypre_StructMatrixDataSpace(mask)    = hypre_StructMatrixDataSpace(matrix);
   hypre_StructMatrixData(mask)         = hypre_StructMatrixData(matrix);
   hypre_StructMatrixDataSize(mask)     = hypre_StructMatrixDataSize(matrix);
   hypre_StructMatrixSymmetric(mask)    = hypre_StructMatrixSymmetric(matrix);
   hypre_StructMatrixSymmElements(mask) =
      hypre_StructMatrixSymmElements(matrix);
   for (i = 0; i < 6; i++)
      hypre_StructMatrixNumGhost(mask)[i] =
         hypre_StructMatrixNumGhost(matrix)[i];
   hypre_StructMatrixCommPkg(mask) = hypre_StructMatrixCommPkg(matrix);

   /* create mask_stencil */
   stencil       = hypre_StructMatrixStencil(matrix);
   stencil_shape = hypre_StructStencilShape(stencil);
   mask_stencil_size  = num_stencil_indices;
   mask_stencil_shape = hypre_CTAlloc(hypre_Index, num_stencil_indices);
   for (i = 0; i < num_stencil_indices; i++)
   {
      hypre_CopyIndex(stencil_shape[stencil_indices[i]],
                      mask_stencil_shape[i]);
   }
   mask_stencil = hypre_NewStructStencil(hypre_StructStencilDim(stencil),
                                         mask_stencil_size,
                                         mask_stencil_shape);

   /* create a new data_indices array */
   data_space   = hypre_StructMatrixDataSpace(matrix);
   data_indices = hypre_StructMatrixDataIndices(matrix);
   mask_data_indices = hypre_CTAlloc(int *, hypre_BoxArraySize(data_space));
   hypre_ForBoxI(i, data_space)
      {
         mask_data_indices[i] = hypre_TAlloc(int, num_stencil_indices);
         for (j = 0; j < num_stencil_indices; j++)
         {
            mask_data_indices[i][j] = data_indices[i][stencil_indices[j]];
         }
      }

   hypre_StructMatrixStencil(mask)     = mask_stencil;
   hypre_StructMatrixDataIndices(mask) = mask_data_indices;
   hypre_StructMatrixGlobalSize(mask) =
      hypre_StructGridGlobalSize(hypre_StructMatrixGrid(mask)) *
      mask_stencil_size;

   return mask;
}

/*--------------------------------------------------------------------------
 * hypre_FreeStructMatrixMask
 *--------------------------------------------------------------------------*/

int 
hypre_FreeStructMatrixMask( hypre_StructMatrix *mask )
{
   int  ierr;

   int  i;

   if (mask)
   {
      hypre_ForBoxI(i, hypre_StructMatrixDataSpace(mask))
         hypre_TFree(hypre_StructMatrixDataIndices(mask)[i]);
      hypre_TFree(hypre_StructMatrixDataIndices(mask));

      hypre_FreeStructStencil(hypre_StructMatrixStencil(mask));

      hypre_TFree(mask);
   }

   return ierr;
}

