#include <stdio.h>
#include <stdlib.h>
#include "../nnList.h"
#include "../nnMemmgr.h"
#include "../nnLog.h"

/* You need to set User's Define for Memory Manager */
#define TEST_TYPE 0

/* Test Structure */
typedef struct testT {
	Int32T data;
} testT;

/* Print List */
void listPrint (ListT *pList)
{
        ListNodeT *pTempNode = NULL;

	ListSubT *pListSub = NULL;
        ListSubNodeT *pTempSubNode = NULL;

	printf("Datas\n");
        for (pTempNode = pList->pHead;
             pTempNode != NULL;
             pTempNode = pTempNode->pNext)
        {
		printf("Parent : %-4d ", ((testT *)pTempNode->pData)->data);
		printf("Sub -> ");

		pListSub = pTempNode->pSubHead;

		if (pListSub != NULL)
		{
		        for (pTempSubNode = pListSub->pHead;
                             pTempSubNode != NULL;
                             pTempSubNode = pTempSubNode->pNext)
		        {
				printf("%-5d ",
                                       ((testT *)pTempSubNode->pData)->data);
			}
		}

		printf("\n");
        }

        printf("\nlistCount : %llu\n\n", nnListCount(pList));
}

/* Data Compare Function */
Int8T dataCmpFunc(void *pOldData, void *pNewData)
{
	/* if you want to sort ASC, set this return 0,
           Left Side is Bigger than Right Side */
	if ((((testT *)pOldData)->data) > (((testT *)pNewData)->data))
	{
		return 0;	/* Sort */
	}
	/* if you want to sort DESC, set this return 0,
           Right Side is Bigger than Left Side */
	else if ((((testT *)pOldData)->data) < (((testT *)pNewData)->data))
	{
		return 1;	/* Not Used Now */
	}
	/* Same Data */
	else /* if (((testT *)pOldData)->data == ((testT *)pNewData)->data) */
	{
		return -1;	/* Used For nnListSearchNode */
	}
}

