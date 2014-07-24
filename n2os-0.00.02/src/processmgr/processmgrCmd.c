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
 * @brief :  Process Manager 의 Command 와 관련된 파일
 * - Block Name : Process Manager
 * - Process Name : procmgr
 * - Creator : Changwoo Lee
 * - Initial Date : 2014/5/12
 */

/**
* @file : processmgrCmd.c
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

#include <stdio.h>
#include <stdlib.h>
#include "nnTypes.h"
#include "nnStr.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdInstall.h"
#include "processmgrMain.h"
#include "processmgrIpc.h"
#include "nnBuffer.h"

#include "lcsUtility.h"

/*******************************************************************************
 *                              GLOBAL VARIABLES
 ******************************************************************************/
extern LcsProcessMgrT *gpProcMgrBase;

/*******************************************************************************
 *                         GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/

/*******************************************************************************
 *                         LOCAL CONSTANTS/LITERALS/TYPES
 ******************************************************************************/
const Int8T sRecovery[][20] = {"No Recommended",
                                "Component Restart",
                                "Node Failover",
                                "Node Switchover",
                                "Node Failfast"};

const Int8T sError[][20] = {"",
                            "None",
                            "Library",
                            "Version",
                            "Init",
                            "Timeout",
                            "Try Again",
                            "Invalid Param",
                            "No Memory",
                            "Bad Handle",
                            "Busy",
                            "Access",
                            "Not Exist",
                            "Name Too Long",
                            "Exist",
                            "No Space",
                            "Interrupt",
                            "",
                            "No Resources",
                            "Not Supported",
                            "Bad Operation",
                            "Failed Operation",
                            "Message Error"};

const Int8T sRedundancy[][14] = {"NONE",
                                "SIMPLEX",
                                "RPR ACTIVE",
                                "RPR STANDBY",
                                "SSO ACTIVE",
                                "SSO STANDBY",
                                "QUIESCING",
                                "QUIESCED"};

const Int8T sProcessName[][10] = {"ipcmgrd",
                                    "procmgrd",
                                    "ribmgrd",
                                    "", // port interface manager
                                    "pold", // policy manager
                                    "cmd",
                                    "", // multicast rib manager
                                    "", // lib manager
                                    "", // mstp
                                    "", // lacp
                                    "", // gvrp
                                    "", // igmp
                                    "", // pim
                                    "ripd",
                                    "", // ospf
                                    "", // bgp
                                    "", // rsvp
                                    ""}; // ldp

/*******************************************************************************
 *                         LOCAL CONSTANTS/LITERALS/TYPES
 ******************************************************************************/

/*******************************************************************************
*                               LOCAL VARIABLES
*******************************************************************************/

/*******************************************************************************
 *                                   CODE
 ******************************************************************************/
