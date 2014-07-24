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
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "nnTypes.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"
#include "nnCmdLink.h"
#include "nnCmdDefines.h"
#include "nnCmdCli.h"

extern void cmdFuncGlobalInstall(struct cmsh *cmsh);
extern Int32T cmdSockConn(Int32T gIndex);
extern Int32T cmdSockClose(Int32T sock);

extern fd_set gMasterFds;  /** For Hook  */
extern Int32T gFdMaxNumber;

typedef Int32T (*outFunc_T)(void *, Int8T *, ...);
struct cmsh *gCmshPtr;
Int8T *telUserName = NULL;

Int8T *
cmshGetUserName(void)
{
  struct passwd *pw = getpwuid(geteuid());
  Int8T *gzUserName;
  if(pw != NULL)
  {
    gzUserName = strdup(pw->pw_name);
  }
  else
  {
    gzUserName = strdup("nobody");
  }
  return gzUserName;
}
Int8T*
cmshGetUser(Int8T *prompt)
{
  Int8T *nbuf, *p;
  Int32T ch, indexs = 0;
  nbuf = malloc(1024);
  if (nbuf == NULL)
    return NULL;
  printf("%s", prompt);
  p = (Int8T *)(nbuf);
  while((ch = getchar()) != '\n')
  {
    if(indexs < 1024)
    {
      *p++ = ch;
    }
  }
  *p = '\0';
  return nbuf;
}

Int8T *
cmshGetPass(Int8T *prompt)
{
  return getpass(prompt);
}

Int32T
cmdMsgLogin(Int32T sockfd, Int32T counter)
{
  Int8T msgBuf[CMD_IPC_MAXLEN];
  size_t nbytes;
  Int8T *szUserName, *szPassword;
  while(counter--)
  {
    bzero(&msgBuf, CMD_IPC_MAXLEN);
    cmdIpcMsg_T cmIpcHdr;
    cmIpcHdr.code = CMD_IPC_CODE_CLIENT_REG_LOGIN;
    cmIpcHdr.requestKey = CMD_IPC_KEY_NULL;
    cmIpcHdr.clientType = CMD_IPC_TYPE_CLI;
    cmIpcHdr.mode = CMD_NODE_NULL;
    cmIpcHdr.userKey = (1<<0);
    cmIpcHdr.srcID = IPC_CMSH_MGR;
    cmIpcHdr.dstID = IPC_CM_MGR;
    cmdIpcAddHdr(msgBuf, &cmIpcHdr);    /**  Add Header  */
    szUserName = cmshGetUser("Login :");
    szPassword = cmshGetPass("Password :");
    if((szUserName != NULL) && (szPassword != NULL))
    {
      telUserName = strdup(szUserName);  /**  Record Username  */
      cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_USERID, strlen(szUserName), szUserName);
      cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_PASSWD,  strlen(szPassword), szPassword);
      cmdIpcSend(sockfd, msgBuf);
      bzero(&msgBuf, CMD_IPC_MAXLEN);
      if((nbytes = cmdIpcRecv(sockfd, msgBuf)) == 0)
      {
        continue;
      }
      cmdIpcUnpackHdr(msgBuf, &cmIpcHdr);

      if(cmIpcHdr.code == CMD_IPC_CODE_CLIENT_RES_LOG_SUCCESS)
      {
        return CMD_IPC_OK;
      }
    }
  }
  close(sockfd);
  return CMD_IPC_ERROR;
}

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN        64
#endif

Int8T *
nosGetHostName(void)
{
  Int8T *hostName = (Int8T *)malloc(MAXHOSTNAMELEN * sizeof(Int8T));
  memset(hostName, 0, MAXHOSTNAMELEN);
  if(gethostname(hostName, MAXHOSTNAMELEN))
  {
    strcpy(hostName, "Nos-Router");
  }
  return hostName;
}

