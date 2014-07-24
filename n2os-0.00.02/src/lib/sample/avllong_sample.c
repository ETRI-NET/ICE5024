#include <stdio.h>
#include <stdlib.h>
#include "../nnDefines.h"
#include "../nnAvlLong.h"
#include "../nnStr.h"
#include "../nnMemmgr.h"

#define TEST_TYPE 0

void avlPrint (AvlNodeLongT * pAvlNodeLong);
void avlInsertTest1 (AvlTreeLongT * pAvlTreeLong);
void avlInsertTest2 (AvlTreeLongT * pAvlTreeLong);
void avlInsertTest3 (AvlTreeLongT * pAvlTreeLong);
void avlInsertTest4 (AvlTreeLongT * pAvlTreeLong);
void avlInsertTest5 (AvlTreeLongT * pAvlTreeLong);
void avlInsertTest6 (AvlTreeLongT * pAvlTreeLong);
void avlDelTest1 (AvlTreeLongT * pAvlTreeLong);
void avlDelTest2 (AvlTreeLongT * pAvlTreeLong);
void nnAvlLookupDataTest(AvlTreeLongT *pAvlTreeLong);

/* Test Structure */
typedef struct testT {
        int data;
} testT;

void avlPrint (AvlNodeLongT * pAvlNodeLong)
{
    if (pAvlNodeLong == NULL)
    {
        LOG("No Data");
    }
    else
    {
        if (pAvlNodeLong->pParent != NULL)
        {
            printf("Parent = %d ", \
                   ((testT *)pAvlNodeLong->pParent->pData)->data);
        }
        else
        {
            printf("Parent = NULL ");
        }

        printf("Key = %lld, Data = %d, Height = %d, Balance = %d\n", \
                pAvlNodeLong->key, ((testT *)pAvlNodeLong->pData)->data, \
                (pAvlNodeLong->height), (pAvlNodeLong->balance));

        printf("Left\n");
        avlPrint(pAvlNodeLong->pLeft);

        printf("Right\n");
        avlPrint(pAvlNodeLong->pRight);
    }
}


/* Insert Left Side 80 ~ 10 */
void avlInsertTest1 (AvlTreeLongT * pAvlTreeLong)
{
    Int32T i = 0;
    Int64T key = 0;
    testT *tt = NULL;

    for (i = 8; i >= 1; --i)
    {
        tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
        tt->data = i * 10;
        printf("tt%d : %d, key(%lld)\n", i, tt->data, key);
        nnAvlAddNodeLong(pAvlTreeLong, key, tt);
    }
}

/* Insert Right Side 10 ~ 70 */
void avlInsertTest2 (AvlTreeLongT * pAvlTreeLong)
{
    Int32T i = 0;
    Int64T key = 0;
    testT *tt = NULL;

    for (i = 1; i <= 7; ++i)
    {
        tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
        tt->data = i * 10;
        printf("tt%d : %d, key(%lld)\n",i, tt->data, ++key);
        nnAvlAddNodeLong(pAvlTreeLong, key, tt);
    }
}

/* Insert 10 5 8 */
void avlInsertTest3 (AvlTreeLongT * pAvlTreeLong)
{
    Int32T i = 0;
    Int32T j = 10;
    Int64T key = 0;
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
        printf("tt%d : %d, key(%lld)\n",i, tt->data, ++key);
        nnAvlAddNodeLong(pAvlTreeLong, key, tt);
    }
}

/* Insert 10 15 12 */
void avlInsertTest4 (AvlTreeLongT * pAvlTreeLong)
{
    Int32T i = 0;
    Int32T j = 10;
    Int64T key = 0;
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
        printf("tt%d : %d, key(%lld)\n",i, tt->data, ++key);
        nnAvlAddNodeLong(pAvlTreeLong, key, tt);
    }
}

/* Insert 10 Input Data */
void avlInsertTest5 (AvlTreeLongT * pAvlTreeLong)
{
    Int32T i = 0;
    Int32T j[10] = {90, 42, 67, 25, 14, 93, 10, 40, 3, 96};
    Int64T key = 0;
    testT *tt = NULL;

    for (i = 0; i <= 9; ++i)
    {
        tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));

        tt->data = j[i];
        printf("tt%d : %d, key(%lld)\n",i, tt->data, ++key);
        nnAvlAddNodeLong(pAvlTreeLong, key, tt);
    }
}

/* Insert 50, 40, 60, 30, 70, 45, 43 Data */
void avlInsertTest6 (AvlTreeLongT * pAvlTreeLong)
{
    Int32T i = 0;
    //        key  1   2   3   4   5   6   7
    Int32T j[7] = {50, 40, 60, 30, 70, 45, 43};
    Int64T key = 0;
    testT *tt = NULL;

    for (i = 0; i <= 6; ++i)
    {
        tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));

        tt->data = j[i];
        printf("tt%d : %d, key(%lld)\n",i, tt->data, ++key);
        nnAvlAddNodeLong(pAvlTreeLong, key, tt);
    }
}

