/*
 * IS-IS Rout(e)ing protocol - isis_zebra.c   
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


#include "command.h"
#include "memory.h"
#include "log.h"
#include "if.h"
#include "network.h"
#include "prefix.h"
#include "zclient.h"
#include "stream.h"
#include "linklist.h"

#include "nosLib.h"
#include "nnBuffer.h"
#include "nnRibDefines.h"

#include "dict.h"
#include "isis_constants.h"
#include "isis_common.h"
#include "isis_flags.h"
#include "isis_misc.h"
#include "isis_circuit.h"
#include "isis_tlv.h"
#include "isisd.h"
#include "isis_circuit.h"
#include "isis_csm.h"
#include "isis_lsp.h"
#include "isis_route.h"
#include "isis_zebra.h"
#include "isisUtil.h"


struct zclient *zclient = NULL;

/* Router-id update message from zebra. */
int
isisRouterIdUpdate (nnBufferT *msgBuff)
{
	struct isis_area *area;
	struct listnode *node;
	PrefixT p;

	p.family = nnBufferGetInt8T(msgBuff);
	p.prefixLen = nnBufferGetInt8T(msgBuff);

	if (p.family == AF_INET) {
		p.u.prefix4 = nnBufferGetInaddr(msgBuff);
	}
#ifdef HAVE_IPV6
	else if (p.family == AF_INET6)
	{
		p.u.prefix6 = nnBufferGetInaddr(msgBuff);
	}
#endif

	printf("############################\n");
	printf(" EVENT ROUTER ID\n");
	printf("############################\n");
	printf("\t family=%d, prefixlen=%d, prefix=%s\n", p.family, p.prefixLen,
			inet_ntoa(p.u.prefix4));
	printf("############################\n");

	if (isis->router_id == p.u.prefix4.s_addr)
		return 0;

	isis->router_id = p.u.prefix4.s_addr;
	for (ALL_LIST_ELEMENTS_RO(isis->area_list, node, area))
		if (listcount (area->area_addrs) > 0)
			lsp_regenerate_schedule(area, area->is_type, 0);

	return 0;
}

static void
isis_zebra_route_add_ipv4 (struct prefix *prefix,
			   struct isis_route_info *route_info)
{
	struct isis_nexthop *nexthop;
	struct listnode *node;

	if (CHECK_FLAG(route_info->flag, ISIS_ROUTE_FLAG_ZEBRA_SYNCED))
		return;

//		NNLOG(LOG_DEBUG, "\n RIP_RIBMANAGER_IPV4_ADD\n");
//		NNLOG(LOG_DEBUG, "family = %d\n", p->family);
//		NNLOG(LOG_DEBUG, "length = %d\n", p->prefixLen);
//		NNLOG(LOG_DEBUG, "prefix = %s\n", inet_ntoa(p->prefix));
//		NNLOG(LOG_DEBUG, "nexthop = %s\n", inet_ntoa(*pNextHop));
//		NNLOG(LOG_DEBUG, "metric = %d\n", metric);

	Uint16T position = 0;
	Uint8T rType = RIB_ROUTE_TYPE_ISIS;

	/* Buffer Reset */
	nnBufferT msgBuff;
	nnBufferReset(&msgBuff);

	/* Number of Route */
	nnBufferSetInt16T(&msgBuff, route_info->nexthops->count);

	/* route type */
	nnBufferSetInt8T(&msgBuff, rType);
	for (ALL_LIST_ELEMENTS_RO(route_info->nexthops, node, nexthop)) {
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
		messages |= RIB_MESSAGE_NEXTHOP;
		messages |= RIB_MESSAGE_METRIC;
		nnBufferSetInt8T(&msgBuff, messages);

		/* prefix length */
		nnBufferSetInt8T(&msgBuff, prefix->prefixlen);

		/* prefix  */
		nnBufferSetInaddr(&msgBuff, prefix->u.prefix4);

		/* Nexthop, ifindex, distance and metric information */
		/* FIXME: can it be ? */
		if (nexthop->ip.s_addr != INADDR_ANY) {
			Uint8T nhType = NEXTHOP_TYPE_IPV4;
			nnBufferSetInt8T(&msgBuff, nhType);
			nnBufferSetInaddr(&msgBuff, nexthop->ip);

		} else {
			Uint8T nhType = NEXTHOP_TYPE_IFINDEX;
			nnBufferSetInt8T(&msgBuff, nhType);
			nnBufferSetInt32T(&msgBuff, nexthop->ifindex);

		}

		/* Metric */
		nnBufferSetInt32T(&msgBuff, route_info->cost);

		/* Message Length */
		Uint16T subLength = nnBufferGetLength(&msgBuff) - position
				- sizeof(Uint16T);
		nnBufferInsertInt16T(&msgBuff, position, subLength);

	}

