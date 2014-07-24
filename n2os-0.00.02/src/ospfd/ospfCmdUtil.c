/*
 * ospfCmdUtil.c
 *
 *  Created on: 2014. 6. 13.
 *      Author: root
 */

#include <netinet/in.h>

#include "log.h"
#include "prefix.h"
#include "table.h"
#include "vty.h"

#include "nnTypes.h"
#include "nnCmdCmsh.h"
#include "nnCmdCommon.h"

#include "ospfd.h"
#include "ospf_asbr.h"
#include "ospf_lsa.h"
#include "ospf_dump.h"
#include "ospf_lsdb.h"
#include "ospf_nsm.h"
#include "ospf_packet.h"
#include "ospf_neighbor.h"
#include "ospf_route.h"
#include "ospf_te.h"
#include "ospf_interface.h"
#include "ospfCmdUtil.h"
#include "ospf_vty.h"
#include "ospf_abr.h"
#include "ospf_flood.h"
#include "ospf_zebra.h"

void
ospf_passive_interface_default (struct ospf *ospf, u_char newval)
{
  struct listnode *ln;
  struct interface *ifp;
  struct ospf_interface *oi;

  ospf->passive_interface_default = newval;

  for (ALL_LIST_ELEMENTS_RO (om->iflist, ln, ifp))
    {
      if (ifp &&
          OSPF_IF_PARAM_CONFIGURED (IF_DEF_PARAMS (ifp), passive_interface))
        UNSET_IF_PARAM (IF_DEF_PARAMS (ifp), passive_interface);
    }
  for (ALL_LIST_ELEMENTS_RO (ospf->oiflist, ln, oi))
    {
      if (OSPF_IF_PARAM_CONFIGURED (oi->params, passive_interface))
        UNSET_IF_PARAM (oi->params, passive_interface);
      /* update multicast memberships */
      ospf_if_set_multicast(oi);
    }
}

void
ospf_passive_interface_update (struct ospf *ospf, struct interface *ifp,
                               struct in_addr addr,
                               struct ospf_if_params *params, u_char value)
{
  u_char dflt;

  params->passive_interface = value;
  if (params != IF_DEF_PARAMS (ifp))
    {
      if (OSPF_IF_PARAM_CONFIGURED (IF_DEF_PARAMS (ifp), passive_interface))
        dflt = IF_DEF_PARAMS (ifp)->passive_interface;
      else
        dflt = ospf->passive_interface_default;

      if (value != dflt)
        SET_IF_PARAM (params, passive_interface);
      else
        UNSET_IF_PARAM (params, passive_interface);

      ospf_free_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }
  else
    {
      if (value != ospf->passive_interface_default)
        SET_IF_PARAM (params, passive_interface);
      else
        UNSET_IF_PARAM (params, passive_interface);
    }
}



void
ospf_vl_config_data_init (struct ospf_vl_config_data *vl_config,
			  struct cmsh *cmsh)
{
  memset (vl_config, 0, sizeof (struct ospf_vl_config_data));
  vl_config->auth_type = OSPF_AUTH_CMD_NOTSEEN;
  vl_config->cmsh = cmsh;
}

int
ospf_area_nssa_cmd_handler (struct cmsh *cmsh, int argc, Int8T *argv[],
                            int nosum)
{
  struct ospf *ospf = om->index;
  struct in_addr area_id;
  int ret, format;

  VTY_GET_OSPF_AREA_ID_NO_BB ("NSSA", area_id, format, argv[1]);

  ret = ospf_area_nssa_set (ospf, area_id);
  if (ret == 0)
    {
      cmdPrint (cmsh, "%% Area cannot be nssa as it contains a virtual link\n");
      return (CMD_IPC_WARNING);
    }

  if (argc > 3 && strncmp(argv[3],"no-s",4))
    {
      if (strncmp (argv[3], "translate-c", 11) == 0)
        ospf_area_nssa_translator_role_set (ospf, area_id,
					    OSPF_NSSA_ROLE_CANDIDATE);
      else if (strncmp (argv[3], "translate-n", 11) == 0)
        ospf_area_nssa_translator_role_set (ospf, area_id,
					    OSPF_NSSA_ROLE_NEVER);
      else if (strncmp (argv[3], "translate-a", 11) == 0)
        ospf_area_nssa_translator_role_set (ospf, area_id,
					    OSPF_NSSA_ROLE_ALWAYS);
    }
  else
    {
      ospf_area_nssa_translator_role_set (ospf, area_id,
                        OSPF_NSSA_ROLE_CANDIDATE);
    }

  if (nosum)
    ospf_area_no_summary_set (ospf, area_id);
  else
    ospf_area_no_summary_unset (ospf, area_id);

  ospf_schedule_abr_task (ospf);

  return (CMD_IPC_OK);
}



static struct ospf_vl_data *
ospf_find_vl_data (struct ospf *ospf, struct ospf_vl_config_data *vl_config)
{
	struct ospf_area *area;
	struct ospf_vl_data *vl_data;
	struct cmsh *cmsh;
	struct in_addr area_id;

	cmsh = vl_config->cmsh;
	area_id = vl_config->area_id;

	if (area_id.s_addr == OSPF_AREA_BACKBONE) {
		cmdPrint(cmsh, "Configuring VLs over the backbone is not allowed\n");
		return NULL;
	}
	area = ospf_area_get(ospf, area_id, vl_config->format);

	if (area->external_routing != OSPF_AREA_DEFAULT) {
		if (vl_config->format == OSPF_AREA_ID_FORMAT_ADDRESS)
		{
			cmdPrint (cmsh, "Area %s is %s\n",
					inet_ntoa (area_id),
					area->external_routing == OSPF_AREA_NSSA?"nssa":"stub");
		}
		else
		{
			cmdPrint(cmsh, "Area %ld is %s\n", (u_long )ntohl(area_id.s_addr),
					area->external_routing == OSPF_AREA_NSSA?"nssa":"stub");
		}
		return NULL;
	}

	if ((vl_data = ospf_vl_lookup(ospf, area, vl_config->vl_peer)) == NULL) {
		vl_data = ospf_vl_data_new(area, vl_config->vl_peer);
		if (vl_data->vl_oi == NULL) {
			vl_data->vl_oi = ospf_vl_new(ospf, vl_data);
			ospf_vl_add(ospf, vl_data);
			ospf_spf_calculate_schedule(ospf);
		}
	}
	return vl_data;
}


static int
ospf_vl_set_security (struct ospf_vl_data *vl_data,
		      struct ospf_vl_config_data *vl_config)
{
  struct crypt_key *ck;
  struct cmsh *cmsh;
  struct interface *ifp = vl_data->vl_oi->ifp;

  cmsh = vl_config->cmsh;

  if (vl_config->auth_type != OSPF_AUTH_CMD_NOTSEEN)
    {
      SET_IF_PARAM (IF_DEF_PARAMS (ifp), auth_type);
      IF_DEF_PARAMS (ifp)->auth_type = vl_config->auth_type;
    }

  if (vl_config->auth_key)
    {
      memset(IF_DEF_PARAMS (ifp)->auth_simple, 0, OSPF_AUTH_SIMPLE_SIZE+1);
      strncpy ((char *) IF_DEF_PARAMS (ifp)->auth_simple, vl_config->auth_key,
	       OSPF_AUTH_SIMPLE_SIZE);
    }
  else if (vl_config->md5_key)
    {
      if (ospf_crypt_key_lookup (IF_DEF_PARAMS (ifp)->auth_crypt, vl_config->crypto_key_id)
	  != NULL)
	{
	  cmdPrint (cmsh, "OSPF: Key %d already exists",
		   vl_config->crypto_key_id);
	  return (CMD_IPC_WARNING);
	}
      ck = ospf_crypt_key_new ();
      ck->key_id = vl_config->crypto_key_id;
      memset(ck->auth_key, 0, OSPF_AUTH_MD5_SIZE+1);
      strncpy ((char *) ck->auth_key, vl_config->md5_key, OSPF_AUTH_MD5_SIZE);

      ospf_crypt_key_add (IF_DEF_PARAMS (ifp)->auth_crypt, ck);
    }
  else if (vl_config->crypto_key_id != 0)
    {
      /* Delete a key */

      if (ospf_crypt_key_lookup (IF_DEF_PARAMS (ifp)->auth_crypt,
				 vl_config->crypto_key_id) == NULL)
	{
	  cmdPrint (cmsh, "OSPF: Key %d does not exist",
		   vl_config->crypto_key_id);
	  return (CMD_IPC_WARNING);
	}

      ospf_crypt_key_delete (IF_DEF_PARAMS (ifp)->auth_crypt, vl_config->crypto_key_id);

    }

