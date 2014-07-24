/*
 * IS-IS Rout(e)ing protocol - isis_circuit.h
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
#ifdef GNU_LINUX
#include <net/ethernet.h>
#else
#include <netinet/if_ether.h>
#endif

#include <sys/time.h>
#include <unistd.h>

#ifndef ETHER_ADDR_LEN
#define	ETHER_ADDR_LEN	ETHERADDRL
#endif

#include "log.h"
#include "memory.h"
#include "if.h"
#include "linklist.h"
#include "command.h"
#include "hash.h"
#include "prefix.h"
#include "stream.h"

#include "dict.h"
#include "include-netbsd/iso.h"
#include "isis_constants.h"
#include "isis_common.h"
#include "isis_flags.h"
#include "isis_circuit.h"
#include "isis_tlv.h"
#include "isis_lsp.h"
#include "isis_pdu.h"
#include "isis_network.h"
#include "isis_misc.h"
#include "isis_constants.h"
#include "isis_adjacency.h"
#include "isis_dr.h"
#include "isisd.h"
#include "isis_csm.h"
#include "isis_events.h"

#include "nnDefines.h"
#include "nosLib.h"

#include "nnCmdDefines.h"
#include "nnCompProcess.h"
#include "nnCmdCmsh.h"

/*
 * Prototypes.
 */
int isis_if_new_hook(struct interface *);
int isis_if_delete_hook(struct interface *);


struct isis_circuit *
isis_circuit_new ()
{
  struct isis_circuit *circuit;
  int i;

  circuit = XCALLOC (MTYPE_ISIS_CIRCUIT, sizeof (struct isis_circuit));
  if (circuit == NULL)
    {
      zlog_err ("Can't malloc isis circuit");
      return NULL;
    }

  /*
   * Default values
   */
  circuit->is_type = IS_LEVEL_1_AND_2;
  circuit->flags = 0;
  circuit->pad_hellos = 1;
  for (i = 0; i < 2; i++)
    {
      circuit->hello_interval[i] = DEFAULT_HELLO_INTERVAL;
      circuit->hello_multiplier[i] = DEFAULT_HELLO_MULTIPLIER;
      circuit->csnp_interval[i] = DEFAULT_CSNP_INTERVAL;
      circuit->psnp_interval[i] = DEFAULT_PSNP_INTERVAL;
      circuit->priority[i] = DEFAULT_PRIORITY;
      circuit->metrics[i].metric_default = DEFAULT_CIRCUIT_METRIC;
      circuit->metrics[i].metric_expense = METRICS_UNSUPPORTED;
      circuit->metrics[i].metric_error = METRICS_UNSUPPORTED;
      circuit->metrics[i].metric_delay = METRICS_UNSUPPORTED;
      circuit->te_metric[i] = DEFAULT_CIRCUIT_METRIC;
    }

  return circuit;
}

void
isis_circuit_del (struct isis_circuit *circuit)
{
  if (!circuit)
    return;

  isis_circuit_if_unbind (circuit, circuit->interface);

  /* and lastly the circuit itself */
  XFREE (MTYPE_ISIS_CIRCUIT, circuit);

  return;
}

void
isis_circuit_configure (struct isis_circuit *circuit, struct isis_area *area)
{
  assert (area);
  circuit->area = area;

  /*
   * The level for the circuit is same as for the area, unless configured
   * otherwise.
   */
  if (area->is_type != IS_LEVEL_1_AND_2 && area->is_type != circuit->is_type)
    zlog_warn ("circut %s is_type %d mismatch with area %s is_type %d",
               circuit->interface->name, circuit->is_type,
               circuit->area->area_tag, area->is_type);

  /*
   * Add the circuit into area
   */
  listnode_add (area->circuit_list, circuit);

  circuit->idx = flags_get_index (&area->flags);

  return;
}

void
isis_circuit_deconfigure (struct isis_circuit *circuit, struct isis_area *area)
{
  /* Free the index of SRM and SSN flags */
  flags_free_index (&area->flags, circuit->idx);
  circuit->idx = 0;
  /* Remove circuit from area */
  assert (circuit->area == area);
  listnode_delete (area->circuit_list, circuit);
  circuit->area = NULL;

  return;
}

struct isis_circuit *
circuit_lookup_by_ifp (struct interface *ifp, struct list *list)
{
  struct isis_circuit *circuit = NULL;
  struct listnode *node;

  if (!list)
    return NULL;

  for (ALL_LIST_ELEMENTS_RO (list, node, circuit))
    if (circuit->interface == ifp)
      {
        assert (ifp->info == circuit);
        return circuit;
      }

  return NULL;
}

struct isis_circuit *
circuit_scan_by_ifp (struct interface *ifp)
{
  struct isis_area *area;
  struct listnode *node;
  struct isis_circuit *circuit;

  if (ifp->info)
    return (struct isis_circuit *)ifp->info;

  if (isis->area_list)
    {
      for (ALL_LIST_ELEMENTS_RO (isis->area_list, node, area))
        {
          circuit = circuit_lookup_by_ifp (ifp, area->circuit_list);
          if (circuit)
            return circuit;
        }
    }
  return circuit_lookup_by_ifp (ifp, isis->init_circ_list);
}

void
isis_circuit_add_addr (struct isis_circuit *circuit,
		       struct connected *connected)
{
  struct listnode *node;
  struct prefix_ipv4 *ipv4;
  u_char buf[BUFSIZ];
#ifdef HAVE_IPV6
  struct prefix_ipv6 *ipv6;
#endif /* HAVE_IPV6 */