	nnBufferPrint(&msgBuff);

	/* send ipc message to ribmgr */
	ipcSendAsync(RIB_MANAGER, IPC_RIB_IPV4_ROUTE_ADD, msgBuff.data,
			msgBuff.length);

	SET_FLAG(route_info->flag, ISIS_ROUTE_FLAG_ZEBRA_SYNCED);
	UNSET_FLAG(route_info->flag, ISIS_ROUTE_FLAG_ZEBRA_RESYNC);
}

static void
isis_zebra_route_del_ipv4 (struct prefix *prefix,
			   struct isis_route_info *route_info)
{
	struct isis_nexthop *nexthop;
	struct listnode *node;

//		NNLOG(LOG_DEBUG, "\n RIP_RIBMANAGER_IPV4_ADD\n");
//		NNLOG(LOG_DEBUG, "family = %d\n", p->family);
//		NNLOG(LOG_DEBUG, "length = %d\n", p->prefixLen);
//		NNLOG(LOG_DEBUG, "prefix = %s\n", inet_ntoa(p->prefix));
//		NNLOG(LOG_DEBUG, "nexthop = %s\n", inet_ntoa(*pNextHop));
//		NNLOG(LOG_DEBUG, "metric = %d\n", metric);

	Uint16T position = 0;
	Uint8T rType = RIB_ROUTE_TYPE_ISIS;

	/* Buffer Reset */
	nnBufferT msgBuff;
	nnBufferReset(&msgBuff);

	/* Number of Route */
	nnBufferSetInt16T(&msgBuff, route_info->nexthops->count);

	/* route type */
	nnBufferSetInt8T(&msgBuff, rType);
	for (ALL_LIST_ELEMENTS_RO(route_info->nexthops, node, nexthop)) {
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
		messages |= RIB_MESSAGE_NEXTHOP;
		messages |= RIB_MESSAGE_METRIC;
		nnBufferSetInt8T(&msgBuff, messages);

		/* prefix length */
		nnBufferSetInt8T(&msgBuff, prefix->prefixlen);

		/* prefix  */
		nnBufferSetInaddr(&msgBuff, prefix->u.prefix4);

		/* Nexthop, ifindex, distance and metric information */
		/* FIXME: can it be ? */
		if (nexthop->ip.s_addr != INADDR_ANY) {
			Uint8T nhType = NEXTHOP_TYPE_IPV4;
			nnBufferSetInt8T(&msgBuff, nhType);
			nnBufferSetInaddr(&msgBuff, nexthop->ip);

		} else {
			Uint8T nhType = NEXTHOP_TYPE_IFINDEX;
			nnBufferSetInt8T(&msgBuff, nhType);
			nnBufferSetInt32T(&msgBuff, nexthop->ifindex);

		}


		/* Metric */
		nnBufferSetInt32T(&msgBuff, route_info->cost);

		/* Message Length */
		Uint16T subLength = nnBufferGetLength(&msgBuff) - position
				- sizeof(Uint16T);
		nnBufferInsertInt16T(&msgBuff, position, subLength);

	}








	nnBufferPrint(&msgBuff);

	/* send ipc message to ribmgr */
	ipcSendAsync(RIB_MANAGER, IPC_RIB_IPV4_ROUTE_ADD, msgBuff.data,
			msgBuff.length);

