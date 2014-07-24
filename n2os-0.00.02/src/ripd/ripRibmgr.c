/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : RIP Protocol이 RIB Manager로의 인터페이스 화일
 *
 * - Block Name : RIP Protocol
 * - Process Name : ripd
 * - Creator : Geontae Park
 * - Initial Date : 2014/02/20
 */

/**
 * @file        : ripRibmgr.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include "nnTypes.h"
#include "nnStr.h"
#include "nnPrefix.h"
#include "nnBuffer.h"
#include "nnRibDefines.h"
#include "nosLib.h"
#include "nnUtility.h"
#include "taskManager.h"

#include "nnCmdCommon.h"

#include "ripd.h"
#include "ripDebug.h"
#include "ripInterface.h"


/*
 * Description : 루트 삭제 메시지 버퍼를 반드는 함수.
 *
 * param [in] apiBuff : 메시지 버퍼
 * param [in] rCount : 루트 개수
 * param [in] rType : 루트 타입
 * param [in] rApi : 각 입력변수를 담는 데이터 스트럭처
 *
 * @retval : 메시지 버퍼 길이
 */
void
ripRibmgrInit ()
{
  /* Buffer Reset */
  nnBufferT msgBuff;
  nnBufferReset(&msgBuff);

  /* Component ID. Defined in nnDefines.h. */
  nnBufferSetInt8T(&msgBuff, RIP);

  /* Routing component type. Defined in nnRibDefines.h. */
  nnBufferSetInt8T(&msgBuff, RIB_ROUTE_TYPE_RIP);

  /* Send ipc message to ribmgr */
  ipcSendAsync(RIB_MANAGER,
               IPC_RIB_CLIENT_INIT, msgBuff.data, msgBuff.length);
}

void
ripRibmgrClose ()
{
  nnBufferT sendBuff;
  nnBufferT recvBuff;

  /* Buffer Reset */
  nnBufferReset(&sendBuff);
  nnBufferReset(&recvBuff);

  /* Component ID. Defined in nnDefines.h. */
  nnBufferSetInt8T(&sendBuff, RIP);

  /* Routing component type. Defined in nnRibDefines.h. */
  nnBufferSetInt8T(&sendBuff, RIB_ROUTE_TYPE_RIP);

  NNLOG (LOG_DEBUG, 
         "%s called. send IPC_RIB_CLIENT_CLOSE to ribmgr\n", __func__);
  /* Send ipc message to ribmgr */
  ipcSendSync(RIB_MANAGER,
              IPC_RIB_CLIENT_CLOSE, IPC_RIB_CLIENT_CLOSE, 
              sendBuff.data, sendBuff.length, recvBuff.data, &recvBuff.length);

//  ipcProcPendingMsg ();
}


/*
 * Description : 루트 삭제 메시지 버퍼를 반드는 함수.
 *
 * param [in] apiBuff : 메시지 버퍼
 * param [in] rCount : 루트 개수
 * param [in] rType : 루트 타입
 * param [in] rApi : 각 입력변수를 담는 데이터 스트럭처
 *
 * @retval : 메시지 버퍼 길이
 */

