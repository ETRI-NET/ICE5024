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
 * @brief : N2OS Common Library - Time 관련
 *  - Block Name : Library
 *  - Process Name : rLibraryibmgr
 *  - Creator : Changwoo Lee, JaeSu Han
 *  - Initial Date : 2013/7/15
 */

/**
* @file	: nnTime.c 
*
* $Id: nnTime.c 830 2014-02-14 02:21:45Z lcw1127 $			
* $Author: lcw1127 $ 
* $Date: 2014-02-14 11:21:45 +0900 (Fri, 14 Feb 2014) $
* $Revision: 830 $
* $LastChangedBy: lcw1127 $
**/

/*******************************************************************************
 *                               INCLUDE FILES
 ******************************************************************************/
#include <stdio.h>
#include <time.h>

#include "nnTime.h"

/*******************************************************************************
 *                              GLOBAL VARIABLES
 ******************************************************************************/


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

/**
 * Description: 현재 시간을 yyyymmdd hh:mm:ss 형태로 저장하여 변환하는 함수
 *
 * @param [out] timeStr : 현재 시간을 저장할 변수
 *
 * @bug  
 *  timeStr 에 대한 메모리 할당은 사용자가 선행해서 해야 함 
 */

void nnTimeGetCurrent (StringT timeStr)
{
    struct tm *t = NULL;
    time_t now = 0;

    now = time(NULL);
    t = localtime(&now);

    sprintf(timeStr, "%d%02d%02d %02d:%02d:%2.2d",
                     t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                     t->tm_hour, t->tm_min, t->tm_sec);
}

/**
 * Description: 현재 날짜를 yyyymmdd 형태로 저장하여 반환하는 함수
 *
 * @param [out] timeStr : 현재 시간을 저장할 변수
 *
 * @bug  
 *  timeStr 에 대한 메모리 할당은 사용자가 선행해서 해야 함 
 */

void nnTimeGetCurrentDate (StringT timeStr)
{
    struct tm *t = NULL;
    time_t now = 0;

    now = time(NULL);
    t = localtime(&now);

    sprintf(timeStr, "%d%02d%02d", 
                     t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
}
