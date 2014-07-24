/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the interface related definitions.
 *  - Block Name : RIP Protocol
 *  - Process Name : ripd
 *  - Creator : geontae park
 *  - Initial Date : 2014/02/19
 */

/**
 * @file : ribInterface.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_ripInterface_h)
#define _ripInterface_h

#include "nnBuffer.h"

#include "nnCmdCmsh.h"

/* Interface related functions. */
extern void ripNetworkClean (void);
extern void ripInterfaceReset (void);
extern void ripInterfaceMulticastSet (Int32T, ConnectedT *);
extern void ripPassiveNonDefaultClean (void);
extern void ripIfInit (void);
extern void ripIfVersionUpdate (void);
extern void ripIfDownAll (void);
extern Int32T ripEnableNetworkAdd(PrefixT *); 
extern Int32T ripEnableNetworkDelete(PrefixT *); 
extern Int32T configWriteRipInterface(struct cmsh *);
extern Int32T configWriteRipNetwork(struct cmsh *, Int32T);
extern Int32T ripNetworkDisplay (struct cmsh *); 
extern Int32T ripEnableIfAdd (const StringT); 
extern Int32T ripEnableIfDelete (const StringT); 
extern Int32T ripNeighborAdd (Prefix4T *); 
extern Int32T ripNeighborDelete (Prefix4T *); 
extern Int32T ripNeighborLookup (struct sockaddr_in *);
extern Int32T ripPassiveNonDefaultSet (const StringT); 
extern Int32T ripPassiveNonDefaultUnset (const StringT);
extern void ripDistributeUpdateInterface (InterfaceT *);
extern void ripIfRmapUpdateInterface (InterfaceT *);
extern Int32T ifCheckAddress (struct in_addr);

/* Interface event handling functions. */
extern Int32T ripInterfaceDown (nnBufferT *);
extern Int32T ripInterfaceUp (nnBufferT *);
extern Int32T ripInterfaceAdd (nnBufferT *);
extern Int32T ripInterfaceDelete (nnBufferT *);
extern Int32T ripInterfaceAddressAdd (nnBufferT *);
extern Int32T ripInterfaceAddressDelete (nnBufferT *);

#endif /* _ripInterface_h */
