/**************************************************************************************************** 
*                      Electronics and Telecommunications Research Institute
* Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.
*           No part of this software shall be reproduced, stored in a retrieval system, or
*           transmitted by any means, electronic, mechanical, photocopying, recording,
*           or otherwise, without written permission from ETRI.
****************************************************************************************************/

/**
 * @brief       : ribmgr에서 사용하는 Command를 정의하고 ipc 메시지를 전송하는 기능을 
 * 수행한다.
 * - Block Name : Command Manager
 * - Process Name : cm
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/12/22
 */

/**
 * @file        : cmdRib.c
 *
 * $Id: cmdRib.c 1160 2014-04-10 01:56:56Z sckim007 $
 * $Author: sckim007 $
 * $Date: 2014-04-10 10:56:56 +0900 (Thu, 10 Apr 2014) $
 * $Revision: 1160 $
 * $LastChangedBy: sckim007 $
 */

#include "nnTypes.h"
#include "nnStr.h"

#include "nnCmdLink.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"
#include "nnCmdInstall.h"
#include "nosLib.h"
#include "nnCmdDefines.h"
#include "nnUtility.h"

#include "nnRibDefines.h"

#include "nnBuffer.h"
#include "nnPrefix.h"

#include "ribmgrRib.h"
#include "ribmgrRouterid.h"
#include "ribmgrInit.h"
#include "ribmgrDebug.h"


/** @name Command Type
 */
/**@{*/
#define         CMD_DELETE              0                 
#define         CMD_ADD                 1
/**@}*/


/*
 * Description : Static Route 설정 IPC 메시지를 구성하는 함수.
 *
 * param [in] apiBuff : 메시지 버퍼
 * param [in] count : 루트 개수
 * param [in] p : PrefixT 자료구조 포인터
 * param [in] si : StaticIpv4T 자료구조 포인터
 * param [in] msgFlags : 메시지 플래그
 *
 * retval : 메시지 버퍼 길이
 */
static Int32T
buildStaticIpv4(nnBufferT * apiBuff, Uint16T count,
                PrefixT *p, StaticIpv4T *si, Uint8T msgFlags)
{
  Uint32T position = 0;
  Uint8T  routeType;

  /* Number of Routes */
  nnBufferSetInt16T(apiBuff, count);

  /* route type  */
  routeType = RIB_ROUTE_TYPE_STATIC;
  nnBufferSetInt8T (apiBuff, routeType);
  position = nnBufferGetIndex(apiBuff);

  /* route entry length */
  nnBufferSetInt16T(apiBuff, 0);

  /* route flag */
  nnBufferSetInt8T (apiBuff, si->flags);

  /* msgFlags, nexthop, ifindex, distance, metrix
   *     #define RIB_MESSAGE_NEXTHOP                0x01
   *     #define RIB_MESSAGE_IFINDEX                0x02
   *     #define RIB_MESSAGE_DISTANCE               0x04
   *     #define RIB_MESSAGE_METRIC                 0x08
   */
  nnBufferSetInt8T (apiBuff, msgFlags);

  /* prefix length */
  nnBufferSetInt8T (apiBuff, p->prefixLen);

  /* prefix  */
  nnBufferSetInaddr (apiBuff, p->u.prefix4);

  /* type, nexthop type
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
  nnBufferSetInt8T (apiBuff, si->type);

  /* NextHop  */
  if(si->type == NEXTHOP_TYPE_IFINDEX)
  {
    ///TBD
  }
  else if(si->type == NEXTHOP_TYPE_IFNAME)
  {
    /* Nexthop Number */
    /// to be apply

    /* Length of Interface Name */
    Uint8T nameLength = strlen(si->gate.ifName);
    nnBufferSetInt8T (apiBuff, nameLength);

    /* Copy String */
    ///strcpy(buffer+index, si->gate.ifname);
    nnBufferSetString (apiBuff, si->gate.ifName, nameLength);
  }
  else if(si->type == NEXTHOP_TYPE_IPV4)
  {
    /* Nexthop Number */
    /// to be apply

    /* Length of Interface Name */
    nnBufferSetInaddr (apiBuff, si->gate.ipv4);
  }
  else if(si->type == NEXTHOP_TYPE_IPV6)
  {
    ///TBD
  }
  else if(si->type == NEXTHOP_TYPE_NULL0)
  {
    ///TBD
  }
  else
  {
    NNLOG (LOG_ERR, "Error : nexthop type = %d\n", si->type);
    return -1;
  }

  /* Distance */
  if(CHECK_FLAG(msgFlags, RIB_MESSAGE_DISTANCE))
  {
    nnBufferSetInt8T (apiBuff, si->distance);
  }

  /* Message Length */
  Uint16T subLength = nnBufferGetLength(apiBuff) - position - sizeof(Uint16T);
  nnBufferInsertInt16T (apiBuff, position, subLength);

  return nnBufferGetLength(apiBuff);
}



/*
 * Description : Static Route 설정 및 삭제하는 함수.
 *
 * param [in] cmsh : cmsh 포인터
 * param [in] addCmd : 명령어 타입
 * param [in] strDest : PrefixT 자료구조 포인터
 * param [in] strMask : 루트 문자열 포인터
 * param [in] strGate : 게이트웨이 문자열 포인터
 * param [in] strFlag : 루트 플래그 문자열 포인터
 * param [in] strDistance : 디스턴스 문자열 포인터
 *
 * retval : CMD_IPC_ERROR
 *          CMD_IPC_WARNING
 *          CMD_IPC_OK
 */
