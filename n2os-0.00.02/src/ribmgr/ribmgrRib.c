/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : RIB Manager에서 관리하는 모든 라우팅 정보를 관리하는 화일.
 *
 * - Block Name : RIB Manager
 * - Process Name : ribmgr
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : ribmgrRib.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ribmgrRib.h"
#include "ribmgrRt.h"
#include "ribmgrRedistribute.h"
#include "ribmgrInit.h"

#include "nnRibDefines.h"
#include "nnVector.h"
#include "nnTypes.h"
#include "nnIf.h"
#include "nnPrefix.h"
#include "nnTable.h"
#include "nnStr.h"
#include "nosLib.h"

void printRouteTable()
{
  RouteNodeT *pRNode = NULL;
  RouteTableT *pTable = NULL;
  NexthopT *pNextHop = NULL;
  RibT *pRib = NULL;

  NNLOG(LOG_DEBUG, "+--------------------------------------\n");
  NNLOG(LOG_DEBUG, "| Routing Table \n");
  NNLOG(LOG_DEBUG, "+--------------------------------------\n");

  pTable = vrfTable (AFI_IP, SAFI_UNICAST, 0);
  if(pTable == NULL)
  {
    NNLOG(LOG_DEBUG, "There is no Routing Table\n");
    return;
  }

  for (pRNode = nnRouteTop (pTable); 
       pRNode; 
       pRNode = nnRouteNext (pRNode))
  {
    if (pRNode->pInfo)
    {
      char buf[INET6_ADDRSTRLEN]={};
      if (pRNode->p.family == AF_INET)
      {
        inet_ntop (pRNode->p.family, &pRNode->p.u.prefix4, buf, INET6_ADDRSTRLEN);
      }
#ifdef HAVE_IPV6
      else if (pRNode->p.family == AF_INET6)
      {
        inet_ntop (pRNode->p.family, &pRNode->p.u.prefix6, buf, INET6_ADDRSTRLEN);
      }
#endif
      else
      {
        NNLOG(LOG_ERR, "Wrong family = %d\n", pRNode->p.family);
        continue;
      }

      char strAddr1[INTERFACE_NAMSIZ] ={0}, strAddr2[INTERFACE_NAMSIZ] = {0};

      // print prefix
      NNLOG(LOG_DEBUG, "%s/%d: queued pRNode %p \n", buf, 
            pRNode->p.prefixLen, pRNode);

      // print pRib
      for(pRib = pRNode->pInfo ; pRib ; pRib = pRib->next)
      {
        NNLOG (LOG_DEBUG,
          "\trefCount == %lu, uptime == %lu, type == %u, pTable == %d\n",
          pRib->refCount,
          (Uint32T) pRib->uptime,
          pRib->type,
          pRib->table
        );
        NNLOG (LOG_DEBUG,
          "\tmetric == %u, distance == %u, flags == %u, status == %u\n",
          pRib->metric,
          pRib->distance,
          pRib->flags,
          pRib->status
        );
        NNLOG (LOG_DEBUG,
          "\tnexthop_num == %u, nexthop_active_num == %u, nexthop_fib_num == %u\n",
          pRib->nexthopNum,
          pRib->nexthopActiveNum,
          pRib->nexthopFibNum
        );
        for (pNextHop = pRib->pNexthop; pNextHop; pNextHop = pNextHop->next)
        {
          if (pNextHop->type == NEXTHOP_TYPE_IFNAME)
          {
            nnStrlCpy (strAddr1, pNextHop->ifName, strlen(pNextHop->ifName));
            nnStrlCpy (strAddr2, pNextHop->ifName, strlen(pNextHop->ifName));
          }
          else
          {
            inet_ntop (AF_INET, &pNextHop->gate.ipv4.s_addr, strAddr1, INET_ADDRSTRLEN);
            inet_ntop (AF_INET, &pNextHop->rGate.ipv4.s_addr, strAddr2, INET_ADDRSTRLEN);
          }
          NNLOG (LOG_DEBUG,
            "\tNH %s (%s) with flags %s%s%s\n",
            strAddr1,
            strAddr2,
            (CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE) ? "ACTIVE " : ""),
            (CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB) ? "FIB " : ""),
            (CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_RECURSIVE) ? "RECURSIVE" : "")
          );
        }
      }
    }
  }
}


/* Each route type's string and default distance value. */
static const struct
{  
  Int32T key;
  Int32T distance;
} ROUTE_INFO[] =
{
  {RIB_ROUTE_TYPE_SYSTEM,    0},
  {RIB_ROUTE_TYPE_KERNEL,    0},
  {RIB_ROUTE_TYPE_CONNECT,   0},
  {RIB_ROUTE_TYPE_STATIC,    1},
  {RIB_ROUTE_TYPE_RIP,     120},
  {RIB_ROUTE_TYPE_RIPNG,   120},
  {RIB_ROUTE_TYPE_OSPF,    110},
  {RIB_ROUTE_TYPE_OSPF6,   110},
  {RIB_ROUTE_TYPE_ISIS,    115},
  {RIB_ROUTE_TYPE_BGP,      20  /* IBGP is 200. */}
};


/**
 * Description : RIB Manager 초기화 시점에서 VRF 테이블을 Vector로 관리
 * 하기 위하여 할당하는 함수.
 *
 * @param [in] name : VRF 테이블 이름
 *
 * @retval : VrfT * : VRF 테이블 엔트리 포인터
 */
static VrfT *
vrfAlloc (StringT name)
{
  VrfT *pVrf = NULL;

  pVrf = NNMALLOC (MEM_VRF, sizeof (VrfT));

  /* Put name.  */
  if (name)
  {
    pVrf->name = nnStrDup(name, MEM_VRF_NAME);
  }

  /* Allocate routing table and static table.  */
  pVrf->pDynamicTable[AFI_IP][SAFI_UNICAST]  = nnRouteTableInit ();
  pVrf->pDynamicTable[AFI_IP6][SAFI_UNICAST] = nnRouteTableInit ();
  pVrf->pStaticTable[AFI_IP][SAFI_UNICAST]   = nnRouteTableInit ();
  pVrf->pStaticTable[AFI_IP6][SAFI_UNICAST]  = nnRouteTableInit ();

  return pVrf;
}



/* Description : VRF 테이블에서 ID를 기반으로 VRF 엔트리를 찾는 함수. */
VrfT *
vrfLookup (Uint32T id)
{
  return vectorLookup (pRibmgr->pVrfVector, id);
}



/**
 * Description : VRF 테이블들을 초기화 하는 함수.
 */
static void
vrfInit (void)
{
  VrfT *pDefaultTable = NULL;

  /* Allocate VRF vector.  */
  pRibmgr->pVrfVector = vectorInit (1);

  /* Allocate default main table.  */
  pDefaultTable = vrfAlloc ("Default-IP-Routing-Table");

  /* Default table index must be 0.  */
  vectorSetIndex (pRibmgr->pVrfVector, 0, pDefaultTable);
}


/* Description : VRF 테이블에서 AFI, SAFI, ID를 기반으로 Route Table 포인터
 * 를 찾는 함수. */
RouteTableT *
vrfTable (afi_t afi, safi_t safi, Uint32T id)
{
  VrfT *pVrf = NULL;

  pVrf = vrfLookup (id);
  if (! pVrf)
  {
    return NULL;
  }

  return pVrf->pDynamicTable[afi][safi];
}


/* Description : VRF 테이블에서 AFI, SAFI, ID를 기반으로 Static Route Table
 * 포인터를 찾는 함수. */
RouteTableT *
vrfStaticTable (afi_t afi, safi_t safi, Uint32T id)
{
  VrfT *pVrf = NULL;

  pVrf = vrfLookup (id);
  if (! pVrf)
  {
    return NULL;
  }

  return pVrf->pStaticTable[afi][safi];
}

/**
 * Description : RIB의 nexthop list의 끝에 nexthop을 추가 하는 함수.
 *
 * @param [in] pRib : RibT *
 * @param [in] pNextHop : NexthopT *
 */
static void
nexthopAdd (RibT *pRib, NexthopT *pNextHop)
{
  NexthopT *pLast = NULL;

  for (pLast = pRib->pNexthop; pLast && pLast->next; pLast = pLast->next)
    ;
  if (pLast)
  {
    pLast->next = pNextHop;
  }
  else
  {
    pRib->pNexthop = pNextHop;
  }
  pNextHop->prev = pLast;

  pRib->nexthopNum++;
}


/**
 * Description : RIB의 nexthop list에서 nexthop을 삭제 하는 함수.
 *
 * @param [in] pRib : RibT *
 * @param [in] pNextHop : NexthopT *
 */
static void
nexthopDelete (RibT *pRib, NexthopT *pNextHop)
{
  if (pNextHop->next)
  {
    pNextHop->next->prev = pNextHop->prev;
  }

  if (pNextHop->prev)
  {
    pNextHop->prev->next = pNextHop->next;
  }
  else
  {
    pRib->pNexthop = pNextHop->next;
  }

  pRib->nexthopNum--;
}


/**
 * Description : RIB의 nexthop list에서 nexthop메모리를 해제 하는 함수.
 *
 * @param [in] pNextHop : NexthopT *
 */
static void
nexthopFree (NexthopT *pNextHop)
{
  if (pNextHop->ifName)
  {
    NNFREE (MEM_IF_NAME, pNextHop->ifName);
  }

  NNFREE (MEM_NEXTHOP, pNextHop);
}



/* Description : RIB의 nexthop 에 Interface Index를 할당하는 함수. */
NexthopT *
nexthopIfindexAdd (RibT *pRib, Uint32T ifIndex)
{
  NexthopT *pNextHop = NULL;

  pNextHop = NNMALLOC (MEM_NEXTHOP, sizeof (NexthopT));
  pNextHop->type = NEXTHOP_TYPE_IFINDEX;
  pNextHop->ifIndex = ifIndex;

  nexthopAdd (pRib, pNextHop);

  return pNextHop;
}


/*
 * Description : RIB의 IPv4 주소로 nexthop을 찾는 함수.
 *
 * param [in] pRib : RibT *
 * param [in] ipv4 : struct in_addr *
 *
 * retval : NexthopT * 
 */
NexthopT *
nexthopIpv4Lookup(RibT *pRib, struct in_addr *pIpv4)
{
  NexthopT * pLast = NULL;

  for(pLast = pRib->pNexthop;pLast;pLast = pLast->next)
  {
    if((pLast->type == NEXTHOP_TYPE_IPV4) &&
       (memcmp(&pLast->gate.ipv4, pIpv4, sizeof(struct in_addr)) == 0))
    {
       NNLOG(LOG_DEBUG, "nexthopIpv4Lookup : Find\n");
       return pLast;
    }
  }

  return NULL;
}

#if 0
NexthopT *
nexthopBlackholeLookup(RibT *rib)
{
  NexthopT * pLast;
  for(pLast = rib->nexthop;pLast;pLast = pLast->next)
  {
    if(pLast->type == NEXTHOP_TYPE_NULL0)
       (memcmp(&pLast->gate.ipv4, ipv4, sizeof(struct in_addr)) == 0))
    {
       NNLOG(LOG_DEBUG, "nexthopIpv4Lookup : Find\n");
       return pLast;
    }
  }
  NNLOG(LOG_DEBUG, "nexthopIpv4Lookup : Failure\n");
  return NULL;
}
#endif


/*
 * Description : RIB에서 인터페이스 이름으로 nexthop을 찾는 함수.
 *
 * param [in] pRib : RibT *
 * param [in] ifName : StringT
 *
 * retval : NexthopT * 
 */
NexthopT *
nexthopIfnameLookup(RibT *pRib, StringT ifName)
{
  NexthopT * pLast = NULL;

  for(pLast = pRib->pNexthop;pLast;pLast = pLast->next)
  {
    NNLOG(LOG_DEBUG, "nexthop ifName = %s, ifName = %s\n", pLast->ifName, ifName);
    if((pLast->type == NEXTHOP_TYPE_IFNAME) && 
       (strcmp(pLast->ifName, ifName) == 0))
    {
       NNLOG(LOG_DEBUG, "nexthopIfnameLookup : Find\n");
       return pLast;
    }
  }

  return NULL;
}

/* Description : RIB에서 nexthop에 인터페이스 이름을 할당하는  함수. */
NexthopT *
nexthopIfnameAdd (RibT *pRib, StringT ifName)
{
  NexthopT *pNextHop = NULL;

  pNextHop = NNMALLOC (MEM_NEXTHOP, sizeof (NexthopT));
  pNextHop->type = NEXTHOP_TYPE_IFNAME;
  pNextHop->ifName = nnStrDup (ifName, MEM_IF_NAME);

  nexthopAdd (pRib, pNextHop);

  return pNextHop;
}


/* Description : RIB에서 nexthop에 Ipv4 주소를 할당하는 함수. */
NexthopT *
nexthopIpv4Add (RibT *pRib, struct in_addr *pIpv4, struct in_addr *pSrc)
{
  NexthopT *pNextHop = NULL;

  pNextHop = NNMALLOC (MEM_NEXTHOP, sizeof (NexthopT));
  pNextHop->type = NEXTHOP_TYPE_IPV4;
  pNextHop->gate.ipv4 = *pIpv4;
  if (pSrc)
  {
    pNextHop->src.ipv4 = *pSrc;
  }

  nexthopAdd (pRib, pNextHop);

  return pNextHop;
}


/*
 * Description : RIB에서 nexthop에 Ipv4 주소를 할당하는 함수.
 *
 * param [in] pRib : RibT *
 * param [in] ipv4 : struct in_addr *
 * param [in] src : struct in_addr *
 * param [in] ifIndex : Uint32T
 *
 * retval : NexthopT * 
 */
static NexthopT *
nexthopIpv4IfindexAdd (RibT *pRib, struct in_addr *pIpv4, 
                       struct in_addr *pSrc, Uint32T ifIndex)
{
  NexthopT *pNextHop = NULL;
 
  pNextHop = NNMALLOC (MEM_NEXTHOP, sizeof (NexthopT));
  pNextHop->type = NEXTHOP_TYPE_IPV4_IFINDEX;
  pNextHop->gate.ipv4 = *pIpv4;
  if (pSrc)
  {
    pNextHop->src.ipv4 = *pSrc;
  }

  pNextHop->ifIndex = ifIndex;

  nexthopAdd (pRib, pNextHop);

  return pNextHop;
}

#ifdef HAVE_IPV6
/* Description : RIB에서 nexthop에 Ipv6 주소를 할당하는 함수. */
NexthopT *
nexthopIpv6Add (RibT *pRib, struct in6_addr *pIpv6)
{
  NexthopT *pNextHop = NULL;

  pNextHop = NNMALLOC (MEM_NEXTHOP, sizeof (NexthopT));
  pNextHop->type = NEXTHOP_TYPE_IPV6;
  pNextHop->gate.ipv6 = *pIpv6;

  nexthopAdd (pRib, pNextHop);

  return pNextHop;
}


/*
 * Description : RIB에서 nexthop에 Ipv6 주소와 인터페이스 이름을 할당하는 함수.
 *
 * param [in] pRib : RibT *
 * param [in] pRib : struct in6_addr *
 *
 * retval : NexthopT * 
 */
