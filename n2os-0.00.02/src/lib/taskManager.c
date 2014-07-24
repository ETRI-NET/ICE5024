/*******************************************************************************
 *            Electronics and Telecommunications Research Institute
 *            No part of this software shall be reproduced, stored in a
 *            retrieval system, or transmitted by any means, electronic,
 *            mechanical, photocopying, recording, or otherwise, without
 *            written permission from ETRI.
*******************************************************************************/
/**
 * @brief : Task Scheduling Serivce Library
 *  - Block Name : Library
 *  - Process Name :
 *  - Creator : JaeSu Han
 *  - Initial Date : 2013/7/15
*/

/**
 * @file : taskManager.c
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
#include <errno.h>
#include "taskManager.h"

/*******************************************************************************
 *                              GLOBAL VARIABLES
 ******************************************************************************/


/*******************************************************************************
 *                         GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/

extern Int32T dluTaskModuleUpdate (void * data);

/*******************************************************************************
 *                         LOCAL CONSTANTS/LITERALS/TYPES
 ******************************************************************************/

/** Message Type String */
const Int8T sIpcMsgTypeName[][32] =
{
    "N2OS_INIT_COMPLETE",
    "COMPONENT_ACCEPT_OK",
    "IPC_PROCESS_REG",
    "IPC_PROCESSINFO_PUT",
    "IPC_BIND_COMPLETE",
    "IPC_BIND_COMPLETE_ACK",
    "IPC_PROCESSINFO_UPDATE",
    "IPC_PROC_ALL_INFO_REQ",
    "IPC_PROC_ALL_INFO_UPDATE",
    "IPC_PROC_SIGNAL_INFO_REQ",
    "IPC_PROC_SINGLE_INFO_UPDATE",
    "IPC_SERVICE",
    "EVENT_SUBSCRIBE",
    "EVENT_UNSUBSCRIBE",
    "EVENT_SERVICE"
};


/*******************************************************************************
*                               LOCAL VARIABLES
*******************************************************************************/


/*******************************************************************************
 *                            LOCAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/

static void ipcInterSignalCb(Int32T fd, Int16T event, void * arg);

/*******************************************************************************
 *                                   CODE
 ******************************************************************************/

/*******************************************************************************
 * Function: <ipcInterReadCb>  
 *  
 * Description: <IPC & EVENT Message Received, Internal Operation>  
 *  
 * Parameters: bev : message buffer  
 *             ctx : user's argument  
 *  
 * Returns: none  
*******************************************************************************/

void ipcInterReadCb(struct bufferevent *bev, void *ctx)
{

    // 1. Check the received message

    struct evbuffer *input = bufferevent_get_input(bev);
    Int32T evBuffLen = evbuffer_get_length(input);
    Int32T fd = bufferevent_getfd(bev);
    Int32T process = ipcGetProcess(fd);

    
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO,
               "Send Message Fd : %d, evBuffLen : %d\n", fd, evBuffLen);
    nLOGDEBUG(LOG_INFO, "Send Message Process : %d\n", process);
    nLOGDEBUG(LOG_INFO, "gIpcCommMgt[%d].bufSize : %d\n",
                     process, gIpcCommMgt[process].bufSize);
    nLOGDEBUG(LOG_DEBUG, "################################\n");


    // If the process has not been registered yet
    if (process == -1)
    {
        process = IPC_MANAGER;
        nLOGDEBUG(LOG_DEBUG, "Process has not been registered yet\n");
        nLOGDEBUG(LOG_INFO, 
                   "Process = %d : This is a temporary\n", IPC_MANAGER);
    }


    // 2. Message Paring and Operation

    if ((evBuffLen + gIpcCommMgt[process].bufSize) > 0)
    {
        Int32T readLen = 0;
        Int32T totalLen = 0;
        Int8T evBuff[IPC_MAX_MESSAGE_SIZE] = {0,};
        Int8T msgBuff[IPC_MAX_MESSAGE_SIZE * 2] = {0,};

        nLOGDEBUG(LOG_DEBUG, "evBuffLen + gIpcCommMgt[%d].bufSize = %d\n",
                          process, evBuffLen + gIpcCommMgt[process].bufSize);
 

        // 2.1 Received Message Copy

        evbuffer_copyout(input, evBuff, evBuffLen);
        nLOGDEBUG(LOG_INFO, "Received Message Copy\n");
        nLOGDEBUG(LOG_DEBUG, "evBuffLen : %d\n", evBuffLen);

        if(gIpcCommMgt[process].bufSize > 0)
        {
            nLOGDEBUG(LOG_DEBUG, "gIpcCommMgt[%d].bufSize = %d\n",
                             process, gIpcCommMgt[process].bufSize);
            memcpy(msgBuff,
                   gIpcCommMgt[process].buf,
                   gIpcCommMgt[process].bufSize);

            nLOGDEBUG(LOG_INFO, 
                       "gIpcCommMgt[%d].buf -> msgBuff : Copy Buffer\n",
                        process);
        }

        memcpy(msgBuff + gIpcCommMgt[process].bufSize, evBuff, evBuffLen);
        nLOGDEBUG(LOG_INFO, "evBuff -> msgBuff : Copy Buffer\n");

        totalLen = gIpcCommMgt[process].bufSize + evBuffLen;
        nLOGDEBUG(LOG_DEBUG, "Message Total Len : %d\n", totalLen);

        if(gIpcCommMgt[process].bufSize > 0)
        {
            nLOGDEBUG(LOG_INFO, "Copyed Buffer Free\n");
            nFREE(IPC_MESSAGE, gIpcCommMgt[process].buf);
            gIpcCommMgt[process].bufSize = 0;
            gIpcCommMgt[process].buf = NULL;

            nLOGDEBUG(LOG_DEBUG, "gIpcCommMgt[%d].bufSize = %d\n",
                              process, gIpcCommMgt[process].bufSize);
            nLOGDEBUG(LOG_DEBUG, "gIpcCommMgt[%d].bug = %d\n",
                              process, gIpcCommMgt[process].buf);
        }


        // 2.2 Message Paring & Operation

        while (1)
        {

            // 2.2.1 Message Paring
            Int32T msgLen = 0;
            IpcMsgHeaderT * pMsg = (IpcMsgHeaderT *)0;

            memcpy(&msgLen, msgBuff+(readLen+sizeof(Uint16T)), sizeof(Uint16T));

            nLOGDEBUG(LOG_DEBUG, "Unit Message Len : %d\n", msgLen);
            nLOGDEBUG(LOG_INFO, "Unit Message Create\n");


            // Received Message Create

            pMsg = (IpcMsgHeaderT *)nMALLOC(IPC_MESSAGE, msgLen);
            if(pMsg <= 0)
            {
                nLOGDEBUG(LOG_DEBUG, "################################\n");
                nLOGDEBUG(LOG_ERR, "Received Message Create Failure\n");
                nLOGDEBUG(LOG_DEBUG, "################################\n");
                return ;
            }

            memcpy(pMsg, msgBuff + readLen, msgLen);
            nLOGDEBUG(LOG_INFO, "Unit Message Copy Complete\n");


            // if) Remind Message Length < IPC_MSG_HEADER_SIZE

            if((msgLen < IPC_MSG_HEADER_SIZE)
                            || 
               (pMsg->length > (totalLen - readLen)))
            {
                nLOGDEBUG(LOG_INFO, 
                           "Unit Message Len < IPC_MSG_HEADER_SIZE\n");

                gIpcCommMgt[process].bufSize = totalLen-readLen;

                nLOGDEBUG(LOG_INFO, 
                           "pMsg->length : %d, totalLen - readLen : %d\n",
                            pMsg->length, totalLen - readLen);

                gIpcCommMgt[process].buf = 
                    nMALLOC(IPC_MESSAGE, gIpcCommMgt[process].bufSize);
                if(gIpcCommMgt[process].buf <= 0)
                {
                    nLOGDEBUG(LOG_DEBUG, "################################\n");
                    nLOGDEBUG(LOG_ERR, "Keep Message Create Failure\n");
                    nLOGDEBUG(LOG_DEBUG, "################################\n");
                    return ;
                }

                memcpy(gIpcCommMgt[process].buf, 
                       msgBuff + readLen, 
                       gIpcCommMgt[process].bufSize);

                nLOGDEBUG(LOG_INFO, 
                           "msgBuff->gIpcCommMgt[%d].buf : Copy Buffer\n",
                            process);
                nLOGDEBUG(LOG_INFO, "Copy Len : %d\n",
                                 gIpcCommMgt[process].bufSize);

                nFREE(IPC_MESSAGE, pMsg);

                break;
            }

            readLen += msgLen;

            nLOGDEBUG(LOG_INFO, "Current Read Len : %d\n", readLen);

            
            // 2.3 Message Operation & Free

            ipcInterReadOperation(pMsg, fd, &process);
            nFREE(IPC_MESSAGE, pMsg);

            if ((totalLen - readLen) == 0)
            {
                nLOGDEBUG(LOG_DEBUG, "################################\n");
                nLOGDEBUG(LOG_INFO,"TotalLen : %d, ReadLen : %d\n",
                                totalLen, readLen);
                nLOGDEBUG(LOG_DEBUG, "################################\n");
                break;
            }
        }
    }
    else
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_WARNING, 
                   "evBuffLen : %d, gIpcCommMgt[%d].bufSize : %d\n",
                    evBuffLen, gIpcCommMgt[process].bufSize);
        nLOGDEBUG(LOG_WARNING, "recevied message len = 0\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");
    }

    // 2.3 remove the input buffer's data

    evbuffer_drain(input, evBuffLen);

}

