/**************************************************************************************************** 
*                      Electronics and Telecommunications Research Institute
* Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.
*           No part of this software shall be reproduced, stored in a retrieval system, or
*           transmitted by any means, electronic, mechanical, photocopying, recording,
*           or otherwise, without written permission from ETRI.
****************************************************************************************************/


/**   
 * @brief : This file defines the table related definitions.
 *  - Block Name : Library
 *  - Process Name : rLibraryibmgr
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/12/17
 */

/**
 * @file : nnTable.h
 *
 * $Id: nnTable.h 807 2014-02-05 07:24:51Z sckim007 $
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_nnTable_h)
#define _nnTable_h

#include "nnTypes.h"
#include "nnPrefix.h"

/*
 * CONSTANTS / LITERALS / TYPES
 */

/* Routing table top structure. */
struct RouteTable
{
  struct RouteNode *pTop;
};
typedef struct RouteTable RouteTableT;

/* Each routing entry. */
struct RouteNode
{
  /* Actual prefix of this radix. */
  PrefixT      p;

  /* Tree link. */
  RouteTableT      *pTable;
  struct RouteNode *pParent;
  struct RouteNode *pLink[2];
#define linkLeft   pLink[0]
#define linkRight  pLink[1]

  /* Lock of this radix */
  unsigned int lock;

  /* Each node of route. */
  void        *pInfo;

  /* Aggregation. */
  void        *pAggregate;
}RouteNode;
typedef struct RouteNode RouteNodeT;

/* Prototypes. */
extern RouteTableT *nnRouteTableInit (void);
extern void nnRouteTableFinish (RouteTableT *);
extern void nnRouteNodeUnlock (RouteNodeT *);
extern void nnRouteNodeDelete (RouteNodeT *);
extern RouteNodeT *nnRouteTop (RouteTableT *);
extern RouteNodeT *nnRouteNext (RouteNodeT *);
extern RouteNodeT *nnRouteNextUntil (RouteNodeT *, RouteNodeT *);
extern RouteNodeT *nnRouteNodeGet (RouteTableT *, PrefixT *);
extern RouteNodeT *nnRouteNodeLookup (RouteTableT *, PrefixT *);
extern RouteNodeT *nnRouteNodeLock (RouteNodeT *);
extern RouteNodeT *nnRouteNodeMatch (const RouteTableT *, const PrefixT *);
extern RouteNodeT *nnRouteNodeMatchIpv4 (const RouteTableT *, const struct in_addr *);
#ifdef HAVE_IPV6
extern RouteNodeT *nnRouteNodeMatchIpv6 (const RouteTableT *, const struct in6_addr *);
#endif /* HAVE_IPV6 */

#endif /* _NN_TABLE_H */
