/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : PIF Manager PAL Interface Management
 * - Block Name : PIF Manager
 * - Process Name : pifmgr
 * - Creator : Suncheul Kim updated by Seungwoo Woo
 * - Initial Date : 2014/04/10
 */

/**
 * @file        : connected.c
 *
 * $Author: $
 * $Date:  $
 * $Revision: $
 * $LastChangedBy: $
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "pal.h"
#include "interface.h"

/*
#include "nnTypes.h"
#include "nnRibDefines.h"
#include "nnPrefix.h"
#include "nnTable.h"
#include "nnList.h"
#include "nnLog.h"
#include "ribmgrIf.h"
#include "ribmgrRib.h"
#include "ribmgrRedistribute.h"
#include "ribmgrInterface.h"
#include "ribmgrConnected.h"
#include "ribmgrRouterid.h"
*/

/*
 * Description : 동인한 인터페이스 주소가 존재 여부를 체크하는 함수.
 *
 * param [in] ifp : 인터페이스 자료구조 포인터
 * param [in] p : PrefixT 자료구조 포인터
 *
 * retval : 커넥티드 자료구조 포인터
 */
ConnectedT *
connectedCheck (InterfaceT *ifp, PrefixT *p)
{
  ListT * pConnectedList;
  ListNodeT * pConnectedNode;
  ConnectedT *ifc;
 
  pConnectedList = ifp->connected;

  for(pConnectedNode = pConnectedList->pHead;
      pConnectedNode != NULL;
      pConnectedNode = pConnectedNode->pNext)
  {
    ifc = pConnectedNode->pData;
    if (nnPrefixSame (ifc->address, p))
      return ifc;
  } 

  return NULL;
}


/*
 * Description : 인터페이스의 목적지 주소가 동일한지를 확인하는 함수.
 *
 * param [in] ifc1 : 커넥티드루트 자료구조 포인터
 * param [in] ifc2 : 커넥티드루트 자료구조 포인터
 *
 * retval : 0 if 목적지 주소가 다른 경우
 *          1 if 목적지 주소가 같은 경우
 */
static Int32T
connectedSame (ConnectedT *ifc1, ConnectedT *ifc2)
{
  PAL_DBUG(LOG_DEBUG, "%s, %d ...called..\n", __func__, __LINE__);

  if (ifc1->ifp != ifc2->ifp)
    return 0;
  
  if (ifc1->destination)
    if (!ifc2->destination)
      return 0;
  if (ifc2->destination)
    if (!ifc1->destination)
      return 0;
  
  if (ifc1->destination && ifc2->destination)
    if (!nnPrefixSame (ifc1->destination, ifc2->destination))
      return 0;

  if (ifc1->flags != ifc2->flags)
    return 0;
  
  return 1;
}


/* Description : 인터페이스에 커넥티드 IPv4 주소 추가를 처리하는 함수. */
void
connectedAddIpv4 (InterfaceT *ifp, Int32T flags, struct in_addr *addr, 
		        Uint8T prefixLen, struct in_addr *broad, const char * label)
{
  Prefix4T *p;
  ConnectedT *ifc;

  /*
  PAL_DBUG(LOG_DEBUG, "%s, %d ...called..\n", __func__, __LINE__);
  PAL_DBUG(LOG_DEBUG, "addr = %s \n", inet_ntoa(*addr));
  */

  /* check already exist */
  ifc = connectedCheck (ifp, (PrefixT *) &p);
  if (ifc) {
  	PAL_DBUG(LOG_DEBUG, "addr(%s) already exist \n", inet_ntoa(*addr));
  	return;
  }

  /* Make connected structure. */
  ifc = connectedNew ();
  ifc->ifp = ifp;
  ifc->flags = flags;

  /* Allocate new connected address. */
  p = nnPrefixIpv4New ();
  p->family = AF_INET;
  p->prefix = *addr;
  p->prefixLen = prefixLen;
  ifc->address = (PrefixT *) p;

  PAL_DBUG(LOG_DEBUG, "prefix = %s \n", inet_ntoa(p->prefix));
  
  /* If there is broadcast or peer address. */
  if (broad)
  {
    p = nnPrefixIpv4New ();
    p->family = AF_INET;
    p->prefix = *broad;
    p->prefixLen = prefixLen;
    ifc->destination = (PrefixT *) p;
  }

  /* Label of this address. */
  if (label)
    ifc->label = strdup(label);

  /* add ifc to interface address list */
  nnListAddNode (ifp->connected, ifc);
  PAL_DBUG(LOG_DEBUG, "add connected list (count=%d)\n", (int)nnListCount(ifp->connected));

  /* trigger interface address add event */
  kernelEventIfAddrAdd (ifp, ifc); 
}