static NexthopT *
nexthopIpv6IfnameAdd (RibT *pRib, struct in6_addr *pIpv6, StringT ifName)
{
  NexthopT *pNextHop = NULL;

  pNextHop = NNMALLOC (MEM_NEXTHOP, sizeof (NexthopT));
  pNextHop->type = NEXTHOP_TYPE_IPV6_IFNAME;
  pNextHop->gate.ipv6 = *pIpv6;
  pNextHop->ifName = nnStrDup (ifName, MEM_IF_NAME);

  nexthopAdd (pRib, pNextHop);

  return pNextHop;
}


/*
 * Description : RIB에서 nexthop에 Ipv6 주소와 인터페이스 인덱스를 할당하는 함수.
 *
 * param [in] pRib : RibT *
 * param [in] pIpv6 : struct in6_addr *
 * param [in] ifIndex : Uint32T
 *
 * retval : NexthopT * 
 */
static NexthopT *
nexthopIpv6IfindexAdd (RibT *pRib, struct in6_addr *pIpv6, Uint32T ifIndex)
{
  NexthopT *pNextHop = NULL;

  pNextHop = NNMALLOC (MEM_NEXTHOP, sizeof (NexthopT));
  pNextHop->type = NEXTHOP_TYPE_IPV6_IFINDEX;
  pNextHop->gate.ipv6 = *pIpv6;
  pNextHop->ifIndex = ifIndex;

  nexthopAdd (pRib, pNextHop);

  return pNextHop;
}
#endif /* HAVE_IPV6 */


/* Description : RIB에서 nexthop에 Black Hole을 할당하는 함수. */
NexthopT *
nexthopBlackholeAdd (RibT *pRib)
{
  NexthopT *pNextHop = NULL;
 
  pNextHop = NNMALLOC (MEM_NEXTHOP, sizeof (NexthopT));
  pNextHop->type = NEXTHOP_TYPE_NULL0;
  SET_FLAG (pRib->flags, RIB_FLAG_NULL0);

  nexthopAdd (pRib, pNextHop);

  return pNextHop;
}

/*
 * Description : RIB에서 nexthop을 활성화 하는 함수.
 * If force flag is not set, do not modify falgs at all for uninstall
 * the route from FIB.
 *
 * param [in] pRib : RibT *
 * param [in] pNextHop : NexthopT *
 * param [in] set : Int32T
 * param [in] pTop : RouteNodeT *
 *
 * retval : 0 if Failure,
 *          1 if Success
 */
static Int32T
nexthopActiveIpv4 (RibT *pRib, NexthopT *pNextHop, Int32T set,
                   RouteNodeT *pTop)
{
  Prefix4T p = {0,};
  RouteTableT *pTable = NULL;
  RouteNodeT *pRNode = NULL;
  NexthopT *pNewHop = NULL;
  RibT *pMatch = NULL;

  if (pNextHop->type == NEXTHOP_TYPE_IPV4)
  {
    pNextHop->ifIndex = 0;
  }

  if (set)
  {
    UNSET_FLAG (pNextHop->flags, NEXTHOP_FLAG_RECURSIVE);
  }

  /* Make lookup prefix. */
  memset (&p, 0, sizeof (Prefix4T));
  p.family = AF_INET;
  p.prefixLen = PREFIX_IPV4_MAX_PREFIXLEN;
  p.prefix = pNextHop->gate.ipv4;

  /* Lookup table.  */
  pTable = vrfTable (AFI_IP, SAFI_UNICAST, 0);
  if (! pTable)
  {
    return 0;
  }

  pRNode = nnRouteNodeMatch (pTable, (PrefixT *) &p);
  while (pRNode)
  {
    NNLOG(LOG_DEBUG, "prefix=%s, prefixlen=%d\n", 
          inet_ntoa(pRNode->p.u.prefix4), pRNode->p.prefixLen);

    nnRouteNodeUnlock (pRNode);
      
    /* If lookup self prefix return immediately. */
    if (pRNode == pTop)
    {
      return 0;
    }

    /* Pick up selected route. */
    for (pMatch = pRNode->pInfo; pMatch; pMatch = pMatch->next)
    {
      if (CHECK_FLAG (pMatch->status, RIB_ENTRY_REMOVED))
      {
        continue;
      }

      if (CHECK_FLAG (pMatch->flags, RIB_FLAG_SELECTED))
      {
        break;
      }
    }

    /* If there is no selected route or matched route is EGP, go up
       tree. */
    if (! pMatch 
        || pMatch->type == RIB_ROUTE_TYPE_BGP)
    {
      do 
      {
        pRNode = pRNode->pParent;
      } while (pRNode && pRNode->pInfo == NULL);
 
      if (pRNode)
      {
        nnRouteNodeLock (pRNode);
      }
    }
    else
    {
      if (pMatch->type == RIB_ROUTE_TYPE_CONNECT)
      {
        /* Directly point connected route. */
        pNewHop = pMatch->pNexthop;
        if (pNewHop && pNextHop->type == NEXTHOP_TYPE_IPV4)
        {
          pNextHop->ifIndex = pNewHop->ifIndex;
        }
	      
        return 1;
      }
      else if (CHECK_FLAG (pRib->flags, RIB_FLAG_INTERNAL))
      {
        for (pNewHop = pMatch->pNexthop; pNewHop; pNewHop = pNewHop->next)
          if (CHECK_FLAG (pNewHop->flags, NEXTHOP_FLAG_FIB) && 
              ! CHECK_FLAG (pNewHop->flags, NEXTHOP_FLAG_RECURSIVE))
          {
            if (set)
            {
              SET_FLAG (pNextHop->flags, NEXTHOP_FLAG_RECURSIVE);
              pNextHop->rType = pNewHop->type;
              if (pNewHop->type == NEXTHOP_TYPE_IPV4 ||
                  pNewHop->type == NEXTHOP_TYPE_IPV4_IFINDEX)
              {
                pNextHop->rGate.ipv4 = pNewHop->gate.ipv4;
              }
              if (pNewHop->type == NEXTHOP_TYPE_IFINDEX
                  || pNewHop->type == NEXTHOP_TYPE_IFNAME
                  || pNewHop->type == NEXTHOP_TYPE_IPV4_IFINDEX)
              {
                pNextHop->rifIndex = pNewHop->ifIndex;
              }
            }
            return 1;
          }
        return 0;
      }
      else
      {
        return 0;
      }
    }
  }
  return 0;
}

#ifdef HAVE_IPV6
/*
 * Description : RIB에서 ipv6 nexthop을 활성화 하는 함수.
 * If force flag is not set, do not modify falgs at all for uninstall
 * the route from FIB.
 *
 * param [in] pRib : RibT *
 * param [in] pNextHop : NexthopT *
 * param [in] set : Int32T
 * param [in] top : RouteNodeT *
 *
 * retval : 0 if Failure,
 *          1 if Success
 */
static Int32T
nexthopActiveIpv6 (RibT *pRib, NexthopT *pNextHop, Int32T set,
                   RouteNodeT *pTop)
{
  Prefix6T p = {0,};
  RouteTableT *pTable = NULL;
  RouteNodeT *pRNode = NULL;
  NexthopT *pNewHop = NULL;
  RibT *pMatch = NULL;

  if (pNextHop->type == NEXTHOP_TYPE_IPV6)
  {
    pNextHop->ifIndex = 0;
  }

  if (set)
  {
    UNSET_FLAG (pNextHop->flags, NEXTHOP_FLAG_RECURSIVE);
  }

  /* Make lookup prefix. */
  memset (&p, 0, sizeof (Prefix6T));
  p.family = AF_INET6;
  p.prefixLen = PREFIX_IPV6_MAX_PREFIXLEN;
  p.prefix = pNextHop->gate.ipv6;

  /* Lookup table.  */
  pTable = vrfTable (AFI_IP6, SAFI_UNICAST, 0);
  if (! pTable)
  {
    return 0;
  }

  pRNode = nnRouteNodeMatch (pTable, (PrefixT *) &p);
  while (pRNode)
  {
    nnRouteNodeUnlock (pRNode);
      
    /* If lookup self prefix return immediately. */
    if (pRNode == pTop)
    {
      return 0;
    }

    /* Pick up selected route. */
    for (pMatch = pRNode->pInfo; pMatch; pMatch = pMatch->next)
    {
      if (CHECK_FLAG (pMatch->status, RIB_ENTRY_REMOVED))
      {
        continue;
      }
      if (CHECK_FLAG (pMatch->flags, RIB_FLAG_SELECTED))
      {
        break;
      }
    }

    /* If there is no selected route or matched route is EGP, go up
    tree. */
    if (! pMatch || pMatch->type == RIB_ROUTE_TYPE_BGP)
    {
      do 
      {
        pRNode = pRNode->pParent;
      } while (pRNode && pRNode->pInfo == NULL);

      if (pRNode)
      {
        nnRouteNodeLock (pRNode);
      }
	}
    else
    {
      if (pMatch->type == RIB_ROUTE_TYPE_CONNECT)
      {
        /* Directly point connected route. */
        pNewHop = pMatch->pNexthop;

        if (pNewHop && pNextHop->type == NEXTHOP_TYPE_IPV6)
        {
          pNextHop->ifIndex = pNewHop->ifIndex;
        }
	      
        return 1;
      }
      else if (CHECK_FLAG (pRib->flags, RIB_FLAG_INTERNAL))
      {
        for (pNewHop = pMatch->pNexthop; pNewHop; pNewHop = pNewHop->next)
          if (CHECK_FLAG (pNewHop->flags, NEXTHOP_FLAG_FIB) && 
             ! CHECK_FLAG (pNewHop->flags, NEXTHOP_FLAG_RECURSIVE))
          {
            if (set)
            {
              SET_FLAG (pNextHop->flags, NEXTHOP_FLAG_RECURSIVE);
              pNextHop->rType = pNewHop->type;
              if (pNewHop->type == NEXTHOP_TYPE_IPV6 || 
                  pNewHop->type == NEXTHOP_TYPE_IPV6_IFINDEX || 
                  pNewHop->type == NEXTHOP_TYPE_IPV6_IFNAME)
              {
                pNextHop->rGate.ipv6 = pNewHop->gate.ipv6;
              }
              if (pNewHop->type == NEXTHOP_TYPE_IFINDEX || 
                  pNewHop->type == NEXTHOP_TYPE_IFNAME || 
                  pNewHop->type == NEXTHOP_TYPE_IPV6_IFINDEX || 
                  pNewHop->type == NEXTHOP_TYPE_IPV6_IFNAME)
              {
                pNextHop->rifIndex = pNewHop->ifIndex;
              }
            }
            return 1;
          }
        return 0;
      }
      else
      {
        return 0;
      }
    }
  }
  return 0;
}
#endif /* HAVE_IPV6 */


/* Description : Route Table에서 요청된 주소에 맞는 rib를 찾는 함수. */
RibT *
ribMatchIpv4 (struct in_addr addr)
{
  Prefix4T prefix4 = {0,};
  RouteTableT *pTable = NULL;
  RouteNodeT *pRNode = NULL;
  NexthopT *pNewHop = NULL;
  RibT *pMatch = NULL;

  /* Lookup table.  */
  pTable = vrfTable (AFI_IP, SAFI_UNICAST, 0);
  if (! pTable)
  {
    return 0;
  }

  memset (&prefix4, 0, sizeof (Prefix4T));
  prefix4.family = AF_INET;
  prefix4.prefixLen = PREFIX_IPV4_MAX_PREFIXLEN;
  prefix4.prefix = addr;

  pRNode = nnRouteNodeMatch (pTable, (PrefixT *) &prefix4);

  while (pRNode)
  {
    nnRouteNodeUnlock (pRNode);
      
    /* Pick up selected route. */
    for (pMatch = pRNode->pInfo; pMatch; pMatch = pMatch->next)
    {
      if (CHECK_FLAG (pMatch->status, RIB_ENTRY_REMOVED))
      {
        continue;
      }
      if (CHECK_FLAG (pMatch->flags, RIB_FLAG_SELECTED))
      {
        break;
      }
    }

    /* If there is no selected route or matched route is EGP, go up
       tree. */
    if (! pMatch || pMatch->type == RIB_ROUTE_TYPE_BGP)
    {
      do 
      {
        pRNode = pRNode->pParent;
      } while (pRNode && pRNode->pInfo == NULL);

      if (pRNode)
      {
        nnRouteNodeLock (pRNode);
      }
    }
    else
    {
      if (pMatch->type == RIB_ROUTE_TYPE_CONNECT)
      {
        /* Directly point connected route. */
        return pMatch;
      }
      else
      {
        for (pNewHop = pMatch->pNexthop; pNewHop; pNewHop = pNewHop->next)
          if (CHECK_FLAG (pNewHop->flags, NEXTHOP_FLAG_FIB))
          {
            return pMatch;
          }
        return NULL;
      }
    }
  }
  return NULL;
}


/* Description : Route Table에서 Prefix4T에 맞는 rib를 찾는 함수. */
RibT *
ribLookupIpv4 (Prefix4T *pPrefix4)
{
  RouteTableT *pTable = NULL;
  RouteNodeT *pRNode = NULL;
  NexthopT *pNextHop = NULL;
  RibT *pMatch = NULL;

  /* Lookup table.  */
  pTable = vrfTable (AFI_IP, SAFI_UNICAST, 0);
  if (! pTable)
  {
    return 0;
  }

  pRNode = nnRouteNodeLookup (pTable, (PrefixT *) pPrefix4);

  /* No route for this prefix. */
  if (! pRNode)
  {
    return NULL;
  }

  /* Unlock node. */
  nnRouteNodeUnlock (pRNode);

  for (pMatch = pRNode->pInfo; pMatch; pMatch = pMatch->next)
  {
    if (CHECK_FLAG (pMatch->status, RIB_ENTRY_REMOVED))
    {
      continue;
    }
    if (CHECK_FLAG (pMatch->flags, RIB_FLAG_SELECTED))
    {
      break;
    }
  }

  if (! pMatch || pMatch->type == RIB_ROUTE_TYPE_BGP)
  {
    return NULL;
  }

  if (pMatch->type == RIB_ROUTE_TYPE_CONNECT)
  {
    return pMatch;
  }
  
  for (pNextHop = pMatch->pNexthop; pNextHop; pNextHop = pNextHop->next)
    if (CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB))
    {
      return pMatch;
    }

  return NULL;
}

/*
 * This clone function, unlike its original ribLookupIpv4(), checks
 * if specified IPv4 route record (prefix/mask -> gate) exists in
 * the whole RIB and has RIB_FLAG_SELECTED set.
 *
 * Return values:
 * -1: error
 * 0: exact match found
 * 1: a match was found with a different gate
 * 2: connected route found
 * 3: no matches found
 */
