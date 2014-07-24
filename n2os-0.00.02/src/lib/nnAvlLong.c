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
 * @brief : N2OS Data Structure - AVL Tree Long
 *  - Block Name : Library
 *  - Process Name : rLibraryibmgr
 *  - Creator : Changwoo Lee, JaeSu Han
 *  - Initial Date : 2013/7/15
 */

/**
* @file : nnAvlLong.c
*
* $Id: nnAvlLong.c 874 2014-02-20 02:39:51Z lcw1127 $
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
 * Description: AvlTreeLong 를 사용할 수 있도록 초기화 하는 함수
 *
 * @param [in] dataType : AvlTreeLong 에 담을 Data 의 Type
 *
 * @retval : AVL_LONG_ERR_AVL_ALLOC(-1) : AvlTreeLong 의
 *                                   메모리 할당에 실패일 때,
 *           AVL_LONG_ERR_ARGUMENT(-4) : 잘못된 Data Type 일 때,
 *           초기화 된 AvlTreeLong : 성공
 *
 * @code
 *  AvlTreeLongT pAvlTre = NULL;
 *
 *  nAvlTreeLong = nnAvlInitLong(TEST_TYPE);
 * @endcode
 *
 * @test
 *  pNodeCmpFunc 를 NULL 로 입력,
 *  dataType 의 값을 사용자 타입의 범위를 벗어난 값으로 입력
 */

AvlTreeLongT *nnAvlInitLong(Uint32T dataType)
{
    AvlTreeLongT *pNewTree = NULL;

    if((dataType >= MEM_MAX_USER_TYPE && dataType < MEM_SYS_START)
       || dataType >= MEM_MAX_SYS_TYPE)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Wrong AvlTreeLong DataType\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (AvlTreeLongT *)AVL_LONG_ERR_ARGUMENT;
    }

    pNewTree = (AvlTreeLongT *)NNMALLOC(AVL_TREE_HEAD, sizeof(AvlTreeLongT));

    if((Int32T)pNewTree <= 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "NewAvlTreeLong Memory Allocate : Failure(%d)\n",
                       (Int32T)pNewTree);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (AvlTreeLongT *)AVL_LONG_ERR_AVL_ALLOC;
    }

    pNewTree->pRoot = NULL;
    pNewTree->count = 0;
    pNewTree->dataType = dataType;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Init NewAvlTreeLong : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return pNewTree;
}


/**
 * Description: AvlTreeLong 에 저장된 Data 삭제 및 메모리를 해제하는 함수
 *
 * @param [in/out] pAvlTreeLong : 모든 Data 및 메모리를 해제 할 AvlTreeLong
 *
 * @retval : None
 *
 * @code
 *  nnAvlFreeLong(pAvlTreeLong);
 * @endcode
 */

