#include <time.h>
#include "../nnCqueue.h"
#include "../nnLog.h"
#include "../nnMemmgr.h"
#include "test.h"

#define TEST_PROGRAM 0
#define TEST_TYPE 0
#define ADD_COUNT 10
#define CHECK_DATA 5

StringT cqueueErrMsg[] = {"SUCCESS", "", "CQUEUE_ERR_INCORRECT_INDEX",
                          "CQUEUE_ERR_CQUEUE_ALLOC",
                          "CQUEUE_ERR_CQUEUENODE_ALLOC",
                          "CQUEUE_ERR_NULL", "QUEUE_ERR_EMPTY",
                          "CQUEUE_ERR_DUP", "CQUEUE_ERR_ARGUMENT"};

/* Test Structure */
typedef struct testT {
        Int32T data;
} testT;

Int32T cqueuePrint(CqueueT * pCqueue);

Int32T menu = 0;
BoolT run = TRUE;
testT *pTestData = NULL;
testT *pTestData2 = NULL;
Int32T cqueueIndex = 0;

void printMenu()
{
    printf("==========================================\n");
    printf("           Cqueue Test Menu\n");
    printf("1. nnCqueueInit Test(Param maxCount 0)\n");
    printf("2. nnCqueueInit Test(Param Wrong dupCheck)\n");
    printf("3. nnCqueueInit Test(Param Wrong dataType)\n");
    printf("4. nnCqueueInit Test\n");

    printf("5. nnCqueueEnqueue Test(Cqueue NULL)\n");
    printf("6. nnCququeEnqueue Test(Param pData NULL)\n");
    printf("7. nnCququeEnqueue Test(Duplicate pData)\n");
    printf("8. nnCqueueEnqueue Test\n");

    printf("9. nnCqueueGetHead Test(Cqueue NULL)\n");
    printf("10. nnCqueueGetHead Test(No Data)\n");
    printf("11. nnCqueueGetHead Test\n");

    printf("12. nnCqueueGetTail Test(Cqueue NULL)\n");
    printf("13. nnCqueueGetTail Test(No Data)\n");
    printf("14. nnCqueueGetTail Test\n");

    printf("15. nnCqueueCount Test(Cqueue NULL)\n");
    printf("16. nnCqueueCount Test(No Data)\n");
    printf("17. nnCqueueCount Test\n");

    printf("18. nnCqueueFree Test(Cqueue NULL)\n");
    printf("19. nnCqueueFree Test(No Data)\n");
    printf("20. nnCqueueFree Test\n");

    printf("21. nnCqueueDequeue Test(Cqueue NULL)\n");
    printf("22. nnCqueueDequeue Test(No Data)\n");
    printf("23. nnCqueueDequeue Test\n");

    printf("24. nnCqueueDeleteAllNode Test(Cqueue NULL)\n");
    printf("25. nnCqueueDeleteAllNode Test(No Data)\n");
    printf("26. nnCqueueDeleteAllNode Test\n");
//////////////// Not Use New Cqueue - Add and Delete // 계속적으로 등록/삭제를 수행 할 때 사용
    printf("27. nnCqueueEnqueue\n");
    printf("28. nnCqueueDequeue\n");
    printf("29. Exit\n");
    printf("==========================================\n");
}

