/*3456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*********************************.*********************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute.
*           All rights reserved. No part of this software shall be reproduced,
*           stored in a retrieval system, or transmitted by any means,
*           electronic, mechanical, photocopying, recording, or otherwise,
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief : This file include major functions for policy manager.
 *  - Block Name : Policy Manager
 *  - Process Name : cmmgr
 *  - Creator : PyungKoo Park
 *  - Initial Date : 2014/03/03
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
#include "nnTypes.h"

#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"
#include "nnCmdDefines.h"

#include "nosLib.h"
#include "cmMgrMain.h"
#include "cmMgrCmIpc.h"
#include "lcsService.h"

#define REGISTER 1

cmMgrT *gCmIpcBase = NULL;
//global Definitions 
extern void** gCompData;
void** gComDataCmsh = NULL;

extern Int32T cmMainInit(void);
extern void cmMainFree(void);
extern void cmdFuncGlobalInstall(struct cmsh *cmsh);

void registerTimerCallback(Int32T fd, Int16T event, void *arg);
void *timeEv = NULL;

//initialize component
void cmInitProcess(void)
{
  fprintf(stdout, "enter %s\n", __FUNCTION__);

  struct timeval registerTv;
  SET_TV(registerTv, REGISTER_TIME);

  gCmIpcBase = NNMALLOC(MEM_GLOBAL, sizeof(struct cm));
  cmMainInit();

  /* Assign shared memory pointer to global memory pointer. */
  (*gCompData) = (void *)gCmIpcBase;

#ifdef REGISTER
    /*
     * For Send Init Message
     */
    timeEv = (void *)taskTimerSet((void *)registerTimerCallback, registerTv, 0, NULL);

    fprintf(stdout, "Add Timer : RegisterTimerCallback\n");
#endif

}

//restart component
void cmRestartProcess(void)
{
  fprintf(stdout, "enter %s\n", __FUNCTION__);
  gCmIpcBase = (cmMgrT *)(*gCompData);
	if(gCmIpcBase->cmsh == NULL){
		gCmIpcBase->cmsh = cmdCmshInit(IPC_CM_MGR);
	}else{
		cmdCmshFree(gCmIpcBase->cmsh);
		gCmIpcBase->cmsh = cmdCmshInit(IPC_CM_MGR);
	}
	cmdFuncGlobalInstall(gCmIpcBase->cmsh);
}

//hold component
void cmHoldProcess(void)
{
  fprintf(stdout, "enter %s\n", __FUNCTION__);
}

//terminate component

void
cmTermProcess(void) 
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);
  /** Component Command Close */
  taskClose();
  nnLogClose();
  memClose();  
}

//signal catch proces
void cmSignalProcess(Int32T sig)
{
  fprintf(stderr, "  Signal(%d) Occured !!\n", sig);

  cmStop ();

  cmTermProcess();
}

//Stop component
void cmStop(void)
{
  fprintf(stdout, "enter %s\n", __FUNCTION__);
  cmMainFree();
}

void closeTimerCallback(Int32T fd, Int16T event, void *pArg)
{
  fprintf(stdout, "I'm %s : %s\n", MY_NAME, __func__);

  cmStop ();
  cmTermProcess();
}

void registerTimerCallback(Int32T fd, Int16T event, void *arg)
{
    LcsAttributeT   lcsAttribute = {0,};
    Int32T ret = 0;

    fprintf(stdout, "I'm %s : %s\n", MY_NAME, __func__);
    fprintf(stdout, "[%s] Send Async Message for Register\n", __func__);

    ret = lcsRegister(MY_PTYPE, MY_ITYPE, lcsAttribute);
    fprintf(stdout, "[%s] Async Result : %d\n", __func__, ret);
}

