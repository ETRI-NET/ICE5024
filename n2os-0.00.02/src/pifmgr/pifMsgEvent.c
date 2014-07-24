/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : pifmgr Post Event
 * - Block Name : PIF Manager
 * - Process Name : pifmgr
 * - Creator : Seungwoo Hong
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : pifMsgEvent.c
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
#include "pifMsgEventData.h" 
#include "pifDataTypes.h" 


#define IPv4_ADDR_SIZE	4
#define IPv6_ADDR_SIZE	16

void dumpVlanEvent(PifVlanEventMsgT* m);
void dumpPortEvent(PifPortEventMsgT* m);
void dumpIPInterfaceEvent(PifInterfaceEventMsgT* m);
void dumpIPInterfaceAddrEvent(PifInterfaceAddrEventMsgT* m);

const char* getPortEventIdStr(Int32T id);
const char* getIPEventIdStr(Int32T id);
const char* getVlanEventIdStr(Int32T id);


void pifPostEventPort(PhysicalPortT* port, Uint32T msgId)
{
	pifLOG(LOG_DEBUG, "EVENT-LINK :: %s(%s) \n", 
						getPortEventIdStr(msgId), port->name);

	if(port->portMode != SWITCH_PORT){
		pifLOG(LOG_DEBUG, "skep event. %s is not switchport \n", port->name);
		return;
	}

	/* make event message */
	PifPortEventMsgT msg;
	memset(&msg, 0, sizeof(PifPortEventMsgT));
	msg.eId = htonl(msgId);

	/* PhysicalPort info */
	strncpy(&msg.ifName[0], port->name, strlen(port->name));
	msg.iId = htonl(port->iid.idx);
	msg.ifIndex = htonl(port->ifIndex);
	msg.ifType = htons(port->hwType);
	
	msg.adminState = port->adminState;
	msg.operState = port->operState;

	/* bandwidth and mac-address */
	msg.bandwidth = htons(port->bandwidth);
	memcpy(&(msg.macAddress[0]), &(port->hwAddr[0]), ETH_ADDR_LEN);

	/* print debug */
	dumpPortEvent(&msg);

	/* publish event */
	eventPublish(EVENT_PIF_L2_LINK, &msg, sizeof(PifPortEventMsgT));
}


void pifPostEventVlan(VlanT* vlan, PhysicalPortT* port, Uint32T msgId)
{
	pifLOG(LOG_DEBUG, "EVENT-VLAN :: \n");

	/* make event message */
	PifVlanEventMsgT msg;
	memset(&msg, 0, sizeof(PifVlanEventMsgT));
	msg.eId = htonl(msgId);

	/* Vlan info */
	if(vlan != NULL) {
		strncpy(msg.vlanName, vlan->name, strlen(vlan->name));
		msg.vId = htonl(vlan->vid);
	}

	/* Port info */
	if(port != NULL) {
		msg.iId = htonl(port->iid.idx);
		msg.ifIndex = htonl(port->ifIndex);
		msg.portMode = htonl(port->switchPort.vlanMode);
	}

	/* print debug */
	dumpVlanEvent(&msg);

	/* publish event */
	eventPublish(EVENT_PIF_L2_VLAN, &msg, sizeof(PifVlanEventMsgT));
}


void pifPostEventAgg(AggGroupT* agg, Uint32T msgId)
{
	pifLOG(LOG_DEBUG, "EVENT-LINK :: \n");
}


