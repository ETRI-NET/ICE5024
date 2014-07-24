#include <stdio.h>

#include "nosLib.h"
#include "ipcmgrInit.h"
#include "nnCmdDefines.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCompProcess.h"

Int32T ipcmgrWriteConfCB
 (struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1, Int32T uargc2, Int8T **uargv2,
  Int32T uargc3, Int8T **uargv3, Int32T uargc4, Int8T **uargv4, 
  Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv)
{

    return CMD_IPC_OK;
}


void
ipcmgrCmIpcProcess(Int32T sockId, void *message, Uint32T size)
{
    compCmdIpcProcess(pIpcmgr->pCmshGlobal, sockId, message, size);
}

void
ipcmgrCmshIpcProcess(Int32T sockId, void *message, Uint32T size)
{
    compCmdIpcProcess(pIpcmgr->pCmshGlobal, sockId, message, size);
}

