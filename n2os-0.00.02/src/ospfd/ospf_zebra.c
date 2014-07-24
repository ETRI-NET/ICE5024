/*
 * Zebra connect library for OSPFd
 * Copyright (C) 1997, 98, 99, 2000 Kunihiro Ishiguro, Toshiaki Takada
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA. 
 */

#include "command.h"
#include "network.h"
#include "prefix.h"
#include "routemap.h"
#include "table.h"
#include "stream.h"
#include "memory.h"
#include "zclient.h"
#include "filter.h"
#include "plist.h"
#include "log.h"

#include "nnVector.h"
#include "nosLib.h"
#include "nnFilter.h"
#include "nnRoutemap.h"
#include "nnCmdCmsh.h"
#include "nnPlist.h"
#include "nnCmdCommon.h"

#include "ospfd.h"
#include "ospf_interface.h"
#include "ospf_ism.h"
#include "ospf_asbr.h"
#include "ospf_asbr.h"
#include "ospf_abr.h"
#include "ospf_lsa.h"
#include "ospf_dump.h"
#include "ospf_route.h"
#include "ospf_zebra.h"
#ifdef HAVE_SNMP
#include "ospf_snmp.h"
#endif /* HAVE_SNMP */

#include "nnRibDefines.h"

/* Zebra structure to hold current status. */
struct zclient *zclient = NULL;



/* Router-id update message from zebra. */


void
ospf_zebra_add (struct prefix_ipv4 *p, struct ospf_route *or)
{
	u_char distance;
	struct ospf_path *path;
	struct listnode *node;

//	  NNLOG(LOG_DEBUG, "\n OSPF_RIBMANAGER_IPV4_ADD\n");
//	  NNLOG(LOG_DEBUG, "family = %d\n", p->family);
//	  NNLOG(LOG_DEBUG, "length = %d\n", p->prefixLen);
//	  NNLOG(LOG_DEBUG, "prefix = %s\n", inet_ntoa(p->prefix));
//	  NNLOG(LOG_DEBUG, "nexthop = %s\n", inet_ntoa(*pNextHop));
//	  NNLOG(LOG_DEBUG, "metric = %d\n", metric);
//	  NNLOG(LOG_DEBUG, "distance = %d\n\n", distance);

	Uint16T position = 0;
//	Uint16T rCount;
	Uint8T rType = RIB_ROUTE_TYPE_OSPF;
	Uint8T nhType;

	/* Buffer Reset */
	nnBufferT msgBuff;
	nnBufferReset(&msgBuff);

	/* Number of Route */
	nnBufferSetInt16T(&msgBuff, or->paths->count);

	/* route type */
	nnBufferSetInt8T(&msgBuff, rType);

	for (ALL_LIST_ELEMENTS_RO(or->paths, node, path)) {
		position = nnBufferGetIndex(&msgBuff);

		/* route entry length */
		nnBufferSetInt16T(&msgBuff, 0);

		/* route flag
		 *  RIB_FLAG_NULL0 or ...
		 */
		Uint8T dummyFlags = 0;
		nnBufferSetInt8T(&msgBuff, dummyFlags);

		/*
		 * msgFlags, nexthop, ifindex, distance, metrix
		 */

		/*     #define RIB_MESSAGE_NEXTHOP                0x01
		 *     #define RIB_MESSAGE_IFINDEX                0x02
		 *     #define RIB_MESSAGE_DISTANCE               0x04
		 *     #define RIB_MESSAGE_METRIC                 0x08
		 */
		Uint8T messages = 0;

		/* OSPF pass nexthop and metric */
		messages |= RIB_MESSAGE_NEXTHOP;
		messages |= RIB_MESSAGE_METRIC;

		/* Distance value. */
		distance = ospf_distance_apply(p, or);
		if (distance)
			messages |= RIB_MESSAGE_DISTANCE;

		nnBufferSetInt8T(&msgBuff, messages);

		/* prefix length */
		nnBufferSetInt8T(&msgBuff, p->prefixlen);

		/* prefix  */
		nnBufferSetInaddr(&msgBuff, p->prefix);

		/* Nexthop, ifindex, distance and metric information. */
//		if (path->nexthop.s_addr != INADDR_ANY && path->ifindex != 0) {
//			Uint8T nhType = NEXTHOP_TYPE_IPV4_IFINDEX;
//			nnBufferSetInt8T(&msgBuff, nhType);
//			nnBufferSetInaddr(&msgBuff, path->nexthop);
//			nnBufferSetInt32T(&msgBuff, path->ifindex);
		if (path->nexthop.s_addr != INADDR_ANY) {
			nhType = NEXTHOP_TYPE_IPV4;
			nnBufferSetInt8T(&msgBuff, nhType);
			nnBufferSetInaddr(&msgBuff, path->nexthop);
		} else {
			nhType = NEXTHOP_TYPE_IFINDEX;
			nnBufferSetInt8T(&msgBuff, nhType);
			if (path->ifindex)
				nnBufferSetInt32T(&msgBuff, path->ifindex);
			else
				nnBufferSetInt32T(&msgBuff, 0);
		}

		if (IS_DEBUG_OSPF(zebra, ZEBRA_REDISTRIBUTE)) {
			char buf[2][INET_ADDRSTRLEN];
			zlog_debug("Zebra: Route add %s/%d nexthop %s",
					inet_ntop(AF_INET, &p->prefix, buf[0], sizeof(buf[0])),
					p->prefixlen,
					inet_ntop(AF_INET, &path->nexthop, buf[1], sizeof(buf[1])));
		}

		/* Distance */
		if (distance)
			nnBufferSetInt8T(&msgBuff, distance);

		/* Metric */
		if (or->path_type == OSPF_PATH_TYPE1_EXTERNAL)
			nnBufferSetInt32T(&msgBuff, or->cost + or->u.ext.type2_cost);
		else if (or->path_type == OSPF_PATH_TYPE2_EXTERNAL)
			nnBufferSetInt32T(&msgBuff, or->u.ext.type2_cost);
		else
			nnBufferSetInt32T(&msgBuff, or->cost);

		/* Message Length */
		Uint16T subLength = nnBufferGetLength(&msgBuff) - position
				- sizeof(Uint16T);
		nnBufferInsertInt16T(&msgBuff, position, subLength);
	}


	nnBufferPrint(&msgBuff);

	/* send ipc message to ribmgr */
	ipcSendAsync(RIB_MANAGER, IPC_RIB_IPV4_ROUTE_ADD, msgBuff.data,
			msgBuff.length);

}

