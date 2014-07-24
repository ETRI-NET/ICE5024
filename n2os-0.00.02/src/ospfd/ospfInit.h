/*
 * ospfInit.h
 *
 *  Created on: 2014. 5. 30.
 *      Author: root
 */

#ifndef OSPFINIT_H_
#define OSPFINIT_H_


#if !defined(_ribmgrInit_h)
#define _ribmgrInit_h

extern void ospfInitProcess();
extern void ospfTermProcess ();
extern void ospfRestartProcess ();
extern void ospfHoldProcess ();

extern void ospfSignalProcess (Int32T signalType);
extern void ospfIpcProcess (Int32T msgId, void * data, Uint32T dataLen);
extern void ospfEventProcess (Int32T msgId, void * data, Uint32T dataLen);



#endif


#endif /* OSPFINIT_H_ */
