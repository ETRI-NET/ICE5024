#if !defined(_ribmgrIpc_h)
#define _ribmgrIpc_h

/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the ipc message processing functions.
 *  - Block Name : RIB Manager
 *  - Process Name : ribmgr
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : ribmgrIpc.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include "nnTypes.h"
#include "nnBuffer.h"
#include "nnIf.h"

#include "ribmgrRib.h"

#define MESSAGE_IPC         1
#define MESSAGE_EVENT       2

/* Lcs related ipc message processing functions. */
extern void ipcLcsRibSetRole (void * msg, Uint32T msgLen);
extern void ipcLcsRibTerminate (void * msg, Uint32T msgLen);
extern void ipcLcsRibHealthcheck (void * msg, Uint32T msgLen);
extern void ipcLcsRibEventComponentErrorOccured (void * msg, Uint32T msgLen);
extern void ipcLcsRibEventComponentServiceStatus (void * msg, Uint32T msgLen);


/**************************************************************************
 * Description : 인터페이스 주소 추가/삭제 시점에서 각프로토콜로 인터페이스
 * 정보를 전달하기 위하여 이벤트 메시지를 전송하는 함수임.
 *
 * @param [in] msgBuff : 메시지버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패
 *
 * Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  ComponentID  | <= defined in nnDeines.h
 * +-+-+-+-+-+-+-+-+
 * |   RouteType   | <= defined in nnRibDeines.h
 * +-+-+-+-+-+-+-+-+
 *
 *************************************************************************/
Int32T 
ipcRibClientInit (nnBufferT * msgBuff);

Int32T 
ipcRibClientClose (nnBufferT * msgBuff);


/**
 * Description : 프로토콜이 Graceful Restart 하는 시점에 RIB Manager 로 
 * 요청된 Route 는 설정된 시간 동안 유지 요청을 처리하는 함수.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
 *
 * 0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 * |                      N2OS IPC Header                          | 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 * |  Route Type   | 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 * |                        Time                                   | 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 *
 * @endverbatim
 * 
 */
Int32T 
ipcRibRoutePreserve(nnBufferT * msgBuff);


/**
 * Description : 프로토콜이 RIB Manager 로 오래된 Route 삭제 요청하며,
 * 프로토콜이 요청하는 메시지를 수신 하였을때 처리하는 함수.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
 *
 * 0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Route Type   |
 * +-+-+-+-+-+-+-+-+
 * @endverbatim
 */
Int32T 
ipcRibRouteStaleRemove(nnBufferT * msgBuff);


/**
 * Description : 프로토콜이 RIB Manager 로 모든 Route 삭제 요청하며, 
 * 프로토콜이 요청하는 메시지를 수신 하였을때 처리하는 함수.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Route Type   |
 * +-+-+-+-+-+-+-+-+
 * @endverbatim
 */
Int32T 
ipcRibRouteClear(nnBufferT * msgBuff);


/*
 * Description : 호출되지 않는 함수.
 */
Int32T 
ipcRibRouterIdSet(nnBufferT * msgBuff);

/**
 * Description : 프로토콜이 Ipv4 Route 를 생성하여 RIB Manager 로 RIB 추가를
 * 요청하는 함수.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
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
 * @endverbatim
 */

Int32T 
ipcRibIpv4RouteAdd(nnBufferT * msgBuff);


/**
 * Description : 프로토콜이 Ipv4 Route 를 삭제하기 위하여 RIB Manager 로 RIB 
 * 삭제를 요청함. 이 메시지르 처리하기 위한 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
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
 * @endverbatim
 */
Int32T 
ipcRibIpv4RouteDelete(nnBufferT * msgBuff);


/**
 * Description : 프로토콜 또는 명령어로  Ipv6 Route를 생성하여 RIB Manager로
 * RIB 추가를 요청함. 이 메시지를 처리하기 위한 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
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
 * |                   Prefix Address IPV6                         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | Nexthop Type  | <= NEXTHOP_TYPE_IPV6
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                    Gateway IPv4 Address                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    Distance   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                            Metric                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * ~~
 * @endverbatim
 */

Int32T 
ipcRibIpv6RouteAdd(nnBufferT * msgBuff);


/**
 * Description : 프로토콜 또는 명령어로  Ipv6 Route를 삭제하기 위하여 
 * RIB Manager 로 RIB 삭제를 요청함. 이 메시지를 처리하기 위한 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
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
 * |                   Prefix Address IPV6                         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | Nexthop Type  | <= NEXTHOP_TYPE_IPV6
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                       Gateway Address                         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                            Metric                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * ~~
 * @endverbatim
*/
Int32T 
ipcRibIpv6RouteDelete(nnBufferT * msgBuff);


