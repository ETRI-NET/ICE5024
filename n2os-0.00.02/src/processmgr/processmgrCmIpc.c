/*3456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*********************************.*********************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute.
*           All rights reserved. No part of this software shall be reproduced,
*           stored in a retrieval system, or transmitted by any means,
*           electronic, mechanical, photocopying, recording, or otherwise,
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief : This file include ipc, event and signal process functions.
 *  - Block Name : CM IPC manager
 *  - Process Name : cmmgr
 *  - Creator : Thanh Nguyen Ba
 *  - Initial Date : 2014/03/18
 */

/**
 * @file        :
 *
 * $Author: $
 * $Date: $
 * $Revision: $
 * $LastChangedBy: $
 */

#include <stdio.h>

#include "nosLib.h"

#include "nnCmdDefines.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCompProcess.h"

#include "processmgrMain.h"

extern LcsProcessMgrT *gpProcMgrBase;

void
procCmIpcProcess(Int32T sockId, void *message, Uint32T size)
{
  compCmdIpcProcess(gpProcMgrBase->gpCmshGlobal, sockId, message, size);
}

void
procCmshIpcProcess(Int32T sockId, void *message, Uint32T size)
{
  compCmdIpcProcess(gpProcMgrBase->gpCmshGlobal, sockId, message, size);
}


Int32T
procMgrWriteConfCB (struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv)
{
  /** TODO: Write Config Here */
  /** FOR TEST */
  Int32T i;

  for(i = 0; i < 10; i++)
  {
    Int8T buff_send[1024];
    sprintf(buff_send,"! test running %d", i);
    cmdPrint(cmsh,buff_send);
    sprintf(buff_send,"! test router rip %d", i);
    cmdPrint(cmsh,buff_send);
    sprintf(buff_send,"!interface %d", i);
    cmdPrint(cmsh,buff_send);
    sprintf(buff_send," test node %d", i);
    cmdPrint(cmsh,buff_send);
     
  }

  return CMD_IPC_OK;
}

