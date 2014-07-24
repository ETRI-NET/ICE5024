#include "nnTypes.h"

#if !defined(_nncqueue_h)
#define _nncqueue_h


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
 * @file : nnCqueue.h 
 *
 * $Id: nnCqueue.h 831 2014-02-14 02:22:16Z lcw1127 $
 * $Author: lcw1127 $
 * $Date: 2014-02-14 11:22:16 +0900 (Fri, 14 Feb 2014) $
 * $Revision: 831 $
 * $LastChangedBy: lcw1127 $
 **/


/*******************************************************************************
*                        CONSTANTS / LITERALS / TYPES
*******************************************************************************/
#define CQUEUE_DEFAULT_COUNT 256 /**< Cqueue Default Max Count */

/* Cqueue Error Code */
#define CQUEUE_ERR_INCORRECT_INDEX  -2 /**< 잘못된 Index */
#define CQUEUE_ERR_FULL             -2 /**< Cqueue Count Full */
#define CQUEUE_ERR_CQUEUE_ALLOC     -3 /**< Cqueue Head 메모리 할당 실패 */
#define CQUEUE_ERR_CQUEUENODE_ALLOC	-4 /**< Cqueue Node 메모리 할당 실패 */
#define CQUEUE_ERR_NULL             -5 /**< Cqueue가 할당되어 있지 않음 */
#define CQUEUE_ERR_EMPTY            -6 /**< Cqueue의 Node가 비어있음 */
#define CQUEUE_ERR_DUP              -7 /**< 추가하려는 Data의 주소 값이 같음 */
#define CQUEUE_ERR_ARGUMENT         -8 /**< 잘못된 인자 값 */

/* Duplicate Option */
#define CQUEUE_DUPLICATED    0 /**< Enqueue하려는 Data 주소 값의 중복을 Check */
#define CQUEUE_NOT_DUPLICATED 1 /**< Data 주소 값의 중복 Check 하지 않음 */ 

typedef struct CqueueNodeT /**< Cqueue의 Node 구조체 */
{
    struct CqueueNodeT * next; /**< Next CqueueNode */
    struct CqueueNodeT * prev; /**< Prev CqueueNode */
    void *data;                /**< Data */

} CqueueNodeT;

typedef struct CqueueT /**< Cqueue 구조체 */
{
    struct CqueueNodeT * head; /**< First CqueueNode */
    struct CqueueNodeT * tail; /**<	Last CqueueNode */

    Uint64T count;             /**< Cqueue Node Count */
    Uint64T maxCount;          /**< Cqueue Node Max Count */
    Uint32T dataType;          /**< Cqueue의 Data Type */
    Uint8T dupCheck;           /**< Cqueue Node Duplication Check Flag */
                               /**< 0 : Duplicated, 1 : Not Duplicated */
} CqueueT;


/*******************************************************************************
*                                GLOBALS VARIABLES
*******************************************************************************/


/*******************************************************************************
 *                           GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/

CqueueT * nnCqueueInit(Uint64T maxCount, Uint8T dupCheck, Uint32T dataType);
void nnCqueueFree(CqueueT * cqueue);
void nnCqueueDeleteAllNode(CqueueT * cqueue);

Int64T nnCqueueCount(CqueueT * cqueue);
Int32T nnCqueueEnqueue(CqueueT * cqueue, void * data);
void * nnCqueueDequeue(CqueueT * cqueue);

void * nnCqueueGetHead(CqueueT * cqueue);
void * nnCqueueGetTail(CqueueT * cqueue);

#endif

