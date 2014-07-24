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
 * @brief :  Process 들을 관리하는 Manager
 * - Block Name : Process Manager
 * - Process Name : procmgr
 * - Creator : Changwoo Lee
 * - Initial Date : 2014/3/03
 */

/**
* @file : processmgrMain.c
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

#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "nnTypes.h"
#include "nnTime.h"

#include "nosLib.h"

#include "nnCmdDefines.h"
#include "nnCompProcess.h"

#include "processmgrMain.h"
#include "processmgrIpc.h"
#include "processmgrUtility.h"

/*******************************************************************************
 *                              GLOBAL VARIABLES
 ******************************************************************************/
extern void **gCompData;
extern LcsProcessMgrT *gpProcMgrBase;

/*******************************************************************************
 *                         GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/
extern Int32T procMgrWriteConfCB (struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T getPidfromName(StringT name);
extern Int8T sProcMgrBinName[64];

/*******************************************************************************
 *                         LOCAL CONSTANTS/LITERALS/TYPES
 ******************************************************************************/

/*******************************************************************************
*                               LOCAL VARIABLES
*******************************************************************************/
Int32T typeSeq[MAX_PROCESS_CNT - 1] = {0,}; /* Process 기동 순서 */
Int32T procTerminateCnt = 0;

void testTimerCallback(Int32T fd, Int16T event, void *pArg);

/*******************************************************************************
 *                                   CODE
 ******************************************************************************/
void procInitProcess(void) //initialize component
{
    Int32T ret = 0;

//    nnLogSetFlag(LOG_STDOUT);

    NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);

    fprintf(stdout, "======================\n");
    fprintf(stdout, "Init Process Manager Start[%d]\n", getpid());
    fprintf(stdout, "======================\n");

    /** Component Global Variable Memory Allocate */
    gpProcMgrBase = NNMALLOC(MEM_GLOBAL, sizeof(LcsProcessMgrT));

    /** Component Command Init **/
//    gpProcMgrBase->gpCmshGlobal = compCmdInit(IPC_PRO_MGR, procMgrWriteConfCB);

    /* == Process Manager 설정 시작 == */
    /* Process Manager 의 run 상태 변경 */
    nnTimeGetCurrent(LCS_INFO(LCS_PROCESS_MANAGER).startTime);

    /* Process Manager 의 run 상태 변경 */
    LCS_INFO(LCS_PROCESS_MANAGER).run = TRUE;

    /* Process Manager 의 Process Type 저장 */
    LCS_INFO(LCS_PROCESS_MANAGER).processType = LCS_PROCESS_MANAGER;

    /* Process Manager 의 IPC Type 저장 */
    LCS_INFO(LCS_PROCESS_MANAGER).ipcType = PROCESS_MANAGER;

    /* Process Manager 의 PID 저장 */
    LCS_INFO(LCS_PROCESS_MANAGER).pid = getpid();

    /* Process Manager 의 Bin Name 저장 */
    getNamefromPid(LCS_INFO(LCS_PROCESS_MANAGER).binName, getpid());

    /* Process Manager 의 Error 관련 값 저장 */
    LCS_INFO(LCS_PROCESS_MANAGER).error = LCS_OK;
    LCS_INFO(LCS_PROCESS_MANAGER).lastError = LCS_OK;
    LCS_INFO(LCS_PROCESS_MANAGER).recovery = LCS_NO_RECOMMENDED;
    LCS_INFO(LCS_PROCESS_MANAGER).lastRecovery = LCS_NO_RECOMMENDED;

    /* Process Manager 의 Timer 관련 값 NULL 저장 */
    delAllTimer(LCS_PROCESS_MANAGER);

    /* == Process Manager 설정 끝 == */

    /* Process Manager 의 구조체 및 Process 들을 기동 */
    ret = initProcmgr();
    if (ret == FAILURE)
    {
        fprintf(stdout, "initProcmgr() Failure\n");
        exit(-1);
    }

    /* IPC Manager 시작시간 입력 */
    nnTimeGetCurrent(LCS_INFO(LCS_IPC_MANAGER).startTime);

