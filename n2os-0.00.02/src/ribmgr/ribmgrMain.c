/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : ribmgr 블록의 메인함수를 포함하는 화일이며, 다음과 같은 초기화
 * 절차를 수행한다.
 *
 * 메모리 매니저를 위한 클라이언트 코드를 초기화 한다.
 * 로그 기능을 초기화 한다.
 * IPC 매니저로부터 ribmgr의 초기화 루틴이 완료됨을 통보받는 콜백함수를 등록한다.
 * IPC 메시지 수신시 호출될 콜백함수를 등록한다.
 * Event 메시지 수신시 호출될 콜백함수를 등록한다.
 * Timer 만료시 호출될 콜백함수를 등록한다.
 * 시그널 발생시 시그널 처리를 위한 콜백함수를 등록한다.
 * 내부 데이터 구조를 초기화 한다.(RouterID, Interface, RIB)
 * 타스크 디스패치
 *
 * - Block Name : RIB Manager
 * - Process Name : ribmgr
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : ribmgrMain.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */


#include "nnRibDefines.h"
#include "nnTypes.h"
#include "nnUtility.h"
#include "nosLib.h"

#include "ribmgrInit.h"
#include "ribmgrRib.h"
#include "ribmgrRouterid.h"

#define TEST_TIMER_INTERVAL      3

/*
 * Global Definitions is here
 */
/* Rib manager global data structure. */

/*
 * External Definitions are here
 */
extern Int32T netlinkInit();

#include "nnBuffer.h"


/**
 * Description : Main function
 *
 * @param [in] argc : Number of argument
 * @param [in] argv : Array of arguments
 *
 * @retval : 0 if normal termination, -1 if abnormal termination
 */
Int32T 
main(Int32T argc, char **argv)
{
  Int8T pStrTimeBegin[22]={};
  Int8T pStrTimeEnd[22]={};
  /*
   * Initialize Memmory Manager
   *  RIB_MANAGER defined in nnTypes.h file.
   */
  if(memInit (RIB_MANAGER) == FAILURE)
  {
    fprintf(stderr, "Could not Initialize Memory Manager !!\n");
    return -1;
  }

  /*
   * Initialize Log
   */
  if(nnLogInit(RIB_MANAGER) != SUCCESS)
  {
   fprintf(stderr, "Could not Initialize Log Service !!\n");
    return -1;
  }

  /*
   * Max Log FileName size : 20
   */
  nnLogSetFile("logRibmgr", 10);
  nnLogSetFlag(LOG_FILE); 

  /*
   * Set Priority
   */
  nnLogSetPriority(LOG_DEBUG);

  nnTimeGetCurrent(pStrTimeBegin);
  printf("Start Time = %s\n", pStrTimeBegin);

  /*
   * Signal Registration
   */
  taskCreate(ribInitProcess);

  /*
   * IPC Callback Function Registration
   */
  ipcChannelOpen(RIB_MANAGER, (void *)ribIpcProcess);

  /*
   * Event Callback Function Registration
   */
  eventOpen((void *)ribEventProcess);

  /*
   * Event Registration
   */
  //eventSubscribe(EVENT_ROUTER_ID, EVENT_PRI_MIDDLE);

  /*
   * Timer Registration
   */
#if 0
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 9;
  taskTimerSet(timerCallback, tv, TASK_PERSIST);
#endif

  /*
   * Signal Registration
   */
  taskSignalSetAggr (ribSignalProcess);

  /*
   * RouterID Data Structure Initialize
   */
  //initRibmgr (_gpRibmgr_);

  /*
   * Task Dispatch
   */
  taskDispatch();

  nnTimeGetCurrent(pStrTimeEnd);
  printf("End Time = %s\n", pStrTimeEnd);

  return 0;
}
