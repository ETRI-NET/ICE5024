/*
 * IS-IS Rout(e)ing protocol - isisd.c
 *
 * Copyright (C) 2001,2002   Sampo Saaristo
 *                           Tampere University of Technology      
 *                           Institute of Communications Engineering
 *
 * This program is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU General Public Licenseas published by the Free 
 * Software Foundation; either version 2 of the License, or (at your option) 
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
 * more details.

 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "vty.h"
#include "command.h"
#include "log.h"
#include "memory.h"
#include "time.h"
#include "linklist.h"
#include "if.h"
#include "hash.h"
#include "stream.h"
#include "prefix.h"
#include "table.h"
#include "filter.h"

#include "nnTypes.h"
#include "nnDefines.h"
#include "nnCmdCommon.h"
#include "nnCmdDefines.h"
#include "nnCompProcess.h"
#include "nnCmdCmsh.h"
#include "nosLib.h"
#include "nnFilter.h"
#include "nnBuffer.h"

#include "dict.h"
#include "include-netbsd/iso.h"
#include "isis_constants.h"
#include "isis_common.h"
#include "isis_flags.h"
#include "isis_circuit.h"
#include "isis_csm.h"
#include "isisd.h"
#include "isis_dynhn.h"
#include "isis_adjacency.h"
#include "isis_pdu.h"
#include "isis_misc.h"
#include "isis_constants.h"
#include "isis_tlv.h"
#include "isis_lsp.h"
#include "isis_spf.h"
#include "isis_route.h"
#include "isis_zebra.h"
#include "isis_events.h"
#include "isisRibmgr.h"
#include "isis_dynhn.h"
#include "isis_dr.h"


#ifdef TOPOLOGY_GENERATE
#include "spgrid.h"
u_char DEFAULT_TOPOLOGY_BASEIS[6] = { 0xFE, 0xED, 0xFE, 0xED, 0x00, 0x00 };
#endif /* TOPOLOGY_GENERATE */

struct isis *isis = NULL;

void
isis_new (unsigned long process_id)
{
  isis = XCALLOC (MTYPE_ISIS, sizeof (struct isis));
  /*
   * Default values
   */
  isis->max_area_addrs = 3;
  isis->process_id = process_id;
  isis->router_id = 0;
  isis->area_list = list_new ();
  isis->init_circ_list = list_new ();
  isis->uptime = time (NULL);
  isis->nexthops = list_new ();
  isis->isisRedistributeDefault = RIB_ROUTE_TYPE_ISIS;
  isis->isisRedistribute[RIB_ROUTE_TYPE_ISIS] = 1;
  isis->isisDefaultInformation = 0;
#ifdef HAVE_IPV6
  isis->nexthops6 = list_new ();
#endif /* HAVE_IPV6 */
  dyn_cache_init ();
  /*
   * uncomment the next line for full debugs
   */
  /* isis->debugs = 0xFFFF; */
}

struct isis_area *
isis_area_create (const char *area_tag)
{
  struct isis_area *area;
  struct timeval tv = {0,0};

  area = XCALLOC (MTYPE_ISIS_AREA, sizeof (struct isis_area));

  /*
   * The first instance is level-1-2 rest are level-1, unless otherwise
   * configured
   */
  if (listcount (isis->area_list) > 0)
    area->is_type = IS_LEVEL_1;
  else
    area->is_type = IS_LEVEL_1_AND_2;

  /*
   * intialize the databases
   */
  if (area->is_type & IS_LEVEL_1)
    {
      area->lspdb[0] = lsp_db_init ();
      area->route_table[0] = route_table_init ();
#ifdef HAVE_IPV6
      area->route_table6[0] = route_table_init ();
#endif /* HAVE_IPV6 */
    }
  if (area->is_type & IS_LEVEL_2)
    {
      area->lspdb[1] = lsp_db_init ();
      area->route_table[1] = route_table_init ();
#ifdef HAVE_IPV6
      area->route_table6[1] = route_table_init ();
#endif /* HAVE_IPV6 */
    }

  spftree_area_init (area);

  area->circuit_list = list_new ();
  area->area_addrs = list_new ();

  tv.tv_sec = 1;
  area->pTimerTick = taskTimerSet(lspTick, tv, 0, (void *)area);

  flags_initialize (&area->flags);

  /*
   * Default values
   */
  area->max_lsp_lifetime[0] = DEFAULT_LSP_LIFETIME;	/* 1200 */
  area->max_lsp_lifetime[1] = DEFAULT_LSP_LIFETIME;	/* 1200 */
  area->lsp_refresh[0] = DEFAULT_MAX_LSP_GEN_INTERVAL;	/* 900 */
  area->lsp_refresh[1] = DEFAULT_MAX_LSP_GEN_INTERVAL;	/* 900 */
  area->lsp_gen_interval[0] = DEFAULT_MIN_LSP_GEN_INTERVAL;
  area->lsp_gen_interval[1] = DEFAULT_MIN_LSP_GEN_INTERVAL;
  area->min_spf_interval[0] = MINIMUM_SPF_INTERVAL;
  area->min_spf_interval[1] = MINIMUM_SPF_INTERVAL;
  area->dynhostname = 1;
  area->oldmetric = 0;
  area->newmetric = 1;
  area->lsp_frag_threshold = 90;
#ifdef TOPOLOGY_GENERATE
  memcpy (area->topology_baseis, DEFAULT_TOPOLOGY_BASEIS, ISIS_SYS_ID_LEN);
#endif /* TOPOLOGY_GENERATE */

  /* FIXME: Think of a better way... */
  area->min_bcast_mtu = 1497;

  area->area_tag = strdup (area_tag);
  listnode_add (isis->area_list, area);
  area->isis = isis;

  return area;
}

struct isis_area *
isis_area_lookup (const char *area_tag)
{
  struct isis_area *area;
  struct listnode *node;

  for (ALL_LIST_ELEMENTS_RO (isis->area_list, node, area))
    if ((area->area_tag == NULL && area_tag == NULL) ||
	(area->area_tag && area_tag
	 && strcmp (area->area_tag, area_tag) == 0))
    return area;

  return NULL;
}

struct isis_area *
isisAreaGet (const char *area_tag)
{
	struct isis_area *area = NULL;

	area = isis_area_lookup(area_tag);

	if (area) {
		return area;
	}

	area = isis_area_create(area_tag);

	if (isis->debugs & DEBUG_EVENTS)
		zlog_debug("New IS-IS area instance %s", area->area_tag);

	return area;
}