#if 0
    /* Command Manager 시작시간 입력 */
    nnTimeGetCurrent(LCS_INFO(LCS_COMMAND_MANAGER).startTime);
#endif

    /* Assign shared memory pointer to global memory pointer. */
    (*gCompData) = (void *)gpProcMgrBase;

    /*
     * Child 가 종료될 때 알림을 받도록
     * SIGNAL CHILD Signal 을 받도록 등록
     */
    taskSignalSet(SIGCHLD);

    fprintf(stdout, "======================\n");
    fprintf(stdout, "Init Process Manager Complete\n");
    fprintf(stdout, "======================\n");
}

void procTermProcess(void) // terminate component
{
    Int32T status = 0;

    fprintf(stdout, "======================\n");
    fprintf(stdout, "Term Process Manager Start\n");
    fprintf(stdout, "======================\n");

    NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);

    /** Component Command Close */
    compCmdFree(gpProcMgrBase->gpCmshGlobal);

    // IPC MANAGER 에게 전송
    sendSignaltoPid(LCS_INFO(LCS_IPC_MANAGER).pid, SIGTERM);
    sleep(1);

    // IPC MANAGER 종료 확인
    NNLOG(LOG_DEBUG, "Check - IPC Manager End\n");
    wait(&status);
    NNLOG(LOG_DEBUG, "Check - IPC Manager End OK\n");

    /** Component Global Variable Memory Free */
    NNFREE(MEM_GLOBAL, gpProcMgrBase);

    /** System Library Close */
    taskSignalDelAggr();
    taskClose();
    nnLogClose();
    memClose();

    fprintf(stdout, "======================\n");
    fprintf(stdout, "Term Process Manager Complete\n");
    fprintf(stdout, "======================\n");
}

void procHoldProcess(void)  // hold component
{
    fprintf(stdout, "======================\n");
    fprintf(stdout, "Hold Process Manager Start\n");
    fprintf(stdout, "======================\n");

    NNLOG(LOG_DEBUG, "Enter PROCESS :: %s\n", __func__);  

    /* 재시작 스케쥴 삭제 */
//    taskTimerDel(gpProcMgrBase->pReloadTimer);
//    gpProcMgrBase->pReloadTimer = NULL;

    fprintf(stdout, "======================\n");
    fprintf(stdout, "Hold Process Manager Complete\n");
    fprintf(stdout, "======================\n");

    NNLOG(LOG_DEBUG, "Exit PROCESS :: %s\n", __func__);  
}

