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
#include <stdio.h>

#include "nosLib.h"
#include "pif.h"
#include "nnMemmgr.h"
#include "pifDataTypes.h"
#include "pifPortMgr.h"
#include "pifInterfaceMgr.h"
#include "pifVlanMgr.h"

#include "pifFeaApi.h"


static PortTableT* portTable = NULL;
static PortDatabaseT* portDB = NULL;

#define PORTDB     (*portDB)
#define PORTTREE   (portTable->tree)


/*************************************************************
 *
 *        PIF Port Table (Key: Iid)
 *
 *************************************************************/
PortTableT* createPortTable()
{
	portTable = NNMALLOC(MEM_IF, sizeof(PortTableT));
	portTable->tree = nnAvlInitLong(MEM_PORT);
	return portTable;
}

PortTableT* restartPortTable(PortTableT* table)
{
	portTable = table;
	return portTable;
}

void removePortTable() { nnAvlFreeLong(PORTTREE); }

void clearPortTable() { nnAvlDeleteAllNodeLong(PORTTREE); }

Int32T getSizePortTable() { return nnAvlCountLong(PORTTREE); }

AvlNodeLongT* getRootPortTable() { return ((PORTTREE)->pRoot); }

void insertPortTable(Int64T key, PhysicalPortT* pData)
{
	if(PORTTREE == NULL) {
		pifLOG(LOG_DEBUG, "Fail, PortTable Null \n");
		return;
	}

	if(nnAvlAddNodeLong(PORTTREE, key, (void*)pData) != SUCCESS){
		pifLOG(LOG_DEBUG, "Fail, PortTable insert (%s)", pData->name);
	};
}

void deletePortTable(Uint64T key) { nnAvlRemoveNodeLong(PORTTREE, key); }

PhysicalPortT* findPortTable(Uint64T key) { 
	if(PORTTREE == NULL) {
		pifLOG(LOG_DEBUG, "Fail, PortTable Null \n");
		return NULL;
	}
	return nnAvlLookupDataLong(PORTTREE, key);
}

static PhysicalPortT* rltPort = NULL;

int inspectPortName(AvlNodeLongT* treeNode, StringT name)
{
	if(treeNode == NULL) return 0;
	PhysicalPortT* currPort = (PhysicalPortT*)treeNode->pData;
	if(!strcmp(name, currPort->name)) { /* if find match */
		rltPort = currPort;
		return 1;
	}

	if(inspectPortName(treeNode->pLeft, name)) return 1;;
	if(inspectPortName(treeNode->pRight, name)) return 1;
	return 0;
}

PhysicalPortT* findNamePortTable(StringT name)
{
	rltPort = NULL;
	AvlNodeLongT *treeRoot = PORTTREE->pRoot;

	inspectPortName(treeRoot, name);

	return rltPort;
}


int inspectPortIfidx(AvlNodeLongT* treeNode, Uint32T idx)
{
	if(treeNode == NULL) return 0;
	PhysicalPortT* currPort = (PhysicalPortT*)treeNode->pData;
	if(currPort->ifIndex == idx) { /* if find match */
		rltPort = currPort;
		return 1;
	}

	if(inspectPortIfidx(treeNode->pLeft, idx)) return 1;;
	if(inspectPortIfidx(treeNode->pRight, idx)) return 1;
	return 0;
}

PhysicalPortT* findIfIdxPortTable(Uint32T idx)
{
	rltPort = NULL;
	AvlNodeLongT *treeRoot = PORTTREE->pRoot;

	inspectPortIfidx(treeRoot, idx);

	return rltPort;
}





void printPort(PhysicalPortT* port)
{
	pifLOG(LOG_DEBUG, "%s \n", port->name);
	pifLOG(LOG_DEBUG, "\tIid=%d, Ifidx=%d Bw=%d \n", 
						port->iid.idx, port->ifIndex, port->bandwidth);
	pifLOG(LOG_DEBUG, "\tAdmin=%s, Oper=%s \n", 
						getIfStateStr(port->adminState), getIfStateStr(port->operState));
	pifLOG(LOG_DEBUG, "\tencap=%d, speed=%d duplex=%d, flowctrl=0\n",
			 			port->encapType, port->speed, port->duplex);
	pifLOG(LOG_DEBUG, "\tHwType=%s, HwAddr=%s \n", 
						getHwTypeStr(port->hwType), mac2str((char*)port->hwAddr));
}