/* RIPd to ribmgr command interface. */
void
ripRibmgrIpv4Add (Prefix4T *p, struct in_addr *pNextHop, 
		          Uint32T metric, Uint8T distance)
{

  if (IS_RIP_DEBUG_RIBMGR)
  {
    NNLOG (LOG_DEBUG, "RIP_RIBMANAGER_IPV4_ADD\n");
    NNLOG (LOG_DEBUG, "\tfamily = %d\n", p->family);
    NNLOG (LOG_DEBUG, "\tlength = %d\n", p->prefixLen);
    NNLOG (LOG_DEBUG, "\tprefix = %s\n", inet_ntoa(p->prefix));
    NNLOG (LOG_DEBUG, "\tnexthop = %s\n", inet_ntoa(*pNextHop));
    NNLOG (LOG_DEBUG, "\tmetric = %d\n", metric);
    NNLOG (LOG_DEBUG, "\tdistance = %d\n\n", distance);
  }

  Uint16T position = 0;
  Uint16T rCount = 1; 
  Uint8T rType = RIB_ROUTE_TYPE_RIP;
 
  /* Buffer Reset */
  nnBufferT msgBuff;
  nnBufferReset(&msgBuff);

  /* Number of Route */
  nnBufferSetInt16T(&msgBuff, rCount);

  /* route type */
  nnBufferSetInt8T(&msgBuff, rType);
  position = nnBufferGetIndex(&msgBuff);

  /* route entry length */
  nnBufferSetInt16T(&msgBuff, 0);

  /* route flag 
   *  RIB_FLAG_NULL0 or ... 
   */
  Uint8T dummyFlags = 0;
  nnBufferSetInt8T(&msgBuff, dummyFlags);

  /* 
   * msgFlags, nexthop, ifindex, distance, metrix
   */

  /*     #define RIB_MESSAGE_NEXTHOP                0x01
   *     #define RIB_MESSAGE_IFINDEX                0x02
   *     #define RIB_MESSAGE_DISTANCE               0x04
   *     #define RIB_MESSAGE_METRIC                 0x08
   */
  Uint8T messages = 0;
  messages |= RIB_MESSAGE_NEXTHOP;
  messages |= RIB_MESSAGE_DISTANCE;
  messages |= RIB_MESSAGE_METRIC;
  nnBufferSetInt8T(&msgBuff, messages);

  /* prefix length */
  nnBufferSetInt8T(&msgBuff, p->prefixLen);

  /* prefix  */
  nnBufferSetInaddr(&msgBuff, p->prefix);

  /* Nexthop type */
  Uint8T nhType = NEXTHOP_TYPE_IPV4;
  nnBufferSetInt8T(&msgBuff, nhType);

  /* Nexthop */
  nnBufferSetInaddr(&msgBuff, *pNextHop);

  /* Distance */
  nnBufferSetInt8T(&msgBuff, distance);

  /* Metric */
  nnBufferSetInt32T(&msgBuff, metric);

  /* Message Length */
  Uint16T subLength = nnBufferGetLength(&msgBuff) - position - sizeof(Uint16T);
  nnBufferInsertInt16T (&msgBuff, position, subLength);

  nnBufferPrint(&msgBuff);

  /* send ipc message to ribmgr */
  ipcSendAsync(RIB_MANAGER, 
               IPC_RIB_IPV4_ROUTE_ADD, msgBuff.data, msgBuff.length);
}

void
ripRibmgrIpv4Delete (Prefix4T *p, struct in_addr *pNextHop, 
		             Uint32T metric)
{
  if (IS_RIP_DEBUG_RIBMGR)
  {
    NNLOG (LOG_DEBUG, "RIP_RIBMANAGER_IPV4_DELETE\n");
    NNLOG (LOG_DEBUG, "\nfamily = %d\n", p->family);
    NNLOG (LOG_DEBUG, "\nlength = %d\n", p->prefixLen);
    NNLOG (LOG_DEBUG, "\nprefix = %s\n", inet_ntoa(p->prefix));
    NNLOG (LOG_DEBUG, "\nnexthop = %s\n", inet_ntoa(*pNextHop));
    NNLOG (LOG_DEBUG, "\nmetric = %d\n\n", metric);
  }

  /* Build Buffer */
 
  Uint16T position = 0;
  Uint16T rCount = 1; 
  Uint8T rType = RIB_ROUTE_TYPE_RIP;
 
  /* Buffer Reset */
  nnBufferT msgBuff;
  nnBufferReset(&msgBuff);

  /* Number of Route */
  nnBufferSetInt16T(&msgBuff, rCount);

  /* route type */
  nnBufferSetInt8T(&msgBuff, rType);
  position = nnBufferGetIndex(&msgBuff);

  /* route entry length */
  nnBufferSetInt16T(&msgBuff, 0);

  /* route flag 
   *  RIB_FLAG_NULL0 or ... 
   */
  Uint8T dummyFlags = 0;
  nnBufferSetInt8T(&msgBuff, dummyFlags);

  /* 
   * msgFlags, nexthop, ifindex, distance, metrix
   */

  /*     #define RIB_MESSAGE_NEXTHOP                0x01
   *     #define RIB_MESSAGE_IFINDEX                0x02
   *     #define RIB_MESSAGE_DISTANCE               0x04
   *     #define RIB_MESSAGE_METRIC                 0x08
   */
  Uint8T messages = 0;
  messages |= RIB_MESSAGE_NEXTHOP;
  messages |= RIB_MESSAGE_DISTANCE;
  messages |= RIB_MESSAGE_METRIC;
  nnBufferSetInt8T(&msgBuff, messages);

  /* prefix length */
  nnBufferSetInt8T(&msgBuff, p->prefixLen);

  /* prefix  */
  nnBufferSetInaddr(&msgBuff, p->prefix);

  /* Nexthop type */
  Uint8T nhType = NEXTHOP_TYPE_IPV4;
  nnBufferSetInt8T(&msgBuff, nhType);

  /* Nexthop */
  nnBufferSetInaddr(&msgBuff, *pNextHop);

  /* Distance */
  // nnBufferSetInt8T(&msgBuff, distance);

  /* Metric */
  nnBufferSetInt32T(&msgBuff, metric);

  /* Message Length */
  Uint16T subLength = nnBufferGetLength(&msgBuff) - position - sizeof(Uint16T);
  nnBufferInsertInt16T (&msgBuff, position, subLength);

  nnBufferPrint(&msgBuff);
 
  /* Send Message to RIB Manager */
  ipcSendAsync(RIB_MANAGER, IPC_RIB_IPV4_ROUTE_DELETE,
                msgBuff.data, msgBuff.length);
}


