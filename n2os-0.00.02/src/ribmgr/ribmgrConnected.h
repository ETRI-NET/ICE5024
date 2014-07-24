#if !defined(_ribmgrConnected_h)
#define _ribmgrConnected_h

/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the connected route related definitions.
 *  - Block Name : RIB Manager
 *  - Process Name : ribmgr
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : ribmgrConnected.h 
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include "nnTypes.h"
#include "nnPrefix.h"

/*
 * Description : 동인한 인터페이스 주소가 존재 여부를 체크하는 함수.
 *
 * param [in] pIf : 인터페이스 자료구조 포인터
 * param [in] p : PrefixT 자료구조 포인터
 *
 * retval : 커넥티드 자료구조 포인터
 */
extern ConnectedT * 
connectedCheck (InterfaceT * pIf, PrefixT * pPrefix);


/*
 * Description : 인터페이스에 커넥티드 IPv4 주소 추가를 처리하는 함수.
 * netlinkInterfaceAddr() 로 부터 호출됨.
 *
 * param [in] pIf : 인터페이스 자료구조 포인터
 * param [in] flags : 인터페이스 플래그
 * param [in] pAddr : IPv4 주소 자료구조 포인터
 * param [in] prefixLen : IPv4 주소의 Prefix Length
 * param [in] pBroad : 인터페이스의 브로드캐스트 IPv4 주소 포인터
 * param [in] label : 인터페이스의 레이블 정보 문자열 포인터
 */
extern void
connectedAddIpv4 (InterfaceT *pIf, Int32T flags, struct in_addr *pAddr, Uint8T prefixLen, struct in_addr *pBroad, const StringT label);


/*
 * Description : 인터페이스에 커넥티드 IPv4 주소 및 루트를  삭제하는 함수.
 *
 * param [in] pIf : 인터페이스 자료구조 포인터
 * param [in] flags : 인터페이스 플래그
 * param [in] pAddr : IPv4 주소 자료구조 포인터
 * param [in] prefixLen : IPv4 주소의 Prefix Length
 * param [in] pBroad : 인터페이스의 브로드캐스트 IPv4 주소 포인터
 */
void
connectedDeleteIpv4 (InterfaceT *pIf, Int32T flags, struct in_addr *pAddr, Uint8T prefixLen, struct in_addr *pBroad);


/*
 * Description : 인터페이스의 커넥티드 IPv4 주소가 UP 되는 경우 처리하는 함수.
 *
 * param [in] pIf : 인터페이스 자료구조 포인터
 * param [in] pIfc : 커넥티드루트 자료구조 포인터
 */
extern void 
connectedUpIpv4 (InterfaceT * pIf, ConnectedT * pIfc);


/*
 * Description : 인터페이스에 커넥티드 IPv4 주소를 삭제하고, 라우팅 테이블로 
 * 부터 CONNECTED_TYPE 루트를 삭제하는 함수.
 *
 * param [in] pIf : 인터페이스 자료구조 포인터
 * param [in] pIfc : 커넥티드루트 자료구조 포인터
 */
extern void 
connectedDownIpv4 (InterfaceT * pIf, ConnectedT * pIfc);

#ifdef HAVE_IPV6

/*
 * Description : 인터페이스에 커넥티드 IPv6 주소 및 루트를 추가하는 함수.
 *
 * param [in] pIf : 인터페이스 자료구조 포인터
 * param [in] flags : 인터페이스 플래그
 * param [in] pAddr : IPv6 주소 자료구조 포인터
 * param [in] prefixLen : IPv6 주소의 프리픽스 길이
 * param [in] pBroad : 인터페이스의 브로드캐스트 IPv6 주소 포인터
 * param [in] label : 인터페이스의 레이블 정보 문자열 포인터
 */
extern void
connectedAddIpv6 (InterfaceT *pIf, Int32T flags, struct in6_addr *pAddr, Uint8T prefixLen, struct in6_addr *pBroad, const StringT label);


/*
 * Description : 인터페이스에 커넥티드 IPv6 주소 및 루트를 삭제하는 함수.
 *
 * param [in] pIf : 인터페이스 자료구조 포인터
 * param [in] pAddr : IPv6 주소 자료구조 포인터
 * param [in] prefixLen : IPv4 주소의 Prefix Length
 * param [in] pBroad : 인터페이스의 브로드캐스트 IPv6 주소 포인터
 */
extern void
connectedDeleteIpv6 (InterfaceT *pIf, struct in6_addr *pAddr, Uint8T prefixLen, struct in6_addr *pBroad);


/*
 * Description : 인터페이스에 커넥티드 IPv6 주소 및 루트를 추가하는 함수.
 *
 * param [in] pIf : 인터페이스 자료구조 포인터
 * param [in] pIfc : 커넥티드루트 자료구조 포인터
 */
extern void 
connectedUpIpv6 (InterfaceT * pIf, ConnectedT * pIfc);


/*
 * Description : 인터페이스에 커넥티드 IPv6 주소 및 루트를 삭제하는 함수.
 *
 * param [in] pIf : 인터페이스 자료구조 포인터
 * param [in] pIfc : 커넥티드루트 자료구조 포인터
 */
extern void 
connectedDownIpv6 (InterfaceT * pIf, ConnectedT * pIfc);

#endif /* HAVE_IPV6 */

#endif /*_ribmgrConnected_h */
