/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : RIP Protocol의 Routemap설정을 제어하는 화일
 *
 * - Block Name : RIP Protocol
 * - Process Name : ripd
 * - Creator : Geontae Park
 * - Initial Date : 2014/02/20
 */

/**
 * @file        : ripRoutemap.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include <limits.h>

#include "nnRibDefines.h"
#include "nnVector.h"
#include "nnFilter.h"
#include "nnPlist.h"

#include "nnTypes.h"
#include "nnStr.h"
#include "nnRoutemap.h"
#include "nnDefines.h"
#include "nnPrefix.h"
#include "nnIf.h"
#include "nosLib.h"

#include "ripd.h"

struct ripMetricModifier
{
  enum 
  {
    metricIncrement,
    metricDecrement,
    metricAbsolute
  } type;

  Uint8T metric;
};
typedef struct ripMetricModifier RipMetricModifierT;

#if 0
/* Add rip route map rule. */
static Int32T
ripRouteMatchAdd (struct vty *vty, RouteMapIndexT *index,
		     const char *command, const char *arg)
{
  Int32T ret;

  ret = routeMapAddMatch (index, command, arg);
  if (ret)
    {
      switch (ret)
	{
	case RMAP_RULE_MISSING:
	  vty_out (vty, "%% Can't find rule.%s", VTY_NEWLINE);
	  return CMD_WARNING;
	case RMAP_COMPILE_ERROR:
	  vty_out (vty, "%% Argument is malformed.%s", VTY_NEWLINE);
	  return CMD_WARNING;
	}
    }

  return CMD_SUCCESS;
}
#endif

#if 0
/* Delete rip route map rule. */
static Int32T
ripRouteMatchDelete (struct vty *vty, RouteMapIndexT *index,
			const char *command, const char *arg)
{
  Int32T ret;

  ret = routeMapDeleteMatch (index, command, arg);
  if (ret)
    {
      switch (ret)
	{
	case RMAP_RULE_MISSING:
	  vty_out (vty, "%% Can't find rule.%s", VTY_NEWLINE);
	  return CMD_WARNING;
	case RMAP_COMPILE_ERROR:
	  vty_out (vty, "%% Argument is malformed.%s", VTY_NEWLINE);
	  return CMD_WARNING;
	}
    }
  return CMD_SUCCESS;
}
#endif

#if 0
/* Add rip route map rule. */
static Int32T
ripRouteSetAdd (struct vty *vty, RouteMapIndexT *index,
		   const char *command, const char *arg)
{
  Int32T ret;

  ret = routeMapAddSet (index, command, arg);
  if (ret)
    {
      switch (ret)
	{
	case RMAP_RULE_MISSING:
	  vty_out (vty, "%% Can't find rule.%s", VTY_NEWLINE);
	  return CMD_WARNING;
	case RMAP_COMPILE_ERROR:
	  /* rip, ripng and other protocols share the set metric command
	     but only values from 0 to 16 are valid for rip and ripng
	     if metric is out of range for rip and ripng, it is not for
	     other protocols. Do not return an error */
	  if (strcmp(command, "metric")) {
	     vty_out (vty, "%% Argument is malformed.%s", VTY_NEWLINE);
	     return CMD_WARNING;
	  }
	}
    }
  return CMD_SUCCESS;
}
#endif

#if 0
/* Delete rip route map rule. */
static Int32T
ripRouteSetDelete (struct vty *vty, RouteMapIndexT *index,
		      const char *command, const char *arg)
{
  Int32T ret;

  ret = routeMapDeleteSet (index, command, arg);
  if (ret)
    {
      switch (ret)
	{
	case RMAP_RULE_MISSING:
	  vty_out (vty, "%% Can't find rule.%s", VTY_NEWLINE);
	  return CMD_WARNING;
	case RMAP_COMPILE_ERROR:
	  vty_out (vty, "%% Argument is malformed.%s", VTY_NEWLINE);
	  return CMD_WARNING;
	}
    }
  return CMD_SUCCESS;
}
#endif

