#include <stdio.h>
#include "taskManager.h"
#include "nnCmdDefines.h"

extern void nosTaskLibInitialize();
extern void nosTaskScheduleInitialize();
extern void nosCompInitialize();
extern void nosCmdInitialize(Int32T cmshID);

#define COMP_NAME "dluTester"
#define COMP_VER "0.0.1"

Int32T main(Int32T argc, char **argv)
{
	// initialize dynamic module & task lib 
	nosTaskLibInitialize(COMP_NAME, COMP_VER);

	// create nos main task
	taskCreate(nosCompInitialize);
  
	// schedule work(ipc/event/timer) 
	nosTaskScheduleInitialize();

    // CM IPC 
    nosCmdInitialize(IPC_DLU_TESTER);

	// Task start
	taskDispatch();

	return(0);
}
