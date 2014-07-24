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
* @file : nnHash.c
*
* $Id: nnHash.c 957 2014-03-07 10:10:31Z lcw1127 $
* $Author: lcw1127 $
* $Date: 2014-03-07 19:10:31 +0900 (Fri, 07 Mar 2014) $
* $Revision: 957 $
* $LastChangedBy: lcw1127 $
**/

/*******************************************************************************
 *                               INCLUDE FILES
 ******************************************************************************/
//#include "nnMemmgr.h"
//#include "nnLog.h"
#include "nnHash.h"
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
 * Description: Hash 를 사용할 수 있도록 초기화 하는 함수
 *
 * @param [in] size : 사용할 Hash Table Index 의 크기
 * @param [in] dataType : Hash 에 담을 Data 의 Type
 * @param [in] hash_key : HashKey 를 생성할 사용자 함수
 * @param [in] hash_cmp : Hash 에 담은 Bucket 의
 *                        Data 를 비교할 사용자 함수
 *
 * @retval : HASH_ERR_HASH_ALLOC(-2) : Hash 의 메모리 할당에 실패일 때,
 *           HASH_ERR_HASHBUCKET_ALLOC(-3) : Bucket 의
                                             메모리 할당 실패일 때,
 *           HASH_ERR_KEY_FUNC(-4) : HashKey 생성 함수가 없을 때,
 *           HASH_ERR_COMP_FUNC(-5) : 사용자 비교 함수가 없을 때,
 *           HASH_ERR_ARGUMENT(-7) : DataType 이 잘못 되었을 때,
 *           초기화 된 Hash : 성공
 *
 * @bug
 *  hash_key 는 사용자가 작성한 HashKey 생성 함수를 사용해야 함
 *  hash_cmp 는 사용자가 작성한 Data 비교 함수를 사용해야 함
 *
 * @code
 *  HashT *pHash = NULL;
 *  pHash = nnHashInit(size, TEST_TYPE, hash_key, hash_cmp);
 * @endcode
 *
 * @test
 *  size 를 0 으로 입력,
 *  dataType 의 값을 사용자 타입의 범위를 벗어난 값으로 입력,
 *  hash_key 를 NULL 로 입력,
 *  hash_cmp 를 NULL 로 입력
 */

HashT *nnHashInit(
    Uint32T size,
    Uint32T dataType,
    Uint32T (*hash_key)(),
    BoolT (*hash_cmp)())
{
    HashT *newHash = NULL;

    if((dataType >= MEM_MAX_USER_TYPE && dataType < MEM_SYS_START)
       || dataType >= MEM_MAX_SYS_TYPE)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Wrong Hash DataType\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (HashT *)HASH_ERR_ARGUMENT;
    }

    if (hash_key == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Hash Key Function Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (HashT *) HASH_ERR_KEY_FUNC;
    }
    else if (hash_cmp == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Comp Function Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (HashT *) HASH_ERR_COMP_FUNC;
    }

    newHash = NNMALLOC(HASH_HEAD, sizeof(HashT));

    if((Int32T)newHash <= 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "NewHash Memory Allocate : Failure(%d)\n",
                       (Int32T)newHash);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (HashT *) HASH_ERR_HASH_ALLOC;
    }

    newHash->index = NULL;
    newHash->hash_key = hash_key;
    newHash->hash_cmp = hash_cmp;
    newHash->count = 0;
    newHash->dataType = dataType;

    if(size <= 0)
    {
        newHash->size = MAX_HASH_INDEX_SIZE;
    }
    else
    {
        newHash->size = size;
    }

    newHash->index = (HashBucketT **)NNMALLOC(HASH_BUCKET,
                                    sizeof(HashBucketT) * newHash->size);

    if((Int32T)newHash->index <= 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Hash Index Memory Allocate : Failure(%d)\n",
                       (Int32T)newHash);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (HashT *) HASH_ERR_HASHBUCKET_ALLOC;
    }    

    Uint32T i = 0;

    for(i = 0; i < newHash->size; ++i)
    {
        newHash->index[i] = NULL;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Hash Create (Size : %d) Compleate\n", newHash->size);
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return newHash;
}