  return (CMD_IPC_OK);
}

static int
ospf_vl_set_timers (struct ospf_vl_data *vl_data,
		    struct ospf_vl_config_data *vl_config)
{
  struct interface *ifp = ifp = vl_data->vl_oi->ifp;
  /* Virtual Link data initialised to defaults, so only set
     if a value given */
  if (vl_config->hello_interval)
    {
      SET_IF_PARAM (IF_DEF_PARAMS (ifp), v_hello);
      IF_DEF_PARAMS (ifp)->v_hello = vl_config->hello_interval;
    }

  if (vl_config->dead_interval)
    {
      SET_IF_PARAM (IF_DEF_PARAMS (ifp), v_wait);
      IF_DEF_PARAMS (ifp)->v_wait = vl_config->dead_interval;
    }

  if (vl_config->retransmit_interval)
    {
      SET_IF_PARAM (IF_DEF_PARAMS (ifp), retransmit_interval);
      IF_DEF_PARAMS (ifp)->retransmit_interval = vl_config->retransmit_interval;
    }

  if (vl_config->transmit_delay)
    {
      SET_IF_PARAM (IF_DEF_PARAMS (ifp), transmit_delay);
      IF_DEF_PARAMS (ifp)->transmit_delay = vl_config->transmit_delay;
    }

  return (CMD_IPC_OK);
}



/* The business end of all of the above */
int
ospf_vl_set (struct ospf *ospf, struct ospf_vl_config_data *vl_config)
{
  struct ospf_vl_data *vl_data;
  int ret;

  vl_data = ospf_find_vl_data (ospf, vl_config);
  if (!vl_data)
    return (CMD_IPC_WARNING);

  /* Process this one first as it can have a fatal result, which can
     only logically occur if the virtual link exists already
     Thus a command error does not result in a change to the
     running configuration such as unexpectedly altered timer
     values etc.*/
  ret = ospf_vl_set_security (vl_data, vl_config);
  if (ret != (CMD_IPC_OK))
    return ret;

  /* Set any time based parameters, these area already range checked */

  ret = ospf_vl_set_timers (vl_data, vl_config);
  if (ret != (CMD_IPC_OK))
    return ret;

  return (CMD_IPC_OK);

}


int
ospf_timers_spf_set (struct cmsh *cmsh, unsigned int delay,
                     unsigned int hold,
                     unsigned int max)
{
  struct ospf *ospf = om->index;

  ospf->spf_delay = delay;
  ospf->spf_holdtime = hold;
  ospf->spf_max_holdtime = max;

  return (CMD_IPC_OK);
}


const char *ospf_abr_type_descr_str[] =
{
  "Unknown",
  "Standard (RFC2328)",
  "Alternative IBM",
  "Alternative Cisco",
  "Alternative Shortcut"
};

const char *ospf_shortcut_mode_descr_str[] =
{
  "Default",
  "Enabled",
  "Disabled"
};



void
show_ip_ospf_area (struct cmsh *cmsh, struct ospf_area *area)
{
	int idx = 0;
	char showBuff[1024] = {};

	/* Show Area ID. */
	idx += sprintf(showBuff + idx, " Area ID: %s", inet_ntoa(area->area_id));

	/* Show Area type/mode. */
	if (OSPF_IS_AREA_BACKBONE(area))
		idx += sprintf(showBuff + idx, " (Backbone)");
	else {
		if (area->external_routing == OSPF_AREA_STUB)
			idx += sprintf(showBuff + idx, " (Stub%s%s)", area->no_summary ? ", no summary" : "",
					area->shortcut_configured ? "; " : "");

		else if (area->external_routing == OSPF_AREA_NSSA)
			idx += sprintf(showBuff + idx, " (NSSA%s%s)", area->no_summary ? ", no summary" : "",
					area->shortcut_configured ? "; " : "");

		cmdPrint(cmsh, "%s", showBuff);
		idx = 0; memset(showBuff, 0, 1024);

		idx += sprintf(showBuff + idx, "   Shortcutting mode: %s",
				ospf_shortcut_mode_descr_str[area->shortcut_configured]);
		idx += sprintf(showBuff + idx, ", S-bit consensus: %s",
				area->shortcut_capability ? "ok" : "no");
	}

	cmdPrint(cmsh, "%s", showBuff);
	idx = 0; memset(showBuff, 0, 1024);

	/* Show number of interfaces. */
	cmdPrint(cmsh, "   Number of interfaces in this area: Total: %d, "
			"Active: %d", listcount(area->oiflist), area->act_ints);

	if (area->external_routing == OSPF_AREA_NSSA) {
		cmdPrint(cmsh, "   It is an NSSA configuration.");
		cmdPrint(cmsh, "Elected NSSA/ABR performs type-7/type-5 LSA translation. ");
		if (!IS_OSPF_ABR(area->ospf))
		{
			cmdPrint(cmsh, "   It is not ABR, therefore not Translator. ");
		}
		else if (area->NSSATranslatorState) {
			idx += sprintf(showBuff + idx, "   We are an ABR and ");
			if (area->NSSATranslatorRole == OSPF_NSSA_ROLE_CANDIDATE)
			{
				cmdPrint(cmsh, "%sthe NSSA Elected Translator.", showBuff);
			}
			else if (area->NSSATranslatorRole == OSPF_NSSA_ROLE_ALWAYS)
			{
				cmdPrint(cmsh, "%salways an NSSA Translator. ", showBuff);
			}
			idx = 0; memset(showBuff, 0, 1024);
		} else {
			idx += sprintf(showBuff + idx, "   We are an ABR, but ");
			if (area->NSSATranslatorRole == OSPF_NSSA_ROLE_CANDIDATE)
			{
				cmdPrint(cmsh, "%snot the NSSA Elected Translator. ", showBuff);
			}
			else
			{
				cmdPrint(cmsh, "%snever an NSSA Translator. ", showBuff);
			}
			idx = 0; memset(showBuff, 0, 1024);
		}
	}
	/* Stub-router state for this area */
	if (CHECK_FLAG(area->stub_router_state, OSPF_AREA_IS_STUB_ROUTED)) {
		char timebuf[OSPF_TIME_DUMP_SIZE];
		cmdPrint(cmsh, "   Originating stub / maximum-distance Router-LSA");
		if (CHECK_FLAG(area->stub_router_state, OSPF_AREA_ADMIN_STUB_ROUTED))
			cmdPrint(cmsh, "     Administratively activated (indefinitely)");
		if (area->pTimerStubRouter)
			cmdPrint(cmsh, "     Active from startup, %s remaining",
					ospf_timer_dump(area->pTimerStubRouter, area->tvUpdateStubRouter,
							timebuf, sizeof(timebuf)));
	}

	/* Show number of fully adjacent neighbors. */
	cmdPrint(cmsh, "   Number of fully adjacent neighbors in this area:"
			" %d", area->full_nbrs);

	/* Show authentication type. */
	idx += sprintf(showBuff + idx, "   Area has ");
	if (area->auth_type == OSPF_AUTH_NULL)
	{
		cmdPrint(cmsh, "%sno authentication", showBuff);
	}
	else if (area->auth_type == OSPF_AUTH_SIMPLE)
	{
		cmdPrint(cmsh, "%ssimple password authentication", showBuff);
	}
	else if (area->auth_type == OSPF_AUTH_CRYPTOGRAPHIC)
	{
		cmdPrint(cmsh, "%smessage digest authentication", showBuff);
	}
	idx = 0; memset(showBuff, 0, 1024);

	if (!OSPF_IS_AREA_BACKBONE(area))
		cmdPrint(cmsh, "   Number of full virtual adjacencies going through"
				" this area: %d", area->full_vls);

	/* Show SPF calculation times. */
	cmdPrint(cmsh, "   SPF algorithm executed %d times", area->spf_calculation);

	/* Show number of LSA. */
	cmdPrint(cmsh, "   Number of LSA %ld", area->lsdb->total);
	cmdPrint(cmsh, "   Number of router LSA %ld. Checksum Sum 0x%08x",
			ospf_lsdb_count(area->lsdb, OSPF_ROUTER_LSA),
			ospf_lsdb_checksum(area->lsdb, OSPF_ROUTER_LSA));
	cmdPrint(cmsh, "   Number of network LSA %ld. Checksum Sum 0x%08x",
			ospf_lsdb_count(area->lsdb, OSPF_NETWORK_LSA),
			ospf_lsdb_checksum(area->lsdb, OSPF_NETWORK_LSA));
	cmdPrint(cmsh, "   Number of summary LSA %ld. Checksum Sum 0x%08x",
			ospf_lsdb_count(area->lsdb, OSPF_SUMMARY_LSA),
			ospf_lsdb_checksum(area->lsdb, OSPF_SUMMARY_LSA));
	cmdPrint(cmsh, "   Number of ASBR summary LSA %ld. Checksum Sum 0x%08x",
			ospf_lsdb_count(area->lsdb, OSPF_ASBR_SUMMARY_LSA),
			ospf_lsdb_checksum(area->lsdb, OSPF_ASBR_SUMMARY_LSA));
	cmdPrint(cmsh, "   Number of NSSA LSA %ld. Checksum Sum 0x%08x",
			ospf_lsdb_count(area->lsdb, OSPF_AS_NSSA_LSA),
			ospf_lsdb_checksum(area->lsdb, OSPF_AS_NSSA_LSA));
#ifdef HAVE_OPAQUE_LSA
	cmdPrint (cmsh, "   Number of opaque link LSA %ld. Checksum Sum 0x%08x",
			ospf_lsdb_count (area->lsdb, OSPF_OPAQUE_LINK_LSA),
			ospf_lsdb_checksum (area->lsdb, OSPF_OPAQUE_LINK_LSA));
	cmdPrint (cmsh, "   Number of opaque area LSA %ld. Checksum Sum 0x%08x",
			ospf_lsdb_count (area->lsdb, OSPF_OPAQUE_AREA_LSA),
			ospf_lsdb_checksum (area->lsdb, OSPF_OPAQUE_AREA_LSA));
#endif /* HAVE_OPAQUE_LSA */
	cmdPrint(cmsh, " ");
}


