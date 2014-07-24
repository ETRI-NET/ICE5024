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

#include "nnMemmgr.h"
#include "nosLib.h"
#include "pif.h"
#include "pifVlanMgr.h"
#include "pifMsgEvent.h"

#include "pifFeaApi.h"

static VlanTableT* vlanTable = NULL;

static VlanDatabaseT* vlanDB = NULL;

#define VLANDB     (*vlanDB)
#define VLANTREE   (vlanTable->tree)

/***********************************************************************
 *
 *                Vlan Table (Key: Vid)
 *
 * *********************************************************************/
VlanTableT* createVlanTable()
{
	vlanTable = NNMALLOC(MEM_IF, sizeof(VlanTableT));
	vlanTable->tree = nnAvlInitLong(MEM_IF);
	return vlanTable;
}

VlanTableT* restartVlanTable(VlanTableT* table)
{
	vlanTable = table;
	return vlanTable;
}

void removeVlanTable() { nnAvlFreeLong(VLANTREE); }

void clearVlanTable() { nnAvlRemoveAllNodeLong(VLANTREE); }

Int32T getSizeVlanTable() { return nnAvlCountLong(VLANTREE); }

AvlNodeLongT* getRootVlanTable() { return ((VLANTREE)->pRoot); }

void insertVlanTable(Uint64T key, VlanT* pData)
{
	if(nnAvlAddNodeLong(VLANTREE, key, (void*)pData) != SUCCESS){
		printf("Error !");
	}
}

void deleteVlanTable(Uint64T key) { nnAvlRemoveNodeLong(VLANTREE, key); }

/* preorder tree inspect */
static VlanT* rltVlan = NULL;
int inspectVlanTable(AvlNodeLongT* treeNode, StringT name)
{
	// inspect parent node
	if(treeNode == NULL) return 0;
	VlanT* currVlan = (VlanT*)treeNode->pData;
	if(!strcmp(name, currVlan->name)) { /* if find match */
		rltVlan = currVlan;
		return 1;
	}

	// inspect left child node
	if(inspectVlanTable(treeNode->pLeft, name)) return 1;;

	// inspect right child node
	if(inspectVlanTable(treeNode->pRight, name)) return 1;

	return 0;
}

VlanT* findVlanTable(Uint64T key) { return nnAvlLookupDataLong(VLANTREE, key); }

VlanT* findNameVlanTable(StringT name)
{
	rltVlan = NULL;
	AvlNodeLongT *treeRoot = VLANTREE->pRoot;

	inspectVlanTable(treeRoot, name);

	return rltVlan;
}

void printVlan(VlanT* port)
{
}

void printVlanTable(AvlNodeLongT* treeNode)
{
	if(treeNode == NULL) return;
	printVlan((VlanT*)treeNode->pData);

	printVlanTable(treeNode->pLeft);
	printVlanTable(treeNode->pRight);
}

/***********************************************************************
 *
 *                MAC Table (Key: mac)
 *
 **********************************************************************/

MacTableT* createMacTable()
{
	MacTableT* macTable = NNMALLOC(MEM_IF, sizeof(MacTableT));
	macTable->tree = nnAvlInitLong(MEM_IF);
	return macTable;
}

void removeMacTable(Uint32T vid) 
{ 
	MacTableT* table = VLANDB.pifMacTable[vid];
	nnAvlFreeLong(table->tree); 
	VLANDB.pifMacTable[vid] = NULL;
}

void clearMacTable(Uint32T vid) 
{ 
	MacTableT* table = VLANDB.pifMacTable[vid];
	nnAvlRemoveAllNodeLong(table->tree); 
}

Int32T getSizeMacTable(Uint32T vid) 
{ 
	MacTableT* table = VLANDB.pifMacTable[vid];
	return nnAvlCountLong(table->tree); 
}

AvlNodeLongT* getRootMacTable(Uint32T vid) 
{ 
	MacTableT* table = VLANDB.pifMacTable[vid];
	if(table == NULL) return NULL;

	return ((table->tree)->pRoot); 
}

void insertMacTable(Uint32T vid, Uint64T key, StaticMacT* pData)
{
	MacTableT* table = VLANDB.pifMacTable[vid];

	if(table == NULL) {
		table = createMacTable(vid);
		VLANDB.pifMacTable[vid] = table;
	}

	if(nnAvlAddNodeLong(table->tree, key, (void*)pData) != SUCCESS){
		printf("nnAvlAddNodeLong Error !");
	}
}

