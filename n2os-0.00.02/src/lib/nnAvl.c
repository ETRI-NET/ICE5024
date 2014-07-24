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
 * @brief : N2OS Data Structure - AVL Tree
 *  - Block Name : Library
 *  - Process Name : rLibraryibmgr
 *  - Creator : Changwoo Lee, JaeSu Han
 *  - Initial Date : 2013/7/15
 */

/**
* @file : nnAvl.c
*
* $Id: nnAvl.c 958 2014-03-07 10:11:10Z lcw1127 $
* $Author: lcw1127 $
* $Date: 2014-03-07 19:11:10 +0900 (Fri, 07 Mar 2014) $
* $Revision: 958 $
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
 * Description: AvlTree 를 사용할 수 있도록 초기화 하는 함수
 *
 * @param [in] pNodeCmpFunc : AvlTree 에 담은 AvlNode 의
 *                            Data 를 비교할 사용자 함수
 * @param [in] dataType : AvlTree 에 담을 Data 의 Type
 *
 * @retval : AVL_ERR_AVL_ALLOC(-1) : AvlTree 의
 *                                   메모리 할당에 실패일 때,
 *           AVL_ERR_COMP_FUNC(-3) : 사용자 함수가 NULL 일 때,
 *           AVL_ERR_ARGUMENT(-5) : 잘못된 Data Type 일 때,
 *           초기화 된 AvlTree : 성공
 *
 * @bug
 *  pNodeCmpFunc 는 사용자가 작성한 Data 비교 함수를 사용해야 하며
 *  초기화 시에만 등록이 가능 함
 *
 * @code
 *  AvlTreeT pAvlTre = NULL;
 *
 *  nAvlTree = nnAvlInit(dataCmpFunc, TEST_TYPE);
 * @endcode
 *
 * @test
 *  pNodeCmpFunc 를 NULL 로 입력,
 *  dataType 의 값을 사용자 타입의 범위를 벗어난 값으로 입력
 */

AvlTreeT *nnAvlInit(avlCmpFuncT pNodeCmpFunc, Uint32T dataType)
{
    AvlTreeT *pNewTree = NULL;

    if (pNodeCmpFunc == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Comp Function Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (AvlTreeT *)AVL_ERR_COMP_FUNC;
    }
  
    if((dataType >= MEM_MAX_USER_TYPE && dataType < MEM_SYS_START)
       || dataType >= MEM_MAX_SYS_TYPE)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Wrong AvlTree DataType\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (AvlTreeT *)AVL_ERR_ARGUMENT;
    }

    pNewTree = (AvlTreeT *)NNMALLOC(AVL_TREE_HEAD, sizeof(AvlTreeT));

    if((Int32T)pNewTree <= 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "NewAvlTree Memory Allocate : Failure(%d)\n",
                       (Int32T)pNewTree);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (AvlTreeT *)AVL_ERR_AVL_ALLOC;
    }

    pNewTree->pRoot = NULL;
    pNewTree->count = 0;
    pNewTree->dataType = dataType;

    pNewTree->dataCmpFunc = pNodeCmpFunc;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Init NewAvlTree : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return pNewTree;
}


/**
 * Description: AvlTree 에 저장된 Data 삭제 및 메모리를 해제하는 함수
 *
 * @param [in/out] pAvlTree : 모든 Data 및 메모리를 해제 할 AvlTree
 *
 * @retval : None
 *
 * @code
 *  nnAvlFree(pAvlTree);
 * @endcode
 */

