#if !defined(_pifIpc_h)
#define _pifIpc_h
/*345678901234567890123456789012345678901234567890123456789012345678901234567890*/
/*********************************************************************************
 *               Electronics and Telecommunications Research Institute
 * Filename : <myFileName>
 * Blockname: <PIF Manager>
 * Overview : <PIF Manager S/W block manages Port/Interface & L2 MAC/VLAN>
 * Creator  : <Seungwoo Hong>
 * Owner    : <Seungwoo Hong>
 * Copyright: 2013 Electronics and Telecommunications Research Institute. 
 *            All rights reserved. No part of this software shall be reproduced, 
 *            stored in a retrieval system, or transmitted by any means, 
 *            electronic, mechanical, photocopying, recording, or otherwise, 
 *            without written permission from ETRI.
 *********************************************************************************/
/*********************************************************************************
 *                        VERSION CONTROL  INFORMATION
 * $Author$
 * $Date$
 * $Revision
 * $Log$ 
 *********************************************************************************/
#include "nosLib.h"

#define PIF_BUFFER_SIZE  1024
#define PIF_RLT_STR_SIZE 1024
#define PIF_IPC_HEADER_SIZE sizeof(PifIpcMsgHeaderT) 
#define ETHER_ADDR_LEN 14

typedef struct
{
    Int32T cmdFd;
    Int32T cmdKey;
} PifIpcMsgHeaderT;

typedef struct
{
	PifIpcMsgHeaderT header;
	Int8T data[PIF_BUFFER_SIZE];
} PifIpcMsgT;


/* CMI Port/Interface */

#define IF_NAME_SIZE   64
#define ACL_NAME_SIZE  64


typedef struct
{
    Int8T  ifName[IF_NAME_SIZE];
    Int16T ifType;
    Int16T shelfId;
    Int16T slotId;
    Int16T portId;
    Int32T speed;
    Int32T duplex;
    Int32T flowControl;
    Int32T address;
    Int32T mask;
} CmiInterfaceMsgT;

void pifCmiIfSet(void* msg);
void pifCmiIfAdminUnset(void* msg);
void pifCmiIfAdminSet(void* msg);
void pifCmiIfSwportUnset(void* msg);
void pifCmiIfSwportset(void* msg);
void pifCmiIfSpeedSet(void* msg);
void pifCmiIfDuplexSet(void* msg);
void pifCmiIfFlowCtrlSet(void* msg);
void pifCmiIfAddressSet(void* msg);
void pifCmiIfStatusGet(void* msg);
void pifCmiIfStatusDetailGet(void* msg);
void pifCmiIfSwportStatusGet(void* msg);
//test
void pifCmiIfShow(void* msg);

/* CMI MAC */
typedef struct
{   
    Int8T  ifName[IF_NAME_SIZE];
    Int8T  macAddr[ETHER_ADDR_LEN];
    Int32T vid;
    Int32T agingTime;
} CmiMacMsgT;

void pifCmiMacAgingSet(void* msg);
void pifCmiMacDynamicClearByAddr(void* msg);
void pifCmiMacDynamicClearByPort(void* msg);
void pifCmiMacDynamicClearByVlan(void* msg);
void pifCmiMacStaticAddrAdd(void* msg);
void pifCmiMacStaticAddrDel(void* msg);
void pifCmiMacTableGetAddr(void* msg);
void pifCmiMacTableGetAging(void* msg);
void pifCmiMacTableGetCount(void* msg);
void pifCmiMacTableGetAll(void* msg);
void pifCmiMacTableGetPort(void* msg);
void pifCmiMacTableGetVlan(void* msg);
void pifCmiMacTableGetStatic(void* msg);

typedef struct
{
    Int8T  ifName[IF_NAME_SIZE];
    Int8T  name[ACL_NAME_SIZE];
    Int32T sequenceNo;
    Int16T type;
    Int8T  souceAddr[ETHER_ADDR_LEN];
    Int8T  destAddr[ETHER_ADDR_LEN];
} CmiAclMsgT;

void pifCmiMacAclTermAdd(void* msg);
void pifCmiMacAclTermDel(void* msg); 
void pifCmiMacAclPortAdd(void* msg);

/* CMI Vlan */
typedef struct
{
    Int8T  ifName[IF_NAME_SIZE];
    Int32T mode;
    Int32T vid;
    Int32T vidRange;
} CmiVlanMsgT;

