#include <stdlib.h>
#include "nnTypes.h"

#include "processmgrMain.h"
#include "processmgrTask.h"

#if !defined(_processmgrIpc_h)
#define _processmgrIpc_h

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
 * @file : processmgrIpc.h
 *
 * $Author:
 * $Date:
 * $Revision:
 * $LastChangedBy:
 */

/*******************************************************************************
 *                        CONSTANTS / LITERALS / TYPES
 ******************************************************************************/

/*******************************************************************************
*                                GLOBALS VARIABLES
*******************************************************************************/

/*******************************************************************************
 *                           GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/
#define IPC_LCS_TEST_QUIESCING               1300
#define IPC_LCS_TEST_RELOAD                  1301

/* Event 는 nnDefines.h 에 입력해야 함 */
//#define EVENT_LCS_COMPONENT_ERROR_OCCURRED   1300
//#define EVENT_LCS_COMPONENT_SERVICE_STATUS   1301

/* nnDefines.h 에 입력함 */
//#define IPC_STARTUP_CONFIG_REQUEST           1390
//#define IPC_STARTUP_CONFIG_RESPONSE          1391

/*
 * Description : IPC 메시지를 수신하는 경우 호출될 콜백 함수임.
 *
 * param [in] msgId : 메시지 ID
 * param [in] data : 메시지 버퍼
 * param [in] datalen : 메시지 버퍼 길이
 */
void procIpcProcesss(Int32T msgId, void *pData, Uint32T dataLen);

/*
 * Description : EVENT 메시지를 수신하는 경우 호출될 콜백 함수임.
 *
 * param [in] msgId : 메시지 ID
 * param [in] data : 메시지 버퍼
 * param [in] datalen : 메시지 버퍼 길이
 */
void procEventProcess(Int32T msgId, void *pData, Uint32T dataLen);

/* Recovery Action 관련 함수 */
Int8T doRecoveryAction(Int32T processType,
                        LcsErrorOccurredEventT lcsErrorEvent);
// Int8T forcefulRestart(Int32T processType);
void forcefulRestart(Int32T processType);

/* Message Send 관련 함수 */
Int8T sendSetRoleMessage(Int32T processType);
Int8T sendTerminateMessage(Int32T processType);

/* Timer 관련 콜백함수 */
void timeoutTimerCallback(Int32T fd, Int16T event, void *pArg);
void healthActiveTimerCallback(Int32T fd, Int16T event, void *pArg);
void healthPassiveTimerCallback(Int32T fd, Int16T event, void *pArg);
void reloadTimerCallback(Int32T fd, Int16T event, void *pArg);

#endif /* _processmgrIpc_h */
