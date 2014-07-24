/**
* @file : ipcmgrInternal.c
* @brief : IPC Manager의 내부 동작
*
* $Id:
* $Author:
* $Date:
* $Log$
* $Revision:
* $LastChangedBy:
* $LastChanged$
*
*            Electronics and Telecommunications Research Institute
* Copyright: 2013 Electronics and Telecommunications Research Institute.
*            All rights reserved.
*            No part of this software shall be reproduced, stored in a
*            retrieval system, or transmitted by any means, electronic,
*            mechanical, photocopying, recording, or otherwise, without
*            written permission from ETRI.
**/

/*******************************************************************************
 *                               INCLUDE FILES
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <arpa/inet.h>

#include "nnMemmgr.h"
#include "nnLog.h"
#include "ipcmgrInternal.h"
#include "nnCmdDefines.h"

/*******************************************************************************
 *                              GLOBAL VARIABLES
 ******************************************************************************/


/*******************************************************************************
 *                         GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/

extern void nosCmdInitialize(Int32T cmshID);

/*******************************************************************************
 *                         LOCAL CONSTANTS/LITERALS/TYPES
 ******************************************************************************/



/*******************************************************************************
*                               LOCAL VARIABLES
*******************************************************************************/


/*******************************************************************************
 *                                   CODE
 ******************************************************************************/

/*
 * Description: IPC/EVENT Message(Read) 를 수신하고 분류하여 실행하는 함수
 *
 * @param [in] bev : 수신 Message
 * @param [in] ctx : user's argument (NULL)
 *
 * @retval : None
 */

