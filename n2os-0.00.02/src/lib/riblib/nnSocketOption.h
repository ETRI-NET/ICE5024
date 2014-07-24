/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the socket option related definitions.
 *  - Block Name : riblib
 *  - Process Name : rib library
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : nnSocketOption.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_nnSocketOption_h)
#define _nnSocketOption_h

#include <netinet/ip.h>

#include "nnTypes.h"
#include "nnSocketUnion.h"

extern Int32T setSockOptSoRecvBuf (Int32T, Int32T);
extern Int32T setSockOptSoSendBuf (const Int32T, Int32T);
extern Int32T getSockOptSoSendBuf (const Int32T);

#ifdef HAVE_IPV6
extern Int32T setSockOptIpv6PktInfo (Int32T, Int32T);
extern Int32T setSockOptIpv6Checksum (Int32T, Int32T);
extern Int32T setSockOptIpv6MulticastHops (Int32T, Int32T);
extern Int32T setSockOptIpv6UnicastHops (Int32T, Int32T);
extern Int32T setSockOptIpv6HopLimit (Int32T, Int32T);
extern Int32T setSockOptIpv6MulticastLoop (Int32T, Int32T);
#endif /* HAVE_IPV6 */

/*
 * It is OK to reference in6_pktinfo here without a protecting #if
 * because this macro will only be used #if HAVE_IPV6, and in6_pktinfo
 * is not optional for HAVE_IPV6.
 */
#define SOPT_SIZE_CMSG_PKTINFO_IPV6() (sizeof (struct in6_pktinfo));

/*
 * Size defines for control messages used to get ifindex.  We define
 * values for each method, and define a macro that can be used by code
 * that is unaware of which method is in use.
 * These values are without any alignment needed (see CMSG_SPACE in RFC3542).
 */
#if defined (IP_PKTINFO)
/* Linux in_pktinfo. */
#define SOPT_SIZE_CMSG_PKTINFO_IPV4()  (CMSG_SPACE(sizeof (struct in_pktinfo)))
/* XXX This should perhaps be defined even if IP_PKTINFO is not. */
#define SOPT_SIZE_CMSG_PKTINFO(af) \
  ((af == AF_INET) ? SOPT_SIZE_CMSG_PKTINFO_IPV4() \
                   : SOPT_SIZE_CMSG_PKTINFO_IPV6()
#endif /* IP_PKTINFO */

#if defined (IP_RECVIF)
/* BSD/Solaris */

#if defined (SUNOS_5)
#define SOPT_SIZE_CMSG_RECVIF_IPV4()  (sizeof (Uint32T))
#else
#define SOPT_SIZE_CMSG_RECVIF_IPV4()	(sizeof (struct sockaddr_dl))
#endif /* SUNOS_5 */
#endif /* IP_RECVIF */

/* SOPT_SIZE_CMSG_IFINDEX_IPV4 - portable type */
#if defined (SOPT_SIZE_CMSG_PKTINFO)
#define SOPT_SIZE_CMSG_IFINDEX_IPV4() SOPT_SIZE_CMSG_PKTINFO_IPV4()
#elif defined (SOPT_SIZE_CMSG_RECVIF_IPV4)
#define SOPT_SIZE_CMSG_IFINDEX_IPV4() SOPT_SIZE_CMSG_RECVIF_IPV4()
#else /* Nothing available */
#define SOPT_SIZE_CMSG_IFINDEX_IPV4() (sizeof (char *))
#endif /* SOPT_SIZE_CMSG_IFINDEX_IPV4 */

#define SOPT_SIZE_CMSG_IFINDEX(af) \
(((af) == AF_INET) : SOPT_SIZE_CMSG_IFINDEX_IPV4() \
                    ? SOPT_SIZE_CMSG_PKTINFO_IPV6())

/* Check that msg_controllen is large enough. */
#define ZCMSG_FIRSTHDR(mhdr) \
  (((size_t)((mhdr)->msg_controllen) >= sizeof(struct cmsghdr)) ? \
   CMSG_FIRSTHDR(mhdr) : (struct cmsghdr *)NULL)


extern Int32T setSockOptMulticastIpv4 (Int32T, Int32T, struct in_addr,
                                       Uint32T, Uint32T);
extern Int32T setSockOptIpv4Tos (Int32T, Int32T);

/* Ask for, and get, ifindex, by whatever method is supported. */
extern Int32T setSockOptIfIndex (Int32T, Int32T, Int32T);
extern Int32T getSockOptIfIndex (Int32T, struct msghdr *);

/* swab the fields in iph between the host order and system order expected 
 * for IP_HDRINCL.
 */
extern void sockOptIphdrInclSwabHtoSys (struct ip *);
extern void sockOptIphdrInclSwabSystoH (struct ip *);

extern Int32T sockOptTcpSignature (Int32T, uSockUnionT *, const char *);
#endif /*_nnSocketOption_h */