void
cmshMainFree(void)
{
  /** TODO: Code main free in here  */
  if(gCmshPtr->cmfd)
  {
    close(gCmshPtr->cmfd);
  }
  cmdCmshFree(gCmshPtr);
}
void
cmshMainExit(void)
{
  if(cmdCmshDeRegister(gCmshPtr) != CMD_IPC_OK)
  {
    fprintf(stderr, "Error DeRegister...\n");
  }
}

Int32T
cmshMainInit(Int32T sock)
{
  gCmshPtr = cmdCmshInit(IPC_CMSH_MGR);
  gCmshPtr->outFunc = (outFunc_T)fprintf;
  gCmshPtr->outFd = stdout;
  if(telUserName != NULL)
    gCmshPtr->userID = strdup(telUserName);//cmshGetUserName();
  else
    gCmshPtr->userID = cmshGetUserName();
  gCmshPtr->hostName = nosGetHostName();  /** Get Host Name of System */
  cmdFuncGlobalInstall(gCmshPtr);
  gCmshPtr->currentDepth = CMD_NODE_VIEW;            /**  View Mode       */
  gCmshPtr->prompt = gCmshPtr->ctree->prompt[gCmshPtr->modeTrace[gCmshPtr->currentDepth].mode];
  gCmshPtr->cmfd = sock;
  if(cmdCmshRegister(gCmshPtr) != CMD_IPC_OK)
  {
    cmshMainFree();
    return CMD_IPC_ERROR;
  }
  
	return CMD_IPC_OK;
}

extern Int32T gCmshBlockHook;  /** Status Comand for hook  */

Int8T *
cmshPromptStr(void)
{
  Int32T currNode = gCmshPtr->currentDepth-1;
  Int32T i;
  Int8T pBuffer[1024] = "";

  if(currNode >= CMD_NODE_CONFIG)
  {
    for(i = 0; i < gCmshPtr->modeTrace[currNode].modeArgc; i++)
    {
        if(i == 0)
        {
          sprintf(pBuffer, "%s", gCmshPtr->modeTrace[currNode].modeArgv[i]);
        }
        else
        {
          sprintf(pBuffer, "%s-%s", pBuffer, gCmshPtr->modeTrace[currNode].modeArgv[i]);
        }
    }
    gCmshPtr->prompt = strdup(pBuffer);
  }

	Int8T promStr[MAXHOSTNAMELEN + 30];
	if(gCmshBlockHook) {
		strcpy(promStr, "");
	} else {
		char sign = '>';
		if(gCmshPtr->modeTrace[gCmshPtr->currentDepth].mode > CMD_NODE_VIEW)
		{
			sign = '#';
      snprintf (promStr, MAXHOSTNAMELEN+30, "%s(%s)%c",gCmshPtr->hostName, gCmshPtr->prompt,sign);
		}else{
			snprintf (promStr, MAXHOSTNAMELEN+30, "%s%c", gCmshPtr->hostName,sign);
		}
	}
  gCmshPtr->proStr = strdup(promStr);
  return gCmshPtr->proStr;
}

