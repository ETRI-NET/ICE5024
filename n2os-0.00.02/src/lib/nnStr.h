#include "nnTypes.h"

#if !defined(_nnstr_h)
#define _nnstr_h


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
 * @file : nnStr.h
 *
 * $Id: nnStr.h 829 2014-02-14 02:21:30Z lcw1127 $
 * $Author: lcw1127 $
 * $Date: 2014-02-14 11:21:30 +0900 (Fri, 14 Feb 2014) $
 * $Revision: 829 $
 * $LastChangedBy: lcw1127 $
 **/

/*******************************************************************************
 *                        CONSTANTS / LITERALS / TYPES
 ******************************************************************************/
/* IPV4 */
#define MIN_IPV4_PREFIX_LEN 1 /**< IPV4 최소 Prefix 값 길이 */
#define MAX_IPV4_PREFIX_LEN 2 /**< IPV4 최대 Prefix 값 길이 */
#define MAX_IPV4_PREFIX_NUM 32 /**< IPV4 최대 Prefix 값 */

#define MIN_IPV4_LEN 7 /**< IPV4 최소 길이 */
#define MAX_IPV4_LEN 15 /**< IPV4 최대 길이 */
#define MAX_IPV4_DOT_COUNT 3 /**< IPV4 최대 . 개수 */

/* IPV6 */
#define MIN_IPV6_PREFIX_LEN 1 /**< IPV6 최소 Prefix 값 길이 */
#define MAX_IPV6_PREFIX_LEN 3 /**< IPV6 최대 Prefix 값 길이 */
#define MAX_IPV6_PREFIX_NUM 128 /**< IPV6 최대 Prefix 값 */

#define MIN_IPV6_LEN 2 /**< IPV6 최소 길이 */
#define MAX_IPV6_LEN 39 /**< IPV6 최대 길이 */
#define MAX_IPV6_COLON_COUNT 7 /**< IPV6 최대 : 개수 */

/*******************************************************************************
 *                           GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/

Int8T nnStrGetAndToken (StringT remainStr, StringT tokenStr, StringT delim);
Int8T nnStrGetOrToken (StringT remainStr, StringT tokenStr, StringT delim);
Int32T nnStrGetTokenNum (StringT stringStr, StringT delim);
StringT nnStrDup (const StringT stringStr, Uint32T memType);
StringT nnStrnDup (const StringT stringStr, Int32T maxlen, Uint32T memType);
Int32T nnStrlCpy (StringT dstStr, const StringT srcStr, Int32T bufSize);
Int32T nnStrlCat (StringT dstStr, const StringT srcStr, Int32T bufSize);
Int32T nnStrnCat (StringT dstStr, Int32T bufSize, Int32T strCnt, ...);
Int32T nnSprintf (StringT stringStr, const StringT format, ...);
Int32T nnSprintfApp (StringT dstStr, Int32T bufSize, const StringT format, ...);

Int8T nnStrDupString(StringT str1, StringT str2);

Int8T nnStrCheckIPv4Type(StringT str);
Int8T nnStrCheckIPv4PrefixType(StringT str);
Int8T nnStrCheckIPv6Type(StringT str);
Int8T nnStrCheckIPv6PrefixType(StringT str);
Int8T nnStrCheckMacAddressType(StringT str);
Int8T nnStrCheckTime(StringT str);

Int32T nnCnvStringtoInt(const StringT source);
Int8T nnCnvInttoString(StringT dest, Int32T source);
Int32T nnCnvChartoInt(const Int8T source);
Int8T nnCnvInttoChar(Int8T *dest, Int32T source);

#if 0
/* later */
Int8T nnCnvStringtoTime(time_t *dest, struct tm *source);
struct tm *nnCnvTimetoString(time_t *source);
#endif

#endif
