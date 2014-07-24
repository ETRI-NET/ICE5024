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
 * @brief : N2OS Common Library - String 관련
 *  - Block Name : Library
 *  - Process Name : rLibraryibmgr
 *  - Creator : Changwoo Lee, JaeSu Han
 *  - Initial Date : 2013/7/15
 */

/**
* @file : nnStr.c
*
* $Id: nnStr.c 960 2014-03-07 10:18:26Z lcw1127 $
* $Author: lcw1127 $
* $Date: 2014-03-07 19:18:26 +0900 (Fri, 07 Mar 2014) $
* $Revision: 960 $
* $LastChangedBy: lcw1127 $
**/

/*******************************************************************************
 *                               INCLUDE FILES
 ******************************************************************************/
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <arpa/inet.h>

//#include "nnMemmgr.h"
//#include "nnLog.h"
#include "nnStr.h"
#include "nnDefines.h"
#include "nosLib.h"

/*******************************************************************************
 *                                  GLOBAL VARIABLES
 ******************************************************************************/


/*******************************************************************************
 *                              GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 *                             LOCAL CONSTANTS/LITERALS/TYPES
 ******************************************************************************/


/*******************************************************************************
*                                   LOCAL VARIABLES
*******************************************************************************/


/*******************************************************************************
 *                                       CODE
 ******************************************************************************/

/**
 * Description: 문자열을 AND형 구분자로 자르는 함수
 *
 * @param [in] delim : 문자열을 자를 구분자 
 * @param [out] remainStr : 구분자를 기준으로 자를 문자열. 남은 문자열 저장
 * @param [out] tokenStr : 구분자를 기준으로 자른 문자열 
 *
 * @retval : SUCCESS(0) : 성공,
 *           FAILURE(-1) : 실패
 */

