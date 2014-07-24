#include <time.h>
#include "../nnAvl.h"
#include "../nnLog.h"
#include "../nnMemmgr.h"
#include "test.h"

#define TEST_PROGRAM 0
#define TEST_TYPE 0
#define ADD_COUNT 10
#define CHECK_DATA 5

StringT avlErrMsg[] = {"SUCCESS", "AVL_ERR_AVL_ALLOC",
                        "AVL_ERR_AVLNODE_ALLOC", "AVL_ERR_COMP_FUNC",
                        "AVL_ERR_NULL", "AVL_ERR_EMPTY", "AVL_ERR_ARGUMENT"};

/* Test Structure */
typedef struct testT {
    int data;
} testT;

void avlPrint(AvlNodeT * pAvlNode);

static Int32T menu = 0;
static testT *pTestData = NULL;
static testT *pTestData2 = NULL;
static Uint32T avlIndex = 0;

/* Data Compare Function */
Int8T dataCmpFunc(void *pOldData, void *pNewData)
{
    /* if sort then ASC */
    if ((((testT *)pOldData)->data) > (((testT *)pNewData)->data))
    {
        return 0;
    }
    /* if sort then DESC */
    else if ((((testT *)pOldData)->data) < (((testT *)pNewData)->data))
    {
        return 1;
    }
    /* same data */
    else /* if (((testT *)pOldData)->data == ((testT *)pNewData)->data) */
    {
        return -1;
    }
}

void printMenu()
{
    printf("==========================================\n");
    printf("           Avl Test Menu\n");
    printf("1. nnAvlInit Test(Param DataCmp NULL)\n");
    printf("2. nnAvlInit Test(Param Wrong dataType)\n");
    printf("3. nnAvlInit Test\n");

    printf("4. nnAvlAddNode Test(Avl NULL)\n");
    printf("5. nnAvlAddNode Test(Param Data NULL)\n");
    printf("6. nnAvlAddNode Test\n");

    printf("7. nnAvlGetRootData Test(Avl NULL)\n");
    printf("8. nnAvlGetRootData Test(No Data NULL)\n");
    printf("9. nnAvlGetRootData Test(Exist Data)\n");

    printf("10. nnAvlCount Test(Avl NULL)\n");
    printf("11. nnAvlCount Test(No Data)\n");
    printf("12. nnAvlCount Test(Exist Data)\n");

    printf("13. nnAvlLookupData Test(Avl NULL)\n");
    printf("14. nnAvlLookupData Test(No Set Comp Function NULL)\n");
    printf("15. nnAvlLookupData Test(Param Data NULL)\n");
    printf("16. nnAvlLookupData Test(No Data)\n");
    printf("17. nnAvlLookupData Test(Not Exist Data)\n");
    printf("18. nnAvlLookupData Test\n");

    printf("19. nnAvlFree Test(Avl NULL)\n");
    printf("20. nnAvlFree Test(No Data)\n");
    printf("21. nnAvlFree Test\n");

    printf("22. nnAvlDeleteNode Test(Avl NULL)\n");
    printf("23. nnAvlDeleteNode Test(No Set Comp Function NULL)\n");
    printf("24. nnAvlDeleteNode Test(Param Data NULL)\n");
    printf("25. nnAvlDeleteNode Test(No Data)\n");
    printf("26. nnAvlDeleteNode Test(Not Exist Data)\n");
    printf("27. nnAvlDeleteNode Test\n");

    printf("28. nnAvlDeleteAllNode Test(Avl NULL)\n");
    printf("29. nnAvlDeleteAllNode Test(No Data)\n");
    printf("30. nnAvlDeleteAllNode Test\n");
//////////////// Not Use New Avl - Add and Delete
// 계속적으로 등록/삭제를 수행 할 때 사용
    printf("31. nnAvlAddNode Test\n");
    printf("32. nnAvlDeleteNode Test\n");
    printf("33. Exit\n");
    printf("==========================================\n");
}

