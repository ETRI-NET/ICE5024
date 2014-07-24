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
 * @brief : Log Service
 *  - Block Name : Library
 *  - Process Name : rLibraryibmgr
 *  - Creator : Changwoo Lee, JaeSu Han
 *  - Initial Date : 2013/7/15
 */

/**
* @file : nnLog.c
*
* $Id: nnLog.c 1060 2014-03-26 02:10:30Z lcw1127 $
* $Author: lcw1127 $
* $Date: 2014-03-26 11:10:30 +0900 (Wed, 26 Mar 2014) $
* $Revision: 1060 $
* $LastChangedBy: lcw1127 $
**/

/*******************************************************************************
 *                               INCLUDE FILES
 ******************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

/* mkdir */
#include <sys/stat.h>
#include <sys/types.h>

/* Send Message */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "nnDefines.h"
#include "nnLog.h"
#include "nnMemmgr.h"
#include "nnTime.h"

/*******************************************************************************
 *                              GLOBAL VARIABLES
 ******************************************************************************/


/*******************************************************************************
 *                         GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 *                         LOCAL CONSTANTS/LITERALS/TYPES
 ******************************************************************************/
/* Log 정보 String */
StringT sLogPriorityName[] =
{
    "",             // LOG_EMERG = 0
    "",             // LOG_ALERT = 1
    "",             // LOG_CRIT = 2
    "LOG_ERR",      // LOG_ERR = 3
    "LOG_WARNING",  // LOG_WARNING = 4
    "LOG_NOTICE",   // LOG_NOTICE = 5
    "LOG_INFO",     // LOG_INFO = 6
    "LOG_DEBUG"     // LOG_DEBUG = 7
};

/* 프로세스 이름 String */
StringT sLogProcessName[] =
{
    "IPC",
    "PM",
    "PORTM",
    "RIBM",
    "POLICY",
    "COMM",
    "MRIBM",
    "LIBM",
    "CKPTM",
    "LACP",
    "MSTP",
    "GVRP",
    "IGMP",
    "RIP",
    "ISIS",
    "OSPF",
    "BGP",
    "RSVP",
    "LDP",
    "RIBTT",
    "DLUTT"
};

/*******************************************************************************
*                               LOCAL VARIABLES
*******************************************************************************/


/*******************************************************************************
 *                                   CODE
 ******************************************************************************/


/**
 * Description: Log Service 를 생성하고 초기화하는 함수
 *
 * @param [in] process : Log Service 를 사용할 Process 번호
 *
 * @retval : Failure(-1) : 실패,
 *           SUCCESS(0) : 성공
 *
 * @code
 *  Int8T ret = 0;
 *
 *  ret = nnLogInit(PROCESS_NUM);
 * @endcode
 *
 * @test
 *  process 를 Process 번호의 범위를 벗어난 값으로 입력
 */

Int8T _nnLogInit(Uint32T process)
{
    gLog = nnLogInitialize(process);

    if ((Int32T)gLog <= 0)
    {
        return FAILURE;
    }

    return SUCCESS;
}


/*******************************************************************************
 * Function: <nnLogInitialize>
 *
 * Description: <Initialize the Log>
 *
 * Parameters: process : Use Process Name
 *
 * Returns: LOG_ERR_ALLOC(-1) : log malloc failed
 *          Log is initalized
*******************************************************************************/

LogT* nnLogInitialize(Uint32T process)
{
    LogT *pN2Log;
    pN2Log = (LogT *)nMALLOC(LOG, sizeof(LogT));
    Int8T ret = 0;

    if (pN2Log == NULL)
    {
        LOG("NewLog's Malloc is Failed");
        return (LogT *)LOG_ERR_ALLOC;
    }

    pN2Log->process = process;
    pN2Log->flags = LOG_FILE;
    pN2Log->priority = LOG_DEBUG;
    pN2Log->fp = NULL;
    strcpy(pN2Log->fileName, LOG_DEFAULT_LOG_FILE_NAME);
    pN2Log->fpDebug = NULL;
    strcpy(pN2Log->fileNameDebug, LOG_DEFAULT_LOG_FILE_NAME_DEBUG);
    pN2Log->logMaxSize = 10;  /* 10Mbyte */

    ret = nnLogSetFilePrint(pN2Log, pN2Log->fileName, pN2Log->logMaxSize);

    if (ret != SUCCESS)
    {
        return (LogT *)FAILURE;
    }

    return pN2Log;
}


/**
 * Description: Log Service 를 삭제하고 종료하는 함수
 *
 * @retval : none
 *
 * @code
 *  nnLogClose();
 * @endcode
 *
 * @test
 *  Log 를 초기화 하지 않은 상태에서 수행
 */