static Int32T
ribStaticIpv4 (struct cmsh* cmsh, Int32T addCmd, const char * strDest,
               const char * strMask, const char * strGate,
               const char * strFlag, const char * strDistance)
{
  PrefixT  p;
  struct in_addr mask;
  struct in_addr gate;
  Int32T         ret;
  Int32T         nhType;
  Uint8T         distance;

  if (IS_RIBMGR_DEBUG_EVENT)
  {
    NNLOG (LOG_DEBUG, " %s : addCmd[%d] dest[%s] mask[%s] gate[%s] flag[%s] distance[%s]\n", 
           __func__, addCmd, strDest, strMask, strGate, strFlag, strDistance);
  }

  ret = nnCnvStringtoPrefix (&p, (StringT)strDest);

  char prefixBuff[INET_ADDRSTRLEN] = {};
  inet_ntop(AF_INET, &p.u.prefix4.s_addr, prefixBuff, INET_ADDRSTRLEN);

  if (ret != SUCCESS)
  {
    return FAILURE;
  }
  else
  {
    if (IS_RIBMGR_DEBUG_EVENT)
    {
      NNLOG (LOG_DEBUG, "prefix/length  = %s/%d \n", prefixBuff, p.prefixLen);
    }
  }

  /* Apply mask for given prefix. */
  if(strMask)
  {
    if(inet_aton(strMask, &mask) == 0)
    {
      NNLOG (LOG_ERR, "Wrong Mask Address Format \n");
      return CMD_IPC_ERROR;
    }
    /* p.prefixLen = nnCnvNetmask6toPrefixLen (mask); */
    nnCnvNetmasktoPrefixLen (&p.prefixLen, &mask);
  }

  /* Apply mask for given prefix. */
  nnApplyNetmasktoPrefix (&p, &p.prefixLen);

  if (IS_RIBMGR_DEBUG_EVENT)
  {
    inet_ntop(AF_INET, &p.u.prefix4.s_addr, prefixBuff, INET_ADDRSTRLEN);
    NNLOG (LOG_DEBUG, "prefix mask address/length = %s/%d\n", 
           prefixBuff, p.prefixLen);
  }

  /* Administrative distance. */
  if(strDistance)
    distance = atoi(strDistance);
  else
    distance = RIB_DISTANCE_DEFAULT_STATIC;

  /* null0 static route. */
  if(strGate)
  {
    /* Check Nexthop is null0 */
    /* Check Nexthop is IPv4 Address */
    /* Check Nexthop is Interface Name */
    if(strncasecmp(strGate, "null0", strlen(strGate)) == 0)
      nhType = NEXTHOP_TYPE_NULL0;
    else if(inet_aton(strGate, &gate))
      nhType = NEXTHOP_TYPE_IPV4;
    else
      nhType = NEXTHOP_TYPE_IFNAME;
  }
  else
  {
      NNLOG (LOG_ERR, "No Nexthop Value...\n");
      return CMD_IPC_ERROR;
  }

  if(addCmd == CMD_ADD)
  {
    staticAddIpv4 (&p, nhType, strGate, distance);
  }
  else if (addCmd == CMD_DELETE)
  {
    staticDeleteIpv4 (&p, nhType, strGate, distance);
  }
  else
  {
    NNLOG (LOG_ERR, "Wrong Command Type, %d\n", addCmd);
  }

  return 0;
}



