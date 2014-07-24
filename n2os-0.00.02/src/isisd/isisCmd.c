/*
 * isisCmd.c
 *
 *  Created on: 2014. 5. 20.
 *      Author: root
 */

#include <stdio.h>
#include "if.h"
#include "linklist.h"
#include "vty.h"

#include "nnTypes.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdInstall.h"

#include "isis_csm.h"
#include "isis_circuit.h"
#include "isisd.h"
#include "isis_spf.h"
#include "isis_misc.h"
#include "isis_events.h"
#include "isis_tlv.h"
#include "isis_lsp.h"
#include "isis_dynhn.h"
#include "isisDebug.h"


DENODEC(cmdFuncEnterIsis,
        CMD_NODE_CONFIG_ISIS,
        IPC_ISIS)
{
	Int32T i;
	cmdPrint(cmsh, "uargc1[%d]\n", uargc1);
	for (i = 0; i < uargc1; i++) {
		cmdPrint(cmsh, "uargv1[%d] = [%s]\n", i, uargv1[i]);
	}
	cmdPrint(cmsh, "uargc2[%d]\n", uargc2);
	for (i = 0; i < uargc2; i++) {
		cmdPrint(cmsh, "uargv2[%d] = [%s]\n", i, uargv2[i]);
	}
	cmdPrint(cmsh, "cargc[%d]\n", cargc);
	for (i = 0; i < cargc; i++) {
		cmdPrint(cmsh, "cargv[%d] = [%s]\n", i, cargv[i]);
	}
	isisAreaGet(cargv[2]);
	return (CMD_IPC_OK);
}

DECMD(cmdFuncIsisIpRouterIsis,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
		"ip router isis WORD",
		"Interface Internet Protocol config commands",
		"IP router interface commands",
		"IS-IS Routing for IP",
		"Routing process tag")
{
	struct isis_circuit *circuit;
	struct interface *ifp;
	struct isis_area *area;

	/* Check isis is installed or not. */
	if (!isis)
	{
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	assert(ifp);

	/* Prevent more than one area per circuit */
	circuit = circuit_scan_by_ifp(ifp);
	if (circuit) {
		if (circuit->ip_router == 1) {
			if (strcmp(circuit->area->area_tag, cargv[3])) {
				cmdPrint(cmsh, "ISIS circuit is already defined on %s\n", circuit->area->area_tag);
				return (CMD_IPC_ERROR);
			}
			return (CMD_IPC_OK);
		}
	}

	area = isisAreaGet(cargv[3]);
	if (!area) {
		cmdPrint(cmsh, "Can't find ISIS instance\n");
		return (CMD_IPC_ERROR);
	}

	circuit = isis_csm_state_change(ISIS_ENABLE, circuit, area);
	isis_circuit_if_bind(circuit, ifp);

	circuit->ip_router = 1;
	area->ip_circuits++;
	circuit_update_nlpids(circuit);

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoIpRouterIsis,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no ip router isis WORD",
       "Negate a command or set its defaults",
       "Interface Internet Protocol config commands",
       "IP router interface commands",
       "IS-IS Routing for IP",
       "Routing process tag")
{
	struct interface *ifp;
	struct isis_area *area;
	struct isis_circuit *circuit;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface \n");
		return (CMD_IPC_ERROR);
	}

	area = isis_area_lookup(cargv[4]);
	if (!area) {
		cmdPrint(cmsh, "Can't find ISIS instance %s\n", cargv[4]);
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_lookup_by_ifp(ifp, area->circuit_list);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	circuit->ip_router = 0;
	area->ip_circuits--;
#if 0 //HAVE_IPV6
	if (circuit->ipv6_router == 0)
#endif
	isis_csm_state_change(ISIS_DISABLE, circuit, area);

	return (CMD_IPC_OK);
}



DECMD (cmdFuncIsisPassive,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis passive",
       "IS-IS commands",
       "Configure the passive mode for interface")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	if (!circuit)
		return (CMD_IPC_ERROR);

	if (circuit->is_passive == 1)
		return (CMD_IPC_OK);

	if (circuit->state != C_STATE_UP) {
		circuit->is_passive = 1;
	} else {
		struct isis_area *area = circuit->area;
		isis_csm_state_change(ISIS_DISABLE, circuit, area);
		circuit->is_passive = 1;
		isis_csm_state_change(ISIS_ENABLE, circuit, area);
	}

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoPassive,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis passive",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Configure the passive mode for interface")
{
	struct interface *ifp;
	struct isis_circuit *circuit;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface \n");
		return (CMD_IPC_ERROR);
	}

	/* FIXME: what is wrong with circuit = ifp->info ? */
	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	if (if_is_loopback(ifp)) {
		cmdPrint(cmsh, "Can't set no passive for loopback interface\n");
		return (CMD_IPC_ERROR);
	}

	if (circuit->is_passive == 0)
		return (CMD_IPC_OK);

	if (circuit->state != C_STATE_UP) {
		circuit->is_passive = 0;
	} else {
		struct isis_area *area = circuit->area;
		isis_csm_state_change(ISIS_DISABLE, circuit, area);
		circuit->is_passive = 0;
		isis_csm_state_change(ISIS_ENABLE, circuit, area);
	}

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisCircuitType,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis circuit-type (level-1|level-1-2|level-2-only)",
       "IS-IS commands",
       "Configure circuit type for interface",
       "Level-1 only adjacencies are formed",
       "Level-1-2 adjacencies are formed",
       "Level-2 only adjacencies are formed")
{
	int circuit_type;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	circuit_type = string2circuit_t(cargv[2]);
	if (!circuit_type) {
		cmdPrint(cmsh, "Unknown circuit-type \n");
		return (CMD_IPC_ERROR);
	}

	if (circuit->state == C_STATE_UP
			&& circuit->area->is_type != IS_LEVEL_1_AND_2
			&& circuit->area->is_type != circuit_type) {
		cmdPrint(cmsh, "Invalid circuit level for area %s.\n", circuit->area->area_tag);
		return (CMD_IPC_ERROR);
	}
	isis_event_circuit_type_change(circuit, circuit_type);

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoCircuitType,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis circuit-type (level-1|level-1-2|level-2-only)",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Configure circuit type for interface",
       "Level-1 only adjacencies are formed",
       "Level-1-2 adjacencies are formed",
       "Level-2 only adjacencies are formed")
{
	int circuit_type;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	/*
	 * Set the circuits level to its default value
	 */
	if (circuit->state == C_STATE_UP)
		circuit_type = circuit->area->is_type;
	else
		circuit_type = IS_LEVEL_1_AND_2;
	isis_event_circuit_type_change(circuit, circuit_type);

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisPasswdMd5,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis password md5 WORD",
       "IS-IS commands",
       "Configure the authentication password for a circuit",
       "Authentication type",
       "Circuit password")
{
	int len;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	len = strlen(cargv[3]);
	if (len > 254) {
		cmdPrint(cmsh, "Too long circuit password (>254)\n");
		return (CMD_IPC_ERROR);
	}
	circuit->passwd.len = len;
	circuit->passwd.type = ISIS_PASSWD_TYPE_HMAC_MD5;
	strncpy((char *) circuit->passwd.passwd, cargv[3], 255);

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisPasswdClear,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis password clear WORD",
       "IS-IS commands",
       "Configure the authentication password for a circuit",
       "Authentication type",
       "Circuit password")
{
	int len;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	len = strlen(cargv[3]);
	if (len > 254) {
		cmdPrint(cmsh, "Too long circuit password (>254)\n");
		return (CMD_IPC_ERROR);
	}
	circuit->passwd.len = len;
	circuit->passwd.type = ISIS_PASSWD_TYPE_CLEARTXT;
	strncpy((char *) circuit->passwd.passwd, cargv[3], 255);

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoPasswd,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis password",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Configure the authentication password for a circuit")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	memset(&circuit->passwd, 0, sizeof(struct isis_passwd));

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisPriority,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis priority <0-127>",
       "IS-IS commands",
       "Set priority for Designated Router election",
       "Priority value")
{
	int prio;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}
	prio = atoi(cargv[2]);
	if (prio < MIN_PRIORITY || prio > MAX_PRIORITY) {
		cmdPrint(cmsh, "Invalid priority %d - should be <0-127>\n", prio);
		return (CMD_IPC_ERROR);
	}

	circuit->priority[0] = prio;
	circuit->priority[1] = prio;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoPriority,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis priority",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set priority for Designated Router election")
{
	struct interface *ifp;
		struct isis_circuit *circuit = NULL;

		/* Check isis is installed or not. */
		if (!isis) {
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

		ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
		if (!ifp) {
			cmdPrint(cmsh, "Invalid interface\n");
			return (CMD_IPC_ERROR);
		}

		circuit = circuit_scan_by_ifp(ifp);
		if (!circuit) {
			cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
			return (CMD_IPC_ERROR);
		}

		if(cargc>3)
		{
			int i;
			for( i = 0; i<2; i++)
			{
				if( atoi(cargv[3]) == circuit->priority[i])
				{
					circuit->priority[i] = DEFAULT_PRIORITY;
					return (CMD_IPC_OK);
				}
			}
			return (CMD_IPC_ERROR);
		}

		circuit->priority[0] = DEFAULT_PRIORITY;
		circuit->priority[1] = DEFAULT_PRIORITY;

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisNoPriority,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis priority <0-127>",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set priority for Designated Router election",
       "Priority value");

DECMD (cmdFuncIsisPriorityL1,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis priority <0-127> level-1",
       "IS-IS commands",
       "Set priority for Designated Router election",
       "Priority value",
       "Specify priority for level-1 routing")
{
	int prio;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	prio = atoi(cargv[2]);
	if (prio < MIN_PRIORITY || prio > MAX_PRIORITY) {
		cmdPrint(cmsh, "Invalid priority %d - should be <0-127>\n", prio);
		return (CMD_IPC_ERROR);
	}

	circuit->priority[0] = prio;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoPriorityL1,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis priority level-1",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set priority for Designated Router election",
       "Specify priority for level-1 routing")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	circuit->priority[0] = DEFAULT_PRIORITY;

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisNoPriorityL1,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis priority <0-127> level-1",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set priority for Designated Router election",
       "Priority value",
       "Specify priority for level-1 routing");