/**
 * Description: Hash 에 저장된 Data 삭제 및 메모리를 해제하는 함수
 *
 * @param [in/out] pHash : 모든 Data 및 메모리를 해제 할 List
 *
 * @retval : None
 *
 * @code
 *  nnHashFree(pHash);
 * @endcode
 *
 * @test
 *  hash 를 NULL 로 입력,
 *  hash 에 아무런 값도 없는 상태에서 함수 수행
 */

void nnHashFree (HashT *hash)
{
    if (hash == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Hash Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    nnHashDeleteAllIndex(hash);

    NNFREE(HASH_BUCKET, hash->index);
    NNFREE(HASH_HEAD, hash);

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Free Hash Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}

/**
 * Description: 사용자가 등록한 Bucket 의 Data 비교 함수를 사용하여
 *              Hash 에서 파라미터로 받은 data 와 같은 Data 를
 *              가진 Bucket 을 검색해 Bucket 은 삭제하고
 *              Data 를 리턴하는 함수
 *
 * @param [in] hash : Bucket 을 삭제하고 Data 를 리턴할 Hash
 * @param [in] data : 삭제할 Data
 *
 * @retval : HASH_NO_DATA(0) : 존재하는 Data
 *                             또는 검색된 Data 가 없을 때,
 *           HASH_ERR_NULL(-6) : 파라미터가 NULL 일 때,
 *           삭제된 Bucket 의 Data : 성공
 *
 * @bug
 *  data 의 메모리는 사용자가 관리하여야 함
 *
 * @code
 *  testT *pRet = NULL;
 *  testT *pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  pRet = nnHashRelease(hash, pTest)
 * @endcode
 *
 * @test
 *  hash 를 NULL 로 입력,
 *  data 를 NULL 로 입력,
 *  hash 에 아무런 값도 없는 상태에서 함수 수행,
 *  hash 에 없는 값을 data 로 입력
 */

void * nnHashRelease (HashT * hash, void * data)
{
    Uint32T key = 0;
    Uint32T index = 0;
    HashBucketT *delBucket = NULL;
    HashBucketT *tempBucket = NULL;
    void * ret = NULL;

    if (hash == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Hash Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *) HASH_ERR_NULL;
    }
    else if (data == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *) HASH_ERR_NULL;
    }

    key = (*hash->hash_key)(data);
    index = key % hash->size;

    if (hash->index[index] == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Hash Is Empty\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *) HASH_NO_DATA;
    }

    for(delBucket = tempBucket = hash->index[index]; delBucket != NULL;
        delBucket = delBucket->next)
    {
        if(delBucket->key == key &&
            (*hash->hash_cmp)(delBucket->data, data) == TRUE)
        {
            if(delBucket == tempBucket)
            {
                hash->index[index] = delBucket->next;
            }
            else 
            {
                tempBucket->next = delBucket->next;
            }

            ret = delBucket->data;
        
            //free(delBucket);
            NNFREE(HASH_BUCKET, delBucket);
            hash->count--;
 
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_INFO, "Release Bucket : Success\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return ret;    
        }

        tempBucket = delBucket;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_ERR, "Release Bucket : Failure\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return (void *) HASH_NO_DATA;
}


/**
 * Description: 파라미터로 받은 index 와 같은 위치의 Hash Index 에 저장된
 *              모든 Bucket 을 삭제하는 함수
 *
 * @param [in] hash : Index 에 저장된 모든 Bucket 을 삭제할 Hash
 * @param [in] index : 삭제할 index 위치
 *
 * @retval : 실패 : HASH_ERR_MIS_INDEX(-1) : 파라미터로 받은
                                             index 가 잘못 되었을 때,
 *                  HASH_ERR_NULL(-6) : 파라미터가 NULL 일 때,
 *           성공 : SUCCESS(0)
 *
 * @code
 *  Int32T ret = 0;
 *
 *  ret = nnHashDeleteAllBucket(hash, index)
 * @endcode
 *
 * @test
 *  hash 를 NULL 로 입력,
 *  index 를 범위를 벗어난 값으로 입력,
 *  hash 에 아무런 값도 없는 상태에서 함수 수행
 */

