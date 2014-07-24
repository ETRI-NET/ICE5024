/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : RIB Manager에서 각 프로토콜로 Route에 대한 Redistribute 기능을
 * 수행한다.  각 프로토콜이 Redistribution 기능을 수행하기 위하여 Static Route
 * 정보를 포함한 다른 프로토콜로부터생성 된 Route 정보를 RIB Manager 로 요청하고,
 * RIB Manager 로 부터 해당 Route 정보를 제공받기 위한 기능이다.
 *
 * - Block Name : RIB Manager
 * - Process Name : ribmgr
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : ribmgrRedistribute.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include <arpa/inet.h>

#include "nnRibDefines.h"
#include "nnPrefix.h"
#include "nnTable.h"
#include "nnUtility.h"
#include "nosLib.h"

#include "ribmgrInit.h"
#include "ribmgrRib.h"
#include "ribmgrRedistribute.h"
#include "ribmgrRouterid.h"
#include "ribmgrIpc.h"
#include "ribmgrInit.h"
#include "ribmgrDebug.h"
#include "ribmgrUtil.h"


/*
 * Description : 입력된 매개 변수의 프리픽스 값이 디폴트 루트임을 확인하는 함수.
 *
 * param [in] p : PrefixT pointer
 *
 * retval : 0 if 입력된 매개 변수의 프릭픽스 값이 디폴트 루트가 아니면,
 *           1 if 입력된 매개 변수의 프릭픽스 값이 디폴트 루트이면
 */
static Int32T
isDefault (PrefixT *pPrefix)
{
  if (pPrefix->family == AF_INET)
  {
    if (pPrefix->u.prefix4.s_addr == 0 && pPrefix->prefixLen == 0)
    {
      return 1;
    }
  }

#ifdef HAVE_IPV6
  /* IPv6 default separation is now pending until protocol daemon
     can handle that. */
  if (pPrefix->family == AF_INET6)
  {
    if (IN6_IS_ADDR_UNSPECIFIED (&pPrefix->u.prefix6) && 
        pPrefix->prefixLen == 0)
    {
      return 1;
    }
  }
#endif /* HAVE_IPV6 */

  return 0;
}


/*
 * Description : ???
 *
 * param [in] msg : ???
 */
static void
ribmgrRedistributeDefault (Uint8T componentId)
{
  Prefix4T prefix4 = {0,};
  RouteTableT * pTable = NULL;
  RouteNodeT * pRNode = NULL;
  RibT * pNewRib = NULL;
#ifdef HAVE_IPV6
  Prefix6T prefix6 = {0,};
#endif /* HAVE_IPV6 */

  NNLOG(LOG_DEBUG, "REDIST : %s %s %d called !!!\n", 
        __FILE__, __FUNCTION__, __LINE__);

  /* Lookup default route. */
  memset (&prefix4, 0, sizeof (Prefix4T));
  prefix4.family = AF_INET;

  /* Lookup table.  */
  pTable = vrfTable (AFI_IP, SAFI_UNICAST, 0);
  if (pTable)
  {
    pRNode = nnRouteNodeLookup (pTable, (PrefixT *)&prefix4);
    if (pRNode)
    {
      for (pNewRib = pRNode->pInfo; pNewRib; pNewRib = pNewRib->next)
        if (CHECK_FLAG (pNewRib->flags, RIB_FLAG_SELECTED) && 
            pNewRib->distance != DISTANCE_INFINITY)
        {
          ipcSendRouteToProtocol (componentId, IPC_RIB_IPV4_ROUTE_ADD, 
                                  &pRNode->p, pNewRib);
        }

      nnRouteNodeUnlock (pRNode);
    }
  }

#ifdef HAVE_IPV6
  /* Lookup default route. */
  memset (&prefix6, 0, sizeof (Prefix6T));
  prefix6.family = AF_INET6;

  /* Lookup table.  */
  pTable = vrfTable (AFI_IP6, SAFI_UNICAST, 0);
  if (pTable)
  {
    pRNode = nnRouteNodeLookup (pTable, (PrefixT *)&prefix6);
    if (pRNode)
	{
      for (pNewRib = pRNode->pInfo; pNewRib; pNewRib = pNewRib->next)
        if (CHECK_FLAG (pNewRib->flags, RIB_FLAG_SELECTED) && 
            pNewRib->distance != DISTANCE_INFINITY)
        {
          ipcSendRouteToProtocol (componentId, IPC_RIB_IPV6_ROUTE_ADD, 
                                  &pRNode->p, pNewRib);
        }

      nnRouteNodeUnlock (pRNode);
    }
  }
#endif /* HAVE_IPV6 */
}



