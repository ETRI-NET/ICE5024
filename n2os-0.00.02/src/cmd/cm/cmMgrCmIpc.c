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
#include <ctype.h>
#include <dlfcn.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdarg.h>
#include "nnTypes.h"

#include "nnCmdDefines.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdLink.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"
#include "nnCmdCmsh.h"
#include "nosLib.h"
#include "cmMgrCmIpc.h"
#include "nnCmdInstall.h"
#include "nnCmdMd5.h"
#include "nnCmdSha2.h"
#include "nnCmdSha4.h"
extern void cmdFuncGlobalInstall(struct cmsh *cmsh);

typedef Int32T (*outFunc_R)(void *, Int8T *, ...);
enum
{
  CM_USER_VIEW_MODE = 0,
  CM_USER_EXEC_MODE,
  CM_USER_CONFIG_MODE,
  CM_USER_MAX_MODE,
};
#define MAX_USER_SIZE  1024

struct cmUserMode_T
{
 Int32T cmUserMode;
 Int8T  cmUserStrs[MAX_USER_SIZE];
} gUserMode[CM_USER_MAX_MODE] __attribute__ ((unused)) = 
{
 { CM_USER_VIEW_MODE,    "view"    },
 { CM_USER_EXEC_MODE,    "enable"  },
 { CM_USER_CONFIG_MODE,  "config"  },
};

void
cmdFuncWriteService(cmMgrT *cm)
{
  FILE *ptr = fopen(NOS_INETD_CONFIG_FILE_PATH, "w");
  if(cm->cmService.pTelnetPortId > 0)
  {
    fprintf(ptr, "%d stream tcp nowait root /usr/sbin/tcpd in.telnetd -L /bin/cmsh\n", cm->cmService.pTelnetPortId);
  }
  if(cm->cmService.pSshPortId > 0)
  {
    fprintf(ptr, "%d stream tcp nowait root /usr/sbin/tcpd sshd -i\n", cm->cmService.pSshPortId);
  }
  fclose(ptr);
}
Int32T
cmdIsExisted(Int8T *sPath)
{
  struct stat fin;
  if (access(sPath, X_OK) == 0 &&
      stat(sPath, &fin) == 0 &&
      S_ISREG(fin.st_mode) &&
      (getuid() != 0 ||
      (fin.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) != 0)) 
  {
    return (1);
  }
  return (0);
}
Int32T
cmdCmdServiceCheck(Int8T *name, Int8T **fullName)
{
  Int8T *pPath, *pch, *envPath;
  static Int8T gPathStr[4096];
  envPath = getenv("PATH");
  if((envPath = getenv("PATH")) != NULL)
  {
    pPath = strdup(envPath);
    pch = strtok (pPath,":");
    while (pch != NULL)
    {
      sprintf(gPathStr, "%s/%s", pch, name);
      if(cmdIsExisted(gPathStr) != 0)
      {
        *fullName = gPathStr;
        return 1;
      }
      pch = strtok (NULL, ":");
    }
    free(pPath);
  }
  return 0;
}

Int32T
cmdCmdServiceRestart(cmMgrT *cm)
{
  Int8T gzCmdStr[1024];
  Int8T *fName;
  sprintf(gzCmdStr, "kill -9 %d", cm->cmService.pInetd);
  if(cm->cmService.pInetd > 0)
  {
    if(cmdCmdServiceCheck("inetd", &fName))
    {
      cmdCallbackShellSrv(gzCmdStr);
      return CMD_IPC_OK;
    }
  }
  return CMD_IPC_ERROR;
}


Int32T
cmdCmdServiceStart(cmMgrT *cm)
{
  Int8T gzCmdStr[1024];
  Int8T *fName;
  cmdFuncWriteService(cm);
  if(cmdCmdServiceCheck("inetd", &fName)) 
  {
    sprintf(gzCmdStr, "%s %s", fName, NOS_INETD_CONFIG_FILE_PATH);
    cm->cmService.pInetd = cmdCallbackShellSrv(gzCmdStr);
    return CMD_IPC_OK;
  }
  return CMD_IPC_ERROR;
}

Int32T
cmdServiceStartup(cmMgrT *cm)
{
	Int8T *cmdStr;
	struct cmdListNode *nn;
	Int32T nCheck = 0;
	CMD_MANAGER_LIST_LOOP(cm->cmConfigTableMul, cmdStr, nn)
	{
		if(nCheck == 1 && strcmp(cmdStr, "!") == 0)
			break;
		if(nCheck == 1){
			struct cmServiceManagerStartup_T *tmpService = (struct cmServiceManagerStartup_T *)malloc(sizeof(*tmpService));
			tmpService->strService = cmdStr; //strdup
			if(strstr(cmdStr, "telnet service") != NULL){
				tmpService->flags = SERVICE_TELNET_FLAG;
			}
			if(strstr(cmdStr, "ssh service") != NULL){
				tmpService->flags = SERVICE_SSH_FLAG;
			}
			cmdListAddNode(cm->cmServiceStartup, tmpService);
		}
		if(strcmp(cmdStr, "service management") == 0){
			nCheck = 1;
		}
	}
	return CMD_IPC_OK;
}

Int32T
cmdCmdServiceInit(cmMgrT *cm)
{
  /** startup service*/
  cm->cmService.pInetd = 0;
  cmdCmdServiceStart(cm);
  return CMD_IPC_OK;
}

Int32T
cmdCmdServiceStop(cmMgrT *cm)
{
  Int8T gzCmdStr[1024];
  Int8T *fName;
  cmdCmdServiceRestart(cm);
  cm->cmService.pTelnetPortId = 0;
  cm->cmService.pSshPortId = 0;
  cmdCmdServiceStart(cm);
  sprintf(gzCmdStr, "kill -9 %d", cm->cmService.pInetd);
  if(cm->cmService.pInetd > 0)
  {
    if(cmdCmdServiceCheck("inetd", &fName))
    {
      cmdCallbackShellSrv(gzCmdStr);
      return CMD_IPC_OK;
    }
  }
  return CMD_IPC_ERROR;
}

#define _NOS_USER_FILE_ "/etc/nos_user.conf"

Int32T
cmdUserGetValue(Int8T* buff, Int8T* name, Int8T* mode, Int8T* encryptm, Int8T* password)
{
	Int32T i, j, k, q, len, cnt1 = 0, cnt2 = 0, cnt3 = 0, cnt4 = 0;

	len = strlen(buff);
	for(i = 0; i < len; i++)
	{
		if(buff[i] == ' ')
		{
			name[cnt1] = '\0';
			for(j = i+1; j < len; j++)
			{
				if(buff[j] == ' '){ 
					mode[cnt2] = '\0';
					for(k = j+1; k < len; k++){
						if(buff[k] == ' '){
							encryptm[cnt3] = '\0';
							for(q = k+1; q < len; q++){
								password[cnt4] = buff[q];
								cnt4 = cnt4 + 1;
							}
							password[cnt4] = '\0';
                    		return CMD_IPC_OK;
						}else{
							encryptm[cnt3] = buff[k];
							cnt3 = cnt3 + 1;
						}
					}
				}else{
					name[cnt1] = '\0';
					if((buff[j] != ' ') && (buff[j] != '\n') && (buff[j] != '\r'))
                    {
                        mode[cnt2] = buff[j];
                        cnt2 = cnt2 + 1;
                    }
					
				}
			}
		}
		else
		{	
			if((buff[i] != ' ') && (buff[i] != '\n') && (buff[i] != '\r'))
			{
				name[cnt1] = buff[i];
				cnt1 = cnt1 + 1;
			}
		}
	}
	return CMD_IPC_ERROR;
}

Int32T
cmdUserReadStore()
{
  Int8T *cfgPtr, cfgLine[1024], userName[255], userMode[255], passWord[255], encryptMethod[255];
  Int32T cfgSize, i;
  FILE *ptr;

  if((ptr = fopen(_NOS_USER_FILE_, "r")) != NULL)
  {
        while(fgets (cfgLine, 1024, ptr))
        {
            cfgSize = strlen(cfgLine);
            cfgLine[cfgSize-1] = '\0';
            cfgPtr = strdup(cfgLine);
			if(cmdUserGetValue(cfgPtr, userName, userMode, encryptMethod, passWord) == CMD_IPC_OK){
				struct cmUserManager_T *tmpUser = (struct cmUserManager_T *)malloc(sizeof(*tmpUser));
				for(i = CM_USER_VIEW_MODE; i < CM_USER_MAX_MODE; i++)
				 {
				   if(!strcmp(gUserMode[i].cmUserStrs, userMode))
				   {
					 tmpUser->cmUserMode = gUserMode[i].cmUserMode;
				   }
				 }
				sprintf(tmpUser->cmUserName, "%s", userName);
				sprintf(tmpUser->cmUserEncryptMethod, "%s", encryptMethod);
				if(strcmp(tmpUser->cmUserEncryptMethod, "none") == 0){
					sprintf(tmpUser->cmUserPassword, "%s", passWord);
				}else{
					sprintf(tmpUser->cmUserPassEncrypt, "%s", passWord);
				}
				 cmdListAddNode(gCmIpcBase->cmUserTable, tmpUser);
			}
        }
    fclose(ptr);
  }

  return CMD_IPC_OK;
}

