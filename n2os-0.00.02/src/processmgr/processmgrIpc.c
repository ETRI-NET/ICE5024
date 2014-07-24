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
 * @brief : Process Manager 에서 사용하는 IPC 및 Event 메시지의
 *          수신 및 송신기능을 수행
 *  - Block Name : Process Manager
 *  - Process Name : procmgr
 *  - Creator : Changwoo Lee
 *  - Initial Date : 2014/3/3
 */

/**
* @file : processmgrIpc.c
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "nnTypes.h"
#include "nnTime.h"

#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"
#include "nosLib.h"

#include "nnBuffer.h"

#include "nnCmdDefines.h"
#include "nnCompProcess.h"

#include "processmgrIpc.h"
#include "processmgrUtility.h"

/*******************************************************************************
 *                              GLOBAL VARIABLES
 ******************************************************************************/
extern LcsProcessMgrT *gpProcMgrBase;

/*******************************************************************************
 *                         GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/
extern Int32T procMgrWriteConfCB (struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);

/*******************************************************************************
 *                         LOCAL CONSTANTS/LITERALS/TYPES
 ******************************************************************************/


/*******************************************************************************
*                               LOCAL VARIABLES
*******************************************************************************/
Int32T init = TRUE;
Int32T reload = FALSE;
Int32T procInitCnt = 0;
Int32T procReloadCnt = 0;
Int8T tempProcName[][20] = {"IPC_MANAGER",     // 0
                            "PROCESS_MANAGER", // 1
                            "RIB_MANAGER",     // 2
                            "",
                            "POLICY_MANAGER",  // 4
                            "",
                            "",
                            "COMMAND_MANAGER", // 7
                            "",
                            "",
                            "",
                            "",
                            "",
                            "",
                            "",
                            "",
                            "",
                            "RIP",  // 17
                            "",
                            "BGP",  // 19
};
Int8T tempVersion[MAX_PROCESS_CNT][20] = {{0,},};

/*******************************************************************************
 *                                   CODE
 ******************************************************************************/
/*
 * Description : IPC 메시지를 수신하는 경우 호출될 콜백 함수임.
 */
