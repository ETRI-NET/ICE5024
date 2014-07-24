/**
 * @file      : nnCmdCmsh.c
 * @brief       :
 *
 * $Id: nnCmdMd5.c 725 2014-01-17 09:11:34Z hyryu $
 * $Author: hyryu $
 * $Date: 2014-01-17 04:11:34 -0500 (Fri, 17 Jan 2014) $
 * $Log$
 * $Revision: 725 $
 * $LastChangedBy: hyryu $
 * $LastChanged$
 *
 *                      Electronics and Telecommunications Research Institute
 * Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.
 *           No part of this software shall be reproduced, stored in a retrieval system, or
 *           transmitted by any means, electronic, mechanical, photocopying, recording,
 *           or otherwise, without written permission from ETRI.
 **/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "nnTypes.h"
#include "nnDefines.h"
#include "nnLog.h"

#include "nnCmdCommon.h"
#include "nnCmdLink.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"
#include "nnCmdCmsh.h"

extern void cmdGlobalInstall(struct cmsh *cmsh);
void
cmdCmshBaseFree(struct cmsh *cmsh)
{
  if(cmsh != NULL)
  {
    free(cmsh);
  }
}
void
cmdCmshFree(struct cmsh *cmsh)
{
  if(cmsh != NULL)
  {
    if(cmsh->callbackList)
    {
      cmdListDel(cmsh->callbackList);
    }
    if(cmsh->ctree)
    {
      free(cmsh->ctree);
    }
  }
  cmdCmshBaseFree(cmsh);
}

struct cmsh *
cmdCmshBaseInit(Int32T gIndex,Int32T zIndex)
{
  struct cmsh *cmsh;
  Int32T i;
  if((cmsh = (struct cmsh *)malloc(sizeof *cmsh)) == NULL)
  {
    return NULL;
  }
  memset(cmsh, 0, sizeof(*cmsh));
  cmsh->gTypes = gIndex;
  cmsh->zTypes = zIndex;
  cmsh->argc = 0;
  for(i = 0; i < CMSH_ARGC_MAX; i++)
  {
    cmsh->argv[i] = NULL;
  }
  cmsh->outMod = CMD_OUTMOD_NULL;
	cmsh->outputStr = NULL;
	cmsh->outputSize = 0;
  return cmsh;
}

struct cmsh *
cmdCmshInit(Int32T gIndex)
{
  struct cmsh *gCmshPtr = cmdCmshBaseInit(gIndex, 0);
  if(gCmshPtr == NULL)
  {
    return NULL;
  }
  /** Init Ctree    */
  gCmshPtr->ctree = (struct cmdTree *)malloc(sizeof(struct cmdTree));
  if(gCmshPtr->ctree == NULL)
  {
    cmdCmshFree(gCmshPtr);
    return NULL;
  }
  memset(gCmshPtr->ctree, 0, sizeof(struct cmdTree));
  gCmshPtr->ctree->cmsh = gCmshPtr;
  
  gCmshPtr->oamFd = gCmshPtr;
  gCmshPtr->oamSock = 0;

  /** Current Depth */
  gCmshPtr->currentDepth = 0;
  Int32T i;
  for(i = 0; i < CMD_MAXNUM_MODEDEPTH ; i++)
  {
      gCmshPtr->modeTrace[i].mode =  CMD_NODE_NULL;
      gCmshPtr->modeTrace[i].modeArgc = 0;
  }
  gCmshPtr->prompt = gCmshPtr->ctree->prompt[gCmshPtr->modeTrace[gCmshPtr->currentDepth].mode];
  gCmshPtr->prevMode = CMD_NODE_NULL;
  gCmshPtr->userPrivilege = PRIVILEGE_MIN;
  gCmshPtr->userKey = 1;
  gCmshPtr->cnode = &gCmshPtr->ctree->modeTree[gCmshPtr->modeTrace[gCmshPtr->currentDepth].mode];
  gCmshPtr->matchNum = 0;
	/**	Init List of Tree	*/
	gCmshPtr->callbackList = cmdListNew();
  /** Init Tree */
  cmdGlobalInstall(gCmshPtr);
  return gCmshPtr;
}

Int32T
cmdOutPuts(void *outFd, Int8T *outStr,...)
{
  struct cmsh *cmsh = (struct cmsh *)(outFd);
  Int8T szBuffer[CMD_IPC_MAXLEN];
  Int8T szBufferTlv[CMD_IPC_MAXLEN];
  Int32T nbytes;
  va_list args;
  cmdIpcMsg_T cmIpcHdr;
  memset(&cmIpcHdr, 0, sizeof(cmdIpcMsg_T));
  if(cmsh->outMod == CMD_OUTMOD_NULL)
  {
    cmsh->outMod = CMD_OUTMOD_BEGIN;
  }
  else if(cmsh->outMod == CMD_OUTMOD_BEGIN)
  {
    cmsh->outMod = CMD_OUTMOD_INCLUDE;
  }
  bzero(&szBuffer, CMD_IPC_MAXLEN);
  cmIpcHdr.requestKey = cmsh->requestKey;
  cmIpcHdr.code = CMD_IPC_OUTPUT_DATA;
  cmIpcHdr.srcID = cmsh->gTypes;
  cmIpcHdr.dstID = cmsh->zTypes;
  cmdIpcAddHdr(szBuffer, &cmIpcHdr);    /**  Add Header  */
  cmdIpcAddTlv(szBuffer, CMD_IPC_TLV_OUTPUT_TYPE, sizeof(Int32T), &cmsh->outMod);
  if(outStr != NULL)
  {
    bzero(&szBufferTlv, CMD_IPC_MAXLEN);
    va_start(args, outStr);
    vsprintf (szBufferTlv, outStr, args);
    va_end(args);
    cmdIpcAddTlv(szBuffer, CMD_IPC_TLV_OUTPUT_DATA, strlen(szBufferTlv), szBufferTlv);
  }
  if((nbytes = cmdIpcSend(cmsh->cmfd, szBuffer)) > 0)
    return CMD_IPC_OK;
  else
    return CMD_IPC_ERROR;
}

Int32T
cmdOamPuts(void *outFd, Int8T *outStr,...)
{
  struct cmsh *cmsh = (struct cmsh *)(outFd);
  Int8T szBuffer[CMD_IPC_MAXLEN];
  Int8T szBufferTlv[CMD_IPC_MAXLEN];
  Int32T nbytes;
  va_list args;
  cmdIpcMsg_T cmIpcHdr;
  memset(&cmIpcHdr, 0, sizeof(cmdIpcMsg_T));
  bzero(&szBuffer, CMD_IPC_MAXLEN);
  cmIpcHdr.requestKey = cmsh->requestKey;
  cmIpcHdr.code = CMD_IPC_OAM_DATA;
  cmIpcHdr.srcID = cmsh->gTypes;
  cmIpcHdr.dstID = cmsh->zTypes;
  cmdIpcAddHdr(szBuffer, &cmIpcHdr);    /**  Add Header  */
  if(outStr != NULL)
  {
    bzero(&szBufferTlv, CMD_IPC_MAXLEN);
    va_start(args, outStr);
    vsprintf (szBufferTlv, outStr, args);
    va_end(args);
    cmdIpcAddTlv(szBuffer, CMD_IPC_TLV_OAM_DATA, strlen(szBufferTlv), szBufferTlv);
  }
  if((nbytes = cmdIpcSend(cmsh->oamSock, szBuffer)) > 0) {
    return CMD_IPC_OK;
  }
  return CMD_IPC_ERROR;
}