/*******************************************************************************
 * Function: <ipcInterReadOperation>  
 *  
 * Description: <IPC & EVENT Message, Internal Operation>  
 *  
 * Parameters: pMsg    : IPC & EVENT Message  
 *             fd      : Send Process Information  
 *             process : Send Process  
 *  
 * Returns: none  
*******************************************************************************/

void ipcInterReadOperation(IpcMsgHeaderT * pMsg, Int32T fd, Int32T *process)
{
    Uint32T dataLen = pMsg->length - IPC_MSG_HEADER_SIZE;

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Message Operation\n");
    nLOGDEBUG(LOG_INFO, "pMsg->type : %s\n \
                     pMsg->length : %d\n \
                     pMsg->msgId : %d\n",
                     sIpcMsgTypeName[pMsg->type], pMsg->length, pMsg->msgId);
    nLOGDEBUG(LOG_INFO, "Data Length : %d\n", dataLen);
    nLOGDEBUG(LOG_DEBUG, "################################\n"); 


    if (pMsg->type == COMPONENT_ACCEPT_OK)
    {
        static Int32T count = 0;
        Int32T ret;

        if(count == 0)
        {
            ret = ipcConnectRegist(gUserProcess, IPC_MANAGER);

            if(ret < 0)
            {
                nLOGDEBUG(LOG_DEBUG, "################################\n");
                nLOGDEBUG(LOG_ERR, "Send Regist Message (To. IPC Manager) Failure\n");
                nLOGDEBUG(LOG_DEBUG, "################################\n");
                return;
            }

            nLOGDEBUG(LOG_DEBUG, "################################\n");
            nLOGDEBUG(LOG_INFO, "Send Regist Message (To. IPC Manager) Success\n");
            nLOGDEBUG(LOG_DEBUG, "################################\n");
            count++;
        }
 
    }
    else if (pMsg->type == IPC_PROCESSINFO_PUT)
    {
        IpcInfoPutMsgT msgData;
        Int32T ret;

        nLOGDEBUG(LOG_INFO, "pMsg->type : IPC_PROCESSINFO_PUT\n");

        memset(msgData.sIpAddr, 0x00, sizeof(msgData.sIpAddr));
        strncpy(msgData.sIpAddr,
                ((IpcInfoPutMsgT *)pMsg->data)->sIpAddr,
                strlen(((IpcInfoPutMsgT *)pMsg->data)->sIpAddr));
        msgData.port = ((IpcInfoPutMsgT *)pMsg->data)->port;

        nLOGDEBUG(LOG_DEBUG, "Recevied Ip : %s, %d\n",
                         msgData.sIpAddr, msgData.port);

        // Process connect info reg. func()
        ret = ipcChannelConnect(msgData.sIpAddr, msgData.port);
        if (ret != SUCCESS)
        {
            nLOGDEBUG(LOG_DEBUG, "################################\n");
            nLOGDEBUG(LOG_ERR, 
                       "IPC Channel Connect Failure - ret : %d\n", ret);
            nLOGDEBUG(LOG_ERR, "Process Exit\n");
            nLOGDEBUG(LOG_DEBUG, "################################\n");
            exit(1);
        }
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_INFO, "IPC Channel Connect Success\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");
    }
    else if (pMsg->type == IPC_PROCESSINFO_UPDATE)
    {
        struct sockaddr msgData[MAX_PROCESS_CNT];

        nLOGDEBUG(LOG_INFO, "pMsg->type : IPC_PROCESSINFO_UPDATE\n");
    
        memcpy(msgData, pMsg->data, sizeof(msgData));
        ipcProcessConnectListUpdate(msgData);

        nLOGDEBUG(LOG_INFO, "Process Connect Info Update End\n");
    }
    else if (pMsg->type == IPC_BIND_COMPLETE_ACK)
    {
        nLOGDEBUG(LOG_INFO, "pMsg->type : IPC_BIND_COMPLETE_ACK\n");
        nLOGDEBUG(LOG_INFO, "User-Defined Init-Function Call\n");
        gInitCb();
    }
    else if (pMsg->type == IPC_PROC_ALL_INFO_UPDATE)
    {
        struct sockaddr msgData[MAX_PROCESS_CNT];

        nLOGDEBUG(LOG_INFO, "pMsg->type : IPC_PROC_ALL_INFO_UPDATE\n");
    
        memcpy(msgData, pMsg->data, sizeof(msgData));
        ipcProcessConnectListUpdate(msgData);

        nLOGDEBUG(LOG_INFO, "Process Connect Info Update End\n");
    }
    else if (pMsg->type == IPC_PROC_SINGLE_INFO_UPDATE)
    {
        //memset(msgData, 0x00, sizeof(struct sockaddr));

        nLOGDEBUG(LOG_INFO, "pMsg->type : IPC_PROC_SINGLE_INFO_UPDATE\n");
    
        //memcpy(msgData, pMsg->data, sizeof(struct sockaddr));
        //ipcProcessConnectListUpdate(msgData);

        nLOGDEBUG(LOG_INFO, "Process Connect Info Update End\n");
    }
    else if (pMsg->type == IPC_PROCESS_REG)
    {
        IpcRegMsgT msgData;

        nLOGDEBUG(LOG_INFO, "pMsg->type : IPC_PROCESS_REG\n");

        msgData.process = ((IpcRegMsgT *)pMsg->data)->process;
        nLOGDEBUG(LOG_INFO, 
                   "Regist Process : %d, fd : %d\n", msgData.process, fd);

        ipcProcessConnectListUpdateFd(fd, msgData.process);
        *process = msgData.process;

        nLOGDEBUG(LOG_INFO, "Process Regist End\n");
    }
    else if (pMsg->type == EVENT_SERVICE)
    {
        nLOGDEBUG(LOG_INFO, "pMsg->type : EVENT_SERVICE\n");

        if(gEventCb != NULL)
        {
            nLOGDEBUG(LOG_INFO, "Event Callback Function call\n");
            gEventCb(pMsg->msgId, pMsg->data, dataLen);
        }
        else
        {
            nLOGDEBUG(LOG_ERR, "Event Callback Function is NULL\n");
        }
    }
    else if (pMsg->type == IPC_SERVICE)
    {
        nLOGDEBUG(LOG_INFO, "pMsg->type : IPC_SERVICE\n");
 
        if(gIpcCb != NULL)
        {
            nLOGDEBUG(LOG_INFO, "IPC Callback Function call\n");
            gIpcCb(pMsg->msgId, pMsg->data, dataLen);
        }
        else
        {
            nLOGDEBUG(LOG_INFO, "IPC Callback Function is NULL\n");
        }
    }
    else
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "None Message Type\n");
        nLOGDEBUG(LOG_ERR, "pMsg->type : %s\n \
                        pMsg->length : %d\n \
                        pMsg->msgId : %d\n",
                       sIpcMsgTypeName[pMsg->type], pMsg->length, pMsg->msgId);
        nLOGDEBUG(LOG_DEBUG, "################################\n"); 
   }
}

/*******************************************************************************
 * Function: <ipcInterWriteCb>  
 *  
 * Description: <Write Callback, Internal Operation>  
 *  
 * Parameters: bev : message buffer  
 *             ctx : user's argument  
 *  
 * Returns: none  
*******************************************************************************/

void ipcInterWriteCb(struct bufferevent *bev, void *ctx)
{
    struct evbuffer *output = bufferevent_get_output(bev);

    if (evbuffer_get_length(output) == 0)
    {
        bufferevent_free(bev);
    }
}

/*******************************************************************************
 * Function: <ipcInterEventCb>  
 *  
 * Description: <'Libevent' Library Event Callback, Internal Operation>  
 *  
 * Parameters: bev : message buffer  
 *             events : flags  
 *             ctx : user's argument  
 *  
 * Returns: none  
*******************************************************************************/

void ipcInterEventCb(struct bufferevent *bev, short events, void *ctx)
{
    Int32T i;
    Int32T inputFd = bufferevent_getfd(bev);
    Int32T fd = -1;
    Int32T process = -1;


    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_DEBUG, "Event Callback Operation\n");
    nLOGDEBUG(LOG_INFO, "inputFd = %d\n", inputFd);
    nLOGDEBUG(LOG_DEBUG, "################################\n");

