/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : ribmgr에서 사용하는 IPC 및 Event 메시지의 처리기능을
 * 수행한다.
 * - Block Name : RIB Manager
 * - Process Name : ribmgr
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : ribmgrIpcProc.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include <sys/ipc.h>
#include <sys/msg.h>

#include <net/if.h>

#include "nnRibDefines.h"
#include "nnStr.h"
#include "nnTable.h"
#include "nnBuffer.h"
#include "nnPrefix.h"
#include "nnUtility.h"
#include "nosLib.h"
#include "lcsService.h"

#include "ribmgrIpc.h"
#include "ribmgrRib.h"
#include "ribmgrRouterid.h"
#include "ribmgrRedistribute.h"
#include "ribmgrUtil.h"
#include "ribmgrInit.h"
#include "ribmgrDebug.h"

#define RIB_TABLE_DEFAULT 0

/* previous declaration of static functions */

/*
 * Lcs ipc message proc functions.
 */

/*
 * Lcs component's role assigned function.
 */
void
ipcLcsRibSetRole (void * msg, Uint32T msgLen)
{
  Int32T ret = 0;
  NNLOG (LOG_DEBUG, "LCS : %s called.\n", __func__);

  LcsSetRoleMsgT lcsSetRoleMsg = {0,};

  /* Message copy. */
  memcpy(&lcsSetRoleMsg, msg, sizeof(LcsSetRoleMsgT));

  /* Role 이 LCS_HA_QUIESCING 인 경우 Service 중지. */
  if (lcsSetRoleMsg.haState == LCS_HA_QUIESCING)
  {
    NNLOG (LOG_DEBUG, "LCS : lcsSetRoleMsg.haState == LCS_HA_QUIESCING \n");
  }

  /* Process Manager 에게 응답 수행. */
  ret = lcsResponse(lcsSetRoleMsg.processType, lcsSetRoleMsg.invocationId);
  if (ret == LCS_OK)
  {
    NNLOG (LOG_DEBUG, "LCS : lcsResponse Success \n");
  }
  else
  {
    NNLOG (LOG_DEBUG, "LCS : lcsResponse Failure \n");
  }
}


/*
 * Lcs component's terminate requested function.
 */
void
ipcLcsRibTerminate (void * msg, Uint32T msgLen)
{
  Int32T ret = 0;

  NNLOG (LOG_DEBUG, "LCS : %s called.\n", __func__);

  LcsTerminateMsgT lcsTerminateMsg = {0,};

  /* Message copy. */
  memcpy(&lcsTerminateMsg, msg, sizeof(LcsTerminateMsgT));

  /* Process Manager 에게 응답 수행. */
  ret = lcsResponse(lcsTerminateMsg.processType,
                    lcsTerminateMsg.invocationId);
  if (ret == LCS_OK)
  {
    NNLOG (LOG_DEBUG, "LCS : lcsResponse Success \n");
  }
  else
  {
    NNLOG (LOG_DEBUG, "LCS : lcsResponse Failure \n");
  }

  /* Call ripTermProcess(). */
  ribTermProcess (0);
}

/*
 * Lcs component's health check requested function.
 */
void
ipcLcsRibHealthcheck (void * msg, Uint32T msgLen)
{
  NNLOG (LOG_DEBUG, "LCS : %s called.\n", __func__);
  Int32T ret = 0;

  LcsHealthcheckRequestMsgT healthcheck = {0,};

  /* Message copy. */
  memcpy(&healthcheck, msg, sizeof(LcsHealthcheckRequestMsgT));

  /* Process Manager 에게 Response Message 전송 */
  ret = lcsResponse(healthcheck.processType, healthcheck.invocationId);
   if (ret == LCS_OK)
  {
    NNLOG (LOG_DEBUG, "LCS : lcsResponse Success \n");
  }
  else
  {
    NNLOG (LOG_DEBUG, "LCS : lcsResponse Failure \n");
  }

  /* Rip health check 수행. To do. */
  /*
   * If rip has some problem. send
   * lcsErrorReport(LCS_POLICY_MANAGER, LCS_ERR_BAD_OPERATION,
   *                LCS_COMPONENT_RESTART);
   */
}

/*
 * Lcs : component's error was occurred, this event will be received.
 */
void
ipcLcsRibEventComponentErrorOccured (void * msg, Uint32T msgLen)
{
  NNLOG (LOG_DEBUG, "LCS : %s called.\n", __func__);

  LcsErrorOccurredEventT errorOccurredEvent = {0,};

  /* Message copy. */
  memcpy(&errorOccurredEvent, msg, sizeof(errorOccurredEvent));

  /* Log error occurred component. */
  NNLOG (LOG_DEBUG, "LCS : error component = %d\n", errorOccurredEvent.processType);
}


/*
 * Lcs : Component 의 Service 동작 상태를 전달
 */
void
ipcLcsRibEventComponentServiceStatus (void * msg, Uint32T msgLen)
{
  NNLOG (LOG_DEBUG, "LCS : %s called.\n", __func__);

  LcsServiceStatusEventT serviceStatusEvent = {0,};

  /* Message copy. */
  memcpy(&serviceStatusEvent, msg, sizeof(LcsServiceStatusEventT));

  /* Log component's service. */
  NNLOG (LOG_DEBUG, "LCS : service status = %d\n", serviceStatusEvent.serviceStatus);
}



/*
 * Description : 프로토콜이 초기화 됨을 ribmgr로 알리는 함수.
 */
