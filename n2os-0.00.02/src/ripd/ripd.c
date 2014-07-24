/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : RIP Protocol의 라우팅 정보 처리하는 주요 화일
 *
 * - Block Name : RIB Manager
 * - Process Name : ribmgr
 * - Creator : Geontae Park
 * - Initial Date : 2014/02/20
 */

/**
 * @file        : ripd.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include <assert.h>
#include <netinet/ip.h>
#include <linux/types.h>
#include <unistd.h>

#include "nnRibDefines.h"
#include "nnSocketOption.h"
#include "nnSocketUnion.h"
#include "nnFilter.h"
#include "nnIfRouteMap.h"
#include "nnPlist.h"
#include "nnDistribute.h"
#include "nnMd5.h"


#include "nnTypes.h"
#include "nnStr.h"
#include "taskManager.h"
#include "nnIf.h"
#include "nnPrefix.h"
#include "nnTable.h"
#include "nnList.h"
#include "nnKeychain.h"
#include "nnBuffer.h"
#include "nosLib.h"

#include "nnCmdDefines.h"
#include "nnCompProcess.h"
#include "nnCmdCmsh.h"

#include "ripd.h"
#include "ripInit.h"
#include "ripInterface.h"
#include "ripPeer.h"
#include "ripOffset.h"
#include "ripRoutemap.h"
#include "ripRibmgr.h"
#include "ripDebug.h"
#include "ripUtil.h"

/* UDP receive buffer size .*/
#define RIP_UDP_RCV_BUF 41600


/* RIP Structure. */
RipT * pRip = NULL;

/* external data structures .*/

/* external functions .*/
extern Int32T
WriteConfCB(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);

/* Prototypes. */
static void ripGarbageTimerSet (RipInfoT *);
void ripUpdateProcess (Int32T);
void ripClearChangedFlag (void);

static void ripOutputProcess (ConnectedT *, struct sockaddr_in *, Int32T, Uint8T);
void ripTriggeredUpdate (Int32T fd, Int16T event, void * arg);

/* RIP output routes type. */
enum
{
  RIP_ALL_ROUTE,
  RIP_CHANGED_ROUTE
};

/* RIP command strings. */
static const struct message ripMsg[] =
{
  {RIP_REQUEST,    "REQUEST"},
  {RIP_RESPONSE,   "RESPONSE"},
  {RIP_TRACEON,    "TRACEON"},
  {RIP_TRACEOFF,   "TRACEOFF"},
  {RIP_POLL,       "POLL"},
  {RIP_POLL_ENTRY, "POLL ENTRY"},
  {0, NULL},
};


static Int32T
ripRouteRte (RipInfoT *pRInfo)
{
  return (pRInfo->type == RIB_ROUTE_TYPE_RIP && 
          pRInfo->subType == RIP_ROUTE_RTE);
}

static RipInfoT *
ripInfoNew (void)
{
  return NNMALLOC (MEM_RIP_INFO, sizeof (RipInfoT));
}

void
ripInfoFree (RipInfoT *pRInfo)
{
  /* Check pRInfo pointer. */
  assert (pRInfo);

  NNFREE (MEM_RIP_INFO, pRInfo);
  pRInfo = NULL;
}


/* RIP's periodical timer. */
void
ripTimerUpdate (Int32T fd, Int16T event, void * arg)
{
  if (IS_RIP_DEBUG_EVENT)
    NNLOG(LOG_DEBUG,"update timer fire.\n");

  /* Process update output. */
  ripUpdateProcess (RIP_ALL_ROUTE);

  /* Set local time for marking updated timer called. */
  gettimeofday (&pRip->tvUpdatedTime, NULL);

  return;
}

static void
ripUpdateTimerSet ()
{
  /* Check rip global pointer. */
  assert (pRip);

  /* Set update timer. */
  pRip->pTimerUpdate = taskTimerSet(ripTimerUpdate, pRip->tvUpdateTime,
                                    TASK_PERSIST, NULL);
  gettimeofday (&pRip->tvUpdatedTime, NULL);
}

static void
ripUpdateTimerDelete ()
{
  /* Check rip global pointer. */
  assert (pRip);

  /* Check rip update timer value. */
  assert (pRip->pTimerUpdate);

  /* Delete update timer. */
  taskTimerDel(pRip->pTimerUpdate);
  pRip->pTimerUpdate = NULL;
}

static void
ripUpdateTimerUpdate ()
{
  /* Check rip global pointer. */
  assert (pRip);

  /* Check rip update timer value. */
  assert (pRip->pTimerUpdate);

  /* Update update timer. */
  taskTimerUpdate (ripTimerUpdate, pRip->pTimerUpdate, 
                   pRip->tvUpdateTime, NULL);
}


static void
ripTriggeredTimerSet()
{
  /* Check rip global pointer. */
  assert (pRip);

  if (pRip->pTimerTriggered)
  {
    if (IS_RIP_DEBUG_EVENT)
      NNLOG (LOG_DEBUG, "%s, already exist, so return.\n", __func__);

    return;
  }

  /* Triggered update timer(just one time called). */
  pRip->pTimerTriggered = taskTimerSet (ripTriggeredUpdate, 
                                        pRip->tvTriggeredTime, 
                                        TASK_PERSIST, NULL);
}

static void
ripTriggeredTimerDelete()
{
  /* Check rip global pointer. */
  assert (pRip);

  /* Check rip triggered timer pointer. */
  assert (pRip->pTimerTriggered);

  if (IS_RIP_DEBUG_EVENT)
    NNLOG (LOG_DEBUG, "%s !\n", __func__);

  /* Delete triggered timer. */
  taskTimerDel (pRip->pTimerTriggered);
  pRip->pTimerTriggered = NULL;
}

/* Execute triggered update. */
void
ripTriggeredUpdate (Int32T fd, Int16T event, void * arg)
{
  /* Logging triggered update. */
  if (IS_RIP_DEBUG_EVENT)
    NNLOG(LOG_DEBUG,"%s !", __func__);

  /* When process multiple rte entry, only one timer should be added.*/
  if (! pRip->pTimerTriggered)
  {
    return;
  }

  /* Split Horizon processing is done when generating triggered
     updates as well as normal updates (see section 2.6). */
  ripUpdateProcess (RIP_CHANGED_ROUTE);

  /* Once all of the triggered updates have been generated, the route
     change flags should be cleared. */
  ripClearChangedFlag ();

  /* Delete triggered timer. */
  ripTriggeredTimerDelete();
}



/* Timeout RIP routes. */
static void
ripTimerTimeout (Int32T fd, Int16T event, void * arg)
{
  NNLOG (LOG_DEBUG, "TIMER. %s called.\n", __func__);

  RipInfoT *pRInfo = (RipInfoT *)arg;
  RouteNodeT *pRNode;

  pRInfo->pTimerTimeout = NULL;

  pRNode = pRInfo->rp;

  /* - The garbage-collection timer is set for 120 seconds. */
  ripGarbageTimerSet (pRInfo);

  ripRibmgrIpv4Delete ((Prefix4T *)&pRNode->p, &pRInfo->nexthop,
			 pRInfo->metric);
  /* - The metric for the route is set to 16 (infinity).  This causes
     the route to be removed from service. */
  pRInfo->metric = RIP_METRIC_INFINITY;
  pRInfo->flags &= ~RIP_RTF_FIB;

  /* - The route change flag is to indicate that this entry has been
     changed. */
  pRInfo->flags |= RIP_RTF_CHANGED;

  ripUpdateProcess (RIP_CHANGED_ROUTE);
  ripClearChangedFlag ();
}

static void
ripTimeoutTimerSet (RipInfoT *pRInfo)
{
  /* Check rip global pointer. */
  assert (pRip);

  /* Check rip infomation pointer. */
  assert (pRInfo);

  /* Set timeout timer. */
  pRInfo->pTimerTimeout = taskTimerSet(ripTimerTimeout, 
                                   pRip->tvTimeoutTime, 0, (void *)pRInfo);
  gettimeofday (&pRInfo->tvUpdated, NULL);
}

static void
ripTimeoutTimerDelete (RipInfoT *pRInfo)
{
  /* Check rip global pointer. */
  assert (pRip);

  /* Check rip infomation pointer. */
  assert (pRInfo);

  /* Check rip update timer value. */
  assert (pRInfo->pTimerTimeout);

  /* Delete timeout timer. */
  taskTimerDel(pRInfo->pTimerTimeout);
  pRInfo->pTimerTimeout = NULL;
}

static void
ripTimeoutTimerUpdate (RipInfoT *pRInfo)
{
  /* Check rip global pointer. */
  assert (pRip);

  /* Check rip infomation pointer. */
  assert (pRInfo);

  if (IS_RIP_DEBUG_EVENT)
    NNLOG (LOG_DEBUG, "%s .\n", __func__);

  /* Update timeout timer. */
  if (pRInfo->metric != RIP_METRIC_INFINITY)
  {
    if(pRInfo->pTimerTimeout)
    {
      ripTimeoutTimerDelete (pRInfo);
    }

    ripTimeoutTimerSet (pRInfo);
  }
}


/* RIP route garbage collect timer : callback function. */
static void
ripTimerGarbageCollect (Int32T fd, Int16T event, void * arg)
{
  RipInfoT *pRInfo = (RipInfoT *)arg;
  RouteNodeT *pRNode;

  pRInfo->pTimerGarbage = NULL;

  /* Off timeout timer. */
  if(pRInfo->pTimerTimeout)
  { 
    NNLOG (LOG_DEBUG, "%s, Prefix = %s\n", 
           __func__, inet_ntoa(pRInfo->rp->p.u.prefix4));
    ripTimeoutTimerDelete (pRInfo);
  }
  
  /* Get route_node pointer. */
  pRNode = pRInfo->rp;

  /* Unlock route_node. */
  pRNode->pInfo = NULL;
  nnRouteNodeUnlock (pRNode);

  /* Free RIP routing information. */
  ripInfoFree (pRInfo);
}

/* RIP route garbage collect timer : set timer. */
static void
ripGarbageTimerSet (RipInfoT * pRInfo)
{
  /* Check rip global pointer. */
  assert (pRip);

  /* Check rip infomation pointer. */
  assert (pRInfo);

  /* Set garbage collect timer. */
  if (!pRInfo->pTimerGarbage)
  {
    pRInfo->pTimerGarbage = taskTimerSet(ripTimerGarbageCollect, 
                                   pRip->tvGarbageTime, 0, (void *)pRInfo);
    gettimeofday (&pRInfo->tvUpdated, NULL);
  }
}


/* RIP route garbage collect timer : delete timer. */
static void 
ripGabageTimerDelete (RipInfoT * pRInfo)
{
  /* Check rip global pointer. */
  assert (pRip);

  /* Check rip infomation pointer. */
  assert (pRInfo);

  /* Check rip update timer value. */
  assert (pRInfo->pTimerGarbage);

  /* Delete garbage collect timer. */
  taskTimerDel(pRInfo->pTimerGarbage);
  pRInfo->pTimerGarbage = NULL;
}

/* RIP route garbage collect timer : update timer. */
static void
ripGarbageTimerUpdate (RipInfoT * pRInfo)
{
  /* Check rip global pointer. */
  assert (pRip);

  /* Check rip infomation pointer. */
  assert (pRInfo);

  /* Check rip update timer value. */
  assert (pRInfo->pTimerGarbage);

  /* Delete garbage collect timer. */
  taskTimerDel(pRInfo->pTimerGarbage);
  taskTimerUpdate (ripTimerGarbageCollect, pRInfo->pTimerGarbage, 
                   pRip->tvGarbageTime, (void *)pRInfo);
  gettimeofday (&pRInfo->tvUpdated, NULL); // testing...
}

static Int32T
ripIncomingFilter (Prefix4T *p, RipInterfaceT *pRIf)
{
  DistributeT *dist;
  AccessListT *alist;
  PrefixListT *plist;

  /* Input distribute-list filtering. */
  if (pRIf->pAccessList[RIP_FILTER_IN])
  {
    if (accessListApply (pRIf->pAccessList[RIP_FILTER_IN], 
                           (PrefixT *) p) == FILTER_DENY)
	{
      if (IS_RIP_DEBUG_PACKET)
        NNLOG(LOG_DEBUG,"%s/%d filtered by distribute in\n",
              inet_ntoa (p->prefix), p->prefixLen);
	  return -1;
	}
  }
  if (pRIf->pPrefixList[RIP_FILTER_IN])
  {
    if (prefixListApply (pRIf->pPrefixList[RIP_FILTER_IN], 
                           (PrefixT *) p) == PREFIX_DENY)
    {
      if (IS_RIP_DEBUG_PACKET)
        NNLOG(LOG_DEBUG,"%s/%d filtered by prefix-list in\n",
              inet_ntoa (p->prefix), p->prefixLen);
	  return -1;
	}
  }

  /* All interface filter check. */
  dist = distributeLookup (NULL);
  if (dist)
  {
    if (dist->filterName[DISTRIBUTE_IN])
    {
      alist = accessListLookup (AFI_IP, dist->filterName[DISTRIBUTE_IN]);
	    
      if (alist)
      {
        if (accessListApply (alist, (PrefixT *) p) == FILTER_DENY)
        {
          if (IS_RIP_DEBUG_PACKET)
            NNLOG(LOG_DEBUG,"%s/%d filtered by distribute in\n",
                  inet_ntoa (p->prefix), p->prefixLen);
		  return -1;
        }
      }
    }
    if (dist->prefixName[DISTRIBUTE_IN])
	{
      plist = prefixListLookup (AFI_IP, dist->prefixName[DISTRIBUTE_IN]);
	  
      if (plist)
      {
        if (prefixListApply (plist, (PrefixT *) p) == PREFIX_DENY)
        {
          if (IS_RIP_DEBUG_PACKET)
            NNLOG(LOG_DEBUG,"%s/%d filtered by prefix-list in\n",
                  inet_ntoa (p->prefix), p->prefixLen);
		  return -1;
        }
      }
    }
  }
  return 0;
}

static Int32T
ripOutgoingFilter (Prefix4T *p, RipInterfaceT *pRIf)
{
  DistributeT *dist;
  AccessListT *alist;
  PrefixListT *plist;

  if (pRIf->pAccessList[RIP_FILTER_OUT])
  {
    if (accessListApply (pRIf->pAccessList[RIP_FILTER_OUT],
                           (PrefixT *) p) == FILTER_DENY)
    {
	  if (IS_RIP_DEBUG_PACKET)
        NNLOG(LOG_DEBUG,"%s/%d is filtered by distribute out\n",
              inet_ntoa (p->prefix), p->prefixLen);
      return -1;
    }
  }
  if (pRIf->pPrefixList[RIP_FILTER_OUT])
  {
    if (prefixListApply (pRIf->pPrefixList[RIP_FILTER_OUT],
                           (PrefixT *) p) == PREFIX_DENY)
    {
	  if (IS_RIP_DEBUG_PACKET)
        NNLOG(LOG_DEBUG,"%s/%d is filtered by prefix-list out\n",
              inet_ntoa (p->prefix), p->prefixLen);
	  return -1;
    }
  }

  /* All interface filter check. */
  dist = distributeLookup (NULL);
  if (dist)
  {
    if (dist->filterName[DISTRIBUTE_OUT])
    {
      alist = accessListLookup (AFI_IP, dist->filterName[DISTRIBUTE_OUT]);
	    
      if (alist)
      {
        if (accessListApply (alist, (PrefixT *) p) == FILTER_DENY)
        {
          if (IS_RIP_DEBUG_PACKET)
            NNLOG(LOG_DEBUG,"%s/%d filtered by distribute out\n",
                  inet_ntoa (p->prefix), p->prefixLen);
          return -1;
        }
      }
    }
    if (dist->prefixName[DISTRIBUTE_OUT])
    {
      plist = prefixListLookup (AFI_IP, dist->prefixName[DISTRIBUTE_OUT]);
	  
      if (plist)
      {
        if (prefixListApply (plist, (PrefixT *) p) == PREFIX_DENY)
		{
          if (IS_RIP_DEBUG_PACKET)
            NNLOG(LOG_DEBUG,"%s/%d filtered by prefix-list out\n",
                  inet_ntoa (p->prefix), p->prefixLen);
          return -1;
        }
      }
    }
  }

  return 0;
}

/* Check nexthop address validity. */
static Int32T
ripNexthopCheck (struct in_addr *pAddr)
{
  ListNodeT * pNode = NULL;
  InterfaceT * pIf = NULL;
  PrefixT * pPrefix = NULL;

  /* If nexthop address matches local configured address then it is
     invalid nexthop. */
  for(pNode = pRip->pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pIf = pNode->pData;

    ConnectedT *pIfc = NULL;
    ListT * pConnectedList = pIf->pConnected;
    ListNodeT * pConnectedNode = NULL;

    for(pConnectedNode = pConnectedList->pHead;
        pConnectedNode != NULL;
        pConnectedNode = pConnectedNode->pNext)
    {
      pIfc = pConnectedNode->pData;

      pPrefix = pIfc->pAddress;

      if (pPrefix->family == AF_INET && 
          PREFIX_IPV4_ADDR_SAME (&pPrefix->u.prefix4, pAddr))
        return -1;
	}
  }

  return 0;
}

