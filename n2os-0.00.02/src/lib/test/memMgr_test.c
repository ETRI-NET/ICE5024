#include <time.h>
#include "../nnLog.h"
#include "../nnMemmgr.h"
#include "test.h"

#define TEST_PROGRAM 0
#define TEST_TYPE 0
#define ADD_COUNT 10
#define CHECK_DATA 5

#define WRONG_DATA (MEM_MAX_USER_TYPE + 1)

StringT memErrMsg[] = {"SUCCESS", "MEMMGR_ERR_ALLOC_REALLOC",
                        "MEMMGR_ERR_DEBUG_ALLOC", "MEMMGR_ERR_DEBUG_FLAG",
                        "MEMMGR_ERR_DEBUG_ENQUEUE", "MEMMGR_ERR_ARGUMENT"};

/* Test Structure */
typedef struct testT {
        Int32T data;
} testT;

Int32T menu = 0;
BoolT run = TRUE;
testT *pTestData = NULL;
testT *pTestData2 = NULL;

void printMenu()
{
    printf("==========================================\n");
    printf("           Memmgr Test Menu\n");
    printf("1. memInit Test(Param Wrong process\n");
    printf("2. memInit Test\n");

    printf("3. memSetDebug Test(Param Wrong dataType)\n");
    printf("4. memSetDebug Test\n");

    printf("5. memUnsetDebug Test(Param Wrong dataType)\n");
    printf("6. memUnsetDebug Test\n");

    printf("7. memSetFile(Param FileName NULL)\n");
    printf("8. memSetFile(Param FileSize 0)\n");
    printf("9. memSetFile\n");

    printf("10. memUnsetFile\n");

    printf("11. NNMALLOC Test(Param Wrong dataType)\n");
    printf("12. NNMALLOC Test(Param Wrong dataSize)\n");
    printf("13. NNMALLOC Test\n");

    printf("14. NNREMALLOC Test(Param Wrong dataType)\n");
    printf("15. NNREMALLOC Test(Param reMem NULL)\n");
    printf("16. NNREMALLOC Test(Param Wrong dataSize)\n");
    printf("17. NNREMALLOC Test\n");

    printf("18. NNFREE Test(Param Wrong dataType)\n");
    printf("19. NNFREE Test(Param freeMem NULL)\n");
    printf("20. NNFREE Test\n");

    printf("21. memShowAllUser Test(Param Wrong dataType)\n");
    printf("22. memShowAllUser Test\n");
//////////////// 
    printf("23. Exit\n");
    printf("==========================================\n");
}

