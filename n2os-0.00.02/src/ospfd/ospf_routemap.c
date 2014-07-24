/*
 * Route map function of ospfd.
 * Copyright (C) 2000 IP Infusion Inc.
 *
 * Written by Toshiaki Takada.
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
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include "memory.h"
#include "prefix.h"
#include "table.h"
#include "routemap.h"
#include "command.h"
#include "log.h"
#include "plist.h"
#include "filter.h"

#include "nnFilter.h"
#include "nnVector.h"
#include "nnRoutemap.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"

#include "ospfd.h"
#include "ospf_asbr.h"
#include "ospf_interface.h"
#include "ospf_lsa.h"
#include "ospf_route.h"
#include "ospf_zebra.h"
#include "ospf_routemap.h"

/* Hook function for updating route_map assignment. */
static void
ospf_route_map_update (const char *name)
{
  struct ospf *ospf;
  int type;

  /* If OSPF instatnce does not exist, return right now. */
  ospf = ospf_lookup ();
  if (ospf == NULL)
    return;

  /* Update route-map */
  for (type = 0; type <= RIB_ROUTE_TYPE_MAX; type++)
    {
      if (ROUTEMAP_NAME (ospf, type)
	  && strcmp (ROUTEMAP_NAME (ospf, type), name) == 0)
	{
	  /* Keep old route-map. */
	  struct route_map *old = ROUTEMAP (ospf, type);

	  /* Update route-map. */
	  ROUTEMAP (ospf, type) =
	    route_map_lookup_by_name (ROUTEMAP_NAME (ospf, type));

	  /* No update for this distribute type. */
	  if (old == NULL && ROUTEMAP (ospf, type) == NULL)
	    continue;

	  ospf_distribute_list_update (ospf, type);
	}
    }
}

static void
ospf_route_map_event (eRouteMapEventT event, const char *name)
{
  struct ospf *ospf;
  int type;

  /* If OSPF instatnce does not exist, return right now. */
  ospf = ospf_lookup ();
  if (ospf == NULL)
    return;

  /* Update route-map. */
  for (type = 0; type <= RIB_ROUTE_TYPE_MAX; type++)
    {
      if (ROUTEMAP_NAME (ospf, type) &&  ROUTEMAP (ospf, type)
	  && !strcmp (ROUTEMAP_NAME (ospf, type), name))
        {
          ospf_distribute_list_update (ospf, type);
        }
    }
}

/* Delete rip route map rule. */
int
ospf_route_match_delete (struct cmsh *cmsh, struct route_map_index *idx,
			 const char *command, const StringT arg)
{
	int ret;

	ret = route_map_delete_match(idx, command, arg);
	if (ret) {
		switch (ret) {
		case RMAP_RULE_MISSING:
			cmdPrint(cmsh, "%% Can't find rule.\n");
			return (CMD_IPC_WARNING);
		case RMAP_COMPILE_ERROR:
			cmdPrint(cmsh, "%% Argument is malformed.\n");
			return (CMD_IPC_WARNING);
		}
	}

	return (CMD_IPC_OK);
}

struct route_map_index *
OspfGetRoutemap (struct cmsh *cmsh, const StringT arg1, const StringT arg2, const StringT arg3)
{
	int permit;
	unsigned long pref;
	struct route_map *map;
	struct route_map_index *idx = NULL;
	char *endptr = NULL;

	/* Permit check. */
	if (strncmp(arg2, "permit", strlen(arg2)) == 0)
		permit = RMAP_PERMIT;
	else if (strncmp(arg2, "deny", strlen(arg2)) == 0)
		permit = RMAP_DENY;
	else {
		cmdPrint(cmsh, "the third field must be [permit|deny]\n");
		return idx;
	}

	/* Preference check. */
	pref = strtoul(arg3, &endptr, 10);
	if (pref == ULONG_MAX || *endptr != '\0') {
		cmdPrint(cmsh, "the fourth field must be positive integer\n");
		return idx;
	}
	if (pref == 0 || pref > 65535) {
		cmdPrint(cmsh, "the fourth field must be <1-65535>\n");
		return idx;
	}

	/* Get route map. */
	map = route_map_get(arg1);
	idx = route_map_index_get(map, permit, pref);

	return idx;
}

