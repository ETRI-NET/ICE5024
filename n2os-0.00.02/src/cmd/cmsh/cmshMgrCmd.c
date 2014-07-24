#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/types.h>
#include <ctype.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <stdlib.h>
#include <locale.h>
#include <time.h>
#include "editline/readline.h"
#include "histedit.h"

#include "nnTypes.h"
#include "nnStr.h"

#include "nnCmdLink.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"
#include "nnCmdInstall.h"
#include "nnCmdDefines.h"

extern Int32T cmshParseCommand(Int8T *msg, Int32T mode);
extern Int32T gCmshBlockHook;
extern Int32T cmshExit(Int32T gIndex);
extern Int32T jmpflag;
Uint32T nodeBackup; 
DENODE(cmdFuncEnterView,
  CMD_NODE_VIEW,
  CMD_NODE_NULL,
  "view",
  PRIVILEGE_MIN,
  "view",
  "Enter View Node")
{
  return CMD_IPC_OK;
}
DECMD(cmdFuncEnterShowHistory,
      CMD_NODE_VIEW,
      IPC_CMSH_MGR,
      "show history",
      "Show running system information",
      "Display the session command history")
{
  rl_history_show();
  return (CMD_IPC_OK);
}


int
cmdEnterEnable (struct cmsh* cmsh, Int32T cargc, Int8T **cargv)
{
    Int8T msgBuf[CMD_IPC_MAXLEN];
    Int8T *passBuf;
    Int32T nbytes;
    cmdIpcMsg_T cmIpcHdr;
    /** 1. Send .mode change request. to CM */
    cmIpcHdr.code = CMD_IPC_CODE_AUTH_ENABLE_REQ;   /** ENABLE authentication request code  */
    cmIpcHdr.requestKey = CMD_IPC_KEY_NULL;
    cmIpcHdr.clientType = CMD_IPC_TYPE_CLI;
    cmIpcHdr.mode = CMD_NODE_VIEW;
    cmIpcHdr.userKey = cmsh->userKey;
    cmIpcHdr.srcID = IPC_CMSH_MGR;
    cmIpcHdr.dstID = IPC_CM_MGR;
    cmdIpcAddHdr(msgBuf, &cmIpcHdr);
    cmdIpcSend(cmsh->cmfd, msgBuf);
    memset(&msgBuf, 0, CMD_IPC_MAXLEN);
    nbytes = cmdIpcRecv(cmsh->cmfd, msgBuf);
    if(nbytes == 0)
    {
        return CMD_IPC_ERROR;
    }
    /** 2-1. If receives RES_PASSWD code, retry with passwd from user */
    cmdIpcUnpackHdr(msgBuf, &cmIpcHdr);
    if(cmIpcHdr.code == CMD_IPC_CODE_AUTH_ENABLE_REQ_PASSWD)
    {
        passBuf = getpass ("Password: ");
        nbytes = strlen(passBuf);
        if(nbytes == 0)
        {
            cmdPrint(cmsh, " % Wrong password.\n");
            return (CMD_IPC_ERROR);
        }
        cmIpcHdr.code = CMD_IPC_CODE_AUTH_ENABLE_REQ_PASSWD;
        cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_PASSWD, nbytes, passBuf);
        cmdIpcSend(cmsh->cmfd, msgBuf);
        nbytes = cmdIpcRecv(cmsh->cmfd, msgBuf);
        if(nbytes == 0)
        {
            return CMD_IPC_ERROR;
        }
        cmdIpcUnpackHdr(msgBuf, &cmIpcHdr);
    }
    /** 2-2. If receives RES_OK code, change mode */
    if(cmIpcHdr.code == CMD_IPC_CODE_AUTH_ENABLE_RES_OK)
    {
        return CMD_IPC_OK;
    }
    cmdPrint(cmsh, " % You don't have permission to access to enable mode.\n");
    return (CMD_IPC_ERROR);
}

DENODE(cmdFuncEnterEnable,
  CMD_NODE_EXEC,
  CMD_NODE_VIEW,
  "exec",
  PRIVILEGE_EXEC,
  "enable",
  "Enter Enable Node")
{
  return cmdEnterEnable(cmsh, cargc, cargv);
}

