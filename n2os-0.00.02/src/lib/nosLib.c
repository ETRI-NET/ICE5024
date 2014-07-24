#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <dlfcn.h>
#include "nosLib.h"
#include "dlu.h"
#include "compConfig.h"

//nos component global data
void** gCompData = NULL;
NosServiceT* NOSLIB = NULL;
LOGLogging nnLogFunc = NULL;
LOGLoggingDebug nnLogDebugFunc = NULL;

void componentInit()
{
    if(sNosInitProcess == NULL) return;
    sNosInitProcess();
}
void componentTerm()
{
    if(sNosTermProcess == NULL) return;
    sNosTermProcess();
}

void componentRestart()
{
    if(sNosRestartProcess == NULL) return;
    sNosRestartProcess();
}

void componentHold()
{
    if(sNosHoldProcess == NULL) return;
    sNosHoldProcess();
}

void componentIpc(Uint32T messageId, void * message, Uint32T size)
{
    if(sNosIpcProcess == NULL) return;
    sNosIpcProcess(messageId, message, size);
}

void componentEvent(Uint32T messageId, void * message, Uint32T size)
{
    if(sNosEventProcess == NULL) return;
    sNosEventProcess(messageId, message, size);
}

void componentSignal(Uint32T sig)
{
    if(sNosSignalProcess == NULL) return;
    sNosSignalProcess(sig);
}

void componentCmCmshProcess(Int32T sockId, void *message, Uint32T size)
{
    if(sNosCmshIpcProcess == NULL) return;
    sNosCmshIpcProcess(sockId, message, size);
}

void componentCmCompProcess(Int32T sockId, void *message, Uint32T size)
{
    if(sNosCompIpcProcess == NULL) return;
    sNosCompIpcProcess(sockId, message, size);  
}

Int32T nosTaskLibMap(NosServiceT* nosLib, void** gData, Int32T* compName)
{
    NOSLIB = nosLib;
    nnLogFunc = (*NOSLIB).LOG.loging;
    nnLogDebugFunc = (*NOSLIB).LOG.loging_debug;
    gCompData = gData;
    (*compName) = sNosCompName;

    return DLU_TASK_OK;
}

Int32T nosTaskLibInit()
{
    // mem lib init
    if(memInit(sNosCompName) < 0){
        fprintf(stderr, "fail to init memory\n");
        return DLU_TASK_FAIL;
    }

    // log lib init
    if(nnLogInit(sNosCompName) < 0){
        fprintf(stderr, "fail to init log\n");
        return DLU_TASK_FAIL;
    }

    // log file & debug level
    nnLogSetFlag(sNosLogLocation); 
    if(nnLogSetFile(sNosLogFile, sNosLogSize) < 0){
        fprintf(stderr, "fail to create log file\n");
        return DLU_TASK_FAIL;
    }
    nnLogSetPriority(sNosLogLevel);

    return DLU_TASK_OK;
}

Int32T nosTaskSchedule()
{
    Int32T num = 0;

    //register event_interests
    for(num = 0; num < sNosComponentEventNum; num++){
        eventSubscribe(sNosEventRegister[num].eventId, 
        sNosEventRegister[num].priority);
    }

    return DLU_TASK_OK;
}

/**************************************************************************
 *                                 NOS Library 
 **************************************************************************/

// Task Scheduling Service API List

void* taskFdSet
 (void (* taskFdCb)(Int32T, Int16T, void *), 
  Int32T fd, Uint32T flags, Uint32T priority, void * arg) 
{
    return (*NOSLIB).TASK.fd_set(taskFdCb, fd, flags, priority, arg); 
}

void  taskFdDel(void * fdEvent)
{
    return (*NOSLIB).TASK.fd_del(fdEvent); 
}

void* taskTimerSet
  (void (* taskTimerCb)(Int32T, Int16T, void *), 
  struct timeval tv, Uint32T persist, void * arg)
{
    return (*NOSLIB).TASK.timer_set(taskTimerCb, tv, persist, arg); 
}

void  taskTimerDel(void * delTimer)
{
    return (*NOSLIB).TASK.timer_del(delTimer); 
}

void * taskFdUpdate
  (void (* taskFdCb)(Int32T, Int16T, void *), void * fdTask, void * newArg)
{
    return (*NOSLIB).TASK.fd_update(taskFdCb, fdTask, newArg);
} 

void * taskTimerUpdate
  (void (* taskTimerCb)(Int32T, Int16T, void *), 
   void * timerTask, struct timeval tv, void * newArg)
{
    return (*NOSLIB).TASK.timer_update(taskTimerCb, timerTask, tv, newArg);
}

Int32T taskSignalSetAggr(SignalRegFunc signalFunc)
{
    return (*NOSLIB).TASK.sig_set_aggr(signalFunc);
}

void taskSignalDelAggr(void)
{
    return (*NOSLIB).TASK.sig_del_aggr();
}

#if 0
void * taskSignalSet(Int32T signalType, SignalFunc signalFunc)
{
    return (*NOSLIB).TASK.sig_set(signalType, signalFunc);
}
#endif

void * taskSignalSet(Int32T signalType)
{
    return (*NOSLIB).TASK.sig_set(signalType);
}

