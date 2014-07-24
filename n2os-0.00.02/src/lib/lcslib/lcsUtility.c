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
 * @brief :  Process 들을 관리하는 Manager
 * - Block Name : Process Manager
 * - Process Name : procmgr
 * - Creator : Changwoo Lee
 * - Initial Date : 2014/3/03
 */

/**
* @file : processmgrMain.c
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

#include "lcsUtility.h"

/*******************************************************************************
 *                              GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 *                         GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/

/*******************************************************************************
 *                         LOCAL CONSTANTS/LITERALS/TYPES
 ******************************************************************************/

/*******************************************************************************
*                               LOCAL VARIABLES
*******************************************************************************/

/*******************************************************************************
 *                                   CODE
 ******************************************************************************/

/* 파라미터로 받은 Binary 를 실행하는 함수 */
Int32T startProcess(StringT binName)
{
    Int32T pid = 0;
    Int32T procIndex = 0;

    StringT argv[2] = {0,};

    argv[0] = binName;
    argv[1] = (char *)0;

    switch((pid = fork()))
    {
        case -1:
            printf("Error fork()\n");
            return -1;

        case 0: // pid = 0 -> 자식 프로세스
            for(procIndex = 0; procIndex < 3; procIndex++)
            {
//                close(procIndex);
            }

            if(execvp(argv[0], argv) == -1)
            {
                printf("Error execvp : %d\n", pid);
                return -1;
            }
            break;
        default:
            break;
    }

    printf("PID : %d, argv : %s\n", pid, argv[0]);
    return pid;
}

/* 파라미터로 받은 Binary 를 종료하는 함수 */
void endProcess(StringT binName)
{
    Int32T ret = 0;
    Int8T sCmd[128] = {0,};
    StringT binary = NULL;

    StringT argv[2] = {0,};

    argv[0] = binName;
    argv[1] = (char *)0;

    if (((binary = rindex(argv[0], '/')) != NULL))
    {
        ++binary;
    }
    else
    {
        binary = argv[0];
    }

    sprintf(sCmd, "killall -s SIGTERM %s", binary);
    fprintf(stdout, "\n\n------> : %s\n\n", sCmd);
    ret = system(sCmd);

    if (ret == FAILURE)
    {
        fprintf(stderr, "Process [%s] Kill Failure\n", binary);
    }
    else
    {
        fprintf(stderr, "Process [%s] Kill Success\n", binary);
    }
}

/* 파라미터로 받은 pid 에 Signal 을 보내 종료하는 함수 */
void sendSignaltoPid(Int32T pid, Int32T sigNum)
{
    Int32T ret = 0;

    ret = kill(pid, sigNum);
    fprintf(stdout, "\n\n------> Pid[%d], Signal[%d]\n\n", pid, sigNum);

    if (ret == FAILURE)
    {
        fprintf(stderr, "Send signal to PID[%d] Failure\n", pid);
    }
    else
    {
        fprintf(stderr, "Send signal to PID[%d] Success\n", pid);
    }
}

/* 파라미터로 받은 값이 모두 숫자로 되어있는지 확인하는 함수 */
Int32T checkNumber(StringT str)
{
    Int32T numIndex;

    if (str == NULL)
    {
        return 0;
    }

    for (numIndex = 0; str[numIndex] != '\0'; ++numIndex)
    {
        if (!isdigit (str[numIndex]))
        {
            return 0;
        }
    }

    return 1;
}

/* 프로세스 이름으로 PID 를 얻는 함수 */
Int32T getPidfromName(StringT name)
{
    DIR *pDirp = NULL;
    FILE *pFp = NULL;
    struct dirent *pEntry = NULL;

    StringT binary = NULL;
    StringT argv[2] = {0,};

    Int8T sPath[1024] = {0,}, sReadBuf[1024] = {0,};
    Int32T ret = 0, pid = 0;

    argv[0] = name;
    argv[1] = (char *)0;

    if (((binary = rindex(argv[0], '/')) != NULL))
    {
        ++binary;
    }
    else
    {
        binary = argv[0];
    }

    pDirp = opendir ("/proc/");

    if (pDirp == NULL)
    {
        fprintf(stderr, "Get Pid From Name Failure\n");

        return FAILURE;
    }

    while ((pEntry = readdir(pDirp)) != NULL)
    {
        if (checkNumber(pEntry->d_name))
        {
            strcpy(sPath, "/proc/");
            strcat(sPath, pEntry->d_name);
            strcat(sPath, "/comm");

            pFp = fopen(sPath, "r");

            if (pFp != NULL)
            {
                ret = fscanf(pFp, "%s", sReadBuf);

                if (ret != EOF)
                {
                    /* 프로세스 이름 비교 */
                    if (strcmp(sReadBuf, binary) == 0)
                    {
                        pid = atoi(pEntry->d_name);

                        break;
                    }
                }

                fclose (pFp);
            }
        }
    }

    closedir (pDirp);

    return pid;
}

/* PID 로 프로세스 이름을 얻는 함수 */
Int8T getNamefromPid(StringT binName, Int32T pid)
{
    FILE *pFp = NULL;
    Int8T sPath[1024] = {0,};
    Int32T size = 0;

    sprintf(sPath, "/proc/%d/cmdline", pid);

    pFp = fopen(sPath, "r");

    if (pFp != NULL)
    {
        size = fread(binName, sizeof(char), 1024, pFp);

        if (binName > 0)
        {
            if ('\n' == binName[size - 1])
            {
                binName[size - 1] = '\0';
            }
        }

        fclose(pFp);
    }
    else
    {
        return FAILURE;
    }

    return SUCCESS;
}

/* 파라미터로 받은 값이 모두 숫자로 되어있는지 확인하는 함수 */
void getDatefromTimet(StringT dest, time_t time)
{
    struct tm *pTp;
    char sDay[][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    char sMon[][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    pTp = localtime(&time);

    sprintf(dest, "%s %s %d %.2d:%.2d:%.2d %d",
        sDay[pTp->tm_wday], sMon[pTp->tm_mon],
        pTp->tm_mday, pTp->tm_hour, pTp->tm_min, pTp->tm_sec, pTp->tm_year + 1900);
}
