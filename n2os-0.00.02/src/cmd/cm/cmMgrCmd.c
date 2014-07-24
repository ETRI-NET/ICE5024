/**
* @file        :  cmMgrCmd.c
* @brief       :  Process node Service
*
* $Id: cmdService.c 730 2014-01-22 09:00:39Z hai $
* $Author: thanh $
* $Date: 2014-01-22 22:57:39 -0500 (Wen, 22 Jan 2014) $
* $Log$
* $Revision: 730 $
* $LastChangedBy: Hai $
* $LastChanged$
*
*                       Electronics and Telecommunications Research Institute
*               Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.
*                          No part of this software shall be reproduced, stored in a retrieval system, or
*                           transmitted by any means, electronic, mechanical, photocopying, recording,
*                            or otherwise, without written permission from ETRI.
*                  **/

/********************************************************************************
 **                                  INCLUDE FILES
 ********************************************************************************/
#include <stdio.h>      /* printf, fgets */
#include <stdlib.h>     /* atoi */
#include <string.h>
#include "nnTypes.h"
#include "nnStr.h"

#include "nnCmdLink.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"
#include "nnCmdInstall.h"
#include "nosLib.h"
#include "nnCmdDefines.h"
#include "nnGlobalCmd.h"
#include "cmMgrCmIpc.h"

Int8T gBannerTxt[1024] = " NOS Version 1.0.0.0"; // "Hello, This is NOS (version 1.0.0.0)\n Copyright Netvision Telecom Inc\n";

void
cmdInsertCommandService(Int8T* strInput, Int32T flags)
{
	struct cmServiceManagerStartup_T *tmpService;
    struct cmdListNode *nn;
	CMD_MANAGER_LIST_LOOP(gCmIpcBase->cmServiceStartup, tmpService, nn)
    {
		if(tmpService->flags == flags){
			if(tmpService->strService != NULL)
				free(tmpService->strService);
			tmpService->strService = strdup(strInput);
			return;
		}
    }
	tmpService = (struct cmServiceManagerStartup_T *)malloc(sizeof(*tmpService));
	tmpService->flags = flags;
	tmpService->strService = strdup(strInput);
	cmdListAddNode(gCmIpcBase->cmServiceStartup, tmpService);
}
DECMD(cmdFuncServiceNodeTelnet,
    CMD_NODE_CONFIG_SERVICE,
    IPC_CM_MGR,
    "telnet service",
    "telnet configure",
    "service configure")
{
  cmdCmdServiceRestart(gCmIpcBase);
  gCmIpcBase->cmService.pTelnetPortId = NOS_TELNET_DEFAULT_PORT_ID;
  if(cmdCmdServiceStart(gCmIpcBase) == CMD_IPC_OK)
  {
	cmdInsertCommandService("telnet service", SERVICE_TELNET_FLAG);	
    cmdPrint(cmsh,"Is Success...");
  }
  else
  {
    cmdPrint(cmsh,"Not Sucecss...\n");
  }
  return (CMD_IPC_OK);
}

DECMD(cmdFuncServiceNodeNoTelnet,
    CMD_NODE_CONFIG_SERVICE,
    IPC_CM_MGR,
    "no telnet service",
    "Negate a command or set its defaults",
    "telnet configure",
    "service configure")
{

  cmdCmdServiceRestart(gCmIpcBase);
  gCmIpcBase->cmService.pTelnetPortId = 0;
  if(cmdCmdServiceStart(gCmIpcBase) == CMD_IPC_OK)
  {
	cmdInsertCommandService("no telnet service", SERVICE_TELNET_FLAG);
    cmdPrint(cmsh,"Is Success...");
  }
  else
  {
    cmdPrint(cmsh,"Not Sucecss...\n");
  }
  return (CMD_IPC_OK);
}

DECMD(cmdFuncServiceNodeTelnetPort,
    CMD_NODE_CONFIG_SERVICE,
    IPC_CM_MGR,
    "telnet service port WORD",
    "Telnet configure",
    "Service configure",
    "Port configure",
    "Number PortID")
{
  cmdCmdServiceRestart(gCmIpcBase);
  gCmIpcBase->cmService.pTelnetPortId = atoi(cargv[cargc-1]);
  if(cmdCmdServiceStart(gCmIpcBase) == CMD_IPC_OK)
  {
	Int8T tmp[255];
	sprintf(tmp, "telnet service port %s", cargv[cargc-1]);
	cmdInsertCommandService(tmp, SERVICE_TELNET_FLAG);
    cmdPrint(cmsh,"Is Success...");
  }
  else
  {
    cmdPrint(cmsh,"Not Sucecss...\n");
  }
  return (CMD_IPC_OK);
}

