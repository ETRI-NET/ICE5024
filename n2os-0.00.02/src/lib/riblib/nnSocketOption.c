/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : 각 컴포넌트에서 사용하는 공통 socket option 관련 기능을 
 *                제공한다.
 * - Block Name : riblib
 * - Process Name : rib library
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : nnSocketOption.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>

#include "nosLib.h"
#include "nnSocketUnion.h"
#include "nnSocketOption.h"

Int32T
setSockOptSoRecvBuf (Int32T sock, Int32T size)
{
  Int32T ret = 0;
  
  if ( (ret = setsockopt (sock, SOL_SOCKET, SO_RCVBUF, (char *)
                          &size, sizeof (Int32T))) < 0)
    NNLOG (LOG_ERR, "fd %d: can't setsockopt SO_RCVBUF to %d: %s",
           sock,size, strerror(errno));

  return ret;
}

Int32T
setSockOptSoSendBuf (const Int32T sock, Int32T size)
{
  Int32T ret = setsockopt (sock, SOL_SOCKET, SO_SNDBUF,
                           (char *)&size, sizeof (Int32T));
  
  if (ret < 0)
    NNLOG (LOG_ERR, "fd %d: can't setsockopt SO_SNDBUF to %d: %s",
           sock, size, strerror (errno));

  return ret;
}

Int32T
getSockOptSoSendBuf (const Int32T sock)
{
  Uint32T optVal = 0;
  socklen_t optLen = sizeof (optVal);
  Int32T ret = getsockopt (sock, SOL_SOCKET, SO_SNDBUF,
                           (char *)&optVal, &optLen);
  if (ret < 0)
  {
    NNLOG (LOG_ERR, "fd %d: can't getsockopt SO_SNDBUF: %d (%s)",
           sock, errno, strerror (errno));
    return ret;
  }
  return optVal;
}

static void *
getSockOptCmsgData (struct msghdr *pMsgHdr, Int32T level, Int32T type)
{
  struct cmsghdr *pCmsgHdr = NULL;
  void *ptr = NULL;
  
  for (pCmsgHdr = ZCMSG_FIRSTHDR(pMsgHdr); 
       pCmsgHdr != NULL;
       pCmsgHdr = CMSG_NXTHDR(pMsgHdr, pCmsgHdr))
    if (pCmsgHdr->cmsg_level == level && pCmsgHdr->cmsg_type)
      return (ptr = CMSG_DATA(pCmsgHdr));

  return NULL;
}

#ifdef HAVE_IPV6
/* Set IPv6 packet info to the socket. */
Int32T
setSockOptIpv6PktInfo (Int32T sock, Int32T val)
{
  Int32T ret = NULL;
    
#ifdef IPV6_RECVPKTINFO		/*2292bis-01*/
  ret = setsockopt(sock, IPPROTO_IPV6, IPV6_RECVPKTINFO, &val, sizeof(val));
  if (ret < 0)
    NNLOG (LOG_WARNING, "can't setsockopt IPV6_RECVPKTINFO : %s", strerror (errno));
#else	/*RFC2292*/
  ret = setsockopt(sock, IPPROTO_IPV6, IPV6_PKTINFO, &val, sizeof(val));
  if (ret < 0)
    NNLOG (LOG_WARNING, "can't setsockopt IPV6_PKTINFO : %s", strerror (errno));
#endif /* INIA_IPV6 */
  return ret;
}

/* Set multicast hops val to the socket. */
Int32T
setSockOptIpv6Checksum (Int32T sock, Int32T val)
{
  Int32T ret = 0;

#ifdef GNU_LINUX
  ret = setsockopt(sock, IPPROTO_RAW, IPV6_CHECKSUM, &val, sizeof(val));
#else
  ret = setsockopt(sock, IPPROTO_IPV6, IPV6_CHECKSUM, &val, sizeof(val));
#endif /* GNU_LINUX */
  if (ret < 0)
    NNLOG (LOG_WARNING, "can't setsockopt IPV6_CHECKSUM");
  return ret;
}

/* Set multicast hops val to the socket. */
Int32T
setSockOptIpv6MulticastHops (Int32T sock, Int32T val)
{
  Int32T ret = 0;

  ret = setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &val, sizeof(val));
  if (ret < 0)
    NNLOG (LOG_WARNING, "can't setsockopt IPV6_MULTICAST_HOPS");
  return ret;
}

