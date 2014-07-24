/*
 * This is an implementation of draft-katz-yeung-ospf-traffic-06.txt
 * Copyright (C) 2001 KDD R&D Laboratories, Inc.
 * http://www.kddlabs.co.jp/
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

/***** MTYPE definition is not reflected to "memory.h" yet. *****/
#define MTYPE_OSPF_MPLS_TE_LINKPARAMS	0


#include "linklist.h"
#include "prefix.h"
#include "if.h"
#include "table.h"
#include "memory.h"
#include "command.h"
#include "vty.h"
#include "stream.h"
#include "log.h"
#include "hash.h"
#include "sockunion.h"		/* for inet_aton() */

#include "nnTypes.h"
#include "nnCmdCmsh.h"

#include "ospfd.h"
#include "ospf_interface.h"
#include "ospf_ism.h"
#include "ospf_asbr.h"
#include "ospf_lsa.h"
#include "ospf_lsdb.h"
#include "ospf_neighbor.h"
#include "ospf_nsm.h"
#include "ospf_flood.h"
#include "ospf_packet.h"
#include "ospf_spf.h"
#include "ospf_dump.h"
#include "ospf_route.h"
#include "ospf_ase.h"
#include "ospf_zebra.h"
#include "ospf_te.h"

#ifdef HAVE_OSPF_TE
#ifndef HAVE_OPAQUE_LSA
#error "Wrong configure option"
#endif /* HAVE_OPAQUE_LSA */




/*
 * Global variable to manage Opaque-LSA/MPLS-TE on this node.
 * Note that all parameter values are stored in network byte order.
 */
struct ospf_mpls_te OspfMplsTE;

enum oifstate {
  OI_ANY, OI_DOWN, OI_UP
};



/*------------------------------------------------------------------------*
 * Followings are initialize/terminate functions for MPLS-TE handling.
 *------------------------------------------------------------------------*/

static int ospf_mpls_te_new_if (struct interface *ifp);
static int ospf_mpls_te_del_if (struct interface *ifp);
static void ospf_mpls_te_ism_change (struct ospf_interface *oi, int old_status);
static void ospf_mpls_te_nsm_change (struct ospf_neighbor *nbr, int old_status);
static void ospfMplsTeConfigWriteRouter (struct cmsh *cmsh);
static void ospfMplsTeConfigWriteIf (struct cmsh *cmsh, struct interface *ifp);
static void ospf_mpls_te_show_info (struct cmsh *cmsh, struct ospf_lsa *lsa);
static int ospf_mpls_te_lsa_originate (void *arg);
static struct ospf_lsa *ospf_mpls_te_lsa_refresh (struct ospf_lsa *lsa);

static void del_mpls_te_link (void *val);

int
ospf_mpls_te_init (void)
{
  int rc;

  rc = ospf_register_opaque_functab (
                OSPF_OPAQUE_AREA_LSA,
                OPAQUE_TYPE_TRAFFIC_ENGINEERING_LSA,
		ospf_mpls_te_new_if,
		ospf_mpls_te_del_if,
		ospf_mpls_te_ism_change,
		ospf_mpls_te_nsm_change,
		ospfMplsTeConfigWriteRouter,
		ospfMplsTeConfigWriteIf,
		NULL,/* ospf_mpls_te_config_write_debug */
                ospf_mpls_te_show_info,
                ospf_mpls_te_lsa_originate,
                ospf_mpls_te_lsa_refresh,
		NULL,/* ospf_mpls_te_new_lsa_hook */
		NULL /* ospf_mpls_te_del_lsa_hook */);
  if (rc != 0)
    {
      zlog_warn ("ospf_mpls_te_init: Failed to register functions");
      goto out;
    }

  memset (&OspfMplsTE, 0, sizeof (struct ospf_mpls_te));
  OspfMplsTE.status = disabled;
  OspfMplsTE.iflist = list_new ();
  OspfMplsTE.iflist->del = del_mpls_te_link;


out:
  return rc;
}

void
ospf_mpls_te_term (void)
{
  list_delete (OspfMplsTE.iflist);

  OspfMplsTE.iflist = NULL;
  OspfMplsTE.status = disabled;

  ospf_delete_opaque_functab (OSPF_OPAQUE_AREA_LSA,
                              OPAQUE_TYPE_TRAFFIC_ENGINEERING_LSA);
  return;
}

/*------------------------------------------------------------------------*
 * Followings are control functions for MPLS-TE parameters management.
 *------------------------------------------------------------------------*/

static void
del_mpls_te_link (void *val)
{
  XFREE (MTYPE_OSPF_MPLS_TE_LINKPARAMS, val);
  return;
}

static u_int32_t
get_mpls_te_instance_value (void)
{
  static u_int32_t seqno = 0;

  if (seqno < MAX_LEGAL_TE_INSTANCE_NUM )
    seqno += 1;
  else
    seqno  = 1; /* Avoid zero. */

  return seqno;
}

static struct ospf_interface *
lookup_oi_by_ifp (struct interface *ifp,
                  struct ospf_area *area, enum oifstate oifstate)
{
  struct ospf_interface *oi = NULL;
  struct route_node *rn;

  for (rn = route_top (IF_OIFS (ifp)); rn; rn = route_next (rn))
    {
      if ((oi = rn->info) == NULL)
        continue;

      switch (oifstate)
        {
        case OI_ANY:
          break;
        case OI_DOWN:
          if (ospf_if_is_enable (oi))
            continue;
          break;
        case OI_UP:
          if (! ospf_if_is_enable (oi))
            continue;
          break;
        default:
          zlog_warn ("lookup_oi_by_ifp: Unknown oifstate: %x", oifstate);
          goto out;
        }

      if (area == NULL || oi->area == area)
        return oi;
    }
out:
  return NULL;
}

struct mpls_te_link *
lookup_linkparams_by_ifp (struct interface *ifp)
{
  struct listnode *node, *nnode;
  struct mpls_te_link *lp;

  for (ALL_LIST_ELEMENTS (OspfMplsTE.iflist, node, nnode, lp))
    if (lp->ifp == ifp)
      return lp;

  return NULL;
}