DECMD(cmdFuncServiceNodeSsh,
    CMD_NODE_CONFIG_SERVICE,
    IPC_CM_MGR,
    "ssh service",
    "ssh configure",
    "service configure")
{
  cmdCmdServiceRestart(gCmIpcBase);
  gCmIpcBase->cmService.pSshPortId = NOS_SSH_DEFAULT_PORT_ID;
  if(cmdCmdServiceStart(gCmIpcBase) == CMD_IPC_OK)
  {
	cmdInsertCommandService("ssh service", SERVICE_SSH_FLAG);
    cmdPrint(cmsh,"Is Success...");
  }
  else
  {
    cmdPrint(cmsh,"Not Sucecss...\n");
  }
  return (CMD_IPC_OK);
}

DECMD(cmdFuncServiceNodeNoSsh,
    CMD_NODE_CONFIG_SERVICE,
    IPC_CM_MGR,
    "no ssh service",
    "Negate a command or set its defaults",
    "ssh configure",
    "service configure")
{
  cmdCmdServiceRestart(gCmIpcBase);
  gCmIpcBase->cmService.pSshPortId = 0;
  if(cmdCmdServiceStart(gCmIpcBase) == CMD_IPC_OK)
  {
	cmdInsertCommandService("no ssh service", SERVICE_SSH_FLAG);
    cmdPrint(cmsh,"Is Success...");
  }
  else
  {
    cmdPrint(cmsh,"Not Sucecss...\n");
  }
  return (CMD_IPC_OK);
}

DECMD(cmdFuncServiceNodeSshPort,
    CMD_NODE_CONFIG_SERVICE,
    IPC_CM_MGR,
    "ssh service port WORD",
    "Ssh configure",
    "Service configure",
    "Port configure",
    "Number PortID")
{
  cmdCmdServiceRestart(gCmIpcBase);
  gCmIpcBase->cmService.pSshPortId = atoi(cargv[cargc-1]);
  if(cmdCmdServiceStart(gCmIpcBase) == CMD_IPC_OK)
  {
	Int8T tmp[255];
	sprintf(tmp,"ssh service port %s", cargv[cargc-1]);
	cmdInsertCommandService(tmp, SERVICE_SSH_FLAG);
    cmdPrint(cmsh,"Is Success...");
  }
  else
  {
    cmdPrint(cmsh,"Not Sucecss...\n");
  }
  return (CMD_IPC_OK);
}

DECMD(cmdFuncGlobalSetBanner,
  CMD_NODE_CONFIG,
	IPC_CM_MGR,
  "banner login LINE",
  "Define a login banner",
  "Set login banner",
  "Describe a login banner")
{
	cmdPrint(cmsh,"Set Bannder(%s)", cargv[2]);
  sprintf(gBannerTxt, "%s", cargv[2]);
  cmdPrint(cmsh,"is success...");
  return (CMD_IPC_OK);
}
DECMD(cmdFuncGlobalGetBanner,
  CMD_NODE_VIEW,
	IPC_CM_MGR|IPC_SHOW_MGR,
  "show banner",
  "Show Banner System",
  "Show Banner System")
{
	time_t rTime;
	struct tm * timeinfo;
	time (&rTime);
	timeinfo = localtime(&rTime);
//	cmdPrint(cmsh,"  NOS Version 1.0.0.0 %s", asctime (timeinfo));
	cmdPrint(cmsh,"  %s %s", gBannerTxt, asctime (timeinfo));
	cmdPrint(cmsh,"    N2OS Consortium\n");
	return (CMD_IPC_OK);
}

DECMD(cmdFuncUserNodeUserName,
    CMD_NODE_CONFIG_USER,
    IPC_CM_MGR,
    "username WORD access-level (view|enable|config)",
    "Establish User Name Authentication",
    "user name",
    "Config the access lever for user",
    "VIEW mode",
    "PRIVIGERE mode ",
    "CONFIGURE mode")
{
  if(cmsh->cmshFuncIndex == 0)
  {
    return CMD_IPC_PASS_WAIT;
  }
  else
  {
    /** Set Password for user here  */
   if(cmdCmdUserAdd(cmsh, cargv[1], cargv[3], cargv[4]) == CMD_IPC_OK)
   {
     cmdPrint(cmsh,"is Success...");
   }
   else
   {
   	 cmdPrint(cmsh,"user is exist, encrypt method set to none");
   }
    return (CMD_IPC_OK);
  }
}

