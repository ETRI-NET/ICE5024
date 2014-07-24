/**
* @file : ipcmgrEvent.c
* @brief : N2OS의 Event 전송 중계
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

#include "nosLib.h"
#include "ipcmgrInit.h"

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
 * Description: Event 관련 Msg를 처리하는 함수
 *
 * @param [in] msgId : Event 관련 Msg Type
 * @param [in] data  : Event 관련 Msg의 실제 Data 
 * @param [in] dataLen : Data의 길이
 *
 * @retval : None
 */


void ipcmgrEventProcess(Int32T msgId, void * data, Uint32T dataLen)
{
    NNLOG(LOG_DEBUG, "enter %s\n", __FUNCTION__);

    if(msgId == IPCMGR_EVENT_SUBSCRIBE)
    {
        EventSubMsgT eventData;

        NNLOG(LOG_INFO, "pMsg->type : EVENT_SUBSCRIGE\n");

        eventData.process = ((EventSubMsgT *)data)->process;
        eventData.eventId = ((EventSubMsgT *)data)->eventId;
        eventData.priority = ((EventSubMsgT *)data)->priority;

        NNLOG(LOG_INFO, "Process : %d, eventId : %d, priority : %d\n",
                        eventData.process, 
                        eventData.eventId, 
                        eventData.priority);

        if(eventSubscribeRegister
           (eventData.process, eventData.eventId, eventData.priority) < 0)
        {
            NNLOG(LOG_ERR, "Event Subscribe Regist Failure\n");
        }
        else
        {
            NNLOG(LOG_INFO, "Event Subscribe Regist Success\n");
        }
        return;
    }
    else if(msgId == IPCMGR_EVENT_UNSUBSCRIBE)
    {
        EventSubMsgT eventData;

        NNLOG(LOG_INFO, "pMsg->type : EVENT_UNSUBSCRIBE\n");

        eventData.process = ((EventSubMsgT *)data)->process;
        eventData.eventId = ((EventSubMsgT *)data)->eventId;

        NNLOG(LOG_INFO, "Process : %d, eventId : %d\n",
                        eventData.process, eventData.eventId);

        eventSubscribeDeregister(eventData.process, eventData.eventId);      
        return;
    }
    else if (msgId == IPCMGR_EVENT_PROCESS_EOF)
    {
        EventSubMsgT eventData;
        ListNodeT *pTempNode = NULL;
        Int32T eventId;

        NNLOG(LOG_INFO, "pMsg->type : PROCESS EOF\n");

        eventData.process = ((EventSubMsgT *)data)->process;

        for(eventId=0; eventId<MAX_EVENT_CNT; eventId++)
        { 
            for(pTempNode = pIpcmgr->gEventSubList[eventId].subProcess->pHead;
                pTempNode != NULL;
                pTempNode = pTempNode->pNext)
            {
                Uint32T process 
                 = ((EventSubProcessT *)pTempNode->pData)->process;

                if(eventData.process == process)
                {
                    eventSubscribeDeregister(process, eventId);
                    break;
                }
            }
        }
    }
    else if(msgId >= 0)
    {
        ListNodeT *pTempNode = NULL;
        Int32T eventId = msgId;

        NNLOG(LOG_INFO, "pMsg->type : EVENT_SERVICE\n");
        NNLOG(LOG_INFO, "msgId : %d, data : %p, length : %d\n",
                         msgId, data, dataLen);

        for(pTempNode = pIpcmgr->gEventSubList[eventId].subProcess->pHead;
            pTempNode != NULL;
            pTempNode = pTempNode->pNext)
        {
            Uint32T process = ((EventSubProcessT *)pTempNode->pData)->process;
            NNLOG(LOG_INFO, "Send event message process : %d\n", process);

            if(eventSubscribeSend(process, eventId, data, dataLen) < 0)
            {
                NNLOG(LOG_ERR, "Event Send Failure\n");
            }
        }
        return;
    }
}


/*
 * Description: Event Subscribe process 관리 구조체를 초기화하는 함수
 *
 * @retval : None
 */

void eventSubInfoInit(void)
{
    Int32T i;

    NNLOG(LOG_DEBUG, "################################\n");
    NNLOG(LOG_INFO, "Event Subscribe List Initialize\n");
    NNLOG(LOG_DEBUG, "################################\n");

    for(i=0; i<MAX_EVENT_CNT; i++)
    {
        pIpcmgr->gEventSubList[i].subProcess 
            = nnListInit(eventSubPriSortFunc, EVENT_SUB_PROCESS);
  
        NNLOG(LOG_DEBUG, "pIpcmgr->gEventSubList[%d].subProcess = %p\n",
                         i, pIpcmgr->gEventSubList[i].subProcess);
    }
}

/*
 * Description: Event Subscribe process 관리 구조체를 삭제하고 종료하는 함수
 *
 * @retval : None
 */

