/*345678901234567890123456789012345678901234567890123456789012345678901234567890*/
/*********************************************************************************
 *               Electronics and Telecommunications Research Institute
 * Filename : 
 * Blockname: 
 * Overview : 
 * Creator  : 
 * Owner    : 
 * Copyright: 2013 Electronics and Telecommunications Research Institute. 
 *            All rights reserved. No part of this software shall be reproduced, 
 *            stored in a retrieval system, or transmitted by any means, 
 *            electronic, mechanical, photocopying, recording, or otherwise, 
 *            without written permission from ETRI.
 *********************************************************************************/

/*********************************************************************************
 *                        VERSION CONTROL  INFORMATION
 * $Author$
 * $Date$
 * $Revision
 * $Log$ 
 *********************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "taskManager.h"
#include "nnCmdDefines.h"

#include "processmgrMain.h"
#include "processmgrUtility.h"

extern void nosTaskLibInitialize();
extern void nosTaskScheduleInitialize();
extern void nosCompInitialize();
extern void nosCmdInitialize(Int32T cmshID);
extern void nosTaskModuleExit();

#define COMP_NAME "procmgr"
#define COMP_VER "0.0.1"

Int8T sProcMgrBinName[64] = {0,}; /* Process Manager 파일이름 */
Int32T ProcMgrPid = 0;            /* Process Manager PID */

Int8T startIpcMgr();    /* IPC Manager 기동하는 함수 */
Int8T startCM();        /* CM 기동하는 함수 */

void initDaemon();      /* Process Maanger 를 Daemon 으로 기동하는 함수 */

/* procmgr.conf 파일에서 Binary 이름을 가져오는 함수 */
Int8T getBinNamefromFile(StringT binName, Int32T processType);

/*
extern void nosCmshProcess();
extern void nosCompProcess();
extern void nosCompCmdInit();

void *gTinerTask = NULL;

void 
timerCallback(Int32T fd, Int16T event, void * arg)
{
  nosCompCmdInit(IPC_POL_MGR, IPC_CM_MGR, 
          nosCmshProcess, nosCompProcess);
  _taskTimerDel(gTimerTask);
}
*/
Int32T main(Int32T argc, char **argv)
{
    Int8T ret = 0;

    strcpy(sProcMgrBinName, argv[0]);

    fprintf(stdout, "Process Manager[%s] PID[%d]\n", argv[0], ProcMgrPid);

    // initialize dynamic module & task lib 
    nosTaskLibInitialize(COMP_NAME, COMP_VER);

    // create nos main task
    taskCreate(nosCompInitialize);

//    initDaemon();

    if (daemon(1, 0) == -1)
    {
        nLOG(LOG_DEBUG, "DAEMON ERROR : %s\n", strerror(errno));
        printf("daemon error: %s", strerror(errno));
    }
    else
    {
        nLOG(LOG_DEBUG, "DAEMON SUCCESS\n");
    }

    ProcMgrPid = getPidfromName(argv[0]);

    fprintf(stdout, "Start IPC Manager\n");
    nLOG(LOG_DEBUG, "Start IPC Manager\n");

    // IPC Manager Open
    ret = startIpcMgr();

    if (ret == FAILURE)
    {
        nLOG(LOG_DEBUG, "Start IPC Manager Failure\n");
        sendSignaltoPid(ProcMgrPid, SIGKILL);

        return FAILURE;
    }

    fprintf(stdout, "Start IPC Manager Success\n");
    nLOG(LOG_DEBUG, "Start IPC Manager Success\n");
    sleep(1);

    // schedule work(ipc/event/timer) 
    nosTaskScheduleInitialize();
    nLOG(LOG_DEBUG, "Init Nos Task Schedule Success\n");

#if 0
    // CM Open
    ret = startCM();

    if (ret == FAILURE)
    {
        sendSignaltoPid(ProcMgrPid, SIGKILL);

        return FAILURE;
    }

    fprintf(stdout, "Start Command Manager Success\n");
    sleep(1);

    // CM IPC 
 //   nosCmdInitialize(IPC_PRO_MGR);
//    fprintf(stdout, "Process Manager Dispatch\n");
#endif

   // Task start
    taskDispatch();
    fprintf(stdout, "Process Manager Main End\n");
    nLOG(LOG_DEBUG, "Process Manager Main End\n");

    nosTaskModuleExit();

    return(0);
}