Int32T 
ipcRibClientInit (nnBufferT * msgBuff)
{
  Int8T componentId = 0;
  Int8T routingType = 0;
  ClientT * pClient = NULL;

  printf ("==> %s %d called.\n", __func__, __LINE__);

  nnBufferPrint(msgBuff);

  /* Component ID. */
  componentId = nnBufferGetInt8T (msgBuff);

  /* Routing Type. */
  routingType = nnBufferGetInt8T (msgBuff);

  NNLOG (LOG_DEBUG, "INTERFACE componentId = %d, routingType = %d\n", componentId, routingType);

  /* Add or get client infomation to list. */
  pClient = clientGetById (componentId);
  pClient->routeType = routingType;

  /*
   * Send Interface Information.
   */
  ListNodeT * pNode = NULL;
  InterfaceT * pIf = NULL;
  for (pNode = pRibmgr->pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pIf = pNode->pData;

    NNLOG (LOG_DEBUG, "INTERFACE name = %s, status = %d, flags = 0x%x \n", 
           pIf->name, pIf->status, pIf->flags);

    if (CHECK_FLAG (pIf->flags, IFF_RUNNING))

    /* Send IPC interface add message. */
    NNLOG (LOG_DEBUG, "INTERFACE : IPC_INTERFACE_ADD (%s) to client.\n", pIf->name);
    InterfaceUpdate (MESSAGE_IPC, componentId, IPC_INTERFACE_ADD, pIf);

    if (CHECK_FLAG (pIf->flags, IFF_UP) && 
        CHECK_FLAG (pIf->flags, IFF_RUNNING))
    {
      NNLOG (LOG_DEBUG, "INTERFACE : IPC_INTERFACE_UP (%s) to client.\n", pIf->name);
      /* Send IPC interface up message. */
      InterfaceStatusUpdate (MESSAGE_IPC, componentId, IPC_INTERFACE_UP, pIf);
    }

    ConnectedT * pConnected = NULL;
    ListT * pConnectedList = pIf->pConnected;
    ListNodeT * pConnectedNode = NULL;
    for (pConnectedNode = pConnectedList->pHead;
         pConnectedNode != NULL;
         pConnectedNode = pConnectedNode->pNext)
    {
      pConnected = pConnectedNode->pData;

      NNLOG (LOG_DEBUG, "INTERFACE : IPC_INTERFACE_ADDRESS_ADD (%s) to client\n", pIf->name);
      /* Send IPC interface address add message. */
      InterfaceAddressUpdate (MESSAGE_IPC, componentId, IPC_INTERFACE_ADDRESS_ADD,
                              pIf, pConnected);
    }
  }

  /*
   * Send Router ID information. 
   */
  Prefix4T routerId;
  memset (&routerId, 0, sizeof (Prefix4T));
  routerIdGet (&routerId);

  if (routerId.prefix.s_addr != 0)
  {
    nnBufferT sendBuff;
    nnBufferReset (&sendBuff);

    buildRouterId (&sendBuff, &routerId);

    ipcSendAsync (componentId, 
                  IPC_RIB_ROUTER_ID_SET, sendBuff.data, sendBuff.length);
  }

  return RIBMGR_OK;
}

/*
 * Description : 프로토콜이 종료됨을 ribmgr로 알리는 함수.
 */
Int32T 
ipcRibClientClose (nnBufferT * msgBuff)
{
  Int8T componentId = 0;
  Int8T routingType = 0;
  printf ("==> %s %d called.\n", __func__, __LINE__);

  nnBufferPrint(msgBuff);

  /* Component ID. */
  componentId = nnBufferGetInt8T (msgBuff);

  /* Component ID. */
  routingType = nnBufferGetInt8T (msgBuff);

  printf ("==> componentId = %d, routingType = %d\n", componentId, routingType);

  /* Delete client infomation from list. */
  clientDelete (componentId);

  /* Send response message. */
  nnBufferT resBuff;
  nnBufferReset (&resBuff);
  nnBufferSetInt8T (&resBuff, 1); /* Set dummy value. */
  
  ipcResponseSync(componentId, IPC_RIB_CLIENT_CLOSE, 
                  resBuff.data, resBuff.length);
  
  return RIBMGR_OK;
}

/*
 * Description : 프로토콜이 Graceful Restart 하는 시점에 RIB Manager 로 
 * 요청된 Route 는 설정된 시간 동안 유지 요청을 처리하는 함수. 
 */
Int32T 
ipcRibRoutePreserve (nnBufferT * msgBuff)
{
//  Uint32T length = nnBufferGetLength(msgBuff);
//  nnBufferPrint(msgBuff);

  return RIBMGR_OK;
}


/*
 * Description : 프로토콜이 RIB Manager 로 오래된 Route 삭제 요청하며,
 * 프로토콜이 요청하는 메시지를 수신 하였을때 처리하는 함수. 
 */
Int32T 
ipcRibRouteStaleRemove (nnBufferT * msgBuff)
{
//  Uint32T length = nnBufferGetLength(msgBuff);
//  nnBufferPrint(msgBuff);

  return RIBMGR_OK;
}


/*
 * Description : 프로토콜이 RIB Manager 로 모든 Route 삭제 요청하며, 
 * 프로토콜이 요청하는 메시지를 수신 하였을때 처리하는 함수. 
 */
Int32T 
ipcRibRouteClear (nnBufferT * msgBuff)
{
//  Uint32T length = nnBufferGetLength(msgBuff);
//  nnBufferPrint(msgBuff);

  return RIBMGR_OK;
}


/*
 * Description : 프로토콜이 RIB Manager 로  RouterID 설정을 요청하며, 
 * 프로토콜이 요청하는 메시지를 수신 하였을때 처리하는 함수. 
 */
Int32T 
ipcRibRouterIdSet (nnBufferT * msgBuff)
{
//  Uint32T length = nnBufferGetLength(msgBuff);
//  nnBufferPrint(msgBuff);
  if (IS_RIBMGR_DEBUG_EVENT)
  {
    NNLOG (LOG_WARNING, "Shouldn't reach this function(%s).\n", __func__);
  }

  return RIBMGR_OK;
}

/*
 * Description : 프로토콜에 의하여 Ipv4 Route 를 생성하여 RIB Manager 로 RIB 
 * 추가를 요청하며, RIB Manager는 Route Type을 읽어 Dynamic 인 경우 이함수를 
 * 호출함.
 *
 * param [in] msgBuff : 메시지 버퍼
 *
 * retval : 0 if 성공,
 *          1 if 경고,
 *          2 if 실패 
 */