#if 0
    for(i=0; i<MAX_PROCESS_CNT; i++)
    {
        if(gIpcCommMgt[i].bev != NULL)
        {
            if(inputFd == gIpcCommMgt[i].acceptFd)
            {
                process = i;
                fd = gIpcCommMgt[i].acceptFd;
                nLOGDEBUG(LOG_INFO, "[%d] AcceptFd : %d, Process : %d\n",
                                 i, fd, process);
                break;
            }
        }
    }
#endif

    for(i=0; i<MAX_PROCESS_CNT; i++)
    {
        if(gIpcCommMgt[i].bev != NULL)
        {
            if((inputFd == (bufferevent_getfd(gIpcCommMgt[i].bev)))
                       ||
               (inputFd == gIpcCommMgt[i].acceptFd))
            {
                if(gIpcCommMgt[i].acceptFd == -1)
                {
                    fd = bufferevent_getfd(gIpcCommMgt[i].bev);
                }
                else 
                {
                    fd = gIpcCommMgt[i].acceptFd;
                }            
                process = i;
                nLOGDEBUG(LOG_INFO, "[%d] AcceptFd : %d, Process : %d\n",
                                 i, fd, process);
                break;
            }
        }
    }

    if(i == MAX_PROCESS_CNT)
    {
        nLOGDEBUG(LOG_INFO, "Not Match Fd\n");
        fd = inputFd;
        nLOGDEBUG(LOG_INFO, "fd : %d, process : %d\n");
    }

    nLOGDEBUG(LOG_INFO, "ipcEventCallback fd : %d\n", fd);

    if (events & BEV_EVENT_CONNECTED)
    {
        Int32T ret;
        nLOGDEBUG(LOG_DEBUG, "BEV_EVENT_CONNECTED\n");

        if(process == IPC_MANAGER)
        {
            ret = ipcConnectRegist(gUserProcess, IPC_MANAGER);

            if(ret < 0)
            {
                nLOGDEBUG(LOG_DEBUG, "################################\n");
                nLOGDEBUG(LOG_ERR, "Send Regist Message (To. IPC Manager) Failure\n");
                nLOGDEBUG(LOG_DEBUG, "################################\n");
                return;
            }

            nLOGDEBUG(LOG_DEBUG, "################################\n");
            nLOGDEBUG(LOG_INFO, "Send Regist Message (To. IPC Manager) Success\n");
            nLOGDEBUG(LOG_DEBUG, "################################\n");
        }
    }
    else if (events & BEV_EVENT_ERROR)
    {
        nLOGDEBUG(LOG_WARNING, "Error from bufferevent\n");

        nLOGDEBUG(LOG_INFO, "Got an error from (%d) %s\n",
            EVUTIL_SOCKET_ERROR(),
            evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));

        if(process == IPC_MANAGER)
        {
            sleep(1);
            bufferevent_free(gIpcCommMgt[process].bev);
            if(ipcProcessConnect(IPC_MANAGER, IPC_MANAGER_IP, IPC_MANAGER_PORT) < 0)
            {
                nLOGDEBUG(LOG_DEBUG, "################################\n");
                nLOGDEBUG(LOG_ERR, "IPC Manager Connect Failure\n");
                nLOGDEBUG(LOG_DEBUG, "################################\n");
                return ;      // -2
            }
        }

    }
    else if (events & BEV_EVENT_EOF)
    {
        nLOGDEBUG(LOG_DEBUG, "BEV_EVENT_EOF\n");
        nLOGDEBUG(LOG_INFO, "Event : %d\n", events);

        if(bev != NULL)
        {
            bufferevent_free(bev);
            bev = NULL;
            nLOGDEBUG(LOG_INFO, "Bev != NULL is Free\n");
        }

        nLOGDEBUG(LOG_INFO, "fd : %d, process : %d\n");


        if(fd > 0 || process != -1)
        {
            nLOGDEBUG(LOG_DEBUG, "Fd(%d) > 0 || process(%d) != -1\n", fd, process);

            if(ipcProcessInfoDelete(fd) < 0)
            {
                nLOGDEBUG(LOG_INFO, "Process Info Delete Failuare\n");

                if(gIpcCommMgt[process].bev != NULL)
                {
                    bufferevent_free(gIpcCommMgt[process].bev);
                    gIpcCommMgt[process].bev = NULL;
                    nLOGDEBUG(LOG_INFO, "gIpcCommMgt[%d].bev != NULL is Free\n",
                                        process);
                }
            }

            nLOGDEBUG(LOG_INFO, "Process Info Delete Success\n");
        }
        nLOGDEBUG(LOG_INFO, "Client Connect End\n");

        if(process == IPC_MANAGER)
        {
            sleep(1);
            if(ipcProcessConnect(IPC_MANAGER, IPC_MANAGER_IP, IPC_MANAGER_PORT) < 0)
            {
                nLOGDEBUG(LOG_DEBUG, "################################\n");
                nLOGDEBUG(LOG_ERR, "IPC Manager Connect Failure\n");
                nLOGDEBUG(LOG_DEBUG, "################################\n");
                return ;      // -2
            }
        }

#if 0
        if(fd > 0)
        {
            nLOGDEBUG(LOG_DEBUG, "fd(%d) > 0, process : %d\n", fd, process);
            bufferevent_free(bev);
            bev = NULL;
            nLOGDEBUG(LOG_INFO, "Bev is Free\n");

            if(process == IPC_MANAGER)
            {
                nLOGDEBUG(LOG_DEBUG, "Process is IPC_MANAGER\n");
                free(gIpcCommMgt[IPC_MANAGER].bev);
                gIpcCommMgt[IPC_MANAGER].bev = NULL;
                nLOGDEBUG(LOG_INFO, "gIpcCommMgt[IPC_MANAGER].bev is NULL\n");
            }
        }

        nLOGDEBUG(LOG_INFO, "Client Connect End\n");
#endif 

   }
    
    nLOGDEBUG(LOG_INFO, "events : %d\n", events);
}

/*******************************************************************************
 * Function: <ipcInterSignalCb>  
 *  
 * Description: <'Libevent' Library Signal Callback, Internal Operation>  
 *  
 * Parameters: fd     : signal fd  
 *             events : flags  
 *             ctx : user's argument  
 *  
 * Returns: none  
*******************************************************************************/

static void ipcInterSignalCb(Int32T fd, Int16T event, void * arg)
{
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Signal Callback Operation\n");
    nLOGDEBUG(LOG_DEBUG, "fd : %d, event : %d\n", fd, event);
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    gSignalCb(fd);   
}

/*******************************************************************************
 * Function: <ipcChannelAccept>  
 *  
 * Description: <Accept Connect Process>  
 *  
 * Parameters: ipcListener : Listener  
 *             fd : Connect process fd  
 *             address : Connect address  
 *             socklen : sock data len  
 *             ctx : user's argument(none)  
 *  
 * Returns: none  
*******************************************************************************/

void ipcChannelAccept
(struct evconnlistener *ipcListener, evutil_socket_t fd, 
 struct sockaddr *address, int socklen, void *ctx)
{

    // 1. New Connect Libevent Registration
  
    Int32T connCnt;
//  struct event_base *newBase = evconnlistener_get_base(gListener);
    struct bufferevent *newBev = bufferevent_socket_new
                                 (gTaskService, fd, BEV_OPT_CLOSE_ON_FREE);

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "accept_conn_cb : %u.%u.%u.%u, %d\n",
                     IP_PRINT(((struct sockaddr_in*)address)->sin_addr.s_addr),
                              ((struct sockaddr_in*)address)->sin_port);
    nLOGDEBUG(LOG_DEBUG, "################################\n");


    // 2. New Connect Info Registration

    if((connCnt = ipcGetAcceptPoint()) == FAILURE)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_INFO, "ipcGetAcceptPoint Failure\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return ;
    }

    nLOGDEBUG(LOG_DEBUG, "connCnt : %d\n", connCnt);

    gIpcAcceptMgt[connCnt].fd = fd;
    memcpy(&gIpcAcceptMgt[connCnt].addr, address, sizeof(struct sockaddr));

    nLOGDEBUG(LOG_DEBUG, "gIpcAcceptMgt[%d].fd : %d\n",
                      connCnt, gIpcAcceptMgt[connCnt].fd);

    nLOGDEBUG(LOG_DEBUG, "gIpcAcceptMgt[%d].addr,sa_family : %d\n",
                      connCnt, gIpcAcceptMgt[connCnt].addr.sa_family);

    nLOGDEBUG(LOG_DEBUG, "gIpcAcceptMgt[%d].addr : %u.%u.%u.%u, %d\n",
        connCnt,
        IP_PRINT
        (((struct sockaddr_in*)&gIpcAcceptMgt[connCnt].addr)->sin_addr.s_addr),
         ((struct sockaddr_in*)&gIpcAcceptMgt[connCnt].addr)->sin_port);


    // 3. Read and Event Callback Registration

    bufferevent_setcb
        (newBev, ipcInterReadCb, NULL, ipcInterEventCb, NULL);

    // 4. Bufferevent flag enable

    bufferevent_enable (newBev, EV_READ);

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Accept Process Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
}

