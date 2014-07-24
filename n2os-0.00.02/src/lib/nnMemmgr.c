/*******************************************************************************
 *            Electronics and Telecommunications Research Institute
 * Copyright: 2013 Electronics and Telecommunications Research Institute.
 *            All rights reserved.
 *            No part of this software shall be reproduced, stored in a
 *            retrieval system, or transmitted by any means, electronic,
 *            mechanical, photocopying, recording, or otherwise, without
 *            written permission from ETRI.
*******************************************************************************/

/**
 * @brief : Memory Management Service
 *  - Block Name : Library
 *  - Process Name :
 *  - Creator : Changwoo Lee, JaeSu Han
 *  - Initial Date : 2013/7/15
*/

/**
 * @file : nnMemmgr.c
 *
 * $Id:
 * $Author:
 * $Date:
 * $Revision:
 * $LastChangedBy:
 **/

/*******************************************************************************
 *                               INCLUDE FILES
 ******************************************************************************/

#include "nnMemmgr.h"
#include "nnLog.h" 

/*******************************************************************************
 *                              GLOBAL VARIABLES
 ******************************************************************************/


/*******************************************************************************
 *                         GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 *                         LOCAL CONSTANTS/LITERALS/TYPES
 ******************************************************************************/

/* Memory Operation Type String */
const Int8T sMemmgrTypeName[][10] = 
{
    "",
    "Malloc",
    "Realloc",
    "Free"

};

/* Memory Complete 여부 String */
const Int8T sMemmgrCompleteName[][10] = 
{
    "Continue",
    "Complete"

};

/* System Memory Type String */
const Int8T sMemmgrSysTypeName[][16] = 
{
    "CQUEUE_HEAD",
    "CQUEUE_NODE",
    "LIST_HEAD",
    "LIST_NODE",
    "LIST_SUB_HEAD",
    "LIST_SUB_NODE",
    "HASH_HEAD",
    "HASH_BUCKET",
    "AVL_TREE_HEAD",
    "AVL_TREE_NODE",
    "MESSAGE_HEADER",
    "IPC_MESSAGE",
    "PROCESS_SIG_IN",
    "EVENT_SUB_MSG",
    "EVENT_SUB_PCS",
    "LOG",
    "DLU_SEND_MESSAGE"

};


/* User Memory Type String */
const Int8T sMemmgrUserTypeName[][16] =
{
/* PROCESS_MANAGER MEMORY TYPE */
   "PROC_REGISTER",
   "PROC_TIMER",

/* RIB_MANAGER MEMORY TYPE */
    "RIB_IF",
    "RIB_IF_NAME",
    "RIB_IF_LABEL",
    "RIB_VRF",
    "RIB_VRF_NAME",
    "RIB_CONNECTED",
    "RIB",
    "RIB_NEXTHOP",

/* POLICY_MANAGER MEMORY TYPE */
    "POLICY_STD_ACL",
    "POLICY_EXT_ACL",

/* RIP MEMORY TYPE */
    "RIP_IF",
    "RIP_PEER",
    "RIP_OFFSET",
    "RIP_ROUTE",
    "RIP_OTHER"

};


/*******************************************************************************
*                               LOCAL VARIABLES
*******************************************************************************/


/*******************************************************************************
 *                            LOCAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/

static MemT * memInitialize(Uint32T process, Uint32T memMaxCnt);
static void memEnd(MemT * memMgr);
static void memSetDebugMode(MemT * memMgr, Uint32T memMaxCnt);
static void memUnsetDebugMode(MemT * memMgr, Uint32T memMaxCnt);
static void memSetFilePrint(MemT * memMgr, StringT fileName, Uint32T size);
static void memUnsetFilePrint(MemT * memMgr);

static void * memMallocate(MemT * memMgr, Uint32T type, SizeT size, \
                    StringT file, StringT func, Uint32T line);
static void * memReallocate(MemT * memMgr, Uint32T type, void * reMem, \
                 SizeT size, StringT file, StringT func, Uint32T line);
static Int8T memFreeMem(MemT * memMgr, Uint32T type, void * freeMem, \
                         StringT file, StringT func, Uint32T line);

// static void memShowCheckList(MemT * memMgr, Uint32T index, Int8T type);

static Int64T memDebugPtrCheck \
       (MemT * memMgr, Uint32T memType, StringT ptr, SizeT size);

static Int8T memMallocDebug(MemT * memMgr, Uint32T memType, SizeT size, \
                        StringT ptr, StringT file, StringT func, Uint32T line);
static Int8T memReallocDebug(MemT * memMgr, Uint32T memType, SizeT size, \
          Int64T refer, StringT ptr, StringT file, StringT func, Uint32T line);
static Int8T memFreeDebug(MemT * memMgr, Uint32T memType, SizeT size, \
          Int64T refer, StringT ptr, StringT file, StringT func, Uint32T line);

static void memShowAll(MemT * memMgr, Uint32T index);


static CircularQueueT * cQueueInit(Uint64T maxCount, Uint8T dupCheck);
static void cQueueFree(CircularQueueT * cqueue);
static void cQueueDeleteAllNode(CircularQueueT * cqueue);
static Int32T cQueueEnqueue
    (CircularQueueT * cqueue, Uint32T dataType, Uint64T dataSize, void * data);

static void * cQueueGetHead(CircularQueueT * cqueue);
static void * cQueueGetTail(CircularQueueT * cqueue);
static void * cQueueGetData(CircularQueueT * cqueue, Uint32T index);

/* Current Not Used Function 
static Uint32T cQueueDeleteNode (CircularQueueT * cqueue, Uint32T index);
static Uint64T cQueueCount(CircularQueueT * cqueue);
static void * cQueueDequeue(CircularQueueT * cqueue);
*/


/*******************************************************************************
 *                                   CODE
 ******************************************************************************/

/**
 * Description: Memory Management Service를 생성하고 초기화하는 함수
 *
 * @param [in] process : Memory Management Service를 사용할 Process
 *
 * @retval : SUCCESS(0) : 성공,
 *           FAILURE(-1) : 실패
 *
 * Example Code Usage
 * @code
 *  Int8T ret = 0;
 *  ret = memInit(RIB_MANAGER);
 * @endcode 
 */

Int32T _memInit(Uint32T process)
{
    // 1. User Memory Manager Initialize

    gMemUserMgr = memInitialize(process, MEM_MAX_USER_CNT);
    if(gMemUserMgr == (MemT *)FAILURE)
    {
        return FAILURE;
    }

    // 2. System Memmory Manager Initialize

    gMemSysMgr = memInitialize(process, MEM_MAX_SYS_CNT);
    if(gMemSysMgr == (MemT *)FAILURE)
    {
        return FAILURE;
    }

    return SUCCESS;
}

/*******************************************************************************
 * Function: <memInitialize>
 *
 * Description: <Initialize the Memory Management Service>
 *
 * Parameters: process : Use Process Name
 *             memMaxCnt : Use Memory Max Count
 *
 * Returns: Memory Management Servicee is initalized
*******************************************************************************/

static MemT * memInitialize(Uint32T process, Uint32T memMaxCnt)
{
    MemT * newMgr = NULL;
    Uint32T i;

    // 1. Memory Aloocate

    newMgr = (MemT *)malloc(sizeof(MemT));
    if(newMgr == NULL)
    {
        return (MemT *) FAILURE;
    }

    // 2. Initialize

    newMgr->process = process;
    newMgr->totalAlloc = 0;
    newMgr->totalFree = 0;
    newMgr->currentAlloc = 0;
    newMgr->debug_flag = MEMMGR_UNSET_DEBUG;
    newMgr->file_flag = MEMMGR_UNSET_FILE;
    newMgr->logMaxSize = 0;

    for(i=0; i<memMaxCnt; i++)
    {
        newMgr->typeAlloc[i].totalAlloc = 0;
        newMgr->typeAlloc[i].totalFree = 0;
        newMgr->typeAlloc[i].currentAlloc = 0;
        newMgr->typeAlloc[i].debug = NULL;
    }

    return newMgr;
}