static Int32T 
procDynamicRouteIpv4 (Uint16T command, Uint8T rType, nnBufferT * msgBuff)
{
  RibT * pRib = NULL; 
  Prefix4T p = {0,};
  struct in_addr nextHop = {0,};
  Uint32T ifIndex = 0;
  Uint8T message = 0, nhType = 0; 

  pRib = NNMALLOC(MEM_ROUTE_RIB, sizeof(RibT));

  NNLOG(LOG_DEBUG, "#######################################\n");
  NNLOG(LOG_DEBUG, " %s %d called.\n", __func__, __LINE__);
  NNLOG(LOG_DEBUG, "#######################################\n");

  pRib->type   = rType;
  pRib->flags  = nnBufferGetInt8T (msgBuff);
  message     = nnBufferGetInt8T (msgBuff);
  pRib->uptime = time(NULL);

  p.family = AF_INET;
  p.prefixLen = nnBufferGetInt8T (msgBuff);
  p.prefix = nnBufferGetInaddr (msgBuff);

  if(message & RIB_MESSAGE_NEXTHOP)
  {
    nhType = nnBufferGetInt8T(msgBuff);
    switch (nhType)
    {    
      case NEXTHOP_TYPE_IFINDEX :
        ifIndex = nnBufferGetInt32T(msgBuff);
        nexthopIfindexAdd(pRib, ifIndex);
        break;
      case NEXTHOP_TYPE_IFNAME :
        NNLOG(LOG_DEBUG, "Dynamic Route don't support IFNAME for nexthop\n");
        break;
      case NEXTHOP_TYPE_IPV4 :
        nextHop = nnBufferGetInaddr(msgBuff);
        nexthopIpv4Add(pRib, &nextHop, NULL);
        break;
      case NEXTHOP_TYPE_NULL0 :
        nexthopBlackholeAdd(pRib);
        break;
      default:
        NNLOG(LOG_ERR, "Wrong message's nexthop type =%d\n", nhType);
        break;
    }    
  }

  if(message & RIB_MESSAGE_DISTANCE)
    pRib->distance = nnBufferGetInt8T(msgBuff);

  if(message & RIB_MESSAGE_METRIC)
    pRib->metric = nnBufferGetInt32T(msgBuff);

  pRib->table = RIB_TABLE_DEFAULT;

  nnBufferPrint(msgBuff);

//  NNLOG(LOG_DEBUG, ":: type      = %d\n", pRib->type);
//  NNLOG(LOG_DEBUG, ":: message   = %d\n", message);
//  NNLOG(LOG_DEBUG, ":: prefixLen = %d\n", p.prefixLen);
//  NNLOG(LOG_DEBUG, ":: prefix    = %s\n", inet_ntoa(p.prefix));
//  NNLOG(LOG_DEBUG, ":: nhType    = %d\n", nhType);
//  NNLOG(LOG_DEBUG, ":: nhAddr    = %s\n", inet_ntoa(nextHop));
//  NNLOG(LOG_DEBUG, ":: distance  = %d\n", pRib->distance);
//  NNLOG(LOG_DEBUG, ":: metric    = %d\n", pRib->metric);

    
    fprintf(stderr, ":: type      = %d\n", pRib->type);
    fprintf(stderr, ":: message   = %d\n", message);
    fprintf(stderr, ":: prefixLen = %d\n", p.prefixLen);
    fprintf(stderr, ":: prefix    = %s\n", inet_ntoa(p.prefix));
    fprintf(stderr, ":: nhType    = %d\n", nhType);
    fprintf(stderr, ":: nhAddr    = %s\n", inet_ntoa(nextHop));
    fprintf(stderr, ":: ifindex   = %d\n", ifIndex);
    fprintf(stderr, ":: distance  = %d\n", pRib->distance);
    fprintf(stderr, ":: metric    = %d\n", pRib->metric);

  /* Dynamic Route Install to RIB table & Kernel */
  if (command == IPC_RIB_IPV4_ROUTE_ADD)
  {
    fprintf(stderr, ":: command = IPC_RIB_IPV4_ROUTE_ADD\n");
    ribAddIpv4Multipath(&p, pRib);
  }
  else if(command == IPC_RIB_IPV4_ROUTE_DELETE)
  {
    fprintf(stderr, ":: command = IPC_RIB_IPV4_ROUTE_DELETE\n");
    ribDeleteIpv4(pRib->type, pRib->flags, &p, &nextHop, ifIndex, pRib->table);
  }
  else
  {
    fprintf(stderr, ":: command = Unknown\n");
    NNLOG(LOG_ERR, "Error] Unknown Command, %s %d\n", __func__, __LINE__);
    return RIBMGR_NOK;
  }

  return RIBMGR_OK;
}


/*
 * Description : 프로토콜이 Ipv4 Route 를 생성하여 RIB Manager 로 RIB 추가를
 * 요청함. 
 */
Int32T 
ipcRibIpv4RouteAdd (nnBufferT * msgBuff)
{
  Int32T  i = 0;
  Uint16T rCount = 0;
  Uint8T  rType = 0;

  nnBufferPrint (msgBuff);  

  /* copy number of route entries */
  rCount = nnBufferGetInt16T (msgBuff);
  NNLOG(LOG_DEBUG, "number of route entries = %d\n", rCount);

  /* route type */
  rType = nnBufferGetInt8T (msgBuff);
  NNLOG(LOG_DEBUG, "route Type = %d\n", rType);

  /* process route info */
  for(i = 0 ; i < rCount ; i++)
  {
    nnBufferT subMsgBuff;
    nnBufferReset (&subMsgBuff);

    Uint16T subLength = nnBufferGetInt16T (msgBuff);
    nnBufferNCpy (&subMsgBuff, msgBuff, subLength);

    if(rType == RIB_ROUTE_TYPE_STATIC)
    {
      NNLOG(LOG_ERR, "Wrong route type. static route should be set by command.\n");
      return RIBMGR_NOK;
    }

    procDynamicRouteIpv4 (IPC_RIB_IPV4_ROUTE_ADD, rType, &subMsgBuff);
    NNLOG(LOG_DEBUG, "DYNAMIC ROUTE ADD CALLED\n");
  } /* for loop */

  nnBufferPrint(msgBuff);

  return RIBMGR_OK;
}


