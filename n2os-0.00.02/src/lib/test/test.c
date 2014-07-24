#include "test.h"

Int32T getFileData(TestHead *pTestHead, StringT fileName) {
        FILE *fp = NULL;
        Int8T szBuffer[256] = {0,};
        Int32T ret = 0;
        Int32T m1 = 0;
        Int32T m2 = 0;
        Int32T m3 = 0;
        Int32T count = 0;
	StringT sharp = NULL;

        fp = fopen(fileName, "r");
        if (fp == NULL)
        {
                return FAILURE;
        }

        while(fgets(szBuffer, 255, fp)) {
		if ((sharp = strstr(szBuffer, "#")) != NULL)
		{
			continue;
		}

                Test *pTest = (Test *)malloc(sizeof(Test));
                pTest->pNext = NULL;

                ret = sscanf(szBuffer, "%d %d %d", &pTest->m1, &pTest->m2, &pTest->m3);
                printf("Menu : %d %d %d\n", pTest->m1, pTest->m2, pTest->m3);
                memset(szBuffer, 0, sizeof(szBuffer));

                if (pTestHead->count == 0)
                {
                        pTestHead->pHead = pTest;
                        pTestHead->pTail = pTest;
                }
                else
                {
                        pTestHead->pTail->pNext = pTest;
                        pTestHead->pTail = pTest;
                }

                ++pTestHead->count;
        }

        fclose(fp);
	return SUCCESS;
}

void freeTestHead(TestHead *pTestHead) {
        Test *pTemp = NULL;

        printf("Test Count = %d\n", pTestHead->count);

        for(pTemp = pTestHead->pHead; pTemp != NULL; pTemp = pTestHead->pHead)
        {
                pTestHead->pHead = pTemp->pNext;
                free(pTemp);
                pTemp = NULL;
        }

        free(pTestHead);
        pTestHead = NULL;
}
