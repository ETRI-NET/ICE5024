#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <arpa/inet.h>

#include "nnTypes.h"
#include "nnMemmgr.h"
#include "nnLog.h"

#if !defined(taskManager_h)
#define _taskManager_h

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
 * @brief : N2OS Task Scheduling, IPC, Event Service
 *  - Block Name : Library
 *  - Process Name :
 *  - Creator : Changwoo Lee, JaeSu Han
 *  - Initial Date : 2013/7/15
*/

/**
 * @file : taskManager.h
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

#define IPC_MANAGER_IP          "127.0.0.1"
#define IPC_MANAGER_PORT        10000

#define N2OS_INIT_NOT_COMPLETE 0 
#define N2OS_INIT_COMPLETE 1     

#define IPV4_ADDRESS_SIZE 16 


/** N2OS Message Type */
enum {                      

    N2OS_INIT_COMPELTE = 0, /**< 해당 Component 초기화 완료 */
    COMPONENT_ACCEPT_OK,
    IPC_PROCESS_REG,        /**< Process 등록 */
    IPC_PROCESSINFO_PUT,    /**< Process Bind 정보 전송 */
    IPC_BIND_COMPLETE,      /**< Bind 완료 */
    IPC_BIND_COMPLETE_ACK,  /**< Bind 완료 Check */
    IPC_PROCESSINFO_UPDATE, /**< 모든 Component 접속정보 업데이트 */
    IPC_PROC_ALL_INFO_REQ,       /**< 모든 Component 접속정보 요청 */
    IPC_PROC_ALL_INFO_UPDATE,    /**< 모든 Component 접속정보 업데이트 */
    IPC_PROC_SINGAL_INFO_REQ,    /**< 개별 Component 접속정보 요청 */
    IPC_PROC_SINGLE_INFO_UPDATE, /**< 개별 Component 접속정보 업데이트 */
    IPC_SERVICE,            /**< IPC Service */
    EVENT_SUBSCRIBE,        /**< EVENT 등록 */
    EVENT_UNSUBSCRIBE,      /**< EVENT 등록 해제 */
    EVENT_SERVICE,          /**< EVENT Service */
    MAX_MESSAGE_CNT

} IpcMsgTypeT;

/** Message Header Format */
typedef struct IpcMsgHeaderT
{
    Uint16T type;    /**< messgae type */
    Uint16T length;  /**< message total length */
    Uint32T msgId;   /**< message id */
    Int8T data[1];   /**< data */

} IpcMsgHeaderT;

#define IPC_MSG_HEADER_SIZE (sizeof(Uint16T) * 2 + sizeof(Uint32T)) 

/** IPC의 Component 관리 구조체 */
typedef struct IpcCommonLibT
{
    Int32T acceptFd;               /**< Accept Fd */
    struct bufferevent *bev;       /**< Message Send Buffer (Async, Event) */
    struct sockaddr connectInfo;   /**< Connect Process Info. */
    struct sockaddr allocateInfo;  /**< Allocate Process Info. */
    Int32T bufSize;                /**< Received Message Temp Buffer Size */
    StringT buf;                   /**< Received Message Temp Buffer */

} IpcCommonLibT;

/** Process Connect list 관리 구조체 */
typedef struct IpcAcceptT
{
    Uint32T fd;              /**< Socket fd */
    struct sockaddr addr;    /**< accept 된 연결 정보 */

} IpcAcceptT;


/** Process Registe Message Data Format */
typedef struct IpcRegMsgT 
{
    Uint32T process;         /**< 등록할 Process Name */

} IpcRegMsgT;

/** Process 할당 연결정보 Data Format */
typedef struct IpcInfoPutMsgT 
{
    Int8T sIpAddr[IPV4_ADDRESS_SIZE];  /**< 할당된 연결정보 IP */
    Uint32T port;                      /**< 할당된 연결정보 PORT */ 

} IpcInfoPutMsgT;

/** Event 등록/삭제 Message Data Format */
typedef struct EventSubMsgT
{
    Uint32T process;    /**< 등록/삭제할 Process Name */
    Uint32T eventId;    /**< 등록/삭제할 Event Id */
    Uint32T priority;   /**< 등록할 Process의 해당 Event Priority */

} EventSubMsgT;