void pifPostEventIPAddr(IPInterfaceT* ifp, ConnectedAddressT* conAddr, Uint32T msgId)
{
	pifLOG(LOG_DEBUG, "EVENT :: %s\n", getIPEventIdStr(msgId));

	/* make event message */
	PifInterfaceAddrEventMsgT msg;
	memset(&msg, 0, sizeof(PifInterfaceAddrEventMsgT));

	/* Event message id */
	msg.eId = htonl(msgId);

	/* IPInterface info */
	strncpy(msg.ifName, ifp->name, strlen(ifp->name));
	msg.iId = htonl(ifp->iid.idx);
	msg.ifIndex = htonl(ifp->ifIndex);

	/* ConnectedAddress info */
	msg.flags = htonl(conAddr->flags);

	PrefixT* address = &(conAddr->address);
	PrefixT* broadcast = &(conAddr->broadcast);
	Uint8T  addrFamily = PREFIX_FAMILY(address);
	Uint32T addrSize = (addrFamily == AF_INET)? IPv4_ADDR_SIZE: IPv6_ADDR_SIZE;

	msg.family = address->family;
	msg.prefixLen = htons(address->prefixLen);

	/* address */
	Uint8T* vAddr = &(msg.variableAddrBuf[0]);
	memcpy(vAddr, &(address->u.prefix), addrSize);

	/* broadcast */
	vAddr = vAddr + addrSize;
	memcpy(vAddr, &(broadcast->u.prefix), addrSize);

	/* print debug */
	dumpIPInterfaceAddrEvent(&msg);

	/* publish event */
	/*
	Uint32T msgLen = sizeof(PifInterfaceAddrEventMsgT) - VARIABLE_ADDR_SIZE;   
	msgLen += (addrSize * 2);
	*/
	Uint32T msgLen = sizeof(PifInterfaceAddrEventMsgT);   
	pifLOG(LOG_DEBUG, "EVENT :: publish msg(%p) size(%d)\n", &msg, msgLen);
	//eventPublish(EVENT_PIF_L3_INTERFACE, &msg, msgLen);

	pifLOG(LOG_DEBUG, "EVENT :: end\n");
}


void pifPostEventIPInterface(IPInterfaceT* ifp, Uint32T msgId)
{
	pifLOG(LOG_DEBUG, "EVENT :: %s\n", getIPEventIdStr(msgId));

	/* make event message */
	PifInterfaceEventMsgT msg;
	memset(&msg, 0, sizeof(PifInterfaceEventMsgT));
	msg.eId = htonl(msgId);

	/* IPInterface info */
	strncpy(msg.ifName, ifp->name, strlen(ifp->name));
	msg.iId = htonl(ifp->iid.idx);
	msg.ifIndex = htonl(ifp->ifIndex);
	msg.ifType = htons(ifp->ifType);
	
	msg.adminState = ifp->adminState;
	msg.operState = ifp->operState;

	msg.flags = ifp->flags;

	msg.metric = htonl(ifp->metric);
	msg.mtu = htonl(ifp->mtu);
	msg.bandwidth = htonl(ifp->bandwidth);
	/* hwAddr need to check */
	// memcpy(msg.hwAddr, port->hwAddr, IF_HWADDR_MAX);

	/* print debug */
	dumpIPInterfaceEvent(&msg);

	/* publish event */
	eventPublish(EVENT_PIF_L3_INTERFACE, &msg, sizeof(msg));
}


#define VLAN_EVT_MAX	4
static const char *VLAN_EVT_STR[] __attribute__ ((unused))  =
{
	"PIF_EVENT_VLAN_ADD",
	"PIF_EVENT_VLAN_DEL",
	"PIF_EVENT_VLAN_PORT_MODE",
	"PIF_EVENT_VLAN_PORT_ADD",
	"PIF_EVENT_VLAN_PORT_DEL",
	"INVALID"
};

const char* getVlanEventIdStr(Int32T id)
{
	id = id - PIF_EVENT_VLAN_BASE;
	if(id > VLAN_EVT_MAX) return VLAN_EVT_STR[VLAN_EVT_MAX+1];
	return VLAN_EVT_STR[id];
}

void dumpVlanEvent(PifVlanEventMsgT* m)
{
	pifLOG(LOG_DEBUG, "\n");
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " EVT: %40s\n", getVlanEventIdStr(ntohl(m->eId)));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " VLN: %30s\n", m->vlanName);
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " VID: %30d\n", ntohl(m->vId));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " IID: %30X\n", ntohl(m->iId));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " IDX: %30X\n", ntohl(m->ifIndex));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " MOD: %35s\n", getVlanModeStr(ntohl(m->portMode)));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, "\n");
}


#define PORT_EVT_MAX	3
static const char *PORT_EVT_STR[] __attribute__ ((unused))  =
{
	"PIF_EVENT_LINK_ADD",
	"PIF_EVENT_LINK_DEL",
	"PIF_EVENT_LINK_UP",
	"PIF_EVENT_LINK_DOWN",
	"INVALID"
};

const char* getPortEventIdStr(Int32T id)
{
	id = id - PIF_EVENT_LINK_BASE;
	if(id > PORT_EVT_MAX) return PORT_EVT_STR[PORT_EVT_MAX+1];
	return PORT_EVT_STR[id];
}