// Process Manager 가 관리하는 Process 의 정보를 보여준다
DECMD(cmdProcShowProcess,
  CMD_NODE_VIEW,
  IPC_PRO_MGR | IPC_SHOW_MGR,
  "show process",
  "Show Command",
  "Managed Processes")
{
    StringT binary = NULL;
    Int8T health[15] = {0,};

//    cmdPrint(cmsh, "Enter [%s][%s][%d] [%d]\n", __FILE__, __func__, __LINE__, cargc);
    cmdPrint(cmsh, "STime  : process start time\n");
    cmdPrint(cmsh, "H(#)   : healthcheck status(healthcheck counter)\n");
    cmdPrint(cmsh, "Rtype  : recommendedRecoveryType\n");
    cmdPrint(cmsh, "Lerror : lastErrorType(lastErrorTime, PerformedRecoveryAction)\n\n");
    cmdPrint(cmsh, "%-10s %-20s %-14s %-20s %s\n", "name", "Stime", "H(#)", "Rtype", "Lerror");

    Int32T repeatIndex = 0;
    for (repeatIndex = 0; repeatIndex < MAX_PROCESS_CNT - 1; ++repeatIndex)
    {
        if (LCS_INFO(repeatIndex).run == TRUE)
        {
            if ((binary = rindex(LCS_INFO(repeatIndex).binName, '/')) != NULL)
            {
                ++binary;
            }
            else
            {
                binary = LCS_INFO(repeatIndex).binName;
            }

            sprintf(health, "ok(%d)", LCS_INFO(repeatIndex).healthcheckCount);

            if (LCS_INFO(repeatIndex).lastError == LCS_OK)
            {
                cmdPrint(cmsh, "%-10s %-20s %-14s %-20s %s\n",
                        binary,
                        LCS_INFO(repeatIndex).startTime,
                        health,
                        sRecovery[LCS_INFO(repeatIndex).recovery],
                        sError[LCS_INFO(repeatIndex).error]);
            }
            else
            {
                cmdPrint(cmsh, "%-10s %-20s %-14s %-20s %s(%s,%s)\n",
                        binary,
                        LCS_INFO(repeatIndex).startTime,
                        health,
                        sRecovery[LCS_INFO(repeatIndex).recovery],
                        sError[LCS_INFO(repeatIndex).lastError],
                        LCS_INFO(repeatIndex).lastStartTime,
                        sRecovery[LCS_INFO(repeatIndex).lastRecovery]);
            }
        }
    }

//    cmdPrint(cmsh, "End\n");

    return CMD_IPC_OK;
}

// Redundancy 의 상태를 보여준다
DECMD(cmdProcRedundancyStates,
  CMD_NODE_VIEW,
  IPC_PRO_MGR | IPC_SHOW_MGR,
  "show redundancy states",
  "Show Command",
  "Redundancy Mode",
  "Redundancy Mode's States")
{
//    cmdPrint(cmsh, "Enter [%s][%s][%d] [%d]\n", __FILE__, __func__, __LINE__, cargc);
    cmdPrint(cmsh, "my state = %s\n", sRedundancy[gpProcMgrBase->haState]);
    cmdPrint(cmsh, "peer state = (later)\n");
    cmdPrint(cmsh, "Unit = (later)\n");
    cmdPrint(cmsh, "Unit ID = (later)\n");
    cmdPrint(cmsh, " \n");

    cmdPrint(cmsh, "Redundancy Protocol (Operational) = Simplex(later)\n");
    cmdPrint(cmsh, "Redundancy Protocol (Configured) = Simplex(later)\n");
    cmdPrint(cmsh, "Communications = Up(later)\n");
    cmdPrint(cmsh, "Ready for switchover(later)\n");
    cmdPrint(cmsh, " \n");

    cmdPrint(cmsh, "Last switchover time = (Date)");
    cmdPrint(cmsh, "Last switchover reason = (Reason)");

//    cmdPrint(cmsh, "End\n");

    return CMD_IPC_OK;
}