int
cmdEnterConfigure (struct cmsh* cmsh, Int32T cargc, Int8T **cargv)
{
    Int8T *passBuf;
    Int32T nbytes, length;
    Int8T msgBuf[CMD_IPC_MAXLEN];
    cmdIpcMsg_T cmIpcHdr;
    /** 1. Send .mode change request. to CM */
    cmIpcHdr.code = CMD_IPC_CODE_AUTH_CONFIGURE_REQ; /** CONFIGURE authentication request code  */
    cmIpcHdr.requestKey = CMD_IPC_KEY_NULL;
    cmIpcHdr.clientType = CMD_IPC_TYPE_CLI;
    cmIpcHdr.mode = CMD_NODE_EXEC;
    cmIpcHdr.userKey = cmsh->userKey;
    cmIpcHdr.srcID = IPC_CMSH_MGR;
    cmIpcHdr.dstID = IPC_CMSH_MGR;
    cmdIpcAddHdr(msgBuf, &cmIpcHdr);

    cmdIpcSend(cmsh->cmfd, msgBuf);
    length = cmdIpcRecv(cmsh->cmfd, msgBuf);
    if(length == 0)
    {
      return CMD_IPC_ERROR;
    }
    cmdIpcUnpackHdr(msgBuf, &cmIpcHdr);
    /** 2-1. If receives RES_EXIST, prInt32T the error to user and finish */
    if(cmIpcHdr.code == CMD_IPC_CODE_AUTH_CONFIGURE_RES_EXIST)
    {
        cmdPrint(cmsh, "Other session is in CONFIGURE mode.\n.");
        return CMD_IPC_ERROR;
    }
    /** 2-2. If receives RES_PASSWD code, retry with passwd from user */
    else if(cmIpcHdr.code == CMD_IPC_CODE_AUTH_CONFIGURE_REQ_PASSWD)
    {
        passBuf = getpass ("Password: ");
        nbytes = strlen(passBuf);
        if(nbytes == 0)
        {
            cmdPrint(cmsh, " %% Wrong password.\n");
            return (CMD_IPC_ERROR);
        }

        cmIpcHdr.code = CMD_IPC_CODE_AUTH_CONFIGURE_REQ_PASSWD;
        cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_PASSWD, nbytes, passBuf);
        cmdIpcSend(cmsh->cmfd, msgBuf);
        nbytes = cmdIpcRecv(cmsh->cmfd, msgBuf);
        if(nbytes == 0)
        {
            return CMD_IPC_ERROR;
        }
        cmdIpcUnpackHdr(msgBuf, &cmIpcHdr);
    }

    /** 2-3. If receives RES_OK code, change mode */
    if(cmIpcHdr.code == CMD_IPC_CODE_AUTH_CONFIGURE_RES_OK)
    {
        cmdPrint(cmsh, "Enter configuration commands, one per line.  End with CNTL/Z\n");
        return (CMD_IPC_OK);
    }
    cmdPrint(cmsh, " %% You don't have permission to access to configure mode.\n.");
    return (CMD_IPC_ERROR);
}

DENODE(cmdFuncEnterConfig,
  CMD_NODE_CONFIG,
  CMD_NODE_EXEC,
  "config",
  PRIVILEGE_CONF,
  "configure terminal",
  "Enter Config Node",
  "Enter Config Node")
{
  return cmdEnterConfigure(cmsh, cargc, cargv);
}

DENODE(cmdFuncEnterInterface,
  CMD_NODE_INTERFACE,
  CMD_NODE_CONFIG,
  "interface",
  PRIVILEGE_INFD,
  "interface WORD",
  "Enter Interface Node",
  "Interface Name WORD ")
{
  return CMD_IPC_OK;
}

DENODE(cmdFuncEnterVlan,
  CMD_NODE_VLAN,
  CMD_NODE_CONFIG,
  "vlan",
  PRIVILEGE_VLAN,
  "vlan database",
  "Enter vlan Node",
  "vlan database mode")
{
  return CMD_IPC_OK;
}


