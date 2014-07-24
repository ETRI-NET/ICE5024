#include "nnTypes.h"

#if !defined(_nntime_h)
#define _nntime_h

/*******************************************************************************
 *            Electronics and Telecommunications Research Institute
 * Copyright: 2013 Electronics and Telecommunications Research Institute.
 *            All rights reserved.
 *            No part of this software shall be reproduced, stored in a
 *            retrieval system, or transmitted by any means, electronic,
 *            mechanical, photocopying, recording, or otherwise, without
 *            written permission from ETRI.
*******************************************************************************/

/**
 * @brief : N2OS Common Library - Time 관련
 *  - Block Name : Library
 *  - Process Name : rLibraryibmgr
 *  - Creator : Changwoo Lee, JaeSu Han
 *  - Initial Date : 2013/7/15
*/

/**   
 * @file : nnTime.h
 *
 * $Id: nnTime.h 830 2014-02-14 02:21:45Z lcw1127 $			
 * $Author: lcw1127 $ 
 * $Date: 2014-02-14 11:21:45 +0900 (Fri, 14 Feb 2014) $
 * $Revision: 830 $
 * $LastChangedBy: lcw1127 $
 **/


/*******************************************************************************
 *                        CONSTANTS / LITERALS / TYPES
 ******************************************************************************/


/*******************************************************************************
*                                GLOBALS VARIABLES
*******************************************************************************/


/*******************************************************************************
 *                           GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/

void nnTimeGetCurrent (StringT timeStr);
void nnTimeGetCurrentDate (StringT timeStr); 
#endif