/* Hook function for updating routeMap assignment. */
/* ARGSUSED */
static void
ripRouteMapUpdate (const char *notused)
{
  Int32T i = 0;

  if (pRip) 
  {
    for (i = 0; i < RIB_ROUTE_TYPE_MAX; i++) 
    {
      if (pRip->routeMap[i].name)
        pRip->routeMap[i].map = 
             routeMapLookupByName (pRip->routeMap[i].name);
    }
  }
}

/* `match metric METRIC' */
/* Match function return 1 if match is success else return zero. */
static eRouteMapResultT
routeMatchMetric (void *rule, PrefixT * pPrefix, 
                  eRouteMapObjectT type, void *object)
{
  Uint32T *metric = NULL;
  Uint32T  check;
  RipInfoT *pRInfo = NULL;

  if (type == RMAP_RIP)
  {
    metric = rule;
    pRInfo = object;
    
    /* If external metric is available, the route-map should
       work on this one (for redistribute purpose)  */
    check = (pRInfo->externalMetric) ? pRInfo->externalMetric :
                                       pRInfo->metric;
    if (check == *metric)
    {
      return RMAP_MATCH;
    }
    else
    {
      return RMAP_NOMATCH;
    }
  }

  return RMAP_NOMATCH;
}

/* Route map `match metric' match statement. `arg' is METRIC value */
static void *
routeMatchMetricCompile (const StringT arg)
{
  Uint32T *metric;

  metric = NNMALLOC (MEM_ROUTEMAP_COMPILED, sizeof (Uint32T));
  *metric = atoi (arg);

  if(*metric > 0)
  {
    return metric;
  }

  NNFREE (MEM_ROUTEMAP_COMPILED, metric);

  return NULL;
}

/* Free route map's compiled `match metric' value. */
static void
routeMatchMetricFree (void *rule)
{
  NNFREE (MEM_ROUTEMAP_COMPILED, rule);
}

/* Route map commands for metric matching. */
RouteMapRuleCmdT route_match_metric_cmd =
{
  "metric",
  routeMatchMetric,
  routeMatchMetricCompile,
  routeMatchMetricFree
};

/* `match interface IFNAME' */
/* Match function return 1 if match is success else return zero. */
static eRouteMapResultT
routeMatchInterface (void *rule, PrefixT * pPrefix,
                     eRouteMapObjectT type, void *object)
{
  RipInfoT * pRInfo = NULL;
  InterfaceT *pIf = NULL;
  char *ifName;

  if (type == RMAP_RIP)
  {
    ifName = rule;
    pIf = ifLookupByName(ifName);

    if (!pIf)
    {
      return RMAP_NOMATCH;
    }

    pRInfo = object;

    if (pRInfo->ifIndexOut == pIf->ifIndex || pRInfo->ifIndex == pIf->ifIndex)
    {
      return RMAP_MATCH;
    }
    else
    {
      return RMAP_NOMATCH;
    }
  }

  return RMAP_NOMATCH;
}

/* Route map `match interface' match statement. `arg' is IFNAME value */
/* XXX I don`t know if I need to check does interface exist? */
static void *
routeMatchInterfaceCompile (const StringT arg)
{
  return nnStrDup (arg, MEM_ROUTEMAP_COMPILED);
}

/* Free route map's compiled `match interface' value. */
static void
routeMatchInterfaceFree (void *rule)
{
  NNFREE (MEM_ROUTEMAP_COMPILED, rule);
}

/* Route map commands for interface matching. */
RouteMapRuleCmdT route_match_interface_cmd =
{
  "interface",
  routeMatchInterface,
  routeMatchInterfaceCompile,
  routeMatchInterfaceFree
};

/* `match ip next-hop IP_ACCESS_LIST' */