/*
 * Description : 프로토콜이 Ipv4 Route 를 삭제하기 위하여 RIB Manager 로 RIB 
 * 삭제를 요청함. 이 메시지르 처리하기 위한 함수임. 
 */
Int32T 
ipcRibIpv4RouteDelete (nnBufferT * msgBuff)
{
  Int32T  i = 0;
  Uint16T rCount = 0;
  Uint8T  rType = 0;

  nnBufferPrint (msgBuff);

  /* copy number of route entries */
  rCount = nnBufferGetInt16T (msgBuff);
  NNLOG(LOG_DEBUG, "number of route entries = %d\n", rCount);

  /* route type */
  rType = nnBufferGetInt8T (msgBuff);
  NNLOG(LOG_DEBUG, "route Type = %d\n", rType);

  /* process route info */
  for(i = 0 ; i < rCount ; i++) 
  {
    nnBufferT subMsgBuff;
    nnBufferReset (&subMsgBuff);

    Uint16T subLength = nnBufferGetInt16T (msgBuff);
    nnBufferNCpy (&subMsgBuff, msgBuff, subLength);

    if(rType == RIB_ROUTE_TYPE_STATIC)
    {    
      NNLOG(LOG_ERR, "Wrong route type. static route should be set by command.\n");
      return RIBMGR_NOK;
    }

    NNLOG(LOG_DEBUG, "DYNAMIC ROUTE DELETE CALLED\n");
    procDynamicRouteIpv4 (IPC_RIB_IPV4_ROUTE_DELETE, rType, &subMsgBuff);
  } /* for loop */

  return RIBMGR_OK;
}


/*
 * Description : 프로토콜이 Ipv4 Route 를 갱신하기 위하여 RIB Manager 로 RIB 
 * 갱신을 요청함. 이메시지를 처리하기 위한 함수임.
 */
Int32T 
ipcRibIpv4RouteUpdate (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);
  return RIBMGR_OK;
}

/*
 * Description : 프로토콜 또는 명령어로  Ipv6 Route를 생성하여 RIB Manager로
 * RIB 추가를 요청함. 이 메시지를 처리하기 위한 함수임.
 */
Int32T 
ipcRibIpv6RouteAdd (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);
  return RIBMGR_OK;
}

/*
 * Description : 프로토콜 또는 명령어로  Ipv6 Route를 삭제하기 위하여 
 * RIB Manager 로 RIB 삭제를 요청함. 이 메시지를 처리하기 위한 함수임.
 */
Int32T 
ipcRibIpv6RouteDelete (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);
  return RIBMGR_OK;
}

/*
 * Description : 프로토콜이 Ipv6 Route 를 갱신하기 위하여 RIB Manager 로 
 * RIB 갱신을 요청함.  이 메시지를 처리하기 위한 함수임.
 */
Int32T 
ipcRibIpv6RouteUpdate (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);
  return RIBMGR_OK;
}

/*
 * Description : 프로토콜 또는 명령어에 의하여 RIB Manager 로 Ipv4 
 * Best-Mach Route 에 대한 Lookup 요청함. 이 메시지를 처리하기 위한 함수임.
 */
Int32T 
ipcRibIpv4NexthopBestLookup (nnBufferT * msgBuff)
{
  RibT * pRib = NULL;
  NexthopT *pNextHop = NULL;
  struct in_addr addr4;
  Uint32T position = 0;
  Uint8T num = 0;
  Uint8T  procNum = 0;
  
//  NNLOG(LOG_DEBUG, "receive message : nexthop best lookup from %s\n",
//        gProcessTypes[procNum].string);

  procNum = nnBufferGetInt8T (msgBuff);
  addr4 = nnBufferGetInaddr (msgBuff);

  /* reset buffer */
  nnBufferReset(msgBuff);

  /* set requested ipv4 address to nnBufferT*/
  nnBufferSetInaddr(msgBuff, addr4);

  /* lookup rib from ipv4 address */
  pRib = ribMatchIpv4(addr4);
  if (pRib)
  {
    /* set metric value to nnBufferT*/
    nnBufferSetInt32T(msgBuff, pRib->metric);
    
    /* assign position */
    position = nnBufferGetIndex(msgBuff);

    /* set dummy value to nnBufferT */
    nnBufferSetInt8T(msgBuff, 0);

    for (pNextHop = pRib->pNexthop; pNextHop; pNextHop = pNextHop->next)
    {
      if (CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB))
      {
        /* set nexthop type to nnBufferT */
        nnBufferSetInt8T (msgBuff, pNextHop->type);
        /* set nexthop value */
        switch (pNextHop->type)
        {
          case NEXTHOP_TYPE_IPV4 :
            nnBufferSetInaddr(msgBuff, pNextHop->gate.ipv4);
            break;
          case NEXTHOP_TYPE_IFINDEX :
          case NEXTHOP_TYPE_IFNAME :
            nnBufferSetInt32T(msgBuff, pNextHop->ifIndex);
            break;
          default:
            NNLOG(LOG_ERR, "Wrong nexthop type = %d\n", pNextHop->type);
            break;
        }
        num++;
      }
    } /* for loop */

    /* set number of nexthop to assigned position */
    nnBufferInsertInt8T(msgBuff, position, num);
  }
  else
  {
    nnBufferSetInt32T (msgBuff, 0); /* metric */
    nnBufferSetInt8T(msgBuff, 0); /* number of nexthop */
  }
 
//  NNLOG(LOG_DEBUG, "send response message : nexthop best lookup to %s\n",
//        gProcessTypes[procNum].string);
 
  /* send response message by sync ipc */
  ipcResponseSync(procNum, IPC_RIB_IPV4_NEXTHOP_BEST_LOOKUP, 
                  msgBuff->data, msgBuff->length);

  return RIBMGR_OK;
}