/**
 * Description : Static Route 설정 명령 실행시 호출되는 콜백함수.
 *
 * @param [in] cmsh : cmsh 자료구조 포인터
 * @param [in] cargc : 입력인자 수
 * @param [in] cargv : 입력인자 배열
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
 
DECMD(cmdFuncRouteAdd,
	CMD_NODE_CONFIG,
    IPC_RIB_MGR,
	"ip route A.B.C.D/M (A.B.C.D|WORD)",
	"IP related command", 
	"Route related command", 
	"IP destination prefix (e.g. 10.0.0.0/8)", 
	"IP gateway address",
	"IP gateway interface name")
{
#if 0
  cmdPrint(cmsh,"Enter [%s][%s][%d] [%d]\n", __FILE__, __func__, __LINE__, cargc);
  Int32T i;
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  ribStaticIpv4 (cmsh, CMD_ADD, cargv[2], NULL, cargv[3], NULL, NULL);

  cmdPrint(cmsh, "End");

  return CMD_IPC_OK;
}

 
/**
 * Description : Static Route 설정(Distnace 포함) 명령 실행시 호출되는 콜백함수.
 *
 * @param [in] cmsh : cmsh 자료구조 포인터
 * @param [in] cargc : 입력인자 수
 * @param [in] cargv : 입력인자 배열
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncRouteAddDistance,
	CMD_NODE_CONFIG,
    IPC_RIB_MGR,
	"ip route A.B.C.D/M (A.B.C.D|WORD) <1-255>",
	"IP related command", 
	"Route related command", 
	"IP destination prefix (e.g. 10.0.0.0/8)", 
	"IP gateway address", 
	"IP gateway interface name", 
	"Distance value")
{
#if 0
  cmdPrint(cmsh,"Enter [%s][%s][%d] [%d]\n", __FILE__, __func__, __LINE__, cargc);
  Int32T i;
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  ribStaticIpv4 (cmsh, CMD_ADD, cargv[2], NULL, cargv[3], NULL, cargv[4]);

  cmdPrint(cmsh, "End");

  return CMD_IPC_OK;
}


/**
 * Description : Static Route 설정(Mask 포함) 명령 실행시 호출되는 콜백함수.
 *
 * @param [in] cmsh : cmsh 자료구조 포인터
 * @param [in] cargc : 입력인자 수
 * @param [in] cargv : 입력인자 배열
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncRouteAddMask,
	CMD_NODE_CONFIG,
    IPC_RIB_MGR,
	"ip route A.B.C.D A.B.C.D (A.B.C.D|WORD)",
	"IP related command", 
	"Route related command", 
	"IP destination prefix", 
	"IP destination prefix mask", 
	"IP gateway address",
	"IP gateway interface name")
{
#if 0
  cmdPrint(cmsh,"Enter [%s][%s][%d] [%d]\n", __FILE__, __func__, __LINE__, cargc);
  Int32T i;
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  ribStaticIpv4 (cmsh, CMD_ADD, cargv[2], cargv[3], cargv[4], NULL, NULL);

  cmdPrint(cmsh, "End");

  return CMD_IPC_OK;
}


/**
 * Description : Static Route 설정(Mask & Distance 포함) 명령 실행시 호출되는 콜백함수.
 *
 * @param [in] cmsh : cmsh 자료구조 포인터
 * @param [in] cargc : 입력인자 수
 * @param [in] cargv : 입력인자 배열
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncRouteAddMaskDistance,
	CMD_NODE_CONFIG,
    IPC_RIB_MGR,
	"ip route A.B.C.D A.B.C.D (A.B.C.D|WORD) <1-255>",
	"IP related command", 
	"Route related command", 
	"IP destination prefix", 
	"IP destination prefix mask", 
	"IP gateway address",
	"IP gateway interface name",
	"Distance value")
{
#if 0
  cmdPrint(cmsh,"Enter [%s][%s][%d] [%d]\n", __FILE__, __func__, __LINE__, cargc);
  Int32T i;
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  ribStaticIpv4 (cmsh, CMD_ADD, cargv[2], cargv[3], cargv[4], NULL, cargv[5]);

  cmdPrint(cmsh, "End");

  return CMD_IPC_OK;
}


/**
 * Description : Static Route 삭제 명령 실행시 호출되는 콜백함수.
 *
 * @param [in] cmsh : cmsh 자료구조 포인터
 * @param [in] cargc : 입력인자 수
 * @param [in] cargv : 입력인자 배열
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncRouteDelete,
	CMD_NODE_CONFIG,
    IPC_RIB_MGR,
	"no ip route A.B.C.D/M (A.B.C.D|WORD)",
	"no",
	"IP related command", 
	"Route related command", 
	"IP destination prefix (e.g. 10.0.0.0/8)", 
	"IP gateway address",
	"IP gateway interface name")
{
#if 0
  cmdPrint(cmsh,"Enter [%s][%s][%d] [%d]\n", __FILE__, __func__, __LINE__, cargc);
  Int32T i;
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  ribStaticIpv4 (cmsh, CMD_DELETE, cargv[3], NULL, cargv[4], NULL, NULL);

  cmdPrint(cmsh, "End");

  return CMD_IPC_OK;
} 


/**
 * Description : Static Route 삭제(Distance 포함) 명령 실행시 호출되는 콜백함수.
 *
 * @param [in] cmsh : cmsh 자료구조 포인터
 * @param [in] cargc : 입력인자 수
 * @param [in] cargv : 입력인자 배열
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncRouteDeleteDistance,
	CMD_NODE_CONFIG,
    IPC_RIB_MGR,
	"no ip route A.B.C.D/M (A.B.C.D|WORD) <1-255>",
	"no",
	"IP related command", 
	"Route related command", 
	"IP destination prefix (e.g. 10.0.0.0/8)", 
	"IP gateway address",
	"IP gateway interface name",
	"Distance value")
{
#if 0
  cmdPrint(cmsh,"Enter [%s][%s][%d] [%d]\n", __FILE__, __func__, __LINE__, cargc);
  Int32T i;
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  ribStaticIpv4 (cmsh, CMD_DELETE, cargv[3], NULL, cargv[4], NULL, cargv[5]);

  cmdPrint(cmsh, "End");

  return CMD_IPC_OK;
}


/**
 * Description : Static Route 삭제(Mask 포함) 명령 실행시 호출되는 콜백함수.
 *
 * @param [in] cmsh : cmsh 자료구조 포인터
 * @param [in] cargc : 입력인자 수
 * @param [in] cargv : 입력인자 배열
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncRouteDeleteMask,
	CMD_NODE_CONFIG,
    IPC_RIB_MGR,
	"no ip route A.B.C.D A.B.C.D (A.B.C.D|WORD)",
	"no",
	"IP related command", 
	"Route related command", 
	"IP destination prefix", 
	"IP destination prefix mask", 
	"IP gateway address",
	"IP gateway interface name")
{
#if 0
  cmdPrint(cmsh,"Enter [%s][%s][%d] [%d]\n", __FILE__, __func__, __LINE__, cargc);
  Int32T i;
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  ribStaticIpv4 (cmsh, CMD_DELETE, cargv[3], cargv[4], cargv[5], NULL, NULL);

  cmdPrint(cmsh, "End");

  return CMD_IPC_OK;
}


/**
 * Description : Static Route 삭제(Mask and Distance 포함) 명령 실행시 호출되는 콜백함수.
 *
 * @param [in] cmsh : cmsh 자료구조 포인터
 * @param [in] cargc : 입력인자 수
 * @param [in] cargv : 입력인자 배열
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncRouteDeleteMaskDistance,
	CMD_NODE_CONFIG,
    IPC_RIB_MGR,
	"no ip route A.B.C.D A.B.C.D (A.B.C.D|WORD) <1-255>",
	"no",
	"IP related command", 
	"Route related command", 
	"IP destination prefix", 
	"IP destination prefix mask", 
	"IP gateway address",
	"IP gateway interface name",
	"Distance value")
{
#if 0
  cmdPrint(cmsh,"Enter [%s][%s][%d] [%d]\n", __FILE__, __func__, __LINE__, cargc);
  Int32T i;
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  ribStaticIpv4 (cmsh, CMD_DELETE, cargv[3], cargv[4], cargv[5], NULL, cargv[6]);

  cmdPrint(cmsh, "End");

  return CMD_IPC_OK;
}


/**
 * Description : RouterID를 갱신하는 명령에 대한 콜백함수.
 *
 * @param [in] cmsh : cmsh 자료구조 포인터
 * @param [in] cargc : 입력인자 수
 * @param [in] cargv : 입력인자 배열
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncRouteridSet,
	CMD_NODE_CONFIG,
    IPC_RIB_MGR,
	"router-id A.B.C.D",
	"Routerid related command", 
	"Router Id(A.B.C.D)")
{
#if 0
  cmdPrint(cmsh,"Enter [%s][%s][%d] [%d]\n", __FILE__, __func__, __LINE__, cargc);
  Int32T i;
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  Prefix4T routerId;

  if (strcmp("router-id", cargv[0]) == 0)
  {
    routerId.prefix.s_addr = inet_addr (cargv[1]);
    /// check
    if (!routerId.prefix.s_addr)
    {
      sprintf(cmsh->outputStr, "Malformed Address\n");
      cmdPrint (cmsh, "End");
      return CMD_IPC_WARNING; /* to be changed to CMD_IPC_WARNING */
    }
    routerId.prefixLen = 32;
    routerId.family = AF_INET;
 
    /* Set Router ID*/ 
    routerIdSet (&routerId);
  }
  else if ((strcmp("no", cargv[0]) == 0) && (strcmp("router-id", cargv[1])==0))
  {
    memset(&routerId, 0, sizeof(PrefixT));
    routerId.prefixLen = 32;
    routerId.family = AF_INET;

    /* Set Router ID*/ 
    routerIdSet (&routerId);
  }

  cmdPrint(cmsh, "End");

  return CMD_IPC_OK;
}
ALICMD(cmdFuncRouteridSet,
	CMD_NODE_CONFIG,
    IPC_RIB_MGR,
	"no router-id",
	"no", 
	"Routerid related command" );


