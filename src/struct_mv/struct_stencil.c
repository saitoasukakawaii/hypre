/*BHEADER**********************************************************************
 * Copyright (c) 2008,  Lawrence Livermore National Security, LLC.
 * Produced at the Lawrence Livermore National Laboratory.
 * This file is part of HYPRE.  See file COPYRIGHT for details.
 *
 * HYPRE is free software; you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License (as published by the Free
 * Software Foundation) version 2.1 dated February 1999.
 *
 * $Revision$
 ***********************************************************************EHEADER*/

/******************************************************************************
 *
 * Constructors and destructors for stencil structure.
 *
 *****************************************************************************/

#include "_hypre_struct_mv.h"

/*--------------------------------------------------------------------------
 * hypre_StructStencilCreate
 *--------------------------------------------------------------------------*/

hypre_StructStencil *
hypre_StructStencilCreate( HYPRE_Int     dim,
                           HYPRE_Int     size,
                           hypre_Index  *shape )
{
   hypre_StructStencil   *stencil;
   HYPRE_Int *stencil_shape_h;
   
   stencil = hypre_TAlloc(hypre_StructStencil, 1);

   hypre_StructStencilShape(stencil)    = shape;
   hypre_StructStencilSize(stencil)     = size;
   hypre_StructStencilNDim(stencil)      = dim;
   hypre_StructStencilRefCount(stencil) = 1;

   stencil_shape_h = hypre_CTAlloc(HYPRE_Int, HYPRE_MAXDIM*size);
   hypre_StructStencilShapeDevice(stencil) = hypre_DeviceTAlloc(HYPRE_Int, HYPRE_MAXDIM*size);
   for (HYPRE_Int i = 0; i < size; i++)
   {
      for (HYPRE_Int j = 0;j < dim;j++)
      {
	 stencil_shape_h[i*HYPRE_MAXDIM+j] = hypre_IndexD(shape[i], j);
      }
   }
      
#if defined(HYPRE_MEMORY_GPU)   
      hypre_DataCopyToData(stencil_shape_h,hypre_StructStencilShapeDevice(stencil),HYPRE_Int,HYPRE_MAXDIM*size);
#else
      hypre_StructStencilShapeDevice(stencil) = stencil_shape_h;
#endif
   return stencil;
}

/*--------------------------------------------------------------------------
 * hypre_StructStencilRef
 *--------------------------------------------------------------------------*/

hypre_StructStencil *
hypre_StructStencilRef( hypre_StructStencil *stencil )
{
   hypre_StructStencilRefCount(stencil) ++;

   return stencil;
}

/*--------------------------------------------------------------------------
 * hypre_StructStencilDestroy
 *--------------------------------------------------------------------------*/

HYPRE_Int
hypre_StructStencilDestroy( hypre_StructStencil *stencil )
{
   if (stencil)
   {
      hypre_StructStencilRefCount(stencil) --;
      if (hypre_StructStencilRefCount(stencil) == 0)
      {
         hypre_TFree(hypre_StructStencilShape(stencil));
#if defined(HYPRE_MEMORY_GPU)
	 if (hypre_StructStencilShapeDevice(stencil))
	    hypre_DeviceTFree(hypre_StructStencilShapeDevice(stencil));
#else
	 if (hypre_StructStencilShapeDevice(stencil))
	   hypre_TFree(hypre_StructStencilShapeDevice(stencil));
#endif
	 hypre_TFree(stencil);
      }
   }

   return hypre_error_flag;
}

/*--------------------------------------------------------------------------
 * hypre_StructStencilElementRank
 *    Returns the rank of the `stencil_element' in `stencil'.
 *    If the element is not found, a -1 is returned.
 *--------------------------------------------------------------------------*/