Int32T nnHashDeleteAllBucket (HashT * hash, Uint64T index)
{
    HashBucketT * tempBucket = NULL;
    HashBucketT * tempNextBucket = NULL;

    if (hash == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Hash Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return HASH_ERR_NULL;
    }

    if (index >= hash->size)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Wrong Index\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return HASH_ERR_MIS_INDEX;
    }

    for(tempBucket = hash->index[index]; tempBucket != NULL;
        tempBucket = tempNextBucket)
    {
        tempNextBucket = tempBucket->next;

        NNFREE(hash->dataType, tempBucket->data);
        NNFREE(HASH_BUCKET, tempBucket);

        --hash->count;
    }

    hash->index[index] = NULL;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Delete All Bucket In Hash Index Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: Hash 에 저장된 모든 Bucket 을 삭제하는 함수
 *
 * @param [in] hash : 저장된 모든 Bucket 을 삭제할 Hash
 *
 * @retval : None
 *
 * @code
 *  nnHashDeleteAllIndex(hash)
 * @endcode
 *
 * @test
 *  hash 를 NULL 로 입력,
 *  hash 에 아무런 값도 없는 상태에서 함수 수행
 */

void nnHashDeleteAllIndex (HashT * hash)
{
    Uint32T i = 0;
    HashBucketT *tempBucket = NULL;
    HashBucketT *tempNextBucket = NULL;

    if (hash == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Hash Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    for(i = 0; i < hash->size; ++i)
    {
        for (tempBucket = hash->index[i]; tempBucket != NULL;
            tempBucket = tempNextBucket)
        {
            tempNextBucket = tempBucket->next;

            NNFREE(hash->dataType, tempBucket->data);
            NNFREE(HASH_BUCKET, tempBucket);

            --hash->count;
        }

        hash->index[i] = NULL;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Delete All Bucket In Hash Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}


/**
 * Description: Hash 에 저장된 Bucket 의 개수를 가져오는 함수
 *
 * @param [in] hash : 저장된 Bucket 개수를 가져올 Hash
 *
 * @retval : HASH_ERR_NULL(-6) : 파라미터가 NULL 일 때,
 *           저장된 Bucket 의 개수 : 성공
 *
 * @code
 *  Int64T ret = 0;
 *
 *  ret = nnHashCount(hash)
 * @endcode
 *
 * @test
 *  hash 를 NULL 로 입력,
 *  hash 에 아무런 값도 없는 상태에서 함수 수행
 */

Int64T nnHashCount(HashT * hash)
{
    if (hash == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Hash Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return HASH_ERR_NULL;
    }
    
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Get Hash Count : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return hash->count;
}


/**
 * Description: Data 를 Bucket 에 담아 Hash 에 저장하는 함수
 *
 * @param [in] hash : Data 를 저장할 Hash
 * @psram [in] data : 저장하려는 Data
 *
 * @retval : HASH_ERR_HASHBUCKET_ALLOC(-2) : Bucket 의
 *                                          메모리 할당 실패일 때,
 *           HASH_ERR_NULL(-6) : 파라미터가 NULL 일 때,
 *           저장된 Data : 성공
 * @bug
 *  저장 실패 시 data 의 메모리를 사용자가 관리하여야 함
 *  저장 시 사용자 비교 함수로 저장하려는 Data 와 Index 의 Data 를 비교하며,
 *  비교 시 같은 Data 가 있을경우 저장된 Data 가 리턴되므로,
 *  이 때에도 data 의 메모리를 사용자가 관리하여야 함
 *
 * @code
 *  testT *pRet = NULL;
 *  testT *pTest = NNMALLOC(TEST_TYPE, sizeof(testT));
 *  pTest->data = 100;
 *
 *  pRet = nnHashAddBucket(hash, pTest)
 * @endcode
 *
 * @test
 *  hash 를 NULL 로 입력,
 *  data 를 NULL 로 입력
 */

void * nnHashAddBucket (HashT * hash, void * data)
{
    Uint32T key = 0;
    Uint32T index = 0;
    HashBucketT *newBucket = NULL;
    HashBucketT *tempBucket = NULL;

    if (hash == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Hash Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *) HASH_ERR_NULL;
    }
    else if (data == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Data Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (void *) HASH_ERR_NULL;
    }

    key = (*hash->hash_key)(data);
    index = key % hash->size;

    for(tempBucket = hash->index[index]; tempBucket != NULL;
        tempBucket = tempBucket->next)
    {
        if(tempBucket->key == key &&
            (*hash->hash_cmp)(tempBucket->data, data) == TRUE)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_INFO, "Already add data\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return tempBucket->data;
        }
    }

    /* New Bucket */
    newBucket = NNMALLOC(HASH_BUCKET, sizeof(HashBucketT));

    if((Int32T)newBucket <= 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "newHashBucket Memory Allocate : Failure(%d)\n",
                       (Int32T)newBucket);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (HashT *) HASH_ERR_HASHBUCKET_ALLOC;
    }

    newBucket->next = NULL;
    newBucket->data = data;
    newBucket->key = key;

    if (hash->index[index] == NULL)
    {
        hash->index[index] = newBucket;
        hash->count++;

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Add Bucket : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return newBucket->data;
    }

    tempBucket = hash->index[index];

    while(tempBucket->next != NULL)
    {
        tempBucket = tempBucket->next;
    }

    tempBucket->next = newBucket;

    hash->count++;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Add Bucket : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return newBucket->data;
}


