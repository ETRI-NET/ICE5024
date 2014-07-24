// -----------------------------------------------------------------
//      Electronics and Telecommunications Research Institute 
//
// Filename:     $Source: /lib/nnBuffer.c
//               $Id: nnBuffer.c,v 0.01 11/28/2013, by suncheul kim
//
// Subsystem:    System Library
//
// Overview:     This file contains the ipc functionis between the 
//               components and rib manager. 
//               These functions provide to other components 
//               by N2OS IPC.
//
// Creator:      $Author: suncheul kim 
//
// Owner:        suncheul kim
//
// Copyright:    2013 ETRI. All Rights Reserved.
//               No part of this software shall be reproduced,
//               stored in a retrieval system, or transmitted by
//               any means, electronic, mechanical, photocopying,
//               recording, or otherwise, without written 
//               permission from ETRI Inc.
//
// Modificaiton History:
// ====================
// 11/28/2013    suncheul kim    Initial Creation.
//
// -----------------------------------------------------------------

/**
 * @file        : nnVector.c
 * @brief       : IPC 메시지 송수신시 사용할 버퍼 및 버퍼에 내용을 추가
 * 삭제 할 수 있는 기능을 제공하는 함수들을 구현한 파일임.
 */

/**               Version Control Information                       */

 /** $Id: nnBuffer.c 570 2014-01-08 08:31:00Z sckim007 $
  * $Author: sckim007 $
  * $Date: 2014-01-08 17:31:00 +0900 (Wed, 08 Jan 2014) $
  * $Log$
  * $Revision: 570 $
  * $LastChangedBy: sckim007 $
  * $LastChanged$
  *
  *            Electronics and Telecommunications Research Institute
  * Copyright: 2013 Electronics and Telecommunications Research Institute.
  * All rights reserved.
  * No part of this software shall be reproduced, stored in a retrieval system, or
  * transmitted by any means, electronic, mechanical, photocopying, recording,
  * or otherwise, without written permission from ETRI.
  *
  */

// testing2 

#include "nnBuffer.h"

/**
 * Description : 버퍼를 초기화 하는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 *
 * @retval : none
 */
void
nnBufferReset (nnBufferT * buffer)
{
  buffer->index = 0;
  buffer->length = 0;
  memset(buffer->data, 0, NN_MAX_BUFFER_SIZE);
}


/**
 * Description : 버퍼를 원하는 크기만큼 생성하는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 * @param [in] msg : pointer of new buffer
 * @param [in] msgLen : specified buffer length
 *
 * @retval : none
 */
void
nnBufferAssign (nnBufferT * buffer, char * msg, Uint32T msgLen)
{
  buffer->length = msgLen;
  memcpy(buffer->data, msg, msgLen);
}


/**
 * Description : 버퍼 길이를 얻는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 *
 * @retval : length of buffer
 */
Uint32T
nnBufferGetLength (nnBufferT * buffer)
{
  return buffer->length;
}


/**
 * Description : 버퍼 인덱스를 얻는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 *
 * @retval : length of buffer
 */
Uint32T
nnBufferGetIndex (nnBufferT * buffer)
{
  return buffer->index;
}


/**
 * Description : 버퍼 인덱스를 강제적으로 설정하는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 * @param [in] newIndex : specified index value
 *
 * @retval : none
 */
void
nnBufferSetIndex (nnBufferT * buffer, Uint32T newIndex)
{
  buffer->index = newIndex;
}

/**
 * Description : 버퍼의 인덱스로 위치 부터 전달한 길이만큼을 복사하는 함수임.
 *
 * @param [in] dest : destination buffer to be copied
 * @param [in] buffer : pointer of buffer
 * @param [in] length : specified length to be copied
 *
 * @retval : none
 */
void
nnBufferNCpy (nnBufferT * dest, nnBufferT * buffer, Uint32T length)
{
  memcpy(dest->data, buffer->data + buffer->index, length);
  buffer->index += length;
  dest->length = length;
}


/**
 * Description : 버퍼에서 저장되어 있는 데이터 버퍼의 포인터를 얻는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 *
 * @retval : pointer of data buffer
 */
char * 
nnBufferGetData (nnBufferT * buffer)
{
  return buffer->data;
}


/**
 * Description : 버퍼의 내용을 헥사 값으로 출력하는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 *
 * @retval : none
 */
void
nnBufferPrint (nnBufferT * buffer)
{
  char * cp   = buffer->data;
  char * p    = buffer->data;
  Int32T len  = buffer->length;
  Int32T hcnt = 0;

  while (len > 0) {
    if (hcnt == 0)
      printf("x%2.2lx: ", (long unsigned int)(cp-p));
    if ((hcnt%4) == 0)
      printf(" ");
    printf("%2.2x", (0xff&*cp++)) ;
    len--;
    hcnt++;
    if (hcnt >= 16 || len == 0) {
      printf("\n");
      hcnt = 0;
    }
  }  
}


