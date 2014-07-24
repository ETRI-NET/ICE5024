#include "nnTypes.h"

#if !defined(_nnAvlStr_h)
#define _nnAvlStr_h


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
 * @file : nnAvlStr.h
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
typedef struct AvlNodeStrT /**< AvlTree 의 Node 구조체 */
{
    struct AvlNodeStrT *pLeft;    /**< 왼쪽 AvlNode */
    struct AvlNodeStrT *pRight;   /**< 오른쪽 AvlNode */
    struct AvlNodeStrT *pParent;  /**< 부모 AvlNode */

    Int8T key[64];        /**< Key 값 */

    Int32T balance;    /**< Tree Balance(왼쪽 AvlNode 높이 -
                                         오른쪽 AvlNode 높이) */
    Int32T height;     /**< Tree 높이 (Leaf Node = 1) */
    void *pData;       /**< Data */
} AvlNodeStrT;

typedef struct AvlTreeStrT /**< AvlTree 구조체 */
{
    struct AvlNodeStrT *pRoot;   /**< 최상위 AvlNode */

    Int64T count;                 /**< AvlNode 의 개수 */
    Uint32T dataType;             /**< Data Type */
} AvlTreeStrT;

/*******************************************************************************
*                                GLOBALS VARIABLES
*******************************************************************************/
/* AvlTree Error Code */
#define AVL_STR_ERR_AVL_ALLOC     -1 /**< AvlTree 의 메모리 할당 실패일 때 */
#define AVL_STR_ERR_AVLNODE_ALLOC -2 /**< AvlNode 의 메모리 할당 실패일 때 */
#define AVL_STR_ERR_NULL          -3 /**< 파라미터가 NULL 일 때 */
#define AVL_STR_ERR_ARGUMENT      -4 /**< DataType 이 잘못 되었을 때 */

/*******************************************************************************
 *                           GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/
/* User Method */
AvlTreeStrT *nnAvlInitStr(Uint32T dataType);
void nnAvlFreeStr(AvlTreeStrT *pAvlTreeStr);
void nnAvlDeleteNodeStr(AvlTreeStrT *pAvlTreeStr, StringT key);
void nnAvlDeleteAllNodeStr(AvlTreeStrT *pAvlTreeStr);
void nnAvlRemoveNodeStr(AvlTreeStrT *pAvlTreeStr, StringT key);
void nnAvlRemoveAllNodeStr(AvlTreeStrT *pAvlTreeStr);
void *nnAvlGetRootDataStr(AvlTreeStrT *pAvlTreeStr);
void *nnAvlLookupDataStr(AvlTreeStrT *pAvlTreeStr, StringT key);
Int64T nnAvlCountStr(AvlTreeStrT *pAvlTreeStr);
Int8T nnAvlAddNodeStr(AvlTreeStrT *pAvlTreeStr, StringT key, void *pData);

/* Inner Method */
AvlNodeStrT *nnAvlLookupNodeStr(AvlTreeStrT *pAvlTreeStr, StringT key);

void nnAvlAdjustTreeStr(AvlTreeStrT *pAvlTreeStr, AvlNodeStrT *pAvlNodeStr);
void nnAvlUpdateBalanceStr(AvlNodeStrT *pAvlNodeStr);

AvlNodeStrT *nnAvlRotateLeftStr(AvlNodeStrT *pAvlNodeStr);
AvlNodeStrT *nnAvlRotateRightStr(AvlNodeStrT *pAvlNodeStr);

Int32T nnAvlGetHeightStr(AvlNodeStrT *pAvlNodeStr);
Int32T nnAvlGetMaxHeightStr(Int32T leftHeight, Int32T rightHeight);
AvlNodeStrT *nnAvlGetMaxLeftNodeStr(AvlNodeStrT *pAvlNodeStr);

Int8T nnAvlKeyCmpFuncStr(StringT leftKey, StringT rightKey);

#endif