#define SHOW_ROUTE_V4_HEADER "Codes: K - kernel route, C - connected, " \
  "S - static, R - RIP, O - OSPF,%s I - ISIS, B - BGP, " \
  "> - selected route, * - FIB route%s%s"

/* Verbose information for IPv4 route. */
static void
showIpRouteVerbose (char * showBuff, RouteNodeT *pRNode, RibT *pRib)
{
  NexthopT *nexthop = NULL;
  Int32T len = 0, buffLength = 0;
  char buf[BUFSIZ] = {};

  /* Nexthop information. */
  for (nexthop = pRib->pNexthop; nexthop; nexthop = nexthop->next)
  {
    Int32T idx = 0;

    if (nexthop == pRib->pNexthop)
    {
      /* Prefix information. */
      idx = sprintf (showBuff + idx, "%c%c%c %s/%d",
                     nnRoute2Char (pRib->type),
                     CHECK_FLAG (pRib->flags, RIB_FLAG_SELECTED) ? '>' : ' ',
                     CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB) ? '*' : ' ',
                     (char *)inet_ntop (AF_INET, &pRNode->p.u.prefix4, buf, BUFSIZ),
                     pRNode->p.prefixLen);
      len = idx;

      /* Distance and metric display. */
      if (pRib->type != RIB_ROUTE_TYPE_CONNECT
          && pRib->type != RIB_ROUTE_TYPE_KERNEL)
      {
        idx += sprintf (showBuff + idx,
                        " [%d/%d]", pRib->distance, pRib->metric);
        len += idx;
      }
    }
    else
    {
      idx += sprintf (showBuff + idx, "  %c%*c",
                     CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_FIB) ? '*' : ' ',
                     len - 3, ' ');
    }

    switch (nexthop->type)
    {
      case NEXTHOP_TYPE_IPV4:
      case NEXTHOP_TYPE_IPV4_IFINDEX:
        idx += sprintf (showBuff + idx, " via %s",
                          (char *)inet_ntoa (nexthop->gate.ipv4));

        if (nexthop->ifIndex)
          idx += sprintf (showBuff + idx, ", %s",
                            (char *)ifIndex2ifName (nexthop->ifIndex));
        break;
      case NEXTHOP_TYPE_IFINDEX:
        idx += sprintf (showBuff + idx, " is directly connected, %s",
                          (char *)ifIndex2ifName (nexthop->ifIndex));
        break;
      case NEXTHOP_TYPE_IFNAME:
        idx += sprintf (showBuff + idx, " is directly connected, %s",
                          (char *)nexthop->ifName);
        break;
      case NEXTHOP_TYPE_NULL0:
        idx += sprintf (showBuff + idx, " is directly connected, Null0");
        break;
      default:
        break;
    }

    if (! CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_ACTIVE))
    {
      idx += sprintf (showBuff + idx, " inactive");
    }

    if (CHECK_FLAG (nexthop->flags, NEXTHOP_FLAG_RECURSIVE))
    {
      idx += sprintf (showBuff + idx, " (recursive");

      switch (nexthop->rType)
      {
        case NEXTHOP_TYPE_IPV4:
        case NEXTHOP_TYPE_IPV4_IFINDEX:
          idx += sprintf (showBuff + idx, " via %s)",
                            (char *)inet_ntoa (nexthop->rGate.ipv4));
          break;
        case NEXTHOP_TYPE_IFINDEX:
        case NEXTHOP_TYPE_IFNAME:
          idx += sprintf (showBuff + idx, " is directly connected, %s)",
                            (char *)ifIndex2ifName (nexthop->rifIndex));
          break;
        default:
          break;
      }
    }
    switch (nexthop->type)
    {
      case NEXTHOP_TYPE_IPV4:
      case NEXTHOP_TYPE_IPV4_IFINDEX:
      case NEXTHOP_TYPE_IPV4_IFNAME:
        if (nexthop->src.ipv4.s_addr)
        {
          if (inet_ntop(AF_INET, &nexthop->src.ipv4, buf, sizeof buf))
          {
            idx += sprintf (showBuff + idx, ", src %s", buf);
          }
        }
        break;
#ifdef HAVE_IPV6
      case NEXTHOP_TYPE_IPV6:
      case NEXTHOP_TYPE_IPV6_IFINDEX:
      case NEXTHOP_TYPE_IPV6_IFNAME:
        if (!PREFIX_IPV6_ADDR_SAME(&nexthop->src.ipv6, &in6addr_any))
        {
          if (inet_ntop(AF_INET6, &nexthop->src.ipv6, buf, sizeof buf))
          {
            idx += sprintf (showBuff + idx, ", src %s", buf);
          }
        }
        break;
#endif /* HAVE_IPV6 */
      default:
        break;
    }
    if (CHECK_FLAG (pRib->flags, RIB_FLAG_NULL0))
    {
      idx += sprintf (showBuff + idx, ", bh");
    }

    if (CHECK_FLAG (pRib->flags, RIB_FLAG_REJECT))
    {
      idx += sprintf (showBuff + idx, ", rej");
    }

    if (pRib->type == RIB_ROUTE_TYPE_RIP ||
        pRib->type == RIB_ROUTE_TYPE_OSPF ||
        pRib->type == RIB_ROUTE_TYPE_ISIS ||
        pRib->type == RIB_ROUTE_TYPE_BGP)
    {
      time_t uptime;
      struct tm *tm;

      uptime = time (NULL);
      uptime -= pRib->uptime;
      tm = gmtime (&uptime);

#define ONE_DAY_SECOND 60*60*24
#define ONE_WEEK_SECOND 60*60*24*7

      if (uptime < ONE_DAY_SECOND)
      {
        idx += sprintf (showBuff + idx, ", %02d:%02d:%02d",
                          tm->tm_hour, tm->tm_min, tm->tm_sec);
      }
      else if (uptime < ONE_WEEK_SECOND)
      {
        idx += sprintf (showBuff + idx, ", %dd%02dh%02dm",
                          tm->tm_yday, tm->tm_hour, tm->tm_min);
      }
      else
      {
        idx += sprintf (showBuff + idx, ", %02dw%dd%02dh",
                          tm->tm_yday/7,
                          tm->tm_yday - ((tm->tm_yday/7) * 7), tm->tm_hour);
      }
    }
    buffLength += idx;
  }
  sprintf (showBuff + buffLength, "\n");
}