void ipcmgrReadCallback (struct bufferevent *bev, void *ctx)
{

    // 1. Check the received message 

    struct evbuffer *input = bufferevent_get_input(bev);
    Int32T evBuffLen = evbuffer_get_length(input);
    Int32T fd = bufferevent_getfd(bev);
    Int32T process = ipcGetProcess(fd);

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "Send Message fd : %d, Process : %d\n", fd, process);
    nLOG(LOG_INFO, "Message Total evBuffLen : %d\n", evBuffLen);
    nLOG(LOG_DEBUG, "################################\n");

    // If the process has not been registered yet
    if (process == -1)
    {
        nLOG(LOG_INFO, "Process has not been registered yet\n");
        process = IPC_MANAGER;
    }
   
    // 2. Message Paring and Operation

    if ((evBuffLen + gIpcCommMgt[process].bufSize) > 0)
    {
        Int32T readLen = 0;
        Int32T totalLen = 0;
        Int8T evBuff[IPC_MAX_MESSAGE_SIZE] = {0, };
        Int8T msgBuff[IPC_MAX_MESSAGE_SIZE * 2] = {0,};

        // 2.1 Received Message Copy
        
        evbuffer_copyout(input, evBuff, evBuffLen);

        nLOG(LOG_INFO, "Received Message Copy\n");
        nLOG(LOG_INFO, "evBuffLen : %d\n", evBuffLen);

        if(gIpcCommMgt[process].bufSize > 0)
        {
            nLOG(LOG_INFO, "gIpcCommMgt[%d].bufSize = %d\n", 
                            process, gIpcCommMgt[process].bufSize);
            memcpy(msgBuff, 
                   gIpcCommMgt[process].buf, 
                   gIpcCommMgt[process].bufSize);
  
            nLOG(LOG_INFO, "gIpcCommMgt[%d].buf -> msgBuff : Copy Buffer\n",
                             process);
        }

        memcpy(msgBuff + gIpcCommMgt[process].bufSize, evBuff, evBuffLen);
        nLOG(LOG_INFO, "evBuff -> msgBuff : Copy Buffer\n"); 

        totalLen = gIpcCommMgt[process].bufSize + evBuffLen;
        nLOG(LOG_INFO, "Message Total Len : %d\n", totalLen);

        if(gIpcCommMgt[process].bufSize > 0)
        {
            nLOG(LOG_INFO, "Copyed Buffer Free\n");
            nFREE(IPC_MESSAGE, gIpcCommMgt[process].buf);
            gIpcCommMgt[process].bufSize = 0;
            gIpcCommMgt[process].buf = NULL;

            nLOG(LOG_DEBUG, "gIpcCommMgt[%d].bufSize = %d\n",
                             process, gIpcCommMgt[process].bufSize);
            nLOG(LOG_DEBUG, "gIpcCommMgt[%d].bug = %d\n",
                             process, gIpcCommMgt[process].buf);
        }

        // 2.2 Message Paring & Operation
	
        while(1)
        {
            // 2.2.1 Message Paring
            Int32T msgLen = 0;
            IpcMsgHeaderT * pMsg = (IpcMsgHeaderT *)0;

            memcpy(&msgLen, msgBuff+(readLen+sizeof(Uint16T)),sizeof(Uint16T));

            // Received Message Create

            nLOG(LOG_INFO, "Unit Message Len : %d\n", msgLen);
            nLOG(LOG_INFO, "Unit Message Create\n");

            pMsg = (IpcMsgHeaderT *)nMALLOC(IPC_MESSAGE, msgLen);
            if(pMsg <= 0)
            {
                nLOG(LOG_DEBUG, "################################\n");
                nLOG(LOG_ERR, "Received Message Create Failure\n");
                nLOG(LOG_DEBUG, "################################\n");
                return ;
            }

            memcpy(pMsg, msgBuff + readLen, msgLen);
            nLOG(LOG_INFO, "Unit Message Copy Complete\n");

            // if) Remind Message Length < IPC_MSG_HEADER_SIZE

            if((msgLen < IPC_MSG_HEADER_SIZE)
                             ||
               (pMsg->length > (totalLen - readLen)))
            {
                nLOG(LOG_INFO, "Unit Message Len < IPC_MSG_HEADER_SIZE\n");
               
                gIpcCommMgt[process].bufSize = totalLen - readLen;

                nLOG(LOG_INFO, "pMsg->length : %d, totalLen - readLen : %d\n",
                                pMsg->length, totalLen - readLen);

                gIpcCommMgt[process].buf 
                 = nMALLOC(IPC_MESSAGE, gIpcCommMgt[process].bufSize);
                if(gIpcCommMgt[process].buf <= 0)
                {
                    nLOG(LOG_DEBUG, "################################\n");
                    nLOG(LOG_ERR, "Keep Message Create Failure\n");
                    nLOG(LOG_DEBUG, "################################\n");
                    return ;
                }

                memcpy(gIpcCommMgt[process].buf, 
                       msgBuff + readLen,
                       gIpcCommMgt[process].bufSize);

               nLOG(LOG_INFO,"msgBuff -> gIpcCommMgt[%d].buf : Copy Buffer\n",
                              process);
               nLOG(LOG_INFO, "Copy Len : %d\n", gIpcCommMgt[process].bufSize);
                 
               nFREE(IPC_MESSAGE, pMsg);

               break;
            }

            readLen += msgLen;
          
            nLOG(LOG_INFO, "Current Read Len : %d\n", readLen);

            // 2.3 Message Operation
            nLOG(LOG_INFO, "pMsg->type : %d\n \
                            pMsg->length : %d\n \
                            pMsg->msgId : %d\n",
                            pMsg->type, 
                            pMsg->length, pMsg->msgId);

            if (pMsg->type == N2OS_INIT_COMPELTE)
            {
                nLOG(LOG_INFO, "N2OS_INIT_COMPLETE\n");
                nFREE(IPC_MESSAGE, pMsg);
            }
            else if (pMsg->type == IPC_BIND_COMPLETE)
            {
                static Int32T cnt = 0;
                cnt++;

                nLOG(LOG_INFO, "pMsg->type : IPC_BIND_COMPLETE(CNT : %d)\n",
                                 cnt);

                ipcmgrProcessSingleConnect(fd);
                nLOG(LOG_INFO, "[IPC Manager -> Process] Connect\n");
                nLOG(LOG_INFO, "Connect Process Fd : %d\n", fd);
 
                ipcmgrProcessInfoSend();

                /* Process Manager Send _ 20140415 JAESU HAN */
                // ipcmgrProcAllInfoSend();

                /* NOS Compoent Send _ 20140418 JAESU HAN */
                // ipcmgrBindCompleteAck();

                nLOG(LOG_INFO, "Updated All Process Conn Info Put\n");
                nFREE(IPC_MESSAGE, pMsg);
            }
            else if (pMsg->type == IPC_PROCESS_REG)
            {
                IpcRegMsgT msgData;
                IpcMsgHeaderT * pReplyMsg = NULL;
                nLOG(LOG_INFO, "pMsg->type : IPC_PROCESS_REG\n");

                // 1. Register Process
                msgData.process = (Uint32T)((IpcRegMsgT *)pMsg->data)->process;
                nLOG(LOG_INFO, "Register Process : %d\n", msgData.process);
          
                if(ipcmgrProcessInfoRegister(fd, msgData.process) < 0)
                {
                    nLOG(LOG_DEBUG, "################################\n");
                    nLOG(LOG_INFO, "Process Regist Failure\n");
                    nLOG(LOG_DEBUG, "################################\n");
                    nFREE(IPC_MESSAGE, pMsg);
                }
                else
                {
                    nLOG(LOG_INFO, "Process Regist Success\n");

                    // 2. Put Allocate Connect Info.
                    pReplyMsg = ipcmgrProcessInfoPut(msgData.process);
                    if(pReplyMsg < 0)
                    {
                        nLOG(LOG_DEBUG, "################################\n");
                        nLOG(LOG_ERR, "Allocate Info Create Failure\n");
                        nLOG(LOG_DEBUG, "################################\n");
                    }
                    else
                    {
                        nLOG(LOG_INFO, "Allocate Connect Info Create\n");
                        bufferevent_write(bev, pReplyMsg, pReplyMsg->length);
                        nLOG(LOG_INFO, "Allocate Connect Info Send\n");
		            }
                   	nFREE(IPC_MESSAGE, pMsg);
                    nFREE(MESSAGE_HEADER, pReplyMsg);
                }

                process = ipcGetProcess(fd);
                nLOG(LOG_INFO, "IPC_PROCESS_REG, process = %d\n", process);
            }
            else if (pMsg->type == IPC_SERVICE)
            {
                if(pMsg->msgId == IPC_LCS_PM2C_COMMAND_MANAGER_UP)
                {
                    // CM IPC Initialize
                    nosCmdInitialize(IPC_POL_MGR);
                    gIpcCb(pMsg->msgId, pMsg->data, pMsg->length);
                }

                gIpcCb(pMsg->msgId, pMsg->data, pMsg->length);
                nFREE(IPC_MESSAGE, pMsg);
            }
            else if (pMsg->type == EVENT_SUBSCRIBE)
            {
                gEventCb(IPCMGR_EVENT_SUBSCRIBE, pMsg->data, pMsg->length);
                nFREE(IPC_MESSAGE, pMsg);
            }
            else if (pMsg->type == EVENT_UNSUBSCRIBE)
            {
                gEventCb(IPCMGR_EVENT_UNSUBSCRIBE, pMsg->data, pMsg->length);
                nFREE(IPC_MESSAGE, pMsg);
            }
            else if (pMsg->type == EVENT_SERVICE)
            {
                gEventCb(pMsg->msgId, pMsg->data, pMsg->length);
                nFREE(IPC_MESSAGE, pMsg);
            }
            else
            {
                nLOG(LOG_DEBUG, "################################\n");
                nLOG(LOG_ERR, "None Msg Type\n");
                nLOG(LOG_ERR, "pMsg->type : %d\n \
                                pMsg->length : %d\n \
                                pMsg->msgId : %d\n",
                                pMsg->type, pMsg->length, pMsg->msgId);
                nLOG(LOG_DEBUG, "################################\n");
                nFREE(IPC_MESSAGE, pMsg);
            }

            if ((totalLen - readLen) == 0)
            {
                nLOG(LOG_DEBUG, "################################\n");
                nLOG(LOG_INFO,"TotalLen : %d, ReadLen : %d\n", 
                                totalLen, readLen);
                nLOG(LOG_DEBUG, "################################\n");
                break;
            }
        }
    }
    else
    {
        nLOG(LOG_DEBUG, "################################\n");
        nLOG(LOG_WARNING, "evBuffLen : %d, gIpcCommMgt[%d].bufSize : %d\n",
                           evBuffLen, gIpcCommMgt[process].bufSize);
        nLOG(LOG_WARNING, "recevied message len = 0\n");
        nLOG(LOG_DEBUG, "################################\n");
    }

    // 2.4 remove the input buffer's data

    evbuffer_drain(input, evBuffLen);
}

