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
 * @brief : This file defines the prefix related definitions.
 *  - Block Name : RIB Manager
 *  - Process Name : ribmgr
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/12/10
 */

/**
 * @file : nnPrefix.c
 *
 * $Id: nnPrefix.c 1051 2014-03-21 08:17:41Z sckim007 $
 * $Author: sckim007 $
 * $Date: 2014-03-21 17:17:41 +0900 (Fri, 21 Mar 2014) $
 * $Revision: 1051 $
 * $LastChangedBy: sckim007 $
 */


/*******************************************************************************
 *                               INCLUDE FILES
 ******************************************************************************/
#include "nnDefines.h"
//#include "nnMemmgr.h"
//#include "nnLog.h"
#include "nnRibDefines.h"
#include "nnPrefix.h"
#include "nnStr.h"
#include "nosLib.h"

#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*******************************************************************************
 *                              GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 *                         GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/

/*******************************************************************************
 *                         LOCAL CONSTANTS/LITERALS/TYPES
 ******************************************************************************/
/** bit 에 따른 16진수 값 */
static const Uint8T gMaskbit[] = {0x00, 0x80, 0xc0, 0xe0, 0xf0,
                                        0xf8, 0xfc, 0xfe, 0xff};

/*******************************************************************************
*                               LOCAL VARIABLES
*******************************************************************************/


/*******************************************************************************
 *                                   CODE
 ******************************************************************************/

/**
 * Description: 문자열 형식의 IPv4 또는 IPv4/Prefix 주소를
 *              struct in_addr 형식으로 변환하는 함수
 *
 * @param [out] pDest : 결과를 저장할 변수
 * @param [in] strAddr : 변환할 문자열 형식의 IPv4 또는 IPv4/Prefix 주소
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  pDest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  struct in_addr testAddr = {0,};
 *  StringT ipv4 = "192.168.1.1";
 *  Int8T ret = 0;
 *
 *  ret = nnCnvStringtoAddr(&testAddr, ipv4);
 * @endcode
 *
 * @test 
 *  pDest 의 값을 NULL 로 입력,
 *  strAddr 의 값을 NULL 로 입력
 */

Int8T nnCnvStringtoAddr(struct in_addr *pDest, StringT strAddr)
{
    Int8T ret = 0;
    Int8T addr[MAX_IPV4_LEN + 1] = {0, };
    Int8T tempAddr[strlen(strAddr)];

    if (pDest == NULL || strAddr == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    memset(pDest, 0, sizeof(struct in_addr));

    /* Type Check IPV4 */
    ret = nnStrCheckIPv4Type(strAddr);

    if (ret == TRUE)
    {
        ret = inet_aton(strAddr, pDest);

        if (ret < 0)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR,
                  "Convert String To Address(struct in_addr) : Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FAILURE;
        }

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO,
              "Convert String To Address(struct in_addr) : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return SUCCESS;
    }

    /* Type check IPV4_PREFIX */
    ret = nnStrCheckIPv4PrefixType(strAddr);

    if (ret == TRUE)
    {
        memset(tempAddr, 0, sizeof(tempAddr));
        strcpy(tempAddr, strAddr);

        nnStrGetAndToken(tempAddr, addr, "/");

        ret = inet_aton(addr, pDest);

        if (ret < 0)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR,
                  "Convert String To Address(struct in_addr) : Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FAILURE;
        }

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO,
              "Convert String To Address(struct in_addr) : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return SUCCESS;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_ERR, "Convert String To Address(struct in_addr) : Failure\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return FAILURE;
}


/**
 * Description: struct in_addr 형식의 IPv4 주소를
 *              문자열 형식으로 변환하는 함수
 *
 * @param [out] dest : 결과를 저장할 변수
 * @param [in] pAddr : 변환할 struct in_addr 형식의 주소
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  dest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  Int8T addrToStr[20] = {0,};
 *  struct in_addr testAddr = {0,};
 *  Int8T ret = 0;
 *
 *  testAddr.s_addr = 0x0101A8C0; // 192.168.1.1
 *
 *  ret = nnCnvAddrtoString(addrToStr, &testAddr);
 * @endcode
 *
 * @test 
 *  dest 의 값을 NULL 로 입력,
 *  pAddr 의 값을 NULL 로 입력
 */

