/*
 * isisIpc.h
 *
 *  Created on: 2014. 4. 24.
 *      Author: root
 */

#ifndef ISISIPC_H_
#define ISISIPC_H_


void isisEventSubscribe( void );

extern void isisLcsSetRole (void *, Uint32T);
extern void isisLcsTerminate (void *, Uint32T);
extern void isisLcsHealthcheck (void *, Uint32T);
extern void isisLcsEventComponentErrorOccured (void *, Uint32T);
extern void isisLcsEventComponentServiceStatus (void *, Uint32T);

#endif /* ISISIPC_H_ */
