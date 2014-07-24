/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : RIP Protocol의 SNMP 설정을 제어하는 화일
 *
 * - Block Name : RIP Protocol
 * - Process Name : ripd
 * - Creator : Geontae Park
 * - Initial Date : 2014/02/20
 */

/**
 * @file        : ripSnmp.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#ifdef HAVE_SNMP
#ifdef HAVE_NETSNMP
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#else
#include <asn1.h>
#include <snmp.h>
#include <snmp_impl.h>
#endif

#include "log.h"
#include "command.h"
#include "smux.h"

#include "nnTypes.h"
#include "nnPrefix.h"
#include "nnTable.h"

#include "ripd.h"

/* RIPv2-MIB. */
#define RIPV2MIB 1,3,6,1,2,1,23

/* RIPv2-MIB rip2Globals values. */
#define RIP2GLOBALROUTECHANGES  1
#define RIP2GLOBALQUERIES       2

/* RIPv2-MIB rip2IfStatEntry. */
#define RIP2IFSTATENTRY         1

/* RIPv2-MIB rip2IfStatTable. */
#define RIP2IFSTATADDRESS       1
#define RIP2IFSTATRCVBADPACKETS 2
#define RIP2IFSTATRCVBADROUTES  3
#define RIP2IFSTATSENTUPDATES   4
#define RIP2IFSTATSTATUS        5

/* RIPv2-MIB rip2IfConfTable. */
#define RIP2IFCONFADDRESS       1
#define RIP2IFCONFDOMAIN        2
#define RIP2IFCONFAUTHTYPE      3
#define RIP2IFCONFAUTHKEY       4
#define RIP2IFCONFSEND          5
#define RIP2IFCONFRECEIVE       6
#define RIP2IFCONFDEFAULTMETRIC 7
#define RIP2IFCONFSTATUS        8
#define RIP2IFCONFSRCADDRESS    9

/* RIPv2-MIB rip2PeerTable. */
#define RIP2PEERADDRESS         1
#define RIP2PEERDOMAIN          2
#define RIP2PEERLASTUPDATE      3
#define RIP2PEERVERSION         4
#define RIP2PEERRCVBADPACKETS   5
#define RIP2PEERRCVBADROUTES    6

/* SNMP value hack. */
#define COUNTER     ASN_COUNTER
#define INTEGER     ASN_INTEGER
#define TIMETICKS   ASN_TIMETICKS
#define IPADDRESS   ASN_IPADDRESS
#define STRING      ASN_OCTET_STR

/* Define SNMP local variables. */
SNMP_LOCAL_VARIABLES

/* RIP-MIB instances. */
oid ripOid [] = { RIPV2MIB };

/* Hook functions. */
static Uint8T *rip2Globals (struct variable *, oid [], size_t *,
                            Int32T, size_t *, WriteMethod **);
static Uint8T *rip2IfStatEntry (struct variable *, oid [], size_t *,
                                Int32T, size_t *, WriteMethod **);
static Uint8T *rip2IfConfAddress (struct variable *, oid [], size_t *,
                                  Int32T, size_t *, WriteMethod **);
static Uint8T *rip2PeerTable (struct variable *, oid [], size_t *,
                              Int32T, size_t *, WriteMethod **);

