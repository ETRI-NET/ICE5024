#include <stdio.h>
#include <stdlib.h>
#include "nnTypes.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdInstall.h"
#include "nosLib.h"
#include "nnBuffer.h"

DECMD(cmdDluTestDupFunc,
  CMD_NODE_EXEC,
  IPC_DLU_TESTER,
  "show abc def",
  "DLU Show Info",
  "DLU abc Help",
  "DLU def Help")
{
  cmdPrint(cmsh,"[%s]\n", __func__);
  return CMD_IPC_OK;
}

// PROCESS Manager Upgrade
DECMD(cmdDluTestFuncUpgradeProcessmgr,
    CMD_NODE_EXEC,
    IPC_DLU_TESTER,
    "upgrade procmgr version WORD",
    "upgrade",
    "ipcmgr",
    "version",
    "upgrade version")
{
  nnBufferT apiBuff, msgBuff;
  Int8T rcvMsg[4096] = {0,};
  Uint32T rcvLength = 0;
  Int32T cmdFd = 0, cmdKey = 0, cmdResult = 0, msgLen = 0;
  Int8T result[64] = {0,};

  nnBufferReset(&apiBuff);
  nnBufferReset(&msgBuff);

  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt8T  (&apiBuff, strlen(cargv[3]));
  nnBufferSetString (&apiBuff, cargv[3], strlen(cargv[3]));

  ipcSendSync(PROCESS_MANAGER,
              IPC_DYNAMIC_UPGRADE_TEST_SEND,
              IPC_DYNAMIC_UPGRADE_TEST_RESPONSE,
              apiBuff.data, apiBuff.length, rcvMsg, &rcvLength);

  nnBufferAssign(&msgBuff, rcvMsg, rcvLength);

  cmdFd = nnBufferGetInt32T(&msgBuff);
  cmdKey = nnBufferGetInt32T(&msgBuff);
  cmdResult = nnBufferGetInt32T(&msgBuff);
  msgLen = nnBufferGetInt8T(&msgBuff);
  nnBufferGetString(&msgBuff, result, msgLen);

  NNLOG(LOG_DEBUG, "cmdFd : %d, cmdKey : %d, cmdResult : %d\n",
                    cmdFd, cmdKey, cmdResult);

  if(cmdResult == SUCCESS)
  {
    cmdPrint(cmsh, "%s is successfully updated to version %s\n",
                    cargv[1], cargv[3]);
  }
  else
  {
    cmdPrint(cmsh, "Failure %s updated to version %s\n",
                    cargv[1], cargv[3]);
  }

  cmdPrint(cmsh,"End");
  ipcProcPendingMsg();

  return CMD_IPC_OK;
}


// IPC Manager Upgrade
DECMD(cmdDluTestFuncUpgradeIpcmgr,
    CMD_NODE_EXEC,
    IPC_DLU_TESTER,
    "upgrade ipcmgr version WORD",
    "upgrade",
    "ipcmgr",
    "version",
    "upgrade version")
{
  nnBufferT apiBuff, msgBuff;
  Int8T rcvMsg[4096] = {0,};
  Uint32T rcvLength = 0;
  Int32T cmdFd = 0, cmdKey = 0, cmdResult = 0, msgLen = 0;
  Int8T result[64] = {0,};

  nnBufferReset(&apiBuff);
  nnBufferReset(&msgBuff);

  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt8T  (&apiBuff, strlen(cargv[3]));
  nnBufferSetString (&apiBuff, cargv[3], strlen(cargv[3]));

  ipcSendSync(IPC_MANAGER, 
              IPC_DYNAMIC_UPGRADE_TEST_SEND,
              IPC_DYNAMIC_UPGRADE_TEST_RESPONSE, 
              apiBuff.data, apiBuff.length, rcvMsg, &rcvLength);

  nnBufferAssign(&msgBuff, rcvMsg, rcvLength);

  cmdFd = nnBufferGetInt32T(&msgBuff);
  cmdKey = nnBufferGetInt32T(&msgBuff);
  cmdResult = nnBufferGetInt32T(&msgBuff);
  msgLen = nnBufferGetInt8T(&msgBuff);
  nnBufferGetString(&msgBuff, result, msgLen);
  
  NNLOG(LOG_DEBUG, "cmdFd : %d, cmdKey : %d, cmdResult : %d\n",
                    cmdFd, cmdKey, cmdResult);

  if(cmdResult == SUCCESS)
  {
    cmdPrint(cmsh, "%s is successfully updated to version %s\n",
                    cargv[1], cargv[3]);
  }
  else
  {
    cmdPrint(cmsh, "Failure %s updated to version %s\n",
                    cargv[1], cargv[3]);
  }

  cmdPrint(cmsh,"End");
  ipcProcPendingMsg();

  return CMD_IPC_OK;
}


