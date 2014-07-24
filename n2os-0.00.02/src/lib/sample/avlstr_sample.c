#include <stdio.h>
#include <stdlib.h>
#include "../nnDefines.h"
#include "../nnAvlStr.h"
#include "../nnStr.h"
#include "../nnMemmgr.h"

#define TEST_TYPE 0

void avlPrint (AvlNodeStrT * pAvlNodeStr);
void avlInsertTest1 (AvlTreeStrT * pAvlTreeStr);
void avlInsertTest2 (AvlTreeStrT * pAvlTreeStr);
void avlInsertTest3 (AvlTreeStrT * pAvlTreeStr);
void avlInsertTest4 (AvlTreeStrT * pAvlTreeStr);
void avlInsertTest5 (AvlTreeStrT * pAvlTreeStr);
void avlInsertTest6 (AvlTreeStrT * pAvlTreeStr);
void avlDelTest1 (AvlTreeStrT * pAvlTreeStr);
void avlDelTest2 (AvlTreeStrT * pAvlTreeStr);
void nnAvlLookupDataTest(AvlTreeStrT *pAvlTreeStr);

/* Test Structure */
typedef struct testT {
        int data;
} testT;

void avlPrint (AvlNodeStrT * pAvlNodeStr)
{
    if (pAvlNodeStr == NULL)
    {
        LOG("No Data");
    }
    else
    {
        if (pAvlNodeStr->pParent != NULL)
        {
            printf("Parent = %d ", \
                   ((testT *)pAvlNodeStr->pParent->pData)->data);
        }
        else
        {
            printf("Parent = NULL ");
        }

        printf("Key = %s, Data = %d, Height = %d, Balance = %d\n", \
                pAvlNodeStr->key, ((testT *)pAvlNodeStr->pData)->data, \
                (pAvlNodeStr->height), (pAvlNodeStr->balance));

        printf("Left\n");
        avlPrint(pAvlNodeStr->pLeft);

        printf("Right\n");
        avlPrint(pAvlNodeStr->pRight);
    }
}


/* Insert Left Side 80 ~ 10 */
void avlInsertTest1 (AvlTreeStrT * pAvlTreeStr)
{
    Int32T i = 0;
    Int8T key[64] = {0,};
    testT *tt = NULL;

    for (i = 8; i >= 1; --i)
    {
        tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
        tt->data = i * 10;
        sprintf(key, "%d", tt->data);
        printf("tt%d : %d, key(%s)\n", i, tt->data, key);
        nnAvlAddNodeStr(pAvlTreeStr, key, tt);
    }
}

/* Insert Right Side 10 ~ 70 */
void avlInsertTest2 (AvlTreeStrT * pAvlTreeStr)
{
    Int32T i = 0;
    Int8T key[64] = {0,};
    testT *tt = NULL;

    for (i = 1; i <= 7; ++i)
    {
        tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
        tt->data = i * 10;
        sprintf(key, "%d", tt->data);
        printf("tt%d : %d, key(%s)\n",i, tt->data, key);
        nnAvlAddNodeStr(pAvlTreeStr, key, tt);
    }
}

/* Insert 10 5 8 */
void avlInsertTest3 (AvlTreeStrT * pAvlTreeStr)
{
    Int32T i = 0;
    Int32T j = 10;
    Int8T key[64] = {0,};
    testT *tt = NULL;

    for (i = 1; i <= 3; ++i)
    {
        tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));

        if (i != 1 && (i % 2) == 1)
        {
            j += 3;
        }
        else if ((i % 2) == 0)
        {
            j -= 5;
        }

        tt->data = j;
        sprintf(key, "%d", tt->data);
        printf("tt%d : %d, key(%s)\n",i, tt->data, key);
        nnAvlAddNodeStr(pAvlTreeStr, key, tt);
    }
}

/* Insert 10 15 12 */
void avlInsertTest4 (AvlTreeStrT * pAvlTreeStr)
{
    Int32T i = 0;
    Int32T j = 10;
    Int8T key[64] = {0,};
    testT *tt = NULL;

    for (i = 1; i <= 3; ++i)
    {
        tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));

        if (i != 1 && (i % 2) == 1)
        {
            j -= 3;
        }
        else if ((i % 2) == 0)
        {
            j += 5;
        }

        tt->data = j;
        sprintf(key, "%d", tt->data);
        printf("tt%d : %d, key(%s)\n",i, tt->data, key);
        nnAvlAddNodeStr(pAvlTreeStr, key, tt);
    }
}