void _nnLogClose(void)
{
    nnLogEnd(gLog);

    gLog = NULL;
}


/*******************************************************************************
 * Function: <nnLogEnd>
 *
 * Description: <Free the Log>
 *
 * Parameters: pN2Log : log structure
 *
 * Returns: none
*******************************************************************************/

void nnLogEnd(LogT *pN2Log){
    if (pN2Log == NULL)
    {
        LOG("Log Must Not NULL");
        return;
    }

    /* 프로세스 로그 파일 닫기 */
    if (pN2Log->fp != NULL)
    {
        fclose(pN2Log->fp);
        pN2Log->fp = NULL;
    }

    /* 라이브러리 로그 파일 닫기 */
    if (pN2Log->fpDebug != NULL)
    {
        fclose(pN2Log->fpDebug);
        pN2Log->fpDebug = NULL;
    }

    nFREE(LOG, pN2Log);

    pN2Log = NULL;
}


/**
 * Description: Log 기록 방법을 설정하는 함수
 *
 * @param [in] flags : 설정할 Log 기록 방법
 *
 * @retval : none
 *
 * @code
 *  nnLogSetFlag(flags);
 * @endcode
 *
 * @test
 *  Log 를 초기화 하지 않은 상태에서 수행,
 *  flags 의 값을 정의된 범위를 벗어난 값으로 입력
 */

void _nnLogSetFlag(Uint8T flags)
{
    if (!(flags & LOG_ALL))
    {
        LOG("Wrong Flag\n");
        return;
    }

    nnLogSetFlagMode(gLog, flags);
}

/*******************************************************************************
 * Function: <nnLogSetFlagMode>
 *
 * Description: <Set the Logging(output location) Flag>
 *
 * Parameters: pN2Log : log structure
 *             flags : flag (Log Flag(Output Location))
 *
 * Returns: none
*******************************************************************************/

void nnLogSetFlagMode(LogT *pN2Log, Uint8T flags)
{
    Int8T openFileName[LOG_MAX_FILELEN] = {0, };
    Int8T openFileNameDebug[LOG_MAX_FILELEN] = {0, };

    if (pN2Log == NULL)
    {
        LOG("Log Must Not NULL");
        return;
    }
    else if (!(flags & LOG_ALL))
    {
        LOG("Wrong Flag\n");
        return;
    }

    pN2Log->flags |= flags;

    if (flags == LOG_FILE)
    {
        if (pN2Log->fp == NULL)
        {
            sprintf(openFileName, "%s.log", pN2Log->fileName);
            pN2Log->fp = fopen(openFileName, "a+");

            /* Checking File Open */
            if (pN2Log->fp == NULL)
            {
                LOG("File Open Failed");
                return;
            }
        }

        if (pN2Log->fpDebug == NULL)
        {
            sprintf(openFileNameDebug, "%s.log", pN2Log->fileNameDebug);
            pN2Log->fpDebug = fopen(openFileNameDebug, "a+");

            /* Checking File Open */
            if (pN2Log->fpDebug == NULL)
            {
                LOG("File Open Failed");
                return;
            }
        }
    }
}


/**
 * Description: Log 기록 방법을 해제하는 함수
 *
 * @param [in] flags : 해제할 Log 기록 방법
 *
 * @retval : none
 *
 * @code
 *  nnLogUnsetFlag(flags);
 * @endcode
 *
 * @test
 *  Log 를 초기화 하지 않은 상태에서 수행,
 */

void _nnLogUnsetFlag(Uint8T flags)
{
    nnLogUnsetFlagMode(gLog, flags);
}


/*******************************************************************************
 * Function: <nnLogUnsetFlagMode>
 *
 * Description: <UnSet the Logging(output location) Flag>
 *
 * Parameters: pN2Log : log structure
 *             flags : flag (Log Flag(Output Location))
 *
 * Returns: none
*******************************************************************************/

void nnLogUnsetFlagMode(LogT *pN2Log, Uint8T flags)
{
    if (pN2Log == NULL)
    {
        LOG("Log Must Not NULL");
        return;
    }

    pN2Log->flags &= ~flags;

    if (flags == LOG_FILE)
    {
        /* If File Opened, File Close */
        if (pN2Log->fp != NULL)
        {
            fclose(pN2Log->fp);
            pN2Log->fp = NULL;
        }

        /* If File Opened, File Close */
        if (pN2Log->fpDebug != NULL)
        {
            fclose(pN2Log->fpDebug);
            pN2Log->fpDebug = NULL;
        }
    }
}