Int32T
isisAreaDestroy (struct cmsh *cmsh, const char *area_tag)
{
	struct isis_area *area;
	struct listnode *node, *nnode;
	struct isis_circuit *circuit;
	struct area_addr *addr;

	area = isis_area_lookup(area_tag);

	if (area == NULL) {
		cmdPrint(cmsh, "Can't find ISIS instance \n");
		return CMD_IPC_ERROR;
	}

	if (area->circuit_list) {
		for (ALL_LIST_ELEMENTS(area->circuit_list, node, nnode, circuit)) {
			circuit->ip_router = 0;
#ifdef HAVE_IPV6
			circuit->ipv6_router = 0;
#endif
			isis_csm_state_change(ISIS_DISABLE, circuit, area);
		}
		list_delete(area->circuit_list);
		area->circuit_list = NULL;
	}

	if (area->lspdb[0] != NULL) {
		lsp_db_destroy(area->lspdb[0]);
		area->lspdb[0] = NULL;
	}
	if (area->lspdb[1] != NULL) {
		lsp_db_destroy(area->lspdb[1]);
		area->lspdb[1] = NULL;
	}

	spftree_area_del(area);

	/* invalidate and validate would delete all routes from zebra */
	isis_route_invalidate(area);
	isis_route_validate(area);

	if (area->route_table[0]) {
		route_table_finish(area->route_table[0]);
		area->route_table[0] = NULL;
	}
	if (area->route_table[1]) {
		route_table_finish(area->route_table[1]);
		area->route_table[1] = NULL;
	}
#ifdef HAVE_IPV6
	if (area->route_table6[0]) {
		route_table_finish(area->route_table6[0]);
		area->route_table6[0] = NULL;
	}
	if (area->route_table6[1]) {
		route_table_finish(area->route_table6[1]);
		area->route_table6[1] = NULL;
	}
#endif /* HAVE_IPV6 */

	for (ALL_LIST_ELEMENTS(area->area_addrs, node, nnode, addr)) {
		list_delete_node(area->area_addrs, node);
		XFREE(MTYPE_ISIS_AREA_ADDR, addr);
	}
	area->area_addrs = NULL;

	if(area->pTimerTick)
	{
		taskTimerDel(area->pTimerTick);
		area->pTimerTick = NULL;
	}
	if(area->pTimerLspRefresh[0])
	{
		taskTimerDel(area->pTimerLspRefresh[0]);
		area->pTimerLspRefresh[0] = NULL;
	}
	if(area->pTimerLspRefresh[1])
	{
		taskTimerDel(area->pTimerLspRefresh[1]);
		area->pTimerLspRefresh[1] = NULL;
	}

	listnode_delete(isis->area_list, area);

	free(area->area_tag);

	XFREE(MTYPE_ISIS_AREA, area);

	if (listcount (isis->area_list) == 0) {
		memset(isis->sysid, 0, ISIS_SYS_ID_LEN);
		isis->sysid_set = 0;
	}

	return CMD_IPC_OK;
}

int
areaNetTitle (struct cmsh *cmsh, const char *net_title, const char *area_tag)
{
	struct isis_area *area = NULL;
	struct area_addr *addr;
	struct area_addr *addrp;
	struct listnode *node;

	u_char buff[255];

	area = isisAreaGet(area_tag);
	if (!area) {
		cmdPrint(cmsh, "Can't find ISIS instance \n");
		return CMD_IPC_ERROR;
	}

	/* We check that we are not over the maximal number of addresses */
	if (listcount (area->area_addrs) >= isis->max_area_addrs) {
		cmdPrint(cmsh, "Maximum of area addresses (%d) already reached \n",
				isis->max_area_addrs);
		return CMD_IPC_ERROR;
	}

	addr = XMALLOC(MTYPE_ISIS_AREA_ADDR, sizeof(struct area_addr));
	addr->addr_len = dotformat2buff(buff, net_title);
	memcpy(addr->area_addr, buff, addr->addr_len);
#ifdef EXTREME_DEBUG
	zlog_debug ("added area address %s for area %s (address length %d)",
			net_title, area->area_tag, addr->addr_len);
#endif /* EXTREME_DEBUG */
	if (addr->addr_len < 8 || addr->addr_len > 20) {
		cmdPrint(cmsh, "area address must be at least 8..20 octets long (%d)\n",
				addr->addr_len);
		XFREE(MTYPE_ISIS_AREA_ADDR, addr);
		return CMD_IPC_ERROR;
	}

	if (addr->area_addr[addr->addr_len - 1] != 0) {
		cmdPrint(cmsh, "nsel byte (last byte) in area address must be 0\n");
		XFREE(MTYPE_ISIS_AREA_ADDR, addr);
		return CMD_IPC_ERROR;
	}

	if (isis->sysid_set == 0) {
		/*
		 * First area address - get the SystemID for this router
		 */
		memcpy(isis->sysid, GETSYSID(addr), ISIS_SYS_ID_LEN);
		isis->sysid_set = 1;
		if (isis->debugs & DEBUG_EVENTS)
			zlog_debug("Router has SystemID %s", sysid_print(isis->sysid));
	} else {
		/*
		 * Check that the SystemID portions match
		 */
		if (memcmp(isis->sysid, GETSYSID(addr), ISIS_SYS_ID_LEN)) {
			cmdPrint(cmsh,
					"System ID must not change when defining additional area"
							" addresses\n");
			XFREE(MTYPE_ISIS_AREA_ADDR, addr);
			return CMD_IPC_ERROR;
		}

		/* now we see that we don't already have this address */
		for (ALL_LIST_ELEMENTS_RO(area->area_addrs, node, addrp)) {
			if ((addrp->addr_len + ISIS_SYS_ID_LEN + ISIS_NSEL_LEN)
					!= (addr->addr_len))
				continue;
			if (!memcmp(addrp->area_addr, addr->area_addr, addr->addr_len)) {
				XFREE(MTYPE_ISIS_AREA_ADDR, addr);
				return CMD_IPC_OK; /* silent fail */
			}
		}
	}

	/*
	 * Forget the systemID part of the address
	 */
	addr->addr_len -= (ISIS_SYS_ID_LEN + ISIS_NSEL_LEN);
	listnode_add(area->area_addrs, addr);

	/* only now we can safely generate our LSPs for this area */
	if (listcount (area->area_addrs) > 0) {
		if (area->is_type & IS_LEVEL_1)
			lsp_generate(area, IS_LEVEL_1);
		if (area->is_type & IS_LEVEL_2)
			lsp_generate(area, IS_LEVEL_2);
	}

	return CMD_IPC_OK;
}

