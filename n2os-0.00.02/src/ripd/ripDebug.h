/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the debug related definitions.
 *  - Block Name : RIP Protocol
 *  - Process Name : ripd
 *  - Creator : geontae park
 *  - Initial Date : 2014/02/19
 */

/**
 * @file : ribDebug.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_ripDebug_h)
#define _ripDebug_h

#include "nnCmdCmsh.h"

/* RIP debug event flags. */
#define RIP_DEBUG_EVENT          0x01

/* RIP debug packet flags. */
#define RIP_DEBUG_PACKET         0x01
#define RIP_DEBUG_SEND           0x02
#define RIP_DEBUG_SEND_DETAIL    0x04
#define RIP_DEBUG_RECV           0x08
#define RIP_DEBUG_RECV_DETAIL    0x10

/* RIP debug zebra flags. */
#define RIP_DEBUG_RIBMGR         0x01

/* Debug related macro. */
#define IS_RIP_DEBUG_EVENT  (pRip->ripDebugEvent & RIP_DEBUG_EVENT)

#define IS_RIP_DEBUG_PACKET (pRip->ripDebugPacket & RIP_DEBUG_PACKET)
#define IS_RIP_DEBUG_SEND   (pRip->ripDebugPacket & RIP_DEBUG_SEND)
#define IS_RIP_DEBUG_SEND_DETAIL (pRip->ripDebugPacket & RIP_DEBUG_SEND_DETAIL)
#define IS_RIP_DEBUG_RECV   (pRip->ripDebugPacket & RIP_DEBUG_RECV)
#define IS_RIP_DEBUG_RECV_DETAIL (pRip->ripDebugPacket & RIP_DEBUG_RECV_DETAIL)

#define IS_RIP_DEBUG_RIBMGR  (pRip->ripDebugRibmgr & RIP_DEBUG_RIBMGR)

extern void ripDebugReset (void);
extern void ripDebugInit (void);
extern Int32T configWriteRipDebug (struct cmsh *);

#endif /* _ripDebug_h */