  memset (&buf, 0, BUFSIZ);
  if (connected->address->family == AF_INET)
    {
      u_int32_t addr = connected->address->u.prefix4.s_addr;
      addr = ntohl (addr);
      if (IPV4_NET0(addr) ||
          IPV4_NET127(addr) ||
          IN_CLASSD(addr) ||
          IPV4_LINKLOCAL(addr))
        return;

      for (ALL_LIST_ELEMENTS_RO (circuit->ip_addrs, node, ipv4))
        if (prefix_same ((struct prefix *) ipv4, connected->address))
          return;

      ipv4 = prefix_ipv4_new ();
      ipv4->prefixlen = connected->address->prefixlen;
      ipv4->prefix = connected->address->u.prefix4;
      listnode_add (circuit->ip_addrs, ipv4);
      if (circuit->area)
        lsp_regenerate_schedule (circuit->area, circuit->is_type, 0);

#ifdef EXTREME_DEBUG
      prefix2str (connected->address, buf, BUFSIZ);
      zlog_debug ("Added IP address %s to circuit %d", buf,
		 circuit->circuit_id);
#endif /* EXTREME_DEBUG */
    }
#ifdef HAVE_IPV6
  if (connected->address->family == AF_INET6)
    {
      if (IN6_IS_ADDR_LOOPBACK(&connected->address->u.prefix6))
        return;

      for (ALL_LIST_ELEMENTS_RO (circuit->ipv6_link, node, ipv6))
        if (prefix_same ((struct prefix *) ipv6, connected->address))
          return;
      for (ALL_LIST_ELEMENTS_RO (circuit->ipv6_non_link, node, ipv6))
        if (prefix_same ((struct prefix *) ipv6, connected->address))
          return;

      ipv6 = prefix_ipv6_new ();
      ipv6->prefixlen = connected->address->prefixlen;
      ipv6->prefix = connected->address->u.prefix6;

      if (IN6_IS_ADDR_LINKLOCAL (&ipv6->prefix))
	listnode_add (circuit->ipv6_link, ipv6);
      else
	listnode_add (circuit->ipv6_non_link, ipv6);
      if (circuit->area)
        lsp_regenerate_schedule (circuit->area, circuit->is_type, 0);

#ifdef EXTREME_DEBUG
      prefix2str (connected->address, buf, BUFSIZ);
      zlog_debug ("Added IPv6 address %s to circuit %d", buf,
		 circuit->circuit_id);
#endif /* EXTREME_DEBUG */
    }
#endif /* HAVE_IPV6 */
  return;
}

void
isis_circuit_del_addr (struct isis_circuit *circuit,
		       struct connected *connected)
{
  struct prefix_ipv4 *ipv4, *ip = NULL;
  struct listnode *node;
  u_char buf[BUFSIZ];
#ifdef HAVE_IPV6
  struct prefix_ipv6 *ipv6, *ip6 = NULL;
  int found = 0;
#endif /* HAVE_IPV6 */

  memset (&buf, 0, BUFSIZ);
  if (connected->address->family == AF_INET)
    {
      ipv4 = prefix_ipv4_new ();
      ipv4->prefixlen = connected->address->prefixlen;
      ipv4->prefix = connected->address->u.prefix4;

      for (ALL_LIST_ELEMENTS_RO (circuit->ip_addrs, node, ip))
        if (prefix_same ((struct prefix *) ip, (struct prefix *) ipv4))
          break;

      if (ip)
	{
	  listnode_delete (circuit->ip_addrs, ip);
          if (circuit->area)
            lsp_regenerate_schedule (circuit->area, circuit->is_type, 0);
	}
      else
	{
	  prefix2str (connected->address, (char *)buf, BUFSIZ);
	  zlog_warn ("Nonexitant ip address %s removal attempt from \
                      circuit %d", buf, circuit->circuit_id);
	}

      prefix_ipv4_free (ipv4);
    }
#ifdef HAVE_IPV6
  if (connected->address->family == AF_INET6)
    {
      ipv6 = prefix_ipv6_new ();
      ipv6->prefixlen = connected->address->prefixlen;
      ipv6->prefix = connected->address->u.prefix6;

      if (IN6_IS_ADDR_LINKLOCAL (&ipv6->prefix))
	{
	  for (ALL_LIST_ELEMENTS_RO (circuit->ipv6_link, node, ip6))
	    {
	      if (prefix_same ((struct prefix *) ip6, (struct prefix *) ipv6))
		break;
	    }
	  if (ip6)
	    {
	      listnode_delete (circuit->ipv6_link, ip6);
	      found = 1;
	    }
	}
      else
	{
	  for (ALL_LIST_ELEMENTS_RO (circuit->ipv6_non_link, node, ip6))
	    {
	      if (prefix_same ((struct prefix *) ip6, (struct prefix *) ipv6))
		break;
	    }
	  if (ip6)
	    {
	      listnode_delete (circuit->ipv6_non_link, ip6);
	      found = 1;
	    }
	}

      if (!found)
	{
	  prefix2str (connected->address, (char *)buf, BUFSIZ);
	  zlog_warn ("Nonexitant ip address %s removal attempt from \
		      circuit %d", buf, circuit->circuit_id);
	}
      else if (circuit->area)
	  lsp_regenerate_schedule (circuit->area, circuit->is_type, 0);

      prefix_ipv6_free (ipv6);
    }
#endif /* HAVE_IPV6 */
  return;
}

static u_char
isis_circuit_id_gen (struct interface *ifp)
{
  u_char id = 0;
  char ifname[16];
  unsigned int i;
  int start = -1, end = -1;

  /*
   * Get a stable circuit id from ifname. This makes
   * the ifindex from flapping when netdevs are created
   * and deleted on the fly. Note that this circuit id
   * is used in pseudo lsps so it is better to be stable.
   * The following code works on any reasonanle ifname
   * like: eth1 or trk-1.1 etc.
   */
  for (i = 0; i < strlen (ifp->name); i++)
    {
      if (isdigit(ifp->name[i]))
        {
          if (start < 0)
            {
              start = i;
              end = i + 1;
            }
          else
            {
              end = i + 1;
            }
        }
      else if (start >= 0)
        break;
    }

  if ((start >= 0) && (end >= start) && (end - start) < 16)
    {
      memset (ifname, 0, 16);
      strncpy (ifname, &ifp->name[start], end - start);
      id = (u_char)atoi(ifname);
    }

  /* Try to be unique. */
  if (!id)
    id = (u_char)((ifp->ifindex & 0xff) | 0x80);

  return id;
}