Int32T main() 
{
    Int32T errno = 0;

    double start = 0;

    TestHead *pTestHead = (TestHead *)malloc(sizeof(TestHead));
    Test *pTempTest = NULL;

    pTestHead->pHead = NULL;
    pTestHead->pTail = NULL;
    pTestHead->count = 0;

    errno = getFileData(pTestHead, "memConf");
    if (errno == FAILURE)
    {
        printf("FAILURE\n");
        return -1;
    }

    memInit(TEST_PROGRAM);

    nnLogInit(TEST_PROGRAM);

    printMenu();
    NNLOG(LOG_DEBUG, "Memory Manager Test Start\n");

    start = clock();

    pTempTest = pTestHead->pHead;

    memClose();

    while(pTestHead->count > 0)
    {
        menu = pTempTest->m1;

        switch(menu)
        {
            case 1:
                printf("1. memInit Test(Param Wrong process\n");

                errno = memInit(WRONG_DATA);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                memClose();

                break;
            case 2:
                printf("2. memInit Test\n");

                errno = memInit(TEST_PROGRAM);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                memClose();

                break;
            case 3:
                printf("3. memSetDebug Test(Param Wrong dataType)\n");

                errno = memInit(TEST_PROGRAM);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                errno = memSetDebug(WRONG_DATA);

                if (errno != SUCCESS)
                {
                    printf("Error memSetDebug() : %s\n", memErrMsg[(-errno)]);
                }

                memClose();

                break;
            case 4:
                printf("4. memSetDebug Test\n");

                errno = memInit(TEST_PROGRAM);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                errno = memSetDebug(TEST_TYPE);

                if (errno != SUCCESS)
                {
                    printf("Error memSetDebug() : %s\n", memErrMsg[(-errno)]);
                }

                memClose();

                break;
            case 5:
                printf("5. memUnsetDebug Test(Param Wrong dataType)\n");

                errno = memInit(TEST_PROGRAM);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                errno = memSetDebug(TEST_TYPE);

                if (errno != SUCCESS)
                {
                    printf("Error memSetDebug() : %s\n", memErrMsg[(-errno)]);
                }

                errno = memUnsetDebug(WRONG_DATA);

                if (errno != SUCCESS)
                {
                    printf("Error memUnsetDebug() : %s\n", memErrMsg[(-errno)]);
                }
                else
                {
                    printf("memUnsetDebug Result : %s\n", memErrMsg[(-errno)]);
                }

                memClose();

                break;
            case 6:
                printf("6. memUnsetDebug Test\n");

                errno = memInit(TEST_PROGRAM);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                errno = memSetDebug(TEST_TYPE);

                if (errno != SUCCESS)
                {
                    printf("Error memSetDebug() : %s\n", memErrMsg[(-errno)]);
                }

                errno = memUnsetDebug(TEST_TYPE);

                if (errno != SUCCESS)
                {
                    printf("Error memUnsetDebug() : %s\n", memErrMsg[(-errno)]);
                }
                else
                {
                    printf("memUnsetDebug Result : %s\n", memErrMsg[(-errno)]);
                }

                memClose();

                break;
            case 7:
                printf("7. memSetFile(Param FileName NULL)\n");

                errno = memInit(TEST_PROGRAM);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                memSetFile(NULL, 0);

                memClose();

                break;
            case 8:
                printf("8. memSetFile(Param FileSize 0)\n");

                errno = memInit(TEST_PROGRAM);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                memSetFile("memTestLog", 0);

                memClose();

                break;
            case 9:
                printf("9. memSetFile\n");

                errno = memInit(TEST_PROGRAM);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                memSetFile("memTestLog", 1);

                memClose();

                break;
            case 10:
                printf("10. memUnsetFile\n");

                errno = memInit(TEST_PROGRAM);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                memUnsetFile();

                memClose();

                break;
            case 11:
                printf("11. NNMALLOC Test(Param Wrong dataType)\n");

                errno = memInit(TEST_PROGRAM);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                pTestData = NNMALLOC(WRONG_DATA, sizeof(testT));

                if ((Int32T)pTestData <= 0)
                {
                    printf("Error NNMALLOC() : %s\n",
                           memErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    printf("NNMALLOC Result : SUCCESS\n");
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;
                }

                memClose();
printf("WRONG RESULT\n");
                break;
            case 12:
                printf("12. NNMALLOC Test(Param Wrong dataSize)\n");

                errno = memInit(TEST_PROGRAM);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                pTestData = NNMALLOC(TEST_TYPE, 0);

                if ((Int32T)pTestData <= 0)
                {
                    printf("Error NNMALLOC() : %s\n",
                           memErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    printf("NNMALLOC Result : SUCCESS\n");
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;
                }

                memClose();

                break;
            case 13:
                printf("13. NNMALLOC Test\n");

                errno = memInit(TEST_PROGRAM);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));

                if ((Int32T)pTestData <= 0)
                {
                    printf("Error NNMALLOC() : %s\n",
                           memErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    printf("NNMALLOC Result : SUCCESS\n");
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;
                }

                memClose();

                break;
            case 14:
                printf("14. NNREMALLOC Test(Param Wrong dataType)\n");

                errno = memInit(TEST_PROGRAM);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));

                if ((Int32T)pTestData <= 0)
                {
                    printf("Error NNMALLOC() : %s\n",
                           memErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    pTestData = NNREALLOC(WRONG_DATA, pTestData, 8);

                    if ((Int32T)pTestData <= 0)
                    {
                        printf("Error NNREALLOC() : %s\n",
                               memErrMsg[(-(Int32T)pTestData)]);
                    }
                    else
                    {
                        printf("NNREALLOC Result : SUCCESS\n");
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;
                    }
                }

                memClose();
printf("WRONG RESULT\n");
                break;
            case 15:
                printf("15. NNREMALLOC Test(Param reMem NULL)\n");

                errno = memInit(TEST_PROGRAM);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));

                if ((Int32T)pTestData <= 0)
                {
                    printf("Error NNMALLOC() : %s\n",
                           memErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    pTestData2 = NNREALLOC(TEST_TYPE, NULL, 8);

                    if ((Int32T)pTestData2 <= 0)
                    {
                        printf("Error NNREALLOC() : %s\n",
                               memErrMsg[(-(Int32T)pTestData2)]);
                    }
                    else
                    {
                        printf("NNREALLOC Result : SUCCESS\n");
                        NNFREE(TEST_TYPE, pTestData2);
                        pTestData2 = NULL;
                    }

                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;
                }

                memClose();
printf("WRONG RESULT\n");
                break;
            case 16:
                printf("16. NNREMALLOC Test(Param Wrong dataSize)\n");

                errno = memInit(TEST_PROGRAM);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));

                if ((Int32T)pTestData <= 0)
                {
                    printf("Error NNMALLOC() : %s\n",
                           memErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    pTestData2 = NNREALLOC(TEST_TYPE, pTestData, 0);

                    if ((Int32T)pTestData2 <= 0)
                    {
                        printf("Error NNREALLOC() : %s\n",
                               memErrMsg[(-(Int32T)pTestData2)]);
                    }
                    else
                    {
                        printf("NNREALLOC Result : SUCCESS\n");
                        NNFREE(TEST_TYPE, pTestData2);
                        pTestData2 = NULL;
                    }

                    pTestData = NULL;
                }

                memClose();

                break;
            case 17:
                printf("17. NNREMALLOC Test\n");

                errno = memInit(TEST_PROGRAM);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));

                if ((Int32T)pTestData <= 0)
                {
                    printf("Error NNMALLOC() : %s\n",
                           memErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    pTestData2 = NNREALLOC(TEST_TYPE, pTestData, 32);

                    if ((Int32T)pTestData2 <= 0)
                    {
                        printf("Error NNREALLOC() : %s\n",
                               memErrMsg[(-(Int32T)pTestData2)]);
                    }
                    else
                    {
                        printf("NNREALLOC Result : SUCCESS\n");
                        NNFREE(TEST_TYPE, pTestData2);
                        pTestData2 = NULL;
                    }

                    if (pTestData2 != NULL)
                    {
                        NNFREE(TEST_TYPE, pTestData);
                        pTestData = NULL;
                    }
                }

                memClose();

                break;
            case 18:
                printf("18. NNFREE Test(Param Wrong dataType)\n");

                errno = memInit(TEST_PROGRAM);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));

                if ((Int32T)pTestData <= 0)
                {
                    printf("Error NNMALLOC() : %s\n",
                           memErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    printf("NNMALLOC Result : SUCCESS\n");
                    NNFREE(WRONG_DATA, pTestData);
                    pTestData = NULL;
                }

                memClose();
printf("WRONG RESULT\n");
                break;
            case 19:
                printf("19. NNFREE Test(Param freeMem NULL)\n");

                errno = memInit(TEST_PROGRAM);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));

                if ((Int32T)pTestData <= 0)
                {
                    printf("Error NNMALLOC() : %s\n",
                           memErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    printf("NNMALLOC Result : SUCCESS\n");
                    NNFREE(TEST_TYPE, NULL);
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;
                }

                memClose();

                break;
            case 20:
                printf("20. NNFREE Test\n");

                errno = memInit(TEST_PROGRAM);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));

                if ((Int32T)pTestData <= 0)
                {
                    printf("Error NNMALLOC() : %s\n",
                           memErrMsg[(-(Int32T)pTestData)]);
                }
                else
                {
                    printf("NNMALLOC Result : SUCCESS\n");
                    NNFREE(TEST_TYPE, pTestData);
                    pTestData = NULL;
                }

                memClose();

                break;
            case 21:
                printf("21. memShowAllUser Test(Param Wrong dataType)\n");

                errno = memInit(TEST_PROGRAM);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                memShowAllUser(WRONG_DATA);

                memClose();

                break;
            case 22:
                printf("22. memShowAllUser Test\n");

                errno = memInit(TEST_PROGRAM);

                printf("memInit Result : %s\n", memErrMsg[(-errno)]);

                if (errno != SUCCESS)
                {
                    printf("Error memInit() : %s\n", memErrMsg[(-errno)]);
                    break;
                }

                pTestData = NNMALLOC(TEST_TYPE, sizeof(testT));
                NNFREE(TEST_TYPE, pTestData);

                memShowAllUser(TEST_TYPE);

                memClose();

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
            printf("\n\n");
            NNLOG(LOG_DEBUG, "Menu %d is %f Seconds.\n",
                  pTempTest->m1, (double)(clock() - start) / CLOCKS_PER_SEC);

            --pTestHead->count;
            pTempTest = pTempTest->pNext;

            start = clock();
        }
    }

    memInit(TEST_PROGRAM);

    NNLOG(LOG_DEBUG, "Memory Manager Test End\n");

    nnLogClose();
    memClose();

    freeTestHead(pTestHead);

    return 0;
}
