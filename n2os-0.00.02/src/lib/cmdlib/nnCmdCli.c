/**
 * @file      : nnCmdCli.c
 * @brief       :
 *
 * $Id: nnCmdMd5.c 725 2014-01-17 09:11:34Z hyryu $
 * $Author: hyryu $
 * $Date: 2014-01-17 04:11:34 -0500 (Fri, 17 Jan 2014) $
 * $Log$
 * $Revision: 725 $
 * $LastChangedBy: hyryu $
 * $LastChanged$
 *
 *                      Electronics and Telecommunications Research Institute
 * Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.
 *           No part of this software shall be reproduced, stored in a retrieval system, or
 *           transmitted by any means, electronic, mechanical, photocopying, recording,
 *           or otherwise, without written permission from ETRI.
 **/

#include "nnTypes.h"
#include "nnLog.h"
#include "taskManager.h"

#include "nnCmdDefines.h"
#include "nnCmdCommon.h"
#include "nnCmdNode.h"
#include "nnCmdCmsh.h"
#include "nnCmdMsg.h"
#include "nnCmdCli.h"

/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
Int32T
cmdCmshRegister(struct cmsh *cmsh)
{
  Int8T szBuffer[CMD_IPC_MAXLEN];
  Int32T nbytes;
  cmdIpcMsg_T cmIpcHdr;
  cmIpcHdr.code = CMD_IPC_CODE_CLIENT_REG_REQ;
  cmIpcHdr.requestKey = CMD_IPC_KEY_NULL;
  cmIpcHdr.clientType = CMD_IPC_TYPE_CLI;
  cmIpcHdr.mode = CMD_NODE_VIEW;
  cmIpcHdr.userKey = cmsh->userKey;
  cmIpcHdr.srcID = cmsh->gTypes;
  cmIpcHdr.dstID = IPC_CM_MGR;
  cmdIpcAddHdr(szBuffer, &cmIpcHdr);    /**  Add Header  */
  if(cmsh->userID)
  cmdIpcAddTlv(szBuffer, CMD_IPC_TLV_USERID, strlen(cmsh->userID), cmsh->userID);
  if((nbytes = cmdIpcSend(cmsh->cmfd, szBuffer)) > 0)
  {
      return CMD_IPC_OK;
  }
  return CMD_IPC_ERROR;
}

Int32T
cmdCmshDeRegister(struct cmsh *cmsh)
{
  Int8T szBuffer[CMD_IPC_MAXLEN];
  Int32T nbytes;
  cmdIpcMsg_T cmIpcHdr;
  cmIpcHdr.code = CMD_IPC_CODE_CLIENT_DEREG_REQ;
  cmIpcHdr.requestKey = CMD_IPC_KEY_NULL;
  cmIpcHdr.clientType = CMD_IPC_TYPE_CLI;
  cmIpcHdr.mode = CMD_NODE_VIEW;
  cmIpcHdr.userKey = cmsh->userKey;
  cmIpcHdr.srcID = cmsh->gTypes;
  cmIpcHdr.dstID = IPC_CM_MGR;
  cmdIpcAddHdr(szBuffer, &cmIpcHdr);    /**  Add Header  */
  if(cmsh->userID)
  cmdIpcAddTlv(szBuffer, CMD_IPC_TLV_USERID, strlen(cmsh->userID), cmsh->userID);
  if((nbytes = cmdIpcSend(cmsh->cmfd, szBuffer)) > 0)
  {
      return CMD_IPC_OK;
  }
  return CMD_IPC_ERROR;
}