/** taskSignalSetAggr() signal task 관리 구조체 */
typedef struct TaskSigAggrT 
{
    Int8T flag;      /**< flag (0 : Available, 1 : Impossible) */
    Int32T sig;      /**< signal num */
    void * taskSig;  /**< signal task 할당 주소 값 */

} TaskSigAggrT;


/* Dynamic Live Upgrade Message Data Format */

#define DLU_FILE_SIZE 64
typedef struct DluMsgT
{
    Int8T fileName[DLU_FILE_SIZE];

} DluMsgT;

/* Task Scheduling, IPC, Event Service Internal Data Structure */

/* Cqueue */
typedef struct QueueNodeT 
{
    struct QueueNodeT * next; 
    struct QueueNodeT * prev; 
    void *data;                
} QueueNodeT;

typedef struct QueueT 
{
    struct QueueNodeT * head; 
    struct QueueNodeT * tail; 
    Uint64T count;             
    Uint64T maxCount;          
    Uint32T dataType;          
    Uint8T dupCheck;           
} QueueT;

#define CQUEUE_DEFAULT_COUNT 256 
#define CQUEUE_ERR_INCORRECT_INDEX  -2 
#define CQUEUE_ERR_FULL             -2
#define CQUEUE_ERR_CQUEUE_ALLOC     -3 
#define CQUEUE_ERR_CQUEUENODE_ALLOC -4 
#define CQUEUE_ERR_NULL             -5 
#define CQUEUE_ERR_EMPTY            -6 
#define CQUEUE_ERR_DUP              -7 
#define CQUEUE_ERR_ARGUMENT         -8 
#define CQUEUE_DUPLICATED    0 
#define CQUEUE_NOT_DUPLICATED 1 



/* Function Pointer */

/** User-Defined Init Func. Pointer */
typedef void (*pTaskInitFunc) (void); 

/** Task : File Discriptor Func. Pointer */
typedef void (*pTaskFdFunc) (Int32T fd, Int16T event, void * arg);

/** Task : Timer Func. Pointer */
typedef void (*pTaskTimerFunc) (Int32T fd, Int16T event, void * arg);

/** Task : taskSignalSetAggr() 전용 Singal Func. Pointer */
typedef void (*pTaskSignalRegFunc) (Int32T sig);

/** Task : Signal Func. Pointer */
typedef void (*pTaskSignalFunc) (Int32T fd, Int16T event, void * arg);

/** IPC Callback Func. Pointer */
typedef void (*pIpcRegFunc) (Uint32T msgId, void * data, Uint32T dataLen);

/** Event Callback Func. Pointer */
typedef void (*pEventRegFunc) (Uint32T eventId, void * data, Uint32T dataLen);


/*******************************************************************************
*                                GLOBALS VARIABLES
*******************************************************************************/

/* Global Callback Function */

pTaskInitFunc gInitCb;        /**< Process Init Callback Func. */
pIpcRegFunc   gIpcCb;         /**< IPC Message Callback Func. */
pEventRegFunc gEventCb;	      /**< EVENT Message Callback Func. */

/** taskSignalSetAggr() 전용 Callback Func.
(SIGINT, SIGTERM, SIGHUP, SIGUSR1, SIGSEGV) */
pTaskSignalRegFunc gSignalCb; 

struct event_base *gTaskService;  /**< IPC : Libevent base structure */
struct evconnlistener *gListener; /**< IPC : Libevent Listener */

Uint32T gUserProcess; /**< Use Process Name */

/**< Global IPC Common Mgt */
IpcCommonLibT gIpcCommMgt[MAX_PROCESS_CNT]; 
Int32T gListenerCheck;

IpcAcceptT gIpcAcceptMgt[MAX_PROCESS_CNT];  /**< Global Accept Process Mgt */
QueueT *gIpcMsgPendingList;                /**< Message Pending List */