void printPortTable(AvlNodeLongT* treeNode)
{
	if(treeNode == NULL) return;
	printPort((PhysicalPortT*)treeNode->pData);

	printPortTable(treeNode->pLeft);
	printPortTable(treeNode->pRight);
}

void printPortAll()
{
	 pifLOG(LOG_DEBUG, "IPIF-MGR :: Dump All Port(%d) \n", (Int32T)getSizePortTable());

	 AvlNodeLongT *treeRoot = PORTTREE->pRoot;
	 printPortTable(treeRoot);
}


/*************************************************************
 *
 *        PIF Port Manager
 *
 *************************************************************/

/*************************************************************
 * Port Manager start/restart/stop
 *************************************************************/
PortDatabaseT* pifPortMgrCreate()
{
	/* initialize PortDatabase */
	portDB    = NNMALLOC(MEM_IF, sizeof(PortDatabaseT));
	PORTDB.defaultPortSpeed   = PORT_SPEED_AUTO;
	PORTDB.defaultDuplex      = PORT_DUPLEX_AUTO;
	PORTDB.defaultVid         = NOS_DEFAULT_VID;
	PORTDB.defaultPortMode    = SWITCH_PORT;
	PORTDB.defaultFlowControl.send 	  = FLOWCTL_DESIRED;
	PORTDB.defaultFlowControl.receive = FLOWCTL_DESIRED;

	PORTDB.pifPortTable = createPortTable();

	/* scan interface */
	// linux interfae scan or H/W interface scan by HAL
	
	return portDB;
}

PortDatabaseT* pifPortMgrRestart(PortDatabaseT* db)
{
	portDB = db;
	PORTDB.pifPortTable = restartPortTable(PORTDB.pifPortTable);
	return portDB;
}

void pifPortMgrInitialize()
{
}

/*************************************************************
 * Event From FEA Interface
 *************************************************************/
Int32T pifPortMgrAddPort(PhysicalPortT* port)
{
	pifLOG(LOG_DEBUG, "PORT-MGR:: %s\n", port->name);

	/* insert new port into table */
	/* notify L2 event for new port */

	/* get interface id */
	Uint32T iid = port->iid.idx;

	/* check port already exist */
	PhysicalPortT* p = findPortTable(iid);
	if(p != NULL) {
		pifLOG(LOG_DEBUG, "Port(%s) already exist\n", p->name);
		return PIF_RTN_ALREADY_EXIST;
	}

	/* insert new port */
	insertPortTable(iid, port);

	/* for debug */
	pifLOG(LOG_DEBUG, "port table size (%d)\n", (Int32T)getSizePortTable());

	/* dump debug */
	dumpPhysicalPort(port, "create");

	if(port->portMode == SWITCH_PORT)
	{
		/* set port mode as switchport */
		pifPortMgrSetPortMode(port, SWITCH_PORT);

		/* post event to L2 protocols */
		pifPostEventPort(port, PIF_EVENT_LINK_ADD);
	}

	pifLOG(LOG_DEBUG, "end\n");

	return PIF_RTN_OK;
}


Int32T pifPortMgrDelPort(StringT ifName, Uint32T ifIndex, Uint32T iid)
{
	pifLOG(LOG_DEBUG, "PORT-MGR :: Delete port (name: %s, iid: %d) \n", 
						ifName, iid);

	/* process Port delete event */
	/* notify L2 event for port deletion */

	/* find the target Port */
	PhysicalPortT* port = findPortTable(iid); 
	if(port == NULL) {
    	pifLOG(LOG_DEBUG, "Port(%s) not exist", ifName);
    	return PIF_RTN_NOT_EXIST;
	}

	/* debug current state */
	printAdminOperState((void*)port);

	/* dump debug */
	dumpPhysicalPort(port, "delete");

	if(port->portMode == SWITCH_PORT)
	{
		/* set port mode as routedport */
		pifPortMgrSetPortMode(port, ROUTED_PORT);

		pifPortMgrClearSwitchPort(port);
	}
	else if(port->portMode == ROUTED_PORT)
	{
		pifPortMgrClearRoutedPort(port);
	}

	/* delete port from table */
	deletePortTable(iid);
	freePhysicalPort(port);

	return PIF_RTN_OK;
}



