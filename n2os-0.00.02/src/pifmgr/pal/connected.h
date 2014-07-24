#if !defined(_connected_h)
#define _connected_h

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
 * @file : connected.h 
 *
 * $Author: sckim007 $
 * $Date: 2014-02-17 09:53:56 +0900 (Mon, 17 Feb 2014) $
 * $Revision: 860 $
 * $LastChangedBy: sckim007 $
 */

#include "nnTypes.h"
#include "nnPrefix.h"

ConnectedT * connectedCheck (InterfaceT * ifp, PrefixT * p);

void connectedAddIpv4 (InterfaceT *ifp, Int32T flags, 
				struct in_addr *addr, Uint8T prefixLen, struct in_addr *broad, const char * label);

void connectedDeleteIpv4 (InterfaceT *ifp, Int32T flags, 
				struct in_addr *addr, Uint8T prefixLen, struct in_addr *broad);

#ifdef HAVE_IPV6
void connectedAddIpv6 (InterfaceT *ifp, Int32T flags, 
		struct in6_addr *addr, Uint8T prefixLen, struct in6_addr *broad, const char * label);

void connectedDeleteIpv6 (InterfaceT *ifp, struct in6_addr *address, 
		Uint8T prefixLen, struct in6_addr *broad);

#endif /* HAVE_IPV6 */

#endif /*_connected_h */