/**
 * Description: Logging 시 사용할 파일 이름과 파일 크기를 설정하는 함수
 *
 * @param [in] fileName : Log 를 수행할 파일이름
 * @param [in] size : Log 파일의 최대 크기(Mbytes 단위)
 *
 * @retval : FAILURE(-1) : 설정을 하지 못했을 때,
 *           SUCCESS(0) : 성공
 *
 * @code
 *  nnLogSetFile(fileName, size);
 * @endcode
 *
 * @test
 *  Log 를 초기화 하지 않은 상태에서 수행,
 *  fileName 의 값을 NULL 로 입력,
 *  fileName 을 최대 길이보다 길게 입력,
 *  size 의 값을 0 으로 입력
 */

Int8T _nnLogSetFile(StringT fileName, Uint32T size)
{
    Int8T ret = 0;
    StringT tempFileName = NULL;

    tempFileName = fileName;

    if (tempFileName == NULL)
    {
        tempFileName = LOG_DEFAULT_LOG_FILE_NAME;
    }

    if (strlen(tempFileName) > (LOG_MAX_FILELEN - 1))
    {
        LOG("Too Long FileName");
	return FAILURE;
    }

    ret = nnLogSetFilePrint(gLog, tempFileName, size);

    if (ret != SUCCESS)
    {
        return FAILURE;
    }

    return SUCCESS;
}

/*******************************************************************************
 * Function: <nnLogSetFilePrint>
 *
 * Description: <Set the log file name and file size>
 *
 * Parameters: pN2Log : log structure
 *             fineName : filename
 *             size : max file size
 *
 * Returns: FAILURE(-1) : pN2Log, fileName is NULL, or over the max filename size
 *          SUCCESS(0)
*******************************************************************************/

Int8T nnLogSetFilePrint(LogT *pN2Log, StringT fileName, Uint32T size)
{
    Int8T openFileName[LOG_MAX_FILELEN] = {0, };
    Int8T openFileNameDebug[LOG_MAX_FILELEN] = {0, };
    StringT tempFileName = NULL;

    tempFileName = fileName;

    if (tempFileName == NULL)
    {
        tempFileName = LOG_DEFAULT_LOG_FILE_NAME;
    }

    if (pN2Log == NULL)
    {
        LOG("Log Must Not NULL");
        return FAILURE;
    }
    else if (strlen(tempFileName) > (LOG_MAX_FILELEN - 1))
    {
        LOG("Too Long FileName");
	return FAILURE;
    }

    /* If File Opened, File Close */
    if (pN2Log->fp != NULL)
    {
        fclose(pN2Log->fp);
        pN2Log->fp = NULL;
    }

    /* If File Opened, File Close */
    if (pN2Log->fpDebug != NULL)
    {
        fclose(pN2Log->fpDebug);
        pN2Log->fpDebug = NULL;
    }

    /* File Open */
    sprintf(openFileName, "%s.log", tempFileName);
    pN2Log->fp = fopen(openFileName, "a+");

    /* File Open */
    sprintf(openFileNameDebug, "%s.log", pN2Log->fileNameDebug);
    pN2Log->fpDebug = fopen(openFileNameDebug, "a+");

    /* Checking File Open */
    if (pN2Log->fp == NULL)
    {
        LOG("File Open Failed");
        return FAILURE;
    }

    /* Checking File Open */
    if (pN2Log->fpDebug == NULL)
    {
        LOG("File Open Failed");
        return FAILURE;
    }

    strcpy(pN2Log->fileName, tempFileName);
    pN2Log->logMaxSize = size * LOG_DEFAULT_LOG_FILE_SIZE;

    nnLogSetFlagMode(pN2Log, LOG_FILE);

    return SUCCESS;
}


/**
 * Description: Logging 을 수행할 최소 Log 정보를 설정하는 함수이며,
 *              설정된 Log 정보보다 큰 값의 Log 정보는 기록되지 않는다
 *
 * @param [in] priority : Log 를 수행할 정보)
 *
 * @code
 *  nnLogSetPriority(priority);
 * @endcode
 *
 * @test
 *  Log 를 초기화 하지 않은 상태에서 수행,
 *  priority 의 값을 정의된 범위를 벗어난 값으로 입력
 */

void _nnLogSetPriority(Uint8T priority)
{
    if (!(priority >= LOG_ERR && priority <= LOG_DEBUG))
    {
        LOG("Wrong Priority");
        return;
    }

    nnLogSetPriorityMode(gLog, priority);
}