	SET_FLAG(route_info->flag, ISIS_ROUTE_FLAG_ZEBRA_SYNCED);
	UNSET_FLAG(route_info->flag, ISIS_ROUTE_FLAG_ZEBRA_RESYNC);
}

#ifdef HAVE_IPV6
void
isis_zebra_route_add_ipv6 (struct prefix *prefix,
			   struct isis_route_info *route_info)
{
  struct zapi_ipv6 api;
  struct in6_addr **nexthop_list;
  unsigned int *ifindex_list;
  struct isis_nexthop6 *nexthop6;
  int i, size;
  struct listnode *node;
  struct prefix_ipv6 prefix6;

  if (CHECK_FLAG (route_info->flag, ISIS_ROUTE_FLAG_ZEBRA_SYNCED))
    return;

  api.type = ZEBRA_ROUTE_ISIS;
  api.flags = 0;
  api.message = 0;
  api.safi = SAFI_UNICAST;
  SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
  SET_FLAG (api.message, ZAPI_MESSAGE_IFINDEX);
  SET_FLAG (api.message, ZAPI_MESSAGE_METRIC);
  api.metric = route_info->cost;
#if 0
  SET_FLAG (api.message, ZAPI_MESSAGE_DISTANCE);
  api.distance = route_info->depth;
#endif
  api.nexthop_num = listcount (route_info->nexthops6);
  api.ifindex_num = listcount (route_info->nexthops6);

  /* allocate memory for nexthop_list */
  size = sizeof (struct isis_nexthop6 *) * listcount (route_info->nexthops6);
  nexthop_list = (struct in6_addr **) XMALLOC (MTYPE_ISIS_TMP, size);
  if (!nexthop_list)
    {
      zlog_err ("isis_zebra_add_route_ipv6: out of memory!");
      return;
    }

  /* allocate memory for ifindex_list */
  size = sizeof (unsigned int) * listcount (route_info->nexthops6);
  ifindex_list = (unsigned int *) XMALLOC (MTYPE_ISIS_TMP, size);
  if (!ifindex_list)
    {
      zlog_err ("isis_zebra_add_route_ipv6: out of memory!");
      XFREE (MTYPE_ISIS_TMP, nexthop_list);
      return;
    }

  /* for each nexthop */
  i = 0;
  for (ALL_LIST_ELEMENTS_RO (route_info->nexthops6, node, nexthop6))
    {
      if (!IN6_IS_ADDR_LINKLOCAL (&nexthop6->ip6) &&
	  !IN6_IS_ADDR_UNSPECIFIED (&nexthop6->ip6))
	{
	  api.nexthop_num--;
	  api.ifindex_num--;
	  continue;
	}

      nexthop_list[i] = &nexthop6->ip6;
      ifindex_list[i] = nexthop6->ifindex;
      i++;
    }

  api.nexthop = nexthop_list;
  api.ifindex = ifindex_list;

  if (api.nexthop_num && api.ifindex_num)
    {
      prefix6.family = AF_INET6;
      prefix6.prefixlen = prefix->prefixlen;
      memcpy (&prefix6.prefix, &prefix->u.prefix6, sizeof (struct in6_addr));
      zapi_ipv6_route (ZEBRA_IPV6_ROUTE_ADD, zclient, &prefix6, &api);
      SET_FLAG (route_info->flag, ISIS_ROUTE_FLAG_ZEBRA_SYNCED);
      UNSET_FLAG (route_info->flag, ISIS_ROUTE_FLAG_ZEBRA_RESYNC);
    }

  XFREE (MTYPE_ISIS_TMP, nexthop_list);
  XFREE (MTYPE_ISIS_TMP, ifindex_list);

  return;
}

static void
isis_zebra_route_del_ipv6 (struct prefix *prefix,
			   struct isis_route_info *route_info)
{
  struct zapi_ipv6 api;
  struct in6_addr **nexthop_list;
  unsigned int *ifindex_list;
  struct isis_nexthop6 *nexthop6;
  int i, size;
  struct listnode *node;
  struct prefix_ipv6 prefix6;

  if (CHECK_FLAG (route_info->flag, ISIS_ROUTE_FLAG_ZEBRA_SYNCED))
    return;

