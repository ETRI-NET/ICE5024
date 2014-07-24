#if !defined(_ribmgrRib_h)
#define _ribmgrRib_h

/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the rib related definitions.
 *  - Block Name : RIB Manager
 *  - Process Name : ribmgr
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : ribmgrRib.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include "nnTypes.h"
#include "nnRibDefines.h"
#include "nnPrefix.h"
#include "nnTable.h"

#include "ribmgrInit.h"

#define DISTANCE_INFINITY  255

#define RIB_SYSTEM_ROUTE(R) \
     ((R)->type == RIB_ROUTE_TYPE_KERNEL || (R)->type == RIB_ROUTE_TYPE_CONNECT)

/* Gateway address data structure. */
union GwAddr {
  struct in_addr ipv4;
#ifdef HAVE_IPV6
  struct in6_addr ipv6;
#endif /* HAVE_IPV6 */
};
typedef union GwAddr uGwAddrT;

/* Routing information base. */
struct Rib
{
  /* Status Flags for the *RouteNodeT*, but kept in the head RIB.. */
  Uint8T rnStatus;
#define RIB_ROUTE_QUEUED(x)	(1 << (x))

  /* Link list. */
  struct Rib *next;
  struct Rib *prev;
  
  /* Nexthop structure */
  struct Nexthop *pNexthop;
  
  /* Refrence count. */
  Uint32T refCount;
  
  /* Uptime. */
  time_t uptime;

  /* Type fo this route. */
  Int32T type;

  /* Which routing table */
  Int32T table;			

  /* Metric */
  Uint32T metric;

  /* Distance. */
  Uint8T distance;

  /* Flags of this route.
   * This flag's definition is in lib/nnRibDefines.h RIB_FLAG_* and is exposed
   * to clients via Zserv
   */
  Uint8T flags;

  /* RIB internal status */
  Uint8T status;
#define RIB_ENTRY_REMOVED	(1 << 0)

  /* Nexthop information. */
  Uint8T nexthopNum;
  Uint8T nexthopActiveNum;
  Uint8T nexthopFibNum;

};
typedef struct Rib RibT;


/* Static route information. */
struct StaticIpv4
{
  /* For linked list. */
  struct StaticIpv4 *prev;
  struct StaticIpv4 *next;

  /* Administrative distance. */
  Uint8T distance;

  /* Flag for this static route's type. */
  Uint8T type;
#define STATIC_IPV4_GATEWAY     1
#define STATIC_IPV4_IFNAME      2
#define STATIC_IPV4_BLACKHOLE   3

  /* Nexthop value. */
  union 
  {
    struct in_addr ipv4;
    StringT ifName;
  } gate;

  /* bit flags */
  Uint8T flags;
/*
 see RIB_FLAG_REJECT
     RIB_FLAG_NULL0
 */
};
typedef struct StaticIpv4 StaticIpv4T;

#ifdef HAVE_IPV6
/* Static route information. */
struct StaticIpv6
{
  /* For linked list. */
  struct StaticIpv6 *prev;
  struct StaticIpv6 *next;

  /* Administrative distance. */
  Uint8T distance;

  /* Flag for this static route's type. */
  Uint8T type;
#define STATIC_IPV6_GATEWAY          1
#define STATIC_IPV6_GATEWAY_IFNAME   2
#define STATIC_IPV6_IFNAME           3

  /* Nexthop value. */
  struct in6_addr ipv6;
  StringT ifName;

  /* bit flags */
  Uint8T flags;
/*
 see RIB_FLAG_REJECT
     RIB_FLAG_NULL0
 */
};
typedef struct StaticIpv6 StaticIpv6T;
#endif /* HAVE_IPV6 */


/* Nexthop structure. */
struct Nexthop
{
  struct Nexthop *next;
  struct Nexthop *prev;

  /* Interface index. */
  StringT ifName;
  Uint32T ifIndex;
 
  /* Route's Nexthop Type */ /*#see nnRibDefines.h */ 
  Int32T type;

  Uint8T flags;
#define NEXTHOP_FLAG_ACTIVE     (1 << 0) /* This nexthop is alive. */
#define NEXTHOP_FLAG_FIB        (1 << 1) /* FIB nexthop. */
#define NEXTHOP_FLAG_RECURSIVE  (1 << 2) /* Recursive nexthop. */

  /* Nexthop address or interface name. */
  uGwAddrT gate;

  /* Recursive lookup nexthop. */
  Uint8T   rType;
  Uint32T  rifIndex;
  uGwAddrT rGate;
  uGwAddrT src;
};
typedef struct Nexthop NexthopT;