void
cmdUserWriteStore()
{
 	struct cmUserManager_T *tmpUser;
  	struct cmdListNode  *nn;
	FILE *ptr;
  	Int32T i = 0;
	
	if((ptr = fopen(_NOS_USER_FILE_, "w")) == NULL)
  	{
    	return;
  	}
  	CMD_MANAGER_LIST_LOOP(gCmIpcBase->cmUserTable, tmpUser, nn)
  	{
   		i++;
		if(strcmp(tmpUser->cmUserEncryptMethod, "none") == 0){
			fprintf(ptr, "%s %s %s %s\n", tmpUser->cmUserName, cmdCmdUserGetMode(tmpUser->cmUserMode), tmpUser->cmUserEncryptMethod, tmpUser->cmUserPassword);
		}else{
			fprintf(ptr, "%s %s %s %s\n", tmpUser->cmUserName, cmdCmdUserGetMode(tmpUser->cmUserMode), tmpUser->cmUserEncryptMethod, tmpUser->cmUserPassEncrypt);
		}
  	}
	fclose(ptr);
}
struct cmUserManager_T *
cmdCmdUserCheck(Int8T *userName)
{
 struct cmUserManager_T *tmpUser;
 struct cmdListNode  *nn;
 CMD_MANAGER_LIST_LOOP(gCmIpcBase->cmUserTable, tmpUser, nn)
 {
   if(strcmp(tmpUser->cmUserName, userName) == 0)
   {
   if(strcmp(tmpUser->cmUserName, userName) == 0)
     return tmpUser;
   }
 }
 return NULL;
}
Int32T 
cmdCmdUserAdd(struct cmsh *cmsh, Int8T *userName, Int8T *userMode, Int8T *passWord)
{
 Int32T i;
 struct cmUserManager_T *tmpUser;
 tmpUser = cmdCmdUserCheck(userName);
 if(tmpUser == NULL){
	 tmpUser = (struct cmUserManager_T *)malloc(sizeof(*tmpUser));	
	 memset(tmpUser, 0, sizeof(*tmpUser));
	 tmpUser->cmUserMode = CM_USER_VIEW_MODE;
	 
	 for(i = CM_USER_VIEW_MODE; i < CM_USER_MAX_MODE; i++)
	 {
	   if(!strcmp(gUserMode[i].cmUserStrs, userMode))
	   {
		 tmpUser->cmUserMode = gUserMode[i].cmUserMode;
	   }
	 }
	 sprintf(tmpUser->cmUserName, "%s", userName);
	 sprintf(tmpUser->cmUserPassword, "%s", passWord);
	 sprintf(tmpUser->cmUserEncryptMethod, "%s",  "none");
 	cmdListAddNode(gCmIpcBase->cmUserTable, tmpUser);
 }else{
   sprintf(tmpUser->cmUserPassword, "%s", passWord);
   sprintf(tmpUser->cmUserEncryptMethod, "%s",  "none");
   cmdUserWriteStore();
   return CMD_IPC_ERROR; 
 }
 cmdUserWriteStore(); 
 return CMD_IPC_OK;
}
Int32T 
cmdCmdUserDel(Int8T *userName)
{
 struct cmUserManager_T *tmpUser;
 struct cmdListNode  *nn;
 CMD_MANAGER_LIST_LOOP(gCmIpcBase->cmUserTable, tmpUser, nn) 
 {
   if(!strcmp(tmpUser->cmUserName, userName))
   {
     cmdListDelNode(gCmIpcBase->cmUserTable, nn);
   }
 }
 cmdUserWriteStore();
 return CMD_IPC_OK;
}

Int8T *
cmdCmdUserGetMode(Int32T modeId)
{
 return gUserMode[modeId].cmUserStrs;
}

#ifndef _NOS_CFG_FILE_
#define _NOS_CFG_FILE_ "/etc/nos.conf"
#define _NOS_CFG_MAX_BUFF_SIZE 1024
#endif

Int8T *
cmdCmdParseNodeMul(struct cmsh *cmsh, Int8T *strCmd, Int32T *cmdType, Int32T *nodeCheck, Int32T *idDepth)
{
  struct cmdListNode *cNN;
  struct cmdElement *cel;
  Int8T *p = strCmd;
  Int32T preDepth, i, rv = 0, nDepth = *idDepth, nCheck = *nodeCheck;
  preDepth = cmsh->currentDepth;
	if(strcmp(strCmd, "!") == 0){
		*cmdType = CMD_RUNNING_CONFIG_NODE;
    	*idDepth = CMD_NODE_CONFIG;
    	*nodeCheck = 0;
	    cmsh->currentDepth = preDepth;
        return strCmd;
    }
    while (isspace ((Int32T) *p) && *p != '\0')
    {
        p++, strCmd++;
    }
    cmsh->currentDepth  = *idDepth;
    cmsh->inputStr = strCmd;
    cmsh->cmshFuncIndex = 0;
    if(cmdParseCommand(cmsh) == CMD_PARSE_SUCCESS)
    {
        CMD_MANAGER_LIST_LOOP(cmsh->cnode->cCelTable, cel, cNN)
        {
          cmsh->cel = cel;
          if(cmsh->cel->nextNode)
          {
            rv = CMD_RUNNING_COMMAND_NODE;
            cmsh->prevMode = cmsh->modeTrace[cmsh->currentDepth].mode;
            nDepth ++;
            cmsh->currentDepth = nDepth;
            cmsh->modeTrace[cmsh->currentDepth].mode = cmsh->cel->nextNode;
            cmsh->prompt = cmsh->ctree->prompt[cmsh->cel->nextNode];
          }
        }
    }else{
      rv = CMD_RUNNING_DEFAULT;
      for(i = nDepth; i >= CMD_NODE_CONFIG; i--)
      {
        cmsh->inputStr = "exit";
        cmsh->currentDepth  = i + 1;
        if(cmdParseCommand(cmsh) == CMD_PARSE_SUCCESS)
        {
          CMD_MANAGER_LIST_LOOP(cmsh->cnode->cCelTable, cel, cNN)
          {
            cmsh->cel = cel;
            cmsh->prevMode = cmsh->modeTrace[cmsh->currentDepth].mode;
            cmsh->currentDepth  --;
            cmsh->modeTrace[cmsh->currentDepth].mode = cmsh->cel->nextNode;
            cmsh->prompt = cmsh->ctree->prompt[cmsh->cel->nextNode];
            cmsh->inputStr = strCmd;
          }
        }
        if(cmdParseCommand(cmsh) == CMD_PARSE_SUCCESS)
        {
          CMD_MANAGER_LIST_LOOP(cmsh->cnode->cCelTable, cel, cNN)
          {
            cmsh->cel = cel;
            cmsh->prevMode = cmsh->modeTrace[cmsh->currentDepth].mode;
            cmsh->currentDepth  ++;
            cmsh->modeTrace[cmsh->currentDepth].mode = cmsh->cel->nextNode;
            cmsh->prompt = cmsh->ctree->prompt[cmsh->cel->nextNode];
            nDepth = i + 1;
            nCheck = i-CMD_NODE_CONFIG;
            rv = CMD_RUNNING_COMMAND_LINE;
            if(cmsh->cel->nextNode)
            {
              rv = CMD_RUNNING_EXIT_AUTO;
            }
          }
          break;
        }
      }
	  }
    *cmdType = rv;
    *idDepth = nDepth;
    *nodeCheck = nCheck;
    cmsh->currentDepth = preDepth;
    return strCmd;
}

Int32T
nosCmdSendStartConfig(struct cmsh *cmsh, Int32T fd, cmdIpcMsg_T *cmIpcHdr, Int8T *nodeStr)
{
  Int8T msgBuf[CMD_IPC_MAXLEN], nodeBuf[128];
  cmdIpcMsg_T tmpHdr;
  Int32T i;
  sprintf(nodeBuf,"%s", nodeStr);
  tmpHdr.code = CMD_IPC_CODE_FUNC_REQ;
  tmpHdr.requestKey = fd;
  tmpHdr.clientType = cmIpcHdr->clientType;//CMD_IPC_TYPE_CLI
  tmpHdr.mode = cmIpcHdr->mode;
  tmpHdr.userKey = cmIpcHdr->userKey;
  tmpHdr.srcID = IPC_CM_MGR;
  tmpHdr.dstID = cmIpcHdr->srcID;
  cmdIpcAddHdr(msgBuf, &tmpHdr);
  cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_INDEX, sizeof (Int32T), &cmsh->cmshFuncIndex);
  cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_FUNC, sizeof (Int32T), &cmsh->cel->funcID);
 
  Int32T cIndex, j;
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

  for(i = 0; i < cmsh->argc; i++)
  {
    cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_ARGSTR, strlen(cmsh->argv[i]), cmsh->argv[i]);
  }

  cmdIpcSend(fd, msgBuf);
  return CMD_IPC_OK;
}

Int32T
cmdCmModuleRunningStartup(struct cmsh *cmsh)
{
	struct cmshCallbackMap *sNode;
	struct cmdListNode *ncm;
	Int8T *cArgvPtr[CMSH_ARGC_MAX];
	Int8T *nArgvPtr1[CMSH_ARGC_MAX];
	Int8T *nArgvPtr2[CMSH_ARGC_MAX];
	Int8T *nArgvPtr3[CMSH_ARGC_MAX];
	Int8T *nArgvPtr4[CMSH_ARGC_MAX];
	Int8T *nArgvPtr5[CMSH_ARGC_MAX];
	Int32T nArgc1 = 0, nArgc2 = 0, nArgc3 = 0, nArgc4 = 0, nArgc5 = 0, cArgc = 0;
	Int32T cIndex, j, i, rv;
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
						nArgvPtr1[j] = cmsh->modeTrace[i].modeArgv[j];
						nArgc1 ++;
					}
				break;
				case 2:
					for(j = 0; j < cmsh->modeTrace[i].modeArgc; j++)
					{
						nArgvPtr2[j] = cmsh->modeTrace[i].modeArgv[j];
						nArgc2 ++;
					}
				break;
				case 3:
					for(j = 0; j < cmsh->modeTrace[i].modeArgc; j++)
					{
						nArgvPtr3[j] = cmsh->modeTrace[i].modeArgv[j];
						nArgc3 ++;
					}
				break;
				case 4:
					for(j = 0; j < cmsh->modeTrace[i].modeArgc; j++)
					{
						nArgvPtr4[j] = cmsh->modeTrace[i].modeArgv[j];
						nArgc4 ++;
					}
				break;
				case 5:
					for(j = 0; j < cmsh->modeTrace[i].modeArgc; j++)
					{
						nArgvPtr5[j] = cmsh->modeTrace[i].modeArgv[j];
						nArgc5 ++;
					}
				break;
			}
			cIndex++;
		}
	}
	for (i = 0; i < (cmsh->argc); i++)
	{
		cArgvPtr[i] = cmsh->argv[i];
		cArgc  ++;
	}
	CMD_MANAGER_LIST_LOOP(cmsh->callbackList, sNode, ncm)
	{
		if((sNode->funcID == cmsh->cel->funcID) &&(sNode->func != NULL))
		{
			rv = (sNode->func)(cmsh, nArgc1, nArgvPtr1, nArgc2, nArgvPtr2, nArgc3, nArgvPtr3, nArgc4, nArgvPtr4, nArgc5, nArgvPtr5, cArgc, cArgvPtr);
			if(rv == CMD_IPC_OK)
			{
				if(cmsh->outMod != CMD_OUTMOD_EXCLUDE) /** Send end of string  */
				{
					cmsh->outMod = CMD_OUTMOD_EXCLUDE;
					cmdPrint(cmsh,"\n");
				}
			}
		}
	}
	return CMD_IPC_OK;
}