static struct mpls_te_link *
lookup_linkparams_by_instance (struct ospf_lsa *lsa)
{
  struct listnode *node;
  struct mpls_te_link *lp;
  unsigned int key = GET_OPAQUE_ID (ntohl (lsa->data->id.s_addr));

  for (ALL_LIST_ELEMENTS_RO (OspfMplsTE.iflist, node, lp))
    if (lp->instance == key)
      return lp;

  zlog_warn ("lookup_linkparams_by_instance: Entry not found: key(%x)", key);
  return NULL;
}

void
ospf_mpls_te_foreach_area (
  void (*func)(struct mpls_te_link *lp, enum sched_opcode),
  enum sched_opcode sched_opcode)
{
  struct listnode *node, *nnode; 
  struct listnode *node2;
  struct mpls_te_link *lp;
  struct ospf_area *area;

  for (ALL_LIST_ELEMENTS (OspfMplsTE.iflist, node, nnode, lp))
    {
      if ((area = lp->area) == NULL)
        continue;
      if (lp->flags & LPFLG_LOOKUP_DONE)
        continue;

      if (func != NULL)
        (* func)(lp, sched_opcode);

      for (node2 = listnextnode (node); node2; node2 = listnextnode (node2))
        if ((lp = listgetdata (node2)) != NULL)
          if (lp->area != NULL)
            if (IPV4_ADDR_SAME (&lp->area->area_id, &area->area_id))
              lp->flags |= LPFLG_LOOKUP_DONE;
    }

  for (ALL_LIST_ELEMENTS_RO (OspfMplsTE.iflist, node, lp))
    if (lp->area != NULL)
      lp->flags &= ~LPFLG_LOOKUP_DONE;

  return;
}

void
set_mpls_te_router_addr (struct in_addr ipv4)
{
  OspfMplsTE.router_addr.header.type   = htons (TE_TLV_ROUTER_ADDR);
  OspfMplsTE.router_addr.header.length = htons (sizeof (ipv4));
  OspfMplsTE.router_addr.value = ipv4;
  return;
}

static void
set_linkparams_link_header (struct mpls_te_link *lp)
{
  struct te_tlv_header *tlvh;
  u_int16_t length = 0;

  /* TE_LINK_SUBTLV_LINK_TYPE */
  if (ntohs (lp->link_type.header.type) != 0)
    length += TLV_SIZE (&lp->link_type.header);

  /* TE_LINK_SUBTLV_LINK_ID */
  if (ntohs (lp->link_id.header.type) != 0)
    length += TLV_SIZE (&lp->link_id.header);

  /* TE_LINK_SUBTLV_LCLIF_IPADDR */
  if ((tlvh = (struct te_tlv_header *) lp->lclif_ipaddr) != NULL
  &&  ntohs (tlvh->type) != 0)
    length += TLV_SIZE (tlvh);

  /* TE_LINK_SUBTLV_RMTIF_IPADDR */
  if ((tlvh = (struct te_tlv_header *) lp->rmtif_ipaddr) != NULL
  &&  ntohs (tlvh->type) != 0)
    length += TLV_SIZE (tlvh);

  /* TE_LINK_SUBTLV_TE_METRIC */
  if (ntohs (lp->te_metric.header.type) != 0)
    length += TLV_SIZE (&lp->te_metric.header);

  /* TE_LINK_SUBTLV_MAX_BW */
  if (ntohs (lp->max_bw.header.type) != 0)
    length += TLV_SIZE (&lp->max_bw.header);

  /* TE_LINK_SUBTLV_MAX_RSV_BW */
  if (ntohs (lp->max_rsv_bw.header.type) != 0)
    length += TLV_SIZE (&lp->max_rsv_bw.header);

  /* TE_LINK_SUBTLV_UNRSV_BW */
  if (ntohs (lp->unrsv_bw.header.type) != 0)
    length += TLV_SIZE (&lp->unrsv_bw.header);

  /* TE_LINK_SUBTLV_RSC_CLSCLR */
  if (ntohs (lp->rsc_clsclr.header.type) != 0)
    length += TLV_SIZE (&lp->rsc_clsclr.header);

  lp->link_header.header.type   = htons (TE_TLV_LINK);
  lp->link_header.header.length = htons (length);

  return;
}

static void
set_linkparams_link_type (struct ospf_interface *oi, struct mpls_te_link *lp)
{
  lp->link_type.header.type   = htons (TE_LINK_SUBTLV_LINK_TYPE);
  lp->link_type.header.length = htons (sizeof (lp->link_type.link_type.value));

  switch (oi->type)
    {
    case OSPF_IFTYPE_POINTOPOINT:
      lp->link_type.link_type.value = LINK_TYPE_SUBTLV_VALUE_PTP;
      break;
    case OSPF_IFTYPE_BROADCAST:
    case OSPF_IFTYPE_NBMA:
      lp->link_type.link_type.value = LINK_TYPE_SUBTLV_VALUE_MA;
      break;
    default:
      /* Not supported yet. *//* XXX */
      lp->link_type.header.type = htons (0);
      break;
    }
  return;
}

static void
set_linkparams_link_id (struct ospf_interface *oi, struct mpls_te_link *lp)
{
  struct ospf_neighbor *nbr;
  int done = 0;

  lp->link_id.header.type   = htons (TE_LINK_SUBTLV_LINK_ID);
  lp->link_id.header.length = htons (sizeof (lp->link_id.value));

  /*
   * The Link ID is identical to the contents of the Link ID field
   * in the Router LSA for these link types.
   */
  switch (oi->type)
    {
    case OSPF_IFTYPE_POINTOPOINT:
      /* Take the router ID of the neighbor. */
      if ((nbr = ospf_nbr_lookup_ptop (oi))
	  && nbr->state == NSM_Full)
        {
          lp->link_id.value = nbr->router_id;
          done = 1;
        }
      break;
    case OSPF_IFTYPE_BROADCAST:
    case OSPF_IFTYPE_NBMA:
      /* Take the interface address of the designated router. */
      if ((nbr = ospf_nbr_lookup_by_addr (oi->nbrs, &DR (oi))) == NULL)
        break;

      if (nbr->state == NSM_Full
      || (IPV4_ADDR_SAME (&oi->address->u.prefix4, &DR (oi))
      &&  ospf_nbr_count (oi, NSM_Full) > 0))
        {
          lp->link_id.value = DR (oi);
          done = 1;
        }
      break;
    default:
      /* Not supported yet. *//* XXX */
      lp->link_id.header.type = htons (0);
      break;
    }

  if (! done)
    {
      struct in_addr mask;
      masklen2ip (oi->address->prefixlen, &mask);
      lp->link_id.value.s_addr = oi->address->u.prefix4.s_addr & mask.s_addr;
     }
  return;
}