int main() 
{
    AvlTreeT *pAvl = NULL;
    Int32T errno = 0;

    double start = 0;

    TestHead *pTestHead = (TestHead *)malloc(sizeof(TestHead));
    Test *pTempTest = NULL;

    pTestHead->pHead = NULL;
    pTestHead->pTail = NULL;
    pTestHead->count = 0;

    errno = getFileData(pTestHead, "avlConf");
    if (errno == FAILURE)
    {
        printf("FAILURE!\n");
        return -1;
    }

    memInit(1);
    memSetDebug(TEST_TYPE);

    nnLogInit(1);

    NNLOG(LOG_DEBUG, "Avl Test Start\n");

    start = clock();

    pTempTest = pTestHead->pHead;
    while(pTestHead->count > 0)
    {
        menu = pTempTest->m1;

        switch(menu)
        {
            case 1:
                printf("1. nnAvlInit Test(Param DataCmp NULL)\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                printf("Avl Address : %d\n", (Int32T)pAvl);
                printf("nnAvlInit()\n");

                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(NULL, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;
                    }
                }

                printf("Avl Address : %d\n", (Int32T)pAvl);

                break;
            case 2:
                printf("2. nnAvlInit Test(Param Wrong dataType)\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                printf("Avl Address : %d\n", (Int32T)pAvl);
                printf("nnAvlInit()\n");
        
                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;
                    }
                }

                printf("Avl Address : %d\n", (Int32T)pAvl);

                break;
            case 3:
                printf("3. nnAvlInit Test\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                printf("Avl Address : %d\n", (Int32T)pAvl);
                printf("nnAvlInit()\n");
        
                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;
                    }
                }

                printf("Avl Address : %d\n", (Int32T)pAvl);

                break;
            case 4:
                printf("4. nnAvlAddNode Test(Avl NULL)\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                for (avlIndex = 0; avlIndex < pTempTest->m3; ++avlIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = avlIndex;

                    errno = nnAvlAddNode(pAvl, pTestData);
                    if(errno != SUCCESS)
                    {
                        printf("Error nnAvlAddNode() : %s\n",
                               avlErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                break;
            case 5:
                printf("5. nnAvlAddNode Test(Param Data NULL)\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                pAvl = nnAvlInit(dataCmpFunc, TEST_TYPE);

                if ((Int32T)pAvl <= 0)
                {
                    printf("Error nnAvlInit() : %s\n",
                           avlErrMsg[(-(Int32T)pAvl)]);
                    pAvl = NULL;

                    break;
                }

                pTestData = NULL;
                errno = nnAvlAddNode(pAvl, pTestData);

                if(errno != SUCCESS)
                {
                    printf("Error nnAvlAddNode() : %s\n",
                           avlErrMsg[(-(errno))]);
                }

                break;
            case 6:
                printf("5. nnAvlAddNode Test\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;

                        break;
                    }
                }

                for (avlIndex = 0; avlIndex < pTempTest->m3; ++avlIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = avlIndex;

                    errno = nnAvlAddNode(pAvl, pTestData);
                    if(errno != SUCCESS)
                    {
                        printf("Error nnAvlAddNode() : %s\n",
                               avlErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                avlPrint(pAvl->pRoot);
                break;
            case 7:
                printf("7. nnAvlGetRootData Test(Avl NULL)\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                pTestData = nnAvlGetRootData(pAvl);

                if ((Int32T)pTestData == 0)
                {
                    printf("HeadNode = NULL\n");
                }
                else if ((Int32T)pTestData < 0)
                {
                    printf("Error nnAvlGetRootData() : %s\n",
                           avlErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    printf("Print Root Data : %d\n",
                           ((testT *)(pTestData))->data);
                }

                break;
            case 8:
                printf("8. nnAvlGetRootData Test(No Data)\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;

                        break;
                    }
                }

                pTestData = nnAvlGetRootData(pAvl);

                if ((Int32T)pTestData == 0)
                {
                    printf("HeadNode = NULL\n");
                }
                else if ((Int32T)pTestData < 0)
                {
                    printf("Error nnAvlGetRootData() : %s\n",
                           avlErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    printf("Print Root Data : %d\n",
                           ((testT *)(pTestData))->data);
                }

                break;
            case 9:
                printf("9. nnAvlGetRootData Test\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;

                        break;
                    }
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = CHECK_DATA;

                errno = nnAvlAddNode(pAvl, pTestData);
                if(errno != SUCCESS)
                {
                    printf("Error nnAvlAddNode() : %s\n",
                           avlErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                pTestData = nnAvlGetRootData(pAvl);

                if ((Int32T)pTestData == 0)
                {
                    printf("HeadNode = NULL\n");
                }
                else if ((Int32T)pTestData < 0)
                {
                    printf("Error nnAvlGetRootData() : %s\n",
                           avlErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    printf("Print Root Data : %d\n",
                           ((testT *)(pTestData))->data);
                }

                break;
            case 10:
                printf("10. nnAvlCount(Avl NULL) Test\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                errno = nnAvlCount(pAvl);

                if (errno < 0)
                {
                    printf("Error nnAvlCount() : %s\n",
                           avlErrMsg[(-(errno))]);
                }
                else
                {
                    printf("AvlCount : %d\n", errno);
                }

                break;
            case 11:
                printf("11. nnAvlCount(No Data) Test\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;

                        break;
                    }
                }

                errno = nnAvlCount(pAvl);

                if (errno < 0)
                {
                    printf("Error nnAvlCount() : %s\n", avlErrMsg[(-(errno))]);
                }
                else
                {
                    printf("AvlCount : %d\n", errno);
                }

                break;
            case 12:
                printf("12. nnAvlCount(Exist Data) Test\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;

                        break;
                    }
                }

                for (avlIndex = 0; avlIndex < pTempTest->m3; ++avlIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = avlIndex;

                    errno = nnAvlAddNode(pAvl, pTestData);
                    if(errno != SUCCESS)
                    {
                        printf("Error nnAvlAddNode() : %s\n",
                               avlErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                errno = nnAvlCount(pAvl);

                if (errno < 0)
                {
                    printf("Error nnAvlCount() : %s\n", avlErrMsg[(-(errno))]);
                }
                else
                {
                    printf("AvlCount : %d\n", errno);
                }

                break;
            case 13:
                printf("13. nnAvlLookupData(Avl NULL) Test\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                pTestData = NULL;
                pTestData2 = nnAvlLookupData(pAvl, pTestData);

                if ((Int32T)pTestData2 == 0)
                {
                    printf("LookupData = NULL\n");
                }
                else if ((Int32T)pTestData2 < 0)
                {
                    printf("Error nnAvlLookupData() : %s\n",
                           avlErrMsg[(-((Int32T)pTestData2))]);
                }
                else
                {
                    printf("Print Lookup Data : %d\n",
                           ((testT *)(pTestData2))->data);
                }

                break;
            case 14:
                printf("14. nnAvlLookupData(No Set Comp Function NULL) Test\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(NULL, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;

                        break;
                    }
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = CHECK_DATA;

                errno = nnAvlAddNode(pAvl, pTestData);
                if(errno != SUCCESS)
                {
                    printf("Error nnAvlAddNode() : %s\n",
                           avlErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                pTestData2 = nnAvlLookupData(pAvl, pTestData);

                NNFREE(TEST_TYPE, pTestData);

                if ((Int32T)pTestData2 == 0)
                {
                    printf("LookupData = NULL\n");
                }
                else if ((Int32T)pTestData2 < 0)
                {
                    printf("Error nnAvlLookupData() : %s\n",
                           avlErrMsg[(-((Int32T)pTestData2))]);
                }
                else
                {
                    printf("Print Lookup Data : %d\n",
                           ((testT *)(pTestData2))->data);
                }

                break;
            case 15:
                printf("15. nnAvlLookupData(Param Data NULL) Test\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;

                        break;
                    }
                }

                pTestData2 = nnAvlLookupData(pAvl, NULL);

                if ((Int32T)pTestData2 == 0)
                {
                    printf("LookupData = NULL\n");
                }
                else if ((Int32T)pTestData2 < 0)
                {
                    printf("Error nnAvlLookupData() : %s\n",
                           avlErrMsg[(-((Int32T)pTestData2))]);
                }
                else
                {
                    printf("Print Lookup Data : %d\n",
                           ((testT *)(pTestData2))->data);
                }

                break;
            case 16:
                printf("16. nnAvlLookupData(No Data) Test\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;

                        break;
                    }
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = CHECK_DATA;

                pTestData2 = nnAvlLookupData(pAvl, pTestData);

                NNFREE(TEST_TYPE, pTestData);

                if ((Int32T)pTestData2 == 0)
                {
                    printf("LookupData = NULL\n");
                }
                else if ((Int32T)pTestData2 < 0)
                {
                    printf("Error nnAvlLookupData() : %s\n",
                           avlErrMsg[(-((Int32T)pTestData2))]);
                }
                else
                {
                    printf("Print Lookup Data : %d\n",
                           ((testT *)(pTestData2))->data);
                }

                break;
            case 17:
                printf("17. nnAvlLookupData(Not Exist Data) Test\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;

                        break;
                    }
                }

                for (avlIndex = 0; avlIndex < pTempTest->m3; ++avlIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = avlIndex;

                    errno = nnAvlAddNode(pAvl, pTestData);
                    if(errno != SUCCESS)
                    {
                        printf("Error nnAvlAddNode() : %s\n",
                               avlErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = ADD_COUNT + 1;

                pTestData2 = nnAvlLookupData(pAvl, pTestData);

                NNFREE(TEST_TYPE, pTestData);

                if ((Int32T)pTestData2 == 0)
                {
                    printf("LookupData = NULL\n");
                }
                else if ((Int32T)pTestData2 < 0)
                {
                    printf("Error nnAvlLookupData() : %s\n",
                           avlErrMsg[(-((Int32T)pTestData2))]);
                }
                else
                {
                    printf("Print Lookup Data : %d\n",
                           ((testT *)(pTestData2))->data);
                }

                break;
            case 18:
                printf("18. nnAvlLookupData Test\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;

                        break;
                    }
                }

                for (avlIndex = 0; avlIndex < pTempTest->m3; ++avlIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = avlIndex;

                    errno = nnAvlAddNode(pAvl, pTestData);
                    if(errno != SUCCESS)
                    {
                        printf("Error nnAvlAddNode() : %s\n",
                               avlErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = CHECK_DATA;

                pTestData2 = nnAvlLookupData(pAvl, pTestData);

                NNFREE(TEST_TYPE, pTestData);

                if ((Int32T)pTestData2 == 0)
                {
                    printf("LookupData = NULL\n");
                }
                else if ((Int32T)pTestData2 < 0)
                {
                    printf("Error nnAvlLookupData() : %s\n",
                           avlErrMsg[(-((Int32T)pTestData2))]);
                }
                else
                {
                    printf("Print Lookup Data : %d\n",
                           ((testT *)(pTestData2))->data);
                }

                break;
            case 19:
                printf("19. nnAvlFree Test(Avl NULL)\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                nnAvlFree(pAvl);
                pAvl = NULL;

                break;

            case 20:
                printf("20. nnAvlFree Test(No Data)\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;

                        break;
                    }
                }

                nnAvlFree(pAvl);
                pAvl = NULL;

                break;
            case 21:
                printf("21. nnAvlFree Test\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;

                        break;
                    }
                }

                for (avlIndex = 0; avlIndex < pTempTest->m3; ++avlIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = avlIndex;

                    errno = nnAvlAddNode(pAvl, pTestData);
                    if(errno != SUCCESS)
                    {
                        printf("Error nnAvlAddNode() : %s\n",
                               avlErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                nnAvlFree(pAvl);
                pAvl = NULL;

                break;
            case 22:
                printf("22. nnAvlDeleteNode Test(Avl NULL)\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                pTestData = NULL;
                nnAvlDeleteNode(pAvl, pTestData);

                break;
            case 23:
                printf("23. nnAvlDeleteNode Test(No Set Comp Function NULL)\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(NULL, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;

                        break;
                    }
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = CHECK_DATA;

                nnAvlDeleteNode(pAvl, pTestData);

                NNFREE(TEST_TYPE, pTestData);

                break;
            case 24:
                printf("24. nnAvlDeleteNode Test(Param Data NULL)\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;

                        break;
                    }
                }

                nnAvlDeleteNode(pAvl, NULL);

                break;
            case 25:
                printf("25. nnAvlDeleteNode Test(No Data)\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;

                        break;
                    }
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = CHECK_DATA;

                nnAvlDeleteNode(pAvl, pTestData);

                NNFREE(TEST_TYPE, pTestData);

                break;
            case 26:
                printf("26. nnAvlDeleteNode Test(Not Exist Data)\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;

                        break;
                    }
                }

                for (avlIndex = 0; avlIndex < pTempTest->m3; ++avlIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = avlIndex;

                    errno = nnAvlAddNode(pAvl, pTestData);
                    if(errno != SUCCESS)
                    {
                        printf("Error nnAvlAddNode() : %s\n",
                               avlErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = ADD_COUNT;

                nnAvlDeleteNode(pAvl, pTestData);

                NNFREE(TEST_TYPE, pTestData);

                break;
            case 27:
                printf("27. nnAvlDeleteNode Test\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;

                        break;
                    }
                }

                for (avlIndex = 0; avlIndex < pTempTest->m3; ++avlIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = avlIndex;

                    errno = nnAvlAddNode(pAvl, pTestData);
                    if(errno != SUCCESS)
                    {
                        printf("Error nnAvlAddNode() : %s\n",
                               avlErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                nnAvlDeleteNode(pAvl, pTestData);

                break;
            case 28:
                printf("28. nnAvlDeleteAllNode Test(Avl NULL)\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                nnAvlDeleteAllNode(pAvl);

                break;
            case 29:
                printf("29. nnAvlDeleteAllNode Test(No Data)\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;

                        break;
                    }
                }

                nnAvlDeleteAllNode(pAvl);

                break;
            case 30:
                printf("30. nnAvlDeleteAllNode Test\n");

                if (pAvl != NULL)
                {
                    nnAvlFree(pAvl);
                    pAvl = NULL;
                }

                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;

                        break;
                    }
                }

                for (avlIndex = 0; avlIndex < pTempTest->m3; ++avlIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = avlIndex;

                    errno = nnAvlAddNode(pAvl, pTestData);
                    if(errno != SUCCESS)
                    {
                        printf("Error nnAvlAddNode() : %s\n",
                               avlErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                nnAvlDeleteAllNode(pAvl);

                break;
            case 31:
                printf("31. Add Test\n");

                if (pAvl == NULL)
                {
                    pAvl = nnAvlInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pAvl <= 0)
                    {
                        printf("Error nnAvlInit() : %s\n",
                               avlErrMsg[(-(Int32T)pAvl)]);
                        pAvl = NULL;

                        break;
                    }
                }

                for (avlIndex = 0; avlIndex < pTempTest->m3; ++avlIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = avlIndex;
                    nnAvlAddNode(pAvl, pTestData);
                }

                printf("Add End\n");

                break;
            case 32:
                printf("32. Delete Test\n");

                for (avlIndex = 0; avlIndex < pTempTest->m3; ++avlIndex)
                {
                    pTestData = nnAvlGetRootData(pAvl);

                    if ((Int32T)pTestData <= 0)
                    {
                    }
                    else
                    {
                        nnAvlDeleteNode(pAvl, pTestData);
                    }
                }

                printf("Delete End\n");

                break;
            case 33:
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

    if (pAvl)
    {
        nnAvlFree(pAvl);
        pAvl = NULL;
    }

    NNLOG(LOG_DEBUG, "Avl Test End\n");

    nnLogClose();

    printf("============== memShowAllSystem ===============\n");
    memShowAllSystem(MEM_MAX_SYS_TYPE);
    printf("============== memShowAllUse ===============\n");
    memShowAllUser(MEM_MAX_USER_TYPE);

    /* Mem Close */
    memClose();

    freeTestHead(pTestHead);

    return 0;
}

/* Print Avl Test */
void avlPrint(AvlNodeT * pAvlNode)
{
    printf("ALV PRINT ========================\n");
    if (pAvlNode <= 0)
    {
        LOG("No Data");
    }
    else
    {
        if (pAvlNode->pParent != NULL)
        {
            printf("Parent = %d ", ((testT *)pAvlNode->pParent->pData)->data);
        }
        else
        {
            printf("Parent = NULL ");

            printf("Data = %d, Height = %d, Balance = %d\n",
                   ((testT *)pAvlNode->pData)->data,
                   (pAvlNode->height), (pAvlNode->balance));

            printf("Left\n");
            avlPrint(pAvlNode->pLeft);

            printf("Right\n");
            avlPrint(pAvlNode->pRight);
        }
    }
}