/* Match function return 1 if match is success else return zero. */
static eRouteMapResultT
routeMatchIpNextHop (void * rule, PrefixT * pPrefix,
                     eRouteMapObjectT type, void * object)
{
  AccessListT * alist = NULL;
  RipInfoT * pRInfo = NULL;
  Prefix4T p = {0,};

  if (type == RMAP_RIP)
  {
    pRInfo = object;
    p.family = AF_INET;
    p.prefix = (pRInfo->nexthop.s_addr) ? pRInfo->nexthop : pRInfo->from;
    p.prefixLen = NN_IPV4_MAX_BITLEN;

    alist = accessListLookup (AFI_IP, (char *) rule);
    if (alist == NULL)
    {
      return RMAP_NOMATCH;
    }

    return (accessListApply (alist, &p) == FILTER_DENY ?
                                             RMAP_NOMATCH : RMAP_MATCH);
  }

  return RMAP_NOMATCH;
}

/* Route map `ip next-hop' match statement.  `arg' should be
   access-list name. */
static void *
routeMatchIpNextHopCompile (const StringT arg)
{
  return nnStrDup (arg, MEM_ROUTEMAP_COMPILED);
}

/* Free route map's compiled `. */
static void
routeMatchIpNextHopFree (void *rule)
{
  NNFREE (MEM_ROUTEMAP_COMPILED, rule);
}

/* Route map commands for ip next-hop matching. */
#if 0
static RouteMapRuleCmdT route_match_ip_next_hop_cmd =
{
  "ip next-hop",
  routeMatchIpNextHop,
  routeMatchIpNextHopCompile,
  routeMatchIpNextHopFree
};
#endif

/* `match ip next-hop prefix-list PREFIX_LIST' */

static eRouteMapResultT
routeMatchIpNextHopPrefixList (void * rule, PrefixT * pPrefix,
                               eRouteMapObjectT type, void * object)
{
  PrefixListT * plist = NULL;
  RipInfoT * pRInfo = NULL;
  Prefix4T p = {0,};

  if (type == RMAP_RIP)
  {
    pRInfo = object;
    p.family = AF_INET;
    p.prefix = (pRInfo->nexthop.s_addr) ? pRInfo->nexthop : pRInfo->from;
    p.prefixLen = NN_IPV4_MAX_BITLEN;

    plist = prefixListLookup (AFI_IP, (char *) rule);
    if (plist == NULL)
    {
      return RMAP_NOMATCH;
    }

    return (prefixListApply (plist, &p) == PREFIX_DENY ?
                                           RMAP_NOMATCH : RMAP_MATCH);
  }

  return RMAP_NOMATCH;
}

static void *
routeMatchIpNextHopPrefixListCompile (const StringT arg)
{
  return nnStrDup (arg, MEM_ROUTEMAP_COMPILED);
}

static void
routeMatchIpNextHopPrefixListFree (void * rule)
{
  NNFREE (MEM_ROUTEMAP_COMPILED, rule);
}

#if 0
static RouteMapRuleCmdT route_match_ip_next_hop_prefix_list_cmd =
{
  "ip next-hop prefix-list",
  routeMatchIpNextHopPrefixList,
  routeMatchIpNextHopPrefixListCompile,
  routeMatchIpNextHopPrefixListFree
};
#endif

/* `match ip address IP_ACCESS_LIST' */

/* Match function should return 1 if match is success else return
   zero. */
static eRouteMapResultT
routeMatchIpAddress (void * rule, PrefixT * pPrefix, 
			         eRouteMapObjectT type, void * object)
{
  AccessListT * alist = NULL;

  if (type == RMAP_RIP)
  {
    alist = accessListLookup (AFI_IP, (char *) rule);
    if (alist == NULL)
    {
      return RMAP_NOMATCH;
    }
    
    return (accessListApply (alist, pPrefix) == FILTER_DENY ?
                                                RMAP_NOMATCH : RMAP_MATCH);
  }

  return RMAP_NOMATCH;
}

/* Route map `ip address' match statement.  `arg' should be
   access-list name. */
static void *
routeMatchIpAddressCompile (const StringT arg)
{
  return nnStrDup (arg, MEM_ROUTEMAP_COMPILED);
}