void
set_linkparams_te_metric (struct mpls_te_link *lp, u_int32_t te_metric)
{
  lp->te_metric.header.type   = htons (TE_LINK_SUBTLV_TE_METRIC);
  lp->te_metric.header.length = htons (sizeof (lp->te_metric.value));
  lp->te_metric.value = htonl (te_metric);
  return;
}

void
set_linkparams_max_bw (struct mpls_te_link *lp, float *fp)
{
  lp->max_bw.header.type   = htons (TE_LINK_SUBTLV_MAX_BW);
  lp->max_bw.header.length = htons (sizeof (lp->max_bw.value));
  htonf (fp, &lp->max_bw.value);
  return;
}

void
set_linkparams_max_rsv_bw (struct mpls_te_link *lp, float *fp)
{
  lp->max_rsv_bw.header.type   = htons (TE_LINK_SUBTLV_MAX_RSV_BW);
  lp->max_rsv_bw.header.length = htons (sizeof (lp->max_rsv_bw.value));
  htonf (fp, &lp->max_rsv_bw.value);
  return;
}

void
set_linkparams_unrsv_bw (struct mpls_te_link *lp, int priority, float *fp)
{
  /* Note that TLV-length field is the size of array. */
  lp->unrsv_bw.header.type   = htons (TE_LINK_SUBTLV_UNRSV_BW);
  lp->unrsv_bw.header.length = htons (sizeof (lp->unrsv_bw.value));
  htonf (fp, &lp->unrsv_bw.value [priority]);
  return;
}

void
set_linkparams_rsc_clsclr (struct mpls_te_link *lp, u_int32_t classcolor)
{
  lp->rsc_clsclr.header.type   = htons (TE_LINK_SUBTLV_RSC_CLSCLR);
  lp->rsc_clsclr.header.length = htons (sizeof (lp->rsc_clsclr.value));
  lp->rsc_clsclr.value = htonl (classcolor);
  return;
}

void
initialize_linkparams (struct mpls_te_link *lp)
{
  struct interface *ifp = lp->ifp;
  struct ospf_interface *oi;
  float fval;
  int i;

  if ((oi = lookup_oi_by_ifp (ifp, NULL, OI_ANY)) == NULL)
    return;

  /*
   * Try to set initial values those can be derived from
   * zebra-interface information.
   */
  set_linkparams_link_type (oi, lp);

  /*
   * Linux and *BSD kernel holds bandwidth parameter as an "int" type.
   * We may have to reconsider, if "ifp->bandwidth" type changes to float.
   */
  fval = (float)((ifp->bandwidth ? ifp->bandwidth
                                 : OSPF_DEFAULT_BANDWIDTH) * 1000 / 8);

  set_linkparams_max_bw (lp, &fval);
  set_linkparams_max_rsv_bw (lp, &fval);

  for (i = 0; i < 8; i++)
    set_linkparams_unrsv_bw (lp, i, &fval);

  return;
}

static int
is_mandated_params_set (struct mpls_te_link *lp)
{
  int rc = 0;

  if (ntohs (OspfMplsTE.router_addr.header.type) == 0)
    goto out;

  if (ntohs (lp->link_type.header.type) == 0)
    goto out;

  if (ntohs (lp->link_id.header.type) == 0)
    goto out;

  rc = 1;
out:
  return rc;
}

/*------------------------------------------------------------------------*
 * Followings are callback functions against generic Opaque-LSAs handling.
 *------------------------------------------------------------------------*/

static int
ospf_mpls_te_new_if (struct interface *ifp)
{
  struct mpls_te_link *new;
  int rc = -1;

  if (lookup_linkparams_by_ifp (ifp) != NULL)
    {
      zlog_warn ("ospf_mpls_te_new_if: ifp(%p) already in use?", ifp);
      rc = 0; /* Do nothing here. */
      goto out;
    }

  new = XCALLOC (MTYPE_OSPF_MPLS_TE_LINKPARAMS,
                  sizeof (struct mpls_te_link));
  if (new == NULL)
    {
      zlog_warn ("ospf_mpls_te_new_if: XMALLOC: %s", safe_strerror (errno));
      goto out;
    }

  new->area = NULL;
  new->flags = 0;
  new->instance = get_mpls_te_instance_value ();
  new->ifp = ifp;

  initialize_linkparams (new);

  listnode_add (OspfMplsTE.iflist, new);

  /* Schedule Opaque-LSA refresh. *//* XXX */

  rc = 0;
out:
  return rc;
}

static int
ospf_mpls_te_del_if (struct interface *ifp)
{
  struct mpls_te_link *lp;
  int rc = -1;

  if ((lp = lookup_linkparams_by_ifp (ifp)) != NULL)
    {
      struct list *iflist_in = OspfMplsTE.iflist;

      /* Dequeue listnode entry from the list. */
      listnode_delete (iflist_in, lp);

      /* Avoid misjudgement in the next lookup. */
      if (listcount (iflist_in) == 0)
    	  iflist_in->head = iflist_in->tail = NULL;

      XFREE (MTYPE_OSPF_MPLS_TE_LINKPARAMS, lp);
    }

  /* Schedule Opaque-LSA refresh. *//* XXX */

  rc = 0;
/*out:*/
  return rc;
}

