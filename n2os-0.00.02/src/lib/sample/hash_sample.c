#include <stdio.h>
#include <stdlib.h>
#include "../nnHash.h"
#include "../nnMemmgr.h"

#define MAX_TEST_INDEX_SIZE 16

#define TEST_TYPE 0

/* Test Structure */
typedef struct testT {
        Int32T data;
} testT;

void hashAddTest(HashT *pHash);
void hashReleaseTest(HashT *pHash);
void hashPrint(HashT *pHash);

/* Use hashInit() */
Uint32T hashMakeKeyFunction(void *pData) {
	Uint32T key = 0;
	key = ((testT *)pData)->data % MAX_TEST_INDEX_SIZE;


	printf("make key : %d\n", key);
	return key;
}

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

/* Add Bucket Test */
void hashAddTest(HashT *pHash)
{
	testT *pTest = NULL;

	Int32T i = 0;
	for(i = 0; i < MAX_TEST_INDEX_SIZE * 2; ++i)
	{
		pTest = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
		pTest->data = i;
		nnHashAddBucket(pHash, pTest);
	}
}

/* Release Bucket Test */
void hashReleaseTest(HashT *pHash)
{
	testT *pFind = NULL;
	testT *pResult = NULL;
	Int32T data = 0;

	pFind = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));

	printf("Input Release Data : ");
	scanf("%d", &data);
	pFind->data = data;

	pResult = (testT *)nnHashRelease(pHash, pFind);

	if((Int32T)pResult == HASH_NO_DATA)
	{
		printf("No Result Data\n");
	}
	else
	{
		printf("Result Data = %d\n", pResult->data);

		NNFREE(TEST_TYPE, pResult);
		pResult = NULL;
	}

	NNFREE(TEST_TYPE, pFind);
	pFind = NULL;
}

/* Delete Index's Bucket Test */
void hashDeleteAllBucketTest(HashT *pHash, Uint64T index)
{
	nnHashDeleteAllBucket(pHash, index);
}

/* Print Bucket Test */
void hashPrint(HashT *pHash)
{
	Int32T i = 0;
	HashBucketT *pTempBucket = NULL;
	for (i = 0; i < MAX_TEST_INDEX_SIZE; ++i)
	{
		printf("INDEX : %d -> ", i);

		for (pTempBucket = pHash->index[i]; pTempBucket != NULL; pTempBucket = pTempBucket->next)
		{
			printf("%d ", ((testT *)(pTempBucket->data))->data);
		}
		printf("\n");
	}
	printf("Total Count : %llu\n", nnHashCount(pHash));
}

Int32T main()
{
	Int32T menu = 0;
	Int32T data = 0;
	Int32T run = TRUE;
	HashT *pHash = NULL;
	memInit(1);

	nnLogInit(1);

	LOG("Hash Sample Start");
	printf("\n!!! Max Hash Index Size = %d\n", MAX_TEST_INDEX_SIZE);
	printf("!!! Index = (Data %% MAX_TEST_INDEX_SIZE)\n\n");

	while (run)
        {
		printf("------------------------------- Menu -------------------------------\n");
		printf("0 : Add Bucket (0 ~ 19)\n");
		printf("1 : hashReleaseBucket Test\n");
		printf("2 : Delete All Bucket Test\n");
		printf("3 : Delete All Index Test\n");
		printf("4 : Exit\n");
		printf("Input Menu Number : ");
		scanf("%d", &menu);

		switch (menu)
		{
			/* hashAddBucket */
			case 0:
				LOG("Add Bucket");

				/* Init Hash */
				pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE, hashMakeKeyFunction, hashCompareFunction);
				if((Int32T)pHash <= 0)
				{
					LOG("Init Failure");
					pHash = NULL;
					break;
				}

				/* Insert Bucket Test */
				LOG("Add Bucket");
				hashAddTest(pHash);

				/* Print Bucket Test */
				LOG("Print Test");
				hashPrint(pHash);

				/* Free Bucket Test */
				LOG("Free");
				nnHashFree(pHash);
				pHash = NULL;

				break;
			/* hashReleaseBucket */
			case 1:
				LOG("Release Bucket");

				/* Init Hash */
				pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE, hashMakeKeyFunction, hashCompareFunction);
				if((Int32T)pHash <= 0)
				{
					LOG("Init Failure");
					pHash = NULL;
					break;
				}

				/* Insert Bucket Test */
				LOG("Add Bucket");
				hashAddTest(pHash);

				/* Print Bucket Test */
				LOG("Print Test");
				hashPrint(pHash);

				/* Release Bucket Test */
				LOG("Release Bucket");
				hashReleaseTest(pHash);

				/* Print Bucket Test */
				LOG("Print Test");
				hashPrint(pHash);

				/* Free Bucket Test */
				LOG("Free");
				nnHashFree(pHash);
				pHash = NULL;

				break;
			/* hashDeleteAllBucket */
			case 2:
				LOG("Delete All Bucket");

				printf("Input Index Number : ");
				scanf("%d", &data);

				/* Init Hash */
				pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE, hashMakeKeyFunction, hashCompareFunction);
				if((Int32T)pHash <= 0)
				{
					LOG("Init Failure");
					pHash = NULL;
					break;
				}

				/* Insert Bucket Test */
				LOG("Add Bucket");
				hashAddTest(pHash);

				/* Delete All Bucket Test */
				LOG("Delete All Bucket");
				hashDeleteAllBucketTest(pHash, data);

				/* Print Bucket Test */
				LOG("Print Test");
				hashPrint(pHash);

				/* Free Bucket Test */
				LOG("Free");
				nnHashFree(pHash);
				pHash = NULL;

				break;
			/* hashDeleteAllIndex */
			case 3:
				LOG("Delete All Index");

				/* Init Hash */
				pHash = nnHashInit(MAX_TEST_INDEX_SIZE, TEST_TYPE, hashMakeKeyFunction, hashCompareFunction);
				if((Int32T)pHash <= 0)
				{
					LOG("Init Failure");
					pHash = NULL;
					break;
				}

				/* Insert Bucket Test */
				LOG("Add Bucket");
				hashAddTest(pHash);

				/* Print Bucket Test */
				LOG("Print Test");
				hashPrint(pHash);

				/* Delete All Index */
				LOG("Delete All Index");
				nnHashDeleteAllIndex(pHash);

				/* Print Bucket Test */
				LOG("Print Test");
				hashPrint(pHash);

				/* Free Bucket Test */
				LOG("Free");
				nnHashFree(pHash);
				pHash = NULL;

				break;
			/* Exit */
			case 4:
				run = FALSE;
				break;
			default:
				printf("Wrong Number\n");
				break;
		}
	}

	if(pHash != NULL)
	{
		nnHashFree(pHash);
	}

	LOG("Hash Sample End");
	nnLogClose();

	memShowAllUser(TEST_TYPE);

	memClose();

	return 0;
}