Int8T nnCnvAddrtoString(StringT dest, struct in_addr *pAddr)
{
    StringT strAddr = NULL;

    /* Parameter Check */
    if (pAddr == NULL || dest == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    strAddr = inet_ntoa(*pAddr);

    if (strAddr == 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Convert Address(struct in_addr) To String : Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return FAILURE;
    }

    strcpy(dest, strAddr);

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Convert Address(struct in_addr) To String : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: 문자열 형식의 IPv4 또는 IPv4/Prefix 주소를
 *              Prefix4 구조체에 저장하는 함수
 *
 * @param [out] pDest : 결과를 저장할 Prefix4 구조체
 * @param [in] strAddr : 저장할 문자열 형식의 IPv4 또는 IPv4/Prefix 주소
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  pDest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  Prefix4T testPrefix4 = {0,};
 *  StringT ipv4 = "192.168.1.1";
 *  Int8T ret = 0;
 *
 *  ret = nnCnvStringtoPrefix4(&testPrefix4, ipv4);
 * @endcode
 *
 * @test 
 *  pDest 의 값을 NULL 로 입력,
 *  strAddr 의 값을 NULL 로 입력
 */

Int8T nnCnvStringtoPrefix4(Prefix4T *pDest, StringT strAddr)
{
    Int8T ret = 0;
    Int8T addr[MAX_IPV4_LEN + 1] = {0, };
    Int8T tempAddr[strlen(strAddr)];

    if (pDest == NULL || strAddr == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    memset(pDest, 0, sizeof(Prefix4T));

    /* Type Check IPV4 */
    ret = nnStrCheckIPv4Type(strAddr);

    if (ret == TRUE)
    {
        ret = inet_aton(strAddr, &pDest->prefix);

        if (ret < 0)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR, "Convert String To Prefix4 : Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FAILURE;
        }

        pDest->family = AF_INET;

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Convert String To Prefix4 : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return SUCCESS;
    }

    /* Type check IPV4_PREFIX */
    ret = nnStrCheckIPv4PrefixType(strAddr);

    if (ret == TRUE)
    {
        memset(tempAddr, 0, sizeof(tempAddr));
        strcpy(tempAddr, strAddr);

        nnStrGetAndToken(tempAddr, addr, "/");

        ret = inet_aton(addr, &pDest->prefix);

        if (ret < 0)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR, "Convert String To Prefix4 : Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FAILURE;
        }

        pDest->family = AF_INET;
        pDest->prefixLen = atoi(tempAddr);

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Convert String To Prefix4 : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return SUCCESS;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_ERR, "Convert String To Prefix4 : Failure\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return FAILURE;
}


/**
 * Description: Prefix4 구조체에 저장된 IP 주소를
 *              문자열 형식으로 변환하는 함수
 *
 * @param [out] dest : 결과를 저장할 변수
 * @param [in] pPrefix4 : 저장된 IP 주소를 변환할 Prefix4 구조체
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  dest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  Int8T addrToStr[20] = {0,};
 *  Prefix4T testPrefix4 = {0,};
 *  Int8T ret = 0;
 *
 *  testPrefix4.family = AF_INET;
 *  testPrefix4.prefixLen = 24;
 *  testPrefix4.prefix.s_addr = 0x0101A8C0;
 *
 *  ret = nnCnvPrefix4toString(addrToStr, &testPrefix4);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 *  strAddr 의 값을 NULL 로 입력
 */

Int8T nnCnvPrefix4toString(StringT dest, Prefix4T *pPrefix4)
{
    StringT strAddr = NULL;

    /* Parameter Check */
    if (pPrefix4 == NULL || dest == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    strAddr = inet_ntoa(pPrefix4->prefix);

    if (strAddr == 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Convert Prefix4 To String: Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return FAILURE;
    }

    strcpy(dest, strAddr);

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Convert Prefix4 To String: Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: 문자열 형식의 IPv4, IPv4/Prefix, IPv6 또는 IPv6/Prefix 주소를
 *              Prefix 구조체에 저장하는 함수
 *
 * @param [out] pDest : 결과를 저장할 Prefix 구조체
 * @param [in] strAddr : 저장할 문자열 형식의 IPv4, IPv4/Prefix, IPv6 또는
 *                       IPv6/Prefix 주소
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  pDest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  PrefixT testPrefix = {0,};
 *  StringT ipv4 = "192.168.1.1";
 *  Int8T ret = 0;
 *
 *  ret = nnCnvStringtoPrefix(&testPrefix, ipv4);
 * @endcode
 *
 * @test 
 *  dest 의 값을 NULL 로 입력,
 *  pPrefix4 의 값을 NULL 로 입력
 */

Int8T nnCnvStringtoPrefix(PrefixT *pDest, StringT strAddr)
{
    Int8T ret = 0;
    Int8T addr[MAX_IPV6_LEN + 1] = {0, };
    Int8T tempAddr[strlen(strAddr)];

    if (pDest == NULL || strAddr == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    memset(pDest, 0, sizeof(PrefixT));

    /* Check IPV4 */
    ret = nnStrCheckIPv4Type(strAddr);

    if (ret == TRUE)
    {
        ret = inet_aton(strAddr, &pDest->u.prefix4);

        if (ret < 0)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR, "Convert String To Prefix : Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FAILURE;
        }

        pDest->family = AF_INET;
        pDest->prefixLen = MAX_IPV4_PREFIX_NUM;	/* MAX PREFIX LEN*/

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Convert String To Prefix : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return SUCCESS;
    }

    /* Check IPV4_PREFIX */
    ret = nnStrCheckIPv4PrefixType(strAddr);

    if (ret == TRUE)
    {
        memset(tempAddr, 0, sizeof(tempAddr));
        strcpy(tempAddr, strAddr);

        nnStrGetAndToken(tempAddr, addr, "/");

        ret = inet_aton(addr, &pDest->u.prefix4);

        if (ret < 0)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR, "Convert String To Prefix : Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FAILURE;
        }

        pDest->family = AF_INET;
        pDest->prefixLen = nnCnvStringtoInt(tempAddr);

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Convert String To Prefix : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return SUCCESS;
    }

#ifdef HAVE_IPV6
    /* Check IPV6 */
    ret = nnStrCheckIPv6Type(strAddr);

    if (ret == TRUE)
    {
        ret = inet_pton(AF_INET6, strAddr, &pDest->u.prefix6);

        if (ret < 0)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR, "Convert String To Prefix : Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FAILURE;
        }

	pDest->family = AF_INET6;
        pDest->prefixLen = MAX_IPV6_PREFIX_NUM; /* MAX_PREFIX LEN */

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Convert String To Prefix : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

	return SUCCESS;
    }

    /* Check IPV6_PREFIX */
    ret = nnStrCheckIPv6PrefixType(strAddr);

    if (ret == TRUE)
    {
        memset(tempAddr, 0, sizeof(tempAddr));
        strcpy(tempAddr, strAddr);

        nnStrGetAndToken(tempAddr, addr, "/");

        ret = inet_pton(AF_INET6, addr, &pDest->u.prefix6);

        if (ret < 0)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR, "Convert String To Prefix : Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FAILURE;
        }

        pDest->family = AF_INET6;
        pDest->prefixLen = nnCnvStringtoInt(tempAddr);

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Convert String To Prefix : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return SUCCESS;
    }
#endif /* HAVE IPV6 */

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_ERR, "Convert String To Prefix : Failure\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return FAILURE;
}


/**
 * Description: Prefix 구조체에 저장된 IP 주소와 PrefixLen 을
 *              문자열 형식으로 변환하는 함수
 *
 * @param [out] dest : 결과를 저장할 변수
 * @param [in] pPrefix : 저장된 IP 주소와 PrefixLen 을 변환할 Prefix 구조체
 *
 * @retval : PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  dest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  Int8T addrToStr[20] = {0,};
 *  PrefixT testPrefix = {0,};
 *  Int8T ret = 0;

 *  testPrefix.family = AF_INET;
 *  testPrefix.prefixLen = 24;
 *  testPrefix.u.prefix4.s_addr = 0x0101A8C0;
 *
 *  ret = nnCnvPrefixtoString(addrToStr, &testPrefix);
 * @endcode
 *
 * @test 
 *  pDest 의 값을 NULL 로 입력,
 *  strAddr 의 값을 NULL 로 입력
 */

Int8T nnCnvPrefixtoString(StringT dest, PrefixT *pPrefix)
{
    Int8T addr[MAX_IPV6_LEN + MAX_IPV6_PREFIX_LEN + 2] = {0, };

    if (dest == NULL || pPrefix == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    if (pPrefix->family == AF_INET)
    {
        nnCnvPrefix4toString(addr, (Prefix4T*)pPrefix);
    }

#ifdef HAVE_IPV6
    else if (pPrefix->family == AF_INET6)
    {
        nnCnvPrefix6toString(addr, (Prefix6T*)pPrefix);
    }
#endif /* HAVE IPV6 */

    sprintf(dest, "%s/%d", addr, pPrefix->prefixLen);

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Convert Prefix To String : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: Uint8T 형식의 PrefixLen 을
 *              struct in_addr 형식의 IPv4 Netmask 주소로 변환하는 함수
 *
 * @param [out] pDest : 결과를 저장할 변수
 * @param [in] pPrefixLen : 변환할 PrefixLen 값
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  pDest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  struct in_addr testAddr = {0,};
 *  Uint8T prefixLen = 0;
 *  Int8T ret = 0;
 *
 *  prefixLen = 24;
 *
 *  ret = nnCnvPrefixLentoNetmask(&testAddr, &prefixLen);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 *  pPrefixLen 의 값을 NULL 로 입력
 */

Int8T nnCnvPrefixLentoNetmask(struct in_addr *pDest, Uint8T *pPrefixLen)
{
    if (pDest == NULL || pPrefixLen == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    memset(pDest, 0, sizeof(struct in_addr));

    if (*pPrefixLen <= MAX_IPV4_PREFIX_NUM)
    {
        if (sizeof(unsigned long long) > 4)
        {
            pDest->s_addr = htonl(0xffffffffULL << (32 - *pPrefixLen));
        }
        else
        {
            pDest->s_addr = htonl(*pPrefixLen ?
                                  0xffffffffU << (32 - *pPrefixLen) :
                                  0);
        }

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Convert PrefixLen To Netmask(IPv4) : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return SUCCESS;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_ERR, "Convert PrefixLen To Netmask(IPv4) : Failure\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return FAILURE;
}


/**
 * Description: struct in_addr 형식의 IPv4 Netmask 주소를
 *              Uint8T 형식의 PrefixLen 으로 변환하는 함수
 *
 * @param [out] pDest : 결과를 저장할 변수
 * @param [in] pNetMask : 변환할 struct in_addr 형식의 IPv4 Netmask 주소
 *
 * @retval : PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  pDest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  Uint8T prefixLen = 0;
 *  struct in_addr testAddr = {0,};
 *  Int8T ret = 0;
 *
 *  testAddr.s_addr = 0x00FFFFFF;
 *
 *  ret = nnCnvNetmasktoPrefixLen(&prefixLen, &testAddr);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 *  pNetMask 의 값을 NULL 로 입력
 */

Int8T nnCnvNetmasktoPrefixLen(Uint8T *pDest, struct in_addr *pNetMask)
{
    Uint32T temp = 0;

    if (pDest == NULL || pNetMask == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    temp = ~ntohl(pNetMask->s_addr);

    if (temp)
    {
        /* return bit 0's count*/
	*pDest = __builtin_clz(temp);
    }
    else
    {
        *pDest = 32;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Convert Netmask(IPv4) To PrefixLen : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: Uint8T 형식의 PrefixLen 을
 *              struct in_addr 형식의 IPv4 Netmask 주소로 변환하는 함수

 * @param [in] prefixLen : 변환할 PrefixLen 값
 * @param [out] pNetMask : 결과를 저장할 변수
 *
 * @bug
 *  pDest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  Int32T prefixLen = 24;
 *  struct in_addr testAddr = {0,};
 *
 *  nnCnvMasklentoIp(prefixLen, &testAddr);
 * @endcode
 *
 * @test
 *  pNetMask 의 값을 NULL 로 입력
 */

void nnCnvMasklentoIp(Int32T prefixLen, struct in_addr *pNetmask)
{
    Int8T ret = 0;
    Uint8T tempPrefix = 0;

    if (pNetmask == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    tempPrefix = (Uint8T)prefixLen;
    ret = nnCnvPrefixLentoNetmask(pNetmask, &tempPrefix);

    if (ret != SUCCESS)
    {
        return;
    }
}


/**
 * Description: struct in_addr 형식의 IPv4 Netmask 주소를
 *              Uint8T 형식의 PrefixLen 으로 변환하여 리턴하는 함수
 *
 * @param [in] netMask : 변환할 struct in_addr 형식의 IPv4 Netmask 주소
 *
 * @retval : 변환 된 PrefixLen 값
 *
 * @code
 *  Uint8T prefixLen = 0;
 *  struct in_addr testAddr = {0,};
 *
 *  testAddr.s_addr = 0x00FFFFFF;
 *
 *  prefixLen = nnCnvIpmasktoLen(testAddr);
 * @endcode
 */

Uint8T nnCnvIpmasktoLen(struct in_addr netMask)
{
    Uint8T prefixLen = 0;
    Uint32T temp = 0;

    temp = ~ntohl(netMask.s_addr);

    if (temp)
    {
        /* return bit 0's count*/
	prefixLen = __builtin_clz(temp);
    }
    else
    {
        prefixLen = 32;
    }

    return prefixLen;
}

/**
 * Description: struct in_addr 형식의 IPv4 주소를
 *              Uint32T 형식의 주소로 변환하는 함수
 *
 * @param [out] pDest : 결과를 저장할 변수
 * @param [in] pAddr : 변환할 struct in_addr 형식의 IPv4 주소
 *
 * @retval : PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  dest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  Uint8T testUint = 0;
 *  struct in_addr testAddr = {0,};
 *  Int8T ret = 0;
 *
 *  testAddr.s_addr = 0x0101A8C0;
 *
 *  ret = nnCnvAddrtoUint(&testUint, &testAddr);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 *  pAddr 의 값을 NULL 로 입력
 */

Int8T nnCnvAddrtoUint(Uint32T *pDest, struct in_addr *pAddr)
{
    if (pDest == NULL || pAddr == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    *pDest = ntohl(pAddr->s_addr);

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Convert Address(struct in_addr) To Uint\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: struct in_addr 형식의 IPv4 주소와 PrefixLen 을 사용하여
 *              struct in_addr 형식의 Broadcast 주소로 변환하는 함수
 *
 * @param [out] pDest : 결과를 저장할 변수
 * @param [in] pAddr4 : 변환에 사용할 struct in_addr 형식의 IPv4 주소
 * @param [in] pPrefixLen : 변환에 사용할 PrefixLen 값
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  pDest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  struct in_addr testBroadAddr = {0,};
 *  Prefix4T testPrefix4 = {0,};
 *  Uint8T prefixLen = 0;
 *  Int8T ret = 0;
 *
 *  prefixLen = 24;
 *  testPrefix4.family = AF_INET;
 *  testPrefix4.prefix.s_addr = 0x0101A8C0;
 *
 *  ret = nnGetBroadcastfromAddr(&testBroadAddr,
 *                               &testPrefix4.prefix, &prefixLen);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 *  pAddr4 의 값을 NULL 로 입력,
 *  pPrefixLen 의 값을 NULL 로 입력
 */

Int8T nnGetBroadcastfromAddr(
    struct in_addr *pDest,
    struct in_addr *pAddr4,
    Uint8T *pPrefixLen)
{
    struct in_addr netMask = {0,};
    Int8T ret = 0;

    if (pDest == NULL || pAddr4 == NULL || pPrefixLen == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }
    else if (*pPrefixLen > MAX_IPV4_PREFIX_NUM)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Over or Less than MAX_IPV4_PREFIX_NUM\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return FAILURE;
    }

    memset(pDest, 0, sizeof(struct in_addr));

    ret = nnCnvPrefixLentoNetmask(&netMask, pPrefixLen);

    if (ret != SUCCESS)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR,
              "Get Broadcast(struct in_addr) From Address(struct in_addr) : \
              Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return FAILURE;
    }

    pDest->s_addr = (*pPrefixLen != MAX_IPV4_PREFIX_NUM - 1) ?
                    (pAddr4->s_addr | ~netMask.s_addr) :
                    (pAddr4->s_addr ^ ~netMask.s_addr);

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO,
          "Get Broadcast(struct in_addr) From Address(struct in_addr) : \
          Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: Prefix 구조체를 사용하여
 *              struct in_addr 형식의 Broadcast 주소로 변환하는 함수
 *
 * @param [out] pDest : 결과를 저장할 변수
 * @param [in] pPrefix : 변환에 사용할 Prefix 구조체
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  pDest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  struct in_addr testBroadAddr = {0,};
 *  PrefixT testPrefix = {0,};
 *  Int8T ret = 0;
 *
 *  testPrefix.family = AF_INET;
 *  testPrefix.prefixLen = 24;
 *  testPrefix.u.prefix4.s_addr = 0x0101A8C0;
 *
 *  ret = nnGetBroadcastfromPrefix(&testBroadAddr, &testPrefix);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 *  pPrefix 의 값을 NULL 로 입력
 */

Int8T nnGetBroadcastfromPrefix(struct in_addr *pDest, PrefixT *pPrefix)
{
    Int8T ret = 0;

    if (pDest == NULL || pPrefix == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    memset(pDest, 0, sizeof(struct in_addr));

    ret = nnGetBroadcastfromAddr(pDest, &pPrefix->u.prefix4,
                                 &pPrefix->prefixLen);

    if (ret < 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Get Broadcast(struct in_addr) From Prefix : Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
    }
    else
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO,
              "Get Broadcast(struct in_addr) From Prefix : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
    }

    return ret;
}


/**
 * Description: Prefix4 구조체의 값을  Prefix 구조체로 복사하는 함수
 *
 * @param [out] pDest : 값이 복사될 변수
 * @param [in] pPrefix4 : 복사할 값을 가진 Prefix4 구조체
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  pDest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  PrefixT testPrefix = {0,};
 *  Prefix4T testPrefix4 = {0,};
 *  Int8T ret = 0;
 *
 *  testPrefix4.family = AF_INET;
 *  testPrefix4.prefixLen = 24;
 *  testPrefix4.prefix.s_addr = 0x0101A8C0;
 *
 *  ret = nnCnvPrefix4TtoPrefixT(&testPrefix, &testPrefix4);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 *  pPrefix4 의 값을 NULL 로 입력
 */

Int8T nnCnvPrefix4TtoPrefixT(PrefixT *pDest, Prefix4T *pPrefix4)
{
    if (pDest == NULL || pPrefix4 == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }
    
    if (pPrefix4->family == AF_INET)
    {
      pDest->family = pPrefix4->family;
      pDest->prefixLen = pPrefix4->prefixLen;
      memcpy(&pDest->u.prefix4, &pPrefix4->prefix, sizeof(struct in_addr));
    }
    else
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR,
              "nnCnvPrefix4TtoPrefixT Wrong Address Family : Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

      return FAILURE;
    }
    return SUCCESS;
}


/**
 * Description: Prefix 구조체의 값을 Prefix4 구조체로 복사하는 함수
 *
 * @param [out] pDest : 값이 복사될 변수
 * @param [in] pPrefix : 복사할 값을 가진 Prefix 구조체
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  pDest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  Prefix4T testPrefix4 = {0,};
 *  PrefixT testPrefix = {0,};
 *  Int8T ret = 0;
 *
 *  testPrefix.family = AF_INET;
 *  testPrefix.prefixLen = 24;
 *  testPrefix.u.prefix4.s_addr = 0x0101A8C0;
 *
 *  ret = nnCnvPrefixTtoPrefix4T(&testPrefix4, &testPrefix);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 *  pPrefix 의 값을 NULL 로 입력
 */

Int8T nnCnvPrefixTtoPrefix4T(Prefix4T * pDest, PrefixT *pPrefix)
{
    if (pDest == NULL || pPrefix == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }
    
    if (pPrefix->family == AF_INET)
    {
      pDest->family = pPrefix->family;
      pDest->prefixLen = pPrefix->prefixLen;
      memcpy(&pDest->prefix, &pPrefix->u.prefix4, sizeof(struct in_addr));
    }
    else
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR,
              "nnCnvPrefix4TtoPrefixT Wrong Address Family : Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

      return FAILURE;
    }
    return SUCCESS;
}


/**
 * Description: 문자열 형식의 IPv4, IPv4/PrefixLen 주소와
 *              PrefixLen 을 사용하여 struct in_addr 형식의 Broadcast 주소로
 *              변환하는 함수
 *
 * @param [out] pDest : 결과를 저장할 변수
 * @param [in] strAddr : 변환에 사용할 문자열 형식의 IPv4, IPv4/PrefixLen 주소
 * @param [in] pPrefixLen : 변환에 사용할 PrefixLen 값
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  pDest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  struct in_addr testBroadAddr = {0,};
 *  StringT ipv4 = "192.168.1.1";
 *  Uint8T prefixLen = 0;
 *  Int8T ret = 0;
 *
 *  prefixLen = 24;
 *
 *  ret = nnGetBroadcastfromString(&testBroadAddr, ipv4, &prefixLen);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 *  strAddr 의 값을 NULL 로 입력,
 *  pPrefixLen 의 값을 NULL 로 입력
 */

Int8T nnGetBroadcastfromString(
    struct in_addr *pDest,
    StringT strAddr,
    Uint8T *pPrefixLen)
{
    Int8T ret = 0;
    struct in_addr tempAddr = {0,};
    Int8T addr[MAX_IPV4_LEN + 1] = {0, };
    Int8T tempStrAddr[strlen(strAddr)];
    Uint8T prefixLen = 0;
 
    if (pDest == NULL || strAddr == NULL || pPrefixLen == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    memset(pDest, 0, sizeof(struct in_addr));

   /* Type Check IPV4 */
    ret = nnStrCheckIPv4Type(strAddr);

    if (ret == TRUE)
    {
        ret = inet_aton(strAddr, &tempAddr);

        if (ret < 0)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR,
                  "Get Broadcast(struct in_addr) From String : Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FAILURE;
        }

        ret = nnGetBroadcastfromAddr(pDest, &tempAddr, pPrefixLen);

        if (ret < 0)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR,
                  "Get Broadcast(struct in_addr) From String : Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
        }
        else
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_INFO,
                  "Get Broadcast(struct in_addr) From String : Success\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
        }

        return ret;
    }

    /* Type check IPV4_PREFIX */
    ret = nnStrCheckIPv4PrefixType(strAddr);

    if (ret == TRUE)
    {
        memset(tempStrAddr, 0, sizeof(tempStrAddr));
        strcpy(tempStrAddr, strAddr);

        nnStrGetAndToken(tempStrAddr, addr, "/");

        prefixLen = atoi(tempStrAddr);

        ret = inet_aton(addr, &tempAddr);

        if (ret < 0)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR,
                  "Get Broadcast(struct in_addr) From String : Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FAILURE;
        }

        ret = nnGetBroadcastfromAddr(pDest, &tempAddr, &prefixLen);

        if (ret < 0)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR,
                  "Get Broadcast(struct in_addr) From String : Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
        }
        else
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_INFO,
                  "Get Broadcast(struct in_addr) From String : Success\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
        }

        return ret;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_ERR, "Get Broadcast(struct in_addr) From String : Failure\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return FAILURE;
}


/**
 * Description: Uint32T 형식의 IPv4 주소와 PrefixLen 을 사용하여
 *              struct in_addr 형식의 Broadcast 주소로 변환하는 함수
 *
 * @param [out] pDest : 결과를 저장할 변수
 * @param [in] pUintAddr : 변환에 사용할 Uint32T 형식의 IPv4 주소
 * @param [in] pPrefixLen : 변환에 사용할 PrefixLen 값
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  pDest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  struct in_addr testBroadAddr = {0,};
 *  Uint8T testUint = 0;
 *  Uint8T prefixLen = 0;
 *  Int8T ret = 0;
 *
 *  testUint = 0x0101A8C0;
 *  prefixLen = 24;
 *
 *  ret = nnGetBroadcastfromUint(&testBroadAddr, &testUint, &prefixLen);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 *  pUintAddr 의 값을 NULL 로 입력,
 *  pPrefixLen 의 값을 NULL 로 입력
 */

Int8T nnGetBroadcastfromUint(
    struct in_addr *pDest,
    Uint32T *pUintAddr,
    Uint8T *pPrefixLen)
{
    Int8T ret = 0;
    struct in_addr tempAddr = {0,};
 
    if (pDest == NULL || pUintAddr == NULL || pPrefixLen == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    memset(pDest, 0, sizeof(struct in_addr));

    tempAddr.s_addr = htonl(*pUintAddr);

    ret = nnGetBroadcastfromAddr(pDest, &tempAddr, pPrefixLen);

    if (ret < 0)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Get Broadcast(struct in_addr) From Uint : Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
    }
    else
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Get Broadcast(struct in_addr) From Uint : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
    }

    return ret;
}


/**
 * Description: AFI 를 Address Family 로 변환하는 함수
 *
 * @param [in] afi : Address Family 로 변환할 AFI 값
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           변환된 Address Family 값 : 성공
 *
 * @code
 *  Int32T ret = 0;
 *
 *  ret = nnCnvAfitoFamily(AFI_IP);
 * @endcode
 *
 * @test
 *  afi 의 값을 잘 못된 값으로 입력
 */

Int32T nnCnvAfitoFamily(Int32T afi)
{
    if (afi == AFI_IP)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Convert Afi To Family : AF_INET\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return AF_INET;
    }

#ifdef HAVE_IPV6
    else if (afi == AFI_IP6)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Convert Afi To Family : AF_INET6\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return AF_INET6;
    }
#endif /* HAVE_IPV6 */

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_ERR, "Convert Afi To Family : Failure\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return FAILURE;
}


/**
 * Description: Address Family 를 AFI 로 변환하는 함수
 *
 * @param [in] family : AFI 로 변환할 Address Family 값
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           변환된 AFI 값 : 성공
 *
 * @code
 *  Int32T ret = 0;
 *
 *  ret = nnCnvFamilytoAfi(AFI_IP);
 * @endcode
 *
 * @test
 *  family 의 값을 잘 못된 값으로 입력
 */

Int32T nnCnvFamilytoAfi(Int32T family)
{
    if (family == AF_INET)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Convert Family To Afi : AFI_IP\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return AFI_IP;
    }

#ifdef HAVE_IPV6
    else if (family == AF_INET6)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Convert Family To Afi : AFI_IP6\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return AFI_IP6;
    }
