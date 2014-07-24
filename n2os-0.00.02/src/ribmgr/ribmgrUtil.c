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


char PROC_NET_SNMP[] = "/proc/net/snmp";

static void 
dropLine (FILE *pFp)
{
  Int32T c;

  while ((c = getc (pFp)) != '\n')
    ;    
}


Int32T
ipForward (void)
{
  FILE *pFp;
  Int32T ipForwarding = 0; 
  char buf[10];

  printf ("====> %s %d\n", __FILE__, __LINE__);
  pFp = fopen (PROC_NET_SNMP, "r");

  if (pFp == NULL)
    return -1;  

  /* We don't care about the first line. */
  dropLine (pFp);
  
  /* Get ip_statistics.IpForwarding : 
     1 => ip forwarding enabled 
     2 => ip forwarding off. */
  if (fgets (buf, 6, pFp))
    sscanf (buf, "Ip: %d", &ipForwarding);

  fclose(pFp);
  
  if (ipForwarding == 1)
    return 1;

  return 0;
}

/* char proc_ipv4_forwarding[] = "/proc/sys/net/ipv4/conf/all/forwarding"; */
char PROC_IPV4_FORWARDING[] = "/proc/sys/net/ipv4/ip_forward";

Int32T
ipForwardEnable (void)
{
  FILE *pFp = NULL;
  
  printf ("====> %s %d\n", __FILE__, __LINE__);
  pFp = fopen (PROC_IPV4_FORWARDING, "w");

  if (pFp == NULL)
  {
    NNLOG (LOG_WARNING, "Can't open %s file\n", PROC_IPV4_FORWARDING);
    return -1;
  }

  fprintf (pFp, "1\n");

  fclose (pFp);

  return ipForward ();
}


Int32T
ipForwardDisable (void)
{
  FILE *pFp = NULL;

  pFp = fopen (PROC_IPV4_FORWARDING, "w");

  if (pFp == NULL)
  {
    NNLOG (LOG_WARNING, "Can't open %s file\n", PROC_IPV4_FORWARDING);
    return -1;
  }

  fprintf (pFp, "0\n");

  fclose (pFp);

  return ipForward ();
}


char PROC_IPV6_FORWARDING[] = "/proc/sys/net/ipv6/conf/all/forwarding";

Int32T
ipForwardIpv6 (void)
{
  FILE * pFp = NULL;
  char buf[5] = {};
  int ipForwarding = 0;

  pFp = fopen (PROC_IPV6_FORWARDING, "r");

  if (pFp == NULL)
    return -1; 

  if (fgets (buf, 2, pFp))
    sscanf (buf, "%d", &ipForwarding);

  fclose (pFp);

  return ipForwarding;
}

Int32T
ipForwardIpv6Enable (void)
{
  FILE * pFp = NULL;

  pFp = fopen (PROC_IPV6_FORWARDING, "w");

  if (pFp == NULL) 
  {
    NNLOG (LOG_WARNING, "Can't open %s file\n", PROC_IPV6_FORWARDING);
    return -1; 
  }

  fprintf (pFp, "1\n");

  fclose (pFp);

  return ipForwardIpv6 (); 
}

Int32T
ipForwardIpv6Disable (void)
{
  FILE *pFp = NULL;

  pFp = fopen (PROC_IPV6_FORWARDING, "w");

  if (pFp == NULL) 
  {
    NNLOG (LOG_WARNING, "Can't open %s file\n", PROC_IPV6_FORWARDING);
    return -1;
  }

  fprintf (pFp, "0\n");

  fclose (pFp);

  return ipForwardIpv6 ();
}

