#if !defined(_pifCmdApi_h)
#define _pifCmdApi_h

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
#include "pif.h"
#include "pifDataTypes.h"

/************************************************
 *
 * CMI Return and Error String 
 *
 ***********************************************/

#define PIF_CMI_RETURN_OK		 	 0
#define PIF_CMI_RETURN_ERROR 		-1

#define cli_out(level, format, ...) \
printf("[%s] %s: " format, __FILE__, __FUNCTION__, ##__VA_ARGS__)

#define pifCmiOut(level, format, ...) \
		{ \
			memset(CMIErrorStr, 0, PIF_CMD_ERROR_STR_SIZE); \
			sprintf(CMIErrorStr, format, ##__VA_ARGS__); \
			pifLOG(level, "%s\n", CMIErrorStr); \
		} 

StringT pifCmiGetErrorStr();

#define PIF_CMI_PORTMGR	 	0
#define PIF_CMI_AGGMGR	 	1
#define PIF_CMI_VLANMGR	 	2
#define PIF_CMI_IPIFMGR	 	3

typedef struct
{
	Uint32T  flag;
	Uint32T  type;
	Uint32T  mode;
	Uint32T  iid;
	void    *obj;
} CmiInterfacT;





/************************************************
 *
 * CMI Interface 
 *
 ***********************************************/

/* CLI interface */
/* interface eth0 */
Int32T pifCmiIfSet(StringT ifName);

/* interface eth0   */
/* 		no shutdown */
Int32T pifCmiIfAdminSet();
/* 		shutdown */
Int32T pifCmiIfAdminUnset();


/* interface eth0   */
/* 		no switchport */
Int32T pifCmiIfSwportUnset();
/* 		switchport */
Int32T pifCmiIfSwportset();


/* interface eth0   */
/* 		speed {10|100|1000|auto} */
Int32T pifCmiIfSpeedSet(PortSpeedT speed);

/* interface eth0   */
/* 		duplex {full|half|auto} */
Int32T pifCmiIfDuplexSet(PortDuplexT duplex);

/* interface eth0   */
/* 		flowcontrol {receive|send} {on|off|desire}} */
#define 	PORT_FLOW_SEND		0
#define 	PORT_FLOW_RECEIVE	1
Int32T pifCmiIfFlowCtrlSet(Int8T dir, PortFlowCtrlT ctl);



/************************************************
 *
 * CMI Interface Address
 *
 ***********************************************/
/* interface eth0 
 * 		ip address 10.1.1.1/24 
 * 		no ip address 10.1.1.1/24 
 * 		*/
Int32T pifCmiIfAddressSet(StringT strAddr);
Int32T pifCmiIfAddressUnset(StringT strAddr);



/************************************************
 *
 * CMI VLAN 
 *
 ***********************************************/

/* interface eth0   */
/* 		switchport mode (access|trunk) */
Int32T pifCmiVlanPortModeSet(VlanPortModeT vlanMode);

/* interface eth0   */
/* 		switchport access */
/* 		switchport access vlan VID */
/* 		no switchport access vlan */
Int32T pifCmiVlanAccessVidAdd(Uint32T vid);
Int32T pifCmiVlanAccessVidDel();

/* interface eth0 
 * 		switchport trunk allowed vlan all 
 * 		switchport trunk allowed vlan none 
 * 		switchport trunk allowed vlan add VLAN_ID 
 * 		switchport trunk allowed vlan remove VLAN_ID 
 * 		switchport trunk allowed vlan except VLAN_ID 
 * 		no switchport trunk 
 * 		*/
Int32T pifCmiVlanTrunkVidAdd(Uint32T vid);
Int32T pifCmiVlanTrunkVidDel(Uint32T vid);
Int32T pifCmiVlanTrunkVidExept(Uint32T vid);
Int32T pifCmiVlanTrunkVidClear();

/* interface eth0  
 * 		no switchport trunk 
 * 		no switchport access 
 * 		*/
Int32T pifCmiVlanTrunkClear();
Int32T pifCmiVlanAccessClear();

/* interface eth0 
 * 		switchport trunk native vlan VLAN_ID
 * 		no switchport trunk native vlan
 * 		*/
Int32T pifCmiVlanTrunkNativeSet(Uint32T vid);
Int32T pifCmiVlanTrunkNativeDel();


/* vlan database 
 * 		vlan VLAN_ID name WORD mtu INT
 * 		no vlan VLAN_ID
 * 		*/
Int32T pifCmiVlanDatabaseAdd(Uint32T vid, StringT name, Uint32T mtu);
Int32T pifCmiVlanDatabaseDel(Uint32T vid);
Int32T pifCmiVlanDatabaseUpdate(Uint32T vid, StringT name, Uint32T mtu);


/************************************************
 *
 * CMI Static Link Aggragation 
 *
 ***********************************************/

/* interface eth0   */
/* 		static-channel-group 1 */
/* 		no static-channel-group */
Int32T pifCmiPortChannelAdd(Uint32T aggNumber);
Int32T pifCmiPortChannelDel();



/************************************************
 *
 * CMI MAC 
 *
 ***********************************************/

void pifCmiMacAgingSet(void* msg);
void pifCmiMacDynamicClearByAddr(void* msg);
void pifCmiMacDynamicClearByPort(void* msg);
void pifCmiMacDynamicClearByVlan(void* msg);
void pifCmiMacStaticAddrAdd(void* msg);
void pifCmiMacStaticAddrDel(void* msg);

typedef struct
{
    Int8T  ifName[IF_NAME_SIZE];
    Int8T  name[IF_NAME_SIZE];
    Int32T sequenceNo;
    Int16T type;
    Int8T  souceAddr[ETH_ADDR_LEN];
    Int8T  destAddr[ETH_ADDR_LEN];
} CmiAclMsgT;

void pifCmiMacAclTermAdd(void* msg);
void pifCmiMacAclTermDel(void* msg); 
void pifCmiMacAclPortAdd(void* msg);




/************************************************
 *
 * CMI Interface Show
 *
 ***********************************************/

/* Interface */
void pifCmiIfStatusGet(void* msg);
void pifCmiIfStatusDetailGet(void* msg);
void pifCmiIfSwportStatusGet(void* msg);
void pifCmiIfShow(void* msg);

/* MAC */
typedef struct
{   
    Int8T  ifName[IF_NAME_SIZE];
    Int8T  macAddr[ETH_ADDR_LEN];
    Int32T vid;
    Int32T agingTime;
} CmiMacMsgT;
void pifCmiMacTableGetAddr(void* msg);
void pifCmiMacTableGetAging(void* msg);
void pifCmiMacTableGetCount(void* msg);
void pifCmiMacTableGetAll(void* msg);
void pifCmiMacTableGetPort(void* msg);
void pifCmiMacTableGetVlan(void* msg);
void pifCmiMacTableGetStatic(void* msg);

/* Vlan */
typedef struct
{
    Int8T  ifName[IF_NAME_SIZE];
    Int32T mode;
    Int32T vid;
    Int32T vidRange;
} CmiVlanMsgT;

#endif   /* _pifCmdApi_h */

