/**
 * @file      : nnCmdInstall.c
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
#include <stdlib.h>
#include <unistd.h>
#include "nnTypes.h"
#include "nnCmdDefines.h"
#include "nnCmdCommon.h"
#include "nnCmdLink.h"
#include "nnCmdNode.h"
#include "nnCmdCmsh.h"
#include "nnCmdMsg.h"
#include "nnCmdInstall.h"
#include "nnGlobalCmd.h"

Int32T
cmdCommandInstall(struct cmsh *cmsh, Int32T node, Int32T *sFlags, Int32T numFlags, struct cmdElement *cel)
{
  struct cmdTree *ctree= cmsh->ctree;
  Int32T i;
	for(i = 0; i < MAX_BYTES_FLAGS; i++)
  {
		cel->flags[i] = 0x00;
	}
  for(i = 0; i < numFlags; i++)
  {
    cmdCompBitAdd(cel->flags, sFlags[i]);
  }

  cel->nextNode = 0;
  //  cel->cmdKey = cmdGetNextKey();
  cel->helpNum = cmdGetTokenNumHelp(cel->str);
  if(cmdBuildRecursive(&ctree->modeTree[node], cel, cel->str, 0) == CMD_IPC_OK)
  {
    struct cmshCallbackMap *temp = (struct cmshCallbackMap *)malloc(sizeof(struct cmshCallbackMap));
    temp->funcID = cel->funcID;
    temp->func = NULL;
    cmdListAddNode(cmsh->callbackList, temp);
  }
  return (CMD_IPC_OK);
}

Int32T
cmdNodeInstall(struct cmsh *cmsh, Int32T pnode, Int32T cnode, Int32T priv, Int8T *str, struct cmdElement *mCel, struct cmdElement *eCel)
{
  struct cmdTree *ctree = cmsh->ctree;
  Int32T i;
  struct cmdTreeNode *pNode = &ctree->modeTree[pnode];
  struct cmdTreeNode *cNode = &ctree->modeTree[cnode];
  if(pNode->cNodeList == NULL)
  {
    pNode->cNodeList = cmdListNew();
  }
  if(cNode->cNodeList == NULL)
  {
    cNode->cNodeList = cmdListNew();
  }
  ctree->prompt[cnode] = str;
  cNode->tokenStr = " ";
  cNode->type = CMD_TOKEN_KEYWORD;
  ctree->nodePrivilege[cnode] = priv;
  for(i = 0; i < CMD_MAXNUM_MODEDEPTH; i++)
  {
    if(ctree->cmsh->modeTrace[i].mode == pnode)
    {
      ctree->cmsh->modeTrace[++i].mode = cnode;
      break;
    }
  }
  ctree->configWrite[cnode] = NULL;

  cmdCompBitAdd(mCel->flags, IPC_CMSH_MGR);
  mCel->nextNode = cnode;
  //  cel->cmdKey = cmdGetNextKey();
  mCel->helpNum = cmdGetTokenNumHelp(mCel->str);
  if(cmdBuildRecursive(&ctree->modeTree[pnode], mCel, mCel->str, 0) == CMD_IPC_OK)
  {
    struct cmshCallbackMap *temp = (struct cmshCallbackMap *)malloc(sizeof(struct cmshCallbackMap));
    temp->funcID = mCel->funcID;
    temp->func = NULL;
    cmdListAddNode(cmsh->callbackList, temp);
  }
  cmdCompBitAdd(eCel->flags, IPC_CMSH_MGR);
  eCel->nextNode = pnode;
  //  eCel->cmdKey = cmdGetNextKey();
  eCel->helpNum = cmdGetTokenNumHelp(eCel->str);
  if(cmdBuildRecursive(&ctree->modeTree[cnode], eCel, eCel->str, 0) == CMD_IPC_OK)
  {
    struct cmshCallbackMap *temp = (struct cmshCallbackMap *)malloc(sizeof(struct cmshCallbackMap));
    temp->funcID = eCel->funcID;
    temp->func = NULL;
    cmdListAddNode(cmsh->callbackList, temp);
  }  
  return (CMD_IPC_OK);
}

Int32T
cmdUpdateMode(struct cmsh* cmsh, Int32T argc, Int8T **argv)
{
    Int8T msgBuf[CMD_IPC_MAXLEN];
    Int32T i;
    cmdIpcMsg_T cmIpcHdr;    
    cmIpcHdr.code = CMD_IPC_CODE_UPDATE_NODE;   /** ENABLE authentication request code  */
    cmIpcHdr.requestKey = CMD_IPC_KEY_NULL;
    cmIpcHdr.clientType = CMD_IPC_TYPE_CLI;
    cmIpcHdr.mode = cmsh->modeTrace[cmsh->currentDepth].mode;
    cmIpcHdr.userKey = cmsh->userKey;
    cmIpcHdr.srcID = IPC_CMSH_MGR;
    cmIpcHdr.dstID = IPC_CM_MGR;
    cmdIpcAddHdr(msgBuf, &cmIpcHdr);
    cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE, 4, &cmsh->modeTrace[cmsh->currentDepth].mode);
    cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGC, 4, &cmsh->modeTrace[cmsh->currentDepth-1].modeArgc);
    for(i = 0; i < cmsh->modeTrace[cmsh->currentDepth-1].modeArgc; i++)
    {
      cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGV, strlen(cmsh->modeTrace[cmsh->currentDepth-1].modeArgv[i]), cmsh->modeTrace[cmsh->currentDepth-1].modeArgv[i]);
    }
    cmdIpcSend(cmsh->cmfd, msgBuf);
    return (CMD_IPC_OK);
}