void
ospf_zebra_delete (struct prefix_ipv4 *p, struct ospf_route *or)
{
	u_char distance;
	struct ospf_path *path;
	struct listnode *node;

//	  NNLOG(LOG_DEBUG, "\n OSPF_RIBMANAGER_IPV4_ADD\n");
//	  NNLOG(LOG_DEBUG, "family = %d\n", p->family);
//	  NNLOG(LOG_DEBUG, "length = %d\n", p->prefixLen);
//	  NNLOG(LOG_DEBUG, "prefix = %s\n", inet_ntoa(p->prefix));
//	  NNLOG(LOG_DEBUG, "nexthop = %s\n", inet_ntoa(*pNextHop));
//	  NNLOG(LOG_DEBUG, "metric = %d\n", metric);
//	  NNLOG(LOG_DEBUG, "distance = %d\n\n", distance);

	Uint16T position = 0;
//	Uint16T rCount;
	Uint8T rType = RIB_ROUTE_TYPE_OSPF;
	Uint8T nhType;

	/* Buffer Reset */
	nnBufferT msgBuff;
	nnBufferReset(&msgBuff);

	/* Number of Route */
	nnBufferSetInt16T(&msgBuff, or->paths->count);

	/* route type */
	nnBufferSetInt8T(&msgBuff, rType);

	for (ALL_LIST_ELEMENTS_RO(or->paths, node, path)) {
		position = nnBufferGetIndex(&msgBuff);

		/* route entry length */
		nnBufferSetInt16T(&msgBuff, 0);

		/* route flag
		 *  RIB_FLAG_NULL0 or ...
		 */
		Uint8T dummyFlags = 0;
		nnBufferSetInt8T(&msgBuff, dummyFlags);

		/*
		 * msgFlags, nexthop, ifindex, distance, metrix
		 */

		/*     #define RIB_MESSAGE_NEXTHOP                0x01
		 *     #define RIB_MESSAGE_IFINDEX                0x02
		 *     #define RIB_MESSAGE_DISTANCE               0x04
		 *     #define RIB_MESSAGE_METRIC                 0x08
		 */
		Uint8T messages = 0;

		/* OSPF pass nexthop and metric */
		messages |= RIB_MESSAGE_NEXTHOP;
		messages |= RIB_MESSAGE_METRIC;

		/* Distance value. */
		distance = ospf_distance_apply(p, or);
		if (distance)
			messages |= RIB_MESSAGE_DISTANCE;

		nnBufferSetInt8T(&msgBuff, messages);

		/* prefix length */
		nnBufferSetInt8T(&msgBuff, p->prefixlen);

		/* prefix  */
		nnBufferSetInaddr(&msgBuff, p->prefix);

		/* Nexthop, ifindex, distance and metric information. */
//		if (path->nexthop.s_addr != INADDR_ANY && path->ifindex != 0) {
//			Uint8T nhType = NEXTHOP_TYPE_IPV4_IFINDEX;
//			nnBufferSetInt8T(&msgBuff, nhType);
//			nnBufferSetInaddr(&msgBuff, path->nexthop);
//			nnBufferSetInt32T(&msgBuff, path->ifindex);
		if (path->nexthop.s_addr != INADDR_ANY) {
			nhType = NEXTHOP_TYPE_IPV4;
			nnBufferSetInt8T(&msgBuff, nhType);
			nnBufferSetInaddr(&msgBuff, path->nexthop);
		} else {
			nhType = NEXTHOP_TYPE_IFINDEX;
			nnBufferSetInt8T(&msgBuff, nhType);
			if (path->ifindex)
				nnBufferSetInt32T(&msgBuff, path->ifindex);
			else
				nnBufferSetInt32T(&msgBuff, 0);
		}

		if (IS_DEBUG_OSPF(zebra, ZEBRA_REDISTRIBUTE)) {
			char buf[2][INET_ADDRSTRLEN];
			zlog_debug("Zebra: Route add %s/%d nexthop %s",
					inet_ntop(AF_INET, &p->prefix, buf[0], sizeof(buf[0])),
					p->prefixlen,
					inet_ntop(AF_INET, &path->nexthop, buf[1], sizeof(buf[1])));
		}

		/* Distance */
		if (distance)
			nnBufferSetInt8T(&msgBuff, distance);

		/* Metric */
		if (or->path_type == OSPF_PATH_TYPE1_EXTERNAL)
			nnBufferSetInt32T(&msgBuff, or->cost + or->u.ext.type2_cost);
		else if (or->path_type == OSPF_PATH_TYPE2_EXTERNAL)
			nnBufferSetInt32T(&msgBuff, or->u.ext.type2_cost);
		else
			nnBufferSetInt32T(&msgBuff, or->cost);

		/* Message Length */
		Uint16T subLength = nnBufferGetLength(&msgBuff) - position
				- sizeof(Uint16T);
		nnBufferInsertInt16T(&msgBuff, position, subLength);
	}


	nnBufferPrint(&msgBuff);

	/* send ipc message to ribmgr */
	ipcSendAsync(RIB_MANAGER, IPC_RIB_IPV4_ROUTE_DELETE, msgBuff.data,
			msgBuff.length);

}

