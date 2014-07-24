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
#include <string.h>
#include <stdlib.h>

#include "pif.h"
#include "nosLib.h"
#include "pifFeaApi.h"
#include "pifPortMgr.h"
#include "pifInterfaceMgr.h"
#include "pifVlanMgr.h"

#include "pifFea.h"

/*************************************************************
 *
 * Event from FEA OS & H/W
 *
 ************************************************************/
void feaInit()
{
	pifLOG(LOG_DEBUG, "enter \n");

	/* init fea and read os kernel configuration */
	pif_fea_init();

	/* debug dump */
	dumpIPInterfaceTable();

	pifLOG(LOG_DEBUG, "end \n");
}

/*************************************************************
 *
 * Event from FEA OS & H/W
 *
 ************************************************************/
void feaEventPortAdd (PhysicalPortT* port)
{
	/* Notify Port Add Event to PortMgr */
	pifLOG(LOG_DEBUG, "enter \n");

	pifPortMgrAddPort(port);
}

void feaEventPortDelete (StringT ifName, Uint32T ifIndex, Uint32T iid)
{
	/* Notify Port Delete Event to PortMgr */
	pifLOG(LOG_DEBUG, "enter \n");

	pifPortMgrDelPort(ifName, ifIndex, iid);
}

void feaEventPortDown (StringT ifName, Uint32T ifIndex, Uint32T iid)
{
	/* Notify Port Down Event to PortMgr */
	pifLOG(LOG_DEBUG, "enter \n");

	pifPortMgrDownPort(ifName, ifIndex, iid);
}

void feaEventPortUp (StringT ifName, Uint32T ifIndex, Uint32T iid)
{
	/* Notify Port Up Event to PortMgr */
	pifLOG(LOG_DEBUG, "enter \n");

	pifPortMgrUpPort(ifName, ifIndex, iid);
}



void feaEventIPInterfaceAdd (IPInterfaceT* ipif)
{
	/* Notify IPInterface Add Event to IPInterfaceMgr */
	pifLOG(LOG_DEBUG, "enter \n");

	pifInterfaceMgrAddInterface(ipif);
}

void feaEventIPInterfaceDelete (StringT ifName, Uint32T ifIndex, Uint32T iid)
{
	/* Notify IPInterface Delete Event to IPInterfaceMgr */
	pifLOG(LOG_DEBUG, "enter \n");

	pifInterfaceMgrDelInterface(ifName, iid);
}

void feaEventIPInterfaceUp (StringT ifName, Uint32T ifIndex, Uint32T iid)
{
	/* Notify IPInterface Up Event to IPInterfaceMgr */
	pifLOG(LOG_DEBUG, "enter \n");

	pifInterfaceMgrUpInterface(ifName, ifIndex, iid);
}

void feaEventIPInterfaceDown (StringT ifName, Uint32T ifIndex, Uint32T iid)
{
	/* Notify IPInterface Down Event to IPInterfaceMgr */
	pifLOG(LOG_DEBUG, "enter \n");

	pifInterfaceMgrDownInterface(ifName, ifIndex, iid);
}


void feaEventIPAddressAdd (StringT ifName, Uint32T iid, ConnectedAddressT *conAddr)
{
	pifLOG(LOG_DEBUG, "enter \n");

	pifInterfaceMgrAddAddress(ifName, iid, conAddr);
}


void feaEventIPAddressDelete (StringT ifName, Uint32T iid, PrefixT *addr)
{
	pifLOG(LOG_DEBUG, "enter \n");

	pifInterfaceMgrDelAddress(ifName, iid, addr);
}


/*************************************************************
 *
 * FEA control API (dependent on OS/H/W)
 *
 ************************************************************/
#include "pal.h"

/* Interface Configuration API */

/*************************************************************
 * Interface Up/Down
 ************************************************************/
void feaApiInterfaceUp (StringT ifName, Uint32T ifIndex, Uint32T iid, Uint32T flags)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "IfName: %s, IfIdx:%d, iid:%08x, flags:%08x \n", 
							ifName, ifIndex, iid, flags);

	/* make os parameters */
	struct interface ifp;
	memset(&ifp, 0, sizeof(struct interface));

	/* set name */
	strncpy(ifp.name, ifName, strlen(ifName));
	ifp.ifIndex = ifIndex;

	/* set flags */
	ifp.flags = flags;

	pif_fea_if_flags_set(&ifp, IFF_UP);

	pifLOG(LOG_DEBUG, "exit \n");
}