/* RIP add route to routing table. */
static void
ripRteProcess (RteT * pRte, struct sockaddr_in * pFrom,
               InterfaceT * pIf)
{
  Int32T ret =0, same = 0, routeReuse = 0;
  Uint8T oldMetric = 0, oldDistance = 0, newDistance = 0;
  Prefix4T p = {0,};
  RouteNodeT * pRNode = NULL;
  RipInfoT * pRInfo = NULL, rInfoTmp = {0,};
  RipInterfaceT * pRIf = NULL;
  struct in_addr * pNextHop = NULL;

  /* Make prefix structure. */
  memset (&p, 0, sizeof (Prefix4T));
  p.family = AF_INET;
  p.prefix = pRte->prefix;
  p.prefixLen = nnCnvIpmasktoLen (pRte->mask);

  /* Make sure mask is applied. */
  nnApplyNetmasktoPrefix4(&p, &p.prefixLen);

  /* Apply input filters. */
  pRIf = pIf->pInfo;

  ret = ripIncomingFilter (&p, pRIf);
  if (ret < 0)
    return;

  /* Modify entry according to the interface routemap. */
  if (pRIf->routeMap[RIP_FILTER_IN])
  {
    RipInfoT newInfo;

    memset (&newInfo, 0, sizeof (newInfo));
    newInfo.type = RIB_ROUTE_TYPE_RIP;
    newInfo.subType = RIP_ROUTE_RTE;
    newInfo.nexthop = pRte->nexthop;
    newInfo.from = pFrom->sin_addr;
    newInfo.ifIndex = pIf->ifIndex;
    newInfo.metric = pRte->metric;
    newInfo.metricOut = pRte->metric; /* XXX */
    newInfo.tag = ntohs (pRte->tag);   /* XXX */

    /* The object should be of the type of rip_info */
    ret = routeMapApply (pRIf->routeMap[RIP_FILTER_IN],
                           (PrefixT *) &p, RMAP_RIP, &newInfo);

    if (ret == RMAP_DENYMATCH)
    {
      if (IS_RIP_DEBUG_PACKET)
        NNLOG(LOG_DEBUG,"RIP %s/%d is filtered by route-map in\n",
              inet_ntoa (p.prefix), p.prefixLen);
      return;
    }

    /* Get back the object */
    pRte->nexthop = newInfo.nexthopOut;
    pRte->tag = htons (newInfo.tagOut); /* XXX */
    pRte->metric = newInfo.metricOut; /* XXX: the routemap uses the metricOut field */
  }

  /* Once the entry has been validated, update the metric by
     adding the cost of the network on wich the message
     arrived. If the result is greater than infinity, use infinity
     (RFC2453 Sec. 3.9.2) */
  /* Zebra ripd can handle offset-list in. */
  ret = ripOffsetListApplyIn (&p, pIf, &pRte->metric);

  /* If offset-list does not modify the metric use interface's
     metric. */
  if (!ret)
    pRte->metric += pIf->metric;

  if (pRte->metric > RIP_METRIC_INFINITY)
    pRte->metric = RIP_METRIC_INFINITY;

  /* Set nexthop pointer. */
  if (pRte->nexthop.s_addr == 0)
    pNextHop = &pFrom->sin_addr;
  else
    pNextHop = &pRte->nexthop;

  /* Check if nexthop address is myself, then do nothing. */
  if (ripNexthopCheck (pNextHop) < 0)
  {
    if (IS_RIP_DEBUG_PACKET)
      NNLOG(LOG_DEBUG,"Nexthop address %s is myself\n", inet_ntoa (*pNextHop));
    return;
  }

  /* Get index for the prefix. */
  pRNode = nnRouteNodeGet (pRip->pRipRibTable, (PrefixT *) &p);

  /* Check to see whether there is already RIP route on the table. */
  pRInfo = pRNode->pInfo;

  if (pRInfo)
  {
    /* Check gabage time is set or not. */
    if (pRInfo->pTimerGarbage)
    {
      if (memcmp(pNextHop, &pRInfo->nexthop, sizeof (struct in_addr)) == 0)
      {
        NNLOG (LOG_DEBUG, "GARBAGE : garbage timer set. \n");
        NNLOG (LOG_DEBUG, "GARBAGE : just return\n");
        nnRouteNodeUnlock (pRNode);
        return;
      }
    }

    /* Local static route. */
    if (pRInfo->type == RIB_ROUTE_TYPE_RIP && 
        ((pRInfo->subType == RIP_ROUTE_STATIC) ||
         (pRInfo->subType == RIP_ROUTE_DEFAULT)) && 
        pRInfo->metric != RIP_METRIC_INFINITY)
    {
      nnRouteNodeUnlock (pRNode);
      return;
    }

    /* Redistributed route check. */
    if (pRInfo->type != RIB_ROUTE_TYPE_RIP && 
        pRInfo->metric != RIP_METRIC_INFINITY)
    {
      /* Fill in a minimaly temporary rip_info structure, for a future
         ripDistanceApply() use) */
      memset (&rInfoTmp, 0, sizeof (rInfoTmp));
      PREFIX_IPV4_ADDR_COPY (&rInfoTmp.from, &pFrom->sin_addr);
      rInfoTmp.rp = pRInfo->rp;
      newDistance = ripDistanceApply (&rInfoTmp);
      newDistance = newDistance ? newDistance : RIB_DISTANCE_DEFAULT_RIP;
      oldDistance = pRInfo->distance;

      /* Only connected routes may have a valid NULL distance */
      if (pRInfo->type != RIB_ROUTE_TYPE_CONNECT)
      {
        oldDistance = oldDistance ? oldDistance : RIB_DISTANCE_DEFAULT_RIP;
      }

      /* If imported route does not have STRICT precedence, 
         mark it as a ghost */
      if (newDistance > oldDistance || pRte->metric == RIP_METRIC_INFINITY)
      {
        nnRouteNodeUnlock (pRNode);
        return;
      }
      else
      {
        if(pRInfo->pTimerTimeout)
        { 
          ripTimeoutTimerDelete (pRInfo);
        }

        if(pRInfo->pTimerGarbage)
        { 
          NNLOG (LOG_DEBUG, 
             "GARBAGE. garbage timer set. but this is different nexthop.\n");
          ripGabageTimerDelete (pRInfo);
        }
                                                                                
        pRNode->pInfo = NULL;
        if (ripRouteRte (pRInfo))
        {
          ripRibmgrIpv4Delete ((Prefix4T *)&pRNode->p, 
                                 &pRInfo->nexthop, pRInfo->metric);
        }
  
        ripInfoFree (pRInfo);
        routeReuse = 1;
      }
    }

    /* Check nexthop is not same && metric is same.
       Check timeline is between halftime and timeout time.
       In this case, we should accept and change this new route. */
    if (pRInfo->pTimerTimeout &&
        (pRInfo->metric == pRte->metric) &&
        (PREFIX_IPV4_ADDR_SAME (&pRInfo->from, pNextHop) == FALSE) &&
        (ripCheckTimeoutHalfTime (pRInfo) == TRUE) &&
        ripRouteRte (pRInfo))
    {
      
      /* Delete timeout timer. */
      ripTimeoutTimerDelete (pRInfo);

      if(pRInfo->pTimerGarbage)
      { 
        NNLOG (LOG_DEBUG, 
             "Half Timeout half. delete garbage timer because garbage timer set.\n");
        ripGabageTimerDelete (pRInfo);
      }
                                                                                
      pRNode->pInfo = NULL;
      ripRibmgrIpv4Delete ((Prefix4T *)&pRNode->p, 
                           &pRInfo->nexthop, pRInfo->metric);
      ripInfoFree (pRInfo);
      pRInfo = NULL;
      routeReuse = 1;
    }
  }

  if (!pRInfo)
  {
    /* Now, check to see whether there is already an explicit route
       for the destination prefix.  If there is no such route, add
       this route to the routing table, unless the metric is
       infinity (there is no point in adding a route which
       unusable). */
    if (pRte->metric != RIP_METRIC_INFINITY)
    {
      pRInfo = ripInfoNew ();

      /* - Setting the destination prefix and length to those in
         the RTE. */
      pRInfo->rp = pRNode;

      /* - Setting the metric to the newly calculated metric (as
         described above). */
      pRInfo->metric = pRte->metric;
      pRInfo->tag = ntohs (pRte->tag);

      /* - Set the next hop address to be the address of the router
         from which the datagram came or the next hop address
         specified by a next hop RTE. */
      PREFIX_IPV4_ADDR_COPY (&pRInfo->nexthop, pNextHop);
      PREFIX_IPV4_ADDR_COPY (&pRInfo->from, &pFrom->sin_addr);
      pRInfo->ifIndex = pIf->ifIndex;

      /* - Initialize the timeout for the route.  If the
         garbage-collection timer is running for this route, stop it
         (see section 2.3 for a discussion of the timers). */
      ripTimeoutTimerUpdate(pRInfo);

      /* - Set the route change flag. */
      pRInfo->flags |= RIP_RTF_CHANGED;

      /* - Signal the output process to trigger an update (see section
         2.5). 
         route entry's triggered processing will be occured 
         after 1 second */
      ripTriggeredTimerSet();

      /* Finally, route goes into the kernel. */
      pRInfo->type = RIB_ROUTE_TYPE_RIP;
      pRInfo->subType = RIP_ROUTE_RTE;

      /* Set distance value. */
      pRInfo->distance = ripDistanceApply (pRInfo);

      pRNode->pInfo = pRInfo;
      ripRibmgrIpv4Add (&p, &pRInfo->nexthop, pRInfo->metric,
                          pRInfo->distance);
      pRInfo->flags |= RIP_RTF_FIB;
    }

    /* Unlock temporary lock, i.e. same behaviour */
    if (routeReuse)
      nnRouteNodeUnlock (pRNode);
  }
  else
  {
    /* Route is there but we are not sure the route is RIP or not. */
    pRInfo = pRNode->pInfo;

    /* If there is an existing route, compare the next hop address
         to the address of the router from which the datagram came.
         If this datagram is from the same router as the existing
         route, reinitialize the timeout.  */
    same = (PREFIX_IPV4_ADDR_SAME (&pRInfo->from, &pFrom->sin_addr) && 
            (pRInfo->ifIndex == pIf->ifIndex));

    if (same)
      ripTimeoutTimerUpdate (pRInfo);

    /* Fill in a minimaly temporary rip_info structure, for a future
       ripDistanceApply() use) */
    memset (&rInfoTmp, 0, sizeof (rInfoTmp));
    PREFIX_IPV4_ADDR_COPY (&rInfoTmp.from, &pFrom->sin_addr);
    rInfoTmp.rp = pRInfo->rp;

    /* Next, compare the metrics.  If the datagram is from the same
       router as the existing route, and the new metric is different
       than the old one; or, if the new metric is lower than the old
       one, or if the tag has been changed; or if there is a route
       with a lower administrave distance; or an update of the
       distance on the actual route; do the following actions: */
    if ((same && pRInfo->metric != pRte->metric) || 
        (pRte->metric < pRInfo->metric) || 
        ((same) && 
         (pRInfo->metric == pRte->metric) && 
         ntohs (pRte->tag) != pRInfo->tag) || 
        (pRInfo->distance > ripDistanceApply (&rInfoTmp)) || 
        ((pRInfo->distance != ripDistanceApply (pRInfo)) && same))
    {
      /* - Adopt the route from the datagram.  That is, put the
         new metric in, and adjust the next hop address (if
         necessary). */
      oldMetric = pRInfo->metric;
      pRInfo->metric = pRte->metric;
      pRInfo->tag = ntohs (pRte->tag);
      PREFIX_IPV4_ADDR_COPY (&pRInfo->from, &pFrom->sin_addr);
      pRInfo->ifIndex = pIf->ifIndex;
      pRInfo->distance = ripDistanceApply (pRInfo);

      /* Should a new route to this network be established
         while the garbage-collection timer is running, the
         new route will replace the one that is about to be
         deleted.  In this case the garbage-collection timer
         must be cleared. */

      if (oldMetric == RIP_METRIC_INFINITY &&
          pRInfo->metric < RIP_METRIC_INFINITY)
      {
        pRInfo->type = RIB_ROUTE_TYPE_RIP;
        pRInfo->subType = RIP_ROUTE_RTE;

        if(pRInfo->pTimerTimeout)
        { 
          ripGabageTimerDelete (pRInfo);
        }

        if (!PREFIX_IPV4_ADDR_SAME (&pRInfo->nexthop, pNextHop))
          PREFIX_IPV4_ADDR_COPY (&pRInfo->nexthop, pNextHop);

        ripRibmgrIpv4Add (&p, pNextHop, pRInfo->metric, pRInfo->distance);
        pRInfo->flags |= RIP_RTF_FIB;

        /* If gabage collect timer is set, we should delete gabage timer. */
        if (pRInfo->pTimerGarbage)
        {
          ripGabageTimerDelete (pRInfo);
        }
        /* Set this entry timeout timer.
         * this case is receive route info about different nexthop, so we
         * make new route entry and set timeout timer(if default 180).
         */
        ripTimeoutTimerUpdate (pRInfo);
        
      }

      /* Update nexthop and/or metric value.  */
      if (oldMetric != RIP_METRIC_INFINITY)
      {
        ripRibmgrIpv4Delete (&p, &pRInfo->nexthop, oldMetric);
        ripRibmgrIpv4Add (&p, pNextHop, pRInfo->metric, pRInfo->distance);
        pRInfo->flags |= RIP_RTF_FIB;

        if (!PREFIX_IPV4_ADDR_SAME (&pRInfo->nexthop, pNextHop))
          PREFIX_IPV4_ADDR_COPY (&pRInfo->nexthop, pNextHop);
      }

      /* - Set the route change flag and signal the output process
         to trigger an update. */
      pRInfo->flags |= RIP_RTF_CHANGED;
      ripUpdateProcess (RIP_CHANGED_ROUTE);
      ripClearChangedFlag ();

      /* - If the new metric is infinity, start the deletion
         process (described above); */
      if (pRInfo->metric == RIP_METRIC_INFINITY)
      {
        /* If the new metric is infinity, the deletion process
           begins for the route, which is no longer used for
           routing packets.  Note that the deletion process is
           started only when the metric is first set to
           infinity.  If the metric was already infinity, then a
           new deletion process is not started. */
        if (oldMetric != RIP_METRIC_INFINITY)
        {
           /* - The garbage-collection timer is set for 120 seconds. */
           ripGarbageTimerSet (pRInfo);

           if(pRInfo->pTimerTimeout)
           {
             ripTimeoutTimerDelete (pRInfo);
           }
           /* - The metric for the route is set to 16
              (infinity).  This causes the route to be removed
              from service. */
           ripRibmgrIpv4Delete (&p, &pRInfo->nexthop, oldMetric);
           pRInfo->flags &= ~RIP_RTF_FIB;

           /* - The route change flag is to indicate that this
              entry has been changed. */
           /* - The output process is signalled to trigger a response. */
           /* Above processes are already done previously. */
        }
      }
      else
      {
        /* otherwise, re-initialize the timeout. */
        ripTimeoutTimerUpdate (pRInfo);
      }
    }
    /* Unlock tempolary lock of the route. */
    nnRouteNodeUnlock (pRNode);

    /* Check nexthop is not same && metric is same.
       Check timeline is between halftime and timeout time.
       In this case, we should accept and change this new route. */

  }
}