/*
 * Description : 프로토콜 또는 명령어에 의하여  RIB Manager 로 Ipv4 
 * Exact-Mach Route 에 대한 Lookup 요청함. 이 메시지를 처리하는 함수임.
 */
Int32T 
ipcRibIpv4NexthopExactLookup (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);
  return RIBMGR_OK;
}


/*
 * Description : 프로토콜 또는 명령어에 의하여  RIB Manager 로 Ipv6 
 * Best-Mach Route 에 대한 Lookup 요청함. 이 메시지를 처리하는 함수임.
 */
Int32T 
ipcRibIpv6NexthopBestLookup (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);
  return RIBMGR_OK;
}


/*
 * Description : 프로토콜 또는 명령어에 의하여  RIB Manager 로 Ipv6 
 * Exact-Mach Route 에 대한 Lookup 요청함. 이메시지를 처리하는 함수임.
 */
Int32T 
ipcRibIpv6NexthopExactLookup (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);

  return RIBMGR_OK;
}


/*
 * Description : 프로토콜이 RIB Manager 로 관심 있는 Redistribute Route 정보 
 * 추가를 요청함. 이메시지를 처리하는 함수임.
 */
Int32T 
ipcRibRedistributeAdd (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);

  ribmgrRedistributeAdd (msgBuff);
  
  return RIBMGR_OK;
}

/*
 * Description : 프로토콜이 RIB Manager 로 관심 있는 Redistribute Route 정보 
 * 삭제를 요청함. 이 메시지를 처리하는 함수임.
 */
Int32T 
ipcRibRedistributeDelete (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);

  ribmgrRedistributeDelete (msgBuff);
  
  return RIBMGR_OK;
}


/*
 * Description : 
 */
Int32T 
ipcRibRedistributeDefaultAdd (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);

  ribmgrRedistributeDefaultAdd (msgBuff);
  
  return RIBMGR_OK;
}

/*
 * Description : 
 */
Int32T 
ipcRibRedistributeDefaultDelete (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);

  ribmgrRedistributeDefaultDelete (msgBuff);
  
  return RIBMGR_OK;
}


/*
 * Description : 프로토콜이 RIB Manager 로 사전 설정한 모든 Redistribute 정보
 * 삭제를 요청함. 이 메시지를 처리하는 함수임.
 */
Int32T 
ipcRibRedistributeClear (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);
  
  return RIBMGR_OK;
}



/*
 * Description : 프로토콜이 관심 있는 Ipv4 Best-Mach Route 를 RIB Manager
 * 에 등록 요청 메시지를 처리하는 함수임.
 */
Int32T 
ipcRibIpv4NexthopBestLookupReg (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);

  return RIBMGR_OK;
}


/*
 * Description : 프로토콜이 등록한 Ipv4 Best-Mach Route 를 RIB Manager
 * 에서 삭제 요청 메시지를 처리하는 함수임.
 */
Int32T 
ipcRibIpv4NexthopBestLookupDereg (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);
  return RIBMGR_OK;
}


/*
 * Description : 프로토콜이 관심 있는 Ipv4 Exact-Mach Route 를 RIB Manager
 * 에 등록 요청 메시지를 처리하는 함수임.
 */
Int32T 
ipcRibIpv4NexthopExactLookupReg (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);
  return RIBMGR_OK;
}


/*
 * Description : 프로토콜이 등록한 Ipv4 Exact-Mach Route 를 RIB Manager
 * 에서 삭제 요청 메시지를 처리하는 함수임.
 */
Int32T 
ipcRibIpv4NexthopExactLookupDereg (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);
  return RIBMGR_OK;
}


/*
 * Description : 프로토콜이 관심 있는 Ipv4 Route 를 RIB Manager에 등록한
 * 경우, 해당 Route 가 추가되었을 때 RIB Manager 가 해당 프로토콜로 
 * Notification 통보 ??? (프로토콜의 IPC 메시지 처리하는 부분..)
 */
Int32T 
ipcRibIpv4RouteNotificationAdd (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);
  return RIBMGR_OK;
}


/*
 * Description : 프로토콜이 관심 있는 Ipv4 Route 를 RIB Manager에 등록한
 * 경우, 해당 Route 가 삭제되었을 때 RIB Manager 가 해당 프로토콜로
 * Notification 통보 ??? (프로토콜의 IPC 메시지 처리하는 부분..)
 */
Int32T 
ipcRibIpv4RouteNotificationDelete (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);
  return RIBMGR_OK;
}


/* 
 * Description : 프로토콜이 관심 있는 Ipv6 Best-Mach Route 를 RIB Manager에
 * 등록 요청 메시지를 처리하는 함수임.
 */
Int32T 
ipcRibIpv6NexthopBestLookupReg (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);
  return RIBMGR_OK;
}


/*
 * Description : 프로토콜이 등록한 Ipv6 Best-Mach Route 를 RIB Manager 
 * 에서 삭제 요청 메시지를 처리하는 함수임.
 */
Int32T 
ipcRibIpv6NexthopBestLookupDereg (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);
  return RIBMGR_OK;
}


/*
 * Description : 프로토콜이 관심 있는 Ipv6 Exact-Mach Route 를 RIB Manager
 * 에 등록 요청 메시지를 처리하는 함수임.
 */
Int32T 
ipcRibIpv6NexthopExactLookupReg (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);
  return RIBMGR_OK;
}

/*
 * Description : 프로토콜이 등록한 Ipv6 Exact-Mach Route 를 RIB Manager
 * 에서 삭제 요청 메시지를 처리하는 함수임.
 */
Int32T 
ipcRibIpv6NexthopExactLookupDereg (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);
  return RIBMGR_OK;
}

/*
 * Description : 프로토콜이 관심 있는 Ipv6 Route 를 RIB Manager에 등록한 
 * 경우, 해당 Route 가 추가되었을 때 RIB Manager 가 해당 프로토콜로 
 * Notification 통보 ??? (프로토콜의 IPC 메시지 처리하는 부분..)
 */
