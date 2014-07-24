/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the vector related definitions.
 *  - Block Name : riblib
 *  - Process Name : rib library
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : nnVector.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_nnVector_h)
#define _nnVector_h

#include "nnTypes.h"

/* struct for vector */
struct _vector 
{
  Uint32T active;		/* number of active slots */
  Uint32T alloced;		/* number of allocated slot */
  void **index;			/* index to data */
};
typedef struct _vector *VectorT;

#define VECTOR_MIN_SIZE 1

/* (Sometimes) usefull macros.  This macro convert index expression to
 array expression. */
/* Reference slot at given index, caller must ensure slot is active */
#define vectorSlot(V,I)  ((V)->index[(I)])
/* Number of active slots. 
 * Note that this differs from vector_count() as it the count returned
 * will include any empty slots
 */
#define vectorActive(V) ((V)->active)

/* Prototypes. */
extern VectorT vectorInit (Uint32T);
extern void vectorEnsure (VectorT, Uint32T);
extern int vectorEmptySlot (VectorT);
extern int vectorSet (VectorT, void *);
extern int vectorSetIndex (VectorT, Uint32T, void *);
extern void vectorUnset (VectorT, Uint32T);
extern Uint32T  vectorCount (VectorT);
extern void vectorOnlyWrapperFree (VectorT);
extern void vectorOnlyIndexFree (void *);
extern void vectorFree (VectorT);
extern VectorT vectorCopy (VectorT);

extern void *vectorLookup (VectorT, Uint32T );
extern void *vectorLookupEnsure (VectorT, Uint32T );

#endif /* _nnVector_h */