  api.type = ZEBRA_ROUTE_ISIS;
  api.flags = 0;
  api.message = 0;
  api.safi = SAFI_UNICAST;
  SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
  SET_FLAG (api.message, ZAPI_MESSAGE_IFINDEX);
  api.nexthop_num = listcount (route_info->nexthops6);
  api.ifindex_num = listcount (route_info->nexthops6);

  /* allocate memory for nexthop_list */
  size = sizeof (struct isis_nexthop6 *) * listcount (route_info->nexthops6);
  nexthop_list = (struct in6_addr **) XMALLOC (MTYPE_ISIS_TMP, size);
  if (!nexthop_list)
    {
      zlog_err ("isis_zebra_route_del_ipv6: out of memory!");
      return;
    }

  /* allocate memory for ifindex_list */
  size = sizeof (unsigned int) * listcount (route_info->nexthops6);
  ifindex_list = (unsigned int *) XMALLOC (MTYPE_ISIS_TMP, size);
  if (!ifindex_list)
    {
      zlog_err ("isis_zebra_route_del_ipv6: out of memory!");
      XFREE (MTYPE_ISIS_TMP, nexthop_list);
      return;
    }

  /* for each nexthop */
  i = 0;
  for (ALL_LIST_ELEMENTS_RO (route_info->nexthops6, node, nexthop6))
    {
      if (!IN6_IS_ADDR_LINKLOCAL (&nexthop6->ip6) &&
	  !IN6_IS_ADDR_UNSPECIFIED (&nexthop6->ip6))
	{
	  api.nexthop_num--;
	  api.ifindex_num--;
	  continue;
	}

      nexthop_list[i] = &nexthop6->ip6;
      ifindex_list[i] = nexthop6->ifindex;
      i++;
    }

  api.nexthop = nexthop_list;
  api.ifindex = ifindex_list;

  if (api.nexthop_num && api.ifindex_num)
    {
      prefix6.family = AF_INET6;
      prefix6.prefixlen = prefix->prefixlen;
      memcpy (&prefix6.prefix, &prefix->u.prefix6, sizeof (struct in6_addr));
      zapi_ipv6_route (ZEBRA_IPV6_ROUTE_DELETE, zclient, &prefix6, &api);
      UNSET_FLAG (route_info->flag, ISIS_ROUTE_FLAG_ZEBRA_SYNCED);
    }

  XFREE (MTYPE_ISIS_TMP, nexthop_list);
  XFREE (MTYPE_ISIS_TMP, ifindex_list);
}

#endif /* HAVE_IPV6 */

void
isis_zebra_route_update (struct prefix *prefix,
			 struct isis_route_info *route_info)
{
	if (!isis->isisRedistribute[RIB_ROUTE_TYPE_ISIS])
		return;

	if (CHECK_FLAG(route_info->flag, ISIS_ROUTE_FLAG_ACTIVE)) {
		if (prefix->family == AF_INET)
			isis_zebra_route_add_ipv4(prefix, route_info);
#ifdef HAVE_IPV6
		else if (prefix->family == AF_INET6)
		isis_zebra_route_add_ipv6 (prefix, route_info);
#endif /* HAVE_IPV6 */
	} else {
		if (prefix->family == AF_INET)
			isis_zebra_route_del_ipv4(prefix, route_info);
#ifdef HAVE_IPV6
		else if (prefix->family == AF_INET6)
		isis_zebra_route_del_ipv6 (prefix, route_info);
#endif /* HAVE_IPV6 */
	}
	return;
}



#ifdef HAVE_IPV6
static int
isis_zebra_read_ipv6 (int command, struct zclient *zclient,
		      zebra_size_t length)
{
  return 0;
}
#endif

#define ISIS_TYPE_IS_REDISTRIBUTED(T) \
T == RIB_ROUTE_TYPE_MAX ? isis->isisDefaultInformation : isis->isisRedistribute[type]

int
isis_distribute_list_update (int routetype)
{
  return 0;
}

#if 0 /* Not yet. */
static int
isis_redistribute_default_set (int routetype, int metric_type,
			       int metric_value)
{
  return 0;
}
#endif /* 0 */