DECMD (cmdFuncIsisPriorityL2,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis priority <0-127> level-2",
       "IS-IS commands",
       "Set priority for Designated Router election",
       "Priority value",
       "Specify priority for level-2 routing")
{
	int prio;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}
	prio = atoi(cargv[2]);
	if (prio < MIN_PRIORITY || prio > MAX_PRIORITY) {
		cmdPrint(cmsh, "Invalid priority %d - should be <0-127>\n\n", prio);
		return (CMD_IPC_ERROR);
	}

	circuit->priority[1] = prio;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoPriorityL2,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis priority level-2",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set priority for Designated Router election",
       "Specify priority for level-2 routing")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	circuit->priority[1] = DEFAULT_PRIORITY;

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisNoPriorityL2,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis priority <0-127> level-2",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set priority for Designated Router election",
       "Priority value",
       "Specify priority for level-2 routing");

/* Metric command */
DECMD (cmdFuncIsisMetric,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis metric <0-16777215>",
       "IS-IS commands",
       "Set default metric for circuit",
       "Default metric value")
{
	int met;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	met = atoi(cargv[2]);

	/* RFC3787 section 5.1 */
	if (circuit->area && circuit->area->oldmetric == 1&&
	met > MAX_NARROW_LINK_METRIC) {
		cmdPrint(cmsh, "Invalid metric %d - should be <0-63> "
				"when narrow metric type enabled\n", met);
		return (CMD_IPC_ERROR);
	}

	/* RFC4444 */
	if (circuit->area && circuit->area->newmetric == 1&&
	met > MAX_WIDE_LINK_METRIC) {
		cmdPrint(cmsh, "Invalid metric %d - should be <0-16777215> "
				"when wide metric type enabled\n", met);
		return (CMD_IPC_ERROR);
	}

	circuit->te_metric[0] = met;
	circuit->te_metric[1] = met;

	circuit->metrics[0].metric_default = met;
	circuit->metrics[1].metric_default = met;

	if (circuit->area)
		lsp_regenerate_schedule(circuit->area, circuit->is_type, 0);

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoMetric,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis metric",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set default metric for circuit")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	if(cargc > 3)
	{
		int i;
		for ( i = 0; i < 2; i++ )
		{
			if( atoi(cargv[3]) == circuit->te_metric[i] )
			{
				circuit->te_metric[i] = DEFAULT_CIRCUIT_METRIC;
				circuit->metrics[i].metric_default = DEFAULT_CIRCUIT_METRIC;
			}
		}
	}
	else
	{
		circuit->te_metric[0] = DEFAULT_CIRCUIT_METRIC;
		circuit->te_metric[1] = DEFAULT_CIRCUIT_METRIC;
		circuit->metrics[0].metric_default = DEFAULT_CIRCUIT_METRIC;
		circuit->metrics[1].metric_default = DEFAULT_CIRCUIT_METRIC;
	}

	if (circuit->area)
		lsp_regenerate_schedule(circuit->area, circuit->is_type, 0);

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisNoMetric,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis metric <0-16777215>",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set default metric for circuit",
       "Default metric value");

DECMD (cmdFuncIsisMetricL1,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis metric <0-16777215> level-1",
       "IS-IS commands",
       "Set default metric for circuit",
       "Default metric value",
       "Specify metric for level-1 routing")
{
	int met;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	met = atoi(cargv[2]);

	/* RFC3787 section 5.1 */
	if (circuit->area && circuit->area->oldmetric == 1&&
	met > MAX_NARROW_LINK_METRIC) {
		cmdPrint(cmsh, "Invalid metric %d - should be <0-63> "
				"when narrow metric type enabled\n", met);
		return (CMD_IPC_ERROR);
	}

	/* RFC4444 */
	if (circuit->area && circuit->area->newmetric == 1&&
	met > MAX_WIDE_LINK_METRIC) {
		cmdPrint(cmsh, "Invalid metric %d - should be <0-16777215> "
				"when wide metric type enabled\n", met);
		return (CMD_IPC_ERROR);
	}

	circuit->te_metric[0] = met;
	circuit->metrics[0].metric_default = met;

	if (circuit->area)
		lsp_regenerate_schedule(circuit->area, IS_LEVEL_1, 0);

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoMetricL1,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis metric level-1",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set default metric for circuit",
       "Specify metric for level-1 routing")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	circuit->te_metric[0] = DEFAULT_CIRCUIT_METRIC;
	circuit->metrics[0].metric_default = DEFAULT_CIRCUIT_METRIC;

	if (circuit->area)
		lsp_regenerate_schedule(circuit->area, IS_LEVEL_1, 0);

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisNoMetricL1,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis metric <0-16777215> level-1",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set default metric for circuit",
       "Default metric value",
       "Specify metric for level-1 routing");

DECMD (cmdFuncIsisMetricL2,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis metric <0-16777215> level-2",
       "IS-IS commands",
       "Set default metric for circuit",
       "Default metric value",
       "Specify metric for level-2 routing")
{
	int met;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}
	met = atoi(cargv[2]);

	/* RFC3787 section 5.1 */
	if (circuit->area && circuit->area->oldmetric == 1&&
	met > MAX_NARROW_LINK_METRIC) {
		cmdPrint(cmsh, "Invalid metric %d - should be <0-63> "
				"when narrow metric type enabled\n", met);
		return (CMD_IPC_ERROR);
	}

	/* RFC4444 */
	if (circuit->area && circuit->area->newmetric == 1&&
	met > MAX_WIDE_LINK_METRIC) {
		cmdPrint(cmsh, "Invalid metric %d - should be <0-16777215> "
				"when wide metric type enabled\n", met);
		return (CMD_IPC_ERROR);
	}

	circuit->te_metric[1] = met;
	circuit->metrics[1].metric_default = met;

	if (circuit->area)
		lsp_regenerate_schedule(circuit->area, IS_LEVEL_2, 0);

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoMetricL2,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis metric level-2",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set default metric for circuit",
       "Specify metric for level-2 routing")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	circuit->te_metric[1] = DEFAULT_CIRCUIT_METRIC;
	circuit->metrics[1].metric_default = DEFAULT_CIRCUIT_METRIC;

	if (circuit->area)
		lsp_regenerate_schedule(circuit->area, IS_LEVEL_2, 0);

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisNoMetricL2,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis metric <0-16777215> level-2",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set default metric for circuit",
       "Default metric value",
       "Specify metric for level-2 routing");
/* end of metrics */

DECMD (cmdFuncIsisHelloInterval,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis hello-interval <1-600>",
       "IS-IS commands",
       "Set Hello interval",
       "Hello interval value")
{
	int interval;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	interval = atoi(cargv[2]);
	if (interval < MIN_HELLO_INTERVAL || interval > MAX_HELLO_INTERVAL) {
		cmdPrint(cmsh, "Invalid hello-interval %d - should be <1-600>\n", interval);
		return (CMD_IPC_ERROR);
	}

	circuit->hello_interval[0] = (u_int16_t) interval;
	circuit->hello_interval[1] = (u_int16_t) interval;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoHelloInterval,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis hello-interval",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set Hello interval")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}
	circuit->hello_interval[0] = DEFAULT_HELLO_INTERVAL;
	circuit->hello_interval[1] = DEFAULT_HELLO_INTERVAL;

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisNoHelloInterval,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis hello-interval <1-600>",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set Hello interval",
       "Hello interval value");

DECMD (cmdFuncIsisHelloIntervalL1,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis hello-interval <1-600> level-1",
       "IS-IS commands",
       "Set Hello interval",
       "Hello interval value",
       "Specify hello-interval for level-1 IIHs")
{
	long interval;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}
	interval = atoi(cargv[2]);
	if (interval < MIN_HELLO_INTERVAL || interval > MAX_HELLO_INTERVAL) {
		cmdPrint(cmsh, "Invalid hello-interval %ld - should be <1-600>\n", interval);
		return (CMD_IPC_ERROR);
	}

	circuit->hello_interval[0] = (u_int16_t) interval;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoHelloIntervalL1,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis hello-interval level-1",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set Hello interval",
       "Specify hello-interval for level-1 IIHs")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}
	circuit->hello_interval[0] = DEFAULT_HELLO_INTERVAL;

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisNoHelloIntervalL1,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis hello-interval <1-600> level-1",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set Hello interval",
       "Hello interval value",
       "Specify hello-interval for level-1 IIHs");

DECMD (cmdFuncIsisHelloIntervalL2,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis hello-interval <1-600> level-2",
       "IS-IS commands",
       "Set Hello interval",
       "Hello interval value",
       "Specify hello-interval for level-2 IIHs")
{
	long interval;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	interval = atoi(cargv[2]);
	if (interval < MIN_HELLO_INTERVAL || interval > MAX_HELLO_INTERVAL) {
		cmdPrint(cmsh, "Invalid hello-interval %ld - should be <1-600>%s\n", interval);
		return (CMD_IPC_ERROR);
	}

	circuit->hello_interval[1] = (u_int16_t) interval;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoHelloIntervalL2,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis hello-interval level-2",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set Hello interval",
       "Specify hello-interval for level-2 IIHs")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	circuit->hello_interval[1] = DEFAULT_HELLO_INTERVAL;

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisNoHelloIntervalL2,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis hello-interval <1-600> level-2",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set Hello interval",
       "Hello interval value",
       "Specify hello-interval for level-2 IIHs");

DECMD (cmdFuncIsisHelloMultiplier,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis hello-multiplier <2-100>",
       "IS-IS commands",
       "Set multiplier for Hello holding time",
       "Hello multiplier value")
{
	int mult;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	mult = atoi(cargv[2]);
	if (mult < MIN_HELLO_MULTIPLIER || mult > MAX_HELLO_MULTIPLIER) {
		cmdPrint(cmsh, "Invalid hello-multiplier %d - should be <2-100>\n", mult);
		return (CMD_IPC_ERROR);
	}

	circuit->hello_multiplier[0] = (u_int16_t) mult;
	circuit->hello_multiplier[1] = (u_int16_t) mult;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoHelloMultiplier,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis hello-multiplier",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set multiplier for Hello holding time")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	circuit->hello_multiplier[0] = DEFAULT_HELLO_MULTIPLIER;
	circuit->hello_multiplier[1] = DEFAULT_HELLO_MULTIPLIER;

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisNoHelloMultiplier,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis hello-multiplier <2-100>",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set multiplier for Hello holding time",
       "Hello multiplier value");

