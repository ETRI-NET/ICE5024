#if !defined(_nnPrefix_h)
#define _nnPrefix_h

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
 * @brief : This file defines the Prefix related definitions.
 *  - Block Name : Library
 *  - Process Name : rLibraryibmgr
 *  - Creator : Changwoo Lee, JaeSu Han
 *  - Initial Date : 2013/12/17
 */

/**
 * @file : nnPrefix.h
 *
 * $Id: nnPrefix.h 1051 2014-03-21 08:17:41Z sckim007 $
 * $Author: sckim007 $
 * $Date: 2014-03-21 17:17:41 +0900 (Fri, 21 Mar 2014) $
 * $Revision: 1051 $
 * $LastChangedBy: sckim007 $
 */
#include <stdio.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "nnTypes.h"



/*******************************************************************************
 *                        CONSTANTS / LITERALS / TYPES
 ******************************************************************************/

#define PREFIX_ERR_PREFIX_ALLOC	-1 /**< Prefix 의 메모리 할당에 실패할 때 */
#define PREFIX_ERR_NULL		-2 /**< 파라미터가 NULL 일 때 */

#define NN_IPV4_MAX_BITLEN 32    /**< IPv4 의 Bit 수 */
#define NN_IPV4_MAX_PREFIXLEN 32 /**< IPv4 의 최대 PrefixLen 값 */

#define PREFIX_PNBBY 8 /**< IPv4, 각 IP 자리의 Bit 수 */

/* Max bit/byte length of IPv4 address. */
#define PREFIX_IPV4_MAX_BYTELEN    4 /**< IPv4 의 Byte 길이 */
#define PREFIX_IPV4_MAX_BITLEN    32 /**< IPv4 의 Bit 길이 */
#define PREFIX_IPV4_MAX_PREFIXLEN 32 /**< IPv4 의 최대 PrefixLen 값 */

#ifdef HAVE_IPV6
/* Max bit/byte length of IPv6 address. */
#define PREFIX_IPV6_MAX_BYTELEN    16 /**< IPv6 의 Byte 길이 */
#define PREFIX_IPV6_MAX_BITLEN    128 /**< IPv6 의 Bit 길이 */
#define PREFIX_IPV6_MAX_PREFIXLEN 128 /**< IPv6 의 최대 PrefixLen 값 */
#endif

typedef struct PrefixT /**< Prefix 구조체 */
{
    Uint8T family;    /**< AF_INET, AF_INET6 의 Address Family */
    Uint8T prefixLen; /**< PrefixLen 값 */

    union /**< IPv4, IPv6 형식의 IP 를 저장할 공용체 */
    {
        Uint8T prefix;  /**< IPv4, IPv6 캐스팅 시 사용 */
        struct in_addr prefix4;    /**< IPv4 형식의 IP */
#ifdef HAVE_IPV6
	struct in6_addr prefix6;   /**< IPv6 형식의 IP */
#endif /* HAVE_IPV6 */
    } u __attribute__ ((aligned (8)));
} PrefixT;

typedef struct Prefix4T /**< IPv4 형식을 저장할 Prefix 구조체 */
{
    Uint8T family;             /**< AF_INET 의 Address Family */
    Uint8T prefixLen;          /**< IPv4 PrefixLen 값 */
    struct in_addr
      prefix __attribute__ ((aligned (8)));     /**< IPv4 형식의 IP */
} Prefix4T;

typedef struct Prefix6T /**< IPv6 형식을 저장할 Prefix 구조체 */
{
    Uint8T family;             /**< AF_INET6 의 Address Family */
    Uint8T prefixLen;          /**< IPv6 PrefixLen 값 */
    struct in6_addr
      prefix __attribute__ ((aligned (8)));    /**< IPv6 형식의 IP */
} Prefix6T;

/*******************************************************************************
*                                GLOBALS VARIABLES
*******************************************************************************/