void feaApiInterfaceDown (StringT ifName, Uint32T ifIndex, Uint32T iid, Uint32T flags)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "IfName: %s, IfIdx:%d, iid:%08x, flags:%08x \n", 
							ifName, ifIndex, iid, flags);

	/* make os parameters */
	struct interface ifp;
	memset(&ifp, 0, sizeof(struct interface));

	/* set name */
	strncpy(ifp.name, ifName, strlen(ifName));
	ifp.ifIndex = ifIndex;

	/* set flags */
	ifp.flags = flags;

	pif_fea_if_flags_unset(&ifp, IFF_UP);

	pifLOG(LOG_DEBUG, "exit \n");
}


/*************************************************************
 * Interface Address
 ************************************************************/
void feaApiInterfaceAddressAdd (StringT ifName, Uint32T ifIndex, ConnectedAddressT* conAddr)
{
	Int8T strAddr[IF_NAME_SIZE];
	nnCnvPrefixtoString(strAddr, &(conAddr->address));
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "IfName: %s, IfIdx:%d, addr:%s\n", ifName, ifIndex, strAddr);

	/* make os parameters */
	struct interface ifp;
	struct connected ifc;
	memset(&ifp, 0, sizeof(struct interface));
	memset(&ifc, 0, sizeof(struct connected));

	strncpy(ifp.name, ifName, strlen(ifName));
	ifp.ifIndex = ifIndex;
	ifc.address = &(conAddr->address);
	ifc.destination = &(conAddr->broadcast);

	pif_fea_if_ipv4_address_add(&ifp, &ifc);

	pifLOG(LOG_DEBUG, "exit \n");
}


void feaApiInterfaceAddressDel (StringT ifName, Uint32T ifIndex, ConnectedAddressT* conAddr)
{
	Int8T strAddr[IF_NAME_SIZE];
	nnCnvPrefixtoString(strAddr, &(conAddr->address));
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "IfName: %s, IfIdx:%d, addr:%s\n", ifName, ifIndex, strAddr);

	/* make os parameters */
	struct interface ifp;
	struct connected ifc;
	memset(&ifp, 0, sizeof(struct interface));
	memset(&ifc, 0, sizeof(struct connected));

	strncpy(ifp.name, ifName, strlen(ifName));
	ifp.ifIndex = ifIndex;
	ifc.address = &(conAddr->address);
	ifc.destination = &(conAddr->broadcast);

	pif_fea_if_ipv4_address_delete(&ifp, &ifc);

	pifLOG(LOG_DEBUG, "exit \n");
}



/*************************************************************
 * Interface Port Type (switchport/routedport)
 ************************************************************/
void feaApiInterfacePortType (StringT ifName, Uint32T ifIndex, Uint32T portType)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "IfName: %s, IfIdx:%d, type:%d\n", 
							ifName, ifIndex, portType);

	struct interface ifp;
	strncpy(ifp.name, ifName, strlen(ifName));
	ifp.ifIndex = ifIndex;

	pif_fea_if_set_port_type(&ifp, portType);

	pifLOG(LOG_DEBUG, "exit \n");
}


/*************************************************************
 * Interface Vlan Mode (access/trunk)
 ************************************************************/
/* NSM filter info. need to change */
#define NSM_VLAN_ENABLE_INGRESS_FILTER        (1 << 0)
#define NSM_VLAN_ACCEPTABLE_FRAME_TYPE_TAGGED (1 << 1)`


void feaApiVlanAdd (Uint32T vid)
{
    pifLOG(LOG_DEBUG, "enter \n");
    pifLOG(LOG_DEBUG, "vid: %d\n", vid);
    
    /* set bridge name */
    Int8T brName[IF_NAME_SIZE];
    sprintf(brName, "%d", pifVlanMgrGetDefaultBridge());
    
    pif_fea_vlan_add (brName, (Uint16T)vid);
    
    pifLOG(LOG_DEBUG, "exit \n");
}   