/**
 * Description: Memory Management Service를 삭제하고 종료하는 함수
 *
 * @retval : None
 *
 * Example Code Usage
 * @code
 *  memClose();
 * @endcode

 */

void _memClose(void)
{
    // 1. Memory Manager Unset Debug
    memUnsetDebugMode(gMemUserMgr, MEM_MAX_USER_CNT);
    memUnsetDebugMode(gMemSysMgr, MEM_MAX_SYS_CNT);

    // 2. Memory Manager Close
    memEnd(gMemUserMgr);
    gMemUserMgr = NULL;

    memEnd(gMemSysMgr);
    gMemSysMgr = NULL;
}

/*******************************************************************************
 * Function: <memEnd>
 *
 * Description: <Close the Memory Management Service>
 *
 * Parameters: memMgr: Close the Memory Management Service
 *
 * Returns: none
*******************************************************************************/

static void memEnd(MemT * memMgr)
{
    // Memory Manager Free

    if(memMgr != NULL)
    {
        free(memMgr);
        memMgr = NULL;
    }
}

/**
 * Description: Debug Mode를 설정하는 함수
 *
 * @param [in] memType : 설정 할 Memory Type
 *
 * @retval : SUCCESS(0) : 성공,
 *           MEMMGR_ERR_ARGUMENT(-4) : 잘못된 인자 값
 *
 * Example Code Usage
 * @code
 *  Int8T ret = 0;
 *  ret = memSetDebug(MEM_MAX_USER_CNT);
 * @endcode
 */

Int32T _memSetDebug(Uint32T memType)
{

    if(memType > MEM_MAX_USER_CNT)
    {
        return MEMMGR_ERR_ARGUMENT;
    }

    // Debug Mode Set

    if(memType == MEM_MAX_USER_CNT)
    {
        memSetDebugMode(gMemUserMgr, MEM_MAX_USER_CNT);
    }
    else
    {
        memSetDebugMode(gMemUserMgr, memType);
    }

    memSetDebugMode(gMemSysMgr, MEM_MAX_SYS_CNT);
    _memSetFile(NULL, 0);

    return SUCCESS;
}


/*******************************************************************************
 * Function: <memSetDebugMode>
 *
 * Description: <Set the debug mode>
 *
 * Parameters: memMgr: Set the debug mode the Memory Management Service
 *             memMaxCnt : Memory Max Count or One Memory Type
 *
 * Returns: none
*******************************************************************************/

static void memSetDebugMode(MemT * memMgr, Uint32T memMaxCnt)
{
    Uint32T i;

    // 1. memType unit Debug Mode Init. 2013.11.14
 
    if ((memMgr == gMemUserMgr) && (memMaxCnt < MEM_MAX_USER_CNT))
    {
        Uint32T memType = memMaxCnt;
            
        if (memMgr->typeAlloc[memType].debug == NULL)
        {
            memMgr->typeAlloc[memType].debug =
                cQueueInit(MEMMGR_MAX_DEBUG_SIZE, CQUEUE_NOT_DUPLICATED);
        }

        memMgr->debug_flag = MEMMGR_SET_DEBUG;

        return ;    
    }

    // 2. DEFAULT Debug Mode Init

    for (i=0; i<memMaxCnt; i++)
    {
        if (memMgr->typeAlloc[i].debug == NULL)
        {
            memMgr->typeAlloc[i].debug =
                cQueueInit(MEMMGR_MAX_DEBUG_SIZE, CQUEUE_NOT_DUPLICATED);
        }
    }

    memMgr->debug_flag = MEMMGR_SET_DEBUG;
}

/**
 * Description: Debug Mode 설정을 해제하는 함수
 *
 * @param [in] memType : 해제할 Memory Type
 *
 * @retval : SUCCESS(0) : 성공,
 *           MEMMGR_ERR_ARGUMENT(-4) : 잘못된 인자 값
 *
 * Example Code Usage
 * @code
 *  Int8T ret = 0;
 *  ret = memUnsetDebug(MEM_MAX_USER_CNT);
 * @endcode
 */

Int32T _memUnsetDebug(Uint32T memType)
{

    if (memType > MEM_MAX_USER_CNT)
    {
        return MEMMGR_ERR_ARGUMENT;
    }

    // Debug Mode Unset

    if (memType == MEM_MAX_USER_CNT)
    {
        memUnsetDebugMode(gMemUserMgr, MEM_MAX_USER_CNT);
        memUnsetDebugMode(gMemSysMgr, MEM_MAX_SYS_CNT);

        gMemUserMgr->debug_flag = MEMMGR_UNSET_DEBUG;
        gMemSysMgr->debug_flag = MEMMGR_UNSET_DEBUG;
    }
    else
    {
        Uint32T i, memDebugCnt = 0;  

        memUnsetDebugMode(gMemUserMgr, memType);

        for(i=0; i<MEM_MAX_USER_CNT; i++)
        {
            if((gMemUserMgr->typeAlloc[i].debug) != NULL)
            {
                memDebugCnt++;
            }
        }

        if(!memDebugCnt)
        {
            memUnsetDebugMode(gMemSysMgr, MEM_MAX_SYS_CNT);
            gMemUserMgr->debug_flag = MEMMGR_UNSET_DEBUG;
            gMemSysMgr->debug_flag = MEMMGR_UNSET_DEBUG;
        }
    }

    return SUCCESS;
}


/*******************************************************************************
 * Function: <memUnsetDebugMode>
 *
 * Description: <Unset the debug mode>
 *
 * Parameters: memMgr: Unset the debug mode the Memory Management Service
 *             memMaxCnt : Use Memory Max Count or One Memory Type
 *
 * Returns: none
*******************************************************************************/

static void memUnsetDebugMode(MemT * memMgr, Uint32T memMaxCnt)
{ 
    Uint32T i;

    if(memMgr == NULL)
    {
        return ;
    }

    // 1. memType unit Debug Unset. 2013.11.14

    if ((memMgr == gMemUserMgr) && (memMaxCnt < MEM_MAX_USER_CNT))
    {
        Uint32T memType = memMaxCnt;

        if (memMgr->typeAlloc[memType].debug != NULL)
        {
            cQueueFree(memMgr->typeAlloc[memType].debug);
            memMgr->typeAlloc[memType].debug = NULL;
        }

        return ;
    }

    // 2. Debug Mode Unset

    for(i=0; i<memMaxCnt; i++)
    {
        if(memMgr->typeAlloc[i].debug != NULL)
        {
            cQueueFree(memMgr->typeAlloc[i].debug);
            memMgr->typeAlloc[i].debug = NULL;
        }
    }
}

/**
 * Description: File 출력을 설정하는 함수
 *
 * @param [in] filename : 설정 할 파일 이름
 * @param [in] size : 파일의 최대 크기
 *
 * @retval : None
 *
 * Example Code Usage
 * @code
 *  memSetFile("ribmgr", 2);
 * @endcode
 */

void _memSetFile(StringT fileName, Uint32T size)
{
    if(fileName == NULL)
    {
        fileName = MEMMGR_DEFAULT_LOG_FILE_NAME;
    }
   
    if(size <= 0)
    {
        size = MEMMGR_DEFAULT_LOG_FILE_SIZE;
    }
    else
    {
        size = size * MEMMGR_DEFAULT_LOG_FILE_SIZE;
    }

    memSetFilePrint(gMemUserMgr, fileName, size);
    memSetFilePrint(gMemSysMgr, fileName, size);
}