#if 0
Int32T
ribLookupIpv4Route (Prefix4T *p, union sockunion * qgate)
{
  RouteTableT *table;
  RouteNodeT *rn;
  RibT *match;
  NexthopT *nexthop;

  /* Lookup table.  */
  table = vrfTable (AFI_IP, SAFI_UNICAST, 0);
  if (! table)
    return RIBMGR_RIB_LOOKUP_ERROR;

  /* Scan the RIB table for exactly matching RIB entry. */
  rn = nnRouteNodeLookup (table, (PrefixT *) p);

  /* No route for this prefix. */
  if (! rn)
    return RIBMGR_RIB_NOTFOUND;

  /* Unlock node. */
  nnRouteNodeUnlock (rn);

  /* Find out if a "selected" RR for the discovered RIB entry exists ever. */
  for (match = rn->info; match; match = match->next)
    {
      if (CHECK_FLAG (match->status, RIB_ENTRY_REMOVED))
	continue;
      if (CHECK_FLAG (match->flags, RIB_FLAG_SELECTED))
	break;
    }

  /* None such found :( */
  if (!match)
    return RIBMGR_RIB_NOTFOUND;

  if (match->type == RIB_ROUTE_TYPE_CONNECT)
    return RIBMGR_RIB_FOUND_CONNECTED;
  
  /* Ok, we have a cood candidate, let's check it's nexthop list... */
  for (nexthop = match->nexthop; nexthop; nexthop = nexthop->next)
    if (CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB))
    {
      /* We are happy with either direct or recursive hexthop */
      if (nexthop->gate.ipv4.s_addr == qgate->sin.sin_addr.s_addr ||
          nexthop->rgate.ipv4.s_addr == qgate->sin.sin_addr.s_addr)
        return RIBMGR_RIB_FOUND_EXACT;
      else
      {
        char gate_buf[INET_ADDRSTRLEN], rgate_buf[INET_ADDRSTRLEN], qgate_buf[INET_ADDRSTRLEN];
        inet_ntop (AF_INET, &nexthop->gate.ipv4.s_addr, gate_buf, INET_ADDRSTRLEN);
        inet_ntop (AF_INET, &nexthop->rgate.ipv4.s_addr, rgate_buf, INET_ADDRSTRLEN);
        inet_ntop (AF_INET, &qgate->sin.sin_addr.s_addr, qgate_buf, INET_ADDRSTRLEN);
        NNLOG (LOG_DEBUG, "%s: qgate == %s, gate == %s, rgate == %s\n", __func__, qgate_buf, gate_buf, rgate_buf);
        return RIBMGR_RIB_FOUND_NOGATE;
      }
    }

  return RIBMGR_RIB_NOTFOUND;
}
#endif

#ifdef HAVE_IPV6
/* Description : Route Table에서 입력된 IPv6 주소에 맞는 rib를 찾는 함수. */
RibT *
ribMatchIpv6 (struct in6_addr *pAddr)
{
  Prefix6T p = {0,};
  RouteTableT *pTable = NULL;
  RouteNodeT *pRNode = NULL;
  NexthopT *pNewHop = NULL;
  RibT *pMatch = NULL;

  /* Lookup table.  */
  pTable = vrfTable (AFI_IP6, SAFI_UNICAST, 0);
  if (! pTable)
    return 0;

  memset (&p, 0, sizeof (Prefix6T));
  p.family = AF_INET6;
  p.prefixLen = PREFIX_IPV6_MAX_PREFIXLEN;
  PREFIX_IPV6_ADDR_COPY (&p.prefix, pAddr);

  pRNode = nnRouteNodeMatch (pTable, (PrefixT *) &p);

  while (pRNode)
  {
    nnRouteNodeUnlock (pRNode);
      
    /* Pick up selected route. */
    for (pMatch = pRNode->pInfo; pMatch; pMatch = pMatch->next)
    {
      if (CHECK_FLAG (pMatch->status, RIB_ENTRY_REMOVED))
      {
        continue;
      }
      if (CHECK_FLAG (pMatch->flags, RIB_FLAG_SELECTED))
      {
        break;
      }
    }

    /* If there is no selected route or matched route is EGP, go up
       tree. */
    if (! pMatch || pMatch->type == RIB_ROUTE_TYPE_BGP)
    {
      do 
      {
        pRNode = pRNode->pParent;
      } while (pRNode && pRNode->pInfo == NULL);

      if (pRNode)
      {
        nnRouteNodeLock (pRNode);
      }
    }
    else
    {
	  if (pMatch->type == RIB_ROUTE_TYPE_CONNECT)
      {
        /* Directly point connected route. */
        return pMatch;
      }
	  else
      {
        for (pNewHop = pMatch->pNexthop; pNewHop; pNewHop = pNewHop->next)
          if (CHECK_FLAG (pNewHop->flags, NEXTHOP_FLAG_FIB))
          {
            return pMatch;
          }
        return NULL;
      }
    }
  }
  return NULL;
}
#endif /* HAVE_IPV6 */



/*
 * Description : Route Table에서 입력된 IPv6 주소에 맞는 rib를 찾는 함수.
 * This function verifies reachability of one given nexthop, which can be
 * numbered or unnumbered, IPv4 or IPv6. The result is unconditionally stored
 * in nexthop->flags field. If the 4th parameter, 'set', is non-zero,
 * nexthop->ifindex will be updated appropriately as well.
 * An existing route map can turn (otherwise active) nexthop into inactive, but
 * not vice versa.
 * The return value is the final value of 'ACTIVE' flag.* 
 * 
 * param [in] pRNode : RouteNodeT *
 * param [in] rib : RibT *
 * param [in] pNextHop : NexthopT *
 * param [in] set : Int32T
 *
 * retval : Uint32T
 */
static Uint32T
nexthopActiveCheck (RouteNodeT *pRNode, RibT *pRib,
                    NexthopT *pNextHop, Int32T set)
{
  InterfaceT *pIf = NULL;
  //route_map_result_t ret = RMAP_MATCH; // blocked by sckim
  //Uint32T ret = 0; // changed by sckim
  //extern char *proto_rm[AFI_MAX][RIB_ROUTE_TYPE_MAX+1];
  //struct route_map *rmap;
  Int32T family = 0;

  switch (pNextHop->type)
  {
    case NEXTHOP_TYPE_IFINDEX:
      pIf = ifLookupByIndex (pNextHop->ifIndex);
      if (pIf && ifIsOperative(pIf))
      {
        SET_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE);
      }
      else
      {
        UNSET_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE);
      }
      break;
    case NEXTHOP_TYPE_IPV6_IFNAME:
      family = AFI_IP6;
    case NEXTHOP_TYPE_IFNAME:
      pIf = ifLookupByName (pNextHop->ifName);
      if (pIf && ifIsOperative(pIf))
      {
        if (set)
        {
          pNextHop->ifIndex = pIf->ifIndex;
        }
        SET_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE);
      }
      else
      {
        if (set)
        {
          pNextHop->ifIndex = 0;
        }
        UNSET_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE);
      }
      break;
    case NEXTHOP_TYPE_IPV4:
    case NEXTHOP_TYPE_IPV4_IFINDEX:
      family = AFI_IP;
      if (nexthopActiveIpv4 (pRib, pNextHop, set, pRNode))
      {
        SET_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE);
      }
      else
      {
        UNSET_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE);
      }
      break;
#ifdef HAVE_IPV6
    case NEXTHOP_TYPE_IPV6:
      family = AFI_IP6;
      if (nexthopActiveIpv6 (pRib, pNextHop, set, pRNode))
      {
        SET_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE);
      }
      else
      {
        UNSET_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE);
      }
      break;
    case NEXTHOP_TYPE_IPV6_IFINDEX:
      family = AFI_IP6;
      if (IN6_IS_ADDR_LINKLOCAL (&pNextHop->gate.ipv6))
      {
        pIf = ifLookupByIndex (pNextHop->ifIndex);
        if (pIf && ifIsOperative(pIf))
        {
          SET_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE);
        }
        else
        {
          UNSET_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE);
        }
      }
      else
      {
        if (nexthopActiveIpv6 (pRib, pNextHop, set, pRNode))
        {
          SET_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE);
        }
        else
        {
          UNSET_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE);
        }
      }
      break;
#endif /* HAVE_IPV6 */
    case NEXTHOP_TYPE_NULL0:
      SET_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE);
      break;
    default:
      NNLOG(LOG_ERR, "Wrong nexthop type =%d\n", pNextHop->type);
      break;
  }

  if (! CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE))
  {
    return 0;
  }

  if (RIB_SYSTEM_ROUTE(pRib) ||
      (family == AFI_IP && pRNode->p.family != AF_INET) ||
      (family == AFI_IP6 && pRNode->p.family != AF_INET6))
  {
    return CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE);
  }

#if 0 // blocked by sckim
  rmap = 0;
  if (pRib->type >= 0 && pRib->type < RIB_ROUTE_TYPE_MAX &&
        	proto_rm[family][pRib->type])
    rmap = route_map_lookup_by_name (proto_rm[family][pRib->type]);
  if (!rmap && proto_rm[family][RIB_ROUTE_TYPE_MAX])
    rmap = route_map_lookup_by_name (proto_rm[family][RIB_ROUTE_TYPE_MAX]);
  if (rmap) {
      ret = route_map_apply(rmap, &pRNode->p, RMAP_ZEBRA, pNextHop);
  }

  if (ret == RMAP_DENYMATCH)
    UNSET_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE);
#endif

  return CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE);
}


/*
 * Description : Active Route 정보를 갱신하는 함수.
 * Iterate over all nexthops of the given RIB entry and refresh their
 * ACTIVE flag. rib->nexthop_active_num is updated accordingly. If any
 * nexthop is found to toggle the ACTIVE flag, the whole rib structure
 * is flagged with RIB_FLAG_CHANGED. The 4th 'set' argument is
 * transparently passed to nexthopActiveCheck().
 * Return value is the new number of active nexthops.
 *
 * param [in] pRNode : RouteNodeT *
 * param [in] pRib : RibT *
 * param [in] set : Int32T
 *
 * retval : number of active route
 */
static Int32T
nexthopActiveUpdate (RouteNodeT *pRNode, RibT *pRib, Int32T set)
{
  NexthopT *pNextHop = NULL;
  Uint32T prevActive = 0, prevIndex = 0, newActive = 0;

  pRib->nexthopActiveNum = 0;
  UNSET_FLAG (pRib->flags, RIB_FLAG_CHANGED);

  for (pNextHop = pRib->pNexthop; pNextHop; pNextHop = pNextHop->next)
  {
    prevActive = CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE);
    prevIndex = pNextHop->ifIndex;
    if ((newActive = nexthopActiveCheck (pRNode, pRib, pNextHop, set)))
    {
      pRib->nexthopActiveNum++;
    }

    if (prevActive != newActive || prevIndex != pNextHop->ifIndex)
    {
      SET_FLAG (pRib->flags, RIB_FLAG_CHANGED);
    }
  }
  return pRib->nexthopActiveNum;
}

/*
 * Description : 선택한 RIB를 커널의 FIB로 설정하는 함수.
 *
 * param [in] pRNode : RouteNodeT *
 * param [in] pRib : RibT *
 */
static void
ribInstallKernel (RouteNodeT *pRNode, RibT *pRib)
{
  Int32T ret = 0, familyType = 0;
  NexthopT *pNextHop = NULL;

  familyType = PREFIX_FAMILY (&pRNode->p);

  switch (familyType)
  {
    case AF_INET:
      ret = kernelAddIpv4 (&pRNode->p, pRib);
      break;
#ifdef HAVE_IPV6
    case AF_INET6:
      ret = kernelAddIpv6 (&pRNode->p, pRib);
      break;
#endif /* HAVE_IPV6 */
    default :
      ret = -1;
      NNLOG(LOG_ERR, "Wrong family type = %d\n", familyType);
      break;
  }

  /* This condition is never met, if we are using rt_socket.c */
  if (ret < 0)
  {
    for (pNextHop = pRib->pNexthop; pNextHop; pNextHop = pNextHop->next)
	  UNSET_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB);
  }
}


/*
 * Description : 선택한 RIB를 커널의 FIB로 부터 삭제하는 함수.
 *
 * param [in] pRNode : RouteNodeT *
 * param [in] rib : RibT *
 *
 * retval : 0 if Failure
 *        > 0 if success
 */
static Int32T
ribUninstallKernel (RouteNodeT *pRNode, RibT *pRib)
{
  Int32T ret = 0, familyType = 0;;
  NexthopT *pNextHop = NULL;

  familyType = PREFIX_FAMILY (&pRNode->p); 
  switch (familyType)
  {
    case AF_INET:
      ret = kernelDeleteIpv4 (&pRNode->p, pRib);
      break;
#ifdef HAVE_IPV6
    case AF_INET6:
      ret = kernelDeleteIpv6 (&pRNode->p, pRib);
      break;
#endif /* HAVE_IPV6 */
    default :
      ret = -1;
      NNLOG(LOG_ERR, "Wrong family type = %d\n", familyType);
      break;
  }

  for (pNextHop = pRib->pNexthop; pNextHop; pNextHop = pNextHop->next)
    UNSET_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB);

  return ret;
}


/*
 * Description : 선택한 RIB를 커널의 FIB로 부터 삭제하는 함수.
 *
 * param [in] pRNode : RouteNodeT *
 * param [in] pRib : RibT *
 */
static void
ribUninstall (RouteNodeT *pRNode, RibT *pRib)
{
  if (CHECK_FLAG (pRib->flags, RIB_FLAG_SELECTED))
  {
    redistributeDelete (&pRNode->p, pRib);

    if (! RIB_SYSTEM_ROUTE (pRib))
    {
      ribUninstallKernel (pRNode, pRib);
    }

    UNSET_FLAG (pRib->flags, RIB_FLAG_SELECTED);
  }
}

static void ribUnlink (RouteNodeT *, RibT *);


/*
 * Description : Route Node 에 추가된 정보를 처리하는 함수.
 * RIB에 대한 Kernel로 설정/삭제 기능을 수행한다.
 *
 * param [in] pRNode : RouteNodeT *
 */