void
ospf_zebra_add_discard (struct prefix_ipv4 *p)
{
  struct zapi_ipv4 api;

  if (om->ospfRedistribute[RIB_ROUTE_TYPE_OSPF])
    {
      api.type = RIB_ROUTE_TYPE_OSPF;
      api.flags = ZEBRA_FLAG_BLACKHOLE;
      api.message = 0;
      api.safi = SAFI_UNICAST;
      SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
      api.nexthop_num = 0;
      api.ifindex_num = 0;

      zapi_ipv4_route (ZEBRA_IPV4_ROUTE_ADD, zclient, p, &api);

      if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
        zlog_debug ("Zebra: Route add discard %s/%d",
                   inet_ntoa (p->prefix), p->prefixlen);
    }
}

void
ospf_zebra_delete_discard (struct prefix_ipv4 *p)
{
  struct zapi_ipv4 api;

  if (om->ospfRedistribute[RIB_ROUTE_TYPE_OSPF])
    {
      api.type = RIB_ROUTE_TYPE_OSPF;
      api.flags = ZEBRA_FLAG_BLACKHOLE;
      api.message = 0;
      api.safi = SAFI_UNICAST;
      SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
      api.nexthop_num = 0;
      api.ifindex_num = 0;

      zapi_ipv4_route (ZEBRA_IPV4_ROUTE_DELETE, zclient, p, &api);

      if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
        zlog_debug ("Zebra: Route delete discard %s/%d",
                   inet_ntoa (p->prefix), p->prefixlen);

    }
}

