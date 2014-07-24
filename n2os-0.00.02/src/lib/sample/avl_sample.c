#include <stdio.h>
#include <stdlib.h>
#include "../nnDefines.h"
#include "../nnAvl.h"
#include "../nnStr.h"
#include "../nnMemmgr.h"

#define TEST_TYPE 0

void avlPrint (AvlNodeT * pAvlNode);
void avlInsertTest1 (AvlTreeT * pAvlTree);
void avlInsertTest2 (AvlTreeT * pAvlTree);
void avlInsertTest3 (AvlTreeT * pAvlTree);
void avlInsertTest4 (AvlTreeT * pAvlTree);
void avlInsertTest5 (AvlTreeT * pAvlTree);
void avlDelTest1 (AvlTreeT * pAvlTree);
void avlDelTest2 (AvlTreeT * pAvlTree);
void nnAvlLookupDataTest(AvlTreeT *pAvlTree);
void avlLongestMatch (AvlTreeT *pAvlTree);
void avlExactMatch (AvlTreeT *pAvlTree);

/* Test Structure */
typedef struct testT {
        int data;
} testT;

void avlPrint (AvlNodeT * pAvlNode)
{
	if (pAvlNode == NULL)
	{
		LOG("No Data");
	}
	else
	{
		if (pAvlNode->pParent != NULL)
			printf("Parent = %d ", ((testT *)pAvlNode->pParent->pData)->data);
		else
			printf("Parent = NULL ");

		printf("Data = %d, Height = %d, Balance = %d\n", ((testT *)pAvlNode->pData)->data, (pAvlNode->height), (pAvlNode->balance));

		printf("Left\n");
		avlPrint(pAvlNode->pLeft);

		printf("Right\n");
		avlPrint(pAvlNode->pRight);
	}
}

void avlPrefixIpPrint (AvlNodeT * pAvlNode)
{
	if (pAvlNode == NULL)
	{
		LOG("No Data");
	}
	else
	{
		Int8T ip[MAX_IPV4_LEN + MAX_IPV4_PREFIX_LEN + 2]; // 2 -> '/' + '\0'
//printf("%s\n", inet_ntoa(((PrefixT *)pAvlNode->pData)->u.prefix4));
		nnCnvPrefixtoString(ip, ((PrefixT *)pAvlNode->pData));
		printf("IP = %s, Height = %d, Balance = %d\n", ip, (pAvlNode->height), (pAvlNode->balance));
//		printf("Data = %p, Height = %d, Balance = %d\n", ((PrefixT *)pAvlNode->pData), (pAvlNode->height), (pAvlNode->balance));

		printf("Left\n");
		avlPrefixIpPrint(pAvlNode->pLeft);

		printf("Right\n");
		avlPrefixIpPrint(pAvlNode->pRight);
	}
}


