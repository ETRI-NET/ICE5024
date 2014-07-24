#include <stdio.h>

#include "nnTypes.h"

#if !defined(_processmgrUtility_h)
#define _processmgrUtility_h

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

/*******************************************************************************
*                                GLOBALS VARIABLES
*******************************************************************************/

/*******************************************************************************
 *                           GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/
Int32T startProcess(StringT binName);
void endProcess(StringT binName);
void sendSignaltoPid(Int32T pid, Int32T sigNum);

Int32T checkNumber(StringT str);
Int32T getPidfromName(StringT name);
Int8T getNamefromPid(StringT binName, Int32T pid);

void getDatefromTimet(StringT dest, time_t time);
#endif /* _processmgrUtility_h */
