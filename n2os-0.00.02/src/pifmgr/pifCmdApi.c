/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : pifmgr에 입력되는 IPC 메시지 처리기능을 수행
 * - Block Name : PIF Manager
 * - Process Name : pifmgr
 * - Creator : Seungwoo Hong
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : pifCmdApi.c
 *
 * $Author: $
 * $Date: $
 * $Revision: $
 * $LastChangedBy: $
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "pif.h" 
#include "nosLib.h" 
#include "pifMsgIpc.h" 
#include "pifDataTypes.h" 
#include "pifCmdApi.h" 

#include "pifPortMgr.h" 
#include "pifInterfaceMgr.h" 
#include "pifVlanMgr.h" 
#include "pifAggMgr.h" 

/* Selected interface object */
static CmiInterfacT CMI;

/* Cmd interface error result string */
#define PIF_CMD_ERROR_STR_SIZE      100
Int8T  CMIErrorStr[PIF_CMD_ERROR_STR_SIZE];

StringT pifCmiGetErrorStr(){
	return CMIErrorStr;
}


/* CLI: INTERFACE */
/* interface eth0 */
Int32T pifCmiIfSet(StringT ifName)
{
	pifLOG(LOG_DEBUG, "PIF-CMI :: enter \n");

	/* verify interface name */
	/* iid */
	Uint32T iid = CMI.iid = getIidbyName(ifName);
	if(iid < 0) {
		pifCmiOut(0, "invalid interface name %s", ifName);
		return PIF_CMI_RETURN_ERROR;
	}

	/* type: physical interface should exist */
	Uint32T type = CMI.type = getIidType(iid);
	if((type == TYPE_ETHERNET) || (type == TYPE_AGGREGATE)   )
	{
		/* case having PhysicalPort 
		 * 1. PhysicalPort:  interface eth0, eth1, fastethernet0/1  
		 * 2. AggregatorPort:  interface po1, port-channel 1  
		 * 3. Sub-Interface: interface eth0.1 fastethernet0/1.1 */ 

		Uint32T lid = getIidLogicalId(iid);

		/* case 1 */
		if(!lid) {
			PhysicalPortT *port = pifPortMgrGetPort(iid);
			if(port == NULL) {
				pifCmiOut(0, "%s interface not exist", ifName);
				CMI.obj = NULL;
				return PIF_CMI_RETURN_ERROR;
			}
			CMI.flag = PIF_CMI_PORTMGR;
			CMI.obj = (void*)port;
			return PIF_CMI_RETURN_OK;
		}
		/* case 2 */
		/* PortMgr will make new logical interface, if not exist */
		else {
			IPInterfaceT *subPort = pifPortMgrGetSubInterface(iid);
			if(subPort == NULL) {
				pifCmiOut(0, "%s logical interface not exist", ifName);
				return PIF_CMI_RETURN_ERROR;
			}
			CMI.flag = PIF_CMI_IPIFMGR;
			CMI.obj = (void*)subPort;
			return PIF_CMI_RETURN_OK;
		}
	}
	/*
	else if(type == TYPE_AGGREGATE) 
	{
	}
	else if(type == TYPE_VLAN) 
	{
	}
	*/
	else 
	{
		/* other cases are logical IPInterface */
		/* interface loopback, svi, tunnel, null ... */
		/* which can make dynamically */

		IPInterfaceT *ifp = pifInterfaceMgrGetInterface(iid);
		if(ifp == NULL) {
			pifCmiOut(0, "%s interface not exist", ifName);
			CMI.obj = NULL;
			return PIF_CMI_RETURN_ERROR;
		}
		CMI.flag = PIF_CMI_IPIFMGR;
		CMI.obj = (void*)ifp;
		return PIF_CMI_RETURN_OK;
	}

	pifLOG(LOG_DEBUG, "PIF-CMI :: end \n");
	return PIF_CMI_RETURN_OK;
}

/* interface eth0 
 * 		no shutdown */
Int32T pifCmiIfAdminSet()
{
	Uint32T iid     = CMI.iid;
	Uint32T ifType  = CMI.type;
	void*   dataObj = CMI.obj;

	pifLOG(LOG_DEBUG, "PIF-CMI :: enter (%s) \n", getIidStr(iid));

	if(dataObj == NULL){
		pifCmiOut(0, "unkown software error");
		return PIF_CMI_RETURN_ERROR;
	}

	Int32T rlt;
	if((ifType == TYPE_ETHERNET) || (ifType == TYPE_AGGREGATE) )
	{
		rlt = pifPortMgrEnablePort((PhysicalPortT*)dataObj);
		if(rlt != PIF_RTN_OK){
			pifCmiOut(0, "interface already up");
			return PIF_CMI_RETURN_ERROR;
		}
	}
	else 
	{
		/* case logical IP interface */
		rlt = pifInterfaceMgrEnableIf((IPInterfaceT*)dataObj);
		if(rlt != PIF_RTN_OK){
			pifCmiOut(0, "interface already up");
			return PIF_CMI_RETURN_ERROR;
		}
	}

	pifLOG(LOG_DEBUG, "PIF-CMI :: end \n");
	return PIF_CMI_RETURN_OK;
} 