DENODE(cmdFuncEnterVty,
  CMD_NODE_VTY,
  CMD_NODE_CONFIG,
  "vty",
  PRIVILEGE_VTY,
  "line vty <0-15>",
  "Configure a terminal line",
  "Virtual terminal",
  "First Line number")
{
  return CMD_IPC_OK;
}
DENODE(cmdFuncEnterConsole,
  CMD_NODE_CONSOLE,
  CMD_NODE_CONFIG,
  "console",
  PRIVILEGE_CONSOLE,
  "line console <0-0>",
  "Configure a terminal line",
  "Primary terminal line",
  "First Line number")
{
  return CMD_IPC_OK;
}

DENODE(cmdFuncEnterRouterMap,
  CMD_NODE_CONFIG_ROUTE_MAP,
  CMD_NODE_CONFIG,
  "router-map",
  PRIVILEGE_CONFIG_ROUTE_MAP,
  "route-map WORD (deny|permit) <1-65535>",
  "Create route-map or enter route-map command mode",
  "Route map tag",
  "Route map denies set operations",
  "Route map permits set operations",
  "Sequence to insert to/delete from existing route-map entry")
{
  return CMD_IPC_OK;
}

DENODE(cmdFuncEnterKeychain,
  CMD_NODE_CONFIG_KEYCHAIN,
  CMD_NODE_CONFIG,
  "config-keychain",
  PRIVILEGE_CONFIG_KEYCHAIN,
  "key chain WORD",
  "Authentication key management",
  "Key-chain management",
  "Key-chain name")
{
  return CMD_IPC_OK;
}

DENODE(cmdFuncEnterKeychainKey,
  CMD_NODE_CONFIG_KEYCHAIN_KEY,
  CMD_NODE_CONFIG_KEYCHAIN,
  "config-keychain-key",
  PRIVILEGE_CONFIG_KEYCHAIN_KEY,
  "key <0-2147483647>",
  "Configure a key",
  "Key identifier number")
{
  return CMD_IPC_OK;
}

DENODE(cmdFuncEnterRip,
  CMD_NODE_CONFIG_RIP,
  CMD_NODE_CONFIG,
  "config-rouer",
  PRIVILEGE_CONFIG_RIP,
  "router rip",
  "Enable a routing process",
  "Routing Information Protocol (RIP)")
{
  return CMD_IPC_OK;
}

DENODE(cmdFuncEnterIsis,
  CMD_NODE_CONFIG_ISIS,
  CMD_NODE_CONFIG,
  "config-rouer",
  PRIVILEGE_CONFIG_ISIS,
  "router isis WORD",
  "Enable a routing process",
  "Routing Information Protocol (ISIS)",
  "Routing area tag")
{
  return CMD_IPC_OK;
}

DENODE(cmdFuncEnterOspf,
  CMD_NODE_CONFIG_OSPF,
  CMD_NODE_CONFIG,
  "config-rouer",
  PRIVILEGE_CONFIG_OSPF,
  "router ospf",
  "Enable a routing process",
  "Routing Information Protocol (OSPF)")
{
  return CMD_IPC_OK;
}

DENODE(cmdFuncEnterUser,
  CMD_NODE_CONFIG_USER,
  CMD_NODE_CONFIG,
  "user-mgr",
  PRIVILEGE_CONFIG_USER,
  "user management",
  "user configure",
  "user managment node")
{
  return CMD_IPC_OK;
}

DENODE(cmdFuncEnterSerivce,
  CMD_NODE_CONFIG_SERVICE,
  CMD_NODE_CONFIG,
  "service-mgr",
  PRIVILEGE_CONFIG_SERVICE,
  "service management",
  "service configure",
  "service managment node")
{
  return CMD_IPC_OK;
}

DENODE(cmdFuncEnterTestnode,
  CMD_NODE_CONFIG_TEST,
  CMD_NODE_INTERFACE,
  "test-node",
  PRIVILEGE_CONFIG_TEST,
  "test WORD",
  "Enter test Node for node-in-node test",
  "Testnode input WORD ")
{
  return CMD_IPC_OK;
}
/** Add test mutil node*/

