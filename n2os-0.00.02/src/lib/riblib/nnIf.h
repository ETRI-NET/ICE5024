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
 * @file : nnIf.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_nnIf_h)
#define _nnIf_h

#include "nnPrefix.h"
#include "nnList.h"


#define INTERFACE_NAMSIZ      20
#define INTERFACE_HWADDR_MAX  20

/** Interface Structure */
struct Interface 
{
  /** Interface name.  This should probably never be changed after the
     interface is created, because the configuration info for this interface
     is associated with this structure.  For that reason, the interface
     should also never be deleted (to avoid losing configuration info).
     To delete, just set ifindex to IFINDEX_INTERNAL to indicate that the
     interface does not exist in the kernel.
   */
  char name[INTERFACE_NAMSIZ + 1];

  /** Interface index (should be IFINDEX_INTERNAL for non-kernel or
     deleted interfaces). */
  Uint32T ifIndex;
#define IFINDEX_INTERNAL	0

  /** Internal interface status */
  Uint8T status;
#define RIBMGR_INTERFACE_ACTIVE         (1 << 0)
#define RIBMGR_INTERFACE_SUB            (1 << 1)
#define RIBMGR_INTERFACE_LINKDETECTION  (1 << 2)
  
  /** Interface flags. */
  Uint64T flags;

  /** Interface metric */
  Int32T metric;

  /** Interface MTU. */
  Uint32T mtu;    /* IPv4 MTU */
  Uint32T mtu6;   /* IPv6 MTU - probably, but not neccessarily same as mtu */

  /** Hardware address. */
#ifdef HAVE_STRUCT_SOCKADDR_DL
  struct sockaddr_dl sdl;
#else
  Uint16T hwType;
  Uint8T  hwAddr[INTERFACE_HWADDR_MAX];
  Int32T  hwAddrLen;
#endif /* HAVE_STRUCT_SOCKADDR_DL */

  /** Interface bandwidth, kbits */
  Uint32T bandwidth;
  
  /** Description of the interface. */
  StringT desc;			

  /* Distribute list. */
//  void *distribute_in;
//  void *distribute_out;

  /** Connected address list. */
  ListT *pConnected;

  /** Daemon specific interface data pointer. */
  void *pInfo;

#ifdef HAVE_NET_RT_IFLIST
  struct if_data stats;
#endif /* HAVE_NET_RT_IFLIST */
};
typedef struct Interface InterfaceT;

/** Connected address structure. */
struct Connected
{
  /** Attached interface. */
  InterfaceT *pIf;

  /** Flags for configuration. */
  Uint8T conf;
#define RIBMGR_IFC_REAL         (1 << 0)
#define RIBMGR_IFC_CONFIGURED   (1 << 1)
  /*
     The RIBMGR_IFC_REAL flag should be set if and only if this address
     exists in the kernel.
     The RIBMGR_IFC_CONFIGURED flag should be set if and only if this address
     was configured by the user from inside quagga.
   */

  /** Flags for connected address. */
  Uint8T flags;
#define RIBMGR_IFA_SECONDARY    (1 << 0)
#define RIBMGR_IFA_PEER         (1 << 1)
  /* N.B. the RIBMGR_IFA_PEER flag should be set if and only if
     a peer address has been configured.  If this flag is set,
     the destination field must contain the peer address.  
     Otherwise, if this flag is not set, the destination address
     will either contain a broadcast address or be NULL.
   */

  /** Address of connected network. */
  PrefixT *pAddress;

  /** Peer or Broadcast address, depending on whether RIBMGR_IFA_PEER is set.
     Note: destination may be NULL if RIBMGR_IFA_PEER is not set. */
  PrefixT *pDestination;

  /** Label for Linux 2.2.X and upper. */
  StringT label;
};
typedef struct Connected ConnectedT;


/** Does the destination field contain a peer address? */
#define CONNECTED_PEER(C) CHECK_FLAG((C)->flags, RIBMGR_IFA_PEER)

/** Prefix to insert into the RIB */
#define CONNECTED_PREFIX(C) \
	(CONNECTED_PEER(C) ? (C)->pDestination : (C)->pAddress)

/** Identifying address.  We guess that if there's a peer address, but the
   local address is in the same prefix, then the local address may be unique. */
#define CONNECTED_ID(C)	\
	((CONNECTED_PEER(C) && !nnPrefixMatch((C)->pDestination, (C)->pAddress)) ?\
	 (C)->pDestination : (C)->pAddress)