/*******************************************************************************
 * Function: <memSetFilePrint>
 *
 * Description: <Set the file flag>
 *
 * Parameters: memMgr: Set the file flag the Memory Management Service
 *             fileName : write file name
 *             size : max file size (Mbyte)
 *
 * Returns: none
*******************************************************************************/

static void memSetFilePrint(MemT * memMgr, StringT fileName, Uint32T size)
{
    memMgr->file_flag = MEMMGR_SET_FILE;
    memMgr->fileName = fileName;
    memMgr->logMaxSize = size;
}

/**
 * Description: File 출력을 해제하는 함수
 *
 * @retval : None
 * Example Code Usage
 * @code
 *  memUnsetFile();
 * @endcode
 *
 */

void _memUnsetFile(void)
{
    memUnsetFilePrint(gMemUserMgr);
    memUnsetFilePrint(gMemSysMgr);
}

/*******************************************************************************
 * Function: <memUnsetFilePrint>
 *
 * Description: <Unset the file flag>
 *
 * Parameters: memMgr: Unset the file flag the Memory Management Service
 *
 * Returns: none
*******************************************************************************/

static void memUnsetFilePrint(MemT * memMgr)
{
    memMgr->file_flag = MEMMGR_UNSET_FILE;
}


/**
 * Description: Memory 할당 함수
 *
 * @param [in] type : Memory Allocation DATA TYPE
 * @param [in] size : Memory Allocation Size
 * @param [in] file : Memory operation (Allocation) file name
 * @param [in] func : Memory operation (Allocation) function name
 * @param [in] line : Memory operation (Allocation) line number
 *
 * @retval : 할당된 Memory 주소 값 : 성공,
 *           MEMMGR_ERR_ALLOC(-1) : 메모리 할당 실패
 *           MEMMGR_ERR_DEBUG_ALLOC(-2) : Debug 구조체 메모리 할당 실패,
 *           MEMMGR_ERR_DEBUG_ENQUEUE(-3) : Debug 구조체 Enqueue 실패
 *
 * Example Code Usage
 * @code
 *  StringT msg = NULL;
 *  msg = NNMALLOC(POLICY_STD_ACL, 1024);
 * @endcode
 *
 */

void * _memMalloc
    (Uint32T type, SizeT size, StringT file, StringT func, Uint32T line)
{
    if(type < MEM_SYS_START)
    {
        return memMallocate(gMemUserMgr, type, size, file, func, line);
    }
    else
    {
        return memMallocate
               (gMemSysMgr, type-MEM_SYS_START, size, file, func, line);
    }
}

/*******************************************************************************
 * Function: <memMallocate>
 *
 * Description: <Memory Allocation>
 *
 * Parameters: memMgr: Memory Management Service
 *	       type  : Memory Allocation DATA TYPE
 * 	       size  : Memory Allocation Size
 *	       file  : Memory operation (Allocation) file name
 * 	       func  : Memory operation (Allocation) function name
 *	       line  : Memory operation (Allocation) line number
 *
 * Returns: The Memory allocated,
 *          -1 : MEMMGR_ERR_ALLOC,
 *          -2 : MEMMGR_ERR_DEBUG_ALLOC, 
 *          -3 : MEMMGR_ERR_DEBUG_ENQUEUE
*******************************************************************************/


static void * memMallocate
    (MemT * memMgr, Uint32T type, SizeT size, 
     StringT file, StringT func, Uint32T line)
{
    void * newMem = NULL;
    SizeT allocSize;
    Int8T newPtr[MAX_PTR_SIZE];
    Int32T ret;

    newMem = (void *)malloc(size);
    if(newMem == NULL)
    {
        if(memMgr->debug_flag == MEMMGR_SET_DEBUG)
        {
	    if((ret = memMallocDebug(memMgr, type, 0, NULL, file, func, line))
                    != SUCCESS)
            {
               return (void *)ret;
            }
        }

        return (void *)MEMMGR_ERR_ALLOC;
    }

    memset(newMem, 0x00, size);
    allocSize = malloc_usable_size(newMem);

    memMgr->totalAlloc += allocSize;
    memMgr->currentAlloc += allocSize;
    memMgr->typeAlloc[type].totalAlloc += allocSize;
    memMgr->typeAlloc[type].currentAlloc += allocSize;

    if(memMgr->debug_flag == MEMMGR_SET_DEBUG)
    {
        if(memMgr->typeAlloc[type].debug != NULL)
        {
            sprintf(newPtr, "%p", newMem);

            if((ret = memMallocDebug
                      (memMgr, type, allocSize, newPtr, file, func, line))
                    != SUCCESS)
            {
                return (void *)ret;
            }
        }
    }


    return newMem;
}


/*******************************************************************************
 * Function: <memMallocDebug>
 *
 * Description: <Memory Allocation, Debug Mode>
 *
 * Parameters: memMgr: Memory Management Service
 *             type  : Memory Allocation DATA TYPE
 *             size  : Memory Allocation Size (System Allocation Size)
 *	       ptr   : Memory address
 *             file  : Memory operation (Allocation) file name
 *             func  : Memory operation (Allocation) function name
 *             line  : Memory operation (Allocation) line number
 *
 * Returns: 0 : SUCCESS, 
 *         -2 : MEMMGR_ERR_DEBUG_ALLOC, 
 *         -3 : MEMMGR_ERR_DEBUG_ENQUEUE
*******************************************************************************/

static Int8T memMallocDebug
    (MemT * memMgr, Uint32T memType, SizeT size, 
    StringT ptr, StringT file, StringT func, Uint32T line)
{
    Int32T ret;
    MemDebugT * newDebugMem = (MemDebugT *)malloc(sizeof(MemDebugT));
    if(newDebugMem == NULL)
    {
        return MEMMGR_ERR_DEBUG_ALLOC;
    }

    newDebugMem->type = MEMMGR_MALLOC;
    newDebugMem->size = size;
    strcpy(newDebugMem->file, file);
    strcpy(newDebugMem->func, func);
    newDebugMem->line = line;

    nnTimeGetCurrent(newDebugMem->time);

    if(ptr != NULL)
    {
        strcpy(newDebugMem->ptr, ptr);
        newDebugMem->check = MEMMGR_MEM_CONTINUE;
    }
    else
    {
        sprintf(newDebugMem->ptr, "0x00000000");
	    newDebugMem->check = MEMMGR_MEM_CHECK;
    }

    newDebugMem->refer = MEMMGR_MEM_NOREFERENACE;

    while(1)
    {
        ret = cQueueEnqueue(memMgr->typeAlloc[memType].debug, 0,
                            sizeof(newDebugMem), newDebugMem);
        if(ret == CQUEUE_ERR_FULL)
        {
            nLOG(LOG_DEBUG, "CQUEUE FULL\n");
            _memShowAllUser(memType);
            cQueueDeleteAllNode(memMgr->typeAlloc[memType].debug);
            nLOG(LOG_DEBUG, "CQUEUE DELETE ALL NODE COMPLETE\n");
        }
        else if(ret == SUCCESS)
        {
            if(memMgr == gMemUserMgr)
            {
                nLOG(LOG_DEBUG, 
                   "MEMORY OPERATION : %s, TYPE : %d, SIZE : %llu, PTR : %s\n",
                    sMemmgrTypeName[1], memType, size, ptr);
            }
            break;
        }
        else
        {
            return MEMMGR_ERR_DEBUG_ENQUEUE;
        }
    }

    return SUCCESS;
}

