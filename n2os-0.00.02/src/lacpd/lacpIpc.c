/********************************************************************************
 *                                  INCLUDE FILES 
 * ********************************************************************************/
#include "nnTypes.h"
#include "nnStr.h"

#include "nnCmdLink.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"
#include "nnCmdInstall.h"
#include "nosLib.h"
#include "nnCmdDefines.h"
#include "nnUtility.h"
#include "nnList.h"

#if 0
#include "lcsService.h"
#endif

#include "nnBuffer.h"
#include "nnPrefix.h"

#include "pifMsgEventData.h"
#include "pifMsgIpc.h"

#include "lacpDef.h"

#define	ipcSendAsyncPif(m)	\
	ipcSendAsync(PORT_INTERFACE_MANAGER, (m), msgBuff.data, msgBuff.length)

static void lacpToPifReport (Int32T groupId, Int32T msgId)
{
	Int8T str[32];
	Int32T i;
	nnBufferT msgBuff; //PiLacpMsgT
	int	count;
	LacpPortT *pPort, *pPortTmp;
	Int8T macAddr[6];

	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	count = getLacpConfig (groupId, &pPort);
	if (count <= 0)
		return;

	nnBufferReset(&msgBuff);
	sprintf (str, "port-channel %d", groupId);
	nnBufferSetString(&msgBuff, str, sizeof(str));		//aggName
	nnBufferSetInt32T(&msgBuff, groupId);				//aggId
	nnBufferSetInt16T(&msgBuff, LACP_AGGR_MODE_BASE);	//aggMode
	nnBufferSetInt16T(&msgBuff, LACP_LB_MODE_SRC_MAC);	//lbMode
	if (lacpGetRunGroupHwAddr(groupId, macAddr) < 0) {
		Int32T x = time(NULL);
		macAddr[0] = 0x02;
		macAddr[1] = 0;
		macAddr[2] = x;
		macAddr[3] = x >> 8;
		macAddr[4] = x >> 16;
		macAddr[5] = x >> 24;
	}
	nnBufferSetString(&msgBuff, macAddr, 6);			//macAddr
	nnBufferSetInt16T(&msgBuff, 0);						//align

	nnBufferSetInt32T(&msgBuff, count);					//portCount
	pPortTmp = pPort;
	for (i = 0; i < count; i++) {
		nnBufferSetInt32T(&msgBuff, count);				//portIdList[] ???
		nnBufferSetInt32T(&msgBuff, pPort->ifIndex);	//portIfIdxList[]
		pPortTmp++;
	}

	DEBUGPRINT("==>> %s %d... %d(g=%d)\n", __FUNCTION__, __LINE__, msgId, groupId);
	ipcSendAsyncPif(msgId);

	NNFREE (MEM_GLOBAL, pPort);
}

void lacpToPifReportAggregatorAdd (Int32T groupId)
{
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	DEBUGPRINT("==>> %s %d... %d\n", __FUNCTION__, __LINE__, groupId);
	return lacpToPifReport (groupId, PIF_LACP_AGGREGATE_ADD);
}

void lacpToPifReportAggregatorDelete (Int32T groupId)
{
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	DEBUGPRINT("==>> %s %d... %d\n", __FUNCTION__, __LINE__, groupId);
	return lacpToPifReport (groupId, PIF_LACP_AGGREGATE_DEL);
}

void lacpToPifReportPortAdd (Int32T groupId)
{
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	DEBUGPRINT("==>> %s %d... %d\n", __FUNCTION__, __LINE__, groupId);
	return lacpToPifReport (groupId, PIF_LACP_ADD_PORT_TO_AGG);
}

void lacpToPifReportPortDelete (Int32T groupId)
{
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	DEBUGPRINT("==>> %s %d... %d\n", __FUNCTION__, __LINE__, groupId);
	lacpToPifReport (groupId, PIF_LACP_DEL_PORT_TO_AGG);
}

#if 0
/*
 * Lcs component's role assigned function.
 */