int
ospf_is_type_redistributed (int type)
{
  return (DEFAULT_ROUTE_TYPE (type)) ?
    om->ospfDefaultInformation : om->ospfRedistribute[type];

}

static int
ospf_external_lsa_originate_check (struct ospf *ospf,
                                   struct external_info *ei)
{
  /* If prefix is multicast, then do not originate LSA. */
  if (IN_MULTICAST (htonl (ei->p.prefix.s_addr)))
    {
      zlog_info ("LSA[Type5:%s]: Not originate AS-external-LSA, "
                 "Prefix belongs multicast", inet_ntoa (ei->p.prefix));
      return 0;
    }

  /* Take care of default-originate. */
  if (is_prefix_default (&ei->p))
    if (ospf->default_originate == DEFAULT_ORIGINATE_NONE)
      {
        zlog_info ("LSA[Type5:0.0.0.0]: Not originate AS-external-LSA "
                   "for default");
        return 0;
      }

  return 1;
}

/* If connected prefix is OSPF enable interface, then do not announce. */
int
ospf_distribute_check_connected (struct ospf *ospf, struct external_info *ei)
{
  struct listnode *node;
  struct ospf_interface *oi;


  for (ALL_LIST_ELEMENTS_RO (ospf->oiflist, node, oi))
      if (prefix_match (oi->address, (struct prefix *) &ei->p))
          return 0;
  return 1;
}

/* return 1 if external LSA must be originated, 0 otherwise */
int
ospf_redistribute_check (struct ospf *ospf,
                         struct external_info *ei, int *changed)
{
  struct route_map_set_values save_values;
  struct prefix_ipv4 *p = &ei->p;
  u_char type = is_prefix_default (&ei->p) ? DEFAULT_ROUTE : ei->type;

  if (changed)
    *changed = 0;

  if (!ospf_external_lsa_originate_check (ospf, ei))
    return 0;

  /* Take care connected route. */
  if (type == ZEBRA_ROUTE_CONNECT &&
      !ospf_distribute_check_connected (ospf, ei))
    return 0;

  if (!DEFAULT_ROUTE_TYPE (type) && DISTRIBUTE_NAME (ospf, type))
    /* distirbute-list exists, but access-list may not? */
    if (DISTRIBUTE_LIST (ospf, type))
      if (access_list_apply (DISTRIBUTE_LIST (ospf, type), p) == FILTER_DENY_ORG)
        {
          if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
            zlog_debug ("Redistribute[%s]: %s/%d filtered by ditribute-list.",
                       ospf_redist_string(type),
                       inet_ntoa (p->prefix), p->prefixlen);
          return 0;
        }

  save_values = ei->route_map_set;
  ospf_reset_route_map_set_values (&ei->route_map_set);

