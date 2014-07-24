/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : RIP Protocol에서 필요한 기능들을 제공하는 화일
 *
 * - Block Name : RIP Protocol
 * - Process Name : ripd
 * - Creator : Geontae Park
 * - Initial Date : 2014/02/20
 */

/**
 * @file        : ripUtility.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include "nnTypes.h"
#include "nnBuffer.h"
#include "nnRibDefines.h"
#include "nnPrefix.h"
#include "nnTable.h"
#include "nosLib.h"

#include "ripd.h"
#include "ripUtil.h"

/* Message lookup function. */
const char * 
lookupStr (const struct message *mes, Int32T key)
{
  const struct message * pnt = NULL;

  for (pnt = mes; pnt->key != 0; pnt++) 
    if (pnt->key == key) 
      return pnt->str;

  return ""; 
}


static Int32T
memConstant(const void *s, Int32T c, size_t n)
{
  const Uint8T *p = s; 

  while (n-- > 0) 
    if (*p++ != c)
      return 0;
  return 1;
}


InterfaceT *
pifInterfaceAddRead (nnBufferT * msgBuff)
{
  InterfaceT * pIf = NULL;
  char ifName[INTERFACE_NAMSIZ] = {};

  /* Name Length, String. */
  Uint8T nameLength = nnBufferGetInt8T(msgBuff);
  nnBufferGetString(msgBuff, ifName, nameLength);

  /* Lookup/create interface by name. */
  pIf = ifGetByNameLen (ifName, strnlen(ifName, INTERFACE_NAMSIZ));
  
  /* Interface Index. */
  pIf->ifIndex = nnBufferGetInt32T(msgBuff);

  /* Status. */
  pIf->status = nnBufferGetInt8T(msgBuff);
  
  /* Flags. */
  pIf->flags = nnBufferGetInt64T(msgBuff);
  
  /* Metric. */
  pIf->metric = nnBufferGetInt32T(msgBuff);

  /* MTU. */
  pIf->mtu = nnBufferGetInt32T(msgBuff);
  
  /* MTU6. */
  pIf->mtu6 = nnBufferGetInt32T(msgBuff);
  
  /* Bandwidth. */
  pIf->bandwidth = nnBufferGetInt32T(msgBuff);

  /* HW Type. */
  pIf->hwType = nnBufferGetInt16T(msgBuff);

  /* HW Address Length. */
  pIf->hwAddrLen = nnBufferGetInt32T(msgBuff);

  if (pIf->hwAddrLen > 0)
  {
    /* HW Address Length. */
    nnBufferGetString(msgBuff, (char *)pIf->hwAddr, pIf->hwAddrLen);

    Int32T i = 0;
    Int32T idx = 0;
    char addrBuff[RIP_BUFFER_MAX_SIZE] = {};
    for (i=0;i<pIf->hwAddrLen;i++)
    {
      idx += sprintf (addrBuff + idx, "%02x:", pIf->hwAddr[i]);
    }
    NNLOG (LOG_DEBUG, "hwType[%d] hwAddrLen[%d] hwAddr[%s]\n",
           pIf->hwType, pIf->hwAddrLen, addrBuff);
  }

  return pIf;
}


/* 
 * Read interface up/down msg (EVENT_INTERFACE_UP/EVENT_INTERFACE_DOWN)
 * from zebra server.  The format of this message is the same as
 * that sent for EVENT_INTERFACE_ADD/EVENT_INTERFACE_DELETE.
 */
