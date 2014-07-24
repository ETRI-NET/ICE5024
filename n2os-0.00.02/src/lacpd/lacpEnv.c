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

#include "nnBuffer.h"
#include "nnPrefix.h"

#include "pifMsgEventData.h"
#include "pifMsgIpc.h"

#include "lacpDef.h"

#include <semaphore.h>

#define LACP_ACTOR_NEW   0
#define LACP_ACTOR_FOUND 1
#define LACP_ACTOR_FOUND_MOD 2
#define LACP_ACTOR_ERROR -1

Int32T lacpGetPortIfIndex(Int8T *ifName)
{
	struct ifreq ifr;
	int	fd;
	int	err;

	strcpy(ifr.ifr_name , ifName);
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		return errno;
	err = ioctl(fd, SIOCGIFINDEX, &ifr);
	if(err) {
		close (fd);
		return err;
	}
	close (fd);

	return ifr.ifr_ifindex;
}

/*
 * Description :
 *
 * param [in] pOldData : ListNode의 포인터
 * param [in] pNewData : ListNode의 포인터
 *
 * retval : 0 if 기존 ListNode의 데이터가 새로운 ListNode의 데이터 보다 큰경우
 *          1 if 기존 ListNode의 데이터가 새로운 ListNode의 데이터 보다 작은경우.
 */
static Int8T lacpPortCmp (void *pOldData, void *pNewData)
{
	if(((LacpPortT *)pOldData)->groupId > ((LacpPortT *)pNewData)->groupId)
		return 0;
	else if(((LacpPortT *)pOldData)->groupId < ((LacpPortT *)pNewData)->groupId)
		return 1;
	else {
		int v;
		v = strcmp(((LacpPortT *)pOldData)->ifName,((LacpPortT *)pNewData)->ifName);
		if (v == 0)
			return -1;
		if (v > 0)
			return 0;
		return 1;
	}
}

static Int8T 
lacpGroupContextCmp (void *pOldData, void *pNewData)
{
	if(((LacpGroupContextT *)pOldData)->groupId > ((LacpGroupContextT *)pNewData)->groupId)
		return 0;
	else
		return 1;
}

static Int8T 
lacpGroupFiniCmp (void *pOldData, void *pNewData)
{
	if(((LacpGroupFiniT *)pOldData)->context > ((LacpGroupFiniT *)pNewData)->context)
		return 0;
	else
		return 1;
}

static Int8T 
lacpPifPortListCmp (void *pOldData, void *pNewData)
{
	if(((PifPortEventMsgT *)pOldData)->iId > ((PifPortEventMsgT *)pNewData)->iId)
		return 0;
	else
		return 1;
}

void lacpGroupInit (void)
{
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	pLacp->pLacpPortList = nnListInit(lacpPortCmp, MEM_GLOBAL);
	pLacp->pLacpGroupContextList = nnListInit(lacpGroupContextCmp, MEM_GLOBAL);
	pLacp->pLacpGroupFiniList = nnListInit(lacpGroupFiniCmp, MEM_GLOBAL);
#if 1
	pLacp->pLacpPifPortList = nnListInit(lacpPifPortListCmp, MEM_GLOBAL);
#endif
}

void lacpGroupFini (void)
{
	ListNodeT *pNode;
	LacpPortT *pPort;
	PifPortEventMsgT *pPifPort;
	LacpGroupContextT *pContext;
	LacpGroupFiniT *pFini;
	Int32T groupId = LACP_GROUP_UNDEFINED;

	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);

loopPif:
	for(pNode = pLacp->pLacpPifPortList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pPifPort = pNode->pData;
		nnListDeleteNode (pLacp->pLacpPifPortList, pPifPort);
		goto loopPif;
	}
	nnListFree(pLacp->pLacpPifPortList);

	for(pNode = pLacp->pLacpGroupContextList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pContext = pNode->pData;
		pFini = NNMALLOC (MEM_GLOBAL, sizeof(*pFini));
		if (pFini) {
			pFini->context = (long) pContext->context;
			nnListAddNodeSort (pLacp->pLacpGroupFiniList, pFini);
		}
	}