/*
 * Description: 내부 Event Callback 함수
 *
 * @param [in] events : 발생한 Event
 * @param [in] ctx : user's argument (NULL)
 * @param [out] bev : 수신 Message (EOF Event시에 bev를 Free 함)
 *
 * @retval : None
 */

void ipcmgrEventCallback(struct bufferevent *bev, Int16T events, void *ctx)
{
    Int32T i;
    Int32T process = -1;
    Int32T fd = -1;;
    Int32T inputFd = bufferevent_getfd(bev);

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_DEBUG, "Event Callback Operation\n");
    nLOG(LOG_INFO, "inputFd = %d\n", inputFd);
    nLOG(LOG_DEBUG, "################################\n");

    nLOG(LOG_DEBUG, "################################\n");
    for(i=0; i<MAX_PROCESS_CNT; i++)
    {
        if(gIpcCommMgt[i].bev != NULL)
        {
            nLOG(LOG_DEBUG, "[%d] acceptFd : %d, receviedFd : %d\n",
             i, gIpcCommMgt[i].acceptFd, bufferevent_getfd(gIpcCommMgt[i].bev));
        }
    }
    nLOG(LOG_DEBUG, "################################\n");

    for(i=0; i<MAX_PROCESS_CNT; i++)
    {
        if(gIpcCommMgt[i].bev != NULL)
        {
            if((inputFd == (bufferevent_getfd(gIpcCommMgt[i].bev)))
                       ||
               (inputFd == gIpcCommMgt[i].acceptFd))
            {
                fd = gIpcCommMgt[i].acceptFd;
                process = i;
                nLOG(LOG_INFO, "[%d] AcceptFd : %d, Process : %d\n",
                                 i, fd, process);
                break;
            }
        }
    }

    if(i == MAX_PROCESS_CNT)
    {
        nLOG(LOG_INFO, "Not Match Fd\n");
        fd = inputFd;
        nLOG(LOG_INFO, "fd : %d, process : %d\n");
    }

    nLOG(LOG_INFO, "ipcmgrEventCallback fd : %d\n", fd);

    if (events & BEV_EVENT_ERROR)
    {
        nLOG(LOG_WARNING, "Error from bufferevent\n");
    }
    else if (events & BEV_EVENT_EOF)
    {
        // JAESU HAN CHECK
        nLOG(LOG_DEBUG, "BEV_EVENT_EOF\n");
        nLOG(LOG_INFO, "Event : %d\n", events);
      
        if(bev != NULL)
        {
            bufferevent_free(bev);
            bev = NULL;
            nLOG(LOG_INFO, "Bev != NULL is Free\n");
        }
        
        nLOG(LOG_INFO, "fd : %d, process : %d\n");

        if(fd > 0 || process != -1)
        {
            nLOG(LOG_DEBUG, "Fd(%d) > 0 || process(%d) != -1\n", fd, process); 

            ipcmgrEventDisconnSubscribeDeregister(fd);
            nLOG(LOG_INFO, "Disconnect Process : Event Subscibe Deregist\n");

            if(ipcmgrProcessInfoDelete(fd) < 0)
            {
                nLOG(LOG_INFO, "Process Info Delete Failuare\n");

                if(gIpcCommMgt[process].bev != NULL)
                {
                    bufferevent_free(gIpcCommMgt[process].bev);
                    gIpcCommMgt[process].bev = NULL;
                    nLOG(LOG_INFO, "gIpcCommMgt[%d].bev != NULL is Free\n", 
                                        process);
                }
            }

            nLOG(LOG_INFO, "Process Info Delete Success\n");

            ipcmgrProcessInfoSend();
            
        }
        nLOG(LOG_INFO, "Client Connect End\n");
    }
    nLOG(LOG_INFO, "events : %d\n", events);
}	