/* interface eth0 
 * 		shutdown */
Int32T pifCmiIfAdminUnset()
{
	Uint32T iid     = CMI.iid;
	Uint32T ifType  = CMI.type;
	void*   dataObj = CMI.obj;

	pifLOG(LOG_DEBUG, "PIF-CMI :: enter (%s) \n", getIidStr(iid));

	if(dataObj == NULL){
		pifCmiOut(0, "unkown software error");
		return PIF_CMI_RETURN_ERROR;
	}

	Int32T rlt;
	if((ifType == TYPE_ETHERNET) || (ifType == TYPE_AGGREGATE) )
	{
		rlt = pifPortMgrDisablePort((PhysicalPortT*)dataObj);
		if(rlt != PIF_RTN_OK){
			pifCmiOut(0, "interface already down");
			return PIF_CMI_RETURN_ERROR;
		}
	}
	else 
	{
		/* case logical IP interface */
		rlt = pifInterfaceMgrDisableIf((IPInterfaceT*)dataObj);
		if(rlt != PIF_RTN_OK){
			pifCmiOut(0, "interface already down");
			return PIF_CMI_RETURN_ERROR;
		}
	}

	if(rlt != PIF_RTN_OK ) {
		return PIF_CMI_RETURN_OK;
	}

	pifLOG(LOG_DEBUG, "PIF-CMI :: end \n");
	return PIF_CMI_RETURN_OK;
}



/* interface eth0 
 * 		no switchport */
