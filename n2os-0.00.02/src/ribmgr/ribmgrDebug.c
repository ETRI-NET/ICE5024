/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : RIB Manager에서 사용하는 디버깅 기능을 설정하는 함수로 이루어진 파일.
 * 디버깅 모드는 Command Manager 로 부터 설정한다.
 * - Block Name : RIB Manager
 * - Process Name : ribmgr
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : ribmgrDebug.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include "nnTypes.h"
#include "ribmgrDebug.h"
#include "ribmgrInit.h"

#include "nnCmdCommon.h"

void
ribmgrDebugReset (void)
{
  pRibmgr->ribmgrDebugEvent = 0;
  pRibmgr->ribmgrDebugKernel = 0;
  pRibmgr->ribmgrDebugRib = 0;
}

void
ribmgrDebugInit (void)
{
  pRibmgr->ribmgrDebugEvent = 0;
  pRibmgr->ribmgrDebugKernel = 0;
  pRibmgr->ribmgrDebugRib = 0;
}

Int32T 
configWriteRibmgrDebug (struct cmsh *cmsh)
{
  cmdPrint (cmsh, "!");
  /* Debug configuration for internal ipc/events. */
  if (IS_RIBMGR_DEBUG_EVENT)
  {
    cmdPrint (cmsh, "debug ribmgr events");
  }

  /* Debug configuration for communication with kernel. */
  if (IS_RIBMGR_DEBUG_KERNEL)
  {
    cmdPrint (cmsh, "debug ribmgr kernel");
  }

  /* Debug configuration for communication with ribmgr. */
  if (IS_RIBMGR_DEBUG_RIB)
  {
    cmdPrint (cmsh, "debug ribmgr rib");
  }
  cmdPrint (cmsh, "!");

  return 0;
}