/* Set multicast hops val to the socket. */
Int32T
setSockOptIpv6UnicastHops (Int32T sock, Int32T val)
{
  Int32T ret = 0;

  ret = setsockopt(sock, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &val, sizeof(val));
  if (ret < 0)
    NNLOG (LOG_WARNING, "can't setsockopt IPV6_UNICAST_HOPS");
  return ret;
}

Int32T
setSockOptIpv6HopLimit (Int32T sock, Int32T val)
{
  Int32T ret = 0;

#ifdef IPV6_RECVHOPLIMIT	/*2292bis-01*/
  ret = setsockopt (sock, IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &val, sizeof(val));
  if (ret < 0)
    NNLOG (LOG_WARNING, "can't setsockopt IPV6_RECVHOPLIMIT");
#else	/*RFC2292*/
  ret = setsockopt (sock, IPPROTO_IPV6, IPV6_HOPLIMIT, &val, sizeof(val));
  if (ret < 0)
    NNLOG (LOG_WARNING, "can't setsockopt IPV6_HOPLIMIT");
#endif
  return ret;
}

/* Set multicast loop zero to the socket. */
Int32T
setSockOptIpv6MulticastLoop (Int32T sock, Int32T val)
{
  Int32T ret = NULL;
    
  ret = setsockopt (sock, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &val,
		    sizeof (val));
  if (ret < 0)
    NNLOG (LOG_WARNING, "can't setsockopt IPV6_MULTICAST_LOOP");
  return ret;
}

static Int32T
getSockOptIpv6IfIndex (struct msghdr *pMsgHdr)
{
  struct in6_pktinfo *pPktInfo = NULL;
  
  pPktInfo = getSockOptCmsgData (pMsgHdr, IPPROTO_IPV6, IPV6_PKTINFO);
  
  return pPktInfo->ipi6_ifindex;
}
#endif /* HAVE_IPV6 */


/*
 * Process multicast socket options for IPv4 in an OS-dependent manner.
 * Supported options are IP_MULTICAST_IF and IP_{ADD,DROP}_MEMBERSHIP.
 *
 * Many operating systems have a limit on the number of groups that
 * can be joined per socket (where each group and local address
 * counts).  This impacts OSPF, which joins groups on each interface
 * using a single socket.  The limit is typically 20, derived from the
 * original BSD multicast implementation.  Some systems have
 * mechanisms for increasing this limit.
 *
 * In many 4.4BSD-derived systems, multicast group operations are not
 * allowed on interfaces that are not UP.  Thus, a previous attempt to
 * leave the group may have failed, leaving it still joined, and we
 * drop/join quietly to recover.  This may not be necessary, but aims to
 * defend against unknown behavior in that we will still return an error
 * if the second join fails.  It is not clear how other systems
 * (e.g. Linux, Solaris) behave when leaving groups on down interfaces,
 * but this behavior should not be harmful if they behave the same way,
 * allow leaves, or implicitly leave all groups joined to down interfaces.
 */
Int32T
setSockOptMulticastIpv4(Int32T sock, Int32T optName, 
                        struct in_addr ifAddr, Uint32T mcastAddr,
                        Uint32T ifIndex)
{

#ifdef HAVE_STRUCT_IP_MREQN_IMR_IFINDEX
  /* This is better because it uses ifIndex directly */
  struct ip_mreqn mreqn = {0,};
  Int32T ret = NULL;
  
  switch (optName)
  {
    case IP_MULTICAST_IF:
    case IP_ADD_MEMBERSHIP:
    case IP_DROP_MEMBERSHIP:
      memset (&mreqn, 0, sizeof(mreqn));

      if (mcastAddr)
        mreqn.imr_multiaddr.s_addr = mcastAddr;
      
      if (ifIndex)
        mreqn.imr_ifindex = ifIndex;
      else
        mreqn.imr_address = ifAddr;
      
      ret = setsockopt(sock, IPPROTO_IP, optName,
                       (void *)&mreqn, sizeof(mreqn));
      if ((ret < 0) && (optName == IP_ADD_MEMBERSHIP) && (errno == EADDRINUSE))
      {
        /* see above: handle possible problem when interface comes back up */
        char buf[2][INET_ADDRSTRLEN] = {};
        NNLOG (LOG_INFO, "setSockOptMulticastIpv4 attempting to drop and "
               "re-add (fd %d, ifaddr %s, mcast %s, ifIndex %u)",
               sock,
               inet_ntop(AF_INET, &ifAddr, buf[0], sizeof(buf[0])),
               inet_ntop(AF_INET, &mreqn.imr_multiaddr,
               buf[1], sizeof(buf[1])), ifIndex);
        setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
                   (void *)&mreqn, sizeof(mreqn));
        ret = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                         (void *)&mreqn, sizeof(mreqn));
      }
      return ret;
      break;

    default:
      /* Can out and give an understandable error */
      errno = EINVAL;
      return -1;
      break;
  }

  /* Example defines for another OS, boilerplate off other code in this
     function, AND handle optName as per other sections for consistency !! */
  /* #elif  defined(BOGON_NIX) && EXAMPLE_VERSION_CODE > -100000 */
  /* Add your favourite OS here! */

