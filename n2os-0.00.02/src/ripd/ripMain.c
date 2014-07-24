/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : rip 프로토콜의 메인함수를 포함하는 화일이며, 다음과 같은 초기화
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
 * - Block Name : RIP Protocol
 * - Process Name : ripd
 * - Creator : Suncheul Kim
 * - Initial Date : 2014/02/20
 */

/**
 * @file        : ripMain.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include "nnTypes.h"
#include "nnMemmgr.h"
#include "taskManager.h"
#include "nnLog.h"

#include "ripd.h"
#include "ripInit.h"
#include "ripIpc.h"


/* Global Definitions is here */


#include "nnPrefix.h"  // testing
/* Main routine of ripd. */
Int32T
main (Int32T argc, char **argv)
{
  /* Initialize Memmory Manager             */
  /* RIB_MANAGER defined in nnTypes.h file. */
  if(memInit(RIP) == FAILURE){
    return -1; 
  }

  /* Initialize Log */
  if(nnLogInit(RIP) != SUCCESS){
    return -1; 
  }

  /* Max Log FileName size : 20 */
  nnLogSetFile("logRIP", 10);
  nnLogSetFlag(LOG_FILE);

  /* Signal Registration */
  taskCreate(ripInitProcess);
  
  /* IPC Callback Function Registration */
  ipcChannelOpen(RIP, (void *)ripIpcProcess);

  /* Event Callback Function Registration */
  eventOpen((void *)ripEventProcess);

  /* Event Registration */
  ripEventSubscribe();
  
  /* Test timer. */                       
  struct timeval tv;
  tv.tv_sec =  2;
  tv.tv_usec = 9;
  taskTimerSet(ripTimerProcess, tv, TASK_PERSIST, NULL);
 
  /* Update Timer Registration (periodic 30sec)*/
  struct timeval tvRipUpdate;
  tvRipUpdate.tv_sec = RIP_UPDATE_TIMER_DEFAULT;
  tvRipUpdate.tv_usec = 0;
  pRip->pUpdateTimer = taskTimerSet(ripTimerUpdate, 
                                    tvRipUpdate, TASK_PERSIST, NULL);
  
  /* Signal Registration */
  taskSignalSetAggr(ripSignalProcess);
        
  /* Task Dispatch */
  taskDispatch();
         
  return (0);
}