/* Description : 인터페이스에 커넥티드 IPv4 주소 및 루트를  삭제하는 함수. */
void
connectedDeleteIpv4 (InterfaceT *ifp, Int32T flags, struct in_addr *addr,
                     Uint8T prefixLen, struct in_addr *broad)
{
  Prefix4T p;
  ConnectedT *ifc;

  PAL_DBUG(LOG_DEBUG, "%s, %d called\n", __FUNCTION__, __LINE__);

  memset (&p, 0, sizeof (Prefix4T));
  p.family = AF_INET;
  p.prefix = *addr;
  p.prefixLen = prefixLen;

  ifc = connectedCheck (ifp, (PrefixT *) &p);
  if (!ifc)
  {
  	PAL_DBUG(LOG_DEBUG, "addr(%s) not exist \n", inet_ntoa(*addr));
  	return;
  }

  /* trigger interface address add event */
  kernelEventIfAddrDel (ifp, ifc); 

  /* delete ifc to interface address list */
  nnListRemoveNode (ifp->connected, ifc);
  connectedFree (ifc);
}


#ifdef HAVE_IPV6

/**
 * Description : 인터페이스에 커넥티드 IPv6 주소 및 루트를 추가하는 함수.
 *
 * @param [in] ifp : 인터페이스 자료구조 포인터
 * @param [in] flags : 인터페이스 플래그
 * @param [in] addr : IPv6 주소 자료구조 포인터
 * @param [in] prefixLen : IPv6 주소의 프리픽스 길이
 * @param [in] broad : 인터페이스의 브로드캐스트 IPv6 주소 포인터
 * @param [in] label : 인터페이스의 레이블 정보 문자열 포인터
 */
void
connectedAddIpv6 (InterfaceT *ifp, Int32T flags, struct in6_addr *addr,
		    Uint8T prefixLen, struct in6_addr *broad, const char * label)
{
  Prefix6T *p;
  ConnectedT *ifc;

  PAL_DBUG(LOG_DEBUG, "%s, %d ...called..\n", __FUNCTION__, __LINE__);

  /* Make connected structure. */
  ifc = connectedNew ();
  ifc->ifp = ifp;
  ifc->flags = flags;

  /* Allocate new connected address. */
  p = prefixIpv6New ();
  p->family = AF_INET6;
  IPV6_ADDR_COPY (&p->prefix, addr);
  p->prefixLen = prefixLen;
  ifc->address = (PrefixT *) p;

  /* If there is broadcast or peer address. */
  if (broad)
  {
    if (IN6_IS_ADDR_UNSPECIFIED(broad))
    {
	  PAL_DBUG(LOG_DEBUG, "warning: %s called for interface %s with unspecified "
		    "destination address; ignoring!\n", __func__, ifp->name);
    }
    else
	{
	  p = prefixIpv6New ();
	  p->family = AF_INET6;
	  IPV6_ADDR_COPY (&p->prefix, broad);
	  p->prefixLen = prefixLen;
	  ifc->destination = (PrefixT *) p;
	}
  }
  if (CHECK_FLAG(ifc->flags, RIBMGR_IFA_PEER) && !ifc->destination)
  {
    PAL_DBUG(LOG_DEBUG, "warning: %s called for interface %s "
          "with peer flag set, but no peer address supplied\n",
          __func__, ifp->name);
    UNSET_FLAG(ifc->flags, RIBMGR_IFA_PEER);
  }

  /* Label of this address. */
  if (label)
    ifc->label = XSTRDUP (MTYPE_CONNECTED_LABEL, label);
  
  if ((ifc = connectedImplicitWithdraw (ifp, ifc)) == NULL)
    return;
  
  connectedAnnounce (ifp, ifc);
}


/* Description : 인터페이스에 커넥티드 IPv6 주소 및 루트를 삭제하는 함수. */
void
connectedDeleteIpv6 (InterfaceT *ifp, struct in6_addr *address,
		       Uint8T prefixLen, struct in6_addr *broad)
{
  Prefix6T p;
  ConnectedT *ifc;
  
  PAL_DBUG(LOG_DEBUG, "%s, %d ...called..\n", __func__, __LINE__);

  memset (&p, 0, sizeof (Prefix6T));
  p.family = AF_INET6;
  memcpy (&p.prefix, address, sizeof (struct in6_addr));
  p.prefixLen = prefixLen;

  ifc = connectedCheck (ifp, (PrefixT *) &p);
  if (! ifc)
    return;

  connectedWithdraw (ifc);

#if 0 // blocked by sckim
  ribUpdate();
#endif
}

#endif /* HAVE_IPV6 */