DENODE(cmdFuncEnterTestnodemul,
  CMD_NODE_CONFIG_TEST_MUL,
  CMD_NODE_CONFIG_TEST,
  "test-node-node",
  PRIVILEGE_CONFIG_TEST_MUL,
  "test-mul WORD",
  "Enter test Node for node-in-node test",
  "Testnode input WORD ")
{
  return CMD_IPC_OK;
}

/**
 * Description: Process command line conect via ssh
 *
 * @param [in]  cmsh: Command structure
 * @param [in]  cargc: int value input
 * @param [in]  cargv: string input
 * @param [out] none
 *
 * @retval : CMD_IPC_OK: Default
 */
DECMD(cmdFuncExecSsh,
  CMD_NODE_EXEC,
  IPC_CMSH_MGR,
  "ssh WORD",
  "Open a secure shell client connection",
  "Remote Address")
{
  Int8T *cargvs[4];
  cargvs[0] = strdup("ssh");
  cargvs[1] = strdup(cargv[1]);
  cargvs[2] = NULL;
  cmdCallbackShellCmd(cargvs);
  return CMD_IPC_OK;
}
/**
 * Description: Process command line connect via telnet
 *
 * @param [in]  cmsh: Command structure
 * @param [in]  cargc: int input
 * @param [in]  cargv: string input
 * @param [out] none
 *
 * @retval : CMD_IPC_OK: Default
 */
DECMD(cmdFuncExecTelnet,
  CMD_NODE_EXEC,
  IPC_CMSH_MGR,
  "telnet WORD",
  "Open a Telnet Connection",
  "Remote Address")
{
  Int8T *cargvs[4];
  cargvs[0] = strdup("telnet");
  cargvs[1] = strdup(cargv[1]);
  cargvs[2] = NULL;
  cmdCallbackShellCmd(cargvs);
  return CMD_IPC_OK;
}
/**
 * Description: Process command line ping
 *
 * @param [in]  cmsh: Command structure
 * @param [in]  cargc: int input
 * @param [in]  cargv: string input
 * @param [out] none
 *
 * @retval : CMD_IPC_OK: Default
 */
DECMD(cmdFuncExecPing,
  CMD_NODE_EXEC,
  IPC_CMSH_MGR,
  "ping WORD",
  "Send echo messages",
  "Ping destination address or hostname")
{
  Int8T *cargvs[4];
  cargvs[0] = strdup("ping");
  cargvs[1] = strdup(cargv[1]);
  cargvs[2] = NULL;
  cmdCallbackShellCmd(cargvs);
  return CMD_IPC_OK;
}
/**
 * Description: process trace route
 *
 * @param [in]  cmsh: Command structure
 * @param [in]  cargc: int input value
 * @param [in]  cargv: string input value
 * @param [out] none
 *
 * @retval : CMD_IPC_OK: Default
 */
DECMD(cmdFuncExecTrace,
  CMD_NODE_EXEC,
  IPC_CMSH_MGR,
  "traceroute WORD",
  "Trace route to destination",
  "Trace route to destination address or hostname")
{
  Int8T *cargvs[4];
  cargvs[0] = strdup("traceroute");
  cargvs[1] = strdup(cargv[1]);
  cargvs[2] = NULL;
  cmdCallbackShellCmd(cargvs);
  return CMD_IPC_OK;
}
/**
 * Description: Process tace route by Ip
 *
 * @param [in]  cmsh: Command structure
 * @param [in]  cargc: int input value
 * @param [in]  cargv: string input value
 * @param [out] none
 *
 * @retval : CMD_IPC_OK: Default
 */
DECMD(cmdFuncExecTraceIp,
  CMD_NODE_EXEC,
  IPC_CMSH_MGR,
  "traceroute ip WORD",
  "Trace route to destination",
  "IP Trace",
  "Trace route to destination address or hostname")
{
  Int8T *cargvs[4];
  cargvs[0] = strdup("traceroute");
  cargvs[1] = strdup(cargv[2]);
  cargvs[2] = NULL;
  cmdCallbackShellCmd(cargvs);
  return CMD_IPC_OK;
}

/**
 * Description: Show infor time
 *
 * @param [in]  cmsh: Command structure
 * @param [in]  cargc: none
 * @param [in]  cargv: none
 * @param [out] none
 *
 * @retval : CMD_IPC_OK: Default
 */