void
isis_circuit_if_add (struct isis_circuit *circuit, struct interface *ifp)
{
  struct listnode *node, *nnode;
  struct connected *conn;

  circuit->circuit_id = isis_circuit_id_gen (ifp);

  isis_circuit_if_bind (circuit, ifp);
  /*  isis_circuit_update_addrs (circuit, ifp); */

  if (if_is_broadcast (ifp))
    {
      if (circuit->circ_type_config == CIRCUIT_T_P2P)
        circuit->circ_type = CIRCUIT_T_P2P;
      else
        circuit->circ_type = CIRCUIT_T_BROADCAST;
    }
  else if (if_is_pointopoint (ifp))
    {
      circuit->circ_type = CIRCUIT_T_P2P;
    }
  else if (if_is_loopback (ifp))
    {
      circuit->circ_type = CIRCUIT_T_LOOPBACK;
      circuit->is_passive = 1;
    }
  else
    {
      /* It's normal in case of loopback etc. */
      if (isis->debugs & DEBUG_EVENTS)
        zlog_debug ("isis_circuit_if_add: unsupported media");
      circuit->circ_type = CIRCUIT_T_UNKNOWN;
    }

  circuit->ip_addrs = list_new ();
#ifdef HAVE_IPV6
  circuit->ipv6_link = list_new ();
  circuit->ipv6_non_link = list_new ();
#endif /* HAVE_IPV6 */

  for (ALL_LIST_ELEMENTS (ifp->connected, node, nnode, conn))
    isis_circuit_add_addr (circuit, conn);

  return;
}

void
isis_circuit_if_del (struct isis_circuit *circuit, struct interface *ifp)
{
  struct listnode *node, *nnode;
  struct connected *conn;

  fprintf(stderr, "xxx circuit xxx \n");
  fprintf(stderr, "xxx pointer %p \n", circuit->interface);
  fprintf(stderr, "xxx pointer %s \n", circuit->interface->name);
  fprintf(stderr, "xxx interface xxx \n");
	fprintf(stderr, "xxx pointer %p \n", ifp);
	fprintf(stderr, "xxx pointer %s \n", ifp->name);



  assert (circuit->interface == ifp);

  /* destroy addresses */
  for (ALL_LIST_ELEMENTS (ifp->connected, node, nnode, conn))
    isis_circuit_del_addr (circuit, conn);

  if (circuit->ip_addrs)
    {
      assert (listcount(circuit->ip_addrs) == 0);
      list_delete (circuit->ip_addrs);
      circuit->ip_addrs = NULL;
    }

#ifdef HAVE_IPV6
  if (circuit->ipv6_link)
    {
      assert (listcount(circuit->ipv6_link) == 0);
      list_delete (circuit->ipv6_link);
      circuit->ipv6_link = NULL;
    }

  if (circuit->ipv6_non_link)
    {
      assert (listcount(circuit->ipv6_non_link) == 0);
      list_delete (circuit->ipv6_non_link);
      circuit->ipv6_non_link = NULL;
    }
#endif /* HAVE_IPV6 */

  circuit->circ_type = CIRCUIT_T_UNKNOWN;
  circuit->circuit_id = 0;

  return;
}

void
isis_circuit_if_bind (struct isis_circuit *circuit, struct interface *ifp)
{
  assert (circuit != NULL);
  assert (ifp != NULL);
  if (circuit->interface)
    assert (circuit->interface == ifp);
  else
    circuit->interface = ifp;
  if (ifp->info)
    assert (ifp->info == circuit);
  else
    ifp->info = circuit;
}

void
isis_circuit_if_unbind (struct isis_circuit *circuit, struct interface *ifp)
{
  assert (circuit != NULL);
  assert (ifp != NULL);
  assert (circuit->interface == ifp);
  assert (ifp->info == circuit);
  circuit->interface = NULL;
  ifp->info = NULL;
}

static void
isis_circuit_update_all_srmflags (struct isis_circuit *circuit, int is_set)
{
  struct isis_area *area;
  struct isis_lsp *lsp;
  dnode_t *dnode, *dnode_next;
  int level;

  assert (circuit);
  area = circuit->area;
  assert (area);
  for (level = ISIS_LEVEL1; level <= ISIS_LEVEL2; level++)
    {
      if (level & circuit->is_type)
        {
          if (area->lspdb[level - 1] &&
              dict_count (area->lspdb[level - 1]) > 0)
            {
              for (dnode = dict_first (area->lspdb[level - 1]);
                   dnode != NULL; dnode = dnode_next)
                {
                  dnode_next = dict_next (area->lspdb[level - 1], dnode);
                  lsp = dnode_get (dnode);
                  if (is_set)
                    {
                      ISIS_SET_FLAG (lsp->SRMflags, circuit);
                    }
                  else
                    {
                      ISIS_CLEAR_FLAG (lsp->SRMflags, circuit);
                    }
                }
            }
        }
    }
}