Int32T
nosCmdConfigParseCommand(cmMgrT *cm, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
  struct cmdListNode *nn, *cNN;
	Int8T *strCmd;
	Int32T preCurrentDepth, i, rv;
 	struct cmsh *cmsh = cm->cmsh;
  struct cmdElement *cel;

	preCurrentDepth = cmsh->currentDepth;
 	cmsh->currentDepth = CMD_NODE_CONFIG;
  CMD_MANAGER_LIST_LOOP(cm->cmConfigTableMul, strCmd, nn)
  {
    rv = 0;
    cmsh->inputStr = strdup(strCmd);
    cmsh->cmshFuncIndex = 0;
    if(strcmp(strCmd, "!") == 0)
    {
      cmsh->currentDepth = CMD_NODE_CONFIG;
    }

    if(cmdParseCommand(cmsh) == CMD_PARSE_SUCCESS)
    {
      CMD_MANAGER_LIST_LOOP(cmsh->cnode->cCelTable, cel, cNN)
      {
        cmsh->cel = cel;
        if(cmsh->cel->nextNode)
        {
          rv = 1;
          break;
        }
      }
    } 
    else 
    {
      Int32T nDepth = cmsh->currentDepth;
      rv = 2;
      for(i = nDepth; i >= CMD_NODE_CONFIG; i--)
      {
        cmsh->inputStr = "exit";
        cmsh->currentDepth  = i + 1;
        if(cmdParseCommand(cmsh) == CMD_PARSE_SUCCESS)
        {
          CMD_MANAGER_LIST_LOOP(cmsh->cnode->cCelTable, cel, cNN)
          {
            cmsh->cel = cel;
            cmsh->prevMode = cmsh->modeTrace[cmsh->currentDepth].mode;
            cmsh->currentDepth  --;
            cmsh->modeTrace[cmsh->currentDepth].mode = cmsh->cel->nextNode;
            cmsh->prompt = cmsh->ctree->prompt[cmsh->cel->nextNode];
            cmsh->inputStr = strdup(strCmd);
          }
        }
        if(cmdParseCommand(cmsh) == CMD_PARSE_SUCCESS)
        {
          CMD_MANAGER_LIST_LOOP(cmsh->cnode->cCelTable, cel, cNN)
          {
            cmsh->cel = cel;
            rv = 0;
            if(cmsh->cel->nextNode)
            {
              rv = 1;
            }
          }
          break;
        }
      }			
    }
    if(rv == 1)
    {
      Int8T *tokenStr, *tmpStr = cmsh->inputStr;
      enum cmdToken type;
 	  Int32T flags;
      cmsh->modeTrace[cmsh->currentDepth].modeArgc = 0;
      while((tmpStr = cmdGetToken(tmpStr, &type, &flags, &tokenStr)))
      {
        cmsh->modeTrace[cmsh->currentDepth].modeArgv[cmsh->modeTrace[cmsh->currentDepth].modeArgc] = strdup(tokenStr);
        cmsh->modeTrace[cmsh->currentDepth].modeArgc++;
      }
      cmsh->prevMode = cmsh->modeTrace[cmsh->currentDepth].mode;
      cmsh->currentDepth++;
      cmsh->modeTrace[cmsh->currentDepth].mode = cmsh->cel->nextNode;
      cmsh->prompt = cmsh->ctree->prompt[cmsh->cel->nextNode];
    }
    else if(rv == 0)
    {
	  if(cmIpcHdr != NULL){
		  if(cmdCompBitCheck(cmsh->cel->flags, cmIpcHdr->srcID))
		  {
		  	nosCmdSendStartConfig(cmsh, fd, cmIpcHdr, strCmd);
		  	cm->compTable[cmIpcHdr->srcID].flagID++;
		  }
      }		
	  if(cmdCompBitCheck(cmsh->cel->flags,IPC_CM_MGR)){
		cmdCmModuleRunningStartup(cmsh);
	  }
    }
  }
	cmsh->currentDepth = preCurrentDepth;
  return CMD_IPC_OK;
}

Int32T
cmdCmdConfigInitMul(cmMgrT *cm)
{
  Int8T *cfgPtr, cfgLine[_NOS_CFG_MAX_BUFF_SIZE];
  Int32T cfgSize;
  FILE *ptr;
  cm->cmConfigTableMul = cmdListNew();
  if((ptr = fopen(_NOS_CFG_FILE_, "r")) != NULL)
  {
    	while(fgets (cfgLine, _NOS_CFG_MAX_BUFF_SIZE, ptr))
    	{
			cfgSize = strlen(cfgLine);
			cfgLine[cfgSize-1] = '\0';
			cfgPtr = strdup(cfgLine);
			cmdListAddNode(cm->cmConfigTableMul, cfgPtr);
		}
    fclose(ptr);
  }
  nosCmdConfigParseCommand(cm, 0, NULL, NULL);
  return CMD_IPC_OK;
}
Int32T
cmdCmdConfigFree(cmMgrT *cm)
{
	Int8T *cmdStr;
  struct cmdListNode *nn;
  CMD_MANAGER_LIST_LOOP(cm->cmConfigTableMul, cmdStr, nn)
  {
	if(cmdStr != NULL)
    	free(cmdStr);
  }
  cmdListDel(cm->cmConfigTableMul);
  return CMD_IPC_OK;
}
Int32T
cmdCmdConfigReLoad(cmMgrT *cm)
{
	if(cm->cmConfigTableMul == NULL)
	{
		return CMD_IPC_ERROR;
	}
	cmdCmdConfigFree(cm);
	cmdCmdConfigInitMul(cm);
	return CMD_IPC_OK;
}

void
CmdFunWriteRuningConfigMul(FILE *ptr,  struct cmConfigManagerMulNode_T * currentNode)
{
	struct cmConfigManagerMulNode_T *node, *nodePre = NULL;
  	struct cmdListNode *nn;
  	CMD_MANAGER_LIST_LOOP(currentNode->cmdStrTable, node, nn)
  	{
		if(nodePre != NULL){
			if(nodePre->cmdStrTable != NULL){
            cmdListDel(nodePre->cmdStrTable);
            nodePre->cmdStrTable = NULL;
        	}
		}
		if(node->cFlagh == 1){
    		fprintf(ptr," %s\n", node->nodeShow);
		}else{
    		fprintf(ptr,"%s\n", node->nodeShow);
		}
		nodePre = node;
		if(node->nodeShow != NULL)
        	free(node->nodeShow);
    	if(node->nodeStr != NULL)
        	free(node->nodeStr);
    	CmdFunWriteRuningConfigMul(ptr, node);
  	}
}

Int32T
cmdCmdConfigWrite(cmMgrT *cm)
{
  struct cmConfigManagerMulNode_T *node;
  struct cmdListNode *nn;
  FILE *ptr;
  if((ptr = fopen(_NOS_CFG_FILE_, "w")) == NULL)
  {
    return CMD_IPC_ERROR;
  }
    CMD_MANAGER_LIST_LOOP(cm->cmCurrConfigTableMul, node, nn)
    {
        fprintf(ptr, "!\n%s\n", node->nodeShow);
        CmdFunWriteRuningConfigMul(ptr, node);
        if(node->cmdStrTable != NULL){
            cmdListDel(node->cmdStrTable);
            node->cmdStrTable = NULL;
            }
    }
	if(cm->cmCurrConfigTableMul != NULL){
            cmdListDel(gCmIpcBase->cmCurrConfigTableMul);
            cm->cmCurrConfigTableMul = NULL;
        }
  fclose(ptr);
  return CMD_IPC_OK;
}


void
cmMainFree(void)
{
	Int32T i;
	for (i = 0; i < CMD_MAX_SESSION; i++)
  {
		if (gCmIpcBase->cmshTable[i].userKey)
		{
			close(gCmIpcBase->cmshTable[i].fd);
		}
	}
	for(i = 0; i < IPC_MAX_MGR; i++)
	{
		if(gCmIpcBase->compTable[i].sockID)
		{
			close(gCmIpcBase->compTable[i].sockID);
		}
	}
  cmdCmdServiceStop(gCmIpcBase);
}

Int32T
cmdSetPasswordEnable(Int8T *password)
{
  if(password != NULL){
    strcpy(gCmIpcBase->passwordCm.enablePw, password);
    gCmIpcBase->passwordCm.flag_enable = 0;
  }else{
    memset(gCmIpcBase->passwordCm.enablePw, 0,CMD_PASSWD_MAX);
    memset(gCmIpcBase->passwordCm.enablePw_encrypt, 0,CMD_PASSWD_MAX);
    gCmIpcBase->passwordCm.flag_enable = 2;
  }
  return SUCCESS;
}
Int32T
cmdSetPasswordConfig(Int8T *password)
{
  if(password != NULL){
      strcpy(gCmIpcBase->passwordCm.configurePw, password);
    gCmIpcBase->passwordCm.flag_config = 0;
  }else{
    memset(gCmIpcBase->passwordCm.configurePw, 0,CMD_PASSWD_MAX);
    memset(gCmIpcBase->passwordCm.configurePw_encrypt, 0,CMD_PASSWD_MAX);
    gCmIpcBase->passwordCm.flag_config = 2;
  }
  return SUCCESS;
}
Int32T
cmdCmdSetPassword(cmMgrT *cm)
{
  memset(&cm->passwordCm, 0, sizeof(struct cmPassword));
  strcpy(cm->passwordCm.enablePw, "root");
  strcpy(cm->passwordCm.configurePw, "root");
  strcpy(cm->passwordCm.key_hash, "MD5");
  return SUCCESS;
}

Int8T*
str2md5(Int8T* str, Int32T length)
{
    Int32T n;
    MD5_CTX c;
    Int8T *out = (Int8T*)malloc(33);
    MD5Init(&c);
    while (length > 0) {
        if (length > 512) {
            MD5Update(&c, (void*)str, 512);
        } else {
            MD5Update(&c, (void*)str, length);
        }
        length -= 512;
        str += 512;
    }
    MD5Final(&c);
    for (n = 0; n < 16; ++n) {
        snprintf(&(out[n*2]), 16*2, "%02x", (Uint32T)c.digest[n]);
    }
    return out;
}

void 
cmdSha256(Int8T *string, Int8T outputBuffer[65])
{
    Uint8T hash[SHA256_DIGEST_LENGTH];
	Int32T i = 0;
    SHA256_Context_T sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, (void *)string, strlen(string));
    SHA256_Finish(&sha256, hash);
    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = 0;
}

void
cmdSha512(Int8T *string, Int8T outputBuffer [129])
{
	Uint8T digest[SHA512_DIGEST_LENGTH];
	Int32T i = 0;
	SHA512_Context_T ctx;
	SHA512_Init(&ctx);
	SHA512_Update(&ctx, (void*)string, strlen(string));
	SHA512_Finish(&ctx, digest);
	for (i = 0; i < SHA512_DIGEST_LENGTH; i++)
			sprintf(&outputBuffer[i*2], "%02x", (Uint32T)digest[i]);
	outputBuffer[128] = 0;
}

Int32T
cmdTestEncryp()
{
    Int8T* output = str2md5("hello", strlen("hello"));
    printf("%s \n", output);
	Int8T buffer[65];
	cmdSha256("hellotesthai", buffer);
	Int8T buff512[129];
	cmdSha512("hello", buff512);
    printf("%s\n", buffer);
    printf("%s\n", buff512);
	
    return SUCCESS;
}