/*
 * Description: IPC Manager Channel 을 Open 하는 함수
 *
 * @retval : None
 */



void ipcmgrChannelOpen(pIpcRegFunc recvFunc)
{
    struct sockaddr_in sin;
  
    // Socket Bind Info Setup

    memset(&sin, 0x00, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(IPC_MANAGER_IP);
    sin.sin_port = htons(IPC_MANAGER_PORT);

    nLOG(LOG_INFO, "Socket Bind Info Setup, IP : %s, Port : %d\n",
                    IPC_MANAGER_IP, IPC_MANAGER_PORT);

    

    //  Bind and Client Connet, Accept Conn Function Registeration

    gListener = evconnlistener_new_bind
                (gTaskService, ipcmgrChannelAccept, NULL,
                 LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,
                 (struct sockaddr*)&sin, sizeof(sin));

    if (!gListener)
    {
        nLOG(LOG_DEBUG, "################################\n");
        nLOG(LOG_ERR, "IPC Manager Socket Bind Failure\n");
        nLOG(LOG_DEBUG, "################################\n");
        exit(-1);
    }
    nLOG(LOG_INFO, "IPC Manager Bind Success\n");
 
    evconnlistener_set_error_cb(gListener, ipcmgrAcceptErrorFunc);
    
    ipcmgrProcessInfoInit();
    gUserProcess = IPC_MANAGER;
    gIpcCb = recvFunc;

    gInitCb();
}

/*
 * Description: Accpet 연결된 Process 를 처리하는 함수
 *
 * @param [in] ipcListener : Listener
 * @param [in] fd : 연결 Process fd
 * @param [in] address : 연결 정보 (Address)
 * @param [in] socklen : Sock 데이터 길이
 * @param [in] ctx : user's argument(none)
 *
 * @retval : None
 */

void ipcmgrChannelAccept
(struct evconnlistener *ipcListener, evutil_socket_t fd, 
 struct sockaddr *address, int socklen, void *ctx)
{

    // 1. New Connect libevent Registration

    Int32T connCnt; 
    struct bufferevent *newBev = bufferevent_socket_new
                                 (gTaskService, fd, BEV_OPT_CLOSE_ON_FREE);

    nLOG(LOG_DEBUG, "################################\n");    
    nLOG(LOG_INFO, "accept_conn_cb : %u.%u.%u.%u, %d\n",
                     IP_PRINT(((struct sockaddr_in*)address)->sin_addr.s_addr),
                              ((struct sockaddr_in*)address)->sin_port);
    nLOG(LOG_DEBUG, "################################\n");


    // 2. New Connect Info Registration

    if((connCnt = ipcmgrGetAcceptPoint()) == FAILURE)
    {
        nLOG(LOG_DEBUG, "################################\n");
        nLOG(LOG_INFO, "ipcmgrGetAcceptPoint Failure\n");
        nLOG(LOG_DEBUG, "################################\n");
        return ;
    }
    nLOG(LOG_DEBUG, "connCnt : %d\n", connCnt);

    gIpcAcceptMgt[connCnt].fd = fd;
    memcpy(&gIpcAcceptMgt[connCnt].addr, address, sizeof(struct sockaddr));

    nLOG(LOG_DEBUG, "gIpcAcceptMgt[%d].fd : %d\n",  
                     connCnt, gIpcAcceptMgt[connCnt].fd);

    nLOG(LOG_DEBUG, "gIpcAcceptMgt[%d].addr,sa_family : %d\n", 
                     connCnt, gIpcAcceptMgt[connCnt].addr.sa_family);

    nLOG(LOG_DEBUG, "gIpcAcceptMgt[%d].addr : %u.%u.%u.%u, %d\n",
         connCnt,
         IP_PRINT
         (((struct sockaddr_in*)&gIpcAcceptMgt[connCnt].addr)->sin_addr.s_addr),
          ((struct sockaddr_in*)&gIpcAcceptMgt[connCnt].addr)->sin_port);

   
    // 3. Read and Event Callback Registration

    bufferevent_setcb
        (newBev, ipcmgrReadCallback, NULL, ipcmgrEventCallback, NULL);


    // 4. Bufferevent flag enable

    bufferevent_enable (newBev, EV_READ);

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "Accept Process Success\n");
    nLOG(LOG_DEBUG, "################################\n");
}