/* Interface hook sort. */
#define IF_NEW_HOOK   0
#define IF_DELETE_HOOK 1

/* There are some interface flags which are only supported by some
   operating system. */

#ifndef IFF_NOTRAILERS
#define IFF_NOTRAILERS 0x0
#endif /* IFF_NOTRAILERS */
#ifndef IFF_OACTIVE
#define IFF_OACTIVE 0x0
#endif /* IFF_OACTIVE */
#ifndef IFF_SIMPLEX
#define IFF_SIMPLEX 0x0
#endif /* IFF_SIMPLEX */
#ifndef IFF_LINK0
#define IFF_LINK0 0x0
#endif /* IFF_LINK0 */
#ifndef IFF_LINK1
#define IFF_LINK1 0x0
#endif /* IFF_LINK1 */
#ifndef IFF_LINK2
#define IFF_LINK2 0x0
#endif /* IFF_LINK2 */
#ifndef IFF_NOXMIT
#define IFF_NOXMIT 0x0
#endif /* IFF_NOXMIT */
#ifndef IFF_NORTEXCH
#define IFF_NORTEXCH 0x0
#endif /* IFF_NORTEXCH */
#ifndef IFF_IPV4
#define IFF_IPV4 0x0
#endif /* IFF_IPV4 */
#ifndef IFF_IPV6
#define IFF_IPV6 0x0
#endif /* IFF_IPV6 */
#ifndef IFF_VIRTUAL
#define IFF_VIRTUAL 0x0
#endif /* IFF_VIRTUAL */

/* 인터페이스 리스트에서 인터페이스 이름으로 정렬하기 위한 비교 함수. */
extern Int32T 
ifCmpFunc (InterfaceT *, InterfaceT *);

/**
 * Description : 인터페이스를 생성하는 함수.
 *
 * @param [in] ifName : 인터페이스 문자열 포인터
 * @param [in] nameLen : 인터페이스 문자열 길이
 *
 * @retval : 인터페이스 포인터 
 */
extern InterfaceT *
ifCreate (const StringT ifName, Int32T nameLen);


/**
 * Description : 인터페이스를 삭제하는 함수.
 * Delete the interface, but do not free the structure, and leave it in the
 * interface list.  It is often advisable to leave the pseudo interface 
 * structure because there may be configuration information attached. 
 *
 * @param [in] ifp : 인터페이스 포인터
 */
extern void 
ifDeleteRetain (InterfaceT * pIf);


/**
 * Description : 인터페이스를 삭제하고, 메모리 해제  함수.
 * Delete and free the interface structure: calls if_delete_retain and then
 * deletes it from the interface list and frees the structure.
 *
 * @param [in] ifp : 인터페이스 포인터
 */
extern void 
ifDelete (InterfaceT * pIf);


/**
 * Description : 리스트에서 인덱스를 기반으로 인터페이스를 검색하는 함수. 
 *
 * @param [in] index : 인터페이스 인덱스
 *
 * @retval : 인터페이스 자료구조 포인터
 */
extern InterfaceT *
ifLookupByIndex (Uint32T index);

/**
 * Description : 리스트에서 인덱스를 기반으로 인터페이스 이름을 검색하는 함수. 
 * Please use ifindex2ifname instead of if_indextoname where possible;
 * ifindex2ifname uses internal interface info, whereas if_indextoname must
 * make a system call.
 *
 * @param [in] index : 인터페이스 인덱스
 *
 * @retval : 인터페이스 문자열 포인터
 */
extern const StringT 
ifIndex2ifName (Uint32T index);

/**
 * Description : 리스트에서 인터페이스 이름을 기반으로 인터페이스 인덱스를 검색하는 함수. 
 * Please use ifname2ifindex instead of if_nametoindex where possible;
 * ifname2ifindex uses internal interface info, whereas if_nametoindex must
 * make a system call.
 *
 * @param [in] ifName : 인터페이스 문자열 포인터
 *
 * @retval : 인터페이스 인덱스
 */
extern Uint32T 
ifName2ifIndex(const StringT ifName);


/**
 * Description : 리스트에서 인터페이스 이름을 기반으로 인터페이스를 검색하는 함수. 
 * These 2 functions are to be used when the ifname argument is terminated
 * by a '\0' character:
 *
 * @param [in] name : 인터페이스 문자열 포인터
 *
 * @retval : 인터페이스 자료구조 포인터
 */