void eventSubInfoClose(void)
{
    Int32T i;

    NNLOG(LOG_DEBUG, "################################\n");
    NNLOG(LOG_INFO, "Event Subscribe List Free (Close)\n");
    NNLOG(LOG_DEBUG, "################################\n");

    for(i=0; i<MAX_EVENT_CNT; i++)
    {
        NNLOG(LOG_DEBUG, 
              "[Before] pIpcmgr->gEventSubList[%d].subProcess = %p\n",
               i, pIpcmgr->gEventSubList[i].subProcess);

        nnListFree(pIpcmgr->gEventSubList[i].subProcess);
        pIpcmgr->gEventSubList[i].subProcess = NULL;

        NNLOG(LOG_DEBUG, 
              "[After] pIpcmgr->gEventSubList[%d].subProcess = %p\n",
               i, pIpcmgr->gEventSubList[i].subProcess);
    }

    NNLOG(LOG_DEBUG, "################################\n");
    NNLOG(LOG_INFO, "Event Subscribe List Free (Close) Complete\n");
    NNLOG(LOG_DEBUG, "################################\n");
}



/*
 * Description: Subscribe process를 Priority를 기준으로 Sort 하는 함수
 *
 * @param [in] pOldData : Subscribe process1 정보
 * @param [in] pNewData : Subscribe process2 정보
 *
 * @retval : Result(ASC, DESC, Same)
 */

Int8T eventSubPriSortFunc (void * pOldData, void * pNewData)
{

    NNLOG(LOG_DEBUG, "################################\n");
    NNLOG(LOG_INFO, "Event Priority Compare Function\n");
    NNLOG(LOG_INFO, "pOldData Pri : %d, pNewData Pri : %d\n",
                    ((EventSubProcessT *)pOldData)->priority,
                    ((EventSubProcessT *)pNewData)->priority);
    NNLOG(LOG_DEBUG, "################################\n");


    if ((((EventSubProcessT *)pOldData)->priority)
          > (((EventSubProcessT *)pNewData)->priority))
    {
        // if sort then ASC, Left Side is Bigger than Right Side
        NNLOG(LOG_DEBUG, "################################\n");
        NNLOG(LOG_INFO, "pOldData->priority > pNewData->priority\n");
        NNLOG(LOG_DEBUG, "################################\n");
        return 0;
    }
    else if ((((EventSubProcessT *)pOldData)->priority)
              < (((EventSubProcessT *)pNewData)->priority))
    {
        // if sort then DESC, Right Side is Bigger than Left Side
       NNLOG(LOG_DEBUG, "################################\n");
       NNLOG(LOG_INFO, "pOldData->priority < pNewData->priority\n");
       NNLOG(LOG_DEBUG, "################################\n");
       return 1;
    }
    else
    {
        // Same priority
        NNLOG(LOG_DEBUG, "################################\n");
        NNLOG(LOG_INFO, "pOldData->priority = pNewData->priority\n");
        NNLOG(LOG_DEBUG, "################################\n");
        return -1;
    }
}

/*
 * Description: Subscribe process의 중복이 있는지 확인하는 함수
 *
 * @param [in] pOldData : Subscribe process1 정보
 * @param [in] pNewData : Subscribe process2 정보
 *
 * @retval : -1 : Duplicate, 0 : Not
 */

Int8T eventSubDupCheckFunc (void * pOldData, void * pNewData)
{
    NNLOG(LOG_DEBUG, "################################\n");
    NNLOG(LOG_INFO, "Event Subscribe Process Dup. Check\n");
    NNLOG(LOG_INFO, "Process 1 : %d, Process 2 : %d\n",
                    ((EventSubProcessT *)pOldData)->process,
                    ((EventSubProcessT *)pNewData)->process);
    NNLOG(LOG_DEBUG, "################################\n");
 
    if ((((EventSubProcessT *)pOldData)->process)
          == (((EventSubProcessT *)pNewData)->process))
    {
        NNLOG(LOG_DEBUG, "################################\n");
        NNLOG(LOG_INFO, "Dup Process\n");
        NNLOG(LOG_DEBUG, "################################\n");
        return -1;
    }
    
    NNLOG(LOG_DEBUG, "################################\n");
    NNLOG(LOG_INFO, "Not Dup\n");
    NNLOG(LOG_DEBUG, "################################\n");
    return 0;
}

/*
 * Description: 해당 Event ID에 Subscribe process를 등록하는 함수
 *
 * @param [in] process : 등록할 process
 * @param [in] eventId : 등록할 Event ID
 * @param [in] priority : 해당 Event ID에 대한 priority
 *
 * @retval : SUCCESS(0) : 성공,
 *           FAILURE(-1) : 실패
 */