/**
 * Description: Accpet 실패시 처리하는 함수
 *
 * @param [in] ipcListener : Listener
 * @param [in] ctx : user's argument(none)
 *
 * @retval : None
 */

void ipcmgrAcceptErrorFunc(struct evconnlistener *ipcListener, void *ctx)
{
    Int32T errno = EVUTIL_SOCKET_ERROR();

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_ERR, "Accept Process Failure : errno - %d\n", errno);
    nLOG(LOG_DEBUG, "################################\n");

    // IPC MANAGER EXIT
    event_base_loopexit (gTaskService, NULL);
}

/*
 * Description: IPC Manager에서 사용하는 Signal Task를 설정하는 함수
 *
 * @retval : SUCCESS(0) : 성공,
 *           FAILURE(-1) : 실패 (Libevent 생성 or 초기화 실패
 */

Int32T ipcmgrSignalInit(void)
{
    struct event *sigInt;
    struct event *sigSegv;
    struct event *sigTerm;
    struct event *sigHup;
    struct event *sigUsr1;
    struct event *sigKill;

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "Signal Task Init Start\n");
    nLOG(LOG_DEBUG, "################################\n");


    sigInt = evsignal_new(gTaskService, SIGINT, ipcmgrSignalCallback,
                               (void *) gTaskService);

    sigSegv = evsignal_new(gTaskService, SIGSEGV,  ipcmgrSignalCallback,
                               (void *) gTaskService);

    sigTerm = evsignal_new(gTaskService, SIGTERM, ipcmgrSignalCallback,
                               (void *) gTaskService);

    sigHup = evsignal_new(gTaskService, SIGHUP, ipcmgrSignalCallback,
                               (void *) gTaskService);

    sigUsr1 = evsignal_new(gTaskService, SIGUSR1, ipcmgrSignalCallback,
                               (void *) gTaskService);

    sigKill = evsignal_new(gTaskService, SIGABRT, ipcmgrSignalCallback,
                               (void *) gTaskService);


    if(!sigInt || event_add (sigInt, NULL) < 0)
    {
        nLOG(LOG_DEBUG, "################################\n");
        nLOG(LOG_ERR, "SIGINT Signal Regist Failure\n");
        nLOG(LOG_DEBUG, "################################\n");
        return FAILURE;
    }

    if(!sigSegv || event_add (sigSegv, NULL) < 0)
    {
        nLOG(LOG_DEBUG, "################################\n");
        nLOG(LOG_ERR, "SIGSEGV Signal Regist Failure\n");
        nLOG(LOG_DEBUG, "################################\n");
        return FAILURE;
    }
   
    if(!sigTerm || event_add (sigTerm, NULL) < 0)
    {
        nLOG(LOG_DEBUG, "################################\n");
        nLOG(LOG_ERR, "SIGTERM Signal Regist Failure\n");
        nLOG(LOG_DEBUG, "################################\n");
        return FAILURE;
    }
  
    if(!sigHup || event_add (sigHup, NULL) < 0)
    {
        nLOG(LOG_DEBUG, "################################\n");
        nLOG(LOG_ERR, "SIGHUP Signal Regist Failure\n");
        nLOG(LOG_DEBUG, "################################\n");
        return FAILURE;
    }
 
    if(!sigUsr1 || event_add (sigUsr1, NULL) < 0)
    {
        nLOG(LOG_DEBUG, "################################\n");
        nLOG(LOG_ERR, "SIGUSR1 Signal Regist Failure\n");
        nLOG(LOG_DEBUG, "################################\n");
        return FAILURE;
    }

    if(!sigKill || event_add (sigKill, NULL) < 0)
    {
        nLOG(LOG_DEBUG, "################################\n");
        nLOG(LOG_ERR, "SIGABRT Signal Regist Failure\n");
        nLOG(LOG_DEBUG, "################################\n");
        return FAILURE;
    }