// PORT Interface Manager Upgrade
DECMD(cmdDluTestFuncUpgradePifmgr,
    CMD_NODE_EXEC,
    IPC_DLU_TESTER,
    "upgrade pifmgr version WORD",
    "upgrade",
    "pifmgr",
    "version",
    "upgrade version")
{
  nnBufferT apiBuff, msgBuff;
  Int8T rcvMsg[4096] = {0,};
  Uint32T rcvLength = 0;
  Int32T cmdFd = 0, cmdKey = 0, cmdResult = 0, msgLen = 0;
  Int8T result[64] = {0,};

  nnBufferReset(&apiBuff);
  nnBufferReset(&msgBuff);

  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt8T  (&apiBuff, strlen(cargv[3]));
  nnBufferSetString (&apiBuff, cargv[3], strlen(cargv[3]));

  ipcSendSync(PORT_INTERFACE_MANAGER,
              IPC_DYNAMIC_UPGRADE_TEST_SEND,
              IPC_DYNAMIC_UPGRADE_TEST_RESPONSE,
              apiBuff.data, apiBuff.length, rcvMsg, &rcvLength);

  nnBufferAssign(&msgBuff, rcvMsg, rcvLength);

  cmdFd = nnBufferGetInt32T(&msgBuff);
  cmdKey = nnBufferGetInt32T(&msgBuff);
  cmdResult = nnBufferGetInt32T(&msgBuff);
  msgLen = nnBufferGetInt8T(&msgBuff);
  nnBufferGetString(&msgBuff, result, msgLen);

  NNLOG(LOG_DEBUG, "cmdFd : %d, cmdKey : %d, cmdResult : %d\n",
                    cmdFd, cmdKey, cmdResult);

  if(cmdResult == SUCCESS)
  {
    cmdPrint(cmsh, "%s is successfully updated to version %s\n",
                    cargv[1], cargv[3]);
  }
  else
  {
    cmdPrint(cmsh, "Failure %s updated to version %s\n",
                    cargv[1], cargv[3]);
  }

  cmdPrint(cmsh,"End");
  ipcProcPendingMsg();

  return CMD_IPC_OK;
}


