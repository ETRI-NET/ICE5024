/*3456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*********************************.*********************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute.
*           All rights reserved. No part of this software shall be reproduced,
*           stored in a retrieval system, or transmitted by any means,
*           electronic, mechanical, photocopying, recording, or otherwise,
*           without written permission from ETRI.
*******************************************************************************/
 
/**
 * @brief : This file include timer functions.
 *  - Block Name : Policy Manager
 *  - Process Name : cmmgr
 *  - Creator : PyungKoo Park
 *  - Initial Date : 2014/03/03
 */
 
/**
 * @file        :
 *
 * $Author: $
 * $Date: $
 * $Revision: $
 * $LastChangedBy: $
 */


#include <stdio.h>
#include "nnTypes.h"

#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"

#include "nosLib.h"


//timer process in every 1 second
void cmTimerProcess(Int32T fd, Int16T event, void * arg)
{
    // 2 sec
    return;
}

//timer process in every 1 second
void cmTimerProcessOne(Int32T fd, Int16T event, void * arg)
{
    // 30 sec
    return;
}

//timer process in every 10 second
void cmTimerProcessTen(Int32T fd, Int16T event, void * arg)
{
    // 50 sec
    return;
}