static void
ribProcess (RouteNodeT *pRNode)
{
  Int32T installed = 0;
  RibT *pRib = NULL;
  RibT *pNext = NULL;
  RibT *pFib = NULL;
  RibT *pSelect = NULL;
  RibT *pDel = NULL;
  NexthopT *pNextHop = NULL;
  char buf[INET6_ADDRSTRLEN] = {};
  
  assert (pRNode);
  
  if (pRNode->p.family == AF_INET)
  {
    inet_ntop (pRNode->p.family, &pRNode->p.u.prefix4, buf, INET6_ADDRSTRLEN);
  }
#ifdef HAVE_IPV6
  else if (pRNode->p.family == AF_INET6)
  {
    inet_ntop (pRNode->p.family, &pRNode->p.u.prefix6, buf, INET6_ADDRSTRLEN);
  }
#endif
  else
  {
    NNLOG(LOG_ERR, "Wrong family = %d. so return\n", pRNode->p.family);
    return;
  }

  for (pRib = pRNode->pInfo; pRib; pRib = pNext)
  {
    /* The next pointer is saved, because current pointer
     * may be passed to ribUnlink() in the middle of iteration.
     */
    pNext = pRib->next;
      
    /* Currently installed rib. */
    if (CHECK_FLAG (pRib->flags, RIB_FLAG_SELECTED))
    {
      assert (pFib == NULL);
      pFib = pRib;
    }
      
    /* Unlock removed routes, so they'll be freed, bar the FIB entry,
     * which we need to do do further work with below.
     */
    if (CHECK_FLAG (pRib->status, RIB_ENTRY_REMOVED))
    {
      if (pRib != pFib)
      {
        NNLOG (LOG_DEBUG, "%s: %s/%d: pRNode %p, removing pRib %p\n", __func__,
                  buf, pRNode->p.prefixLen, pRNode, pRib);

        ribUnlink (pRNode, pRib);
      }
      else
      {
        pDel = pRib;
      }
          
      continue;
    }
      
    /* Skip unreachable nexthop. */
    if (! nexthopActiveUpdate (pRNode, pRib, 0))
    {
      continue;
    }

    /* Infinit distance. */
    if (pRib->distance == DISTANCE_INFINITY)
    {
        continue;
    }

    /* Newly selected rib, the common case. */
    if (!pSelect)
    {
      pSelect = pRib;
      continue;
    }
      
    /* filter route selection in following order:
     * - connected beats other types
     * - lower distance beats higher
     * - lower metric beats higher for equal distance
     * - last, hence oldest, route wins tie break.
     */
      
    /* Connected routes. Pick the last connected
     * route of the set of lowest metric connected routes.
     */
    if (pRib->type == RIB_ROUTE_TYPE_CONNECT)
    {
      if (pSelect->type != RIB_ROUTE_TYPE_CONNECT || 
          pRib->metric <= pSelect->metric)
      {
        pSelect = pRib;
      }
      continue;
    }
    else if (pSelect->type == RIB_ROUTE_TYPE_CONNECT)
    {
      continue;
    }
    
    /* higher distance loses */
    if (pRib->distance > pSelect->distance)
    {
      continue;
    }
      
    /* lower wins */
    if (pRib->distance < pSelect->distance)
    {
      pSelect = pRib;
      continue;
    }
      
    /* metric tie-breaks equal distance */
    if (pRib->metric <= pSelect->metric)
    {
      pSelect = pRib;
    }
  } /* for (pRib = pRNode->info; pRib; pRib = pNext) */
  NNLOG(LOG_DEBUG, "pRib=0x%p, pSelect = 0x%p, pFib=0x%p\n", pRib, pSelect, pFib);


  /* After the cycle is finished, the following pointers will be set:
   * pSelect --- the winner RIB entry, if any was found, otherwise NULL
   * pFib    --- the SELECTED RIB entry, if any, otherwise NULL
   * pDel    --- equal to fib, if fib is queued for deletion, NULL otherwise
   * pRib    --- NULL
   */

  /* Same RIB entry is selected. Update FIB and finish. */
  if (pSelect && pSelect == pFib)
  {
    NNLOG (LOG_DEBUG, 
           "%s: %s/%d: Updating existing route, pSelect %p, pFib %p\n",
           __func__, buf, pRNode->p.prefixLen, pSelect, pFib);

    if (CHECK_FLAG (pSelect->flags, RIB_FLAG_CHANGED))
    {
      redistributeDelete (&pRNode->p, pSelect);

      if (! RIB_SYSTEM_ROUTE (pSelect))
      {
        ribUninstallKernel (pRNode, pSelect);
      }

      /* Set real nexthop. */
      nexthopActiveUpdate (pRNode, pSelect, 1);
  
      if (! RIB_SYSTEM_ROUTE (pSelect))
      {
        ribInstallKernel (pRNode, pSelect);
      }

      redistributeAdd (&pRNode->p, pSelect);
    }
    else if (! RIB_SYSTEM_ROUTE (pSelect))
    {
      /* Housekeeping code to deal with 
       * race conditions in kernel with linux
       * netlink reporting interface up before IPv4 or IPv6 protocol
       * is ready to add routes.
       * This makes sure the routes are IN the kernel.
       */

      for (pNextHop = pSelect->pNexthop; pNextHop; pNextHop = pNextHop->next)
        if (CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB))
        {
          installed = 1;
          break;
        }
      if (! installed) 
      {
        ribInstallKernel (pRNode, pSelect);
      }
    }
    goto end;
  }

  /* At this point we either haven't found the best RIB entry or it is
   * different from what we currently intend to flag with SELECTED. In both
   * cases, if a RIB block is present in FIB, it should be withdrawn.
   */
  if (pFib)
  {
    NNLOG (LOG_DEBUG, "%s: %s/%d: Removing existing route, pFib %p\n", 
           __func__, buf, pRNode->p.prefixLen, pFib);

    redistributeDelete (&pRNode->p, pFib);
    if (! RIB_SYSTEM_ROUTE (pFib))
    {
      ribUninstallKernel (pRNode, pFib);
    }
    UNSET_FLAG (pFib->flags, RIB_FLAG_SELECTED);

    /* Set real nexthop. */
    nexthopActiveUpdate (pRNode, pFib, 1);
  }

  /* Regardless of some RIB entry being SELECTED or not before, now we can
   * tell, that if a new winner exists, FIB is still not updated with this
   * data, but ready to be.
   */
  if (pSelect)
  {
    /* Set real nexthop. */
    nexthopActiveUpdate (pRNode, pSelect, 1);

    if (! RIB_SYSTEM_ROUTE (pSelect))
    {
      ribInstallKernel (pRNode, pSelect);
    }

    SET_FLAG (pSelect->flags, RIB_FLAG_SELECTED);
    redistributeAdd (&pRNode->p, pSelect);
  }

  /* FIB route was removed, should be deleted */
  if (pDel)
  {
    NNLOG (LOG_DEBUG, 
           "%s: %s/%d: Deleting pFib %p, pRNode %p\n", 
           __func__, buf, pRNode->p.prefixLen, pDel, pRNode);

    ribUnlink (pRNode, pDel);
  }

end:
  NNLOG (LOG_DEBUG, "%s: %s/%d: pRNode %p dequeued\n", __func__, buf, pRNode->p.prefixLen, pRNode);
}

/* RIB updates are processed via a queue of pointers to RouteNodeT.
 *
 * The queue length is bounded by the maximal size of the routing table,
 * as a RouteNodeT will not be requeued, if already queued.
 *
 * RIBs are submitted via ribAddNode or ribDeleteNode which set minimal
 * state, or staticInstallIpv{4,6} (when an existing RIB is updated)
 * and then submit RouteNodeT to queue for best-path selection later.
 * Order of add/delete state changes are preserved for any given RIB.
 *
 * Deleted RIBs are reaped during best-path selection.
 *
 * ribAddNode
 * |-> ribLink or unset RIB_ENTRY_REMOVE        |->Update kernel with
 *       |-------->|                             |  best RIB, if required
 *                 |                             |
 * staticInstall->|->rib_addqueue...... -> rib_process
 *                 |                             |
 *       |-------->|                             |-> rib_unlink
 * |-> set RIB_ENTRY_REMOVE                           |
 * rib_delnode                                  (RIB freed)
 *
 *
 * Queueing state for a RouteNodeT is kept in the head RIB entry, this
 * state must be preserved as and when the head RIB entry of a
 * RouteNodeT is changed by rib_unlink / rib_link. A small complication,
 * but saves having to allocate a dedicated object for this.
 * 
 * Refcounting (aka "locking" throughout the ribmgr):
 *
 * - RouteNodeTs: refcounted by:
 *   - RIBs attached to RouteNodeT:
 *     - managed by: rib_link/unlink
 *   - RouteNodeT processing queue
 *     - managed by: rib_addqueue, rib_process.
 *
 */


/*
 * Description : Route Node의 Head에 RIB를 추가하는 함수.
 *
 * param [in] pRNode : RouteNodeT *
 * param [in] pRib :  RibT *
 */
static void
ribLink (RouteNodeT *pRNode, RibT *pRib)
{
  RibT *head = NULL;
  char buf[INET6_ADDRSTRLEN]={};
  
  assert (pRib && pRNode);
  
  nnRouteNodeLock (pRNode); /* pRNode route table reference */

  NNLOG(LOG_DEBUG, "RouteNode family=%d, addr=%s, len=%d\n", 
        pRNode->p.family, inet_ntoa(pRNode->p.u.prefix4), pRNode->p.prefixLen);

  if (pRNode->p.family == AF_INET)
  {
    inet_ntop (pRNode->p.family, &pRNode->p.u.prefix4, buf, INET6_ADDRSTRLEN);
  }
#ifdef HAVE_IPV6
  else if (pRNode->p.family == AF_INET6)
  {
    inet_ntop (pRNode->p.family, &pRNode->p.u.prefix6, buf, INET6_ADDRSTRLEN);
  }
#endif
  else
  {
    NNLOG(LOG_ERR, "Wrong family = %d\n", pRNode->p.family);
    return;
  }
  
  NNLOG (LOG_DEBUG, "%s: %s/%d: pRNode %p, pRib %p\n", __func__,
          buf, pRNode->p.prefixLen, pRNode, pRib);

  head = pRNode->pInfo;
  if (head)
  {
    NNLOG (LOG_DEBUG, "%s: %s/%d: new head, rnStatus copied over\n", 
           __func__, buf, pRNode->p.prefixLen);

    head->prev = pRib;
    /* Transfer the rn status flags to the new head RIB */
    pRib->rnStatus = head->rnStatus;
  }
  pRib->next = head;
  pRNode->pInfo = pRib;

  // replace ribInstallKernel
  NNLOG(LOG_DEBUG, "Calling rib_queue_add ...%s, %s, %d\n", 
          __FILE__, __FUNCTION__, __LINE__); 
}


/*
 * Description : Route Node의 Head에 RIB를 추가하는 함수.
 *
 * param [in] pRNode : RouteNodeT *
 * param [in] pRib :  RibT *
 */
static void
ribAddNode (RouteNodeT *pRNode, RibT *pRib)
{
  NNLOG(LOG_DEBUG, "RouteNode family=%d, addr=%s, len=%d\n", 
        pRNode->p.family, inet_ntoa(pRNode->p.u.prefix4), pRNode->p.prefixLen);

  /* RIB node has been un-removed before route-node is processed. 
   * RouteNodeT must hence already be on the queue for processing.. 
   */
  if (CHECK_FLAG (pRib->status, RIB_ENTRY_REMOVED))
  {
    char buf[INET6_ADDRSTRLEN] = {};
    if (pRNode->p.family == AF_INET)
    {
      inet_ntop (pRNode->p.family, &pRNode->p.u.prefix4, buf, INET6_ADDRSTRLEN);
    }
#ifdef HAVE_IPV6
    else if (pRNode->p.family == AF_INET6)
    {
      inet_ntop (pRNode->p.family, &pRNode->p.u.prefix6, buf, INET6_ADDRSTRLEN);
    }
#endif
    else
    {
      NNLOG(LOG_ERR, "Wrong family = %d\n", pRNode->p.family);
      return;
    }

    NNLOG (LOG_DEBUG, "%s: %s/%d: pRNode %p, un-removed pRib %p\n",
           __func__, buf, pRNode->p.prefixLen, pRNode, pRib);

    UNSET_FLAG (pRib->status, RIB_ENTRY_REMOVED);

    return;
  }

  ribLink (pRNode, pRib);
}


/*
 * Description : Route Node의 Head에 RIB를 삭제하는 함수.
 *
 * param [in] pRNode : RouteNodeT *
 * param [in] pRib :  RibT *
 */
static void
ribUnlink (RouteNodeT *pRNode, RibT *pRib)
{
  NexthopT *pNextHop = NULL, *pNext = NULL;
  char buf[INET6_ADDRSTRLEN] = {};

  assert (pRNode && pRib);

  if (pRNode->p.family == AF_INET)
  {
    inet_ntop (pRNode->p.family, &pRNode->p.u.prefix4, buf, INET6_ADDRSTRLEN);
  }
#ifdef HAVE_IPV6
  else if (pRNode->p.family == AF_INET6)
  {
    inet_ntop (pRNode->p.family, &pRNode->p.u.prefix6, buf, INET6_ADDRSTRLEN);
  }
#endif
  else
  {
    NNLOG(LOG_ERR, "Wrong family = %d\n", pRNode->p.family);
    return;
  }

  NNLOG (LOG_DEBUG, "%s: %s/%d: pRNode %p, pRib %p\n", 
          __func__, buf, pRNode->p.prefixLen, pRNode, pRib);

  if (pRib->next)
  {
    pRib->next->prev = pRib->prev;
  }

  if (pRib->prev)
  {
    pRib->prev->next = pRib->next;
  }
  else
  {
    pRNode->pInfo = pRib->next;
      
    if (pRNode->pInfo)
    {
      NNLOG (LOG_DEBUG, "%s: %s/%d: pRNode %p, pRib %p, new head copy\n",
             __func__, buf, pRNode->p.prefixLen, pRNode, pRib);

      pRib->next->rnStatus = pRib->rnStatus;
    }
  }

  /* free RIB and nexthops */
  for (pNextHop = pRib->pNexthop; pNextHop; pNextHop = pNext)
  {
    pNext = pNextHop->next;
    nexthopFree (pNextHop);
  }
  NNFREE (MEM_NEXTHOP, pRib);

  nnRouteNodeUnlock (pRNode); /* rn route table reference */
}


/*
 * Description : Route Node의 Head에 RIB를 삭제하는 함수.
 *
 * param [in] pRNode : RouteNodeT *
 * param [in] pRib :  RibT *
 */
static void
ribDeleteNode (RouteNodeT *pRNode, RibT *pRib)
{
  char buf[INET6_ADDRSTRLEN] = {};

  if (pRNode->p.family == AF_INET)
  {
    inet_ntop (pRNode->p.family, &pRNode->p.u.prefix4, buf, INET6_ADDRSTRLEN);
  }
#ifdef HAVE_IPV6
  else if (pRNode->p.family == AF_INET6)
  {
    inet_ntop (pRNode->p.family, &pRNode->p.u.prefix6, buf, INET6_ADDRSTRLEN);
  }
#endif
  else
  {
    NNLOG(LOG_ERR, "Wrong family = %d\n", pRNode->p.family);
    return;
  }

  NNLOG (LOG_DEBUG, "%s/%d: pRNode %p, pRib %p, removing\n",
          buf, pRNode->p.prefixLen, pRNode, pRib);
  
  SET_FLAG (pRib->status, RIB_ENTRY_REMOVED);

  NNLOG(LOG_DEBUG, "## Calling ribUninstallKernel\n"); 
  ribUninstallKernel (pRNode, pRib);
}