/* Detailed information for IPv4 route. */
static void
showIpRouteDetail (char * showBuff, RouteNodeT *pRNode)
{
  RibT *pRib = NULL;
  NexthopT *pNextHop = NULL;
  Int32T idx = 0;

  for (pRib = pRNode->pInfo; pRib; pRib = pRib->next)
  {
    idx += sprintf (showBuff + idx, "Routing entry for %s/%d \n",
             (char *)inet_ntoa (pRNode->p.u.prefix4), pRNode->p.prefixLen);
    idx += sprintf (showBuff + idx, "  Known via \"%s\"", (char *)nnRoute2String (pRib->type));
    idx += sprintf (showBuff + idx, ", distance %d, metric %d", pRib->distance, pRib->metric);
    if (CHECK_FLAG (pRib->flags, RIB_FLAG_SELECTED))
    {
      idx += sprintf (showBuff + idx, ", best");
    }
    if (pRib->refCount)
    {
      idx += sprintf (showBuff + idx, ", refCount %d", pRib->refCount);
    }
    if (CHECK_FLAG (pRib->flags, RIB_FLAG_NULL0))
    {
      idx += sprintf (showBuff + idx, ", blackhole");
    }
    if (CHECK_FLAG (pRib->flags, RIB_FLAG_REJECT))
    {
      idx += sprintf (showBuff + idx, ", reject");
    }
    idx += sprintf (showBuff + idx, "\n");

#define ONE_DAY_SECOND 60*60*24
#define ONE_WEEK_SECOND 60*60*24*7
    if (pRib->type == RIB_ROUTE_TYPE_RIP ||
        pRib->type == RIB_ROUTE_TYPE_OSPF ||
        pRib->type == RIB_ROUTE_TYPE_ISIS ||
        pRib->type == RIB_ROUTE_TYPE_BGP)
    {
      time_t uptime;
      struct tm *tm;

      uptime = time (NULL);
      uptime -= pRib->uptime;
      tm = gmtime (&uptime);

      idx += sprintf(showBuff + idx, "  Last update ");

      if (uptime < ONE_DAY_SECOND)
      {
        idx += sprintf(showBuff + idx, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
      }
      else if (uptime < ONE_WEEK_SECOND)
      {
        idx += sprintf(showBuff + idx, "%dd%02dh%02dm", tm->tm_yday, tm->tm_hour, tm->tm_min);
      }
      else
      {
        idx += sprintf(showBuff + idx, "%02dw%dd%02dh",
                   tm->tm_yday/7,
                   tm->tm_yday - ((tm->tm_yday/7) * 7), tm->tm_hour);
      }
      idx += sprintf(showBuff + idx, " ago\n");
    }

    for (pNextHop = pRib->pNexthop; pNextHop; pNextHop = pNextHop->next)
    {
      char addrstr[32];

      idx += sprintf(showBuff + idx, "  %c",
             CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB) ? '*' : ' ');

      switch (pNextHop->type)
      {
        case NEXTHOP_TYPE_IPV4:
        case NEXTHOP_TYPE_IPV4_IFINDEX:
          idx += sprintf (showBuff + idx, " %s", (char *)inet_ntoa (pNextHop->gate.ipv4));
          if (pNextHop->ifIndex)
          {
             idx += sprintf (showBuff + idx, ", via %s", (char *)ifIndex2ifName (pNextHop->ifIndex));
          }
          break;
        case NEXTHOP_TYPE_IFINDEX:
          idx += sprintf (showBuff + idx, " directly connected, %s",
                   (char *)ifIndex2ifName (pNextHop->ifIndex));
          break;
        case NEXTHOP_TYPE_IFNAME:
          idx += sprintf (showBuff + idx, " directly connected, %s", (char *)pNextHop->ifName);
          break;
        case NEXTHOP_TYPE_NULL0:
          idx += sprintf (showBuff + idx, " directly connected, Null0");
          break;
        default:
          break;
      }
      if (! CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE))
      {
         idx += sprintf (showBuff + idx, " inactive");
      }

      if (CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_RECURSIVE))
      {
        idx += sprintf (showBuff + idx, " (recursive");

        switch (pNextHop->rType)
        {
          case NEXTHOP_TYPE_IPV4:
          case NEXTHOP_TYPE_IPV4_IFINDEX:
            idx += sprintf (showBuff + idx, " via %s)", (char *)inet_ntoa (pNextHop->rGate.ipv4));
            break;
          case NEXTHOP_TYPE_IFINDEX:
          case NEXTHOP_TYPE_IFNAME:
            idx += sprintf (showBuff + idx, " is directly connected, %s)",
                     (char *)ifIndex2ifName (pNextHop->rifIndex));
            break;
          default:
            break;
        }
      }

      switch (pNextHop->type)
      {
        case NEXTHOP_TYPE_IPV4:
        case NEXTHOP_TYPE_IPV4_IFINDEX:
        case NEXTHOP_TYPE_IPV4_IFNAME:
          if (pNextHop->src.ipv4.s_addr)
          {
            if (inet_ntop(AF_INET, &pNextHop->src.ipv4, addrstr,
                      sizeof addrstr))
            {
              idx += sprintf (showBuff + idx, ", src %s", addrstr);
            }
          }
          break;
#ifdef HAVE_IPV6
        case NEXTHOP_TYPE_IPV6:
        case NEXTHOP_TYPE_IPV6_IFINDEX:
        case NEXTHOP_TYPE_IPV6_IFNAME:
          if (!PREFIX_IPV6_ADDR_SAME(&pNextHop->src.ipv6, &in6addr_any))
          {
            if (inet_ntop(AF_INET6, &pNextHop->src.ipv6, addrstr,
               sizeof addrstr))
            {
              idx += sprintf (showBuff + idx, ", src %s", addrstr);
            }
          }
          break;
#endif /* HAVE_IPV6 */
        default:
          break;
      }
      idx += sprintf (showBuff + idx, "\n");
    }
    idx += sprintf (showBuff + idx, "\n");
  }

}



