/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : pifmgr Data Structure Utility 처리기능을 수행
 * - Block Name : PIF Manager
 * - Process Name : pifmgr
 * - Creator : Seungwoo Hong
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : pifMsg.c
 *
 * $Author: $
 * $Date: $
 * $Revision: $
 * $LastChangedBy: $
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>

#include "pif.h" 
#include "nosLib.h" 
#include "pifDataTypes.h" 

#include "pifInterfaceMgr.h"

/************************************************************/
/* Instantiation of Data Structures 
 ************************************************************/

PhysicalPortT* newPhysicalPort(void)
{
	PhysicalPortT* port = (PhysicalPortT*)NNMALLOC(MEM_IF, sizeof(PhysicalPortT));
	memset(port, 0, sizeof(PhysicalPortT));
	port->routedPort.connectedSubIpIfs = nnListInit(NULL, MEM_IF);
	return port;
}

void freePhysicalPort(PhysicalPortT* port)
{
	if(port == NULL) return;
	if(port->routedPort.connectedSubIpIfs != NULL) 
		nnListFree(port->routedPort.connectedSubIpIfs);
	NNFREE(MEM_IF, port);
}


AggGroupT* newAggGroup(void)
{
	AggGroupT* agg = (AggGroupT*)NNMALLOC(MEM_IF, sizeof(AggGroupT));
	memset(agg, 0, sizeof(AggGroupT));
	agg->portList = nnListInit(NULL, MEM_IF); 
	agg->routedPort.connectedSubIpIfs = nnListInit(NULL, MEM_IF);
	return agg;
}

void freeAggGroup(AggGroupT* agg)
{
	if(agg == NULL) return;
	if(agg->portList != NULL) 
		nnListFree(agg->portList);
	if(agg->routedPort.connectedSubIpIfs != NULL) 
		nnListFree(agg->routedPort.connectedSubIpIfs);
	NNFREE(MEM_IF, agg);
}


VlanT* newVlan(void)
{
	VlanT* vlan = (VlanT*)NNMALLOC(MEM_IF, sizeof(VlanT));
	memset(vlan, 0, sizeof(VlanT));
	vlan->portList = nnListInit(NULL, MEM_IF); 
	return vlan;
}

void freeVlan(VlanT* vlan)
{
	if(vlan == NULL) return;

	if(nnListCount(vlan->portList) != 0){
		nnListRemoveAllNode(vlan->portList);
	}

	if(vlan->portList != NULL){ 
		NNFREE(LIST_HEAD, vlan->portList);
	}
	NNFREE(MEM_IF, vlan);
}


void clearVlan(VlanT* vlan)
{
	if(vlan == NULL) return;

	if(nnListCount(vlan->portList) != 0){
		nnListRemoveAllNode(vlan->portList);
	}

	if(vlan->portList != NULL){ 
		NNFREE(LIST_HEAD, vlan->portList);
	}
}



StaticMacT* newStaticMac(void)
{
	StaticMacT* mac = (StaticMacT*)NNMALLOC(MEM_IF, sizeof(StaticMacT));
	memset(mac, 0, sizeof(StaticMacT));
	//mac->portList = nnListInit(NULL, MEM_IF); 
	return mac;
}


void freeStaticMac(StaticMacT* mac)
{
	if(mac == NULL) return;

	if(mac->portList != NULL){ 
		if(nnListCount(mac->portList) != 0){
			nnListRemoveAllNode(mac->portList);
		}
		NNFREE(LIST_HEAD, mac->portList);
	}
	NNFREE(MEM_IF, mac);
}





IPInterfaceT* newIPInterface(void)
{
	IPInterfaceT* ifp = (IPInterfaceT*)NNMALLOC(MEM_IF, sizeof(IPInterfaceT));
	memset(ifp, 0, sizeof(IPInterfaceT));
	ifp->connectedAddrs = nnListInit(NULL, MEM_IF); 
	return ifp;
}

void freeIPInterface(IPInterfaceT* ifp)
{
	if(ifp == NULL) return;
	if(ifp->connectedAddrs != NULL) 
		nnListFree(ifp->connectedAddrs);
	NNFREE(MEM_IF, ifp);
}