Int32T
cmMainInit(void)
{
  gCmIpcBase->cmsh = cmdCmshInit(IPC_CM_MGR);
  cmdFuncGlobalInstall(gCmIpcBase->cmsh);
  gCmIpcBase->configSession = -1;
  cmdCmdSetPassword(gCmIpcBase);
  gCmIpcBase->cmUserTable = cmdListNew();  /** Create Table User */
  cmdUserReadStore();
  gCmIpcBase->cmService.pTelnetPortId = 0;
  gCmIpcBase->cmService.pSshPortId = 0;
  gCmIpcBase->cmServiceStartup = cmdListNew();
  /** cmdCmdServiceInit  */
  cmdCmdServiceInit(gCmIpcBase);
  cmdCmdConfigInitMul(gCmIpcBase);
  return CMD_IPC_OK;
}

Int32T
cmdGetPrivilegeFromUserID(Int8T *uName)
{
 struct cmUserManager_T *tmpUser;
 struct cmdListNode  *nn;
 CMD_MANAGER_LIST_LOOP(gCmIpcBase->cmUserTable, tmpUser, nn)
 {
   if(strcmp(tmpUser->cmUserName, uName) == 0)
   {
    return tmpUser->cmUserMode;
   }
  }
  return PRIVILEGE_MAX;
}

Int32T
cmdEncryptPassword(Int8T *password, Int8T **encrypt_password)
{
  Int8T *tmp =  (Int8T *)malloc(sizeof(password));
  strcpy(tmp, password);
  *encrypt_password = tmp;
  return SUCCESS;
}
Int32T
cmdIpcServRegister(cmMgrT * cm, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
    Int8T tlvUserID[CMD_USERID_MAX];
    Int32T userIDLen;
    Int8T msgBuf[CMD_IPC_MAXLEN];
    Int16T privilege = PRIVILEGE_MIN;  /** Change from PRIVILEGE_MAX --> PRIVILEGE_MIN */
    Int32T i;
    for (i = 0; i < CMD_MAX_SESSION; i++)
    {
        /** Note: if (userKey, fd) is same, we assume that they are same session. */
        if ((cm->cmshTable[i].userKey == cmIpcHdr->userKey)
            && (cm->cmshTable[i].fd == fd))
        {
            /** Note: to delete session, we can set (userKey = NULL) for the session list entry */
            cm->cmshTable[i].userKey = FALSE;
            /** If that session has configure lock, release it. */
            if(cm->configSession == i)
            {
                cm->configSession = -1;
            }
        }
    }

    cmdIpcUnpackTlv(data, CMD_IPC_TLV_USERID, &userIDLen, tlvUserID);
    tlvUserID[userIDLen] = '\0';
    privilege = cmdGetPrivilegeFromUserID(tlvUserID);
    for (i = 0; i< CMD_MAX_SESSION; i++)
    {
        if (cm->cmshTable[i].userKey != 1)
        {
            cm->cmshTable[i].userKey = cmIpcHdr->userKey;
            strncpy(cm->cmshTable[i].userID, tlvUserID, userIDLen);
            cm->cmshTable[i].clientType = cmIpcHdr->clientType;
            cm->cmshTable[i].mode = cmIpcHdr->mode;
            cm->cmshTable[i].srcID = cmIpcHdr->srcID;
            cm->cmshTable[i].dstID = cmIpcHdr->dstID;
            cm->cmshTable[i].fd = fd;
            cm->cmshTable[i].privilege = privilege;
            break;
        }
    }
    /**  SEND BACK   */
    cmIpcHdr->code = CMD_IPC_CODE_CLIENT_REG_RES_OK;
    cmIpcHdr->dstID = cmIpcHdr->srcID;
    cmIpcHdr->srcID = cm->cmsh->gTypes;
    cmdIpcAddHdr(msgBuf, cmIpcHdr);
    cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_PRIVILEGE, sizeof(Uint16T), &privilege);
    cmdIpcSend(fd, msgBuf);
    return CMD_IPC_OK;
}

Int32T
cmdIpcServDeregister(cmMgrT * cm, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
    char msgBuf[CMD_IPC_MAXLEN];
    Int32T i;
    UNUSED(data);
    for (i = 0; i < CMD_MAX_SESSION; i++)
    {
        /** Note: if (userKey, fd) is same, we assume that they are same session. */
        if ((cm->cmshTable[i].userKey == cmIpcHdr->userKey)
            && (cm->cmshTable[i].fd == fd))
        {
            /** Note: to delete session, we can set (userKey = NULL) for the session list entry */
            cm->cmshTable[i].userKey = 0;
            /** If that session has configure lock, release it. */
            if(cm->configSession == i)
            {
                cm->configSession = -1;
            }
        }
    }
    /**  Send Back   */
    cmIpcHdr->code = CMD_IPC_CODE_CLIENT_DEREG_RES_OK;
    cmIpcHdr->dstID = cmIpcHdr->srcID;     /** IPC_CM_MGRSH  */
    cmIpcHdr->srcID = IPC_CM_MGR;
    cmdIpcAddHdr(msgBuf, cmIpcHdr);
    cmdIpcSend(fd, msgBuf);
    return CMD_IPC_OK;
}
Int32T
cmdIpcServAuthEnable(cmMgrT * cm, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
    Int8T msgBuf[CMD_IPC_MAXLEN];
    Int32T i;
    cmdCmCmshSession_T *cmdss = NULL;
    UNUSED(data);
    for (i=0; i < CMD_MAX_SESSION; i++)
    {
        if ((cm->cmshTable[i].userKey == cmIpcHdr->userKey)
            && (cm->cmshTable[i].fd == fd))
        {
            cmdss = &(cm->cmshTable[i]);
        }
    }
    if (cmdss->privilege < cm->cmsh->ctree->nodePrivilege[CMD_NODE_EXEC])
    {
        cmIpcHdr->code = CMD_IPC_CODE_AUTH_ENABLE_RES_NOK;
    }
    else if (cm->passwordCm.flag_enable != 2)
    {
        cmIpcHdr->code = CMD_IPC_CODE_AUTH_ENABLE_REQ_PASSWD;
    }
    else
    {
        cmIpcHdr->code = CMD_IPC_CODE_AUTH_ENABLE_RES_OK;
        cmdss->mode = CMD_NODE_EXEC;
    }
    cmIpcHdr->dstID = cmIpcHdr->srcID;     /** IPC_CM_MGRSH  */
    cmIpcHdr->srcID = IPC_CM_MGR;
    cmdIpcAddHdr(msgBuf, cmIpcHdr);
    if(cmdIpcSend(fd, msgBuf) < 0)
    {
      fprintf(stderr,"[%s][%s][%d][Error send]\n", __FILE__, __func__, __LINE__);
    }
    return CMD_IPC_OK;
}

Int32T
cmdIpcServAuthEnablePasswd(cmMgrT * cm, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
    Int8T passwd[CMD_PASSWD_MAX];
    Int32T passwdLen;
    Int8T msgBuf[CMD_IPC_MAXLEN];
    Int32T i;
    cmdCmCmshSession_T *cmdss = NULL;
    for (i=0; i < CMD_MAX_SESSION; i++)
    {
        /** Note: if (userKey, fd) is same, we assume that they are same session. */
        if ((cm->cmshTable[i].userKey == cmIpcHdr->userKey)
            && (cm->cmshTable[i].fd == fd))
        {
            cmdss = &(cm->cmshTable[i]);
            break;
        }
    }

    if (cmdss->privilege < cm->cmsh->ctree->nodePrivilege[CMD_NODE_EXEC])
    {
        cmIpcHdr->code = CMD_IPC_CODE_AUTH_ENABLE_RES_NOK;
    }
    memset(passwd, 0, CMD_PASSWD_MAX);
    cmdIpcUnpackTlv(data, CMD_IPC_TLV_PASSWD, &passwdLen, passwd);
    passwd[passwdLen] = '\0';
    if(gCmIpcBase->passwordCm.flag_enable == 0) {
      if (0 == strcmp(cm->passwordCm.enablePw, passwd))

      {
        cmIpcHdr->code = CMD_IPC_CODE_AUTH_ENABLE_RES_OK;
        cmdss->mode = CMD_NODE_EXEC;
      }
      else
      {
        cmIpcHdr->code = CMD_IPC_CODE_AUTH_ENABLE_RES_NOK;
      }
    }else if(gCmIpcBase->passwordCm.flag_enable == 1){
      Int8T *encrypted;
      cmdEncryptPassword(passwd, &encrypted);

      if (0 == strcmp(cm->passwordCm.enablePw_encrypt, (Int8T*)encrypted))
      {
        cmIpcHdr->code = CMD_IPC_CODE_AUTH_ENABLE_RES_OK;
        cmdss->mode = CMD_NODE_EXEC;
      }
      else
      {
        cmIpcHdr->code = CMD_IPC_CODE_AUTH_ENABLE_RES_NOK;
      }
      free(encrypted);
    }
    cmIpcHdr->dstID = cmIpcHdr->srcID;     /** IPC_CM_MGRSH  */
    cmIpcHdr->srcID = IPC_CM_MGR;
    cmdIpcAddHdr(msgBuf, cmIpcHdr);
    cmdIpcSend(fd, msgBuf);
    return CMD_IPC_OK;
}
Int32T
cmdIpcServAuthConfig(cmMgrT * cm, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
    char msgBuf[CMD_IPC_MAXLEN];
    int i;
    cmdCmCmshSession_T *cmdss = NULL;
    UNUSED(data);
    for (i=0; i < CMD_MAX_SESSION; i++)
    {
        /** Note: if (userKey, fd) is same, we assume that they are same session. */
        if ((cm->cmshTable[i].userKey == cmIpcHdr->userKey)
            && (cm->cmshTable[i].fd == fd))
        {
            cmdss = &(cm->cmshTable[i]);
            break;
        }
    }

    if (cmdss->privilege < cm->cmsh->ctree->nodePrivilege[CMD_NODE_CONFIG])
    {
        cmIpcHdr->code = CMD_IPC_CODE_AUTH_CONFIGURE_RES_NOK;
    }
    else if (cm->configSession >= 0)
    {
        cmIpcHdr->code = CMD_IPC_CODE_AUTH_CONFIGURE_RES_EXIST;
    }
    else if (cm->passwordCm.flag_config != 2)
    {
        cmIpcHdr->code = CMD_IPC_CODE_AUTH_CONFIGURE_REQ_PASSWD;
    }
    else
    {
        cmIpcHdr->code = CMD_IPC_CODE_AUTH_CONFIGURE_RES_OK;
        cmdss->mode = CMD_NODE_CONFIG;
    }
    cmIpcHdr->dstID = cmIpcHdr->srcID;     /** IPC_CM_MGRSH  */
    cmIpcHdr->srcID = IPC_CM_MGR;
    cmdIpcAddHdr(msgBuf, cmIpcHdr);
    cmdIpcSend(fd, msgBuf);
    return CMD_IPC_OK;
}
Int32T
cmdIpcServAuthConfigPasswd(cmMgrT * cm, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
    char passwd[CMD_PASSWD_MAX];
    int passwdLen;
    char msgBuf[CMD_IPC_MAXLEN];
    int i;
    cmdCmCmshSession_T *cmdss = NULL;
    for (i=0; i < CMD_MAX_SESSION; i++)
    {
        /** Note: if (userKey, fd) is same, we assume that they are same session. */
        if ((cm->cmshTable[i].userKey == cmIpcHdr->userKey)
            && (cm->cmshTable[i].fd == fd))
        {
            cmdss = &(cm->cmshTable[i]);
            break;
        }
    }

    if (cmdss->privilege < cm->cmsh->ctree->nodePrivilege[CMD_NODE_CONFIG])
    {
        cmIpcHdr->code = CMD_IPC_CODE_AUTH_CONFIGURE_RES_NOK;
    }
    else if (cm->configSession >= 0)
    {
        cmIpcHdr->code = CMD_IPC_CODE_AUTH_CONFIGURE_RES_EXIST;

    }
    memset(passwd, 0, CMD_PASSWD_MAX);
    cmdIpcUnpackTlv(data, CMD_IPC_TLV_PASSWD, &passwdLen, passwd);
    passwd[passwdLen] = '\0';
    if(cm->passwordCm.flag_config == 0){
      if (0 == strcmp(cm->passwordCm.configurePw, passwd))
      {
        cmIpcHdr->code = CMD_IPC_CODE_AUTH_CONFIGURE_RES_OK;
        cmdss->mode = CMD_NODE_CONFIG;
        cm->configSession = i;
      }
      else
      {
        cmIpcHdr->code = CMD_IPC_CODE_AUTH_CONFIGURE_RES_NOK;
      }
    }else if(cm->passwordCm.flag_config == 1){
      Int8T *encrypted;
      cmdEncryptPassword(passwd, &encrypted);
      if (0 == strcmp(cm->passwordCm.configurePw_encrypt, (char*)encrypted))
      {
        cmIpcHdr->code = CMD_IPC_CODE_AUTH_CONFIGURE_RES_OK;
        cmdss->mode = CMD_NODE_CONFIG;
        cm->configSession = i;
      }
      else
      {
        cmIpcHdr->code = CMD_IPC_CODE_AUTH_CONFIGURE_RES_NOK;
      }
    }
    cmIpcHdr->dstID = cmIpcHdr->srcID;     /** IPC_CM_MGRSH  */
    cmIpcHdr->srcID = IPC_CM_MGR;
    cmdIpcAddHdr(msgBuf, cmIpcHdr);
    cmdIpcSend(fd, msgBuf);

    return CMD_IPC_OK;
}
Int32T
cmdIpcServUpdateMode(cmMgrT * cm, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
  cmdCmCmshSession_T *cmdss = NULL;
  Int8T value[CMSH_ARGV_MAX_LEN];
  Int32T i, size;
  for (i=0; i < CMD_MAX_SESSION; i++)
  {
      /** Note: if (userKey, fd) is same, we assume that they are same session. */
      if ((cm->cmshTable[i].userKey == cmIpcHdr->userKey)
          && (cm->cmshTable[i].fd == fd))
      {
          cmdss = &(cm->cmshTable[i]);
          break;
      }
  }
  if(cmdss != NULL)
  {
    cmdIpcUnpackTlv(data, CMD_IPC_TLV_MODE, &size, &cmdss->mode);
    cmdIpcUnpackTlv(data, CMD_IPC_TLV_MODE_ARGC, &size, &cmdss->argc);
    for(i = 0; i < cmdss->argc; i++)
    {
      memset(&value, 0, CMSH_ARGV_MAX_LEN);
      cmdIpcUnpackTlv(data, CMD_IPC_TLV_MODE_ARGV, &size, &value);
      cmdss->argv[i] = strdup(value);
    }
  }
  return CMD_IPC_OK;
}