Int32T pifPortMgrUpPort(StringT ifName, Uint32T ifIndex, Uint32T iid)
{
	pifLOG(LOG_DEBUG, "PORT-MGR :: Up port (name: %s, iid: %d) \n", 
						ifName, iid);

	/* process Port bring up event */
	/* check AdminState & Operstat, and post Port Event */

	/* find the target Port */
	PhysicalPortT* port = findPortTable(iid); 
	if(port == NULL) {
    	pifLOG(LOG_DEBUG, "Port(%s) not exist", ifName);
    	return PIF_RTN_NOT_EXIST;
	}

	/* debug current state */
	printAdminOperState((void*)port);

	/* check AdminState & OperState */
	if(port->operState == STATE_UP){
    	pifLOG(LOG_DEBUG, "Operstate is already up");
    	return PIF_RTN_BAD_ATTR;
	}

	/* set Operstate as UP */
	port->operState = STATE_UP;

	/* dump debug */
	dumpPhysicalPort(port, "up");

	/* post event to L2 protocols */
	pifPostEventPort(port, PIF_EVENT_LINK_UP);

	return PIF_RTN_OK;
}


Int32T pifPortMgrDownPort(StringT ifName, Uint32T ifIndex, Uint32T iid)
{
	pifLOG(LOG_DEBUG, "PORT-MGR :: Down port  (name: %s, iid: %d) \n", 
						ifName, iid);

	/* process Port down event */
	/* check AdminState & Operstat, and post Port Event */

	/* find the target Port */
	PhysicalPortT* port = findPortTable(iid); 
	if(port == NULL) {
    	pifLOG(LOG_DEBUG, "Port(%s) not exist \n", ifName);
    	return PIF_RTN_NOT_EXIST;
	}

	/* debug current state */
	printAdminOperState((void*)port);

	/* check AdminState & OperState */
	if(port->operState == STATE_DOWN){
    	pifLOG(LOG_DEBUG, "Operstate is already down");
    	return PIF_RTN_BAD_ATTR;
	}

	/* set Operstate as Down */
	port->operState = STATE_DOWN;

	/* dump debug */
	dumpPhysicalPort(port, "down");

	/* post event to L2 protocols */
	pifPostEventPort(port, PIF_EVENT_LINK_DOWN);

	return PIF_RTN_OK;
}



/*************************************************************
 * API of Port Manager
 *************************************************************/

Int32T pifPortMgrCheckPort(Uint32T iid)
{
	pifLOG(LOG_DEBUG, "PORT-MGR enter(%s) \n", getIidStr(iid));

	if(findPortTable(iid)) {
		return PIF_RTN_OK;
	}
	else {
		return PIF_RTN_NOT_EXIST;
	}
}


PhysicalPortT* pifPortMgrGetPort(Uint32T iid)
{
	pifLOG(LOG_DEBUG, "PORT-MGR enter \n");

	PhysicalPortT* port = findPortTable(iid);
	if(port == NULL){
		pifLOG(LOG_DEBUG, "port iid %d not exist \n", iid);
	}

	return port;
}


IPInterfaceT* pifPortMgrGetSubInterface(Uint32T iid)
{
	/* find Sub-Interface 
	 * if not exist, create new Sub-Interface 
	 * if exist return OK  */
	pifLOG(LOG_DEBUG, "PORT-MGR enter \n");

	PhysicalPortT* port = findPortTable(iid);
	if(port == NULL){
		pifLOG(LOG_DEBUG, "port iid %d not exist \n", iid);
		return NULL;
	}

	IPInterfaceT* conIf = port->routedPort.connectedIpIf;
	if(conIf == NULL){
		/* need to change to make new local IPInterface */
		pifLOG(LOG_DEBUG, "connected IPInterface is NULL \n");
		return NULL;
	}

	return conIf;
}



void pifPortMgrSetPortMode(PhysicalPortT* port, PortConfigModeT mode)
{
	pifLOG(LOG_DEBUG, "------------------------------------------\n");
	pifLOG(LOG_DEBUG, "Port(%s) Port Mode: %s --> %s\n",
						port->name,
			 			getIfModeStr(port->portMode), getIfModeStr(mode));
	pifLOG(LOG_DEBUG, "------------------------------------------\n");

	/* set port mode */
	port->portMode = mode;

	/* set FEA as SWITCH_PORT */
	/* currently, null function not supported in l2forwarder */
	feaApiInterfacePortType(port->name, port->ifIndex, port->portMode);

	/* add/del port to bridge */
	if(port->portMode == SWITCH_PORT){
		feaApiBridgeAddPort(port->name, port->ifIndex);
		//pifPortMgrSetPortState(port, 0, PORT_STATE_FORWARDING);
	}
	else if(port->portMode == ROUTED_PORT){
		feaApiBridgeDeletePort(port->name, port->ifIndex);
	}
}

