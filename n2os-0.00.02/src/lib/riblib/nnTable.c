/**************************************************************************************************** 
*                      Electronics and Telecommunications Research Institute
* Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.
*           No part of this software shall be reproduced, stored in a retrieval system, or
*           transmitted by any means, electronic, mechanical, photocopying, recording,
*           or otherwise, without written permission from ETRI.
****************************************************************************************************/


/**   
 * @brief : This file defines the table related definitions.
 *  - Block Name : riblib
 *  - Process Name : rib library
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/12/17
 */

/**
 * @file : nnTable.c
 *
 * $Id: nnTable.c 1051 2014-03-21 08:17:41Z sckim007 $
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "nnDefines.h"
#include "nnPrefix.h"
#include "nnTable.h"
#include "nosLib.h"


/**************************************************************************
 *                        CONSTANTS / LITERALS / TYPES
 **************************************************************************/


void nnRouteNodeDelete (RouteNodeT *);
void nnRouteTableFree (RouteTableT *);

RouteTableT *
nnRouteTableInit (void)
{
  RouteTableT *pRt = NULL;

  pRt = NNMALLOC (MEM_ROUTE_TABLE, sizeof (RouteTableT));
  return pRt;
}

void
nnRouteTableFinish (RouteTableT *pRt)
{
  nnRouteTableFree (pRt);
}

/* Allocate new route node. */
static RouteNodeT *
nnRouteNodeNew (void)
{
  RouteNodeT *pNode = NULL;
  pNode = NNMALLOC (MEM_ROUTE_NODE, sizeof (RouteNodeT));
  return pNode;
}

/* Allocate new route node with prefix set. */
static RouteNodeT *
nnRouteNodeSet (RouteTableT *pTable, PrefixT *pPrefix)
{
  RouteNodeT *pNode = NULL;
  
  pNode = nnRouteNodeNew ();

  nnPrefixCopy (&pNode->p, pPrefix);
  pNode->pTable = pTable;

  return pNode;
}

/* Free route node. */
static void
nnRouteNodeFree (RouteNodeT *pNode)
{
  NNFREE (MEM_ROUTE_NODE, pNode);
}

/* Free route table. */
void
nnRouteTableFree (RouteTableT *pRt)
{
  RouteNodeT *pTmpNode = NULL;
  RouteNodeT *pNode = NULL;
 
  if (pRt == NULL)
    return;

  pNode = pRt->pTop;

  while (pNode)
  {
    if (pNode->linkLeft)
    {
      pNode = pNode->linkLeft;
      continue;
    }

    if (pNode->linkRight)
    {
      pNode = pNode->linkRight;
      continue;
    }

    pTmpNode = pNode;
    pNode = pNode->pParent;

    if (pNode != NULL)
    {
      if (pNode->linkLeft == pTmpNode)
        pNode->linkLeft = NULL;
      else
        pNode->linkRight = NULL;

      nnRouteNodeFree (pTmpNode);
    }
    else
    {
      nnRouteNodeFree (pTmpNode);
      break;
    }
  }
 
  NNFREE (MEM_ROUTE_TABLE, pRt);
  return;
}

/* Utility mask array. */
static const Uint8T maskbit[] =
{
  0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff
};

/* Common prefix route genaration. */
static void
nnRouteCommon (PrefixT *pNPrefix, PrefixT *pPrefix, PrefixT *pNew)
{
  Int32T i = 0;
  Uint8T diff = 0;
  Uint8T mask = 0;

  Uint8T *pNP = (u_char *)&pNPrefix->u.prefix;
  Uint8T *pPP = (u_char *)&pPrefix->u.prefix;
  Uint8T *pNEWP = (u_char *)&pNew->u.prefix;

  for (i = 0; i < pPrefix->prefixLen / 8; i++)
  {
    if (pNP[i] == pPP[i])
      pNEWP[i] = pNP[i];
    else
      break;
  }

  pNew->prefixLen = i * 8;

  if (pNew->prefixLen != pPrefix->prefixLen)
  {
    diff = pNP[i] ^ pPP[i];
    mask = 0x80;
    while (pNew->prefixLen < pPrefix->prefixLen && !(mask & diff))
    {
      mask >>= 1;
      pNew->prefixLen++;
    }
    pNEWP[i] = pNP[i] & maskbit[pNew->prefixLen % 8];
  }
}

