/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : RIB Manager???? ?????ϴ? ???? ?????? ??�� ?? ?ʱ?ȭ ?Լ??? ��???? ????
 * 
 * - Block Name : RIB Manager
 * - Process Name : ribmgr
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : ribmgrInit.c
 *
 * $Author: sckim007 $
 * $Date: 2014-02-17 09:53:56 +0900 (Mon, 17 Feb 2014) $
 * $Revision: 860 $
 * $LastChangedBy: sckim007 $
 */
#include <stdlib.h>
#include <sys/socket.h>
#include <bits/sockaddr.h>
#include <linux/netlink.h>

#include "nnTypes.h"
#include "nnStr.h"
#include "nnVector.h"
#include "nnList.h"
#include "nnPrefix.h"
#include "nnBuffer.h"

#include "nnCmdDefines.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCompProcess.h"

#include "nosLib.h"
#include "lcsService.h"

#include "ribmgrInit.h"
#include "ribmgrRib.h"
#include "ribmgrRouterid.h"
#include "ribmgrDebug.h"
#include "ribmgrIpc.h"


/*
 * External Definitions for Global Data Structure
 */
extern void ** gCompData;


/*
 * External Definitions are here
 */
extern Int32T netlinkInit();
extern void kernelUpdate (void);


/*
 * Rib manager's data structure using in shared code.
 */
RibmgrT  * pRibmgr = NULL;


/*
 * Description : When we display running config, this function will be called.
 */
Int32T
ribWriteConfCB(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv)
{

  if (IS_RIBMGR_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "%s called\n", __func__);
  
  /*
   * debug configuration. 
   */
  configWriteRibmgrDebug (cmsh);

  /*
   * Static route configuration. 
   */
  RouteTableT * pStaticTable = NULL;
  RouteNodeT *pRNode = NULL;
  StaticIpv4T *pSi = NULL;
  Int8T strBuff[1024] = {};

  /* Lookup table.  */
  pStaticTable = vrfStaticTable (AFI_IP, SAFI_UNICAST, 0);
  if (! pStaticTable)
  {
    NNLOG(LOG_ERR, "Not exist static route table. \n");
    return -1; 
  }

  for (pRNode = nnRouteTop (pStaticTable); pRNode; pRNode = nnRouteNext (pRNode))
    for (pSi = pRNode->pInfo; pSi; pSi = pSi->next)
    {   
      Int32T idx = 0 ;
      memset (strBuff, 0, 1024);
      idx += sprintf (strBuff + idx, "ip route %s/%d", 
                     inet_ntoa (pRNode->p.u.prefix4), pRNode->p.prefixLen);

      switch (pSi->type)
      {   
        case NEXTHOP_TYPE_IPV4:
          idx += sprintf (strBuff + idx, " %s", inet_ntoa (pSi->gate.ipv4));
          break;
        case NEXTHOP_TYPE_IFNAME:
          idx += sprintf (strBuff + idx, " %s", pSi->gate.ifName);
          break;
        case NEXTHOP_TYPE_NULL0:
          idx += sprintf (strBuff + idx, " null0");
          break;
      }   

      if (pSi->distance != RIB_DISTANCE_DEFAULT_STATIC)
      {
        idx += sprintf (strBuff + idx, " %d", pSi->distance);
      }

      cmdPrint (cmsh, strBuff);
    }   

  /*
   * Router-id configuration.
   */ 
  if (pRibmgr->ridUserAssigned.prefix.s_addr)
  {
    memset (strBuff, 0, 1024);
    sprintf (strBuff, 
             "router-id %s", inet_ntoa(pRibmgr->ridUserAssigned.prefix));
    cmdPrint (cmsh, strBuff);
  }

  return CMD_IPC_OK;
}


/*
 * Description : When rib manager is started, this function should be called
 *               to initialize global data structure pointer.
 */
void
ribInitProcess()
{
  /* Allocation rib manager's global data structure */
  pRibmgr = NNMALLOC (MEM_GLOBAL, sizeof(RibmgrT));

  /* Command Init. */
  pRibmgr->pCmshGlobal = compCmdInit(IPC_RIB_MGR, ribWriteConfCB);

  /* Initialize RouterID list. */
  routerIdInit ();

  /* Initialize interface list. */
  pRibmgr->pIfList = ifInit ();

  /* Initialize rib table. */
  ribInit ();

  /* Initialize client list. */
  clientListInit ();

  /*  
   * Initialize Netlink
   *   & Scan Interface Informations
   *     - Build Interface List
   *     - Build Connected List
   *   & Scan Route Informations
   *     - Build System and Kernel Route
   */
  netlinkInit ();

  /* Assign shared memory pointer to global memory pointer. */
  (*gCompData) = (void *)pRibmgr;

  if (IS_RIBMGR_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "%s called\n", __func__);

  /*
   * Process Manager 가 Process 를 관리할 때 사용할 속성들을 담은 구조체로
   * 변경을 수행하지 않을 땐 값을 0 으로 사용
   */
  LcsAttributeT lcsAttribute = {0,};

  /* 초기화 작업 이후 Process Manager 에게 Register Message 전송 */
  lcsRegister(LCS_RIB_MANAGER, RIB_MANAGER, lcsAttribute);
}


