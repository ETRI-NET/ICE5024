#if !defined(_ribmgrRouterid_h)
#define _ribmgrRouterid_h

/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the Router-ID related definitions.
 *  - Block Name : RIB Manager
 *  - Process Name : ribmgr
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : ribmgrRouterid.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include "nnRibDefines.h"
#include "nnPrefix.h"
#include "nnIf.h"

/**
 * Description : RouterID 이벤트 메시지 구성 함수.
 *
 * @param [in] apiBuff : 이벤트 메시지 버퍼
 * @param [in] p : Prefix4T 자료구조 포인터
 *
 * @retval : 이벤트 메시지 버퍼 길이
 */
extern Int32T 
buildRouterId(nnBufferT * apiBuff, Prefix4T * pPrefix4);

/*
 * Description : 인터페이스에서 주소를 RouterID에 설정하는 함수
 *
 * param [in] pIfc : ConnectedT *
 */
extern void 
routerIdAddAddress(ConnectedT * pIfc);


/*
 * Description : 인터페이스에서 주소를 RouterID에 삭제하는 함수
 *
 * param [in] pIfc : ConnectedT *
 */
extern void 
routerIdDelAddress(ConnectedT * pIfc);


/*
 * Description : 현재 설정된 RouterID값을 획득하는 함수.
 *
 * param [in] pPrefix4 : Prefix4T * 자료구조 포인터
 */
extern void 
routerIdGet(Prefix4T *pPrefix4);


/*
 * Description : 운용자에 의해 RouterID가 설정/삭제되는 경우에 호출되는 함수.
 *
 * param [in] pPrefix4 : Prefix4T * 자료구조포인터
 */
extern void 
routerIdSet(Prefix4T *pPrefix4);


/*
 * Description : RouterID의 자료구조를 초기화 하는 함수.
 */
extern void 
routerIdInit(void);


/*
 * Description : 
 */
extern void 
routerIdUpdate(void);


/*
 * Description : RouterID의 자료구조의 메모리를 해제하는 함수.
 */
extern void 
routerIdClose(void);
#endif /* _ribmgrRouterid_h */
