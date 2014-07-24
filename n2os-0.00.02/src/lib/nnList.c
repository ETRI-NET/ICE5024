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
 * @brief : N2OS Data Structure - Linked List
 *  - Block Name : Library
 *  - Process Name : rLibraryibmgr
 *  - Creator : Changwoo Lee, JaeSu Han
 *  - Initial Date : 2013/7/15
 */

/**
* @file : nnList.c
*
* $Id: nnList.c 955 2014-03-07 10:08:40Z lcw1127 $
* $Author: lcw1127 $
* $Date: 2014-03-07 19:08:40 +0900 (Fri, 07 Mar 2014) $
* $Revision: 955 $
* $LastChangedBy: lcw1127 $
**/

/*******************************************************************************
 *                               INCLUDE FILES
 ******************************************************************************/
//#include "nnMemmgr.h"
//#include "nnLog.h"
#include "nnList.h"
#include "nnDefines.h"
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
 * Description: List 를 사용할 수 있도록 초기화 하는 함수
 *
 * @param [in] pNodeCmpFunc : List 에 담은 ListNode 의
 *                            Data 를 비교할 사용자 함수
 * @param [in] dataType : List 에 담을 Data 의 Type
 *
 * @retval : LIST_ERR_LIST_ALLOC(-1) : List 의 메모리 할당에 실패일 때,
 *           LIST_ERR_ARGUMENT(-8) : 잘못된 Data Type 일 때,
 *           초기화 된 List : 성공
 *
 * @bug
 *  pNodeCmpFunc 는 사용자가 작성한 Data 비교 함수를 사용해야 함
 *
 * @code
 *  ListT *pList = NULL;
 *
 *  pList = nnListInit(dataCmpFunc, TEST_TYPE);
 * @endcode
 *
 * @test
 *  dataType 의 값을 사용자 타입의 범위를 벗어난 값으로 입력
 */

ListT *nnListInit(listCmpFuncT pNodeCmpFunc, Uint32T dataType)
{
    ListT *pNewList = NULL;

    if((dataType >= MEM_MAX_USER_TYPE && dataType < MEM_SYS_START)
       || dataType >= MEM_MAX_SYS_TYPE)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Wrong List DataType\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (ListT *)LIST_ERR_ARGUMENT;
    }

    pNewList = (ListT *)NNMALLOC(LIST_HEAD, sizeof(ListT));

    if((Int32T)pNewList <= 0 && (Int32T)pNewList >= MEMMGR_ERR_ARGUMENT)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "NewList Memory Allocate : Failure(%d)\n",
                       (Int32T)pNewList);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (ListT *)LIST_ERR_LIST_ALLOC;
    }

    pNewList->pHead = NULL;
    pNewList->pTail = NULL;
    pNewList->count = 0;
    pNewList->dataType = dataType;

    pNewList->dataCmpFunc = pNodeCmpFunc;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Init NewList : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return pNewList;
}


/**
 * Description: List 에 저장된 Data 삭제 및 메모리를 해제하는 함수
 *
 * @param [in/out] pList : 모든 Data 및 메모리를 해제 할 List
 *
 * @retval : None
 *
 * @code
 *  nnListFree(pList);
 * @endcode
 *
 * @test
 *  pList 의 값을 NULL 로 입력
 */