/*
 * Description : ???
 *
 * param [in] msg : ???
 * param [in] type : ???
 */
/* Redistribute routes. */
static void
ribmgrRedistribute (Int32T componentId, Int32T requestedType)
{
  RibT *pNewRib = NULL;
  RouteTableT *pTable = NULL;
  RouteNodeT *pRNode = NULL;

  NNLOG(LOG_DEBUG, "REDIST : componentId = %d, requestedType = %s \n", 
                   componentId, (char *)nnRoute2String(requestedType));

  pTable = vrfTable (AFI_IP, SAFI_UNICAST, 0);
  if (pTable)
  {
    for (pRNode = nnRouteTop (pTable); pRNode; pRNode = nnRouteNext (pRNode))
    {
      for (pNewRib = pRNode->pInfo; pNewRib; pNewRib = pNewRib->next)
      {
        if (CHECK_FLAG (pNewRib->flags, RIB_FLAG_SELECTED) && 
            pNewRib->type == requestedType && 
            pNewRib->distance != DISTANCE_INFINITY && 
            ribCheckAddr (&pRNode->p))
        {
          ipcSendRouteToProtocol (componentId, IPC_RIB_IPV4_ROUTE_ADD, &pRNode->p, pNewRib);
        }
      }
    }
  }
  
#ifdef HAVE_IPV6
  pTable = vrfTable (AFI_IP6, SAFI_UNICAST, 0);
  if (pTable)
  {
    for (pRNode = nnRouteTop (pTable); pRNode; pRNode = nnRouteNext (pRNode))
    {
      for (pNewRib = pRNode->pInfo; pNewRib; pNewRib = pNewRib->next)
      {
        if (CHECK_FLAG (pNewRib->flags, RIB_FLAG_SELECTED) && 
            pNewRib->type == requestedType && 
            pNewRib->distance != DISTANCE_INFINITY && 
            ribCheckAddr (&pRNode->p))
        {
          ipcSendRouteToProtocol (componentId, RIB_IPV6_ROUTE_ADD, &pRNode->p, pNewRib);
        }
      }
    }
  }
#endif /* HAVE_IPV6 */
}


/**
 * Description : ???
 *
 * @param [in] msg : ???
 */
/* Redistribute routes. */
void
redistributeAdd (PrefixT *pPrefix, RibT *pRib)
{
  ListNodeT *  pNode = NULL;  
  ClientT * pClient = NULL;

  for (pNode = pRibmgr->pClientList->pHead;
       pNode != NULL;
       pNode = pNode->pNext)
  {
    pClient = pNode->pData;

    if (isDefault (pPrefix))
    {
      if (pClient->redistributeDefault || pClient->redistribute[pRib->type])
      {
        if (pPrefix->family == AF_INET)
        {
          ipcSendRouteToProtocol (pClient->componentId, 
                                  IPC_RIB_IPV4_ROUTE_ADD, pPrefix, pRib);
        }
#ifdef HAVE_IPV6
        if (p->family == AF_INET6)
        {
          ipcSendRouteToProtocol (pClient->componentId,
                                  IPC_RIB_IPV6_ROUTE_ADD, pPrefix, pRib);
        }
#endif /* HAVE_IPV6 */	  
	  }
    }
    else if (pClient->redistribute[pRib->type])
    {
      if (pPrefix->family == AF_INET)
      {
        ipcSendRouteToProtocol (pClient->componentId,
                                IPC_RIB_IPV4_ROUTE_ADD, pPrefix, pRib);
      }
#ifdef HAVE_IPV6
      if (pPrefix->family == AF_INET6)
      {
        ipcSendRouteToProtocol (pClient->componentId,
                                IPC_RIB_IPV6_ROUTE_ADD, pPrefix, pRib);
      }
#endif /* HAVE_IPV6 */	  
    }
    else
    {
      NNLOG(LOG_ERR, "Wrong pClient->redistribute[pRib->type] = %d\n", 
            pClient->redistribute[pRib->type]);
    }
  }
}


/**
 * Description : ???
 *
 * @param [in] msg : ???
 */