Int8T nnStrGetAndToken(StringT remainStr, StringT tokenStr, StringT delim)
{
    StringT tempStr = NULL;

    if(strlen(remainStr) == 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "remainStr Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return FAILURE;
    }

    for(;; remainStr++)
    {
        if((tempStr = strstr(remainStr, delim)) != NULL)
        {
            strncpy(tokenStr, remainStr, (strlen(remainStr) - strlen(tempStr)));
            strncpy(remainStr, tempStr + (strlen(delim)), strlen(tempStr));
            remainStr[strlen(tempStr) + 1] = '\0';
            break;
        }
        else
        {
            memset(tokenStr, 0x00, strlen(tokenStr));

            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR, "String Token : Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FAILURE;
        }
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "String Token : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}

/**
 * Description: 문자열을 OR형 구분자로 자르는 함수
 *
 * @param [in] delim : 문자열을 자를 구분자
 * @param [out] remainStr : 구분자를 기준으로 자를 문자열. 남은 문자열 저장
 * @param [out] tokenStr : 구분자를 기준으로 자른 문자열
 *
 * @retval : SUCCESS(0) : 성공,
 *           FAILURE(-1) : 실패
 */

Int8T nnStrGetOrToken(StringT remainStr, StringT tokenStr, StringT delim)
{
    StringT tempStr = NULL;
    Int8T tempStr2[strlen(remainStr)];

    if(strlen(remainStr) == 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "remainStr Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        memset(tokenStr, 0x00, strlen(tokenStr));
        return FAILURE;
    }

    strncpy(tempStr2, remainStr, strlen(remainStr) + 1);
    tempStr = strtok(remainStr, delim);

    if(strcmp(remainStr, tempStr2) == 0)
    {
        memset(tokenStr, 0x00, strlen(tokenStr));

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "String Token : Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return FAILURE;
    }

    strncpy (tokenStr, tempStr, strlen(tempStr) + 1);
    strcpy (remainStr, tempStr2+strlen(remainStr));

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "String Token : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}

/**
 * Description: 주어진 문자열을 구분자를 기준으로 개수를 세는 함수
 *
 * @param [in] stringStr : 개수를 셀 문자열
 * @param [in] delim : 구분자
 *
 * @retval : 문자열안의 구분자의 개수 
 */

Int32T nnStrGetTokenNum (StringT stringStr, StringT delim)
{
    Int32T tokenNum = 0;
    StringT tempStr = NULL;

    for(;; stringStr++)
    {
        if((tempStr = strstr(stringStr, delim)) != NULL)
        {
            tokenNum++;
            stringStr = tempStr;
        }
        else
        {
            break;
        }
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Get Token Number : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return tokenNum;
}

/**
 * Description: 주어진 문자열을 복사하여 리턴하는 함수
 *
 * @param [in] memType : 문자열의 Memory Type
 * @param [out] stringStr : 복사할 String
 *
 * @retval : 복사된 문자열,
 *           NULL : 실패
 */

StringT nnStrDup (const StringT stringStr, Uint32T memType)
{
    Int32T len = strlen(stringStr);
    StringT newStr = NNMALLOC (memType, (len + 1));

    if (newStr == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "StrDup : Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return NULL;
    }
    else
    {
        newStr[len] = '\0';

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "StrDup : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (StringT) memcpy (newStr, stringStr, len);
    }
}

/**
 * Description: 주어진 문자열을 주어진 길이만큼, 복사하여 리턴하는 함수
 * 
 * @param [in] maxlen : 복사할 최대 길이
 * @param [in] memType : 문자열의 Memory Type
 * @param [out] stringStr : 복사할 String
 *
 * @retval : 복사된 문자열,
 *           NULL : 실패
 */

StringT nnStrnDup (const StringT stringStr, Int32T maxlen, Uint32T memType)
{
    Int32T len = strnlen (stringStr, maxlen);
    StringT newStr = NNMALLOC (memType, (len + 1));

    if (newStr == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "StrnDup : Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return NULL;
    }
    else
    {
        newStr[len] = '\0';

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "StrnDup : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (StringT) memcpy (newStr, stringStr, len);
    }
}

/**
 * Description: 주어진 문자열을 주어진 길이만큼, 복사하여 리턴하는 함수
 *
 * @param [in] srcStr : 복사할 문자열
 * @param [in] bufSize : 복사할 최대 길이
 * @param [out] desStr : 최종적으로 복사되는 문자열
 *
 * @retval : 복사된 문자열의 길이
 */

Int32T nnStrlCpy (StringT dstStr, const StringT srcStr, Int32T bufSize)
{
    Int32T srcLen = strlen(srcStr);
    Int32T ret = srcLen;
    
    if (bufSize > 0)
    {
        if(srcLen >= bufSize)
        {
            srcLen = bufSize -1;
        }
        memcpy (dstStr, srcStr, srcLen);
        dstStr[srcLen] = 0;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "StrlCpy : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return ret;
}

/**
 * Description: 주어진 문자열을 길이만큼, Append형식으로 복사하여 리턴하는 함수
 *
 * @param [in] srcStr : 복사할 문자열
 * @param [in] bufSize : 복사할 최대 길이
 * @param [out] desStr : 최종적으로 Append 형식으로 복사되는 문자열
 *
 * @retval : 복사된 문자열의 길이
 */

Int32T nnStrlCat (StringT dstStr, const StringT srcStr, Int32T bufSize)
{
    Int32T dstLen = strlen(dstStr);
    Int32T srcLen = strlen(srcStr);
    Int32T ret = dstLen + srcLen;
    
    if (dstLen < (bufSize - 1))
    {
        if(srcLen >= bufSize - dstLen)
        {
            srcLen = bufSize - dstLen - 1;
        }
    
        memcpy (dstStr + dstLen, srcStr, srcLen);
        dstStr[dstLen + srcLen] = 0;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "StrlCat : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return ret;
}

/**
 * Description: 가변 개수의 문자열을 길이만큼, Append형식으로 복사하여 리턴하는 함수
 *
 * @param [in] bufsize : 복사할 최대 길이
 * @param [in] strCnt : 복사할 문자열의 개수
 * @param [out] dstStr : 최종적으로 Append 형식으로 복사되는 문자열
 *
 * @retval : 복사된 문자열의 길이
 */

Int32T nnStrnCat (StringT dstStr, Int32T bufSize, Int32T strCnt, ...)
{
    va_list args;
    Int32T i, ret = 0;

    va_start (args, strCnt);

    for (i=0; i<strCnt; i++)
    {
        ret = nnStrlCat (dstStr, va_arg(args, StringT), bufSize);
    }

    va_end (args);

    return ret;
}

/**
 * Description: 사용자 정의 Format에 맞게 String을 만드는 함수
 *
 * @param [in] format : 사용자 정의 문자열 Format
 * @param [in] ... : Format에 맞는 인자 값
 * @param [out] stringStr : 사용자 정의 Format에 맞게 만들어진 String
 *
 * @retval : 만들어진 문자열 크기
 */

Int32T nnSprintf (StringT stringStr, const StringT format, ...)
{
    va_list args;
    Int32T ret;
    
    va_start (args, format);
    ret = vsprintf (stringStr, format, args);
    va_end (args);

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Snprintf : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return ret;
}

Int32T nnSprintfApp (StringT dstStr, Int32T bufSize, const StringT format, ...)
{
    va_list args;
    Int32T ret;
    Int8T strBuf[65535];

    nnStrlCat (dstStr, format, bufSize);
//    memcpy(strBuf, dstStr, bufSize);

    va_start (args, format);
    ret = vsprintf (strBuf, dstStr, args);
    va_end (args);

    memset(dstStr, 0x00, bufSize);
    memcpy(dstStr, strBuf, bufSize);

    return ret;
}

/**
 * Description: 파라미터로 받은 str1 과 str2 두 개의 문자열을 비교하여
 *              결과를 리턴하는 함수
 *
 * @param [in] str1 : 비교할 문자열
 * @param [in] str2 : 비교할 문자열
 *
 * @retval : -2 : 파라미터가 NULL 일 때,
 *           -1 : str1 문자열이 str2 문자열보다 클 때,
 *            1 : str2 문자열이 str1 문자열보다 클 때,
 *            0 : 두 문자열이 같을 때
 */

Int8T nnStrDupString(StringT str1, StringT str2)
{
    if (str1 != NULL && str2 != NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "String Compare : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return strcmp(str1, str2);
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return -2;
}


/**
 * Description: 파라미터로 받은 str 이
 *              IPv4(X.X.X.X) 형식의 문자인지 확인하는 함수>
 *
 * @param [in] str : 검사할 문자열
 *
 * @retval : FALSE(0) : IPv4 형식이 아닐 때,
 *           TRUE(1) : IPv4 형식일 때
 */

Int8T nnStrCheckIPv4Type(StringT str)
{
    Int8T strLen = 0;
    Int32T index = 0;
    Int32T num = 0;
    Int32T dotCount = 0;

    strLen = strlen(str);

    /* Check Length */
    if (!(strLen >= MIN_IPV4_LEN && strLen <= MAX_IPV4_LEN))
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Over or Less than IPv4 Length\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return FALSE;
    }

    /* Check IPV4 one by one */
    for (index = 0; index < strLen; ++index)
    {
        if ((isdigit(str[index])))
        {
            num = num * 10 + (str[index] - 48);
        }
        else if ((str[index] == '.'))
        {
            if (num > 255)
            {
                NNLOGDEBUG(LOG_DEBUG, "################################\n");
                NNLOGDEBUG(LOG_ERR, "Over the IPv4 Value(255)\n");
                NNLOGDEBUG(LOG_DEBUG, "################################\n");

                return FALSE;
            }
            num = 0;
            ++dotCount;

            if (dotCount > MAX_IPV4_DOT_COUNT)
            {
                NNLOGDEBUG(LOG_DEBUG, "################################\n");
                NNLOGDEBUG(LOG_ERR, "Over the IPv4 Dot Count(3)\n");
                NNLOGDEBUG(LOG_DEBUG, "################################\n");

                return FALSE;
            }
        }
        else
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR, "Not IPv4 Format\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FALSE;
        }
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Check IPv4 Type : True\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return TRUE;
}


/**
 * Description: 파라미터로 받은 str 이
 *              IPv4/Prefix(X.X.X.X/X) 형식의 문자인지 확인하는 함수
 *
 * @param [in] str : 검사할 문자열
 *
 * @retval : FALSE(0) : IPv4/Prefix 형식이 아닐 때,
 *           TRUE(1) : IPv4/Prefix 형식일 때
 */

Int8T nnStrCheckIPv4PrefixType(StringT str)
{
    Int8T strLen = 0;
    Int8T index = 0;

    StringT slash = NULL;
    Int8T addr[MAX_IPV4_LEN + 1] = {0,};
    Int8T addrLen = 0;
    Int8T prefixLen = 0;
    Int32T num = 0;

    strLen = strlen(str);

    if ((slash = strchr(str, '/')) != NULL)
    {
        addrLen = (slash - str);
        prefixLen = strLen - (slash - str) - 1;

        if (!(prefixLen >= MIN_IPV4_PREFIX_LEN &&
            prefixLen <= MAX_IPV4_PREFIX_LEN))
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR, "Over or Less than IPv4/Prefix Length\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FALSE;
        }

        strncpy(addr, str, addrLen);

        // Check Prefix
        for (index = 0; index < prefixLen; ++index)
        {
            if(!(isdigit(str[strLen - prefixLen + index])))
            {
                NNLOGDEBUG(LOG_DEBUG, "################################\n");
                NNLOGDEBUG(LOG_ERR, "Not IPv4/Prefix Value\n");
                NNLOGDEBUG(LOG_DEBUG, "################################\n");

                return FALSE;
            }
            else
            {
                num = num * 10 + (str[strLen - prefixLen + index] - 48);
            }
        }

        if (num > MAX_IPV4_PREFIX_NUM)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR, "Over the IPv4/Prefix Value(32)\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FALSE;
        }

        // Check IPV4
        if (nnStrCheckIPv4Type(addr) == FALSE)
        {
            return FALSE;
        }
    }
    else
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Not IPv4/Prefix Format\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return FALSE;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Check IPv4/Prefix Type : True\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return TRUE;
}


/**
 * Description: 파라미터로 받은 str 이
 *              IPv6(X:X:X:X:X:X:X:X) 형식의 문자인지 확인하는 함수
 *
 * @param [in] str : 검사할 문자열
 *
 * @retval : FALSE(0) : IPv6 형식이 아닐 때,
 *           TRUE(1) : IPv6 형식일 때
 */

Int8T nnStrCheckIPv6Type(StringT str)
{
    Int8T strLen = 0;
    Int32T index = 0;

    Int8T reduce = 0;
    Int8T numCount = 0;
    Int8T colonCount = 0;
    StringT tempStr = NULL;

    tempStr = str;

    if (!(strncmp("::", str, 2)))
    {
        if (strlen(tempStr) == 2)
        {
            return TRUE;
        }

        reduce = 1;

        tempStr += 2;
    }

    strLen = strlen(tempStr);

    if (strLen > MAX_IPV6_LEN - (reduce * 4))
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Over than IPv6 Length\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return FALSE;
    }

    for (index = 0; index < strLen; ++index)
    {
        if (!(isxdigit(tempStr[index])))
        {
            if (tempStr[index] != ':')
            {
                NNLOGDEBUG(LOG_DEBUG, "################################\n");
                NNLOGDEBUG(LOG_ERR, "Not IPv6 Format\n");
                NNLOGDEBUG(LOG_DEBUG, "################################\n");

                return FALSE;
            }
            else
            {
                if (numCount == 0)
                {
                    if (reduce == 1)
                    {
                        NNLOGDEBUG(LOG_DEBUG, "################################\n");
                        NNLOGDEBUG(LOG_ERR, "Not IPv6 Format\n");
                        NNLOGDEBUG(LOG_DEBUG, "################################\n");

                        return FALSE;
                    }
                    else
                    {
                        reduce = 1;
                    }
                }
                numCount = 0;
                ++colonCount;
            }
        }
        else
        {
            ++numCount;

            if (numCount > 4)
            {
                NNLOGDEBUG(LOG_DEBUG, "################################\n");
                NNLOGDEBUG(LOG_ERR, "Not IPv6 Format\n");
                NNLOGDEBUG(LOG_DEBUG, "################################\n");

                return FALSE;
            }
        }
    }

    if (colonCount > MAX_IPV6_COLON_COUNT)
    {
        if (strncmp("::", (tempStr + index - 2), 2))
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR, "Not IPv6 Format\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FALSE;
        }
    }
    else if (numCount == 0 &&
            !(tempStr[index - 1] == ':' &&
            tempStr[index - 2] == ':'))
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Not IPv6 Format\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return FALSE;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Check IPv6 Type : True\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return TRUE;
}


/**
 * Description: 파라미터로 받은 str 이
 *              IPv6/Prefix(X:X:X:X:X:X:X:X/X) 형식의 문자인지 확인하는 함수
 *
 * @param [in] str : 검사할 문자열
 *
 * @retval : FALSE(0) : IPv6/Prefix 형식이 아닐 때,
 *           TRUE(1) : IPv6/Prefix 형식일 때
 */

Int8T nnStrCheckIPv6PrefixType(StringT str)
{
    Int8T strLen = 0;
    Int8T index = 0;

    StringT slash = NULL;
    Int8T addr[MAX_IPV6_LEN + 1] = {0,};
    Int8T addrLen = 0;
    Int8T prefixLen = 0;
    Int32T num = 0;

    strLen = strlen(str);

    if ((slash = strchr(str, '/')) != NULL)
    {
        addrLen = (slash - str);
        prefixLen = strLen - (slash - str) - 1;

        if (!(prefixLen >= MIN_IPV6_PREFIX_LEN &&
            prefixLen <= MAX_IPV6_PREFIX_LEN))
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR, "Over or Less than IPv6/Prefix Length\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FALSE;
        }

        strncpy(addr, str, addrLen);

        // Check Prefix
        for (index = 0; index < prefixLen; ++index)
        {
            if(!(isdigit(str[strLen - prefixLen + index])))
            {
                NNLOGDEBUG(LOG_DEBUG, "################################\n");
                NNLOGDEBUG(LOG_ERR, "Not IPv6/Prefix Value\n");
                NNLOGDEBUG(LOG_DEBUG, "################################\n");

                return FALSE;
            }
            else
            {
                num = num * 10 + (str[strLen - prefixLen + index] - 48);
            }
        }

        if (num > MAX_IPV6_PREFIX_NUM)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR, "Over the IPv6/Prefix Value(128)\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FALSE;
        }

        // Check IPv6
        if (nnStrCheckIPv6Type(addr) == FALSE)
        {
            return FALSE;
        }
    }
    else
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Not IPv6/Prefix Format\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return FALSE;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Check IPv6/Prefix Type : True\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return TRUE;
}