/* Description : Ipv4 Route를 Route Table에 추가하는 함수. */
Int32T
ribAddIpv4 (Int32T type, Int32T flags, Prefix4T *pPrefix4, struct in_addr *pGate, 
            struct in_addr *pSrc, Uint32T ifIndex, Uint32T vrfId, Uint32T metric, 
            Uint8T distance)
{
  RibT *pRib = NULL;
  RibT *pSame = NULL;
  RouteTableT *pTable = NULL;
  RouteNodeT *pRNode = NULL;
  NexthopT *pNextHop = NULL;

  char buf[INET_ADDRSTRLEN] = {};
  char * routeType= NULL;

  if(type == RIB_ROUTE_TYPE_SYSTEM)
  {
    routeType = "RIB_ROUTE_TYPE_SYSTEM";
  }
  else if(type == RIB_ROUTE_TYPE_KERNEL)
  {
    routeType = "RIB_ROUTE_TYPE_KERNEL";
  }
  else if(type == RIB_ROUTE_TYPE_CONNECT)
  {
    routeType = "RIB_ROUTE_TYPE_CONNECT";
  }
  else
  {
    routeType = "RIB_ROUTE_TYPE_OTHER";
  }
 
  NNLOG(LOG_DEBUG, "CCCC route_type=%s prefix/length  = %s/%d\n", 
        routeType,
        inet_ntop(AF_INET, &pPrefix4->prefix, buf, INET_ADDRSTRLEN), 
        pPrefix4->prefixLen);

  /* Lookup table.  */
  pTable = vrfTable (AFI_IP, SAFI_UNICAST, 0);
  if (! pTable)
  {
    return 0;
  }

  /* Make it sure prefixLen is applied to the prefix. */
  nnApplyNetmasktoPrefix4 (pPrefix4, &pPrefix4->prefixLen);

  /* Set default distance by route type. */
  if (distance == 0)
  {
    distance = ROUTE_INFO[type].distance;

      /* iBGP distance is 200. */
    if (type == RIB_ROUTE_TYPE_BGP && CHECK_FLAG (flags, RIB_FLAG_IBGP))
    {
	  distance = 200;
    }
  }

  /* Convert Prefix4T to PrefixT.*/
  PrefixT prefix;
  if (nnCnvPrefix4TtoPrefixT (&prefix, pPrefix4) < 0)
  {
    NNLOG(LOG_ERR, "nnCnvPrefix4TtoPrefixT\n");
  }
  
  /* Lookup route node.*/
  pRNode = nnRouteNodeGet (pTable, &prefix);

  /* If same type of route are installed, treat it as a implicit
     withdraw. */
  for (pRib = pRNode->pInfo; pRib; pRib = pRib->next)
  {
    if (CHECK_FLAG (pRib->status, RIB_ENTRY_REMOVED))
    {
      continue;
    }
      
    if (pRib->type != type)
    {
	  continue;
    }

    if (pRib->type != RIB_ROUTE_TYPE_CONNECT)
    {
      pSame = pRib;
      break;
    }
    /* Duplicate connected route comes in. */
    else if ((pNextHop = pRib->pNexthop) &&
      	     (pNextHop->type == NEXTHOP_TYPE_IFINDEX) &&
	         (pNextHop->ifIndex == ifIndex) &&
	         !CHECK_FLAG (pRib->status, RIB_ENTRY_REMOVED))
	{
	  pRib->refCount++;
	  return 0 ;
	}
    else
    {
      NNLOG(LOG_WARNING, "Unknown case happened.\n");
    }
  }

  /* Allocate new rib structure. */
  pRib = NNMALLOC (MEM_ROUTE_RIB, sizeof (RibT));
  pRib->type = type;
  pRib->distance = distance;
  pRib->flags = flags;
  pRib->metric = metric;
  pRib->table = vrfId;
  pRib->nexthopNum = 0;
  pRib->uptime = time (NULL);

  /* Nexthop settings. */
  if (pGate)
  {
    if (ifIndex)
    {
	  nexthopIpv4IfindexAdd (pRib, pGate, pSrc, ifIndex);
    }
    else
    {
	  nexthopIpv4Add (pRib, pGate, pSrc);
    }
  }
  else
  {
    nexthopIfindexAdd (pRib, ifIndex);
  }

  /* If this route is kernel route, set FIB flag to the route. */
  if (type == RIB_ROUTE_TYPE_KERNEL || type == RIB_ROUTE_TYPE_CONNECT)
  {
    for (pNextHop = pRib->pNexthop; pNextHop; pNextHop = pNextHop->next)
      SET_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB);
  }

  /* Link new rib to node.*/
  NNLOG(LOG_DEBUG, "RouteNode family=%d, addr=%s, len=%d\n", 
        pRNode->p.family, inet_ntoa(pRNode->p.u.prefix4), pRNode->p.prefixLen);
  ribAddNode (pRNode, pRib);
  NNLOG(LOG_DEBUG, "RouteNode family=%d, addr=%s, len=%d\n", 
        pRNode->p.family, inet_ntoa(pRNode->p.u.prefix4), pRNode->p.prefixLen);

  /////////////////////////////////////////////////////////////////////////////
  // call this function to set pRib->flags to RIB_FLAG_SELECTED 
  /////////////////////////////////////////////////////////////////////////////
  ribProcess(pRNode); // add by sckim
  
  /* Free implicit route.*/
  if (pSame)
  {
    NNLOG (LOG_DEBUG, 
           "%s: calling ribDeleteNode (%p, %p)\n", __func__, pRNode, pRib);
    ribDeleteNode (pRNode, pSame);
  }
  
  nnRouteNodeUnlock (pRNode);

  // just print all of routing table
  printRouteTable();

  return 0;
}


/* Description : RIB 정보를 덤프하는 함수. */
void 
ribDump (const char * func, const Prefix4T * p, const RibT * pRib)
{
  char straddr1[INET_ADDRSTRLEN] = {}, straddr2[INET_ADDRSTRLEN] = {};
  NexthopT *pNextHop = NULL;

  inet_ntop (AF_INET, &p->prefix, straddr1, INET_ADDRSTRLEN);
  NNLOG (LOG_DEBUG, "%s: dumping RIB entry %p for %s/%d\n", 
         func, pRib, straddr1, p->prefixLen);
  NNLOG(LOG_DEBUG,
    "%s: refCount == %lu, uptime == %lu, type == %u, table == %d\n",
    func, pRib->refCount, (Uint32T) pRib->uptime, pRib->type, pRib->table);
  NNLOG (LOG_DEBUG,
    "%s: metric == %u, distance == %u, flags == %u, status == %u\n",
    func, pRib->metric, pRib->distance, pRib->flags, pRib->status);
  NNLOG (LOG_DEBUG,
    "%s: nexthop_num == %u, nexthop_active_num == %u, nexthop_fib_num == %u\n",
    func, pRib->nexthopNum, pRib->nexthopActiveNum, pRib->nexthopFibNum);

  for (pNextHop = pRib->pNexthop; pNextHop; pNextHop = pNextHop->next)
  {
    inet_ntop (AF_INET, &pNextHop->gate.ipv4.s_addr, straddr1, INET_ADDRSTRLEN);
    inet_ntop (AF_INET, &pNextHop->rGate.ipv4.s_addr, straddr2, INET_ADDRSTRLEN);
    NNLOG (LOG_DEBUG,
      "%s: NH %s (%s) with flags %s%s%s\n",
      func, straddr1, straddr2,
      (CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE) ? "ACTIVE " : ""),
      (CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB) ? "FIB " : ""),
      (CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_RECURSIVE) ? "RECURSIVE" : "")
    );
  }
  NNLOG (LOG_DEBUG, "%s: dump complete\n", func);
}


/* Description : Prefix4T의 주소에 맞는 RIB를 찾아 덤프하는함수. */
void 
ribLookupAndDump (Prefix4T * pPrefix4)
{
  RouteTableT *pTable = NULL;
  RouteNodeT *pRNode = NULL;
  RibT *pRib = NULL;
  char prefixBuf[INET_ADDRSTRLEN] = {};

  /* Lookup table.  */
  pTable = vrfTable (AFI_IP, SAFI_UNICAST, 0);
  if (! pTable)
  {
    NNLOG (LOG_DEBUG, "%s: vrfTable() returned NULL\n", __func__);
    return;
  }

  inet_ntop (AF_INET, &pPrefix4->prefix.s_addr, prefixBuf, INET_ADDRSTRLEN);
  /* Scan the RIB table for exactly matching RIB entry. */
  pRNode = nnRouteNodeLookup (pTable, (PrefixT *) pPrefix4);

  /* No route for this prefix. */
  if (! pRNode)
  {

    NNLOG (LOG_DEBUG, "%s: lookup failed for %s/%d \n", 
            __func__, prefixBuf, pPrefix4->prefixLen);

    return;
  }

  /* Unlock node. */
  nnRouteNodeUnlock (pRNode);

  /* let's go */
  for (pRib = pRNode->pInfo; pRib; pRib = pRib->next)
  {
    NNLOG (LOG_DEBUG, "%s: pRNode %p, pRib %p: %s, %s\n", __func__, pRNode, pRib,
      (CHECK_FLAG (pRib->status, RIB_ENTRY_REMOVED) ? "removed" : "NOT removed"),
      (CHECK_FLAG (pRib->flags, RIB_FLAG_SELECTED) ? "selected" : "NOT selected") );

    ribDump (__func__, pPrefix4, pRib);
  }
}

#if 0
/* Check if requested address assignment will fail due to another
 * route being installed by ribmgr in FIB already. Take necessary
 * actions, if needed: remove such a route from FIB and deSELECT
 * corresponding RIB entry. Then put affected RN into RIBQ head.
 */
void ribLookupAndPushup (Prefix4T * pPrefix4)
{
  RouteTableT *table;
  RouteNodeT *rn;
  RibT *rib;
  unsigned changed = 0;

  if (NULL == (table = vrfTable (AFI_IP, SAFI_UNICAST, 0)))
  {
    NNLOG (LOG_DEBUG, "%s: vrfTable() returned NULL \n", __func__);
    return;
  }

  /* No matches would be the simplest case. */
  if (NULL == (rn = nnRouteNodeLookup (table, (PrefixT *) p)))
    return;

  /* Unlock node. */
  nnRouteNodeUnlock (rn);

  /* Check all RIB entries. In case any changes have to be done, requeue
   * the RN into RIBQ head. If the routing message about the new connected
   * route (generated by the IP address we are going to assign very soon)
   * comes before the RIBQ is processed, the new RIB entry will join
   * RIBQ record already on head. This is necessary for proper revalidation
   * of the rest of the RIB.
   */
  for (rib = rn->info; rib; rib = rib->next)
  {
    if (CHECK_FLAG (rib->flags, RIB_FLAG_SELECTED) &&
      ! RIB_SYSTEM_ROUTE (rib))
    {
      changed = 1;

      char buf[INET_ADDRSTRLEN];
      inet_ntop (rn->p.family, &p->prefix, buf, INET_ADDRSTRLEN);
      NNLOG (LOG_DEBUG, "%s: freeing way for connected prefix %s/%d", 
              __func__, buf, p->prefixLen);
      ribDump (__func__, (Prefix4T *)&rn->p, rib);

      ribUninstall (rn, rib);
    }
  }

  if (changed)
  {
    // replace ribInstallKernel
    NNLOG(LOG_DEBUG, "## Calling ribQueueAdd ...%s, %s, %d\n", 
            __FILE__, __FUNCTION__, __LINE__); 
  }
}
#endif


/* Description : 주어진 RIB와 Prefix4T값을 기반으로 Kernel로 IPv4 Route를
 * 설정하는 함수. */
Int32T
ribAddIpv4Multipath (Prefix4T *pPrefix4, RibT *pRib)
{
  RouteTableT *pTable = NULL;
  RouteNodeT *pRNode = NULL;
  NexthopT *pNextHop = NULL;
  RibT *pSame = NULL;
  
  /* Lookup table.  */
  pTable = vrfTable (AFI_IP, SAFI_UNICAST, 0);
  if (! pTable)
  {
    return 0;
  }
  /* Make it sure prefixLen is applied to the prefix. */
  nnApplyNetmasktoPrefix4 (pPrefix4, &pPrefix4->prefixLen);

  /* Set default distance by route type. */
  if (pRib->distance == 0)
  {
    pRib->distance = ROUTE_INFO[pRib->type].distance;

    /* iBGP distance is 200. */
    if (pRib->type == RIB_ROUTE_TYPE_BGP && 
        CHECK_FLAG (pRib->flags, RIB_FLAG_IBGP))
    {
	  pRib->distance = 200;
    }
  }

  /* Lookup route node.*/
  pRNode = nnRouteNodeGet (pTable, (PrefixT *) pPrefix4);

  /* If same type of route are installed, treat it as a implicit
     withdraw. */
  for (pSame = pRNode->pInfo; pSame; pSame = pSame->next)
  {
    if (CHECK_FLAG (pSame->status, RIB_ENTRY_REMOVED))
    {
      continue;
    }
      
    if (pSame->type == pRib->type && pSame->table == pRib->table && 
        pSame->type != RIB_ROUTE_TYPE_CONNECT)
    {
      break;
    }
  }
  
  /* If this route is kernel route, set FIB flag to the route. */
  if (pRib->type == RIB_ROUTE_TYPE_KERNEL || pRib->type == RIB_ROUTE_TYPE_CONNECT)
  {
    for (pNextHop = pRib->pNexthop; pNextHop; pNextHop = pNextHop->next)
      SET_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB);
  }

  /* Free implicit route.*/
  if (pSame)
  {
    NNLOG (LOG_DEBUG, "calling ribDeleteNode (%p, %p) on existing RIB entry\n", 
           pRNode, pSame);
    ribDump (__func__, pPrefix4, pSame);

    ribDeleteNode (pRNode, pSame);
  }
 
  /* Link new rib to node.*/
  
  /* Link this rib to the tree. */
  ribAddNode (pRNode, pRib);

  /* Try RIB to kernel */
  ribProcess(pRNode);


  NNLOG (LOG_DEBUG, "called ribAddNode (%p, %p) on new RIB entry\n", pRNode, pRib);
  ribDump (__func__, pPrefix4, pRib);
 
  nnRouteNodeUnlock (pRNode);

  return 0;
}


/* Description : IPv4 RIB를 삭제하는 함수. */
Int32T
ribDeleteIpv4 (Int32T type, Int32T flags, Prefix4T *pPrefix4,
               struct in_addr *pGate, Uint32T ifIndex, Uint32T vrfId)
{
  RibT *pRib = NULL;
  RibT *pFib = NULL;
  RibT *pSame = NULL;
  NexthopT *pNextHop = NULL;
  RouteNodeT *pRNode = NULL;
  RouteTableT *pTable = NULL;
  char buf1[INET_ADDRSTRLEN] = {};
  char buf2[INET_ADDRSTRLEN] = {};

  /* Lookup table.  */
  pTable = vrfTable (AFI_IP, SAFI_UNICAST, 0);
  if (! pTable)
  {
    return 0;
  }

  /* Apply mask. */
  nnApplyNetmasktoPrefix4 (pPrefix4, &pPrefix4->prefixLen);

  if (pGate)
  {
    NNLOG (LOG_DEBUG, "ribDeleteIpv4(): route delete %s/%d via %s ifIndex %d\n",
            inet_ntop (AF_INET, &pPrefix4->prefix, buf1, INET_ADDRSTRLEN),
            pPrefix4->prefixLen, inet_ntoa (*pGate), ifIndex);
  }

  /* Lookup route node. */
  pRNode = nnRouteNodeLookup (pTable, (PrefixT *) pPrefix4);
  if (! pRNode)
  {
    if (pGate)
    {
      NNLOG (LOG_DEBUG, "route %s/%d via %s ifIndex %d doesn't exist in rib\n",
              inet_ntop (AF_INET, &pPrefix4->prefix, buf1, INET_ADDRSTRLEN),
              pPrefix4->prefixLen,
              inet_ntop (AF_INET, pGate, buf2, INET_ADDRSTRLEN),
              ifIndex);
    }
    else
    {
      NNLOG (LOG_DEBUG, "route %s/%d ifIndex %d doesn't exist in rib\n",
              inet_ntop (AF_INET, &pPrefix4->prefix, buf1, INET_ADDRSTRLEN),
              pPrefix4->prefixLen, ifIndex);
    }

    return RIBMGR_ERR_RTNOEXIST;
  }

