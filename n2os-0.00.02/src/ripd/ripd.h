/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the ripd related definitions.
 *  - Block Name : RIP Protocol
 *  - Process Name : ripd
 *  - Creator : geontae park
 *  - Initial Date : 2014/02/19
 */

/**
 * @file : ribd.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_ripd_h)
#define _ripd_h

#include "nnIf.h"
#include "nnTable.h"
#include "nnPrefix.h"
#include "nnVector.h"
#include "nnRoutemap.h"
#include "nnFilter.h"
#include "nnPlist.h"
#include "nnRibDefines.h"
#include "hash.h"
#include "nnCmdCmsh.h"

/* RIP */
#define RIP_BUFFER_MAX_SIZE           1024

/* RIP version number. */
#define RIPv1                            1
#define RIPv2                            2
/* N.B. stuff will break if
	(RIPv1 != RI_RIP_VERSION_1) || (RIPv2 != RI_RIP_VERSION_2) */


/* RIP command list. */
#define RIP_REQUEST                      1
#define RIP_RESPONSE                     2
#define RIP_TRACEON                      3	/* Obsolete */
#define RIP_TRACEOFF                     4	/* Obsolete */
#define RIP_POLL                         5
#define RIP_POLL_ENTRY                   6
#define RIP_COMMAND_MAX                  7

/* RIP metric infinity value.*/
#define RIP_METRIC_INFINITY             16

/* Normal RIP packet min and max size. */
#define RIP_PACKET_MINSIZ                4
#define RIP_PACKET_MAXSIZ              512

#define RIP_HEADER_SIZE                  4
#define RIP_RTE_SIZE                    20

/* Max count of routing table entry in one rip packet. */
#define RIP_MAX_RTE                     25

/* RIP version 2 multicast address. */
#ifndef INADDR_RIP_GROUP
#define INADDR_RIP_GROUP        0xe0000009    /* 224.0.0.9 */
#endif

/* RIP timers */
#define RIP_TRIGGERED_TIMER_DEFAULT      1    /* sec. */
#define RIP_UPDATE_TIMER_DEFAULT        30    /* sec. */
#define RIP_TIMEOUT_TIMER_DEFAULT      180    /* sec. */
#define RIP_GARBAGE_TIMER_DEFAULT      120    /* sec. */

/* RIP peer timeout value. */
#define RIP_PEER_TIMER_DEFAULT         180

/* RIP port number. */
#define RIP_PORT_DEFAULT               520

/* RIP route types. */
#define RIP_ROUTE_RTE                    0
#define RIP_ROUTE_STATIC                 1
#define RIP_ROUTE_DEFAULT                2
#define RIP_ROUTE_REDISTRIBUTE           3
#define RIP_ROUTE_INTERFACE              4

/* RIPv2 special RTE family types */
#define RIP_FAMILY_AUTH                  0xffff

/* RIPv2 authentication types, for RIP_FAMILY_AUTH RTE's */
#define RIP_NO_AUTH                      0
#define RIP_AUTH_DATA                    1
#define RIP_AUTH_SIMPLE_PASSWORD         2
#define RIP_AUTH_MD5                     3

/* RIPv2 Simple authentication */
#define RIP_AUTH_SIMPLE_SIZE            16

/* RIPv2 MD5 authentication. */
#define RIP_AUTH_MD5_SIZE               16
#define RIP_AUTH_MD5_COMPAT_SIZE        RIP_RTE_SIZE

/* RIP structure. */
struct Rip 
{
  /* RIP configuration. */
#define RIP_CONFIG_INIT   0
#define RIP_CONFIG_SET    1
#define RIP_CONFIG_UNSET  2

  Int32T isConfigured;

  /* RIP socket. */
  Int32T sock;
  void * pSockFdEvent; /* use taskFdUpdate(...). */

  /* Command related pointer. */
  void * pCmshGlobal;

  /* RIP 1 sec triggered timer task pointer. */
  void * pTimerTriggered;
  struct timeval tvTriggeredTime;  /* default 1sec. */

  /* RIP 30 sec update timer task pointer. */
  void * pTimerUpdate;
  struct timeval tvUpdatedTime;
 
  /* RIP time value. use in taskTimerSet, Del, Update. */
  struct timeval tvUpdateTime;
  struct timeval tvTimeoutTime;
  struct timeval tvGarbageTime;
 
  /* Default version of rip instance. */
  Int32T versionSend;	/* version 1 or 2 (but not both) */
  Int32T versionRecv;	/* version 1 or 2 or both */

  /* RIP routing information base. */
  RouteTableT *pRipRibTable;

  /* RIP only static routing information. */
  RouteTableT *pStaticRoute;
  
  /* RIP neighbor. */
  RouteTableT *pNeighbor;


  /* RIP default metric. */
  Int32T defaultMetric;

  /* RIP default-information originate. */
  Uint8T defaultInformation;
  char *defaultInformationRouteMap;