static const char *ospf_network_type_str[] =
{
  "Null",
  "POINTOPOINT",
  "BROADCAST",
  "NBMA",
  "POINTOMULTIPOINT",
  "VIRTUALLINK",
  "LOOPBACK"
};

void
show_ip_ospf_interface_sub (struct cmsh *cmsh, struct ospf *ospf,
			    struct interface *ifp)
{
	int is_up;
	struct ospf_neighbor *nbr;
	struct route_node *rn;
	int idx = 0;
	char showBuff[1024] = { };

	/* Is interface up? */
	cmdPrint(cmsh, "%s is %s", ifp->name,
			((is_up = if_is_operative(ifp)) ? "up" : "down"));
	cmdPrint(cmsh, "  ifindex %u, MTU %u bytes, BW %u Kbit %s", ifp->ifindex,
			ifp->mtu, ifp->bandwidth, if_flag_dump(ifp->flags));

	/* Is interface OSPF enabled? */
	if (ospf_oi_count(ifp) == 0) {
		cmdPrint(cmsh, "  OSPF not enabled on this interface");
		return;
	} else if (!is_up) {
		cmdPrint(cmsh, "  OSPF is enabled, but not running on this interface");
		return;
	}

	for (rn = route_top(IF_OIFS(ifp)); rn; rn = route_next(rn)) {
		struct ospf_interface *oi = rn->info;

		if (oi == NULL)
			continue;

		/* Show OSPF interface information. */
		idx += sprintf(showBuff + idx, "  Internet Address %s/%d,",
				inet_ntoa(oi->address->u.prefix4), oi->address->prefixlen);

		if (oi->connected->destination || oi->type == OSPF_IFTYPE_VIRTUALLINK) {
			struct in_addr *dest;
			const char *dstr;

			if (CONNECTED_PEER(
					oi->connected) || oi->type == OSPF_IFTYPE_VIRTUALLINK)
				dstr = "Peer";
			else
				dstr = "Broadcast";

			/* For Vlinks, showing the peer address is probably more
			 * informative than the local interface that is being used
			 */
			if (oi->type == OSPF_IFTYPE_VIRTUALLINK)
				dest = &oi->vl_data->peer_addr;
			else
				dest = &oi->connected->destination->u.prefix4;

			idx += sprintf(showBuff + idx, " %s %s,", dstr, inet_ntoa(*dest));
		}

		cmdPrint(cmsh, " %sArea %s", showBuff, ospf_area_desc_string(oi->area));
		idx = 0;
		memset(showBuff, 0, 1024);

		cmdPrint(cmsh, "  MTU mismatch detection:%s",
				OSPF_IF_PARAM(oi, mtu_ignore) ? "disabled" : "enabled");

		cmdPrint(cmsh, "  Router ID %s, Network Type %s, Cost: %d",
				inet_ntoa(ospf->router_id), ospf_network_type_str[oi->type],
				oi->output_cost);

		cmdPrint(cmsh, "  Transmit Delay is %d sec, State %s, Priority %d",
				OSPF_IF_PARAM (oi,transmit_delay),
				LOOKUP (ospf_ism_state_msg, oi->state), PRIORITY (oi));

		/* Show DR information. */
		if (DR (oi).s_addr == 0) {
			cmdPrint(cmsh, "  No designated router on this network");
		} else {
			nbr = ospf_nbr_lookup_by_addr(oi->nbrs, &DR(oi));
			if (nbr == NULL) {
				cmdPrint(cmsh, "  No designated router on this network");
			} else {
				cmdPrint(cmsh,
						"  Designated Router (ID) %s, Interface Address %s",
						inet_ntoa(nbr->router_id),
						inet_ntoa(nbr->address.u.prefix4));
			}
		}

		/* Show BDR information. */
		if (BDR (oi).s_addr == 0)
		{
			cmdPrint (cmsh, "  No backup designated router on this network");
		}
		else {
			nbr = ospf_nbr_lookup_by_addr(oi->nbrs, &BDR(oi));
			if (nbr == NULL)
			{
				cmdPrint (cmsh, "  No backup designated router on this network");
			}
			else
			{
				cmdPrint(cmsh,
						"  Backup Designated Router (ID) %s, Interface Address %s",
						inet_ntoa(nbr->router_id),
						inet_ntoa(nbr->address.u.prefix4));
			}
		}

		/* Next network-LSA sequence number we'll use, if we're elected DR */
		if (oi->params && ntohl(oi->params->network_lsa_seqnum)
						!= OSPF_INITIAL_SEQUENCE_NUMBER)
			cmdPrint(cmsh, "  Saved Network-LSA sequence number 0x%x",
					ntohl(oi->params->network_lsa_seqnum));

		idx += sprintf(showBuff + idx, "  Multicast group memberships:");
		if (OI_MEMBER_CHECK(oi,
				MEMBER_ALLROUTERS) || OI_MEMBER_CHECK(oi, MEMBER_DROUTERS)) {
			if (OI_MEMBER_CHECK(oi, MEMBER_ALLROUTERS))
				idx += sprintf(showBuff + idx, " OSPFAllRouters");
			if (OI_MEMBER_CHECK(oi, MEMBER_DROUTERS))
				idx += sprintf(showBuff + idx, " OSPFDesignatedRouters");
		} else
			idx += sprintf(showBuff + idx, " <None>");
		cmdPrint(cmsh, "%s", showBuff);
		idx = 0;
		memset(showBuff, 0, 1024);

		idx += sprintf(showBuff + idx, "  Timer intervals configured,");
		idx += sprintf(showBuff + idx, " Hello ");
		if (OSPF_IF_PARAM (oi, fast_hello) == 0)
			idx += sprintf(showBuff + idx, "%ds,", OSPF_IF_PARAM(oi, v_hello));
		else
			idx += sprintf(showBuff + idx, "%dms,",
					1000 / OSPF_IF_PARAM(oi, fast_hello));
		cmdPrint(cmsh, "%s Dead %ds, Wait %ds, Retransmit %d", showBuff,
				OSPF_IF_PARAM (oi, v_wait), OSPF_IF_PARAM (oi, v_wait),
				OSPF_IF_PARAM (oi, retransmit_interval));
		idx = 0;
		memset(showBuff, 0, 1024);

		if (OSPF_IF_PASSIVE_STATUS (oi) == OSPF_IF_ACTIVE) {
			char timebuf[OSPF_TIME_DUMP_SIZE];
			cmdPrint(cmsh, "    Hello due in %s",
					ospf_timer_dump(oi->pTimerHello, oi->tvUpdateHello, timebuf, sizeof(timebuf)));
		} else /* passive-interface is set */
		cmdPrint(cmsh, "    No Hellos (Passive interface)");

		cmdPrint(cmsh, "  Neighbor Count is %d, Adjacent neighbor count is %d",
				ospf_nbr_count(oi, 0), ospf_nbr_count (oi, NSM_Full));
	}
}