/*******************************************************************************
 * Function: <nnLogSetPriorityMode>
 *
 * Description: <Set the Log Priority Flag>
 *
 * Parameters: pN2Log : log structure
 *             flags : flag (Priority Value)
 *
 * Returns: none
*******************************************************************************/

void nnLogSetPriorityMode(LogT *pN2Log, Uint8T priority)
{
    if (pN2Log == NULL)
    {
        LOG("Log Must Not NULL");
        return;
    }
    else if (!(priority >= LOG_ERR && priority <= LOG_DEBUG))
    {
        LOG("Wrong Priority");
        return;
    }

    pN2Log->priority = priority;
}


/*******************************************************************************
 * Function: <nnLogSetRemoteAddr>
 *
 * Description: <Set the log remote ip and port>
 *
 * Parameters: ip : Remote ip(IPV4)
 *             port : Remote Port
 *
 * Returns: none
*******************************************************************************/

void nnLogSetRemoteAddr(StringT ip, Uint16T port)
{
    if (ip == NULL)
    {
        LOG("Parameter Must Not NULL");
        return;
    }

    nnLogSetRemoteAddrMode(gLog, ip, port);
}


/*******************************************************************************
 * Function: <nnLogSetRemoteAddrMode>
 *
 * Description: <Set the log remote ip and port>
 *
 * Parameters: pN2Log : log structure
 *             ip : Remote ip(IPV4)
 *             port : Remote port
 *
 * Returns: none
*******************************************************************************/

void nnLogSetRemoteAddrMode(LogT *pN2Log, StringT ip, Uint16T port)
{
    if (pN2Log == NULL)
    {
        LOG("Log Must Not NULL");
        return;
    }
    if (ip == NULL)
    {
        LOG("Parameter Must Not NULL");
        return;
    }

    strcpy(pN2Log->ip, ip);
    pN2Log->port = port;
}


/**
 * Description: 설정된 Log 정보에 따라 Logging 을 수행하는 함수
 *
 * @param [in] file : Log 를 수행하는 File 의 이름
 * @param [in] line : Log 를 수행하는 Line
 * @param [in] priority : Log 정보
 * @param [in] format : 사용자 정의 문자열 형식
 * @param [in] ... : format 에 맞는 인자 값
 *
 * @retval : none
 *
 * @code
 *  NNLOG("%s", "Logging Test");
 * @endcode
 *
 * @test
 *  Log 를 초기화 하지 않은 상태에서 수행,
 *  fileName 이 NULL 인 상태에서 파일에 쓰기 수행,
 *  gLog 의 priority 값을 변경하면서 함수 수행,
 *  다수의 Logging 을 수행하여 파일 백업이 되는지 확인
 */

