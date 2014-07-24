/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the IPC related definitions.
 *  - Block Name : RIP Protocol
 *  - Process Name : ripd
 *  - Creator : geontae park
 *  - Initial Date : 2014/02/19
 */

/**
 * @file : ribIpc.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_ripIpc_h)
#define _ripIpc_h

extern void ripLcsSetRole (void *, Uint32T);
extern void ripLcsTerminate (void *, Uint32T);
extern void ripLcsHealthcheck (void *, Uint32T);
extern void ripLcsEventComponentErrorOccured (void *, Uint32T);
extern void ripLcsEventComponentServiceStatus (void *, Uint32T);

#endif /* _ripIpc_h */