/**
 * Description : 프로토콜 또는 명령어에 의하여 RIB Manager 로 Ipv4 
 * Best-Mach Route 에 대한 Lookup 요청함. 이 메시지를 처리하기 위한 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * Requested Message format
 * @verbatim
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Route Type  |     
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         IPv4 Address                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
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
 * @endverbatim
 */
Int32T 
ipcRibIpv4NexthopBestLookup(nnBufferT * msgBuff);


/**
 * Description : 프로토콜 또는 명령어에 의하여  RIB Manager 로 Ipv4 
 * Exact-Mach Route 에 대한 Lookup 요청함. 이 메시지를 처리하는 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Route Type  | <= Requesting Protocols
 * +-+-+-+-+-+-+-+-+
 * |  PrefixLength |    
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         IPv4 Address                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Response Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  PrefixLength |    
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
 * @endverbatim
 */
Int32T 
ipcRibIpv4NexthopExactLookup(nnBufferT * msgBuff);


/**
 * Description : 프로토콜 또는 명령어에 의하여  RIB Manager 로 Ipv6 
 * Best-Mach Route 에 대한 Lookup 요청함. 이 메시지를 처리하는 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Route Type  |     
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         IPv6 Address                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Response Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         IPv6 Address                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         Metric                                |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |Num of Nexthop |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |Type of Nexthop|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  < if type == NEXTHOP_TYPE_IPV6 >
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                    Gateway IPv6 Address                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  < if type == NEXTHOP_TYPE_IFINDEX > or
 *  < if type == NEXTHOP_TYPE_IFNAME > 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                     Interface Index                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endverbatim
 */
Int32T 
ipcRibIpv6NexthopBestLookup(nnBufferT * msgBuff);


/**
 * Description : 프로토콜 또는 명령어에 의하여  RIB Manager 로 Ipv6 
 * Exact-Mach Route 에 대한 Lookup 요청함. 이메시지를 처리하는 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Route Type  |
 * +-+-+-+-+-+-+-+-+
 * |  PrefixLength |    
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         IPv6 Address                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Response Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         IPv6 Address                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         Metric                                |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |Num of Nexthop |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |Type of Nexthop|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  < if type == NEXTHOP_TYPE_IPV6 >
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                    Gateway IPv6 Address                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  < if type == NEXTHOP_TYPE_IFINDEX > or
 *  < if type == NEXTHOP_TYPE_IFNAME > 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                     Interface Index                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endverbatim
 */

Int32T 
ipcRibIpv6NexthopExactLookup(nnBufferT * msgBuff);


/**
 * Description : 프로토콜이 RIB Manager 로 관심 있는 Redistribute Route 정보 
 * 추가를 요청함. 이메시지를 처리하는 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Component ID |
 * +-+-+-+-+-+-+-+-+
 * |   Route Type  |    
 * +-+-+-+-+-+-+-+-+
 */
Int32T 
ipcRibRedistributeAdd(nnBufferT * msgBuff);


/**
 * Description : 프로토콜이 RIB Manager 로 관심 있는 Redistribute Route 정보 
 * 삭제를 요청함. 이 메시지를 처리하는 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Component ID |
 * +-+-+-+-+-+-+-+-+
 * |   Route Type  |    
 * +-+-+-+-+-+-+-+-+
 */
Int32T 
ipcRibRedistributeDelete(nnBufferT * msgBuff);


/**
 * Description : 프로토콜이 RIB Manager 로 default route에 대하여 
 * redistribute 요청함. 메시지를 처리하는 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Component ID |
 * +-+-+-+-+-+-+-+-+
 */
Int32T 
ipcRibRedistributeDefaultAdd(nnBufferT * msgBuff);


/**
 * Description : 프로토콜이 RIB Manager 로 요청한 default route에 대한 정보
 * 삭제를 요청함. 이 메시지를 처리하는 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Component ID |
 * +-+-+-+-+-+-+-+-+
 */
Int32T 
ipcRibRedistributeDefaultDelete(nnBufferT * msgBuff);


/**
 * Description : 프로토콜이 RIB Manager 로 사전 설정한 모든 Redistribute 정보
 * 삭제를 요청함. 이 메시지를 처리하는 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @verbatim
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Component ID |
 * +-+-+-+-+-+-+-+-+
 */
Int32T 
ipcRibRedistributeClear(nnBufferT * msgBuff);



/**
 * Description : 프로토콜이 관심 있는 Ipv4 Best-Mach Route 를 RIB Manager
 * 에 등록 요청 메시지를 처리하는 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Route Type  | <= Requesting Protocol     
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         IPv4 Address                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endverbatim
 *
 */
