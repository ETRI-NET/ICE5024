/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : RIB Manager에서 Interface 추가/삭제, 상태 UP/DOWN, IP 주소 추가/삭제
 * 이벤트를 처리하는 과정에서 인터페이스에 설정된 IP 주소에 대하여 CONNECTED_ROUTE로 
 * 라우팅 테이블에 추가/삭제 등으로 관리하기 위한 기능들을 처리하는 함수들로 이루어진
 * 파일임.
 * - Block Name : RIB Manager
 * - Process Name : ribmgr
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : ribmgrConnected.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "nnTypes.h"
#include "nnStr.h"
#include "nnRibDefines.h"
#include "nnIf.h"
#include "nnPrefix.h"
#include "nnTable.h"
#include "nnList.h"
#include "nosLib.h"

#include "ribmgrRib.h"
#include "ribmgrRedistribute.h"
#include "ribmgrInterface.h"
#include "ribmgrConnected.h"
#include "ribmgrRouterid.h"


/*
 * Description : 커넥티드 주소를 삭제 하는 함수.
 *
 * param [in] pIfc : 커넥티드루트 자료구조 포인터
 */
static void
connectedWithdraw (ConnectedT *pIfc)
{
  if (! pIfc)
  {
    return;
  }

  /* Update interface address information to protocol daemon. */
  if (CHECK_FLAG (pIfc->conf, RIBMGR_IFC_REAL))
  {
    /* Delete RouterID */
    routerIdDelAddress(pIfc);

    /* Notify Interface Address Delete to Client */
    ribmgrInterfaceAddressDeleteUpdate (pIfc->pIf, pIfc);

    /* Delete Subnet Address of Interface */
    ifSubnetDelete (pIfc->pIf, pIfc);
      
    if (pIfc->pAddress->family == AF_INET)
    {
      connectedDownIpv4 (pIfc->pIf, pIfc);
    }
#ifdef HAVE_IPV6
    else
    {
      connectedDownIpv6 (pIfc->pIf, pIfc);
    }
#endif

      UNSET_FLAG (pIfc->conf, RIBMGR_IFC_REAL);
  }

  if (!CHECK_FLAG (pIfc->conf, RIBMGR_IFC_CONFIGURED))
  {
    /* So we should not free below function 
       because nnListDeleteNode function free pIfc memory pointer. */
    connectedFree (pIfc);

    /* The pIfc memory pointer will be free in this function. */
    nnListDeleteNode (pIfc->pIf->pConnected, pIfc);
   
  }
}


/*
 * Description : 커넥티드 주소를 처리하는 함수로, 인터페이스 플래그를 변경하고, 
 * 인터펭디스 주소를 기반으로 Router ID를 처리하고, 다른 컴포넌트로 인터페이스 
 * 주소 추가에 대한 이벤트 메시지를 전송한다.
 *
 * param [in] pIf : 인터페이스 자료구조 포인터
 * param [in] pIfc : 커넥티드루트 자료구조 포인터
 */
static void
connectedAnnounce (InterfaceT *pIf, ConnectedT *pIfc)
{
  if (!pIfc)
  {
    return;
  }
  
  nnListAddNode (pIf->pConnected, pIfc);

  /* Update interface address information to protocol daemon. */
  if (! CHECK_FLAG (pIfc->conf, RIBMGR_IFC_REAL))
  {
#if 0 // blocked by sckim
    if (pIfc->address->family == AF_INET)
      ifSubnetAdd (pIf, pIfc);
#endif

    SET_FLAG (pIfc->conf, RIBMGR_IFC_REAL);
     
    // add by sckim for router-id 
    routerIdAddAddress(pIfc);

    ribmgrInterfaceAddressAddUpdate (pIf, pIfc);

    if (ifIsOperative(pIf))
    {
      if (pIfc->pAddress->family == AF_INET)
      {
	    connectedUpIpv4 (pIf, pIfc);
      }
#ifdef HAVE_IPV6
      else
      {
        connectedUpIpv6 (pIf, pIfc);
      }
#endif
    }
  }
}