void nnAvlFreeLong(AvlTreeLongT *pAvlTreeLong)
{
    if (pAvlTreeLong == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Avl Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    nnAvlDeleteAllNodeLong(pAvlTreeLong);

    NNFREE(AVL_TREE_HEAD, pAvlTreeLong);

    pAvlTreeLong = NULL;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_ERR, "Free AvlTreeLong Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}


/**
 * Description: key 를 가진 AvlNodeLong 를 삭제하는 함수
 *
 * @param [in] pAvlTreeLong : AvlNodeLong 를 삭제할 AvlTreeLong
 * @param [in] key : 삭제할 Key
 *
 * @retval : None
 *
 * @code
 *  Int64T key = 0;
 *  nnAvlDeleteNodeLong(pAvlTreeLong, key);
 * @endcode
 *
 * @test
 *  pAvlTreeLong 를 NULL 로 입력,
 *  pAvlTreeLong 의 사용자 비교함수를 NULL 인 상태에서 함수 수행,
 *  pAvlTreeLong 에 아무런 값도 없는 상태에서 수행,
 *  pAvlTreeLong 에 없는 값을 key 의 값으로 입력
 */

void nnAvlDeleteNodeLong(AvlTreeLongT *pAvlTreeLong, Int64T key)
{
    if (pAvlTreeLong == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Avl Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    AvlNodeLongT *pDelNode = NULL;
    AvlNodeLongT *pMaxLeftNode = NULL;
    AvlNodeLongT *pAdjustNode = NULL;

    pDelNode = nnAvlLookupNodeLong(pAvlTreeLong, key);

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
        pMaxLeftNode = nnAvlGetMaxLeftNodeLong(pDelNode->pLeft);

        /* 삭제할 노드의 왼쪽 Max 노드가 삭제할 노드의 왼쪽 자식일 때 */
        if (pMaxLeftNode == pDelNode->pLeft)
        {
            /* 삭제할 노드가 루트가 일 때 */
            if (pDelNode == pAvlTreeLong->pRoot)
            {
                pAvlTreeLong->pRoot = pMaxLeftNode;
                pMaxLeftNode->pParent = NULL;
            }
            /* 삭제할 노드가 루트가 아닐 때 */
            else /* if (pDelNode != pAvlTreeLong->pRoot) */
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
            if (pDelNode == pAvlTreeLong->pRoot)
            {
                pAvlTreeLong->pRoot = pMaxLeftNode;
                pMaxLeftNode->pParent = NULL;
            }
            else /* if (pDelNode != pAvlTreeLong->pRoot) */
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
            if (pDelNode == pAvlTreeLong->pRoot)
            {
                pAvlTreeLong->pRoot = pDelNode->pRight;
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
        if (pAvlTreeLong->pRoot == pDelNode)
        {
            if (pDelNode->pLeft != NULL)
            {
                pAvlTreeLong->pRoot = pDelNode->pLeft;
                pDelNode->pLeft->pParent = NULL;

                pAdjustNode = pDelNode->pLeft;
            }
            else if (pDelNode->pRight != NULL)
            {
                pAvlTreeLong->pRoot = pDelNode->pRight;
                pDelNode->pRight->pParent = NULL;

                pAdjustNode = pDelNode->pRight;
            }
            else
            {
                pAvlTreeLong->pRoot = NULL;
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

    NNFREE(pAvlTreeLong->dataType, pDelNode->pData);
    NNFREE(AVL_TREE_NODE, pDelNode);

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Delete Node : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    --pAvlTreeLong->count;

    nnAvlAdjustTreeLong(pAvlTreeLong, pAdjustNode);
}


/**
 * Description: AvlTreeLong 에 저장된 모든 AvlNodeLong 를 삭제하는 함수
 *
 * @param [in] pAvlTreeLong : 모든 AvlNodeLong 를 삭제할 List
 *
 * @retval : None
 *
 * @code
 *  nnAvlDeleteAllNodeLong(pAvlTreeLong);
 * @endcode
 *
 * @test
 *  pAvlTreeLong 를 NULL 로 입력,
 *  pAvlTreeLong 에 아무런 값도 없는 상태에서 수행
 */

void nnAvlDeleteAllNodeLong(AvlTreeLongT *pAvlTreeLong)
{
    if (pAvlTreeLong == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTreeLong Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }
    else if (pAvlTreeLong->pRoot == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "AvlTreeLong Is Empty\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }
    else
    {
        AvlNodeLongT *pTempNode = NULL;
        AvlNodeLongT *pDelNode = NULL;

        for (pTempNode = pAvlTreeLong->pRoot; pTempNode != NULL; )
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

                NNFREE(pAvlTreeLong->dataType, pDelNode->pData);
                NNFREE(AVL_TREE_NODE, pDelNode);

                pDelNode = NULL;

                --pAvlTreeLong->count;
            }
        }

        pAvlTreeLong->pRoot = NULL;
    }


    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Delete All Node Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}


/**
 * Description: key 를 가진 AvlNodeLong 를 삭제하고
 *              그 Data 는 삭제하지 않는 함수
 *
 * @param [in] pAvlTreeLong : AvlNodeLong 를 삭제할 AvlTreeLong
 * @param [in] key : 삭제할 Key
 *
 * @retval : None
 *
 * @code
 *  Int64T key = 0;
 *  nnAvlRemoveNodeLong(pAvlTreeLong, key);
 * @endcode
 *
 * @test
 *  pAvlTreeLong 를 NULL 로 입력,
 *  pAvlTreeLong 의 사용자 비교함수를 NULL 인 상태에서 함수 수행,
 *  pAvlTreeLong 에 아무런 값도 없는 상태에서 수행,
 *  pAvlTreeLong 에 없는 값을 key 의 값으로 입력
 */

void nnAvlRemoveNodeLong(AvlTreeLongT *pAvlTreeLong, Int64T key)
{
    if (pAvlTreeLong == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Avl Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    AvlNodeLongT *pDelNode = NULL;
    AvlNodeLongT *pMaxLeftNode = NULL;
    AvlNodeLongT *pAdjustNode = NULL;

    pDelNode = nnAvlLookupNodeLong(pAvlTreeLong, key);

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
        pMaxLeftNode = nnAvlGetMaxLeftNodeLong(pDelNode->pLeft);

        /* 삭제할 노드의 왼쪽 Max 노드가 삭제할 노드의 왼쪽 자식일 때 */
        if (pMaxLeftNode == pDelNode->pLeft)
        {
            /* 삭제할 노드가 루트가 일 때 */
            if (pDelNode == pAvlTreeLong->pRoot)
            {
                pAvlTreeLong->pRoot = pMaxLeftNode;
                pMaxLeftNode->pParent = NULL;
            }
            /* 삭제할 노드가 루트가 아닐 때 */
            else /* if (pDelNode != pAvlTreeLong->pRoot) */
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
            if (pDelNode == pAvlTreeLong->pRoot)
            {
                pAvlTreeLong->pRoot = pMaxLeftNode;
                pMaxLeftNode->pParent = NULL;
            }
            else /* if (pDelNode != pAvlTreeLong->pRoot) */
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
            if (pDelNode == pAvlTreeLong->pRoot)
            {
                pAvlTreeLong->pRoot = pDelNode->pRight;
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
        if (pAvlTreeLong->pRoot == pDelNode)
        {
            if (pDelNode->pLeft != NULL)
            {
                pAvlTreeLong->pRoot = pDelNode->pLeft;
                pDelNode->pLeft->pParent = NULL;

                pAdjustNode = pDelNode->pLeft;
            }
            else if (pDelNode->pRight != NULL)
            {
                pAvlTreeLong->pRoot = pDelNode->pRight;
                pDelNode->pRight->pParent = NULL;

                pAdjustNode = pDelNode->pRight;
            }
            else
            {
                pAvlTreeLong->pRoot = NULL;
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
//  NNFREE(pAvlTreeLong->dataType, pDelNode->pData);
    NNFREE(AVL_TREE_NODE, pDelNode);

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Delete Node : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    --pAvlTreeLong->count;

    nnAvlAdjustTreeLong(pAvlTreeLong, pAdjustNode);
}


/**
 * Description: AvlTreeLong 에 저장된 모든 AvlNodeLong 를 삭제하며
 *              저장된 Data 는 삭제하지 않는 함수
 *
 * @param [in] pAvlTreeLong : 모든 AvlNodeLong 를 삭제할 List
 *
 * @retval : None
 *
 * @code
 *  nnAvlRemoveAllNodeLong(pAvlTreeLong);
 * @endcode
 *
 * @test
 *  pAvlTreeLong 를 NULL 로 입력,
 *  pAvlTreeLong 에 아무런 값도 없는 상태에서 수행
 */

void nnAvlRemoveAllNodeLong(AvlTreeLongT *pAvlTreeLong)
{
    if (pAvlTreeLong == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTreeLong Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }
    else if (pAvlTreeLong->pRoot == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "AvlTreeLong Is Empty\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }
    else
    {
        AvlNodeLongT *pTempNode = NULL;
        AvlNodeLongT *pDelNode = NULL;

        for (pTempNode = pAvlTreeLong->pRoot; pTempNode != NULL; )
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
//              NNFREE(pAvlTreeLong->dataType, pDelNode->pData);
                NNFREE(AVL_TREE_NODE, pDelNode);

                pDelNode = NULL;

                --pAvlTreeLong->count;
            }
        }

        pAvlTreeLong->pRoot = NULL;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Delete All Node Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}


/**
 * Description: AvlTreeLong 의 최상위 Node 의 Data 를 리턴하는 함수
 *
 * @param [in] pAvlTreeLong : 최상위 Node 의 Data 를 가져올 AvlTreeLong
 *
 * @retval : AVL_LONG_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           NULL : 최상위 Node 가 없을 때,
 *           최상위 Node 의 Data : 성공
 *
 * @code
 *  testT *pRet = NULL;
 *
 *  pRet = nnAvlGetRootDataLong(pAvlTreeLong);
 * @endcode
 *
 * @test
 *  pAvlTreeLong 를 NULL 로 입력,
 *  pAvlTreeLong 에 아무런 값도 없는 상태에서 수행
 */

void *nnAvlGetRootDataLong(AvlTreeLongT *pAvlTreeLong)
{
    if ((Int32T)pAvlTreeLong <= 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTreeLong Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *)AVL_LONG_ERR_NULL;
    }
    else if (pAvlTreeLong->pRoot == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "AvlTreeLong Is Empty\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *)NULL;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Get Root Node : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return pAvlTreeLong->pRoot->pData;
}


/**
 * Description: AvlTreeLong 에서 key 값을
 *              가진 AvlNodeLong 를 검색해 그 Data 를 리턴하는 함수
 *
 * @param [in] pAvlTreeLong : Data 를 검색할 AvlTreeLong
 * @param [in] key : 검색하려는 key
 *
 * @retval : AVL_LONG_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           NULL : key 와 같은 값을 가진 AvlNodeLong 가 없을 때,
 *           검색된 AvlNodeLong 의 Data : 성공
 *
 * @code
 *  Int64T key = 0;
 *  testT *pRet = NULL;
 *
 *  pRet = nnAvlLookupDataLong(pAvlTreeLong, key);
 * @endcode
 *
 * @test
 *  pAvlTreeLong 를 NULL 로 입력,
 *  pAvlTreeLong 의 사용자 비교함수를 NULL 인 상태에서 함수 수행,
 *  pAvlTreeLong 에 아무런 값도 없는 상태에서 수행,
 *  pAvlTreeLong 에 없는 값을 key 의 값으로 입력
 */

void *nnAvlLookupDataLong(AvlTreeLongT *pAvlTreeLong, Int64T key)
{
    AvlNodeLongT *tempNode = NULL;

    if (pAvlTreeLong == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTreeLong Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *)AVL_LONG_ERR_NULL;
    }

    tempNode = nnAvlLookupNodeLong(pAvlTreeLong, key);

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
 * Description: AvlTreeLong 에 저장된 AvlNodeLong 의 개수를 가져오는 함수
 *
 * @param [in] pAvlTreeLong : 저장된 AvlNodeLong 개수를 가져올 AvlTreeLong
 *
 * @retval : AVL_LONG_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           저장된 AvlNodeLong 의 개수 : 성공
 *
 * @code
 *  Int64T ret = 0;
 *
 *  ret = nnAvlCountLong(pAvlTreeLong);
 * @endcode
 *
 * @test
 *  pAvlTreeLong 를 NULL 로 입력,
 *  pAvlTreeLong 에 아무런 값도 없는 상태에서 수행
 */

Int64T nnAvlCountLong(AvlTreeLongT *pAvlTreeLong)
{
    if (pAvlTreeLong == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTreeLong Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        return AVL_LONG_ERR_NULL;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Get AvlTreeLong Count : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return pAvlTreeLong->count;
}


/**
 * Description: Data 를 AvlNodeLong 에 담아 AvlTreeLong 에 저장하는 함수
 *
 * @param [in] pAvlTreeLong : Data 를 저장할 AvlTreeLong
 * @param [in] key : 저장에 사용하려는 key 값
 * @param [in] pData : 저장하려는 Data
 *
 * @retval : AVL_LONG_ERR_AVLNODE_ALLOC(-2) - AvlNodeLong 의
 *                                       메모리 할당에 실패 일 때
 *           AVL_LONG_ERR_NULL(-3) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  저장 실패 시 pData 의 메모리는 사용자가 관리하여야 함
 *  비교 함수를 통해 AvlTreeLong 에서 저장될 위치가 결정 됨
 *
 * @code
 *  Int8T ret = 0;
 *  Int64T key = 0;
 *  testT pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  ret = nnAvlAddNodeLong(pAvlTreeLong, key, pTest);
 * @endcode
 *
 * @test
 *  pAvlTreeLong 를 NULL 로 입력,
 *  pData 를 NULL 로 입력
 */

Int8T nnAvlAddNodeLong(AvlTreeLongT *pAvlTreeLong, Int64T key, void *pData)
{
    if (pAvlTreeLong == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTreeLong Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return AVL_LONG_ERR_NULL;
    }
    else if (pData == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return AVL_LONG_ERR_NULL;
    }

    Int8T compareResult = 0;

    AvlNodeLongT *pNewNode = (AvlNodeLongT *)NNMALLOC(AVL_TREE_NODE,
                                                      sizeof(AvlNodeLongT));

    if((Int32T)pNewNode <= 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "NewNode Memory Allocate : Failure(%d)\n",
                       (Int32T)pNewNode);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return AVL_LONG_ERR_AVLNODE_ALLOC;    
    }

    pNewNode->pLeft = NULL;
    pNewNode->pRight = NULL;
    pNewNode->pParent = NULL;
    pNewNode->balance = 0;
    pNewNode->height = 1;
    pNewNode->key = key;
    pNewNode->pData = pData;

    /* No Node */
    if (pAvlTreeLong->count == 0 && pAvlTreeLong->pRoot == NULL)
    {
        pAvlTreeLong->pRoot = pNewNode;
    }
    else
    {
        /* Parent Node */
        AvlNodeLongT *pTempNode = pAvlTreeLong->pRoot;

        /* Find Parent Location */
        while (pTempNode != NULL)
        {
            compareResult = nnAvlKeyCmpFuncLong(pNewNode->key, pTempNode->key);

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

    ++pAvlTreeLong->count;

    nnAvlAdjustTreeLong(pAvlTreeLong, pNewNode);

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Add Node : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/*******************************************************************************
 * Function: <nnAvlLookupNodeLong>
 *
 * Description: <Lookup the AVL Tree>
 *
 * Parameters: pAvlTreeLong : AVL Tree lookup the node
               key : Want to find key
 *
 * Returns: NULL : no lookup node
 *          AVL_LONG_ERR_NULL(-3) : pAvlTreeLong is NULL
 *        lookup node
*******************************************************************************/

AvlNodeLongT *nnAvlLookupNodeLong(AvlTreeLongT *pAvlTreeLong, Int64T key)
{
    if (pAvlTreeLong == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTreeLong Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (AvlNodeLongT *) AVL_LONG_ERR_NULL;
    }
    else if (pAvlTreeLong->pRoot == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "AvlTreeLong Is Empty\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return NULL;
    }

    AvlNodeLongT *pTempNode = NULL;

    for (pTempNode = pAvlTreeLong->pRoot; pTempNode != NULL; )
    {
	// 비교함수 추가
        /* Smaller than Node key */
        if (nnAvlKeyCmpFuncLong(pTempNode->key, key) == 0)
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
        else if (nnAvlKeyCmpFuncLong(pTempNode->key, key) == 1)
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
        else if (nnAvlKeyCmpFuncLong(pTempNode->key, key) == -1)
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
 * Function: <nnAvlAdjustTreeLong>
 *
 * Description: <Adjust to the AVL Tree>
 *
 * Parameters: pAvlTreeLong : AVL Tree have a pAvlNodeLong
               pAvlNodeLong : AVL Node to do adjust
 *
 * Returns: None
*******************************************************************************/

void nnAvlAdjustTreeLong(AvlTreeLongT *pAvlTreeLong, AvlNodeLongT *pAvlNodeLong)
{
    if (pAvlTreeLong == NULL)
    {
/*
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTreeLong Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
*/
        return;
    }
    else if (pAvlNodeLong == NULL)
    {
/*
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlNodeLong Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
*/
        return;
    }

    nnAvlUpdateBalanceLong(pAvlNodeLong);

    if (pAvlNodeLong->balance == 2) {
        if (pAvlNodeLong->pLeft->balance == 1)
        {
            AvlNodeLongT *pTempNode = NULL;

            pTempNode = nnAvlRotateRightLong(pAvlNodeLong);

            /* Select Root */
            if (pTempNode->pParent == NULL)
            {
                pAvlTreeLong->pRoot = pTempNode;
            }
        }
        else if (pAvlNodeLong->pLeft->balance == -1)
        {
            AvlNodeLongT *pTempNode = NULL;

            pAvlNodeLong->pLeft = nnAvlRotateLeftLong(pAvlNodeLong->pLeft);
            pTempNode = nnAvlRotateRightLong(pAvlNodeLong);

            /* Select Root */
            if (pTempNode->pParent == NULL)
            {
                pAvlTreeLong->pRoot = pTempNode;
            }
        }
    }
    else if (pAvlNodeLong->balance == -2)
    {
        if (pAvlNodeLong->pRight->balance == -1)
        {
            AvlNodeLongT *pTempNode = NULL;

            pTempNode = nnAvlRotateLeftLong(pAvlNodeLong);

            /* Select Root */
            if (pTempNode->pParent == NULL)
            {
                pAvlTreeLong->pRoot = pTempNode;
            }
        }
        else if (pAvlNodeLong->pRight->balance == 1)
        {
            AvlNodeLongT *pTempNode = NULL;

            pAvlNodeLong->pRight = nnAvlRotateRightLong(pAvlNodeLong->pRight);
            pTempNode = nnAvlRotateLeftLong(pAvlNodeLong);

            /* Select Root */
            if (pTempNode->pParent == NULL)
            {
                pAvlTreeLong->pRoot = pTempNode;
            }
        }

    }

    if (pAvlTreeLong->pRoot != pAvlNodeLong)
    {
        nnAvlAdjustTreeLong(pAvlTreeLong, pAvlNodeLong->pParent);
    }
}


/*******************************************************************************
 * Function: <nnAvlUpdateBalanceLong>
 *
 * Description: <Update the AVL Node's Balance>
 *
 * Parameters: pAvlNodeLong : AVL Node to do update the balance
 *
 * Returns: None
*******************************************************************************/

void nnAvlUpdateBalanceLong(AvlNodeLongT *pAvlNodeLong)
{
    if (pAvlNodeLong == NULL)
    {
/*
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlNodeLong Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
*/
        return;
    }

    pAvlNodeLong->height = nnAvlGetMaxHeightLong(nnAvlGetHeightLong(pAvlNodeLong->pLeft),
                                        nnAvlGetHeightLong(pAvlNodeLong->pRight)) + 1;
    pAvlNodeLong->balance = nnAvlGetHeightLong(pAvlNodeLong->pLeft) -
                        nnAvlGetHeightLong(pAvlNodeLong->pRight);
}


/*******************************************************************************
 * Function: <nnAvlRotateLeftLong>
 *
 * Description: <Rotate Left the AVL Tree's Nodes>
 *
 * Parameters: pAvlNodeLong : AVL Node Broken balance
 *
 * Returns: NULL : pAvlNodeLong is NULL
 *          Right Node
*******************************************************************************/

AvlNodeLongT *nnAvlRotateLeftLong(AvlNodeLongT *pAvlNodeLong)
{
    if (pAvlNodeLong == NULL)
    {
/*
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlNodeLong Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
*/
        return NULL;
    }

    AvlNodeLongT *pRightNode = NULL;
    AvlNodeLongT *pParentNode = NULL;

    pRightNode = pAvlNodeLong->pRight;
    pParentNode = pAvlNodeLong->pParent;

    if (pRightNode->pLeft != NULL)
    {
        pAvlNodeLong->pRight = pRightNode->pLeft;
        pRightNode->pLeft->pParent = pAvlNodeLong;
    }
    else
    {
        pAvlNodeLong->pRight = NULL;
    }

    pRightNode->pLeft = pAvlNodeLong;
    pAvlNodeLong->pParent = pRightNode;

    if (pParentNode != NULL)
    {
        if (pAvlNodeLong == pParentNode->pLeft)
        {
            pParentNode->pLeft = pRightNode;
        }
        else
        {
            pParentNode->pRight = pRightNode;
        }
    }

    pRightNode->pParent = pParentNode;

    nnAvlUpdateBalanceLong(pAvlNodeLong);
    nnAvlUpdateBalanceLong(pRightNode);

    return pRightNode;
}


/*******************************************************************************
 * Function: <nnAvlRotateRightLong>
 *
 * Description: <Rotate Right the AVL Tree's Nodes>
 *
 * Parameters: pAvlNodeLong : AVL Node Broken balance
 *
 * Returns: NULL : pAvlNodeLong is NULL
 *          Left Node
*******************************************************************************/

AvlNodeLongT *nnAvlRotateRightLong(AvlNodeLongT *pAvlNodeLong)
{
    if (pAvlNodeLong == NULL)
    {
/*
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlNodeLong Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
*/
        return NULL;
    }

    AvlNodeLongT *pLeftNode = NULL;
    AvlNodeLongT *pParentNode = NULL;

    pLeftNode = pAvlNodeLong->pLeft;
    pParentNode = pAvlNodeLong->pParent;

    if (pLeftNode->pRight != NULL)
    {
        pAvlNodeLong->pLeft = pLeftNode->pRight;
        pLeftNode->pRight->pParent = pAvlNodeLong;
    }
    else
    {
        pAvlNodeLong->pLeft = NULL;
    }

    pLeftNode->pRight = pAvlNodeLong;
    pAvlNodeLong->pParent = pLeftNode;

    if (pParentNode != NULL)
    {
        if (pAvlNodeLong == pParentNode->pLeft)
        {
            pParentNode->pLeft = pLeftNode;
        }
        else
        {
            pParentNode->pRight = pLeftNode;
        }
    }

    pLeftNode->pParent = pParentNode;

    nnAvlUpdateBalanceLong(pAvlNodeLong);
    nnAvlUpdateBalanceLong(pLeftNode);

    return pLeftNode;

}


/*******************************************************************************
 * Function: <nnAvlGetHeightLong>
 *
 * Description: <AVL Node brings the height>
 *
 * Parameters: pAvlNodeLong : AVL Node want to know about height
 *
 * Returns: AVL Node's height
*******************************************************************************/

Int32T nnAvlGetHeightLong(AvlNodeLongT *pAvlNodeLong)
{
    if (pAvlNodeLong == NULL)
    {
        return 0;
    }
    else
    {
        return pAvlNodeLong->height;
    }
}


/*******************************************************************************
 * Function: <nnAvlGetMaxHeightLong>
 *
 * Description: <Compare with left AVL Node height and right AVL Node height>
 *
 * Parameters: leftHeight : AVL Node compare with rightHeight
               rightHeight : AVL Node compare with leftHeight
 *
 * Returns: Larger AVL Node's height
*******************************************************************************/

Int32T nnAvlGetMaxHeightLong(Int32T leftHeight, Int32T rightHeight)
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
 * Function: <nnAvlGetMaxLeftNodeLong>
 *
 * Description: <AVL Tree brings the max left node>
 *
 * Parameters: pAvlNodeLong : AVL Tree's root node want to know
                          about left side's max node
 *
 * Returns: Left side's max node
*******************************************************************************/

AvlNodeLongT *nnAvlGetMaxLeftNodeLong(AvlNodeLongT *pAvlNodeLong)
{
    if (pAvlNodeLong == NULL)
    {
/*
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlNodeLong Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
*/
        return NULL;
    }

    AvlNodeLongT *pTempNode = pAvlNodeLong;

    while (pTempNode->pRight != NULL)
    {
        pTempNode = pTempNode->pRight;
    }

    return pTempNode;
}


/*******************************************************************************
 * Function: <nnAvlKeyCmpFuncLong>
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

Int8T nnAvlKeyCmpFuncLong(Int64T leftKey, Int64T rightKey)
{
    if (leftKey > rightKey)
    {
        return 0;
    }
    else if (leftKey < rightKey)
    {
        return 1;
    }
    else /* if (leftKey == rightKey) */
    {
        return -1;
    }
}