Int32T 
ipcRibIpv6RouteNotificationAdd (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);
  return RIBMGR_OK;
}

/*
 * Description : 프로토콜이 관심 있는 Ipv6 Route 를 RIB Manager에 등록한 
 * 경우, 해당 Route 가 삭제되었을 때 RIB Manager 가 해당 프로토콜로 
 * Notification 통보 ??? (프로토콜의 IPC 메시지 처리하는 부분..)
 */
Int32T 
ipcRibIpv6RouteNotificationDelete (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);
  return RIBMGR_OK;
}

/*
 * Description : 프로토콜이 관심 있는 Ipv6 Route 를 RIB Manager에 등록한 
 * 경우, 해당 Route 가 변경되었을 때 RIB Manager 가 해당 프로토콜로 
 * Notification 통보 ??? (프로토콜의 IPC 메시지 처리하는 부분..)
 */
Int32T 
ipcRibIpv6RouteNotificationUpdate (nnBufferT * msgBuff)
{
  Uint32T length = nnBufferGetLength(msgBuff);
  NNLOG(LOG_DEBUG, "%s %d, length=%d\n", __FUNCTION__, __LINE__, length);
  nnBufferPrint (msgBuff);
  return RIBMGR_OK;
}



/**************************************************************************
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |      type     | <== pRib->type
 * +-+-+-+-+-+-+-+-+
 * |      flags    | <== pRib->flags
 * +-+-+-+-+-+-+-+-+
 * |   messages    | <== nexthop flag (postioning & set)
 * +-+-+-+-+-+-+-+-+
 * | Prefix Length |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                   Prefix Address IPV4/6                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *                                ~
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    NH count   | <== nexthop count (positioning & set)
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                    Gateway IPv4/6 Address                     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |        1      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                           IfIndex                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *                                ~
 *          << below 2 fields is used, when add route case >>
 *
 * +-+-+-+-+-+-+-+-+
 * |    Distance   |  <== in case of 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                            Metric                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 *
 ******************************************************************/
void
ipcSendRouteToProtocol (Int32T componentId, Int32T cmd, PrefixT *pPrefix, 
                        RibT * pRib)
{
  NNLOG(LOG_DEBUG, "REDIST :: cmd=%d, componentId=%d, pPrefix=%p, pRib=%p\n",
                      cmd, componentId, pPrefix, pRib);

  nnBufferT msgBuff;
  nnBufferReset (&msgBuff);

  NexthopT * pNexthop = NULL;
  Uint32T posNhCnt = 0, posMessages = 0; 
  Int32T nhCnt = 0; 
  Uint8T  zapi_flags = 0; 

  /* Set type and nexthop. */
  nnBufferSetInt8T (&msgBuff, pRib->type);
  nnBufferSetInt8T (&msgBuff, pRib->flags);

  /* Positioning for message flags field. */
  posMessages = nnBufferGetIndex (&msgBuff);
  nnBufferSetInt8T (&msgBuff, 0);

  /* Set prefix field. */
  //nnBufferSetInt8T (&msgBuff, pPrefix->family);
  nnBufferSetInt8T (&msgBuff, pPrefix->prefixLen);
  if (pPrefix->family == AF_INET)
  {
    nnBufferSetInaddr (&msgBuff, pPrefix->u.prefix4);
  }
#ifdef HAVE_IPV6
  else if (pPrefix->family == AF_INET6)
  {
    nnBufferSetInaddr6 (&msgBuff, pPrefix->u.prefix6);
  }
#endif
  else
  {
    NNLOG (LOG_ERR, "Wrong prefix family = %d\n", pPrefix->family);
    return;
  }

  /* Set nexthop fields. */
  for (pNexthop = pRib->pNexthop; pNexthop; pNexthop = pNexthop->next)
  {
    if (CHECK_FLAG (pNexthop->flags, NEXTHOP_FLAG_FIB))
    {
      SET_FLAG (zapi_flags, RIB_MESSAGE_NEXTHOP);
      SET_FLAG (zapi_flags, RIB_MESSAGE_IFINDEX);

      if (posNhCnt == 0)
      {
        posNhCnt = nnBufferGetIndex (&msgBuff);
        nnBufferSetInt8T (&msgBuff, 1); /* set default nexthop count */
      }

      nhCnt++;

      switch(pNexthop->type)
      {
        case NEXTHOP_TYPE_IPV4:
        case NEXTHOP_TYPE_IPV4_IFINDEX:
          nnBufferSetInaddr (&msgBuff, pNexthop->gate.ipv4);
          //stream_put_in_addr (s, &nexthop->gate.ipv4);
          break;
#ifdef HAVE_IPV6
        case NEXTHOP_TYPE_IPV6:
        case NEXTHOP_TYPE_IPV6_IFINDEX:
        case NEXTHOP_TYPE_IPV6_IFNAME:
          nnBufferSetInaddr6 (&msgBuff, &pNexthop->gate.ipv6);
          //stream_write (s, (u_char *) &pNexthop->gate.ipv6, 16);
          break;
#endif
        default:
        {
          if (cmd == IPC_RIB_IPV4_ROUTE_ADD || cmd == IPC_RIB_IPV4_ROUTE_DELETE)
          {
            struct in_addr zeroAddr;
            memset (&zeroAddr, 0, sizeof (struct in_addr));
            nnBufferSetInaddr (&msgBuff, zeroAddr);
            //stream_write (s, (u_char *) &empty, IPV4_MAX_BYTELEN);
          }
          else
          {
            struct in6_addr zeroAddr;
            memset (&zeroAddr, 0, sizeof (struct in6_addr));
            nnBufferSetInaddr6 (&msgBuff, zeroAddr);
            //stream_write (s, (u_char *) &empty, IPV6_MAX_BYTELEN);
          }
        }
      }

      /* Interface index. */
      nnBufferSetInt8T (&msgBuff, 1);
      nnBufferSetInt32T (&msgBuff, pNexthop->ifIndex);
      //stream_putc (s, 1);
      //stream_putl (s, nexthop->ifindex);

      break;
    }
  }

  /* Set distance and metric field. */
  if (cmd == IPC_RIB_IPV4_ROUTE_ADD || IPC_RIB_IPV6_ROUTE_ADD)
  {
    SET_FLAG (zapi_flags, RIB_MESSAGE_DISTANCE);
    nnBufferSetInt8T (&msgBuff, pRib->distance);
    //stream_putc (s, rib->distance);
    SET_FLAG (zapi_flags, RIB_MESSAGE_METRIC);

    nnBufferSetInt32T (&msgBuff, pRib->metric);
    //stream_putl (s, rib->metric);
  }
  
  /* Set real message flags value. */
  nnBufferInsertInt8T (&msgBuff, posMessages, zapi_flags);
  //stream_putc_at (s, messmark, zapi_flags);

  /* Set real nexthop count. */  
  if (posNhCnt)
  {
    nnBufferInsertInt8T (&msgBuff, posNhCnt, nhCnt);
    //stream_putc_at (s, nhnummark, nhnum);
  }

  /* Print buffer. */
  nnBufferPrint (&msgBuff);

  /* Send ip message to relative protocol. */
  ipcSendAsync(componentId, cmd, msgBuff.data, msgBuff.length);
}


