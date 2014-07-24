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
#include <dlfcn.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "nnTypes.h"

#include "nnCmdDefines.h"
#include "nnCmdLink.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"
#include "nnCmdCli.h"
#include "nnCmdInstall.h"
#include "nosLib.h"

struct compCmMgr
{
  /** For startup-config loading */
  struct cmsh *cmsh;
  /** TODO: Add more here */
};

struct compCmMgr *gCmIpcBase;
extern void cmdFuncGlobalInstall(struct cmsh *cmsh);

Int32T WriteConfCB(struct cmsh *cmsh, Int32T uargc, Int8T **uargv, Int32T cargc, Int8T **cargv);

void
polMainFree(void)
{
  NNLOG(LOG_DEBUG, "enter %s\n", __FUNCTION__);
}


void *
pifCmdInit(void *data)
{
  NNLOG(LOG_DEBUG, "enter %s\n", __FUNCTION__);
  gCmIpcBase = (struct compCmMgr *)malloc(sizeof(*gCmIpcBase));
  if(gCmIpcBase != NULL)
  {
        memset(gCmIpcBase, 0, sizeof(*gCmIpcBase));
        gCmIpcBase->cmsh = cmdCmshInit(IPC_DLUSAMPLE);
        cmdFuncGlobalInstall(gCmIpcBase->cmsh);
        cmdWriteConfigInstall(gCmIpcBase->cmsh, *WriteConfCB); /** For config write  */
    }

  NNLOG(LOG_DEBUG, "exit %s\n", __FUNCTION__);

  return (void *)gCmIpcBase;
}


Int32T
WriteConfCB(struct cmsh *cmsh, Int32T uargc, Int8T **uargv, Int32T cargc, Int8T **cargv)
{
  /** TODO: Write Config Here */
	/**	FOR TEST								*/
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

Int32T
cmCmIpcFuncCb(struct compCmMgr *pol, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
  Int32T funcID, funcIndex, sSize,nArgc = 0, cArgc = 0;
  struct cmsh *vCmsh = NULL;
  Int8T cArgv[CMSH_ARGC_MAX][CMSH_ARGV_MAX_LEN];
  Int8T nArgv[CMSH_ARGC_MAX][CMSH_ARGV_MAX_LEN];
  
  Int8T *cArgvPtr[CMSH_ARGC_MAX];
  Int8T *nArgvPtr[CMSH_ARGC_MAX];

  if((vCmsh = cmdCmshBaseInit(cmIpcHdr->dstID, cmIpcHdr->srcID)) == NULL) {
    return CMD_IPC_ERROR;
  }
  vCmsh->outFunc = cmdOutPuts;
  vCmsh->outFd = vCmsh;
  vCmsh->cmfd = fd;
  vCmsh->requestKey = cmIpcHdr->requestKey;
  cmdIpcUnpackTlv(data, CMD_IPC_TLV_CALLBACK_INDEX, &sSize, &funcIndex);
  cmdIpcUnpackTlv(data, CMD_IPC_TLV_CALLBACK_FUNC, &sSize, &funcID);
  memset(cArgv, 0, CMSH_ARGC_MAX*CMSH_ARGV_MAX_LEN);
  memset(nArgv, 0, CMSH_ARGC_MAX*CMSH_ARGV_MAX_LEN);
  
  while(cmdIpcUnpackTlv(data, CMD_IPC_TLV_MODE_ARGV, &sSize, nArgv[nArgc]) == CMD_IPC_OK)
  {
      nArgvPtr[nArgc] = nArgv[nArgc];
      nArgc++;
  }

  while(cmdIpcUnpackTlv(data, CMD_IPC_TLV_CALLBACK_ARGSTR, &sSize, cArgv[cArgc]) == CMD_IPC_OK)
  {
      cArgvPtr[cArgc] = cArgv[cArgc];
      cArgc++;
  }

  vCmsh->cmshFuncIndex = funcIndex;
  if(cmIpcHdr->dstID == IPC_POL_MGR)
  {
    struct cmshCallbackMap *sNode;
    struct cmdListNode *nn;
    CMD_MANAGER_LIST_LOOP(pol->cmsh->callbackList, sNode, nn)
    {
      if(sNode->funcID == funcID)
      {
        if((sNode->func)(vCmsh, nArgc, nArgvPtr, cArgc, cArgvPtr) == CMD_IPC_OK)
        {
          if(vCmsh->outMod != CMD_OUTMOD_EXCLUDE) /** Send end of string  */
          {
            vCmsh->outMod = CMD_OUTMOD_EXCLUDE;
            cmdPrint(vCmsh,"\n");
          }
        }
      }
    }
  }
  cmdCmshBaseFree(vCmsh);
  return CMD_IPC_OK;
}

void
pifCmIpcProcess(Int32T sockId, void *message, Uint32T size)
{
  Int8T *szBuffer = (Int8T *)message;
  Int8T  szTmpBuffer[CMD_IPC_MAXLEN];
  Uint32T nMsgSize, nTotalBytes = 0;
  cmdIpcMsg_T cmIpcHdr;
  while(nTotalBytes < size)
  {
    cmdIpcUnpackHdr(szBuffer, &cmIpcHdr);
    nMsgSize = cmIpcHdr.length + CMD_IPC_HDR_LEN;
    bzero(&szTmpBuffer, CMD_IPC_MAXLEN);
    memcpy(&szTmpBuffer, szBuffer, nMsgSize);
    switch(cmIpcHdr.code)
    {
      case CMD_IPC_CODE_FUNC_REQ:
      cmCmIpcFuncCb(gCmIpcBase, sockId, szBuffer, &cmIpcHdr);
      break;
      default:
        fprintf(stderr,"[Unknown code]\n");
      break;
    }
    nTotalBytes += nMsgSize;
    szBuffer += nMsgSize;
  }
}

void
pifCmshIpcProcess(Int32T sockId, void *message, Uint32T size)
{
	Int8T *szBuffer = (Int8T *)message;
  Int8T  szTmpBuffer[CMD_IPC_MAXLEN];
  Uint32T nMsgSize, nTotalBytes = 0;
  cmdIpcMsg_T cmIpcHdr;
  while(nTotalBytes < size)
  {
    cmdIpcUnpackHdr(szBuffer, &cmIpcHdr);
    nMsgSize = cmIpcHdr.length + CMD_IPC_HDR_LEN;
    bzero(&szTmpBuffer, CMD_IPC_MAXLEN);
    memcpy(&szTmpBuffer, szBuffer, nMsgSize);
    switch(cmIpcHdr.code)
    {
      case CMD_IPC_CODE_FUNC_REQ:
      cmCmIpcFuncCb(gCmIpcBase, sockId, szBuffer, &cmIpcHdr);
      break;
      default:
        fprintf(stderr,"[Unknown code]\n");
      break;
    }
    nTotalBytes += nMsgSize;
    szBuffer += nMsgSize;
  }
}
