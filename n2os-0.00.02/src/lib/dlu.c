/*******************************************************************************
 *            Electronics and Telecommunications Research Institute
 * Copyright: 2013 Electronics and Telecommunications Research Institute.
 *            All rights reserved.
 *            No part of this software shall be reproduced, stored in a
 *            retrieval system, or transmitted by any means, electronic,
 *            mechanical, photocopying, recording, or otherwise, without
 *            written permission from ETRI.
*******************************************************************************/

/**
 * @brief : Dynamic Live Update
 *  - Block Name : 
 *  - Process Name :
 *  - Creator : 
 *  - Initial Date :
*/

/**
 * @file : 
 *
 * $Id:
 * $Author:
 * $Date:
 * $Revision:
 * $LastChangedBy:
 **/


/*******************************************************************************
 *                        CONSTANTS / LITERALS / TYPES
 ******************************************************************************/

#include <unistd.h>
#include <dlfcn.h>
#include "nnLog.h"
#include "nnMemmgr.h"
#include "taskManager.h"
#include "dlu.h"
#include "nnBuffer.h"
#include "nnCmdCommon.h"
#include "nnCmdDefines.h"
#include "compStatic.h"

#ifdef IS_PROCMGR
  #include "lcsService.h"
  #include "nnCmdMsg.h"
#endif 

//#include "nosSyslib.h"

/*******************************************************************************
 *                              GLOBAL VARIABLES
 ******************************************************************************/

#define COMPONENT_INIT       0
#define COMPONENT_TERM       1
#define COMPONENT_RESTART    2
#define COMPONENT_HOLD       3
#define COMPONENT_IPC        4
#define COMPONENT_EVENT      5
#define COMPONENT_SIGNAL     6
#define NOS_TASK_LIB_MAP     7
#define NOS_TASK_LIB_INIT    8
#define NOS_TASK_SCHEDULE    9
#define CMIPC_CMSH_FUNC_CB   10
#define CMIPC_COMP_FUNC_CB   11

#define DYNAMC_FUNCTION_SIZE        12
#define DYNAMC_FUNCTION_NAME_SIZE   48
#define DYNAMC_CHECK_TIME	        2
#define DYNAMC_RESULT_SIZE          1024
#define COMP_NAME_SIZE	            64
#define COMP_VER_SIZE				64
#define DYNAMC_LIBRARY_NAME         "./lib%s_%s.so"

/* NOS task global variabls */
Int32T gNosCompName = 0;
Int8T gNosCompString[COMP_NAME_SIZE];
Int8T gNosCompVersion[COMP_VER_SIZE];
void*  gNosCompData = NULL;
Int32T gCmCmshID = -1;

/*******************************************************************************
 *                         GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 *                         LOCAL CONSTANTS/LITERALS/TYPES
 ******************************************************************************/


/*******************************************************************************
 *                               LOCAL VARIABLES
 ******************************************************************************/

/* NOS component module handle */
static Int8T sCompModuleName[COMP_NAME_SIZE];
static void *sCurrentModuleHandle = NULL;

typedef void   (*VOID)    ();
typedef void   (*INIT)    ();
typedef void   (*TERM)    ();
typedef void   (*RESTART) ();
typedef void   (*IPC)     (Uint32T messageId, void * message, Uint32T size);
typedef void   (*EVENT)   (Uint32T messageId, void * message, Uint32T size);
typedef void   (*SIGNAL)  (Uint32T sig);
typedef Int32T (*LIBMAP)  (NosServiceT* nosLib, void** gData, Int32T* compName);
typedef Int32T (*LIBINIT) ();
typedef Int32T (*SCHEDULE)();
typedef void   (*CMIPC)   (Int32T sockId, void *message, Uint32T size);

struct NoSDynamicFunction 
{
  Int32T seq;
  Int32T funcId;
  Int8T  funcName[DYNAMC_FUNCTION_NAME_SIZE];
  void (*func)(void);
} sDynamicFunction[DYNAMC_FUNCTION_SIZE] __attribute__ ((unused)) =
{
  {0, COMPONENT_INIT,     "componentInit",          NULL    },
  {1, COMPONENT_TERM,     "componentTerm",          NULL    },
  {2, COMPONENT_RESTART,  "componentRestart",       NULL    },
  {3, COMPONENT_HOLD,     "componentHold",          NULL    },
  {4, COMPONENT_IPC,      "componentIpc",           NULL    },
  {5, COMPONENT_EVENT,    "componentEvent",         NULL    },
  {6, COMPONENT_SIGNAL,   "componentSignal",        NULL    },
  {7, NOS_TASK_LIB_MAP,   "nosTaskLibMap",          NULL    },
  {8, NOS_TASK_LIB_INIT,  "nosTaskLibInit",         NULL    },
  {9, NOS_TASK_SCHEDULE,  "nosTaskSchedule",        NULL    },
  {10,CMIPC_CMSH_FUNC_CB, "componentCmCmshProcess", NULL    },
  {11,CMIPC_COMP_FUNC_CB, "componentCmCompProcess", NULL    },
};

