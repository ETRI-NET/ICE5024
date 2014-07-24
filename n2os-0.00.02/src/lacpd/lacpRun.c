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

#include "lacpDef.h"

static inline Int8T *lacpAggDevName (Int32T groupId)
{
	static char name[32];

	sprintf (name, "team%d", groupId);
	return name;
}

Int32T lacpGetRunGroupHwAddr(Int32T groupId, Int8T *mac)
{
	struct ifreq ifr;
	int	fd;
	int	err;

	strcpy(ifr.ifr_name, lacpAggDevName(groupId));
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		return errno;
	err = ioctl(fd, SIOCGIFHWADDR, &ifr);
	if(err) {
		close (fd);
		return err;
	}
	close (fd);

	memcpy (mac, &ifr.ifr_hwaddr.sa_data[0], 6);
	return 0;
}

#define setLacpConfigFile(f,g) sprintf ((f), "/tmp/lacp-config-%d", (g))

Int32T getLacpTimeout (Int32T groupId);
Int32T getLacpSysPriority (Int32T groupId);

static Int8T *lacpMakeConfFile (Int32T groupId, Int32T newGroup)
{
	Int32T count;
	LacpPortT *pPort = NULL, *pPortTmp;
	Int8T *f;
	Int8T configFile[64];
	Int32T	longTimeout;
	Int32T	sysPriority;

	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);

	if (newGroup) {
		longTimeout = LACP_DEFAULT_SYS_TIMEOUT;
		sysPriority = LACP_DEFAULT_SYS_PRIORITY;
	}
	else {
		longTimeout = getLacpTimeout (groupId);
		sysPriority = getLacpSysPriority (groupId);
	}

	count = getLacpConfig (groupId, &pPort);
	if (count <= 0) {
		return NULL;
	}

	setLacpConfigFile (configFile, groupId);
	FILE *fp = fopen (configFile, "w+");
	if (fp) {
		//BEGIN
		fprintf (fp, "{\n");
		//device
		fprintf (fp, "  \"device\": \"%s\",\n", lacpAggDevName(groupId));
		//runner
		fprintf (fp, "  \"runner\": {\n");
		fprintf (fp, "    \"name\": \"lacp\",\n");
		fprintf (fp, "    \"fast_rate\": %s,\n", longTimeout == LACP_SYS_TIMEOUT_SHORT ? "true" : "false");
		fprintf (fp, "    \"sys_prio\": %d,\n", sysPriority);
		fprintf (fp, "    \"tx_hash\": [\"eth\"]\n");
		fprintf (fp, "  },\n");
		//ports
		fprintf (fp, "  \"ports\": {\n");
		pPortTmp = pPort;
		while(count > 0) {
			fprintf (fp, "    \"%s\": {\n", pPortTmp->ifName);
			fprintf (fp, "      \"active\": %s,\n", pPortTmp->active ? "true" : "false");
			fprintf (fp, "      \"prio\": %d\n", pPortTmp->priority);
			fprintf (fp, "    }");
			count--;
			if (count > 0)
				fprintf (fp, ",\n");
			else
				fprintf (fp, "\n");
			pPortTmp++;
		}
		fprintf (fp, "  },\n");
		//watch
		fprintf (fp, "  \"link_watch\": {\n");
		fprintf (fp, "    \"name\": \"ethtool\"\n");
		fprintf (fp, "  }\n");
		//END
		fprintf (fp, "}\n");
		fclose (fp);
		f = strdup (configFile);
		NNFREE (MEM_GLOBAL, pPort);
		return f;
	}
	NNFREE (MEM_GLOBAL, pPort);
	return NULL;
}

void lacpRemoveConfFile (Int32T groupId)
{
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	Int8T configFile[64];
	setLacpConfigFile (configFile, groupId);
	unlink (configFile);
}