/**
 * Description: 파라미터로 받은 str 이
 *              Mac Address(X:X:X:X:X:X) 형식의 문자인지 확인하는 함수
 *
 * @param [in] str : 검사할 문자열
 *
 * @retval : FALSE(0) : Mac Address 형식이 아닐 때,
 *           TRUE(1) : Mac Address 형식일 때
 */

Int8T nnStrCheckMacAddressType(StringT str)
{
    Int8T strLen = 0;
    Int32T index = 0;

    strLen = strlen(str);

    if (strLen == 17)
    {
        for(index = 0; index < 17; ++index)
        {
            if ((index % 3) == 2)
            {
                if (str[index] != ':')
                {
                    NNLOGDEBUG(LOG_DEBUG, "################################\n");
                    NNLOGDEBUG(LOG_ERR, "Not Mac Address Format\n");
                    NNLOGDEBUG(LOG_DEBUG, "################################\n");

                    return FALSE;
                }
            }
            else
            {
                if(!(isxdigit(str[index])))
                {
                    NNLOGDEBUG(LOG_DEBUG, "################################\n");
                    NNLOGDEBUG(LOG_ERR, "Not Mac Address Value\n");
                    NNLOGDEBUG(LOG_DEBUG, "################################\n");

                    return FALSE;
                }
            }
        }
    }
    else
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Not Mac Address Format\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return FALSE;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Check Mac Address Type : True\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return TRUE;
}