void procRestartProcess(void) // restart component
{
    struct timeval updateTv = {0,};
    Int32T procIndex = 0;

    fprintf(stdout, "======================\n");
    fprintf(stdout, "Restart Process Manager Start\n");
    fprintf(stdout, "======================\n");

    NNLOG(LOG_DEBUG, "enter %s\n", __FUNCTION__);
    fprintf(stdout, "Enter PROCESS :: %s\n", __func__);  

    /* Re-assign shared memory pointer to global memory pointer. */
    NNLOG(LOG_DEBUG, "Re-assign Shared Memory\n");  
    gpProcMgrBase = (LcsProcessMgrT *)(*gCompData);

    NNLOG(LOG_DEBUG, "compCmdUpdate\n");
    /** Component Command Restart **/
    compCmdUpdate(gpProcMgrBase->gpCmshGlobal, procMgrWriteConfCB);
  
    /** TODO : Add more here **/

    // Oam Printf Update
    // polMgrOamTimerUpdate();

#if 0
    NNLOG(LOG_DEBUG, "Update Reload Timer\n");
    /* Reload Timer Update */
    taskTimerDel(gpProcMgrBase->pReloadTimer);
    gpProcMgrBase->pReloadTimer = NULL;

    gpProcMgrBase->pReloadTimer =
       taskTimerUpdate(reloadTimerCallback,
                        gpProcMgrBase->pReloadTimer,
                        updateTv, NULL);
#endif

    NNLOG(LOG_DEBUG, "Update Process Timer\n");  
    /* Procee 의 Timer Update */
    for (procIndex = 0; procIndex < MAX_PROCESS_CNT - 1; ++procIndex)
    {
        if (procIndex != LCS_IPC_MANAGER &&
            procIndex != LCS_PROCESS_MANAGER &&
            LCS_INFO(procIndex).run == TRUE)
        {
            /* Active Health Check 에 대한 Timer */
            if (LCS_HEALTH_ACTIVE_TIMER(procIndex) != NULL)
            {
                SET_TV(updateTv, LCS_HEALTH_INTERVAL(procIndex));

                LCS_HEALTH_ACTIVE_TIMER(procIndex) =
                    taskTimerUpdate(healthActiveTimerCallback,
                                    LCS_HEALTH_ACTIVE_TIMER(procIndex),
                                    updateTv, &LCS_INFO(procIndex));
            }

            /* Passive Health Check 에 대한 Timer */
            if (LCS_HEALTH_PASSIVE_TIMER(procIndex) != NULL)
            {
                SET_TV(updateTv, LCS_HEALTH_INTERVAL(procIndex));

                LCS_HEALTH_PASSIVE_TIMER(procIndex) =
                    taskTimerUpdate(healthPassiveTimerCallback,
                                    LCS_HEALTH_PASSIVE_TIMER(procIndex),
                                    updateTv, &LCS_INFO(procIndex));
            }

            /* Health Check Response 에 대한 Timer */
            if (LCS_HEALTH_RESPONSE_TIMER(procIndex) != NULL)
            {
                SET_TV(updateTv, LCS_HEALTH_TIMEOUT(procIndex));

                LCS_HEALTH_RESPONSE_TIMER(procIndex) =
                    taskTimerUpdate(timeoutTimerCallback,
                                    LCS_HEALTH_RESPONSE_TIMER(procIndex),
                                    updateTv, &LCS_INFO(procIndex));
            }

            /* Response 에 대한 Timer */
            if (LCS_RESPONSE_TIMER(procIndex) != NULL)
            {
                SET_TV(updateTv, LCS_RESPONSE_TIMEOUT(procIndex));

                LCS_RESPONSE_TIMER(procIndex) =
                    taskTimerUpdate(timeoutTimerCallback,
                                    LCS_RESPONSE_TIMER(procIndex),
                                    updateTv, &LCS_INFO(procIndex));
            }

            /* Quiescing 에 대한 Timer */
            if (LCS_QUIESCING_TIMER(procIndex) != NULL)
            {
                SET_TV(updateTv, LCS_RESPONSE_TIMEOUT(procIndex));

                LCS_QUIESCING_TIMER(procIndex) =
                    taskTimerUpdate(timeoutTimerCallback,
                                    LCS_QUIESCING_TIMER(procIndex),
                                    updateTv, &LCS_INFO(procIndex));
            }
        }
    }

    NNLOG(LOG_DEBUG, "Print Process Info\n");  
    /* 기존의 정보 출력 */
    for (procIndex = 0; procIndex < MAX_PROCESS_CNT - 1; ++procIndex)
    {
        if (LCS_INFO(procIndex).run == TRUE)
        {
            fprintf(stdout, "=====================================\n");
            fprintf(stdout, "Process Info\n");
            fprintf(stdout, "Process BinName : %s\n", LCS_INFO(procIndex).binName);
            fprintf(stdout, "Process Start Time : %s\n", LCS_INFO(procIndex).startTime);
            fprintf(stdout, "Process Prev Start Time : %s\n", LCS_INFO(procIndex).lastStartTime);
            fprintf(stdout, "Process Component Version : %s\n", LCS_INFO(procIndex).version);
            fprintf(stdout, "Process Error : %d\n", LCS_INFO(procIndex).error);
            fprintf(stdout, "Process Prev Error : %d\n", LCS_INFO(procIndex).lastError);
            fprintf(stdout, "Process Memory Threshold : %d\n", LCS_ATTR(procIndex).memoryThreshold);
            fprintf(stdout, "Process Cpu Threshold : %d\n", LCS_ATTR(procIndex).cpuThreshold);
            fprintf(stdout, "Process PID : %d\n", LCS_INFO(procIndex).pid);
            fprintf(stdout, "=====================================\n");

            fprintf(stdout, "======================\n");
            fprintf(stdout, "Restart Process Manager Complete\n");
            fprintf(stdout, "======================\n");
        }
    }

    NNLOG(LOG_DEBUG, "Exit PROCESS :: %s\n", __func__);  
}