/* Ribmgr route add and delete treatment. 
 **************************************************************************
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
 *        << below 2 fields is used, when add route case >>
 *
 * +-+-+-+-+-+-+-+-+
 * |    Distance   |  <== in case of 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                            Metric                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *************************************************************************/
Int32T
ripRibmgrReadIpv4 (Int32T command,  nnBufferT *msgBuff, Uint16T length)
{
  Int8T type = 0;
  Int8T flags = 0;
  Int8T messages = 0;
  Prefix4T prefix = {0,};
  Int8T nhCnt = 0;
  struct in_addr nextHop = {0,};
  Int8T ifCnt = 0;
  Uint32T ifIndex = 0;
  Int8T distance = 0;
  Uint32T metric = 0;

  /* Type, flags, message. */
  type = nnBufferGetInt8T (msgBuff);
  flags = nnBufferGetInt8T (msgBuff);
  messages = nnBufferGetInt8T (msgBuff);

  /* IPv4 prefix. */
  memset (&prefix, 0, sizeof (Prefix4T));
  prefix.family = AF_INET;
  prefix.prefixLen = nnBufferGetInt8T (msgBuff);
  prefix.prefix = nnBufferGetInaddr (msgBuff);

  /* Nexthop, ifindex, distance, metric. */
  if (CHECK_FLAG (messages, RIB_MESSAGE_NEXTHOP))
  {
    nhCnt = nnBufferGetInt8T (msgBuff);
    nextHop = nnBufferGetInaddr (msgBuff);
  }
  if (CHECK_FLAG (messages, RIB_MESSAGE_IFINDEX))
  {
    ifCnt = nnBufferGetInt8T (msgBuff);
    ifIndex = nnBufferGetInt32T (msgBuff);
  }
  /* Get distance. */
  if (CHECK_FLAG (messages, RIB_MESSAGE_DISTANCE))
  {
    distance = nnBufferGetInt8T (msgBuff);
  }
  else
  {
    distance = 255;
  }
  /* Get metric. */
  if (CHECK_FLAG (messages, RIB_MESSAGE_METRIC))
  {
    metric = nnBufferGetInt32T (msgBuff);
  }
  else
  {
    metric = 0;
  }

  if (IS_RIP_DEBUG_RIBMGR)
  {
    NNLOG (LOG_DEBUG, "%s\n",__func__);
    if (command == IPC_RIB_IPV4_ROUTE_ADD)
      NNLOG (LOG_DEBUG, "IPC_RIB_IPV4_ROUTE_ADD\n");
    else
      NNLOG (LOG_DEBUG, "IPC_RIB_IPV4_ROUTE_DELETE\n");
    NNLOG (LOG_DEBUG, "\t prefix/length=%s/%d\n", inet_ntoa(prefix.prefix), prefix.prefixLen);
    NNLOG (LOG_DEBUG, "\t nexthop count=%d, address=%s\n", nhCnt, inet_ntoa(nextHop));
    NNLOG (LOG_DEBUG, "\t intf count = %d, intf index = %d\n", ifCnt, ifIndex);
    NNLOG (LOG_DEBUG, "\t distance = %d, metric = %d\n", distance, metric);
    NNLOG (LOG_DEBUG, "\t type = %d, flags = %d, messages = %d\n", type, flags, messages);
  }

  /* Then fetch IPv4 prefixes. */
  if (command == IPC_RIB_IPV4_ROUTE_ADD)
  {
    ripRedistributeAdd (type, RIP_ROUTE_REDISTRIBUTE, &prefix, ifIndex, 
                        &nextHop, metric, distance);
  }
  else 
  {
    ripRedistributeDelete (type, RIP_ROUTE_REDISTRIBUTE, &prefix, ifIndex);
  }

  return 0;
}