static void
nnSetLink (RouteNodeT *pNode, RouteNodeT *pNew)
{
  pNode->pLink[nnGetPrefixBit ((u_char *)&pNew->p.u.prefix, &pNode->p.prefixLen)] = pNew;
  pNew->pParent = pNode;
}

/* Lock node. */
RouteNodeT *
nnRouteNodeLock (RouteNodeT *pNode)
{
  pNode->lock++;
  return pNode;
}

/* Unlock node. */
void
nnRouteNodeUnlock (RouteNodeT *pNode)
{
  pNode->lock--;

  if (pNode->lock == 0)
    nnRouteNodeDelete (pNode);
}

/* Find matched prefix. */
RouteNodeT *
nnRouteNodeMatch (const RouteTableT *pTable, const PrefixT *pPrefix)
{
  RouteNodeT *pNode = NULL;
  RouteNodeT *pMatched = NULL;

  pMatched = NULL;
  pNode = pTable->pTop;

  /* Walk down tree.  If there is matched route then store it to
     pMatched. */
  while (pNode && pNode->p.prefixLen <= pPrefix->prefixLen && 
         nnPrefixMatch (&pNode->p, pPrefix))
  {
    if (pNode->pInfo)
      pMatched = pNode;

    pNode = pNode->pLink[nnGetPrefixBit(&pPrefix->u.prefix, &pNode->p.prefixLen)];
  }

  /* If matched route found, return it. */
  if (pMatched)
    return nnRouteNodeLock (pMatched);

  return NULL;
}

RouteNodeT *
nnRouteNodeMatchIpv4 (const RouteTableT *pTable,
                       const struct in_addr *pAddr)
{
  Prefix4T p = {0,};

  memset (&p, 0, sizeof (Prefix4T));
  p.family = AF_INET;
  p.prefixLen = NN_IPV4_MAX_PREFIXLEN;
  p.prefix = *pAddr;

  return nnRouteNodeMatch (pTable, (PrefixT *) &p);
}

#ifdef HAVE_IPV6
RouteNodeT *
nnRouteNodeMatchIpv6 (const RouteTableT *pTable,
                       const struct in6_addr *pAddr)
{
  Prefix6T p = {0,};

  memset (&p, 0, sizeof (Prefix6T));
  p.family = AF_INET6;
  p.prefixLen = PREFIX_IPV6_MAX_PREFIXLEN;
  p.prefix = *pAddr;

  return nnRouteNodeMatch (pTable, (PrefixT *) &p);
}
#endif /* HAVE_IPV6 */

/* Lookup same prefix node.  Return NULL when we can't find route. */
RouteNodeT *
nnRouteNodeLookup (RouteTableT *pTable, PrefixT *pPrefix)
{
  RouteNodeT *pNode = NULL;

  pNode = pTable->pTop;

  while (pNode && pNode->p.prefixLen <= pPrefix->prefixLen && 
         nnPrefixMatch (&pNode->p, pPrefix))
  {
    if (pNode->p.prefixLen == pPrefix->prefixLen && pNode->pInfo)
      return nnRouteNodeLock (pNode);

    pNode = pNode->pLink[nnGetPrefixBit((u_char *)&pPrefix->u.prefix, 
                                     &pNode->p.prefixLen)];
  }

  return NULL;
}

/* Add node to routing table. */
RouteNodeT *
nnRouteNodeGet (RouteTableT *pTable, PrefixT *pPrefix)
{
  RouteNodeT *pNew = NULL;
  RouteNodeT *pNode = NULL;
  RouteNodeT *pMatch = NULL;

  pMatch = NULL;
  pNode = pTable->pTop;
  
  while (pNode && pNode->p.prefixLen <= pPrefix->prefixLen && 
         nnPrefixMatch (&pNode->p, pPrefix))
  {
    if (pNode->p.prefixLen == pPrefix->prefixLen)
    {
      nnRouteNodeLock (pNode);
      return pNode;
    }
    pMatch = pNode;
    pNode = pNode->pLink[nnGetPrefixBit((u_char *)&pPrefix->u.prefix, 
                                        &pNode->p.prefixLen)];
  }

  if (pNode == NULL)
  {
    pNew = nnRouteNodeSet (pTable, pPrefix);
    if (pMatch)
      nnSetLink (pMatch, pNew);
    else
      pTable->pTop = pNew;
  }
  else
  {
    pNew = nnRouteNodeNew ();
    nnRouteCommon (&pNode->p, pPrefix, &pNew->p);
    pNew->p.family = pPrefix->family;
    pNew->pTable = pTable;
    nnSetLink (pNew, pNode);

    if (pMatch)
      nnSetLink (pMatch, pNew);
    else
      pTable->pTop = pNew;

    if (pNew->p.prefixLen != pPrefix->prefixLen)
    {
      pMatch = pNew;
      pNew = nnRouteNodeSet (pTable, pPrefix);
      nnSetLink (pMatch, pNew);
    }
  }
  nnRouteNodeLock (pNew);
  
  return pNew;
}