/**
 * Description: Memory 재할당 함수
 *
 * @param [in] type : Memory reallocation DATA TYPE
 * @param [in] size : Memory reallocation Size
 * @param [in] file : Memory operation (Reallocation) file name
 * @param [in] func : Memory operation (Reallocation) function name
 * @param [in] line : Memory operation (Reallocation) line number
 * @param [out] reMem : Memory to be reallocated
 *
 * @retval : 할당된 Memory 주소 값 : 성공,
 *           MEMMGR_ERR_REALLOC(-1) : 메모리 재할당 실패
 *           MEMMGR_ERR_DEBUG_ALLOC(-2) : Debug 구조체 메모리 할당 실패,
 *           MEMMGR_ERR_DEBUG_ENQUEUE(-3) : Debug 구조체 Enqueue 실패
 *
 * Example Code Usage
 * @code
 *  StringT msg = NULL;
 *  msg = NNMALLOC(POLICY_STD_ACL, 1024);
 *
 *  ...
 *
 *  msg = NNREALLOC(POLICY_STD_ACL, msg, 1024);
 * @endcode
 *
 */

void * _memRealloc
    (Uint32T type, void * reMem, SizeT size, 
    StringT file, StringT func, Uint32T line)
{
    if(type < MEM_SYS_START)
    {
        return memReallocate(gMemUserMgr, type, reMem, size, file, func, line);
    }
    else
    {
        return memReallocate(gMemSysMgr, type-MEM_SYS_START, reMem, size,
                            file, func, line);
    }
}

/*******************************************************************************
 * Function: <memReallocate>
 *
 * Description: <Memory Reallocation>
 *
 * Parameters: memMgr: Memory Management Service
 *	           type  : Memory reallocation DATA TYPE
 *             reMem : Memory to be reallocated
 *             size  : Memory reallocation Size
 *             file  : Memory operation (Reallocation) file name
 *             func  : Memory operation (Reallocation) function name
 *             line  : Memory operation (Reallocation) line number
 *
 * Returns: The Memory reallocated,
 *          -1 : MEMMGR_ERR_REALLOC,           
 *          -2 : MEMMGR_ERR_DEBUG_ALLOC,
 *          -3 : MEMMGR_ERR_DEBUG_ENQUEUE
*******************************************************************************/

static void * memReallocate
    (MemT * memMgr, Uint32T type, void * reMem, 
    SizeT size, StringT file, StringT func, Uint32T line)
{
    Int8T memPtr[MAX_PTR_SIZE];
    SizeT currentSize = malloc_usable_size(reMem);
    SizeT newSize, addSize;
    Int64T refer;
    Int32T ret;

    sprintf(memPtr, "%p", reMem);	

    reMem = (void *)realloc(reMem, size);
    if(reMem == NULL)
    {
        if(memMgr->debug_flag == MEMMGR_SET_DEBUG)
        {
            refer = memDebugPtrCheck (memMgr, type, memPtr, currentSize);
	
            if((ret = memReallocDebug
                     (memMgr, type, 0, refer, memPtr, file, func, line)) 
                    != SUCCESS)
            {
                return (void *)ret;
            }
        }

        return (void *)MEMMGR_ERR_REALLOC;
    }

    newSize = malloc_usable_size(reMem);
    addSize = newSize - currentSize;

    memMgr->totalAlloc += addSize;
    memMgr->currentAlloc += addSize;

    memMgr->typeAlloc[type].totalAlloc += addSize;
    memMgr->typeAlloc[type].currentAlloc += addSize;

    if(memMgr->debug_flag == MEMMGR_SET_DEBUG)
    {
        if(memMgr->typeAlloc[type].debug != NULL)
        {
            refer = memDebugPtrCheck (memMgr, type, memPtr, currentSize);
            sprintf(memPtr, "%p", reMem);

            if((ret = memReallocDebug 
                     (memMgr, type, newSize, refer, memPtr, file, func, line))
                    != SUCCESS)
            {
                return (void *)ret;
            }

        }
    }

    return reMem;
}


/*******************************************************************************
 * Function: <memRellocDebug>
 *
 * Description: <Memory Reallocation, Debug Mode>
 *
 * Parameters: memMgr  : Memory Management Service
 *             memtype : Memory reallocation DATA TYPE
 *             size    : Memory reallocation Size
 *	       refer   : Memory Management Service - reference step
 *             ptr     : Memory address
 *             file    : Memory operation (Reallocation) file name
 *             func    : Memory operation (Reallocation) function name
 *             line    : Memory operation (Reallocation) line number
 *
 * Returns: 0 : SUCCESS, 
 *         -2 : MEMMGR_ERR_DEBUG_ALLOC,
 *         -3 : MEMMGR_ERR_DEBUG_ENQUEUE
*******************************************************************************/

static Int8T memReallocDebug
    (MemT * memMgr, Uint32T memType, SizeT size, Int64T refer, 
    StringT ptr, StringT file, StringT func, Uint32T line)
{
    Int32T ret;
    MemDebugT * newDebugMem = (MemDebugT *)malloc(sizeof(MemDebugT));
    if(newDebugMem == NULL)
    {
        return MEMMGR_ERR_DEBUG_ALLOC;
    }

    newDebugMem->type = MEMMGR_REALLOC;
    strcpy(newDebugMem->ptr, ptr);
    strcpy(newDebugMem->file, file);
    strcpy(newDebugMem->func, func);
    newDebugMem->line = line;
    nnTimeGetCurrent(newDebugMem->time);
    newDebugMem->size = size;

    if(size == 0)
    {
        newDebugMem->check = MEMMGR_MEM_CHECK;
    }
    else
    {
        newDebugMem->check = MEMMGR_MEM_CONTINUE;
    }

    newDebugMem->refer = refer;

    while(1)
    {
        ret = cQueueEnqueue(memMgr->typeAlloc[memType].debug, 0,
                            sizeof(newDebugMem), newDebugMem);

        if(ret == CQUEUE_ERR_FULL)
        {
            _memShowAllUser(memType);
            cQueueDeleteAllNode(memMgr->typeAlloc[memType].debug);
        }
        else if(ret == SUCCESS)
        {
            if(memMgr == gMemUserMgr)
            {
                nLOG(LOG_DEBUG,
                   "MEMORY OPERATION : %s, TYPE : %d, SIZE : %llu, PTR : %s\n",
                   sMemmgrTypeName[2], memType, size, ptr);
            }

            break;
        }
        else
        {
            return MEMMGR_ERR_DEBUG_ENQUEUE;
        }
    }

    return SUCCESS;
}

/**
 * Description: Memory 해제 함수
 *
 * @param [in] type : Memory Free DATA TYPE
 * @param [in] freeMem : Memory to be free
 * @param [in] file : Memory operation (free) file name
 * @param [in] func : Memory operation (free) function name
 * @param [in] line : Memory operation (free) line number
 *
 * @retval : SUCCESS(0) : 성공,
 *           MEMMGR_ERR_DEBUG_ALLOC(-2) : Debug 구조체 메모리 할당 실패,
 *           MEMMGR_ERR_DEBUG_ENQUEUE(-3) : Debug 구조체 Enqueue 실패
 *
 * Example Code Usage
 * @code
 *  Int8T ret = 0;
 *  StringT msg = NULL;
 *  msg = NNMALLOC(POLICY_STD_ACL, 1024);
 *
 *  ...
 *
 *  ret = NNFREE(POLICY_STD_ACL, msg);
 * @endcode
 */

Int8T _memFree
    (Uint32T type, void * freeMem, StringT file, StringT func, Uint32T line)
{
    if(type < MEM_SYS_START)
    {
        return memFreeMem(gMemUserMgr, type, freeMem, file, func, line);
    }
    else
    {
        return memFreeMem(gMemSysMgr, type-MEM_SYS_START, freeMem,
                            file, func, line);
    }
}


/*******************************************************************************
 * Function: <memFreeMem>
 *
 * Description: <Memory Free>
 *
 * Parameters: memMgr: Memory Management Service
 *             type  : Memory Free DATA TYPE
 *             freeMem : Memory to be free
 *             file  : Memory operation (free) file name
 *             func  : Memory operation (free) function name
 *             line  : Memory operation (free) line number
 *
 * Returns: 0 : SUCCESS,
 *         -2 : MEMMGR_ERR_DEBUG_ALLOC,
 *         -3 : MEMMGR_ERR_DEBUG_ENQUEUE>
*******************************************************************************/

