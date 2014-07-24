/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : RIP Protocol
 * 
 * - Block Name : RIP Protocol
 * - Process Name : ripd
 * - Creator : Suncheul Kim
 * - Initial Date : 2014/04/17/
 */

/**
 * @file        : ripInit.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include "nnTypes.h"
#include "nnStr.h"
#include "nnVector.h"
#include "nnList.h"
#include "nnPrefix.h"
#include "nnBuffer.h"
#include "nosLib.h"

#include "lcsService.h"

#include "nnKeychain.h"

#include "nnCmdDefines.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCompProcess.h"

#include "ripInit.h"
#include "ripIpc.h"
#include "ripd.h"
#include "ripInterface.h"
#include "ripRibmgr.h"
#include "ripDebug.h"
#include "nnRipDefines.h"


/*
 * Definitions for Global Data Structure
 */

/*
 * External Definitions for Global Data Structure
 */
extern void ** gCompData;


/*
 * External Definitions are here
 */


/*
 * Description : When we display running config, this function will be called.
 */
Int32T
WriteConfCB(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv)
{
  /* Check global rip pointer. */
  if (!pRip)
  {
    return CMD_IPC_OK;
  }

  /* Rib debug's configure. */
  configWriteRipDebug (cmsh);

  /* Interface Node's configure. */
  configWriteRipInterface (cmsh);

  /* Rip node's configure. */
  configWriteRip (cmsh);

  /* Rip node's Keychain. */
  configWriteKeychain (cmsh);

  return CMD_IPC_OK;
}


/*
 * Description : When rip protocol is started, this function should be called
 *               to initialize global data structure pointer.
 */
void
ripInitProcess()
{
  /*
   * Process Manager 가 Process 를 관리할 때 사용할 속성들을 담은 구조체로
   * 변경을 수행하지 않을 땐 값을 0 으로 사용
   */
  LcsAttributeT lcsAttribute = {0,};

  /* Rip 프로토콜 초기화 함수. */
  ripInit();

  /* Assign shared memory pointer to global memory pointer. */
  (*gCompData) = (void *)pRip;

  if (IS_RIP_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "%s\n", __func__);

  /* 초기화 작업 이후 Process Manager 에게 Register Message 전송 */
  lcsRegister(LCS_RIP, RIP, lcsAttribute);
  
}


/*
 * Description : 
 */
void
ripTermProcess ()
{
  /* Check rip global pointer. */
  if (!pRip)
    return;

  if (IS_RIP_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "%s\n", __func__);

  ripSignalProcess (0);
}

/*
 * Description : 
 */
void
ripRestartProcess ()
{
  /* Assign global memory pointer to shared memory pointr. */
  pRip = (RipT *)(*gCompData);

  /* Check rip global pointer. */
  if (!pRip)
    return;

  if (IS_RIP_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "%s\n", __func__);

  /* Assign each data pointer. */
  ripVersionUpdate();
}


/*
 * Description : 
 */
void
ripHoldProcess ()
{
  /* Check rip global pointer. */
  if (!pRip)
    return;

  if (IS_RIP_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "%s\n", __func__);
}


/*
 * Description : every one second
 */
Uint32T bOnlyOnce=0;
void
ripTimerProcess (Int32T fd, Int16T event, void * arg)
{
  /* Check rip global pointer. */
  if (!pRip)
    return;

  if (IS_RIP_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "%s\n", __func__);
}


/**
 * Description : Signal comes up, This Callback is called.
 *
 * @retval : none
 */