DECMD(cmdFuncViewShowClock,
    CMD_NODE_VIEW,
    IPC_CMSH_MGR,
    "show clock",
    "Show running system information",
    "Display the system clock")
{
  time_t tp;
  struct tm *t;
  Int8T  fmt[64];
  time(&tp);
  t = localtime(&tp);
  strftime(fmt, sizeof(fmt), "%a %b %d %H:%M:%S %Y", t);
  cmdPrint(cmsh, "%s \n", fmt);
  return CMD_IPC_OK;
}
ALICMD(cmdFuncViewShowClock,
    CMD_NODE_EXEC,
    IPC_CMSH_MGR,
    "show clock",
    "Show running system information",
    "Display the system clock");

/**
 * Description: Show host name
 *
 * @param [in]  cmsh: Command structure
 * @param [in]  cargc: none
 * @param [in]  cargv: none
 * @param [out] none
 *
 * @retval : CMD_IPC_OK: Default
 */
DECMD(cmdFuncEnterShowHostName,
    CMD_NODE_VIEW,
    IPC_CMSH_MGR,
    "show hostname",
    "Show running system information",
    "IP domain-name and host table")
{
  cmdPrint(cmsh,"%s\n", cmsh->hostName);
  return CMD_IPC_OK;
}
ALICMD(cmdFuncEnterShowHostName,
    CMD_NODE_EXEC,
    IPC_CMSH_MGR,
    "show hostname",
    "Show running system information",
    "IP domain-name and host table");

Int32T
cmdFuncSetTime(Int8T *szBuffer, struct tm *rTime)
{
	Int8T *rvTime;
	if((rvTime = strptime(szBuffer, "%H:%M:%S %d %b %Y",rTime)) == NULL) {
		return CMD_IPC_ERROR;
	}
	return CMD_IPC_OK;
}
/**
 * Description: Functon set time
 *
 * @param [in]  cmsh: Command structure
 * @param [in]  cargc: int input
 * @param [in]  cargv: string time info
 * @param [out] none
 *
 * @retval : CMD_IPC_OK: Default
 */
DECMD(cmdFuncConfigSetClock,
  CMD_NODE_CONFIG,
  IPC_CMSH_MGR,
  "clock set HH:MM:SS <1-31> MONTH <2013-2100>",
  "Configure time-of-day clock",
  "Set the time and date",
  "Current Time <hh:mm:ss>",
  "Day of the month <1-31>",
  "Month of the year",
  "Year")
{
	Int8T szBuffTime[1024];
	struct tm rTime;
	bzero(&szBuffTime, 1024);
	sprintf(szBuffTime, "%s %s %s %s", cargv[2], cargv[3], cargv[4], cargv[5]);
	if(cmdFuncSetTime(szBuffTime, &rTime) != CMD_IPC_OK){
	  cmdPrint(cmsh, "[%s]Time is [%s] Wrong...\n", __func__, szBuffTime);
  }
	else
	{
		struct timeval tv;
		tv.tv_sec = mktime(&rTime);
		tv.tv_usec = 0;
		settimeofday(&tv, 0);
		cmdPrint(cmsh, "[%s]Time is [%s] Success...\n", __func__, szBuffTime);
	}
  return CMD_IPC_OK;
}
/**
 * Description: Function set time and date
 *
 * @param [in]  cmsh: Command structure
 * @param [in]  cargc: int input
 * @param [in]  cargv: string date info
 * @param [out] none
 *
 * @retval : CMD_IPC_OK: Default
 */