static Int8T memFreeMem
    (MemT * memMgr, Uint32T type, void * freeMem, 
    StringT file, StringT func, Uint32T line)
{

    SizeT freeSize = malloc_usable_size(freeMem);
    Int8T freePtr[MAX_PTR_SIZE];
    Int64T refer;
    Int8T ret;

    sprintf(freePtr, "%p", freeMem);
    free(freeMem);

    memMgr->totalFree += freeSize;
    memMgr->currentAlloc -= freeSize;

    memMgr->typeAlloc[type].totalFree += freeSize;
    memMgr->typeAlloc[type].currentAlloc -= freeSize;

    if(memMgr->debug_flag == MEMMGR_SET_DEBUG)
    {
        if(memMgr->typeAlloc[type].debug != NULL)
        {
            refer = memDebugPtrCheck (memMgr, type, freePtr, freeSize);
        
            if((ret = memFreeDebug
                    (memMgr, type, freeSize, refer, freePtr, file, func, line)) 
                   != SUCCESS)
            {
                return ret;
            }
        }
    }

    return SUCCESS;
}


/*******************************************************************************
 * Function: <memFreeDebug>
 *
 * Description: <Memory Free, Debug Mode>
 *
 * Parameters: memMgr  : Memory Management Service
 *             memtype : Memory Free DATA TYPE
 *             size    : Memory Free Size
 *             refer   : Memory Management Service - reference step
 *             ptr     : Memory address
 *             file    : Memory operation (Free) file name
 *             func    : Memory operation (Free) function name
 *             line    : Memory operation (Free) line number
 *
 * Returns: 0 : SUCCESS, 
 *         -2 : MEMMGR_ERR_DEBUG_ALLOC, 
 *         -3 : MEMMGR_ERR_DEBUG_ENQUEUE
*******************************************************************************/

static Int8T memFreeDebug
    (MemT * memMgr, Uint32T memType, SizeT size, Int64T refer, 
    StringT ptr, StringT file, StringT func, Uint32T line)
{
    Int32T ret;
    MemDebugT * newDebugMem = (MemDebugT *)malloc(sizeof(MemDebugT));

    if(newDebugMem == NULL)
    {
        return MEMMGR_ERR_DEBUG_ALLOC;
    }

    newDebugMem->type = MEMMGR_FREE;
    newDebugMem->size = size;
    strcpy(newDebugMem->ptr, ptr);
    strcpy(newDebugMem->file, file);
    strcpy(newDebugMem->func, func);
    newDebugMem->line = line;

    nnTimeGetCurrent(newDebugMem->time);
	
    newDebugMem->check = MEMMGR_MEM_COMPLETE;
    newDebugMem->refer = refer;

    while(1)
    {
        ret = cQueueEnqueue(memMgr->typeAlloc[memType].debug, 0,
                            sizeof(newDebugMem), newDebugMem);

        if(ret == CQUEUE_ERR_FULL)
        {
            _memShowAllUser(memType);
            cQueueDeleteAllNode(memMgr->typeAlloc[memType].debug);
        }
        else if(ret == SUCCESS)
        {
            if(memMgr == gMemUserMgr)
            {
                nLOG(LOG_DEBUG,
                    "MEMORY OPERATION : %s, TYPE : %d, SIZE : %llu, PTR : %s\n",
                    sMemmgrTypeName[3], memType, size, ptr);
            }
            break;
        }
        else
        {
            return MEMMGR_ERR_DEBUG_ENQUEUE;
        }
    }

    return SUCCESS;
}


/*******************************************************************************
 * Function: <memDebugPtrCheck>
 *
 * Description: <Memory Menagement Service Debug Mode Addree Matching Check>
 *
 * Parameters: memMgr  : Memory Management Service
 *             memtype : Memory Free DATA TYPE
 *             ptr     : Memory address
 *             size    : Memory size
 *
 * Returns: Matchint Success : Matching Index 
 *          Matching None : SUCCESS(0)
*******************************************************************************/

static Int64T memDebugPtrCheck
    (MemT * memMgr, Uint32T memType, StringT ptr, SizeT size)
{
    MemDebugT * temp;
    Uint32T i;

    if(memMgr->typeAlloc[memType].debug->count == 0)
    {
        return SUCCESS;
    }

    for(i=0; i<=(memMgr->typeAlloc[memType].debug->count-1); i++)
    {
        temp = cQueueGetData(memMgr->typeAlloc[memType].debug, i);

        if(temp->check != MEMMGR_MEM_COMPLETE)
        {
            if((strcmp(temp->ptr, ptr) == 0) && (temp->size == size))
            {
                temp->check = MEMMGR_MEM_COMPLETE;
                temp->refer = memMgr->typeAlloc[memType].debug->count + 1;
                return i+1;
            }
        }
    }

    return SUCCESS;
}

/*******************************************************************************
 * Function: <memShowAllSystem>
 *
 * Description: <N2OS System Memory Menagement Service Show>
 *
 * Parameters: memMgr  : N2OS System Memory Management Service
 *             memtype : N2OS System Memory Type (or MEM_MAX_SYS_TYPE)
 *
 * Returns: NONE (Show Memory)
*******************************************************************************/

void memShowAllSystem(Uint32T index)
{
    memShowAll(gMemSysMgr, index-MEM_SYS_START);
}


/**
 * Description: 현재 Memory 사용현황을 확인하는 함수 (User 영역)
 *
 * @param [in] index : 사용현황을 볼 Memory Type (MEM_MAX_USER_CNT = 전체)
 *
 * @retval : None
 *
 * Example Code Usage
 * @code
 *  _memShowAllUser(MEM_MAX_USER_CNT);
 * @endcode
 *
 */

void _memShowAllUser(Uint32T index)
{
    if(index >= MEM_MAX_SYS_CNT)
    {
        return ;
    }
    memShowAll(gMemUserMgr, index);
}

/*******************************************************************************
 * Function: <memShowAll>
 *
 * Description: <Memory Menagement Service Display>
 *
 * Parameters: memMgr  : N2OS Memory Management Service
 *             memtype : N2OS Memory Type
 *                       (or MEM_MAX_USER_CNT or MEM_MAX_SYS_CNT)
 *
 * Returns: NONE (Display Memory Used)
*******************************************************************************/

