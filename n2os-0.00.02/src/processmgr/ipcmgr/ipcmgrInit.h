#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "nnTypes.h"
#include "nosLib.h"

#include "nnList.h"

#if !defined(ipcmgrMain_h)
#define _ipcmgrMain_h

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
 * @brief : Component간 IPC Channel Open과 Event 전송을 중계
 *  - Block Name : IPC Manager
 *  - Process Name : IPC Manager
 *  - Creator : JaeSu Han
 *  - Initial Date : 2013/7/15
*/

/**
 * @file : ipcmgrMain.h
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

#define IPCMGR_EVENT_SUBSCRIBE -1
#define IPCMGR_EVENT_UNSUBSCRIBE -2
#define IPCMGR_EVENT_PROCESS_EOF -3

/* Event 등록/삭제 Message Data Format */
typedef struct EventSubMsgT
{
    Uint32T process;
    Uint32T eventId;
    Uint32T priority;

} EventSubMsgT;

/* Event Subscribe process Node 구조체 */
typedef struct EventSubProcessT 
{
    Uint32T priority;       /* 해당 Event의 Priority */
    Uint32T process;        /* Subscribe process */

} EventSubProcessT;

/* Event Subscribe process 관리 구조제 */
typedef struct EventSubT
{
	ListT * subProcess;

} EventSubT;


/* IPC Manager Global DataStructure */
typedef struct Ipcmgr {

  /* Command related pointer. */
  void * pCmshGlobal;

  /* Event : Subscribe process List */
  EventSubT gEventSubList[MAX_EVENT_CNT];

} IpcmgrT;

/*******************************************************************************
*                                GLOBALS VARIABLES
*******************************************************************************/

extern IpcmgrT * pIpcmgr;


/*******************************************************************************
 *                           GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/

void eventSubInfoInit(void);
void eventSubInfoClose(void);
Int8T eventSubPriSortFunc(void *pOldData, void *pNewData);
Int8T eventSubDupCheckFunc (void * pOldData, void * pNewData);
Int32T eventSubscribeRegister
              (Uint32T process, Uint32T eventId, Uint32T priority);
void eventSubscribeDeregister(Uint32T process, Uint32T eventId);
void eventSubscribePrint(Uint32T eventId);

#endif