/* Free route map's compiled `ip address' value. */
static void
routeMatchIpAddressFree (void * rule)
{
  NNFREE (MEM_ROUTEMAP_COMPILED, rule);
}

/* Route map commands for ip address matching. */
#if 0
static RouteMapRuleCmdT route_match_ip_address_cmd =
{
  "ip address",
  routeMatchIpAddress,
  routeMatchIpAddressCompile,
  routeMatchIpAddressFree
};
#endif


/* `match ip address prefix-list PREFIX_LIST' */
static eRouteMapResultT
routeMatchIpAddressPrefixList (void * rule, PrefixT * pPrefix, 
                               eRouteMapObjectT type, void * object)
{
  PrefixListT * plist = NULL;

  if (type == RMAP_RIP)
  {
    plist = prefixListLookup (AFI_IP, (char *) rule);
    if (plist == NULL)
    {
      return RMAP_NOMATCH;
    }
    return (prefixListApply (plist, pPrefix) == PREFIX_DENY ?
                                                RMAP_NOMATCH : RMAP_MATCH);
  }

  return RMAP_NOMATCH;
}

static void *
routeMatchIpAddressPrefixListCompile (const StringT arg)
{
  return nnStrDup (arg, MEM_ROUTEMAP_COMPILED);
}

static void
routeMatchIpAddressPrefixListFree (void * rule)
{
  NNFREE (MEM_ROUTEMAP_COMPILED, rule);
}

#if 0
static RouteMapRuleCmdT route_match_ip_address_prefix_list_cmd =
{
  "ip address prefix-list",
  routeMatchIpAddressPrefixList,
  routeMatchIpAddressPrefixListCompile,
  routeMatchIpAddressPrefixListFree
};
#endif

/* `match tag TAG' */
/* Match function return 1 if match is success else return zero. */
static eRouteMapResultT
routeMatchTag (void * rule, PrefixT * pPrefix, 
               eRouteMapObjectT type, void * object)
{
  Uint16T * tag = NULL;
  RipInfoT * pRInfo = NULL;

  if (type == RMAP_RIP)
  {
    tag = rule;
    pRInfo = object;

    /* The information stored by pRInfo is host ordered. */
    if (pRInfo->tag == *tag)
    {
      return RMAP_MATCH;
    }
    else
    {
      return RMAP_NOMATCH;
    }
  }

  return RMAP_NOMATCH;
}

/* Route map `match tag' match statement. `arg' is TAG value */
static void *
routeMatchTagCompile (const StringT arg)
{
  Uint16T *tag = NULL;

  tag = NNMALLOC (MEM_ROUTEMAP_COMPILED, sizeof (Uint16T));
  *tag = atoi (arg);

  return tag;
}

/* Free route map's compiled `match tag' value. */
static void
routeMatchTagFree (void *rule)
{
  NNFREE (MEM_ROUTEMAP_COMPILED, rule);
}

/* Route map commands for tag matching. */
RouteMapRuleCmdT route_match_tag_cmd =
{
  "tag",
  routeMatchTag,
  routeMatchTagCompile,
  routeMatchTagFree
};

/* `set metric METRIC' */

/* Set metric to attribute. */
static eRouteMapResultT
routeSetMetric (void * rule, PrefixT * pPrefix, 
                eRouteMapObjectT type, void * object)
{
  if (type == RMAP_RIP)
  {
    RipMetricModifierT * mod = NULL;
    RipInfoT * pRInfo = NULL;

    mod = rule;
    pRInfo = object;

    if (mod->type == metricIncrement)
      pRInfo->metricOut += mod->metric;
    else if (mod->type == metricDecrement)
      pRInfo->metricOut -= mod->metric;
    else if (mod->type == metricAbsolute)
      pRInfo->metricOut = mod->metric;

    if ((Int32T)pRInfo->metricOut < 1)
      pRInfo->metricOut = 1;
    if (pRInfo->metricOut > RIP_METRIC_INFINITY)
      pRInfo->metricOut = RIP_METRIC_INFINITY;

    pRInfo->metricSet = 1;
  }

  return RMAP_OKAY;
}

