#if !defined(_pifDataTypes_h)
#define _pifDataTypes_h
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

#include "nnList.h"
#define PListT 		ListT
#include "nnPrefix.h"

#define IF_NAME_SIZE      32
#define IF_HWADDR_MAX     20 // i don't know exactly, need to check

#define ETH_ADDR_LEN      6  // in nos.h
#define NOS_DEFAULT_VID   1  
#define NOS_ALL_VID       4096  
#define NOS_NONE_VID      0  

struct IPInterface; 
struct PhysicalPort;
struct AggGroup;
struct Vlan;

/* NOS Interface Types*/

typedef enum {
	TYPE_UNKNOWN     = 0,
	TYPE_LOOPBACK    ,
	TYPE_ETHERNET    ,
	TYPE_HDLC        ,
	TYPE_PPP     	,
	TYPE_ATM     	,
	TYPE_FRELAY      ,
	TYPE_VLAN        ,
	TYPE_PORT        ,
	TYPE_AGGREGATE   ,
	TYPE_MANAGE      ,
	TYPE_IPIP        ,
	TYPE_GREIP       ,
	TYPE_IPV6IP      ,
	TYPE_6TO4        ,
	TYPE_ISATAP      
} InterfaceTypeT;


/* NOS Interface Index Macro */
/* examples
 * -. physical interface "eth0" 
 * 	- shelf(0), slot(0), port(0), type(2), id(0)
 * -. physical interface "fastethernet0/1/1" 
 * 	- shelf(0), slot(1), port(1), type(2), id(0)
 * -. physical sub-interface "fastethernet 1/1.2" 
 * 	- shelf(0), slot(1), port(1), type(2), id(2)
 * -. port-channel interface "port-channel 1" 
 * 	- shelf(0), slot(0), port(0), type(9), id(1)
 * -. svi interface "interface vlan 10" 
 * 	- shelf(0), slot(0), port(0), type(7), id(10)
 * -. loopback interface "interface lo 0" 
 * 	- shelf(0), slot(0), port(0), type(1), id(0)
 * -. tunnel interface "interface tunnel 1" 
 * 	- shelf(0), slot(0), port(0), type(11), id(1)
 * -. null interface "interface null 0" 
 * 	- shelf(0), slot(0), port(0), type(0), id(0)
 * */
typedef union InterfaceId 
{
	Uint32T  idx;
#define Shelf	bits.shelf
#define Slot	bits.slot
#define Port	bits.port
#define Type	bits.type
#define Id		bits.id
	struct _bits
	{
		/* shelf number (0~15) */
		Uint32T  shelf:4;
		/* slot number (0-15) */
		Uint32T  slot:4;
		/* port number (0-255) */
		Uint32T  port:8;
		/* interface enum type (0~5) */
		Uint32T  type:4;
		/* interface logical id */
		Uint32T  id:12;
	} bits;
} InterfaceIdT;

typedef enum {
    ADMIN_DOWN = 0,
    ADMIN_UP,
    STATE_DISABLED
} NoSAdminStateT;

typedef enum {
    STATE_DOWN = 0,
    STATE_UP
} NoSOperStateT;

typedef enum {
    FAST_ETHERNET = 0,
    GIGA_ETHERNET,
    AGG_ETHERNET
} PortHwTypeT;

typedef enum {
    ENCAP_ARPA = 0,
    ENCAP_SNAP,
    ENCAP_DOT1Q
} PortEncapTypeT;

typedef enum {
    PORT_SPEED_10 = 0,
    PORT_SPEED_100,
    PORT_SPEED_1000,
    PORT_SPEED_10000,
    PORT_SPEED_AUTO
} PortSpeedT;

typedef enum {
    PORT_DUPLEX_HALF = 0,
    PORT_DUPLEX_FULL,
    PORT_DUPLEX_AUTO
} PortDuplexT;

typedef enum {
    FLOWCTL_ON = 0,
    FLOWCTL_OFF,
    FLOWCTL_DESIRED
} PortFlowCtrlT;

typedef struct {
	PortFlowCtrlT send;
	PortFlowCtrlT receive;
} PortFlowControlT;

typedef enum {
    NOT_CONFIGURED = 0,
    SWITCH_PORT,
    ROUTED_PORT
} PortConfigModeT;
#define PIF_DEFAULT_PORTMODE	ROUTED_PORT

typedef enum {
    VLAN_NOT_CONFIGURED   = 0,
    VLAN_MODE_ACCESS,
    VLAN_MODE_TRUNK
} VlanPortModeT;


typedef enum {
    PORT_STATE_DISABLED   = 0,
	PORT_STATE_LISTENING,
	PORT_STATE_LEARNING,
	PORT_STATE_FORWARDING,
	PORT_STATE_BLOCKING
} PortStateT;