void
show_ip_ospf_neighbour_header (struct cmsh *cmsh)
{
  cmdPrint (cmsh, "\n%15s %3s %-15s %9s %-15s %-20s %5s %5s %5s",
           "Neighbor ID", "Pri", "State", "Dead Time",
           "Address", "Interface", "RXmtL", "RqstL", "DBsmL");
}

void
show_ip_ospf_neighbor_sub (struct cmsh *cmsh, struct ospf_interface *oi)
{
	struct route_node *rn;
	struct ospf_neighbor *nbr;
	char msgbuf[16];
	char timebuf[OSPF_TIME_DUMP_SIZE];
	int idx = 0;
	char showBuff[1024] = { };

	for (rn = route_top(oi->nbrs); rn; rn = route_next(rn))
		if ((nbr = rn->info))
			/* Do not show myself. */
			if (nbr != oi->nbr_self)
				/* Down state is not shown. */
				if (nbr->state != NSM_Down) {
					ospf_nbr_state_message(nbr, msgbuf, 16);

					if (nbr->state == NSM_Attempt && nbr->router_id.s_addr == 0)
						idx += sprintf(showBuff + idx, "%-15s %3d %-15s ", "-",
								nbr->priority, msgbuf);
					else
						idx += sprintf(showBuff + idx, "%-15s %3d %-15s ",
								inet_ntoa(nbr->router_id), nbr->priority,
								msgbuf);

					idx += sprintf(showBuff + idx, "%9s ",
							ospf_timer_dump(nbr->pTimerInactivity, nbr->tvUpdateInactivity,
									timebuf, sizeof(timebuf)));

					idx += sprintf(showBuff + idx, "%-15s ", inet_ntoa(nbr->src));
					cmdPrint(cmsh, "%s%-20s %5ld %5ld %5d%s", 
							showBuff,
							IF_NAME(oi),
							ospf_ls_retransmit_count(nbr),
							ospf_ls_request_count(nbr),
							ospf_db_summary_count(nbr));
					idx = 0; memset(showBuff, 0, 1024);
				}
}


void
show_ip_ospf_nbr_nbma_detail_sub (struct cmsh *cmsh, struct ospf_interface *oi,
				  struct ospf_nbr_nbma *nbr_nbma)
{
  char timebuf[OSPF_TIME_DUMP_SIZE];

  /* Show neighbor ID. */  /* Show interface address. */
  cmdPrint (cmsh, " Neighbor -, interface address %s", inet_ntoa (nbr_nbma->addr));
  /* Show Area ID. */
  cmdPrint (cmsh, "    In the area %s via interface %s",
	   ospf_area_desc_string (oi->area), IF_NAME (oi));
  /* Show neighbor priority and state. */  /* Show state changes. */
  cmdPrint (cmsh, "    Neighbor priority is %d, State is %s, %d state changes%s",
		  nbr_nbma->priority, "Down", nbr_nbma->state_change);

  /* Show PollInterval */
  cmdPrint (cmsh, "    Poll interval %d", nbr_nbma->v_poll);

  /* Show poll-interval timer. */
  cmdPrint (cmsh, "    Poll timer due in %s",
	   ospf_timer_dump (nbr_nbma->pTimerPoll, nbr_nbma->tvUpdatePoll, timebuf, sizeof(timebuf)));

  /* Show poll-interval timer thread. */
  cmdPrint (cmsh, "    Thread Poll Timer %s",
	   nbr_nbma->pTimerPoll != NULL ? "on" : "off");
}

void
show_ip_ospf_neighbor_detail_sub (struct cmsh *cmsh, struct ospf_interface *oi,
				  struct ospf_neighbor *nbr)
{
  char timebuf[OSPF_TIME_DUMP_SIZE];
  int idx = 0;
  char showBuff[1024] = {};

  /* Show neighbor ID. */
  if (nbr->state == NSM_Attempt && nbr->router_id.s_addr == 0)
	  idx += sprintf(showBuff + idx, " Neighbor %s,", "-");
  else
	  idx += sprintf(showBuff + idx, " Neighbor %s,", inet_ntoa (nbr->router_id));

  /* Show interface address. */
  cmdPrint (cmsh, "%s interface address %s", showBuff, inet_ntoa (nbr->address.u.prefix4));
  idx = 0; memset(showBuff, 0, 1024);
  /* Show Area ID. */
  cmdPrint (cmsh, "    In the area %s via interface %s",
	   ospf_area_desc_string (oi->area), oi->ifp->name);
  /* Show neighbor priority and state. Show state changes.*/
  cmdPrint (cmsh, "    Neighbor priority is %d, State is %s, %d state changes%s",
	   nbr->priority, LOOKUP (ospf_nsm_state_msg, nbr->state), nbr->state_change);
  if (nbr->ts_last_progress.tv_sec || nbr->ts_last_progress.tv_usec)
    {
      struct timeval res
        = tv_sub (ospfRecentRelativeTime(), nbr->ts_last_progress);
      cmdPrint (cmsh, "    Most recent state change statistics:");
      cmdPrint (cmsh, "      Progressive change %s ago",
               ospf_timeval_dump (&res, timebuf, sizeof(timebuf)));
    }
  if (nbr->ts_last_regress.tv_sec || nbr->ts_last_regress.tv_usec)
    {
      struct timeval res
        = tv_sub (ospfRecentRelativeTime(), nbr->ts_last_regress);
      cmdPrint (cmsh, "      Regressive change %s ago, due to %s",
               ospf_timeval_dump (&res, timebuf, sizeof(timebuf)),
               (nbr->last_regress_str ? nbr->last_regress_str : "??"));
    }
  /* Show Designated Rotuer ID. Show Backup Designated Rotuer ID. */
  cmdPrint (cmsh, "    DR is %s, BDR is %s%s",
		  inet_ntoa (nbr->d_router), inet_ntoa (nbr->bd_router));
  /* Show options. */
  cmdPrint (cmsh, "    Options %d %s", nbr->options,
	   ospf_options_dump (nbr->options));
  /* Show Router Dead interval timer. */
  cmdPrint (cmsh, "    Dead timer due in %s",
	   ospf_timer_dump (nbr->pTimerInactivity, nbr->tvUpdateInactivity, timebuf, sizeof (timebuf)));
  /* Show Database Summary list. */
  cmdPrint (cmsh, "    Database Summary List %d",
	   ospf_db_summary_count (nbr));
  /* Show Link State Request list. */
  cmdPrint (cmsh, "    Link State Request List %ld",
	   ospf_ls_request_count (nbr));
  /* Show Link State Retransmission list. */
  cmdPrint (cmsh, "    Link State Retransmission List %ld",
	   ospf_ls_retransmit_count (nbr));
  /* Show inactivity timer thread. */
  cmdPrint (cmsh, "    Thread Inactivity Timer %s",
	   nbr->pTimerInactivity != NULL ? "on" : "off");
  /* Show Database Description retransmission thread. */
  cmdPrint (cmsh, "    Thread Database Description Retransmision %s",
	   nbr->pTimerDbDesc != NULL ? "on" : "off");
  /* Show Link State Request Retransmission thread. */
  cmdPrint (cmsh, "    Thread Link State Request Retransmission %s",
	   nbr->pTimerLsReq != NULL ? "on" : "off");
  /* Show Link State Update Retransmission thread. */
  cmdPrint (cmsh, "    Thread Link State Update Retransmission %s",
	   nbr->pTimerLsUpd != NULL ? "on" : "off");
}

