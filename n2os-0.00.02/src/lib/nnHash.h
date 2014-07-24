#include "nnTypes.h"

#if !defined(_nnhash_h)
#define _nnhash_h


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
 * @brief : N2OS Data Structure - Hash
 *  - Block Name : Library
 *  - Process Name : rLibraryibmgr
 *  - Creator : Changwoo Lee, JaeSu Han
 *  - Initial Date : 2013/7/15
*/

/**
 * @file : nnHash.h
 *
 * $Id: nnHash.h 826 2014-02-14 02:20:29Z lcw1127 $
 * $Author: lcw1127 $
 * $Date: 2014-02-14 11:20:29 +0900 (Fri, 14 Feb 2014) $
 * $Revision: 826 $
 * $LastChangedBy: lcw1127 $
 **/


/*******************************************************************************
 *                        CONSTANTS / LITERALS / TYPES
 ******************************************************************************/
/* Hash Error Code */
#define HASH_ERR_MIS_INDEX         -1  /**< Hash Index 가 잘못 되었을 때 */
#define HASH_ERR_HASH_ALLOC        -2  /**< Hash 의 메모리 할당 실패일 때 */
#define HASH_ERR_HASHBUCKET_ALLOC  -3  /**< Bucket 의 메모리 할당 실패일 때 */
#define HASH_ERR_KEY_FUNC          -4  /**< HashKey 생성 함수가 없을 때 */
#define HASH_ERR_COMP_FUNC         -5  /**< 사용자 비교 함수가 없을 때 */
#define HASH_ERR_NULL              -6  /**< 파라미터가 NULL 일 때 */
#define HASH_ERR_ARGUMENT          -7  /**< DataType 이 잘못 되었을 때 */

#define MAX_HASH_INDEX_SIZE 128 /**< 기본 Hash Index */
#define HASH_NO_DATA 0 /**< 존재하는 Data 또는 검색된 Data 가 없을 때 */

typedef struct HashBucketT /**< Hash 의 Bucket 구조체 */
{
    struct HashBucketT *next;  /**< 다음 Bucket */
    Uint32T key;               /**< HashKey 값 */
    void *data;                /**< Data */
} HashBucketT;

typedef struct HashT /**< Hash 구조체 */
{
    struct HashBucketT ** index;  /**< Bucket 을 저장하기 위한 Index */
    Uint32T size;                 /**< Hash Table Index 의 크기 */
    Uint32T dataType;             /**< Data Type */
    Uint32T (*hash_key)();        /**< HashKey 를 생성하는 함수 */
    BoolT (*hash_cmp)();          /**< Bucket 의
                                       Data 를 비교할 때 사용되는 함수 */
    Int64T count;                 /**< Bucket 의 개수 */
} HashT;


/*******************************************************************************
 *                        CONSTANTS / LITERALS / TYPES
 ******************************************************************************/


/*******************************************************************************
*                                GLOBALS VARIABLES
*******************************************************************************/


/*******************************************************************************
 *                           GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/

HashT * nnHashInit(
    Uint32T size,
    Uint32T dataType,
    Uint32T (*hash_key)(),
    BoolT (*hash_cmp)());

void nnHashFree(HashT * hash);
void * nnHashRelease(HashT * hash, void * data);
Int32T nnHashDeleteAllBucket(HashT * hash, Uint64T index);
void nnHashDeleteAllIndex(HashT * hash);
Int64T nnHashCount(HashT * hash);
void * nnHashAddBucket(HashT * hash, void * data);
void nnHashIterate(
    HashT * hash,
    void (*func) (HashBucketT *, void *),
    void *arg);
void nnHashIterate2(
    HashT * hash,
    void (*func) (HashBucketT *, void *, void *),
    void *arg1,
    void *arg2);
void nnHashIterate3(
    HashT * hash,
    void (*func) (HashBucketT *, void *, void *, void *),
    void *arg1,
    void *arg2,
    void *arg3);
#endif