void nnListFree(ListT *pList)
{
    if(pList == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "List Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    nnListDeleteAllNode(pList);

    NNFREE(LIST_HEAD, pList);
    pList = NULL;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Free List Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}


/**
 * Description: 파라미터로 받은 pData 와 ListNode 의 Data 를 비교하여
 *              같은 주소의 Data 인 경우 해당 ListNode 와
 *              그 Data 그리고 그 하위인 ListSubNode 를 삭제하는 함수
 *
 * @param [in] pList : ListNode 를 삭제할 List
 * @param [in] pData : 삭제할 Data
 *
 * @retval : None
 *
 * @bug
 *  pData 와 같은 값이 없을 경우, pData 의 메모리는 사용자가 관리하여야 함
 *
 * @code
 *  testT *pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  nnListDeleteNode(pList, pTest);
 * @endcode
 *
 * @test
 *  pList 의 값을 NULL 로 입력,
 *  pData 의 값을 NULL 로 입력,
 *  pList 에 아무런 값도 없는 상태에서 함수 수행,
 *  pList 에 없는 값을 pData 의 값으로 입력
 */

void nnListDeleteNode(ListT *pList, void *pData)
{
    if(pList == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "List Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }
    else if(pData == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    ListNodeT *pTempNode = NULL;

    if(pList->count == 1 && pList->pHead->pData == pData)
    {
        pTempNode = pList->pHead;

        /* Delete All ListSub */
        if (pTempNode->pSubHead != NULL)
        {
            nnListDeleteAllSubNode(pTempNode);

            NNFREE(LIST_SUB_HEAD, pTempNode->pSubHead);

            pTempNode->pSubHead = NULL;
        }

        NNFREE(pList->dataType, pTempNode->pData);
        NNFREE(LIST_NODE, pTempNode);

        pList->pHead = NULL;
        pList->pTail = NULL;
        --pList->count;

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Delete Node : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    for(pTempNode = pList->pHead; pTempNode != NULL;
        pTempNode = pTempNode->pNext)
    {
        if(pTempNode->pData == pData)
        {
            if(pTempNode->pPrev == NULL)
            {
                pList->pHead = pTempNode->pNext;

                if(pList->pHead != NULL)
                {
                    pList->pHead->pPrev = NULL;
                }
            }
            else
            {
                pTempNode->pPrev->pNext = pTempNode->pNext;
            }

            if(pTempNode->pNext == NULL)
            {
                pList->pTail = pTempNode->pPrev;

                if(pList->pTail != NULL)
                {
                    pList->pTail->pNext = NULL;
                }
            }
            else
            {
                pTempNode->pNext->pPrev = pTempNode->pPrev;
            }

            /* Delete All ListSub */
            if (pTempNode->pSubHead != NULL)
            {
                nnListDeleteAllSubNode(pTempNode);

                NNFREE(LIST_SUB_HEAD, pTempNode->pSubHead);

                pTempNode->pSubHead = NULL;
            }

            NNFREE(pList->dataType, pTempNode->pData);
            NNFREE(LIST_NODE, pTempNode);

            --pList->count;

            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_INFO, "Delete Node : Success\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return;
        }
    }

    /* Can't Find Data */
    if(pTempNode == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Delete Node : Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
    }
}


/**
 * Description: List 에 저장된 모든 ListNode 및 ListSubNode 를 삭제하는 함수
 *
 * @param [in] pList : 모든 ListNode 및 ListSubNode 를 삭제할 List
 *
 * @retval : None
 *
 * @code
 *  nnListDeleteAllNode(pList);
 * @endcode
 *
 * @test
 *  pList 의 값을 NULL 로 입력,
 *  pList 에 아무런 값도 없는 상태에서 함수 수행
 */

void nnListDeleteAllNode(ListT *pList)
{
    if(pList == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "List Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    if(pList->pHead == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "No Head Data\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    ListNodeT *pTempNode = NULL;

    /* Delete All ListNode */
    for(pTempNode = pList->pHead; pTempNode != NULL;
        pTempNode = pList->pHead)
    {
        if (pTempNode->pSubHead != NULL)
        {
            /* Delete All ListSub */
            nnListDeleteAllSubNode(pTempNode);

            NNFREE(LIST_SUB_HEAD, pTempNode->pSubHead);

            pTempNode->pSubHead = NULL;
        }

        NNFREE(pList->dataType, pTempNode->pData);

        pList->pHead = pTempNode->pNext;

        NNFREE(LIST_NODE, pTempNode);

        --pList->count;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Delete All Node Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}


/**
 * Description: 파라미터로 받은 pData 와 ListNode 의 Data 를 비교하여
 *              같은 주소의 Data 인 경우 해당 ListNode 와
 *              그 하위인 ListSubNode 를 삭제하며
 *              ListNode 의 Data 는 삭제하지 않는  함수
 *
 * @param [in] pList : ListNode 를 삭제할 List
 * @param [in] pData : 삭제할 ListNode 를 찾을 Data
 *
 * @retval : None
 *
 * @bug
 *  pData 의 메모리는 사용자가 관리하여야 함
 *
 * @code
 *  testT *pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  nnListRemoveNode(pList, pTest);
 * @endcode
 *
 * @test
 *  pList 의 값을 NULL 로 입력,
 *  pData 의 값을 NULL 로 입력,
 *  pList 에 아무런 값도 없는 상태에서 함수 수행,
 *  pList 에 없는 값을 pData 의 값으로 입력
 */

void nnListRemoveNode(ListT *pList, void *pData)
{
    if(pList == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "List Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }
    else if(pData == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    ListNodeT *pTempNode = NULL;

    if(pList->count == 1 && pList->pHead->pData == pData)
    {
        pTempNode = pList->pHead;

        /* Delete All ListSub */
        if (pTempNode->pSubHead != NULL)
        {
            nnListDeleteAllSubNode(pTempNode);

            NNFREE(LIST_SUB_HEAD, pTempNode->pSubHead);

            pTempNode->pSubHead = NULL;
        }

//      Data 는 삭제하지 않는다
//      NNFREE(pList->dataType, pTempNode->pData);
        NNFREE(LIST_NODE, pTempNode);

        pList->pHead = NULL;
        pList->pTail = NULL;
        --pList->count;

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Delete Node : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    for(pTempNode = pList->pHead; pTempNode != NULL;
        pTempNode = pTempNode->pNext)
    {
        if(pTempNode->pData == pData)
        {
            if(pTempNode->pPrev == NULL)
            {
                pList->pHead = pTempNode->pNext;

                if(pList->pHead != NULL)
                {
                    pList->pHead->pPrev = NULL;
                }
            }
            else
            {
                pTempNode->pPrev->pNext = pTempNode->pNext;
            }

            if(pTempNode->pNext == NULL)
            {
                pList->pTail = pTempNode->pPrev;

                if(pList->pTail != NULL)
                {
                    pList->pTail->pNext = NULL;
                }
            }
            else
            {
                pTempNode->pNext->pPrev = pTempNode->pPrev;
            }

            /* Delete All ListSub */
            if (pTempNode->pSubHead != NULL)
            {
                nnListDeleteAllSubNode(pTempNode);

                NNFREE(LIST_SUB_HEAD, pTempNode->pSubHead);

                pTempNode->pSubHead = NULL;
            }

//          Data 는 삭제하지 않는다
//          NNFREE(pList->dataType, pTempNode->pData);
            NNFREE(LIST_NODE, pTempNode);

            --pList->count;

            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_INFO, "Delete Node : Success\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return;
        }
    }

    /* Can't Find Data */
    if(pTempNode == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Delete Node : Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
    }
}


/**
 * Description: List 에 저장된 모든 ListNode 및 ListSubNode 를 삭제하며
 *              ListNode 의 Data 는 삭제하지 않는 함수
 *
 * @param [in] pList : 모든 ListNode 및 ListSubNode 를 삭제할 List
 *
 * @retval : None
 *
 * @code
 *  nnListRemoveAllNode(pList);
 * @endcode
 *
 * @test
 *  pList 의 값을 NULL 로 입력,
 *  pList 에 아무런 값도 없는 상태에서 함수 수행
 */

void nnListRemoveAllNode(ListT *pList)
{
    if(pList == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "List Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    if(pList->pHead == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "No Head Data\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    ListNodeT *pTempNode = NULL;

    /* Delete All ListNode */
    for(pTempNode = pList->pHead; pTempNode != NULL;
        pTempNode = pList->pHead)
    {
        if (pTempNode->pSubHead != NULL)
        {
            /* Delete All ListSub */
            nnListDeleteAllSubNode(pTempNode);

            NNFREE(LIST_SUB_HEAD, pTempNode->pSubHead);

            pTempNode->pSubHead = NULL;
        }

//      Data 는 삭제하지 않는다
//      NNFREE(pList->dataType, pTempNode->pData);

        pList->pHead = pTempNode->pNext;

        NNFREE(LIST_NODE, pTempNode);

        --pList->count;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Delete All Node Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}







/**
 * Description: List 의 첫 번째 ListNode 를 리턴하는 함수
 *
 * @param [in] pList : 첫 번째 ListNode 를 가져올 List
 *
 * @retval : LIST_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           NULL : ListNode 가 없을 때,
 *           첫 번째 ListNode : 성공
 *
 * @code
 *  ListNodeT *pRet = NULL;
 *
 *  pRet = nnListGetHeadNode(pList);
 * @endcode
 *
 * @test
 *  pList 의 값을 NULL 로 입력,
 *  pList 에 아무런 값도 없는 상태에서 함수 수행
 */

ListNodeT *nnListGetHeadNode(ListT *pList)
{
    if(pList == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "List Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (ListNodeT *) LIST_ERR_NULL;
    }

    if(pList->pHead == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "List Is Empty\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return NULL;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Get Head Node : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return pList->pHead;
}


/**
 * Description: List 의 마지막 ListNode 를 리턴하는 함수
 *
 * @param [in] pList : 마지막 ListNode 를 가져올 List
 *
 * @retval : LIST_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           NULL : ListNode 가 없을 때,
 *           마지막 ListNode : 성공
 *
 * @code
 *  ListNodeT *pRet = NULL;
 *
 *  pRet = nnListGetTailNode(pList);
 * @endcode
 *
 * @test
 *  pList 의 값을 NULL 로 입력,
 *  pList 에 아무런 값도 없는 상태에서 함수 수행
 */

ListNodeT *nnListGetTailNode(ListT *pList)
{
    if(pList == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "List Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (ListNodeT *) LIST_ERR_NULL;
    }

    if(pList->pHead == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "List Is Empty\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return NULL;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Get Tail Node : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return pList->pTail;
}


/**
 * Description: 사용자가 등록한 ListNode 의 Data 비교 함수를 사용하여
 *              List 에서 파라미터로 받은 pData 와 같은 Data 를
 *              가진 ListNode 를 검색해 리턴하는 함수
 *
 * @param [in] pList : Data 를 검색할 List
 * @param [in] pData : 검색하려는 Data
 *
 * @retval : LIST_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           LIST_ERR_COMP_FUNC(-5) : 비교 함수가 등록되지 않았을 때,
 *           NULL : pData 와 같은 Data 를 갖는 ListNode 가 없을 때,
 *           검색 된 ListNode : 성공
 *
 * @code
 *  ListNodeT *pRet = NULL;
 *  testT *pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  pRet = nnListSearchNode(pList, pTest);
 * @endcode
 *
 * @test
 *  pList 의 값을 NULL 로 입력,
 *  pList 의 사용자 비교함수를 NULL 인 상태에서 함수 수행,
 *  pData 의 값을 NULL 로 입력,
 *  pList 에 아무런 값도 없는 상태에서 함수 수행,
 *  pList 에 없는 값을 pData 의 값으로 입력
 */

ListNodeT *nnListSearchNode(ListT *pList, void *pData)
{
    if(pList == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "List Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (ListNodeT *) LIST_ERR_NULL;
    }
    else if(pData == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (ListNodeT *) LIST_ERR_NULL;
    }
    else if(pList->dataCmpFunc == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Comp Function Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (ListNodeT *) LIST_ERR_COMP_FUNC;
    }

    ListNodeT * pTempNode = NULL;

    for(pTempNode = pList->pHead; pTempNode != NULL;
        pTempNode = pTempNode->pNext)
    {
        /* Same Data */
        if(pList->dataCmpFunc(pTempNode->pData, pData) == -1)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_INFO, "Search Node : Success\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return pTempNode;
        }
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_ERR, "Search Node : Failure\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return NULL;
}


/**
 * Description: List 에 저장된 ListNode 의 개수를 가져오는 함수
 *
 * @param [in] pList : 저장된 ListNode 개수를 가져올 List
 *
 * @retval : LIST_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           저장 된 ListNode 의 개수 : 성공
 *
 * @code
 *  Int64T ret = 0;
 *
 *  ret = nnListCount(pList);
 * @endcode
 *
 * @test
 *  pList 의 값을 NULL 로 입력,
 *  pList 에 아무런 값도 없는 상태에서 함수 수행
 */

Int64T nnListCount(ListT *pList)
{
    if(pList == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "List Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Get List Count : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return pList->count;
}


/**
 * Description: Data 를 ListNode 에 담아 List 에 저장하는 함수
 *
 * @param [in] pList : Data 를 저장할 List
 * @param [in] pData : 저장하려는 Data
 *
 * @retval : LIST_ERR_LISTNODE_ALLOC(-2) : ListNode 의
 *                                         메모리 할당에 실패 일 때,
 *           LIST_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  저장 실패 시 pData 의 메모리는 사용자가 관리하여야 함
 *
 * @code
 *  Int8T ret = 0;
 *  testT *pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  ret = nnListAddNode(pList, pTest);
 * @endcode
 *
 * @test
 *  pList 의 값을 NULL 로 입력,
 *  pData 의 값을 NULL 로 입력
 */

Int8T nnListAddNode(ListT *pList, void *pData)
{
    if(pList == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "List Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }
    else if(pData == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }

    ListNodeT *pNewNode = NULL;

    pNewNode = (ListNodeT *)NNMALLOC(LIST_NODE, sizeof(ListNodeT));

    if((Int32T)pNewNode <= 0 && (Int32T)pNewNode >= MEMMGR_ERR_ARGUMENT)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "NewListNode Memory Allocate : Failure(%d)\n",
                       (Int32T)pNewNode);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_LISTNODE_ALLOC;
    }

    pNewNode->pPrev = NULL;
    pNewNode->pNext = NULL;
    pNewNode->pData = pData;

    pNewNode->pSubHead = NULL;

    /* No Node */
    if(pList->count == 0)
    {
        pList->pHead = pNewNode;
        pList->pTail = pNewNode;
    }
    else
    {
        pList->pTail->pNext = pNewNode;

        pNewNode->pPrev = pList->pTail;

        pList->pTail = pNewNode;
    }

    ++pList->count;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Add Node : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: Data 를 ListNode 에 담아 List 의
 *              특정 ListNode 앞에 저장하는 함수
 *
 * @param [in] pList : Data 를 저장할 List
 * @param [in] pNode : Data 를 담은 ListNode 의 다음에 올 특정 ListNode
 * @param [in] pData : 저장하려는 Data
 *
 * @retval : LIST_ERR_LISTNODE_ALLOC(-2) : ListNode 의
 *                                         메모리 할당에 실패 일 때,
 *           LIST_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  pNode 를 찾을 수 없을경우 List 의 마지막에 추가 되며,
 *  저장 실패 시 pData 의 메모리는 사용자가 관리하여야 함
 *
 * @code
 *  Int8T ret = 0;
 *  testT *pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  ret = nnListAddNodePrev(pList, pNode, pTest);
 * @endcode
 *
 * @test
 *  pList 의 값을 NULL 로 입력,
 *  pData 의 값을 NULL 로 입력,
 *  pNode 의 값을 NULL 로 입력
 */

Int8T nnListAddNodePrev(ListT *pList, ListNodeT *pNode, void *pData)
{
    Int8T ret = 0;

    if(pList == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "List Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }
    else if(pData == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }

    if(pNode == NULL)
    {
        ret = nnListAddNode(pList, pData);
        if(ret != SUCCESS)
        {
            return ret;
        }

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Add Data To End Of List\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return SUCCESS;
    }

    ListNodeT *pNewNode = NULL;

    pNewNode = (ListNodeT *)NNMALLOC(LIST_NODE, sizeof(ListNodeT));

    if((Int32T)pNewNode <= 0 && (Int32T)pNewNode >= MEMMGR_ERR_ARGUMENT)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "NewNode Memory Allocate : Failure(%d)\n",
                       (Int32T)pNewNode);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_LISTNODE_ALLOC;
    }

    pNewNode->pPrev = NULL;
    pNewNode->pNext = NULL;
    pNewNode->pData = pData;

    ListNodeT *pTempNode = NULL;

    for(pTempNode = pList->pHead; pTempNode != NULL;
        pTempNode = pTempNode->pNext)
    {
        if(pTempNode == pNode)
        {
            if(pNode->pPrev == NULL)
            {
                pList->pHead = pNewNode;
            }
            else
            {
                pNode->pPrev->pNext = pNewNode;
            }

            pNewNode->pPrev = pNode->pPrev;
            pNewNode->pNext = pNode;

            pNode->pPrev = pNewNode;

            ++pList->count;

            return SUCCESS;
        }
    }

    ret = nnListAddNode(pList, pData);
    if(ret != SUCCESS)
    {
        return ret;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Add Data To End Of List\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: Data 를 ListNode 에 담아 List 의
 *              특정 ListNode 뒤에 저장하는 함수
 *
 * @param [in] pList : Data 를 저장할 List
 * @param [in] pNode : Data 를 담은 ListNode 의 이전에 올 특정 ListNode
 * @param [in] pData : 저장하려는 Data
 *
 * @retval : LIST_ERR_LISTNODE_ALLOC(-2) : ListNode 의
 *                                         메모리 할당에 실패 일 때,
 *           LIST_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  pNode 를 찾을 수 없을경우 List 의 마지막에 추가 되며,
 *  저장 실패 시 pData 의 메모리는 사용자가 관리하여야 함
 *
 * @code
 *  Int8T ret = 0;
 *  testT *pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  ret = nnListAddNodeNext(pList, pNode, pTest);
 * @endcode
 *
 * @test
 *  pList 의 값을 NULL 로 입력,
 *  pData 의 값을 NULL 로 입력,
 *  pNode 의 값을 NULL 로 입력
 */

Int8T nnListAddNodeNext(ListT *pList, ListNodeT *pNode, void *pData)
{
    Int8T ret = 0;

    if(pList == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "List Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }
    else if(pData == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }

    if(pNode == NULL)
    {
        ret = nnListAddNode(pList, pData);
        if(ret != SUCCESS)
        {
            return ret;
        }

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Add Data To End Of List\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return SUCCESS;
    }

    ListNodeT *pNewNode = NULL;

    pNewNode = (ListNodeT *)NNMALLOC(LIST_NODE, sizeof(ListNodeT));

    if((Int32T)pNewNode <= 0 && (Int32T)pNewNode >= MEMMGR_ERR_ARGUMENT)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "NewListNode Memory Allocate : Failure(%d)\n",
                       (Int32T)pNewNode);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_LISTNODE_ALLOC;
    }

    pNewNode->pPrev = NULL;
    pNewNode->pNext = NULL;
    pNewNode->pData = pData;

    ListNodeT *pTempNode = NULL;

    for(pTempNode = pList->pHead; pTempNode != NULL;
        pTempNode = pTempNode->pNext)
    {
        if(pTempNode == pNode)
        {
            if(pNode->pNext == NULL)
            {
                pList->pTail = pNewNode;
            }
            else
            {
                pNode->pNext->pPrev = pNewNode;
            }

            pNewNode->pPrev = pNode;
            pNewNode->pNext = pNode->pNext;

            pNode->pNext = pNewNode;

            ++pList->count;

            return SUCCESS;
        }
    }

    ret = nnListAddNode(pList, pData);
    if(ret != SUCCESS)
    {
        return ret;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Add Data To End Of List\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: Data 를 ListNode 에 담아
 *              List 의 첫 번째 Node 로 저장하는 함수
 *
 * @param [in] pList : Data 를 저장할 List
 * @param [in] pData : 저장하려는 Data
 *
 * @retval : IST_ERR_LISTNODE_ALLOC(-2) : ListNode 의
 *                                        메모리 할당에 실패 일 때,
 *           LIST_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  저장 실패 시 pData 의 메모리는 사용자가 관리하여야 함
 *
 * @code
 *  Int8T ret = 0;
 *  testT *pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  ret = nnListAddNodeHead(pList, pTest);
 * @endcode
 *
 * @test
 *  pList 의 값을 NULL 로 입력,
 *  pData 의 값을 NULL 로 입력
 */

Int8T nnListAddNodeHead(ListT *pList, void *pData)
{
    ListNodeT *pTempHead = NULL;

    if(pList == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "List Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }
    else if(pData == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }

    ListNodeT *pNewNode = NULL;

    pNewNode = (ListNodeT *)NNMALLOC(LIST_NODE, sizeof(ListNodeT));

    if((Int32T)pNewNode <= 0 && (Int32T)pNewNode >= MEMMGR_ERR_ARGUMENT)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "NewListNode Memory Allocate : Failure(%d)\n",
                       (Int32T)pNewNode);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_LISTNODE_ALLOC;
    }

    pNewNode->pPrev = NULL;
    pNewNode->pNext = NULL;
    pNewNode->pData = pData;

    pNewNode->pSubHead = NULL;

    /* No Node */
    if(pList->count == 0)
    {
        pList->pHead = pNewNode;
        pList->pTail = pNewNode;
    }
    else
    {
        pTempHead = pList->pHead;

        pList->pHead = pNewNode;

        pNewNode->pNext = pTempHead;

        pTempHead->pPrev = pNewNode;
    }

    ++pList->count;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Add Node : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: 사용자가 등록한 ListNode 의 Data 비교 함수를 사용하여
 *              List 에 파라미터로 받은 pData 와 같은 Data 를 가진
 *              ListNode 가 있는지 확인하여 없을 경우에만 저장하는 함수
 *
 * @param [in] pList : Data 를 저장할 List
 * @param [in] pData : 저장하려는 Data
 *
 * @retval : LIST_ERR_LISTNODE_ALLOC(-2) : ListNode 의
 *                                         메모리 할당에 실패 일 때,
 *           LIST_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           LIST_ERR_DUP(-4) : 같은 Data 를 가진 ListNode 가 있을 때,
 *           LIST_ERR_COMP_FUNC(-5) : 비교 함수가 등록되지 않았을 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  저장 실패 시 pData 의 메모리는 사용자가 관리하여야 함
 *
 * @code
 *  Int8T ret = 0;
 *  testT *pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  ret = nnListAddNodeDupNot(pList, pTest);
 * @endcode
 *
 * @test
 *  pList 의 값을 NULL 로 입력,
 *  pList 의 사용자 비교함수를 NULL 인 상태에서 함수 수행,
 *  pData 의 값을 NULL 로 입력
 */

Int8T nnListAddNodeDupNot(ListT *pList, void *pData)
{
    int ret = 0;

    if(pList == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "List Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        return LIST_ERR_NULL;

    }
    else if(pData == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }

    ret = nnListCheckDupData(pList, pData);

    if (ret < 0) {
        return ret;
    }

    ret = nnListAddNode(pList, pData);
    if(ret != SUCCESS)
    {
        return ret;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Add Node : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: 사용자가 등록한 ListNode 의 Data 비교 함수를 사용하여
 *              List 에 파라미터로 받은 pData 와 같은 Data 를 가진
 *              ListNode 가 있는지 확인하는 함수
 *
 * @param [in] pList : 중복 Data 가 있는지 확인할 List
 * @param [in] pData : 검색하려는 Data
 *
 * @retval : LIST_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           LIST_ERR_DUP(-4) : 같은 Data 를 가진 ListNode 가 있을 때,
 *           LIST_ERR_COMP_FUNC(-5) : 비교 함수가 등록되지 않았을 때,
 *           SUCCESS(0) : 성공
 *
 * @code
 *  Int8T ret = 0;
 *  testT *pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  ret = nnListCheckDupData(pList, pTest);
 * @endcode
 *
 * @test
 *  pList 의 값을 NULL 로 입력,
 *  pList 의 사용자 비교함수를 NULL 인 상태에서 함수 수행,
 *  pData 의 값을 NULL 로 입력
 */

Int8T nnListCheckDupData(ListT *pList, void *pData)
{
    if(pList == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "List Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }
    else if(pData == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        return LIST_ERR_NULL;
    }
    else if(pList->dataCmpFunc == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Compare Function Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        return LIST_ERR_COMP_FUNC;
    }

    if(pList->dataCmpFunc != NULL)
    {
        ListNodeT *pTempNode = NULL;

        for(pTempNode = pList->pHead; pTempNode != NULL;
            pTempNode = pTempNode->pNext)
        {
            if(pList->dataCmpFunc(pTempNode->pData, pData) == -1)
            {
                NNLOGDEBUG(LOG_DEBUG, "################################\n");
                NNLOGDEBUG(LOG_ERR, "Duplication Data Found\n");
                NNLOGDEBUG(LOG_DEBUG, "################################\n");

                return LIST_ERR_DUP;
            }
        }
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Duplication Data Not Found\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: Data 를 ListNode 에 담아 List 에 저장한 다음
 *              사용자가 등록한 ListNode 의 Data 비교 함수를 사용하여
 *              List 에 저장된 모든 Data 를 비교하여 정렬하는 함수
 *
 * @param [in] pList : Data 를 저장하고 정렬할  List
 * @param [in] pData : 저장하려는 Data
 *
 * @retval : LIST_ERR_LISTNODE_ALLOC(-2) : ListNode 의
 *                                         메모리 할당에 실패 일 때,
 *           LIST_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  사용자 등록 함수가 없을 경우 정렬은 수행하지 않음 
 *
 * @code
 *  Int8T ret = 0;
 *  testT *pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  ret = nnListAddNodeSort(pList, pTest);
 * @endcode
 *
 * @test
 *  pList 의 값을 NULL 로 입력,
 *  pList 의 사용자 비교함수를 NULL 인 상태에서 함수 수행,
 *  pData 의 값을 NULL 로 입력
 */

Int8T nnListAddNodeSort(ListT *pList, void *pData) {
    Int8T ret = 0;

    if(pList == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "List Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }
    else if(pData == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }

    ListNodeT *pTempIndexNode = NULL;
    ListNodeT *pTempFixNode = NULL;
    ListNodeT *pTempMoveNode = NULL;

    ListNodeT *pTempPrevNode = NULL;

    BoolT swap = FALSE;

    // Add Data to End of List
    ret = nnListAddNode(pList, pData);
    if(ret != SUCCESS)
    {
        return ret;
    }

    if(pList->dataCmpFunc != NULL)
    {
        // Sort
        for(pTempFixNode = pList->pHead->pNext; pTempFixNode != NULL;
            pTempFixNode = pTempIndexNode)
        {
            if(pTempFixNode != NULL)
            {
                pTempIndexNode = pTempFixNode->pNext;

                for(pTempMoveNode = pTempFixNode->pPrev;
                                    pTempMoveNode != NULL;
                                    pTempMoveNode = pTempMoveNode->pPrev)
                {
                    // Left Data is Big -> ASC(0), DESC(1)
                    if(pList->dataCmpFunc(
                                pTempMoveNode->pData,
                                pTempFixNode->pData) == 0) {
                        if(pList->pHead == pTempMoveNode)
                        {
                            pList->pHead = pTempFixNode;
                        }

                        if(pList->pTail == pTempFixNode)
                        {
                            pList->pTail = pTempMoveNode;
                        }

                        pTempPrevNode = pTempMoveNode->pPrev;

                        pTempMoveNode->pPrev = pTempFixNode;
                        pTempMoveNode->pNext = pTempFixNode->pNext;

                        if(pTempFixNode->pNext != NULL)
                        {
                            pTempFixNode->pNext->pPrev = pTempMoveNode;
                        }

                        pTempFixNode->pPrev = pTempPrevNode;
                        pTempFixNode->pNext = pTempMoveNode;

                        if(pTempPrevNode != NULL)
                        {
                            pTempPrevNode->pNext = pTempFixNode;
                        }

                        pTempMoveNode = pTempMoveNode->pPrev;

                        if(!swap)
                        {
                            pTempIndexNode = pTempMoveNode->pNext;
                            swap = TRUE;
                        }
                    }
                }
            }
        }
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Sort Node : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: List 의 ListNode 에 담은 Data 를 비교할
 *              사용자 함수를 등록하는 함수
 *
 * @param [in] pList : 사용자 함수를 등록할 List
 * @param [in] pNodeCmpFunc : 등록할 사용자 함수
 *
 * @retval : LIST_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @code
 *  Int8T ret = 0;
 *
 *  ret = nnListSetNodeCompFunc(pList, dataNodeCmpFunc);
 * @endcode
 *
 * @test
 *  pList 의 값을 NULL 로 입력,
 *  pNodeCmpFunc 의 값을 NULL 로 입력
 */

Int8T nnListSetNodeCompFunc(ListT *pList, listCmpFuncT pNodeCmpFunc)
{
    if(pList == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "List Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }

    pList->dataCmpFunc = pNodeCmpFunc;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Set Node Compare Function : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: ListNode 에 하위 Node 인 ListSubNode 를 저장할 수 있는
 *              ListSub 를 사용할 수 있도록 초기화 하는 함수
 *
 * @param [in] pListNode : ListSub 를 초기화할 ListNode
 * @param [in] pSubNodeCmpFunc : ListSub 에 담은 ListSubNode 의
 *                               Data 를 비교할 사용자 함수
 * @param [in] dataType : ListSub 에 담을 Data 의 Type
 *
 * @retval : LIST_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           LIST_ERR_LISTSUB_ALLOC(-6) : ListSub 의
 *                                        메모리 할당에 실패일 때,
 *           LIST_ERR_ARGUMENT(-8) : 잘못된 Data Type 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  pSubNodeCmpFunc 는 사용자가 작성한 Data 비교 함수를 사용해야 함
 *
 * @code
 *  Int8T ret = 0;
 *
 *  ret = nnListSubInit(pListNode, subNodeCmpFunc, dataType);
 * @endcode
 *
 * @test
 *  pListNode 의 값을 NULL 로 입력,
 *  pSubNodeCmpFunc 의 값을 NULL 로 입력,
 *  dataType 의 값을 사용자 타입의 범위를 벗어난 값으로 입력
 */

Int8T nnListSubInit(ListNodeT *pListNode,
    listCmpFuncT pSubNodeCmpFunc,
    Uint32T dataType)
{
    ListSubT *pNewListSub = NULL;

    if(pListNode == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "ListNode Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }

    if((dataType >= MEM_MAX_USER_TYPE && dataType < MEM_SYS_START)
       || dataType >= MEM_MAX_SYS_TYPE)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Wrong ListSub DataType\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_ARGUMENT;
    }

    pNewListSub = (ListSubT *)NNMALLOC(LIST_SUB_HEAD, sizeof(ListSubT));

    if((Int32T)pNewListSub <= 0 && (Int32T)pNewListSub >= MEMMGR_ERR_ARGUMENT)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "NewListSub Memory Allocate : Failure(%d)\n",
                       (Int32T)pNewListSub);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_LISTSUB_ALLOC;
    }

    pNewListSub->pHead = NULL;
    pNewListSub->pTail = NULL;
    pNewListSub->count = 0;
    pNewListSub->dataType = dataType;

    pNewListSub->dataCmpFunc = pSubNodeCmpFunc;

    pListNode->pSubHead = pNewListSub;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Init NewSubList : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: 파라미터로 받은 pData 와 ListSubNode 의 Data 를 비교하여
 *              같은 주소의 Data 인 경우 해당 ListSubNode 를 삭제하는 함수
 *
 * @param [in] pListNode : ListSubNode 를 삭제할 ListNode
 * @param [in] pData : 삭제할 Data
 *
 * @retval : None
 *
 * @bug
 *  pData 의 메모리는 사용자가 관리하여야 함
 *
 * @code
 *  testT *pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  nnListDeleteSubNode(pListNode, pTest);
 * @endcode
 *
 * @test
 *  pListNode 의 값을 NULL 로 입력,
 *  SubList 를 초기화하지 않은 상태에서 수행,
 *  pData 의 값을 NULL 로 입력,
 *  ListSub 에 아무런 값도 없는 상태에서 함수 수행,
 *  ListSub 에 없는 값을 pData 의 값으로 입력
 */

void nnListDeleteSubNode(ListNodeT *pListNode, void *pData)
{
    ListSubT *pTempSub = NULL;
    ListSubNodeT *pTempSubNode = NULL;

    if(pListNode == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "ListNode Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }
    else if(pListNode->pSubHead == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "SubList Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }
    else if(pData == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    pTempSub = pListNode->pSubHead;

    if(pTempSub->count == 1 && pTempSub->pHead->pData == pData)
    {
        NNFREE(pTempSub->dataType, pTempSub->pHead->pData);
        NNFREE(LIST_NODE, pTempSub->pHead);

        pTempSub->pHead = NULL;
        pTempSub->pTail = NULL;
        --pTempSub->count;

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Delete SubNode : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    for(pTempSubNode = pTempSub->pHead; pTempSubNode != NULL;
        pTempSubNode = pTempSubNode->pNext)
    {
        if(pTempSubNode->pData == pData)
        {
            if(pTempSubNode->pPrev == NULL)
            {
                pTempSub->pHead = pTempSubNode->pNext;

                if(pTempSub->pHead != NULL)
                {
                    pTempSub->pHead->pPrev = NULL;
                }
            }
            else
            {
                pTempSubNode->pPrev->pNext = pTempSubNode->pNext;
            }

            if(pTempSubNode->pNext == NULL)
            {
                pTempSub->pTail = pTempSubNode->pPrev;

                if(pTempSub->pTail != NULL)
                {
                    pTempSub->pTail->pNext = NULL;
                }
            }
            else
            {
                pTempSubNode->pNext->pPrev = pTempSubNode->pPrev;
            }

            NNFREE(pTempSub->dataType, pTempSubNode->pData);
            NNFREE(LIST_SUB_NODE, pTempSubNode);

            --pTempSub->count;

            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_INFO, "Delete SubNode : Success\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return;
        }
    }

    /* Can't Find Data */
    if(pTempSubNode == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Delete SubNode : Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
    }
}


/**
 * Description: ListNode 에 저장된 모든 ListSubNode 를 삭제하는 함수
 *
 * @param [in] pList : 모든 ListSubNode 를 삭제할 ListNode
 *
 * @retval : None
 *
 * @code
 *  nnListDeleteAllSubNode(pListNode);
 * @endcode
 *
 * @test
 *  pListNode 의 값을 NULL 로 입력,
 *  SubList 를 초기화하지 않은 상태에서 수행,
 *  ListSub 에 아무런 값도 없는 상태에서 함수 수행
 */

void nnListDeleteAllSubNode(ListNodeT *pListNode)
{
    ListSubT *pTempSub = NULL;
    ListSubNodeT *pTempSubNode = NULL;

    if(pListNode == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "ListNode Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }
    else if(pListNode->pSubHead == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "SubList Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    pTempSub = pListNode->pSubHead;

    /* Delete All ListSubNode */
    for(pTempSubNode = pTempSub->pHead; pTempSubNode != NULL;
        pTempSubNode = pTempSub->pHead)
    {
        NNFREE(pTempSub->dataType, pTempSubNode->pData);

        pTempSub->pHead = pTempSubNode->pNext;

        NNFREE(LIST_SUB_NODE, pTempSubNode);

        --pTempSub->count;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Delete All SubNode Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}


/**
 * Description: Data 를 ListSubNode 에 담아 ListNode 에 저장하는 함수
 *
 * @param [in] pListNode : Data 를 저장할 ListNode
 * @param [in] pData : 저장하려는 Data
 *
 * @retval : LIST_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           LIST_ERR_LISTSUBNODE_ALLOC(-7) : ListSubNode 의
 *                                            메모리 할당에 실패 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  저장 실패 시 pData 의 메모리는 사용자가 관리하여야 함
 *
 * @code
 *  Int8T ret = 0;
 *  testT *pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  ret = nnListAddSubNode(pListNode, pTest);
 * @endcode
 *
 * @test
 *  pListNode 의 값을 NULL 로 입력,
 *  SubList 를 초기화하지 않은 상태에서 수행,
 *  pData 의 값을 NULL 로 입력
 */

Int8T nnListAddSubNode(ListNodeT *pListNode, void *pData)
{
    ListSubT *pListSub = NULL;

    /* Add ListSubNode First Time */
    if(pListNode == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "ListNode Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }
    else if(pListNode->pSubHead == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "SubList Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }
    else if(pData == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }

    pListSub = pListNode->pSubHead;

    /* Make New ListSubNode */
    ListSubNodeT *pNewSubNode = NULL;

    pNewSubNode = (ListSubNodeT *)NNMALLOC(LIST_SUB_NODE, sizeof(ListSubNodeT));

    if((Int32T)pNewSubNode <= 0 && (Int32T)pNewSubNode >= MEMMGR_ERR_ARGUMENT)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "NewListSubNode Memory Allocate : Failure(%d)\n",
                       (Int32T)pNewSubNode);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_LISTSUBNODE_ALLOC;
    }

    pNewSubNode->pPrev = NULL;
    pNewSubNode->pNext = NULL;
    pNewSubNode->pData = pData;

    /* No Node */
    if(pListSub->count == 0)
    {
        pListSub->pHead = pNewSubNode;
        pListSub->pTail = pNewSubNode;
    }
    else
    {
        pListSub->pTail->pNext = pNewSubNode;

        pNewSubNode->pPrev = pListSub->pTail;

        pListSub->pTail = pNewSubNode;
    }

    ++pListSub->count;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Add SubNode : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: Data 를 ListSubNode 에 담아
 *              ListNode 의 첫 번째 SubNode 로 저장하는 함수
 *
 * @param [in] pListNode : Data 를 저장할 ListNode
 * @param [in] pData : 저장하려는 Data
 *
 * @retval : LIST_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           LIST_ERR_LISTSUBNODE_ALLOC(-2) : ListSubNode 의
 *                                            메모리 할당에 실패 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  저장 실패 시 pData 의 메모리는 사용자가 관리하여야 함
 *
 * @code
 *  Int8T ret = 0;
 *  testT *pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  ret = nnListAddSubNodeHead(pListNode, pTest);
 * @endcode
 *
 * @test
 *  pListNode 의 값을 NULL 로 입력,
 *  SubList 를 초기화하지 않은 상태에서 수행,
 *  pData 의 값을 NULL 로 입력
 */

Int8T nnListAddSubNodeHead(ListNodeT *pListNode, void *pData)
{
    ListSubT *pListSub = NULL;
    ListSubNodeT *pTempSubHead = NULL;

    /* Add ListSubNode First Time */
    if(pListNode == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "ListNode Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }
    else if(pListNode->pSubHead == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "SubList Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }
    else if(pData == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }

    pListSub = pListNode->pSubHead;

    /* Make New ListSubNode */
    ListSubNodeT *pNewSubNode = NULL;

    pNewSubNode = (ListSubNodeT *)NNMALLOC(LIST_SUB_NODE, sizeof(ListSubNodeT));

    if((Int32T)pNewSubNode <= 0 && (Int32T)pNewSubNode >= MEMMGR_ERR_ARGUMENT)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "NewListSubNode Memory Allocate : Failure(%d)\n",
                       (Int32T)pNewSubNode);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_LISTSUBNODE_ALLOC;
    }

    pNewSubNode->pPrev = NULL;
    pNewSubNode->pNext = NULL;
    pNewSubNode->pData = pData;

    /* No Node */
    if(pListSub->count == 0)
    {
        pListSub->pHead = pNewSubNode;
        pListSub->pTail = pNewSubNode;
    }
    else
    {
        pTempSubHead = pListSub->pHead;

        pListSub->pHead = pNewSubNode;

        pNewSubNode->pNext = pTempSubHead;

	pTempSubHead->pPrev = pNewSubNode;
    }

    ++pListSub->count;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Add SubNode : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: ListNode 의 첫 번째 ListSubNode 를 리턴하는 함수
 *
 * @param [in] pListNode : 첫 번째 ListSubNode 를 가져올 ListNode
 *
 * @retval : LIST_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           NULL : ListSubNode 가 없을 때,
 *           첫 번째 ListSubNode : 성공
 *
 * @code
 *  ListSubNodeT *pRet = NULL;
 *
 *  pRet = nnListGetHeadSubNode(pListNode);
 * @endcode
 *
 * @test
 *  pListNode 의 값을 NULL 로 입력,
 *  SubList 를 초기화하지 않은 상태에서 수행,
 *  ListSub 에 아무런 값도 없는 상태에서 함수 수행
 */

ListSubNodeT *nnListGetHeadSubNode(ListNodeT *pListNode)
{
    if(pListNode == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "ListNode Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (ListSubNodeT *) LIST_ERR_NULL;
    }
    else if(pListNode->pSubHead == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "SubList Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (ListSubNodeT *) LIST_ERR_NULL;
    }
    else if(pListNode->pSubHead->pHead == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "ListSubNode is Empty\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return NULL;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Get Head SubNode : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return pListNode->pSubHead->pHead;
}


/**
 * Description: 사용자가 등록한 ListSubNode 의 Data 비교 함수를 사용하여
 *              ListNode 에서 파라미터로 받은 pData 와 같은 Data 를
 *              가진 ListSubNode 를 검색해 리턴하는 함수
 *
 * @param [in] pListNode : Data 를 검색할 ListNode
 * @param [in] pData : 검색하려는 Data
 *
 * @retval : LIST_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           LIST_ERR_COMP_FUNC(-5) : 비교 함수가 등록되지 않았을 때,
 *           NULL : pData 와 같은 Data 를 갖는 ListSubNode 가 없을 때,
 *           검색 된 ListSubNode : 성공
 *
 * @code
 *  ListSubNodeT *pRet = NULL;
 *  testT *pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  pRet = nnListSearchSubNode(pListNode, pTest);
 * @endcode
 *
 * @test
 *  pListNode 의 값을 NULL 로 입력,
 *  SubList 를 초기화하지 않은 상태에서 수행,
 *  ListSub 의 사용자 비교함수를 NULL 인 상태에서 함수 수행,
 *  pData 의 값을 NULL 로 입력,
 *  ListSub 에 아무런 값도 없는 상태에서 함수 수행,
 *  ListSub 에 없는 값을 pData 의 값으로 입력
 */

ListSubNodeT *nnListSearchSubNode(ListNodeT *pListNode, void *pData)
{
    ListSubNodeT * pTempSubNode = NULL;

    if(pListNode == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "ListNode Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (ListSubNodeT *) LIST_ERR_NULL;
    }
    else if(pListNode->pSubHead == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "SubList Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (ListSubNodeT *) LIST_ERR_NULL;
    }
    else if(pData == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (ListSubNodeT *) LIST_ERR_NULL;
    }
    else if(pListNode->pSubHead->dataCmpFunc == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Comp Function Must Not NULL\n");
	NNLOGDEBUG(LOG_INFO,
              "Need to Set Comp Funcion -> Use nnListSetSubNodeCompFunc()\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (ListSubNodeT *) LIST_ERR_COMP_FUNC;
    }

    for(pTempSubNode = pListNode->pSubHead->pHead; pTempSubNode != NULL;
        pTempSubNode = pTempSubNode->pNext)
    {
        /* Same Data */
        if(pListNode->pSubHead->dataCmpFunc(pTempSubNode->pData, pData) == -1)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_INFO, "Search Data : Success\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return pTempSubNode;
        }
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Search Data : Failure\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return NULL;
}


/**
 * Description: ListSub 에 저장된 ListSubNode 의 개수를 가져오는 함수
 *
 * @param [in] pListNode : 저장된 ListSubNode 개수를 가져올
 *                         ListSub 를 가진 ListNode
 *
 * @retval : LIST_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           저장 된 ListSubNode 의 개수 : 성공
 *
 * @code
 *  Int64T ret = 0;
 *
 *  ret = nnListSubCount(pListNode);
 * @endcode
 *
 * @test
 *  pListNode 의 값을 NULL 로 입력,
 *  SubList 를 초기화하지 않은 상태에서 수행,
 *  ListSub 에 아무런 값도 없는 상태에서 함수 수행
 */

Int64T nnListSubCount(ListNodeT *pListNode)
{
    if(pListNode == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "ListNode Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }
    else if(pListNode->pSubHead == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "SubList Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Get SubList Count : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return pListNode->pSubHead->count;
}


/**
 * Description: ListSub 의 ListSubNode 에 담은 Data 를 비교할
 *              사용자 함수를 등록하는 함수
 *
 * @param [in] pListNode : 사용자 함수를 등록할 ListNode
 * @param [in] pNodeCmpFunc : 등록할 사용자 함수
 *
 * @retval : LIST_ERR_NULL(-3) : pListNode 또는 pSubHead 가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @code
 *  Int8T ret = 0;
 *
 *  ret = nnListSetSubNodeCompFunc(pListNode, dataNodeCmpFunc);
 * @endcode
 *
 * @test
 *  pListNode 의 값을 NULL 로 입력,
 *  SubList 를 초기화하지 않은 상태에서 수행,
 *  pSubNodeCmpFunc 의 값을 NULL 로 입력
 */

Int8T nnListSetSubNodeCompFunc(
    ListNodeT *pListNode,
    listCmpFuncT pSubNodeCmpFunc)
{
    if(pListNode == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "ListNode Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }

    if(pListNode->pSubHead == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "SubList Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return LIST_ERR_NULL;
    }

    pListNode->pSubHead->dataCmpFunc = pSubNodeCmpFunc;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Set SubNode Compare Function : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
 
    return SUCCESS;
}
