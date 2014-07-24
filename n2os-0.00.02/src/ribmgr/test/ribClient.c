/**************************************************************************************************** 
*                      Electronics and Telecommunications Research Institute
* Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.
*           No part of this software shall be reproduced, stored in a retrieval system, or
*           transmitted by any means, electronic, mechanical, photocopying, recording,
*           or otherwise, without written permission from ETRI.
****************************************************************************************************/

/**
 * @brief       : ribmgr에서 사용하는 IPC 및 Event 메시지의 처리기능을
 * 확인하기 위한 테스트 코드임.
 * - Block Name : ripd
 * - Process Name : ripd
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : ribClient.c
 *
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include "nnTypes.h"
#include "nnDefines.h"
#include "nnRibDefines.h"
#include "ribmgrRib.h"
#include "nnBuffer.h"
#include "nnPrefix.h"
#include "ribClient.h"
#include "nnLog.h"


/* Print buffer to hexf */
void
print_hexf(char * p, Int32T len) 
{
    char * cp = p;
    Int32T    hcnt = 0;

    while (len > 0) {
        if (hcnt == 0)
            printf("x%2.2lx: ", (long unsigned int)(cp-p));
        if ((hcnt%4) == 0)
            printf(" ");
        printf("%2.2x", (0xff&*cp++)) ;
        len--;
        hcnt++;
        if (hcnt >= 16 || len == 0) {
            printf("\n");
            hcnt = 0;
        }   
    }   
}


/*
 * Description : 루트 설정 메시지 버퍼를 반드는 함수.
 *
 * param [in] apiBuff : 메시지 버퍼
 * param [in] rCount : 루트 개수
 * param [in] rType : 루트 타입
 * param [in] rApi : 각 입력변수를 담는 데이터 스트럭처
 *
 * @retval : 메시지 버퍼 길이
 */