/* Dump RIP packet */
static void
ripPacketDump (RipPacketT *pPacket, Int32T size, const StringT strSndRcv)
{
  caddr_t lim;
  RteT * pRte = NULL;
  const char *  strCommand;
  char pbuf[BUFSIZ] = {}, nbuf[BUFSIZ] = {};
  Uint8T netmask = 0;
  Uint8T * p = NULL;

  /* Set command string. */
  if (pPacket->command > 0 && pPacket->command < RIP_COMMAND_MAX)
    strCommand = lookupStr (ripMsg, pPacket->command);
  else
    strCommand = "unknown";

  /* Dump packet header. */
  NNLOG(LOG_DEBUG,"%s %s version %d packet size %d\n",
	     strSndRcv, strCommand, pPacket->version, size);

  /* Dump each routing table entry. */
  pRte = pPacket->rte;
  
  for (lim = (caddr_t) pPacket + size; (caddr_t) pRte < lim; pRte++)
  {
    if (pPacket->version == RIPv2)
    {
      netmask = nnCnvIpmasktoLen (pRte->mask);

      if (pRte->family == htons (RIP_FAMILY_AUTH))
      {
        if (pRte->tag == htons (RIP_AUTH_SIMPLE_PASSWORD))
        {
          p = (Uint8T *)&pRte->prefix;

          NNLOG(LOG_DEBUG,"  family 0x%X type %d auth string: %s\n",
                ntohs (pRte->family), ntohs (pRte->tag), p);
        }
        else if (pRte->tag == htons (RIP_AUTH_MD5))
        {
          RipMd5InfoT *md5;

          md5 = (RipMd5InfoT *) &pPacket->rte;

          NNLOG(LOG_DEBUG,"  family 0x%X type %d (MD5 authentication)\n",
                          ntohs (md5->family), ntohs (md5->type));
          NNLOG(LOG_DEBUG,"    RIP-2 packet len %d Key ID %d"
                          " Auth Data len %d\n",
                          ntohs (md5->packetLen), md5->keyId,
                          md5->authLen);
          NNLOG(LOG_DEBUG,"    Sequence Number %ld\n",
                          (Uint32T) ntohl (md5->sequence));
        }
        else if (pRte->tag == htons (RIP_AUTH_DATA))
        {
          p = (Uint8T *)&pRte->prefix;

          NNLOG(LOG_DEBUG,"  family 0x%X type %d (MD5 data)\n",
                          ntohs (pRte->family), ntohs (pRte->tag));
          NNLOG(LOG_DEBUG,"    MD5: %02X%02X%02X%02X%02X%02X%02X%02X"
                          "%02X%02X%02X%02X%02X%02X%02X \n",
                             p[0], p[1], p[2], p[3], p[4], p[5], p[6],
                             p[7], p[9], p[10], p[11], p[12], p[13],
                             p[14], p[15]);
        }
        else
        {
          NNLOG(LOG_DEBUG,"  family 0x%X type %d (Unknown auth type) \n",
                          ntohs (pRte->family), ntohs (pRte->tag));
        }
      }
	  else
        NNLOG(LOG_DEBUG,"  %s/%d -> %s family %d tag %d metric %ld \n", // gtpark
                       inet_ntop (AF_INET, &pRte->prefix, pbuf, BUFSIZ),
                       netmask, inet_ntop (AF_INET, &pRte->nexthop, nbuf,
                                           BUFSIZ), ntohs (pRte->family),
                       ntohs (pRte->tag), (Uint32T) ntohl (pRte->metric));
    }
    else
    {
      NNLOG(LOG_DEBUG,"  %s family %d tag %d metric %ld\n", 
                       inet_ntop (AF_INET, &pRte->prefix, pbuf, BUFSIZ),
                       ntohs (pRte->family), ntohs (pRte->tag),
                       (Uint32T)ntohl (pRte->metric));
    }
  }
}

/* Check if the destination address is valid (unicast; not net 0
   or 127) (RFC2453 Section 3.9.2 - Page 26).  But we don't
   check net 0 because we accept default route. */
static Int32T
ripDestinationCheck (struct in_addr pAddr)
{
  Uint32T destination = 0;

  /* Convert to host byte order. */
  destination = ntohl (pAddr.s_addr);

  if (PREFIX_IPV4_NET127 (destination))
  {
    return 0;
  }

  /* Net 0 may match to the default route. */
  if (PREFIX_IPV4_NET0 (destination) && destination != 0)
  {
    return 0;
  }

  /* Unicast address must belong to class A, B, C. */
  if (IN_CLASSA (destination))
  {
    return 1;
  }
  if (IN_CLASSB (destination))
  {
    return 1;
  }
  if (IN_CLASSC (destination))
  {
    return 1;
  }

  return 0;
}


/* RIP version 2 authentication. */
static Int32T
ripAuthSimplePassword (RteT *pRte, struct sockaddr_in *pFrom,
                       InterfaceT * pIf)
{
  RipInterfaceT * pRIf = NULL;
  char * authStr = NULL;

  if (IS_RIP_DEBUG_EVENT)
    NNLOG(LOG_DEBUG,"RIPv2 simple password authentication from %s\n",
          inet_ntoa (pFrom->sin_addr));

  pRIf = pIf->pInfo;

  if (pRIf->authType != RIP_AUTH_SIMPLE_PASSWORD || 
      pRte->tag != htons(RIP_AUTH_SIMPLE_PASSWORD))
    return 0;

  /* Simple password authentication. */
  if (pRIf->authStr)
  {
    authStr = (char *) &pRte->prefix;

    if (strncmp (authStr, pRIf->authStr, 16) == 0)
    {
      return 1;
    }
  }
  if (pRIf->keyChain)
  {
    KeychainT *keychain;
    KeyT *key;

    keychain = keychainLookup (pRIf->keyChain);
    if (keychain == NULL)
    {
      return 0;
    }

    key = keyMatchForAccept (keychain, (char *) &pRte->prefix);
    if (key)
    {
      return 1;
    }
  }
  return 0;
}

/* RIP version 2 authentication with MD5. */
static Int32T
ripAuthMd5 (RipPacketT * pPacket, struct sockaddr_in * pFrom,
            Int32T length, InterfaceT * pIf)
{
  RipInterfaceT * pRIf = NULL;
  RipMd5InfoT * pMd5 = NULL;
  RipMd5DataT * pMd5data = NULL;
  KeychainT * pKeychain = NULL;
  KeyT * key = NULL;
  MD5_CTX ctx;
  Uint16T packetLen;
  char authStr[RIP_AUTH_MD5_SIZE]= {};

  if (IS_RIP_DEBUG_EVENT)
    NNLOG(LOG_DEBUG,"RIPv2 MD5 authentication from %s\n",
               inet_ntoa (pFrom->sin_addr));

  pRIf = pIf->pInfo;
  pMd5 = (RipMd5InfoT *) &pPacket->rte;

  /* Check auth type. */
  if (pRIf->authType != RIP_AUTH_MD5 || pMd5->type != htons(RIP_AUTH_MD5))
  {	
      return 0;
  }

  /* If the authentication length is less than 16, then it must be wrong for
   * any interpretation of rfc2082. Some implementations also interpret
   * this as RIP_HEADER_SIZE+ RIP_AUTH_MD5_SIZE, aka RIP_AUTH_MD5_COMPAT_SIZE.
   */
  if ( !((pMd5->authLen == RIP_AUTH_MD5_SIZE) || 
       (pMd5->authLen == RIP_AUTH_MD5_COMPAT_SIZE)))
  {
    if (IS_RIP_DEBUG_EVENT)
      NNLOG(LOG_DEBUG,"RIPv2 MD5 authentication, strange authentication\n "
            "length field %d", pMd5->authLen);
    return 0;
  }

  /* grab and verify check packet length */
  packetLen = ntohs (pMd5->packetLen);

  if (packetLen > (length - RIP_HEADER_SIZE - RIP_AUTH_MD5_SIZE))
  {
    if (IS_RIP_DEBUG_EVENT)
      NNLOG(LOG_DEBUG,"RIPv2 MD5 authentication, packet length field %d \n"
                      "greater than received length %d!",
                      pMd5->packetLen, length);
      return 0;
  }

  /* retrieve authentication data */
  pMd5data = (RipMd5DataT *) (((Uint8T *) pPacket) + packetLen);
  
  memset (authStr, 0, RIP_AUTH_MD5_SIZE);

  if (pRIf->keyChain)
  {
    pKeychain = keychainLookup (pRIf->keyChain);
    if (pKeychain == NULL)
    {
      return 0;
    }

    key = keyLookupForAccept (pKeychain, pMd5->keyId);
    if (key == NULL)
    {
      return 0;
    }

    strncpy (authStr, key->string, RIP_AUTH_MD5_SIZE);
  }
  else if (pRIf->authStr)
    strncpy (authStr, pRIf->authStr, RIP_AUTH_MD5_SIZE);

  if (authStr[0] == 0)
  {
    return 0;
  }
  
  /* MD5 digest authentication. */
  memset (&ctx, 0, sizeof(ctx));
  MD5Init(&ctx);
  MD5Update(&ctx, (void *)pPacket, packetLen + RIP_HEADER_SIZE);
  MD5Update(&ctx, (void *)authStr, RIP_AUTH_MD5_SIZE);
  MD5Final(&ctx);
  
  if (memcmp (pMd5data->digest, ctx.digest, RIP_AUTH_MD5_SIZE) == 0)
  {
    return packetLen;
  }
  else
  {
    return 0;
  }
}

/* Pick correct auth string for sends, prepare authStr buffer for use.
 * (left justified and padded).
 *
 * presumes one of ri or key is valid, and that the auth strings they point
 * to are nul terminated. If neither are present, authStr will be fully
 * zero padded.
 *
 */
static void
ripAuthPrepareStrSend (RipInterfaceT * pRIf, KeyT * pKey, 
                       StringT strAuth, Int32T len)
{
  assert (pRIf || pKey);

  memset (strAuth, 0, len);
  if (pKey && pKey->string)
    strncpy (strAuth, pKey->string, len);
  else if (pRIf->authStr)
    strncpy (strAuth, pRIf->authStr, len);

  return;
}

/* Write RIPv2 simple password authentication information
 *
 * authStr is presumed to be 2 bytes and correctly prepared 
 * (left justified and zero padded).
 */
static void
ripAuthSimpleWrite (nnBufferT * pBuf, StringT strAuth, Int32T len)
{
  assert (pBuf && len == RIP_AUTH_SIMPLE_SIZE);
  
  nnBufferSetInt16T (pBuf, RIP_FAMILY_AUTH);
  nnBufferSetInt16T (pBuf, RIP_AUTH_SIMPLE_PASSWORD);
  nnBufferSetString (pBuf, strAuth, RIP_AUTH_SIMPLE_SIZE);

  return;
}

/* write RIPv2 MD5 "authentication header" 
 * (uses the auth key data field)
 *
 * Digest offset field is set to 0.
 *
 * returns: offset of the digest offset field, which must be set when
 * length to the auth-data MD5 digest is known.
 */
static size_t
ripAuthMd5AhWrite (nnBufferT * pBuf, RipInterfaceT * pRIf, KeyT * pKey)
{
  size_t doff = 0;

  assert (pBuf && pRIf && pRIf->authType == RIP_AUTH_MD5);

  /* MD5 authentication. */
  nnBufferSetInt16T (pBuf, RIP_FAMILY_AUTH);
  nnBufferSetInt16T (pBuf, RIP_AUTH_MD5);

  /* MD5 AH digest offset field.
   *
   * Set to placeholder value here, to true value when RIP-2 Packet length
   * is known.  Actual value is set in .....().
   */
  doff = nnBufferGetLength (pBuf);
  nnBufferSetInt16T (pBuf, 0);

  /* Key ID. */
  if (pKey)
    nnBufferSetInt8T (pBuf, pKey->index % 256);
  else
    nnBufferSetInt8T (pBuf, 1);

  /* Auth Data Len.  Set 16 for MD5 authentication data. Older ripds 
   * however expect RIP_HEADER_SIZE + RIP_AUTH_MD5_SIZE so we allow for this
   * to be configurable. 
   */
  nnBufferSetInt8T (pBuf, pRIf->md5AuthLen);

  /* Sequence Number (non-decreasing). */
  /* RFC2080: The value used in the sequence number is
     arbitrary, but two suggestions are the time of the
     message's creation or a simple message counter. */
  nnBufferSetInt32T (pBuf, time (NULL));
	      
  /* Reserved field must be zero. */
  nnBufferSetInt32T (pBuf, 0);
  nnBufferSetInt32T (pBuf, 0);

  return doff;
}


/* If authentication is in used, write the appropriate header
 * returns buffer offset to which length must later be written
 * or 0 if this is not required
 */
static size_t
ripAuthHeaderWrite (nnBufferT * pBuf, RipInterfaceT * pRIf, 
                    KeyT * pKey, StringT strAuth, Int32T len)
{
  assert (pRIf->authType != RIP_NO_AUTH);
  
  switch (pRIf->authType)
  {
    case RIP_AUTH_SIMPLE_PASSWORD:
      ripAuthPrepareStrSend (pRIf, pKey, strAuth, len);
      ripAuthSimpleWrite (pBuf, strAuth, len);
      return 0;

    case RIP_AUTH_MD5:
      return ripAuthMd5AhWrite (pBuf, pRIf, pKey);
  }
  assert (1);

  return 0;
}


/* Write RIPv2 MD5 authentication data trailer */
static void
ripAuthMd5Set (nnBufferT * pBuf, RipInterfaceT * pRIf, size_t doff,
               StringT strAuth, Int32T authLen)
{
  Uint32T len = 0;
  MD5_CTX ctx;

  /* Make it sure this interface is configured as MD5
     authentication. */
  assert ((pRIf->authType == RIP_AUTH_MD5) && (authLen == RIP_AUTH_MD5_SIZE));
  assert (doff > 0);
  
  /* Get packet length. */
  len = nnBufferGetLength (pBuf);

  /* Check packet length. */
  if (len < (RIP_HEADER_SIZE + RIP_RTE_SIZE))
  {
    NNLOG(LOG_DEBUG,"ripAuthMd5Set(): packet length %ld is less than minimum length.\n", len);
    return;
  }

  /* Set the digest offset length in the header */
  nnBufferInsertInt16T (pBuf, doff, len);
  
  /* Set authentication data. */
  nnBufferSetInt16T (pBuf, RIP_FAMILY_AUTH);
  nnBufferSetInt16T (pBuf, RIP_AUTH_DATA);

  /* Generate a digest for the RIP packet. */
  memset(&ctx, 0, sizeof(ctx));
  MD5Init(&ctx);
  MD5Update(&ctx, (Uint8T *)pBuf->data, pBuf->length);
  MD5Update(&ctx, (Uint8T *)strAuth, RIP_AUTH_MD5_SIZE);
  MD5Final(&ctx);

  /* Copy the digest to the packet. */
  nnBufferSetString (pBuf, (StringT)ctx.digest, RIP_AUTH_MD5_SIZE);
}

/* RIP routing information. */
static void
ripResponseProcess (RipPacketT * pPacket, Int32T size, 
                    struct sockaddr_in * pFrom, ConnectedT * pIfc)
{
  caddr_t lim = 0;
  RteT * pRte = NULL;
  Prefix4T ifAddr = {0,};
  Prefix4T ifAddrClass = {0,};
  Int32T subnetted = 0;

  /* We don't know yet. */
  subnetted = -1;

  /* The Response must be ignored if it is not from the RIP
     port. (RFC2453 - Sec. 3.9.2)*/
  if (pFrom->sin_port != htons(RIP_PORT_DEFAULT))
  {
    NNLOG(LOG_DEBUG,"response doesn't come from RIP port: %d\n",
          pFrom->sin_port);
    ripPeerBadPacket (pFrom);
    return;
  }

  /* The datagram's IPv4 source address should be checked to see
     whether the datagram is from a valid neighbor; the source of the
     datagram must be on a directly connected network 
     (RFC2453 - Sec. 3.9.2) */
  if (ifLookupAddress(pFrom->sin_addr) == NULL)
  {
    NNLOG(LOG_DEBUG,
          "This datagram doesn't came from a valid neighbor: %s\n",
          inet_ntoa (pFrom->sin_addr));

    ripPeerBadPacket (pFrom);
    return;
  }

  /* It is also worth checking to see whether the response is from one
     of the router's own addresses. */

  /* Alredy done in rip_read () */

  /* Update RIP peer. */
  ripPeerUpdate (pFrom, pPacket->version);

  /* Set RTE pointer. */
  pRte = pPacket->rte;