void pifCmiVlanPortModeSet(void* msg);
void pifCmiVlanAccessVidAdd(void* msg); 
void pifCmiVlanAccessVidDel(void* msg);
void pifCmiVlanTrunkVidAdd(void* msg);
void pifCmiVlanTrunkVidDel(void* msg);
void pifCmiVlanTrunkNativeSet(void* msg);
void pifCmiVlanDatabaseAdd(void* msg);
void pifCmiVlanDatabaseDel(void* msg);

/* STP/LACP */
typedef struct
{
    Int32T ifIndex;
    Int16T flags;
} PiStpMsgT;
void PifPiStpPortStateSet(void *msg);

#define LACP_PORTS_MAX      16
typedef struct
{   
    Int8T  aggName[IF_NAME_SIZE];
    Int32T aggId;
    Int8T  macAddr[ETHER_ADDR_LEN];
    Int32T portCount;
    Int32T portList[LACP_PORTS_MAX];
    Int32T aggMode;
    Int32T lbMode;
} CmiLacpMsgT;

void PifPiLacpAggrateAdd(void *msg);
void PifPiLacpAggrateDel(void *msg);
void PifPiLacpAttachMuxToAgg(void *msg);
void PifPiLacpDetachMuxToAgg(void *msg);
void PifPiLacpCollectingEnable(void *msg);
void PifPiLacpCollectingDisable(void *msg);
void PifPiLacpDistributingEnable(void *msg);
void PifPiLacpDistributingDisable(void *msg);
void PifPiLacpLoadBalanceSet(void *msg);
void PifPiLacpPortStatusGet(void *msg);

/*************** CMI Interfaces ***************/
/* INTERFACE */
#define PIF_IF_SET                             1
#define PIF_IF_ADMIN_UNSET                     2
#define PIF_IF_ADMIN_SET                       3
#define PIF_IF_SWPORT_UNSET                    4
#define PIF_IF_SWPORT_SET                      5
#define PIF_IF_SPEED_SET                       6
#define PIF_IF_DUPLEX_SET                      7
#define PIF_IF_FLOWCTL_SET                     8
#define PIF_IF_ADDRESS_SET                     9
#define PIF_IF_STATUS_GET                      10
#define PIF_IF_STATUS_DETAIL_GET               11
#define PIF_IF_SWPORT_STATUS_GET               12
/* MAC */
#define PIF_MAC_AGING_SET                      31
#define PIF_MAC_DYNAMIC_ADDR_CLEAR             32
#define PIF_MAC_DYNAMIC_PORT_CLEAR             33
#define PIF_MAC_DYNAMIC_VLAN_CLEAR             34
#define PIF_MAC_STATIC_SET                     35
#define PIF_MAC_STATIC_DEL                     36
#define PIF_MAC_ADDRESS_GET                    37
#define PIF_MAC_AGING_GET                      38
#define PIF_MAC_COUNT_GET                      39
#define PIF_MAC_DYNAMIC_GET                    41
#define PIF_MAC_STATIC_GET                     42 
#define PIF_MAC_VLAN_GET                       43
#define PIF_MAC_PORT_GET                       44
#define PIF_MAC_ACL_ADD                        45
#define PIF_MAC_ACL_DEL                        46
#define PIF_MAC_ACL_PORT_ADD                   47
/* VLAN */
#define PIF_VLAN_PORT_MODE_SET                 61
#define PIF_VLAN_ACCESS_VID_ADD                62
#define PIF_VLAN_ACCESS_VID_DEL                63
#define PIF_VLAN_TRUNK_VID_ADD                 64
#define PIF_VLAN_TRUNK_VID_DEL                 65
#define PIF_VLAN_TRUNK_NATIVE                  66 
#define PIF_VLAN_DATABASE_ADD                  67
#define PIF_VLAN_DATABASE_DEL                  68
#define PIF_VLAN_DATABASE_GET                  69

/****PROTOCOLS(STP/LACP) Interfaces ************/
#define PIF_xSTP_PORT_STATE_SET                81
#define PIF_LACP_AGGREGATE_ADD                 85
#define PIF_LACP_AGGREGATE_DEL                 86
#define PIF_LACP_ATTACH_MUX_TO_AGG             87
#define PIF_LACP_DETACH_MUX_TO_AGG             88
#define PIF_LACP_COLLECTING_ENABLE             89
#define PIF_LACP_COLLECTING_DISABLE            90
#define PIF_LACP_DISTRIBUTING_ENABLE           91
#define PIF_LACP_DISTRIBUTING_DISABLE          92
#define PIF_LACP_LOAD_BALANCE_SET              93
#define PIF_LACP_PORT_STATUS_GET               94

