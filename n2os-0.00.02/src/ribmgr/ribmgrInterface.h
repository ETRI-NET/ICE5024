#if !defined(_ribmgrInterface_h)
#define _ribmgrInterface_h

/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the interface related definitions.
 *  - Block Name : RIB Manager
 *  - Process Name : ribmgr
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : ribmgrInterface.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include "nnTypes.h"
#include "nnTable.h"
#include "ribmgrRedistribute.h"

/* For interface multicast configuration. */
#define IF_RIBMGR_MULTICAST_UNSPEC        0
#define IF_RIBMGR_MULTICAST_ON            1
#define IF_RIBMGR_MULTICAST_OFF           2

/* For interface shutdown configuration. */
#define IF_RIBMGR_SHUTDOWN_UNSPEC         0
#define IF_RIBMGR_SHUTDOWN_ON             1
#define IF_RIBMGR_SHUTDOWN_OFF            2

/* Router advertisement feature. */
#if (defined(LINUX_IPV6) && (defined(__GLIBC__) && __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 1)) || defined(KAME)
  #ifdef HAVE_RTADV
    #define RTADV
  #endif
#endif

#if 0
#ifdef RTADV
/* Router advertisement parameter.  From RFC2461, RFC3775 and RFC4191. */
struct rtadvconf
{
  /* A flag indicating whether or not the router sends periodic Router
     Advertisements and responds to Router Solicitations.
     Default: FALSE */
  Int32T AdvSendAdvertisements;

  /* The maximum time allowed between sending unsolicited multicast
     Router Advertisements from the interface, in milliseconds.
     MUST be no less than 70 ms (RFC3775, section 7.4) and no greater 
     than 1800000 ms (See RFC2461).

     Default: 600000 milliseconds */
  Int32T MaxRtrAdvInterval;
#define RTADV_MAX_RTR_ADV_INTERVAL 600000

  /* The minimum time allowed between sending unsolicited multicast
     Router Advertisements from the interface, in milliseconds.
     MUST be no less than 30 ms (See RFC3775, section 7.4). 
     MUST be no greater than .75 * MaxRtrAdvInterval.

     Default: 0.33 * MaxRtrAdvInterval */
  Int32T MinRtrAdvInterval;
#define RTADV_MIN_RTR_ADV_INTERVAL (0.33 * RTADV_MAX_RTR_ADV_INTERVAL)

  /* Unsolicited Router Advertisements' interval timer. */
  Int32T AdvIntervalTimer;

  /* The TRUE/FALSE value to be placed in the "Managed address
     configuration" flag field in the Router Advertisement.  See
     [ADDRCONF].
 
     Default: FALSE */
  Int32T AdvManagedFlag;


  /* The TRUE/FALSE value to be placed in the "Other stateful
     configuration" flag field in the Router Advertisement.  See
     [ADDRCONF].

     Default: FALSE */
  Int32T AdvOtherConfigFlag;

  /* The value to be placed in MTU options sent by the router.  A
     value of zero indicates that no MTU options are sent.

     Default: 0 */
  Int32T AdvLinkMTU;


  /* The value to be placed in the Reachable Time field in the Router
     Advertisement messages sent by the router.  The value zero means
     unspecified (by this router).  MUST be no greater than 3,600,000
     milliseconds (1 hour).

     Default: 0 */
  Uint32T AdvReachableTime;
#define RTADV_MAX_REACHABLE_TIME 3600000


  /* The value to be placed in the Retrans Timer field in the Router
     Advertisement messages sent by the router.  The value zero means
     unspecified (by this router).

     Default: 0 */
  Int32T AdvRetransTimer;

  /* The default value to be placed in the Cur Hop Limit field in the
     Router Advertisement messages sent by the router.  The value
     should be set to that current diameter of the Internet.  The
     value zero means unspecified (by this router).

     Default: The value specified in the "Assigned Numbers" RFC
     [ASSIGNED] that was in effect at the time of implementation. */
  Int32T AdvCurHopLimit;

  /* The value to be placed in the Router Lifetime field of Router
     Advertisements sent from the interface, in seconds.  MUST be
     either zero or between MaxRtrAdvInterval and 9000 seconds.  A
     value of zero indicates that the router is not to be used as a
     default router.

     Default: 3 * MaxRtrAdvInterval */
  Int32T AdvDefaultLifetime;
#define RTADV_ADV_DEFAULT_LIFETIME (3 * RTADV_MAX_RTR_ADV_INTERVAL)


  /* A list of prefixes to be placed in Prefix Information options in
     Router Advertisement messages sent from the interface.

     Default: all prefixes that the router advertises via routing
     protocols as being on-link for the interface from which the
     advertisement is sent. The link-local prefix SHOULD NOT be
     included in the list of advertised prefixes. */
  struct list *AdvPrefixList;