void procSignalProcess(Int32T sigType)  // signal catch proces
{
    NNLOG(LOG_DEBUG, "Enter PROCESS :: %s\n", __func__);  

    NNLOG(LOG_DEBUG, "################################\n");
    NNLOG(LOG_DEBUG, "  Signal(%d) Occured !!!\n", sigType);
    NNLOG(LOG_DEBUG, "################################\n");

    fprintf(stdout, "################################\n");
    fprintf(stdout, "  Signal(%d) Occured !!!\n", sigType);
    fprintf(stdout, "################################\n");

    if (sigType == SIGINT)
    {
        gpProcMgrBase->status = LCS_STOP;
    }

    if (sigType == SIGCHLD)
    {
        Int32T pid = 0;
        Int32T status = 0;
        Int32T sigIndex = 0;
        Int32T ret = 0;

        NNLOG(LOG_DEBUG, "########## Sig Child Occured ##########\n");
        fprintf(stdout, "########## Sig Child Occured ##########\n");
#if 1
        /*
         * procmgr.conf 에서 읽은 데이터를 사용하여
         * IPC Manager 를 제외하고 Process 들의 종료를 검사
         */
        for (sigIndex = LCS_SEND_SIGNAL;
                sigIndex < gpProcMgrBase->runCount;
                ++sigIndex)
        {
            if (LCS_INFO(typeSeq[sigIndex]).run == TRUE)
            {
                NNLOG(LOG_DEBUG, "Check Child Process End : %s\n",
                                LCS_INFO(typeSeq[sigIndex]).binName);
                printf("Check Child Process End : %s\n",
                                LCS_INFO(typeSeq[sigIndex]).binName);

                ret = waitpid(LCS_INFO(typeSeq[sigIndex]).pid,
                                &status, WNOHANG);

                /* Process 가 종료 된걸 확인 */
                if (ret > 0)
                {
                    /* 등록된 Timer 모두 제거 */
                    delAllTimer(LCS_INFO(typeSeq[sigIndex]).processType);

                    /* 해당 프로세스의 상태 제거 후 Event 전송 */
                    if (LCS_STATUS.serviceStatus &
                                (1 << LCS_INFO(typeSeq[sigIndex]).processType))
                    {
                        LCS_STATUS.serviceStatus ^=
                                (1 << LCS_INFO(typeSeq[sigIndex]).processType);
                    }

                    NNLOG(LOG_DEBUG, "%s is End - Event Publish ^=\n",
                                    LCS_INFO(typeSeq[sigIndex]).binName);
                    fprintf(stdout, "%s is End - Event Publish ^=\n",
                                    LCS_INFO(typeSeq[sigIndex]).binName);
                    eventPublish(EVENT_LCS_COMPONENT_SERVICE_STATUS,
                                &LCS_STATUS, sizeof(LcsServiceStatusEventT));

                    if (gpProcMgrBase->status == LCS_INIT ||
                        gpProcMgrBase->status == LCS_RUN ||
                        gpProcMgrBase->status == LCS_QUIESCING)
                    {
                        if ((pid = startProcess(LCS_INFO(typeSeq[sigIndex]).binName)) == FAILURE)
                        {
                            fprintf(stdout, "Re-Run Process Error - [%s]\n",
                                    LCS_INFO(typeSeq[sigIndex]).binName);
                            NNLOG(LOG_DEBUG, "Re-Run Process Error - [%s]\n",
                                    LCS_INFO(typeSeq[sigIndex]).binName);

                            /* Process 의 실행이 실패했다면 Process Manger 종료 */
                            sendSignaltoPid(LCS_INFO(LCS_PROCESS_MANAGER).pid, SIGTERM);

                            exit(0);
                        }
                        else
                        {
                            fprintf(stdout, "Re-Run Process - [%s][%d][%d]\n",
                                    LCS_INFO(typeSeq[sigIndex]).binName,
                                    typeSeq[sigIndex], pid);
                            NNLOG(LOG_DEBUG, "Re-Run Process - [%s][%d][%d]\n",
                                    LCS_INFO(typeSeq[sigIndex]).binName,
                                    typeSeq[sigIndex], pid);

                            LCS_RESPONSE_TIMER(typeSeq[sigIndex]) =
                                timerAdd(typeSeq[sigIndex],
                                    LCS_RESPONSE_TIMEOUT(typeSeq[sigIndex]),
                                    0,
                                    (void *)timeoutTimerCallback,
                                    &LCS_INFO(typeSeq[sigIndex]));

                            nnTimeGetCurrent(
                                        LCS_INFO(typeSeq[sigIndex]).startTime);

                            LCS_INFO(typeSeq[sigIndex]).pid = pid;
                        }
                    }
                    else if (gpProcMgrBase->status == LCS_STOPING)
                    {
                        ++procTerminateCnt;
                        NNLOG(LOG_DEBUG, "Terminate CNT : %d\n",
                                        procTerminateCnt);
                        fprintf(stdout, "Terminate CNT : %d\n",
                                        procTerminateCnt);
//                        fprintf(stdout, "TERMINATE %d\n", gpProcMgrBase->runCount - LCS_SEND_SIGNAL);

                        if ((gpProcMgrBase->runCount - LCS_SEND_SIGNAL)
                                == procTerminateCnt)
                        {
                            NNLOG(LOG_DEBUG,
                                "ALL Process is Terminated(LCS_STOP)\n");
                            fprintf(stdout,
                                "ALL Process is Terminated(LCS_STOP)\n");
                            gpProcMgrBase->status = LCS_STOP;

                            procTerminateCnt = 0;
                        }
                    }

//                    sleep(1);
                }
            }
        }
#endif
    }
    else
    {
        NNLOG(LOG_DEBUG, "Do Close Process Manager\n");  

        closeProcmgr();
    }

    NNLOG(LOG_DEBUG, "Exit PROCESS :: %s\n", __func__);  

    fprintf(stdout, "======================\n");
    fprintf(stdout, "Signal Process Manager Complete\n");
    fprintf(stdout, "======================\n");
}

