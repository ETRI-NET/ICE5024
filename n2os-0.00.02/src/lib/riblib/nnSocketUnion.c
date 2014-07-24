/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : 각 컴포넌트에서 사용하는 공통 socket union 관련 기능을 제공한다.
 * - Block Name : riblib
 * - Process Name : rib library
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : nnSocketUnion.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "nnPrefix.h"
#include "nnSocketUnion.h"
#include "nosLib.h"


const StringT 
inet_sutop (uSockUnionT *pSu, StringT str)
{
  switch (pSu->sa.sa_family)
    {
    case AF_INET:
      inet_ntop (AF_INET, &pSu->sin.sin_addr, str, INET_ADDRSTRLEN);
      break;
#ifdef HAVE_IPV6
    case AF_INET6:
      inet_ntop (AF_INET6, &pSu->sin6.sin6_addr, str, INET6_ADDRSTRLEN);
      break;
#endif /* HAVE_IPV6 */
    }
  return str;
}

Int32T
strToSockUnion (const StringT str, uSockUnionT *pSu)
{
  Int32T ret = 0;

  memset (pSu, 0, sizeof (uSockUnionT));

  ret = inet_pton (AF_INET, str, &pSu->sin.sin_addr);
  if (ret > 0)			/* Valid IPv4 address format. */
    {
      pSu->sin.sin_family = AF_INET;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
      pSu->sin.sin_len = sizeof(struct sockaddr_in);
#endif /* HAVE_STRUCT_SOCKADDR_IN_SIN_LEN */
      return 0;
    }
#ifdef HAVE_IPV6
  ret = inet_pton (AF_INET6, str, &pSu->sin6.sin6_addr);
  if (ret > 0)			/* Valid IPv6 address format. */
    {
      pSu->sin6.sin6_family = AF_INET6;
#ifdef SIN6_LEN
      pSu->sin6.sin6_len = sizeof(struct sockaddr_in6);
#endif /* SIN6_LEN */
      return 0;
    }
#endif /* HAVE_IPV6 */
  return -1;
}

const char * 
sockUnionToStr (uSockUnionT *pSu, StringT buf, Uint32T len)
{
  if  (pSu->sa.sa_family == AF_INET)
    return inet_ntop (AF_INET, &pSu->sin.sin_addr, buf, len);
#ifdef HAVE_IPV6
  else if (pSu->sa.sa_family == AF_INET6)
    return inet_ntop (AF_INET6, &pSu->sin6.sin6_addr, buf, len);
#endif /* HAVE_IPV6 */
  return NULL;
}

uSockUnionT *
sockUnionStrToSu (const StringT str)
{
  Int32T ret = 0;
  uSockUnionT *pSu = NULL;

  pSu = NNMALLOC (MEM_SOCKET_UNION, sizeof (uSockUnionT));

  ret = inet_pton (AF_INET, str, &pSu->sin.sin_addr);
  if (ret > 0)			/* Valid IPv4 address format. */
  {
    pSu->sin.sin_family = AF_INET;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
    pSu->sin.sin_len = sizeof(struct sockaddr_in);
#endif /* HAVE_STRUCT_SOCKADDR_IN_SIN_LEN */
    return pSu;
  }
#ifdef HAVE_IPV6
  ret = inet_pton (AF_INET6, str, &pSu->sin6.sin6_addr);
  if (ret > 0)			/* Valid IPv6 address format. */
  {
    pSu->sin6.sin6_family = AF_INET6;
#ifdef SIN6_LEN
    pSu->sin6.sin6_len = sizeof(struct sockaddr_in6);
#endif /* SIN6_LEN */
      return pSu;
  }
#endif /* HAVE_IPV6 */

  NNFREE (MEM_SOCKET_UNION, pSu);
  return NULL;
}

StringT 
sockUnionSuToStr (uSockUnionT *pSu)
{
  char str[SU_ADDRSTRLEN] = {};

  switch (pSu->sa.sa_family)
  {
    case AF_INET:
      inet_ntop (AF_INET, &pSu->sin.sin_addr, str, sizeof (str));
      break;
#ifdef HAVE_IPV6
    case AF_INET6:
      inet_ntop (AF_INET6, &pSu->sin6.sin6_addr, str, sizeof (str));
      break;
#endif /* HAVE_IPV6 */
  }
  return strdup (str);
}