/* IPC Manager 기동하는 함수 */
Int8T startIpcMgr()
{
    Int8T binName[30] = {0,};
    Int32T ret = 0;

    ret = getBinNamefromFile(binName, IPC_MANAGER);
    nLOG(LOG_DEBUG, "getBinNameformFile Result [%d]\n", ret);

    if (ret == FAILURE)
    {
        return FAILURE;
    }

    ret = startProcess(binName);
    nLOG(LOG_DEBUG, "startProcess Result [%d]\n", ret);

    if (ret == 0)
    {
        return FAILURE;
    }
    fprintf(stdout, "IPC MANAGER PID [%d]\n", ret);
    nLOG(LOG_DEBUG, "IPC MANAGER PID [%d]\n", ret);

//    nnTimeGetCurrent(LCS_INFO(LCS_IPC_MANAGER).startTime);

    return SUCCESS;
}

#if 0
/* CM 기동하는 함수 */
Int8T startCM()
{
    Int8T binName[30] = {0,};
    Int32T ret = 0;

    ret = getBinNamefromFile(binName, COMMAND_MANAGER);

    if (ret == FAILURE)
    {
        return FAILURE;
    }

    ret = startProcess(binName);

    if (ret == 0)
    {
        return FAILURE;
    }
    fprintf(stdout, "COMMAND MANAGER PID [%d]\n", ret);

//    nnTimeGetCurrent(LCS_INFO(LCS_COMMAND_MANAGER).startTime);

    return SUCCESS;
}
#endif

/* 구동할 Process 의 정보를 라인별로 읽는 함수 */
Int8T getBinNamefromFile(StringT binName, Int32T processType)
{
    FILE *fp = NULL;
    Int8T sBuffer[256] = {0,};
    Int8T sTempProcess[10] = {0,};
    Int32T tempType = 0;
    Int8T ret = FAILURE;

    fp = fopen(LCS_PROCESS_MANAGER_CONF, "r");
    if (fp == NULL)
    {
        return ret;
    }

    /* Line 수 얻기 */
    while (fgets(sBuffer, 255, fp))
    {
        if (strstr(sBuffer, "#") != NULL)
        {
            continue;
        }
    }

    /* 처음으로 가서 Data 저장 */
    fseek(fp, 0L, SEEK_SET);

    while (fgets(sBuffer, 255, fp))
    {
        if (strstr(sBuffer, "#") != NULL)
        {
            continue;
        }

        sscanf(sBuffer, "%s %d %s",
               sTempProcess, &tempType, binName);

        if (tempType == processType)
        {
            fprintf(stdout, "Get BinName from File SUCCESS\n");

            ret = SUCCESS;

            break;
        }

        memset(sBuffer, 0, sizeof(sBuffer));
    }

    fclose(fp);
    fp = NULL;

    return ret;
}

void initDaemon()
{
    int fd = 0;
    pid_t pid, sid;

    pid = fork();
    if (pid < 0)
    {
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }

    sid = setsid();
    if (sid < 0)
    {
        exit(EXIT_FAILURE);
    }

    if ((fd = open("/dev/null", O_RDWR, 0)) >= 0)
    {
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        (void)dup2(fd, STDIN_FILENO);
        (void)dup2(fd, STDOUT_FILENO);
        (void)dup2(fd, STDERR_FILENO);

        nLOG(LOG_DEBUG, "FD CLOSE\n");
    }
}