int
isis_circuit_up (struct isis_circuit *circuit)
{
	int retv;
	struct timeval tv = { 0, 0};

	/* Set the flags for all the lsps of the circuit. */
	isis_circuit_update_all_srmflags(circuit, 1);

	if (circuit->state == C_STATE_UP)
		return ISIS_OK;

	if (circuit->is_passive)
		return ISIS_OK;

	if (circuit->circ_type == CIRCUIT_T_BROADCAST) {
		/*
		 * Get the Hardware Address
		 */
#ifdef HAVE_STRUCT_SOCKADDR_DL
#ifndef SUNOS_5
		if (circuit->interface->sdl.sdl_alen != ETHER_ADDR_LEN)
		zlog_warn ("unsupported link layer");
		else
		memcpy (circuit->u.bc.snpa, LLADDR (&circuit->interface->sdl),
				ETH_ALEN);
#endif
#else
		if (circuit->interface->hw_addr_len != ETH_ALEN) {
			zlog_warn("unsupported link layer");
		} else {
			memcpy(circuit->u.bc.snpa, circuit->interface->hw_addr, ETH_ALEN);
		}
#ifdef EXTREME_DEGUG
		zlog_debug ("isis_circuit_if_add: if_id %d, isomtu %d snpa %s",
				circuit->interface->ifindex, ISO_MTU (circuit),
				snpa_print (circuit->u.bc.snpa));
#endif /* EXTREME_DEBUG */
#endif /* HAVE_STRUCT_SOCKADDR_DL */

		circuit->u.bc.adjdb[0] = list_new();
		circuit->u.bc.adjdb[1] = list_new();

		if (circuit->area->min_bcast_mtu == 0 ||
		ISO_MTU (circuit) < circuit->area->min_bcast_mtu)
			circuit->area->min_bcast_mtu = ISO_MTU(circuit);
		/*
		 * ISO 10589 - 8.4.1 Enabling of broadcast circuits
		 */

		/* initilizing the hello sending threads
		 * for a broadcast IF
		 */

		/* 8.4.1 a) commence sending of IIH PDUs */

		if (circuit->is_type & IS_LEVEL_1) {
			tv.tv_sec = 0;
			taskTimerSet(sendLanL1Hello, tv, 0, (void *) circuit);
			circuit->u.bc.lan_neighs[0] = list_new();
		}

		if (circuit->is_type & IS_LEVEL_2) {
			tv.tv_sec = 0;
			taskTimerSet(sendLanL2Hello, tv, 0, (void *) circuit);
			circuit->u.bc.lan_neighs[1] = list_new();
		}

		/* 8.4.1 b) FIXME: solicit ES - 8.4.6 */
		/* 8.4.1 c) FIXME: listen for ESH PDUs */

		/* 8.4.1 d) */
		/* dr election will commence in... */
		if (circuit->is_type & IS_LEVEL_1) {
			tv.tv_sec = 2 * circuit->hello_interval[0];
			circuit->u.bc.pTimerRunDr[0] = taskTimerSet(isisRunDrL1, tv, 0,
					(void *) circuit);
		}

		if (circuit->is_type & IS_LEVEL_2) {
			tv.tv_sec = 2 * circuit->hello_interval[1];
			circuit->u.bc.pTimerRunDr[1] = taskTimerSet(isisRunDrL2, tv, 0,
					(void *) circuit);
		}

	} else {
		/* initializing the hello send threads
		 * for a ptp IF
		 */
		tv.tv_sec = 0;
		circuit->u.p2p.neighbor = NULL;
		taskTimerSet(sendP2pHello, tv, 0, (void *) circuit);
	}

	/* initializing PSNP timers */
	if (circuit->is_type & IS_LEVEL_1) {
		tv.tv_sec = circuit->psnp_interval[0];
		circuit->pTimerSendPsnp[0] = taskTimerSet(sendL1Psnp, tv, 0,
				(void *) circuit);
	}

	if (circuit->is_type & IS_LEVEL_2) {
		tv.tv_sec = circuit->psnp_interval[1];
		circuit->pTimerSendPsnp[1] = taskTimerSet(sendL2Psnp, tv, 0,
				(void *) circuit);
	}

	/* unified init for circuits; ignore warnings below this level */
	retv = isis_sock_init(circuit);
	if (retv != ISIS_OK) {
		isis_circuit_down(circuit);
		return retv;
	}

	/* initialize the circuit streams after opening connection */
	if (circuit->rcv_stream == NULL)
		circuit->rcv_stream = stream_new(ISO_MTU(circuit));

	if (circuit->snd_stream == NULL)
		circuit->snd_stream = stream_new(ISO_MTU(circuit));

	if (!circuit->is_passive) {
		circuit->pSockFdEvent
				= taskFdSet(isisReceive, circuit->fd,
								TASK_READ | TASK_PERSIST, TASK_PRI_MIDDLE, (void *) circuit);
	} else {
		circuit->pSockFdEvent
				= taskFdSet(isisReceive, circuit->fd,
									TASK_READ, TASK_PRI_MIDDLE, (void *) circuit);
	}

	circuit->lsp_queue = list_new();
	circuit->lsp_queue_last_cleared = time(NULL);

	return ISIS_OK;
}

