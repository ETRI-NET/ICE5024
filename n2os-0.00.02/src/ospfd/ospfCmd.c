/*
 * ospfCmd.c
 *
 *  Created on: 2014. 6. 2.
 *      Author: root
 */
#include <stdio.h>

#include "prefix.h"
#include "table.h"
#include "vty.h"
#include "plist.h"

#include "nnTypes.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdInstall.h"
#include "nosLib.h"

#include "ospfd.h"
#include "ospf_abr.h"
#include "ospf_interface.h"
#include "ospf_dump.h"
#include "ospf_asbr.h"
#include "ospf_lsa.h"
#include "ospfCmdUtil.h"
#include "ospf_nsm.h"
#include "ospf_lsdb.h"
#include "ospf_neighbor.h"
#include "ospf_ism.h"
#include "ospf_zebra.h"
#include "ospf_te.h"
#include "ospf_vty.h"
#include "ospf_routemap.h"
#include "ospfRibmgr.h"

DENODEC(cmdFuncEnterOspf,
        CMD_NODE_CONFIG_OSPF,
        IPC_OSPF)
{
	Int32T i;
	cmdPrint(cmsh, "uargv1[%d]\n", uargv1);
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

	om->index = ospf_get ();

	return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoRouterOspf,
		CMD_NODE_CONFIG,
		IPC_OSPF,
       "no router ospf",
       "Negate a command or set its defaults",
       "Enable a routing process",
       "Start OSPF configuration")
{
  struct ospf *ospf;

  ospf = ospf_lookup ();
  if (ospf == NULL)
    {
      cmdPrint (cmsh, "There isn't active ospf instance\n");
      return (CMD_IPC_WARNING);
    }

  ospf_finish (ospf);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfRouterId,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "ospf router-id A.B.C.D",
       "OSPF specific commands",
       "router-id for the OSPF process",
       "OSPF router-id in IP address format")
{
  struct ospf *ospf = om->index;
  struct in_addr router_id;
  int ret;

  ret = inet_aton (cargc == 3 ? cargv[2] : cargv[1], &router_id);
  if (!ret)
    {
      cmdPrint (cmsh, "Please specify Router ID by A.B.C.D\n");
      return (CMD_IPC_WARNING);
    }

  ospf->router_id_static = router_id;

  ospf_router_id_update (ospf);

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfRouterId,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "router-id A.B.C.D",
       "router-id for the OSPF process",
       "OSPF router-id in IP address format");

DECMD (cmdFuncOspfNoRouterId,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no ospf router-id",
       "Negate a command or set its defaults",
       "OSPF specific commands",
       "router-id for the OSPF process")
{
  struct ospf *ospf = om->index;

  ospf->router_id_static.s_addr = 0;

  ospf_router_id_update (ospf);

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoRouterId,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no router-id",
       "Negate a command or set its defaults",
       "router-id for the OSPF process");


DECMD (cmdFuncOspfPassiveInterface,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "passive-interface WORD A.B.C.D",
       "Suppress routing updates on an interface",
       "Interface's name",
       "Interface address")
{
  struct interface *ifp;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
  struct route_node *rn;
  struct ospf *ospf = om->index;

  if (!strcmp(cargv[1], "default"))
    {
      ospf_passive_interface_default (ospf, OSPF_IF_PASSIVE);
      return (CMD_IPC_OK);
    }

  ifp = if_get_by_name (cargv[1]);

  params = IF_DEF_PARAMS (ifp);

  if (cargc == 3)
    {
      ret = inet_aton(cargv[2], &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }
  ospf_passive_interface_update (ospf, ifp, addr, params, OSPF_IF_PASSIVE);

  /* XXX We should call ospf_if_set_multicast on exactly those
   * interfaces for which the passive property changed.  It is too much
   * work to determine this set, so we do this for every interface.
   * This is safe and reasonable because ospf_if_set_multicast uses a
   * record of joined groups to avoid systems calls if the desired
   * memberships match the current memership.
   */

  for (rn = route_top(IF_OIFS(ifp)); rn; rn = route_next (rn))
    {
      struct ospf_interface *oi = rn->info;

      if (oi && (OSPF_IF_PARAM(oi, passive_interface) == OSPF_IF_PASSIVE))
	ospf_if_set_multicast(oi);
    }
  /*
   * XXX It is not clear what state transitions the interface needs to
   * undergo when going from active to passive.  Fixing this will
   * require precise identification of interfaces having such a
   * transition.
   */

 return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfPassiveInterface,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "passive-interface WORD",
       "Suppress routing updates on an interface",
       "Interface's name");

ALICMD (cmdFuncOspfPassiveInterface,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "passive-interface default",
       "Suppress routing updates on an interface",
       "Suppress routing updates on interfaces by default");

DECMD (cmdFuncOspfNoPassiveInterface,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no passive-interface WORD A.B.C.D",
       "Negate a command or set its defaults",
       "Allow routing updates on an interface",
       "Interface's name",
       "Interface address")
{
  struct interface *ifp;
  struct in_addr addr;
  struct ospf_if_params *params;
  int ret;
  struct route_node *rn;
  struct ospf *ospf = om->index;

  if (!strcmp(cargv[2], "default"))
    {
      ospf_passive_interface_default (ospf, OSPF_IF_ACTIVE);
      return (CMD_IPC_OK);
    }

  ifp = if_get_by_name (cargv[2]);

  params = IF_DEF_PARAMS (ifp);

  if (cargc == 4)
    {
      ret = inet_aton(cargv[3], &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D\n");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_lookup_if_params (ifp, addr);
      if (params == NULL)
	return (CMD_IPC_OK);
    }
  ospf_passive_interface_update (ospf, ifp, addr, params, OSPF_IF_ACTIVE);

  /* XXX We should call ospf_if_set_multicast on exactly those
   * interfaces for which the passive property changed.  It is too much
   * work to determine this set, so we do this for every interface.
   * This is safe and reasonable because ospf_if_set_multicast uses a
   * record of joined groups to avoid systems calls if the desired
   * memberships match the current memership.
   */
  for (rn = route_top(IF_OIFS(ifp)); rn; rn = route_next (rn))
    {
      struct ospf_interface *oi = rn->info;

      if (oi && (OSPF_IF_PARAM(oi, passive_interface) == OSPF_IF_ACTIVE))
        ospf_if_set_multicast(oi);
    }

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoPassiveInterface,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no passive-interface WORD",
       "Negate a command or set its defaults",
       "Allow routing updates on an interface",
       "Interface's name");

ALICMD (cmdFuncOspfNoPassiveInterface,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no passive-interface default",
       "Negate a command or set its defaults",
       "Allow routing updates on an interface",
       "Allow routing updates on interfaces by default");

DECMD (cmdFuncOspfNetworkArea,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "network A.B.C.D/M area (A.B.C.D|<0-4294967295>)",
       "Enable routing on an IP network",
       "OSPF network prefix",
       "Set the OSPF area ID",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value")
{
  struct ospf *ospf= om->index;
  struct prefix_ipv4 p;
  struct in_addr area_id;
  int ret, format;

  /* Get network prefix and Area ID. */
  VTY_GET_IPV4_PREFIX ("network prefix", p, cargv[1]);
  VTY_GET_OSPF_AREA_ID (area_id, format, cargv[3]);

  ret = ospf_network_set (ospf, &p, area_id);
  if (ret == 0)
    {
      cmdPrint (cmsh, "There is already same network statement.\n");
      return (CMD_IPC_WARNING);
    }

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoNetworkArea,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no network A.B.C.D/M area (A.B.C.D|<0-4294967295>)",
       "Negate a command or set its defaults",
       "Enable routing on an IP network",
       "OSPF network prefix",
       "Set the OSPF area ID",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value")
{
  struct ospf *ospf = (struct ospf *) om->index;
  struct prefix_ipv4 p;
  struct in_addr area_id;
  int ret, format;

  /* Get network prefix and Area ID. */
  VTY_GET_IPV4_PREFIX ("network prefix", p, cargv[2]);
  VTY_GET_OSPF_AREA_ID (area_id, format, cargv[4]);

  ret = ospf_network_unset (ospf, &p, area_id);
  if (ret == 0)
    {
      cmdPrint (cmsh, "Can't find specified network area configuration.\n");
      return (CMD_IPC_WARNING);
    }

  return (CMD_IPC_OK);
}



DECMD (cmdFuncOspfAreaRange,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) range A.B.C.D/M",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Summarize routes matching address/mask (border routers only)",
       "Area range prefix")
{
  struct ospf *ospf = om->index;
  struct prefix_ipv4 p;
  struct in_addr area_id;
  int format;
  u_int32_t cost;

  VTY_GET_OSPF_AREA_ID (area_id, format, cargv[1]);
  VTY_GET_IPV4_PREFIX ("area range", p, cargv[3]);

  ospf_area_range_set (ospf, area_id, &p, OSPF_AREA_RANGE_ADVERTISE);
  if (cargc > 5)
    {
      VTY_GET_INTEGER ("range cost", cost, cargv[cargc - 1]);
      ospf_area_range_cost_set (ospf, area_id, &p, cost);
    }

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfAreaRange,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) range A.B.C.D/M advertise",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Summarize routes matching address/mask (border routers only)",
       "Area range prefix",
       "Advertise this range (default)");

ALICMD (cmdFuncOspfAreaRange,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) range A.B.C.D/M cost <0-16777215>",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Summarize routes matching address/mask (border routers only)",
       "Area range prefix",
       "User specified metric for this range",
       "Advertised metric for this range");

ALICMD (cmdFuncOspfAreaRange,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) range A.B.C.D/M advertise cost <0-16777215>",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Summarize routes matching address/mask (border routers only)",
       "Area range prefix",
       "Advertise this range (default)",
       "User specified metric for this range",
       "Advertised metric for this range")

DECMD (cmdFuncOspfAreaRangeNotAdvertise,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) range A.B.C.D/M not-advertise",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Summarize routes matching address/mask (border routers only)",
       "Area range prefix",
       "DoNotAdvertise this range")
{
  struct ospf *ospf = om->index;
  struct prefix_ipv4 p;
  struct in_addr area_id;
  int format;

  VTY_GET_OSPF_AREA_ID (area_id, format, cargv[1]);
  VTY_GET_IPV4_PREFIX ("area range", p, cargv[3]);

  ospf_area_range_set (ospf, area_id, &p, 0);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoAreaRange,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) range A.B.C.D/M",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Summarize routes matching address/mask (border routers only)",
       "Area range prefix")
{
  struct ospf *ospf = om->index;
  struct prefix_ipv4 p;
  struct in_addr area_id;
  int format;

  VTY_GET_OSPF_AREA_ID (area_id, format, cargv[2]);
  VTY_GET_IPV4_PREFIX ("area range", p, cargv[4]);

  ospf_area_range_unset (ospf, area_id, &p);

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoAreaRange,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) range A.B.C.D/M (advertise|not-advertise)",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Summarize routes matching address/mask (border routers only)",
       "Area range prefix",
       "Advertise this range (default)",
       "DoNotAdvertise this range");

ALICMD (cmdFuncOspfNoAreaRange,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) range A.B.C.D/M cost <0-16777215>",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Summarize routes matching address/mask (border routers only)",
       "Area range prefix",
       "User specified metric for this range",
       "Advertised metric for this range");

ALICMD (cmdFuncOspfNoAreaRange,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) range A.B.C.D/M advertise cost <0-16777215>",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Summarize routes matching address/mask (border routers only)",
       "Area range prefix",
       "Advertise this range (default)",
       "User specified metric for this range",
       "Advertised metric for this range");

DECMD (cmdFuncOspfAreaRangeSubstitute,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) range A.B.C.D/M substitute A.B.C.D/M",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Summarize routes matching address/mask (border routers only)",
       "Area range prefix",
       "Announce area range as another prefix",
       "Network prefix to be announced instead of range")
{
  struct ospf *ospf = om->index;
  struct prefix_ipv4 p, s;
  struct in_addr area_id;
  int format;

  VTY_GET_OSPF_AREA_ID (area_id, format, cargv[1]);
  VTY_GET_IPV4_PREFIX ("area range", p, cargv[3]);
  VTY_GET_IPV4_PREFIX ("substituted network prefix", s, cargv[5]);

  ospf_area_range_substitute_set (ospf, area_id, &p, &s);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfno_ospf_area_range_substitute,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) range A.B.C.D/M substitute A.B.C.D/M",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Summarize routes matching address/mask (border routers only)",
       "Area range prefix",
       "Announce area range as another prefix",
       "Network prefix to be announced instead of range")
{
  struct ospf *ospf = om->index;
  struct prefix_ipv4 p, s;
  struct in_addr area_id;
  int format;

  VTY_GET_OSPF_AREA_ID (area_id, format, cargv[2]);
  VTY_GET_IPV4_PREFIX ("area range", p, cargv[4]);
  VTY_GET_IPV4_PREFIX ("substituted network prefix", s, cargv[6]);

  ospf_area_range_substitute_unset (ospf, area_id, &p);

  return (CMD_IPC_OK);
}



DECMD (cmdFuncOspfAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR")
{
	struct ospf *ospf = om->index;
	struct ospf_vl_config_data vl_config;
	char auth_key[OSPF_AUTH_SIMPLE_SIZE + 1];
	char md5_key[OSPF_AUTH_MD5_SIZE + 1];
	int i;
	int ret;

	ospf_vl_config_data_init(&vl_config, cmsh);

	/* Read off first 2 parameters and check them */
	ret = ospf_str2area_id(cargv[1], &vl_config.area_id, &vl_config.format);
	if (ret < 0) {
		cmdPrint(cmsh, "OSPF area ID is invalid\n");
		return (CMD_IPC_WARNING);
	}

	ret = inet_aton(cargv[3], &vl_config.vl_peer);
	if (!ret) {
		cmdPrint(cmsh, "Please specify valid Router ID as a.b.c.d\n");
		return (CMD_IPC_WARNING);
	}

	if (cargc <= 4) {
		/* Thats all folks! - BUGS B. strikes again!!!*/

		return ospf_vl_set(ospf, &vl_config);
	}

	/* Deal with other parameters */
	for (i = 4; i < cargc; i++) {

		/* cmdPrint (vty, "argv[%d] - %s%s", i, argv[i], VTY_NEWLINE); */

		switch (cargv[i][0]) {

		case 'a':
			if (i > 2 || strncmp(cargv[i], "authentication-", 15) == 0) {
				/* authentication-key - this option can occur anywhere on
				 command line.  At start of command line
				 must check for authentication option. */
				memset(auth_key, 0, OSPF_AUTH_SIMPLE_SIZE + 1);
				strncpy(auth_key, cargv[i + 1], OSPF_AUTH_SIMPLE_SIZE);
				vl_config.auth_key = auth_key;
				i++;
			} else if (strncmp(cargv[i], "authentication", 14) == 0) {
				/* authentication  - this option can only occur at start
				 of command line */
				vl_config.auth_type = OSPF_AUTH_SIMPLE;
				if ((i + 1) < cargc) {
					if (strncmp(cargv[i + 1], "n", 1) == 0) {
						/* "authentication null" */
						vl_config.auth_type = OSPF_AUTH_NULL;
						i++;
					} else if (strncmp(cargv[i + 1], "m", 1) == 0
							&& strcmp(cargv[i + 1], "message-digest-") != 0) {
						/* "authentication message-digest" */
						vl_config.auth_type = OSPF_AUTH_CRYPTOGRAPHIC;
						i++;
					}
				}
			}
			break;

		case 'm':
			/* message-digest-key */
			i++;
			vl_config.crypto_key_id = strtol(cargv[i], NULL, 10);
			if (vl_config.crypto_key_id < 0)
				return (CMD_IPC_WARNING);
			i++;
			memset(md5_key, 0, OSPF_AUTH_MD5_SIZE + 1);
			strncpy(md5_key, cargv[i], OSPF_AUTH_MD5_SIZE);
			vl_config.md5_key = md5_key;
			break;

		case 'h':
			/* Hello interval */
			i++;
			vl_config.hello_interval = strtol(cargv[i], NULL, 10);
			if (vl_config.hello_interval < 0)
				return (CMD_IPC_WARNING);
			break;

		case 'r':
			/* Retransmit Interval */
			i++;
			vl_config.retransmit_interval = strtol(cargv[i], NULL, 10);
			if (vl_config.retransmit_interval < 0)
				return (CMD_IPC_WARNING);
			break;

		case 't':
			/* Transmit Delay */
			i++;
			vl_config.transmit_delay = strtol(cargv[i], NULL, 10);
			if (vl_config.transmit_delay < 0)
				return (CMD_IPC_WARNING);
			break;

		case 'd':
			/* Dead Interval */
			i++;
			vl_config.dead_interval = strtol(cargv[i], NULL, 10);
			if (vl_config.dead_interval < 0)
				return (CMD_IPC_WARNING);
			break;
		}
	}

	/* Action configuration */

	return ospf_vl_set(ospf, &vl_config);

}

DECMD (cmdFuncOspfNoAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR")
{
	struct ospf *ospf = om->index;
	struct ospf_area *area;
	struct ospf_vl_config_data vl_config;
	struct ospf_vl_data *vl_data = NULL;
	char auth_key[OSPF_AUTH_SIMPLE_SIZE + 1];
	int i;
	int ret, format;

	ospf_vl_config_data_init(&vl_config, cmsh);

	ret = ospf_str2area_id(cargv[2], &vl_config.area_id, &format);
	if (ret < 0) {
		cmdPrint(cmsh, "OSPF area ID is invalid\n");
		return (CMD_IPC_WARNING);
	}

	area = ospf_area_lookup_by_area_id(ospf, vl_config.area_id);
	if (!area) {
		cmdPrint(cmsh, "Area does not exist\n");
		return (CMD_IPC_WARNING);
	}

	ret = inet_aton(cargv[4], &vl_config.vl_peer);
	if (!ret) {
		cmdPrint(cmsh, "Please specify valid Router ID as a.b.c.d\n");
		return (CMD_IPC_WARNING);
	}

	if (cargc <= 5) {
		/* Basic VLink no command */
		/* Thats all folks! - BUGS B. strikes again!!!*/
		if ((vl_data = ospf_vl_lookup(ospf, area, vl_config.vl_peer)))
			ospf_vl_delete(ospf, vl_data);

		ospf_area_check_free(ospf, vl_config.area_id);

		return (CMD_IPC_OK);
	}

	/* If we are down here, we are reseting parameters */

	/* Deal with other parameters */
	for (i = 5; i < cargc; i++) {
		/* cmdPrint (vty, "argv[%d] - %s%s", i, argv[i], VTY_NEWLINE); */

		switch (cargv[i][0]) {

		case 'a':
			if (i > 2 || strncmp(cargv[i], "authentication-", 15) == 0) {
				/* authentication-key - this option can occur anywhere on
				 command line.  At start of command line
				 must check for authentication option. */
				memset(auth_key, 0, OSPF_AUTH_SIMPLE_SIZE + 1);
				vl_config.auth_key = auth_key;
			} else if (strncmp(cargv[i], "authentication", 14) == 0) {
				/* authentication  - this option can only occur at start
				 of command line */
				vl_config.auth_type = OSPF_AUTH_NOTSET;
			}
			break;

		case 'm':
			/* message-digest-key */
			/* Delete one key */
			i++;
			vl_config.crypto_key_id = strtol(cargv[i], NULL, 10);
			if (vl_config.crypto_key_id < 0)
				return (CMD_IPC_WARNING);
			vl_config.md5_key = NULL;
			break;

		case 'h':
			/* Hello interval */
			vl_config.hello_interval = OSPF_HELLO_INTERVAL_DEFAULT;
			break;

		case 'r':
			/* Retransmit Interval */
			vl_config.retransmit_interval = OSPF_RETRANSMIT_INTERVAL_DEFAULT;
			break;

		case 't':
			/* Transmit Delay */
			vl_config.transmit_delay = OSPF_TRANSMIT_DELAY_DEFAULT;
			break;

		case 'd':
			/* Dead Interval */
			i++;
			vl_config.dead_interval = OSPF_ROUTER_DEAD_INTERVAL_DEFAULT;
			break;
		}
	}

	/* Action configuration */

	return ospf_vl_set(ospf, &vl_config);
}

ALICMD (cmdFuncOspfAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535>",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR",
       "Time between HELLO packets",
       "Time between retransmitting lost link state advertisements",
       "Link state transmit delay",
       "Interval after which a neighbor is declared dead",
       "Seconds");

ALICMD (cmdFuncOspfNoAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval)",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR",
       "Time between HELLO packets",
       "Time between retransmitting lost link state advertisements",
       "Link state transmit delay",
       "Interval after which a neighbor is declared dead",
       "Seconds");

ALICMD (cmdFuncOspfAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535> "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535>",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR",
       "Time between HELLO packets",
       "Time between retransmitting lost link state advertisements",
       "Link state transmit delay",
       "Interval after which a neighbor is declared dead",
       "Seconds",
       "Time between HELLO packets",
       "Time between retransmitting lost link state advertisements",
       "Link state transmit delay",
       "Interval after which a neighbor is declared dead",
       "Seconds");

ALICMD (cmdFuncOspfNoAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval)",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR",
       "Time between HELLO packets",
       "Time between retransmitting lost link state advertisements",
       "Link state transmit delay",
       "Interval after which a neighbor is declared dead",
       "Seconds",
       "Time between HELLO packets",
       "Time between retransmitting lost link state advertisements",
       "Link state transmit delay",
       "Interval after which a neighbor is declared dead",
       "Seconds");

ALICMD (cmdFuncOspfAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535> "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535> "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535>",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR",
       "Time between HELLO packets",
       "Time between retransmitting lost link state advertisements",
       "Link state transmit delay",
       "Interval after which a neighbor is declared dead",
       "Seconds",
       "Time between HELLO packets",
       "Time between retransmitting lost link state advertisements",
       "Link state transmit delay",
       "Interval after which a neighbor is declared dead",
       "Seconds",
       "Time between HELLO packets",
       "Time between retransmitting lost link state advertisements",
       "Link state transmit delay",
       "Interval after which a neighbor is declared dead",
       "Seconds");

ALICMD (cmdFuncOspfNoAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval)",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR"
       "Time between HELLO packets",
       "Time between retransmitting lost link state advertisements",
       "Link state transmit delay",
       "Interval after which a neighbor is declared dead",
       "Seconds",
       "Time between HELLO packets",
       "Time between retransmitting lost link state advertisements",
       "Link state transmit delay",
       "Interval after which a neighbor is declared dead",
       "Seconds",
       "Time between HELLO packets",
       "Time between retransmitting lost link state advertisements",
       "Link state transmit delay",
       "Interval after which a neighbor is declared dead",
       "Seconds");

ALICMD (cmdFuncOspfAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535> "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535> "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535> "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) <1-65535>",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR",
       "Time between HELLO packets",
       "Time between retransmitting lost link state advertisements",
       "Link state transmit delay",
       "Interval after which a neighbor is declared dead",
       "Seconds",
       "Time between HELLO packets",
       "Time between retransmitting lost link state advertisements",
       "Link state transmit delay",
       "Interval after which a neighbor is declared dead",
       "Seconds",
       "Time between HELLO packets",
       "Time between retransmitting lost link state advertisements",
       "Link state transmit delay",
       "Interval after which a neighbor is declared dead",
       "Seconds",
       "Time between HELLO packets",
       "Time between retransmitting lost link state advertisements",
       "Link state transmit delay",
       "Interval after which a neighbor is declared dead",
       "Seconds");

ALICMD (cmdFuncOspfNoAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval) "
       "(hello-interval|retransmit-interval|transmit-delay|dead-interval)",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR"
       "Time between HELLO packets",
       "Time between retransmitting lost link state advertisements",
       "Link state transmit delay",
       "Interval after which a neighbor is declared dead",
       "Seconds",
       "Time between HELLO packets",
       "Time between retransmitting lost link state advertisements",
       "Link state transmit delay",
       "Interval after which a neighbor is declared dead",
       "Seconds",
       "Time between HELLO packets",
       "Time between retransmitting lost link state advertisements",
       "Link state transmit delay",
       "Interval after which a neighbor is declared dead",
       "Seconds",
       "Time between HELLO packets",
       "Time between retransmitting lost link state advertisements",
       "Link state transmit delay",
       "Interval after which a neighbor is declared dead",
       "Seconds");

ALICMD (cmdFuncOspfAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D "
       "(authentication|) (message-digest|null)",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR",
       "Enable authentication on this virtual link",
       "dummy string ",
       "Use null authentication",
       "Use message-digest authentication");

ALICMD (cmdFuncOspfAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D "
       "(authentication|)",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR",
       "Enable authentication on this virtual link",
       "dummy string ");

ALICMD (cmdFuncOspfNoAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D "
       "(authentication|)",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR",
       "Enable authentication on this virtual link",
       "dummy string ");

ALICMD (cmdFuncOspfAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D "
       "(message-digest-key|) <1-255> md5 KEY",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR",
       "Message digest authentication password (key)",
       "dummy string ",
       "Key ID",
       "Use MD5 algorithm",
       "The OSPF password (key)");

