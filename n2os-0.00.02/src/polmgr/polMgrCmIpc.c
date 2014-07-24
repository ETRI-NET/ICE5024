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

#include "polMgrMain.h"
//#include "polMgrCmIpc.h"


extern polMgrT *gPolMgrBase;

void
polCmIpcProcess(Int32T sockId, void *message, Uint32T size)
{
  compCmdIpcProcess(gPolMgrBase->gCmshGlobal, sockId, message, size);
}

void
polCmshIpcProcess(Int32T sockId, void *message, Uint32T size)
{
  compCmdIpcProcess(gPolMgrBase->gCmshGlobal, sockId, message, size);
}


Int32T
polMgrWriteConfCB (struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv)
{
  /** TODO: Write Config Here */
  /** FOR TEST */
  Int32T i;

  for(i = 0; i < 10; i++)
  {
    Int8T buff_send[1024];
    //"!" were used with below Normal-Command to indicate that this Normal-Command belong to "configure-terminal" node.
#if 1 
    cmdPrint(cmsh,"!");
    sprintf(buff_send,"test router rip %d", i);
   cmdPrint(cmsh,buff_send);
 
    cmdPrint(cmsh,"!");
    //below Node-Command changes current node to "interface %d" node.
    sprintf(buff_send,"interface %d", i);
    cmdPrint(cmsh,buff_send);
    //below Normal-Command will be inserted under "interface %d" node since current node had been change to "interface %d".
    sprintf(buff_send, "test %d", i);
    cmdPrint(cmsh,buff_send);
    sprintf(buff_send,"test node %d", i);
    cmdPrint(cmsh,buff_send);
	sprintf(buff_send,"test-mul %d", i);
	cmdPrint(cmsh, buff_send);
	sprintf(buff_send,"test mul %d", i);
    cmdPrint(cmsh, buff_send);
	sprintf(buff_send, "do show clock");
    cmdPrint(cmsh,buff_send);
//	cmdPrint(cmsh, "  exit");
//    cmdPrint(cmsh,"!");
//	sprintf(buff_send," interface %d", i);
//    cmdPrint(cmsh,buff_send);
//    cmdPrint(cmsh, "  exit");
    //below Node-Command changes current node to "configure-terminal" node.
//    cmdPrint(cmsh,"!");
    //below Normal-Command will be inserted under "configure-terminal" node since current node had been change back to "configure-terminal" node.
//    sprintf(buff_send," test running %d", i);
//    cmdPrint(cmsh,buff_send);

	cmdPrint(cmsh,"!");
	sprintf(buff_send,"test running %d", i);
    cmdPrint(cmsh,buff_send);

	 cmdPrint(cmsh,"!");
	sprintf(buff_send,"interface %d", i);
    cmdPrint(cmsh,buff_send);
//    cmdPrint(cmsh,"exit");
#endif
#if 1 
    cmdPrint(cmsh, "!");
	sprintf(buff_send, "key chain %d", i);
	cmdPrint(cmsh, buff_send);

	sprintf(buff_send, "key %d", i);
	cmdPrint(cmsh, buff_send);
	//cmdPrint(cmsh, "  exit");
	 sprintf(buff_send, "keya %d", i+2);
    cmdPrint(cmsh, buff_send);
	//cmdPrint(cmsh, "  exit");
	sprintf(buff_send, "keyb %d", i+1);
	cmdPrint(cmsh, buff_send);
	//cmdPrint(cmsh, "  exit");

//	cmdPrint(cmsh, "!");
   // sprintf(buff_send, "key chain %d", i);
   // cmdPrint(cmsh, buff_send);
    //cmdPrint(cmsh, "  exit");    
/*
	cmdPrint(cmsh, "!");
    sprintf(buff_send, "key chain %d", i);
	cmdPrint(cmsh, buff_send);
	cmdPrint(cmsh, "exit"); 
*/
#endif
  }

  return CMD_IPC_OK;
}