// Reload 동작을 설정 시간에 수행한다
DECMD(cmdProcReloadAt,
  CMD_NODE_EXEC,
  IPC_PRO_MGR,
  "reload at WOLD",
  "Reload Command",
  "Set Static Time",
  "Reload Time(HH:MM) Set Day")
{
    Int8T date[10] = {0,};
    Int8T tempStr2[3] = {0,};
    Int8T ret = 0;
    Int32T hour = 0;
    Int32T min = 0;

    struct tm *pSetTime = NULL;
    time_t nowTime = 0;
    time_t setTime = 0;

    sprintf(date, "%s:00", cargv[2]);

    ret = nnStrCheckTime(date);

    cmdPrint(cmsh, "Date : %s\n", date);

    if (ret == TRUE)
    {
        cmdPrint(cmsh, "It is Time Format\n");
#if 0
        cmdPrint(cmsh, "date : %s\n", date);
        cmdPrint(cmsh, "temp : %s\n", tempStr2);
#endif
        nnStrGetOrToken(date, tempStr2, ":");
#if 0
        cmdPrint(cmsh, "date : %s\n", date);
        cmdPrint(cmsh, "temp : %s\n", tempStr2);
#endif
        hour = atoi(tempStr2);

        nnStrGetOrToken(date, tempStr2, ":");
#if 0
        cmdPrint(cmsh, "date : %s\n", date);
        cmdPrint(cmsh, "temp : %s\n", tempStr2);
#endif
        min = atoi(tempStr2);

        // 현재 시간 설정
        nowTime = time(NULL);
        pSetTime = localtime(&nowTime);

        // 설정을 원하는 시간으로 변경 후 time_t 값 얻기
        pSetTime->tm_hour = hour;
        pSetTime->tm_min = min;
        pSetTime->tm_sec = 0;

        setTime = mktime(pSetTime);
#if 0
        cmdPrint(cmsh, "setTime[%ld] - nowTime[%ld] : %ld\n", setTime, nowTime, setTime - nowTime);
#endif
        // 시간 비교하여 지난 시간이 아닐 때 타이머 등록
        if ((setTime - nowTime) > 0)
        {
            cmdPrint(cmsh, "Proceed with reload? [y/n]\n");
            cmdPrint(cmsh, "Later set confirm procedure\n");

            struct timeval reloadTv = {0,};
            Int8T atDate[30] = {0,};

            reloadTv.tv_sec = setTime - nowTime;

            if (gpProcMgrBase->pReloadTimer != NULL)
            {
                cmdPrint(cmsh, "Del Prev Reload Timer\n");
                taskTimerDel(gpProcMgrBase->pReloadTimer);
                gpProcMgrBase->pReloadTimer = NULL;
            }

            gpProcMgrBase->pReloadTimer =
               taskTimerSet((void *)reloadTimerCallback, reloadTv, 0, NULL);

            gpProcMgrBase->reloadTime = setTime;

            getDatefromTimet(atDate, gpProcMgrBase->reloadTime);
            hour = (gpProcMgrBase->reloadTime - nowTime) / 3600;
            min = ((gpProcMgrBase->reloadTime - nowTime) % 3600) / 60;

            cmdPrint(cmsh, "Reload scheduled for %s ( in %d hours %d minutes )\n", atDate, hour, min);
        }
        else
        {
            cmdPrint(cmsh, "It is Past Time\n");
        }
    }
    else
    {
        cmdPrint(cmsh, "It is Not Time Format\n");
    }

//    cmdPrint(cmsh, "End\n");

    return CMD_IPC_OK;
}

// Reload 동작을 설정 시간을 확인한다
DECMD(cmdProcShowReload,
  CMD_NODE_VIEW,
  IPC_PRO_MGR | IPC_SHOW_MGR,
  "show reload",
  "Show Command",
  "Reload Time")
{
    if (gpProcMgrBase->pReloadTimer <= NULL)
    {
        cmdPrint(cmsh, "No Scheduled\n");
    }
    else
    {
        time_t nowTime = 0;
        Int8T showDate[30] = {0,};
        Int32T hour = 0;
        Int32T min = 0;
        nowTime = time(NULL);
#if 0
        cmdPrint(cmsh, "Set Time : %ld\n", gpProcMgrBase->reloadTime);
        cmdPrint(cmsh, "Now Time : %ld\n", nowTime);
        cmdPrint(cmsh, "Re  Time : %ld\n", gpProcMgrBase->reloadTime - nowTime);
#endif
        getDatefromTimet(showDate, gpProcMgrBase->reloadTime);
        hour = (gpProcMgrBase->reloadTime - nowTime) / 3600;
        min = ((gpProcMgrBase->reloadTime - nowTime) % 3600) / 60;

        cmdPrint(cmsh, "Reload scheduled for %s ( in %d hours %d minutes )\n", showDate, hour, min);
    }

//    cmdPrint(cmsh, "End\n");

    return CMD_IPC_OK;
}