  for (lim = (caddr_t) pPacket + size; (caddr_t) pRte < lim; pRte++)
  {
    /* RIPv2 authentication check. */
    /* If the Address Family Identifier of the first (and only the
	   first) entry in the message is 0xFFFF, then the remainder of
	   the entry contains the authentication. */
    /* If the packet gets here it means authentication enabled */
    /* Check is done in rip_read(). So, just skipping it */
    if (pPacket->version == RIPv2 &&
        pRte == pPacket->rte &&
        pRte->family == htons(RIP_FAMILY_AUTH))
    {
      continue;
    }

    if (pRte->family != htons(AF_INET))
    {
      /* Address family check.  RIP only supports AF_INET. */
      NNLOG(LOG_DEBUG,"Unsupported family %d from %s.\n",
            ntohs (pRte->family), inet_ntoa (pFrom->sin_addr));
      continue;
    }

    /* - is the destination address valid (e.g., unicast; not net 0
         or 127) */
    if (! ripDestinationCheck (pRte->prefix))
    {
      NNLOG(LOG_DEBUG,"Network is net 0 or net 127 or it is not unicast network\n");
      ripPeerBadRoute (pFrom);
      continue;
    } 

    /* Convert metric value to host byte order. */
    pRte->metric = ntohl (pRte->metric);

    /* - is the metric valid (i.e., between 1 and 16, inclusive) */
    if (! (pRte->metric >= 1 && pRte->metric <= 16))
	{
      NNLOG(LOG_DEBUG,"Route's metric is not in the 1-16 range.\n");
      ripPeerBadRoute (pFrom);
      continue;
    }

    /* RIPv1 does not have nexthop value. */
    if (pPacket->version == RIPv1 && pRte->nexthop.s_addr != 0)
    {
      NNLOG(LOG_DEBUG,"RIPv1 pPacket with nexthop value %s\n",
            inet_ntoa (pRte->nexthop));
      ripPeerBadRoute (pFrom);
      continue;
    }

    /* That is, if the provided information is ignored, a possibly
       sub-optimal, but absolutely valid, route may be taken.  If
       the received Next Hop is not directly reachable, it should be
       treated as 0.0.0.0. */
    if (pPacket->version == RIPv2 && pRte->nexthop.s_addr != 0)
    {
      Uint32T addrval;

      /* Multicast address check. */
      addrval = ntohl (pRte->nexthop.s_addr);
      if (IN_CLASSD (addrval))
      {
        NNLOG(LOG_DEBUG,"Nexthop %s is multicast address, skip this rte\n",
              inet_ntoa (pRte->nexthop));
        continue;
      }

      if (! ifLookupAddress (pRte->nexthop))
      {
        RouteNodeT *pRNode = NULL;
        RipInfoT *rinfo = NULL;

        pRNode = nnRouteNodeMatchIpv4 (pRip->pRipRibTable, &pRte->nexthop);

        if (pRNode)
        {
          rinfo = pRNode->pInfo;

          if (rinfo->type == RIB_ROUTE_TYPE_RIP && 
              rinfo->subType == RIP_ROUTE_RTE)
          {
            if (IS_RIP_DEBUG_EVENT)
              NNLOG(LOG_DEBUG,
                    "Next hop %s is on RIP network.  Set nexthop to the packet's originator\n", 
                    inet_ntoa (pRte->nexthop));
		      pRte->nexthop = rinfo->from;
          }
          else
          {
            if (IS_RIP_DEBUG_EVENT)
              NNLOG(LOG_DEBUG,
                    "Next hop %s is not directly reachable. Treat it as 0.0.0.0\n", 
                    inet_ntoa (pRte->nexthop));
            pRte->nexthop.s_addr = 0;
          }

		  nnRouteNodeUnlock (pRNode);
        }
        else
        {
          if (IS_RIP_DEBUG_EVENT)
            NNLOG(LOG_DEBUG,
                  "Next hop %s is not directly reachable. Treat it as 0.0.0.0\n", 
                  inet_ntoa (pRte->nexthop));
          pRte->nexthop.s_addr = 0;
        }

      }
    }

    /* For RIPv1, there won't be a valid netmask.  

       This is a best guess at the masks.  If everyone was using old
       Ciscos before the 'ip subnet zero' option, it would be almost
       right too :-)
      
       Cisco summarize ripv1 advertisments to the classful boundary
       (/16 for class B's) except when the RIP packet does to inside
       the classful network in question.  */

    if ((pPacket->version == RIPv1 && pRte->prefix.s_addr != 0) || 
        (pPacket->version == RIPv2 && 
        (pRte->prefix.s_addr != 0 && pRte->mask.s_addr == 0)))
    {
      Uint32T destination;

      if (subnetted == -1)
      {
        memcpy (&ifAddr, pIfc->pAddress, sizeof (Prefix4T));
        memcpy (&ifAddrClass, &ifAddr, sizeof (Prefix4T));
        nnApplyClassfulMaskIpv4 (&ifAddrClass);
        subnetted = 0;
        if (ifAddr.prefixLen > ifAddrClass.prefixLen)
          subnetted = 1;
      }

      destination = ntohl (pRte->prefix.s_addr);

      if (IN_CLASSA (destination))
        nnCnvMasklentoIp (8, &pRte->mask);
      else if (IN_CLASSB (destination))
        nnCnvMasklentoIp (16, &pRte->mask);
      else if (IN_CLASSC (destination))
        nnCnvMasklentoIp (24, &pRte->mask);

      if (subnetted == 1)
        nnCnvMasklentoIp (ifAddrClass.prefixLen, (struct in_addr *)&destination);

	  if ((subnetted == 1) && 
          ((pRte->prefix.s_addr & destination) == ifAddrClass.prefix.s_addr))
      {
        nnCnvMasklentoIp (ifAddr.prefixLen, &pRte->mask);
        if ((pRte->prefix.s_addr & pRte->mask.s_addr) != pRte->prefix.s_addr)
          nnCnvMasklentoIp (32, &pRte->mask);
        if (IS_RIP_DEBUG_EVENT)
          NNLOG(LOG_DEBUG,"Subnetted route %s\n", inet_ntoa (pRte->prefix));
      }
	  else
      {
        if ((pRte->prefix.s_addr & pRte->mask.s_addr) != pRte->prefix.s_addr)
          continue;
      }

      if (IS_RIP_DEBUG_EVENT)
      {
        NNLOG(LOG_DEBUG,"Resultant route %s\n", inet_ntoa (pRte->prefix));
        NNLOG(LOG_DEBUG,"Resultant mask %s\n", inet_ntoa (pRte->mask));
      }
    }

    /* In case of RIPv2, if prefix in RTE is not netmask applied one
       ignore the entry.  */
    if ((pPacket->version == RIPv2) && 
        (pRte->mask.s_addr != 0) && 
        ((pRte->prefix.s_addr & pRte->mask.s_addr) != pRte->prefix.s_addr))
    {
      NNLOG(LOG_DEBUG,"RIPv2 address %s is not mask /%d applied one\n",
            inet_ntoa (pRte->prefix), nnCnvIpmasktoLen (pRte->mask));
      ripPeerBadRoute (pFrom);
      continue;
    }

    /* Default route's netmask is ignored. */
    if (pPacket->version == RIPv2 && (pRte->prefix.s_addr == 0) && 
        (pRte->mask.s_addr != 0))
    {
      if (IS_RIP_DEBUG_EVENT)
        NNLOG(LOG_DEBUG,"Default route with non-zero netmask.  Set zero to netmask\n");
      pRte->mask.s_addr = 0;
    }
	  
    /* Routing table updates. */
    ripRteProcess (pRte, pFrom, pIfc->pIf);
  } /* for loop */

}

/* Make socket for RIP protocol. */
static Int32T 
ripCreateSocket (struct sockaddr_in *pFrom)
{
  Int32T ret = 0;
  Int32T sock = 0;
  struct sockaddr_in addr;

  memset (&addr, 0, sizeof (struct sockaddr_in));
  
  if (!pFrom)
  {
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
    addr.sin_len = sizeof (struct sockaddr_in);
#endif /* HAVE_STRUCT_SOCKADDR_IN_SIN_LEN */
  } 
  else 
  {
    memcpy(&addr, pFrom, sizeof(addr));
  }
  
  /* sending port must always be the RIP port */
  addr.sin_port = htons (RIP_PORT_DEFAULT);
  
  /* Make datagram socket. */
  sock = socket (AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) 
  {
    NNLOG(LOG_DEBUG,"Cannot create UDP socket: %s\n", errno);
	exit (1);
  }

  sockOptBroadcast (sock);
  sockOptReuseAddr (sock);
  sockOptReusePort (sock);
#ifdef RIP_RECVMSG
  setSockOptPktInfo (sock);
#endif /* RIP_RECVMSG */
#ifdef IPTOS_PREC_INTERNETCONTROL
  setSockOptIpv4Tos (sock, IPTOS_PREC_INTERNETCONTROL);
#endif

  setSockOptSoRecvBuf (sock, RIP_UDP_RCV_BUF);
  if ( (ret = bind (sock, (struct sockaddr *) & addr, sizeof (addr))) < 0) 
  {
    Int32T save_errno = errno;
      
    NNLOG(LOG_DEBUG,"%s: Can't bind socket %d to %s port %d: %s\n", __func__,
	       sock, inet_ntoa(addr.sin_addr), 
	       (Int32T) ntohs(addr.sin_port), 
	       save_errno);
      
    close (sock);

    return ret;
  }
  
  return sock;
}

/* RIP packet send to destination address, on interface denoted by
 * by connected argument. NULL to argument denotes destination should be
 * should be RIP multicast group
 */
static Int32T
ripSendPacket (Uint8T * pBuff, Int32T size, struct sockaddr_in * pTo,
               ConnectedT * pIfc)
{
  Int32T ret = 0, send_sock = 0;
  struct sockaddr_in sin = {0,};

  assert (pIfc != NULL);
  
  if (IS_RIP_DEBUG_PACKET)
  {
#define ADDRESS_SIZE 20
    char dst[ADDRESS_SIZE];
    dst[ADDRESS_SIZE - 1] = '\0';
      
    if (pTo)
    {
      strncpy (dst, inet_ntoa(pTo->sin_addr), ADDRESS_SIZE - 1);
    }
    else
    {
      sin.sin_addr.s_addr = htonl (INADDR_RIP_GROUP);
      strncpy (dst, inet_ntoa(sin.sin_addr), ADDRESS_SIZE - 1);
    }
#undef ADDRESS_SIZE
    NNLOG(LOG_DEBUG,"ripSendPacket %s > %s (%s)\n",
                inet_ntoa(pIfc->pAddress->u.prefix4),
                dst, pIfc->pIf->name);
  }
  
  if ( CHECK_FLAG (pIfc->flags, RIBMGR_IFA_SECONDARY) )
  {
    /*
     * RIBMGR_IFA_SECONDARY is set on linux when an interface is configured
     * with multiple addresses on the same subnet: the first address
     * on the subnet is configured "primary", and all subsequent addresses
     * on that subnet are treated as "secondary" addresses. 
     * In order to avoid routing-table bloat on other rip listeners, 
     * we do not send out RIP packets with RIBMGR_IFA_SECONDARY source addrs.
     * XXX Since Linux is the only system for which the RIBMGR_IFA_SECONDARY
     * flag is set, we would end up sending a packet for a "secondary"
     * source address on non-linux systems.  
     */
    if (IS_RIP_DEBUG_PACKET)
      NNLOG(LOG_DEBUG,"duplicate dropped\n");

    return 0;
  }

  /* Make destination address. */
  memset (&sin, 0, sizeof (struct sockaddr_in));
  sin.sin_family = AF_INET;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  sin.sin_len = sizeof (struct sockaddr_in);
#endif /* HAVE_STRUCT_SOCKADDR_IN_SIN_LEN */

  /* When destination is specified, use it's port and address. */
  if (pTo)
  {
      sin.sin_port = pTo->sin_port;
      sin.sin_addr = pTo->sin_addr;
      send_sock = pRip->sock;
  }
  else
  {
    struct sockaddr_in from;
      
    sin.sin_port = htons (RIP_PORT_DEFAULT);
    sin.sin_addr.s_addr = htonl (INADDR_RIP_GROUP);
      
    /* multicast send should bind to local interface address */
    from.sin_family = AF_INET;
    from.sin_port = htons (RIP_PORT_DEFAULT);
    from.sin_addr = pIfc->pAddress->u.prefix4;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
    from.sin_len = sizeof (struct sockaddr_in);
#endif /* HAVE_STRUCT_SOCKADDR_IN_SIN_LEN */
      
    /*
     * we have to open a new socket for each packet because this
     * is the most portable way to bind to a different source
     * ipv4 address for each packet. 
     */
    if ( (send_sock = ripCreateSocket (&from)) < 0)
    {
      NNLOG(LOG_DEBUG,"ripSendPacket could not create socket.\n");
      return -1;
    }
    ripInterfaceMulticastSet (send_sock, pIfc);
  }

  ret = sendto (send_sock, pBuff, size, 0, (struct sockaddr *)&sin,
                sizeof (struct sockaddr_in));

  if (IS_RIP_DEBUG_EVENT)
  {
      NNLOG(LOG_DEBUG,"SEND to  %s.%d\n", inet_ntoa(sin.sin_addr), 
            ntohs (sin.sin_port));
  }

  if (ret < 0)
  {
    NNLOG(LOG_DEBUG,"can't send packet : %d\n", strerror(errno));
  }

  if (!pTo)
    close(send_sock);

  return ret;
}

/* Add redistributed route to RIP table. */
void
ripRedistributeAdd (Int32T type, Int32T subType, Prefix4T * p, 
                    Uint32T ifIndex, struct in_addr * pNextHop,
                    Uint32T metric, unsigned char distance)
{
  Int32T ret = 0;
  RouteNodeT * pRNode = NULL;
  RipInfoT * pRInfo = NULL;

  /* Redistribute route  */
  ret = ripDestinationCheck (p->prefix);
  if (! ret)
  {
    return;
  }

  pRNode = nnRouteNodeGet (pRip->pRipRibTable, (PrefixT *) p);
  if (! pRNode)
  {
    NNLOG (LOG_ERR, "Error] Not exist pRnode, %s %d\n", __func__, __LINE__);
  }

  pRInfo = pRNode->pInfo;
  if (pRInfo)
  {
    if (pRInfo->type == RIB_ROUTE_TYPE_CONNECT && 
        pRInfo->subType == RIP_ROUTE_INTERFACE && 
        pRInfo->metric != RIP_METRIC_INFINITY)
	{
	  nnRouteNodeUnlock (pRNode);
	  return;
	}

    /* Manually configured RIP route check. */
    if (pRInfo->type == RIB_ROUTE_TYPE_RIP && 
        ((pRInfo->subType == RIP_ROUTE_STATIC) || 
         (pRInfo->subType == RIP_ROUTE_DEFAULT)) )
	{
	  if (type != RIB_ROUTE_TYPE_RIP || 
          ((subType != RIP_ROUTE_STATIC) && (subType != RIP_ROUTE_DEFAULT)))
	  {
	    nnRouteNodeUnlock (pRNode);
	    return;
	  }
	}

    if(pRInfo->pTimerTimeout)
    { 
      ripTimeoutTimerDelete (pRInfo);
    }

    if(pRInfo->pTimerGarbage)
    { 
      ripGabageTimerDelete (pRInfo);
    }

    if (ripRouteRte (pRInfo))
    {
	  ripRibmgrIpv4Delete ((Prefix4T *)&pRNode->p, &pRInfo->nexthop,
			               pRInfo->metric);
    }

    pRNode->pInfo = NULL;

    ripInfoFree (pRInfo);
      
    nnRouteNodeUnlock (pRNode);      
  }

  pRInfo = ripInfoNew ();
    
  pRInfo->type = type;
  pRInfo->subType = subType;
  pRInfo->ifIndex = ifIndex;
  pRInfo->metric = 1;
  pRInfo->externalMetric = metric;
  pRInfo->distance = distance;
  pRInfo->rp = pRNode;

  if (pNextHop)
    pRInfo->nexthop = *pNextHop;

  pRInfo->flags |= RIP_RTF_FIB;
  pRNode->pInfo = pRInfo;

  pRInfo->flags |= RIP_RTF_CHANGED;

  if (IS_RIP_DEBUG_EVENT) 
  {
    if (!pNextHop)
    {
      NNLOG(LOG_DEBUG,"Redistribute new prefix %s/%d on the interface %s\n",
                 inet_ntoa(p->prefix), p->prefixLen,
                 ifIndex2ifName(ifIndex));
    }
    else
    {
      NNLOG(LOG_DEBUG,"Redistribute new prefix %s/%d with nexthop %s on the interface %s\n",
                 inet_ntoa(p->prefix), p->prefixLen, inet_ntoa(pRInfo->nexthop),
                 ifIndex2ifName(ifIndex));
    }
  }

  /* replace triggered event */
  ripUpdateProcess (RIP_CHANGED_ROUTE);
  ripClearChangedFlag ();
}

/* Delete redistributed route from RIP table. */
void
ripRedistributeDelete (Int32T type, Int32T subType, Prefix4T *pPrefix4, 
                       Uint32T ifIndex)
{
  Int32T ret = 0;
  RouteNodeT * pRNode = NULL;
  RipInfoT * pRInfo = NULL;

  ret = ripDestinationCheck (pPrefix4->prefix);
  if (! ret)
    return;

  pRNode = nnRouteNodeLookup (pRip->pRipRibTable, (PrefixT *) pPrefix4);
  if (pRNode)
  {
    pRInfo = pRNode->pInfo;

    if (pRInfo != NULL && pRInfo->type == type && 
        pRInfo->subType == subType && pRInfo->ifIndex == ifIndex)
    {
      /* Perform poisoned reverse. */
      pRInfo->metric = RIP_METRIC_INFINITY;
      
      pRInfo->redistribute = RIP_REDISTRIBUTE_UNSET;

      ripGarbageTimerSet (pRInfo);
		            
      if(pRInfo->pTimerTimeout)
      {
        ripTimeoutTimerDelete (pRInfo);
      }

      pRInfo->flags |= RIP_RTF_CHANGED;

      if (IS_RIP_DEBUG_EVENT)
        NNLOG(LOG_DEBUG,"Poisone %s/%d on the interface %s with an infinity metric [delete]\n",
              inet_ntoa(pPrefix4->prefix), pPrefix4->prefixLen,
              ifIndex2ifName(ifIndex));

      ripUpdateProcess (RIP_CHANGED_ROUTE);
      ripClearChangedFlag ();
    }
  }

}