Int32T 
cmdChangeNodeReq(struct cmsh* cmsh, Int32T nodeID, Int32T funcID, Int32T compID, Int32T argc, char **argv)
{
  Int8T msgBuf[CMD_IPC_MAXLEN], outString[CMD_IPC_MAXLEN];
  Int32T i, j, nbytes, sSize, cIndex;
  cmdIpcMsg_T cmIpcHdr;
  cmIpcHdr.code = CMD_IPC_CODE_CHANGE_NODE_REQ;
  cmIpcHdr.requestKey = cmsh->cmfd;
  cmIpcHdr.clientType = CMD_IPC_TYPE_CLI;
  cmIpcHdr.mode = cmsh->modeTrace[cmsh->currentDepth].mode;
  cmIpcHdr.userKey = cmsh->userKey;
  cmIpcHdr.srcID = IPC_CMSH_MGR;
  cmIpcHdr.dstID = compID;
  cmdIpcAddHdr(msgBuf, &cmIpcHdr);
  cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_FUNC, 4, &funcID);
  cIndex = 1;
  for(i = 0; i < cmsh->currentDepth; i++)
  {
    if(cmsh->modeTrace[i].mode >= CMD_NODE_CONFIG)
    {
      switch(cIndex)
      {
      case 1:
        for(j = 0; j < cmsh->modeTrace[i].modeArgc; j++)
        {
        cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGV1, strlen(cmsh->modeTrace[i].modeArgv[j]), cmsh->modeTrace[i].modeArgv[j]);
        }
      break;
      case 2:
        for(j = 0; j < cmsh->modeTrace[i].modeArgc; j++)
        {
        cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGV2, strlen(cmsh->modeTrace[i].modeArgv[j]), cmsh->modeTrace[i].modeArgv[j]);
        }
      break;
      case 3:
        for(j = 0; j < cmsh->modeTrace[i].modeArgc; j++)
        {
        cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGV3, strlen(cmsh->modeTrace[i].modeArgv[j]), cmsh->modeTrace[i].modeArgv[j]);
        }
      break;
      case 4:
        for(j = 0; j < cmsh->modeTrace[i].modeArgc; j++)
        {
        cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGV4, strlen(cmsh->modeTrace[i].modeArgv[j]), cmsh->modeTrace[i].modeArgv[j]);
        }
      break;
      case 5:
        for(j = 0; j < cmsh->modeTrace[i].modeArgc; j++)
        {
        cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGV5, strlen(cmsh->modeTrace[i].modeArgv[j]), cmsh->modeTrace[i].modeArgv[j]);
        }
      break;
      }
      cIndex++;
    }
  }  
  for(i = 0; i < argc; i++)
  {
    cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_ARGSTR, strlen(argv[i]), argv[i]);
  }
  cmdIpcSend(cmsh->cmfd, msgBuf);
  memset(&msgBuf, 0, CMD_IPC_MAXLEN);
  if((nbytes = cmdIpcRecv(cmsh->cmfd, msgBuf)) <= 0)
  {
    cmdPrint(cmsh, "Component is not running...\n");
    return CMD_IPC_ERROR;
  }
  cmdIpcUnpackHdr(msgBuf, &cmIpcHdr);
  memset(&outString, 0, CMD_IPC_MAXLEN);
  cmdIpcUnpackTlv(msgBuf, CMD_IPC_OUTPUT_DATA, &sSize, outString);
  cmdPrint(cmsh, "%s", outString);
  if(cmIpcHdr.code != CMD_IPC_CODE_CHANGE_NODE_RES_OK)
  {
    return CMD_IPC_ERROR;
  }
  return (CMD_IPC_OK);
}

