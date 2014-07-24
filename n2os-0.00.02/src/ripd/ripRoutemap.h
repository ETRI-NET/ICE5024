/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the routemap handling definitions.
 *  - Block Name : RIP Protocol
 *  - Process Name : ripd
 *  - Creator : geontae park
 *  - Initial Date : 2014/02/19
 */

/**
 * @file : ribRoutemap.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_ripRoutemap_h)
#define _ripRoutemap_h

extern void ripRouteMapInit (void);
extern void ripRouteMapVersionUpdate (); 
extern void ripRouteMapReset (void);

#endif /* _ripRoutemap_h */