int
ospf_route_match_add (struct cmsh *cmsh, struct route_map_index *idx,
		      const char *command, const StringT arg)
{
	int ret;

	ret = route_map_add_match(idx, command, arg);
	if (ret) {
		switch (ret) {
		case RMAP_RULE_MISSING:
			cmdPrint(cmsh, "%% Can't find rule.\n");
			return (CMD_IPC_WARNING);
		case RMAP_COMPILE_ERROR:
			cmdPrint(cmsh, "%% Argument is malformed.\n");
			return (CMD_IPC_WARNING);
		}
	}

	return (CMD_IPC_OK);
}

int
ospf_route_set_add (struct cmsh *cmsh, struct route_map_index *idx,
		    const char *command, const StringT arg)
{
	int ret;

	ret = route_map_add_set(idx, command, arg);
	if (ret) {
		switch (ret) {
		case RMAP_RULE_MISSING:
			cmdPrint(cmsh, "%% Can't find rule.\n");
			return (CMD_IPC_WARNING);
		case RMAP_COMPILE_ERROR:
			cmdPrint(cmsh, "%% Argument is malformed.\n");
			return (CMD_IPC_WARNING);
		}
	}

	return (CMD_IPC_OK);
}

/* Delete rip route map rule. */
int
ospf_route_set_delete (struct cmsh *cmsh, struct route_map_index *idx,
		       const char *command, const StringT arg)
{
	int ret;

	ret = route_map_delete_set(idx, command, arg);
	if (ret) {
		switch (ret) {
		case RMAP_RULE_MISSING:
			cmdPrint(cmsh, "%% Can't find rule.\n");
			return (CMD_IPC_WARNING);
		case RMAP_COMPILE_ERROR:
			cmdPrint(cmsh, "%% Argument is malformed.\n");
			return (CMD_IPC_WARNING);
		}
	}

	return (CMD_IPC_OK);
}

/* `match ip netxthop ' */
/* Match function return 1 if match is success else return zero. */
static eRouteMapResultT
route_match_ip_nexthop (void *rule, PrefixT *prefix, eRouteMapObjectT type, void *object)
{
  struct access_list *alist;
  struct external_info *ei = object;
  struct prefix_ipv4 p;

  if (type == RMAP_OSPF)
    {
      p.family = AF_INET;
      p.prefix = ei->nexthop;
      p.prefixlen = IPV4_MAX_BITLEN;

      alist = access_list_lookup (AFI_IP, (char *) rule);
      if (alist == NULL)
        return RMAP_NOMATCH;

      return (access_list_apply (alist, &p) == FILTER_DENY_ORG ?
              RMAP_NOMATCH : RMAP_MATCH);
    }
  return RMAP_NOMATCH;
}

/* Route map `ip next-hop' match statement. `arg' should be
   access-list name. */
static void *
route_match_ip_nexthop_compile (const StringT arg)
{
  return XSTRDUP (MTYPE_ROUTE_MAP_COMPILED, arg);
}

/* Free route map's compiled `ip address' value. */
static void
route_match_ip_nexthop_free (void *rule)
{
  XFREE (MTYPE_ROUTE_MAP_COMPILED, rule);
}

/* Route map commands for metric matching. */
RouteMapRuleCmdT route_match_ip_nexthop_cmd =
{
  "ip next-hop",
  route_match_ip_nexthop,
  route_match_ip_nexthop_compile,
  route_match_ip_nexthop_free
};

/* `match ip next-hop prefix-list PREFIX_LIST' */

static eRouteMapResultT
route_match_ip_next_hop_prefix_list (void *rule, PrefixT *prefix, eRouteMapObjectT type, void *object)
{
  struct prefix_list *plist;
  struct external_info *ei = object;
  struct prefix_ipv4 p;

  if (type == RMAP_OSPF)
    {
      p.family = AF_INET;
      p.prefix = ei->nexthop;
      p.prefixlen = IPV4_MAX_BITLEN;

      plist = prefix_list_lookup (AFI_IP, (char *) rule);
      if (plist == NULL)
        return RMAP_NOMATCH;

      return (prefix_list_apply (plist, &p) == PREFIX_DENY_ORG ?
              RMAP_NOMATCH : RMAP_MATCH);
    }
  return RMAP_NOMATCH;
}