/*******************************************************************************
 *                           GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/
#define PREFIX_IPV4_ADDR_CMP(A, B) \
        memcmp ((A), (B), PREFIX_IPV4_MAX_BYTELEN)

#define PREFIX_IPV4_ADDR_SAME(A, B) \
        (memcmp ((A), (B), PREFIX_IPV4_MAX_BYTELEN) == 0)

#define PREFIX_IPV4_ADDR_COPY(A, B) \
        memcpy ((A), (B), PREFIX_IPV4_MAX_BYTELEN)

#define PREFIX_IPV4_NET0(A) \
        ((((u_int32_t) (A)) & 0xff000000) == 0x00000000)

#define PREFIX_IPV4_NET127(A) \
        ((((u_int32_t) (A)) & 0xff000000) == 0x7f000000)

#define PREFIX_IPV4_LINKLOCAL(A) \
        ((((u_int32_t) (A)) & 0xffff0000) == 0xa9fe0000)

#define PREFIX_MASKBIT(OFFSET) \
        ((0xff << (PREFIX_PNBBY - (OFFSET))) & 0xff)

/* Count prefix size from mask length */
#define PREFIX_PSIZE(A) (((A) + 7) / (8))

/* Prefix's family member. */
#define PREFIX_FAMILY(P)  ((P)->family)

#define PREFIX_COPY_IPV4(D, S)  \
    *((Prefix4T *)(D)) = *((const Prefix4T *)(S));

Int8T nnCnvStringtoAddr(struct in_addr *pDest, StringT strAddr);
Int8T nnCnvAddrtoString(StringT dest, struct in_addr *pAddr);

Int8T nnCnvStringtoPrefix4(Prefix4T *pDest, StringT strAddr);
Int8T nnCnvPrefix4toString(StringT dest, Prefix4T *pPrefix4);

Int8T nnCnvStringtoPrefix(PrefixT *pDest, StringT strAddr);
Int8T nnCnvPrefixtoString(StringT dest, PrefixT *pPrefix);

Int8T nnCnvPrefixLentoNetmask(struct in_addr *pDest, Uint8T *pPrefixLen);
Int8T nnCnvNetmasktoPrefixLen(Uint8T *pDest, struct in_addr *pNetMask);

void nnCnvMasklentoIp (Int32T, struct in_addr *);
Uint8T nnCnvIpmasktoLen (struct in_addr);


Int8T nnCnvAddrtoUint(Uint32T *pDest, struct in_addr *pAddr);

Int8T nnGetBroadcastfromAddr(
    struct in_addr *pDest,
    struct in_addr *pAddr4,
    Uint8T *pPrefixLen);

Int8T nnGetBroadcastfromPrefix(struct in_addr *pDest, PrefixT *pPrefix);

Int8T nnGetBroadcastfromString(
    struct in_addr *pDest,
    StringT strAddr,
    Uint8T *pPrefixLen);
Int8T nnGetBroadcastfromUint(
    struct in_addr *pDest,
    Uint32T *pUintAddr,
    Uint8T *pPrefixLen);

Int8T nnCnvPrefix4TtoPrefixT(PrefixT *pDest, Prefix4T *pPrefix4);
Int8T nnCnvPrefixTtoPrefix4T(Prefix4T *pDest, PrefixT *pPrefix);

Int32T nnCnvAfitoFamily(Int32T afi);
Int32T nnCnvFamilytoAfi(Int32T family);
Int8T nnPrefixMatch(const PrefixT *pPrefix1, const PrefixT *pPrefix2);
Int8T nnPrefixCopy(PrefixT *pDest, const PrefixT *pSrc);
Int8T nnPrefixSame(const PrefixT *pPrefix1, const PrefixT *pPrefix2);
Int8T nnPrefixCmp(const PrefixT *pPrefix1, const PrefixT *pPrefix2);
StringT nnCnvPrefixFamilytoString(const PrefixT *pPrefix);
Prefix4T *nnPrefixIpv4New(void);
void nnPrefixIpv4Free(Prefix4T *pPrefix4);

void nnApplyNetmasktoPrefix4(Prefix4T *pDest, Uint8T *pPrefixLen);
Int8T nnPrefixCheckIpv4Any(const Prefix4T *);

void nnApplyNetmasktoPrefix(PrefixT *pDest, Uint8T *pPrefixLen);
Int8T nnGetPrefixByteLen(const PrefixT *pPrefix);
PrefixT *nnPrefixNew(void);
void nnPrefixFree(PrefixT *pPrefix);
Int8T nnCheckAllDigit(const StringT str);
Int8T nnApplyClassfulMaskIpv4(Prefix4T *pDest);
Int8T nnApplyPrefixLentoAddr(
    struct in_addr *pDest,
    struct in_addr *pAddr4,
    Uint8T *pPrefixLen);