/*
 * Description : 동인한 인터페이스 주소가 존재 여부를 체크하는 함수.
 *
 * param [in] pIf : 인터페이스 자료구조 포인터
 * param [in] p : PrefixT 자료구조 포인터
 *
 * retval : 커넥티드 자료구조 포인터
 */
ConnectedT *
connectedCheck (InterfaceT *pIf, PrefixT *pPrefix)
{
  ListT * pConnectedList = NULL;
  ListNodeT * pConnectedNode = NULL;
  ConnectedT *pIfc = NULL;
 
  pConnectedList = pIf->pConnected;
  for(pConnectedNode = pConnectedList->pHead;
      pConnectedNode != NULL;
      pConnectedNode = pConnectedNode->pNext)
  {
    pIfc = pConnectedNode->pData;
    if (nnPrefixSame (pIfc->pAddress, pPrefix))
    {
      return pIfc;
    }
  } 

  return NULL;
}


/*
 * Description : 인터페이스의 목적지 주소가 동일한지를 확인하는 함수.
 *
 * param [in] pIfc1 : 커넥티드루트 자료구조 포인터
 * param [in] pIfc2 : 커넥티드루트 자료구조 포인터
 *
 * retval : 0 if 목적지 주소가 다른 경우
 *          1 if 목적지 주소가 같은 경우
 */
static Int32T
connectedSame (ConnectedT *pIfc1, ConnectedT *pIfc2)
{
  if (pIfc1->pIf != pIfc2->pIf)
  {
    return 0;
  }
  
  if (pIfc1->pDestination)
  {
    if (!pIfc2->pDestination)
    {
      return 0;
    }
  }

  if (pIfc2->pDestination)
  {
    if (!pIfc1->pDestination)
    {
      return 0;
    }
  }
  
  if (pIfc1->pDestination && pIfc2->pDestination)
  {
    if (!nnPrefixSame (pIfc1->pDestination, pIfc2->pDestination))
    {
      return 0;
    }
  }

  if (pIfc1->flags != pIfc2->flags)
  {
    return 0;
  }
  
  return 1;
}


/*
 * Description : 인터페이스의 connected 주소를 삭제하는 함수.
 * (Handle implicit withdrawals of addresses, where a system ADDs an address
 * to an interface which already has the same address configured.)
 *
 * param [in] pIf : 인터페이스 자료구조 포인터
 * param [in] pIfc : 커넥티드루트 자료구조 포인터
 *
 * retval : NULL if nothing to do
 *          connected 자료구조 포인터
 */
static ConnectedT *
connectedImplicitWithdraw (InterfaceT *pIf, ConnectedT *pIfc)
{
  ConnectedT *pConnected = NULL;

  /* Check same connected route. */
  if ((pConnected = connectedCheck (pIf, (PrefixT *) pIfc->pAddress)))
  {
    if (CHECK_FLAG(pConnected->conf, RIBMGR_IFC_CONFIGURED))
    {
      SET_FLAG(pIfc->conf, RIBMGR_IFC_CONFIGURED);
    }
	
    /* Avoid spurious withdraws, this might be just the kernel 'reflecting'
     * back an address we have already added.
     */
    if (connectedSame (pConnected, pIfc) && 
        CHECK_FLAG(pConnected->conf, RIBMGR_IFC_REAL))
    {
      /* nothing to do */
      connectedFree (pIfc);
      return NULL;
    }
      
    UNSET_FLAG(pConnected->conf, RIBMGR_IFC_CONFIGURED);
    connectedWithdraw (pConnected); /* implicit withdraw - freebsd does this */
  }

  return pIfc;
}



