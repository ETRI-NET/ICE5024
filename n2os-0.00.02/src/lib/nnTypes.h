#if !defined(_nnTypes_h)
#define _nnTypes_h

/*******************************************************************************
 *            Electronics and Telecommunications Research Institute
 *                      (Co-developer of ARCHYS Co., Ltd.)
 *
 * Filename: <nnTypes.h>
 *
 * Blockname: <N2OS Common Library : N2OS Types>
 *
 * Overview: <N2OS Defined Data Types>
 *
 * Creator: <Changwoo Lee, JaeSu Han>
 *
 * Owner: <Changwoo Lee, Jaesu Han> (The person to contact regarding this file)
 *
 * Copyright: 2013 Electronics and Telecommunications Research Institute.
 *           All rights reserved.
 *           No part of this software shall be reproduced,
 *           stored in a retrieval system, or transmitted by any means,
 *           electronic, mechanical, photocopying, recording,
 *           or otherwise, without written permission from ETRI.
 ******************************************************************************/


/*******************************************************************************
 *                        VERSION CONTROL  INFORMATION
 * $Author: hyryu $     Jaesu Han
 * $Date: 2014-01-17 18:11:34 +0900 (Fri, 17 Jan 2014) $       2013.10.07
 * $Revision    0.1
 * $Log$        Develop draft
 ******************************************************************************/


/*******************************************************************************
 *                        CONSTANTS / LITERALS / TYPES
 ******************************************************************************/

typedef enum {
	FALSE = 0,
	TRUE = 1
} BoolT;

typedef char Int8T; 
typedef short Int16T; 
typedef int Int32T;
typedef long long Int64T;

typedef unsigned char Uint8T;
typedef unsigned short Uint16T;
typedef unsigned int Uint32T;
typedef unsigned long long Uint64T;

typedef float FloatT;
typedef double DoubleT;

typedef char * StringT;

typedef Uint64T SizeT;

typedef Uint64T OffsetT;

typedef Uint64T TimeT;

#define SUCCESS 0
#define FAILURE -1

#define TIME_UNKNOWN 0x8000000000000000LL

#define TIME_END 0x7FFFFFFFFFFFFFFFLL
#define TIME_BEGAN 0x0LL

#define TIME_ONE_MICROSECOND			                1000LL
#define TIME_ONE_MILLISECOND				     1000000LL
#define TIME_ONE_SECOND 				  1000000000LL
#define TIME_ONE_MINUTE 			       	 60000000000LL
#define TIME_ONE_HOUR 			               3600000000000LL
#define TIME_ONE_DAY 				      86400000000000LL
#define TIME_MAX				              TIME_END

#define SUCCESS 0
#define FAILED -1

#define LOG(msg) printf("%s - %s\n", msg, __FUNCTION__);


/*******************************************************************************
*                                GLOBALS VARIABLES
*******************************************************************************/


#endif