DECMD (cmdFuncIsisHelloMultiplierL1,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis hello-multiplier <2-100> level-1",
       "IS-IS commands",
       "Set multiplier for Hello holding time",
       "Hello multiplier value",
       "Specify hello multiplier for level-1 IIHs")
{
	int mult;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	mult = atoi(cargv[2]);
	if (mult < MIN_HELLO_MULTIPLIER || mult > MAX_HELLO_MULTIPLIER) {
		cmdPrint(cmsh, "Invalid hello-multiplier %d - should be <2-100>\n", mult);
		return (CMD_IPC_ERROR);
	}

	circuit->hello_multiplier[0] = (u_int16_t) mult;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoHelloMultiplierL1,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis hello-multiplier level-1",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set multiplier for Hello holding time",
       "Specify hello multiplier for level-1 IIHs")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	circuit->hello_multiplier[0] = DEFAULT_HELLO_MULTIPLIER;

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisNoHelloMultiplierL1,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis hello-multiplier <2-100> level-1",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set multiplier for Hello holding time",
       "Hello multiplier value",
       "Specify hello multiplier for level-1 IIHs");

DECMD (cmdFuncIsisHelloMultiplierL2,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis hello-multiplier <2-100> level-2",
       "IS-IS commands",
       "Set multiplier for Hello holding time",
       "Hello multiplier value",
       "Specify hello multiplier for level-2 IIHs")
{
	int mult;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	mult = atoi(cargv[2]);
	if (mult < MIN_HELLO_MULTIPLIER || mult > MAX_HELLO_MULTIPLIER) {
		cmdPrint(cmsh, "Invalid hello-multiplier %d - should be <2-100>\n", mult);
		return (CMD_IPC_ERROR);
	}

	circuit->hello_multiplier[1] = (u_int16_t) mult;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoHelloMultiplierL2,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis hello-multiplier level-2",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set multiplier for Hello holding time",
       "Specify hello multiplier for level-2 IIHs")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	circuit->hello_multiplier[1] = DEFAULT_HELLO_MULTIPLIER;

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisNoHelloMultiplierL2,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis hello-multiplier <2-100> level-2",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set multiplier for Hello holding time",
       "Hello multiplier value",
       "Specify hello multiplier for level-2 IIHs");

DECMD (cmdFuncIsisHelloPadding,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis hello padding",
       "IS-IS commands",
       "Add padding to IS-IS hello packets",
       "Pad hello packets",
       "<cr>")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	circuit->pad_hellos = 1;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoHelloPadding,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis hello padding",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Add padding to IS-IS hello packets",
       "Pad hello packets",
       "<cr>")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	circuit->pad_hellos = 0;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisCsnpInterval,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis csnp-interval <1-600>",
       "IS-IS commands",
       "Set CSNP interval in seconds",
       "CSNP interval value")
{
	unsigned long interval;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	interval = atol(cargv[2]);
	if (interval < MIN_CSNP_INTERVAL || interval > MAX_CSNP_INTERVAL) {
		cmdPrint(cmsh, "Invalid csnp-interval %lu - should be <1-600>", interval);
		return (CMD_IPC_ERROR);
	}

	circuit->csnp_interval[0] = (u_int16_t) interval;
	circuit->csnp_interval[1] = (u_int16_t) interval;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoCsnpInterval,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis csnp-interval",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set CSNP interval in seconds")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	circuit->csnp_interval[0] = DEFAULT_CSNP_INTERVAL;
	circuit->csnp_interval[1] = DEFAULT_CSNP_INTERVAL;

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisNoCsnpInterval,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis csnp-interval <1-600>",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set CSNP interval in seconds",
       "CSNP interval value");

DECMD (cmdFuncIsisCsnpIntervalL1,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis csnp-interval <1-600> level-1",
       "IS-IS commands",
       "Set CSNP interval in seconds",
       "CSNP interval value",
       "Specify interval for level-1 CSNPs")
{
	unsigned long interval;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	interval = atol(cargv[2]);
	if (interval < MIN_CSNP_INTERVAL || interval > MAX_CSNP_INTERVAL) {
		cmdPrint(cmsh, "Invalid csnp-interval %lu - should be <1-600>\n", interval);
		return (CMD_IPC_ERROR);
	}

	circuit->csnp_interval[0] = (u_int16_t) interval;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoCsnpIntervalL1,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis csnp-interval level-1",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set CSNP interval in seconds",
       "Specify interval for level-1 CSNPs")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	circuit->csnp_interval[0] = DEFAULT_CSNP_INTERVAL;

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisNoCsnpIntervalL1,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis csnp-interval <1-600> level-1",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set CSNP interval in seconds",
       "CSNP interval value",
       "Specify interval for level-1 CSNPs");

DECMD (cmdFuncIsisCsnpIntervalL2,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis csnp-interval <1-600> level-2",
       "IS-IS commands",
       "Set CSNP interval in seconds",
       "CSNP interval value",
       "Specify interval for level-2 CSNPs")
{
	unsigned long interval;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	interval = atol(cargv[2]);
	if (interval < MIN_CSNP_INTERVAL || interval > MAX_CSNP_INTERVAL) {
		cmdPrint(cmsh, "Invalid csnp-interval %lu - should be <1-600>\n", interval);
		return (CMD_IPC_ERROR);
	}

	circuit->csnp_interval[1] = (u_int16_t) interval;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoCsnpIntervalL2,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis csnp-interval level-2",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set CSNP interval in seconds",
       "Specify interval for level-2 CSNPs")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	circuit->csnp_interval[1] = DEFAULT_CSNP_INTERVAL;

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisNoCsnpIntervalL2,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis csnp-interval <1-600> level-2",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set CSNP interval in seconds",
       "CSNP interval value",
       "Specify interval for level-2 CSNPs");

DECMD (cmdFuncIsisPsnpInterval,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis psnp-interval <1-120>",
       "IS-IS commands",
       "Set PSNP interval in seconds",
       "PSNP interval value")
{
	unsigned long interval;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	interval = atol(cargv[2]);
	if (interval < MIN_PSNP_INTERVAL || interval > MAX_PSNP_INTERVAL) {
		cmdPrint(cmsh, "Invalid psnp-interval %lu - should be <1-120>\n", interval);
		return (CMD_IPC_ERROR);
	}

	circuit->psnp_interval[0] = (u_int16_t) interval;
	circuit->psnp_interval[1] = (u_int16_t) interval;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoPsnpInterval,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis psnp-interval",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set PSNP interval in seconds")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	circuit->psnp_interval[0] = DEFAULT_PSNP_INTERVAL;
	circuit->psnp_interval[1] = DEFAULT_PSNP_INTERVAL;

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisNoPsnpInterval,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis psnp-interval <1-120>",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set PSNP interval in seconds",
       "PSNP interval value");

DECMD (cmdFuncIsisPsnpIntervalL1,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis psnp-interval <1-120> level-1",
       "IS-IS commands",
       "Set PSNP interval in seconds",
       "PSNP interval value",
       "Specify interval for level-1 PSNPs")
{
	unsigned long interval;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	interval = atol(cargv[2]);
	if (interval < MIN_PSNP_INTERVAL || interval > MAX_PSNP_INTERVAL) {
		cmdPrint(cmsh, "Invalid psnp-interval %lu - should be <1-120>\n", interval);
		return (CMD_IPC_ERROR);
	}

	circuit->psnp_interval[0] = (u_int16_t) interval;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoPsnpIntervalL1,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis psnp-interval level-1",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set PSNP interval in seconds",
       "Specify interval for level-1 PSNPs")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	circuit->psnp_interval[0] = DEFAULT_PSNP_INTERVAL;

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisNoPsnpIntervalL1,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis psnp-interval <1-120> level-1",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set PSNP interval in seconds",
       "PSNP interval value",
       "Specify interval for level-1 PSNPs");

DECMD (cmdFuncIsisPsnpIntervalL2,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis psnp-interval <1-120> level-2",
       "IS-IS commands",
       "Set PSNP interval in seconds",
       "PSNP interval value",
       "Specify interval for level-2 PSNPs")
{
	unsigned long interval;
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	interval = atol(cargv[2]);
	if (interval < MIN_PSNP_INTERVAL || interval > MAX_PSNP_INTERVAL) {
		cmdPrint(cmsh, "Invalid psnp-interval %lu - should be <1-120>\n", interval);
		return (CMD_IPC_ERROR);
	}

	circuit->psnp_interval[1] = (u_int16_t) interval;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoPsnpIntervalL2,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis psnp-interval level-2",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set PSNP interval in seconds",
       "Specify interval for level-2 PSNPs")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	circuit->psnp_interval[1] = DEFAULT_PSNP_INTERVAL;

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisNoPsnpIntervalL2,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis psnp-interval <1-120> level-2",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set PSNP interval in seconds",
       "PSNP interval value",
       "Specify interval for level-2 PSNPs");