/* Show functions */
static int
show_lsa_summary (struct cmsh *cmsh, struct ospf_lsa *lsa, int self)
{
	struct router_lsa *rl;
	struct summary_lsa *sl;
	struct as_external_lsa *asel;
	struct prefix_ipv4 p;
	int idx = 0;
	char showBuff[1024] = { };

	if (lsa != NULL)
		/* If self option is set, check LSA self flag. */
		if (self == 0 || IS_LSA_SELF(lsa)) {
			/* LSA common part show. */
			idx += sprintf(showBuff + idx, "%-15s ", inet_ntoa(lsa->data->id));
			idx += sprintf(showBuff + idx, "%-15s %4d 0x%08lx 0x%04x",
					inet_ntoa(lsa->data->adv_router), LS_AGE(lsa),
					(u_long) ntohl(lsa->data->ls_seqnum),
					ntohs(lsa->data->checksum));
			/* LSA specific part show. */
			switch (lsa->data->type) {
			case OSPF_ROUTER_LSA:
				rl = (struct router_lsa *) lsa->data;
				idx += sprintf(showBuff + idx, " %-d", ntohs(rl->links));
				break;
			case OSPF_SUMMARY_LSA:
				sl = (struct summary_lsa *) lsa->data;

				p.family = AF_INET;
				p.prefix = sl->header.id;
				p.prefixlen = ip_masklen(sl->mask);
				apply_mask_ipv4(&p);

				idx += sprintf(showBuff + idx, " %s/%d", inet_ntoa(p.prefix), p.prefixlen);
				break;
			case OSPF_AS_EXTERNAL_LSA:
			case OSPF_AS_NSSA_LSA:
				asel = (struct as_external_lsa *) lsa->data;

				p.family = AF_INET;
				p.prefix = asel->header.id;
				p.prefixlen = ip_masklen(asel->mask);
				apply_mask_ipv4(&p);

				idx += sprintf(showBuff + idx, " %s %s/%d [0x%lx]",
						IS_EXTERNAL_METRIC (asel->e[0].tos) ? "E2" : "E1",
								inet_ntoa(p.prefix), p.prefixlen,
								(u_long) ntohl(asel->e[0].route_tag));
				break;
			case OSPF_NETWORK_LSA:
			case OSPF_ASBR_SUMMARY_LSA:
#ifdef HAVE_OPAQUE_LSA
			case OSPF_OPAQUE_LINK_LSA:
			case OSPF_OPAQUE_AREA_LSA:
			case OSPF_OPAQUE_AS_LSA:
#endif /* HAVE_OPAQUE_LSA */
			default:
				break;
			}
			cmdPrint(cmsh, "%s", showBuff);
			idx = 0; memset(showBuff, 0, 1024);
		}

	return 0;
}

static const char *show_database_desc[] =
{
  "unknown",
  "Router Link States",
  "Net Link States",
  "Summary Link States",
  "ASBR-Summary Link States",
  "AS External Link States",
  "Group Membership LSA",
  "NSSA-external Link States",
#ifdef HAVE_OPAQUE_LSA
  "Type-8 LSA",
  "Link-Local Opaque-LSA",
  "Area-Local Opaque-LSA",
  "AS-external Opaque-LSA",
#endif /* HAVE_OPAQUE_LSA */
};

static const char *show_database_header[] =
{
  "",
  "Link ID         ADV Router      Age  Seq#       CkSum  Link count",
  "Link ID         ADV Router      Age  Seq#       CkSum",
  "Link ID         ADV Router      Age  Seq#       CkSum  Route",
  "Link ID         ADV Router      Age  Seq#       CkSum",
  "Link ID         ADV Router      Age  Seq#       CkSum  Route",
  " --- header for Group Member ----",
  "Link ID         ADV Router      Age  Seq#       CkSum  Route",
#ifdef HAVE_OPAQUE_LSA
  " --- type-8 ---",
  "Opaque-Type/Id  ADV Router      Age  Seq#       CkSum",
  "Opaque-Type/Id  ADV Router      Age  Seq#       CkSum",
  "Opaque-Type/Id  ADV Router      Age  Seq#       CkSum",
#endif /* HAVE_OPAQUE_LSA */
};

static void
show_ip_ospf_database_header (struct cmsh *cmsh, struct ospf_lsa *lsa)
{
  struct router_lsa *rlsa = (struct router_lsa*) lsa->data;
  int idx = 0;
  char showBuff[1024] = {};

  cmdPrint (cmsh, "  LS age: %d", LS_AGE (lsa));
  cmdPrint (cmsh, "  Options: 0x%-2x : %s",
           lsa->data->options,
           ospf_options_dump(lsa->data->options));
  cmdPrint (cmsh, "  LS Flags: 0x%-2x %s",
           lsa->flags,
           ((lsa->flags & OSPF_LSA_LOCAL_XLT) ? "(Translated from Type-7)" : ""));

  if (lsa->data->type == OSPF_ROUTER_LSA)
    {
	  idx += sprintf(showBuff + idx, "  Flags: 0x%x" , rlsa->flags);

      if (rlsa->flags)
    	  idx += sprintf(showBuff + idx, " :%s%s%s%s",
    				 IS_ROUTER_LSA_BORDER (rlsa) ? " ABR" : "",
    				 IS_ROUTER_LSA_EXTERNAL (rlsa) ? " ASBR" : "",
    				 IS_ROUTER_LSA_VIRTUAL (rlsa) ? " VL-endpoint" : "",
    				 IS_ROUTER_LSA_SHORTCUT (rlsa) ? " Shortcut" : "");

      cmdPrint (cmsh, "%s", showBuff);
      idx = 0; memset(showBuff, 0, 1024);
    }
  cmdPrint (cmsh, "  LS Type: %s",
           LOOKUP (ospf_lsa_type_msg, lsa->data->type));
  cmdPrint (cmsh, "  Link State ID: %s %s", inet_ntoa (lsa->data->id),
           LOOKUP (ospf_link_state_id_type_msg, lsa->data->type));
  cmdPrint (cmsh, "  Advertising Router: %s",
           inet_ntoa (lsa->data->adv_router));
  cmdPrint (cmsh, "  LS Seq Number: %08lx", (u_long)ntohl (lsa->data->ls_seqnum));
  cmdPrint (cmsh, "  Checksum: 0x%04x", ntohs (lsa->data->checksum));
  cmdPrint (cmsh, "  Length: %d", ntohs (lsa->data->length));
}

const char *link_type_desc[] =
{
  "(null)",
  "another Router (point-to-point)",
  "a Transit Network",
  "Stub Network",
  "a Virtual Link",
};

const char *link_id_desc[] =
{
  "(null)",
  "Neighboring Router ID",
  "Designated Router address",
  "Net",
  "Neighboring Router ID",
};

const char *link_data_desc[] =
{
  "(null)",
  "Router Interface address",
  "Router Interface address",
  "Network Mask",
  "Router Interface address",
};

/* Show router-LSA each Link information. */
static void
show_ip_ospf_database_router_links (struct cmsh *cmsh,
                                    struct router_lsa *rl)
{
  int len, i, type;

