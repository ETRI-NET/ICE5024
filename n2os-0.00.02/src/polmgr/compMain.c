/*345678901234567890123456789012345678901234567890123456789012345678901234567890*/
/*********************************************************************************
 *               Electronics and Telecommunications Research Institute
 * Filename : 
 * Blockname: 
 * Overview : 
 * Creator  : 
 * Owner    : 
 * Copyright: 2013 Electronics and Telecommunications Research Institute. 
 *            All rights reserved. No part of this software shall be reproduced, 
 *            stored in a retrieval system, or transmitted by any means, 
 *            electronic, mechanical, photocopying, recording, or otherwise, 
 *            without written permission from ETRI.
 *********************************************************************************/

/*********************************************************************************
 *                        VERSION CONTROL  INFORMATION
 * $Author$
 * $Date$
 * $Revision
 * $Log$ 
 *********************************************************************************/

#include <stdio.h>
#include "taskManager.h"
#include "nnCmdDefines.h"

extern void nosTaskLibInitialize();
extern void nosTaskScheduleInitialize();
extern void nosCompInitialize();
extern void nosCmdInitialize(Int32T cmshID);

#define COMP_NAME "pol"
#define COMP_VER "0.0.1"

/*
extern void nosCmshProcess();
extern void nosCompProcess();
extern void nosCompCmdInit();

void *gTinerTask = NULL;

void 
timerCallback(Int32T fd, Int16T event, void * arg)
{
  nosCompCmdInit(IPC_POL_MGR, IPC_CM_MGR, 
          nosCmshProcess, nosCompProcess);
  _taskTimerDel(gTimerTask);
}
*/
Int32T main(Int32T argc, char **argv)
{
	// initialize dynamic module & task lib 
	nosTaskLibInitialize(COMP_NAME, COMP_VER);

	// create nos main task
	taskCreate(nosCompInitialize);
  
	// schedule work(ipc/event/timer) 
	nosTaskScheduleInitialize();

    // CM IPC 
    nosCmdInitialize(IPC_POL_MGR);
/*
  // CM IPC
  struct timeval tv = {2, 0};
  gTimerTask = _taskTimerSet(timerCallback, tv, TASK_PERSIST, NULL);
*/
	// Task start
	taskDispatch();

	return(0);
}