  /* Lookup same type route. */
  for (pRib = pRNode->pInfo; pRib; pRib = pRib->next)
  {
    NNLOG(LOG_DEBUG, "%s %d\n", __FUNCTION__, __LINE__);
    if (CHECK_FLAG (pRib->status, RIB_ENTRY_REMOVED))
    {
      continue;
    }

    if (CHECK_FLAG (pRib->flags, RIB_FLAG_SELECTED))
    {
      pFib = pRib;
    }

    if (pRib->type != type)
    {
      continue;
    }

    if (pRib->type == RIB_ROUTE_TYPE_CONNECT && (pNextHop = pRib->pNexthop) &&
        pNextHop->type == NEXTHOP_TYPE_IFINDEX && pNextHop->ifIndex == ifIndex)
    {
      NNLOG(LOG_DEBUG, "%s %d\n", __FUNCTION__, __LINE__);
      if (pRib->refCount)
      {
        pRib->refCount--;
        nnRouteNodeUnlock (pRNode);
        nnRouteNodeUnlock (pRNode);
        return 0;
      }
      pSame = pRib;
      break;
    }
    /* Make sure that the route found has the same gateway. */
    else if (pGate == NULL ||
             ((pNextHop = pRib->pNexthop) &&
              (PREFIX_IPV4_ADDR_SAME (&pNextHop->gate.ipv4, pGate) ||
               PREFIX_IPV4_ADDR_SAME (&pNextHop->rGate.ipv4, pGate)))) 
    {
      pSame = pRib;
      NNLOG(LOG_DEBUG, "%s %d\n", __FUNCTION__, __LINE__);
      NNLOG (LOG_DEBUG, "pSame = %p, pRib = %p\n", pSame, pRib);
      break;
    }
    else
    {
      NNLOG(LOG_WARNING, "Unknown case happened.\n"); 
    }
  }

  /* If same type of route can't be found and this message is from
     kernel. */
  if (! pSame)
  {
    NNLOG(LOG_DEBUG, "%s %d\n", __FUNCTION__, __LINE__);
    if (pFib && type == RIB_ROUTE_TYPE_KERNEL)
    {
      NNLOG(LOG_DEBUG, "%s %d\n", __FUNCTION__, __LINE__);
	  /* Unset flags. */
	  for (pNextHop = pFib->pNexthop; pNextHop; pNextHop = pNextHop->next)
	    UNSET_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB);

	  UNSET_FLAG (pFib->flags, RIB_FLAG_SELECTED);
    }
    else
    {
      NNLOG(LOG_DEBUG, "%s %d\n", __FUNCTION__, __LINE__);

      if (pGate)
      {
        NNLOG (LOG_DEBUG, "route %s/%d via %s ifIndex %d type %d doesn't exist in rib\n",
                inet_ntop (AF_INET, &pPrefix4->prefix, buf1, INET_ADDRSTRLEN),
                pPrefix4->prefixLen,
                inet_ntop (AF_INET, pGate, buf2, INET_ADDRSTRLEN),
                ifIndex, type);
      }
      else
      {
        NNLOG (LOG_DEBUG, "route %s/%d ifIndex %d type %d doesn't exist in rib\n",
                inet_ntop (AF_INET, &pPrefix4->prefix, buf1, INET_ADDRSTRLEN),
                pPrefix4->prefixLen, ifIndex, type);
      }

      nnRouteNodeUnlock (pRNode);
      return RIBMGR_ERR_RTNOEXIST;
    }
  }
  
  if (pSame)
  {
    NNLOG(LOG_DEBUG, "%s %d\n", __FUNCTION__, __LINE__);
    ribDeleteNode (pRNode, pSame);
    ribUnlink (pRNode, pSame); // testing...
  }
  
  nnRouteNodeUnlock (pRNode);

  // just print all of routing table
  printRouteTable();

  return 0;
}


/* Description : Static IPv4 Route를 설정하는 함수. */
void
staticInstallIpv4 (PrefixT *pPrefix, StaticIpv4T *pSi)
{
  RibT *pRib = NULL;
  RouteNodeT *pRNode = NULL;
  RouteTableT *pTable = NULL;

  /* Lookup table. */
  pTable = vrfTable (AFI_IP, SAFI_UNICAST, 0);
  if (! pTable)
    return;

  /* Lookup existing route */
  pRNode = nnRouteNodeGet (pTable, pPrefix);
  for (pRib = pRNode->pInfo; pRib; pRib = pRib->next)
  {
    if (CHECK_FLAG (pRib->status, RIB_ENTRY_REMOVED))
    {
      continue;
    }

    if (pRib->type == RIB_ROUTE_TYPE_STATIC && pRib->distance == pSi->distance)
    {
      break;
    }
  }

  if (pRib)
  {
    /* Same distance static route is there.  Update it with new nexthop. */
    nnRouteNodeUnlock (pRNode);
    switch (pSi->type)
    {
      case NEXTHOP_TYPE_IPV4:
        if(nexthopIpv4Lookup(pRib, &pSi->gate.ipv4) == NULL)
        {
          nexthopIpv4Add (pRib, &pSi->gate.ipv4, NULL);
        }
        break;
      case NEXTHOP_TYPE_IFNAME:
        if(nexthopIfnameLookup(pRib, pSi->gate.ifName) == NULL)
        {
          nexthopIfnameAdd (pRib, pSi->gate.ifName);
        }
        break;
      case NEXTHOP_TYPE_NULL0:
          nexthopBlackholeAdd (pRib);
        break;
      default :
        NNLOG(LOG_ERR, "Wrong pSi->type = %d\n", pSi->type);
        break;
    }

    NNLOG(LOG_DEBUG, "RIB Finded ...\n");
    NNLOG(LOG_DEBUG, "## Calling ribQueueAdd ...%s, %s, %d\n", 
              __FILE__, __FUNCTION__, __LINE__); 
  }
  else
  {
    /* New Static Route. */
    pRib = NNMALLOC (MEM_ROUTE_RIB, sizeof (RibT));
    pRib->type = RIB_ROUTE_TYPE_STATIC;
    pRib->distance = pSi->distance;
    pRib->metric = 0;
    pRib->nexthopNum = 0;

    /* Add Nexthop to RIB */
    switch (pSi->type)
    {
      case NEXTHOP_TYPE_IPV4:
        nexthopIpv4Add (pRib, &pSi->gate.ipv4, NULL);
        break;
      case NEXTHOP_TYPE_IFNAME:
        nexthopIfnameAdd (pRib, pSi->gate.ifName);
        break;
      case NEXTHOP_TYPE_NULL0:
        nexthopBlackholeAdd (pRib);
        break;
      default :
        NNLOG(LOG_ERR, "Wrong pSi->type = %d\n", pSi->type);
        break;
    }

    /* Save the flags of this static routes (reject, blackhole) */
    pRib->flags = pSi->flags;

    NNLOG(LOG_DEBUG, "pRib->flags = 0x%x, pSi->flags = 0x%x\n", 
          pRib->flags, pSi->flags);
    NNLOG(LOG_DEBUG, "RIB NEW ...\n");

    /* Link this rib to the tree. */
    ribAddNode (pRNode, pRib);
   
    /* Testing Flag */
    //SET_FLAG (pRib->flags, RIB_FLAG_SELECTED);
  }

  /* Try RIB to Kernel */
  ribProcess(pRNode);
}


/**
 * Description : Static IPv4 Nexthop이 동일한가를 확인하는 함수.
 *
 * @param [in] pNextHop : NexthopT *
 * @param [in] pSi : StaticIpv4T *
 *
 * @retval : 0 if not Same,
 *           1 if same
 */
static Int32T
staticIpv4NexthopSame (NexthopT *pNextHop, StaticIpv4T *pSi)
{
  if (pNextHop->type == NEXTHOP_TYPE_IPV4 && 
      pSi->type == NEXTHOP_TYPE_IPV4 && 
      PREFIX_IPV4_ADDR_SAME (&pNextHop->gate.ipv4, &pSi->gate.ipv4))
  {
    return 1;
  }

  if (pNextHop->type == NEXTHOP_TYPE_IFNAME && 
      pSi->type == NEXTHOP_TYPE_IFNAME && 
      strcmp (pNextHop->ifName, pSi->gate.ifName) == 0)
  {
    return 1;
  }

  if (pNextHop->type == NEXTHOP_TYPE_NULL0 && 
      pSi->type == NEXTHOP_TYPE_NULL0)
  {
    return 1;
  }

  return 0;
}

/* Description : Static IPv4 Route를 삭제하는 함수. */
void
staticUninstallIpv4 (PrefixT *pPrefix, StaticIpv4T *pSi)
{
  RibT *pRib = NULL;
  NexthopT *pNextHop = NULL;
  RouteNodeT *pRNode = NULL;
  RouteTableT *pTable = NULL;

  /* Lookup table.  */
  pTable = vrfTable (AFI_IP, SAFI_UNICAST, 0);
  if (! pTable)
    return;
  
  /* Lookup existing route with type and distance. */
  pRNode = nnRouteNodeLookup (pTable, pPrefix);
  if (! pRNode)
  {
    return;
  }

  for (pRib = pRNode->pInfo; pRib; pRib = pRib->next)
  {
    if (CHECK_FLAG (pRib->status, RIB_ENTRY_REMOVED))
    {
      continue;
    }

    NNLOG(LOG_DEBUG, " pRib->type = %d, pRib->distance =%d, pSi->distance=%d\n", 
                        pRib->type, pRib->distance, pSi->distance);

    if (pRib->type == RIB_ROUTE_TYPE_STATIC && 
        pRib->distance == pSi->distance)
    {
      break;
    }
  }

  if (! pRib)
  {
    nnRouteNodeUnlock (pRNode);
    return;
  }

  /* Lookup nexthop. */
  for (pNextHop = pRib->pNexthop; pNextHop; pNextHop = pNextHop->next)
    if (staticIpv4NexthopSame (pNextHop, pSi))
    {
      break;
    }
  
  /* Can't find nexthop. */
  if (! pNextHop)
  {
    nnRouteNodeUnlock (pRNode);
    return;
  }

  /* Check nexthop. */
  if (pRib->nexthopNum == 1)
  {
    ribDeleteNode (pRNode, pRib);
    ribUnlink (pRNode, pRib); // testing...
  }
  else
  {
    if (CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB))
    {
      ribUninstall (pRNode, pRib);
    }

    nexthopDelete (pRib, pNextHop);
    nexthopFree (pNextHop);

    NNLOG(LOG_DEBUG, "## Calling ribQueueAdd ...%s, %s, %d\n", 
              __FILE__, __FUNCTION__, __LINE__); 
  }
  /* Unlock node. */
  nnRouteNodeUnlock (pRNode);
}


/*
 * Description : Static Route 설정하는 함수.
 *
 * param [in] p : PrefixT 자료구조 포인터
 * param [in] nhType : nexthop 타입
 * param [in] strNexthop : nexthop 문자열
 * param [in] distance : 루트 디스턴스
 *
 * retval : always 1
 */
Int32T
staticAddIpv4(PrefixT *pPrefix, Uint8T nhType,
              const char * strNexthop, Uint8T distance)
{
  RouteTableT * pStaticTable = NULL;
  RouteNodeT *pRNode = NULL;
  StaticIpv4T *pSi = NULL;
  StaticIpv4T *pPp = NULL;
  StaticIpv4T *pCp = NULL;
  StaticIpv4T *pUpdate = NULL;
  struct in_addr gate = {0,};
  Uint8T msgFlags = 0; 


  /* Lookup static route table. */
  pStaticTable = vrfStaticTable (AFI_IP, SAFI_UNICAST, 0);
  if (!pStaticTable)
  {
    NNLOG(LOG_ERR, "Not exist static table. \n");
    return -1;
  }

  /* Lookup static route prefix. */
  pRNode = (RouteNodeT  *)nnRouteNodeGet (pStaticTable, pPrefix);

  /* Check & Convert IPv4 Address. */
  if (nhType == NEXTHOP_TYPE_IPV4)
    inet_aton(strNexthop, &gate);

  /* Do nothing if there is a same static route. */
  for (pSi = pRNode->pInfo; pSi; pSi = pSi->next)
  {
    if (nhType == pSi->type && 
        ((nhType == NEXTHOP_TYPE_IPV4 && PREFIX_IPV4_ADDR_SAME(&gate, &pSi->gate.ipv4)) || 
         (nhType == NEXTHOP_TYPE_IFNAME && strcmp(strNexthop, pSi->gate.ifName) == 0) ||
         (nhType == NEXTHOP_TYPE_NULL0)))
    {    
      if (distance == pSi->distance)
      {    
        NNLOG (LOG_DEBUG, "Same Routing Information\n");
        nnRouteNodeUnlock (pRNode);
        return 0;
      }    
      else 
      {    
        NNLOG (LOG_DEBUG, "Update Routing Information\n");
        pUpdate = pSi;
      }    
    }    
  }

  /* Distance changed. */
  if (pUpdate)
  {
    NNLOG (LOG_DEBUG, "Update So should delete...\n");
    /* Delete Static Route installed before. */
    staticDeleteIpv4 (pPrefix, nhType, strNexthop, pUpdate->distance);
  }

  /* Make new static route structure. */
  pSi = malloc(sizeof (StaticIpv4T));
  memset(pSi, 0, sizeof(StaticIpv4T));

  /* Copy Nexthop Type and Distance. */
  pSi->type = nhType;
  pSi->distance = distance;
  SET_FLAG(msgFlags, RIB_MESSAGE_DISTANCE);

  /* Copy Nexthop Value. */
  if (nhType == NEXTHOP_TYPE_IPV4)
  {
    pSi->gate.ipv4 = gate;
  }
  else if (nhType == NEXTHOP_TYPE_IFNAME)
  {
    pSi->gate.ifName = strdup(strNexthop);
  }
  else if (nhType == NEXTHOP_TYPE_NULL0)
  {
    pSi->flags = RIB_FLAG_NULL0;
  }

  /* Add new static route information to the tree with sort by
     distance value and gateway address. */
  for (pPp = NULL, pCp = pRNode->pInfo; pCp; pPp = pCp, pCp = pCp->next)
  {
    if (pSi->distance < pCp->distance)
      break;
    if (pSi->distance > pCp->distance)
      continue;
    if (pSi->type == NEXTHOP_TYPE_IPV4 && pCp->type == NEXTHOP_TYPE_IPV4)
    {
      if (ntohl (pSi->gate.ipv4.s_addr) < ntohl (pCp->gate.ipv4.s_addr))
        break;
      if (ntohl (pSi->gate.ipv4.s_addr) > ntohl (pCp->gate.ipv4.s_addr))
        continue;
    }
  }

  /* Make linked list. */
  if (pPp)
    pPp->next = pSi;
  else
    pRNode->pInfo = pSi;

  if (pCp)
    pCp->prev = pSi;

  pSi->prev = pPp;
  pSi->next = pCp;

  /* Set route node. */
  staticInstallIpv4(pPrefix, pSi);

  return 1; 
}


/*
 * Description : Static Route 삭제하는 함수.
 *
 * param [in] pPrefix : PrefixT 자료구조 포인터
 * param [in] nhType : nexthop 타입
 * param [in] strNexthop : nexthop 문자열
 * param [in] distance : 루트 디스턴스
 *
 * retval : always 0
 */
