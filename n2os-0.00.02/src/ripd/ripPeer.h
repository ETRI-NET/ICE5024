/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the peernode handling definitions.
 *  - Block Name : RIP Protocol
 *  - Process Name : ripd
 *  - Creator : geontae park
 *  - Initial Date : 2014/02/19
 */

/**
 * @file : ribPeer.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_ripPeer_h)
#define _ripPeer_h

extern void ripPeerInit (void);
extern void ripPeerClean (void);
extern void ripPeerVersionUpdate (void);
extern void ripPeerUpdate (struct sockaddr_in *, Uint8T);
extern void ripPeerBadRoute (struct sockaddr_in *);
extern void ripPeerBadPacket (struct sockaddr_in *);
extern void ripPeerDisplay (struct cmsh *);
extern RipPeerT *ripPeerLookup (struct in_addr *);
extern RipPeerT *ripPeerLookupNext (struct in_addr *);

#endif /* _ripPeer_h */