ALICMD (cmdFuncOspfNoAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D "
       "(message-digest-key|) <1-255>",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR",
       "Message digest authentication password (key)",
       "dummy string ",
       "Key ID",
       "Use MD5 algorithm",
       "The OSPF password (key)")

ALICMD (cmdFuncOspfAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D "
       "(authentication-key|) AUTH_KEY",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR",
       "Authentication password (key)",
       "The OSPF password (key)");

ALICMD (cmdFuncOspfNoAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D "
       "(authentication-key|)",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR",
       "Authentication password (key)",
       "The OSPF password (key)");

ALICMD (cmdFuncOspfAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D "
       "(authentication|) (message-digest|null) "
       "(authentication-key|) AUTH_KEY",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR",
       "Enable authentication on this virtual link",
       "dummy string ",
       "Use null authentication",
       "Use message-digest authentication",
       "Authentication password (key)",
       "The OSPF password (key)");

ALICMD (cmdFuncOspfAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D "
       "(authentication|) "
       "(authentication-key|) AUTH_KEY",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR",
       "Enable authentication on this virtual link",
       "dummy string ",
       "Authentication password (key)",
       "The OSPF password (key)");

ALICMD (cmdFuncOspfNoAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D "
       "(authentication|) "
       "(authentication-key|)",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR",
       "Enable authentication on this virtual link",
       "dummy string ",
       "Authentication password (key)",
       "The OSPF password (key)");

ALICMD (cmdFuncOspfAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D "
       "(authentication|) (message-digest|null) "
       "(message-digest-key|) <1-255> md5 KEY",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR",
       "Enable authentication on this virtual link",
       "dummy string ",
       "Use null authentication",
       "Use message-digest authentication",
       "Message digest authentication password (key)",
       "dummy string ",
       "Key ID",
       "Use MD5 algorithm",
       "The OSPF password (key)");

ALICMD (cmdFuncOspfAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D "
       "(authentication|) "
       "(message-digest-key|) <1-255> md5 KEY",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR",
       "Enable authentication on this virtual link",
       "dummy string ",
       "Message digest authentication password (key)",
       "dummy string ",
       "Key ID",
       "Use MD5 algorithm",
       "The OSPF password (key)");

ALICMD (cmdFuncOspfNoAreaVlink,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) virtual-link A.B.C.D "
       "(authentication|) "
       "(message-digest-key|)",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure a virtual link",
       "Router ID of the remote ABR",
       "Enable authentication on this virtual link",
       "dummy string ",
       "Message digest authentication password (key)",
       "dummy string ",
       "Key ID",
       "Use MD5 algorithm",
       "The OSPF password (key)");


DECMD (cmdFuncOspfAreaShortcut,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) shortcut (default|enable|disable)",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure the area's shortcutting mode",
       "Set default shortcutting behavior",
       "Enable shortcutting through the area",
       "Disable shortcutting through the area")
{
  struct ospf *ospf = om->index;
  struct ospf_area *area;
  struct in_addr area_id;
  int mode;
  int format;

  VTY_GET_OSPF_AREA_ID_NO_BB ("shortcut", area_id, format, cargv[1]);

  area = ospf_area_get (ospf, area_id, format);

  if (strncmp (cargv[3], "de", 2) == 0)
    mode = OSPF_SHORTCUT_DEFAULT;
  else if (strncmp (cargv[3], "di", 2) == 0)
    mode = OSPF_SHORTCUT_DISABLE;
  else if (strncmp (cargv[3], "e", 1) == 0)
    mode = OSPF_SHORTCUT_ENABLE;
  else
    return (CMD_IPC_WARNING);

  ospf_area_shortcut_set (ospf, area, mode);

  if (ospf->abr_type != OSPF_ABR_SHORTCUT)
    cmdPrint (cmsh, "Shortcut area setting will take effect "
	     "only when the router is configured as Shortcut ABR\n");

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoOspfAreaShortcut,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) shortcut (enable|disable)",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Deconfigure the area's shortcutting mode",
       "Deconfigure enabled shortcutting through the area",
       "Deconfigure disabled shortcutting through the area")
{
  struct ospf *ospf = om->index;
  struct ospf_area *area;
  struct in_addr area_id;
  int format;

  VTY_GET_OSPF_AREA_ID_NO_BB ("shortcut", area_id, format, cargv[2]);

  area = ospf_area_lookup_by_area_id (ospf, area_id);
  if (!area)
    return (CMD_IPC_OK);

  ospf_area_shortcut_unset (ospf, area);

  return (CMD_IPC_OK);
}


DECMD (cmdFuncOspfAreaStub,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) stub",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure OSPF area as stub")
{
  struct ospf *ospf = om->index;
  struct in_addr area_id;
  int ret, format;

  VTY_GET_OSPF_AREA_ID_NO_BB ("stub", area_id, format, cargv[1]);

  ret = ospf_area_stub_set (ospf, area_id);
  if (ret == 0)
    {
      cmdPrint (cmsh, "First deconfigure all virtual link through this area\n");
      return (CMD_IPC_WARNING);
    }

  ospf_area_no_summary_unset (ospf, area_id);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfAreaStubNoSummary,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) stub no-summary",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure OSPF area as stub",
       "Do not inject inter-area routes into stub")
{
  struct ospf *ospf = om->index;
  struct in_addr area_id;
  int ret, format;

  VTY_GET_OSPF_AREA_ID_NO_BB ("stub", area_id, format, cargv[1]);

  ret = ospf_area_stub_set (ospf, area_id);
  if (ret == 0)
    {
      cmdPrint (cmsh, "%% Area cannot be stub as it contains a virtual link\n");
      return (CMD_IPC_WARNING);
    }

  ospf_area_no_summary_set (ospf, area_id);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoAreaStub,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) stub",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure OSPF area as stub")
{
  struct ospf *ospf = om->index;
  struct in_addr area_id;
  int format;

  VTY_GET_OSPF_AREA_ID_NO_BB ("stub", area_id, format, cargv[2]);

  ospf_area_stub_unset (ospf, area_id);
  ospf_area_no_summary_unset (ospf, area_id);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoAreaStubNoSummary,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) stub no-summary",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure OSPF area as stub",
       "Do not inject inter-area routes into area")
{
  struct ospf *ospf = om->index;
  struct in_addr area_id;
  int format;

  VTY_GET_OSPF_AREA_ID_NO_BB ("stub", area_id, format, cargv[2]);
  ospf_area_no_summary_unset (ospf, area_id);

  return (CMD_IPC_OK);
}


DECMD (cmdFuncOspfAreaNssaTranslateNoSummary,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) nssa (translate-candidate|translate-never|translate-always) no-summary",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure OSPF area as nssa",
       "Configure NSSA-ABR for translate election (default)",
       "Configure NSSA-ABR to never translate",
       "Configure NSSA-ABR to always translate",
       "Do not inject inter-area routes into nssa")
{
   return ospf_area_nssa_cmd_handler (cmsh, cargc, cargv, 1);
}

DECMD (cmdFuncOspfAreaNssaTranslate,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) nssa (translate-candidate|translate-never|translate-always)",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure OSPF area as nssa",
       "Configure NSSA-ABR for translate election (default)",
       "Configure NSSA-ABR to never translate",
       "Configure NSSA-ABR to always translate")
{
  return ospf_area_nssa_cmd_handler (cmsh, cargc, cargv, 0);
}

DECMD (cmdFuncOspfAreaNssa,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) nssa",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure OSPF area as nssa")
{
  return ospf_area_nssa_cmd_handler (cmsh, cargc, cargv, 0);
}

DECMD (cmdFuncOspfAreaNssaNoSummary,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) nssa no-summary",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure OSPF area as nssa",
       "Do not inject inter-area routes into nssa")
{
  return ospf_area_nssa_cmd_handler (cmsh, cargc, cargv, 1);
}

DECMD (cmdFuncOspfNoAareaNssa,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) nssa",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure OSPF area as nssa")
{
  struct ospf *ospf = om->index;
  struct in_addr area_id;
  int format;

  VTY_GET_OSPF_AREA_ID_NO_BB ("NSSA", area_id, format, cargv[2]);

  ospf_area_nssa_unset (ospf, area_id);
  ospf_area_no_summary_unset (ospf, area_id);

  ospf_schedule_abr_task (ospf);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoAreaNssaNoSummary,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) nssa no-summary",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Configure OSPF area as nssa",
       "Do not inject inter-area routes into nssa")
{
  struct ospf *ospf = om->index;
  struct in_addr area_id;
  int format;

  VTY_GET_OSPF_AREA_ID_NO_BB ("NSSA", area_id, format, cargv[2]);
  ospf_area_no_summary_unset (ospf, area_id);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfAreaDefaultCost,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) default-cost <0-16777215>",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Set the summary-default cost of a NSSA or stub area",
       "Stub's advertised default summary cost")
{
  struct ospf *ospf = om->index;
  struct ospf_area *area;
  struct in_addr area_id;
  u_int32_t cost;
  int format;
  struct prefix_ipv4 p;

  VTY_GET_OSPF_AREA_ID_NO_BB ("default-cost", area_id, format, cargv[1]);
  VTY_GET_INTEGER_RANGE ("stub default cost", cost, cargv[3], 0, 16777215);

  area = ospf_area_get (ospf, area_id, format);

  if (area->external_routing == OSPF_AREA_DEFAULT)
    {

      cmdPrint (cmsh, "The area is neither stub, nor NSSA\n");
      return (CMD_IPC_WARNING);
    }

  area->default_cost = cost;

  p.family = AF_INET;
  p.prefix.s_addr = OSPF_DEFAULT_DESTINATION;
  p.prefixlen = 0;
  if (IS_DEBUG_OSPF_EVENT)
    zlog_debug ("ospf_abr_announce_stub_defaults(): "
                "announcing 0.0.0.0/0 to area %s",
               inet_ntoa (area->area_id));
  ospf_abr_announce_network_to_area (&p, area->default_cost, area);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoAreaDefaultCost,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) default-cost <0-16777215>",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Set the summary-default cost of a NSSA or stub area",
       "Stub's advertised default summary cost")
{
  struct ospf *ospf = om->index;
  struct ospf_area *area;
  struct in_addr area_id;
  int format;
  struct prefix_ipv4 p;

  VTY_GET_OSPF_AREA_ID_NO_BB ("default-cost", area_id, format, cargv[2]);
  VTY_CHECK_INTEGER_RANGE ("stub default cost", cargv[4], 0, OSPF_LS_INFINITY);

  area = ospf_area_lookup_by_area_id (ospf, area_id);
  if (area == NULL)
    return (CMD_IPC_OK);

  if (area->external_routing == OSPF_AREA_DEFAULT)
    {
      cmdPrint (cmsh, "The area is neither stub, nor NSSA\n");
      return (CMD_IPC_WARNING);
    }

  area->default_cost = 1;

  p.family = AF_INET;
  p.prefix.s_addr = OSPF_DEFAULT_DESTINATION;
  p.prefixlen = 0;
  if (IS_DEBUG_OSPF_EVENT)
    zlog_debug ("ospf_abr_announce_stub_defaults(): "
                "announcing 0.0.0.0/0 to area %s",
               inet_ntoa (area->area_id));
  ospf_abr_announce_network_to_area (&p, area->default_cost, area);


  ospf_area_check_free (ospf, area_id);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfAreaExportList,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) export-list WORD",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Set the filter for networks announced to other areas",
       "Name of the access-list")
{
  struct ospf *ospf = om->index;
  struct ospf_area *area;
  struct in_addr area_id;
  int format;

  VTY_GET_OSPF_AREA_ID (area_id, format, cargv[1]);

  area = ospf_area_get (ospf, area_id, format);
  ospf_area_export_list_set (ospf, area, cargv[3]);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoAreaExportList,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) export-list WORD",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Unset the filter for networks announced to other areas",
       "Name of the access-list")
{
  struct ospf *ospf = om->index;
  struct ospf_area *area;
  struct in_addr area_id;
  int format;

  VTY_GET_OSPF_AREA_ID (area_id, format, cargv[2]);

  area = ospf_area_lookup_by_area_id (ospf, area_id);
  if (area == NULL)
    return (CMD_IPC_OK);

  ospf_area_export_list_unset (ospf, area);

  return (CMD_IPC_OK);
}


DECMD (cmdFuncOspfAreaImportList,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) import-list WORD",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Set the filter for networks from other areas announced to the specified one",
       "Name of the access-list")
{
  struct ospf *ospf = om->index;
  struct ospf_area *area;
  struct in_addr area_id;
  int format;

  VTY_GET_OSPF_AREA_ID (area_id, format, cargv[1]);

  area = ospf_area_get (ospf, area_id, format);
  ospf_area_import_list_set (ospf, area, cargv[3]);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoAreaImportList,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) import-list WORD",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Unset the filter for networks announced to other areas",
       "Name of the access-list")
{
  struct ospf *ospf = om->index;
  struct ospf_area *area;
  struct in_addr area_id;
  int format;

  VTY_GET_OSPF_AREA_ID (area_id, format, cargv[2]);

  area = ospf_area_lookup_by_area_id (ospf, area_id);
  if (area == NULL)
    return (CMD_IPC_OK);

  ospf_area_import_list_unset (ospf, area);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfAreaFilterList,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) filter-list prefix WORD (in|out)",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Filter networks between OSPF areas",
       "Filter prefixes between OSPF areas",
       "Name of an IP prefix-list",
       "Filter networks sent to this area",
       "Filter networks sent from this area")
{
  struct ospf *ospf = om->index;
  struct ospf_area *area;
  struct in_addr area_id;
  struct prefix_list *plist;
  int format;

  VTY_GET_OSPF_AREA_ID (area_id, format, cargv[1]);

  area = ospf_area_get (ospf, area_id, format);
  plist = prefix_list_lookup (AFI_IP, cargv[4]);
  if (strncmp (cargv[5], "in", 2) == 0)
    {
      PREFIX_LIST_IN (area) = plist;
      if (PREFIX_NAME_IN (area))
	free (PREFIX_NAME_IN (area));

      PREFIX_NAME_IN (area) = strdup (cargv[4]);
      ospf_schedule_abr_task (ospf);
    }
  else
    {
      PREFIX_LIST_OUT (area) = plist;
      if (PREFIX_NAME_OUT (area))
	free (PREFIX_NAME_OUT (area));

      PREFIX_NAME_OUT (area) = strdup (cargv[4]);
      ospf_schedule_abr_task (ospf);
    }

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoAreaFilterList,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) filter-list prefix WORD (in|out)",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Filter networks between OSPF areas",
       "Filter prefixes between OSPF areas",
       "Name of an IP prefix-list",
       "Filter networks sent to this area",
       "Filter networks sent from this area")
{
  struct ospf *ospf = om->index;
  struct ospf_area *area;
  struct in_addr area_id;
  int format;

  VTY_GET_OSPF_AREA_ID (area_id, format, cargv[2]);

  if ((area = ospf_area_lookup_by_area_id (ospf, area_id)) == NULL)
    return (CMD_IPC_OK);

  if (strncmp (cargv[6], "in", 2) == 0)
    {
      if (PREFIX_NAME_IN (area))
	if (strcmp (PREFIX_NAME_IN (area), cargv[5]) != 0)
	  return (CMD_IPC_OK);

      PREFIX_LIST_IN (area) = NULL;
      if (PREFIX_NAME_IN (area))
	free (PREFIX_NAME_IN (area));

      PREFIX_NAME_IN (area) = NULL;

      ospf_schedule_abr_task (ospf);
    }
  else
    {
      if (PREFIX_NAME_OUT (area))
	if (strcmp (PREFIX_NAME_OUT (area), cargv[5]) != 0)
	  return (CMD_IPC_OK);

      PREFIX_LIST_OUT (area) = NULL;
      if (PREFIX_NAME_OUT (area))
	free (PREFIX_NAME_OUT (area));

      PREFIX_NAME_OUT (area) = NULL;

      ospf_schedule_abr_task (ospf);
    }

  return (CMD_IPC_OK);
}