/* Redistribute routes. */
void
redistributeDelete (PrefixT *pPrefix, RibT *pRib)
{
  ListNodeT *  pNode = NULL;  
  ClientT * pClient = NULL;

  /* Add DISTANCE_INFINITY check. */
  if (pRib->distance == DISTANCE_INFINITY)
  {
    return;
  }

  for (pNode = pRibmgr->pClientList->pHead;
       pNode != NULL;
       pNode = pNode->pNext)
  {
    pClient = pNode->pData;

    if (isDefault (pPrefix))
    {
      if (pClient->redistributeDefault || pClient->redistribute[pRib->type])
      {
        if (pPrefix->family == AF_INET)
        {
          ipcSendRouteToProtocol (pClient->componentId,
                                  IPC_RIB_IPV4_ROUTE_DELETE, pPrefix, pRib);
        }
#ifdef HAVE_IPV6
        if (pPrefix->family == AF_INET6)
        {
          ipcSendRouteToProtocol (pClient->componentId,
                                  IPC_RIB_IPV6_ROUTE_DELETE, pPrefix, pRib);
        }
#endif /* HAVE_IPV6 */
      }
    }
    else if (pClient->redistribute[pRib->type])
    {
      if (pPrefix->family == AF_INET)
      {
        ipcSendRouteToProtocol (pClient->componentId,
                                IPC_RIB_IPV4_ROUTE_DELETE, pPrefix, pRib);
      }
#ifdef HAVE_IPV6
      if (pPrefix->family == AF_INET6)
      {
	    ipcSendRouteToProtocol (pClient->componentId,
                                IPC_RIB_IPV6_ROUTE_DELETE, pPrefix, pRib);
      }
#endif /* HAVE_IPV6 */
    }
    else
    {
      NNLOG(LOG_ERR, "Wrong pClient->redistribute[pRib->type] = %d\n", 
            pClient->redistribute[pRib->type]);
    }
  }
}


/**
 * Description : ???
 *
 * @param [in] msg : ???
 */
/* Redistribute routes. */
void
ribmgrRedistributeAdd (nnBufferT *msgBuff)
{
  Int32T componentId = 0; 
  Int32T requestedType = 0;
  ClientT * pClient = NULL;

  /* Get request's protocol type. */
  componentId = nnBufferGetInt8T (msgBuff);

  /* Get requested route type. */
  requestedType = nnBufferGetInt8T (msgBuff);

  /* Get client node. */
  pClient = clientGetById (componentId);
  if (!pClient)
  {
    NNLOG (LOG_ERR, "Not exist client[%d].\n", componentId);
    return;
  }

  switch (requestedType)
  {
    case RIB_ROUTE_TYPE_KERNEL:
    case RIB_ROUTE_TYPE_CONNECT:
    case RIB_ROUTE_TYPE_STATIC:
    case RIB_ROUTE_TYPE_RIP:
    case RIB_ROUTE_TYPE_RIPNG:
    case RIB_ROUTE_TYPE_OSPF:
    case RIB_ROUTE_TYPE_OSPF6:
    case RIB_ROUTE_TYPE_ISIS:
    case RIB_ROUTE_TYPE_BGP:
      if (! pClient->redistribute[requestedType])
      {
        pClient->redistribute[requestedType] = 1;
        ribmgrRedistribute (componentId, requestedType);
      }
      break;
    default:
      break;
  }
}     


/**
 * Description : ???
 *
 * @param [in] msg : ???
 */
/* Redistribute routes. */
void
ribmgrRedistributeDelete (nnBufferT * msgBuff)
{
  Int32T componentId = 0;
  Int32T requestedType = 0;
  ClientT * pClient = NULL;

  /* Get request's protocol type. */
  componentId = nnBufferGetInt8T (msgBuff);

  /* Get requested route type. */
  requestedType = nnBufferGetInt8T (msgBuff);

  /* Get client node. */
  pClient = clientGetById (componentId);
  if (!pClient)
  {
    NNLOG (LOG_ERR, "Not exist client[%d].\n", componentId);
    return;
  }

  NNLOG(LOG_DEBUG, 
        "componentId=%d, , requestedType=%d\n", componentId, requestedType);

  switch (requestedType)
  {
    case RIB_ROUTE_TYPE_KERNEL:
    case RIB_ROUTE_TYPE_CONNECT:
    case RIB_ROUTE_TYPE_STATIC:
    case RIB_ROUTE_TYPE_RIP:
    case RIB_ROUTE_TYPE_RIPNG:
    case RIB_ROUTE_TYPE_OSPF:
    case RIB_ROUTE_TYPE_OSPF6:
    case RIB_ROUTE_TYPE_ISIS:
    case RIB_ROUTE_TYPE_BGP:
      pClient->redistribute[requestedType] = 0;
      break;
    default:
      break;
  }
}     


/**
 * Description : 프로토콜이 RIB Manager 로 default route에 대하여
 * redistribute 요청함. 메시지를 처리하는 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 */
void
ribmgrRedistributeDefaultAdd (nnBufferT * msgBuff)
{
  ClientT * pClient = NULL;
  Uint8T componentId = 0;

  /* Get component id. */
  componentId = nnBufferGetInt8T (msgBuff);

  /* Lookup client pointer. */
  pClient = clientLookupById (componentId);
  if (!pClient)
  {
    NNLOG (LOG_ERR, "Not exist pClient, componentId = %d\n", componentId);
    return;
  }

  /* Set client's redistributeDefault field to 1. */ 
  pClient->redistributeDefault = 1;

  /* Process redistribute default. */
  ribmgrRedistributeDefault (pClient->componentId);
}     