void
lacpLcsSetRole (void * msg, Uint32T msgLen)
{
	Int32T ret = 0;
	DEBUGPRINT ("lacp : LCS %s called.\n", __FUNCTION__);

	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	LcsSetRoleMsgT lcsSetRoleMsg = {0,};

	/* Message copy. */
	memcpy(&lcsSetRoleMsg, msg, sizeof(LcsSetRoleMsgT));

	/* Role 이 LCS_HA_QUIESCING 인 경우 Service 중지. */
	if (lcsSetRoleMsg.haState == LCS_HA_QUIESCING) {
		DEBUGPRINT ("lacp : LCS lcsSetRoleMsg.haState == LCS_HA_QUIESCING \n");
	}

	/* Process Manager 에게 응답 수행. */
	ret = lcsResponse(lcsSetRoleMsg.processType, lcsSetRoleMsg.invocationId);
	if (ret == LCS_OK) {
		DEBUGPRINT ("lacp : LCS lcsResponse Success \n");
	}
	else {
		DEBUGPRINT ("lacp : LCS lcsResponse Failure \n");
	}

}

/*
 * Lcs component's terminate requested function.
 */
void
lacpLcsTerminate (void * msg, Uint32T msgLen)
{
	Int32T ret = 0;

	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	DEBUGPRINT ("lacp : LCS %s called.\n", __FUNCTION__);

	LcsTerminateMsgT lcsTerminateMsg = {0,};

	/* Message copy. */
	memcpy(&lcsTerminateMsg, msg, sizeof(LcsTerminateMsgT));

	/* Process Manager 에게 응답 수행. */
	ret = lcsResponse(lcsTerminateMsg.processType, lcsTerminateMsg.invocationId); 
	if (ret == LCS_OK) {
		DEBUGPRINT ("lacp : LCS lcsResponse Success \n");
	}
	else {
		DEBUGPRINT ("lacp : LCS lcsResponse Failure \n");
	}

	lacpTermProcess ();
}

/*
 * Lcs component's health check requested function.
 */
void
lacpLcsHealthcheck (void * msg, Uint32T msgLen)
{
	Int32T ret;

	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	DEBUGPRINT ("lacp : LCS %s called.\n", __FUNCTION__);

	LcsHealthcheckRequestMsgT healthcheck = {0,};

	/* Message copy. */
	memcpy(&healthcheck, msg, sizeof(LcsHealthcheckRequestMsgT));

	/* Process Manager 에게 Response Message 전송 */
	ret = lcsResponse(healthcheck.processType, healthcheck.invocationId);
	if (ret == LCS_OK) {
		DEBUGPRINT ("lacp : LCS lcsResponse Success \n");
	}
	else {
		DEBUGPRINT ("lacp : LCS lcsResponse Failure \n");
	}

	/* health check 수행. To do. */
	/*
	* If some problems are detected. send
	* lcsErrorReport(LCS_POLICY_MANAGER, LCS_ERR_BAD_OPERATION, LCS_COMPONENT_RESTART);
	*/ 
}


/*
 * Lcs : component's error was occurred, this event will be received.
 */
void
lacpLcsEventComponentErrorOccured (void * msg, Uint32T msgLen)
{
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	DEBUGPRINT ("lacp : LCS %s called.\n", __FUNCTION__);

	LcsErrorOccurredEventT errorOccurredEvent = {0,};

	/* Message copy. */
	memcpy(&errorOccurredEvent, msg, sizeof(errorOccurredEvent));

	/* Log error occurred component. */
	DEBUGPRINT ("lacp : LCS error component = %d\n", errorOccurredEvent.processType);
}


/*
 * Lcs : Component 의 Service 동작 상태를 전달
 */
void
lacpLcsEventComponentServiceStatus (void * msg, Uint32T msgLen)
{
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	DEBUGPRINT ("lacp : LCS %s called.\n", __FUNCTION__);

	LcsServiceStatusEventT serviceStatusEvent = {0,};

	/* Message copy. */
	memcpy(&serviceStatusEvent, msg, sizeof(LcsServiceStatusEventT));

	/* Log component's service. */
	DEBUGPRINT ("lacp : LCS service status = %llu\n", serviceStatusEvent.serviceStatus);
}
#endif