Int32T
areaClearNetTitle (struct cmsh *cmsh, const char *net_title, const char *area_tag)
{
	struct isis_area *area;
	struct area_addr addr, *addrp = NULL;
	struct listnode *node;
	u_char buff[255];

	area = isisAreaGet(area_tag);
	if (!area) {
		cmdPrint(cmsh, "Can't find ISIS instance \n");
		return CMD_IPC_ERROR;
	}

	addr.addr_len = dotformat2buff(buff, net_title);
	if (addr.addr_len < 8 || addr.addr_len > 20) {
		cmdPrint(cmsh, "Unsupported area address length %d, should be 8...20 \n",
				addr.addr_len);
		return CMD_IPC_ERROR;
	}

	memcpy(addr.area_addr, buff, (int) addr.addr_len);

	for (ALL_LIST_ELEMENTS_RO(area->area_addrs, node, addrp))
		if ((addrp->addr_len + ISIS_SYS_ID_LEN + 1) == addr.addr_len
				&& !memcmp(addrp->area_addr, addr.area_addr, addr.addr_len))
			break;

	if (!addrp) {
		cmdPrint(cmsh, "No area address %s for area %s \n", net_title,
				area->area_tag);
		return CMD_IPC_ERROR;
	}

	listnode_delete(area->area_addrs, addrp);
	XFREE(MTYPE_ISIS_AREA_ADDR, addrp);

	/*
	 * Last area address - reset the SystemID for this router
	 */
	if (listcount (area->area_addrs) == 0) {
		memset(isis->sysid, 0, ISIS_SYS_ID_LEN);
		isis->sysid_set = 0;
		if (isis->debugs & DEBUG_EVENTS)
			zlog_debug("Router has no SystemID");
	}

	return CMD_IPC_OK;
}

/*
 * 'show isis interface' command
 */

Int32T
showIsisInterfaceCommon (struct cmsh *cmsh, const char *ifname, char detail)
{
  struct listnode *anode, *cnode;
  struct isis_area *area;
  struct isis_circuit *circuit;

  if (!isis)
    {
      cmdPrint (cmsh, "IS-IS Routing Process not enabled\n");
      return (CMD_IPC_OK);
    }

  for (ALL_LIST_ELEMENTS_RO (isis->area_list, anode, area))
    {
      cmdPrint (cmsh, "Area %s:\n", area->area_tag);

      if (detail == ISIS_UI_LEVEL_BRIEF)
        cmdPrint (cmsh, "  Interface   CircId   State    Type     Level\n");

      for (ALL_LIST_ELEMENTS_RO (area->circuit_list, cnode, circuit))
        if (!ifname)
          isisCircuitPrintCmsh (circuit, cmsh, detail);
        else if (strcmp(circuit->interface->name, ifname) == 0)
        	isisCircuitPrintCmsh (circuit, cmsh, detail);
    }

  return (CMD_IPC_OK);
}

/*
 * 'show isis neighbor' command
 */

Int32T
showIsisNeighborCommon (struct cmsh *cmsh, const char *id, char detail)
{
	struct listnode *anode, *cnode, *node;
	struct isis_area *area;
	struct isis_circuit *circuit;
	struct list *adjdb;
	struct isis_adjacency *adj;
	struct isis_dynhn *dynhn;
	u_char sysid[ISIS_SYS_ID_LEN];
	int i;

	if (!isis) {
		cmdPrint(cmsh, "IS-IS Routing Process not enabled\n");
		return (CMD_IPC_OK);
	}

	memset(sysid, 0, ISIS_SYS_ID_LEN);
	if (id) {
		if (sysid2buff(sysid, id) == 0) {
			dynhn = dynhn_find_by_name(id);
			if (dynhn == NULL) {
				cmdPrint(cmsh, "Invalid system id %s", id);
				return (CMD_IPC_OK);
			}
			memcpy(sysid, dynhn->id, ISIS_SYS_ID_LEN);
		}
	}

	for (ALL_LIST_ELEMENTS_RO(isis->area_list, anode, area)) {
		cmdPrint(cmsh, "Area %s:", area->area_tag);

		if (detail == ISIS_UI_LEVEL_BRIEF)
			cmdPrint(cmsh, "  System Id           Interface   L  State"
					"        Holdtime SNPA");

		for (ALL_LIST_ELEMENTS_RO(area->circuit_list, cnode, circuit)) {
			if (circuit->circ_type == CIRCUIT_T_BROADCAST) {
				for (i = 0; i < 2; i++) {
					adjdb = circuit->u.bc.adjdb[i];
					if (adjdb && adjdb->count) {
						for (ALL_LIST_ELEMENTS_RO(adjdb, node, adj))
							if (!id || !memcmp(adj->sysid, sysid,
							ISIS_SYS_ID_LEN))
								isisAdjPrintCmsh(adj, cmsh, detail);
					}
				}
			} else if (circuit->circ_type == CIRCUIT_T_P2P
					&& circuit->u.p2p.neighbor) {
				adj = circuit->u.p2p.neighbor;
				if (!id || !memcmp(adj->sysid, sysid, ISIS_SYS_ID_LEN))
					isisAdjPrintCmsh(adj, cmsh, detail);
			}
		}
	}

	return (CMD_IPC_OK);
}

/*
 * 'clear isis neighbor' command
 */
