#if !defined(_cmMain_h)
#define _cmMain_h

/*3456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file include major functions for cmicy manager. 
 *  - Block Name : Policy Manager
 *  - Process Name : cmmgr
 *  - Creator : PyungKoo Park
 *  - Initial Date : 2014/03/03
 */

/**
 * @file:
 *
 * $Author:
 * $Date:
 * $Revision:
 * $LastChangedBy:
 */

#include "nnTypes.h"

/*********/
#define TEN_SEC         10000000L // ms = 10 sec
#define FIVE_SEC        5000000L  // ms = 5 sec
#define THREE_SEC       3000000L  // ms = 3 sec

#define REGISTER_TIME THREE_SEC

#define DEFAULT_TIMEOUT TEN_SEC
#define DEFAULT_CLOSE   THREE_SEC

#define SET_TV(A, B) A.tv_sec = (B / TIME_ONE_MILLISECOND); \
                    A.tv_usec = (B % TIME_ONE_MILLISECOND);

#define MY_NAME "CM MAMANGER"
#define MY_PTYPE LCS_COMMAND_MANAGER
#define MY_ITYPE COMMAND_MANAGER
/************/

/*
 * Description : Initialize function for cmicy manager
 */
void cmInitProcess(void);

/*
 * Description : Terminnation function for cmicy manager
 */
void cmTermProcess(void);

/*
 * Description : Restarting function for cmicy manager
 */
void cmRestartProcess(void);

/*
 * Description : Holding function for cmicy manager
 */
void cmHoldProcess(void);

/*
 * Description : Signal processing function
 *
 * param [in] sig : signal id
 */
void cmSignalProcess(Int32T sig);

/*
 * Description : Stop precess
 *
 */
void cmStop(void);

/*
 * Description : Stop precess
 *
 */

void closeTimerCallback(Int32T fd, Int16T event, void *pArg);

#endif /* _cmMain_h */