#endif /* HAVE_IPV6 */

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_ERR, "Convert Family To Afi : Failure\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return FAILURE;
}


/**
 * Description: 받은 두 개의 Prefix 구조체의 값 중
 *              pPrefix1 의 PrefixLen 이 pPrefix2 보다
 *              크지 않을 때 IPv4, IPv6 를 구분하지 않고
 *              pPrefix1 의 PrefixLen 까지의
 *              IP bit 를 비교하는 함수
 *
 * @param [in] pPrefix1 : 비교할 Prefix 구조체
 * @param [in] pPrefix2 : 비교할 Prefix 구조체
 *
 * @retval : PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           FALSE(0) : 조건에 일치하지 않음,
 *           TRUE(1) : 조건에 일치함
 *
 * @code
 *  PrefixT testPrefix = {0,};
 *  PrefixT testPrefix2 = {0,};
 *  Int32T ret = 0;
 *
 *  testPrefix.family = AF_INET;
 *  testPrefix.prefixLen = 24;
 *  testPrefix.u.prefix4.s_addr = 0x0101A8C0;
 *
 *  testPrefix2.family = AF_INET;
 *  testPrefix2.prefixLen = 24;
 *  testPrefix2.u.prefix4.s_addr = 0x0101A8C0;
 *
 *  ret = nnPrefixMatch(&testPrefix, &testPrefix2);
 * @endcode
 *
 * @test
 *  pPrefix1 의 값을 NULL 로 입력,
 *  pPrefix2 의 값을 NULL 로 입력
 */