// Reload 의 동작을 취소,
// Both/Peer/Active Controller 를 Confirm 수행 후 Reload 한다
DECMD(cmdProcReloadCancelAllPeerPower,
  CMD_NODE_EXEC,
  IPC_PRO_MGR,
  "reload (cancel|all|peer|power)",
  "Reload Command",
  "Reload Timer Cancel",
  "Reload Active/Peer Controller",
  "Reload Peer Controller",
  "Reload Active Controller")
{
    if (!strcmp(cargv[1], "cancel"))
    {
        if (gpProcMgrBase->pReloadTimer <= NULL)
        {
            cmdPrint(cmsh, "No Scheduled\n");
        }
        else
        {
            taskTimerDel(gpProcMgrBase->pReloadTimer);
            gpProcMgrBase->pReloadTimer = NULL;
            gpProcMgrBase->reloadTime = 0;

            cmdPrint(cmsh, "Scheduled reload has been cancelled\n");
        }
    }
    else if (!strcmp(cargv[1], "all"))
    {
        cmdPrint(cmsh, "Reload Both Controller\n");
    }
    else if (!strcmp(cargv[1], "peer"))
    {
        cmdPrint(cmsh, "Reload Peer Controller\n");
        cmdPrint(cmsh, "Later\n");
    }
    else if (!strcmp(cargv[1], "power"))
    {
        cmdPrint(cmsh, "Reload Active Controller\n");
    }

//    cmdPrint(cmsh, "End\n");

    return CMD_IPC_OK;
}

// Both/Peer/Active Controller 를 Reload 하며
// now 를 입력하여 Confirm 을 하지 않고 바로 Reload 한다.
DECMD(cmdProcReloadAllPeerPowerNow,
  CMD_NODE_EXEC,
  IPC_PRO_MGR,
  "reload (all|peer|power) now",
  "Reload Command",
  "Reload Active/Peer Controller",
  "Reload Peer Controller",
  "Reload Active Controller",
  "Reload Immediately")
{
    if (!strcmp(cargv[1], "all"))
    {
        cmdPrint(cmsh, "Reload Both Controller Now\n");
    }
    else if (!strcmp(cargv[1], "peer"))
    {
        cmdPrint(cmsh, "Reload Peer Controller Now\n");
        cmdPrint(cmsh, "Later\n");
    }
    else if (!strcmp(cargv[1], "power"))
    {
        cmdPrint(cmsh, "Reload Active Controller Now\n");
    }

//    cmdPrint(cmsh, "End\n");

    return CMD_IPC_OK;
}

// Reload 동작을 취소한다
DECMD(cmdProcNoReload,
  CMD_NODE_EXEC,
  IPC_PRO_MGR,
  "no reload",
  "Cancel Command",
  "Cancel Reload Timer")
{
    if (gpProcMgrBase->pReloadTimer <= NULL)
    {
        cmdPrint(cmsh, "No Scheduled\n");
    }
    else
    {
        taskTimerDel(gpProcMgrBase->pReloadTimer);
        gpProcMgrBase->pReloadTimer = NULL;
        gpProcMgrBase->reloadTime = 0;

        cmdPrint(cmsh, "Scheduled reload has been cancelled\n");
    }

//    cmdPrint(cmsh, "End\n");

    return CMD_IPC_OK;
}

// 최근의 Reload 이유를 확인한다
DECMD(cmdProShowReloadCause,
  CMD_NODE_VIEW,
  IPC_PRO_MGR | IPC_SHOW_MGR,
  "show reload cause",
  "Show Command",
  "Reload Action",
  "Show Reload Cause")
{
    cmdPrint(cmsh, "Reload Cause 1:\n");
    cmdPrint(cmsh, "------------------\n");
    cmdPrint(cmsh, "(Cause)\n");
    cmdPrint(cmsh, " \n");

    cmdPrint(cmsh, "Recommended Action:\n");
    cmdPrint(cmsh, "------------------\n");
    cmdPrint(cmsh, "(Action)\n");
    cmdPrint(cmsh, " \n");

    cmdPrint(cmsh, "Debugging Information:\n");
    cmdPrint(cmsh, "------------------\n");
    cmdPrint(cmsh, "(Info)\n");
    cmdPrint(cmsh, " \n");

//    cmdPrint(cmsh, "End\n");

    return CMD_IPC_OK;
}