/*
 * Description : restribute 를 위하여 ribmgr로 루트를 요청하는 함수. 
 *
 * param [in] routeType : 루트 타입
 *
 * @retval : 메시지 버퍼 길이
 */

/* RIPd to ribmgr command interface. */
static void
ripRibmgrRedistributeSet (Int32T cmd, Int32T routeType)
{
  /* Buffer Reset */
  nnBufferT msgBuff;
  nnBufferReset(&msgBuff);

  /* Add commponent id. */
  nnBufferSetInt8T(&msgBuff, RIP); /* RIP component id */

  /* Add requested route type. */
  nnBufferSetInt8T(&msgBuff, routeType);

  if (IS_RIP_DEBUG_RIBMGR)
  {
    NNLOG (LOG_DEBUG, "%s\n", __func__);

    if (cmd == IPC_RIB_REDISTRIBUTE_ADD)
    {
      NNLOG (LOG_DEBUG, "IPC_RIB_REDISTRIBUTE_ADD\n");
    }
    else
    {
      NNLOG (LOG_DEBUG, "IPC_RIB_REDISTRIBUTE_DELETE\n");
    }

    NNLOG (LOG_DEBUG, "\tcompId = %d, protoId = %d, requestRoute=%d\n", 
           RIP, RIB_ROUTE_TYPE_RIP, routeType);
  }

  /* send ipc message to ribmgr */
  ipcSendAsync(RIB_MANAGER, cmd, msgBuff.data, msgBuff.length);
}



/* RIP route-map set for redistribution */
static void
ripRoutemapSet (Int32T type, const StringT name)
{
  if (pRip->routeMap[type].name)
    NNFREE(MEM_ROUTEMAP_NAME, pRip->routeMap[type].name);

  pRip->routeMap[type].name = nnStrDup (name, MEM_ROUTEMAP_NAME);
  pRip->routeMap[type].map = routeMapLookupByName (name);
}

/* RIP route-map unset for redistribution */
static Int32T
ripRoutemapUnset (Int32T type, const StringT name)
{
  if (! pRip->routeMap[type].name ||
      (name != NULL && strcmp(pRip->routeMap[type].name,name)))
    return 1;

  NNFREE (MEM_ROUTEMAP_NAME, pRip->routeMap[type].name);
  pRip->routeMap[type].name = NULL;
  pRip->routeMap[type].map = NULL;

  return 0;
}


/* Redistribution types */
static struct {
  Int32T type;
  Int32T str_min_len;
  const char *str;
} redist_type[] = {
  {RIB_ROUTE_TYPE_KERNEL,  1, "kernel"},
  {RIB_ROUTE_TYPE_CONNECT, 1, "connected"},
  {RIB_ROUTE_TYPE_STATIC,  1, "static"},
  {RIB_ROUTE_TYPE_OSPF,    1, "ospf"},
  {RIB_ROUTE_TYPE_BGP,     1, "bgp"},
  {0, 0, NULL}
};


void
ripRedistributeMetricSet (char * strRoute, Uint32T metric)
{
  Int32T i = 0;

  for (i = 0; redist_type[i].str; i++)
  {
    if (strncmp (redist_type[i].str, strRoute, 
        redist_type[i].str_min_len) == 0)
    {
      pRip->routeMap[redist_type[i].type].metricConfig = 1;
      pRip->routeMap[redist_type[i].type].metric = metric;
    }
  }
}

Int32T
ripRedistributeMetricUnSet (char * strRoute, Uint32T metric)
{
#define DONT_CARE_METRIC_RIP 17  
  Int32T i = 0;


  for (i = 0; redist_type[i].str; i++)
  {
    if (strncmp (redist_type[i].str, strRoute, 
        redist_type[i].str_min_len) == 0)
    {
      if (metric != DONT_CARE_METRIC_RIP &&
          pRip->routeMap[redist_type[i].type].metric != metric)
      {
        return FAILURE;
      }

      pRip->routeMap[redist_type[i].type].metricConfig = 0;
      pRip->routeMap[redist_type[i].type].metric = 0;
        return SUCCESS;
    }
  }

  return FAILURE;
}


