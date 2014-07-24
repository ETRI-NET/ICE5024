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
#include "pifInterfaceMgr.h"
#include "pifMsgEvent.h"

 #include "pifFeaApi.h"


static InterfaceTableT* interfaceTable = NULL;
static InterfaceDatabaseT* interfaceDB = NULL;

#define INTERFACEDB     (*interfaceDB)
#define IFTREE    		(interfaceTable->tree)

/***********************************************************************
 *
 *                IPInterface Table (Key: Iid)
 *
 * *********************************************************************/
InterfaceTableT* createInterfaceTable()
{
	interfaceTable = NNMALLOC(MEM_IF, sizeof(InterfaceTableT));
	interfaceTable->tree = nnAvlInitLong(MEM_IF);
	return interfaceTable;
}

InterfaceTableT* restartInterfaceTable(InterfaceTableT* table)
{
	interfaceTable = table;
	return interfaceTable;
}

void removeInterfaceTable() { nnAvlFreeLong(IFTREE); }

void clearInterfaceTable() { nnAvlRemoveAllNodeLong(IFTREE); }

Int32T getSizeInterfaceTable() { return nnAvlCountLong(IFTREE); }

AvlNodeLongT* getRootInterfaceTable() { return ((IFTREE)->pRoot); }

void insertInterfaceTable(Uint64T key, IPInterfaceT* pData)
{
	if(nnAvlAddNodeLong(IFTREE, key, (void*)pData) != SUCCESS){
		printf("Error !");
	}
}

void deleteInterfaceTable(Uint64T key) { nnAvlRemoveNodeLong(IFTREE, key); }

IPInterfaceT* findInterfaceTable(Uint64T key) { return nnAvlLookupDataLong(IFTREE, key); }


/* preorder tree inspect */
static IPInterfaceT* rltInterface = NULL;


int inspectInterfaceName(AvlNodeLongT* treeNode, StringT name)
{
	// inspect parent node
	if(treeNode == NULL) return 0;
	IPInterfaceT* currInterface = (IPInterfaceT*)treeNode->pData;
	if(!strcmp(name, currInterface->name)) { /* if find match */
		rltInterface = currInterface;
		return 1;
	}

	// inspect left child node
	if(inspectInterfaceName(treeNode->pLeft, name)) return 1;;

	// inspect right child node
	if(inspectInterfaceName(treeNode->pRight, name)) return 1;

	return 0;
}

IPInterfaceT* findNameInterfaceTable(StringT name)
{
	rltInterface = NULL;
	AvlNodeLongT *treeRoot = IFTREE->pRoot;

	inspectInterfaceName(treeRoot, name);

	return rltInterface;
}


int inspectInterfaceIdx(AvlNodeLongT* treeNode, Uint32T idx)
{
	// inspect parent node
	if(treeNode == NULL) return 0;
	IPInterfaceT* currInterface = (IPInterfaceT*)treeNode->pData;
	if(currInterface->ifIndex == idx) { /* if find match */
		rltInterface = currInterface;
		return 1;
	}

	// inspect left child node
	if(inspectInterfaceIdx(treeNode->pLeft, idx)) return 1;;

	// inspect right child node
	if(inspectInterfaceIdx(treeNode->pRight, idx)) return 1;

	return 0;
}

IPInterfaceT* findIfIdxInterfaceTable(Uint32T idx)
{
	rltInterface = NULL;
	AvlNodeLongT *treeRoot = IFTREE->pRoot;

	inspectInterfaceIdx(treeRoot, idx);

	return rltInterface;
}









void printInterface(IPInterfaceT* ifp)
{
	/* IPInterface dump */
	pifLOG(LOG_DEBUG, "%s \n", ifp->name);
	pifLOG(LOG_DEBUG, "\tiid=%d, ifidx=%d \n", ifp->iid.idx, ifp->ifIndex);
	pifLOG(LOG_DEBUG, "\tAdmin=%s, Oper=%s \n", 
						getIfStateStr(ifp->adminState), getIfStateStr(ifp->operState));
	pifLOG(LOG_DEBUG, "\tifType=%s, ifMode=%s \n", 
						getIfTypeStr(ifp->ifType), getIfModeStr(ifp->ifIndex));
	pifLOG(LOG_DEBUG, "\tmetric=%d, mtu=%d, bw=%d \n", 
						ifp->metric, ifp->mtu, ifp->bandwidth);

	/* ConnectedAddress dump */
	if(nnListCount(ifp->connectedAddrs) == 0) return;

	ListNodeT * listNode;
	ConnectedAddressT *pAddr;
	Int8T strAddr[100];
	PLIST_LOOP(ifp->connectedAddrs, pAddr, listNode)  
	{
		//if(pAddr->address)
		{
			memset(strAddr, 0, 100);
			nnCnvPrefixtoString(strAddr, &(pAddr->address));
			pifLOG(LOG_DEBUG, "\taddr=%s \n", strAddr);
		}
	}

}