/* NOS library service handle */
NosServiceT nosService __attribute__ ((unused)) =
{
  // Task Scheduling Service
  { _taskFdSet, _taskFdDel, 
    _taskTimerSet, _taskTimerDel, 
    _taskFdUpdate, _taskTimerUpdate,
    _taskSignalSetAggr, _taskSignalDelAggr, _taskSignalUpdateAggr,
    _taskSignalSet, _taskSignalDel, _taskSignalUpdate,
    _taskClose}, 

  // IPC Service
  { _ipcChannelOpen, _ipcSendAsync, _ipcSendSync, 
    _ipcResponseSync, _ipcProcessingPendingMsg, _ipcChannelUpdate,
    _ipcProcessInfoGet},

  // Event Service
  { _eventOpen, _eventSubscribe, _eventUnsubscribe, _eventPublish,
    _eventChannelUpdate, _eventSubscribeSend},

  // Log Service
  { _nnLogInit, _nnLogClose, _nnLogSetFlag, _nnLogUnsetFlag,
    _nnLogSetFile, _nnLogSetPriority, _nnLogLogging, _nnLogLoggingDebug},

  // Memory Management Service
  { _memInit, _memClose, _memSetDebug, _memUnsetDebug, _memSetFile, 
    _memUnsetFile, _memShowAllUser, _memMalloc, _memRealloc, _memFree}
};

void nosCmdInitialize(Int32T cmshID);
void nosTaskModuleExit(void);
void nosCmshProcess(Int32T sockId, void *message, Uint32T size);
void nosCompProcess(Int32T sockId, void *message, Uint32T size);
Int32T nosTaskModuleUpdate(Int32T msgId, void* data, Int32T dataLen);
void nosTaskModuleVersionSend(void);

/*******************************************************************************
 *                                   CODE
 ******************************************************************************/

void nosCompInitialize()
{
  static Int32T initCount = 0;

  if(initCount == 0)
  {
    // nosCmdInitialize() check
    if(gCmCmshID != -1)
    {
      // cmdInitialize() call
      if(gNosCompName == COMMAND_MANAGER)
      {
        nosCmCmdInit
        (gCmCmshID, IPC_CM_MGR, nosCmshProcess, nosCompProcess);
      }
      else
      {
        // Process Manager and IPC Manager is gCmCmshId == -1
        nosCompCmdInit
          (gCmCmshID, IPC_CM_MGR, nosCmshProcess, nosCompProcess);
      }
    }  

    //request to schedule task (event_interests)
    SCHEDULE schFunc = (SCHEDULE)sDynamicFunction[NOS_TASK_SCHEDULE].func;
    if(schFunc() < 0)
    {
      nLOG(LOG_ERR, "fail to Event subscribe task \n");
      exit(-1);
    }

    // Component Initialize call
    INIT tmpFunc = (INIT)sDynamicFunction[COMPONENT_INIT].func;
    tmpFunc();

    if(gNosCompName != IPC_MANAGER)
    {
        nosTaskModuleVersionSend();
    }
    initCount++;

  }
  else
  {
    //request to schedule task (event_interests)
    SCHEDULE schFunc = (SCHEDULE)sDynamicFunction[NOS_TASK_SCHEDULE].func;
    if(schFunc() < 0)
    {
      nLOG(LOG_ERR, "fail to Event subscribe task \n");
      exit(-1);
    }
    nLOG(LOG_INFO, "request to schedule task (event_interests) Complete \n"); 
  }
}

void nosCompTerminate()
{ 
  TERM tmpFunc = (TERM)sDynamicFunction[COMPONENT_TERM].func;
  tmpFunc();

  if(gNosCompName != PROCESS_MANAGER)
  {
    nosTaskModuleExit();
  }
}

void nosCompRestart()
{ 
  RESTART tmpFunc = (RESTART)sDynamicFunction[COMPONENT_RESTART].func;
  tmpFunc();
}

void nosCompHold()
{ 
  RESTART tmpFunc = (RESTART)sDynamicFunction[COMPONENT_HOLD].func;
  tmpFunc();
}

