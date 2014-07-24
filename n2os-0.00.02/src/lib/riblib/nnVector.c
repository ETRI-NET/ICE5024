/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : 각 컴포넌트에서 사용하는 공통 vector 관련 기능을 제공한다.
 * - Block Name : riblib
 * - Process Name : rib library
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : nnVector.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nnVector.h"
#include "nosLib.h"

/**
 * Description : Initialize vector : allocate memory and return vector. 
 *
 * @param [in] size : wanted vector number
 *
 * @retval : size of allocated vector
 */
VectorT
vectorInit (Uint32T  size)
{
  VectorT v = NNMALLOC (MEM_VECTOR, sizeof (struct _vector));

  /* allocate at least one slot */
  if (size == 0)
    size = 1;

  v->alloced = size;
  v->active = 0;
  v->index = NNMALLOC (MEM_VECTOR_INDEX, sizeof (void *) * size);
  return v;
}

void
vectorOnlyWrapperFree (VectorT v)
{
  NNFREE (MEM_VECTOR, v);
}

void
vectorOnlyIndexFree (void *index)
{
  NNFREE (MEM_VECTOR_INDEX, index);
}

void
vectorFree (VectorT v)
{
  NNFREE (MEM_VECTOR_INDEX, v->index);
  NNFREE (MEM_VECTOR, v);
}

VectorT
vectorCopy (VectorT v)
{
  Uint32T  size = 0;
  VectorT new = NNMALLOC (MEM_VECTOR, sizeof (struct _vector));

  new->active = v->active;
  new->alloced = v->alloced;

  size = sizeof (void *) * (v->alloced);
  new->index = NNMALLOC (MEM_VECTOR_INDEX, size);
  memcpy (new->index, v->index, size);

  return new;
}

/**
 * Description : Check assigned index, and if it runs short double index pointer
 *
 * @param [in] v : vector pointer
 * @param [in] num : number of vector
 *
 * @retval : none
 */
void
vectorEnsure (VectorT v, Uint32T  num)
{
  if (v->alloced > num)
    return;

  v->index = realloc (v->index, sizeof (void *) * (v->alloced * 2));
  memset (&v->index[v->alloced], 0, sizeof (void *) * v->alloced);
  v->alloced *= 2;
  
  if (v->alloced <= num)
    vectorEnsure (v, num);
}

/**
 * Description : This function only returns next empty slot index.
 * It dose not mean the slot's index memory is assigned,  call
 * vector_ensure() after calling this function.
 *
 * @param [in] v : vector pointer
 *
 * @retval : index of vector
 */
int
vectorEmptySlot (VectorT v)
{
  Uint32T  i = 0;

  if (v->active == 0)
    return 0;

  for (i = 0; i < v->active; i++)
    if (v->index[i] == 0)
      return i;

  return i;
}

/**
 * Description : Set value to the smallest empty slot.
 *
 * @param [in] v : vector pointer
 * @param [in] pVal : pointer of value that will be copied
 *
 * @retval : index of vector
 */
int
vectorSet (VectorT v, void *pVal)
{
  Uint32T  i = 0;

  i = vectorEmptySlot (v);
  vectorEnsure (v, i);

  v->index[i] = pVal;

  if (v->active <= i)
    v->active = i + 1;

  return i;
}

/**
 * Description : Set value to specified index slot.
 *
 * @param [in] v : vector pointer
 * @param [in] i : specified index slot
 * @param [in] pVal : pointer of value that will be copied
 *
 * @retval : index of vector
 */
int
vectorSetIndex (VectorT v, Uint32T  i, void *pVal)
{
  vectorEnsure (v, i);

  v->index[i] = pVal;

  if (v->active <= i)
    v->active = i + 1;

  return i;
}

/**
 * Description : Look up vector.
 *
 * @param [in] v : vector pointer
 * @param [in] i : specified index slot
 *
 * @retval : value of specified index
 */
void *
vectorLookup (VectorT v, Uint32T  i)
{
  if (i >= v->active)
    return NULL;
  return v->index[i];
}

/**
 * Description : Lookup vector, ensure it.
 *
 * @param [in] v : vector pointer
 * @param [in] i : specified index slot
 *
 * @retval : value of specified index
 */
void *
vectorLookupEnsure (VectorT v, Uint32T  i)
{
  vectorEnsure (v, i);
  return v->index[i];
}

/**
 * Description : Unset value at specified index slot.
 *
 * @param [in] v : vector pointer
 * @param [in] i : specified index slot
 *
 * @retval : none
 */
void
vectorUnset (VectorT v, Uint32T  i)
{
  if (i >= v->alloced)
    return;

  v->index[i] = NULL;

  if (i + 1 == v->active) 
    {
      v->active--;
      while (i && v->index[--i] == NULL && v->active--) 
	;				/* Is this ugly ? */
    }
}

/**
 * Description : Count the number of not emplty slot.
 *
 * @param [in] v : vector pointer
 *
 * @retval : count of specified vector
 */
Uint32T 
vectorCount (VectorT v)
{
  Uint32T  i;
  Uint32T  count = 0;

  for (i = 0; i < v->active; i++) 
    if (v->index[i] != NULL)
      count++;

  return count;
}