void printInterfaceTable(AvlNodeLongT* treeNode)
{
	if(treeNode == NULL) return;
	printInterface((IPInterfaceT*)treeNode->pData);

	printInterfaceTable(treeNode->pLeft);
	printInterfaceTable(treeNode->pRight);
}

void printInterfaceAll()
{
	pifLOG(LOG_DEBUG, "IPIF-MGR :: Dump All Interfaces \n");

	AvlNodeLongT *treeRoot = IFTREE->pRoot;
	printInterfaceTable(treeRoot);
}


/***********************************************************************
 *
 *                       IPInterface Manager 
 *
 * *********************************************************************/

/***********************************************************************
 * IPInterface Manager start/restart/stop
 **********************************************************************/

InterfaceDatabaseT* pifInterfaceMgrCreate()
{
	/* initialize InterfaceDatabase */
	interfaceDB    = NNMALLOC(MEM_IF, sizeof(InterfaceDatabaseT));
	INTERFACEDB.defaultMtu    = PIF_DEFAULT_MTU;
	INTERFACEDB.defaultMetric = PIF_DEFAULT_METRIC;
	INTERFACEDB.defaultVid    = PIF_DEFAULT_VID;

	INTERFACEDB.pifInterfaceTable = createInterfaceTable();

	return interfaceDB;
}

void pifInterfaceMgrInitialize()
{
}


InterfaceDatabaseT* pifInterfaceMgrRestart(InterfaceDatabaseT* db)
{
	interfaceDB = db;
	INTERFACEDB.pifInterfaceTable = restartInterfaceTable(INTERFACEDB.pifInterfaceTable);
	return interfaceDB;
}

/***********************************************************************
 * Event From FEA Interface
 **********************************************************************/

extern Int32T  getSizePortTable();
Int32T pifInterfaceMgrAddInterface(IPInterfaceT* ifp)
{
	pifLOG(LOG_DEBUG, "IPIF-MGR :: Add IPIF (%s) \n", ifp->name);

	/* insert new IPInterface into table */
	/* notify L3 event for new IPInterface */

	/* get interface id */
	Uint32T iid = ifp->iid.idx;

	/* check IPInterface already exist */
	if(findInterfaceTable(iid) != NULL) {
    	pifLOG(LOG_DEBUG, "IPInterface(%s) already exist", ifp->name);
    	return PIF_RTN_ALREADY_EXIST;
	}

	/* debug dump */
	dumpIPInterface(ifp, "create");

	/* insert new IPInterface */
	insertInterfaceTable(iid, ifp);

	/* skip not a routed interface */
	if(ifp->ifMode != ROUTED_PORT) return PIF_RTN_OK;
	
	/* post event to L3 protocols */
	pifPostEventIPInterface(ifp, PIF_EVENT_INTERFACE_ADD);

	return PIF_RTN_OK;
}


Int32T pifInterfaceMgrDelInterface(StringT ifName, Uint32T iid)
{
	pifLOG(LOG_DEBUG, "IPIF-MGR :: Delete IPIF (%s) \n", ifName);

	/* process IPInterface delete event */

	/* find the target IPInterface */
	IPInterfaceT* ifp = findInterfaceTable(iid); 
	if(ifp == NULL) {
    	pifLOG(LOG_DEBUG, "IPInterface(%s) not exist", ifName);
    	return PIF_RTN_NOT_EXIST;
	}

	/* debug dump */
	dumpIPInterface(ifp, "delete");

	/* delete all connected-address */
	// need to add

	/* post event to L3 protocols */
	pifPostEventIPInterface(ifp, PIF_EVENT_INTERFACE_DEL);

	/* delete from table */
	deleteInterfaceTable(iid);
	freeIPInterface(ifp);

	return PIF_RTN_OK;
}