void deleteMacTable(Uint32T vid, Uint64T key) 
{ 
	MacTableT* table = VLANDB.pifMacTable[vid];
	if(table == NULL) return;

	nnAvlRemoveNodeLong(table->tree, key); 

	if(getSizeMacTable(vid) == 0) removeMacTable(vid);
}

StaticMacT* findMacTable(Uint32T vid, Uint64T key) 
{ 
	MacTableT* table = VLANDB.pifMacTable[vid];
	if(table == NULL) return NULL;
	return nnAvlLookupDataLong(table->tree, key); 
}

Int64T getMacKey(StringT mac)
{
	Int64T key = 0;
	memcpy(&key, mac, ETH_ADDR_LEN);
	return key;
}

Int32T getCurrentAgingTime()
{
	return VLANDB.currentAging;
}

void setCurrentAgingTime(Uint32T agingTime)
{
	VLANDB.currentAging = agingTime;
}


Int32T getDefaultAgingTime()
{
	return VLANDB.defaultAging;
}



/***********************************************************************
 *
 *                       Vlan Manager 
 *
 **********************************************************************/

/***********************************************************************
 * Vlan Manager start/restart/stop
 **********************************************************************/

VlanDatabaseT* pifVlanMgrCreate()
{
	/* initialize VlanDatabase */
	vlanDB    = NNMALLOC(MEM_IF, sizeof(VlanDatabaseT));
	memset(vlanDB, 0, sizeof(VlanDatabaseT));

	VLANDB.defaultVid    = PIF_DEFAULT_VID;
	VLANDB.defaultMetric = PIF_DEFAULT_METRIC;
	VLANDB.defaultAging  = PIF_DEFAULT_AGING_TIME;
	VLANDB.currentAging  = PIF_DEFAULT_AGING_TIME;
	VLANDB.defaultBridge = PIF_DEFAULT_BRIDGE;
	sprintf(&VLANDB.defaultBridgeName[0], "%d", PIF_DEFAULT_BRIDGE);

	/* Init Vlan Table */
	VLANDB.pifVlanTable = createVlanTable();

	/* Init Mac Table for Default Vlan */
	VLANDB.pifMacTable[PIF_DEFAULT_VID] = createMacTable();

	return vlanDB;
}

VlanDatabaseT* pifVlanMgrRestart(VlanDatabaseT* db)
{
	vlanDB = db;
	VLANDB.pifVlanTable = restartVlanTable(VLANDB.pifVlanTable);
	return vlanDB;
}

void pifVlanMgrInitialize()
{
	/* create default vlan */
	pifVlanMgrAddVlan(PIF_DEFAULT_VID, "default", 1500); 
}

/***********************************************************************
 * Vlan Manager create/delete vlan
 **********************************************************************/
extern Int32T pifPortMgrApplyTrunkVidAll(Uint32T vid);

Int32T pifVlanMgrAddVlan(Uint32T vid, StringT name, Uint32T mtu)
{
	pifLOG(LOG_DEBUG, "VLAN-MGR:: %d\n", vid);
	/* create and insert new port into table */
	/* notify L2 event for new vlan */

	/* check vlan already exist */
	if(findVlanTable(vid) != NULL){
		 pifLOG(LOG_DEBUG, "Vlan(%d) already exist", vid);
		 return PIF_RTN_ALREADY_EXIST;
	}

	/* create new vlan */
	VlanT* vlan = newVlan();
	vlan->vid = vid;

	if(name != NULL){ 
		sprintf(vlan->name, "%s", name);
	}
	else {
		sprintf(vlan->name, "%s", "none");
	}

	if(mtu != 0) {
		vlan->mtu = mtu;
	}
	else {
		vlan->mtu = PIF_DEFAULT_MTU;
	}

	vlan->adminState = ADMIN_UP;
	vlan->operState = STATE_UP;
	vlan->connectedIpIf = NULL;

	/* insert new vlan into fea */
	feaApiVlanAdd(vid);
	feaApiVlanEnable(vid);

	/* insert new vlan */
	insertVlanTable(vid, vlan);

	/* post event to L2 protocols */
	pifPostEventVlan(vlan, NULL, PIF_EVENT_VLAN_ADD);

	/* notify portManager to config allowed-all-trunk port */
	pifPortMgrApplyTrunkVidAll(vid);

	return PIF_RTN_OK;
}