/* Data Compare Function */
Int8T dataCmpFunc(void *pOldData, void *pNewData)
{
/*      printf("Compare : %d, %d\n", ((testT *)pOldData)->data, ((testT *)pNewData)->data); */
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

/* Data Compare Function */
Int8T dataCmpFunc2(void *pOldData, void *pNewData)
{
/*      printf("Compare : %d, %d\n", ((testT *)pOldData)->data, ((testT *)pNewData)->data); */
        /* if sort then ASC */
        if ((((PrefixT *)pOldData)->prefixLen) > (((PrefixT *)pNewData)->prefixLen))
        {
                return 0;
        }
        /* if sort then DESC */
        else if ((((PrefixT *)pOldData)->prefixLen) < (((PrefixT *)pNewData)->prefixLen))
        {
                return 1;
        }
        /* same data */
        else /* if (((testT *)pOldData)->data == ((testT *)pNewData)->data) */
        {
                return -1;
        }
}


/* Insert Left Side 80 ~ 10 */
void avlInsertTest1 (AvlTreeT * pAvlTree)
{
	Int32T i = 0;
	testT *tt = NULL;

	for (i = 8; i >= 1; --i)
	{
//		tt = (testT *)malloc(sizeof(testT));
		tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
		tt->data = i * 10;
		printf("tt%d : %d\n",i, tt->data);
		nnAvlAddNode(pAvlTree, tt);
	}
}

/* Insert Right Side 10 ~ 70 */
void avlInsertTest2 (AvlTreeT * pAvlTree)
{
	Int32T i = 0;
	testT *tt = NULL;

	for (i = 1; i <= 7; ++i)
	{
//		tt = (testT *)malloc(sizeof(testT));
		tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
		tt->data = i * 10;
		printf("tt%d : %d\n",i, tt->data);
		nnAvlAddNode(pAvlTree, tt);
	}
}

/* Insert 10 5 8 */
void avlInsertTest3 (AvlTreeT * pAvlTree)
{
	Int32T i = 0;
	Int32T j = 10;
	testT *tt = NULL;

	for (i = 1; i <= 3; ++i)
	{
//		tt = (testT *)malloc(sizeof(testT));
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
		printf("tt%d : %d\n",i, tt->data);
		nnAvlAddNode(pAvlTree, tt);
	}
}

/* Insert 10 15 12 */
void avlInsertTest4 (AvlTreeT * pAvlTree)
{
	Int32T i = 0;
	Int32T j = 10;
	testT *tt = NULL;

	for (i = 1; i <= 3; ++i)
	{
//		tt = (testT *)malloc(sizeof(testT));
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
		printf("tt%d : %d\n",i, tt->data);
		nnAvlAddNode(pAvlTree, tt);
	}
}

/* Insert 10 Input Data */
void avlInsertTest5 (AvlTreeT * pAvlTree)
{
	Int32T i = 0;
	Int32T j[10] = {90, 42, 67, 25, 14, 93, 10, 40, 3, 96};
	testT *tt = NULL;

	for (i = 0; i <= 9; ++i)
	{
//		tt = (testT *)malloc(sizeof(testT));
		tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));

		tt->data = j[i];
		printf("tt%d : %d\n",i, tt->data);
		nnAvlAddNode(pAvlTree, tt);
	}
}

/* Test Delete Insert 7 Input Data */
void avlDelTest1(AvlTreeT * pAvlTree) {
	Int32T i = 0;
	Int32T j[7] = {50, 40, 60, 30, 70, 45, 43};
	testT *tt = NULL;

	for (i = 0; i <= 6; ++i)
	{
//		tt = (testT *)malloc(sizeof(testT));
		tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));

		tt->data = j[i];
		printf("tt%d : %d\n",i, tt->data);
		nnAvlAddNode(pAvlTree, tt);
	}

avlPrint(pAvlTree->pRoot);

//	tt = (testT *)malloc(sizeof(testT));
	tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));

	LOG("Del Data ===========================");
	tt->data = 50;
	nnAvlDeleteNode(pAvlTree, tt);

	LOG("Del Data ===========================");
	tt->data = 45;
	nnAvlDeleteNode(pAvlTree, tt);

	LOG("Del Data ===========================");
	tt->data = 43;
	nnAvlDeleteNode(pAvlTree, tt);

	LOG("Del Data ===========================");
	tt->data = 40;
	nnAvlDeleteNode(pAvlTree, tt);

	LOG("Del Data ===========================");
	tt->data = 60;
	nnAvlDeleteNode(pAvlTree, tt);

	LOG("Del Data ===========================");
	tt->data = 30;
	nnAvlDeleteNode(pAvlTree, tt);

	LOG("Del Data ===========================");
	tt->data = 70;
	nnAvlDeleteNode(pAvlTree, tt);

//	free(tt);
	NNFREE(TEST_TYPE, tt);
}

/* Test Delete Insert 7 Input Data */
void avlDelTest2(AvlTreeT * pAvlTree) {
	Int32T i = 0;
	Int32T j[7] = {50, 40, 60, 30, 70, 45, 43};
	testT *tt = NULL;

	for (i = 0; i <= 6; ++i)
	{
//		tt = (testT *)malloc(sizeof(testT));
		tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));

		tt->data = j[i];
		printf("tt%d : %d\n",i, tt->data);
		nnAvlAddNode(pAvlTree, tt);
	}

	LOG("Del Data ===========================");
	
