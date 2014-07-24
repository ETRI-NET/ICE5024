/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the interface related definitions.
 *  - Block Name : riblib
 *  - Process Name : rib library
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : nnDistribute.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_nnDistribute_h)
#define _nnDistribute_h

#include "hash.h"
#include "nnCmdCmsh.h"
#include "nnIf.h"

/* Disctirubte list types. */
typedef enum eDistributeType
{
  DISTRIBUTE_IN,
  DISTRIBUTE_OUT,
  DISTRIBUTE_MAX
} eDistributeTypeT;

struct Distribute
{
  /* Name of the interface. */
  char *ifName;

  /* Filter name of `in' and `out' */
  char *filterName[DISTRIBUTE_MAX];

  /* prefix-list name of `in' and `out' */
  char *prefixName[DISTRIBUTE_MAX];
};
typedef struct Distribute DistributeT;

/* Prototypes for distribute-list. */
extern void distributeListInit (int);
extern struct hash * distributeListGetPtr ();
extern void distributeListVersionUpdate (Int32T, struct hash *);
extern void distributeListReset (void);
extern void distributeListAddHook (void (*) (DistributeT *));
extern void distributeListDeleteHook (void (*) (DistributeT *));
extern DistributeT *distributeLookup (const StringT);
extern int configWriteDistribute (struct cmsh *);
extern int configShowDistribute (struct cmsh *);

#endif /* _nnDistribute_h */
