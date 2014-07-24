/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : ribmgr에서 사용하는 Interface 정보를 저장 관리 하는 기능을 수행한다.
 * - Block Name : RIB Manager
 * - Process Name : ribmgr
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : ribmgrIf.c
 *
 * $Author: sckim007 $
 * $Date: 2014-02-17 09:53:56 +0900 (Mon, 17 Feb 2014) $
 * $Revision: 860 $
 * $LastChangedBy: sckim007 $
 */

//#include <net/if.h>
//#include <arpa/inet.h>

//#include "nnRibDefines.h"
#include "pal.h"

#include "interface.h"
//#include "ribmgrInterface.h"

/* Master list of interfaces. */
ListT * pIfList;

/* external definition */
extern size_t strlcpy(char *d, const char *s, size_t bufsize);
extern size_t strlcat(char *d, const char *s, size_t bufsize);


void
print_interface_list()
{
  ListNodeT * pNode = NULL;
  InterfaceT * pIf = NULL;

  PAL_DBUG(0, "\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
  PAL_DBUG(0, ">>> INTERFACE LIST :  %s %d \n", __FUNCTION__, __LINE__);
  PAL_DBUG(0, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

  Int32T i=0;
  for(pNode = pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pIf = pNode->pData;

    PAL_DBUG(0, "no[%d] ifname[%s] ifindex[%d]\n", i++, pIf->name, pIf->ifIndex);

    ConnectedT * connected;
    ListT * pConnectedList = pIf->connected;
    ListNodeT * pConnectedNode;

    for(pConnectedNode = pConnectedList->pHead;
        pConnectedNode != NULL;
        pConnectedNode = pConnectedNode->pNext);
    {   
      char address_str[INET6_ADDRSTRLEN]={};
      char destination_str[INET6_ADDRSTRLEN]={};
      PrefixT *p = NULL;
      connected = pConnectedNode->pData;
      if(connected->address)
      {   
        p = connected->address;
        if(p->family == AF_INET)
          inet_ntop (p->family, &p->u.prefix4, address_str, sizeof (address_str));
#ifdef HAVE_IPV6
        else if(p->family == AF_INET6)
          inet_ntop (p->family, &p->u.prefix6, address_str, sizeof (address_str));
#endif
      }   

      if(connected->destination)
      {   
        p = connected->destination;
        if(p->family == AF_INET)
          inet_ntop (p->family, &p->u.prefix4, destination_str, sizeof (destination_str));
#ifdef HAVE_IPV6
        else if(p->family == AF_INET6)
          inet_ntop (p->family, &p->u.prefix6, destination_str, sizeof (destination_str));
#endif
      }   

      PAL_DBUG(0, "\t address =%s, destination =%s\n", address_str, destination_str);
    } /* inner for loop */

  } /* outer for loop */
  PAL_DBUG(0, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
}


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
ifCreate (const char *name, Int32T namelen)
{
  InterfaceT *ifp;

  ifp = NNMALLOC (MEM_IF, sizeof (InterfaceT));
  ifp->ifIndex = IFINDEX_INTERNAL;

  
  assert (name);
  assert (namelen <= INTERFACE_NAMSIZ);	/* Need space for '\0' at end. */
  strncpy (ifp->name, name, namelen);
  ifp->name[namelen] = '\0';
  if (ifLookupByName(ifp->name) == NULL)
  {
    nnListAddNodeSort (pIfList, ifp);
  }
  else
    PAL_DBUG(0, "Err : ifCreate(%s): corruption detected -- interface with this "
	     "name exists already!", ifp->name);
  ifp->connected = nnListInit(NULL, MEM_CONNECTED);

  return ifp;
}


/* Description : 인터페이스를 삭제하는 함수. */
void
ifDeleteRetain (InterfaceT *ifp)
{
  /* Free connected address list */
  nnListFree(ifp->connected);
}


/* Description : 인터페이스를 삭제하고, 메모리 해제  함수. */
void
ifDelete (InterfaceT *ifp)
{
  ListNodeT * node;

  if ((node = nnListSearchNode (pIfList, ifp)))
  {
    ifDeleteRetain(ifp); // delete connected pointer
    nnListDeleteNode (pIfList, node); // delete interface
  }
}



/* Description : 리스트에서 인덱스를 기반으로 인터페이스를 검색하는 함수. */
InterfaceT *
ifLookupByIndex (Uint32T idx)
{
  ListNodeT * pNode = NULL;
  InterfaceT * pIf = NULL;

  /*
  PAL_DBUG(LOG_DEBUG, "enter \n");
  PAL_DBUG(LOG_DEBUG, "ifList ptr = %p\n", pIfList);
  PAL_DBUG(LOG_DEBUG, "List Count = %d\n", (int)nnListCount(pIfList));
  */

  for(pNode = pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pIf = pNode->pData;
    if(pIf->ifIndex == idx)
      return pIf;
  }

  return NULL;
}


/* Description : 리스트에서 인덱스를 기반으로 인터페이스 이름을 검색하는 함수. */
const char *
ifIndex2ifName (Uint32T idx)
{
  InterfaceT *pIf;

  return ((pIf = ifLookupByIndex(idx)) != NULL) ?
  	 pIf->name : "unknown";
}


/* Description : 리스트에서 인터페이스 이름을 기반으로 인터페이스 인덱스를 검색하는 함수. */
Uint32T
ifName2ifIndex (const char *name)
{
  InterfaceT * pIf;

  return ((pIf = ifLookupByName(name)) != NULL) ? pIf->ifIndex
                                                   : IFINDEX_INTERNAL;
}


/* Description : 리스트에서 인터페이스 이름을 기반으로 인터페이스를 검색하는 함수. */
InterfaceT *
ifLookupByName (const char *name)
{
  ListNodeT * pNode = NULL;
  InterfaceT * pIf = NULL;

  if (name)
  {
    for(pNode = pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
    {
      pIf = pNode->pData;
      if(strcmp(name, pIf->name) == 0){
  		klog(KER_DEBUG, "interface found ok: %s\n", name);
        return pIf;
	  }
    } /* for loop */
  } /* if */

  return NULL;
}


/* Description : 리스트에서 인터페이스 이름 및 길이를 기반으로 인터페이스를 검색하는 함수. */
InterfaceT *
ifLookupByNameLen(const char *name, size_t namelen)
{
  ListNodeT * pNode = NULL;
  InterfaceT * pIf = NULL;

  if (namelen > INTERFACE_NAMSIZ)
    return NULL;

  for(pNode = pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pIf = pNode->pData;
    if (!memcmp(name, pIf->name, namelen) && (pIf->name[namelen] == '\0'))
	  return pIf;
  } /* for loop */

  return NULL;
}


/* Description : 리스트에서 IPv4 주소를 기반으로 인터페이스를 검색하는 함수. */
InterfaceT *
ifLookupExactAddress (struct in_addr src)
{
  PrefixT *p;
  ConnectedT *c;

  ListNodeT * pNode = NULL;
  InterfaceT * pIf = NULL;

  for(pNode = pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pIf = pNode->pData;

    ListT * pConnectedList = pIf->connected;
    ListNodeT * pConnectedNode;
    for (pConnectedNode = pConnectedList->pHead;
         pConnectedNode != NULL;
         pConnectedNode = pConnectedNode->pNext)
	{
      c = pConnectedNode->pData;
      p = c->address;

      if (p && p->family == AF_INET)
      {
        if (PREFIX_IPV4_ADDR_SAME (&p->u.prefix4, &src))
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
  PrefixT addr;
  Int32T bestlen = 0;
  ConnectedT *c;
  InterfaceT *match;

  ListNodeT * pNode = NULL;
  InterfaceT * pIf = NULL;

  addr.family = AF_INET;
  addr.u.prefix4 = src;
  addr.prefixLen = PREFIX_IPV4_MAX_BITLEN;

  match = NULL;
  for(pNode = pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pIf = pNode->pData;

    ListT * pConnectedList = pIf->connected;
    ListNodeT * pConnectedNode;
    for (pConnectedNode = pConnectedList->pHead;
         pConnectedNode != NULL;
         pConnectedNode = pConnectedNode->pNext)
    {
      c = pConnectedNode->pData;
      if (c->address && (c->address->family == AF_INET) &&
          nnPrefixMatch(CONNECTED_PREFIX(c), &addr) &&
          (c->address->prefixLen > bestlen))
      {
        bestlen = c->address->prefixLen;
        match = pIf;
      } /* if */
    } /* inner for loop */
  } /* outer for loop */

  return match;
}


/* Description : 인터페이스 이름을 기반으로 인터페이스를 검색하고, 없는 경우
 * 인터페이스를 생성하여 포인터를 반환하는 함수. */
InterfaceT *
ifGetByName (const char *name)
{
  InterfaceT * pIf;

  return ((pIf = ifLookupByName(name)) != NULL) ? pIf :
	 ifCreate(name, strlen(name));
}


/* Description : 인터페이스 이름과 길이를 기반으로 인터페이스를 검색하는 함수. */
InterfaceT *
ifGetByNameLen(const char *name, size_t namelen)
{
  InterfaceT * pIf;

  return ((pIf = ifLookupByNameLen(name, namelen)) != NULL) ? pIf :
	 ifCreate(name, namelen);
}


/* Description : 인터페이스 플래그가 IFF_UP 임을 확인하는 함수. */
Int32T
ifIsUp (InterfaceT * pIf)
{
  return pIf->flags & IFF_UP;
}


/* Description : 인터페이스 플래그가 IFF_RUNNING 임을 확인하는 함수. */
Int32T
ifIsRunning (InterfaceT * pIf)
{
  return pIf->flags & IFF_RUNNING;
}


/* Description : 인터페이스 플래그가 Operative 상태 임을 확인하는 함수. */
Int32T
ifIsOperative (InterfaceT * pIf)
{
  return ((pIf->flags & IFF_UP) && (pIf->flags & IFF_RUNNING ));
}


/* Description : 인터페이스가 Loopback 임을 확인하는 함수. */
Int32T
ifIsLoopback (InterfaceT * pIf)
{
  /* XXX: Do this better, eg what if IFF_WHATEVER means X on platform M
   * but Y on platform N?
   */
  return (pIf->flags & (IFF_LOOPBACK|IFF_NOXMIT|IFF_VIRTUAL));
}


/* Description : 인터페이스가 Broadcast 타입이 설정되어 있는가를 확인하는 함수. */
Int32T
ifIsBroadcast (InterfaceT * pIf)
{
  return pIf->flags & IFF_BROADCAST;
}


/* Description : 인터페이스가 P2P 타입이 설정되어 있는가를 확인하는 함수. */
Int32T
ifIsPointPoint (InterfaceT * pIf)
{
  return pIf->flags & IFF_POINTOPOINT;
}


/* Description : 인터페이스가 Multicast 타입이 설정되어 있는가를 확인하는 함수. */
Int32T
ifIsMulticast (InterfaceT * pIf)
{
  return pIf->flags & IFF_MULTICAST;
}


/* Description : 인터페이스 플래그를 덤프하는 함수. */
const char *
ifFlagDump (Uint32T flag)
{
  Int32T separator = 0;
  static char logbuf[BUFSIZ];

#define IFF_OUT_LOG(X,STR) \
  if (flag & (X)) \
    { \
      if (separator) \
	strlcat (logbuf, ",", BUFSIZ); \
      else \
	separator = 1; \
      strlcat (logbuf, STR, BUFSIZ); \
    }

  strlcpy (logbuf, "<", BUFSIZ);
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

  strlcat (logbuf, ">", BUFSIZ);

  return logbuf;
#undef IFF_OUT_LOG
}

#define IF_TYPE_MAX 	15
static const char *IF_TYPES_STR[] __attribute__ ((unused))  =
{
	"IF_TYPE_UNKNOWN",
	"IF_TYPE_LOOPBACK",
	"IF_TYPE_ETHERNET",
	"IF_TYPE_HDLC",
	"IF_TYPE_PPP",
	"IF_TYPE_ATM",
	"IF_TYPE_FRELAY",
	"IF_TYPE_VLAN",
	"IF_TYPE_PORT",
	"IF_TYPE_AGGREGATE",
	"IF_TYPE_MANAGE",
	"IF_TYPE_IPIP",
	"IF_TYPE_GREIP",
	"IF_TYPE_IPV6IP",
	"IF_TYPE_6TO4",
	"IF_TYPE_ISATAP"
};

const char *
ifTypeDump (Uint16T type)
{
  if(type > IF_TYPE_MAX) type = 0;
  return IF_TYPES_STR[type];
}

static Int8T macBuffer[INTERFACE_HWADDR_MAX];
const char *
ifMacDump (Uint8T* mac)
{
	sprintf(macBuffer, "%02x:%02x:%02x:%02x:%02x:%02x",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return macBuffer;
}



/*
 * Description : 인터페이스 정보를 덤프하는 함수.
 *
 * @param [in] pIf : 인터페이스 자료구조 포인터
 */
static void
ifDump (InterfaceT * pIf)
{

  PAL_DBUG (0, "Interface %s idx %d metric %d mtu %d "
#ifdef HAVE_IPV6
             "mtu6 %d "
#endif /* HAVE_IPV6 */
             "flags %s hw-type %s hw-addr %s bw %d\n",
	     pIf->name, pIf->ifIndex, pIf->metric, pIf->mtu, 
#ifdef HAVE_IPV6
	     pIf->mtu6,
#endif /* HAVE_IPV6 */
	     ifFlagDump (pIf->flags), ifTypeDump(pIf->hwType), 
		 ifMacDump(pIf->hwAddr), pIf->bandwidth);

  ConnectedT *connected;

  ListT * pConnectedList = pIf->connected;
  ListNodeT *pConnectedNode;
  for (pConnectedNode = pConnectedList->pHead;
       pConnectedNode != NULL;
       pConnectedNode = pConnectedNode->pNext)
  {   
    char addrStr[INET6_ADDRSTRLEN]={};
    char destStr[INET6_ADDRSTRLEN]={};
    PrefixT *p = NULL;
    connected = pConnectedNode->pData;
    if(connected->address)
    {   
      p = connected->address;
      if(p->family == AF_INET)
        inet_ntop (p->family, &p->u.prefix4, addrStr, sizeof (addrStr));
#ifdef HAVE_IPV6
      else if(p->family == AF_INET6)
        inet_ntop (p->family, &p->u.prefix6, addrStr, sizeof (addrStr));
#endif
    }/* if */

    if(connected->destination)
    {   
      p = connected->destination;
      if(p->family == AF_INET)
        inet_ntop (p->family, &p->u.prefix4, destStr, sizeof (destStr));
#ifdef HAVE_IPV6
      else if(p->family == AF_INET6)
        inet_ntop (p->family, &p->u.prefix6, destStr, sizeof (destStr));
#endif
    }/* if */

    PAL_DBUG(0, "\t address =%s/%d, destination =%s\n", 
			addrStr, connected->address->prefixLen, destStr);
  } /* for loop */
}


/* Description : 인터페이스 정보를 덤프하는 함수. */
void
ifDumpAll (void)
{
  PAL_DBUG(0, "enter\n");
  ListNodeT * pNode;
  InterfaceT * pIf;

  if(pIfList->pHead == NULL) {
    PAL_DBUG(0, "--------------------------------------------------------------\n");
    PAL_DBUG(0, "    Empty Kernel Interfae List \n");
    PAL_DBUG(0, "--------------------------------------------------------------\n");
	return;
  }

  PAL_DBUG(0, "--------------------------------------------------------------\n");
  for(pNode = pIfList->pHead;pNode != NULL;pNode=pNode->pNext)
  {
    pIf = pNode->pData;
    ifDump(pIf);
  }
  PAL_DBUG(0, "--------------------------------------------------------------\n");
}


/* Description : 커넥티드 자료구조를 생성하는 함수. */
ConnectedT *
connectedNew (void)
{
  return NNMALLOC (MEM_CONNECTED, sizeof (ConnectedT));
}


/* Description : 커넥티드 자료구조를 메모리를 해제하는 함수. */
void
connectedFree (ConnectedT *connected)
{
  if (connected->address)
    nnPrefixFree (connected->address);

  if (connected->destination)
    nnPrefixFree (connected->destination);

  if (connected->label)
    NNFREE (MEM_CONNECTED, connected->label);

  NNFREE (MEM_CONNECTED, connected);
}


/* Description : 인터페이스 주소 자료구조를 로깅하는 함수. */
static void __attribute__ ((unused))
connectedLog (ConnectedT *connected, char *str)
{
  PrefixT *p;
  InterfaceT * pIf;
  char logbuf[BUFSIZ];
  char buf[BUFSIZ];
  
  pIf = connected->ifp;
  p = connected->address;

  if (p->family == AF_INET)
    snprintf (logbuf, BUFSIZ, "%s interface %s %s %s/%d ", 
	      str, pIf->name, nnCnvPrefixFamilytoString (p),
	      (char *)inet_ntop (p->family, &p->u.prefix4, buf, BUFSIZ),
	      p->prefixLen);
#ifdef HAVE_IPV6
  else if (p->family == AF_INET6)
    snprintf (logbuf, BUFSIZ, "%s interface %s %s %s/%d ", 
	      str, pIf->name, nnCnvPrefixFamilytoString (p),
	      (char *)inet_ntop (p->family, &p->u.prefix6, buf, BUFSIZ),
	      p->prefixLen);
#endif

  p = connected->destination;
  if (p)
  {
    if (p->family == AF_INET)
      strncat (logbuf, (char *)inet_ntop (p->family, &p->u.prefix4, buf, BUFSIZ),
             BUFSIZ - strlen(logbuf));
#ifdef HAVE_IPV6
    else if (p->family == AF_INET6)
      strncat (logbuf, (char *)inet_ntop (p->family, &p->u.prefix6, buf, BUFSIZ),
             BUFSIZ - strlen(logbuf));
#endif
  }
  PAL_DBUG(LOG_DEBUG, "%s \n", logbuf);
}


/* Description : PrefixT 자료구조 내의 IP 주소가 같은지를 확인하는 함수.
 *
 * @param [in] p1 : PrefixT 자료구조 포인터
 * @param [in] p2 : PrefixT 자료구조 포인터
 *
 * @retval : 1 if IP 주소가 같음,
 *           0 if IP 주소가 같지 않음
 */
static Int32T
connectedSamePrefix (PrefixT *p1, PrefixT *p2)
{
  if (p1->family == p2->family)
    {
      if (p1->family == AF_INET &&
	  PREFIX_IPV4_ADDR_SAME (&p1->u.prefix4, &p2->u.prefix4))
	return 1;
#ifdef HAVE_IPV6
      if (p1->family == AF_INET6 &&
	  IPV6_ADDR_SAME (&p1->u.prefix6, &p2->u.prefix6))
	return 1;
#endif /* HAVE_IPV6 */
    }
  return 0;
}


/* Description : Connected List에서 Prefix 값으로 ListNode를 삭제하는 함수. */
ConnectedT *
connectedDeleteByPrefix (InterfaceT * pIf, PrefixT *p)
{
  ConnectedT *ifc;

  /* In case of same prefix come, replace it with new one. */
  ListT * pConnectedList = pIf->connected;
  ListNodeT * pConnectedNode;
  for (pConnectedNode = pConnectedList->pHead; 
       pConnectedNode != NULL;
       pConnectedNode = pConnectedNode->pNext)
  {
    ifc = pConnectedNode->pData;

    if (connectedSamePrefix (ifc->address, p))
    {
      nnListDeleteNode(pConnectedList, ifc);
      return ifc;
    }
  }

  return NULL;
}


/* Description : 인터페이스에 설정된 주소에서 connected 정보를 얻는 함수. */
ConnectedT *
connectedLookupAddress (InterfaceT * pIf, struct in_addr dst)
{
  PrefixT addr;
  ConnectedT *c;
  ConnectedT *match;

  addr.family = AF_INET;
  addr.u.prefix4 = dst;
  addr.prefixLen = PREFIX_IPV4_MAX_BITLEN;


  ListT * pConnectedList = pIf->connected;
  ListNodeT * pConnectedNode;
  match = NULL;

  for (pConnectedNode = pConnectedList->pHead;
       pConnectedNode != NULL;
       pConnectedNode = pConnectedNode->pNext)
  {
    c = pConnectedNode->pData;
    if (c->address && (c->address->family == AF_INET) &&
        nnPrefixMatch(CONNECTED_PREFIX(c), &addr) &&
       (!match || (c->address->prefixLen > match->address->prefixLen)))
      match = c;
  }

  return match;
}


/* Description : 인터페이스에 설정된 주소에서 Prefix 정보를 기반으로 connected 
 * 정보를 얻는 함수. */
ConnectedT *
connectedAddByPrefix (InterfaceT * pIf, PrefixT *p, PrefixT *destination)
{
  ConnectedT *ifc;

  /* Allocate new connected address. */
  ifc = connectedNew ();
  ifc->ifp = pIf;

  /* Fetch interface address */
  ifc->address = nnPrefixNew();
  memcpy (ifc->address, p, sizeof(PrefixT));

  /* Fetch dest address */
  if (destination)
  {
    ifc->destination = nnPrefixNew();
    memcpy (ifc->destination, destination, sizeof(PrefixT));
  }

  /* Add connected address to the interface. */
  nnListAddNode (pIf->connected, ifc);

  return ifc;
}

#ifndef HAVE_IF_NAMETOINDEX
/* Description : 인터페이스 이름으로 인덱스를 찾는 함수. */
Uint32T
ifNametoIndex (const char *name)
{
  InterfaceT * pIf;
  //int len = strnlen(name, IFNAMSIZ);
  int len = strlen(name);

  return ((pIf = ifLookupByNameLen(name, len)) != NULL) ? pIf->ifIndex : 0;
  //return ((pIf = ifLookupByNameLen(name, strnlen(name, IFNAMSIZ))) != NULL) ? pIf->ifIndex : 0;
}
#endif

#ifndef HAVE_IF_INDEXTONAME
/* Description : 인덱스로 인터페이스 이름을 찾는 함수. */
char *
ifIndextoName (Uint32T ifindex, char *name)
{
  InterfaceT * pIf;

  if (!(pIf = ifLookupByIndex(ifindex)))
    return NULL;
  strncpy (name, pIf->name, IFNAMSIZ);
  return pIf->name;
}
#endif

/* Description : 인터페이스 정보를 저장하는 리스트를 초기화 하는 함수. */
void
ifInit (void)
{
  PAL_DBUG(0, "enter \n");
  pIfList = nnListInit (ifListCmp, MEM_IF); 
}