Int32T	lacpRunGroupAdd (Int32T groupId)
{
	Int8T *configFile;

	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	DEBUGPRINT("==>> %s %d\n", __FUNCTION__, groupId);
	configFile = lacpMakeConfFile (groupId, 1);
	if (configFile == NULL) {
		DEBUGPRINT("==>> %s %d %s file error\n", __FUNCTION__, groupId, configFile);
		return -1;
	}

	LacpGroupContextT *pContext = NNMALLOC (MEM_GLOBAL, sizeof(*pContext));
	if (pContext == NULL) {
		DEBUGPRINT("%s %d context creation error\n", __FUNCTION__, groupId);
		goto out;
	}

	void *context;
	context = lacpDevAggregatorAdd (configFile, 1);
	if (context == NULL) {
		DEBUGPRINT("%s %d aggregator add error\n", __FUNCTION__, groupId);
		goto out;
	}

	pContext->context = context;
	pContext->groupId = groupId;
	pContext->longTimeout = LACP_DEFAULT_SYS_TIMEOUT;
	pContext->sysPriority = LACP_DEFAULT_SYS_PRIORITY;
	nnListAddNodeSort (pLacp->pLacpGroupContextList, pContext);

	lacpToPifReportAggregatorAdd (groupId);
	free (configFile);
	return 0;

out:
	free (configFile);
	return -1;
}

Int32T	lacpRunGroupDel (Int32T groupId)
{

	DEBUGPRINT("==>> %s %d\n", __FUNCTION__, groupId);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	ListNodeT *pNode;
	LacpGroupContextT *pContext;
	for(pNode = pLacp->pLacpGroupContextList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pContext = pNode->pData;
		if (pContext->groupId == groupId) {
			lacpDevAggregatorDelete (pContext->context);
			nnListDeleteNode (pLacp->pLacpGroupContextList, pContext);
			DEBUGPRINT("==>> %s %d 2\n", __FUNCTION__, groupId);
			break;
		}
	}
	lacpToPifReportAggregatorDelete (groupId);
	lacpRemoveConfFile (groupId);
	return 0;
}

Int32T	lacpRunPortAdd (Int32T groupId, Int8T *ifName)
{
	Int8T *configFile;

	DEBUGPRINT("==>> %s %d %s\n", __FUNCTION__, groupId, ifName);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	configFile = lacpMakeConfFile (groupId, 0);
	if (configFile)
		free (configFile);
	void *context = NULL;
	ListNodeT *pNode;
	LacpGroupContextT *pContext;
	for(pNode = pLacp->pLacpGroupContextList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pContext = pNode->pData;
		if (pContext->groupId == groupId) {
			context = pContext->context;
			break;
		}
	}
	if (context)
		lacpDevPortAdd (context, ifName);
	lacpToPifReportPortAdd (groupId);
	return 0;
}

Int32T	lacpRunPortDel (Int32T groupId, Int8T *ifName) 
{
	Int8T *configFile;

	sleep (1);
	DEBUGPRINT("==>> %s %d %s\n", __FUNCTION__, groupId, ifName);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	configFile = lacpMakeConfFile (groupId, 0);
	if (configFile)
		free (configFile);
	void *context = NULL;
	ListNodeT *pNode;
	LacpGroupContextT *pContext;
	for(pNode = pLacp->pLacpGroupContextList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pContext = pNode->pData;
		if (pContext->groupId == groupId) {
			context = pContext->context;
			break;
		}
	}
	if (context)
		lacpDevPortDelete (context, ifName);
	lacpToPifReportPortDelete (groupId);
	return 0;
}

Int32T	lacpRunPortChange (Int32T groupId, Int8T *ifName)
{
	Int8T *configFile;

	DEBUGPRINT("==>> %s %d %s\n", __FUNCTION__, groupId, ifName);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	configFile = lacpMakeConfFile (groupId, 0);
	if (configFile)
		free (configFile);
	void *context = NULL;
	ListNodeT *pNode;
	LacpGroupContextT *pContext;
	for(pNode = pLacp->pLacpGroupContextList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pContext = pNode->pData;
		if (pContext->groupId == groupId) {
			context = pContext->context;
			break;
		}
	}
	if (context)
		lacpDevPortModify (context, ifName);
	return 0;
}

Int32T setLacpTimeout (Int32T groupId, Int32T longTimeout)
{
	ListNodeT *pNode;
	LacpGroupContextT *pContext;
	for(pNode = pLacp->pLacpGroupContextList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pContext = pNode->pData;
		if (pContext->groupId == groupId) {
			if (pContext->longTimeout == longTimeout)
				return 2;
			pContext->longTimeout = longTimeout;
			return 0;
		}
	}
	return 1;
}