DECMD(cmdFuncConfigSetClockMonth,
  CMD_NODE_CONFIG,
  IPC_CMSH_MGR,
  "clock set HH:MM:SS MONTH <1-31> <2013-2100>",
  "Configure time-of-day clock",
  "Set the time and date",
  "Current Time <hh:mm:ss>",
  "Month of the year",
  "Day of the month",
  "Year")
{
	Int8T szBuffTime[1024];
	struct tm rTime;
  bzero(&szBuffTime, 1024);
  sprintf(szBuffTime, "%s %s %s %s", cargv[2], cargv[4], cargv[3], cargv[5]);
  if(cmdFuncSetTime(szBuffTime, &rTime) != CMD_IPC_OK){
    cmdPrint(cmsh, "[%s]Time is [%s] Wrong...\n", __func__, szBuffTime);
  }
  else
	{
		struct timeval tv;
		tv.tv_sec = mktime(&rTime);
    tv.tv_usec = 0;
    settimeofday(&tv, 0);
    cmdPrint(cmsh, "[%s]Time is [%s] Success...\n", __func__, szBuffTime);
	}
  return CMD_IPC_OK;
}
/**
 * Description: Function set time zone
 *
 * @param [in]  cmsh: Command structure
 * @param [in]  cargc: int input
 * @param [in]  cargv: string time zone
 * @param [out] none
 *
 * @retval : CMD_IPC_OK: Default
 */
Int32T
isValidNum(Int8T *str)
{
	if (*str == '-')
	{
    ++str;
	}
	if (!*str)
	{
		return 0;
	}
	while (*str)
 	{
		if (!isdigit(*str))
		 return 0;
		else
		 ++str;
 	}
	return 1;
}

DECMD(cmdFuncConfigureZoneClock,
  CMD_NODE_CONFIG,
  IPC_CMSH_MGR,
  "clock timezone WORD <-23-23> <0-59>",
  "Configure time-of-day clock",
  "Configure time zone",
  "name of time zone",
  "Hours offset from UTC",
  "Minutes offset from UTC <0-59>")
{
  struct tm * timeinfo;
	time_t rawtime;
	/**	Set Timezone Name	*/
	setenv("TZ", cargv[0], 1);
	tzset();
	/**	Shift Clock	*/
	time(&rawtime );
  timeinfo = gmtime(&rawtime);
	if(isValidNum(cargv[1]))
	{
		timeinfo->tm_hour = atoi(cargv[1]);
		if(cargc < 3)
			timeinfo->tm_min = 0;
		else
		{
			if(isValidNum(cargv[2]))
			{
				timeinfo->tm_min = atoi(cargv[2]);
			}
		}
		struct timeval tv;
    tv.tv_sec = mktime(timeinfo);
    tv.tv_usec = 0;
    settimeofday(&tv, 0);
    cmdPrint(cmsh, "[%s]Set Timezone is Success...\n", __func__);
	}
	else
	{
  	cmdPrint(cmsh, "[%s]Set Timezone is Error...\n", __func__);
	}
  return CMD_IPC_OK;
}

/**
 * Description: Function process command line 'do' in enable node
 *
 * @param [in]  cmsh: Command structure
 * @param [in]  cargc: int input
 * @param [in]  cargv: string command line run in enable node
 * @param [out] none
 *
 * @retval : CMD_IPC_OK: Default
 */
DECMD(cmdFuncConfigureDo,
  CMD_NODE_CONFIG,
  IPC_CMSH_MGR,
  "&&do LINE",
  "To run exec commands in config mode",
  "Exec Command")
{
  jmpflag = 0;
  nodeBackup = cmsh->currentDepth;
  cmsh->currentDepth = CMD_NODE_EXEC;
  gCmshBlockHook = cmshParseCommand(cargv[1], 1);
  if(gCmshBlockHook == 0 && cmsh->currentDepth == CMD_NODE_EXEC)
  {
    cmsh->currentDepth = nodeBackup;//CMD_NODE_CONFIG;
  }
  jmpflag = 1;
  return gCmshBlockHook;
}

ALICMD(cmdFuncConfigureDo,
  CMD_NODE_INTERFACE,
  IPC_CMSH_MGR,
  "&&do LINE",
  "To run exec commands in config mode",
  "Exec Command");

ALICMD(cmdFuncConfigureDo,
  CMD_NODE_VLAN,
  IPC_CMSH_MGR,
  "&&do LINE",
  "To run exec commands in config mode",
  "Exec Command");

ALICMD(cmdFuncConfigureDo,
  CMD_NODE_CONFIG_TEST,
  IPC_CMSH_MGR,
  "&&do LINE",
  "To run exec commands in config mode",
  "Exec Command");


