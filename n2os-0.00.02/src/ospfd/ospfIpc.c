/*
 * ospfIpc.c
 *
 *  Created on: 2014. 7. 8.
 *      Author: root
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

#include "ospfIpc.h"
#include "ospfInit.h"

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
ospfLcsSetRole (void * msg, Uint32T msgLen)
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
ospfLcsTerminate (void * msg, Uint32T msgLen)
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

  /* Call ospfTermProcess(). */
  ospfTermProcess();
}

/*
 * Lcs component's health check requested function.
 */
void
ospfLcsHealthcheck (void * msg, Uint32T msgLen)
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

  /* ospf health check 수행. To do. */
  /*
   * If ospf has some problem. send
   * lcsErrorReport(LCS_POLICY_MANAGER, LCS_ERR_BAD_OPERATION,
   *                LCS_COMPONENT_RESTART);
   */
}


/*
 * Lcs : component's error was occurred, this event will be received.
 */
void
ospfLcsEventComponentErrorOccured (void * msg, Uint32T msgLen)
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
ospfLcsEventComponentServiceStatus (void * msg, Uint32T msgLen)
{
  NNLOG (LOG_DEBUG, "LCS : %s called.\n", __func__);

  LcsServiceStatusEventT serviceStatusEvent = {0,};

  /* Message copy. */
  memcpy(&serviceStatusEvent, msg, sizeof(LcsServiceStatusEventT));

  /* Log component's service. */
  NNLOG (LOG_DEBUG, "LCS : service status = %d\n", serviceStatusEvent.serviceStatus);
}