void dumpPortEvent(PifPortEventMsgT* m)
{
	pifLOG(LOG_DEBUG, "\n");
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " EVT: %40s\n", getPortEventIdStr(ntohl(m->eId)));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " IFN: %30s\n", m->ifName);
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " IID: %30X\n", ntohl(m->iId));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " IDX: %30X\n", ntohl(m->ifIndex));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " IFT: %20s | Admin(%s) | Oper(%s)\n", 
							getHwTypeStr(ntohs(m->ifType)), 
							getIfStateStr(m->adminState), 
							getIfStateStr(m->operState));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " BW : %20X | %30s\n", ntohl(m->bandwidth), mac2str(m->macAddress));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, "\n");
}


#define IPIF_EVT_MAX	5
static const char *IPIF_EVT_STR[] __attribute__ ((unused))  =
{
	"PIF_EVENT_INTERFACE_ADD",
	"PIF_EVENT_INTERFACE_DEL",
	"PIF_EVENT_INTERFACE_UP",
	"PIF_EVENT_INTERFACE_DOWN",
	"PIF_EVENT_INTERFACE_ADDRES_ADD",
	"PIF_EVENT_INTERFACE_ADDRESS_DEL",
	"INVALID"
};

const char* getIPEventIdStr(Int32T id)
{
	id = id - PIF_EVENT_INTERFACE_BASE;
	if(id > IPIF_EVT_MAX) return IPIF_EVT_STR[IPIF_EVT_MAX+1];
	return IPIF_EVT_STR[id];
}

void dumpIPInterfaceEvent(PifInterfaceEventMsgT* m)
{
	pifLOG(LOG_DEBUG, "\n");
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " EVT: %40s\n", getIPEventIdStr(ntohl(m->eId)));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " IFN: %30s\n", m->ifName);
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " IID: %30X\n", ntohl(m->iId));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " IDX: %30X\n", ntohl(m->ifIndex));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " IFT: %20s | Admin(%s) | Oper(%s)\n", 
							getIfTypeStr(ntohs(m->ifType)), 
							getIfStateStr(m->adminState), 
							getIfStateStr(m->operState));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " FLG: %30X\n", ntohl(m->flags));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " MTR: %30d\n", ntohl(m->metric));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " MTU: %30d\n", ntohl(m->mtu));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " BW : %30d\n", ntohl(m->bandwidth));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, "\n");
}

static const char *ADDR_FAMILY[] __attribute__ ((unused))  =
{
	"AF_INET",
	"AF_INET6",
	"INVALID"
};

const char* getAddrFamilyStr(Int32T af)
{
	const char* str = (af == AF_INET) ? ADDR_FAMILY[0] : ADDR_FAMILY[1];
	return str;
}

void dumpIPInterfaceAddrEvent(PifInterfaceAddrEventMsgT* m)
{
	pifLOG(LOG_DEBUG, "\n");
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " EVT: %40s\n", getIPEventIdStr(ntohl(m->eId)));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " IFN: %30s\n", m->ifName);
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " IID: %30X\n", ntohl(m->iId));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " IDX: %30X\n", ntohl(m->ifIndex));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " |       Flag(%02X)     |       %s       |       PL(%d)\n", 
							m->flags, 
							getAddrFamilyStr(m->family), 
							ntohs(m->prefixLen));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");

	char addrStr[INET6_ADDRSTRLEN];
	char broadStr[INET6_ADDRSTRLEN];
	memset(addrStr, 0, INET6_ADDRSTRLEN);
	memset(broadStr, 0, INET6_ADDRSTRLEN);
	Uint8T*  variableAddrBuf =  m->variableAddrBuf;

	if(m->family == AF_INET){
		sprintf(addrStr, "%s", (char*)inet_ntoa(*((struct in_addr *)variableAddrBuf)));
		variableAddrBuf += 4;
		sprintf(broadStr, "%s", (char*)inet_ntoa(*((struct in_addr *)variableAddrBuf)));
	}
	else {
		inet_ntop(AF_INET6, (struct in6_addr *)variableAddrBuf, addrStr, INET6_ADDRSTRLEN);
		variableAddrBuf += 16;
		inet_ntop(AF_INET6, (struct in6_addr *)variableAddrBuf, broadStr, INET6_ADDRSTRLEN);
	}

	pifLOG(LOG_DEBUG, " |      %30s\n", addrStr);
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " |      %30s\n", broadStr);
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, "\n");
	pifLOG(LOG_DEBUG, "\n");
}