/*
 * Description : 
 */
void
ribTermProcess ()
{
  if (IS_RIBMGR_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "%s called\n", __func__);

  ribSignalProcess (0);
}

/*
 * Description : 
 */
void
ribRestartProcess ()
{
  if (IS_RIBMGR_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "%s called\n", __func__);

  /* Assign global memory pointer to shared memory pointr. */
  pRibmgr = (RibmgrT *)(*gCompData);

  /* Assign Command related pointer. */
  compCmdUpdate(pRibmgr->pCmshGlobal, ribWriteConfCB);

  /* Assign interface list pointer. */
  ifUpdate (pRibmgr->pIfList);

  /* Assign RouterID compare function mapping. */
  routerIdUpdate();

  /* Assign netlink socket callback. */
  kernelUpdate ();
}


/*
 * Description : 
 */
void
ribHoldProcess ()
{
  if (IS_RIBMGR_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "%s called\n", __func__);
}


/*
 * Description : every one second
 */
void
ribTimerProcess (Int32T fd, Int16T event, void * arg)
{
  if (IS_RIBMGR_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "%s called\n", __func__);

  /* Call ribSignalProcess(). */
  ribSignalProcess (0);
}


/**
 * Description : Signal comes up, This Callback is called.
 *
 * @retval : none
 */
void 
ribSignalProcess(Int32T signalType)
{
  if (IS_RIBMGR_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "%s called\n", __func__);

  clientListFree ();
  ribClose(); /* Free routing table's memory */
  routerIdClose (); /* Free router id's memory */
  ifClose(); /* Free interface's memory */
  NNFREE (MEM_GLOBAL, pRibmgr); /* Free Global Ribmgr Memory */

  taskClose();
  nnLogClose();
  memClose();
}


/**
 * Description : Command manager channel
 *
 * @retval : none
 */
void 
ribCmIpcProcess(Int32T sockId, void *message, Uint32T size)
{
  if (IS_RIBMGR_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "%s called\n", __func__);

  compCmdIpcProcess(pRibmgr->pCmshGlobal, sockId, message, size);
}


/**
 * Description : Command shell channel
 *
 * @retval : none
 */
void
ribCmshIpcProcess (Int32T sockId, void *message, Uint32T size)
{
  if (IS_RIBMGR_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "%s called\n", __func__);

  compCmdIpcProcess(pRibmgr->pCmshGlobal, sockId, message, size);
}


/*
 * Define Table for IPC & Callback Functions 
 */
