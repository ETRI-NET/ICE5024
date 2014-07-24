/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : 각 컴포넌트에서 사용하는 공통 interface 관련 기능을 제공한다.
 * - Block Name : riblib
 * - Process Name : ribmgr
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : nnIf.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include <net/if.h>
#include <arpa/inet.h>

#include "nnRibDefines.h"
#include "nnStr.h"
#include "nnPrefix.h"
#include "nnTypes.h"
#include "nnList.h"
#include "nnIf.h"
#include "nosLib.h"


/* Master list of interfaces. */
ListT * pIfList = NULL;

/* One for each program.  This structure is needed to store hooks. */
struct ifMaster
{
  int (*ifNewHook) (InterfaceT *); 
  int (*ifDeleteHook) (InterfaceT *); 
} ifMaster;



/*
 * Description : 인터페이스를 저장 할때 정렬하기 위하여 인터페이스 이름을 기반으로 
 * 비교하는 함수.
 * Compare interface names, returning an integer greater than, equal to, or
 * less than 0, (following the strcmp convention), according to the
 * relationship between ifp1 and ifp2.  Interface names consist of an
 * alphabetic prefix and a numeric suffix.  The primary sort key is
 * lexicographic by name, and then numeric by number.  No number sorts
 * before all numbers.  Examples: de0 < de1, de100 < fxp0 < xl0, devpty <
 * devpty0, de0 < del0
 *
 * @param [in] ifp1 : 인터페이스 자료구조 포인터
 * @param [in] ifp2 : 인터페이스 자료구조 포인터
 *
 * @retval : -1 if ifp1 < ifp2,
 *            1 if ifp2 < ifp1
 */
Int8T 
ifListCmp (void * ifp1, void * ifp2)
{
  Uint32T l1, l2;
  Uint32T x1, x2;
  char *p1, *p2;
  Int32T res;

  p1 = ((InterfaceT *)ifp1)->name;
  p2 = ((InterfaceT *)ifp2)->name;

  while (*p1 && *p2) {
    /* look up to any number */
    l1 = strcspn(p1, "0123456789");
    l2 = strcspn(p2, "0123456789");

    /* name lengths are different -> compare names */
    if (l1 != l2)
      return (strcmp(p1, p2));

    /* Note that this relies on all numbers being less than all letters, so
     * that de0 < del0.
     */
    res = strncmp(p1, p2, l1);

    /* names are different -> compare them */
    if (res)
      return res;

    /* with identical name part, go to numeric part */
    p1 += l1;
    p2 += l1;

    if (!*p1) 
      return -1;
    if (!*p2) 
      return 1;

    x1 = strtol(p1, &p1, 10);
    x2 = strtol(p2, &p2, 10);

    /* let's compare numbers now */
    if (x1 < x2)
      return -1;
    if (x1 > x2)
      return 1;

    /* numbers were equal, lets do it again..
    (it happens with name like "eth123.456:789") */
  }
  if (*p1)
    return 1;
  if (*p2)
    return -1;
  return 0;
}


/* Description : 인터페이스를 생성하는 함수.  */
InterfaceT *
ifCreate (const StringT ifName, Int32T nameLen)
{
  InterfaceT *pIf = NULL;

  pIf = NNMALLOC (MEM_IF, sizeof (InterfaceT));
  pIf->ifIndex = IFINDEX_INTERNAL;

  
  assert (ifName);
  assert (nameLen <= INTERFACE_NAMSIZ);	/* Need space for '\0' at end. */
  strncpy (pIf->name, ifName, nameLen);
  pIf->name[nameLen] = '\0';
  if (ifLookupByName(pIf->name) == NULL)
  {
    nnListAddNodeSort (pIfList, pIf);
  }
  else
  {
    NNLOG (LOG_ERR, "Err : ifCreate(%s): corruption detected -- interface with this "
	       "name exists already!", pIf->name);
  }

  pIf->pConnected = nnListInit(NULL, MEM_CONNECTED);

  if (ifMaster.ifNewHook)
    (*ifMaster.ifNewHook) (pIf);

  return pIf;
}


/* Description : 인터페이스를 삭제하는 함수. */
void
ifDeleteRetain (InterfaceT *pIf)
{
  assert (pIf);

  if (ifMaster.ifDeleteHook)
    (*ifMaster.ifDeleteHook) (pIf);

  /* Free connected address list */
  nnListFree(pIf->pConnected);
}