Int32T pifInterfaceMgrAddAddress(StringT ifName, Uint32T iid, ConnectedAddressT *conAddr)
{
	pifLOG(LOG_DEBUG, "IPIF-MGR :: Add IPIF Address (%s) \n", ifName);

	/* add new address to the target IPInterface */
	/* notify L3 event for new IPInterface address */

	/* find the target IPInterface */
	IPInterfaceT* ifp = findInterfaceTable(iid);
	if(ifp == NULL) {
    	pifLOG(LOG_DEBUG, "IPInterface(%s) not exist", ifName);
    	return PIF_RTN_NOT_EXIST;
	}

	/* debug dump */
	dumpConnectedAddress(ifp, conAddr, "add");

	/* add new address to the IPInterface */
	nnListAddNode(ifp->connectedAddrs, conAddr);
	conAddr->ifp = ifp;

	/* skip post event if not a routed interface */
	if(ifp->ifMode != ROUTED_PORT) return PIF_RTN_OK;
	
	/* post event to L3 protocols */
	pifPostEventIPAddr(ifp, conAddr, PIF_EVENT_INTERFACE_ADDRESS_ADD);

    pifLOG(LOG_DEBUG, "end\n");
	return PIF_RTN_OK;
}


Int32T pifInterfaceMgrDelAddress(StringT ifName, Uint32T iid, PrefixT *addr)
{
	Int8T strAddr[IF_NAME_SIZE];
	nnCnvPrefixtoString(strAddr, addr);
	pifLOG(LOG_DEBUG, "IPIF-MGR :: Delete IPIF Address (%s, %s) \n", ifName, strAddr);

	/* delete target address from the target IPInterface */
	/* notify L3 event for deletion of IPInterface address */

	/* find the target IPInterface */
	IPInterfaceT* ifp = findInterfaceTable(iid);
	if(ifp == NULL) {
    	pifLOG(LOG_DEBUG, "IPInterface(%s) not exist\n", ifName);
    	return PIF_RTN_NOT_EXIST;
	}

	/* find the target connected address */
	ConnectedAddressT* conAddr = pifInterfaceMgrGetConnectedAddr(ifp, addr);
	if(conAddr == NULL) {
    	pifLOG(LOG_DEBUG, "address(%s) not exist\n", strAddr);
    	return PIF_RTN_NOT_EXIST;
	}

	/* debug dump */
	dumpConnectedAddress(ifp, conAddr, "delete");

	/* post event if a routed interface */
	if(ifp->ifMode == ROUTED_PORT) {
		pifPostEventIPAddr(ifp, conAddr, PIF_EVENT_INTERFACE_ADDRESS_DEL);
	}

	/* delete address from the IPInterface */
    pifLOG(LOG_DEBUG, "Remove address (%s) from list \n", strAddr);
	nnListRemoveNode(ifp->connectedAddrs, conAddr);
    pifLOG(LOG_DEBUG, "free address (%s) \n", strAddr);
	freeConnectedAddress(conAddr);

    pifLOG(LOG_DEBUG, "end\n");
	return PIF_RTN_OK;
}


Int32T pifInterfaceMgrUpInterface(StringT ifName, Uint32T ifIndex, Uint32T iid)
{
	pifLOG(LOG_DEBUG, "IPIF-MGR :: Up IPIF (name: %s, iid: %d) \n", 
						ifName, iid);

	/* process IPInterface bring up event */
	/* check AdminState & Operstat, and post Interface Event */

	/* find the target IPInterface */
	IPInterfaceT* ifp = findInterfaceTable(iid); 
	if(ifp == NULL) {
    	pifLOG(LOG_DEBUG, "IPInterface(%s) not exist", ifName);
    	return PIF_RTN_NOT_EXIST;
	}

	/* check AdminState */
	if(ifp->adminState != ADMIN_UP){
    	pifLOG(LOG_DEBUG, "Adminstate is down, but Operstate is up");
	}

	if(ifp->operState == STATE_UP){
    	pifLOG(LOG_DEBUG, "Operstate is already up");
    	return PIF_RTN_BAD_ATTR;
	}

	/* set Operstate as UP */
	ifp->operState = STATE_UP;

	/* debug dump */
	dumpIPInterface(ifp, "oper-state up");

	/* skip in case of switchport */
	if(ifp->ifMode == SWITCH_PORT) {
    	pifLOG(LOG_DEBUG, "skip, interface %s is not L3 interface\n", ifName);
		return PIF_RTN_OK;
	}

	/* post event to L3 protocols */
	pifPostEventIPInterface(ifp, PIF_EVENT_INTERFACE_UP);

	return PIF_RTN_OK;
}