void
isis_circuit_down (struct isis_circuit *circuit)
{
  if (circuit->state != C_STATE_UP)
    return;

  /* Clear the flags for all the lsps of the circuit. */
  isis_circuit_update_all_srmflags (circuit, 0);

  if (circuit->circ_type == CIRCUIT_T_BROADCAST)
    {
      /* destroy neighbour lists */
      if (circuit->u.bc.lan_neighs[0])
        {
          list_delete (circuit->u.bc.lan_neighs[0]);
          circuit->u.bc.lan_neighs[0] = NULL;
        }
      if (circuit->u.bc.lan_neighs[1])
        {
          list_delete (circuit->u.bc.lan_neighs[1]);
          circuit->u.bc.lan_neighs[1] = NULL;
        }
      /* destroy adjacency databases */
      if (circuit->u.bc.adjdb[0])
        {
          circuit->u.bc.adjdb[0]->del = isis_delete_adj;
          list_delete (circuit->u.bc.adjdb[0]);
          circuit->u.bc.adjdb[0] = NULL;
        }
      if (circuit->u.bc.adjdb[1])
        {
          circuit->u.bc.adjdb[1]->del = isis_delete_adj;
          list_delete (circuit->u.bc.adjdb[1]);
          circuit->u.bc.adjdb[1] = NULL;
        }
      if (circuit->u.bc.is_dr[0])
        {
          isis_dr_resign (circuit, 1);
          circuit->u.bc.is_dr[0] = 0;
        }
      memset (circuit->u.bc.l1_desig_is, 0, ISIS_SYS_ID_LEN + 1);
      if (circuit->u.bc.is_dr[1])
        {
          isis_dr_resign (circuit, 2);
          circuit->u.bc.is_dr[1] = 0;
        }
      memset (circuit->u.bc.l2_desig_is, 0, ISIS_SYS_ID_LEN + 1);
      memset (circuit->u.bc.snpa, 0, ETH_ALEN);

      if(circuit->u.bc.pTimerSendLanHello[0])
      {
    	  taskTimerDel(circuit->u.bc.pTimerSendLanHello[0]);
    	  circuit->u.bc.pTimerSendLanHello[0] = NULL;
      }

      if(circuit->u.bc.pTimerSendLanHello[1])
      {
    	  taskTimerDel(circuit->u.bc.pTimerSendLanHello[1]);
    	  circuit->u.bc.pTimerSendLanHello[1] = NULL;
      }

      if(circuit->u.bc.pTimerRunDr[0])
      {
    	  taskTimerDel(circuit->u.bc.pTimerRunDr[0]);
    	  circuit->u.bc.pTimerRunDr[0] = NULL;
      }

      if(circuit->u.bc.pTimerRunDr[1])
      {
    	  taskTimerDel(circuit->u.bc.pTimerRunDr[1]);
    	  circuit->u.bc.pTimerRunDr[1] = NULL;
      }

      if(circuit->u.bc.pTimerRefreshPseudoLsp[0])
      {
    	  taskTimerDel(circuit->u.bc.pTimerRefreshPseudoLsp[0]);
    	  circuit->u.bc.pTimerRefreshPseudoLsp[0] = NULL;
      }

      if(circuit->u.bc.pTimerRefreshPseudoLsp[1])
      {
    	  taskTimerDel(circuit->u.bc.pTimerRefreshPseudoLsp[1]);
    	  circuit->u.bc.pTimerRefreshPseudoLsp[1] = NULL;
      }
    }
  else if (circuit->circ_type == CIRCUIT_T_P2P)
    {
      isis_delete_adj (circuit->u.p2p.neighbor);
      circuit->u.p2p.neighbor = NULL;

      if(circuit->u.p2p.pTimerSendP2pHello)
      {
    	  taskTimerDel(circuit->u.p2p.pTimerSendP2pHello);
    	  circuit->u.p2p.pTimerSendP2pHello = NULL;
      }
    }

  /* Cancel all active threads */
  if(circuit->pTimerSendCsnp[0])
  {
	  taskTimerDel(circuit->pTimerSendCsnp[0]);
	  circuit->pTimerSendCsnp[0] = NULL;
  }
  if(circuit->pTimerSendCsnp[1])
  {
	  taskTimerDel(circuit->pTimerSendCsnp[1]);
	  circuit->pTimerSendCsnp[1] = NULL;
  }

  if(circuit->pTimerSendPsnp[0])
  {
	  taskTimerDel(circuit->pTimerSendPsnp[0]);
	  circuit->pTimerSendPsnp[0] = NULL;
  }
  if(circuit->pTimerSendPsnp[1])
  {
	  taskTimerDel(circuit->pTimerSendPsnp[1]);
	  circuit->pTimerSendPsnp[1] = NULL;
  }
  if(circuit->pSockFdEvent)
  {
		taskFdDel(circuit->pSockFdEvent);
		circuit->pSockFdEvent = NULL;
  }

  if (circuit->lsp_queue)
    {
      circuit->lsp_queue->del = NULL;
      list_delete (circuit->lsp_queue);
      circuit->lsp_queue = NULL;
    }

  /* send one gratuitous hello to spead up convergence */
  if (circuit->is_type & IS_LEVEL_1)
    send_hello (circuit, IS_LEVEL_1);
  if (circuit->is_type & IS_LEVEL_2)
    send_hello (circuit, IS_LEVEL_2);

  circuit->upadjcount[0] = 0;
  circuit->upadjcount[1] = 0;

  /* close the socket */
  if (circuit->fd)
    {
      close (circuit->fd);
      circuit->fd = 0;
    }

  if (circuit->rcv_stream != NULL)
    {
      stream_free (circuit->rcv_stream);
      circuit->rcv_stream = NULL;
    }

  if (circuit->snd_stream != NULL)
    {
      stream_free (circuit->snd_stream);
      circuit->snd_stream = NULL;
    }

//  thread_cancel_event (master, circuit);

  return;
}

void
circuit_update_nlpids (struct isis_circuit *circuit)
{
  circuit->nlpids.count = 0;

  if (circuit->ip_router)
    {
      circuit->nlpids.nlpids[0] = NLPID_IP;
      circuit->nlpids.count++;
    }
#ifdef HAVE_IPV6
  if (circuit->ipv6_router)
    {
      circuit->nlpids.nlpids[circuit->nlpids.count] = NLPID_IPV6;
      circuit->nlpids.count++;
    }
#endif /* HAVE_IPV6 */
  return;
}