/* Routing table instance.  */
struct Vrf
{
  /* Identifier.  This is same as routing table vector index.  */
  Uint32T id;

  /* Routing table name.  */
  StringT name;

  /* Description.  */
  StringT desc;

  /* FIB identifier.  */
  Uint8T fibId;

  /* Routing table.  */
  RouteTableT *pDynamicTable[AFI_MAX][SAFI_MAX];

  /* Static route configuration.  */
  RouteTableT *pStaticTable[AFI_MAX][SAFI_MAX];
};
typedef struct Vrf VrfT;


/*
 * Description : RIB의 nexthop 에 Interface Index를 할당하는 함수.
 *
 * param [in] pRib : RibT * 자료구조 포인터
 * param [in] ifIndex : 인터페이스 ID
 *
 * retval : NexthopT * 
 */
extern NexthopT *
nexthopIfindexAdd (RibT *pRib, Uint32T ifIndex);


/*
 * Description : RIB에서 nexthop에 인터페이스 이름을 할당하는  함수.
 *
 * param [in] pRib : RibT * 자료구조 포인터
 * param [in] ifName : StringT 문자열
 *
 * retval : NexthopT * 
 */
extern NexthopT *
nexthopIfnameAdd (RibT *pRib, StringT ifName);


/*
 * Description : RIB에서 nexthop에 Ipv4 주소를 할당하는 함수.
 *
 * param [in] pRib : RibT * 자료구조 포인터
 * param [in] pIpv4 : struct in_addr * 자료구조 포인터
 * param [in] pSrc : struct in_addr * 자료구조 포인터
 *
 * retval : NexthopT * 
 */
extern NexthopT *
nexthopIpv4Add (RibT *pRib, struct in_addr *pIpv4, struct in_addr *pSrc);


/*
 * Description : RIB에서 nexthop에 Black Hole을 할당하는 함수.
 *
 * param [in] pRib : RibT * 자료구조 포인터
 *
 * retval : NexthopT *  자료구조 포인터
 */
extern NexthopT *
nexthopBlackholeAdd (RibT *pRib);


/*
 * Description : Prefix4T의 주소에 맞는 RIB를 찾아 덤프하는함수.
 * This is an exported helper to rtm_read() to dump the strange
 * RIB entry found by ribLookupIpv4Route()
 *
 * param [in] pPrefix : Prefix4T *
 */
extern void 
ribLookupAndDump (Prefix4T *pPrefix4);

#if 0
extern void 
ribLookupAndPushup (Prefix4T *pPrefix4);
#endif


/*
 * Description : RIB 정보를 덤프하는 함수.
 * This function dumps the contents of a given RIB entry into
 * standard debug log. Calling function name and IP prefix in
 * question are passed as 1st and 2nd arguments.
 *
 * param [in] func : char * 함수 포인터
 * param [in] pPrefix4 : Prefix4T * 자료구조 포인터
 * param [in] pRib : RibT * 자료구조 포인터
 */
extern void 
ribDump (const char *func,  const Prefix4T *pPrefix4, const RibT *pRib);

//extern Int32T ribLookupIpv4Route (Prefix4T *, union sockunion *);
#define RIBMGR_RIB_LOOKUP_ERROR     -1
#define RIBMGR_RIB_FOUND_EXACT       0
#define RIBMGR_RIB_FOUND_NOGATE      1
#define RIBMGR_RIB_FOUND_CONNECTED   2
#define RIBMGR_RIB_NOTFOUND          3

#ifdef HAVE_IPV6
/*
 * Description : RIB에서 nexthop에 Ipv6 주소를 할당하는 함수.
 *
 * param [in] pRib : RibT *
 * param [in] pIpv6 : struct in6_addr *
 *
 * retval : NexthopT * 
 */
extern NexthopT *
nexthopIpv6Add (RibT *pRib, struct in6_addr *pIpv6);
#endif /* HAVE_IPV6 */


/*
 * Description : VRF 테이블에서 ID를 기반으로 VRF 엔트리를 찾는 함수.
 *
 * param [in] id : VRF ID
 *
 * retval : VrfT * : VRF 테이블 엔트리 포인터
 */
extern VrfT *
vrfLookup (Uint32T id);


/*
 * Description : VRF 테이블에서 AFI, SAFI, ID를 기반으로 Route Table 포인터
 * 를 찾는 함수.
 *
 * param [in] afi : afi_t 자료구조
 * param [in] safi : safi_t 자료구조
 * param [in] id : VRF ID
 *
 * retval : RouteTableT * : Route Tabel 포인터
 */
extern RouteTableT *
vrfTable (afi_t afi, safi_t safi, Uint32T id);


