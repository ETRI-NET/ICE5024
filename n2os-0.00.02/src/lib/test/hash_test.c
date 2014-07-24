#include <time.h>
#include "../nnHash.h"
#include "../nnLog.h"
#include "../nnMemmgr.h"
#include "test.h"

#define TEST_PROGRAM 0
#define TEST_TYPE 0
#define ADD_COUNT 10
#define CHECK_DATA 5
#define MAX_TEST_INDEX_SIZE 5

StringT hashErrMsg[] = {"SUCCESS", "HASH_ERR_MIS_INDEX", "HASH_ERR_HASH_ALLOC",
                        "HASH_ERR_HASHBUCKET_ALLOC", "HASH_ERR_KEY_FUNC",
                        "HASH_ERR_COMP_FUNC", "HASH_ERR_NULL",
                        "HASH_ERR_EMPTY", "HASH_ERR_ARGUMENT"};

/* Test Structure */
typedef struct testT {
    Int32T data;
} testT;

Int32T hashPrint(HashT * pHash);
void *nnHashGetHeadBucket(HashT *pHash);

Int32T menu = 0;
BoolT run = TRUE;
testT *pTestData = NULL;
testT *pTestData2 = NULL;
HashBucketT *pTestBucket = NULL;
Int32T hashIndex = 0;
Int32T addData = 0;

void printMenu()
{
    printf("==========================================\n");
    printf("           Hash Test Menu\n");
    printf("1. nnHashInit Test(Param DataCmp NULL)\n");
    printf("2. nnHashInit Test(Param HashKey NULL)\n");
    printf("3. nnHashInit Test(Param Wrong dataType)\n");
    printf("4. nnHashInit Test(Param Size 0)\n");
    printf("5. nnHashInit Test\n");

    printf("6. nnHashAddBucket Test(Hash NULL)\n");
    printf("7. nnHashAddBucket Test(Param Data NULL)\n");
    printf("8. nnHashAddBucket Test\n");

    printf("9. nnHashCount Test(Hash NULL)\n");
    printf("10. nnHashCount Test(No Data)\n");
    printf("11. nnHashCount Test\n");

    printf("12. nnHashFree Test(Hash NULL)\n");
    printf("13. nnHashFree Test(No Data)\n");
    printf("14. nnHashFree Test\n");

    printf("15. nnHashRelease Test(Hash NULL)\n");
    printf("16. nnHashRelease Test(Param Data NULL)\n");
    printf("17. nnHashRelease Test(No Data) Test\n");
    printf("18. nnHashRelease Test(Not Exist Data) Test\n");
    printf("19. nnHashRelease Test\n");

    printf("20. nnHashDeleteAllBucket Test(Hash NULL)\n");
    printf("21. nnHashDeleteAllBucket Test(Param Wrong Index)\n");
    printf("22. nnHashDeleteAllBucket Test(No Data)\n");
    printf("23. nnHashDeleteAllBucket Test Test\n");

    printf("24. nnHashDeleteAllIndex Test(Hash NULL)\n");
    printf("25. nnHashDeleteAllIndex Test(No Data)\n");
    printf("26. nnHashDeleteAllIndex Test\n");

    printf("27. nnHashIterate Test(Hash NULL)\n");
    printf("28. nnHashIterate Test(Param func NULL)\n");
    printf("29. nnHashIterate Test\n");
//////////////// Not Use New Hash - Add and Delete
// 계속적으로 등록/삭제를 수행 할 때 사용
    printf("30. nnHashAddBucket Test\n");
    printf("31. nnHashDeleteBucket Test\n");
    printf("32. Exit\n");
    printf("==========================================\n");
}

/* Use nnHashInit() */
Uint32T hashMakeKeyFunction(void *pData) {
    Uint32T key = 0;
    key = ((testT *)pData)->data % MAX_TEST_INDEX_SIZE;

    return key;
}