Int32T getLacpTimeout (Int32T groupId)
{
	ListNodeT *pNode;
	LacpGroupContextT *pContext;
	for(pNode = pLacp->pLacpGroupContextList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pContext = pNode->pData;
		if (pContext->groupId == groupId) {
			return pContext->longTimeout;
		}
	}
	return LACP_DEFAULT_SYS_TIMEOUT;
}

Int32T getLacpSysPriority (Int32T groupId)
{
	ListNodeT *pNode;
	LacpGroupContextT *pContext;
	for(pNode = pLacp->pLacpGroupContextList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pContext = pNode->pData;
		if (pContext->groupId == groupId) {
			return pContext->sysPriority;
		}
	}
	return LACP_DEFAULT_SYS_PRIORITY;
}

Int32T lacpGroupTimeoutChange (struct cmsh *cmsh, Int32T groupId, Int32T longTimeout)
{
	Int8T *configFile;

	DEBUGPRINT("==>> %s %d %s\n", __FUNCTION__, groupId, longTimeout ? "Long" : "Short");
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);

	if (setLacpTimeout (groupId, longTimeout) != 0)
		return CMD_IPC_OK;

	configFile = lacpMakeConfFile (groupId, 0);
	if (configFile)
		free (configFile);

	void *context = NULL;
	ListNodeT *pNode;
	LacpGroupContextT *pContext;
	for(pNode = pLacp->pLacpGroupContextList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pContext = pNode->pData;
		if (pContext->groupId == groupId) {
			context = pContext->context;
			break;
		}
	}
	if (context)
		lacpDevModify (context);
	return CMD_IPC_OK;
}

#define	LACP_OUTPUT_SIZE	128
char *lacpConfigPrint[] = {
	"ports",
	"runner",
	NULL
};
char *lacpStatePrint[] = {
	"ports",
	"runner",
	NULL
};

void printLacpOutput (struct cmsh *cmsh, Int8T *str, char **prt)
{
	Int8T file[256];
	Int8T buf[256];
	FILE *fp;
	Int32T prtEnable;
	Int32T match;

	sprintf (file, "/tmp/lacp-show-%ld", time(NULL));

	fp = fopen (file, "w");
	if (fp == NULL) {
		cmdPrint (cmsh, "Cannot display");
		return;
	}
	fprintf (fp, "%s", str);
	fclose (fp);

	fp = fopen (file, "r");
	if (fp == NULL) {
		cmdPrint (cmsh, "Cannot display");
		return;
	}
	prtEnable = 0;
	while (fgets (buf, 256, fp)) {
		if (strlen(buf) < 5)
			continue;
		if (buf[4] == '\"') {
			match = 0;
			int	i = 0;
			while (prt[i]) {
				if (!strncmp (prt[i], &buf[5], strlen(prt[i]))) {
					match = 1;
					break;
				}
				i++;
			}
			prtEnable = match;
		}
		if (prtEnable)
			cmdPrint (cmsh, "%s", buf);
	}

	fclose (fp);
	unlink (file);
}

void lacpRunStateDisplay (struct cmsh *cmsh, Int32T groupId)
{
	void *context = NULL;
	ListNodeT *pNode;
	LacpGroupContextT *pContext;
	DEBUGPRINT("==>> %s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	for(pNode = pLacp->pLacpGroupContextList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pContext = pNode->pData;
		if (pContext->groupId == groupId) {
			context = pContext->context;
			break;
		}
	}
	if (context) {
    	char *state = lacpDevStateDump (context);
		if (state) {
			printLacpOutput (cmsh, state, lacpStatePrint);
			free (state);
		}
	}
	else {
		cmdPrint (cmsh, "\"port-channel\" %d not found", groupId);
	}
}

void lacpRunConfigDisplay (struct cmsh *cmsh, Int32T groupId)
{
	void *context = NULL;
	ListNodeT *pNode;
	LacpGroupContextT *pContext;
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	for(pNode = pLacp->pLacpGroupContextList->pHead;pNode != NULL;pNode = pNode->pNext)
	{
		pContext = pNode->pData;
		if (pContext->groupId == groupId) {
			context = pContext->context;
			break;
		}
	}
	if (context) {
    	char *cfg = lacpDevConfigDump (context);
		if (cfg) {
			printLacpOutput (cmsh, cfg, lacpConfigPrint);
			free (cfg);
		}
	}
	else {
		cmdPrint (cmsh, "\"port-channel\" %d not found", groupId);
	}
}

