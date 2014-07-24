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

#ifndef _ZEBRA_OSPF_MPLS_TE_H
#define _ZEBRA_OSPF_MPLS_TE_H

extern struct ospf_mpls_te OspfMplsTE;

/*
 * Opaque LSA's link state ID for Traffic Engineering is
 * structured as follows.
 *
 *        24       16        8        0
 * +--------+--------+--------+--------+
 * |    1   |  MBZ   |........|........|
 * +--------+--------+--------+--------+
 * |<-Type->|<Resv'd>|<-- Instance --->|
 *
 *
 * Type:      IANA has assigned '1' for Traffic Engineering.
 * MBZ:       Reserved, must be set to zero.
 * Instance:  User may select an arbitrary 16-bit value.
 *
 */

#define	MAX_LEGAL_TE_INSTANCE_NUM (0xffff)

/*
 *        24       16        8        0
 * +--------+--------+--------+--------+ ---
 * |   LS age        |Options |   10   |  A
 * +--------+--------+--------+--------+  |
 * |    1   |   0    |    Instance     |  |
 * +--------+--------+--------+--------+  |
 * |        Advertising router         |  |  Standard (Opaque) LSA header;
 * +--------+--------+--------+--------+  |  Only type-10 is used.
 * |        LS sequence number         |  |
 * +--------+--------+--------+--------+  |
 * |   LS checksum   |     Length      |  V
 * +--------+--------+--------+--------+ ---
 * |      Type       |     Length      |  A
 * +--------+--------+--------+--------+  |  TLV part for TE; Values might be
 * |              Values ...           |  V  structured as a set of sub-TLVs.
 * +--------+--------+--------+--------+ ---
 */

/*
 * Following section defines TLV (tag, length, value) structures,
 * used for Traffic Engineering.
 */
struct te_tlv_header
{
  u_int16_t	type;			/* TE_TLV_XXX (see below) */
  u_int16_t	length;			/* Value portion only, in octets */
};

#define TLV_HDR_SIZE \
	(sizeof (struct te_tlv_header))

#define TLV_BODY_SIZE(tlvh) \
	(ROUNDUP (ntohs ((tlvh)->length), sizeof (u_int32_t)))

#define TLV_SIZE(tlvh) \
	(TLV_HDR_SIZE + TLV_BODY_SIZE(tlvh))

#define TLV_HDR_TOP(lsah) \
	(struct te_tlv_header *)((char *)(lsah) + OSPF_LSA_HEADER_SIZE)

#define TLV_HDR_NEXT(tlvh) \
	(struct te_tlv_header *)((char *)(tlvh) + TLV_SIZE(tlvh))

/*
 * Following section defines TLV body parts.
 */
/* Router Address TLV *//* Mandatory */
#define	TE_TLV_ROUTER_ADDR		1
struct te_tlv_router_addr
{
  struct te_tlv_header	header;		/* Value length is 4 octets. */
  struct in_addr	value;
};

/* Link TLV */
#define	TE_TLV_LINK			2
struct te_tlv_link
{
  struct te_tlv_header	header;
  /* A set of link-sub-TLVs will follow. */
};

/* Link Type Sub-TLV *//* Mandatory */
#define	TE_LINK_SUBTLV_LINK_TYPE		1
struct te_link_subtlv_link_type
{
  struct te_tlv_header	header;		/* Value length is 1 octet. */
  struct {
#define	LINK_TYPE_SUBTLV_VALUE_PTP	1
#define	LINK_TYPE_SUBTLV_VALUE_MA	2
      u_char	value;
      u_char	padding[3];
  } link_type;
};

/* Link Sub-TLV: Link ID *//* Mandatory */
#define	TE_LINK_SUBTLV_LINK_ID			2
struct te_link_subtlv_link_id
{
  struct te_tlv_header	header;		/* Value length is 4 octets. */
  struct in_addr	value;		/* Same as router-lsa's link-id. */
};

/* Link Sub-TLV: Local Interface IP Address *//* Optional */
#define	TE_LINK_SUBTLV_LCLIF_IPADDR		3
struct te_link_subtlv_lclif_ipaddr
{
  struct te_tlv_header	header;		/* Value length is 4 x N octets. */
  struct in_addr	value[1];	/* Local IP address(es). */
};

/* Link Sub-TLV: Remote Interface IP Address *//* Optional */
#define	TE_LINK_SUBTLV_RMTIF_IPADDR		4
struct te_link_subtlv_rmtif_ipaddr
{
  struct te_tlv_header	header;		/* Value length is 4 x N octets. */
  struct in_addr	value[1];	/* Neighbor's IP address(es). */
};

/* Link Sub-TLV: Traffic Engineering Metric *//* Optional */
#define	TE_LINK_SUBTLV_TE_METRIC		5
struct te_link_subtlv_te_metric
{
  struct te_tlv_header	header;		/* Value length is 4 octets. */
  u_int32_t		value;		/* Link metric for TE purpose. */
};

/* Link Sub-TLV: Maximum Bandwidth *//* Optional */
#define	TE_LINK_SUBTLV_MAX_BW			6
struct te_link_subtlv_max_bw
{
  struct te_tlv_header	header;		/* Value length is 4 octets. */
  float			value;		/* bytes/sec */
};