/* Description : 인터페이스를 삭제하고, 메모리 해제  함수. */
void
ifDelete (InterfaceT *pIf)
{
  ListNodeT * pNode = NULL;
  
  assert (pIf);

  if ((pNode = nnListSearchNode (pIfList, pIf)))
  {
    ifDeleteRetain(pIf); // delete connected pointer
    nnListDeleteNode (pIfList, pNode); // delete interface
  }
}


/* Add hook to interface callback handler. */
void
ifAddHook (int type, int (*func)(InterfaceT *pIf))
{
  switch (type) {
  case IF_NEW_HOOK:
    ifMaster.ifNewHook = func;
    break;
  case IF_DELETE_HOOK:
    ifMaster.ifDeleteHook = func;
    break;
  default:
    break;
  }
}


/* Description : 리스트에서 인덱스를 기반으로 인터페이스를 검색하는 함수. */
InterfaceT *
ifLookupByIndex (Uint32T index)
{
  ListNodeT * pNode = NULL;
  InterfaceT * pIf = NULL;

  for(pNode = pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pIf = pNode->pData;
    if(pIf->ifIndex == index)
      return pIf;
  }

  return NULL;
}


/* Description : 리스트에서 인덱스를 기반으로 인터페이스 이름을 검색하는 함수. */
const StringT 
ifIndex2ifName (Uint32T index)
{
  InterfaceT *pIf = NULL;

  return ((pIf = ifLookupByIndex(index)) != NULL) ?
  	 pIf->name : "unknown";
}


/* Description : 리스트에서 인터페이스 이름을 기반으로 인터페이스 인덱스를 검색하는 함수. */
Uint32T
ifName2ifIndex (const StringT ifName)
{
  InterfaceT * pIf = NULL;

  return ((pIf = ifLookupByName(ifName)) != NULL) ? pIf->ifIndex
                                                   : IFINDEX_INTERNAL;
}