/* Ribmgr redistribute add and delete and clear treatment. */
Int32T ripRibmgrRedistribute (struct cmsh * cmsh, Int32T cmd, char * strRoute)
{
  Int32T i = 0;

  /* Check route string. */
  for(i = 0; redist_type[i].str; i++)
  {
    if (strncmp (redist_type[i].str, strRoute,
                 redist_type[i].str_min_len) == 0)
    {
      /* Request routemap to ribmgr. */
      if (cmd == IPC_RIB_REDISTRIBUTE_ADD)
      {
        if (pRip->ripRedistribute[redist_type[i].type])
        {
          cmdPrint (cmsh, "Already set redistribute = %s\n", strRoute);
          return (CMD_IPC_OK);
        }
        
        /* Set flag of redistribute. */        
        cmdPrint (cmsh, "Set redistribute = %s\n", strRoute);
        pRip->ripRedistribute[redist_type[i].type] = 1;

        /* Send request message to ribmgr. */
        ripRibmgrRedistributeSet (cmd, redist_type[i].type);
      }
      else if (cmd == IPC_RIB_REDISTRIBUTE_DELETE)
      {
        if (!pRip->ripRedistribute[redist_type[i].type])
        {
          cmdPrint (cmsh, "Already no set redistribute = %s\n", strRoute);
          return (CMD_IPC_OK);
        }
        
        /* Set flag of redistribute. */        
        cmdPrint (cmsh, "no Set redistribute = %s\n", strRoute);
        pRip->ripRedistribute[redist_type[i].type] = 0;

        /* Send request message to ribmgr. */
        ripRibmgrRedistributeSet (cmd, redist_type[i].type);

        /* Delete redistributed route of RIP. */
        ripRedistributeWithdraw (redist_type[i].type);
      }
      else if (cmd == IPC_RIB_REDISTRIBUTE_CLEAR)
      {
        cmdPrint (cmsh, "Not implemented yet !!!\n");
      }
      else
      {
        cmdPrint (cmsh, "Wrong command = %d\n", cmd);
        return (CMD_IPC_ERROR);
      }

      return (CMD_IPC_OK);
    }
  }

  cmdPrint (cmsh, "Wrong route string = %s\n", strRoute);
  return (CMD_IPC_ERROR);  
}


static Int32T
ripRedistributeUnset (Int32T type)
{
#if 0
  if (! zclient->redist[type])
    return CMD_SUCCESS;

  zclient->redist[type] = 0;

  if (zclient->sock > 0)
    zebra_redistribute_send (IPC_RIB_REDISTRIBUTE_DELETE, zclient, type);

  /* Remove the routes from RIP table. */
  ripRedistributeWithdraw (type);

  NNLOG(LOG_DEBUG,"gtpark end : %s %d", __FUNCTION__, __LINE__);
  return CMD_SUCCESS;
#endif
  return 0;
}

Int32T
ripRedistributeCheck (Int32T type)
{
#if 0
  return (zclient->redist[type]);
#endif
  return 0;
}

void
ripRedistributeClean (void)
{
  Int32T i;
  for (i = 0; redist_type[i].str; i++)
  {
    if (pRip->ripRedistribute[redist_type[i].type])
    {
      ripRibmgrRedistributeSet (IPC_RIB_REDISTRIBUTE_DELETE, 
                                redist_type[i].type);

      pRip->ripRedistribute[redist_type[i].type] = 0;

      /* Remove the routes from RIP table. */
      ripRedistributeWithdraw (redist_type[i].type);
    }
  }
}



Int32T
configWriteRipRedistribute (struct cmsh *cmsh, Int32T configMode)
{
  Int32T i = 0;

  for (i = 0; i < RIB_ROUTE_TYPE_MAX; i++)
  {
    if (i != pRip->ripRedistributeDefault && pRip->ripRedistribute[i])
    {
      if (configMode)
      {
        if (pRip->routeMap[i].metricConfig)
        {
          if (pRip->routeMap[i].name)
          {
            cmdPrint (cmsh, " redistribute %s metric %d route-map %s",
                      (char *)nnRoute2String(i), pRip->routeMap[i].metric,
                      pRip->routeMap[i].name);
          }
          else
          {
            cmdPrint (cmsh, " redistribute %s metric %d",
                      (char *)nnRoute2String(i), pRip->routeMap[i].metric);
          }
        }
        else
        {
          if (pRip->routeMap[i].name)
          {
            cmdPrint (cmsh, " redistribute %s route-map %s",
                      (char *)nnRoute2String(i), pRip->routeMap[i].name);
          }
          else
          {
            cmdPrint (cmsh, " redistribute %s", (char *)nnRoute2String(i));
          }
        }
      }
      else
      {
        cmdPrint (cmsh, " %s", (char *)nnRoute2String(i));
      }
    }
  }

  return 0;
}