Int32T pifInterfaceMgrDownInterface(StringT ifName, Uint32T ifIndex, Uint32T iid)
{
	pifLOG(LOG_DEBUG, "IPIF-MGR :: Down IPIF (name: %s, iid: %d) \n", 
						ifName, iid);

	/* process IPInterface down event */
	/* check AdminState & Operstat, and post Interface Event */

	/* find the target IPInterface */
	IPInterfaceT* ifp = findInterfaceTable(iid); 
	if(ifp == NULL) {
    	pifLOG(LOG_DEBUG, "IPInterface(%s) not exist\n", ifName);
    	return PIF_RTN_NOT_EXIST;
	}

	/* check AdminState */
	if(ifp->adminState != ADMIN_UP){
    	pifLOG(LOG_DEBUG, "Adminstate is already down\n");
	}

	if(ifp->operState == STATE_DOWN){
    	pifLOG(LOG_DEBUG, "Operstate is already down\n");
    	return PIF_RTN_BAD_ATTR;
	}

	/* set Operstate as UP */
	ifp->operState = STATE_DOWN;

	/* debug dump */
	dumpIPInterface(ifp, "oper-state down");

	/* skip in case of switchport */
	if(ifp->ifMode == SWITCH_PORT) {
    	pifLOG(LOG_DEBUG, "skip, interface %s is not L3 interface\n", ifName);
		return PIF_RTN_OK;
	}


	/* post event to L3 protocols */
	pifPostEventIPInterface(ifp, PIF_EVENT_INTERFACE_DOWN);

	return PIF_RTN_OK;
}



/*************************************************************
 * API of IPInterface Manager
 ************************************************************/

IPInterfaceT* pifInterfaceMgrGetInterface(Uint32T iid)
{

	IPInterfaceT* ifp = findInterfaceTable(iid);
	if(ifp == NULL){
		pifLOG(LOG_DEBUG, "interface iid %d not exist \n", iid);
		return NULL;
	}

	pifLOG(LOG_DEBUG, "IF-MGR found %s ok \n", ifp->name);
	return ifp;
}


ConnectedAddressT* pifInterfaceMgrGetConnectedAddr(IPInterfaceT* ifp, PrefixT* addr)
{
	pifLOG(LOG_DEBUG, "IF-MGR enter \n");

	if(nnListCount(ifp->connectedAddrs) == 0) {
		pifLOG(LOG_DEBUG, "addr not found \n");
		return NULL;
	}

	ListNodeT * listNode;
	ConnectedAddressT *pAddr;
	Int8T strAddr[100];
	PLIST_LOOP(ifp->connectedAddrs, pAddr, listNode)
	{
		if(nnPrefixSame(&(pAddr->address), addr) == TRUE) {
			nnCnvPrefixtoString(strAddr, addr);
			pifLOG(LOG_DEBUG, "addr(%s) found ok.\n", strAddr);
			return pAddr;
		}
	}

	pifLOG(LOG_DEBUG, "addr not found \n");
	return NULL;
}


void pifInterfaceMgrSetPortMode(IPInterfaceT* ifp, PortConfigModeT mode)
{
	pifLOG(LOG_DEBUG, "------------------------------------------\n");
	pifLOG(LOG_DEBUG, " %s Port Mode: %s --> %s\n", 
						ifp->name,
						getIfModeStr(ifp->ifMode), getIfModeStr(mode));
	pifLOG(LOG_DEBUG, "------------------------------------------\n");
	ifp->ifMode = mode;
}





Int32T pifInterfaceMgrConfigAddAddress(IPInterfaceT* ifp, PrefixT* addr)
{
	Int8T strAddr[IF_NAME_SIZE];
	nnCnvPrefixtoString(strAddr, addr);
	pifLOG(LOG_DEBUG, "IF-MGR enter %s, %s\n", ifp->name, strAddr);

	/* double check address exist */
	if(pifInterfaceMgrGetConnectedAddr(ifp, addr) != NULL){
		pifLOG(LOG_DEBUG, "address already exist \n");
		return PIF_RTN_ALREADY_SET;
	}


	/* Create new ConnectedAddress */
	ConnectedAddressT* conAddr = newConnectedAddress();

	/* set prefix & broadcast address */
	nnPrefixCopy(&(conAddr->address), addr);

	/* set broadcast address */
	nnPrefixCopy(&(conAddr->broadcast), addr);
	Prefix4T* broadcast = (Prefix4T*)&(conAddr->broadcast);
	struct in_addr mask;
	nnCnvPrefixLentoNetmask(&mask, &(broadcast->prefixLen));
	broadcast->prefix.s_addr |= ~mask.s_addr;

	/* add new address to the IPInterface */
	nnListAddNode(ifp->connectedAddrs, conAddr);
	conAddr->ifp = ifp;

	/* set FEA interface address */
	feaApiInterfaceAddressAdd (ifp->name, ifp->ifIndex, conAddr);

	/* skip post event if not a routed interface */
	if(ifp->ifMode != ROUTED_PORT) return PIF_RTN_OK;

	/* debug dump */
	dumpConnectedAddress(ifp, conAddr, "config-add");

	/* post event to L3 protocols */
	pifPostEventIPAddr(ifp, conAddr, PIF_EVENT_INTERFACE_ADDRESS_ADD);

	return PIF_RTN_OK;
}


