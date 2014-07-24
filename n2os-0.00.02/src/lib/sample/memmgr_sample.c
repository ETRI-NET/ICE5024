#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "../nnMemmgr.h"

#define TEST_TYPE 0
#define mm() do{s_m = mallinfo(); fprintf(stdout, "%d/%d --\n", s_m.arena, s_m.uordblks);}while(0) 

/* Test Structure */
typedef struct testT {
        int data;
	int data2;
	char kkk[5];
} testT;

struct mallinfo s_m;

int main() 
{
	Uint32T i, j;
	struct testT * ta;
	Uint32T size;
	void * tB = NULL;
	void * tC = NULL;

	memInit(1);
	nnLogInit(1);

	memShowAllUser(MEM_MAX_USER_CNT);
/*
	printf(" memmgr address : %p \n", mem);	

	printf(" process : %d\n totalAlloc : %d\n totalFree : %d\n currentAlloc : %d\n debug_flag : %d\n\n",
		mem->process, mem->totalAlloc, mem->totalFree, mem->currentAlloc, mem->debug_flag);

	for(i=0; i<MEM_MAX_USER_CNT; i++)
	{
		printf("mem->typeAlloc[%d].totalAlloc = %d\n", i, mem->typeAlloc[i].totalAlloc);
		printf("mem->typeAlloc[%d].totalFree = %d\n", i, mem->typeAlloc[i].totalFree);
		printf("mem->typeAlloc[%d].currentAlloc = %d\n", i, mem->typeAlloc[i].currentAlloc);
		printf("mem->typeAlloc[%d].debug = %p\n\n", i, mem->typeAlloc[i].debug);
	}

	printf("=========================================================================\n");
	
*/
	memSetDebug(TEST_TYPE);
	memSetFile(NULL, 1);
	memShowAllUser(MEM_MAX_USER_CNT);
/*
	printf(" process : %d\n totalAlloc : %d\n totalFree : %d\n currentAlloc : %d\n debug_flag : %d\n\n",
                mem->process, mem->totalAlloc, mem->totalFree, mem->currentAlloc, mem->debug_flag);
	
	for(i=0; i<MEM_MAX_USER_CNT; i++)
        {
                printf("mem->typeAlloc[%d].totalAlloc = %d\n", i, mem->typeAlloc[i].totalAlloc);
                printf("mem->typeAlloc[%d].totalFree = %d\n", i, mem->typeAlloc[i].totalFree);
                printf("mem->typeAlloc[%d].currentAlloc = %d\n", i, mem->typeAlloc[i].currentAlloc);
                printf("mem->typeAlloc[%d].debug = %p\n\n", i, mem->typeAlloc[i].debug);
        }

	printf("=========================================================================\n");
*/
	ta = NNMALLOC(0, sizeof(testT));

	memShowAllUser(MEM_MAX_USER_CNT);
//	printf ("ta = %p\n", ta);
//	printf ("ta sizeof() : %d \n", malloc_usable_size((void *)ta));

	NNFREE(0, ta);

	memShowAllUser(MEM_MAX_USER_CNT);

	//sleep(3);
	tB = NNMALLOC(1, 100);
	memShowAllUser(MEM_MAX_USER_CNT);

	tC= NNMALLOC(1, 300);
	memShowAllUser(MEM_MAX_USER_CNT);

	tB = NNREALLOC(1, tB, 200);
	memShowAllUser(MEM_MAX_USER_CNT);

	//memShowCheckList (mem, MEM_MAX_USER_CNT, 0);

	NNFREE(1, tB);
	memShowAllUser(MEM_MAX_USER_CNT);
	NNFREE(1, tC);
	memShowAllUser(MEM_MAX_USER_CNT);

	//memShowCheckList (MEM_MAX_USER_CNT, 1);

	nnLogClose();

	memUnsetDebug(TEST_TYPE);

	memClose();
}
