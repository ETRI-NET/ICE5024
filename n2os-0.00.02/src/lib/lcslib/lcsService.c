/*******************************************************************************
 *            Electronics and Telecommunications Research Institute
 * Copyright: 2013 Electronics and Telecommunications Research Institute.
 *            All rights reserved.
 *            No part of this software shall be reproduced, stored in a
 *            retrieval system, or transmitted by any means, electronic,
 *            mechanical, photocopying, recording, or otherwise, without
 *            written permission from ETRI.
*******************************************************************************/

/**
 * @brief : Process Manager 가 지원하는 API 를 포함
 *  - Block Name : Process Manager
 *  - Process Name : procmgr
 *  - Creator : Changwoo Lee
 *  - Initial Date : 2014/4/15
 */

/**
* @file : processmgrApi.c
*
* $Id:
* $Author:
* $Date:
* $Revision:
* $LastChangedBy:
**/

/*******************************************************************************
 *                               INCLUDE FILES
 ******************************************************************************/
#include "nnDefines.h"

#include "lcsService.h"

/*******************************************************************************
 *                              GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 *                         GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/

/*******************************************************************************
 *                         LOCAL CONSTANTS/LITERALS/TYPES
 ******************************************************************************/

/*******************************************************************************
*                               LOCAL VARIABLES
*******************************************************************************/

/*******************************************************************************
 *                                   CODE
 ******************************************************************************/

/*
 * Process 초기화 후 Process Manager 에 등록하기 위한 함수
 */
LcsErrorT lcsRegister(LcsProcessTypeT process, Int32T ipc,
                    LcsAttributeT lcsAttribute)
{
    LcsRegisterMsgT lcsRegisterMsg = {0,};
    Int32T ret = 0;

    lcsRegisterMsg.processType = process;
    lcsRegisterMsg.ipcType = ipc;

    memcpy(&lcsRegisterMsg.attribute, &lcsAttribute, sizeof(LcsAttributeT));

    ret = ipcSendAsync(PROCESS_MANAGER, IPC_LCS_C2PM_REGISTER,
                       &lcsRegisterMsg, sizeof(LcsRegisterMsgT));

    fprintf(stdout, "[%s] Async Result : %d\n", __func__, ret);

    if (ret == SUCCESS)
    {
        return LCS_OK;
    }
    else
    {
        return LCS_ERR_LIBRARY;
    }
}

/*
 * Process 가 스스로 종료한다고
 * Process Manager 에게 메시지를 전달하기 위한 함수
 */
LcsErrorT lcsFinalize(LcsProcessTypeT process)
{
    LcsFinalizeMsgT finalizeMsg = {0,};
    Int32T ret = 0;

    finalizeMsg.processType = process;

    ret = ipcSendAsync(PROCESS_MANAGER, IPC_LCS_C2PM_FINALIZE,
                        &finalizeMsg, sizeof(LcsFinalizeMsgT));

    fprintf(stdout, "[%s] Async Result : %d\n", __func__, ret);

    if (ret == SUCCESS)
    {
        return LCS_OK;
    }
    else
    {
        return LCS_ERR_LIBRARY;
    }
}

/*
 * Process 의 Attribute 를 설정하기 위해
 * Process Manager 에게 메시지를 전달하기 위한 함수
 */
LcsErrorT lcsAttributeSet(LcsProcessTypeT process, LcsAttributeT lcsAttribute)
{
    Int32T ret = 0;
    LcsAttributeMsgT lcsAttributeMsg = {0,};

    lcsAttributeMsg.processType = process;
    memcpy(&lcsAttributeMsg.attribute, &lcsAttribute, sizeof(LcsAttributeT));

    ret = ipcSendAsync(PROCESS_MANAGER, IPC_LCS_C2PM_ATTRIBUTE_SET,
                       &lcsAttributeMsg, sizeof(LcsAttributeMsgT));

    fprintf(stdout, "[%s] Async Result : %d\n", __func__, ret);

    if (ret == SUCCESS)
    {
        return LCS_OK;
    }
    else
    {
        return LCS_ERR_LIBRARY;
    }
}

/*
 * Process 를 관리하기 위한 설정을 얻기 위해(Sync)
 * Process Manager 에게 메시지를 전달하기 위한 함수
 */
