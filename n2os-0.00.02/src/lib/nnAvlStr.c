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
 * @brief : N2OS Data Structure - AVL Tree Str
 *  - Block Name : Library
 *  - Process Name : rLibraryibmgr
 *  - Creator : Changwoo Lee, JaeSu Han
 *  - Initial Date : 2013/7/15
 */

/**
* @file : nnAvlStr.c
*
* $Id: nnAvlStr.c 874 2014-02-20 02:39:51Z lcw1127 $
* $Author: lcw1127 $
* $Date: 2014-02-20 11:39:51 +0900 (2014-02-20, 목) $
* $Revision: 874 $
* $LastChangedBy: lcw1127 $
**/

/*******************************************************************************
 *                               INCLUDE FILES
 ******************************************************************************/
//#include "nnMemmgr.h"
//#include "nnLog.h"
#include <string.h>
#include "nnPrefix.h"
#include "nnAvl.h"
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
static const Uint8T gMaskbit[] = {0x00, 0x80, 0xc0, 0xe0, 0xf0,
                                        0xf8, 0xfc, 0xfe, 0xff};

/*******************************************************************************
*                               LOCAL VARIABLES
*******************************************************************************/


/*******************************************************************************
 *                                   CODE
 ******************************************************************************/

/**
 * Description: AvlTreeStr 를 사용할 수 있도록 초기화 하는 함수
 *
 * @param [in] dataType : AvlTreeStr 에 담을 Data 의 Type
 *
 * @retval : AVL_STR_ERR_AVL_ALLOC(-1) : AvlTreeStr 의
 *                                   메모리 할당에 실패일 때,
 *           AVL_STR_ERR_ARGUMENT(-4) : 잘못된 Data Type 일 때,
 *           초기화 된 AvlTreeStr : 성공
 *
 * @code
 *  AvlTreeStrT pAvlTre = NULL;
 *
 *  nAvlTreeStr = nnAvlInitStr(TEST_TYPE);
 * @endcode
 *
 * @test
 *  pNodeCmpFunc 를 NULL 로 입력,
 *  dataType 의 값을 사용자 타입의 범위를 벗어난 값으로 입력
 */

AvlTreeStrT *nnAvlInitStr(Uint32T dataType)
{
    AvlTreeStrT *pNewTree = NULL;

    if((dataType >= MEM_MAX_USER_TYPE && dataType < MEM_SYS_START)
       || dataType >= MEM_MAX_SYS_TYPE)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Wrong AvlTreeStr DataType\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (AvlTreeStrT *)AVL_STR_ERR_ARGUMENT;
    }

    pNewTree = (AvlTreeStrT *)NNMALLOC(AVL_TREE_HEAD, sizeof(AvlTreeStrT));

    if((Int32T)pNewTree <= 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "NewAvlTreeStr Memory Allocate : Failure(%d)\n",
                       (Int32T)pNewTree);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (AvlTreeStrT *)AVL_STR_ERR_AVL_ALLOC;
    }

    pNewTree->pRoot = NULL;
    pNewTree->count = 0;
    pNewTree->dataType = dataType;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Init NewAvlTreeStr : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return pNewTree;
}


/**
 * Description: AvlTreeStr 에 저장된 Data 삭제 및 메모리를 해제하는 함수
 *
 * @param [in/out] pAvlTreeStr : 모든 Data 및 메모리를 해제 할 AvlTreeStr
 *
 * @retval : None
 *
 * @code
 *  nnAvlFreeStr(pAvlTreeStr);
 * @endcode
 */