/* set metric compilation. */
static void *
routeSetMetricCompile (const char *arg)
{
  Int32T len = 0;
  Int32T type = 0;
  Int32T metric = 0;
  const char *pnt = NULL;
  char *endptr = NULL;
  RipMetricModifierT *mod = NULL;

  len = strlen (arg);
  pnt = arg;

  if (len == 0)
    return NULL;

  /* Examine first character. */
  if (arg[0] == '+')
  {
    type = metricIncrement;
    pnt++;
  }
  else if (arg[0] == '-')
  {
    type = metricDecrement;
    pnt++;
  }
  else
    type = metricAbsolute;

  /* Check beginning with digit string. */
  if (*pnt < '0' || *pnt > '9')
    return NULL;

  /* Convert string to integer. */
  metric = strtol (pnt, &endptr, 10);

  if (metric == LONG_MAX || *endptr != '\0')
    return NULL;
  if (metric < 0 || metric > RIP_METRIC_INFINITY)
    return NULL;

  mod = NNMALLOC (MEM_ROUTEMAP_COMPILED, sizeof (RipMetricModifierT));
  mod->type = type;
  mod->metric = metric;

  return mod;
}

/* Free route map's compiled `set metric' value. */
static void
routeSetMetricFree (void *rule)
{
  NNFREE (MEM_ROUTEMAP_COMPILED, rule);
}

/* Set metric rule structure. */
#if 0
static RouteMapRuleCmdT route_set_metric_cmd = 
{
  "metric",
  routeSetMetric,
  routeSetMetricCompile,
  routeSetMetricFree,
};
#endif

/* `set ip next-hop IP_ADDRESS' */

/* Set nexthop to object.  ojbect must be pointer to struct attr. */
static eRouteMapResultT
routeSetIpNexthop (void * rule, PrefixT * pPrefix, 
                   eRouteMapObjectT type, void *object)
{
  struct in_addr * pAddress = NULL;
  RipInfoT * pRInfo = NULL;

  if(type == RMAP_RIP)
  {
    /* Fetch routemap's rule information. */
    pAddress = rule;
    pRInfo = object;

    /* Set next hop value. */ 
    pRInfo->nexthopOut = *pAddress;
  }

  return RMAP_OKAY;
}

/* Route map `ip nexthop' compile function.  Given string is converted
   to struct in_addr structure. */
static void *
routeSetIpNexthopCompile (const char *arg)
{
  Int32T ret = 0;
  struct in_addr * pAddress = NULL;

  pAddress = NNMALLOC (MEM_ROUTEMAP_COMPILED, sizeof (struct in_addr));

  ret = inet_aton (arg, pAddress);

  if (ret == 0)
  {
    NNFREE (MEM_ROUTEMAP_COMPILED, pAddress);
    return NULL;
  }

  return pAddress;
}

/* Free route map's compiled `ip nexthop' value. */
static void
routeSetIpNexthopFree (void *rule)
{
  NNFREE (MEM_ROUTEMAP_COMPILED, rule);
}

/* Route map commands for ip nexthop set. */
#if 0
static RouteMapRuleCmdT route_set_ip_nexthop_cmd =
{
  "ip next-hop",
  routeSetIpNexthop,
  routeSetIpNexthopCompile,
  routeSetIpNexthopFree
};
#endif

/* `set tag TAG' */

/* Set tag to object.  ojbect must be pointer to struct attr. */
static eRouteMapResultT
routeSetTag (void * rule, PrefixT * pPrefix, 
             eRouteMapObjectT type, void * object)
{
  Uint16T * tag = NULL;
  RipInfoT * pRInfo = NULL;

  if(type == RMAP_RIP)
  {
    /* Fetch routemap's rule information. */
    tag = rule;
    pRInfo = object;
    
    /* Set next hop value. */ 
    pRInfo->tagOut = *tag;
  }

  return RMAP_OKAY;
}

/* Route map `tag' compile function.  Given string is converted
   to Uint16T. */
