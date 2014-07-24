#include <stdio.h>
#include <syslog.h>

#include "nnTypes.h"

#if !defined(_log_h)
#define _log_h


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
 * @brief : N2OS Log Service
 *  - Block Name : Library
 *  - Process Name : 
 *  - Creator : Changwoo Lee, JaeSu Han
 *  - Initial Date : 2013/7/15
*/

/**
 * @file : nnLog.h
 *
 * $Id: nnLog.h 1060 2014-03-26 02:10:30Z lcw1127 $
 * $Author: lcw1127 $
 * $Date: 2014-03-26 11:10:30 +0900 (Wed, 26 Mar 2014) $
 * $Revision: 1060 $
 * $LastChangedBy: lcw1127 $
 **/

/*******************************************************************************
 *                        CONSTANTS / LITERALS / TYPES
 ******************************************************************************/
#define LOG_DEFAULT_LOG_FILE_NAME "comp" /**< 기본 Log 파일 이름*/
#define LOG_DEFAULT_LOG_FILE_NAME_DEBUG "n2os" /**< 기본 Library Log 파일 이름*/

#define LOG_DEFAULT_LOG_BACKUP_DIRECTORY "compBak" /**< Log
                                                        백업 디렉토리 이름 */
#define LOG_DEFAULT_LOG_BACKUP_DIRECTORY_DEBUG "n2osBak" /**< Library Log
                                                          백업 디렉토리 이름 */

#define LOG_DEFAULT_LOG_FILE_SIZE 1048576 /* 1M */ /**< 기본 Log 파일 크기 */
#define LOG_MAX_FILE 10  /**< 최대 Log 파일 개수 */
#define LOG_MAX_FILE_DIGITS 2 /**< 최대 Log 파일 개수의 자릿수 */
#define LOG_FILE_FORMAT "%s.log.%02d" /**< 설정된 파일 크기를 넘었을 때
                                           사용될 이름 형식 */
#define LOG_RENAME_FORMAT "%s/%s.log.%02d" /**< Backup 수행 시
                                                사용될 이름 형식 */

#define LOG_MAX_FILELEN   256  /**< 최대 파일 이름 길이 */
#define LOG_MAX_BUFLEN    1024 /**< Log 버퍼 크기 */

/* Log Error Code */
#define LOG_ERR_ALLOC    -1 /**< Log 의 메모리 할당 실패일 때 */

/* */
#define LOG_DEFAULT_REMOTE_PORT 514 /**< 원격 syslog 기록 시
                                         사용될 기본 포트번호 */

typedef struct LogT /**< Log 구조체 */
{
    Uint32T process;          /**< 프로세스에 할당된 번호 */
    Uint8T flags;             /**< Log 기록 방법(STDOUT, syslog, file) */
    Uint8T priority;          /**< Log 정보(LOG_ERR, LOG_WARNING, etc..) */
    FILE *fp;                 /**< Log 파일 포인터 */
    FILE *fpDebug;            /**< Library Log 파일 포인터 */

    Int8T fileName[LOG_MAX_FILELEN];         /**< Log 파일 이름 */
    Int8T fileNameDebug[LOG_MAX_FILELEN];    /**< Library Log 파일 이름 */
    Int8T ip[16];                            /**< 원격 syslog Server IP */

    Uint16T port;             /**< 원격 syslog Server Port */
    Uint32T logMaxSize;       /**< 최대 Log 파일 크기 */
} LogT;

/*******************************************************************************
*                                GLOBALS VARIABLES
*******************************************************************************/
/* Globals Log */
LogT *gLog; /**< 전역으로 사용되는 Log 변수 */

/*******************************************************************************
 *                           GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/

/* Use Global Log Structure */
Int8T _nnLogInit(Uint32T process);
void _nnLogClose(void);

void _nnLogSetFlag(Uint8T flags);
void _nnLogUnsetFlag(Uint8T flags);
Int8T _nnLogSetFile(StringT filename, Uint32T size);
void _nnLogSetPriority(Uint8T priority);
void nnLogSetRemoteAddr(StringT ip, Uint16T port);

void _nnLogLogging(
    StringT file,
    Uint32T line,
    Uint32T priority,
    const StringT format,
    ...);

void _nnLogLoggingDebug(
    StringT file,
    Uint32T line,
    Uint32T priority,
    const StringT format,
    ...);

/* Need Local Log Structure */
LogT *nnLogInitialize(Uint32T process);
void nnLogEnd(LogT *pN2Log);
void nnLogSetFlagMode(LogT *pN2Log, Uint8T flags);
void nnLogUnsetFlagMode(LogT *pN2Log, Uint8T flags);
Int8T nnLogSetFilePrint(LogT *pN2Log, StringT filename, Uint32T size);
void nnLogSetPriorityMode(LogT *pN2Log, Uint8T priority);
void nnLogSetRemoteAddrMode(LogT *pN2Log, StringT ip, Uint16T port);

void nnLogLoggingPrint(
    LogT *pN2Log,
    StringT file,
    Uint32T line,
    Uint32T priority,
    const StringT format,
    ...);

/* Temporary Send Log Message */
Int8T nnLogSendMessage(StringT msg);

/* Temporary Backup Function */
Int8T nnLogBackup(LogT *pN2Log);
Int8T nnLogBackupDebug(LogT *pN2Log);
/*
#define NNLOG(A, B, ...) nnLogLogging(__FILE__, __LINE__, A, B, ##__VA_ARGS__)
#define NNLOGDEBUG(A, B, ...) nnLogLoggingDebug(__FILE__, __LINE__, A, B, ##__VA_ARGS__)
*/
#define nLOG(A, B, ...) _nnLogLogging(__FILE__, __LINE__, A, B, ##__VA_ARGS__)
#define nLOGDEBUG(A, B, ...) _nnLogLoggingDebug(__FILE__, __LINE__, A, B, ##__VA_ARGS__)

#endif