#else /* #if OS_TYPE */ 
  /* standard BSD API */

  struct in_addr m;
  struct ip_mreq mreq;
  Int32T ret = 0;

#ifdef HAVE_BSD_STRUCT_IP_MREQ_HACK
  if (ifIndex)
    m.s_addr = htonl(ifIndex);
  else
#endif
    m = ifAddr;

  switch (optName)
  {
    case IP_MULTICAST_IF:
      return setsockopt (sock, IPPROTO_IP, optName, (void *)&m, sizeof(m)); 
      break;

    case IP_ADD_MEMBERSHIP:
    case IP_DROP_MEMBERSHIP:
      memset (&mreq, 0, sizeof(mreq));
      mreq.imr_multiaddr.s_addr = mcastAddr;
      mreq.imr_interface = m;
      
      ret = setsockopt (sock, IPPROTO_IP, optName, (void *)&mreq, sizeof(mreq));
      if ((ret < 0) && (optName == IP_ADD_MEMBERSHIP) && (errno == EADDRINUSE))
      {
        /* see above: handle possible problem when interface comes back up */
        char buf[2][INET_ADDRSTRLEN];
        NNLOG(LOG_INFO, "setSockOptMulticastIpv4 attempting to drop and "
              "re-add (fd %d, ifaddr %s, mcast %s, ifIndex %u)",
              sock,
              inet_ntop(AF_INET, &ifAddr, buf[0], sizeof(buf[0])),
              inet_ntop(AF_INET, &mreq.imr_multiaddr,
              buf[1], sizeof(buf[1])), ifIndex);
        setsockopt (sock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
                    (void *)&mreq, sizeof(mreq));
        ret = setsockopt (sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                          (void *)&mreq, sizeof(mreq));
      }
      return ret;
      break;
      
    default:
      /* Can out and give an understandable error */
      errno = EINVAL;
      return -1;
      break;
  }
#endif /* #if OS_TYPE */

}

static Int32T
setSockOptIpv4IfIndex (Int32T sock, Int32T val)
{
  Int32T ret = 0;

#if defined (IP_PKTINFO)
  if ((ret = setsockopt (sock, IPPROTO_IP, IP_PKTINFO, &val, sizeof (val))) < 0)
    NNLOG (LOG_WARNING, "Can't set IP_PKTINFO option for fd %d to %d: %s",
           sock,val,strerror(errno));
#elif defined (IP_RECVIF)
  if ((ret = setsockopt (sock, IPPROTO_IP, IP_RECVIF, &val, sizeof (val))) < 0)
    NNLOG (LOG_WARNING, "Can't set IP_RECVIF option for fd %d to %d: %s",
           sock,val,strerror(errno));
#else
#warning "Neither IP_PKTINFO nor IP_RECVIF is available."
#warning "Will not be able to receive link info."
#warning "Things might be seriously broken.."
  /* XXX Does this ever happen?  Should there be a zlog_warn message here? */
  ret = -1;
#endif
  return ret;
}

Int32T
setSockOptIpv4Tos(Int32T sock, Int32T tos)
{
  Int32T ret = 0;

  ret = setsockopt (sock, IPPROTO_IP, IP_TOS, &tos, sizeof (tos));
  if (ret < 0)
    NNLOG (LOG_WARNING, "Can't set IP_TOS option for fd %d to %#x: %s",
           sock, tos, strerror(errno));
  return ret;
}


