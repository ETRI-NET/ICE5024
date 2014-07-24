#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "nnTypes.h"
#include "nnRibDefines.h"
#include "nnPrefix.h"


#if !defined(_nnUtility_h)
#define _nnUtility_h

/**
 * @mainpage N2OS Common Library - Utility 관련
 * @section intro 소개
 *   - 블록이름 : Common Library - Utility 관련
 *   - Overview : Utility와 관련된 Common Library
 * @section CREATEINFO  생성자
 *   - 생성자 : Changwoo Lee, JaeSu Han (Co-developer of ARCHYS Co., Ltd.)
 * @section MODIFYINFO 변경 이력
 *   - Date : 2014.01.21
 */

/**
 * @brief Overview : N2OS Common Library - Utility 관련
 * @brief Creator: <Changwoo Lee, JaeSu Han>
 * @file : nnUtility.h
 *
 * $Id:
 * $Author:
 * $Date:
 * $Log$
 * $Revision:
 * $LastChangedBy:
 * $LastChanged$
 *
 *            Electronics and Telecommunications Research Institute
 * Copyright: 2013 Electronics and Telecommunications Research Institute.
 *            All rights reserved.
 *            No part of this software shall be reproduced, stored in a
 *            retrieval system, or transmitted by any means, electronic,
 *            mechanical, photocopying, recording, or otherwise, without
 *            written permission from ETRI.
 **/

/*******************************************************************************
 *                        CONSTANTS / LITERALS / TYPES
 ******************************************************************************/

// Flag manipulation macros
#define NN_CHECK_FLAG(V,F)      ((V) & (F))
#define NN_SET_FLAG(V,F)        (V) |= (F)
#define NN_UNSET_FLAG(V,F)      (V) &= ~(F)


// Type Entry
#define NN_DESC_ENTRY(T,S) [(T)]={(T),(S)}

typedef struct TypeDescTableT
{
    Uint32T type;
    const StringT string;

} TypeDescTableT;


typedef struct routeDescTable
{
    Uint32T type;
    const StringT string;
    Int8T chr;

} RouteDescTableT;



/*******************************************************************************
 *                           GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/

extern const TypeDescTableT gProcessTypes[];
void nnPrintHexf(StringT p, Int32T len, Int32T logPri);

const StringT nnRoute2String(u_int routeNum);
Int8T nnRoute2Char(u_int routeNum);

#endif