  /* RIP default distance. */
  Uint8T distance;

  /* RIP redistribute information. */
  Uint8T ripRedistributeDefault;
  Uint8T ripRedistribute[RIB_ROUTE_TYPE_MAX];

  /* For redistribute route map. */
  struct
  {
    char *name;
    RouteMapT *map;
    Int32T metricConfig;
    Uint32T metric;
  } routeMap[RIB_ROUTE_TYPE_MAX];

  /* List for interface list.  */
  ListT * pIfList;

  /* RIP enabled network vector. */
  VectorT ripEnableInterface;

  /* RIP enabled interface table. */
  RouteTableT *pRipEnableNetwork;

  /* List for peer list. */
  ListT * pPeerList;

  /* Vector to store passive-interface name. */
  Int32T passiveDefault; /* are we in passive-interface default mode? */
  VectorT vripPassiveNonDefault;

  /* RIP route changes. */
  Int32T ripGlobalRouteChanges; 

  /* RIP queries. */
  Int32T ripGlobalQueries;

  /* RIP offset list. */
  ListT *pRipOffsetList;

  /* RIP keychain list. */
  ListT *pKeychainList;

  /* RIP access list. */
  AccessMasterT accessMasterIpv4;
#ifdef HAVE_IPV6
  AccessMasterT accessMasterIpv6;
#endif

  /* RIP prefix list. */
  PrefixMasterT prefixMasterIpv4;
#ifdef HAVE_IPV6
  PrefixMasterT prefixMasterIpv6;
#endif

  /* RIP distribute list. */
  struct hash * pDistHash;

  /* RIP Routemap related pointers */
  VectorT routeMatchVec; /* Vector for route match rules */
  VectorT routeSetVec; /* Vector for route set rules. */

  /* RIP interface route map. */
  struct hash * pIfRouteMapHash;

  /* RIP distance control. */
  RouteTableT * pRipDistanceTable;

  /* For debug statement. */
  Uint32T ripDebugEvent;
  Uint32T ripDebugPacket;
  Uint32T ripDebugRibmgr;

  /* For snmp infomation. */
  RouteTableT *pRipIfaddrTable;
};
typedef struct Rip RipT;

/* RIP routing table entry which belong to rip_packet. */
struct Rte
{
  Uint16T family;		/* Address family of this route. */
  Uint16T tag;		/* Route Tag which included in RIP2 packet. */
  struct in_addr prefix;	/* Prefix of rip route. */
  struct in_addr mask;		/* Netmask of rip route. */
  struct in_addr nexthop;	/* Next hop of rip route. */
  Uint32T metric;		/* Metric value of rip route. */
} __attribute__((__packed__));
typedef struct Rte RteT;

/* RIP packet structure. */
struct RipPacket
{
  Uint8T command;	/* Command type of RIP packet. */
  Uint8T version;	/* RIP version which coming from peer. */
  Uint8T pad1;		/* Padding of RIP packet header. */
  Uint8T pad2;		/* Same as above. */
  RteT rte[1];		/* Address structure. */
} __attribute__((__packed__));
typedef struct RipPacket RipPacketT;


/* Buffer to read RIP packet. */
union RipBuf
{
  RipPacketT ripPacket;
  char buf[RIP_PACKET_MAXSIZ];
} __attribute__((__packed__));


/* RIP route information. */
struct RipInfo
{
  /* This route's type. */
  Int32T type;

  /* Sub type. */
  Int32T subType;

  /* RIP nexthop. */
  struct in_addr nexthop;
  struct in_addr from;

  /* Which interface does this route come from. */
  Uint32T ifIndex;

  /* Metric of this route. */
  Uint32T metric;

  /* External metric of this route. 
     if learnt from an externalm proto */
  Uint32T externalMetric;

  /* Tag information of this route. */
  Uint16T tag;

  /* Flags of RIP route. */
#define RIP_RTF_FIB      1
#define RIP_RTF_CHANGED  2
  Uint8T flags;

  /* Redistribute set or unset. */
#define RIP_REDISTRIBUTE_UNSET 0
#define RIP_REDISTRIBUTE_SET   1
  Uint8T redistribute;

  /* Garbage collect timer. */
  void *pTimerTimeout;
  void *pTimerGarbage;

  /* Updated time. */
  struct timeval tvUpdated;

  /* Route-map futures - this variables can be changed. */
  struct in_addr nexthopOut;
  Uint8T metricSet;
  Uint32T metricOut;
  Uint16T tagOut;
  Uint32T ifIndexOut;

  RouteNodeT *rp;

  Uint8T distance;

#ifdef NEW_RIP_TABLE
  struct RipInfo *next;
  struct RipInfo *prev;
#endif /* NEW_RIP_TABLE */
};
typedef struct RipInfo RipInfoT;


typedef enum eSplitHorizonPolicy{
  RIP_NO_SPLIT_HORIZON = 0,
  RIP_SPLIT_HORIZON,
  RIP_SPLIT_HORIZON_POISONED_REVERSE
} eSplitHorizonPolicyT;


