/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the ribmgr interface handling definitions.
 *  - Block Name : RIP Protocol
 *  - Process Name : ripd
 *  - Creator : geontae park
 *  - Initial Date : 2014/02/19
 */

/**
 * @file : ribRibmgr.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_ripRibmgr_h)
#define _ripRibmgr_h

#include "nnBuffer.h"
#include "nnCmdCommon.h"

extern void ripRibmgrInit ();
extern void ripRibmgrClose ();
extern Int32T ripRedistributeCheck (Int32T);
extern void ripRibmgrIpv4Add (Prefix4T *, struct in_addr *, Uint32T, Uint8T);
extern void ripRibmgrIpv4Delete (Prefix4T *, struct in_addr *, Uint32T);
extern Int32T ripRibmgrReadIpv4 (Int32T,  nnBufferT *, Uint16T);
extern void ripRedistributeClean (void);
extern Int32T ripRibmgrRedistribute (struct cmsh *, Int32T, char *);
extern void ripRedistributeMetricSet (char *, Uint32T);
extern Int32T ripRedistributeMetricUnSet (char *, Uint32T);
extern Int32T configWriteRipRedistribute (struct cmsh *, Int32T);

#endif /* _ripRibmgr_h */