ConnectedAddressT* newConnectedAddress(void)
{
	ConnectedAddressT* addr = (ConnectedAddressT*)NNMALLOC(MEM_IF, sizeof(ConnectedAddressT));
	memset(addr, 0, sizeof(ConnectedAddressT));

	/*
	addr->address = nnPrefixNew();
	memset(addr->address, 0, sizeof(PrefixT));
	addr->broadcast = nnPrefixNew();
	memset(addr->broadcast, 0, sizeof(PrefixT));
	*/
	return addr;
}

void freeConnectedAddress(ConnectedAddressT* addr)
{
	if(addr == NULL) return;

	/*
	if(addr->address != NULL) nnPrefixFree(addr->address);
	if(addr->broadcast != NULL) nnPrefixFree(addr->broadcast);
	*/
	NNFREE(MEM_IF, addr);
	pifLOG(LOG_DEBUG, "FREE ConnectedAddress(%p) end\n", addr);
}

/************************************************************/
/* Get attribute of Data Structures 
 ************************************************************/

SwitchPortT* getSwitchPort(PhysicalPortT* port)
{
	return &(port->switchPort);
}



/************************************************************/
/* Print of Data Structures 
 ************************************************************/
void printAdminOperState(void* obj)
{
	PhysicalPortT* ifp = (PhysicalPortT*)obj;
	pifLOG(LOG_DEBUG, "------------------------------------------\n");
	pifLOG(LOG_DEBUG, " Current State: Admin=%s, Oper=%s \n", 
			getIfStateStr(ifp->adminState), getIfStateStr(ifp->operState));
	pifLOG(LOG_DEBUG, "------------------------------------------\n");
}


void printSwitchPort(SwitchPortT* port)
{
	Int8T strBuff[1024] = {};
	printStrSwitchPort(strBuff, port);
	pifLOG(LOG_DEBUG, "%s", strBuff);
}


#include <linux/if.h>
#define IF_FLAG_OUT(X,FLAG_STR) if (flags & (X)) strPrintf("%s ", FLAG_STR);
Uint32T printStrIfFlag(StringT str, Uint32T flags)
{
  StringT saveStr = str;
  if(flags == 0) return 0;
  strPrintf("\tFlag: ");
  IF_FLAG_OUT (IFF_UP, "UP");
  IF_FLAG_OUT (IFF_BROADCAST, "BROADCAST");
  IF_FLAG_OUT (IFF_DEBUG, "DEBUG");
  IF_FLAG_OUT (IFF_LOOPBACK, "LOOPBACK");
  IF_FLAG_OUT (IFF_POINTOPOINT, "POINTOPOINT");
  IF_FLAG_OUT (IFF_NOTRAILERS, "NOTRAILERS");
  IF_FLAG_OUT (IFF_RUNNING, "RUNNING");
  IF_FLAG_OUT (IFF_NOARP, "NOARP");
  IF_FLAG_OUT (IFF_PROMISC, "PROMISC");
  IF_FLAG_OUT (IFF_ALLMULTI, "ALLMULTI");
  IF_FLAG_OUT (IFF_MASTER, "IFF_MASTER");
  IF_FLAG_OUT (IFF_SLAVE, "IFF_SLAVE");
  IF_FLAG_OUT (IFF_MULTICAST, "MULTICAST");
  IF_FLAG_OUT (IFF_PORTSEL, "PORTSEL");
  IF_FLAG_OUT (IFF_AUTOMEDIA, "AUTOMEDIA");
  IF_FLAG_OUT (IFF_DYNAMIC, "DYNAMIC");
  /*
  IF_FLAG_OUT (IFF_LOWER_UP, "LOWER_UP");
  IF_FLAG_OUT (IFF_DORMANT, "DORMANT");
  */
  strPrintf("\n");
  return (Uint32T)(str - saveStr);
}




