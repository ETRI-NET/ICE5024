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
 * @brief : N2OS Data Structure - Circular Queue
 *  - Block Name : Library
 *  - Process Name : rLibraryibmgr
 *  - Creator : Changwoo Lee, JaeSu Han
 *  - Initial Date : 2013/7/15
 */

/**
* @file : nnCqueue.c
*
* $Id: nnCqueue.c 956 2014-03-07 10:09:00Z lcw1127 $
* $Author: lcw1127 $
* $Date: 2014-03-07 19:09:00 +0900 (Fri, 07 Mar 2014) $
* $Revision: 956 $
* $LastChangedBy: lcw1127 $
**/

/*******************************************************************************
 *                               INCLUDE FILES
 ******************************************************************************/
//#include "nnMemmgr.h"
//#include "nnLog.h"
#include "nnDefines.h"
#include "nnCqueue.h"
#include "nosLib.h"

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
 * Description: circular queue를 생성하고 초기화하는 함수
 *
 * @param [in] maxCount : 저장할 Node의 Max Count
 * @param [in] dupCheck : Node의 Data 주소 값의 중복 Check 여부
 * @param [in] dataType : Cqueue를 사용할 Data Type
 *
 * @retval : 초기화된 circular queue,
 *           CQUEUE_ERR_CQUEUE_ALLOC(-3) : 메모리할당 실패,
 *           CQUEUE_ERR_ARGUMENT(-8) : 잘못된 DataType 
 *
 * @code
 *  CqueueT *pCqueue = NULL;
 *
 *  pCqueue = nnCqueueInit(maxCount, dupCheck, dataType);
 * @endcode
 *
 * @test
 *  maxCount 의 값을 0 으로 입력,
 *  dupCheck 의 값을 잘 못된 값으로 입력,
 *  dataType 의 값을 사용자 타입의 범위를 벗어난 값으로 입력
 */

CqueueT * nnCqueueInit(Uint64T maxCount, Uint8T dupCheck, Uint32T dataType)
{
    CqueueT * newCqueue = NULL;

    if((dataType >= MEM_MAX_USER_TYPE && dataType < MEM_SYS_START)
       || dataType >= MEM_MAX_SYS_TYPE)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Wrong Cqueue DataType\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (CqueueT *)CQUEUE_ERR_ARGUMENT;
    }

    newCqueue = NNMALLOC(CQUEUE_HEAD, sizeof(CqueueT));

    if((Int32T)newCqueue <= 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "NewCqueue Memory Allocate : Failure(%d)\n", 
                       (Int32T)newCqueue);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (CqueueT*)CQUEUE_ERR_CQUEUE_ALLOC;
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

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Init NewCqueue : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return newCqueue;
}

/**
 * Description: circular queue를 삭제 하는 함수
 *
 * @param [in] cqueue : 삭제할 Cqueue
 *
 * @retval : None
 *
 * @code
 *  nnCqueueFree(pCqueue);
 * @endcode
 *
 * @test
 *  cqueue 를 NULL 로 입력,
 *  cqueue 에 아무런 값도 없는 상태에서 함수 수행
 */