/*******************************************************************************
 * Function: <nnHashIterate>
 *
 * Description: <Iterate to the Hash>
 *
 * Parameters: hash : Hash to lterator
 *             func : User-Defined Functions
 *             arg  : The parameters for user-defined functions
 *
 * Returns: None
*******************************************************************************/

void nnHashIterate(
    HashT * hash,
    void (*func) (HashBucketT *, void *),
    void *arg)
{
    HashBucketT *itBucket;
    Uint32T i;

    if (hash == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Hash Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }
    else if (func == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Func Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    for (i = 0; i < hash->size; i++)
    {
        for(itBucket = hash->index[i]; itBucket; itBucket = itBucket -> next)
        {
            (*func) (itBucket, arg);
        }
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Hash Iterate Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
} 


/*******************************************************************************
 * Function: <nnHashIterate2>
 *
 * Description: <Iterate to the Hash wite 2 args>
 *
 * Parameters: hash : Hash to lterator
 *             func : User-Defined Functions
 *             arg1 : The parameters for user-defined functions
 *             arg2 : The parameters for user-defined functions
 *
 * Returns: None
*******************************************************************************/


void nnHashIterate2(
    HashT * hash,
    void (*func) (HashBucketT *, void *, void *),
    void *arg1, void *arg2)
{
    HashBucketT *itBucket;
    Uint32T i;

    if (hash == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Hash Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        return;
    }
    else if (func == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Func Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        return;
    }

    for (i = 0; i < hash->size; i++)
    {
        for(itBucket = hash->index[i]; itBucket;
            itBucket = itBucket -> next)
        {
            (*func) (itBucket, arg1, arg2);
        }
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Hash Iterate2 Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}


/*******************************************************************************
 * Function: <nnHashIterate3>
 *
 * Description: <Iterate to the Hash wite 3 args>
 *
 * Parameters: hash : Hash to lterator
 *             func : User-Defined Functions
 *             arg1 : The parameters for user-defined functions
 *             arg2 : The parameters for user-defined functions
 *             arg3 : The parameters for user-defined functions
 *
 * Returns: None
*******************************************************************************/


void nnHashIterate3 (
    HashT * hash,
    void (*func) (HashBucketT *, void *, void *, void *),
    void *arg1,
    void *arg2,
    void *arg3)
{
    HashBucketT *itBucket;
    Uint32T i;

    if (hash == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Hash Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }
    else if (func == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Func Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    for (i = 0; i < hash->size; i++)
    {
        for(itBucket = hash->index[i]; itBucket;
            itBucket = itBucket->next)
        {
            (*func) (itBucket, arg1, arg2, arg3);
        }
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Hash Iterate3 Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}