Int32T pifInterfaceMgrConfigDelAddress(IPInterfaceT* ifp, PrefixT* addr)
{
	Int8T strAddr[IF_NAME_SIZE];
	nnCnvPrefixtoString(strAddr, addr);
	pifLOG(LOG_DEBUG, "IF-MGR enter %s, %s\n", ifp->name, strAddr);

	/* double check address exist */
	ConnectedAddressT* conAddr = pifInterfaceMgrGetConnectedAddr(ifp, addr);
	if(conAddr == NULL){
		pifLOG(LOG_DEBUG, "address not exist \n");
		return PIF_RTN_NOT_EXIST;
	}

	/* set FEA interface address */
	feaApiInterfaceAddressDel (ifp->name, ifp->ifIndex, conAddr);

	/* debug dump */
	dumpConnectedAddress(ifp, conAddr, "config-delete");

	/* post event if a routed interface */
	pifPostEventIPAddr(ifp, conAddr, PIF_EVENT_INTERFACE_ADDRESS_DEL);

	/* delete address from the IPInterface */
	nnListRemoveNode(ifp->connectedAddrs, conAddr);
	freeConnectedAddress(conAddr);

	return PIF_RTN_OK;
}


Int32T pifInterfaceMgrConfigClearAddress(IPInterfaceT* ifp)
{
	pifLOG(LOG_DEBUG, "IF-MGR enter %s \n", ifp->name);

	ListNodeT * listNode;
	ConnectedAddressT *pAddr;
	PLIST_LOOP(ifp->connectedAddrs, pAddr, listNode)
	{
		/* set FEA interface address */
		feaApiInterfaceAddressDel (ifp->name, ifp->ifIndex, pAddr);

		/* debug dump */
		dumpConnectedAddress(ifp, pAddr, "config-delete");

		/* post event if a routed interface */
		pifPostEventIPAddr(ifp, pAddr, PIF_EVENT_INTERFACE_ADDRESS_DEL);

		/* delete address from the IPInterface */
		nnListRemoveNode(ifp->connectedAddrs, pAddr);
		freeConnectedAddress(pAddr);

	} /* end of LIST_LOOP */

	return PIF_RTN_OK;
}



/* set IPInterface no shutdown/shutdown */
#include <linux/if.h>
Int32T pifInterfaceMgrEnableIf(IPInterfaceT* ifp)
{
	pifLOG(LOG_DEBUG, "IF-MGR enter \n");

	pifLOG(LOG_DEBUG, "ifName:%s, AdminState:%d \n", 
						ifp->name, ifp->adminState);

	/* check adminstate */
	if(ifp->adminState == ADMIN_UP){
		pifLOG(LOG_DEBUG, "interface already enabled \n");
		return PIF_RTN_ALREADY_SET;
	}

	/* set FEA adminstate as up */
	feaApiInterfaceUp(ifp->name, ifp->ifIndex, ifp->iid.idx, ifp->flags);

	ifp->adminState = ADMIN_UP;
	Uint32T flags = IFF_UP;
	ifp->flags |= flags;

	return PIF_RTN_OK;
}

Int32T pifInterfaceMgrDisableIf(IPInterfaceT* ifp)
{
	pifLOG(LOG_DEBUG, "IF-MGR enter \n");

	pifLOG(LOG_DEBUG, "ifName:%s, AdminState:%d \n", 
						ifp->name, ifp->adminState);

	/* check adminstate */
	if(ifp->adminState == ADMIN_DOWN){
		pifLOG(LOG_DEBUG, "interface already disabled \n");
		return PIF_RTN_ALREADY_SET;
	}

	/* set FEA adminstate as up */
	feaApiInterfaceDown(ifp->name, ifp->ifIndex, ifp->iid.idx, ifp->flags);
	ifp->adminState = ADMIN_DOWN;

	Uint32T flags = IFF_UP;
	ifp->flags &= ~flags;

	return PIF_RTN_OK;
}

