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
 * $Author: sckim007 $
 * $Date: 2014-05-09 18:23:27 +0900 (2014-05-09, 금) $
 * $Revision
 * $Log$ 
 *********************************************************************************/

#include <stdio.h>
#include "taskManager.h"
#include "nnCmdDefines.h"

extern void nosTaskLibInitialize();
extern void nosTaskScheduleInitialize();
extern void nosCompInitialize();
extern void nosCmdInitialize(Int32T);

#define COMP_NAME "lacp"
#define COMP_VER "0.0.1"

/* Rib manager global data structure. */
void ** gCompData = NULL;

Int32T main(Int32T argc, char **argv)
{
  /* Initialize dynamic module & task lib. */
  nosTaskLibInitialize(COMP_NAME, COMP_VER);

  /* Create nos main task. */
  taskCreate(nosCompInitialize);

  /* schedule work(ipc/event/timer). */
  nosTaskScheduleInitialize();

  /* Command channel open */
  nosCmdInitialize(IPC_LACP);

  /* Task start. */
  taskDispatch();

  return(0);
}
