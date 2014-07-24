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
 * @brief : Process Manager 에서 사용하는 Task 관련 기능을 수행
 *  - Block Name : Process Manager
 *  - Process Name : procmgr
 *  - Creator : Changwoo Lee
 *  - Initial Date : 2014/3/3
 */

/**
* @file : processmgrTask.c
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
#include "nnTypes.h"

#include "nnCmdDefines.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"
#include "nnCompProcess.h"

#include "nosLib.h"

#include "processmgrMain.h"
#include "processmgrTask.h"

/*******************************************************************************
 *                              GLOBAL VARIABLES
 ******************************************************************************/
//extern LcsProcessMgrT *gpProcMgrBase;

/*******************************************************************************
 *                         GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/

/*******************************************************************************
 *                         LOCAL CONSTANTS/LITERALS/TYPES
 ******************************************************************************/

/*******************************************************************************
*                               LOCAL VARIABLES
*******************************************************************************/

/*******************************************************************************
 *                                   CODE
 ******************************************************************************/

/* Timer 를 등록하는 함수 */
void *timerAdd(Int32T processType, Uint32T timeout,
                Uint32T persist, timerCallbackT timerFunc, void *pArg)
{
    struct timeval tv = {0,};

    SET_TV(tv, timeout);

//    fprintf(stdout, "@@@@@@@@ ADD TIMER : [%d][%d]\n", processType, timeout);

    return taskTimerSet((void *)timerFunc, tv, persist, pArg);
}

/* Timer 를 삭제하는 함수 */
void timerDel(Int32T processType, void *pTimer)
{
//    fprintf(stdout, "@@@@@@@@ DELETE TIMER Find Type : %d\n", processType);

    if (pTimer <= NULL)
    {
//      fprintf(stdout, "pTimer is NULL!!!!!\n");
    }
    else
    {
        taskTimerDel(pTimer);
    }

    pTimer = NULL;
}
