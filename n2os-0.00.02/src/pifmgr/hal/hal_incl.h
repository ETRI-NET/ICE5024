/* Copyright (C) 2004 IP Infusion, Inc. All Rights Reserved. */

#ifndef _HAL_INCL_H_
#define _HAL_INCL_H_

/* Hal config definition */
#define HAVE_L2  		1
#define HAVE_LACPD  	1

/* rewrite pal api */
#define pal_strncpy strncpy
typedef enum
{
	PAL_FALSE = 0,            /* Everybody calls zero false... */
	PAL_TRUE = (!PAL_FALSE)   /* Some want TRUE=1 or TRUE=-1 or TRUE!=0 */
} bool_t;


#include <errno.h>
#include <sys/types.h>
#include <linux/types.h>

#include "hal_types.h"
#include "hal_error.h"
#include "hal.h"
#include "hal_if.h"
#include "hal_acl.h"

#ifdef HAVE_L2
#include "L2/hal_l2.h"
#include "L2/hal_auth.h"
#include "L2/hal_bridge.h"
#include "L2/hal_igmp_snoop.h"
#include "L2/hal_l2_fdb.h"
#include "L2/hal_vlan.h"
#include "L2/hal_socket.h"
#include "L2/hal_garp.h"
#include "L2/hal_lacp.h"
#include "l2swfwdr/if_lacp.h"

#include "hal_comm.h"
#include "hal_ipifwd.h"
#endif /* HAVE_L2 */

#ifdef HAVE_L3
#include "L3/hal_l3.h"
#include "L3/hal_fib.h"
#include "L3/hal_ipv4_arp.h"
#include "L3/hal_ipv4_if.h"
#include "L3/hal_ipv4_uc.h"
#include "L3/hal_ipv4_mc.h"
#ifdef HAVE_IPV6
#include "L3/hal_ipv6_if.h"
#include "L3/hal_ipv6_uc.h"
#include "L3/hal_ipv6_mc.h"
#include "L3/hal_ipv6_nbr.h"
#endif /* HAVE_IPV6 */
#endif /* HAVE_L3 */


#ifdef HAVE_PVLAN
#include "L2/hal_pvlan.h"
#endif /* HAVE_PVLAN */


#endif /* _HAL_INCL_H_ */
