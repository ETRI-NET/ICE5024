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
 * @brief : Event Service
 *  - Block Name : Event Service Library
 *  - Process Name :
 *  - Creator : JaeSu Han
 *  - Initial Date : 2013/7/15
*/

/**
 * @file : eventService.c
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

#include "taskManager.h"

/*******************************************************************************
 *                              GLOBAL VARIABLES
 ******************************************************************************/


/*******************************************************************************
 *                         GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 *                         LOCAL CONSTANTS/LITERALS/TYPES
 ******************************************************************************/

// N2OS Component String (Ref. nnDefines.h)
const Int8T sEventProcessString[][24] = {
  "IPC MANAGER",
  "PROCESS MANAGER",
  "PORT INTERFACE MANAGER",
  "RIB MANAGER",
  "POLICY MANAGER",
  "COMMAND MANAGER",
  "MULTICAST RIB MANAGER",
  "LIB MANAGER",
  "CHECKPOINT MANAGER",
  "LACP",
  "MSTP",
  "GVRP",
  "IGMP",
  "RIP",
  "ISIS",
  "OSPF",
  "BGP",
  "PIM",
  "RSVP",
  "LDP",
  "RIB_TESTER",
  "DLU_TESTER"

};

/*******************************************************************************
*                               LOCAL VARIABLES
*******************************************************************************/


/*******************************************************************************
 *                                   CODE
 ******************************************************************************/

/**
 * Description: EVENT Serivce Channel을 Open 하는 함수
 *
 * @param [in] recvFunc : EVENT Message 가 수신되었을 때, Callback 되는 함수
 *
 * @retval : SUCCESS(0) : 성공,
 *           EVENT_ERR_ARGUMENT(-1) : 잘못된 인자 값
 *
 * @bug
 *  반드시, taskCreate(), ipcChannelOpen()을 먼저 실행해야 한다.
 *
 * Example Code Usage
 * @code
 *  void * ipcCallback (int msgId, void * data, Uint32T dataLen)
 *  {
 *    ... // IPC MsgId에 따라 Message 처리
 *  }
 *
 *  void * eventCallback (int msgId, void * data, Uint32T dataLen)
 *  {
 *    ... // Event msgId 에 따라 Message 처리
 *  }
 *
 *  Int main()
 *  {
 *    Int32T ret = 0;
 *
 *    ...
 *
 *    ret = _eventOpen(eventCallback);
 *    
 *    ...
 *  }
 * @endcode
 */

Int32T _eventOpen (pEventRegFunc recvFunc)
{
  // 1. Argument Checking

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "Event Service Open Start\n");

  if(recvFunc == NULL)
  {
    nLOGDEBUG(LOG_ERR, "Event RecvFunc() is NULL\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return EVENT_ERR_ARGUMENT;    // -1
  }

  // 2. Event Message Callback Func. is Setting

  gEventCb = recvFunc;

  nLOGDEBUG(LOG_INFO, "Event Service Open Success\n");
  nLOGDEBUG(LOG_DEBUG, "################################\n");

  return SUCCESS;    // 0
}

Int32T _eventChannelUpdate(pEventRegFunc recvFunc)
{
    gEventCb = recvFunc;
    return SUCCESS;
}

/**
 * Description: 전달받으려는 EVENT를 등록하는 함수
 *
 * @param [in] eventId : 등록하려는 EVENT ID
 * @param [in] priority : 해당 EVENT에 대한 Priority
 *
 * @retval : SUCCESS(0) : 성공,
 *           EVENT_ERR_ARGUMENT(-1) : 잘못된 인자 값,
 *           EVENT_ERR_MSG_CREATE(-2) : Message 생성 실패,
 *           EVENT_ERR_PRECEDING_TASK(-4) : 선행조건을 완료하지 못함,
 *           EVENT_ERR_MSG_SEND(-5) : Message 전송 실패
 *
 * @bug
 *  반드시, _eventOpen()을 먼저 실행해야 한다.
 *
 * Example Code Usage
 * @code
 *  Int32T ribmgr_eventSubscribe(Int32T eventId, Int32T pri)
 *  {
 *    Int32T ret = 0;
 *    ...
 *
 *    ret = _eventSubscribe(eventId, pri);
 *
 *    ...
 *  }
 * @endcode
 * @code
 *  Int32T ret = 0;
 *  ret = _eventSubscribe(EVENT_ROUTER_ID, EVENT_PRI_HIGH);
 * @endcode
 */