struct variable rip_variables[] = 
{
  /* RIP Global Counters. */
  {RIP2GLOBALROUTECHANGES,    COUNTER, RONLY, rip2Globals,
   2, {1, 1}},
  {RIP2GLOBALQUERIES,         COUNTER, RONLY, rip2Globals,
   2, {1, 2}},
  /* RIP Interface Tables. */
  {RIP2IFSTATADDRESS,         IPADDRESS, RONLY, rip2IfStatEntry,
   3, {2, 1, 1}},
  {RIP2IFSTATRCVBADPACKETS,   COUNTER, RONLY, rip2IfStatEntry,
   3, {2, 1, 2}},
  {RIP2IFSTATRCVBADROUTES,    COUNTER, RONLY, rip2IfStatEntry,
   3, {2, 1, 3}},
  {RIP2IFSTATSENTUPDATES,     COUNTER, RONLY, rip2IfStatEntry,
   3, {2, 1, 4}},
  {RIP2IFSTATSTATUS,          COUNTER, RWRITE, rip2IfStatEntry,
   3, {2, 1, 5}},
  {RIP2IFCONFADDRESS,         IPADDRESS, RONLY, rip2IfConfAddress,
   /* RIP Interface Configuration Table. */
   3, {3, 1, 1}},
  {RIP2IFCONFDOMAIN,          STRING, RONLY, rip2IfConfAddress,
   3, {3, 1, 2}},
  {RIP2IFCONFAUTHTYPE,        COUNTER, RONLY, rip2IfConfAddress,
   3, {3, 1, 3}},
  {RIP2IFCONFAUTHKEY,         STRING, RONLY, rip2IfConfAddress,
   3, {3, 1, 4}},
  {RIP2IFCONFSEND,            COUNTER, RONLY, rip2IfConfAddress,
   3, {3, 1, 5}},
  {RIP2IFCONFRECEIVE,         COUNTER, RONLY, rip2IfConfAddress,
   3, {3, 1, 6}},
  {RIP2IFCONFDEFAULTMETRIC,   COUNTER, RONLY, rip2IfConfAddress,
   3, {3, 1, 7}},
  {RIP2IFCONFSTATUS,          COUNTER, RONLY, rip2IfConfAddress,
   3, {3, 1, 8}},
  {RIP2IFCONFSRCADDRESS,      IPADDRESS, RONLY, rip2IfConfAddress,
   3, {3, 1, 9}},
  {RIP2PEERADDRESS,           IPADDRESS, RONLY, rip2PeerTable,
   /* RIP Peer Table. */
   3, {4, 1, 1}},
  {RIP2PEERDOMAIN,            STRING, RONLY, rip2PeerTable,
   3, {4, 1, 2}},
  {RIP2PEERLASTUPDATE,        TIMETICKS, RONLY, rip2PeerTable,
   3, {4, 1, 3}},
  {RIP2PEERVERSION,           INTEGER, RONLY, rip2PeerTable,
   3, {4, 1, 4}},
  {RIP2PEERRCVBADPACKETS,     COUNTER, RONLY, rip2PeerTable,
   3, {4, 1, 5}},
  {RIP2PEERRCVBADROUTES,      COUNTER, RONLY, rip2PeerTable,
   3, {4, 1, 6}}
};

extern struct thread_master *master;

static Uint8T *
rip2Globals (struct variable *v, oid name[], size_t *length,
	     Int32T exact, size_t *var_len, WriteMethod **write_method)
{
  if (smux_header_generic(v, name, length, exact, var_len, write_method)
      == MATCH_FAILED)
    return NULL;

  /* Retrun global counter. */
  switch (v->magic)
  {
    case RIP2GLOBALROUTECHANGES:
      return SNMP_INTEGER (pRip->ripGlobalRouteChanges);
      break;

    case RIP2GLOBALQUERIES:
      return SNMP_INTEGER (pRip->ripGlobalQueries);
      break;

    default:
      return NULL;
      break;
  }

  return NULL;
}

void
ripIfaddrAdd (InterfaceT *pIf, ConnectedT *pIfc)
{
  PrefixT *p = NULL;
  RouteNodeT *rn = NULL;

  p = pIfc->address;

  if (p->family != AF_INET)
    return;

  rn = nnRouteNodeGet (pRip->pRipIfaddrTable, p);
  rn->info = pIf;
}

void
ripIfaddrDelete (InterfaceT *pIf, ConnectedT *pIfc)
{
  PrefixT *p = NULL;
  RouteNodeT *rn = NULL;
  InterfaceT *i = NULL;

  p = ifc->address;

  if (p->family != AF_INET)
    return;

  rn = nnRouteNodeLookup (pRip->pRipIfaddrTable, p);
  if (! rn)
    return;
  i = rn->info;
  if (rn && !strncmp(i->name,pIf->name,INTERFACE_NAMSIZ))
  {
    rn->info = NULL;
    nnRouteNodeUnlock (rn);
    nnRouteNodeUnlock (rn);
  }
}

static InterfaceT *
ripIfaddrLookupNext (struct in_addr *addr)
{
  Prefix4T p = NULL;
  RouteNodeT *rn = NULL;
  InterfaceT *pIf = NULL;

  p.family = AF_INET;
  p.prefixLen = IPV4_MAX_BITLEN;
  p.prefix = *addr;

  rn = nnRouteNodeGet (pRip->pRipIfaddrTable, (PrefixT *) &p);

  for (rn = nnRouteNext (rn); rn; rn = nnRouteNext (rn))
    if (rn->info)
      break;

  if (rn && rn->info)
  {
    pIf = rn->info;
    *addr = rn->p.u.prefix4;
    nnRouteNodeUnlock (rn);
    return pIf;
  }

  return NULL;
}

static InterfaceT *
rip2IfLookup (struct variable *v, oid name[], size_t *length, 
              struct in_addr *addr, Int32T exact)
{
  Int32T len;
  InterfaceT * pIf = NULL;