HYPRE_Int
hypre_StructStencilElementRank( hypre_StructStencil *stencil,
                                hypre_Index          stencil_element )
{
   hypre_Index  *stencil_shape;
   HYPRE_Int     rank;
   HYPRE_Int     i, ndim;

   rank = -1;
   ndim = hypre_StructStencilNDim(stencil);
   stencil_shape = hypre_StructStencilShape(stencil);
   for (i = 0; i < hypre_StructStencilSize(stencil); i++)
   {
      if (hypre_IndexesEqual(stencil_shape[i], stencil_element, ndim))
      {
         rank = i;
         break;
      }
   }

   return rank;
}

/*--------------------------------------------------------------------------
 * hypre_StructStencilSymmetrize:
 *    Computes a new "symmetrized" stencil.
 *
 *    An integer array called `symm_elements' is also set up.  A non-negative
 *    value of `symm_elements[i]' indicates that the `i'th stencil element
 *    is a "symmetric element".  That is, this stencil element is the
 *    transpose element of an element that is not a "symmetric element".
 *--------------------------------------------------------------------------*/

HYPRE_Int
hypre_StructStencilSymmetrize( hypre_StructStencil  *stencil,
                               hypre_StructStencil **symm_stencil_ptr,
                               HYPRE_Int           **symm_elements_ptr )
{
   hypre_Index          *stencil_shape = hypre_StructStencilShape(stencil);
   HYPRE_Int             stencil_size  = hypre_StructStencilSize(stencil); 

   hypre_StructStencil  *symm_stencil;
   hypre_Index          *symm_stencil_shape;
   HYPRE_Int             symm_stencil_size;
   HYPRE_Int            *symm_elements;

   HYPRE_Int             no_symmetric_stencil_element, symmetric;
   HYPRE_Int             i, j, d, ndim;
                       
   /*------------------------------------------------------
    * Copy stencil elements into `symm_stencil_shape'
    *------------------------------------------------------*/

   ndim = hypre_StructStencilNDim(stencil);
   symm_stencil_shape = hypre_CTAlloc(hypre_Index, 2*stencil_size);
   for (i = 0; i < stencil_size; i++)
   {
      hypre_CopyIndex(stencil_shape[i], symm_stencil_shape[i]);
   }

   /*------------------------------------------------------
    * Create symmetric stencil elements and `symm_elements'
    *------------------------------------------------------*/

   symm_elements = hypre_CTAlloc(HYPRE_Int, 2*stencil_size);
   for (i = 0; i < 2*stencil_size; i++)
      symm_elements[i] = -1;

   symm_stencil_size = stencil_size;
   for (i = 0; i < stencil_size; i++)
   {
      if (symm_elements[i] < 0)
      {
         /* note: start at i to handle "center" element correctly */
         no_symmetric_stencil_element = 1;
         for (j = i; j < stencil_size; j++)
         {
            symmetric = 1;
            for (d = 0; d < ndim; d++)
            {
               if (hypre_IndexD(symm_stencil_shape[j], d) !=
                   -hypre_IndexD(symm_stencil_shape[i], d))
               {
                  symmetric = 0;
                  break;
               }
            }
            if (symmetric)
            {
               /* only "off-center" elements have symmetric entries */
               if (i != j)
               {
                  symm_elements[j] = i;
               }
               no_symmetric_stencil_element = 0;
            }
         }

         if (no_symmetric_stencil_element)
         {
            /* add symmetric stencil element to `symm_stencil' */
            for (d = 0; d < ndim; d++)
            {
               hypre_IndexD(symm_stencil_shape[symm_stencil_size], d) =
                  -hypre_IndexD(symm_stencil_shape[i], d);
            }
               
            symm_elements[symm_stencil_size] = i;
            symm_stencil_size++;
         }
      }
   }

   symm_stencil = hypre_StructStencilCreate(hypre_StructStencilNDim(stencil),
                                            symm_stencil_size,
                                            symm_stencil_shape);

   *symm_stencil_ptr  = symm_stencil;
   *symm_elements_ptr = symm_elements;

   return hypre_error_flag;
}