/* Response to request called from rip_read ().*/
static void
ripRequestProcess (RipPacketT * pPacket, Int32T size, 
                   struct sockaddr_in * pFrom, ConnectedT * pIfc)
{
  caddr_t lim = 0;
  RteT * pRte = NULL;
  Prefix4T p = {0,};
  RouteNodeT * pRNode = NULL;
  RipInfoT * pRInfo = NULL;
  RipInterfaceT * pRIf = NULL;

  /* Does not reponse to the requests on the loopback interfaces */
  if (ifIsLoopback (pIfc->pIf))
  {
    return;
  }

  /* Check RIP process is enabled on this interface. */
  pRIf = pIfc->pIf->pInfo;
  if (! pRIf->running)
  {
    return;
  }

  /* When passive interface is specified, suppress responses */
  if (pRIf->passive)
  {
    return;
  }
  
  /* RIP peer update. */
  ripPeerUpdate (pFrom, pPacket->version);

  lim = ((caddr_t) pPacket) + size;
  pRte = pPacket->rte;

  /* The Request is processed entry by entry.  If there are no
     entries, no response is given. */
  if (lim == (caddr_t) pRte)
  {
    return;
  }

  /* There is one special case.  If there is exactly one entry in the
     request, and it has an address family identifier of zero and a
     metric of infinity (i.e., 16), then this is a request to send the
     entire routing table. */
  if (lim == ((caddr_t) (pRte + 1)) &&
      ntohs (pRte->family) == 0 &&
      ntohl (pRte->metric) == RIP_METRIC_INFINITY)
  {	
   /* All route with split horizon */
    ripOutputProcess (pIfc, pFrom, RIP_ALL_ROUTE, pPacket->version);
  }
  else
  {
    /* Examine the list of RTEs in the Request one by one.  For each
       entry, look up the destination in the router's routing
       database and, if there is a route, put that route's metric in
       the metric field of the RTE.  If there is no explicit route
       to the specified destination, put infinity in the metric
       field.  Once all the entries have been filled in, change the
       command from Request to Response and send the datagram back
       to the requestor. */
    p.family = AF_INET;

    for (; ((caddr_t) pRte) < lim; pRte++)
    {
      p.prefix = pRte->prefix;
      p.prefixLen = nnCnvIpmasktoLen (pRte->mask);
      nnApplyNetmasktoPrefix4(&p, &p.prefixLen);
	  
      pRNode = nnRouteNodeLookup (pRip->pRipRibTable, (PrefixT *) &p);
      if (pRNode)
      {
        pRInfo = pRNode->pInfo;
        pRte->metric = htonl (pRInfo->metric);
        nnRouteNodeUnlock (pRNode);
      }
	  else
        pRte->metric = htonl (RIP_METRIC_INFINITY);
    }
    pPacket->command = RIP_RESPONSE;

    ripSendPacket ((Uint8T *)pPacket, size, pFrom, pIfc);
  }
  pRip->ripGlobalQueries++;
  
}

#if RIP_RECVMSG
/* Set IPv6 packet info to the socket. */
static Int32T
setSockOptPktInfo (Int32T sock)
{
  Int32T ret = 0;
  Int32T val = 1;

  ret = setsockopt(sock, IPPROTO_IP, IP_PKTINFO, &val, sizeof(val));
  if (ret < 0)
    NNLOG(LOG_DEBUG,"Can't setsockopt IP_PKTINFO : %s\n", errno);

  return ret;
}

/* Read RIP packet by recvmsg function. */
Int32T
ripRecvmsg (Int32T sock, Uint8T * pBuff, Int32T size, 
            struct sockaddr_in *pFrom, Int32T *ifIndex)
{
  Int32T ret = 0;
  struct msghdr msg = {0,};
  struct iovec iov = {0,};
  struct cmsghdr *ptr = NULL;
  char adata[RIP_BUFFER_MAX_SIZE]= {};

  msg.msg_name = (void *) pFrom;
  msg.msg_namelen = sizeof (struct sockaddr_in);
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = (void *) adata;
  msg.msg_controllen = sizeof adata;
  iov.iov_base = pBuff;
  iov.iov_len = size;

  ret = recvmsg (sock, &msg, 0);
  if (ret < 0)
    return ret;

  for (ptr = ZCMSG_FIRSTHDR(&msg); ptr != NULL; ptr = CMSG_NXTHDR(&msg, ptr))
    if (ptr->cmsg_level == IPPROTO_IP && ptr->cmsg_type == IP_PKTINFO) 
    {
      struct in_pktinfo *pktinfo;
      Int32T i;

      pktinfo = (struct in_pktinfo *) CMSG_DATA (ptr);
      i = pktinfo->ipi_ifindex;
    }

  return ret;
}
#endif /* RIP_RECVMSG */

/* First entry point of RIP packet. */
static void
ripPacketRead (Int32T fd, Int16T event, void * arg)
{
  Int32T ret = 0, rtenum = 0, len = 0, vrecv = 0;
  union RipBuf ripBuff;
  struct sockaddr_in from;
  socklen_t fromLen = 0;
  RipPacketT * pPacket = NULL;
  InterfaceT * pIf = NULL;
  ConnectedT * pIfc = NULL;
  RipInterfaceT * pRIf = NULL;

  if (IS_RIP_DEBUG_EVENT || IS_RIP_DEBUG_PACKET)
    NNLOG (LOG_DEBUG, "packet receive : %s \n", __func__);

  /* reset ripBuff */
  memset (&ripBuff, 0, sizeof(union RipBuf));

  /* RIPd manages only IPv4. */
  memset (&from, 0, sizeof (struct sockaddr_in));
  fromLen = sizeof (struct sockaddr_in);

  len = recvfrom (pRip->sock, (char *)&ripBuff.buf, sizeof (ripBuff.buf), 0, 
                  (struct sockaddr *) &from, &fromLen);
  if (len < 0)
  {
    NNLOG(LOG_DEBUG,"recvfrom failed: %s\n", errno);
    return;
  }

  /* Check is this packet comming from myself? */
  if (ifCheckAddress (from.sin_addr))
  {
    if (IS_RIP_DEBUG_PACKET)
      NNLOG(LOG_DEBUG,"ignore packet comes from myself.\n");

    return;
  }

  /* Which interface is this packet comes from. */
  pIf = ifLookupAddress (from.sin_addr);
  
  /* RIP packet received */
  if (IS_RIP_DEBUG_EVENT)
    NNLOG(LOG_DEBUG,"RECV packet from %s port %d on %s\n",
          inet_ntoa (from.sin_addr), ntohs (from.sin_port),
          pIf ? pIf->name : "unknown");

  /* If this packet come from unknown interface, ignore it. */
  if (pIf == NULL)
  {
    NNLOG(LOG_DEBUG,"ripPacketRead: cannot find interface for packet from %s port %d\n",
          inet_ntoa(from.sin_addr), ntohs (from.sin_port));

    return;
  }
  
  pIfc = connectedLookupAddress (pIf, from.sin_addr);
  
  if (pIfc == NULL)
  {
    NNLOG(LOG_DEBUG,"ripPacketRead: cannot find connected address for packet from %s "
          "port %d on interface %s\n",
          inet_ntoa(from.sin_addr), ntohs (from.sin_port), pIf->name);
    return;
  }

  /* Packet length check. */
  if (len < RIP_PACKET_MINSIZ)
  {
    NNLOG(LOG_DEBUG,"packet size %d is smaller than minimum size %d\n",
          len, RIP_PACKET_MINSIZ);

    ripPeerBadPacket (&from);

    return;
  }

  if (len > RIP_PACKET_MAXSIZ)
  {
    NNLOG(LOG_DEBUG,"packet size %d is larger than max size %d\n",
          len, RIP_PACKET_MAXSIZ);

    ripPeerBadPacket (&from);

    return;
  }

  /* Packet alignment check. */
  if ((len - RIP_PACKET_MINSIZ) % 20)
  {
    NNLOG(LOG_DEBUG,"packet size %d is wrong for RIP packet alignment\n", len);

    ripPeerBadPacket (&from);

    return;
  }

  /* Set RTE number. */
  rtenum = ((len - RIP_PACKET_MINSIZ) / 20);

  /* For easy to handle. */
  pPacket = &ripBuff.ripPacket;

  /* RIP version check. */
  if (pPacket->version == 0)
  {
    NNLOG(LOG_DEBUG,"version 0 with command %d received.\n", pPacket->command);

    ripPeerBadPacket (&from);

    return;
  }

  /* Dump RIP packet. */
  if (IS_RIP_DEBUG_RECV)
    ripPacketDump (pPacket, len, "RECV");

  /* RIP version adjust.  This code should rethink now.  RFC1058 says
     that "Version 1 implementations are to ignore this extra data and
     process only the fields specified in this document.". So RIPv3
     packet should be treated as RIPv1 ignoring must be zero field. */
  if (pPacket->version > RIPv2)
    pPacket->version = RIPv2;

  /* Is RIP running or is this RIP neighbor ?*/
  pRIf = pIf->pInfo;
  if (! pRIf->running && ! ripNeighborLookup (&from))
  {
    if (IS_RIP_DEBUG_EVENT)
      NNLOG(LOG_DEBUG,"RIP is not enabled on interface %s.\n", pIf->name);

    ripPeerBadPacket (&from);

    return;
  }

  /* RIP Version check. RFC2453, 4.6 and 5.1 */
  vrecv = ((pRIf->riReceive == RI_RIP_UNSPEC) ?  pRip->versionRecv : pRIf->riReceive);
  if ((pPacket->version == RIPv1) && !(vrecv & RIPv1))
  {
    if (IS_RIP_DEBUG_PACKET)
      NNLOG(LOG_DEBUG,"  packet's v%d doesn't fit to if version spec\n", 
            pPacket->version);

    ripPeerBadPacket (&from);

    return;
  }

  if ((pPacket->version == RIPv2) && !(vrecv & RIPv2))
  {
    if (IS_RIP_DEBUG_PACKET)
      NNLOG(LOG_DEBUG,"  packet's v%d doesn't fit to if version spec\n", 
            pPacket->version);
    ripPeerBadPacket (&from);

    return;
  }
  
  /* RFC2453 5.2 If the router is not configured to authenticate RIP-2
     messages, then RIP-1 and unauthenticated RIP-2 messages will be
     accepted; authenticated RIP-2 messages shall be discarded.  */
  if ((pRIf->authType == RIP_NO_AUTH)
      && rtenum 
      && (pPacket->version == RIPv2) 
      && (pPacket->rte->family == htons(RIP_FAMILY_AUTH)))
  {
    if (IS_RIP_DEBUG_EVENT)
      NNLOG(LOG_DEBUG,"packet RIPv%d is dropped because authentication disabled\n", 
            pPacket->version);

    ripPeerBadPacket (&from);

    return;
  }
  
  /* RFC:
     If the router is configured to authenticate RIP-2 messages, then
     RIP-1 messages and RIP-2 messages which pass authentication
     testing shall be accepted; unauthenticated and failed
     authentication RIP-2 messages shall be discarded.  For maximum
     security, RIP-1 messages should be ignored when authentication is
     in use (see section 4.1); otherwise, the routing information from
     authenticated messages will be propagated by RIP-1 routers in an
     unauthenticated manner. 
  */
  /* We make an exception for RIPv1 REQUEST packets, to which we'll
   * always reply regardless of authentication settings, because:
   *
   * - if there other authorised routers on-link, the REQUESTor can
   *   passively obtain the routing updates anyway
   * - if there are no other authorised routers on-link, RIP can
   *   easily be disabled for the link to prevent giving out information
   *   on state of this routers RIP routing table..
   *
   * I.e. if RIPv1 has any place anymore these days, it's as a very
   * simple way to distribute routing information (e.g. to embedded
   * hosts / appliances) and the ability to give out RIPv1
   * routing-information freely, while still requiring RIPv2
   * authentication for any RESPONSEs might be vaguely useful.
   */
  if (pRIf->authType != RIP_NO_AUTH 
      && pPacket->version == RIPv1)
  {
    /* Discard RIPv1 messages other than REQUESTs */
    if (pPacket->command != RIP_REQUEST)
    {
      if (IS_RIP_DEBUG_PACKET)
        NNLOG(LOG_DEBUG,"RIPv1" " dropped because authentication enabled\n");

      ripPeerBadPacket (&from);

      return;
    }
  }
  else if (pRIf->authType != RIP_NO_AUTH)
  {
    const char * authDesc;
      
    if (rtenum == 0)
    {
      /* There definitely is no authentication in the packet. */
      if (IS_RIP_DEBUG_PACKET)
        NNLOG(LOG_DEBUG,"RIPv2 authentication failed: no auth RTE in packet\n");
      ripPeerBadPacket (&from);
      return;
    }
      
    /* First RTE must be an Authentication Family RTE */
    if (pPacket->rte->family != htons(RIP_FAMILY_AUTH))
    {
      if (IS_RIP_DEBUG_PACKET)
        NNLOG(LOG_DEBUG,"RIPv2" " dropped because authentication enabled\n");

	  ripPeerBadPacket (&from);

	  return;
    }
      
    /* Check RIPv2 authentication. */
    switch (ntohs(pPacket->rte->tag))
    {
      case RIP_AUTH_SIMPLE_PASSWORD:
        authDesc = "simple";
        ret = ripAuthSimplePassword (pPacket->rte, &from, pIf);
        break;
          
      case RIP_AUTH_MD5:
        authDesc = "MD5";
        ret = ripAuthMd5 (pPacket, &from, len, pIf);
        /* Reset RIP packet length to trim MD5 data. */
        len = ret;
        break;
          
      default:
        ret = 0;
        authDesc = "unknown type";

      if (IS_RIP_DEBUG_PACKET)
        NNLOG(LOG_DEBUG,"RIPv2 Unknown authentication type %d\n", ntohs (pPacket->rte->tag));
    }
      
    if (ret)
    {
      if (IS_RIP_DEBUG_PACKET)
        NNLOG(LOG_DEBUG,"RIPv2 %s authentication success\n", authDesc);
    }
    else
    {
      if (IS_RIP_DEBUG_PACKET)
        NNLOG(LOG_DEBUG,"RIPv2 %s authentication failure\n", authDesc);
      ripPeerBadPacket (&from);
      return;
    }
  }
 
  /* Process each command. */
  switch (pPacket->command)
  {
    case RIP_RESPONSE:
      ripResponseProcess (pPacket, len, &from, pIfc);
      break;
    case RIP_REQUEST:
    case RIP_POLL:
      ripRequestProcess (pPacket, len, &from, pIfc);
      break;
    case RIP_TRACEON:
    case RIP_TRACEOFF:
      NNLOG(LOG_DEBUG,"Obsolete command %s received, please sent it to routed\n", 
		 lookupStr (ripMsg, pPacket->command));
      ripPeerBadPacket (&from);
      break;
    case RIP_POLL_ENTRY:
      NNLOG(LOG_DEBUG,"Obsolete command %s received\n", 
		 lookupStr (ripMsg, pPacket->command));
      ripPeerBadPacket (&from);
      break;
    default:
      NNLOG(LOG_DEBUG,"Unknown RIP command %d received\n", pPacket->command);
      ripPeerBadPacket (&from);
      break;
  }

  return;
}

/* Write routing table entry to the buffer and return next index of
   the routing table entry in the buffer. */
static Int32T
ripWriteRte (Int32T num, nnBufferT * buf, Prefix4T * p,
             Uint8T version, RipInfoT *pRInfo)
{
  struct in_addr mask;
  struct in_addr anyAddr;

  /* memset anyAddr to 0 */
  memset (&anyAddr, 0, sizeof(anyAddr));

  /* Write routing table entry. */
  if (version == RIPv1)
  {
    nnBufferSetInt16T (buf, AF_INET);
    nnBufferSetInt16T (buf, 0);
    nnBufferSetInaddr (buf, p->prefix);
    nnBufferSetInaddr (buf, anyAddr);
    nnBufferSetInaddr (buf, anyAddr);
    nnBufferSetInt32T (buf, pRInfo->metricOut);
  }
  else
  {
    nnCnvMasklentoIp (p->prefixLen, &mask);

    nnBufferSetInt16T (buf, AF_INET);
    nnBufferSetInt16T (buf, pRInfo->tagOut);
    nnBufferSetInaddr (buf, p->prefix);
    nnBufferSetInaddr (buf, mask);
    nnBufferSetInaddr (buf, pRInfo->nexthopOut);
    nnBufferSetInt32T (buf, pRInfo->metricOut);
  }

  return ++num;
}