/**
 * Description: cmdDumpNode
 *
 * @param [in]  currentNode : Current tree node of recursive traversing
 * @param [in]  depth  :  depth
 * @param [out] none
 *
 * @retval : none
 */
void
cmdDumpNode(struct cmsh *cmsh, struct cmdTreeNode *currentNode, Int32T numSpace)
{
  struct cmdTreeNode *node;
  struct cmdListNode *nn, *hNn;
  Int8T *helpStr;
  Int32T hIndex;
  CMD_MANAGER_LIST_LOOP(currentNode->cNodeList, node, nn)
  {
//    cmdPrint(cmsh, "%*c+-%s %*c [%s]\n", numSpace, ' ', node->tokenStr, 5, ' ', node->helpStr);
    cmdPrint(cmsh, "%*c+-%s %*c [", numSpace, ' ', node->tokenStr, 5, ' ');
    hIndex = 0;
    CMD_MANAGER_LIST_LOOP(node->cHelpTable, helpStr, hNn)
    {
      if(!hIndex)
      {
        cmdPrint(cmsh, "%s", helpStr);
      } else {
        cmdPrint(cmsh, "/%s", helpStr);
      }
      hIndex++;
    }
    cmdPrint(cmsh, "]\n");
    cmdDumpNode(cmsh, node, numSpace + 3);
  }
}

DECMD(cmdFuncShowCli,
  CMD_NODE_VIEW,
  IPC_CMSH_MGR,
  "show cli",
  "Show running system information",
  "CLI Tree Node")
{
  struct cmdTree *ctree = cmsh->ctree;
  if(cmsh->currentDepth == CMD_NODE_VIEW)
  {
    cmdPrint(cmsh, "View Mode:\n");
  } else if(cmsh->currentDepth == CMD_NODE_EXEC) {
    cmdPrint(cmsh, "Enable Mode:\n");
  } else if(cmsh->currentDepth == CMD_NODE_CONFIG) {
    cmdPrint(cmsh, "Configure Mode:\n");
  }
  cmdDumpNode(cmsh, &ctree->modeTree[cmsh->currentDepth], 0);
  return (CMD_IPC_OK);
}
ALICMD(cmdFuncShowCli,
  CMD_NODE_EXEC,
  IPC_CMSH_MGR,
  "show cli",
  "Show running system information",
  "CLI Tree Node");

ALICMD(cmdFuncShowCli,
  CMD_NODE_CONFIG,
  IPC_CMSH_MGR,
  "show cli",
  "Show running system information",
  "CLI Tree Node");

DECMD(cmdFuncEnterConfigHostname,
    CMD_NODE_CONFIG,
    IPC_CMSH_MGR,
    "hostname WORD",
    "Ser system's network name",
    "This system's network name")
{
  sprintf(cmsh->hostName, "%s", cargv[1]);
  if(sethostname(cargv[1], strlen(cargv[1])) < 0)
  {
    cmdPrint(cmsh, "Not Success....\n");
  }
  else
  {
    cmdPrint(cmsh, "Is Success....\n");
  }
  return CMD_IPC_OK;
}

ALICMD(cmdFuncEnterConfigHostname,
    CMD_NODE_CONFIG,
    IPC_CMSH_MGR,
    "hostname WORD",
    "Ser system's network name",
    "This system's network name");


DECMD(cmdFuncEnterSetHistory,
    CMD_NODE_VIEW,
    IPC_CMSH_MGR,
    "terminal history size <0-256>",
    "Set terminal line parameters",
    "Enable and control the command history function",
    "set history buffer size",
    "Size history buffer")
{
  Int8T *endptr = NULL;
  Int32T lines = strtol (cargv[3], &endptr, 10);
  if (lines < 0 || lines > 256 || *endptr != '\0')
  {
      cmdPrint(cmsh, "Length is malformed\n");
  }
  else
  {
    stifle_history(lines);
    cmdPrint(cmsh, "Is Success...\n");
  }
  return (CMD_IPC_OK);
}

ALICMD(cmdFuncEnterSetHistory,
    CMD_NODE_EXEC,
    IPC_CMSH_MGR,
    "terminal history size <0-256>",
    "Set terminal line parameters",
    "Enable and control the command history function",
    "set history buffer size",
    "Size history buffer");