  /* The TRUE/FALSE value to be placed in the "Home agent"
     flag field in the Router Advertisement.  See [RFC3775 7.1].

     Default: FALSE */
  Int32T AdvHomeAgentFlag;
#ifndef ND_RA_FLAG_HOME_AGENT
#define ND_RA_FLAG_HOME_AGENT 	0x20
#endif

  /* The value to be placed in Home Agent Information option if Home 
     Flag is set.
     Default: 0 */
  Int32T HomeAgentPreference;

  /* The value to be placed in Home Agent Information option if Home 
     Flag is set. Lifetime (seconds) MUST not be greater than 18.2 
     hours. 
     The value 0 has special meaning: use of AdvDefaultLifetime value.
     
     Default: 0 */
  Int32T HomeAgentLifetime;
#define RTADV_MAX_HALIFETIME 65520 /* 18.2 hours */

  /* The TRUE/FALSE value to insert or not an Advertisement Interval
     option. See [RFC 3775 7.3]

     Default: FALSE */
  Int32T AdvIntervalOption;

  /* The value to be placed in the Default Router Preference field of
     a router advertisement. See [RFC 4191 2.1 & 2.2]

     Default: 0 (medium) */
  Int32T DefaultPreference;
#define RTADV_PREF_MEDIUM 0x0 /* Per RFC4191. */
};
#endif /* RTADV */
#endif

/** ribmgr daemon local interface structure. */
struct RibmgrIf
{
  /** Shutdown configuration. */
  Uint8T shutdown;

  /** Multicast configuration. */
  Uint8T multicast;

  /* Router advertise configuration. */
  Uint8T rtAdvEnable;

  /** Installed addresses chains tree. */
  RouteTableT *ipv4Subnets;

#ifdef RTADV
//  struct rtadvconf rtadv;
#endif /* RTADV */

#ifdef HAVE_IRDP
  struct irdp_interface irdp;
#endif

#ifdef SUNOS_5
  /* the real IFF_UP state of the primary interface.
   * need this to differentiate between all interfaces being
   * down (but primary still plumbed) and primary having gone
   * ~IFF_UP, and all addresses gone.
   */
  Uint8T primaryState;
#endif /* SUNOS_5 */
};
typedef struct RibmgrIf RibmgrIfT;



/*
 * Description : 인터페이스 정보를 후킹 설정하는 함수.
 *
 * param [in] pIf : 인터페이스 포인터
 *
 * retval : 0 always
 */
extern Int32T 
ifRibmgrNewHook (InterfaceT *pIf);


/*
 * Description : 인터페이스 정보를 후킹 해제하는 함수.
 *
 * param [in] pIf : 인터페이스 포인터
 *
 * retval : 0 always
 */
extern Int32T 
ifRibmgrDeleteHook (InterfaceT *pIf);


/*
 * Description : 인터페이스에 서브 네트워크를 추가하는 함수.
 *
 * param [in] pIf : 인터페이스 포인터
 * param [in] pIfc : connected 포인터
 *
 * retval : > 0 : count of subnet
 */
extern Int32T 
ifSubnetAdd (InterfaceT *pIf, ConnectedT *pIfc);


/*
 * Description : 인터페이스에 서브 네트워크를 삭제하는 함수.
 * Untie an interface address from its derived subnet list of addresses.
 *
 * param [in] pIf : 인터페이스 포인터
 * param [in] pIfc : connected 포인터
 *
 * retval : -1 : Failure,
 *          > 0 : Success 
 */
extern Int32T 
ifSubnetDelete (InterfaceT *pIf, ConnectedT *pIfc);


/*
 * Description : 인터페이스가 추가를 처리하는 함수.
 *
 * param [in] pIf : 인터페이스 포인터
 */
extern void 
ifAddUpdate (InterfaceT *pIf);


/*
 * Description : 인터페이스가 삭제 이벤트를 처리하는 함수.
 *
 * param [in] pIf : 인터페이스 포인터
 */
extern void 
ifDeleteUpdate (InterfaceT *pIf);


/*
 * description : 인터페이스가 상태의 up 이벤트를 처리하는 함수.
 *
 * param [in] pIf : 인터페이스 포인터
 */
extern void 
ifStateUp (InterfaceT *pIf);


/*
 * description : 인터페이스가 상태의 DOWN 이벤트를 처리하는 함수.
 *
 * param [in] pIf : 인터페이스 포인터
 */
extern void 
ifStateDown (InterfaceT *pIf);


/*
 * description : 인터페이스의 플래그를 갱신하는 함수.
 *
 * param [in] pIf : 인터페이스 포인터
 */
extern void 
ifRefresh (InterfaceT *pIf);

#if 0
extern void 
ifFlagsUpdate (InterfaceT *, Uint64T);
#endif

#endif /* _ribmgrInterface_h */