/**
 * Description : 버퍼에서 8비트(1바이트)를 읽어내는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 *
 * @retval : value to be read
 */
Int8T  
nnBufferGetInt8T (nnBufferT * buffer)
{
  Int8T val;
  
  memcpy(&val, buffer->data + buffer->index, sizeof(Int8T));
  buffer->index += sizeof(Int8T);

  return val;
}


/**
 * Description : 버퍼에서 16비트(2바이트)를 읽어내는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 *
 * @retval : value to be read
 */
Int16T 
nnBufferGetInt16T(nnBufferT * buffer)
{
  Int16T val;

  memcpy(&val, buffer->data + buffer->index, sizeof(Int16T));
  buffer->index += sizeof(Int16T);

  return ntohs(val);
}


/**
 * Description : 버퍼에서 32비트(4바이트)를 읽어내는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 *
 * @retval : value to be read
 */
Int32T 
nnBufferGetInt32T(nnBufferT * buffer)
{
  Int32T val;

  memcpy(&val, buffer->data + buffer->index, sizeof(Int32T));
  buffer->index += sizeof(Int32T);

  return ntohl(val);
}


/**
 * Description : 버퍼에서 64/32비트(8/4바이트)를 읽어내는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 *
 * @retval : value to be read
 */
Int64T 
nnBufferGetInt64T(nnBufferT * buffer)
{
  Int64T val;

  memcpy(&val, buffer->data + buffer->index, sizeof(Int64T));
  buffer->index += sizeof(Int64T);

  return ntohl(val);
}


/**
 * Description : 버퍼에서 Prefix4T 크기만큼을 읽어내는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 *
 * @retval : value to be read
 */
Prefix4T 
nnBufferGetPrefix4T(nnBufferT * buffer)
{
  Prefix4T val;

  memcpy(&val, buffer->data + buffer->index, sizeof(Prefix4T));
  buffer->index += sizeof(Prefix4T);

  return val;
}


/**
 * Description : 버퍼에서 Prefix6T 크기만큼을 읽어내는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 *
 * @retval : value to be read
 */
Prefix6T 
nnBufferGetPrefix6T(nnBufferT * buffer)
{
  Prefix6T val;

  memcpy(&val, buffer->data + buffer->index, sizeof(Prefix6T));
  buffer->index += sizeof(Prefix6T);

  return val;
}


/**
 * Description : 버퍼에서 PrefixT 크기만큼을 읽어내는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 *
 * @retval : value to be read
 */
PrefixT 
nnBufferGetPrefixT(nnBufferT * buffer)
{
  PrefixT val;

  memcpy(&val, buffer->data + buffer->index, sizeof(PrefixT));
  buffer->index += sizeof(PrefixT);

  return val;
}



/**
 * Description : 버퍼에서 in_addr 크기만큼을 읽어내는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 *
 * @retval : value to be read
 */
struct in_addr 
nnBufferGetInaddr(nnBufferT * buffer)
{
  struct in_addr val;

  memcpy(&val, buffer->data + buffer->index, sizeof(struct in_addr));
  buffer->index += sizeof(struct in_addr);

  return val;
}


/**
 * Description : 버퍼에서 in6_addr 크기만큼을 읽어내는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 *
 * @retval : value to be read
 */
struct in6_addr 
nnBufferGetInaddr6(nnBufferT * buffer)
{
  struct in6_addr val;

  memcpy(&val, buffer->data + buffer->index, sizeof(struct in6_addr));
  buffer->index += sizeof(struct in6_addr);

  return val;
}


/**
 * Description : 버퍼에서 명시된 크기만큼의 문자열을 읽어내는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 * @param [in] dest : destination buffer to be copied
 * @param [in] length : specified length
 *
 * @retval : none
 */
void
nnBufferGetString(nnBufferT * buffer, char * dest, Int32T length)
{
  memcpy(dest, buffer->data + buffer->index, length);
  buffer->index += length;
}


/**
 * Description : 버퍼에 8비트(1바이트)를 추가하는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 * @param [in] val : variable to be added
 *
 * @retval : none
 */
void
nnBufferSetInt8T (nnBufferT * buffer, Int8T val)
{
  memcpy(buffer->data + buffer->index, &val, sizeof(Int8T));
  buffer->index += sizeof(Int8T);
  buffer->length = buffer->index;
}


/**
 * Description : 버퍼에 16비트(2바이트)를 추가하는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 * @param [in] val : variable to be added
 *
 * @retval : none
 */
void 
nnBufferSetInt16T(nnBufferT * buffer, Int16T val)
{
  Int16T nwVal = htons(val);

  memcpy(buffer->data + buffer->index, &nwVal, sizeof(Int16T));
  buffer->index += sizeof(Int16T);
  buffer->length = buffer->index;
}