loopPort:
	for(pNode = pLacp->pLacpPortList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pPort = pNode->pData;
		if (groupId == LACP_GROUP_UNDEFINED || groupId != pPort->groupId) {
			groupId = pPort->groupId;
			lacpRunGroupDel (groupId);
		}
		nnListDeleteNode (pLacp->pLacpPortList, pPort);
		goto loopPort;
	}
	nnListFree(pLacp->pLacpPortList);

loopGC:
	for(pNode = pLacp->pLacpGroupContextList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pContext = pNode->pData;
		nnListDeleteNode (pLacp->pLacpGroupContextList, pContext);
		goto loopGC;
	}
	nnListFree(pLacp->pLacpGroupContextList);
}

static Int32T lacpGroupFind (Int32T groupId)
{
	ListNodeT *pNode = NULL;
	LacpPortT *pPort = NULL;

	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	for(pNode = pLacp->pLacpPortList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pPort = pNode->pData;
		if (pPort->groupId == groupId)
			return 1;
	}
	return 0;
}

static LacpPortT *lacpPortFind (Int8T *ifName)
{
	ListNodeT *pNode = NULL;
	LacpPortT *pPort = NULL;

	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	for(pNode = pLacp->pLacpPortList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pPort = pNode->pData;
		if (!strcmp(pPort->ifName, ifName))
		return pPort;
	}
	return NULL;
}

static inline LacpPortT *lacpPortNew (Int8T *ifName, Int32T groupId, Int32T active, Int32T priority, Int32T ifIndex)
{
	LacpPortT *pPort = NNMALLOC (MEM_GLOBAL, sizeof(*pPort));
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	if (pPort) {
		strcpy (pPort->ifName, ifName);
		pPort->groupId = groupId;
		pPort->active = active;
		pPort->priority = priority;
		pPort->ifIndex = ifIndex;
		nnListAddNodeSort (pLacp->pLacpPortList, pPort);
		return pPort;
	}
	return NULL;
}

static Int32T lacpPortInit (Int8T *ifName, Int32T groupId, Int32T active, Int32T priority, Int32T ifIndex, Int32T *oldGroupId)
{
	Int32T ret;
	*oldGroupId = LACP_GROUP_UNDEFINED;
	LacpPortT *pPort = lacpPortFind (ifName);
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	if (pPort == NULL) {
newPort:
		if (lacpPortNew (ifName, groupId, active, priority, ifIndex))
			return LACP_ACTOR_NEW;
	return LACP_ACTOR_ERROR;
	}
	if (pPort->groupId != groupId) {
	*oldGroupId = pPort->groupId;
		nnListDeleteNode (pLacp->pLacpPortList, pPort);
		goto newPort;
	}

	ret = LACP_ACTOR_FOUND;
	if (pPort->active != active) {
		ret = LACP_ACTOR_FOUND_MOD;
		pPort->active = active;
	}
	if (pPort->priority != priority) {
		ret = LACP_ACTOR_FOUND_MOD;
		pPort->priority = priority;
	}
	return ret;
}

static Int32T lacpPortFini (Int8T *ifName, Int32T *oldGroupId)
{
	LacpPortT *pPort = lacpPortFind (ifName);
	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	if (pPort) {
		*oldGroupId = pPort->groupId;
		nnListDeleteNode (pLacp->pLacpPortList, pPort);
		return LACP_ACTOR_FOUND;
	}
	return LACP_ACTOR_ERROR;
}

void configWriteLacp (struct cmsh *cmsh)
{
	ListNodeT *pNode;
	LacpPortT *pPort;

	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	for(pNode = pLacp->pLacpPortList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pPort = pNode->pData;
		cmdPrint (cmsh, "!interface %s\n", pPort->ifName);
		cmdPrint (cmsh, " channel-group %d %s\n", pPort->groupId, pPort->active ? "active" : "passive");
		cmdPrint (cmsh, "!\n");
	}
}