void nnAvlFree(AvlTreeT *pAvlTree)
{
    if (pAvlTree == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Avl Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    nnAvlDeleteAllNode(pAvlTree);

    NNFREE(AVL_TREE_HEAD, pAvlTree);

    pAvlTree = NULL;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_ERR, "Free AvlTree Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}


/**
 * Description: 사용자가 등록한 AvlNode 의 Data 비교 함수를 사용하여
 *              pData 를 가진 AvlNode 를 삭제하는 함수
 *
 * @param [in] pAvlTree : AvlNode 를 삭제할 AvlTree
 * @param [in] pData : 삭제할 Data
 *
 * @retval : None
 *
 * @bug
 *  pData 의 메모리는 사용자가 관리하여야 함
 *
 * @code
 *  testT pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  nnAvlDeleteNode(pAvlTree, pTest);
 * @endcode
 *
 * @test
 *  pAvlTree 를 NULL 로 입력,
 *  pAvlTree 의 사용자 비교함수를 NULL 인 상태에서 함수 수행,
 *  pData 를 NULL 로 입력,
 *  pAvlTree 에 아무런 값도 없는 상태에서 수행,
 *  pAvlTree 에 없는 값을 pData 의 값으로 입력
 */


void nnAvlDeleteNode(AvlTreeT *pAvlTree, void *pData)
{
    if (pAvlTree == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Avl Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }
    else if (pData == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    AvlNodeT *pDelNode = NULL;
    AvlNodeT *pMaxLeftNode = NULL;
    AvlNodeT *pAdjustNode = NULL;

    pDelNode = nnAvlLookupNode(pAvlTree, pData);

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
        pMaxLeftNode = nnAvlGetMaxLeftNode(pDelNode->pLeft);

        /* 삭제할 노드의 왼쪽 Max 노드가 삭제할 노드의 왼쪽 자식일 때 */
        if (pMaxLeftNode == pDelNode->pLeft)
        {
            /* 삭제할 노드가 루트가 일 때 */
            if (pDelNode == pAvlTree->pRoot)
            {
                pAvlTree->pRoot = pMaxLeftNode;
                pMaxLeftNode->pParent = NULL;
            }
            /* 삭제할 노드가 루트가 아닐 때 */
            else /* if (pDelNode != pAvlTree->pRoot) */
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
            if (pDelNode == pAvlTree->pRoot)
            {
                pAvlTree->pRoot = pMaxLeftNode;
                pMaxLeftNode->pParent = NULL;
            }
            else /* if (pDelNode != pAvlTree->pRoot) */
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
            if (pDelNode == pAvlTree->pRoot)
            {
                pAvlTree->pRoot = pDelNode->pRight;
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
        if (pAvlTree->pRoot == pDelNode)
        {
            if (pDelNode->pLeft != NULL)
            {
                pAvlTree->pRoot = pDelNode->pLeft;
                pDelNode->pLeft->pParent = NULL;

                pAdjustNode = pDelNode->pLeft;
            }
            else if (pDelNode->pRight != NULL)
            {
                pAvlTree->pRoot = pDelNode->pRight;
                pDelNode->pRight->pParent = NULL;

                pAdjustNode = pDelNode->pRight;
            }
            else
            {
                pAvlTree->pRoot = NULL;
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

    NNFREE(pAvlTree->dataType, pDelNode->pData);
    NNFREE(AVL_TREE_NODE, pDelNode);

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Delete Node : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    --pAvlTree->count;

    nnAvlAdjustTree(pAvlTree, pAdjustNode);
}


/**
 * Description: AvlTree 에 저장된 모든 AvlNode 를 삭제하는 함수
 *
 * @param [in] pAvlTree : 모든 AvlNode 를 삭제할 List
 *
 * @retval : None
 *
 * @code
 *  nnAvlDeleteAllNode(pAvlTree);
 * @endcode
 *
 * @test
 *  pAvlTree 를 NULL 로 입력,
 *  pAvlTree 에 아무런 값도 없는 상태에서 수행
 */

void nnAvlDeleteAllNode(AvlTreeT *pAvlTree)
{
    if (pAvlTree == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTree Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }
    else if (pAvlTree->pRoot == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "AvlTree Is Empty\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }
    else
    {
        AvlNodeT *pTempNode = NULL;
        AvlNodeT *pDelNode = NULL;

        for (pTempNode = pAvlTree->pRoot; pTempNode != NULL; )
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

                NNFREE(pAvlTree->dataType, pDelNode->pData);
                NNFREE(AVL_TREE_NODE, pDelNode);

                pDelNode = NULL;

                --pAvlTree->count;
            }
        }

        pAvlTree->pRoot = NULL;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Delete All Node Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}


/**
 * Description: 사용자가 등록한 AvlNode 의 Data 비교 함수를 사용하여
 *              pData 를 가진 AvlNode 를 삭제하고 그 Data 는
 *              삭제하지 않는  함수
 *
 * @param [in] pAvlTree : AvlNode 를 삭제할 AvlTree
 * @param [in] pData : 삭제할 AvlNode 를 가진 Data
 *
 * @retval : None
 *
 * @bug
 *  pData 의 메모리는 사용자가 관리하여야 함
 *
 * @code
 *  testT pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  nnAvlRemoveNode(pAvlTree, pTest);
 * @endcode
 *
 * @test
 *  pAvlTree 를 NULL 로 입력,
 *  pAvlTree 의 사용자 비교함수를 NULL 인 상태에서 함수 수행,
 *  pData 를 NULL 로 입력,
 *  pAvlTree 에 아무런 값도 없는 상태에서 수행,
 *  pAvlTree 에 없는 값을 pData 의 값으로 입력
 */


void nnAvlRemoveNode(AvlTreeT *pAvlTree, void *pData)
{
    if (pAvlTree == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Avl Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }
    else if (pData == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    AvlNodeT *pDelNode = NULL;
    AvlNodeT *pMaxLeftNode = NULL;
    AvlNodeT *pAdjustNode = NULL;

    pDelNode = nnAvlLookupNode(pAvlTree, pData);

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
        pMaxLeftNode = nnAvlGetMaxLeftNode(pDelNode->pLeft);

        /* 삭제할 노드의 왼쪽 Max 노드가 삭제할 노드의 왼쪽 자식일 때 */
        if (pMaxLeftNode == pDelNode->pLeft)
        {
            /* 삭제할 노드가 루트가 일 때 */
            if (pDelNode == pAvlTree->pRoot)
            {
                pAvlTree->pRoot = pMaxLeftNode;
                pMaxLeftNode->pParent = NULL;
            }
            /* 삭제할 노드가 루트가 아닐 때 */
            else /* if (pDelNode != pAvlTree->pRoot) */
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
            if (pDelNode == pAvlTree->pRoot)
            {
                pAvlTree->pRoot = pMaxLeftNode;
                pMaxLeftNode->pParent = NULL;
            }
            else /* if (pDelNode != pAvlTree->pRoot) */
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
            if (pDelNode == pAvlTree->pRoot)
            {
                pAvlTree->pRoot = pDelNode->pRight;
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
        if (pAvlTree->pRoot == pDelNode)
        {
            if (pDelNode->pLeft != NULL)
            {
                pAvlTree->pRoot = pDelNode->pLeft;
                pDelNode->pLeft->pParent = NULL;

                pAdjustNode = pDelNode->pLeft;
            }
            else if (pDelNode->pRight != NULL)
            {
                pAvlTree->pRoot = pDelNode->pRight;
                pDelNode->pRight->pParent = NULL;

                pAdjustNode = pDelNode->pRight;
            }
            else
            {
                pAvlTree->pRoot = NULL;
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
//  NNFREE(pAvlTree->dataType, pDelNode->pData);
    NNFREE(AVL_TREE_NODE, pDelNode);

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Delete Node : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    --pAvlTree->count;

    nnAvlAdjustTree(pAvlTree, pAdjustNode);
}


/**
 * Description: AvlTree 에 저장된 모든 AvlNode 를 삭제하며
 *              저장된 Data 는 삭제하지 않는 함수
 *
 * @param [in] pAvlTree : 모든 AvlNode 를 삭제할 List
 *
 * @retval : None
 *
 * @code
 *  nnAvlRemoveAllNode(pAvlTree);
 * @endcode
 *
 * @test
 *  pAvlTree 를 NULL 로 입력,
 *  pAvlTree 에 아무런 값도 없는 상태에서 수행
 */

void nnAvlRemoveAllNode(AvlTreeT *pAvlTree)
{
    if (pAvlTree == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTree Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }
    else if (pAvlTree->pRoot == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "AvlTree Is Empty\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }
    else
    {
        AvlNodeT *pTempNode = NULL;
        AvlNodeT *pDelNode = NULL;

        for (pTempNode = pAvlTree->pRoot; pTempNode != NULL; )
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
//              NNFREE(pAvlTree->dataType, pDelNode->pData);
                NNFREE(AVL_TREE_NODE, pDelNode);

                pDelNode = NULL;

                --pAvlTree->count;
            }
        }

        pAvlTree->pRoot = NULL;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Delete All Node Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}


/**
 * Description: AvlTree 의 최상위 Node 의 Data 를 리턴하는 함수
 *
 * @param [in] pAvlTree : 최상위 Node 의 Data 를 가져올 AvlTree
 *
 * @retval : AVL_ERR_NULL(-4) : 파라미터가 NULL 일 때,
 *           NULL : 최상위 Node 가 없을 때,
 *           최상위 Node 의 Data : 성공
 *
 * @code
 *  testT *pRet = NULL;
 *
 *  pRet = nnAvlGetRootData(pAvlTree);
 * @endcode
 *
 * @test
 *  pAvlTree 를 NULL 로 입력,
 *  pAvlTree 에 아무런 값도 없는 상태에서 수행
 */

void *nnAvlGetRootData(AvlTreeT *pAvlTree)
{
    if ((Int32T)pAvlTree <= 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTree Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *)AVL_ERR_NULL;
    }
    else if (pAvlTree->pRoot == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "AvlTree Is Empty\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *)NULL;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Get Root Node : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return pAvlTree->pRoot->pData;
}


/**
 * Description: 사용자가 등록한 AvlNode 의 Data 비교 함수를 사용하여
 *              AvlTree 에서 pData 와 같은 Data 를
 *              가진 AvlNode 를 검색해 그 Data 를 리턴하는 함수
 *
 * @param [in] pAvlTree : Data 를 검색할 AvlTree
 * @param [in] pData : 검색하려는 Data
 *
 * @retval : AVL_ERR_COMP_FUNC(-3) : 비교 함수가 등록되지 않았을 때,
 *           AVL_ERR_NULL(-4) : 파라미터가 NULL 일 때,
 *           NULL : pData 와 같은 Data 를 가진 AvlNode 가 없을 때,
 *           검색된 AvlNode 의 Data : 성공
 *
 * @code
 *  testT *pRet = NULL;
 *  testT pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  pRet = nnAvlLookupData(pAvlTree, pTest);
 * @endcode
 *
 * @test
 *  pAvlTree 를 NULL 로 입력,
 *  pAvlTree 의 사용자 비교함수를 NULL 인 상태에서 함수 수행,
 *  pData 를 NULL 로 입력,
 *  pAvlTree 에 아무런 값도 없는 상태에서 수행,
 *  pAvlTree 에 없는 값을 pData 의 값으로 입력
 */

void *nnAvlLookupData(AvlTreeT *pAvlTree, void *pData)
{
    AvlNodeT *tempNode = NULL;

    if (pAvlTree == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTree Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *)AVL_ERR_NULL;
    }

    if (pData == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *) AVL_ERR_NULL;
    }

    tempNode = nnAvlLookupNode(pAvlTree, pData);

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
 * Description: AvlTree 에 저장된 AvlNode 의 개수를 가져오는 함수
 *
 * @param [in] pAvlTree : 저장된 AvlNode 개수를 가져올 AvlTree
 *
 * @retval : AVL_ERR_NULL(-4) : 파라미터가 NULL 일 때,
 *           저장된 AvlNode 의 개수 : 성공
 *
 * @code
 *  Int64T ret = 0;
 *
 *  ret = nnAvlCount(pAvlTree);
 * @endcode
 *
 * @test
 *  pAvlTree 를 NULL 로 입력,
 *  pAvlTree 에 아무런 값도 없는 상태에서 수행
 */

Int64T nnAvlCount(AvlTreeT *pAvlTree)
{
    if (pAvlTree == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTree Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        return AVL_ERR_NULL;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Get AvlTree Count : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return pAvlTree->count;
}


/**
 * Description: Data 를 AvlNode 에 담아 AvlTree 에 저장하는 함수
 *
 * @param [in] pAvlTree : Data 를 저장할 AvlTree
 * @param [in] pData : 저장하려는 Data
 *
 * @retval : AVL_ERR_AVLNODE_ALLOC(-2) - AvlNode 의
 *                                       메모리 할당에 실패 일 때
 *           AVL_ERR_COMP_FUNC(-3) : 비교 함수가 등록되지 않았을 때,
 *           AVL_ERR_NULL(-4) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  저장 실패 시 pData 의 메모리는 사용자가 관리하여야 함
 *  사용자가 등록한 비교 함수를 통해 AvlTree 에서 저장될 위치가 결정 됨
 *
 * @code
 *  Int8T ret = 0;
 *  testT pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  ret = nnAvlAddNode(pAvlTree, pTest);
 * @endcode
 *
 * @test
 *  pAvlTree 를 NULL 로 입력,
 *  pData 를 NULL 로 입력
 */

Int8T nnAvlAddNode(AvlTreeT *pAvlTree, void *pData)
{
    if (pAvlTree == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTree Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return AVL_ERR_NULL;
    }
    else if (pData == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return AVL_ERR_NULL;
    }
    else if (pAvlTree->dataCmpFunc == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Comp Function is NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return AVL_ERR_COMP_FUNC;
    }

    Int8T compareResult = 0;

    AvlNodeT *pNewNode = (AvlNodeT *)NNMALLOC(AVL_TREE_NODE, sizeof(AvlNodeT));

    if((Int32T)pNewNode <= 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "NewNode Memory Allocate : Failure(%d)\n",
                       (Int32T)pNewNode);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return AVL_ERR_AVLNODE_ALLOC;    
    }

    pNewNode->pLeft = NULL;
    pNewNode->pRight = NULL;
    pNewNode->pParent = NULL;
    pNewNode->balance = 0;
    pNewNode->height = 1;
    pNewNode->pData = pData;

    /* No Node */
    if (pAvlTree->count == 0 && pAvlTree->pRoot == NULL)
    {
        pAvlTree->pRoot = pNewNode;
    }
    else
    {
        /* Parent Node */
        AvlNodeT *pTempNode = pAvlTree->pRoot;

        /* Find Parent Location */
        while (pTempNode != NULL)
        {
            compareResult = pAvlTree->dataCmpFunc(pNewNode->pData,
                                                    pTempNode->pData);

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

    ++pAvlTree->count;

    nnAvlAdjustTree(pAvlTree, pNewNode);

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Add Node : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: AvlTree 에서 pPrefix 와
 *              PrefixLen 및 PrefixLen 만큼 IP bit 수가 일치하는
 *              Prefix Data 가 있는지 AvlNode 를 검색 해
 *              조건에 맞는 Prefix Data 를 리턴하는 함수

 * @param [in] pAvlTree : Prefix Data 를 검색할 AvlTree
 * @param [in] pPrefix : 검색하려는 Prefix Data 조건
 *
 * @retval : AVL_ERR_NULL(-4) : 파라미터가 NULL 일 때,
 *           NULL : 조건에 맞는 Prefix Data 가 없을 때,
 *           조건에 맞는 Prefix Data : 성공
 */

PrefixT *nnAvlLookupDataExactMatch(AvlTreeT *pAvlTree, PrefixT *pPrefix)
{
    AvlNodeT *pResult = NULL;

    if (pAvlTree == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTree Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (PrefixT *)AVL_ERR_NULL;
    }
    else if (pPrefix == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Prefix Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (PrefixT *)AVL_ERR_NULL;
    }
    else if (pAvlTree->pRoot == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "AvlTree Is Empty\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return NULL;
    }

    if (pAvlTree->pRoot->pLeft != NULL)
    {
        pResult = nnAvlGetMaxLeftNode(pAvlTree->pRoot->pLeft);
    }

    if (pResult != NULL)
    {
        if (((PrefixT *)pResult->pData)->prefixLen >= pPrefix->prefixLen)
        {
            pResult = nnAvlLookupNodeExactMatch(pAvlTree->pRoot->pLeft, pPrefix);
        }
        else if (((PrefixT *)pResult->pData)->prefixLen < pPrefix->prefixLen)
        {
            pResult = nnAvlLookupNodeExactMatch(pAvlTree->pRoot->pRight, pPrefix);
        }
        else
        {
            pResult = nnAvlLookupNodeExactMatch(pAvlTree->pRoot, pPrefix);
        }
    }
    else
    {
        pResult = nnAvlLookupNodeExactMatch(pAvlTree->pRoot, pPrefix);
    }

    if (pResult == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Find Exact Match Node : Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return NULL;
    }
    else
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Find Exact Match Node : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (PrefixT *)pResult->pData;
    }
}


/**
 * Description: AvlTree 에서 pPrefix 와
 *              PrefixLen 이 같거나 작고 PrefixLen 만큼 IP bit 수가 일치하는
 *              Prefix Data 가 있는지 AvlNode 를 검색 해
 *              조건에 맞는 Prefix Data 중 가장 큰 PrefixLen 값을 가진
 *              Prefix Data 를 리턴하는 함수
 *
 * @param [in] pAvlTree : Prefix Data 를 검색할 AvlTree
 * @param [in] pPrefix : 검색하려는 Prefix Data 조건
 *
 * @retval : AVL_ERR_NULL(-4) : 파라미터가 NULL 일 때,
 *           NULL : 조건에 맞는 Prefix Data 가 없을 때,
 *           조건에 맞는 Prefix Data : 성공
 */

PrefixT *nnAvlLookupDataLongestMatch(AvlTreeT *pAvlTree, PrefixT *pPrefix)
{
    AvlNodeT *pResult = NULL;

    if (pAvlTree == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTree Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (PrefixT *)AVL_ERR_NULL;
    }
    else if (pPrefix == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Prefix Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (PrefixT *)AVL_ERR_NULL;
    }
    else if (pAvlTree->pRoot == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "AvlTree Is Empty\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return NULL;
    }

    pResult = nnAvlLookupNodeLongestMatch(pAvlTree->pRoot, pPrefix, NULL);

    if (pResult == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Find Longest Match Node : Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return NULL;
    }
    else
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Find Longest Match Node : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (PrefixT *)pResult->pData;
    }
}


/*******************************************************************************
 * Function: <nnAvlLookupNode>
 *
 * Description: <Lookup the AVL Tree>
 *
 * Parameters: pAvlTree : AVL Tree lookup the node
               pData : Want to find data
 *
 * Returns: NULL : no lookup node
 *          AVL_ERR_COMP_FUNC(-3) : comp function is NULL
 *          AVL_ERR_NULL(-4) : pAvlTree or pData is NULL
 *        lookup node
*******************************************************************************/

AvlNodeT *nnAvlLookupNode(AvlTreeT *pAvlTree, void *pData)
{
    if (pAvlTree == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTree Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (AvlNodeT *) AVL_ERR_NULL;
    }
    else if (pData == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (AvlNodeT *) AVL_ERR_NULL;
    }
    else if (pAvlTree->dataCmpFunc == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Comp Function is NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (AvlNodeT *) AVL_ERR_COMP_FUNC;
    }
    else if (pAvlTree->pRoot == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "AvlTree Is Empty\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return NULL;
    }

    AvlNodeT *pTempNode = NULL;

    for (pTempNode = pAvlTree->pRoot; pTempNode != NULL; )
    {
        /* Smaller than Node Data*/
        if (pAvlTree->dataCmpFunc(pTempNode->pData, pData) == 0)
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
        /* Larger than Node Data*/
        else if (pAvlTree->dataCmpFunc(pTempNode->pData, pData) == 1)
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
        else if (pAvlTree->dataCmpFunc(pTempNode->pData, pData) == -1)
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
 * Function: <nnAvlAdjustTree>
 *
 * Description: <Adjust to the AVL Tree>
 *
 * Parameters: pAvlTree : AVL Tree have a pAvlNode
               pAvlNode : AVL Node to do adjust
 *
 * Returns: None
*******************************************************************************/

void nnAvlAdjustTree(AvlTreeT *pAvlTree, AvlNodeT *pAvlNode)
{
    if (pAvlTree == NULL)
    {
/*
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlTree Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
*/
        return;
    }
    else if (pAvlNode == NULL)
    {
/*
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlNode Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
*/
        return;
    }

    nnAvlUpdateBalance(pAvlNode);

    if (pAvlNode->balance == 2) {
        if (pAvlNode->pLeft->balance == 1)
        {
            AvlNodeT *pTempNode = NULL;

            pTempNode = nnAvlRotateRight(pAvlNode);

            /* Select Root */
            if (pTempNode->pParent == NULL)
            {
                pAvlTree->pRoot = pTempNode;
            }
        }
        else if (pAvlNode->pLeft->balance == -1)
        {
            AvlNodeT *pTempNode = NULL;

            pAvlNode->pLeft = nnAvlRotateLeft(pAvlNode->pLeft);
            pTempNode = nnAvlRotateRight(pAvlNode);

            /* Select Root */
            if (pTempNode->pParent == NULL)
            {
                pAvlTree->pRoot = pTempNode;
            }
        }
    }
    else if (pAvlNode->balance == -2)
    {
        if (pAvlNode->pRight->balance == -1)
        {
            AvlNodeT *pTempNode = NULL;

            pTempNode = nnAvlRotateLeft(pAvlNode);

            /* Select Root */
            if (pTempNode->pParent == NULL)
            {
                pAvlTree->pRoot = pTempNode;
            }
        }
        else if (pAvlNode->pRight->balance == 1)
        {
            AvlNodeT *pTempNode = NULL;

            pAvlNode->pRight = nnAvlRotateRight(pAvlNode->pRight);
            pTempNode = nnAvlRotateLeft(pAvlNode);

            /* Select Root */
            if (pTempNode->pParent == NULL)
            {
                pAvlTree->pRoot = pTempNode;
            }
        }

    }

    if (pAvlTree->pRoot != pAvlNode)
    {
        nnAvlAdjustTree(pAvlTree, pAvlNode->pParent);
    }
}


/*******************************************************************************
 * Function: <nnAvlUpdateBalance>
 *
 * Description: <Update the AVL Node's Balance>
 *
 * Parameters: pAvlNode : AVL Node to do update the balance
 *
 * Returns: None
*******************************************************************************/

void nnAvlUpdateBalance(AvlNodeT *pAvlNode)
{
    if (pAvlNode == NULL)
    {
/*
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlNode Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
*/
        return;
    }

    pAvlNode->height = nnAvlGetMaxHeight(nnAvlGetHeight(pAvlNode->pLeft),
                                        nnAvlGetHeight(pAvlNode->pRight)) + 1;
    pAvlNode->balance = nnAvlGetHeight(pAvlNode->pLeft) -
                        nnAvlGetHeight(pAvlNode->pRight);
}


/*******************************************************************************
 * Function: <nnAvlRotateLeft>
 *
 * Description: <Rotate Left the AVL Tree's Nodes>
 *
 * Parameters: pAvlNode : AVL Node Broken balance
 *
 * Returns: NULL : pAvlNode is NULL
 *          Right Node
*******************************************************************************/

AvlNodeT *nnAvlRotateLeft(AvlNodeT *pAvlNode)
{
    if (pAvlNode == NULL)
    {
/*
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlNode Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
*/
        return NULL;
    }

    AvlNodeT *pRightNode = NULL;
    AvlNodeT *pParentNode = NULL;

    pRightNode = pAvlNode->pRight;
    pParentNode = pAvlNode->pParent;

    if (pRightNode->pLeft != NULL)
    {
        pAvlNode->pRight = pRightNode->pLeft;
        pRightNode->pLeft->pParent = pAvlNode;
    }
    else
    {
        pAvlNode->pRight = NULL;
    }

    pRightNode->pLeft = pAvlNode;
    pAvlNode->pParent = pRightNode;

    if (pParentNode != NULL)
    {
        if (pAvlNode == pParentNode->pLeft)
        {
            pParentNode->pLeft = pRightNode;
        }
        else
        {
            pParentNode->pRight = pRightNode;
        }
    }

    pRightNode->pParent = pParentNode;

    nnAvlUpdateBalance(pAvlNode);
    nnAvlUpdateBalance(pRightNode);

    return pRightNode;
}


/*******************************************************************************
 * Function: <nnAvlRotateRight>
 *
 * Description: <Rotate Right the AVL Tree's Nodes>
 *
 * Parameters: pAvlNode : AVL Node Broken balance
 *
 * Returns: NULL : pAvlNode is NULL
 *          Left Node
*******************************************************************************/

AvlNodeT *nnAvlRotateRight(AvlNodeT *pAvlNode)
{
    if (pAvlNode == NULL)
    {
/*
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlNode Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
*/
        return NULL;
    }

    AvlNodeT *pLeftNode = NULL;
    AvlNodeT *pParentNode = NULL;

    pLeftNode = pAvlNode->pLeft;
    pParentNode = pAvlNode->pParent;

    if (pLeftNode->pRight != NULL)
    {
        pAvlNode->pLeft = pLeftNode->pRight;
        pLeftNode->pRight->pParent = pAvlNode;
    }
    else
    {
        pAvlNode->pLeft = NULL;
    }

    pLeftNode->pRight = pAvlNode;
    pAvlNode->pParent = pLeftNode;

    if (pParentNode != NULL)
    {
        if (pAvlNode == pParentNode->pLeft)
        {
            pParentNode->pLeft = pLeftNode;
        }
        else
        {
            pParentNode->pRight = pLeftNode;
        }
    }

    pLeftNode->pParent = pParentNode;

    nnAvlUpdateBalance(pAvlNode);
    nnAvlUpdateBalance(pLeftNode);

    return pLeftNode;

}


/*******************************************************************************
 * Function: <nnAvlGetHeight>
 *
 * Description: <AVL Node brings the height>
 *
 * Parameters: pAvlNode : AVL Node want to know about height
 *
 * Returns: AVL Node's height
*******************************************************************************/

Int32T nnAvlGetHeight(AvlNodeT *pAvlNode)
{
    if (pAvlNode == NULL)
    {
        return 0;
    }
    else
    {
        return pAvlNode->height;
    }
}


/*******************************************************************************
 * Function: <nnAvlGetMaxHeight>
 *
 * Description: <Compare with left AVL Node height and right AVL Node height>
 *
 * Parameters: leftHeight : AVL Node compare with rightHeight
               rightHeight : AVL Node compare with leftHeight
 *
 * Returns: Larger AVL Node's height
*******************************************************************************/

Int32T nnAvlGetMaxHeight(Int32T leftHeight, Int32T rightHeight)
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
 * Function: <nnAvlGetMaxLeftNode>
 *
 * Description: <AVL Tree brings the max left node>
 *
 * Parameters: pAvlNode : AVL Tree's root node want to know
                          about left side's max node
 *
 * Returns: Left side's max node
*******************************************************************************/

AvlNodeT *nnAvlGetMaxLeftNode(AvlNodeT *pAvlNode)
{
    if (pAvlNode == NULL)
    {
/*
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "AvlNode Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
*/
        return NULL;
    }

    AvlNodeT *pTempNode = pAvlNode;

    while (pTempNode->pRight != NULL)
    {
        pTempNode = pTempNode->pRight;
    }

    return pTempNode;
}


/*******************************************************************************
 * Function: <nnAvlLookupNodeExactMatch>
 *
 * Description: <Lookup the exact match node>
 *
 * Parameters: pAvlTree : AVL Tree lookup the node
               pData : Want to find data
 *
 * Returns: NULL : no lookup data
 *          Lookup Node's Data
*******************************************************************************/

AvlNodeT *nnAvlLookupNodeExactMatch(AvlNodeT *pAvlNode, PrefixT *pPrefix)
{
    AvlNodeT *pResultNode = NULL;
    PrefixT *pNodePrefix = NULL;
    Int8T nodeComp = 0;
    Int8T leftComp = 1;
    Int8T rightComp = 1;
    Int8T ret = 0;

    if (pAvlNode != NULL)
    {
        pNodePrefix = (PrefixT *)pAvlNode->pData;

        nodeComp = (pNodePrefix->prefixLen == pPrefix->prefixLen) ? 1 : 0;

        if (pAvlNode->pLeft != NULL)
        {
            leftComp = (((PrefixT *)pAvlNode->pLeft->pData)->prefixLen
                            <= pPrefix->prefixLen)
                            ? 1 : 0;

            if (leftComp == 0)
            {
                leftComp = (((PrefixT *)pAvlNode->pLeft->pData)->prefixLen
                            > pPrefix->prefixLen)
                            ? 1 : 0;
            }
        }

        if (pAvlNode->pRight != NULL)
        {
            rightComp = (((PrefixT *)pAvlNode->pRight->pData)->prefixLen
                            <= pPrefix->prefixLen)
                            ? 1 : 0;

            if (rightComp == 0)
            {
                rightComp = (((PrefixT *)pAvlNode->pRight->pData)->prefixLen
                            > pPrefix->prefixLen)
                            ? 1 : 0;
            }
        }

        if (nodeComp)
        {
            ret = nnAvlCheckMatch(pNodePrefix, pPrefix);

            if (ret == 1)
            {
                if (pPrefix->prefixLen == pNodePrefix->prefixLen)
                {
                    pResultNode = pAvlNode;
                    return pResultNode;
                }
            }
        }

        if (leftComp)
        {
            pResultNode
                = nnAvlLookupNodeExactMatch(pAvlNode->pLeft, pPrefix);
        }

        if (rightComp)
        {
            pResultNode
                = nnAvlLookupNodeExactMatch(pAvlNode->pRight, pPrefix);
        }
    }

    return pResultNode;
}


/*******************************************************************************
 * Function: <nnAvlLookupNodeLongestMatch>
 *
 * Description: <Lookup the longest match node>
 *
 * Parameters: pAvlTree : AVL Tree lookup the node
               pData : Want to find data
 *
 * Returns: NULL : no lookup data
 *          Lookup Node's Data
*******************************************************************************/

AvlNodeT *nnAvlLookupNodeLongestMatch(AvlNodeT *pAvlNode, PrefixT *pPrefix,
        AvlNodeT *pMatchNode)
{
    AvlNodeT *pResultNode = NULL;
    PrefixT *pNodePrefix = NULL;
    Int8T nodeComp = 0;
    Int8T matchComp = 1;
    Int8T leftComp = 1;
    Int8T ret = 0;

    Int8T ip[16] = {0,};
    PrefixT *pTempPrefix = NULL;

    pResultNode = pMatchNode;

    if (pAvlNode != NULL)
    {
        pNodePrefix = (PrefixT *)pAvlNode->pData;

        nodeComp = (pNodePrefix->prefixLen <= pPrefix->prefixLen) ? 1 : 0;

        if (pMatchNode != NULL)
        {
            matchComp = (pNodePrefix->prefixLen
                            < ((PrefixT *)pMatchNode->pData)->prefixLen)
                            ? 0 : 1;
        }

        if (pAvlNode->pLeft == NULL)
        {
            leftComp = 0;
        }
        if (pMatchNode != NULL && pAvlNode->pLeft != NULL)
        {
            leftComp = (((PrefixT *)pAvlNode->pLeft->pData)->prefixLen
                            < ((PrefixT *)pMatchNode->pData)->prefixLen)
                            ? 0 : 1;
        }

        if (nodeComp && matchComp)
        {
            ret = nnAvlCheckMatch(pNodePrefix, pPrefix);
        }

        if (ret == 1)
        {
            pResultNode = pAvlNode;
        }

        pTempPrefix = pNodePrefix;
        nnCnvPrefixtoString(ip, pTempPrefix);

        if (nodeComp)
        {
            pResultNode
                = nnAvlLookupNodeLongestMatch(
                        pAvlNode->pRight, pPrefix, pResultNode);
        }

        if (leftComp)
        {
            pResultNode
                = nnAvlLookupNodeLongestMatch(
                        pAvlNode->pLeft, pPrefix, pResultNode);
        }
    }

    return pResultNode;
}


/*******************************************************************************
 * Function: <nnAvlCmpPrefixFunc>
 *
 * Description: <Compare with two PrefixT structures,
                  which prefixLen is bigger, Prefix Only>
 *
 * Parameters: pLeftPrefix : Left prefix data(PrefixT structure)
               pRightPrefix : Right prefix data(PrefixT structure)
 *
 * Returns: 0 : Left prefixLen is bigger than Right prefixLen
 *          1 : Right prefixLen is bigger than Left prefixLen
 *         -1 : Same two prefixLen
*******************************************************************************/

Int8T nnAvlCmpPrefixFunc(PrefixT *pLeftData, PrefixT *pRightData)
{
    /* if sort then ASC */
    if (pLeftData->prefixLen > pRightData->prefixLen)
    {
        return 0;
    }
    /* if sort then DESC */
    else if (pLeftData->prefixLen < pRightData->prefixLen)
    {
        return 1;
    }
    /* same data */
    else /* if (pLeftData->prefixLen == pRightData->prefixLen) */
    {
        return -1;
    }
}


/*******************************************************************************
 * Function: <nnAvlCheckMatch>
 *
 * Description: <Compare with two PrefixT structure
                 with pNodePrefix1's prefixLen>
 *
 * Parameters: pNodePrefix : AvlNodeT's pData(PrefixT structure)
               pUserPrefix : User's Data(PrefixT structure)
 *
 * Returns: 1 : Not matched
 *          0 : matched
*******************************************************************************/

Int8T nnAvlCheckMatch(PrefixT *pNodePrefix, PrefixT *pUserPrefix)
{
    Uint8T *pNodeIp = NULL, *pUserIp = NULL;
    Int32T offset = 0;
    Int32T shift = 0;

    if (pNodePrefix->prefixLen > pUserPrefix->prefixLen)
    {
        return 0;
    }

    pNodeIp = (Uint8T *)&pNodePrefix->u.prefix4.s_addr;
    pUserIp = (Uint8T *)&pUserPrefix->u.prefix4.s_addr;

    offset = pNodePrefix->prefixLen / AVL_COMPARE_IP;
    shift = pNodePrefix->prefixLen % AVL_COMPARE_IP;

    if (shift) {
        if (gMaskbit[shift] & (pNodeIp[offset] ^ pUserIp[offset])) {
            return 0;
        }
    }

    while (offset--) {
        if (pNodeIp[offset] != pUserIp[offset]) {
            return 0;
        }
    }

    return 1;
}