DECMD (cmdFuncOspfAreaAuthenticationMessageDigest,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) authentication message-digest",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Enable authentication",
       "Use message-digest authentication")
{
  struct ospf *ospf = om->index;
  struct ospf_area *area;
  struct in_addr area_id;
  int format;

  VTY_GET_OSPF_AREA_ID (area_id, format, cargv[1]);

  area = ospf_area_get (ospf, area_id, format);
  area->auth_type = OSPF_AUTH_CRYPTOGRAPHIC;

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfAreaAuthentication,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "area (A.B.C.D|<0-4294967295>) authentication",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Enable authentication")
{
  struct ospf *ospf = om->index;
  struct ospf_area *area;
  struct in_addr area_id;
  int format;

  VTY_GET_OSPF_AREA_ID (area_id, format, cargv[1]);

  area = ospf_area_get (ospf, area_id, format);
  area->auth_type = OSPF_AUTH_SIMPLE;

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoAreaAuthentication,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no area (A.B.C.D|<0-4294967295>) authentication",
       "Negate a command or set its defaults",
       "OSPF area parameters",
       "OSPF area ID in IP address format",
       "OSPF area ID as a decimal value",
       "Enable authentication")
{
  struct ospf *ospf = om->index;
  struct ospf_area *area;
  struct in_addr area_id;
  int format;

  VTY_GET_OSPF_AREA_ID (area_id, format, cargv[2]);

  area = ospf_area_lookup_by_area_id (ospf, area_id);
  if (area == NULL)
    return (CMD_IPC_OK);

  area->auth_type = OSPF_AUTH_NULL;

  ospf_area_check_free (ospf, area_id);

  return (CMD_IPC_OK);
}


DECMD (cmdFuncOspfAbrType,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "ospf abr-type (cisco|ibm|shortcut|standard)",
       "OSPF specific commands",
       "Set OSPF ABR type",
       "Alternative ABR cisco implementation",
       "Alternative ABR IBM implementation",
       "Shortcut ABR",
       "Standard behavior (RFC2328)")
{
	struct ospf *ospf = om->index;
	u_char abr_type = OSPF_ABR_UNKNOWN;

	if (strncmp(cargv[2], "c", 1) == 0)
		abr_type = OSPF_ABR_CISCO;
	else if (strncmp(cargv[2], "i", 1) == 0)
		abr_type = OSPF_ABR_IBM;
	else if (strncmp(cargv[2], "sh", 2) == 0)
		abr_type = OSPF_ABR_SHORTCUT;
	else if (strncmp(cargv[2], "st", 2) == 0)
		abr_type = OSPF_ABR_STAND;
	else
		return (CMD_IPC_WARNING);

	/* If ABR type value is changed, schedule ABR task. */
	if (ospf->abr_type != abr_type) {
		ospf->abr_type = abr_type;
		ospf_schedule_abr_task(ospf);
	}

	return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoAbrType,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no ospf abr-type (cisco|ibm|shortcut|standard)",
       "Negate a command or set its defaults",
       "OSPF specific commands",
       "Set OSPF ABR type",
       "Alternative ABR cisco implementation",
       "Alternative ABR IBM implementation",
       "Shortcut ABR",
       "Standard behavior (RFC2328)")
{
	struct ospf *ospf = om->index;
	u_char abr_type = OSPF_ABR_UNKNOWN;

	if (strncmp(cargv[3], "c", 1) == 0)
		abr_type = OSPF_ABR_CISCO;
	else if (strncmp(cargv[3], "i", 1) == 0)
		abr_type = OSPF_ABR_IBM;
	else if (strncmp(cargv[3], "sh", 2) == 0)
		abr_type = OSPF_ABR_SHORTCUT;
	else if (strncmp(cargv[3], "st", 2) == 0)
		abr_type = OSPF_ABR_STAND;
	else
		return (CMD_IPC_WARNING);

	/* If ABR type value is changed, schedule ABR task. */
	if (ospf->abr_type == abr_type) {
		ospf->abr_type = OSPF_ABR_DEFAULT;
		ospf_schedule_abr_task(ospf);
	}

	return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfLogAdjacencyChanges,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "log-adjacency-changes",
       "Log changes in adjacency state")
{
  struct ospf *ospf = om->index;

  SET_FLAG(ospf->config, OSPF_LOG_ADJACENCY_CHANGES);
  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfLogAdjacencyChangesDetail,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "log-adjacency-changes detail",
       "Log changes in adjacency state",
       "Log all state changes")
{
  struct ospf *ospf = om->index;

  SET_FLAG(ospf->config, OSPF_LOG_ADJACENCY_CHANGES);
  SET_FLAG(ospf->config, OSPF_LOG_ADJACENCY_DETAIL);
  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoLogAdjacencyChanges,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no log-adjacency-changes",
       "Negate a command or set its defaults",
       "Log changes in adjacency state")
{
  struct ospf *ospf = om->index;

  UNSET_FLAG(ospf->config, OSPF_LOG_ADJACENCY_DETAIL);
  UNSET_FLAG(ospf->config, OSPF_LOG_ADJACENCY_CHANGES);
  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoLogAdjacencyChangesDetail,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no log-adjacency-changes detail",
       "Negate a command or set its defaults",
       "Log changes in adjacency state",
       "Log all state changes")
{
  struct ospf *ospf = om->index;

  UNSET_FLAG(ospf->config, OSPF_LOG_ADJACENCY_DETAIL);
  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfCompatibleRfc1583,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "compatible rfc1583",
       "OSPF compatibility list",
       "compatible with RFC 1583")
{
  struct ospf *ospf = om->index;

  if (!CHECK_FLAG (ospf->config, OSPF_RFC1583_COMPATIBLE))
    {
      SET_FLAG (ospf->config, OSPF_RFC1583_COMPATIBLE);
      ospf_spf_calculate_schedule (ospf);
    }
  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoCompatibleRfc1583,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no compatible rfc1583",
       "Negate a command or set its defaults",
       "OSPF compatibility list",
       "compatible with RFC 1583")
{
  struct ospf *ospf = om->index;

  if (CHECK_FLAG (ospf->config, OSPF_RFC1583_COMPATIBLE))
    {
      UNSET_FLAG (ospf->config, OSPF_RFC1583_COMPATIBLE);
      ospf_spf_calculate_schedule (ospf);
    }
  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfCompatibleRfc1583,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "ospf rfc1583compatibility",
       "OSPF specific commands",
       "Enable the RFC1583Compatibility flag");

ALICMD (cmdFuncOspfNoCompatibleRfc1583,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no ospf rfc1583compatibility",
       "Negate a command or set its defaults",
       "OSPF specific commands",
       "Disable the RFC1583Compatibility flag");


DECMD (cmdFuncOspfTimersThrottleSpf,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "timers throttle spf <0-600000> <0-600000> <0-600000>",
       "Adjust routing timers",
       "Throttling adaptive timer",
       "OSPF SPF timers",
       "Delay (msec) from first change received till SPF calculation",
       "Initial hold time (msec) between consecutive SPF calculations",
       "Maximum hold time (msec)")
{
  unsigned int delay, hold, max;

  if (cargc != 6)
    {
      cmdPrint (cmsh, "Insufficient arguments\n");
      return (CMD_IPC_WARNING);
    }

  VTY_GET_INTEGER_RANGE ("SPF delay timer", delay, cargv[3], 0, 600000);
  VTY_GET_INTEGER_RANGE ("SPF hold timer", hold, cargv[4], 0, 600000);
  VTY_GET_INTEGER_RANGE ("SPF max-hold timer", max, cargv[5], 0, 600000);

  return ospf_timers_spf_set (cmsh, delay, hold, max);
}

DECMD (cmdFuncOspfTimersSpf,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "timers spf <0-4294967295> <0-4294967295>",
       "Adjust routing timers",
       "OSPF SPF timers",
       "Delay (s) between receiving a change to SPF calculation",
       "Hold time (s) between consecutive SPF calculations")
{
  unsigned int delay, hold;

  if (cargc != 4)
    {
      cmdPrint (cmsh, "Insufficient number of arguments\n");
      return (CMD_IPC_WARNING);
    }

  VTY_GET_INTEGER ("SPF delay timer", delay, cargv[2]);
  VTY_GET_INTEGER ("SPF hold timer", hold, cargv[3]);

  /* truncate down the second values if they're greater than 600000ms */
  if (delay > (600000 / 1000))
    delay = 600000;
  else if (delay == 0)
    /* 0s delay was probably specified because of lack of ms resolution */
    delay = OSPF_SPF_DELAY_DEFAULT;
  if (hold > (600000 / 1000))
    hold = 600000;

  return ospf_timers_spf_set (cmsh, delay * 1000, hold * 1000, hold * 1000);
}

DECMD (cmdFuncOspfNoTimersThrottleSpf,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no timers throttle spf",
       "Negate a command or set its defaults",
       "Adjust routing timers",
       "Throttling adaptive timer",
       "OSPF SPF timers")
{
  return ospf_timers_spf_set (cmsh,
                              OSPF_SPF_DELAY_DEFAULT,
                              OSPF_SPF_HOLDTIME_DEFAULT,
                              OSPF_SPF_MAX_HOLDTIME_DEFAULT);
}

ALICMD (cmdFuncOspfNoTimersThrottleSpf,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
	  "no timers spf",
	  "Negate a command or set its defaults",
	  "Adjust routing timers",
	  "OSPF SPF timers");

DECMD (cmdFuncOspfNeighbor,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "neighbor A.B.C.D",
       "Specify neighbor router",
       "Neighbor IP address")
{
  struct ospf *ospf = om->index;
  struct in_addr nbr_addr;
  unsigned int priority = OSPF_NEIGHBOR_PRIORITY_DEFAULT;
  unsigned int interval = OSPF_POLL_INTERVAL_DEFAULT;

  VTY_GET_IPV4_ADDRESS ("neighbor address", nbr_addr, cargv[1]);

  if (cargc > 2)
    VTY_GET_INTEGER_RANGE ("neighbor priority", priority, cargv[3], 0, 255);

  if (cargc > 4)
    VTY_GET_INTEGER_RANGE ("poll interval", interval, cargv[5], 1, 65535);

  ospf_nbr_nbma_set (ospf, nbr_addr);
  if (cargc > 2)
    ospf_nbr_nbma_priority_set (ospf, nbr_addr, priority);
  if (cargc > 4)
    ospf_nbr_nbma_poll_interval_set (ospf, nbr_addr, interval);

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNeighbor,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "neighbor A.B.C.D priority <0-255> poll-interval <1-65535>",
       "Specify neighbor router",
       "Neighbor IP address",
       "Neighbor Priority",
       "Priority",
       "Dead Neighbor Polling interval",
       "Seconds");

ALICMD (cmdFuncOspfNeighbor,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "neighbor A.B.C.D priority <0-255>",
       "Specify neighbor router",
       "Neighbor IP address",
       "Neighbor Priority",
       "Seconds");

DECMD (cmdFuncOspfNeighborPollInterval,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "neighbor A.B.C.D poll-interval <1-65535>",
       "Specify neighbor router",
       "Neighbor IP address",
       "Dead Neighbor Polling interval",
       "Seconds")
{
  struct ospf *ospf = om->index;
  struct in_addr nbr_addr;
  unsigned int priority = OSPF_NEIGHBOR_PRIORITY_DEFAULT;
  unsigned int interval = OSPF_POLL_INTERVAL_DEFAULT;

  VTY_GET_IPV4_ADDRESS ("neighbor address", nbr_addr, cargv[1]);

  if (cargc > 2)
    VTY_GET_INTEGER_RANGE ("poll interval", interval, cargv[3], 1, 65535);

  if (cargc > 4)
    VTY_GET_INTEGER_RANGE ("neighbor priority", priority, cargv[5], 0, 255);

  ospf_nbr_nbma_set (ospf, nbr_addr);
  if (cargc > 2)
    ospf_nbr_nbma_poll_interval_set (ospf, nbr_addr, interval);
  if (cargc > 4)
    ospf_nbr_nbma_priority_set (ospf, nbr_addr, priority);

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNeighborPollInterval,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "neighbor A.B.C.D poll-interval <1-65535> priority <0-255>",
       "Specify neighbor router",
       "Neighbor address",
       "OSPF dead-router polling interval",
       "Seconds",
       "OSPF priority of non-broadcast neighbor",
       "Priority");

DECMD (cmdFuncOspfno_ospf_neighbor,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no neighbor A.B.C.D",
       "Negate a command or set its defaults",
       "Specify neighbor router",
       "Neighbor IP address")
{
  struct ospf *ospf = om->index;
  struct in_addr nbr_addr;

  VTY_GET_IPV4_ADDRESS ("neighbor address", nbr_addr, cargv[2]);

  (void)ospf_nbr_nbma_unset (ospf, nbr_addr);

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfno_ospf_neighbor,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no neighbor A.B.C.D priority <0-255>",
       "Negate a command or set its defaults",
       "Specify neighbor router",
       "Neighbor IP address",
       "Neighbor Priority",
       "Priority");

ALICMD (cmdFuncOspfno_ospf_neighbor,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no neighbor A.B.C.D poll-interval <1-65535>",
       "Negate a command or set its defaults",
       "Specify neighbor router",
       "Neighbor IP address",
       "Dead Neighbor Polling interval",
       "Seconds");

ALICMD (cmdFuncOspfno_ospf_neighbor,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no neighbor A.B.C.D priority <0-255> poll-interval <1-65535>",
       "Negate a command or set its defaults",
       "Specify neighbor router",
       "Neighbor IP address",
       "Neighbor Priority",
       "Priority",
       "Dead Neighbor Polling interval",
       "Seconds");


DECMD (cmdFuncOspfRefreshTimer,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "refresh timer <10-1800>",
       "Adjust refresh parameters",
       "Set refresh timer",
       "Timer value in seconds")
{
  struct ospf *ospf = om->index;
  unsigned int interval;

  VTY_GET_INTEGER_RANGE ("refresh timer", interval, cargv[2], 10, 1800);
  interval = (interval / 10) * 10;

  ospf_timers_refresh_set (ospf, interval);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoOspfRefreshTimer,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no refresh timer <10-1800>",
       "Negate a command or set its defaults",
       "Adjust refresh parameters",
       "Unset refresh timer",
       "Timer value in seconds")
{
  struct ospf *ospf = om->index;
  unsigned int interval;

  if (cargc == 4)
    {
      VTY_GET_INTEGER_RANGE ("refresh timer", interval, cargv[3], 10, 1800);

      if (ospf->lsa_refresh_interval != interval ||
	  interval == OSPF_LSA_REFRESH_INTERVAL_DEFAULT)
	return (CMD_IPC_OK);
    }

  ospf_timers_refresh_unset (ospf);

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoOspfRefreshTimer,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no refresh timer",
       "Negate a command or set its defaults",
       "Adjust refresh parameters",
       "Unset refresh timer");

DECMD (cmdFuncOspfAutoCostReferenceBandwidth,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "auto-cost reference-bandwidth <1-4294967>",
       "Calculate OSPF interface cost according to bandwidth",
       "Use reference bandwidth method to assign OSPF cost",
       "The reference bandwidth in terms of Mbits per second")
{
  struct ospf *ospf = om->index;
  u_int32_t refbw;
  struct listnode *node;
  struct interface *ifp;

  refbw = strtol (cargv[2], NULL, 10);
  if (refbw < 1 || refbw > 4294967)
    {
      cmdPrint (cmsh, "reference-bandwidth value is invalid\n");
      return (CMD_IPC_WARNING);
    }

  /* If reference bandwidth is changed. */
  if ((refbw * 1000) == ospf->ref_bandwidth)
    return (CMD_IPC_OK);

  ospf->ref_bandwidth = refbw * 1000;
  for (ALL_LIST_ELEMENTS_RO (om->iflist, node, ifp))
    ospf_if_recalculate_output_cost (ifp);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoAutoCostReferenceBandwidth,
		CMD_NODE_CONFIG_OSPF,
		IPC_OSPF,
       "no auto-cost reference-bandwidth",
       "Negate a command or set its defaults",
       "Calculate OSPF interface cost according to bandwidth",
       "Use reference bandwidth method to assign OSPF cost")
{
  struct ospf *ospf = om->index;
  struct listnode *node, *nnode;
  struct interface *ifp;

  if (ospf->ref_bandwidth == OSPF_DEFAULT_REF_BANDWIDTH)
    return (CMD_IPC_OK);

  ospf->ref_bandwidth = OSPF_DEFAULT_REF_BANDWIDTH;
  cmdPrint (cmsh, "%% OSPF: Reference bandwidth is changed.\n");
  cmdPrint (cmsh, "        Please ensure reference bandwidth is consistent across all routers\n");

  for (ALL_LIST_ELEMENTS (om->iflist, node, nnode, ifp))
    ospf_if_recalculate_output_cost (ifp);

  return (CMD_IPC_OK);
}


DECMD (cmdFuncOspfShowIp,
		CMD_NODE_EXEC,
	   IPC_OSPF | IPC_SHOW_MGR,
       "show ip ospf",
       "Show running system information",
       "IP information",
       "OSPF information")
{
	struct listnode *node, *nnode;
	struct ospf_area * area;
	struct ospf *ospf;
	struct timeval result;
	char timebuf[OSPF_TIME_DUMP_SIZE];

	int idx = 0;
	char showBuff[1024] = { };

	/* Check OSPF is enable. */
	ospf = ospf_lookup();
	if (ospf == NULL) {
		cmdPrint(cmsh, " OSPF Routing Process not enabled");
		return (CMD_IPC_OK);
	}

	/* Show Router ID. */
	cmdPrint(cmsh, " OSPF Routing Process, Router ID: %s",
			inet_ntoa(ospf->router_id));

	/* Graceful shutdown */
	if (ospf->pTimerDeferredShutdown)
		cmdPrint(cmsh, " Deferred shutdown in progress, %s remaining",
				ospf_timer_dump(ospf->pTimerDeferredShutdown, ospf->tvUpdateDS, timebuf,
						sizeof(timebuf)));
	/* Show capability. */
	cmdPrint(cmsh, " Supports only single TOS (TOS0) routes");
	cmdPrint(cmsh, " This implementation conforms to RFC2328");
	cmdPrint(cmsh, " RFC1583Compatibility flag is %s",
			CHECK_FLAG (ospf->config, OSPF_RFC1583_COMPATIBLE) ? "enabled" : "disabled");
#ifdef HAVE_OPAQUE_LSA
	cmdPrint(cmsh, " OpaqueCapability flag is %s%s",
			CHECK_FLAG (ospf->config, OSPF_OPAQUE_CAPABLE) ? "enabled" : "disabled",
			IS_OPAQUE_LSA_ORIGINATION_BLOCKED (ospf->opaque) ? " (origination blocked)" : "");
#endif /* HAVE_OPAQUE_LSA */

	/* Show stub-router configuration */
	if (ospf->stub_router_startup_time != OSPF_STUB_ROUTER_UNCONFIGURED
			|| ospf->stub_router_shutdown_time != OSPF_STUB_ROUTER_UNCONFIGURED) {
		cmdPrint(cmsh, " Stub router advertisement is configured");
		if (ospf->stub_router_startup_time != OSPF_STUB_ROUTER_UNCONFIGURED)
			cmdPrint(cmsh, "   Enabled for %us after start-up",
					ospf->stub_router_startup_time);
		if (ospf->stub_router_shutdown_time != OSPF_STUB_ROUTER_UNCONFIGURED)
			cmdPrint(cmsh, "   Enabled for %us prior to full shutdown",
					ospf->stub_router_shutdown_time);
	}

	/* Show SPF timers. */
	cmdPrint(cmsh, " Initial SPF scheduling delay %d millisec(s)",
			ospf->spf_delay);
	cmdPrint(cmsh, " Minimum hold time between consecutive SPFs %d millisec(s)",
			ospf->spf_holdtime);
	cmdPrint(cmsh, " Maximum hold time between consecutive SPFs %d millisec(s)",
			ospf->spf_max_holdtime);
	cmdPrint(cmsh, " Hold time multiplier is currently %d",
			ospf->spf_hold_multiplier);
	idx += sprintf(showBuff + idx, " SPF algorithm ");
	if (ospf->ts_spf.tv_sec || ospf->ts_spf.tv_usec) {
		result = tv_sub(ospfRecentRelativeTime(), ospf->ts_spf);
		cmdPrint(cmsh, "%slast executed %s ago", showBuff,
				ospf_timeval_dump(&result, timebuf, sizeof(timebuf)));
	} else
		cmdPrint(cmsh, "%s has not been run",showBuff);
	idx = 0; memset(showBuff, 0, 1024);
	cmdPrint(cmsh, " SPF timer %s%s", (ospf->pTimerSpfCalc ? "due in " : "is "),
			ospf_timer_dump(ospf->pTimerSpfCalc, ospf->tvUpdateSpfCalc, timebuf, sizeof(timebuf)));
	idx = 0;	memset(showBuff, 0, 1024);

	/* Show refresh parameters. */
	cmdPrint(cmsh, " Refresh timer %d secs", ospf->lsa_refresh_interval);

	/* Show ABR/ASBR flags. */
	if (CHECK_FLAG(ospf->flags, OSPF_FLAG_ABR))
		cmdPrint(cmsh, " This router is an ABR, ABR type is: %s",
				ospf_abr_type_descr_str[ospf->abr_type]);

	if (CHECK_FLAG(ospf->flags, OSPF_FLAG_ASBR))
		cmdPrint(cmsh, " This router is an ASBR "
				"(injecting external routing information)");

	/* Show Number of AS-external-LSAs. */
	cmdPrint(cmsh, " Number of external LSA %ld. Checksum Sum 0x%08x",
			ospf_lsdb_count (ospf->lsdb, OSPF_AS_EXTERNAL_LSA),
			ospf_lsdb_checksum (ospf->lsdb, OSPF_AS_EXTERNAL_LSA));
#ifdef HAVE_OPAQUE_LSA
	cmdPrint(cmsh, " Number of opaque AS LSA %ld. Checksum Sum 0x%08x",
			ospf_lsdb_count (ospf->lsdb, OSPF_OPAQUE_AS_LSA),
			ospf_lsdb_checksum (ospf->lsdb, OSPF_OPAQUE_AS_LSA));
#endif /* HAVE_OPAQUE_LSA */
	/* Show number of areas attached. */
	cmdPrint(cmsh, " Number of areas attached to this router: %d",
			listcount (ospf->areas));

	if (CHECK_FLAG(ospf->config, OSPF_LOG_ADJACENCY_CHANGES)) {
		if (CHECK_FLAG(ospf->config, OSPF_LOG_ADJACENCY_DETAIL)) {
			cmdPrint(cmsh, " All adjacency changes are logged");
		} else {
			cmdPrint(cmsh, " Adjacency changes are logged");
		}
	}

	cmdPrint(cmsh, " ");

	/* Show each area status. */
	for (ALL_LIST_ELEMENTS(ospf->areas, node, nnode, area))
		show_ip_ospf_area(cmsh, area);

	return (CMD_IPC_OK);
}



DECMD (cmdFuncOspfShowIpInterface,
		CMD_NODE_EXEC,
	   IPC_OSPF | IPC_SHOW_MGR,
       "show ip ospf interface",
       "Show running system information",
       "IP information",
       "OSPF information",
       "Interface information")
{
	struct interface *ifp;
	struct ospf *ospf;
	struct listnode *node;

	ospf = ospf_lookup();
	if (ospf == NULL) {
		cmdPrint(cmsh, "OSPF Routing Process not enabled");
		return (CMD_IPC_OK);
	}

	/* Show All Interfaces. */
	if (cargc == 4)
		for (ALL_LIST_ELEMENTS_RO(iflist, node, ifp))
			show_ip_ospf_interface_sub(cmsh, ospf, ifp);
	/* Interface name is specified. */
	else {
		if ((ifp = if_lookup_by_name(cargv[4])) == NULL) {
			cmdPrint(cmsh, "No such interface name");
		} else {
			show_ip_ospf_interface_sub(cmsh, ospf, ifp);
		}
	}

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfShowIpInterface,
		CMD_NODE_EXEC,
	   IPC_OSPF | IPC_SHOW_MGR,
       "show ip ospf interface WORD",
       "Show running system information",
       "IP information",
       "OSPF information",
       "Interface information",
       "Interface name");


DECMD (cmdFuncOspfShowIpNeighbor,
		CMD_NODE_EXEC,
	   IPC_OSPF | IPC_SHOW_MGR,
       "show ip ospf neighbor",
       "Show running system information",
       "IP information",
       "OSPF information",
       "Neighbor list")
{
  struct ospf *ospf;
  struct ospf_interface *oi;
  struct listnode *node;

  ospf = ospf_lookup ();
  if (ospf == NULL)
    {
      cmdPrint (cmsh, " OSPF Routing Process not enabled");
      return (CMD_IPC_OK);
    }

  show_ip_ospf_neighbour_header (cmsh);

  for (ALL_LIST_ELEMENTS_RO (ospf->oiflist, node, oi))
    show_ip_ospf_neighbor_sub (cmsh, oi);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfShowIpNeighborAll,
		CMD_NODE_EXEC,
	   IPC_OSPF | IPC_SHOW_MGR,
       "show ip ospf neighbor all",
       "Show running system information",
       "IP information",
       "OSPF information",
       "Neighbor list",
       "include down status neighbor")
{
	struct ospf *ospf = ospf_lookup();
	struct listnode *node;
	struct ospf_interface *oi;

	if (ospf == NULL) {
		cmdPrint(cmsh, " OSPF Routing Process not enabled");
		return (CMD_IPC_OK);
	}

	show_ip_ospf_neighbour_header(cmsh);

	for (ALL_LIST_ELEMENTS_RO(ospf->oiflist, node, oi)) {
		struct listnode *nbr_node;
		struct ospf_nbr_nbma *nbr_nbma;

		show_ip_ospf_neighbor_sub(cmsh, oi);

		/* print Down neighbor status */
		for (ALL_LIST_ELEMENTS_RO(oi->nbr_nbma, nbr_node, nbr_nbma)) {
			if (nbr_nbma->nbr == NULL || nbr_nbma->nbr->state == NSM_Down)
				cmdPrint(cmsh, "%-15s %3d %-15s %9s %-15s %-20s %5d %5d %5d",
						"-", nbr_nbma->priority, "Down", "-",
						inet_ntoa(nbr_nbma->addr), IF_NAME (oi), 0, 0, 0);
		}
	}

	return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfShowIpNeighborInt,
		CMD_NODE_EXEC,
	   IPC_OSPF | IPC_SHOW_MGR,
       "show ip ospf neighbor WORD",
       "Show running system information",
       "IP information",
       "OSPF information",
       "Neighbor list",
       "Interface name")
{
  struct ospf *ospf;
  struct interface *ifp;
  struct route_node *rn;

  ifp = if_lookup_by_name (cargv[4]);
  if (!ifp)
    {
      cmdPrint (cmsh, "No such interface.");
      return (CMD_IPC_WARNING);
    }

  ospf = ospf_lookup ();
  if (ospf == NULL)
    {
      cmdPrint (cmsh, " OSPF Routing Process not enabled");
      return (CMD_IPC_OK);
    }

  show_ip_ospf_neighbour_header (cmsh);

  for (rn = route_top (IF_OIFS (ifp)); rn; rn = route_next (rn))
    {
      struct ospf_interface *oi = rn->info;

      if (oi == NULL)
	continue;

      show_ip_ospf_neighbor_sub (cmsh, oi);
    }

  return (CMD_IPC_OK);
}


DECMD (cmdFuncOspfShowIpNeighborId,
		CMD_NODE_EXEC,
	   IPC_OSPF | IPC_SHOW_MGR,
       "show ip ospf neighbor A.B.C.D",
       "Show running system information",
       "IP information",
       "OSPF information",
       "Neighbor list",
       "Neighbor ID")
{
  struct ospf *ospf;
  struct listnode *node;
  struct ospf_neighbor *nbr;
  struct ospf_interface *oi;
  struct in_addr router_id;
  int ret;

  ret = inet_aton (cargv[4], &router_id);
  if (!ret)
    {
      cmdPrint (cmsh, "Please specify Neighbor ID by A.B.C.D");
      return (CMD_IPC_WARNING);
    }

  ospf = ospf_lookup ();
  if (ospf == NULL)
    {
      cmdPrint (cmsh, " OSPF Routing Process not enabled");
      return (CMD_IPC_OK);
    }

  for (ALL_LIST_ELEMENTS_RO (ospf->oiflist, node, oi))
    if ((nbr = ospf_nbr_lookup_by_routerid (oi->nbrs, &router_id)))
      show_ip_ospf_neighbor_detail_sub (cmsh, oi, nbr);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfShowIpNeighborDetail,
		CMD_NODE_EXEC,
	   IPC_OSPF | IPC_SHOW_MGR,
       "show ip ospf neighbor detail",
       "Show running system information",
       "IP information",
       "OSPF information",
       "Neighbor list",
       "detail of all neighbors")
{
  struct ospf *ospf;
  struct ospf_interface *oi;
  struct listnode *node;

  ospf = ospf_lookup ();
  if (ospf == NULL)
    {
      cmdPrint (cmsh, " OSPF Routing Process not enabled");
      return (CMD_IPC_OK);
    }

  for (ALL_LIST_ELEMENTS_RO (ospf->oiflist, node, oi))
    {
      struct route_node *rn;
      struct ospf_neighbor *nbr;

      for (rn = route_top (oi->nbrs); rn; rn = route_next (rn))
	if ((nbr = rn->info))
	  if (nbr != oi->nbr_self)
	    if (nbr->state != NSM_Down)
	      show_ip_ospf_neighbor_detail_sub (cmsh, oi, nbr);
    }

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfShowIpNeighborDetailAll,
		CMD_NODE_EXEC,
	   IPC_OSPF | IPC_SHOW_MGR,
       "show ip ospf neighbor detail all",
       "Show running system information",
       "IP information",
       "OSPF information",
       "Neighbor list",
       "detail of all neighbors",
       "include down status neighbor")
{
  struct ospf *ospf;
  struct listnode *node;
  struct ospf_interface *oi;

  ospf = ospf_lookup ();
  if (ospf == NULL)
    {
      cmdPrint (cmsh, " OSPF Routing Process not enabled");
      return (CMD_IPC_OK);
    }

  for (ALL_LIST_ELEMENTS_RO (ospf->oiflist, node, oi))
    {
      struct route_node *rn;
      struct ospf_neighbor *nbr;
      struct ospf_nbr_nbma *nbr_nbma;

      for (rn = route_top (oi->nbrs); rn; rn = route_next (rn))
	if ((nbr = rn->info))
	  if (nbr != oi->nbr_self)
	    if (oi->type == OSPF_IFTYPE_NBMA && nbr->state != NSM_Down)
	      show_ip_ospf_neighbor_detail_sub (cmsh, oi, rn->info);

      if (oi->type == OSPF_IFTYPE_NBMA)
	{
	  struct listnode *nd;

	  for (ALL_LIST_ELEMENTS_RO (oi->nbr_nbma, nd, nbr_nbma))
            if (nbr_nbma->nbr == NULL
                || nbr_nbma->nbr->state == NSM_Down)
              show_ip_ospf_nbr_nbma_detail_sub (cmsh, oi, nbr_nbma);
	}
    }

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfShowIpNeighborIntDetail,
		CMD_NODE_EXEC,
	   IPC_OSPF | IPC_SHOW_MGR,
       "show ip ospf neighbor WORD detail",
       "Show running system information",
       "IP information",
       "OSPF information",
       "Neighbor list",
       "Interface name",
       "detail of all neighbors")
{
  struct ospf *ospf;
  struct ospf_interface *oi;
  struct interface *ifp;
  struct route_node *rn, *nrn;
  struct ospf_neighbor *nbr;

  ifp = if_lookup_by_name (cargv[4]);
  if (!ifp)
    {
      cmdPrint (cmsh, "No such interface.");
      return (CMD_IPC_WARNING);
    }

  ospf = ospf_lookup ();
  if (ospf == NULL)
    {
      cmdPrint (cmsh, " OSPF Routing Process not enabled");
      return (CMD_IPC_OK);
    }


  for (rn = route_top (IF_OIFS (ifp)); rn; rn = route_next (rn))
    if ((oi = rn->info))
      for (nrn = route_top (oi->nbrs); nrn; nrn = route_next (nrn))
	if ((nbr = nrn->info))
	  if (nbr != oi->nbr_self)
	    if (nbr->state != NSM_Down)
	      show_ip_ospf_neighbor_detail_sub (cmsh, oi, nbr);

  return (CMD_IPC_OK);
}



DECMD (cmdFuncOspfShowIpDatabase,
		CMD_NODE_EXEC,
	   IPC_OSPF | IPC_SHOW_MGR,
       "show ip ospf database",
       "Show running system information",
       "IP information",
       "OSPF information",
       "Database summary")
{
  struct ospf *ospf;
  int type, ret;
  struct in_addr id, adv_router;

  ospf = ospf_lookup ();
  if (ospf == NULL)
    {
      cmdPrint (cmsh, " OSPF Routing Process not enabled");
      return (CMD_IPC_OK);
    }

  cmdPrint (cmsh, " ");
  cmdPrint (cmsh, "       OSPF Router with ID (%s)",
           inet_ntoa (ospf->router_id));
  cmdPrint (cmsh, " ");

  /* Show all LSA. */
  if (cargc == 4)
    {
      show_ip_ospf_database_summary (cmsh, ospf, 0);
      return (CMD_IPC_OK);
    }

  /* Set database type to show. */
  if (strncmp (cargv[4], "r", 1) == 0)
    type = OSPF_ROUTER_LSA;
  else if (strncmp (cargv[4], "ne", 2) == 0)
    type = OSPF_NETWORK_LSA;
  else if (strncmp (cargv[4], "ns", 2) == 0)
    type = OSPF_AS_NSSA_LSA;
  else if (strncmp (cargv[4], "su", 2) == 0)
    type = OSPF_SUMMARY_LSA;
  else if (strncmp (cargv[4], "a", 1) == 0)
    type = OSPF_ASBR_SUMMARY_LSA;
  else if (strncmp (cargv[4], "e", 1) == 0)
    type = OSPF_AS_EXTERNAL_LSA;
  else if (strncmp (cargv[4], "se", 2) == 0)
    {
      show_ip_ospf_database_summary (cmsh, ospf, 1);
      return (CMD_IPC_OK);
    }
  else if (strncmp (cargv[4], "m", 1) == 0)
    {
      show_ip_ospf_database_maxage (cmsh, ospf);
      return (CMD_IPC_OK);
    }
#ifdef HAVE_OPAQUE_LSA
  else if (strncmp (cargv[4], "opaque-l", 8) == 0)
    type = OSPF_OPAQUE_LINK_LSA;
  else if (strncmp (cargv[4], "opaque-ar", 9) == 0)
    type = OSPF_OPAQUE_AREA_LSA;
  else if (strncmp (cargv[4], "opaque-as", 9) == 0)
    type = OSPF_OPAQUE_AS_LSA;
#endif /* HAVE_OPAQUE_LSA */
  else
    return (CMD_IPC_WARNING);

  /* `show ip ospf database LSA'. */
  if (cargc == 5)
    show_lsa_detail (cmsh, ospf, type, NULL, NULL);
  else if (cargc >= 6)
    {
      ret = inet_aton (cargv[5], &id);
      if (!ret)
	return (CMD_IPC_WARNING);

      /* `show ip ospf database LSA ID'. */
      if (cargc == 6)
	show_lsa_detail (cmsh, ospf, type, &id, NULL);
      /* `show ip ospf database LSA ID adv-router ADV_ROUTER'. */
      else if (cargc >= 7)
	{
	  if (strncmp (cargv[6], "s", 1) == 0)
	    adv_router = ospf->router_id;
	  else
	    {
	      ret = inet_aton (cargv[7], &adv_router);
	      if (!ret)
		return (CMD_IPC_WARNING);
	    }
	  show_lsa_detail (cmsh, ospf, type, &id, &adv_router);
	}
    }

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfShowIpDatabase,
		CMD_NODE_EXEC,
	   IPC_OSPF | IPC_SHOW_MGR,
       "show ip ospf database ("
	   "asbr-summary|external|network|router|summary"
		"|nssa-external"
		"|opaque-link|opaque-area|opaque-as"
	   "|max-age|self-originate)",
       "Show running system information",
       "IP information",
       "OSPF information",
       "Database summary",
       "ASBR summary link states",
	   "External link states",
	   "Network link states",
	   "Router link states",
	   "Network summary link states",
	   "NSSA external link state",
	   "Link local Opaque-LSA",
	   "Link area Opaque-LSA" ,
	   "Link AS Opaque-LSA"   ,
       "LSAs in MaxAge list",
       "Self-originated link states");

ALICMD (cmdFuncOspfShowIpDatabase,
		CMD_NODE_EXEC,
	   IPC_OSPF | IPC_SHOW_MGR,
       "show ip ospf database ("
	   "asbr-summary|external|network|router|summary"
		"|nssa-external"
		"|opaque-link|opaque-area|opaque-as"
	   ") A.B.C.D",
       "Show running system information",
       "IP information",
       "OSPF information",
       "Database summary",
       "ASBR summary link states",
	   "External link states",
	   "Network link states",
	   "Router link states",
	   "Network summary link states",
	   "NSSA external link state",
	   "Link local Opaque-LSA",
	   "Link area Opaque-LSA" ,
	   "Link AS Opaque-LSA"   ,
       "Link State ID (as an IP address)");

ALICMD (cmdFuncOspfShowIpDatabase,
		CMD_NODE_EXEC,
	   IPC_OSPF | IPC_SHOW_MGR,
       "show ip ospf database ("
	   "asbr-summary|external|network|router|summary"
		"|nssa-external"
		"|opaque-link|opaque-area|opaque-as"
	   ") A.B.C.D adv-router A.B.C.D",
       "Show running system information",
       "IP information",
       "OSPF information",
       "Database summary",
       "ASBR summary link states",
	   "External link states",
	   "Network link states",
	   "Router link states",
	   "Network summary link states",
	   "NSSA external link state",
	   "Link local Opaque-LSA",
	   "Link area Opaque-LSA" ,
	   "Link AS Opaque-LSA"   ,
       "Link State ID (as an IP address)",
       "Advertising Router link states",
       "Advertising Router (as an IP address)");

ALICMD (cmdFuncOspfShowIpDatabase,
		CMD_NODE_EXEC,
	   IPC_OSPF | IPC_SHOW_MGR,
       "show ip ospf database ("
	   "asbr-summary|external|network|router|summary"
		"|nssa-external"
		"|opaque-link|opaque-area|opaque-as"
	   ") A.B.C.D (self-originate|)",
       "Show running system information",
       "IP information",
       "OSPF information",
       "Database summary",
       "ASBR summary link states",
	   "External link states",
	   "Network link states",
	   "Router link states",
	   "Network summary link states",
	   "NSSA external link state",
	   "Link local Opaque-LSA",
	   "Link area Opaque-LSA" ,
	   "Link AS Opaque-LSA"   ,
       "Link State ID (as an IP address)",
       "Self-originated link states");

DECMD (cmdFuncOspfShowIpDatabaseTypeAdvRouter,
		CMD_NODE_EXEC,
	   IPC_OSPF | IPC_SHOW_MGR,
       "show ip ospf database ("
	   "asbr-summary|external|network|router|summary"
		"|nssa-external"
		"|opaque-link|opaque-area|opaque-as"
	   ") adv-router A.B.C.D",
       "Show running system information",
       "IP information",
       "OSPF information",
       "Database summary",
       "ASBR summary link states",
	   "External link states",
	   "Network link states",
	   "Router link states",
	   "Network summary link states",
	   "NSSA external link state",
	   "Link local Opaque-LSA",
	   "Link area Opaque-LSA" ,
	   "Link AS Opaque-LSA"   ,
       "Advertising Router link states",
       "Advertising Router (as an IP address)")
{
  struct ospf *ospf;
  int type, ret;
  struct in_addr adv_router;

  ospf = ospf_lookup ();
  if (ospf == NULL)
    {
      cmdPrint (cmsh, " OSPF Routing Process not enabled");
      return (CMD_IPC_OK);
    }

  cmdPrint (cmsh, "\n       OSPF Router with ID (%s)\n",
           inet_ntoa (ospf->router_id));

  if (cargc != 6 && cargc != 7)
    return (CMD_IPC_WARNING);

  /* Set database type to show. */
  if (strncmp (cargv[4], "r", 1) == 0)
    type = OSPF_ROUTER_LSA;
  else if (strncmp (cargv[4], "ne", 2) == 0)
    type = OSPF_NETWORK_LSA;
  else if (strncmp (cargv[4], "ns", 2) == 0)
    type = OSPF_AS_NSSA_LSA;
  else if (strncmp (cargv[4], "s", 1) == 0)
    type = OSPF_SUMMARY_LSA;
  else if (strncmp (cargv[4], "a", 1) == 0)
    type = OSPF_ASBR_SUMMARY_LSA;
  else if (strncmp (cargv[4], "e", 1) == 0)
    type = OSPF_AS_EXTERNAL_LSA;
#ifdef HAVE_OPAQUE_LSA
  else if (strncmp (cargv[4], "opaque-l", 8) == 0)
    type = OSPF_OPAQUE_LINK_LSA;
  else if (strncmp (cargv[4], "opaque-ar", 9) == 0)
    type = OSPF_OPAQUE_AREA_LSA;
  else if (strncmp (cargv[4], "opaque-as", 9) == 0)
    type = OSPF_OPAQUE_AS_LSA;
#endif /* HAVE_OPAQUE_LSA */
  else
    return (CMD_IPC_WARNING);

  /* `show ip ospf database LSA adv-router ADV_ROUTER'. */
  if (strncmp (cargv[5], "s", 1) == 0)
    adv_router = ospf->router_id;
  else
    {
      ret = inet_aton (cargv[6], &adv_router);
      if (!ret)
	return (CMD_IPC_WARNING);
    }

  show_lsa_detail_adv_router (cmsh, ospf, type, &adv_router);

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfShowIpDatabaseTypeAdvRouter,
		CMD_NODE_EXEC,
	   IPC_OSPF | IPC_SHOW_MGR,
       "show ip ospf database ("
	   "asbr-summary|external|network|router|summary"
		"|nssa-external"
		"|opaque-link|opaque-area|opaque-as"
	   ") (self-originate|)",
       "Show running system information",
       "IP information",
       "OSPF information",
       "Database summary",
       "ASBR summary link states",
	   "External link states",
	   "Network link states",
	   "Router link states",
	   "Network summary link states",
	   "NSSA external link state",
	   "Link local Opaque-LSA",
	   "Link area Opaque-LSA" ,
	   "Link AS Opaque-LSA"   ,
       "Self-originated link states");


DECMD (cmdFuncOspfIpAuthenticationArgs,
       CMD_NODE_INTERFACE,
       IPC_OSPF,
       "ip ospf authentication (null|message-digest) A.B.C.D",
       "IP Information",
       "OSPF interface commands",
       "Enable authentication on this interface",
       "Use null authentication",
       "Use message-digest authentication",
       "Address of interface")
{
  struct interface *ifp;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  params = IF_DEF_PARAMS (ifp);

  if (cargc == 5)
    {
      ret = inet_aton(cargv[4], &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D\n");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  /* Handle null authentication */
  if ( cargv[3][0] == 'n' )
    {
      SET_IF_PARAM (params, auth_type);
      params->auth_type = OSPF_AUTH_NULL;
      return (CMD_IPC_OK);
    }

  /* Handle message-digest authentication */
  if ( cargv[3][0] == 'm' )
    {
      SET_IF_PARAM (params, auth_type);
      params->auth_type = OSPF_AUTH_CRYPTOGRAPHIC;
      return (CMD_IPC_OK);
    }

  cmdPrint (cmsh, "You shouldn't get here!\n");
  return (CMD_IPC_WARNING);
}

ALICMD (cmdFuncOspfIpAuthenticationArgs,
	   CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf authentication (null|message-digest)",
       "IP Information",
       "OSPF interface commands",
       "Enable authentication on this interface",
       "Use null authentication",
       "Use message-digest authentication");

DECMD (cmdFuncOspfIpAuthentication,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf authentication A.B.C.D",
       "IP Information",
       "OSPF interface commands",
       "Enable authentication on this interface",
       "Address of interface")
{
  struct interface *ifp;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  params = IF_DEF_PARAMS (ifp);

  if (cargc == 4)
    {
      ret = inet_aton(cargv[3], &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D\n");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  SET_IF_PARAM (params, auth_type);
  params->auth_type = OSPF_AUTH_SIMPLE;

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfIpAuthentication,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf authentication",
       "IP Information",
       "OSPF interface commands",
       "Enable authentication on this interface");

DECMD (cmdFuncOspfNoIpAuthentication,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf authentication A.B.C.D",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Enable authentication on this interface",
       "Address of interface")
{
  struct interface *ifp;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  params = IF_DEF_PARAMS (ifp);

  if (cargc == 5)
    {
      ret = inet_aton(cargv[4], &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D\n");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_lookup_if_params (ifp, addr);
      if (params == NULL)
	return (CMD_IPC_OK);
    }

  params->auth_type = OSPF_AUTH_NOTSET;
  UNSET_IF_PARAM (params, auth_type);

  if (params != IF_DEF_PARAMS (ifp))
    {
      ospf_free_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoIpAuthentication,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf authentication",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Enable authentication on this interface");

DECMD (cmdFuncOspfIpAuthenticationKey,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf authentication-key WORD A.B.C.D",
       "IP Information",
       "OSPF interface commands",
       "Authentication password (key)",
       "The OSPF password (key)",
       "Address of interface")
{
  struct interface *ifp;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  params = IF_DEF_PARAMS (ifp);

  if (cargc == 5)
    {
      ret = inet_aton(cargv[4], &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D\n");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }


  memset (params->auth_simple, 0, OSPF_AUTH_SIMPLE_SIZE + 1);
  strncpy ((char *) params->auth_simple,
		  cargc == 3 ? cargv[2] : cargv[3],
				  OSPF_AUTH_SIMPLE_SIZE);
  SET_IF_PARAM (params, auth_simple);

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfIpAuthenticationKey,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf authentication-key WORD",
       "IP Information",
       "OSPF interface commands",
       "Authentication password (key)",
       "The OSPF password (key)");

ALICMD (cmdFuncOspfIpAuthenticationKey,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ospf authentication-key WORD",
       "OSPF interface commands",
       "Authentication password (key)",
       "The OSPF password (key)");

DECMD (cmdFuncOspfNoIpAuthenticationKey,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf authentication-key A.B.C.D",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Authentication password (key)",
       "Address of interface")
{
  struct interface *ifp;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  params = IF_DEF_PARAMS (ifp);

  if (cargc == 5)
    {
      ret = inet_aton(cargv[4], &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D\n");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_lookup_if_params (ifp, addr);
      if (params == NULL)
	return (CMD_IPC_OK);
    }

  memset (params->auth_simple, 0, OSPF_AUTH_SIMPLE_SIZE);
  UNSET_IF_PARAM (params, auth_simple);

  if (params != IF_DEF_PARAMS (ifp))
    {
      ospf_free_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoIpAuthenticationKey,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf authentication-key",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Authentication password (key)");

ALICMD (cmdFuncOspfNoIpAuthenticationKey,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ospf authentication-key",
       "Negate a command or set its defaults",
       "OSPF interface commands",
       "Authentication password (key)");

DECMD (cmdFuncOspfIpMessageDigestKey,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf message-digest-key <1-255> md5 WORD A.B.C.D",
       "IP Information",
       "OSPF interface commands",
       "Message digest authentication password (key)",
       "Key ID",
       "Use MD5 algorithm",
       "The OSPF password (key)",
       "Address of interface")
{
  struct interface *ifp;
  struct crypt_key *ck;
  u_char key_id;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  params = IF_DEF_PARAMS (ifp);

  if (cargc == 7)
    {
      ret = inet_aton(cargv[6], &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  key_id = strtol (cargc == 5 ? cargv[2] : cargv[3], NULL, 10);
  if (ospf_crypt_key_lookup (params->auth_crypt, key_id) != NULL)
    {
      cmdPrint (cmsh, "OSPF: Key %d already exists\n", key_id);
      return (CMD_IPC_WARNING);
    }

  ck = ospf_crypt_key_new ();
  ck->key_id = (u_char) key_id;
  memset (ck->auth_key, 0, OSPF_AUTH_MD5_SIZE+1);
  strncpy ((char *) ck->auth_key, cargc == 5 ? cargv[4] : cargv[5], OSPF_AUTH_MD5_SIZE);

  ospf_crypt_key_add (params->auth_crypt, ck);
  SET_IF_PARAM (params, auth_crypt);

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfIpMessageDigestKey,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf message-digest-key <1-255> md5 WORD",
       "IP Information",
       "OSPF interface commands",
       "Message digest authentication password (key)",
       "Key ID",
       "Use MD5 algorithm",
       "The OSPF password (key)");

ALICMD (cmdFuncOspfIpMessageDigestKey,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ospf message-digest-key <1-255> md5 WORD",
       "OSPF interface commands",
       "Message digest authentication password (key)",
       "Key ID",
       "Use MD5 algorithm",
       "The OSPF password (key)");

DECMD (cmdFuncOspfNoIpMessageDigestKey,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf message-digest-key <1-255> A.B.C.D",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Message digest authentication password (key)",
       "Key ID",
       "Address of interface")
{
  struct interface *ifp;
  struct crypt_key *ck;
  int key_id;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  params = IF_DEF_PARAMS (ifp);

  if (cargc == 6)
    {
      ret = inet_aton(cargv[5], &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_lookup_if_params (ifp, addr);
      if (params == NULL)
	return (CMD_IPC_OK);
    }

  key_id = strtol (cargc == 4 ? cargv[3] : cargv[4], NULL, 10);
  ck = ospf_crypt_key_lookup (params->auth_crypt, key_id);
  if (ck == NULL)
    {
      cmdPrint (cmsh, "OSPF: Key %d does not exist", key_id);
      return (CMD_IPC_WARNING);
    }

  ospf_crypt_key_delete (params->auth_crypt, key_id);

  if (params != IF_DEF_PARAMS (ifp))
    {
      ospf_free_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoIpMessageDigestKey,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf message-digest-key <1-255>",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Message digest authentication password (key)",
       "Key ID");

ALICMD (cmdFuncOspfNoIpMessageDigestKey,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ospf message-digest-key <1-255>",
       "Negate a command or set its defaults",
       "OSPF interface commands",
       "Message digest authentication password (key)",
       "Key ID");

DECMD (cmdFuncOspfIpCost,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf cost <1-65535> A.B.C.D",
       "IP Information",
       "OSPF interface commands",
       "Interface cost",
       "Cost",
       "Address of interface")
{
  struct interface *ifp ;
  u_int32_t cost;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  params = IF_DEF_PARAMS (ifp);

  cost = strtol (cargv[0][0] == 'i' ? cargv[3] : cargv[2], NULL, 10);

  /* cost range is <1-65535>. */
  if (cost < 1 || cost > 65535)
    {
      cmdPrint (cmsh, "Interface output cost is invalid\n");
      return (CMD_IPC_WARNING);
    }

  if (cargc == 5 || (cargc == 4 && cargv[0][0] == 'o' ))
    {
      ret = inet_aton(cargc == 5 ? cargv[4] : cargv[3], &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D\n");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  SET_IF_PARAM (params, output_cost_cmd);
  params->output_cost_cmd = cost;

  ospf_if_recalculate_output_cost (ifp);

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfIpCost,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf cost <1-65535>",
       "IP Information",
       "OSPF interface commands",
       "Interface cost",
       "Cost");

ALICMD (cmdFuncOspfIpCost,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ospf cost <1-65535>",
       "OSPF interface commands",
       "Interface cost",
       "Cost");

ALICMD (cmdFuncOspfIpCost,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ospf cost <1-65535> A.B.C.D",
       "OSPF interface commands",
       "Interface cost",
       "Cost",
       "Address of interface");

DECMD (cmdFuncOspfNoIpCost,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf cost A.B.C.D",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Interface cost",
       "Address of interface")
{
  struct interface *ifp;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  params = IF_DEF_PARAMS (ifp);

  if (cargc == 5 || (cargc == 4 && cargv[0][0] == 'o' ))
    {
      ret = inet_aton(cargc == 5 ? cargv[4] : cargv[3], &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_lookup_if_params (ifp, addr);
      if (params == NULL)
	return (CMD_IPC_OK);
    }

  UNSET_IF_PARAM (params, output_cost_cmd);

  if (params != IF_DEF_PARAMS (ifp))
    {
      ospf_free_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  ospf_if_recalculate_output_cost (ifp);

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoIpCost,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf cost",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Interface cost");

ALICMD (cmdFuncOspfNoIpCost,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ospf cost",
       "Negate a command or set its defaults",
       "OSPF interface commands",
       "Interface cost");

ALICMD (cmdFuncOspfNoIpCost,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ospf cost A.B.C.D",
       "Negate a command or set its defaults",
       "OSPF interface commands",
       "Interface cost",
       "Address of interface");

DECMD (cmdFuncOspfNoIpCost2,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf cost <1-65535>",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Interface cost",
       "Cost")
{
  struct interface *ifp;
  struct in_addr addr;
  u_int32_t cost;
  int ret;
  struct ospf_if_params *params;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  params = IF_DEF_PARAMS (ifp);

  /* According to the semantics we are mimicking "no ip ospf cost N" is
   * always treated as "no ip ospf cost" regardless of the actual value
   * of N already configured for the interface. Thus the first argument
   * is always checked to be a number, but is ignored after that.
   */
  cost = strtol (cargv[1][0] == 'i' ? cargv[4] : cargv[3], NULL, 10);
  if (cost < 1 || cost > 65535)
    {
      cmdPrint (cmsh, "Interface output cost is invalid");
      return (CMD_IPC_WARNING);
    }

  if (cargc == 6 || (cargc == 5 && cargv[1][0] == 'o' ))
    {
      ret = inet_aton(cargc == 6 ? cargv[5] : cargv[4], &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D\n");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_lookup_if_params (ifp, addr);
      if (params == NULL)
	return (CMD_IPC_OK);
    }

  UNSET_IF_PARAM (params, output_cost_cmd);

  if (params != IF_DEF_PARAMS (ifp))
    {
      ospf_free_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  ospf_if_recalculate_output_cost (ifp);

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoIpCost2,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ospf cost <1-65535>",
       "Negate a command or set its defaults",
       "OSPF interface commands",
       "Interface cost",
       "Cost");

ALICMD (cmdFuncOspfNoIpCost2,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf cost <1-65535> A.B.C.D",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Interface cost",
       "Cost",
       "Address of interface");

ALICMD (cmdFuncOspfNoIpCost2,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ospf cost <1-65535> A.B.C.D",
       "Negate a command or set its defaults",
       "OSPF interface commands",
       "Interface cost",
       "Cost",
       "Address of interface");


DECMD (cmdFuncOspfIpDeadInterval,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf dead-interval <1-65535> A.B.C.D",
       "IP Information",
       "OSPF interface commands",
       "Interval after which a neighbor is declared dead",
       "Seconds",
       "Address of interface")
{
  struct interface *ifp;
	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  if (cargc == 5)
    return ospf_vty_dead_interval_set (ifp, cmsh, cargv[3], cargv[4], NULL);
  else
    return ospf_vty_dead_interval_set (ifp, cmsh, cargc == 3 ? cargv[2] : cargv[3], NULL, NULL);
}

ALICMD (cmdFuncOspfIpDeadInterval,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf dead-interval <1-65535>",
       "IP Information",
       "OSPF interface commands",
       "Interval after which a neighbor is declared dead",
       "Seconds");

ALICMD (cmdFuncOspfIpDeadInterval,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ospf dead-interval <1-65535>",
       "OSPF interface commands",
       "Interval after which a neighbor is declared dead",
       "Seconds");

DECMD (cmdFuncOspfIpDeadIntervalMinimal,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf dead-interval minimal hello-multiplier <1-10> A.B.C.D",
       "IP Information",
       "OSPF interface commands",
       "Interval after which a neighbor is declared dead",
       "Minimal 1s dead-interval with fast sub-second hellos",
       "Hello multiplier factor",
       "Number of Hellos to send each second",
       "Address of interface")
{
  struct interface *ifp;
	ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  if (cargc == 7)
    return ospf_vty_dead_interval_set (ifp, cmsh, NULL, cargv[6], cargv[5]);
  else
    return ospf_vty_dead_interval_set (ifp, cmsh, NULL, NULL, cargv[5]);
}

ALICMD (cmdFuncOspfIpDeadIntervalMinimal,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf dead-interval minimal hello-multiplier <1-10>",
       "IP Information",
       "OSPF interface commands",
       "Interval after which a neighbor is declared dead",
       "Minimal 1s dead-interval with fast sub-second hellos",
       "Hello multiplier factor",
       "Number of Hellos to send each second");

DECMD (cmdFuncOspfNoIpDeadInterval,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf dead-interval A.B.C.D",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Interval after which a neighbor is declared dead",
       "Address of interface")
{
  struct interface *ifp;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
  struct ospf_interface *oi;
  struct route_node *rn;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  params = IF_DEF_PARAMS (ifp);

  if (cargc == 5)
    {
      ret = inet_aton(cargv[4], &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D\n");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_lookup_if_params (ifp, addr);
      if (params == NULL)
	return (CMD_IPC_OK);
    }

  UNSET_IF_PARAM (params, v_wait);
  params->v_wait = OSPF_ROUTER_DEAD_INTERVAL_DEFAULT;

  UNSET_IF_PARAM (params, fast_hello);
  params->fast_hello = OSPF_FAST_HELLO_DEFAULT;

  if (params != IF_DEF_PARAMS (ifp))
    {
      ospf_free_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  /* Update timer values in neighbor structure. */
  if (cargc == 5)
    {
      struct ospf *ospf;

      if ((ospf = ospf_lookup()))
	{
	  oi = ospf_if_lookup_by_local_addr (ospf, ifp, addr);
	  if (oi)
	    ospf_nbr_timer_update (oi);
	}
    }
  else
    {
      for (rn = route_top (IF_OIFS (ifp)); rn; rn = route_next (rn))
	if ((oi = rn->info))
	  ospf_nbr_timer_update (oi);
    }

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoIpDeadInterval,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf dead-interval",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Interval after which a neighbor is declared dead");

ALICMD (cmdFuncOspfNoIpDeadInterval,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ospf dead-interval",
       "Negate a command or set its defaults",
       "OSPF interface commands",
       "Interval after which a neighbor is declared dead");

DECMD (cmdFuncOspfip_ospf_hello_interval,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf hello-interval <1-65535> A.B.C.D",
       "IP Information",
       "OSPF interface commands",
       "Time between HELLO packets",
       "Seconds"
       "Address of interface")
{
  struct interface *ifp;
  u_int32_t seconds;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  params = IF_DEF_PARAMS (ifp);

  seconds = strtol (cargc == 3 ? cargv[2] : cargv[3], NULL, 10);

  /* HelloInterval range is <1-65535>. */
  if (seconds < 1 || seconds > 65535)
    {
      cmdPrint (cmsh, "Hello Interval is invalid\n");
      return (CMD_IPC_WARNING);
    }

  if (cargc == 5)
    {
      ret = inet_aton(cargv[4], &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D\n");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  SET_IF_PARAM (params, v_hello);
  params->v_hello = seconds;

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfip_ospf_hello_interval,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf hello-interval <1-65535>",
       "IP Information",
       "OSPF interface commands",
       "Time between HELLO packets",
       "Seconds");

ALICMD (cmdFuncOspfip_ospf_hello_interval,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ospf hello-interval <1-65535>",
       "OSPF interface commands",
       "Time between HELLO packets",
       "Seconds");

DECMD (cmdFuncOspfNoIpHelloInterval,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf hello-interval A.B.C.D",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Time between HELLO packets",
       "Address of interface")
{
  struct interface *ifp;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  params = IF_DEF_PARAMS (ifp);

  if (cargc == 5)
    {
      ret = inet_aton(cargv[4], &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D\n");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_lookup_if_params (ifp, addr);
      if (params == NULL)
	return (CMD_IPC_OK);
    }

  UNSET_IF_PARAM (params, v_hello);
  params->v_hello = OSPF_HELLO_INTERVAL_DEFAULT;

  if (params != IF_DEF_PARAMS (ifp))
    {
      ospf_free_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoIpHelloInterval,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf hello-interval",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Time between HELLO packets");

ALICMD (cmdFuncOspfNoIpHelloInterval,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ospf hello-interval",
       "Negate a command or set its defaults",
       "OSPF interface commands",
       "Time between HELLO packets");

DECMD (cmdFuncOspfIpNetwork,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf network (broadcast|non-broadcast|point-to-multipoint|point-to-point)",
       "IP Information",
       "OSPF interface commands",
       "Network type",
       "Specify OSPF broadcast multi-access network",
       "Specify OSPF NBMA network",
       "Specify OSPF point-to-multipoint network",
       "Specify OSPF point-to-point network")
{
  struct interface *ifp
  	  	  	  = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  int old_type = IF_DEF_PARAMS (ifp)->type;
  struct route_node *rn;

  if (strncmp (cargc == 4 ? cargv[3] : cargv[2], "b", 1) == 0)
    IF_DEF_PARAMS (ifp)->type = OSPF_IFTYPE_BROADCAST;
  else if (strncmp (cargc == 4 ? cargv[3] : cargv[2], "n", 1) == 0)
    IF_DEF_PARAMS (ifp)->type = OSPF_IFTYPE_NBMA;
  else if (strncmp (cargc == 4 ? cargv[3] : cargv[2], "point-to-m", 10) == 0)
    IF_DEF_PARAMS (ifp)->type = OSPF_IFTYPE_POINTOMULTIPOINT;
  else if (strncmp (cargc == 4 ? cargv[3] : cargv[2], "point-to-p", 10) == 0)
    IF_DEF_PARAMS (ifp)->type = OSPF_IFTYPE_POINTOPOINT;

  if (IF_DEF_PARAMS (ifp)->type == old_type)
    return (CMD_IPC_OK);

  SET_IF_PARAM (IF_DEF_PARAMS (ifp), type);

  for (rn = route_top (IF_OIFS (ifp)); rn; rn = route_next (rn))
    {
      struct ospf_interface *oi = rn->info;

      if (!oi)
	continue;

      oi->type = IF_DEF_PARAMS (ifp)->type;

      if (oi->state > ISM_Down)
	{
	  OSPF_ISM_EVENT_EXECUTE (oi, ISM_InterfaceDown);
	  OSPF_ISM_EVENT_EXECUTE (oi, ISM_InterfaceUp);
	}
    }

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfIpNetwork,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ospf network (broadcast|non-broadcast|point-to-multipoint|point-to-point)",
       "OSPF interface commands",
       "Network type",
       "Specify OSPF broadcast multi-access network",
       "Specify OSPF NBMA network",
       "Specify OSPF point-to-multipoint network",
       "Specify OSPF point-to-point network");

DECMD (cmdFuncOspfNoIpNetwork,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf network",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Network type")
{
  struct interface *ifp
  	  	  	  = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  int old_type = IF_DEF_PARAMS (ifp)->type;
  struct route_node *rn;

  IF_DEF_PARAMS (ifp)->type = ospf_default_iftype(ifp);

  if (IF_DEF_PARAMS (ifp)->type == old_type)
    return (CMD_IPC_OK);

  for (rn = route_top (IF_OIFS (ifp)); rn; rn = route_next (rn))
    {
      struct ospf_interface *oi = rn->info;

      if (!oi)
	continue;

      oi->type = IF_DEF_PARAMS (ifp)->type;

      if (oi->state > ISM_Down)
	{
	  OSPF_ISM_EVENT_EXECUTE (oi, ISM_InterfaceDown);
	  OSPF_ISM_EVENT_EXECUTE (oi, ISM_InterfaceUp);
	}
    }

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoIpNetwork,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ospf network",
       "Negate a command or set its defaults",
       "OSPF interface commands",
       "Network type");

DECMD (cmdFuncOspfIpPriority,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf priority <0-255> A.B.C.D",
       "IP Information",
       "OSPF interface commands",
       "Router priority",
       "Priority",
       "Address of interface")
{
  struct interface *ifp;
  long priority;
  struct route_node *rn;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  params = IF_DEF_PARAMS (ifp);

  priority = strtol (cargc == 3 ? cargv[2] : cargv[3], NULL, 10);

  /* Router Priority range is <0-255>. */
  if (priority < 0 || priority > 255)
    {
      cmdPrint (cmsh, "Router Priority is invalid\n");
      return (CMD_IPC_WARNING);
    }

  if (cargc == 5)
    {
      ret = inet_aton(cargv[4], &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  SET_IF_PARAM (params, priority);
  params->priority = priority;

  for (rn = route_top (IF_OIFS (ifp)); rn; rn = route_next (rn))
    {
      struct ospf_interface *oi = rn->info;

      if (!oi)
	continue;


      if (PRIORITY (oi) != OSPF_IF_PARAM (oi, priority))
	{
	  PRIORITY (oi) = OSPF_IF_PARAM (oi, priority);
	  OSPF_ISM_EVENT_SCHEDULE (oi, ISM_NeighborChange);
	}
    }

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfIpPriority,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf priority <0-255>",
       "IP Information",
       "OSPF interface commands",
       "Router priority",
       "Priority");

ALICMD (cmdFuncOspfIpPriority,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ospf priority <0-255>",
       "OSPF interface commands",
       "Router priority",
       "Priority");

DECMD (cmdFuncOspfNoIpPriority,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf priority A.B.C.D",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Router priority",
       "Address of interface")
{
  struct interface *ifp;
  struct route_node *rn;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  params = IF_DEF_PARAMS (ifp);

  if (cargc == 5)
    {
      ret = inet_aton(cargv[4], &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D\n");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_lookup_if_params (ifp, addr);
      if (params == NULL)
	return (CMD_IPC_OK);
    }

  UNSET_IF_PARAM (params, priority);
  params->priority = OSPF_ROUTER_PRIORITY_DEFAULT;

  if (params != IF_DEF_PARAMS (ifp))
    {
      ospf_free_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  for (rn = route_top (IF_OIFS (ifp)); rn; rn = route_next (rn))
    {
      struct ospf_interface *oi = rn->info;

      if (!oi)
	continue;


      if (PRIORITY (oi) != OSPF_IF_PARAM (oi, priority))
	{
	  PRIORITY (oi) = OSPF_IF_PARAM (oi, priority);
	  OSPF_ISM_EVENT_SCHEDULE (oi, ISM_NeighborChange);
	}
    }

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoIpPriority,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf priority",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Router priority");

ALICMD (cmdFuncOspfNoIpPriority,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ospf priority",
       "Negate a command or set its defaults",
       "OSPF interface commands",
       "Router priority");

DECMD (cmdFuncOspfIpRetransmitInterval,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf retransmit-interval <3-65535> A.B.C.D",
       "IP Information",
       "OSPF interface commands",
       "Time between retransmitting lost link state advertisements",
       "Seconds",
       "Address of interface")
{
  struct interface *ifp;
  u_int32_t seconds;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  params = IF_DEF_PARAMS (ifp);
  seconds = strtol (cargc == 3 ? cargv[2] : cargv[3], NULL, 10);

  /* Retransmit Interval range is <3-65535>. */
  if (seconds < 3 || seconds > 65535)
    {
      cmdPrint (cmsh, "Retransmit Interval is invalid\n");
      return (CMD_IPC_WARNING);
    }


  if (cargc == 5)
    {
      ret = inet_aton(cargv[4], &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D\n");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  SET_IF_PARAM (params, retransmit_interval);
  params->retransmit_interval = seconds;

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfIpRetransmitInterval,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf retransmit-interval <3-65535>",
       "IP Information",
       "OSPF interface commands",
       "Time between retransmitting lost link state advertisements",
       "Seconds");

ALICMD (cmdFuncOspfIpRetransmitInterval,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ospf retransmit-interval <3-65535>",
       "OSPF interface commands",
       "Time between retransmitting lost link state advertisements",
       "Seconds");

DECMD (cmdFuncOspfNoIpRetransmitInterval,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf retransmit-interval A.B.C.D",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Time between retransmitting lost link state advertisements",
       "Address of interface")
{
  struct interface *ifp;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  params = IF_DEF_PARAMS (ifp);

  if (cargc == 5)
    {
      ret = inet_aton(cargv[4], &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D\n");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_lookup_if_params (ifp, addr);
      if (params == NULL)
	return (CMD_IPC_OK);
    }

  UNSET_IF_PARAM (params, retransmit_interval);
  params->retransmit_interval = OSPF_RETRANSMIT_INTERVAL_DEFAULT;

  if (params != IF_DEF_PARAMS (ifp))
    {
      ospf_free_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoIpRetransmitInterval,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf retransmit-interval",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Time between retransmitting lost link state advertisements");

ALICMD (cmdFuncOspfNoIpRetransmitInterval,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ospf retransmit-interval",
       "Negate a command or set its defaults",
       "OSPF interface commands",
       "Time between retransmitting lost link state advertisements");

DECMD (cmdFuncOspfIpTransmitDelay,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf transmit-delay <1-65535> A.B.C.D",
       "IP Information",
       "OSPF interface commands",
       "Link state transmit delay",
       "Seconds",
       "Address of interface")
{
  struct interface *ifp;
  u_int32_t seconds;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  params = IF_DEF_PARAMS (ifp);
  seconds = strtol (cargc == 3 ? cargv[2] : cargv[3], NULL, 10);

  /* Transmit Delay range is <1-65535>. */
  if (seconds < 1 || seconds > 65535)
    {
      cmdPrint (cmsh, "Transmit Delay is invalid\n");
      return (CMD_IPC_WARNING);
    }

  if (cargc == 5)
    {
      ret = inet_aton(cargv[4], &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D\n");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  SET_IF_PARAM (params, transmit_delay);
  params->transmit_delay = seconds;

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfIpTransmitDelay,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ip ospf transmit-delay <1-65535>",
       "IP Information",
       "OSPF interface commands",
       "Link state transmit delay",
       "Seconds");

ALICMD (cmdFuncOspfIpTransmitDelay,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "ospf transmit-delay <1-65535>",
       "OSPF interface commands",
       "Link state transmit delay",
       "Seconds");

DECMD (cmdFuncOspfNoIpTransmitDelay,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf transmit-delay A.B.C.D",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Link state transmit delay",
       "Address of interface")
{
  struct interface *ifp;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  params = IF_DEF_PARAMS (ifp);

  if (cargc == 5)
    {
      ret = inet_aton(cargv[4], &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D\n");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_lookup_if_params (ifp, addr);
      if (params == NULL)
	return (CMD_IPC_OK);
    }

  UNSET_IF_PARAM (params, transmit_delay);
  params->transmit_delay = OSPF_TRANSMIT_DELAY_DEFAULT;

  if (params != IF_DEF_PARAMS (ifp))
    {
      ospf_free_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoIpTransmitDelay,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf transmit-delay",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Link state transmit delay");

ALICMD (cmdFuncOspfNoIpTransmitDelay,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ospf transmit-delay",
       "Negate a command or set its defaults",
       "OSPF interface commands",
       "Link state transmit delay");


DECMD (cmdFuncOspfRedistributeSourceMetricType,
       CMD_NODE_CONFIG_OSPF,
       IPC_OSPF,
		"redistribute (kernel|connected|static|rip|isis|bgp)"
		" metric <0-16777214> metric-type (1|2) route-map WORD",
		"Redistribute information from another routing protocol",
		"Kernel routes (not installed via the zebra RIB)",
		"Connected routes (directly attached subnet or host)",
		"Statically configured routes",
		"Routing Information Protocol (RIP)",
		"Intermediate System to Intermediate System (IS-IS)",
		"Border Gateway Protocol (BGP)",
		"Metric for redistributed routes",
		"OSPF default metric",
		"OSPF exterior metric type for redistributed routes",
		"Set OSPF External Type 1 metrics",
		"Set OSPF External Type 2 metrics",
		"Route map reference",
		"Pointer to route-map entries")
{
	struct ospf *ospf = om->index;
	int source;
	int type = -1;
	int metric = -1;

	/* Get distribute source. */
	source = protoRedistNum(AFI_IP, cargv[1]);
	if (source < 0 || source == RIB_ROUTE_TYPE_OSPF)
		return (CMD_IPC_WARNING);

	/* Get metric value. */
	if (cargc >= 4)
		if (!str2metric(cargv[3], &metric))
			return (CMD_IPC_WARNING);

	/* Get metric type. */
	if (cargc >= 6)
		if (!str2metric_type(cargv[5], &type))
			return (CMD_IPC_WARNING);

	if (cargc == 8)
		ospf_routemap_set(ospf, source, cargv[7]);
	else
		ospf_routemap_unset(ospf, source);

	return ospfRedistributeSet(cmsh, ospf, source, type, metric);
}

ALICMD (cmdFuncOspfRedistributeSourceMetricType,
	   CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "redistribute (kernel|connected|static|rip|isis|bgp)"
       " metric <0-16777214> metric-type (1|2)",
       "Redistribute information from another routing protocol",
       "Kernel routes (not installed via the zebra RIB)",
		"Connected routes (directly attached subnet or host)",
		"Statically configured routes",
		"Routing Information Protocol (RIP)",
		"Intermediate System to Intermediate System (IS-IS)",
		"Border Gateway Protocol (BGP)",
       "Metric for redistributed routes",
       "OSPF default metric",
       "OSPF exterior metric type for redistributed routes",
       "Set OSPF External Type 1 metrics",
       "Set OSPF External Type 2 metrics");

ALICMD (cmdFuncOspfRedistributeSourceMetricType,
	   CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "redistribute (kernel|connected|static|rip|isis|bgp) metric <0-16777214>",
       "Redistribute information from another routing protocol",
       "Kernel routes (not installed via the zebra RIB)",
		"Connected routes (directly attached subnet or host)",
		"Statically configured routes",
		"Routing Information Protocol (RIP)",
		"Intermediate System to Intermediate System (IS-IS)",
		"Border Gateway Protocol (BGP)",
       "Metric for redistributed routes",
       "OSPF default metric");

DECMD (cmdFuncOspfRedistributeSourceTypeMetric,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "redistribute (kernel|connected|static|rip|isis|bgp)"
       " metric-type (1|2) metric <0-16777214> route-map WORD",
       "Redistribute information from another routing protocol",
       "Kernel routes (not installed via the zebra RIB)",
		"Connected routes (directly attached subnet or host)",
		"Statically configured routes",
		"Routing Information Protocol (RIP)",
		"Intermediate System to Intermediate System (IS-IS)",
		"Border Gateway Protocol (BGP)",
       "OSPF exterior metric type for redistributed routes",
       "Set OSPF External Type 1 metrics",
       "Set OSPF External Type 2 metrics",
       "Metric for redistributed routes",
       "OSPF default metric",
       "Route map reference",
       "Pointer to route-map entries")
{
  struct ospf *ospf = om->index;
  int source;
  int type = -1;
  int metric = -1;

  /* Get distribute source. */
  source = protoRedistNum(AFI_IP, cargv[1]);
  if (source < 0 || source == RIB_ROUTE_TYPE_OSPF)
    return (CMD_IPC_WARNING);

  /* Get metric value. */
  if (cargc >= 4)
    if (!str2metric_type (cargv[3], &type))
      return (CMD_IPC_WARNING);

  /* Get metric type. */
  if (cargc >= 6)
    if (!str2metric (cargv[5], &metric))
      return (CMD_IPC_WARNING);

  if (cargc == 8)
    ospf_routemap_set (ospf, source, cargv[7]);
  else
    ospf_routemap_unset (ospf, source);

  return ospfRedistributeSet(cmsh, ospf, source, type, metric);
}

ALICMD (cmdFuncOspfRedistributeSourceTypeMetric,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "redistribute (kernel|connected|static|rip|isis|bgp)"
       " metric-type (1|2) metric <0-16777214>",
       "Redistribute information from another routing protocol",
       "Kernel routes (not installed via the zebra RIB)",
		"Connected routes (directly attached subnet or host)",
		"Statically configured routes",
		"Routing Information Protocol (RIP)",
		"Intermediate System to Intermediate System (IS-IS)",
		"Border Gateway Protocol (BGP)",
       "OSPF exterior metric type for redistributed routes",
       "Set OSPF External Type 1 metrics",
       "Set OSPF External Type 2 metrics",
       "Metric for redistributed routes",
       "OSPF default metric");

ALICMD (cmdFuncOspfRedistributeSourceTypeMetric,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "redistribute (kernel|connected|static|rip|isis|bgp) metric-type (1|2)",
       "Redistribute information from another routing protocol",
       "Kernel routes (not installed via the zebra RIB)",
		"Connected routes (directly attached subnet or host)",
		"Statically configured routes",
		"Routing Information Protocol (RIP)",
		"Intermediate System to Intermediate System (IS-IS)",
		"Border Gateway Protocol (BGP)",
       "OSPF exterior metric type for redistributed routes",
       "Set OSPF External Type 1 metrics",
       "Set OSPF External Type 2 metrics");

ALICMD (cmdFuncOspfRedistributeSourceTypeMetric,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "redistribute (kernel|connected|static|rip|isis|bgp)",
       "Redistribute information from another routing protocol",
       "Kernel routes (not installed via the zebra RIB)",
		"Connected routes (directly attached subnet or host)",
		"Statically configured routes",
		"Routing Information Protocol (RIP)",
		"Intermediate System to Intermediate System (IS-IS)",
		"Border Gateway Protocol (BGP)");

DECMD (cmdFuncOspfRedistributeSourceMetricRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "redistribute (kernel|connected|static|rip|isis|bgp)"
       " metric <0-16777214> route-map WORD",
       "Redistribute information from another routing protocol",
       "Kernel routes (not installed via the zebra RIB)",
		"Connected routes (directly attached subnet or host)",
		"Statically configured routes",
		"Routing Information Protocol (RIP)",
		"Intermediate System to Intermediate System (IS-IS)",
		"Border Gateway Protocol (BGP)",
       "Metric for redistributed routes",
       "OSPF default metric",
       "Route map reference",
       "Pointer to route-map entries")
{
  struct ospf *ospf = om->index;
  int source;
  int metric = -1;

  /* Get distribute source. */
  source = protoRedistNum(AFI_IP, cargv[1]);
  if (source < 0 || source == RIB_ROUTE_TYPE_OSPF)
    return (CMD_IPC_WARNING);

  /* Get metric value. */
  if (cargc >= 4)
    if (!str2metric (cargv[3], &metric))
      return (CMD_IPC_WARNING);

  if (cargc == 6)
    ospf_routemap_set (ospf, source, cargv[5]);
  else
    ospf_routemap_unset (ospf, source);

  return ospfRedistributeSet(cmsh, ospf, source, -1, metric);
}

DECMD (cmdFuncOspfRedistributeSourceTypeRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "redistribute (kernel|connected|static|rip|isis|bgp)"
       " metric-type (1|2) route-map WORD",
       "Redistribute information from another routing protocol",
       "Kernel routes (not installed via the zebra RIB)",
		"Connected routes (directly attached subnet or host)",
		"Statically configured routes",
		"Routing Information Protocol (RIP)",
		"Intermediate System to Intermediate System (IS-IS)",
		"Border Gateway Protocol (BGP)",
       "OSPF exterior metric type for redistributed routes",
       "Set OSPF External Type 1 metrics",
       "Set OSPF External Type 2 metrics",
       "Route map reference",
       "Pointer to route-map entries")
{
  struct ospf *ospf = om->index;
  int source;
  int type = -1;

  /* Get distribute source. */
  source = protoRedistNum(AFI_IP, cargv[1]);
  if (source < 0 || source == RIB_ROUTE_TYPE_OSPF)
    return (CMD_IPC_WARNING);

  /* Get metric value. */
  if (cargc >= 4)
    if (!str2metric_type (cargv[3], &type))
      return (CMD_IPC_WARNING);

  if (cargc == 6)
    ospf_routemap_set (ospf, source, cargv[5]);
  else
    ospf_routemap_unset (ospf, source);

  return ospfRedistributeSet(cmsh, ospf, source, type, -1);
}

DECMD (cmdFuncOspfRedistributeSourceRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "redistribute (kernel|connected|static|rip|isis|bgp) route-map WORD",
       "Redistribute information from another routing protocol",
       "Kernel routes (not installed via the zebra RIB)",
		"Connected routes (directly attached subnet or host)",
		"Statically configured routes",
		"Routing Information Protocol (RIP)",
		"Intermediate System to Intermediate System (IS-IS)",
		"Border Gateway Protocol (BGP)",
       "Route map reference",
       "Pointer to route-map entries")
{
  struct ospf *ospf = om->index;
  int source;

  /* Get distribute source. */
  source = protoRedistNum(AFI_IP, cargv[1]);
  if (source < 0 || source == RIB_ROUTE_TYPE_OSPF)
    return (CMD_IPC_WARNING);

  if (cargc == 4)
    ospf_routemap_set (ospf, source, cargv[3]);
  else
    ospf_routemap_unset (ospf, source);

  return ospfRedistributeSet(cmsh, ospf, source, -1, -1);
}

DECMD (cmdFuncOspfNoRedistributeSource,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "no redistribute (kernel|connected|static|rip|isis|bgp)",
       "Negate a command or set its defaults",
       "Redistribute information from another routing protocol",
       "Kernel routes (not installed via the zebra RIB)",
		"Connected routes (directly attached subnet or host)",
		"Statically configured routes",
		"Routing Information Protocol (RIP)",
		"Intermediate System to Intermediate System (IS-IS)",
		"Border Gateway Protocol (BGP)")
{
  struct ospf *ospf = om->index;
  int source;

  source = protoRedistNum(AFI_IP, cargv[2]);
  if (source < 0 || source == RIB_ROUTE_TYPE_OSPF)
    return (CMD_IPC_WARNING);

  ospf_routemap_unset (ospf, source);
  return ospf_redistribute_unset (cmsh, ospf, source);
}

DECMD (cmdFuncOspfDistributeListOut,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "distribute-list WORD out (kernel|connected|static|rip|isis|bgp)",
       "Filter networks in routing updates",
       "Access-list name",
       "Filter outgoing routing updates",
       "Kernel routes (not installed via the zebra RIB)",
		"Connected routes (directly attached subnet or host)",
		"Statically configured routes",
		"Routing Information Protocol (RIP)",
		"Intermediate System to Intermediate System (IS-IS)",
		"Border Gateway Protocol (BGP)")
{
  struct ospf *ospf = om->index;
  int source;

  /* Get distribute source. */
  source = protoRedistNum(AFI_IP, cargv[3]);
  if (source < 0 || source == RIB_ROUTE_TYPE_OSPF)
    return (CMD_IPC_WARNING);

  return ospf_distribute_list_out_set (ospf, source, cargv[1]);
}

DECMD (cmdFuncOspfNoDistributeListOut,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "no distribute-list WORD out (kernel|connected|static|rip|isis|bgp)",
       "Negate a command or set its defaults",
       "Filter networks in routing updates",
       "Access-list name",
       "Filter outgoing routing updates",
       "Kernel routes (not installed via the zebra RIB)",
		"Connected routes (directly attached subnet or host)",
		"Statically configured routes",
		"Routing Information Protocol (RIP)",
		"Intermediate System to Intermediate System (IS-IS)",
		"Border Gateway Protocol (BGP)")
{
  struct ospf *ospf = om->index;
  int source;

  source = protoRedistNum(AFI_IP, cargv[4]);
  if (source < 0 || source == RIB_ROUTE_TYPE_OSPF)
    return (CMD_IPC_WARNING);

  return ospf_distribute_list_out_unset (ospf, source, cargv[2]);
}

/* Default information originate. */
DECMD (cmdFuncOspfDefaultInformationOriginateMetricTypeRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "default-information originate metric <0-16777214> metric-type (1|2) route-map WORD",
       "Control distribution of default information",
       "Distribute a default route",
       "OSPF default metric",
       "OSPF metric",
       "OSPF metric type for default routes",
       "Set OSPF External Type 1 metrics",
       "Set OSPF External Type 2 metrics",
       "Route map reference",
       "Pointer to route-map entries")
{
  struct ospf *ospf = om->index;
  int type = -1;
  int metric = -1;

  /* Get metric value. */
  if (cargc > 3)
    if (!str2metric (cargv[3], &metric))
      return (CMD_IPC_WARNING);

  /* Get metric type. */
  if (cargc >= 6)
    if (!str2metric_type (cargv[5], &type))
      return (CMD_IPC_WARNING);

  if (cargc == 8)
    ospf_routemap_set (ospf, DEFAULT_ROUTE, cargv[7]);
  else
    ospf_routemap_unset (ospf, DEFAULT_ROUTE);

  return ospf_redistribute_default_set (cmsh, ospf, DEFAULT_ORIGINATE_ZEBRA,
					type, metric);
}

ALICMD (cmdFuncOspfDefaultInformationOriginateMetricTypeRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "default-information originate metric <0-16777214> metric-type (1|2)",
       "Control distribution of default information",
       "Distribute a default route",
       "OSPF default metric",
       "OSPF metric",
       "OSPF metric type for default routes",
       "Set OSPF External Type 1 metrics",
       "Set OSPF External Type 2 metrics");

ALICMD (cmdFuncOspfDefaultInformationOriginateMetricTypeRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "default-information originate metric <0-16777214>",
       "Control distribution of default information",
       "Distribute a default route",
       "OSPF default metric",
       "OSPF metric");

ALICMD (cmdFuncOspfDefaultInformationOriginateMetricTypeRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "default-information originate",
       "Control distribution of default information",
       "Distribute a default route");

/* Default information originate. */
DECMD (cmdFuncOspfDefaultInformationOriginateMetricRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "default-information originate metric <0-16777214> route-map WORD",
       "Control distribution of default information",
       "Distribute a default route",
       "OSPF default metric",
       "OSPF metric",
       "Route map reference",
       "Pointer to route-map entries")
{
  struct ospf *ospf = om->index;
  int metric = -1;

  /* Get metric value. */
  if (cargc > 3)
    if (!str2metric (cargv[3], &metric))
      return (CMD_IPC_WARNING);

  if (cargc == 6)
    ospf_routemap_set (ospf, DEFAULT_ROUTE, cargv[5]);
  else
    ospf_routemap_unset (ospf, DEFAULT_ROUTE);

  return ospf_redistribute_default_set (cmsh, ospf, DEFAULT_ORIGINATE_ZEBRA,
					-1, metric);
}

/* Default information originate. */
DECMD (cmdFuncOspfDefaultInformationOriginateRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "default-information originate route-map WORD",
       "Control distribution of default information",
       "Distribute a default route",
       "Route map reference",
       "Pointer to route-map entries")
{
  struct ospf *ospf = om->index;

  if (cargc == 4)
    ospf_routemap_set (ospf, DEFAULT_ROUTE, cargv[3]);
  else
    ospf_routemap_unset (ospf, DEFAULT_ROUTE);

  return ospf_redistribute_default_set (cmsh, ospf, DEFAULT_ORIGINATE_ZEBRA, -1, -1);
}

DECMD (cmdFuncOspfDefaultInformationOriginateTypeMetricRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "default-information originate metric-type (1|2) metric <0-16777214> route-map WORD",
       "Control distribution of default information",
       "Distribute a default route",
       "OSPF metric type for default routes",
       "Set OSPF External Type 1 metrics",
       "Set OSPF External Type 2 metrics",
       "OSPF default metric",
       "OSPF metric",
       "Route map reference",
       "Pointer to route-map entries")
{
  struct ospf *ospf = om->index;
  int type = -1;
  int metric = -1;

  /* Get metric type. */
  if (cargc >= 4)
    if (!str2metric_type (cargv[3], &type))
      return (CMD_IPC_WARNING);

  /* Get metric value. */
  if (cargc >= 6)
    if (!str2metric (cargv[5], &metric))
      return (CMD_IPC_WARNING);

  if (cargc == 8)
    ospf_routemap_set (ospf, DEFAULT_ROUTE, cargv[7]);
  else
    ospf_routemap_unset (ospf, DEFAULT_ROUTE);

  return ospf_redistribute_default_set (cmsh, ospf, DEFAULT_ORIGINATE_ZEBRA,
					type, metric);
}

ALICMD (cmdFuncOspfDefaultInformationOriginateTypeMetricRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "default-information originate metric-type (1|2) metric <0-16777214>",
       "Control distribution of default information",
       "Distribute a default route",
       "OSPF metric type for default routes",
       "Set OSPF External Type 1 metrics",
       "Set OSPF External Type 2 metrics",
       "OSPF default metric",
       "OSPF metric");

ALICMD (cmdFuncOspfDefaultInformationOriginateTypeMetricRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "default-information originate metric-type (1|2)",
       "Control distribution of default information",
       "Distribute a default route",
       "OSPF metric type for default routes",
       "Set OSPF External Type 1 metrics",
       "Set OSPF External Type 2 metrics");

DECMD (cmdFuncOspfDefaultInformationOriginateTypeRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "default-information originate metric-type (1|2) route-map WORD",
       "Control distribution of default information",
       "Distribute a default route",
       "OSPF metric type for default routes",
       "Set OSPF External Type 1 metrics",
       "Set OSPF External Type 2 metrics",
       "Route map reference",
       "Pointer to route-map entries")
{
  struct ospf *ospf = om->index;
  int type = -1;

  /* Get metric type. */
  if (cargc >= 4)
    if (!str2metric_type (cargv[3], &type))
      return (CMD_IPC_WARNING);

  if (cargc == 6)
    ospf_routemap_set (ospf, DEFAULT_ROUTE, cargv[5]);
  else
    ospf_routemap_unset (ospf, DEFAULT_ROUTE);

  return ospf_redistribute_default_set (cmsh, ospf, DEFAULT_ORIGINATE_ZEBRA,
					type, -1);
}

DECMD (cmdFuncOspfDefaultInformationOriginateAlwaysMetricTypeRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "default-information originate always metric <0-16777214> metric-type (1|2) route-map WORD",
       "Control distribution of default information",
       "Distribute a default route",
       "Always advertise default route",
       "OSPF default metric",
       "OSPF metric",
       "OSPF metric type for default routes",
       "Set OSPF External Type 1 metrics",
       "Set OSPF External Type 2 metrics",
       "Route map reference",
       "Pointer to route-map entries")
{
  struct ospf *ospf = om->index;
  int type = -1;
  int metric = -1;

  /* Get metric value. */
  if (cargc >= 5)
    if (!str2metric (cargv[4], &metric))
      return (CMD_IPC_WARNING);

  /* Get metric type. */
  if (cargc >= 7)
    if (!str2metric_type (cargv[6], &type))
      return (CMD_IPC_WARNING);

  if (cargc == 9)
    ospf_routemap_set (ospf, DEFAULT_ROUTE, cargv[8]);
  else
    ospf_routemap_unset (ospf, DEFAULT_ROUTE);

  return ospf_redistribute_default_set (cmsh, ospf, DEFAULT_ORIGINATE_ALWAYS,
					type, metric);
}

ALICMD (cmdFuncOspfDefaultInformationOriginateAlwaysMetricTypeRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "default-information originate always metric <0-16777214> metric-type (1|2)",
       "Control distribution of default information",
       "Distribute a default route",
       "Always advertise default route",
       "OSPF default metric",
       "OSPF metric",
       "OSPF metric type for default routes",
       "Set OSPF External Type 1 metrics",
       "Set OSPF External Type 2 metrics");

ALICMD (cmdFuncOspfDefaultInformationOriginateAlwaysMetricTypeRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "default-information originate always metric <0-16777214>",
       "Control distribution of default information",
       "Distribute a default route",
       "Always advertise default route",
       "OSPF default metric",
       "OSPF metric",
       "OSPF metric type for default routes");

ALICMD (cmdFuncOspfDefaultInformationOriginateAlwaysMetricTypeRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "default-information originate always",
       "Control distribution of default information",
       "Distribute a default route",
       "Always advertise default route");

DECMD (cmdFuncOspfDefaultInformationOriginateAlwaysMetricRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "default-information originate always metric <0-16777214> route-map WORD",
       "Control distribution of default information",
       "Distribute a default route",
       "Always advertise default route",
       "OSPF default metric",
       "OSPF metric",
       "Route map reference",
       "Pointer to route-map entries")
{
  struct ospf *ospf = om->index;
  int metric = -1;

  /* Get metric value. */
  if (cargc >= 5)
    if (!str2metric (cargv[4], &metric))
      return (CMD_IPC_WARNING);

  if (cargc == 7)
    ospf_routemap_set (ospf, DEFAULT_ROUTE, cargv[6]);
  else
    ospf_routemap_unset (ospf, DEFAULT_ROUTE);

  return ospf_redistribute_default_set (cmsh, ospf, DEFAULT_ORIGINATE_ALWAYS,
					-1, metric);
}

DECMD (cmdFuncOspfDefaultInformationOriginateAlwaysRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "default-information originate always route-map WORD",
       "Control distribution of default information",
       "Distribute a default route",
       "Always advertise default route",
       "Route map reference",
       "Pointer to route-map entries")
{
  struct ospf *ospf = om->index;

  if (cargc == 5)
    ospf_routemap_set (ospf, DEFAULT_ROUTE, cargv[4]);
  else
    ospf_routemap_unset (ospf, DEFAULT_ROUTE);

  return ospf_redistribute_default_set (cmsh, ospf, DEFAULT_ORIGINATE_ALWAYS, -1, -1);
}

DECMD (cmdFuncOspfDefaultInformationOriginateAlwaysTypeMetricRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "default-information originate always metric-type (1|2) metric <0-16777214> route-map WORD",
       "Control distribution of default information",
       "Distribute a default route",
       "Always advertise default route",
       "OSPF metric type for default routes",
       "Set OSPF External Type 1 metrics",
       "Set OSPF External Type 2 metrics",
       "OSPF default metric",
       "OSPF metric",
       "Route map reference",
       "Pointer to route-map entries")
{
  struct ospf *ospf = om->index;
  int type = -1;
  int metric = -1;

  /* Get metric type. */
  if (cargc >= 5)
    if (!str2metric_type (cargv[4], &type))
      return (CMD_IPC_WARNING);

  /* Get metric value. */
  if (cargc >= 7)
    if (!str2metric (cargv[6], &metric))
      return (CMD_IPC_WARNING);

  if (cargc == 9)
    ospf_routemap_set (ospf, DEFAULT_ROUTE, cargv[8]);
  else
    ospf_routemap_unset (ospf, DEFAULT_ROUTE);

  return ospf_redistribute_default_set (cmsh, ospf, DEFAULT_ORIGINATE_ALWAYS,
					type, metric);
}

ALICMD (cmdFuncOspfDefaultInformationOriginateAlwaysTypeMetricRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "default-information originate always metric-type (1|2) metric <0-16777214>",
       "Control distribution of default information",
       "Distribute a default route",
       "Always advertise default route",
       "OSPF metric type for default routes",
       "Set OSPF External Type 1 metrics",
       "Set OSPF External Type 2 metrics",
       "OSPF default metric",
       "OSPF metric");

ALICMD (cmdFuncOspfDefaultInformationOriginateAlwaysTypeMetricRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "default-information originate always metric-type (1|2)",
       "Control distribution of default information",
       "Distribute a default route",
       "Always advertise default route",
       "OSPF metric type for default routes",
       "Set OSPF External Type 1 metrics",
       "Set OSPF External Type 2 metrics");

DECMD (cmdFuncOspfDefaultInformationOriginateAlwaysTypeRoutemap,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "default-information originate always metric-type (1|2) route-map WORD",
       "Control distribution of default information",
       "Distribute a default route",
       "Always advertise default route",
       "OSPF metric type for default routes",
       "Set OSPF External Type 1 metrics",
       "Set OSPF External Type 2 metrics",
       "Route map reference",
       "Pointer to route-map entries")
{
  struct ospf *ospf = om->index;
  int type = -1;

  /* Get metric type. */
  if (cargc >= 5)
    if (!str2metric_type (cargv[4], &type))
      return (CMD_IPC_WARNING);

  if (cargc == 7)
    ospf_routemap_set (ospf, DEFAULT_ROUTE, cargv[6]);
  else
    ospf_routemap_unset (ospf, DEFAULT_ROUTE);

  return ospf_redistribute_default_set (cmsh, ospf, DEFAULT_ORIGINATE_ALWAYS,
					type, -1);
}

DECMD (cmdFuncOspfNoDefaultInformationOriginate,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "no default-information originate",
       "Negate a command or set its defaults",
       "Control distribution of default information",
       "Distribute a default route")
{
  struct ospf *ospf = om->index;
  struct prefix_ipv4 p;

  p.family = AF_INET;
  p.prefix.s_addr = 0;
  p.prefixlen = 0;

  ospf_external_lsa_flush (ospf, DEFAULT_ROUTE, &p, 0);

  if (EXTERNAL_INFO (DEFAULT_ROUTE)) {
    ospf_external_info_delete (DEFAULT_ROUTE, p);
    route_table_finish (EXTERNAL_INFO (DEFAULT_ROUTE));
    EXTERNAL_INFO (DEFAULT_ROUTE) = NULL;
  }

  ospf_routemap_unset (ospf, DEFAULT_ROUTE);
  return ospf_redistribute_default_unset (cmsh, ospf);
}

DECMD (cmdFuncOspfDefaultMetric,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "default-metric <0-16777214>",
       "Set metric of redistributed routes",
       "Default metric")
{
  struct ospf *ospf = om->index;
  int metric = -1;

  if (!str2metric (cargv[1], &metric))
    return (CMD_IPC_WARNING);

  ospf->default_metric = metric;

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoDefaultMetric,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "no default-metric",
       "Negate a command or set its defaults",
       "Set metric of redistributed routes")
{
  struct ospf *ospf = om->index;

  ospf->default_metric = -1;

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoDefaultMetric,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "no default-metric <0-16777214>",
       "Negate a command or set its defaults",
       "Set metric of redistributed routes",
       "Default metric");

DECMD (cmdFuncOspfDistance,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "distance <1-255>",
       "Define an administrative distance",
       "OSPF Administrative distance")
{
  struct ospf *ospf = om->index;

  ospf->distance_all = atoi (cargv[1]);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoDistance,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "no distance <1-255>",
       "Negate a command or set its defaults",
       "Define an administrative distance",
       "OSPF Administrative distance")
{
  struct ospf *ospf = om->index;

  ospf->distance_all = 0;

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoDistanceOspf,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "no distance ospf",
       "Negate a command or set its defaults",
       "Define an administrative distance",
       "OSPF Administrative distance")
{
  struct ospf *ospf = om->index;

  ospf->distance_intra = 0;
  ospf->distance_inter = 0;
  ospf->distance_external = 0;

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfDistanceOspfIntra,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "distance ospf intra-area <1-255>",
       "Define an administrative distance",
       "OSPF Administrative distance",
       "Intra-area routes",
       "Distance for intra-area routes")
{
  struct ospf *ospf = om->index;

  ospf->distance_intra = atoi (cargv[3]);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfDistanceOspfIntraInter,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "distance ospf intra-area <1-255> inter-area <1-255>",
       "Define an administrative distance",
       "OSPF Administrative distance",
       "Intra-area routes",
       "Distance for intra-area routes",
       "Inter-area routes",
       "Distance for inter-area routes")
{
  struct ospf *ospf = om->index;

  ospf->distance_intra = atoi (cargv[3]);
  ospf->distance_inter = atoi (cargv[5]);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfDistanceOspfIntraExternal,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "distance ospf intra-area <1-255> external <1-255>",
       "Define an administrative distance",
       "OSPF Administrative distance",
       "Intra-area routes",
       "Distance for intra-area routes",
       "External routes",
       "Distance for external routes")
{
  struct ospf *ospf = om->index;

  ospf->distance_intra = atoi (cargv[3]);
  ospf->distance_external = atoi (cargv[5]);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfDistanceOspfIntraInterExternal,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "distance ospf intra-area <1-255> inter-area <1-255> external <1-255>",
       "Define an administrative distance",
       "OSPF Administrative distance",
       "Intra-area routes",
       "Distance for intra-area routes",
       "Inter-area routes",
       "Distance for inter-area routes",
       "External routes",
       "Distance for external routes")
{
  struct ospf *ospf = om->index;

  ospf->distance_intra = atoi (cargv[3]);
  ospf->distance_inter = atoi (cargv[5]);
  ospf->distance_external = atoi (cargv[7]);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfDistanceOspfIntraExternalInter,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "distance ospf intra-area <1-255> external <1-255> inter-area <1-255>",
       "Define an administrative distance",
       "OSPF Administrative distance",
       "Intra-area routes",
       "Distance for intra-area routes",
       "External routes",
       "Distance for external routes",
       "Inter-area routes",
       "Distance for inter-area routes")
{
  struct ospf *ospf = om->index;

  ospf->distance_intra = atoi (cargv[3]);
  ospf->distance_external = atoi (cargv[5]);
  ospf->distance_inter = atoi (cargv[7]);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfDistanceOspfInter,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "distance ospf inter-area <1-255>",
       "Define an administrative distance",
       "OSPF Administrative distance",
       "Inter-area routes",
       "Distance for inter-area routes")
{
  struct ospf *ospf = om->index;

  ospf->distance_inter = atoi (cargv[3]);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfDistanceOspfInterIntra,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "distance ospf inter-area <1-255> intra-area <1-255>",
       "Define an administrative distance",
       "OSPF Administrative distance",
       "Inter-area routes",
       "Distance for inter-area routes",
       "Intra-area routes",
       "Distance for intra-area routes")
{
  struct ospf *ospf = om->index;

  ospf->distance_inter = atoi (cargv[3]);
  ospf->distance_intra = atoi (cargv[5]);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfDistanceOspfInterExternal,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "distance ospf inter-area <1-255> external <1-255>",
       "Define an administrative distance",
       "OSPF Administrative distance",
       "Inter-area routes",
       "Distance for inter-area routes",
       "External routes",
       "Distance for external routes")
{
  struct ospf *ospf = om->index;

  ospf->distance_inter = atoi (cargv[3]);
  ospf->distance_external = atoi (cargv[5]);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfDistanceOspfInterIntraExternal,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "distance ospf inter-area <1-255> intra-area <1-255> external <1-255>",
       "Define an administrative distance",
       "OSPF Administrative distance",
       "Inter-area routes",
       "Distance for inter-area routes",
       "Intra-area routes",
       "Distance for intra-area routes",
       "External routes",
       "Distance for external routes")
{
  struct ospf *ospf = om->index;

  ospf->distance_inter = atoi (cargv[3]);
  ospf->distance_intra = atoi (cargv[5]);
  ospf->distance_external = atoi (cargv[7]);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfDistanceOspfInterExternalIntra,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "distance ospf inter-area <1-255> external <1-255> intra-area <1-255>",
       "Define an administrative distance",
       "OSPF Administrative distance",
       "Inter-area routes",
       "Distance for inter-area routes",
       "External routes",
       "Distance for external routes",
       "Intra-area routes",
       "Distance for intra-area routes")
{
  struct ospf *ospf = om->index;

  ospf->distance_inter = atoi (cargv[3]);
  ospf->distance_external = atoi (cargv[5]);
  ospf->distance_intra = atoi (cargv[7]);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfDistanceOspfExternal,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "distance ospf external <1-255>",
       "Define an administrative distance",
       "OSPF Administrative distance",
       "External routes",
       "Distance for external routes")
{
  struct ospf *ospf = om->index;

  ospf->distance_external = atoi (cargv[3]);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfDistanceOspfExternalIntra,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "distance ospf external <1-255> intra-area <1-255>",
       "Define an administrative distance",
       "OSPF Administrative distance",
       "External routes",
       "Distance for external routes",
       "Intra-area routes",
       "Distance for intra-area routes")
{
  struct ospf *ospf = om->index;

  ospf->distance_external = atoi (cargv[3]);
  ospf->distance_intra = atoi (cargv[5]);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfDistanceOspfExternalInter,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "distance ospf external <1-255> inter-area <1-255>",
       "Define an administrative distance",
       "OSPF Administrative distance",
       "External routes",
       "Distance for external routes",
       "Inter-area routes",
       "Distance for inter-area routes")
{
  struct ospf *ospf = om->index;

  ospf->distance_external = atoi (cargv[3]);
  ospf->distance_inter = atoi (cargv[5]);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfDistanceOspfExternalIntraInter,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "distance ospf external <1-255> intra-area <1-255> inter-area <1-255>",
       "Define an administrative distance",
       "OSPF Administrative distance",
       "External routes",
       "Distance for external routes",
       "Intra-area routes",
       "Distance for intra-area routes",
       "Inter-area routes",
       "Distance for inter-area routes")
{
  struct ospf *ospf = om->index;

  ospf->distance_external = atoi (cargv[3]);
  ospf->distance_intra = atoi (cargv[5]);
  ospf->distance_inter = atoi (cargv[7]);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfDistanceOspfExternalInterIntra,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "distance ospf external <1-255> inter-area <1-255> intra-area <1-255>",
       "Define an administrative distance",
       "OSPF Administrative distance",
       "External routes",
       "Distance for external routes",
       "Inter-area routes",
       "Distance for inter-area routes",
       "Intra-area routes",
       "Distance for intra-area routes")
{
  struct ospf *ospf = om->index;

  ospf->distance_external = atoi (cargv[3]);
  ospf->distance_inter = atoi (cargv[5]);
  ospf->distance_intra = atoi (cargv[7]);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfDistanceSource,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "distance <1-255> A.B.C.D/M",
       "Define an administrative distance",
       "OSPF Administrative distance",
       "IP source prefix")
{
  struct ospf *ospf = om->index;

  ospf_distance_set (cmsh, ospf, cargv[1], cargv[2], NULL);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoDistanceSource,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "no distance <1-255> A.B.C.D/M",
       "Negate a command or set its defaults",
       "Define an administrative distance",
       "OSPF Administrative distance",
       "IP source prefix")
{
  struct ospf *ospf = om->index;

  ospf_distance_unset (cmsh, ospf, cargv[2], cargv[3], NULL);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfDistanceSourceAccessList,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "distance <1-255> A.B.C.D/M WORD",
       "Define an administrative distance",
       "OSPF Administrative distance",
       "IP source prefix",
       "Access list name")
{
  struct ospf *ospf = om->index;

  ospf_distance_set (cmsh, ospf, cargv[1], cargv[2], cargv[3]);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoDistanceSourceAccessList,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "no distance <1-255> A.B.C.D/M WORD",
       "Negate a command or set its defaults",
       "Define an administrative distance",
       "OSPF Administrative distance",
       "IP source prefix",
       "Access list name")
{
  struct ospf *ospf = om->index;

  ospf_distance_unset (cmsh, ospf, cargv[2], cargv[3], cargv[4]);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfIpMtuIgnore,
       CMD_NODE_INTERFACE,
       IPC_OSPF,
       "ip ospf mtu-ignore A.B.C.D",
       "IP Information",
       "OSPF interface commands",
       "Disable mtu mismatch detection",
       "Address of interface")
{
  struct interface *ifp;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  params = IF_DEF_PARAMS (ifp);

  if (cargc == 4)
    {
      ret = inet_aton(cargv[3], &addr);
      if (!ret)
        {
          cmdPrint (cmsh, "Please specify interface address by A.B.C.D\n");
          return (CMD_IPC_WARNING);
        }
      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }
  params->mtu_ignore = 1;
  if (params->mtu_ignore != OSPF_MTU_IGNORE_DEFAULT)
    SET_IF_PARAM (params, mtu_ignore);
  else
    {
      UNSET_IF_PARAM (params, mtu_ignore);
      if (params != IF_DEF_PARAMS (ifp))
        {
          ospf_free_if_params (ifp, addr);
          ospf_if_update_params (ifp, addr);
        }
    }
  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfIpMtuIgnore,
	   CMD_NODE_INTERFACE,
	   IPC_OSPF,
      "ip ospf mtu-ignore",
      "IP Information",
      "OSPF interface commands",
      "Disable mtu mismatch detection");


DECMD (cmdFuncOspfNoIpMtuIgnore,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "no ip ospf mtu-ignore A.B.C.D",
       "Negate a command or set its defaults",
       "IP Information",
       "OSPF interface commands",
       "Disable mtu mismatch detection",
       "Address of interface")
{
  struct interface *ifp;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));
  params = IF_DEF_PARAMS (ifp);

  if (cargc == 5)
    {
      ret = inet_aton(cargv[4], &addr);
      if (!ret)
        {
          cmdPrint (cmsh, "Please specify interface address by A.B.C.D\n");
          return (CMD_IPC_WARNING);
        }
      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }
  params->mtu_ignore = 0;
  if (params->mtu_ignore != OSPF_MTU_IGNORE_DEFAULT)
    SET_IF_PARAM (params, mtu_ignore);
  else
    {
      UNSET_IF_PARAM (params, mtu_ignore);
      if (params != IF_DEF_PARAMS (ifp))
        {
          ospf_free_if_params (ifp, addr);
          ospf_if_update_params (ifp, addr);
        }
    }
  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoIpMtuIgnore,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
      "no ip ospf mtu-ignore",
      "Negate a command or set its defaults",
      "IP Information",
      "OSPF interface commands",
      "Disable mtu mismatch detection");

DECMD (cmdFuncOspfMaxMetricRouterLsaAdmin,
       CMD_NODE_CONFIG_OSPF,
       IPC_OSPF,
       "max-metric router-lsa administrative",
       "OSPF maximum / infinite-distance metric",
       "Advertise own Router-LSA with infinite distance (stub router)",
       "Administratively applied for an indefinite period")
{
  struct listnode *ln;
  struct ospf_area *area;
  struct ospf *ospf = om->index;

  for (ALL_LIST_ELEMENTS_RO (ospf->areas, ln, area))
    {
      SET_FLAG (area->stub_router_state, OSPF_AREA_ADMIN_STUB_ROUTED);

      if (!CHECK_FLAG (area->stub_router_state, OSPF_AREA_IS_STUB_ROUTED))
          ospf_router_lsa_update_area (area);
    }

  /* Allows for areas configured later to get the property */
  ospf->stub_router_admin_set = OSPF_STUB_ROUTER_ADMINISTRATIVE_SET;

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoMaxMetricRouterLsaAdmin,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "no max-metric router-lsa administrative",
       "Negate a command or set its defaults",
       "OSPF maximum / infinite-distance metric",
       "Advertise own Router-LSA with infinite distance (stub router)",
       "Administratively applied for an indefinite period")
{
  struct listnode *ln;
  struct ospf_area *area;
  struct ospf *ospf = om->index;

  for (ALL_LIST_ELEMENTS_RO (ospf->areas, ln, area))
    {
      UNSET_FLAG (area->stub_router_state, OSPF_AREA_ADMIN_STUB_ROUTED);

      /* Don't trample on the start-up stub timer */
      if (CHECK_FLAG (area->stub_router_state, OSPF_AREA_IS_STUB_ROUTED)
          && !area->pTimerStubRouter)
        {
          UNSET_FLAG (area->stub_router_state, OSPF_AREA_IS_STUB_ROUTED);
          ospf_router_lsa_update_area (area);
        }
    }
  ospf->stub_router_admin_set = OSPF_STUB_ROUTER_ADMINISTRATIVE_UNSET;
  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfMaxMetricRouterLsaStartup,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "max-metric router-lsa on-startup <5-86400>",
       "OSPF maximum / infinite-distance metric",
       "Advertise own Router-LSA with infinite distance (stub router)",
       "Automatically advertise stub Router-LSA on startup of OSPF",
       "Time (seconds) to advertise self as stub-router")
{
  unsigned int seconds;
  struct ospf *ospf = om->index;

  if (cargc != 4)
    {
      cmdPrint (cmsh, "%% Must supply stub-router period");
      return (CMD_IPC_WARNING);
    }

  VTY_GET_INTEGER ("stub-router startup period", seconds, cargv[3]);

  ospf->stub_router_startup_time = seconds;

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoMaxMetricRouterLsaStartup,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "no max-metric router-lsa on-startup",
       "Negate a command or set its defaults",
       "OSPF maximum / infinite-distance metric",
       "Advertise own Router-LSA with infinite distance (stub router)",
       "Automatically advertise stub Router-LSA on startup of OSPF")
{
  struct listnode *ln;
  struct ospf_area *area;
  struct ospf *ospf = om->index;

  ospf->stub_router_startup_time = OSPF_STUB_ROUTER_UNCONFIGURED;

  for (ALL_LIST_ELEMENTS_RO (ospf->areas, ln, area))
    {
      SET_FLAG (area->stub_router_state, OSPF_AREA_WAS_START_STUB_ROUTED);
      OSPF_TIMER_OFF (area->pTimerStubRouter);

      /* Don't trample on admin stub routed */
      if (!CHECK_FLAG (area->stub_router_state, OSPF_AREA_ADMIN_STUB_ROUTED))
        {
          UNSET_FLAG (area->stub_router_state, OSPF_AREA_IS_STUB_ROUTED);
          ospf_router_lsa_update_area (area);
        }
    }
  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfMaxMetricRouterLsaShutdown,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "max-metric router-lsa on-shutdown <5-86400>",
       "OSPF maximum / infinite-distance metric",
       "Advertise own Router-LSA with infinite distance (stub router)",
       "Advertise stub-router prior to full shutdown of OSPF",
       "Time (seconds) to wait till full shutdown")
{
  unsigned int seconds;
  struct ospf *ospf = om->index;

  if (cargc != 4)
    {
      cmdPrint (cmsh, "%% Must supply stub-router shutdown period");
      return (CMD_IPC_WARNING);
    }

  VTY_GET_INTEGER ("stub-router shutdown wait period", seconds, cargv[3]);

  ospf->stub_router_shutdown_time = seconds;

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoMaxMetricRouterLsaShutdown,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "no max-metric router-lsa on-shutdown",
       "Negate a command or set its defaults",
       "OSPF maximum / infinite-distance metric",
       "Advertise own Router-LSA with infinite distance (stub router)",
       "Advertise stub-router prior to full shutdown of OSPF")
{
  struct ospf *ospf = om->index;

  ospf->stub_router_shutdown_time = OSPF_STUB_ROUTER_UNCONFIGURED;

  return (CMD_IPC_OK);
}


DECMD (cmdFuncOspfShowIpBorderRouters,
		CMD_NODE_EXEC,
	   IPC_OSPF | IPC_SHOW_MGR,
       "show ip ospf border-routers",
       "Show running system information",
       "IP information",
       "show all the ABR's and ASBR's",
       "for this area")
{
  struct ospf *ospf;

  if ((ospf = ospf_lookup ()) == NULL)
    {
      cmdPrint (cmsh, " OSPF Routing Process not enabled");
      return (CMD_IPC_OK);
    }

  if (ospf->new_table == NULL)
    {
      cmdPrint (cmsh, "No OSPF routing information exist");
      return (CMD_IPC_OK);
    }

  /* Show Network routes.
  show_ip_ospf_route_network (vty, ospf->new_table);   */

  /* Show Router routes. */
  show_ip_ospf_route_router (cmsh, ospf->new_rtrs);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfShowIpRoute,
		CMD_NODE_EXEC,
	   IPC_OSPF | IPC_SHOW_MGR,
       "show ip ospf route",
       "Show running system information",
       "IP information",
       "OSPF information",
       "OSPF routing table")
{
  struct ospf *ospf;

  if ((ospf = ospf_lookup ()) == NULL)
    {
      cmdPrint (cmsh, " OSPF Routing Process not enabled");
      return (CMD_IPC_OK);
    }

  if (ospf->new_table == NULL)
    {
      cmdPrint (cmsh, "No OSPF routing information exist");
      return (CMD_IPC_OK);
    }

  /* Show Network routes. */
  show_ip_ospf_route_network (cmsh, ospf->new_table);

  /* Show Router routes. */
  show_ip_ospf_route_router (cmsh, ospf->new_rtrs);

  /* Show AS External routes. */
  show_ip_ospf_route_external (cmsh, ospf->old_external_route);

  return (CMD_IPC_OK);
}

/*------------------------------------------------------------------------*
 * Followings are configuration functions for Opaque-LSAs handling.
 *------------------------------------------------------------------------*/

DECMD (cmdFuncOspfCapabilityOpaque,
       CMD_NODE_CONFIG_OSPF,
       IPC_OSPF,
       "capability opaque",
       "Enable specific OSPF feature",
       "Opaque LSA")
{
  struct ospf *ospf = (struct ospf *) om->index;

  /* Turn on the "master switch" of opaque-lsa capability. */
  if (!CHECK_FLAG (ospf->config, OSPF_OPAQUE_CAPABLE))
    {
      if (IS_DEBUG_OSPF_EVENT)
        zlog_debug ("Opaque capability: OFF -> ON");

      SET_FLAG (ospf->config, OSPF_OPAQUE_CAPABLE);
      ospf_renegotiate_optional_capabilities (ospf);
    }
  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfCapabilityOpaque,
	   CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "ospf opaque-lsa",
       "OSPF specific commands",
       "Enable the Opaque-LSA capability (rfc2370)");

DECMD (cmdFuncOspfNoCapabilityOpaque,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "no capability opaque",
       "Negate a command or set its defaults",
       "Enable specific OSPF feature",
       "Opaque LSA")
{
  struct ospf *ospf = (struct ospf *) om->index;

  /* Turn off the "master switch" of opaque-lsa capability. */
  if (CHECK_FLAG (ospf->config, OSPF_OPAQUE_CAPABLE))
    {
      if (IS_DEBUG_OSPF_EVENT)
        zlog_debug ("Opaque capability: ON -> OFF");

      UNSET_FLAG (ospf->config, OSPF_OPAQUE_CAPABLE);
      ospf_renegotiate_optional_capabilities (ospf);
    }
  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoCapabilityOpaque,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "no ospf opaque-lsa",
       "Negate a command or set its defaults",
       "OSPF specific commands",
       "Disable the Opaque-LSA capability (rfc2370)");

DECMD (cmdFuncOspfMplsTe,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "mpls-te",
       "Configure MPLS-TE parameters",
       "Enable the MPLS-TE functionality")
{
  struct listnode *node, *nnode;
  struct mpls_te_link *lp;

  if (OspfMplsTE.status == enabled)
    return (CMD_IPC_OK);

  if (IS_DEBUG_OSPF_EVENT)
    zlog_debug ("MPLS-TE: OFF -> ON");

  OspfMplsTE.status = enabled;

  /*
   * Following code is intended to handle two cases;
   *
   * 1) MPLS-TE was disabled at startup time, but now become enabled.
   * 2) MPLS-TE was once enabled then disabled, and now enabled again.
   */
  for (ALL_LIST_ELEMENTS (OspfMplsTE.iflist, node, nnode, lp))
    initialize_linkparams (lp);

  ospf_mpls_te_foreach_area (ospf_mpls_te_lsa_schedule, REORIGINATE_PER_AREA);

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfMplsTe,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "mpls-te on",
       "Configure MPLS-TE parameters",
       "Enable the MPLS-TE functionality");

DECMD (cmdFuncOspfNoMplsTe,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "no mpls-te",
       "Negate a command or set its defaults",
       "Configure MPLS-TE parameters",
       "Disable the MPLS-TE functionality")
{
  struct listnode *node, *nnode;
  struct mpls_te_link *lp;

  if (OspfMplsTE.status == disabled)
    return (CMD_IPC_OK);

  if (IS_DEBUG_OSPF_EVENT)
    zlog_debug ("MPLS-TE: ON -> OFF");

  OspfMplsTE.status = disabled;

  for (ALL_LIST_ELEMENTS (OspfMplsTE.iflist, node, nnode, lp))
    if (lp->area != NULL)
      if (lp->flags & LPFLG_LSA_ENGAGED)
        ospf_mpls_te_lsa_schedule (lp, FLUSH_THIS_LSA);

  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfMplsTeRouterAddr,
		CMD_NODE_CONFIG_OSPF,
	   IPC_OSPF,
       "mpls-te router-address A.B.C.D",
       "Configure MPLS-TE parameters",
       "Stable IP address of the advertising router",
       "MPLS-TE router address in IPv4 address format")
{
  struct te_tlv_router_addr *ra = &OspfMplsTE.router_addr;
  struct in_addr value;

  if (! inet_aton (cargv[2], &value))
    {
      cmdPrint (cmsh, "Please specify Router-Addr by A.B.C.D\n");
      return (CMD_IPC_WARNING);
    }

  if (ntohs (ra->header.type) == 0
      || ntohl (ra->value.s_addr) != ntohl (value.s_addr))
    {
      struct listnode *node, *nnode;
      struct mpls_te_link *lp;
      int need_to_reoriginate = 0;

      set_mpls_te_router_addr (value);

      if (OspfMplsTE.status == disabled)
        goto out;

      for (ALL_LIST_ELEMENTS (OspfMplsTE.iflist, node, nnode, lp))
        {
          if (lp->area == NULL)
            continue;

          if ((lp->flags & LPFLG_LSA_ENGAGED) == 0)
            {
              need_to_reoriginate = 1;
              break;
            }
        }

      for (ALL_LIST_ELEMENTS (OspfMplsTE.iflist, node, nnode, lp))
        {
          if (lp->area == NULL)
            continue;

          if (need_to_reoriginate)
            lp->flags |= LPFLG_LSA_FORCED_REFRESH;
          else
            ospf_mpls_te_lsa_schedule (lp, REFRESH_THIS_LSA);
        }

      if (need_to_reoriginate)
        ospf_mpls_te_foreach_area (
            ospf_mpls_te_lsa_schedule, REORIGINATE_PER_AREA);
    }
out:
  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfMplsTeLinkMetric,
       CMD_NODE_INTERFACE,
       IPC_OSPF,
       "mpls-te link metric <0-4294967295>",
       "Configure MPLS-TE parameters",
       "Configure MPLS-TE link parameters",
       "Link metric for MPLS-TE purpose",
       "Metric")
{
  struct interface *ifp;
  struct mpls_te_link *lp;
  u_int32_t value;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));

  if ((lp = lookup_linkparams_by_ifp (ifp)) == NULL)
    {
      cmdPrint (cmsh, "mpls_te_link_metric: Something wrong!\n");
      return (CMD_IPC_WARNING);
    }

  value = strtoul (cargv[3], NULL, 10);

  if (ntohs (lp->te_metric.header.type) == 0
  ||  ntohl (lp->te_metric.value) != value)
    {
      set_linkparams_te_metric (lp, value);

      if (OspfMplsTE.status == enabled)
        if (lp->area != NULL)
          {
            if (lp->flags & LPFLG_LSA_ENGAGED)
              ospf_mpls_te_lsa_schedule (lp, REFRESH_THIS_LSA);
            else
              ospf_mpls_te_lsa_schedule (lp, REORIGINATE_PER_AREA);
          }
    }
  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfMplsTeLinkMaxbw,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "mpls-te link max-bw WORD",
       "Configure MPLS-TE parameters",
       "Configure MPLS-TE link parameters",
       "Maximum bandwidth that can be used",
       "Bytes/second (IEEE floating point format)")
{
  struct interface *ifp;
  struct mpls_te_link *lp;
  float f1, f2;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));

  if ((lp = lookup_linkparams_by_ifp (ifp)) == NULL)
    {
      cmdPrint (cmsh, "mpls_te_link_maxbw: Something wrong!\n");
      return (CMD_IPC_WARNING);
    }

  ntohf (&lp->max_bw.value, &f1);
  if (sscanf (cargv[3], "%g", &f2) != 1)
    {
      cmdPrint (cmsh, "mpls_te_link_maxbw: fscanf: %s\n", safe_strerror (errno));
      return (CMD_IPC_WARNING);
    }

  if (ntohs (lp->max_bw.header.type) == 0
  ||  f1 != f2)
    {
      set_linkparams_max_bw (lp, &f2);

      if (OspfMplsTE.status == enabled)
        if (lp->area != NULL)
          {
            if (lp->flags & LPFLG_LSA_ENGAGED)
              ospf_mpls_te_lsa_schedule (lp, REFRESH_THIS_LSA);
            else
              ospf_mpls_te_lsa_schedule (lp, REORIGINATE_PER_AREA);
          }
    }
  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfMplsTeLinkMaxRsvBw,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "mpls-te link max-rsv-bw WORD",
       "Configure MPLS-TE parameters",
       "Configure MPLS-TE link parameters",
       "Maximum bandwidth that may be reserved",
       "Bytes/second (IEEE floating point format)")
{
  struct interface *ifp;
  struct mpls_te_link *lp;
  float f1, f2;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));

  if ((lp = lookup_linkparams_by_ifp (ifp)) == NULL)
    {
      cmdPrint (cmsh, "mpls_te_link_max_rsv_bw: Something wrong!\n");
      return (CMD_IPC_WARNING);
    }

  ntohf (&lp->max_rsv_bw.value, &f1);
  if (sscanf (cargv[3], "%g", &f2) != 1)
    {
      cmdPrint (cmsh, "mpls_te_link_max_rsv_bw: fscanf: %s\n", safe_strerror (errno));
      return (CMD_IPC_WARNING);
    }

  if (ntohs (lp->max_rsv_bw.header.type) == 0
  ||  f1 != f2)
    {
      set_linkparams_max_rsv_bw (lp, &f2);

      if (OspfMplsTE.status == enabled)
        if (lp->area != NULL)
          {
            if (lp->flags & LPFLG_LSA_ENGAGED)
              ospf_mpls_te_lsa_schedule (lp, REFRESH_THIS_LSA);
            else
              ospf_mpls_te_lsa_schedule (lp, REORIGINATE_PER_AREA);
          }
    }
  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfMplsTeLinkUnrsvBw,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "mpls-te link unrsv-bw <0-7> WORD",
       "Configure MPLS-TE parameters",
       "Configure MPLS-TE link parameters",
       "Unreserved bandwidth at each priority level",
       "Priority",
       "Bytes/second (IEEE floating point format)")
{
  struct interface *ifp;
  struct mpls_te_link *lp;
  int priority;
  float f1, f2;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));

  if ((lp = lookup_linkparams_by_ifp (ifp)) == NULL)
    {
      cmdPrint (cmsh, "mpls_te_link_unrsv_bw: Something wrong!\n");
      return (CMD_IPC_WARNING);
    }

  /* We don't have to consider about range check here. */
  if (sscanf (cargv[3], "%d", &priority) != 1)
    {
      cmdPrint (cmsh, "mpls_te_link_unrsv_bw: fscanf: %s\n", safe_strerror (errno));
      return (CMD_IPC_WARNING);
    }

  ntohf (&lp->unrsv_bw.value [priority], &f1);
  if (sscanf (cargv[4], "%g", &f2) != 1)
    {
      cmdPrint (cmsh, "mpls_te_link_unrsv_bw: fscanf: %s\n", safe_strerror (errno));
      return (CMD_IPC_WARNING);
    }

  if (ntohs (lp->unrsv_bw.header.type) == 0
  ||  f1 != f2)
    {
      set_linkparams_unrsv_bw (lp, priority, &f2);

      if (OspfMplsTE.status == enabled)
        if (lp->area != NULL)
          {
            if (lp->flags & LPFLG_LSA_ENGAGED)
              ospf_mpls_te_lsa_schedule (lp, REFRESH_THIS_LSA);
            else
              ospf_mpls_te_lsa_schedule (lp, REORIGINATE_PER_AREA);
          }
    }
  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfMplsTeLinkRscClsclr,
		CMD_NODE_INTERFACE,
	   IPC_OSPF,
       "mpls-te link rsc-clsclr WORD",
       "Configure MPLS-TE parameters",
       "Configure MPLS-TE link parameters",
       "Administrative group membership",
       "32-bit Hexadecimal value (ex. 0xa1)")
{
  struct interface *ifp;
  struct mpls_te_link *lp;
  unsigned long value;

  ifp = if_get_by_name_len(uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));

  if ((lp = lookup_linkparams_by_ifp (ifp)) == NULL)
    {
      cmdPrint (cmsh, "mpls_te_link_rsc_clsclr: Something wrong!");
      return (CMD_IPC_WARNING);
    }

  if (sscanf (cargv[3], "0x%lx", &value) != 1)
    {
      cmdPrint (cmsh, "mpls_te_link_rsc_clsclr: fscanf: %s\n", safe_strerror (errno));
      return (CMD_IPC_WARNING);
    }

  if (ntohs (lp->rsc_clsclr.header.type) == 0
  ||  ntohl (lp->rsc_clsclr.value) != value)
    {
      set_linkparams_rsc_clsclr (lp, value);

      if (OspfMplsTE.status == enabled)
        if (lp->area != NULL)
          {
            if (lp->flags & LPFLG_LSA_ENGAGED)
              ospf_mpls_te_lsa_schedule (lp, REFRESH_THIS_LSA);
            else
              ospf_mpls_te_lsa_schedule (lp, REORIGINATE_PER_AREA);
          }
    }
  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfShowMplsTeRouter,
       CMD_NODE_EXEC,
       IPC_OSPF | IPC_SHOW_MGR,
       "show mpls-te router",
       "Show running system information",
       "MPLS-TE information",
       "Router information")
{
  if (OspfMplsTE.status == enabled)
    {
      cmdPrint (cmsh, "--- MPLS-TE router parameters ---");

      if (ntohs (OspfMplsTE.router_addr.header.type) != 0)
        show_vty_router_addr (cmsh, &OspfMplsTE.router_addr.header);
      else if (cmsh != NULL)
        cmdPrint (cmsh, "  N/A");
    }
  return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfShowMplsTeLink,
		CMD_NODE_EXEC,
	   IPC_OSPF | IPC_SHOW_MGR,
       "show mpls-te interface WORD",
       "Show running system information",
       "MPLS-TE information",
       "Interface information",
       "Interface name")
{
  struct interface *ifp;
  struct listnode *node, *nnode;

  /* Show All Interfaces. */
  if (cargc == 3)
    {
      for (ALL_LIST_ELEMENTS (iflist, node, nnode, ifp))
        show_mpls_te_link_sub (cmsh, ifp);
    }
  /* Interface name is specified. */
  else
    {
      if ((ifp = if_lookup_by_name (cargv[3])) == NULL)
      {
        cmdPrint (cmsh, "No such interface name");
      }
      else
      {
        show_mpls_te_link_sub (cmsh, ifp);
      }
    }

  return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfShowMplsTeLink,
		CMD_NODE_EXEC,
	   IPC_OSPF | IPC_SHOW_MGR,
       "show mpls-te interface",
       "Show running system information",
       "MPLS-TE information",
       "Interface information");

DECMD (cmdFuncOspfDebugPacket,
       CMD_NODE_CONFIG,
       IPC_OSPF,
       "debug ospf packet (hello|dd|ls-request|ls-update|ls-ack|all)",
       "Debugging functions (see also 'undebug')",
       "OSPF information",
       "OSPF packets",
       "OSPF Hello",
       "OSPF Database Description",
       "OSPF Link State Request",
       "OSPF Link State Update",
       "OSPF Link State Acknowledgment",
       "OSPF all packets")
{
	int type = 0;
	int flag = 0;
	int i;

	assert(cargc > 3);

	/* Check packet type. */
	if (strncmp(cargv[3], "h", 1) == 0)
		type = OSPF_DEBUG_HELLO;
	else if (strncmp(cargv[3], "d", 1) == 0)
		type = OSPF_DEBUG_DB_DESC;
	else if (strncmp(cargv[3], "ls-r", 4) == 0)
		type = OSPF_DEBUG_LS_REQ;
	else if (strncmp(cargv[3], "ls-u", 4) == 0)
		type = OSPF_DEBUG_LS_UPD;
	else if (strncmp(cargv[3], "ls-a", 4) == 0)
		type = OSPF_DEBUG_LS_ACK;
	else if (strncmp(cargv[3], "a", 1) == 0)
		type = OSPF_DEBUG_ALL;

	/* Default, both send and recv. */
	if (cargc == 4)
		flag = OSPF_DEBUG_SEND | OSPF_DEBUG_RECV;

	/* send or recv. */
	if (cargc >= 5) {
		if (strncmp(cargv[4], "s", 1) == 0)
			flag = OSPF_DEBUG_SEND;
		else if (strncmp(cargv[4], "r", 1) == 0)
			flag = OSPF_DEBUG_RECV;
		else if (strncmp(cargv[4], "d", 1) == 0)
			flag = OSPF_DEBUG_SEND | OSPF_DEBUG_RECV | OSPF_DEBUG_DETAIL;
	}

	/* detail. */
	if (cargc == 6)
		if (strncmp(cargv[5], "d", 1) == 0)
			flag |= OSPF_DEBUG_DETAIL;

	for (i = 0; i < 5; i++)
		if (type & (0x01 << i)) {
			DEBUG_PACKET_ON(i, flag);
		}

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfDebugPacket,
	   CMD_NODE_CONFIG,
	   IPC_OSPF,
       "debug ospf packet (hello|dd|ls-request|ls-update|ls-ack|all) (send|recv|detail)",
       "Debugging functions",
       "OSPF information",
       "OSPF packets",
       "OSPF Hello",
       "OSPF Database Description",
       "OSPF Link State Request",
       "OSPF Link State Update",
       "OSPF Link State Acknowledgment",
       "OSPF all packets",
       "Packet sent",
       "Packet received",
       "Detail information");

ALICMD (cmdFuncOspfDebugPacket,
	   CMD_NODE_CONFIG,
	   IPC_OSPF,
       "debug ospf packet (hello|dd|ls-request|ls-update|ls-ack|all) (send|recv) (detail|)",
       "Debugging functions",
       "OSPF information",
       "OSPF packets",
       "OSPF Hello",
       "OSPF Database Description",
       "OSPF Link State Request",
       "OSPF Link State Update",
       "OSPF Link State Acknowledgment",
       "OSPF all packets",
       "Packet sent",
       "Packet received",
       "Detail Information");


DECMD (cmdFuncOspfNoDebugPacket,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "no debug ospf packet (hello|dd|ls-request|ls-update|ls-ack|all)",
       "Negate a command or set its defaults",
       "Debugging functions (see also 'undebug')",
       "OSPF information",
       "OSPF packets",
       "OSPF Hello",
       "OSPF Database Description",
       "OSPF Link State Request",
       "OSPF Link State Update",
       "OSPF Link State Acknowledgment",
       "OSPF all packets")
{
	int type = 0;
	int flag = 0;
	int i;

	assert(cargc > 4);

	/* Check packet type. */
	if (strncmp(cargv[4], "h", 1) == 0)
		type = OSPF_DEBUG_HELLO;
	else if (strncmp(cargv[4], "d", 1) == 0)
		type = OSPF_DEBUG_DB_DESC;
	else if (strncmp(cargv[4], "ls-r", 4) == 0)
		type = OSPF_DEBUG_LS_REQ;
	else if (strncmp(cargv[4], "ls-u", 4) == 0)
		type = OSPF_DEBUG_LS_UPD;
	else if (strncmp(cargv[4], "ls-a", 4) == 0)
		type = OSPF_DEBUG_LS_ACK;
	else if (strncmp(cargv[4], "a", 1) == 0)
		type = OSPF_DEBUG_ALL;

	/* Default, both send and recv. */
	if (cargc == 5)
		flag = OSPF_DEBUG_SEND | OSPF_DEBUG_RECV | OSPF_DEBUG_DETAIL;

	/* send or recv. */
	if (cargc == 6) {
		if (strncmp(cargv[5], "s", 1) == 0)
			flag = OSPF_DEBUG_SEND | OSPF_DEBUG_DETAIL;
		else if (strncmp(cargv[5], "r", 1) == 0)
			flag = OSPF_DEBUG_RECV | OSPF_DEBUG_DETAIL;
		else if (strncmp(cargv[5], "d", 1) == 0)
			flag = OSPF_DEBUG_DETAIL;
	}

	/* detail. */
	if (cargc == 7)
		if (strncmp(cargv[6], "d", 1) == 0)
			flag = OSPF_DEBUG_DETAIL;

	for (i = 0; i < 5; i++)
		if (type & (0x01 << i)) {
			DEBUG_PACKET_OFF(i, flag);
		}

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoDebugPacket,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "no debug ospf packet (hello|dd|ls-request|ls-update|ls-ack|all) (send|recv|detail)",
       "Negate a command or set its defaults",
       "Debugging functions",
       "OSPF information",
       "OSPF packets",
       "OSPF Hello",
       "OSPF Database Description",
       "OSPF Link State Request",
       "OSPF Link State Update",
       "OSPF Link State Acknowledgment",
       "OSPF all packets",
       "Packet sent",
       "Packet received"
       "Detail Information");

ALICMD (cmdFuncOspfNoDebugPacket,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "no debug ospf packet (hello|dd|ls-request|ls-update|ls-ack|all) (send|recv) (detail|)",
       "Negate a command or set its defaults",
       "Debugging functions",
       "OSPF information",
       "OSPF packets",
       "OSPF Hello",
       "OSPF Database Description",
       "OSPF Link State Request",
       "OSPF Link State Update",
       "OSPF Link State Acknowledgment",
       "OSPF all packets",
       "Packet sent",
       "Packet received",
       "Detail Information");


DECMD (cmdFuncOspfdebug_ospf_ism,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "debug ospf ism",
       "Debugging functions (see also 'undebug')",
       "OSPF information",
       "OSPF Interface State Machine")
{
	if (cargc == 3)
		DEBUG_ON(ism, ISM);
	else if (cargc == 4) {
		if (strncmp(cargv[3], "s", 1) == 0)
			DEBUG_ON(ism, ISM_STATUS);
		else if (strncmp(cargv[3], "e", 1) == 0)
			DEBUG_ON(ism, ISM_EVENTS);
		else if (strncmp(cargv[3], "t", 1) == 0)
			DEBUG_ON(ism, ISM_TIMERS);
	}

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfdebug_ospf_ism,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "debug ospf ism (status|events|timers)",
       "Debugging functions (see also 'undebug')",
       "OSPF information",
       "OSPF Interface State Machine",
       "ISM Status Information",
       "ISM Event Information",
       "ISM TImer Information");

DECMD (cmdFuncOspfNoDebugIsm,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "no debug ospf ism",
       "Negate a command or set its defaults",
       "Debugging functions (see also 'undebug')",
       "OSPF information",
       "OSPF Interface State Machine")
{
	if (cargc == 4)
		DEBUG_OFF(ism, ISM);
	else if (cargc == 5) {
		if (strncmp(cargv[4], "s", 1) == 0)
			DEBUG_OFF(ism, ISM_STATUS);
		else if (strncmp(cargv[4], "e", 1) == 0)
			DEBUG_OFF(ism, ISM_EVENTS);
		else if (strncmp(cargv[4], "t", 1) == 0)
			DEBUG_OFF(ism, ISM_TIMERS);
	}
	return (CMD_IPC_OK);

}

ALICMD (cmdFuncOspfNoDebugIsm,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "no debug ospf ism (status|events|timers)",
       "Negate a command or set its defaults",
       "Debugging functions",
       "OSPF information",
       "OSPF Interface State Machine",
       "ISM Status Information",
       "ISM Event Information",
       "ISM Timer Information");


DECMD (cmdFuncOspfDebugNsm,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "debug ospf nsm",
       "Debugging functions (see also 'undebug')",
       "OSPF information",
       "OSPF Neighbor State Machine")
{
	if (cargc == 3)
		DEBUG_ON(nsm, NSM);
	else if (cargc == 4) {
		if (strncmp(cargv[3], "s", 1) == 0)
			DEBUG_ON(nsm, NSM_STATUS);
		else if (strncmp(cargv[3], "e", 1) == 0)
			DEBUG_ON(nsm, NSM_EVENTS);
		else if (strncmp(cargv[3], "t", 1) == 0)
			DEBUG_ON(nsm, NSM_TIMERS);
	}

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfDebugNsm,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "debug ospf nsm (status|events|timers)",
       "Debugging functions (see also 'undebug')",
       "OSPF information",
       "OSPF Neighbor State Machine",
       "NSM Status Information",
       "NSM Event Information",
       "NSM Timer Information");

DECMD (cmdFuncOspfNoDebugNsm,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "no debug ospf nsm",
       "Negate a command or set its defaults",
       "Debugging functions (see also 'undebug')",
       "OSPF information",
       "OSPF Neighbor State Machine")
{
	if (cargc == 4)
		DEBUG_OFF(nsm, NSM);
	else if (cargc == 5) {
		if (strncmp(cargv[4], "s", 1) == 0)
			DEBUG_OFF(nsm, NSM_STATUS);
		else if (strncmp(cargv[4], "e", 1) == 0)
			DEBUG_OFF(nsm, NSM_EVENTS);
		else if (strncmp(cargv[4], "t", 1) == 0)
			DEBUG_OFF(nsm, NSM_TIMERS);
	}

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoDebugNsm,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "no debug ospf nsm (status|events|timers)",
       "Negate a command or set its defaults",
       "Debugging functions",
       "OSPF information",
       "OSPF Interface State Machine",
       "NSM Status Information",
       "NSM Event Information",
       "NSM Timer Information");


DECMD (cmdFuncOspfDebugLsa,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "debug ospf lsa",
       "Debugging functions (see also 'undebug')",
       "OSPF information",
       "OSPF Link State Advertisement")
{
	if (cargc == 3)
		DEBUG_ON(lsa, LSA);
	else if (cargc == 4) {
		if (strncmp(cargv[3], "g", 1) == 0)
			DEBUG_ON(lsa, LSA_GENERATE);
		else if (strncmp(cargv[3], "f", 1) == 0)
			DEBUG_ON(lsa, LSA_FLOODING);
		else if (strncmp(cargv[3], "i", 1) == 0)
			DEBUG_ON(lsa, LSA_INSTALL);
		else if (strncmp(cargv[3], "r", 1) == 0)
			DEBUG_ON(lsa, LSA_REFRESH);
	}

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfDebugLsa,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "debug ospf lsa (generate|flooding|install|refresh)",
       "Debugging functions (see also 'undebug')",
       "OSPF information",
       "OSPF Link State Advertisement",
       "LSA Generation",
       "LSA Flooding",
       "LSA Install/Delete",
       "LSA Refresh");

DECMD (cmdFuncOspfNoDebugLsa,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "no debug ospf lsa",
       "Negate a command or set its defaults",
       "Debugging functions (see also 'undebug')",
       "OSPF information",
       "OSPF Link State Advertisement")
{
	if (cargc == 4)
		DEBUG_OFF(lsa, LSA);
	else if (cargc == 5) {
		if (strncmp(cargv[4], "g", 1) == 0)
			DEBUG_OFF(lsa, LSA_GENERATE);
		else if (strncmp(cargv[4], "f", 1) == 0)
			DEBUG_OFF(lsa, LSA_FLOODING);
		else if (strncmp(cargv[4], "i", 1) == 0)
			DEBUG_OFF(lsa, LSA_INSTALL);
		else if (strncmp(cargv[4], "r", 1) == 0)
			DEBUG_OFF(lsa, LSA_REFRESH);
	}

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoDebugLsa,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "no debug ospf lsa (generate|flooding|install|refresh)",
       "Negate a command or set its defaults",
       "Debugging functions (see also 'undebug')",
       "OSPF information",
       "OSPF Link State Advertisement",
       "LSA Generation",
       "LSA Flooding",
       "LSA Install/Delete",
       "LSA Refres");


DECMD (cmdFuncOspfDebugZebra,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "debug ospf zebra",
       "Debugging functions (see also 'undebug')",
       "OSPF information",
       "OSPF Zebra information")
{
	if (cargc == 3)
		DEBUG_ON(zebra, ZEBRA);
	else if (cargc == 4) {
		if (strncmp(cargv[3], "i", 1) == 0)
			DEBUG_ON(zebra, ZEBRA_INTERFACE);
		else if (strncmp(cargv[3], "r", 1) == 0)
			DEBUG_ON(zebra, ZEBRA_REDISTRIBUTE);
	}

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfDebugZebra,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "debug ospf zebra (interface|redistribute)",
       "Debugging functions (see also 'undebug')",
       "OSPF information",
       "OSPF Zebra information",
       "Zebra interface",
       "Zebra redistribute");

DECMD (cmdFuncOspfNoDebugZebra,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "no debug ospf zebra",
       "Negate a command or set its defaults",
       "Debugging functions (see also 'undebug')",
       "OSPF information",
       "OSPF Zebra information")
{
	if (cargc == 4)
		DEBUG_OFF(zebra, ZEBRA);
	else if (cargc == 5) {
		if (strncmp(cargv[4], "i", 1) == 0)
			DEBUG_OFF(zebra, ZEBRA_INTERFACE);
		else if (strncmp(cargv[4], "r", 1) == 0)
			DEBUG_OFF(zebra, ZEBRA_REDISTRIBUTE);
	}

	return (CMD_IPC_OK);
}

ALICMD (cmdFuncOspfNoDebugZebra,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "no debug ospf zebra (interface|redistribute)",
       "Negate a command or set its defaults",
       "Debugging functions (see also 'undebug')",
       "OSPF information",
       "OSPF Zebra information",
       "Zebra interface",
       "Zebra redistribute");

DECMD (cmdFuncOspfDebugEvent,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "debug ospf event",
       "Debugging functions (see also 'undebug')",
       "OSPF information",
       "OSPF event information")
{
	CONF_DEBUG_ON(event, EVENT);
	return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoDebugEvent,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "no debug ospf event",
       "Negate a command or set its defaults",
       "Debugging functions (see also 'undebug')",
       "OSPF information",
       "OSPF event information")
{
	CONF_DEBUG_OFF(event, EVENT);
	return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfDebugNssa,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "debug ospf nssa",
       "Debugging functions (see also 'undebug')",
       "OSPF information",
       "OSPF nssa information")
{
	CONF_DEBUG_ON(nssa, NSSA);
	return (CMD_IPC_OK);
}

DECMD (cmdFuncOspfNoDebugNssa,
		CMD_NODE_CONFIG,
	   IPC_OSPF,
       "no debug ospf nssa",
       "Negate a command or set its defaults",
       "Debugging functions (see also 'undebug')",
       "OSPF information",
       "OSPF nssa information")
{
	CONF_DEBUG_OFF(nssa, NSSA);
	return (CMD_IPC_OK);
}


DECMD (cmdFuncOspfMatchIpNexthop,
		CMD_NODE_CONFIG_ROUTE_MAP,
		IPC_OSPF,
       "match ip next-hop (<1-199>|<1300-2699>|WORD)",
       "Match values from routing table",
       "IP information",
       "Match next-hop address of route",
       "IP access-list number",
       "IP access-list number (expanded range)",
       "IP access-list name")
{
	struct route_map_index *idx;

	idx = OspfGetRoutemap(cmsh, uargv1[1], uargv1[2], uargv1[3]);
	if(idx == NULL) return CMD_IPC_WARNING;

	return ospf_route_match_add(cmsh, idx, "ip next-hop", cargv[3]);
}

DECMD (cmdFuncOspfNoMatchIpNexthop,
		CMD_NODE_CONFIG_ROUTE_MAP,
		IPC_OSPF,
       "no match ip next-hop",
       "Negate a command or set its defaults",
       "Match values from routing table",
       "IP information",
       "Match next-hop address of route")
{
	struct route_map_index *idx;

	idx = OspfGetRoutemap(cmsh, uargv1[1], uargv1[2], uargv1[3]);
	if(idx == NULL) return CMD_IPC_WARNING;

	if (cargc == 4)
		return ospf_route_match_delete(cmsh, idx, "ip next-hop", NULL);

	return ospf_route_match_delete(cmsh, idx, "ip next-hop", cargv[4]);
}

ALICMD (cmdFuncOspfNoMatchIpNexthop,
		CMD_NODE_CONFIG_ROUTE_MAP,
		IPC_OSPF,
       "no match ip next-hop (<1-199>|<1300-2699>|WORD)",
       "Negate a command or set its defaults",
       "Match values from routing table",
       "IP information",
       "Match next-hop address of route",
       "IP access-list number",
       "IP access-list number (expanded range)",
       "IP access-list name");

DECMD (cmdFuncOspfMatchIpNextHopPrefixList,
		CMD_NODE_CONFIG_ROUTE_MAP,
		IPC_OSPF,
       "match ip next-hop prefix-list WORD",
       "Match values from routing table",
       "IP information",
       "Match next-hop address of route",
       "Match entries of prefix-lists",
       "IP prefix-list name")
{
	struct route_map_index *idx;

	idx = OspfGetRoutemap(cmsh, uargv1[1], uargv1[2], uargv1[3]);
	if(idx == NULL) return CMD_IPC_WARNING;

	return ospf_route_match_add(cmsh, idx, "ip next-hop prefix-list", cargv[4]);
}

DECMD (cmdFuncOspfNoMatchIpNextHopPrefixList,
		CMD_NODE_CONFIG_ROUTE_MAP,
		IPC_OSPF,
       "no match ip next-hop prefix-list",
       "Negate a command or set its defaults",
       "Match values from routing table",
       "IP information",
       "Match next-hop address of route",
       "Match entries of prefix-lists")
{
	struct route_map_index *idx;

	idx = OspfGetRoutemap(cmsh, uargv1[1], uargv1[2], uargv1[3]);
	if(idx == NULL) return CMD_IPC_WARNING;

	if (cargc == 5)
		return ospf_route_match_delete(cmsh, idx, "ip next-hop prefix-list", NULL);
	return ospf_route_match_delete(cmsh, idx, "ip next-hop prefix-list", cargv[5]);
}

ALICMD (cmdFuncOspfNoMatchIpNextHopPrefixList,
		CMD_NODE_CONFIG_ROUTE_MAP,
		IPC_OSPF,
       "no match ip next-hop prefix-list WORD",
       "Negate a command or set its defaults",
       "Match values from routing table",
       "IP information",
       "Match next-hop address of route",
       "Match entries of prefix-lists",
       "IP prefix-list name");

DECMD (cmdFuncOspfMatchIpAddress,
		CMD_NODE_CONFIG_ROUTE_MAP,
		IPC_OSPF,
       "match ip address (<1-199>|<1300-2699>|WORD)",
       "Match values from routing table",
       "IP information",
       "Match address of route",
       "IP access-list number",
       "IP access-list number (expanded range)",
       "IP access-list name")
{
	struct route_map_index *idx;

	idx = OspfGetRoutemap(cmsh, uargv1[1], uargv1[2], uargv1[3]);
	if(idx == NULL) return CMD_IPC_WARNING;

	return ospf_route_match_add(cmsh, idx, "ip address", cargv[3]);
}

DECMD (cmdFuncOspfNoMatchIpAddress,
		CMD_NODE_CONFIG_ROUTE_MAP,
		IPC_OSPF,
       "no match ip address",
       "Negate a command or set its defaults",
       "Match values from routing table",
       "IP information",
       "Match address of route")
{
	struct route_map_index *idx;

	idx = OspfGetRoutemap(cmsh, uargv1[1], uargv1[2], uargv1[3]);
	if(idx == NULL) return CMD_IPC_WARNING;

	if (cargc == 4)
		return ospf_route_match_delete(cmsh, idx, "ip address", NULL);

	return ospf_route_match_delete(cmsh, idx, "ip address", cargv[4]);
}

ALICMD (cmdFuncOspfNoMatchIpAddress,
		CMD_NODE_CONFIG_ROUTE_MAP,
		IPC_OSPF,
       "no match ip address (<1-199>|<1300-2699>|WORD)",
       "Negate a command or set its defaults",
       "Match values from routing table",
       "IP information",
       "Match address of route",
       "IP access-list number",
       "IP access-list number (expanded range)",
       "IP access-list name");

DECMD (cmdFuncOspfMatchIpAddressPrefixList,
		CMD_NODE_CONFIG_ROUTE_MAP,
		IPC_OSPF,
       "match ip address prefix-list WORD",
       "Match values from routing table",
       "IP information",
       "Match address of route",
       "Match entries of prefix-lists",
       "IP prefix-list name")
{
	struct route_map_index *idx;

	idx = OspfGetRoutemap(cmsh, uargv1[1], uargv1[2], uargv1[3]);
	if(idx == NULL) return CMD_IPC_WARNING;

	return ospf_route_match_add(cmsh, idx, "ip address prefix-list", cargv[4]);
}

DECMD (cmdFuncOspfNoMatchIpAddressPrefixList,
		CMD_NODE_CONFIG_ROUTE_MAP,
		IPC_OSPF,
       "no match ip address prefix-list",
       "Negate a command or set its defaults",
       "Match values from routing table",
       "IP information",
       "Match address of route",
       "Match entries of prefix-lists")
{
	struct route_map_index *idx;

	idx = OspfGetRoutemap(cmsh, uargv1[1], uargv1[2], uargv1[3]);
	if(idx == NULL) return CMD_IPC_WARNING;

	if (cargc == 5)
		return ospf_route_match_delete(cmsh, idx, "ip address prefix-list", NULL);
	return ospf_route_match_delete(cmsh, idx, "ip address prefix-list", cargv[5]);
}

ALICMD (cmdFuncOspfNoMatchIpAddressPrefixList,
		CMD_NODE_CONFIG_ROUTE_MAP,
		IPC_OSPF,
       "no match ip address prefix-list WORD",
       "Negate a command or set its defaults",
       "Match values from routing table",
       "IP information",
       "Match address of route",
       "Match entries of prefix-lists",
       "IP prefix-list name");

DECMD (cmdFuncOspfMatchInterface,
		CMD_NODE_CONFIG_ROUTE_MAP,
		IPC_OSPF,
       "match interface WORD",
       "Match values from routing table",
       "Match first hop interface of route",
       "Interface name")
{
	struct route_map_index *idx;

	idx = OspfGetRoutemap(cmsh, uargv1[1], uargv1[2], uargv1[3]);
	if(idx == NULL) return CMD_IPC_WARNING;

	return ospf_route_match_add(cmsh, idx, "interface", cargv[2]);
}

DECMD (cmdFuncOspfNoMatchInterface,
		CMD_NODE_CONFIG_ROUTE_MAP,
		IPC_OSPF,
       "no match interface",
       "Negate a command or set its defaults",
       "Match values from routing table",
       "Match first hop interface of route")
{
	struct route_map_index *idx;

	idx = OspfGetRoutemap(cmsh, uargv1[1], uargv1[2], uargv1[3]);
	if(idx == NULL) return CMD_IPC_WARNING;

	if (cargc == 3)
		return ospf_route_match_delete(cmsh, idx, "interface", NULL);

	return ospf_route_match_delete(cmsh, idx, "interface", cargv[3]);
}

ALICMD (cmdFuncOspfNoMatchInterface,
		CMD_NODE_CONFIG_ROUTE_MAP,
		IPC_OSPF,
       "no match interface WORD",
       "Negate a command or set its defaults",
       "Match values from routing table",
       "Match first hop interface of route",
       "Interface name");

DECMD (cmdFuncOspfSetMetric,
		CMD_NODE_CONFIG_ROUTE_MAP,
		IPC_OSPF,
       "set metric <0-4294967295>",
       "Set values in destination routing protocol",
       "Metric value for destination routing protocol",
       "Metric value")
{
	struct route_map_index *idx;

	idx = OspfGetRoutemap(cmsh, uargv1[1], uargv1[2], uargv1[3]);
	if(idx == NULL) return CMD_IPC_WARNING;

	return ospf_route_set_add(cmsh, idx, "metric", cargv[2]);
}

DECMD (cmdFuncOspfNoSetMetric,
		CMD_NODE_CONFIG_ROUTE_MAP,
		IPC_OSPF,
       "no set metric",
       "Negate a command or set its defaults",
       "Set values in destination routing protocol",
       "Metric value for destination routing protocol")
{
	struct route_map_index *idx;

	idx = OspfGetRoutemap(cmsh, uargv1[1], uargv1[2], uargv1[3]);
	if(idx == NULL) return CMD_IPC_WARNING;

	if (cargc == 3)
		return ospf_route_set_delete(cmsh, idx, "metric", NULL);

	return ospf_route_set_delete(cmsh, idx, "metric", cargv[3]);
}

ALICMD (cmdFuncOspfNoSetMetric,
		CMD_NODE_CONFIG_ROUTE_MAP,
		IPC_OSPF,
       "no set metric <0-4294967295>",
       "Negate a command or set its defaults",
       "Set values in destination routing protocol",
       "Metric value for destination routing protocol",
       "Metric value");

DECMD (cmdFuncOspfSetMetricType,
		CMD_NODE_CONFIG_ROUTE_MAP,
		IPC_OSPF,
       "set metric-type (type-1|type-2)",
       "Set values in destination routing protocol",
       "Type of metric for destination routing protocol",
       "OSPF[6] external type 1 metric",
       "OSPF[6] external type 2 metric")
{
	struct route_map_index *idx;

	idx = OspfGetRoutemap(cmsh, uargv1[1], uargv1[2], uargv1[3]);
	if(idx == NULL) return CMD_IPC_WARNING;

	return ospf_route_set_add(cmsh, idx, "metric-type", cargv[2]);
}

DECMD (cmdFuncOspfNoSetMetricType,
		CMD_NODE_CONFIG_ROUTE_MAP,
		IPC_OSPF,
       "no set metric-type",
       "Negate a command or set its defaults",
       "Set values in destination routing protocol",
       "Type of metric for destination routing protocol")
{
	struct route_map_index *idx;

	idx = OspfGetRoutemap(cmsh, uargv1[1], uargv1[2], uargv1[3]);
	if(idx == NULL) return CMD_IPC_WARNING;

	if (cargc == 3)
		return ospf_route_set_delete(cmsh, idx, "metric-type", NULL);

	return ospf_route_set_delete(cmsh, idx, "metric-type", cargv[3]);
}

ALICMD (cmdFuncOspfNoSetMetricType,
		CMD_NODE_CONFIG_ROUTE_MAP,
		IPC_OSPF,
       "no set metric-type (type-1|type-2)",
       "Negate a command or set its defaults",
       "Set values in destination routing protocol",
       "Type of metric for destination routing protocol",
       "OSPF[6] external type 1 metric",
       "OSPF[6] external type 2 metric");