void printStrSwitchPort(StringT str, SwitchPortT* port)
{
	/* PhysicalPortT dump */
	strPrintf("\tSwitchPort:: \n");
	strPrintf("\tFwdState=%d, VlanMode=%s \n", port->fwdState, getVlanModeStr(port->vlanMode));
	strPrintf("\tAccessVid=%d, TrunkNative=%d \n", port->accessVid, port->trunkNativeVid);
	strPrintf("\ttrunkVlanAllowedMode=%d \n", port->trunkVlanAllowedMode);
	strPrintf("\ttrunkVlanAllowedVid= \n\t");
	Int32T i;
	for(i=0; i< PIF_VLAN_MAX+1; i++){
		if(port->trunkAllowedVid[i] == 1) {
			strPrintf("%d ", i);
		}
	}
}


void printPhysicalPort(PhysicalPortT* ifp)
{
	Int8T strBuff[1024] = {};
	printStrPhysicalPort(strBuff, ifp);
	pifLOG(LOG_DEBUG, "------------------------------------------\n");
	pifLOG(LOG_DEBUG, "%s", strBuff);
	pifLOG(LOG_DEBUG, "------------------------------------------\n");
}

void printStrPhysicalPort(StringT str, PhysicalPortT* ifp)
{
	/* PhysicalPortT dump */
	strPrintf("\tPhysicalPort:: %s \n", ifp->name);
	strPrintf("\tIid=%d, Ifidx=%d Bw=%d \n", ifp->iid.idx, ifp->ifIndex, ifp->bandwidth);
	strPrintf("\tAdmin=%s, Oper=%s \n", getIfStateStr(ifp->adminState), getIfStateStr(ifp->operState));
	strPrintf("\tHwType=%s, HwAddr=%s \n", getHwTypeStr(ifp->hwType), mac2str((char*)ifp->hwAddr));
	strPrintf("\tencap=%d, speed=%d duplex=%d, flowctrl=0\n", 
				ifp->encapType, ifp->speed, ifp->duplex);
	strPrintf("\tportMode=%s \n", getIfModeStr(ifp->portMode));
}


void printStrPhysicalPortSwitchPort(StringT str, PhysicalPortT* ifp)
{
	/* PhysicalPortT dump */
	strPrintf("Port:: %s \n", ifp->name);
	strPrintf("\tIid=%d, Ifidx=%d Bw=%d \n", ifp->iid.idx, ifp->ifIndex, ifp->bandwidth);
	strPrintf("\tAdmin=%s, Oper=%s \n", getIfStateStr(ifp->adminState), getIfStateStr(ifp->operState));
	strPrintf("\tHwType=%s, HwAddr=%s \n", getHwTypeStr(ifp->hwType), mac2str((char*)ifp->hwAddr));
	strPrintf("\tencap=%d, speed=%d duplex=%d, flowctrl=0\n", 
				ifp->encapType, ifp->speed, ifp->duplex);
	strPrintf("\tportMode=%s \n", getIfModeStr(ifp->portMode));

	SwitchPortT* port = &(ifp->switchPort);
	strPrintf("\tSwitchPort:: %s\n", (ifp->portMode == SWITCH_PORT)?"enabled":"disabled");
	strPrintf("\tFwdState=%d, VlanMode=%s \n", port->fwdState, getVlanModeStr(port->vlanMode));
	strPrintf("\tAccessVid=%d, TrunkNative=%d \n", port->accessVid, port->trunkNativeVid);
	strPrintf("\ttrunkVlanAllowedMode=%d \n", port->trunkVlanAllowedMode);
	strPrintf("\ttrunkVlanAllowedVid= \n\t");
	Int32T i;
	for(i=0; i< PIF_VLAN_MAX+1; i++){
		if(port->trunkAllowedVid[i] == 1) {
			strPrintf("%d ", i);
		}
	}
}