DECMD(cmdFuncUserNodeNoUserName,
    CMD_NODE_CONFIG_USER,
    IPC_CM_MGR,
    "no username WORD",
    "Negate a command or set its defaults",
    "Establish User Name Authentication",
    "User name")
{
 if(cmdCmdUserDel(cargv[2]) == CMD_IPC_OK)
 {
   cmdPrint(cmsh,"is Success...");
 }
 else
 {
   cmdPrint(cmsh,"is Error...");
 }
  return (CMD_IPC_OK);
}
DECMD(cmdFuncUserNodeShowUserName,
    CMD_NODE_CONFIG_USER,
    IPC_CM_MGR|IPC_SHOW_MGR,
    "show username",
    "Show running system information",
    "Show list of user created")
{
  struct cmUserManager_T *tmpUser;
  struct cmdListNode  *nn;
  cmdPrint(cmsh, "%2s %10s  %15s  %10s\n", "ID", "User-ID", "User-Mode", "Encrypt-Method");
  cmdPrint(cmsh, "----------------------------------------------\n");
  Int32T i = 0;
  CMD_MANAGER_LIST_LOOP(gCmIpcBase->cmUserTable, tmpUser, nn)
  {
   i++;
   cmdPrint(cmsh,"%2d %10s %15s %10s\n", i, tmpUser->cmUserName, cmdCmdUserGetMode(tmpUser->cmUserMode), tmpUser->cmUserEncryptMethod);
  }  
  return (CMD_IPC_OK);
}

DECMD(cmdFuncUserNodeReconfigurePassword,
    CMD_NODE_CONFIG_USER,
    IPC_CM_MGR,
    "password WORD newpassword",
    "Change password for user",
    "User name",
	"Change new password")
{
  struct cmUserManager_T *tmpUser;
  struct cmdListNode  *nn;
   if(cmsh->cmshFuncIndex == 0)
  {
    return CMD_IPC_PASS_WAIT;
  }else{
	  CMD_MANAGER_LIST_LOOP(gCmIpcBase->cmUserTable, tmpUser, nn)
	  {
		if(strcmp(tmpUser->cmUserName, cargv[1]) == 0){
			sprintf(tmpUser->cmUserPassword,"%s", cargv[3]);
			if(strcmp("md5", tmpUser->cmUserEncryptMethod) == 0){
				Int8T* buffer = str2md5(tmpUser->cmUserPassword, strlen(tmpUser->cmUserPassword));
				sprintf(tmpUser->cmUserPassEncrypt, "%s", buffer);
			}else if(strcmp("sha256", tmpUser->cmUserEncryptMethod) == 0){
				Int8T buffer[65];
				cmdSha256(tmpUser->cmUserPassword, buffer);
				sprintf(tmpUser->cmUserPassEncrypt, "%s", buffer);
			}else if(strcmp("sha512", tmpUser->cmUserEncryptMethod) == 0){
				Int8T buffer[129];
				cmdSha512(tmpUser->cmUserPassword, buffer);
				sprintf(tmpUser->cmUserPassEncrypt, "%s", buffer);
			}
		    cmdUserWriteStore();
			cmdPrint(cmsh,"\nis Success...\n");
			return (CMD_IPC_OK);
		}
	  }
  }
  cmdPrint(cmsh,"\nUser not exists\n");
  return (CMD_IPC_OK);
}

