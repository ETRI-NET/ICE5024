#include <stdio.h>
#include "nnCmdDefines.h"
#include "ipcmgrInternal.h"

extern void nosTaskLibInitialize();
extern Int32T nosIpcmgrScheduleInitialize();
extern void nosCompInitialize();
extern void nosCmdInitialize(Int32T cmshID);

#define COMP_NAME "ipcmgr"
#define COMP_VER "0.0.1"

Int32T main(Int32T argc, char **argv)
{
	// initialize dynamic module & task lib 
	nosTaskLibInitialize(COMP_NAME, COMP_VER);

	// create nos main task
	taskCreate(nosCompInitialize);

	// schedule work(ipc/event/timer) 
    nosIpcmgrScheduleInitialize();

    // CM IPC 
    //nosCmdInitialize(IPC_IPC_MGR);

	// Task start
	taskDispatch();

	return(0);
}