Int32T
cmshParseCmd(struct cmsh *cmsh)
{
  /** TODO : Parser Here */
  struct cmdElement *currCel;
  struct cmdListNode *celNN;

  Int32T isWaiting = 0; /** Waiting */

  Int32T cIndex,  i, j, m, sockfd; /**   For Loop and socket connection to another components */
  Int8T msgBuf[CMD_IPC_MAXLEN];
  cmdIpcMsg_T cmIpcHdr;
  if(cmdParseCommand(cmsh) == CMD_PARSE_SUCCESS)
  {
    CMD_MANAGER_LIST_LOOP(cmsh->cnode->cCelTable, currCel, celNN)
    {
      cmsh->cel = currCel;

      if(cmsh->inputMode)
      {
        if((cmsh->cel->nextNode != 0) && (cmsh->cel->nextNode != cmsh->currentDepth))
        {
          cmdPrint(gCmshPtr, " %% No such command\n");
          return 0;
        }
      }
      /** END   */
      if(cmdCompBitCheck(cmsh->cel->flags, IPC_CMSH_MGR))
      {
        /** Callback in CMSH side */
        struct cmshCallbackMap *sNode;
        struct cmdListNode *nn;
        CMD_MANAGER_LIST_LOOP(cmsh->callbackList, sNode, nn)
        {
          if((sNode->funcID == cmsh->cel->funcID) &&(sNode->func != NULL))
          {
            if(((sNode->func)(cmsh, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, cmsh->argc, cmsh->argv)) != CMD_IPC_OK)
            {
              isWaiting++;
            }
          }
        }
        return isWaiting;	/**	Call function from cmsh not need blocking */
      }
      else
      {
        if(cmdCompBitCheck(cmsh->cel->flags, IPC_SHOW_MGR))
        {
          /** Show Mode */
          cmIpcHdr.code = CMD_IPC_CODE_FUNC_REQ;
          cmIpcHdr.clientType = CMD_IPC_TYPE_CLI;
          cmIpcHdr.mode = cmsh->modeTrace[cmsh->currentDepth].mode;
          cmIpcHdr.userKey = cmsh->userKey;
          cmIpcHdr.srcID = IPC_CMSH_MGR;
          for(m = IPC_CMSH_MGR; m < IPC_MAX_MGR; m++)
          {
            if(cmdCompBitCheck(cmsh->cel->flags, m))
            {
              if(m == IPC_CM_MGR)
              {
                sockfd = cmsh->cmfd;
              }
              else
              {
                if((sockfd = cmdSockConn(m)) > 0)
                {
                  FD_SET(sockfd, &gMasterFds);
                  gFdMaxNumber = MAX(gFdMaxNumber, sockfd);
                }
              }
              if(sockfd > 0)
              {
                cmIpcHdr.requestKey = sockfd;
                cmIpcHdr.dstID = m;
                cmdIpcAddHdr(msgBuf, &cmIpcHdr);
                /** 2-2. CM IPC TLV: Add callback function ID */
                cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_INDEX, sizeof (Int32T), &cmsh->cmshFuncIndex);  /** 0 = First Time  */
                cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_FUNC, sizeof (Int32T), &cmsh->cel->funcID);
                /** 2-3. Add Node Argv  */
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
                /** 2-4. CM IPC TLV: Add callback function argument */
                for (i = 0; i < (cmsh->argc); i++)
                {
                    cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_ARGSTR, strlen(cmsh->argv[i]), cmsh->argv[i]);
                }
                /** 2-5. CM IPC send */
                cmdIpcSend(sockfd, msgBuf);
                isWaiting++;
              }
            }
          }
        }
        else
        {
          /** Send To CM manager  */
          cmIpcHdr.code = CMD_IPC_CODE_FUNC_REQ;
          cmIpcHdr.requestKey = cmsh->cmfd;
          cmIpcHdr.clientType = CMD_IPC_TYPE_CLI;
          cmIpcHdr.mode = cmsh->modeTrace[cmsh->currentDepth].mode;
          cmIpcHdr.userKey = cmsh->userKey;
          cmIpcHdr.srcID = IPC_CMSH_MGR;
          for(m = 0; m < IPC_MAX_MGR; m++)
          {
            if(cmdCompBitCheck(cmsh->cel->flags, m))
            {
              cmIpcHdr.dstID = m;
              cmdIpcAddHdr(msgBuf, &cmIpcHdr);
              /** 2-2. CM IPC TLV: Add callback function ID */
              cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_INDEX, sizeof (Int32T), &cmsh->cmshFuncIndex);  /** 0 = First Time  */
              cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_FUNC, sizeof (Int32T), &cmsh->cel->funcID);
              /** 2-3. Add Node Argv  */

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
              /** 2-3. CM IPC TLV: Add callback function argument */
              for (i = 0; i < (cmsh->argc); i++)
              {
                  cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_ARGSTR, strlen(cmsh->argv[i]), cmsh->argv[i]);
              }
              /** 2-4. CM IPC send */
              cmdIpcSend(cmsh->cmfd, msgBuf);
              isWaiting++;
            }
          }
        }
      }
    }
  }
	return isWaiting;	/**	default not loop hook	*/
}