/* Send update to the pIf or spcified neighbor. */
void
ripOutputProcess (ConnectedT * pIfc, struct sockaddr_in * pTo, 
                  Int32T routeType, Uint8T version)
{
  Int32T ret = 0, num = 0, rtemax =0, subnetted = 0;
  RouteNodeT * pRNode = NULL;
  RipInfoT * pRinfo = NULL;
  RipInterfaceT * pRIf = NULL;
  Prefix4T * p = NULL;
  Prefix4T classfull = {0,};
  Prefix4T ifAddrClass = {0,};
  KeyT * key = NULL;
  nnBufferT buf = {0,};

  /* this might need to made dynamic if RIP ever supported auth methods
     with larger key string sizes */
  char authStr[RIP_AUTH_SIMPLE_SIZE] = {};
  size_t doff = 0; /* offset of digest offset field */

  /* Logging output event. */
  if (IS_RIP_DEBUG_EVENT)
  {
    if (pTo)
    {
      NNLOG(LOG_DEBUG,"update routes to neighbor %s\n", inet_ntoa (pTo->sin_addr));
    }
    else
    {
      NNLOG(LOG_DEBUG,"update routes on interface %s ifindex %d\n",
            pIfc->pIf->name, pIfc->pIf->ifIndex);
    }
  }

  /* Reset buffer and RTE counter. */
  nnBufferReset(&buf);
  rtemax = (RIP_PACKET_MAXSIZ - 4) / 20;

  /* Get RIP interface. */
  pRIf = pIfc->pIf->pInfo;
    
  /* If output interface is in simple password authentication mode, we
     need space for authentication data.  */
  if (pRIf->authType == RIP_AUTH_SIMPLE_PASSWORD)
    rtemax -= 1;

  /* If output interface is in MD5 authentication mode, we need space
     for authentication header and data. */
  if (pRIf->authType == RIP_AUTH_MD5)
    rtemax -= 2;

  /* If output interface is in simple password authentication mode
     and string or keychain is specified we need space for auth. data */
  if (pRIf->authType != RIP_NO_AUTH)
  {
    if (pRIf->keyChain)
    {
      KeychainT *keychain;

      keychain = keychainLookup (pRIf->keyChain);
      if (keychain)
        key = keyLookupForSend (keychain);
    }
    /* to be passed to auth functions later */
    ripAuthPrepareStrSend (pRIf, key, authStr, RIP_AUTH_SIMPLE_SIZE);
  }

  if (version == RIPv1)
  {
    memcpy (&ifAddrClass, pIfc->pAddress, sizeof (Prefix4T));
    nnApplyClassfulMaskIpv4 (&ifAddrClass);
    subnetted = 0;
    if (pIfc->pAddress->prefixLen > ifAddrClass.prefixLen)
      subnetted = 1;
  }

  for (pRNode = nnRouteTop (pRip->pRipRibTable); pRNode; pRNode = nnRouteNext (pRNode))
    if ((pRinfo = pRNode->pInfo) != NULL)
    {
	/* For RIPv1, if we are subnetted, output subnets in our network    */
	/* that have the same mask as the output "interface". For other     */
	/* networks, only the classfull version is output.                  */
	
      if (version == RIPv1)
      {
        p = (Prefix4T *) &pRNode->p;

        if (IS_RIP_DEBUG_PACKET)
          NNLOG(LOG_DEBUG,"RIPv1 mask check, %s/%d considered for output\n",
                inet_ntoa (pRNode->p.u.prefix4), pRNode->p.prefixLen);

        if (subnetted &&
            nnPrefixMatch ((PrefixT *) &ifAddrClass, &pRNode->p))
        {
          if ((pIfc->pAddress->prefixLen != pRNode->p.prefixLen) &&
              (pRNode->p.prefixLen != 32))
          {
            continue;
          }
        }
        else
        {
          memcpy (&classfull, &pRNode->p, sizeof(Prefix4T));
          nnApplyClassfulMaskIpv4(&classfull);
          if (pRNode->p.u.prefix4.s_addr != 0 &&
              classfull.prefixLen != pRNode->p.prefixLen)
          {
            continue;
          }
        }
        if (IS_RIP_DEBUG_PACKET)
          NNLOG(LOG_DEBUG,"RIPv1 mask check, %s/%d made it through\n",
                inet_ntoa (pRNode->p.u.prefix4), pRNode->p.prefixLen);
      }
      else 
        p = (Prefix4T *) &pRNode->p;


      /* Apply output filters. */
      ret = ripOutgoingFilter (p, pRIf);
      if (ret < 0)
      {
        continue;
      }

      /* Changed route only output. */
      if (routeType == RIP_CHANGED_ROUTE &&
         (! (pRinfo->flags & RIP_RTF_CHANGED)))
      {
        continue;
      }

      /* Split horizon. */
      if (pRIf->splitHorizon == RIP_SPLIT_HORIZON)
      {
        /* 
         * We perform split horizon for RIP and connected route. 
         * For rip routes, we want to suppress the route if we would
         * end up sending the route back on the interface that we
         * learned it from, with a higher metric. For connected routes,
         * we suppress the route if the prefix is a subset of the
         * source address that we are going to use for the packet 
         * (in order to handle the case when multiple subnets are
         * configured on the same interface).
         */
        if (pRinfo->type == RIB_ROUTE_TYPE_RIP  &&
            pRinfo->ifIndex == pIfc->pIf->ifIndex) 
        {
          continue;
        }
        if (pRinfo->type == RIB_ROUTE_TYPE_CONNECT &&
            nnPrefixMatch((PrefixT *)p, pIfc->pAddress))
        {
          continue;
        }
      }

      /* Preparation for route-map. */
      pRinfo->metricSet = 0;
      pRinfo->nexthopOut.s_addr = 0;
      pRinfo->metricOut = pRinfo->metric;
      pRinfo->tagOut = pRinfo->tag;
      pRinfo->ifIndexOut = pIfc->pIf->ifIndex;

      /* In order to avoid some local loops,
       * if the RIP route has a nexthop via this interface, keep the nexthop,
       * otherwise set it to 0. The nexthop should not be propagated
       * beyond the local broadcast/multicast area in order
       * to avoid an IGP multi-level recursive look-up.
       * see (4.4)
       */
      if (pRinfo->ifIndex == pIfc->pIf->ifIndex)
        pRinfo->nexthopOut = pRinfo->nexthop;

      /* Interface route-map */
      if (pRIf->routeMap[RIP_FILTER_OUT])
      {
        ret = routeMapApply (pRIf->routeMap[RIP_FILTER_OUT], 
                             (PrefixT *) p, RMAP_RIP, pRinfo);

        if (ret == RMAP_DENYMATCH)
        {
          if (IS_RIP_DEBUG_PACKET)
            NNLOG(LOG_DEBUG,"RIP %s/%d is filtered by route-map out\n",
                  inet_ntoa (p->prefix), p->prefixLen);
		  continue;
        }
      }
           
      /* Apply redistribute route map - continue, if deny */
      if (pRip->routeMap[pRinfo->type].name && 
          pRinfo->subType != RIP_ROUTE_INTERFACE)
      {
        ret = routeMapApply (pRip->routeMap[pRinfo->type].map,
                             (PrefixT *)p, RMAP_RIP, pRinfo);

        if (ret == RMAP_DENYMATCH) 
        {
          if (IS_RIP_DEBUG_PACKET)
            NNLOG(LOG_DEBUG,"%s/%d is filtered by route-map\n",
                  inet_ntoa (p->prefix), p->prefixLen);
          continue;
        }
      }

      /* When route-map does not set metric. */
      if (! pRinfo->metricSet)
      {
        /* If redistribute metric is set. */
        if (pRip->routeMap[pRinfo->type].metricConfig && 
            pRinfo->metric != RIP_METRIC_INFINITY)
        {
          pRinfo->metricOut = pRip->routeMap[pRinfo->type].metric;
        }
        else
        {
          /* If the route is not connected or localy generated
             one, use default-metric value*/
          if (pRinfo->type != RIB_ROUTE_TYPE_RIP && 
              pRinfo->type != RIB_ROUTE_TYPE_CONNECT && 
              pRinfo->metric != RIP_METRIC_INFINITY)
          pRinfo->metricOut = pRip->defaultMetric;
        }
      }

      /* Apply offset-list */
      if (pRinfo->metric != RIP_METRIC_INFINITY)
        ripOffsetListApplyOut (p, pIfc->pIf, &pRinfo->metricOut);

      if (pRinfo->metricOut > RIP_METRIC_INFINITY)
        pRinfo->metricOut = RIP_METRIC_INFINITY;

      /* Perform split-horizon with poisoned reverse 
       * for RIP and connected routes.
       */
      if (pRIf->splitHorizon == RIP_SPLIT_HORIZON_POISONED_REVERSE) 
      {
        /* 
        * We perform split horizon for RIP and connected route. 
        * For rip routes, we want to suppress the route if we would
        * end up sending the route back on the interface that we
        * learned it from, with a higher metric. For connected routes,
        * we suppress the route if the prefix is a subset of the
        * source address that we are going to use for the packet 
        * (in order to handle the case when multiple subnets are
        * configured on the same interface).
        */
        if (pRinfo->type == RIB_ROUTE_TYPE_RIP  &&
            pRinfo->ifIndex == pIfc->pIf->ifIndex)
          pRinfo->metricOut = RIP_METRIC_INFINITY;

        if (pRinfo->type == RIB_ROUTE_TYPE_CONNECT &&
            nnPrefixMatch((PrefixT *)p, pIfc->pAddress))
          pRinfo->metricOut = RIP_METRIC_INFINITY;
      }
	
      /* Prepare preamble, auth headers, if needs be */
      if (num == 0)
      {
        nnBufferSetInt8T  (&buf, RIP_RESPONSE);
        nnBufferSetInt8T  (&buf, version);
        nnBufferSetInt16T (&buf, 0);
	    
        /* auth header for !v1 && !no_auth */
        if ( (pRIf->authType != RIP_NO_AUTH) && (version != RIPv1) )
          doff = ripAuthHeaderWrite (&buf, pRIf, key, authStr, 
                                     RIP_AUTH_SIMPLE_SIZE);
      }
        
      /* Write RTE to the buffer. */
      num = ripWriteRte (num, &buf, p, version, pRinfo);
      if (num == rtemax)
      {
        if (version == RIPv2 && pRIf->authType == RIP_AUTH_MD5)
          ripAuthMd5Set (&buf, pRIf, doff, authStr, RIP_AUTH_SIMPLE_SIZE);

        ret = ripSendPacket ((Uint8T *)buf.data, buf.length, pTo, pIfc);

        if (ret >= 0 && IS_RIP_DEBUG_SEND)
          ripPacketDump ((RipPacketT *)buf.data, buf.length, "SEND");
        num = 0;
        nnBufferReset(&buf);
      }

    }

  /* Flush unwritten RTE. */
  if (num != 0)
  {
    if (version == RIPv2 && pRIf->authType == RIP_AUTH_MD5)
      ripAuthMd5Set (&buf, pRIf, doff, authStr, RIP_AUTH_SIMPLE_SIZE);

    ret = ripSendPacket ((Uint8T *)buf.data, buf.length, pTo, pIfc);

    if (ret >= 0 && IS_RIP_DEBUG_SEND)
      ripPacketDump ((RipPacketT *)buf.data, buf.length, "SEND");

    num = 0;
    nnBufferReset (&buf);
  }

  /* Statistics updates. */
  pRIf->sentUpdates++;

}

/* Send RIP packet to the interface. */
static void
ripUpdateInterface (ConnectedT *pIfc, Uint8T version, Int32T routeType)
{
  struct sockaddr_in to;

  /* When RIP version is 2 and multicast enable interface. */
  if (version == RIPv2 && ifIsMulticast (pIfc->pIf)) 
  {
    if (IS_RIP_DEBUG_EVENT)
	  NNLOG(LOG_DEBUG,"multicast announce on %s \n", pIfc->pIf->name);

    ripOutputProcess (pIfc, NULL, routeType, version);

    return;
  }
  
  /* If we can't send multicast packet, send it with unicast. */
  if (ifIsBroadcast (pIfc->pIf) || ifIsPointPoint (pIfc->pIf))
  {
    if (pIfc->pAddress->family == AF_INET)
    {
      /* Destination address and port setting. */
      memset (&to, 0, sizeof (struct sockaddr_in));
      if (pIfc->pDestination)
        /* use specified broadcast or peer destination addr */
        to.sin_addr = pIfc->pDestination->u.prefix4;
      else if (pIfc->pAddress->prefixLen < NN_IPV4_MAX_PREFIXLEN)
      {
        /* calculate the appropriate broadcast address */
        nnGetBroadcastfromAddr(&to.sin_addr, 
                               &pIfc->pAddress->u.prefix4,
                               &pIfc->pAddress->prefixLen);
      }
      else
      {
        /* do not know where to send the packet */
        return;
      }
      to.sin_port = htons (RIP_PORT_DEFAULT);

      if (IS_RIP_DEBUG_EVENT)
        NNLOG(LOG_DEBUG,"%s announce to %s on %s\n",
              CONNECTED_PEER(pIfc) ? "unicast" : "broadcast",
              inet_ntoa (to.sin_addr), pIfc->pIf->name);

      ripOutputProcess (pIfc, &to, routeType, version);
    }
  }

}

/* Update send to all interface and neighbor. */
void
ripUpdateProcess (Int32T routeType)
{
  ListNodeT * pNode = NULL;
  ConnectedT * pConnected = NULL;
  InterfaceT * pIf = NULL;
  Prefix4T * p = NULL;

  RipInterfaceT * pRIf;
  RouteNodeT * pRNode;
  struct sockaddr_in to;

  /* Send RIP update to each interface. */
  for(pNode = pRip->pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pIf = pNode->pData;

    if (ifIsLoopback (pIf))
      continue;

    if (! ifIsOperative (pIf))
      continue;

    /* Fetch RIP interface information. */
    pRIf = pIf->pInfo;

    /* When passive interface is specified, suppress announce to the
       interface. */
    if (pRIf->passive)
      continue;

    if (pRIf->running)
	{
      /* 
       * If there is no version configuration in the interface,
       * use rip's version setting. 
       */
      Int32T vsend = ((pRIf->riSend == RI_RIP_UNSPEC) ?
                   pRip->versionSend : pRIf->riSend);

      if (IS_RIP_DEBUG_EVENT) 
        NNLOG(LOG_DEBUG,"SEND UPDATE to %s ifindex %d\n",
              (pIf->name ? pIf->name : "_unknown_"), pIf->ifIndex);

      /* send update on each connected network */
      ListT * pConnectedList = pIf->pConnected;
      ListNodeT * pConnectedNode;
      for(pConnectedNode = pConnectedList->pHead;
          pConnectedNode != NULL;
          pConnectedNode = pConnectedNode->pNext)
      {
        pConnected = pConnectedNode->pData;
        if (pConnected->pAddress->family == AF_INET)
        {
          if (vsend & RIPv1)
          {
            ripUpdateInterface (pConnected, RIPv1, routeType);
          }
          if ((vsend & RIPv2) && ifIsMulticast(pIf))
          {
            ripUpdateInterface (pConnected, RIPv2, routeType);
          }
        }
      }
    }
  }

  /* RIP send updates to each neighbor. */
  for (pRNode = nnRouteTop (pRip->pNeighbor); 
       pRNode; 
       pRNode = nnRouteNext (pRNode))
  {
    if (pRNode->pInfo != NULL)
    {
      p = (Prefix4T *) &pRNode->p;

      pIf = ifLookupAddress (p->prefix);
      if (! pIf)
      {
        NNLOG(LOG_DEBUG,"Neighbor %s doesnt have connected interface!\n",
              inet_ntoa (p->prefix));
        continue;
      }
        
      if ( (pConnected = connectedLookupAddress (pIf, p->prefix)) == NULL)
      {
        NNLOG(LOG_DEBUG,"Neighbor %s doesnt have connected network\n",
              inet_ntoa (p->prefix));
        continue;
      }
        
      /* Set destination address and port */
      memset (&to, 0, sizeof (struct sockaddr_in));
      to.sin_addr = p->prefix;
      to.sin_port = htons (RIP_PORT_DEFAULT);

      /* RIP version is rip's configuration. */
      ripOutputProcess (pConnected, &to, routeType, pRip->versionSend);
    }
  }
}

