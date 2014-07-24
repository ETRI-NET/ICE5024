/*
 * isisInterface.h
 *
 *  Created on: 2014. 4. 23.
 *      Author: root
 */

#ifndef ISISINTERFACE_H_
#define ISISINTERFACE_H_

#include "nnBuffer.h"

void isisIfInit( void );

/* Interface event handling functions. */
extern Int32T isisInterfaceDown (nnBufferT *);
extern Int32T isisInterfaceUp (nnBufferT *);
extern Int32T isisInterfaceAdd (nnBufferT *);
extern Int32T isisInterfaceDelete (nnBufferT *);
extern Int32T isisInterfaceAddressAdd (nnBufferT *);
extern Int32T isisInterfaceAddressDelete (nnBufferT *);


#endif /* ISISINTERFACE_H_ */