static void
ospf_mpls_te_ism_change (struct ospf_interface *oi, int old_state)
{
  struct te_link_subtlv_link_type old_type;
  struct te_link_subtlv_link_id   old_id;
  struct mpls_te_link *lp;

  if ((lp = lookup_linkparams_by_ifp (oi->ifp)) == NULL)
    {
      zlog_warn ("ospf_mpls_te_ism_change: Cannot get linkparams from OI(%s)?", IF_NAME (oi));
      goto out;
    }
  if (oi->area == NULL || oi->area->ospf == NULL)
    {
      zlog_warn ("ospf_mpls_te_ism_change: Cannot refer to OSPF from OI(%s)?",
IF_NAME (oi));
      goto out;
    }
#ifdef notyet
  if ((lp->area != NULL
  &&   ! IPV4_ADDR_SAME (&lp->area->area_id, &oi->area->area_id))
  || (lp->area != NULL && oi->area == NULL))
    {
      /* How should we consider this case? */
      zlog_warn ("MPLS-TE: Area for OI(%s) has changed to [%s], flush previous LSAs", IF_NAME (oi), oi->area ? inet_ntoa (oi->area->area_id) : "N/A");
      ospf_mpls_te_lsa_schedule (lp, FLUSH_THIS_LSA);
    }
#endif
  /* Keep Area information in conbination with linkparams. */
  lp->area = oi->area;

  switch (oi->state)
    {
    case ISM_PointToPoint:
    case ISM_DROther:
    case ISM_Backup:
    case ISM_DR:
      old_type = lp->link_type;
      old_id   = lp->link_id;

      set_linkparams_link_type (oi, lp);
      set_linkparams_link_id (oi, lp);

      if ((ntohs (old_type.header.type) != ntohs (lp->link_type.header.type)
      ||   old_type.link_type.value     != lp->link_type.link_type.value)
      ||  (ntohs (old_id.header.type)   != ntohs (lp->link_id.header.type)
      ||   ntohl (old_id.value.s_addr)  != ntohl (lp->link_id.value.s_addr)))
        {
          if (lp->flags & LPFLG_LSA_ENGAGED)
            ospf_mpls_te_lsa_schedule (lp, REFRESH_THIS_LSA);
          else
            ospf_mpls_te_lsa_schedule (lp, REORIGINATE_PER_AREA);
        }
      break;
    default:
      lp->link_type.header.type = htons (0);
      lp->link_id.header.type   = htons (0);

      if (lp->flags & LPFLG_LSA_ENGAGED)
        ospf_mpls_te_lsa_schedule (lp, FLUSH_THIS_LSA);
      break;
    }

out:
  return;
}

static void
ospf_mpls_te_nsm_change (struct ospf_neighbor *nbr, int old_state)
{
  /* So far, nothing to do here. */
  return;
}

/*------------------------------------------------------------------------*
 * Followings are OSPF protocol processing functions for MPLS-TE.
 *------------------------------------------------------------------------*/

static void
build_tlv_header (struct stream *s, struct te_tlv_header *tlvh)
{
  stream_put (s, tlvh, sizeof (struct te_tlv_header));
  return;
}

static void
build_router_tlv (struct stream *s)
{
  struct te_tlv_header *tlvh = &OspfMplsTE.router_addr.header;
  if (ntohs (tlvh->type) != 0)
    {
      build_tlv_header (s, tlvh);
      stream_put (s, tlvh+1, TLV_BODY_SIZE (tlvh));
    }
  return;
}

static void
build_link_subtlv_link_type (struct stream *s, struct mpls_te_link *lp)
{
  struct te_tlv_header *tlvh = &lp->link_type.header;
  if (ntohs (tlvh->type) != 0)
    {
      build_tlv_header (s, tlvh);
      stream_put (s, tlvh+1, TLV_BODY_SIZE (tlvh));
    }
  return;
}

static void
build_link_subtlv_link_id (struct stream *s, struct mpls_te_link *lp)
{
  struct te_tlv_header *tlvh = &lp->link_id.header;
  if (ntohs (tlvh->type) != 0)
    {
      build_tlv_header (s, tlvh);
      stream_put (s, tlvh+1, TLV_BODY_SIZE (tlvh));
    }
  return;
}

static void
build_link_subtlv_lclif_ipaddr (struct stream *s, struct mpls_te_link *lp)
{
  struct te_tlv_header *tlvh = (struct te_tlv_header *) lp->lclif_ipaddr;
  if (tlvh != NULL && ntohs (tlvh->type) != 0)
    {
      build_tlv_header (s, tlvh);
      stream_put (s, tlvh+1, TLV_BODY_SIZE (tlvh));
    }
  return;
}

static void
build_link_subtlv_rmtif_ipaddr (struct stream *s, struct mpls_te_link *lp)
{
  struct te_tlv_header *tlvh = (struct te_tlv_header *) lp->rmtif_ipaddr;
  if (tlvh != NULL && ntohs (tlvh->type) != 0)
    {
      build_tlv_header (s, tlvh);
      stream_put (s, tlvh+1, TLV_BODY_SIZE (tlvh));
    }
  return;
}

static void
build_link_subtlv_te_metric (struct stream *s, struct mpls_te_link *lp)
{
  struct te_tlv_header *tlvh = &lp->te_metric.header;
  if (ntohs (tlvh->type) != 0)
    {
      build_tlv_header (s, tlvh);
      stream_put (s, tlvh+1, TLV_BODY_SIZE (tlvh));
    }
  return;
}

static void
build_link_subtlv_max_bw (struct stream *s, struct mpls_te_link *lp)
{
  struct te_tlv_header *tlvh = &lp->max_bw.header;
  if (ntohs (tlvh->type) != 0)
    {
      build_tlv_header (s, tlvh);
      stream_put (s, tlvh+1, TLV_BODY_SIZE (tlvh));
    }
  return;
}

static void
build_link_subtlv_max_rsv_bw (struct stream *s, struct mpls_te_link *lp)
{
  struct te_tlv_header *tlvh = &lp->max_rsv_bw.header;
  if (ntohs (tlvh->type) != 0)
    {
      build_tlv_header (s, tlvh);
      stream_put (s, tlvh+1, TLV_BODY_SIZE (tlvh));
    }
  return;
}

static void
build_link_subtlv_unrsv_bw (struct stream *s, struct mpls_te_link *lp)
{
  struct te_tlv_header *tlvh = &lp->unrsv_bw.header;
  if (ntohs (tlvh->type) != 0)
    {
      build_tlv_header (s, tlvh);
      stream_put (s, tlvh+1, TLV_BODY_SIZE (tlvh));
    }
  return;
}