/*
    ret = taskSignalSetAggr(ipcmgrSignalOperation);
    if(ret < 0)
    {
        return FAILURE;
    }

    ret = taskSignalSet(SIGSEGV, ipcmgrSignalCallback);
    if(ret < 0)
    {
        return FAILURE;
    }
*/

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "Signal Task Init Success\n");
    nLOG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}

/*
 * Description: 내부 Signal Callback 함수
 *
 * @param [in] sig : signal의 종류
 * @param [in] events : signal flags
 * @param [in] data : user's data
 *
 * @retval : None
 */

void ipcmgrSignalCallback
(evutil_socket_t sig, Int16T events, void * data)
{
    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "Signal Callback\n");
    nLOG(LOG_ERR, "Signal : %d, events : %d\n", sig, events);
    nLOG(LOG_DEBUG, "################################\n");

    ipcmgrSignalOperation(sig);
}

/*
 * Description: 분류한 Signal을 처리하는 함수
 *
 * @param [in] sig : signal의 종류
 *
 * @retval : None
 */

void ipcmgrSignalOperation (Int32T sig)
{
    struct timeval delay = {1, 0};

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_ERR, "Signal Number : %d\n", sig);
    nLOG(LOG_ERR, "Caught an interrupt signal exiting cleanly in on second\n");

    event_base_loopexit (gTaskService, &delay);

    nLOG(LOG_INFO, "gTaskService LoopExit\n");
    nLOG(LOG_DEBUG, "################################\n");

   exit(1);
}