/**
 * Description : 모든 루트를 출력하는 명령에 대한 콜백함수.
 *
 * @param [in] cmsh : cmsh 자료구조 포인터
 * @param [in] cargc : 입력인자 수
 * @param [in] cargv : 입력인자 배열
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncShowRoute,
	CMD_NODE_EXEC,
    IPC_RIB_MGR|IPC_SHOW_MGR,
	"show ip route",
	"Show related command", 
	"Ip related command",
	"Route related command")
{
  RouteTableT *pTable = NULL;
  RouteNodeT *pRNode = NULL;
  RibT *pRib = NULL;
  Int32T first = 1;
#if 0
  Int32T i = 0;

  cmdPrint(cmsh,"Enter [%s][%s][%d] [%d]\n", __FILE__, __func__, __LINE__, cargc);

  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  /* Show Start. */
  /* Lookup Table. */
  pTable = vrfTable (AFI_IP, SAFI_UNICAST, 0);
  if (! pTable)
  {
    cmdPrint (cmsh, "End");
    return CMD_IPC_WARNING;
  }

  /* Show all IPv4 routes. */
  for (pRNode = nnRouteTop (pTable); pRNode; pRNode = nnRouteNext (pRNode))
    for (pRib = pRNode->pInfo; pRib; pRib = pRib->next)
    {
      char showBuff[1024] = {};
      if (first)
      {
        /* Print route table header. */
        sprintf(showBuff, SHOW_ROUTE_V4_HEADER, "\n", "\n", "\n");
        cmdPrint (cmsh, "%s", showBuff);

        first = 0;
      }

        /* Print route entries. */
      memset(showBuff, 0, 1024);
      showIpRouteVerbose (showBuff, pRNode, pRib);
      cmdPrint (cmsh, "%s", showBuff);
    }

  /* Show End. */
  cmdPrint(cmsh,"End");

  return CMD_IPC_OK;
}


/**
 * Description : 프로토콜에 해당하는 루트를 출력하는 명령에 대한 콜백함수.
 *
 * @param [in] cmsh : cmsh 자료구조 포인터
 * @param [in] cargc : 입력인자 수
 * @param [in] cargv : 입력인자 배열
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncShowRouteProtocol,
	CMD_NODE_EXEC,
    IPC_RIB_MGR|IPC_SHOW_MGR,
	"show ip route (bgp|connected|isis|kernel|ospf|rip|static)",
	"show", 
	"ip",
	"route",
	"bgp route",
	"connected route",
	"isis route",
	"kernel route",
	"ospf route",
	"rip route",
	"static route")
{
  RouteTableT *pTable = NULL;
  RouteNodeT *pRNode = NULL;
  RibT *pRib = NULL;
  Uint8T  protocolType = 0;
  Int32T first = 1;
#if 0
  Int32T i = 0;
  cmdPrint(cmsh,"Enter [%s][%s][%d] [%d]\n", __FILE__, __func__, __LINE__, cargc);
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  /* Comparision each protocol's types. */
  if (strncmp (cargv[3], "b", 1) == 0)
    protocolType = RIB_ROUTE_TYPE_BGP;
  else if (strncmp (cargv[3], "c", 1) == 0)
    protocolType = RIB_ROUTE_TYPE_CONNECT;
  else if (strncmp (cargv[3], "k", 1) ==0)
    protocolType = RIB_ROUTE_TYPE_KERNEL;
  else if (strncmp (cargv[3], "o", 1) == 0)
    protocolType = RIB_ROUTE_TYPE_OSPF;
  else if (strncmp (cargv[3], "i", 1) == 0)
    protocolType = RIB_ROUTE_TYPE_ISIS;
  else if (strncmp (cargv[3], "r", 1) == 0)
    protocolType = RIB_ROUTE_TYPE_RIP;
  else if (strncmp (cargv[3], "s", 1) == 0)
    protocolType = RIB_ROUTE_TYPE_STATIC;
  else
    {   
      return CMD_IPC_WARNING;
    }

  /* Start Show. */
  pTable = vrfTable (AFI_IP, SAFI_UNICAST, 0);
  if (! pTable)
  {
    cmdPrint (cmsh, "End");
    return CMD_IPC_WARNING;
  }

  /* Show matched type IPv4 routes. */
  for (pRNode = nnRouteTop (pTable); pRNode; pRNode = nnRouteNext (pRNode))
    for (pRib = pRNode->pInfo; pRib; pRib = pRib->next)
      if (pRib->type == protocolType)
      {
        char showBuff[1024] ={};
        if (first)
        {
          /* Print route table header. */
          sprintf (showBuff, SHOW_ROUTE_V4_HEADER, "\n", "\n", "\n");
          cmdPrint (cmsh, "%s", showBuff);
          first = 0;
        }

        /* Print route entries. */
        memset(showBuff, 0, 1024);
        showIpRouteVerbose (showBuff, pRNode, pRib);
        cmdPrint (cmsh, "%s", showBuff);
      }

  /* End Show. */
  cmdPrint(cmsh, "End"); 

  return CMD_IPC_OK;  
}