#define TASK_SIG_AGGR_CNT 5
TaskSigAggrT gTaskSigAggrMgt[TASK_SIG_AGGR_CNT];

#define IPC_MAX_PENDING_SIZE 1024 

#define IPC_REG_NOT_CHECK 0
#define IPC_REG_CHECK     1

/* Aggr Signal Flag */
#define TASK_SIGNAL_AVAILABLE 0
#define TASK_SIGNAL_IMPOSSIBLE 1

// MAX SIZE
#define IPC_MAX_MESSAGE_SIZE 4096   /**< IPC Message Max Size */
#define EVENT_MAX_MESSAGE_SIZE 4096 /**< Event Message Max Size */

// ERROR CODE
#define TASK_ERR_LIBEVENT    -1  /**< Libevent Library 실패 */
#define TASK_ERR_ARGUMENT    -2  /**< 잘못된 인자 값 */
#define TASK_ERR_ALREADY_SET -3  /**< 이미 설정된 값 */

#define IPC_ERR_ARGUMENT          -1 /**< 잘못된 인자 값 */
#define IPC_ERR_MGR_CONN          -2 /**< IPC Manager 연결 실패 */
#define IPC_ERR_PENDING_INIT      -3 /**< Msg Pending List 초기화 실패 */
#define IPC_ERR_MSG_CREATE        -4 /**< Message 생성 실패 */
#define IPC_ERR_PROCESS_NOT_CONN  -5 /**< 해당 Process 가 연결되어 있지 않음 */
#define IPC_ERR_MSG_SIZE_BIG      -6 /**< Msg 크기 큼 */
#define IPC_ERR_MSG_SYNC_FAILURE  -7  /**< Sync Message 관련 실패 */
#define IPC_ERR_MSG_SYNC_WRITE    -8  /**< Sync Message 전송 실패 */
#define IPC_ERR_MSG_SYNC_READ     -9  /**< Sync Message 읽기 실패 */
#define IPC_ERR_MSG_REGIST        -10 /**< Process 등록 실패 */
#define IPC_ERR_MSG_SEND          -11 /**< Message 전송 실패 */

#define EVENT_ERR_ARGUMENT       -1 /**< 잘못된 인자 값 */
#define EVENT_ERR_MSG_CREATE     -2 /**< Message 생성 실패 */
#define EVENT_ERR_MSG_SIZE_BIG   -3 /**< Message 크기 큼 */
#define EVENT_ERR_PRECEDING_TASK -4 /**< 선행 조건을 완료하지 않음 */
#define EVENT_ERR_MSG_SEND       -5 /**< Message 전송 실패 */

#define DLU_ERR_ARGUMENT    -1 /**< 잘못된 인자 값 */
#define DLU_ERR_MSG_CREATE  -2 /**< Message 생성 실패 */
#define DLU_ERR_MSG_SET     -3 /**< Message Setting 실패 */
#define DLU_ERR_MSG_SEND    -4 /**< Message 전송 실패 */

// Utility
#define IP_PRINT(a) \
    *((Uint8T *)(&a)), \
    *(((Uint8T *)(&a)) + 1), \
    *(((Uint8T *)(&a)) + 2), \
    *(((Uint8T *)(&a)) + 3)


// Function List

/* taskManage.c Function List*/
Int32T taskCreate(pTaskInitFunc initFunc);
void taskDispatch(void);
void _taskClose(void);

void * _taskFdSet(
    void (* taskFdCb)(Int32T, Int16T, void *), 
    Int32T fd, 
    Uint32T flags, 
    Uint32T priority,
    void * arg);
void _taskFdDel(void * fdEvent);

void * _taskTimerSet(
    void (* taskTimerCb)(Int32T, Int16T, void *), 
    struct timeval tv, 
    Uint32T persist, 
    void * arg);
void _taskTimerDel(void * delTimer);

Int32T _taskSignalSetAggr(pTaskSignalRegFunc signalFunc);
void _taskSignalDelAggr(void);

//void * _taskSignalSet(Int32T signalType, pTaskSignalFunc signalFunc);
void * _taskSignalSet(Int32T signalType);
void _taskSignalDel(void * delSignal);