  len = ntohs (rl->header.length) - 4;
  for (i = 0; i < ntohs (rl->links) && len > 0; len -= 12, i++)
    {
      type = rl->link[i].type;

      cmdPrint (cmsh, "    Link connected to: %s",
	       link_type_desc[type]);
      cmdPrint (cmsh, "     (Link ID) %s: %s", link_id_desc[type],
	       inet_ntoa (rl->link[i].link_id));
      cmdPrint (cmsh, "     (Link Data) %s: %s", link_data_desc[type],
	       inet_ntoa (rl->link[i].link_data));
      cmdPrint (cmsh, "      Number of TOS metrics: 0");
      cmdPrint (cmsh, "       TOS 0 Metric: %d",
	       ntohs (rl->link[i].metric));
      cmdPrint (cmsh, "\n");
    }
}

/* Show router-LSA detail information. */
static int
show_router_lsa_detail (struct cmsh *cmsh, struct ospf_lsa *lsa)
{
  if (lsa != NULL)
    {
      struct router_lsa *rl = (struct router_lsa *) lsa->data;

      show_ip_ospf_database_header (cmsh, lsa);

      cmdPrint (cmsh, "   Number of Links: %d\n", ntohs (rl->links));

      show_ip_ospf_database_router_links (cmsh, rl);
      cmdPrint (cmsh, "\n");
    }

  return 0;
}

/* Show network-LSA detail information. */
static int
show_network_lsa_detail (struct cmsh *cmsh, struct ospf_lsa *lsa)
{
  int length, i;

  if (lsa != NULL)
    {
      struct network_lsa *nl = (struct network_lsa *) lsa->data;

      show_ip_ospf_database_header (cmsh, lsa);

      cmdPrint (cmsh, "  Network Mask: /%d", ip_masklen (nl->mask));

      length = ntohs (lsa->data->length) - OSPF_LSA_HEADER_SIZE - 4;

      for (i = 0; length > 0; i++, length -= 4)
	cmdPrint (cmsh, "        Attached Router: %s", inet_ntoa (nl->routers[i]));

      cmdPrint (cmsh, "\n");
    }

  return 0;
}

/* Show summary-LSA detail information. */
static int
show_summary_lsa_detail (struct cmsh *cmsh, struct ospf_lsa *lsa)
{
  if (lsa != NULL)
    {
      struct summary_lsa *sl = (struct summary_lsa *) lsa->data;

      show_ip_ospf_database_header (cmsh, lsa);

      cmdPrint (cmsh, "  Network Mask: /%d", ip_masklen (sl->mask));
      cmdPrint (cmsh, "        TOS: 0  Metric: %d", GET_METRIC (sl->metric));
	  cmdPrint (cmsh, "\n");
    }

  return 0;
}

/* Show summary-ASBR-LSA detail information. */
static int
show_summary_asbr_lsa_detail (struct cmsh *cmsh, struct ospf_lsa *lsa)
{
  if (lsa != NULL)
    {
      struct summary_lsa *sl = (struct summary_lsa *) lsa->data;

      show_ip_ospf_database_header (cmsh, lsa);

      cmdPrint (cmsh, "  Network Mask: /%d", ip_masklen (sl->mask));
      cmdPrint (cmsh, "        TOS: 0  Metric: %d", GET_METRIC (sl->metric));
	  cmdPrint (cmsh, "\n");
    }

  return 0;
}

/* Show AS-external-LSA detail information. */
static int
show_as_external_lsa_detail (struct cmsh *cmsh, struct ospf_lsa *lsa)
{
  if (lsa != NULL)
    {
      struct as_external_lsa *al = (struct as_external_lsa *) lsa->data;

      show_ip_ospf_database_header (cmsh, lsa);

      cmdPrint (cmsh, "  Network Mask: /%d", ip_masklen (al->mask));
      cmdPrint (cmsh, "        Metric Type: %s",
    		  IS_EXTERNAL_METRIC (al->e[0].tos) ? "2 (Larger than any link state path)" : "1");
      cmdPrint (cmsh, "        TOS: 0\n");
      cmdPrint (cmsh, "        Metric: %d", GET_METRIC (al->e[0].metric));
      cmdPrint (cmsh, "        Forward Address: %s", inet_ntoa (al->e[0].fwd_addr));

      cmdPrint (cmsh, "        External Route Tag: %lu\n",
	       (u_long)ntohl (al->e[0].route_tag));
    }

  return 0;
}

/* Show AS-NSSA-LSA detail information. */
static int
show_as_nssa_lsa_detail (struct cmsh *cmsh, struct ospf_lsa *lsa)
{
  if (lsa != NULL)
    {
      struct as_external_lsa *al = (struct as_external_lsa *) lsa->data;

      show_ip_ospf_database_header (cmsh, lsa);

      cmdPrint (cmsh, "  Network Mask: /%d", ip_masklen (al->mask));
      cmdPrint (cmsh, "        Metric Type: %s",
	       IS_EXTERNAL_METRIC (al->e[0].tos) ? "2 (Larger than any link state path)" : "1");
      cmdPrint (cmsh, "        TOS: 0");
      cmdPrint (cmsh, "        Metric: %d", GET_METRIC (al->e[0].metric));
      cmdPrint (cmsh, "        NSSA: Forward Address: %s", inet_ntoa (al->e[0].fwd_addr));

      cmdPrint (cmsh, "        External Route Tag: %u\n", ntohl (al->e[0].route_tag));
    }

  return 0;
}

static int
show_func_dummy (struct cmsh *cmsh, struct ospf_lsa *lsa)
{
  return 0;
}

#ifdef HAVE_OPAQUE_LSA
static int
show_opaque_lsa_detail (struct cmsh *cmsh, struct ospf_lsa *lsa)
{
  if (lsa != NULL)
    {
      show_ip_ospf_database_header (cmsh, lsa);
      show_opaque_info_detail (cmsh, lsa);

      cmdPrint (cmsh, "\n");
    }
  return 0;
}
#endif /* HAVE_OPAQUE_LSA */

int (*show_function[])(struct cmsh *, struct ospf_lsa *) =
{
  NULL,
  show_router_lsa_detail,
  show_network_lsa_detail,
  show_summary_lsa_detail,
  show_summary_asbr_lsa_detail,
  show_as_external_lsa_detail,
  show_func_dummy,
  show_as_nssa_lsa_detail,  /* almost same as external */
#ifdef HAVE_OPAQUE_LSA
  NULL,				/* type-8 */
  show_opaque_lsa_detail,
  show_opaque_lsa_detail,
  show_opaque_lsa_detail,
#endif /* HAVE_OPAQUE_LSA */
};

static void
show_lsa_prefix_set (struct cmsh *cmsh, struct prefix_ls *lp, struct in_addr *id,
		     struct in_addr *adv_router)
{
  memset (lp, 0, sizeof (struct prefix_ls));
  lp->family = 0;
  if (id == NULL)
    lp->prefixlen = 0;
  else if (adv_router == NULL)
    {
      lp->prefixlen = 32;
      lp->id = *id;
    }
  else
    {
      lp->prefixlen = 64;
      lp->id = *id;
      lp->adv_router = *adv_router;
    }
}

static void
show_lsa_detail_proc (struct cmsh *cmsh, struct route_table *rt,
		      struct in_addr *id, struct in_addr *adv_router)
{
  struct prefix_ls lp;
  struct route_node *rn, *start;
  struct ospf_lsa *lsa;

  show_lsa_prefix_set (cmsh, &lp, id, adv_router);
  start = route_node_get (rt, (struct prefix *) &lp);
  if (start)
    {
      route_lock_node (start);
      for (rn = start; rn; rn = route_next_until (rn, start))
	if ((lsa = rn->info))
	  {
	    if (show_function[lsa->data->type] != NULL)
	      show_function[lsa->data->type] (cmsh, lsa);
	  }
      route_unlock_node (start);
    }
}

/* Show detail LSA information
   -- if id is NULL then show all LSAs. */
void
show_lsa_detail (struct cmsh *cmsh, struct ospf *ospf, int type,
		 struct in_addr *id, struct in_addr *adv_router)
{
  struct listnode *node;
  struct ospf_area *area;

