#if !defined(_pifMsgEventData_h)
#define _pifMsgEventData_h
/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/
  
/**   
 * @brief : This file defines the interface related definitions.
 *  - Block Name : PIF Manager
 *  - Process Name : pifmgr
 *  - Creator : Seungwoo Hong
 *  - Initial Date : 2013/10/10
 */
  
/**
 * @file : 
 *
 * $Author: $
 * $Date: $
 * $Revision: $
 * $LastChangedBy: $
 */

#include "nosLib.h"

/****************************************************************************
 * PIF Event ID in nnDefines.h
 * 
 * L2 Event  : EVENT_PIF_L2_LINK
 * VLAN Event: EVENT_PIF_L2_VLAN
 * L3 Event  : EVENT_PIF_L3_INTERFACE
 ***************************************************************************/


/* L2 link Event ID */ 
#define PIF_EVENT_LINK_BASE     0
#define PIF_EVENT_LINK_ADD  	PIF_EVENT_LINK_BASE + 0
#define PIF_EVENT_LINK_DEL 		PIF_EVENT_LINK_BASE + 1
#define PIF_EVENT_LINK_UP   	PIF_EVENT_LINK_BASE + 2
#define PIF_EVENT_LINK_DOWN  	PIF_EVENT_LINK_BASE + 3

/****************************************************************************
 * Message format (Port Event)
 * 
 * PORT_EVENT_ADD/PORT_EVENT_DEL
 * PORT_EVENT_UP /PORT_EVENT_DOWN
 * 
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                       Port Event ID                           | 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      Port Name String                         | 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                        Interface ID                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      Interface Index                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    ifType                       | adminState    | operState   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   bandwidth                     |                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                        macAddr[6bytes]                        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

typedef struct
{
	/** Event ID (4 bytes) */
    Int32T eId;

	/** Interface name (4 bytes) */
    Int8T  ifName[32];  

	/** Interface ID (4 bytes) */
    Int32T iId;

	/** Interface index (4 bytes) */
    Int32T ifIndex;

#define PIF_PORT_TYPE_FAST_ETHERNET 0
#define PIF_PORT_TYPE_GIGA_ETHERNET 1
#define PIF_PORT_TYPE_AGGREGATED    2
	/** Interface type */
    Int16T ifType;

	/** state (admin/oper) state */
#define PIF_PORT_STATE_DOWN 	0
#define PIF_PORT_STATE_UP 		1
    Int8T adminState;
    Int8T operState;

	/** Interface bandwidth */
    Int16T bandwidth;

	/** bridge id (always 1)*/
    Int8T  macAddress[6];

} PifPortEventMsgT;


/* L2 link Event ID */ 
#define PIF_EVENT_VLAN_BASE         10
#define PIF_EVENT_VLAN_ADD  		PIF_EVENT_VLAN_BASE + 0
#define PIF_EVENT_VLAN_DEL 			PIF_EVENT_VLAN_BASE + 1
#define PIF_EVENT_VLAN_PORT_MODE 	PIF_EVENT_VLAN_BASE + 2
#define PIF_EVENT_VLAN_PORT_ADD  	PIF_EVENT_VLAN_BASE + 3
#define PIF_EVENT_VLAN_PORT_DEL 	PIF_EVENT_VLAN_BASE + 4

/****************************************************************************
 * Message format (Vlan Event)
 * 
 * PIF_EVENT_VLAN_ADD / PIF_EVENT_VLAN_DEL / PIF_EVENT_VLAN_PORT_MODE
 * 
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                       Vlan Event ID                           | 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      Vlan Name String                         | 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                          Vlan ID                              |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                        Interface ID                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      Interface Index                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                          Port Mode                            |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

typedef struct
{
	/** Event ID (4 bytes) */
    Int32T eId;

	/** Interface name (4 bytes) */
    Int8T  vlanName[32];  

	/** Interface ID (4 bytes) */
    Int32T vId;

	/** Interface ID (4 bytes) */
    Int32T iId;

	/** Interface index (4 bytes) */
    Int32T ifIndex;

#define PIF_L2_PORT_MODE_NOT_CONFIGURED		0
#define PIF_L2_PORT_MODE_SWITCH_PORT		1
#define PIF_L2_PORT_MODE_ROUTED_PORT		2
	/** Port Mode */
    Int32T portMode;

} PifVlanEventMsgT;



