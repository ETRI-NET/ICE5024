/**
 * @file    	:  cmdMsg.c
 * @brief       :  Process packec IPC
 * 
 * $Id: cmdMsg.c 725 2014-01-17 09:11:34Z hyryu $			
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

/********************************************************************************
 *                                  INCLUDE FILES 
 ********************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "nnTypes.h"
#include "nnCmdCommon.h"
#include "nnCmdMsg.h"

/*******************************************************************************************************
 *                                       CODE
 *******************************************************************************************************/

/**
 * Description: function which used for CM IPC message making
 *
 * @param [in]  msg: Message buffer for CM IPC, Header structure to add 
 * @param [out] msgBuf: Updated (header is added) message buffer
 *
 * @retval : code
 */
Int32T
cmdIpcAddHdr(Int8T *msgBuf, cmdIpcMsg_T *msg)
{
    cmdIpcMsg_T *node = (cmdIpcMsg_T *)(msgBuf);
    node->length = 0;
    node->requestKey = msg->requestKey;
    node->code = msg->code;
    node->clientType = msg->clientType;
    node->mode = msg->mode;
    node->userKey = msg->userKey;
    node->srcID = msg->srcID;
    node->dstID = msg->dstID;
    return (CMD_IPC_OK);
}
/**
 * Description: used to unpack message header
 *
 * @param [in]  msgBuf: Message buffer of CM IPC to extract header 
 * @param [out] msg: Extracted header structure 
 *
 * @retval : code
 */
Int32T 
cmdIpcUnpackHdr(Int8T *msgBuf, cmdIpcMsg_T *msg)
{
    memcpy(msg, msgBuf, CMD_IPC_HDR_LEN);
    return (CMD_IPC_OK);
}
/**
 * Description: function which used for CM IPC message making
 *
 * @param [in]  data: Message buffer for CM IPC, TLV(type, length, value) to add.
 * @param [in]  type: type TLV
 * @param [in]  length: length TLV
 * @param [out] msgBuf: Updated (TLV is added) message buffer 
 *
 * @retval : code
 */
Int32T
cmdIpcAddTlv(Int8T *msgBuf, Int32T type, Int32T length, void *data)
{
    cmdIpcMsg_T *msgBufTemp;
    Int32T existSize;
    msgBufTemp = (cmdIpcMsg_T *) msgBuf;
    existSize = msgBufTemp->length;
    /** 2. Copy new TLV to message buffer */
    memcpy(msgBuf + sizeof(cmdIpcMsg_T) + existSize, &type, 4);
    memcpy(msgBuf + sizeof(cmdIpcMsg_T) + existSize + 4, &length, 4);
    memcpy(msgBuf + sizeof(cmdIpcMsg_T) + existSize + 8, data, length);
    /** 3. Update TLV size (existSize + type(4) + length(4) + value(length)) */
    msgBufTemp->length = existSize + 8 + length;
    return (CMD_IPC_OK);
}
/**
 * Description: used to unpack message TLV
 *
 * @param [in]  msgBuf  : Message buffer of CM IPC to extract header
 * @param [in]  type: type TLV
 * @param [out] size: size of data
 * @param [out] data  : Extracted header structure
 *
 * @retval : code
 */
Int32T
cmdIpcUnpackTlv(Int8T *msgBuf, Int32T type, Int32T *size, void *data)
{
    cmdIpcMsg_T *msg = (cmdIpcMsg_T *)(msgBuf);
    Int32T *tlvType, *tlvLength;
    void* tlvValue;
    Int32T existSize = msg->length + CMD_IPC_HDR_LEN;
    Int32T tlvPtr = CMD_IPC_HDR_LEN;
    do
    {
        tlvType = (Int32T *)(msgBuf + tlvPtr);
        tlvLength = (Int32T *)(msgBuf + tlvPtr + 4);
        tlvValue = msgBuf + tlvPtr + 8; 
        if(*tlvType == type) 
        {
            break;
        }
        tlvPtr = tlvPtr + 8 + *tlvLength;
    } while (tlvPtr < existSize);
    if (tlvPtr >= existSize) 
    {
        return (CMD_IPC_ERROR);
    }
    memcpy(data, tlvValue, *tlvLength);   	/** return value of TLV	*/    
    *tlvType = CMD_IPC_TLV_NULL; 			/** delete this TLV		*/
    *size = *tlvLength;
    return (CMD_IPC_OK);
}
/**
 * Description: send request
 *
 * @param [in]  sock  :  socket fd
 * @param [in]  msgBuf  : CM IPC message to send
 * @param [out] none
 *
 * @retval : number bytes sent over sockets or CMD_IPC_ERROR if error send
 */
Int32T
cmdIpcSend(Int32T sock, Int8T *msgBuf)
{
  cmdIpcMsg_T *cmIpcHdr = (cmdIpcMsg_T *)(msgBuf);
  ssize_t size = 0, numx = cmIpcHdr->length + CMD_IPC_HDR_LEN;
  Int32T error = 0;
  socklen_t sSize = sizeof (error);
  if(getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &sSize) < 0)
  {
    return CMD_IPC_ERROR; /** */
  }
  while (size < numx) 
  {
    size = send(sock, msgBuf, numx, 0);
    if (size < 0) 
    {
      if (errno == EAGAIN)
      {
        continue;
      }
fprintf(stderr, "[%s][%s][%d]\n", __FILE__, __func__, __LINE__);
      return CMD_IPC_ERROR; /** */
    }
  }
  return size;
}
/**
 * Description: receive response
 *
 * @param [in]  sock  :  socket fd 
 * @param [out] msgBuf : Received CM IPC message
 *
 * @retval : code
 */
Int32T
cmdIpcRecv(Int32T sock, Int8T *msgBuf)
{
  ssize_t size;
  Int32T error = 0;
  socklen_t sSize = sizeof (error);
  if(getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &sSize) < 0)
  {
    return CMD_IPC_ERROR; /** */
  }
  while (1) 
  {
    size = recv(sock, msgBuf, SO_RCV_BUF_SIZE_MAX, 0);
    if (size >= 0)
    {
      break;
    }
    if (size < 0) 
    {
      if (errno == EAGAIN)
      {
        continue;
      }
      return CMD_IPC_ERROR; /** */
    }
  }
  return size;
}