  /* apply route-map if needed */
  if (ROUTEMAP_NAME (ospf, type))
    {
      int ret;

      ret = route_map_apply (ROUTEMAP (ospf, type), (struct prefix *) p,
                             RMAP_OSPF, ei);

      if (ret == RMAP_DENYMATCH)
        {
          ei->route_map_set = save_values;
          if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
            zlog_debug ("Redistribute[%s]: %s/%d filtered by route-map.",
                       ospf_redist_string(type),
                       inet_ntoa (p->prefix), p->prefixlen);
          return 0;
        }

      /* check if 'route-map set' changed something */
      if (changed)
        *changed = !ospf_route_map_set_compare (&ei->route_map_set,
                                                &save_values);
    }

  return 1;
}

/* OSPF route-map set for redistribution */
void
ospf_routemap_set (struct ospf *ospf, int type, const char *name)
{
  if (ROUTEMAP_NAME (ospf, type))
    free (ROUTEMAP_NAME (ospf, type));

  ROUTEMAP_NAME (ospf, type) = strdup (name);
  ROUTEMAP (ospf, type) = route_map_lookup_by_name (name);
}

void
ospf_routemap_unset (struct ospf *ospf, int type)
{
  if (ROUTEMAP_NAME (ospf, type))
    free (ROUTEMAP_NAME (ospf, type));

  ROUTEMAP_NAME (ospf, type) = NULL;
  ROUTEMAP (ospf, type) = NULL;
}

int
ospf_distribute_list_out_set (struct ospf *ospf, int type, const char *name)
{
  /* Lookup access-list for distribute-list. */
  DISTRIBUTE_LIST (ospf, type) = access_list_lookup (AFI_IP, name);

  /* Clear previous distribute-name. */
  if (DISTRIBUTE_NAME (ospf, type))
    free (DISTRIBUTE_NAME (ospf, type));

  /* Set distribute-name. */
  DISTRIBUTE_NAME (ospf, type) = strdup (name);

  /* If access-list have been set, schedule update timer. */
  if (DISTRIBUTE_LIST (ospf, type))
    ospf_distribute_list_update (ospf, type);

  return (CMD_IPC_OK);
}

int
ospf_distribute_list_out_unset (struct ospf *ospf, int type, const char *name)
{
  /* Schedule update timer. */
  if (DISTRIBUTE_LIST (ospf, type))
    ospf_distribute_list_update (ospf, type);

  /* Unset distribute-list. */
  DISTRIBUTE_LIST (ospf, type) = NULL;

  /* Clear distribute-name. */
  if (DISTRIBUTE_NAME (ospf, type))
    free (DISTRIBUTE_NAME (ospf, type));

  DISTRIBUTE_NAME (ospf, type) = NULL;

  return (CMD_IPC_OK);
}

/* distribute-list update timer. */
void
ospfDistributeListUpdateTimer (Int32T fd, Int16T event, void * arg)
{
  struct route_node *rn;
  struct external_info *ei;
  struct route_table *rt;
  struct ospf_lsa *lsa;
  int type, default_refresh = 0;
  struct ospf *ospf;

  ospf = ospf_lookup ();
  if (ospf == NULL)
	  return;
  ospf->pTimerDistributeUpdate = NULL;

  zlog_info ("Zebra[Redistribute]: distribute-list update timer fired!");

  /* foreach all external info. */
  for (type = 0; type <= RIB_ROUTE_TYPE_MAX; type++)
    {
      rt = EXTERNAL_INFO (type);
      if (!rt)
	continue;
      for (rn = route_top (rt); rn; rn = route_next (rn))
	if ((ei = rn->info) != NULL)
	  {
	    if (is_prefix_default (&ei->p))
	      default_refresh = 1;
	    else if ((lsa = ospf_external_info_find_lsa (ospf, &ei->p)))
	      ospf_external_lsa_refresh (ospf, lsa, ei, LSA_REFRESH_IF_CHANGED);
	    else
	      ospf_external_lsa_originate (ospf, ei);
	  }
    }
  if (default_refresh)
    ospf_external_lsa_refresh_default (ospf);
  return;
}

#define OSPF_DISTRIBUTE_UPDATE_DELAY 5

