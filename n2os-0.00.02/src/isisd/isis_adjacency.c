/*
 * IS-IS Rout(e)ing protocol - isis_adjacency.c   
 *                             handling of IS-IS adjacencies
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

//#include <zebra.h>

#include "log.h"
#include "memory.h"
#include "hash.h"
#include "vty.h"
#include "linklist.h"
#include "if.h"
#include "stream.h"

#include "nnTypes.h"
#include "nnCmdCmsh.h"
#include "nosLib.h"

#include "dict.h"
#include "include-netbsd/iso.h"
#include "isis_constants.h"
#include "isis_common.h"
#include "isis_flags.h"
#include "isisd.h"
#include "isis_circuit.h"
#include "isis_adjacency.h"
#include "isis_misc.h"
#include "isis_dr.h"
#include "isis_dynhn.h"
#include "isis_pdu.h"
#include "isis_tlv.h"
#include "isis_lsp.h"
#include "isis_spf.h"
#include "isis_events.h"

extern struct isis *isis;

static struct isis_adjacency *
adj_alloc (u_char * id)
{
  struct isis_adjacency *adj;

  adj = XCALLOC (MTYPE_ISIS_ADJACENCY, sizeof (struct isis_adjacency));
  memcpy (adj->sysid, id, ISIS_SYS_ID_LEN);

  return adj;
}

struct isis_adjacency *
isis_new_adj (u_char * id, u_char * snpa, int level,
	      struct isis_circuit *circuit)
{
  struct isis_adjacency *adj;
  int i;

  adj = adj_alloc (id);		/* P2P kludge */

  if (adj == NULL)
    {
      zlog_err ("Out of memory!");
      return NULL;
    }

  if (snpa) {
    memcpy (adj->snpa, snpa, ETH_ALEN);
  } else {
    memset (adj->snpa, ' ', ETH_ALEN);
  }

  adj->circuit = circuit;
  adj->level = level;
  adj->flaps = 0;
  adj->last_flap = time (NULL);
  if (circuit->circ_type == CIRCUIT_T_BROADCAST)
    {
      listnode_add (circuit->u.bc.adjdb[level - 1], adj);
      adj->dischanges[level - 1] = 0;
      for (i = 0; i < DIS_RECORDS; i++)	/* clear N DIS state change records */
	{
	  adj->dis_record[(i * ISIS_LEVELS) + level - 1].dis
	    = ISIS_UNKNOWN_DIS;
	  adj->dis_record[(i * ISIS_LEVELS) + level - 1].last_dis_change
	    = time (NULL);
	}
    }

  return adj;
}

struct isis_adjacency *
isis_adj_lookup (u_char * sysid, struct list *adjdb)
{
  struct isis_adjacency *adj;
  struct listnode *node;

  for (ALL_LIST_ELEMENTS_RO (adjdb, node, adj))
    if (memcmp (adj->sysid, sysid, ISIS_SYS_ID_LEN) == 0)
      return adj;

  return NULL;
}

struct isis_adjacency *
isis_adj_lookup_snpa (u_char * ssnpa, struct list *adjdb)
{
  struct listnode *node;
  struct isis_adjacency *adj;

  for (ALL_LIST_ELEMENTS_RO (adjdb, node, adj))
    if (memcmp (adj->snpa, ssnpa, ETH_ALEN) == 0)
      return adj;

  return NULL;
}

void
isis_delete_adj (void *arg)
{
  struct isis_adjacency *adj = arg;

  if (!adj)
    return;

  if(adj->pTimerExpire)
  {
	  taskTimerDel(adj->pTimerExpire);
	  adj->pTimerExpire = NULL;
  }

  /* remove from SPF trees */
  spftree_area_adj_del (adj->circuit->area, adj);

  if (adj->area_addrs)
    list_delete (adj->area_addrs);
  if (adj->ipv4_addrs)
    list_delete (adj->ipv4_addrs);
#ifdef HAVE_IPV6
  if (adj->ipv6_addrs)
    list_delete (adj->ipv6_addrs);
#endif

  XFREE (MTYPE_ISIS_ADJACENCY, adj);
  return;
}

static const char *
adj_state2string (int state)
{

  switch (state)
    {
    case ISIS_ADJ_INITIALIZING:
      return "Initializing";
    case ISIS_ADJ_UP:
      return "Up";
    case ISIS_ADJ_DOWN:
      return "Down";
    default:
      return "Unknown";
    }

  return NULL;			/* not reached */
}