void pifPortMgrSetPortFwdState(PhysicalPortT* port, Uint32T instance, PortStateT state)
{
	pifLOG(LOG_DEBUG, "------------------------------------------\n");
	pifLOG(LOG_DEBUG, "Port(%s) Port Mode: %s --> %s\n",
						port->name,
			 			getPortStateStr(port->switchPort.fwdState), getPortStateStr(state));
	pifLOG(LOG_DEBUG, "------------------------------------------\n");

	/* set state */
	port->switchPort.fwdState = state;

	/* apply to fea */
	feaApiMacTableClearByPort(port->ifIndex);
	feaApiBridgeSetPortState(port->name, port->ifIndex, instance, state);
}




/* set port admin state up */
#include <linux/if.h>
Int32T pifPortMgrEnablePort(PhysicalPortT* port)
{
	pifLOG(LOG_DEBUG, "PORT-MGR enter %s\n", port->name);

	/* check adminstate */
	if(port->adminState == ADMIN_UP){
		return PIF_RTN_ALREADY_SET;
	}
	port->adminState = ADMIN_UP;

	/* set connectedIpIf's adminstate as up */
	pifLOG(LOG_DEBUG, "enable ip interface state %s\n", port->name);
	IPInterfaceT* conIf = port->routedPort.connectedIpIf;
	conIf->adminState = ADMIN_UP; 

	/* set FEA adminstate as up */
	pifLOG(LOG_DEBUG, "enable fea interface state %s\n", port->name);
	feaApiInterfaceUp(port->name, port->ifIndex, port->iid.idx, conIf->flags);

	/* set flags */
	Uint32T flags = IFF_UP;
	conIf->flags |= flags;

	return PIF_RTN_OK;
}


/* set port admin state down */
Int32T pifPortMgrDisablePort(PhysicalPortT* port)
{
	pifLOG(LOG_DEBUG, "PORT-MGR enter %s \n", port->name);

	/* check adminstate */
	if(port->adminState == ADMIN_DOWN){
		return PIF_RTN_ALREADY_SET;
	}
	port->adminState = ADMIN_DOWN;

	/* set connectedIpIf's adminstate as down */
	pifLOG(LOG_DEBUG, "disable ip interface state %s\n", port->name);
	IPInterfaceT* conIf = port->routedPort.connectedIpIf;
	conIf->adminState = ADMIN_DOWN; 

	/* set FEA adminstate as up */
	pifLOG(LOG_DEBUG, "disable fea interface state %s\n", port->name);
	feaApiInterfaceDown(port->name, port->ifIndex, port->iid.idx, conIf->flags);

	/* set flags */
	Uint32T flags = IFF_UP;
	conIf->flags &= ~flags;

	return PIF_RTN_OK;
}


/* set port as layer2 switchport */
Int32T pifPortMgrSetSwitchPort(PhysicalPortT* port)
{
	pifLOG(LOG_DEBUG, "PORT-MGR (%s) \n", port->name);

	/* check previous port state */
	PortConfigModeT previousMode = port->portMode;
	if(previousMode == NOT_CONFIGURED){
		/* set interface as routed interface */
	}
	else if(previousMode == SWITCH_PORT){
		/* currently, set port routed port state */
		/* do not return */
	}
	else if(previousMode == ROUTED_PORT){
		pifPortMgrClearRoutedPort(port);
	}

	/* set port mode */
	pifPortMgrSetPortMode(port, SWITCH_PORT);

	/* set as default access port */
	if((previousMode == NOT_CONFIGURED) || (previousMode == ROUTED_PORT)){
		pifPortMgrSetVlanAccess(port);
	}

	/* set port fwd state as FORWARDING */
	pifPortMgrSetPortFwdState(port, 0, PORT_STATE_FORWARDING);

	/* post event to L2 protocols */
	pifPostEventPort(port, PIF_EVENT_LINK_ADD);

	/* notify interface manager to set IPInterface */
	IPInterfaceT* conIf = port->routedPort.connectedIpIf;
	pifInterfaceMgrSetSwitchPort(conIf);

	return PIF_RTN_OK;
}


