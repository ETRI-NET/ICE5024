/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : RIP Protocol의 디버깅 설정을 제어하는 화일
 *
 * - Block Name : RIP Protocol
 * - Process Name : ripd
 * - Creator : Geontae Park
 * - Initial Date : 2014/02/20
 */

/**
 * @file        : ripDebug.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include "nnTypes.h"
#include "ripDebug.h"
#include "ripd.h"

#include "nnCmdCommon.h"


void
ripDebugReset (void)
{
  pRip->ripDebugEvent  = 0;
  pRip->ripDebugPacket = 0;
  pRip->ripDebugRibmgr = 0;
}

void
ripDebugInit (void)
{
  pRip->ripDebugEvent  = 0;
  pRip->ripDebugPacket = 0;
  pRip->ripDebugRibmgr = 0;
}

Int32T 
configWriteRipDebug (struct cmsh *cmsh)
{
  char packetBuff[RIP_BUFFER_MAX_SIZE] = {};
  Int32T idx = 0;

  cmdPrint (cmsh, "!");
  /* Debug configuration for internal ipc/events. */
  if (IS_RIP_DEBUG_EVENT)
  {
    cmdPrint (cmsh, "debug rip events");
  }

  /* Debug configuration for communication with peer. */
  if (IS_RIP_DEBUG_PACKET)
  {
    idx += sprintf (packetBuff, "debug rip packet");

    if (!IS_RIP_DEBUG_SEND && !IS_RIP_DEBUG_RECV)
    {
      cmdPrint (cmsh, "%s", packetBuff);
    }

    if (IS_RIP_DEBUG_SEND)
    {
      char sendBuff[RIP_BUFFER_MAX_SIZE] = {};
      Int32T sendIdx = idx;

      strcpy (sendBuff, packetBuff);
      sendIdx += sprintf (sendBuff + sendIdx, " send");
      if (IS_RIP_DEBUG_SEND_DETAIL)
      {
        sendIdx += sprintf (sendBuff + sendIdx, " detail");
      }
      cmdPrint (cmsh, "%s", sendBuff);
    }

    if (IS_RIP_DEBUG_RECV)
    {
      char recvBuff[RIP_BUFFER_MAX_SIZE] = {};
      Int32T recvIdx = idx;
      strcpy(recvBuff, packetBuff);
      recvIdx += sprintf (recvBuff + recvIdx, " recv");
      if (IS_RIP_DEBUG_RECV_DETAIL)
      {
        recvIdx += sprintf (recvBuff + recvIdx, " detail");
      }
      cmdPrint (cmsh, "%s", recvBuff);
    }
  }

  /* Debug configuration for communication with ribmgr. */
  if (IS_RIP_DEBUG_RIBMGR)
  {
    cmdPrint (cmsh, "debug rip ribmgr");
  }
  cmdPrint (cmsh, "!");

  return 0;
}