Int32T
buildRouteAddIpv4 (nnBufferT * apiBuff, 
                   Uint16T rCount, Uint8T rType, routeApi4T rApi)
{
  Uint16T position = 0;

  /* Number of Route */
  nnBufferSetInt16T(apiBuff, rCount);

  /* route type */
  nnBufferSetInt8T(apiBuff, rType);
  position = nnBufferGetIndex(apiBuff);

  /* route entry length */
  nnBufferSetInt16T(apiBuff, 0);

  /* route flag 
   *  RIB_FLAG_NULL0 or ... 
   */
  Uint8T dummyFlags = 0;
  nnBufferSetInt8T(apiBuff, rApi.flags);

  /* 
   * msgFlags, nexthop, ifindex, distance, metrix
   */

  /*     #define RIB_MESSAGE_NEXTHOP                0x01
   *     #define RIB_MESSAGE_IFINDEX                0x02
   *     #define RIB_MESSAGE_DISTANCE               0x04
   *     #define RIB_MESSAGE_METRIC                 0x08
   */
  nnBufferSetInt8T(apiBuff, rApi.message);

  /* prefix length */
  nnBufferSetInt8T(apiBuff, rApi.prefix.prefixLen);

  /* prefix  */
  nnBufferSetInaddr(apiBuff, rApi.prefix.prefix);

  /*
   * type, nexthop type
   *   enum nexthop_types_t
   *   {
   *     NEXTHOP_TYPE_IFINDEX = 1,
   *     NEXTHOP_TYPE_IFNAME,
   *     NEXTHOP_TYPE_IPV4,
   *     NEXTHOP_TYPE_IPV4_IFINDEX,
   *     NEXTHOP_TYPE_IPV4_IFNAME,
   *     NEXTHOP_TYPE_IPV6,
   *     NEXTHOP_TYPE_IPV6_IFINDEX,
   *     NEXTHOP_TYPE_IPV6_IFNAME,
   *     NEXTHOP_TYPE_NULL0,
   *   };
  */
  Uint8T nhType = NEXTHOP_TYPE_IPV4;
  nnBufferSetInt8T(apiBuff, nhType);

  /* Nexthop Number */
  // to be apply

  /* Nexthop Address */
  if(rApi.message & RIB_MESSAGE_NEXTHOP)
    nnBufferSetInaddr(apiBuff, rApi.nextHop);

  /* Distance */
  if(rApi.message & RIB_MESSAGE_DISTANCE)
    nnBufferSetInt8T(apiBuff, rApi.distance);

  /* Metric */
  if(rApi.message & RIB_MESSAGE_METRIC)
    nnBufferSetInt32T(apiBuff, rApi.metric);

  /* Message Length */
  Uint16T subLength = nnBufferGetLength(apiBuff) - position - sizeof(Uint16T);
  nnBufferInsertInt16T (apiBuff, position, subLength);

  nnBufferPrint(apiBuff);

  return nnBufferGetLength(apiBuff);
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
Int32T
buildRouteDeleteIpv4 (nnBufferT * apiBuff, 
                      Uint16T rCount, Uint8T rType, routeApi4T rApi)
{
  Uint16T position = 0;

  /* Number of Route */
  nnBufferSetInt16T(apiBuff, rCount);

  /* route type */
  nnBufferSetInt8T(apiBuff, rType);
  position = nnBufferGetIndex(apiBuff);

  /* route entry length */
  nnBufferSetInt16T(apiBuff, 0);

  /* route flag */
  nnBufferSetInt8T(apiBuff, rApi.flags);

  /* msgFlags, nexthop, ifindex, distance, metrix */
  nnBufferSetInt8T(apiBuff, rApi.message);

  /* prefix length */
  nnBufferSetInt8T(apiBuff, rApi.prefix.prefixLen);

  /* prefix */
  nnBufferSetInaddr(apiBuff, rApi.prefix.prefix);

  /* type, nexthop type */
  Uint8T nhType = NEXTHOP_TYPE_IPV4;
  nnBufferSetInt8T(apiBuff, nhType);

  /* Nexthop Number */
  // to be apply

  /* Nexthop Address */
  if(rApi.message & RIB_MESSAGE_NEXTHOP)
    nnBufferSetInaddr(apiBuff, rApi.nextHop);

  /* Metric */
  if(rApi.message & RIB_MESSAGE_METRIC)
    nnBufferSetInt8T(apiBuff, rApi.metric);

  /* Message Length */
  Uint16T subLength = nnBufferGetLength(apiBuff) - position - sizeof(Uint16T);
  nnBufferInsertInt16T (apiBuff, position, subLength);

  return nnBufferGetLength(apiBuff);
}


/*
 * Description : 루트 검색 메시지 버퍼를 반드는 함수.
 *
 * param [in] apiBuff : 메시지 버퍼
 * param [in] rType : 루트 타입
 * param [in] nextHop : 검색하고자 하는 루트 주소
 *
 * @retval : 메시지 버퍼 길이
 */
Int32T
buildRouteLookupIpv4(nnBufferT * apiBuff, Uint8T rType, struct in_addr nextHop)
{
  /* Add Process Number to Buffer */
  nnBufferSetInt8T (apiBuff, rType);

  /* Add Ipv4 Address to Buffer */
  nnBufferSetInaddr (apiBuff, nextHop);

  /* return Buffer Size */
  return nnBufferGetIndex (apiBuff);
}


/*
 * Description : 루트 설정기능을 수행하는 함수.
 *
 * param [in] rType : 루트 타입
 * param [in] rApi : 각 입력변수를 담는 데이터 스트럭처
 *
 * Request Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |        Number of Route        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Route Type   |
 * +-+-+-+-+-+-+-+-+
 * ~~
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |      Length of Sub-Message    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     Flags     |
 * +-+-+-+-+-+-+-+-+
 * |    Message    |
 * +-+-+-+-+-+-+-+-+
 * | Prefix Length |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                   Prefix Address IPV4                         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | Nexthop Type  | <= NEXTHOP_TYPE_IPV4
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                    Gateway IPv4 Address                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    Distance   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                            Metric                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * ~~
 *
 */
void
routeAddIpv4 (Uint8T rType, routeApi4T rApi)
{
  Uint16T rCount = 1;
  nnBufferT msgBuff;

  /* Buffer Reset */
  nnBufferReset(&msgBuff);
  
  /* Build Buffer */
  buildRouteAddIpv4 (&msgBuff, rCount, rType, rApi);
    
  /* Send Message to RIB Manager */
  ipcSendAsync(RIB_MANAGER, IPC_RIB_IPV4_ROUTE_ADD, msgBuff.data, msgBuff.length);
}


/*
 * Description : 루트 삭제 기능을 수행하는 함수.
 *
 * param [in] rType : 루트 타입
 * param [in] rApi : 각 입력변수를 담는 데이터 스트럭처
 *
 * Request Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |        Number of Route        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Route Type   |
 * +-+-+-+-+-+-+-+-+
 * ~~
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |      Length of Sub-Message    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     Flags     |
 * +-+-+-+-+-+-+-+-+
 * |    Message    |
 * +-+-+-+-+-+-+-+-+
 * | Prefix Length |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                   Prefix Address IPV4                         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | Nexthop Type  | <= NEXTHOP_TYPE_IPV4
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                       Gateway Address                         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                            Metric                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * ~~
 *
 */
void
routeDeleteIpv4 (Uint8T rType, routeApi4T rApi)
{
  nnBufferT msgBuff;
  Uint16T rCount = 1;

  /* Buffer Reset */
  nnBufferReset(&msgBuff);
  
  /* Build Buffer */
  buildRouteDeleteIpv4 (&msgBuff, rCount, rType, rApi);
    
  /* Send Message to RIB Manager */
  ipcSendAsync(RIB_MANAGER, IPC_RIB_IPV4_ROUTE_DELETE, 
               msgBuff.data, msgBuff.length);
}

/*
 * Description : 루트 검색 기능을 수행하는 함수.
 *
 * param [in] rType : 루트 타입
 * param [in] nextHop : 검색하고자 하는 루트 주소
 *
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Process Num  |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         IPv4 Address                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *
 * Response Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         IPv4 Address                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         Metric                                |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |Num of Nexthop |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |Type of Nexthop|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  < if type == NEXTHOP_TYPE_IPV4 >
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                    Gateway IPv4 Address                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  < if type == NEXTHOP_TYPE_IFINDEX > or
 *  < if type == NEXTHOP_TYPE_IFNAME >
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                     Interface Index                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */
void
routeLookupIpv4 (Uint8T rType, struct in_addr nextHop)
{
  nnBufferT sndBuff;
  nnBufferT rcvBuff;
 
  /* Buffer Reset */
  nnBufferReset(&sndBuff);
  nnBufferReset(&rcvBuff);

  /* Build Buffer */
  buildRouteLookupIpv4 (&sndBuff, rType, nextHop);

  /* Send & Receive Message with RIB Manager */
  printf("send before, len=%d\n", sndBuff.index);
  ipcSendSync(RIB_MANAGER, 
              IPC_RIB_IPV4_NEXTHOP_BEST_LOOKUP, IPC_RIB_IPV4_NEXTHOP_BEST_LOOKUP,
              sndBuff.data, sndBuff.index, rcvBuff.data, &rcvBuff.length);
  printf("send after\n");

  print_hexf(rcvBuff.data, rcvBuff.length);


  /* Parsing Packet */
  struct in_addr addr4;
  struct in_addr gate4;
  Uint32T metric;
  Uint32T ifIndex;
  Uint8T type;
  Uint8T num;

  addr4 = nnBufferGetInaddr(&rcvBuff); /* requesting address */
  metric = nnBufferGetInt32T(&rcvBuff); /* metric */
  num = nnBufferGetInt8T(&rcvBuff); /* number of nexthop */
  type = nnBufferGetInt8T(&rcvBuff); /* type */
  switch (type)
  {
    case NEXTHOP_TYPE_IPV4 :
      gate4 = nnBufferGetInaddr(&rcvBuff);
      break;

    case NEXTHOP_TYPE_IFINDEX :
    case NEXTHOP_TYPE_IFNAME :
      ifIndex = nnBufferGetInt32T(&rcvBuff);
      break;
  }
  
  printf("###########################################\n");
  printf("route lookup ipv4 \n");
  printf("###########################################\n");
  printf("\t address ipv4 = %s\n", inet_ntoa(addr4));
  printf("\t metric = %d\n", metric);
  printf("\t nexthop number = %d\n", num);
  switch (type)
  {
    case NEXTHOP_TYPE_IPV4 : 
      printf("\t gateway ipv4 = %s\n", inet_ntoa(gate4));
      break;
    case NEXTHOP_TYPE_IFINDEX :
    case NEXTHOP_TYPE_IFNAME :
      printf("\t interface index = %d\n", ifIndex);
      break;
  }
  printf("###########################################\n");
  
  /* Notify Complete Aync IPC to IPC Manager */
  ipcProcessingPendingMsg();  
}

/*
 * Description : IPC 메시지가 수신되었을때 호출되는 콜백 함수.
 *
 * param [in] msgId : 메시지 ID
 * param [in] data : 메시지 버퍼
 * param [in] dataLen : 메시지 길이
 */
Int32T gCount=0;
void
ipcReadCallback (Int32T msgId, void * data, Uint32T dataLen)
{


  printf("###################################\n");
  printf("## %s called..\n", __func__);
  printf("###################################\n");
  NNLOG(LOG_DEBUG, 0, "receive data : msgId = %d len=%d\n", msgId, dataLen);
  printf("\tcount = %d\n", gCount++);

  return;
}


/*
 * Event Process Parts
 */


/*
 * Description : RouterID 이벤트 메시지 수신시 호출되는 함수.
 *
 * param [in] msgBuff : 메시지 버퍼
 *
 * Event Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         RouterID                              |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */
void
recvRouterId(nnBufferT * msgBuff)
{
  PrefixT p;

  p.family = nnBufferGetInt8T(msgBuff);
  p.prefixLen = nnBufferGetInt8T(msgBuff);

  if (p.family == AF_INET)
  {
    p.u.prefix4 = nnBufferGetInaddr(msgBuff);
  }
#ifdef HAVE_IPV6
  else if (p.family == AF_INET6)
  {
    p.u.prefix6 = nnBufferGetInaddr(msgBuff);
  }
#endif

  printf("############################\n");
  printf(" EVENT ROUTER ID\n");
  printf("############################\n");
  printf("\t family=%d, prefixlen=%d, prefix=%s\n",
          p.family, p.prefixLen, inet_ntoa(p.u.prefix4));
  printf("############################\n");
}


/*
 * Description : 인터페이스가 추가되었을때 발생하는 이벤트 수신시 호출되는 함수.
 *
 * param [in] msgBuff : 메시지 버퍼
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
 *
 */
void
recvInterfaceAddUpdate(nnBufferT * msgBuff)
{
  char ifName[1024] = {};

  /* Name Length */
  Uint8T nameLength = nnBufferGetInt8T(msgBuff);

  /* Name String */
  nnBufferGetString(msgBuff, ifName, nameLength);

  /* Interface Index */
  Uint32T ifIndex = nnBufferGetInt32T(msgBuff);
  
  /* Status */
  Uint8T status = nnBufferGetInt8T(msgBuff);
  
  /* Flags */
  Uint64T flags = nnBufferGetInt64T(msgBuff);
  
  /* Metric */
  Int32T metric = nnBufferGetInt32T(msgBuff);

  /* MTU */
  Uint32T mtu = nnBufferGetInt32T(msgBuff);
  
  /* MTU6 */
  Uint32T mtu6 = nnBufferGetInt32T(msgBuff);
  
  /* Bandwidth */
  Uint32T bandwidth = nnBufferGetInt32T(msgBuff);
  
  printf("############################\n");
  printf(" EVENT INTERFACE ADD\n");
  printf("############################\n");
  printf("\t ifName=%s, ifIndex=%d, status=%d, flags=%d\n",
          ifName, ifIndex, status, (int)flags);
  printf("\t metric=%d, mtu=%d, mtu6=%d, bandwidth=%d\n",
          metric, mtu, mtu6, bandwidth);
  printf("############################\n");
}


/*
 * Description : 인터페이스가 삭제되었을때 발생하는 이벤트 수신시 호출되는 함수.
 *
 * param [in] msgBuff : 메시지 버퍼
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
 *
 */
void
recvInterfaceDeleteUpdate(nnBufferT * msgBuff)
{
  char ifName[1024] = {};

  /* Name Length */
  Uint8T nameLength = nnBufferGetInt8T(msgBuff);

  /* Name String */
  nnBufferGetString(msgBuff, ifName, nameLength);

  /* Interface Index */
  Uint32T ifIndex = nnBufferGetInt32T(msgBuff);
  
  /* Status */
  Uint8T status = nnBufferGetInt8T(msgBuff);
  
  /* Flags */
  Uint64T flags = nnBufferGetInt64T(msgBuff);
  
  /* Metric */
  Int32T metric = nnBufferGetInt32T(msgBuff);

  /* MTU */
  Uint32T mtu = nnBufferGetInt32T(msgBuff);
  
  /* MTU6 */
  Uint32T mtu6 = nnBufferGetInt32T(msgBuff);
  
  /* Bandwidth */
  Uint32T bandwidth = nnBufferGetInt32T(msgBuff);
  
  printf("############################\n");
  printf(" EVENT INTERFACE DELETE\n");
  printf("############################\n");
  printf("\t ifName=%s, ifIndex=%d, status=%d, flags=%d\n",
          ifName, ifIndex, status, (int)flags);
  printf("\t metric=%d, mtu=%d, mtu6=%d, bandwidth=%d\n",
          metric, mtu, mtu6, bandwidth);
  printf("############################\n");
}


/*
 * Description : 인터페이스 상태의 UP 이벤트를 수신했을때 호출되는 함수.
 *
 * param [in] msgBuff : 메시지 버퍼
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
 *
 */
void
recvInterfaceUpUpdate(nnBufferT * msgBuff)
{
  char ifName[1024] = {};

  /* Name Length */
  Uint8T nameLength = nnBufferGetInt8T(msgBuff);

  /* Name String */
  nnBufferGetString(msgBuff, ifName, nameLength);

  /* Interface Index */
  Uint32T ifIndex = nnBufferGetInt32T(msgBuff);
  
  /* Status */
  Uint8T status = nnBufferGetInt8T(msgBuff);
  
  /* Flags */
  Uint64T flags = nnBufferGetInt64T(msgBuff);
  
  /* Metric */
  Int32T metric = nnBufferGetInt32T(msgBuff);

  /* MTU */
  Uint32T mtu = nnBufferGetInt32T(msgBuff);
  
  /* MTU6 */
  Uint32T mtu6 = nnBufferGetInt32T(msgBuff);
  
  /* Bandwidth */
  Uint32T bandwidth = nnBufferGetInt32T(msgBuff);
  
  printf("############################\n");
  printf(" EVENT INTERFACE UP\n");
  printf("############################\n");
  printf("\t ifName=%s, ifIndex=%d, status=%d, flags=%d\n",
          ifName, ifIndex, status, (int)flags);
  printf("\t metric=%d, mtu=%d, mtu6=%d, bandwidth=%d\n",
          metric, mtu, mtu6, bandwidth);
  printf("############################\n");
}


/*
 * Description : 인터페이스 상태의 DOWN 이벤트를 수신했을때 호출되는 함수.
 *
 * param [in] msgBuff : 메시지 버퍼
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
 *
 */
void
recvInterfaceDownUpdate(nnBufferT * msgBuff)
{
  char ifName[1024] = {};

  /* Name Length */
  Uint8T nameLength = nnBufferGetInt8T(msgBuff);

  /* Name String */
  nnBufferGetString(msgBuff, ifName, nameLength);

  /* Interface Index */
  Uint32T ifIndex = nnBufferGetInt32T(msgBuff);
  
  /* Status */
  Uint8T status = nnBufferGetInt8T(msgBuff);
  
  /* Flags */
  Uint64T flags = nnBufferGetInt64T(msgBuff);
  
  /* Metric */
  Int32T metric = nnBufferGetInt32T(msgBuff);

  /* MTU */
  Uint32T mtu = nnBufferGetInt32T(msgBuff);
  
  /* MTU6 */
  Uint32T mtu6 = nnBufferGetInt32T(msgBuff);
  
  /* Bandwidth */
  Uint32T bandwidth = nnBufferGetInt32T(msgBuff);
  
  printf("############################\n");
  printf(" EVENT INTERFACE DOWN\n");
  printf("############################\n");
  printf("\t ifName=%s, ifIndex=%d, status=%d, flags=%d\n",
          ifName, ifIndex, status, (int)flags);
  printf("\t metric=%d, mtu=%d, mtu6=%d, bandwidth=%d\n",
          metric, mtu, mtu6, bandwidth);
  printf("############################\n");
}



/*
 * Description : 인터페이스 주소 추가 이벤트를 수신했을때 호출되는 함수.
 *
 * param [in] msgBuff : 메시지 버퍼
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
 */
void
recvInterfaceAddressAddUpdate(nnBufferT * msgBuff)
{
  PrefixT prefix;
  PrefixT destination;

  /* Interface Index */
  Uint32T ifIndex = nnBufferGetInt32T(msgBuff);

  /* Flags */
  Uint8T flags = nnBufferGetInt8T(msgBuff);
  
  /* family */
  prefix.family = nnBufferGetInt8T(msgBuff);

  /* Interface Address */
  if (prefix.family == AF_INET)
  {
    prefix.u.prefix4 = nnBufferGetInaddr(msgBuff);
    prefix.prefixLen = nnBufferGetInt8T(msgBuff);
    /*Destination*/
    destination.u.prefix4 = nnBufferGetInaddr(msgBuff);
  }
#ifdef HAVE_IPV6
  else if (prefix.family == AF_INET6)
  {
    prefix.u.prefix6 = nnBufferGetIn6addr(msgBuff);
    prefix.prefixLen = nnBufferGetInt8T(msgBuff);
    /*Destination*/
    destination.u.prefix4 = nnBufferGetIn6addr(msgBuff);
  }
#endif
  
  printf("############################\n");
  printf(" EVENT INTERFACE ADDRESS ADD\n");
  printf("############################\n");
  printf("\t ifIndex=%d, flags=%d\n", ifIndex, flags);
  printf("\t address/len = %s/%d, destination\n", 
             inet_ntoa(prefix.u.prefix4), prefix.prefixLen, 
             inet_ntoa(destination.u.prefix4));
  printf("############################\n");
  
}


/*
 * Description : 인터페이스 주소 삭제 이벤트를 수신했을때 호출되는 함수.
 *
 * param [in] msgBuff : 메시지 버퍼
 *
 * Event Message format
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
 */
void
recvInterfaceAddressDeleteUpdate(nnBufferT * msgBuff)
{
  PrefixT prefix;
  PrefixT destination;

  /* Interface Index */
  Uint32T ifIndex = nnBufferGetInt32T(msgBuff);

  /* Flags */
  Uint8T flags = nnBufferGetInt8T(msgBuff);
  
  /* family */
  prefix.family = nnBufferGetInt8T(msgBuff);

  /* Interface Address */
  if (prefix.family == AF_INET)
  {
    prefix.u.prefix4 = nnBufferGetInaddr(msgBuff);
    prefix.prefixLen = nnBufferGetInt8T(msgBuff);
    /*Destination*/
    destination.u.prefix4 = nnBufferGetInaddr(msgBuff);
  }
#ifdef HAVE_IPV6
  else if (prefix.family == AF_INET6)
  {
    prefix.u.prefix6 = nnBufferGetIn6addr(msgBuff);
    prefix.prefixLen = nnBufferGetInt8T(msgBuff);
    /*Destination*/
    destination.u.prefix4 = nnBufferGetIn6addr(msgBuff);
  }
#endif
 
  printf("############################\n");
  printf(" EVENT INTERFACE ADDRESS DELETE\n");
  printf("############################\n");
  printf("\t ifIndex=%d, flags=%d\n", ifIndex, flags);
  printf("\t address/len = %s/%d, destination\n", 
             inet_ntoa(prefix.u.prefix4), prefix.prefixLen, 
             inet_ntoa(destination.u.prefix4));
  printf("############################\n");
}

/*
 * Description : 이벤트 메시지가 수신되었을때 호출되는 콜백 함수.
 *
 * param [in] msgId : 메시지 ID
 * param [in] data : 메시지 버퍼
 * param [in] dataLen : 메시지 길이
 */
void
eventReadCallback (Int32T msgId, void * data, Uint32T dataLen)
{
  printf("msgId = %d, dataLen=%d\n", msgId, dataLen);
  /* Buffer Reset & Assign */
  nnBufferT msgBuff;
  nnBufferReset (&msgBuff);
  nnBufferAssign (&msgBuff, data, dataLen);

  /* Switch to each function for event id */
  switch(msgId)
  {
    case EVENT_ROUTER_ID :
      printf("NNNN EVENT_ROUTER_ID\n");
      recvRouterId(&msgBuff);
      break;

    case EVENT_INTERFACE_ADD :
      printf("NNNN EVENT_INTERFACE_ADD\n");
      recvInterfaceAddUpdate(&msgBuff);
      break;

    case EVENT_INTERFACE_DELETE :
      printf("NNNN EVENT_INTERFACE_DELETE\n");
      recvInterfaceDeleteUpdate(&msgBuff);
      break;

    case EVENT_INTERFACE_UP :
      printf("NNNN EVENT_INTERFACE_UP\n");
      recvInterfaceUpUpdate(&msgBuff);
      break;

    case EVENT_INTERFACE_DOWN :
      printf("NNNN EVENT_INTERFACE_DOWN\n");
      recvInterfaceDownUpdate(&msgBuff);
      break;

    case EVENT_INTERFACE_ADDRESS_ADD :
      printf("NNNN EVENT_INTERFACE_ADDRESS_ADD\n");
      recvInterfaceAddressAddUpdate(&msgBuff);
      break;

    case EVENT_INTERFACE_ADDRESS_DELETE :
      printf("NNNN EVENT_INTERFACE_ADDRESS_DELETE\n");
      recvInterfaceAddressDeleteUpdate(&msgBuff);
      break;

    default :
      break;
  }
}