#define PIF_VLAN_MAX	4096
typedef struct SwitchPort
{ 
	/* Port Forwarding State */
	/* PORT_STATE_DISABLED/PORT_STATE_LISTENING/
	 * PORT_STATE_LEARNING/PORT_STATE_FORWARDING/
	 * PORT_STATE_BLOCKING
	 */
	PortStateT	  fwdState;

	/* VLAN_MODE_ACCESS/VLAN_MODE_TRUNK */
	VlanPortModeT vlanMode;

	/* ACCESS VLAN */
	Uint32T       accessVid; 

	/* TRUNK VLAN */
	Uint32T       trunkNativeVid;

	/* TRUNK VLAN Allowed Mode */
#define PIF_VLAN_ALLOWED_NOT_CONFIG         0
#define PIF_VLAN_ALLOWED_ALL                1
#define PIF_VLAN_ALLOWED_NONE               2
#define PIF_VLAN_ALLOWED_CONFIG             3
	Uint32T       trunkVlanAllowedMode;

	/* NOTE !!! need some method to process range values */

	/* Allowed Vlan Flag Index */
#define PIF_VLAN_ALLOWED_FLAGE_DISABLE         0
#define PIF_VLAN_ALLOWED_FLAGE_ENABLE          1
	Uint8T        trunkAllowedVid[PIF_VLAN_MAX+1];

	/* Removed Vlan Flag Index */
	Uint8T        trunkRemoveVid[PIF_VLAN_MAX+1];

} SwitchPortT;

typedef struct RoutedPort
{
	/*- Interface metric */
	Uint32T  metric;

	/*- Interface MTU. */
	Uint32T  mtu;

	/*- Interface bandwidth */
	Uint32T  bandwidth;

	/*- Interface flags in linux os */
	Uint32T  flags;

	/* connected IP interface */
	struct IPInterface* connectedIpIf;

	/* connected IP sub-interface index(pointer?) list */
	PListT*        connectedSubIpIfs;

} RoutedPortT;

typedef struct PortHwConfig
{
	Uint32T shelfId;
	Uint32T slotId;
	Uint32T portId;
} PortHwConfigT;

/* PhysicalPort */
typedef struct PhysicalPort
{
	/* ethernet 0/1 */
	Int8T name[IF_NAME_SIZE];
	InterfaceIdT  iid;
	Uint32T		  ifIndex; 
	Uint32T		  bandwidth; 

	NoSAdminStateT adminState;
	NoSOperStateT  operState;

	PortHwTypeT    hwType;
	Uint8T         hwAddr[IF_HWADDR_MAX];

	PortEncapTypeT    encapType;
	PortSpeedT        speed;
	PortDuplexT       duplex;
	PortFlowControlT  flowCtrl;

	PortConfigModeT portMode;  /* NOT_CONFIGURED/SWITCH_PORT/ROUTED_PORT */
	SwitchPortT     switchPort;
	RoutedPortT     routedPort;

	/* Aggregator information */
	Uint32T  aggId;

} PhysicalPortT;

/* Port Group */
typedef struct AggGroup
{
	/* port-channel 0 */
	Int8T name[IF_NAME_SIZE];
	InterfaceIdT  iid;
	Uint32T		  ifIndex; 
	Uint32T		  bandwidth; 

	NoSAdminStateT adminState;
	NoSOperStateT  operState;

	PortHwTypeT    hwType;
	Uint8T         hwAddr[IF_HWADDR_MAX];

	PortEncapTypeT encapType;
	PortSpeedT     speed;
	PortDuplexT    duplex;
	PortFlowCtrlT  flowCtrl;

	/* SWITCH_PORT/ROUTED_PORT */
	PortConfigModeT portMode; 
	SwitchPortT     switchPort;
	RoutedPortT     routedPort;

	/* Aggregator information */
	Uint32T  aggId;
	Int16T   aggMode;
	Int16T   staticFlag;
	PListT*  portList;

} AggGroupT;

/* VLAN */
typedef struct Vlan
{
	/* vlan description name */
	Int8T name[IF_NAME_SIZE];

	/* vlan id */
	Uint32T       vid;

	/* nos admin/oper state */
	NoSAdminStateT adminState;
	NoSOperStateT  operState;

	/* MTU size */
	Uint32T       mtu;

	/* PhysicalPortT index List */
	PListT*  portList;

	/* connected SVI IP interface */
	struct IPInterface*  connectedIpIf;

} VlanT;


/* Static MAC Address */
#define PIF_DEFAULT_AGING_TIME      300
typedef struct StaticMac
{
	/* static mac address */
	Int8T macAddr[ETH_ADDR_LEN];

	/* type */
#define PIF_MAC_TYPE_CLI            0
#define PIF_MAC_TYPE_PROTO_1        1
#define PIF_MAC_TYPE_PROTO_2        2
	Uint32T       type;

	/* Vlan Id */
	Uint32T       vid;

	/* PhysicalPortT index */
	Uint32T       iid;

	/* PhysicalPortT interface index */
	Uint32T       ifIndex;
	Int8T         ifName[IF_NAME_SIZE];

	/* PhysicalPortT index List (same mac in multiple ports) */
	/* currently not support (will be done in next year) */
	PListT*  	  portList;

} StaticMacT;