static const struct
{
  Int32T key;
  Int32T (* func) (nnBufferT *);
} ipcRibCallbackInfo[] =
{
  {IPC_NUM_RIB_MANAGER_START,               NULL                             },
  {IPC_RIB_CLIENT_INIT,                     ipcRibClientInit                 },
  {IPC_RIB_CLIENT_CLOSE,                    ipcRibClientClose                },
  {IPC_RIB_ROUTE_PRESERVE,                  ipcRibRoutePreserve              },
  {IPC_RIB_ROUTE_STALE_REMOVE,              ipcRibRouteStaleRemove           },
  {IPC_RIB_ROUTE_CLEAR,                     ipcRibRouteClear                 },
  {IPC_RIB_ROUTER_ID_SET,                   ipcRibRouterIdSet                },
  {IPC_RIB_IPV4_ROUTE_ADD,                  ipcRibIpv4RouteAdd               },
  {IPC_RIB_IPV4_ROUTE_DELETE,               ipcRibIpv4RouteDelete            },
  {IPC_RIB_IPV6_ROUTE_ADD,                  ipcRibIpv6RouteAdd               },
  {IPC_RIB_IPV6_ROUTE_DELETE,               ipcRibIpv6RouteDelete            },
  {IPC_RIB_IPV4_NEXTHOP_BEST_LOOKUP,        ipcRibIpv4NexthopBestLookup      },
  {IPC_RIB_IPV4_NEXTHOP_EXACT_LOOKUP,       ipcRibIpv4NexthopExactLookup     },
  {IPC_RIB_IPV6_NEXTHOP_BEST_LOOKUP,        ipcRibIpv6NexthopBestLookup      },
  {IPC_RIB_IPV6_NEXTHOP_EXACT_LOOKUP,       ipcRibIpv6NexthopExactLookup     },
  {IPC_RIB_REDISTRIBUTE_ADD,                ipcRibRedistributeAdd            },
  {IPC_RIB_REDISTRIBUTE_DELETE,             ipcRibRedistributeDelete         },
  {IPC_RIB_REDISTRIBUTE_DEFAULT_ADD,        ipcRibRedistributeDefaultAdd     },
  {IPC_RIB_REDISTRIBUTE_DEFAULT_DELETE,     ipcRibRedistributeDefaultDelete  },
  {IPC_RIB_REDISTRIBUTE_CLEAR,              ipcRibRedistributeClear          },
  {IPC_RIB_IPV4_NEXTHOP_BEST_LOOKUP_REG,    ipcRibIpv4NexthopBestLookupReg   },
  {IPC_RIB_IPV4_NEXTHOP_BEST_LOOKUP_DEREG,  ipcRibIpv4NexthopBestLookupDereg },
  {IPC_RIB_IPV4_NEXTHOP_EXACT_LOOKUP_REG,   ipcRibIpv4NexthopExactLookupReg  },
  {IPC_RIB_IPV4_NEXTHOP_EXACT_LOOKUP_DEREG, ipcRibIpv4NexthopExactLookupDereg},
  {IPC_RIB_IPV4_ROUTE_NOTIFICATION_ADD,     ipcRibIpv4RouteNotificationAdd   },
  {IPC_RIB_IPV4_ROUTE_NOTIFICATION_DELETE,  ipcRibIpv4RouteNotificationDelete},
  {IPC_RIB_IPV6_NEXTHOP_BEST_LOOKUP_REG,    ipcRibIpv6NexthopBestLookupReg   },
  {IPC_RIB_IPV6_NEXTHOP_BEST_LOOKUP_DEREG,  ipcRibIpv6NexthopBestLookupDereg },
  {IPC_RIB_IPV6_NEXTHOP_EXACT_LOOKUP_REG,   ipcRibIpv6NexthopExactLookupReg  },
  {IPC_RIB_IPV6_NEXTHOP_EXACT_LOOKUP_DEREG, ipcRibIpv6NexthopExactLookupDereg},
  {IPC_RIB_IPV6_ROUTE_NOTIFICATION_ADD,     ipcRibIpv6RouteNotificationAdd   },
  {IPC_RIB_IPV6_ROUTE_NOTIFICATION_DELETE,  ipcRibIpv6RouteNotificationDelete}
};


/*
 * Description : IPC 메시지를 수신하는 경우 호출될 콜백 함수임. 
 */
void
ribIpcProcess (Int32T msgId, void * data, Uint32T dataLen)
{
  if (IS_RIBMGR_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "%s : msgId = %d len=%d\n", __func__, msgId, dataLen);

  /*
   * Below is IPC message from processmgr.
   */
  switch (msgId)
  {
    case IPC_LCS_PM2C_SETROLE :
      {
      if (IS_RIBMGR_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "LCS :: IPC_LCS_PM2C_SETROLE \n");
      ipcLcsRibSetRole (data, dataLen);
      return;
      }
    case IPC_LCS_PM2C_TERMINATE :
      {
      if (IS_RIBMGR_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "LCS :: IPC_LCS_PM2C_TERMINATE \n");
      ipcLcsRibTerminate (data, dataLen);
      return;
      }
    case IPC_LCS_PM2C_HEALTHCHECK :
      {
      if (IS_RIBMGR_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "LCS :: IPC_LCS_PM2C_HEALTHCHECK \n");
      ipcLcsRibHealthcheck (data, dataLen);
      return;
      }
  }


  /*
   * Below is IPC message from other component.
   */

  /* Buffer Reset & Assign */
  nnBufferT msgBuff;
  nnBufferReset (&msgBuff);
  nnBufferAssign (&msgBuff, data, dataLen);

  /* Jump to Callback Function */
  Uint16T tableKey =  msgId - IPC_NUM_RIB_MANAGER_START;
  ipcRibCallbackInfo[tableKey].func (&msgBuff);
}


/* Description : EVENT 메시지를 수신하는 경우 호출될 콜백 함수임. */
void
ribEventProcess (Int32T msgId, void * data, Uint32T dataLen)
{
  if (IS_RIBMGR_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "%s : msgId = %d len=%d\n", __func__, msgId, dataLen);

  switch (msgId)
  {
    /* Process Manager Event. */
    case EVENT_LCS_COMPONENT_ERROR_OCCURRED :
      {
      if (IS_RIBMGR_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "LCS : EVENT_LCS_COMPONENT_ERROR_OCCURRED \n");
      ipcLcsRibEventComponentErrorOccured (data, dataLen);
      }
      break;
    case EVENT_LCS_COMPONENT_SERVICE_STATUS :
      {
      if (IS_RIBMGR_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "LCS : EVENT_LCS_COMPONENT_SERVICE_STATUS \n");
      ipcLcsRibEventComponentServiceStatus (data, dataLen);
      }
      break;
    default :
      NNLOG (LOG_ERR, "EVENT : Unknown event id = %d\n", msgId);
      break;
  }
}