void dumpPhysicalPort(PhysicalPortT* ifp, StringT msg)
{
	/* IPInterface dump */
	pifLOG(LOG_DEBUG, "\n");
	pifLOG(LOG_DEBUG, "---------------------------------------------------------\n");
	pifLOG(LOG_DEBUG, "PhysicalLink %s %s\n", ifp->name, msg);
	pifLOG(LOG_DEBUG, "\tIid=%d, Ifidx=%d Bw=%d \n", ifp->iid.idx, ifp->ifIndex, ifp->bandwidth);
	pifLOG(LOG_DEBUG, "\tAdmin=%s, Oper=%s \n", getIfStateStr(ifp->adminState), getIfStateStr(ifp->operState));
	pifLOG(LOG_DEBUG, "\tHwType=%s, HwAddr=%s \n", getHwTypeStr(ifp->hwType), mac2str((char*)ifp->hwAddr));
	pifLOG(LOG_DEBUG, "\tencap=%d, speed=%d duplex=%d, flowctrl=0\n", 
											ifp->encapType, ifp->speed, ifp->duplex);
	pifLOG(LOG_DEBUG, "\tPortMode=%s \n", getIfModeStr(ifp->portMode));
	pifLOG(LOG_DEBUG, "---------------------------------------------------------\n");
	pifLOG(LOG_DEBUG, "\n");
}



void printAggGroup(AggGroupT* ifp)
{
	Int8T strBuff[1024] = {};
	printStrAggGroup(strBuff, ifp);
	pifLOG(LOG_DEBUG, "------------------------------------------\n");
	pifLOG(LOG_DEBUG, "%s", strBuff);
	pifLOG(LOG_DEBUG, "------------------------------------------\n");
}

void printStrAggGroup(StringT str, AggGroupT* ifp)
{
	strPrintf("\tAggGroup:: %s \n", ifp->name);
	strPrintf("\tIid=%d, Ifidx=%d Bw=%d \n", ifp->iid.idx, ifp->ifIndex, ifp->bandwidth);
	strPrintf("\tAdmin=%s, Oper=%s \n", getIfStateStr(ifp->adminState), getIfStateStr(ifp->operState));
	strPrintf("\tHwType=%s, HwAddr=%s \n", getHwTypeStr(ifp->hwType), mac2str((char*)ifp->hwAddr));
	strPrintf("\tEncap=%d, Speed=%d Duplex=%d, Flowctrl=0\n", 
				ifp->encapType, ifp->speed, ifp->duplex);
	strPrintf("\tportMode=%s \n", getIfModeStr(ifp->portMode));

	/* Port List dump */
	if(nnListCount(ifp->portList) == 0) {
		strPrintf("\tPort list= no port \n");
		return;
	}
	else {
		strPrintf("\tPort list: \n");
	}
	ListNodeT * listNode;
	PhysicalPortT *pPort;
	PLIST_LOOP(ifp->portList, pPort, listNode)
	{
		/* print port */
		strPrintf("\t\tName=%s, ifIndex=%d, portMode=%s \n", 
					pPort->name, pPort->ifIndex, 
					getIfModeStr(pPort->portMode));
	}
}

void dumpAggGroup(AggGroupT* ifp, StringT msg)
{
	pifLOG(LOG_DEBUG, "\n");
	pifLOG(LOG_DEBUG, "---------------------------------------------------------\n");
	pifLOG(LOG_DEBUG, "AggGroup %s %s\n", ifp->name, msg);
	pifLOG(LOG_DEBUG, "\tIid=%d, Ifidx=%d Bw=%d \n", ifp->iid.idx, ifp->ifIndex, ifp->bandwidth);
	pifLOG(LOG_DEBUG, "\tAdmin=%s, Oper=%s \n", getIfStateStr(ifp->adminState), getIfStateStr(ifp->operState));
	pifLOG(LOG_DEBUG, "\tHwType=%s, HwAddr=%s \n", getHwTypeStr(ifp->hwType), mac2str((char*)ifp->hwAddr));
	pifLOG(LOG_DEBUG, "\tEncap=%d, Speed=%d Duplex=%d, Flowctrl=0\n", 
											ifp->encapType, ifp->speed, ifp->duplex);
	pifLOG(LOG_DEBUG, "\tPortMode=%s \n", getIfModeStr(ifp->portMode));
	pifLOG(LOG_DEBUG, "\tisStatic=%d \n", ifp->staticFlag);

	/* Port List dump */
	if(nnListCount(ifp->portList) == 0) {
	pifLOG(LOG_DEBUG, "\tPort list= no port \n");
	pifLOG(LOG_DEBUG, "---------------------------------------------------------\n");
	pifLOG(LOG_DEBUG, "\n");
	return;
	}
	else {
	pifLOG(LOG_DEBUG, "\tPort list: \n");
	}
	ListNodeT * listNode;
	PhysicalPortT *pPort;
	PLIST_LOOP(ifp->portList, pPort, listNode)
	{
	/* print port */
	pifLOG(LOG_DEBUG, "\t\tName=%s, ifIndex=%d, portMode=%s \n", 
							pPort->name, pPort->ifIndex, getIfModeStr(pPort->portMode));
	}
	pifLOG(LOG_DEBUG, "---------------------------------------------------------\n");
	pifLOG(LOG_DEBUG, "\n");
}