Int8T nnPrefixMatch(const PrefixT *pPrefix1, const PrefixT *pPrefix2)
{
    Int32T offset = 0;
    Int32T shift = 0;
    const Uint8T *pP1 = NULL;
    const Uint8T *pP2 = NULL;

    if (pPrefix1 == NULL || pPrefix2 == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    /* Set both prefix's head pointer. */
    if (pPrefix1->family == AF_INET)
    {
        pP1 = (const Uint8T *)&pPrefix1->u.prefix4;
    }

#ifdef HAVE_IPV6
    else if (pPrefix1->family == AF_INET6)
    {
        pP1 = (const Uint8T *)&pPrefix1->u.prefix6;
    }
#endif
   
    if (pPrefix2->family == AF_INET)
    {
        pP2 = (const Uint8T *)&pPrefix2->u.prefix4;
    }

#ifdef HAVE_IPV6
    else if (pPrefix2->family == AF_INET6)
    {
        pP2 = (const Uint8T *)&pPrefix2->u.prefix6;
    }
#endif

  /* If n's prefix is longer than p's one return 0. */
    if (pPrefix1->prefixLen > pPrefix2->prefixLen)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Left PrefixLen is Longer Than Right PrefixLen\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return FALSE;
    }

    offset = pPrefix1->prefixLen / PREFIX_PNBBY;
    shift =  pPrefix1->prefixLen % PREFIX_PNBBY;

    if (shift)
    {
        if (gMaskbit[shift] & (pP1[offset] ^ pP2[offset]))
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_INFO, "Two Prefix Are Not Match\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FALSE;
        }
    }

    while (offset--)
    {
        if (pP1[offset] != pP2[offset])
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_INFO, "Two Prefix Are Not Match\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FALSE;
        }
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Two Prefix Are Match\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return TRUE;
}


/**
 * Description: Prefix 구조체의 값을 Prefix 구조체로 복사하는 함수
 *
 * @param [out] pDest : 값이 복사될 변수
 * @param [in] pSrc : 복사할 값을 가진 Prefix 구조체
 *
 * @retval : FAILURE(-1) : 복사 실패,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  pDest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  PrefixT testPrefix = {0,};
 *  PrefixT testPrefix2 = {0,};
 *  Int32T ret = 0;
 *
 *  testPrefix.family = AF_INET;
 *  testPrefix.prefixLen = 24;
 *  testPrefix.u.prefix4.s_addr = 0x0101A8C0;
 *
 *  ret = nnPrefixMatch(&testPrefix2, &testPrefix);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 *  pSrc 의 값을 NULL 로 입력
 */

Int8T nnPrefixCopy(PrefixT *pDest, const PrefixT *pSrc)
{
    if (pDest == NULL || pSrc == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    memset(pDest, 0, sizeof(PrefixT));

    pDest->family = pSrc->family;
    pDest->prefixLen = pSrc->prefixLen;

    if (pSrc->family == AF_INET)
    {
        pDest->u.prefix4 = pSrc->u.prefix4;
    }

#ifdef HAVE_IPV6
    else if (pSrc->family == AF_INET6)
    {
        pDest->u.prefix6 = pSrc->u.prefix6;
    }
#endif /* HAVE_IPV6 */

    else
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR,
              "Prefix Copy : Failure, Unknown Address Family %d\n",
              pSrc->family);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return FAILURE;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Prefix Copy : Successs\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: 두 개의 Prefix 구조체의 Address Family,
 *              prefixLen, IP 가 같은 값인지 비교하는 함수
 *
 * @param [in] pPrefix1 : 비교할 Prefix 구조체
 * @param [in] pPrefix2 : 비교할 Prefix 구조체
 *
 * @retval : PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           FALSE(0) : 조건에 일치하지 않음,
 *           TRUE(1) : 조건에 일치함
 *
 * @code
 *  PrefixT testPrefix = {0,};
 *  PrefixT testPrefix2 = {0,};
 *  Int32T ret = 0;
 *
 *  testPrefix.family = AF_INET;
 *  testPrefix.prefixLen = 24;
 *  testPrefix.u.prefix4.s_addr = 0x0101A8C0;
 *
 *  testPrefix2.family = AF_INET;
 *  testPrefix2.prefixLen = 24;
 *  testPrefix2.u.prefix4.s_addr = 0x0101A8C0;
 *
 *  ret = nnPrefixSame(&testPrefix, &testPrefix2);
 * @endcode
 *
 * @test
 *  pPrefix1 의 값을 NULL 로 입력,
 *  pPrefix2 의 값을 NULL 로 입력
 */

Int8T nnPrefixSame(const PrefixT *pPrefix1, const PrefixT *pPrefix2)
{
    if (pPrefix1 == NULL || pPrefix2 == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    if (pPrefix1->family == pPrefix2->family &&
        pPrefix1->prefixLen == pPrefix2->prefixLen)
    {
        if (pPrefix1->family == AF_INET)
        {
            if (PREFIX_IPV4_ADDR_SAME(&pPrefix1->u.prefix4,
                                    &pPrefix2->u.prefix4))
            {
                NNLOGDEBUG(LOG_DEBUG, "################################\n");
                NNLOGDEBUG(LOG_INFO, "Two Prefix Are Same\n");
                NNLOGDEBUG(LOG_DEBUG, "################################\n");

                return TRUE;
            }
        }

#ifdef HAVE_IPV6
        if (pPrefix1->family == AF_INET6)
        {
            if (PREFIX_IPV6_ADDR_SAME(&pPrefix1->u.prefix6,
                                    &pPrefix2->u.prefix6))
            {
                NNLOGDEBUG(LOG_DEBUG, "################################\n");
                NNLOGDEBUG(LOG_INFO, "Two Prefix Are Same\n");
                NNLOGDEBUG(LOG_DEBUG, "################################\n");

                return TRUE;
            }
        }
#endif /* HAVE_IPV6 */

    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Two Prefix Are Not Same\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return FALSE;
}


/**
 * Description: 두 개의 Prefix 구조체의 Address Family,
 *              PrefixLen 이 같고 pPrefix1 의 PrefixLen 까지의
 *              IP bit 가 같은 값인지 비교하는 함수
 *
 * @param [in] pPrefix1 : 비교할 Prefix 구조체
 * @param [in] pPrefix2 : 비교할 Prefix 구조체
 *
 * @retval : PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           FALSE(0) : 조건에 일치하지 않음,
 *           TRUE(1) : 조건에 일치함
 *
 * @code
 *  PrefixT testPrefix = {0,};
 *  PrefixT testPrefix2 = {0,};
 *  Int32T ret = 0;
 *
 *  testPrefix.family = AF_INET;
 *  testPrefix.prefixLen = 24;
 *  testPrefix.u.prefix4.s_addr = 0x0101A8C0;
 *
 *  testPrefix2.family = AF_INET;
 *  testPrefix2.prefixLen = 24;
 *  testPrefix2.u.prefix4.s_addr = 0x0101A8C0;
 *
 *  ret = nnPrefixCmp(&testPrefix, &testPrefix2);
 * @endcode
 *
 * @test
 *  pPrefix1 의 값을 NULL 로 입력,
 *  pPrefix2 의 값을 NULL 로 입력
 */

Int8T nnPrefixCmp(const PrefixT *pPrefix1, const PrefixT *pPrefix2)
{
    Int32T offset = 0;
    Int32T shift = 0;
    const Uint8T *pP1 = NULL;
    const Uint8T *pP2 = NULL;

    if (pPrefix1 == NULL || pPrefix2 == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    /* Check family and prefixlen. */
    if (pPrefix1->family != pPrefix2->family ||
        pPrefix1->prefixLen != pPrefix2->prefixLen)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Family or PrefixLen Is Not Same\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return FALSE;
    }

    /* Set both prefix's head pointer. */
    if (pPrefix1->family == AF_INET && pPrefix2->family == AF_INET)
    {
        pP1 = (const Uint8T *)&pPrefix1->u.prefix4;
        pP2 = (const Uint8T *)&pPrefix2->u.prefix4;
    }

#if HAVE_IPV6
    else if (pPrefix1->family == AF_INET6 && pPrefix2->family == AF_INET6)
    {
        pP1 = (const Uint8T *)&pPrefix1->u.prefix6;
        pP2 = (const Uint8T *)&pPrefix2->u.prefix6;
    }
#endif

    offset = pPrefix1->prefixLen / 8;
    shift = pPrefix1->prefixLen % 8;

    if (shift)
    {
        if (gMaskbit[shift] & (pP1[offset] ^ pP2[offset]))
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_INFO, "Two Prefix Are Not Same\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FALSE;
        }
    }

    while (offset--)
    {
        if (pP1[offset] != pP2[offset])
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_INFO, "Two Prefix Are Not Same\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FALSE;
        }
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Two Prefix Are Same\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return TRUE;
}


/**
 * Description: Prefix 구조체의 Address Family 를
 *              문자열로 변환하는 함수
 *
 * @param [in] pPrefix : 문자열로 변환할 Address Family 를 가진 Prefix 구조체
 *
 * @retval : "unspec" : 잘 못된 값일 때,
 *           "inet" 또는 "inet6" : 성공
 *
 * @code
 *  PrefixT testPrefix = {0,};
 *  StringT strRet = NULL;
 *
 *  memset(&testPrefix, 0x0, sizeof(testPrefix));
 *
 *  testPrefix.family = AF_INET;
 *  testPrefix.prefixLen = 24;
 *  testPrefix.u.prefix4.s_addr = 0x0101A8C0;
 *
 *  strRet = nnCnvPrefixFamilytoString(&testPrefix);
 * @endcode
 *
 * @test
 *  pPrefix 의 값을 NULL 로 입력
 */

StringT nnCnvPrefixFamilytoString(const PrefixT *pPrefix)
{
    if (pPrefix->family == AF_INET)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Convert Prefix Family To String : inet\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return "inet";
    }

#ifdef HAVE_IPV6
    if (pPrefix->family == AF_INET6)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Convert Prefix Family To String : inet6\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return "inet6";
    }
#endif /* HAVE_IPV6 */

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Convert Prefix Family To String : unspec\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return "unspec";
}