Int32T
staticDeleteIpv4(PrefixT *pPrefix, Uint8T nhType,
                 const char * strNexthop, Uint8T distance)
{
  StaticIpv4T * pSi = NULL;
  RouteTableT * pStaticTable = NULL;
  RouteNodeT * pRNode = NULL;
  struct in_addr gate = {0,};

  /* Lookup table.  */
  pStaticTable = vrfStaticTable (AFI_IP, SAFI_UNICAST, 0);
  if (! pStaticTable)
  {
    NNLOG(LOG_ERR, "Not exist static route table. \n");
    return -1;
  }

  /* Lookup static route prefix. */
  pRNode = nnRouteNodeGet (pStaticTable, pPrefix);
  if (!pRNode)
  {
    NNLOG(LOG_ERR, "Not exist static route node. \n");
    return -1;
  }

  /* Check & Convert IPv4 Address. */
  if (nhType == NEXTHOP_TYPE_IPV4)
    inet_aton(strNexthop, &gate);

  /* Find same static route is the tree. */
  for (pSi = pRNode->pInfo; pSi; pSi = pSi->next)
  {
    if (nhType == pSi->type &&
        ((nhType == NEXTHOP_TYPE_IPV4 && PREFIX_IPV4_ADDR_SAME(&gate, &pSi->gate.ipv4)) || 
         (nhType == NEXTHOP_TYPE_IFNAME && strcmp(strNexthop, pSi->gate.ifName) == 0) ||
         (nhType == NEXTHOP_TYPE_NULL0)))
    {    
      if(distance != pSi->distance)
        return 0;
      else 
        break;
    }    
  }

  /* Can't find static route. */
  if (! pSi)
  {
    NNLOG(LOG_ERR, "Not exist static ipv4 info.\n");
    nnRouteNodeUnlock (pRNode);
    return 0;
  }

  /* Unlink static route from linked list. */
  if (pSi->prev)
    pSi->prev->next = pSi->next;
  else
    pRNode->pInfo = pSi->next;
  if (pSi->next)
    pSi->next->prev = pSi->prev;
  nnRouteNodeUnlock (pRNode);
 
  /* Set distance. */
  pSi->distance = distance; 

  /* Unlink static route from linked list. */
  staticUninstallIpv4 (pPrefix, pSi);

  /* Free static route configuration. */
  if ((nhType == NEXTHOP_TYPE_IFNAME) && pSi->gate.ifName)
    free (pSi->gate.ifName);

  free (pSi);

  nnRouteNodeUnlock (pRNode);  

  return 0;
}


#ifdef HAVE_IPV6
/**
 * Description : IPv6 Route 값이 Bogus 인지를 확인하는 함수.
 *
 * @param [in] type : route type
 * @param [in] pPrefix6 : Prefix6T *
 * @param [in] pGate : struct in6_addr *
 * @param [in] ifIndex : nexthop's interface index
 * @param [in] table : table id
 *
 * @retval : 1 if True
 *           0 if False
 */
static Int32T
ribBogusIpv6 (Int32T type, Prefix6T *pPrefix6,
              struct in6_addr *pGate, Uint32T ifIndex, Int32T table)
{
  if (type == RIB_ROUTE_TYPE_CONNECT && 
      IN6_IS_ADDR_UNSPECIFIED (&pPrefix6->prefix)) 
  {
#if defined (MUSICA) || defined (LINUX)
    /* IN6_IS_ADDR_V4COMPAT(&p->prefix) */
    if (pPrefix6->prefixLen == 96)
    {
      return 0;
    }
#endif /* MUSICA */
    return 1;
  }

  if (type == RIB_ROUTE_TYPE_KERNEL && 
      IN6_IS_ADDR_UNSPECIFIED (&pPrefix6->prefix) && 
      pPrefix6->prefixLen == 96 && pGate && IN6_IS_ADDR_UNSPECIFIED (pGate))
  {
      kernelDeleteIpv6Old (pPrefix6, pGate, ifIndex, 0, table);
      return 1;
  }

  return 0;
}


/* Description : IPv6 Route를 설정하는 함수. */
Int32T
ribAddIpv6 (Int32T type, Int32T flags, Prefix6T *pPrefix6,
            struct in6_addr *pGate, Uint32T ifIndex, Uint32T vrfId,
            Uint32T metric, Uint8T distance)
{
  RibT *pRib = NULL;
  RibT *pSame = NULL;
  NexthopT *pNextHop = NULL;
  RouteNodeT *pRNode = NULL;
  RouteTableT *pTable = NULL;

  /* Lookup table.  */
  pTable = vrfTable (AFI_IP6, SAFI_UNICAST, 0);
  if (! pTable)
  {
    return 0;
  }

  /* Make sure mask is applied. */
  nnApplyNetmasktoPrefix6 (pPrefix6, &pPrefix6->prefixLen);

  /* Set default distance by route type. */
  if (!distance)
  {
    distance = ROUTE_INFO[type].distance;
  }
  
  if (type == RIB_ROUTE_TYPE_BGP && CHECK_FLAG (flags, RIB_FLAG_IBGP))
  {
    distance = 200;
  }

  /* Filter bogus route. */
  if (ribBogusIpv6 (type, pPrefix6, pGate, ifIndex, 0))
  {
    return 0;
  }

  /* Lookup route node.*/
  pRNode = nnRouteNodeGet (pTable, (PrefixT *) pPrefix6);

  /* If same type of route are installed, treat it as a implicit
     withdraw. */
  for (pRib = pRNode->pInfo; pRib; pRib = pRib->next)
  {
    if (CHECK_FLAG (pRib->status, RIB_ENTRY_REMOVED))
    {
      continue;
    }

    if (pRib->type != type)
    {
      continue;
    }

    if (pRib->type != RIB_ROUTE_TYPE_CONNECT)
    {
      pSame = pRib;
      break;
    }
    else if ((pNextHop = pRib->pNexthop) &&
             (pNextHop->type == NEXTHOP_TYPE_IFINDEX) &&
             (pNextHop->ifIndex == ifIndex))
    {
      pRib->refCount++;
      return 0;
    }
    else
    {
      NNLOG(LOG_WARNING, "Unknown case happened.\n");
    }
  }

  /* Allocate new rib structure. */
  pRib = NNMALLOC (MEM_ROUTE_RIB, sizeof (RibT));
  pRib->type = type;
  pRib->distance = distance;
  pRib->flags = flags;
  pRib->metric = metric;
  pRib->table = vrfId;
  pRib->nexthopNum = 0;
  pRib->uptime = time (NULL);

  /* Nexthop settings. */
  if (pGate)
  {
    if (ifIndex)
    {
      nexthopIpv6IfindexAdd (pRib, pGate, ifIndex);
    }
    else
    {
      nexthopIpv6Add (pRib, pGate);
    }
  }
  else
  {
    nexthopIfindexAdd (pRib, ifIndex);
  }

  /* If this route is kernel route, set FIB flag to the route. */
  if (type == RIB_ROUTE_TYPE_KERNEL || type == RIB_ROUTE_TYPE_CONNECT)
  {
    for (pNextHop = pRib->pNexthop; pNextHop; pNextHop = pNextHop->next)
      SET_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB);
  }

  /* Link new rib to node.*/
  ribAddNode (pRNode, pRib);

  /* Free implicit route.*/
  if (pSame)
  {
    ribDeleteNode (pRNode, pSame);
  }
  
  nnRouteNodeUnlock (pRNode);
  return 0;
}


/* Description : IPv6 Route를 삭제하는 함수. */
Int32T
ribDeleteIpv6 (Int32T type, Int32T flags, Prefix6T *pPrefix6,
               struct in6_addr *pGate, Uint32T ifIndex, Uint32T vrfId)
{
  RibT *pRib = NULL;
  RibT *pFib = NULL;
  RibT *pSame = NULL;
  NexthopT *pNextHop = NULL;
  RouteNodeT *pRNode = NULL;
  RouteTableT *pTable = NULL;
  char buf1[INET6_ADDRSTRLEN] = {};
  char buf2[INET6_ADDRSTRLEN] = {};

  /* Apply mask. */
  nnApplyNetmasktoPrefix6 (pPrefix6, &pPrefix6->prefixLen);

  /* Lookup table.  */
  pTable = vrfTable (AFI_IP6, SAFI_UNICAST, 0);
  if (! pTable)
  {
    return 0;
  }
  
  /* Lookup route node. */
  pRNode = nnRouteNodeLookup (pTable, (PrefixT *) pPrefix6);
  if (! pRNode)
  {

    if (pGate)
    {
      NNLOG (LOG_DEBUG, "route %s/%d via %s ifIndex %d doesn't exist in rib\n",
              inet_ntop (AF_INET6, &pPrefix6->prefix, buf1, INET6_ADDRSTRLEN),
              pPrefix6->prefixLen,
              inet_ntop (AF_INET6, pGate, buf2, INET6_ADDRSTRLEN),
              ifIndex);
    }
    else
    {
      NNLOG (LOG_DEBUG, "route %s/%d ifIndex %d doesn't exist in rib\n",
              inet_ntop (AF_INET6, &pPrefix6->prefix, buf1, INET6_ADDRSTRLEN),
              pPrefix6->prefixLen, ifIndex);
    }

    return RIBMGR_ERR_RTNOEXIST;
  }

  /* Lookup same type route. */
  for (pRib = pRNode->pInfo; pRib; pRib = pRib->next)
  {
    if (CHECK_FLAG(pRib->status, RIB_ENTRY_REMOVED))
    {
      continue;
    }

    if (CHECK_FLAG (pRib->flags, RIB_FLAG_SELECTED))
    {
      pFib = pRib;
    }

    if (pRib->type != type)
    {
      continue;
    }

    if (pRib->type == RIB_ROUTE_TYPE_CONNECT && 
        (pNextHop = pRib->pNexthop) &&
	    pNextHop->type == NEXTHOP_TYPE_IFINDEX && 
        pNextHop->ifIndex == ifIndex)
	{
      if (pRib->refCount)
      {
        pRib->refCount--;
        nnRouteNodeUnlock (pRNode);
        nnRouteNodeUnlock (pRNode);
        return 0;
      }

      pSame = pRib;
      break;
    }
    /* Make sure that the route found has the same gateway. */
    else if (pGate == NULL ||
             ((pNextHop = pRib->pNexthop) &&
             (PREFIX_IPV6_ADDR_SAME (&pNextHop->gate.ipv6, pGate) ||
             PREFIX_IPV6_ADDR_SAME (&pNextHop->rGate.ipv6, pGate))))
    {
      pSame = pRib;
      break;
    }
    else
    {
      NNLOG(LOG_WARNING, "Unknown case happened.\n");
    }
  }

  /* If same type of route can't be found and this message is from
     kernel. */
  if (! pSame)
  {
    if (pFib && type == RIB_ROUTE_TYPE_KERNEL)
    {
	  /* Unset flags. */
      for (pNextHop = pFib->pNexthop; pNextHop; pNextHop = pNextHop->next)
        UNSET_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB);

      UNSET_FLAG (pFib->flags, RIB_FLAG_SELECTED);
    }
    else
    {
      if (pGate)
      {
        NNLOG (LOG_DEBUG, "route %s/%d via %s ifIndex %d type %d doesn't exist in rib\n",
                inet_ntop (AF_INET6, &pPrefix6->prefix, buf1, INET6_ADDRSTRLEN),
                pPrefix6->prefixLen, inet_ntop (AF_INET6, pGate, buf2, INET6_ADDRSTRLEN),
                ifIndex, type);
      }
      else
      {
        NNLOG (LOG_DEBUG, "route %s/%d ifIndex %d type %d doesn't exist in rib\n",
                inet_ntop (AF_INET6, &pPrefix6->prefix, buf1, INET6_ADDRSTRLEN),
                pPrefix6->prefixLen, ifIndex, type);
      }

      nnRouteNodeUnlock (pRNode);
      return RIBMGR_ERR_RTNOEXIST;
    }
  }

  if (pSame)
  {
    ribDeleteNode (pRNode, pSame);
  }
  
  nnRouteNodeUnlock (pRNode);
  return 0;
}

/**
 * Description : Static IPv6 Route를 설정하는 함수.
 *
 * @param [in] pPrefix : PrefixT *
 * @param [in] pSi : StaticIpv6T *
 */
static void
staticInstallIpv6 (PrefixT *pPrefix, StaticIpv6T *pSi)
{
  RouteTableT *pTable = NULL;
  RouteNodeT *pRNode = NULL;
  RibT *pRib = NULL;

  /* Lookup table.  */
  pTable = vrfTable (AFI_IP6, SAFI_UNICAST, 0);
  if (! pTable)
  {
    return;
  }

  /* Lookup existing route */
  pRNode = nnRouteNodeGet (pTable, pPrefix);
  for (pRib = pRNode->pInfo; pRib; pRib = pRib->next)
  {
    if (CHECK_FLAG(pRib->status, RIB_ENTRY_REMOVED))
    {
      continue;
    }

    if (pRib->type == RIB_ROUTE_TYPE_STATIC && 
        pRib->distance == pSi->distance)
    {
      break;
    }
  }

  if (pRib)
  {
    /* Same distance static route is there.  Update it with new
       nexthop. */
    nnRouteNodeUnlock (pRNode);

    switch (pSi->type)
    {
      case STATIC_IPV6_GATEWAY:
        nexthopIpv6Add (pRib, &pSi->ipv6);
        break;
      case STATIC_IPV6_IFNAME:
        nexthopIfnameAdd (pRib, pSi->ifName);
        break;
      case STATIC_IPV6_GATEWAY_IFNAME:
        nexthopIpv6IfnameAdd (pRib, &pSi->ipv6, pSi->ifName);
        break;
      default :
        NNLOG(LOG_ERR, "Wrong pSi->type = %d\n", pSi->type);
        break;
    }

    NNLOG(LOG_DEBUG, "## Calling ribQueueAdd ...%s, %s, %d\n", 
          __FILE__, __FUNCTION__, __LINE__); 
  }
  else
  {
    /* This is new static route. */
    pRib = NNMALLOC (MEM_ROUTE_RIB, sizeof (RibT));
    pRib->type = RIB_ROUTE_TYPE_STATIC;
    pRib->distance = pSi->distance;
    pRib->metric = 0;
    pRib->nexthopNum = 0;

    switch (pSi->type)
    {
      case STATIC_IPV6_GATEWAY:
        nexthopIpv6Add (pRib, &pSi->ipv6);
        break;
      case STATIC_IPV6_IFNAME:
        nexthopIfnameAdd (pRib, pSi->ifName);
        break;
      case STATIC_IPV6_GATEWAY_IFNAME:
        nexthopIpv6IfnameAdd (pRib, &pSi->ipv6, pSi->ifName);
        break;
      default :
        NNLOG(LOG_ERR, "Wrong pSi->type = %d\n", pSi->type);
        break;

    }

    /* Save the flags of this static routes (reject, blackhole) */
    pRib->flags = pSi->flags;

    /* Link this rib to the tree. */
    ribAddNode (pRNode, pRib);
  }
}

/*
 * Description : Static IPv6 Route의 nexthop이 동일한가를 확인하는 함수.
 *
 * param [in] pNextHop : NexthopT *
 * param [in] pSi : StaticIpv6T *
 *
 * retval : 1 if True,
 *          0 if False
 */
static Int32T
staticIpv6NexthopSame (NexthopT *pNextHop, StaticIpv6T *pSi)
{
  if (pNextHop->type == NEXTHOP_TYPE_IPV6 && 
      pSi->type == STATIC_IPV6_GATEWAY && 
      PREFIX_IPV6_ADDR_SAME (&pNextHop->gate.ipv6, &pSi->ipv6))
  {
    return 1;
  }

  if (pNextHop->type == NEXTHOP_TYPE_IFNAME && 
      pSi->type == STATIC_IPV6_IFNAME && 
      strcmp (pNextHop->ifName, pSi->ifName) == 0)
  {
    return 1;
  }

  if (pNextHop->type == NEXTHOP_TYPE_IPV6_IFNAME && 
      pSi->type == STATIC_IPV6_GATEWAY_IFNAME && 
      PREFIX_IPV6_ADDR_SAME (&pNextHop->gate.ipv6, &pSi->ipv6) && 
      strcmp (pNextHop->ifName, pSi->ifName) == 0)
  {
    return 1;
  }

  return 0;
}