/* clear switchport */
Int32T pifPortMgrClearSwitchPort(PhysicalPortT* port)
{
	pifLOG(LOG_DEBUG, "PORT-MGR enter \n");

	/* get switchport data */
	SwitchPortT* switchport = getSwitchPort(port);
	VlanPortModeT vlanMode = switchport->vlanMode;

	if(vlanMode == VLAN_MODE_ACCESS){
		pifPortMgrClearAccess(port);
	}
	else if(vlanMode == VLAN_MODE_TRUNK){
		pifPortMgrClearTrunk(port);
	}

	/* post event to L2 protocols */
	pifPostEventPort(port, PIF_EVENT_LINK_DEL);

	/* notify vlan manager */
	// pifVlanMgrDownPort(port);

	return PIF_RTN_OK;
}



/* set port as layer3 routedport */
Int32T pifPortMgrSetRoutedPort(PhysicalPortT* port)
{
	pifLOG(LOG_DEBUG, "PORT-MGR enter \n");

	/* check previous port state */
	PortConfigModeT previousMode = port->portMode;
	if(previousMode == NOT_CONFIGURED){
		/* set interface as routed interface */
	}
	else if(previousMode == SWITCH_PORT){
		pifPortMgrClearSwitchPort(port);
	}
	else if(previousMode == ROUTED_PORT){
		/* currently, set port routed port state */
		/* do not return */
	}

	/* set port mode */
	pifPortMgrSetPortMode(port, ROUTED_PORT);

	/* post event to L2 protocols */
	pifPostEventPort(port, PIF_EVENT_LINK_DEL);

	/* notify interface manager to set IPInterface */
	IPInterfaceT* conIf = port->routedPort.connectedIpIf;
	pifInterfaceMgrSetRoutedPort(conIf);

	/* notify vlan manager */
	// pifVlanMgrDownPort(port);

	return PIF_RTN_OK;
}


/* clear routed port */
Int32T pifPortMgrClearRoutedPort(PhysicalPortT* port)
{
	pifLOG(LOG_DEBUG, "PORT-MGR enter \n");

	/* currently nothing to be done */

	return PIF_RTN_OK;
}






/* set port speed */
Int32T pifPortMgrSetSpeed(PhysicalPortT* port, PortSpeedT speed)
{
	pifLOG(LOG_DEBUG, "PORT-MGR enter \n");

	return PIF_RTN_OK;
}

/* set port duplex */
Int32T pifPortMgrSetDuplex(PhysicalPortT* port, PortDuplexT speed)
{
	pifLOG(LOG_DEBUG, "PORT-MGR enter \n");

	return PIF_RTN_OK;
}

/* set port flow control */
Int32T pifPortMgrSetFlow(PhysicalPortT* port, Int8T dir, PortFlowCtrlT ctrl)
{
	pifLOG(LOG_DEBUG, "PORT-MGR enter \n");

	return PIF_RTN_OK;
}

/* set port vlan access mode */
Int32T pifPortMgrSetVlanAccess(PhysicalPortT* port) 
{
	pifLOG(LOG_DEBUG, "PORT-MGR enter \n");

	/* get current switchport configuation */
	SwitchPortT* switchPort = &(port->switchPort);

	/* check if current mode is trunk, if then, remove trunk mode */
	if(switchPort->vlanMode == VLAN_MODE_TRUNK){
		// nsm_bridge_delete_all_fdb_by_port(ifp->ifindex)
		// nsm_vlan_clear_trunk_port(port);
	}

	/* set switchport mode as access */
	switchPort->vlanMode = VLAN_MODE_ACCESS;

	/* notify vlanManager to config port mode as access  */
	pifVlanMgrSetPortMode(port, VLAN_MODE_ACCESS);

	/* set switchport as default vlan id 
	 * belows need to be checked */

	/* notify vlanManager to add the access port in default vlan  */
	/* first, check if vlan is already set as default vlan id */
	if(switchPort->accessVid == PIF_NONE_VID){
		pifLOG(LOG_DEBUG, "set port vide as default 1. \n");
		switchPort->accessVid = PIF_DEFAULT_VID;
		pifVlanMgrAddAccessPortMember((void*)port, PIF_DEFAULT_VID);
	}

	return PIF_RTN_OK;
}