//	tt = (testT *)malloc(sizeof(testT));
	tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
	tt->data = 70;
	nnAvlDeleteNode(pAvlTree, tt);

//	free(tt);
	NNFREE(TEST_TYPE, tt);
}

/* Test Lookup Data */
void nnAvlLookupDataTest(AvlTreeT *pAvlTree) {
	testT *tt1 = NULL;
	testT *tt2 = NULL;

	Int32T i = 0;
	Int32T j[7] = {50, 40, 60, 30, 70, 45, 43};
	testT *tt = NULL;

	for (i = 0; i <= 6; ++i)
	{
//		tt = (testT *)malloc(sizeof(testT));
		tt = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));

		tt->data = j[i];
		printf("tt%d : %d\n",i, tt->data);
		nnAvlAddNode(pAvlTree, tt);
	}

	LOG("Lookup Data ===========================");
	printf("Input Find Data : ");
	scanf("%d", &i);

//	tt1 = (testT *)malloc(sizeof(testT));
	tt1 = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
	tt1->data = i;

	tt2 = nnAvlLookupData(pAvlTree, tt1);

	if (tt2 == NULL)
	{
		printf("Can't Find Data = %d\n", tt1->data);
	}
	else
	{
		printf("The Lookup Data is = %d\n", tt2->data);
	}

//	free(tt1);
	NNFREE(TEST_TYPE, tt1);
}

void avlLongestMatch (AvlTreeT *pAvlTree) {
	StringT ips[] = {"192.168.0.100/16", "192.168.0.105/22", "192.168.0.14/26",
			"192.168.0.23/32", "192.168.0.49/24", "192.168.0.244/30",
			"192.168.20.201/24"};
	Int32T len = 7;
	PrefixT *tt = NULL;
	PrefixT *result = NULL;
	Int32T i = 0;
	Int8T ip[MAX_IPV4_LEN + MAX_IPV4_PREFIX_LEN + 2] = {0,};
	Int8T ret = 0;

	for (i = 0; i < len; ++i)
	{
		tt = (PrefixT *)NNMALLOC(TEST_TYPE, sizeof(PrefixT));
		ret = nnCnvStringtoPrefix(tt, ips[i]);

		if (i < len - 1) {
			printf("%d INPUT DATA ============= %s\n", ret, ips[i]);
			nnAvlAddNode(pAvlTree, tt);
		}
	}

printf("PREFIX IP PRINT\n");
	avlPrefixIpPrint(pAvlTree->pRoot);

	printf("Lookup LONGEST MATCH!!!!!!!!!\n");

	printf("INPUT DATA!!!!!!!\n");

	ret = nnCnvPrefixtoString(ip, tt);

	printf("IP : %s, PrefixLen : %d, Family : %d\n", ip, tt->prefixLen, tt->family);

	printf("FIND!!!!!!!!!!!!\n");
	result = nnAvlLookupDataLongestMatch(pAvlTree, tt);

	printf("RESULT!!!!!!!!!!!!!!!\n");

	ret = nnCnvPrefixtoString(ip, result);

	if (ret >= 0)
	printf("-> IP : %s, PrefixLen : %d, Family : %d\n", ip, result->prefixLen, result->family);

        NNFREE(TEST_TYPE, tt);
}