InterfaceT *
pifInterfaceStateRead (nnBufferT * msgBuff)
{
  InterfaceT *pIf;
  char ifName[INTERFACE_NAMSIZ];

  /* Name Length, String */
  Uint8T nameLength = nnBufferGetInt8T(msgBuff);
  nnBufferGetString(msgBuff, ifName, nameLength);

  /* Lookup this by interface index. */
  pIf = ifLookupByNameLen (ifName,
                               strnlen(ifName, INTERFACE_NAMSIZ));
 
  /* Check interface exist */
  if (!pIf)
    return NULL;
 
  /* Interface index */
  pIf->ifIndex = nnBufferGetInt32T(msgBuff);

  /* Status */
  pIf->status = nnBufferGetInt8T(msgBuff);
  
  /* Flags */
  pIf->flags = nnBufferGetInt64T(msgBuff);
  
  /* Metric */
  pIf->metric = nnBufferGetInt32T(msgBuff);

  /* MTU */
  pIf->mtu = nnBufferGetInt32T(msgBuff);
  
  /* MTU6 */
  pIf->mtu6 = nnBufferGetInt32T(msgBuff);
  
  /* Bandwidth */
  pIf->bandwidth = nnBufferGetInt32T(msgBuff);

  /* HW Type. */
  pIf->hwType = nnBufferGetInt16T(msgBuff);

  /* HW Address Length. */
  pIf->hwAddrLen = nnBufferGetInt32T(msgBuff);

  if (pIf->hwAddrLen > 0)
  {
    /* HW Address Length. */
    nnBufferGetString(msgBuff, (char *)pIf->hwAddr, pIf->hwAddrLen);

    Int32T i = 0;
    Int32T idx = 0;
    char addrBuff[RIP_BUFFER_MAX_SIZE] = {}; 
    for (i=0;i<pIf->hwAddrLen;i++)
    {   
      idx += sprintf (addrBuff + idx, "%02x:", pIf->hwAddr[i]);
    }   
    NNLOG (LOG_DEBUG, "hwType[%d] hwAddrLen[%d] hwAddr[%s]\n",
           pIf->hwType, pIf->hwAddrLen, addrBuff);
  }

  return pIf;
}


ConnectedT *
pifInterfaceAddressRead (Int32T type, nnBufferT * msgBuff)
{
  Uint32T ifIndex = 0, plen = 0;
  Uint8T ifcFlags = 0;
  InterfaceT * pIf = NULL;
  ConnectedT * pIfc = NULL;
  PrefixT p = {0,}, d = {0,};

  memset (&p, 0, sizeof(p));
  memset (&d, 0, sizeof(d));

  /* Get interface index. */
  ifIndex = nnBufferGetInt32T(msgBuff);

  nnBufferPrint(msgBuff);

  /* Lookup index. */
  pIf = ifLookupByIndex (ifIndex);
  if (pIf == NULL)
  {
    NNLOG (LOG_DEBUG, "pifInterfaceAddressRead(%s): "
                      "Can't find interface by ifindex: %d ",
                 (type == EVENT_INTERFACE_ADDRESS_ADD? "ADD" : "DELETE"),
                 ifIndex);
    return NULL;
  }

  /* Fetch flag. */
  ifcFlags = nnBufferGetInt8T(msgBuff);

  /* Fetch interface address. */
  p.family = nnBufferGetInt8T(msgBuff);
  if (p.family == AF_INET)
  {
    /* Prefix Address, Length */
    p.u.prefix4 = nnBufferGetInaddr (msgBuff);
    p.prefixLen = nnBufferGetInt8T (msgBuff);

    /* Destination */
    d.u.prefix4 = nnBufferGetInaddr(msgBuff);
  }
#ifdef HAVE_IPV6
  else if (p.family == AF_INET6)
  {
    /* Prefix Address, Length */
    p.u.prefix6 = nnBufferGetInaddr6 (msgBuff);
    p.prefixLen = nnBufferGetInt8T (msgBuff);

    /* Destination */
    d.u.prefix6 = nnBufferGetInaddr6(msgBuff);
  }
#endif

  if (type == EVENT_INTERFACE_ADDRESS_ADD)
  {
    /* N.B. NULL destination pointers are encoded as all zeroes */
    pIfc = connectedAddByPrefix(pIf, &p,(memConstant(&d.u.prefix,0,plen) ?
                          NULL : &d));
    if (pIfc != NULL)
    {
      pIfc->flags = ifcFlags;
      if (pIfc->pDestination)
      {
        pIfc->pDestination->prefixLen = pIfc->pAddress->prefixLen;
      }
    }
  }
  else
  {
    assert (type == EVENT_INTERFACE_ADDRESS_DELETE);
    pIfc = connectedDeleteByPrefix(pIf, &p);
  }

  return pIfc;
}