/* set port vlan trunk mode */
Int32T pifPortMgrSetVlanTrunk(PhysicalPortT* port) 
{
	pifLOG(LOG_DEBUG, "PORT-MGR enter \n");

	/* get current switchport configuation */
	SwitchPortT* switchPort = &(port->switchPort);

	/* check if current mode is access, if then, remove access mode */
	if(switchPort->vlanMode == VLAN_MODE_ACCESS){
		pifPortMgrClearAccess(port);
	}

	/* set switchport mode as access */
	switchPort->vlanMode = VLAN_MODE_TRUNK;

	/* notify vlanManager to config port mode as trunk  */
	pifVlanMgrSetPortMode(port, VLAN_MODE_TRUNK);

	/* notify vlanManager to attach port to default vlan */
	pifVlanMgrAddTrunkPortMember(port, pifVlanMgrGetDefaultVid());
	return PIF_RTN_OK;
}


/* set switchport access vid */
Int32T pifPortMgrAddAccessVid(PhysicalPortT* port, Uint32T vid)
{
	pifLOG(LOG_DEBUG, "PORT-MGR enter \n");

	/* get current switchport configuation */
	SwitchPortT* switchPort = &(port->switchPort);

	/* in case of changing access vid, delete previous vid first */
	if((switchPort->accessVid != PIF_NONE_VID) && 
	   (switchPort->accessVid != vid)) {
		pifPortMgrDeleteAccessVid(port);
	}

	/* now, set access vid */
	switchPort->accessVid = vid;
	  
	/* notify vlanManager to attach port to vlan */
	return pifVlanMgrAddAccessPortMember(port, vid);
}


/* set switchport access vid */
Int32T pifPortMgrDeleteAccessVid(PhysicalPortT* port)
{
	pifLOG(LOG_DEBUG, "PORT-MGR enter \n");

	/* get current switchport configuation */
	SwitchPortT* switchPort = &(port->switchPort);
	Uint32T vid = switchPort->accessVid;

	/* check if clear default vid */
	if(vid == PIF_NONE_VID) return PIF_RTN_OK;

	/* set access vid as default */
	switchPort->accessVid = PIF_NONE_VID;
	  
	/* notify vlanManager to detach port from vlan */
	return pifVlanMgrDeleteAccessPortMember(port, vid);
}


/* set switchport trunk all vid */
Int32T pifPortMgrAddTrunkVidAll(PhysicalPortT* port)
{
	pifLOG(LOG_DEBUG, "PORT-MGR enter \n");

	/* get current switchport configuation */
	SwitchPortT* switchPort = &(port->switchPort);

	/* now, set port as trunk allowed vlan all */
	switchPort->trunkVlanAllowedMode = PIF_VLAN_ALLOWED_ALL;
	switchPort->trunkNativeVid = pifVlanMgrGetDefaultVid();
	  
	/* notify vlanManager to attach port to vlan */
	pifVlanMgrAddTrunkPortMemberAll(port);

	return PIF_RTN_OK;
}


/* set switchport trunk none vid */
Int32T pifPortMgrAddTrunkVidNone(PhysicalPortT* port)
{
	pifLOG(LOG_DEBUG, "PORT-MGR enter \n");

	/* get current switchport configuation */
	SwitchPortT* switchPort = &(port->switchPort);

	/* now, set port as trunk allowed vlan all */
	switchPort->trunkVlanAllowedMode = PIF_VLAN_ALLOWED_NONE;
	switchPort->trunkNativeVid = pifVlanMgrGetDefaultVid();
	  
	/* notify vlanManager to detach port from all vlan */
	pifVlanMgrDeleteTrunkPortMemberAll(port);

	return PIF_RTN_OK;
}


