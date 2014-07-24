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
 *  - Process Name : polmgr
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

#include "nnCmdDefines.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"
#include "nnCompProcess.h"

#include "nosLib.h"
#include "polMgrMain.h"

extern polMgrT *gPolMgrBase;
extern StringT sNosCompModuleVer;
int polOamSwitch = 0;	///global variable used for turning on/off the cmdOamPrint() function tester

// OamTimer Callback Function
void
polMgrOamTimerCb(Int32T fd, Int16T event, void * arg)
{
  struct tm * timeinfo;
  time_t rtime;
  time(&rtime);
  timeinfo = localtime(&rtime);

  // Cmd OamPrint exec
  if (polOamSwitch == 1)
  {
  cmdOamPrint("Notify %s : version %s", 
               asctime(timeinfo), sNosCompModuleVer);
  }
}

// OamTimer Set
void 
polMgrOamTimerSet(void)
{
  struct timeval tv = {2, 0};

  // OamTimer Set
  gPolMgrBase->gOamTimerEvent
   = taskTimerSet(polMgrOamTimerCb, tv, TASK_PERSIST, NULL);
}

// OamTimer Update(Dynamic Live Update Call) Function
void
polMgrOamTimerUpdate(void)
{
  struct timeval tv = {2, 0};

  // Oam Timer Update
  gPolMgrBase->gOamTimerEvent
   = taskTimerUpdate(polMgrOamTimerCb, gPolMgrBase->gOamTimerEvent, tv, NULL);
}