/* Use CompareFunction */
BoolT hashCompareFunction(void *pOldData, void *pNewData) {
    if(((testT *)pOldData)->data == ((testT *)pNewData)->data)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* Iterate Function */
void hashIterate(HashBucketT *pBucket, void *pData)
{
    if (((testT *)pBucket->data)->data == ((testT *)pData)->data)
    {
        printf("Same Data\n");
    }
}

Int32T main() 
{
    HashT *pHash = NULL;
    Int32T errno = 0;

    double start = 0;

    TestHead *pTestHead = (TestHead *)malloc(sizeof(TestHead));
    Test *pTempTest = NULL;

    pTestHead->pHead = NULL;
    pTestHead->pTail = NULL;
    pTestHead->count = 0;

    errno = getFileData(pTestHead, "hashConf");
    if (errno == FAILURE)
    {
        printf("FAILURE\n");
        return -1;
    }

    memInit(1);
    memSetDebug(TEST_TYPE);

    nnLogInit(1);

    printMenu();
    NNLOG(LOG_DEBUG, "Hash Test Start\n");

    start = clock();

    pTempTest = pTestHead->pHead;
    while(pTestHead->count > 0)
    {
        menu = pTempTest->m1;

        switch(menu)
        {
            case 1:
                printf("1. nnHashInit Test(Param DataCmp NULL)\n");

                if (pHash != NULL)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                printf("Hash Address : %d\n", (Int32T)pHash);
                printf("nnHashInit()\n");
        
                pHash = nnHashInit(MAX_TEST_INDEX_SIZE,
                                   TEST_TYPE, hashMakeKeyFunction, NULL);

                if ((Int32T)pHash <= 0)
                {
                    printf("Error nnHashInit() : %s\n",
                           hashErrMsg[(-(Int32T)pHash)]);
                    pHash = NULL;
                    break;
                }

                printf("Hash Address : %d\n", (Int32T)pHash);

                break;
            case 2:
                printf("2. nnHashInit Test(Param HashKey NULL)\n");

                if (pHash != NULL)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                printf("Hash Address : %d\n", (Int32T)pHash);
                printf("nnHashInit()\n");
        
                pHash = nnHashInit(MAX_TEST_INDEX_SIZE,
                                   TEST_TYPE, NULL, hashCompareFunction);

                if ((Int32T)pHash <= 0)
                {
                    printf("Error nnHashInit() : %s\n",
                           hashErrMsg[(-(Int32T)pHash)]);
                    pHash = NULL;
                    break;
                }

                printf("Hash Address : %d\n", (Int32T)pHash);

                break;
            case 3:
                printf("3. nnHashInit Test(Param Wrong dataType)\n");

                if (pHash != NULL)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                printf("Hash Address : %d\n", (Int32T)pHash);
                printf("nnHashInit()\n");
        
                pHash = nnHashInit(MAX_TEST_INDEX_SIZE, MEM_MAX_USER_TYPE + 1,
                                   NULL, hashCompareFunction);

                if ((Int32T)pHash <= 0)
                {
                    printf("Error nnHashInit() : %s\n",
                           hashErrMsg[(-(Int32T)pHash)]);
                    pHash = NULL;
                }

                printf("Hash Address : %d\n", (Int32T)pHash);

                break;
            case 4:
                printf("4. nnHashInit Test(Param Size 0)\n");

                if (pHash != NULL)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                printf("Hash Address : %d\n", (Int32T)pHash);
                printf("nnHashInit()\n");
        
                pHash = nnHashInit(0, TEST_TYPE,
                                   hashMakeKeyFunction, hashCompareFunction);

                if ((Int32T)pHash <= 0)
                {
                    printf("Error nnHashInit() : %s\n",
                           hashErrMsg[(-(Int32T)pHash)]);
                    pHash = NULL;
                }

                printf("Hash Address : %d\n", (Int32T)pHash);

                break;
            case 5:
                printf("5. nnHashInit Test\n");

                if (pHash != NULL)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                printf("Hash Address : %d\n", (Int32T)pHash);
                printf("nnHashInit()\n");
        
                pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE,
                                   hashMakeKeyFunction, hashCompareFunction);

                if ((Int32T)pHash <= 0)
                {
                    printf("Error nnHashInit() : %s\n",
                           hashErrMsg[(-(Int32T)pHash)]);
                    pHash = NULL;
                }

                printf("Hash Address : %d\n", (Int32T)pHash);

                break;
            case 6:
                printf("6. nnHashAddBucket Test(Hash NULL)\n");

                if (pHash != NULL)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                pTestData = NULL;
                pTestData2 = nnHashAddBucket(pHash, pTestData);

                if (pTestData2 != pTestData)
                {
                    if ((Int32T)pTestData2 < 0
                        && (Int32T)pTestData2 > HASH_ERR_NULL)
                    {
                        printf("Error nnHashAddBucket() : %s\n",
                               hashErrMsg[(-(Int32T)pTestData2)]);
                    }
                    else
                    {
                        printf("Error nnHashAddBucket() : Same Data\n");
                    }

                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                break;
            case 7:
                printf("7. nnHashAddBucket(Param Data NULL) Test\n");

                if (pHash != NULL)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                if (pHash == NULL)
                {
                    pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE,
                                      hashMakeKeyFunction, hashCompareFunction);

                    if ((Int32T)pHash <= 0)
                    {
                        printf("Error nnHashInit() : %s\n",
                               hashErrMsg[(-(Int32T)pHash)]);
                        pHash = NULL;
                        break;
                    }
                }

                pTestData = NULL;
                pTestData2 = nnHashAddBucket(pHash, pTestData);
                if (pTestData2 != pTestData)
                {
                    if ((Int32T)pTestData2 < 0
                        && (Int32T)pTestData2 > HASH_ERR_NULL)
                    {
                        printf("Error nnHashAddBucket() : %s\n",
                               hashErrMsg[(-(Int32T)pTestData2)]);
                    }
                    else
                    {
                        printf("Error nnHashAddBucket() : Same Data\n");
                    }

                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                break;
            case 8:
                printf("8. nnHashAddBucket Test\n");

                if (pHash != NULL)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                if (pHash == NULL)
                {
                    pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE,
                                      hashMakeKeyFunction, hashCompareFunction);

                    if ((Int32T)pHash <= 0)
                    {
                        printf("Error nnHashInit() : %s\n",
                               hashErrMsg[(-(Int32T)pHash)]);
                        pHash = NULL;
                        break;
                    }
                }

                for(hashIndex = 0; hashIndex < pTempTest->m3; ++hashIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = hashIndex + 1;

                    pTestData2 = nnHashAddBucket(pHash, pTestData);
                    if (pTestData2 != pTestData)
                    {
                        if ((Int32T)pTestData2 < 0
                            && (Int32T)pTestData2 > HASH_ERR_NULL)
                        {
                            printf("Error nnHashAddBucket() : %s\n",
                                   hashErrMsg[(-(Int32T)pTestData2)]);
                        }
                        else
                        {
                            printf("Error nnHashAddBucket() : Same Data\n");
                        }

                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                hashPrint(pHash);
                break;
            case 9:
                printf("9. nnHashCount Test(Hash NULL)\n");

                if (pHash)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                errno = nnHashCount(pHash);

                if (errno < 0)
                {
                    printf("Error nnHashCount() : %s\n", 
                           hashErrMsg[(-(errno))]);

                    break;
                }
                else
                {
                    printf("HashCount : %d\n", errno);
                }

                break;
            case 10:
                printf("10. nnHashCount Test(No Data)\n");

                if (pHash != NULL)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                if (pHash == NULL)
                {
                    pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE,
                                      hashMakeKeyFunction, hashCompareFunction);

                    if ((Int32T)pHash <= 0)
                    {
                        printf("Error nnHashInit() : %s\n",
                               hashErrMsg[(-(Int32T)pHash)]);
                        pHash = NULL;
                        break;
                    }
                }

                errno = nnHashCount(pHash);

                if (errno < 0)
                {
                    printf("Error nnHashCount() : %s\n",
                           hashErrMsg[(-(errno))]);

                    break;
                }
                else
                {
                    printf("HashCount : %d\n", errno);
                }

                break;
            case 11:
                printf("11. nnHashCount Test\n");

                if (pHash != NULL)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                if (pHash == NULL)
                {
                    pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE,
                                      hashMakeKeyFunction, hashCompareFunction);

                    if ((Int32T)pHash <= 0)
                    {
                        printf("Error nnHashInit() : %s\n",
                               hashErrMsg[(-(Int32T)pHash)]);
                        pHash = NULL;
                        break;
                    }
                }

                for(hashIndex = 0; hashIndex < pTempTest->m3; ++hashIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = hashIndex + 1;

                    pTestData2 = nnHashAddBucket(pHash, pTestData);

                    if (pTestData2 != pTestData)
                    {
                        if ((Int32T)pTestData2 < 0
                            && (Int32T)pTestData2 > HASH_ERR_NULL)
                        {
                            printf("Error nnHashAddBucket() : %s\n",
                                   hashErrMsg[(-(Int32T)pTestData2)]);
                        }
                        else
                        {
                            printf("Error nnHashAddBucket() : Same Data\n");
                        }

                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                errno = nnHashCount(pHash);

                if (errno < 0)
                {
                    printf("Error nnHashCount() : %s\n", 
                           hashErrMsg[(-(errno))]);

                    break;
                }
                else
                {
                    printf("HashCount : %d\n", errno);
                }

                break;
            case 12:
                printf("12. nnHashFree Test(Hash NULL)\n");

                if (pHash)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                nnHashFree(pHash);
                pHash = NULL;

                break;
            case 13:
                printf("13. nnHashFree Test(No Data)\n");

                if (pHash)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                if (pHash == NULL)
                {
                    pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE,
                                      hashMakeKeyFunction, hashCompareFunction);

                    if ((Int32T)pHash <= 0)
                    {
                        printf("Error nnHashInit() : %s\n",
                               hashErrMsg[(-(Int32T)pHash)]);
                        pHash = NULL;
                        break;
                    }
                }

                nnHashFree(pHash);
                pHash = NULL;

                break;
            case 14:
                printf("14. nnHashFree Test\n");

                if (pHash)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                if (pHash == NULL)
                {
                    pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE,
                                      hashMakeKeyFunction, hashCompareFunction);

                    if ((Int32T)pHash <= 0)
                    {
                        printf("Error nnHashInit() : %s\n",
                               hashErrMsg[(-(Int32T)pHash)]);
                        pHash = NULL;
                        break;
                    }
                }

                for(hashIndex = 0; hashIndex < pTempTest->m3; ++hashIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = hashIndex + 1;

                    pTestData2 = nnHashAddBucket(pHash, pTestData);
                    if (pTestData2 != pTestData)
                    {
                        if ((Int32T)pTestData2 < 0
                            && (Int32T)pTestData2 > HASH_ERR_NULL)
                        {
                            printf("Error nnHashAddBucket() : %s\n",
                                   hashErrMsg[(-(Int32T)pTestData2)]);
                        }
                        else
                        {
                            printf("Error nnHashAddBucket() : Same Data\n");
                        }

                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                nnHashFree(pHash);
                pHash = NULL;

                break;
            case 15:
                printf("15. nnHashRelease Test(Hash NULL)\n");

                if (pHash)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = CHECK_DATA;

                pTestData2 = nnHashRelease(pHash, pTestData);

                if (pTestData2 == NULL)
                {
                    printf("ReleaseNode = NULL\n");
                }
                else if ((Int32T)pTestData2 < 0)
                {
                    printf("Error nnHashRelease() : %s\n",
                           hashErrMsg[(-((Int32T)pTestData2))]);
                }
                else
                {
                    printf("Print ReleaseNode's Data %d\n", pTestData2->data);
                }

                NNFREE(TEST_TYPE, pTestData);
                pTestData = NULL;

                break;
            case 16:
                printf("16. nnHashRelease Test(Param Data NULL)\n");

                if (pHash)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                if (pHash == NULL)
                {
                    pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE,
                                      hashMakeKeyFunction, hashCompareFunction);

                    if ((Int32T)pHash <= 0)
                    {
                        printf("Error nnHashInit() : %s\n",
                               hashErrMsg[(-(Int32T)pHash)]);
                        pHash = NULL;
                        break;
                    }
                }

                pTestData = NULL;
                pTestData2 = nnHashRelease(pHash, pTestData);

                if (pTestData2 == NULL)
                {
                    printf("ReleaseNode = NULL\n");
                }
                else if ((Int32T)pTestData2 < 0)
                {
                    printf("Error nnHashRelease() : %s\n", 
                           hashErrMsg[(-((Int32T)pTestData2))]);
                }
                else
                {
                    printf("Print ReleaseNode's Data %d\n", pTestData2->data);
                }

                break;
            case 17:
                printf("17. nnHashRelease Test(No Data)\n");

                if (pHash)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                if (pHash == NULL)
                {
                    pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE,
                                      hashMakeKeyFunction, hashCompareFunction);

                    if ((Int32T)pHash <= 0)
                    {
                        printf("Error nnHashInit() : %s\n",
                               hashErrMsg[(-(Int32T)pHash)]);
                        pHash = NULL;
                        break;
                    }
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = CHECK_DATA;

                pTestData2 = nnHashRelease(pHash, pTestData);

                if (pTestData2 == NULL)
                {
                    printf("ReleaseNode = NULL\n");
                }
                else if ((Int32T)pTestData2 < 0)
                {
                    printf("Error nnHashRelease() : %s\n",
                           hashErrMsg[(-((Int32T)pTestData2))]);
                }
                else
                {
                    printf("Print ReleaseNode's Data %d\n", pTestData2->data);
                }

                NNFREE(TEST_TYPE, pTestData);
                pTestData = NULL;

                break;
            case 18:
                printf("18. nnHashRelease Test(Not Exist Data)\n");

                if (pHash)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                if (pHash == NULL)
                {
                    pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE,
                                      hashMakeKeyFunction, hashCompareFunction);

                    if ((Int32T)pHash <= 0)
                    {
                        printf("Error nnHashInit() : %s\n",
                               hashErrMsg[(-(Int32T)pHash)]);
                        pHash = NULL;
                        break;
                    }
                }

                for(hashIndex = 0; hashIndex < pTempTest->m3; ++hashIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = hashIndex + 1;

                    pTestData2 = nnHashAddBucket(pHash, pTestData);
                    if (pTestData2 != pTestData)
                    {
                        if ((Int32T)pTestData2 < 0
                            && (Int32T)pTestData2 > HASH_ERR_NULL)
                        {
                            printf("Error nnHashAddBucket() : %s\n",
                                   hashErrMsg[(-(Int32T)pTestData2)]);
                        }
                        else
                        {
                            printf("Error nnHashAddBucket() : Same Data\n");
                        }

                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = pTempTest->m3 + 1;

                printf("Release Data is %d\n", pTestData->data);
                pTestData2 = nnHashRelease(pHash, pTestData);

                if (pTestData2 == NULL)
                {
                    printf("ReleaseNode = NULL\n");
                }
                else if ((Int32T)pTestData2 < 0)
                {
                    printf("Error nnHashRelease() : %s\n",
                           hashErrMsg[(-((Int32T)pTestData2))]);
                }
                else
                {
                    printf("Print ReleaseNode's Data %d\n", pTestData2->data);
                }

                NNFREE(TEST_TYPE, pTestData);
                pTestData = NULL;

                hashPrint(pHash);
                break;
            case 19:
                printf("19. nnHashRelease Test\n");

                if (pHash)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                if (pHash == NULL)
                {
                    pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE,
                                      hashMakeKeyFunction, hashCompareFunction);

                    if ((Int32T)pHash <= 0)
                    {
                        printf("Error nnHashInit() : %s\n",
                               hashErrMsg[(-(Int32T)pHash)]);
                        pHash = NULL;
                        break;
                    }
                }

                for(hashIndex = 0; hashIndex < pTempTest->m3; ++hashIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = hashIndex + 1;

                    pTestData2 = nnHashAddBucket(pHash, pTestData);
                    if (pTestData2 != pTestData)
                    {
                        if ((Int32T)pTestData2 < 0
                            && (Int32T)pTestData2 > HASH_ERR_NULL)
                        {
                            printf("Error nnHashAddBucket() : %s\n",
                                   hashErrMsg[(-(Int32T)pTestData2)]);
                        }
                        else
                        {
                            printf("Error nnHashAddBucket() : Same Data\n");
                        }

                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = CHECK_DATA;

                printf("Release Data is %d\n", pTestData->data);
                pTestData2 = nnHashRelease(pHash, pTestData);

                if (pTestData2 == NULL)
                {
                    printf("ReleaseNode = NULL\n");
                }
                else if ((Int32T)pTestData2 < 0)
                {
                    printf("Error nnHashRelease() : %s\n",
                           hashErrMsg[(-((Int32T)pTestData2))]);
                }
                else
                {
                    printf("Print ReleaseNode's Data %d\n", pTestData2->data);
                }

                NNFREE(TEST_TYPE, pTestData);
                NNFREE(TEST_TYPE, pTestData2);
                pTestData = NULL;

                hashPrint(pHash);
                break;
            case 20:
                printf("20. nnHashDeleteAllBucket Test(Hash NULL)\n");

                if (pHash)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                errno = nnHashDeleteAllBucket(pHash, 0);

                if (errno < 0)
                {
                    printf("Error nnHashDeleteAllBucket() : %s\n",
                           hashErrMsg[(-(errno))]);

                    break;
                }
                else
                {
                    printf("nnHashDeleteAllBucket() : %s\n",
                           hashErrMsg[(-(errno))]);
                }

                break;
            case 21:
                printf("21. nnHashDeleteAllBucket Test(Param Wrong Index)\n");

                if (pHash)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                if (pHash == NULL)
                {
                    pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE,
                                      hashMakeKeyFunction, hashCompareFunction);

                    if ((Int32T)pHash <= 0)
                    {
                        printf("Error nnHashInit() : %s\n",
                               hashErrMsg[(-(Int32T)pHash)]);
                        pHash = NULL;
                        break;
                    }
                }

                errno = nnHashDeleteAllBucket(pHash, MAX_TEST_INDEX_SIZE + 1);

                if (errno < 0)
                {
                    printf("Error nnHashDeleteAllBucket() : %s\n",
                           hashErrMsg[(-(errno))]);

                    break;
                }
                else
                {
                    printf("nnHashDeleteAllBucket() : %s\n",
                           hashErrMsg[(-(errno))]);
                }

                hashPrint(pHash);
                break;
            case 22:
                printf("22. nnHashDeleteAllBucket Test(No Data)\n");

                if (pHash)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                if (pHash == NULL)
                {
                    pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE,
                                      hashMakeKeyFunction, hashCompareFunction);

                    if ((Int32T)pHash <= 0)
                    {
                        printf("Error nnHashInit() : %s\n", 
                               hashErrMsg[(-(Int32T)pHash)]);
                        pHash = NULL;
                        break;
                    }
                }

                errno = nnHashDeleteAllBucket(pHash, 0);

                if (errno < 0)
                {
                    printf("Error nnHashDeleteAllBucket() : %s\n",
                           hashErrMsg[(-(errno))]);

                    break;
                }
                else
                {
                    printf("nnHashDeleteAllBucket() : %s\n",
                           hashErrMsg[(-(errno))]);
                }

                hashPrint(pHash);
                break;
            case 23:
                printf("23. nnHashDeleteAllBucket Test\n");

                if (pHash)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                if (pHash == NULL)
                {
                    pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE,
                                      hashMakeKeyFunction, hashCompareFunction);

                    if ((Int32T)pHash <= 0)
                    {
                        printf("Error nnHashInit() : %s\n",
                               hashErrMsg[(-(Int32T)pHash)]);
                        pHash = NULL;
                        break;
                    }
                }

                for(hashIndex = 0; hashIndex < pTempTest->m3; ++hashIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = hashIndex + 1;

                    pTestData2 = nnHashAddBucket(pHash, pTestData);
                    if (pTestData2 != pTestData)
                    {
                        if ((Int32T)pTestData2 < 0
                            && (Int32T)pTestData2 > HASH_ERR_NULL)
                        {
                            printf("Error nnHashAddBucket() : %s\n",
                                   hashErrMsg[(-(Int32T)pTestData2)]);
                        }
                        else
                        {
                            printf("Error nnHashAddBucket() : Same Data\n");
                        }

                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                errno = nnHashDeleteAllBucket(pHash, 0);

                if (errno < 0)
                {
                    printf("Error nnHashDeleteAllBucket() : %s\n",
                           hashErrMsg[(-(errno))]);

                    break;
                }
                else
                {
                    printf("nnHashDeleteAllBucket() : %s\n",
                           hashErrMsg[(-(errno))]);
                }

                hashPrint(pHash);
                break;
            case 24:
                printf("24. nnHashDeleteAllIndex Test(Hash NULL)\n");

                if (pHash)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                nnHashDeleteAllIndex(pHash);

                break;
            case 25:
                printf("25. nnHashDeleteAllIndex Test(No Data)\n");

                if (pHash)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                if (pHash == NULL)
                {
                    pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE,
                                      hashMakeKeyFunction, hashCompareFunction);

                    if ((Int32T)pHash <= 0)
                    {
                        printf("Error nnHashInit() : %s\n",
                               hashErrMsg[(-(Int32T)pHash)]);
                        pHash = NULL;
                        break;
                    }
                }

                nnHashDeleteAllIndex(pHash);

                break;
            case 26:
                printf("26. nnHashDeleteAllIndex Test\n");

                if (pHash)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                if (pHash == NULL)
                {
                    pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE,
                                      hashMakeKeyFunction, hashCompareFunction);

                    if ((Int32T)pHash <= 0)
                    {
                        printf("Error nnHashInit() : %s\n",
                               hashErrMsg[(-(Int32T)pHash)]);
                        pHash = NULL;
                        break;
                    }
                }

                for(hashIndex = 0; hashIndex < pTempTest->m3; ++hashIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = hashIndex + 1;

                    pTestData2 = nnHashAddBucket(pHash, pTestData);
                    if (pTestData2 != pTestData)
                    {
                        if ((Int32T)pTestData2 < 0
                            && (Int32T)pTestData2 > HASH_ERR_NULL)
                        {
                            printf("Error nnHashAddBucket() : %s\n",
                                   hashErrMsg[(-(Int32T)pTestData2)]);
                        }
                        else
                        {
                            printf("Error nnHashAddBucket() : Same Data\n");
                        }

                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                nnHashDeleteAllIndex(pHash);

                hashPrint(pHash);
                break;
            case 27:
                printf("27. nnHashIterate(Hash NULL) Test\n");

                if (pHash)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                nnHashIterate(pHash, NULL, NULL);

                break;
            case 28:
                printf("28. nnHashIterate(Param func NULL) Test\n");

                if (pHash)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                if (pHash == NULL)
                {
                    pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE,
                                      hashMakeKeyFunction, hashCompareFunction);

                    if ((Int32T)pHash <= 0)
                    {
                        printf("Error nnHashInit() : %s\n",
                               hashErrMsg[(-(Int32T)pHash)]);
                        pHash = NULL;
                        break;
                    }
                }

                nnHashIterate(pHash, NULL, NULL);

                break;
            case 29:
                printf("29. nnHashIterate Test\n");

                if (pHash)
                {
                    nnHashFree(pHash);
                    pHash = NULL;
                }

                if (pHash == NULL)
                {
                    pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE,
                                      hashMakeKeyFunction, hashCompareFunction);

                    if ((Int32T)pHash <= 0)
                    {
                        printf("Error nnHashInit() : %s\n",
                               hashErrMsg[(-(Int32T)pHash)]);
                        pHash = NULL;
                        break;
                    }
                }

                for(hashIndex = 0; hashIndex < pTempTest->m3; ++hashIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = hashIndex + 1;

                    pTestData2 = nnHashAddBucket(pHash, pTestData);
                    if (pTestData2 != pTestData)
                    {
                        if ((Int32T)pTestData2 < 0
                            && (Int32T)pTestData2 > HASH_ERR_NULL)
                        {
                            printf("Error nnHashAddBucket() : %s\n",
                                   hashErrMsg[(-(Int32T)pTestData2)]);
                        }
                        else
                        {
                            printf("Error nnHashAddBucket() : Same Data\n");
                        }

                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                nnHashIterate(pHash, hashIterate, pTestData);

                break;
            case 30:
                printf("30. nnHashAddBucket Test\n");

                if (pHash == NULL)
                {
                    pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE,
                                      hashMakeKeyFunction, hashCompareFunction);

                    if ((Int32T)pHash <= 0)
                    {
                        printf("Error nnHashInit() : %s\n",
                               hashErrMsg[(-(Int32T)pHash)]);
                        pHash = NULL;
                        break;
                    }
                }

                for(hashIndex = 0; hashIndex < pTempTest->m3; ++hashIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = ++addData;

                    pTestData2 = nnHashAddBucket(pHash, pTestData);
                    if (pTestData2 != pTestData)
                    {
                        if ((Int32T)pTestData2 < 0
                            && (Int32T)pTestData2 > HASH_ERR_NULL)
                        {
                            printf("Error nnHashAddBucket() : %s\n",
                                   hashErrMsg[(-(Int32T)pTestData2)]);
                        }
                        else
                        {
                            printf("Error nnHashAddBucket() : Same Data\n");
                        }

                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                hashPrint(pHash);

                break;
            case 31:
                printf("31. nnHashDeleteBucket Test\n");

                if (pHash == NULL)
                {
                    pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE,
                                      hashMakeKeyFunction, hashCompareFunction);

                    if ((Int32T)pHash <= 0)
                    {
                        printf("Error nnHashInit() : %s\n",
                               hashErrMsg[(-(Int32T)pHash)]);
                        pHash = NULL;
                        break;
                    }
                }

                for(hashIndex = 0; hashIndex < pTempTest->m3; ++hashIndex)
                {
                    pTestData = nnHashGetHeadBucket(pHash);
                    if ((Int32T)pTestData != FAILURE)
                    {
                        pTestData2 = nnHashRelease(pHash, pTestData);

                        if (pTestData2 != pTestData)
                        {
                            continue;
                        }

                        NNFREE(TEST_TYPE, pTestData2);
                    }
                }

                hashPrint(pHash);

                break;
            case 32:
                printf("Exit\n");
                pTestHead->count = 0;
                break;
            default:
                printf("Wrong Number\n");
                menu = 0;
                break;

        }

        --pTempTest->m2;

        if (pTempTest->m2 == 0)
        {
            printf("Temp->m3 : %d\n", pTempTest->m3);
            printf("Menu %d is End. %f Seconds.\n",
                   pTempTest->m1, (double)(clock() - start) / CLOCKS_PER_SEC);
            NNLOG(LOG_DEBUG, "Menu %d is %f Seconds.\n",
                  pTempTest->m1, (double)(clock() - start) / CLOCKS_PER_SEC);

            --pTestHead->count;
            pTempTest = pTempTest->pNext;

            start = clock();
        }
    }

    if (pHash)
    {
        nnHashFree(pHash);
        pHash = NULL;
    }

    NNLOG(LOG_DEBUG, "Hash Test End\n");

    nnLogClose();

    memShowAllSystem(MEM_MAX_SYS_CNT);
    memShowAllUser (TEST_TYPE);

    memClose();

    freeTestHead(pTestHead);

        return 0;
}

/* Print Bucket Test */
int hashPrint(HashT *pHash)
{
    Int32T i = 0;
    HashBucketT *pTempBucket = NULL;

    if (pHash == NULL || pHash < 0)
    {
        printf("hashPrint : Hash is NULL\n");
        return FAILURE;
    }

    printf("HASH PRINT START========================\n");

    for (i = 0; i < MAX_TEST_INDEX_SIZE; ++i)
    {
        printf("INDEX : %d -> ", i);

        for (pTempBucket = pHash->index[i];
             pTempBucket != NULL;
             pTempBucket = pTempBucket->next)
        {
            printf("%d ", ((testT *)(pTempBucket->data))->data);
        }

        printf("\n");
    }
    printf("Total Count : %llu\n", nnHashCount(pHash));

    return 0;
}

// HashBucket->data, FAILURE;
void *nnHashGetHeadBucket(HashT *pHash)
{
    Int32T i = 0;

    if (pHash->size == 0)
        return (HashBucketT *)FAILURE;

    for (i = 0; i < pHash->size; ++i)
    {
        if (pHash->index[i] == NULL)
        {
            continue;
        }
        else
        {
            return pHash->index[i]->data;
        }
    }

    return (HashBucketT *)FAILURE;
}
