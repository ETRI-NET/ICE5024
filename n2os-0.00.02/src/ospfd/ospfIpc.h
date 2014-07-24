/*
 * ospfIpc.h
 *
 *  Created on: 2014. 7. 8.
 *      Author: root
 */

#ifndef OSPFIPC_H_
#define OSPFIPC_H_


extern void ospfLcsSetRole (void *, Uint32T);
extern void ospfLcsTerminate (void *, Uint32T);
extern void ospfLcsHealthcheck (void *, Uint32T);
extern void ospfLcsEventComponentErrorOccured (void *, Uint32T);
extern void ospfLcsEventComponentServiceStatus (void *, Uint32T);


#endif /* OSPFIPC_H_ */