/* Walk down the RIP routing table then clear changed flag. */
void
ripClearChangedFlag (void)
{
  RouteNodeT *pRNode = NULL;
  RipInfoT *rinfo = NULL;

  for (pRNode = nnRouteTop (pRip->pRipRibTable); 
       pRNode; 
       pRNode = nnRouteNext (pRNode))
  {
    if ((rinfo = pRNode->pInfo) != NULL)
      if (rinfo->flags & RIP_RTF_CHANGED)
        rinfo->flags &= ~RIP_RTF_CHANGED;
  }
}

/* Withdraw redistributed route. */
void
ripRedistributeWithdraw (Int32T type)
{
  RouteNodeT *pRNode = NULL;
  RipInfoT *pRInfo = NULL;

  if (!pRip)
  {
    return;
  }

  if (!pRip->pRipRibTable)
  {
    return;
  }

  for (pRNode = nnRouteTop (pRip->pRipRibTable); 
       pRNode; 
       pRNode = nnRouteNext (pRNode))
  {
    if ((pRInfo = pRNode->pInfo) != NULL)
    {
      if (pRInfo->type == type && 
          pRInfo->subType != RIP_ROUTE_INTERFACE)
      {
        /* Perform poisoned reverse. */
        pRInfo->metric = RIP_METRIC_INFINITY;

        ripGarbageTimerSet (pRInfo);
			            
        if(pRInfo->pTimerTimeout)
        {
          ripTimeoutTimerDelete (pRInfo);
        }

        pRInfo->flags |= RIP_RTF_CHANGED;

        if (IS_RIP_DEBUG_EVENT) 
        {
          Prefix4T *p = (Prefix4T *) &pRNode->p;
          NNLOG(LOG_DEBUG,"Poisone %s/%d on the interface %s with an infinity metric [withdraw]\n",
                inet_ntoa(p->prefix), p->prefixLen, ifIndex2ifName(pRInfo->ifIndex));
        }

        ripUpdateProcess (RIP_CHANGED_ROUTE);
        ripClearChangedFlag ();
      }
    }
  }

}

/* Create new RIP instance and set it to global variable. */
Int32T
ripCreate (void)
{
  if (!pRip)
  {
    pRip = NNMALLOC (MEM_GLOBAL, sizeof (RipT));
  }

  /* Set initial values. */
  pRip->versionSend = RI_RIP_VERSION_2;
  pRip->versionRecv = RI_RIP_VERSION_1_AND_2;
  pRip->defaultMetric = RIP_DEFAULT_METRIC_DEFAULT;

  /* Set timeval values. */
  pRip->tvTriggeredTime.tv_sec = RIP_TRIGGERED_TIMER_DEFAULT;
  pRip->tvUpdateTime.tv_sec = RIP_UPDATE_TIMER_DEFAULT;
  pRip->tvTimeoutTime.tv_sec = RIP_TIMEOUT_TIMER_DEFAULT;
  pRip->tvGarbageTime.tv_sec = RIP_GARBAGE_TIMER_DEFAULT;

  /* Initialize RIP routig table. */
  pRip->pRipRibTable = nnRouteTableInit (); /* for operation route*/
  pRip->pStaticRoute = nnRouteTableInit (); /* for configuration route*/
  pRip->pNeighbor = nnRouteTableInit (); /* for neighbor */

  /* Make socket. */
  pRip->sock = ripCreateSocket (NULL);
  if (pRip->sock < 0)
  {
    return pRip->sock;
  }

  /* Create read and timer thread. */
  pRip->pSockFdEvent = taskFdSet(ripPacketRead, pRip->sock, 
                          TASK_READ | TASK_PERSIST, TASK_PRI_MIDDLE, NULL);

  return 0;
}

/* Sned RIP request to the destination. */
Int32T
ripRequestSend (struct sockaddr_in *pTo, InterfaceT * pIf,
                  Uint8T version, ConnectedT *pConnected)
{
  RteT *pRte = NULL;
  RipPacketT ripPacket = {0,};

  memset (&ripPacket, 0, sizeof (ripPacket));

  ripPacket.command = RIP_REQUEST;
  ripPacket.version = version;
  pRte = ripPacket.rte;
  pRte->metric = htonl (RIP_METRIC_INFINITY);

  if (pConnected)
  {
    /* 
     * connected is only sent for ripv1 case, or when
     * interface does not support multicast.  Caller loops
     * over each connected address for this case.
     */
    if (ripSendPacket ((Uint8T *) &ripPacket, sizeof (ripPacket), 
                         pTo, pConnected) != sizeof (ripPacket))
      return -1;
    else
      return sizeof (ripPacket);
  }

  /* send request on each connected network */
  ListT * pConnectedList = pIf->pConnected;
  ListNodeT * pConnectedNode;

  for(pConnectedNode = pConnectedList->pHead;
      pConnectedNode != NULL;
      pConnectedNode = pConnectedNode->pNext)
  {
    pConnected = pConnectedNode->pData;

    Prefix4T *p;
    p = (Prefix4T *) pConnected->pAddress;

    if (p->family != AF_INET)
      continue;

    if (ripSendPacket ((Uint8T *) &ripPacket, sizeof (ripPacket), 
                         pTo, pConnected) != sizeof (ripPacket))
      return -1;
  }

  return sizeof (ripPacket);
}


static void
ripUpdateDefaultMetric (void)
{
  RouteNodeT * pRNode = NULL;
  RipInfoT * pRInfo = NULL;

  for (pRNode = nnRouteTop (pRip->pRipRibTable); 
       pRNode; 
       pRNode = nnRouteNext (pRNode))
  {
    if ((pRInfo = pRNode->pInfo) != NULL)
      if (pRInfo->type != RIB_ROUTE_TYPE_RIP && 
          pRInfo->type != RIB_ROUTE_TYPE_CONNECT)
        pRInfo->metric = pRip->defaultMetric;
  }
}


struct ripDistance
{
  /* Distance value for the IP source prefix. */
  Uint8T distance;

  /* Name of the access-list to be matched. */
  StringT accessListName;
};
typedef struct ripDistance RipDistanceT;

RipDistanceT *
ripDistanceNew (void)
{
  return NNMALLOC (MEM_RIP_DISTANCE, sizeof (RipDistanceT));
}

void
ripDistanceFree (RipDistanceT *pRDistance)
{
  assert (pRDistance);

  NNFREE (MEM_RIP_DISTANCE, pRDistance);
  pRDistance = NULL;
}

Int32T
ripDistanceSet (Uint8T distance, PrefixT * pPrefix,
                const StringT strAccessList)
{
  RouteNodeT * pRNode = NULL;
  RipDistanceT * pRDistance = NULL;

  /* Get RIP distance node. */
  pRNode = nnRouteNodeGet (pRip->pRipDistanceTable, pPrefix);
  if (pRNode->pInfo)
  {
    pRDistance = pRNode->pInfo;
    nnRouteNodeUnlock (pRNode);
  }
  else
  {
    pRDistance = ripDistanceNew ();
    pRNode->pInfo = pRDistance;
  }

  /* Set distance value. */
  pRDistance->distance = distance;

  /* Reset access-list configuration. */
  if (pRDistance->accessListName)
  {
    NNFREE (MEM_ACCESSLIST, pRDistance->accessListName);
    pRDistance->accessListName = NULL;
  }
  if (strAccessList)
    pRDistance->accessListName = nnStrDup (strAccessList, MEM_ACCESSLIST);

  return SUCCESS;
}


Int32T
ripDistanceUnset (Uint8T distance,
		          PrefixT * pPrefix, const StringT strAccessList)
{
  RouteNodeT * pRNode = NULL;
  RipDistanceT * pRDistance = NULL;

  pRNode = nnRouteNodeLookup (pRip->pRipDistanceTable, pPrefix);
  if (! pRNode)
  {
      NNLOG (LOG_ERR, "Error] Can't find specified prefix !!!\n");
      return FAILURE;
  }

  pRDistance = pRNode->pInfo;

  if (pRDistance->accessListName)
  {
    NNFREE (MEM_ACCESSLIST, pRDistance->accessListName);
    pRDistance->accessListName = NULL;
  }
  ripDistanceFree (pRDistance);

  pRNode->pInfo = NULL;
  nnRouteNodeUnlock (pRNode);
  nnRouteNodeUnlock (pRNode);

  return SUCCESS;
}


static void
ripDistanceReset (void)
{
  RouteNodeT * pRNode = NULL;
  RipDistanceT * pRDistance = NULL;

  if (!pRip)
    return;

  if (!pRip->pRipDistanceTable)
    return;

  for (pRNode = nnRouteTop (pRip->pRipDistanceTable); 
       pRNode; 
       pRNode = nnRouteNext (pRNode))
  {
    if ((pRDistance = pRNode->pInfo) != NULL)
    {
	  if (pRDistance->accessListName)
      {
	    NNFREE (MEM_ACCESSLIST, pRDistance->accessListName);
        pRDistance->accessListName = NULL;
      }
      ripDistanceFree (pRDistance);
      pRNode->pInfo = NULL;
      nnRouteNodeUnlock (pRNode);
    }
  }

  NNFREE (MEM_ROUTE_TABLE, pRip->pRipDistanceTable);
  pRip->pRipDistanceTable = NULL;
}

/* Apply RIP information to distance method. */
Uint8T
ripDistanceApply (RipInfoT *pRInfo)
{
  RouteNodeT * pRNode = NULL;
  Prefix4T p = {0,};
  RipDistanceT * pRDistance = NULL;
  AccessListT * alist = NULL;

  if (! pRip)
    return 0;

  memset (&p, 0, sizeof (Prefix4T));
  p.family = AF_INET;
  p.prefix = pRInfo->from;
  p.prefixLen = NN_IPV4_MAX_BITLEN;

  /* Check source address. */
  pRNode = nnRouteNodeMatch (pRip->pRipDistanceTable, (PrefixT *) &p);
  if (pRNode)
  {
    pRDistance = pRNode->pInfo;
    nnRouteNodeUnlock (pRNode);

    if (pRDistance->accessListName)
    {
      alist = accessListLookup (AFI_IP, pRDistance->accessListName);
      if (alist == NULL)
      {
        return 0;
      }
      if (accessListApply (alist, &pRInfo->rp->p) == FILTER_DENY)
      {
        return 0;
      }
      return pRDistance->distance;
    }
    else
    {
      return pRDistance->distance;
    }
  }

  if (pRip->distance)
  {
      return pRip->distance;
  }

  return 0;
}

void
ripDistanceDisplay (struct cmsh * cmsh)
{
  RouteNodeT * pRNode = NULL;
  RipDistanceT * pRDistance = NULL;
  Int32T header = 1;
  char buf[BUFSIZ]={};

  cmdPrint (cmsh, "  Distance: (default is %d)\n",
	        pRip->distance ? pRip->distance :RIB_DISTANCE_DEFAULT_RIP);

  for (pRNode = nnRouteTop (pRip->pRipDistanceTable); 
       pRNode; 
       pRNode = nnRouteNext (pRNode))
    if ((pRDistance = pRNode->pInfo) != NULL)
    {
      if (header)
      {
        cmdPrint (cmsh, "    Address           Distance  List\n");
	    header = 0;
      }
      sprintf(buf, "%s/%d", inet_ntoa (pRNode->p.u.prefix4), pRNode->p.prefixLen);
      cmdPrint (cmsh, "    %-20s  %4d  %s\n",
		 buf, pRDistance->distance,
		 pRDistance->accessListName ? pRDistance->accessListName : "");
    }
}

/* Print out update timer remain time. */
Int32T
ripCmshUpdateTimerRemain ()
{
  Int32T remainTime = 0;
  struct timeval tvNow = {0,};

  /* Get local time. */
  gettimeofday (&tvNow, NULL);
  
  /* Calculate update remain time. */
  remainTime = pRip->tvUpdatedTime.tv_sec + 30 - tvNow.tv_sec;

  return remainTime;
   
}

/* Calculate timeout timer remain time. */
Int32T
ripCheckTimeoutHalfTime (RipInfoT * pRInfo)
{
  struct timeval tvNow = {0,};

  /* Get local time. */
  gettimeofday(&tvNow, NULL);

  /* Check this is over timeout half time. */
  if (tvNow.tv_sec > 
      pRInfo->tvUpdated.tv_sec + (pRip->tvTimeoutTime.tv_sec / 2))
  {
    return TRUE;
  }

  return FALSE;
}

/* Print out routes update time. */
Int32T
ripCmshUptime (char *strBuff, Int32T idx, RipInfoT *pRInfo)
{
  time_t diffTime = 0;
  struct tm *tm = NULL;
  struct timeval tvNow = {0,};
#define TIME_BUF 25
  char timeBuff [TIME_BUF] = {};

  if (pRInfo->pTimerTimeout != NULL)
  {
    gettimeofday (&tvNow, NULL);
    diffTime = 
       pRInfo->tvUpdated.tv_sec + pRip->tvTimeoutTime.tv_sec - tvNow.tv_sec;
    tm = gmtime (&diffTime);
    strftime (timeBuff, TIME_BUF, "%M:%S", tm);

    return sprintf (strBuff + idx, "%5s", timeBuff);
  }
  else if (pRInfo->pTimerGarbage != NULL)
  {
    gettimeofday (&tvNow, NULL);
    diffTime = 
       pRInfo->tvUpdated.tv_sec + pRip->tvGarbageTime.tv_sec - tvNow.tv_sec;

    tm = gmtime (&diffTime);
    strftime (timeBuff, TIME_BUF, "%M:%S", tm);

    return sprintf (strBuff + idx, "%5s", timeBuff);
  }
  return 0;
}


const StringT
ripRouteTypePrint (Int32T subType)
{
  switch (subType)
    {
      case RIP_ROUTE_RTE:
	return "n";
      case RIP_ROUTE_STATIC:
	return "s";
      case RIP_ROUTE_DEFAULT:
	return "d";
      case RIP_ROUTE_REDISTRIBUTE:
	return "r";
      case RIP_ROUTE_INTERFACE:
	return "i";
      default:
	return "?";
    }
}


/* RIP configuration write function. */
Int32T
configWriteRip (struct cmsh *cmsh)
{
  RouteNodeT *pRNode = NULL;
  RipDistanceT *pRDistance = NULL;

  if (!pRip)
  {
    NNLOG(LOG_ERR, "Not installed rip instance.\n");
    return FAILURE;
  }

  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    NNLOG(LOG_ERR, "Unset rip instance.\n");
    return FAILURE;
  }

  /* Router RIP statement. */
  cmdPrint (cmsh, "!");

  /* Router RIP statement. */
  cmdPrint (cmsh, "router rip");
  
  /* RIP version statement.  Default is RIP version 2. */
  if (pRip->versionSend != RI_RIP_VERSION_2 || 
      pRip->versionRecv != RI_RIP_VERSION_1_AND_2)
  {
	cmdPrint (cmsh, " version %d", pRip->versionSend);
  }
 
  /* RIP timer configuration. */
  if (pRip->tvUpdateTime.tv_sec != RIP_UPDATE_TIMER_DEFAULT || 
      pRip->tvTimeoutTime.tv_sec != RIP_TIMEOUT_TIMER_DEFAULT || 
      pRip->tvGarbageTime.tv_sec != RIP_GARBAGE_TIMER_DEFAULT)
  {
    cmdPrint (cmsh, " timers basic %lu %lu %lu",
		 pRip->tvUpdateTime.tv_sec,
		 pRip->tvTimeoutTime.tv_sec,
		 pRip->tvGarbageTime.tv_sec);
  }

  /* Default information configuration. */
  if (pRip->defaultInformation)
  {
    if (pRip->defaultInformationRouteMap)
    {
      cmdPrint (cmsh, " default-information originate route-map %s",
                pRip->defaultInformationRouteMap);
    }
    else
    {
      cmdPrint (cmsh, " default-information originate");
    }
  }

  /* Redistribute configuration. */
  configWriteRipRedistribute (cmsh, 1);

  /* RIP offset-list configuration. */
  configWriteRipOffsetList (cmsh);

  /* RIP enabled network and interface configuration. */
  configWriteRipNetwork (cmsh, 1);
			
  /* RIP default metric configuration */
  if (pRip->defaultMetric != RIP_DEFAULT_METRIC_DEFAULT)
  {
    cmdPrint (cmsh, " default-metric %d\n", pRip->defaultMetric);
  }

  /* Distribute configuration. */
  configWriteDistribute (cmsh);

  /* Interface routemap configuration */
  configWriteIfRouteMap (cmsh);

  /* Distance configuration. */
  if (pRip->distance)
  {
    cmdPrint (cmsh, " distance %d", pRip->distance);
  }

  /* RIP source IP prefix distance configuration. */
  for (pRNode = nnRouteTop (pRip->pRipDistanceTable); 
       pRNode; 
       pRNode = nnRouteNext (pRNode))
  {
    if ((pRDistance = pRNode->pInfo) != NULL)
    {
      cmdPrint (cmsh, " distance %d %s/%d %s", pRDistance->distance,
               inet_ntoa (pRNode->p.u.prefix4), pRNode->p.prefixLen,
               pRDistance->accessListName ? pRDistance->accessListName : "");
    }
  }

  /* RIP static route configuration. */
  if (pRip->pStaticRoute)
    for (pRNode = nnRouteTop (pRip->pStaticRoute); 
         pRNode; 
         pRNode = nnRouteNext (pRNode))
    {
      if (pRNode->pInfo)
      {
        cmdPrint (cmsh, " route %s/%d", 
                  inet_ntoa (pRNode->p.u.prefix4), pRNode->p.prefixLen);
      }
    }

  cmdPrint (cmsh, "!%s", "\n");

  return SUCCESS;
}


