/*3456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*********************************.*********************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute.
*           All rights reserved. No part of this software shall be reproduced,
*           stored in a retrieval system, or transmitted by any means,
*           electronic, mechanical, photocopying, recording, or otherwise,
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief : This file include ipc, event and signal process functions. 
 *  - Block Name : Policy Manager
 *  - Process Name : cmmgr
 *  - Creator : PyungKoo Park
 *  - Initial Date : 2014/03/03
 */

/**
 * @file        :
 *
 * $Author: $
 * $Date: $
 * $Revision: $
 * $LastChangedBy: $
 */

#include <stdio.h>
#include "nnTypes.h"

#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"

#include "nosLib.h"
#include "cmMgrMain.h"
#include "lcsService.h"

void healthcheckTimerCallback(Int32T fd, Int16T event, void *arg);
void quiescingTimerCallback(Int32T fd, Int16T event, void *arg);
//component ipc process
void 
cmIpcProcess(Int32T msgId, void * data, Uint32T dataLen)
{
    NNLOG(LOG_DEBUG, "enter %s\n", __FUNCTION__);
    Int8T msg[4096] = {0,};

    memcpy(msg, data, dataLen);

    NNLOG(LOG_DEBUG, "%s RECV -> msgId : %d, dataLen : %d, msg : %s\n", MY_NAME, msgId, dataLen, msg);
    printf("%s RECV -> msgId : %d, dataLen : %d, msg : %s\n", MY_NAME, msgId, dataLen, msg);

    switch(msgId)
    {
        case IPC_LCS_PM2C_SETROLE:
        {
            LcsSetRoleMsgT lcsSetRoleMsg = {0,};
            memcpy(&lcsSetRoleMsg, msg, sizeof(LcsSetRoleMsgT));

            if (lcsSetRoleMsg.haState == LCS_HA_QUIESCING)
            {
                struct timeval setRoleTv;

                SET_TV(setRoleTv, FIVE_SEC);

                taskTimerSet((void *)quiescingTimerCallback, setRoleTv, 0, NULL);
            }
            lcsResponse(lcsSetRoleMsg.processType, lcsSetRoleMsg.invocationId);
        }
        break;
        case IPC_LCS_PM2C_HEALTHCHECK:
        {
            struct timeval healthTv = {0,};

            SET_TV(healthTv, THREE_SEC);

            fprintf(stdout, "%s RECV -> HEALTHCHECK Msg\n", MY_NAME);
            taskTimerSet((void *)healthcheckTimerCallback, healthTv, 0, NULL);
        }
        break;
        case IPC_STARTUP_CONFIG_REQUEST:
        {
            Int32T haState = LCS_HA_SIMPLEX;
            ipcSendAsync(PROCESS_MANAGER, IPC_STARTUP_CONFIG_RESPONSE,
                         &haState, sizeof(LcsHaStateT));

            fprintf(stdout, "Send Role : %d\n", haState);
        }
        break;
        case IPC_LCS_PM2C_TERMINATE:
        {
            struct timeval termTv = {0,};
            LcsTerminateMsgT lcsTerminateMsg = {0,};
            memcpy(&lcsTerminateMsg, msg, sizeof(LcsTerminateMsgT));

            NNLOG(LOG_DEBUG, "%s RECV -> TERMINATE Msg\n", MY_NAME);
            fprintf(stdout, "%s RECV -> TERMINATE Msg\n", MY_NAME);
            SET_TV(termTv, THREE_SEC);
            taskTimerSet((void *)closeTimerCallback, termTv, 0, NULL);
        }
        break;
        default:
            fprintf(stdout, "%s MsgID [%d]\n", MY_NAME, msgId);
        break;
    }
}

void 
healthcheckTimerCallback(Int32T fd, Int16T event, void *arg)
{
    Int32T ret = 0;

    fprintf(stdout, "I'm %s : %s\n", MY_NAME, __func__);

    ret = lcsResponse(MY_PTYPE, IPC_LCS_PM2C_HEALTHCHECK);
    fprintf(stdout, "[%s] Async Result : %d\n", __func__, ret);
}
void quiescingTimerCallback(Int32T fd, Int16T event, void *arg)
{
    LcsRoleQuiescingCompleteMsgT completeMsg = {0,};
    Int32T ret = 0;

    fprintf(stdout, "I'm %s : %s\n", MY_NAME, __func__);

    completeMsg.processType = MY_PTYPE;
    completeMsg.invocationId = IPC_LCS_C2PM_ROLE_QUIESCING_COMPLETE;
    completeMsg.error = LCS_OK;

    ret = ipcSendAsync(PROCESS_MANAGER, IPC_LCS_C2PM_ROLE_QUIESCING_COMPLETE,
                        &completeMsg, sizeof(completeMsg));

    fprintf(stdout, "[%s] Async Result : %d\n", __func__, ret);
}
//component event process
void 
cmEventProcess(Int32T msgId, void * data, Uint32T dataLen)
{
    NNLOG(LOG_DEBUG, "enter %s\n", __FUNCTION__);
}