void printStrIPInterface(StringT str, IPInterfaceT* ifp)
{
	/* IPInterface print */
	strPrintf("%s \n", ifp->name);
	strPrintf("\tiid=%d, ifidx=%d \n", ifp->iid.idx, ifp->ifIndex);
	strPrintf("\tAdmin=%s, Oper=%s, Flags=%08x \n", 
						getIfStateStr(ifp->adminState), 
						getIfStateStr(ifp->operState),
						ifp->flags);
	str += printStrIfFlag(str, ifp->flags);
	strPrintf("\tifType=%s, ifMode=%s \n",
			getIfTypeStr(ifp->ifType), getIfModeStr(ifp->ifMode));
	strPrintf("\tmetric=%d, mtu=%d, bw=%d \n",
			ifp->metric, ifp->mtu, ifp->bandwidth);

	/* ConnectedAddress dump */
	if(nnListCount(ifp->connectedAddrs) != 0) {
		ListNodeT * listNode;
		ConnectedAddressT *pAddr;
		Int8T strAddr[100];
		PLIST_LOOP(ifp->connectedAddrs, pAddr, listNode)
		{
			//if(pAddr->address)
			{
			 	memset(strAddr, 0, 100);
			 	nnCnvPrefixtoString(strAddr, &(pAddr->address));
			 	strPrintf("\taddr=%s \n", strAddr);
			}
		}
	}

	/* PhysicalPort dump if it's type is switchport*/
	if(ifp->ifMode == SWITCH_PORT) {
		if(ifp->attachedPort != NULL){
			PhysicalPortT* port = ifp->attachedPort;
			Int8T strBuff[1024] = {};
			printStrPhysicalPort(strBuff, port);
			strPrintf("%s", strBuff);
			SwitchPortT* switchPort = &(port->switchPort);
			printStrSwitchPort(str, switchPort);
		}
		else{
			strPrintf("\tPhysicalPort:: null \n");
		}
	}

}


void printStrIPInterfaceBrief(StringT str, IPInterfaceT* ifp)
{
	/* IPInterface print */
	strPrintf("%s \n", ifp->name);
	strPrintf("\tifidx=%d Admin=%s Oper=%s ifType=%s \n", 
						ifp->ifIndex,
						getIfStateStr(ifp->adminState), 
						getIfStateStr(ifp->operState),
						getIfTypeStr(ifp->ifType));

	/* ConnectedAddress dump */
	if(nnListCount(ifp->connectedAddrs) != 0) {
		ListNodeT * listNode;
		ConnectedAddressT *pAddr;
		Int8T strAddr[100];
		PLIST_LOOP(ifp->connectedAddrs, pAddr, listNode)
		{
			//if(pAddr->address)
			{
			 	memset(strAddr, 0, 100);
			 	nnCnvPrefixtoString(strAddr, &(pAddr->address));
			 	strPrintf("\taddr=%s \n", strAddr);
			}
		}
	}
}



