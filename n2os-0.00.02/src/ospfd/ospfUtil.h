/*
 * ospfUtil.h
 *
 *  Created on: 2014. 6. 2.
 *      Author: root
 */

#ifndef OSPFUTIL_H_
#define OSPFUTIL_H_

#include "nnBuffer.h"

struct ospfEventArg
{
	void *arg1;
	int arg2;
};

extern void interfaceIfSetValue (nnBufferT * , struct interface *);
extern struct interface * pifInterfaceStateRead (nnBufferT *);
extern struct interface * pifInterfaceAddRead (nnBufferT *);
extern struct connected * pifInterfaceAddressRead (Int32T, nnBufferT *);

extern struct timeval ospfGetUpdateTime(long sec, long usec);
extern struct timeval ospfRecentRelativeTime (void);


#endif /* OSPFUTIL_H_ */
