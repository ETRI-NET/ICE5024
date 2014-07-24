/*
 * isisRibmgr.c
 *
 *  Created on: 2014. 6. 10.
 *      Author: root
 */

#include "nnDefines.h"
#include "nnRibDefines.h"
#include "nnBuffer.h"
#include "nosLib.h"


void isisRibmgrInit()
{
	/* Buffer Reset */
	nnBufferT msgBuff;
	nnBufferReset(&msgBuff);

	/* Component ID. Defined in nnDefines.h. */
	nnBufferSetInt8T(&msgBuff, ISIS);

	/* Routing component type. Defined in nnRibDefines.h. */
	nnBufferSetInt8T(&msgBuff, RIB_ROUTE_TYPE_ISIS);

	/* Send ipc message to ribmgr */
	ipcSendAsync(RIB_MANAGER, IPC_RIB_CLIENT_INIT, msgBuff.data,
			msgBuff.length);
}

void isisRibmgrClose()
{
	  nnBufferT sendBuff;
	  nnBufferT recvBuff;

	  /* Buffer Reset */
	  nnBufferReset(&sendBuff);
	  nnBufferReset(&recvBuff);

	  /* Component ID. Defined in nnDefines.h. */
	  nnBufferSetInt8T(&sendBuff, ISIS);

	  /* Routing component type. Defined in nnRibDefines.h. */
	  nnBufferSetInt8T(&sendBuff, RIB_ROUTE_TYPE_ISIS);

	  NNLOG (LOG_DEBUG,
	         "%s called. send IPC_RIB_CLIENT_CLOSE to ribmgr\n", __func__);
	  /* Send ipc message to ribmgr */
	  ipcSendSync(RIB_MANAGER,
	              IPC_RIB_CLIENT_CLOSE, IPC_RIB_CLIENT_CLOSE,
	              sendBuff.data, sendBuff.length, recvBuff.data, &recvBuff.length);

	  ipcProcPendingMsg ();

}


Int32T
isisRibmgrReadIpv4 (Int32T command,  nnBufferT *msgBuff, Uint16T length)
{
	Int8T type = 0;
	Int8T flags = 0;
	Int8T messages = 0;
	Prefix4T prefix = { 0, };
	Int8T nhCnt = 0;
	struct in_addr nextHop = { 0, };
	Int8T ifCnt = 0;
	Uint32T ifIndex = 0;
	Int8T distance = 0;
	Uint32T metric = 0;

	/* Type, flags, message. */
	type = nnBufferGetInt8T(msgBuff);
	flags = nnBufferGetInt8T(msgBuff);
	messages = nnBufferGetInt8T(msgBuff);

	/* IPv4 prefix. */
	memset(&prefix, 0, sizeof(Prefix4T));
	prefix.family = AF_INET;
	prefix.prefixLen = nnBufferGetInt8T(msgBuff);
	prefix.prefix = nnBufferGetInaddr(msgBuff);

	/* Nexthop, ifindex, distance, metric. */
	if (CHECK_FLAG(messages, RIB_MESSAGE_NEXTHOP)) {
		nhCnt = nnBufferGetInt8T(msgBuff);
		nextHop = nnBufferGetInaddr(msgBuff);
	}
	if (CHECK_FLAG(messages, RIB_MESSAGE_IFINDEX)) {
		ifCnt = nnBufferGetInt8T(msgBuff);
		ifIndex = nnBufferGetInt32T(msgBuff);
	}
	/* Get distance. */
	if (CHECK_FLAG(messages, RIB_MESSAGE_DISTANCE)) {
		distance = nnBufferGetInt8T(msgBuff);
	}
	/* Get metric. */
	if (CHECK_FLAG(messages, RIB_MESSAGE_METRIC)) {
		metric = nnBufferGetInt32T(msgBuff);
	} else {
		metric = 0;
	}

//	  if (IS_RIP_DEBUG_RIBMGR)
	{
		NNLOG(LOG_DEBUG, "%s\n", __func__);
		if (command == IPC_RIB_IPV4_ROUTE_ADD)
			NNLOG(LOG_DEBUG, "IPC_RIB_IPV4_ROUTE_ADD\n");
		else
			NNLOG(LOG_DEBUG, "IPC_RIB_IPV4_ROUTE_DELETE\n");
		NNLOG(LOG_DEBUG, "\t prefix/length=%s/%d\n", inet_ntoa(prefix.prefix),
				prefix.prefixLen);
		NNLOG(LOG_DEBUG, "\t nexthop count=%d, address=%s\n", nhCnt,
				inet_ntoa(nextHop));
		NNLOG(LOG_DEBUG, "\t intf count = %d, intf index = %d\n", ifCnt,
				ifIndex);
		NNLOG(LOG_DEBUG, "\t distance = %d, metric = %d\n", distance, metric);
		NNLOG(LOG_DEBUG, "\t type = %d, flags = %d, messages = %d\n", type,
				flags, messages);
	}

	/* Then fetch IPv4 prefixes. */
	if (command == IPC_RIB_IPV4_ROUTE_ADD) {
//		  if (isis->debugs & DEBUG_ZEBRA)
//			zlog_debug ("IPv4 Route add from Z");
	}

	return 0;
}