/*
 * Description : VRF 테이블에서 AFI, SAFI, ID를 기반으로 Static Route Table
 * 포인터를 찾는 함수.
 *
 * param [in] afi : afi_t 자료구조
 * param [in] safi : safi_t 자료구조
 * param [in] id : VRF ID
 *
 * retval : RouteTableT * : Route Tabel 포인터
 */
extern RouteTableT *
vrfStaticTable (afi_t afi, safi_t safi, Uint32T id);

/* NOTE:
 * All ribAddIpv[46]* functions will not just add prefix into RIB, but
 * also implicitly withdraw equal prefix of same type. */
/*
 * Description : Ipv4 Route를 Route Table에 추가하는 함수.
 *
 * param [in] type : route type
 * param [in] flags : route flags
 * param [in] pPrefix4 : Prefix4T * 자료구조 포인터
 * param [in] pGate : route's gateway address 
 * param [in] pSrc :  route's source address
 * param [in] ifIndex : nexthop's interface index
 * param [in] vrfId : vrf id 
 * param [in] metric : route's metric
 * param [in] distance : route's distance
 *
 * retval : 0 if Failure,
 *           1 if Success
 */
extern Int32T 
ribAddIpv4 (Int32T type, Int32T flags, Prefix4T *pPrefix4, struct in_addr *pGate, struct in_addr *pSrc, Uint32T ifIndex, Uint32T vrfId, Uint32T metric, Uint8T distance);


/*
 * Description : 주어진 RIB와 Prefix4T값을 기반으로 Kernel로 IPv4 Route를
 * 설정하는 함수.
 *
 * param [in] pPrefix4 : Prefix4T * 자료구조 포인터
 * param [in] pRib : Prefix4T * 자료구조 포인터
 *
 * retval : 0 always
 */
extern Int32T 
ribAddIpv4Multipath (Prefix4T *pPrefix4, RibT *pRib);


/*
 * Description : IPv4 RIB를 삭제하는 함수.
 *
 * param [in] type : route type
 * param [in] flags : route flags
 * param [in] pPrefix4 : Prefix4T *
 * param [in] pGate : nexthop's gateway address
 * param [in] ifIndex : nexthop's interface index
 * param [in] vrfId : vrf id
 *
 * retval : 0 if Success
 *          RIBMGR_ERR_RTNOEXIST if Failure
 */
extern Int32T 
ribDeleteIpv4 (Int32T type, Int32T flags, Prefix4T *pPrefix4, struct in_addr *pGate, Uint32T ifIndex, Uint32T vrfId);

/*
 * Description : Route Table에서 요청된 주소에 맞는 rib를 찾는 함수.
 *
 * param [in] addr : Ipv4 Address 자료구조
 *
 * retval : RibT * Rib 자료구조 포인터 
 */
extern RibT *
ribMatchIpv4 (struct in_addr addr);


/*
 * Description : Route Table에서 Prefix4T에 맞는 rib를 찾는 함수.
 *
 * param [in] pPrefix4 : Prefix4T * 자료구조 포인터
 *
 * retval : RibT * 자료구조 포인터 
 */
extern RibT *
ribLookupIpv4 (Prefix4T * pPrefix4);


/*
 * Description : RIB 정보를 갱신하는 함수.
 */
extern void 
ribUpdate (void);


/*
 * Description : Main Table로 부터 설정된 Route 외의 모든 Route를 삭제하는 함수.
 */
extern void 
ribWeedTables (void);


/*
 * Description : RIB Manager가 기동된 이후에 자체적으로 만들어진 모든 Route를
 * 삭제하는 함수.
 */
extern void 
ribSweepRoute (void);


/*
 * Description : 모든 RIB 테이블을 닫는 함수.
 */
extern void 
ribClose (void);


/*
 * Description : RIB 테이블 초기화.
 */
extern void 
ribInit (void);


/* 
 * Description : create client node. 
 */
extern ClientT *
clientCreate (Int8T);

/* 
 * Description : free client node.
 */
extern void
clientDelete (Int8T);

/*
 * Description : client pointer lookup.
 */
extern ClientT * 
clientLookupById (Int8T);

/*
 * Description : client pointer lookup or create node.
 */
extern ClientT *
clientGetById (Int8T);


/*
 * Description : Client list initialize.
 */
extern void 
clientListInit (void);


/*
 * Description : Client list initialize.
 */
extern void 
clientListFree (void);

/*
 * Description : Static IPv4 Route를 설정하는 함수.
 *
 * param [in] pPrefix : PrefixT * 자료구조 포인터
 * param [in] pSi : StaticIpv4T * 자료구조 포인터
 */