static void *
route_match_ip_next_hop_prefix_list_compile (const StringT arg)
{
  return XSTRDUP (MTYPE_ROUTE_MAP_COMPILED, arg);
}

static void
route_match_ip_next_hop_prefix_list_free (void *rule)
{
  XFREE (MTYPE_ROUTE_MAP_COMPILED, rule);
}

RouteMapRuleCmdT route_match_ip_next_hop_prefix_list_cmd =
{
  "ip next-hop prefix-list",
  route_match_ip_next_hop_prefix_list,
  route_match_ip_next_hop_prefix_list_compile,
  route_match_ip_next_hop_prefix_list_free
};

/* `match ip address IP_ACCESS_LIST' */
/* Match function should return 1 if match is success else return
   zero. */
static eRouteMapResultT
route_match_ip_address (void *rule, PrefixT *prefix, eRouteMapObjectT type, void *object)
{
  struct access_list *alist;
  /* struct prefix_ipv4 match; */

  if (type == RMAP_OSPF)
    {
      alist = access_list_lookup (AFI_IP, (char *) rule);
      if (alist == NULL)
        return RMAP_NOMATCH;

      return (access_list_apply (alist, prefix) == FILTER_DENY_ORG ?
              RMAP_NOMATCH : RMAP_MATCH);
    }
  return RMAP_NOMATCH;
}

/* Route map `ip address' match statement.  `arg' should be
   access-list name. */
static void *
route_match_ip_address_compile (const StringT arg)
{
  return XSTRDUP (MTYPE_ROUTE_MAP_COMPILED, arg);
}

/* Free route map's compiled `ip address' value. */
static void
route_match_ip_address_free (void *rule)
{
  XFREE (MTYPE_ROUTE_MAP_COMPILED, rule);
}

/* Route map commands for ip address matching. */
RouteMapRuleCmdT route_match_ip_address_cmd =
{
  "ip address",
  route_match_ip_address,
  route_match_ip_address_compile,
  route_match_ip_address_free
};

/* `match ip address prefix-list PREFIX_LIST' */
static eRouteMapResultT
route_match_ip_address_prefix_list (void *rule, PrefixT *prefix, eRouteMapObjectT type, void *object)
{
  struct prefix_list *plist;

  if (type == RMAP_OSPF)
    {
      plist = prefix_list_lookup (AFI_IP, (char *) rule);
      if (plist == NULL)
        return RMAP_NOMATCH;

      return (prefix_list_apply (plist, prefix) == PREFIX_DENY_ORG ?
              RMAP_NOMATCH : RMAP_MATCH);
    }
  return RMAP_NOMATCH;
}

static void *
route_match_ip_address_prefix_list_compile (const StringT arg)
{
  return XSTRDUP (MTYPE_ROUTE_MAP_COMPILED, arg);
}

static void
route_match_ip_address_prefix_list_free (void *rule)
{
  XFREE (MTYPE_ROUTE_MAP_COMPILED, rule);
}

RouteMapRuleCmdT route_match_ip_address_prefix_list_cmd =
{
  "ip address prefix-list",
  route_match_ip_address_prefix_list,
  route_match_ip_address_prefix_list_compile,
  route_match_ip_address_prefix_list_free
};

/* `match interface IFNAME' */
/* Match function should return 1 if match is success else return
   zero. */
static eRouteMapResultT
route_match_interface (void *rule, PrefixT *prefix, eRouteMapObjectT type, void *object)
{
  struct interface *ifp;
  struct external_info *ei;

  if (type == RMAP_OSPF)
    {
      ei = object;
      ifp = if_lookup_by_name ((char *)rule);

      if (ifp == NULL || ifp->ifindex != ei->ifindex)
	return RMAP_NOMATCH;

      return RMAP_MATCH;
    }
  return RMAP_NOMATCH;
}

/* Route map `interface' match statement.  `arg' should be
   interface name. */
static void *
route_match_interface_compile (const StringT arg)
{
  return XSTRDUP (MTYPE_ROUTE_MAP_COMPILED, arg);
}

/* Free route map's compiled `interface' value. */
static void
route_match_interface_free (void *rule)
{
  XFREE (MTYPE_ROUTE_MAP_COMPILED, rule);
}