/* add a vid to switchport trunk */
Int32T pifPortMgrAddTrunkVid(PhysicalPortT* port, Uint32T vid)
{
	pifLOG(LOG_DEBUG, "PORT-MGR enter \n");

	/* get current switchport configuation */
	SwitchPortT* switchPort = &(port->switchPort);

	if(switchPort->trunkVlanAllowedMode == PIF_VLAN_ALLOWED_NOT_CONFIG) {
		/* set port as trunk allowed vlan configured */
		switchPort->trunkVlanAllowedMode = PIF_VLAN_ALLOWED_CONFIG;
		switchPort->trunkNativeVid = pifVlanMgrGetDefaultVid();
	}

	/* check the port is already member of incoming vid */
	if(switchPort->trunkAllowedVid[vid] == PIF_VLAN_ALLOWED_FLAGE_ENABLE){
		pifLOG(LOG_DEBUG, "vid % is already enabled \n", vid);
		return PIF_RTN_OK;
	}
	  
	/* notify vlanManager to attach port to vlan */
	pifVlanMgrAddTrunkPortMember(port, vid);

	return PIF_RTN_OK;
}


/* delete a vid from switchport trunk */
Int32T pifPortMgrDeleteTrunkVid(PhysicalPortT* port, Uint32T vid)
{
	pifLOG(LOG_DEBUG, "PORT-MGR enter \n");

	/* get current switchport configuation */
	SwitchPortT* switchPort = &(port->switchPort);

	/* check the port is not a member of incoming vid */
	if(switchPort->trunkAllowedVid[vid] == PIF_VLAN_ALLOWED_FLAGE_DISABLE){
		pifLOG(LOG_DEBUG, "vid % is not enabled \n", vid);
		return PIF_RTN_OK;
	}
	  
	/* just notify vlanManager to detach port from vlan */
	pifVlanMgrDeleteTrunkPortMember(port, vid);

	return PIF_RTN_OK;
}


/* clear switchport trunk */
Int32T pifPortMgrClearTrunk(PhysicalPortT* port)
{
	pifLOG(LOG_DEBUG, "PORT-MGR enter \n");

	/* remove all attached vid first */
	pifVlanMgrDeleteTrunkPortMemberAll(port);

	/* set switchport as default */
	SwitchPortT* switchPort = &(port->switchPort);
	switchPort->vlanMode = VLAN_MODE_ACCESS;
	switchPort->accessVid = PIF_DEFAULT_VID;
	switchPort->trunkVlanAllowedMode = PIF_VLAN_ALLOWED_NOT_CONFIG;

	/* notify vlanManager to config port mode as default access  */
	pifVlanMgrSetPortMode(port, VLAN_MODE_ACCESS);

	return PIF_RTN_OK;
}



void pifPortMgrRecursiveVidAdd(AvlNodeLongT* treeNode, Uint32T vid)
{
	/* recursive preorder */
	if(treeNode == NULL) return;

	/**********************************************/
	/* process each port                          */
	/**********************************************/
	/* get port */
	PhysicalPortT* port = (PhysicalPortT*)treeNode->pData;
	SwitchPortT* switchPort = getSwitchPort(port);

	/* check associate port is trunk mode and allowed all */
	if((switchPort->vlanMode == VLAN_MODE_TRUNK) && 
	   (switchPort->trunkVlanAllowedMode == PIF_VLAN_ALLOWED_ALL))
	{
		pifPortMgrAddTrunkVid(port, vid);
	}

	/**********************************************/
	/* end of procesing each port                 */
	/**********************************************/

	/* recursive preorder */
	pifPortMgrRecursiveVidAdd(treeNode->pLeft, vid);
	pifPortMgrRecursiveVidAdd(treeNode->pRight, vid);
}



/* apply allowed-all-trunk port */
Int32T pifPortMgrApplyTrunkVidAll(Uint32T vid)
{
	pifLOG(LOG_DEBUG, "PORT-MGR enter \n");

	AvlNodeLongT *treeRoot = getRootPortTable();
	pifPortMgrRecursiveVidAdd(treeRoot, vid);

	return PIF_RTN_OK;
}





/* clear switchport trunk */
Int32T pifPortMgrClearAccess(PhysicalPortT* port)
{
	pifLOG(LOG_DEBUG, "PORT-MGR enter \n");

	/* set switchport as default */
	SwitchPortT* switchPort = &(port->switchPort);

	/* remove attached vid first */
	if(switchPort->accessVid != PIF_NONE_VID) {
		pifPortMgrDeleteAccessVid(port);
	}

	/* set switchport as default */
	switchPort->vlanMode = VLAN_NOT_CONFIGURED;
	switchPort->trunkVlanAllowedMode = PIF_VLAN_ALLOWED_NOT_CONFIG;

	//feaApiMacTableClearByPort(port->ifIndex);

	return PIF_RTN_OK;
}









