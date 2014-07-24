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
#include "nnMemmgr.h"
#include "pif.h"
#include "pifAggMgr.h"

#include "pifMsgEvent.h"
#include "pifMsgIpc.h"

#include "pifFeaApi.h"

#include "pifInterfaceMgr.h"
#include "pifPortMgr.h"

static AggDatabaseT* aggDB = NULL;
static AggTableT* aggTable = NULL;

#define AGGTREE    (aggTable->tree)
#define AGGDB     (*aggDB)

/*************************************************************
 *
 *       PIF Aggregation Table
 *
 ************************************************************/

AggTableT* createAggTable()
{
	aggTable = NNMALLOC(MEM_IF, sizeof(AggTableT));
	aggTable->tree = nnAvlInitLong(MEM_IF);
	return aggTable;
}

AggTableT* restartAggTable(AggTableT* table)
{
	aggTable = table;
	return aggTable;
}

void removeAggTable() { nnAvlFreeLong(AGGTREE); }

void clearAggTable() { nnAvlDeleteAllNodeLong(AGGTREE); }

Int32T getSizeAggTable() { return nnAvlCountLong(AGGTREE); }

AvlNodeLongT* getRootAggTable() { return ((AGGTREE)->pRoot); }

void insertAggTable(Int64T key, AggGroupT* pData)
{
	if(nnAvlAddNodeLong(AGGTREE, key, (void*)pData) <= 0){
		printf("Error !");
	};
}

void deleteAggTable(Uint64T key) { nnAvlRemoveNodeLong(AGGTREE, key); }

/* preorder tree inspect */
static AggGroupT* rltAgg = NULL;
int inspectAggTable(AvlNodeLongT* treeNode, StringT name)
{
	// inspect parent node
	if(treeNode == NULL) return 0;
	AggGroupT* currAgg = (AggGroupT*)treeNode->pData;
	if(!strcmp(name, currAgg->name)) { /* if find match */
		rltAgg = currAgg;
		return 1;
	}

	// inspect left child node
	if(inspectAggTable(treeNode->pLeft, name)) return 1;;

	// inspect right child node
	if(inspectAggTable(treeNode->pRight, name)) return 1;

	return 0;
}

AggGroupT* findAggTable(Uint64T key) { return nnAvlLookupDataLong(AGGTREE, key); }

AggGroupT* findNameAggTable(StringT name)
{
	rltAgg = NULL;
	AvlNodeLongT *treeRoot = AGGTREE->pRoot;

	inspectAggTable(treeRoot, name);

	return rltAgg;
}


void printAgg(AggGroupT* port)
{
}

void printAggTable(AvlNodeLongT* treeNode)
{
	if(treeNode == NULL) return;
	printAgg((AggGroupT*)treeNode->pData);

	printAggTable(treeNode->pLeft);
	printAggTable(treeNode->pRight);
}


/*************************************************************
 *
 *       PIF Aggregation Manager
 *
 ************************************************************/

/******************************************************************
 * Agg Manager start/restart/stop
 ******************************************************************/
AggDatabaseT* pifAggMgrCreate()
{
	/* initialize AggDatabase */
	aggDB    = NNMALLOC(MEM_IF, sizeof(AggDatabaseT));
	AGGDB.defaultVid   		= 1;

	AGGDB.pifAggTable = createAggTable();

	return aggDB;
}

AggDatabaseT* pifAggMgrRestart(AggDatabaseT* db)
{
	aggDB = db;
	AGGDB.pifAggTable = restartAggTable(AGGDB.pifAggTable);
	return aggDB;
}


void pifAggMgrInitialize()
{
}


/******************************************************************
 * Agg Manager external API
 ******************************************************************/
AggGroupT* pifAggMgrGetGroup(Uint32T aggId)
{
	pifLOG(LOG_DEBUG, "AGG-MGR :: %d\n", aggId);

	AggGroupT* agg = findAggTable(aggId);
	if(agg == NULL){
		pifLOG(LOG_DEBUG, "Group(%d) not exist\n", aggId);
	}

	return agg;
}

Int32T pifAggMgrGetPortCount(AggGroupT* agg)
{
	if(agg == NULL) {
		pifLOG(LOG_DEBUG, "AGG-MGR :: null \n");
		return 0;
	}

	pifLOG(LOG_DEBUG, "AGG-MGR ::\n");
	
	return nnListCount(agg->portList);
}

void pifAggMgrSetAggName(StringT name, Int32T aggId)
{
	pifLOG(LOG_DEBUG, "AGG-MGR ::\n");
	sprintf(name, "po%d", aggId);
}

void pifAggMgrSetMacAddr(StringT mac, Int32T aggId)
{
	pifLOG(LOG_DEBUG, "AGG-MGR ::\n");
	Int8T macStr[IF_NAME_SIZE];
	sprintf(macStr, "00:30:48:01:01:%02x", aggId);

	memcpy(mac, str2mac(macStr), ETH_ADDR_LEN);
}


/******************************************************************
 * Agg Manager create aggregator
 ******************************************************************/