void feaApiVlanDelete (Uint32T vid)
{
    pifLOG(LOG_DEBUG, "enter \n");
    pifLOG(LOG_DEBUG, "vid: %d\n", vid);
    
    /* set bridge name */
    Int8T brName[IF_NAME_SIZE];
    sprintf(brName, "%d", pifVlanMgrGetDefaultBridge());

    pif_fea_vlan_delete (brName, (Uint16T)vid);

    pifLOG(LOG_DEBUG, "exit \n");
}


void feaApiVlanEnable (Uint32T vid)
{
    pifLOG(LOG_DEBUG, "enter \n");
    pifLOG(LOG_DEBUG, "vid: %d\n", vid);

    /* set bridge name */
    Int8T brName[IF_NAME_SIZE];
    sprintf(brName, "%d", pifVlanMgrGetDefaultBridge());

    pif_fea_vlan_enable (brName, (Uint16T)vid);

    pifLOG(LOG_DEBUG, "exit \n");
}


void feaApiVlanDisable (Uint32T vid)
{
    pifLOG(LOG_DEBUG, "enter \n");
    pifLOG(LOG_DEBUG, "vid: %d\n", vid);

    /* set bridge name */
    Int8T brName[IF_NAME_SIZE];
    sprintf(brName, "%d", pifVlanMgrGetDefaultBridge());

    pif_fea_vlan_disable (brName, (Uint16T)vid);

    pifLOG(LOG_DEBUG, "exit \n");
}


void feaApiVlanDefaultPVid (StringT ifName, Uint32T ifIndex, Uint32T pvid, Uint32T vlanMode)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "IfName: %s, IfIdx:%d, mode:%d, pvid:%d\n", 
							ifName, ifIndex, vlanMode, pvid);

	/* set bridge name */
	Int8T brName[IF_NAME_SIZE];
	sprintf(brName, "%d", pifVlanMgrGetDefaultBridge());
	
	/* set egress vlan tagging type */
	enum hal_vlan_egress_type egress = HAL_VLAN_EGRESS_UNTAGGED;
	if(vlanMode == VLAN_MODE_TRUNK)
		egress = HAL_VLAN_EGRESS_TAGGED;

	pif_fea_vlan_set_default_pvid (brName, ifIndex, pvid, egress);

	pifLOG(LOG_DEBUG, "exit \n");
}





void feaApiVlanPortMode (StringT ifName, Uint32T ifIndex, Uint32T vlanMode)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "IfName: %s, IfIdx:%d, mode:%d\n", 
							ifName, ifIndex, vlanMode);

	/* set bridge name */
	Int8T brName[IF_NAME_SIZE];
	sprintf(brName, "%d", pifVlanMgrGetDefaultBridge());
	
	/* set hal vlan-mode */
	enum hal_vlan_port_type port_type;
	if(vlanMode == VLAN_MODE_ACCESS) port_type = HAL_VLAN_ACCESS_PORT;
	else if(vlanMode == VLAN_MODE_TRUNK) port_type = HAL_VLAN_TRUNK_PORT;
	else return;

	/* set hal hal_vlan_acceptable_frame_type */
	enum hal_vlan_acceptable_frame_type acceptable_frame_types;
	if(vlanMode == VLAN_MODE_ACCESS) {
		acceptable_frame_types = HAL_VLAN_ACCEPTABLE_FRAME_TYPE_UNTAGGED;
	}
	else if(vlanMode == VLAN_MODE_TRUNK) {
		acceptable_frame_types = HAL_VLAN_ACCEPTABLE_FRAME_TYPE_ALL;
	}

	/* set enable_ingress_filter */
	unsigned short enable_ingress_filter;
	enable_ingress_filter = NSM_VLAN_ENABLE_INGRESS_FILTER;

	/* request to set l2 forwarder */
	pif_fea_vlan_set_port_type(brName, ifIndex, port_type, 
			                   acceptable_frame_types,
		                       enable_ingress_filter);

	pifLOG(LOG_DEBUG, "exit \n");
}


/*************************************************************
 * L2 Port Vid Add to SwitchPort
 ************************************************************/
