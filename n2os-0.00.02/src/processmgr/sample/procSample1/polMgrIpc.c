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
#include "lcsService.h"

LcsHaStateT gHaState = 0; /* Role 값을 담을 변수 */
LcsServiceStatusEventT gStatusEvent = {0,}; /* Process 기동 정보를 담을 변수 */

//component ipc process
void polIpcProcess(Int32T msgId, void * data, Uint32T dataLen)
{
    NNLOG(LOG_DEBUG, "%s === IPC S === IPC S === IPC S ===\n", MY_NAME);
    fprintf(stdout, "%s === IPC S === IPC S === IPC S ===\n", MY_NAME);

    NNLOG(LOG_DEBUG, "%s RECV -> msgId : %d, dataLen : %d\n", MY_NAME, msgId, dataLen);
    fprintf(stdout, "%s RECV -> msgId : %d, dataLen : %d\n", MY_NAME, msgId, dataLen);

    switch(msgId)
    {
        /* Role 설정에 대한 Message */
        case IPC_LCS_PM2C_SETROLE:
        {
            NNLOG(LOG_DEBUG, "%s === SETROLE S === SETROLE S ===\n", MY_NAME);
            fprintf(stdout, "%s === SETROLE S === SETROLE S ===\n", MY_NAME);

            LcsSetRoleMsgT lcsSetRole = {0,};
            memcpy(&lcsSetRole, data, sizeof(LcsSetRoleMsgT));

            NNLOG(LOG_DEBUG, "%s RECV -> SETROLE Msg\n", MY_NAME);
            fprintf(stdout, "%s RECV -> SETROLE Msg\n", MY_NAME);

            /* Role 값 저장 */
            gHaState = lcsSetRole.haState;

            /* Process Manager 에게 Response Message 를 보냄 */
            lcsResponse(lcsSetRole.processType, lcsSetRole.invocationId);

            /* Role 이 LCS_HA_QUIESCING 인 경우 Service 중지 */
            if (lcsSetRole.haState == LCS_HA_QUIESCING)
            {
                NNLOG(LOG_DEBUG, "%s === QUIESCING S === QUIESCING S ===\n", MY_NAME);
                fprintf(stdout, "%s === QUIESCING S === QUIESCING S ===\n", MY_NAME);
                /* Service 중지 수행*/

                lcsRoleQuiescingComplete(MY_PTYPE,
                                        IPC_LCS_C2PM_ROLE_QUIESCING_COMPLETE,
                                        LCS_OK);

                NNLOG(LOG_DEBUG, "%s === QUIESCING E === QUIESCING E ===\n", MY_NAME);
                fprintf(stdout, "%s === QUIESCING E === QUIESCING E ===\n", MY_NAME);
            }

            NNLOG(LOG_DEBUG, "%s === SETROLE E === SETROLE E ===\n", MY_NAME);
            fprintf(stdout, "%s === SETROLE E === SETROLE E ===\n", MY_NAME);
        }
            break;
         /* Process 에게 종료를 수행하라는 메시지 */
        case IPC_LCS_PM2C_TERMINATE:
        {
            NNLOG(LOG_DEBUG, "%s === TEMINATE S === TERMINATE S ===\n", MY_NAME);
            fprintf(stdout, "%s === TEMINATE S === TERMINATE S ===\n", MY_NAME);

            LcsTerminateMsgT lcsTerminate = {0,};
            memcpy(&lcsTerminate, data, sizeof(LcsTerminateMsgT));

            /* Process Manager 에게 Response Message 를 보냄 */
            lcsResponse(lcsTerminate.processType, lcsTerminate.invocationId);

            NNLOG(LOG_DEBUG, "%s RECV -> TERMINATE Msg\n", MY_NAME);
            fprintf(stdout, "%s RECV -> TERMINATE Msg\n", MY_NAME);

            /* 자신의 종료 절차를 수행 */
            polTermProcess();

            NNLOG(LOG_DEBUG, "%s === TEMINATE E === TERMINATE E ===\n", MY_NAME);
            fprintf(stdout, "%s === TEMINATE E === TERMINATE E ===\n", MY_NAME);
        }
            break;
        // Health Check 시 사용되는 메시지
        case IPC_LCS_PM2C_HEALTHCHECK:
        {
            NNLOG(LOG_DEBUG, "%s === HEALTHCHECK S === HEALTHCHECK S ===\n", MY_NAME);
            fprintf(stdout, "%s === HEALTHCHECK S === HEALTHCHECK S ===\n", MY_NAME);

            NNLOG(LOG_DEBUG, "%s RECV -> HEALTHCHECK Msg\n", MY_NAME);
            fprintf(stdout, "%s RECV -> HEALTHCHECK Msg\n", MY_NAME);

            LcsHealthcheckRequestMsgT healthcheck = {0,};
            memcpy(&healthcheck, data, sizeof(LcsHealthcheckRequestMsgT));

            /* Process Manager 에게 Response Message 를 보냄 */
            lcsResponse(healthcheck.processType, healthcheck.invocationId);

#if 0
            /* 문제 있을 경우 lcsErrorReport() 수행 */
             ret = lcsErrorReport(MY_PTYPE, LCS_ERR_BAD_OPERATION, LCS_COMPONENT_RESTART);
#endif

            NNLOG(LOG_DEBUG, "%s === HEALTHCHECK E === HEALTHCHECK E ===\n", MY_NAME);
            fprintf(stdout, "%s === HEALTHCHECK E === HEALTHCHECK E ===\n", MY_NAME);
        }
            break;
    }

    NNLOG(LOG_DEBUG, "%s === IPC E === IPC E === IPC E ===\n", MY_NAME);
    fprintf(stdout, "%s === IPC E === IPC E === IPC E ===\n", MY_NAME);

    return;
}