/**************************************************************************
 * Description : 인터페이스 설정/삭제 시점에서 각 프로토콜로 인터페이스
 * 정보를 전달하기 위하여 이벤트 메시지를 전송하는 함수임.
 *
 * @param [in] cmd : 명령(EVENT_INTERFACE_ADD or EVENT_INTERFACE_DELETE)
 * @param [in] pIf : 인터페이스 자료구조 포인터
 * @retval : 0 if 성공,
 *          0 < if 실패
 *
 * Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | Name Length   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | Name String                       | 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      Interface Index                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    Status     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                        Flags(Uint64T)                         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                            Metric                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                             MTU                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                             MTU6                              |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                          Bandwidth                            |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |           HW Type                 |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                        HW Addr Length                         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                          HW Address                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *************************************************************************/
Int32T
InterfaceUpdate(Uint32T ipcType, Int8T componentId, 
                     Uint32T cmd, InterfaceT *pIf)
{
  /* check pointer is not null*/
  assert(pIf);

  printf ("======> event : cmd[%d] %s %d\n", cmd, __func__, __LINE__);

  if (IS_RIBMGR_DEBUG_EVENT)
  {
    char addrBuff[1024] = {};
    Int32T idx = 0;
    int i = 0;

    for (i=0;i<pIf->hwAddrLen;i++)
    {
      idx += sprintf(addrBuff +idx, "%02x:", pIf->hwAddr[i]);
    }

    NNLOG(LOG_DEBUG, "hwType[%d] hwAddrLen[%d] hwAddr[%s]\n",
          pIf->hwType, pIf->hwAddrLen, addrBuff);
  }

  nnBufferT msgBuff;

  /* Buffer Reset */
  nnBufferReset(&msgBuff);

  /*
   * Add each value to buffer 
   */

  /* Check Name Length */
  Uint8T nameLength = strlen(pIf->name);

  /* Interface Name Length*/
  nnBufferSetInt8T(&msgBuff, nameLength);

  /* Interface Name */
  nnBufferSetString(&msgBuff, pIf->name, nameLength);

  /* Interface Index */
  nnBufferSetInt32T (&msgBuff, pIf->ifIndex);

  /* Interface Status */
  nnBufferSetInt8T (&msgBuff, pIf->status);

  /* Interface Flags */
  nnBufferSetInt64T (&msgBuff, pIf->flags);

  /* Interface Metric */
  nnBufferSetInt32T (&msgBuff, pIf->metric);

  /* Interface Mtu */
  nnBufferSetInt32T (&msgBuff, pIf->mtu);

  /* Interface Mtu6 */
  nnBufferSetInt32T (&msgBuff, pIf->mtu6);

  /* Interface Bandwidth */
  nnBufferSetInt32T (&msgBuff, pIf->bandwidth);

  /* Hardware Type. */
  nnBufferSetInt16T (&msgBuff, pIf->hwType);

  /* Hardware Address Length. */
  nnBufferSetInt32T (&msgBuff, pIf->hwAddrLen);

  if (pIf->hwAddrLen > 0)
  {
    /* Hardware Address. */
    nnBufferSetString (&msgBuff, (char *)pIf->hwAddr, pIf->hwAddrLen);
  }

  /*
   * Send Event or IPC Message
   */
  if (ipcType == MESSAGE_EVENT)
  {
    eventPublish (cmd, msgBuff.data, msgBuff.length);
  }
  else if (ipcType == MESSAGE_IPC)
  {
    ipcSendAsync (componentId, cmd, msgBuff.data, msgBuff.length);
  }
  return 0;
}


/**************************************************************************
 * Description : 인터페이스 UP/DOWN 시점에서 각 프로토콜로 인터페이스
 * 정보를 전달하기 위하여 이벤트 메시지를 전송하는 함수임.
 *
 * @param [in] cmd : 명령(EVENT_INTERFACE_UP or EVENT_INTERFACE_DOWN)
 * @param [in] ifp : 인터페이스 자료구조 포인터
 * @retval : 0 if 성공,
 *           0 < if 실패
 *
 * Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | ~ Name Length ~  |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | ~ Name String ~                      | 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      Interface Index                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    Status     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                        Flags(Uint64T)                         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                            Metric                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                             MTU                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                             MTU6                              |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                          Bandwidth                            |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |           HW Type                 |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                        HW Addr Length                         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                          HW Address                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *************************************************************************/