/* Description : 인터페이스에 커넥티드 IPv4 주소 추가를 처리하는 함수. */
void
connectedAddIpv4 (InterfaceT *pIf, Int32T flags, struct in_addr *pAddr, 
                  Uint8T prefixLen, struct in_addr *pBroad, const StringT label)
{
  Prefix4T *pPrefix4 = NULL;
  ConnectedT *pIfc = NULL;

  /* Make connected structure. */
  pIfc = connectedNew ();
  pIfc->pIf = pIf;
  pIfc->flags = flags;

  /* Allocate new connected address. */
  pPrefix4 = nnPrefixIpv4New ();
  pPrefix4->family = AF_INET;
  pPrefix4->prefix = *pAddr;
  pPrefix4->prefixLen = prefixLen;
  pIfc->pAddress = (PrefixT *) pPrefix4;

  /* If there is broadcast or peer address. */
  if (pBroad)
  {
    pPrefix4 = nnPrefixIpv4New ();
    pPrefix4->family = AF_INET;
    pPrefix4->prefix = *pBroad;
    pPrefix4->prefixLen = prefixLen;
    pIfc->pDestination = (PrefixT *) pPrefix4;

    /* validate the destination address */
    if (CONNECTED_PEER(pIfc))
    {
	  if (PREFIX_IPV4_ADDR_SAME(pAddr,pBroad))
	    NNLOG(LOG_DEBUG, "warning: interface %s has same local and peer "
		      "address %s, routing protocols may malfunction\n",
		      pIf->name,inet_ntoa(*pAddr));
    }
    else
    {
      struct in_addr chkAddr;
      nnGetBroadcastfromAddr (&chkAddr, pAddr, &prefixLen);
	  if (pBroad->s_addr != chkAddr.s_addr)
      {
        char buf[2][INET_ADDRSTRLEN];
        struct in_addr bcalc;
        nnGetBroadcastfromAddr (&bcalc, pAddr, &prefixLen);
        NNLOG(LOG_DEBUG, "warning: interface %s broadcast addr %s/%d != "
              "calculated %s, routing protocols may malfunction\n",
              pIf->name,
              inet_ntop (AF_INET, pBroad, buf[0], sizeof(buf[0])),
              prefixLen,
              inet_ntop (AF_INET, &bcalc, buf[1], sizeof(buf[1])));
      }
    }

  }
  else
  {
    if (CHECK_FLAG(pIfc->flags, RIBMGR_IFA_PEER))
    {
      NNLOG(LOG_DEBUG, "warning: %s called for interface %s "
            "with peer flag set, but no peer address supplied\n",
            __func__, pIf->name);
      UNSET_FLAG(pIfc->flags, RIBMGR_IFA_PEER);
    }

    /* no broadcast or destination address was supplied */
    if ((prefixLen == PREFIX_IPV4_MAX_PREFIXLEN) && ifIsPointPoint(pIf))
    {
      NNLOG(LOG_DEBUG, "warning: PtP interface %s with addr %s/%d needs a "
          "peer address\n",pIf->name,inet_ntoa(*pAddr),prefixLen);
    }
  }

  /* Label of this address. */
  if (label)
  {
    pIfc->label = nnStrDup (label, MEM_IF_LABEL);
  }

  /* nothing to do? */
  if ((pIfc = connectedImplicitWithdraw (pIf, pIfc)) == NULL)
  {
    return;
  }
  
  connectedAnnounce (pIf, pIfc);
}


/* Description : 인터페이스에 커넥티드 IPv4 주소 및 루트를  삭제하는 함수. */
void
connectedDeleteIpv4 (InterfaceT *pIf, Int32T flags, struct in_addr *pAddr,
                     Uint8T prefixLen, struct in_addr *pBroad)
{
  Prefix4T p = {0,};
  ConnectedT *pIfc = NULL;

  memset (&p, 0, sizeof (Prefix4T));
  p.family = AF_INET;
  p.prefix = *pAddr;
  p.prefixLen = prefixLen;

  pIfc = connectedCheck (pIf, (PrefixT *) &p);
  if (! pIfc)
    return;
    
  connectedWithdraw (pIfc);

#if 1 // blocked by sckim
  ribUpdate();
#endif
}