extern InterfaceT *
ifLookupByName (const StringT ifName);


/**
 * Description : 리스트에서 인터페이스 이름 및 길이를 기반으로 인터페이스를 검색하는 함수. 
 * For these 2 functions, the namelen argument should be the precise length
 * of the ifname string (not counting any optional trailing '\0' character).
 * In most cases, strnlen should be used to calculate the namelen value.
 *
 * @param [in] name : 인터페이스 문자열 포인터
 * @param [in] namelen : 인터페이스 문자열 길이
 *
 * @retval : 인터페이스 자료구조 포인터
 */
extern InterfaceT *
ifLookupByNameLen(const StringT ifName, size_t nameLen);


/**
 * Description : 리스트에서 IPv4 주소를 기반으로 인터페이스를 검색하는 함수. 
 *
 * @param [in] src : 인터페이스 IPv4 주소
 *
 * @retval : 인터페이스 자료구조 포인터
 */
extern InterfaceT *
ifLookupExactAddress (struct in_addr src);


/**
 * Description : 리스트에서 IPv4 주소를 기반으로 인터페이스를 검색하는 함수. 
 *
 * @param [in] src : 인터페이스 IPv4 주소
 *
 * @retval : 인터페이스 자료구조 포인터
 */
extern InterfaceT *
ifLookupAddress (struct in_addr src);


/**
 * Description : 인터페이스 이름을 기반으로 인터페이스를 검색하고, 없는 경우
 * 인터페이스를 생성하여 포인터를 반환하는 함수.
 *
 * @param [in] name : 인터페이스 문자열 포인터
 *
 * @retval : 인터페이스 자료구조 포인터
 */
extern InterfaceT *
ifGetByName (const StringT ifName);


/**
 * Description : 인터페이스 이름과 길이를 기반으로 인터페이스를 검색하는 함수.
 *
 * @param [in] name : 인터페이스 문자열 포인터
 * @param [in] namelen : 인터페이스 문자열 길이
 *
 * @retval : 인터페이스 자료구조 포인터
 */
extern InterfaceT *
ifGetByNameLen(const StringT ifName, size_t nameLen);


/**
 * Description : 인터페이스 플래그가 IFF_UP 임을 확인하는 함수.
 *
 * @param [in] pIf : 인터페이스 자료구조 포인터
 *
 * @retval : > 0 if 인터페이스 플래그가 IFF_UP
 *             0 if 인터페이스 플래그가 IFF_UP이 아닌 경우
 */
extern Int32T 
ifIsUp (InterfaceT * pIf);


/**
 * Description : 인터페이스 플래그가 IFF_RUNNING 임을 확인하는 함수.
 *
 * @param [in] pIf : 인터페이스 자료구조 포인터
 *
 * @retval : > 0 if 인터페이스 플래그가 IFF_RUNNING
 *             0 if 인터페이스 플래그가 IFF_RUNNING이 아닌 경우
 */
extern Int32T 
ifIsRunning (InterfaceT * pIf);


/**
 * Description : 인터페이스 플래그가 Operative 상태 임을 확인하는 함수.
 *
 * @param [in] pIf : 인터페이스 자료구조 포인터
 *
 * @retval : > 0 if 인터페이스 플래그가 Operative
 *             0 if 인터페이스 플래그가 Operative가 아닌 경우
 */
extern Int32T 
ifIsOperative (InterfaceT * pIf);


/**
 * Description : 인터페이스가 Loopback 임을 확인하는 함수.
 *
 * @param [in] pIf : 인터페이스 자료구조 포인터
 *
 * @retval : > 0 if 인터페이스 플래그가 Loopback
 *             0 if 인터페이스 플래그가 Loopback이 아닌 경우
 */
extern Int32T 
ifIsLoopback (InterfaceT * pIf);


/**
 * Description : 인터페이스가 Broadcast 타입이 설정되어 있는가를 확인하는 함수.
 *
 * @param [in] pIf : 인터페이스 자료구조 포인터
 *
 * @retval : > 0 if 인터페이스 플래그가 IFF_BROADCAST
 *             0 if 인터페이스 플래그가 IFF_BROADCAST가 아닌 경우
 */
extern Int32T 
ifIsBroadcast (InterfaceT * pIf);