// RIB Manager Upgrade
DECMD(cmdDluTestFuncUpgradeRibmgr,
    CMD_NODE_EXEC,
    IPC_DLU_TESTER,
    "upgrade ribmgr version WORD",
    "upgrade",
    "ribmgr",
    "version",
    "upgrade version")
{
  nnBufferT apiBuff, msgBuff;
  Int8T rcvMsg[4096] = {0,};
  Uint32T rcvLength = 0;
  Int32T cmdFd = 0, cmdKey = 0, cmdResult = 0, msgLen = 0;
  Int8T result[64] = {0,};

  nnBufferReset(&apiBuff);
  nnBufferReset(&msgBuff);

  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt8T  (&apiBuff, strlen(cargv[3]));
  nnBufferSetString (&apiBuff, cargv[3], strlen(cargv[3]));

  ipcSendSync(RIB_MANAGER,
              IPC_DYNAMIC_UPGRADE_TEST_SEND,
              IPC_DYNAMIC_UPGRADE_TEST_RESPONSE,
              apiBuff.data, apiBuff.length, rcvMsg, &rcvLength);

  nnBufferAssign(&msgBuff, rcvMsg, rcvLength);

  cmdFd = nnBufferGetInt32T(&msgBuff);
  cmdKey = nnBufferGetInt32T(&msgBuff);
  cmdResult = nnBufferGetInt32T(&msgBuff);
  msgLen = nnBufferGetInt8T(&msgBuff);
  nnBufferGetString(&msgBuff, result, msgLen);

  NNLOG(LOG_DEBUG, "cmdFd : %d, cmdKey : %d, cmdResult : %d\n",
                    cmdFd, cmdKey, cmdResult);

  if(cmdResult == SUCCESS)
  {
    cmdPrint(cmsh, "%s is successfully updated to version %s\n",
                    cargv[1], cargv[3]);
  }
  else
  {
    cmdPrint(cmsh, "Failure %s updated to version %s\n",
                    cargv[1], cargv[3]);
  }

  cmdPrint(cmsh,"End");
  ipcProcPendingMsg();

  return CMD_IPC_OK;
}


// POLICY Manager Upgrade
DECMD(cmdDluTestFuncUpgradePolmgr,
    CMD_NODE_EXEC,
    IPC_DLU_TESTER,
    "upgrade polmgr version WORD",
    "upgrade",
    "polmgr",
    "version",
    "upgrade version")
{
  nnBufferT apiBuff, msgBuff;
  Int8T rcvMsg[4096] = {0,};
  Uint32T rcvLength = 0;
  Int32T cmdFd = 0, cmdKey = 0, cmdResult = 0, msgLen = 0;
  Int8T result[64] = {0,};

  nnBufferReset(&apiBuff);
  nnBufferReset(&msgBuff);

  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt8T  (&apiBuff, strlen(cargv[3]));
  nnBufferSetString (&apiBuff, cargv[3], strlen(cargv[3]));

  ipcSendSync(POLICY_MANAGER,
              IPC_DYNAMIC_UPGRADE_TEST_SEND,
              IPC_DYNAMIC_UPGRADE_TEST_RESPONSE,
              apiBuff.data, apiBuff.length, rcvMsg, &rcvLength);

  nnBufferAssign(&msgBuff, rcvMsg, rcvLength);

  cmdFd = nnBufferGetInt32T(&msgBuff);
  cmdKey = nnBufferGetInt32T(&msgBuff);
  cmdResult = nnBufferGetInt32T(&msgBuff);
  msgLen = nnBufferGetInt8T(&msgBuff);
  nnBufferGetString(&msgBuff, result, msgLen);

  NNLOG(LOG_DEBUG, "cmdFd : %d, cmdKey : %d, cmdResult : %d\n",
                    cmdFd, cmdKey, cmdResult);

  if(cmdResult == SUCCESS)
  {
    cmdPrint(cmsh, "%s is successfully updated to version %s\n",
                    cargv[1], cargv[3]);
  }
  else
  {
    cmdPrint(cmsh, "Failure %s updated to version %s\n",
                    cargv[1], cargv[3]);
  }

  cmdPrint(cmsh,"End");
  ipcProcPendingMsg();

  return CMD_IPC_OK;
}


