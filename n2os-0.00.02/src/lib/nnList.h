#include "nnTypes.h"

#if !defined(_nnList_h)
#define _nnList_h


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
 * @file : nnList.h
 *
 * $Id: nnList.h 827 2014-02-14 02:20:45Z lcw1127 $
 * $Author: lcw1127 $
 * $Date: 2014-02-14 11:20:45 +0900 (Fri, 14 Feb 2014) $
 * $Revision: 827 $
 * $LastChangedBy: lcw1127 $
 **/


/*******************************************************************************
 *                        CONSTANTS / LITERALS / TYPES
 ******************************************************************************/
/* List Error Code */
#define LIST_ERR_LIST_ALLOC           -1  /**< List 의 메모리 할당 실패일 때 */
#define LIST_ERR_LISTNODE_ALLOC       -2  /**< ListNode 의
                                               메모리 할당 실패일 때 */

#define LIST_ERR_NULL                 -3  /**< 파라미터가 NULL 일 때 */
#define LIST_ERR_DUP                  -4  /**< 중복된 Data 가 있을 때 */
#define LIST_ERR_COMP_FUNC            -5  /**< 사용자 비교 함수가 없을 때 */

#define LIST_ERR_LISTSUB_ALLOC        -6  /**< ListSub 의
                                               메모리 할당 실패일 때 */
#define LIST_ERR_LISTSUBNODE_ALLOC    -7  /**< ListSubNode 의
                                               메모리 할당 실패일 때 */

#define LIST_ERR_ARGUMENT             -8  /**< DataType 이 잘못 되었을 때 */

typedef Int8T (*listCmpFuncT)(
    void *pOldData,
    void *pNewData);    /**< 사용자 비교 함수를 가리킬 함수 포인터 */

typedef struct ListSubT /**< ListNode 의 ListSub 구조체 */
{
    struct ListSubNodeT *pHead;  /**< 첫 번째 ListSubNode */
    struct ListSubNodeT *pTail;  /**< 마지막 ListSubNode */
    Int64T count;                /**< ListSubNode 의 개수 */
    Uint32T dataType;            /**< Data Type */

    listCmpFuncT dataCmpFunc;    /**< ListSubNode 의
                                      Data 를 비교할 때 사용되는 함수 */
} ListSubT;

typedef struct ListSubNodeT /**< ListSub 의 Node 구조체*/
{
    struct ListSubNodeT *pPrev;  /**< 이전 ListSubNode */
    struct ListSubNodeT *pNext;  /**< 다음 ListSubNode */

    void *pData;                 /**< Data */
} ListSubNodeT;

typedef struct ListT /**< List 구조체 */
{
    struct ListNodeT *pHead;  /**< 첫 번째 ListNode */
    struct ListNodeT *pTail;  /**< 마지막 ListNode */
    Int64T count;             /**< ListNode 의 개수 */
    Uint32T dataType;         /**< Data Type */

    listCmpFuncT dataCmpFunc; /**< ListNode 의
                                   Data 를 비교할 때 사용되는 함수 */
} ListT;

typedef struct ListNodeT /**< List 의 Node 구조체 */
{
    struct ListNodeT *pPrev;  /**< 이전 ListNode */
    struct ListNodeT *pNext;  /**< 다음 ListNode */

    void *pData;              /**< Data */

    ListSubT *pSubHead;       /**< ListSubNode 를 저장할 구조체 */
} ListNodeT;


/*******************************************************************************
*                                GLOBALS VARIABLES
*******************************************************************************/


/*******************************************************************************
 *                           GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/

ListT *nnListInit(listCmpFuncT pNodeCmpFunc, Uint32T dataType);
void nnListFree(ListT *pList);

void nnListDeleteNode(ListT *pList, void *pData);
void nnListDeleteAllNode(ListT *pList);

void nnListRemoveNode(ListT *pList, void *pData);
void nnListRemoveAllNode(ListT *pList);

ListNodeT *nnListGetHeadNode(ListT *pList);
ListNodeT *nnListGetTailNode(ListT *pList);
Int64T nnListCount(ListT *pList);
Int8T nnListAddNode(ListT *pList, void *pData);

Int8T nnListAddNodePrev(ListT *pList, ListNodeT *pNode, void *pData);

Int8T nnListAddNodeNext(ListT *pList, ListNodeT *pNode, void *pData);

Int8T nnListAddNodeHead(ListT *pList, void *pData);

Int8T nnListAddNodeDupNot(ListT *pList, void *pData);
Int8T nnListCheckDupData(ListT *pList, void *pData);

Int8T nnListAddNodeSort(ListT *pList, void *data);
ListNodeT *nnListSearchNode (ListT *pList, void *pData);

Int8T nnListSetNodeCompFunc(ListT *pList, listCmpFuncT pNodeCmpFunc);

/* SubNode */
Int8T nnListSubInit(ListNodeT *pListNode,
    listCmpFuncT pSubNodeCmpFunc,
    Uint32T dataType);

void nnListDeleteSubNode(ListNodeT *pListNode, void *pData);
void nnListDeleteAllSubNode(ListNodeT *pListNode);
Int8T nnListAddSubNode(ListNodeT *pListNode, void *pData);
Int8T nnListAddSubNodeHead(ListNodeT *pListNode, void *pData);

ListSubNodeT *nnListSearchSubNode (ListNodeT *pListNode, void *pData);

ListSubNodeT *nnListGetHeadSubNode(ListNodeT *pListNode);
Int64T nnListSubCount(ListNodeT *pListNode);
Int8T nnListSetSubNodeCompFunc(
    ListNodeT *pListNode,
    listCmpFuncT pSubNodeCmpFunc);

#endif