void feaApiVlanAddToPort (StringT ifName, Uint32T ifIndex, Uint32T vid, Uint32T vlanMode)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "IfName: %s, IfIdx:%d, mode:%d, vid:%d\n", 
							ifName, ifIndex, vlanMode, vid);

	/* set bridge name */
	Int8T brName[IF_NAME_SIZE];
	sprintf(brName, "%d", pifVlanMgrGetDefaultBridge());
	
	/* set egress vlan tagging type */
	enum hal_vlan_egress_type egress = HAL_VLAN_EGRESS_UNTAGGED;
	if((vlanMode == VLAN_MODE_TRUNK) && (vid != pifVlanMgrGetDefaultVid()))
	{
		egress = HAL_VLAN_EGRESS_TAGGED;
	}

	pif_fea_vlan_add_vid_to_port (brName, ifIndex, vid, egress);

	pifLOG(LOG_DEBUG, "exit \n");
}

/*************************************************************
 * L2 Port Vid Delete from SwitchPort
 ************************************************************/

void feaApiVlanDeleteFromPort (StringT ifName, Uint32T ifIndex, Uint32T vid)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "IfName: %s, IfIdx:%d, vid:%d\n", ifName, ifIndex, vid);

	/* set bridge name */
	Int8T brName[IF_NAME_SIZE];
	sprintf(brName, "%d", pifVlanMgrGetDefaultBridge());

	pif_fea_vlan_delete_vid_from_port (brName, ifIndex, vid);

	pifLOG(LOG_DEBUG, "exit \n");
}



/*************************************************************
 *
 * FEA Bridge API
 *
 ************************************************************/

/*************************************************************
 * add bridge 
 ************************************************************/
void feaApiBridgeAdd(StringT brName, Uint32T isVlan, Uint32T protocol)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "brName=%s, isVlan=%d, protocol:%d \n", 
						brName, isVlan, protocol);

	pif_fea_bridge_add(brName, isVlan, protocol);

	pifLOG(LOG_DEBUG, "exit \n");
}

void feaApiBridgeDelete(StringT brName)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "brName=%s \n", brName);

	pif_fea_bridge_delete(brName);

	pifLOG(LOG_DEBUG, "exit \n");
}


void feaApiBridgeAddPort(StringT ifName, Uint32T ifIndex)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "ifName=%s , ifIndex=%d\n", ifName, ifIndex);

	/* set default bridge name */
	Int8T brName[IF_NAME_SIZE];
	sprintf(brName, "%s", PIF_HAL_BRIDGE_NAME);

	pif_fea_bridge_add_port(brName, ifIndex);

	pifLOG(LOG_DEBUG, "exit \n");
}


void feaApiBridgeDeletePort(StringT ifName, Uint32T ifIndex)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "ifName=%s , ifIndex=%d\n", ifName, ifIndex);

	/* set default bridge name */
	Int8T brName[IF_NAME_SIZE];
	sprintf(brName, "%s", PIF_HAL_BRIDGE_NAME);

	pif_fea_bridge_delete_port(brName, ifIndex);

	pifLOG(LOG_DEBUG, "exit \n");
}


void feaApiBridgeAddInstance(Uint32T instance)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "instance=%d\n", instance);

	/* currently null function */

	pifLOG(LOG_DEBUG, "exit \n");
}


void feaApiBridgeDeleteInstance(StringT ifName, Uint32T instance)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "instance=%d\n", instance);

	/* currently null function */

	pifLOG(LOG_DEBUG, "exit \n");
}



void feaApiBridgeAddVlanToInstance(Uint32T vid, Uint32T instance)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "vlan=%d , instance=%d\n", vid, instance);

	/* set default bridge name */
	Int8T brName[IF_NAME_SIZE];
	sprintf(brName, "%s", PIF_HAL_BRIDGE_NAME);

	pif_fea_bridge_add_vlan_to_instance(brName, instance, vid);

	pifLOG(LOG_DEBUG, "exit \n");
}


void feaApiBridgeDeleteVlanFromInstance(Uint32T vid, Uint32T instance)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "vlan=%d , instance=%d\n", vid, instance);

	/* set default bridge name */
	Int8T brName[IF_NAME_SIZE];
	sprintf(brName, "%s", PIF_HAL_BRIDGE_NAME);

	pif_fea_bridge_add_delete_from_instance(brName, instance, vid);

	pifLOG(LOG_DEBUG, "exit \n");
}


