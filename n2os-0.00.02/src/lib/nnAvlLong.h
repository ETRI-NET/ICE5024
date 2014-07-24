#include "nnTypes.h"

#if !defined(_nnAvlLong_h)
#define _nnAvlLong_h


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
 * @file : nnAvlLong.h
 *
 * $Id: nnAvl.h 887 2014-02-20 03:47:32Z lcw1127 $
 * $Author: lcw1127 $
 * $Date: 2014-02-20 12:47:32 +0900 (2014-02-20, 목) $
 * $Revision: 887 $
 * $LastChangedBy: lcw1127 $
**/


/*******************************************************************************
 *                        CONSTANTS / LITERALS / TYPES
 ******************************************************************************/
typedef struct AvlNodeLongT /**< AvlTree 의 Node 구조체 */
{
    struct AvlNodeLongT *pLeft;    /**< 왼쪽 AvlNode */
    struct AvlNodeLongT *pRight;   /**< 오른쪽 AvlNode */
    struct AvlNodeLongT *pParent;  /**< 부모 AvlNode */

    Int64T key;        /**< Key 값 */

    Int32T balance;    /**< Tree Balance(왼쪽 AvlNode 높이 -
                                         오른쪽 AvlNode 높이) */
    Int32T height;     /**< Tree 높이 (Leaf Node = 1) */
    void *pData;       /**< Data */
} AvlNodeLongT;

typedef struct AvlTreeLongT /**< AvlTree 구조체 */
{
    struct AvlNodeLongT *pRoot;   /**< 최상위 AvlNode */

    Int64T count;                 /**< AvlNode 의 개수 */
    Uint32T dataType;             /**< Data Type */
} AvlTreeLongT;

/*******************************************************************************
*                                GLOBALS VARIABLES
*******************************************************************************/
/* AvlTree Error Code */
#define AVL_LONG_ERR_AVL_ALLOC     -1 /**< AvlTree 의 메모리 할당 실패일 때 */
#define AVL_LONG_ERR_AVLNODE_ALLOC -2 /**< AvlNode 의 메모리 할당 실패일 때 */
#define AVL_LONG_ERR_NULL          -3 /**< 파라미터가 NULL 일 때 */
#define AVL_LONG_ERR_ARGUMENT      -4 /**< DataType 이 잘못 되었을 때 */

/*******************************************************************************
 *                           GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/
/* User Method */
AvlTreeLongT *nnAvlInitLong(Uint32T dataType);
void nnAvlFreeLong(AvlTreeLongT *pAvlTreeLong);
void nnAvlDeleteNodeLong(AvlTreeLongT *pAvlTreeLong, Int64T key);
void nnAvlDeleteAllNodeLong(AvlTreeLongT *pAvlTreeLong);
void nnAvlRemoveNodeLong(AvlTreeLongT *pAvlTreeLong, Int64T key);
void nnAvlRemoveAllNodeLong(AvlTreeLongT *pAvlTreeLong);
void *nnAvlGetRootDataLong(AvlTreeLongT *pAvlTreeLong);
void *nnAvlLookupDataLong(AvlTreeLongT *pAvlTreeLong, Int64T key);
Int64T nnAvlCountLong(AvlTreeLongT *pAvlTreeLong);
Int8T nnAvlAddNodeLong(AvlTreeLongT *pAvlTreeLong, Int64T key, void *pData);

/* Inner Method */
AvlNodeLongT *nnAvlLookupNodeLong(AvlTreeLongT *pAvlTreeLong, Int64T key);

void nnAvlAdjustTreeLong(AvlTreeLongT *pAvlTreeLong, AvlNodeLongT *pAvlNodeLong);
void nnAvlUpdateBalanceLong(AvlNodeLongT *pAvlNodeLong);

AvlNodeLongT *nnAvlRotateLeftLong(AvlNodeLongT *pAvlNodeLong);
AvlNodeLongT *nnAvlRotateRightLong(AvlNodeLongT *pAvlNodeLong);

Int32T nnAvlGetHeightLong(AvlNodeLongT *pAvlNodeLong);
Int32T nnAvlGetMaxHeightLong(Int32T leftHeight, Int32T rightHeight);
AvlNodeLongT *nnAvlGetMaxLeftNodeLong(AvlNodeLongT *pAvlNodeLong);

Int8T nnAvlKeyCmpFuncLong(Int64T leftKey, Int64T rightKey);

#endif
