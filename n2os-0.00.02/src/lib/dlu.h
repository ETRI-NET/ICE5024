#if !defined(_dlu_h)
#define _dlu_h
/*345678901234567890123456789012345678901234567890123456789012345678901234567890*/
/*********************************************************************************
 *               Electronics and Telecommunications Research Institute
 * Filename : <myFileName>
 * Blockname: <PIF Manager>
 * Overview : <PIF Manager S/W block manages Port/Interface & L2 MAC/VLAN>
 * Creator  : <Seungwoo Hong>
 * Owner    : <Seungwoo Hong>
 * Copyright: 2013 Electronics and Telecommunications Research Institute. 
 *            All rights reserved. No part of this software shall be reproduced, 
 *            stored in a retrieval system, or transmitted by any means, 
 *            electronic, mechanical, photocopying, recording, or otherwise, 
 *            without written permission from ETRI.
 *********************************************************************************/
/*********************************************************************************
 *                        VERSION CONTROL  INFORMATION
 * $Author$
 * $Date$
 * $Revision
 * $Log$ 
 *********************************************************************************/

#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include "nnTypes.h"

#define DLU_TASK_OK	       0
#define DLU_TASK_FAIL     -1

// Task
typedef void (*TaskSignalRegFunc) (Int32T sig);
typedef void (*TaskSignalFunc) (Int32T fd, Int16T event, void * arg);
typedef void* (*TASKFdSet)
 (void (* taskFdCb)(Int32T, Int16T, void *), 
  Int32T fd, Uint32T flags, Uint32T priority, void * arg); 
typedef void  (*TASKFdDel)(void * fdEvent); 
typedef void* (*TASKTimerSet)
 (void (* taskTimerCb)(Int32T, Int16T, void *), 
  struct timeval tv, Uint32T persist, void *arg);
typedef void  (*TASKTimerDel)(void * delTimer); 
typedef void* (*TASKFdUpdate)
  (void (* taskFdCb)(Int32T, Int16T, void *), 
   void * fdTask, void * newArg);
typedef void* (*TASKTimerUpdate)
 (void (* taskTimerCb)(Int32T, Int16T, void *), 
  void * timerTask, struct timeval tv, void * newArg);
typedef Int32T (*TASKSigAggrSet)(TaskSignalRegFunc signalFunc);
typedef void   (*TASKSigAggrDel)(void);
typedef Int32T  (*TASKSigAggrUpdate)(TaskSignalRegFunc signalFunc);
//typedef void*  (*TASKSigSet)(Int32T signalType, TaskSignalFunc signalFunc);
typedef void*  (*TASKSigSet)(Int32T signalType);
typedef void   (*TASKSigDel)(void * delSignal);
typedef void * (*TASKSigUpdate)(void * sigEvent, void * taskSigCb);
typedef void (*TASKClose) (void);
//typedef void (*TASKPmUpdate)(void *data, Uint32T dataLen);


// IPC
typedef void (*IpcRegFunc) (Uint32T msgId, void * data, Uint32T dataLen);
typedef Int32T (*IPCChannelOpen) (Uint32T process, IpcRegFunc recvFunc);
typedef Int32T (*IPCSendAsync) 
    (Uint32T process, Uint32T messageId, void * message, Uint32T size);
typedef Int32T (*IPCSendSync) 
    (Uint32T process, Uint32T messageId, Uint32T retId, void * message, 
     Uint32T size, StringT outBuff, Uint32T * outSize);
typedef Int32T (*IPCResponseSync) 
    (Uint32T process, Uint32T messageId, void * message, Uint32T size);
typedef Int32T (*IPCProcessingPendingMsg) (void);
typedef Int32T (*IPCChannelUpdate) (IpcRegFunc recvFunc);
typedef Int32T (*IPCInfoGet) (Int32T process, struct sockaddr *processInfo);

// Event
typedef void (*EventRegFunc) (Uint32T eventId, void * data, Uint32T dataLen);
typedef Int32T (*EVTOpen)(EventRegFunc recvFunc);
typedef Int32T (*EVTSubscribe)(Uint32T eventId, Uint32T priority);
typedef Int32T (*EVTUnsubscribe)(Uint32T eventId);
typedef Int32T (*EVTPublish)
 (Uint32T eventId, void * eventData, Uint32T dataSize);