/* Route map commands for ip address matching. */
RouteMapRuleCmdT route_match_interface_cmd =
{
  "interface",
  route_match_interface,
  route_match_interface_compile,
  route_match_interface_free
};

/* `set metric METRIC' */
/* Set metric to attribute. */
static eRouteMapResultT
route_set_metric (void *rule, PrefixT *prefix, eRouteMapObjectT type, void *object)
{
  u_int32_t *metric;
  struct external_info *ei;

  if (type == RMAP_OSPF)
    {
      /* Fetch routemap's rule information. */
      metric = rule;
      ei = object;

      /* Set metric out value. */
      ei->route_map_set.metric = *metric;
    }
  return RMAP_OKAY;
}

/* set metric compilation. */
static void *
route_set_metric_compile (const StringT arg)
{
  u_int32_t *metric;
  int32_t ret;

  metric = XCALLOC (MTYPE_ROUTE_MAP_COMPILED, sizeof (u_int32_t));
  ret = atoi (arg);

  if (ret >= 0)
    {
      *metric = (u_int32_t)ret;
      return metric;
    }

  XFREE (MTYPE_ROUTE_MAP_COMPILED, metric);
  return NULL;
}

/* Free route map's compiled `set metric' value. */
static void
route_set_metric_free (void *rule)
{
  XFREE (MTYPE_ROUTE_MAP_COMPILED, rule);
}

/* Set metric rule structure. */
RouteMapRuleCmdT route_set_metric_cmd =
{
  "metric",
  route_set_metric,
  route_set_metric_compile,
  route_set_metric_free,
};

/* `set metric-type TYPE' */
/* Set metric-type to attribute. */
static eRouteMapResultT
route_set_metric_type (void *rule, PrefixT *prefix, eRouteMapObjectT type, void *object)
{
  u_int32_t *metricT;
  struct external_info *ei;

  if (type == RMAP_OSPF)
    {
      /* Fetch routemap's rule information. */
      metricT = rule;
      ei = object;

      /* Set metric out value. */
      ei->route_map_set.metric_type = *metricT;
    }
  return RMAP_OKAY;
}

/* set metric-type compilation. */
static void *
route_set_metric_type_compile (const StringT arg)
{
  u_int32_t *metricT;

  metricT = XCALLOC (MTYPE_ROUTE_MAP_COMPILED, sizeof (u_int32_t));
  if (strcmp (arg, "type-1") == 0)
    *metricT = EXTERNAL_METRIC_TYPE_1;
  else if (strcmp (arg, "type-2") == 0)
    *metricT = EXTERNAL_METRIC_TYPE_2;

  if (*metricT == EXTERNAL_METRIC_TYPE_1 ||
      *metricT == EXTERNAL_METRIC_TYPE_2)
    return metricT;

  XFREE (MTYPE_ROUTE_MAP_COMPILED, metricT);
  return NULL;
}

/* Free route map's compiled `set metric-type' value. */
static void
route_set_metric_type_free (void *rule)
{
  XFREE (MTYPE_ROUTE_MAP_COMPILED, rule);
}

/* Set metric rule structure. */
RouteMapRuleCmdT route_set_metric_type_cmd =
{
  "metric-type",
  route_set_metric_type,
  route_set_metric_type_compile,
  route_set_metric_type_free,
};



/* Route-map init */
void
ospf_route_map_init (void)
{
	/* Routemap vector init. */
	routeMapInit();

	/* Assign routemap vector pointer. */
	om->routeMatchVec = routeMapMatchVecGetPtr();
	om->routeSetVec = routeMapSetVecGetPtr();

	/* Assign hook functions. */
	routeMapAddHook(ospf_route_map_update);
	routeMapDeleteHook(ospf_route_map_update);
	routeMapEventHook(ospf_route_map_event);

	routeMapInitVty();

	routeMapInstallMatch(&route_match_ip_nexthop_cmd);
	routeMapInstallMatch(&route_match_ip_next_hop_prefix_list_cmd);
	routeMapInstallMatch(&route_match_ip_address_cmd);
	routeMapInstallMatch(&route_match_ip_address_prefix_list_cmd);
	routeMapInstallMatch(&route_match_interface_cmd);

	routeMapInstallSet(&route_set_metric_cmd);
	routeMapInstallSet(&route_set_metric_type_cmd);
}