/* Insert 10 Input Data */
void avlInsertTest5 (AvlTreeStrT * pAvlTreeStr)
{
    Int32T i = 0;
    Int32T j[10] = {90, 42, 67, 25, 14, 93, 10, 40, 3, 96};
    Int8T key[64] = {0,};
    testT *tt = NULL;

    for (i = 0; i <= 9; ++i)
    {
        tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));

        tt->data = j[i];
        sprintf(key, "%d", tt->data);
        printf("tt%d : %d, key(%s)\n",i, tt->data, key);
        nnAvlAddNodeStr(pAvlTreeStr, key, tt);
    }
}

/* Insert 50, 40, 60, 30, 70, 45, 43 Data */
void avlInsertTest6 (AvlTreeStrT * pAvlTreeStr)
{
    Int32T i = 0;
    //        key  1   2   3   4   5   6   7
    Int32T j[7] = {50, 40, 60, 30, 70, 45, 43};
    Int8T key[64] = {0,};
    testT *tt = NULL;

    for (i = 0; i <= 6; ++i)
    {
        tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));

        tt->data = j[i];
        sprintf(key, "%d", tt->data);
        printf("tt%d : %d, key(%s)\n",i, tt->data, key);
        nnAvlAddNodeStr(pAvlTreeStr, key, tt);
    }
}

/* Test Delete Insert 7 Input Data */
void avlDelTest1(AvlTreeStrT * pAvlTreeStr) {
    Int32T i = 0;
    //        key  1   2   3   4   5   6   7
    Int32T j[7] = {50, 40, 60, 30, 70, 45, 43};
    Int8T key[64] = {0,};
    testT *tt = NULL;

    for (i = 0; i <= 6; ++i)
    {
        tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));

        tt->data = j[i];
        sprintf(key, "%d", i + 1);
        printf("tt%d : %d, key(%s)\n",i, tt->data, key);
        nnAvlAddNodeStr(pAvlTreeStr, key, tt);
    }

    avlPrint(pAvlTreeStr->pRoot);

    strcpy(key, "1");
    LOG("Del Data =========================== 1");
    nnAvlDeleteNodeStr(pAvlTreeStr, key);
    avlPrint(pAvlTreeStr->pRoot);

    strcpy(key, "6");
    LOG("Del Data =========================== 6");
    nnAvlDeleteNodeStr(pAvlTreeStr, key);
    avlPrint(pAvlTreeStr->pRoot);

    strcpy(key, "7");
    LOG("Del Data =========================== 7");
    nnAvlDeleteNodeStr(pAvlTreeStr, key);
    avlPrint(pAvlTreeStr->pRoot);

    strcpy(key, "2");
    LOG("Del Data =========================== 2");
    nnAvlDeleteNodeStr(pAvlTreeStr, key);
    avlPrint(pAvlTreeStr->pRoot);

    strcpy(key, "3");
    LOG("Del Data =========================== 3");
    nnAvlDeleteNodeStr(pAvlTreeStr, key);
    avlPrint(pAvlTreeStr->pRoot);

    strcpy(key, "4");
    LOG("Del Data =========================== 4");
    nnAvlDeleteNodeStr(pAvlTreeStr, key);
    avlPrint(pAvlTreeStr->pRoot);

    strcpy(key, "5");
    LOG("Del Data =========================== 5");
    nnAvlDeleteNodeStr(pAvlTreeStr, key);
    avlPrint(pAvlTreeStr->pRoot);
}

/* Test Delete Insert 7 Input Data */
void avlDelTest2(AvlTreeStrT * pAvlTreeStr) {
    Int32T i = 0;
    //        key  1   2   3   4   5   6   7
    Int32T j[7] = {50, 40, 60, 30, 70, 45, 43};
    Int8T key[64] = {0,};
    testT *tt = NULL;

    for (i = 0; i <= 6; ++i)
    {
        tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));

        tt->data = j[i];
        sprintf(key, "%d", i + 1);
        printf("tt%d : %d, key(%s)\n",i, tt->data, key);
        nnAvlAddNodeStr(pAvlTreeStr, key, tt);
    }

    LOG("Del Data ===========================");

    nnAvlDeleteNodeStr(pAvlTreeStr, "5");
}