void
isisCircuitPrintCmsh (struct isis_circuit *circuit, struct cmsh *cmsh,
                        char detail)
{
	Int32T idx = 0;
	Int8T printBuff[1024] = {};

  if (detail == ISIS_UI_LEVEL_BRIEF)
    {
	  sprintf(printBuff, "  %-12s0x%-7x%-9s%-9s%-9s",
				  circuit->interface->name,
				  circuit->circuit_id,
				  circuit_state2string (circuit->state),
				  circuit_type2string (circuit->circ_type),
				  circuit_t2string (circuit->is_type));
      cmdPrint (cmsh, "%s",printBuff);
      memset(printBuff, 0, 1024);
    }

  if (detail == ISIS_UI_LEVEL_DETAIL)
    {
	  idx += sprintf(printBuff + idx, "  Interface: %s, State: %s",
						  circuit->interface->name,
						  circuit_state2string (circuit->state)
	  	  	  	  	  );
      if (circuit->is_passive)
    	  idx += sprintf(printBuff + idx, ", Passive");
      else
    	  idx += sprintf(printBuff + idx, ", Active");
      idx += sprintf(printBuff + idx, ", Circuit Id: 0x%x", circuit->circuit_id);
      cmdPrint (cmsh, "%s",printBuff);
      idx = 0; memset(printBuff, 0, 1024);
      idx += sprintf(printBuff + idx, "    Type: %s, Level: %s",
    		  	  	  	  circuit_type2string (circuit->circ_type),
    		  	  	  	  circuit_t2string (circuit->is_type)
    		  	  	  	  );
      if (circuit->circ_type == CIRCUIT_T_BROADCAST)
    	  idx += sprintf(printBuff + idx, ", SNPA: %-10s", snpa_print (circuit->u.bc.snpa));
      cmdPrint (cmsh, "%s",printBuff);
      idx = 0; memset(printBuff, 0, 1024);
      if (circuit->is_type & IS_LEVEL_1)
        {
          cmdPrint (cmsh, "    Level-1 Information:");
          if (circuit->area->newmetric)
        	  idx += sprintf(printBuff + idx, "      Metric: %d", circuit->te_metric[0]);
          else
        	  idx += sprintf(printBuff + idx, "      Metric: %d", circuit->metrics[0].metric_default);
          if (!circuit->is_passive)
            {
        	  	idx += sprintf(printBuff + idx, ", Active neighbors: %u", circuit->upadjcount[0]);
              cmdPrint (cmsh, "%s", printBuff);
              idx = 0; memset(printBuff, 0, 1024);
              cmdPrint (cmsh, "      Hello interval: %u, "
                            "Holddown count: %u %s",
                       circuit->hello_interval[0],
                       circuit->hello_multiplier[0],
                       (circuit->pad_hellos ? "(pad)" : "(no-pad)"));
              cmdPrint (cmsh, "      CNSP interval: %u, "
                            "PSNP interval: %u",
                       circuit->csnp_interval[0],
                       circuit->psnp_interval[0]);
              if (circuit->circ_type == CIRCUIT_T_BROADCAST)
                cmdPrint (cmsh, "      LAN Priority: %u, %s",
                         circuit->priority[0],
                         (circuit->u.bc.is_dr[0] ? \
                          "is DIS" : "is not DIS"));
            }
          else
            {
              cmdPrint (cmsh, "\n");
            }
        }
      if (circuit->is_type & IS_LEVEL_2)
        {
			cmdPrint(cmsh, "    Level-2 Information:");
			if (circuit->area->newmetric)
				idx += sprintf(printBuff + idx, "      Metric: %d", circuit->te_metric[1]);
			else
				idx += sprintf(printBuff + idx, "      Metric: %d", circuit->metrics[1].metric_default);
			if (!circuit->is_passive) {
				idx += sprintf(printBuff + idx, ", Active neighbors: %u", circuit->upadjcount[1]);
				cmdPrint(cmsh, "%s", printBuff);
				idx = 0; memset(printBuff, 0, 1024);
				cmdPrint(cmsh, "      Hello interval: %u, "
						"Holddown count: %u %s", circuit->hello_interval[1],
						circuit->hello_multiplier[1],
						(circuit->pad_hellos ? "(pad)" : "(no-pad)"));
				cmdPrint(cmsh, "      CNSP interval: %u, "
						"PSNP interval: %u", circuit->csnp_interval[1],
						circuit->psnp_interval[1]);
				if (circuit->circ_type == CIRCUIT_T_BROADCAST)
					cmdPrint(cmsh, "      LAN Priority: %u, %s",
							circuit->priority[1],
							(circuit->u.bc.is_dr[1] ? "is DIS" : "is not DIS"));
			} else {
				cmdPrint(cmsh, "\n");
			}
		}
      if (circuit->ip_addrs && listcount (circuit->ip_addrs) > 0)
        {
          struct listnode *node;
          struct prefix *ip_addr;
          u_char buf[BUFSIZ];
          cmdPrint (cmsh, "    IP Prefix(es):");
          for (ALL_LIST_ELEMENTS_RO (circuit->ip_addrs, node, ip_addr))
            {
              prefix2str (ip_addr, (char*)buf, BUFSIZ);
              cmdPrint (cmsh, "      %s", buf);
            }
        }
      cmdPrint (cmsh, "");
    }
  return;
}

