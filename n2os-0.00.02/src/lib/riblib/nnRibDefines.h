/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the ribmgr related definitions.
 *  - Block Name : riblib
 *  - Process Name : rib library
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : nnRibDefines.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_nnRibDefines_h)
#define _nnRibDefines_h

/* I/O, Character  Related Header */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Variable Type Related Header */
#include <sys/types.h>

/* IP Address Related Header */
#include <netinet/in.h>

/* Assertion Related Header */
#include <assert.h>

/* Netlink Related Header */
#ifdef HAVE_NETLINK
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/filter.h>
#include <stddef.h>
#else
#define RT_TABLE_MAIN           0
#endif /* HAVE_NETLINK */

/* File Control Related Header */
#include <fcntl.h>

/* Error Control Related Header */
#include <errno.h>

/* n2os defined header */
#include "nnDefines.h"

/*
 * Define of Routing Main Table
 */
//#define RT_TABLE_MAIN                         0

/*
 * Define of Return Value of the Commands
 */
#define RIBMGR_OK                             0
#define RIBMGR_WARNING                        1
#define RIBMGR_NOK                            2


/*
 * Address family numbers from RFC1700.
 */
#define AFI_IP                                1
#define AFI_IP6                               2
#define AFI_MAX                               3

/*
 * Subsequent Address Family Identifier.
 */
#define SAFI_UNICAST                          1
#define SAFI_MULTICAST                        2
#define SAFI_UNICAST_MULTICAST                3
#define SAFI_MPLS_VPN                         4
#define SAFI_MAX                              5

/*
 * AFI and SAFI type.
 */
typedef u_int16_t afi_t;
typedef u_int8_t safi_t;

/*
 * RIB Manager's Unix Domain Socket Path.
 */
#define RIBMGR_UNIX_SOCK_PATH "/tmp/ribmgr_socket"


/** @name Route's Nexthop Type
 * This definitions are about Route's Nexthop Type
 */
/**@{*/
#define NEXTHOP_TYPE_IFINDEX                  1  /* Directly connected.  */
#define NEXTHOP_TYPE_IFNAME                   2  /* Interface route.  */
#define NEXTHOP_TYPE_IPV4                     3  /* IPv4 nexthop.  */
#define NEXTHOP_TYPE_IPV4_IFINDEX             4  /* IPv4 nexthop with ifindex.  */
#define NEXTHOP_TYPE_IPV4_IFNAME              5  /* IPv4 nexthop with ifname.  */
#define NEXTHOP_TYPE_IPV6                     6  /* IPv6 nexthop.  */
#define NEXTHOP_TYPE_IPV6_IFINDEX             7  /* IPv6 nexthop with ifindex.  */
#define NEXTHOP_TYPE_IPV6_IFNAME              8  /* IPv6 nexthop with ifname.  */
#define NEXTHOP_TYPE_NULL0                    9  /* Null0 nexthop.  */
#define NEXTHOP_TYPE_MAX                     10
/**@}*/


/** @name Route Type
 * This definitions are about Route Type
 */
/**@{*/
#define RIB_ROUTE_TYPE_SYSTEM                 0
#define RIB_ROUTE_TYPE_KERNEL                 1
#define RIB_ROUTE_TYPE_CONNECT                2
#define RIB_ROUTE_TYPE_STATIC                 3
#define RIB_ROUTE_TYPE_RIP                    4
#define RIB_ROUTE_TYPE_RIPNG                  5
#define RIB_ROUTE_TYPE_OSPF	                  6
#define RIB_ROUTE_TYPE_OSPF6                  7
#define RIB_ROUTE_TYPE_ISIS                   8
#define RIB_ROUTE_TYPE_BGP                    9
#define RIB_ROUTE_TYPE_HSLS                  10
#define RIB_ROUTE_TYPE_MAX                   11
/**@}*/


/** @name ribmgr's IPC Message ID
 * This definitions are aboute IPC Message ID
 */
/**@{*/
enum
{
    IPC_NUM_RIB_START = IPC_NUM_RIB_MANAGER_START, /**< Rib Manager use 1400 ~ 1599 */

    IPC_RIB_CLIENT_INIT,
    IPC_RIB_CLIENT_CLOSE,
    
    IPC_RIB_ROUTE_PRESERVE,
    IPC_RIB_ROUTE_STALE_REMOVE,
    IPC_RIB_ROUTE_CLEAR,

    IPC_RIB_ROUTER_ID_SET,

    IPC_RIB_IPV4_ROUTE_ADD,
    IPC_RIB_IPV4_ROUTE_DELETE,

    IPC_RIB_IPV6_ROUTE_ADD,
    IPC_RIB_IPV6_ROUTE_DELETE,

    IPC_RIB_IPV4_NEXTHOP_BEST_LOOKUP,
    IPC_RIB_IPV4_NEXTHOP_EXACT_LOOKUP,

    IPC_RIB_IPV6_NEXTHOP_BEST_LOOKUP,
    IPC_RIB_IPV6_NEXTHOP_EXACT_LOOKUP,

    IPC_RIB_REDISTRIBUTE_ADD,
    IPC_RIB_REDISTRIBUTE_DELETE,
    IPC_RIB_REDISTRIBUTE_DEFAULT_ADD,
    IPC_RIB_REDISTRIBUTE_DEFAULT_DELETE,
    IPC_RIB_REDISTRIBUTE_CLEAR,

    IPC_RIB_IPV4_NEXTHOP_BEST_LOOKUP_REG,
    IPC_RIB_IPV4_NEXTHOP_BEST_LOOKUP_DEREG,

    IPC_RIB_IPV4_NEXTHOP_EXACT_LOOKUP_REG,
    IPC_RIB_IPV4_NEXTHOP_EXACT_LOOKUP_DEREG,