void nnCqueueFree(CqueueT * cqueue)
{
    if (cqueue == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Cqueue Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    nnCqueueDeleteAllNode(cqueue);
    NNFREE(CQUEUE_HEAD, cqueue);

    cqueue = NULL;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Free Cqueue Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}

/**
 * Description: circular queue의 모든 Node를 삭제 하는 함수
 *
 * @param [in] cqueue : 모든 Node를 삭제할 Cqueue
 *
 * @retval : none
 *
 * @code
 *  nnCqueueDeleteAllNode(pCqueue);
 * @endcode
 *
 * @test
 *  cqueue 를 NULL 로 입력,
 *  cqueue 에 아무런 값도 없는 상태에서 함수 수행
 */

void nnCqueueDeleteAllNode(CqueueT * cqueue)
{
    CqueueNodeT * tempNode = NULL;

    if (cqueue == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Cqueue Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    if (cqueue->count == 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Cqueue Is Empty\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    if(cqueue->count == 1)
    {
        NNFREE(cqueue->dataType, cqueue->head->data);
        NNFREE(CQUEUE_NODE, cqueue->head);

        --cqueue->count;

        cqueue->head = NULL;
        cqueue->tail = NULL;

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Delete All Node Complete\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    for (; tempNode != cqueue->tail;)
    {
        tempNode = cqueue->head;
    
        cqueue->head = tempNode->next;
        cqueue->tail->next = tempNode->next;
        tempNode->next->prev = cqueue->tail;

        NNFREE(cqueue->dataType, tempNode->data);
        NNFREE(CQUEUE_NODE, tempNode);

        --cqueue->count;

        if(cqueue->count == 1)
        {
            NNFREE(cqueue->dataType, cqueue->head->data);
            NNFREE(CQUEUE_NODE, cqueue->head);

            --cqueue->count;

            cqueue->head = NULL;
            cqueue->tail = NULL;
            break;
        }
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Delete All Node Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return;
}

/**
 * Description: circular queue의 첫번째 Data를 가져오는 함수 (Dequeue는 아님)
 *
 * @param [in] cqueue : 첫번째 Data를 가져올 Cqueue
 *
 * @retval : Cqueue의 첫번째 Node의 Data,
 *           CQUEUE_ERR_NULL(-5) : Cqueue가 Null인 경우(메모리 할당되지 않음),
 *           CQUEUE_ERR_EMPTY(-6) : Cqueue의 Node가 없는 경우
 *
 * @code
 *  testT *pRet = NULL;
 *
 *  pRet = nnCqueueGetHead(pCqueue);
 * @endcode
 *
 * @test
 *  cqueue 를 NULL 로 입력,
 *  cqueue 에 아무런 값도 없는 상태에서 함수 수행
 */

void * nnCqueueGetHead (CqueueT * cqueue)
{
    if (cqueue == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Cqueue Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *) CQUEUE_ERR_NULL;
    }
    else if (cqueue->head == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Cqueue Head is NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *) CQUEUE_ERR_EMPTY;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Get Head Data : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return cqueue->head->data;
}

/**
 * Description: circular queue의 마지막 Data를 가져오는 함수 (Dequeue는 아님)
 *
 * @param [in] cqueue : 마지막 Data를 가져올 Cqueue
 *
 * @retval : Cqueue의 마지막 Node의 Data,
 *           CQUEUE_ERR_NULL(-5) : Cqueue가 Null인 경우(메모리 할당되지 않음),
 *           CQUEUE_ERR_EMPTY(-6) : Cqueue의 Node가 없는 경우
 *
 * @code
 *  testT *pRet = NULL;
 *
 *  pRet = nnCqueueGetTail(pCqueue);
 * @endcode
 *
 * @test
 *  cqueue 를 NULL 로 입력,
 *  cqueue 에 아무런 값도 없는 상태에서 함수 수행
 */

void * nnCqueueGetTail (CqueueT * cqueue)
{
    if (cqueue == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Cqueue Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *) CQUEUE_ERR_NULL;
    }
    else if(cqueue->tail == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Cqueue Tail is NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *) CQUEUE_ERR_EMPTY;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Get Tail Data : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return cqueue->tail->data;
}

/**
 * Description: circular queue의 Enqueue되어 있는 Node 수를 가져오는 함수
 *
 * @param [in] cqueue : 현재 Node 수 를 가져올 Cqueue
 *
 * @retval : Cqueue의 현재 Node 수,
 *           CQUEUE_ERR_NULL(-5) : Cqueue가 Null인 경우(메모리 할당되지 않음)
 *
 * @code
 *  Int64T ret = 0;
 *
 *  ret = nnCqueueCount(cqueue);
 * @endcode
 *
 * @test
 *  cqueue 를 NULL 로 입력,
 *  cqueue 에 아무런 값도 없는 상태에서 함수 수행
 */

Int64T nnCqueueCount (CqueueT * cqueue)
{
    if (cqueue == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Cqueue Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return CQUEUE_ERR_NULL; 
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Get Cqueue Count : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return cqueue->count;
}

/**
 * Description: circular queue에 Data를 Enqueue 하는 함수
 *
 * @param [in] cqueue : Data를 Enqueue Cqueue
 * @param [in] data : Enqueue할 Data
 *
 * @retval : SUCCESS(0),
 *           CQUEUE_ERR_FULL(-2) : Cqueue가 Full 상태
 *           CQUEUE_ERR_CQUEUENODE_ALLOC(-3) : NODE 메모리 할당 실패
 *           CQUEUE_ERR_NULL(-5) : Cqueue가 Null인 경우(메모리 할당되지 않음),
 *           CQUEUE_ERR_DUP(-7) : Data가 중복인 경우
 *
 * @code
 *  Int32T ret = 0;
 *  testT pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  ret = nnCqueueEnqueue(cqueue, pTest);
 * @endcode
 *
 * @test
 *  cqueue 를 NULL 로 입력,
 *  data 를 NULL 로 입력,
 *  data 를 cqueue 에 있는 값으로 입력
 */


Int32T nnCqueueEnqueue(CqueueT * cqueue, void * data)
{
    CqueueNodeT * tempNode = NULL;
    CqueueNodeT * newNode;

    if (cqueue == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Cqueue Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return CQUEUE_ERR_NULL;
    }
    else if (data == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return CQUEUE_ERR_NULL;
    }
    
    if(cqueue->count == cqueue->maxCount)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Cqueue Full\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return CQUEUE_ERR_FULL;
    }

    newNode = NNMALLOC(CQUEUE_NODE, sizeof(CqueueNodeT));

    if((Int32T)newNode <= 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "NewCqueueNode Memory Allocate : Failure(%d)\n", 
                       (Int32T)newNode);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

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
                    NNFREE(CQUEUE_NODE, newNode);

                    NNLOGDEBUG(LOG_DEBUG, "################################\n");
                    NNLOGDEBUG(LOG_ERR, "Duplication Data Found\n");
                    NNLOGDEBUG(LOG_DEBUG, "################################\n");

                    return CQUEUE_ERR_DUP;
                }    
            }

            if(tempNode->data == data)
            {
                NNFREE(CQUEUE_NODE, newNode);

                NNLOGDEBUG(LOG_DEBUG, "################################\n");
                NNLOGDEBUG(LOG_ERR, "Duplication Data Found\n");
                NNLOGDEBUG(LOG_DEBUG, "################################\n");

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

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Enqueue Data : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}

/**
 * Description: circular queue에서 Data를 Dequeue 하는 함수
 *
 * @param [in] cqueue : Data를 Dequeue Cqueue
 *
 * @retval : Dequeue된 Node의 Data,
 *           CQUEUE_ERR_NULL(-5) : Cqueue가 Null인 경우(메모리 할당되지 않음),
 *           CQUEUE_ERR_EMPTY(-6) : Cqueue에 저장되어 있는 Node가 없는 경우
 *
 * @code
 *  testT pRet = NULL;
 *
 *  pRet = nnCqueueDequeue(cqueue);
 * @endcode
 *
 * @test
 *  cqueue 를 NULL 로 입력,
 *  data 를 NULL 로 입력,
 *  data 를 cqueue 에 있는 값으로 입력
 */

void * nnCqueueDequeue (CqueueT * cqueue)
{
    CqueueNodeT * tempNode = NULL;
    void * data = NULL;

    if (cqueue == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Cqueue Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *) CQUEUE_ERR_NULL;
    }

    if(cqueue->count == 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Cqueue Is Empty\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *)CQUEUE_ERR_EMPTY;
    }
    else if (cqueue->count == 1)
    {
        cqueue->head->prev = NULL;
        cqueue->head->next = NULL;

        data = cqueue->head->data;

        NNFREE(CQUEUE_NODE, cqueue->head);

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

        NNFREE(CQUEUE_NODE, tempNode);
    }
    
    --cqueue->count;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Dequeue Data : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return data;
}