/* Update distribute-list and set timer to apply access-list. */
void
ospf_distribute_list_update (struct ospf *ospf, uintptr_t type)
{
  struct route_table *rt;
  struct timeval tv = {0,0};

  /* External info does not exist. */
  if (!(rt = EXTERNAL_INFO (type)))
    return;

  /* If exists previously invoked thread, then let it continue. */
  if (ospf->pTimerDistributeUpdate)
    return;

  /* Set timer. */
  tv.tv_sec = OSPF_DISTRIBUTE_UPDATE_DELAY;
  ospf->pTimerDistributeUpdate =
		  taskTimerSet(ospfDistributeListUpdateTimer, tv, 0, (void *) type);
}

/* If access-list is updated, apply some check. */
void
ospf_filter_update (AccessListT *accessList)
{
  struct ospf *ospf;
  int type;
  int abr_inv = 0;
  struct ospf_area *area;
  struct listnode *node;

  /* If OSPF instatnce does not exist, return right now. */
  ospf = ospf_lookup ();
  if (ospf == NULL)
    return;

  /* Update distribute-list, and apply filter. */
  for (type = 0; type <= RIB_ROUTE_TYPE_MAX; type++)
    {
      if (ROUTEMAP (ospf, type) != NULL)
        {
          /* if route-map is not NULL it may be using this access list */
          ospf_distribute_list_update (ospf, type);
          continue;
        }

      /* There is place for route-map for default-information (RIB_ROUTE_TYPE_MAX),
       * but no distribute list. */
      if (type == RIB_ROUTE_TYPE_MAX)
	break;

      if (DISTRIBUTE_NAME (ospf, type))
        {
          /* Keep old access-list for distribute-list. */
          struct access_list *old = DISTRIBUTE_LIST (ospf, type);

          /* Update access-list for distribute-list. */
          DISTRIBUTE_LIST (ospf, type) =
            access_list_lookup (AFI_IP, DISTRIBUTE_NAME (ospf, type));

          /* No update for this distribute type. */
          if (old == NULL && DISTRIBUTE_LIST (ospf, type) == NULL)
            continue;

          /* Schedule distribute-list update timer. */
          if (DISTRIBUTE_LIST (ospf, type) == NULL ||
              strcmp (DISTRIBUTE_NAME (ospf, type), accessList->name) == 0)
            ospf_distribute_list_update (ospf, type);
        }
    }

  /* Update Area access-list. */
  for (ALL_LIST_ELEMENTS_RO (ospf->areas, node, area))
    {
      if (EXPORT_NAME (area))
        {
          EXPORT_LIST (area) = NULL;
          abr_inv++;
        }

      if (IMPORT_NAME (area))
        {
          IMPORT_LIST (area) = NULL;
          abr_inv++;
        }
    }

  /* Schedule ABR tasks -- this will be changed -- takada. */
  if (IS_OSPF_ABR (ospf) && abr_inv)
    ospf_schedule_abr_task (ospf);
}

/* If prefix-list is updated, do some updates. */
void
ospf_prefix_list_update (PrefixListT *plist)
{
  struct ospf *ospf;
  int type;
  int abr_inv = 0;
  struct ospf_area *area;
  struct listnode *node;

  /* If OSPF instatnce does not exist, return right now. */
  ospf = ospf_lookup ();
  if (ospf == NULL)
    return;

  /* Update all route-maps which are used as redistribution filters.
   * They might use prefix-list.
   */
  for (type = 0; type <= RIB_ROUTE_TYPE_MAX; type++)
    {
      if (ROUTEMAP (ospf, type) != NULL)
        {
          /* If route-map is not NULL it may be using this prefix list */
          ospf_distribute_list_update (ospf, type);
          continue;
        }
    }

  /* Update area filter-lists. */
  for (ALL_LIST_ELEMENTS_RO (ospf->areas, node, area))
    {
      /* Update filter-list in. */
      if (PREFIX_NAME_IN (area))
        if (strcmp (PREFIX_NAME_IN (area), plist->name) == 0)
          {
            PREFIX_LIST_IN (area) =
              prefix_list_lookup (AFI_IP, PREFIX_NAME_IN (area));
            abr_inv++;
          }

      /* Update filter-list out. */
      if (PREFIX_NAME_OUT (area))
        if (strcmp (PREFIX_NAME_OUT (area), plist->name) == 0)
          {
            PREFIX_LIST_IN (area) =
              prefix_list_lookup (AFI_IP, PREFIX_NAME_OUT (area));
            abr_inv++;
          }
    }

  /* Schedule ABR task. */
  if (IS_OSPF_ABR (ospf) && abr_inv)
    ospf_schedule_abr_task (ospf);
}