/****************************************************************************
 * Message format (VlanPort Event)
 * 
 * PIF_EVENT_PORT_VLAN_ADD / PIF_EVENT_PORT_VLAN_DEL
 * 
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                       Vlan Event ID                           | 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                        Interface ID                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      Interface Index                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                          Vid Number                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |        vid                  :               vid               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |        vid                  :               vid               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                            ...                                |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

typedef struct
{
	/** Event ID (4 bytes) */
    Int32T eId;

	/** Interface ID (4 bytes) */
    Int32T iId;

	/** Interface index (4 bytes) */
    Int32T ifIndex;

	/** Vid number */
    Int32T num;

	/** variable size vid list */
    Int32T vid[4096];

} PifVlanPortEventMsgT;



/* L3 Interface Event */ 
#define PIF_EVENT_INTERFACE_BASE               20
#define PIF_EVENT_INTERFACE_ADD                PIF_EVENT_INTERFACE_BASE + 0
#define PIF_EVENT_INTERFACE_DEL                PIF_EVENT_INTERFACE_BASE + 1
#define PIF_EVENT_INTERFACE_UP                 PIF_EVENT_INTERFACE_BASE + 2
#define PIF_EVENT_INTERFACE_DOWN               PIF_EVENT_INTERFACE_BASE + 3

#define PIF_EVENT_INTERFACE_ADDRESS_ADD         PIF_EVENT_INTERFACE_BASE + 4
#define PIF_EVENT_INTERFACE_ADDRESS_DEL        PIF_EVENT_INTERFACE_BASE + 5

/****************************************************************************
 * Message format (Interface Event)
 * 
 * EVENT_INTERFACE_ADD/EVENT_INTERFACE_DELETE
 * EVENT_INTERFACE_UP/EVENT_INTERFACE_DOWN
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                     Interface Event ID                        | 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                   Interface Name String                       | 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                        Interface ID                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      Interface Index                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |          ifType                 | adminState    | operState   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                          HW Address                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                            Flags                              |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                            Metric                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                             MTU                               |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                             MTU6                              |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                          Bandwidth                            |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
 
typedef struct
{
	/** Event ID (4 bytes) */
    Int32T eId;

	/** Interface name (4 bytes) */
    Int8T  ifName[32];  

	/** Interface ID (4 bytes) */
    Int32T iId;

	/** Interface index (4 bytes) */
    Int32T ifIndex;

#define PIF_INTERFACE_TYPE_UNKNOWN		0
#define PIF_INTERFACE_TYPE_LOOPBACK		1
#define PIF_INTERFACE_TYPE_ETHERNET		2
#define PIF_INTERFACE_TYPE_HDLC			3
#define PIF_INTERFACE_TYPE_PPP			4
#define PIF_INTERFACE_TYPE_ATM			5
#define PIF_INTERFACE_TYPE_FRELAY		6
#define PIF_INTERFACE_TYPE_VLAN			7
#define PIF_INTERFACE_TYPE_PORT			8
#define PIF_INTERFACE_TYPE_AGGREGATE	9
#define PIF_INTERFACE_TYPE_MANAGE		10
#define PIF_INTERFACE_TYPE_IPIP			11
#define PIF_INTERFACE_TYPE_GREIP		12
#define PIF_INTERFACE_TYPE_IPV6IP		13
#define PIF_INTERFACE_TYPE_6TO4			14
#define PIF_INTERFACE_TYPE_ISATAP		15
	/** Interface type */
    Int16T ifType;

	/** state (admin/oper) */
    Int8T adminState;
    Int8T operState;

	/** HW Address */
    Int8T hwAddr[32];

	/** Interface flags from linux kernel */
    Uint32T flags;

	/** metric  */
    Int32T metric;

	/** mtu  */
    Int32T mtu;

	/** bandwidth  */
    Int32T bandwidth;

} PifInterfaceEventMsgT;

 /* EVENT_INTERFACE_ADDRESS_ADD/EVENT_INTERFACE_ADDRESS_DELETE
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                     Interface Event ID                        | 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                   Interface Name String                       | 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                        Interface ID                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      Interface Index                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    Flags      |     Family    |        PrefixLen              |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                    Prefix Address (v4, v6)                    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                     Destination (V4, V6)                      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define VARIABLE_ADDR_SIZE	32
typedef struct
{
	/** Event ID (4 bytes) */
    Int32T eId;

	/** Interface name (4 bytes) */
    Int8T  ifName[32];  

	/** Interface ID (4 bytes) */
    Int32T iId;

	/** Interface index (4 bytes) */
    Int32T ifIndex;

	/** Address flags from linux ifa_flags */
    Uint8T flags;

	/** Address Family */
    Uint8T family;

	/** Prefix Length */
    Uint16T prefixLen;

	/** Prefix address, Broadcast address */
    Uint8T  variableAddrBuf[VARIABLE_ADDR_SIZE];

} PifInterfaceAddrEventMsgT;


#endif   /* _pifMsgEventData_h */

