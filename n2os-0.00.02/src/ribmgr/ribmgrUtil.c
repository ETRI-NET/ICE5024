/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : ribmgr에서 사용하는 유용한 함수 및 자료구조를 정의한 파일임.
 *
 * - Block Name : RIB Manager
 * - Process Name : ribmgr
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : ribmgrUtil.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */


#include <sys/time.h>

#include "nnTypes.h"
#include "nnRibDefines.h"
#include "nnPrefix.h"
#include "nosLib.h"

/* Description : PrefixT 값이 정상적인 값인지를 확인하는 함수 */
Int32T
ribCheckAddr (PrefixT *pPrefix)
{
  if (pPrefix->family == AF_INET)
  {
    Uint32T addr = 0;

    addr = pPrefix->u.prefix4.s_addr;
    addr = ntohl (addr);

    if (PREFIX_IPV4_NET127 (addr) || 
        IN_CLASSD (addr) || 
        PREFIX_IPV4_LINKLOCAL(addr))
    {
      return 0;
    }
  }
#ifdef HAVE_IPV6
  if (pPrefix->family == AF_INET6)
  {
    if (IN6_IS_ADDR_LOOPBACK (&pPrefix->u.prefix6))
    {
      return 0;
    }

    if (IN6_IS_ADDR_LINKLOCAL(&pPrefix->u.prefix6))
    {
      return 0;
    }
  }
#endif /* HAVE_IPV6 */
  return 1;
}

/* Description : Prefix4T 값이 정상적인 값인지를 확인하는 함수 */
Int32T
ribCheckAddr4 (Prefix4T * pPrefix)
{
  if (pPrefix->family == AF_INET)
  {
    Uint32T addr = 0;

    addr = pPrefix->prefix.s_addr;
    addr = ntohl (addr);

    if (PREFIX_IPV4_NET127 (addr) || 
        IN_CLASSD (addr) || 
        PREFIX_IPV4_LINKLOCAL(addr))
    {
      return 0;
    }
  }

  return 1;
}


#ifdef HAVE_IPV6
/* Utility function for making IPv6 address string. */
const char *
inet6_ntoa (struct in6_addr addr)
{
  static char buf[INET6_ADDRSTRLEN] = {};

  inet_ntop (AF_INET6, &addr, buf, INET6_ADDRSTRLEN);

  return buf;
}
#endif /* HAVE_IPV6 */