/*
 * Description: 연결이 끊어진 Process의 Event Subscribe 목록을 삭제하는 함수
 *
 * @param [in] fd : 연결이 끊어진 Process의 fd
 *
 * @retval : None
 */

void ipcmgrEventDisconnSubscribeDeregister(Int32T fd)
{
    EventSubMsgT eventData;
    Int32T i, process;

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "DisConnect Process Deregister\n");
    nLOG(LOG_INFO, "Input Fd : %d\n", fd);
    nLOG(LOG_DEBUG, "################################\n");

    for(i=1; i<MAX_PROCESS_CNT; i++)
    {
        if(fd == gIpcCommMgt[i].acceptFd)
        {
            process = i;
            nLOG(LOG_INFO, "DisConnect Process : %d\n", process);
            break;
        }
    }

    if(i == MAX_PROCESS_CNT)
    {
        nLOG(LOG_DEBUG, "################################\n");
        nLOG(LOG_WARNING, "Process is Not Regist\n");
        nLOG(LOG_DEBUG, "################################\n");
        return ;
    }

    nLOG(LOG_INFO, "DisConnect Process Deregister Start\n");

    eventData.process = process;
    gEventCb(IPCMGR_EVENT_PROCESS_EOF, &eventData, sizeof(EventSubMsgT));   
 
    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "DisConnect Process Deregister Complete\n");
    nLOG(LOG_DEBUG, "################################\n");
}