/*******************************************************************************
 * Function: <ipcAcceptErrorFunc>  
 *  
 * Description: <Accept Error>  
 *  
 * Parameters: ipcListener : Listener  
 *             ctx : user's argument(none)  
 *  
 * Returns: none  
*******************************************************************************/

void ipcAcceptErrorFunc(struct evconnlistener *ipcListener, void *ctx)
{
//  struct event_base *newBase = evconnlistener_get_base(gListener);
/*
    Int32T errno = EVUTIL_SOCKET_ERROR();

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_ERR, "Accept Process Failure : errno - %d\n", errno);
    nLOGDEBUG(LOG_DEBUG, "################################\n");
*/
    // IPC MANAGER EXIT
    event_base_loopexit (gTaskService, NULL);
    event_base_free(gTaskService);
    gTaskService = NULL;
}

/**
 * Description: Task Scheduling Service를 생성하고 초기화하는 함수
 *
 * @param [in] initFunc : 사용자 정의 Init 함수
 *
 * @retval : SUCCESS(0) : 성공,
 *           TASK_ERR_LIBEVENT(-1) : Libevent 생성 or 초기화 실패,
 *           TASK_ERR_ARGUMENT(-2) : 잘못된 인자 값
 *
 * @bug
 *  반드시, memInit()를 먼저 실행해야 한다.
 *
 * Example Code Usage
 * @code
 *  void * ribmgrInitFunc(void)
 *  {
 *    ... // Component Init
 *  }
 *
 *  Int32T main(void)
 *  {
 *    Int32T ret;
 *  
 *    ... 
 *    
 *    ret = taskCreate(ribmgrInitFunc); 
 *
 *    ...
 *
 *  }
 * @endcode
 */

Int32T taskCreate (pTaskInitFunc initFunc)
{
    Int32T i;

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "TaskMgt Create Start\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");


    // 1. User-Defined Process Init Function Setting

    if(initFunc == NULL)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "User-Defined Init. Function is Failure\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return TASK_ERR_ARGUMENT;    // -2
    }

    gInitCb = initFunc;
    nLOGDEBUG(LOG_INFO, "User-Defined Init. Function Regist. Success\n");

   
    // 2. Libevent Base Setting (& Priority Set)

    gTaskService = event_base_new();
    if(!gTaskService)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Libevent Init Failure\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return TASK_ERR_LIBEVENT;    // -1
    }
    nLOGDEBUG(LOG_INFO, "Libevent Init. Success\n");

    event_base_priority_init(gTaskService, 3);
    nLOGDEBUG(LOG_INFO, "Task Priority(3) Init. Success\n");

  
    // 3. Aggr Signal Management Structure Init.

    nLOGDEBUG(LOG_INFO, "Aggr Signal Management Structure Init.\n");
    gSignalCb = NULL;

    for(i = 0; i<TASK_SIG_AGGR_CNT; i++)
    {
        gTaskSigAggrMgt[i].flag = TASK_SIGNAL_AVAILABLE;
        gTaskSigAggrMgt[i].taskSig = NULL;
        nLOGDEBUG(LOG_DEBUG, "gTaskSigAggrMgt[%d].flag = %d\n",
                          i, gTaskSigAggrMgt[i].flag);
        nLOGDEBUG(LOG_DEBUG, "gTaskSigAggrMgt[%d].taskSig = %d\n",
                          i, gTaskSigAggrMgt[i].taskSig);
    }  

    gTaskSigAggrMgt[0].sig = SIGINT;
    gTaskSigAggrMgt[1].sig = SIGTERM;
    gTaskSigAggrMgt[2].sig = SIGHUP;
    gTaskSigAggrMgt[3].sig = SIGUSR1;
    gTaskSigAggrMgt[4].sig = SIGSEGV;

    for(i=0; i<TASK_SIG_AGGR_CNT; i++)
    {
        nLOGDEBUG(LOG_DEBUG, "gTaskSigAggrMgt[%d].sig = %d\n",
                          i, gTaskSigAggrMgt[i].sig);
    }

    nLOGDEBUG(LOG_INFO, "Aggr Signal Management Structure Init. Success\n");

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "TaskMgt Create Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;	
}

/**
 * Description: Task Scheduling Service를 Dispatch하는 함수
 *
 * @retval : None
 *
 * @bug
 *  반드시, taskCreate()를 먼저 실행해야 한다.
 *
 * Example Code Usage
 * @code
 *  void * ribmgrInitFunc(void)
 *  {
 *    ... // Component Init
 *  }
 *
 *  Int32T main(void)
 *  {
 *    Int32T ret;
 *  
 *    ... 
 *    
 *    ret = taskCreate(ribmgrInitFunc); 
 *
 *    ...
 *
 *    taskDispatch();
 *
 *    ...
 *
 *  }
 * @endcode
 */

void taskDispatch (void)
{
    event_base_dispatch(gTaskService);

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "TaskMgt Dispatch Start\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

}

/**
 * Description: Task Scheduling Service를 삭제하고 종료하는 함수
 *
 * @retval : None
 *
 * @bug
 *  반드시, taskcreate()를 먼저 실행해야 한다.
 *
 * Example Code Usage
 * @code
 *  void * ribmgrSigCallback(void)
 *  {
 *    ... // Signal 처리
 *
 *    _taskClose();
 *
 *    ...
 *  }
 *
 *  Int32T main(void)
 *  {
 *    Int32T ret;
 *  
 *    ... 
 *    
 *    ret = taskCreate(ribmgrInitFunc); 
 *
 *    ret = _taskSignalSetAggr(ribmgrSigCallback);
 *
 *    ...
 *
 *    taskDispatch();
 *
 *    ...
 *
 *  }
 * @endcode
 */

void _taskClose(void)
{
    Int32T i;

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "TaskMgt Close Start\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    for(i=0; i<MAX_PROCESS_CNT; i++)
    {
        if(gIpcCommMgt[i].bev != NULL)
        {
            bufferevent_free(gIpcCommMgt[i].bev);
            gIpcCommMgt[i].bev = NULL;
        }
        nLOGDEBUG(LOG_DEBUG, "gIpcCommMgt[%d].bev = %d\n",
                          i, gIpcCommMgt[i].bev);
    }

    if(gListener != NULL)
    {
        evconnlistener_free(gListener);
    }

    nLOGDEBUG(LOG_INFO, "gListener is Free\n");

    event_base_loopexit(gTaskService, NULL);
    nLOGDEBUG(LOG_INFO, "TaskMgt loop exit\n");

    if(gTaskService != NULL)
    {
        event_base_free(gTaskService);
    }

    nLOGDEBUG(LOG_INFO, "gTaskService is Free\n");
 
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "TaskMgt Close Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
}

/**
 * Description: FD Task를 설정하는 함수
 *
 * @param [in] taskFdCb : Callback 함수
 * @param [in] fd : 설정할 Fd
 * @param [in] flags : 설정 Flags (TASK_READ, TASK_WRITE, TASK_PERSIST)
 * @param [in] priority : 설정 Priority
 *
 * @retval : 설정한 FD Task 주소 값 : 성공,
 *           TASK_ERR_LIBEVENT(-1) : Libevent 생성 or 초기화 실패,
 *           TASK_ERR_ARGUMENT(-2) : 잘못된 인자 값
 *
 * @bug
 *  반드시, taskcreate()를 먼저 실행해야 한다.
 *
 * Example Code Usage
 * @code
 *  void * gFdTask;
 *  
 *  void * fdCallback(Int32T fd, Int16T event, void * arg)
 *  {
 *  
 *    ... // fd 처리
 *    
 *  }
 *
 *  void * fdSetFunc(void)
 *  {
 *    Int32T setFd;
 *
 *    ... // setFd를 사용자 에 맞게 설정 (예) netlink socket 등
 *
 *    gFdTask 
 *     = _taskFdSet
 *        (fdCallback, setFd, TASK_READ|TASK_WRITE, TASK_PRI_MIDDLE, NULL);
 *
 *    ...
 *
 *  }
 *
 * @endcode
 */

void * _taskFdSet
  (void (* taskFdCb)(Int32T, Int16T, void *), 
   Int32T fd, Uint32T flags, Uint32T priority, void * arg)
{
    struct event *fdEvent = NULL;

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "FD Task Set\n");
    nLOGDEBUG(LOG_INFO, "FD : %d, flags : %d, priority : %d\n",
                     fd, flags, priority);
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    if(priority < 0)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Invalid priority : %d\n", priority);
        nLOGDEBUG(LOG_ERR, 
                   "Priority Range %d ~ %d\n", TASK_PRI_LOW, TASK_PRI_HIGH);
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return (void *) TASK_ERR_ARGUMENT;
    }

    nLOGDEBUG(LOG_INFO, "Task Fd priority Check Success\n");

    fdEvent = event_new(gTaskService, fd, flags, taskFdCb, arg);
    if(!fdEvent || event_add(fdEvent, NULL) < 0)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Task Create or Add Failure\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return (void *) TASK_ERR_LIBEVENT;
    }

    nLOGDEBUG(LOG_INFO, 
               "Task Fd Create or Add Success - fdEvent : %p\n", fdEvent);

    if(event_priority_set(fdEvent, priority) < 0)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Task Priority Set Failure\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return (void *) TASK_ERR_LIBEVENT;
    }
  
    nLOGDEBUG(LOG_INFO, "Task Priority Set Success\n");
    
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "FD Task Set Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    return fdEvent;
}