/**
 * Description: Prefix4 구조체를 생성하여 리턴하는 함수
 *
 * @retval : PREFIX_ERR_PREFIX_ALLOC(-1) : Prefix 의
 *                                         메모리 할당에 실패할 때,
 *           생성된 Prefix4 구조체 : 성공
 *
 * @code
 *  Prefix4T *pTestPrefix4 = NULL;
 *
 *  pTestPrefix4 = nnPrefixIpv4New();
 * @endcode
 */

Prefix4T *nnPrefixIpv4New()
{
    Prefix4T *pPrefix4 = NULL;

    pPrefix4 = (Prefix4T *)nnPrefixNew();

    if ((Int32T)pPrefix4 <= 0 && (Int32T)pPrefix4 >= MEMMGR_ERR_ARGUMENT)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR,
              "NewPrefix4T Memory Allocate : Failure(%d)\n", (Int32T)pPrefix4);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (Prefix4T *)PREFIX_ERR_PREFIX_ALLOC;
    }

    pPrefix4->family = AF_INET;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Init NewPrefix4T : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return pPrefix4;
}


/**
 * Description: Prefix4 구조체를 제거하는 함수
 *
 * @param [in] pPrefix4 : 제거할 Prefix4 구조체
 *
 * @retval : None
 *
 * @code
 *  nnPrefixIpv4Free(pTestPrefix4);
 * @endcode
 *
 * @test
 *  pPrefix4 의 값을 NULL 로 입력
 */