// Command Manager Upgrade
DECMD(cmdDluTestFuncUpgradeCmd,
    CMD_NODE_EXEC,
    IPC_DLU_TESTER,
    "upgrade cmd version WORD",
    "upgrade",
    "cmd",
    "version",
    "upgrade version")
{
  nnBufferT apiBuff, msgBuff;
  Int8T rcvMsg[4096] = {0,};
  Uint32T rcvLength = 0;
  Int32T cmdFd = 0, cmdKey = 0, cmdResult = 0, msgLen = 0;
  Int8T result[64] = {0,};

  nnBufferReset(&apiBuff);
  nnBufferReset(&msgBuff);

  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt8T  (&apiBuff, strlen(cargv[3]));
  nnBufferSetString (&apiBuff, cargv[3], strlen(cargv[3]));

  ipcSendSync(COMMAND_MANAGER,
              IPC_DYNAMIC_UPGRADE_TEST_SEND,
              IPC_DYNAMIC_UPGRADE_TEST_RESPONSE,
              apiBuff.data, apiBuff.length, rcvMsg, &rcvLength);

  nnBufferAssign(&msgBuff, rcvMsg, rcvLength);

  cmdFd = nnBufferGetInt32T(&msgBuff);
  cmdKey = nnBufferGetInt32T(&msgBuff);
  cmdResult = nnBufferGetInt32T(&msgBuff);
  msgLen = nnBufferGetInt8T(&msgBuff);
  nnBufferGetString(&msgBuff, result, msgLen);

  NNLOG(LOG_DEBUG, "cmdFd : %d, cmdKey : %d, cmdResult : %d\n",
                    cmdFd, cmdKey, cmdResult);

  if(cmdResult == SUCCESS)
  {
    cmdPrint(cmsh, "%s is successfully updated to version %s\n",
                    cargv[1], cargv[3]);
  }
  else
  {
    cmdPrint(cmsh, "Failure %s updated to version %s\n",
                    cargv[1], cargv[3]);
  }

  cmdPrint(cmsh,"End");
  ipcProcPendingMsg();

  return CMD_IPC_OK;
}

// LACP Upgrade
DECMD(cmdDluTestFuncUpgradeLacp,
    CMD_NODE_EXEC,
    IPC_DLU_TESTER,
    "upgrade lacp version WORD",
    "upgrade",
    "lacp",
    "version",
    "upgrade version")
{
  nnBufferT apiBuff, msgBuff;
  Int8T rcvMsg[4096] = {0,};
  Uint32T rcvLength = 0;
  Int32T cmdFd = 0, cmdKey = 0, cmdResult = 0, msgLen = 0;
  Int8T result[64] = {0,};

  nnBufferReset(&apiBuff);
  nnBufferReset(&msgBuff);

  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt8T  (&apiBuff, strlen(cargv[3]));
  nnBufferSetString (&apiBuff, cargv[3], strlen(cargv[3]));

  ipcSendSync(LACP,
              IPC_DYNAMIC_UPGRADE_TEST_SEND,
              IPC_DYNAMIC_UPGRADE_TEST_RESPONSE,
              apiBuff.data, apiBuff.length, rcvMsg, &rcvLength);

  nnBufferAssign(&msgBuff, rcvMsg, rcvLength);

  cmdFd = nnBufferGetInt32T(&msgBuff);
  cmdKey = nnBufferGetInt32T(&msgBuff);
  cmdResult = nnBufferGetInt32T(&msgBuff);
  msgLen = nnBufferGetInt8T(&msgBuff);
  nnBufferGetString(&msgBuff, result, msgLen);

  NNLOG(LOG_DEBUG, "cmdFd : %d, cmdKey : %d, cmdResult : %d\n",
                    cmdFd, cmdKey, cmdResult);

  if(cmdResult == SUCCESS)
  {
    cmdPrint(cmsh, "%s is successfully updated to version %s\n",
                    cargv[1], cargv[3]);
  }
  else
  {
    cmdPrint(cmsh, "Failure %s updated to version %s\n",
                    cargv[1], cargv[3]);
  }

  cmdPrint(cmsh,"End");
  ipcProcPendingMsg();

  return CMD_IPC_OK;
}