/**
 * Description : Best Lookup에 해당하는 루트를 출력하는 명령에 대한 콜백함수.
 *
 * @param [in] cmsh : cmsh 자료구조 포인터
 * @param [in] cargc : 입력인자 수
 * @param [in] cargv : 입력인자 배열
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncShowRouteAddress,
	CMD_NODE_EXEC,
    IPC_RIB_MGR|IPC_SHOW_MGR,
	"show ip route A.B.C.D",
	"show", 
	"ip",
	"route",
	"IP destination prefix (e.g. 1.2.3.4)")
{
  RouteTableT *pTable = NULL;
  RouteNodeT *pRNode = NULL;
#if 0
  Int32T i = 0;
  cmdPrint(cmsh,"Enter [%s][%s][%d] [%d]\n", __FILE__, __func__, __LINE__, cargc);
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  Int32T ret = 0;
  Prefix4T prefix4 = {0,};
  PrefixT prefix = {0,};

  ret = nnCnvStringtoPrefix4 (&prefix4, (StringT)cargv[3]);
  if (ret < 0)
  {
    NNLOG (LOG_WARNING, "%% Wrong ip format : %s.\n", cargv[3]);
    cmdPrint (cmsh, "End");
    return CMD_IPC_WARNING;
  }

  /* Start show .*/
  /* Select Table for Ipv4 and Unicast */
  pTable = vrfTable (AFI_IP, SAFI_UNICAST, 0);
  if (! pTable)
  {
    NNLOG (LOG_WARNING, "%% Table not exist.\n");
    cmdPrint (cmsh, "End");
    return CMD_IPC_WARNING;
  }

  /* Lock Route Node */
  nnCnvPrefix4TtoPrefixT (&prefix, &prefix4);
  pRNode = nnRouteNodeMatch (pTable, &prefix);
  if (! pRNode)
  {
    NNLOG (LOG_WARNING, "%% Network not in table\n");
    cmdPrint (cmsh, "End");
    return RIBMGR_WARNING;
  }

  /* Display Route Information of Route Node */
  char showBuff[1024] = {};
  showIpRouteDetail (showBuff, pRNode);
  cmdPrint (cmsh, "%s", showBuff);

  /* UnLock Route Node */
  nnRouteNodeUnlock (pRNode);
  /* End show .*/
  cmdPrint(cmsh, "End");

  return CMD_IPC_OK;
}


/**
 * Description : Exact Lookup에 해당하는 루트를 출력하는 명령에 대한 콜백함수.
 *
 * @param [in] cmsh : cmsh 자료구조 포인터
 * @param [in] cargc : 입력인자 수
 * @param [in] cargv : 입력인자 배열
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncShowRoutePrefix,
	CMD_NODE_EXEC,
    IPC_RIB_MGR|IPC_SHOW_MGR,
	"show ip route A.B.C.D/M",
	"show", 
	"ip",
	"route",
	"IP destination prefix mask (e.g. 1.2.3.0/24)" )
{
  RouteTableT *pTable = NULL;
  RouteNodeT *pRNode = NULL;
#if 0
  Int32T i = 0;
  cmdPrint(cmsh,"Enter [%s][%s][%d] [%d]\n", __FILE__, __func__, __LINE__, cargc);
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif
  Int32T ret = 0;
  Prefix4T prefix4 = {0,};
  PrefixT prefix = {0,};

  ret = nnCnvStringtoPrefix4 (&prefix4, (StringT)cargv[3]);
  if (ret < 0)
  {
    cmdPrint (cmsh, "End");
    return CMD_IPC_WARNING;
  }

  /* Start show .*/
  /* Select Table for Ipv4 and Unicast */
  pTable = vrfTable (AFI_IP, SAFI_UNICAST, 0);
  if (! pTable)
  {
    NNLOG (LOG_WARNING, "%% Table not exist.\n");
    cmdPrint (cmsh, "End");
    return CMD_IPC_WARNING;
  }

  /* Lock Route Node */
  nnCnvPrefix4TtoPrefixT (&prefix, &prefix4);
  pRNode = nnRouteNodeMatch (pTable, &prefix);
  if (! pRNode || pRNode->p.prefixLen != prefix.prefixLen)
  {
    NNLOG (LOG_WARNING, "%% Network not in table\n");
    cmdPrint (cmsh, "End");
    return CMD_IPC_WARNING;
  }

  /* Display Route Information of Route Node */
  char showBuff[1024] = {};
  showIpRouteDetail (showBuff, pRNode);
  cmdPrint (cmsh, "%s", showBuff);

  /* UnLock Route Node */
  nnRouteNodeUnlock (pRNode);

  /* End show .*/
  cmdPrint(cmsh, "End");


  return CMD_IPC_OK;
}

/**
 * Description : 요약된 루트정보를 출력하는 명령에 대한 콜백함수.
 *
 * @param [in] cmsh : cmsh 자료구조 포인터
 * @param [in] cargc : 입력인자 수
 * @param [in] cargv : 입력인자 배열
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncShowRouteSummary,
	CMD_NODE_EXEC,
    IPC_RIB_MGR|IPC_SHOW_MGR,
	"show ip route summary",
	"show", 
	"ip",
	"route",
	"summary" )
{
  RouteTableT *pTable = NULL;
  RouteNodeT *pRNode = NULL;
  NexthopT *pNextHop = NULL;
  RibT *pRib = NULL;
  Int32T i = 0;

#if 0
  cmdPrint(cmsh,"Enter [%s][%s][%d] [%d]\n", __FILE__, __func__, __LINE__, cargc);
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  /* Start Show. */
  pTable = vrfTable (AFI_IP, SAFI_UNICAST, 0);
  if (! pTable)
  {
    cmdPrint(cmsh, "End");
    return CMD_IPC_WARNING;
  }