Int32T
setSockOptIfIndex (Int32T af, Int32T sock, Int32T val)
{
  Int32T ret = -1;
  
  switch (af)
  {
    case AF_INET:
      ret = setSockOptIpv4IfIndex (sock, val);
      break;
#ifdef HAVE_IPV6
    case AF_INET6:
      ret = setSockOptIpv6PktInfo (sock, val);
      break;
#endif
    default:
      NNLOG (LOG_WARNING, "setSockOptIfIndex: unknown address family %d", af);
  }
  return ret;
}
  
/*
 * Requires: pMsgHdr is not NULL and points to a valid struct msghdr, which
 * may or may not have control data about the incoming interface.
 *
 * Returns the interface index (small integer >= 1) if it can be
 * determined, or else 0.
 */
static Int32T
getSockOptIpv4IfIndex (struct msghdr *pMsgHdr)
{
  /* XXX: initialize to zero?  (Always overwritten, so just cosmetic.) */
  Int32T ifIndex = -1;

#if defined(IP_PKTINFO)
/* Linux pktinfo based ifIndex retrieval */
  struct in_pktinfo *pktinfo;
  
  pktinfo = 
    (struct in_pktinfo *)getSockOptCmsgData (pMsgHdr, IPPROTO_IP, IP_PKTINFO);
  /* XXX Can pktinfo be NULL?  Clean up post 0.98. */
  ifIndex = pktinfo->ipi_ifindex;
  
#elif defined(IP_RECVIF)

  /* retrieval based on IP_RECVIF */

#ifndef SUNOS_5
  /* BSD systems use a sockaddr_dl as the control message payload. */
  struct sockaddr_dl *sdl;
#else
  /* SUNOS_5 uses an integer with the index. */
  Int32T *ifindex_p;
#endif /* SUNOS_5 */

#ifndef SUNOS_5
  /* BSD */
  sdl = 
    (struct sockaddr_dl *)getSockOptCmsgData (pMsgHdr, IPPROTO_IP, IP_RECVIF);
  if (sdl != NULL)
    ifIndex = sdl->sdl_index;
  else
    ifIndex = 0;
#else
  /*
   * Solaris.  On Solaris 8, IP_RECVIF is defined, but the call to
   * enable it fails with errno=99, and the struct msghdr has
   * controllen 0.
   */
  ifindex_p = (Uint32T *)getSockOptCmsgData (pMsgHdr, IPPROTO_IP, IP_RECVIF); 
  if (ifindex_p != NULL)
    ifIndex = *ifindex_p;
  else
    ifIndex = 0;
#endif /* SUNOS_5 */

#else
  /*
   * Neither IP_PKTINFO nor IP_RECVIF defined - warn at compile time.
   * XXX Decide if this is a core service, or if daemons have to cope.
   * Since Solaris 8 and OpenBSD seem not to provide it, it seems that
   * daemons have to cope.
   */
#warning "getSockOptIpv4IfIndex: Neither IP_PKTINFO nor IP_RECVIF defined."
#warning "Some daemons may fail to operate correctly!"
  ifIndex = 0;

#endif /* IP_PKTINFO */ 

  return ifIndex;
}

/* return ifindex, 0 if none found */
Int32T
getSockOptIfIndex (Int32T af, struct msghdr *pMsgHdr)
{
  Int32T ifIndex = 0;
  
  switch (af)
  {
    case AF_INET:
      return (getSockOptIpv4IfIndex (pMsgHdr));
      break;
#ifdef HAVE_IPV6
    case AF_INET6:
      return (getSockOptIpv6IfIndex (pMsgHdr));
      break;
#endif
    default:
      NNLOG (LOG_WARNING, "getSockOptIfIndex: unknown address family %d", af);
      return (ifIndex = 0);
  }
}

/* swab iph between order system uses for IP_HDRINCL and host order */
void
sockOptIphdrInclSwabHtoSys (struct ip *pIpHdr)
{
  /* BSD and derived take pIpHdr in network order, except for 
   * ip_len and ip_off
   */
#ifndef HAVE_IP_HDRINCL_BSD_ORDER
  pIpHdr->ip_len = htons(pIpHdr->ip_len);
  pIpHdr->ip_off = htons(pIpHdr->ip_off);
#endif /* HAVE_IP_HDRINCL_BSD_ORDER */

  pIpHdr->ip_id = htons(pIpHdr->ip_id);
}

void
sockOptIphdrInclSwabSystoH (struct ip *pIpHdr)
{
#ifndef HAVE_IP_HDRINCL_BSD_ORDER
  pIpHdr->ip_len = ntohs(pIpHdr->ip_len);
  pIpHdr->ip_off = ntohs(pIpHdr->ip_off);
#endif /* HAVE_IP_HDRINCL_BSD_ORDER */

  pIpHdr->ip_id = ntohs(pIpHdr->ip_id);
}