// MSTP Upgrade
DECMD(cmdDluTestFuncUpgradeMstp,
    CMD_NODE_EXEC,
    IPC_DLU_TESTER,
    "upgrade mstp version WORD",
    "upgrade",
    "mstp",
    "version",
    "upgrade version")
{
  nnBufferT apiBuff, msgBuff;
  Int8T rcvMsg[4096] = {0,};
  Uint32T rcvLength = 0;
  Int32T cmdFd = 0, cmdKey = 0, cmdResult = 0, msgLen = 0;
  Int8T result[64] = {0,};

  nnBufferReset(&apiBuff);
  nnBufferReset(&msgBuff);

  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt8T  (&apiBuff, strlen(cargv[3]));
  nnBufferSetString (&apiBuff, cargv[3], strlen(cargv[3]));

  ipcSendSync(MSTP,
              IPC_DYNAMIC_UPGRADE_TEST_SEND,
              IPC_DYNAMIC_UPGRADE_TEST_RESPONSE,
              apiBuff.data, apiBuff.length, rcvMsg, &rcvLength);

  nnBufferAssign(&msgBuff, rcvMsg, rcvLength);

  cmdFd = nnBufferGetInt32T(&msgBuff);
  cmdKey = nnBufferGetInt32T(&msgBuff);
  cmdResult = nnBufferGetInt32T(&msgBuff);
  msgLen = nnBufferGetInt8T(&msgBuff);
  nnBufferGetString(&msgBuff, result, msgLen);

  NNLOG(LOG_DEBUG, "cmdFd : %d, cmdKey : %d, cmdResult : %d\n",
                    cmdFd, cmdKey, cmdResult);

  if(cmdResult == SUCCESS)
  {
    cmdPrint(cmsh, "%s is successfully updated to version %s\n",
                    cargv[1], cargv[3]);
  }
  else
  {
    cmdPrint(cmsh, "Failure %s updated to version %s\n",
                    cargv[1], cargv[3]);
  }

  cmdPrint(cmsh,"End");
  ipcProcPendingMsg();

  return CMD_IPC_OK;
}

// RIP Upgrade
DECMD(cmdDluTestFuncUpgradeRip,
    CMD_NODE_EXEC,
    IPC_DLU_TESTER,
    "upgrade rip version WORD",
    "upgrade",
    "rip",
    "version",
    "upgrade version")
{
  nnBufferT apiBuff, msgBuff;
  Int8T rcvMsg[4096] = {0,};
  Uint32T rcvLength = 0;
  Int32T cmdFd = 0, cmdKey = 0, cmdResult = 0, msgLen = 0;
  Int8T result[64] = {0,};

  nnBufferReset(&apiBuff);
  nnBufferReset(&msgBuff);

  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt8T  (&apiBuff, strlen(cargv[3]));
  nnBufferSetString (&apiBuff, cargv[3], strlen(cargv[3]));

  ipcSendSync(RIP,
              IPC_DYNAMIC_UPGRADE_TEST_SEND,
              IPC_DYNAMIC_UPGRADE_TEST_RESPONSE,
              apiBuff.data, apiBuff.length, rcvMsg, &rcvLength);

  nnBufferAssign(&msgBuff, rcvMsg, rcvLength);

  cmdFd = nnBufferGetInt32T(&msgBuff);
  cmdKey = nnBufferGetInt32T(&msgBuff);
  cmdResult = nnBufferGetInt32T(&msgBuff);
  msgLen = nnBufferGetInt8T(&msgBuff);
  nnBufferGetString(&msgBuff, result, msgLen);

  NNLOG(LOG_DEBUG, "cmdFd : %d, cmdKey : %d, cmdResult : %d\n",
                    cmdFd, cmdKey, cmdResult);

  if(cmdResult == SUCCESS)
  {
    cmdPrint(cmsh, "%s is successfully updated to version %s\n",
                    cargv[1], cargv[3]);
  }
  else
  {
    cmdPrint(cmsh, "Failure %s updated to version %s\n",
                    cargv[1], cargv[3]);
  }

  cmdPrint(cmsh,"End");
  ipcProcPendingMsg();

  return CMD_IPC_OK;
}