void feaApiBridgeSetPortState(StringT ifName, Uint32T ifIndex, Uint32T instance, Int32T state)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "ifName=%s , instance=%d, state=%d\n", 
							ifName, instance, state);

	Int32T learn, forward;

	/* set default bridge name */
	Int8T brName[IF_NAME_SIZE];
	sprintf(brName, "%s", PIF_HAL_BRIDGE_NAME);

	/* set port state */
	pif_fea_bridge_set_port_state(brName, ifIndex, instance, state);

	/* set learn and forward */
	learn = (state == HAL_BR_PORT_STATE_LEARNING ) || 
		    (state == HAL_BR_PORT_STATE_FORWARDING);

	forward = (state == HAL_BR_PORT_STATE_FORWARDING);

	pif_fea_bridge_set_learn_fwd(brName, ifIndex, instance, learn, forward);

	pifLOG(LOG_DEBUG, "exit \n");
}


void feaApiBridgeSetLearning(Int32T flag)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "isLearning=%d\n", flag);

	/* set default bridge name */
	Int8T brName[IF_NAME_SIZE];
	sprintf(brName, "%s", PIF_HAL_BRIDGE_NAME);

	/* set port state */
	pif_fea_bridge_set_set_learning(brName, flag);

	pifLOG(LOG_DEBUG, "exit \n");
}



/*************************************************************
 * mac address table
 ************************************************************/
void feaApiMacTableSetAgingTime(Uint32T timeVal)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "time=%d\n", timeVal);

	/* set default bridge name */
	Int8T brName[IF_NAME_SIZE];
	sprintf(brName, "%s", PIF_HAL_BRIDGE_NAME);

	pif_fea_bridge_set_ageing_time (brName, timeVal);

	pifLOG(LOG_DEBUG, "exit \n");
}





void feaApiMacTableAddStatic(StringT mac, Uint32T ifIndex, Uint32T vid)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "mac=%s ifidx=%d vid=%d\n", mac2str(mac), ifIndex, vid);

	/* set default bridge name */
	Int8T brName[IF_NAME_SIZE];
	sprintf(brName, "%s", PIF_HAL_BRIDGE_NAME);

	pif_fea_bridge_add_mac_static (brName, mac, (int)ifIndex, (unsigned short)vid);

	pifLOG(LOG_DEBUG, "exit \n");
}


void feaApiMacTableDeleteStatic(StringT mac, Uint32T ifIndex, Uint32T vid)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "mac=%s ifidx=%d vid=%d\n", mac2str(mac), ifIndex, vid);

	/* set default bridge name */
	Int8T brName[IF_NAME_SIZE];
	sprintf(brName, "%s", PIF_HAL_BRIDGE_NAME);

	pif_fea_bridge_del_mac_static (brName, mac, (int)ifIndex, (unsigned short)vid);

	pifLOG(LOG_DEBUG, "exit \n");
}


void feaApiMacTableAddDynamic(StringT mac, Uint32T ifIndex, Uint32T vid)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "mac=%s ifidx=%d vid=%d\n", mac2str(mac), ifIndex, vid);

	/* set default bridge name */
	Int8T brName[IF_NAME_SIZE];
	sprintf(brName, "%s", PIF_HAL_BRIDGE_NAME);

	pif_fea_bridge_add_mac_dynamic (brName, mac, (int)ifIndex, (unsigned short)vid);

	pifLOG(LOG_DEBUG, "exit \n");
}


void feaApiMacTableDeleteDynamic(StringT mac, Uint32T ifIndex, Uint32T vid)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "mac=%s ifidx=%d vid=%d\n", mac2str(mac), ifIndex, vid);

	/* set default bridge name */
	Int8T brName[IF_NAME_SIZE];
	sprintf(brName, "%s", PIF_HAL_BRIDGE_NAME);

	pif_fea_bridge_del_mac_dynamic (brName, mac, (int)ifIndex, (unsigned short)vid);

	pifLOG(LOG_DEBUG, "exit \n");
}




void feaApiMacTableClearByPort(Uint32T ifIndex)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "ifidx=%d\n", ifIndex);

	/* set default bridge name */
	Int8T brName[IF_NAME_SIZE];
	sprintf(brName, "%s", PIF_HAL_BRIDGE_NAME);

	/* set default bridge name */
	//Uint16T vid = PIF_DEFAULT_VID;
	Uint16T vid = 0;

	pif_fea_bridge_clear_mac_dynamic (brName, (int)ifIndex, (unsigned short)vid);

	pifLOG(LOG_DEBUG, "exit \n");
}

