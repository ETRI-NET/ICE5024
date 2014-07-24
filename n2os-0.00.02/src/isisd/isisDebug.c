/*
 * isisDebug.c
 *
 *  Created on: 2014. 6. 18.
 *      Author: root
 */

/*
 * 'isis debug', 'show debugging'
 */

#include <string.h>

#include "nnTypes.h"
#include "nnCmdCmsh.h"

#include "isisd.h"

void
print_debug (struct cmsh *cmsh, int flags, int onoff)
{
	char onoffs[4];
	if (onoff)
		strcpy(onoffs, "on");
	else
		strcpy(onoffs, "off");

	if (flags & DEBUG_ADJ_PACKETS)
		cmdPrint(cmsh, "IS-IS Adjacency related packets debugging is %s\n",
				onoffs);
	if (flags & DEBUG_CHECKSUM_ERRORS)
		cmdPrint(cmsh, "IS-IS checksum errors debugging is %s\n", onoffs);
	if (flags & DEBUG_LOCAL_UPDATES)
		cmdPrint(cmsh, "IS-IS local updates debugging is %s\n", onoffs);
	if (flags & DEBUG_PROTOCOL_ERRORS)
		cmdPrint(cmsh, "IS-IS protocol errors debugging is %s\n", onoffs);
	if (flags & DEBUG_SNP_PACKETS)
		cmdPrint(cmsh, "IS-IS CSNP/PSNP packets debugging is %s\n", onoffs);
	if (flags & DEBUG_SPF_EVENTS)
		cmdPrint(cmsh, "IS-IS SPF events debugging is %s\n", onoffs);
	if (flags & DEBUG_SPF_STATS)
		cmdPrint(cmsh, "IS-IS SPF Timing and Statistics Data debugging is %s\n",
				onoffs);
	if (flags & DEBUG_SPF_TRIGGERS)
		cmdPrint(cmsh, "IS-IS SPF triggering events debugging is %s\n", onoffs);
	if (flags & DEBUG_UPDATE_PACKETS)
		cmdPrint(cmsh, "IS-IS Update related packet debugging is %s\n", onoffs);
	if (flags & DEBUG_RTE_EVENTS)
		cmdPrint(cmsh, "IS-IS Route related debuggin is %s\n", onoffs);
	if (flags & DEBUG_EVENTS)
		cmdPrint(cmsh, "IS-IS Event debugging is %s\n", onoffs);
	if (flags & DEBUG_PACKET_DUMP)
		cmdPrint(cmsh, "IS-IS Packet dump debugging is %s\n", onoffs);
}

int
configWriteIsisDebug (struct cmsh *cmsh)
{
	int flags = isis->debugs;

	cmdPrint(cmsh, "!");

	if (flags & DEBUG_ADJ_PACKETS) {
		cmdPrint(cmsh, "debug isis adj-packets");
	}
	if (flags & DEBUG_CHECKSUM_ERRORS) {
		cmdPrint(cmsh, "debug isis checksum-errors");
	}
	if (flags & DEBUG_LOCAL_UPDATES) {
		cmdPrint(cmsh, "debug isis local-updates");
	}
	if (flags & DEBUG_PROTOCOL_ERRORS) {
		cmdPrint(cmsh, "debug isis protocol-errors");
	}
	if (flags & DEBUG_SNP_PACKETS) {
		cmdPrint(cmsh, "debug isis snp-packets");
	}
	if (flags & DEBUG_SPF_EVENTS) {
		cmdPrint(cmsh, "debug isis spf-events");
	}
	if (flags & DEBUG_SPF_STATS) {
		cmdPrint(cmsh, "debug isis spf-statistics");
	}
	if (flags & DEBUG_SPF_TRIGGERS) {
		cmdPrint(cmsh, "debug isis spf-triggers");
	}
	if (flags & DEBUG_UPDATE_PACKETS) {
		cmdPrint(cmsh, "debug isis update-packets");
	}
	if (flags & DEBUG_RTE_EVENTS) {
		cmdPrint(cmsh, "debug isis route-events");
	}
	if (flags & DEBUG_EVENTS) {
		cmdPrint(cmsh, "debug isis events");
	}
	if (flags & DEBUG_PACKET_DUMP) {
		cmdPrint(cmsh, "debug isis packet-dump");
	}

	return 0;
}