Int32T pifVlanMgrAddUpdate(Uint32T vid, StringT name, Uint32T mtu)
{
	pifLOG(LOG_DEBUG, "VLAN-MGR:: %d\n", vid);

	/* check vlan exist */
	VlanT* vlan = findVlanTable(vid);
	if(vlan == NULL){
		 pifLOG(LOG_DEBUG, "Vlan(%d) not exist", vid);
		 return PIF_RTN_NOT_EXIST;
	}

	/* update name */
	if(name != NULL) sprintf(vlan->name, "%s", name);

	/* update name */
	if(mtu != 0) vlan->mtu = mtu;

	return PIF_RTN_OK;
}


Int32T pifVlanMgrDeleteVlan(Uint32T vid)
{
	pifLOG(LOG_DEBUG, "VLAN-MGR:: vid=%d\n", vid);
	/* delete vlan from table */
	/* notify L2 event for new vlan */

	/* check vlan exist */
	VlanT* vlan = findVlanTable(vid);
	if(vlan == NULL) {
		 pifLOG(LOG_DEBUG, "Vlan(%d) not exist\n", vid);
		 return PIF_RTN_NOT_EXIST;
	}

	/* delete associated allowed-all-trunk trunk port */ 
	Int32T portCount = nnListCount(vlan->portList);
	if(portCount != 0)
	{
		pifLOG(LOG_DEBUG, "Vlan(%d) has %d associated port \n", vid, portCount);

		ListNodeT *listNode;
		PhysicalPortT *pPort;

		PLIST_LOOP(vlan->portList, pPort, listNode)
		{
			pifLOG(LOG_DEBUG, "Vlan(%d) check port(%s) \n", vid, pPort->name);

			SwitchPortT* switchPort = getSwitchPort(pPort);
			if((switchPort->vlanMode == VLAN_MODE_TRUNK) && 
				(switchPort->trunkVlanAllowedMode == PIF_VLAN_ALLOWED_ALL))
			{
				pifLOG(LOG_DEBUG, "delete allowed-all-trunk (%s) \n", pPort->name);

				/* delete vid from switchport of FEA switch  */
				feaApiVlanDeleteFromPort (pPort->name, pPort->ifIndex, vid);

				/* set switchport allowed vid flag index */
				switchPort->trunkAllowedVid[vid] = PIF_VLAN_ALLOWED_FLAGE_DISABLE;

				/* port vlan event to L2 protocols */
				pifPostEventVlan(vlan, pPort, PIF_EVENT_VLAN_PORT_DEL);
			}

		} /*end of LOOP */
	}

	/* post event to L2 protocols */
	pifPostEventVlan(vlan, NULL, PIF_EVENT_VLAN_DEL);

	/* remove vlan */
	deleteVlanTable(vid);
	freeVlan(vlan);

	pifLOG(LOG_DEBUG, "VLAN-MGR:: end\n");
	return PIF_RTN_OK;
}


/*************************************************************
 * API of Vlan Manager
 ************************************************************/
Uint32T pifVlanMgrGetDefaultBridge()
{
	return VLANDB.defaultBridge;
}

StringT pifVlanMgrGetDefaultBridgeName()
{
	return (&(VLANDB.defaultBridgeName[0]));
}

Uint32T pifVlanMgrGetDefaultVid()
{
	return VLANDB.defaultVid;
}


VlanT* pifVlanMgrGetVlan(Uint32T vid)
{
	pifLOG(LOG_DEBUG, "VLAN-MGR enter \n");

	VlanT* vlan = findVlanTable(vid);
	if(vlan == NULL){
		 pifLOG(LOG_DEBUG, "Vlan(%d) not exist\n", vid);
		 return NULL;
	}

	return vlan;
}