/**
 * Description: 설정한 FD Task를 해제하는 함수
 *
 * @param [out] delFd : 해제할 FD Task
 *
 * @retval : None
 *
 * @bug
 *  반드시, taskcreate()를 먼저 실행해야 한다.
 *
 * Example Code Usage
 * @code
 *  void * gFdTask;
 *  
 *  void * fdCallback(Int32T fd, Int16T event, void * arg)
 *  {
 *  
 *    ... // fd 처리
 *
 *    _taskFdDel(gFdTask);    // FD Task 삭제
 *    
 *  }
 *
 *  void * fdSetFunc(void)
 *  {
 *    Int32T setFd;
 *
 *    ... // setFd를 사용자 에 맞게 설정 (예) netlink socket 등
 *
 *    gFdTask
 *     = _taskFdSet
 *        (fdCallback, setFd, TASK_READ|TASK_WRITE, TASK_PRI_MIDDLE, NULL);
 *
 *    ...
 *
 *  }
 *
 * @endcode
 */

void _taskFdDel(void * fdEvent)
{
    struct event * ev = fdEvent;

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "FD Task Del\n");
    nLOGDEBUG(LOG_INFO, "fdEvent : %p\n", fdEvent);
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    event_del(ev);
    event_free(ev);

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "FD Task Del Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
}

void * _taskFdUpdate
  (void (* taskFdCb)(Int32T, Int16T, void *), 
   void * fdTask, void * newArg)
{
    struct event_base *base;
    struct event *ev = (struct event *)fdTask;
    void * oldArg = NULL;
    void * assignArg = NULL;
    Int32T fd = 0;
    Int16T fdFlags = 0;

    if(fdTask == NULL)
    {
        return (void *)NULL;
    }

    event_get_assignment(ev, &base, &fd, &fdFlags, NULL, oldArg);

    if(newArg == NULL)
    {
        assignArg = oldArg;
    }
    else
    {
        assignArg = newArg;
    }

    event_del(ev);
    
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "fd : %d, fdFlags: %d\n", fd, fdFlags);

    event_assign(ev, base, fd, fdFlags, taskFdCb, assignArg);
    event_add(ev, NULL);
    
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "FD Task Update Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return ev;
}

void * _taskTimerUpdate
  (void (* taskTimerCb)(Int32T, Int16T, void *), 
   void * timerTask, struct timeval tv, void * newArg)
{
    struct event_base *base;
    struct event *ev = (struct event *)timerTask;
    void * oldArg = NULL;
    void * assignArg = NULL;
    Int32T fd = 0;
    Int16T fdFlags = 0;

    if(timerTask == NULL)
    {
        return (void *)NULL;
    }

    event_get_assignment(ev, &base, &fd, &fdFlags, NULL, oldArg);

    if(newArg == NULL)
    {
        assignArg = oldArg;
    }
    else
    {
        assignArg = newArg;
    }

    event_del(ev);

    event_assign(ev, base, fd, fdFlags, taskTimerCb, assignArg);
    event_add(ev, &tv);
 
    return ev;
}

/**
 * Description: Timer Task를 설정하는 함수
 *
 * @param [in] taskTimerCb : Callback 함수
 * @param [in] tv : 설정할 Time
 * @param [in] persist : 영속성 flag (TASK_PERSIST)
 * @param [in] arg : Callback 시, 전달받을 변수
 *
 * @retval : 설정한 Timer Task 주소 값 : 성공,
 *           TASK_ERR_LIBEVENT(-1) : Libevent 생성 or 초기화 실패,
 *           TASK_ERR_ARGUMENT(-2) : 잘못된 인자 값
 *
 * @bug
 *  반드시, taskcreate()를 먼저 실행해야 한다.
 *
 * Example Code Usage
 * @code
 *  void * gTimerTask;
 *  
 *  void * timerCallback(Int32T fd, Int16T event, void * arg)
 *  {
 *  
 *    ... // timer 처리
 *
 *    _taskTimerDel(gTimerTask);     // Timer Task 삭제
 *    
 *  }
 *
 *  void * timerSetFunc(void)
 *  {
 *    struct timeval tv;
 *
 *    tv.tv_set = 10;
 *    tv.tv_usec = 0;
 *
 *    gTimerTask = _taskTimerSet(timerCallback, tv, NULL, NULL);
 *
 *    ...
 *
 *  }
 *
 * @endcode
 */

void * _taskTimerSet
  (void (* taskTimerCb)(Int32T, Int16T, void *), 
   struct timeval tv, Uint32T persist, void * arg)
{
    struct event *timerEvent = NULL;

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Timer Set\n");
    nLOGDEBUG(LOG_INFO, "Set Time : %ld.%ld, persist : %d\n",
                     tv.tv_sec, tv.tv_usec, persist);
    nLOGDEBUG(LOG_DEBUG, "################################\n");


    if(persist != TASK_PERSIST && persist != 0)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Invalid Persist : %d\n", persist);
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return (void *) TASK_ERR_ARGUMENT;
    }
    nLOGDEBUG(LOG_DEBUG, "Persist Check Success\n");

    if(arg == NULL)
    {
        timerEvent =
         event_new(gTaskService, -1, persist, taskTimerCb, event_self_cbarg());
    }
    else
    {
        timerEvent =
         event_new(gTaskService, -1, persist, taskTimerCb, arg);
    }

    if(!timerEvent || evtimer_add(timerEvent, &tv) < 0)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Timer Create or Add Failure\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return (void *) TASK_ERR_LIBEVENT;
    }

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Timer Create or Add Success - timerEvent : %p\n", 
                     timerEvent);
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    return timerEvent;
}

/**
 * Description: 설정한 Timer Task를 해제하는 함수
 *
 * @param [out] delTimer : 해제할 Timer Task
 *
 * @retval : None
 *
 * @bug
 *  반드시, taskcreate()를 먼저 실행해야 한다.
 *
 * Example Code Usage
 * @code
 *  void * gTimerTask;
 *  
 *  void * timerCallback(Int32T fd, Int16T event, void * arg)
 *  {
 *  
 *    ... // timer 처리
 *
 *    _taskTimerDel(gTimerTask);     // Timer Task 삭제
 *    
 *  }
 *
 *  void * timerSetFunc(void)
 *  {
 *    struct timeval tv;
 *
 *    tv.tv_set = 10;
 *    tv.tv_usec = 0;
 *
 *    gTimerTask = _taskTimerSet(timerCallback, tv, NULL, NULL);
 *
 *    ...
 *
 *  }
 *
 * @endcode
 */

void _taskTimerDel(void * delTimer)
{
    struct event * timerEvent = delTimer;

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Timer Task Del\n");
    nLOGDEBUG(LOG_INFO, "delTimer : %p\n", delTimer);
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    if(delTimer == NULL)
      return ;

    event_del(timerEvent);
    event_free(timerEvent);

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Timer Task Del Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
}

/**
 * Description: 자주사용되는 Signal Task를 한번에 설정하는 함수
 *              (SIGINT, SIGTERM, SIGHUP, SIGUSR1, SIGSEGV
 *
 * @param [in] signalFunc : Callback 함수
 *
 * @retval : SUCCESS(0) : 성공,
 *           TASK_ERR_LIBEVENT(-1) : Libevent 생성 or 초기화 실패,
 *           TASK_ERR_ARGUMENT(-2) : 잘못된 인자 값
 * @bug
 *  반드시, taskcreate()를 먼저 실행해야 한다.
 *
 * Example Code Usage
 * @code
 *  Int32T main(void)
 *  {
 *    Int32T ret;
 *  
 *    ... 
 *    
 *    ret = taskCreate(ribmgrInitFunc); 
 *
 *    ret = _taskSignalSetAggr(ribmgrSigCallback);
 *
 *    ...
 *
 *    taskDispatch();
 *
 *    ...
 *
 *  }
 * @endcode
 */

