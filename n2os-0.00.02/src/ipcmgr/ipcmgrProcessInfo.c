/**
* @file : ipcmgrProcessInfo.c
* @brief : N2OS Component의 Process 정보 관리
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

/*
 * Description: IPC Manager의 Process 연결정보 리스트를 초기화하는 함수
 *
 * @retval : None
 */

void ipcmgrProcessInfoInit(void)
{
    Int32T i;

    // 1. process connect info list init.

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "Process Connect Info List Init Start\n");
    nLOG(LOG_DEBUG, "################################\n");

    for(i=0; i<MAX_PROCESS_CNT; i++)
    {
        // IPC Common Management Structure Init.

        gIpcCommMgt[i].acceptFd = -1;
        gIpcCommMgt[i].bev = NULL;
        memset(&gIpcCommMgt[i].connectInfo, 0x00, sizeof(struct sockaddr));
        memset(&gIpcCommMgt[i].allocateInfo, 0x00, sizeof(struct sockaddr));
        gIpcCommMgt[i].bufSize = 0;
        gIpcCommMgt[i].buf = NULL;

        // Accept Process list init.
  
        gIpcAcceptMgt[i].fd = -1;
        memset(&gIpcAcceptMgt[i].addr, 0x00, sizeof(struct sockaddr));


        nLOG(LOG_DEBUG, "gIpcCommMgt[%d].acceptFd = %d\n",
                         i, gIpcCommMgt[i].acceptFd);
        nLOG(LOG_DEBUG, "gIpcCommMgt[%d].bev = %d\n",
                         i, gIpcCommMgt[i].bev);
        nLOG(LOG_DEBUG, "gIpcCommMgt[%d].connectInfo = %u.%u.%u.%u, %d\n",
                         i,
                         IP_PRINT
                         (((struct sockaddr_in*)&gIpcCommMgt[i].connectInfo)
                            ->sin_addr.s_addr),
                          ((struct sockaddr_in*)&gIpcCommMgt[i].connectInfo)
                            ->sin_port);
        nLOG(LOG_DEBUG, "gIpcCommMgt[%d].allocateInfo = %u.%u.%u.%u, %d\n",
                         i,
                         IP_PRINT
                         (((struct sockaddr_in*)&gIpcCommMgt[i].allocateInfo)
                            ->sin_addr.s_addr),
                          ((struct sockaddr_in*)&gIpcCommMgt[i].allocateInfo)
                            ->sin_port);
        nLOG(LOG_DEBUG, "gIpcCommMgt[%d].bufSize = %d\n",
                         i, gIpcCommMgt[i].bufSize);
        nLOG(LOG_DEBUG, "gIpcCommMgt[%d].buf = %d\n",
                         i, gIpcCommMgt[i].buf);

        nLOG(LOG_DEBUG, "gIpcAcceptMgt[%d].fd = %d\n",
                         i, gIpcAcceptMgt[i].fd);
        nLOG(LOG_DEBUG, "gIpcAcceptMgt[%d].addr = %u.%u.%u.%u, %d\n",
                         i, 
                         IP_PRINT
                         (((struct sockaddr_in*)&gIpcAcceptMgt[i].addr)
                           ->sin_addr.s_addr),
                          ((struct sockaddr_in*)&gIpcAcceptMgt[i].addr)
                           ->sin_port);
    }

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "Process Connect Info List Init Complete\n");
    nLOG(LOG_DEBUG, "################################\n");

}

/*
 * Description: Accept된 Process 정보를 가져오는 함수
 *
 * @retval : Accept 된 Process 정보가 있는 Number : 성공,
             FAILURE(-1) : 해당 Process 정보가 없음
 */

Int32T ipcmgrGetAcceptPoint(void)
{
    Int32T i;

    // 1. process connect info list init.

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "Accpet Point Check\n"); 
    nLOG(LOG_DEBUG, "################################\n");

    for(i=0; i<MAX_PROCESS_CNT; i++)
    {
        if(gIpcAcceptMgt[i].fd == -1)
        {
            nLOG(LOG_INFO, "gIpcAcceptMgt[%d] is Accepte Point\n", i);
            return i;
        }
    }

    nLOG(LOG_DEBUG, "################################\n"); 
    nLOG(LOG_ERR, "Full Accepte Point\n");
    nLOG(LOG_DEBUG, "################################\n");

    return FAILURE;
}