Int32T pifInterfaceMgrSetRoutedPort(IPInterfaceT* ifp)
{
	pifLOG(LOG_DEBUG, "IPIF-MGR :: Add IPIF (%s) \n", ifp->name);

	/* check IPInterface exist */
	Uint32T iid = ifp->iid.idx;
	if(findInterfaceTable(iid) == NULL) {
    	pifLOG(LOG_DEBUG, "IPInterface(%s) not exist", ifp->name);
    	return PIF_RTN_NOT_EXIST;
	}

	/* check previous port state and set state */
	PortConfigModeT previousMode = ifp->ifMode;
	if(previousMode == SWITCH_PORT){
		pifInterfaceMgrClearSwitchPort(ifp);
	}	
	else if(previousMode == NOT_CONFIGURED){
		/* set interface as routed interface */
	}
	else if(previousMode == ROUTED_PORT){
		/* option.1 just return */
		/* option.2 process set port routed port state */
		/* currently, choose option.2 */
	}

	/* post event to L3 protocols */
	pifPostEventIPInterface(ifp, PIF_EVENT_INTERFACE_ADD);

	/* post addr event to L3 protocols if exist */
	ListNodeT * listNode;
	ConnectedAddressT *pAddr;
	PLIST_LOOP(ifp->connectedAddrs, pAddr, listNode)
	{
		//if(pAddr->address)
		{
			pifPostEventIPAddr(ifp, pAddr, PIF_EVENT_INTERFACE_ADDRESS_ADD);
		}
	}

	/* set interface as routed interface */
	pifInterfaceMgrSetPortMode(ifp, ROUTED_PORT);

	return PIF_RTN_OK;
}


Int32T pifInterfaceMgrClearRoutedPort(IPInterfaceT* ifp)
{
	pifLOG(LOG_DEBUG, "IPIF-MGR :: IPIF (%s) \n", ifp->name);

	/* check IPInterface exist */
	Uint32T iid = ifp->iid.idx;
	if(findInterfaceTable(iid) == NULL) {
    	pifLOG(LOG_DEBUG, "IPInterface(%s) not exist", ifp->name);
    	return PIF_RTN_NOT_EXIST;
	}

	/* clear all L3 address */
	pifInterfaceMgrConfigClearAddress(ifp);

	/* clear L3 interface */
	/* note. we do not delete interface instance actually */
	/* just post event to L3 protocols */
	pifPostEventIPInterface(ifp, PIF_EVENT_INTERFACE_DEL);

	return PIF_RTN_OK;
}



Int32T pifInterfaceMgrSetSwitchPort(IPInterfaceT* ifp)
{
	/* no change */
	if(ifp->ifMode == SWITCH_PORT) return PIF_RTN_OK;

	pifLOG(LOG_DEBUG, "IPIF-MGR :: Del IPIF (%s) \n", ifp->name);

	/* check IPInterface exist */
	Uint32T iid = ifp->iid.idx;
	if(findInterfaceTable(iid) == NULL) {
    	pifLOG(LOG_DEBUG, "IPInterface(%s) not exist", ifp->name);
    	return PIF_RTN_NOT_EXIST;
	}

	/* check previous port state and set state */
	PortConfigModeT previousMode = ifp->ifMode;
	if(previousMode == ROUTED_PORT){
		pifInterfaceMgrClearRoutedPort(ifp);
	}	
	else if(previousMode == NOT_CONFIGURED){
		/* set interface as routed interface */
	}
	else if(previousMode == SWITCH_PORT){
		/* option.1 just return */
		/* option.2 process set port switch port state */
		/* currently, choose option.2 */
	}

	/* set interface as switch interface */
	pifInterfaceMgrSetPortMode(ifp, SWITCH_PORT);

	return PIF_RTN_OK;
}


Int32T pifInterfaceMgrClearSwitchPort(IPInterfaceT* ifp)
{
	pifLOG(LOG_DEBUG, "IPIF-MGR :: IPIF (%s) \n", ifp->name);

	/* check IPInterface exist */
	Uint32T iid = ifp->iid.idx;
	if(findInterfaceTable(iid) == NULL) {
    	pifLOG(LOG_DEBUG, "IPInterface(%s) not exist", ifp->name);
    	return PIF_RTN_NOT_EXIST;
	}

	/* currently nothing to be done */

	return PIF_RTN_OK;
}