static void memShowAll (MemT * memMgr, Uint32T index)
{
    MemDebugT * temp;
    Uint32T memCount = 0;
    Uint32T i, j;
    Uint32T fileSize = 0;
    Int32T fileNum = 0;

    /* 2013-10-17 */
    Int8T date[10] = {0,};
    Int8T fileName[30] = {0,};

    FILE *fpDebugWrite = NULL;

    if (memMgr->file_flag == MEMMGR_SET_FILE)
    {
        /* make filename -> memMgr_20131017 */
        nnTimeGetCurrentDate(date);

        while(1)
        {
            if (fileNum == 0)
            {
                sprintf(fileName, "%s_%s", memMgr->fileName, date);
            }
            else    /* over the fileSize, then make next number's file */
            {
                sprintf(fileName, "%s_%s_%d", memMgr->fileName, date, fileNum);
            }


            if (fpDebugWrite != NULL)
            {
                fclose(fpDebugWrite);
                fpDebugWrite = NULL;
            }

            fpDebugWrite = fopen(fileName, "a+");

            fseek(fpDebugWrite, 0, SEEK_END);
            fileSize = ftell(fpDebugWrite);

            /* Checking File Open */
            if (fpDebugWrite == NULL)
            {
                LOG("File Open Failed");
                return;
            }
                                                                               
            /* Checking File Size */
            if (fileSize > memMgr->logMaxSize)
            {
                ++fileNum;
                continue;
            }
            else
            {
                break;
            }
        }

        if (fpDebugWrite == NULL)
        {
	    printf("File Open Failed\n");
	    return;
        }
    }
    else
    {
        fpDebugWrite = stdout;
    }

    /* 2013-10-17 */
    if(memMgr == gMemSysMgr)
    {
        fprintf(fpDebugWrite, 
                "\n==== [N2OS Memory Management Service : Info] ====\n");
        fprintf(fpDebugWrite, "N2OS SYSTEM MEMORY INFORMATION\n");
        memCount = MEM_MAX_SYS_CNT;
    }
    else
    {
        fprintf(fpDebugWrite, 
                "\n==== [N2OS Memory Management Service : Info] ====\n");
        fprintf(fpDebugWrite, "USER MEMORY INFORMATION\n");
        memCount = MEM_MAX_USER_CNT;
    }

    if(memMgr->debug_flag == MEMMGR_SET_DEBUG)
    {
        fprintf(fpDebugWrite,"Process                 : %d \n",
                              memMgr->process);
        fprintf(fpDebugWrite,"Total Allocate Memory   : %d \n",
                              memMgr->totalAlloc);
        fprintf(fpDebugWrite,"Total Free Memory       : %d \n",
                              memMgr->totalFree);
        fprintf(fpDebugWrite,
                "Current Allocate Memory : %d \n", memMgr->currentAlloc);  
        fprintf(fpDebugWrite,
                "Debug Mode              : %d \n\n", memMgr->debug_flag);
  
        if(index == memCount)
        {
            for(i=0; i<memCount; i++)
            {
                fprintf(fpDebugWrite, "List by Type : TYPE %d ", i);

                if(memMgr == gMemSysMgr)
                {
                    fprintf(fpDebugWrite, "[ %s ]\n", sMemmgrSysTypeName[i]);
                }
                else
                {
                    fprintf(fpDebugWrite, "[ %s ]\n", sMemmgrUserTypeName[i]);
                }

                fprintf(fpDebugWrite, "Total Allocate Memory   : %d \n", 
                        memMgr->typeAlloc[i].totalAlloc);
                fprintf(fpDebugWrite, "Total Free Memory       : %d \n",
                        memMgr->typeAlloc[i].totalFree);
                fprintf(fpDebugWrite, "Current Allocate Memory : %d \n",
                        memMgr->typeAlloc[i].currentAlloc);

                if((memMgr->typeAlloc[i].debug != NULL)
                    &&
                   (memMgr->typeAlloc[i].debug->count > 0))
                {
                    fprintf(fpDebugWrite, 
                   "%-4s %-19s %-8s %-8s %-12s %-20s %-20s %-8s %-10s %-7s\n",
                   "[NO]", "[OPERATION TIME]", "[TYPE]", "[SIZE]", "[PTR]",
                   "[FILE]", "[FUNC]", "[LINE]", "[COMPLETE]", "[REFER]");

                    for(j=0; j<=((memMgr->typeAlloc[i].debug->count)-1); j++)
                    {
		                temp = cQueueGetData (memMgr->typeAlloc[i].debug, j);

                        fprintf(fpDebugWrite,
                    " %-4d %-19s %-8s %-8d %-12s %-20s %-20s %-8d %-10s %-7d\n",
	                j+1, temp->time, sMemmgrTypeName[temp->type], temp->size, 
                    temp->ptr, temp->file, temp->func, temp->line, 
                    sMemmgrCompleteName[temp->check], temp->refer);
		            }   
                }

                fprintf(fpDebugWrite, "\n");
            }
        }
        else if(index < memCount - 1)
        {
            fprintf(fpDebugWrite, "List by Type : TYPE %d", index);

            if(memMgr == gMemSysMgr)
            {
                fprintf(fpDebugWrite, "[ %s ]\n", sMemmgrSysTypeName[index]);
            }
            else
            {
                fprintf(fpDebugWrite, "[ %s ]\n", sMemmgrUserTypeName[index]);
            }
 
            fprintf(fpDebugWrite, "Total Allocate Memory   : %d \n",
                    memMgr->typeAlloc[index].totalAlloc);
            fprintf(fpDebugWrite, "Total Free Memory       : %d \n",
                    memMgr->typeAlloc[index].totalFree);
            fprintf(fpDebugWrite, "Current Allocate Memory : %d \n",
                    memMgr->typeAlloc[index].currentAlloc);

            if((memMgr->typeAlloc[index].debug != NULL)
                &&
               (memMgr->typeAlloc[index].debug->count > 0))
            {
                fprintf(fpDebugWrite,
             "%-4s %-19s %-8s %-8s %-12s %-20s %-20s %-8s %-10s %-7s\n",
             "[NO]", "[OPERATION TIME]", "[TYPE]", "[SIZE]", "[PTR]", "[FILE]",
             "[FUNC]", "[LINE]", "[COMPLETE]", "[REFER]");


                for(j=0; j<=((memMgr->typeAlloc[index].debug->count)-1); j++)
                {
	                temp = cQueueGetData(memMgr->typeAlloc[index].debug, j);
		
                    fprintf(fpDebugWrite, 
                    " %-4d %-19s %-8s %-8d %-12s %-20s %-20s %-8d %-10s %-7d\n",
                    j+1, temp->time, sMemmgrTypeName[temp->type], temp->size, 
                    temp->ptr, temp->file, temp->func, temp->line, 
                    sMemmgrCompleteName[temp->check], temp->refer);
                }
            }   
                fprintf(fpDebugWrite, "\n");
        }
        else
        {
            fprintf(fpDebugWrite, "Invaild Index\n");
        }
    }
    else
    {
        fprintf(fpDebugWrite, 
            "\t\t\t  [Total Allocate]\t    [Total Free]\t[Current Allocate]\n");

        fprintf(fpDebugWrite,"%14d\t\t\t%d\t\t\t%d\t\t\t%d\n",
                              memMgr->process, memMgr->totalAlloc, 
                              memMgr->totalFree, memMgr->currentAlloc);

        if(index == memCount)
        {
            for(i=0; i<memCount; i++)
            {
                if(memMgr == gMemSysMgr)
                {
                    fprintf(fpDebugWrite, "%14s\t\t\t", 
                                           sMemmgrSysTypeName[i]);
                }
                else
                {
                    fprintf(fpDebugWrite, "%14s\t\t\t",
                                           sMemmgrUserTypeName[i]);
                }

                fprintf(fpDebugWrite, "%d\t\t\t", 
                                      memMgr->typeAlloc[i].totalAlloc); 
                fprintf(fpDebugWrite, "%d\t\t\t",
                                      memMgr->typeAlloc[i].totalFree);
                fprintf(fpDebugWrite, "%d\n",
                                      memMgr->typeAlloc[i].currentAlloc);
            }
        }
        else if(index < memCount - 1)
        {
            if(memMgr == gMemSysMgr)
            {
                fprintf(fpDebugWrite, "%14s\t\t\t", sMemmgrSysTypeName[index]);
            }
            else
            {
                fprintf(fpDebugWrite, "%14s\t\t\t", sMemmgrUserTypeName[index]);
            }

            fprintf(fpDebugWrite, "%d\t\t\t",
                                  memMgr->typeAlloc[index].totalAlloc);
            fprintf(fpDebugWrite, "%d\t\t\t",
                                  memMgr->typeAlloc[index].totalFree);
            fprintf(fpDebugWrite, "%d\n",
                                  memMgr->typeAlloc[index].currentAlloc);
        }
        else
        {
            fprintf(fpDebugWrite, "Invaild Index\n");
        }


    }
  
    fprintf(fpDebugWrite, "================================================\n");

    /* 2013-10-17 */
    if (memMgr->file_flag == MEMMGR_SET_FILE)
    {
        fclose(fpDebugWrite);
    }
}