    IPC_RIB_IPV4_ROUTE_NOTIFICATION_ADD,
    IPC_RIB_IPV4_ROUTE_NOTIFICATION_DELETE,

    IPC_RIB_IPV6_NEXTHOP_BEST_LOOKUP_REG,
    IPC_RIB_IPV6_NEXTHOP_BEST_LOOKUP_DEREG,

    IPC_RIB_IPV6_NEXTHOP_EXACT_LOOKUP_REG,
    IPC_RIB_IPV6_NEXTHOP_EXACT_LOOKUP_DEREG,

    IPC_RIB_IPV6_ROUTE_NOTIFICATION_ADD,
    IPC_RIB_IPV6_ROUTE_NOTIFICATION_DELETE,

    IPC_RIB_IPV4_ROUTE_SHOW_REQ,

    IPC_INTERFACE_ADD,
    IPC_INTERFACE_DELETE,
    IPC_INTERFACE_ADDRESS_ADD,
    IPC_INTERFACE_ADDRESS_DELETE,
    IPC_INTERFACE_UP,
    IPC_INTERFACE_DOWN,
    IPC_RIB_END = 1599
};
/**@}*/


/** @name rimbgr's show route IPC Message ID 
 * This definitions are about show route ipc message id.
 */
/**@{*/
#define ROUTE_SHOW_TYPE_ALL                   1
#define ROUTE_SHOW_TYPE_SUMMARY               2
#define ROUTE_SHOW_TYPE_PROTOCOL              3
#define ROUTE_SHOW_TYPE_BEST                  4
#define ROUTE_SHOW_TYPE_EXACT                 5
/**@}*/


/** @name Message flags
 * This definitions are about using in message flags of ipc message.
 */
/**@{*/
#define RIB_MESSAGE_NEXTHOP                0x01
#define RIB_MESSAGE_IFINDEX                0x02
#define RIB_MESSAGE_DISTANCE               0x04
#define RIB_MESSAGE_METRIC                 0x08
/**@}*/


/*
 * RIB Manager's Error Codes
 */
#define RIBMGR_ERR_NOERROR                    0
#define RIBMGR_ERR_RTEXIST                   -1
#define RIBMGR_ERR_RTUNREACH                 -2
#define RIBMGR_ERR_EPERM                     -3
#define RIBMGR_ERR_RTNOEXIST                 -4
#define RIBMGR_ERR_KERNEL                    -5


/** @name RIB flags
 * This flags are about using in RibT's flags field.
 */
/**@{*/
#define RIB_FLAG_INTERNAL                  0x01
#define RIB_FLAG_SELFROUTE                 0x02
#define RIB_FLAG_NULL0                     0x04
#define RIB_FLAG_IBGP                      0x08
#define RIB_FLAG_SELECTED                  0x10
#define RIB_FLAG_CHANGED                   0x20
#define RIB_FLAG_STATIC                    0x40
#define RIB_FLAG_REJECT                    0x80
/**@}*/



/** @name Default Administrative Distance
 * This definitions are about default administrative distance of each protocols.
 */
/**@{*/
#define RIB_DISTANCE_DEFAULT_KERNEL           0
#define RIB_DISTANCE_DEFAULT_CONNECT          0
#define RIB_DISTANCE_DEFAULT_STATIC           1
#define RIB_DISTANCE_DEFAULT_EIGRP            5
#define RIB_DISTANCE_DEFAULT_EBGP            20
#define RIB_DISTANCE_DEFAULT_IEIGRP          90
#define RIB_DISTANCE_DEFAULT_IGRP           100
#define RIB_DISTANCE_DEFAULT_OSPF           110
#define RIB_DISTANCE_DEFAULT_OSPF6          110
#define RIB_DISTANCE_DEFAULT_ISIS           115
#define RIB_DISTANCE_DEFAULT_RIP            120
#define RIB_DISTANCE_DEFAULT_RIPNG          120
#define RIB_DISTANCE_DEFAULT_EGP            140
#define RIB_DISTANCE_DEFAULT_EEIGRP         170
#define RIB_DISTANCE_DEFAULT_IBGP           200
#define RIB_DISTANCE_DEFAULT_UNKNOWN        255
/**@}*/


/*
 * Definitaion Size of IPC message
 */
#define IPC_MAX_MESSAGE_SIZE               4096 // bytes
#define IPC_HEADER_SIZE                       6 // bytes

typedef struct ipcHeader
{
  unsigned short length;
  unsigned char  reserved;
  unsigned char  version;
#define NSI_VERSION   1
  unsigned short command;
} ipcHeader_t;


typedef struct ipcRouteIpv4
{
  u_char         type;
  u_char         flags;
  u_char         message;
  u_char         nexthop_num;
  struct in_addr **nexthop;
  u_char         ifindex_num;
  unsigned int  *ifindex;
  u_char distance;
  u_int32_t metric;
  
} ipcRouteIpv4_t;


/*
 * Definitaion Message Queue IPC Message
 */
#define MSG_QUEUE_ID_COMMAND_MANAGER       1000
#define MSG_QUEUE_ID_PIF_MANAGER           1001
#define MSG_QUEUE_ID_RIB_MANAGER           1002
#define MSG_QUEUE_ID_MRIB_MANAGER          1003
#define MSG_QUEUE_ID_LIB_MANAGER           1004
// Add Additional Message Queeu ID


typedef struct msgQBuff
{
#define SHOW_CONN_PID_CM                 1
    long pidType;
#define SHOW_MSG_TYPE_CONTINUE           1
#define SHOW_MSG_TYPE_EXIT               2
    long msgType;
    char msgBuff[IPC_MAX_MESSAGE_SIZE];
}msgQBuff_t;

#endif /*_nnRibDefines_h */