/**
 * Description : 인터페이스가 P2P 타입이 설정되어 있는가를 확인하는 함수.
 *
 * @param [in] pIf : 인터페이스 자료구조 포인터
 *
 * @retval : > 0 if 인터페이스 플래그가 IFF_POINTOPOINT
 *             0 if 인터페이스 플래그가 IFF_POINTOPOINT가 아닌 경우
 */
extern Int32T 
ifIsPointPoint (InterfaceT * pIf);


/**
 * Description : 인터페이스가 Multicast 타입이 설정되어 있는가를 확인하는 함수.
 *
 * @param [in] pIf : 인터페이스 자료구조 포인터
 *
 * @retval : > 0 if 인터페이스 플래그가 IFF_MULTICAST
 *             0 if 인터페이스 플래그가 IFF_MULTICAST가 아닌 경우
 */
extern Int32T 
ifIsMulticast (InterfaceT * pIf);


/**
 * Description : 인터페이스 플래그를 덤프하는 함수.
 *
 * @param [in] flag : 인터페이스 플래그
 *
 * @retval : 문자열 포인터
 */
extern const StringT
ifFlagDump(Uint32T flag);


/*
 * Description : 인터페이스 정보를 덤프하는 함수.
 */
extern void 
ifDumpAll (void);


extern void 
ifAddHook (Int32T, Int32T (*)(InterfaceT *));


/**
 * Description : 커넥티드 자료구조를 생성하는 함수.
 *
 * @retval : 커넥티드 자료구조 포인터
 */
extern ConnectedT *
connectedNew (void);


/**
 * Description : 커넥티드 자료구조를 메모리를 해제하는 함수.
 *
 * @param [in] pConnected : 커넥티드 자료구조 포인터
 */
extern void 
connectedFree (ConnectedT * pConnected);


extern void 
connectedAdd (InterfaceT *, ConnectedT *);


/**
 * Description : PrefixT 자료구조 내의 IP 주소가 같은지를 확인하는 함수.
 *
 * @param [in] pIf : 인터페이스 자료구조 포인터
 * @param [in] p : PrefixT 자료구조 포인터
 *
 * @retval : connected 자료구조 포인터
 */
extern ConnectedT  *
connectedDeleteByPrefix (InterfaceT * pIf, PrefixT * pPrefix);


/**
 * Description : 인터페이스에 설정된 주소에서 connected 정보를 얻는 함수.
 *
 * @param [in] pIf : 인터페이스 자료구조 포인터
 * @param [in] dst : IPv4 자료구조
 *
 * @retval : connected 자료구조 포인터
 */
extern ConnectedT  *
connectedLookupAddress (InterfaceT * pIf, struct in_addr dst);


/**
 * Description : 인터페이스에 설정된 주소에서 Prefix 정보를 기반으로 connected 
 * 정보를 얻는 함수.
 *
 * @param [in] pIf : 인터페이스 자료구조 포인터
 * @param [in] pPrefix : PrefixT 자료구조 포인터
 * @param [in] pDestination : PrefixT 자료구조 포인터
 *
 * @retval : connected 자료구조 포인터
 */
extern ConnectedT  *
connectedAddByPrefix (InterfaceT * pIf, PrefixT * pPrefix, PrefixT * pDst);

#ifndef HAVE_IF_NAMETOINDEX
/**
 * Description : 인터페이스 이름으로 인덱스를 찾는 함수.
 *
 * @param [in] name : 인터페이스 문자열 포인터
 *
 * @retval : Uint32T : 인터페이스 인덱스
 */
extern Uint32T
ifNametoIndex (const StringT ifName);
#endif
#ifndef HAVE_IF_INDEXTONAME
/**
 * Description : 인덱스로 인터페이스 이름을 찾는 함수.
 *
 * @param [in] ifindex : 인터페이스 인덱스
 * @param [in] name : 인터페이스 이름 문자열 포인터
 *
 * @retval : 문자열 포인터
 */
extern StringT 
ifIndextoName (Uint32T ifIndex, StringT ifName);
#endif

/**
 * Description : 인터페이스 정보를 저장하는 리스트를 초기화 하는 함수.
 */
extern ListT * 
ifInit (void);

/**
 * Description : 인터페이스 리스트 포인터를 갱신하는 함수.
 */
extern void 
ifUpdate (ListT * gpIfList);

/* Description : 인터페이스 리스트 포인터의 메모리를 해제 함수. */
extern void 
ifClose ();

extern ListT * pIfList;

#endif /* _nnIf_h */