Int32T pifCmiIfSwportUnset()
{
	Uint32T iid     = CMI.iid;
	Uint32T ifType  = CMI.type;
	void*   dataObj = CMI.obj;

	pifLOG(LOG_DEBUG, "PIF-CMI :: enter (%s) \n", getIidStr(iid));

	/* check interface type */
	if((ifType != TYPE_ETHERNET) && (ifType != TYPE_AGGREGATE) )
	{
		/* L2 commands are allowd for TYPE_ETHERNET and TYPE_AGGREGATE */
		pifCmiOut(0, "Can't set switchport to logical interface.");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check port mode */
	PortConfigModeT portMode = ((PhysicalPortT*)dataObj)->portMode;

	if(portMode == ROUTED_PORT){
		pifCmiOut(0, "interface already a routed port");
		return PIF_CMI_RETURN_ERROR;
	}

	if(portMode == SWITCH_PORT){
		/* check previous l2 configuration */

		/* get current port state */
		SwitchPortT* switchport = &(((PhysicalPortT*)dataObj)->switchPort);
		VlanPortModeT vMode =  switchport->vlanMode;
		Uint32T accessVid = switchport->accessVid;

		/* check port set as access port */
		if((vMode == VLAN_MODE_ACCESS) && (accessVid != PIF_DEFAULT_VID)){
			pifCmiOut(0, "access vlan %d clear first", accessVid);
			return PIF_CMI_RETURN_ERROR;
		}

		/* check port set as trunk port */
		if(vMode == VLAN_MODE_TRUNK){
			pifCmiOut(0, "trunk vlan clear first");
			return PIF_CMI_RETURN_ERROR;
		}
	}

	Int32T rlt = pifPortMgrSetRoutedPort(dataObj);

	pifLOG(LOG_DEBUG, "PIF-CMI :: end \n");

	if(rlt != PIF_RTN_OK ) {
		return PIF_CMI_RETURN_OK;
	}

	return PIF_CMI_RETURN_OK;
} 


/* interface eth0 
 * 		switchport */
Int32T pifCmiIfSwportset()
{
	Uint32T iid     = CMI.iid;
	Uint32T ifType  = CMI.type;
	void*   dataObj = CMI.obj;

	pifLOG(LOG_DEBUG, "PIF-CMI :: enter (%s) \n", getIidStr(iid));

	/* check interface type */
	if((ifType != TYPE_ETHERNET) && (ifType != TYPE_AGGREGATE) )
	{
		/* L2 commands are allowd for TYPE_ETHERNET and TYPE_AGGREGATE */
		pifCmiOut(0, "Can't set switchport to logical interface.");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check port mode */
	PortConfigModeT portMode = ((PhysicalPortT*)dataObj)->portMode;

	if(portMode == SWITCH_PORT){
		pifCmiOut(0, "interface already a switchport");
		return PIF_CMI_RETURN_ERROR;
	}

	if(portMode == ROUTED_PORT){
		/* check l3 configuration */
		// need to check ip address ?? */
	}

	Int32T rlt;
	rlt = pifPortMgrSetSwitchPort(dataObj);
	if(rlt != PIF_RTN_OK ) {
		return PIF_CMI_RETURN_OK;
	}

	pifLOG(LOG_DEBUG, "PIF-CMI :: end \n");
	return PIF_CMI_RETURN_OK;
} 


/* interface eth0   */
/* 		speed {10|100|1000|auto} */
Int32T pifCmiIfSpeedSet(PortSpeedT speed)
{
	Uint32T iid     = CMI.iid;
	Uint32T ifType  = CMI.type;
	void*   dataObj = CMI.obj;

	pifLOG(LOG_DEBUG, "PIF-CMI :: enter (%s) \n", getIidStr(iid));

	if(ifType == TYPE_ETHERNET){
		pifPortMgrSetSpeed(dataObj, speed);
	}
	else if(ifType == TYPE_AGGREGATE){
		pifCmiOut(0, "Can't set port speed to aggregator interface.");
		return PIF_CMI_RETURN_ERROR;
	}
	else {
		/* Port commands are allowd for TYPE_ETHERNET */
		pifCmiOut(0, "Can't set port speed to logical interface.");
		return PIF_CMI_RETURN_ERROR;
	}

	pifLOG(LOG_DEBUG, "PIF-CMI :: end \n");
	return PIF_CMI_RETURN_OK;
}


/* interface eth0   */
/* 		duplex {full|half|auto} */
Int32T pifCmiIfDuplexSet(PortDuplexT duplex)
{
	Uint32T iid     = CMI.iid;
	Uint32T ifType  = CMI.type;
	void*   dataObj = CMI.obj;

	pifLOG(LOG_DEBUG, "PIF-CMI :: enter (%s) \n", getIidStr(iid));

	if(ifType == TYPE_ETHERNET){
		pifPortMgrSetSpeed(dataObj, duplex);
	}
	else if(ifType == TYPE_AGGREGATE){
		pifCmiOut(0, "Can't set port duplex to aggregator interface.");
		return PIF_CMI_RETURN_ERROR;
	}
	else {
		/* Port commands are allowd for TYPE_ETHERNET */
		pifCmiOut(0, "Can't set port duplex to logical interface.");
		return PIF_CMI_RETURN_ERROR;
	}

	pifLOG(LOG_DEBUG, "PIF-CMI :: end \n");
	return PIF_CMI_RETURN_OK;
}


/* interface eth0   */
/* 		flowcontrol {receive|send} {on|off|desire}} */
Int32T pifCmiIfFlowCtrlSet(Int8T dir, PortFlowCtrlT ctl)
{
	Uint32T iid     = CMI.iid;
	Uint32T ifType  = CMI.type;
	void*   dataObj = CMI.obj;

	pifLOG(LOG_DEBUG, "PIF-CMI :: enter (%s) \n", getIidStr(iid));

	if(ifType == TYPE_ETHERNET){
		pifPortMgrSetFlow(dataObj, dir, ctl);
	}
	else if(ifType == TYPE_AGGREGATE){
		pifCmiOut(0, "Can't set port flow control to aggregator interface.");
		return PIF_CMI_RETURN_ERROR;
	}
	else {
		/* Port commands are allowd for TYPE_ETHERNET */
		pifCmiOut(0, "Can't set port flow control to logical interface.");
		return PIF_CMI_RETURN_ERROR;
	}

	pifLOG(LOG_DEBUG, "PIF-CMI :: end \n");
	return PIF_CMI_RETURN_OK;
}



/* CLI: Interface Address */

/* interface eth0 
 * 		ip address 10.1.1.1/24  */
Int32T pifCmiIfAddressSet(StringT strAddr)
{
	Uint32T iid     = CMI.iid;
	Uint32T ifType  = CMI.type;
	void*   dataObj = CMI.obj;

	pifLOG(LOG_DEBUG, "PIF-CMI :: enter (%s) \n", getIidStr(iid));
	pifLOG(LOG_DEBUG, "iid(%x) , dataobj(%p), ifType(%d) \n", iid, dataObj, ifType);

	/* get layer3 interface */
	IPInterfaceT* ifp = pifInterfaceMgrGetInterface(iid);
	if(ifp == NULL){
		pifCmiOut(0, "interface does not exist");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check interface type is L3 */
	if(ifp->ifMode != ROUTED_PORT){
		pifCmiOut(0, "interface should be no switchport");
		return PIF_CMI_RETURN_ERROR;
	}

	/* Get IP prefix */
	Prefix4T prefix;
	if(nnCnvStringtoPrefix4(&prefix, strAddr) < 0){
		pifCmiOut(0, "invalid IPv4 address format %s", strAddr);
		return PIF_CMI_RETURN_ERROR;
	}

	/* check address exist */
	 if(pifInterfaceMgrGetConnectedAddr(ifp, (PrefixT*)&prefix) != NULL){
		pifCmiOut(0, "prefix %s already exist", strAddr);
		return PIF_CMI_RETURN_ERROR;
	 }

	/* request to insert new ip address */
	Int32T rlt = pifInterfaceMgrConfigAddAddress(ifp, (PrefixT*)&prefix);
	if(rlt != PIF_RTN_OK){
		pifCmiOut(0, "fail to configure address %s", strAddr);
		return PIF_CMI_RETURN_ERROR;
	}

	pifLOG(LOG_DEBUG, "PIF-CMI :: end \n");
	return PIF_CMI_RETURN_OK;
}


/* CLI: Interface Address */

/* interface eth0 
 * 		no ip address 10.1.1.1/24  */
Int32T pifCmiIfAddressUnset(StringT strAddr)
{
	Uint32T iid     = CMI.iid;
	Uint32T ifType  = CMI.type;
	void*   dataObj = CMI.obj;

	pifLOG(LOG_DEBUG, "PIF-CMI :: enter (%s) \n", getIidStr(iid));
	pifLOG(LOG_DEBUG, "iid(%x) , dataobj(%p), ifType(%d) \n", iid, dataObj, ifType);

	/* get layer3 interface */
	IPInterfaceT* ifp = pifInterfaceMgrGetInterface(iid);
	if(ifp == NULL){
		pifCmiOut(0, "interface does not exist");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check interface type is L3 */
	if(ifp->ifMode != ROUTED_PORT){
		pifCmiOut(0, "interface should be no switchport");
		return PIF_CMI_RETURN_ERROR;
	}

	/* Get IP prefix */
	Prefix4T prefix;
	if(nnCnvStringtoPrefix4(&prefix, strAddr) < 0){
		pifCmiOut(0, "invalid IPv4 address format %s", strAddr);
		return PIF_CMI_RETURN_ERROR;
	}

	/* check address exist */
	 if(pifInterfaceMgrGetConnectedAddr(ifp, (PrefixT*)&prefix) == NULL){
		pifCmiOut(0, "prefix %s not exist", strAddr);
		return PIF_CMI_RETURN_ERROR;
	 }

	/* request to insert new ip address */
	Int32T rlt = pifInterfaceMgrConfigDelAddress(ifp, (PrefixT*)&prefix);
	if(rlt != PIF_RTN_OK){
		pifCmiOut(0, "fail to configure address %s", strAddr);
		return PIF_CMI_RETURN_ERROR;
	}

	pifLOG(LOG_DEBUG, "PIF-CMI :: end \n");
	return PIF_CMI_RETURN_OK;
}


/* CLI: Interface static-port-channel */

/* interface eth0 
 * 		static-port-channel 1  */
Int32T pifCmiPortChannelAdd(Uint32T aggId)
{
	Uint32T iid     = CMI.iid;
	Uint32T ifType  = CMI.type;
	void*   dataObj = CMI.obj;

	pifLOG(LOG_DEBUG, "PIF-CMI :: enter (%s) \n", getIidStr(iid));

	if(dataObj == NULL){
		pifCmiOut(0, "unkown software error");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check interface type */
	if(ifType != TYPE_ETHERNET){
		pifCmiOut(0, "port-channel only support ethernet type \n");
		return PIF_CMI_RETURN_ERROR;
	}

	/* get attaching port */
	PhysicalPortT* port = (PhysicalPortT*)dataObj;

	/* check target group exist */
	Int32T rlt;
	AggGroupT* agg = pifAggMgrGetGroup(aggId);
	if(agg == NULL) {
		rlt = pifAggMgrAddStaticGroup(aggId, (Int8T*)port->hwAddr);
		if(rlt != PIF_RTN_OK){
			pifCmiOut(0, "group creation error\n");
			return PIF_CMI_RETURN_ERROR;
		}
		else {
			agg = pifAggMgrGetGroup(aggId);
			if(agg == NULL) {
				pifCmiOut(0, "group creation error\n");
				return PIF_CMI_RETURN_ERROR;
			}
		}
	}

	/* add port to group */
	rlt = pifAggMgrAddPortMember(agg->name, agg->aggId, port->ifIndex);
	if(rlt != PIF_RTN_OK){
		pifCmiOut(0, "group creation error\n");
		return PIF_CMI_RETURN_ERROR;
	}

	pifLOG(LOG_DEBUG, "PIF-CMI :: end \n");
	return PIF_CMI_RETURN_OK;
} 


/* interface eth0 
 * 		no static-port-channel  */
Int32T pifCmiPortChannelDel()
{
	Uint32T iid     = CMI.iid;
	Uint32T ifType  = CMI.type;
	void*   dataObj = CMI.obj;

	pifLOG(LOG_DEBUG, "PIF-CMI :: enter (%s) \n", getIidStr(iid));

	if(dataObj == NULL){
		pifCmiOut(0, "unkown software error");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check interface type */
	if(ifType != TYPE_ETHERNET){
		pifCmiOut(0, "port-channel only support ethernet type \n");
		return PIF_CMI_RETURN_ERROR;
	}

	/* get group id from port */
	PhysicalPortT* port = (PhysicalPortT*)dataObj;
	if(port->aggId == 0){
		pifCmiOut(0, "interface %s is not static-port-channel \n", port->name);
		return PIF_CMI_RETURN_ERROR;
	}

	/* check target group exist */
	AggGroupT* agg = pifAggMgrGetGroup(port->aggId);
	if(agg == NULL){
		pifCmiOut(0, "group %d not exist\n", port->aggId);
		return PIF_CMI_RETURN_ERROR;
	}

	/* delete port from group */
	Int32T rlt = pifAggMgrDeletePortMember(agg->name, agg->aggId, port->ifIndex);
	if(rlt != PIF_RTN_OK){
		pifCmiOut(0, "group deletion error\n");
		return PIF_CMI_RETURN_ERROR;
	}

	/* group member count. If it is empty, delete group */
	if(pifAggMgrGetPortCount(agg) == 0){
		pifAggMgrDeleteStaticGroup(agg->aggId);
	}

	pifLOG(LOG_DEBUG, "PIF-CMI :: end \n");
	return PIF_CMI_RETURN_OK;
} 




/* CLI: VLAN */

/* interface eth0 
 * 		switchport mode access
 * 		switchport mode trunk  */
Int32T pifCmiVlanPortModeSet(VlanPortModeT vlanMode)
{
	Uint32T iid     = CMI.iid;
	Uint32T ifType  = CMI.type;
	void*   dataObj = CMI.obj;

	pifLOG(LOG_DEBUG, "PIF-CMI :: enter (%s) \n", getIidStr(iid));

	/* vlan port is allowed only for Ethernet & port-channel */
	if((ifType != TYPE_ETHERNET) && (ifType != TYPE_AGGREGATE)){
		pifCmiOut(0, "Layer3 Interface not allowed for vlan port");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check the port is configured for switchport */
	PortConfigModeT portMode = ((PhysicalPortT*)dataObj)->portMode;
	if(portMode != SWITCH_PORT){
		pifCmiOut(0, "Interface not configured for switchport");
		return PIF_CMI_RETURN_ERROR;
	}

	/* get current port state */
	SwitchPortT* switchport = &(((PhysicalPortT*)dataObj)->switchPort);
	VlanPortModeT vMode =  switchport->vlanMode;
	Uint32T accessVid = switchport->accessVid;
	Uint32T trunkVlanAllowedMode = switchport->trunkVlanAllowedMode;

	/* check port is already configured as same vlan mode */
	if(vMode == vlanMode){
		pifCmiOut(0, "vlan mode already set as %s", getVlanModeStr(vlanMode));
		return PIF_CMI_RETURN_ERROR;
	}

	/* if change from VLAN_MODE_ACCESS to VLAN_MODE_TRUNK */
	/* need to check access vid is already, if then should clear first */
	if((vMode == VLAN_MODE_ACCESS) && (accessVid != PIF_DEFAULT_VID)){
		pifCmiOut(0, "access vlan %d clear first", accessVid);
		return PIF_CMI_RETURN_ERROR;
	}

	/* if change from VLAN_MODE_TRUNK to VLAN_MODE_ACCESS */
	/* need to check trunk vid is already set, if then should clear first */
	if((vMode == VLAN_MODE_TRUNK) && (trunkVlanAllowedMode != PIF_VLAN_ALLOWED_NOT_CONFIG)){
		pifCmiOut(0, "trunk vlan clear first");
		return PIF_CMI_RETURN_ERROR;
	}

	/* request to set vlan mode */
	if(vlanMode == VLAN_MODE_ACCESS) {
		pifPortMgrSetVlanAccess((PhysicalPortT*)dataObj);
	}
	else if(vlanMode == VLAN_MODE_TRUNK) {
		pifPortMgrSetVlanTrunk((PhysicalPortT*)dataObj);
	}

	pifLOG(LOG_DEBUG, "PIF-CMI :: end \n");
	return PIF_CMI_RETURN_OK;
}


/* interface eth0 
 * 		switchport access  (default vid 1)
 * 		switchport access vlan 10 
 * 		*/
Int32T pifCmiVlanAccessVidAdd(Uint32T vid)
{
	Uint32T ifType  = CMI.type;
	void*   dataObj = CMI.obj;

	/* vlan port is allowed only for Ethernet & port-channel */
	if((ifType != TYPE_ETHERNET) && (ifType != TYPE_AGGREGATE)){
		pifCmiOut(0, "Interface not allowed for vlan port");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check port is configured for switchport */
	PortConfigModeT portMode = ((PhysicalPortT*)dataObj)->portMode;
	if(portMode != SWITCH_PORT){
		pifCmiOut(0, "Interface not configured for switchport");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check switchport is configured as access */
	VlanPortModeT vlanMode = ((PhysicalPortT*)dataObj)->switchPort.vlanMode;
	if(vlanMode != VLAN_MODE_ACCESS){
		pifCmiOut(0, "switchport not configured as access");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check whether configuring vid is valid */
	if(pifVlanMgrCheckValid(vid) != PIF_RTN_OK){
		pifCmiOut(0, "vlan %d not configured or invalid", vid);
		return PIF_CMI_RETURN_ERROR;
	}

	/* request manager to configure port with vid  */
	if(ifType == TYPE_ETHERNET){
		pifPortMgrAddAccessVid(dataObj, vid);
	}
	else if(ifType == TYPE_AGGREGATE){
		//pifAggMgrAddAccessVid(dataObj, vid);
	}

	pifLOG(LOG_DEBUG, "PIF-CMI :: end \n");
	return PIF_CMI_RETURN_OK;
}


/* interface eth0 
 * 		no switchport access vlan */
/* set to default VID of 1 */
Int32T pifCmiVlanAccessVidDel()
{
	Uint32T ifType  = CMI.type;
	void*   dataObj = CMI.obj;

	/* vlan port is allowed only for Ethernet & port-channel */
	if((ifType != TYPE_ETHERNET) && (ifType != TYPE_AGGREGATE)){
		pifCmiOut(0, "Interface not allowed for vlan port");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check port is configured for switchport */
	PortConfigModeT portMode = ((PhysicalPortT*)dataObj)->portMode;
	if(portMode != SWITCH_PORT){
		pifCmiOut(0, "Interface not configured for switchport");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check switchport is configured as access */
	VlanPortModeT vlanMode = ((PhysicalPortT*)dataObj)->switchPort.vlanMode;
	if(vlanMode != VLAN_MODE_ACCESS){
		pifCmiOut(0, "switchport not configured as access");
		return PIF_CMI_RETURN_ERROR;
	}

	/* request manager to configure port with vid  */
	if(ifType == TYPE_ETHERNET){
		pifPortMgrDeleteAccessVid(dataObj);
	}
	else if(ifType == TYPE_AGGREGATE){
		//pifAggMgrAddAccessVid(dataObj, vid);
	}

	pifLOG(LOG_DEBUG, "PIF-CMI :: end \n");
	return PIF_CMI_RETURN_OK;
}


/* interface eth0 
 * 		switchport trunk allowed vlan all 
 * 		switchport trunk allowed vlan none 
 * 		switchport trunk allowed vlan add VLAN_ID 
 * 		*/
Int32T pifCmiVlanTrunkVidAdd(Uint32T vid)
{
	Uint32T ifType  = CMI.type;
	void*   dataObj = CMI.obj;

	/* vlan port is allowed only for Ethernet & port-channel */
	if((ifType != TYPE_ETHERNET) && (ifType != TYPE_AGGREGATE)){
		pifCmiOut(0, "Interface not allowed for vlan port");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check port is configured for switchport */
	PortConfigModeT portMode = ((PhysicalPortT*)dataObj)->portMode;
	if(portMode != SWITCH_PORT){
		pifCmiOut(0, "Interface not configured for switchport");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check switchport is configured as trunk */
	VlanPortModeT vlanMode = ((PhysicalPortT*)dataObj)->switchPort.vlanMode;
	if(vlanMode != VLAN_MODE_TRUNK){
		pifCmiOut(0, "switchport not configured as trunk");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check whether configuring vid is valid */
	if(pifVlanMgrCheckValid(vid) != PIF_RTN_OK){
		pifCmiOut(0, "vlan %d not configured or invalid", vid);
		return PIF_CMI_RETURN_ERROR;
	}

	/* request manager to configure port with vid  */
	if(ifType == TYPE_ETHERNET){
		PhysicalPortT* port = (PhysicalPortT*)dataObj;
		Uint32T allowedMode = port->switchPort.trunkVlanAllowedMode;
		if(vid == NOS_ALL_VID) {
			if(allowedMode == PIF_VLAN_ALLOWED_ALL){
				pifCmiOut(0, "port already configured as all vlan");
				return PIF_CMI_RETURN_ERROR;
			}
			if(allowedMode != PIF_VLAN_ALLOWED_NOT_CONFIG){
				pifCmiOut(0, "port configured as all vlan, clear first");
				return PIF_CMI_RETURN_ERROR;
			}
			/* request to configure */
			pifPortMgrAddTrunkVidAll(dataObj);
		}
		else if(vid == NOS_NONE_VID){
			if(allowedMode == PIF_VLAN_ALLOWED_NONE){
				pifCmiOut(0, "port already configured as none vlan");
				return PIF_CMI_RETURN_ERROR;
			}
			if(allowedMode != PIF_VLAN_ALLOWED_NOT_CONFIG){
				pifCmiOut(0, "port configured as none vlan, clear first");
				return PIF_CMI_RETURN_ERROR;
			}
			/* request to configure */
			pifPortMgrAddTrunkVidNone(dataObj);
		}
		else { 
			if((allowedMode == PIF_VLAN_ALLOWED_ALL) || 
			   (allowedMode == PIF_VLAN_ALLOWED_NONE)) {
				pifCmiOut(0, "port configured with specific vlan, clear first");
				return PIF_CMI_RETURN_ERROR;
			}
			pifPortMgrAddTrunkVid(dataObj, vid);
		}
	}/* end of TYPE_ETHERNET */

	else if(ifType == TYPE_AGGREGATE){
		//pifAggMgrAddTrunkVid(dataObj, vid);
	}

	pifLOG(LOG_DEBUG, "PIF-CMI :: end \n");
	return PIF_CMI_RETURN_OK;
}



/* interface eth0 
 * 		switchport trunk allowed vlan remove VLAN_ID 
 * 		*/
Int32T pifCmiVlanTrunkVidDel(Uint32T vid)
{
	Uint32T ifType  = CMI.type;
	void*   dataObj = CMI.obj;

	/* vlan port is allowed only for Ethernet & port-channel */
	if((ifType != TYPE_ETHERNET) && (ifType != TYPE_AGGREGATE)){
		pifCmiOut(0, "Interface not allowed for vlan port");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check port is configured for switchport */
	PortConfigModeT portMode = ((PhysicalPortT*)dataObj)->portMode;
	if(portMode != SWITCH_PORT){
		pifCmiOut(0, "Interface not configured for switchport");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check switchport is configured as trunk */
	VlanPortModeT vlanMode = ((PhysicalPortT*)dataObj)->switchPort.vlanMode;
	if(vlanMode != VLAN_MODE_TRUNK){
		pifCmiOut(0, "switchport not configured as trunk");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check whether configuring vid is valid */
	if(pifVlanMgrCheckValid(vid) != PIF_RTN_OK){
		pifCmiOut(0, "vlan %d not configured or invalid", vid);
		return PIF_CMI_RETURN_ERROR;
	}

	/* check trunk vlan allowed mode */
	/* this is only PIF_VLAN_ALLOWED_CONFIG case */
	PhysicalPortT* port = (PhysicalPortT*)dataObj;
	Uint32T allowedMode = port->switchPort.trunkVlanAllowedMode;
	if(allowedMode == PIF_VLAN_ALLOWED_ALL) {
			pifCmiOut(0, "port configured as all vlan, clear first");
			return PIF_CMI_RETURN_ERROR;
	}
	else if(allowedMode == PIF_VLAN_ALLOWED_ALL) {
			pifCmiOut(0, "port configured as none vlan, clear first");
			return PIF_CMI_RETURN_ERROR;
	}

	/* request manager to configure port with vid  */
	if(ifType == TYPE_ETHERNET){
		pifPortMgrDeleteTrunkVid(dataObj, vid);
	}
	else if(ifType == TYPE_AGGREGATE){
		//pifAggMgrAddTrunkVid(dataObj, vid);
	}

	pifLOG(LOG_DEBUG, "PIF-CMI :: end \n");
	return PIF_CMI_RETURN_OK;
}



/* interface eth0 
 * 		switchport trunk allowed vlan exept VLAN_ID 
 * 		*/

Int32T pifCmiVlanTrunkVidExept(Uint32T vid)
{
	pifLOG(LOG_DEBUG, "PIF-CMI :: enter\n");
	return PIF_CMI_RETURN_OK;
}


/* interface eth0 
 * 		no switchport trunk 
 * 		*/
Int32T pifCmiVlanTrunkClear()
{
	Uint32T ifType  = CMI.type;
	void*   dataObj = CMI.obj;

	/* vlan port is allowed only for Ethernet & port-channel */
	if((ifType != TYPE_ETHERNET) && (ifType != TYPE_AGGREGATE)){
		pifCmiOut(0, "Interface not allowed for vlan port");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check port is configured for switchport */
	PortConfigModeT portMode = ((PhysicalPortT*)dataObj)->portMode;
	if(portMode != SWITCH_PORT){
		pifCmiOut(0, "Interface not configured for switchport");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check switchport is configured as trunk */
	VlanPortModeT vlanMode = ((PhysicalPortT*)dataObj)->switchPort.vlanMode;
	if(vlanMode != VLAN_MODE_TRUNK){
		pifCmiOut(0, "switchport not configured as trunk");
		return PIF_CMI_RETURN_ERROR;
	}

	/* request manager to clear trunk port  */
	if(ifType == TYPE_ETHERNET){
		pifPortMgrClearTrunk(dataObj);
	}
	else if(ifType == TYPE_AGGREGATE){
		//pifAggMgrAddTrunkVid(dataObj);
	}

	pifLOG(LOG_DEBUG, "PIF-CMI :: end \n");
	return PIF_CMI_RETURN_OK;
}

/* interface eth0 
 * 		no switchport access 
 * 		*/
Int32T pifCmiVlanAccessClear()
{
	Uint32T ifType  = CMI.type;
	void*   dataObj = CMI.obj;

	/* vlan port is allowed only for Ethernet & port-channel */
	if((ifType != TYPE_ETHERNET) && (ifType != TYPE_AGGREGATE)){
		pifCmiOut(0, "Interface not allowed for vlan port");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check port is configured for switchport */
	PortConfigModeT portMode = ((PhysicalPortT*)dataObj)->portMode;
	if(portMode != SWITCH_PORT){
		pifCmiOut(0, "Interface not configured for switchport");
		return PIF_CMI_RETURN_ERROR;
	}

	/* check switchport is configured as access */
	VlanPortModeT vlanMode = ((PhysicalPortT*)dataObj)->switchPort.vlanMode;
	if(vlanMode != VLAN_MODE_ACCESS){
		pifCmiOut(0, "switchport not configured as trunk");
		return PIF_CMI_RETURN_ERROR;
	}

	/* request manager to clear trunk port  */
	if(ifType == TYPE_ETHERNET){
		pifPortMgrClearAccess(dataObj);
	}
	else if(ifType == TYPE_AGGREGATE){
		//pifAggMgrAddTrunkVid(dataObj, vid);
	}

	pifLOG(LOG_DEBUG, "PIF-CMI :: end \n");
	return PIF_CMI_RETURN_OK;
}





/* interface eth0 
 * 		switchport trunk native vlan VLAN_ID
 * 		*/
Int32T pifCmiVlanTrunkNativeSet(Uint32T vid)
{
	pifLOG(LOG_DEBUG, "PIF-CMI :: end \n");
	return PIF_CMI_RETURN_OK;
}

/* interface eth0 
 * 		no switchport trunk native vlan
 * 		*/
Int32T pifCmiVlanTrunkNativeDel()
{
	pifLOG(LOG_DEBUG, "PIF-CMI :: end \n");
	return PIF_CMI_RETURN_OK;
}




/* vlan database 
 * 		vlan VLAN_ID name WORD mtu INT
 * 		*/
Int32T pifCmiVlanDatabaseAdd(Uint32T vid, StringT name, Uint32T mtu)
{
	pifLOG(LOG_DEBUG, "PIF-CMI :: enter \n");

	/* check vid already exist */
	if(pifVlanMgrGetVlan(vid) != NULL){
		pifVlanMgrAddUpdate(vid, name, mtu);
	}
	else {
		pifVlanMgrAddVlan(vid, name, mtu);
	}

	return PIF_CMI_RETURN_OK;
}


/* vlan database 
 * 		vlan VLAN_ID name WORD mtu INT
 * 		*/
Int32T pifCmiVlanDatabaseUpdate(Uint32T vid, StringT name, Uint32T mtu)
{
	pifLOG(LOG_DEBUG, "PIF-CMI :: enter \n");

	/* check vid not exist */
	if(pifVlanMgrGetVlan(vid) == NULL){
		pifCmiOut(0, "vlan %d not exist", vid);
		return PIF_CMI_RETURN_ERROR;
	}

	/* request to update vlan */
	pifVlanMgrAddVlan(vid, name, mtu);

	return PIF_CMI_RETURN_OK;
}


/* vlan database 
 * 		no vlan VLAN_ID
 * 		*/
Int32T pifCmiVlanDatabaseDel(Uint32T vid)
{
	pifLOG(LOG_DEBUG, "PIF-CMI :: enter \n");

	/* check vid not exist */
	VlanT* vlan = pifVlanMgrGetVlan(vid);
	if(vlan == NULL){
		pifCmiOut(0, "vlan %d not exist", vid);
		return PIF_CMI_RETURN_ERROR;
	}

	/* check any vid port exist */
	if(nnListCount(vlan->portList) != 0){
		ListNodeT * listNode;
		PhysicalPortT *pPort;

		Int8T strBuf[1000], *buf;
		buf = strBuf;
		Int32T errFlag = 0;
		PLIST_LOOP(vlan->portList, pPort, listNode){

			/* if associated port is trunk mode and allowed all 
			 * then we will delete the trunk port from vid,
			 * else we prevent delete target vlan */ 
			SwitchPortT* switchPort = getSwitchPort(pPort);

			/* check associate port is trunk mode and allowed all */
			if((switchPort->vlanMode == VLAN_MODE_TRUNK) && 
			   (switchPort->trunkVlanAllowedMode == PIF_VLAN_ALLOWED_ALL)) 
			{
				/* vlanMgr will delete later */
				/* pifVlanMgrDeleteTrunkPortMember(pPort, vid); */
			}
			else {
				buf += sprintf(buf, " %s", pPort->name );
				errFlag = 1;
			}
		}

		if(errFlag){
			pifCmiOut(0, "fail, vlan %d has port. %s", vid, strBuf);
			return PIF_CMI_RETURN_ERROR;
		}
	}

	/* request to add vid */
	pifVlanMgrDeleteVlan(vid);

	return PIF_CMI_RETURN_OK;
}



/***********************************************************
 *
 * CLI: show 
 *
 ***********************************************************/

void pifCmiCliIfShow(void* msgBuff) 
{ 
}



/***********************************************************
 *
 * CLI: Delayed 
 *
 ***********************************************************/
/* 2013.05.13 */
/* CLI: MAC */
void pifCmiMacAgingSet(void *msg){}
void pifCmiMacDynamicClearByAddr(void *msg){}
void pifCmiMacDynamicClearByPort(void *msg){}
void pifCmiMacDynamicClearByVlan(void *msg){}
void pifCmiMacStaticAddrAdd(void *msg){}
void pifCmiMacStaticAddrDel(void *msg){}

/* CLI: MAC ACL */
void pifCmiMacAclTermAdd(void *msg){}
void pifCmiMacAclTermDel(void *msg){}
void pifCmiMacAclPortAdd(void *msg){}

/* CLI: SHOW */
/* CLI: Interface */
void pifCmiIfStatusGet(void* msg){}
void pifCmiIfStatusDetailGet(void* msg){}
void pifCmiIfSwportStatusGet(void* msg){}

/* CLI: MAC */
void pifCmiMacTableGetAddr(void* msg){}
void pifCmiMacTableGetAging(void* msg){}
void pifCmiMacTableGetCount(void* msg){}
void pifCmiMacTableGetAll(void* msg){}
void pifCmiMacTableGetPort(void* msg){}
void pifCmiMacTableGetVlan(void* msg){}
void pifCmiMacTableGetStatic(void* msg){}