DECMD (cmdFuncIsisNetwork,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "isis network point-to-point",
       "IS-IS commands",
       "Set network type",
       "point-to-point network type")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	/* RFC5309 section 4 */
	if (circuit->circ_type == CIRCUIT_T_P2P)
		return (CMD_IPC_OK);

	if (circuit->state != C_STATE_UP) {
		circuit->circ_type = CIRCUIT_T_P2P;
		circuit->circ_type_config = CIRCUIT_T_P2P;
	} else {
		struct isis_area *area = circuit->area;
		if (!if_is_broadcast(circuit->interface)) {
			cmdPrint(cmsh, "isis network point-to-point "
					"is valid only on broadcast interfaces\n");
			return (CMD_IPC_ERROR);
		}

		isis_csm_state_change(ISIS_DISABLE, circuit, area);
		circuit->circ_type = CIRCUIT_T_P2P;
		circuit->circ_type_config = CIRCUIT_T_P2P;
		isis_csm_state_change(ISIS_ENABLE, circuit, area);
	}

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoNetwork,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no isis network point-to-point",
       "Negate a command or set its defaults",
       "IS-IS commands",
       "Set network type for circuit",
       "point-to-point network type")
{
	struct interface *ifp;
	struct isis_circuit *circuit = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface\n");
		return (CMD_IPC_ERROR);
	}

	circuit = circuit_scan_by_ifp(ifp);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s\n", ifp->name);
		return (CMD_IPC_ERROR);
	}

	/* RFC5309 section 4 */
	if (circuit->circ_type == CIRCUIT_T_BROADCAST)
		return (CMD_IPC_OK);

	if (circuit->state != C_STATE_UP) {
		circuit->circ_type = CIRCUIT_T_BROADCAST;
		circuit->circ_type_config = CIRCUIT_T_BROADCAST;
	} else {
		struct isis_area *area = circuit->area;
		if (circuit->interface && !if_is_broadcast(circuit->interface)) {
			cmdPrint(cmsh, "no isis network point-to-point "
					"is valid only on broadcast interfaces\n");
			return (CMD_IPC_ERROR);
		}

		isis_csm_state_change(ISIS_DISABLE, circuit, area);
		circuit->circ_type = CIRCUIT_T_BROADCAST;
		circuit->circ_type_config = CIRCUIT_T_BROADCAST;
		isis_csm_state_change(ISIS_ENABLE, circuit, area);
	}

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisShowTopology,
       CMD_NODE_EXEC,
       IPC_ISIS | IPC_SHOW_MGR,
       "show isis topology",
       "Show running system information",
       "IS-IS information",
       "IS-IS paths to Intermediate Systems")
{
	struct listnode *node;
	struct isis_area *area;
	int level;

	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

	if (!isis->area_list || isis->area_list->count == 0)
		return (CMD_IPC_OK);

	for (ALL_LIST_ELEMENTS_RO(isis->area_list, node, area)) {

		cmdPrint(cmsh, "Area %s:\n", area->area_tag ? area->area_tag : "null");

		for (level = 0; level < ISIS_LEVELS; level++) {
			if (area->ip_circuits > 0 && area->spftree[level]
					&& area->spftree[level]->paths->count > 0) {
				cmdPrint(cmsh, "IS-IS paths to level-%d routers that speak IP\n", level + 1);
				isisPrintPaths(cmsh, area->spftree[level]->paths, isis->sysid);
				cmdPrint(cmsh, "\n");
			}
#if 0 //HAVE_IPV6
			if (area->ipv6_circuits > 0 && area->spftree6[level]
					&& area->spftree6[level]->paths->count > 0)
			{
				cmdPrint (cmsh,
						"IS-IS paths to level-%d routers that speak IPv6%s",
						level + 1, VTY_NEWLINE);
				isis_print_paths (cmsh, area->spftree6[level]->paths, isis->sysid);
				cmdPrint (cmsh, "%s", VTY_NEWLINE);
			}
#endif /* HAVE_IPV6 */
		}
		cmdPrint(cmsh, "\n");
	}

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisShowTopologyL1,
	   CMD_NODE_EXEC,
	   IPC_ISIS | IPC_SHOW_MGR,
       "show isis topology level-1",
       "Show running system information",
       "IS-IS information",
       "IS-IS paths to Intermediate Systems",
       "Paths to all level-1 routers in the area")
{
	struct listnode *node;
	struct isis_area *area;

	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

	if (!isis->area_list || isis->area_list->count == 0)
		return (CMD_IPC_OK);

	for (ALL_LIST_ELEMENTS_RO(isis->area_list, node, area)) {
		cmdPrint(cmsh, "Area %s:\n", area->area_tag ? area->area_tag : "null");

		if (area->ip_circuits > 0 && area->spftree[0]
				&& area->spftree[0]->paths->count > 0) {
			cmdPrint(cmsh, "IS-IS paths to level-1 routers that speak IP\n");
			isisPrintPaths(cmsh, area->spftree[0]->paths, isis->sysid);
			cmdPrint(cmsh, "\n");
		}
#if 0 //HAVE_IPV6
		if (area->ipv6_circuits > 0 && area->spftree6[0]
				&& area->spftree6[0]->paths->count > 0) {
			cmdPrint(cmsh, "IS-IS paths to level-1 routers that speak IPv6%s",
					VTY_NEWLINE);
			isis_print_paths(cmsh, area->spftree6[0]->paths, isis->sysid);
			cmdPrint(cmsh, "%s", VTY_NEWLINE);
		}
#endif /* HAVE_IPV6 */
		cmdPrint(cmsh, "/n");
	}

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisShowTopologyL2,
	   CMD_NODE_EXEC,
	   IPC_ISIS | IPC_SHOW_MGR,
       "show isis topology level-2",
       "Show running system information",
       "IS-IS information",
       "IS-IS paths to Intermediate Systems",
       "Paths to all level-2 routers in the domain")
{
	struct listnode *node;
	struct isis_area *area;

	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

	if (!isis->area_list || isis->area_list->count == 0)
		return (CMD_IPC_OK);

	for (ALL_LIST_ELEMENTS_RO(isis->area_list, node, area)) {
		cmdPrint(cmsh, "Area %s:\n", area->area_tag ? area->area_tag : "null");

		if (area->ip_circuits > 0 && area->spftree[1]
				&& area->spftree[1]->paths->count > 0) {
			cmdPrint(cmsh, "IS-IS paths to level-2 routers that speak IP\n");
			isisPrintPaths(cmsh, area->spftree[1]->paths, isis->sysid);
			cmdPrint(cmsh, "\n");
		}
#if 0 //HAVE_IPV6
		if (area->ipv6_circuits > 0 && area->spftree6[1]
				&& area->spftree6[1]->paths->count > 0) {
			cmdPrint(cmsh, "IS-IS paths to level-2 routers that speak IPv6%s",
					VTY_NEWLINE);
			isis_print_paths(cmsh, area->spftree6[1]->paths, isis->sysid);
			cmdPrint(cmsh, "%s", VTY_NEWLINE);
		}
#endif /* HAVE_IPV6 */
		cmdPrint(cmsh, "\n");
	}

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisShowInterface,
		CMD_NODE_EXEC,
	   IPC_ISIS | IPC_SHOW_MGR,
       "show isis interface",
       "Show running system information",
       "ISIS network information",
       "ISIS interface")
{
	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

  return showIsisInterfaceCommon (cmsh, NULL, ISIS_UI_LEVEL_BRIEF);
}

DECMD (cmdFuncIsisShowInterfaceDetail,
		CMD_NODE_EXEC,
	   IPC_ISIS | IPC_SHOW_MGR,
       "show isis interface detail",
       "Show running system information",
       "ISIS network information",
       "ISIS interface",
       "show detailed information")
{
	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

  return showIsisInterfaceCommon (cmsh, NULL, ISIS_UI_LEVEL_DETAIL);
}

DECMD (cmdFuncIsisShowInterfaceArg,
		CMD_NODE_EXEC,
	   IPC_ISIS | IPC_SHOW_MGR,
       "show isis interface WORD",
       "Show running system information",
       "ISIS network information",
       "ISIS interface",
       "ISIS interface name")
{
	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

  return showIsisInterfaceCommon (cmsh, cargv[3], ISIS_UI_LEVEL_DETAIL);
}

DECMD (cmdFuncIsisShowNeighbor,
		CMD_NODE_EXEC,
	   IPC_ISIS | IPC_SHOW_MGR,
       "show isis neighbor",
       "Show running system information",
       "ISIS network information",
       "ISIS neighbor adjacencies")
{
	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

  return showIsisNeighborCommon (cmsh, NULL, ISIS_UI_LEVEL_BRIEF);
}

DECMD (cmdFuncIsisShowNeighborDetail,
		CMD_NODE_EXEC,
	   IPC_ISIS | IPC_SHOW_MGR,
       "show isis neighbor detail",
       "Show running system information",
       "ISIS network information",
       "ISIS neighbor adjacencies",
       "show detailed information")
{
	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

  return showIsisNeighborCommon (cmsh, NULL, ISIS_UI_LEVEL_DETAIL);
}

DECMD (cmdFuncIsisShowNeighborArg,
		CMD_NODE_EXEC,
	   IPC_ISIS | IPC_SHOW_MGR,
       "show isis neighbor WORD",
       "Show running system information",
       "ISIS network information",
       "ISIS neighbor adjacencies",
       "System id")
{
	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

  return showIsisNeighborCommon (cmsh, cargv[3], ISIS_UI_LEVEL_DETAIL);
}

DECMD (cmdFuncIsisClearNeighbor,
		CMD_NODE_EXEC,
	   IPC_ISIS,
       "clear isis neighbor",
       "Reset functions",
       "Reset ISIS network information",
       "Reset ISIS neighbor adjacencies")
{
	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

  return clearIsisNeighborCommon (cmsh, NULL);
}

DECMD (cmdFuncIsisClearNeighborArg,
		CMD_NODE_EXEC,
	   IPC_ISIS,
       "clear isis neighbor WORD",
       "Reset functions",
       "ISIS network information",
       "ISIS neighbor adjacencies",
       "System id")
{
	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

  return clearIsisNeighborCommon (cmsh, cargv[3]);
}

DECMD (cmdFuncIsisShowHostname,
       CMD_NODE_EXEC,
       IPC_ISIS | IPC_SHOW_MGR,
       "show isis hostname",
       "Show running system information",
       "IS-IS information",
       "IS-IS Dynamic hostname mapping")
{
	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

  dynhnPrintAll (cmsh);

  return (CMD_IPC_OK);
}