void
isis_adj_state_change (struct isis_adjacency *adj, enum isis_adj_state new_state,
		       const char *reason)
{
  int old_state;
  int level;
  struct isis_circuit *circuit;

  old_state = adj->adj_state;
  adj->adj_state = new_state;

  circuit = adj->circuit;

  if (isis->debugs & DEBUG_ADJ_PACKETS)
    {
      zlog_debug ("ISIS-Adj (%s): Adjacency state change %d->%d: %s",
		 circuit->area->area_tag,
		 old_state, new_state, reason ? reason : "unspecified");
    }

  if (circuit->area->log_adj_changes)
    {
      const char *adj_name;
      struct isis_dynhn *dyn;

      dyn = dynhn_find_by_id (adj->sysid);
      if (dyn)
	adj_name = (const char *)dyn->name.name;
      else
	adj_name = adj->sysid ? sysid_print (adj->sysid) : "unknown";

      zlog_info ("%%ADJCHANGE: Adjacency to %s (%s) changed from %s to %s, %s",
		 adj_name,
		 adj->circuit->interface->name,
		 adj_state2string (old_state),
		 adj_state2string (new_state),
		 reason ? reason : "unspecified");
    }

  if (circuit->circ_type == CIRCUIT_T_BROADCAST)
    {
      for (level = IS_LEVEL_1; level <= IS_LEVEL_2; level++)
      {
        if ((adj->level & level) == 0)
          continue;
        if (new_state == ISIS_ADJ_UP)
        {
          circuit->upadjcount[level - 1]++;
          isis_event_adjacency_state_change (adj, new_state);
          /* update counter & timers for debugging purposes */
          adj->last_flap = time (NULL);
          adj->flaps++;
        }
        else if (new_state == ISIS_ADJ_DOWN)
        {
          listnode_delete (circuit->u.bc.adjdb[level - 1], adj);
          circuit->upadjcount[level - 1]--;
          if (circuit->upadjcount[level - 1] == 0)
            {
              /* Clean lsp_queue when no adj is up. */
              if (circuit->lsp_queue)
                list_delete_all_node (circuit->lsp_queue);
            }
          isis_event_adjacency_state_change (adj, new_state);
          isis_delete_adj (adj);
        }

        if (circuit->u.bc.lan_neighs[level - 1])
          {
            list_delete_all_node (circuit->u.bc.lan_neighs[level - 1]);
            isis_adj_build_neigh_list (circuit->u.bc.adjdb[level - 1],
                                       circuit->u.bc.lan_neighs[level - 1]);
          }

        /* On adjacency state change send new pseudo LSP if we are the DR */
        if (circuit->u.bc.is_dr[level - 1])
          lsp_regenerate_schedule_pseudo (circuit, level);
      }
    }
  else if (circuit->circ_type == CIRCUIT_T_P2P)
    {
      for (level = IS_LEVEL_1; level <= IS_LEVEL_2; level++)
      {
        if ((adj->level & level) == 0)
          continue;
        if (new_state == ISIS_ADJ_UP)
        {
          circuit->upadjcount[level - 1]++;
          isis_event_adjacency_state_change (adj, new_state);

          if (adj->sys_type == ISIS_SYSTYPE_UNKNOWN)
            send_hello (circuit, level);

          /* update counter & timers for debugging purposes */
          adj->last_flap = time (NULL);
          adj->flaps++;

          /* 7.3.17 - going up on P2P -> send CSNP */
          /* FIXME: yup, I know its wrong... but i will do it! (for now) */
          send_csnp (circuit, level);
        }
        else if (new_state == ISIS_ADJ_DOWN)
        {
          if (adj->circuit->u.p2p.neighbor == adj)
            adj->circuit->u.p2p.neighbor = NULL;
          circuit->upadjcount[level - 1]--;
          if (circuit->upadjcount[level - 1] == 0)
            {
              /* Clean lsp_queue when no adj is up. */
              if (circuit->lsp_queue)
                list_delete_all_node (circuit->lsp_queue);
            }
          isis_event_adjacency_state_change (adj, new_state);
          isis_delete_adj (adj);
        }
      }
    }

  return;
}


