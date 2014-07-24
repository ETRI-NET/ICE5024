/*
 * isisInit.h
 *
 *  Created on: 2014. 4. 23.
 *      Author: root
 */

#ifndef ISISINIT_H_
#define ISISINIT_H_

#if !defined(_ribmgrInit_h)
#define _ribmgrInit_h

#include <linux/netlink.h>

#include "nnTypes.h"
#include "nnVector.h"
#include "nnList.h"
#include "nnPrefix.h"

extern void isisInitProcess();
extern void isisTermProcess ();
extern void isisRestartProcess ();
extern void isisHoldProcess ();

extern void isisSignalProcess (Int32T signalType);
extern void isisIpcProcess (Int32T msgId, void * data, Uint32T dataLen);
extern void isisEventProcess (Int32T msgId, void * data, Uint32T dataLen);

#endif

#endif /* ISISINIT_H_ */