#if 0
static void memShowCheckList (MemT * memMgr, Uint32T index, Int8T type)
{
    MemDebugT * temp;
    Uint32T i = 0, j = 0, count = 0;

    /* 2013-10-17 */
    Int8T date[10] = {0,};
    Int8T fileName[30] = {0,};
    Uint32T fileSize = 0;
    Int32T fileNum = 0;

    FILE *fpDebugWrite = NULL;

    if (memMgr->file_flag == MEMMGR_SET_FILE)
    {
        /* make filename -> memMgr_20131017 */
        nnTimeGetCurrentDate(date);

        while(1)
        {
            if (fileNum == 0)
            {
                sprintf(fileName, "%s_%s", memMgr->fileName, date);
            }
            else    /* over the fileSize, then make next number's file */
            {
                sprintf(fileName, "%s_%s_%d", memMgr->fileName, date, fileNum);
            }

            if (fpDebugWrite != NULL)
            {
                fclose(fpDebugWrite);
                fpDebugWrite = NULL;
            }

            fpDebugWrite = fopen(fileName, "a+");

            fseek(fpDebugWrite, 0, SEEK_END);
            fileSize = ftell(fpDebugWrite);

            /* Checking File Open */
            if (fpDebugWrite == NULL)
            {
                LOG("File Open Failed");
                return;
            }

            /* Checking File Size */
            if (fileSize > memMgr->logMaxSize)
            {
                ++fileNum;
                continue;
            }
            else
            {
                break;
            }
        }

        if (fpDebugWrite == NULL)
        {
            printf("File Open Failed\n");
            return;
        }
    }
    else
    {
        fpDebugWrite = stdout;
    }

    /* 2013-10-17 */
    fprintf(fpDebugWrite, 
        "\n==== [N2OS Memory Management Service : Info] ====\n");
    fprintf(fpDebugWrite, "N2OS SYSTEM MEMORY CHECK LIST\n");

    if(index == MEM_MAX_USER_TYPE)
    {
        for(i=0; i<MEM_MAX_USER_TYPE; i++)
        {
	    fprintf(fpDebugWrite, "List by Type : TYPE %d \n", i);

            if((memMgr->debug_flag == MEMMGR_SET_DEBUG)
                &&
               (memMgr->typeAlloc[i].debug->count > 0))
            {
                fprintf(fpDebugWrite, 
                "%-4s %-19s %-8s %-8s %-12s %-20s %-20s %-8s %-10s %-7s\n",
                "[NO]", "[OPERATION TIME]", "[TYPE]", "[SIZE]", "[PTR]",
                "[FILE]", "[FUNC]", "[LINE]", "[COMPLETE]", "[REFER]");

                for(j=0; j<=((memMgr->typeAlloc[i].debug->count)-1); j++)
                {
                    temp = cQueueGetData(memMgr->typeAlloc[i].debug, j);
				
                    if(temp->check == type)
                    {
                        count++;
                        fprintf(fpDebugWrite,
                    " %-4d %-19s %-8s %-8d %-12s %-20s %-20s %-8d %-10s %-7d\n",
                        j+1, temp->time, sMemmgrTypeName[temp->type], 
                        temp->size, temp->ptr, temp->file, temp->func,
                        temp->line, sMemmgrCompleteName[temp->check],
                        temp->refer);
                    }
                }

                if(count == 0)
                {
                    fprintf(fpDebugWrite, "NO DATA\n");
                }
            }

            count = 0;
            fprintf(fpDebugWrite, "\n");
        }
    }
/*  2013-11-04( >= 0 always true)
    else if (index >= 0 && index < MEM_MAX_USER_TYPE - 1)
*/
    else if (index < MEM_MAX_USER_TYPE - 1)
    {
        fprintf(fpDebugWrite, "List by Type : TYPE %d \n", i);

        if ((memMgr->debug_flag == MEMMGR_SET_DEBUG)
             &&
           (memMgr->typeAlloc[index].debug->count > 0))
        {
            fprintf(fpDebugWrite,
            "%-4s %-19s %-8s %-8s %-12s %-20s %-20s %-8s %-10s %-7s\n",
            "[NO]", "[OPERATION TIME]", "[TYPE]", "[SIZE]", "[PTR]", "[FILE]",
            "[FUNC]", "[LINE]", "[COMPLETE]", "[REFER]");

            for(j=0; j<=((memMgr->typeAlloc[index].debug->count)-1); j++)
            {
                temp = cQueueGetData (memMgr->typeAlloc[i].debug, j);

                if(temp->check == type)
                {
	            count++;
                    fprintf(fpDebugWrite,
                    " %-4d %-19s %-8s %-8d %-12s %-20s %-20s %-8d %-10s %-7d\n",
                    j+1, temp->time, sMemmgrTypeName[temp->type], temp->size,
                    temp->ptr, temp->file, temp->func, temp->line,
                    sMemmgrCompleteName[temp->check], temp->refer);
                }
            }

            if(count == 0)
            {
 	        fprintf(fpDebugWrite, "NO DATA\n");
            }
        }

        count = 0;
        fprintf(fpDebugWrite, "\n");
    }
    else
    {
        fprintf(fpDebugWrite, "Invaild Index\n");
    }

    /* 2013-10-17 */
    if (memMgr->file_flag == MEMMGR_SET_FILE)
    {
        fclose(fpDebugWrite);
    }
}
#endif

/*******************************************************************************
 * Function: <cQueueInit>
 *
 * Description: <Initialize the circular queue>
 *
 * Parameters: maxCount : The maximun number of the circular queue node
 *             dupCheck : Have a duplicate of the circular queue node
 *
 * Returns: Circular queue is initalized,
 *          FAILURE(-1)
*******************************************************************************/

static CircularQueueT * cQueueInit (Uint64T maxCount, Uint8T dupCheck)
{
    CircularQueueT * newCqueue = NULL;
    newCqueue = (CircularQueueT *)malloc(sizeof(CircularQueueT));

    if (newCqueue == NULL)
    {
        return (CircularQueueT*)FAILURE;
    }

    newCqueue->head = NULL;
    newCqueue->tail = NULL;
    newCqueue->count = 0;
    newCqueue->dupCheck = dupCheck;

    if (maxCount <= 0)
    {
        newCqueue->maxCount = CQUEUE_DEFAULT_COUNT;
    }
    else
    {
        newCqueue->maxCount = maxCount;
    }

    return newCqueue;
}

/*******************************************************************************
 * Function: <cQueueFree>
 *
 * Description: <Delete the circular queue>
 *
 * Parameters: cqueue : Delete circualr queue
 *
 * Returns: none
*******************************************************************************/

static void cQueueFree (CircularQueueT * cqueue)
{
    if (cqueue == NULL)
    {
        return ;
    }

    cQueueDeleteAllNode(cqueue);
    free(cqueue);

    cqueue = NULL;
}

/*******************************************************************************
 * Function: <cQueueDeleteAllNode>
 *
 * Description: <Delete all of the node>
 *
 * Parameters: cqueue : circualr queue delete all node
 *
 * Returns: none
*******************************************************************************/

static void cQueueDeleteAllNode (CircularQueueT * cqueue)
{
    CircularQueueNodeT * tempNode = NULL;

    if (cqueue->count == 1)
    {
        free(cqueue->head->data);
        free(cqueue->head);
        --cqueue->count;

        cqueue->head = NULL;
        cqueue->tail = NULL;

        return ;
    }

    for (tempNode = cqueue->head; cqueue->count > 0; tempNode = cqueue->head)
    {
        cqueue->head = tempNode->next;
        free(tempNode->data);
        tempNode->data = NULL;

        free(tempNode);
        tempNode = NULL;

        --cqueue->count;
    }

    return ;
}