void _nnLogLogging(
    StringT file,
    Uint32T line,
    Uint32T priority,
    const StringT format,
    ...)
{
    va_list args;
    Int8T dateTime[22] = {0,}, date[10] = {0,};
    Int8T fileName[LOG_MAX_FILELEN + 4] = {0,};  /* MAX_FILENAME + ".log" */

    /* MAX_FILENAME + ".log" + "MAX_DIGITS" */
    Int8T tempFileName[LOG_MAX_FILELEN + 4 + LOG_MAX_FILE_DIGITS] = {0,};

    Int8T buf[LOG_MAX_BUFLEN] = {0,};    /* logging message buffer */
    Int32T strLen = 0;
    Int32T ret = 0;
    Int32T fileNum = 0;
    Uint32T fileSize = 0;

    /* NNLOG 를 사용하기 위한 임시 변수 */
    Int8T tempBuf[LOG_MAX_BUFLEN] = {0,};    /* logging message buffer */
    StringT tempFile = NULL;
    Uint32T tempLine = 0;
    Uint32T tempPriority = 0;

    if (gLog == NULL)
    {
        LOG("gLog Must Not NULL");
        return;
    }
    if (gLog->flags == LOG_NOLOG)
    {
        return;
    }

    /* Over than LogT's priority value, Not Logging */
    if(gLog->priority < priority)
    {
        return;
    }

    nnTimeGetCurrent(dateTime);
    strLen = sprintf(buf, "[%s] [%s][%s][%s][%d] ", dateTime,
                    sLogProcessName[gLog->process],
                    sLogPriorityName[priority], file, line);

    /* Variable Arguments */
    va_start(args, format);
    vsprintf(&buf[strLen], format, args);  /* logging message */

    /* NNLOG 를 수행하기 위한 임시 변수에 값 저장 */
//  vsprintf(tempBuf, format, args);  /* logging message */
    sprintf(tempBuf, &buf[strLen], sizeof(tempBuf));
    tempFile = file;
    tempLine = line;
    tempPriority = priority;

    /* Write to STDOUT */
    if(gLog->flags & LOG_STDOUT)
    {
        fprintf(stdout, "%s", buf);
        fflush(stdout);
    }

    /* Write to syslog */
    if(gLog->flags & LOG_SYSTEM)
    {
	syslog(priority, "%s", buf);

        ret = nnLogSendMessage(buf);

        if (ret != SUCCESS)
        {
            LOG("Send Message Failed");
            return;
        }
    }

    /* Write to File */
    if(gLog->flags & LOG_FILE)
    {
        /* Checking File Name */
        if (gLog->fileName == NULL)
        {
            strcpy(gLog->fileName, LOG_DEFAULT_LOG_FILE_NAME);
        }

        nnTimeGetCurrentDate(date);

        while(1)
        {
            if (fileNum == 0)
            {
                sprintf(fileName, "%s.log", gLog->fileName);
            }
            else    /* over the fileSize, then rename last number's file */
            {
                /* Change File Name to FILENAME.log.NUMBER */
                while(1)
                {
                    memset(tempFileName, 0, sizeof(tempFileName));
                    sprintf(tempFileName, LOG_FILE_FORMAT,
                            gLog->fileName, fileNum);

                    /* file check */
                    if (access(tempFileName, F_OK) == -1)
                    {
                        rename(fileName, tempFileName);

                        break;
                    }

                    ++fileNum;

                    /* backup log file */
                    if (fileNum > LOG_MAX_FILE)
                    {
                        nnLogBackup(gLog);
                        fileNum = 1;
                    }
                }

                if (gLog->fp != NULL)
                {
                    fclose(gLog->fp);
                    gLog->fp = NULL;
                }

                /* After Rename, Reopen Log File */
                gLog->fp = fopen(fileName, "a+");

                /* Checking File Open */
                if (gLog->fp == NULL)
                {
                    LOG("File Open Failed");
                    return;
                }
            }

            /* Checking fp is NULL */
            if (gLog->fp == NULL)
            {
                /* Open Log File */
                gLog->fp = fopen(fileName, "a+");

                /* Checking File Open */
                if (gLog->fp == NULL)
                {
                    LOG("File Open Failed");
                    return;
                }
            }

            /* Get File Size */
            fseek(gLog->fp, 0L, SEEK_END);

            fileSize = ftell(gLog->fp);

            /* Checking File Size */
            if (fileSize > gLog->logMaxSize)
            {
                ++fileNum;
                continue;
            }
            else
            {
                break;
            }
        }

        fprintf(gLog->fp, "%s", buf);
        fflush(gLog->fp);
    }
    va_end(args);

    _nnLogLoggingDebug(tempFile, tempLine, tempPriority, "%s", tempBuf);
}


/**
 * Description: 설정된 Log 정보에 따라 Logging 을 수행하는 함수
 *
 * @param [in] file : Log 를 수행하는 File 의 이름
 * @param [in] line : Log 를 수행하는 Line
 * @param [in] priority : Log 정보
 * @param [in] format : 사용자 정의 문자열 형식
 * @param [in] ... : format 에 맞는 인자 값
 *
 * @retval : none
 *
 * @code
 *  NNLOG("%s", "Logging Test");
 * @endcode
 *
 * @test
 *  Log 를 초기화 하지 않은 상태에서 수행,
 *  fileName 이 NULL 인 상태에서 파일에 쓰기 수행,
 *  gLog 의 priority 값을 변경하면서 함수 수행,
 *  다수의 Logging 을 수행하여 파일 백업이 되는지 확인
 */

/*******************************************************************************
 * Description: Library 의 Logging 을 수행하는 함수
 *
 * Parameters: file : log operation file name
 *             line : log operation line number
 *             priority : log priority
 *             format : string
 * Returns: none
*******************************************************************************/

