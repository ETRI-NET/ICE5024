/*345678901234567890123456789012345678901234567890123456789012345678901234567890*/
/*********************************************************************************
 *               Electronics and Telecommunications Research Institute
 * Filename : 
 * Blockname: 
 * Overview : 
 * Creator  : 
 * Owner    : 
 * Copyright: 2013 Electronics and Telecommunications Research Institute. 
 *            All rights reserved. No part of this software shall be reproduced, 
 *            stored in a retrieval system, or transmitted by any means, 
 *            electronic, mechanical, photocopying, recording, or otherwise, 
 *            without written permission from ETRI.
 *********************************************************************************/

/*********************************************************************************
 *                        VERSION CONTROL  INFORMATION
 * $Author$
 * $Date$
 * $Revision
 * $Log$ 
 *********************************************************************************/

#include <stdio.h>
#include "taskManager.h"

extern void nosTaskLibInitialize();
extern void nosTaskScheduleInitialize();
extern void nosCompSignalProcess(Int32T sig);
extern void nosCompInitialize();

#define COMP_NAME "pif"
#define COMP_VER "0.0.1"

Int32T main(Int32T argc, char **argv)
{
	//initialize dynamic module & task lib 
	nosTaskLibInitialize(COMP_NAME, COMP_VER);

	//create nos main task
	taskCreate(nosCompInitialize);

	// schedule work(ipc/event/timer) 
	nosTaskScheduleInitialize();

	// Task start
	taskDispatch();

	return(0);
}