// Hardware, Software 버전 및 Memory 사용량을 확인한다
DECMD(cmdProcShowVersion,
  CMD_NODE_VIEW,
  IPC_PRO_MGR | IPC_SHOW_MGR,
  "show version",
  "Show Command",
  "Show Version")
{
    Int32T verIndex = 0;
    StringT binary = NULL;

    cmdPrint(cmsh, "-------------------------------------------------\n");
    cmdPrint(cmsh, "Package Name : n2os-0.00.02\n");
    cmdPrint(cmsh, "-------------------------------------------------\n");
    cmdPrint(cmsh, "%-18s %s\n", "Component Name", "Shared Library Version\n");

    for (verIndex = 0; verIndex < MAX_PROCESS_CNT - 1; ++verIndex)
    {
        if (LCS_INFO(verIndex).run == TRUE)
        {
            if ((binary = rindex(LCS_INFO(verIndex).binName, '/')) != NULL)
            {
                ++binary;
            }
            else
            {
                binary = LCS_INFO(verIndex).binName;
            }

            cmdPrint(cmsh, "%-18s %s", binary, LCS_INFO(verIndex).version);
        }
    }

    cmdPrint(cmsh, "-------------------------------------------------\n");

//    cmdPrint(cmsh, "End\n");

    return CMD_IPC_OK;
}

#if 0
// Redundancy configuration mode 로 진입한다
DECMD(cmdProcRedundancy,
  CMD_NODE_VIEW,
  IPC_PRO_MGR | IPC_SHOW_MGR,
  "redundancy",
  "Redundancy Mode",
  "Process Manager")
{
    cmdPrint(cmsh, "End\n");

    return CMD_IPC_OK;
}
#endif

// 
DECMD(cmdProcReloadIn,
  CMD_NODE_EXEC,
  IPC_PRO_MGR,
  "reload in WOLD",
  "Reload Command",
  "Set After Time",
  "Reload Time(HH:MM)")
{
    Int8T date[10] = {0,};
    Int8T tempStr2[3] = {0,};
    Int8T ret = 0;
    Int32T hour = 0;
    Int32T min = 0;

    time_t nowTime = 0;
    time_t setTime = 0;

    sprintf(date, "%s:00", cargv[2]);

    ret = nnStrCheckTime(date);

    cmdPrint(cmsh, "Date : %s\n", date);

    if (ret == TRUE)
    {
        cmdPrint(cmsh, "It is Time Format\n");
#if 0
        cmdPrint(cmsh, "date : %s\n", date);
        cmdPrint(cmsh, "temp : %s\n", tempStr2);
#endif
        nnStrGetOrToken(date, tempStr2, ":");
#if 0
        cmdPrint(cmsh, "date : %s\n", date);
        cmdPrint(cmsh, "temp : %s\n", tempStr2);
#endif
        hour = atoi(tempStr2);

        nnStrGetOrToken(date, tempStr2, ":");
#if 0
        cmdPrint(cmsh, "date : %s\n", date);
        cmdPrint(cmsh, "temp : %s\n", tempStr2);
#endif
        min = atoi(tempStr2);

        // 현재 시간 설정
        nowTime = time(NULL);

        // 설정을 원하는 시간으로 변경
        setTime = nowTime + (hour * 3600) + (min * 60);
#if 0
        cmdPrint(cmsh, "setTime[%ld] - nowTime[%ld] : %ld\n", setTime, nowTime, setTime - nowTime);
#endif
        cmdPrint(cmsh, "Proceed with reload? [confirm]\n");
        cmdPrint(cmsh, "Later set confirm procedure\n");

        struct timeval reloadTv = {0,};
        Int8T inDate[30] = {0,};

        reloadTv.tv_sec = setTime - nowTime;

        if (gpProcMgrBase->pReloadTimer != NULL)
        {
            cmdPrint(cmsh, "Del Prev Reload Timer\n");
            taskTimerDel(gpProcMgrBase->pReloadTimer);
            gpProcMgrBase->pReloadTimer = NULL;
        }

        gpProcMgrBase->pReloadTimer =
           taskTimerSet((void *)reloadTimerCallback, reloadTv, 0, NULL);

        gpProcMgrBase->reloadTime = setTime;

        getDatefromTimet(inDate, gpProcMgrBase->reloadTime);
        hour = (gpProcMgrBase->reloadTime - nowTime) / 3600;
        min = ((gpProcMgrBase->reloadTime - nowTime) % 3600) / 60;

        cmdPrint(cmsh, "Reload scheduled for %s ( in %d hours %d minutes )\n", inDate, hour, min);
    }
    else
    {
        cmdPrint(cmsh, "It is Not Time Format\n");
    }

//    cmdPrint(cmsh, "End\n");

    return CMD_IPC_OK;
}

