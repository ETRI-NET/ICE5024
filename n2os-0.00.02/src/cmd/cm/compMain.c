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
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>

#include "taskManager.h"
#include "nnCmdDefines.h"
#include "compStatic.h"

extern void nosTaskLibInitialize();
extern void nosTaskScheduleInitialize();
extern void nosCompInitialize();
extern void nosCmdInitialize(Int32T cmshID);
#if 0
extern void nosCmshProcess();
extern void nosCompProcess();

extern void nosCmCmdInit();
#endif
#define COMP_NAME "cm"
#define COMP_VER "0.0.1"
#if 0
void *gTimerTask = NULL;

void
timerCallback(Int32T fd, Int16T event, void * arg)
{
  nosCmCmdInit(IPC_CMSH_MGR, IPC_CM_MGR, 
        nosCmshProcess, nosCompProcess);
  _taskTimerDel(gTimerTask);
}
#endif

Int32T main(Int32T argc, char **argv)
{
	//find inetd start nos.conf and kill
	nosMainServiceFree();

	//initialize dynamic module & task lib 
	nosTaskLibInitialize(COMP_NAME, COMP_VER);

	//create nos main task
	taskCreate(nosCompInitialize);

	// schedule work(ipc/event/timer) 
	nosTaskScheduleInitialize();
#if 0
  // CM IPC
  struct timeval tv = {2, 0};
  gTimerTask = _taskTimerSet(timerCallback, tv, TASK_PERSIST, NULL);
#endif
	nosCmdInitialize(IPC_CMSH_MGR);
	// Task start
	taskDispatch();

	return(0);
}