void _nnLogLoggingDebug(
    StringT file,
    Uint32T line,
    Uint32T priority,
    const StringT format,
    ...)
{
    va_list args;
    Int8T dateTime[22] = {0,}, date[10] = {0,};
    Int8T fileName[LOG_MAX_FILELEN + 4] = {0,};  /* MAX_FILENAME + ".log" */

    /* MAX_FILENAME + ".log" + "MAX_DIGITS" */
    Int8T tempFileName[LOG_MAX_FILELEN + 4 + LOG_MAX_FILE_DIGITS] = {0,};

    Int8T buf[LOG_MAX_BUFLEN] = {0,};    /* logging message buffer */
    Int32T strLen = 0;
    Int32T fileNum = 0;
    Uint32T fileSize = 0;

    if (gLog == NULL)
    {
        LOG("gLog Must Not NULL");
        return;
    }
    if (gLog->flags == LOG_NOLOG)
    {
        return;
    }

    /* Over than LogT's priority value, Not Logging */
    if(gLog->priority < priority){
        return;
    }

    nnTimeGetCurrent(dateTime);
    strLen = sprintf(buf, "[%s] [%s][%s][%s][%d] ", dateTime,
                    sLogProcessName[gLog->process],
                    sLogPriorityName[priority], file, line);

    /* Variable Arguments */
    va_start(args, format);
    vsprintf(&buf[strLen], format, args);  /* logging message */

    /* Write to File */
    if(gLog->flags & LOG_FILE)
    {
        /* Checking File Name */
        if (gLog->fileNameDebug == NULL)
        {
            strcpy(gLog->fileNameDebug, LOG_DEFAULT_LOG_FILE_NAME_DEBUG);
        }

        nnTimeGetCurrentDate(date);

        while(1)
        {
            if (fileNum == 0)
            {
                sprintf(fileName, "%s.log", gLog->fileNameDebug);
            }
            else    /* over the fileSize, then rename last number's file */
            {
                /* Change File Name to FILENAME.log.NUMBER */
                while(1)
                {
                    memset(tempFileName, 0, sizeof(tempFileName));
                    sprintf(tempFileName, LOG_FILE_FORMAT,
                            gLog->fileNameDebug, fileNum);

                    /* file check */
                    if (access(tempFileName, F_OK) == -1)
                    {
                        rename(fileName, tempFileName);

                        break;
                    }

                    ++fileNum;

                    /* backup log file */
                    if (fileNum > LOG_MAX_FILE)
                    {
                        nnLogBackupDebug(gLog);
                        fileNum = 1;
                    }
                }

                if (gLog->fpDebug != NULL)
                {
                    fclose(gLog->fpDebug);
                    gLog->fpDebug = NULL;
                }

                /* After Rename, Reopen Log File */
                gLog->fpDebug = fopen(fileName, "a+");

                /* Checking File Open */
                if (gLog->fpDebug == NULL)
                {
                    LOG("Library Log File Open Failed");
                    return;
                }
            }

            /* Checking fp is NULL */
            if (gLog->fpDebug == NULL)
            {
                /* Open Log File */
                gLog->fpDebug = fopen(fileName, "a+");

                /* Checking File Open */
                if (gLog->fpDebug == NULL)
                {
                    LOG("File Open Failed");
                    return;
                }
            }

            /* Get File Size */
            fseek(gLog->fpDebug, 0L, SEEK_END);

            fileSize = ftell(gLog->fpDebug);

            /* Checking File Size */
            if (fileSize > gLog->logMaxSize)
            {
                ++fileNum;
                continue;
            }
            else
            {
                break;
            }
        }

        fprintf(gLog->fpDebug, "%s", buf);
        fflush(gLog->fpDebug);
    }
    va_end(args);
}


/*******************************************************************************
 * Function: <nnLogLoggingPrint>
 *
 * Description: <Do Logging>
 *
 * Parameters: pN2Log : log structure
 *             file : log operation file name
 *             line : log operation line number
 *             priority : log priority
 *             format : string
 * Returns: none
*******************************************************************************/