Int32T _taskSignalSetAggr(pTaskSignalRegFunc signalFunc)
{
    struct event * taskSigAggr[TASK_SIG_AGGR_CNT];
    Int32T i;

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Aggregate Signal Set\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    if(signalFunc == NULL)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "singalFunc is Null\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return TASK_ERR_ARGUMENT;
    }

    gSignalCb = signalFunc;

    nLOGDEBUG(LOG_DEBUG, "signalFunc check Success\n");

    for(i=0; i<TASK_SIG_AGGR_CNT; i++)
    {
        if(gTaskSigAggrMgt[i].flag == TASK_SIGNAL_AVAILABLE)
        {
            taskSigAggr[i] = evsignal_new
                             (gTaskService, gTaskSigAggrMgt[i].sig,
                              ipcInterSignalCb, NULL);
            if(!taskSigAggr[i])
            {
                nLOGDEBUG(LOG_DEBUG, "################################\n");
                nLOGDEBUG(LOG_ERR, "taskSigAggr[%d] Create Failure\n", i);
                nLOGDEBUG(LOG_DEBUG, "################################\n");
                return TASK_ERR_LIBEVENT;
            }

            if(event_add(taskSigAggr[i], NULL) < 0)
            {
                nLOGDEBUG(LOG_DEBUG, "################################\n");
                nLOGDEBUG(LOG_ERR, "taskSigAggr[%d] Add Failure\n", i);
                nLOGDEBUG(LOG_DEBUG, "################################\n");
                return TASK_ERR_LIBEVENT;
            }

            gTaskSigAggrMgt[i].flag = TASK_SIGNAL_IMPOSSIBLE;
            gTaskSigAggrMgt[i].taskSig = taskSigAggr[i];

            nLOGDEBUG(LOG_INFO, 
                      "gTaskSigAggrMgt[%d] - flag : %d, taskSig : %p\n",
                       i, gTaskSigAggrMgt[i].flag, gTaskSigAggrMgt[i].taskSig);
        }
    }

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Aggregate Signal Set Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}

/**
 * Description: _taskSignalSetAggr() 로 설정한 Signal Task를 해제하는 함수
 *
 * @retval : None
 *
 * @bug
 *  반드시, _taskClose() 사용 전에 실행해야 한다.
 *
 * Example Code Usage
 * @code
 *  
 *  void * sigAggrCallback(Int32T sig)
 *  {
 *  
 *    ...             // Component 종료 과정 구현
 *
 *    _taskSignalDelAggr();    // Signal 삭제
 *
 *
 *     ...             
 *
 *  }
 *
 *  Int32T main(void)
 *  {
 *    Int32T ret;
 *  
 *    ... 
 *    
 *    ret = taskCreate(ribmgrInitFunc); 
 *
 *    ret = _taskSignalSetAggr(ribmgrSigCallback);
 *
 *    ...
 *
 *    taskDispatch();
 *
 *    ...
 *
 *  }
 *
 * @endcode
 */

void _taskSignalDelAggr(void)
{
    struct event * signalEvent = NULL;
    Int32T i;

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Aggregate Signal Del\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    for(i=0; i<TASK_SIG_AGGR_CNT; i++)
    {
        signalEvent = gTaskSigAggrMgt[i].taskSig;

        if(signalEvent != NULL)
        {
            event_del(signalEvent);
            event_free(signalEvent);
            gTaskSigAggrMgt[i].flag = TASK_SIGNAL_AVAILABLE;
            gTaskSigAggrMgt[i].taskSig = NULL;

            nLOGDEBUG(LOG_INFO, 
                      "gTaskSigAggrMgt[%d] - flag : %d, taskSig : %p\n",
                       i, gTaskSigAggrMgt[i].flag, gTaskSigAggrMgt[i].taskSig);
        }
    }

    gSignalCb = NULL;

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Aggregate Signal Del Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
}

/**
 * Description: 자주사용되는 Signal Task의 Callback 함수를 재 등록 하는 함수
 *              (SIGINT, SIGTERM, SIGHUP, SIGUSR1, SIGSEGV)
 *
 * @param [in] signalFunc : Callback 함수
 *
 * @retval : SUCCESS(0) : 성공,
 *           TASK_ERR_LIBEVENT(-1) : Libevent 생성 or 초기화 실패,
 *           TASK_ERR_ARGUMENT(-2) : 잘못된 인자 값
 * @bug
 *  반드시, _taskSignalSetAggr() 사용 후, 사용해야 한다.
 *
 * Example Code Usage
 * @code
 *  Int32T ribmgrRestart(void)
 *  {
 *    Int32T ret;
 *  
 *    ... 
 *    
 *    ret = _taskSignalUpdateAggr(ribmgrSigCallback);
 *
 *    ...
 *
 *  }
 * @endcode
 */


Int32T _taskSignalUpdateAggr(pTaskSignalRegFunc signalFunc)
{
    struct event_base *base;
    Int32T fd = 0;
    Int16T fdFlags = 0;
    Int32T i;

    if(signalFunc == NULL)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "singalFunc is Null\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return TASK_ERR_ARGUMENT;
    }

    nLOGDEBUG(LOG_DEBUG, "signalFunc check Success\n");
    gSignalCb = signalFunc;

    for(i=0; i<TASK_SIG_AGGR_CNT; i++)
    {
        if(gTaskSigAggrMgt[i].flag == TASK_SIGNAL_IMPOSSIBLE)
        {
            event_get_assignment(gTaskSigAggrMgt[i].taskSig,
                                 &base, 
                                 &fd,
                                 &fdFlags,
                                 NULL,
                                 NULL);

            event_del(gTaskSigAggrMgt[i].taskSig);

            event_assign(gTaskSigAggrMgt[i].taskSig, 
                         base, 
                         fd, 
                         fdFlags,
                         ipcInterSignalCb,
                         NULL);

            event_add(gTaskSigAggrMgt[i].taskSig, NULL);
            gTaskSigAggrMgt[i].flag = TASK_SIGNAL_IMPOSSIBLE;

            nLOGDEBUG(LOG_INFO,
                      "gTaskSigAggrMgt[%d] - flag : %d, taskSig : %p\n",
                       i, gTaskSigAggrMgt[i].flag, gTaskSigAggrMgt[i].taskSig);
        }
    } 
     
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Aggregate Signal Update Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}

/**
 * Description: Signal Task를 설정하는 함수
 *
 * @param [in] sigType : 설정할 Signal 
 * @param [in] signalFunc : Callback 함수
 *
 * @retval : 설정한 Signal Task 주소 값 : 성공,
 *           TASK_ERR_LIBEVENT(-1) : Libevent 생성 or 초기화 실패,
 *           TASK_ERR_ARGUMENT(-2) : 잘못된 인자 값
 *           TASK_ERR_ALREADY_SET(-3): 이미 설정되어 있는 경우          
 *
 * @bug
 *     SIGKILL, SIGSTOP 은 등록되지 않음
 *     taskcreate()를 먼저 실행해야 한다.
 *
 * Example Code Usage
 * @code
 *  void * gSigInt;
 *
 *  void * sigCallback(Int32T sig, Int16T event, void * arg)
 *  {
 *  
 *    ...             // Component 종료 과정 구현
 *
 *    _taskSignalDel(gSigInt);    // Signal 삭제
 *
 *
 *     ...             
 *
 *  }
 *
 *  Int32T main(void)
 *  {
 *    Int32T ret;
 *  
 *    ... 
 *    
 *    ret = taskCreate(ribmgrInitFunc); 
 *
 *    gSigInt = _taskSignalSet(sigCallback);
 *
 *    ...
 *
 *    taskDispatch();
 *
 *    ...
 *
 *  }
 *
 * @endcode 
 */

//void * _taskSignalSet(Int32T sigType, pTaskSignalFunc signalFunc)
void * _taskSignalSet(Int32T sigType)
{
    struct event * signalEvent = NULL;
    Int32T i;

//  signalEvent = evsignal_new(gTaskService, sigType, signalFunc,
//                               event_self_cbarg());

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Signal Task Set\n");
    nLOGDEBUG(LOG_INFO, "Set SigType : %d\n", sigType);
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    if(sigType == SIGKILL || sigType == SIGSTOP)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "SIGKILL(%d) or SIGSTOP(%d) is Not registered\n",
                        SIGKILL, SIGSTOP);
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return (void *) TASK_ERR_ARGUMENT;
    }

    nLOGDEBUG(LOG_DEBUG, "SIGKILL and SIGSTOP check success\n");

    for(i=0; i<TASK_SIG_AGGR_CNT; i++)
    {
        if(sigType == gTaskSigAggrMgt[i].sig)
        {
            if(gTaskSigAggrMgt[i].flag == TASK_SIGNAL_IMPOSSIBLE)
            {
                nLOGDEBUG(LOG_DEBUG, "################################\n");
                nLOGDEBUG(LOG_ERR, "SigType(%d) Already Set\n", sigType);
                nLOGDEBUG(LOG_DEBUG, "################################\n");
                return (void *) TASK_ERR_ALREADY_SET;
            }
            else
            {
                gTaskSigAggrMgt[i].flag = TASK_SIGNAL_IMPOSSIBLE;
                gTaskSigAggrMgt[i].taskSig = NULL;
                nLOGDEBUG(LOG_INFO, 
                           "Although other methods are registerable\n");
                break;
            }
        }
    }

//  signalEvent = evsignal_new(gTaskService, sigType, signalFunc, NULL);
    signalEvent = evsignal_new(gTaskService, sigType, ipcInterSignalCb, NULL);

    if(!signalEvent || event_add (signalEvent, NULL) < 0)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Signal Task Create or Add Failure\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return (void *) TASK_ERR_LIBEVENT; 
    }

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Signal Task Set Success - signalEvent : %p\n", 
                     signalEvent);
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    return signalEvent;
}

