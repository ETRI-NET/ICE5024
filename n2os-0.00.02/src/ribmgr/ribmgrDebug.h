#if !defined(_ribmgrDebug_h)
#define _ribmgrDebug_h


/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the connected route related definitions.
 *  - Block Name : RIB Manager
 *  - Process Name : ribmgr
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : ribmgrDebug.h
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include "nnCmdCmsh.h"

/* Debug flags. */
#define RIBMGR_DEBUG_EVENT   0x01
#define RIBMGR_DEBUG_KERNEL  0x01
#define RIBMGR_DEBUG_RIB     0x01

/* Debug related macro. */
#define IS_RIBMGR_DEBUG_EVENT  \
        (pRibmgr->ribmgrDebugEvent & RIBMGR_DEBUG_EVENT)
#define IS_RIBMGR_DEBUG_KERNEL \
        (pRibmgr->ribmgrDebugKernel & RIBMGR_DEBUG_KERNEL)
#define IS_RIBMGR_DEBUG_RIB    \
        (pRibmgr->ribmgrDebugRib & RIBMGR_DEBUG_RIB)

extern void ribmgrDebugInit (void);
extern Int32T configWriteRibmgrDebug (struct cmsh *);

#endif /* _ribmgrDebug_h  */