/* Write isis configuration of each interface. */
Int32T
configWriteIsisInterface(struct cmsh *cmsh)
{
	  struct listnode *node, *node2;
	  struct interface *ifp;
	  struct isis_area *area;
	  struct isis_circuit *circuit;
	  int i;

	  NNLOG(LOG_ERR, "CCCCCCC configWriteIsisInterface.\n");

	  for (ALL_LIST_ELEMENTS_RO (iflist, node, ifp))
	    {
	      /* IF name */
		  NNLOG(LOG_ERR, "CCCCCCC !interface %s\n", ifp->name);
		  cmdPrint (cmsh, "!");
	      cmdPrint (cmsh, "interface %s", ifp->name);
	      /* IF desc */
	      if (ifp->desc)
	        {
	    	  NNLOG(LOG_ERR, "CCCCCCC description %s\n", ifp->desc);
	          cmdPrint (cmsh, " description %s", ifp->desc);
	        }
	      /* ISIS Circuit */
	      for (ALL_LIST_ELEMENTS_RO (isis->area_list, node2, area))
	        {
	          circuit = circuit_lookup_by_ifp (ifp, area->circuit_list);
	          if (circuit == NULL)
	            continue;
	          if (circuit->ip_router)
	            {
	        	  NNLOG(LOG_ERR, "CCCCCCC ip router isis  %s\n", area->area_tag);
	              cmdPrint (cmsh, " ip router isis %s", area->area_tag);
	            }
	          if (circuit->is_passive)
	            {
	        	  NNLOG(LOG_ERR, "CCCCCCC isis passive  \n");
	              cmdPrint (cmsh, " isis passive");
	            }
	          if (circuit->circ_type_config == CIRCUIT_T_P2P)
	            {
	        	  NNLOG(LOG_ERR, "CCCCCCC isis network point-to-point \n");
	              cmdPrint (cmsh, " isis network point-to-point");
	            }
	#ifdef HAVE_IPV6
	          if (circuit->ipv6_router)
	            {
	        	  NNLOG(LOG_ERR, "CCCCCCC ipv6 router isis %s\n", area->area_tag);
	              cmdPrint (cmsh, " ipv6 router isis %s", area->area_tag);
	            }
	#endif /* HAVE_IPV6 */

	          /* ISIS - circuit type */
	          if (circuit->is_type == IS_LEVEL_1)
	            {
	        	  NNLOG(LOG_ERR, "CCCCCCC isis circuit-type level-1\n");
	              cmdPrint (cmsh, " isis circuit-type level-1");
	            }
	          else
	            {
	              if (circuit->is_type == IS_LEVEL_2)
	                {
	            	  NNLOG(LOG_ERR, "CCCCCCC isis circuit-type level-2-only\n");
	                  cmdPrint (cmsh, " isis circuit-type level-2-only");
	                }
	            }

	          /* ISIS - CSNP interval */
	          if (circuit->csnp_interval[0] == circuit->csnp_interval[1])
	            {
	              if (circuit->csnp_interval[0] != DEFAULT_CSNP_INTERVAL)
	                {
	            	  NNLOG(LOG_ERR, "CCCCCCC isis csnp-interval %d\n", circuit->csnp_interval[0]);
	                  cmdPrint (cmsh, " isis csnp-interval %d",
	                           circuit->csnp_interval[0]);
	                }
	            }
	          else
	          {
	            for (i = 0; i < 2; i++)
	              {
	                if (circuit->csnp_interval[i] != DEFAULT_CSNP_INTERVAL)
	                  {
	                	NNLOG(LOG_ERR, "CCCCCCC isis csnp-interval %d\n", circuit->csnp_interval[0]);
	                    cmdPrint (cmsh, " isis csnp-interval %d level-%d",
	                             circuit->csnp_interval[i], i + 1);
	                  }
	              }
	          }

	          /* ISIS - PSNP interval */
	          if (circuit->psnp_interval[0] == circuit->psnp_interval[1])
	            {
	              if (circuit->psnp_interval[0] != DEFAULT_PSNP_INTERVAL)
	                {
	            	  NNLOG(LOG_ERR, "CCCCCCC isis psnp-interval %d\n", circuit->psnp_interval[0]);
	                  cmdPrint (cmsh, " isis psnp-interval %d",
	                           circuit->psnp_interval[0]);
	                }
	            }
	          else
	            {
	              for (i = 0; i < 2; i++)
	                {
	                  if (circuit->psnp_interval[i] != DEFAULT_PSNP_INTERVAL)
	                  {
	                	  NNLOG(LOG_ERR, "CCCCCCC isis psnp-interval %d level-%d\n", circuit->psnp_interval[i], i + 1);
	                    cmdPrint (cmsh, " isis psnp-interval %d level-%d",
	                             circuit->psnp_interval[i], i + 1);
	                  }
	                }
	            }

	          /* ISIS - Hello padding - Defaults to true so only display if false */
	          if (circuit->pad_hellos == 0)
	            {
	        	  NNLOG(LOG_ERR, "CCCCCCC no isis hello padding\n");
	              cmdPrint (cmsh, " no isis hello padding");
	            }

	          /* ISIS - Hello interval */
	          if (circuit->hello_interval[0] == circuit->hello_interval[1])
	            {
	              if (circuit->hello_interval[0] != DEFAULT_HELLO_INTERVAL)
	                {
	            	  NNLOG(LOG_ERR, "CCCCCCC isis hello-interval %d\n", circuit->hello_interval[0]);
	                  cmdPrint (cmsh, " isis hello-interval %d",
	                           circuit->hello_interval[0]);
	                }
	            }
	          else
	            {
	              for (i = 0; i < 2; i++)
	                {
	                  if (circuit->hello_interval[i] != DEFAULT_HELLO_INTERVAL)
	                    {
	                	  NNLOG(LOG_ERR, "CCCCCCC isis hello-interval %d level-%d\n", circuit->hello_interval[i], i + 1);
	                      cmdPrint (cmsh, " isis hello-interval %d level-%d",
	                               circuit->hello_interval[i], i + 1);
	                    }
	                }
	            }

	          /* ISIS - Hello Multiplier */
	          if (circuit->hello_multiplier[0] == circuit->hello_multiplier[1])
	            {
	              if (circuit->hello_multiplier[0] != DEFAULT_HELLO_MULTIPLIER)
	                {
	            	  NNLOG(LOG_ERR, "CCCCCCC isis hello-multiplier %d\n", circuit->hello_multiplier[0]);
	                  cmdPrint (cmsh, " isis hello-multiplier %d",
	                           circuit->hello_multiplier[0]);
	                }
	            }
	          else
	            {
	              for (i = 0; i < 2; i++)
	                {
	                  if (circuit->hello_multiplier[i] != DEFAULT_HELLO_MULTIPLIER)
	                    {
	                	  NNLOG(LOG_ERR, "CCCCCCC isis hello-multiplier %d level-%d\n", circuit->hello_multiplier[i], i + 1);
	                      cmdPrint (cmsh, " isis hello-multiplier %d level-%d",
	                               circuit->hello_multiplier[i], i + 1);
	                    }
	                }
	            }

	          /* ISIS - Priority */
	          if (circuit->priority[0] == circuit->priority[1])
	            {
	              if (circuit->priority[0] != DEFAULT_PRIORITY)
	                {
	            	  NNLOG(LOG_ERR, "CCCCCCC isis priority %d\n", circuit->priority[0]);
	                  cmdPrint (cmsh, " isis priority %d",
	                           circuit->priority[0]);
	                }
	            }
	          else
	            {
	              for (i = 0; i < 2; i++)
	                {
	                  if (circuit->priority[i] != DEFAULT_PRIORITY)
	                    {
	                	  NNLOG(LOG_ERR, "CCCCCCC isis priority %d level-%d\n", circuit->priority[i], i + 1);
	                      cmdPrint (cmsh, " isis priority %d level-%d",
	                               circuit->priority[i], i + 1);
	                    }
	                }
	            }

	          /* ISIS - Metric */
	          if (circuit->te_metric[0] == circuit->te_metric[1])
	            {
	              if (circuit->te_metric[0] != DEFAULT_CIRCUIT_METRIC)
	                {
	            	  NNLOG(LOG_ERR, "CCCCCCC isis metric %d\n", circuit->te_metric[0]);
	                  cmdPrint (cmsh, " isis metric %d", circuit->te_metric[0]);
	                }
	            }
	          else
	            {
	              for (i = 0; i < 2; i++)
	                {
	                  if (circuit->te_metric[i] != DEFAULT_CIRCUIT_METRIC)
	                    {
	                	  NNLOG(LOG_ERR, "CCCCCCC isis metric %d level-%d\n", circuit->te_metric[i], i + 1);
	                      cmdPrint (cmsh, " isis metric %d level-%d",
	                               circuit->te_metric[i], i + 1);
	                    }
	                }
	            }
	          if (circuit->passwd.type == ISIS_PASSWD_TYPE_HMAC_MD5)
	            {
	        	  NNLOG(LOG_ERR, "CCCCCCC isis password md5 %s\n", circuit->passwd.passwd);
	              cmdPrint (cmsh, " isis password md5 %s", circuit->passwd.passwd);
	            }
	          else if (circuit->passwd.type == ISIS_PASSWD_TYPE_CLEARTXT)
	            {
	        	  NNLOG(LOG_ERR, "CCCCCCC isis password clear %s\n", circuit->passwd.passwd);
	              cmdPrint (cmsh, " isis password clear %s", circuit->passwd.passwd);
	            }
	        }
	    }

	  return SUCCESS;
}