Int32T eventSubscribeRegister
              (Uint32T process, Uint32T eventId, Uint32T priority)
{
    Int32T ret;
    EventSubProcessT * newProcess = NULL;

    NNLOG(LOG_DEBUG, "################################\n");
    NNLOG(LOG_INFO, "Event Subscribe Register\n");
    NNLOG(LOG_INFO, "Process : %d, eventId : %d, priority : %d\n",
                    process, eventId, priority);
    NNLOG(LOG_DEBUG, "################################\n");
 
    newProcess = 
     (EventSubProcessT *)NNMALLOC(EVENT_SUB_PROCESS, sizeof(EventSubProcessT));
	
    if(newProcess <= 0)
    {
        NNLOG(LOG_DEBUG, "################################\n");
        NNLOG(LOG_ERR, "Subsrcibe Structure Create Failure\n");
        NNLOG(LOG_DEBUG, "################################\n");
        return FAILURE;	
    }

    NNLOG(LOG_INFO, "Subsrcibe Structure Setting\n");	
    newProcess->process = process;
    newProcess->priority = priority;

    NNLOG(LOG_INFO, "Duplicate Process Check\n");
    nnListSetNodeCompFunc
        (pIpcmgr->gEventSubList[eventId].subProcess, eventSubDupCheckFunc);

    if(nnListCheckDupData
       (pIpcmgr->gEventSubList[eventId].subProcess, newProcess)
       == LIST_ERR_DUP)
    {
        NNLOG(LOG_DEBUG, "################################\n");
        NNLOG(LOG_ERR,"Duplicate Process\n");
        NNLOG(LOG_DEBUG, "################################\n");

        NNFREE(EVENT_SUB_PROCESS, newProcess);
        eventSubscribePrint(eventId);
        return FAILURE;
    }

    NNLOG(LOG_INFO, "Subsrcibe Process Add\n");

    nnListSetNodeCompFunc
        (pIpcmgr->gEventSubList[eventId].subProcess, eventSubPriSortFunc);

    ret = nnListAddNodeSort
         (pIpcmgr->gEventSubList[eventId].subProcess, newProcess);

    if(ret < 0)
    {
        NNLOG(LOG_DEBUG, "################################\n");
        NNLOG(LOG_ERR, "Subsrcibe Add Failure\n");
        NNLOG(LOG_DEBUG, "################################\n");
        return ret;
    }

    eventSubscribePrint(eventId);
	
    return SUCCESS;
}

/*
 * Description: 해당 Event ID에 Subscribe process를 삭제하는 함수
 *
 * @param [in] process : 삭제할 Subscribe process
 * @param [in] eventId : 삭제할 Event ID
 *
 * @retval : None
 */

void eventSubscribeDeregister (Uint32T process, Uint32T eventId)
{
    ListNodeT *pTempNode = NULL;

    NNLOG(LOG_DEBUG, "################################\n");
    NNLOG(LOG_INFO, "Event Subscribe Deregister\n");
    NNLOG(LOG_INFO, "Process : %d, EventId : %d\n", process, eventId);
    NNLOG(LOG_DEBUG, "################################\n");

    for (pTempNode = pIpcmgr->gEventSubList[eventId].subProcess->pHead; 
         pTempNode != NULL; 
         pTempNode = pTempNode->pNext)
    {
        if(process == ((EventSubProcessT *)pTempNode->pData)->process)
        {
            nnListDeleteNode(pIpcmgr->gEventSubList[eventId].subProcess,
                            (EventSubProcessT *)pTempNode->pData);

            NNLOG(LOG_INFO, "Event Deregister Complete\n");
            break;
        } 
    }
    eventSubscribePrint(eventId);
}

/*
 * Description: 현재 Event Subscribe List를 출력하는 함수
 *
 * @param [in] eventId : 출력하려는 Event ID
 *
 * @retval : None
 */
void eventSubscribePrint (Uint32T eventId)
{
    ListNodeT *pTempNode = NULL;

    NNLOG(LOG_DEBUG, "################################\n");
    NNLOG(LOG_DEBUG, "Event Subscribe Process List\n");
    NNLOG(LOG_DEBUG, "################################\n");

    for (pTempNode = pIpcmgr->gEventSubList[eventId].subProcess->pHead; 
         pTempNode != NULL; 
         pTempNode = pTempNode->pNext)
    {
        if(pTempNode == NULL)
	    {
            break;
        }

        NNLOG(LOG_DEBUG, "[Event Id : %d]  Process : %d, Priority : %d\n", 
              eventId,
              ((EventSubProcessT *)pTempNode->pData)->process,
	          ((EventSubProcessT *)pTempNode->pData)->priority);
    }
 
    NNLOG(LOG_DEBUG, "Current Event Subscribe Process Count : %llu\n",
          nnListCount(pIpcmgr->gEventSubList[eventId].subProcess));

}