Int32T
sockOptTcpSignature (Int32T sock, uSockUnionT *pSu, const char *password)
{
#if defined(HAVE_TCP_MD5_LINUX24) && defined(GNU_LINUX)
  /* Support for the old Linux 2.4 TCP-MD5 patch, taken from Hasso Tepper's
   * version of the Quagga patch (based on work by Rick Payne, and Bruce
   * Simpson)
   */
#define TCP_MD5_AUTH 13
#define TCP_MD5_AUTH_ADD 1
#define TCP_MD5_AUTH_DEL 2
  struct tcp_rfc2385_cmd {
    Uint8T     command;    /* Command - Add/Delete */
    Uint32T    address;    /* IPV4 address associated */
    Uint8T     keylen;     /* MD5 Key len (do NOT assume 0 terminated ascii) */
    void         *key;       /* MD5 Key */
  } cmd;
  struct in_addr *addr = &pSu->sin.sin_addr;
  
  cmd.command = (password != NULL ? TCP_MD5_AUTH_ADD : TCP_MD5_AUTH_DEL);
  cmd.address = addr->s_addr;
  cmd.keylen = (password != NULL ? strlen (password) : 0);
  cmd.key = password;
  
  return setsockopt (sock, IPPROTO_TCP, TCP_MD5_AUTH, &cmd, sizeof cmd);
  
#elif HAVE_DECL_TCP_MD5SIG
  Int32T ret = NULL;
#ifndef GNU_LINUX
  /*
   * XXX Need to do PF_KEY operation here to add/remove an SA entry,
   * and add/remove an SP entry for this peer's packet flows also.
   */
  Int32T md5sig = password && *password ? 1 : 0;
#else
  Int32T keyLen = password ? strlen (password) : 0;
  struct tcp_md5sig md5sig;
  union sockunion *pSu2, *pSuSock;
  
  /* Figure out whether the socket and the sockunion are the same family..
   * adding AF_INET to AF_INET6 needs to be v4 mapped, you'd think..
   */
  if (!(pSuSock = sockunion_getsockname (sock)))
    return -1;
  
  if (pSuSock->sa.sa_family == pSu->sa.sa_family)
    pSu2 = pSu;
  else
  {
    /* oops.. */
    pSu2 = pSuSock;
      
    if (pSu2->sa.sa_family == AF_INET)
    {
      sockunion_free (pSuSock);
      return 0;
    }
      
#ifdef HAVE_IPV6
    /* If this does not work, then all users of this sockopt will need to
     * differentiate between IPv4 and IPv6, and keep seperate sockets for
     * each. 
     *
     * Sadly, it doesn't seem to work at present. It's unknown whether
     * this is a bug or not.
     */
    if (pSu2->sa.sa_family == AF_INET6 && pSu->sa.sa_family == AF_INET)
    {
      pSu2->sin6.sin6_family = AF_INET6;
      /* V4Map the address */
      memset (&pSu2->sin6.sin6_addr, 0, sizeof (struct in6_addr));
      pSu2->sin6.sin6_addr.s6_addr32[2] = htonl(0xffff);
      memcpy (&pSu2->sin6.sin6_addr.s6_addr32[3], &pSu->sin.sin_addr, 4);
    }
#endif
  }
  
  memset (&md5sig, 0, sizeof (md5sig));
  memcpy (&md5sig.tcpm_addr, pSu2, sizeof (*pSu2));
  md5sig.tcpm_keylen = keyLen;
  if (keyLen)
    memcpy (md5sig.tcpm_key, password, keyLen);
  sockunion_free (pSuSock);
#endif /* GNU_LINUX */
  if ((ret = setsockopt (sock, IPPROTO_TCP, TCP_MD5SIG, &md5sig, sizeof md5sig)) < 0)
  {
    /* ENOENT is harmless.  It is returned when we clear a password for which
       one was not previously set. */
    if (ENOENT == errno)
      ret = 0;
    else
      NNLOG (LOG_ERR, "sockopt_tcp_signature: setsockopt(%d): %s",
             sock, strerror(errno));
  }
  return ret;
#else /* HAVE_TCP_MD5SIG */
  return -2;
#endif /* !HAVE_TCP_MD5SIG */
}