Int32T
cmdIpcServAuthExit(cmMgrT * cm, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
  cmdCmCmshSession_T *cmdss = NULL;
	Int8T msgBuf[CMD_IPC_MAXLEN];
  Int32T i;
  for (i=0; i < CMD_MAX_SESSION; i++)
  {
      /** Note: if (userKey, fd) is same, we assume that they are same session. */
      if ((cm->cmshTable[i].userKey == cmIpcHdr->userKey)
          && (cm->cmshTable[i].fd == fd))
      {
          cmdss = &(cm->cmshTable[i]);
          break;
      }
  }
  if(cmdss != NULL)
  {
	switch(cmIpcHdr->mode)
    {
            case CMD_NODE_VIEW:
                cmIpcHdr->code = CMD_IPC_CODE_FUNC_EXIT;
                cmdss->mode = CMD_NODE_NULL;
            //        cm->configSession = -1;
            break;
            case CMD_NODE_EXEC:
                if(cmdss->mode >= CMD_NODE_CONFIG){
                    cm->configSession = -1;
				}
				cmdss->mode = CMD_NODE_VIEW;
                cmIpcHdr->code = CMD_IPC_CODE_CLIENT_REG_RES_OK;
            break;
            case CMD_NODE_CONFIG:
                cmIpcHdr->code = CMD_IPC_CODE_AUTH_ENABLE_RES_OK;
                cmdss->mode = CMD_NODE_EXEC;
                cm->configSession = -1;
            break;
            default:
                cmIpcHdr->code = CMD_IPC_CODE_AUTH_ENABLE_RES_OK;
                cmdss->mode = CMD_NODE_CONFIG;
            break;
    }

    cmIpcHdr->dstID = cmIpcHdr->srcID;     /** MO_CMSH  */
    cmIpcHdr->srcID = IPC_CM_MGR;
    cmdIpcAddHdr(msgBuf, cmIpcHdr);
    cmdIpcSend(fd, msgBuf);
	}
	return CMD_IPC_OK;
}

Int32T
cmdIpcServFuncCli(cmMgrT * cm, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
  cmdCmCmshSession_T *cmdss = NULL;
  Int32T i, rv;
  Int32T funcID, funcIndex, sSize, cArgc = 0;
  Int32T nArgc1 = 0, nArgc2 = 0, nArgc3 = 0, nArgc4 = 0, nArgc5 = 0;
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
  
  for (i=0; i< CMD_MAX_SESSION; i++)
  {
      /** Note: if (userKey, fd) is same, we assume that they are same session. */
      if ((cm->cmshTable[i].userKey == cmIpcHdr->userKey)
          && (cm->cmshTable[i].fd == fd))
      {
          cmdss = &(cm->cmshTable[i]);
          break;
      }
  }
  if(cmdss != NULL)
  {
		struct cmsh *vCmsh = NULL;
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
    /** Find Func here  */
    if(cmIpcHdr->dstID == IPC_CM_MGR)
    {
      struct cmshCallbackMap *sNode;
      struct cmdListNode *nn;
      CMD_MANAGER_LIST_LOOP(cm->cmsh->callbackList, sNode, nn)
      {
        if(sNode->funcID == funcID)
        {
          rv = (sNode->func)(vCmsh, nArgc1, nArgvPtr1, nArgc2, nArgvPtr2, nArgc3, nArgvPtr3, nArgc4, nArgvPtr4, nArgc5, nArgvPtr5, cArgc, cArgvPtr);
          switch(rv)
          {
            case CMD_IPC_OK:
            {
              if(vCmsh->outMod != CMD_OUTMOD_EXCLUDE) /** Send end of string  */
              {
                vCmsh->outMod = CMD_OUTMOD_EXCLUDE;
                cmdPrint(vCmsh,"\n");
              }
            }
            break;
            case CMD_IPC_PASS_WAIT:
            {
              Int8T msgBuf[CMD_IPC_MAXLEN];
              cmdIpcMsg_T tmpHdr;
              tmpHdr.code = CMD_IPC_CODE_FUNC_REQ_PASSWD;
              tmpHdr.requestKey = cmIpcHdr->requestKey;
              tmpHdr.clientType = cmIpcHdr->clientType;
              tmpHdr.mode = cmIpcHdr->mode;
              tmpHdr.userKey = cmIpcHdr->userKey;
              tmpHdr.srcID = IPC_CM_MGR;
              tmpHdr.dstID = cmIpcHdr->srcID;
              cmdIpcAddHdr(msgBuf, &tmpHdr);
              cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_INDEX, sizeof (Int32T), &funcIndex);
              cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_FUNC, sizeof (Int32T), &funcID);
              
              for(i = 0; i < nArgc1; i++)
              {
                cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGV1, strlen(nArgvPtr1[i]), nArgvPtr1[i]);
              }
              for(i = 0; i < nArgc2; i++)
              {
                cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGV2, strlen(nArgvPtr2[i]), nArgvPtr2[i]);
              }
              for(i = 0; i < nArgc3; i++)
              {
                cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGV3, strlen(nArgvPtr3[i]), nArgvPtr3[i]);
              }
              for(i = 0; i < nArgc4; i++)
              {
                cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGV4, strlen(nArgvPtr4[i]), nArgvPtr4[i]);
              }
              for(i = 0; i < nArgc5; i++)
              {
                cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGV5, strlen(nArgvPtr5[i]), nArgvPtr5[i]);
              }

              for(i = 0; i < cArgc; i++)
              {
                cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_ARGSTR, strlen(cArgvPtr[i]), cArgvPtr[i]);
              }
              cmdIpcSend(fd, msgBuf);
            }
            break;
            case CMD_IPC_WAIT:
            {
              Int8T msgBuf[CMD_IPC_MAXLEN];
              cmdIpcMsg_T tmpHdr;
              tmpHdr.code = CMD_IPC_CODE_FUNC_RES_WAIT;
              tmpHdr.requestKey = cmIpcHdr->requestKey;
              tmpHdr.clientType = cmIpcHdr->clientType;
              tmpHdr.mode = cmIpcHdr->mode;
              tmpHdr.userKey = cmIpcHdr->userKey;
              tmpHdr.srcID = IPC_CM_MGR;
              tmpHdr.dstID = cmIpcHdr->srcID;
              cmdIpcAddHdr(msgBuf, &tmpHdr);
              cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_INDEX, sizeof (Int32T), &funcIndex);
              cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_FUNC, sizeof (Int32T), &funcID);

              for(i = 0; i < nArgc1; i++)
              {
                cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGV1, strlen(nArgvPtr1[i]), nArgvPtr1[i]);
              }
              for(i = 0; i < nArgc2; i++)
              {
                cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGV2, strlen(nArgvPtr2[i]), nArgvPtr2[i]);
              }
              for(i = 0; i < nArgc3; i++)
              {
                cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGV3, strlen(nArgvPtr3[i]), nArgvPtr3[i]);
              }
              for(i = 0; i < nArgc4; i++)
              {
                cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGV4, strlen(nArgvPtr4[i]), nArgvPtr4[i]);
              }
              for(i = 0; i < nArgc5; i++)
              {
                cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGV5, strlen(nArgvPtr5[i]), nArgvPtr5[i]);
              }

              for(i = 0; i < cArgc; i++)
              {
                cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_ARGSTR, strlen(cArgvPtr[i]), cArgvPtr[i]);
              }
              cmdIpcSend(fd, msgBuf);
            }
            break;
            default:
            {
              fprintf(stderr,"Unknown return ...\n");
            }
            break;
          }
        }
      }
    }
    else
    /** Forward to components  */
    {
      Int8T msgBuf[CMD_IPC_MAXLEN];
      cmdIpcMsg_T tmpHdr;
      tmpHdr.code = cmIpcHdr->code;
      tmpHdr.requestKey = fd;
      tmpHdr.clientType = cmIpcHdr->clientType;
      tmpHdr.mode = cmIpcHdr->mode;
      tmpHdr.userKey = cmIpcHdr->userKey;
      tmpHdr.srcID = cmIpcHdr->srcID;
      tmpHdr.dstID = cmIpcHdr->dstID;
      cmdIpcAddHdr(msgBuf, &tmpHdr);
      cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_INDEX, sizeof (Int32T), &funcIndex);
      cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_FUNC, sizeof (Int32T), &funcID);
      /** Add Node before send to componetnts */
      for(i = 0; i < nArgc1; i++)
      {
        cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGV1, strlen(nArgvPtr1[i]), nArgvPtr1[i]);
      }
      for(i = 0; i < nArgc2; i++)
      {
        cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGV2, strlen(nArgvPtr2[i]), nArgvPtr2[i]);
      }
      for(i = 0; i < nArgc3; i++)
      {
        cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGV3, strlen(nArgvPtr3[i]), nArgvPtr3[i]);
      }
      for(i = 0; i < nArgc4; i++)
      {
        cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGV4, strlen(nArgvPtr4[i]), nArgvPtr4[i]);
      }
      for(i = 0; i < nArgc5; i++)
      {
        cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_MODE_ARGV5, strlen(nArgvPtr5[i]), nArgvPtr5[i]);
      }

      /** End   */
      for(i = 0; i < cArgc; i++)
      {
        cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_ARGSTR, strlen(cArgvPtr[i]), cArgvPtr[i]);
      }
      cmdIpcSend(cm->compTable[cmIpcHdr->dstID].sockID, msgBuf);
    }
		cmdCmshBaseFree(vCmsh);
  }
  return CMD_IPC_OK;
}