DECMD(cmdFuncUserNodeEncryptPassword,
    CMD_NODE_CONFIG_USER,
    IPC_CM_MGR,
    "password WORD encrypt (md5|sha256|sha512)",
    "Encrypt password for user",
    "User name",
	"Encrypt method",
    "MD5 Encrypt method",
    "SHA256 Encrypt method",
    "SHA512 Encrypt method")
{
  struct cmUserManager_T *tmpUser;
  struct cmdListNode  *nn;
  CMD_MANAGER_LIST_LOOP(gCmIpcBase->cmUserTable, tmpUser, nn)
  {
    if(strcmp(tmpUser->cmUserName, cargv[1]) == 0){
        if(strcmp("md5", cargv[3]) == 0){
			Int8T* buffer = str2md5(tmpUser->cmUserPassword, strlen(tmpUser->cmUserPassword));
			sprintf(tmpUser->cmUserEncryptMethod,"%s", cargv[3]);
			sprintf(tmpUser->cmUserPassEncrypt, "%s", buffer);	
		}else if(strcmp("sha256", cargv[3]) == 0){
			Int8T buffer[65];
			sprintf(tmpUser->cmUserEncryptMethod,"%s", cargv[3]);
			cmdSha256(tmpUser->cmUserPassword, buffer);
			sprintf(tmpUser->cmUserPassEncrypt, "%s", buffer);	
		}else{
			Int8T buffer[129];
			sprintf(tmpUser->cmUserEncryptMethod,"%s", cargv[3]);
			cmdSha512(tmpUser->cmUserPassword, buffer);
			sprintf(tmpUser->cmUserPassEncrypt, "%s", buffer);	
		}
      cmdPrint(cmsh,"\nis Success...\n");
	  cmdUserWriteStore();
      return (CMD_IPC_OK);
    }
  }
  cmdPrint(cmsh,"\nUser not exists\n");
  return (CMD_IPC_OK);
}

Int32T 
cmdFunEnterRunningConfigSend(struct cmsh *cmsh, cmdCmCompSession_T* comSend)
{
  Int8T msgBuf[CMD_IPC_MAXLEN];
  cmdIpcMsg_T cmIpcHdr;
  Int32T funcIndex = 0;
  Int32T funcID = CMD_FUNC_WRITE_FUNC_ID;
  cmIpcHdr.requestKey = cmsh->cmfd;
  cmIpcHdr.code = CMD_IPC_CODE_FUNC_REQ;
  cmIpcHdr.clientType = CMD_IPC_TYPE_CLI;
  cmIpcHdr.srcID = IPC_CM_MGR;
  cmIpcHdr.dstID = comSend->compID;
  cmdIpcAddHdr(msgBuf, &cmIpcHdr);
  cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_INDEX, sizeof (Int32T), &funcIndex);
  cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_FUNC, sizeof (Int32T), &funcID);
  if(cmdIpcSend(comSend->sockID, msgBuf) == CMD_IPC_ERROR)
  {
    return CMD_IPC_ERROR;
  }
  return CMD_IPC_OK;
}

void
CmdFunShowRuningConfigMul(struct cmsh *cmsh, struct cmConfigManagerMulNode_T *currentNode)
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
	nodePre = node;
	if(node->cFlagh == 1){
		cmdPrint(cmsh," %s\n", node->nodeShow);
	}else{
		cmdPrint(cmsh,"%s\n", node->nodeShow);
	}
	if(node->nodeShow != NULL)
		free(node->nodeShow);
	if(node->nodeStr != NULL)
		free(node->nodeStr);
    CmdFunShowRuningConfigMul(cmsh, node);
  }
}
void
cmdAddServiceStartup()
{
    struct cmRunConfigManager_T *cmdCms = (struct cmRunConfigManager_T *)malloc(sizeof(*cmdCms));
    struct cmServiceManagerStartup_T *tmpService;
    struct cmdListNode *nn;
    cmdCms->compId = IPC_CM_MGR;
    cmdCms->cmdStrTable = cmdListNew();
    cmdListAddNode(gCmIpcBase->cmRunConfigTableMul, cmdCms);
    cmdListAddNode(cmdCms->cmdStrTable, "!");
    cmdListAddNode(cmdCms->cmdStrTable, "service management");
    CMD_MANAGER_LIST_LOOP(gCmIpcBase->cmServiceStartup, tmpService, nn)
    {
        cmdListAddNode(cmdCms->cmdStrTable, tmpService->strService);
    }
    cmdListAddNode(cmdCms->cmdStrTable, "!");
}