void avlExactMatch (AvlTreeT *pAvlTree) {
	StringT ips[] = {"192.168.0.100/24", "192.168.0.105/24", "192.168.0.14/24",
			"192.168.0.23/24", "192.168.0.49/24", "192.168.0.245/24",
			"192.168.0.201/24"};

	Int32T len = 7;
	PrefixT *tt = NULL;
	PrefixT *result = NULL;
	Int32T i = 0;
	Int8T ip[MAX_IPV4_LEN + MAX_IPV4_PREFIX_LEN + 2] = {0,};
	Int8T ret = 0;
	for (i = 0; i < len; ++i)
	{
		tt = (PrefixT *)NNMALLOC(TEST_TYPE, sizeof(PrefixT));
		nnCnvStringtoPrefix(tt, ips[i]);

		if (i < len) {
			printf("INPUT DATA ============= %s\n", ips[i]);
			nnAvlAddNode(pAvlTree, tt);
		}
	}

	avlPrefixIpPrint(pAvlTree->pRoot);

	printf("Lookup EXACT MATCH!!!!!!!!!\n");

	printf("INPUT DATA!!!!!!!\n");

	ret = nnCnvPrefixtoString(ip, tt);

	printf("IP : %s, PrefixLen : %d, Family : %d\n", ip, tt->prefixLen, tt->family);

	printf("FIND!!!!!!!!!!!!\n");
	result = nnAvlLookupDataExactMatch(pAvlTree, tt);

	printf("RESULT!!!!!!!!!!!!!!!\n");

	ret = nnCnvPrefixtoString(ip, result);

	if (ret >= 0)
	printf("IP : %s, PrefixLen : %d, Family : %d\n", ip, result->prefixLen, result->family);
}


int main() {
	Int32T menu = 0;
	Int32T run = TRUE;
	AvlTreeT *pAvlTree = NULL;

	LOG("AvlTree Sample Start");

	/* MemT Init */
	memInit(1);
	memSetDebug(TEST_TYPE);

	/* Log Init */
	nnLogInit(1);

	while (run)
	{
		printf("------------------------------- Menu -------------------------------\n");
		printf("0 : Left Side Insert (10 ~ 70) - Right Rotate\n");
		printf("1 : Right Side Insert (70 ~ 10) - Left Rotate\n");
		printf("2 : Insert 10 5 8 - Left Right Rotate\n");
		printf("3 : Insert 10 15 12 - Right Left Rotate\n");
		printf("4 : Insert 10 Data (90, 42, 67, 25, 14, 93, 10, 40, 3, 96)\n");
		printf("5 : Insert Data (50, 40, 60, 30, 70, 45, 43)\n    Delete Data (50, 45, 43, 40, 60, 30, 70)\n");
		printf("6 : Insert Data (50, 40, 60, 30, 70, 45, 43)\n    Delete Data (70)\n");
		printf("7 : Lookup Data\n");
		printf("8 : LongestMatch \n");
		printf("9 : ExactMatch \n");
		printf("10 : Exit\n");
		printf("Input Menu Number : ");
		scanf("%d", &menu);

		/* Init Test */
		LOG("Init");
		pAvlTree = nnAvlInit(dataCmpFunc, TEST_TYPE);

		switch (menu)
		{
			/* Insert Test */
			case 0:
				avlInsertTest1(pAvlTree);
				break;
			case 1:
				avlInsertTest2(pAvlTree);
				break;
			case 2:
				avlInsertTest3(pAvlTree);
				break;
			case 3:
				avlInsertTest4(pAvlTree);
				break;
			case 4:
				avlInsertTest5(pAvlTree);
				break;
			/* Delete Test */
			case 5:
				avlDelTest1(pAvlTree);
				break;
			case 6:
				avlDelTest2(pAvlTree);
				break;
			case 7:
				nnAvlLookupDataTest(pAvlTree);
				break;
			case 8:
				nnAvlFree(pAvlTree);
				pAvlTree = nnAvlInit(dataCmpFunc2, TEST_TYPE);
				avlLongestMatch(pAvlTree);
				break;
			case 9:
				nnAvlFree(pAvlTree);
				pAvlTree = nnAvlInit(dataCmpFunc2, TEST_TYPE);
				avlExactMatch(pAvlTree);
				break;
			case 10:
				run = FALSE;
				break;
			default:
				printf("Wrong Number\n");
				break;
		}

		if (menu < 8) {
			/* Print Data Test */
			LOG("Print AVL Tree ===========================");
			printf("Root\n");
			avlPrint(pAvlTree->pRoot);
			printf("Total AvlTree Size: %llu\n\n", nnAvlCount(pAvlTree));
		}

		/* Free Test */
		LOG("Free");
		nnAvlFree(pAvlTree);
	}

	memShowAllUser(MEM_MAX_USER_TYPE);

	/* Log Close */
	nnLogClose();

	/* Mem Close */
	memClose();

	LOG("AvlTree Sample End");

	return 0;
}