  switch (type)
    {
    case OSPF_AS_EXTERNAL_LSA:
#ifdef HAVE_OPAQUE_LSA
    case OSPF_OPAQUE_AS_LSA:
#endif /* HAVE_OPAQUE_LSA */
      cmdPrint (cmsh, "                %s \n", show_database_desc[type]);
      show_lsa_detail_proc (cmsh, AS_LSDB (ospf, type), id, adv_router);
      break;
    default:
      for (ALL_LIST_ELEMENTS_RO (ospf->areas, node, area))
        {
          cmdPrint (cmsh, "\n                %s (Area %s)\n",show_database_desc[type],
                   ospf_area_desc_string (area));
          show_lsa_detail_proc (cmsh, AREA_LSDB (area, type), id, adv_router);
        }
      break;
    }
}

static void
show_lsa_detail_adv_router_proc (struct cmsh *cmsh, struct route_table *rt,
				 struct in_addr *adv_router)
{
  struct route_node *rn;
  struct ospf_lsa *lsa;

  for (rn = route_top (rt); rn; rn = route_next (rn))
    if ((lsa = rn->info))
      if (IPV4_ADDR_SAME (adv_router, &lsa->data->adv_router))
	{
	  if (CHECK_FLAG (lsa->flags, OSPF_LSA_LOCAL_XLT))
	    continue;
	  if (show_function[lsa->data->type] != NULL)
	    show_function[lsa->data->type] (cmsh, lsa);
	}
}

/* Show detail LSA information. */
void
show_lsa_detail_adv_router (struct cmsh *cmsh, struct ospf *ospf, int type,
			    struct in_addr *adv_router)
{
  struct listnode *node;
  struct ospf_area *area;

  switch (type)
    {
    case OSPF_AS_EXTERNAL_LSA:
#ifdef HAVE_OPAQUE_LSA
    case OSPF_OPAQUE_AS_LSA:
#endif /* HAVE_OPAQUE_LSA */
      cmdPrint (cmsh, "                %s \n", show_database_desc[type]);
      show_lsa_detail_adv_router_proc (cmsh, AS_LSDB (ospf, type),
                                       adv_router);
      break;
    default:
      for (ALL_LIST_ELEMENTS_RO (ospf->areas, node, area))
        {
          cmdPrint (cmsh, "\n                %s (Area %s)\n", show_database_desc[type],
                   ospf_area_desc_string (area));
          show_lsa_detail_adv_router_proc (cmsh, AREA_LSDB (area, type),
                                           adv_router);
	}
      break;
    }
}

void
show_ip_ospf_database_summary (struct cmsh *cmsh, struct ospf *ospf, int self)
{
  struct ospf_lsa *lsa;
  struct route_node *rn;
  struct ospf_area *area;
  struct listnode *node;
  int type;

  for (ALL_LIST_ELEMENTS_RO (ospf->areas, node, area))
    {
      for (type = OSPF_MIN_LSA; type < OSPF_MAX_LSA; type++)
	{
	  switch (type)
	    {
	    case OSPF_AS_EXTERNAL_LSA:
#ifdef HAVE_OPAQUE_LSA
            case OSPF_OPAQUE_AS_LSA:
#endif /* HAVE_OPAQUE_LSA */
	      continue;
	    default:
	      break;
	    }
          if (ospf_lsdb_count_self (area->lsdb, type) > 0 ||
              (!self && ospf_lsdb_count (area->lsdb, type) > 0))
            {
              cmdPrint (cmsh, "                %s (Area %s)\n",
                       show_database_desc[type],
                       ospf_area_desc_string (area));
        	  	cmdPrint (cmsh, " ");
              cmdPrint (cmsh, "%s", show_database_header[type]);

	      LSDB_LOOP (AREA_LSDB (area, type), rn, lsa)
		show_lsa_summary (cmsh, lsa, self);

              cmdPrint (cmsh, " ");
	  }
	}
    }

  for (type = OSPF_MIN_LSA; type < OSPF_MAX_LSA; type++)
    {
      switch (type)
        {
          case OSPF_AS_EXTERNAL_LSA:
#ifdef HAVE_OPAQUE_LSA
          case OSPF_OPAQUE_AS_LSA:
#endif /* HAVE_OPAQUE_LSA */
            break;
          default:
            continue;
        }
      if (ospf_lsdb_count_self (ospf->lsdb, type) ||
         (!self && ospf_lsdb_count (ospf->lsdb, type)))
        {
          cmdPrint (cmsh, "                %s", show_database_desc[type]);
          cmdPrint (cmsh, " ");
          cmdPrint (cmsh, "%s", show_database_header[type]);

	  LSDB_LOOP (AS_LSDB (ospf, type), rn, lsa)
	    show_lsa_summary (cmsh, lsa, self);

          cmdPrint (cmsh, " ");
        }
    }

  cmdPrint (cmsh, " ");
}

void
show_ip_ospf_database_maxage (struct cmsh *cmsh, struct ospf *ospf)
{
	struct route_node *rn;
	struct ospf_lsa *lsa;

	cmdPrint(cmsh, "\n                MaxAge Link States:\n");

	for (rn = route_top(ospf->maxage_lsa); rn; rn = route_next(rn)) {

		if ((lsa = rn->info) != NULL) {
			cmdPrint(cmsh, "Link type: %d", lsa->data->type);
			cmdPrint(cmsh, "Link State ID: %s", inet_ntoa(lsa->data->id));
			cmdPrint(cmsh, "Advertising Router: %s",
					inet_ntoa(lsa->data->adv_router));
			cmdPrint(cmsh, "LSA lock count: %d", lsa->lock);
			cmdPrint(cmsh, "\n");
		}
	}
}

void
ospf_nbr_timer_update (struct ospf_interface *oi)
{
  struct route_node *rn;
  struct ospf_neighbor *nbr;

  for (rn = route_top (oi->nbrs); rn; rn = route_next (rn))
    if ((nbr = rn->info))
      {
	nbr->v_inactivity = OSPF_IF_PARAM (oi, v_wait);
	nbr->v_db_desc = OSPF_IF_PARAM (oi, retransmit_interval);
	nbr->v_ls_req = OSPF_IF_PARAM (oi, retransmit_interval);
	nbr->v_ls_upd = OSPF_IF_PARAM (oi, retransmit_interval);
      }
}

