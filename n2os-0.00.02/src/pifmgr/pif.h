#if !defined(_pif_h)
#define _pif_h
/*345678901234567890123456789012345678901234567890123456789012345678901234567890*/
/*********************************************************************************
 *               Electronics and Telecommunications Research Institute
 * Filename : <myFileName>
 * Blockname: <PIF Manager>
 * Overview : <PIF Manager S/W block manages Port/Interface & L2 MAC/VLAN>
 * Creator  : <Seungwoo Hong>
 * Owner    : <Seungwoo Hong>
 * Copyright: 2013 Electronics and Telecommunications Research Institute. 
 *            All rights reserved. No part of this software shall be reproduced, 
 *            stored in a retrieval system, or transmitted by any means, 
 *            electronic, mechanical, photocopying, recording, or otherwise, 
 *            without written permission from ETRI.
 *********************************************************************************/
/*********************************************************************************
 *                        VERSION CONTROL  INFORMATION
 * $Author$
 * $Date$
 * $Revision
 * $Log$ 
 *********************************************************************************/
#include "nosLib.h"

#define pifLOG(level, format, ...) printf("[%s] %s: " format, __FILE__, __FUNCTION__, ##__VA_ARGS__)

/* PIF Data Manager */
#define PIF_PORT_MGR				0
#define PIF_VLAN_MGR				1
#define PIF_AGG_MGR					2
#define PIF_IPIF_MGR				3

/* PIF Return Value */
#define PIF_RTN_OK					0
#define PIF_RTN_FAIL				1
#define PIF_RTN_BAD_ATTR			2
#define PIF_RTN_ALREADY_EXIST		3
#define PIF_RTN_NOT_EXIST			4
#define PIF_RTN_INTERNAL_ERROR		5
#define PIF_RTN_UNKNOWN_ERROR		6
#define PIF_RTN_ALREADY_SET			7


StringT getIidStr(Int32T id);
Int32T getIid(Int32T shelf, Int32T slot, Int32T port, Int32T type, Int32T id);
Int32T getIidbyName(StringT name);
Int32T getIidType(Int32T iid);
Int32T getIidLogicalId(Int32T iid);
Int32T getIidPhysicalId(Int32T iid);

#endif   /* _portData_h */