void nosCompIpcProcess(Int32T msgId, void* data, Uint32T dataLen)
{ 
  if((msgId == IPC_LCS_PM2C_DYNAMIC_UPGRADE) ||
     (msgId == IPC_DYNAMIC_UPGRADE_TEST_SEND))
  {
    if(nosTaskModuleUpdate(msgId, data, dataLen) == DLU_TASK_FAIL)
    {
      nosCompTerminate();
    }

    return;
  }

#ifdef IS_IPCMGR
  if(msgId == IPC_LCS_PM2C_COMMAND_MANAGER_UP)
  {
    nosCmdInitialize(IPC_IPC_MGR);
    nosCompCmdInit(gCmCmshID, IPC_CM_MGR, nosCmshProcess, nosCompProcess);
  }
  if(msgId == IPC_DYNAMIC_UPGRADE_VERSION)
  {
    nosTaskModuleVersionSend();
    return ;
  }
#endif

#ifdef IS_PROCMGR
  if(msgId == IPC_LCS_C2PM_REGISTER)
  {
    Int32T processType = 0;
    memcpy(&processType, data, sizeof(Int32T));

    if(processType == LCS_COMMAND_MANAGER)
    {
      nosCmdInitialize(IPC_PRO_MGR);
      nosCompCmdInit(gCmCmshID, IPC_CM_MGR, nosCmshProcess, nosCompProcess); 
    }
  }
#endif

  IPC tmpFunc = (IPC)sDynamicFunction[COMPONENT_IPC].func;
  tmpFunc(msgId, data, dataLen);
}

void nosCompEventProcess(Int32T msgId, void* data, Uint32T dataLen)
{ 
  EVENT tmpFunc = (EVENT)sDynamicFunction[COMPONENT_EVENT].func;
  tmpFunc(msgId, data, dataLen);
}

void nosCompSignalProcess(Int32T sig)
{ 
  SIGNAL tmpFunc = (SIGNAL)sDynamicFunction[COMPONENT_SIGNAL].func;
  tmpFunc(sig);

  if((sig == SIGINT)  ||
     (sig == SIGTERM) ||
     (sig == SIGHUP)  ||
     (sig == SIGUSR1) ||
     (sig == SIGSEGV))
  {
    if(gNosCompName != PROCESS_MANAGER)
    {
      nosTaskModuleExit();
    }
  }
}

void nosCmshProcess(Int32T sockId, void *message, Uint32T size)
{
  CMIPC tmpFunc = (CMIPC)sDynamicFunction[CMIPC_CMSH_FUNC_CB].func;
  tmpFunc(sockId, message, size);
}

#ifdef IS_PROCMGR
void nosProcmgrTaskModuleUpdate (StringT compVer)
{
  nnBufferT msgBuff;
  nnBufferReset(&msgBuff);

  nnBufferSetInt32T(&msgBuff, 0);
  nnBufferSetInt32T(&msgBuff, 0);
  nnBufferSetInt32T(&msgBuff, 0);
  nnBufferSetInt8T(&msgBuff, strlen(compVer));
  nnBufferSetString(&msgBuff, compVer, strlen(compVer));

  if(nosTaskModuleUpdate 
     (IPC_LCS_PM2C_DYNAMIC_UPGRADE, &msgBuff.data, msgBuff.length) 
     == DLU_TASK_FAIL)
  {
    nosCompTerminate();
  }
}

void nosProcmgrDluCheck(void *message)
{
  Int32T funcID, funcIndex, sSize, cArgc = 0;
  Int8T *szBuffer = (Int8T *)message;
  cmdIpcMsg_T cmIpcHdr;
  Int8T cArgv[128][256];
    
  cmdIpcUnpackHdr(szBuffer, &cmIpcHdr);
    
  if(cmIpcHdr.code == CMD_IPC_CODE_FUNC_REQ)
  {
    cmdIpcUnpackTlv(szBuffer, CMD_IPC_TLV_CALLBACK_INDEX, &sSize, &funcIndex);
    cmdIpcUnpackTlv(szBuffer, CMD_IPC_TLV_CALLBACK_FUNC, &sSize, &funcID);

    while(cmdIpcUnpackTlv
          (szBuffer, CMD_IPC_TLV_CALLBACK_ARGSTR, &sSize, cArgv[cArgc])
          == CMD_IPC_OK)
    {
      cArgc++;
    }

    if((strcmp(cArgv[0], "liveupdate") == SUCCESS) &&
       (strcmp(cArgv[1], "procmgrd") == SUCCESS))
    {
      nosProcmgrTaskModuleUpdate(cArgv[2]);
    }

  }

  return ;
}
#endif

void nosCompProcess(Int32T sockId, void *message, Uint32T size)
{
#ifdef IS_PROCMGR
  Int8T szBuffer[1024] = {0, };
  memcpy(szBuffer, message, sizeof(szBuffer));
  nosProcmgrDluCheck(szBuffer);
#endif

  CMIPC tmpFunc = (CMIPC)sDynamicFunction[CMIPC_COMP_FUNC_CB].func;
  tmpFunc(sockId, message, size);
}