/*
 * Description : Static IPv6 Route를 삭제하는 함수.
 *
 * param [in] pPrefix : PrefixT *
 * param [in] pSi : StaticIpv6T *
 */
static void
staticUninstallIpv6 (PrefixT *pPrefix, StaticIpv6T *pSi)
{
  RibT *pRib = NULL;
  RouteTableT *pTable = NULL;
  RouteNodeT *pRNode = NULL;
  NexthopT *pNextHop = NULL;

  /* Lookup table.  */
  pTable = vrfTable (AFI_IP6, SAFI_UNICAST, 0);
  if (! pTable)
  {
    return;
  }

  /* Lookup existing route with type and distance. */
  pRNode = nnRouteNodeLookup (pTable, (PrefixT *) pPrefix);
  if (! pRNode)
  {
    return;
  }

  for (pRib = pRNode->pInfo; pRib; pRib = pRib->next)
  {
    if (CHECK_FLAG (pRib->status, RIB_ENTRY_REMOVED))
    {
      continue;
    }
    
    if (pRib->type == RIB_ROUTE_TYPE_STATIC && 
        pRib->distance == pSi->distance)
    {
      break;
    }
  }

  if (! pRib)
  {
    nnRouteNodeUnlock (pRNode);
    return;
  }

  /* Lookup nexthop. */
  for (pNextHop = pRib->pNexthop; pNextHop; pNextHop = pNextHop->next)
    if (staticIpv6NexthopSame (pNextHop, pSi))
    {
      break;
    }

  /* Can't find nexthop. */
  if (! pNextHop)
  {
    nnRouteNodeUnlock (pRNode);
    return;
  }
  
  /* Check nexthop. */
  if (pRib->nexthopNum == 1)
  {
    ribDeleteNode (pRNode, pRib);
  }
  else
  {
    if (CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB))
    {
      ribUninstall (pRNode, pRib);
    }
    nexthopDelete (pRib, pNextHop);
    nexthopFree (pNextHop);

    NNLOG(LOG_DEBUG, "## Calling ribQueueAdd ...%s, %s, %d\n", 
          __FILE__, __FUNCTION__, __LINE__); 
  }
  /* Unlock node. */
  nnRouteNodeUnlock (pRNode);
}


/* Description : Static IPv6 Route를 Configuration 정보로 저장하는 함수. */
Int32T
staticAddIpv6 (PrefixT *pPrefix, Uint8T type, struct in6_addr *pGate,
               StringT ifName, Uint8T flags, Uint8T distance,
               Uint32T vrfId)
{
  RouteTableT *pStable = NULL;
  RouteNodeT *pRNode = NULL;
  StaticIpv6T *si = NULL;
  StaticIpv6T *pp = NULL;
  StaticIpv6T *cp = NULL;

  /* Lookup table.  */
  pStable = vrfStaticTable (AFI_IP6, SAFI_UNICAST, vrfId);
  if (! pStable)
  {
    return -1;
  }
    
  if (!pGate &&
      (type == STATIC_IPV6_GATEWAY || type == STATIC_IPV6_GATEWAY_IFNAME))
  {
    return -1;
  }
  
  if (!ifName && 
      (type == STATIC_IPV6_GATEWAY_IFNAME || type == STATIC_IPV6_IFNAME))
  {
    return -1;
  }

  /* Lookup static route prefix. */
  pRNode = nnRouteNodeGet (pStable, pPrefix);

  /* Do nothing if there is a same static route.  */
  for (si = pRNode->pInfo; si; si = si->next)
  {
    if (distance == si->distance && 
        type == si->type && 
        (! pGate || PREFIX_IPV6_ADDR_SAME (pGate, &si->ipv6)) && 
        (! ifName || strcmp (ifName, si->ifName) == 0))
    {
      nnRouteNodeUnlock (pRNode);
      return 0;
    }
  }

  /* Make new static route structure. */
  si = NNMALLOC (MEM_ROUTE_RIB, sizeof (StaticIpv6T));
  si->type = type;
  si->distance = distance;
  si->flags = flags;

  switch (type)
  {
    case STATIC_IPV6_GATEWAY:
      si->ipv6 = *pGate;
      break;
    case STATIC_IPV6_IFNAME:
      si->ifName = nnStrDup (ifName, MEM_IF_NAME);
      break;
    case STATIC_IPV6_GATEWAY_IFNAME:
      si->ipv6 = *pGate;
      si->ifName = nnStrDup (ifName, MEM_IF_NAME);
      break;
    default :
      NNLOG(LOG_ERR, "Wrong type = %d\n", type);
      break;
  }

  /* Add new static route information to the tree with sort by
     distance value and gateway address. */
  for (pp = NULL, cp = pRNode->pInfo; cp; pp = cp, cp = cp->next)
  {
    if (si->distance < cp->distance)
    {
      break;
    }

    if (si->distance > cp->distance)
    {
      continue;
    }
  }

  /* Make linked list. */
  if (pp)
  {
    pp->next = si;
  }
  else
  {
    pRNode->pInfo = si;
  }

  if (cp)
  {
    cp->prev = si;
  }

  si->prev = pp;
  si->next = cp;

  /* Install into rib. */
  staticInstallIpv6 (pPrefix, si);

  return 1;
}


/* Description : Static IPv6 Route를 Configuration 정보를 삭제하는 함수. */
Int32T
staticDeleteIpv6 (PrefixT *pPrefix, Uint8T type, struct in6_addr *pGate,
                  const StringT ifName, Uint8T distance, Uint32T vrfId)
{
  RouteTableT *pStable = NULL;
  RouteNodeT *pRNode = NULL;
  StaticIpv6T *pSi = NULL;

  /* Lookup table.  */
  pStable = vrfStaticTable (AFI_IP6, SAFI_UNICAST, vrfId);
  if (! pStable)
  {
    return -1;
  }

  /* Lookup static route prefix. */
  pRNode = nnRouteNodeLookup (pStable, pPrefix);
  if (! pRNode)
  {
    return 0;
  }

  /* Find same static route is the tree */
  for (pSi = pRNode->pInfo; pSi; pSi = pSi->next)
    if ((distance == pSi->distance) && 
        (type == pSi->type) && 
        (! pGate || PREFIX_IPV6_ADDR_SAME (pGate, &pSi->ipv6)) && 
        (! ifName || strcmp (ifName, pSi->ifName) == 0))
    {
      break;
    }

  /* Can't find static route. */
  if (! pSi)
  {
    nnRouteNodeUnlock (pRNode);
    return 0;
  }

  /* Install into rib. */
  staticUninstallIpv6 (pPrefix, pSi);

  /* Unlink static route from linked list. */
  if (pSi->prev)
  {
    pSi->prev->next = pSi->next;
  }
  else
  {
    pRNode->pInfo = pSi->next;
  }

  if (pSi->next)
  {
    pSi->next->prev = pSi->prev;
  }
  
  /* Free static route configuration. */
  if (ifName)
  {
    NNFREE (MEM_IF_NAME, pSi->ifName);
  }

  NNFREE (MEM_ROUTE_RIB, pSi);

  return 1;
}
#endif /* HAVE_IPV6 */


/* Description : RIB 정보를 갱신하는 함수. */
void
ribUpdate (void)
{
  RouteNodeT *pRNode = NULL;
  RouteTableT *pTable = NULL;
  
  pTable = vrfTable (AFI_IP, SAFI_UNICAST, 0);
  if (pTable)
  {
    for (pRNode = nnRouteTop (pTable); pRNode; pRNode = nnRouteNext (pRNode))
      if (pRNode->pInfo)
      {
        char strPrefix[20] = {};
        nnCnvPrefixtoString (strPrefix, &pRNode->p);
        NNLOG(LOG_DEBUG, "%s called. prefix=%s\n", __FUNCTION__, strPrefix); 

        /* Handling interface related route. */
        ribProcess (pRNode);
      }
  }

  pTable = vrfTable (AFI_IP6, SAFI_UNICAST, 0);
  if (pTable)
  {
    for (pRNode = nnRouteTop (pTable); pRNode; pRNode = nnRouteNext (pRNode))
      if (pRNode->pInfo)
      {
        char strPrefix[20] = {};
        nnCnvPrefixtoString (strPrefix, &pRNode->p);
        NNLOG(LOG_DEBUG, "%s called. prefix=%s\n", __FUNCTION__, strPrefix); 

        /* Handling interface related route. */
        ribProcess (pRNode);
      }
  }
}


/* Description : Main Table로 부터 설정된 Route 외의 모든 Route를 삭제하는 함수. */
static void
ribWeedTable (RouteTableT *pTable)
{
  RouteNodeT *pRNode = NULL;
  RibT *pRib = NULL;
  RibT *pNext = NULL;

  if (pTable)
  {
    for (pRNode = nnRouteTop (pTable); 
         pRNode; 
         pRNode = nnRouteNext (pRNode))
    {
      for (pRib = pRNode->pInfo; pRib; pRib = pNext)
      {
        pNext = pRib->next;

        if (CHECK_FLAG (pRib->status, RIB_ENTRY_REMOVED))
        {
          continue;
        }

        if (pRib->table != pRibmgr->rtmTableDefault && 
            pRib->table != RT_TABLE_MAIN)
        {
          ribDeleteNode (pRNode, pRib);
        }
      }
    }
  }
}


/* Description : Main Table로 부터 설정된 Route 외의 모든 Route를 삭제하는 함수. */
void
ribWeedTables (void)
{
  ribWeedTable (vrfTable (AFI_IP, SAFI_UNICAST, 0));
#ifdef HAVE_IPV6
  ribWeedTable (vrfTable (AFI_IP6, SAFI_UNICAST, 0));
#endif
}


/*
 * Description : RIB Manager가 기동된 이후에 자체적으로 만들어진 모든 Route를
 * 삭제하는 함수.
 *
 * param [in] table : RouteTableT *
 */
static void
ribSweepTable (RouteTableT *pTable)
{
  RouteNodeT *pRNode = NULL;
  RibT *pRib = NULL;
  RibT *pNext = NULL;
  Int32T ret = 0;

  if (pTable)
  {
    for (pRNode = nnRouteTop (pTable); 
         pRNode; 
         pRNode = nnRouteNext (pRNode))
    {
      for (pRib = pRNode->pInfo; pRib; pRib = pNext)
      {
        pNext = pRib->next;
        if (CHECK_FLAG (pRib->status, RIB_ENTRY_REMOVED))
        {
          continue;
        }

        if (pRib->type == RIB_ROUTE_TYPE_KERNEL && 
            CHECK_FLAG (pRib->flags, RIB_FLAG_SELFROUTE))
        {
          ret = ribUninstallKernel (pRNode, pRib);
          if (! ret)
          {
            ribDeleteNode (pRNode, pRib);
          }
        }
      }
    }
  }
}


/* Description : RIB Manager가 기동된 이후에 자체적으로 만들어진 모든 Route를
 * 삭제하는 함수. */
void
ribSweepRoute (void)
{
  ribSweepTable (vrfTable (AFI_IP, SAFI_UNICAST, 0));
#ifdef HAVE_IPV6
  ribSweepTable (vrfTable (AFI_IP6, SAFI_UNICAST, 0));
#endif
}

/**
 * Description : RIB를 종료하고, Kernel Route를 정리하는 함수.
 *
 * @param [in] pTable : RouteTableT *
 */
static void
ribCloseTable (RouteTableT *pTable)
{
  RouteNodeT *pRNode = NULL;
  RibT *pRib = NULL;

  if (pTable)
  {
    for (pRNode = nnRouteTop (pTable); 
         pRNode; 
         pRNode = nnRouteNext (pRNode))
    {
      for (pRib = pRNode->pInfo; pRib; pRib = pRib->next)
      {
          if (! RIB_SYSTEM_ROUTE (pRib) && 
              CHECK_FLAG (pRib->flags, RIB_FLAG_SELECTED))
          {
            ribUninstallKernel (pRNode, pRib);
          }
      }
    }
  }
}

/* Description : 모든 RIB 테이블을 닫는 함수. */
void
ribClose (void)
{
  ribCloseTable (vrfTable (AFI_IP, SAFI_UNICAST, 0));
#ifdef HAVE_IPV6
  ribCloseTable (vrfTable (AFI_IP6, SAFI_UNICAST, 0));
#endif
}


/* Description : RIB 테이블 초기화. */
void
ribInit (void)
{
  /* VRF initialization.  */
  vrfInit ();
}


/*************************************************************************
 * Below code is for client component handling.
 ************************************************************************/

/* Description : create client node. */
ClientT *
clientCreate (Int8T componentId)
{
  ClientT * pClient = NULL;

  pClient = clientLookupById (componentId);
  if (pClient)
  {
    NNLOG (LOG_WARNING, "Client[%d] already exist.\n", componentId);
    return pClient; 
  }

  pClient = NNMALLOC (MEM_OTHER, sizeof (ClientT));
  pClient->componentId = componentId;

  nnListAddNode (pRibmgr->pClientList, pClient);

  return pClient;
}


/* Description : free client node. */
void
clientDelete (Int8T componentId)
{
  ClientT * pClient = NULL;

  /* Lookup client node ptr. */
  pClient = clientLookupById (componentId);
  if (pClient)
  {
    NNLOG (LOG_WARNING, "Client[%d] not exist. \n", componentId);
    return;
  }

  NNLOG (LOG_DEBUG, "Client[%d] delete in client list. \n", componentId);
  nnListDeleteNode (pRibmgr->pClientList, pClient);  
}

/* Description : client pointer lookup. */
ClientT * 
clientLookupById (Int8T componentId)
{
  ClientT * pClient = NULL;
  ListNodeT * pNode = NULL;

  for(pNode = pRibmgr->pClientList->pHead;
      pNode != NULL;
      pNode = pNode->pNext)
  {
    pClient = pNode->pData;
    if (pClient->componentId == componentId)
    {
      return pClient;
    }
  }

  return NULL;
}

/* Description : client pointer lookup or create node. */
ClientT *
clientGetById (Int8T componentId)
{
  ClientT * pClient = NULL;

  pClient = clientLookupById (componentId);
  if (pClient)
  {
    return pClient;
  }

  pClient = clientCreate (componentId);
  return pClient;
}


/* Description : Client list init. */
void
clientListInit ()
{
  printf ("===> %s %d called. \n", __func__, __LINE__);

  /* Check ribmgr global pointer. */
  assert (pRibmgr);

  /* Initialize client list. */
  pRibmgr->pClientList = nnListInit (NULL, MEM_OTHER);
  
}


/* Description : Client list free. */
void 
clientListFree ()
{
  printf ("===> %s %d called. \n", __func__, __LINE__);

  /* Check ribmgr global pointer. and client list pointer. */
  assert (pRibmgr);
  assert (pRibmgr->pClientList);

  /* Free client list. */
  nnListFree (pRibmgr->pClientList);
}
