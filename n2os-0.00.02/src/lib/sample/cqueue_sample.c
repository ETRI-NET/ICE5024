#include <stdio.h>
#include <stdlib.h>
#include "../nnCqueue.h"
#include "../nnMemmgr.h"
#include "../nnLog.h"

#define TEST_TYPE 0

/* Test Structure */
typedef struct testT {
        int data;
} testT;

static int queueprint(CqueueT * cqueue);

int main() 
{
        CqueueT *cqueue = NULL;
	void * data;

        LOG("Cqueue Sample Start\n");

	memInit(1);
	memSetDebug(TEST_TYPE);
        cqueue = nnCqueueInit(CQUEUE_DEFAULT_COUNT, CQUEUE_NOT_DUPLICATED, TEST_TYPE);
        LOG("Init\n");

	nnLogInit(1);

        /* AddNode Test */
        testT *tt1 = NULL;
     // tt1 = (testT *)malloc(sizeof(testT));
	tt1 = NNMALLOC(TEST_TYPE, sizeof(testT));
        tt1->data = 10;
        nnCqueueEnqueue(cqueue, tt1);
	queueprint (cqueue);

        /* AddNode Test */
        testT *tt2 = NULL;
    //  tt2 = (testT *)malloc(sizeof(testT));
      	tt2 = NNMALLOC(TEST_TYPE, sizeof(testT));
	tt2->data = 20;
        nnCqueueEnqueue(cqueue, tt2);
	queueprint (cqueue);

        /* AddNode Test */
        testT *tt3 = NULL;
      //tt3 = (testT *)malloc(sizeof(testT));
	tt3 = NNMALLOC(TEST_TYPE, sizeof(testT));
        tt3->data = 30;
        nnCqueueEnqueue(cqueue, tt3);
	queueprint (cqueue);

        /* AddNodePrev Test */
        testT *tt4 = NULL;
        //tt4 = (testT *)malloc(sizeof(testT));
        tt4 = NNMALLOC(TEST_TYPE, sizeof(testT));
	tt4->data = 40;
        nnCqueueEnqueue(cqueue, tt4);
	queueprint (cqueue);

        /* AddNodeNext Test */
        testT *tt5 = NULL;
     // tt5 = (testT *)malloc(sizeof(testT));
        tt5 = NNMALLOC(TEST_TYPE, sizeof(testT));
	tt5->data = 50;

	nnCqueueEnqueue(cqueue, tt5);
	queueprint (cqueue);


	printf("return : %d\n", nnCqueueEnqueue(cqueue, tt5));
	queueprint (cqueue);

        /* Delete Node Test */
        data = nnCqueueDequeue(cqueue);
	printf("Dequeue : %d\n", ((testT *)data)->data);
	queueprint (cqueue);
	NNFREE(TEST_TYPE, data);
	
	data = nnCqueueDequeue(cqueue);
	printf("Dequeue : %d\n", ((testT *)data)->data);
	queueprint (cqueue);
	NNFREE(TEST_TYPE, data);

	data = nnCqueueDequeue(cqueue);
        printf("Dequeue : %d\n", ((testT *)data)->data);
	queueprint (cqueue);
	NNFREE(TEST_TYPE, data);

	data = nnCqueueDequeue(cqueue);
        printf("Dequeue : %d\n", ((testT *)data)->data);
	queueprint (cqueue);
	NNFREE(TEST_TYPE, data);

	data = nnCqueueDequeue(cqueue);
        printf("Dequeue : %d\n", ((testT *)data)->data);
	NNFREE(TEST_TYPE, data);

	data = nnCqueueDequeue(cqueue);
	if(data == NULL)
	{
		printf("NULL\n");
	}

	queueprint (cqueue);

        /* Free Test */
//        cqueueFree(cqueue);
        LOG("Free\n");

        printf("cqueueSize : %llu\n", nnCqueueCount(cqueue));

        LOG("Cqueue Sample End\n");

	nnCqueueFree(cqueue);

	nnLogClose();

	memShowAllUser(TEST_TYPE);

	memClose();

        return 0;
}

static int queueprint(CqueueT * cqueue)
{
	CqueueNodeT * tempNode = NULL;

	tempNode = cqueue->head;

	if(tempNode == NULL)
	{
		printf("no data\n");
		return 0;
	}

	for(; tempNode != cqueue->tail; tempNode = tempNode->next)
	{
		printf("data : %d\n", ((testT *)tempNode->data)->data);
	}
	printf("data : %d\n", ((testT *)tempNode->data)->data);

	printf("cqueueSize : %llu\n\n", nnCqueueCount(cqueue));

	return 0;
}