Int32T pifVlanMgrCheckValid(Uint32T vid)
{
	pifLOG(LOG_DEBUG, "VLAN-MGR enter (vid=%d) \n", vid);
	
	/* check vlan all or none */
	if( (vid == NOS_ALL_VID) || (vid == NOS_NONE_VID)){
		return PIF_RTN_OK;
	}

	/* check vlan exist */
	VlanT* vlan = findVlanTable(vid);
	if(vlan == NULL){
		 pifLOG(LOG_DEBUG, "Vlan(%d) not exist \n", vid);
		 return PIF_RTN_NOT_EXIST;
	}

	/* check other attributes */

	return PIF_RTN_OK;
}


Int32T pifVlanMgrSetPortMode(void* port, VlanPortModeT vlanMode)
{
	pifLOG(LOG_DEBUG, "VLAN-MGR enter \n");

	/* just set FEA & notify L2 vlan event */
	PhysicalPortT* commonPort = (PhysicalPortT*)port;

	/* set switchport mode of FEA switch as access  */
	feaApiVlanPortMode(commonPort->name, 
					   commonPort->ifIndex, vlanMode);

	/* port vlan event to L2 protocols */
	VlanT* vlan = NULL;
	pifPostEventVlan(vlan, port, PIF_EVENT_VLAN_PORT_MODE);

	return PIF_RTN_OK;
}


Int32T pifVlanMgrAddAccessPortMember(void* port, Uint32T vid)
{
	pifLOG(LOG_DEBUG, "VLAN-MGR enter \n");

	/* check vlan exist */
	VlanT* vlan = findVlanTable(vid);
	if(vlan == NULL){
		 pifLOG(LOG_DEBUG, "Vlan(%d) not exist \n", vid);
		 return PIF_RTN_NOT_EXIST;
	}

	/* get port */
	PhysicalPortT* commonPort = (PhysicalPortT*)port;

	/* add port to vlan */
	nnListAddNode(vlan->portList, commonPort);

	/* add vid to switchport of FEA switch  */
	feaApiVlanAddToPort (commonPort->name, commonPort->ifIndex,	
						vid, VLAN_MODE_ACCESS);

	/* set default pvid of FEA switch  */
	feaApiVlanDefaultPVid (commonPort->name, commonPort->ifIndex,	
						vid, VLAN_MODE_ACCESS);

	/* port vlan event to L2 protocols */
	pifPostEventVlan(vlan, commonPort, PIF_EVENT_VLAN_PORT_ADD);

	return PIF_RTN_OK;
}

Int32T pifVlanMgrDeleteAccessPortMember(void* port, Uint32T vid)
{
	pifLOG(LOG_DEBUG, "VLAN-MGR enter \n");

	/* check vlan exist */
	VlanT* vlan = findVlanTable(vid);
	if(vlan == NULL){
		 pifLOG(LOG_DEBUG, "Vlan(%d) not exist \n", vid);
		 return PIF_RTN_NOT_EXIST;
	}

	/* get port */
	PhysicalPortT* commonPort = (PhysicalPortT*)port;

	pifLOG(LOG_DEBUG, "delete if(%s) form vid(%d) \n", commonPort->name, vid);

	/* delete port from vlan */
	nnListRemoveNode(vlan->portList, commonPort);

	/* delete vid from switchport of FEA switch  */
	feaApiVlanDeleteFromPort (commonPort->name, commonPort->ifIndex, vid);

	/* port vlan event to L2 protocols */
	pifPostEventVlan(vlan, commonPort, PIF_EVENT_VLAN_PORT_DEL);

	return PIF_RTN_OK;
}

Int32T pifVlanMgrAddTrunkPortMember(void* port, Uint32T vid)
{
	pifLOG(log_debug, "VLAN-MGR enter \n");

	/* check vlan exist */
	VlanT* vlan = findVlanTable(vid);
	if(vlan == NULL){
		 pifLOG(LOG_DEBUG, "Vlan(%d) not exist \n", vid);
		 return PIF_RTN_NOT_EXIST;
	}

	/* get port */
	PhysicalPortT* commonPort = (PhysicalPortT*)port;

	/* add port to vlan */
	nnListAddNode(vlan->portList, commonPort);

	/* add vid to switchport of FEA switch  */
	feaApiVlanAddToPort (commonPort->name, commonPort->ifIndex,	
						vid, VLAN_MODE_TRUNK);

	/* set switchport allowed vid flag index */
	SwitchPortT* switchPort = &(commonPort->switchPort);
	switchPort->trunkAllowedVid[vid] = PIF_VLAN_ALLOWED_FLAGE_ENABLE;

	/* port vlan event to L2 protocols */
	pifPostEventVlan(vlan, commonPort, PIF_EVENT_VLAN_PORT_ADD);

	return PIF_RTN_OK;
}

