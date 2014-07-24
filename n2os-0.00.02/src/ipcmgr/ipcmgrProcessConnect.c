/**
* @file : ipcmgrProcessConnect.c
* @brief : N2OS Component와 IPC Manager 와의 연결
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

#include "ipcmgrInternal.h"

/*******************************************************************************
 *                              GLOBAL VARIABLES
 ******************************************************************************/


/*******************************************************************************
 *                         GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 *                         LOCAL CONSTANTS/LITERALS/TYPES
 ******************************************************************************/


/*******************************************************************************
*                               LOCAL VARIABLES
*******************************************************************************/


/*******************************************************************************
 *                                   CODE
 ******************************************************************************/

/**
 * Description: 해당 Process와 Connect를 맺는 함수
 *
 * @param [in] fd : Connect를 맺으려는 Process의 fd
 *
 * @retval : SUCCESS(0) : 해당 Process 연결정보가 있음,
 *           FAILURE(-1) : 해당 Process 연결정보가 없음
 */

Int32T ipcmgrProcessSingleConnect(Int32T fd)
{
    Int32T i = 1;
    Int8T ip[16] = {0, };
    Int32T port = -1;

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "Signal Process Connect [After Starting]\n");
    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "Input fd : %d\n", fd);

    for(i=1; i<MAX_PROCESS_CNT; i++)
    {
        if(gIpcCommMgt[i].acceptFd == fd)
        {
            sprintf(ip, "%u.%u.%u.%u",
                    IP_PRINT
                    (((struct sockaddr_in*)&gIpcCommMgt[i].allocateInfo)
                      ->sin_addr.s_addr));
            
            port = (((struct sockaddr_in*)&gIpcCommMgt[i].allocateInfo)
                     ->sin_port);

            nLOG(LOG_INFO, "Connect Process : %d] : %s, %d\n", i, ip, port);
            ipcmgrProcessConnect(i, ip, port);

            nLOG(LOG_INFO, "IPC Manager->Process[%d] Reg. Message Send\n", i); 
            ipcConnectRegist(IPC_MANAGER, i);

            nLOG(LOG_DEBUG, "################################\n");
            nLOG(LOG_INFO, "Signal Process Connect - Success\n");
            nLOG(LOG_DEBUG, "################################\n");

            return SUCCESS;
        }
    }

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "Signal Process Connect - Failure\n");
    nLOG(LOG_DEBUG, "################################\n");

    return FAILURE;
}

/**
 * Description: Process 연결정보에 있는 모든 Process와 Connect를 맺는 함수
 *
 * @retval : SUCCESS(0) : 성공
 */

Int32T ipcmgrProcessAllConnect(void)
{
    Int32T i;

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "All Process Connect\n");
    nLOG(LOG_DEBUG, "################################\n");

    for(i=1; i<MAX_PROCESS_CNT; i++)
    {
        // current connect process seek
        if(gIpcCommMgt[i].acceptFd != -1)
        {
            Int8T ip[16] = {0, };
            Int32T port = -1;
           
            sprintf(ip, "%u.%u.%u.%u",
                    IP_PRINT
                    (((struct sockaddr_in*)&gIpcCommMgt[i].allocateInfo)
                      ->sin_addr.s_addr));
            
            port = (((struct sockaddr_in*)&gIpcCommMgt[i].allocateInfo)
                     ->sin_port);

            nLOG(LOG_INFO, "Connect Process[%d] : %s %d\n", i, ip, port);
            ipcmgrProcessConnect(i, ip, port);

            nLOG(LOG_INFO, "IPC Manager->Process[%d] Reg. Message Send\n", i); 
            ipcConnectRegist(IPC_MANAGER, i);
         }
    }

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "All Process Connect Complete\n");
    nLOG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}

/*
 * Description: 실제 Connect를 맺는 함수
 *
 * @param [in] process : Connect를 맺으려는 Process
 * @param [in] ip : Connect 정보 (ip)
 * @param [in] port : Connect 정보 (port)
 *
 * @retval : SUCCESS(0) : 성공,
 *           FAILURE(-1) : 실패
 */

Int32T ipcmgrProcessConnect(Uint32T process, StringT ipAddr, Int32T port)
{
    struct sockaddr_in sin;

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "Process Connect Start\n");
    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "Connect Process[%d] : %s %d\n", process, ipAddr, port);

    memset(&sin, 0, sizeof(sin));

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(ipAddr);
    sin.sin_port = htons(port);

    gIpcCommMgt[process].bev = bufferevent_socket_new
                               (gTaskService, -1, BEV_OPT_CLOSE_ON_FREE);

    nLOG(LOG_DEBUG, "gIpcCommMgt[%d].bev : %p\n",
                      process, gIpcCommMgt[process].bev);

    bufferevent_setcb
     (gIpcCommMgt[process].bev, ipcmgrReadCallback, NULL, ipcmgrEventCallback, NULL);
    bufferevent_enable(gIpcCommMgt[process].bev, EV_READ);

    if(bufferevent_socket_connect
       (gIpcCommMgt[process].bev, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        nLOG(LOG_DEBUG, "################################\n");
        nLOG(LOG_ERR, "Process[%d] Connect Failure\n", process);
        nLOG(LOG_DEBUG, "################################\n");

        bufferevent_free(gIpcCommMgt[process].bev);
        return FAILURE;
    }

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "Process Connect Complete\n");
    nLOG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}

/* 20140418 JAESU HAN 

Int32T ipcmgrBindCompleteAck(Int32T fd)
{
    IpcMsgHeaderT *pMsg = (IpcMsgHeaderT *) 0;
    Int32T msgLen = sizeof(IpcMsgHeaderT);
    Int32T process;

    // 1. Process Info Check

    process = ipcGetProcess(fd);
    nLOG(LOG_INFO, "Bind Complete Ack - fd(%d), Process(%d)\n", fd, process);

    // 2. Message Header init.
   
    pMsg = (IpcMsgHeaderT *)NNMALLOC(MESSAGE_HEADER, msgLen);
    if(pMsg <= 0)
    {
        nLOG(LOG_DEBUG, "################################\n");
        nLOG(LOG_ERR, "Message Create Failure\n");
        nLOG(LOG_DEBUG, "################################\n");
        return (IpcMsgHeaderT *) FAILURE;
    }
   
    // 3. Message context setting
    
    pMsg->type = IPC_BIND_COMPLETE_ACK;
    pMsg->msgId = gipcMgrRunFlag;
    pMsg->length = msgLen;

    // 4. Async Message Send and Free
    bufferevent_write(gIpcCommMgt[process].bev, pMsg, pMsg->length);
    NNFREE(MESSAGE_HEADER, pMsg);

    nLOG(LOG_INFO, "Bind Complete Ack Message Send Complete\n");

    return SUCCESS;
}
*/
