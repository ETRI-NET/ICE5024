#include <stdio.h>

#include "nosLib.h"
#include "nnDefines.h"

#include "lcsService.h"

#if !defined(_processmgrMain_h)
#define _processmgrMain_h

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
 * @brief : Process 들을 관리하는 Manager
 *  - Block Name : Process Manager
 *  - Process Name : procmgr
 *  - Creator : Changwoo Lee
 *  - Initial Date : 2014/3/3
*/

/**
 * @file : processmgrMain.h
 *
 * $Id:
 * $Author:
 * $Date:
 * $Revision:
 * $LastChangedBy:
 **/

/*******************************************************************************
 *                        CONSTANTS / LITERALS / TYPES
 ******************************************************************************/
//#define LCS_BIN_DIRECTORY "/home/test/Development/n2os-project/trunk/n2os-0.00.02/bin/"
//#define LCS_PROCESS_MANAGER_CONF "/home/test/Development/n2os-project/trunk/n2os-0.00.02/bin/procmgr.conf"

#define LCS_PROCESS_MANAGER_CONF "./procmgr.conf"

#define LCS_ONE_MEGA 1048576 /* 1M */

#define LCS_MAX_BUF 1024
#define LCS_PID_LIST_BLOCK 32

#define LCS_SEND_SIGNAL 1

#define TEN_SEC         10000000L // ms = 10 sec
#define FIVE_SEC        5000000L  // ms = 5 sec
#define THREE_SEC       3000000L  // ms = 3 sec

#define DEFAULT_TIMEOUT TEN_SEC
#define DEFAULT_CLOSE   THREE_SEC

#define LCS_INFO(i) gpProcMgrBase->processInfo[i]
#define LCS_ATTR(i) gpProcMgrBase->processInfo[i].attribute
#define LCS_STATUS  gpProcMgrBase->serviceStatus

#define LCS_HEALTH_ACTIVE_TIMER(i) LCS_INFO(i).pHealthcheckActiveTimer
#define LCS_HEALTH_PASSIVE_TIMER(i) LCS_INFO(i).pHealthcheckPassiveTimer
#define LCS_HEALTH_RESPONSE_TIMER(i) LCS_INFO(i).pHealthcheckResponseTimer
#define LCS_RESPONSE_TIMER(i) LCS_INFO(i).pResponseTimer
#define LCS_QUIESCING_TIMER(i) LCS_INFO(i).pQuiescingTimer

#define LCS_HEALTH_INTERVAL(i) LCS_ATTR(i).healthcheckInterval
#define LCS_HEALTH_TIMEOUT(i) LCS_ATTR(i).healthcheckResponseTimeout
#define LCS_RESPONSE_TIMEOUT(i) LCS_ATTR(i).requestResponseTimeout


#define SET_TV(A, B) A.tv_sec = (B / TIME_ONE_MILLISECOND); \
                    A.tv_usec = (B % TIME_ONE_MILLISECOND);

#define OLD 0

typedef enum
{
    LCS_INIT = 0,
    LCS_STOP,
    LCS_STOPING,
    LCS_QUIESCING,
    LCS_RELOAD,
    LCS_RUN
} LcsStatusT;

typedef struct
{
    LcsProcessTypeT processType;
    Int32T          ipcType;
    LcsAttributeT   attribute;

    Int8T           run;
    Int8T           binName[30];
    Int8T           startTime[20];
    Int8T           lastStartTime[20];
    Int8T           version[64];
    Int32T          pid;
    Int32T          healthcheckCount;
    LcsErrorT       error;
    LcsErrorT       lastError;
    LcsRecoveryT    recovery;
    LcsRecoveryT    lastRecovery;

    // Process 에게 요청하는 메시지의 응답에 대한 Timeout Timer
    void            *pResponseTimer;
    // Quiescing 의 Timeout Timer
    void            *pQuiescingTimer;
    // Healthcheck Active 수행하는 Timer
    void            *pHealthcheckActiveTimer;
    // Healthcheck Passive 수행하는 Timer
    void            *pHealthcheckPassiveTimer;
    // Healthcheck 의 응답에 대한 Timeout Timer
    void            *pHealthcheckResponseTimer;
} LcsProcessInfoT;

typedef struct
{
    LcsProcessInfoT processInfo[MAX_PROCESS_CNT - 1];

    void *gpCmshGlobal;

    time_t reloadTime;
    LcsStatusT status;
    Int32T runCount;
    Int32T haState;
    LcsServiceStatusEventT serviceStatus;

    // Reload 를 수행하는 Timer
    void *pReloadTimer;
} LcsProcessMgrT;

LcsProcessMgrT *gpProcMgrBase;

typedef struct procMgr
{
  void *gpCmshGlobal;      /** Component Cmd Variable **/

  /** TODO: Add more here */
  void *gOamTimerEvent;   /** Component Timer Variable **/
  void *gTcpFdEvent;      /** Component Fd Varialbe **/

} polMgrT;

//extern polMgrT *gPolMgrBase;

/*
 * Description : Initialize function for policy manager
 */
void procInitProcess(void);

/*
 * Description : Terminnation function for policy manager
 */
void procTermProcess(void);

/*
 * Description : Restarting function for policy manager
 */
void procRestartProcess(void);

/*
 * Description : Holding function for policy manager
 */
void procHoldProcess(void);

/*
 * Description : Signal processing function
 *
 * param [in] sig : signal id
 */
void procSingalProcess(Int32T sig);

/*
 * Description : Stop precess
 *
 */

Int8T initProcmgr();
void closeProcmgr();

void initTimerCallback(Int32T fd, Int16T event, void *pArg);
void closeTimerCallback(Int32T fd, Int16T event, void *pArg);

Int8T readProcConf();
void delAllTimer();

#endif /* _processmgrMain_h */