void lacpCliConfigDisplay (struct cmsh *cmsh)
{
	ListNodeT *pNode;
	LacpPortT *pPort;

	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);

	cmdPrint(cmsh, "Configuration interfaces");
	cmdPrint(cmsh, "  %-16s %-5s %-8s %s\n", "Interface", "Group", "Priority", "Mode");
	for(pNode = pLacp->pLacpPortList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pPort = pNode->pData;
		cmdPrint(cmsh, "  %-16s %5d %8d %s\n",
							pPort->ifName,
							pPort->groupId,
							pPort->priority,
							pPort->active ? "active" : "passive");
	}
}

Int32T getLacpConfig (Int32T groupId, LacpPortT **pPortGet)
{
	int count = 0;
	ListNodeT *pNode;
	LacpPortT *pPort;
	LacpPortT *pPortAlloc, *pPortTmp;

	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	for(pNode = pLacp->pLacpPortList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pPort = pNode->pData;
		if (pPort->groupId == groupId)
			count++;
	}

	if (count == 0)
		return 0;

	pPortAlloc = NNMALLOC (MEM_GLOBAL, sizeof(*pPortAlloc) * count);
	if (pPortAlloc == NULL)
		return -1;
	pPortTmp = pPortAlloc;
	for(pNode = pLacp->pLacpPortList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pPort = pNode->pData;
		if (pPort->groupId == groupId) {
			memcpy (pPortTmp, pPort, sizeof(*pPort));
			pPortTmp++;
		}
	}

	*pPortGet = pPortAlloc;
	return count;
}

Int32T lacpPortAttach (struct cmsh *cmsh, Int32T groupId, Int8T *ifName, Int32T active)
{
	Int32T ifIndex;
	Int32T ret = CMD_IPC_OK;
	Int32T priority= 255;
	Int32T actorInit;
	Int32T groupFound = 0;
	Int32T oldGroupFound = 0;
	Int32T oldPortGroupId = LACP_GROUP_UNDEFINED;

	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	ifIndex = lacpGetPortIfIndex (ifName);
	if (ifIndex < 0) {
		cmdPrint (cmsh, "Unavailable interface %s.\n", ifName);
		return (CMD_IPC_OK);	//ERROR
	}

	groupFound = lacpGroupFind (groupId);
	actorInit = lacpPortInit (ifName, groupId, active, priority, ifIndex, &oldPortGroupId);
	if (oldPortGroupId != LACP_GROUP_UNDEFINED) {
		oldGroupFound = lacpGroupFind (oldPortGroupId);
		if (oldGroupFound) {
			lacpRunPortDel (oldPortGroupId, ifName);
		}
		else {
			if (lacpRunGroupDel (groupId))
				ret = CMD_IPC_OK;	//ERROR
		}
	}
	switch (actorInit) {
	case LACP_ACTOR_NEW:
		if (groupFound) {
			lacpRunPortAdd (groupId, ifName);
		}
		else {
			if (lacpRunGroupAdd (groupId)) {
				lacpPortFini (ifName, &groupId);
				ret = CMD_IPC_OK;	//ERROR
			}
		}
		break;
	case LACP_ACTOR_FOUND_MOD:
		lacpRunPortChange (groupId, ifName);
		break;
	case LACP_ACTOR_FOUND:
		break;
	default:
		break;
	}
	return (CMD_IPC_OK);
}

Int32T lacpPortDetach (struct cmsh *cmsh, Int8T *ifName)
{
	Int32T ifIndex;
	Int32T ret = CMD_IPC_OK;
	Int32T groupFound = 0;
	Int32T actorGroupId = LACP_GROUP_UNDEFINED;

	DEBUGPRINT("==>> %s %d...\n", __func__, __LINE__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);

	ifIndex = lacpGetPortIfIndex (ifName);
	if (ifIndex < 0) {
		cmdPrint (cmsh, "Unavailable interface %s.\n", ifName);
		return (CMD_IPC_OK);	//ERROR
	}

	if (lacpPortFini (ifName, &actorGroupId) == LACP_ACTOR_FOUND) {
		groupFound = lacpGroupFind (actorGroupId);
		if (groupFound) {
			lacpRunPortDel (actorGroupId, ifName);
		}
		else {
			if (lacpRunGroupDel (actorGroupId))
				ret = CMD_IPC_OK;	//ERROR
		}
	}
	return (ret);
}