/* Process Manager 의 시작시 호출되는 함수 */
Int8T initProcmgr()
{
    struct timeval initTv = {0,};
    Int8T ret = 0;

    /* gpProcMgrBase 초기화 */
    gpProcMgrBase->reloadTime = 0;
    gpProcMgrBase->status = LCS_INIT;
    gpProcMgrBase->runCount = 0;
    gpProcMgrBase->haState = LCS_HA_NONE;
    LCS_STATUS.serviceStatus = 0;
    gpProcMgrBase->pReloadTimer = NULL;

    /* procmgr.conf 에서 실행할 Process 들 읽어오기 */
    ret = readProcConf();
    if (ret != SUCCESS)
    {
        fprintf(stdout, "Process Manager Init Failed\n");
        return FAILURE;
    }

    fprintf(stdout, "ProcessManager's Run Count is [%d]\n", gpProcMgrBase->runCount);
    fprintf(stdout, "ProcessManager's Initialize is Completed !!\n\n");

    /* Init Timer 설정 */
    SET_TV(initTv, FIVE_SEC);
    taskTimerSet((void *)initTimerCallback, initTv, 0, NULL);

    return SUCCESS;
}

/* Process Manager 의 종료시 호출되는 함수 */
void closeProcmgr()
{
    struct timeval closeTv = {0, 0};

    Int32T ret = 0;
    Int32T procIndex = 0;
    Int32T processType = 0;

    /* Process 들에게 종료 메시지 전송하기 */
    NNLOG(LOG_DEBUG, "End Process\n");  
    fprintf(stdout, "End Process\n");

    gpProcMgrBase->status = LCS_STOPING;

    for (procIndex = gpProcMgrBase->runCount - 1;
        procIndex > (LCS_SEND_SIGNAL - 1); --procIndex)
    {
        if (LCS_INFO(typeSeq[procIndex]).run == TRUE)
        {
            processType = LCS_INFO(typeSeq[procIndex]).processType;

            if (LCS_STATUS.serviceStatus & (1 << processType))
            {
                NNLOG(LOG_DEBUG, "Send Terminate Message to [P:%d][I:%d]\n",
                        processType, LCS_INFO(processType).ipcType);
                ret = sendTerminateMessage(processType);

                fprintf(stdout, "Send Terminate Message to [P:%d][I:%d] Result[%d]\n",
                        processType, LCS_INFO(processType).ipcType, ret);

                // Healthcheck Active Interval Timer 삭제
                timerDel(processType, LCS_HEALTH_ACTIVE_TIMER(processType));
                LCS_HEALTH_ACTIVE_TIMER(processType) = NULL;

                // Healthcheck Passive Interval Timer 삭제
                timerDel(processType, LCS_HEALTH_PASSIVE_TIMER(processType));
                LCS_HEALTH_PASSIVE_TIMER(processType) = NULL;

                // Healthcheck Response Timer 삭제
                timerDel(processType, LCS_HEALTH_RESPONSE_TIMER(processType));
                LCS_HEALTH_RESPONSE_TIMER(processType) = NULL;
            }
            else
            {
                NNLOG(LOG_DEBUG, "Send SIGTERM to [P:%d][%d]\n", processType,
                                 LCS_INFO(processType).pid);
                sendSignaltoPid(LCS_INFO(processType).pid, SIGTERM);

//                delAllTimer(processType);
            }

//            sleep(1);
        }
    }

    sleep(1);

    if (LCS_STATUS.serviceStatus == 0)
    {
        NNLOG(LOG_DEBUG, "Do procTermProcess()\n");
        fprintf(stdout, "Do procTermProcess()\n");
        procTermProcess(); // terminate component

        sleep(1);
    }
    else
    {
        NNLOG(LOG_DEBUG, "Set closeTimerCallback Timer\n");

        /* Process 종료 Timer 설정 */
        SET_TV(closeTv, FIVE_SEC);
        taskTimerSet((void *)closeTimerCallback, closeTv, 0, NULL);
        fprintf(stdout, "Set Close Timer\n");
    }
}