Int32T
clearIsisNeighborCommon (struct cmsh *cmsh, const char *id)
{
	struct listnode *anode, *cnode, *cnextnode, *node, *nnode;
	struct isis_area *area;
	struct isis_circuit *circuit;
	struct list *adjdb;
	struct isis_adjacency *adj;
	struct isis_dynhn *dynhn;
	u_char sysid[ISIS_SYS_ID_LEN];
	int i;

	if (!isis) {
		cmdPrint(cmsh, "IS-IS Routing Process not enabled\n");
		return (CMD_IPC_OK);
	}

	memset(sysid, 0, ISIS_SYS_ID_LEN);
	if (id) {
		if (sysid2buff(sysid, id) == 0) {
			dynhn = dynhn_find_by_name(id);
			if (dynhn == NULL) {
				cmdPrint(cmsh, "Invalid system id %s\n", id);
				return (CMD_IPC_OK);
			}
			memcpy(sysid, dynhn->id, ISIS_SYS_ID_LEN);
		}
	}

	for (ALL_LIST_ELEMENTS_RO(isis->area_list, anode, area)) {
		for (ALL_LIST_ELEMENTS(area->circuit_list, cnode, cnextnode, circuit)) {
			if (circuit->circ_type == CIRCUIT_T_BROADCAST) {
				for (i = 0; i < 2; i++) {
					adjdb = circuit->u.bc.adjdb[i];
					if (adjdb && adjdb->count) {
						for (ALL_LIST_ELEMENTS(adjdb, node, nnode, adj))
							if (!id
									|| !memcmp(adj->sysid, sysid,
											ISIS_SYS_ID_LEN))
								isis_adj_state_change(adj, ISIS_ADJ_DOWN,
										"clear user request");
					}
				}
			} else if (circuit->circ_type == CIRCUIT_T_P2P
					&& circuit->u.p2p.neighbor) {
				adj = circuit->u.p2p.neighbor;
				if (!id || !memcmp(adj->sysid, sysid, ISIS_SYS_ID_LEN))
					isis_adj_state_change(adj, ISIS_ADJ_DOWN,
							"clear user request");
			}
		}
	}

	return (CMD_IPC_OK);
}