void nnLogLoggingPrint(
    LogT *pN2Log,
    StringT file,
    Uint32T line,
    Uint32T priority,
    const StringT format,
    ...)
{
    va_list args;
    Int8T dateTime[22] = {0,}, date[10] = {0,};

    Int8T fileName[LOG_MAX_FILELEN + 4] = {0,};  /* MAX_FILENAME + ".log" */

    /* MAX_FILENAME + ".log" + "MAX_DIGITS" */
    Int8T tempFileName[LOG_MAX_FILELEN + 4 + LOG_MAX_FILE_DIGITS] = {0,};

    Int8T buf[LOG_MAX_BUFLEN] = {0,};    /* logging message buffer */
    Int32T strLen = 0;
    Int32T ret = 0;
    Int32T fileNum = 0;
    Uint32T fileSize = 0;

    if (pN2Log == NULL)
    {
        LOG("pN2Log Must Not NULL");
        return;
    }
    else if (pN2Log->flags == LOG_NOLOG)
    {
        return;
    }

    /* Over than LogT's priority value, Not Logging */
    if(pN2Log->priority < priority){
        return;
    }

    nnTimeGetCurrent(dateTime);
    strLen = sprintf(buf, "[%s] [%s][%s][%s][%d] ", dateTime,
                    sLogProcessName[pN2Log->process],
                    sLogPriorityName[priority], file, line);

    /* Variable Arguments */
    va_start(args, format);
    vsprintf(&buf[strLen], format, args);  /* logging message */

    /* Write to STDOUT */
    if(pN2Log->flags & LOG_STDOUT)
    {
        fprintf(stdout, "%s", buf);
        fflush(stdout);
    }

    /* Write to syslog */
    if(pN2Log->flags & LOG_SYSTEM)
    {
	syslog(priority, "%s", buf);

        ret = nnLogSendMessage(buf);

        if (ret != SUCCESS)
        {
            LOG("Send Message Failed");
            return;
        }
    }

    /* Write to File */
    if(pN2Log->flags & LOG_FILE)
    {
        /* Checking File Name */
        if (pN2Log->fileName == NULL)
        {
            strcpy(pN2Log->fileName, LOG_DEFAULT_LOG_FILE_NAME);
        }

        nnTimeGetCurrentDate(date);

        while(1)
        {
            if (fileNum == 0)
            {
                sprintf(fileName, "%s.log", pN2Log->fileName);
            }
            else    /* over the fileSize, then rename last number's file */
            {
                /* Change File Name to FILENAME.log.NUMBER */
                while(1)
                {
                    memset(tempFileName, 0, sizeof(tempFileName));
                    sprintf(tempFileName,
                            LOG_FILE_FORMAT, pN2Log->fileName, fileNum);

                    /* file check */
                    if (access(tempFileName, F_OK) == -1)
                    {
                        rename(fileName, tempFileName);

                        break;
                    }

                    ++fileNum;

                    /* backup log file */
                    if (fileNum > LOG_MAX_FILE)
                    {
                        nnLogBackup(pN2Log);
                        fileNum = 1;
                    }
                }

                if (gLog->fp != NULL)
                {
                    fclose(gLog->fp);
                    gLog->fp = NULL;
                }

                /* After Rename, Reopen Log File */
                pN2Log->fp = fopen(fileName, "a+");

                /* Checking File Open */
                if (pN2Log->fp == NULL)
                {
                    LOG("File Open Failed");
                    return;
                }
            }

            /* Checking fp is NULL */
            if (pN2Log->fp == NULL)
            {
                /* Open Log File */
                pN2Log->fp = fopen(fileName, "a+");

                /* Checking File Open */
                if (pN2Log->fp == NULL)
                {
                    LOG("File Open Failed");
                    return;
                }
            }

            /* Get File Size */
            fseek(pN2Log->fp, 0L, SEEK_END);
            fileSize = ftell(pN2Log->fp);

            /* Checking File Size */
            if (fileSize > pN2Log->logMaxSize)
            {
                ++fileNum;
                continue;
            }
            else
            {
                break;
            }
        }

        fprintf(pN2Log->fp, "%s", buf);
        fflush(pN2Log->fp);
    }

    va_end(args);
}


/*******************************************************************************
 * Function: <nnLogSendMessage>
 *
 * Description: <Send the log message to remote log server>
 *
 * Parameters: msg
 *             port : Remote Port
 *
 * Returns: SUCCESS(0) : Message send sucess
 *          FAILURE(-1) : Operation Failed
*******************************************************************************/

Int8T nnLogSendMessage(StringT msg)
{
    Int32T sock = 0, servLen = 0;
    Int32T sendSize = 0;
    struct sockaddr_in servAddr;

    if (gLog->ip == NULL)
    {
        return SUCCESS;
    }
    if (msg == NULL)
    {
        LOG("Parameter Must Not NULL");
        return FAILURE;
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        LOG("Socket Open Failed");
        return FAILURE;
    }

    /* Server Address */
    servLen = sizeof(struct sockaddr_in);
    memset(&servAddr, 0, servLen);
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(gLog->ip);
    servAddr.sin_port = htons(gLog->port);

    sendSize = sendto(sock, msg, strlen(msg),
               0, (struct sockaddr *)&servAddr, servLen);

    if (sendSize < 0)
    {
        LOG("Send Message Failed");
        return FAILURE;
    }

    close(sock);

    return SUCCESS;
}


/*******************************************************************************
 * Function: <nnLogBackup>
 *
 * Description: <When log file's count is over the LOG_MAX_FILE(100)
                 move the log file to backup directory>
 *
 * Parameters: none
 *
 * Returns: SUCCESS(0) : Backup Complete
 *          FAILURE(-1) : Backup Failed
*******************************************************************************/

