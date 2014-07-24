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
#include "lcsService.h"

#define TEN_SEC         10000000L // ms = 10 sec
#define FIVE_SEC        5000000L  // ms = 5 sec
#define THREE_SEC       3000000L  // ms = 3 sec
#define ONE_SEC         1000000L  // ms = 1 sec

#define SET_TV(A, B) A.tv_sec = (B / TIME_ONE_MILLISECOND); \
                    A.tv_usec = (B % TIME_ONE_MILLISECOND);

extern Int32T polMgrWriteConfCB (struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern void polMgrOamTimerSet(void);
extern void polMgrOamTimerUpdate(void);

/*
 * External Definitions for Global Data Structure
 */
extern struct polMgr *gPolIpcBase;
extern void ** gCompData;

void printName(Int32T fd, Int16T event, void *arg);

polMgrT *gPolMgrBase;
LcsAttributeT gLcsAttribute = {0,};

void polInitProcess(void) //initialize component
{
    NNLOG(LOG_DEBUG, "%s === Init S === Init S === Init S ===\n", MY_NAME);
    fprintf(stdout, "%s === Init S === Init S === Init S ===\n", MY_NAME);

    struct timeval attributeGetTv = {0,};
    struct timeval attributeSetTv = {0,};
    struct timeval printTv = {0,};

    NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);

    /** Component Global Variable Memory Allocate */
    gPolMgrBase = NNMALLOC(MEM_GLOBAL, sizeof(polMgrT));

    /** Component Command Init **/
    gPolMgrBase->gCmshGlobal = compCmdInit(IPC_RIP, polMgrWriteConfCB);

    /** TODO : Add more here **/

    // Oam Timer Set
    polMgrOamTimerSet();

    /* Assign shared memory pointer to global memory pointer. */
    (*gCompData) = (void *)gPolMgrBase;

#ifdef OPTIONAL
    /*
     * Component 관리 속성에 대해선 원하는 경우 값을 변경
     * 0 일 경우 Default 값 사용
     */
    /* Component 의 Recovery Action */
    gLcsAttribute.defaultRR = LCS_COMPONENT_RESTART;

    /* HealthCheck 간격(ms) */
    gLcsAttribute.healthcheckInterval = 0;

    /* HealtCheck Response Timeout 시간(ms) */
    gLcsAttribute.healthcheckResponseTimeout = 0;

    /* Process Manager 의 Message 에 대한 응답의 Timeout 시간(ms) */
    gLcsAttribute.requestResponseTimeout = 0;

    /* Passive Monitoring 시 사용될 값들 */
    gLcsAttribute.memoryThreshold = 0; // Memory 임계치(Mbyte)
    gLcsAttribute.cpuThreshold = 0;    // CPU 임계치(%)
#endif

    /* 초기화 작업 이후 Process Manager 에게 Register Message 전송 */
    lcsRegister(LCS_LDP, LDP, gLcsAttribute);

    /* 사용 예시를 위해 추가함 */
    /* 5초 후 Process Manager 에게서 Attribute 를 얻어 옴 */
    SET_TV(attributeGetTv, FIVE_SEC);
    taskTimerSet((void *)attributeGetTimerCallback, attributeGetTv, 0, NULL);
    fprintf(stdout, "Add Timer : attributeGet\n");

    /* 10초 후 Process Manager 에게 변경한 Attribute 를 전달 함 */
    SET_TV(attributeSetTv, TEN_SEC);
    taskTimerSet((void *)attributeSetTimerCallback, attributeSetTv, 0, NULL);
    fprintf(stdout, "Add Timer : attributeSet\n");

    /* 자신의 이름을 찍는 Timer */
    SET_TV(printTv, FIVE_SEC);
    taskTimerSet((void *)printName, printTv, 0, NULL);
    fprintf(stdout, "Add Timer : printName\n");

    NNLOG(LOG_DEBUG, "%s === Init E === Init E === Init E ===\n", MY_NAME);
    fprintf(stdout, "%s === Init E === Init E === Init E ===\n", MY_NAME);
}

void polTermProcess(void) // terminate component
{
    NNLOG(LOG_DEBUG, "%s === Term S === Term S === Term S ===\n", MY_NAME);
    fprintf(stdout, "%s === Term S === Term S === Term S ===\n", MY_NAME);

    /** Component Command Close */
    compCmdFree(gPolMgrBase->gCmshGlobal);
 
    /** TODO : Add more here **/

    /** Component Global Variable Memory Free */
    NNFREE(MEM_GLOBAL, gPolMgrBase);

    fprintf(stdout, "Terminate LDP Success\n");

    NNLOG(LOG_DEBUG, "%s === Term E === Term E === Term E ===\n", MY_NAME);
    fprintf(stdout, "%s === Term E === Term E === Term E ===\n", MY_NAME);

    /** System Library Close */
    taskClose();
    nnLogClose();
    memClose();
}

void polHoldProcess(void)  // hold component
{
    NNLOG(LOG_DEBUG, "%s === Hold S === Hold S === Hold S ===\n", MY_NAME);
    fprintf(stdout, "%s === Hold S === Hold S === Hold S ===\n", MY_NAME);

    /** TODO : Add more here **/

    NNLOG(LOG_DEBUG, "%s === Hold E === Hold E === Hold E ===\n", MY_NAME);
    fprintf(stdout, "%s === Hold E === Hold E === Hold E ===\n", MY_NAME);
}