/* Process 의 시작을 위한 Timer 콜백함수 */
void initTimerCallback(Int32T fd, Int16T event, void *pArg)
{
    Int32T pid = 0;
    Int32T procIndex = 0;

    /* procmgr.conf 에서 읽은 데이터를 사용하여 Process 들을 실행하기 */
    printf("Start Process\n");

    /*
     * procmgr.conf 에서 읽은 데이터를 사용하여
     * IPC Manager, Command Manager 를 제외하고 Process 들을 실행하기
     */
    for (procIndex = 1; procIndex < gpProcMgrBase->runCount; ++procIndex)
    {
        if (LCS_INFO(typeSeq[procIndex]).run == TRUE)
        {
            printf("\n\nRunProcess : %s\n", LCS_INFO(typeSeq[procIndex]).binName);

            if ((pid = startProcess(LCS_INFO(typeSeq[procIndex]).binName)) == FAILURE)
            {
                fprintf(stdout, "Run Process Error - [%s]\n",
                                LCS_INFO(typeSeq[procIndex]).binName);
                NNLOG(LOG_DEBUG, "Run Process Error - [%s]\n",
                                LCS_INFO(typeSeq[procIndex]).binName);

                /* Process 의 실행이 실패했다면 Process Manger 종료 */
                sendSignaltoPid(LCS_INFO(LCS_PROCESS_MANAGER).pid, SIGTERM);

                exit(0);
            }
            else
            {
                fprintf(stdout, "Run Process - [%s][%d][%d]\n",
                        LCS_INFO(typeSeq[procIndex]).binName,
                        typeSeq[procIndex], pid);
                NNLOG(LOG_DEBUG, "Run Process - [%s][%d][%d]\n",
                        LCS_INFO(typeSeq[procIndex]).binName,
                        typeSeq[procIndex], pid);

                LCS_RESPONSE_TIMER(typeSeq[procIndex]) =
                    timerAdd(typeSeq[procIndex],
                            LCS_RESPONSE_TIMEOUT(typeSeq[procIndex]),
                            0,
                            (void *)timeoutTimerCallback,
                            &LCS_INFO(typeSeq[procIndex]));

                nnTimeGetCurrent(LCS_INFO(typeSeq[procIndex]).startTime);
                LCS_INFO(typeSeq[procIndex]).pid = pid;
            }

//            sleep(1);
        }
    }

    printf("ProcessManager's Initialize Callback is Completed !!\n\n");

    return;
}