#define RIB_ROUTE_IBGP  RIB_ROUTE_TYPE_MAX
#define RIB_ROUTE_TOTAL (RIB_ROUTE_IBGP + 1)
  Uint32T rib_cnt[RIB_ROUTE_TOTAL + 1] = {};
  Uint32T fib_cnt[RIB_ROUTE_TOTAL + 1] = {};

  memset (&rib_cnt, 0, sizeof(rib_cnt));
  memset (&fib_cnt, 0, sizeof(fib_cnt));
  for (pRNode = nnRouteTop (pTable); pRNode; pRNode = nnRouteNext (pRNode))
    for (pRib = pRNode->pInfo; pRib; pRib = pRib->next)
      for (pNextHop = pRib->pNexthop; pNextHop; pNextHop = pNextHop->next)
      {
        rib_cnt[RIB_ROUTE_TOTAL]++;
        rib_cnt[pRib->type]++;
        if (CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB))
        {
          fib_cnt[RIB_ROUTE_TOTAL]++;
          fib_cnt[pRib->type]++;
        }
        if (pRib->type == RIB_ROUTE_TYPE_BGP &&
              CHECK_FLAG (pRib->flags, RIB_FLAG_IBGP))
        {
          rib_cnt[RIB_ROUTE_IBGP]++;
          if (CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB))
          {
            fib_cnt[RIB_ROUTE_IBGP]++;
          }
        }
      }

  char showBuff[1024] = {};
  Int32T idx = 0;

  idx +=
  sprintf (showBuff + idx, "%-20s %-20s %-20s \n",
                    "Route Source", "Routes", "FIB");
  /* Print table header. */
  cmdPrint (cmsh, "%s", showBuff);

  for (i = 0; i < RIB_ROUTE_TYPE_MAX; i++)
  {
    idx = 0;
    memset(showBuff, 0, 1024);

    if (rib_cnt[i] > 0)
    {
      if (i == RIB_ROUTE_TYPE_BGP)
      {
        idx +=
        sprintf (showBuff + idx, "%-20s %-20d %-20d \n",
                 "ebgp",
                 rib_cnt[RIB_ROUTE_TYPE_BGP] - rib_cnt[RIB_ROUTE_IBGP],
                 fib_cnt[RIB_ROUTE_TYPE_BGP] - fib_cnt[RIB_ROUTE_IBGP]);
        idx +=
        sprintf (showBuff + idx, "%-20s %-20d %-20d \n",
                 "ibgp",
                 rib_cnt[RIB_ROUTE_IBGP], fib_cnt[RIB_ROUTE_IBGP]);
      }
      else
      {
        idx +=
        sprintf (showBuff + idx, "%-20s %-20d %-20d \n",
                 (char *)nnRoute2String(i), rib_cnt[i], fib_cnt[i]);
      }
    }

    cmdPrint (cmsh, "%s", showBuff);
  }

  idx = 0;
  memset(showBuff, 0, 1024);

  idx +=
  sprintf (showBuff + idx, "------\n");
  cmdPrint (cmsh, "%s", showBuff);

  idx = 0;
  memset(showBuff, 0, 1024);

  idx +=
  sprintf (showBuff + idx, "%-20s %-20d %-20d \n",
           "Totals", rib_cnt[RIB_ROUTE_TOTAL], fib_cnt[RIB_ROUTE_TOTAL]);
  cmdPrint (cmsh, "%s", showBuff);

  /* End Show. */
  cmdPrint(cmsh, "End");

  return CMD_IPC_OK;
}


DECMD (cmdFuncRibmgrDebugEvents,
       CMD_NODE_CONFIG,
       IPC_RIB_MGR,
       "debug ribmgr events",
       "Debugging functions",
       "Routing Information Base Manager (ribmgr)",
       "Ribmgr events")
{
  pRibmgr->ribmgrDebugEvent =  RIBMGR_DEBUG_EVENT;

  return CMD_IPC_OK;
}

DECMD (cmdFuncNoRibmgrDebugEvents,
       CMD_NODE_CONFIG,
       IPC_RIB_MGR,
       "no debug ribmgr events",
       "Negate a command or set its defaults",
       "Debugging functions",
       "Routing Information Base Manager (ribmgr)",
       "Ribmgr events")
{
  pRibmgr->ribmgrDebugEvent = 0; 

  return CMD_IPC_OK;
}


DECMD (cmdFuncRibmgrDebugKernel,
       CMD_NODE_CONFIG,
       IPC_RIB_MGR,
       "debug ribmgr kernel",
       "Debugging functions",
       "Routing Information Base Manager (ribmgr)",
       "Kernel interface information")
{
  pRibmgr->ribmgrDebugKernel =  RIBMGR_DEBUG_EVENT;

  return CMD_IPC_OK;
}

DECMD (cmdFuncNoRibmgrDebugKernel,
       CMD_NODE_CONFIG,
       IPC_RIB_MGR,
       "no debug ribmgr kernel",
       "Negate a command or set its defaults",
       "Debugging functions",
       "Routing Information Base Manager (ribmgr)",
       "Kernel interface information")
{
  pRibmgr->ribmgrDebugKernel = 0; 

  return CMD_IPC_OK;
}


DECMD (cmdFuncRibmgrDebugRib,
       CMD_NODE_CONFIG,
       IPC_RIB_MGR,
       "debug ribmgr rib",
       "Debugging functions",
       "Routing Information Base Manager (ribmgr)",
       "Route Information Base information")
{
  pRibmgr->ribmgrDebugRib =  RIBMGR_DEBUG_EVENT;

  return CMD_IPC_OK;
}

DECMD (cmdFuncNoRibmgrDebugRib,
       CMD_NODE_CONFIG,
       IPC_RIB_MGR,
       "no debug ribmgr rib",
       "Negate a command or set its defaults",
       "Debugging functions",
       "Routing Information Base Manager (ribmgr)",
       "Route Information Base information")
{
  pRibmgr->ribmgrDebugRib = 0; 

  return CMD_IPC_OK;
}