/* RIP specific interface configuration. */
struct RipInterface
{
  /* RIP is enabled on this interface. */
  Int32T enableNetwork;
  Int32T enableInterface;

  /* RIP is running on this interface. */
  Int32T running;

  /* RIP version control. */
  Int32T riSend;
  Int32T riReceive;

  /* RIPv2 authentication type. */
  Int32T authType;

  /* RIPv2 authentication string. */
  char *authStr;

  /* RIPv2 authentication key chain. */
  char *keyChain;

  /* value to use for md5->auth_len */
  Uint8T md5AuthLen;

  /* Split horizon flag. */
  eSplitHorizonPolicyT splitHorizon;
  eSplitHorizonPolicyT splitHorizonDefault;

  /* For filter type slot. */
#define RIP_FILTER_IN  0
#define RIP_FILTER_OUT 1
#define RIP_FILTER_MAX 2

  /* Access-list. */
  AccessListT *pAccessList[RIP_FILTER_MAX];

  /* Prefix-list. */
  PrefixListT *pPrefixList[RIP_FILTER_MAX];

  /* Route-map. */
  RouteMapT *routeMap[RIP_FILTER_MAX];

  /* Interface statistics. */
  Int32T recvBadPackets;
  Int32T recvBadRoutes;
  Int32T sentUpdates;

  /* Passive interface. */
  Int32T passive;
};
typedef struct RipInterface RipInterfaceT;


/* RIP peer information. */
struct RipPeer
{
  /* Peer address. */
  struct in_addr addr;

  /* Peer RIP tag value. */
  Int32T domain;

  /* Last update time. */
  time_t uptime;

  /* Peer RIP version. */
  Uint8T version;

  /* Statistics. */
  Int32T recvBadPackets;
  Int32T recvBadRoutes;

  /* Timeout thread. */
  //struct thread *tTimeout;
  struct timeval tvTimeout;
  void * pTimeout;
};
typedef struct RipPeer RipPeerT;


struct RipMd5Info
{
  Uint16T family;
  Uint16T type;
  Uint16T packetLen;
  Uint8T keyId;
  Uint8T authLen;
  Uint32T sequence;
  Uint32T reserv1;
  Uint32T reserv2;
};
typedef struct RipMd5Info RipMd5InfoT;


struct RipMd5Data
{
  Uint16T family;
  Uint16T type;
  Uint8T digest[16];
};
typedef struct RipMd5Data RipMd5DataT;


/* RIP accepet/announce methods. */
#define RI_RIP_UNSPEC                      0
#define RI_RIP_VERSION_1                   1
#define RI_RIP_VERSION_2                   2
#define RI_RIP_VERSION_1_AND_2             3
/* N.B. stuff will break if
	(RIPv1 != RI_RIP_VERSION_1) || (RIPv2 != RI_RIP_VERSION_2) */

/* Default value for "default-metric" command. */
#define RIP_DEFAULT_METRIC_DEFAULT         1


/* Macro for timer turn on. */
#define RIP_TIMER_ON(T,F,V) \
  do { \
    if (!(T)) \
      (T) = thread_add_timer (master, (F), rinfo, (V)); \
  } while (0)

/* Macro for timer turn off. */
#define RIP_TIMER_OFF(X) \
  do { \
    if (X) \
      { \
        thread_cancel (X); \
        (X) = NULL; \
      } \
  } while (0)

/* Prototypes. */
extern Int32T ripCreate (void);
extern void ripTimerChange (void);
extern void ripInit (void);
extern void ripVersionUpdate(void);
extern void ripClean (void);
extern void ripTimerUpdate (Int32T fd, Int16T event, void * arg);
extern Int32T ripDistanceSet (Uint8T, PrefixT *, const StringT);
extern Int32T ripDistanceUnset (Uint8T, PrefixT *, const StringT);

extern Int32T ripRequestSend (struct sockaddr_in *, InterfaceT *, Uint8T,
                              ConnectedT *);
extern void ripInfoFree (RipInfoT *);
extern Uint8T ripDistanceApply (RipInfoT *);
extern void ripDistanceDisplay (struct cmsh *);
extern const StringT ripRouteTypePrint (Int32T sub_type);

extern void ripRedistributeAdd (Int32T, Int32T, Prefix4T *, Uint32T, 
                                struct in_addr *, Uint32T, Uint8T);
extern void ripRedistributeDelete (Int32T, Int32T, Prefix4T *, Uint32T);
extern void ripRedistributeWithdraw (Int32T);
extern Int32T ripCmshUpdateTimerRemain();
extern Int32T ripCheckTimeoutHalfTime (RipInfoT * pRInfo);
extern Int32T ripCmshUptime (char *, Int32T, RipInfoT *);
extern Int32T configWriteRip (struct cmsh *);

/* There is only one rip strucutre. */
extern RipT * pRip;

#endif /* _ripd_h */