LcsErrorT lcsAttributeGet(LcsProcessTypeT process, LcsAttributeT *pLcsAttribute)
{
    LcsAttributeMsgT lcsAttributeMsg = {0,};
    Int8T sndMsg[4096] = {0,};
    Int8T rcvMsg[4096] = {0,};
    Uint32T rcvLength = 0;
    Int32T ret = 0;

    lcsAttributeMsg.processType = process;

    memcpy(sndMsg, &lcsAttributeMsg, sizeof(LcsAttributeMsgT));

    ret = ipcSendSync(PROCESS_MANAGER, IPC_LCS_C2PM_ATTRIBUTE_REQUEST,
                    IPC_LCS_PM2C_ATTRIBUTE_RESPONSE, sndMsg,
                    sizeof(LcsAttributeMsgT), rcvMsg, &rcvLength);

    memcpy(pLcsAttribute, rcvMsg, rcvLength);

    fprintf(stdout, "[%s] Sync Result : %d\n", __func__, ret);

    ipcProcPendingMsg();

    if (ret == SUCCESS)
    {
        return LCS_OK;
    }
    else
    {
        return LCS_ERR_LIBRARY;
    }
}

/*
 * Process 가 발생한 Error 를
 * Process Manager 에게 전달하기 위한 함수
 */
LcsErrorT lcsErrorReport(LcsProcessTypeT process, LcsErrorT error,
            LcsRecoveryT recommendedRecovery)
{
    LcsErrorReportMsgT lcsErrorReport = {0,};
    Int32T ret = 0;

    lcsErrorReport.processType = process;
    lcsErrorReport.error = error;
    lcsErrorReport.recommendedRecovery = recommendedRecovery;

    ret = ipcSendAsync(PROCESS_MANAGER, IPC_LCS_C2PM_ERROR_REPORT,
                       &lcsErrorReport, sizeof(LcsErrorReportMsgT));

    fprintf(stdout, "[%s] Async Result : %d\n", __func__, ret);

    if (ret == SUCCESS)
    {
        return LCS_OK;
    }
    else
    {
        return LCS_ERR_LIBRARY;
    }
}

/*
 * Process 가 Service 의 중지를 완료했다고
 * Process Manager 에게 메시지를 전달하기 위한 함수
 */
LcsErrorT lcsRoleQuiescingComplete(LcsProcessTypeT process, Uint32T invocation,
            LcsErrorT error)
{
    LcsRoleQuiescingCompleteMsgT complete = {0,};
    Int32T ret = 0;

    complete.processType = process;
    complete.invocationId = invocation;
    complete.error = error;

    ret = ipcSendAsync(PROCESS_MANAGER, IPC_LCS_C2PM_ROLE_QUIESCING_COMPLETE,
                        &complete, sizeof(complete));

    fprintf(stdout, "[%s] Async Result : %d\n", __func__, ret);

    if (ret == SUCCESS)
    {
        return LCS_OK;
    }
    else
    {
        return LCS_ERR_LIBRARY;
    }
}

/*
 * Process 가 Process Manager 에서 전달받은 메시지에 대한
 * 응답메시지를 전달하기 위한 함수
 */
LcsErrorT lcsResponse(LcsProcessTypeT process, Uint32T invocation)
{
    LcsResponseMsgT responseMsg = {0,};
    Int8T sndMsg[4096] = {0,};
    Int8T rcvMsg[4096] = {0,};
    Uint32T rcvLength = 0;
    Int32T ret = 0;

    responseMsg.processType = process;
    responseMsg.invocationId = invocation;

    fprintf(stdout, "&&&&&&&&& LCS RESPONSE START &&&&&&&&&&\n");

    fprintf(stdout, "Process : [%d], Invacation : [%d]\n", process, invocation);

    if (invocation == IPC_LCS_PM2C_TERMINATE)
    {
        memcpy(sndMsg, &responseMsg, sizeof(LcsResponseMsgT));

        ret = ipcSendSync(PROCESS_MANAGER, IPC_LCS_C2PM_RESPONSE,
                        IPC_LCS_PM2C_RESPONSE_ACK, sndMsg,
                        sizeof(LcsAttributeMsgT), rcvMsg, &rcvLength);

        fprintf(stdout, "[%s] Sync Result : %d\n", __func__, ret);
        NNLOG(LOG_DEBUG, "Send Sync : Process [%d] Invocation [%d]\n",
                        process, invocation);

        ipcProcPendingMsg();
    }
    else
    {
        ret = ipcSendAsync(PROCESS_MANAGER, IPC_LCS_C2PM_RESPONSE,
                            &responseMsg, sizeof(responseMsg));

        fprintf(stdout, "[%s] Async Result : %d\n", __func__, ret);
        NNLOG(LOG_DEBUG, "Send ASync : Process [%d] Invocation [%d]\n",
                        process, invocation);
    }

    fprintf(stdout, "&&&&&&&&& LCS RESPONSE END &&&&&&&&&&\n");

    if (ret == SUCCESS)
    {
        return LCS_OK;
    }
    else
    {
        return LCS_ERR_LIBRARY;
    }
}