Int32T
InterfaceStatusUpdate(Uint32T ipcType, Int8T componentId, 
                      Uint32T cmd, InterfaceT *pIf)
{
  /* check pointer is not null*/
  assert(pIf);

  printf ("======> event : cmd[%d] %s %d\n", cmd, __func__, __LINE__);

  if (IS_RIBMGR_DEBUG_EVENT)
  {
    NNLOG(LOG_DEBUG, "%s called \n",__func__);

    char addrBuff[1024] = {};
    Int32T idx = 0;
    int i = 0;

    for (i=0;i<pIf->hwAddrLen;i++)
    {
      idx += sprintf(addrBuff +idx, "%02x:", pIf->hwAddr[i]);
    }

    NNLOG(LOG_DEBUG, "hwType[%d] hwAddrLen[%d] hwAddr[%s]\n",
          pIf->hwType, pIf->hwAddrLen, addrBuff);
  }

  nnBufferT msgBuff;

  /* Buffer Reset */
  nnBufferReset(&msgBuff);

  /*
   * Add each value to buffer
   */

  /* Check Name Length */
  Uint8T nameLength = strlen(pIf->name);

  /* Interface Name Length*/
  nnBufferSetInt8T(&msgBuff, nameLength);

  /* Interface Name */
  nnBufferSetString(&msgBuff, pIf->name, nameLength);

  /* Interface Index */
  nnBufferSetInt32T (&msgBuff, pIf->ifIndex);

  /* Interface Status */
  nnBufferSetInt8T (&msgBuff, pIf->status);

  /* Interface Flags */
  nnBufferSetInt64T (&msgBuff, pIf->flags);

  /* Interface Metric */
  nnBufferSetInt32T (&msgBuff, pIf->metric);

  /* Interface Mtu */
  nnBufferSetInt32T (&msgBuff, pIf->mtu);

  /* Interface Mtu6 */
  nnBufferSetInt32T (&msgBuff, pIf->mtu6);

  /* Interface Bandwidth */
  nnBufferSetInt32T (&msgBuff, pIf->bandwidth);

  // add this fields because isis
  /* Hardware Type. */
  nnBufferSetInt16T (&msgBuff, pIf->hwType);

  /* Hardware Address Length. */
  nnBufferSetInt32T (&msgBuff, pIf->hwAddrLen);

  if (pIf->hwAddrLen > 0)
  {
    /* Hardware Address. */
    nnBufferSetString (&msgBuff, (char *)pIf->hwAddr, pIf->hwAddrLen);
  }

  /*
   * Send Event Message
   */
  if (ipcType == MESSAGE_EVENT)
  {
    eventPublish(cmd, msgBuff.data, msgBuff.length);
  }
  else if (ipcType == MESSAGE_IPC)
  {
    ipcSendAsync (componentId, cmd, msgBuff.data, msgBuff.length);
  }
  return 0;
}


/**************************************************************************
 * Description : 인터페이스 주소 추가/삭제 시점에서 각프로토콜로 인터페이스
 * 정보를 전달하기 위하여 이벤트 메시지를 전송하는 함수임.
 *
 * @param [in] cmd : 명령
 *                   EVENT_INTERFACE_ADDRESS_ADD or
 *                   EVENT_INTERFACE_ADDRESS_DELETE
 * @param [in] pIf : 인터페이스 자료구조 포인터
 *
 * @retval : 0 if 성공,
 *           0 < if 실패
 *
 * Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      Interface Index                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    Flags      |     Family    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                    Prefix Address (v4, v6)                    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   PrefixLen   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                     Destination (V4, V6)                      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *************************************************************************/
Int32T
InterfaceAddressUpdate(Uint32T ipcType, Int8T componentId, 
                       Uint32T cmd, InterfaceT *pIf, ConnectedT *pIfc)
{
  /* check pointer is not null*/
  assert(pIf && pIfc);

  printf ("======> event : cmd[%d] %s %d\n", cmd, __func__, __LINE__);

  if (IS_RIBMGR_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "  %s called \n",__func__);

  nnBufferT msgBuff;

  /* Buffer Reset */
  nnBufferReset(&msgBuff);

  /*
   * Add each value to buffer
   */

  /* Interface Index */
  nnBufferSetInt32T (&msgBuff, pIf->ifIndex);

  /* Connected Flags */
  nnBufferSetInt8T (&msgBuff, pIfc->flags);

  /* Prefix Information. */
  nnBufferSetInt8T (&msgBuff, pIfc->pAddress->family);
  if (pIfc->pAddress->family == AF_INET)
  {
    /* Prefix Address */
    nnBufferSetInaddr (&msgBuff, pIfc->pAddress->u.prefix4);

    /* Prefix Length */
    nnBufferSetInt8T (&msgBuff, pIfc->pAddress->prefixLen);

    /* Destination */
    if (pIfc->pDestination)
    {
      nnBufferSetInaddr (&msgBuff, pIfc->pDestination->u.prefix4);
    }
    else
    {
      struct in_addr zeroAddr;
      memset(&zeroAddr, 0, sizeof(struct in_addr));
      nnBufferSetInaddr (&msgBuff, zeroAddr);
    }
  }
#ifdef HAVE_IPV6
  else if (pIfc->pAddress->family == AF_INET6) 
  {
    /* Prefix Address */
    nnBufferSetInaddr6 (&msgBuff, pIfc->pAddress->u.prefix6);

    /* Prefix Length */
    nnBufferSetInt8T (&msgBuff, pIfc->pAddress->prefixLen);

    /* Destination */
    if (pIfc->pDestination)
    {
      nnBufferSetInaddr6 (&msgBuff, pIfc->pDestination->u.prefix6);
    }
    else
    {
      struct in6_addr zeroAddr6;
      memset(&zeroAddr6, 0, sizeof(struct in6_addr));
      nnBufferSetInaddr6 (&msgBuff, zeroAddr6);
    }
  }
#endif
  else 
  {
    NNLOG(LOG_ERR, "Wrong family type = %d\n", pIfc->pAddress->family);
  }

  /*
   * Send Event Message
   */
  if (ipcType == MESSAGE_EVENT)
  {
    eventPublish(cmd, msgBuff.data, msgBuff.length);
  }
  else if (ipcType == MESSAGE_IPC)
  {
    ipcSendAsync (componentId, cmd, msgBuff.data, msgBuff.length);
  }
  return 0;
}

