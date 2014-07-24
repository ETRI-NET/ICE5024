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
 *  - Process Name : polmgr
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
#include <stdlib.h>
#include <string.h>
#include "nnTypes.h"

#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"

#include "nosLib.h"
#include "polMgrMain.h"


void quiescingTimerCallback(Int32T fd, Int16T event, void *arg);
void healthcheckTimerCallback(Int32T fd, Int16T event, void *arg);
void finalizeTimerCallback(Int32T fd, Int16T event, void *arg);

//component ipc process
void polIpcProcess(Int32T msgId, void * data, Uint32T dataLen)
{
    Int32T ret;

    Int8T msg[4096] = {0,};

    memcpy(msg, data, dataLen);

    NNLOG(LOG_DEBUG, "enter %s\n", __FUNCTION__);

    NNLOG(LOG_DEBUG, "%s RECV -> msgId : %d, dataLen : %d, msg : %s\n", MY_NAME, msgId, dataLen, msg);
    printf("%s RECV -> msgId : %d, dataLen : %d, msg : %s\n", MY_NAME, msgId, dataLen, msg);

    switch(msgId)
    {
        case IPC_LCS_PM2C_SETROLE:
        {
            LcsSetRoleMsgT lcsSetRoleMsg = {0,};
            memcpy(&lcsSetRoleMsg, msg, sizeof(LcsSetRoleMsgT));

            // Role 이 LCS_HA_QUIESCING 인 경우 Service 중지
            if (lcsSetRoleMsg.haState == LCS_HA_QUIESCING)
            {
                struct timeval setRoleTv = {0,};

                // Service 중지
                // Process 의 중지가 아닌 Service 의 중지를 수행한다
                /* Service 중지 작업 수행 */

                SET_TV(setRoleTv, THREE_SEC);

                taskTimerSet((void *)quiescingTimerCallback, setRoleTv, 0, NULL);
            }

            // Process Manager 에게 응답 수행
            ret = lcsResponse(lcsSetRoleMsg.processType, lcsSetRoleMsg.invocationId);

            if (ret == LCS_OK)
            {
                // 성공
            }
            else
            {
                // 문제 발생
            } 
        }
        break;
        case IPC_LCS_PM2C_TERMINATE:
        {
            struct timeval termTv = {0,};

            NNLOG(LOG_DEBUG, "IPC_LCS_PM2C_TERMINATE\n");

            LcsTerminateMsgT lcsTerminateMsg = {0,};
            memcpy(&lcsTerminateMsg, msg, sizeof(LcsTerminateMsgT));

            ret = lcsResponse(lcsTerminateMsg.processType,
                                lcsTerminateMsg.invocationId);

            NNLOG(LOG_DEBUG, "\n\n%s RECV -> TERMINATE Msg[%d]\n", MY_NAME, ret);
            fprintf(stdout, "%s RECV -> TERMINATE Msg[%d]\n\n", MY_NAME, ret);

            if (ret == LCS_OK)
            {
                // 성공
            }
            else
            {
                // 실패
            }

            // 자신의 종료 절차를 수행
            // 절차 수행 후 반드시
            // ret = lcsResponse(MY_PTYPE, IPC_LCS_PM2C_TERMINATE);
            // 를 사용하여 Process Manager 에게 응답을 수행한다.
            // (polTermProcess() 확인)

            SET_TV(termTv, THREE_SEC);
            taskTimerSet((void *)closeTimerCallback, termTv, 0, NULL);
        }
        break;
        case IPC_LCS_PM2C_HEALTHCHECK :
        {
            struct timeval healthTv = {0,};

            SET_TV(healthTv, THREE_SEC); 

            NNLOG(LOG_DEBUG, "IPC_LCS_PM2C_HEALTHCHECK\n");

            // Self Diagnosis

            // if) Component Problem Conditions
            // (1) lcsErrorReport();
            // (2) ret =
            //      lcsErrorReport(MY_PTYPE, LCS_ERR_BAD_OPERATION, LCS_COMPONENT_RESTART);

            // Must Response(Both Problem and Normal Condition)
            taskTimerSet((void *)healthcheckTimerCallback, healthTv, 0, NULL);
        }
        break;
    }

    return;
}

//component event process
// Event 를 사용하기 위해
// compConfig.h 에 NosEventRegister 구조체에 등록을 해야함
void polEventProcess(Int32T msgId, void * data, Uint32T dataLen)
{
    NNLOG(LOG_DEBUG, "enter %s\n", __FUNCTION__);

    switch(msgId)
    {
        // Process 가 기동했을 때 전달되는 Event 입니다.
        // Process 의 기동여부(서비스 가능인지)를 확인할 수 있습니다.
        case EVENT_LCS_COMPONENT_SERVICE_STATUS:
        {
            memcpy(&statusEvent, data, sizeof(LcsServiceStatusEventT));

            fprintf(stdout, "%s \
                    -> Receive SERVICE_STATUS[%llu]\n",
                    MY_NAME, statusEvent.serviceStatus);
        }
        break;
        // Process 가 종료 됐을 때(서비스가 불가능할 때) 전달되는 Event
        case EVENT_LCS_COMPONENT_ERROR_OCCURRED:
        {
            LcsErrorOccurredEventT errorOccurredEvent = {0,};

            memcpy(&errorOccurredEvent, data, sizeof(errorOccurredEvent));

            // Lib Manager 가 종료할 때 자신도 종료를 수행할 경우
            if (errorOccurredEvent.processType == LCS_LIB_MANAGER)
            {
                struct timeval eventErrorTv = {0,};

                SET_TV(eventErrorTv, ONE_SEC);

                fprintf(stdout, "LIB MANAGER IS ERROR, DO FINALIZE...\n");

                // 자신이 동작하는데 필요한 프로토콜이 종료가 됐을 경우
                // 자신도 재시작을 하기위해 lcsFinalize() 함수를 사용
                taskTimerSet((void *)finalizeTimerCallback, eventErrorTv, 0, NULL);
            }
        }
        break;
    }

    return;
}

/* QUISCING 시 호출되는 Timer 콜백함수 */
void quiescingTimerCallback(Int32T fd, Int16T event, void *arg)
{
    LcsRoleQuiescingCompleteMsgT completeMsg = {0,};
    Int32T ret = 0;

    completeMsg.processType = MY_PTYPE;
    completeMsg.invocationId = IPC_LCS_C2PM_ROLE_QUIESCING_COMPLETE;
    completeMsg.error = LCS_OK;

    ret = ipcSendAsync(PROCESS_MANAGER, IPC_LCS_C2PM_ROLE_QUIESCING_COMPLETE,
                        &completeMsg, sizeof(completeMsg));

    if (ret == SUCCESS)
    {
        // 성공
    }
    else
    {
        // 문제 발생
    }
}

/* Healthcheck 시 호출되는 Timer 콜백함수 */
void healthcheckTimerCallback(Int32T fd, Int16T event, void *arg)
{
    Int32T ret = 0;

    ret = lcsResponse(MY_PTYPE, IPC_LCS_PM2C_HEALTHCHECK);

    if (ret == LCS_OK)
    {
        // 성공
    }
    else
    {
        // 문제 발생
    }
}

/* 타 Process 가 종료 시 Finalize 수행하는 Timer 콜백함수 */
void finalizeTimerCallback(Int32T fd, Int16T event, void *arg)
{
    Int32T ret = 0;

    ret = lcsFinalize(MY_PTYPE);

    if (ret == LCS_OK)
    {
        // 성공
    }
    else
    {
        // 문제 발생
    }
}
