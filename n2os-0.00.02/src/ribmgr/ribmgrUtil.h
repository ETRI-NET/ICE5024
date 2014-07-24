#if !defined(_ribmgrUtil_h)
#define _ribmgrUtil_h

/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the utility related definitions.
 *  - Block Name : RIB Manager
 *  - Process Name : ribmgr
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : ribmgrUtil.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include "nnRibDefines.h"
#include "nnTypes.h"
#include "nnPrefix.h"

/*
 * Description : 루트 타입에 따른 문자열값을 제공하는 함수.
 *
 * param [in] routeNum : 루트타입
 *
 * retval : ribRouteDesTable Entry에서 루트를 나타내는 문자열
 */
extern const StringT 
ribRoute2String(Uint32T routeNum);


/*
 * Description : 루트 타입에 따른 문자값을 제공하는 함수.
 *
 * param [in] routeNum : 루트타입
 *
 * retval : ribRouteDesTable Entry에서 루트를 나타내는 문자
 */
extern Int8T 
ribRoute2Char(Uint32T routeNum);


/*
 * Description : PrefixT 값이 정상적인 값인지를 확인하는 함수
 *
 * param [in] pPrefix : PrefixT 포인터
 *
 * retval : 0 if 비정상
 *          1 if 정상
 */
extern Int32T 
ribCheckAddr (PrefixT *pPrefix);


/*
 * Description : Prefix4T 값이 정상적인 값인지를 확인하는 함수
 *
 * param [in] p : Prefix4T 포인터
 *
 * retval : 0 if 비정상
 *          1 if 정상
 */
extern Int32T 
ribCheckAddr4 (Prefix4T * pPrefix);

#ifdef HAVE_IPV6
/*
 * Description : Ipv6 
 *
 * param [in] addr : Ipv6 자료구조
 *
 * retval : 문자열 포인터
 */
const StringT
inet6_ntoa (struct in6_addr addr);
#endif

/*
 * Description : IP Forwarding 기능의 설정여부를 확인하는 함수.
 *
 * retval : 0 if 비활성화 됨
 *          1 if 활성화 됨
 */
extern Int32T
ipForward (void);

/*
 * Description : IP Forwarding 기능을 활성화 하는 함수.
 *
 * retval : 0 if 비활성화 됨
 *          1 if 활성화 됨
 */
extern Int32T
ipForwardEnable (void);

/*
 * Description : IP Forwarding 기능을 비활성화 하는 함수.
 *
 * retval : 0 if 비활성화 됨
 *          1 if 활성화 됨
 */
extern Int32T
ipForwardDisable (void);

#ifdef HAVE_IPV6
/*
 * Description : IP Forwarding 기능의 설정여부를 확인하는 함수.
 *
 * retval : 0 if 비활성화 됨
 *          1 if 활성화 됨
 */
extern Int32T
ipForwardIpv6 (void);

/*
 * Description : IP Forwarding 기능을 활성화 하는 함수.
 *
 * retval : 0 if 비활성화 됨
 *          1 if 활성화 됨
 */
extern Int32T
ipForwardIpv6Enable (void);

/*
 * Description : IP Forwarding 기능을 비활성화 하는 함수.
 *
 * retval : 0 if 비활성화 됨
 *          1 if 활성화 됨
 */
extern Int32T
ipForwardIpv6Disable (void);
#endif

#endif /* _ribmgrUtil_h */
