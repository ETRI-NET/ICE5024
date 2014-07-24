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
#include <string.h>
#include "nosLib.h"

#include "nnCmdDefines.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCompProcess.h"

#include "dluTesterMain.h"

extern dluTestT *gDluTestBase;

void
dluTesterCmIpcProcess(Int32T sockId, void *message, Uint32T size)
{
  compCmdIpcProcess(gDluTestBase->gCmshGlobal, sockId, message, size);
}

void
dluTesterCmshIpcProcess(Int32T sockId, void *message, Uint32T size)
{
  compCmdIpcProcess(gDluTestBase->gCmshGlobal, sockId, message, size);
}


Int32T
dluTesterWriteConfCB (struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1, 
                      Int32T uargc2, Int8T **uargv2, Int32T uargc3, Int8T **uargv3, 
                      Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, 
                      Int32T cargc, Int8T **cargv)
{
  /** TODO: Write Config Here */
  /** FOR TEST */
  Int32T i;
#if 0
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
#endif
	for(i = 0; i < 10; i++)
   {
		Int8T buff_send[1024];
		memset(buff_send, 0x00, 1024);
		sprintf(buff_send,"! test router rip %d", i);
		cmdPrint(cmsh,buff_send);
		sprintf(buff_send,"!interface %d", i);      //change current node to "interface %d" node.
		cmdPrint(cmsh,buff_send);
		sprintf(buff_send," test node %d", i);
		cmdPrint(cmsh,buff_send);
		cmdPrint(cmsh,"!");        //change current node to "configure-terminal" node.
		sprintf(buff_send," test running %d", i);
		cmdPrint(cmsh,buff_send);
	}
  return CMD_IPC_OK;
}

