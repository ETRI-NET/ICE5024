/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : RIB Manager???? ?????Ï´? ???? ?????? ??Á¶ ?? ?Ê±?È­ ?Ô¼??? Á¤???? ????
 * 
 * - Block Name : RIB Manager
 * - Process Name : ribmgr
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : ribmgrInit.c
 *
 * $Author: sckim007 $
 * $Date: 2014-02-17 09:53:56 +0900 (Mon, 17 Feb 2014) $
 * $Revision: 860 $
 * $LastChangedBy: sckim007 $
 */
#include <stdlib.h>
#include <sys/socket.h>
#include <bits/sockaddr.h>

#include <linux/netlink.h>
#include "nnTypes.h"
#include "nnStr.h"
#include "nnVector.h"
#include "nnList.h"
#include "nnPrefix.h"
#include "nnBuffer.h"

#include "nnCmdDefines.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCompProcess.h"

#include "nosLib.h"

/*
 * External Definitions for Global Data Structure
 */
extern void ** gCompData;


/*
 * External Definitions are here
 */
extern void initMenuThread();
extern Int8T routeListCmpFunc(void  * pOldData, void * pNewData);

extern ListT * pRouteList;



/*
 * Rib manager's data structure using in shared code.
 */


Int32T
WriteConfCB(struct cmsh *cmsh, Int32T uargc, Int8T **uargv, Int32T cargc, Int8T **cargv)
{
  return CMD_IPC_OK;
}


void
testerInitProcess()
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);
  printf("PROCESS :: %s called.\n", __func__);

  pRouteList = nnListInit (routeListCmpFunc, MEM_ROUTE_RIB);

  /* menu thread. */
  initMenuThread();
}


/*
 * Description : 
 */
void
testerTermProcess ()
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);
}

/*
 * Description : 
 */
void
testerRestartProcess ()
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);
}


/*
 * Description : 
 */
void
testerHoldProcess ()
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);
}


/*
 * Description : every one second
 */
void
testerTimerProcess (Int32T fd, Int16T event, void * arg)
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);
}


/**
 * Description : Signal comes up, This Callback is called.
 *
 * @retval : none
 */
void 
testerSignalProcess(Int32T signalType)
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);
  taskClose();
  nnLogClose();
  memClose();
}

void
testerIpcProcess (Int32T msgId, void * data, Uint32T dataLen)
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);
}

void  
testerEventProcess (Int32T msgId, void * data, Uint32T dataLen)
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);
}

/**
 * Description : Command manager channel
 *
 * @retval : none
 */
void 
testerCmIpcProcess(Int32T sockId, void *message, Uint32T size)
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);
}


/**
 * Description : Command shell channel
 *
 * @retval : none
 */
void
testerCmshIpcProcess (Int32T sockId, void *message, Uint32T size)
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);
}


