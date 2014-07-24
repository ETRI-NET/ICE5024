/*3456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*********************************.*********************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute.
*           All rights reserved. No part of this software shall be reproduced,
*           stored in a retrieval system, or transmitted by any means,
*           electronic, mechanical, photocopying, recording, or otherwise,
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief : This file include major functions for policy manager.
 *  - Block Name : Policy Manager
 *  - Process Name : polmgr
 *  - Creator : PyungKoo Park
 *  - Initial Date : 2014/03/03
 */

/**
 * @file        :
 *
 * $Author: $
 * $Date: $
 * $Revision: $
 * $LastChangedBy: $
 */

#include <stdio.h>
#include "nnTypes.h"
#include "nosLib.h"

#include "nnCmdDefines.h"
#include "nnCompProcess.h"

#include "polMgrMain.h"

extern Int32T polMgrWriteConfCB (struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern void polMgrOamTimerSet(void);
extern void polMgrOamTimerUpdate(void);

/* Register 를 위한 함수 */
void registerTimerCallback(Int32T fd, Int16T event, void *arg);

/*
 * External Definitions for Global Data Structure
 */
extern void ** gCompData;

polMgrT *gPolMgrBase;

void 
polInitProcess(void) //initialize component
{
  struct timeval registerTv = {0,};

  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);

  /** Component Global Variable Memory Allocate */
  gPolMgrBase = NNMALLOC(MEM_GLOBAL, sizeof(polMgrT));

  /** Component Command Init **/
  gPolMgrBase->gCmshGlobal = compCmdInit(IPC_POL_MGR, polMgrWriteConfCB);

  /** TODO : Add more here **/

  // Oam Timer Set
  polMgrOamTimerSet();

  /* Assign shared memory pointer to global memory pointer. */
  (*gCompData) = (void *)gPolMgrBase;

  /* 초기화 작업 이후 Process Manager 에게 Register Message 전송 */
  SET_TV(registerTv, FIVE_SEC);
  taskTimerSet((void *)registerTimerCallback, registerTv, 0, NULL);
}


void 
polTermProcess(void) // terminate component
{
  Int8T ret = 0;
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);

  /** Component Command Close */
  compCmdFree(gPolMgrBase->gCmshGlobal);
 
  /** TODO : Add more here **/



  /** Component Global Variable Memory Free */
  NNFREE(MEM_GLOBAL, gPolMgrBase);

  /** System Library Close */
  taskClose();
  nnLogClose();
  memClose();
}


void polHoldProcess(void)  // hold component
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);  

  /** TODO : Add more here **/


}

extern struct polMgr *gPolIpcBase;
void polRestartProcess(void) // restart component
{
  fprintf(stdout, "enter %s\n", __FUNCTION__);

  /* Re-assign shared memory pointer to global memory pointer. */
  gPolMgrBase = (polMgrT *)(*gCompData);

  /** Component Command Restart **/
  compCmdUpdate(gPolMgrBase->gCmshGlobal, polMgrWriteConfCB);

  
  /** TODO : Add more here **/

  // Oam Printf Update
  polMgrOamTimerUpdate();

}

void polSignalProcess(Int32T sigType)  // signal catch proces
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);
  NNLOG(LOG_DEBUG, "Signal(%d) Occured !! \n", sigType);

  /** TODO : Add more here **/


  polTermProcess();

}

/* 종료를 위한 Timer 콜백함수 */
void closeTimerCallback(Int32T fd, Int16T event, void *pArg)
{
    fprintf(stdout, "I'm %s : %s\n", MY_NAME, __func__);

    polTermProcess(); // terminate component
}

/* 등록을 위한 Timer 콜백함수 */
void registerTimerCallback(Int32T fd, Int16T event, void *arg)
{
    LcsAttributeT lcsAttribute = {0,};
    Int8T ret = 0;

    /*
        // LcsAttribute 는 Process Manager 가 Process 를 관리할 때
        // 사용하는 값으로 여기서 원하는 시간으로 설정하거나
        // 모두 0으로 입력시 procmgr.conf 에서 읽은 기본값으로 관리함

        // HealthCheck 간격
        lcsAttribute.healthcheckInterval = 0;

        // HealtCheck Response Timeout 시간
        lcsAttribute.healthcheckResponseTimeout = 0;

        // Process Manager 의 Message 에 대한 응답의 Timeout 시간
        lcsAttribute.requestResponseTimeout = 0;

        /// Passive Monitoring 시 사용될 값들 ///
        lcsAttribute.memoryThreshold = 0;  // Memory 임계치
        lcsAttribute.cpuThreshold = 0;     // CPU 임계치
    */

    ret = lcsRegister(MY_PTYPE, MY_ITYPE, lcsAttribute);

    if (ret == LCS_OK)
    {
        // 성공
    }
    else
    {
        // 실패
    }
}