void * _taskFdUpdate(
    void (* taskFdCb)(Int32T, Int16T, void *), 
    void * fdEvent, 
    void * newArg);
void * _taskTimerUpdate(
    void (* taskTimerCb)(Int32T, Int16T, void *),
    void * timerTask, 
    struct timeval tv, 
    void * newArg);
Int32T _taskSignalUpdateAggr(pTaskSignalRegFunc signalFunc);
void * _taskSignalUpdate(void * sigEvent, void * taskSigCb);


/* ipcService.c Function List*/
Int32T _ipcChannelOpen (Uint32T process, pIpcRegFunc recvFunc);
Int32T _ipcSendAsync(
    Uint32T process, 
    Uint32T messageId, 
    void * message, 
    Uint32T size);
Int32T _ipcSendSync(
    Uint32T process, 
    Uint32T messageId, 
    Uint32T retId, 
    void * message, 
    Uint32T size, 
    StringT outBuff, 
    Uint32T * outSize);
Int32T _ipcResponseSync(
    Uint32T process, 
    Uint32T messageId, 
    void * message, 
    Uint32T size);
Int32T _ipcProcessingPendingMsg (void);
Int32T _ipcChannelUpdate(pIpcRegFunc recvFunc);
Int32T _ipcProcessInfoGet(Int32T process, struct sockaddr *processInfo);

/* eventService.c Function List*/
Int32T _eventOpen(pEventRegFunc recvFunc);
Int32T _eventSubscribe(Uint32T eventId, Uint32T priority);
Int32T _eventUnsubscribe(Uint32T eventId);
Int32T _eventPublish(Uint32T eventId, void * eventData, Uint32T dataSize);
Int32T _eventChannelUpdate(pEventRegFunc recvFunc);
Int32T _eventSubscribeSend
 (Uint32T process, Uint32T eventId, void * data, Int32T size);

/* Dynamic Live Upgrade Function List */
//void _taskPmModuleUpdate (void * data, Uint32T dataLen);
//Int32T _dluSendMsg (Uint32T process, Uint32T msgId, void *msg, Uint32T size);
//Int32T dluTaskModuleUpdate (void * data);



/* Internal Function */
void ipcInterReadCb(struct bufferevent *bev, void *ctx);
void ipcInterReadOperation(IpcMsgHeaderT *pMsg, Int32T fd, Int32T *process);
void ipcInterWriteCb(struct bufferevent *bev, void *ctx);
void ipcInterEventCb(struct bufferevent *bev, short events, void *ptr);
void ipcChannelAccept(struct evconnlistener *ipcListener,
          evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx);
void ipcAcceptErrorFunc(struct evconnlistener *ipcListener, void *ctx);

Int32T ipcProcessConnect(Uint32T process, StringT ip, Uint32T port);
Int32T ipcProcessInfoDelete(Int32T fd);
Int32T ipcGetAcceptPoint(void);
Int32T ipcProcessConnectListUpdate(struct sockaddr *recvList);
Int32T ipcProcessConnectListUpdateFd(Int32T fd, Uint32T process);
Int32T ipcGetProcess(Int32T fd);
Int32T ipcGetFd(Uint32T process);
Int32T ipcConnectRegist (Uint32T process, Uint32T target);
Int32T ipcConnectRegistSync (Uint32T process, Int32T socket);
Int32T ipcChannelConnect (StringT ip, Uint32T port);
Int32T ipcBindRegist (void);
Int32T ipcSendMsgSync
 (Int32T socket, Uint16T type, Uint32T messageId, void * message, Uint32T size);


/* Internal Data Structure */
QueueT * queueInit(Uint64T maxCount, Uint8T dupCheck, Uint32T dataType);
void queueFree(QueueT * cqueue);
void queueDeleteAllNode(QueueT * cqueue);
Int64T queueCount(QueueT * cqueue);
Int32T queueEnqueue(QueueT * cqueue, void * data);
void * queueDequeue(QueueT * cqueue);
void * queueGetHead(QueueT * cqueue);
void * queueGetTail(QueueT * cqueue);


#endif