/* Test Lookup Data */
void nnAvlLookupDataTest(AvlTreeStrT *pAvlTreeStr) {
    testT *tt1 = NULL;

    Int32T i = 0;
    //        key  1   2   3   4   5   6   7
    Int32T j[7] = {50, 40, 60, 30, 70, 45, 43};
    Int8T key[64] = {0,};
    testT *tt = NULL;

    for (i = 0; i <= 6; ++i)
    {
        tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));

        tt->data = j[i];
        sprintf(key, "%d", i + 1);
        printf("tt%d : %d, key(%s)\n",i, tt->data, key);
        nnAvlAddNodeStr(pAvlTreeStr, key, tt);
    }

    LOG("Lookup Data ===========================");
    printf("Input Find Key : ");
    scanf("%s", key);

    tt1 = nnAvlLookupDataStr(pAvlTreeStr, key);

    if (tt1 == NULL)
    {
        printf("Can't Find Key = %s\n", key);
    }
    else
   {
        printf("The Lookup Key is = %s,  Data is = %d\n", key, tt1->data);
    }
}

Uint32T makeKeytoInt(StringT key)
{
    Uint32T sum = 0;
    Int32T keyLen = 0;

    keyLen = strlen(key);

    printf("Key Len : %d\n", keyLen);

    printf("Input Key is [%s]\n", key);

    while(keyLen-- > 0)
    {
        sum += (Uint8T)*key++;
    }

    return sum; 
}

int main() {
    Int32T menu = 0;
    Int32T run = TRUE;
    AvlTreeStrT *pAvlTreeStr = NULL;
    Int8T key[64];
    Uint32T ret = 0;

    LOG("AvlTree Sample Start");

    /* MemT Init */
    memInit(1);
    memSetDebug(TEST_TYPE);

    /* Log Init */
    nnLogInit(1);

    while (run)
    {
        printf("-------------------------- Menu --------------------------\n");
        printf("0 : Left Side Insert (10 ~ 70) - Right Rotate\n");
        printf("1 : Right Side Insert (70 ~ 10) - Left Rotate\n");
        printf("2 : Insert 10 5 8 - Left Right Rotate\n");
        printf("3 : Insert 10 15 12 - Right Left Rotate\n");
        printf("4 : Insert 10 Data (90, 42, 67, 25, 14, 93, 10, 40, 3, 96)\n");
        printf("5 : Insert Data (50, 40, 60, 30, 70, 45, 43)\n");
        printf("6 : Insert Data (50, 40, 60, 30, 70, 45, 43)\n \
                    Delete Data (50, 45, 43, 40, 60, 30, 70)\n");
        printf("7 : Insert Data (50, 40, 60, 30, 70, 45, 43)\n \
                    Delete Data (70)\n");
        printf("8 : Lookup Data\n");
        printf("9 : Exit\n");
        printf("Input Menu Number : ");
        scanf("%d", &menu);

        /* Init Test */
        LOG("Init");
        pAvlTreeStr = nnAvlInitStr(TEST_TYPE);

        switch (menu)
        {
            /* Insert Test */
            case 0:
                avlInsertTest1(pAvlTreeStr);
                break;
            case 1:
                avlInsertTest2(pAvlTreeStr);
                break;
            case 2:
                avlInsertTest3(pAvlTreeStr);
                break;
            case 3:
                avlInsertTest4(pAvlTreeStr);
                break;
            case 4:
                avlInsertTest5(pAvlTreeStr);
                break;
            case 5:
                avlInsertTest6(pAvlTreeStr);
                break;
            /* Delete Test */
            case 6:
                avlDelTest1(pAvlTreeStr);
                break;
            case 7:
                avlDelTest2(pAvlTreeStr);
                break;
            case 8:
                nnAvlLookupDataTest(pAvlTreeStr);
                break;
            case 9:
                run = FALSE;
                break;
            default:
                printf("Wrong Number\n");
                break;
        }

        if (menu < 9) {
            /* Print Data Test */
            LOG("Print AVL Tree ===========================");
            printf("Root\n");
            avlPrint(pAvlTreeStr->pRoot);
            printf("Total AvlTree Size: %lld\n\n",
                   nnAvlCountStr(pAvlTreeStr));
        }

        /* Free Test */
        LOG("Free");
        nnAvlFreeStr(pAvlTreeStr);
    }

    memShowAllUser(MEM_MAX_USER_TYPE);

    /* Log Close */
    nnLogClose();

    /* Mem Close */
    memClose();

    LOG("AvlTree Sample End");

    return 0;
}
