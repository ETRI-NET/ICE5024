#include <stdio.h>

#include "nnTypes.h"

#if !defined(_processmgrTask_h)
#define _processmgrTask_h

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
 * @briefa : Process Manager 에서 사용하는 Task 관련 기능을 수행
 *  - Block Name : Process Manager
 *  - Process Name : procmgr
 *  - Creator : Changwoo Lee
 *  - Initial Date : 2014/3/28
*/

/**
 * @file : processmgrTask.h
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

/** Timer 에 사용될 콜백함수 */
typedef void (*timerCallbackT)(Int32T msgId, void *pData, Uint32T dataLen);

/*******************************************************************************
*                                GLOBALS VARIABLES
*******************************************************************************/

/*******************************************************************************
 *                           GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/
void *timerAdd(Int32T processType, Uint32T timeout,
                Uint32T persist, timerCallbackT timerFunc, void *pArg);
void timerDel(Int32T processType, void *pTimer);

#endif /* _processmgrTask_h */
