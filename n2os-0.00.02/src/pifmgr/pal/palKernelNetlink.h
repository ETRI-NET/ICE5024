#if !defined(_palKernelNetlink_h)
#define _palKernelNetlink_h

/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the Netlink related definitions.
 *  - Block Name : PIF Manager
 *  - Process Name : pifmgr
 *  - Creator : Suncheul Kim, modified by Seungwoo Hong
 *  - Initial Date : 2013/10/10 modifed 2014/03/17
 */

/**
 * @file : rtNetLink.h
 *
 * $Author:  $
 * $Date:  $
 * $Revision:  $
 * $LastChangedBy:  $
 */

#include "pal.h"

void netlinkkernelInit  (void);
void netlinkkernelStart (void);
void netlinkkernelStop  (void);

void netlinkkernelIfUpdate (void);
Int32T netlinkUpdateIfAll(void);

void netlinkInit (void);
Int32T netlinkAddressDeleteIpv4(InterfaceT *ifp, ConnectedT *ifc);
Int32T netlinkAddressAddIpv4(InterfaceT *ifp, ConnectedT *ifc);

//Int32T kernelAddressAddIpv4 (Uint32T ifIdx, PrefixT *p, Uint8T bcast, Uint8T second, char* label);
//Int32T kernelAddressDeleteIpv4 (Uint32T ifIdx, PrefixT *p, Uint8T bcast, Uint8T second, char* label);

#endif /* _palKernelNetlink_h */