DECMD(cmdFunEnterRunningConfig,
    CMD_NODE_EXEC,
    IPC_CM_MGR|IPC_SHOW_MGR,
    "show running-config",
    "show running system information",
    "Running config infomation")
{
	Int32T nC = 0;
  if(cmsh->cmshFuncIndex == 0)
  {
    gCmIpcBase->cmRunConfigTableMul = cmdListNew();  /** Malloc  */
    gCmIpcBase->cmConfigNumComp = 0;  /** Reset Number Components */
    Int32T i;
	cmdAddServiceStartup();
    for(i = 0; i < IPC_MAX_MGR; i++)
    {
      if(gCmIpcBase->compTable[i].compID != 0)
      {
        if(cmdFunEnterRunningConfigSend(cmsh, &gCmIpcBase->compTable[i]) == CMD_IPC_OK)
        {
		  nC = 1;
          struct cmRunConfigManager_T *tmpNode = (struct cmRunConfigManager_T *)malloc(sizeof(*tmpNode));
          tmpNode->compId = gCmIpcBase->compTable[i].compID;
          tmpNode->cmdStrTable = cmdListNew();  /** Init Table command line */
          cmdListAddNode(gCmIpcBase->cmRunConfigTableMul, tmpNode);
          gCmIpcBase->cmConfigNumComp ++; /** Increase */
        }
      }
    }
	if(nC == 0){
        cmdSortRunningTableMul(gCmIpcBase);
    }

  }
  else
  {
    if(gCmIpcBase->cmConfigNumComp == 0)
    {
		struct cmConfigManagerMulNode_T *node;
		struct cmdListNode *nn;
		if(gCmIpcBase->cmCurrConfigTableMul == NULL){
			cmdPrint(cmsh,"");
			return CMD_IPC_OK;
		 }
		 gCmIpcBase->cmConfigNumComp = 1;
		CMD_MANAGER_LIST_LOOP(gCmIpcBase->cmCurrConfigTableMul, node, nn)
		{
			cmdPrint(cmsh,"!\n%s\n", node->nodeShow);
			CmdFunShowRuningConfigMul(cmsh, node);
			if(node->cmdStrTable != NULL){
				cmdListDel(node->cmdStrTable);
				node->cmdStrTable = NULL;
			}
		}
		if(gCmIpcBase->cmCurrConfigTableMul != NULL){
			cmdListDel(gCmIpcBase->cmCurrConfigTableMul);
			gCmIpcBase->cmCurrConfigTableMul = NULL;
		}
		 cmdPrint(cmsh,"!\nEnd\n");
		gCmIpcBase->cmConfigNumComp = 0;
		return CMD_IPC_OK;
    }
  }
  return CMD_IPC_WAIT;
}

DECMD(cmdFuncExecWriteRunningConfig,
    CMD_NODE_EXEC,
    IPC_CM_MGR,
    "write running-config",
    "Write running configuration",
    "Write configuration to the file")
{
  Int32T nC = 0;
  if(cmsh->cmshFuncIndex == 0)
  {
    gCmIpcBase->cmRunConfigTableMul = cmdListNew();  /** Malloc  */
    gCmIpcBase->cmConfigNumComp = 0;  /** Reset Number Components */
    Int32T i;
	cmdAddServiceStartup();
    for(i = 0; i < IPC_MAX_MGR; i++)
    {
      if(gCmIpcBase->compTable[i].compID != 0)
      {
        if(cmdFunEnterRunningConfigSend(cmsh, &gCmIpcBase->compTable[i]) == CMD_IPC_OK)
        {
			
          struct cmRunConfigManager_T *tmpNode = (struct cmRunConfigManager_T *)malloc(sizeof(*tmpNode));
		  nC = 1;
          tmpNode->compId = gCmIpcBase->compTable[i].compID;
          tmpNode->cmdStrTable = cmdListNew();  /** Init Table command line */
          cmdListAddNode(gCmIpcBase->cmRunConfigTableMul, tmpNode);
          gCmIpcBase->cmConfigNumComp ++; /** Increase */
        }
      }
    }
	if(nC == 0){
        cmdSortRunningTableMul(gCmIpcBase);
    }
  }
  else
  {
    if(gCmIpcBase->cmConfigNumComp == 0)
    {
      /** Write Here */
       gCmIpcBase->cmConfigNumComp = 1;
      if(gCmIpcBase->cmCurrConfigTableMul == NULL){
         cmdPrint(cmsh,"");
         return CMD_IPC_OK;
      }
	 cmdCmdConfigWrite(gCmIpcBase);
			cmdCmdConfigReLoad(gCmIpcBase);	/**	Reload Startup Config	*/
      cmdPrint(cmsh,"Write configure is success....\n");
	  gCmIpcBase->cmConfigNumComp = 0;
      return CMD_IPC_OK;
    }
  }
  return CMD_IPC_WAIT;
}