/**
 * Description : 버퍼에 32비트(4바이트)를 추가하는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 * @param [in] val : variable to be added
 *
 * @retval : none
 */
void 
nnBufferSetInt32T(nnBufferT * buffer, Int32T val)
{
  Int32T nwVal = htonl(val);

  memcpy(buffer->data + buffer->index, &nwVal, sizeof(Int32T));
  buffer->index += sizeof(Int32T);
  buffer->length = buffer->index;
}


/**
 * Description : 버퍼에 64/32비트(8/4바이트)를 추가하는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 * @param [in] val : variable to be added
 *
 * @retval : none
 */
void 
nnBufferSetInt64T(nnBufferT * buffer, Int64T val)
{
  Int32T nwVal = htonl(val);

  memcpy(buffer->data + buffer->index, &nwVal, sizeof(Int64T));
  buffer->index += sizeof(Int64T);
  buffer->length = buffer->index;
}


/**
 * Description : 버퍼에 Prefix4T를 추가하는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 * @param [in] val : variable to be added
 *
 * @retval : none
 */
void 
nnBufferSetPrefix4T(nnBufferT * buffer, Prefix4T * val)
{
  memcpy(buffer->data + buffer->index, &val, sizeof(Prefix4T));
  buffer->index += sizeof(Prefix4T);
  buffer->length = buffer->index;
}


/**
 * Description : 버퍼에 Prefix6T를 추가하는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 * @param [in] val : variable to be added
 *
 * @retval : none
 */
void 
nnBufferSetPrefix6T(nnBufferT * buffer, Prefix6T * val)
{
  memcpy(buffer->data + buffer->index, &val, sizeof(Prefix6T));
  buffer->index += sizeof(Prefix6T);
  buffer->length = buffer->index;
}


/**
 * Description : 버퍼에 PrefixT를 추가하는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 * @param [in] val : variable to be added
 *
 * @retval : none
 */
void 
nnBufferSetPrefixT(nnBufferT * buffer, PrefixT * val)
{
  memcpy(buffer->data + buffer->index, &val, sizeof(PrefixT));
  buffer->index += sizeof(PrefixT);
  buffer->length = buffer->index;
}


/**
 * Description : 버퍼에 in_addr를 추가하는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 * @param [in] val : variable to be added
 *
 * @retval : none
 */
void
nnBufferSetInaddr(nnBufferT * buffer, struct in_addr val)
{
  memcpy(buffer->data + buffer->index, &val, sizeof(struct in_addr));
  buffer->index += sizeof(struct in_addr);
  buffer->length = buffer->index;
}


/**
 * Description : 버퍼에 in6_addr를 추가하는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 * @param [in] val : variable to be added
 *
 * @retval : none
 */
void
nnBufferSetInaddr6(nnBufferT * buffer, struct in6_addr val)
{
  memcpy(buffer->data + buffer->index, &val, sizeof(struct in6_addr));
  buffer->index += sizeof(struct in6_addr);
  buffer->length = buffer->index;
}


/**
 * Description : 버퍼에 원하는 길이만큼의 문자열을 추가하는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 * @param [in] src : pointer of character buffer to be added
 * @param [in] length : specified length
 *
 * @retval : none
 */
void
nnBufferSetString(nnBufferT * buffer, char * src, Int32T length)
{
  memcpy(buffer->data + buffer->index, src, length);
  buffer->index += length;
  buffer->length = buffer->index;
}


/**
 * Description : 버퍼의 원하는 위치에 8비트(1바이트)를 추가하는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 * @param [in] src : position of buffer
 * @param [in] val : value to be added
 *
 * @retval : none
 */
void 
nnBufferInsertInt8T(nnBufferT * buffer, Uint32T position, Uint8T val)
{
  memcpy(buffer->data + position, &val, sizeof(Int8T));
}


/**
 * Description : 버퍼의 원하는 위치에 16비트(2바이트)를 추가하는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 * @param [in] src : position of buffer
 * @param [in] val : value to be added
 *
 * @retval : none
 */
void 
nnBufferInsertInt16T(nnBufferT * buffer, Uint32T position, Uint16T val)
{
  Int16T nwVal = htons(val);

  memcpy(buffer->data + position, &nwVal, sizeof(Int16T));
}


/**
 * Description : 버퍼의 원하는 위치에 32비트(4바이트)를 추가하는 함수임.
 *
 * @param [in] buffer : pointer of buffer
 * @param [in] src : position of buffer
 * @param [in] val : value to be added
 *
 * @retval : none
 */
void 
nnBufferInsertInt32T(nnBufferT * buffer, Uint32T position, Uint32T val)
{
  Int32T nwVal = htonl(val);

  memcpy(buffer->data + position, &nwVal, sizeof(Int32T));
}