// liveupdate 를 수행 할 수 있는 방법 및
// Process Manager 가 관리하는 Process 들을 보여준다
DECMD(cmdProcLiveupdateQuestion,
  CMD_NODE_VIEW,
  IPC_PRO_MGR | IPC_SHOW_MGR,
  "liveupdate usage",
  "Liveupdate Command",
  "Show Managed Process name")
{
    StringT binary = NULL;
    Int8T processName[256] = {0,};

//    cmdPrint(cmsh, "Enter [%s][%s][%d] [%d]\n", __FILE__, __func__, __LINE__, cargc);

    cmdPrint(cmsh, "liveupdate process version\n");
    sprintf(processName, "process :");

    Int32T usageIndex = 0;
    for (usageIndex = 0; usageIndex < MAX_PROCESS_CNT - 1; ++usageIndex)
    {
        if (LCS_INFO(usageIndex).run == TRUE)
        {
            if ((binary = rindex(LCS_INFO(usageIndex).binName, '/')) != NULL)
            {
                ++binary;
            }
            else
            {
                binary = LCS_INFO(usageIndex).binName;
            }

            sprintf(processName, "%s %s", processName, binary);
        }
    }

    cmdPrint(cmsh, "%s", processName);
    cmdPrint(cmsh, "version : x.y.z");

//    cmdPrint(cmsh, "End\n");

    return CMD_IPC_OK;
}