Int8T nnCnvNetmaskstrtoPrefixstr(
    StringT destStr,
    const StringT netStr,
    const StringT maskStr);

/* Check bit of the prefix. */
static inline Uint32T
nnGetPrefixBit(const Uint8T *pPrefix, const Uint8T *pPrefixLen)
{
    Uint32T offset = 0;
    Uint32T shift = 0;

    if (pPrefix == NULL || pPrefixLen == NULL)
    {
        return PREFIX_ERR_NULL;
    }

    offset = *pPrefixLen / 8;
    shift = 7 - (*pPrefixLen % 8);

    return (pPrefix[offset] >> shift) & 1;
}


#ifdef HAVE_IPV6
/* Max bit/byte length of IPv6 address. */
#define PREFIX_IPV6_ADDR_CMP(A, B) \
        memcmp ((A), (B), PREFIX_IPV6_MAX_BYTELEN)

#define PREFIX_IPV6_ADDR_SAME(A, B) \
        (memcmp ((A), (B), PREFIX_IPV6_MAX_BYTELEN) == 0)

#define PREFIX_IPV6_ADDR_COPY(D, S) \
        memcpy ((D), (S), PREFIX_IPV6_MAX_BYTELEN)

#define PREFIX_COPY_IPV6(D, S)  \
    *((Prefix6T *)(D)) = *((const Prefix6T *)(S));

Int8T nnCnvStringtoAddr6(struct in6_addr *pDest, StringT strAddr);
Int8T nnCnvAddr6toString(StringT dest, struct in6_addr *pAddr);

Int8T nnCnvStringtoPrefix6(Prefix6T *pDest, StringT strAddr);
Int8T nnCnvPrefix6toString(StringT dest, Prefix6T *pPrefix6);

Int8T nnCnvPrefix6TtoPrefixT(PrefixT *pDest, Prefix6T *pPrefix6);
Int8T nnCnvPrefixTtoPrefix6T(Prefix6T *pDest, PrefixT *pPrefix);

Prefix6T *nnPrefixIpv6New(void);
void nnPrefixIpv6Free(Prefix6T *pPrefix6);
void nnApplyNetmasktoPrefix6(Prefix6T *pDest, Uint8T *pPrefixLen);
Int8T nnCnvNetmask6toPrefixLen(Uint8T *pDest, struct in6_addr *pNetmask);

Int8T nnCnvPrefixLentoNetmask6(struct in6_addr *pDest, Uint8T *pPrefixLen);

static inline Uint32T
nnGetPrefix6Bit(const struct in6_addr *pPrefix, const Uint8T *pPrefixLen)
{
    return nnGetPrefixBit((const Uint8T *)&pPrefix->s6_addr, pPrefixLen);
}

#endif //HAVE_IPV6


#if 0
/* Later */
PrefixT *nnNewPrefixfromAddr(struct in_addr *pAddr, Uint8T *pPrefixLen);

/* #ifdef HAVE_IPV6*/
PrefixT *nnNewPrefixfromAddr6(struct in6_addr *pAddr6, Uint8T *pPrefixLen);

PrefixT *nnNewPrefixfromPrefix4(Prefix4T *pPrefix4, Uint8T *pPrefixLen);

/* #ifdef HAVE_IPV6*/
PrefixT *nnNewPrefixfromPrefix6(Prefix6T *pPrefix6, Uint8T *pPrefixLen);

PrefixT *nnNewPrefixfromString(StringT strAddr, Uint8T *pPrefixLen);
Prefix4T *nnNewPrefix4fromUint(Uint32T *pUintAddr, Uint8T *pPrefixLen);
#endif

/*
extern Int32T nnCnvStrtoPrefixIpv4 (const StringT, Prefix4T *);

extern Int32T nnCnvStrtoPrefixIpv6(const char *, Prefix6T *);
extern void nnCnvStringtoAddr6(const char *, struct in6_addr *);
extern const StringT nnCnvAddr6toString(struct in6_addr addr);

extern Int32T nnCnvStrtoPrefix (const StringT, PrefixT *);
extern Int32T nnCnvPrefixtoStr (const PrefixT *, StringT, Int32T);
*/

#endif
