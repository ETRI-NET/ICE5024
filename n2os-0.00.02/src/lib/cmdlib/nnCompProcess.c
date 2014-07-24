/**
 * @file      : nnCompProcess.c
 * @brief       :
 *
 * $Id: nnCompProcess.c 725 2014-01-17 09:11:34Z hyryu $
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
#include <stdlib.h>
#include <unistd.h>
#include "nnTypes.h"
#include "nnCmdCommon.h"
#include "nnCmdDefines.h"
#include "nnCmdCmsh.h"
#include "nnCompProcess.h"
#include "nnCmdInstall.h"
#include "nnCmdMsg.h"
#include "nnCmdLink.h"

extern void cmdFuncGlobalInstall(struct cmsh *cmsh);

compCmshGlobal_T *gCmshData = NULL;

void *
compCmdInit(Int32T compId, 
Int32T (*funcName)(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv))
{
	compCmshGlobal_T *gCmsh = (compCmshGlobal_T *)malloc(sizeof(compCmshGlobal_T));
	memset(gCmsh, 0, sizeof(compCmshGlobal_T));
	gCmsh->compId = compId;
	gCmsh->cmsh = cmdCmshInit(compId);
	cmdFuncGlobalInstall(gCmsh->cmsh);
	cmdWriteConfigInstall(gCmsh->cmsh, funcName);
	gCmshData = gCmsh;
	return gCmsh;
}
Int32T
compCmdIpcMsgProcess(void *gCmshGlobal, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
  Int32T funcID, funcIndex, sSize, cArgc = 0;
  Int32T nArgc1 = 0, nArgc2 = 0, nArgc3 = 0, nArgc4 = 0, nArgc5 = 0;
  struct cmsh *vCmsh = NULL;
	compCmshGlobal_T *gCmsh = (compCmshGlobal_T *)(gCmshGlobal);

  Int8T cArgv[CMSH_ARGC_MAX][CMSH_ARGV_MAX_LEN];
  Int8T nArgv1[CMSH_ARGC_MAX][CMSH_ARGV_MAX_LEN];
  Int8T nArgv2[CMSH_ARGC_MAX][CMSH_ARGV_MAX_LEN];
  Int8T nArgv3[CMSH_ARGC_MAX][CMSH_ARGV_MAX_LEN];
  Int8T nArgv4[CMSH_ARGC_MAX][CMSH_ARGV_MAX_LEN];
  Int8T nArgv5[CMSH_ARGC_MAX][CMSH_ARGV_MAX_LEN];

  Int8T *cArgvPtr[CMSH_ARGC_MAX];
  Int8T *nArgvPtr1[CMSH_ARGC_MAX];
  Int8T *nArgvPtr2[CMSH_ARGC_MAX];
  Int8T *nArgvPtr3[CMSH_ARGC_MAX];
  Int8T *nArgvPtr4[CMSH_ARGC_MAX];
  Int8T *nArgvPtr5[CMSH_ARGC_MAX];

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
  memset(nArgv1, 0, CMSH_ARGC_MAX*CMSH_ARGV_MAX_LEN);
  memset(nArgv2, 0, CMSH_ARGC_MAX*CMSH_ARGV_MAX_LEN);
  memset(nArgv3, 0, CMSH_ARGC_MAX*CMSH_ARGV_MAX_LEN);
  memset(nArgv4, 0, CMSH_ARGC_MAX*CMSH_ARGV_MAX_LEN);
  memset(nArgv5, 0, CMSH_ARGC_MAX*CMSH_ARGV_MAX_LEN);

  while(cmdIpcUnpackTlv(data, CMD_IPC_TLV_MODE_ARGV1, &sSize, nArgv1[nArgc1]) == CMD_IPC_OK)
  {
      nArgvPtr1[nArgc1] = nArgv1[nArgc1];
      nArgc1++;
  }

  while(cmdIpcUnpackTlv(data, CMD_IPC_TLV_MODE_ARGV2, &sSize, nArgv2[nArgc2]) == CMD_IPC_OK)
  {
      nArgvPtr2[nArgc2] = nArgv2[nArgc2];
      nArgc2++;
  }
  while(cmdIpcUnpackTlv(data, CMD_IPC_TLV_MODE_ARGV3, &sSize, nArgv3[nArgc3]) == CMD_IPC_OK)
  {
      nArgvPtr3[nArgc3] = nArgv3[nArgc3];
      nArgc3++;
  }
  while(cmdIpcUnpackTlv(data, CMD_IPC_TLV_MODE_ARGV4, &sSize, nArgv4[nArgc4]) == CMD_IPC_OK)
  {
      nArgvPtr4[nArgc4] = nArgv4[nArgc4];
      nArgc4++;
  }
  while(cmdIpcUnpackTlv(data, CMD_IPC_TLV_MODE_ARGV5, &sSize, nArgv5[nArgc5]) == CMD_IPC_OK)
  {
      nArgvPtr5[nArgc5] = nArgv5[nArgc5];
      nArgc5++;
  }

  while(cmdIpcUnpackTlv(data, CMD_IPC_TLV_CALLBACK_ARGSTR, &sSize, cArgv[cArgc]) == CMD_IPC_OK)
  {
      cArgvPtr[cArgc] = cArgv[cArgc];
      cArgc++;
  }
  vCmsh->cmshFuncIndex = funcIndex;
  if(cmIpcHdr->dstID == gCmsh->compId)
  {
    struct cmshCallbackMap *sNode;
    struct cmdListNode *nn;
    CMD_MANAGER_LIST_LOOP(gCmsh->cmsh->callbackList, sNode, nn)
    {
      if(sNode->funcID == funcID)
      {
        if((sNode->func)(vCmsh, nArgc1, nArgvPtr1, nArgc2, nArgvPtr2, nArgc3, nArgvPtr3, nArgc4, nArgvPtr4, nArgc5, nArgvPtr5, cArgc, cArgvPtr) == CMD_IPC_OK)
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

Int32T
compCmdIpcNodeMsgProcess(void *gCmshGlobal, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
  Int32T nnReturn, funcID, sSize, cArgc = 0;
  Int32T nArgc1 = 0, nArgc2 = 0, nArgc3 = 0, nArgc4 = 0, nArgc5 = 0;
  struct cmsh *vCmsh = NULL;
  compCmshGlobal_T *gCmsh = (compCmshGlobal_T *)(gCmshGlobal);

  Int8T nArgv1[CMSH_ARGC_MAX][CMSH_ARGV_MAX_LEN];
  Int8T nArgv2[CMSH_ARGC_MAX][CMSH_ARGV_MAX_LEN];
  Int8T nArgv3[CMSH_ARGC_MAX][CMSH_ARGV_MAX_LEN];
  Int8T nArgv4[CMSH_ARGC_MAX][CMSH_ARGV_MAX_LEN];
  Int8T nArgv5[CMSH_ARGC_MAX][CMSH_ARGV_MAX_LEN];

  Int8T *nArgvPtr1[CMSH_ARGC_MAX];
  Int8T *nArgvPtr2[CMSH_ARGC_MAX];
  Int8T *nArgvPtr3[CMSH_ARGC_MAX];
  Int8T *nArgvPtr4[CMSH_ARGC_MAX];
  Int8T *nArgvPtr5[CMSH_ARGC_MAX];


  Int8T cArgv[CMSH_ARGC_MAX][CMSH_ARGV_MAX_LEN];
  Int8T *cArgvPtr[CMSH_ARGC_MAX];


  if((vCmsh = cmdCmshBaseInit(cmIpcHdr->dstID, cmIpcHdr->srcID)) == NULL) {
    return CMD_IPC_ERROR;
  }
  vCmsh->outFd = vCmsh;
  vCmsh->cmfd = fd;
  vCmsh->requestKey = cmIpcHdr->requestKey;
  cmdIpcUnpackTlv(data, CMD_IPC_TLV_CALLBACK_FUNC, &sSize, &funcID);
  memset(cArgv, 0, CMSH_ARGC_MAX*CMSH_ARGV_MAX_LEN);

  memset(nArgv1, 0, CMSH_ARGC_MAX*CMSH_ARGV_MAX_LEN);
  memset(nArgv2, 0, CMSH_ARGC_MAX*CMSH_ARGV_MAX_LEN);
  memset(nArgv3, 0, CMSH_ARGC_MAX*CMSH_ARGV_MAX_LEN);
  memset(nArgv4, 0, CMSH_ARGC_MAX*CMSH_ARGV_MAX_LEN);
  memset(nArgv5, 0, CMSH_ARGC_MAX*CMSH_ARGV_MAX_LEN);

  while(cmdIpcUnpackTlv(data, CMD_IPC_TLV_MODE_ARGV1, &sSize, nArgv1[nArgc1]) == CMD_IPC_OK)
  {
      nArgvPtr1[nArgc1] = nArgv1[nArgc1];
      nArgc1++;
  }

  while(cmdIpcUnpackTlv(data, CMD_IPC_TLV_MODE_ARGV2, &sSize, nArgv2[nArgc2]) == CMD_IPC_OK)
  {
      nArgvPtr2[nArgc2] = nArgv2[nArgc2];
      nArgc2++;
  }
  while(cmdIpcUnpackTlv(data, CMD_IPC_TLV_MODE_ARGV3, &sSize, nArgv3[nArgc3]) == CMD_IPC_OK)
  {
      nArgvPtr3[nArgc3] = nArgv3[nArgc3];
      nArgc3++;
  }
  while(cmdIpcUnpackTlv(data, CMD_IPC_TLV_MODE_ARGV4, &sSize, nArgv4[nArgc4]) == CMD_IPC_OK)
  {
      nArgvPtr4[nArgc4] = nArgv4[nArgc4];
      nArgc4++;
  }
  while(cmdIpcUnpackTlv(data, CMD_IPC_TLV_MODE_ARGV5, &sSize, nArgv5[nArgc5]) == CMD_IPC_OK)
  {
      nArgvPtr5[nArgc5] = nArgv5[nArgc5];
      nArgc5++;
  }

  while(cmdIpcUnpackTlv(data, CMD_IPC_TLV_CALLBACK_ARGSTR, &sSize, cArgv[cArgc]) == CMD_IPC_OK)
  {
      cArgvPtr[cArgc] = cArgv[cArgc];
      cArgc++;
  }
  if(cmIpcHdr->dstID == gCmsh->compId)
  {
    struct cmshCallbackMap *sNode;
    struct cmdListNode *nn;
    vCmsh->outputStr = strdup("");
    CMD_MANAGER_LIST_LOOP(gCmsh->cmsh->callbackList, sNode, nn)
    {
      if(sNode->funcID == funcID)
      {
        nnReturn = (sNode->func)(vCmsh, nArgc1, nArgvPtr1, nArgc2, nArgvPtr2, nArgc3, nArgvPtr3, nArgc4, nArgvPtr4, nArgc5, nArgvPtr5, cArgc, cArgvPtr);
        cmdChangeNodeRes(vCmsh, gCmsh->compId, nnReturn, vCmsh->outputStr);
        break;
      }
    }
  }
  cmdCmshBaseFree(vCmsh);
  return CMD_IPC_OK;
}

void                                                                                                           
compCmdUpdate(compCmshGlobal_T *cmshData,                                                                      
Int32T (*funcName)(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv))               
{                                                                                                              
  gCmshData = cmshData;
  cmdFuncGlobalInstall(cmshData->cmsh);                                                                        
  cmdWriteConfigUpdate(cmshData->cmsh, funcName);                                                             
}

void                                                                                                         
compCmdIpcRegProcess(void *gCmshGlobal, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)                             
{                                                                                                              
  compCmshGlobal_T *gCmsh = (compCmshGlobal_T *)(gCmshGlobal);                                                   
  struct cmsh *vCmsh = (struct cmsh *)(gCmsh->cmsh);                                                             
  vCmsh->oamSock = fd;                                                                                           
}    

void 
compCmdIpcProcess(void *gCmshGlobal, Int32T sockId, void *message, Uint32T size)
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
      case CMD_IPC_CODE_CLIENT_REG_RES_OK:                                                                     
          compCmdIpcRegProcess(gCmshGlobal, sockId, szBuffer, &cmIpcHdr);                                         
        break;                                                                                                 
      case CMD_IPC_CODE_CLIENT_DEREG_RES_OK:                                                                 
        break;          
      case CMD_IPC_CODE_FUNC_REQ:
        compCmdIpcMsgProcess(gCmshGlobal, sockId, szBuffer, &cmIpcHdr);
      break;
      case CMD_IPC_CODE_CHANGE_NODE_REQ:
        compCmdIpcNodeMsgProcess(gCmshGlobal, sockId, szBuffer, &cmIpcHdr);
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
compCmdFree(void *gCmsh)
{
  if(gCmsh != NULL)
  {
    compCmshGlobal_T *gCmshPtr = (compCmshGlobal_T *)(gCmsh);
    if(gCmshPtr->cmsh != NULL)
    {
      cmdCmshFree(gCmshPtr->cmsh);
    }
    free(gCmshPtr);
  }
}