void
cmdPrintTimestr(Int8T *buff, Int32T *idx, time_t uptime)
{
	struct tm *tm;
	time_t diffTime = time(NULL);
	diffTime -= uptime;
	tm = gmtime(&diffTime);

#define ONE_DAY_SECOND 60*60*24
#define ONE_WEEK_SECOND 60*60*24*7
	if (diffTime < ONE_DAY_SECOND)
		*idx += sprintf(buff + *idx, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
	else if (diffTime < ONE_WEEK_SECOND)
		*idx += sprintf(buff + *idx, "%dd%02dh%02dm", tm->tm_yday, tm->tm_hour, tm->tm_min);
	else
		*idx += sprintf(buff + *idx, "%02dw%dd%02dh", tm->tm_yday / 7,
							tm->tm_yday - ((tm->tm_yday / 7) * 7), tm->tm_hour);
	*idx += sprintf(buff + *idx, " ago");
}

/*
 * This function supports following display options:
 * [ show isis database [detail] ]
 * [ show isis database <sysid> [detail] ]
 * [ show isis database <hostname> [detail] ]
 * [ show isis database <sysid>.<pseudo-id> [detail] ]
 * [ show isis database <hostname>.<pseudo-id> [detail] ]
 * [ show isis database <sysid>.<pseudo-id>-<fragment-number> [detail] ]
 * [ show isis database <hostname>.<pseudo-id>-<fragment-number> [detail] ]
 * [ show isis database detail <sysid> ]
 * [ show isis database detail <hostname> ]
 * [ show isis database detail <sysid>.<pseudo-id> ]
 * [ show isis database detail <hostname>.<pseudo-id> ]
 * [ show isis database detail <sysid>.<pseudo-id>-<fragment-number> ]
 * [ show isis database detail <hostname>.<pseudo-id>-<fragment-number> ]
 */
Int32T
showIsisDatabase (struct cmsh *cmsh, const char *argv, int ui_level)
{
	struct listnode *node;
	struct isis_area *area;
	struct isis_lsp *lsp;
	struct isis_dynhn *dynhn;
	const char *pos = argv;
	u_char lspid[ISIS_SYS_ID_LEN + 2];
	char sysid[255];
	u_char number[3];
	int level, lsp_count;

	if (isis->area_list->count == 0)
		return (CMD_IPC_OK);

	memset(&lspid, 0, ISIS_SYS_ID_LEN);
	memset(&sysid, 0, 255);

	/*
	 * extract fragment and pseudo id from the string argv
	 * in the forms:
	 * (a) <systemid/hostname>.<pseudo-id>-<framenent> or
	 * (b) <systemid/hostname>.<pseudo-id> or
	 * (c) <systemid/hostname> or
	 * Where systemid is in the form:
	 * xxxx.xxxx.xxxx
	 */
	if (argv)
		strncpy(sysid, argv, 254);
	if (argv && strlen(argv) > 3) {
		pos = argv + strlen(argv) - 3;
		if (strncmp(pos, "-", 1) == 0) {
			memcpy(number, ++pos, 2);
			lspid[ISIS_SYS_ID_LEN + 1] = (u_char) strtol((char *) number, NULL,
					16);
			pos -= 4;
			if (strncmp(pos, ".", 1) != 0)
				return CMD_ERR_AMBIGUOUS;
		}
		if (strncmp(pos, ".", 1) == 0) {
			memcpy(number, ++pos, 2);
			lspid[ISIS_SYS_ID_LEN] = (u_char) strtol((char *) number, NULL, 16);
			sysid[pos - argv - 1] = '\0';
		}
	}

	for (ALL_LIST_ELEMENTS_RO(isis->area_list, node, area)) {
		cmdPrint(cmsh, "Area %s:", area->area_tag ? area->area_tag : "null");

		for (level = 0; level < ISIS_LEVELS; level++) {
			if (area->lspdb[level] && dict_count (area->lspdb[level]) > 0) {
				lsp = NULL;
				if (argv != NULL) {
					/*
					 * Try to find the lsp-id if the argv string is in
					 * the form hostname.<pseudo-id>-<fragment>
					 */
					if (sysid2buff(lspid, sysid)) {
						lsp = lsp_search(lspid, area->lspdb[level]);
					} else if ((dynhn = dynhn_find_by_name(sysid))) {
						memcpy(lspid, dynhn->id, ISIS_SYS_ID_LEN);
						lsp = lsp_search(lspid, area->lspdb[level]);
					} else if (strncmp(unix_hostname(), sysid, 15) == 0) {
						memcpy(lspid, isis->sysid, ISIS_SYS_ID_LEN);
						lsp = lsp_search(lspid, area->lspdb[level]);
					}
				}

				if (lsp != NULL || argv == NULL) {
					cmdPrint(cmsh, "IS-IS Level-%d link-state database:", level + 1);

					/* print the title in all cases */
					cmdPrint(cmsh, "LSP ID                  PduLen  "
							"SeqNumber   Chksum  Holdtime  ATT/P/OL");
				}

				if (lsp) {
					if (ui_level == ISIS_UI_LEVEL_DETAIL)
						lspPrintDetail(lsp, cmsh, area->dynhostname);
					else
						lspPrint(lsp, cmsh, area->dynhostname);
				} else if (argv == NULL) {
					lsp_count = lspPrintAll(cmsh, area->lspdb[level], ui_level,
							area->dynhostname);

					cmdPrint(cmsh, "    %u LSPs\n", lsp_count);
				}
			}
		}
	}

	return (CMD_IPC_OK);
}

Int32T
setLspGenInterval (struct cmsh *cmsh, struct isis_area *area,
                      uint16_t interval, int level)
{
	int lvl;

	for (lvl = IS_LEVEL_1; lvl <= IS_LEVEL_2; ++lvl) {
		if (!(lvl & level))
			continue;

		if (interval >= area->lsp_refresh[lvl - 1]) {
			cmdPrint(cmsh, "LSP gen interval %us must be less than "
					"the LSP refresh interval %us\n", interval,
					area->lsp_refresh[lvl - 1]);
			return CMD_IPC_ERROR;
		}
	}

	for (lvl = IS_LEVEL_1; lvl <= IS_LEVEL_2; ++lvl) {
		if (!(lvl & level))
			continue;
		area->lsp_gen_interval[lvl - 1] = interval;
	}

	return CMD_IPC_OK;
}

Int32T
validateMetricStyleNarrow (struct cmsh *cmsh, struct isis_area *area)
{
	struct isis_circuit *circuit;
	struct listnode *node;

	if (!area) {
		cmdPrint(cmsh, "ISIS area is invalid\n");
		return CMD_IPC_ERROR;
	}

	for (ALL_LIST_ELEMENTS_RO(area->circuit_list, node, circuit)) {
		if ((area->is_type & IS_LEVEL_1) && (circuit->is_type & IS_LEVEL_1)
				&& (circuit->te_metric[0] > MAX_NARROW_LINK_METRIC)) {
			cmdPrint(cmsh, "ISIS circuit %s metric is invalid\n",
					circuit->interface->name);
			return CMD_IPC_ERROR;
		}
		if ((area->is_type & IS_LEVEL_2) && (circuit->is_type & IS_LEVEL_2)
				&& (circuit->te_metric[1] > MAX_NARROW_LINK_METRIC)) {
			cmdPrint(cmsh, "ISIS circuit %s metric is invalid\n",
					circuit->interface->name);
			return CMD_IPC_ERROR;
		}
	}

	return CMD_IPC_OK;
}

Int32T
setLspMaxLifetime (struct cmsh *cmsh, struct isis_area *area,
                      uint16_t interval, int level)
{
	int lvl;
	int set_refresh_interval[ISIS_LEVELS] = { 0, 0 };
	uint16_t refresh_interval;

	refresh_interval = interval - 300;

	for (lvl = IS_LEVEL_1; lvl <= IS_LEVEL_2; lvl++) {
		if (!(lvl & level))
			continue;
		if (refresh_interval < area->lsp_refresh[lvl - 1]) {
			cmdPrint(cmsh,
					"Level %d Max LSP lifetime %us must be 300s greater than "
							"the configured LSP refresh interval %us\n", lvl,
					interval, area->lsp_refresh[lvl - 1]);
			cmdPrint(cmsh, "Automatically reducing level %d LSP refresh interval "
					"to %us", lvl, refresh_interval);
			set_refresh_interval[lvl - 1] = 1;

			if (refresh_interval <= area->lsp_gen_interval[lvl - 1]) {
				cmdPrint(cmsh, "LSP refresh interval %us must be greater than "
						"the configured LSP gen interval %us",
						refresh_interval, area->lsp_gen_interval[lvl - 1]);
				return CMD_IPC_ERROR;
			}
		}
	}

	for (lvl = IS_LEVEL_1; lvl <= IS_LEVEL_2; lvl++) {
		if (!(lvl & level))
			continue;
		area->max_lsp_lifetime[lvl - 1] = interval;
		/* Automatically reducing lsp_refresh_interval to interval - 300 */
		if (set_refresh_interval[lvl - 1])
			area->lsp_refresh[lvl - 1] = refresh_interval;
	}

	lsp_regenerate_schedule(area, level, 1);

	return CMD_IPC_OK;
}

Int32T
setLspRefreshInterval (struct cmsh *cmsh, struct isis_area *area,
                          uint16_t interval, int level)
{
	int lvl;

	for (lvl = IS_LEVEL_1; lvl <= IS_LEVEL_2; ++lvl) {
		if (!(lvl & level))
			continue;
		if (interval <= area->lsp_gen_interval[lvl - 1]) {
			cmdPrint(cmsh, "LSP refresh interval %us must be greater than "
					"the configured LSP gen interval %us\n", interval,
					area->lsp_gen_interval[lvl - 1]);
			return CMD_IPC_ERROR;
		}
		if (interval > (area->max_lsp_lifetime[lvl - 1] - 300)) {
			cmdPrint(cmsh, "LSP refresh interval %us must be less than "
					"the configured LSP lifetime %us less 300\n", interval,
					area->max_lsp_lifetime[lvl - 1]);
			return CMD_IPC_ERROR;
		}
	}

	for (lvl = IS_LEVEL_1; lvl <= IS_LEVEL_2; ++lvl) {
		if (!(lvl & level))
			continue;
		area->lsp_refresh[lvl - 1] = interval;
	}
	lsp_regenerate_schedule(area, level, 1);

	return CMD_IPC_OK;
}

#ifdef TOPOLOGY_GENERATE

DEFUN (topology_generate_grid,
       topology_generate_grid_cmd,
       "topology generate grid <1-100> <1-100> <1-65000> [param] [param] "
       "[param]",
       "Topology generation for IS-IS\n"
       "Topology generation\n"
       "Grid topology\n"
       "X parameter of the grid\n"
       "Y parameter of the grid\n"
       "Random seed\n"
       "Optional param 1\n"
       "Optional param 2\n"
       "Optional param 3\n"
       "Topology\n")
{
  struct isis_area *area;

  area = vty->index;
  assert (area);

  if (!spgrid_check_params (vty, argc, argv))
    {
      if (area->topology)
	list_delete (area->topology);
      area->topology = list_new ();
      memcpy (area->top_params, vty->buf, 200);
      gen_spgrid_topology (vty, area->topology);
      remove_topology_lsps (area);
      generate_topology_lsps (area);
      /* Regenerate L1 LSP to get two way connection to the generated
       * topology. */
      lsp_regenerate_schedule (area, IS_LEVEL_1 | IS_LEVEL_2, 1);
    }

  return (CMD_IPC_OK);
}

DEFUN (show_isis_generated_topology,
       show_isis_generated_topology_cmd,
       "show isis generated-topologies",
       SHOW_STR
       "ISIS network information\n"
       "Show generated topologies\n")
{
  struct isis_area *area;
  struct listnode *node;
  struct listnode *node2;
  struct arc *arc;

  for (ALL_LIST_ELEMENTS_RO (isis->area_list, node, area))
    {
      if (!area->topology)
	continue;

      vty_out (vty, "Topology for isis area: %s%s", area->area_tag,
	       VTY_NEWLINE);
      vty_out (vty, "From node     To node     Distance%s", VTY_NEWLINE);

      for (ALL_LIST_ELEMENTS_RO (area->topology, node2, arc))
	vty_out (vty, "%9ld %11ld %12ld%s", arc->from_node, arc->to_node,
		 arc->distance, VTY_NEWLINE);
    }
  return (CMD_IPC_OK);
}

/* Base IS for topology generation. */
DEFUN (topology_baseis,
       topology_baseis_cmd,
       "topology base-is WORD",
       "Topology generation for IS-IS\n"
       "A Network IS Base for this topology\n"
       "XXXX.XXXX.XXXX Network entity title (NET)\n")
{
  struct isis_area *area;
  u_char buff[ISIS_SYS_ID_LEN];

  area = vty->index;
  assert (area);

  if (sysid2buff (buff, argv[0]))
    sysid2buff (area->topology_baseis, argv[0]);

  return (CMD_IPC_OK);
}

DEFUN (no_topology_baseis,
       no_topology_baseis_cmd,
       "no topology base-is WORD",
       NO_STR
       "Topology generation for IS-IS\n"
       "A Network IS Base for this topology\n"
       "XXXX.XXXX.XXXX Network entity title (NET)\n")
{
  struct isis_area *area;

  area = vty->index;
  assert (area);

  memcpy (area->topology_baseis, DEFAULT_TOPOLOGY_BASEIS, ISIS_SYS_ID_LEN);
  return (CMD_IPC_OK);
}

ALIAS (no_topology_baseis,
       no_topology_baseis_noid_cmd,
       "no topology base-is",
       NO_STR
       "Topology generation for IS-IS\n"
       "A Network IS Base for this topology\n")

DEFUN (topology_basedynh,
       topology_basedynh_cmd,
       "topology base-dynh WORD",
       "Topology generation for IS-IS\n"
       "Dynamic hostname base for this topology\n"
       "Dynamic hostname base\n")
{
  struct isis_area *area;

  area = vty->index;
  assert (area);

  /* I hope that it's enough. */
  area->topology_basedynh = strndup (argv[0], 16);
  return (CMD_IPC_OK);
}

#endif /* TOPOLOGY_GENERATE */

/* IS-IS configuration write function */
Int32T
configWriteIsis (struct cmsh *cmsh)
{
	Int8T printBuff[1024] = {};
	Int32T idx = 0;

	if (isis != NULL) {
		struct isis_area *area;
		struct listnode *node, *node2;

		for (ALL_LIST_ELEMENTS_RO(isis->area_list, node, area)) {
			/* ISIS - Area name */
			cmdPrint(cmsh, "!");
			cmdPrint(cmsh, "router isis %s", area->area_tag);

			/* ISIS - Net */
			if (listcount (area->area_addrs) > 0) {
				struct area_addr *area_addr;
				for (ALL_LIST_ELEMENTS_RO(area->area_addrs, node2, area_addr)) {
					cmdPrint(cmsh, " net %s",
							isonet_print(area_addr->area_addr,
									area_addr->addr_len + ISIS_SYS_ID_LEN + 1));

				}
			}
			/* ISIS - Dynamic hostname - Defaults to true so only display if
			 * false. */
			if (!area->dynhostname) {
				cmdPrint(cmsh, " no hostname dynamic");

			}
			/* ISIS - Metric-Style - when true displays wide */
			if (area->newmetric) {
				if (!area->oldmetric)
				{
					cmdPrint(cmsh, " metric-style wide");
				}
				else
				{
					cmdPrint(cmsh, " metric-style transition");
				}

			} else {
				cmdPrint(cmsh, " metric-style narrow");

			}
			/* ISIS - overload-bit */
			if (area->overload_bit) {
				cmdPrint(cmsh, " set-overload-bit");

			}
			/* ISIS - Area is-type (level-1-2 is default) */
			if (area->is_type == IS_LEVEL_1) {
				cmdPrint(cmsh, " is-type level-1");

			} else if (area->is_type == IS_LEVEL_2) {
				cmdPrint(cmsh, " is-type level-2-only");

			}
			/* ISIS - Lsp generation interval */
			if (area->lsp_gen_interval[0] == area->lsp_gen_interval[1]) {
				if (area->lsp_gen_interval[0] != DEFAULT_MIN_LSP_GEN_INTERVAL) {
					cmdPrint(cmsh, " lsp-gen-interval %d",
							area->lsp_gen_interval[0]);

				}
			} else {
				if (area->lsp_gen_interval[0] != DEFAULT_MIN_LSP_GEN_INTERVAL) {
					cmdPrint(cmsh, " lsp-gen-interval level-1 %d",
							area->lsp_gen_interval[0]);

				}
				if (area->lsp_gen_interval[1] != DEFAULT_MIN_LSP_GEN_INTERVAL) {
					cmdPrint(cmsh, " lsp-gen-interval level-2 %d",
							area->lsp_gen_interval[1]);

				}
			}
			/* ISIS - LSP lifetime */
			if (area->max_lsp_lifetime[0] == area->max_lsp_lifetime[1]) {
				if (area->max_lsp_lifetime[0] != DEFAULT_LSP_LIFETIME) {
					cmdPrint(cmsh, " max-lsp-lifetime %u",
							area->max_lsp_lifetime[0]);

				}
			} else {
				if (area->max_lsp_lifetime[0] != DEFAULT_LSP_LIFETIME) {
					cmdPrint(cmsh, " max-lsp-lifetime level-1 %u",
							area->max_lsp_lifetime[0]);

				}
				if (area->max_lsp_lifetime[1] != DEFAULT_LSP_LIFETIME) {
					cmdPrint(cmsh, " max-lsp-lifetime level-2 %u",
							area->max_lsp_lifetime[1]);

				}
			}
			/* ISIS - LSP refresh interval */
			if (area->lsp_refresh[0] == area->lsp_refresh[1]) {
				if (area->lsp_refresh[0] != DEFAULT_MAX_LSP_GEN_INTERVAL) {
					cmdPrint(cmsh, " lsp-refresh-interval %u",
							area->lsp_refresh[0]);

				}
			} else {
				if (area->lsp_refresh[0] != DEFAULT_MAX_LSP_GEN_INTERVAL) {
					cmdPrint(cmsh, " lsp-refresh-interval level-1 %u",
							area->lsp_refresh[0]);

				}
				if (area->lsp_refresh[1] != DEFAULT_MAX_LSP_GEN_INTERVAL) {
					cmdPrint(cmsh, " lsp-refresh-interval level-2 %u",
							area->lsp_refresh[1]);

				}
			}
			/* Minimum SPF interval. */
			if (area->min_spf_interval[0] == area->min_spf_interval[1]) {
				if (area->min_spf_interval[0] != MINIMUM_SPF_INTERVAL) {
					cmdPrint(cmsh, " spf-interval %d",
							area->min_spf_interval[0]);

				}
			} else {
				if (area->min_spf_interval[0] != MINIMUM_SPF_INTERVAL) {
					cmdPrint(cmsh, " spf-interval level-1 %d",
							area->min_spf_interval[0]);

				}
				if (area->min_spf_interval[1] != MINIMUM_SPF_INTERVAL) {
					cmdPrint(cmsh, " spf-interval level-2 %d",
							area->min_spf_interval[1]);

				}
			}

			/* Authentication passwords. */
			if (area->area_passwd.type == ISIS_PASSWD_TYPE_HMAC_MD5) {
				idx += sprintf(printBuff + idx, " area-password md5 %s",
										area->area_passwd.passwd);
				if (CHECK_FLAG(area->area_passwd.snp_auth, SNP_AUTH_SEND)) {
					idx += sprintf(printBuff + idx, " authenticate snp ");
					if (CHECK_FLAG(area->area_passwd.snp_auth, SNP_AUTH_RECV))
						idx += sprintf(printBuff + idx, "validate");
					else
						idx += sprintf(printBuff + idx, "send-only");
				}
			} else if (area->area_passwd.type == ISIS_PASSWD_TYPE_CLEARTXT) {
				idx += sprintf(printBuff + idx, " area-password clear %s",
										area->area_passwd.passwd);
				if (CHECK_FLAG(area->area_passwd.snp_auth, SNP_AUTH_SEND)) {
					idx += sprintf(printBuff + idx, " authenticate snp ");
					if (CHECK_FLAG(area->area_passwd.snp_auth, SNP_AUTH_RECV))
						idx += sprintf(printBuff + idx, "validate");
					else
						idx += sprintf(printBuff + idx, "send-only");
				}
			}
			cmdPrint(cmsh, "%s",printBuff);
			idx = 0; memset(printBuff, 0, 1024);
			if (area->domain_passwd.type == ISIS_PASSWD_TYPE_HMAC_MD5) {
				idx += sprintf(printBuff + idx, " domain-password md5 %s",
									area->domain_passwd.passwd);
				if (CHECK_FLAG(area->domain_passwd.snp_auth, SNP_AUTH_SEND)) {
					idx += sprintf(printBuff + idx, " authenticate snp ");
					if (CHECK_FLAG(area->domain_passwd.snp_auth, SNP_AUTH_RECV))
						idx += sprintf(printBuff + idx, "validate");
					else
						idx += sprintf(printBuff + idx, "send-only");
				}
			} else if (area->domain_passwd.type == ISIS_PASSWD_TYPE_CLEARTXT) {
				idx += sprintf(printBuff + idx, " domain-password clear %s",
									area->domain_passwd.passwd);
				if (CHECK_FLAG(area->domain_passwd.snp_auth, SNP_AUTH_SEND)) {
					idx += sprintf(printBuff + idx, " authenticate snp ");
					if (CHECK_FLAG(area->domain_passwd.snp_auth, SNP_AUTH_RECV))
						idx += sprintf(printBuff + idx, "validate");
					else
						idx += sprintf(printBuff + idx, "send-only");
				}
			}
			cmdPrint(cmsh, "%s",printBuff);
			idx = 0; memset(printBuff, 0, 1024);

			if (area->log_adj_changes) {
				cmdPrint(cmsh, " log-adjacency-changes\n");

			}

#ifdef TOPOLOGY_GENERATE
			if (memcmp (area->topology_baseis, DEFAULT_TOPOLOGY_BASEIS,
							ISIS_SYS_ID_LEN))
			{
				cmdPrint (cmsh, " topology base-is %s\n",
						sysid_print ((u_char *)area->topology_baseis));

			}
			if (area->topology_basedynh)
			{
				cmdPrint (cmsh, " topology base-dynh %s\n",
						area->topology_basedynh);

			}
			/* We save the whole command line here. */
			if (strlen(area->top_params))
			{
				cmdPrint (cmsh, " %s\n", area->top_params);

			}
#endif /* TOPOLOGY_GENERATE */

		}
	}

	return SUCCESS;
}

struct cmd_node isis_node = {
  ISIS_NODE,
  "%s(config-router)# ",
  1
};

extern Int32T
isisWriteConfCB(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);

void
isisInit ()
{
	/* random seed from time */
	srand(time(NULL));

	/* create the global 'isis' instance */
	isis_new(1);

	/* Command Init. */
	isis->pCmshGlobal = compCmdInit (IPC_ISIS, isisWriteConfCB);

	accessListInit();
	isis_circuit_init();

	isisRibmgrInit();
}

static void
isisUpdateSocket()
{
	struct isis_area *area;
	struct listnode *node;
	struct isis_circuit *circuit;

	for (ALL_LIST_ELEMENTS_RO(isis->area_list, node, area)) {
		if (area->circuit_list) {
			for (ALL_LIST_ELEMENTS_RO(area->circuit_list, node, circuit)) {
				if (circuit->pSockFdEvent)
					taskFdUpdate(isisReceive, circuit->pSockFdEvent,
							(void *) circuit);
			}
		}
	}

	if (isis->init_circ_list) {
		for (ALL_LIST_ELEMENTS_RO(isis->init_circ_list, node, circuit))
			if (circuit->pSockFdEvent)
				taskFdUpdate(isisReceive, circuit->pSockFdEvent,
						(void *) circuit);
	}
}

static void
circuitTimerVersionUpdate (struct isis_circuit *circuit)
{
	struct timeval tv = { 0, 0 };

	if (circuit->u.bc.pTimerRunDr[0]) {
		tv.tv_sec = 2 * circuit->hello_interval[0];
		taskTimerUpdate(isisRunDrL1, circuit->u.bc.pTimerRunDr[0], tv,
				(void *) circuit);
	}
	if (circuit->u.bc.pTimerRunDr[1]) {
		tv.tv_sec = 2 * circuit->hello_interval[1];
		taskTimerUpdate(isisRunDrL2, circuit->u.bc.pTimerRunDr[0], tv,
				(void *) circuit);
	}

	if (circuit->u.bc.pTimerSendLanHello[0]) {
		tv.tv_sec = circuit->hello_interval[0];
		taskTimerUpdate(sendLanL1Hello, circuit->u.bc.pTimerSendLanHello[0], tv,
				(void *) circuit);
	}
	if (circuit->u.bc.pTimerSendLanHello[1]) {
		tv.tv_sec = circuit->hello_interval[1];
		taskTimerUpdate(sendLanL2Hello, circuit->u.bc.pTimerSendLanHello[1], tv,
				(void *) circuit);
	}

	if (circuit->u.bc.pTimerRefreshPseudoLsp[0]) {
		tv.tv_sec = 30;
		taskTimerUpdate(lspL1RefreshPseudo,
				circuit->u.bc.pTimerRefreshPseudoLsp[0], tv, (void *) circuit);
	}
	if (circuit->u.bc.pTimerRefreshPseudoLsp[1]) {
		tv.tv_sec = 30;
		taskTimerUpdate(lspL2RefreshPseudo,
				circuit->u.bc.pTimerRefreshPseudoLsp[1], tv, (void *) circuit);
	}

	if (circuit->u.p2p.pTimerSendP2pHello) {
		tv.tv_sec = 0;
		taskTimerUpdate(sendP2pHello, circuit->u.p2p.pTimerSendP2pHello, tv,
				(void *) circuit);
	}

	if (circuit->pTimerSendPsnp[0]) {
		tv.tv_sec = circuit->psnp_interval[0];
		taskTimerUpdate(sendL1Psnp, circuit->pTimerSendPsnp[0], tv,
				(void *) circuit);
	}
	if (circuit->pTimerSendPsnp[1]) {
		tv.tv_sec = circuit->psnp_interval[1];
		taskTimerUpdate(sendL2Psnp, circuit->pTimerSendPsnp[0], tv,
				(void *) circuit);
	}

	if (circuit->pTimerSendCsnp[0]) {
		tv.tv_sec = circuit->csnp_interval[0];
		taskTimerUpdate(sendL1Csnp, circuit->pTimerSendCsnp[0], tv,
				(void *) circuit);
	}
	if (circuit->pTimerSendCsnp[1]) {
		tv.tv_sec = circuit->csnp_interval[1];
		taskTimerUpdate(sendL2Csnp, circuit->pTimerSendCsnp[0], tv,
				(void *) circuit);
	}
}

static void
areaTimerVersionUpdate (struct isis_area *area)
{
	struct listnode *node;
	struct isis_circuit *circuit;

	struct isis_spftree *spftree;
	struct timeval tv = { 0, 0 };

	if (area->pTimerLspRefresh[0]) {
		tv.tv_sec = 30;
		taskTimerUpdate(lspL1Refresh, area->pTimerLspRefresh[0], tv,
				(void *) area);
	}
	if (area->pTimerLspRefresh[1]) {
		tv.tv_sec = 30;
		taskTimerUpdate(lspL2Refresh, area->pTimerLspRefresh[1], tv,
				(void *) area);
	}

	if (area->pTimerTick) {
		tv.tv_sec = 1;
		taskTimerUpdate(lspTick, area->pTimerTick, tv, (void *) area);
	}

	spftree = area->spftree[0];
	if(spftree)
		if (spftree->pTimerSpf) {
			tv.tv_sec = area->min_spf_interval[0];
			taskTimerUpdate(isisRunSpfL1, spftree->pTimerSpf, tv, (void *) area);
		}

	spftree = area->spftree[1];
	if (spftree)
		if (spftree->pTimerSpf) {
			tv.tv_sec = area->min_spf_interval[1];
			taskTimerUpdate(isisRunSpfL2, spftree->pTimerSpf, tv, (void *) area);
		}

	if (area->circuit_list) {
		for (ALL_LIST_ELEMENTS_RO(area->circuit_list, node, circuit))
			circuitTimerVersionUpdate(circuit);
	}
}

static void
isisTimerVersionUpdate (void)
{
	struct isis_area *area;
	struct listnode *node;
	struct isis_circuit *circuit;
	struct timeval tv = { 0, 0 };

	if (isis->pTimerDyncClean) {
		tv.tv_sec = 60;
		taskTimerUpdate(dynCacheCleanup, isis->pTimerDyncClean, tv, NULL);
	}

	if (isis->area_list) {
		for (ALL_LIST_ELEMENTS_RO(isis->area_list, node, area))
			areaTimerVersionUpdate(area);
	}

	if (isis->init_circ_list) {
		for (ALL_LIST_ELEMENTS_RO (isis->init_circ_list, node, circuit))
			circuitTimerVersionUpdate(circuit);
	}

}

void
isisVersionUpdate()
{
	assert(isis);

	/* Update Command pointer. */
	compCmdUpdate(isis->pCmshGlobal, isisWriteConfCB);

	/* Update socket. */
	if (isis->area_list)
		isisUpdateSocket();

	/* Update timers. */
	isisTimerVersionUpdate();
}