static struct ospf_distance *
ospf_distance_new (void)
{
  return XCALLOC (MTYPE_OSPF_DISTANCE, sizeof (struct ospf_distance));
}

static void
ospf_distance_free (struct ospf_distance *odistance)
{
  XFREE (MTYPE_OSPF_DISTANCE, odistance);
}

int
ospf_distance_set (struct cmsh *cmsh, struct ospf *ospf,
                   const char *distance_str,
                   const char *ip_str, 
                   const char *access_list_str)
{
  int ret;
  struct prefix_ipv4 p;
  u_char distance;
  struct route_node *rn;
  struct ospf_distance *odistance;

  ret = str2prefix_ipv4 (ip_str, &p);
  if (ret == 0)
    {
      cmdPrint (cmsh, "Malformed prefix\n");
      return CMD_WARNING;
    }

  distance = atoi (distance_str);

  /* Get OSPF distance node. */
  rn = route_node_get (ospf->distance_table, (struct prefix *) &p);
  if (rn->info)
    {
      odistance = rn->info;
      route_unlock_node (rn);
    }
  else
    {
      odistance = ospf_distance_new ();
      rn->info = odistance;
    }

  /* Set distance value. */
  odistance->distance = distance;

  /* Reset access-list configuration. */
  if (odistance->access_list)
    {
      free (odistance->access_list);
      odistance->access_list = NULL;
    }
  if (access_list_str)
    odistance->access_list = strdup (access_list_str);

  return (CMD_IPC_OK);
}

int
ospf_distance_unset (struct cmsh *cmsh, struct ospf *ospf,
                     const char *distance_str,
                     const char *ip_str, char 
                     const *access_list_str)
{
  int ret;
  struct prefix_ipv4 p;
  struct route_node *rn;
  struct ospf_distance *odistance;

  ret = str2prefix_ipv4 (ip_str, &p);
  if (ret == 0)
    {
      cmdPrint (cmsh, "Malformed prefix\n");
      return CMD_WARNING;
    }

  rn = route_node_lookup (ospf->distance_table, (struct prefix *) &p);
  if (!rn)
    {
      cmdPrint (cmsh, "Can't find specified prefix\n");
      return CMD_WARNING;
    }

  odistance = rn->info;

  if (odistance->access_list)
    free (odistance->access_list);
  ospf_distance_free (odistance);

  rn->info = NULL;
  route_unlock_node (rn);
  route_unlock_node (rn);

  return (CMD_IPC_OK);
}

void
ospf_distance_reset (struct ospf *ospf)
{
  struct route_node *rn;
  struct ospf_distance *odistance;

  for (rn = route_top (ospf->distance_table); rn; rn = route_next (rn))
    if ((odistance = rn->info) != NULL)
      {
        if (odistance->access_list)
          free (odistance->access_list);
        ospf_distance_free (odistance);
        rn->info = NULL;
        route_unlock_node (rn);
      }
}

u_char
ospf_distance_apply (struct prefix_ipv4 *p, struct ospf_route *or)
{
  struct ospf *ospf;

  ospf = ospf_lookup ();
  if (ospf == NULL)
    return 0;

  if (ospf->distance_intra)
    if (or->path_type == OSPF_PATH_INTRA_AREA)
      return ospf->distance_intra;

  if (ospf->distance_inter)
    if (or->path_type == OSPF_PATH_INTER_AREA)
      return ospf->distance_inter;

  if (ospf->distance_external)
    if (or->path_type == OSPF_PATH_TYPE1_EXTERNAL
        || or->path_type == OSPF_PATH_TYPE2_EXTERNAL)
      return ospf->distance_external;

  if (ospf->distance_all)
    return ospf->distance_all;

  return 0;
}

void
ospf_zebra_init ()
{

}