//component event process
void polEventProcess(Int32T msgId, void * data, Uint32T dataLen)
{
    NNLOG(LOG_DEBUG, "%s === EVENT S === EVENT S === EVENT S ===\n", MY_NAME);
    fprintf(stdout, "%s === EVENT S === EVENT S === EVENT S ===\n", MY_NAME);

    NNLOG(LOG_DEBUG, "%s RECV -> eventId : %d, dataLen : %d\n", MY_NAME, msgId, dataLen);
    printf("%s RECV -> eventId : %d, dataLen : %d\n", MY_NAME, msgId, dataLen);

    switch(msgId)
    {
        /*
         * Component 가 Process Manager 에게 Register 를
         * 수행한 후 전달되는 Event 로
         * Component 의 서비스 가능여부를 확인할 수 있음
         */
        case EVENT_LCS_COMPONENT_SERVICE_STATUS:
        {
            NNLOG(LOG_DEBUG, "%s === EVENT SERVICE S === EVENT SERVICE S ===\n", MY_NAME);
            fprintf(stdout, "%s === EVENT SERVICE S === EVENT SERVICE S ===\n", MY_NAME);

            memcpy(&gStatusEvent, data, sizeof(LcsServiceStatusEventT));

            fprintf(stdout, "%s \
                    -> Receive SERVICE_STATUS[%llu]\n",
                    MY_NAME, gStatusEvent.serviceStatus);

            NNLOG(LOG_DEBUG, "%s === EVENT SERVICE E === EVENT SERVICE E ===\n", MY_NAME);
            fprintf(stdout, "%s === EVENT SERVICE E === EVENT SERVICE E ===\n", MY_NAME);
        }
            break;
        /* Process 의 서비스 불가능할 때 전달되는 Event */
        case EVENT_LCS_COMPONENT_ERROR_OCCURRED:
        {
            NNLOG(LOG_DEBUG, "%s === EVENT ERROR S === EVENT ERROR S ===\n", MY_NAME);
            fprintf(stdout, "%s === EVENT ERROR S === EVENT ERROR S ===\n", MY_NAME);

            LcsErrorOccurredEventT errorOccurredEvent = {0,};

            memcpy(&errorOccurredEvent, data, sizeof(errorOccurredEvent));

            fprintf(stdout, "%s \
                    -> Receive ERROR_PROCESS_TYPE[%d]\n",
                    MY_NAME, errorOccurredEvent.processType);

            if (errorOccurredEvent.processType == LCS_LACP)
            {
                NNLOG(LOG_DEBUG, "=== FINALIZE S === FINALIZE S ===\n");
                fprintf(stdout, "=== FINALIZE S === FINALIZE S ===\n");

                fprintf(stdout, "LIB MANAGER IS ERROR, DO FINALIZE...\n");

                /*
                 * 자신이 동작하는데 필요한 프로토콜이 종료가 됐을 경우
                 * 자신도 재시작을 하기위해 lcsFinalize() 함수를 사용
                 */
                lcsFinalize(MY_PTYPE);

                NNLOG(LOG_DEBUG, "=== FINALIZE E === FINALIZE E ===\n");
                fprintf(stdout, "=== FINALIZE E === FINALIZE E ===\n");
            }

            NNLOG(LOG_DEBUG, "%s === EVENT ERROR E === EVENT ERROR E ===\n", MY_NAME);
            fprintf(stdout, "%s === EVENT ERROR E === EVENT ERROR E ===\n", MY_NAME);
        }
            break;
    }

    NNLOG(LOG_DEBUG, "%s === EVENT E === EVENT E === EVENT E ===\n", MY_NAME);
    fprintf(stdout, "%s === EVENT E === EVENT E === EVENT E ===\n", MY_NAME);

    return;
}