void dumpIPInterface(IPInterfaceT* ifp, StringT str)
{
	/* IPInterface dump */
	pifLOG(LOG_DEBUG, "\n");
	pifLOG(LOG_DEBUG, "---------------------------------------------------------\n");
	pifLOG(LOG_DEBUG, "%s %s\n", ifp->name, str);
	pifLOG(LOG_DEBUG, "\tiid=%d, ifidx=%d \n", ifp->iid.idx, ifp->ifIndex);
	pifLOG(LOG_DEBUG, "\tAdmin=%s, Oper=%s \n", 
			getIfStateStr(ifp->adminState), getIfStateStr(ifp->operState));
	pifLOG(LOG_DEBUG, "\tifType=%s, ifMode=%s \n",
			getIfTypeStr(ifp->ifType), getIfModeStr(ifp->ifMode));
	pifLOG(LOG_DEBUG, "\tmetric=%d, mtu=%d, bw=%d \n",
			ifp->metric, ifp->mtu, ifp->bandwidth);

	/* ConnectedAddress dump */
	if(nnListCount(ifp->connectedAddrs) == 0) {
		pifLOG(LOG_DEBUG, "---------------------------------------------------------\n");
		pifLOG(LOG_DEBUG, "\n");
		return;
	}
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
	pifLOG(LOG_DEBUG, "---------------------------------------------------------\n");
	pifLOG(LOG_DEBUG, "\n");
}


void dumpConnectedAddress(IPInterfaceT* ifp, ConnectedAddressT* addr, StringT str)
{
	/* ConnectedAddressT dump */
	Int8T strAddr[100];
	nnCnvPrefixtoString(strAddr, &(addr->address));

	pifLOG(LOG_DEBUG, "\n");
	pifLOG(LOG_DEBUG, "---------------------------------------------------------\n");
	pifLOG(LOG_DEBUG, "%s %s\n", ifp->name, str);
	pifLOG(LOG_DEBUG, "\tAddr=%s, Flag=%d \n", strAddr, addr->flags);
	pifLOG(LOG_DEBUG, "---------------------------------------------------------\n");
	pifLOG(LOG_DEBUG, "\n");
}



void dumpRecursiveIPInterface(AvlNodeLongT* treeNode)
{
	if(treeNode == NULL) return;
	dumpIPInterface((IPInterfaceT*)treeNode->pData, "");
	dumpRecursiveIPInterface(treeNode->pLeft);
	dumpRecursiveIPInterface(treeNode->pRight);
}


void dumpIPInterfaceTable()
{
	AvlNodeLongT *treeRoot = getRootInterfaceTable();
	dumpRecursiveIPInterface(treeRoot);
}



void printStrVlan(StringT str, VlanT* vlan)
{
	/* Vlan dump */
	strPrintf("Vlan %d \n", vlan->vid);
	strPrintf("\tName=%s, mtu=%d \n", vlan->name, vlan->mtu);
	strPrintf("\tAdmin=%s, Oper=%s \n", 
			getIfStateStr(vlan->adminState), 
			getIfStateStr(vlan->operState));

	/* Port List dump */
	if(nnListCount(vlan->portList) == 0) {
		strPrintf("\tPort list= no port \n");
		return;
	}
	else {
		strPrintf("\tPort list: \n");
	}
	ListNodeT * listNode;
	PhysicalPortT *pPort;
	PLIST_LOOP(vlan->portList, pPort, listNode)
	{
		/* print port */
		strPrintf("\t\tName=%s, ifIndex=%d, %s \n", 
					pPort->name, pPort->ifIndex, 
					getVlanModeStr(pPort->switchPort.vlanMode));
	}
}





#define IF_TYPE_MAX     15
static const char *IFT_STR[] __attribute__ ((unused))  =
{
    "IF_UNKNOWN",
    "IF_LOOPBACK",
    "IF_ETHERNET",
    "IF_HDLC",
    "IF_PPP",
    "IF_ATM",
    "IF_FRELAY",
    "IF_VLAN",
    "IF_PORT",
    "IF_AGGREGATE",
    "IF_MANAGE",
    "IF_IPIP",
    "IF_GREIP",
    "IF_IPV6IP", 
    "IF_6TO4",
    "IF_ISATAP",
    "INVAILD"
};  
const char* getIfTypeStr(Int32T ifType)
{
	if(ifType > IF_TYPE_MAX) return IFT_STR[IF_TYPE_MAX+1];
	return IFT_STR[ifType];
}

