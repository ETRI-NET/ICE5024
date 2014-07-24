#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "nnTypes.h"
#include "taskManager.h"

#if !defined(ipcmgrInternal_h)
#define _ipcmgrInternal_h


/*******************************************************************************
 *                        CONSTANTS / LITERALS / TYPES
 ******************************************************************************/

#define IPCMGR_EVENT_SUBSCRIBE -1
#define IPCMGR_EVENT_UNSUBSCRIBE -2
#define IPCMGR_EVENT_PROCESS_EOF -3

/*******************************************************************************
*                                GLOBALS VARIABLES
*******************************************************************************/

Int32T gIpcMgrRunFlag;

/*******************************************************************************
 *                           GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/

/* ipcmgrTaskScheduling Function */
void ipcmgrReadCallback (struct bufferevent *bev, void *ctx);
void ipcmgrEventCallback (struct bufferevent *bev, Int16T events, void *ctx);
void ipcmgrChannelOpen(pIpcRegFunc recvFunc);
void ipcmgrChannelAccept 
(struct evconnlistener *ipcListener, evutil_socket_t fd, 
 struct sockaddr *address, int socklen, void *ctx);
void ipcmgrAcceptErrorFunc (struct evconnlistener *ipcListener, void *ctx);
Int32T ipcmgrSignalInit (void);
void ipcmgrSignalCallback (evutil_socket_t sig, Int16T events, void * data);
void ipcmgrSignalOperation (Int32T sig);
void ipcmgrEventDisconnSubscribeDeregister (Int32T fd);

/* ipcmgrProcessInfo Function */
void ipcmgrProcessInfoInit (void);
Int32T ipcmgrGetAcceptPoint (void);
Int32T ipcmgrProcessInfoRegister (Int32T fd, Uint32T process);
Int32T ipcmgrProcessInfoDelete (Int32T fd);
IpcMsgHeaderT * ipcmgrProcessInfoPut (Uint32T process);
Int32T ipcmgrProcessInfoSend (void);

/* ipcmgrProcessConnect Function */
Int32T ipcmgrProcessSingleConnect (Int32T fd);
Int32T ipcmgrProcessAllConnect (void);
Int32T ipcmgrProcessConnect (Uint32T process, StringT ipAddr, Int32T port);

Int32T ipcmgrBindCompleteAck (Int32T process);

#endif

