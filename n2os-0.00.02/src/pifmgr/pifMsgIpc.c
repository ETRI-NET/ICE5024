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

#include "pif.h" 
#include "nosLib.h" 
#include "pifMsgIpc.h" 
#include "pifDataTypes.h" 

#include "pifPortMgr.h" 
#include "pifVlanMgr.h" 
#include "pifInterfaceMgr.h" 
#include "pifAggMgr.h" 

/* Process STP IPC request  */
void PifPiStpPortStateSet(PiStpMsgT *msg);

/* Process LACP IPC request  */
void PifPiLacpAggrateAdd(PiLacpMsgT *msg);
void PifPiLacpAggrateDel(PiLacpMsgT *msg);
void PifPiLacpAttachMuxToAgg(PiLacpMsgT *msg);
void PifPiLacpDetachMuxToAgg(PiLacpMsgT *msg);
/* Blowe functions could be delayed */
void PifPiLacpCollectingEnable(void *msg);
void PifPiLacpCollectingDisable(void *msg);
void PifPiLacpDistributingEnable(void *msg);
void PifPiLacpDistributingDisable(void *msg);
void PifPiLacpLoadBalanceSet(void *msg);
void PifPiLacpPortStatusGet(void *msg);

/* Process L3 interface refresh */
void PifPiInterfaceRefresh(PiInterfaceRefleshMsgT *msg);

/* Debug Ipc messages */
void dumpStpIpcMessage(PiStpMsgT *m);
void dumpLacpIpcMessage(PiLacpMsgT *m);
void dumpL3IpcMessageReq(PiInterfaceRefleshMsgT *m);
void dumpL3IpcMessageRes(PiInterfaceRefleshMsgT *m);



void pifIpcProcess(Int32T msgId, void * data, Uint32T dataLen)
{
	pifLOG(LOG_DEBUG, "PIF-IPC :: \n");

	switch(msgId){
		/* STP */
		case PIF_xSTP_PORT_STATE_SET: 
			PifPiStpPortStateSet(data);
			break;
		/* LACP */
		case PIF_LACP_AGGREGATE_ADD: 
			PifPiLacpAggrateAdd(data);
			break;
		case PIF_LACP_AGGREGATE_DEL: 
			PifPiLacpAggrateDel(data);
			break;
		case PIF_LACP_ADD_PORT_TO_AGG: 
			PifPiLacpAttachMuxToAgg(data);
			break;
		case PIF_LACP_DEL_PORT_TO_AGG: 
			PifPiLacpDetachMuxToAgg(data);
			break;
		/* INTERFACE REFRESH */
		case PIF_INTERFACE_REFRESH_REQ: 
			PifPiInterfaceRefresh(data);
			break;
		default:
			break;
	}
}

void pifEventProcess(Int32T msgId, void * data, Uint32T dataLen)
{
	pifLOG(LOG_DEBUG, "PIF-EVT-RECEIVE :: \n");
}


/*********************************************
* Protocols (STP)
*********************************************/
void PifPiStpPortStateSet(PiStpMsgT *msg)
{
	pifLOG(LOG_DEBUG, "PIF-STP \n");
	dumpStpIpcMessage((PiStpMsgT*)msg);

	/* get port instance */
	Int32T iid = ntohl(msg->iid);
	PhysicalPortT *port = pifPortMgrGetPort(iid);
	if(port == NULL) {
		pifLOG(LOG_DEBUG, "PIF-STP  port(%d) not exist\n", iid);
		return;
	}

	/* get port state */
	Int16T state = ntohs(msg->state);

	/* get instance */
	Int16T instance = ntohs(msg->instance);

	/* send request to pifPortMgr */
	pifPortMgrSetPortFwdState(port, instance, state);
}



/*********************************************
* Protocols (LACP)
*********************************************/
void PifPiLacpAggrateAdd(PiLacpMsgT *msg)
{
	pifLOG(LOG_DEBUG, "PIF-LACP \n");
	dumpLacpIpcMessage((PiLacpMsgT*)msg);

	/* send request to pifAggMgr */
	StringT aggName = msg->aggName;
	Int32T aggId = ntohl(msg->aggId);
	Int8T* macAddr = msg->macAddr;
	//Int16T aggMode = ntohs(msg->aggMode); 
	Int16T portMode = SWITCH_PORT; 

	pifAggMgrAddGroup(aggName, aggId, macAddr, portMode);
}

void PifPiLacpAggrateDel(PiLacpMsgT *msg)
{
	pifLOG(LOG_DEBUG, "PIF-LACP \n");
	dumpLacpIpcMessage((PiLacpMsgT*)msg);

	/* send request to pifAggMgr */
	StringT aggName = msg->aggName;

	pifAggMgrDeleteGroup(aggName);
}

void PifPiLacpAttachMuxToAgg(PiLacpMsgT *msg)
{
	pifLOG(LOG_DEBUG, "PIF-LACP \n");
	dumpLacpIpcMessage((PiLacpMsgT*)msg);

	/* send request to pifAggMgr */
	StringT aggName = msg->aggName;

	int i, cnt =  ntohl(msg->portCount);
	Int32T iid, ifIndex;
	for(i=0; i<cnt; i++){
		iid = ntohl(msg->portIdList[i]);
		ifIndex =  ntohl(msg->portIfIdxList[i]);
		pifAggMgrAddPortMember(aggName, iid, ifIndex);
	}
}