typedef Int32T (*EVTUpdate)(EventRegFunc recvFunc);  
typedef Int32T (*EVTSubSend)
 (Uint32T process, Uint32T eventId, void * data, Int32T size);

//LOG
typedef Int8T (*LOGInit)(Uint32T process);
typedef void  (*LOGClose)();
typedef void  (*LOGSetFlag)(Uint8T flags);
typedef void  (*LOGUnsetFlag)(Uint8T flags);
typedef Int8T (*LOGSetFile)(StringT filename, Uint32T size);
typedef void  (*LOGSetPriority)(Uint8T priority);
typedef void  (*LOGLogging)
 (StringT file, Uint32T line, Uint32T priority, const StringT format, ...);
typedef void  (*LOGLoggingDebug)
 (StringT file, Uint32T line, Uint32T priority, const StringT format, ...);


//MEM
typedef Int32T (*MEMInit)(Uint32T process);
typedef void   (*MEMClose)(void);
typedef Int32T (*MEMSetDebug)(Uint32T memType);
typedef Int32T (*MEMUnsetDebug)(Uint32T memType);
typedef void   (*MEMSetFile)(StringT fileName, Uint32T size);
typedef void   (*MEMUnsetFile)(void);
typedef void   (*MEMShowAllUser)(Uint32T index);
typedef void*  (*MEMMalloc)
 (Uint32T type, SizeT size, StringT file, StringT func, Uint32T line);
typedef void*  (*MEMRealloc)
 (Uint32T type, void * reMem, SizeT size, 
  StringT file, StringT func, Uint32T line);
typedef Int8T  (*MEMFree)
 (Uint32T type, void * freeMem, StringT file, StringT func, Uint32T line);


typedef struct
{
    TASKFdSet         fd_set;
    TASKFdDel         fd_del;
    TASKTimerSet      timer_set;
    TASKTimerDel      timer_del;
    TASKFdUpdate      fd_update;
    TASKTimerUpdate   timer_update;
    TASKSigAggrSet    sig_set_aggr;
    TASKSigAggrDel    sig_del_aggr;
    TASKSigAggrUpdate sig_update_aggr;
    TASKSigSet        sig_set;
    TASKSigDel        sig_del;
    TASKSigUpdate     sig_update;
    TASKClose         task_close;
//    TASKPmUpdate      task_pm_dlu;
} NosTaskServiceT;

typedef struct
{
    IPCChannelOpen          open;
    IPCSendAsync            send_async;
    IPCSendSync             send_sync;
    IPCResponseSync         response_sync;
    IPCProcessingPendingMsg pending_msg;
    IPCChannelUpdate        update;
    IPCInfoGet              info_get;
} NosIpcServiceT;

typedef struct
{
    EVTOpen         open;
    EVTSubscribe    sub;
    EVTUnsubscribe  unsub;
    EVTPublish      publish;
    EVTUpdate       update;
    EVTSubSend      sub_send;
} NosEventServiceT; 

typedef struct
{
    LOGInit         init;
    LOGClose        close;
    LOGSetFlag      set_flag;
    LOGUnsetFlag    unset_flag;
    LOGSetFile      set_file;
    LOGSetPriority  set_pri;
    LOGLogging      loging;
    LOGLoggingDebug loging_debug;
} NosLogServiceT;

typedef struct
{
    MEMInit         init;
    MEMClose        close;
    MEMSetDebug     set_debug;
    MEMUnsetDebug   unset_debug;
    MEMSetFile      set_file;
    MEMUnsetFile    unset_file;
    MEMShowAllUser  show;
    MEMMalloc       alloc;
    MEMRealloc      realloc;
    MEMFree         free;
} NosMemServiceT;

typedef struct
{
    NosTaskServiceT     TASK;
    NosIpcServiceT      IPC;
    NosEventServiceT    EVT;
    NosLogServiceT      LOG;
    NosMemServiceT      MEM;
} NosServiceT;



#endif   /* _dlu_h */