/*
 * Description : IPC 메시지를 수신하는 경우 호출될 콜백 함수임. 
 */
void
lacpIpcProcess (Int32T msgId, void * data, Uint32T dataLen)
{ 
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	DEBUGPRINT("==>> %s %d...\n", __FUNCTION__, __LINE__);

	switch(msgId)
	{
#if 0
	/* ProcessManager Interface IPC message. */
	case IPC_LCS_PM2C_SETROLE :
		DEBUGPRINT ("lacp : IPC_LCS_PM2C_SETROLE \n");
		lacpLcsSetRole (data, dataLen);
		break;
	case IPC_LCS_PM2C_TERMINATE :
		DEBUGPRINT ("lacp : IPC_LCS_PM2C_TERMINATE \n");
		lacpLcsTerminate (data, dataLen);
		break;
	case IPC_LCS_PM2C_HEALTHCHECK :
		DEBUGPRINT ("lacp : IPC_LCS_PM2C_HEALTHCHECK \n");
		lacpLcsHealthcheck (data, dataLen);
		break;
#endif
	default:
		break;
	}
}

#if 1
static PifPortEventMsgT *lacpEventPifLinkFind (Int32T iId)
{
	ListNodeT *pNode;
	PifPortEventMsgT *pPifPort;
	DEBUGPRINT("==>> %s %d...\n", __FUNCTION__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	for(pNode = pLacp->pLacpPifPortList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pPifPort = pNode->pData;
		if (pPifPort->iId == iId) {
			nnListDeleteNode (pLacp->pLacpPifPortList, pPifPort);
			return pPifPort;
		}
	}
	return NULL;
}

static void lacpEventPifLinkDel (PifPortEventMsgT *pPifPortMsg)
{
	ListNodeT *pNode;
	PifPortEventMsgT *pPifPort;
	DEBUGPRINT("==>> %s %d...\n", __FUNCTION__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	for(pNode = pLacp->pLacpPifPortList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pPifPort = pNode->pData;
		if (pPifPort->iId == pPifPortMsg->iId) {
			nnListDeleteNode (pLacp->pLacpPifPortList, pPifPort);
			return;
		}
	}
}

static void lacpEventPifLinkUpdate (PifPortEventMsgT *pPifPortMsg)
{
	PifPortEventMsgT *pPifPort;
	DEBUGPRINT("==>> %s %d...\n", __FUNCTION__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	pPifPort = lacpEventPifLinkFind (pPifPortMsg->iId);
	if (pPifPort) {
		*pPifPort = *pPifPortMsg;
		return;
	}

	pPifPort = NNMALLOC (MEM_GLOBAL, sizeof(*pPifPort));
	if (pPifPort) {
		*pPifPort = *pPifPortMsg;
		nnListAddNodeSort (pLacp->pLacpPifPortList, pPifPort);
	}
}

static void lacpEventPifLinkState (PifPortEventMsgT *pPifPortMsg, Int32T state)
{
	PifPortEventMsgT *pPifPort;
	DEBUGPRINT("==>> %s %d...\n", __FUNCTION__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	pPifPort = lacpEventPifLinkFind (pPifPortMsg->iId);
	if (pPifPort) {
		pPifPort->operState = state;
		return;
	}
}

void lacpPifPortDisplay (struct cmsh *cmsh)
{
	ListNodeT *pNode;
	PifPortEventMsgT *pPifPort;

	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);

	cmdPrint(cmsh, "Pif-reported interfaces");
	cmdPrint(cmsh, "  %-16s %-8s %-8s %-4s %-5s\n", "Interface", "ID", "index", "Type", "State");
	for(pNode = pLacp->pLacpPifPortList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pPifPort = pNode->pData;
		cmdPrint(cmsh, "  %-16s %8x %8x %4d %5d\n",
							pPifPort->ifName,
							pPifPort->iId,
							pPifPort->ifIndex,
							pPifPort->ifType,
							pPifPort->operState);
	}
}
#endif

static void lacpEventPifLink (void * data, Uint32T dataLen)
{
	PifPortEventMsgT	msgPortEvent;

	/* Buffer Reset & Assign */
	nnBufferT msgBuff;
	nnBufferReset (&msgBuff);
	nnBufferAssign (&msgBuff, data, dataLen);

	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	msgPortEvent.eId = nnBufferGetInt32T(&msgBuff);
	nnBufferGetString(&msgBuff, msgPortEvent.ifName, 32);
	msgPortEvent.iId = nnBufferGetInt32T(&msgBuff);
	msgPortEvent.ifIndex = nnBufferGetInt32T(&msgBuff);
	msgPortEvent.ifType = nnBufferGetInt16T(&msgBuff);
	msgPortEvent.adminState = nnBufferGetInt8T(&msgBuff);
	msgPortEvent.operState = nnBufferGetInt8T(&msgBuff);

	if (msgPortEvent.ifType < PIF_PORT_TYPE_FAST_ETHERNET
		&& msgPortEvent.ifType > PIF_PORT_TYPE_AGGREGATED) {
		DEBUGPRINT ("pif_l2_link : unknown interface type\n");
		return;
	}

	switch (msgPortEvent.eId) {
	case PIF_EVENT_LINK_ADD:
#if 1
		lacpEventPifLinkUpdate (&msgPortEvent);
#endif
		DEBUGPRINT ("pif_l2_link : link_add ");
		break;
	case PIF_EVENT_LINK_DEL:
#if 1
		lacpEventPifLinkDel (&msgPortEvent);
#endif
		DEBUGPRINT ("pif_l2_link : link_del ");
		break;
	case PIF_EVENT_LINK_UP:
#if 1
		lacpEventPifLinkState (&msgPortEvent, PIF_PORT_STATE_UP);
#endif
		DEBUGPRINT ("pif_l2_link : link_up ");
		break;
	case PIF_EVENT_LINK_DOWN:
#if 1
		lacpEventPifLinkState (&msgPortEvent, PIF_PORT_STATE_DOWN);
#endif
		DEBUGPRINT ("pif_l2_link : link_down ");
		break;
	default:
		DEBUGPRINT ("pif_l2_link : unknown\n");
		return;
	}

	DEBUGPRINT ("dev=%s(id=%d,idx=%d) type=%d,ad=%s,op=%s",
		msgPortEvent.ifName,
		msgPortEvent.iId,
		msgPortEvent.ifIndex,
		msgPortEvent.ifType,
		msgPortEvent.adminState == PIF_PORT_STATE_DOWN ? "down" : "up",
		msgPortEvent.operState == PIF_PORT_STATE_DOWN ? "down" : "up");
}

void lacpEventProcess (Int32T msgId, void * data, Uint32T dataLen)
{
	DEBUGPRINT("==>> %s %d...\n", __FUNCTION__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);

	switch (msgId) {
#if 0
	/* Process Manager Event. */
	case EVENT_LCS_COMPONENT_ERROR_OCCURRED :
		DEBUGPRINT ("lacp : EVENT_LCS_COMPONENT_ERROR_OCCURRED \n");
		lacpLcsEventComponentErrorOccured (data, dataLen);
		return;
	case EVENT_LCS_COMPONENT_SERVICE_STATUS :
		DEBUGPRINT ("lacp : EVENT_LCS_COMPONENT_SERVICE_STATUS \n");
		lacpLcsEventComponentServiceStatus (data, dataLen);
		return;
#endif
	case EVENT_PIF_L2_LINK:
		DEBUGPRINT("lacp : EVENT_PIF_L2_LINK\n");
		lacpEventPifLink (data, dataLen);
		break;
	default:
		break;
	}
}