/* Description : 리스트에서 인터페이스 이름을 기반으로 인터페이스를 검색하는 함수. */
InterfaceT *
ifLookupByName (const StringT ifName)
{
  ListNodeT * pNode = NULL;
  InterfaceT * pIf = NULL;

  if (ifName)
  {
    for(pNode = pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
    {
      pIf = pNode->pData;
      if(strcmp(ifName, pIf->name) == 0)
        return pIf;
    } /* for loop */
  } /* if */

  return NULL;
}


/* Description : 리스트에서 인터페이스 이름 및 길이를 기반으로 인터페이스를 검색하는 함수. */
InterfaceT *
ifLookupByNameLen(const StringT ifName, size_t nameLen)
{
  ListNodeT * pNode = NULL;
  InterfaceT * pIf = NULL;

  if (nameLen > INTERFACE_NAMSIZ)
    return NULL;

  for(pNode = pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pIf = pNode->pData;
    if (!memcmp(ifName, pIf->name, nameLen) && (pIf->name[nameLen] == '\0'))
	  return pIf;
  } /* for loop */

  return NULL;
}


/* Description : 리스트에서 IPv4 주소를 기반으로 인터페이스를 검색하는 함수. */
InterfaceT *
ifLookupExactAddress (struct in_addr src)
{
  PrefixT *pPrefix = NULL;
  ConnectedT *pConnected = NULL;
  ListNodeT * pNode = NULL;
  InterfaceT * pIf = NULL;

  for(pNode = pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pIf = pNode->pData;

    ListT * pConnectedList = pIf->pConnected;
    ListNodeT * pConnectedNode;
    for (pConnectedNode = pConnectedList->pHead;
         pConnectedNode != NULL;
         pConnectedNode = pConnectedNode->pNext)
	{
      pConnected = pConnectedNode->pData;
      pPrefix = pConnected->pAddress;

      if (pPrefix && pPrefix->family == AF_INET)
      {
        if (PREFIX_IPV4_ADDR_SAME (&pPrefix->u.prefix4, &src))
        return pIf;
      }	/* if */
	} /* for loop */
  } /* for loop */

  return NULL;
}


/* Description : 리스트에서 IPv4 주소를 기반으로 인터페이스를 검색하는 함수. */
InterfaceT *
ifLookupAddress (struct in_addr src)
{
  PrefixT addr = {0,};
  Int32T bestlen = 0;
  ConnectedT *c = NULL;
  InterfaceT *match = NULL;
  ListNodeT * pNode = NULL;
  InterfaceT * pIf = NULL;

  addr.family = AF_INET;
  addr.u.prefix4 = src;
  addr.prefixLen = PREFIX_IPV4_MAX_BITLEN;

  for(pNode = pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pIf = pNode->pData;

    ListT * pConnectedList = pIf->pConnected;
    ListNodeT * pConnectedNode;
    for (pConnectedNode = pConnectedList->pHead;
         pConnectedNode != NULL;
         pConnectedNode = pConnectedNode->pNext)
    {
      c = pConnectedNode->pData;
      if (c->pAddress && (c->pAddress->family == AF_INET) &&
          nnPrefixMatch(CONNECTED_PREFIX(c), &addr) &&
          (c->pAddress->prefixLen > bestlen))
      {
        bestlen = c->pAddress->prefixLen;
        match = pIf;
      } /* if */
    } /* inner for loop */
  } /* outer for loop */

  return match;
}


/* Description : 인터페이스 이름을 기반으로 인터페이스를 검색하고, 없는 경우
 * 인터페이스를 생성하여 포인터를 반환하는 함수. */
InterfaceT *
ifGetByName (const StringT ifName)
{
  InterfaceT * pIf = NULL;

  return ((pIf = ifLookupByName(ifName)) != NULL) ? pIf :
	 ifCreate(ifName, strlen(ifName));
}


/* Description : 인터페이스 이름과 길이를 기반으로 인터페이스를 검색하는 함수. */
InterfaceT *
ifGetByNameLen(const StringT ifName, size_t nameLen)
{
  InterfaceT * pIf = NULL;

  return ((pIf = ifLookupByNameLen(ifName, nameLen)) != NULL) ? pIf :
	 ifCreate(ifName, nameLen);
}


/* Description : 인터페이스 플래그가 IFF_UP 임을 확인하는 함수. */
Int32T
ifIsUp (InterfaceT * pIf)
{
  assert (pIf);

  return pIf->flags & IFF_UP;
}


/* Description : 인터페이스 플래그가 IFF_RUNNING 임을 확인하는 함수. */
Int32T
ifIsRunning (InterfaceT * pIf)
{
  assert (pIf);

  return pIf->flags & IFF_RUNNING;
}


/* Description : 인터페이스 플래그가 Operative 상태 임을 확인하는 함수. */
Int32T
ifIsOperative (InterfaceT * pIf)
{
  return ((pIf->flags & IFF_UP) &&
	  (pIf->flags & IFF_RUNNING || !CHECK_FLAG(pIf->status, RIBMGR_INTERFACE_LINKDETECTION)));
}


/* Description : 인터페이스가 Loopback 임을 확인하는 함수. */
Int32T
ifIsLoopback (InterfaceT * pIf)
{
  assert (pIf);

  /* XXX: Do this better, eg what if IFF_WHATEVER means X on platform M
   * but Y on platform N?
   */
  return (pIf->flags & (IFF_LOOPBACK|IFF_NOXMIT|IFF_VIRTUAL));
}


/* Description : 인터페이스가 Broadcast 타입이 설정되어 있는가를 확인하는 함수. */
Int32T
ifIsBroadcast (InterfaceT * pIf)
{
  assert (pIf);

  return pIf->flags & IFF_BROADCAST;
}


/* Description : 인터페이스가 P2P 타입이 설정되어 있는가를 확인하는 함수. */
Int32T
ifIsPointPoint (InterfaceT * pIf)
{
  assert (pIf);

  return pIf->flags & IFF_POINTOPOINT;
}


/* Description : 인터페이스가 Multicast 타입이 설정되어 있는가를 확인하는 함수. */
Int32T
ifIsMulticast (InterfaceT * pIf)
{
  assert (pIf);

  return pIf->flags & IFF_MULTICAST;
}


/* Description : 인터페이스 플래그를 덤프하는 함수. */
const StringT 
ifFlagDump (Uint32T flag)
{
  Int32T separator = 0;
  static char logbuf[BUFSIZ];

#define IFF_OUT_LOG(X,STR) \
  if (flag & (X)) \
    { \
      if (separator) \
        nnStrlCat (logbuf, ",", BUFSIZ); \
      else \
        separator = 1; \
      nnStrlCat (logbuf, STR, BUFSIZ); \
    }

  nnStrlCpy (logbuf, "<", BUFSIZ);
  IFF_OUT_LOG (IFF_UP, "UP");
  IFF_OUT_LOG (IFF_BROADCAST, "BROADCAST");
  IFF_OUT_LOG (IFF_DEBUG, "DEBUG");
  IFF_OUT_LOG (IFF_LOOPBACK, "LOOPBACK");
  IFF_OUT_LOG (IFF_POINTOPOINT, "POINTOPOINT");
  IFF_OUT_LOG (IFF_NOTRAILERS, "NOTRAILERS");
  IFF_OUT_LOG (IFF_RUNNING, "RUNNING");
  IFF_OUT_LOG (IFF_NOARP, "NOARP");
  IFF_OUT_LOG (IFF_PROMISC, "PROMISC");
  IFF_OUT_LOG (IFF_ALLMULTI, "ALLMULTI");
  IFF_OUT_LOG (IFF_OACTIVE, "OACTIVE");
  IFF_OUT_LOG (IFF_SIMPLEX, "SIMPLEX");
  IFF_OUT_LOG (IFF_LINK0, "LINK0");
  IFF_OUT_LOG (IFF_LINK1, "LINK1");
  IFF_OUT_LOG (IFF_LINK2, "LINK2");
  IFF_OUT_LOG (IFF_MULTICAST, "MULTICAST");
  IFF_OUT_LOG (IFF_NOXMIT, "NOXMIT");
  IFF_OUT_LOG (IFF_NORTEXCH, "NORTEXCH");
  IFF_OUT_LOG (IFF_VIRTUAL, "VIRTUAL");
  IFF_OUT_LOG (IFF_IPV4, "IPv4");
  IFF_OUT_LOG (IFF_IPV6, "IPv6");

  nnStrlCat (logbuf, ">", BUFSIZ);

  return logbuf;
#undef IFF_OUT_LOG
}


/*
 * Description : 인터페이스 정보를 덤프하는 함수.
 *
 * @param [in] pIf : 인터페이스 자료구조 포인터
 */
static void
ifDump (const InterfaceT * pIf)
{
  ConnectedT *connected = NULL;
  ListT * pConnectedList = pIf->pConnected;
  ListNodeT *pConnectedNode = NULL;

  assert (pIf);

  NNLOG (LOG_DEBUG, "\n");
  NNLOG (LOG_DEBUG, "Interface %s index %d metric %d mtu %d "
#ifdef HAVE_IPV6
             "mtu6 %d "
#endif /* HAVE_IPV6 */
             "%s\n",
	     pIf->name, pIf->ifIndex, pIf->metric, pIf->mtu, 
#ifdef HAVE_IPV6
	     pIf->mtu6,
#endif /* HAVE_IPV6 */
	     ifFlagDump (pIf->flags));


  for (pConnectedNode = pConnectedList->pHead;
       pConnectedNode != NULL;
       pConnectedNode = pConnectedNode->pNext)
  {   
    char addrStr[INET6_ADDRSTRLEN]={};
    char destStr[INET6_ADDRSTRLEN]={};
    PrefixT *p = NULL;
    connected = pConnectedNode->pData;
    if(connected->pAddress)
    {   
      PrefixT *p = connected->pAddress;
      if(p->family == AF_INET)
        inet_ntop (p->family, &p->u.prefix4, addrStr, sizeof (addrStr));
#ifdef HAVE_IPV6
      else if(p->family == AF_INET6)
        inet_ntop (p->family, &p->u.prefix6, addrStr, sizeof (addrStr));
#endif
    }/* if */

    if(connected->pDestination)
    {   
      p = connected->pDestination;
      if(p->family == AF_INET)
        inet_ntop (p->family, &p->u.prefix4, destStr, sizeof (destStr));
#ifdef HAVE_IPV6
      else if(p->family == AF_INET6)
        inet_ntop (p->family, &p->u.prefix6, destStr, sizeof (destStr));
#endif
    }/* if */

    NNLOG (LOG_DEBUG, "\t address=%s, destination=%s\n", addrStr, destStr);
  } /* for loop */
}


/* Description : 인터페이스 정보를 덤프하는 함수. */
void
ifDumpAll (void)
{
  ListNodeT * pNode = NULL;
  InterfaceT * pIf = NULL;

  for(pNode = pIfList->pHead;pNode != NULL;pNode=pNode->pNext)
  {
    pIf = pNode->pData;
    ifDump(pIf);
  }
}


/* Description : 커넥티드 자료구조를 생성하는 함수. */
ConnectedT *
connectedNew (void)
{
  return NNMALLOC (MEM_CONNECTED, sizeof (ConnectedT));
}


/* Description : 커넥티드 자료구조를 메모리를 해제하는 함수. */
void
connectedFree (ConnectedT *pConnected)
{
  assert (pConnected);

  if (pConnected->pAddress)
    nnPrefixFree (pConnected->pAddress);

  if (pConnected->pDestination)
    nnPrefixFree (pConnected->pDestination);

  if (pConnected->label)
    NNFREE (MEM_CONNECTED, pConnected->label);
}


/* Description : 인터페이스 주소 자료구조를 로깅하는 함수. */
static void __attribute__ ((unused))
connectedLog (ConnectedT *pConnected, StringT str)
{
  PrefixT *pPrefix = NULL;
  InterfaceT * pIf = NULL;
  char logbuf[BUFSIZ] = {};
  char buf[BUFSIZ] = {};
  
  assert (pConnected);

  pIf = pConnected->pIf;
  pPrefix = pConnected->pAddress;

  if (pPrefix->family == AF_INET)
    snprintf (logbuf, BUFSIZ, "%s interface %s %s %s/%d ", 
	      str, pIf->name, nnCnvPrefixFamilytoString (pPrefix),
	      (char *)inet_ntop (pPrefix->family, &pPrefix->u.prefix4, buf, BUFSIZ),
	      pPrefix->prefixLen);
#ifdef HAVE_IPV6
  else if (pPrefix->family == AF_INET6)
    snprintf (logbuf, BUFSIZ, "%s interface %s %s %s/%d ", 
	      str, pIf->name, nnCnvPrefixFamilytoString (pPrefix),
	      (char *)inet_ntop (pPrefix->family, &pPrefix->u.prefix6, buf, BUFSIZ),
	      pPrefix->prefixLen);
#endif

  pPrefix = pConnected->pDestination;
  if (pPrefix)
  {
    if (pPrefix->family == AF_INET)
      strncat (logbuf, (char *)inet_ntop (pPrefix->family, &pPrefix->u.prefix4, buf, BUFSIZ),
             BUFSIZ - strlen(logbuf));
#ifdef HAVE_IPV6
    else if (pPrefix->family == AF_INET6)
      strncat (logbuf, (char *)inet_ntop (pPrefix->family, &pPrefix->u.prefix6, buf, BUFSIZ),
             BUFSIZ - strlen(logbuf));
#endif
  }
  NNLOG(LOG_DEBUG, logbuf);
}


/* Description : PrefixT 자료구조 내의 IP 주소가 같은지를 확인하는 함수.
 *
 * @param [in] pPrefix1 : PrefixT 자료구조 포인터
 * @param [in] pPrefix2 : PrefixT 자료구조 포인터
 *
 * @retval : 1 if IP 주소가 같음,
 *           0 if IP 주소가 같지 않음
 */
static Int32T
connectedSamePrefix (PrefixT *pPrefix1, PrefixT *pPrefix2)
{
  assert (pPrefix1);
  assert (pPrefix2);

  if (pPrefix1->family == pPrefix2->family)
    {
      if (pPrefix1->family == AF_INET &&
	  PREFIX_IPV4_ADDR_SAME (&pPrefix1->u.prefix4, &pPrefix2->u.prefix4))
	return 1;
#ifdef HAVE_IPV6
      if (pPrefix1->family == AF_INET6 &&
	  PREFIX_IPV6_ADDR_SAME (&pPrefix1->u.prefix6, &pPrefix2->u.prefix6))
	return 1;
#endif /* HAVE_IPV6 */
    }
  return 0;
}


/* Description : Connected List에서 Prefix 값으로 ListNode를 삭제하는 함수. */
ConnectedT *
connectedDeleteByPrefix (InterfaceT * pIf, PrefixT *pPrefix)
{
  ConnectedT *pIfc = NULL;

  assert (pIf);
  assert (pPrefix);

  /* In case of same prefix come, replace it with new one. */
  ListT * pConnectedList = pIf->pConnected;
  ListNodeT * pConnectedNode;
  for (pConnectedNode = pConnectedList->pHead; 
       pConnectedNode != NULL;
       pConnectedNode = pConnectedNode->pNext)
  {
    pIfc = pConnectedNode->pData;

    if (connectedSamePrefix (pIfc->pAddress, pPrefix))
    {
      nnListDeleteNode(pConnectedList, pIfc);
      return pIfc;
    }
  }

  return NULL;
}


/* Description : 인터페이스에 설정된 주소에서 connected 정보를 얻는 함수. */
ConnectedT *
connectedLookupAddress (InterfaceT * pIf, struct in_addr dst)
{
  PrefixT addr = {0,};
  ConnectedT *pConnected = NULL;
  ConnectedT *pMatch = NULL;

  assert (pIf);

  addr.family = AF_INET;
  addr.u.prefix4 = dst;
  addr.prefixLen = PREFIX_IPV4_MAX_BITLEN;


  ListT * pConnectedList = pIf->pConnected;
  ListNodeT * pConnectedNode;
  pMatch = NULL;

  for (pConnectedNode = pConnectedList->pHead;
       pConnectedNode != NULL;
       pConnectedNode = pConnectedNode->pNext)
  {
    pConnected = pConnectedNode->pData;
    if (pConnected->pAddress && 
        (pConnected->pAddress->family == AF_INET) &&
        nnPrefixMatch(CONNECTED_PREFIX(pConnected), &addr) &&
        (!pMatch || (pConnected->pAddress->prefixLen > pMatch->pAddress->prefixLen)))
      pMatch = pConnected;
  }

  return pMatch;
}


/* Description : 인터페이스에 설정된 주소에서 Prefix 정보를 기반으로 connected 
 * 정보를 얻는 함수. */
ConnectedT *
connectedAddByPrefix (InterfaceT * pIf, PrefixT *pPrefix, PrefixT *pDst)
{
  ConnectedT *pIfc = NULL;

  assert (pIf);
  assert (pPrefix);

  /* Allocate new connected address. */
  pIfc = connectedNew ();
  pIfc->pIf = pIf;

  /* Fetch interface address */
  pIfc->pAddress = nnPrefixNew();
  memcpy (pIfc->pAddress, pPrefix, sizeof(PrefixT));

  /* Fetch dest address */
  if (pDst)
  {
    pIfc->pDestination = nnPrefixNew();
    memcpy (pIfc->pDestination, pDst, sizeof(PrefixT));
  }

  /* Add connected address to the interface. */
  nnListAddNode (pIf->pConnected, pIfc);

  return pIfc;
}

#ifndef HAVE_IF_NAMETOINDEX
/* Description : 인터페이스 이름으로 인덱스를 찾는 함수. */
Uint32T
ifNametoIndex (const StringT ifName)
{
  InterfaceT * pIf = NULL;

  return ((pIf = ifLookupByNameLen(ifName, strnlen(ifName, IFNAMSIZ))) != NULL)
  	 ? pIf->ifIndex : 0;
}
#endif

#ifndef HAVE_IF_INDEXTONAME
/* Description : 인덱스로 인터페이스 이름을 찾는 함수. */
char *
ifIndextoName (Uint32T ifIndex, StringT ifName)
{
  InterfaceT * pIf = NULL;

  if (!(pIf = ifLookupByIndex(ifIndex)))
    return NULL;

  strncpy (ifName, pIf->name, IFNAMSIZ);

  return pIf->name;
}
#endif

/* Description : 인터페이스 정보를 저장하는 리스트를 초기화 하는 함수. */
ListT *
ifInit ()
{
  pIfList = nnListInit (ifListCmp, MEM_IF_LIST);

  return pIfList;
}

/* Description : 인터페이스 리스트 포인터를 갱신하는 함수. */
void 
ifUpdate (ListT * gpIfList)
{
  assert (gpIfList);
  pIfList = gpIfList;
  nnListSetNodeCompFunc (pIfList, ifListCmp);
}

/* Description : 인터페이스 리스트 포인터의 메모리를 해제 함수. */
void 
ifClose ()
{
  assert (pIfList);
  nnListFree (pIfList);
}