Int32T main() 
{
        ListT *list = NULL;
	Int32T errorNo = 0;

	/* Mem Init */
	/* You need to change the parameter, your process's number */
	memInit(1);
	memSetDebug(TEST_TYPE);	/* Memory Manager - Debug Mode */

	/* Log Init, Must After Mem Init */
	/* You need to change the parameter, your process's number */
	nnLogInit(1);

	/* If you want to Print STDOUT, Not need to this API */
	nnLogSetFlag(LOG_STDOUT); /* Default */

        NNLOGDEBUG(LOG_DEBUG, "@@ List Sample Start\n");

	/* List Init */
        list = nnListInit(dataCmpFunc, TEST_TYPE);

	/* Check Memory Alloc */
	if (list <= 0)
	{
		NNLOGDEBUG(LOG_DEBUG, "Init NO MEMORY\n");
		return -1;
	}
	else
	{
	        NNLOGDEBUG(LOG_DEBUG, "Init SUCCESS\n");
	}

	/* AddNode */
        testT *tt0 = NULL;
        tt0 = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
        tt0->data = 100;
	NNLOGDEBUG(LOG_DEBUG, "@@ AddNode : %d\n", tt0->data);

	nnListAddNode(list, tt0);

	/* Print List */
        listPrint(list);

        /* AddNode */
        testT *tt1 = NULL;
        tt1 = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
        tt1->data = 10;
	NNLOGDEBUG(LOG_DEBUG, "@@ AddNode : %d\n", tt1->data);

	errorNo = nnListAddNode(list, tt1);
	if (errorNo != SUCCESS)
	{
		NNLOGDEBUG(LOG_DEBUG, "AddNode FAILURE\n");
	}
	else
	{
		NNLOGDEBUG(LOG_DEBUG, "AddNode SUCCESS\n");
	}

        /* Print List */
        listPrint(list);

        /* AddNode */
        testT *tt2 = NULL;
        tt2 = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
        tt2->data = 20;
	NNLOGDEBUG(LOG_DEBUG, "@@ AddNode : %d\n", tt2->data);

	errorNo = nnListAddNode(list, tt2);
	if (errorNo != SUCCESS)
	{
		NNLOGDEBUG(LOG_DEBUG, "AddNode FAILURE\n");
	}
	else
	{
		NNLOGDEBUG(LOG_DEBUG, "AddNode SUCCESS\n");
	}

        /* Print List */
        listPrint(list);

        /* AddNode */
        testT *tt3 = NULL;
        tt3 = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
        tt3->data = 30;
	NNLOGDEBUG(LOG_DEBUG, "@@ AddNode : %d\n", tt3->data);

	errorNo = nnListAddNode(list, tt3);
	if (errorNo != SUCCESS)
	{
		NNLOGDEBUG(LOG_DEBUG, "AddNode FAILURE\n");
	}
	else
	{
		NNLOGDEBUG(LOG_DEBUG, "AddNode SUCCESS\n");
	}

        /* Print List */
        listPrint(list);

        /* SearchNode */
        NNLOGDEBUG(LOG_DEBUG, "@@ Search Sample\n");
        NNLOGDEBUG(LOG_DEBUG, " Search Data = %d\n", tt2->data);
        testT *ttResult = NULL;
	ListNodeT *tempNode = NULL;

        tempNode = nnListSearchNode(list, tt2);

	printf("----------------------\n");
	if (tempNode == NULL)
	{
		NNLOGDEBUG(LOG_DEBUG, "No Data\n");
	}
	else if ((Int32T)tempNode < 0)
	{
		NNLOGDEBUG(LOG_DEBUG, "Search Operation is Failed\n");
	}
	else
	{
        	ttResult = ((testT *)(tempNode->pData));
		NNLOGDEBUG(LOG_DEBUG,
                      "Result SearchNode Data : %d\n", ttResult->data);
	}

        /* AddNodePrev */
        tempNode = nnListSearchNode(list, tt2);
        testT *tt4 = NULL;
        tt4 = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
        tt4->data = 40;
	NNLOGDEBUG(LOG_DEBUG,
              "@@ AddNodePrev : tempNode : %d, AddNode : %d\n",
              tt2->data, tt4->data);

	errorNo = nnListAddNodePrev(list, tempNode, tt4);
	if (errorNo != SUCCESS)
	{
		NNLOGDEBUG(LOG_DEBUG, "AddNode FAILURE\n");
	}
	else
	{
		NNLOGDEBUG(LOG_DEBUG, "AddNode SUCCESS\n");
	}

        /* Print List */
        listPrint(list);

        /* AddNodeNext */
        testT *tt5 = NULL;
        tt5 = (testT *)NNMALLOC( TEST_TYPE, sizeof(testT));
        tt5->data = 50;
	NNLOGDEBUG(LOG_DEBUG,
              "@@ AddNodeNext : tempNode : %d, AddNode : %d\n",
              tt2->data, tt5->data);

	errorNo = nnListAddNodeNext(list, tempNode, tt5);
	if (errorNo != SUCCESS)
	{
		NNLOGDEBUG(LOG_DEBUG, "AddNodeNext FAILURE\n");
	}
	else
	{
		NNLOGDEBUG(LOG_DEBUG, "AddNodeNext SUCCESS\n");
	}

        /* Print List */
        listPrint(list);

        /* AddNodeDupNot */
        testT *tt6 = NULL;
        tt6 = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
        tt6->data = 40;
	Uint32T result = 0;

	NNLOGDEBUG(LOG_DEBUG, "@@ AddNodeDupNot : %d\n", tt6->data);

	errorNo = nnListAddNodeDupNot(list, tt6);
	if (errorNo == LIST_ERR_DUP)
	{
		NNLOGDEBUG(LOG_DEBUG, "AddNodeDupNot Duplication Found\n");
		NNFREE(TEST_TYPE, tt6);
	}
	else if (errorNo != SUCCESS)
	{
		NNLOGDEBUG(LOG_DEBUG, "AddNodeDupNot Operation is Failed\n");
		NNFREE(TEST_TYPE, tt6);
	}
	else
	{
		NNLOGDEBUG(LOG_DEBUG, "AddNodeDupNot No Duplication\n");
	}

	/* Print List */
        listPrint(list);

	/* AddNodeSort */
        testT *tt7 = NULL;
        tt7 = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
        tt7->data = 0;
	NNLOGDEBUG(LOG_DEBUG, "@@ AddNodeSort : %d\n", tt7->data);

	errorNo = nnListAddNodeSort(list, tt7);
	if (errorNo != SUCCESS)
	{
		NNLOGDEBUG(LOG_DEBUG, "AddNodeSort FAILURE\n");
	}
	else
	{
		NNLOGDEBUG(LOG_DEBUG, "AddNodeSort SUCCESS\n");
	}

	/* Print List */
        listPrint(list);

	/* AddNodeSort */
        testT *tt8 = NULL;
        tt8 = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
        tt8->data = 110;
	NNLOGDEBUG(LOG_DEBUG, "@@ AddNodeSort : %d\n", tt8->data);

	errorNo = nnListAddNodeSort(list, tt8);
	if (errorNo != SUCCESS)
	{
		NNLOGDEBUG(LOG_DEBUG, "AddNodeSort FAILURE\n");
	}
	else
	{
		NNLOGDEBUG(LOG_DEBUG, "AddNodeSort SUCCESS\n");
	}

	/* Print List */
        listPrint(list);

	/* AddNodeSort */
        testT *tt9 = NULL;
        tt9 = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
        tt9->data = 0;
	NNLOGDEBUG(LOG_DEBUG, "@@ AddNodeSort : %d\n", tt9->data);

	errorNo = nnListAddNodeSort(list, tt9);
	if (errorNo != SUCCESS)
	{
		NNLOGDEBUG(LOG_DEBUG, "AddNodeSort FAILURE\n");
	}
	else
	{
		NNLOGDEBUG(LOG_DEBUG, "AddNodeSort SUCCESS\n");
	}

	/* Print List */
        listPrint(list);

	/* AddNodeHead */
        testT *tt10 = NULL;
        tt10 = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
        tt10->data = 1010;
	NNLOGDEBUG(LOG_DEBUG, "@@ AddNodeHead : %d\n", tt10->data);

	errorNo = nnListAddNodeHead(list, tt10);
	if (errorNo != SUCCESS)
	{
		NNLOGDEBUG(LOG_DEBUG, "AddNodeHead FAILURE\n");
	}
	else
	{
		NNLOGDEBUG(LOG_DEBUG, "AddNodeHead SUCCESS\n");
	}

	/* Print List */
        listPrint(list);

        /* Delete Node */
	NNLOGDEBUG(LOG_DEBUG, "@@ DelNode : %d\n", tt1->data);
        nnListDeleteNode(list, tt1);

	NNLOGDEBUG(LOG_DEBUG,
              "===================================================\n");
        /* Print List */
        listPrint(list);
	NNLOGDEBUG(LOG_DEBUG,
              "===================================================\n");

	ListNodeT *ttt = NULL;

	/* GetHeadNode */
	ttt = nnListGetHeadNode(list);

	if (ttt == NULL)
	{
		NNLOGDEBUG(LOG_DEBUG, "No Head Node\n");
	}
	else if ((Int32T)ttt < 0)
	{
		NNLOGDEBUG(LOG_DEBUG, "GetHeadNode Operation Fail\n");
	}
	else
	{
		NNLOGDEBUG(LOG_DEBUG, "GetHeadNode Success : %d\n",
                                 ((testT *)ttt->pData)->data);
	}

	/* GetTailNode */
	ttt = nnListGetTailNode(NULL);

	if (ttt == NULL)
	{
		NNLOGDEBUG(LOG_DEBUG, "No Tail Node\n");
	}
	else if ((Int32T)ttt < 0)
	{
		NNLOGDEBUG(LOG_DEBUG, "GetTailNode Operation Fail\n");
	}
	else
	{
		NNLOGDEBUG(LOG_DEBUG, "GetTailNode Success : %d\n",
                                 ((testT *)ttt->pData)->data);
	}


	NNLOGDEBUG(LOG_DEBUG,
              "=============== SUB NODE Sample Start =====================\n");

        testT *tt100 = NULL;
        tt100 = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
        tt100->data = 900;

        testT *tt200 = NULL;
        tt200 = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
        tt200->data = 905;

	ttt = nnListSearchNode(list, tt2);

	if (ttt == NULL)
	{
		NNLOGDEBUG(LOG_DEBUG, "Can't Search Node\n");
	}
	else
	{
                NNLOGDEBUG(LOG_DEBUG, "Search ListNode OK\n");
                NNLOGDEBUG(LOG_DEBUG, "AddSubNode\n");

                /* Init ListSub */
                errorNo = nnListSubInit(ttt, dataCmpFunc, TEST_TYPE);

                if (errorNo < 0)
                {
                        printf("nnListSubInit Failed\n");
                }
                else
                {
                        /* AddSubNode */
                        nnListAddSubNode(ttt, tt100);
                        nnListAddSubNode(ttt, tt200);
                }
	}

	NNLOGDEBUG(LOG_DEBUG,
              "===================================================\n");
        /* Print List */
        listPrint(list);
	NNLOGDEBUG(LOG_DEBUG,
              "===================================================\n");

        testT *tt300 = NULL;
        tt300 = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
        tt300->data = 910;

        testT *tt400 = NULL;
        tt400 = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
        tt400->data = 915;

        testT *tt500 = NULL;
        tt500 = (testT *)NNMALLOC(TEST_TYPE, sizeof(testT));
        tt500->data = 90000;

	ttt = nnListSearchNode(list, tt0);

	if (ttt == NULL)
	{
		NNLOGDEBUG(LOG_DEBUG, "Can't Search Node\n");
	}
	else
	{
                NNLOGDEBUG(LOG_DEBUG, "Search ListNode OK\n");

                /* Init ListSub */
                errorNo = nnListSubInit(ttt, dataCmpFunc, TEST_TYPE);

                if (errorNo < 0)
                {
                        printf("nnListSubInit Failed\n");
                }
                else
                {
                        /* AddSubNodeHead */
                        nnListAddSubNode(ttt, tt300);

                        /* Sub Node Count */
                        NNLOGDEBUG(LOG_DEBUG, "ListSub Count : %llu\n",
                                         nnListSubCount(ttt));

                        /* AddSubNodeHead */
                        nnListAddSubNode(ttt, tt400);

                        /* Sub Node Count */
                        NNLOGDEBUG(LOG_DEBUG, "ListSub Count : %llu\n",
                                         nnListSubCount(ttt));

                        /* AddSubNodeHead */
                        nnListAddSubNodeHead(ttt, tt500);

                        /* Sub Node Count */
                        NNLOGDEBUG(LOG_DEBUG, "ListSub Count : %llu\n",
                                         nnListSubCount(ttt));
                }
	}

	NNLOGDEBUG(LOG_DEBUG,
              "===================================================\n");
        /* Print List */
        listPrint(list);
	NNLOGDEBUG(LOG_DEBUG,
              "===================================================\n");

	/* DeleteSubNode */
	NNLOGDEBUG(LOG_DEBUG, "=== Delete Data : %d\n", tt400->data);
	nnListDeleteSubNode(ttt, tt400);
	printf("ListSub Count : %llu\n", nnListSubCount(ttt));

	/* DeleteSubNode */
	nnListDeleteSubNode(ttt, tt400);
	printf("ListSub Count : %llu\n", nnListSubCount(ttt));

	NNLOGDEBUG(LOG_DEBUG,
              "===================================================\n");
        /* Print List */
        listPrint(list);
	NNLOGDEBUG(LOG_DEBUG,
              "===================================================\n");

        /* SearchSubNode */
	ListSubNodeT *subResult = NULL;
	NNLOGDEBUG(LOG_DEBUG, " Search SubNode : %d\n", tt300->data);

        /* SetSubNodeCompFunc */
	errorNo = nnListSetSubNodeCompFunc(ttt, dataCmpFunc);

	if(errorNo != SUCCESS)
	{
		NNLOGDEBUG(LOG_DEBUG,
                      "Set SubNode Data Compare Function is Failed\n");
	}
	else
	{
		NNLOGDEBUG(LOG_DEBUG,
                      "Set SubNode Data Compare Function is Success\n");
	}

        /* SearchSubNode */
	subResult = nnListSearchSubNode(ttt, tt300);

	NNLOGDEBUG(LOG_DEBUG, "----------------------\n");
	if (subResult == NULL)
	{
		NNLOGDEBUG(LOG_DEBUG, "No SubData\n");
	}
	else if ((Int32T)subResult < 0)
	{
		NNLOGDEBUG(LOG_DEBUG, "Operation Fail\n");
	}
	else
	{
        	ttResult = ((testT *)(subResult->pData));
		NNLOGDEBUG(LOG_DEBUG, "SearchSubNode Data : %d\n", ttResult->data);
	}

        /* SearchSubNode */
	NNLOGDEBUG(LOG_DEBUG, " Search SubNode : %d\n", tt100->data);

	subResult = nnListSearchSubNode(ttt, tt100);

	if (subResult == NULL)
	{
		NNLOGDEBUG(LOG_DEBUG, "No SubData\n");
	}
	else if ((Int32T)subResult < 0)
	{
		NNLOGDEBUG(LOG_DEBUG, "Operation Fail\n");
	}
	else
	{
	        ttResult = ((testT *)subResult->pData);
		NNLOGDEBUG(LOG_DEBUG, "SearchSubNode Data : %d", ttResult->data);
	}

        /* GetHeadSubNode */
	NNLOGDEBUG(LOG_DEBUG, " Get SubHeadNode\n");

	subResult = nnListGetHeadSubNode(ttt);

	if (subResult == NULL)
	{
		NNLOGDEBUG(LOG_DEBUG, "No SubData\n");
	}
	else if ((Int32T)subResult < 0)
	{
		NNLOGDEBUG(LOG_DEBUG, "Operation Fail\n");
	}
	else
	{
	        ttResult = ((testT *)subResult->pData);
		NNLOGDEBUG(LOG_DEBUG, "SearchSubNode Data : %d\n", ttResult->data);
	}

	NNLOGDEBUG(LOG_DEBUG,
              "===================================================\n");
        /* Print List */
        listPrint(list);
	NNLOGDEBUG(LOG_DEBUG,
              "===================================================\n");
	NNLOGDEBUG(LOG_DEBUG,
              "=============== SUB NODE Sample End =====================\n");

        /* List Free */
        nnListFree(list);
	list = NULL;
        NNLOGDEBUG(LOG_DEBUG, "@@ Free\n");

        NNLOGDEBUG(LOG_DEBUG, "List Sample End\n");
	nnLogClose();

	memShowAllUser(TEST_TYPE);

	/* Mem Close */
	memClose();

        return 0;
}