static void *
routeSetTagCompile (const char *arg)
{
  Uint16T *tag;

  tag = NNMALLOC (MEM_ROUTEMAP_COMPILED, sizeof (Uint16T));
  *tag = atoi (arg);

  return tag;
}

/* Free route map's compiled `ip nexthop' value. */
static void
routeSetTagFree (void *rule)
{
  NNFREE (MEM_ROUTEMAP_COMPILED, rule);
}

/* Route map commands for tag set. */
#if 0
static RouteMapRuleCmdT route_set_tag_cmd =
{
  "tag",
  routeSetTag,
  routeSetTagCompile,
  routeSetTagFree
};
#endif

#define MATCH_STR "Match values from routing table\n"
#define SET_STR "Set values in destination routing protocol\n"

#if 0
DEFUN (match_metric, 
       match_metric_cmd,
       "match metric <0-4294967295>",
       MATCH_STR
       "Match metric of route\n"
       "Metric value\n")
{
  return ripRouteMatchAdd (vty, vty->index, "metric", argv[0]);
}

DEFUN (no_match_metric,
       no_match_metric_cmd,
       "no match metric",
       NO_STR
       MATCH_STR
       "Match metric of route\n")
{
  if (argc == 0)
    return ripRouteMatchDelete (vty, vty->index, "metric", NULL);

  return ripRouteMatchDelete (vty, vty->index, "metric", argv[0]);
}

ALIAS (no_match_metric,
       no_match_metric_val_cmd,
       "no match metric <0-4294967295>",
       NO_STR
       MATCH_STR
       "Match metric of route\n"
       "Metric value\n")

DEFUN (match_interface,
       match_interface_cmd,
       "match interface WORD",
       MATCH_STR
       "Match first hop interface of route\n"
       "Interface name\n")
{
  return ripRouteMatchAdd (vty, vty->index, "interface", argv[0]);
}

DEFUN (no_match_interface,
       no_match_interface_cmd,
       "no match interface",
       NO_STR
       MATCH_STR
       "Match first hop interface of route\n")
{
  if (argc == 0)
    return ripRouteMatchDelete (vty, vty->index, "interface", NULL);

  return ripRouteMatchDelete (vty, vty->index, "interface", argv[0]);
}

ALIAS (no_match_interface,
       no_match_interface_val_cmd,
       "no match interface WORD",
       NO_STR
       MATCH_STR
       "Match first hop interface of route\n"
       "Interface name\n")

DEFUN (match_ip_next_hop,
       match_ip_next_hop_cmd,
       "match ip next-hop (<1-199>|<1300-2699>|WORD)",
       MATCH_STR
       IP_STR
       "Match next-hop address of route\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP Access-list name\n")
{
  return ripRouteMatchAdd (vty, vty->index, "ip next-hop", argv[0]);
}

DEFUN (no_match_ip_next_hop,
       no_match_ip_next_hop_cmd,
       "no match ip next-hop",
       NO_STR
       MATCH_STR
       IP_STR
       "Match next-hop address of route\n")
{
  if (argc == 0)
    return ripRouteMatchDelete (vty, vty->index, "ip next-hop", NULL);

  return ripRouteMatchDelete (vty, vty->index, "ip next-hop", argv[0]);
}

ALIAS (no_match_ip_next_hop,
       no_match_ip_next_hop_val_cmd,
       "no match ip next-hop (<1-199>|<1300-2699>|WORD)",
       NO_STR
       MATCH_STR
       IP_STR
       "Match next-hop address of route\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP Access-list name\n")

DEFUN (match_ip_next_hop_prefix_list,
       match_ip_next_hop_prefix_list_cmd,
       "match ip next-hop prefix-list WORD",
       MATCH_STR
       IP_STR
       "Match next-hop address of route\n"
       "Match entries of prefix-lists\n"
       "IP prefix-list name\n")
{
  return ripRouteMatchAdd (vty, vty->index, "ip next-hop prefix-list", argv[0]);
}