/* Distribute-list update functions. */
static void
ripDistributeUpdate (DistributeT * pDist)
{
  InterfaceT * pIf = NULL;
  RipInterfaceT * pRIf = NULL;
  AccessListT * alist = NULL;
  PrefixListT * plist = NULL;

  if (! pDist->ifName)
  {
    return;
  }

  pIf = ifLookupByName (pDist->ifName);
  if (pIf == NULL)
  {
    return;
  }

  pRIf = pIf->pInfo;

  if (pDist->filterName[DISTRIBUTE_IN])
  {
    alist = accessListLookup (AFI_IP, pDist->filterName[DISTRIBUTE_IN]);
    if (alist)
      pRIf->pAccessList[RIP_FILTER_IN] = alist;
    else
      pRIf->pAccessList[RIP_FILTER_IN] = NULL;
  }
  else
    pRIf->pAccessList[RIP_FILTER_IN] = NULL;

  if (pDist->filterName[DISTRIBUTE_OUT])
  {
    alist = accessListLookup (AFI_IP, pDist->filterName[DISTRIBUTE_OUT]);
    if (alist)
      pRIf->pAccessList[RIP_FILTER_OUT] = alist;
    else
      pRIf->pAccessList[RIP_FILTER_OUT] = NULL;
  }
  else
    pRIf->pAccessList[RIP_FILTER_OUT] = NULL;

  if (pDist->prefixName[DISTRIBUTE_IN])
  {
    plist = prefixListLookup (AFI_IP, pDist->prefixName[DISTRIBUTE_IN]);
    if (plist)
      pRIf->pPrefixList[RIP_FILTER_IN] = plist;
    else
      pRIf->pPrefixList[RIP_FILTER_IN] = NULL;
  }
  else
    pRIf->pPrefixList[RIP_FILTER_IN] = NULL;

  if (pDist->prefixName[DISTRIBUTE_OUT])
  {
    plist = prefixListLookup (AFI_IP, pDist->prefixName[DISTRIBUTE_OUT]);
    if (plist)
      pRIf->pPrefixList[RIP_FILTER_OUT] = plist;
    else
      pRIf->pPrefixList[RIP_FILTER_OUT] = NULL;
  }
  else
    pRIf->pPrefixList[RIP_FILTER_OUT] = NULL;
}

void
ripDistributeUpdateInterface (InterfaceT * pIf)
{
  DistributeT * pDist = NULL;

  pDist = distributeLookup (pIf->name);
  if (pDist)
    ripDistributeUpdate (pDist);
}

/* Update all interface's distribute list. */
/* ARGSUSED */
static void
ripDistributeUpdateAll (PrefixListT *notused)
{
  ListNodeT * pNode = NULL;
  InterfaceT * pIf = NULL;

  for(pNode = pRip->pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pIf = pNode->pData;
    ripDistributeUpdateInterface (pIf);
  }
}
/* ARGSUSED */
static void
ripDistributeUpdateAllWrapper(AccessListT *notused)
{
  ripDistributeUpdateAll(NULL);
}

/* Delete all added rip route. */
void
ripClean (void)
{
  Int32T i = 0;
  RouteNodeT * pRNode = NULL;
  RipInfoT * pRInfo = NULL;

  if (!pRip)
    return;
  
  /* Delete debug configuration. */
  ripDebugReset ();

  /* Delete RIP routes */
  for (pRNode = nnRouteTop (pRip->pRipRibTable); 
       pRNode; 
       pRNode = nnRouteNext (pRNode))
  {
    if ((pRInfo = pRNode->pInfo) != NULL)
    {
      if (pRInfo->type == RIB_ROUTE_TYPE_RIP &&
          pRInfo->subType == RIP_ROUTE_RTE)
      {
        ripRibmgrIpv4Delete ((Prefix4T *)&pRNode->p,
                             &pRInfo->nexthop, pRInfo->metric);
      }

      /* Delete RIP timeout timers. */
      if(pRInfo->pTimerTimeout)
      {
        ripTimeoutTimerDelete (pRInfo);
      }

      /* Delete RIP garbage timers. */
      if(pRInfo->pTimerGarbage)
      { 
        ripGabageTimerDelete (pRInfo);
      }

      /* Delete RIP route node entries. */
      pRNode->pInfo = NULL;
      nnRouteNodeUnlock (pRNode);

      ripInfoFree (pRInfo);
    }
  }
  NNFREE (MEM_ROUTE_TABLE, pRip->pRipRibTable);
  pRip->pRipRibTable = NULL;


  /* Cancel RIP update timer. */
  if(pRip->pTimerUpdate)
  {
    ripUpdateTimerDelete ();
  }

  /* Close RIP socket. */
  if (pRip->sock >= 0)
  {
    /* Delete RIP socket from taskManager. */
    taskFdDel (pRip->pSockFdEvent);

    /* Close RIP socket. */
    close (pRip->sock);
    pRip->sock = -1;
  }

  /* Static RIP route configuration. */
  for (pRNode = nnRouteTop (pRip->pStaticRoute); 
       pRNode; 
       pRNode = nnRouteNext (pRNode))
  {
    if (pRNode->pInfo)
    {
      pRNode->pInfo = NULL; // check...
      nnRouteNodeUnlock (pRNode);
    }
  }
  NNFREE (MEM_ROUTE_TABLE, pRip->pStaticRoute);
  pRip->pStaticRoute = NULL;


  /* RIP neighbor configuration. */
  for (pRNode = nnRouteTop (pRip->pNeighbor); 
       pRNode; 
       pRNode = nnRouteNext (pRNode))
  {
    if (pRNode->pInfo)
    {
      pRNode->pInfo = NULL;
      nnRouteNodeUnlock (pRNode);
    }
  }
  NNFREE (MEM_ROUTE_TABLE, pRip->pNeighbor);
  pRip->pNeighbor = NULL;


  /* Redistribute related clear. */
  if (pRip->defaultInformationRouteMap)
  {
    NNFREE (MEM_ROUTEMAP, pRip->defaultInformationRouteMap);
    pRip->defaultInformationRouteMap = NULL;
  }

  for (i = 0; i < RIB_ROUTE_TYPE_MAX; i++)
  {
    if (pRip->routeMap[i].name)
    {
      NNFREE (MEM_ROUTEMAP_NAME, pRip->routeMap[i].name);
      pRip->routeMap[i].name = NULL;
    }
  }

  /* Clean or reset of library functions. */
  /* Clean peer list. */
  ripPeerClean ();

  /* Clean network list. */
  ripNetworkClean ();

  /* Clean passive default . */
  ripPassiveNonDefaultClean ();

  /* Clean offset value. */
  ripOffsetClean ();

  /* Reset interface list . */
  ripInterfaceReset ();

  /* Reset distance value. */
  ripDistanceReset ();

  ripRedistributeClean ();

  /* Reset keychain. */
  keychainReset ();
  pRip->pKeychainList = NULL;

  /* Reset global counters. */
  pRip->ripGlobalRouteChanges = 0;
  pRip->ripGlobalQueries = 0;

  /* Reset routemap. */
  ripRouteMapReset ();

  /* Reset library functions. */
  accessListReset ();
  prefixListReset ();
  distributeListReset ();  

}


static void
ripIfRmapUpdate (IfRouteMapT *pIfRmap)
{
  InterfaceT * pIf = NULL;
  RipInterfaceT *pRIf = NULL;
  RouteMapT *pRmap = NULL;

  pIf = ifLookupByName (pIfRmap->ifName);
  if (pIf == NULL)
    return;

  pRIf = pIf->pInfo;

  if (pIfRmap->routeMap[IF_RMAP_IN])
  {
    pRmap = routeMapLookupByName (pIfRmap->routeMap[IF_RMAP_IN]);
    if (pRmap)
      pRIf->routeMap[IF_RMAP_IN] = pRmap;
    else
      pRIf->routeMap[IF_RMAP_IN] = NULL;
  }
  else
    pRIf->routeMap[RIP_FILTER_IN] = NULL;

  if (pIfRmap->routeMap[IF_RMAP_OUT])
  {
    pRmap = routeMapLookupByName (pIfRmap->routeMap[IF_RMAP_OUT]);
    if (pRmap)
      pRIf->routeMap[IF_RMAP_OUT] = pRmap;
    else
      pRIf->routeMap[IF_RMAP_OUT] = NULL;
  }
  else
    pRIf->routeMap[RIP_FILTER_OUT] = NULL;
}

void
ripIfRmapUpdateInterface (InterfaceT * pIf)
{
  IfRouteMapT *pIfRmap;

  pIfRmap = ifRouteMapLookup (pIf->name);
  if (pIfRmap)
    ripIfRmapUpdate (pIfRmap);
}

static void
ripRoutemapUpdateRedistribute (void)
{
  Int32T i;

  if (pRip)
  {
    for (i = 0; i < RIB_ROUTE_TYPE_MAX; i++) 
    {
      if (pRip->routeMap[i].name)
        pRip->routeMap[i].map = 
          routeMapLookupByName (pRip->routeMap[i].name);
    }
  }
}

/* ARGSUSED */
static void
ripRoutemapUpdate (const char *notUsed)
{
  ListNodeT * pNode = NULL;
  InterfaceT * pIf = NULL;

  for(pNode = pRip->pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pIf = pNode->pData;
    ripIfRmapUpdateInterface (pIf);
  }

  ripRoutemapUpdateRedistribute ();
}


/* Initialize rip timer. */
static void
ripTimerInit ()
{
  /* Create persist 30 seconds timer. */
  ripUpdateTimerSet ();
}

/* Change timers basic values. */
void
ripTimerChange (void)
{
  /* Update persist 30 seconds timer. */
  ripUpdateTimerUpdate ();

  /* Update timeout timer & gabage collection timer. */
  RouteNodeT *pRNode = NULL;
  RipInfoT *pRInfo = NULL;
  for (pRNode = nnRouteTop (pRip->pRipRibTable); 
       pRNode; 
       pRNode = nnRouteNext (pRNode))
  {
    /* Check rip information in RouteNodeT. */
    if ((pRInfo = pRNode->pInfo) != NULL)
    {
      /* Check and update gabage collection timer. */
      if (pRInfo->pTimerGarbage)
      {
        ripGarbageTimerUpdate (pRInfo);
      }

      /* Check and update timer timer. */
      if (pRInfo->pTimerTimeout)
      {
        ripTimeoutTimerUpdate (pRInfo);
      }
    }
  }
}


/* Update rip timer. */
static void
ripTimerVersionUpdate ()
{
  ripTimerChange ();
}


/* Allocate new rip structure and set default value. */
void
ripInit (void)
{
  /* Randomize for triggered update random(). */
  srand (time (NULL));

  /* Allocate rip global data structure & socket. */
  Int32T ret = ripCreate ();
  if (ret < 0)
  {
    NNLOG (LOG_DEBUG, "Error] Can't create RIP\n");
    return;
  }

  /* Command Init. */
  if (!pRip->pCmshGlobal)
  {
    pRip->pCmshGlobal = compCmdInit (IPC_RIP, WriteConfCB);
  }

  /* Init rip timer. */
  ripTimerInit();

  /* Init interface list. */
  ripIfInit();

  /* Init offset list. */
  ripOffsetInit ();

  /* Init peer prefix list. */
  ripPeerInit();

  /* Init key chain. */
  keychainInit();
  pRip->pKeychainList = keychainGetPtr ();

  /* Init access list. */
  accessListInit ();
  accessListAddHook (ripDistributeUpdateAllWrapper);
  accessListDeleteHook (ripDistributeUpdateAllWrapper);
  pRip->accessMasterIpv4 = accessListGetMaster4 ();
#ifdef HAVE_IPV6
  pRip->accessMasterIpv6 = accessListGetMaster6 ();
#endif

  /* Init prefix list. */
  prefixListInit ();
  prefixListAddHook (ripDistributeUpdateAll);
  prefixListDeleteHook (ripDistributeUpdateAll);
  pRip->prefixMasterIpv4 = prefixListGetMaster4 ();
#ifdef HAVE_IPV6
  pRip->prefixMasterIpv6 = prefixListGetMaster6 ();
#endif

  /* Init distribute list. */
  distributeListInit (RIP);
  distributeListAddHook (ripDistributeUpdate);
  distributeListDeleteHook (ripDistributeUpdate);
  pRip->pDistHash = distributeListGetPtr ();

  /* Init route map */
  ripRouteMapInit ();
 
  /* Init interface route map */
  ifRouteMapInit (RIP);
  ifRouteMapHookAdd (ripIfRmapUpdate);
  ifRouteMapHookDelete (ripIfRmapUpdate);
  pRip->pIfRouteMapHash = ifRouteMapGetPtr ();

  /* Init distance control. */
  pRip->pRipDistanceTable = nnRouteTableInit ();

  /* Init debug control. */
  ripDebugInit ();

#ifdef HAVE_SNMP
  /* Init SNMP control. */
  ripSnmpInit ();
#endif /* HAVE_SNMP */

  /* Registrate to Ribmgr. */
  ripRibmgrInit ();
}


void
ripVersionUpdate (void)
{
  assert (pRip);

  /* Update Command pointer. */
  compCmdUpdate (pRip->pCmshGlobal, WriteConfCB);

  /* Update socket. */
  if (pRip->pSockFdEvent)
  {
    taskFdUpdate (ripPacketRead, pRip->pSockFdEvent, NULL);
  }
  else
  {
    NNLOG(LOG_ERR, "pRip->pSockFdEvent=%p\n", pRip->pSockFdEvent);
    return;
  }

  /* Update persist 30 seconds timer. */
  /* Update Timeout 180 time timer. */
  /* Update Garbage 120 time timer. */
  if (pRip->pTimerUpdate)
  {
    ripTimerVersionUpdate();
  }
  else
  {
    NNLOG(LOG_ERR, "pRip->pTimerUpdate=%p\n", pRip->pTimerUpdate);
    return;
  } 


  /* Update interface list. */
  ripIfVersionUpdate ();

  /* Update callback function in peer list. */
  ripPeerVersionUpdate();

  /* Update offset list. - None. */

  /* Update peer prefix list. - None. */

  /* Update keychain list. */
  if (pRip->pKeychainList)
  {
    keychainVersionUpdate (pRip->pKeychainList);
  }
  else
  {
    NNLOG(LOG_WARNING, "pRip->pKeychainList=%p\n", pRip->pKeychainList);
  }

  /* Update accesslist master. */
  accessListVersionUpdate4 (pRip->accessMasterIpv4);
#ifdef HAVE_IPV6
  accessListGetMaster6 (pRip->accessMasterIpv6);
#endif
  accessListAddHook (ripDistributeUpdateAllWrapper);
  accessListDeleteHook (ripDistributeUpdateAllWrapper);

  /* Update prefix list. */
  prefixListVersionUpdate4 (pRip->prefixMasterIpv4);
#ifdef HAVE_IPV6
  prefixListGetMaster6 (pRip->prefixMasterIpv6);
#endif
  prefixListAddHook (ripDistributeUpdateAll);
  prefixListDeleteHook (ripDistributeUpdateAll);
  
  /* Update distribute list. */
  if (pRip->pDistHash)
  {
    distributeListVersionUpdate (RIP, pRip->pDistHash);
    distributeListAddHook (ripDistributeUpdate);
    distributeListDeleteHook (ripDistributeUpdate);
  }
  else
  {
    NNLOG(LOG_WARNING, "pRip->pDistHash=%p\n", pRip->pDistHash);
  }

  /* Update route map. */
  ripRouteMapVersionUpdate ();

  /* Update interface route map. */
  if (pRip->pIfRouteMapHash)
  {
    ifRouteMapVersionUpdate (RIP, pRip->pIfRouteMapHash);
    ifRouteMapHookAdd (ripIfRmapUpdate);
    ifRouteMapHookDelete (ripIfRmapUpdate);
  }
  else
  {
    NNLOG(LOG_WARNING, "pRip->pIfRouteMapHash=%p\n", pRip->pIfRouteMapHash);
  }

  /* Update distance control. - None. */

  /* Update debug control. - Node. */
 
  /* Update SNMP control. - None. */
}

