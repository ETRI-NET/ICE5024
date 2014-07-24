#if !defined(_cmMgrCmIpc_h)
#define _cmMgrCmIpc_h

/*345678901234567890123456789012345678901234567890123456789012345678901234567890*/
/********************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute.
*           All rights reserved. No part of this software shall be reproduced,
*           stored in a retrieval system, or transmitted by any means,
*           electronic, mechanical, photocopying, recording, or otherwise,
*           without written permission from ETRI.
********************************************************************************/

/**
 * @brief : This file include timer functions.
 *  - Block Name : Policy Manager
 *  - Process Name : cmmgr
 *  - Creator : PyungKoo Park
 *  - Initial Date : 2014/03/03
 */

/**
 * @file:
 *
 * $Author:
 * $Date:
 * $Revision:
 * $LastChangedBy:
 */
typedef struct cmdCmCompSession
{
  Int32T compID;
  Int32T sockID;
  /** FLAGS:  */
  Int32T flagID;
} cmdCmCompSession_T;

typedef struct cmdCmCmshSession
{
    Int32T requestKey;         /** unique ID for command   */
    Int32T clientType;         /** CMD_IPC_TYPE_CLI/WEB/...  */
    Int32T mode;               /** CMD_NODE_CONFIG,      */
    Int32T argc;
    Int8T *argv[CMSH_ARGC_MAX];
    Int32T srcID;              /** MO_CMSH, MO_CM,     */
    Int32T dstID;              /** MO_CMSH, MO_CM,     */
    Int32T isWaiting;
    Int32T fd;                 /** connected CMSH fd   */
    Int32T userKey;            /**  == 1 if used     */
    Int8T userID[CMD_USERID_MAX];    /** user ID     */
    Int32T privilege;          /** privilege for user    */
    Int32T historyFile;        /** opened history file fd  */
    struct cmdList *notifyTable;
}cmdCmCmshSession_T;

/** Tu Hai Define struct CmPassword */
struct cmPassword
{
  Int8T key_hash[10];
  /** Configure password  */
  Int8T configurePw[CMD_PASSWD_MAX];
  Int8T configurePw_encrypt[CMD_PASSWD_MAX];

  /** Enable password */
  Int8T enablePw[CMD_PASSWD_MAX];
  Int8T enablePw_encrypt[CMD_PASSWD_MAX];
  /** flags*/
  Int32T flag_enable;
  Int32T flag_config;
};

#define CMD_MAX_SESSION 1024

struct cmService_T
{
  Int32T pInetd;        /** Pid of Inetd    */
  Int32T pTelnetPortId; /** Port of Telnet  */
  Int32T pSshPortId;    /** Port of SSh     */
};

#define CM_USER_MAX_SIZE  1024
#define CM_PASS_MAX_SIZE  1024

struct cmUserManager_T
{
 Int32T cmUserMode;
 Int8T  cmUserName[CM_USER_MAX_SIZE];
 Int8T  cmUserPassword[CM_PASS_MAX_SIZE];
 Int8T	cmUserEncryptMethod[20];
 Int8T  cmUserPassEncrypt[CM_PASS_MAX_SIZE]; 
};

struct cmConfigManager_T
{
 Int8T *nodeStr;
 struct cmdList *cmdStrTable;   /** Command Line  */
};

enum{
	CMD_RUNNING_COMMAND_LINE = 0,
	CMD_RUNNING_COMMAND_NODE,
	CMD_RUNNING_CONFIG_NODE,
	CMD_RUNNING_EXIT_AUTO,
	CMD_RUNNING_DEFAULT,
};

struct cmConfigManagerMulNode_T
{
	Int8T *nodeStr;
	Int8T *nodeShow;
	Int8T *nodePrevious[CMD_IPC_MAXLEN];
	Int32T idDepth;
	Int32T cFlagh;
	struct cmdList *cmdStrTable;
};
struct cmRunConfigManager_T
{
  Int32T compId;
  struct cmdList *cmdStrTable;
};
#define SERVICE_TELNET_FLAG   	1
#define SERVICE_SSH_FLAG     	2
struct cmServiceManagerStartup_T
{
	Int32T flags;
	Int8T *strService;
};
struct cmdStartupManager_T
{
	Int32T fd;
	Int8T *nodeStr;
};

typedef struct cm
{
  /** For startup-config loading */
  struct cmsh *cmsh;

  /** User information file fd (userID, userKey, privilege, history filename) */
  Int32T userInfo;

  /** Config Lock */
  Int32T configSession; /** 0~15: sessionID which is in configure mode, minus: no one is in configure mode. */

  /** Current CM status.  */
  enum {
      CM_NORMAL,
      CM_CLOSE,
  } status;
  cmdCmCompSession_T compTable[IPC_MAX_MGR];
  cmdCmCmshSession_T cmshTable[CMD_MAX_SESSION];
  /** Use for check pass*/
  struct cmPassword passwordCm;
  /** Telnet & Ssh Service  */
  struct cmService_T cmService;
  /** User Manager  */
  struct cmdList *cmUserTable;

  Int32T cmConfigNumComp;
  /** Startup Configure Table*/
  struct cmdList *cmConfigTableMul;
  /** Running Configure Table */
	struct cmdList *cmCurrConfigTable;	/**	Sorted	*/
  
  struct cmdList *cmRunConfigTableMul;
  struct cmdList *cmCurrConfigTableMul;  /** Sorted  */

	struct cmdList *cmRunStartupTable;
	struct cmdList *cmServiceStartup;
	void *gCmdStartupTimerEvent;
} cmMgrT;

extern cmMgrT *gCmIpcBase;

Int32T cmdCmdServiceStart(cmMgrT *cm);
Int32T cmdCmdServiceInit(cmMgrT *cm);
Int32T cmdCmdServiceStop(cmMgrT *cm);
Int32T cmdCmdServiceStart(cmMgrT *cm);

Int32T cmdCmdUserAdd(struct cmsh *cmsh, Int8T *userName, Int8T *userMode, Int8T *passWord);
Int32T cmdCmdUserDel(Int8T *userName);
Int8T *cmdCmdUserGetMode(Int32T modeId);

/**    Config Init     */
Int32T cmdCmdConfigInit(cmMgrT *cm);
Int32T cmdCmdConfigFree(cmMgrT *cm);
Int32T cmdCmdConfigWrite(cmMgrT *cm);
Int32T cmdCmdConfigReLoad(cmMgrT *cm);
Int32T cmdCmdConfigDump(cmMgrT *cm, Int32T stUpState);

/* Running config*/
void cmdSortRunningTableMul(cmMgrT * cm);

/** Encrypt password user*/
Int8T* str2md5(Int8T* str, Int32T length);
void cmdSha256(Int8T *string, Int8T outputBuffer[65]);
void cmdSha512(Int8T *string, Int8T outputBuffer [129]);
void cmdUserWriteStore(); 
#endif