Int32T 
ipcRibIpv4NexthopBestLookupReg(nnBufferT * msgBuff);


/**
 * Description : 프로토콜이 등록한 Ipv4 Best-Mach Route 를 RIB Manager
 * 에서 삭제 요청 메시지를 처리하는 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Route Type  | <= Requesting Protocol
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         IPv4 Address                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endverbatim
 */
Int32T 
ipcRibIpv4NexthopBestLookupDereg(nnBufferT * msgBuff);


/**
 * Description : 프로토콜이 관심 있는 Ipv4 Exact-Mach Route 를 RIB Manager
 * 에 등록 요청 메시지를 처리하는 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Route Type  | <= Requesting Protocol
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | PrefixLength  |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         IPv4 Address                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endverbatim
 */
Int32T 
ipcRibIpv4NexthopExactLookupReg(nnBufferT * msgBuff);


/**
 * Description : 프로토콜이 등록한 Ipv4 Exact-Mach Route 를 RIB Manager
 * 에서 삭제 요청 메시지를 처리하는 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Route Type  | <= Requesting Protocol
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | PrefixLength  |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         IPv4 Address                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endverbatim
 */
Int32T 
ipcRibIpv4NexthopExactLookupDereg(nnBufferT * msgBuff);


/**
 * Description : 프로토콜이 관심 있는 Ipv4 Route 를 RIB Manager에 등록한
 * 경우, 해당 Route 가 추가되었을 때 RIB Manager 가 해당 프로토콜로 
 * Notification 통보 ??? (프로토콜의 IPC 메시지 처리하는 부분..)
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Notification Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | PrefixLength  |              
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
 * @endverbatim
 */
Int32T 
ipcRibIpv4RouteNotificationAdd(nnBufferT * msgBuff);


/**
 * Description : 프로토콜이 관심 있는 Ipv4 Route 를 RIB Manager에 등록한
 * 경우, 해당 Route 가 삭제되었을 때 RIB Manager 가 해당 프로토콜로
 * Notification 통보 ??? (프로토콜의 IPC 메시지 처리하는 부분..)
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Notification Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | PrefixLength  |              
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         IPv4 Address                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         Metric                                |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endverbatim
 */
Int32T 
ipcRibIpv4RouteNotificationDelete(nnBufferT * msgBuff);


/** 
 * Description : 프로토콜이 관심 있는 Ipv6 Best-Mach Route 를 RIB Manager에
 * 등록 요청 메시지를 처리하는 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Route Type  | <= Requesting Protocol     
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         IPv6 Address                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endverbatim
 */
Int32T 
ipcRibIpv6NexthopBestLookupReg(nnBufferT * msgBuff);


/**
 * Description : 프로토콜이 등록한 Ipv6 Best-Mach Route 를 RIB Manager 
 * 에서 삭제 요청 메시지를 처리하는 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Route Type  | <= Requesting Protocol
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         IPv6 Address                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endverbatim
 */
Int32T 
ipcRibIpv6NexthopBestLookupDereg(nnBufferT * msgBuff);


/**
 * Description : 프로토콜이 관심 있는 Ipv6 Exact-Mach Route 를 RIB Manager
 * 에 등록 요청 메시지를 처리하는 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Route Type  | <= Requesting Protocol
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | PrefixLength  |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         IPv6 Address                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endverbatim
 */
Int32T 
ipcRibIpv6NexthopExactLookupReg(nnBufferT * msgBuff);


/**
 * Description : 프로토콜이 등록한 Ipv6 Exact-Mach Route 를 RIB Manager
* 에서 삭제 요청 메시지를 처리하는 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Route Type  | <= Requesting Protocol
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | PrefixLength  |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         IPv6 Address                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endverbatim
 */
Int32T 
ipcRibIpv6NexthopExactLookupDereg(nnBufferT * msgBuff);


/**
 * Description : 프로토콜이 관심 있는 Ipv6 Route 를 RIB Manager에 등록한 
 * 경우, 해당 Route 가 추가되었을 때 RIB Manager 가 해당 프로토콜로 
 * Notification 통보 (프로토콜의 IPC 메시지 처리하는 부분..)
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 
 *  - 0 if 성공
 *  - 1 if 경고
 *  - 2 if 실패 
 */
 /**
 * @verbatim
 * Notification Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | PrefixLength  |              
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         IPv6 Address                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         Metric                                |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |Num of Nexthop |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |Type of Nexthop|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  < if type == NEXTHOP_TYPE_IPV6 >
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                    Gateway IPv4 Address                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  < if type == NEXTHOP_TYPE_IFINDEX > or
 *  < if type == NEXTHOP_TYPE_IFNAME > 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                     Interface Index                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endverbatim
 */