Int32T pifAggMgrAddGroup(StringT name, Uint32T id, Int8T* mac, Int16T mode)
{
	/* create and insert new group into table */

	pifLOG(LOG_DEBUG, "AGG-MGR :: %s\n", name);

	/* check group name */
	Uint32T iid = getIidbyName(name);
	if(iid < 0) {
		pifLOG(LOG_DEBUG, "invalid group name (%s, %d)", name, id);
		return PIF_RTN_BAD_ATTR;
	}

	/* check group already exist */
	if(findAggTable(id) != NULL){
		pifLOG(LOG_DEBUG, "group (%s, %d) already exist", name, id);
		return PIF_RTN_ALREADY_EXIST;
	}

	/* create new group */
	AggGroupT* agg = newAggGroup();

	/* set name and id */
	agg->iid.idx = iid;
	agg->aggId = id;
	strncpy(agg->name, name, strlen(name));

	/* set hw type */
	agg->hwType = AGG_ETHERNET;
	memcpy(agg->hwAddr, mac, ETH_ADDR_LEN);

	/* set agg mode */
	agg->aggMode = mode;

	/* insert new group into table */
	insertAggTable(id, agg);

	/* set Fea to create aggregator interface */
	feaApiLacpAddAggregator(agg->name, (char*)agg->hwAddr, agg->portMode);

	/* dump debug */
	dumpAggGroup(agg, "CREATE");

	return PIF_RTN_OK;
}



/******************************************************************
 * Agg Manager delete aggregator
 ******************************************************************/
Int32T pifAggMgrDeleteGroup(StringT name)
{
	/* delete a group from table */

	pifLOG(LOG_DEBUG, "AGG-MGR :: %s\n", name);

	/* check group name */
	Uint32T iid = getIidbyName(name);
	if(iid < 0) {
		pifLOG(LOG_DEBUG, "invalid group name (%s)", name);
		return PIF_RTN_BAD_ATTR;
	}

	/* check group exist */
	AggGroupT* agg = findAggTable(getIidLogicalId(iid));
	if(agg == NULL){
		pifLOG(LOG_DEBUG, "group (%s) not exist", name);
		return PIF_RTN_NOT_EXIST;
	}

	/* check group has port members */

	/* set Fea to delete aggregator interface */
	feaApiLacpDeleteAggregator(agg->name);

	/* dump debug */
	dumpAggGroup(agg, "DELETE");

	/* remove group form table */
	deleteAggTable(agg->aggId);
	freeAggGroup(agg);

	return PIF_RTN_OK;
}


/******************************************************************
 * Agg Manager create static aggregator
 ******************************************************************/
Int32T pifAggMgrAddStaticGroup(Uint32T id, Int8T* mac)
{
	/* create and insert new group into table */

	pifLOG(LOG_DEBUG, "AGG-MGR :: %d\n", id);

	/* get group name */
	Int8T name[IF_NAME_SIZE];
	pifAggMgrSetAggName(name, id);

	/* get mac address */
	/*
	Int8T macAddr[ETH_ADDR_LEN];
	pifAggMgrSetMacAddr(macAddr, id);
	*/

	/* get iid */
	Uint32T iid = getIidbyName(name);
	if(iid < 0) {
		pifLOG(LOG_DEBUG, "invalid group name (%s, %d)", name, id);
		return PIF_RTN_BAD_ATTR;
	}

	/* check group already exist */
	if(findAggTable(id) != NULL){
		pifLOG(LOG_DEBUG, "group (%s, %d) already exist", name, id);
		return PIF_RTN_ALREADY_EXIST;
	}

	/* create new group */
	pifLOG(LOG_DEBUG, "create new group (%s)\n ", name);
	AggGroupT* agg = newAggGroup();

	/* set name and id */
	agg->iid.idx = iid;
	agg->aggId = id;
	strncpy(agg->name, name, strlen(name));

	/* set hw type */
	agg->hwType = AGG_ETHERNET;
	memcpy(agg->hwAddr, mac, ETH_ADDR_LEN);

	/* set agg mode */
	agg->aggMode = 0;

	/* set agg mode */
	agg->portMode = SWITCH_PORT;

	/* set static flag */
	agg->staticFlag = 1;

	/* insert new group into table */
	insertAggTable(id, agg);

	/* set Fea to create aggregator interface */
	feaApiLacpAddAggregator(agg->name, (char*)agg->hwAddr, agg->portMode);

	/* dump debug */
	dumpAggGroup(agg, "STATIC-CREATE");

	return PIF_RTN_OK;
}


/******************************************************************
 * Agg Manager delete static aggregator
 ******************************************************************/
