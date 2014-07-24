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
#include "pifMsg.h" 
#include "pifIf.h" 


//static variable for ipc return
//static	Int8T cmiRltStr[PIF_RLT_STR_SIZE];
//static  cmdSetResT cmiRltRes; 
static	PifIpcMsgT ipcMsg;

//component ipc process
void pifIpcProcess(Int32T msgId, void * data, Uint32T dataLen)
{
	if(msgId > PIF_IPC_MAP_MAX) return;
	memcpy((void*)&ipcMsg, data, dataLen);
	pifIpcWorkFuction[msgId].func ((void*)ipcMsg.data);
}

//component event process
void pifEventProcess(Int32T msgId, void * data, Uint32T dataLen)
{
	//do anything
}


/* CLI: Interface */
inline static void pifCmiRB()
{
/*
    memcpy((void*)&cmiRltRes, (void*)&ipcMsg, PIF_IPC_HEADER_SIZE);
    cmiRltRes.result = CMD_SET_WARNING;
    sprintf ((char *)cmiRltRes.reason, "%s\n", cmiRltStr);
    nosIpcSendAsync(COMMAND_MANAGER, 0, &cmiRltRes, sizeof(cmdSetResT));
*/
}

#define PIF_PROC_CMD_RETURN() pifCmiRB() 
void pifCmiIfSet(void* msgBuff) 
{
/*
	CmiInterfaceMsgT *msg = (CmiInterfaceMsgT*)msgBuff;
	printf("   - INTERFACE SET: In(%s)\n", msg->ifName);

	Int32T rlt =  pifInterfaceConfigLU(msg->ifName);

	pifInterfaceStr(cmiRltStr, rlt);
	PIF_PROC_CMD_RETURN();
*/
}

void pifCmiIfShow(void* msgBuff) 
{
/*
	CmiInterfaceMsgT *msg = (CmiInterfaceMsgT*)msgBuff;
	printf("   - INTERFACE SHOW: In(%s)\n", msg->ifName);

	PifCmiInterfaceT* ifData =  pifInterfaceConfigGetLU(msg->ifName);

	sprintf(cmiRltStr, "Interface-Name: %s", ifData->ifName);
	PIF_PROC_CMD_RETURN();
*/
}


void pifCmiIfAdminUnset(void* msg){}
void pifCmiIfAdminSet(void* msg){} 
void pifCmiIfSwportUnset(void* msg){} 
void pifCmiIfSwportset(void* msg){} 
void pifCmiIfSpeedSet(void* msg){}
void pifCmiIfDuplexSet(void* msg){}
void pifCmiIfFlowCtrlSet(void* msg){}
void pifCmiIfAddressSet(void* msg){}
void pifCmiIfStatusGet(void* msg){}
void pifCmiIfStatusDetailGet(void* msg){}
void pifCmiIfSwportStatusGet(void* msg){}

/* CLI: VLAN */
void pifCmiVlanPortModeSet(void *msg){}
void pifCmiVlanAccessVidAdd(void *msg){}
void pifCmiVlanAccessVidDel(void *msg){}
void pifCmiVlanTrunkVidAdd(void *msg){}
void pifCmiVlanTrunkVidDel(void *msg){}
void pifCmiVlanTrunkNativeSet(void *msg){}
void pifCmiVlanDatabaseAdd(void *msg){}
void pifCmiVlanDatabaseDel(void *msg){}

/* CLI: MAC */
void pifCmiMacAgingSet(void *msg){}
void pifCmiMacDynamicClearByAddr(void *msg){}
void pifCmiMacDynamicClearByPort(void *msg){}
void pifCmiMacDynamicClearByVlan(void *msg){}
void pifCmiMacStaticAddrAdd(void *msg){}
void pifCmiMacStaticAddrDel(void *msg){}
void pifCmiMacTableGetAddr(void* msg){}
void pifCmiMacTableGetAging(void* msg){}
void pifCmiMacTableGetCount(void* msg){}
void pifCmiMacTableGetAll(void* msg){}
void pifCmiMacTableGetPort(void* msg){}
void pifCmiMacTableGetVlan(void* msg){}
void pifCmiMacTableGetStatic(void* msg){}

/* CLI: MAC ACL */
void pifCmiMacAclTermAdd(void *msg){}
void pifCmiMacAclTermDel(void *msg){}
void pifCmiMacAclPortAdd(void *msg){}

//Protocols (STP/LACP)
void PifPiStpPortStateSet(void *msg){}
void PifPiLacpAggrateAdd(void *msg){}
void PifPiLacpAggrateDel(void *msg){}
void PifPiLacpAttachMuxToAgg(void *msg){}
void PifPiLacpDetachMuxToAgg(void *msg){}
void PifPiLacpCollectingEnable(void *msg){}
void PifPiLacpCollectingDisable(void *msg){}
void PifPiLacpDistributingEnable(void *msg){}
void PifPiLacpDistributingDisable(void *msg){}
void PifPiLacpLoadBalanceSet(void *msg){}
void PifPiLacpPortStatusGet(void *msg){}
