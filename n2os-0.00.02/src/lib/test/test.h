#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../nnTypes.h"

typedef struct Test
{
        Int32T m1; // Menu
        Int32T m2; // Number
        Int32T m3; // Count
        struct Test *pNext;
} Test;

typedef struct TestHead
{
        Test *pHead;
        Test *pTail;
        Int32T count;
} TestHead;

Int32T getFileData(TestHead *pTestHead, StringT fileName);
void freeTestHead(TestHead *pTestHead);
