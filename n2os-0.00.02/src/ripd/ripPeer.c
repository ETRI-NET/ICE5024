/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : RIP Protocol의 Peer 정보 설정을 제어하는 화일
 *
 * - Block Name : RIP Protocol
 * - Process Name : ripd
 * - Creator : Geontae Park
 * - Initial Date : 2014/02/20
 */

/**
 * @file        : ripPeer.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include <assert.h>

#include "nnTypes.h"
#include "nnPrefix.h"
#include "nnList.h"
#include "nosLib.h"

#include "ripd.h"


static RipPeerT *
ripPeerNew (void)
{
  return NNMALLOC (MEM_RIP_PEER, sizeof (RipPeerT));
}

static void
ripPeerFree (RipPeerT *pPeer)
{
  //NNFREE (MEM_RIP_PEER, pPeer);
}

RipPeerT *
ripPeerLookup (struct in_addr * pAddr)
{
  ListNodeT * pNode = NULL;
  RipPeerT * pPeer = NULL;

  for(pNode = pRip->pPeerList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pPeer = pNode->pData;
    if (PREFIX_IPV4_ADDR_SAME (&pPeer->addr, pAddr))
    {
      return pPeer;
    }
  }

  return NULL;
}

RipPeerT *
ripPeerLookupNext (struct in_addr * pAddr)
{
  ListNodeT * pNode = NULL;
  RipPeerT * pPeer = NULL;

  for(pNode = pRip->pPeerList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pPeer = pNode->pData;
    if (htonl (pPeer->addr.s_addr) > htonl (pAddr->s_addr))
    {
      return pPeer;
    }
  }

  return NULL;
}


/* RIP peer is timeout. */
static void
ripPeerTimeout (Int32T fd, Int16T event, void * arg)
{
  NNLOG (LOG_DEBUG, "ripPeerTimeout. peer ptr=%p\n", arg);

  /* Assign arg pointer to peer pointer. */
  RipPeerT * pPeer = (RipPeerT *)arg;

  /* Delete timer. */
  taskTimerDel (pPeer->pTimeout);
 
  /* Delete peer from list. */
  nnListDeleteNode (pRip->pPeerList, pPeer);
   
  /* Free rip peer. - but do nothing - . auto free in nnListDeleteNode. */
  ripPeerFree (pPeer);
}


/* Get RIP peer.  At the same time update timeout thread. */
static RipPeerT *
ripPeerGet (struct in_addr *pAddr)
{
  RipPeerT * pPeer = NULL;

  pPeer = ripPeerLookup (pAddr);

  if (pPeer)
  {
    /* Update timer. */
    taskTimerUpdate (ripPeerTimeout, pPeer->pTimeout, 
                     pPeer->tvTimeout, pPeer);
  }
  else
  {
    pPeer = ripPeerNew ();
    pPeer->addr = *pAddr;

    /* Set time value. */
    pPeer->tvTimeout.tv_sec = RIP_PEER_TIMER_DEFAULT;

    /* Add timer. */
    pPeer->pTimeout = taskTimerSet (ripPeerTimeout, pPeer->tvTimeout, 
                                    0, (void *)pPeer);
    /* Add peer to list. */
    nnListAddNodeSort (pRip->pPeerList, pPeer);
  }

  /* Last update time set. */
  time (&pPeer->uptime);
  
  return pPeer;
}

void
ripPeerUpdate (struct sockaddr_in * pFrom, Uint8T version)
{
  RipPeerT *pPeer = NULL;

  pPeer = ripPeerGet (&pFrom->sin_addr);
  if (! pPeer)
  {
    NNLOG (LOG_ERR, "Error] Not exist peer.\n");
    return;
  }
  pPeer->version = version;
}

void
ripPeerBadRoute (struct sockaddr_in *pFrom)
{
  RipPeerT *pPeer = NULL;

  pPeer = ripPeerGet (&pFrom->sin_addr);
  if (! pPeer)
  {
    NNLOG (LOG_ERR, "Error] Not exist peer.\n");
    return;
  }
  pPeer->recvBadRoutes++;
}

void
ripPeerBadPacket (struct sockaddr_in *pFrom)
{
  RipPeerT *pPeer = NULL;

  pPeer = ripPeerGet (&pFrom->sin_addr);
  if (! pPeer)
  {
    NNLOG (LOG_ERR, "Error] Not exist peer.\n");
    return;
  }
  pPeer->recvBadPackets++;
}