/*
 * Description: Process 연결정보 리스트에 Process 연결정보를 등록하는 함수
 *
 * @param [in] fd : 등록할 Process의 fd
 * @param [in] process : 등록할 Process
 *
 * @retval : SUCCESS(0) : Process 등록 성공,
 *           FAILURE(-1) : Process 정보가 잘못되었음 or 이미 등록되어 있음
 */

Int32T ipcmgrProcessInfoRegister(Int32T fd, Uint32T process)
{
    Uint32T i;

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "Process InFormation Register\n");
    nLOG(LOG_DEBUG, "################################\n");   

    if(process < 0 || process >= MAX_PROCESS_CNT)
    {
        nLOG(LOG_ERR, "Argument Failure - Process : %d\n", process);
        return FAILURE;
    }

    if(gIpcCommMgt[process].acceptFd == -1)
    {
        for(i=0; i<MAX_PROCESS_CNT; i++)
        {
            if(gIpcAcceptMgt[i].fd == fd)
            {
                gIpcCommMgt[process].acceptFd = gIpcAcceptMgt[i].fd;

                memcpy(&gIpcCommMgt[process].connectInfo,
                       &gIpcAcceptMgt[i].addr,
                       sizeof(struct sockaddr));

                memcpy(&gIpcCommMgt[process].allocateInfo,
                       &gIpcAcceptMgt[i].addr,
                       sizeof(struct sockaddr));
 
                ((struct sockaddr_in*)&gIpcCommMgt[process].allocateInfo)
                 ->sin_port = IPC_MANAGER_PORT + process;

                nLOG(LOG_DEBUG, "gIpcCommMgt[%d].acceptFd = %d\n", 
                       process, gIpcCommMgt[process].acceptFd);

                nLOG(LOG_DEBUG, "connectInfo : %u.%u.%u.%u, %d\n",
                      IP_PRINT
                       (((struct sockaddr_in*)&gIpcCommMgt[process].connectInfo)
                         ->sin_addr.s_addr),
                        ((struct sockaddr_in*)&gIpcCommMgt[process].connectInfo)
                         ->sin_port);
 
                nLOG(LOG_DEBUG, "allocateInfo : %u.%u.%u.%u, %d\n",
                     IP_PRINT
                      (((struct sockaddr_in*)&gIpcCommMgt[process].allocateInfo)
                        ->sin_addr.s_addr),
                       ((struct sockaddr_in*)&gIpcCommMgt[process].allocateInfo)
                        ->sin_port);

                gIpcAcceptMgt[i].fd = -1;
                memset(&gIpcAcceptMgt[i].addr, 0x00, sizeof(struct sockaddr));

                return SUCCESS;
            }
        }
    }
    else
    {
        nLOG(LOG_DEBUG, "################################\n");
        nLOG(LOG_WARNING, "Process - %d : same process multi access\n", 
                            process);
        nLOG(LOG_DEBUG, "################################\n");
        return FAILURE;
    }

    return SUCCESS;
}

/*
 * Description: Process 연결정보 리스트에 Process 연결정보를 삭제하는 함수
 *
 * @param [in] fd : 삭제할 Process의 fd
 *
 * @retval : SUCCESS(0) : Process 삭제 성공,
 *           FAILURE(-1) : 해당 Process 정보가 없음
 */

