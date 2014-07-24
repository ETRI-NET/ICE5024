/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the snmp handling definitions.
 *  - Block Name : RIP Protocol
 *  - Process Name : ripd
 *  - Creator : geontae park
 *  - Initial Date : 2014/02/19
 */

/**
 * @file : ribSnmp.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_ripSnmp_h)
#define _ripSnmp_h

extern void ripSnmpInit (void);
extern void ripIfaddrAdd (InterfaceT *, ConnectedT *);
extern void ripIfaddrDelete (InterfaceT *, ConnectedT *);

#endif /* _ripSnmp_h */