/* Display peer uptime. */
static char *
ripPeerUptime (RipPeerT *pPeer, char *buf, size_t len)
{
  time_t uptime;
  struct tm * tm = NULL;

  /* If there is no connection has been done before print `never'. */
  if (pPeer->uptime == 0)
  {
    snprintf (buf, len, "never   ");
    return buf;
  }

  /* Get current time. */
  uptime = time (NULL);
  uptime -= pPeer->uptime;
  tm = gmtime (&uptime);

  /* Making formatted timer strings. */
#define ONE_DAY_SECOND 60*60*24
#define ONE_WEEK_SECOND 60*60*24*7

  if (uptime < ONE_DAY_SECOND)
    snprintf (buf, len, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
  else if (uptime < ONE_WEEK_SECOND)
    snprintf (buf, len, "%dd%02dh%02dm", tm->tm_yday, tm->tm_hour, tm->tm_min);
  else
    snprintf (buf, len, "%02dw%dd%02dh", tm->tm_yday/7, tm->tm_yday - ((tm->tm_yday/7) * 7), tm->tm_hour);

  return buf;
}


void
ripPeerDisplay (struct cmsh * cmsh)
{
  ListNodeT * pNode = NULL;
  RipPeerT * pPeer = NULL;
#define RIP_UPTIME_LEN 25
  char timebuf[RIP_UPTIME_LEN];

  if (!pRip->pPeerList)
  {
    NNLOG (LOG_WARNING, "pPeerList is null.\n");
    return;
  }

  for(pNode = pRip->pPeerList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pPeer = pNode->pData;
    cmdPrint (cmsh, "    %-16s %9d %9d %9d   %s\n", inet_ntoa (pPeer->addr),
              pPeer->recvBadPackets, pPeer->recvBadRoutes,
              RIB_DISTANCE_DEFAULT_RIP,
              ripPeerUptime (pPeer, timebuf, RIP_UPTIME_LEN));
  }
}


/*
 * Description : List에서 Peer IP Address 값을 기반으로 소트하여 정렬하기 위한 비교함수.
 *
 * param [in] pOldData : ListNode의 포인터
 * param [in] pNewData : ListNode의 포인터
 *
 * retval : 0 if 기존 ListNode의 데이터가 새로운 ListNode의 데이터 보다 큰경우
 *          1 if 기존 ListNode의 데이터가 새로운 ListNode의 데이터 보다 작은경우.
 */
static Int8T 
ripPeerCmp (void *pOldData, void *pNewData)
{
  if ((((RipPeerT *)pOldData)->addr.s_addr) > (((RipPeerT *)pNewData)->addr.s_addr))
  {
     return 0;  
  }
  else if ((((RipPeerT *)pOldData)->addr.s_addr) < (((RipPeerT *)pNewData)->addr.s_addr))
  {
    return 1;
  }
  /* same data */
  else
  {
    return -1; 
  }
}


/* Description : PeerList의 자료구조를 초기화 하는 함수. */
void
ripPeerInit (void)
{
  pRip->pPeerList = nnListInit(ripPeerCmp, MEM_RIP_PEER);
}

/* Re-mapping callback functions in peer list. */
void
ripPeerVersionUpdate(void)
{
  ListNodeT * pNode = NULL;
  RipPeerT * pPeer = NULL;

  /* Re-mapping peer list's compare function. */
  nnListSetNodeCompFunc (pRip->pPeerList, ripPeerCmp);
  NNLOG (LOG_DEBUG, "update peer list's compare function. \n");
  
  /* Re-mapping each peer's timer callback functions. */ 
  for(pNode = pRip->pPeerList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pPeer = pNode->pData;

    /* Update timer. */
    if (pPeer)
    {
      NNLOG (LOG_DEBUG, "update each peer's timeout callback functions.\n");
      taskTimerUpdate (ripPeerTimeout, pPeer->pTimeout, 
                       pPeer->tvTimeout, pPeer);
    }
  }
}

/* Description : Peer 정보를 초기화 하는 함수. */
void
ripPeerClean()
{
  ListNodeT * pNode = NULL;
  RipPeerT * pPeer = NULL;

  /* Check rip global data structure pointer. */
  if (!pRip)
    return;

  /* Check peer list pointer. */
  if (!pRip->pPeerList)
    return;

  /* Delete all peer's timeout. */
  for(pNode = pRip->pPeerList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pPeer = pNode->pData;

    if (pPeer)
    {
      taskTimerDel(pPeer->pTimeout);
    }
  }

  /* Delete all peer node && list. */
  nnListFree (pRip->pPeerList);
  pRip->pPeerList = NULL;
}