void procIpcProcess(Int32T msgId, void *pData, Uint32T dataLen)
{
    NNLOG(LOG_DEBUG, "enter %s\n", __FUNCTION__);

    Int8T msg[4096] = {0,};
    Int32T ipcIndex = 0;
    Int32T processType = 0;
    Int32T ipcType = 0;
    Int8T ret = 0;

    memcpy(msg, pData, dataLen);
    NNLOG(LOG_DEBUG, "PROCMGR RECEIVE DATA : msgId = %d len = %d\n",
            msgId, dataLen);

    fprintf(stdout,
            "PROCMGR RECEIVE DATA : msgId = %d len = %d\n", msgId, dataLen);

    switch(msgId)
    {
        case IPC_LCS_C2PM_REGISTER:
        {
            LcsRegisterMsgT lcsRegisterMsg = {0,};

            NNLOG(LOG_DEBUG, "=== IPC_LCS_C2PM_REGISTER ===\n");
            fprintf(stdout, "\n\n=== IPC_LCS_C2PM_REGISTER ===\n");

            memcpy(&lcsRegisterMsg, pData, sizeof(LcsRegisterMsgT));

            NNLOG(LOG_DEBUG, "Register Process - [P:%d][I:%d]\n",
                    lcsRegisterMsg.processType,
                    lcsRegisterMsg.ipcType);
            fprintf(stdout, "Register Process - [P:%d[[I:%d]\n",
                    lcsRegisterMsg.processType,
                    lcsRegisterMsg.ipcType);

            /* COMMAND_MANAGER 인지 확인 */
            if ((Int32T)lcsRegisterMsg.processType
                == (Int32T)LCS_COMMAND_MANAGER)
            {
                /* Component Command Init */
                gpProcMgrBase->gpCmshGlobal =
                                compCmdInit(IPC_PRO_MGR, procMgrWriteConfCB);
#if 1
                /* IPC Manager 에게 Command Manager 의 기동 알림 */
                ipcSendAsync(IPC_MANAGER,
                            IPC_LCS_PM2C_COMMAND_MANAGER_UP, NULL, 0);
#endif
                /* Startup Config 를 받아온다 */
                ipcSendAsync(COMMAND_MANAGER,
                            IPC_STARTUP_CONFIG_REQUEST, NULL, 0);

                NNLOG(LOG_DEBUG, "Request Startup Config!!!\n");
                fprintf(stdout, "Request Startup Config!!!\n");
            }

            NNLOG(LOG_DEBUG, "DEL REGISTER RESPONSE TIMER [%d]\n",
                            lcsRegisterMsg.processType);
            fprintf(stdout, "DEL REGISTER RESPONSE TIMER [%d]\n",
                            lcsRegisterMsg.processType);
            timerDel(lcsRegisterMsg.processType,
                    LCS_RESPONSE_TIMER(lcsRegisterMsg.processType));

            /* 기동한 Process 수 증가 */
            ++procInitCnt;

            /* IPC Type 등록 */
            LCS_INFO(lcsRegisterMsg.processType).ipcType = lcsRegisterMsg.ipcType;

            /* Version 등록 */
            strcpy(LCS_INFO(lcsRegisterMsg.processType).version,
                   tempVersion[LCS_INFO(lcsRegisterMsg.processType).ipcType]);

            fprintf(stdout, "$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
            fprintf(stdout,
                    "REGISTER PROCESS[%d] - VERSION[%s]\n",
                    lcsRegisterMsg.processType,
                    LCS_INFO(lcsRegisterMsg.processType).version);
            fprintf(stdout, "$$$$$$$$$$$$$$$$$$$$$$$$$$\n");

            fprintf(stdout, "ADD ACTIVE HEALTH TIMER [%d]\n", lcsRegisterMsg.processType);
            /* HealthCheck Active 하는 타이머 함수 추가 */
            LCS_HEALTH_ACTIVE_TIMER(lcsRegisterMsg.processType) =
                timerAdd(lcsRegisterMsg.processType,
                        LCS_HEALTH_INTERVAL(lcsRegisterMsg.processType),
                        0,
                        (void *)healthActiveTimerCallback,
                        &LCS_INFO(lcsRegisterMsg.processType));
            NNLOG(LOG_DEBUG, "ADD ACTIVE HEALTH TIMER [%d][%d]\n",
                        lcsRegisterMsg.processType,
                        LCS_HEALTH_ACTIVE_TIMER(lcsRegisterMsg.processType));

            fprintf(stdout, "ADD PASSIVE HEALTH TIMER [%d]\n", lcsRegisterMsg.processType);
            /* HealthCheck Passive 하는 타이머 함수 추가 */
            LCS_HEALTH_PASSIVE_TIMER(lcsRegisterMsg.processType) =
                timerAdd(lcsRegisterMsg.processType,
                        LCS_HEALTH_INTERVAL(lcsRegisterMsg.processType),
                        0,
                        (void *)healthPassiveTimerCallback,
                        &LCS_INFO(lcsRegisterMsg.processType));
            NNLOG(LOG_DEBUG, "ADD PASSIVE HEALTH TIMER [%d][%d]\n",
                        lcsRegisterMsg.processType,
                        LCS_HEALTH_PASSIVE_TIMER(lcsRegisterMsg.processType));

            /* 자신이 살린 Binary 수 가 gBinCnt - 1 일 때
               모든 프로세스를 살렸으므로 모두에게 Role 을 설정한다 */
#if 0
NNLOG(LOG_DEBUG, "-------------========== %d %d ==========------\n",
gpProcMgrBase->runCount - LCS_SEND_SIGNAL, procInitCnt);
#endif
            if (gpProcMgrBase->status == LCS_INIT &&
                (gpProcMgrBase->runCount - LCS_SEND_SIGNAL) == procInitCnt)
            {
                NNLOG(LOG_DEBUG, "===== All Process Register =====\n");

                fprintf(stdout, "\n\n===== All Process Register =====\n");
                fprintf(stdout, "===== Start Send Role to All Processes =====\n");

                for (ipcIndex = 0; ipcIndex < MAX_PROCESS_CNT - 1; ++ipcIndex)
                {
                    if (ipcIndex != LCS_IPC_MANAGER &&
                        ipcIndex != LCS_PROCESS_MANAGER && 
                        LCS_INFO(ipcIndex).run == TRUE)
                    {
                        processType = LCS_INFO(ipcIndex).processType;
                        ipcType = LCS_INFO(ipcIndex).ipcType;

                        NNLOG(LOG_DEBUG,
                                "Send Set Role Message - [P:%d][I:%d]\n", 
                                processType, ipcType);
                        fprintf(stdout,
                                "Send Set Role Message - [P:%d][I:%d]\n",
                                processType, ipcType);
                        ret = sendSetRoleMessage(processType);

                        if (ret == SUCCESS)
                        {
                            fprintf(stdout, "ADD RESPONSE(SETROLE) TIMER : Register [%d]\n", processType);
                            /* Timeout Timer 등록 */
                            LCS_RESPONSE_TIMER(processType) =
                                timerAdd(processType,
                                        LCS_RESPONSE_TIMEOUT(processType),
                                        0,
                                        (void *)timeoutTimerCallback,
                                        &LCS_INFO(processType));
                        }
                    }
                }

                NNLOG(LOG_DEBUG, "===== End Send Role to All Processes =====\n");
                fprintf(stdout, "===== End Send Role to All Processes =====\n\n");

                procInitCnt = 0;
                gpProcMgrBase->status = LCS_RUN;
            }

            /* Recovery(Restart) Action 수행 후 Register 시 */
            else if (gpProcMgrBase->status == LCS_RUN
                    || gpProcMgrBase->status == LCS_QUIESCING)
     
            {
                processType = lcsRegisterMsg.processType;

                LCS_INFO(processType).error = LCS_OK;

                fprintf(stdout, "ELSE Send Set Role Message\n");
                ret = sendSetRoleMessage(processType);

                if (ret == SUCCESS)
                {
                    fprintf(stdout, "ELSE ADD RESPONSE(SETROLE) TIMER : Register\n");
                    /* Timeout Timer 등록 */
                    LCS_RESPONSE_TIMER(processType) =
                        timerAdd(processType,
                                LCS_RESPONSE_TIMEOUT(processType),
                                0,
                                (void *)timeoutTimerCallback,
                                &LCS_INFO(processType));

                    if (gpProcMgrBase->status == LCS_QUIESCING)
                    {
                        LCS_QUIESCING_TIMER(processType) =
                                timerAdd(processType,
                                LCS_RESPONSE_TIMEOUT(processType),
                                0,
                                (void *)timeoutTimerCallback,
                                &LCS_INFO(processType));
                    }
                }
            }
        }
            break;
#if 1
        case IPC_LCS_C2PM_FINALIZE:
        {
            LcsFinalizeMsgT lcsFinalizeMsg = {0,};
            LcsErrorOccurredEventT lcsErrorEvent = {0,};

            memcpy(&lcsFinalizeMsg, msg, sizeof(LcsFinalizeMsgT));

            processType = lcsFinalizeMsg.processType;

            fprintf(stdout, "+++++++++++++++++++++++++++++++++++++++++\n");
            fprintf(stdout, "Finalize Message From [%d]\n", processType);

            lcsErrorEvent.processType = processType;
            lcsErrorEvent.error = LCS_ERR_UNAVAILABLE;
            lcsErrorEvent.recoveryAction = LCS_COMPONENT_RESTART;

            /* Recovery Action 수행 */
            ret = doRecoveryAction(processType, lcsErrorEvent);
            fprintf(stdout, "+++++++++++++++++++++++++++++++++++++++++\n");
        }
            break;
        case IPC_LCS_C2PM_ERROR_REPORT:
        {
            LcsErrorReportMsgT lcsErrorReportMsg = {0,};
            LcsErrorOccurredEventT lcsErrorEvent = {0,};

            memcpy(&lcsErrorReportMsg, msg, sizeof(LcsErrorReportMsgT));
            memcpy(&lcsErrorEvent, msg, sizeof(LcsErrorOccurredEventT));

            processType = lcsErrorReportMsg.processType;

            fprintf(stdout, "=========================================\n");
            fprintf(stdout, "Error Report From [%d] Error[%d] Recovery[%d]\n",
                    lcsErrorReportMsg.processType, lcsErrorReportMsg.error,
                    lcsErrorReportMsg.recommendedRecovery);

            /* Recovery Action 수행 */
            ret = doRecoveryAction(processType, lcsErrorEvent);

            fprintf(stdout, "=========================================\n");
        }
            break;
#endif
        case IPC_LCS_C2PM_ROLE_QUIESCING_COMPLETE:
        {
            Uint32T invocationId = 0;

            memcpy(&processType, msg, sizeof(Int32T));
            memcpy(&invocationId, msg + sizeof(Int32T), sizeof(Int32T));

            NNLOG(LOG_DEBUG, "Receive From[%d] MsgId[%u]\n",
                    processType, invocationId);
            fprintf(stdout, "Receive From[%d] MsgID[%u]\n",
                    processType, invocationId);

            if (LCS_STATUS.serviceStatus & (1 << processType))
            {
                LCS_STATUS.serviceStatus ^= (1 << processType);
            }

            NNLOG(LOG_DEBUG, "Event Publish ^=\n");
            fprintf(stdout, "Event Publish ^=\n");
            eventPublish(EVENT_LCS_COMPONENT_SERVICE_STATUS,
                        &LCS_STATUS, sizeof(LcsServiceStatusEventT));

            fprintf(stdout, "DEL QUIESCING TIMER\n");

            timerDel(processType, LCS_QUIESCING_TIMER(processType));
            LCS_QUIESCING_TIMER(processType) = NULL;

            /* Reload 시 수행되는 부분 */
            if (gpProcMgrBase->status == LCS_RELOAD &&
                LCS_STATUS.serviceStatus == 0)
            {
                for (ipcIndex = 1; ipcIndex < MAX_PROCESS_CNT - 1; ++ipcIndex)
                {
                    if (LCS_INFO(ipcIndex).run == TRUE)
                    {
                        processType = LCS_INFO(ipcIndex).processType;

                        ret = sendTerminateMessage(processType);

                        if (ret == SUCCESS)
                        {
                            /* Timeout Timer 등록 */
                            LCS_RESPONSE_TIMER(processType) =
                                timerAdd(processType,
                                    LCS_RESPONSE_TIMEOUT(processType),
                                    0,
                                    (void *)timeoutTimerCallback,
                                    &LCS_INFO(processType));
                        }

                        NNLOG(LOG_DEBUG, "[%d] Send Terminate Message to [%d]\n",
                                IPC_LCS_PM2C_TERMINATE,
                                processType);
                        fprintf(stdout, "[%d] Send Terminate Message to [%d]\n",
                                IPC_LCS_PM2C_TERMINATE,
                                processType);
                        sleep(1);
                    }
                }
            }
        }
            break;
        case IPC_LCS_C2PM_RESPONSE:
        {
            Uint32T invocationId = 0;

            memcpy(&processType, msg, sizeof(Int32T));
            memcpy(&invocationId, msg + sizeof(Int32T), sizeof(Int32T));

            NNLOG(LOG_DEBUG, "Receive From[%d] MsgID[%u]\n",
                    processType, invocationId);
            fprintf(stdout, "Receive From[%d] MsgID[%u]\n",
                    processType, invocationId);

            switch (invocationId)
            {
                case IPC_LCS_PM2C_SETROLE:
                {
                    if (gpProcMgrBase->haState != LCS_HA_QUIESCING)
                    {
                        LCS_STATUS.serviceStatus |= (1 << processType);

                        NNLOG(LOG_DEBUG, "Event Publish |=\n");
                        fprintf(stdout, "Event Publish |=\n");
                        eventPublish(EVENT_LCS_COMPONENT_SERVICE_STATUS,
                                    &LCS_STATUS, sizeof(LcsServiceStatusEventT));
                    }

                    fprintf(stdout, "DEL RESPONSE(SETROLE) TIMER [%d]\n", processType);
                    timerDel(processType, LCS_RESPONSE_TIMER(processType));
                    LCS_RESPONSE_TIMER(processType) = NULL;
                }
                    break;

                case IPC_LCS_PM2C_TERMINATE:
                {
                    /* Reload 시 모든 Process 의 TERMINATE 메시지를 받으면
                     * IPC 종료 및 Restart
                     */
                    Int32T dummy = 0;
                    ret = ipcResponseSync(LCS_INFO(processType).ipcType,
                                            IPC_LCS_PM2C_RESPONSE_ACK,
                                            &dummy, sizeof(Int32T));
#if 1
                    /* Process Manager 가 종료 중 일 때  */
                    if (gpProcMgrBase->status == LCS_STOPING)
                    {
                        fprintf(stdout, "TERMINATE %d\n",
                                gpProcMgrBase->runCount - LCS_SEND_SIGNAL);
                    }
#endif
                    else if (gpProcMgrBase->status == LCS_RELOAD)
                    {
                        ++procReloadCnt;

                        if ((gpProcMgrBase->runCount - LCS_SEND_SIGNAL)
                                == procReloadCnt)
                        {
                            NNLOG(LOG_DEBUG,
                                "ALL Process is Terminated(LCS_RELOAD)\n");
                            fprintf(stdout,
                                "ALL Process is Terminated(LCS_RELOAD)\n");
                            sendSignaltoPid(LCS_INFO(LCS_IPC_MANAGER).pid,
                                            SIGTERM);
                            sleep(1);

                            reloadTimerCallback(0, 0, NULL);

                            procReloadCnt = 0;
                        }
                    }
                    else
                    {
                        Int32T status = 0;
                        Int32T pid = 0;

                        // 자식의 종료를 확인
//                        waitpid(-1, &status, WNOHANG);
                        wait(&status);
                        NNLOG(LOG_DEBUG, "Check Child Process End [%d]\n",
                                        status);
                        fprintf(stdout, "Check Child Process End [%d]\n",
                                        status);

                        delAllTimer(processType);

                        // 해당 프로세스의 상태 제거 후 Event 전송
                        if (LCS_STATUS.serviceStatus & (1 << processType))
                        {
                            LCS_STATUS.serviceStatus ^= (1 << processType);
                        }

                        NNLOG(LOG_DEBUG, "Event Publish ^=\n");
                        fprintf(stdout, "Event Publish ^=\n");
                        eventPublish(EVENT_LCS_COMPONENT_SERVICE_STATUS,
                                &LCS_STATUS, sizeof(LcsServiceStatusEventT));

                        // 해당 프로세스 fork()
                        if (gpProcMgrBase->status == LCS_INIT ||
                            gpProcMgrBase->status == LCS_RUN ||
                            gpProcMgrBase->status == LCS_QUIESCING)
                        {
                            if ((pid = startProcess(
                                        LCS_INFO(processType).binName)) == FAILURE)
                            {
                                fprintf(stdout, "Run Process Error - [%s]\n",
                                        LCS_INFO(processType).binName);

                                 /*
                                  * Process 의 실행이 실패했다면
                                  * Process Manager 종료
                                  */
                                 sendSignaltoPid(
                                    LCS_INFO(LCS_PROCESS_MANAGER).pid, SIGTERM);

                                 exit(0);
                            }
                            else
                            {
                                fprintf(stdout, "Run Process - [%s][%d][%d]\n",
                                        LCS_INFO(processType).binName,
                                        processType, pid);

                                LCS_RESPONSE_TIMER(processType) =
                                    timerAdd(processType,
                                            LCS_RESPONSE_TIMEOUT(processType),
                                            0,
                                            (void *)timeoutTimerCallback,
                                            &LCS_INFO(processType));

                                nnTimeGetCurrent(
                                        LCS_INFO(processType).startTime);

                                LCS_INFO(processType).pid = pid;
                            }
                        }
                    }

                    fprintf(stdout, "^^^^^^^^^^^^^^^^^^^PROC CNT\n");
                    fprintf(stdout, "Reload CNT : %d\n", procReloadCnt);
                    fprintf(stdout, "^^^^^^^^^^^^^^^^^^^PROC CNT\n");

                    fprintf(stdout, "DEL RESPONSE(TERMINATE) TIMER [%d]\n", processType);
                    timerDel(processType, LCS_RESPONSE_TIMER(processType));
                    LCS_RESPONSE_TIMER(processType) = NULL;
                }
                    break;

                case IPC_LCS_PM2C_HEALTHCHECK:
                {
                    fprintf(stdout, "DEL HEALTH RESPONSE TIMER [%d]\n", processType);
                    timerDel(processType, LCS_HEALTH_RESPONSE_TIMER(processType));
                    LCS_HEALTH_RESPONSE_TIMER(processType) = NULL;
                }
                    break;
                default:
                    break;

            }
        }
            break;
#if 1
        case IPC_LCS_C2PM_ATTRIBUTE_SET:
        {
            LcsAttributeMsgT attribute = {0,};

            memcpy(&attribute, msg, sizeof(LcsAttributeMsgT));

            processType = attribute.processType;

            NNLOG(LOG_DEBUG, "PROCMGR RECV ATTRIBUTE SET : [%d]\n",
                            processType);
            fprintf(stdout, "PROCMGR RECV ATTRIBUTE SET : [%d]\n", processType);

            memcpy(&LCS_INFO(processType).attribute,
                    &attribute.attribute, sizeof(LcsAttributeT));

            NNLOG(LOG_DEBUG, "ATTRIBUTE SET COMPLETE : [%d]\n", processType);
            fprintf(stdout, "ATTRIBUTE SET COMPLETE : [%d]\n", processType);
        }
            break;
        case IPC_LCS_C2PM_ATTRIBUTE_REQUEST:
        {
            LcsAttributeMsgT attribute = {0,};

            memcpy(&attribute, msg, sizeof(LcsAttributeMsgT));

            processType = attribute.processType;

            NNLOG(LOG_DEBUG, "PROCMGR RECV ATTRIBUTE REQUEST : [%d]\n",
                            processType);
            fprintf(stdout, "PROCMGR RECV ATTRIBUTE REQUEST : [%d]\n",
                            processType);

            ret = ipcResponseSync(LCS_INFO(processType).ipcType,
                    IPC_LCS_PM2C_ATTRIBUTE_RESPONSE,
                    &LCS_INFO(processType).attribute, sizeof(LcsAttributeMsgT));

            printf("Sync Result : %d\n", ret);

            ipcProcPendingMsg();
        }
            break;
#endif
        case IPC_STARTUP_CONFIG_RESPONSE:
        {
            NNLOG(LOG_DEBUG, "Before PROCMGR RECV HA State : %d\n",
                            gpProcMgrBase->haState);
            fprintf(stdout, "Before PROCMGR RECV HA State : %d\n",
                            gpProcMgrBase->haState);

            // Role 값 얻기
            memcpy(&gpProcMgrBase->haState, msg, sizeof(LcsHaStateT));

            NNLOG(LOG_DEBUG, "After PROCMGR RECV HA State : %d\n",
                            gpProcMgrBase->haState);
            fprintf(stdout, "After PROCMGR RECV HA State : %d\n",
                            gpProcMgrBase->haState);
        }
            break;
        case IPC_DYNAMIC_UPGRADE_VERSION:
        {
            nnBufferT msgBuff = {0,};
            Int8T compVer[64] = {0,};
            Int32T type = 0, versionLen = 0;
            Int32T verIndex = 0;

            nnBufferReset(&msgBuff);

            nnBufferAssign(&msgBuff, msg, dataLen);

            type = nnBufferGetInt32T(&msgBuff);
            versionLen = nnBufferGetInt8T(&msgBuff);
            nnBufferGetString(&msgBuff, compVer, versionLen);
 
            NNLOG(LOG_DEBUG, 
                    "Receive Dynamic Upgrade Version: [%d][%d][%s]\n",
                    type, versionLen, compVer);
            fprintf(stdout,
                    "Receive Dynamic Upgrade Version: [%d][%d][%s]\n",
                    type, versionLen, compVer);

            /* Process Manager 의 Version 일 경우 */
            if (type == PROCESS_MANAGER)
            {
                strcpy(LCS_INFO(LCS_PROCESS_MANAGER).version, compVer);
            }
            /* IPC Manager 의 Version 일 경우 */
            else if (type == IPC_MANAGER)
            {
                strcpy(LCS_INFO(LCS_IPC_MANAGER).version, compVer);
            }
            /* 그 외의 Version 일 경우 */
            else
            {
                /* 아직 LCS 가 초기 상태일 때 */
                if (gpProcMgrBase->status == LCS_INIT)
                {
                    strcpy(tempVersion[type], compVer);
                }
                else
                {
                    for (verIndex = 0;
                        verIndex < MAX_PROCESS_CNT - 1; ++verIndex)
                    {
                        if (LCS_INFO(verIndex).ipcType == type)
                        {
                            strcpy(LCS_INFO(verIndex).version, compVer);

                            NNLOG(LOG_DEBUG,
                            "Receive Dynamic Upgrade Version: [%d][%d][%s]\n",
                            type, versionLen, compVer);
                            fprintf(stdout,
                            "Receive Dynamic Upgrade Version: [%d][%d][%s]\n",
                            type, versionLen, compVer);

                            break;
                        }
                    }
                }
            }
        }
            break;
        /* 나의 업데이트만 메시지가 온다 */
        case IPC_LCS_PM2C_DYNAMIC_UPGRADE_RESPONSE:
        {
            nnBufferT msgBuff = {0,};
            Int8T compVer[64] = {0,};
            Int8T versionLen = 0;
            Int32T cmdFd = 0, cmdKey = 0, cmdResult = 0;

            NNLOG(LOG_DEBUG,
                  "== IPC_LCS_PM2C_DYNAMIC_UPGRADE_RESPONSE Start ==\n");

            nnBufferAssign(&msgBuff, msg, dataLen);

            cmdFd = nnBufferGetInt32T(&msgBuff);
            cmdKey = nnBufferGetInt32T(&msgBuff);
            cmdResult = nnBufferGetInt32T(&msgBuff);
            versionLen = nnBufferGetInt8T(&msgBuff);
            nnBufferGetString(&msgBuff, compVer, versionLen);

            NNLOG(LOG_DEBUG, "cmdFd[%d], cmdKey[%d], cmdResult[%d], len[%d]\n",
                            cmdFd, cmdKey, cmdResult, versionLen);
            fprintf(stdout, "cmdFd[%d], cmdKey[%d], cmdResult[%d], len[%d]\n",
                            cmdFd, cmdKey, cmdResult, versionLen);

            if (cmdResult == SUCCESS)
            {
                NNLOG(LOG_DEBUG, "Process Manager Dynamic Update Success\n");
                NNLOG(LOG_DEBUG, "-> Update Version : %s\n", compVer);

                cmdOamPrint("Process Manager Dynamic Update Success");
                cmdOamPrint("-> Update Version : %s", compVer);
            }
            else
            {
                NNLOG(LOG_DEBUG, "Process Manager Dynamic Update Failed\n");
                NNLOG(LOG_DEBUG, "-> Reason : %s\n", compVer);

                cmdOamPrint("Process Manager Dynamic Update Failed");
                cmdOamPrint("-> Reason : %s", compVer);
            }

            NNLOG(LOG_DEBUG,
                  "== IPC_LCS_PM2C_DYNAMIC_UPGRADE_RESPONSE End ==\n");
        }
            break;
#if 0
        /* Stop Service 를 하기 위한 Test 용 Message */
        case IPC_LCS_TEST_QUIESCING:
        {
            fprintf(stdout, "*** PROCMGR RECV TEST QUIESCING ***\n");
            fprintf(stdout, "Start Send Quiescing to All Processes\n");

            gpProcMgrBase->status = LCS_QUIESCING;
            gpProcMgrBase->haState = LCS_HA_QUIESCING;

            for (ipcIndex = 1; ipcIndex < MAX_PROCESS_CNT - 1; ++ipcIndex)
            {
                if (LCS_INFO(ipcIndex).run == TRUE)
                {
                    processType = LCS_INFO(ipcIndex).processType;
                    ret = sendSetRoleMessage(processType);

                    if (ret == SUCCESS)
                    {
                        LCS_RESPONSE_TIMER(processType) =
                                timerAdd(processType,
                                LCS_RESPONSE_TIMEOUT(processType),
                                0,
                                (void *)timeoutTimerCallback,
                                &LCS_INFO(processType));

                        LCS_QUIESCING_TIMER(processType) =
                                timerAdd(processType,
                                LCS_RESPONSE_TIMEOUT(processType),
                                0,
                                (void *)timeoutTimerCallback,
                                &LCS_INFO(processType));
                    }
                }
            }
        }
            break;

        /* Reload 를 하기 위한 Test 용 Message */
        case IPC_LCS_TEST_RELOAD:
        {
            fprintf(stdout, "*** PROCMGR RECV TEST RELOAD ***\n");
            fprintf(stdout, "Start Send Quiescing to All Processes\n");

            gpProcMgrBase->status = LCS_RELOAD;
            gpProcMgrBase->haState = LCS_HA_QUIESCING;

            for (ipcIndex = 1; ipcIndex < MAX_PROCESS_CNT - 1; ++ipcIndex)
            {
                if (LCS_INFO(ipcIndex).run == TRUE)
                {
                    processType = LCS_INFO(ipcIndex).processType;

                    fprintf(stdout, "[%d] Send Role[%d] to %s[%d]\n",
                            IPC_LCS_PM2C_SETROLE,
                            gpProcMgrBase->haState,
                            tempProcName[processType],
                            processType);

                    ret = sendSetRoleMessage(processType);

                    if (ret == SUCCESS)
                    {
                        LCS_RESPONSE_TIMER(processType) =
                            timerAdd(processType,
                                LCS_RESPONSE_TIMEOUT(processType),
                                0,
                                (void *)timeoutTimerCallback,
                                &LCS_INFO(processType));
                    }
                }
            }
        }
            break;
#endif
        default:
            NNLOG(LOG_DEBUG, "Error Wrong IPC Message ID = %d\n", msgId);
            fprintf(stdout, "Error Wrong IPC Message ID = %d\n", msgId);
            break;
    }

    return;
}

//component event process
void procEventProcess(Int32T msgId, void *pData, Uint32T dataLen)
{
    NNLOG(LOG_DEBUG, "enter %s\n", __FUNCTION__);

    fprintf(stdout, "############################\n");
    fprintf(stdout, "## %s called !!!\n", __func__);
    fprintf(stdout, "############################\n");

    return;
}

/* Timeout 시 호출되는 Timer 콜백함수 */
void timeoutTimerCallback(Int32T fd, Int16T event, void *pArg)
{
    LcsProcessInfoT *pProcessInfo = NULL;
    Int32T processType = 0;

    pProcessInfo = (LcsProcessInfoT *)pArg;
    processType = pProcessInfo->processType;

    NNLOG(LOG_DEBUG, "############# Timeout Timer Callback ###############\n");
    NNLOG(LOG_DEBUG, "ProcessType [%d], Error [%d]\n",
                    processType, pProcessInfo->error);

    fprintf(stdout, "\n\n############ Timeout Timer Callback ##############\n");
    fprintf(stdout, "ProcessType [%d], Error [%d]\n",
                    processType, pProcessInfo->error);

    // Process 의 재시작 시간, 에러내용을 입력
    nnTimeGetCurrent(LCS_INFO(processType).lastStartTime);
    LCS_INFO(processType).lastError = LCS_ERR_TIMEOUT;
    LCS_INFO(processType).lastRecovery = LCS_INFO(processType).recovery;

    NNLOG(LOG_DEBUG, "^^^^^^^^TIMEOUT^^^^^^^^ RestartTime : %s, Action : %d\n",
                    LCS_INFO(processType).lastStartTime,
                    LCS_INFO(processType).lastRecovery);
    fprintf(stdout, "^^^^^^^TIMEOUT^^^^^^^ RestartTime : %s, Action : %d\n",
                    LCS_INFO(processType).lastStartTime,
                    LCS_INFO(processType).lastRecovery);

    // 에러이므로 강제 재시작
    forcefulRestart(processType);

    NNLOG(LOG_DEBUG, "############ Timeout Timer Callback ##############\n\n");
    fprintf(stdout, "############ Timeout Timer Callback ##############\n\n");
}

/* Healthcheck 시 호출되는 Timer 콜백함수 */
void healthActiveTimerCallback(Int32T fd, Int16T event, void *pArg)
{
    LcsHealthcheckRequestMsgT healthcheck = {0,};
    Int32T processType = 0;
    Int32T ret = 0;

    processType = ((LcsProcessInfoT *)pArg)->processType;

    NNLOG(LOG_DEBUG, "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
    NNLOG(LOG_DEBUG, "HEALTHCHECK Active Timer Callback [%d]\n", processType);

    fprintf(stdout, "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
    fprintf(stdout, "HEALTHCHECK Active Timer Callback [%d]\n", processType);

    // RELOAD 시 Active Health Check 안함
    if (gpProcMgrBase->status != LCS_RELOAD)
    {
        healthcheck.processType = processType;
        healthcheck.invocationId = IPC_LCS_PM2C_HEALTHCHECK;

        ret = ipcSendAsync(LCS_INFO(healthcheck.processType).ipcType,
                            IPC_LCS_PM2C_HEALTHCHECK,
                            &healthcheck, sizeof(LcsHealthcheckRequestMsgT));

        if (ret == SUCCESS)
        {
            // Process 의 Active Health Check 횟수를 증가
            ++LCS_INFO(processType).healthcheckCount;

            fprintf(stdout, "ADD ACTIVE HEALTH RESPONSE TIMER [%d]\n",
                            healthcheck.processType);
            LCS_HEALTH_RESPONSE_TIMER(healthcheck.processType) =
                timerAdd(healthcheck.processType,
                    LCS_HEALTH_TIMEOUT(healthcheck.processType),
                    0,
                    (void *)timeoutTimerCallback,
                    &LCS_INFO(healthcheck.processType));

            /* HealthCheck Active 하는 타이머 함수 추가 */
            fprintf(stdout, "ADD ACTIVE HEALTH TIMER [%d]\n", processType);
            LCS_HEALTH_ACTIVE_TIMER(processType) =
                        timerAdd(processType,
                                LCS_HEALTH_INTERVAL(processType),
                                0,
                                (void *)healthActiveTimerCallback,
                                &LCS_INFO(processType));
        }
        else
        {
            fprintf(stdout, "SET ERROR Process [%d]\n",
                            healthcheck.processType);
            LCS_INFO(healthcheck.processType).error = LCS_ERR_MESSAGE_ERROR;

            // Process 의 재시작 시간, 에러내용을 입력
            nnTimeGetCurrent(LCS_INFO(processType).lastStartTime);
            LCS_INFO(processType).lastError = LCS_ERR_MESSAGE_ERROR;
            LCS_INFO(processType).lastRecovery = LCS_INFO(processType).recovery;

            NNLOG(LOG_DEBUG, "^^^^^^^HEALTH CHECK^^^^^^^ RestartTime : %s, \
                            Action : %d\n",
                            LCS_INFO(processType).lastStartTime,
                            LCS_INFO(processType).lastRecovery);
            fprintf(stdout, "^^^^^^^HEALTH CHECK^^^^^^^ RestartTime : %s, \
                            Action : %d\n",
                            LCS_INFO(processType).lastStartTime,
                            LCS_INFO(processType).lastRecovery);

            // 에러이므로 강제 재시작
            forcefulRestart(processType);
        }
        printf("Async Result : %d\n", ret);
    }
    else
    {
        NNLOG(LOG_DEBUG, "LCS_RELOAD - Do Not Active Health Check\n");
        fprintf(stdout, "LCS_RELOAD - Do Not Active Health Check\n");
    }

    NNLOG(LOG_DEBUG, "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
    fprintf(stdout, "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
}

/* Healthcheck Passive 시 호출되는 Timer 콜백함수 */
void healthPassiveTimerCallback(Int32T fd, Int16T event, void *pArg)
{
    FILE *fpResult = NULL;
    StringT binary = NULL;
    float cpu = 0.0f;

    Int8T sCmd[128] = {0,};
    Int8T sBuffer[128] = {0,};
    Int32T mem = 0;
    Int32T processType = 0;

    processType = ((LcsProcessInfoT *)pArg)->processType;

    NNLOG(LOG_DEBUG, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    NNLOG(LOG_DEBUG, "HEALTHCHECK Passive Timer Callback : %d\n", processType);

    fprintf(stdout, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    fprintf(stdout, "HEALTHCHECK Passive Timer Callback : %d\n", processType);

    if ((binary = rindex(LCS_INFO(processType).binName, '/')) != NULL)
    {
        ++binary;
    }

    /* ps 명령어 실행 후 cpu%, memory(kb)결과 얻기 */
    sprintf(sCmd, "ps -C %s -o %%cpu,rss", binary);

    fprintf(stdout, "Command -> %s\n", sCmd);

    fpResult = popen(sCmd, "r");

    if (fpResult != NULL)
    {
        while (fgets(sBuffer, 128, fpResult))
        {
            if (strstr(sBuffer, "%") != NULL)
            {
                continue;
            }

            sscanf(sBuffer, "%f %d", &cpu, &mem);

            NNLOG(LOG_DEBUG, "%s -> CPU [%.1f]%%, MEMORY [%dM]\n", binary, cpu,
                    (mem / LCS_ONE_MEGA));
            fprintf(stdout, "%s -> CPU [%.1f]%%, MEMORY [%dM]\n", binary, cpu,
                    (mem / LCS_ONE_MEGA));

            break;
        }

        fclose(fpResult);
        fpResult = NULL;
    }

    /* Process 의 Attribute 와 비교 후 넘어가면 Recovery 수행 */
    if (LCS_ATTR(processType).memoryThreshold < (mem / LCS_ONE_MEGA) ||
        LCS_ATTR(processType).cpuThreshold < (Int32T)cpu)
    {
        LcsErrorOccurredEventT lcsErrorEvent = {0,};

        LCS_INFO(processType).error = LCS_ERR_NO_MEMORY;

        NNLOG(LOG_DEBUG, "Over Process Threshold, Do Recovery Action\n");
        fprintf(stdout, "\n\nOver Process Threshold, Do Recovery Action\n\n");

        lcsErrorEvent.processType = processType;
        lcsErrorEvent.error = LCS_INFO(processType).error;
        lcsErrorEvent.recoveryAction = LCS_COMPONENT_RESTART;

        /* Recovery Action 수행 */
        doRecoveryAction(processType, lcsErrorEvent);
    }
#if 0
    /* Test 용 */
    if (processType == 17)
    {
        testData += 5;
    }
#endif
    NNLOG(LOG_DEBUG, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    fprintf(stdout, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

    /* HealthCheck Passive 하는 타이머 함수 추가 */
    fprintf(stdout, "ADD PASSIVE HEALTH RESPONSE TIMER [%d]\n", processType);
    LCS_HEALTH_PASSIVE_TIMER(processType) =
                timerAdd(processType,
                        LCS_HEALTH_INTERVAL(processType),
                        0,
                        (void *)healthPassiveTimerCallback,
                        &LCS_INFO(processType));
}

Int8T sendSetRoleMessage(Int32T processType)
{
    LcsSetRoleMsgT lcsSetRole = {0,};
    Int8T ret = 0;

    lcsSetRole.processType = processType;
    lcsSetRole.invocationId = IPC_LCS_PM2C_SETROLE;
    lcsSetRole.haState = gpProcMgrBase->haState;

    LCS_ATTR(processType).haState = gpProcMgrBase->haState;

    /* Set Role 메시지 전송 */
    NNLOG(LOG_DEBUG, "Set Role Message Send [%d][%d]\n",
            lcsSetRole.processType,
            LCS_INFO(lcsSetRole.processType).ipcType);

    fprintf(stdout, "Set Role Message Send [%d][%d]\n",
            lcsSetRole.processType,
            LCS_INFO(lcsSetRole.processType).ipcType);

    ret = ipcSendAsync(LCS_INFO(lcsSetRole.processType).ipcType,
                        IPC_LCS_PM2C_SETROLE,
                        &lcsSetRole,
                        sizeof(LcsSetRoleMsgT));

    return ret;
}

/* Terminate 메시지 전송 */
Int8T sendTerminateMessage(Int32T processType)
{
    LcsTerminateMsgT lcsTerminate = {0,};
    Int8T ret = 0;

    lcsTerminate.processType = processType;
    lcsTerminate.invocationId = IPC_LCS_PM2C_TERMINATE;

    NNLOG(LOG_DEBUG, "TERMINATE Message Send [%d][%d]\n",
            lcsTerminate.processType,
            LCS_INFO(lcsTerminate.processType).ipcType);
    fprintf(stdout, "TERMINATE Message Send [%d][%d]\n",
            lcsTerminate.processType,
            LCS_INFO(lcsTerminate.processType).ipcType);

    ret = ipcSendAsync(LCS_INFO(lcsTerminate.processType).ipcType,
                        IPC_LCS_PM2C_TERMINATE,
                        &lcsTerminate,
                        sizeof(LcsTerminateMsgT));

    return ret;
}

Int8T doRecoveryAction(Int32T processType, LcsErrorOccurredEventT lcsErrorEvent)
{
    Int8T ret = 0;

    NNLOG(LOG_DEBUG, "Error Report From [%d] Error[%d] Recovery[%d]\n",
            processType, lcsErrorEvent.error, lcsErrorEvent.recoveryAction);

    fprintf(stdout, "Error Report From [%d] Error[%d] Recovery[%d]\n",
            processType, lcsErrorEvent.error, lcsErrorEvent.recoveryAction);

    // 해당 프로세스의 에러상태값 변경
    LCS_INFO(processType).error = lcsErrorEvent.error;

    // 에러 이벤트 전송
    fprintf(stdout, "\n\nEvent Publish [Error]\n\n");
    eventPublish(EVENT_LCS_COMPONENT_ERROR_OCCURRED,
                &lcsErrorEvent, sizeof(LcsErrorOccurredEventT));

    delAllTimer(processType);

    // Process 의 재시작 시간, 에러내용을 입력
    nnTimeGetCurrent(LCS_INFO(processType).lastStartTime);
    LCS_INFO(processType).lastError = LCS_INFO(processType).error;
    LCS_INFO(processType).lastRecovery = lcsErrorEvent.recoveryAction;

    fprintf(stdout, "^^^^^^^^^^^DO RECOVERY^^^^^^^^^^^^^^^^^^^^^^^ RestartTime : %s, Action : %d\n", LCS_INFO(processType).lastStartTime, LCS_INFO(processType).lastRecovery);

    // Process 의 상태가 정상인지 확인
    if (LCS_INFO(processType).error != LCS_OK)
    {
        forcefulRestart(processType);
    }
    // Process 의 상태가 정상일 때, Recovery Action 수행
    else
    {
        // 받은 내용을 토대로 리커버리 액션 수행(RESTART)
        if (lcsErrorEvent.recoveryAction == (Int32T)LCS_COMPONENT_RESTART)
        {
            ret = sendTerminateMessage(processType);

            fprintf(stdout, "TERMINATE Message Send [%d]\n",
                    processType);

            if (ret == SUCCESS)
            {
#if 1
    // 생각 좀 해보기
                fprintf(stdout, "ADD TERMINATE TIMER : Recovery [%d]\n", processType);
                /* Timeout Timer 등록 */
                LCS_RESPONSE_TIMER(processType) =
                            timerAdd(processType,
                                    LCS_RESPONSE_TIMEOUT(processType),
                                    0,
                                    (void *)timeoutTimerCallback,
                                    &LCS_INFO(processType));
#endif
            }
        }
    }

    return ret;
}

void forcefulRestart(Int32T processType)
{
    LcsErrorOccurredEventT lcsErrorEvent = {0,};
    Int32T status = 0;
    Int32T pid = 0;

    sendSignaltoPid(LCS_INFO(processType).pid, SIGKILL);

    fprintf(stdout, "Child Process End [%d][Wait]\n", LCS_INFO(processType).pid);
    wait(&status);
    fprintf(stdout, "Child Process End [%d][%d]\n", LCS_INFO(processType).pid, status);

    delAllTimer(processType);

    // LCS_RUN, LCS_QUIESCING 일 경우만 상태 보내기
    if (gpProcMgrBase->status == LCS_RUN || gpProcMgrBase->status == LCS_QUIESCING)
    {
        // 해당 프로세스의 상태 제거 후 Event 전송
        if (LCS_STATUS.serviceStatus & (1 << processType))
        {
            LCS_STATUS.serviceStatus ^= (1 << processType);
        }

        fprintf(stdout, "Event Publish ^=\n");
        eventPublish(EVENT_LCS_COMPONENT_SERVICE_STATUS,
                    &LCS_STATUS, sizeof(LcsServiceStatusEventT));
    }

    // 에러 이벤트 전송
    lcsErrorEvent.processType = processType;
    lcsErrorEvent.error = LCS_INFO(processType).error;
    lcsErrorEvent.recoveryAction = LCS_COMPONENT_RESTART;

    fprintf(stdout, "Event Publish [Error]\n");

    eventPublish(EVENT_LCS_COMPONENT_ERROR_OCCURRED,
                &lcsErrorEvent, sizeof(LcsErrorOccurredEventT));

    // LCS_INIT, LCS_RUN 일 때만 수행
    if (gpProcMgrBase->status == LCS_INIT || gpProcMgrBase->status == LCS_RUN
        || gpProcMgrBase->status == LCS_QUIESCING)
    {
        // 해당 프로세스 fork()
        if ((pid = startProcess(LCS_INFO(processType).binName)) == FAILURE)
        {
            fprintf(stdout, "Run Process Error - [%s]\n",
                    LCS_INFO(processType).binName);

            /* Process 의 실행이 실패했다면 Process Manager 종료 */
            sendSignaltoPid(LCS_INFO(LCS_PROCESS_MANAGER).pid, SIGTERM);

            exit(0);
        }
        else
        {
            fprintf(stdout, "Run Process - [%s][%d][%d]\n",
                    LCS_INFO(processType).binName,
                    processType, pid);

            fprintf(stdout, "ADD FORCEFUL RESPONSE TIMER [%d]\n", processType);
            LCS_RESPONSE_TIMER(processType) =
                    timerAdd(processType,
                            LCS_RESPONSE_TIMEOUT(processType),
                            0,
                            (void *)timeoutTimerCallback,
                            &LCS_INFO(processType));

            nnTimeGetCurrent(LCS_INFO(processType).startTime);

            LCS_INFO(processType).pid = pid;
        }

        sleep(1);
    }
}

/* Reload 시 호출되는 Timer 콜백함수 */
void reloadTimerCallback(Int32T fd, Int16T event, void *pArg)
{
    Int8T sCmd[24] = {0,};
    Int8T ret = 0;

    NNLOG(LOG_DEBUG, "System Reboot\n");
    fprintf(stdout, "System Reboot\n");

    sprintf(sCmd, "reboot");

    NNLOG(LOG_DEBUG, "--> : %s\n", sCmd);
    fprintf(stdout, "--> : %s\n", sCmd);

    ret = system(sCmd);

    if (ret == FAILURE)
    {
        NNLOG(LOG_DEBUG, "System Reboot Failure\n");
        fprintf(stderr, "System Reboot Failure\n");
    }
    else
    {
        NNLOG(LOG_DEBUG, "System Reboot Success\n");
        fprintf(stderr, "System Reboot Success\n");
    }
}