void
isis_adj_print (struct isis_adjacency *adj)
{
  struct isis_dynhn *dyn;
  struct listnode *node;
  struct in_addr *ipv4_addr;
#ifdef HAVE_IPV6
  struct in6_addr *ipv6_addr;
  u_char ip6[INET6_ADDRSTRLEN];
#endif /* HAVE_IPV6 */

  if (!adj)
    return;
  dyn = dynhn_find_by_id (adj->sysid);
  if (dyn)
    zlog_debug ("%s", dyn->name.name);

  zlog_debug ("SystemId %20s SNPA %s, level %d\nHolding Time %d",
	      adj->sysid ? sysid_print (adj->sysid) : "unknown",
	      snpa_print (adj->snpa), adj->level, adj->hold_time);
  if (adj->ipv4_addrs && listcount (adj->ipv4_addrs) > 0)
    {
      zlog_debug ("IPv4 Address(es):");

      for (ALL_LIST_ELEMENTS_RO (adj->ipv4_addrs, node, ipv4_addr))
        zlog_debug ("%s", inet_ntoa (*ipv4_addr));
    }

#ifdef HAVE_IPV6
  if (adj->ipv6_addrs && listcount (adj->ipv6_addrs) > 0)
    {
      zlog_debug ("IPv6 Address(es):");
      for (ALL_LIST_ELEMENTS_RO (adj->ipv6_addrs, node, ipv6_addr))
	{
	  inet_ntop (AF_INET6, ipv6_addr, (char *)ip6, INET6_ADDRSTRLEN);
	  zlog_debug ("%s", ip6);
	}
    }
#endif /* HAVE_IPV6 */
  zlog_debug ("Speaks: %s", nlpid2string (&adj->nlpids));

  return;
}

void
isisAdjExpire (Int32T fd, Int16T event, void *args)
{
  struct isis_adjacency *adj;

  /*
   * Get the adjacency
   */
  adj = (struct isis_adjacency *)args;
  assert (adj);
  adj->pTimerExpire = NULL;

  /* trigger the adj expire event */
  isis_adj_state_change (adj, ISIS_ADJ_DOWN, "holding time expired");

}

/*
 * show isis neighbor [detail]
 */