Int32T
cmdIpcServNodeChange(cmMgrT * cm, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
  cmdIpcMsg_T *tmpHdr = (cmdIpcMsg_T *)(data);
  if(cm->compTable[cmIpcHdr->dstID].sockID > 0)
  {
    tmpHdr->requestKey = fd;
    cmdIpcSend(cm->compTable[cmIpcHdr->dstID].sockID, data);
  }
  else
  {
    Int8T szBuffer[1024] = "Component is not running...\n";
    tmpHdr->mode = CMD_IPC_CODE_CHANGE_NODE_RES_NOT_OK;
    cmdIpcAddTlv(data, CMD_IPC_OUTPUT_DATA, strlen(szBuffer), szBuffer);
    cmdIpcSend(fd, data);
  }
  return CMD_IPC_OK;
}

Int32T
cmdIpcServFuncWeb(cmMgrT * cm, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
  cmdCmCmshSession_T *cmdss = NULL;
  Int32T i;
  for (i=0; i < CMD_MAX_SESSION; i++)
  {
      /** Note: if (userKey, fd) is same, we assume that they are same session. */
      if ((cm->cmshTable[i].userKey == cmIpcHdr->userKey)
          && (cm->cmshTable[i].fd == fd))
      {
          cmdss = &(cm->cmshTable[i]);
          break;
      }
  }
  if(cmdss != NULL)
  {
    /** TODO  : Fixed here  */
  }
  return CMD_IPC_OK;
}
Int32T
cmdIpcServFuncSsh(cmMgrT * cm, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
  cmdCmCmshSession_T *cmdss = NULL;
  Int32T i;
  for (i=0; i < CMD_MAX_SESSION; i++)
  {
      /** Note: if (userKey, fd) is same, we assume that they are same session. */
      if ((cm->cmshTable[i].userKey == cmIpcHdr->userKey)
          && (cm->cmshTable[i].fd == fd))
      {
          cmdss = &(cm->cmshTable[i]);
          break;
      }
  }
  if(cmdss != NULL)
  {
    /** TODO  : Fixed here  */
  }
  return CMD_IPC_OK;
}

Int32T
cmdIpcServAuthLogin(cmMgrT * cm, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
  char msgBuf[CMD_IPC_MAXLEN];
  char szUser[CMD_PASSWD_MAX];
  char szPass[CMD_PASSWD_MAX];
  memset(szUser, 0, CMD_PASSWD_MAX);
  memset(szPass, 0, CMD_PASSWD_MAX);
  struct cmUserManager_T *tmpUser;
  struct cmdListNode  *nn;
  int sizeUser, sizePass;
  cmdIpcUnpackTlv(data, CMD_IPC_TLV_USERID, &sizeUser, szUser);
  szUser[sizeUser] = '\0';
  cmdIpcUnpackTlv(data, CMD_IPC_TLV_PASSWD, &sizePass, szPass);
  szPass[sizePass] = '\0';
  cmIpcHdr->code = CMD_IPC_CODE_CLIENT_RES_LOGIN_NOT_OK;  /** Default is not success  */
  CMD_MANAGER_LIST_LOOP(gCmIpcBase->cmUserTable, tmpUser, nn)
  {
    if(strcmp(tmpUser->cmUserName, szUser) == 0)
    {
	 	if(strcmp("md5",tmpUser->cmUserEncryptMethod) == 0){
			Int8T* buffer = str2md5(szPass, strlen(szPass));
			if(strcmp(buffer, tmpUser->cmUserPassEncrypt) == 0){
				cmIpcHdr->code = CMD_IPC_CODE_CLIENT_RES_LOG_SUCCESS;
                break;
			}	
		}else if(strcmp("sha256",tmpUser->cmUserEncryptMethod) == 0){
			Int8T buffer[65];
			cmdSha256(szPass, buffer);
            if(strcmp(buffer, tmpUser->cmUserPassEncrypt) == 0){
                cmIpcHdr->code = CMD_IPC_CODE_CLIENT_RES_LOG_SUCCESS;
                break;
            }
		}else if(strcmp("sha512", tmpUser->cmUserEncryptMethod) == 0){
			Int8T buffer[129];
			cmdSha512(szPass, buffer);
            if(strcmp(buffer, tmpUser->cmUserPassEncrypt) == 0){
                cmIpcHdr->code = CMD_IPC_CODE_CLIENT_RES_LOG_SUCCESS;
                break;
            }
		}else{
			if(0 == strcmp(tmpUser->cmUserPassword, szPass))
			{
				/**  Send back to cmsh  */
				cmIpcHdr->code = CMD_IPC_CODE_CLIENT_RES_LOG_SUCCESS;
				break;
		  	}
	 	}
    }
  }
  cmIpcHdr->dstID = cmIpcHdr->srcID;
  cmIpcHdr->srcID = cmIpcHdr->dstID;
  cmdIpcAddHdr(msgBuf, cmIpcHdr);
  cmdIpcSend(fd, msgBuf);
  return CMD_IPC_OK;
}

Int32T
cmdOamSetStatus(cmMgrT * cm, Int32T fd, cmdIpcMsg_T *cmIpcHdr, Int32T status)
{
  cmdCmCmshSession_T *cmdss = NULL;
  Int32T i;
  struct cmdListNode  *nn;
  for (i=0; i < CMD_MAX_SESSION; i++)
  {
      /** Note: if (userKey, fd) is same, we assume that they are same session. */
      if ((cm->cmshTable[i].userKey == cmIpcHdr->userKey)
          && (cm->cmshTable[i].fd == fd))
      {
          cmdss = &(cm->cmshTable[i]);
          break;
      }
  }
  if(cmdss != NULL)
  {
    /** TODO  : Fixed here  */
    cmdss->isWaiting = status;
    if(status)
    {
      if(cmdss->notifyTable == NULL)
      {
        cmdss->notifyTable = cmdListNew();
      }
    }
    else
    {
      /** Send OAM to Clients */
      Int8T *value;
      if(cmdss->notifyTable != NULL)
      {
        CMD_MANAGER_LIST_LOOP(cmdss->notifyTable, value, nn)
        {
          cmdIpcSend(cmdss->fd, value);
        }
        cmdListDel(cmdss->notifyTable);
        cmdss->notifyTable = NULL;
      }
    }
  }
  return CMD_IPC_OK;
}