Int32T pifVlanMgrDeleteTrunkPortMember(void* port, Uint32T vid)
{
	pifLOG(LOG_DEBUG, "VLAN-MGR enter \n");

	/* check vlan exist */
	VlanT* vlan = findVlanTable(vid);
	if(vlan == NULL){
		 pifLOG(LOG_DEBUG, "Vlan(%d) not exist \n", vid);
		 return PIF_RTN_NOT_EXIST;
	}

	/* get port */
	PhysicalPortT* commonPort = (PhysicalPortT*)port;

	/* delete port from vlan */
	nnListRemoveNode(vlan->portList, commonPort);

	/* delete vid from switchport of FEA switch  */
	feaApiVlanDeleteFromPort (commonPort->name, commonPort->ifIndex, vid);

	/* set switchport allowed vid flag index */
	SwitchPortT* switchPort = &(commonPort->switchPort);
	switchPort->trunkAllowedVid[vid] = PIF_VLAN_ALLOWED_FLAGE_DISABLE;

	/* port vlan event to L2 protocols */
	pifPostEventVlan(vlan, commonPort, PIF_EVENT_VLAN_PORT_DEL);

	return PIF_RTN_OK;
}

void pifVlanMgrRecursiveAdd(AvlNodeLongT* treeNode, PhysicalPortT* port)
{
	/* recursive preorder */
	if(treeNode == NULL) return;

	/**********************************************/
	/* process each vlan                          */
	/**********************************************/
	/* get vlan */
	VlanT* vlan = (VlanT*)treeNode->pData;
	Uint32T vid = vlan->vid;

	/* add port to vlan */
	nnListAddNode(vlan->portList, port);

	/* add vid to switchport of FEA switch  */
	feaApiVlanAddToPort (port->name, port->ifIndex,	vid, VLAN_MODE_TRUNK);

	/* port vlan event to L2 protocols */
	pifPostEventVlan(vlan, port, PIF_EVENT_VLAN_PORT_ADD);

	/* set switchport allowed vid flag index */
	SwitchPortT* switchPort = &(port->switchPort);
	switchPort->trunkAllowedVid[vid] = PIF_VLAN_ALLOWED_FLAGE_ENABLE;

	/**********************************************/
	/* end of procesing each vlan                 */
	/**********************************************/

	/* recursive preorder */
	pifVlanMgrRecursiveAdd(treeNode->pLeft, port);
	pifVlanMgrRecursiveAdd(treeNode->pRight, port);
}

Int32T pifVlanMgrAddTrunkPortMemberAll(void* port)
{
	AvlNodeLongT *treeRoot = getRootVlanTable();
	pifVlanMgrRecursiveAdd(treeRoot, (PhysicalPortT*)port);

	pifLOG(LOG_DEBUG, "vlan-mgr enter \n");
	return PIF_RTN_OK;
}




void pifVlanMgrRecursiveDelete(AvlNodeLongT* treeNode, PhysicalPortT* port)
{
	/* recursive preorder */
	if(treeNode == NULL) return;

	/**********************************************/
	/* process each vlan                          */
	/**********************************************/
	/* get vlan */
	VlanT* vlan = (VlanT*)treeNode->pData;
	Uint32T vid = vlan->vid;

	/* delete port from vlan */
	nnListRemoveNode(vlan->portList, port);

	/* delete vid from switchport of FEA switch  */
	feaApiVlanDeleteFromPort (port->name, port->ifIndex, vid);

	/* set switchport allowed vid flag index */
	SwitchPortT* switchPort = &(port->switchPort);
	switchPort->trunkAllowedVid[vid] = PIF_VLAN_ALLOWED_FLAGE_DISABLE;

	/* port vlan event to L2 protocols */
	pifPostEventVlan(vlan, port, PIF_EVENT_VLAN_PORT_DEL);

	/**********************************************/
	/* end of procesing each vlan                 */
	/**********************************************/

	/* recursive preorder */
	pifVlanMgrRecursiveDelete(treeNode->pLeft, port);
	pifVlanMgrRecursiveDelete(treeNode->pRight, port);
}


