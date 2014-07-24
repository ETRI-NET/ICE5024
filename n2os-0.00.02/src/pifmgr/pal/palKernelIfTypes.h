/* Copyright (C) 2002-2003 IP Infusion, Inc.  All Rights Reserved.  */

#ifndef _PAL_IF_TYPES_DEF
#define _PAL_IF_TYPES_DEF

/* This file defines PAL interface type definitions.  */

#define IF_TYPE_UNKNOWN			0
#define IF_TYPE_LOOPBACK		1
#define IF_TYPE_ETHERNET		2
#define IF_TYPE_HDLC			3
#define IF_TYPE_PPP		    	4
#define IF_TYPE_ATM		    	5
#define IF_TYPE_FRELAY			6
#define IF_TYPE_VLAN            7
#define IF_TYPE_PORT            8
#define IF_TYPE_AGGREGATE       9
#define IF_TYPE_MANAGE          10

#define IF_TYPE_IPIP			11
#define IF_TYPE_GREIP			12
#define IF_TYPE_IPV6IP			13
#define IF_TYPE_6TO4			14
#define IF_TYPE_ISATAP			15

#define IS_IF_TYPE(I, T)	((I)->hw_type == IF_TYPE_ ## T)

/* Ethernet hardware address length.  */
#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN		6
#endif /* !ETHER_ADDR_LEN */

/* Logical Link Control(LLC) header length.  */
#ifndef LLC_HDR_LEN
#define LLC_HDR_LEN		3
#endif /* !LLC_HDR_LEN */

/* HDLC header length.  */
#ifndef CHDLC_HDR_LEN
#define CHDLC_HDR_LEN		4
#endif /* !CHDLC_HDR_LEN */

#ifndef ETHER_ADDR_SAME
#define ETHER_ADDR_SAME(A, B)                                                 \
    (pal_mem_cmp ((A), (B), ETHER_ADDR_LEN) == 0)
#endif /* !ETHER_ADDR_SAME */

#ifndef ETHER_ADDR_CMP
#define ETHER_ADDR_CMP(A, B)                                                  \
    pal_mem_cmp ((A), (B), ETHER_ADDR_LEN)
#endif /* !ETHER_ADDR_CMP */

#ifndef ETHER_ADDR_COPY
#define ETHER_ADDR_COPY(D, S)                                                 \
    pal_mem_cpy (D, S, ETHER_ADDR_LEN)
#endif /* !ETHER_ADDR_COPY */

/* Prototypes.  */
unsigned short pal_if_type (int);

#endif /* _PAL_IF_TYPES_DEF */
