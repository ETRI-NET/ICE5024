#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include "nnTypes.h"
#include "nnDefines.h"
#include "nnTime.h"

#if !defined(_nnmemmgr_h)
#define _nnmemmgr_h

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
 * @brief : N2OS Memory Management Service
 *  - Block Name : Library
 *  - Process Name : 
 *  - Creator : Changwoo Lee, JaeSu Han
 *  - Initial Date : 2013/7/15
*/

/**
 * @file : nnMemmgr.h
 *
 * $Id: 
 * $Author:
 * $Date:
 * $Revision:
 * $LastChangedBy:
 **/

/*******************************************************************************
 *                        CONSTANTS / LITERALS / TYPES
 ******************************************************************************/

#define MAX_PTR_SIZE 16   /**< Address(ptr) 최대 크기 */
#define MAX_TIME_SIZE 32  /**< 시간 최대 크기 */
#define MAX_FILE_SIZE 256 /**< File 이름 최대 크기 */
#define MAX_FUNC_SIZE 256 /**< Function 이름 최대 크기 */

#define MEM_MAX_SYS_CNT  MEM_MAX_SYS_TYPE - MEM_SYS_START 
#define MEM_MAX_USER_CNT MEM_MAX_USER_TYPE - 0

#if MEM_MAX_USER_CNT < MEM_MAX_SYS_CNT
	#define MEM_MAX_CNT    MEM_MAX_SYS_CNT
#else
	#define MEM_MAX_CNT    MEM_MAX_USER_CNT
#endif


/** Memory Management Service Internal Queue */
typedef struct CircularQueueNodeT 
{
    struct CircularQueueNodeT * next; 
    struct CircularQueueNodeT * prev;
    Uint32T dataType;                 
    Uint64T dataSize;                 
    void *data;                       

} CircularQueueNodeT;

typedef struct CircularQueueT 
{
    struct CircularQueueNodeT * head; 
    struct CircularQueueNodeT * tail;

    Uint64T count;                    
    Uint64T maxCount;                 
    Uint8T dupCheck;

} CircularQueueT;


#define CQUEUE_DEFAULT_COUNT 256 

#define CQUEUE_ERR_INCORRECT_INDEX  -2 
#define CQUEUE_ERR_FULL             -2 
#define CQUEUE_ERR_CQUEUE_ALLOC     -3 
#define CQUEUE_ERR_CQUEUENODE_ALLOC -4 
#define CQUEUE_ERR_NULL             -5 
#define CQUEUE_ERR_EMPTY            -6 
#define CQUEUE_ERR_DUP              -7 
#define CQUEUE_ERR_ARGUMENT         -8

#define CQUEUE_DUPLICATED    0 
#define CQUEUE_NOT_DUPLICATED 1 


/** Memory Management Service Structure */

/** Memory Type 별 관리 구조체 */
typedef struct MemTypeT	  
{
    Uint32T totalAlloc;	   /**< 총 할당된 메모리 크기 */
    Uint32T totalFree;	   /**< 현재까지 해제된 메모리 크기 */
    Uint32T currentAlloc;  /**< 현재 할당 중인 메모리 크기 */
				
    CircularQueueT *debug; /**< Debug Mode시 관리 Queue */

} MemTypeT;

/** Process Memory 관리 구조체 */
typedef struct MemT		  
{
    Uint32T process;	  /**< 사용하는 Process */
    Uint32T totalAlloc;	  /**< 총 할당된 메모리 크기 */
    Uint32T totalFree;	  /**< 현재까지 해제된 메모리 크기 */
    Uint32T currentAlloc; /**< 현재 할당 중인 메모리 크기 */

    struct MemTypeT typeAlloc[MEM_MAX_CNT];	/**< Memory Type 별 관리 구조체 */
    Uint8T debug_flag;			            /**< Debug Mode flag */
    Uint8T file_flag;			            /**< File Write flag */
    StringT fileName;			            /**< File Name */
    Uint32T logMaxSize;			            /**< 출력 File 최대 크기 */

} MemT;