void feaApiMacTableClearByVlan(Uint32T vid)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "vid=%d\n", vid);

	/* set default bridge name */
	Int8T brName[IF_NAME_SIZE];
	sprintf(brName, "%s", PIF_HAL_BRIDGE_NAME);

	/* set default ifindex */
	Uint32T ifIndex = 0;

	pif_fea_bridge_clear_mac_dynamic (brName, (int)ifIndex, (unsigned short)vid);

	pifLOG(LOG_DEBUG, "exit \n");
}

void feaApiMacTableClearByMac(StringT mac)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "mac=%s\n", mac2str(mac));

	/* set default bridge name */
	Int8T brName[IF_NAME_SIZE];
	sprintf(brName, "%s", PIF_HAL_BRIDGE_NAME);

	pif_fea_bridge_clear_mac_dynamic_by_mac (brName, mac);

	pifLOG(LOG_DEBUG, "exit \n");
}









/*************************************************************
 *
 * FEA LACP API
 *
 ************************************************************/

/*************************************************************
 * LACP Aggregator Add
 ************************************************************/
void feaApiLacpAddAggregator(StringT aggIfName, StringT macAddr, PortConfigModeT mode)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "aggName=%s, aggMac=%s, aggType:%s \n", 
						aggIfName, mac2str(macAddr), getIfModeStr(mode));

	/* set aggregator name */
	Int8T aggName[IF_NAME_SIZE];
	sprintf(aggName, "%s", aggIfName);

	/* set mac address */
	Uint8T mac[ETH_ADDR_LEN];
	memcpy(mac, macAddr, ETH_ADDR_LEN);

	/* set agg port type, default is HAL_IF_TYPE_ETHERNET */
	int agg_type = 0;
	if(mode == ROUTED_PORT) agg_type = HAL_IF_TYPE_IP;
	else if(mode == SWITCH_PORT) agg_type = HAL_IF_TYPE_ETHERNET;
	else agg_type = HAL_IF_TYPE_ETHERNET;

	pif_fea_lacp_add_aggregator (aggName, mac, agg_type);

	pifLOG(LOG_DEBUG, "exit \n");
}


/*************************************************************
 * LACP Aggregator Delete
 ************************************************************/
void feaApiLacpDeleteAggregator(StringT aggIfName)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "aggName: %s \n", aggIfName);

	/* set aggregator name */
	Int8T aggName[IF_NAME_SIZE];
	sprintf(aggName, "%s", aggIfName);

	pif_fea_lacp_delete_aggregator (aggName);

	pifLOG(LOG_DEBUG, "exit \n");
}



/*************************************************************
 * LACP Aggregator Add Port
 ************************************************************/
void feaApiLacpAddPortToAggregator(StringT aggIfName, StringT ifName, Uint32T ifIndex)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "aggName: %s, portName: %s \n", aggIfName, ifName);

	/* set aggregator name */
	Int8T aggName[IF_NAME_SIZE];
	sprintf(aggName, "%s", aggIfName);

	/* set port name */
	Int8T portName[IF_NAME_SIZE];
	sprintf(portName, "%s", ifName);

	pif_fea_lacp_attach_mux_to_aggregator (aggName, portName, ifIndex);

	pifLOG(LOG_DEBUG, "exit \n");
}



/*************************************************************
 * LACP Aggregator Delete Port
 ************************************************************/
void feaApiLacpDeletePortToAggregator(StringT aggIfName, StringT ifName, Uint32T ifIndex)
{
	pifLOG(LOG_DEBUG, "enter \n");
	pifLOG(LOG_DEBUG, "aggName: %s, portName: %s \n", aggIfName, ifName);

	/* set aggregator name */
	Int8T aggName[IF_NAME_SIZE];
	sprintf(aggName, "%s", aggIfName);

	/* set port name */
	Int8T portName[IF_NAME_SIZE];
	sprintf(portName, "%s", ifName);

	pif_fea_lacp_detach_mux_from_aggregator (aggName, portName, ifIndex);

	pifLOG(LOG_DEBUG, "exit \n");
}