Int32T main() 
{
    CqueueT *pCqueue = NULL;
    Int32T errno = 0;

    double start = 0;

    TestHead *pTestHead = (TestHead *)malloc(sizeof(TestHead));
    Test *pTempTest = NULL;

    pTestHead->pHead = NULL;
    pTestHead->pTail = NULL;
    pTestHead->count = 0;

    errno = getFileData(pTestHead, "cqueueConf");
    if (errno == FAILURE)
    {
        printf("FAILURE\n");
        return -1;
    }

    memInit(1);
    memSetDebug(TEST_TYPE);

    nnLogInit(1);

    printMenu();
    NNLOG(LOG_DEBUG, "Cqueue Test Start\n");

    start = clock();

    pTempTest = pTestHead->pHead;
    while(pTestHead->count > 0)
    {
        menu = pTempTest->m1;

        switch(menu)
        {
            case 1:
                printf("1. nnCqueueInit Test(Param maxCount 0)\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                printf("Cqueue Address : %d\n", (Int32T)pCqueue);
                printf("nnCqueueInit()\n");
        
                pCqueue = nnCqueueInit(0, CQUEUE_NOT_DUPLICATED, TEST_TYPE);

                if ((Int32T)pCqueue <= 0)
                {
                    printf("Error nnCqueueInit() : %s\n",
                           cqueueErrMsg[(-(Int32T)pCqueue)]);
                    pCqueue = NULL;
                    break;
                }

                printf("Cqueue Address : %d\n", (Int32T)pCqueue);

                break;
            case 2:
                printf("2. nnCqueueInit Test(Param Wrong dupCheck)\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                printf("Cqueue Address : %d\n", (Int32T)pCqueue);
                printf("nnCqueueInit()\n");
        
                pCqueue = nnCqueueInit(CQUEUE_DEFAULT_COUNT,
                                       CHECK_DATA, TEST_TYPE);

                if ((Int32T)pCqueue <= 0)
                {
                    printf("Error nnCqueueInit() : %s\n",
                           cqueueErrMsg[(-(Int32T)pCqueue)]);
                    pCqueue = NULL;
                    break;
                }

                printf("Cqueue Address : %d\n", (Int32T)pCqueue);

                break;
            case 3:
                printf("3. nnCqueueInit Test(Param Wrong dataType)\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                printf("Cqueue Address : %d\n", (Int32T)pCqueue);
                printf("nnCqueueInit()\n");
        
                pCqueue = nnCqueueInit(CQUEUE_DEFAULT_COUNT,
                                  CQUEUE_NOT_DUPLICATED, MEM_MAX_USER_TYPE + 1);

                if ((Int32T)pCqueue <= 0)
                {
                    printf("Error nnCqueueInit() : %s\n",
                           cqueueErrMsg[(-(Int32T)pCqueue)]);
                    pCqueue = NULL;
                    break;
                }

                printf("Cqueue Address : %d\n", (Int32T)pCqueue);

                break;
            case 4:
                printf("4. nnCqueueInit Test\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                printf("Cqueue Address : %d\n", (Int32T)pCqueue);
                printf("nnCqueueInit()\n");
        
                pCqueue = nnCqueueInit(CQUEUE_DEFAULT_COUNT,
                                       CQUEUE_NOT_DUPLICATED, TEST_TYPE);

                if ((Int32T)pCqueue <= 0)
                {
                    printf("Error nnCqueueInit() : %s\n",
                           cqueueErrMsg[(-(Int32T)pCqueue)]);
                    pCqueue = NULL;
                    break;
                }

                printf("Cqueue Address : %d\n", (Int32T)pCqueue);

                break;
            case 5:
                printf("5. nnCqueueEnqueue Test(Cqueue NULL)\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                errno = nnCqueueEnqueue(pCqueue, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnCqueueEnqueue() : %s\n",
                           cqueueErrMsg[(-(errno))]);

                    break;
                }

                cqueuePrint(pCqueue);
                break;
            case 6:
                printf("6. nnCqueueEnqueue Test(Param pData NULL)\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                if (pCqueue == NULL)
                {
                    pCqueue = nnCqueueInit(CQUEUE_DEFAULT_COUNT,
                                           CQUEUE_NOT_DUPLICATED, TEST_TYPE);

                    if ((Int32T)pCqueue <= 0)
                    {
                        printf("Error nnCqueueInit() : %s\n",
                               cqueueErrMsg[(-(Int32T)pCqueue)]);
                        pCqueue = NULL;
                        break;
                    }
                }

                errno = nnCqueueEnqueue(pCqueue, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnCqueueEnqueue() : %s\n",
                           cqueueErrMsg[(-(errno))]);

                    break;
                }

                cqueuePrint(pCqueue);
                break;
            case 7:
                printf("7. nnCqueueEnqueue Test(Duplicate pData)\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                if (pCqueue == NULL)
                {
                    pCqueue = nnCqueueInit(CQUEUE_DEFAULT_COUNT,
                                           CQUEUE_NOT_DUPLICATED, TEST_TYPE);

                    if ((Int32T)pCqueue <= 0)
                    {
                        printf("Error nnCqueueInit() : %s\n",
                               cqueueErrMsg[(-(Int32T)pCqueue)]);
                        pCqueue = NULL;
                        break;
                    }
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = CHECK_DATA;

                errno = nnCqueueEnqueue(pCqueue, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnCqueueEnqueue() : %s\n",
                           cqueueErrMsg[(-(errno))]);

                    break;
                }

                errno = nnCqueueEnqueue(pCqueue, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnCqueueEnqueue() : %s\n",
                           cqueueErrMsg[(-(errno))]);

                    break;
                }

                cqueuePrint(pCqueue);
                break;
            case 8:
                printf("8. nnCqueueEnqueue Test\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                if (pCqueue == NULL)
                {
                    pCqueue = nnCqueueInit(CQUEUE_DEFAULT_COUNT,
                                           CQUEUE_NOT_DUPLICATED, TEST_TYPE);

                    if ((Int32T)pCqueue <= 0)
                    {
                        printf("Error nnCqueueInit() : %s\n",
                               cqueueErrMsg[(-(Int32T)pCqueue)]);
                        pCqueue = NULL;
                        break;
                    }
                }

                for (cqueueIndex = 0;
                     cqueueIndex < pTempTest->m3;
                     ++cqueueIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = cqueueIndex + 1;

                    errno = nnCqueueEnqueue(pCqueue, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnCqueueEnqueue() : %s\n",
                               cqueueErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                cqueuePrint(pCqueue);
                break;
            case 9:
                printf("9. nnCqueueGetHead Test(Cqueue NULL)\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                pTestData = nnCqueueGetHead(pCqueue);
                if ((Int32T)pTestData == 0)
                {
                    printf("HeadNode = NULL\n");
                }
                else if ((Int32T)pTestData < 0)
                {
                    printf("Error nnCqueueGetHead() : %s\n",
                           cqueueErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    printf("Print HeadNode's Data : %d\n",
                           (((testT *)pTestData)->data));
                }

                break;
            case 10:
                printf("10. nnCqueueGetHead Test(No Data)\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                if (pCqueue == NULL)
                {
                    pCqueue = nnCqueueInit(CQUEUE_DEFAULT_COUNT,
                                           CQUEUE_NOT_DUPLICATED, TEST_TYPE);

                    if ((Int32T)pCqueue <= 0)
                    {
                        printf("Error nnCqueueInit() : %s\n",
                               cqueueErrMsg[(-(Int32T)pCqueue)]);
                        pCqueue = NULL;
                        break;
                    }
                }

                pTestData = nnCqueueGetHead(pCqueue);
                if ((Int32T)pTestData == 0)
                {
                    printf("HeadNode = NULL\n");
                }
                else if ((Int32T)pTestData < 0)
                {
                    printf("Error nnCqueueGetHead() : %s\n",
                           cqueueErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    printf("Print HeadNode's Data : %d\n",
                           (((testT *)pTestData)->data));
                }

                break;
            case 11:
                printf("11. nnCqueueGetHead Test\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                if (pCqueue == NULL)
                {
                    pCqueue = nnCqueueInit(CQUEUE_DEFAULT_COUNT,
                                           CQUEUE_NOT_DUPLICATED, TEST_TYPE);

                    if ((Int32T)pCqueue <= 0)
                    {
                        printf("Error nnCqueueInit() : %s\n",
                               cqueueErrMsg[(-(Int32T)pCqueue)]);
                        pCqueue = NULL;
                        break;
                    }
                }

                for (cqueueIndex = 0;
                     cqueueIndex < pTempTest->m3;
                     ++cqueueIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = cqueueIndex + 1;

                    errno = nnCqueueEnqueue(pCqueue, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnCqueueEnqueue() : %s\n",
                               cqueueErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                pTestData = nnCqueueGetHead(pCqueue);
                if ((Int32T)pTestData == 0)
                {
                    printf("HeadNode = NULL\n");
                }
                else if ((Int32T)pTestData < 0)
                {
                    printf("Error nnCqueueGetHead() : %s\n",
                           cqueueErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    printf("Print HeadNode's Data : %d\n",
                           (((testT *)pTestData)->data));
                }

                break;
            case 12:
                printf("12. nnCqueueGetTail Test(Cqueue NULL)\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                pTestData = nnCqueueGetTail(pCqueue);
                if ((Int32T)pTestData == 0)
                {
                    printf("TailNode = NULL\n");
                }
                else if ((Int32T)pTestData < 0)
                {
                    printf("Error nnCqueueGetTail() : %s\n",
                           cqueueErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    printf("Print TailNode's Data : %d\n",
                           (((testT *)pTestData)->data));
                }

                break;
            case 13:
                printf("13. nnCqueueGetTail Test(No Data)\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                if (pCqueue == NULL)
                {
                    pCqueue = nnCqueueInit(CQUEUE_DEFAULT_COUNT,
                              CQUEUE_NOT_DUPLICATED, TEST_TYPE);

                    if ((Int32T)pCqueue <= 0)
                    {
                        printf("Error nnCqueueInit() : %s\n",
                               cqueueErrMsg[(-(Int32T)pCqueue)]);
                        pCqueue = NULL;
                        break;
                    }
                }

                pTestData = nnCqueueGetTail(pCqueue);
                if ((Int32T)pTestData == 0)
                {
                    printf("TailNode = NULL\n");
                }
                else if ((Int32T)pTestData < 0)
                {
                    printf("Error nnCqueueGetTail() : %s\n",
                           cqueueErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    printf("Print TailNode's Data : %d\n",
                           (((testT *)pTestData)->data));
                }

                break;
            case 14:
                printf("14. nnCqueueGetTail Test\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                if (pCqueue == NULL)
                {
                    pCqueue = nnCqueueInit(CQUEUE_DEFAULT_COUNT,
                                           CQUEUE_NOT_DUPLICATED, TEST_TYPE);

                    if ((Int32T)pCqueue <= 0)
                    {
                        printf("Error nnCqueueInit() : %s\n",
                               cqueueErrMsg[(-(Int32T)pCqueue)]);
                        pCqueue = NULL;
                        break;
                    }
                }

                for (cqueueIndex = 0;
                     cqueueIndex < pTempTest->m3;
                     ++cqueueIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = cqueueIndex + 1;

                    errno = nnCqueueEnqueue(pCqueue, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnCqueueEnqueue() : %s\n",
                               cqueueErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                pTestData = nnCqueueGetTail(pCqueue);
                if ((Int32T)pTestData == 0)
                {
                    printf("TailNode = NULL\n");
                }
                else if ((Int32T)pTestData < 0)
                {
                    printf("Error nnCqueueGetTail() : %s\n",
                           cqueueErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    printf("Print TailNode's Data : %d\n",
                           (((testT *)pTestData)->data));
                }

                break;
            case 15:
                printf("12. nnCqueueCount Test(Cqueue NULL)\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                errno = nnCqueueCount(pCqueue);

                if (errno < 0)
                {
                    printf("Error nnCqueueCount() : %s\n",
                           cqueueErrMsg[(-(errno))]);
                }
                else
                {
                    printf("Cqueue Count : %d\n", errno);
                }

                break;
            case 16:
                printf("16. nnCqueueCount Test(No Data)\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                if (pCqueue == NULL)
                {
                    pCqueue = nnCqueueInit(CQUEUE_DEFAULT_COUNT,
                                           CQUEUE_NOT_DUPLICATED, TEST_TYPE);

                    if ((Int32T)pCqueue <= 0)
                    {
                        printf("Error nnCqueueInit() : %s\n",
                               cqueueErrMsg[(-(Int32T)pCqueue)]);
                        pCqueue = NULL;
                        break;
                    }
                }

                errno = nnCqueueCount(pCqueue);

                if (errno < 0)
                {
                    printf("Error nnCqueueCount() : %s\n",
                           cqueueErrMsg[(-(errno))]);
                }
                else
                {
                    printf("Cqueue Count : %d\n", errno);
                }

                break;
            case 17:
                printf("17. nnCqueueCount Test\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                if (pCqueue == NULL)
                {
                    pCqueue = nnCqueueInit(CQUEUE_DEFAULT_COUNT,
                                           CQUEUE_NOT_DUPLICATED, TEST_TYPE);

                    if ((Int32T)pCqueue <= 0)
                    {
                        printf("Error nnCqueueInit() : %s\n",
                               cqueueErrMsg[(-(Int32T)pCqueue)]);
                        pCqueue = NULL;
                        break;
                    }
                }

                for (cqueueIndex = 0;
                     cqueueIndex < pTempTest->m3;
                     ++cqueueIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = cqueueIndex + 1;

                    errno = nnCqueueEnqueue(pCqueue, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnCqueueEnqueue() : %s\n",
                               cqueueErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                errno = nnCqueueCount(pCqueue);

                if (errno < 0)
                {
                    printf("Error nnCqueueCount() : %s\n",
                           cqueueErrMsg[(-(errno))]);
                }
                else
                {
                    printf("Cqueue Count : %d\n", errno);
                }

                break;
            case 18:
                printf("18. nnCqueueFree Test(Cqueue NULL)\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                nnCqueueFree(pCqueue);
                pCqueue = NULL;

                break;
            case 19:
                printf("19. nnCqueueFree Test(No Data)\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                if (pCqueue == NULL)
                {
                    pCqueue = nnCqueueInit(CQUEUE_DEFAULT_COUNT,
                                           CQUEUE_NOT_DUPLICATED, TEST_TYPE);

                    if ((Int32T)pCqueue <= 0)
                    {
                        printf("Error nnCqueueInit() : %s\n",
                               cqueueErrMsg[(-(Int32T)pCqueue)]);
                        pCqueue = NULL;
                        break;
                    }
                }

                nnCqueueFree(pCqueue);
                pCqueue = NULL;

                break;
            case 20:
                printf("20. nnCqueueFree Test(No Data)\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                if (pCqueue == NULL)
                {
                    pCqueue = nnCqueueInit(CQUEUE_DEFAULT_COUNT,
                                           CQUEUE_NOT_DUPLICATED, TEST_TYPE);

                    if ((Int32T)pCqueue <= 0)
                    {
                        printf("Error nnCqueueInit() : %s\n",
                               cqueueErrMsg[(-(Int32T)pCqueue)]);
                        pCqueue = NULL;
                        break;
                    }
                }

                for (cqueueIndex = 0;
                     cqueueIndex < pTempTest->m3;
                     ++cqueueIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = cqueueIndex + 1;

                    errno = nnCqueueEnqueue(pCqueue, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnCqueueEnqueue() : %s\n",
                               cqueueErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                nnCqueueFree(pCqueue);
                pCqueue = NULL;

                break;
            case 21:
                printf("21. nnCqueueDequeue Test(Cqueue NULL)\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                pTestData = nnCqueueDequeue(pCqueue);
                if ((Int32T)pTestData == 0)
                {
                    printf("DequeueNode = NULL\n");
                }
                else if ((Int32T)pTestData < 0)
                {
                    printf("Error nnCqueueDequeue() : %s\n",
                           cqueueErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    printf("Print DequeueNode's Data : %d\n",
                           (((testT *)pTestData)->data));
                }

                break;
            case 22:
                printf("22. nnCqueueDequeue Test(No Data)\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                if (pCqueue == NULL)
                {
                    pCqueue = nnCqueueInit(CQUEUE_DEFAULT_COUNT,
                                           CQUEUE_NOT_DUPLICATED, TEST_TYPE);

                    if ((Int32T)pCqueue <= 0)
                    {
                        printf("Error nnCqueueInit() : %s\n",
                               cqueueErrMsg[(-(Int32T)pCqueue)]);
                        pCqueue = NULL;
                        break;
                    }
                }

                pTestData = nnCqueueDequeue(pCqueue);
                if ((Int32T)pTestData == 0)
                {
                    printf("DequeueNode = NULL\n");
                }
                else if ((Int32T)pTestData < 0)
                {
                    printf("Error nnCqueueDequeue() : %s\n",
                           cqueueErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    printf("Print DequeueNode's Data : %d\n",
                           (((testT *)pTestData)->data));
                }

                break;
            case 23:
                printf("23. nnCqueueDequeue Test\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                if (pCqueue == NULL)
                {
                    pCqueue = nnCqueueInit(CQUEUE_DEFAULT_COUNT,
                                           CQUEUE_NOT_DUPLICATED, TEST_TYPE);

                    if ((Int32T)pCqueue <= 0)
                    {
                        printf("Error nnCqueueInit() : %s\n",
                               cqueueErrMsg[(-(Int32T)pCqueue)]);
                        pCqueue = NULL;
                        break;
                    }
                }

                for (cqueueIndex = 0;
                     cqueueIndex < pTempTest->m3;
                     ++cqueueIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = cqueueIndex + 1;

                    errno = nnCqueueEnqueue(pCqueue, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnCqueueEnqueue() : %s\n",
                               cqueueErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                pTestData = nnCqueueDequeue(pCqueue);
                if ((Int32T)pTestData == 0)
                {
                    printf("DequeueNode = NULL\n");
                }
                else if ((Int32T)pTestData < 0)
                {
                    printf("Error nnCqueueDequeue() : %s\n",
                           cqueueErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    printf("Print DequeueNode's Data : %d\n",
                           (((testT *)pTestData)->data));
                    NNFREE(TEST_TYPE, pTestData);
                }

                break;
            case 24:
                printf("24. nnCqueueDeleteAllNode Test(Cqueue NULL)\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                nnCqueueDeleteAllNode(pCqueue);

                break;
            case 25:
                printf("25. nnCqueueDeleteAllNode Test(No Data)\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                if (pCqueue == NULL)
                {
                    pCqueue = nnCqueueInit(CQUEUE_DEFAULT_COUNT,
                                           CQUEUE_NOT_DUPLICATED, TEST_TYPE);

                    if ((Int32T)pCqueue <= 0)
                    {
                        printf("Error nnCqueueInit() : %s\n",
                               cqueueErrMsg[(-(Int32T)pCqueue)]);
                        pCqueue = NULL;
                        break;
                    }
                }

                nnCqueueDeleteAllNode(pCqueue);

                break;
            case 26:
                printf("26. nnCqueueDeleteAllNode Test\n");

                if (pCqueue != NULL)
                {
                    nnCqueueFree(pCqueue);
                    pCqueue = NULL;
                }

                if (pCqueue == NULL)
                {
                    pCqueue = nnCqueueInit(CQUEUE_DEFAULT_COUNT,
                                           CQUEUE_NOT_DUPLICATED, TEST_TYPE);

                    if ((Int32T)pCqueue <= 0)
                    {
                        printf("Error nnCqueueInit() : %s\n",
                               cqueueErrMsg[(-(Int32T)pCqueue)]);
                        pCqueue = NULL;
                        break;
                    }
                }

                for (cqueueIndex = 0;
                     cqueueIndex < pTempTest->m3;
                     ++cqueueIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = cqueueIndex + 1;

                    errno = nnCqueueEnqueue(pCqueue, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnCqueueEnqueue() : %s\n",
                               cqueueErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                nnCqueueDeleteAllNode(pCqueue);

                break;
            case 27:
                printf("27. nnCqueueEnqueue\n");

                if (pCqueue == NULL)
                {
                    pCqueue = nnCqueueInit(CQUEUE_DEFAULT_COUNT,
                                           CQUEUE_NOT_DUPLICATED, TEST_TYPE);

                    if ((Int32T)pCqueue <= 0)
                    {
                        printf("Error nnCqueueInit() : %s\n",
                               cqueueErrMsg[(-(Int32T)pCqueue)]);
                        pCqueue = NULL;
                        break;
                    }
                }

                for (cqueueIndex = 0;
                     cqueueIndex < pTempTest->m3;
                     ++cqueueIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = cqueueIndex + 1;

                    errno = nnCqueueEnqueue(pCqueue, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnCqueueEnqueue() : %s\n",
                               cqueueErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                break;
            case 28:
                printf("28. nnCqueueDequeue\n");

                if (pCqueue == NULL)
                {
                    pCqueue = nnCqueueInit(CQUEUE_DEFAULT_COUNT,
                                           CQUEUE_NOT_DUPLICATED, TEST_TYPE);

                    if ((Int32T)pCqueue <= 0)
                    {
                        printf("Error nnCqueueInit() : %s\n",
                               cqueueErrMsg[(-(Int32T)pCqueue)]);
                        pCqueue = NULL;
                        break;
                    }
                }

                if (pCqueue->head != NULL)
                {
                    for (cqueueIndex = 0;
                         cqueueIndex < pTempTest->m3;
                         ++cqueueIndex)
                    {
                        pTestData = nnCqueueDequeue(pCqueue);
                        if ((Int32T)pTestData == 0)
                        {
                            printf("DequeueNode = NULL\n");
                            break;
                        }
                        else if ((Int32T)pTestData < 0)
                        {
                            printf("Error nnCqueueDequeue() : %s\n",
                                   cqueueErrMsg[(-(Int32T)pTestData)]);
                            break;
                        }
                        else
                        {
                            NNFREE(TEST_TYPE, pTestData);
                        }
                    }
                }

                break;
            case 29:
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

    if (pCqueue)
    {
        nnCqueueFree(pCqueue);
        pCqueue = NULL;
    }

    NNLOG(LOG_DEBUG, "Cqueue Test End\n");

    nnLogClose();

    printf("============== memShowAllSystem ===============\n");
    memShowAllSystem(MEM_MAX_SYS_CNT);
    printf("============== memShowAllUse ===============\n");
    memShowAllUser(TEST_TYPE);

    memClose();

    freeTestHead(pTestHead);

    return 0;
}

Int32T cqueuePrint(CqueueT * pCqueue)
{
    CqueueNodeT * tempNode = NULL;

    if (pCqueue == NULL)
    {
        printf("Cqueue is NULL\n");
        return -1;
    }

    tempNode = pCqueue->head;

    if(tempNode == NULL)
    {
        printf("no data\n");
        return 0;
    }

    printf("Datas : ");
    for(; tempNode != pCqueue->tail; tempNode = tempNode->next)
    {
        printf("%d ", ((testT *)tempNode->data)->data);
    }
    printf("%d\n", ((testT *)tempNode->data)->data);

    printf("pCqueueSize : %llu\n\n", nnCqueueCount(pCqueue));

    return 0;
}