extern void 
staticInstallIpv4 (PrefixT *pPrefix, StaticIpv4T *pSi);


/*
 * Description : Static IPv4 Route를 삭제하는 함수.
 *
 * param [in] pPrefix : PrefixT * 자료구조 포인터
 * param [in] pSi : StaticIpv4T * 자료구조 포인터
 */
extern void 
staticUninstallIpv4 (PrefixT *pPrefix, StaticIpv4T *pSi);


/*
 * Description : Static Route 설정하는 함수.
 *
 * param [in] p : PrefixT 자료구조 포인터
 * param [in] nhType : nexthop 타입
 * param [in] strNexthop : nexthop 문자열
 * param [in] distance : 루트 디스턴스
 *
 * retval : always 1
 */
extern Int32T
staticAddIpv4(PrefixT *pPrefix, Uint8T nhType,
              const char * strNexthop, Uint8T distance);


/*
 * Description : Static Route 삭제하는 함수.
 *
 * param [in] pPrefix : PrefixT 자료구조 포인터
 * param [in] nhType : nexthop 타입
 * param [in] strNexthop : nexthop 문자열
 * param [in] distance : 루트 디스턴스
 *
 * retval : always 0
 */
extern Int32T
staticDeleteIpv4(PrefixT *pPrefix, Uint8T nhType,
                 const char * strNexthop, Uint8T distance);


#ifdef HAVE_IPV6
/*
 * Description : IPv6 Route를 설정하는 함수.
 *
 * param [in] type : route type
 * param [in] flags : route flags
 * param [in] pPrefix6 : Prefix6T *자료구조 포인터
 * param [in] pGate : struct in6_addr *자료구조 포인터
 * param [in] ifIndex : nexthop's interface index
 * param [in] vrfId : vrf id
 * param [in] metric : 메트릭 값
 * param [in] distance : 디스턴스 값
 *
 * retval : 0 always
 */
extern Int32T
ribAddIpv6 (Int32T type, Int32T flags, Prefix6T *pPrefix6, struct in6_addr *pGate, Uint32T ifIndex, Uint32T vrfId, Uint32T metric, Uint8T distance);


/*
 * Description : IPv6 Route를 삭제하는 함수.
 *
 * param [in] type : route type
 * param [in] flags : route flags
 * param [in] pPrefix6 : Prefix6T *자료구조 포인터
 * param [in] pGate : struct in6_addr *자료구조 포인터
 * param [in] ifIndex : nexthop's interface index
 * param [in] vrfId : vrf id
 *
 * retval : 0 always
 */
extern Int32T
ribDeleteIpv6 (Int32T type, Int32T flags, Prefix6T *pPrefix6, struct in6_addr *pGate, Uint32T ifindex, Uint32T vrfId);

extern RibT *
ribLookupIpv6 (struct in6_addr *);


/*
 * Description : Route Table에서 입력된 IPv6 주소에 맞는 rib를 찾는 함수.
 *
 * param [in] addr : struct in6_addr * 자료구조 포인터
 *
 * retval : RibT * 자료구조 포인터
 */
extern RibT *
ribMatchIpv6 (struct in6_addr *pAddr);


/*
 * Description : Static IPv6 Route를 Configuration 정보로 저장하는 함수.
 *
 * param [in] pPrefix : PrefixT *자료구조 포인터
 * param [in] type : route type
 * param [in] pGate : nexthop's gateway address
 * param [in] ifName : nexthop's interface name
 * param [in] flags : route flags
 * param [in] distance : route's distance
 * param [in] vrfId : vrf id
 *
 * retval : -1 if Failure
 *           0 if Success
 */
extern Int32T
staticAddIpv6 (PrefixT *pPrefix, Uint8T type, struct in6_addr *pGate, const StringT ifName, Uint8T flags, Uint8T distance, Uint32T vrfId);


/*
 * Description : Static IPv6 Route를 Configuration 정보를 삭제하는 함수.
 *
 * param [in] pPrefix : PrefixT *자료구조 포인터
 * param [in] type : route type
 * param [in] pGate : nexthop's gateway address
 * param [in] ifName : nexthop's interface name
 * param [in] distance : route's distance
 * param [in] vrfId : vrf id
 *
 * retval : -1 if Failure
 *           0 if Success
 */
extern Int32T
staticDeleteIpv6 (PrefixT *pPrefix, Uint8T type, struct in6_addr *pGate, const StringT ifName, Uint8T distance, Uint32T vrfId);

#endif /* HAVE_IPV6 */

#endif /* _ribmgrRib_h */