/**
 * Description : 프로토콜이 RIB Manager 로 사전 설정한 모든 Redistribute 정보
 * 삭제를 요청함. 이 메시지를 처리하는 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 */
void
ribmgrRedistributeDefaultDelete (nnBufferT * msgBuff)
{
  ClientT * pClient = NULL;
  Uint8T componentId = 0;

  /* Get component id. */
  componentId = nnBufferGetInt8T (msgBuff);

  /* Lookup client pointer. */
  pClient = clientLookupById (componentId);

  /* Set client's redistributeDefault field to 1. */ 
  pClient->redistributeDefault = 0;
}     




/* Description : 인터페이스가 추가 되는 경우 각 프로토콜로 통지하는 함수. */
void
ribmgrInterfaceAddUpdate (InterfaceT *pIf)
{
  NNLOG (LOG_DEBUG, "MESSAGE: RIBMGR_INTERFACE_ADD %s\n", pIf->name);
  InterfaceUpdate (MESSAGE_EVENT, 0, EVENT_INTERFACE_ADD, pIf);
}


/* Description : 인터페이스가 삭제 되는 경우 각 프로토콜로 통지하는 함수. */
void
ribmgrInterfaceDeleteUpdate (InterfaceT *pIf)
{
  NNLOG (LOG_DEBUG, "MESSAGE: RIBMGR_INTERFACE_DELETE %s\n", pIf->name);
  InterfaceUpdate (MESSAGE_EVENT, 0, EVENT_INTERFACE_DELETE, pIf);
}


/* Description : 인터페이스 상태가 UP 되는 경우 각 프로토콜로 통지하는 함수. */
void
ribmgrInterfaceUpUpdate (InterfaceT *pIf)
{
  NNLOG (LOG_DEBUG, "MESSAGE: RIBMGR_INTERFACE_UP %s \n", pIf->name);
  InterfaceStatusUpdate (MESSAGE_EVENT, 0, EVENT_INTERFACE_UP, pIf);
}


/* Description : 인터페이스 상태가 DOWN 되는 경우 각 프로토콜로 통지하는 함수. */
void
ribmgrInterfaceDownUpdate (InterfaceT *pIf)
{
  NNLOG (LOG_DEBUG, "MESSAGE: RIBMGR_INTERFACE_DOWN %s\n", pIf->name);
  InterfaceStatusUpdate (MESSAGE_EVENT, 0, EVENT_INTERFACE_DOWN, pIf);  
}


/* Description : 인터페이스 주소가 추가되는 경우 각 프로토콜로 통지하는 함수. */
void
ribmgrInterfaceAddressAddUpdate (InterfaceT *pIf, ConnectedT *pIfc)
{
  PrefixT *pPrefix = NULL;
  char buf[INET6_ADDRSTRLEN] = {};

  pPrefix = pIfc->pAddress;
  NNLOG (LOG_DEBUG, "MESSAGE: RIBMGR_INTERFACE_ADDRESS_ADD %s/%d on %s\n",
          inet_ntop (pPrefix->family, &pPrefix->u.prefix, buf, INET6_ADDRSTRLEN),
          pPrefix->prefixLen, pIfc->pIf->name);

  routerIdAddAddress(pIfc);

  InterfaceAddressUpdate (MESSAGE_EVENT, 0, EVENT_INTERFACE_ADDRESS_ADD, pIf, pIfc);
}


/* Description : 인터페이스 주소가 삭제되는 경우 각 프로토콜로 통지하는 함수. */
void
ribmgrInterfaceAddressDeleteUpdate (InterfaceT *pIf, ConnectedT *pIfc)
{
  PrefixT *pPrefix = NULL;

  if (IS_RIBMGR_DEBUG_EVENT)
  {
    char buf[INET6_ADDRSTRLEN] = {};

    pPrefix = pIfc->pAddress;
    NNLOG(LOG_DEBUG,"MESSAGE: RIBMGR_INTERFACE_ADDRESS_DELETE %s/%d on %s\n",
          inet_ntop (pPrefix->family, &pPrefix->u.prefix, buf, 
          INET6_ADDRSTRLEN), pPrefix->prefixLen, pIfc->pIf->name);
  }

  routerIdDelAddress(pIfc);

  InterfaceAddressUpdate (MESSAGE_EVENT, 0, EVENT_INTERFACE_ADDRESS_DELETE, pIf, pIfc);
}
