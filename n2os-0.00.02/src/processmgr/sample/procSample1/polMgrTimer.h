#if !defined(_polTimer_h)
#define _polTimer_h

/*345678901234567890123456789012345678901234567890123456789012345678901234567890*/
/********************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
********************************************************************************/

/**   
 * @brief : This file include timer functions.
 *  - Block Name : Policy Manager
 *  - Process Name : polmgr
 *  - Creator : PyungKoo Park
 *  - Initial Date : 2014/03/03
 */

/**
 * @file:
 *
 * $Author:
 * $Date:
 * $Revision:
 * $LastChangedBy:
 */

#include "nnTypes.h"


void polMgrOamTimerSet(void);
void polMgrOamTimerUpdate(void);

/*
 * Description : Timer callback function
 *
 * param [in] fd : Timer Fd
 * param [in] event : Evnet id
 * param [in] arg : Arguments when is called
 */
void polMgrOamTimerCb(Int32T fd, Int16T event, void * arg);

#endif /* _polTimer_h */