void nosTaskLibInitialize(char* compName, char* compVer)
{
  //make component .so name
  Int8T tempVer[COMP_VER_SIZE] = {0, };

  strncpy(gNosCompString, compName, strlen(compName));
  strncpy(tempVer, compVer, strlen(compVer));
  strncpy(gNosCompVersion, compVer, strlen(compVer));
  sprintf(sCompModuleName, DYNAMC_LIBRARY_NAME, gNosCompString, tempVer);

  fprintf(stdout, "NOS : open module (%s) \n",sCompModuleName);

  //open dynamic component module
  sCurrentModuleHandle = dlopen(sCompModuleName, RTLD_LAZY);
  if(!sCurrentModuleHandle)
  {
    fprintf(stderr, "NOS : fail to open module: %s(%s) \n"
                  , sCompModuleName, dlerror());
    exit(-1);
  }

  //resolve nos component function symbols
  Int32T i;
  for(i = 0; i < DYNAMC_FUNCTION_SIZE; i++)
  {
    sDynamicFunction[i].func = 
        (VOID)dlsym(sCurrentModuleHandle, sDynamicFunction[i].funcName);

    if(sDynamicFunction[i].func == NULL)
    {
      fprintf(stderr, "NOS : fail to resolve symbol: %s \n"
                    , sDynamicFunction[i].funcName);
      exit(-1);
    }
  }

  //resolve nos library function symbols & get component name
  LIBMAP mapFunc = (LIBMAP)sDynamicFunction[NOS_TASK_LIB_MAP].func;
  if((mapFunc(&nosService, &gNosCompData, &gNosCompName)) < 0)
  {
    fprintf(stderr, "NOS : fail to resolve nos library \n");
    exit(-1);
  }

  //request to initialize nos task library(mem & log)
  LIBINIT libInitFunc = (LIBINIT)sDynamicFunction[NOS_TASK_LIB_INIT].func;
  if(libInitFunc() < 0)
  {
    fprintf(stderr, "NOS : fail to initialize nos library (mem, log) \n");
    exit(-1);
  }

  nLOG(LOG_INFO, "Library function symbols resolve, library init success\n");

#if 0
  // nos data library set
  printf("sCurrentModuleHandle : %d\n", sCurrentModuleHandle);
  nosSharedHandleInit(sCurrentModuleHandle);
#endif

}

Int32T nosTaskScheduleInitialize() 
{ 
  //register receiving ipc process
  if(_ipcChannelOpen(gNosCompName, (void*)nosCompIpcProcess) != SUCCESS)
  {
    nLOG(LOG_ERR, "NOS_DYNAMIC: fail to open ipc channel \n");
    exit(-1);
  } 

  //register receiving event process
  if(_eventOpen((void*)nosCompEventProcess) != SUCCESS)
  {
    nLOG(LOG_ERR, "fail to open event channel \n");
    exit(-1);
  }

  //register system signal catch process
  if(_taskSignalSetAggr(nosCompSignalProcess) != SUCCESS)
  {
    nLOG(LOG_ERR, "fail to signal set aggr \n");
    exit(-1);
  }

#if 0
  //request to schedule task (event_interests)
  SCHEDULE schFunc = (SCHEDULE)sDynamicFunction[NOS_TASK_SCHEDULE].func;
  if(schFunc() < 0)
  {
    nLOG(LOG_ERR, "fail to Event subscribe task \n");
    exit(-1);
  }
#endif

  nLOG(LOG_INFO, "Nos Task Schedule Init Success \n", __FUNCTION__);

  return DLU_TASK_OK;
}

#ifdef IS_IPCMGR
extern void ipcmgrChannelOpen(void*);
Int32T nosIpcmgrScheduleInitialize()
{
  ipcmgrChannelOpen((void*)nosCompIpcProcess);

  //register receiving event process
  if(_eventOpen((void*)nosCompEventProcess) != SUCCESS)
  {
    nLOG(LOG_ERR, "fail to open event channel \n");
    exit(-1);
  }

  //register system signal catch process
  if(_taskSignalSetAggr(nosCompSignalProcess) != SUCCESS)
  {
    nLOG(LOG_ERR, "fail to signal set aggr \n");
    exit(-1);
  }

  nLOG(LOG_INFO, "Nos Task Schedule Init Success \n", __FUNCTION__);

  return DLU_TASK_OK;

}
#endif

void nosCmdInitialize(Int32T cmshID)
{
  gCmCmshID = cmshID;
}

void nosTaskModuleExit(void)
{
  printf("%s : %s\n", __func__, sCompModuleName);
  dlclose(sCurrentModuleHandle);
}