int
isis_if_new_hook (struct interface *ifp)
{
  return 0;
}

int
isis_if_delete_hook (struct interface *ifp)
{
  struct isis_circuit *circuit;
  /* Clean up the circuit data */
  if (ifp && ifp->info)
    {
      circuit = ifp->info;
      isis_csm_state_change (IF_DOWN_FROM_Z, circuit, circuit->area);
      isis_csm_state_change (ISIS_DISABLE, circuit, circuit->area);
    }

  return 0;
}

void
isis_circuit_init ()
{
  /* Initialize Zebra interface data structure */
  if_init ();
  if_add_hook (IF_NEW_HOOK, isis_if_new_hook);
  if_add_hook (IF_DELETE_HOOK, isis_if_delete_hook);

}

#if 0
#ifdef HAVE_IPV6
DECMD (cmdFuncIsisIpv6RouterIsis,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "ipv6 router isis WORD",
       "IPv6 interface subcommands",
       "IPv6 Router interface commands",
       "IS-IS Routing for IPv6",
       "Routing process tag")
{
	struct isis_circuit *circuit;
	struct interface *ifp;
	struct isis_area *area = NULL;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv[1], strnlen(uargv[1], INTERFACE_NAMSIZ));
	assert(ifp);

	circuit = circuit_lookup_by_ifp(ifp, area->circuit_list);
	if (circuit) {
		if (circuit->ipv6_router == 1) {
			if (strcmp(circuit->area->area_tag, cargv[3])) {
				cmdPrint(cmsh,
						"ISIS circuit is already defined for IPv6 on %s\n",
						circuit->area->area_tag);
				return (CMD_IPC_ERROR);
			}
			return (CMD_IPC_OK);
		}
	}

	area = isisAreaGet(cargv[3]);
	if (!area) {
		cmdPrint(cmsh, "Can't find ISIS instance \n");
		return (CMD_IPC_ERROR);
	}

	circuit = isis_csm_state_change(ISIS_ENABLE, circuit, area);
	isis_circuit_if_bind(circuit, ifp);

	circuit->ipv6_router = 1;
	area->ipv6_circuits++;
	circuit_update_nlpids(circuit);

	return (CMD_IPC_OK);
}

DECMD (cmdFuncIsisNoIpv6RouterIsis,
		CMD_NODE_INTERFACE,
		IPC_ISIS,
       "no ipv6 router isis WORD",
       "Negate a command or set its defaults",
       "IPv6 interface subcommands",
       "IPv6 Router interface commands",
       "IS-IS Routing for IPv6",
       "Routing process tag")
{
	struct interface *ifp;
	struct isis_area *area;
	struct listnode *node;
	struct isis_circuit *circuit;

	/* Check isis is installed or not. */
	if (!isis) {
		cmdPrint(cmsh, "Not installed isis.\n");
		return (CMD_IPC_ERROR);
	}

	ifp = if_get_by_name_len(uargv[1], strnlen(uargv[1], INTERFACE_NAMSIZ));
	if (!ifp) {
		cmdPrint(cmsh, "Invalid interface \n");
		return CMD_IPC_ERROR;
	}

	area = isis_area_lookup(cargv[4]);
	if (!area) {
		cmdPrint(cmsh, "Can't find ISIS instance %s\n", cargv[4]);
		return CMD_IPC_ERROR;
	}

	circuit = circuit_lookup_by_ifp(ifp, area->circuit_list);
	if (!circuit) {
		cmdPrint(cmsh, "ISIS is not enabled on circuit %s", ifp->name);
		return CMD_IPC_ERROR;
	}

	circuit->ipv6_router = 0;
	area->ipv6_circuits--;
	if (circuit->ip_router == 0)
		isis_csm_state_change(ISIS_DISABLE, circuit, area);

	return (CMD_IPC_OK);
}
#endif /* HAVE_IPV6 */
#endif