Int32T pifVlanMgrDeleteTrunkPortMemberAll(void* port)
{
	AvlNodeLongT *treeRoot = getRootVlanTable();
	pifVlanMgrRecursiveDelete(treeRoot, (PhysicalPortT*)port);

	pifLOG(LOG_DEBUG, "vlan-mgr enter \n");
	return PIF_RTN_OK;
}



/***********************************************************************
 * Vlan Manager add/delete static mac address
 **********************************************************************/

Int32T pifMacAddressTableSetAgingTime(Uint32T agingTime)
{
	pifLOG(LOG_DEBUG, "MAC-ADDRESS-TABLE enter \n");
	pifLOG(LOG_DEBUG, "aging-time=%d\n", agingTime);

	/* set current aging-time */
	setCurrentAgingTime(agingTime);

	/* apply to fea */
	feaApiMacTableSetAgingTime(agingTime);

	return PIF_RTN_OK;
}


Int32T pifMacAddressTableGetAgingTime()
{
	pifLOG(LOG_DEBUG, "MAC-ADDRESS-TABLE enter \n");
	pifLOG(LOG_DEBUG, "aging-time=%d\n", getCurrentAgingTime());

	return getCurrentAgingTime();
}



Int32T pifMacAddressTableDelAgingTime()
{
	pifLOG(LOG_DEBUG, "MAC-ADDRESS-TABLE enter \n");
	pifLOG(LOG_DEBUG, "aging-time=%d\n", getDefaultAgingTime());

	/* set current aging-time */
	setCurrentAgingTime(getDefaultAgingTime());

	/* apply to fea */
	feaApiMacTableSetAgingTime(getDefaultAgingTime());

	return PIF_RTN_OK;
}




Int32T pifMacAddressTableAdd(StringT mac, Uint32T type, Uint32T vid, PhysicalPortT *port)
{
	pifLOG(LOG_DEBUG, "MAC-ADDRESS-TABLE enter \n");
	pifLOG(LOG_DEBUG, "mac=%s, type=%d, vid=%d, if=%s \n", mac2str(mac), type, vid, port->name);

	/* check mac exist */
	StaticMacT* macAddr = findMacTable(vid, getMacKey(mac));
	if(macAddr != NULL){
		 pifLOG(LOG_DEBUG, "Mac(%s) already exist. return. \n", mac2str(mac));
		 return PIF_RTN_NOT_EXIST;
	}

	/* create new mac */
	macAddr = newStaticMac();
	memcpy(macAddr->macAddr, mac, ETH_ADDR_LEN);
	macAddr->type = type;
	macAddr->vid = vid;
	macAddr->iid = port->iid.idx;
	macAddr->ifIndex = port->ifIndex;
	memcpy(&(macAddr->ifName[0]), &(port->name[0]), IF_NAME_SIZE);

	/* insert mac into mac table */
	insertMacTable(vid, getMacKey(mac), macAddr);

	/* apply mac to fea */
	feaApiMacTableAddStatic(macAddr->macAddr, macAddr->ifIndex, macAddr->vid);

	return PIF_RTN_OK;
}



Int32T pifMacAddressTableDelete(StringT mac, Uint32T type, Uint32T vid, PhysicalPortT *port)
{
	pifLOG(LOG_DEBUG, "MAC-ADDRESS-TABLE enter \n");
	pifLOG(LOG_DEBUG, "mac=%s, type=%d, vid=%d, if=%s \n", mac2str(mac), type, vid, port->name);

	/* check mac exist */
	StaticMacT* macAddr = findMacTable(vid, getMacKey(mac));
	if(macAddr == NULL){
		 pifLOG(LOG_DEBUG, "Mac(%s) not exist. return. \n", mac2str(mac));
		 return PIF_RTN_ALREADY_EXIST;
	}


	/* delete mac into mac table */
	deleteMacTable(vid, getMacKey(mac));

	/* free target mac */
	freeStaticMac(macAddr);

	/* apply mac to fea */
	feaApiMacTableDeleteStatic(macAddr->macAddr, macAddr->ifIndex, macAddr->vid);

	return PIF_RTN_OK;
}



Int32T pifMacAddressTableClear()
{
	pifLOG(LOG_DEBUG, "MAC-ADDRESS-TABLE enter \n");
	return PIF_RTN_OK;
}