/**
 * Description: 설정한 Signal Task를 해제하는 함수
 *
 * @param [out] delSignal : 해제할 Signal Task
 *
 * @retval : None
 *
 * @bug
 *  반드시, _taskClose() 사용 전에 실행해야 한다.
 *
 * Example Code Usage
 * @code
 *  void * gSigInt;
 *
 *  void * sigCallback(Int32T sig, Int16T event, void * arg)
 *  {
 *  
 *    ...             // Component 종료 과정 구현
 *
 *    _taskSignalDel(gSigInt);    // Signal 삭제
 *
 *
 *     ...             
 *
 *  }
 *
 *  Int32T main(void)
 *  {
 *    Int32T ret;
 *  
 *    ... 
 *    
 *    ret = taskCreate(ribmgrInitFunc); 
 *
 *    gSigInt = _taskSignalSet(sigCallback);
 *
 *    ...
 *
 *    taskDispatch();
 *
 *    ...
 *
 *  }
 *
 * @endcode
 */

void _taskSignalDel(void * delSignal)
{
    Int32T i;
    Int32T sigType = event_get_signal(delSignal);    
    struct event * signalEvent = delSignal;

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Signal Task Del\n");
    nLOGDEBUG(LOG_INFO, "delSignal : %p, sigType : %d\n", delSignal, sigType);
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    event_del(signalEvent);
    event_free(signalEvent);

    for(i=0; i<TASK_SIG_AGGR_CNT; i++)
    {
        if(sigType == gTaskSigAggrMgt[i].sig)
        {
            gTaskSigAggrMgt[i].flag = TASK_SIGNAL_AVAILABLE;
            gTaskSigAggrMgt[i].taskSig = NULL;

            nLOGDEBUG(LOG_INFO, "Aggregate Signal\n");
            nLOGDEBUG(LOG_DEBUG, 
                      "gTaskSigAggrMgt[%d] - flag : %d, taskSig : %d\n",
                       i, gTaskSigAggrMgt[i].flag, gTaskSigAggrMgt[i].taskSig);
            break;
        }
    }

 
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Signal Task Del Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
}

void * _taskSignalUpdate(void * sigEvent, void * taskSigCb)
{
    struct event_base *base;
    struct event *ev = (struct event *)sigEvent;
    Int32T fd = 0;
    Int16T fdFlags = 0;

    if(sigEvent == NULL)
    {
        return (void *)NULL;
    }

    event_get_assignment(ev, &base, &fd, &fdFlags, NULL, NULL);
    event_del(ev);

    event_assign(ev, base, fd, fdFlags, taskSigCb, NULL);
    event_add(ev, NULL);

    return ev;
}

#if 0
void _taskPmModuleUpdate (void * data, Uint32T dataLen)
{
    printf("In %s\n", __func__);
    if(gUserProcess == PROCESS_MANAGER)
    {
        gIpcCb(IPC_LCS_PM2C_DYNAMIC_UPGRADE, data, dataLen);
    }
    printf("Out %s\n", __func__);
}
#endif
#if 0
Int32T _dluSendMsg (Uint32T process, Uint32T msgId, void *msg, Uint32T size)
{
    IpcMsgHeaderT *pMsg = (IpcMsgHeaderT *) 0;
    Int32T msgLen = IPC_MSG_HEADER_SIZE;
    DluMsgT *pData = NULL;

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Dynamic Live Update Message Send Start\n");
    nLOGDEBUG(LOG_INFO, "process : %d, msgId : %d\n", process, msgId);
    nLOGDEBUG(LOG_DEBUG, "################################\n");


    // 1. Argument Checking

    if(process < 0 && process >= MAX_PROCESS_CNT)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Invalid process : %d\n", process);
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return DLU_ERR_ARGUMENT;    // -1
    }

    nLOGDEBUG(LOG_DEBUG, "Process Id Check Success\n");


    // 2. DLU Message Header, Message init.

    msgLen = msgLen + sizeof(DluMsgT);
    nLOGDEBUG(LOG_DEBUG,
               "Message Len = %d (Header(%d) + body(%d))\n",
               msgLen, IPC_MSG_HEADER_SIZE, sizeof(DluMsgT));

    pMsg = (IpcMsgHeaderT *)nMALLOC(MESSAGE_HEADER, msgLen);
    if (pMsg <= 0)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Message Header Create Failure\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return DLU_ERR_MSG_CREATE;    // -2
    }
    nLOGDEBUG(LOG_DEBUG, "Message Header Create Success\n");

    pData = (DluMsgT *)nMALLOC(DLU_SEND_MESSAGE, sizeof(DluMsgT));
    if (pData <= 0)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Message Body Create Failure\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return DLU_ERR_MSG_CREATE;    // -2
    }
    nLOGDEBUG(LOG_DEBUG, "Message Body Create Success\n");


    // 3. Message context setting

    pMsg->type = DYNAMIC_UPGRADE;
    pMsg->msgId = msgId;
    pMsg->length = msgLen;

    nLOGDEBUG(LOG_DEBUG, "Message Header Information\n");
    nLOGDEBUG(LOG_DEBUG,
               "pMsg->type : %d, pMsg->msgId : %d, pMsg->length : %d\n",
               pMsg->type, pMsg->msgId, pMsg->length);

    if (msg != NULL && size != 0)
    {
        memcpy(pMsg->data, msg, size);
        nLOGDEBUG(LOG_DEBUG, "Message Context Setting Success\n");
    }
    else
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_DEBUG, "Message Context Setting Failure\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return DLU_ERR_MSG_SET;    // -3
    }
    if(gIpcCommMgt[IPC_MANAGER].bev != NULL)
    {
        bufferevent_write(gIpcCommMgt[IPC_MANAGER].bev, pMsg, pMsg->length);
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_INFO, "DLU Message Send Success\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        nFREE(MESSAGE_HEADER, pMsg);
        nFREE(DLU_SEND_MESSAGE, pData);
        return SUCCESS;

    }
    else
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Message Send Failure - process Not Connect\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        nFREE(MESSAGE_HEADER, pMsg);
        nFREE(DLU_SEND_MESSAGE, pData);
        return DLU_ERR_MSG_SEND;
    }
}

Int32T dluSendMsg (Uint32T process, Uint32T msgId, void *msg, Uint32T size)
{
    IpcMsgHeaderT *pMsg = (IpcMsgHeaderT *) 0;
    Int32T msgLen = IPC_MSG_HEADER_SIZE;
    DluMsgT *pData = NULL;

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Dynamic Live Update Message Send Start\n");
    nLOGDEBUG(LOG_INFO, "process : %d, msgId : %d\n", process, msgId);
    nLOGDEBUG(LOG_DEBUG, "################################\n");


    // 1. Argument Checking

    if(process < 0 && process >= MAX_PROCESS_CNT)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Invalid process : %d\n", process);
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return DLU_ERR_ARGUMENT;    // -1
    }

    nLOGDEBUG(LOG_DEBUG, "Process Id Check Success\n");


    // 2. DLU Message Header, Message init.

    msgLen = msgLen + sizeof(DluMsgT);
    nLOGDEBUG(LOG_DEBUG,
               "Message Len = %d (Header(%d) + body(%d))\n",
               msgLen, IPC_MSG_HEADER_SIZE, sizeof(DluMsgT));

    pMsg = (IpcMsgHeaderT *)nMALLOC(MESSAGE_HEADER, msgLen);
    if (pMsg <= 0)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Message Header Create Failure\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return DLU_ERR_MSG_CREATE;    // -2
    }
    nLOGDEBUG(LOG_DEBUG, "Message Header Create Success\n");

    pData = (DluMsgT *)nMALLOC(DLU_SEND_MESSAGE, sizeof(DluMsgT));
    if (pData <= 0)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Message Body Create Failure\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return DLU_ERR_MSG_CREATE;    // -2
    }
    nLOGDEBUG(LOG_DEBUG, "Message Body Create Success\n");


    // 3. Message context setting

    pMsg->type = DYNAMIC_UPGRADE;
    pMsg->msgId = msgId;
    pMsg->length = msgLen;

    nLOGDEBUG(LOG_DEBUG, "Message Header Information\n");
    nLOGDEBUG(LOG_DEBUG,
               "pMsg->type : %d, pMsg->msgId : %d, pMsg->length : %d\n",
               pMsg->type, pMsg->msgId, pMsg->length);

    if (msg != NULL && size != 0)
    {
        memcpy(pMsg->data, msg, size);
        nLOGDEBUG(LOG_DEBUG, "Message Context Setting Success\n");
    }
    else
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_DEBUG, "Message Context Setting Failure\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return DLU_ERR_MSG_SET;    // -3
    }


    // 4. Message Send and free