/* Process 의 종료를 위한 Timer 콜백함수 */
void closeTimerCallback(Int32T fd, Int16T event, void *pArg)
{
    struct timeval closeTimerTv = {0, 0};

    NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);

    fprintf(stdout, "======================\n");
    fprintf(stdout, "PROCESS MANAGER CLOSE\n");
    fprintf(stdout, "PROCESS MANAGER State [%d]\n", gpProcMgrBase->status);
    fprintf(stdout, "======================\n");

    if (gpProcMgrBase->status == LCS_STOP)
    {
        NNLOG(LOG_DEBUG, "Do procTermProcess()\n");

        fprintf(stdout, "Do procTermProcess()\n");
        procTermProcess(); // terminate component
    }
    else
    {
        NNLOG(LOG_DEBUG, "Retier closeTimerCallback\n");

        fprintf(stdout, "Retire closeTimerCallback\n");
        /* Process 종료 Timer 설정 */
        SET_TV(closeTimerTv, THREE_SEC);
        taskTimerSet((void *)closeTimerCallback, closeTimerTv, 0, NULL);
    }
}

/* 구동할 Process 의 정보를 읽는 함수 */
Int8T readProcConf()
{
    FILE *fp = NULL;
    Int8T sBuffer[256] = {0,};
    Int8T sTempProcess[10] = {0,};
    Int32T tempType = 0;
    Int8T sTempBinary[256] = {0,};
    Int32T tempMemory = 0;
    Int32T tempCpu = 0;
    Int32T lineIndex = 0;

    fp = fopen(LCS_PROCESS_MANAGER_CONF, "r");
    if (fp == NULL)
    {
        return FAILURE;
    }

    /* Line 수 얻기 */
    while (fgets(sBuffer, 255, fp))
    {
        if (strstr(sBuffer, "#") != NULL)
        {
            continue;
        }

        ++gpProcMgrBase->runCount;
    }

    /* 처음으로 가서 Data 저장 */
    fseek(fp, 0L, SEEK_SET);

    while (fgets(sBuffer, 255, fp))
    {
        if (strstr(sBuffer, "#") != NULL)
        {
            continue;
        }

        sscanf(sBuffer, "%s %d %s %d %d",
               sTempProcess, &tempType, sTempBinary, &tempMemory, &tempCpu);

        typeSeq[lineIndex] = tempType;

        memset(sBuffer, 0, sizeof(sBuffer));

        LCS_INFO(tempType).processType = tempType;
        LCS_INFO(tempType).ipcType = 0;
        LCS_ATTR(tempType).defaultRR = LCS_COMPONENT_RESTART;
        LCS_ATTR(tempType).healthcheckInterval = DEFAULT_TIMEOUT;
        LCS_ATTR(tempType).healthcheckResponseTimeout = DEFAULT_TIMEOUT;
        LCS_ATTR(tempType).requestResponseTimeout = DEFAULT_TIMEOUT;
        LCS_ATTR(tempType).memoryThreshold = tempMemory;
        LCS_ATTR(tempType).cpuThreshold = tempCpu;

        LCS_INFO(tempType).run = TRUE;
        strcpy(LCS_INFO(tempType).binName, sTempBinary);
        memset(&LCS_INFO(tempType).startTime, 0,
                    sizeof(LCS_INFO(tempType).startTime));
        memset(&LCS_INFO(tempType).lastStartTime, 0,
                    sizeof(LCS_INFO(tempType).lastStartTime));
        memset(&LCS_INFO(tempType).version, 0,
                    sizeof(LCS_INFO(tempType).version));
        LCS_INFO(tempType).pid = getPidfromName(sTempBinary);
        LCS_INFO(tempType).error = LCS_OK;
        LCS_INFO(tempType).lastError = LCS_OK;
        LCS_INFO(tempType).recovery = LCS_NO_RECOMMENDED;
        LCS_INFO(tempType).lastRecovery = LCS_NO_RECOMMENDED;

        LCS_RESPONSE_TIMER(tempType) = NULL;
        LCS_QUIESCING_TIMER(tempType) = NULL;
        LCS_HEALTH_ACTIVE_TIMER(tempType) = NULL;
        LCS_HEALTH_PASSIVE_TIMER(tempType) = NULL;
        LCS_HEALTH_RESPONSE_TIMER(tempType) = NULL;

        fprintf(stdout, "=====================================\n");
        fprintf(stdout, "Process Info\n");
        fprintf(stdout, "Process BinName : %s\n", LCS_INFO(tempType).binName);
        fprintf(stdout, "Process Start Time : %s\n", LCS_INFO(tempType).startTime);
        fprintf(stdout, "Process Prev Start Time : %s\n", LCS_INFO(tempType).lastStartTime);
        fprintf(stdout, "Process Version : %s\n", LCS_INFO(tempType).version);
        fprintf(stdout, "Process Error : %d\n", LCS_INFO(tempType).error);
        fprintf(stdout, "Process Prev Error : %d\n", LCS_INFO(tempType).lastError);
        fprintf(stdout, "Process Memory Threshold : %d\n", LCS_ATTR(tempType).memoryThreshold);
        fprintf(stdout, "Process Cpu Threshold : %d\n", LCS_ATTR(tempType).cpuThreshold);
        fprintf(stdout, "Process PID : %d\n", LCS_INFO(tempType).pid);
        fprintf(stdout, "Type Sequence : %d\n", typeSeq[lineIndex]);
        fprintf(stdout, "=====================================\n");

        ++lineIndex;
    }

    fclose(fp);
    fp = NULL;

    return SUCCESS;
}