/* Connected IP Address */
typedef struct ConnectedAddress
{
	/* IP interface that address belong to */
	struct IPInterface	*ifp;

	/* linux ifa_flags */
#define PIF_IFA_SECONDARY     (1 << 1)
#define PIF_IFA_ANYCAST       (1 << 2)
#define PIF_IFA_VIRTUAL       (1 << 2)
	Uint8T flags;

	/* Address Prefix */ 
	PrefixT address;
	PrefixT broadcast;

} ConnectedAddressT ;

/* IP Interface */
#define PIF_DEFAULT_MTU      	1500
#define PIF_DEFAULT_METRIC      4
#define PIF_NONE_VID 	     	0
#define PIF_DEFAULT_VID      	1
#define PIF_DEFAULT_BRIDGE     	0
typedef struct IPInterface
{
	/*- ethernet 0/1 */
	Int8T name[IF_NAME_SIZE];
	InterfaceIdT  iid;
	Uint32T		  ifIndex; 

	/* shelf/slot/port */
	PortHwConfigT hwConfig;

	/*- nos admin/oper state */
	NoSAdminStateT  adminState;
	NoSOperStateT   operState;

	/*- SWITCH_PORT/ROUTED_PORT/NOT_CONFIGURED */
	PortConfigModeT ifMode;

	/*- IP interface type */
	InterfaceTypeT ifType;

	/*- Interface metric */
	Uint32T  metric;

	/*- Interface MTU. */
	Uint32T  mtu;

	/*- Interface bandwidth */
	Uint32T  bandwidth;

	/*- Interface flags in linux os */
	Uint32T  flags;

	/* connected address list */
	PListT*   connectedAddrs; 

	/* port that interface belong to */
	PhysicalPortT* attachedPort; 

	/* Vlan that interface(in case SVI) belong to */
	VlanT* attachedVlan;

	/* Agg that interface(in case port-channel) belong to */
	AggGroupT* attachedAgg;

} IPInterfaceT;


/* Instantiation of Data Structures */
PhysicalPortT* newPhysicalPort(void);
void freePhysicalPort(PhysicalPortT* port);

AggGroupT* newAggGroup(void);
void freeAggGroup(AggGroupT* agg);

VlanT* newVlan(void);
void freeVlan(VlanT* vlan);
void clearVlan(VlanT* vlan);

StaticMacT* newStaticMac(void);
void freeStaticMac(StaticMacT* mac);

IPInterfaceT* newIPInterface(void);
void freeIPInterface(IPInterfaceT* ifp); 

ConnectedAddressT* newConnectedAddress(void);
void freeConnectedAddress(ConnectedAddressT* addr);


/* Get attribute of Data Structure */
SwitchPortT* getSwitchPort(PhysicalPortT* port);




/* print data structures */

/* Admin and Oper state */
void printAdminOperState(void* obj);

/* switchPort */
void printSwitchPort(SwitchPortT* port);
void printStrSwitchPort(StringT str, SwitchPortT* port);

/* physicalPort */
void printPhysicalPort(PhysicalPortT* ifp);
void printStrPhysicalPort(StringT str, PhysicalPortT* ifp);
void printStrPhysicalPortSwitchPort(StringT str, PhysicalPortT* ifp);
void dumpPhysicalPort(PhysicalPortT* ifp, StringT msg);

/* AggGroup */
void printAggGroup(AggGroupT* ifp);
void printStrAggGroup(StringT str, AggGroupT* ifp);
void dumpAggGroup(AggGroupT* ifp, StringT msg);

/* Connected Address */
void dumpConnectedAddress(IPInterfaceT* ifp, ConnectedAddressT* addr, StringT str);

/* IPInterface */
void dumpIPInterface(IPInterfaceT* ifp, StringT str);
void dumpIPInterfaceTable();
void printIPInterface(IPInterfaceT* ifp);
void printStrIPInterface(StringT str, IPInterfaceT* ifp);
void printStrIPInterfaceBrief(StringT str, IPInterfaceT* ifp);

/* Vlan */
void printStrVlan(StringT str, VlanT* vlan);




/* print data structures attributes */
const char* getIfTypeStr(Int32T ifType);
const char* getIfStateStr(Int32T state);
const char* getHwTypeStr(Int32T type);
const char* getIfModeStr(Int32T mode);
const char* getVlanModeStr(Int32T mode);
const char* getPortStateStr(Int16T state);


/* print address */
char* mac2str(char *macAddr);
char* str2mac(char *macStr);
char * ip2str(uint32_t ip);
uint32_t str2ip(char *ipstr);



#define PLIST_LOOP(L,V,N) \
  if (L) \
    for ((N) = (L)->pHead; (N); (N) = (N)->pNext) \
      if (((V) = (N)->pData) != NULL)

#define strPrintf(format, ...) str += sprintf(str, format,  ##__VA_ARGS__)





#endif   /* _pifDataTypes_h */