/* Description : 인터페이스의 커넥티드 IPv4 주소가 UP 되는 경우 처리하는 함수. */
void
connectedUpIpv4 (InterfaceT *pIf, ConnectedT *pIfc)
{
  Prefix4T pPrefix4 = {0,};

  if (! CHECK_FLAG (pIfc->conf, RIBMGR_IFC_REAL))
  {
    return;
  }

  PREFIX_COPY_IPV4(&pPrefix4, CONNECTED_PREFIX(pIfc));

  /* Apply mask to the network. */
  nnApplyNetmasktoPrefix4 (&pPrefix4, &pPrefix4.prefixLen);

  /* In case of connected address is 0.0.0.0/0 we treat it tunnel
     address. */
  if (nnPrefixCheckIpv4Any (&pPrefix4))
  {
    return;
  }

  ribAddIpv4 (RIB_ROUTE_TYPE_CONNECT, 0, &pPrefix4, NULL, NULL, pIf->ifIndex,
              RT_TABLE_MAIN, pIf->metric, 0);

#if 1 // blocked by sckim
  ribUpdate ();
#endif
}


/* Description : 인터페이스에 커넥티드 IPv4 주소를 삭제하고, 라우팅 테이블로  
 * 부터 CONNECTED_TYPE 루트를 삭제하는 함수. */
void
connectedDownIpv4 (InterfaceT *pIf, ConnectedT *pIfc)
{
  Prefix4T prefix4 = {0,};

  if (! CHECK_FLAG (pIfc->conf, RIBMGR_IFC_REAL))
  {
    return;
  }

  PREFIX_COPY_IPV4(&prefix4, CONNECTED_PREFIX(pIfc));

  /* Apply mask to the network. */
  nnApplyNetmasktoPrefix4 (&prefix4, &prefix4.prefixLen);

  /* In case of connected address is 0.0.0.0/0 we treat it tunnel
     address. */
  if (nnPrefixCheckIpv4Any (&prefix4))
  {
    return;
  }

  /* Same logic as for connectedUpIpv4(): push the changes into the head. */
  ribDeleteIpv4 (RIB_ROUTE_TYPE_CONNECT, 0, &prefix4, NULL, pIf->ifIndex, 0);

#if 1 // blocked by sckim
  ribUpdate ();
#endif
}


#ifdef HAVE_IPV6

/**
 * Description : 인터페이스에 커넥티드 IPv6 주소 및 루트를 추가하는 함수.
 *
 * @param [in] pIf : 인터페이스 자료구조 포인터
 * @param [in] flags : 인터페이스 플래그
 * @param [in] addr : IPv6 주소 자료구조 포인터
 * @param [in] prefixLen : IPv6 주소의 프리픽스 길이
 * @param [in] broad : 인터페이스의 브로드캐스트 IPv6 주소 포인터
 * @param [in] label : 인터페이스의 레이블 정보 문자열 포인터
 */
void
connectedAddIpv6 (InterfaceT *pIf, Int32T flags, struct in6_addr *pAddr,
                  Uint8T prefixLen, struct in6_addr *pBroad, 
                  const StringT label)
{
  Prefix6T *pPrefix6 = NULL;
  ConnectedT *pIfc = NULL;

  /* Make connected structure. */
  pIfc = connectedNew ();
  pIfc->pIf = pIf;
  pIfc->flags = flags;

  /* Allocate new connected address. */
  pPrefix6 = nnPrefixIpv6New ();
  pPrefix6->family = AF_INET6;
  PREFIX_IPV6_ADDR_COPY (&pPrefix6->prefix, pAddr);
  pPrefix6->prefixLen = prefixLen;
  pIfc->pAddress = (PrefixT *) pPrefix6;

  /* If there is broadcast or peer address. */
  if (pBroad)
  {
    if (IN6_IS_ADDR_UNSPECIFIED(pBroad))
    {
	  NNLOG(LOG_DEBUG, "warning: %s called for interface %s with unspecified "
		    "destination address; ignoring!\n", __func__, pIf->name);
    }
    else
	{
	  pPrefix6 = nnPrefixIpv6New ();
	  pPrefix6->family = AF_INET6;
	  PREFIX_IPV6_ADDR_COPY (&pPrefix6->prefix, pBroad);
	  pPrefix6->prefixLen = prefixLen;
	  pIfc->pDestination = (PrefixT *) pPrefix6;
	}
  }
  if (CHECK_FLAG(pIfc->flags, RIBMGR_IFA_PEER) && !pIfc->pDestination)
  {
    NNLOG(LOG_DEBUG, "warning: %s called for interface %s "
          "with peer flag set, but no peer address supplied\n",
          __func__, pIf->name);
    UNSET_FLAG(pIfc->flags, RIBMGR_IFA_PEER);
  }

  /* Label of this address. */
  if (label)
  {
    pIfc->label = nnStrDup (label, MEM_IF_LABEL);
  }
  
  if ((pIfc = connectedImplicitWithdraw (pIf, pIfc)) == NULL)
  {
    return;
  }
  
  connectedAnnounce (pIf, pIfc);
}