Int32T ipcmgrProcessInfoDelete(Int32T fd)
{
    Int32T i;

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "Process Infomation Delete\n");
    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "Input Fd : %d\n", fd);

    for(i=0; i<MAX_PROCESS_CNT; i++)
    {
        if(fd == gIpcCommMgt[i].acceptFd)
        {
            nLOG(LOG_DEBUG, "[Before Information]\n");
            nLOG(LOG_DEBUG, "gIpcCommMgt[%d].acceptFd = %d\n", 
                             i, gIpcCommMgt[i].acceptFd);
 
            nLOG(LOG_DEBUG, "connectInfo : %u.%u.%u.%u, %d\n",
                           IP_PRINT
                             (((struct sockaddr_in*)&gIpcCommMgt[i].connectInfo)
                               ->sin_addr.s_addr),
                              ((struct sockaddr_in*)&gIpcCommMgt[i].connectInfo)
                               ->sin_port);
 
            nLOG(LOG_DEBUG, "allocateInfo : %u.%u.%u.%u, %d\n",
                           IP_PRINT
                            (((struct sockaddr_in*)&gIpcCommMgt[i].allocateInfo)
                              ->sin_addr.s_addr),
                             ((struct sockaddr_in*)&gIpcCommMgt[i].allocateInfo)
                              ->sin_port);
                
            gIpcCommMgt[i].acceptFd = -1;
            
            if(gIpcCommMgt[i].bev != NULL)
            {
                bufferevent_free(gIpcCommMgt[i].bev);
                gIpcCommMgt[i].bev = NULL;
            }

            gIpcCommMgt[i].bufSize = 0;

            if(gIpcCommMgt[i].buf != NULL)
            {
                nFREE(IPC_MESSAGE, gIpcCommMgt[i].buf);
            }
             
            memset(&gIpcCommMgt[i].connectInfo, 0x00, sizeof(struct sockaddr));
            memset(&gIpcCommMgt[i].allocateInfo, 0x00, sizeof(struct sockaddr));

            nLOG(LOG_DEBUG, "[After Information]\n");

            nLOG(LOG_DEBUG, "gIpcCommMgt[%d].acceptFd = %d\n",
                             i, gIpcCommMgt[i].acceptFd);
            nLOG(LOG_DEBUG, "gIpcCommMgt[%d].bev = %d\n",
                             i, gIpcCommMgt[i].bev);
            nLOG(LOG_DEBUG, "gIpcCommMgt[%d].connectInfo = %u.%u.%u.%u, %d\n",
                             i,
                             IP_PRINT
                             (((struct sockaddr_in*)&gIpcCommMgt[i].connectInfo)
                                ->sin_addr.s_addr),
                              ((struct sockaddr_in*)&gIpcCommMgt[i].connectInfo)
                                ->sin_port);
            nLOG(LOG_DEBUG, "gIpcCommMgt[%d].allocateInfo = %u.%u.%u.%u, %d\n",
                            i,
                            IP_PRINT
                            (((struct sockaddr_in*)&gIpcCommMgt[i].allocateInfo)
                                ->sin_addr.s_addr),
                             ((struct sockaddr_in*)&gIpcCommMgt[i].allocateInfo)
                                ->sin_port);
            nLOG(LOG_DEBUG, "gIpcCommMgt[%d].bufSize = %d\n",
                             i, gIpcCommMgt[i].bufSize);
            nLOG(LOG_DEBUG, "gIpcCommMgt[%d].buf = %d\n",
                             i, gIpcCommMgt[i].buf);

            nLOG(LOG_DEBUG, "################################\n");
            nLOG(LOG_INFO, "Process Info Delete : Success\n");
            nLOG(LOG_DEBUG, "################################\n");

            return SUCCESS;
        }
    }

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_ERR, "Process Info Delete : Failure\n");
    nLOG(LOG_DEBUG, "################################\n");

    return FAILURE;
}

/*
 * Description: 할당된 Process 연결정보를 Process에게 전송하는 함수
 *
 * @param [in] process : 전송할 Process
 *
 * @retval : Message : 전송할 Message,
 *           FAILURE(-1) : Message 생성 실패
 */