//Currently, test version nosTaskModuleUpdate
Int32T nosTaskModuleUpdate(Int32T msgId, void* data, Int32T dataLen)
{
  Int32T cmdFd, cmdKey, cmdResult;
  Int8T versionLen;
  Int8T compVer[COMP_VER_SIZE] = {0, };
  Int8T tempModuleName[COMP_NAME_SIZE] = {0, };
  Int8T cmdReason[DYNAMC_RESULT_SIZE] = {0, };
  
  // dlu message set
  nnBufferT msgBuff;
  nnBufferReset(&msgBuff);
  nnBufferAssign(&msgBuff, data, dataLen);

  cmdFd = nnBufferGetInt32T(&msgBuff);
  cmdKey = nnBufferGetInt32T(&msgBuff);
  cmdResult = nnBufferGetInt32T(&msgBuff);
  versionLen = nnBufferGetInt8T(&msgBuff);
  nnBufferGetString(&msgBuff, compVer, versionLen);

  nLOG(LOG_DEBUG, "dlu message received\n");
  nLOG(LOG_DEBUG, 
       "cmdFd(%d), cmdKey(%d), cmdResult(%d), versionLen(%d), compVer(%s)\n",
        cmdFd, cmdKey, cmdResult, versionLen, compVer);

  cmdFd = htonl(cmdFd);
  cmdKey = htonl(cmdKey);
  nLOG(LOG_DEBUG, 
       "cmdFd(%d), cmdKey(%d), cmdResult(%d), versionLen(%d), compVer(%s)\n",
        cmdFd, cmdKey, cmdResult, versionLen, compVer);


  // response message set
  nnBufferReset(&msgBuff);
  nnBufferSetInt32T (&msgBuff, cmdFd); 
  nnBufferSetInt32T (&msgBuff, cmdKey); 
  cmdResult = ntohl(CMD_SHOW);
  nnBufferSetInt32T (&msgBuff, cmdResult);
    
  nLOG(LOG_INFO, 
       "Activating live upgrading. Old Module : %s\n", sCompModuleName);


  //check firstly to prevent unexpected error
  if(sCurrentModuleHandle == NULL) 
  {
    nLOG(LOG_ERR, "DLU : module handle error %s\n", sCompModuleName);

    // error message send
    sprintf(cmdReason, "DLU : Module handle error %s\n", sCompModuleName);
    cmdResult = ntohl(FAILURE);

    nnBufferReset (&msgBuff);
    nnBufferSetInt32T (&msgBuff, cmdFd);
    nnBufferSetInt32T (&msgBuff, cmdKey);
    nnBufferSetInt32T (&msgBuff, cmdResult);
    nnBufferSetInt8T (&msgBuff, strlen(cmdReason));
    nnBufferSetString(&msgBuff, cmdReason, strlen(cmdReason));

    if(msgId == IPC_LCS_PM2C_DYNAMIC_UPGRADE)
    {
      if(gNosCompName == PROCESS_MANAGER)
      {
        return DLU_TASK_FAIL;
      }

      _ipcResponseSync(PROCESS_MANAGER, 
                       IPC_LCS_PM2C_DYNAMIC_UPGRADE_RESPONSE, 
                       msgBuff.data,
                       msgBuff.length);
    }
    else
    {
      _ipcResponseSync(DLU_TESTER,
                       IPC_DYNAMIC_UPGRADE_TEST_RESPONSE,
                       msgBuff.data,
                       msgBuff.length);
    }

#if 0
        _ipcSendAsync(COMMAND_MANAGER, 0, msgBuff.data, msgBuff.length);

        cmdResult = ntohl(CMD_SHOW_END);
        nnBufferReset (&msgBuff);

        nnBufferSetInt32T (&msgBuff, cmdFd);
        nnBufferSetInt32T (&msgBuff, cmdKey);
        nnBufferSetInt32T (&msgBuff, cmdResult);
        _ipcSendAsync(COMMAND_MANAGER, 0, msgBuff.data, msgBuff.length);
#endif

        return DLU_TASK_FAIL;
    }

  // New Module Exist Check

  sprintf(tempModuleName, DYNAMC_LIBRARY_NAME, gNosCompString, compVer);
  nLOG(LOG_INFO, "New Module Exist Check : %s\n", tempModuleName);

  if(access(tempModuleName, F_OK) == -1)
  {
    nLOG(LOG_ERR, "DLU : New Module Not Exist : %s\n", tempModuleName);

    // error message send
    sprintf(cmdReason, "New Module(%s) Not Exist\n", tempModuleName);
    cmdResult = ntohl(FAILURE);

    nnBufferReset (&msgBuff);
    nnBufferSetInt32T (&msgBuff, cmdFd);
    nnBufferSetInt32T (&msgBuff, cmdKey);
    nnBufferSetInt32T (&msgBuff, cmdResult);
    nnBufferSetInt8T (&msgBuff, strlen(cmdReason));
    nnBufferSetString(&msgBuff, cmdReason, strlen(cmdReason));

    if(msgId == IPC_LCS_PM2C_DYNAMIC_UPGRADE)
    {
      if(gNosCompName == PROCESS_MANAGER)
      {
        IPC tmpFunc = (IPC)sDynamicFunction[COMPONENT_IPC].func;
        tmpFunc
        (IPC_LCS_PM2C_DYNAMIC_UPGRADE_RESPONSE, &msgBuff.data, msgBuff.length);

        return DLU_TASK_OK;        
      }

      _ipcResponseSync(PROCESS_MANAGER,
                       IPC_LCS_PM2C_DYNAMIC_UPGRADE_RESPONSE,
                       msgBuff.data,
                       msgBuff.length);
    }
    else
    {
      _ipcResponseSync(DLU_TESTER,
                       IPC_DYNAMIC_UPGRADE_TEST_RESPONSE,
                       msgBuff.data,
                       msgBuff.length);
    }

    return DLU_TASK_OK;
  }

  nLOG(LOG_INFO, "New Module Exist Check Success : %s\n", tempModuleName);


  //send hold signal to component
  nLOG(LOG_INFO, "hold component \n");
  nosCompHold();


  //unload old module
  nLOG(LOG_INFO, "unloading current module \n");
  dlclose(sCurrentModuleHandle);

  memset(tempModuleName, 0x00, COMP_NAME_SIZE);
  strncpy(tempModuleName, sCompModuleName, strlen(sCompModuleName));

  memset(sCompModuleName, 0x00, COMP_NAME_SIZE);
  sprintf(sCompModuleName, DYNAMC_LIBRARY_NAME, gNosCompString, compVer);

  nLOG(LOG_INFO, "Activating live upgrading : %s\n", sCompModuleName);


  //open dynamic component module
  sCurrentModuleHandle = dlopen(sCompModuleName, RTLD_LAZY);
  if(!sCurrentModuleHandle)
  {
    nLOG(LOG_ERR, "DLU : module open error %s\n", sCompModuleName);

#if 0
    // error message send
    sprintf(cmdReason, "DLU : Module open error %s\n", sCompModuleName);
    nnBufferSetString(&msgBuff, cmdReason, strlen(cmdReason));
    _ipcSendAsync(COMMAND_MANAGER, 0, msgBuff.data, msgBuff.length);
#endif

    // old module rollback (load)
    sprintf(cmdReason, "DLU : Module old module open error : %s\n",
                        sCompModuleName);

    memset(sCompModuleName, 0x00, COMP_NAME_SIZE);
    sprintf(sCompModuleName, "%s", tempModuleName);
    sCurrentModuleHandle = dlopen(sCompModuleName, RTLD_LAZY);

    if(!sCurrentModuleHandle)
    {
      nLOG(LOG_ERR, "module open error %s\n", sCompModuleName);

#if 0
            // error message send
            cmdResult = ntohl(CMD_SHOW);
            nnBufferReset(&msgBuff);
            nnBufferSetInt32T (&msgBuff, cmdFd); 
            nnBufferSetInt32T (&msgBuff, cmdKey); 
            nnBufferSetInt32T (&msgBuff, cmdResult);
            sprintf(cmdReason, "DLU : Module old module open error %s\n", 
                                sCompModuleName);
            nnBufferSetString(&msgBuff, cmdReason, strlen(cmdReason));
            _ipcSendAsync(COMMAND_MANAGER, 0, msgBuff.data, msgBuff.length);
 
            cmdResult = ntohl(CMD_SHOW_END);
            nnBufferReset (&msgBuff);
            nnBufferSetInt32T (&msgBuff, cmdFd);
            nnBufferSetInt32T (&msgBuff, cmdKey);
            nnBufferSetInt32T (&msgBuff, cmdResult);
            _ipcSendAsync(COMMAND_MANAGER, 0, msgBuff.data, msgBuff.length);
#endif

      sprintf(cmdReason, "%s\n DLU : Module old module reopen error %s\n",
                          cmdReason, sCompModuleName);
      cmdResult = ntohl(FAILURE);

      nnBufferReset (&msgBuff);
      nnBufferSetInt32T (&msgBuff, cmdFd);
      nnBufferSetInt32T (&msgBuff, cmdKey);
      nnBufferSetInt32T (&msgBuff, cmdResult);
      nnBufferSetInt8T (&msgBuff, strlen(cmdReason));
      nnBufferSetString(&msgBuff, cmdReason, strlen(cmdReason));

      if(msgId == IPC_LCS_PM2C_DYNAMIC_UPGRADE)
      {
        if(gNosCompName == PROCESS_MANAGER)
        {
          return DLU_TASK_FAIL;
        }

        _ipcResponseSync(PROCESS_MANAGER,
                         IPC_LCS_PM2C_DYNAMIC_UPGRADE_RESPONSE,
                         msgBuff.data,
                         msgBuff.length);
      }
      else
      {
        _ipcResponseSync(DLU_TESTER,
                         IPC_DYNAMIC_UPGRADE_TEST_RESPONSE,
                         msgBuff.data,
                         msgBuff.length);
      }

      return DLU_TASK_FAIL;
    }
    else
    {
      nLOG(LOG_INFO, "Reopen old module Success : %s\n", sCompModuleName);
#if 0
            nnBufferReset(&msgBuff);
            nnBufferSetInt32T (&msgBuff, cmdFd); 
            nnBufferSetInt32T (&msgBuff, cmdKey); 
            cmdResult = ntohl(CMD_SHOW);
            nnBufferSetInt32T (&msgBuff, cmdResult);
#endif

    }
  }

  //resolve nos component function symbols
  Int32T i;
  for(i = 0; i < DYNAMC_FUNCTION_SIZE; i++)
  {
    sDynamicFunction[i].func = 
      (VOID)dlsym(sCurrentModuleHandle, sDynamicFunction[i].funcName);
    if(sDynamicFunction[i].func != NULL)
    {

    }
    else
    {
      nLOG(LOG_ERR, "fail to resolve symbols %s\n",
                     sDynamicFunction[i].funcName);
#if 0
            // error message send
            sprintf(cmdReason, "DLU : Fail to resolve symbols\n");
            nnBufferSetString(&msgBuff, cmdReason, strlen(cmdReason));
            _ipcSendAsync(COMMAND_MANAGER, 0, msgBuff.data, msgBuff.length);

            cmdResult = ntohl(CMD_SHOW_END);
            nnBufferReset (&msgBuff);
            nnBufferSetInt32T (&msgBuff, cmdFd);
            nnBufferSetInt32T (&msgBuff, cmdKey);
            nnBufferSetInt32T (&msgBuff, cmdResult);
            _ipcSendAsync(COMMAND_MANAGER, 0, msgBuff.data, msgBuff.length);
#endif

      // error message send
      sprintf(cmdReason, "fail to resolve symbols : %s\n", sCompModuleName);
      cmdResult = ntohl(FAILURE);

      nnBufferReset (&msgBuff);
      nnBufferSetInt32T (&msgBuff, cmdFd);
      nnBufferSetInt32T (&msgBuff, cmdKey);
      nnBufferSetInt32T (&msgBuff, cmdResult);
      nnBufferSetInt8T (&msgBuff, strlen(cmdReason));
      nnBufferSetString(&msgBuff, cmdReason, strlen(cmdReason));

      if(msgId == IPC_LCS_PM2C_DYNAMIC_UPGRADE)
      {
        if(gNosCompName == PROCESS_MANAGER)
        {
          return DLU_TASK_FAIL;
        }              

        _ipcResponseSync(PROCESS_MANAGER,
                         IPC_LCS_PM2C_DYNAMIC_UPGRADE_RESPONSE,
                         msgBuff.data,
                         msgBuff.length);
      }
      else
      {
        _ipcResponseSync(DLU_TESTER,
                         IPC_DYNAMIC_UPGRADE_TEST_RESPONSE,
                         msgBuff.data,
                         msgBuff.length);
      }

      return DLU_TASK_FAIL;
    }
  }

  //resolve nos library function symbols 
  LIBMAP mapFunc = (LIBMAP)sDynamicFunction[NOS_TASK_LIB_MAP].func;
  if((mapFunc(&nosService, &gNosCompData, &gNosCompName)) < 0)
  {
    nLOG(LOG_ERR, "fail to resolve library\n");
#if 0 
        // error message send
        sprintf(cmdReason, "fail to resolve library\n");
        nnBufferSetString(&msgBuff, cmdReason, strlen(cmdReason));
        _ipcSendAsync(COMMAND_MANAGER, 0, msgBuff.data, msgBuff.length);

        cmdResult = ntohl(CMD_SHOW_END);
        nnBufferReset (&msgBuff);
        nnBufferSetInt32T (&msgBuff, cmdFd);
        nnBufferSetInt32T (&msgBuff, cmdKey);
        nnBufferSetInt32T (&msgBuff, cmdResult);
        _ipcSendAsync(COMMAND_MANAGER, 0, msgBuff.data, msgBuff.length);
#endif

    // error message send
    sprintf(cmdReason, "fail to resolve library\n");
    cmdResult = ntohl(FAILURE);

    nnBufferReset (&msgBuff);
    nnBufferSetInt32T (&msgBuff, cmdFd);
    nnBufferSetInt32T (&msgBuff, cmdKey);
    nnBufferSetInt32T (&msgBuff, cmdResult);
    nnBufferSetInt8T (&msgBuff, strlen(cmdReason));
    nnBufferSetString(&msgBuff, cmdReason, strlen(cmdReason));

    if(msgId == IPC_LCS_PM2C_DYNAMIC_UPGRADE)
    {
      if(gNosCompName == PROCESS_MANAGER)
      {
        return DLU_TASK_FAIL;
      }
 
      _ipcResponseSync(PROCESS_MANAGER,
                       IPC_LCS_PM2C_DYNAMIC_UPGRADE_RESPONSE,
                       msgBuff.data,
                       msgBuff.length);
    }
    else
    {
      _ipcResponseSync(DLU_TESTER,
                       IPC_DYNAMIC_UPGRADE_TEST_RESPONSE,
                       msgBuff.data,
                       msgBuff.length);
    }


    return DLU_TASK_FAIL;
  }

  //send restart signal to component
  nLOG(LOG_INFO, "restart component \n");
  nosCompRestart();

#if 0 
    sprintf(cmdReason, "DLU : Success to upgrade module %s\n", sCompModuleName);
    nnBufferSetString(&msgBuff, cmdReason, strlen(cmdReason));
    _ipcSendAsync(COMMAND_MANAGER, 0, msgBuff.data, msgBuff.length);

    cmdResult = ntohl(CMD_SHOW_END);
    nnBufferReset (&msgBuff);
    nnBufferSetInt32T (&msgBuff, cmdFd);
    nnBufferSetInt32T (&msgBuff, cmdKey);
    nnBufferSetInt32T (&msgBuff, cmdResult);

    nLOG(LOG_DEBUG, "dlu response message send\n");
    nLOG(LOG_DEBUG, "cmdFd(%d), cmdKey(%d), cmdResult(%d)\n",
                     cmdFd, cmdKey, cmdResult);

    nnBufferPrint(&msgBuff);
    nnBufferSetString (&msgBuff, "\n", strlen("\n"));
    _ipcSendAsync(COMMAND_MANAGER, 0, msgBuff.data, msgBuff.length);
#endif

  memset(gNosCompVersion, 0x00, COMP_VER_SIZE);
  strncpy(gNosCompVersion, compVer, strlen(compVer));

  sprintf(cmdReason, "DLU : Success to upgrade module %s\n", sCompModuleName);
  cmdResult = ntohl(SUCCESS);

  nnBufferReset (&msgBuff);
  nnBufferSetInt32T (&msgBuff, cmdFd);
  nnBufferSetInt32T (&msgBuff, cmdKey);
  nnBufferSetInt32T (&msgBuff, cmdResult);
  nnBufferSetInt8T (&msgBuff, strlen(cmdReason));
  nnBufferSetString(&msgBuff, cmdReason, strlen(cmdReason));

  nLOG(LOG_DEBUG, "dlu response message send\n");
  nLOG(LOG_DEBUG, "cmdFd(%d), cmdKey(%d), cmdResult(%d)\n",
                   cmdFd, cmdKey, cmdResult);
  
  if(msgId == IPC_LCS_PM2C_DYNAMIC_UPGRADE)
  {
    if(gNosCompName == PROCESS_MANAGER)
    {
      IPC tmpFunc = (IPC)sDynamicFunction[COMPONENT_IPC].func;
      tmpFunc
        (IPC_LCS_PM2C_DYNAMIC_UPGRADE_RESPONSE, &msgBuff.data, msgBuff.length);
    }
    else
    {
      _ipcResponseSync(PROCESS_MANAGER,
                       IPC_LCS_PM2C_DYNAMIC_UPGRADE_RESPONSE,
                       msgBuff.data,
                       msgBuff.length);
    }
  }
  else
  {
    _ipcResponseSync(DLU_TESTER,
                     IPC_DYNAMIC_UPGRADE_TEST_RESPONSE,
                     msgBuff.data,
                     msgBuff.length);
  }

  nosTaskModuleVersionSend();

  nLOG(LOG_INFO, "Complete live upgrading %s\n", sCompModuleName);

  return DLU_TASK_OK;
}

void nosTaskModuleVersionSend(void)
{
  nnBufferT msgBuff;
  nnBufferReset(&msgBuff);

  nnBufferSetInt32T (&msgBuff, gNosCompName);
  nnBufferSetInt8T  (&msgBuff, strlen(gNosCompVersion));
  nnBufferSetString (&msgBuff, gNosCompVersion, strlen(gNosCompVersion));

  if(gNosCompName == PROCESS_MANAGER)
  {
    IPC tmpFunc = (IPC)sDynamicFunction[COMPONENT_IPC].func;
    tmpFunc(IPC_DYNAMIC_UPGRADE_VERSION, &msgBuff.data, msgBuff.length); 
  }
  else
  {
    _ipcSendAsync (PROCESS_MANAGER, IPC_DYNAMIC_UPGRADE_VERSION,
                   msgBuff.data, msgBuff.length);
  }
}