static void
build_link_subtlv_rsc_clsclr (struct stream *s, struct mpls_te_link *lp)
{
  struct te_tlv_header *tlvh = &lp->rsc_clsclr.header;
  if (ntohs (tlvh->type) != 0)
    {
      build_tlv_header (s, tlvh);
      stream_put (s, tlvh+1, TLV_BODY_SIZE (tlvh));
    }
  return;
}

static void
build_link_tlv (struct stream *s, struct mpls_te_link *lp)
{
  set_linkparams_link_header (lp);
  build_tlv_header (s, &lp->link_header.header);

  build_link_subtlv_link_type (s, lp);
  build_link_subtlv_link_id (s, lp);
  build_link_subtlv_lclif_ipaddr (s, lp);
  build_link_subtlv_rmtif_ipaddr (s, lp);
  build_link_subtlv_te_metric (s, lp);
  build_link_subtlv_max_bw (s, lp);
  build_link_subtlv_max_rsv_bw (s, lp);
  build_link_subtlv_unrsv_bw (s, lp);
  build_link_subtlv_rsc_clsclr (s, lp);
  return;
}

static void
ospf_mpls_te_lsa_body_set (struct stream *s, struct mpls_te_link *lp)
{
  /*
   * The router address TLV is type 1, and ...
   *                                      It must appear in exactly one
   * Traffic Engineering LSA originated by a router.
   */
  build_router_tlv (s);

  /*
   * Only one Link TLV shall be carried in each LSA, allowing for fine
   * granularity changes in topology.
   */
  build_link_tlv (s, lp);
  return;
}

/* Create new opaque-LSA. */
static struct ospf_lsa *
ospf_mpls_te_lsa_new (struct ospf_area *area, struct mpls_te_link *lp)
{
  struct stream *s;
  struct lsa_header *lsah;
  struct ospf_lsa *new = NULL;
  u_char options, lsa_type;
  struct in_addr lsa_id;
  u_int32_t tmp;
  u_int16_t length;

  /* Create a stream for LSA. */
  if ((s = stream_new (OSPF_MAX_LSA_SIZE)) == NULL)
    {
      zlog_warn ("ospf_mpls_te_lsa_new: stream_new() ?");
      goto out;
    }
  lsah = (struct lsa_header *) STREAM_DATA (s);

  options  = LSA_OPTIONS_GET (area);
  options |= LSA_OPTIONS_NSSA_GET (area);
  options |= OSPF_OPTION_O; /* Don't forget this :-) */

  lsa_type = OSPF_OPAQUE_AREA_LSA;
  tmp = SET_OPAQUE_LSID (OPAQUE_TYPE_TRAFFIC_ENGINEERING_LSA, lp->instance);
  lsa_id.s_addr = htonl (tmp);

  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    zlog_debug ("LSA[Type%d:%s]: Create an Opaque-LSA/MPLS-TE instance", lsa_type, inet_ntoa (lsa_id));

  /* Set opaque-LSA header fields. */
  lsa_header_set (s, options, lsa_type, lsa_id, area->ospf->router_id);

  /* Set opaque-LSA body fields. */
  ospf_mpls_te_lsa_body_set (s, lp);

  /* Set length. */
  length = stream_get_endp (s);
  lsah->length = htons (length);

  /* Now, create an OSPF LSA instance. */
  if ((new = ospf_lsa_new ()) == NULL)
    {
      zlog_warn ("ospf_mpls_te_lsa_new: ospf_lsa_new() ?");
      stream_free (s);
      goto out;
    }
  if ((new->data = ospf_lsa_data_new (length)) == NULL)
    {
      zlog_warn ("ospf_mpls_te_lsa_new: ospf_lsa_data_new() ?");
      ospf_lsa_unlock (&new);
      new = NULL;
      stream_free (s);
      goto out;
    }

  new->area = area;
  SET_FLAG (new->flags, OSPF_LSA_SELF);
  memcpy (new->data, lsah, length);
  stream_free (s);

out:
  return new;
}

static int
ospf_mpls_te_lsa_originate1 (struct ospf_area *area, struct mpls_te_link *lp)
{
  struct ospf_lsa *new;
  int rc = -1;

  /* Create new Opaque-LSA/MPLS-TE instance. */
  if ((new = ospf_mpls_te_lsa_new (area, lp)) == NULL)
    {
      zlog_warn ("ospf_mpls_te_lsa_originate1: ospf_mpls_te_lsa_new() ?");
      goto out;
    }

  /* Install this LSA into LSDB. */
  if (ospf_lsa_install (area->ospf, NULL/*oi*/, new) == NULL)
    {
      zlog_warn ("ospf_mpls_te_lsa_originate1: ospf_lsa_install() ?");
      ospf_lsa_unlock (&new);
      goto out;
    }

  /* Now this linkparameter entry has associated LSA. */
  lp->flags |= LPFLG_LSA_ENGAGED;

  /* Update new LSA origination count. */
  area->ospf->lsa_originate_count++;

  /* Flood new LSA through area. */
  ospf_flood_through_area (area, NULL/*nbr*/, new);

  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    {
      char area_id[INET_ADDRSTRLEN];
      strcpy (area_id, inet_ntoa (area->area_id));
      zlog_debug ("LSA[Type%d:%s]: Originate Opaque-LSA/MPLS-TE: Area(%s), Link(%s)", new->data->type, inet_ntoa (new->data->id), area_id, lp->ifp->name);
      ospf_lsa_header_dump (new->data);
    }

  rc = 0;
out:
  return rc;
}