void
cmCmshProcess(Int32T sockId, void *message, Uint32T size)
{
  Int8T* szBuffer = (Int8T*)message;
  Int8T  szTmpBuffer[CMD_IPC_MAXLEN];
  cmdIpcMsg_T cmIpcHdr;
  Uint32T nMsgSize, nTotalBytes = 0;

  while(nTotalBytes < size)
  {
    cmdIpcUnpackHdr(szBuffer, &cmIpcHdr);
    nMsgSize = cmIpcHdr.length + CMD_IPC_HDR_LEN;
    bzero(&szTmpBuffer, CMD_IPC_MAXLEN);
    memcpy(&szTmpBuffer, szBuffer, nMsgSize);
    switch (cmIpcHdr.code)
    {
      case CMD_IPC_CODE_CLIENT_REG_REQ:
			{
        cmdIpcServRegister(gCmIpcBase, sockId, szTmpBuffer, &cmIpcHdr);       /**  FIXME:  */
			}
      break;
      case CMD_IPC_CODE_CLIENT_DEREG_REQ:
			{
        cmdIpcServDeregister(gCmIpcBase, sockId, szTmpBuffer, &cmIpcHdr);     /**  FIXME:  */
			}
      break;
      case CMD_IPC_CODE_AUTH_ENABLE_REQ:
			{
        cmdOamSetStatus(gCmIpcBase, sockId, &cmIpcHdr, 1);
        cmdIpcServAuthEnable(gCmIpcBase, sockId, szTmpBuffer, &cmIpcHdr);     /**  FIXME:  */
			}
      break;
      case CMD_IPC_CODE_AUTH_ENABLE_REQ_PASSWD:
			{
        cmdOamSetStatus(gCmIpcBase, sockId, &cmIpcHdr, 1);
        cmdIpcServAuthEnablePasswd(gCmIpcBase, sockId, szTmpBuffer, &cmIpcHdr);   /**  FIXME:  */
			}
      break;
      case CMD_IPC_CODE_AUTH_CONFIGURE_REQ:
			{
        cmdOamSetStatus(gCmIpcBase, sockId, &cmIpcHdr, 1);
        cmdIpcServAuthConfig(gCmIpcBase, sockId, szTmpBuffer, &cmIpcHdr);     /**  FIXME:  */
			}
      break;
      case CMD_IPC_CODE_AUTH_CONFIGURE_REQ_PASSWD:
			{
        cmdOamSetStatus(gCmIpcBase, sockId, &cmIpcHdr, 1);
        cmdIpcServAuthConfigPasswd(gCmIpcBase, sockId, szTmpBuffer, &cmIpcHdr);   /**  FIXME:  */
			}
      break;
      case CMD_IPC_CODE_UPDATE_NODE:
			{
        cmdIpcServUpdateMode(gCmIpcBase, sockId, szTmpBuffer, &cmIpcHdr);
        cmdOamSetStatus(gCmIpcBase, sockId, &cmIpcHdr, 0);
			}
      break;
			case CMD_IPC_CODE_FUNC_EXIT:
			{ 
				cmdIpcServAuthExit(gCmIpcBase, sockId, szTmpBuffer, &cmIpcHdr);
			}
			break;
      case CMD_IPC_CODE_FUNC_REQ:
      case CMD_IPC_CODE_FUNC_RES_PASSWD:
			{
        if (cmIpcHdr.clientType == CMD_IPC_TYPE_CLI) {
          cmdIpcServFuncCli(gCmIpcBase, sockId, szTmpBuffer, &cmIpcHdr);      /**  FIXME:  */
        }
        if (cmIpcHdr.clientType == CMD_IPC_TYPE_WEB) {
          cmdIpcServFuncWeb(gCmIpcBase, sockId, szTmpBuffer, &cmIpcHdr);      /**  FIXME:  */
        }
        if (cmIpcHdr.clientType == CMD_IPC_TYPE_SSH) {
          cmdIpcServFuncSsh(gCmIpcBase, sockId, szTmpBuffer, &cmIpcHdr);      /**  FIXME:  */
        }
			}
      break;
      case CMD_IPC_CODE_CLIENT_REG_LOGIN:
          cmdIpcServAuthLogin(gCmIpcBase, sockId, szTmpBuffer, &cmIpcHdr); 
      break;
      case CMD_IPC_CODE_CHANGE_NODE_REQ:
      {
        cmdIpcServNodeChange(gCmIpcBase, sockId, szTmpBuffer, &cmIpcHdr);      /**  FIXME:  */
      }
      break;
      default:
			{
        fprintf(stderr,"[%s][%s][%d] <Unkown code><%d>\n", __FILE__, __func__, __LINE__, cmIpcHdr.code);
			}
      break;
    }
    nTotalBytes += nMsgSize;
    szBuffer += nMsgSize;
  }
}

Int32T
cmdIpcCompServReg(cmMgrT * cm, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
  Int8T msgBuf[CMD_IPC_MAXLEN];

  cm->compTable[cmIpcHdr->srcID].compID = cmIpcHdr->srcID;
  cm->compTable[cmIpcHdr->srcID].sockID = fd;
  cm->compTable[cmIpcHdr->srcID].flagID = 0;
  /** Send back */
  bzero(&msgBuf, CMD_IPC_MAXLEN);
  cmIpcHdr->code = CMD_IPC_CODE_CLIENT_REG_RES_OK;
  cmIpcHdr->dstID = cmIpcHdr->srcID;     /** IPC_CM_MGRSH  */
  cmIpcHdr->srcID = IPC_CM_MGR;
  cmdIpcAddHdr(msgBuf, cmIpcHdr);
  cmdIpcSend(fd, msgBuf);
  return CMD_IPC_OK;
}

Int32T
cmdIpcCompServDereg(cmMgrT * cm, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
  Int8T msgBuf[CMD_IPC_MAXLEN];
  cm->compTable[cmIpcHdr->srcID].compID = 0;
  cm->compTable[cmIpcHdr->srcID].sockID = 0;
  cm->compTable[cmIpcHdr->srcID].flagID = 0;
  /** Send back */
  bzero(&msgBuf, CMD_IPC_MAXLEN);
  cmIpcHdr->code = CMD_IPC_CODE_CLIENT_DEREG_RES_OK;
  cmIpcHdr->dstID = cmIpcHdr->srcID;     /** IPC_CM_MGRSH  */
  cmIpcHdr->srcID = IPC_CM_MGR;
  cmdIpcAddHdr(msgBuf, cmIpcHdr);
  cmdIpcSend(fd, msgBuf);
  return CMD_IPC_OK;
}


void
cmdRunConfigAddCommandLine(cmMgrT * cm, Int32T compId, Int8T *strCmd)
{
  struct cmRunConfigManager_T *node;
  struct cmdListNode *nn;
  CMD_MANAGER_LIST_LOOP(cm->cmRunConfigTableMul, node, nn)
  {
    if(node->compId == compId)
    {
      Int8T *cmdStr = strdup(strCmd);
      cmdListAddNode(node->cmdStrTable, cmdStr);
    }
  }
}

void
cmdSortAddCommandLine(struct cmConfigManager_T *currNode, Int8T *cmdLine)
{
	struct cmdListNode *nn;
	Int8T *currCmdLine;
	Int8T *cmdTemp = strdup(cmdLine);
	CMD_MANAGER_LIST_LOOP(currNode->cmdStrTable, currCmdLine, nn)
	{
		if(!strcmp(currCmdLine, cmdLine))
		{
			return;
		}
	}
	cmdListAddNode(currNode->cmdStrTable, cmdTemp);
}

void
cmdSortAddCommandLineMul(struct cmConfigManagerMulNode_T *currNode, Int8T *cmdStr, Int8T *cmdLine)
{
  struct cmdListNode *nn;
  Int8T *currCmdLine;
  CMD_MANAGER_LIST_LOOP(currNode->cmdStrTable, currCmdLine, nn)
  {
    while(isspace ((Int32T) *currCmdLine) && *currCmdLine != '\0')
    {
        currCmdLine++;
    }
    if(!strcmp(currCmdLine, cmdLine))
    {
      return;
    }
  }
  cmdListAddNode(currNode->cmdStrTable, cmdStr);
}



struct cmConfigManagerMulNode_T *
cmConfigFindNodeMul(cmMgrT *cm, Int32T stUpState, Int8T* nodeShow, Int8T *nodeCurr, Int32T cDepth)
{
  struct cmConfigManagerMulNode_T *node;
  struct cmdListNode *nn;
  struct cmdList *currList;
  if(stUpState)
  {
    currList = cm->cmConfigTableMul;
  }
  else
  {
    currList = cm->cmCurrConfigTableMul;
  }

  CMD_MANAGER_LIST_LOOP(currList, node, nn)
  {
   	if(strcmp(node->nodeStr, nodeCurr) == 0  && cDepth ==node->idDepth){
		return node;
	} 
  }
  return NULL;
}

struct cmConfigManagerMulNode_T *
cmConfigFindNodeChildMul(struct cmConfigManagerMulNode_T *nodeCurent, Int8T *nodePre[MAX_USER_SIZE], Int8T* nodeCurr, Int8T *nodeShow, Int32T cDepth)
{
  struct cmConfigManagerMulNode_T *node;
  struct cmdListNode *nn;
	Int32T i, nCheck;
  CMD_MANAGER_LIST_LOOP(nodeCurent->cmdStrTable, node, nn)
  {
    if(strcmp(node->nodeStr, nodeCurr) == 0  && cDepth ==node->idDepth){
		if(node->idDepth >= cDepth){
			nCheck = 0;
			for(i = 0; i <= cDepth - CMD_NODE_CONFIG; i ++){
				if(strcmp(node->nodePrevious[i], nodePre[i]) == 0){
					nCheck ++;
				}
		  	}
			if(nCheck == cDepth - CMD_NODE_CONFIG + 1){
        		return node;
      		}
		}
    }
	cmConfigFindNodeChildMul(node, nodePre, nodeCurr, nodeShow, cDepth);
  }
	
  return NULL;
}

struct cmConfigManagerMulNode_T *
cmConfigCheckNodeMul_tmp(cmMgrT *cm, Int8T *nodePre[CMD_IPC_MAXLEN], Int8T* nodeCurr, Int8T *nodeShow, Int32T cDepth)
{
  	struct cmConfigManagerMulNode_T *node, *currentNode = NULL;
  	struct cmdListNode *nn;
    CMD_MANAGER_LIST_LOOP(cm->cmCurrConfigTableMul, node, nn)
    {
        currentNode = cmConfigFindNodeChildMul(node, nodePre, nodeCurr, nodeShow, cDepth);
		if(currentNode != NULL)
			return currentNode;
    }
  	return NULL;
}

struct cmConfigManagerMulNode_T *
cmConfigCheckNodeMul(struct cmConfigManagerMulNode_T *nodeCurent, Int8T *nodePre[MAX_USER_SIZE], Int8T* nodeCurr, Int8T *nodeShow, Int32T cDepth)
{
  struct cmConfigManagerMulNode_T *node;
  struct cmdListNode *nn;
    Int32T i, nCheck;
  CMD_MANAGER_LIST_LOOP(nodeCurent->cmdStrTable, node, nn)
  {
    if(strcmp(node->nodeStr, nodeCurr) == 0  && cDepth ==node->idDepth){
        if(node->idDepth >= cDepth){
            nCheck = 0;
            for(i = 0; i <= cDepth - CMD_NODE_CONFIG; i ++){
                if(strcmp(node->nodePrevious[i], nodePre[i]) == 0){
                    nCheck ++;
                }
            }
            if(nCheck == cDepth - CMD_NODE_CONFIG + 1){
                return node;
            }
        }
    }
  }
  return NULL;
}


struct cmConfigManagerMulNode_T *
cmConfigFindNodeParentMul(struct cmConfigManagerMulNode_T *nodeCurent, Int8T *nodePre[MAX_USER_SIZE], Int32T cDepth)
{
  struct cmConfigManagerMulNode_T *node;
  struct cmdListNode *nn;
    Int32T i, nCheck;
  CMD_MANAGER_LIST_LOOP(nodeCurent->cmdStrTable, node, nn)
  {
    if(cDepth ==node->idDepth){
		nCheck = 0;
		for(i = 0; i <= cDepth - CMD_NODE_CONFIG; i ++){
			if(strcmp(node->nodePrevious[i], nodePre[i]) == 0){
				nCheck ++;
			}
		}
		if(nCheck == cDepth - CMD_NODE_CONFIG + 1){
			return node;
		}
    }
    cmConfigFindNodeParentMul(node, nodePre, cDepth);
  }

  return NULL;
}


struct cmConfigManagerMulNode_T *
cmConfigCheckNodeParentMul(cmMgrT *cm, Int8T *nodePre[CMD_IPC_MAXLEN], Int32T cDepth)
{
    struct cmConfigManagerMulNode_T *node, *currentNode = NULL;
    struct cmdListNode *nn;
    CMD_MANAGER_LIST_LOOP(cm->cmCurrConfigTableMul, node, nn)
    {
        currentNode = cmConfigFindNodeParentMul(node, nodePre, cDepth);
        if(currentNode != NULL)
            return currentNode;
    }
    return NULL;
}