/* Convert IPv4 compatible IPv6 address to IPv4 address. */
static void
sockUnionNormaliseMapped (uSockUnionT *pSu)
{
#ifdef HAVE_IPV6
  struct sockaddr_in sin;
  if (pSu->sa.sa_family == AF_INET6 && 
      IN6_IS_ADDR_V4MAPPED (&pSu->sin6.sin6_addr))
  {
    memset (&sin, 0, sizeof (struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port = pSu->sin6.sin6_port;
    memcpy (&sin.sin_addr, ((char *)&pSu->sin6.sin6_addr) + 12, 4);
    memcpy (pSu, &sin, sizeof (struct sockaddr_in));
  }
#endif /* HAVE_IPV6 */
}

/* Return socket of sockunion. */
Int32T
sockUnionSocket (uSockUnionT *pSu)
{
  Int32T sock = 0;

  sock = socket (pSu->sa.sa_family, SOCK_STREAM, 0);
  if (sock < 0)
  {
    NNLOG (LOG_WARNING, "Can't make socket : %s", strerror(errno));
    return -1;
  }

  return sock;
}

/* Return accepted new socket file descriptor. */
Int32T
sockUnionAccept (Int32T sock, uSockUnionT *pSu)
{
  socklen_t len = 0;
  Int32T clientSock = 0;

  len = sizeof (uSockUnionT);
  clientSock = accept (sock, (struct sockaddr *) pSu, &len);
  
  sockUnionNormaliseMapped (pSu);
  return clientSock;
}

/* Return sizeof uSockUnionT.  */
static Int32T
sockUnionSizeof (uSockUnionT *pSu)
{
  Int32T ret = 0;

  switch (pSu->sa.sa_family)
  {
    case AF_INET:
      ret = sizeof (struct sockaddr_in);
      break;
#ifdef HAVE_IPV6
    case AF_INET6:
      ret = sizeof (struct sockaddr_in6);
      break;
#endif /* AF_INET6 */
  }
  return ret;
}

/* return sockunion structure : this function should be revised. */
static StringT 
sockUnionLog (uSockUnionT *pSu)
{
  static char buf[SU_ADDRSTRLEN] = {};

  switch (pSu->sa.sa_family) 
  {
    case AF_INET:
      snprintf (buf, SU_ADDRSTRLEN, "%s", inet_ntoa (pSu->sin.sin_addr));
      break;
#ifdef HAVE_IPV6
    case AF_INET6:
      snprintf (buf, SU_ADDRSTRLEN, "%s",
		inet_ntop (AF_INET6, &(pSu->sin6.sin6_addr), buf, SU_ADDRSTRLEN));
      break;
#endif /* HAVE_IPV6 */
    default:
      snprintf (buf, SU_ADDRSTRLEN, "af_unknown %d ", pSu->sa.sa_family);
      break;
  }
  return (strdup (buf));
}

/* sockUnionConnect returns
   -1 : error occured
   0 : connect success
   1 : connect is in progress */
eConnectResult
sockUnionConnect (Int32T fd, uSockUnionT *pPeerSU, Uint16T port,
                  Uint32T ifIndex)
{
  Int32T ret = 0;
  Int32T val = 0;
  uSockUnionT su = {};

  memcpy (&su, pPeerSU, sizeof (uSockUnionT));

  switch (su.sa.sa_family)
  {
    case AF_INET:
      su.sin.sin_port = port;
      break;
#ifdef HAVE_IPV6
    case AF_INET6:
      su.sin6.sin6_port  = port;
#ifdef KAME
      if (IN6_IS_ADDR_LINKLOCAL(&su.sin6.sin6_addr) && ifIndex)
	{
#ifdef HAVE_STRUCT_SOCKADDR_IN6_SIN6_SCOPE_ID
	  /* su.sin6.sin6_scope_id = ifIndex; */
#ifdef MUSICA
	  su.sin6.sin6_scope_id = ifIndex; 
#endif
#endif /* HAVE_STRUCT_SOCKADDR_IN6_SIN6_SCOPE_ID */
#ifndef MUSICA
	  SET_IN6_LINKLOCAL_IFINDEX (su.sin6.sin6_addr, ifIndex);
#endif
	}
#endif /* KAME */
      break;
#endif /* HAVE_IPV6 */
  }      

  /* Make socket non-block. */
  val = fcntl (fd, F_GETFL, 0);
  fcntl (fd, F_SETFL, val|O_NONBLOCK);

  /* Call connect function. */
  ret = connect (fd, (struct sockaddr *) &su, sockUnionSizeof (&su));

  /* Immediate success */
  if (ret == 0)
  {
    fcntl (fd, F_SETFL, val);
    return CONNECT_SUCCESS;
  }

  /* If connect is in progress then return 1 else it's real error. */
  if (ret < 0)
  {
    if (errno != EINPROGRESS)
    {
      NNLOG (LOG_ERR, "can't connect to %s fd %d : %s",
             sockUnionLog (&su), fd, strerror(errno));
      return CONNECT_ERROR;
    }
  }

  fcntl (fd, F_SETFL, val);

  return CONNECT_IN_PROGRESS;
}

/* Make socket from sockunion union. */
Int32T
sockUnionStreamSocket (uSockUnionT *pSu)
{
  Int32T sock = 0;

  if (pSu->sa.sa_family == 0)
    pSu->sa.sa_family = AF_INET_UNION;

  sock = socket (pSu->sa.sa_family, SOCK_STREAM, 0);

  if (sock < 0)
    NNLOG (LOG_WARNING, "can't make socket sockUnionStreamSocket");

  return sock;
}

/* Bind socket to specified address. */
Int32T
sockUnionBind (Int32T sock, uSockUnionT *pSU, Uint16T port, 
               uSockUnionT *pSUAddr)
{
  Int32T size = 0;
  Int32T ret = 0;

  if (pSU->sa.sa_family == AF_INET)
  {
    size = sizeof (struct sockaddr_in);
    pSU->sin.sin_port = htons (port);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
    pSU->sin.sin_len = size;
#endif /* HAVE_STRUCT_SOCKADDR_IN_SIN_LEN */
    if (pSUAddr == NULL)
      pSU->sin.sin_addr.s_addr = htonl (INADDR_ANY);
  }
#ifdef HAVE_IPV6
  else if (pSU->sa.sa_family == AF_INET6)
  {
    size = sizeof (struct sockaddr_in6);
    pSU->sin6.sin6_port = htons (port);
#ifdef SIN6_LEN
    pSU->sin6.sin6_len = size;
#endif /* SIN6_LEN */
    if (pSUAddr == NULL)
    {
#if defined(LINUX_IPV6) || defined(NRL)
      memset (&pSU->sin6.sin6_addr, 0, sizeof (struct in6_addr));
#else
      pSU->sin6.sin6_addr = in6addr_any;
#endif /* LINUX_IPV6 */
    }
  }
#endif /* HAVE_IPV6 */
  

  ret = bind (sock, (struct sockaddr *)pSU, size);
  if (ret < 0)
    NNLOG (LOG_WARNING, "can't bind socket : %s", strerror(errno));

  return ret;
}

Int32T
sockOptReuseAddr (Int32T sock)
{
  Int32T ret = 0;
  Int32T on = 1;

  ret = setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, 
                    (void *) &on, sizeof (on));
  if (ret < 0)
  {
    NNLOG (LOG_WARNING, "can't set sockopt SO_REUSEADDR to socket %d", sock);
    return -1;
  }
  return 0;
}

#ifdef SO_REUSEPORT
Int32T
sockOptReusePort (Int32T sock)
{
  Int32T ret = 0;
  Int32T on = 1;

  ret = setsockopt (sock, SOL_SOCKET, SO_REUSEPORT, 
                    (void *) &on, sizeof (on));
  if (ret < 0)
  {
    NNLOG (LOG_WARNING, "can't set sockopt SO_REUSEPORT to socket %d", sock);
    return -1;
  }
  return 0;
}
#else
Int32T
sockOptReusePort (Int32T sock)
{
  return 0;
}
#endif /* 0 */

/* Utility function to set boradcast option to the socket. */
Int32T
sockOptBroadcast (Int32T sock)
{
  Int32T ret = 0;
  Int32T on = 1; 

  ret = setsockopt (sock, SOL_SOCKET, SO_BROADCAST, (char *) &on, sizeof on); 
  if (ret < 0) 
  {
    NNLOG(LOG_DEBUG,"can't set sockopt SO_BROADCAST to socket %d\n", sock);
    return -1;
  }

  return 0;
}

Int32T
sockOptTtl (Int32T family, Int32T sock, Int32T ttl)
{
  Int32T ret = 0;

#ifdef IP_TTL
  if (family == AF_INET)
  {
    ret = setsockopt (sock, IPPROTO_IP, IP_TTL, 
                      (void *) &ttl, sizeof (Int32T));
    if (ret < 0)
    {
      NNLOG (LOG_WARNING, "can't set sockopt IP_TTL %d to socket %d", ttl, sock);
      return -1;
    }
    return 0;
  }
#endif /* IP_TTL */
#ifdef HAVE_IPV6
  if (family == AF_INET6)
  {
    ret = setsockopt (sock, IPPROTO_IPV6, IPV6_UNICAST_HOPS, 
                      (void *) &ttl, sizeof (Int32T));
    if (ret < 0)
    {
      NNLOG (LOG_WARNING, "can't set sockopt IPV6_UNICAST_HOPS %d to socket %d",
             ttl, sock);
      return -1;
    }
    return 0;
  }
#endif /* HAVE_IPV6 */
  return 0;
}

/* If same family and same prefix return 1. */
Int32T
sockUnionSame (uSockUnionT *pSu1, uSockUnionT *pSu2)
{
  Int32T ret = 0;

  if (pSu1->sa.sa_family != pSu2->sa.sa_family)
    return 0;

  switch (pSu1->sa.sa_family)
  {
    case AF_INET:
      ret = memcmp (&pSu1->sin.sin_addr, &pSu2->sin.sin_addr,
                    sizeof (struct in_addr));
      break;
#ifdef HAVE_IPV6
    case AF_INET6:
      ret = memcmp (&pSu1->sin6.sin6_addr, &pSu2->sin6.sin6_addr,
                    sizeof (struct in6_addr));
      break;
#endif /* HAVE_IPV6 */
  }
  if (ret == 0)
    return 1;
  else
    return 0;
}

/* After TCP connection is established.  Get local address and port. */
uSockUnionT *
sockUnionGetSockName (Int32T fd)
{
  Int32T ret = 0;
  socklen_t len = 0;
  union
  {
    struct sockaddr sa;
    struct sockaddr_in sin;
#ifdef HAVE_IPV6
    struct sockaddr_in6 sin6;
#endif /* HAVE_IPV6 */
    char tmp_buffer[128];
  } name;
  uSockUnionT *su = NULL;

  memset (&name, 0, sizeof name);
  len = sizeof name;

  ret = getsockname (fd, (struct sockaddr *)&name, &len);
  if (ret < 0)
  {
    NNLOG (LOG_ERR, "Can't get local address and port by getsockname: %s",
           strerror(errno));
    return NULL;
  }

  if (name.sa.sa_family == AF_INET)
  {
    su = NNMALLOC (MEM_SOCKET_UNION, sizeof (uSockUnionT));
    memcpy (su, &name, sizeof (struct sockaddr_in));
    return su;
  }
#ifdef HAVE_IPV6
  if (name.sa.sa_family == AF_INET6)
  {
    su = NNMALLOC (MEM_SOCKET_UNION, sizeof (uSockUnionT));
    memcpy (su, &name, sizeof (struct sockaddr_in6));
    sockUnionNormaliseMapped (su);
    return su;
  }
#endif /* HAVE_IPV6 */
  return NULL;
}

/* After TCP connection is established.  Get remote address and port. */
uSockUnionT *
sockUnionGetPeerName (Int32T fd)
{
  Int32T ret = 0;
  socklen_t len = 0;
  union
  {
    struct sockaddr sa;
    struct sockaddr_in sin;
#ifdef HAVE_IPV6
    struct sockaddr_in6 sin6;
#endif /* HAVE_IPV6 */
    char tmp_buffer[128];
  } name;
  uSockUnionT *su = NULL;

  memset (&name, 0, sizeof name);
  len = sizeof name;
  ret = getpeername (fd, (struct sockaddr *)&name, &len);
  if (ret < 0)
  {
    NNLOG (LOG_WARNING, "Can't get remote address and port: %s",
           strerror(errno));
    return NULL;
  }

  if (name.sa.sa_family == AF_INET)
  {
    su = NNMALLOC (MEM_SOCKET_UNION, sizeof (uSockUnionT));
    memcpy (su, &name, sizeof (struct sockaddr_in));
    return su;
  }
#ifdef HAVE_IPV6
  if (name.sa.sa_family == AF_INET6)
  {
    su = NNMALLOC (MEM_SOCKET_UNION, sizeof (uSockUnionT));
    memcpy (su, &name, sizeof (struct sockaddr_in6));
    sockUnionNormaliseMapped (su);
    return su;
  }
#endif /* HAVE_IPV6 */
  return NULL;
}

/* Print sockunion structure */
static void __attribute__ ((unused))
sockunion_print (uSockUnionT *pSu)
{
  if (pSu == NULL)
    return;

  switch (pSu->sa.sa_family) 
  {
    case AF_INET:
      NNLOG (LOG_DEBUG, "%s\n", inet_ntoa (pSu->sin.sin_addr));
      break;
#ifdef HAVE_IPV6
    case AF_INET6:
      {
      char buf [SU_ADDRSTRLEN] = {};
      NNLOG (LOG_DEBUG, "%s\n", inet_ntop (AF_INET6, &(pSu->sin6.sin6_addr),
              buf, sizeof (buf)));
      }
      break;
#endif /* HAVE_IPV6 */

#ifdef AF_LINK
    case AF_LINK:
      {
      struct sockaddr_dl *sdl;
      sdl = (struct sockaddr_dl *)&(pSu->sa);
      NNLOG (LOG_DEBUG, "link#%d\n", sdl->sdl_index);
      }
      break;
#endif /* AF_LINK */
    default:
      NNLOG (LOG_DEBUG, "af_unknown %d\n", pSu->sa.sa_family);
      break;
  }
}

#ifdef HAVE_IPV6
static Int32T
in6AddrCmp (struct in6_addr *pAddr1, struct in6_addr *pAddr2)
{
  Uint32T i = 0;
  Uint8T *p1, *p2;

  p1 = (Uint8T *)pAddr1;
  p2 = (Uint8T *)pAddr2;

  for (i = 0; i < sizeof (struct in6_addr); i++)
    {
      if (p1[i] > p2[i])
	return 1;
      else if (p1[i] < p2[i])
	return -1;
    }
  return 0;
}
#endif /* HAVE_IPV6 */

Int32T
sockUnionCmp (uSockUnionT *pSu1, uSockUnionT *pSu2)
{
  if (pSu1->sa.sa_family > pSu2->sa.sa_family)
    return 1;
  if (pSu1->sa.sa_family < pSu2->sa.sa_family)
    return -1;

  if (pSu1->sa.sa_family == AF_INET)
    {
      if (ntohl (pSu1->sin.sin_addr.s_addr) == ntohl (pSu2->sin.sin_addr.s_addr))
	return 0;
      if (ntohl (pSu1->sin.sin_addr.s_addr) > ntohl (pSu2->sin.sin_addr.s_addr))
	return 1;
      else
	return -1;
    }
#ifdef HAVE_IPV6
  if (pSu1->sa.sa_family == AF_INET6)
    return in6AddrCmp (&pSu1->sin6.sin6_addr, &pSu2->sin6.sin6_addr);
#endif /* HAVE_IPV6 */
  return 0;
}

/* Duplicate sockunion. */
uSockUnionT *
sockUnionDup (uSockUnionT *pSu)
{
  uSockUnionT *dup = NNMALLOC (MEM_SOCKET_UNION, sizeof (uSockUnionT));
  memcpy (dup, pSu, sizeof (uSockUnionT));
  return dup;
}

void
sockUnionFree (uSockUnionT *pSu)
{
  NNFREE (MEM_SOCKET_UNION, pSu);
}