DECMD (cmdFuncIsisShowSummary,
	   CMD_NODE_EXEC,
	   IPC_ISIS | IPC_SHOW_MGR,
       "show isis summary",
       "Show running system information",
       "IS-IS information",
       "IS-IS summary")
{
	struct listnode *node, *node2;
	struct isis_area *area;
	struct isis_spftree *spftree;
	int level;

	Int32T idx = 0;
	Int8T printBuff[1024] = { };

	if (isis == NULL) {
		cmdPrint(cmsh, "ISIS is not running\n");
		return (CMD_IPC_OK);
	}

	cmdPrint(cmsh, "Process Id      : %ld", isis->process_id);
	if (isis->sysid_set)
		cmdPrint(cmsh, "System Id       : %s", sysid_print(isis->sysid));

	idx += sprintf(printBuff + idx, "Up time         : ");
	cmdPrintTimestr(printBuff, &idx, isis->uptime);

	cmdPrint(cmsh, "%s", printBuff);
	idx = 0;
	memset(printBuff, 0, 1024);

	if (isis->area_list)
		cmdPrint(cmsh, "Number of areas : %d", isis->area_list->count);

	for (ALL_LIST_ELEMENTS_RO(isis->area_list, node, area)) {
		cmdPrint(cmsh, "Area %s:", area->area_tag ? area->area_tag : "null");

		if (listcount (area->area_addrs) > 0) {
			struct area_addr *area_addr;
			for (ALL_LIST_ELEMENTS_RO(area->area_addrs, node2, area_addr)) {
				cmdPrint(cmsh, "  Net: %s",
						isonet_print (area_addr->area_addr, area_addr->addr_len + ISIS_SYS_ID_LEN + 1));
			}
		}

		for (level = ISIS_LEVEL1; level <= ISIS_LEVELS; level++) {
			if ((area->is_type & level) == 0)
				continue;

			cmdPrint(cmsh, "  Level-%d:", level);
			spftree = area->spftree[level - 1];
			if (spftree->pending) {
				cmdPrint(cmsh, "    IPv4 SPF: (pending)");
			} else {
				cmdPrint(cmsh, "    IPv4 SPF:");
			}

			cmdPrint(cmsh, "      minimum interval  : %d",
					area->min_spf_interval[level - 1]);

			idx += sprintf(printBuff + idx, "      last run elapsed  : ");
			cmdPrintTimestr(printBuff, &idx, spftree->last_run_timestamp);

			cmdPrint(cmsh, "%s", printBuff);
			idx = 0;
			memset(printBuff, 0, 1024);

			cmdPrint(cmsh, "      last run duration : %u usec",
					(u_int32_t )spftree->last_run_duration);
			cmdPrint(cmsh, "      run count         : %d",
					spftree->runcount);

#if 0// HAVE_IPV6
			spftree = area->spftree6[level - 1];
			if (spftree->pending)
			cmdPrint (cmsh, "    IPv6 SPF: (pending)%s", VTY_NEWLINE);
			else
			cmdPrint(cmsh, "    IPv6 SPF:%s", VTY_NEWLINE);

			cmdPrint(cmsh, "      minimum interval  : %d%s",
					area->min_spf_interval[level - 1], VTY_NEWLINE);

			cmdPrint(cmsh, "      last run elapsed  : ");
			cmdPrintTimestr(cmsh, spftree->last_run_timestamp);
			cmdPrint(cmsh, "%s", VTY_NEWLINE);

			cmdPrint(cmsh, "      last run duration : %u msec%s",
					spftree->last_run_duration, VTY_NEWLINE);

			cmdPrint(cmsh, "      run count         : %d%s", spftree->runcount,
					VTY_NEWLINE);
#endif
		}
	}
	cmdPrint(cmsh, "\n");

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisShowDatabaseBrief,
	   CMD_NODE_EXEC,
	   IPC_ISIS | IPC_SHOW_MGR,
       "show isis database",
       "Show running system information",
       "IS-IS information",
       "IS-IS link state database")
{
	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

  return showIsisDatabase (cmsh, NULL, ISIS_UI_LEVEL_BRIEF);
}

DECMD (cmdFuncIsisShowDatabaseLspBrief,
		CMD_NODE_EXEC,
	   IPC_ISIS | IPC_SHOW_MGR,
       "show isis database WORD",
       "Show running system information",
       "IS-IS information",
       "IS-IS link state database",
       "LSP ID")
{
	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

  return showIsisDatabase (cmsh, cargv[3], ISIS_UI_LEVEL_BRIEF);
}

DECMD (cmdFuncIsisShowDatabaseLspDetail,
		CMD_NODE_EXEC,
	   IPC_ISIS | IPC_SHOW_MGR,
       "show isis database WORD detail",
       "Show running system information",
       "IS-IS information",
       "IS-IS link state database",
       "LSP ID",
       "Detailed information")
{
	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

  return showIsisDatabase (cmsh, cargv[3], ISIS_UI_LEVEL_DETAIL);
}

DECMD (cmdFuncIsisShowDatabaseDetail,
	   CMD_NODE_EXEC,
	   IPC_ISIS | IPC_SHOW_MGR,
       "show isis database detail",
       "Show running system information",
       "IS-IS information",
       "IS-IS link state database",
       "Detailed information")
{
	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

  return showIsisDatabase (cmsh, NULL, ISIS_UI_LEVEL_DETAIL);
}

DECMD (cmdFuncIsisShowDatabaseDetailLsp,
	   CMD_NODE_EXEC,
	   IPC_ISIS | IPC_SHOW_MGR,
       "show isis database detail WORD",
       "Show running system information",
       "IS-IS information",
       "IS-IS link state database",
       "Detailed information",
       "LSP ID")
{
	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

  return showIsisDatabase (cmsh, cargv[4], ISIS_UI_LEVEL_DETAIL);
}

/*
 *'no router isis' command
 */
DECMD (cmdFuncIsisNoRouterIsis,
       CMD_NODE_CONFIG,
       IPC_ISIS,
       "no router isis WORD",
       "no",
       "Enable a routing process",
       "ISO IS-IS",
       "ISO Routing area tag")
{
	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

  return isisAreaDestroy (cmsh, cargv[3]);
}

/*
 * 'net' command
 */
DECMD (cmdFuncIsisNet,
       CMD_NODE_CONFIG_ISIS,
       IPC_ISIS,
       "net WORD",
       "A Network Entity Title for this process (OSI only)",
       "XX.XXXX. ... .XXX.XX  Network entity title (NET)")
{
	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

  return areaNetTitle (cmsh, cargv[1], uargv1[2]);
}

/*
 * 'no net' command
 */
DECMD (cmdFuncIsisNoNet,
	   CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no net WORD",
       "Negate a command or set its defaults",
       "A Network Entity Title for this process (OSI only)",
       "XX.XXXX. ... .XXX.XX  Network entity title (NET)")
{
	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

  return areaClearNetTitle (cmsh, cargv[2], uargv1[2]);
}

DECMD (cmdFuncIsisAreaPasswdMd5,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "area-password md5 WORD",
       "Configure the authentication password for an area",
       "Authentication type",
       "Area password")
{
	struct isis_area *area;
	int len;

	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

	area = isisAreaGet(uargv1[2]);

	if (!area) {
		cmdPrint(cmsh, "Can't find IS-IS instance\n");
		return (CMD_IPC_ERROR);
	}

	len = strlen(cargv[2]);
	if (len > 254) {
		cmdPrint(cmsh, "Too long area password (>254)\n");
		return (CMD_IPC_ERROR);
	}

	area->area_passwd.len = (u_char) len;
	area->area_passwd.type = ISIS_PASSWD_TYPE_HMAC_MD5;
	strncpy((char *) area->area_passwd.passwd, cargv[2], 255);

	if (cargc > 3) {
		SET_FLAG(area->area_passwd.snp_auth, SNP_AUTH_SEND);
		if (strncmp(cargv[5], "v", 1) == 0)
			SET_FLAG(area->area_passwd.snp_auth, SNP_AUTH_RECV);
		else
			UNSET_FLAG(area->area_passwd.snp_auth, SNP_AUTH_RECV);
	} else {
		UNSET_FLAG(area->area_passwd.snp_auth, SNP_AUTH_SEND);
		UNSET_FLAG(area->area_passwd.snp_auth, SNP_AUTH_RECV);
	}
	lsp_regenerate_schedule(area, IS_LEVEL_1 | IS_LEVEL_2, 1);

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisAreaPasswdMd5,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "area-password md5 WORD authenticate snp (send-only|validate)",
       "Configure the authentication password for an area",
       "Authentication type",
       "Area password",
       "Authentication",
       "SNP PDUs",
       "Send but do not check PDUs on receiving",
       "Send and check PDUs on receiving");

DECMD (cmdFuncIsisAreaPasswdClear,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "area-password clear WORD",
       "Configure the authentication password for an area",
       "Authentication type",
       "Area password")
{
	struct isis_area *area;
	int len;

	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

	area = isisAreaGet(uargv1[2]);

	if (!area) {
		cmdPrint(cmsh, "Can't find IS-IS instance\n");
		return (CMD_IPC_ERROR);
	}

	len = strlen(cargv[2]);
	if (len > 254) {
		cmdPrint(cmsh, "Too long area password (>254)\n");
		return (CMD_IPC_ERROR);
	}

	area->area_passwd.len = (u_char) len;
	area->area_passwd.type = ISIS_PASSWD_TYPE_CLEARTXT;
	strncpy((char *) area->area_passwd.passwd, cargv[2], 255);

	if (cargc > 3) {
		SET_FLAG(area->area_passwd.snp_auth, SNP_AUTH_SEND);
		if (strncmp(cargv[5], "v", 1) == 0)
			SET_FLAG(area->area_passwd.snp_auth, SNP_AUTH_RECV);
		else
			UNSET_FLAG(area->area_passwd.snp_auth, SNP_AUTH_RECV);
	} else {
		UNSET_FLAG(area->area_passwd.snp_auth, SNP_AUTH_SEND);
		UNSET_FLAG(area->area_passwd.snp_auth, SNP_AUTH_RECV);
	}
	lsp_regenerate_schedule(area, IS_LEVEL_1 | IS_LEVEL_2, 1);

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisAreaPasswdClear,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "area-password clear WORD authenticate snp (send-only|validate)",
       "Configure the authentication password for an area",
       "Authentication type",
       "Area password",
       "Authentication",
       "SNP PDUs",
       "Send but do not check PDUs on receiving",
       "Send and check PDUs on receiving");

DECMD (cmdFuncIsisNoAreaPasswd,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no area-password",
       "Negate a command or set its defaults",
       "Configure the authentication password for an area")
{
	struct isis_area *area;

	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

	area = isisAreaGet(uargv1[2]);

	if (!area) {
		cmdPrint(cmsh, "Can't find IS-IS instance\n");
		return (CMD_IPC_ERROR);
	}

	memset(&area->area_passwd, 0, sizeof(struct isis_passwd));
	lsp_regenerate_schedule(area, IS_LEVEL_1 | IS_LEVEL_2, 1);

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisDomainPasswdMd5,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "domain-password md5 WORD",
       "Set the authentication password for a routing domain",
       "Authentication type",
       "Routing domain password")
{
	struct isis_area *area;
	int len;

	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

	area = isisAreaGet(uargv1[2]);

	if (!area) {
		cmdPrint(cmsh, "Can't find IS-IS instance\n");
		return CMD_IPC_ERROR;
	}

	len = strlen(cargv[2]);
	if (len > 254) {
		cmdPrint(cmsh, "Too long area password (>254)\n");
		return (CMD_IPC_ERROR);
	}

	area->domain_passwd.len = (u_char) len;
	area->domain_passwd.type = ISIS_PASSWD_TYPE_HMAC_MD5;
	strncpy((char *) area->domain_passwd.passwd, cargv[2], 255);

	if (cargc > 3) {
		SET_FLAG(area->domain_passwd.snp_auth, SNP_AUTH_SEND);
		if (strncmp(cargv[5], "v", 1) == 0)
			SET_FLAG(area->domain_passwd.snp_auth, SNP_AUTH_RECV);
		else
			UNSET_FLAG(area->domain_passwd.snp_auth, SNP_AUTH_RECV);
	} else {
		UNSET_FLAG(area->domain_passwd.snp_auth, SNP_AUTH_SEND);
		UNSET_FLAG(area->domain_passwd.snp_auth, SNP_AUTH_RECV);
	}
	lsp_regenerate_schedule(area, IS_LEVEL_1 | IS_LEVEL_2, 1);

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisDomainPasswdMd5,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "domain-password md5 WORD authenticate snp (send-only|validate)",
       "Set the authentication password for a routing domain",
       "Authentication type",
       "Routing domain password",
       "Authentication",
       "SNP PDUs",
       "Send but do not check PDUs on receiving",
       "Send and check PDUs on receiving");