void delAllTimer(Int32T processType)
{
    // Reponse Timer 삭제
    NNLOG(LOG_DEBUG, "DEL RESPONSE(FORCEFUL) TIMER [%d]\n", processType);
    fprintf(stdout, "DEL RESPONSE(FORCEFUL) TIMER [%d]\n", processType);
    timerDel(processType, LCS_RESPONSE_TIMER(processType));
    LCS_RESPONSE_TIMER(processType) = NULL;

    // Healthcheck Active Timer 삭제
    NNLOG(LOG_DEBUG, "DEL ACTIVE HEALTHCHECK RESPONSE(FORCEFUL) TIMER [%d]\n", processType);
    fprintf(stdout, "DEL ACTIVE HEALTHCHECK RESPONSE(FORCEFUL) TIMER [%d]\n", processType);
    timerDel(processType, LCS_HEALTH_ACTIVE_TIMER(processType));
    LCS_HEALTH_ACTIVE_TIMER(processType) = NULL;

    // Healthcheck Passive Timer 삭제
    NNLOG(LOG_DEBUG, "DEL PASSIVE HEALTHCHECK RESPONSE(FORCEFUL) TIMER [%d]\n", processType);
    fprintf(stdout, "DEL PASSIVE HEALTHCHECK RESPONSE(FORCEFUL) TIMER [%d]\n", processType);
    timerDel(processType, LCS_HEALTH_PASSIVE_TIMER(processType));
    LCS_HEALTH_PASSIVE_TIMER(processType) = NULL;

    // Healthcheck Response Timer 삭제
    NNLOG(LOG_DEBUG, "DEL HEALTH RESPONSE(FORCEFUL) TIMER [%d]\n", processType);
    fprintf(stdout, "DEL HEALTH RESPONSE(FORCEFUL) TIMER [%d]\n", processType);
    timerDel(processType, LCS_HEALTH_RESPONSE_TIMER(processType));
    LCS_HEALTH_RESPONSE_TIMER(processType) = NULL;
}