  if (exact)
  {
    /* Check the length. */
    if (*length - v->namelen != sizeof (struct in_addr))
      return NULL;

    /* Defined in smux.[ch] */
    oid2in_addr (name + v->namelen, sizeof (struct in_addr), addr);

    return ifLookupExactAddress (*addr);
  }
  else
  {
    len = *length - v->namelen;
    if (len > 4) len = 4;

    oid2in_addr (name + v->namelen, len, addr);

    pIf = ripIfaddrLookupNext (addr);

    if (pIf == NULL)
      return NULL;

    /* Defined in smux.[ch] */
    oid_copy_addr (name + v->namelen, addr, sizeof (struct in_addr));

    *length = v->namelen + sizeof (struct in_addr);

    return pIf;
  }

  return NULL;
}

static RipPeerT *
rip2PeerLookup (struct variable *v, oid name[], size_t *length, 
                struct in_addr *addr, Int32T exact)
{
  Int32T len;
  RipPeerT * peer = NULL;

  if (exact)
  {
    /* Check the length. */
    if (*length - v->namelen != sizeof (struct in_addr) + 1)
      return NULL;

    oid2in_addr (name + v->namelen, sizeof (struct in_addr), addr);

    peer = ripPeerLookup (addr);

    if (peer->domain == name[v->namelen + sizeof (struct in_addr)])
      return peer;

    return NULL;
  }
  else
  {
    len = *length - v->namelen;
    if (len > 4) len = 4;

    oid2in_addr (name + v->namelen, len, addr);

    len = *length - v->namelen;
    peer = ripPeerLookup (addr);
    if (peer)
    {
      if ((len < sizeof (struct in_addr) + 1) ||
          (peer->domain > name[v->namelen + sizeof (struct in_addr)]))
      {
        oid_copy_addr (name + v->namelen, &peer->addr,
                       sizeof (struct in_addr));
        name[v->namelen + sizeof (struct in_addr)] = peer->domain;
        *length = sizeof (struct in_addr) + v->namelen + 1;
        return peer;
      }
    } 
    peer = ripPeerLookupNext (addr);

    if (! peer)
      return NULL;

    oid_copy_addr (name + v->namelen, &peer->addr, sizeof (struct in_addr));
    name[v->namelen + sizeof (struct in_addr)] = peer->domain;
    *length = sizeof (struct in_addr) + v->namelen + 1;

    return peer;
  }

  return NULL;
}

static Uint8T *
rip2IfStatEntry (struct variable *v, oid name[], size_t *length,
                 Int32T exact, size_t *var_len, WriteMethod **write_method)
{
  InterfaceT *pIf = NULL;
  RipInterfaceT *pRIf = NULL;
  static struct in_addr addr;
  static Int32T valid = SNMP_VALID;

  memset (&addr, 0, sizeof (struct in_addr));
  
  /* Lookup interface. */
  pIf = rip2IfLookup (v, name, length, &addr, exact);
  if (! pIf)
    return NULL;

  /* Fetch rip_interface information. */
  pRIf = pIf->info;

  switch (v->magic)
  {
    case RIP2IFSTATADDRESS:
      return SNMP_IPADDRESS (addr);

    case RIP2IFSTATRCVBADPACKETS:
      *var_len = sizeof (Int32T);
      return (Uint8T *) &pRIf->recvBadpackets;

    case RIP2IFSTATRCVBADROUTES:
      *var_len = sizeof (Int32T);
      return (Uint8T *) &pRIf->recvBadroutes;

    case RIP2IFSTATSENTUPDATES:
      *var_len = sizeof (Int32T);
      return (Uint8T *) &pRIf->sentUpdates;

    case RIP2IFSTATSTATUS:
      *var_len = sizeof (Int32T);
      v->type = ASN_INTEGER;
      return (Uint8T *) &valid;

    default:
      return NULL;
  }

  return NULL;
}

static Int32T
rip2IfConfSend (RipInterfaceT *pRIf)
{
#define doNotSend       1
#define ripVersion1     2
#define rip1Compatible  3
#define ripVersion2     4
#define ripV1Demand     5
#define ripV2Demand     6

  if (! pRIf->running)
    return doNotSend;
    
  if (pRIf->riSend & RIPv2)
    return ripVersion2;
  else if (pRIf->riSend & RIPv1)
    return ripVersion1;
  else if (pRip)
  {
    if (pRip->versionSend == RIPv2)
      return ripVersion2;
    else if (pRip->versionSend == RIPv1)
      return ripVersion1;
  }

  return doNotSend;
}

static Int32T
rip2IfConfReceive (RipInterfaceT *pRIf)
{
#define rip1            1
#define rip2            2
#define rip1OrRip2      3
#define doNotReceive    4

  Int32T recvv;

  if (! pRIf->running)
    return doNotReceive;

  recvv = (pRIf->riReceive == RI_RIP_UNSPEC) ?  pRip->versionRecv :
                                              pRIf->riReceive;
  if (recvv == RI_RIP_VERSION_1_AND_2)
  {
    return rip1OrRip2;
  }
  else if (recvv & RIPv2)
  {
    return rip2;
  }
  else if (recvv & RIPv1)
  {
    return rip1;
  }
  else
  {
    return doNotReceive;
  }
}

