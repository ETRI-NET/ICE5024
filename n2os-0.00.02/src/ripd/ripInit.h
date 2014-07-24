/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the Global data structure definitions.
 *  - Block Name : RIB Manager
 *  - Process Name : ribmgr
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : ribmgrInit.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_ribmgrInit_h)
#define _ribmgrInit_h

#include <linux/netlink.h>

#include "nnTypes.h"
#include "nnVector.h"
#include "nnList.h"
#include "nnPrefix.h"

extern void ripInitProcess();
extern void ripTermProcess ();
extern void ripRestartProcess ();
extern void ripHoldProcess ();
extern void ripTimerProcess (Int32T fd, Int16T event, void * arg);
extern void ripTimerProcessOne (Int32T fd, Int16T event, void * arg);
extern void ripTimerProcessTen (Int32T fd, Int16T event, void * arg);

extern void ripSignalProcess (Int32T signalType);
extern void ripIpcProcess (Int32T msgId, void * data, Uint32T dataLen);
extern void ripEventProcess (Int32T msgId, void * data, Uint32T dataLen);

#endif
