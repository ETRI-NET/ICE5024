#if !defined(_ribmgrRt_h)
#define _ribmgrRt_h

/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the Netlink related definitions.
 *  - Block Name : RIB Manager
 *  - Process Name : ribmgr
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : ribmgrRt.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include "nnTypes.h"
#include "nnPrefix.h"
#include "nnIf.h"

#include "ribmgrRib.h"

/*
 * Description : RIB Manager의 초기화 시점에 Kernel로부터 인터페이스 정보를 
 * 요청하는 함수.
 *
 * retval : -1 if 실패
 *           0 if 성공
 */
extern Int32T 
interfaceLookupNetlink (void);


/*
 * Description : RIB Manager의 초기화 시점에 Kernel로부터 루트 정보를 
 * 요청하는 함수.
 * 
 * retval : -1 if 실패
 *           0 if 성공
 */
extern Int32T 
routeReadNetlink  (void);


/*
 * Description : Netlink 소켓을 이용하여 Kernel로 IPv4 루트 설정을 요청하는 함수.
 *
 * param [in] pPrefix : PrefixT 자료구조 포인터
 * param [in] pRib : rib 자료구조 포인터
 *
 * retval : -1 if 실패,
 *           0 if 성공
 */
extern Int32T 
kernelAddIpv4 (PrefixT * pPrefix, RibT * pRib);


/*
 * Description : Netlink 소켓을 이용하여 Kernel로 IPv4 루트 삭제를 요청하는 함수.
 *
 * param [in] pPrefix : PrefixT 자료구조 포인터
 * param [in] pRib : rib 자료구조 포인터
 *
 * retval : -1 if 실패,
 *           0 if 성공
 */
extern Int32T 
kernelDeleteIpv4 (PrefixT * pPrefix, RibT * pRib);


/*
 * Description : Netlink 소켓을 이용하여 Kernel로 IPv4 주소 설정을 요청하는 함수.
 *
 * param [in] pIf : 인터페이스 자료구조 포인터
 * param [in] pIfc : 커넥티드 자료구조 포인터
 *
 * retval : -1 if 실패,
 *           0 if 성공
 */
extern Int32T 
kernelAddressAddIpv4 (InterfaceT * pIf, ConnectedT * pIfc);


/*
 * Description : Netlink 소켓을 이용하여 Kernel로 IPv4 주소 삭제를 요청하는 함수.
 *
 * param [in] pIf : 인터페이스 자료구조 포인터
 * param [in] pIfc : 커넥티드 자료구조 포인터
 *
 * retval : -1 if 실패,
 *           0 if 성공
 */
extern Int32T 
kernelAddressDeleteIpv4 (InterfaceT * pIf, ConnectedT * pIfc);

#ifdef HAVE_IPV6
/*
 * Description : Netlink 소켓을 이용하여 Kernel로 IPv6 루트 설정을 요청하는 함수.
 *
 * param [in] p : PrefixT 자료구조 포인터
 * param [in] pRib : rib 자료구조 포인터
 *
 * retval : -1 if 실패,
 *           0 if 성공
 */
extern Int32T 
kernelAddIpv6 (PrefixT *, RibT *pRib);


/*
 * Description : Netlink 소켓을 이용하여 Kernel로 IPv6 루트 삭제를 요청하는 함수.
 *
 * param [in] p : PrefixT 자료구조 포인터
 * param [in] pRib : rib 자료구조 포인터
 *
 * retval : -1 if 실패,
 *           0 if 성공
 */
extern Int32T 
kernelDeleteIpv6 (PrefixT * p, RibT * pRib);


/*
 * Description : Netlink 소켓을 이용하여 Kernel로 IPv6 루트 삭제를 요청하는 함수.
 *
 * param [in] pDest : IP 목적지 주소
 * param [in] pGate : nexthop gateway 주소 
 * param [in] index : 인덱스
 * param [in] flags : 플래그
 * param [in] table : 루트 테이블 번호
 *
 * retval : -1 if 실패,
 *           0 if 성공
 */
extern Int32T 
kernelDeleteIpv6Old (Prefix6T * pDest, struct in6_addr * pGate, Uint32T index, Int32T flags, Int32T table);

#endif /* HAVE_IPV6 */

#endif /* _ribmgrRt_h */