static int
ospf_mpls_te_lsa_originate (void *arg)
{
  struct ospf_area *area = (struct ospf_area *) arg;
  struct listnode *node, *nnode;
  struct mpls_te_link *lp;
  int rc = -1;

  if (OspfMplsTE.status == disabled)
    {
      zlog_info ("ospf_mpls_te_lsa_originate: MPLS-TE is disabled now.");
      rc = 0; /* This is not an error case. */
      goto out;
    }

  for (ALL_LIST_ELEMENTS (OspfMplsTE.iflist, node, nnode, lp))
    {
      if (lp->area == NULL)
        continue;
      if (! IPV4_ADDR_SAME (&lp->area->area_id, &area->area_id))
        continue;

      if (lp->flags & LPFLG_LSA_ENGAGED)
        {
          if (lp->flags & LPFLG_LSA_FORCED_REFRESH)
            {
              lp->flags &= ~LPFLG_LSA_FORCED_REFRESH;
              ospf_mpls_te_lsa_schedule (lp, REFRESH_THIS_LSA);
            }
          continue;
        }
      if (! is_mandated_params_set (lp))
        {
          zlog_warn ("ospf_mpls_te_lsa_originate: Link(%s) lacks some mandated MPLS-TE parameters.", lp->ifp ? lp->ifp->name : "?");
          continue;
        }

      /* Ok, let's try to originate an LSA for this area and Link. */
      if (ospf_mpls_te_lsa_originate1 (area, lp) != 0)
        goto out;
    }

  rc = 0;
out:
  return rc;
}

static struct ospf_lsa *
ospf_mpls_te_lsa_refresh (struct ospf_lsa *lsa)
{
  struct mpls_te_link *lp;
  struct ospf_area *area = lsa->area;
  struct ospf_lsa *new = NULL;

  if (OspfMplsTE.status == disabled)
    {
      /*
       * This LSA must have flushed before due to MPLS-TE status change.
       * It seems a slip among routers in the routing domain.
       */
      zlog_info ("ospf_mpls_te_lsa_refresh: MPLS-TE is disabled now.");
      lsa->data->ls_age = htons (OSPF_LSA_MAXAGE); /* Flush it anyway. */
    }

  /* At first, resolve lsa/lp relationship. */
  if ((lp = lookup_linkparams_by_instance (lsa)) == NULL)
    {
      zlog_warn ("ospf_mpls_te_lsa_refresh: Invalid parameter?");
      lsa->data->ls_age = htons (OSPF_LSA_MAXAGE); /* Flush it anyway. */
    }

  /* If the lsa's age reached to MaxAge, start flushing procedure. */
  if (IS_LSA_MAXAGE (lsa))
    {
      lp->flags &= ~LPFLG_LSA_ENGAGED;
      ospf_opaque_lsa_flush_schedule (lsa);
      goto out;
    }

  /* Create new Opaque-LSA/MPLS-TE instance. */
  if ((new = ospf_mpls_te_lsa_new (area, lp)) == NULL)
    {
      zlog_warn ("ospf_mpls_te_lsa_refresh: ospf_mpls_te_lsa_new() ?");
      goto out;
    }
  new->data->ls_seqnum = lsa_seqnum_increment (lsa);

  /* Install this LSA into LSDB. */
  /* Given "lsa" will be freed in the next function. */
  if (ospf_lsa_install (area->ospf, NULL/*oi*/, new) == NULL)
    {
      zlog_warn ("ospf_mpls_te_lsa_refresh: ospf_lsa_install() ?");
      ospf_lsa_unlock (&new);
      goto out;
    }

  /* Flood updated LSA through area. */
  ospf_flood_through_area (area, NULL/*nbr*/, new);

  /* Debug logging. */
  if (IS_DEBUG_OSPF (lsa, LSA_GENERATE))
    {
      zlog_debug ("LSA[Type%d:%s]: Refresh Opaque-LSA/MPLS-TE",
		 new->data->type, inet_ntoa (new->data->id));
      ospf_lsa_header_dump (new->data);
    }

out:
  return new;
}

void
ospf_mpls_te_lsa_schedule (struct mpls_te_link *lp,
                           enum sched_opcode opcode)
{
  struct ospf_lsa lsa;
  struct lsa_header lsah;
  u_int32_t tmp;

  memset (&lsa, 0, sizeof (lsa));
  memset (&lsah, 0, sizeof (lsah));

  lsa.area = lp->area;
  lsa.data = &lsah;
  lsah.type = OSPF_OPAQUE_AREA_LSA;
  tmp = SET_OPAQUE_LSID (OPAQUE_TYPE_TRAFFIC_ENGINEERING_LSA, lp->instance);
  lsah.id.s_addr = htonl (tmp);

  switch (opcode)
    {
    case REORIGINATE_PER_AREA:
      ospf_opaque_lsa_reoriginate_schedule ((void *) lp->area,
          OSPF_OPAQUE_AREA_LSA, OPAQUE_TYPE_TRAFFIC_ENGINEERING_LSA);
      break;
    case REFRESH_THIS_LSA:
      ospf_opaque_lsa_refresh_schedule (&lsa);
      break;
    case FLUSH_THIS_LSA:
      lp->flags &= ~LPFLG_LSA_ENGAGED;
      ospf_opaque_lsa_flush_schedule (&lsa);
      break;
    default:
      zlog_warn ("ospf_mpls_te_lsa_schedule: Unknown opcode (%u)", opcode);
      break;
    }

  return;
}

/*------------------------------------------------------------------------*
 * Followings are vty session control functions.
 *------------------------------------------------------------------------*/

u_int16_t
show_vty_router_addr (struct cmsh *cmsh, struct te_tlv_header *tlvh)
{
  struct te_tlv_router_addr *top = (struct te_tlv_router_addr *) tlvh;

  if (cmsh != NULL)
  {
    cmdPrint (cmsh, "  Router-Address: %s", inet_ntoa (top->value));
  }
  else
  {
    zlog_debug ("    Router-Address: %s", inet_ntoa (top->value));
  }

  return TLV_SIZE (tlvh);
}

static u_int16_t
show_vty_link_header (struct cmsh *cmsh, struct te_tlv_header *tlvh)
{
  struct te_tlv_link *top = (struct te_tlv_link *) tlvh;

  if (cmsh != NULL)
  {
    cmdPrint (cmsh, "  Link: %u octets of data", ntohs (top->header.length));
  }
  else
  {
    zlog_debug ("    Link: %u octets of data", ntohs (top->header.length));
  }

  return TLV_HDR_SIZE;	/* Here is special, not "TLV_SIZE". */
}

