/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : RIP Protocol의 IPC 기능을 제어하는 화일
 *
 * - Block Name : RIP Protocol
 * - Process Name : ripd
 * - Creator : Geontae Park
 * - Initial Date : 2014/02/20
 */

/**
 * @file        : ripIpc.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include "nnRibDefines.h"
#include "nnRipDefines.h"
#include "taskManager.h"
#include "nnStr.h"
#include "nnPrefix.h"
#include "nnTable.h"
#include "nnUtility.h"
#include "nosLib.h"

#include "lcsService.h"

#include "ripd.h"
#include "ripInterface.h"
#include "ripUtil.h"
#include "ripPeer.h"
#include "ripInit.h"

/*
 * External Definitions are here
 */

/*
 * Lcs ipc message proc functions.
 */

/*
 * Lcs component's role assigned function.
 */
void
ripLcsSetRole (void * msg, Uint32T msgLen)
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
  if (ret != LCS_OK)
  {
    NNLOG (LOG_DEBUG, "LCS : lcsResponse Failure \n");
  }
  
}

/*
 * Lcs component's terminate requested function.
 */
void
ripLcsTerminate (void * msg, Uint32T msgLen)
{
  Int32T ret = 0;

  NNLOG (LOG_DEBUG, "LCS : %s called.\n", __func__);
  
  LcsTerminateMsgT lcsTerminateMsg = {0,};

  /* Message copy. */
  memcpy(&lcsTerminateMsg, msg, sizeof(LcsTerminateMsgT));

  /* Process Manager 에게 응답 수행. */
  ret = lcsResponse(lcsTerminateMsg.processType,
                    lcsTerminateMsg.invocationId); 
  if (ret != LCS_OK)
  {
    NNLOG (LOG_DEBUG, "LCS : lcsResponse Failure \n");
  }

  /* Call ripTermProcess(). */
  ripTermProcess();
}

/*
 * Lcs component's health check requested function.
 */
void
ripLcsHealthcheck (void * msg, Uint32T msgLen)
{
  Int32T ret = 0;

  LcsHealthcheckRequestMsgT healthcheck = {0,};

  /* Message copy. */
  memcpy(&healthcheck, msg, sizeof(LcsHealthcheckRequestMsgT));

  /* Process Manager 에게 Response Message 전송 */
  ret = lcsResponse(healthcheck.processType, healthcheck.invocationId);
  if (ret != LCS_OK)
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
ripLcsEventComponentErrorOccured (void * msg, Uint32T msgLen)
{
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
ripLcsEventComponentServiceStatus (void * msg, Uint32T msgLen)
{
  LcsServiceStatusEventT serviceStatusEvent = {0,};

  /* Message copy. */
  memcpy(&serviceStatusEvent, msg, sizeof(LcsServiceStatusEventT));

  /* Log component's service. */
  NNLOG (LOG_DEBUG, "LCS : service status = %d\n", serviceStatusEvent.serviceStatus);
}





void
procRipRouterRip(void * data, Uint32T dataLen)
{
  Uint32T ret = 0;
  Uint8T type = 0;
  nnBufferT msgBuff;

  /* Assign buffer. */
  nnBufferReset(&msgBuff);
  nnBufferAssign (&msgBuff, data, dataLen);

  /* Read buffer. */
  type = nnBufferGetInt8T(&msgBuff);

  /* Set function. */
  if (type == RIP_CLI_COMMAND_TYPE_SET)
  {
    /* If rip is not enabled before. */
    if (! pRip) 
    {    
      ret = ripCreate ();
      if (ret < 0) 
      {    
        NNLOG (LOG_ERR, "Can't create RIP !!!\n");
        return;
      }    
    }
  }
  else if (type == RIP_CLI_COMMAND_TYPE_UNSET)
  {
    if(pRip)
      ripClean();
  }

  return;
}

void ripEventSubscribe()
{
  eventSubscribe (EVENT_INTERFACE_ADD, EVENT_PRI_MIDDLE);
  eventSubscribe (EVENT_INTERFACE_DELETE, EVENT_PRI_MIDDLE);
  eventSubscribe (EVENT_INTERFACE_UP, EVENT_PRI_MIDDLE);
  eventSubscribe (EVENT_INTERFACE_DOWN, EVENT_PRI_MIDDLE);
  eventSubscribe (EVENT_INTERFACE_ADDRESS_ADD, EVENT_PRI_MIDDLE);
  eventSubscribe (EVENT_INTERFACE_ADDRESS_DELETE, EVENT_PRI_MIDDLE);
}