/* Delete node from the routing table. */
void
nnRouteNodeDelete (RouteNodeT *pNode)
{
  RouteNodeT *pChild = NULL;
  RouteNodeT *pParent = NULL;

  assert (pNode->lock == 0);
  assert (pNode->pInfo == NULL);

  if (pNode->linkLeft && pNode->linkRight)
    return;

  if (pNode->linkLeft)
    pChild = pNode->linkLeft;
  else
    pChild = pNode->linkRight;

  pParent = pNode->pParent;

  if (pChild)
    pChild->pParent = pParent;

  if (pParent)
  {
    if (pParent->linkLeft == pNode)
      pParent->linkLeft = pChild;
    else
      pParent->linkRight = pChild;
  }
  else
    pNode->pTable->pTop = pChild;

  nnRouteNodeFree (pNode);

  /* If parent node is stub then delete it also. */
  if (pParent && pParent->lock == 0)
    nnRouteNodeDelete (pParent);
}

/* Get fist node and lock it.  This function is useful when one want
   to lookup all the node exist in the routing table. */
RouteNodeT *
nnRouteTop (RouteTableT *pTable)
{
  /* If there is no node in the routing table return NULL. */
  if (pTable->pTop == NULL)
    return NULL;

  /* Lock the top node and return it. */
  nnRouteNodeLock (pTable->pTop);
  return pTable->pTop;
}

/* Unlock current node and lock next node then return it. */
RouteNodeT *
nnRouteNext (RouteNodeT *pNode)
{
  RouteNodeT *pNext = NULL;
  RouteNodeT *pStart = NULL;

  /* Node may be deleted from nnRouteNodeUnlock so we have to preserve
     next node's pointer. */

  if (pNode->linkLeft)
  {
    pNext = pNode->linkLeft;
    nnRouteNodeLock (pNext);
    nnRouteNodeUnlock (pNode);
    return pNext;
  }
  if (pNode->linkRight)
  {
    pNext = pNode->linkRight;
    nnRouteNodeLock (pNext);
    nnRouteNodeUnlock (pNode);
    return pNext;
  }

  pStart = pNode;
  while (pNode->pParent)
  {
    if (pNode->pParent->linkLeft == pNode && pNode->pParent->linkRight)
    {
      pNext = pNode->pParent->linkRight;
      nnRouteNodeLock (pNext);
      nnRouteNodeUnlock (pStart);
      return pNext;
    }
    pNode = pNode->pParent;
  }
  nnRouteNodeUnlock (pStart);
  return NULL;
}

/* Unlock current node and lock next node until limit. */
RouteNodeT *
nnRouteNextUntil (RouteNodeT *pNode, RouteNodeT *pLimit)
{
  RouteNodeT *pNext = NULL;
  RouteNodeT *pStart = NULL;

  /* Node may be deleted from nnRouteNodeUnlock so we have to preserve
     next node's pointer. */

  if (pNode->linkLeft)
  {
    pNext = pNode->linkLeft;
    nnRouteNodeLock (pNext);
    nnRouteNodeUnlock (pNode);
    return pNext;
  }
  if (pNode->linkRight)
  {
    pNext = pNode->linkRight;
    nnRouteNodeLock (pNext);
    nnRouteNodeUnlock (pNode);
    return pNext;
  }

  pStart = pNode;
  while (pNode->pParent && pNode != pLimit)
  {
    if (pNode->pParent->linkLeft == pNode && pNode->pParent->linkRight)
    {
      pNext = pNode->pParent->linkRight;
      nnRouteNodeLock (pNext);
      nnRouteNodeUnlock (pStart);
      return pNext;
    }
    pNode = pNode->pParent;
  }
  nnRouteNodeUnlock (pStart);
  return NULL;
}