void polRestartProcess(void) // restart component
{
    NNLOG(LOG_DEBUG, "%s === Restert S === Restart S === Restart S ===\n", MY_NAME);
    fprintf(stdout, "%s === Restert S === Restart S === Restart S ===\n", MY_NAME);

    /* Re-assign shared memory pointer to global memory pointer. */
    gPolMgrBase = (polMgrT *)(*gCompData);

    /** Component Command Restart **/
    compCmdUpdate(gPolMgrBase->gCmshGlobal, polMgrWriteConfCB);

    /** TODO : Add more here **/

    // Oam Printf Update
    polMgrOamTimerUpdate();

    NNLOG(LOG_DEBUG, "%s === Restert E === Restart E === Restart E ===\n", MY_NAME);
    fprintf(stdout, "%s === Restert E === Restart E === Restart E ===\n", MY_NAME);
}

void polSignalProcess(Int32T sigType)  // signal catch proces
{
    NNLOG(LOG_DEBUG, "%s === Signal S === Signal S === Signal S ===\n", MY_NAME);
    fprintf(stdout, "%s === Signal S === Signal S === Signal S ===\n", MY_NAME);

    NNLOG(LOG_DEBUG, "Signal(%d) Occured !! \n", sigType);
    fprintf(stdout, "Signal(%d) Occured !! \n", sigType);

    /** TODO : Add more here **/

    /* 종료 작업 수행 */
    polTermProcess();

    NNLOG(LOG_DEBUG, "%s === Signal E === Signal E === Signal E ===\n", MY_NAME);
    fprintf(stdout, "%s === Signal E === Signal E === Signal E ===\n", MY_NAME);
}

/* 사용 예시를 위해 추가 */
/* Attribute 를 설정을 위한 Timer 콜백함수 */
void attributeSetTimerCallback(Int32T fd, Int16T event, void *arg)
{
    struct timeval setTv = {0,};

    NNLOG(LOG_DEBUG, "%s === Attribute Set S === Attribute Set S ===\n", MY_NAME);
    fprintf(stdout, "%s === Attribute Set S === Attribute Set S ===\n", MY_NAME);

    fprintf(stdout, "\n\n===== SET ATTRIBUTE =====\n");
    /* HealthCheck 간격을 30초로 설정 */
    gLcsAttribute.healthcheckInterval = (TEN_SEC * 3);

    /* HealtCheck Response Timeout 시간을 30초로 설정 */
    gLcsAttribute.healthcheckResponseTimeout = (TEN_SEC * 3);

    /* Process Manager 의 Message 에 대한 응답의 Timeout 시간을 30초로 설정 */
    gLcsAttribute.requestResponseTimeout = (TEN_SEC * 3);

    lcsAttributeSet(LCS_LDP, gLcsAttribute);

    /* 값이 제대로 설정 됐는지 확인을 위해 AttributeGet Timer 설정(5초) */
    SET_TV(setTv, FIVE_SEC);
    taskTimerSet((void *)attributeGetTimerCallback, setTv, 0, NULL);

    NNLOG(LOG_DEBUG, "%s === Attribute Set E === Attribute Set E ===\n", MY_NAME);
    fprintf(stdout, "%s === Attribute Set E === Attribute Set E ===\n", MY_NAME);
}

/* Attribute 를 가져오기 위한 Timer 콜백함수 */
void attributeGetTimerCallback(Int32T fd, Int16T event, void *arg)
{
    NNLOG(LOG_DEBUG, "%s === Attribute Get S === Attribute Get S ===\n", MY_NAME);
    fprintf(stdout, "%s === Attribute Get S === Attribute Get S ===\n", MY_NAME);

    lcsAttributeGet(LCS_LDP, &gLcsAttribute);

    /* Process Manager 에서 관리 속성을 가져와 출력 */
    fprintf(stdout, "\n\n===== PRINT ATTRIBUTE =====\n");
    fprintf(stdout, "defaultRR : %d\n", gLcsAttribute.defaultRR);
    fprintf(stdout, "healthInterval : %d\n", gLcsAttribute.healthcheckInterval);
    fprintf(stdout, "healthTimeout : %d\n", gLcsAttribute.healthcheckResponseTimeout);
    fprintf(stdout, "requestTimeout : %d\n", gLcsAttribute.requestResponseTimeout);
    fprintf(stdout, "haState : %d\n", gLcsAttribute.haState);
    fprintf(stdout, "memoryThreshold : %d\n", gLcsAttribute.memoryThreshold);
    fprintf(stdout, "cpuThreshold : %d\n", gLcsAttribute.cpuThreshold);
    fprintf(stdout, "===== PRINT ATTRIBUTE =====\n\n");

    NNLOG(LOG_DEBUG, "%s === Attribute Get E === Attribute Get E ===\n", MY_NAME);
    fprintf(stdout, "%s === Attribute Get E === Attribute Get E ===\n", MY_NAME);
}

/* 자신의 이름을 찍는 함수 */
void printName(Int32T fd, Int16T event, void *arg)
{
    struct timeval printTv2;

    SET_TV(printTv2, FIVE_SEC);

    fprintf(stdout, "My Name[%s], Process Type[%d], Ipc Type[%d]\n", MY_NAME, MY_PTYPE, MY_ITYPE);

    (void *)taskTimerSet((void *)printName, printTv2, 0, NULL);
}