int
ospf_vty_dead_interval_set (struct interface *ifp,
								struct cmsh *cmsh,
								const char *interval_str,
                            const char *nbr_str,
                            const char *fast_hello_str)
{
  u_int32_t seconds;
  u_char hellomult;
  struct in_addr addr;
  int ret;
  struct ospf_if_params *params;
  struct ospf_interface *oi;
  struct route_node *rn;

  params = IF_DEF_PARAMS (ifp);

  if (nbr_str)
    {
      ret = inet_aton(nbr_str, &addr);
      if (!ret)
	{
	  cmdPrint (cmsh, "Please specify interface address by A.B.C.D\n");
	  return (CMD_IPC_WARNING);
	}

      params = ospf_get_if_params (ifp, addr);
      ospf_if_update_params (ifp, addr);
    }

  if (interval_str)
    {
      VTY_GET_INTEGER_RANGE ("Router Dead Interval", seconds, interval_str,
                             1, 65535);

      /* reset fast_hello too, just to be sure */
      UNSET_IF_PARAM (params, fast_hello);
      params->fast_hello = OSPF_FAST_HELLO_DEFAULT;
    }
  else if (fast_hello_str)
    {
      VTY_GET_INTEGER_RANGE ("Hello Multiplier", hellomult, fast_hello_str,
                             1, 10);
      /* 1s dead-interval with sub-second hellos desired */
      seconds = OSPF_ROUTER_DEAD_INTERVAL_MINIMAL;
      SET_IF_PARAM (params, fast_hello);
      params->fast_hello = hellomult;
    }
  else
    {
      cmdPrint (cmsh, "Please specify dead-interval or hello-multiplier\n");
      return (CMD_IPC_WARNING);
    }

  SET_IF_PARAM (params, v_wait);
  params->v_wait = seconds;

  /* Update timer values in neighbor structure. */
  if (nbr_str)
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

void
show_ip_ospf_route_network (struct cmsh *cmsh, struct route_table *rt)
{
	struct route_node *rn;
	struct ospf_route *or;
	struct listnode *pnode, *pnnode;
	struct ospf_path *path;

	cmdPrint(cmsh, "============ OSPF network routing table ============");

	for (rn = route_top(rt); rn; rn = route_next(rn))
		if ((or = rn->info) != NULL) {
			char buf1[19];
			snprintf(buf1, 19, "%s/%d", inet_ntoa(rn->p.u.prefix4),
					rn->p.prefixlen);

			switch (or->path_type) {
			case OSPF_PATH_INTER_AREA:
				if (or->type == OSPF_DESTINATION_NETWORK) {
					cmdPrint(cmsh, "N IA %-18s    [%d] area: %s", buf1,
							or->cost, inet_ntoa(or->u.std.area_id));
				} else if (or->type == OSPF_DESTINATION_DISCARD) {
					cmdPrint(cmsh, "D IA %-18s    Discard entry", buf1);
				}
				break;
			case OSPF_PATH_INTRA_AREA:
				cmdPrint(cmsh, "N    %-18s    [%d] area: %s", buf1, or->cost,
						inet_ntoa(or->u.std.area_id));
				break;
			default:
				break;
			}

			if (or->type == OSPF_DESTINATION_NETWORK)
				for (ALL_LIST_ELEMENTS(or->paths, pnode, pnnode, path)) {
					if (if_lookup_by_index(path->ifindex)) {
						if (path->nexthop.s_addr == 0) {
							cmdPrint(cmsh, "%24s   directly attached to %s", "",
									ifindex2ifname(path->ifindex));
						} else {
							cmdPrint(cmsh, "%24s   via %s, %s", "",
									inet_ntoa(path->nexthop),
									ifindex2ifname(path->ifindex));
						}
					}
				}
		}
	cmdPrint(cmsh, " ");
}

void
show_ip_ospf_route_router (struct cmsh *cmsh, struct route_table *rtrs)
{
	struct route_node *rn;
	struct ospf_route *or;
	struct listnode *pnode;
	struct listnode *node;
	struct ospf_path *path;
	int idx = 0;
	char showBuff[1024] = { };

	cmdPrint(cmsh, "============ OSPF router routing table =============");
	for (rn = route_top(rtrs); rn; rn = route_next(rn))
		if (rn->info) {
			int flag = 0;

			idx += sprintf(showBuff + idx, "R    %-15s    ", inet_ntoa(rn->p.u.prefix4));

			for (ALL_LIST_ELEMENTS_RO((struct list * )rn->info, node, or)) {
				if (flag++)
					idx += sprintf(showBuff + idx, "%24s", "");

				/* Show path. */
				idx += sprintf(showBuff + idx, "%s [%d] area: %s",
						(or->path_type == OSPF_PATH_INTER_AREA ? "IA" : "  "),
						or->cost, inet_ntoa(or->u.std.area_id));
				/* Show flags. */
				cmdPrint(cmsh, "%s%s%s", showBuff,
						(or->u.std.flags & ROUTER_LSA_BORDER ? ", ABR" : ""),
						(or->u.std.flags & ROUTER_LSA_EXTERNAL ? ", ASBR" : ""));

				idx = 0; memset(showBuff, 0, 1024);

				for (ALL_LIST_ELEMENTS_RO(or->paths, pnode, path)) {
					if (if_lookup_by_index(path->ifindex)) {
						if (path->nexthop.s_addr == 0)
						{
							cmdPrint(cmsh, "%24s   directly attached to %s",
									"", ifindex2ifname(path->ifindex));
						}
						else
						{
							cmdPrint(cmsh, "%24s   via %s, %s", "",
									inet_ntoa(path->nexthop),
									ifindex2ifname(path->ifindex));
						}
					}
				}
			}
		}
	cmdPrint(cmsh, " ");
}

void
show_ip_ospf_route_external (struct cmsh *cmsh, struct route_table *rt)
{
	struct route_node *rn;
	struct ospf_route *er;
	struct listnode *pnode, *pnnode;
	struct ospf_path *path;

	cmdPrint(cmsh, "============ OSPF external routing table ===========");
	for (rn = route_top(rt); rn; rn = route_next(rn))
		if ((er = rn->info) != NULL) {
			char buf1[19];
			snprintf(buf1, 19, "%s/%d", inet_ntoa(rn->p.u.prefix4),
					rn->p.prefixlen);

			switch (er->path_type) {
			case OSPF_PATH_TYPE1_EXTERNAL:
				cmdPrint(cmsh, "N E1 %-18s    [%d] tag: %u", buf1, er->cost,
						er->u.ext.tag);
				break;
			case OSPF_PATH_TYPE2_EXTERNAL:
				cmdPrint(cmsh, "N E2 %-18s    [%d/%d] tag: %u", buf1, er->cost,
						er->u.ext.type2_cost, er->u.ext.tag);
				break;
			}

			for (ALL_LIST_ELEMENTS(er->paths, pnode, pnnode, path)) {
				if (if_lookup_by_index(path->ifindex)) {
					if (path->nexthop.s_addr == 0)
					{
						cmdPrint(cmsh, "%24s   directly attached to %s", "",
								ifindex2ifname(path->ifindex));
					}
					else
					{
						cmdPrint(cmsh, "%24s   via %s, %s", "",
								inet_ntoa(path->nexthop),
								ifindex2ifname(path->ifindex));
					}
				}
			}
		}
	cmdPrint(cmsh, " ");
}

void
show_mpls_te_link_sub (struct cmsh *cmsh, struct interface *ifp)
{
  struct mpls_te_link *lp;
  struct te_tlv_header *tlvh;

  if ((OspfMplsTE.status == enabled)
  &&  (! if_is_loopback (ifp) && if_is_up (ifp) && ospf_oi_count (ifp) > 0)
  &&  ((lp = lookup_linkparams_by_ifp (ifp)) != NULL))
    {
      cmdPrint (cmsh, "-- MPLS-TE link parameters for %s --",
               ifp->name);

      show_vty_link_subtlv_link_type (cmsh, &lp->link_type.header);
      show_vty_link_subtlv_link_id (cmsh, &lp->link_id.header);

      if ((tlvh = (struct te_tlv_header *) lp->lclif_ipaddr) != NULL)
        show_vty_link_subtlv_lclif_ipaddr (cmsh, tlvh);
      if ((tlvh = (struct te_tlv_header *) lp->rmtif_ipaddr) != NULL)
        show_vty_link_subtlv_rmtif_ipaddr (cmsh, tlvh);

      show_vty_link_subtlv_te_metric (cmsh, &lp->te_metric.header);

      show_vty_link_subtlv_max_bw (cmsh, &lp->max_bw.header);
      show_vty_link_subtlv_max_rsv_bw (cmsh, &lp->max_rsv_bw.header);
      show_vty_link_subtlv_unrsv_bw (cmsh, &lp->unrsv_bw.header);
      show_vty_link_subtlv_rsc_clsclr (cmsh, &lp->rsc_clsclr.header);
    }
  else
    {
      cmdPrint (cmsh, "  %s: MPLS-TE is disabled on this interface",
               ifp->name);
    }

  return;
}



int
str2metric (const char *str, int *metric)
{
  /* Sanity check. */
  if (str == NULL)
    return 0;

  *metric = strtol (str, NULL, 10);
  if (*metric < 0 && *metric > 16777214)
    {
      /* vty_out (vty, "OSPF metric value is invalid%s", VTY_NEWLINE); */
      return 0;
    }

  return 1;
}

int
str2metric_type (const char *str, int *type)
{
  /* Sanity check. */
  if (str == NULL)
    return 0;

  if (strncmp (str, "1", 1) == 0)
    *type = EXTERNAL_METRIC_TYPE_1;
  else if (strncmp (str, "2", 1) == 0)
    *type = EXTERNAL_METRIC_TYPE_2;
  else
    return 0;

  return 1;
}