Int32T pifAggMgrDeleteStaticGroup(Uint32T aggId)
{
	/* delete a group from table */

	pifLOG(LOG_DEBUG, "AGG-MGR :: (group=%d)\n", aggId);

	/* get group name */
	Int8T name[IF_NAME_SIZE];
	pifAggMgrSetAggName(name, aggId);

	/* check group name */
	Uint32T iid = getIidbyName(name);
	if(iid < 0) {
		pifLOG(LOG_DEBUG, "fail, invalid group name (%s)", name);
		return PIF_RTN_BAD_ATTR;
	}

	/* check group exist */
	AggGroupT* agg = findAggTable(aggId);
	if(agg == NULL){
		pifLOG(LOG_DEBUG, "fail, group (%s) not exist", name);
		return PIF_RTN_NOT_EXIST;
	}

	/* check group has port members */
	if(pifAggMgrGetPortCount(agg) != 0) {
		pifLOG(LOG_DEBUG, "fail, group (%s) has port members", name);
		return PIF_RTN_NOT_EXIST;
	}

	/* set Fea to delete aggregator interface */
	feaApiLacpDeleteAggregator(agg->name);

	/* dump debug */
	dumpAggGroup(agg, "STATIC-DELETE");

	/* remove group form table */
	deleteAggTable(aggId);
	freeAggGroup(agg);

	return PIF_RTN_OK;
}




/******************************************************************
 * Agg Manager add port to aggregator 
 ******************************************************************/

Int32T pifAggMgrAddPortMember(StringT name, Uint32T id, Uint32T portIfIndex)
{
	pifLOG(LOG_DEBUG, "AGG-MGR :: agg=%s, port=%d \n", name, portIfIndex);

	/* check group exist */

	/* check group name */
	Uint32T iid = getIidbyName(name);
	if(iid < 0) {
		pifLOG(LOG_DEBUG, "fail, invalid group name (%s)", name);
		return PIF_RTN_BAD_ATTR;
	}

	/* check group exist */
	AggGroupT* agg = findAggTable(id);
	if(agg == NULL){
		pifLOG(LOG_DEBUG, "fail, group (%s) not exist", name);
		return PIF_RTN_NOT_EXIST;
	}

	/* notify portMgr and interfaceMgr to down the interface */

	/* get port by IfIndex */
	PhysicalPortT* port = findIfIdxPortTable(portIfIndex);
	if(port == NULL) {
		pifLOG(LOG_DEBUG, "fail, port (%d) not found \n", portIfIndex);
		return PIF_RTN_NOT_EXIST;
	}

	/* get interface by IfIndex */
	IPInterfaceT* ifp = findIfIdxInterfaceTable(portIfIndex);
	if(ifp == NULL) {
		pifLOG(LOG_DEBUG, "fail, interface (%d) not found \n", portIfIndex);
		return PIF_RTN_NOT_EXIST;
	}

	/* notify to portMgr */
	pifPortMgrDownPort(port->name, port->ifIndex, port->iid.idx);

	/* notify to interfaceMgr */
	pifInterfaceMgrDownInterface(ifp->name, ifp->ifIndex, ifp->iid.idx);

	/* now, set fea to add port to group */
	feaApiLacpAddPortToAggregator(agg->name, port->name, port->ifIndex);

	/* set port aggId */
	port->aggId = id;

	/* add port to group */
	nnListAddNode(agg->portList, port);

	/* dump debug */
	dumpAggGroup(agg, "ADD-PORT");

	return PIF_RTN_OK;
}

/******************************************************************
 * Agg Manager delete port from aggregator 
 ******************************************************************/

Int32T pifAggMgrDeletePortMember(StringT name, Uint32T id, Uint32T portIfIndex)
{
	pifLOG(LOG_DEBUG, "AGG-MGR :: agg=%s, port=%d \n", name, portIfIndex);

	/* check group exist */

	/* check group name */
	Uint32T iid = getIidbyName(name);
	if(iid < 0) {
		pifLOG(LOG_DEBUG, "fail, invalid group name (%s)", name);
		return PIF_RTN_BAD_ATTR;
	}

	/* check group exist */
	AggGroupT* agg = findAggTable(id);
	if(agg == NULL){
		pifLOG(LOG_DEBUG, "fail, group (%s) not exist", name);
		return PIF_RTN_NOT_EXIST;
	}

	/* notify portMgr and interfaceMgr to up the interface */

	/* get port by IfIndex */
	PhysicalPortT* port = findIfIdxPortTable(portIfIndex);
	if(port == NULL) {
		pifLOG(LOG_DEBUG, "fail, port (%d) not found \n", portIfIndex);
		return PIF_RTN_NOT_EXIST;
	}

	/* get interface by IfIndex */
	IPInterfaceT* ifp = findIfIdxInterfaceTable(portIfIndex);
	if(ifp == NULL) {
		pifLOG(LOG_DEBUG, "fail, interface (%d) not found \n", portIfIndex);
		return PIF_RTN_NOT_EXIST;
	}

	/* notify to portMgr */
	pifPortMgrUpPort(port->name, port->ifIndex, port->iid.idx);

	/* notify to interfaceMgr */
	pifInterfaceMgrUpInterface(ifp->name, ifp->ifIndex, ifp->iid.idx);

	/* now, set fea to delete port from group */
	feaApiLacpDeletePortToAggregator(agg->name, port->name, port->ifIndex);

	/* clear port aggId */
	port->aggId = 0;

	/* delete port from group */
	nnListRemoveNode(agg->portList, port);

	/* dump debug */
	dumpAggGroup(agg, "REMOVE-PORT");

	return PIF_RTN_OK;
}