Int32T 
ipcRibIpv6RouteNotificationAdd(nnBufferT * msgBuff);


/**
 * Description : 프로토콜이 관심 있는 Ipv6 Route 를 RIB Manager에 등록한 
 * 경우, 해당 Route 가 삭제되었을 때 RIB Manager 가 해당 프로토콜로 
 * Notification 통보 (프로토콜의 IPC 메시지 처리하는 부분..)
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * Notification Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | PrefixLength  |              
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         IPv4 Address                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         Metric                                |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endverbatim
 */
Int32T 
ipcRibIpv6RouteNotificationDelete(nnBufferT * msgBuff);


#if 0
/**
 * Description : 명령어에 의하여 운용자가 설정된 루트 정보를 확인하는 
 * 명령에 대한 메시지를 처리하는 함수임.
 *
 * @param [in] msgBuff : 메시지 버퍼
 *
 * @retval : 0 if 성공,
 *           1 if 경고,
 *           2 if 실패 
 *
 * @verbatim
 * 1) Related Command 
 *   # show ip route
 *
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    Show ID    | <= ROUTE_SHOW_TYPE_ALL
 * +-+-+-+-+-+-+-+-+
 *
 * 2) Related Command 
 *   # show ip route (bgp|connected|isis|kernel|ospf|rip|static
 *
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    Show ID    | <= ROUTE_SHOW_TYPE_PROTOCOL
 * +-+-+-+-+-+-+-+-+
 * |   Protocol    | <= Protocol ID
 * +-+-+-+-+-+-+-+-+
 *
 * 3) Related Command 
 *   # show ip route A.B.C.D
 *
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    Show ID    | <= ROUTE_SHOW_TYPE_BEST
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                        IPv4 Address                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * 4) Related Command 
 *   # show ip route A.B.C.D/M
 *
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    Show ID    | <= ROUTE_SHOW_TYPE_EXACT
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | PrefixLength  |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                        IPv4 Address                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * 5) Related Command 
 *   # show ip route summary
 *
 * Requested Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    Show ID    | <= ROUTE_SHOW_TYPE_SUMMARY
 * +-+-+-+-+-+-+-+-+
 * @endverbatim
 */
Int32T 
ipcRibIpv4RouteShowReq(nnBufferT * msgBuff);
#endif

extern void ipcSendRouteToProtocol (Int32T, Int32T, PrefixT *, RibT *);

/**
 * Description : 인터페이스 설정/삭제 시점에서 각 프로토콜로 인터페이스
 * 정보를 전달하기 위하여 이벤트 메시지를 전송하는 함수임.
 *
 * @param [in] cmd : 명령(EVENT_INTERFACE_ADD or EVENT_INTERFACE_DELETE)
 * @param [in] pIf : 인터페이스 자료구조 포인터
 * @retval : 0 if 성공,
 *          0 < if 실패
 *
 * @verbatim
 * Event Message format
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
 * @endverbatim
 *
 */
Int32T 
InterfaceUpdate(Uint32T ipcType, Int8T componentId, 
                Uint32T cmd, InterfaceT * pIf);


/**
 * Description : 인터페이스 UP/DOWN 시점에서 각 프로토콜로 인터페이스
 * 정보를 전달하기 위하여 이벤트 메시지를 전송하는 함수임.
 *
 * @param [in] cmd : 명령(EVENT_INTERFACE_UP or EVENT_INTERFACE_DOWN)
 * @param [in] pIf : 인터페이스 자료구조 포인터
 * @retval : 0 if 성공,
 *           0 < if 실패
 *
 * @verbatim
 * Event Message format
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
 * @endverbatim
 *
 */
Int32T 
InterfaceStatusUpdate(Uint32T ipcType, Int8T componentId,
                      Uint32T cmd, InterfaceT *pIf);


/**
 * Description : 인터페이스 주소 추가/삭제 시점에서 각프로토콜로 인터페이스
 * 정보를 전달하기 위하여 이벤트 메시지를 전송하는 함수임.
 *
 * @param [in] cmd : 명령
 *                   EVENT_INTERFACE_ADDRESS_ADD or
 *                   EVENT_INTERFACE_ADDRESS_DELETE
 * @param [in] pIf : 인터페이스 자료구조 포인터
 * @param [in] pIfc : 커넥티드 자료구조 포인터
 *
 * @retval : 0 if 성공,
 *           0 < if 실패
 *
 * @verbatim
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
 * @endverbatim
 *
 */
Int32T 
InterfaceAddressUpdate(Uint32T ipcType, Int8T componentId,
                       Uint32T cmd, InterfaceT *pIf, ConnectedT *pIfc);

#endif /* _ribmgrIpc_h */

