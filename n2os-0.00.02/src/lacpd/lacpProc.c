/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : LACP Protocol
 * 
 * - Block Name : LACP Protocol
 * - Process Name : lacpd
 * - Creator : Jaeho You
 * - Initial Date : 2014/06/02/
 */

/**
 * @file        : lacpInit.c
 *
 * $Author: sckim007 $
 * $Date: 2014-05-20 12:06:42 +0900 (2014-05-20, 화) $
 * $Revision: 1595 $
 * $LastChangedBy: sckim007 $
 */

#include "nnTypes.h"
#include "nnStr.h"
#include "nnVector.h"
#include "nnList.h"
#include "nnPrefix.h"
#include "nnBuffer.h"
#include "nosLib.h"

#include "nnCmdDefines.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCompProcess.h"

#if 0
#include "lcsService.h"
#endif

#include <signal.h>
#include <sys/eventfd.h>

#include "lacpDef.h"

/*
 * External Definitions for Global Data Structure
 */
extern void ** gCompData;
LacpT * pLacp = NULL;

static void lacpFiniProcess(void)
{
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp) {
		if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
		compCmdFree(pLacp->pCmshGlobal);
		NNFREE (MEM_GLOBAL, pLacp);
	}

	taskClose();
	nnLogClose();
	memClose();
}

static void lacpSigFdDel (void)
{
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp) {
		if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
		nnListFree(pLacp->pLacpGroupFiniList);
		taskFdDel (pLacp->sigIntEventTask);
		taskFdDel (pLacp->sigTermEventTask);
		taskFdDel (pLacp->sigQuitEventTask);
		pLacp->sigIntEventTask = NULL;
		pLacp->sigTermEventTask = NULL;
		pLacp->sigQuitEventTask = NULL;
		close (pLacp->sigIntEventFd);
		close (pLacp->sigTermEventFd);
		close (pLacp->sigQuitEventFd);
		pLacp->sigIntEventFd = -1;
		pLacp->sigTermEventFd = -1;
		pLacp->sigQuitEventFd = -1;
	}
}

static void lacpExitProcess(void)
{
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp) {
		if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
		pLacp->quit = 1;
		if (pLacp->quit && pLacp->pLacpGroupContextList->pHead == NULL) {
			lacpFiniProcess();
			_exit (0);
		}
		lacpGroupFini ();
	}
}

static void sigIntEventCallback(Int32T fd, Int16T event, void *arg)
{
	Uint64T x;
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp) {
		if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	}
	if (read(pLacp->sigIntEventFd, &x, sizeof(x)) < 0) {
	/* silence warn_unused_result */
	}
	lacpExitProcess ();
}

static void
sigInt(Int32T signum)
{
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	Uint64T x = 1;
	if (write(pLacp->sigIntEventFd, &x, sizeof(x)) < 0) {
	/* silence warn_unused_result */
	}
}

void sigIntInit(void)
{
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	pLacp->sigIntEventFd = -1;
	pLacp->sigIntEventTask = NULL;
	if ((pLacp->sigIntEventFd = eventfd(0, 0)) < 0) {
		DEBUGPRINT("%s :: error\n", __FUNCTION__);
		return;
	}
	signal(SIGINT, sigInt);
	pLacp->sigIntEventTask = taskFdSet (sigIntEventCallback, pLacp->sigIntEventFd, TASK_READ, TASK_PRI_MIDDLE, pLacp);
}

static void sigTermEventCallback(Int32T fd, Int16T event, void *arg)
{
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	Uint64T x;
	if (read(pLacp->sigTermEventFd, &x, sizeof(x)) < 0) {
	/* silence warn_unused_result */
	}
	lacpExitProcess ();
}

static void
sigTerm(Int32T signum)
{
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	Uint64T x = 1;
	if (write(pLacp->sigTermEventFd, &x, sizeof(x)) < 0) {
	/* silence warn_unused_result */
	}
}

void sigTermInit(void)
{
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	pLacp->sigTermEventFd = -1;
	pLacp->sigTermEventTask = NULL;
	if ((pLacp->sigTermEventFd = eventfd(0, 0)) < 0) {
		DEBUGPRINT("%s :: error\n", __FUNCTION__);
		return;
	}
	signal(SIGTERM, sigTerm);
	pLacp->sigTermEventTask = taskFdSet (sigTermEventCallback, pLacp->sigTermEventFd, TASK_READ, TASK_PRI_MIDDLE, pLacp);
}

static void sigQuitEventCallback(Int32T fd, Int16T event, void *arg)
{
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	Uint64T x;
	if (read(pLacp->sigQuitEventFd, &x, sizeof(x)) < 0) {
	/* silence warn_unused_result */
	}
	lacpExitProcess ();
}