void 
ripSignalProcess(Int32T signalType)
{
  if (IS_RIP_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "%s\n", __func__);

  /* Check rip global pointer. */
  if (!pRip)
    return;

//  ripRibmgrClose (); /* Close request to ribmgr. */

  /* Component Command Close */
  compCmdFree(pRip->pCmshGlobal);

  /* Close rip related data structure. */
  ripClean();

  /* Close additional data structure. */
  ifClose(); /* Free interface's memory */
  NNFREE (MEM_GLOBAL, pRip); /* Free Global Ribmgr Memory */

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
ripCmIpcProcess(Int32T sockId, void *message, Uint32T size)
{
  /* Check rip global pointer. */
  if (!pRip)
    return;

  if (IS_RIP_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "%s\n", __func__);

  compCmdIpcProcess(pRip->pCmshGlobal, sockId, message, size);
}


/**
 * Description : Command shell channel
 *
 * @retval : none
 */
void
ripCmshIpcProcess (Int32T sockId, void *message, Uint32T size)
{
  /* Check rip global pointer. */
  if (!pRip)
    return;

  if (IS_RIP_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "%s\n", __func__);

  compCmdIpcProcess(pRip->pCmshGlobal, sockId, message, size);
}


/*
 * Description : IPC 메시지를 수신하는 경우 호출될 콜백 함수임. 
 */
void
ripIpcProcess (Int32T msgId, void * data, Uint32T dataLen)
{
  /* Check rip global pointer. */
  if (!pRip)
    return;

  if (IS_RIP_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "%s, msgId=%d, msgLen=%d\n", __func__, msgId, dataLen);

  /* Buffer Reset & Assign. */
  nnBufferT msgBuff;
  nnBufferReset (&msgBuff);
  nnBufferAssign (&msgBuff, data, dataLen);

  switch(msgId)
  {
    /* ProcessManager Interface IPC message. */
    case IPC_LCS_PM2C_SETROLE :
      {
      if (IS_RIP_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "LCS :: IPC_LCS_PM2C_SETROLE \n");
      ripLcsSetRole (data, dataLen);
      }
      break;
    case IPC_LCS_PM2C_TERMINATE :
      {
      if (IS_RIP_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "LCS :: IPC_LCS_PM2C_TERMINATE \n");
      ripLcsTerminate (data, dataLen);
      }
      break;
    case IPC_LCS_PM2C_HEALTHCHECK :
      {
      if (IS_RIP_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "LCS :: IPC_LCS_PM2C_HEALTHCHECK \n");
      ripLcsHealthcheck (data, dataLen);
      }
      break;

    /* Ribmgr Interface IPC message. */
    case IPC_RIB_ROUTER_ID_SET :
      {
      if (IS_RIP_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "ROUTER-ID  :: SET, but do nothing in RIP \n");
      }
    case IPC_RIB_IPV4_ROUTE_ADD :
      {
      if (IS_RIP_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "REDISTRIBUTE  :: IPC_RIB_IPV4_ROUTE_ADD \n");
      ripRibmgrReadIpv4 (msgId, &msgBuff, dataLen);
      }
      break;

    case IPC_RIB_IPV4_ROUTE_DELETE :
      {
      if (IS_RIP_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "REDISTRIBUTE  :: IPC_RIB_IPV4_ROUTE_DELETE \n");
      ripRibmgrReadIpv4 (msgId, &msgBuff, dataLen);
      }
      break;

    case IPC_INTERFACE_ADD :
      {
      if (IS_RIP_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "INTERFACE  :: IPC_INTERFACE_ADD \n");
      ripInterfaceAdd(&msgBuff);
      }
      break;

    case IPC_INTERFACE_UP :
      {
      if (IS_RIP_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "INTERFACE :: IPC_INTERFACE_UP \n");
      ripInterfaceUp(&msgBuff);
      }
      break;

    case IPC_INTERFACE_ADDRESS_ADD :
      {
      if (IS_RIP_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "INTERFACE  :: IPC_INTERFACE_ADDRESS_ADD \n");
      ripInterfaceAddressAdd(&msgBuff);
      }
      break;


    default :
      break;
  }
}


/* Description : EVENT 메시지를 수신하는 경우 호출될 콜백 함수임. */
void
ripEventProcess (Int32T msgId, void * data, Uint32T dataLen)
{
  /* Check rip global pointer. */
  if (!pRip)
    return;

  if (IS_RIP_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "%s, msgId=%d, msgLen=%d\n", __func__, msgId, dataLen);

  /* Buffer Reset & Assign */
  nnBufferT msgBuff;
  nnBufferReset (&msgBuff);
  nnBufferAssign (&msgBuff, data, dataLen);

  switch (msgId)
  {
    /* Process Manager Event. */
    case EVENT_LCS_COMPONENT_ERROR_OCCURRED :
      {
      if (IS_RIP_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "LCS : EVENT_LCS_COMPONENT_ERROR_OCCURRED \n");
      ripLcsEventComponentErrorOccured (data, dataLen);
      }
      break;
    case EVENT_LCS_COMPONENT_SERVICE_STATUS :
      {
      if (IS_RIP_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "LCS : EVENT_LCS_COMPONENT_SERVICE_STATUS \n");
      ripLcsEventComponentServiceStatus (data, dataLen);
      }
      break;

    /* Rib Manager Event. */
    case EVENT_INTERFACE_ADD :
      {
      if (IS_RIP_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "EVENT_INTERFACE_ADD\n");
      ripInterfaceAdd(&msgBuff);
      }
      break;

    case EVENT_INTERFACE_DELETE :
      {
      if (IS_RIP_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "EVENT_INTERFACE_DELETE\n");
      ripInterfaceDelete(&msgBuff);
      }
      break;

    case EVENT_INTERFACE_ADDRESS_ADD :
      {
      if (IS_RIP_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "EVENT_INTERFACE_ADDRESS_ADD\n");
      ripInterfaceAddressAdd(&msgBuff);
      }
      break;

    case EVENT_INTERFACE_ADDRESS_DELETE :
      {
      if (IS_RIP_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "EVENT_INTERFACE_ADDRESS_DELETE\n");
      ripInterfaceAddressDelete(&msgBuff);
      }
      break;

    case EVENT_INTERFACE_UP :
      {
      if (IS_RIP_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "EVENT_INTERFACE_UP\n");
      ripInterfaceUp(&msgBuff);
      }
      break;

    case EVENT_INTERFACE_DOWN :
      {
      if (IS_RIP_DEBUG_EVENT)
        NNLOG (LOG_DEBUG, "EVENT_INTERFACE_DOWN\n");
      ripInterfaceDown(&msgBuff);
      }
      break;

    default :
      NNLOG (LOG_ERR, "EVENT_UNKNOWN....\n");
      break;
  }
}
