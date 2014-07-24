#include "nnTypes.h"
#include "nnPrefix.h"
#include "nnAvlStr.h"
#include "nnAvlLong.h"
//#include "nnLog.h"

#if !defined(_nnAvl_h)
#define _nnAvl_h


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
 * @file : nnAvl.h
 *
 * $Id: nnAvl.h 1055 2014-03-25 02:00:15Z lcw1127 $
 * $Author: lcw1127 $
 * $Date: 2014-03-25 11:00:15 +0900 (Tue, 25 Mar 2014) $
 * $Revision: 1055 $
 * $LastChangedBy: lcw1127 $
**/


/*******************************************************************************
 *                        CONSTANTS / LITERALS / TYPES
 ******************************************************************************/
typedef Int8T (*avlCmpFuncT)(
    void *pOldData,
    void *pNewData);    /**< 사용자 비교 함수를 가리킬 함수 포인터 */

typedef struct AvlNodeT /**< AvlTree 의 Node 구조체 */
{
    struct AvlNodeT *pLeft;    /**< 왼쪽 AvlNode */
    struct AvlNodeT *pRight;   /**< 오른쪽 AvlNode */
    struct AvlNodeT *pParent;  /**< 부모 AvlNode */

    Int32T balance;    /**< Tree Balance(왼쪽 AvlNode 높이 -
                                         오른쪽 AvlNode 높이) */
    Int32T height;     /**< Tree 높이 (Leaf Node = 1) */
    void *pData;       /**< Data */
} AvlNodeT;

typedef struct AvlTreeT /**< AvlTree 구조체 */
{
    struct AvlNodeT *pRoot;   /**< 최상위 AvlNode */

    Int64T count;             /**< AvlNode 의 개수 */
    Int32T dataType;          /**< Data Type */

    avlCmpFuncT dataCmpFunc;  /**< AvlNode 의
                                   Data 를 비교햘 때 사용되는 함수 */
} AvlTreeT;

/*******************************************************************************
*                                GLOBALS VARIABLES
*******************************************************************************/
/* AvlTree Error Code */
#define AVL_ERR_AVL_ALLOC	-1 /**< AvlTree 의 메모리 할당 실패일 때 */
#define AVL_ERR_AVLNODE_ALLOC	-2 /**< AvlNode 의 메모리 할당 실패일 때 */
#define AVL_ERR_COMP_FUNC	-3 /**< 사용자 비교 함수가 없을 때 */
#define AVL_ERR_NULL		-4 /**< 파라미터가 NULL 일 때 */
#define AVL_ERR_ARGUMENT        -5 /**< DataType 이 잘못 되었을 때 */

#define AVL_COMPARE_IP           8 /**< 비교할 IP Bit 수 */

/*******************************************************************************
 *                           GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/
/* User Method */
AvlTreeT *nnAvlInit(avlCmpFuncT pNodeCmpFunc, Uint32T dataType);
void nnAvlFree(AvlTreeT *pAvlTree);
void nnAvlDeleteNode(AvlTreeT *pAvlTree, void *pData);
void nnAvlDeleteAllNode(AvlTreeT *pAvlTree);
void nnAvlRemoveNode(AvlTreeT *pAvlTree, void *pData);
void nnAvlRemoveAllNode(AvlTreeT *pAvlTree);
void *nnAvlGetRootData(AvlTreeT *pAvlTree);
void *nnAvlLookupData(AvlTreeT *pAvlTree, void *pData);
Int64T nnAvlCount(AvlTreeT *pAvlTree);
Int8T nnAvlAddNode(AvlTreeT *pAvlTree, void *pData);

PrefixT *nnAvlLookupDataExactMatch(AvlTreeT *pAvlTree, PrefixT *pData);
PrefixT *nnAvlLookupDataLongestMatch(AvlTreeT *pAvlTree, PrefixT *pData);

/* Inner Method */
Int8T nnAvlCmpPrefixFunc(PrefixT *pLeftData, PrefixT *pRightData);
AvlNodeT *nnAvlLookupNode(AvlTreeT *pAvlTree, void *pData);

void nnAvlAdjustTree(AvlTreeT *pAvlTree, AvlNodeT *pAvlNode);
void nnAvlUpdateBalance(AvlNodeT *pAvlNode);

AvlNodeT *nnAvlRotateLeft(AvlNodeT *pAvlNode);
AvlNodeT *nnAvlRotateRight(AvlNodeT *pAvlNode);

Int32T nnAvlGetHeight(AvlNodeT *pAvlNode);
Int32T nnAvlGetMaxHeight(Int32T leftHeight, Int32T rightHeight);
AvlNodeT *nnAvlGetMaxLeftNode(AvlNodeT *pAvlNode);

AvlNodeT *nnAvlLookupNodeExactMatch(AvlNodeT *pAvlNode, PrefixT *pPrefix);
AvlNodeT *nnAvlLookupNodeLongestMatch(AvlNodeT *pAvlNode, PrefixT *pPrefix,
        AvlNodeT *pMatchNode);
Int8T nnAvlCheckMatch(PrefixT *pNodePrefix, PrefixT *pUserPrefix);

#endif