struct cmConfigManagerMulNode_T*
cmdInitNodeRunnignConfig(struct cmdList *cmdTable, Int8T *cfgPtr, Int8T *cmdStr, Int32T cDepth, Int8T *nodePre[MAX_USER_SIZE], Int32T cFlagh){
    struct cmConfigManagerMulNode_T *node;
    Int32T i;
    node = (struct cmConfigManagerMulNode_T *)malloc(sizeof(*node));
    node->nodeShow = strdup(cmdStr);
    node->nodeStr = strdup(cfgPtr);
    node->idDepth = cDepth;
	node->cFlagh = 1;
    node->cFlagh = cFlagh;
    for(i = 0; i <= cDepth - CMD_NODE_CONFIG; i++){
        node->nodePrevious[i] = strdup(nodePre[i]);
    }
    node->cmdStrTable = cmdListNew();
    cmdListAddNode(cmdTable, node);
    return node;
}

void
cmdSortRunningTableMul(cmMgrT * cm)
{
    struct cmRunConfigManager_T *node;
    struct cmdListNode *nn, *cnn;
    Int8T *cfgPtr, *cmdStr, *nodePre[CMD_IPC_MAXLEN];;
	Int32T cmdType = 0, nodeCheck;
    struct cmConfigManagerMulNode_T *nodeConfig = NULL, *preNode = NULL, *bigNode, *preBigNode, *smallNode;
    cm->cmCurrConfigTableMul = cmdListNew();   /** Init cmCurrConfigTable  */
	nodePre[0] = "configure";
	if(nodeConfig == NULL)
	{
		nodeConfig = (struct cmConfigManagerMulNode_T *)malloc(sizeof(*nodeConfig));
		nodeConfig->nodeStr = strdup("!");
		nodeConfig->nodeShow = strdup("!");
		nodeConfig->cmdStrTable = cmdListNew();
		nodeConfig->idDepth = 0;
		nodeConfig->nodePrevious[0] = nodePre[0];
		cmdListAddNode(cm->cmCurrConfigTableMul, nodeConfig);
	}
	preBigNode = nodeConfig;
	Int32T cDepth = 0;;
    CMD_MANAGER_LIST_LOOP(cm->cmRunConfigTableMul, node, nn)
    {
		nodeCheck = 0;
		cDepth = CMD_NODE_CONFIG; 
        CMD_MANAGER_LIST_LOOP(node->cmdStrTable, cmdStr, cnn)
        {
            cfgPtr = cmdCmdParseNodeMul(cm->cmsh, cmdStr, &cmdType, &nodeCheck, &cDepth);
			if(cmdType == CMD_RUNNING_EXIT_AUTO){
                preNode = cmConfigCheckNodeParentMul(cm, nodePre, cDepth);
                if(cDepth == CMD_NODE_CONFIG + 1)
                    preNode=preBigNode;
                if(cDepth == CMD_NODE_CONFIG)
                    preNode = nodeConfig;
				cmdType = CMD_RUNNING_COMMAND_NODE;
            }
            if((cmdType == CMD_RUNNING_COMMAND_NODE))
			{
				nodePre[cDepth-CMD_NODE_CONFIG] = cfgPtr;
				bigNode = cmConfigFindNodeMul(cm, 0, cmdStr, cfgPtr, cDepth);
				if(nodeCheck == 0){
					nodeCheck = 1;
					if(bigNode == NULL){
						bigNode = cmdInitNodeRunnignConfig(cm->cmCurrConfigTableMul, cfgPtr, cmdStr, cDepth, nodePre, 0);
						preNode = bigNode;
						preBigNode = bigNode;
 					}else{
						preNode= bigNode;
						preBigNode = bigNode;
					}
				}else{
					/** Find node small continues*/
					nodeCheck ++;
					smallNode = cmConfigCheckNodeMul(preBigNode, nodePre, cfgPtr, cmdStr, cDepth);	
					if(smallNode == NULL){
						smallNode = cmdInitNodeRunnignConfig(preNode->cmdStrTable, cfgPtr, cmdStr, cDepth, nodePre, 0);
						preNode = smallNode;	
					}else{
						preNode = smallNode;	
						cDepth = smallNode->idDepth;						
					}
				}	
			}
			if(cmdType == CMD_RUNNING_CONFIG_NODE){
				preNode = nodeConfig;
			}
			if(cmdType == CMD_RUNNING_COMMAND_LINE)
			{
				if(preNode != NULL){
					if(cmConfigCheckNodeMul(preBigNode, nodePre, cfgPtr, cmdStr, cDepth) == NULL){
						cmdInitNodeRunnignConfig(preNode->cmdStrTable, cfgPtr, cmdStr, cDepth, nodePre, 1);
					}
				}
			}
			if(cmdType == CMD_RUNNING_DEFAULT){
				fprintf(stderr,"[%s][%s][%d] <Unkown command> <%s>\n", __FILE__, __func__, __LINE__, cmdStr);
			}
		}
		if(node->cmdStrTable != NULL)
            cmdListDel(node->cmdStrTable);
    }
  cmdListDel(cm->cmRunConfigTableMul);
}

Int32T
cmdIpcCompFuncRes(cmMgrT * cm, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
  cmdIpcMsg_T *tmpHdr = (cmdIpcMsg_T *)(data);
  if(cmIpcHdr->dstID != IPC_CM_MGR)
  {
    tmpHdr->srcID = IPC_CM_MGR;
    tmpHdr->dstID = IPC_CMSH_MGR;
    cmdIpcSend(cmIpcHdr->requestKey, data);
  }
  else  /** Configure Write */
  {
    Int8T szBufferTlv[CMD_IPC_MAXLEN];
    Int32T tlvLengthType, tlvLengthData, outTypeTlv;
    bzero(&szBufferTlv, CMD_IPC_MAXLEN);
    cmdIpcUnpackTlv(data, CMD_IPC_TLV_OUTPUT_TYPE, &tlvLengthType, &outTypeTlv);
    cmdIpcUnpackTlv(data, CMD_IPC_TLV_OUTPUT_DATA, &tlvLengthData, szBufferTlv);
    if(cm->compTable[cmIpcHdr->srcID].flagID == 0)
    {
      switch(outTypeTlv) 
      {
        case CMD_OUTMOD_INCLUDE:
        case CMD_OUTMOD_BEGIN:
        {
          cmdRunConfigAddCommandLine(cm, tmpHdr->srcID, szBufferTlv);
        }
        break;
        case CMD_OUTMOD_EXCLUDE:
        {
          cm->cmConfigNumComp--;  /** Decrease number comp  */
        }
        break;
      }
      /**	Sort Config Here 	*/
      if(cm->cmConfigNumComp == 0)
      {

        cmdSortRunningTableMul(cm);
      }
      /**	End	*/
    }
    else
    {
      if(outTypeTlv == CMD_OUTMOD_EXCLUDE)
      {
        cm->compTable[cmIpcHdr->srcID].flagID--;
      }
    }
  }
  return CMD_IPC_OK;
}
Int32T
cmdIpcCompFuncChangeNodeRes(cmMgrT * cm, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
  Int32T sockId = cmIpcHdr->requestKey;
  cmdIpcSend(sockId, data);
  return CMD_IPC_OK;
}
Int32T
cmdIpcOamProcess(cmMgrT * cm, Int32T fd, void *data, cmdIpcMsg_T *cmIpcHdr)
{
  cmdCmCmshSession_T *cmdss = NULL;
  Int32T i;
  cmdIpcMsg_T *tmpHdr = (cmdIpcMsg_T *)(data);
  tmpHdr->srcID = IPC_CM_MGR;
  tmpHdr->dstID = IPC_CMSH_MGR;
  for (i=0; i < CMD_MAX_SESSION; i++)
  {
    if(cm->cmshTable[i].fd > 0 && cm->cmshTable[i].userKey)
    { 
      cmdss = &(cm->cmshTable[i]);
      if(cm->cmshTable[i].isWaiting == 0)
      {
        /** Send  */
        cmdIpcSend(cmdss->fd, data);
      }
      else
      {
        /** Add to notifyTable  */
        Int8T *szBufferTlv = malloc(CMD_IPC_MAXLEN *sizeof(Int8T));
        Int32T tlvLengthData = tmpHdr->length + CMD_IPC_HDR_LEN;
        memcpy(szBufferTlv, data, tlvLengthData);
        cmdListAddNode(cmdss->notifyTable, szBufferTlv);
      }
    }
  }
  return CMD_IPC_OK;
}

void
cmCompProcess(Int32T sockId, void *message, Uint32T size)
{
  Int8T* szBuffer = (Int8T *)message;
  Int8T  szTmpBuffer[CMD_IPC_MAXLEN];
  cmdIpcMsg_T cmIpcHdr;
  Uint32T nMsgSize, nTotalBytes = 0;
  while(nTotalBytes < size)
  {
    cmdIpcUnpackHdr(szBuffer, &cmIpcHdr);
    nMsgSize = cmIpcHdr.length + CMD_IPC_HDR_LEN;
    bzero(&szTmpBuffer, CMD_IPC_MAXLEN);
    memcpy(&szTmpBuffer, szBuffer, nMsgSize);
    switch (cmIpcHdr.code)
    {
      case CMD_IPC_CODE_CLIENT_REG_REQ:
        /** Register Components     */
        cmdIpcCompServReg(gCmIpcBase, sockId, szTmpBuffer, &cmIpcHdr);
      break;
      case CMD_IPC_CODE_CLIENT_DEREG_REQ:
        /** DeRegister Components   */
        cmdIpcCompServDereg(gCmIpcBase, sockId, szTmpBuffer, &cmIpcHdr);
      break;
      case CMD_IPC_OAM_DATA:
        cmdIpcOamProcess(gCmIpcBase, sockId, szTmpBuffer, &cmIpcHdr);
      break;
      case CMD_IPC_OUTPUT_DATA: /** CMD_OUTMOD_XXX  */
        /** Send back when set from cmsh  */
        cmdIpcCompFuncRes(gCmIpcBase, sockId, szTmpBuffer, &cmIpcHdr);
      break;
      case CMD_IPC_CODE_STARTUP_CONFIG:
        nosCmdConfigParseCommand(gCmIpcBase, sockId, szTmpBuffer, &cmIpcHdr); 
      break;
      case CMD_IPC_CODE_CHANGE_NODE_RES_OK:
      case CMD_IPC_CODE_CHANGE_NODE_RES_NOT_OK:
        /** Send Back to CMSH */
        cmdIpcCompFuncChangeNodeRes(gCmIpcBase, sockId, szTmpBuffer, &cmIpcHdr);
      break; 
      default:
        fprintf(stderr,"[%s][%s][%d] <Unkown code><%d>\n", __FILE__, __func__, __LINE__, cmIpcHdr.code);
      break;
    }
    nTotalBytes += nMsgSize;
    szBuffer += nMsgSize;
  }
}
