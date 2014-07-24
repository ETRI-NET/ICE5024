/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the socket union related definitions.
 *  - Block Name : riblib
 *  - Process Name : rib library
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : nnSocketUnion.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */


#if !defined(_nnSockUnion_h)
#define _nnSockUnion_h

#include <netinet/in.h>

typedef union uSockUnion 
{
  struct sockaddr sa;
  struct sockaddr_in sin;
#ifdef HAVE_IPV6
  struct sockaddr_in6 sin6;
#endif /* HAVE_IPV6 */
}uSockUnionT;

typedef enum eConnectResult
{
  CONNECT_ERROR,
  CONNECT_SUCCESS,
  CONNECT_IN_PROGRESS
}eConnectResult;

/* Default address family. */
#ifdef HAVE_IPV6
#define AF_INET_UNION AF_INET6
#else
#define AF_INET_UNION AF_INET
#endif

/* Sockunion address string length.  Same as INET6_ADDRSTRLEN. */
#define SU_ADDRSTRLEN 46

/* Macro to set link local index to the IPv6 address.  For KAME IPv6
   stack. */
#ifdef KAME
#define	IN6_LINKLOCAL_IFINDEX(a)  ((a).s6_addr[2] << 8 | (a).s6_addr[3])
#define SET_IN6_LINKLOCAL_IFINDEX(a, i) \
  do { \
    (a).s6_addr[2] = ((i) >> 8) & 0xff; \
    (a).s6_addr[3] = (i) & 0xff; \
  } while (0)
#else
#define	IN6_LINKLOCAL_IFINDEX(a)
#define SET_IN6_LINKLOCAL_IFINDEX(a, i)
#endif /* KAME */

/* shortcut macro to specify address field of struct sockaddr */
#define sock2ip(X)   (((struct sockaddr_in *)(X))->sin_addr.s_addr)
#ifdef HAVE_IPV6
#define sock2ip6(X)  (((struct sockaddr_in6 *)(X))->sin6_addr.s6_addr)
#endif /* HAVE_IPV6 */

#define sockunion_family(X)  (X)->sa.sa_family

/* Prototypes. */
extern Int32T strToSockUnion (const StringT, uSockUnionT *);
extern const char * sockUnionToStr (uSockUnionT *, StringT, Uint32T);
extern Int32T sockUnionCmp (uSockUnionT *, uSockUnionT *);
extern Int32T sockUnionSame (uSockUnionT *, uSockUnionT *);

extern StringT sockUnionSuToStr (uSockUnionT *);
extern uSockUnionT *sockUnionStrToSu (const StringT);
extern Int32T sockUnionAccept (Int32T, uSockUnionT *);
extern Int32T sockUnionStreamSocket (uSockUnionT *);
extern Int32T sockOptReuseAddr (Int32T);
extern Int32T sockOptReusePort (Int32T);
extern Int32T sockOptBroadcast (Int32T);
extern Int32T sockUnionBind (Int32T, uSockUnionT *, Uint16T, uSockUnionT *);
extern Int32T sockOptTtl (Int32T, Int32T, Int32T);
extern Int32T sockUnionSocket (uSockUnionT *);
extern const StringT inet_sutop (uSockUnionT *, StringT);
extern eConnectResult sockUnionConnect (Int32T, uSockUnionT *, Uint16T, Uint32T);
extern uSockUnionT *sockUnionGetSockName (Int32T);
extern uSockUnionT *sockUnionGetPeerName (Int32T);
extern uSockUnionT *sockUnionDup (uSockUnionT *);
extern void sockUnionFree (uSockUnionT *);


#endif /* _nnSockUnion_h */