/* Description : 인터페이스에 커넥티드 IPv6 주소 및 루트를 삭제하는 함수. */
void
connectedDeleteIpv6 (InterfaceT *pIf, struct in6_addr *pAddr,
                     Uint8T prefixLen, struct in6_addr *pBroad)
{
  Prefix6T prefix6 = {0,};
  ConnectedT *pIfc = NULL;
  
  memset (&prefix6, 0, sizeof (Prefix6T));
  prefix6.family = AF_INET6;
  memcpy (&prefix6.prefix, pAddr, sizeof (struct in6_addr));
  prefix6.prefixLen = prefixLen;

  pIfc = connectedCheck (pIf, (PrefixT *) &prefix6);
  if (! pIfc)
  {
    return;
  }

  connectedWithdraw (pIfc);

#if 1 // blocked by sckim
  ribUpdate();
#endif
}


/* Description : 인터페이스에 커넥티드 IPv6 주소 및 루트를 추가하는 함수. */
void
connectedUpIpv6 (InterfaceT *pIf, ConnectedT *pIfc)
{
  Prefix6T prefix6 = {0,};

  if (! CHECK_FLAG (pIfc->conf, RIBMGR_IFC_REAL))
  {
    return;
  }

  PREFIX_COPY_IPV6(&prefix6, CONNECTED_PREFIX(pIfc));

  /* Apply mask to the network. */
  nnApplyNetmasktoPrefix6 (&prefix6, &prefix6.prefixLen);

#if ! defined (MUSICA) && ! defined (LINUX)
  /* XXX: It is already done by ribBogusIpv6 within ribAddIpv6 */
  if (IN6_IS_ADDR_UNSPECIFIED (&prefix6.prefix))
  {
    return;
  }
#endif

  ribAddIpv6 (RIB_ROUTE_TYPE_CONNECT, 0, &prefix6, NULL, pIf->ifIndex, 
              RT_TABLE_MAIN, pIf->metric, 0);

#if 1 // blocked by sckim
  ribUpdate ();
#endif
}


/* Description : 인터페이스에 커넥티드 IPv6 주소 및 루트를 삭제하는 함수. */
void
connectedDownIpv6 (InterfaceT *pIf, ConnectedT *pIfc)
{
  Prefix6T prefix6 = {0,};

  if (! CHECK_FLAG (pIfc->conf, RIBMGR_IFC_REAL))
  {
    return;
  }

  PREFIX_COPY_IPV6(&prefix6, CONNECTED_PREFIX(pIfc));

  nnApplyNetmasktoPrefix6 (&prefix6, &prefix6.prefixLen);

  if (IN6_IS_ADDR_UNSPECIFIED (&prefix6.prefix))
  {
    return;
  }

  ribDeleteIpv6 (RIB_ROUTE_TYPE_CONNECT, 0, &prefix6, NULL, pIf->ifIndex, 0);

#if 1 // blocked by sckim
  ribUpdate ();
#endif
}

#endif /* HAVE_IPV6 */