/* Link Sub-TLV: Maximum Reservable Bandwidth *//* Optional */
#define	TE_LINK_SUBTLV_MAX_RSV_BW		7
struct te_link_subtlv_max_rsv_bw
{
  struct te_tlv_header	header;		/* Value length is 4 octets. */
  float			value;		/* bytes/sec */
};

/* Link Sub-TLV: Unreserved Bandwidth *//* Optional */
#define	TE_LINK_SUBTLV_UNRSV_BW			8
struct te_link_subtlv_unrsv_bw
{
  struct te_tlv_header	header;		/* Value length is 32 octets. */
  float			value[8];	/* One for each priority level. */
};

/* Link Sub-TLV: Resource Class/Color *//* Optional */
#define	TE_LINK_SUBTLV_RSC_CLSCLR		9
struct te_link_subtlv_rsc_clsclr
{
  struct te_tlv_header	header;		/* Value length is 4 octets. */
  u_int32_t		value;		/* Admin. group membership. */
};

/* Here are "non-official" architechtual constants. */
#define MPLS_TE_MINIMUM_BANDWIDTH	1.0	/* Reasonable? *//* XXX */

enum sched_opcode {
  REORIGINATE_PER_AREA, REFRESH_THIS_LSA, FLUSH_THIS_LSA
};

struct mpls_te_link
{
  /*
   * According to MPLS-TE (draft) specification, 24-bit Opaque-ID field
   * is subdivided into 8-bit "unused" field and 16-bit "instance" field.
   * In this implementation, each Link-TLV has its own instance.
   */
  u_int32_t instance;

  /* Reference pointer to a Zebra-interface. */
  struct interface *ifp;

  /* Area info in which this MPLS-TE link belongs to. */
  struct ospf_area *area;

  /* Flags to manage this link parameters. */
  u_int32_t flags;
#define LPFLG_LOOKUP_DONE		0x1
#define LPFLG_LSA_ENGAGED		0x2
#define LPFLG_LSA_FORCED_REFRESH	0x4

  /* Store Link-TLV in network byte order. */
  struct te_tlv_link link_header;
  struct te_link_subtlv_link_type link_type;
  struct te_link_subtlv_link_id link_id;
  struct te_link_subtlv_lclif_ipaddr *lclif_ipaddr;
  struct te_link_subtlv_rmtif_ipaddr *rmtif_ipaddr;
  struct te_link_subtlv_te_metric te_metric;
  struct te_link_subtlv_max_bw max_bw;
  struct te_link_subtlv_max_rsv_bw max_rsv_bw;
  struct te_link_subtlv_unrsv_bw unrsv_bw;
  struct te_link_subtlv_rsc_clsclr rsc_clsclr;
};

/* Following structure are internal use only. */
struct ospf_mpls_te
{
  enum { disabled, enabled } status;

  /* List elements are zebra-interfaces (ifp), not ospf-interfaces (oi). */
  struct list *iflist;

  /* Store Router-TLV in network byte order. */
  struct te_tlv_router_addr router_addr;
};

/* Prototypes. */
extern int ospf_mpls_te_init (void);
extern void ospf_mpls_te_term (void);

extern u_int16_t show_vty_link_subtlv_rsc_clsclr (struct cmsh *, struct te_tlv_header *);
extern u_int16_t show_vty_link_subtlv_unrsv_bw (struct cmsh *, struct te_tlv_header *);
extern u_int16_t show_vty_link_subtlv_max_rsv_bw (struct cmsh *, struct te_tlv_header *);
extern u_int16_t show_vty_link_subtlv_max_bw (struct cmsh *, struct te_tlv_header *);
extern u_int16_t show_vty_link_subtlv_te_metric (struct cmsh *, struct te_tlv_header *);
extern u_int16_t show_vty_link_subtlv_rmtif_ipaddr (struct cmsh *, struct te_tlv_header *);
extern u_int16_t show_vty_link_subtlv_lclif_ipaddr (struct cmsh *, struct te_tlv_header *);
extern u_int16_t show_vty_link_subtlv_link_id (struct cmsh *, struct te_tlv_header *);
extern u_int16_t show_vty_link_subtlv_link_type (struct cmsh *, struct te_tlv_header *);
extern u_int16_t show_vty_router_addr (struct cmsh *, struct te_tlv_header *);

extern struct mpls_te_link *lookup_linkparams_by_ifp (struct interface *);
extern void ospf_mpls_te_lsa_schedule (struct mpls_te_link *, enum sched_opcode );
extern void initialize_linkparams (struct mpls_te_link *);
extern void ospf_mpls_te_foreach_area (void (*func)(struct mpls_te_link *,
												enum sched_opcode), enum sched_opcode );
extern void set_mpls_te_router_addr (struct in_addr );
extern void set_linkparams_te_metric (struct mpls_te_link *, u_int32_t );
extern void set_linkparams_max_bw (struct mpls_te_link *, float *);
extern void set_linkparams_max_rsv_bw (struct mpls_te_link *, float *);
extern void set_linkparams_unrsv_bw (struct mpls_te_link *, int , float *);
extern void set_linkparams_rsc_clsclr (struct mpls_te_link *, u_int32_t );

#endif /* _ZEBRA_OSPF_MPLS_TE_H */