/********** Event Interfaces ************/
/* L2 Port Event */
#define PIF_PORT_EVENT                         100
#define PIF_EVENT_PORT_ADD                     101 
#define PIF_EVENT_PORT_DEL                     102 
#define PIF_EVENT_VLAN_ADD                     103
#define PIF_EVENT_VLAN_DEL                     104 
#define PIF_EVENT_LINK_UP                      105
#define PIF_EVENT_LINK_DOWN                    106
/* L3 Interface Event */ 
#define PIF_INTERFACE_EVENT                    200
#define PIF_EVENT_INTERFACE_ADD                201
#define PIF_EVENT_INTERFACE_DEL                202
#define PIF_EVENT_INTERFACE_ADDRES_ADD         203
#define PIF_EVENT_INTERFACE_ADDRESS_DEL        204
#define PIF_EVENT_INTERFACE_UP                 205
#define PIF_EVENT_INTERFACE_DOWN               206


/************** IPC to Function Map *************/
#define PIF_IF_SHOW                            13
#define PIF_IPC_MAP_MAX                        100
static const struct
{
    Int32T seq;
    Int32T id;
    void (*func)(void*);
} pifIpcWorkFuction[PIF_IPC_MAP_MAX] __attribute__ ((unused)) =
{
    {0, 0,                                0                            },
    /* CMI Port/Interface */
    {1,   PIF_IF_SET,                     pifCmiIfSet                  },
    {2,   PIF_IF_ADMIN_UNSET,             pifCmiIfAdminUnset           },
    {3,   PIF_IF_ADMIN_SET,               pifCmiIfAdminSet             },
    {4,   PIF_IF_SWPORT_UNSET,            pifCmiIfSwportUnset          },
    {5,   PIF_IF_SWPORT_SET,              pifCmiIfSwportset            },
    {6,   PIF_IF_SPEED_SET,               pifCmiIfSpeedSet             },
    {7,   PIF_IF_DUPLEX_SET,              pifCmiIfDuplexSet            },
    {8,   PIF_IF_FLOWCTL_SET,             pifCmiIfFlowCtrlSet          },
    {9,   PIF_IF_ADDRESS_SET,             pifCmiIfAddressSet           },
    {10,  PIF_IF_STATUS_GET,              pifCmiIfStatusGet            },
    {11,  PIF_IF_STATUS_DETAIL_GET,       pifCmiIfStatusDetailGet      },
    {12,  PIF_IF_SWPORT_STATUS_GET,       pifCmiIfSwportStatusGet      },
    {13,  PIF_IF_SHOW,                    pifCmiIfShow                 },
    {14,  0,                              0                            },
    {15,  0,                              0                            },
    {16,  0,                              0                            },
    {17,  0,                              0                            },
    {18,  0,                              0                            },
    {19,  0,                              0                            },
    {20,  0,                              0                            },
    {21,  0,                              0                            },
    {22,  0,                              0                            },
    {23,  0,                              0                            },
    {24,  0,                              0                            },
    {25,  0,                              0                            },
    {26,  0,                              0                            },
    {27,  0,                              0                            },
    {28,  0,                              0                            },
    {29,  0,                              0                            },
    {30,  0,                              0                            },
    /* CMI MAC */
    {31,  PIF_MAC_AGING_SET,              pifCmiMacAgingSet            },
    {32,  PIF_MAC_DYNAMIC_ADDR_CLEAR,     pifCmiMacDynamicClearByAddr  },
    {33,  PIF_MAC_DYNAMIC_PORT_CLEAR,     pifCmiMacDynamicClearByPort  },
    {34,  PIF_MAC_DYNAMIC_VLAN_CLEAR,     pifCmiMacDynamicClearByVlan  },
    {35,  PIF_MAC_STATIC_SET,             pifCmiMacStaticAddrAdd       },
    {36,  PIF_MAC_STATIC_DEL,             pifCmiMacStaticAddrDel       },
    {37,  PIF_MAC_ADDRESS_GET,            pifCmiMacTableGetAddr        },
    {38,  PIF_MAC_AGING_GET,              pifCmiMacTableGetAging       },
    {39,  PIF_MAC_COUNT_GET,              pifCmiMacTableGetCount       },
    {40,  PIF_MAC_DYNAMIC_GET,            pifCmiMacTableGetAll         },
    {41,  PIF_MAC_VLAN_GET,               pifCmiMacTableGetVlan        },
    {42,  PIF_MAC_PORT_GET,               pifCmiMacTableGetPort        },
    {43,  PIF_MAC_STATIC_GET,             pifCmiMacTableGetStatic      },
    {44,  PIF_MAC_ACL_ADD,                pifCmiMacAclTermAdd          },
    {45,  PIF_MAC_ACL_DEL,                pifCmiMacAclTermDel          },
    {46,  PIF_MAC_ACL_PORT_ADD,           pifCmiMacAclPortAdd          },
    {47,  0,                              0                            },
    {48,  0,                              0                            },
    {49,  0,                              0                            },
    {50,  0,                              0                            },
    {51,  0,                              0                            },
    {52,  0,                              0                            },
    {53,  0,                              0                            },
    {54,  0,                              0                            },
    {55,  0,                              0                            },
    {56,  0,                              0                            },
    {57,  0,                              0                            },
    {58,  0,                              0                            },
    {59,  0,                              0                            },
    {60,  0,                              0                            },
    /* CMI Vlan */
    {61,  PIF_VLAN_PORT_MODE_SET,         pifCmiVlanPortModeSet        },
    {62,  PIF_VLAN_ACCESS_VID_ADD,        pifCmiVlanAccessVidAdd       },
    {63,  PIF_VLAN_ACCESS_VID_DEL,        pifCmiVlanAccessVidDel       },
    {64,  PIF_VLAN_TRUNK_VID_ADD,         pifCmiVlanTrunkVidAdd        },
    {65,  PIF_VLAN_TRUNK_VID_DEL,         pifCmiVlanTrunkVidDel        },
    {66,  PIF_VLAN_TRUNK_NATIVE,          pifCmiVlanTrunkNativeSet     },
    {67,  PIF_VLAN_DATABASE_ADD,          pifCmiVlanDatabaseAdd        },
    {68,  PIF_VLAN_DATABASE_DEL,          pifCmiVlanDatabaseDel        },
    {69,  0,                              0                            },
    {70,  0,                              0                            },
    {71,  0,                              0                            },
    {72,  0,                              0                            },
    {73,  0,                              0                            },
    {74,  0,                              0                            },
    {75,  0,                              0                            },
    {76,  0,                              0                            },
    {77,  0,                              0                            },
    {78,  0,                              0                            },
    {79,  0,                              0                            },
    {80,  0,                              0                            },
    /* Protocol xSTP */
    {81, PIF_xSTP_PORT_STATE_SET,         PifPiStpPortStateSet         },
    {82,  0,                              0                            },
    {83,  0,                              0                            },
    {84,  0,                              0                            },
    /* Protcol LACP */
    {85, PIF_LACP_AGGREGATE_ADD,          PifPiLacpAggrateAdd          },
    {86, PIF_LACP_AGGREGATE_DEL,          PifPiLacpAggrateDel          },
    {87, PIF_LACP_ATTACH_MUX_TO_AGG,      PifPiLacpAttachMuxToAgg      },
    {88, PIF_LACP_DETACH_MUX_TO_AGG,      PifPiLacpDetachMuxToAgg      },
    {89, PIF_LACP_COLLECTING_ENABLE,      PifPiLacpCollectingEnable    },
    {90, PIF_LACP_COLLECTING_DISABLE,     PifPiLacpCollectingDisable   },
    {91, PIF_LACP_DISTRIBUTING_ENABLE,    PifPiLacpDistributingEnable  },
    {92, PIF_LACP_DISTRIBUTING_DISABLE,   PifPiLacpDistributingDisable },
    {93, PIF_LACP_LOAD_BALANCE_SET,       PifPiLacpLoadBalanceSet      },
    {94, PIF_LACP_PORT_STATUS_GET,        PifPiLacpPortStatusGet       },
    {95,  0,                              0                            },
    {96,  0,                              0                            },
    {97,  0,                              0                            },
    {98,  0,                              0                            },
    {99,  0,                              0                            }
};

#endif   /* _pifIpc_h */