DECMD (cmdFuncIsisDomainPasswdClear,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "domain-password clear WORD",
       "Set the authentication password for a routing domain",
       "Authentication type",
       "Routing domain password")
{
	struct isis_area *area;
	int len;

	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

	area = isisAreaGet(uargv1[2]);

	if (!area) {
		cmdPrint(cmsh, "Can't find IS-IS instance\n");
		return CMD_IPC_ERROR;
	}

	len = strlen(cargv[2]);
	if (len > 254) {
		cmdPrint(cmsh, "Too long area password (>254)\n");
		return (CMD_IPC_ERROR);
	}

	area->domain_passwd.len = (u_char) len;
	area->domain_passwd.type = ISIS_PASSWD_TYPE_CLEARTXT;
	strncpy((char *) area->domain_passwd.passwd, cargv[2], 255);

	if (cargc > 3) {
		SET_FLAG(area->domain_passwd.snp_auth, SNP_AUTH_SEND);
		if (strncmp(cargv[5], "v", 1) == 0)
			SET_FLAG(area->domain_passwd.snp_auth, SNP_AUTH_RECV);
		else
			UNSET_FLAG(area->domain_passwd.snp_auth, SNP_AUTH_RECV);
	} else {
		UNSET_FLAG(area->domain_passwd.snp_auth, SNP_AUTH_SEND);
		UNSET_FLAG(area->domain_passwd.snp_auth, SNP_AUTH_RECV);
	}
	lsp_regenerate_schedule(area, IS_LEVEL_1 | IS_LEVEL_2, 1);

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisDomainPasswdClear,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "domain-password clear WORD authenticate snp (send-only|validate)",
       "Set the authentication password for a routing domain",
       "Authentication type",
       "Routing domain password",
       "Authentication",
       "SNP PDUs",
       "Send but do not check PDUs on receiving",
       "Send and check PDUs on receiving");

DECMD (cmdFuncIsisNoDomainPasswd,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no domain-password",
       "Negate a command or set its defaults",
       "Set the authentication password for a routing domain")
{
	struct isis_area *area;

	/* Check isis is installed or not. */
		if (!isis)
		{
			cmdPrint(cmsh, "Not installed isis.\n");
			return (CMD_IPC_ERROR);
		}

	area = isisAreaGet(uargv1[2]);

	if (!area) {
		cmdPrint(cmsh, "Can't find IS-IS instance\n");
		return CMD_IPC_ERROR;
	}

	memset(&area->domain_passwd, 0, sizeof(struct isis_passwd));
	lsp_regenerate_schedule(area, IS_LEVEL_1 | IS_LEVEL_2, 1);

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisIsType,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "is-type (level-1|level-1-2|level-2-only)",
       "IS Level for this routing process (OSI only)",
       "Act as a station router only",
       "Act as both a station router and an area router",
       "Act as an area router only")
{
  struct isis_area *area;
  int type;

  /* Check isis is installed or not. */
  	if (!isis)
  	{
  		cmdPrint(cmsh, "Not installed isis.\n");
  		return (CMD_IPC_ERROR);
  	}

  area = isisAreaGet(uargv1[2]);

  if (!area)
    {
      cmdPrint (cmsh, "Can't find IS-IS instance\n");
      return CMD_IPC_ERROR;
    }

  type = string2circuit_t (cargv[1]);
  if (!type)
    {
      cmdPrint (cmsh, "Unknown IS level \n");
      return (CMD_IPC_OK);
    }

  isis_event_system_type_change (area, type);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoIsType,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no is-type (level-1|level-1-2|level-2-only)",
       "Negate a command or set its defaults",
       "IS Level for this routing process (OSI only)",
       "Act as a station router only",
       "Act as both a station router and an area router",
       "Act as an area router only")
{
  struct isis_area *area;
  int type;

  /* Check isis is installed or not. */
  	if (!isis)
  	{
  		cmdPrint(cmsh, "Not installed isis.\n");
  		return (CMD_IPC_ERROR);
  	}

  area = isisAreaGet(uargv1[2]);
  assert (area);

  /*
   * Put the is-type back to defaults:
   * - level-1-2 on first area
   * - level-1 for the rest
   */
  if (listgetdata (listhead (isis->area_list)) == area)
    type = IS_LEVEL_1_AND_2;
  else
    type = IS_LEVEL_1;

  isis_event_system_type_change (area, type);

  return (CMD_IPC_OK);
}


DECMD (cmdFuncIsisLspGenInterval,
       CMD_NODE_CONFIG_ISIS,
       IPC_ISIS,
       "lsp-gen-interval <1-120>",
       "Minimum interval between regenerating same LSP",
       "Minimum interval in seconds")
{
	struct isis_area *area;
	uint16_t interval;
	int level;

	area = isisAreaGet(uargv1[2]);
	interval = atoi(cargv[1]);
	level = IS_LEVEL_1 | IS_LEVEL_2;
	return setLspGenInterval(cmsh, area, interval, level);
}

DECMD (cmdFuncIsisNoLspGenInterval,
	   CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no lsp-gen-interval",
       "Negate a command or set its defaults",
       "Minimum interval between regenerating same LSP")
{
	struct isis_area *area;
	uint16_t interval;
	int level;

	area = isisAreaGet(uargv1[2]);
	interval = DEFAULT_MIN_LSP_GEN_INTERVAL;
	level = IS_LEVEL_1 | IS_LEVEL_2;
	return setLspGenInterval(cmsh, area, interval, level);
}

ALICMD (cmdFuncIsisNoLspGenInterval,
	   CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no lsp-gen-interval <1-120>",
       "Negate a command or set its defaults",
       "Minimum interval between regenerating same LSP",
       "Minimum interval in seconds");

DECMD (cmdFuncIsisLspGenIntervalL1,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "lsp-gen-interval level-1 <1-120>",
       "Minimum interval between regenerating same LSP",
       "Set interval for level 1 only",
       "Minimum interval in seconds")
{
	struct isis_area *area;
	uint16_t interval;
	int level;

	area = isisAreaGet(uargv1[2]);
	interval = atoi(cargv[2]);
	level = IS_LEVEL_1;
	return setLspGenInterval(cmsh, area, interval, level);
}

DECMD (cmdFuncIsisNoLspGenIntervalL1,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no lsp-gen-interval level-1",
       "Negate a command or set its defaults",
       "Minimum interval between regenerating same LSP",
       "Set interval for level 1 only")
{
	struct isis_area *area;
	uint16_t interval;
	int level;

	area = isisAreaGet(uargv1[2]);
	interval = DEFAULT_MIN_LSP_GEN_INTERVAL;
	level = IS_LEVEL_1;
	return setLspGenInterval(cmsh, area, interval, level);
}

ALICMD (cmdFuncIsisNoLspGenIntervalL1,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no lsp-gen-interval level-1 <1-120>",
       "Negate a command or set its defaults",
       "Minimum interval between regenerating same LSP",
       "Set interval for level 1 only",
       "Minimum interval in seconds");

DECMD (cmdFuncIsisLspGenIntervalL2,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "lsp-gen-interval level-2 <1-120>",
       "Minimum interval between regenerating same LSP",
       "Set interval for level 2 only",
       "Minimum interval in seconds")
{
	struct isis_area *area;
	uint16_t interval;
	int level;

	area = isisAreaGet(uargv1[2]);
	interval = atoi(cargv[2]);
	level = IS_LEVEL_2;
	return setLspGenInterval(cmsh, area, interval, level);
}

DECMD (cmdFuncIsisNoLspGenIntervalL2,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no lsp-gen-interval level-2",
       "Negate a command or set its defaults",
       "Minimum interval between regenerating same LSP",
       "Set interval for level 2 only")
{
  struct isis_area *area;
  uint16_t interval;
  int level;

  area = isisAreaGet(uargv1[2]);
  interval = DEFAULT_MIN_LSP_GEN_INTERVAL;
  level = IS_LEVEL_2;
  return setLspGenInterval (cmsh, area, interval, level);
}

ALICMD (cmdFuncIsisNoLspGenIntervalL2,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no lsp-gen-interval level-2 <1-120>",
       "Negate a command or set its defaults",
       "Minimum interval between regenerating same LSP",
       "Set interval for level 2 only",
       "Minimum interval in seconds");

