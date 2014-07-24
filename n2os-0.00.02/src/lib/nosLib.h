#if !defined(_nosLib_h)
#define _nosLib_h

#include <arpa/inet.h>
#include "nnTypes.h"
#include "nnDefines.h"
#include "dlu.h"

/**************************************************************************
 *                      NOS Library
 *************************************************************************/
/* Task */
typedef void (*SignalRegFunc) (Int32T sig);
typedef void (*SignalFunc) (Int32T fd, Int16T event, void * arg);

void* taskFdSet
 (void (* taskFdCb)(Int32T, Int16T, void *), 
  Int32T fd, Uint32T flags, Uint32T priority, void * arg);
void  taskFdDel(void * fdEvent);
void* taskTimerSet
 (void (* taskTimerCb)(Int32T, Int16T, void *), 
  struct timeval tv, Uint32T persist, void * arg);
void  taskTimerDel(void * delTimer);
void * taskFdUpdate
  (void (* taskFdCb)(Int32T, Int16T, void *), 
  void * fdTask, void * newArg);
void * taskTimerUpdate
 (void (* taskTimerCb)(Int32T, Int16T, void *), 
  void * timerTask, struct timeval tv, void * newArg);
Int32T taskSignalSetAggr(SignalRegFunc signalFunc);
void taskSignalDelAggr(void);
//void * taskSignalSet(Int32T signalType, SignalFunc signalFunc);
void * taskSignalSet(Int32T signalType);
void taskSignalDel(void * delSignal);
Int32T taskSignalUpdateAggr(SignalRegFunc signalFunc);
void * taskSignalUpdate(void * sigEvent, void * taskSigCb);
void taskClose(void);
//void taskPmModuleUpdate (void * data, Uint32T dataLen);


/* Ipc */
typedef void (*msgFunc) (Uint32T msgId, void * data, Uint32T dataLen);
Int32T ipcChannelOpen (Uint32T process, msgFunc recvFunc);
Int32T ipcSendAsync (Uint32T process, Uint32T messageId, void * message, Uint32T size);
Int32T ipcSendSync (Uint32T process, Uint32T messageId, Uint32T retId, void * message, Uint32T size, StringT outBuff, Uint32T * outSize);
Int32T ipcResponseSync (Uint32T process, Uint32T messageId, void * message, Uint32T size);
Int32T ipcProcPendingMsg (void);
Int32T ipcChannelUpdate(msgFunc recvFunc);
Int32T ipcProcessInfoGet(Int32T process, struct sockaddr *processInfo);

/* Event */
Int32T eventOpen(msgFunc recvFunc);
Int32T eventSubscribe(Uint32T eventId, Uint32T priority);
Int32T eventUnsubscribe(Uint32T eventId);
Int32T eventPublish(Uint32T eventId, void * eventData, Uint32T dataSize);
Int32T eventChannelUpdate(msgFunc recvFunc);
Int32T eventSubscribeSend(Uint32T process, Uint32T eventId, void * data, Int32T size);

/* Log */
Int8T nnLogInit(Uint32T process);
void  nnLogClose();
void  nnLogSetFlag(Uint8T flags);
void  nnLogUnsetFlag(Uint8T flags);
Int8T nnLogSetFile(StringT filename, Uint32T size);
void  nnLogSetPriority(Uint8T priority);
extern void (*nnLogFunc)(StringT file, Uint32T line, Uint32T priority, const StringT format, ...);
extern void (*nnLogDebugFunc)(StringT file, Uint32T line, Uint32T priority, const StringT format, ...);

#define NNLOG(A, B, ...) nnLogFunc(__FILE__, __LINE__, A, B, ##__VA_ARGS__)
#define NNLOGDEBUG(A, B, ...) nnLogDebugFunc(__FILE__, __LINE__, A, B, ##__VA_ARGS__) 

/* Mem */
Int32T memInit(Uint32T process);
void   memClose(void);
Int32T memSetDebug(Uint32T memType);
Int32T memUnsetDebug(Uint32T memType);
void   memSetFile(StringT fileName, Uint32T size);
void   memUnsetFile(void);
void   memShowAllUser(Uint32T index);
void*  memMalloc(Uint32T type, SizeT size, StringT file, StringT func, Uint32T line);
void*  memRealloc(Uint32T type, void * reMem, SizeT size, StringT file, StringT func, Uint32T line);
Int8T  memFree(Uint32T type, void * freeMem, StringT file, StringT func, Uint32T line);

#define NNMALLOC(A, B) memMalloc(A, B, __FILE__, (StringT)__func__, __LINE__)
#define NNREALLOC(A, B, C) memRealloc(A, B, C, __FILE__, (StringT)__func__, __LINE__)
#define NNFREE(A, B) memFree(A, B, __FILE__, (StringT)__func__, __LINE__)

#endif   /* _nosLib */
