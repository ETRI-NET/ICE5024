#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nnTypes.h"
#include "nosLib.h"

#include "ipcmgrInit.h"
#include "nnCmdDefines.h"
#include "nnCompProcess.h"

extern Int32T ipcmgrWriteConfCB
 (struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1, Int32T uargc2, Int8T **uargv2,
  Int32T uargc3, Int8T **uargv3, Int32T uargc4, Int8T **uargv4,
  Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);


//component ipc process
void ipcmgrIpcProcess(Int32T msgId, void * data, Uint32T dataLen)
{
	NNLOG(LOG_DEBUG, "enter %s\n", __FUNCTION__);

    /** Component Command Init **/
    if(msgId == IPC_LCS_PM2C_COMMAND_MANAGER_UP)
    {
        if(pIpcmgr->pCmshGlobal != NULL)
        {
            pIpcmgr->pCmshGlobal 
             = compCmdInit(IPC_IPC_MGR, ipcmgrWriteConfCB);
        }
    }

	return;
}