DECMD (cmdFuncIsisMetricStyle,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "metric-style (narrow|transition|wide)",
       "Use old-style (ISO 10589) or new-style packet formats",
       "Use old style of TLVs with narrow metric",
       "Send and accept both styles of TLVs during transition",
       "Use new style of TLVs to carry wider metric")
{
	struct isis_area *area;
	int ret;

	area = isisAreaGet(uargv1[2]);
	assert(area);

	if (strncmp(cargv[1], "w", 1) == 0) {
		area->newmetric = 1;
		area->oldmetric = 0;
	} else {
		ret = validateMetricStyleNarrow(cmsh, area);
		if (ret != (CMD_IPC_OK))
			return ret;

		if (strncmp(cargv[1], "t", 1) == 0) {
			area->newmetric = 1;
			area->oldmetric = 1;
		} else if (strncmp(cargv[1], "n", 1) == 0) {
			area->newmetric = 0;
			area->oldmetric = 1;
		}
	}

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoMetricStyle,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no metric-style",
       "Negate a command or set its defaults",
       "Use old-style (ISO 10589) or new-style packet formats")
{
  struct isis_area *area;
  int ret;

  area = isisAreaGet(uargv1[2]);
  assert (area);

  ret = validateMetricStyleNarrow (cmsh, area);
  if (ret != (CMD_IPC_OK))
    return ret;

  /* Default is narrow metric. */
  area->newmetric = 0;
  area->oldmetric = 1;

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisSetOverloadBit,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "set-overload-bit",
       "Set overload bit to avoid any transit traffic",
       "Set overload bit")
{
  struct isis_area *area;

  area = isisAreaGet(uargv1[2]);
  assert (area);

  area->overload_bit = LSPBIT_OL;
  lsp_regenerate_schedule (area, IS_LEVEL_1 | IS_LEVEL_2, 1);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoSetOverloadBit,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no set-overload-bit",
       "Reset overload bit to accept transit traffic",
       "Reset overload bit")
{
  struct isis_area *area;

  area = isisAreaGet(uargv1[2]);
  assert (area);

  area->overload_bit = 0;
  lsp_regenerate_schedule (area, IS_LEVEL_1 | IS_LEVEL_2, 1);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisDynamicHostname,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "hostname dynamic",
       "Dynamic hostname for IS-IS",
       "Dynamic hostname")
{
	struct isis_area *area;

	area = isisAreaGet(uargv1[2]);
	assert(area);

	if (!area->dynhostname) {
		area->dynhostname = 1;
		lsp_regenerate_schedule(area, IS_LEVEL_1 | IS_LEVEL_2, 0);
	}

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoDynamicHostname,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no hostname dynamic",
       "Negate a command or set its defaults",
       "Dynamic hostname for IS-IS",
       "Dynamic hostname")
{
	struct isis_area *area;

	area = isisAreaGet(uargv1[2]);
	assert(area);

	if (area->dynhostname) {
		area->dynhostname = 0;
		lsp_regenerate_schedule(area, IS_LEVEL_1 | IS_LEVEL_2, 0);
	}

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisSpfInterval,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "spf-interval <1-120>",
       "Minimum interval between SPF calculations",
       "Minimum interval between consecutive SPFs in seconds")
{
	struct isis_area *area;
	u_int16_t interval;

	area = isisAreaGet(uargv1[2]);
	interval = atoi(cargv[1]);
	area->min_spf_interval[0] = interval;
	area->min_spf_interval[1] = interval;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoSpfInterval,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no spf-interval",
       "Negate a command or set its defaults",
       "Minimum interval between SPF calculations")
{
	struct isis_area *area;

	area = isisAreaGet(uargv1[2]);

	area->min_spf_interval[0] = MINIMUM_SPF_INTERVAL;
	area->min_spf_interval[1] = MINIMUM_SPF_INTERVAL;

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisNoSpfInterval,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no spf-interval <1-120>",
       "Negate a command or set its defaults",
       "Minimum interval between SPF calculations",
       "Minimum interval between consecutive SPFs in seconds");

DECMD (cmdFuncIsisSpfIntervalL1,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "spf-interval level-1 <1-120>",
       "Minimum interval between SPF calculations",
       "Set interval for level 1 only",
       "Minimum interval between consecutive SPFs in seconds")
{
	struct isis_area *area;
	u_int16_t interval;

	area = isisAreaGet(uargv1[2]);
	interval = atoi(cargv[2]);
	area->min_spf_interval[0] = interval;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoSpfIntervalL1,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no spf-interval level-1",
       "Negate a command or set its defaults",
       "Minimum interval between SPF calculations",
       "Set interval for level 1 only")
{
	struct isis_area *area;

	area = isisAreaGet(uargv1[2]);

	area->min_spf_interval[0] = MINIMUM_SPF_INTERVAL;

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisNoSpfIntervalL1,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no spf-interval level-1 <1-120>",
       "Negate a command or set its defaults",
       "Minimum interval between SPF calculations",
       "Set interval for level 1 only",
       "Minimum interval between consecutive SPFs in seconds");

DECMD (cmdFuncIsisSpfIntervalL2,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "spf-interval level-2 <1-120>",
       "Minimum interval between SPF calculations",
       "Set interval for level 2 only",
       "Minimum interval between consecutive SPFs in seconds")
{
	struct isis_area *area;
	u_int16_t interval;

	area = isisAreaGet(uargv1[2]);
	interval = atoi(cargv[2]);
	area->min_spf_interval[1] = interval;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoSpfIntervalL2,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no spf-interval level-2",
       "Negate a command or set its defaults",
       "Minimum interval between SPF calculations",
       "Set interval for level 2 only")
{
	struct isis_area *area;

	area = isisAreaGet(uargv1[2]);

	area->min_spf_interval[1] = MINIMUM_SPF_INTERVAL;

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncIsisNoSpfIntervalL2,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no spf-interval level-2 <1-120>",
       "Negate a command or set its defaults",
       "Minimum interval between SPF calculations",
       "Set interval for level 2 only",
       "Minimum interval between consecutive SPFs in seconds");


DECMD (cmdFuncIsisMaxLspLifetime,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "max-lsp-lifetime <350-65535>",
       "Maximum LSP lifetime",
       "LSP lifetime in seconds")
{
  struct isis_area *area;
  uint16_t interval;
  int level;

  area = isisAreaGet(uargv1[2]);
  interval = atoi (cargv[1]);
  level = IS_LEVEL_1 | IS_LEVEL_2;
  return setLspMaxLifetime (cmsh, area, interval, level);
}

DECMD (cmdFuncIsisNoMaxLspLifetime,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no max-lsp-lifetime",
       "Negate a command or set its defaults",
       "LSP lifetime in seconds")
{
	struct isis_area *area;
	uint16_t interval;
	int level;

	area = isisAreaGet(uargv1[2]);
	interval = DEFAULT_LSP_LIFETIME;
	level = IS_LEVEL_1 | IS_LEVEL_2;
	return setLspMaxLifetime(cmsh, area, interval, level);
}

ALICMD (cmdFuncIsisNoMaxLspLifetime,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no max-lsp-lifetime <350-65535>",
       "Negate a command or set its defaults",
       "Maximum LSP lifetime",
       "LSP lifetime in seconds");

DECMD (cmdFuncIsisMaxLspLifetimeL1,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "max-lsp-lifetime level-1 <350-65535>",
       "Maximum LSP lifetime",
       "Maximum LSP lifetime for Level 1 only",
       "LSP lifetime for Level 1 only in seconds")
{
	struct isis_area *area;
	uint16_t interval;
	int level;

	area = isisAreaGet(uargv1[2]);
	interval = atoi(cargv[2]);
	level = IS_LEVEL_1;
	return setLspMaxLifetime(cmsh, area, interval, level);
}

DECMD (cmdFuncIsisNoMaxLspLifetimeL1,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no max-lsp-lifetime level-1",
       "Negate a command or set its defaults",
       "LSP lifetime in seconds",
       "LSP lifetime for Level 1 only in seconds")
{
	struct isis_area *area;
	uint16_t interval;
	int level;

	area = isisAreaGet(uargv1[2]);
	interval = DEFAULT_LSP_LIFETIME;
	level = IS_LEVEL_1;
	return setLspMaxLifetime(cmsh, area, interval, level);
}

ALICMD (cmdFuncIsisNoMaxLspLifetimeL1,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no max-lsp-lifetime level-1 <350-65535>",
       "Negate a command or set its defaults",
       "Maximum LSP lifetime",
       "Maximum LSP lifetime for Level 1 only",
       "LSP lifetime for Level 1 only in seconds");

DECMD (cmdFuncIsisMaxLspLifetimeL2,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "max-lsp-lifetime level-2 <350-65535>",
       "Maximum LSP lifetime",
       "Maximum LSP lifetime for Level 2 only",
       "LSP lifetime for Level 2 only in seconds")
{
  struct isis_area *area;
  uint16_t interval;
  int level;

  area = isisAreaGet(uargv1[2]);
  interval = atoi (cargv[2]);
  level = IS_LEVEL_2;
  return setLspMaxLifetime (cmsh, area, interval, level);
}

DECMD (cmdFuncIsisNoMaxLspLifetimeL2,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no max-lsp-lifetime level-2",
       "Negate a command or set its defaults",
       "LSP lifetime in seconds",
       "LSP lifetime for Level 2 only in seconds")
{
	struct isis_area *area;
	uint16_t interval;
	int level;

	area = isisAreaGet(uargv1[2]);
	interval = DEFAULT_LSP_LIFETIME;
	level = IS_LEVEL_2;
	return setLspMaxLifetime(cmsh, area, interval, level);
}

ALICMD (cmdFuncIsisNoMaxLspLifetimeL2,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no max-lsp-lifetime level-2 <350-65535>",
       "Negate a command or set its defaults",
       "Maximum LSP lifetime",
       "Maximum LSP lifetime for Level 2 only",
       "LSP lifetime for Level 2 only in seconds");

DECMD (cmdFuncIsisLspRefreshInterval,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "lsp-refresh-interval <1-65235>",
       "LSP refresh interval",
       "LSP refresh interval in seconds")
{
	struct isis_area *area;
	uint16_t interval;
	int level;

	area = isisAreaGet(uargv1[2]);
	interval = atoi(cargv[1]);
	level = IS_LEVEL_1 | IS_LEVEL_2;
	return setLspRefreshInterval(cmsh, area, interval, level);
}

DECMD (cmdFuncIsisNoLspRefreshInterval,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no lsp-refresh-interval",
       "Negate a command or set its defaults",
       "LSP refresh interval in seconds")
{
	struct isis_area *area;
	uint16_t interval;
	int level;

	area = isisAreaGet(uargv1[2]);
	interval = DEFAULT_MAX_LSP_GEN_INTERVAL;
	level = IS_LEVEL_1 | IS_LEVEL_2;
	return setLspRefreshInterval(cmsh, area, interval, level);
}

ALICMD (cmdFuncIsisNoLspRefreshInterval,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no lsp-refresh-interval <1-65235>",
       "Negate a command or set its defaults",
       "LSP refresh interval",
       "LSP refresh interval in seconds");

DECMD (cmdFuncIsisLspRefreshIntervalL1,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "lsp-refresh-interval level-1 <1-65235>",
       "LSP refresh interval",
       "LSP refresh interval for Level 1 only",
       "LSP refresh interval for Level 1 only in seconds")
{
	struct isis_area *area;
	uint16_t interval;
	int level;

	area = isisAreaGet(uargv1[2]);
	interval = atoi(cargv[2]);
	level = IS_LEVEL_1;
	return setLspRefreshInterval(cmsh, area, interval, level);
}

DECMD (cmdFuncIsisNoLspRefreshIntervalL1,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no lsp-refresh-interval level-1",
       "Negate a command or set its defaults",
       "LSP refresh interval in seconds",
       "LSP refresh interval for Level 1 only in seconds")
{
	struct isis_area *area;
	uint16_t interval;
	int level;

	area = isisAreaGet(uargv1[2]);
	interval = DEFAULT_MAX_LSP_GEN_INTERVAL;
	level = IS_LEVEL_1;
	return setLspRefreshInterval(cmsh, area, interval, level);
}

