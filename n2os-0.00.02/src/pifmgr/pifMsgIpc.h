#if !defined(_pifIpc_h)
#define _pifIpc_h

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

#define PIF_PROTOCOL_IF_BASE               IPC_NUM_PIF_MANAGER_START
#define PIF_xSTP_PORT_STATE_SET            PIF_PROTOCOL_IF_BASE + 1

/*******************************************************************
 * Message format (STP IPC Message)
 * PIF_xSTP_PORT_STATE_SET
 * 
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                        Interface ID                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                          ifIndex                              |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |             State             |           Instance            |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
typedef struct
{
	/** Interface ID (4 bytes) */
    Int32T iid;

	/** Interface index (4 bytes) */
    Int32T ifIndex;
	
#define PIF_PORT_STATE_DISABLED          0
#define PIF_PORT_STATE_LISTENING         1
#define PIF_PORT_STATE_LEARNING          2
#define PIF_PORT_STATE_FORWARDING        3
#define PIF_PORT_STATE_BLOCKING          4
#define PIF_PORT_STATE_DISCARDING        5
	/** Port state (2 bytes) */
    Int16T state;

	/** mSTP instance id (2 bytes) */
    Int16T instance;
} PiStpMsgT;


/* LACP */
#define PIF_LACP_AGGREGATE_ADD             PIF_PROTOCOL_IF_BASE + 5
#define PIF_LACP_AGGREGATE_DEL             PIF_PROTOCOL_IF_BASE + 6
#define PIF_LACP_ADD_PORT_TO_AGG           PIF_PROTOCOL_IF_BASE + 7
#define PIF_LACP_DEL_PORT_TO_AGG           PIF_PROTOCOL_IF_BASE + 8
/* Below functions could be implemented in next year */ 
#define PIF_LACP_COLLECTING_ENABLE         PIF_PROTOCOL_IF_BASE + 9
#define PIF_LACP_COLLECTING_DISABLE        PIF_PROTOCOL_IF_BASE + 10
#define PIF_LACP_DISTRIBUTING_ENABLE       PIF_PROTOCOL_IF_BASE + 11
#define PIF_LACP_DISTRIBUTING_DISABLE      PIF_PROTOCOL_IF_BASE + 12
#define PIF_LACP_LOAD_BALANCE_SET          PIF_PROTOCOL_IF_BASE + 13
#define PIF_LACP_PORT_STATUS_GET           PIF_PROTOCOL_IF_BASE + 14

/*******************************************************************
 * Message format (LACP IPC Message)
 * PIF_LACP_AGGREGATE_ADD / PIF_LACP_AGGREGATE_DEL
 * PIF_LACP_ADD_PORT_TO_AGG / PIF_LACP_DEL_PORT_TO_AGG
 * 
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                       Agg Name (4 bytes)                      | 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                            Agg Id                             | 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |             Agg Mode          |           LB mode             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         Mac Address (6 bytes)                 |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |         mac addr(cont.)       |       4 byte  align           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         port list count                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                          ifIndex ...                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */


#define LACP_PORTS_MAX                   16
typedef struct
{   
	/** aggregator name e.g. "port-channel 1" */
    Int8T  aggName[32];

	/** aggregator id e.g. "1" */
    Int32T aggId;

#define LACP_AGGR_MODE_BASE              0
#define LACP_AGGR_MODE_SA                LACP_AGGR_MODE_BASE + 1`
#define LACP_AGGR_MODE_DA                LACP_AGGR_MODE_BASE + 2
#define LACP_AGGR_MODE_SA_XOR_DA         LACP_AGGR_MODE_BASE + 3 
#define LACP_AGGR_MODE_SA_AND_DA         LACP_AGGR_MODE_BASE + 4
#define LACP_AGGR_MODE_SIP               LACP_AGGR_MODE_BASE + 5
#define LACP_AGGR_MODE_DIP               LACP_AGGR_MODE_BASE + 6
#define LACP_AGGR_MODE_SIP_XOR_DIP       LACP_AGGR_MODE_BASE + 7
#define LACP_AGGR_MODE_SIP_AND_DIP       LACP_AGGR_MODE_BASE + 8 
	/** aggregator mode */
    Int16T aggMode;

#define LACP_LB_MODE_SRC_MAC             0
#define LACP_LB_MODE_DST_MAC             1
	/** load balance mode */
    Int16T lbMode;

	/** aggregator MAC address */
    Int8T  macAddr[6];
    Int16T align;

	/** member port list count */
    Int32T portCount;

	/** member port list with iid */
    Int32T portIdList[LACP_PORTS_MAX];

	/** member port list with ifIndex */
    Int32T portIfIdxList[LACP_PORTS_MAX];
} PiLacpMsgT;



/* L3 refresh interface */
#define PIF_INTERFACE_REFRESH_REQ             PIF_PROTOCOL_IF_BASE + 20
#define PIF_INTERFACE_REFRESH_RES             PIF_PROTOCOL_IF_BASE + 21

/*******************************************************************
 * Message format (L3 reflesh IPC Message)
 * PIF_INTERFACE_REFRESH_REQ / PIF_INTERFACE_REFRESH_RES
 * 
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                        request component ID                   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                       interface list count                    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         interface info list                   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define PIF_ADDR_MAX_COUNT  	8
#define PIF_IF_MAX  			16

typedef struct
{
	/** Address Family */
	Uint8T family;
	Uint8T align;

    /** Prefix Length */
    Uint16T prefixLen;

    /** Prefix address  v4 or v6 */
    Uint8T  variableAddrBuf[32];

} PifInterfaceAddrMsgT;

typedef struct
{
    /** Interface name (4 bytes) */
    Int8T  ifName[32];

    /** Interface ID (4 bytes) */
    Int32T iId;

    /** Interface index (4 bytes) */
    Int32T ifIndex;

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

	/** interface address count */
    Int32T addrCount;

    /** prefix list  */
	PifInterfaceAddrMsgT prefix[PIF_ADDR_MAX_COUNT];

} PifInterfacetMsgT;

typedef struct
{   
	/** component id who want to refresh interface list */
    Int32T compId;

	/** interface list count */
    Int32T ifCount;

	/** member interface list */
    PifInterfacetMsgT ifList[PIF_IF_MAX];

} PiInterfaceRefleshMsgT;


#endif   /* _pifIpc_h */