void taskSignalDel(void * delSignal)
{
    return (*NOSLIB).TASK.sig_del(delSignal);
}

Int32T taskSignalUpdateAggr(SignalRegFunc signalFunc)
{
    return (*NOSLIB).TASK.sig_update_aggr(signalFunc);
}

void * taskSignalUpdate(void * sigEvent, void * taskSigCb)
{
    return (*NOSLIB).TASK.sig_update(sigEvent, taskSigCb);
}

void taskClose(void)
{
    return (*NOSLIB).TASK.task_close();
}

#if 0
void taskPmModuleUpdate (void * data, Uint32T dataLen)
{
    printf("In %s\n", __func__);
    (*NOSLIB).TASK.task_pm_dlu(data, dataLen);
    printf("Out %s\n", __func__);

//    return (*NOSLIB).TASK.task_pm_dlu(data, dataLen);
}
#endif

// IPC Service API List

Int32T ipcChannelOpen (Uint32T process, msgFunc recvFunc)
{
    return (*NOSLIB).IPC.open(process, recvFunc); 
}

Int32T ipcSendAsync (Uint32T process, Uint32T messageId, void * message, Uint32T size)
{
    return (*NOSLIB).IPC.send_async(process, messageId, message, size); 
}

Int32T ipcSendSync 
(Uint32T process, Uint32T messageId, Uint32T retId, void * message, Uint32T size, 
 StringT outBuff, Uint32T * outSize)
{
  return (*NOSLIB).IPC.send_sync(process, messageId, retId, message, size, outBuff, outSize); 
}

Int32T ipcResponseSync (Uint32T process, Uint32T messageId, void * message, Uint32T size) 
{
    return (*NOSLIB).IPC.response_sync(process, messageId, message, size); 
}

Int32T ipcProcPendingMsg(void) 
{
    return (*NOSLIB).IPC.pending_msg(); 
}

Int32T ipcChannelUpdate(msgFunc recvFunc)
{
    return (*NOSLIB).IPC.update(recvFunc);
}

Int32T ipcProcessInfoGet(Int32T process, struct sockaddr *processInfo)
{
    return (*NOSLIB).IPC.info_get(process, processInfo);
}

// Event Service API List

Int32T eventOpen(msgFunc recvFunc)
{
    return (*NOSLIB).EVT.open(recvFunc); 
}

Int32T eventSubscribe(Uint32T eventId, Uint32T priority)
{
    return (*NOSLIB).EVT.sub(eventId, priority); 
}

Int32T eventUnsubscribe(Uint32T eventId)
{
    return (*NOSLIB).EVT.unsub(eventId); 
}

Int32T eventPublish(Uint32T eventId, void * eventData, Uint32T dataSize)
{
    return (*NOSLIB).EVT.publish(eventId, eventData, dataSize); 
}

Int32T eventChannelUpdate(msgFunc recvFunc)
{
    return (*NOSLIB).EVT.update(recvFunc);
}

Int32T eventSubscribeSend
 (Uint32T process, Uint32T eventId, void * data, Int32T size)
{
    return (*NOSLIB).EVT.sub_send(process, eventId, data, size);
}

//Log Service API List

Int8T nnLogInit(Uint32T process)
{
    return (*NOSLIB).LOG.init(process); 
}

void nnLogClose() 
{
    return (*NOSLIB).LOG.close(); 
}

void nnLogSetFlag(Uint8T flags) 
{
    return (*NOSLIB).LOG.set_flag(flags); 
}

void nnLogUnsetFlag(Uint8T flags) 
{
    return (*NOSLIB).LOG.unset_flag(flags); 
}

Int8T nnLogSetFile(StringT filename, Uint32T size) 
{
    return (*NOSLIB).LOG.set_file(filename, size); 
}

void nnLogSetPriority(Uint8T priority) 
{
    return (*NOSLIB).LOG.set_pri(priority); 
}

// Memory Management Service API List

Int32T memInit(Uint32T process) 
{
    return (*NOSLIB).MEM.init(process); 
}

void memClose(void) 
{
    return (*NOSLIB).MEM.close(); 
}

Int32T memSetDebug(Uint32T memType) 
{
    return (*NOSLIB).MEM.set_debug(memType); 
}

Int32T memUnsetDebug(Uint32T memType)
{
    return (*NOSLIB).MEM.unset_debug(memType); 
}

void memSetFile(StringT fileName, Uint32T size)
{
    return (*NOSLIB).MEM.set_file(fileName, size); 
}

void memUnsetFile(void) 
{
    return (*NOSLIB).MEM.unset_file(); 
}

void memShowAllUser(Uint32T idx) 
{
    return (*NOSLIB).MEM.show(idx); 
}

void* memMalloc(Uint32T type, SizeT size, StringT file, StringT func, Uint32T line)
{
    return (*NOSLIB).MEM.alloc(type, size, file, func, line); 
}

void* memRealloc
 (Uint32T type, void * reMem, SizeT size, StringT file, StringT func, Uint32T line)
{
    return (*NOSLIB).MEM.realloc(type, reMem, size, file, func, line); 
}

Int8T memFree(Uint32T type, void * freeMem, StringT file, StringT func, Uint32T line)
{
    return (*NOSLIB).MEM.free(type, freeMem, file, func, line); 
}