DEFUN (no_match_ip_next_hop_prefix_list,
       no_match_ip_next_hop_prefix_list_cmd,
       "no match ip next-hop prefix-list",
       NO_STR
       MATCH_STR
       IP_STR
       "Match next-hop address of route\n"
       "Match entries of prefix-lists\n")
{
  if (argc == 0)
    return ripRouteMatchDelete (vty, vty->index, "ip next-hop prefix-list", NULL);

  return ripRouteMatchDelete (vty, vty->index, "ip next-hop prefix-list", argv[0]);
}

ALIAS (no_match_ip_next_hop_prefix_list,
       no_match_ip_next_hop_prefix_list_val_cmd,
       "no match ip next-hop prefix-list WORD",
       NO_STR
       MATCH_STR
       IP_STR
       "Match next-hop address of route\n"
       "Match entries of prefix-lists\n"
       "IP prefix-list name\n")

DEFUN (match_ip_address,
       match_ip_address_cmd,
       "match ip address (<1-199>|<1300-2699>|WORD)",
       MATCH_STR
       IP_STR
       "Match address of route\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP Access-list name\n")

{
  return ripRouteMatchAdd (vty, vty->index, "ip address", argv[0]);
}

DEFUN (no_match_ip_address, 
       no_match_ip_address_cmd,
       "no match ip address",
       NO_STR
       MATCH_STR
       IP_STR
       "Match address of route\n")
{
  if (argc == 0)
    return ripRouteMatchDelete (vty, vty->index, "ip address", NULL);

  return ripRouteMatchDelete (vty, vty->index, "ip address", argv[0]);
}

ALIAS (no_match_ip_address,
       no_match_ip_address_val_cmd,
       "no match ip address (<1-199>|<1300-2699>|WORD)",
       NO_STR
       MATCH_STR
       IP_STR
       "Match address of route\n"
       "IP access-list number\n"
       "IP access-list number (expanded range)\n"
       "IP Access-list name\n")

DEFUN (match_ip_address_prefix_list, 
       match_ip_address_prefix_list_cmd,
       "match ip address prefix-list WORD",
       MATCH_STR
       IP_STR
       "Match address of route\n"
       "Match entries of prefix-lists\n"
       "IP prefix-list name\n")
{
  return ripRouteMatchAdd (vty, vty->index, "ip address prefix-list", argv[0]);
}

DEFUN (no_match_ip_address_prefix_list,
       no_match_ip_address_prefix_list_cmd,
       "no match ip address prefix-list",
       NO_STR
       MATCH_STR
       IP_STR
       "Match address of route\n"
       "Match entries of prefix-lists\n")
{
  if (argc == 0)
    return ripRouteMatchDelete (vty, vty->index, "ip address prefix-list", NULL);

  return ripRouteMatchDelete (vty, vty->index, "ip address prefix-list", argv[0]);
}

ALIAS (no_match_ip_address_prefix_list,
       no_match_ip_address_prefix_list_val_cmd,
       "no match ip address prefix-list WORD",
       NO_STR
       MATCH_STR
       IP_STR
       "Match address of route\n"
       "Match entries of prefix-lists\n"
       "IP prefix-list name\n")

DEFUN (match_tag, 
       match_tag_cmd,
       "match tag <0-65535>",
       MATCH_STR
       "Match tag of route\n"
       "Metric value\n")
{
  return ripRouteMatchAdd (vty, vty->index, "tag", argv[0]);
}

DEFUN (no_match_tag,
       no_match_tag_cmd,
       "no match tag",
       NO_STR
       MATCH_STR
       "Match tag of route\n")
{
  if (argc == 0)
    return ripRouteMatchDelete (vty, vty->index, "tag", NULL);

  return ripRouteMatchDelete (vty, vty->index, "tag", argv[0]);
}

ALIAS (no_match_tag,
       no_match_tag_val_cmd,
       "no match tag <0-65535>",
       NO_STR
       MATCH_STR
       "Match tag of route\n"
       "Metric value\n")

/* set functions */