Int8T nnLogBackup(LogT *pN2Log)
{
    Int8T checkPath[LOG_MAX_FILELEN] = {0,};        /* n2osBak/nowdate */
    Int8T renamePath[LOG_MAX_FILELEN * 2] = {0,};   /* checkPath/filename */
    /* MAX_FILENAME + ".log" + "MAX_DIGITS" */
    Int8T tempFileName[LOG_MAX_FILELEN + 4 + LOG_MAX_FILE_DIGITS] = {0,};


    Int8T date[10] = {0,};
    Int32T dirNum = 0;
    Int32T fileNum = 1;

    /* check n2os directory */
    if (access(LOG_DEFAULT_LOG_BACKUP_DIRECTORY, F_OK) == -1)
    {
        mkdir(LOG_DEFAULT_LOG_BACKUP_DIRECTORY, 0777);
    }

    /* get current date */
    nnTimeGetCurrentDate(date);

    /* make directory */
    while(1)
    {
        if (dirNum == 0)
        {
            sprintf(checkPath, "%s/%s",
                    LOG_DEFAULT_LOG_BACKUP_DIRECTORY, date);
        }
        else
        {
            /* next number directory */
            while(1)
            {
                memset(checkPath, 0, sizeof(checkPath));
                sprintf(checkPath, "%s/%s_%d",
                        LOG_DEFAULT_LOG_BACKUP_DIRECTORY, date, dirNum);

                if (access(checkPath, F_OK) == -1)
                {
                    break;
                }

                ++dirNum;
            }
        }

        if (access(checkPath, F_OK) == -1)
        {
            mkdir(checkPath, 0777);
            break;
        }

        ++dirNum;
    }

    for(fileNum = 1; fileNum <= LOG_MAX_FILE; ++fileNum)
    {
        memset(tempFileName, 0, sizeof(tempFileName));
        sprintf(tempFileName, LOG_FILE_FORMAT, gLog->fileName, fileNum);

        sprintf(renamePath, LOG_RENAME_FORMAT,
                checkPath, gLog->fileName, fileNum);

        rename(tempFileName, renamePath);
    }

    return SUCCESS;
}

/*******************************************************************************
 * Function: <nnLogBackupDebug>
 *
 * Description: <When log file's count is over the LOG_MAX_FILE(100)
                 move the log file to backup directory>
 *
 * Parameters: none
 *
 * Returns: SUCCESS(0) : Backup Complete
 *          FAILURE(-1) : Backup Failed
*******************************************************************************/

Int8T nnLogBackupDebug(LogT *pN2Log)
{
    Int8T checkPath[LOG_MAX_FILELEN] = {0,};        /* n2osBak/nowdate */
    Int8T renamePath[LOG_MAX_FILELEN * 2] = {0,};   /* checkPath/filename */
    /* MAX_FILENAME + ".log" + "MAX_DIGITS" */
    Int8T tempFileName[LOG_MAX_FILELEN + 4 + LOG_MAX_FILE_DIGITS] = {0,};


    Int8T date[10] = {0,};
    Int32T dirNum = 0;
    Int32T fileNum = 1;

    /* check n2os directory */
    if (access(LOG_DEFAULT_LOG_BACKUP_DIRECTORY_DEBUG, F_OK) == -1)
    {
        mkdir(LOG_DEFAULT_LOG_BACKUP_DIRECTORY_DEBUG, 0777);
    }

    /* get current date */
    nnTimeGetCurrentDate(date);

    /* make directory */
    while(1)
    {
        if (dirNum == 0)
        {
            sprintf(checkPath, "%s/%s",
                    LOG_DEFAULT_LOG_BACKUP_DIRECTORY_DEBUG, date);
        }
        else
        {
            /* next number directory */
            while(1)
            {
                memset(checkPath, 0, sizeof(checkPath));
                sprintf(checkPath, "%s/%s_%d",
                        LOG_DEFAULT_LOG_BACKUP_DIRECTORY_DEBUG, date, dirNum);

                if (access(checkPath, F_OK) == -1)
                {
                    break;
                }

                ++dirNum;
            }
        }

        if (access(checkPath, F_OK) == -1)
        {
            mkdir(checkPath, 0777);
            break;
        }

        ++dirNum;
    }

    for(fileNum = 1; fileNum <= LOG_MAX_FILE; ++fileNum)
    {
        memset(tempFileName, 0, sizeof(tempFileName));
        sprintf(tempFileName, LOG_FILE_FORMAT, gLog->fileNameDebug, fileNum);

        sprintf(renamePath, LOG_RENAME_FORMAT,
                checkPath, gLog->fileNameDebug, fileNum);

        rename(tempFileName, renamePath);
    }

    return SUCCESS;
}