// ISIS Upgrade
DECMD(cmdDluTestFuncUpgradeIsis,
    CMD_NODE_EXEC,
    IPC_DLU_TESTER,
    "upgrade isis version WORD",
    "upgrade",
    "isis",
    "version",
    "upgrade version")
{
  nnBufferT apiBuff, msgBuff;
  Int8T rcvMsg[4096] = {0,};
  Uint32T rcvLength = 0;
  Int32T cmdFd = 0, cmdKey = 0, cmdResult = 0, msgLen = 0;
  Int8T result[64] = {0,};

  nnBufferReset(&apiBuff);
  nnBufferReset(&msgBuff);

  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt8T  (&apiBuff, strlen(cargv[3]));
  nnBufferSetString (&apiBuff, cargv[3], strlen(cargv[3]));

  ipcSendSync(ISIS,
              IPC_DYNAMIC_UPGRADE_TEST_SEND,
              IPC_DYNAMIC_UPGRADE_TEST_RESPONSE,
              apiBuff.data, apiBuff.length, rcvMsg, &rcvLength);

  nnBufferAssign(&msgBuff, rcvMsg, rcvLength);

  cmdFd = nnBufferGetInt32T(&msgBuff);
  cmdKey = nnBufferGetInt32T(&msgBuff);
  cmdResult = nnBufferGetInt32T(&msgBuff);
  msgLen = nnBufferGetInt8T(&msgBuff);
  nnBufferGetString(&msgBuff, result, msgLen);

  NNLOG(LOG_DEBUG, "cmdFd : %d, cmdKey : %d, cmdResult : %d\n",
                    cmdFd, cmdKey, cmdResult);

  if(cmdResult == SUCCESS)
  {
    cmdPrint(cmsh, "%s is successfully updated to version %s\n",
                    cargv[1], cargv[3]);
  }
  else
  {
    cmdPrint(cmsh, "Failure %s updated to version %s\n",
                    cargv[1], cargv[3]);
  }

  cmdPrint(cmsh,"End");
  ipcProcPendingMsg();

  return CMD_IPC_OK;
}

// OSPF Upgrade
DECMD(cmdDluTestFuncUpgradeOspf,
    CMD_NODE_EXEC,
    IPC_DLU_TESTER,
    "upgrade ospf version WORD",
    "upgrade",
    "ospf",
    "version",
    "upgrade version")
{
  nnBufferT apiBuff, msgBuff;
  Int8T rcvMsg[4096] = {0,};
  Uint32T rcvLength = 0;
  Int32T cmdFd = 0, cmdKey = 0, cmdResult = 0, msgLen = 0;
  Int8T result[64] = {0,};

  nnBufferReset(&apiBuff);
  nnBufferReset(&msgBuff);

  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt32T (&apiBuff, 0);
  nnBufferSetInt8T  (&apiBuff, strlen(cargv[3]));
  nnBufferSetString (&apiBuff, cargv[3], strlen(cargv[3]));

  ipcSendSync(OSPF,
              IPC_DYNAMIC_UPGRADE_TEST_SEND,
              IPC_DYNAMIC_UPGRADE_TEST_RESPONSE,
              apiBuff.data, apiBuff.length, rcvMsg, &rcvLength);

  nnBufferAssign(&msgBuff, rcvMsg, rcvLength);

  cmdFd = nnBufferGetInt32T(&msgBuff);
  cmdKey = nnBufferGetInt32T(&msgBuff);
  cmdResult = nnBufferGetInt32T(&msgBuff);
  msgLen = nnBufferGetInt8T(&msgBuff);
  nnBufferGetString(&msgBuff, result, msgLen);

  NNLOG(LOG_DEBUG, "cmdFd : %d, cmdKey : %d, cmdResult : %d\n",
                    cmdFd, cmdKey, cmdResult);

  if(cmdResult == SUCCESS)
  {
    cmdPrint(cmsh, "%s is successfully updated to version %s\n",
                    cargv[1], cargv[3]);
  }
  else
  {
    cmdPrint(cmsh, "Failure %s updated to version %s\n",
                    cargv[1], cargv[3]);
  }

  cmdPrint(cmsh,"End");
  ipcProcPendingMsg();

  return CMD_IPC_OK;
}