#if 0

/*******************************************************************************
 * Function: <cQueueDeleteNode>
 *
 * Description: <Delete the node in the index>
 *
 * Parameters: cqueue : circualr queue delete the node in the index
 *
 * Returns: SUCCESS(0) : delete node success
 *          CQUEUE_ERR_INCORRECT_INDEX(-2) : incorrect index
*******************************************************************************/

static Uint32T cQueueDeleteNode (CircularQueueT * cqueue, Uint32T index)
{
    CircularQueueNodeT * tempNode = NULL;
    Uint32T i;

/*  2013-11-04( >= 0 always true)
    if (index > (cqueue->count -1) || index < 0)
*/
    if (index > (cqueue->count -1))
    {
        return CQUEUE_ERR_INCORRECT_INDEX;
    }

    if(index == 0 && cqueue->count == 1)
    {
        free(cqueue->head->data);
        free(cqueue->head);

        cqueue->head = NULL;
        cqueue->tail = NULL;
        --cqueue->count;

        return SUCCESS;
    }
    else if (index == 0 && cqueue->count > 1)
    {
        tempNode = cqueue->head;

        cqueue->head = tempNode->next;

        cqueue->tail->next = cqueue->head;
        cqueue->head->prev = cqueue->tail;

        free(tempNode->data);
        free(tempNode);

        --cqueue->count;
        return SUCCESS;
    }
    else if (index == (cqueue->count - 1))
    {
        tempNode = cqueue->tail;

        cqueue->tail = tempNode->prev;

        cqueue->head->prev = cqueue->tail;
        cqueue->tail->next = cqueue->head;

        free(tempNode->data);
        free(tempNode);
        --cqueue->count;
        return SUCCESS;
    }
    else
    {
        tempNode = cqueue->head;

        for(i=0; i<=index; i++)
        {
            tempNode = tempNode->next;
        }

        tempNode->prev->next = tempNode->next;
        tempNode->next->prev = tempNode->prev;

        free(tempNode->data);
        free(tempNode);

        --cqueue->count;
        return SUCCESS;
    }
}

#endif

/*******************************************************************************
 * Function: <cQueueGetHead>
 *
 * Description: <circualr queue brings the head data>
 *
 * Parameters: cqueue : Import circualr queue head data
 *
 * Returns: circualr queue head data,
 *          CQUEUE_ERR_EMPTY(-6)
*******************************************************************************/

static void * cQueueGetHead (CircularQueueT * cqueue)
{
    if (cqueue->head == NULL)
    {
        return (void *) CQUEUE_ERR_EMPTY;
    }

    return cqueue->head->data;
}


/*******************************************************************************
 * Function: <cQueueGetTail>
 *
 * Description: <circualr queue brings the tail data>
 *
 * Parameters: cqueue : Import circualr queue tail data
 *
 * Returns: circualr queue tail data,
 *          CQUEUE_ERR_EMPTY(-6)
*******************************************************************************/

static void * cQueueGetTail (CircularQueueT * cqueue)
{
    if (cqueue->tail == NULL)
    {
        return (void *) CQUEUE_ERR_EMPTY;
    }

    return cqueue->tail->data;
}

/*******************************************************************************
 * Function: <cQueueGetData>
 *
 * Description: <circualr queue brings data of the index>
 *
 * Parameters: cqueue : Import circualr queue data
 *             index  : Get the index
 *
 * Returns: circualr queue data of the index,
 *          CQUEUE_ERR_INCORRECT_INDEX(-2)
*******************************************************************************/

static void * cQueueGetData (CircularQueueT * cqueue, Uint32T index)
{
    CircularQueueNodeT * tempNode = NULL;
    Uint32T i;

/*  2013-11-04( >= 0 always true)
    if (index > (cqueue->count - 1) || index < 0)
*/
    if (index > (cqueue->count - 1))
    {
        return (void *) CQUEUE_ERR_INCORRECT_INDEX;
    }

    if (index == 0)
    {
        return cQueueGetHead(cqueue);
    }
    else if (index == cqueue->count -1)
    {
        return cQueueGetTail(cqueue);
    }
    else
    {
        tempNode = cqueue->head;

        for(i = 0; i<index; i++)
        {
            tempNode = tempNode->next;
        }
        return tempNode->data;
    }
}

#if 0

/*******************************************************************************
 * Function: <cQueueCount>
 *
 * Description: <The Count of the circualr queue>
 *
 * Parameters: cqueue : Measure the count of circual queue
 *
 * Returns: circualr queue count
*******************************************************************************/

static Uint64T cQueueCount (CircularQueueT * cqueue)
{
    return cqueue->count;
}

#endif


/*******************************************************************************
 * Function: <cQueueEnqueue>
 *
 * Description: <Enqueue to the circualr queue>
 *
 * Parameters: cqueue : circual queue to enqueue
 *             data : QueueNode data
 *
 * Returns: SUCCESS(0),
 *          FAILURE(-1),
 *          CQUEUE_ERR_FULL(-2) 
*******************************************************************************/

static Int32T cQueueEnqueue
    (CircularQueueT * cqueue, Uint32T dataType, Uint64T dataSize, void * data)
{
    CircularQueueNodeT * tempNode = NULL;
    CircularQueueNodeT * newNode;

    if (cqueue->count == cqueue->maxCount)
    {
        return CQUEUE_ERR_FULL;
    }

    newNode = (CircularQueueNodeT *)malloc(sizeof(CircularQueueNodeT));
    if (newNode == NULL)
    {
        return FAILURE;
    }

    newNode->prev = NULL;
    newNode->next = NULL;
    newNode->data = data;
    newNode->dataType = dataType;
    newNode->dataSize = dataSize;

    /* No Node */
    if (cqueue->count == 0)
    {
        cqueue->head = newNode;
        cqueue->tail = newNode;

        newNode->prev = newNode;
        newNode->next = newNode;
    }
    else
    {
        if(cqueue->dupCheck == CQUEUE_NOT_DUPLICATED)
        {
            for(tempNode = cqueue->head; 
                tempNode != cqueue->tail; 
                tempNode = tempNode->next)
            {
                if(tempNode->data == data)
                {
                    free(newNode);
                    return CQUEUE_ERR_DUP;
                }
            }

            if(tempNode->data == data)
            {
                free(newNode);
                return CQUEUE_ERR_DUP;
            }
        }

        cqueue->tail->next = newNode;

        newNode->prev = cqueue->tail;
        newNode->next = cqueue->head;

        cqueue->tail = newNode;
        cqueue->head->prev = cqueue->tail;
    }

    ++cqueue->count;

    return SUCCESS;
}

#if 0

/*******************************************************************************
 * Function: <cQueueDequeue>
 *
 * Description: <Dequeue to the circualr queue>
 *
 * Parameters: cqueue : circual queue to dequeue
 *
 * Returns: Dequeue the data,
 *          CQUEUE_ERR_EMPTY(-6)
*******************************************************************************/

static void * cQueueDequeue (CircularQueueT * cqueue)
{
    CircularQueueNodeT * tempNode = NULL;
    void * data = NULL;

    if (cqueue->count == 0)
    {
        return (void *)CQUEUE_ERR_EMPTY;
    }
    else if (cqueue->count == 1)
    {
        cqueue->head->prev = NULL;
        cqueue->head->next = NULL;

        data = cqueue->head->data;

        free(cqueue->head);

        cqueue->head = NULL;
        cqueue->tail = NULL;
    }
    else
    {
        tempNode = cqueue->head;

        cqueue->head = tempNode->next;
        cqueue->tail->next = tempNode->next;

        tempNode->next->prev = cqueue->tail;

        tempNode->prev = NULL;
        tempNode->next = NULL;

        data = tempNode->data;

        free(tempNode);
    }

    --cqueue->count;

    return data;
}

#endif