#define IF_STAT_MAX     2
static const char *IFS_STR[] __attribute__ ((unused))  =
{
    "DOWN",
    "UP",
    "DISABLED",
    "INVAILD"
};  
const char* getIfStateStr(Int32T state)
{
	if(state > IF_STAT_MAX) return IFS_STR[IF_STAT_MAX+1];
	return IFS_STR[state];
}


#define HW_TYPE_MAX     2
static const char *HW_TYPE_STR[] __attribute__ ((unused))  =
{
    "FAST_ETHERNET",
    "GIGA_ETHERNET",
    "AGG_ETHERNET",
    "INVAILD"
};  
const char* getHwTypeStr(Int32T state)
{
	if(state > HW_TYPE_MAX) return HW_TYPE_STR[HW_TYPE_MAX+1];
	return HW_TYPE_STR[state];
}


#define IF_MODE_MAX     2
static const char *IFM_STR[] __attribute__ ((unused))  =
{
    "NOT_CONFIGURED",
    "SWITCH_PORT",
    "ROUTED_PORT",
    "INVAILD"
};  
const char* getIfModeStr(Int32T mode)
{
	if(mode > IF_MODE_MAX) return IFS_STR[IF_MODE_MAX+1];
	return IFM_STR[mode];
}



#define VLAN_MODE_MAX     2
static const char *VLANM_STR[] __attribute__ ((unused))  =
{
    "VLAN_NOT_CONFIGURED",
    "VLAN_MODE_ACCESS",
    "VLAN_MODE_TRUNK",
    "INVAILD"
};  

const char* getVlanModeStr(Int32T mode)
{
	if(mode > VLAN_MODE_MAX) return VLANM_STR[VLAN_MODE_MAX+1];
	return VLANM_STR[mode];
}


#define PORT_STATE_MAX     4
static const char *PORT_STATE_STR[] __attribute__ ((unused))  =
{
    "DISABLED",
    "LISTENING",
    "LEARNING",
    "FORWARDING"
    "BLOCKING"
    "INVAILD"
};  

const char* getPortStateStr(Int16T state)
{
	if(state > PORT_STATE_MAX) return PORT_STATE_STR[PORT_STATE_MAX+1];
	return PORT_STATE_STR[state];
}






#define BUF_STR_SIZE 100
static char bufs[BUF_STR_SIZE];

char* ip2str(uint32_t ip)
{
  memset(bufs, 0, 16);
  uint8_t     *p = (uint8_t *)&ip;
  sprintf(bufs, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
  return (bufs);
}

uint32_t str2ip(char *ipstr)
{
  uint32_t ip, p[4] = {0,0,0,0};
  sscanf(ipstr, "%d.%d.%d.%d", &p[0], &p[1], &p[2], &p[3]);
  ip = (p[0]&0xff)<<24 | (p[1]&0xff)<<16 | (p[2]&0xff)<<8 | (p[3]&0xff);
  return (ip);
}

char* mac2str(char *mac)
{
	memset(bufs, 0, BUF_STR_SIZE);
	sprintf(bufs, "%02x:%02x:%02x:%02x:%02x:%02x",
			(unsigned char)mac[0], (unsigned char)mac[1], 
			(unsigned char)mac[2], (unsigned char)mac[3], 
			(unsigned char)mac[4], (unsigned char)mac[5]);
	return (bufs);
}

char* str2mac(char* macstr)
{
	memset(bufs, 0, BUF_STR_SIZE);
	Uint32T  p[8] = {0,0,0,0,0,0};
	sscanf(macstr, "%x:%x:%x:%x:%x:%x", &p[0], &p[1], &p[2], &p[3], &p[4], &p[5]);
	bufs[0] = p[0] & 0xff;
	bufs[1] = p[1] & 0xff;
	bufs[2] = p[2] & 0xff;
	bufs[3] = p[3] & 0xff;
	bufs[4] = p[4] & 0xff;
	bufs[5] = p[5] & 0xff;
	return (bufs);
}



