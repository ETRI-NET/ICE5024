#include <time.h>
#include "../nnList.h"
#include "../nnLog.h"
#include "../nnMemmgr.h"
#include "test.h"

#define TEST_PROGRAM 0 
#define TEST_TYPE 0
#define ADD_COUNT 10
#define CHECK_DATA 5

StringT listErrMsg[] = {"SUCCESS", "LIST_ERR_LIST_ALLOC",
                        "LIST_ERR_LISTNODE_ALLOC", "LIST_ERR_NULL",
                        "LIST_ERR_DUP", "LIST_ERR_COMP_FUNC",
                        "LIST_ERR_LISTSUB_ALLOC",
                        "LIST_ERR_LISTSUBNODE_ALLOC", "LIST_ERR_ARGUMENT"};

/* Test Structure */
typedef struct testT {
    Int32T data;
} testT;

Int32T listPrint(ListT * pList);
Int32T listSubPrint(ListNodeT * pListNode);

Int32T menu = 0;
testT *pTestData = NULL;
testT *pTestData2 = NULL;
ListNodeT *pTestNode = NULL;
ListSubNodeT *pTestSubNode = NULL;
Uint32T listIndex = 0;

void printMenu()
{
    printf("==========================================\n");
    printf("           List Test Menu\n");
    printf("1. nnListInit Test(Param DataCmp NULL)\n");
    printf("2. nnListInit Test(Param Wrong dataType)\n");
    printf("3. nnListInit Test\n");

    printf("4. nnListAddNode Test(List NULL)\n");
    printf("5. nnListAddNode Test(Param Data NULL)\n");
    printf("6. nnListAddNode Test\n");

    printf("7. nnListAddNodeHead Test(List NULL)\n");
    printf("8. nnListAddNodeHead Test(Param Data NULL)\n");
    printf("9. nnListAddNodeHead Test\n");

    printf("10. nnListAddNodePrev Test(List NULL)\n");
    printf("11. nnListAddNodePrev Test(Param pData NULL)\n");
    printf("12. nnListAddNodePrev Test(Param pNode NULL)\n");
    printf("13. nnListAddNodePrev Test\n");

    printf("14. nnListAddNodeNext Test(List NULL)\n");
    printf("15. nnListAddNodeNext Test(Param pData NULL)\n");
    printf("16. nnListAddNodeNext Test(Param pNode NULL)\n");
    printf("17. nnListAddNodeNext Test\n");

    printf("18. nnListAddNodeDupNot Test(List NULL)\n");
    printf("19. nnListAddNodeDupNot Test(No Set Comp Function)\n");
    printf("20. nnListAddNodeDupNot Test(Param pData NULL)\n");
    printf("21. nnListAddNodeDupNot Test\n");

    printf("22. nnListCheckDupData Test(List NULL)\n");
    printf("23. nnListCheckDupData Test(No Set Comp Function)\n");
    printf("24. nnListCheckDupData Test(Param pData NULL)\n");
    printf("25. nnListCheckDupData Test\n");

    printf("26. nnListAddNodeSort Test(List NULL)\n");
    printf("27. nnListAddNodeSort Test(No Set Comp Function NULL)\n");
    printf("28. nnListAddNodeSort Test(Param pData NULL)\n");
    printf("29. nnListAddNodeSort Test\n");

    printf("30. nnListGetHeadNode Test(List NULL)\n");
    printf("31. nnListGetHeadNode Test(No Data)\n");
    printf("32. nnListGetHeadNode Test\n");

    printf("33. nnListGetTailNode Test(List NULL)\n");
    printf("34. nnListGetTailNode Test(No Data)\n");
    printf("35. nnListGetTailNode Test\n");

    printf("36. nnListCount Test(List NULL)\n");
    printf("37. nnListCount Test(No Data)\n");
    printf("38. nnListCount Test\n");

    printf("39. nnListSearchNode Test(List NULL)\n");
    printf("40. nnListSearchNode Test(No Set Comp Function NULL)\n");
    printf("41. nnListSearchNode Test(Param Data NULL)\n");
    printf("42. nnListSearchNode Test(No Data)\n");
    printf("43. nnListSearchNode Test(Not Exist Data)\n");
    printf("44. nnListSearchNode Test\n");

    printf("45. nnListFree Test(List NULL)\n");
    printf("46. nnListFree Test(No Data)\n");
    printf("47. nnListFree Test\n");

    printf("48. nnListDeleteNode Test(List NULL)\n");
    printf("49. nnListDeleteNode Test(Param Data NULL)\n");
    printf("50. nnListDeleteNode Test(No Data)\n");
    printf("51. nnListDeleteNode Test(Not Exist Data)\n");
    printf("52. nnListDeleteNode Test\n");

    printf("53. nnListDeleteAllNode Test(List NULL)\n");
    printf("54. nnListDeleteAllNode Test(No Data)\n");
    printf("55. nnListDeleteAllNode Test\n");

    printf("56. nnListSetNodeCompFunc Test(List NULL)\n");
    printf("57. nnListSetNodeCompFunc Test(Param Comp NULL)\n");
    printf("58. nnListSetNodeCompFunc Test\n");
//////////////// Sub List
    printf("59. SubList - nnListSubInit Test(ListNode NULL)\n");
    printf("60. SubList - nnListSubInit Test(Param DataCmp NULL)\n");
    printf("61. SubList - nnListSubInit Test(Param Wrong dataType)\n");
    printf("62. SubList - nnListSubInit Test\n");

    printf("63. SubList - nnListAddSubNode Test(ListNode NULL)\n");
    printf("64. SubList - nnListAddSubNode Test(Param pData NULL)\n");
    printf("65. SubList - nnListAddSubNode Test\n");

    printf("66. SubList - nnListAddSubNodeHead Test(ListNode NULL)\n");
    printf("67. SubList - nnListAddSubNodeHead Test(Param Data NULL)\n");
    printf("68. SubList - nnListAddSubNodeHead Test\n");

    printf("69. SubList - nnListGetHeadSubNode Test(ListNode NULL)\n");
    printf("70. SubList - nnListGetHeadSubNode Test(No Data)\n");
    printf("71. SubList - nnListGetHeadSubNode Test\n");

    printf("72. SubList - nnListSubCount Test(ListNode NULL)\n");
    printf("73. SubList - nnListSubCount Test(No Data)\n");
    printf("74. SubList - nnListSubCount Test\n");

    printf("75. SubList - nnListSearchSubNode Test(ListNode NULL)\n");
    printf(
        "76. SubList - nnListSearchSubNode Test(No Set Comp Function NULL)\n");
    printf("77. SubList - nnListSearchSubNode Test(Param Data NULL)\n");
    printf("78. SubList - nnListSearchSubNode Test(No Data)\n");
    printf("79. SubList - nnListSearchSubNode Test(Not Exist Data)\n");
    printf("80. SubList - nnListSearchSubNode Test\n");

    printf("81. SubList - nnListDeleteSubNode Test(ListNode NULL)\n");
    printf("82. SubList - nnListDeleteSubNode Test(Param Data NULL)\n");
    printf("83. SubList - nnListDeleteSubNode Test(No Data)\n");
    printf("84. SubList - nnListDeleteSubNode Test(Not Exist Data)\n");
    printf("85. SubList - nnListDeleteSubNode Test\n");

    printf("86. SubList - nnListDeleteAllSubNode Test(ListNode NULL)\n");
    printf("87. SubList - nnListDeleteAllSubNode Test(No Data)\n");
    printf("88. SubList - nnListDeleteAllSubNode Test\n");

    printf("89. SubList - nnListSetSubNodeCompFunc Test(List NULL)\n");
    printf("90. SubList - nnListSetSubNodeCompFunc Test(Param Comp NULL)\n");
    printf("91. SubList - nnListSetSubNodeCompFunc Test\n");
//////////////// Not Use New List - Add and Delete
// 계속적으로 등록/삭제를 수행 할 때 사용
    printf("92. nnListAddNode Test\n");
    printf("93. nnListDeleteNode Test\n");
    printf("94. Exit\n");
    printf("==========================================\n");
}

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