u_int16_t
show_vty_link_subtlv_link_type (struct cmsh *cmsh, struct te_tlv_header *tlvh)
{
  struct te_link_subtlv_link_type *top;
  const char *cp = "Unknown";

  top = (struct te_link_subtlv_link_type *) tlvh;
  switch (top->link_type.value)
    {
    case LINK_TYPE_SUBTLV_VALUE_PTP:
      cp = "Point-to-point";
      break;
    case LINK_TYPE_SUBTLV_VALUE_MA:
      cp = "Multiaccess";
      break;
    default:
      break;
    }

  if (cmsh != NULL)
  {
    cmdPrint (cmsh, "  Link-Type: %s (%u)", cp, top->link_type.value);
  }
  else
  {
    zlog_debug ("    Link-Type: %s (%u)", cp, top->link_type.value);
  }

  return TLV_SIZE (tlvh);
}

u_int16_t
show_vty_link_subtlv_link_id (struct cmsh *cmsh, struct te_tlv_header *tlvh)
{
  struct te_link_subtlv_link_id *top;

  top = (struct te_link_subtlv_link_id *) tlvh;
  if (cmsh != NULL)
  {
    cmdPrint (cmsh, "  Link-ID: %s", inet_ntoa (top->value));
  }
  else
  {
    zlog_debug ("    Link-ID: %s", inet_ntoa (top->value));
  }

  return TLV_SIZE (tlvh);
}

u_int16_t
show_vty_link_subtlv_lclif_ipaddr (struct cmsh *cmsh, struct te_tlv_header *tlvh)
{
  struct te_link_subtlv_lclif_ipaddr *top;
  int i, n;

  top = (struct te_link_subtlv_lclif_ipaddr *) tlvh;
  n = ntohs (tlvh->length) / sizeof (top->value[0]);

  if (cmsh != NULL)
  {
    cmdPrint (cmsh, "  Local Interface IP Address(es): %d", n);
  }
  else
  {
    zlog_debug ("    Local Interface IP Address(es): %d", n);
  }

  for (i = 0; i < n; i++)
    {
      if (cmsh != NULL)
      {
        cmdPrint (cmsh, "    #%d: %s", i, inet_ntoa (top->value[i]));
      }
      else
      {
        zlog_debug ("      #%d: %s", i, inet_ntoa (top->value[i]));
      }
    }
  return TLV_SIZE (tlvh);
}

u_int16_t
show_vty_link_subtlv_rmtif_ipaddr (struct cmsh *cmsh, struct te_tlv_header *tlvh)
{
  struct te_link_subtlv_rmtif_ipaddr *top;
  int i, n;

  top = (struct te_link_subtlv_rmtif_ipaddr *) tlvh;
  n = ntohs (tlvh->length) / sizeof (top->value[0]);
  if (cmsh != NULL)
  {
    cmdPrint (cmsh, "  Remote Interface IP Address(es): %d", n);
  }
  else
  {
    zlog_debug ("    Remote Interface IP Address(es): %d", n);
  }

  for (i = 0; i < n; i++)
    {
      if (cmsh != NULL)
      {
        cmdPrint (cmsh, "    #%d: %s", i, inet_ntoa (top->value[i]));
      }
      else
      {
        zlog_debug ("      #%d: %s", i, inet_ntoa (top->value[i]));
      }
    }
  return TLV_SIZE (tlvh);
}

u_int16_t
show_vty_link_subtlv_te_metric (struct cmsh *cmsh, struct te_tlv_header *tlvh)
{
  struct te_link_subtlv_te_metric *top;

  top = (struct te_link_subtlv_te_metric *) tlvh;
  if (cmsh != NULL)
  {
    cmdPrint (cmsh, "  Traffic Engineering Metric: %u", (u_int32_t) ntohl (top->value));
  }
  else
  {
    zlog_debug ("    Traffic Engineering Metric: %u", (u_int32_t) ntohl (top->value));
  }

  return TLV_SIZE (tlvh);
}

u_int16_t
show_vty_link_subtlv_max_bw (struct cmsh *cmsh, struct te_tlv_header *tlvh)
{
  struct te_link_subtlv_max_bw *top;
  float fval;

  top = (struct te_link_subtlv_max_bw *) tlvh;
  ntohf (&top->value, &fval);

  if (cmsh != NULL)
  {
    cmdPrint (cmsh, "  Maximum Bandwidth: %g (Bytes/sec)", fval);
  }
  else
  {
    zlog_debug ("    Maximum Bandwidth: %g (Bytes/sec)", fval);
  }

  return TLV_SIZE (tlvh);
}

u_int16_t
show_vty_link_subtlv_max_rsv_bw (struct cmsh *cmsh, struct te_tlv_header *tlvh)
{
  struct te_link_subtlv_max_rsv_bw *top;
  float fval;

  top = (struct te_link_subtlv_max_rsv_bw *) tlvh;
  ntohf (&top->value, &fval);

  if (cmsh != NULL)
  {
    cmdPrint (cmsh, "  Maximum Reservable Bandwidth: %g (Bytes/sec)", fval);
  }
  else
  {
    zlog_debug ("    Maximum Reservable Bandwidth: %g (Bytes/sec)", fval);
  }

  return TLV_SIZE (tlvh);
}

u_int16_t
show_vty_link_subtlv_unrsv_bw (struct cmsh *cmsh, struct te_tlv_header *tlvh)
{
  struct te_link_subtlv_unrsv_bw *top;
  float fval;
  int i;

  top = (struct te_link_subtlv_unrsv_bw *) tlvh;
  for (i = 0; i < 8; i++)
    {
      ntohf (&top->value[i], &fval);
      if (cmsh != NULL)
      {
        cmdPrint (cmsh, "  Unreserved Bandwidth (pri %d): %g (Bytes/sec)", i, fval);
      }
      else
      {
        zlog_debug ("    Unreserved Bandwidth (pri %d): %g (Bytes/sec)", i, fval);
      }
    }

  return TLV_SIZE (tlvh);
}

u_int16_t
show_vty_link_subtlv_rsc_clsclr (struct cmsh *cmsh, struct te_tlv_header *tlvh)
{
  struct te_link_subtlv_rsc_clsclr *top;

  top = (struct te_link_subtlv_rsc_clsclr *) tlvh;
  if (cmsh != NULL)
  {
    cmdPrint (cmsh, "  Resource class/color: 0x%x", (u_int32_t) ntohl (top->value));
  }
  else
  {
    zlog_debug ("    Resource Class/Color: 0x%x", (u_int32_t) ntohl (top->value));
  }

  return TLV_SIZE (tlvh);
}