Int32T
cmdChangeNodeRes(struct cmsh* cmsh, Int32T compID, Int32T outData, Int8T *outString)
{
  Int8T msgBuf[CMD_IPC_MAXLEN];
  cmdIpcMsg_T cmIpcHdr;
  if(outData == CMD_IPC_OK)
  {
    cmIpcHdr.code = CMD_IPC_CODE_CHANGE_NODE_RES_OK;
  }
  else
  {
    cmIpcHdr.code = CMD_IPC_CODE_CHANGE_NODE_RES_NOT_OK;
  }
  cmIpcHdr.requestKey = cmsh->requestKey;
  cmIpcHdr.clientType = CMD_IPC_TYPE_CLI;
  cmIpcHdr.userKey = cmsh->userKey;
  cmIpcHdr.srcID = compID;
  cmIpcHdr.dstID = IPC_CMSH_MGR;
  cmdIpcAddHdr(msgBuf, &cmIpcHdr);
  cmdIpcAddTlv(msgBuf, CMD_IPC_OUTPUT_DATA, strlen(outString), outString);
  cmdIpcSend(cmsh->cmfd, msgBuf);
  return CMD_IPC_OK;
}

Int32T
cmdFuncInstall(struct cmsh *cmsh, Int32T funcID, Int32T (*funcName)(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv))
{
  struct cmshCallbackMap *node;
  struct cmdListNode *nn;
  CMD_MANAGER_LIST_LOOP(cmsh->callbackList, node, nn)
  {
    if(node->funcID == funcID)
    {
      node->func = funcName;
      return (CMD_IPC_OK);
    }
  }
  struct cmshCallbackMap *temp = (struct cmshCallbackMap *)malloc(sizeof(struct cmshCallbackMap));
  temp->funcID = funcID;
  temp->func = funcName;
  cmdListAddNode(cmsh->callbackList, temp);
  return (CMD_IPC_OK);
}
Int32T
cmdWriteConfigInstall(struct cmsh *cmsh, Int32T (*funcName)(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv))
{
  struct cmshCallbackMap *temp = (struct cmshCallbackMap *)malloc(sizeof(struct cmshCallbackMap));
  temp->funcID = CMD_FUNC_WRITE_FUNC_ID;
  temp->func = funcName;
  cmdListAddNode(cmsh->callbackList, temp);
  return (CMD_IPC_OK);
}

Int32T
cmdWriteConfigUpdate(struct cmsh *cmsh, Int32T (*funcName)(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv))
{
  struct cmshCallbackMap *node;
  struct cmdListNode *nn;
  CMD_MANAGER_LIST_LOOP(cmsh->callbackList, node, nn)
  {
    if(node->funcID == CMD_FUNC_WRITE_FUNC_ID)
    {
      node->func = funcName;
    }
  }
  return (CMD_IPC_OK);  
}

Int32T 
cmdNodeRunningInstall(struct cmdList *table, Int32T node, Int8T *str)
{
  struct cmshNodeCmTree *tmpNode = (struct cmshNodeCmTree *)malloc(sizeof *tmpNode);
  tmpNode->nodeID = node;
  tmpNode->nodeStr = strdup(str);
  cmdListAddNode(table, tmpNode);
  return (CMD_IPC_OK);
}