void nnAvlFreeStr(AvlTreeStrT *pAvlTreeStr)
{
    if (pAvlTreeStr == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Avl Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    nnAvlDeleteAllNodeStr(pAvlTreeStr);

    NNFREE(AVL_TREE_HEAD, pAvlTreeStr);

    pAvlTreeStr = NULL;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_ERR, "Free AvlTreeStr Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}


/**
 * Description: key 를 가진 AvlNodeStr 를 삭제하는 함수
 *
 * @param [in] pAvlTreeStr : AvlNodeStr 를 삭제할 AvlTreeStr
 * @param [in] key : 삭제할 Key
 *
 * @retval : None
 *
 * @code
 *  Int8T key[64] = "key";
 *  nnAvlDeleteNodeStr(pAvlTreeStr, key);
 * @endcode
 *
 * @test
 *  pAvlTreeStr 를 NULL 로 입력,
 *  pAvlTreeStr 의 사용자 비교함수를 NULL 인 상태에서 함수 수행,
 *  key 를 NULL 로 입력,
 *  pAvlTreeStr 에 아무런 값도 없는 상태에서 수행,
 *  pAvlTreeStr 에 없는 값을 key 의 값으로 입력
 */

void nnAvlDeleteNodeStr(AvlTreeStrT *pAvlTreeStr, StringT key)
{
    if (pAvlTreeStr == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Avl Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    if (key == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Key Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    AvlNodeStrT *pDelNode = NULL;
    AvlNodeStrT *pMaxLeftNode = NULL;
    AvlNodeStrT *pAdjustNode = NULL;

    pDelNode = nnAvlLookupNodeStr(pAvlTreeStr, key);

    if (pDelNode != NULL)
    {
    }
    else
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Find  Delete Node : Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    /* 삭제할 노드의 왼쪽 자식이 있을 때 */
    if (pDelNode->pLeft != NULL)
    {
        pMaxLeftNode = nnAvlGetMaxLeftNodeStr(pDelNode->pLeft);

        /* 삭제할 노드의 왼쪽 Max 노드가 삭제할 노드의 왼쪽 자식일 때 */
        if (pMaxLeftNode == pDelNode->pLeft)
        {
            /* 삭제할 노드가 루트가 일 때 */
            if (pDelNode == pAvlTreeStr->pRoot)
            {
                pAvlTreeStr->pRoot = pMaxLeftNode;
                pMaxLeftNode->pParent = NULL;
            }
            /* 삭제할 노드가 루트가 아닐 때 */
            else /* if (pDelNode != pAvlTreeStr->pRoot) */
            {
                pDelNode->pParent->pRight = pMaxLeftNode;
                pMaxLeftNode->pParent = pDelNode->pParent;
            }

            if (pDelNode->pRight != NULL)
            {
                pDelNode->pRight->pParent = pMaxLeftNode;
            }

            pMaxLeftNode->pRight = pDelNode->pRight;

            pAdjustNode = pMaxLeftNode;
        }
        /* 그 외 */
        /* 삭제할 노드의 왼쪽 Max 노드 */
        else
        {
            if (pMaxLeftNode->pParent != NULL && pMaxLeftNode->pParent != pDelNode)
            {
                pAdjustNode = pMaxLeftNode->pParent;
            }
            else
            {
                pAdjustNode = pMaxLeftNode;
            }

            /* Max 노드의 부모는 Max 의 왼쪽 노드를 오른쪽 자식으로 등록 */
            if (pMaxLeftNode->pLeft != NULL)
            {
                pMaxLeftNode->pParent->pRight = pMaxLeftNode->pLeft;
                pMaxLeftNode->pLeft->pParent = pMaxLeftNode->pParent;
            }
            /* Max 의 왼쪽 노드가 NULL 이면 */
            else
            {
                pMaxLeftNode->pParent->pRight = NULL;
            }

            /* 삭제할 노드가 루트 일 때 */
            if (pDelNode == pAvlTreeStr->pRoot)
            {
                pAvlTreeStr->pRoot = pMaxLeftNode;
                pMaxLeftNode->pParent = NULL;
            }
            else /* if (pDelNode != pAvlTreeStr->pRoot) */
            {
                pDelNode->pParent->pRight = pMaxLeftNode;
                pMaxLeftNode->pParent = pDelNode->pParent;
            }

            /* 삭제할 노드의 오른쪽 자식이 있을 때 */
            if (pDelNode->pRight != NULL)
            {
                pDelNode->pRight->pParent = pMaxLeftNode;
                pMaxLeftNode->pRight = pDelNode->pRight;
            }
            /* 삭제할 노드의 오른쪽 자식이 없을 때 */
            else
            {
                pMaxLeftNode->pRight = NULL;
            }

            /* 삭제할 노드의 왼쪽 자식의 연결 수행 */
            if (pDelNode->pLeft != NULL)
            {
                pDelNode->pLeft->pParent = pMaxLeftNode;
            }

            pMaxLeftNode->pLeft = pDelNode->pLeft;

            pAdjustNode = pMaxLeftNode;
        }
    }
    /* 삭제할 노드의 왼쪽 자식이 없고 오른쪽 자식이 있을 때 */
    /* 왼쪽 자식이 없다면 오른쪽 자식은 본인 밖에 없음  */
    else if (pDelNode->pRight != NULL)
    {
        /* 오른쪽 자식의 왼쪽, 오른쪽 자식이 모두 NULL 일 때 */
        if (pDelNode->pRight->pLeft == NULL && pDelNode->pRight->pRight == NULL)
        {
            /* 삭제할 노드가 루트 일 때 */
            if (pDelNode == pAvlTreeStr->pRoot)
            {
                pAvlTreeStr->pRoot = pDelNode->pRight;
                pDelNode->pRight->pParent = NULL;
            }
            else if (pDelNode == pDelNode->pParent->pRight)
            {
                pDelNode->pParent->pRight = pDelNode->pRight;
                pDelNode->pRight->pParent = pDelNode->pParent;
            }
            else if (pDelNode == pDelNode->pParent->pLeft)
            {
                pDelNode->pParent->pLeft = pDelNode->pRight;
                pDelNode->pRight->pParent = pDelNode->pParent;
            }

            pAdjustNode = pDelNode->pRight;
        }
        /* 그 외 */
        else
        {
        }
    }
    else
    {
        if (pAvlTreeStr->pRoot == pDelNode)
        {
            if (pDelNode->pLeft != NULL)
            {
                pAvlTreeStr->pRoot = pDelNode->pLeft;
                pDelNode->pLeft->pParent = NULL;

                pAdjustNode = pDelNode->pLeft;
            }
            else if (pDelNode->pRight != NULL)
            {
                pAvlTreeStr->pRoot = pDelNode->pRight;
                pDelNode->pRight->pParent = NULL;

                pAdjustNode = pDelNode->pRight;
            }
            else
            {
                pAvlTreeStr->pRoot = NULL;
                pAdjustNode = NULL;
            }
        }
        else
        {
            pAdjustNode = pDelNode->pParent;

            if (pDelNode->pParent->pLeft == pDelNode)
            {
                pDelNode->pParent->pLeft = NULL;
            }
            else
            {
                pDelNode->pParent->pRight = NULL;
            }
        }
    }

    NNFREE(pAvlTreeStr->dataType, pDelNode->pData);
    NNFREE(AVL_TREE_NODE, pDelNode);

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Delete Node : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    --pAvlTreeStr->count;

    nnAvlAdjustTreeStr(pAvlTreeStr, pAdjustNode);
}


/**
 * Description: AvlTreeStr 에 저장된 모든 AvlNodeStr 를 삭제하는 함수
 *
 * @param [in] pAvlTreeStr : 모든 AvlNodeStr 를 삭제할 AvlTreeStr
 *
 * @retval : None
 *
 * @code
 *  nnAvlDeleteAllNodeStr(pAvlTreeStr);
 * @endcode
 *
 * @test
 *  pAvlTreeStr 를 NULL 로 입력,
 *  pAvlTreeStr 에 아무런 값도 없는 상태에서 수행
 */

void nnAvlDeleteAllNodeStr(AvlTreeStrT *pAvlTreeStr)
{
    if (pAvlTreeStr == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTreeStr Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }
    else if (pAvlTreeStr->pRoot == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "AvlTreeStr Is Empty\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }
    else
    {
        AvlNodeStrT *pTempNode = NULL;
        AvlNodeStrT *pDelNode = NULL;

        for (pTempNode = pAvlTreeStr->pRoot; pTempNode != NULL; )
        {
            if (pTempNode->pLeft != NULL)
            {
                pTempNode = pTempNode->pLeft;
            }
            else if (pTempNode->pRight != NULL)
            {
                pTempNode = pTempNode->pRight;
            }
            else if (pTempNode->pLeft == NULL && pTempNode->pRight == NULL)
            {
                pDelNode = pTempNode;

                if (pTempNode->pParent != NULL)
                {
                    pTempNode = pTempNode->pParent;

                    if (pTempNode->pLeft == pDelNode)
                    {
                        pTempNode->pLeft = NULL;
                    }
                    else
                    {
                        pTempNode->pRight = NULL;
                    }
                }
                else
                {
                    pTempNode = NULL;
                }

                NNFREE(pAvlTreeStr->dataType, pDelNode->pData);
                NNFREE(AVL_TREE_NODE, pDelNode);

                pDelNode = NULL;

                --pAvlTreeStr->count;
            }
        }

        pAvlTreeStr->pRoot = NULL;
    }


    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Delete All Node Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}


/**
 * Description: key 를 가진 AvlNodeStr 를 삭제하고
 *              그 Data 는 삭제하지 않는 함수
 *
 * @param [in] pAvlTreeStr : AvlNodeStr 를 삭제할 AvlTreeStr
 * @param [in] key : 삭제할 Key
 *
 * @retval : None
 *
 * @code
 *  Int8T key[64] = "key";
 *  nnAvlRemoveNodeStr(pAvlTreeStr, key);
 * @endcode
 *
 * @test
 *  pAvlTreeStr 를 NULL 로 입력,
 *  pAvlTreeStr 의 사용자 비교함수를 NULL 인 상태에서 함수 수행,
 *  key 를 NULL 로 입력,
 *  pAvlTreeStr 에 아무런 값도 없는 상태에서 수행,
 *  pAvlTreeStr 에 없는 값을 key 의 값으로 입력
 */

void nnAvlRemoveNodeStr(AvlTreeStrT *pAvlTreeStr, StringT key)
{
    if (pAvlTreeStr == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Avl Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    if (key == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Key Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    AvlNodeStrT *pDelNode = NULL;
    AvlNodeStrT *pMaxLeftNode = NULL;
    AvlNodeStrT *pAdjustNode = NULL;

    pDelNode = nnAvlLookupNodeStr(pAvlTreeStr, key);

    if (pDelNode != NULL)
    {
    }
    else
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Find  Delete Node : Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    /* 삭제할 노드의 왼쪽 자식이 있을 때 */
    if (pDelNode->pLeft != NULL)
    {
        pMaxLeftNode = nnAvlGetMaxLeftNodeStr(pDelNode->pLeft);

        /* 삭제할 노드의 왼쪽 Max 노드가 삭제할 노드의 왼쪽 자식일 때 */
        if (pMaxLeftNode == pDelNode->pLeft)
        {
            /* 삭제할 노드가 루트가 일 때 */
            if (pDelNode == pAvlTreeStr->pRoot)
            {
                pAvlTreeStr->pRoot = pMaxLeftNode;
                pMaxLeftNode->pParent = NULL;
            }
            /* 삭제할 노드가 루트가 아닐 때 */
            else /* if (pDelNode != pAvlTreeStr->pRoot) */
            {
                pDelNode->pParent->pRight = pMaxLeftNode;
                pMaxLeftNode->pParent = pDelNode->pParent;
            }

            if (pDelNode->pRight != NULL)
            {
                pDelNode->pRight->pParent = pMaxLeftNode;
            }

            pMaxLeftNode->pRight = pDelNode->pRight;

            pAdjustNode = pMaxLeftNode;
        }
        /* 그 외 */
        /* 삭제할 노드의 왼쪽 Max 노드 */
        else
        {
            if (pMaxLeftNode->pParent != NULL && pMaxLeftNode->pParent != pDelNode)
            {
                pAdjustNode = pMaxLeftNode->pParent;
            }
            else
            {
                pAdjustNode = pMaxLeftNode;
            }

            /* Max 노드의 부모는 Max 의 왼쪽 노드를 오른쪽 자식으로 등록 */
            if (pMaxLeftNode->pLeft != NULL)
            {
                pMaxLeftNode->pParent->pRight = pMaxLeftNode->pLeft;
                pMaxLeftNode->pLeft->pParent = pMaxLeftNode->pParent;
            }
            /* Max 의 왼쪽 노드가 NULL 이면 */
            else
            {
                pMaxLeftNode->pParent->pRight = NULL;
            }

            /* 삭제할 노드가 루트 일 때 */
            if (pDelNode == pAvlTreeStr->pRoot)
            {
                pAvlTreeStr->pRoot = pMaxLeftNode;
                pMaxLeftNode->pParent = NULL;
            }
            else /* if (pDelNode != pAvlTreeStr->pRoot) */
            {
                pDelNode->pParent->pRight = pMaxLeftNode;
                pMaxLeftNode->pParent = pDelNode->pParent;
            }

            /* 삭제할 노드의 오른쪽 자식이 있을 때 */
            if (pDelNode->pRight != NULL)
            {
                pDelNode->pRight->pParent = pMaxLeftNode;
                pMaxLeftNode->pRight = pDelNode->pRight;
            }
            /* 삭제할 노드의 오른쪽 자식이 없을 때 */
            else
            {
                pMaxLeftNode->pRight = NULL;
            }

            /* 삭제할 노드의 왼쪽 자식의 연결 수행 */
            if (pDelNode->pLeft != NULL)
            {
                pDelNode->pLeft->pParent = pMaxLeftNode;
            }

            pMaxLeftNode->pLeft = pDelNode->pLeft;

            pAdjustNode = pMaxLeftNode;
        }
    }
    /* 삭제할 노드의 왼쪽 자식이 없고 오른쪽 자식이 있을 때 */
    /* 왼쪽 자식이 없다면 오른쪽 자식은 본인 밖에 없음  */
    else if (pDelNode->pRight != NULL)
    {
        /* 오른쪽 자식의 왼쪽, 오른쪽 자식이 모두 NULL 일 때 */
        if (pDelNode->pRight->pLeft == NULL && pDelNode->pRight->pRight == NULL)
        {
            /* 삭제할 노드가 루트 일 때 */
            if (pDelNode == pAvlTreeStr->pRoot)
            {
                pAvlTreeStr->pRoot = pDelNode->pRight;
                pDelNode->pRight->pParent = NULL;
            }
            else if (pDelNode == pDelNode->pParent->pRight)
            {
                pDelNode->pParent->pRight = pDelNode->pRight;
                pDelNode->pRight->pParent = pDelNode->pParent;
            }
            else if (pDelNode == pDelNode->pParent->pLeft)
            {
                pDelNode->pParent->pLeft = pDelNode->pRight;
                pDelNode->pRight->pParent = pDelNode->pParent;
            }

            pAdjustNode = pDelNode->pRight;
        }
        /* 그 외 */
        else
        {
        }
    }
    else
    {
        if (pAvlTreeStr->pRoot == pDelNode)
        {
            if (pDelNode->pLeft != NULL)
            {
                pAvlTreeStr->pRoot = pDelNode->pLeft;
                pDelNode->pLeft->pParent = NULL;

                pAdjustNode = pDelNode->pLeft;
            }
            else if (pDelNode->pRight != NULL)
            {
                pAvlTreeStr->pRoot = pDelNode->pRight;
                pDelNode->pRight->pParent = NULL;

                pAdjustNode = pDelNode->pRight;
            }
            else
            {
                pAvlTreeStr->pRoot = NULL;
                pAdjustNode = NULL;
            }
        }
        else
        {
            pAdjustNode = pDelNode->pParent;

            if (pDelNode->pParent->pLeft == pDelNode)
            {
                pDelNode->pParent->pLeft = NULL;
            }
            else
            {
                pDelNode->pParent->pRight = NULL;
            }
        }
    }

//  Data 는 삭제하지 않는다
//  NNFREE(pAvlTreeStr->dataType, pDelNode->pData);
    NNFREE(AVL_TREE_NODE, pDelNode);

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Delete Node : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    --pAvlTreeStr->count;

    nnAvlAdjustTreeStr(pAvlTreeStr, pAdjustNode);
}


/**
 * Description: AvlTreeStr 에 저장된 모든 AvlNodeStr 를 삭제하며
 *              저장된 Data 는 삭제하지 않는 함수
 *
 * @param [in] pAvlTreeStr : 모든 AvlNodeStr 를 삭제할 AvlTreeStr
 *
 * @retval : None
 *
 * @code
 *  nnAvlRemoveAllNodeStr(pAvlTreeStr);
 * @endcode
 *
 * @test
 *  pAvlTreeStr 를 NULL 로 입력,
 *  pAvlTreeStr 에 아무런 값도 없는 상태에서 수행
 */

void nnAvlRemoveAllNodeStr(AvlTreeStrT *pAvlTreeStr)
{
    if (pAvlTreeStr == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTreeStr Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }
    else if (pAvlTreeStr->pRoot == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "AvlTreeStr Is Empty\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }
    else
    {
        AvlNodeStrT *pTempNode = NULL;
        AvlNodeStrT *pDelNode = NULL;

        for (pTempNode = pAvlTreeStr->pRoot; pTempNode != NULL; )
        {
            if (pTempNode->pLeft != NULL)
            {
                pTempNode = pTempNode->pLeft;
            }
            else if (pTempNode->pRight != NULL)
            {
                pTempNode = pTempNode->pRight;
            }
            else if (pTempNode->pLeft == NULL && pTempNode->pRight == NULL)
            {
                pDelNode = pTempNode;

                if (pTempNode->pParent != NULL)
                {
                    pTempNode = pTempNode->pParent;

                    if (pTempNode->pLeft == pDelNode)
                    {
                        pTempNode->pLeft = NULL;
                    }
                    else
                    {
                        pTempNode->pRight = NULL;
                    }
                }
                else
                {
                    pTempNode = NULL;
                }

//              Data 는 삭제하지 않는다
//              NNFREE(pAvlTreeStr->dataType, pDelNode->pData);
                NNFREE(AVL_TREE_NODE, pDelNode);

                pDelNode = NULL;

                --pAvlTreeStr->count;
            }
        }

        pAvlTreeStr->pRoot = NULL;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Delete All Node Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}


/**
 * Description: AvlTreeStr 의 최상위 Node 의 Data 를 리턴하는 함수
 *
 * @param [in] pAvlTreeStr : 최상위 Node 의 Data 를 가져올 AvlTreeStr
 *
 * @retval : AVL_STR_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           NULL : 최상위 Node 가 없을 때,
 *           최상위 Node 의 Data : 성공
 *
 * @code
 *  testT *pRet = NULL;
 *
 *  pRet = nnAvlGetRootDataStr(pAvlTreeStr);
 * @endcode
 *
 * @test
 *  pAvlTreeStr 를 NULL 로 입력,
 *  pAvlTreeStr 에 아무런 값도 없는 상태에서 수행
 */

void *nnAvlGetRootDataStr(AvlTreeStrT *pAvlTreeStr)
{
    if ((Int32T)pAvlTreeStr <= 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTreeStr Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *)AVL_STR_ERR_NULL;
    }
    else if (pAvlTreeStr->pRoot == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "AvlTreeStr Is Empty\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *)NULL;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Get Root Node : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return pAvlTreeStr->pRoot->pData;
}


/**
 * Description: AvlTreeStr 에서 key 값을
 *              가진 AvlNodeStr 를 검색해 그 Data 를 리턴하는 함수
 *
 * @param [in] pAvlTreeStr : Data 를 검색할 AvlTreeStr
 * @param [in] key : 검색하려는 key
 *
 * @retval : AVL_STR_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           NULL : key 와 같은 값을 가진 AvlNodeStr 가 없을 때,
 *           검색된 AvlNodeStr 의 Data : 성공
 *
 * @code
 *  Int8T key[64] = "key";
 *  testT *pRet = NULL;
 *
 *  pRet = nnAvlLookupDataStr(pAvlTreeStr, key);
 * @endcode
 *
 * @test
 *  pAvlTreeStr 를 NULL 로 입력,
 *  pAvlTreeStr 의 사용자 비교함수를 NULL 인 상태에서 함수 수행,
 *  key 를 NULL 로 입력,
 *  pAvlTreeStr 에 아무런 값도 없는 상태에서 수행,
 *  pAvlTreeStr 에 없는 값을 key 의 값으로 입력
 */

void *nnAvlLookupDataStr(AvlTreeStrT *pAvlTreeStr, StringT key)
{
    AvlNodeStrT *tempNode = NULL;

    if (pAvlTreeStr == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTreeStr Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *)AVL_STR_ERR_NULL;
    }
    else if (key == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Key Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *)AVL_STR_ERR_NULL;
    }

    tempNode = nnAvlLookupNodeStr(pAvlTreeStr, key);

    if (tempNode == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Lookup Data : Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return NULL;
    }
    else if ((Int32T)tempNode < 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Lookup Data : Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *)tempNode;
    }
    else
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_DEBUG, "Lookup Data : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return tempNode->pData;
    }
}


/**
 * Description: AvlTreeStr 에 저장된 AvlNodeStr 의 개수를 가져오는 함수
 *
 * @param [in] pAvlTreeStr : 저장된 AvlNodeStr 개수를 가져올 AvlTreeStr
 *
 * @retval : AVL_STR_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           저장된 AvlNodeStr 의 개수 : 성공
 *
 * @code
 *  Int64T ret = 0;
 *
 *  ret = nnAvlCountStr(pAvlTreeStr);
 * @endcode
 *
 * @test
 *  pAvlTreeStr 를 NULL 로 입력,
 *  pAvlTreeStr 에 아무런 값도 없는 상태에서 수행
 */

Int64T nnAvlCountStr(AvlTreeStrT *pAvlTreeStr)
{
    if (pAvlTreeStr == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTreeStr Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        return AVL_STR_ERR_NULL;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Get AvlTreeStr Count : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return pAvlTreeStr->count;
}


/**
 * Description: Data 를 AvlNodeStr 에 담아 AvlTreeStr 에 저장하는 함수
 *
 * @param [in] pAvlTreeStr : Data 를 저장할 AvlTreeStr
 * @param [in] key : 저장에 사용하려는 key 값
 * @param [in] pData : 저장하려는 Data
 *
 * @retval : AVL_STR_ERR_AVLNODE_ALLOC(-2) - AvlNodeStr 의
 *                                       메모리 할당에 실패 일 때
 *           AVL_STR_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  저장 실패 시 pData 의 메모리는 사용자가 관리하여야 함
 *  비교 함수를 통해 AvlTreeStr 에서 저장될 위치가 결정 됨
 *
 * @code
 *  Int8T ret = 0;
 *  StringT key = "key";
 *  testT pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  ret = nnAvlAddNodeStr(pAvlTreeStr, key, pTest);
 * @endcode
 *
 * @test
 *  pAvlTreeStr 를 NULL 로 입력,
 *  key 를 NULL 로 입력,
 *  pData 를 NULL 로 입력
 */

Int8T nnAvlAddNodeStr(AvlTreeStrT *pAvlTreeStr, StringT key, void *pData)
{
    if (pAvlTreeStr == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTreeStr Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return AVL_STR_ERR_NULL;
    }
    else if (key == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Key Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return AVL_STR_ERR_NULL;
    }
    else if (pData == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return AVL_STR_ERR_NULL;
    }

    Int8T compareResult = 0;

    AvlNodeStrT *pNewNode = (AvlNodeStrT *)NNMALLOC(AVL_TREE_NODE,
                                                      sizeof(AvlNodeStrT));

    if((Int32T)pNewNode <= 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "NewNode Memory Allocate : Failure(%d)\n",
                       (Int32T)pNewNode);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return AVL_STR_ERR_AVLNODE_ALLOC;    
    }

    pNewNode->pLeft = NULL;
    pNewNode->pRight = NULL;
    pNewNode->pParent = NULL;
    pNewNode->balance = 0;
    pNewNode->height = 1;
    strcpy(pNewNode->key, key);
    pNewNode->pData = pData;

    /* No Node */
    if (pAvlTreeStr->count == 0 && pAvlTreeStr->pRoot == NULL)
    {
        pAvlTreeStr->pRoot = pNewNode;
    }
    else
    {
        /* Parent Node */
        AvlNodeStrT *pTempNode = pAvlTreeStr->pRoot;

        /* Find Parent Location */
        while (pTempNode != NULL)
        {
            compareResult = nnAvlKeyCmpFuncStr(pNewNode->key, pTempNode->key);

            /* Smaller Than pTempNode */
            if (compareResult == 1 || compareResult == -1)
            {
                if (pTempNode->pLeft == NULL)
                {
                    pTempNode->pLeft = pNewNode;

                    pNewNode->pParent = pTempNode;

                    break;
                }
                else
                {
                    pTempNode = pTempNode->pLeft;
                }
            }
            /* Larger Than pTempNode */
            else
            {
                if (pTempNode->pRight == NULL)
                {
                    pTempNode->pRight = pNewNode;

                    pNewNode->pParent = pTempNode;

                    break;
                }
                else
                {
                    pTempNode = pTempNode->pRight;
                }

            }
        }
    }

    ++pAvlTreeStr->count;

    nnAvlAdjustTreeStr(pAvlTreeStr, pNewNode);

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Add Node : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/*******************************************************************************
 * Function: <nnAvlLookupNodeStr>
 *
 * Description: <Lookup the AVL Tree>
 *
 * Parameters: pAvlTreeStr : AVL Tree lookup the node
               key : Want to find key
 *
 * Returns: NULL : no lookup node
 *          AVL_STR_ERR_NULL(-3) : pAvlTreeStr is NULL
 *        lookup node
*******************************************************************************/

AvlNodeStrT *nnAvlLookupNodeStr(AvlTreeStrT *pAvlTreeStr, StringT key)
{
    if (pAvlTreeStr == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTreeStr Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (AvlNodeStrT *) AVL_STR_ERR_NULL;
    }
    else if (key == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Key Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (AvlNodeStrT *) AVL_STR_ERR_NULL;
    }
    else if (pAvlTreeStr->pRoot == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "AvlTreeStr Is Empty\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return NULL;
    }

    AvlNodeStrT *pTempNode = NULL;

    for (pTempNode = pAvlTreeStr->pRoot; pTempNode != NULL; )
    {
	// 비교함수 추가
        /* Smaller than Node key */
        if (nnAvlKeyCmpFuncStr(pTempNode->key, key) == 0)
        {
            if (pTempNode->pLeft != NULL)
            {
                pTempNode = pTempNode->pLeft;
            }
            else
            {
                break;
            }
        }
        /* Larger than Node key */
        else if (nnAvlKeyCmpFuncStr(pTempNode->key, key) == 1)
        {
            if (pTempNode->pRight != NULL)
            {
                pTempNode = pTempNode->pRight;
            }
            else
            {
                break;
            }
        }
        else if (nnAvlKeyCmpFuncStr(pTempNode->key, key) == -1)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_INFO, "Lookup Node : Success\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return pTempNode;
        }
        else
        {
            break;
        }
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_ERR, "Lookup Node : Failure\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return NULL;
}


/*******************************************************************************
 * Function: <nnAvlAdjustTreeStr>
 *
 * Description: <Adjust to the AVL Tree>
 *
 * Parameters: pAvlTreeStr : AVL Tree have a pAvlNodeStr
               pAvlNodeStr : AVL Node to do adjust
 *
 * Returns: None
*******************************************************************************/

void nnAvlAdjustTreeStr(AvlTreeStrT *pAvlTreeStr, AvlNodeStrT *pAvlNodeStr)
{
    if (pAvlTreeStr == NULL)
    {
/*
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTreeStr Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
*/
        return;
    }
    else if (pAvlNodeStr == NULL)
    {
/*
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlNodeStr Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
*/
        return;
    }

    nnAvlUpdateBalanceStr(pAvlNodeStr);

    if (pAvlNodeStr->balance == 2) {
        if (pAvlNodeStr->pLeft->balance == 1)
        {
            AvlNodeStrT *pTempNode = NULL;

            pTempNode = nnAvlRotateRightStr(pAvlNodeStr);

            /* Select Root */
            if (pTempNode->pParent == NULL)
            {
                pAvlTreeStr->pRoot = pTempNode;
            }
        }
        else if (pAvlNodeStr->pLeft->balance == -1)
        {
            AvlNodeStrT *pTempNode = NULL;

            pAvlNodeStr->pLeft = nnAvlRotateLeftStr(pAvlNodeStr->pLeft);
            pTempNode = nnAvlRotateRightStr(pAvlNodeStr);

            /* Select Root */
            if (pTempNode->pParent == NULL)
            {
                pAvlTreeStr->pRoot = pTempNode;
            }
        }
    }
    else if (pAvlNodeStr->balance == -2)
    {
        if (pAvlNodeStr->pRight->balance == -1)
        {
            AvlNodeStrT *pTempNode = NULL;

            pTempNode = nnAvlRotateLeftStr(pAvlNodeStr);

            /* Select Root */
            if (pTempNode->pParent == NULL)
            {
                pAvlTreeStr->pRoot = pTempNode;
            }
        }
        else if (pAvlNodeStr->pRight->balance == 1)
        {
            AvlNodeStrT *pTempNode = NULL;

            pAvlNodeStr->pRight = nnAvlRotateRightStr(pAvlNodeStr->pRight);
            pTempNode = nnAvlRotateLeftStr(pAvlNodeStr);

            /* Select Root */
            if (pTempNode->pParent == NULL)
            {
                pAvlTreeStr->pRoot = pTempNode;
            }
        }

    }

    if (pAvlTreeStr->pRoot != pAvlNodeStr)
    {
        nnAvlAdjustTreeStr(pAvlTreeStr, pAvlNodeStr->pParent);
    }
}


/*******************************************************************************
 * Function: <nnAvlUpdateBalanceStr>
 *
 * Description: <Update the AVL Node's Balance>
 *
 * Parameters: pAvlNodeStr : AVL Node to do update the balance
 *
 * Returns: None
*******************************************************************************/

void nnAvlUpdateBalanceStr(AvlNodeStrT *pAvlNodeStr)
{
    if (pAvlNodeStr == NULL)
    {
/*
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlNodeStr Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
*/
        return;
    }

    pAvlNodeStr->height = nnAvlGetMaxHeightStr(nnAvlGetHeightStr(pAvlNodeStr->pLeft),
                                        nnAvlGetHeightStr(pAvlNodeStr->pRight)) + 1;
    pAvlNodeStr->balance = nnAvlGetHeightStr(pAvlNodeStr->pLeft) -
                        nnAvlGetHeightStr(pAvlNodeStr->pRight);
}


/*******************************************************************************
 * Function: <nnAvlRotateLeftStr>
 *
 * Description: <Rotate Left the AVL Tree's Nodes>
 *
 * Parameters: pAvlNodeStr : AVL Node Broken balance
 *
 * Returns: NULL : pAvlNodeStr is NULL
 *          Right Node
*******************************************************************************/

AvlNodeStrT *nnAvlRotateLeftStr(AvlNodeStrT *pAvlNodeStr)
{
    if (pAvlNodeStr == NULL)
    {
/*
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlNodeStr Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
*/
        return NULL;
    }

    AvlNodeStrT *pRightNode = NULL;
    AvlNodeStrT *pParentNode = NULL;

    pRightNode = pAvlNodeStr->pRight;
    pParentNode = pAvlNodeStr->pParent;

    if (pRightNode->pLeft != NULL)
    {
        pAvlNodeStr->pRight = pRightNode->pLeft;
        pRightNode->pLeft->pParent = pAvlNodeStr;
    }
    else
    {
        pAvlNodeStr->pRight = NULL;
    }

    pRightNode->pLeft = pAvlNodeStr;
    pAvlNodeStr->pParent = pRightNode;

    if (pParentNode != NULL)
    {
        if (pAvlNodeStr == pParentNode->pLeft)
        {
            pParentNode->pLeft = pRightNode;
        }
        else
        {
            pParentNode->pRight = pRightNode;
        }
    }

    pRightNode->pParent = pParentNode;

    nnAvlUpdateBalanceStr(pAvlNodeStr);
    nnAvlUpdateBalanceStr(pRightNode);

    return pRightNode;
}


/*******************************************************************************
 * Function: <nnAvlRotateRightStr>
 *
 * Description: <Rotate Right the AVL Tree's Nodes>
 *
 * Parameters: pAvlNodeStr : AVL Node Broken balance
 *
 * Returns: NULL : pAvlNodeStr is NULL
 *          Left Node
*******************************************************************************/

AvlNodeStrT *nnAvlRotateRightStr(AvlNodeStrT *pAvlNodeStr)
{
    if (pAvlNodeStr == NULL)
    {
/*
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlNodeStr Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
*/
        return NULL;
    }

    AvlNodeStrT *pLeftNode = NULL;
    AvlNodeStrT *pParentNode = NULL;

    pLeftNode = pAvlNodeStr->pLeft;
    pParentNode = pAvlNodeStr->pParent;

    if (pLeftNode->pRight != NULL)
    {
        pAvlNodeStr->pLeft = pLeftNode->pRight;
        pLeftNode->pRight->pParent = pAvlNodeStr;
    }
    else
    {
        pAvlNodeStr->pLeft = NULL;
    }

    pLeftNode->pRight = pAvlNodeStr;
    pAvlNodeStr->pParent = pLeftNode;

    if (pParentNode != NULL)
    {
        if (pAvlNodeStr == pParentNode->pLeft)
        {
            pParentNode->pLeft = pLeftNode;
        }
        else
        {
            pParentNode->pRight = pLeftNode;
        }
    }

    pLeftNode->pParent = pParentNode;

    nnAvlUpdateBalanceStr(pAvlNodeStr);
    nnAvlUpdateBalanceStr(pLeftNode);

    return pLeftNode;

}


/*******************************************************************************
 * Function: <nnAvlGetHeightStr>
 *
 * Description: <AVL Node brings the height>
 *
 * Parameters: pAvlNodeStr : AVL Node want to know about height
 *
 * Returns: AVL Node's height
*******************************************************************************/

Int32T nnAvlGetHeightStr(AvlNodeStrT *pAvlNodeStr)
{
    if (pAvlNodeStr == NULL)
    {
        return 0;
    }
    else
    {
        return pAvlNodeStr->height;
    }
}


/*******************************************************************************
 * Function: <nnAvlGetMaxHeightStr>
 *
 * Description: <Compare with left AVL Node height and right AVL Node height>
 *
 * Parameters: leftHeight : AVL Node compare with rightHeight
               rightHeight : AVL Node compare with leftHeight
 *
 * Returns: Larger AVL Node's height
*******************************************************************************/

Int32T nnAvlGetMaxHeightStr(Int32T leftHeight, Int32T rightHeight)
{
    if (leftHeight >= rightHeight)
    {
        return leftHeight;
    }
    else
    {
        return rightHeight;
    }
}


/*******************************************************************************
 * Function: <nnAvlGetMaxLeftNodeStr>
 *
 * Description: <AVL Tree brings the max left node>
 *
 * Parameters: pAvlNodeStr : AVL Tree's root node want to know
                          about left side's max node
 *
 * Returns: Left side's max node
*******************************************************************************/

AvlNodeStrT *nnAvlGetMaxLeftNodeStr(AvlNodeStrT *pAvlNodeStr)
{
    if (pAvlNodeStr == NULL)
    {
/*
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlNodeStr Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
*/
        return NULL;
    }

    AvlNodeStrT *pTempNode = pAvlNodeStr;

    while (pTempNode->pRight != NULL)
    {
        pTempNode = pTempNode->pRight;
    }

    return pTempNode;
}


/*******************************************************************************
 * Function: <nnAvlKeyCmpFuncStr>
 *
 * Description: <Compare with two keys, which key is bigger>
 *
 * Parameters: leftKey : Left key value
               rightKey : Righ key value
 *
 * Returns: 0 : Left key is bigger than Right key
 *          1 : Right key is bigger than Left key
 *         -1 : Same two keys
*******************************************************************************/

Int8T nnAvlKeyCmpFuncStr(StringT leftKey, StringT rightKey)
{
    if ((Uint8T)*leftKey > (Uint8T)*rightKey)
    {
        return 0;
    }
    else if ((Uint8T)*leftKey < (Uint8T)*rightKey)
    {
        return 1;
    }
    else /* if (leftKey == rightKey) */
    {
        return -1;
    }
}