void nnPrefixIpv4Free(Prefix4T *pPrefix4)
{
    if (pPrefix4 == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    nnPrefixFree((PrefixT *)pPrefix4);

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Free Prefix4T Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}


/**
 * Description: Prefix4 구조체에 NetMask 를 적용하는 함수
 *
 * @param [in/out] pDest : Netmask 를 적용할 Prefix4 구조체
 * @param [in] pPrefixLen : Uint8T 형식의 PrefixLen 값
 *
 * @retval : None
 *
 * @code
 *  Prefix4T testPrefix4 = {0,};
 *  Uint8T prefixLen = 0;
 *
 *  prefixLen = 24;
 *  testPrefix4.family = AF_INET;
 *  testPrefix4.u.prefix4.s_addr = 0x0101A8C0;
 *
 *  nnApplyNetmasktoPrefix4(&testPrefix4, &prefixLen);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 *  pPrefixLen 의 값을 NULL 로 입력
 */

void nnApplyNetmasktoPrefix4(Prefix4T *pDest, Uint8T *pPrefixLen)
{
    Uint8T *pPnt = NULL;
    Int32T index = 0;
    Int32T offset = 0;

    if (pDest == NULL || pPrefixLen == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    index = *pPrefixLen / 8;

    if (index < 4)
    {
        pPnt = (Uint8T *) &pDest->prefix;
        offset = *pPrefixLen % 8;

        pPnt[index] &= gMaskbit[offset];
        index++;

        while (index < 4)
        {
            pPnt[index++] = 0;
        }
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Netmask Apply To Prefix4T Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}


/**
 * Description: pPrefix 구조체가 0.0.0.0/0 값을 갖는지
 *              검사하는 함수
 *
 * @param [in] pPrefix : 검사할 Prefix4 구조체
 *
 * @retval : PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           FALSE(0) : 조건에 일치하지 않음,
 *           TRUE(1) : 조건에 일치함
 *
 * @code
 *  Prefix4T testPrefix4 = {0,};
 *  Int8T ret = 0;
 *
 *  testPrefix4.family = AF_INET;
 *  testPrefix4.prefixLen = 0;
 *  testPrefix4.u.prefix4.s_addr = 0x0;
 *
 *  ret = nnPrefixCheckIpv4Any(&testPrefix4);
 * @endcode
 *
 * @test
 *  pPrefix 의 값을 NULL 로 입력
 */

Int8T nnPrefixCheckIpv4Any(const Prefix4T *pPrefix)
{
    Int8T ret = 0;

    if (pPrefix == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    ret = (pPrefix->prefix.s_addr == 0 && pPrefix->prefixLen == 0);

    if (ret == 1)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Prefix Is IPV4 Any Address\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return TRUE;
    }
    else
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Prefix Is Not IPV4 Any Address\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return FALSE;
    }
}


/**
 * Description: Prefix 구조체에 NetMask 를 적용하는 함수
 *
 * @param [in/out] pDest : Netmask 를 적용할 Prefix 구조체
 * @param [in] pPrefixLen : Uint8T 형식의 PrefixLen 값
 *
 * @retval : None
 *
 * @code
 *  PrefixT testPrefix = {0,};
 *  Uint8T prefixLen = 0;
 *
 *  prefixLen = 24;
 *  testPrefix.family = AF_INET;
 *  testPrefix.u.prefix4.s_addr = 0x0101A8C0;
 *
 *  nnApplyNetmasktoPrefix(&testPrefix, &prefixLen);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 *  pPrefixLen 의 값을 NULL 로 입력
 */

void nnApplyNetmasktoPrefix(PrefixT *pDest, Uint8T *pPrefixLen)
{
    if (pDest == NULL || pPrefixLen == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    switch (pDest->family)
    {
        case AF_INET:
            nnApplyNetmasktoPrefix4((Prefix4T *)pDest, pPrefixLen);
            break;

#ifdef HAVE_IPV6
        case AF_INET6:
            nnApplyNetmasktoPrefix6((Prefix6T *)pDest, pPrefixLen);
            break;
#endif /* HAVE_IPV6 */

        default:
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR, "Netmask Apply To Prefix : Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            break;
    }

    return;
}


/**
 * Description: Prefix 구조체의 Addrss Family 를 검사하여
 *              IPv4 또는 IPv6 의 최대 IP Byte 값을 리턴하는 함수
 *
 * @param [in] pPrefix : Address Family 를 검사할 Prefix 구조체
 *
 * @retval : FAILURE(-1) : 잘못된 Address Family,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           PREFIX_IPV4_MAX_BYTELEN 또는 PREFIX_IPV6_MAX_BYTELEN : 성공
 *
 * @code
 *  PrefixT testPrefix = {0,};
 *  Int8T ret = 0;
 *
 *  testPrefix.family = AF_INET;
 *
 *  ret = nnGetPrefixByteLen(&testPrefix);
 * @endcode
 *
 * @test
 *  pPrefix 의 값을 NULL 로 입력
 */

Int8T nnGetPrefixByteLen(const PrefixT *pPrefix)
{
    if (pPrefix == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    switch (pPrefix->family)
    {
        case AF_INET:
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_INFO,
                  "Get Prefix ByteLen : IPV4(%d)\n", PREFIX_IPV4_MAX_BYTELEN);
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return PREFIX_IPV4_MAX_BYTELEN;

#ifdef HAVE_IPV6
        case AF_INET6:
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_INFO,
                  "Get Prefix ByteLen : IPV6(%d)\n", PREFIX_IPV6_MAX_BYTELEN);
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return PREFIX_IPV6_MAX_BYTELEN;
#endif /* HAVE_IPV6 */

    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_ERR, "Get Prefix ByteLen : Failure\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return FAILURE;
}


/**
 * Description: Prefix 구조체를 생성하여 리턴하는 함수
 *
 * @retval : PREFIX_ERR_PREFIX_ALLOC(-1) : Prefix 의
 *                                         메모리 할당에 실패할 때,
 *           생성된 Prefix 구조체 : 성공
 *
 * @code
 *  PrefixT *pTestPrefix = NULL;
 *
 *  pTestPrefix = nnPrefixNew();
 * @endcode
 */

PrefixT *nnPrefixNew()
{
    PrefixT *pPrefix = NULL;

    pPrefix = NNMALLOC(MEM_ROUTE_RIB, sizeof(PrefixT));

    if ((Int32T)pPrefix <= 0 && (Int32T)pPrefix >= MEMMGR_ERR_ARGUMENT)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR,
              "NewPrefixT Memory Allocate : Failure(%d)\n", (Int32T)pPrefix);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (PrefixT *)PREFIX_ERR_PREFIX_ALLOC;
    }
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Init NewPrefixT : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return pPrefix;
}


/**
 * Description: Prefix 구조체를 제거하는 함수
 *
 * @param [in] pPrefix : 제거할 Prefix 구조체
 *
 * @retval : None
 *
 * @code
 *   nnPrefixFree(pPrefix);
 * @endcode
 *
 * @test
 *  pPrefix 의 값을 NULL 로 입력
 */

void nnPrefixFree(PrefixT *pPrefix)
{
    if (pPrefix == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    NNFREE(MEM_ROUTE_RIB, pPrefix);

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Free PrefixT Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}


/**
 * Description: 문자열이 0 ~ 9 의 값인지 검사하는 함수
 *
 * @param [in] str : 검사할 문자열
 *
 * @retval : PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           FALSE(0) : 조건에 일치하지 않음,
 *           TRUE(1) : 조건에 일치 함
 *
 * @code
 *  StringT strNum = "123456";
 *  Int8T ret = 0;
 *
 *  ret = nnCheckAllDigit(strNum);
 * @endcode
 *
 * @test
 *  str 의 값을 NULL 로 입력
 */

Int8T nnCheckAllDigit(const StringT str)
{
    StringT temp = NULL;

    if (str == NULL)
    {
        return PREFIX_ERR_NULL;
    }

    temp = str;

    for (; *temp != '\0'; temp++)
    {
        if (!isdigit ((Int32T) *temp))
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_INFO, "Check All Digit : False\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FALSE;
        }
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Check All Digit : True\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return TRUE;
}


/**
 * Description: Prefix4 구조체에 Classful NetMask 를 적용하는 함수
 *
 * @param [in/out] pDest : Classful Netmask 를 적용할 Prefix4 구조체
 *
 * @retval : PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @code
 *  Prefix4T testPrefix4 = {0,};
 *
 *  testPrefix4.family = AF_INET;
 *  testPrefix4.u.prefix4.s_addr = 0x0101A8C0;
 *
 *  ret = nnApplyClassfulMaskIpv4(&testPrefix4);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 */

Int8T nnApplyClassfulMaskIpv4(Prefix4T *pDest)
{
    Uint32T destination = 0;

    if (pDest == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    destination = ntohl (pDest->prefix.s_addr);

    if(pDest->prefixLen == PREFIX_IPV4_MAX_PREFIXLEN)
    {
        /* do nothing for host routes */
    }
    else if(IN_CLASSC(destination))
    {
        pDest->prefixLen = 24;
        nnApplyNetmasktoPrefix4(pDest, &pDest->prefixLen);
    }
    else if (IN_CLASSB(destination))
    {
        pDest->prefixLen = 16;
        nnApplyNetmasktoPrefix4(pDest, &pDest->prefixLen);
    }
    else
    {
        pDest->prefixLen = 8;
        nnApplyNetmasktoPrefix4(pDest, &pDest->prefixLen);
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Classful Netmask Apply To Prefix4T Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: Uint8T 형식의 pPrefixLen 을
 *              struct in_addr 형식의 주소로 변환한 후
 *              struct in_addr pAddr4 에
 *              Uint8T 형식의 pPrefixLen 을 적용한 결과를
 *              struct in_addr 형식의 pDest 에 저장하는 함수
 *
 * @param [out] pDest : 결과를 저장할 변수
 * @param [in] pAddr4 : 사용할 struct in_addr 형식의 주소
 * @param [in] pPrefixLen : Uint8T 형식의 PrefixLen 값
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @code
 *  struct in_addr testAddr1 = {0,};
 *  struct in_addr testAddr2 = {0,};
 *  Uint8T prefixLen = 0;
 *  Int8T ret = 0;
 *
 *  prefixLen = 24;
 *  testAddr2.s_addr = 0x0101A8C0;
 *
 *  ret = nnApplyPrefixLentoAddr(&testAddr1, &testAddr2, &prefixLen);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 *  pAddr4 의 값을 NULL 로 입력,
 *  pPrefixLen 의 값을 NULL 로 입력
 */

Int8T nnApplyPrefixLentoAddr(
    struct in_addr *pDest,
    struct in_addr *pAddr4,
    Uint8T *pPrefixLen)
{
    struct in_addr mask = {0,};
    Int8T ret = 0;

    if (pDest == NULL || pAddr4 == NULL || pPrefixLen == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    ret = nnCnvPrefixLentoNetmask(&mask, pPrefixLen);

    if (ret != SUCCESS)
    {
        return FAILURE;
    }

    pDest->s_addr = (pAddr4->s_addr & mask.s_addr);

    return SUCCESS;
}


/**
 * Description: 문자열 형식의 netStr 를 주소로
 *              문자열 형식의 maskStr 을 Netmask 로 변환하여 적용한 결과를
 *              문자열 형식의 destStr 에
 *              IP/PrefixLen 형식으로 저장하는 함수
 *
 * @param [out] destStr : 결과를 저장할 변수
 * @param [in] netStr : 문자열 형식의 IPv4 주소
 * @param [in] maskStr : 문자열 형식의 IPv4 Netmask 주소
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @code
 *  Int8T testAddr = {0,};
 *  StringT network = "192.168.1.1";
 *  StringT netmask = "255.255.255.0";
 *  Int8T ret = 0;
 *
 *  ret = nnCnvNetmaskstrtoPrefixstr(testAddr, network, netmask);
 * @endcode
 *
 * @test
 *  destStr 의 값을 NULL 로 입력,
 *  netStr 의 값을 NULL 로 입력,
 *  maskStr 의 값을 NULL 로 입력
 */

Int8T nnCnvNetmaskstrtoPrefixstr(
    StringT destStr,
    const StringT netStr,
    const StringT maskStr)
{
    struct in_addr network = {0,};
    struct in_addr mask = {0,};
    Uint8T prefixlen = 0;
    Uint32T destination = 0;
    Int8T ret = 0;

    if (destStr == NULL || netStr == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    ret = inet_aton(netStr, &network);

    if (!ret)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR,
              "Convert Netmask String to Prefix String : \
              inet_aton netStr Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return FAILURE;
    }

    if (maskStr)
    {   
        ret = inet_aton(maskStr, &mask);

        if (!ret)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR,
                  "Convert Netmask String to Prefix String : \
                  inet_aton maskStr Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FAILURE;
        }

        ret = nnCnvNetmasktoPrefixLen(&prefixlen, &mask);

        if (ret != SUCCESS)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR,
                  "Convert Netmask String to Prefix String : Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FAILURE;
        }
    }   
    else 
    {   
        destination = ntohl (network.s_addr);

        if (network.s_addr == 0)
        {
            prefixlen = 0;
        }
        else if (IN_CLASSC (destination))
        {
            prefixlen = 24;
        }
        else if (IN_CLASSB (destination))
        {
            prefixlen = 16;
        }
        else if (IN_CLASSA (destination))
        {
            prefixlen = 8;
        }
        else
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR,
                  "Convert Netmask String to Prefix String : Class Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FAILURE;
        }
    }

    sprintf (destStr, "%s/%d", netStr, prefixlen);

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Convert Netmask String to Prefix String : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


#ifdef HAVE_IPV6
/**
 * Description: 문자열 형식의 IPv6 또는 IPv6/Prefix 주소를
 *              struct in6_addr 형식으로 변환하는 함수
 *
 * @param [out] pDest : 결과를 저장할 변수
 * @param [in] strAddr6 : 변환할 문자열 형식의 IPv6 또는 IPv6/Prefix 주소
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  pDest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  struct in6_addr testAddr6 = {0,};
 *  StringT ipv6 = "1111:2222:3333:4444:5555:6666:7777:8888";
 *  Int8T ret = 0;
 *
 *  ret = nnCnvStringtoAddr6(&testAddr6, ipv6);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 *  strAddr6 의 값을 NULL 로 입력
 */

Int8T nnCnvStringtoAddr6(struct in6_addr *pDest, StringT strAddr6)
{
    Int8T ret = 0;
    Int8T addr6[MAX_IPV6_LEN + 1] = {0, };
    Int8T tempAddr6[strlen(strAddr6)];

    if (pDest == NULL || strAddr6 == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    memset(pDest, 0, sizeof(struct in6_addr));

    /* Type Check IPV6 */
    ret = nnStrCheckIPv6Type(strAddr6);

    if (ret == TRUE)
    {
        ret = inet_pton(AF_INET6, strAddr6, pDest);

        if (ret <= 0)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR, "Convert String To struct in6_addr : Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FAILURE;
        }

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Convert String To struct in6_addr : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return SUCCESS;
    }

    /* Type check IPV6_PREFIX */
    ret = nnStrCheckIPv6PrefixType(strAddr6);

    if (ret == TRUE)
    {
        memset(tempAddr6, 0, sizeof(tempAddr6));
        strcpy(tempAddr6, strAddr6);

        nnStrGetAndToken(tempAddr6, addr6, "/");

        ret = inet_pton(AF_INET6, addr6, pDest);

        if (ret <= 0)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR, "Convert String To struct in6_addr : Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FAILURE;
        }

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Convert String To struct in6_addr : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return SUCCESS;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_ERR, "Convert String To struct in6_addr : Failure\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return FAILURE;
}


/**
 * Description: struct in6_addr 형식의 IPv6 주소를
 *              문자열 형식으로 변환하는 함수
 *
 * @param [out] dest : 결과를 저장할 변수
 * @param [in] pAddr6 : 변환할 struct in6_addr 형식의 주소
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  dest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  Int8T addr6ToStr[40] = {0,};
 *  struct in6_addr testAddr6 = {0,};
 *  Int8T ret = 0;
 *
 *  testAddr6.s6_addr[0] = 0x11;
 *  ...
 *  testAddr6.s6_addr[15] = 0x88;
 *
 *  ret = nnCnvAddr6toString(addr6ToStr, &testAddr6);
 * @endcode
 *
 * @test
 *  dest 의 값을 NULL 로 입력,
 *  pAddr6 의 값을 NULL 로 입력
 */

Int8T nnCnvAddr6toString(StringT dest, struct in6_addr *pAddr6)
{
    StringT ret = 0;

    if (dest == NULL || pAddr6 == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    ret = (StringT)inet_ntop(AF_INET6, pAddr6, dest, MAX_IPV6_LEN + 1);

    if (ret == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Convert struct in6_addr To String : Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return FAILURE;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Convert struct in6_addr To String : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: 문자열 형식의 IPv6 또는 IPv6/Prefix 주소를
 *              Prefix6 구조체에 저장하는 함수
 *
 * @param [out] pDest : 결과를 저장할 변수
 * @param [in] strAddr6 : 저장할 문자열 형식의 IPv6 또는 IPv6/Prefix 주소
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  pDest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  Prefix6T testPrefix6 = {0,};
 *  StringT ipv6 = "1111:2222:3333:4444:5555:6666:7777:8888";
 *  Int8T ret = 0;
 *
 *  ret = nnCnvStringtoPrefix6(&testPrefix6, ipv6);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 *  strAddr6 의 값을 NULL 로 입력
 */

Int8T nnCnvStringtoPrefix6(Prefix6T *pDest, StringT strAddr6)
{
    Int8T ret = 0;
    Int8T addr6[MAX_IPV6_LEN + 1] = {0, };
    Int8T tempAddr6[strlen(strAddr6)];

    if (pDest == NULL || strAddr6 == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    memset(pDest, 0, sizeof(Prefix6T));

    /* Type Check IPV6 */
    ret = nnStrCheckIPv6Type(strAddr6);

    if (ret == TRUE)
    {
        ret = inet_pton(AF_INET6, strAddr6, &pDest->prefix);

        if (ret <= 0)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR, "Convert String To Prefix6 : Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FAILURE;
        }

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Convert String To Prefix6 : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return SUCCESS;
    }

    /* Type check IPV6_PREFIX */
    ret = nnStrCheckIPv6PrefixType(strAddr6);

    if (ret == TRUE)
    {
        memset(tempAddr6, 0, sizeof(tempAddr6));
        strcpy(tempAddr6, strAddr6);

        nnStrGetAndToken(tempAddr6, addr6, "/");

        ret = inet_pton(AF_INET6, addr6, &pDest->prefix);

        if (ret <= 0)
        {
            NNLOGDEBUG(LOG_DEBUG, "################################\n");
            NNLOGDEBUG(LOG_ERR, "Convert String To Prefix6 : Failure\n");
            NNLOGDEBUG(LOG_DEBUG, "################################\n");

            return FAILURE;
        }

        pDest->family = AF_INET6;
        pDest->prefixLen = atoi(tempAddr6);

        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_INFO, "Convert String To Prefix6 : Success\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return SUCCESS;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_ERR, "Convert String To Prefix6 : Failure\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return FAILURE;

}


/**
 * Description: Prefix6 구조체에 저장된 IP 주소를
 *              문자열 형식으로 변환하는 함수
 *
 * @param [out] dest : 결과를 저장할 변수
 * @param [in] pPrefix6 : 저장된 IP 주소를 변환할 Prefix6 구조체
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  dest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  Int8T addr6ToStr[40] = {0,};
 *  Prefix4T testPrefix6 = {0,};
 *  Int8T ret = 0;
 *
 *  testPrefix6.prefixLen = 100;
 *  testPrefix6.prefix.s6_addr[0] = 0x11;
 *  ...
 *  testPrefix6.prefix.s6_addr[15] = 0x88;
 *
 *  ret = nnCnvPrefix6toString(addr6ToStr, &testPrefix6);
 * @endcode
 *
 * @test
 *  dest 의 값을 NULL 로 입력,
 *  pPrefix6 의 값을 NULL 로 입력
 */

Int8T nnCnvPrefix6toString(StringT dest, Prefix6T *pPrefix6)
{
    StringT ret = 0;

    if (dest == NULL || pPrefix6 == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    ret = (StringT)inet_ntop(AF_INET6, &pPrefix6->prefix,
                             dest, MAX_IPV6_LEN + 1);

    if (ret == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Convert Prefix6 To String: Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return FAILURE;
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Convert Prefix6 To String: Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


 /**
 * Description: Prefix6 구조체의 값을 Prefix 구조체로 복사하는 함수
 *
 * @param [out] pDest : 값이 복사될 변수
 * @param [in] pPrefix6 : 복사할 값을 가진 Prefix6 구조체
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  pDest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  PrefixT testPrefix = {0,};
 *  Prefix6T testPrefix6 = {0,};
 *  Int8T ret = 0;
 *
 *  testPrefix6.family = AF_INET6;
 *  testPrefix6.prefixLen = 100;
 *  testPrefix6.prefix.s6_addr[0] = 0x11;
 *  ...
 *  testPrefix6.prefix.s6_addr[15] = 0x88;
 *
 *  ret = nnCnvPrefix6TtoPrefixT(&testPrefix, &testPrefix6);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 *  pPrefix6 의 값을 NULL 로 입력
 */

Int8T nnCnvPrefix6TtoPrefixT(PrefixT *pDest, Prefix6T *pPrefix6)
{
    if (pDest == NULL || pPrefix6 == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }
    
    if (pPrefix6->family == AF_INET6)
    {
      pDest->family = pPrefix6->family;
      pDest->prefixLen = pPrefix6->prefixLen;
      memcpy(&pDest->u.prefix6, &pPrefix6->prefix, sizeof(struct in6_addr));
    }
    else
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR,
              "nnCnvPrefix6TtoPrefixT Wrong Address Family : Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

      return FAILURE;
    }

    return SUCCESS;
}


/**
 * Description: Prefix 구조체의 값을 Prefix6 구조체로 복사하는 함수
 *
 * @param [out] pDest : 값이 복사될 변수
 * @param [in] pPrefix : 복사할 값을 가진 Prefix 구조체
 *
 * @retval : FAILURE(-1) : 변환 실패,
 *           PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @bug
 *  pDest 의 메모리는 사용자가 관리해야 함
 *
 * @code
 *  Prefix4T testPrefix6 = {0,};
 *  PrefixT testPrefix = {0,};
 *  Int8T ret = 0;
 *
 *  testPrefix.family = AF_INET6;
 *  testPrefix.prefixLen = 100;
 *  testPrefix.u.prefix6.s6_addr[0] = 0x11;
 *  ...
 *  testPrefix.u.prefix6.s6_addr[15] = 0x88;
 *
 *  ret = nnCnvPrefixTtoPrefix6T(&testPrefix6, &testPrefix);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 *  pPrefix 의 값을 NULL 로 입력
 */

Int8T nnCnvPrefixTtoPrefix6T(Prefix6T *pDest, PrefixT *pPrefix)
{
    if (pDest == NULL || pPrefix == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }
    
    if (pPrefix->family == AF_INET6)
    {
      pDest->family = pPrefix->family;
      pDest->prefixLen = pPrefix->prefixLen;
      memcpy(&pDest->prefix, &pPrefix->u.prefix6, sizeof(struct in6_addr));
    }
    else
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR,
              "nnCnvPrefix6TtoPrefixT Wrong Address Family : Failure\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

      return FAILURE;
    }

    return SUCCESS;
}


/**
 * Description: Prefix6 구조체를 생성하여 리턴하는 함수
 *
 * @retval : PREFIX_ERR_PREFIX_ALLOC(-1) : Prefix 의
 *                                         메모리 할당에 실패할 때,
 *           생성된 Prefix6 구조체 : 성공
 *
 * @code
 *  Prefix6T *pTestPrefix6 = NULL;
 *
 *  pTestPrefix6 = nnPrefixIpv6New();
 * @endcode
 */

Prefix6T *nnPrefixIpv6New(void)
{
    Prefix6T *pPrefix6 = NULL;

    pPrefix6 = (Prefix6T *)nnPrefixNew();

    if ((Int32T)pPrefix6 <= 0 && (Int32T)pPrefix6 >= MEMMGR_ERR_ARGUMENT)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR,
              "NewPrefix6T Memory Allocate : Failure(%d)\n", (Int32T)pPrefix6);
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return (Prefix6T *)PREFIX_ERR_PREFIX_ALLOC;
    }

    pPrefix6->family = AF_INET6;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Init NewPrefix6T : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return pPrefix6;
}


/**
 * Description: Prefix6 구조체를 제거하는 함수
 *
 * @param [in] pPrefix6 : 제거할 Prefix6 구조체
 *
 * @retval : None
 *
 * @code
 *  nnPrefixIpv6Free(pPrefix6);
 * @endcode
 *
 * @test
 *  pPrefix6 의 값을 NULL 로 입력
 */

void nnPrefixIpv6Free(Prefix6T *pPrefix6)
{
    if (pPrefix6 == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    nnPrefixFree((PrefixT *)pPrefix6);

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Free Prefix6T Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}


/**
 * Description: Prefix6 구조체에 NetMask 를 적용하는 함수
 *
 * @param [in/out] pDest : Netmask 를 적용할 Prefix6 구조체
 * @param [in] pPrefixLen : Uint8T 형식의 PrefixLen 값
 *
 * @retval : None
 *
 * @code
 *  Prefix6T testPrefix6 = {0,};
 *  Uint8T prefixLen = 0;
 *
 *  prefixLen = 100;
 *  testPrefix6.prefix.s6_addr[0] = 0x11;
 *  ...
 *  testPrefix6.prefix.s6_addr[15] = 0x88;
 *
 *  nnApplyNetmasktoPrefix6(&testPrefix6, &prefixLen);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 *  pPrefixLen 의 값을 NULL 로 입력
 */

void nnApplyNetmasktoPrefix6(Prefix6T *pDest, Uint8T *pPrefixLen)
{
    Uint8T *pPnt = NULL;
    Int32T index = 0;
    Int32T offset = 0;

    if (pDest == NULL || pPrefixLen == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return;
    }

    index = *pPrefixLen / 8;

    if (index < 16)
    {
        pPnt = (Uint8T *) &pDest->prefix;
        offset = *pPrefixLen % 8;

        pPnt[index] &= gMaskbit[offset];
        index++;

        while (index < 16)
        {
            pPnt[index++] = 0;
        }
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Netmask Apply To Prefix6T Complete\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");
}


/**
 * Description: struct in6_addr 형식의 Netmask 를
 *              Uint8T 형식의 PrefixLen 으로 변환하는 함수
 *
 * @param [out] pDest : 결과를 담을 변수
 * @param [in] pNetmask : 변환할 struct in6_addr 형식의 Netmask
 *
 * @retval : PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @code
 *  Uint8T prefixLen = 0;
 *  struct in6_addr testAddr6 = {0,};
 *  Int8T ret = 0;
 *
 *  testAddr.s_addr = 0x00FFFFFF;
 *
 *  ret = nnCnvNetmasktoPrefixLen(&prefixLen, &testAddr);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 *  pNetmask 의 값을 NULL 로 입력
 */

Int8T nnCnvNetmask6toPrefixLen(Uint8T *pDest, struct in6_addr *pNetmask)
{
    Int32T len = 0;
    Uint8T val = 0;
    Uint8T *pPnt = NULL;
  
    if (pDest == NULL || pNetmask == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    pPnt = (Uint8T *)pNetmask;

    while ((*pPnt == 0xff) && len < 128)
    {
        len += 8;
        pPnt++;
    }

    if (len < 128)
    {
        val = *pPnt;
        while (val)
        {
            len++;
            val <<= 1;
        }
    }

    *pDest = len;

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_INFO, "Convert Netmask(IPv6) To PrefixLen : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}


/**
 * Description: Uint8T 형식의 PrefixLen 을
 *              struct in6_addr 형식의 Netmask 로 변환하는 함수
 *
 * @param [out] pDest : 결과를 담을 변수
 * @param [in] pPrefixLen : 변환할 Uint8T 형식의 PrefixLen
 *
 * @retval : PREFIX_ERR_NULL(-2) : 파라미터가 NULL 일 때,
 *           SUCCESS(0) : 성공
 *
 * @code
 *  struct in6_addr testAddr6 = {0,};
 *  Uint8T prefixLen = 0;
 *  Int8T ret = 0;
 *
 *  prefixLen = 100;
 *
 *  ret = nnCnvPrefixLentoNetmask6(&testAddr6, &prefixLen);
 * @endcode
 *
 * @test
 *  pDest 의 값을 NULL 로 입력,
 *  pPrefixLen 의 값을 NULL 로 입력
 */

Int8T nnCnvPrefixLentoNetmask6(struct in6_addr *pDest, Uint8T *pPrefixLen)
{
    Uint8T *pPnt = NULL;
    Int32T bit = 0;
    Int32T offset = 0;

    if (pDest == NULL || pPrefixLen == NULL)
    {
        NNLOGDEBUG(LOG_DEBUG, "################################\n");
        NNLOGDEBUG(LOG_ERR, "Parameter Must Not NULL\n");
        NNLOGDEBUG(LOG_DEBUG, "################################\n");

        return PREFIX_ERR_NULL;
    }

    memset (pDest, 0, sizeof (struct in6_addr));
    pPnt = (Uint8T *)pDest;

    offset = *pPrefixLen / 8;
    bit = *pPrefixLen % 8;

    while (offset--)
    {
        *pPnt++ = 0xff;
    }

    if (bit)
    {
        *pPnt = gMaskbit[bit];
    }

    NNLOGDEBUG(LOG_DEBUG, "################################\n");
    NNLOGDEBUG(LOG_ERR, "Convert PrefixLen To Netmask(IPv6) : Success\n");
    NNLOGDEBUG(LOG_DEBUG, "################################\n");

    return SUCCESS;
}
#endif /* HAVE_IPV6 */



#if 0
/*******************************************************************************
 * Function: <nnNewPrefixfromAddr>
 *
 * Description: <Get new PrefixT from struct in_addr address(IPV4)>
 *
 * Parameters: pAddr : struct in_addr address(IPV4)
 *             pPrefixLen : Prefix length
 *
 * Returns: PREFIX_ERR_PREFIX_ALLOC(-1) : prefix malloc fail
 *          PREFIX_ERR_NULL(-2) : pAddr is null
 *          new PrefixT
*******************************************************************************/

PrefixT *nnNewPrefixfromAddr(struct in_addr *pAddr, Uint8T *pPrefixLen)
{
    PrefixT *pNewPrefix = NULL;

    if (pAddr == NULL || pPrefixLen == NULL)
    {
        LOG("Parameter Must Not NULL");
        return (PrefixT *)PREFIX_ERR_NULL;
    }

    pNewPrefix = malloc(sizeof(PrefixT));

    if ((Int32T)pNewPrefix <= 0)
    {
        LOG("NewPrefix's Malloc is Failed");
        return (PrefixT *)PREFIX_ERR_PREFIX_ALLOC;
    }

    memset(pNewPrefix, 0, sizeof(PrefixT));
    pNewPrefix->family = AF_INET;
    pNewPrefix->prefixLen = *pPrefixLen;
    pNewPrefix->u.prefix4 = *pAddr;

    return pNewPrefix;
}


#ifdef HAVE_IPV6
/*******************************************************************************
 * Function: <nnNewPrefixfromAddr6>
 *
 * Description: <Get new PrefixT from struct in6_addr address(IPV6)>
 *
 * Parameters: pAddr6 : struct in6_addr address(IPV6)
 *             pPrefixLen : Prefix length
 *
 * Returns: PREFIX_ERR_PREFIX_ALLOC(-1) : prefix malloc fail
 *          PREFIX_ERR_NULL(-2) : pAddr6 is null
 *          new PrefixT
*******************************************************************************/

PrefixT *nnNewPrefixfromAddr6(struct in6_addr *pAddr6, Uint8T *pPrefixLen)
{
    PrefixT *pNewPrefix = NULL;

    if (pAddr6 == NULL || pPrefixLen == NULL)
    {
        LOG("Parameter Must Not NULL");
        return (PrefixT *)PREFIX_ERR_NULL;
    }

    pNewPrefix = malloc(sizeof(PrefixT));

    if ((Int32T)pNewPrefix <= 0)
    {
        LOG("NewPrefix's Malloc is Failed");
        return (PrefixT *)PREFIX_ERR_PREFIX_ALLOC;
    }

    memset(pNewPrefix, 0, sizeof(PrefixT));
    pNewPrefix->family = AF_INET6;
    pNewPrefix->prefixLen = *pPrefixLen;
    pNewPrefix->u.prefix6 = *pAddr6;

    return pNewPrefix;
}
#endif /* HAVE_IPV6 */


/*******************************************************************************
 * Function: <nnNewPrefixfromPrefix4>
 *
 * Description: <Get new PrefixT from struct Prefix4T>
 *
 * Parameters: pPrefix4 : struct Prefix4T
 *             pPrefixLen : Prefix length
 *
 * Returns: PREFIX_ERR_PREFIX_ALLOC(-1) : prefix malloc fail
 *          PREFIX_ERR_NULL(-2) : pPrefix4 is null
 *          new PrefixT
*******************************************************************************/

PrefixT *nnNewPrefixfromPrefix4(Prefix4T *pPrefix4, Uint8T *pPrefixLen)
{
    PrefixT *pNewPrefix = NULL;

    if (pPrefix4 == NULL || pPrefixLen == NULL)
    {
        LOG("Parameter Must Not NULL");
        return (PrefixT *)PREFIX_ERR_NULL;
    }

    pNewPrefix = malloc(sizeof(PrefixT));

    if ((Int32T)pNewPrefix <= 0)
    {
        LOG("NewPrefix's Malloc is Failed");
        return (PrefixT *)PREFIX_ERR_PREFIX_ALLOC;
    }

    memset(pNewPrefix, 0, sizeof(PrefixT));
    pNewPrefix->family = pPrefix4->family;
    pNewPrefix->prefixLen = *pPrefixLen;
    pNewPrefix->u.prefix4 = pPrefix4->prefix;

    return pNewPrefix;
}


#ifdef HAVE_IPV6
/*******************************************************************************
 * Function: <nnNewPrefixfromPrefix6>
 *
 * Description: <Get new PrefixT from struct Prefix6T>
 *
 * Parameters: pPrefix6 : struct Prefix6T
 *             pPrefixLen : Prefix length
 *
 * Returns: PREFIX_ERR_PREFIX_ALLOC(-1) : prefix malloc fail
 *          PREFIX_ERR_NULL(-2) : pPrefix6 is null
 *          new PrefixT
*******************************************************************************/

PrefixT *nnNewPrefixfromPrefix6(Prefix6T *pPrefix6, Uint8T *pPrefixLen)
{
    PrefixT *pNewPrefix = NULL;

    if (pPrefix6 == NULL || pPrefixLen == NULL)
    {
        LOG("Parameter Must Not NULL");
        return (PrefixT *)PREFIX_ERR_NULL;
    }

    pNewPrefix = malloc(sizeof(PrefixT));

    if ((Int32T)pNewPrefix <= 0)
    {
        LOG("NewPrefix's Malloc is Failed");
        return (PrefixT *)PREFIX_ERR_PREFIX_ALLOC;
    }

    memset(pNewPrefix, 0, sizeof(PrefixT));
    pNewPrefix->family = pPrefix6->family;
    pNewPrefix->prefixLen = *pPrefixLen;
    pNewPrefix->u.prefix6 = pPrefix6->prefix;

    return pNewPrefix;
}
#endif /* HAVE_IPV6 */


/*******************************************************************************
 * Function: <nnNewPrefixfromString>
 *
 * Description: <Get new PrefixT from StringT type address(IPV4 or IPV4_PREFIX)>
 *
 * Parameters: strAddr : StringT type Address(IPV4 or IPV4_PREFIX)
 *             pPrefixLen : Prefix length
 *
 * Returns: FAILURE(-1) : Fail operation
 *          PREFIX_ERR_PREFIX_ALLOC(-1) : prefix malloc fail
 *          PREFIX_ERR_NULL(-2) : strAddr is null
 *          new PrefixT
*******************************************************************************/

PrefixT *nnNewPrefixfromString(StringT strAddr, Uint8T *pPrefixLen)
{
    PrefixT *pNewPrefix = NULL;
    Int32T ret = 0;

    if (strAddr == NULL || pPrefixLen == NULL)
    {
        LOG("Parameter Must Not NULL");
        return (PrefixT *)PREFIX_ERR_NULL;
    }

    pNewPrefix = malloc(sizeof(PrefixT));

    if ((Int32T)pNewPrefix <= 0)
    {
        LOG("NewPrefix's Malloc is Failed");
        return (PrefixT *)PREFIX_ERR_PREFIX_ALLOC;
    }

    memset(pNewPrefix, 0, sizeof(PrefixT));


    ret = nnCnvStringtoPrefix(pNewPrefix, strAddr);

    if (ret != SUCCESS)
    {
        return (PrefixT *)ret;
    }

    pNewPrefix->prefixLen = *pPrefixLen;

    return pNewPrefix;

}


/*******************************************************************************
 * Function: <nnNewPrefixfromUint>
 *
 * Description: <Get new PrefixT from Uint32T type address(IPV4)>
 *
 * Parameters: strAddr : Uint32T type Address(IPV4)
 *             pPrefixLen : Prefix length
 *
 * Returns: FAILURE(-1) : Fail operation
 *          PREFIX_ERR_PREFIX_ALLOC(-1) : prefix malloc fail
 *          PREFIX_ERR_NULL(-2) : strAddr is null
 *          new PrefixT
*******************************************************************************/

Prefix4T *nnNewPrefix4fromUint(Uint32T *pUintAddr, Uint8T *pPrefixLen)
{
    Prefix4T *pNewPrefix4 = NULL;
    struct in_addr tempAddr = {0,};

    if (pUintAddr == NULL || pPrefixLen == NULL)
    {
        LOG("Parameter Must Not NULL");
        return (Prefix4T *)PREFIX_ERR_NULL;
    }

    pNewPrefix4 = malloc(sizeof(Prefix4T));

    if ((Int32T)pNewPrefix4 <= 0)
    {
        LOG("NewPrefix4's Malloc is Failed");
        return (Prefix4T *)PREFIX_ERR_PREFIX_ALLOC;
    }

    tempAddr.s_addr = htonl(*pUintAddr);

    memset(pNewPrefix4, 0, sizeof(Prefix4T));
    pNewPrefix4->family = AF_INET;
    pNewPrefix4->prefixLen = *pPrefixLen;
    pNewPrefix4->prefix = tempAddr;

    return pNewPrefix4;
}
#endif /* 0 */
