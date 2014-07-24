#if !defined(_polMgrMain_h)
#define _polMgrMain_h

/*3456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*******************************************************************************
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
 * @file:
 *
 * $Author:
 * $Date:
 * $Revision:
 * $LastChangedBy:
 */

#include "nnTypes.h"
#include "lcsService.h"

#define TEN_SEC         10000000L // ms = 10 sec
#define FIVE_SEC        5000000L  // ms = 5 sec
#define THREE_SEC       3000000L  // ms = 3 sec
#define ONE_SEC         1000000L  // ms = 1 sec

#define REGISTER_TIME THREE_SEC

#define DEFAULT_TIMEOUT TEN_SEC
#define DEFAULT_CLOSE   THREE_SEC

#define SET_TV(A, B) A.tv_sec = (B / TIME_ONE_MILLISECOND); \
                    A.tv_usec = (B % TIME_ONE_MILLISECOND);

#define MY_NAME "POLICY"
#define MY_PTYPE LCS_POLICY_MANAGER /**< Process Type(lcsService.h) **/
#define MY_ITYPE POLICY_MANAGER        /**< IPC Type(nnDefines.h) **/

typedef struct polMgr
{
  void * gCmshGlobal;      /** Component Cmd Variable **/

  /** TODO: Add more here */
  void * gOamTimerEvent;   /** Component Timer Variable **/
  void * gTcpFdEvent;      /** Component Fd Varialbe **/

} polMgrT;

LcsServiceStatusEventT statusEvent;

//extern polMgrT *gPolMgrBase;

/*
 * Description : Initialize function for policy manager
 */
void polInitProcess(void);

/*
 * Description : Terminnation function for policy manager
 */
void polTermProcess(void);

/*
 * Description : Restarting function for policy manager
 */
void polRestartProcess(void);

/*
 * Description : Holding function for policy manager
 */
void polHoldProcess(void);

/*
 * Description : Signal processing function
 *
 * param [in] sig : signal id
 */
void polSingalProcess(Int32T sig);

/*
 * Description : Stop precess
 *
 */

void closeTimerCallback(Int32T fd, Int16T event, void *pArg);

#endif /* _polMain_h */