DEFUN (set_metric,
       set_metric_cmd,
       "set metric <0-4294967295>",
       SET_STR
       "Metric value for destination routing protocol\n"
       "Metric value\n")
{
  return ripRouteSetAdd (vty, vty->index, "metric", argv[0]);
}

ALIAS (set_metric,
       set_metric_addsub_cmd,
       "set metric <+/-metric>",
       SET_STR
       "Metric value for destination routing protocol\n"
       "Add or subtract metric\n")

DEFUN (no_set_metric,
       no_set_metric_cmd,
       "no set metric",
       NO_STR
       SET_STR
       "Metric value for destination routing protocol\n")
{
  if (argc == 0)
    return ripRouteSetDelete (vty, vty->index, "metric", NULL);

  return ripRouteSetDelete (vty, vty->index, "metric", argv[0]);
}

ALIAS (no_set_metric,
       no_set_metric_val_cmd,
       "no set metric (<0-4294967295>|<+/-metric>)",
       NO_STR
       SET_STR
       "Metric value for destination routing protocol\n"
       "Metric value\n"
       "Add or subtract metric\n")

DEFUN (set_ip_nexthop,
       set_ip_nexthop_cmd,
       "set ip next-hop A.B.C.D",
       SET_STR
       IP_STR
       "Next hop address\n"
       "IP address of next hop\n")
{
  union sockunion su;
  Int32T ret;

  ret = str2sockunion (argv[0], &su);
  if (ret < 0)
    {
      vty_out (vty, "%% Malformed next-hop address%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  return ripRouteSetAdd (vty, vty->index, "ip next-hop", argv[0]);
}

DEFUN (no_set_ip_nexthop,
       no_set_ip_nexthop_cmd,
       "no set ip next-hop",
       NO_STR
       SET_STR
       IP_STR
       "Next hop address\n")
{
  if (argc == 0)
    return ripRouteSetDelete (vty, vty->index, "ip next-hop", NULL);
  
  return ripRouteSetDelete (vty, vty->index, "ip next-hop", argv[0]);
}

ALIAS (no_set_ip_nexthop,
       no_set_ip_nexthop_val_cmd,
       "no set ip next-hop A.B.C.D",
       NO_STR
       SET_STR
       IP_STR
       "Next hop address\n"
       "IP address of next hop\n")

DEFUN (set_tag,
       set_tag_cmd,
       "set tag <0-65535>",
       SET_STR
       "Tag value for routing protocol\n"
       "Tag value\n")
{
  return ripRouteSetAdd (vty, vty->index, "tag", argv[0]);
}

DEFUN (no_set_tag,
       no_set_tag_cmd,
       "no set tag",
       NO_STR
       SET_STR
       "Tag value for routing protocol\n")
{
  if (argc == 0)
    return ripRouteSetDelete (vty, vty->index, "tag", NULL);
  
  return ripRouteSetDelete (vty, vty->index, "tag", argv[0]);
}

ALIAS (no_set_tag,
       no_set_tag_val_cmd,
       "no set tag <0-65535>",
       NO_STR
       SET_STR
       "Tag value for routing protocol\n"
       "Tag value\n")
#endif

void
ripRouteMapReset ()
{
  ;
}

/* Route-map init */
void
ripRouteMapInit ()
{
  /* Routemap vector init. */
  routeMapInit ();

  /* Assign routemap vector pointer. */
  pRip->routeMatchVec = routeMapMatchVecGetPtr ();
  pRip->routeSetVec = routeMapSetVecGetPtr ();

  /* Assign hook functions. */
  routeMapAddHook (ripRouteMapUpdate);
  routeMapDeleteHook (ripRouteMapUpdate);

  /* Vty init(to be changed). */
  routeMapInitVty ();

}

/* Route-map Update */
void
ripRouteMapVersionUpdate ()
{
  /* Assign routemap vector pointer. */
  routeMapUpdate (pRip->routeMatchVec, pRip->routeSetVec);

  /* Assign hook functions. */
  routeMapAddHook (ripRouteMapUpdate);
  routeMapDeleteHook (ripRouteMapUpdate);
}