void
isisAdjPrintCmsh (struct isis_adjacency *adj, struct cmsh *cmsh, char detail)
{
#ifdef HAVE_IPV6
	struct in6_addr *ipv6_addr;
	u_char ip6[INET6_ADDRSTRLEN];
#endif /* HAVE_IPV6 */
	struct in_addr *ip_addr;
	time_t now;
	struct isis_dynhn *dyn;
	int level;
	struct listnode *node;

	Int32T idx = 0;
	Int8T printBuff[1024] = {};

	dyn = dynhn_find_by_id(adj->sysid);
	if (dyn) {
		idx += sprintf(printBuff + idx, "  %-20s", dyn->name.name);
	} else if (adj->sysid) {
		idx += sprintf(printBuff + idx, "  %-20s", sysid_print(adj->sysid));
	} else {
		idx += sprintf(printBuff + idx, "  unknown ");
	}

	if (detail == ISIS_UI_LEVEL_BRIEF) {
		if (adj->circuit)
			idx += sprintf(printBuff + idx, "%-12s", adj->circuit->interface->name);
		else
			idx += sprintf(printBuff + idx, "NULL circuit!");
		idx += sprintf(printBuff + idx, "%-3u%-13s",
				adj->level, adj_state2string(adj->adj_state));
		now = time(NULL);
		if (adj->last_upd)
			idx += sprintf(printBuff + idx, "%-9lu", adj->last_upd + adj->hold_time - now);
		else
			idx += sprintf(printBuff + idx, "-        ");
		idx += sprintf(printBuff + idx, "%-10s", snpa_print(adj->snpa));

		cmdPrint(cmsh, "%s", printBuff);
		idx = 0; memset(printBuff, 0, 1024);
	}

	if (detail == ISIS_UI_LEVEL_DETAIL) {
		level = adj->level;
		cmdPrint(cmsh, "%s", printBuff);
		idx = 0; memset(printBuff, 0, 1024);
		if (adj->circuit)
			idx += sprintf(printBuff + idx, "    Interface: %s",
					adj->circuit->interface->name);
		else
			idx += sprintf(printBuff + idx, "    Interface: NULL circuit");
		idx += sprintf(printBuff + idx, ", Level: %u, State: %s",
				adj->level, adj_state2string(adj->adj_state));
		now = time(NULL);
		if (adj->last_upd)
			idx += sprintf(printBuff + idx, ", Expires in %s",
					time2string(adj->last_upd + adj->hold_time - now));
		else
			idx += sprintf(printBuff + idx, ", Expires in %s",
					time2string(adj->hold_time));

		cmdPrint(cmsh, "%s", printBuff);
		idx = 0; memset(printBuff, 0, 1024);

		cmdPrint(cmsh, "    Adjacency flaps: %u, Last: %s ago",
				adj->flaps, time2string(now - adj->last_flap));
		cmdPrint(cmsh, "    Circuit type: %s, Speaks: %s",
				circuit_t2string(adj->circuit_t), nlpid2string(&adj->nlpids));
		idx += sprintf(printBuff + idx, "    SNPA: %s", snpa_print(adj->snpa));
		if (adj->circuit && (adj->circuit->circ_type == CIRCUIT_T_BROADCAST)) {
			dyn = dynhn_find_by_id(adj->lanid);
			if (dyn)
				idx += sprintf(printBuff + idx, ", LAN id: %s.%02x", dyn->name.name,
						adj->lanid[ISIS_SYS_ID_LEN]);
			else
				idx += sprintf(printBuff + idx, ", LAN id: %s.%02x", sysid_print(adj->lanid),
						adj->lanid[ISIS_SYS_ID_LEN]);

			cmdPrint(cmsh, "%s", printBuff);
			idx = 0; memset(printBuff, 0, 1024);

			idx += sprintf(printBuff + idx,
					"    LAN Priority: %u, %s, DIS flaps: %u, Last: %s ago",
					adj->prio[adj->level - 1],
					isis_disflag2string( adj->dis_record[ISIS_LEVELS + level - 1].dis),
					adj->dischanges[level - 1],
					time2string( now - (adj->dis_record[ISIS_LEVELS + level - 1].last_dis_change)));

		}
		cmdPrint(cmsh, "%s", printBuff);
		idx = 0; memset(printBuff, 0, 1024);

		if (adj->area_addrs && listcount (adj->area_addrs) > 0) {
			struct area_addr *area_addr;
			cmdPrint(cmsh, "    Area Address(es):");
			for (ALL_LIST_ELEMENTS_RO(adj->area_addrs, node, area_addr))
				cmdPrint(cmsh, "      %s",
						isonet_print(area_addr->area_addr, area_addr->addr_len));
		}
		if (adj->ipv4_addrs && listcount (adj->ipv4_addrs) > 0) {
			cmdPrint(cmsh, "    IPv4 Address(es):");
			for (ALL_LIST_ELEMENTS_RO(adj->ipv4_addrs, node, ip_addr))
				cmdPrint(cmsh, "      %s", inet_ntoa(*ip_addr));
		}
#ifdef HAVE_IPV6
		if (adj->ipv6_addrs && listcount (adj->ipv6_addrs) > 0) {
			cmdPrint(cmsh, "    IPv6 Address(es):\n");
			for (ALL_LIST_ELEMENTS_RO(adj->ipv6_addrs, node, ipv6_addr)) {
				inet_ntop(AF_INET6, ipv6_addr, (char *) ip6, INET6_ADDRSTRLEN);
				cmdPrint(cmsh, "      %s", ip6);
			}
		}
#endif /* HAVE_IPV6 */
		cmdPrint(cmsh, "\n");
	}
	return;
}

void
isis_adj_build_neigh_list (struct list *adjdb, struct list *list)
{
  struct isis_adjacency *adj;
  struct listnode *node;

  if (!list)
    {
      zlog_warn ("isis_adj_build_neigh_list(): NULL list");
      return;
    }

  for (ALL_LIST_ELEMENTS_RO (adjdb, node, adj))
    {
      if (!adj)
	{
	  zlog_warn ("isis_adj_build_neigh_list(): NULL adj");
	  return;
	}

      if ((adj->adj_state == ISIS_ADJ_UP ||
	   adj->adj_state == ISIS_ADJ_INITIALIZING))
	listnode_add (list, adj->snpa);
    }
  return;
}

void
isis_adj_build_up_list (struct list *adjdb, struct list *list)
{
  struct isis_adjacency *adj;
  struct listnode *node;

  if (!list)
    {
      zlog_warn ("isis_adj_build_up_list(): NULL list");
      return;
    }

  for (ALL_LIST_ELEMENTS_RO (adjdb, node, adj))
    {
      if (!adj)
	{
	  zlog_warn ("isis_adj_build_up_list(): NULL adj");
	  return;
	}

      if (adj->adj_state == ISIS_ADJ_UP)
	listnode_add (list, adj);
    }

  return;
}
