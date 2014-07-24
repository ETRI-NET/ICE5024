/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the utility related definitions.
 *  - Block Name : RIP Protocol
 *  - Process Name : ripd
 *  - Creator : geontae park
 *  - Initial Date : 2014/02/19
 */

/**
 * @file : ribUtil.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_ripUtil_h)
#define _ripUtil_h

#include "nnBuffer.h"
#include "nnIf.h"

/* Message structure. */
struct message
{
  Int32T key;
  const char *str;
};

/* external functions */
extern const char * lookupStr (const struct message *, Int32T);
extern InterfaceT * pifInterfaceAddRead (nnBufferT *);
extern InterfaceT * pifInterfaceStateRead (nnBufferT *);
extern ConnectedT * pifInterfaceAddressRead (Int32T, nnBufferT *);
#endif /* _ripUtil_h */