IpcMsgHeaderT * ipcmgrProcessInfoPut (Uint32T process)
{
    IpcMsgHeaderT *pMsg = (IpcMsgHeaderT *) 0;
    Int32T msgLen = IPC_MSG_HEADER_SIZE;
    IpcInfoPutMsgT *pData = (IpcInfoPutMsgT *) 0;
    Int32T dataLen = sizeof(IpcInfoPutMsgT);
	
    // 1. Process Info Put Message Header, Message init.

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "Process Infomation Put - process : %d\n", process);
    nLOG(LOG_DEBUG, "################################\n");

    msgLen = msgLen + dataLen;

    pMsg = (IpcMsgHeaderT *)nMALLOC(MESSAGE_HEADER, msgLen);
    if(pMsg <= 0)
    {
        nLOG(LOG_DEBUG, "################################\n");
        nLOG(LOG_ERR, "Message Create Failure\n"); 
        nLOG(LOG_DEBUG, "################################\n");
        return (IpcMsgHeaderT *) FAILURE;
    }

    pData = nMALLOC(PROCESS_SINGLE_INFO, dataLen);
    if(pData <= 0)
    {
        nLOG(LOG_DEBUG, "################################\n");
        nLOG(LOG_ERR, "Message Data Create Failure\n");
        nLOG(LOG_DEBUG, "################################\n");
        return (IpcMsgHeaderT *) FAILURE;
    }

    // 2. Message context setting

    pMsg->type = IPC_PROCESSINFO_PUT;
    pMsg->msgId = gIpcMgrRunFlag;

    sprintf(pData->sIpAddr, "%u.%u.%u.%u\n",
            IP_PRINT
            (((struct sockaddr_in*)&gIpcCommMgt[process].allocateInfo)
              ->sin_addr.s_addr));

    pData->port
     = ((struct sockaddr_in*)&gIpcCommMgt[process].allocateInfo)->sin_port;


    pMsg->length = msgLen;	
    memcpy(pMsg->data, pData, dataLen);

    nLOG(LOG_DEBUG, "type : %d, gIpcMgrRunFlag : %d, msgLen : %d\n \
                      pData->sIpAddr : %s, pData->port : %d\n",
                      pMsg->type, pMsg->msgId, pMsg->length,
                      pData->sIpAddr, pData->port);


    nFREE(PROCESS_SINGLE_INFO, pData);

    return pMsg;
}


/*
 * Description: Process Bind 완료 확인 메시지를  Component에 전송하는 함수
 *
 * @retval : SUCCESS(0) : 전송 성공,
 *           FAILURE(-1) : Message 생성 실패
 */

Int32T ipcmgrBindCompleteAck(Int32T process)
{
    IpcMsgHeaderT *pMsg = (IpcMsgHeaderT *) 0;
    Int32T msgLen = IPC_MSG_HEADER_SIZE;

    nLOGDEBUG(LOG_INFO, "Send 'Bind Complete Ack' Message (Async Mode)\n");

    pMsg = (IpcMsgHeaderT *)nMALLOC(MESSAGE_HEADER, msgLen);
    if (pMsg <= 0)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Message Header Create Failure\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return FAILURE;
    }

    nLOGDEBUG(LOG_DEBUG, "Message Header Create Success\n");

    pMsg->type = IPC_BIND_COMPLETE_ACK;
    pMsg->msgId = -1;
    pMsg->length = msgLen;

    nLOGDEBUG(LOG_DEBUG, "Message Header Information\n");
    nLOGDEBUG(LOG_DEBUG,
               "pMsg->type : %d, pMsg->msgId : %d, pMsg->length : %d\n",
                pMsg->type, pMsg->msgId, pMsg->length);


    if(gIpcCommMgt[process].bev != NULL)
    {
        bufferevent_write(gIpcCommMgt[process].bev, pMsg, pMsg->length);
        nLOGDEBUG(LOG_INFO, "Bind Complete Ack Message Send Success\n");
        nFREE(MESSAGE_HEADER, pMsg);
        return SUCCESS;
    }
    else
    {
        nLOGDEBUG(LOG_ERR, 
                  "Message Send Failure:Component(%d) Not Connect\n", process);
        nFREE(MESSAGE_HEADER, pMsg);
        return FAILURE;
    }

    return SUCCESS;
}

/*
 * Description: 모든 Process 연결정보 리스트를 PM 에게 전송하는 함수
 *
 * @retval : SUCCESS(0) : 전송 성공,
 *           FAILURE(-1) : Message 생성 실패
 */