static Uint8T *
rip2IfConfAddress (struct variable *v, oid name[], size_t *length,
                  Int32T exact, size_t *val_len, WriteMethod **write_method)
{
  static struct in_addr addr;
  static Int32T valid = SNMP_INVALID;
  static Int32T domain = 0;
  static Int32T config = 0;
  static u_int auth = 0;
  InterfaceT *pIf = NULL;
  RipInterfaceT *pRIf = NULL;

  memset (&addr, 0, sizeof (struct in_addr));
  
  /* Lookup interface. */
  pIf = rip2IfLookup (v, name, length, &addr, exact);
  if (! pIf)
    return NULL;

  /* Fetch rip_interface information. */
  pRIf = pIf->info;

  switch (v->magic)
  {
    case RIP2IFCONFADDRESS:
      *val_len = sizeof (struct in_addr);
      return (Uint8T *) &addr;

    case RIP2IFCONFDOMAIN:
      *val_len = 2;
      return (Uint8T *) &domain;

    case RIP2IFCONFAUTHTYPE:
      auth = pRIf->auth_type;
      *val_len = sizeof (Int32T);
      v->type = ASN_INTEGER;
      return (Uint8T *)&auth;

    case RIP2IFCONFAUTHKEY:
      *val_len = 0;
      return (Uint8T *) &domain;
    case RIP2IFCONFSEND:
      config = rip2IfConfSend (pRIf);
      *val_len = sizeof (Int32T);
      v->type = ASN_INTEGER;
      return (Uint8T *) &config;
    case RIP2IFCONFRECEIVE:
      config = rip2IfConfReceive (pRIf);
      *val_len = sizeof (Int32T);
      v->type = ASN_INTEGER;
      return (Uint8T *) &config;

    case RIP2IFCONFDEFAULTMETRIC:
      *val_len = sizeof (Int32T);
      v->type = ASN_INTEGER;
      return (Uint8T *) &ifp->metric;
    case RIP2IFCONFSTATUS:
      *val_len = sizeof (Int32T);
      v->type = ASN_INTEGER;
      return (Uint8T *) &valid;
    case RIP2IFCONFSRCADDRESS:
      *val_len = sizeof (struct in_addr);
      return (Uint8T *) &addr;

    default:
      return NULL;

  }

  return NULL;
}

static Uint8T *
rip2PeerTable (struct variable *v, oid name[], size_t *length,
               Int32T exact, size_t *val_len, WriteMethod **write_method)
{
  static struct in_addr addr;
  static Int32T domain = 0;
  static Int32T version = 0;
  /* static time_t uptime; */

  RipPeerT *pPeer = NULL;

  memset (&addr, 0, sizeof (struct in_addr));
  
  /* Lookup interface. */
  pPeer = rip2PeerLookup (v, name, length, &addr, exact);
  if (! pPeer)
    return NULL;

  switch (v->magic)
    {
    case RIP2PEERADDRESS:
      *val_len = sizeof (struct in_addr);
      return (Uint8T *) &pPeer->addr;

    case RIP2PEERDOMAIN:
      *val_len = 2;
      return (Uint8T *) &domain;

    case RIP2PEERLASTUPDATE:
#if 0 
      /* We don't know the SNMP agent startup time. We have two choices here:
       * - assume ripd startup time equals SNMP agent startup time
       * - don't support this variable, at all
       * Currently, we do the latter...
       */
      *val_len = sizeof (time_t);
      uptime = pPeer->uptime; /* now - snmp_agent_startup - pPeer->uptime */
      return (Uint8T *) &uptime;
#else
      return (Uint8T *) NULL;
#endif

    case RIP2PEERVERSION:
      *val_len = sizeof (Int32T);
      version = pPeer->version;
      return (Uint8T *) &version;

    case RIP2PEERRCVBADPACKETS:
      *val_len = sizeof (Int32T);
      return (Uint8T *) &pPeer->recvBadpackets;

    case RIP2PEERRCVBADROUTES:
      *val_len = sizeof (Int32T);
      return (Uint8T *) &pPeer->recvBadroutes;

    default:
      return NULL;

    }
  return NULL;
}

/* Register RIPv2-MIB. */
void
ripSnmpInit ()
{
  pRip->pRipIfaddrTable = nnRouteTableInit ();

  smux_init (master);
  REGISTER_MIB("mibII/rip", rip_variables, variable, ripOid);
}
#endif /* HAVE_SNMP */