// Process 들에게 liveupdate 를 수행하라고 전달한다
DECMD(cmdProcLiveupdateProcess,
  CMD_NODE_EXEC,
  IPC_PRO_MGR,
  "liveupdate WOLD WOLD",
  "Liveupdate Command",
  "Process name",
  "Update Version")
{
    nnBufferT apiBuff, msgBuff;
    StringT binary = NULL;
    Int8T rcvMsg[4096] = {0,};
    Int8T compVer[64] = {0,};
    Int8T versionLen = 0;
    Int32T cmdFd = 0, cmdKey = 0, cmdResult = 0;
    Int32T updateIndex = 0;
    Uint32T rcvLength = 0;

    nnBufferReset(&apiBuff);
    nnBufferReset(&msgBuff);

    nnBufferSetInt32T(&apiBuff, 0);
    nnBufferSetInt32T(&apiBuff, 0);
    nnBufferSetInt32T(&apiBuff, 0);
    nnBufferSetInt8T(&apiBuff, strlen(cargv[2]));
    nnBufferSetString(&apiBuff, cargv[2], strlen(cargv[2]));

#if 0
    for (updateIndex = 0; updateIndex < MAX_PROCESS_CNT - 1; ++updateIndex)
    {
        if (!strcmp(cargv[1], sProcessName[updateIndex]))
        {
            break;
        }
    }
#else
    for (updateIndex = 0; updateIndex < MAX_PROCESS_CNT - 1; ++updateIndex)
    {
        if ((binary = rindex(LCS_INFO(updateIndex).binName, '/')) != NULL)
        {
            ++binary;
        }
        else
        {
            binary = LCS_INFO(updateIndex).binName;
        }

        if (!strcmp(cargv[1], binary))
        {
            break;
        }
    }
#endif

    if (updateIndex == MAX_PROCESS_CNT - 1)
    {
        cmdPrint(cmsh, "Can't Find Process [%s]\n", cargv[1]);
        NNLOG(LOG_DEBUG, "Can't Find Process [%s]\n", cargv[1]);

        return CMD_IPC_OK;
    }
    if (LCS_INFO(updateIndex).ipcType == PROCESS_MANAGER)
    {
        printf("Process Manager Update\n");
//        taskPmModuleUpdate(apiBuff.data, apiBuff.length);
/*
        cmdPrint(cmsh, "%s is successfully updated to version %s.%s.so\n",
                        cargv[1], cargv[1], cargv[2]);
*/
        NNLOG(LOG_DEBUG, "%s is successfully updated to version %s.%s.so\n",
                        cargv[1], cargv[1], cargv[2]);

        cmdPrint(cmsh, "end\n");
    }
    else
    {

#if 0
        ret = ipcSendAsync(LCS_INFO(updateIndex).ipcType,
                    IPC_LCS_PM2C_DYNAMIC_UPGRADE, apiBuff.data, apiBuff.length);
#else
        NNLOG(LOG_DEBUG, "Send Dynamic Message to [%d]\n", LCS_INFO(updateIndex).ipcType);

        ipcSendSync(LCS_INFO(updateIndex).ipcType, IPC_LCS_PM2C_DYNAMIC_UPGRADE,
                    IPC_LCS_PM2C_DYNAMIC_UPGRADE_RESPONSE, apiBuff.data,
                    apiBuff.length, rcvMsg, &rcvLength);

        NNLOG(LOG_DEBUG, "Response Dynamic Message from [%d]\n", LCS_INFO(updateIndex).ipcType);

        nnBufferAssign(&msgBuff, rcvMsg, rcvLength);

        cmdFd = nnBufferGetInt32T(&msgBuff);
        cmdKey = nnBufferGetInt32T(&msgBuff);
        cmdResult = nnBufferGetInt32T(&msgBuff);
        versionLen = nnBufferGetInt8T(&msgBuff);
        nnBufferGetString(&msgBuff, compVer, versionLen);

        NNLOG(LOG_DEBUG, "cmdFd[%d], cmdKey[%d], cmdResult[%d]\n",
                        cmdFd, cmdKey, cmdResult);
        fprintf(stdout, "cmdFd[%d], cmdKey[%d], cmdResult[%d]\n",
                        cmdFd, cmdKey, cmdResult);
#endif
        if (cmdResult == SUCCESS)
        {
            cmdPrint(cmsh, "%s is successfully updated to version %s.%s.so\n",
                        cargv[1], cargv[1], cargv[2]);
            NNLOG(LOG_DEBUG, "Dynamic Update Successfully [%d]\n", LCS_INFO(updateIndex).ipcType);
        }
        else
        {
            cmdPrint(cmsh, "%s is not successfully update to version %s.%s.so\n",
                        cargv[1], cargv[1], cargv[2]);
            NNLOG(LOG_DEBUG, "Dynamic Update Not Successfully [%d]\n", LCS_INFO(updateIndex).ipcType);
        }
    }

 //   cmdPrint(cmsh, "End\n");

    ipcProcPendingMsg();

    return CMD_IPC_OK;
}
