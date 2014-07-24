#if !defined(_ribmgrRedistribute_h)
#define _ribmgrRedistribute_h

/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the redistribute & interface event related definitions.
 *  - Block Name : RIB Manager
 *  - Process Name : ribmgr
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : ribmgrRedistribute.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include "nnTypes.h"
#include "nnIf.h"
#include "nnBuffer.h"

#include "ribmgrRib.h"


extern void 
redistributeAdd (PrefixT *, RibT *);

extern void 
redistributeDelete (PrefixT *, RibT *);

extern void 
ribmgrRedistributeAdd (nnBufferT *);

extern void 
ribmgrRedistributeDelete (nnBufferT *);

extern void 
ribmgrRedistributeDefaultAdd (nnBufferT *);

extern void 
ribmgrRedistributeDefaultDelete (nnBufferT *);

/**
 * Description : 인터페이스가 추가 되는 경우 각 프로토콜로 통지하는 함수.
 *
 * @param [in] pIf : 인터페이스 자료구조 포인터
 *
 * @see InterfaceUpdate() function
 */
extern void 
ribmgrInterfaceAddUpdate (InterfaceT * pIf);


/**
 * Description : 인터페이스가 삭제 되는 경우 각 프로토콜로 통지하는 함수.
 *
 * @param [in] pIf : 인터페이스 자료구조 포인터
 *
 * @see InterfaceUpdate() function
 */
extern void 
ribmgrInterfaceDeleteUpdate (InterfaceT * pIf);


/**
 * Description : 인터페이스 상태가 UP 되는 경우 각 프로토콜로 통지하는 함수.
 *
 * @param [in] pIf : 인터페이스 자료구조 포인터
 *
 * @see InterfaceStatusUpdate() function
 */
extern void 
ribmgrInterfaceUpUpdate (InterfaceT * pIf);


/**
 * Description : 인터페이스 상태가 DOWN 되는 경우 각 프로토콜로 통지하는 함수.
 *
 * @param [in] pIf : 인터페이스 자료구조 포인터
 *
 * @see InterfaceStatusUpdate() function
 */
extern void 
ribmgrInterfaceDownUpdate (InterfaceT * pIf);



/**
 * Description : 인터페이스 주소가 추가되는 경우 각 프로토콜로 통지하는 함수.
 *
 * @param [in] pIf : 인터페이스 자료구조 포인터
 * @param [in] pIfc : 커넥티드 자료구조 포인터
 *
 * @see InterfaceAddressUpdate() function
 */
extern void 
ribmgrInterfaceAddressAddUpdate (InterfaceT *pIf, ConnectedT *pIfc);


/**
 * Description : 인터페이스 주소가 삭제되는 경우 각 프로토콜로 통지하는 함수.
 *
 * @param [in] pIf : 인터페이스 자료구조 포인터
 * @param [in] pIfc : 커넥티드 자료구조 포인터
 *
 * @see InterfaceAddressUpdate() function
 */
extern void 
ribmgrInterfaceAddressDeleteUpdate (InterfaceT * pIf, ConnectedT * pIfc);

#endif /* _ribmgrRedistribute_h */