Int32T _eventSubscribe (Uint32T eventId, Uint32T priority)
{
  IpcMsgHeaderT *pMsg = (IpcMsgHeaderT *) 0;
  Int32T msgLen = IPC_MSG_HEADER_SIZE;
  EventSubMsgT *pData = NULL;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "Event Subscribe Start\n");
  nLOGDEBUG(LOG_INFO, "eventId : %d, priority : %d\n", eventId, priority);


  // 1. Check the Proceding _eventOpen()

  if(gEventCb == NULL)
  {
    nLOGDEBUG(LOG_ERR, "Event RecvFunc() is NULL\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return EVENT_ERR_PRECEDING_TASK;    // -4
  }

  nLOGDEBUG(LOG_DEBUG, "Event RecvFunc() Check Success\n");


  // 2. Argument Checking

  if(priority < EVENT_PRI_HIGH && priority > EVENT_PRI_LOW)
  {
    nLOGDEBUG(LOG_ERR, "Invalid priority : %d\n", priority);
    nLOGDEBUG(LOG_ERR, "Priority Range %d ~ %d\n", 
                        EVENT_PRI_LOW, EVENT_PRI_HIGH);
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return EVENT_ERR_ARGUMENT;    // -1
  }

  nLOGDEBUG(LOG_DEBUG, "Priority Check Success\n");

  if(eventId < 0 && eventId >= MAX_EVENT_CNT)
  {
    nLOGDEBUG(LOG_ERR, "Invalid eventId : %d\n", eventId);
    nLOGDEBUG(LOG_ERR, "eventId Range 0 ~ %d\n", MAX_EVENT_CNT-1);
    nLOGDEBUG(LOG_DEBUG, "################################\n"); 
    return EVENT_ERR_ARGUMENT;    // -1
  }

  nLOGDEBUG(LOG_DEBUG, "Event Id Check Success\n");


  // 3. Subscribe Message Header, Message init.

  msgLen = msgLen + sizeof(EventSubMsgT);
  nLOGDEBUG(LOG_DEBUG, 
            "Subscribe Message Len = %d (Header(%d) + body(%d))\n",
             msgLen, IPC_MSG_HEADER_SIZE, sizeof(EventSubMsgT));

  pMsg = (IpcMsgHeaderT *)nMALLOC(MESSAGE_HEADER, msgLen);
  if (pMsg <= 0)
  {
    nLOGDEBUG(LOG_ERR, "Message Header Create Failure\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return EVENT_ERR_MSG_CREATE;    // -2
  }
  nLOGDEBUG(LOG_DEBUG, "Message Header Create Success\n");

  pData = (EventSubMsgT *)nMALLOC(EVENT_SUB_MESSAGE, sizeof(EventSubMsgT));
  if (pData <= 0)
  {
    nLOGDEBUG(LOG_ERR, "Message Body Create Failure\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return EVENT_ERR_MSG_CREATE;    // -2
  }
  nLOGDEBUG(LOG_DEBUG, "Message Body Create Success\n");


  // 4. Message context setting

  pMsg->type = EVENT_SUBSCRIBE;
  pMsg->msgId = -1;
  pMsg->length = msgLen;

  nLOGDEBUG(LOG_DEBUG, "Message Header Information\n");
  nLOGDEBUG(LOG_DEBUG, 
            "pMsg->type : %d, pMsg->msgId : %d, pMsg->length : %d\n",
             pMsg->type, pMsg->msgId, pMsg->length);

  pData->process = gUserProcess;
  pData->eventId = eventId;
  pData->priority = priority;

  nLOGDEBUG(LOG_DEBUG, "Message Body Infomation\n");
  nLOGDEBUG(LOG_DEBUG, "process : %s(%d), eventId : %d, priority : %d\n",
                        sEventProcessString[pData->process], pData->process, 
                        pData->eventId, pData->priority);

  memcpy (pMsg->data, pData, sizeof(EventSubMsgT));

  nLOGDEBUG(LOG_DEBUG, "Message Context Setting Success\n");


  // 5. Message Send and free

  if(gIpcCommMgt[IPC_MANAGER].bev != NULL)
  {
    bufferevent_write(gIpcCommMgt[IPC_MANAGER].bev, pMsg, pMsg->length);
    nLOGDEBUG(LOG_INFO, "Event Subscribe Message Send Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n"); 

    nFREE(MESSAGE_HEADER, pMsg);
    nFREE(EVENT_SUB_MESSAGE, pData);
    return SUCCESS;
    
  }
  else
  {
    nLOGDEBUG(LOG_ERR, "Message Send Failure - IPC Manager Not Connect\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    nFREE(MESSAGE_HEADER, pMsg);
    nFREE(EVENT_SUB_MESSAGE, pData);
    return EVENT_ERR_MSG_SEND;
  }
}

/**
 * Description: 등록한 EVENT를 해제하는 함수
 *
 * @param [in] eventId : 등록을 해제하려는 EVENT ID
 *
 * @retval : SUCCESS(0) : 성공,
 *           EVENT_ERR_ARGUMENT(-1) : 잘못된 인자 값,
 *           EVENT_ERR_MSG_CREATE(-2) : Message 생성 실패,
 *           EVENT_ERR_MSG_SEND(-5) : Message 전송 실패
 *
 * @bug
 *  반드시, _eventOpen()을 먼저 실행해야 한다.
 *  등록하지 않은 Event를 Unsubscribe 해도 실행된다.
 *
 * Example Code Usage
 * @code
 *  Int32T ribmgr_eventUnsubscribe(Int32T eventId)
 *  {
 *    Int32T ret = 0;
 *    ...
 *
 *    ret = _eventUnsubscribe(eventId);
 *
 *    ...
 *  }
 * @endcode
 * @code
 *  Int32T ret = 0;
 *  ret = _eventUnsubscribe(EVENT_ROUTER_ID);
 * @endcode
 */

Int32T _eventUnsubscribe (Uint32T eventId)
{
  IpcMsgHeaderT *pMsg = (IpcMsgHeaderT *) 0;
  Int32T msgLen = IPC_MSG_HEADER_SIZE;
  EventSubMsgT *pData = NULL;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "Event Unsubscribe Start\n");
  nLOGDEBUG(LOG_INFO, "eventId : %d\n", eventId);

  // 1. Argument Checking

  if(eventId < 0 && eventId >= MAX_EVENT_CNT)
  {
    nLOGDEBUG(LOG_ERR, "Invalid eventId : %d\n", eventId);
    nLOGDEBUG(LOG_ERR, "eventId Range 0 ~ %d\n", MAX_EVENT_CNT-1);
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return EVENT_ERR_ARGUMENT;    // -1
  }

  nLOGDEBUG(LOG_DEBUG, "Event Id Check Success\n");


  // 2. Unsubscribe Message Header, Message init.

  msgLen = msgLen + sizeof(EventSubMsgT);
  nLOGDEBUG(LOG_DEBUG, 
            "Unsubscribe Message Len = %d (Header(%d) + body(%d))\n",
             msgLen, IPC_MSG_HEADER_SIZE, sizeof(EventSubMsgT));

  pMsg = (IpcMsgHeaderT *)nMALLOC(MESSAGE_HEADER, msgLen);
  if (pMsg <= 0)
  {
    nLOGDEBUG(LOG_ERR, "Message Header Create Failure\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return EVENT_ERR_MSG_CREATE;    // -2
  }
  nLOGDEBUG(LOG_DEBUG, "Message Header Create Success\n");

  pData = (EventSubMsgT *)nMALLOC(EVENT_SUB_MESSAGE, sizeof(EventSubMsgT));
  if (pData <= 0)
  {
    nLOGDEBUG(LOG_ERR, "Message Body Create Failure\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return EVENT_ERR_MSG_CREATE;    // -2
  }
  nLOGDEBUG(LOG_DEBUG, "Message Body Create Success\n");


  // 3. Message context setting

  pMsg->type = EVENT_UNSUBSCRIBE;
  pMsg->msgId = -1;
  pMsg->length = msgLen;

  nLOGDEBUG(LOG_DEBUG, "Message Header Information\n");
  nLOGDEBUG(LOG_DEBUG, 
            "pMsg->type : %d, pMsg->msgId : %d, pMsg->length : %d\n",
             pMsg->type, pMsg->msgId, pMsg->length);

  pData->process = gUserProcess;
  pData->eventId = eventId;
  pData->priority = -1;

  nLOGDEBUG(LOG_DEBUG, "Message Body Infomation\n");
  nLOGDEBUG(LOG_DEBUG, "process : %s(%d), eventId : %d, priority : %d\n",
                        sEventProcessString[pData->process], pData->process, 
                        pData->eventId, pData->priority);

  memcpy (pMsg->data, pData, sizeof(EventSubMsgT));

  nLOGDEBUG(LOG_DEBUG, "Message Context Setting Success\n");


  // 4. Message Send and free

  if(gIpcCommMgt[IPC_MANAGER].bev != NULL)
  {
    bufferevent_write(gIpcCommMgt[IPC_MANAGER].bev, pMsg, pMsg->length);
    nLOGDEBUG(LOG_INFO, "Event Unsubscribe Message Send Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    nFREE(MESSAGE_HEADER, pMsg);
    nFREE(EVENT_SUB_MESSAGE, pData);
    return SUCCESS;
  }
  else
  {
    nLOGDEBUG(LOG_ERR, "Message Send Failure - IPC Manager Not Connect\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    nFREE(MESSAGE_HEADER, pMsg);
    nFREE(EVENT_SUB_MESSAGE, pData);
    return EVENT_ERR_MSG_SEND;
  }
}

/**
 * Description: 해당 EVENT를 발생시키는 함수
 *
 * @param [in] eventId : 발생시키려는 EVENT ID
 * @param [in] eventData : 전송하려는 Data
 * @param [in] dataSize : 전송하려는 Data의 크기
 *
 * @retval : SUCCESS(0) : 성공,
 *           EVENT_ERR_ARGUMENT(-1) : 잘못된 인자 값,
 *           EVENT_ERR_MSG_CREATE(-2) : Message 생성 실패,
 *           EVENT_ERR_MSG_SIZE_BIG(-3) : Message 크기가 너무 큼,
 *           EVENT_ERR_MSG_SEND(-5) : Message 전송 실패
 * @bug
 *  반드시, taskCreate(), ipcChannelOpen()을 먼저 실행해야 한다.
 *
 * Example Code Usage
 * @code
 *    Int32T ret = 0;
 *    ret = _eventPublish(EVENT_ROUTER_ID, eventData, size);
 * @endcode
 */

Int32T _eventPublish (Uint32T eventId, void * eventData, Uint32T dataSize)
{
  IpcMsgHeaderT *pMsg = (IpcMsgHeaderT *) 0;
  Int32T msgLen = IPC_MSG_HEADER_SIZE + dataSize;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "Event Pubilsh Start : Process(%d) -> IPC Manager\n", 
                       gUserProcess);
  nLOGDEBUG(LOG_INFO, "eventId : %d, eventData : %p, dataSize : %d\n",
                       eventId, eventData, dataSize);
  nLOGDEBUG(LOG_DEBUG, "################################\n");


  // 1. Argument Checking

  if(eventId < 0 && eventId >= MAX_EVENT_CNT)
  {
    nLOGDEBUG(LOG_ERR, "Invalid eventId : %d\n", eventId);
    nLOGDEBUG(LOG_ERR, "eventId Range 0 ~ %d\n", MAX_EVENT_CNT-1);
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return EVENT_ERR_ARGUMENT;
  }

  nLOGDEBUG(LOG_DEBUG, "Event Id Check Success\n");

  if(msgLen > EVENT_MAX_MESSAGE_SIZE)
  {
    nLOGDEBUG(LOG_ERR,
              "Total Message Size : %d > EVENT_MAX_MESSAGE_SIZE(%d)\n",
               msgLen, EVENT_MAX_MESSAGE_SIZE);
    nLOGDEBUG(LOG_DEBUG, "################################\n");  
    return EVENT_ERR_MSG_SIZE_BIG;
  }

  nLOGDEBUG(LOG_DEBUG, "Total Message Size Check Success\n");


  // 2. Publish Message Header, Message init.

  pMsg = (IpcMsgHeaderT *)nMALLOC(MESSAGE_HEADER, msgLen);
  if (pMsg <= 0)
  {
    nLOGDEBUG(LOG_ERR, "Message Header Create Failure\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return EVENT_ERR_MSG_CREATE;
  }
  nLOGDEBUG(LOG_DEBUG, "Message Header Create Success\n");


  // 3. Message context setting

  pMsg->type = EVENT_SERVICE;
  pMsg->msgId = eventId;
  pMsg->length = msgLen;

  nLOGDEBUG(LOG_DEBUG, "Message Header Information\n");
  nLOGDEBUG(LOG_DEBUG,
            "pMsg->type : %d, pMsg->msgId : %d, pMsg->length : %d\n",
             pMsg->type, pMsg->msgId, pMsg->length);

  memcpy(pMsg->data, eventData, dataSize);

  nLOGDEBUG(LOG_DEBUG, "Message Context Setting Success\n");


  // 4. Event Message Send and free

  if(gIpcCommMgt[IPC_MANAGER].bev != NULL)
  {
    bufferevent_write(gIpcCommMgt[IPC_MANAGER].bev, pMsg, pMsg->length);
    nLOGDEBUG(LOG_INFO, "Event Pubilsh Message Send Success\n");
    nLOGDEBUG(LOG_INFO, "%s(%d) -> IPC Manager\n", 
                         sEventProcessString[gUserProcess], gUserProcess);
    nLOGDEBUG(LOG_DEBUG, "################################\n");
       
    nFREE(MESSAGE_HEADER, pMsg);
    return SUCCESS;

  }
  else
  {
    nLOGDEBUG(LOG_ERR, "Message Send Failure - IPC Manager Not Connect\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    nFREE(MESSAGE_HEADER, pMsg);
    return EVENT_ERR_MSG_SEND;
  }  
}

/*
 * Description: 해당 Event ID에 Event Service를 발생시키는 함수
 *              (IPC Manager 전용 Function)
 *
 * @param [in] process : event 전송 대상 Process
 * @param [in] eventId : 발생시키려는 event ID
 * @param [in] data : 전송하려는 Data
 * @param [in] size : Event Message 총 크기 (Header + data)
 *
 * @retval : SUCCESS(0) : 성공, FAILURE(-1) : 실패
 */

Int32T _eventSubscribeSend 
 (Uint32T process, Uint32T eventId, void * data, Int32T size)
{

  IpcMsgHeaderT *pMsg = (IpcMsgHeaderT *) 0;
  Int32T msgLen = size;
  Int32T dataLen = size - IPC_MSG_HEADER_SIZE;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "EventId(%d) : %s(%d)\n", 
                       eventId, sEventProcessString[process], process);
  nLOGDEBUG(LOG_DEBUG, "data : %p, Message Size : %d, Data Size : %d\n",
                        data, msgLen, dataLen);


  // 1. Message Header Init

  pMsg = (IpcMsgHeaderT *)nMALLOC(MESSAGE_HEADER, msgLen);
  if(pMsg <= 0)
  {
    nLOGDEBUG(LOG_ERR, "Event Message Create Failure\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");  
    return FAILURE;
  }

  // 2. Message Context Setting

  nLOGDEBUG(LOG_INFO, "Event Message Setting\n");
  pMsg->type = EVENT_SERVICE;
  pMsg->msgId = eventId;
  pMsg->length = msgLen;

  if(data != NULL)
  {
    memcpy(pMsg->data, data, dataLen);
  }

  nLOGDEBUG(LOG_INFO, "type : %d, Event Id : %d, msgLen : %d\n",
                       pMsg->type, pMsg->msgId, pMsg->length);
  nLOGDEBUG(LOG_INFO, "Event Message Send\n");

  // 3. Message Send and Free

  if(gIpcCommMgt[process].bev != NULL)
  {
    bufferevent_write(gIpcCommMgt[process].bev, pMsg, pMsg->length);
    nLOGDEBUG(LOG_INFO, "Event Publish Success \n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
  }
  else
  {
    nLOGDEBUG(LOG_ERR, "current process : %s(%d) not connect \n", 
                        sEventProcessString[process], process);
    nLOGDEBUG(LOG_DEBUG, "################################\n");
  }

  nFREE(MESSAGE_HEADER, pMsg);

  return SUCCESS;
}