/** Debug Mode 관리 구조체 */
typedef struct MemDebugT
{			
    Uint8T type;		        /**< Memory Operation Type */
    Uint32T size;		        /**< Memory operation 크기 */
    Int8T ptr[MAX_PTR_SIZE];	/**< Memory operation address */
    Int8T file[MAX_FILE_SIZE]; 	/**< Memory operation file name */
    Int8T func[MAX_FUNC_SIZE]; 	/**< Memory operation function name */
    Uint32T line;		        /**< Memory operation line number */
    Int8T time[MAX_TIME_SIZE];	/**< Memory operation time */
    Uint8T check;		        /**< Memory 해제 여부 */
    Uint32T refer;		        /**< Memory operation reference Number */

} MemDebugT;


/** Memory Operation Type */
#define MEMMGR_MALLOC 1  /**< 할당 */
#define MEMMGR_REALLOC 2 /**< 재할당 */
#define MEMMGR_FREE 3    /**< 해제 */


/** Debug Mode Flag */
#define MEMMGR_SET_DEBUG 1        /**< Debug Mode Set Flag */
#define MEMMGR_UNSET_DEBUG 0      /**< Debug Mode Unset Flag */
#define MEMMGR_MAX_DEBUG_SIZE 256 /**< Debug Mode 관리 최대 크기 */


/** Memory Complete 여부 */
#define MEMMGR_MEM_CHECK 9        /**< 확인해봐야 하는 메모리 */
#define MEMMGR_MEM_CONTINUE 0     /**< 현재 사용 중인 메모리 */
#define MEMMGR_MEM_COMPLETE 1     /**< 사용이 완료된 메모리 */

#define MEMMGR_MEM_NOREFERENACE 0 /** 참조 할 메모리가 없음 */


/** File 출력 관린 Flag */
#define MEMMGR_SET_FILE 1   /**< Debug Mode -> File Write Flag */
#define MEMMGR_UNSET_FILE 0 /**< Debug Mode -> File Write Unset Flag */
#define MEMMGR_DEFAULT_LOG_FILE_NAME "memMgr" /**< Default 출력파일 이름 */
#define MEMMGR_DEFAULT_LOG_FILE_SIZE 1048576  /**< Default 출력파일 최대 크기 */


/*******************************************************************************
*                                GLOBALS VARIABLES
*******************************************************************************/

/** Globals Memory Manager */
MemT * gMemUserMgr; /**< User Memory Manager 변수 */
MemT * gMemSysMgr;  /**< System Memory Manger 변수 */


/*******************************************************************************
 *                           GLOBAL FUNCTIONS/PROTOTYPES
*******************************************************************************/

Int32T _memInit(Uint32T process);
void _memClose(void);
Int32T _memSetDebug(Uint32T memType);
Int32T _memUnsetDebug(Uint32T memType);
void _memSetFile(StringT fileName, Uint32T size);
void _memUnsetFile(void);

void * _memMalloc(Uint32T type, SizeT size, \
                 StringT file, StringT func, Uint32T line);
void * _memRealloc(Uint32T type, void * reMem, SizeT size, \
                  StringT file, StringT func, Uint32T line);
Int8T _memFree(Uint32T type, void * freeMem,\
		    StringT file, StringT func, Uint32T line);

void memShowAllSystem(Uint32T index);
void _memShowAllUser(Uint32T index);

#define nMALLOC(A, B) _memMalloc(A, B, __FILE__, (StringT)__func__, __LINE__)

#define nREALLOC(A, B, C) \
                  _memRealloc(A, B, C, __FILE__, (StringT)__func__, __LINE__)

#define nFREE(A, B) _memFree(A, B, __FILE__, (StringT)__func__, __LINE__)

#endif