void PifPiLacpDetachMuxToAgg(PiLacpMsgT *msg)
{
	pifLOG(LOG_DEBUG, "PIF-LACP \n");
	dumpLacpIpcMessage((PiLacpMsgT*)msg);

	/* send request to pifAggMgr */
	StringT aggName = msg->aggName;

	int i, cnt =  ntohl(msg->portCount);
	Int32T iid, ifIndex;
	for(i=0; i<cnt; i++){
		iid = ntohl(msg->portIdList[i]);
		ifIndex =  ntohl(msg->portIfIdxList[i]);
		pifAggMgrDeletePortMember(aggName, iid, ifIndex);
	}
}

void PifPiLacpCollectingEnable(void *msg){}
void PifPiLacpCollectingDisable(void *msg){}
void PifPiLacpDistributingEnable(void *msg){}
void PifPiLacpDistributingDisable(void *msg){}
void PifPiLacpLoadBalanceSet(void *msg){}
void PifPiLacpPortStatusGet(void *msg){}


/*********************************************
 * Protocols (L3 interface)
 *********************************************/
void setInterfaceMessage(PifInterfacetMsgT* msg, AvlNodeLongT* treeNode);

void PifPiInterfaceRefresh(PiInterfaceRefleshMsgT *msqReq)
{
	pifLOG(LOG_DEBUG, "PIF-L3-REFRESH` \n");
	dumpL3IpcMessageReq((PiInterfaceRefleshMsgT*)msqReq);

	/* make return ipc message */
	PiInterfaceRefleshMsgT msg;
	memset(&msg, 0, sizeof(PifInterfaceEventMsgT));
	msg.compId = msqReq->compId; 
	msg.ifCount = htonl(getSizeInterfaceTable());

	AvlNodeLongT *treeRoot = getRootInterfaceTable();
	setInterfaceMessage(&(msg.ifList[0]), treeRoot);

	/* send ipc message */
	ipcSendAsync(ntohl(msg.compId), PIF_INTERFACE_REFRESH_RES,
			     &msg, sizeof(PiInterfaceRefleshMsgT));

}

void setInterfaceMessage(PifInterfacetMsgT* msg, AvlNodeLongT* treeNode)
{
	if(treeNode == NULL) return;

	/* set each interface */
	 IPInterfaceT* ifp = (IPInterfaceT*)treeNode->pData;

	/* IPInterface */
	strncpy((*msg).ifName, ifp->name, strlen(ifp->name));
	(*msg).iId = htonl(ifp->iid.idx);
	(*msg).ifIndex = htonl(ifp->ifIndex);
	(*msg).ifType = htons(ifp->ifType);
	(*msg).adminState = ifp->adminState;
	(*msg).operState = ifp->operState;
	(*msg).flags = ifp->flags;
	(*msg).metric = htonl(ifp->metric);
	(*msg).mtu = htonl(ifp->mtu);
	(*msg).bandwidth = htonl(ifp->bandwidth);
	// memcpy((*msg).hwAddr, ifp->hwAddr, IF_HWADDR_MAX);

	/* ConnectedAddress list */
	(*msg).addrCount = htonl(nnListCount(ifp->connectedAddrs)); 
	ListNodeT * listNode;
	ConnectedAddressT *pAddr;
	Int32T idx = 0;
	PLIST_LOOP(ifp->connectedAddrs, pAddr, listNode)
	{
		 //if(pAddr->address)
		 {
			 PrefixT *addr = &(pAddr->address);
			 (*msg).prefix[idx].family = addr->family;
			 (*msg).prefix[idx].prefixLen = htons(addr->prefixLen);
			 memcpy((*msg).prefix[idx].variableAddrBuf, &(addr->u), 32);
			 idx++;
		 }
	}

	/* set recursive */
	setInterfaceMessage(msg+1, treeNode->pLeft);
	setInterfaceMessage(msg+1, treeNode->pRight);
}






/*********************************************
 * Debug Ipc Message
 *********************************************/
void dumpStpIpcMessage(PiStpMsgT *m)
{
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " IID: %d %40s\n", ntohl(m->iid), getIidStr(ntohl(m->iid)));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " IDX: %30X\n", ntohl(m->ifIndex));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " STA: %30d\n", ntohs(m->state));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " INS: %30d\n", ntohs(m->instance));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
}

void dumpLacpIpcMessage(PiLacpMsgT *m)
{
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " AGG: %40s\n", m->aggName);
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " AID: %30d\n", ntohl(m->aggId));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " AGG-MODE: %10d |  LB-MODE: %10d", 
										ntohs(m->aggMode), ntohs(m->aggMode));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " MAC: %30s\n", mac2str(m->macAddr));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " CNT: %30d\n", ntohl(m->portCount));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	
	int i, cnt =  ntohl(m->portCount);
	for(i = 0; i<cnt; i++){
	pifLOG(LOG_DEBUG, " 	PORT(IID,IDX): %20X, %20d \n", 
							ntohl(m->portIdList[i]), ntohl(m->portIfIdxList[i]));
	}
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");

}


void dumpL3IpcMessageReq(PiInterfaceRefleshMsgT *m)
{
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " CID: %d \n", ntohl(m->compId));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " CNT: %30X\n", ntohl(m->ifCount));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
}

void dumpL3IpcMessageRes(PiInterfaceRefleshMsgT *m)
{
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " CID: %d \n", ntohl(m->compId));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	pifLOG(LOG_DEBUG, " CNT: %30X\n", ntohl(m->ifCount));
	pifLOG(LOG_DEBUG, " +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
}