/* Test Delete Insert 7 Input Data */
void avlDelTest1(AvlTreeLongT * pAvlTreeLong) {
    Int32T i = 0;
    //        key  1   2   3   4   5   6   7
    Int32T j[7] = {50, 40, 60, 30, 70, 45, 43};
    Int64T key = 0;
    testT *tt = NULL;

    for (i = 0; i <= 6; ++i)
    {
        tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));

        tt->data = j[i];
        printf("tt%d : %d, key(%lld)\n",i, tt->data, ++key);
        nnAvlAddNodeLong(pAvlTreeLong, key, tt);
    }

    avlPrint(pAvlTreeLong->pRoot);

    LOG("Del Data =========================== 1");
    nnAvlDeleteNodeLong(pAvlTreeLong, 1);
    avlPrint(pAvlTreeLong->pRoot);

    LOG("Del Data =========================== 6");
    nnAvlDeleteNodeLong(pAvlTreeLong, 6);
    avlPrint(pAvlTreeLong->pRoot);

    LOG("Del Data =========================== 7");
    nnAvlDeleteNodeLong(pAvlTreeLong, 7);
    avlPrint(pAvlTreeLong->pRoot);

    LOG("Del Data =========================== 2");
    nnAvlDeleteNodeLong(pAvlTreeLong, 2);
    avlPrint(pAvlTreeLong->pRoot);

    LOG("Del Data =========================== 3");
    nnAvlDeleteNodeLong(pAvlTreeLong, 3);
    avlPrint(pAvlTreeLong->pRoot);

    LOG("Del Data =========================== 4");
    nnAvlDeleteNodeLong(pAvlTreeLong, 4);
    avlPrint(pAvlTreeLong->pRoot);

    LOG("Del Data =========================== 5");
    nnAvlDeleteNodeLong(pAvlTreeLong, 5);
    avlPrint(pAvlTreeLong->pRoot);
}

/* Test Delete Insert 7 Input Data */
void avlDelTest2(AvlTreeLongT * pAvlTreeLong) {
    Int32T i = 0;
    //        key  1   2   3   4   5   6   7
    Int32T j[7] = {50, 40, 60, 30, 70, 45, 43};
    Int64T key = 0;
    testT *tt = NULL;

    for (i = 0; i <= 6; ++i)
    {
        tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));

        tt->data = j[i];
        printf("tt%d : %d, key(%lld)\n",i, tt->data, ++key);
        nnAvlAddNodeLong(pAvlTreeLong, key, tt);
    }

    LOG("Del Data ===========================");

    nnAvlDeleteNodeLong(pAvlTreeLong, 5);
}

/* Test Lookup Data */
void nnAvlLookupDataTest(AvlTreeLongT *pAvlTreeLong) {
    testT *tt1 = NULL;

    Int32T i = 0;
    //        key  1   2   3   4   5   6   7
    Int32T j[7] = {50, 40, 60, 30, 70, 45, 43};
    Int64T key = 0;
    testT *tt = NULL;

    for (i = 0; i <= 6; ++i)
    {
        tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));

        tt->data = j[i];
        printf("tt%d : %d, key(%lld)\n",i, tt->data, ++key);
        nnAvlAddNodeLong(pAvlTreeLong, key, tt);
    }

    LOG("Lookup Data ===========================");
    printf("Input Find Key : ");
    scanf("%lld", &key);

    tt1 = nnAvlLookupDataLong(pAvlTreeLong, key);

    if (tt1 == NULL)
    {
        printf("Can't Find Key = %lld\n", key);
    }
    else
    {
        printf("The Lookup Key is = %lld,  Data is = %d\n", key, tt1->data);
    }
}

int main() {
    Int32T menu = 0;
    Int32T run = TRUE;
    AvlTreeLongT *pAvlTreeLong = NULL;

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
        pAvlTreeLong = nnAvlInitLong(TEST_TYPE);

        switch (menu)
        {
            /* Insert Test */
            case 0:
                avlInsertTest1(pAvlTreeLong);
                break;
            case 1:
                avlInsertTest2(pAvlTreeLong);
                break;
            case 2:
                avlInsertTest3(pAvlTreeLong);
                break;
            case 3:
                avlInsertTest4(pAvlTreeLong);
                break;
            case 4:
                avlInsertTest5(pAvlTreeLong);
                break;
            case 5:
                avlInsertTest6(pAvlTreeLong);
                break;
            /* Delete Test */
            case 6:
                avlDelTest1(pAvlTreeLong);
                break;
            case 7:
                avlDelTest2(pAvlTreeLong);
                break;
            case 8:
                nnAvlLookupDataTest(pAvlTreeLong);
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
            avlPrint(pAvlTreeLong->pRoot);
            printf("Total AvlTree Size: %lld\n\n",
                   nnAvlCountLong(pAvlTreeLong));
        }

        /* Free Test */
        LOG("Free");
        nnAvlFreeLong(pAvlTreeLong);
    }

    memShowAllUser(MEM_MAX_USER_TYPE);

    /* Log Close */
    nnLogClose();

    /* Mem Close */
    memClose();

    LOG("AvlTree Sample End");

    return 0;
}