/**
 * Description: 파라미터로 받은 str 이
 *              Time (HH:MM:SS) 형식의 문자인지 확인하는 함수
 *
 * @param [in] str : 검사할 문자열
 *
 * @retval : FALSE(0) : Time 형식이 아닐 때,
 *           TRUE(1) : Time 형식일 때
 */

Int8T nnStrCheckTime(StringT str)
{
    Int8T strLen = 0;
    Int32T index = 0;
    strLen = strlen(str);

    if(strLen == 8)
    {
        for(index = 0; index < 8; ++index)
        {
            if ((index % 3) == 2)
            {
                if (str[index] != ':')
                {
                    return FALSE;
                }
            }
            else
            {
                if(!isdigit(str[index]))
                {
                    return FALSE;
                }
            }
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}


/**
 * Description: 파라미터로 받은 Char Type 의 숫자로 된 source 를
 *              Int Type 의 숫자로 변환하여 리턴하는 함수
 *
 * @param [in] source : Int Type 으로 변환할 Char Type 숫자
 *
 * @retval : 실패 : FAILURE(-1) : 파라미터가 NULL 일 때
 *           성공 : 변환된 숫자
 */

Int32T nnCnvStringtoInt(const StringT source)
{
    Int32T cnvIntNum = 0;

    if (source != NULL)
    {
        cnvIntNum = atoi(source);

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Convert String To Int : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return cnvIntNum;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return FAILURE;
}


/**
 * Description: 파라미터로 받은 Int Type 의 숫자로 된 source 를
 *              Char Type 의 숫자로 변환하는 함수
 *
 * @param [out] dest : 결과를 저장할 변수
 * @param [in] source : Char Type 으로 변환할 Int Type 숫자
 *
 * @retval : 실패 : FAILURE(-1) : 변환 실패 또는 파라미터가 NULL 일 때
 *           성공 : SUCCESS(0)
 */

Int8T nnCnvInttoString(StringT dest, Int32T source)
{
    Int32T ret = 0;

    if (dest != NULL)
    {
        ret = sprintf(dest, "%d", source);

        if (ret < 0)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_INFO, "Convert Int To String : Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FAILURE;
        }
        else
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_INFO, "Convert Int To String : Success\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return SUCCESS;
        }
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return FAILURE;
}


/**
 * Description: 파라미터로 받은 Char Type 의 문자를
 *              Int Type 의 Ascii 값으로 변환하여 리턴하는 함수
 *
 * @param [in] source : Int Type 의 Ascii 값으로 변환할 문자
 *
 * @retval : 변환된 숫자
 */

Int32T nnCnvChartoInt(Int8T source)
{
    Int32T cnvIntNum = 0;

    cnvIntNum = (Int32T)source;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Convert Char To Int : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return cnvIntNum;
}


/**
 * Description: 파라미터로 받은 Int Type 의 숫자로 된 source 를
 *              Char Type 의 Ascii 값 으로 변환하는 함수
 *
 * @param [out] dest : 결과를 저장할 변수
 * @param [in] source : Char Type 의 Ascii 값으로 변환할 숫자
 *
 * @retval : 실패 : FAILURE(-1) : 변환 실패일 때
 *           성공 : SUCCESS(0)
 */

Int8T nnCnvInttoChar(Int8T *dest, Int32T source)
{
    if (source >= 0 && source <= 127)
    {
        *dest = (char)source;

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Convert Int To Char : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return SUCCESS;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return FAILURE;
}

#if 0
/* Later */
/*******************************************************************************
 * Function: <nnCnvStringtoTime>
 *
 * Description: <Conversion struct tm type to time_t type>
 *
 * Parameters: dest : time_t, Variable to store the time
 *             source : struct tm, conversion to time_t
 *
 * Returns: FAILUE : Parameter is NULL
 *          SUCCESS : conversion struct tm to time_t
*******************************************************************************/

Int8T nnCnvStringtoTime(time_t *dest, struct tm *source)
{
    if (source == NULL)
    {
        LOG("Parameter Must Not NULL");
        return FAILURE;
    }

    *dest = mktime(source);

    if ((Int32T)*dest == -1)
    {
        LOG("Operation Faild");
        return FAILURE;
    }

    return SUCCESS;
}


/*******************************************************************************
 * Function: <nnCnvTimetoString>
 *
 * Description: <Conversion TimeT type to StringT type>
 *
 * Parameters: dest : String, Variable to store the time
 *             source : struct tm, conversion to String Type
 *
 * Returns: FAILURE : Parameter is NULL
 *          SUCCESS : conversion time_t to struct tm
*******************************************************************************/

struct tm *nnCnvTimetoString(time_t *source)
{
    struct tm *temp = NULL;

    if (source == NULL)
    {
        LOG("Parameter Must Not NULL");
        return (struct tm *)FAILURE;
    }

    temp = localtime(source);

    if (temp == NULL)
    {
        LOG("Operation Failed");
        return (struct tm *)FAILURE;
    }

    return temp;
}
#endif