// Int32T ipcmgrProcAllInfoSend(void)
Int32T ipcmgrProcessInfoSend(void)
{
    IpcMsgHeaderT *pMsg = (IpcMsgHeaderT *) 0;
    Int32T msgLen = IPC_MSG_HEADER_SIZE;
    struct sockaddr sendAddr[MAX_PROCESS_CNT];
    Int32T sendProcess[MAX_PROCESS_CNT] = {0, };
    Int32T i;

    // 1. send message setting

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "All Process Connect Info Put [All Complete]\n");
    nLOG(LOG_DEBUG, "################################\n");
 
    for(i=0; i<MAX_PROCESS_CNT; i++)
    {
        memset(&sendAddr[i], 0x00, sizeof(struct sockaddr));
    }

    for(i=0; i<MAX_PROCESS_CNT; i++)
    {
        memcpy(&sendAddr[i], 
               &gIpcCommMgt[i].allocateInfo, 
               sizeof(struct sockaddr));

        if(gIpcCommMgt[i].acceptFd != -1)
        {
            sendProcess[i] = IPC_REG_CHECK;

            nLOG(LOG_DEBUG, "sendAddr[Process Id : %d] : %u.%u.%u.%u, %d\n", 
                i, 
                IP_PRINT(((struct sockaddr_in*)&sendAddr + i)->sin_addr.s_addr),
                         ((struct sockaddr_in*)&sendAddr + i)->sin_port);
        }
    }
          
 
    // 2. Message Header init

    msgLen = msgLen + sizeof(sendAddr);
    pMsg = (IpcMsgHeaderT *)nMALLOC(MESSAGE_HEADER, msgLen);
    if(pMsg <= 0)
    {
        nLOG(LOG_DEBUG, "################################\n");
        nLOG(LOG_ERR, "Message Create Failure\n");
        nLOG(LOG_DEBUG, "################################\n");
        return FAILURE;
    }

    // 3. Message Context Setting

    pMsg->type = IPC_PROCESSINFO_UPDATE;
    // pMsg->type = IPC_PROC_ALL_INFO_UPDATE;
    pMsg->msgId = gIpcMgrRunFlag;
    pMsg->length = msgLen;

    memcpy (pMsg->data, sendAddr, sizeof(sendAddr));

    nLOG(LOG_DEBUG, "type : %d, gIpcMgrRunFlag : %d, msgLen : %d\n",
                      pMsg->type, pMsg->msgId, pMsg->length);

    // 4. Message Send and free

    for(i=1; i<MAX_PROCESS_CNT; i++)
    {
        if(sendProcess[i] == IPC_REG_CHECK)
        {
            bufferevent_write(gIpcCommMgt[i].bev, pMsg, pMsg->length);
        }
    }

    /* Process Manager Send _ 20140415 JAESU HAN */

    /*
    if(gIpcCommMgt[PROCESS_MANAGER].bev != NULL)
    {
        bufferevent_write(gIpcCommMgt[PROCESS_MANAGER].bev, pMsg, pMsg->length);
        nLOG(LOG_DEBUG, "################################\n");
        nLOG(LOG_INFO, "Process Info List Send Success\n");
        nLOG(LOG_INFO, "IPC Manager -> Process Manager\n");
        nLOG(LOG_DEBUG, "################################\n");

        nFREE(MESSAGE_HEADER, pMsg);
        return SUCCESS;

    }
    else
    {
        nLOG(LOG_DEBUG, "################################\n");
        nLOG(LOG_ERR, 
                  "Message Send Failure - Process Manager Not Connect\n");
        nLOG(LOG_DEBUG, "################################\n");

        nFREE(MESSAGE_HEADER, pMsg);
        return FAILURE;
    }

    */
    

    nFREE(MESSAGE_HEADER, pMsg);

    nLOG(LOG_DEBUG, "################################\n");
    nLOG(LOG_INFO, "All Process Connect Info Put - SUCCESS\n");
    nLOG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}