Int32T main() 
{
    ListT *pList = NULL;
    Int32T errno = 0;

    double start = 0;

    TestHead *pTestHead = (TestHead *)malloc(sizeof(TestHead));
    Test *pTempTest = NULL;

    pTestHead->pHead = NULL;
    pTestHead->pTail = NULL;
    pTestHead->count = 0;

    errno = getFileData(pTestHead, "listConf");
    if (errno == FAILURE)
    {
        printf("FAILURE\n");
        return -1;
    }

    memInit(1);
    memSetDebug(TEST_TYPE);

    nnLogInit(1);

    NNLOG(LOG_DEBUG, "List Test Start\n");

    start = clock();

    pTempTest = pTestHead->pHead;
    while(pTestHead->count > 0)
    {
        menu = pTempTest->m1;

        switch(menu)
        {
            case 1:
                printf("1. nnListInit Test(Param DataCmp NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                printf("List Address : %d\n", (Int32T)pList);
                printf("nnListInit()\n");

                pList = nnListInit(NULL, TEST_TYPE);

                if ((Int32T)pList <= 0)
                {
                    printf("Error nnListInit() : %s\n",
                           listErrMsg[(-(Int32T)pList)]);
                    pList = NULL;
                }

                printf("List Address : %d\n", (Int32T)pList);

                break;
            case 2:
                printf("2. nnListInit Test(Param Wrong dataType)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                printf("List Address : %d\n", (Int32T)pList);
                printf("nnListInit()\n");
        
                pList = nnListInit(dataCmpFunc, MEM_MAX_USER_TYPE + 1);

                if ((Int32T)pList <= 0)
                {
                    printf("Error nnListInit() : %s\n",
                           listErrMsg[(-(Int32T)pList)]);
                    pList = NULL;
                }

                printf("List Address : %d\n", (Int32T)pList);

                break;
            case 3:
                printf("3. nnListInit Test)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                printf("List Address : %d\n", (Int32T)pList);
                printf("nnListInit()\n");

                pList = nnListInit(dataCmpFunc, TEST_TYPE);

                if ((Int32T)pList <= 0)
                {
                    printf("Error nnListInit() : %s\n",
                           listErrMsg[(-(Int32T)pList)]);
                    pList = NULL;
                }

                printf("List Address : %d\n", (Int32T)pList);

                break;
            case 4:
                printf("4. nnListAddNode Test(List NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                                        pTestData->data = listIndex + 1;

                    errno = nnListAddNode(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNode() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                                }

                break;
            case 5:
                printf("5. nnListAddNode Test(Param Data NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                pTestData = NULL;
                errno = nnListAddNode(pList, pTestData);

                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                }

                break;
            case 6:
                printf("6. nnListAddNode Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddNode(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNode() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                listPrint(pList);
                break;
            case 7:
                printf("7. nnListAddNodeHead Test(List NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddNodeHead(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNodeHead() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                break;
            case 8:
                printf("8. nnListAddNodeHead Test(Param Data NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;

                        break;
                    }
                }

                pTestData = NULL;
                errno = nnListAddNodeHead(pList, pTestData);

                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNodeHead() : %s\n",
                           listErrMsg[(-(errno))]);
                }

                break;
            case 9:
                printf("9. nnListAddNodeHead Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;

                        break;
                    }
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddNodeHead(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNodeHead() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                listPrint(pList);
                break;
            case 10:
                printf("10. nnListAddNodePrev Test(List NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddNodePrev(pList, NULL, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNodePrev() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                break;
            case 11:
                printf("11. nnListAddNodePrev Test(Param pData NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                pTestData = NULL;
                errno = nnListAddNodePrev(pList, NULL, pTestData);

                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNodePrev() : %s\n",
                           listErrMsg[(-(errno))]);
                }

                break;
            case 12:
                printf("12. nnListAddNodePrev Test(Param pNode NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = (rand() + 1) % 10;

                errno = nnListAddNodePrev(pList, NULL,  pTestData);
                printf("Result : %s\n", listErrMsg[(-(errno))]);

                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNodePrev() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;
                }

                listPrint(pList);
                break;
            case 13:
                printf("13. nnListAddNodePrev Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                // Add Node Prev
                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestNode = nnListGetHeadNode(pList);

                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddNodePrev(pList, pTestNode, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNodePrev() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                listPrint(pList);
                break;
            case 14:
                printf("14. nnListAddNodeNext Test(List NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddNodeNext(pList, NULL, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNodeNext() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                break;
            case 15:
                printf("15. nnListAddNodeNext Test(Param pData NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                pTestData = NULL;
                errno = nnListAddNodeNext(pList, NULL, pTestData);

                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNodeNext() : %s\n",
                           listErrMsg[(-(errno))]);
                }

                break;
            case 16:
                printf("16. nnListAddNodeNext Test(Param pNode NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = (rand() + 1) % 10;

                errno = nnListAddNodeNext(pList, NULL,  pTestData);
                printf("Result : %s\n", listErrMsg[(-(errno))]);

                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNodeNext() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;
                }

                listPrint(pList);
                break;
            case 17:
                printf("17. nnListAddNodeNext Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                // Add Node Next
                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestNode = nnListGetHeadNode(pList);

                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddNodeNext(pList, pTestNode, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNodeNext() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                listPrint(pList);
                break;
            case 18:
                printf("18. nnListAddNodeDupNot Test(List NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddNodeDupNot(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNodeDupNot() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                break;
            case 19:
                printf("19. nnListAddNodeDupNot Test(No Set Comp Function)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(NULL, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddNodeDupNot(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNodeDupNot() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                break;
            case 20:
                printf("20. nnListAddNodeDupNot Test(Param pData NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                pTestData = NULL;
                errno = nnListAddNodeDupNot(pList, pTestData);

                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNodeDupNot() : %s\n",
                           listErrMsg[(-(errno))]);
                }

                break;
            case 21:
                printf("21. nnListAddNodeDupNot Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                srand(time(NULL));

                printf("Input Data : ");
                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = (rand() + 1) % 10;

                    printf("%d ", pTestData->data);

                    errno = nnListAddNodeDupNot(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNodeDupNot() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;
                    }
                                }

                listPrint(pList);
                break;
            case 22:
                printf("22. nnListCheckDupData Test(List NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListCheckDupData(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListCheckDupData() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                break;
            case 23:
                printf("23. nnListCheckDupData Test(No Set Comp Function)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(NULL, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                printf("Check Data is %d\n", CHECK_DATA);

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListCheckDupData(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListCheckDupData() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                break;
            case 24:
                printf("24. nnListCheckeDupData Test(Param pData NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                printf("Check Data is %d\n", CHECK_DATA);

                pTestData = NULL;
                errno = nnListCheckDupData(pList, pTestData);

                if (errno != SUCCESS)
                {
                    printf("Error nnListCheckDupData() : %s\n",
                           listErrMsg[(-(errno))]);
                }

                break;
            case 25:
                printf("25. nnListCheckDupData Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = CHECK_DATA;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                srand(time(NULL));

                printf("Check Data is %d\n", CHECK_DATA);
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                
                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData->data = listIndex + 1;
                    printf("\n%d ", pTestData->data);

                    errno = nnListCheckDupData(pList, pTestData);

                    if (errno != SUCCESS)
                    {
                        printf("Error nnListCheckDupData() : %s\n",
                               listErrMsg[(-(errno))]);
                    }
                }

                NNFREE(TEST_TYPE, pTestData);

                break;
            case 26:
                printf("26. nnListAddNodeSort Test(List NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                srand(time(NULL));

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = (rand() + 1) % 10;

                    errno = nnListAddNodeSort(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNodeSort() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                break;
            case 27:
                printf("27. nnListAddNodeSort Test(No Set Comp Function)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(NULL, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                srand(time(NULL));

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = (rand() + 1) % 10;

                    errno = nnListAddNodeSort(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNodeSort() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                listPrint(pList);
                break;
            case 28:
                printf("28. nnListAddNodeSort Test(Param pData NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(NULL, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                pTestData = NULL;
                errno = nnListCheckDupData(pList, pTestData);

                if (errno != SUCCESS)
                {
                    printf("Error nnListCheckDupData() : %s\n",
                           listErrMsg[(-(errno))]);
                }

                break;
            case 29:
                printf("29. nnListAddNodeSort Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                srand(time(NULL));

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = (rand() + 1) % 10;

                    errno = nnListAddNodeSort(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNodeSort() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                listPrint(pList);
                break;
            case 30:
                printf("30. nnListGetHeadNode Test(List NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                pTestNode = nnListGetHeadNode(pList);

                if ((Int32T)pTestNode == 0)
                {
                    printf("HeadNode = NULL\n");
                }
                else if ((Int32T)pTestNode < 0)
                {
                    printf("Error nnListGetHeadNode() : %s\n",
                           listErrMsg[(-(Int32T)pTestNode)]);
                }
                else
                {
                    printf("Print HeadNode's Data : %d\n",
                           ((testT *)(pTestNode->pData))->data);
                }

                break;
            case 31:
                printf("31. nnListGetHeadNode Test(No Data)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                pTestNode = nnListGetHeadNode(pList);

                if ((Int32T)pTestNode == 0)
                {
                    printf("HeadNode = NULL\n");
                }
                else if ((Int32T)pTestNode < 0)
                {
                    printf("Error nnListGetHeadNode() : %s\n",
                           listErrMsg[(-(Int32T)pTestNode)]);
                }

                else
                {
                    printf("Print HeadNode's Data : %d\n",
                           ((testT *)(pTestNode->pData))->data);
                }

                break;
            case 32:
                printf("32. nnListGetHeadNode Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddNode(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNode() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                pTestNode = nnListGetHeadNode(pList);

                if ((Int32T)pTestNode == 0)
                {
                    printf("HeadNode = NULL\n");
                }
                else if ((Int32T)pTestNode < 0)
                {
                    printf("Error nnListGetHeadNode() : %s\n",
                           listErrMsg[(-(Int32T)pTestNode)]);
                }
                else
                {
                    printf("Print HeadNode's Data : %d\n",
                           ((testT *)(pTestNode->pData))->data);
                }

                break;
            case 33:
                printf("33. nnListGetTailNode Test(List NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                pTestNode = nnListGetTailNode(pList);

                if ((Int32T)pTestNode == 0)
                {
                    printf("TailNode = NULL\n");
                }
                else if ((Int32T)pTestNode < 0)
                {
                    printf("Error nnListGetTailNode() : %s\n",
                           listErrMsg[(-(Int32T)pTestNode)]);
                }
                else
                {
                    printf("Print TailNode's Data : %d\n",
                           ((testT *)(pTestNode->pData))->data);
                }

                break;
            case 34:
                printf("34. nnListGetTailNode Test(No Data)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                pTestNode = nnListGetTailNode(pList);

                if ((Int32T)pTestNode == 0)
                {
                    printf("TailNode = NULL\n");
                }
                else if ((Int32T)pTestNode < 0)
                {
                    printf("Error nnListGetTailNode() : %s\n",
                           listErrMsg[(-(Int32T)pTestNode)]);
                }
                else
                {
                    printf("Print TailNode's Data : %d\n",
                           ((testT *)(pTestNode->pData))->data);
                }

                break;
            case 35:
                printf("35. nnListGetTailNode Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddNode(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNode() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                pTestNode = nnListGetTailNode(pList);

                if ((Int32T)pTestNode == 0)
                {
                    printf("TailNode = NULL\n");
                }
                else if ((Int32T)pTestNode < 0)
                {
                    printf("Error nnListGetTailNode() : %s\n",
                           listErrMsg[(-(Int32T)pTestNode)]);
                }
                else
                {
                    printf("Print TailNode's Data : %d\n",
                           ((testT *)(pTestNode->pData))->data);
                }

                break;
            case 36:
                printf("36. nnListCount Test(List NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                errno = nnListCount(pList);

                if (errno < 0)
                {
                    printf("Error nnListCount() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }
                else
                {
                    printf("ListCount : %d\n", errno);
                }

                break;
            case 37:
                printf("37. nnListCount Test(No Data)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                errno = nnListCount(pList);

                if (errno < 0)
                {
                    printf("Error nnListCount() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }
                else
                {
                    printf("ListCount : %d\n", errno);
                }

                break;
            case 38:
                printf("38. nnListCount Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = (rand() + 1) % 10;

                    errno = nnListAddNode(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNode() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                errno = nnListCount(pList);

                if (errno < 0)
                {
                    printf("Error nnListCount() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }
                else
                {
                    printf("ListCount : %d\n", errno);
                }

                break;
            case 39:
                printf("39. nnListSearchNode Test(List NULL) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                pTestData = NULL;
                pTestNode = nnListSearchNode(pList, pTestData);

                if ((Int32T)pTestNode == 0)
                {
                    printf("SearchNode = NULL\n");
                }
                else if ((Int32T)pTestNode < 0)
                {
                    printf("Error nnListSearchNode() : %s\n",
                           listErrMsg[(-((Int32T)pTestNode))]);
                }
                else
                {
                    printf("Print SearchNode's Data : %d\n",
                           ((testT *)(pTestNode->pData))->data);
                }

                break;
            case 40:
                printf(
                "40. nnListSearchNode Test(No Set Comp Function NULL) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(NULL, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = pTempTest->m3 + 1;

                pTestNode = nnListSearchNode(pList, pTestData);

                NNFREE(TEST_TYPE, pTestData);

                if ((Int32T)pTestNode == 0)
                {
                    printf("SearchNode = NULL\n");
                }
                else if ((Int32T)pTestNode < 0)
                {
                    printf("Error nnListSearchNode() : %s\n",
                           listErrMsg[(-((Int32T)pTestNode))]);
                }
                else
                {
                    printf("Print SearchNode's Data : %d\n",
                           ((testT *)(pTestNode->pData))->data);
                }

                break;
            case 41:
                printf("41. nnListSearchNode Test(Param Data NULL) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                pTestData = NULL;
                pTestNode = nnListSearchNode(pList, pTestData);

                if ((Int32T)pTestNode == 0)
                {
                    printf("SearchNode = NULL\n");
                }
                else if ((Int32T)pTestNode < 0)
                {
                    printf("Error nnListSearchNode() : %s\n",
                           listErrMsg[(-((Int32T)pTestNode))]);
                }
                else
                {
                    printf("Print SearchNode's Data : %d\n",
                           ((testT *)(pTestNode->pData))->data);
                }

                break;
            case 42:
                printf("42. nnListSearchNode Test(No Data) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = pTempTest->m3 + 1;

                pTestNode = nnListSearchNode(pList, pTestData);

                NNFREE(TEST_TYPE, pTestData);

                if ((Int32T)pTestNode == 0)
                {
                    printf("SearchNode = NULL\n");
                }
                else if ((Int32T)pTestNode < 0)
                {
                    printf("Error nnListSearchNode() : %s\n",
                           listErrMsg[(-((Int32T)pTestNode))]);
                }
                else
                {
                    printf("Print SearchNode's Data : %d\n",
                           ((testT *)(pTestNode->pData))->data);
                }

                break;
            case 43:
                printf("43. nnListSearchNode Test(Not Exist Data) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex;

                    errno = nnListAddNode(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNode() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = pTempTest->m3 + 1;

                printf("Search Data is %d\n", pTestData->data);

                pTestNode = nnListSearchNode(pList, pTestData);

                NNFREE(TEST_TYPE, pTestData);

                if ((Int32T)pTestNode == 0)
                {
                    printf("SearchNode = NULL\n");
                }
                else if ((Int32T)pTestNode < 0)
                {
                    printf("Error nnListSearchNode() : %s\n",
                           listErrMsg[(-((Int32T)pTestNode))]);
                }
                else
                {
                    printf("Print SearchNode's Data : %d\n",
                           ((testT *)(pTestNode->pData))->data);
                }

                break;
            case 44:
                printf("44. nnListSearchNode Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddNode(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNode() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = pTempTest->m3;

                printf("Search Data is %d\n", pTestData->data);

                pTestNode = nnListSearchNode(pList, pTestData);

                NNFREE(TEST_TYPE, pTestData);

                if ((Int32T)pTestNode == 0)
                {
                    printf("SearchNode = NULL\n");
                }
                else if ((Int32T)pTestNode < 0)
                {
                    printf("Error nnListSearchNode() : %s\n",
                           listErrMsg[(-((Int32T)pTestNode))]);
                }
                else
                {
                    printf("Print SearchNode's Data : %d\n",
                           ((testT *)(pTestNode->pData))->data);
                }

                break;
            case 45:
                printf("45. nnListFree Test(List NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                nnListFree(pList);
                pList = NULL;

                break;
            case 46:
                printf("46. nnListFree Test(No Data)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                nnListFree(pList);
                pList = NULL;

                break;
            case 47:
                printf("47. nnListFree Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddNode(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNode() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                nnListFree(pList);
                pList = NULL;

                break;
            case 48:
                printf("48. nnListDeleteNode(List NULL) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                pTestData = NULL;
                nnListDeleteNode(pList, pTestData);

                break;
            case 49:
                printf("49. nnListDeleteNode(Param Data NULL) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                pTestData = NULL;
                nnListDeleteNode(pList, pTestData);

                break;
            case 50:
                printf("50. nnListDeleteNode(No Data) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = pTempTest->m3;

                nnListDeleteNode(pList, pTestData);

                NNFREE(TEST_TYPE, pTestData);

                break;
            case 51:
                printf("51. nnListDeleteNode(Not Exist Data) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddNode(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNode() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = pTempTest->m3 + 1;

                printf("Delete Data is %d\n", pTestData->data);
                nnListDeleteNode(pList, pTestData);

                NNFREE(TEST_TYPE, pTestData);

                break;
            case 52:
                printf("52. nnListDeleteNode Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddNode(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNode() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                printf("Delete Data is %d\n", pTestData->data);
                nnListDeleteNode(pList, pTestData);

                listPrint(pList);
                break;
            case 53:
                printf("53. nnListDeleteAllNode Test(List NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                nnListDeleteAllNode(pList);

                break;
            case 54:
                printf("54. nnListDeleteAllNode Test(No Data)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                nnListDeleteAllNode(pList);

                break;
            case 55:
                printf("55. nnListDeleteAllNode Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddNode(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNode() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                nnListDeleteAllNode(pList);

                listPrint(pList);

                break;
            case 56:
                printf("56. nnListDeleteAllNode Test(List NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                errno = nnListSetNodeCompFunc(pList, dataCmpFunc);

                if (errno != SUCCESS)
                {
                    printf("Error nnListSetNodeCompFunc() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                break;
            case 57:
                printf("57. nnListDeleteAllNode Test(Param Comp NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                errno = nnListSetNodeCompFunc(pList, NULL);
                printf("Result : %s\n", listErrMsg[(-(errno))]);

                if (errno != SUCCESS)
                {
                    printf("Error nnListSetNodeCompFunc() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                break;
            case 58:
                printf("58. nnListDeleteAllNode Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                errno = nnListSetNodeCompFunc(pList, dataCmpFunc);
                printf("Result : %s\n", listErrMsg[(-(errno))]);

                if (errno != SUCCESS)
                {
                    printf("Error nnListSetNodeCompFunc() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                break;
            case 59:
                printf("59. SubList - nnListSubInit Test(ListNode NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                errno = nnListSubInit(NULL, NULL, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                break;
            case 60:
                printf(
                    "60. SubList - nnListSubInit Test(Param DataCmp NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, NULL, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                break;
            case 61:
                printf(
                    "61. SubList - nnListSubInit Test(Param Wrong dataType)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(
                            pTestNode, dataCmpFunc, MEM_MAX_USER_TYPE + 1);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                break;
            case 62:
                printf("62. SubList - nnListSubInit Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, dataCmpFunc, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                break;
            case 63:
                printf("63. SubList - nnListAddSubNode Test(ListNode NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, dataCmpFunc, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                errno = nnListAddSubNode(NULL, NULL);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddSubNode() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                break;
            case 64:
                printf(
                    "64. SubList - nnListAddSubNode Test(Param pData NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, dataCmpFunc, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                errno = nnListAddSubNode(pTestNode, NULL);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddSubNode() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                break;
            case 65:
                printf("65. SubList - nnListAddSubNode Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, dataCmpFunc, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddSubNode(pTestNode, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddSubNode() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                listSubPrint(pTestNode);

                break;
            case 66:
                printf(
                    "66. SubList - nnListAddSubNodeHead Test(ListNode NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, dataCmpFunc, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                errno = nnListAddSubNodeHead(NULL, NULL);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddSubNodeHead() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                break;
            case 67:
                printf(
                "67. SubList - nnListAddSubNodeHead Test(Param pData NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, dataCmpFunc, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                errno = nnListAddSubNodeHead(pTestNode, NULL);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddSubNodeHead() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                break;
            case 68:
                printf("68. SubList - nnListAddSubNodeHead Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, dataCmpFunc, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }


                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddSubNodeHead(pTestNode, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddSubNodeHead() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                listSubPrint(pTestNode);

                break;
            case 69:
                printf(
                "69. SubList - nnListGetHeadSubNode Test(ListNode NULL)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, dataCmpFunc, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                pTestSubNode = nnListGetHeadSubNode(NULL);

                if ((Int32T)pTestSubNode == 0)
                {
                    printf("HeadSubNode = NULL\n");
                }
                else if ((Int32T)pTestSubNode < 0)
                {
                    printf("Error nnListGetHeadSubNode() : %s\n",
                           listErrMsg[(-(Int32T)pTestSubNode)]);
                }
                else
                {
                    printf("Print HeadSubNode's Data : %d\n",
                           ((testT *)(pTestSubNode->pData))->data);
                }

                break;
            case 70:
                printf("70. SubList - nnListGetHeadSubNode Test(No Data)\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, dataCmpFunc, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                pTestSubNode = nnListGetHeadSubNode(pTestNode);

                if ((Int32T)pTestSubNode == 0)
                {
                    printf("HeadSubNode = NULL\n");
                }
                else if ((Int32T)pTestSubNode < 0)
                {
                    printf("Error nnListGetHeadSubNode() : %s\n",
                           listErrMsg[(-(Int32T)pTestSubNode)]);
                }
                else
                {
                    printf("Print HeadSubNode's Data : %d\n",
                           ((testT *)(pTestSubNode->pData))->data);
                }

                break;
            case 71:
                printf("71. SubList - nnListGetHeadSubNode Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, dataCmpFunc, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddSubNode(pTestNode, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddSubNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                pTestSubNode = nnListGetHeadSubNode(pTestNode);

                if ((Int32T)pTestSubNode == 0)
                {
                    printf("HeadSubNode = NULL\n");
                }
                else if ((Int32T)pTestSubNode < 0)
                {
                    printf("Error nnListGetHeadSubNode() : %s\n",
                           listErrMsg[(-(Int32T)pTestSubNode)]);
                }
                else
                {
                    printf("Print HeadSubNode's Data : %d\n",
                           ((testT *)(pTestSubNode->pData))->data);
                }

                break;
            case 72:
                printf("72. SubList - nnListSubCount(ListNode NULL) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, dataCmpFunc, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                errno = nnListSubCount(NULL);

                if (errno < 0)
                {
                    printf("Error nnListSubCount() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }
                else
                {
                    printf("ListSub Count : %d\n", errno);
                }


                break;
            case 73:
                printf("73. SubList - nnListSubCount(No Data) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, dataCmpFunc, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                errno = nnListSubCount(pTestNode);

                if (errno < 0)
                {
                    printf("Error nnListSubCount() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }
                else
                {
                    printf("ListSub Count : %d\n", errno);
                }

                break;
            case 74:
                printf("74. SubList - nnListSubCount Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, dataCmpFunc, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                                        pTestData->data = listIndex + 1;

                    errno = nnListAddSubNode(pTestNode, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddSubNode() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                                }

                errno = nnListSubCount(pTestNode);

                if (errno < 0)
                {
                    printf("Error nnListSubCount() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }
                else
                {
                    printf("ListSub Count : %d\n", errno);
                }

                break;
            case 75:
                printf(
                    "75. SubList - nnListSearchSubNode(ListNode NULL) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, NULL, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                pTestSubNode = nnListSearchSubNode(NULL, NULL);

                if ((Int32T)pTestSubNode == 0)
                {
                    printf("Search SubNode = NULL\n");
                }
                else if ((Int32T)pTestSubNode < 0)
                {
                    printf("Error nnListSearchSubNode() : %s\n",
                           listErrMsg[(-(Int32T)pTestSubNode)]);
                }
                else
                {
                    printf("Print Search SubNode's Data : %d\n",
                           ((testT *)(pTestSubNode->pData))->data);
                }

                break;
            case 76:
                printf(
                    "76. SubList - nnListSearchSubNode\
                    (No Set Comp Function NULL) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, NULL, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = listIndex + 1;

                pTestSubNode = nnListSearchSubNode(pTestNode, pTestData);

                NNFREE(TEST_TYPE, pTestData);

                if ((Int32T)pTestSubNode == 0)
                {
                    printf("Search SubNode = NULL\n");
                }
                else if ((Int32T)pTestSubNode < 0)
                {
                    printf("Error nnListSearchSubNode() : %s\n",
                           listErrMsg[(-(Int32T)pTestSubNode)]);
                }
                else
                {
                    printf("Print Search SubNode's Data : %d\n",
                           ((testT *)(pTestSubNode->pData))->data);
                }

                break;
            case 77:
                printf(
                "77. SubList - nnListSearchSubNode(Param Data NULL) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, dataCmpFunc, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                pTestSubNode = nnListSearchSubNode(pTestNode, NULL);

                if ((Int32T)pTestSubNode == 0)
                {
                    printf("Search SubNode = NULL\n");
                }
                else if ((Int32T)pTestSubNode < 0)
                {
                    printf("Error nnListSearchSubNode() : %s\n",
                           listErrMsg[(-(Int32T)pTestSubNode)]);
                }
                else
                {
                    printf("Print Search SubNode's Data : %d\n",
                           ((testT *)(pTestSubNode->pData))->data);
                }

                break;
            case 78:
                printf("78. SubList - nnListSearchSubNode(No Data) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, dataCmpFunc, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 1;

                pTestSubNode = nnListSearchSubNode(pTestNode, pTestData);

                NNFREE(TEST_TYPE, pTestData);

                if ((Int32T)pTestSubNode == 0)
                {
                    printf("Search SubNode = NULL\n");
                }
                else if ((Int32T)pTestSubNode < 0)
                {
                    printf("Error nnListSearchSubNode() : %s\n",
                           listErrMsg[(-(Int32T)pTestSubNode)]);
                }
                else
                {
                    printf("Print Search SubNode's Data : %d\n",
                           ((testT *)(pTestSubNode->pData))->data);
                }

                break;
            case 79:
                printf(
                    "79. SubList - nnListSearchSubNode(Not Exist Data) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, dataCmpFunc, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = CHECK_DATA;

                errno = nnListAddSubNode(pTestNode, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddSubNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                pTestSubNode = nnListSearchSubNode(pTestNode, pTestData);

                NNFREE(TEST_TYPE, pTestData);

                if ((Int32T)pTestSubNode == 0)
                {
                    printf("Search SubNode = NULL\n");
                }
                else if ((Int32T)pTestSubNode < 0)
                {
                    printf("Error nnListSearchSubNode() : %s\n",
                           listErrMsg[(-(Int32T)pTestSubNode)]);
                }
                else
                {
                    printf("Print Search SubNode's Data : %d\n",
                           ((testT *)(pTestSubNode->pData))->data);
                }

                break;
            case 80:
                printf("80. SubList - nnListSearchSubNode Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, dataCmpFunc, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddSubNode(pTestNode, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddSubNode() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = CHECK_DATA;

                pTestSubNode = nnListSearchSubNode(pTestNode, pTestData);

                NNFREE(TEST_TYPE, pTestData);

                if ((Int32T)pTestSubNode == 0)
                {
                    printf("Search SubNode = NULL\n");
                }
                else if ((Int32T)pTestSubNode < 0)
                {
                    printf("Error nnListSearchSubNode() : %s\n",
                           listErrMsg[(-(Int32T)pTestSubNode)]);
                }
                else
                {
                    printf("Print Search SubNode's Data : %d\n",
                           ((testT *)(pTestSubNode->pData))->data);
                }

                break;
            case 81:
                printf(
                    "81. SubList - nnListDeleteSubNode(ListNode NULL) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, NULL, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                nnListDeleteSubNode(NULL, NULL);

                break;
            case 82:
                printf(
                "82. SubList - nnListDeleteSubNode(Param Data NULL) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, NULL, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                nnListDeleteSubNode(pTestNode, NULL);

                break;
            case 83:
                printf("83. SubList - nnListDeleteSubNode(No Data) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, NULL, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = CHECK_DATA;

                nnListDeleteSubNode(pTestNode, pTestData);

                NNFREE(TEST_TYPE, pTestData);

                break;
            case 84:
                printf(
                    "84. SubList - nnListDeleteSubNode(Not Exist Data) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, NULL, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = CHECK_DATA;

                errno = nnListAddSubNode(pTestNode, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddSubNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = CHECK_DATA;

                nnListDeleteSubNode(pTestNode, pTestData);

                NNFREE(TEST_TYPE, pTestData);

                break;
            case 85:
                printf("85. SubList - nnListDeleteSubNode Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, NULL, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddSubNode(pTestNode, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddSubNode() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                printf("Delete Sub Data is %d\n", pTestData->data);
                nnListDeleteSubNode(pTestNode, pTestData);

                listSubPrint(pTestNode);

                break;
            case 86:
                printf(
                "86. SubList - nnListDeleteAllSubNode(ListNode NULL) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, NULL, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                nnListDeleteAllSubNode(NULL);

                break;
            case 87:
                printf("87. SubList - nnListDeleteAllSubNode(No Data) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, NULL, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                nnListDeleteAllSubNode(pTestNode);

                break;
            case 88:
                printf("88. SubList - nnListDeleteAllSubNode Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, NULL, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddSubNode(pTestNode, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddSubNode() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                }

                nnListDeleteAllSubNode(pTestNode);

                break;
            case 89:
                printf(
                "89. SubList - nnListSetSubNodeCompFunc(ListNode NULL) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, NULL, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                nnListSetSubNodeCompFunc(NULL, NULL);

                break;
            case 90:
                printf(
                "90. SubList - nnListSetSubNodeCompFunc\
                (Param Comp NULL) Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, NULL, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                nnListSetSubNodeCompFunc(pTestNode, NULL);

                break;
            case 91:
                printf("91. SubList - nnListSetSubNodeCompFunc Test\n");

                if (pList != NULL)
                {
                    nnListFree(pList);
                    pList = NULL;
                }

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                // Add Temp Data
                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                pTestData->data = 0;

                errno = nnListAddNode(pList, pTestData);
                if (errno != SUCCESS)
                {
                    printf("Error nnListAddNode() : %s\n",
                           listErrMsg[(-(errno))]);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;

                    break;
                }

                // Get Head
                pTestNode = nnListGetHeadNode(pList);

                errno = nnListSubInit(pTestNode, NULL, TEST_TYPE);
                if (errno != SUCCESS)
                {
                    printf("Error nnListSubInit() : %s\n",
                           listErrMsg[(-(errno))]);

                    break;
                }

                nnListSetSubNodeCompFunc(pTestNode, dataCmpFunc);

                break;
            case 92:
                printf("92. nnListAddNode Test\n");

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                for (listIndex = 0; listIndex < pTempTest->m3; ++listIndex)
                                {
                    pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                    pTestData->data = listIndex + 1;

                    errno = nnListAddNode(pList, pTestData);
                    if (errno != SUCCESS)
                    {
                        printf("Error nnListAddNode() : %s\n",
                               listErrMsg[(-(errno))]);
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;

                        break;
                    }
                                }

                break;
            case 93:
                printf("93. nnListDeleteNode Test\n");

                if (pList == NULL)
                {
                    pList = nnListInit(dataCmpFunc, TEST_TYPE);

                    if ((Int32T)pList <= 0)
                    {
                        printf("Error nnListInit() : %s\n",
                               listErrMsg[(-(Int32T)pList)]);
                        pList = NULL;
                        break;
                    }
                }

                listIndex = 0;

                if (pList->pHead != NULL)
                {
                    for (pTestNode = pList->pHead;
                        listIndex < pTempTest->m3 && pTestNode != NULL;
                        pTestNode = pList->pHead)
                    {
                        pList->pHead = pTestNode->pNext;

                        if (pTestNode->pSubHead != NULL)
                        {
                            nnListDeleteAllSubNode(pTestNode);

                            NNFREE(LIST_SUB_HEAD, pTestNode->pSubHead);

                            pTestNode->pSubHead = NULL;
                        }

                        NNFREE(pList->dataType, pTestNode->pData);
                        pTestNode->pData = NULL;

                        NNFREE(LIST_NODE, pTestNode);
                        pTestNode = NULL;

                        --pList->count;
                        ++listIndex;
                    }
                }

                break;
            case 94:
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

    if (pList)
    {
        nnListFree(pList);
        pList = NULL;
    }

    NNLOG(LOG_DEBUG, "List Test End\n");

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

Int32T listPrint(ListT * pList)
{
    ListNodeT * tempNode = NULL;

    if (pList == NULL)
    {
        printf("listPrint : List is NULL\n");
        return FAILURE;
    }

    tempNode = pList->pHead;

    if(tempNode == NULL)
    {
        printf("listPrint : no data\n");
        return 0;
    }

    printf("Datas : ");
    for (; tempNode != pList->pTail; tempNode = tempNode->pNext)
    {
        printf("%d ", ((testT *)tempNode->pData)->data);
    }
    printf("%d\n", ((testT *)tempNode->pData)->data);

    printf("pListSize : %llu\n\n", nnListCount(pList));

    return 0;
}

Int32T listSubPrint(ListNodeT * pListNode)
{
    ListSubT * tempSub = NULL;
    ListSubNodeT *tempSubNode = NULL;

    if (pListNode == NULL)
    {
        printf("listSubPrint : ListNode is NULL\n");
        return FAILURE;
    }

    tempSub = pListNode->pSubHead;

    if (tempSub == NULL)
    {
        printf("listSubPrint : ListSub is NULL\n");
    }

    tempSubNode = tempSub->pHead;

    if(tempSubNode == NULL)
    {
        printf("listSubPrint : no data\n");
        return 0;
    }

    printf("Datas : ");
    for (; tempSubNode != tempSub->pTail; tempSubNode = tempSubNode->pNext)
    {
        printf("%d ", ((testT *)tempSubNode->pData)->data);
    }
    printf("%d\n", ((testT *)tempSubNode->pData)->data);

    printf("pListSubSize : %llu\n\n", nnListSubCount(pListNode));

    return 0;
}
