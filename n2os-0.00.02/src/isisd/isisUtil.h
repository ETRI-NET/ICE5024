/*
 * isisUtil.h
 *
 *  Created on: 2014. 4. 22.
 *      Author: root
 */

#ifndef ISISUTIL_H_
#define ISISUTIL_H_

#include "if.h"

#include "nnBuffer.h"
#include "stream.h"

extern struct interface * pifInterfaceStateRead (nnBufferT *);
extern struct interface * pifInterfaceAddRead (nnBufferT *);
extern struct connected * pifInterfaceAddressRead (Int32T, nnBufferT *);


#endif /* ISISUTIL_H_ */