//    if(gIpcCommMgt[process].bev != NULL)
//    {
//        bufferevent_write(gIpcCommMgt[process].bev, pMsg, pMsg->length);
//        nLOGDEBUG(LOG_DEBUG, "################################\n");
//        nLOGDEBUG(LOG_INFO, "DLU Message Send Success\n");
//        nLOGDEBUG(LOG_DEBUG, "################################\n");

//        nFREE(MESSAGE_HEADER, pMsg);
//        nFREE(DLU_SEND_MESSAGE, pData);
//        return SUCCESS;

//    }
//    else
//    {
//        nLOGDEBUG(LOG_DEBUG, "################################\n");
//        nLOGDEBUG(LOG_ERR, "Message Send Failure - process Not Connect\n");
//        nLOGDEBUG(LOG_DEBUG, "################################\n");

//        nFREE(MESSAGE_HEADER, pMsg);
//        nFREE(DLU_SEND_MESSAGE, pData);
//        return DLU_ERR_MSG_SEND;
//    }

     if(gIpcCommMgt[IPC_MANAGER].bev != NULL)
     {
        bufferevent_write(gIpcCommMgt[IPC_MANAGER].bev, pMsg, pMsg->length);
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_INFO, "DLU Message Send Success\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        nFREE(MESSAGE_HEADER, pMsg);
        nFREE(DLU_SEND_MESSAGE, pData);
        return SUCCESS;

    }
    else
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Message Send Failure - process Not Connect\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        nFREE(MESSAGE_HEADER, pMsg);
        nFREE(DLU_SEND_MESSAGE, pData);
        return DLU_ERR_MSG_SEND;
    }
}
#endif

QueueT * queueInit(Uint64T maxCount, Uint8T dupCheck, Uint32T dataType)
{
    QueueT * newCqueue = NULL;

    if((dataType >= MEM_MAX_USER_TYPE && dataType < MEM_SYS_START)
       || dataType >= MEM_MAX_SYS_TYPE)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Wrong Cqueue DataType\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        return (QueueT *)CQUEUE_ERR_ARGUMENT;
    }

    newCqueue = nMALLOC(CQUEUE_HEAD, sizeof(QueueT));

    if((Int32T)newCqueue <= 0)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "NewCqueue Memory Allocate : Failure(%d)\n",
                       (Int32T)newCqueue);
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        return (QueueT*)CQUEUE_ERR_CQUEUE_ALLOC;
    }

    newCqueue->head = NULL;
    newCqueue->tail = NULL;
    newCqueue->count = 0;
    newCqueue->dataType = dataType;

    if(dupCheck != CQUEUE_DUPLICATED && dupCheck != CQUEUE_NOT_DUPLICATED)
    {
        newCqueue->dupCheck = CQUEUE_DUPLICATED;
    }
    else
    {
        newCqueue->dupCheck = dupCheck;
    }

    if(maxCount <= 0)
    {
        newCqueue->maxCount = CQUEUE_DEFAULT_COUNT;
    }
    else
    {
        newCqueue->maxCount = maxCount;
    }

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Init NewCqueue : Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    return newCqueue;
}

void queueFree(QueueT * cqueue)
{
    if (cqueue == NULL)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Cqueue Must Not NULL\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    queueDeleteAllNode(cqueue);
    nFREE(CQUEUE_HEAD, cqueue);

    cqueue = NULL;

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Free Cqueue Complete\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
}

void queueDeleteAllNode(QueueT * cqueue)
{
    QueueNodeT * tempNode = NULL;

    if (cqueue == NULL)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Cqueue Must Not NULL\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    if (cqueue->count == 0)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_INFO, "Cqueue Is Empty\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    if(cqueue->count == 1)
    {
        nFREE(cqueue->dataType, cqueue->head->data);
        nFREE(CQUEUE_NODE, cqueue->head);

        --cqueue->count;

        cqueue->head = NULL;
        cqueue->tail = NULL;

        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_INFO, "Delete All Node Complete\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    for (; tempNode != cqueue->tail;)
    {
        tempNode = cqueue->head;

        cqueue->head = tempNode->next;
        cqueue->tail->next = tempNode->next;
        tempNode->next->prev = cqueue->tail;

        nFREE(cqueue->dataType, tempNode->data);
        nFREE(CQUEUE_NODE, tempNode);

        --cqueue->count;

        if(cqueue->count == 1)
        {
            nFREE(cqueue->dataType, cqueue->head->data);
            nFREE(CQUEUE_NODE, cqueue->head);

            --cqueue->count;

            cqueue->head = NULL;
            cqueue->tail = NULL;
            break;
        }
    }

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Delete All Node Complete\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    return;
}

void * queueGetHead (QueueT * cqueue)
{
    if (cqueue == NULL)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Cqueue Must Not NULL\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *) CQUEUE_ERR_NULL;
    }
    else if (cqueue->head == NULL)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Cqueue Head is NULL\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *) CQUEUE_ERR_EMPTY;
    }

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Get Head Data : Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    return cqueue->head->data;
}

void * queueGetTail (QueueT * cqueue)
{
    if (cqueue == NULL)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Cqueue Must Not NULL\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *) CQUEUE_ERR_NULL;
    }
    else if(cqueue->tail == NULL)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Cqueue Tail is NULL\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *) CQUEUE_ERR_EMPTY;
    }

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Get Tail Data : Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    return cqueue->tail->data;
}

Int64T queueCount (QueueT * cqueue)
{
    if (cqueue == NULL)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Cqueue Must Not NULL\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        return CQUEUE_ERR_NULL;
    }

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Get Cqueue Count : Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    return cqueue->count;
}

Int32T queueEnqueue(QueueT * cqueue, void * data)
{
    QueueNodeT * tempNode = NULL;
    QueueNodeT * newNode;

    if (cqueue == NULL)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Cqueue Must Not NULL\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        return CQUEUE_ERR_NULL;
    }
    else if (data == NULL)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        return CQUEUE_ERR_NULL;
    }

    if(cqueue->count == cqueue->maxCount)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Cqueue Full\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        return CQUEUE_ERR_FULL;
    }

    newNode = nMALLOC(CQUEUE_NODE, sizeof(QueueNodeT));

    if((Int32T)newNode <= 0)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "NewCqueueNode Memory Allocate : Failure(%d)\n",
                       (Int32T)newNode);
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        return CQUEUE_ERR_CQUEUENODE_ALLOC;
    }

    newNode->prev = NULL;
    newNode->next = NULL;
    newNode->data = data;

    /* No Node */
    if (cqueue->count == 0)
    {
        cqueue->head = newNode;
        cqueue->tail = newNode;

        newNode->prev = newNode;
        newNode->next = newNode;
    }
    else
    {
        if(cqueue->dupCheck == CQUEUE_NOT_DUPLICATED)
        {
            for(tempNode = cqueue->head; tempNode != cqueue->tail;
                tempNode = tempNode->next)
            {
                if(tempNode->data == data)
                {
                    nFREE(CQUEUE_NODE, newNode);

                    nLOGDEBUG(LOG_DEBUG, "################################\n");
                    nLOGDEBUG(LOG_ERR, "Duplication Data Found\n");
                    nLOGDEBUG(LOG_DEBUG, "################################\n");

                    return CQUEUE_ERR_DUP;
                }
            }

            if(tempNode->data == data)
            {
                nFREE(CQUEUE_NODE, newNode);

                nLOGDEBUG(LOG_DEBUG, "################################\n");
                nLOGDEBUG(LOG_ERR, "Duplication Data Found\n");
                nLOGDEBUG(LOG_DEBUG, "################################\n");

                return CQUEUE_ERR_DUP;
            }
        }

        cqueue->tail->next = newNode;

        newNode->prev = cqueue->tail;
        newNode->next = cqueue->head;

        cqueue->tail = newNode;
        cqueue->head->prev = cqueue->tail;
    }

    ++cqueue->count;

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Enqueue Data : Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}

void * queueDequeue (QueueT * cqueue)
{
    QueueNodeT * tempNode = NULL;
    void * data = NULL;

    if (cqueue == NULL)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Cqueue Must Not NULL\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *) CQUEUE_ERR_NULL;
    }

    if(cqueue->count == 0)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Cqueue Is Empty\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *)CQUEUE_ERR_EMPTY;
    }
    else if (cqueue->count == 1)
    {
        cqueue->head->prev = NULL;
        cqueue->head->next = NULL;

        data = cqueue->head->data;

        nFREE(CQUEUE_NODE, cqueue->head);

        cqueue->head = NULL;
        cqueue->tail = NULL;
    }
    else
    {
        tempNode = cqueue->head;

        cqueue->head = tempNode->next;
        cqueue->tail->next = tempNode->next;

        tempNode->next->prev = cqueue->tail;

        tempNode->prev = NULL;
        tempNode->next = NULL;

        data = tempNode->data;

        nFREE(CQUEUE_NODE, tempNode);
    }

    --cqueue->count;

    nLOGDEBUG(LOG_DEBUG, "################################\n");
    nLOGDEBUG(LOG_INFO, "Dequeue Data : Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    return data;
}