ALICMD (cmdFuncIsisNoLspRefreshIntervalL1,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no lsp-refresh-interval level-1 <1-65235>",
       "Negate a command or set its defaults",
       "LSP refresh interval",
       "LSP refresh interval for Level 1 only",
       "LSP refresh interval for Level 1 only in seconds");

DECMD (cmdFuncIsisLspRefreshIntervalL2,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "lsp-refresh-interval level-2 <1-65235>",
       "LSP refresh interval",
       "LSP refresh interval for Level 2 only",
       "LSP refresh interval for Level 2 only in seconds")
{
	struct isis_area *area;
	uint16_t interval;
	int level;

	area = isisAreaGet(uargv1[2]);
	interval = atoi(cargv[2]);
	level = IS_LEVEL_2;
	return setLspRefreshInterval(cmsh, area, interval, level);
}

DECMD (cmdFuncIsisNoLspRefreshIntervalL2,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no lsp-refresh-interval level-2",
       "Negate a command or set its defaults",
       "LSP refresh interval in seconds",
       "LSP refresh interval for Level 2 only in seconds")
{
	struct isis_area *area;
	uint16_t interval;
	int level;

	area = isisAreaGet(uargv1[2]);
	interval = DEFAULT_MAX_LSP_GEN_INTERVAL;
	level = IS_LEVEL_2;
	return setLspRefreshInterval(cmsh, area, interval, level);
}

ALICMD (cmdFuncIsisNoLspRefreshIntervalL2,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no lsp-refresh-interval level-2 <1-65235>",
       "Negate a command or set its defaults",
       "LSP refresh interval",
       "LSP refresh interval for Level 2 only",
       "LSP refresh interval for Level 2 only in seconds");

DECMD (cmdFuncIsisLogAdjChanges,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "log-adjacency-changes-isis",
       "Log changes in adjacency state")
{
	struct isis_area *area;

	area = isisAreaGet(uargv1[2]);
	assert(area);

	area->log_adj_changes = 1;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoLogAdjChanges,
		CMD_NODE_CONFIG_ISIS,
	   IPC_ISIS,
       "no log-adjacency-changes-isis",
       "Negate a command or set its defaults",
       "Stop logging changes in adjacency state")
{
	struct isis_area *area;

	area = isisAreaGet(uargv1[2]);
	assert(area);

	area->log_adj_changes = 0;

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisDebugAdj,
       CMD_NODE_CONFIG,
       IPC_ISIS,
       "debug isis adj-packets",
       "Debugging functions (see also 'undebug')",
       "IS-IS information",
       "IS-IS Adjacency related packets")
{
  isis->debugs |= DEBUG_ADJ_PACKETS;
  print_debug (cmsh, DEBUG_ADJ_PACKETS, 1);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoDebugAdj,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "no debug isis adj-packets",
       "Negate a command or set its defaults",
       "Disable debugging functions (see also 'debug')",
       "IS-IS information",
       "IS-IS Adjacency related packets")
{
  isis->debugs &= ~DEBUG_ADJ_PACKETS;
  print_debug (cmsh, DEBUG_ADJ_PACKETS, 0);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisDebugCsum,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "debug isis checksum-errors",
       "Debugging functions (see also 'undebug')",
       "IS-IS information",
       "IS-IS LSP checksum errors")
{
  isis->debugs |= DEBUG_CHECKSUM_ERRORS;
  print_debug (cmsh, DEBUG_CHECKSUM_ERRORS, 1);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoDebugCsum,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "no debug isis checksum-errors",
       "Negate a command or set its defaults",
       "Disable debugging functions (see also 'debug')",
       "IS-IS information",
       "IS-IS LSP checksum errors")
{
  isis->debugs &= ~DEBUG_CHECKSUM_ERRORS;
  print_debug (cmsh, DEBUG_CHECKSUM_ERRORS, 0);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisDebugLupd,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "debug isis local-updates",
       "Debugging functions (see also 'undebug')",
       "IS-IS information",
       "IS-IS local update packets")
{
  isis->debugs |= DEBUG_LOCAL_UPDATES;
  print_debug (cmsh, DEBUG_LOCAL_UPDATES, 1);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoDebugLupd,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "no debug isis local-updates",
       "Negate a command or set its defaults",
       "Disable debugging functions (see also 'debug')",
       "IS-IS information",
       "IS-IS local update packets")
{
  isis->debugs &= ~DEBUG_LOCAL_UPDATES;
  print_debug (cmsh, DEBUG_LOCAL_UPDATES, 0);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisDebugErr,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "debug isis protocol-errors",
       "Debugging functions (see also 'undebug')",
       "IS-IS information",
       "IS-IS LSP protocol errors")
{
  isis->debugs |= DEBUG_PROTOCOL_ERRORS;
  print_debug (cmsh, DEBUG_PROTOCOL_ERRORS, 1);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoDebugErr,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "no debug isis protocol-errors",
       "Negate a command or set its defaults",
       "Disable debugging functions (see also 'debug')",
       "IS-IS information",
       "IS-IS LSP protocol errors")
{
  isis->debugs &= ~DEBUG_PROTOCOL_ERRORS;
  print_debug (cmsh, DEBUG_PROTOCOL_ERRORS, 0);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisDebugSnp,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "debug isis snp-packets",
       "Debugging functions (see also 'undebug')",
       "IS-IS information",
       "IS-IS CSNP/PSNP packets")
{
  isis->debugs |= DEBUG_SNP_PACKETS;
  print_debug (cmsh, DEBUG_SNP_PACKETS, 1);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoDebugSnp,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "no debug isis snp-packets",
       "Negate a command or set its defaults",
       "Disable debugging functions (see also 'debug')",
       "IS-IS information",
       "IS-IS CSNP/PSNP packets")
{
  isis->debugs &= ~DEBUG_SNP_PACKETS;
  print_debug (cmsh, DEBUG_SNP_PACKETS, 0);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisDebugUpd,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "debug isis update-packets",
       "Debugging functions (see also 'undebug')",
       "IS-IS information",
       "IS-IS Update related packets")
{
  isis->debugs |= DEBUG_UPDATE_PACKETS;
  print_debug (cmsh, DEBUG_UPDATE_PACKETS, 1);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoDebugUpd,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "no debug isis update-packets",
       "Negate a command or set its defaults",
       "Disable debugging functions (see also 'debug')",
       "IS-IS information",
       "IS-IS Update related packets")
{
  isis->debugs &= ~DEBUG_UPDATE_PACKETS;
  print_debug (cmsh, DEBUG_UPDATE_PACKETS, 0);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisDebugSpfevents,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "debug isis spf-events",
       "Debugging functions (see also 'undebug')",
       "IS-IS information",
       "IS-IS Shortest Path First Events")
{
  isis->debugs |= DEBUG_SPF_EVENTS;
  print_debug (cmsh, DEBUG_SPF_EVENTS, 1);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoDebugSpfevents,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "no debug isis spf-events",
       "Negate a command or set its defaults",
       "Disable debugging functions (see also 'debug')",
       "IS-IS information",
       "IS-IS Shortest Path First Events")
{
  isis->debugs &= ~DEBUG_SPF_EVENTS;
  print_debug (cmsh, DEBUG_SPF_EVENTS, 0);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisDebugSpfstats,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "debug isis spf-statistics ",
       "Debugging functions (see also 'undebug')",
       "IS-IS information",
       "IS-IS SPF Timing and Statistic Data")
{
  isis->debugs |= DEBUG_SPF_STATS;
  print_debug (cmsh, DEBUG_SPF_STATS, 1);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoDebugSpfstats,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "no debug isis spf-statistics",
       "Negate a command or set its defaults",
       "Disable debugging functions (see also 'debug')",
       "IS-IS information",
       "IS-IS SPF Timing and Statistic Data")
{
  isis->debugs &= ~DEBUG_SPF_STATS;
  print_debug (cmsh, DEBUG_SPF_STATS, 0);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisDebugSpftrigg,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "debug isis spf-triggers",
       "Debugging functions (see also 'undebug')",
       "IS-IS information",
       "IS-IS SPF triggering events")
{
  isis->debugs |= DEBUG_SPF_TRIGGERS;
  print_debug (cmsh, DEBUG_SPF_TRIGGERS, 1);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoDebugSpftrigg,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "no debug isis spf-triggers",
       "Negate a command or set its defaults",
       "Disable debugging functions (see also 'debug')",
       "IS-IS information",
       "IS-IS SPF triggering events")
{
  isis->debugs &= ~DEBUG_SPF_TRIGGERS;
  print_debug (cmsh, DEBUG_SPF_TRIGGERS, 0);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisDebugRtevents,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "debug isis route-events",
       "Debugging functions (see also 'undebug')",
       "IS-IS information",
       "IS-IS Route related events")
{
  isis->debugs |= DEBUG_RTE_EVENTS;
  print_debug (cmsh, DEBUG_RTE_EVENTS, 1);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoDebugRtevents,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "no debug isis route-events",
       "Negate a command or set its defaults",
       "Disable debugging functions (see also 'debug')",
       "IS-IS information",
       "IS-IS Route related events")
{
  isis->debugs &= ~DEBUG_RTE_EVENTS;
  print_debug (cmsh, DEBUG_RTE_EVENTS, 0);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisDebugEvents,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "debug isis events",
       "Debugging functions (see also 'undebug')",
       "IS-IS information",
       "IS-IS Events")
{
  isis->debugs |= DEBUG_EVENTS;
  print_debug (cmsh, DEBUG_EVENTS, 1);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoDebugEvents,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "no debug isis events",
       "Negate a command or set its defaults",
       "Disable debugging functions (see also 'debug')",
       "IS-IS information",
       "IS-IS Events")
{
  isis->debugs &= ~DEBUG_EVENTS;
  print_debug (cmsh, DEBUG_EVENTS, 0);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisDebugPacketDump,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "debug isis packet-dump",
       "Debugging functions (see also 'undebug')",
       "IS-IS information",
       "IS-IS packet dump")
{
  isis->debugs |= DEBUG_PACKET_DUMP;
  print_debug (cmsh, DEBUG_PACKET_DUMP, 1);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoDebugPacketDump,
		CMD_NODE_CONFIG,
	   IPC_ISIS,
       "no debug isis packet-dump",
       "Negate a command or set its defaults",
       "Disable debugging functions (see also 'debug')",
       "IS-IS information",
       "IS-IS packet dump")
{
  isis->debugs &= ~DEBUG_PACKET_DUMP;
  print_debug (cmsh, DEBUG_PACKET_DUMP, 0);

  return (CMD_IPC_OK);
}