static u_int16_t
show_vty_unknown_tlv (struct cmsh *cmsh, struct te_tlv_header *tlvh)
{
  if (cmsh != NULL)
  {
    cmdPrint (cmsh, "  Unknown TLV: [type(0x%x), length(0x%x)]", ntohs (tlvh->type), ntohs (tlvh->length));
  }
  else
  {
    zlog_debug ("    Unknown TLV: [type(0x%x), length(0x%x)]", ntohs (tlvh->type), ntohs (tlvh->length));
  }

  return TLV_SIZE (tlvh);
}

static u_int16_t
ospf_mpls_te_show_link_subtlv (struct cmsh *cmsh, struct te_tlv_header *tlvh0,
                               u_int16_t subtotal, u_int16_t total)
{
  struct te_tlv_header *tlvh, *next;
  u_int16_t sum = subtotal;

  for (tlvh = tlvh0; sum < total; tlvh = (next ? next : TLV_HDR_NEXT (tlvh)))
    {
      next = NULL;
      switch (ntohs (tlvh->type))
        {
        case TE_LINK_SUBTLV_LINK_TYPE:
          sum += show_vty_link_subtlv_link_type (cmsh, tlvh);
          break;
        case TE_LINK_SUBTLV_LINK_ID:
          sum += show_vty_link_subtlv_link_id (cmsh, tlvh);
          break;
        case TE_LINK_SUBTLV_LCLIF_IPADDR:
          sum += show_vty_link_subtlv_lclif_ipaddr (cmsh, tlvh);
          break;
        case TE_LINK_SUBTLV_RMTIF_IPADDR:
          sum += show_vty_link_subtlv_rmtif_ipaddr (cmsh, tlvh);
          break;
        case TE_LINK_SUBTLV_TE_METRIC:
          sum += show_vty_link_subtlv_te_metric (cmsh, tlvh);
          break;
        case TE_LINK_SUBTLV_MAX_BW:
          sum += show_vty_link_subtlv_max_bw (cmsh, tlvh);
          break;
        case TE_LINK_SUBTLV_MAX_RSV_BW:
          sum += show_vty_link_subtlv_max_rsv_bw (cmsh, tlvh);
          break;
        case TE_LINK_SUBTLV_UNRSV_BW:
          sum += show_vty_link_subtlv_unrsv_bw (cmsh, tlvh);
          break;
        case TE_LINK_SUBTLV_RSC_CLSCLR:
          sum += show_vty_link_subtlv_rsc_clsclr (cmsh, tlvh);
          break;
        default:
          sum += show_vty_unknown_tlv (cmsh, tlvh);
          break;
        }
    }
  return sum;
}

static void
ospf_mpls_te_show_info (struct cmsh *cmsh, struct ospf_lsa *lsa)
{
  struct lsa_header *lsah = (struct lsa_header *) lsa->data;
  struct te_tlv_header *tlvh, *next;
  u_int16_t sum, total;
  u_int16_t (* subfunc)(struct cmsh *cmsh, struct te_tlv_header *tlvh,
                        u_int16_t subtotal, u_int16_t total) = NULL;

  sum = 0;
  total = ntohs (lsah->length) - OSPF_LSA_HEADER_SIZE;

  for (tlvh = TLV_HDR_TOP (lsah); sum < total;
			tlvh = (next ? next : TLV_HDR_NEXT (tlvh)))
    {
      if (subfunc != NULL)
        {
          sum = (* subfunc)(cmsh, tlvh, sum, total);
	  next = (struct te_tlv_header *)((char *) tlvh + sum);
          subfunc = NULL;
          continue;
        }

      next = NULL;
      switch (ntohs (tlvh->type))
        {
        case TE_TLV_ROUTER_ADDR:
          sum += show_vty_router_addr (cmsh, tlvh);
          break;
        case TE_TLV_LINK:
          sum += show_vty_link_header (cmsh, tlvh);
	  subfunc = ospf_mpls_te_show_link_subtlv;
	  next = tlvh + 1;
          break;
        default:
          sum += show_vty_unknown_tlv (cmsh, tlvh);
          break;
        }
    }
  return;
}

static void
ospfMplsTeConfigWriteRouter (struct cmsh *cmsh)
{
	if (OspfMplsTE.status == enabled) {
		cmdPrint(cmsh, "  mpls-te");
		cmdPrint(cmsh, "  mpls-te router-address %s",
				inet_ntoa(OspfMplsTE.router_addr.value));
	}
	return;
}

static void
ospfMplsTeConfigWriteIf (struct cmsh *cmsh, struct interface *ifp)
{
	struct mpls_te_link *lp;

	if ((OspfMplsTE.status == enabled)
			&& (!if_is_loopback(ifp) && if_is_up(ifp) && ospf_oi_count(ifp) > 0)
			&& ((lp = lookup_linkparams_by_ifp(ifp)) != NULL)) {
		float fval;
		int i;

		cmdPrint(cmsh, " mpls-te link metric %u",
				(u_int32_t) ntohl(lp->te_metric.value));

		ntohf(&lp->max_bw.value, &fval);
		if (fval >= MPLS_TE_MINIMUM_BANDWIDTH)
			cmdPrint(cmsh, " mpls-te link max-bw %g", fval);

		ntohf(&lp->max_rsv_bw.value, &fval);
		if (fval >= MPLS_TE_MINIMUM_BANDWIDTH)
			cmdPrint(cmsh, " mpls-te link max-rsv-bw %g", fval);

		for (i = 0; i < 8; i++) {
			ntohf(&lp->unrsv_bw.value[i], &fval);
			if (fval >= MPLS_TE_MINIMUM_BANDWIDTH)
				cmdPrint(cmsh, " mpls-te link unrsv-bw %d %g", i, fval);
		}

		cmdPrint(cmsh, " mpls-te link rsc-clsclr 0x%x",
				(u_int32_t) ntohl(lp->rsc_clsclr.value));
	}
	return;
}




#endif /* HAVE_OSPF_TE */