static void
sigQuit(Int32T signum)
{
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	Uint64T x = 1;
	if (write(pLacp->sigQuitEventFd, &x, sizeof(x)) < 0) {
	/* silence warn_unused_result */
	}
}

void sigQuitInit(void)
{
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	pLacp->sigQuitEventFd = -1;
	pLacp->sigQuitEventTask = NULL;
	if ((pLacp->sigQuitEventFd = eventfd(0, 0)) < 0) {
		DEBUGPRINT("%s :: error\n", __FUNCTION__);
		return;
	}
	signal(SIGQUIT, sigQuit);
	pLacp->sigQuitEventTask = taskFdSet (sigQuitEventCallback, pLacp->sigQuitEventFd, TASK_READ, TASK_PRI_MIDDLE, pLacp);
}

void lacpDevFiniStart (void *context, struct timeval *tv, void *callback)
{
	void *timerEvent;

	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	timerEvent = taskTimerSet(callback, *tv, 0, context);
	DEBUGPRINT("%s ev=%p (tv=%ld.%ld)\n", __FUNCTION__, timerEvent, tv->tv_sec, tv->tv_usec);
}

void lacpDevFiniEnd (void *context)
{
	ListNodeT *pNode;
	LacpGroupFiniT *pFini;

	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	if (! pLacp->quit)
		return;

	for(pNode = pLacp->pLacpGroupFiniList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pFini = pNode->pData;
		if (pFini->context == (long) context) {
			DEBUGPRINT("%s %lx\n", __FUNCTION__, pFini->context);
			nnListDeleteNode (pLacp->pLacpGroupFiniList, pFini);
			break;
		}
	}
	if (pLacp->pLacpGroupFiniList->pHead == NULL) {
		lacpSigFdDel ();
		lacpFiniProcess();
		_exit (0);
	}
}

/*
 * Description : When we display running config, this function will be called.
 */
static Int32T
lacpWriteConfCB(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv)
{
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp) {
		if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	}
	configWriteLacp (cmsh);

	return CMD_IPC_OK;
}

/*
 * Description :
 */
void lacpInitProcess(void)
{
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);

#if 0
	LcsAttributeT lcsAttribute = {0,};
#endif

	pLacp = NNMALLOC (MEM_GLOBAL, sizeof(*pLacp));
	if (pLacp) {
		pLacp->debug = 1;
		pLacp->quit = 0;
		if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
		pLacp->pCmshGlobal = compCmdInit (IPC_LACP, lacpWriteConfCB);
		sigIntInit();
		sigTermInit();
		sigQuitInit();
		lacpGroupInit();
	}

	/* Assign shared memory pointer to global memory pointer. */
	(*gCompData) = (void *)pLacp;

#if 0
	lcsRegister(LCS_LACP, LACP, lcsAttribute);
#endif
}

/*
 * Description : 
 */
void lacpTermProcess (void)
{
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp) {
		if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	}
	lacpExitProcess();
}

/*
 * Description : 
 */
void lacpRestartProcess (void)
{
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp) {
		if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	}

	/* Assign global memory pointer to shared memory pointr. */
	pLacp = (LacpT *)(*gCompData);

#if 0	//(procmgr)
	ListNodeT *pNode;
	LacpGroupContextT *pContext;

	for(pNode = pLacp->pLacpGroupContextList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pContext = pNode->pData;
		DEBUGPRINT("%s %p\n", __FUNCTION__, pContext);
		lacpDevCallbackChange(pContext);
	}
#endif
}


/*
 * Description : 
 */
void
lacpHoldProcess (void)
{
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp) {
		if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	}
}

/**
 * Description : Command manager channel
 *
 * @retval : none
 */
void
lacpCmIpcProcess(Int32T sockId, void *message, Uint32T size)
{
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp) {
		if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
		compCmdIpcProcess(pLacp->pCmshGlobal, sockId, message, size);
	}
}

/**
 * Description : Command shell channel
 *
 * @retval : none
 */
void
lacpCmshIpcProcess (Int32T sockId, void *message, Uint32T size)
{
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp) {
		if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
		compCmdIpcProcess(pLacp->pCmshGlobal, sockId, message, size);
	}
}

/**
 * Description : Signal comes up, This Callback is called.
 *
 * @retval : none
 */
void 
lacpSignalProcess(Int32T signalType) 
{
	DEBUGPRINT("%s :: %u\n", __FUNCTION__, signalType);
	if (pLacp) {
		if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	}
}
