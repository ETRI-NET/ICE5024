/*
 * IS-IS Rout(e)ing protocol - isisd.h   
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

#ifndef ISISD_H
#define ISISD_H

#define ISISD_VERSION "0.0.7"

#include <sys/types.h>
#include <stdint.h>

#include "route_types.h"

#include "nnList.h"
#include "nnRibDefines.h"

#include "isis_constants.h"
#include "dict.h"
#include "isis_flags.h"
#include "isis_common.h"



/* uncomment if you are a developer in bug hunt */
/* #define EXTREME_DEBUG  */
/* #define EXTREME_TLV_DEBUG */

struct rmap
{
  char *name;
  struct route_map *map;
};

struct isis
{
	/* Command related pointer. */
	void * pCmshGlobal;

	u_long process_id;
	int sysid_set;
	u_char sysid[ISIS_SYS_ID_LEN]; /* SystemID for this IS */
	u_int32_t router_id; /* Router ID from zebra */
	struct list *area_list; /* list of IS-IS areas */
	struct list *init_circ_list;
	struct list *nexthops; /* IPv4 next hops from this IS */
#ifdef HAVE_IPV6
	struct list *nexthops6; /* IPv6 next hops from this IS */
#endif				/* HAVE_IPV6 */
	u_char max_area_addrs; /* maximumAreaAdresses */
	struct area_addr *man_area_addrs; /* manualAreaAddresses */
	u_int32_t debugs; /* bitmap for debug */
	time_t uptime; /* when did we start */

	void *pTimerDyncClean; /* dynamic hostname cache cleanup timer */

	/* Redistributed external information. */
	struct route_table *external_info[RIB_ROUTE_TYPE_MAX + 1];
	/* Redistribute metric info. */
	struct {
		int type; /* Internal or External  */
		int value; /* metric value */
	} dmetric[RIB_ROUTE_TYPE_MAX + 1];

	struct {
		char *name;
		struct route_map *map;
	} rmap[RIB_ROUTE_TYPE_MAX + 1];
#ifdef HAVE_IPV6
	struct
	{
		struct
		{
			char *name;
			struct route_map *map;
		}rmap[RIB_ROUTE_TYPE_MAX + 1];
	}inet6_afmode;
#endif

	/*  List for interface list.  */
	ListT * pIfList;

	/* ISIS redistribute information. */
	Uint8T isisRedistributeDefault;
	Uint8T isisRedistribute[RIB_ROUTE_TYPE_MAX];
	Uint8T isisDefaultInformation;
};
typedef struct isis IsisT;

extern struct isis *isis;

struct isis_area
{
	struct isis *isis; /* back pointer */
	dict_t *lspdb[ISIS_LEVELS]; /* link-state dbs */
	struct isis_spftree *spftree[ISIS_LEVELS]; /* The v4 SPTs */
	struct route_table *route_table[ISIS_LEVELS]; /* IPv4 routes */

#ifdef HAVE_IPV6
	struct isis_spftree *spftree6[ISIS_LEVELS]; /* The v6 SPTs */
	struct route_table *route_table6[ISIS_LEVELS]; /* IPv6 routes */
#endif

	unsigned int min_bcast_mtu;
	struct list *circuit_list; /* IS-IS circuits */
	struct flags flags;

	void *pTimerTick;

	void *pTimerLspRefresh[ISIS_LEVELS];
	int lsp_regenerate_pending[ISIS_LEVELS];

	/*
	 * Configurables
	 */
	struct isis_passwd area_passwd;
	struct isis_passwd domain_passwd;
	/* do we support dynamic hostnames?  */
	char dynhostname;
	/* do we support new style metrics?  */
	char newmetric;
	char oldmetric;
	/* identifies the routing instance   */
	char *area_tag;
	/* area addresses for this area      */
	struct list *area_addrs;
	u_int16_t max_lsp_lifetime[ISIS_LEVELS];
	char is_type; /* level-1 level-1-2 or level-2-only */
	/* are we overloaded? */
	char overload_bit;
	u_int16_t lsp_refresh[ISIS_LEVELS];
	/* minimum time allowed before lsp retransmission */
	u_int16_t lsp_gen_interval[ISIS_LEVELS];
	/* min interval between between consequtive SPFs */
	u_int16_t min_spf_interval[ISIS_LEVELS];
	/* the percentage of LSP mtu size used, before generating a new frag */
	int lsp_frag_threshold;
	int ip_circuits;
	/* logging adjacency changes? */
	u_char log_adj_changes;

#ifdef HAVE_IPV6
	int ipv6_circuits;
#endif

	/* Counters */
	u_int32_t circuit_state_changes;

#ifdef TOPOLOGY_GENERATE
	struct list *topology;
	u_char topology_baseis[ISIS_SYS_ID_LEN]; /* IS for the first IS emulated. */
	char *topology_basedynh; /* Dynamic hostname base. */
	char top_params[200]; /* FIXME: what is reasonable? */
#endif /* TOPOLOGY_GENERATE */
};



/*
 * Prototypes.
 */
extern void isisInit (void);
extern void isis_new(unsigned long);
extern void isisVersionUpdate(void);
extern struct isis_area *isis_area_create(const char *);
extern struct isis_area *isis_area_lookup (const char *);
extern Int32T configWriteIsis(struct cmsh *);
extern struct isis_area *isisAreaGet(const char *);
extern Int32T showIsisInterfaceCommon(struct cmsh *, const char *ifname, char);
extern Int32T showIsisNeighborCommon(struct cmsh *, const char *id, char);
extern Int32T clearIsisNeighborCommon(struct cmsh *, const char *id);
extern Int32T isisAreaDestroy(struct cmsh *, const char *);
extern Int32T areaNetTitle(struct cmsh *, const char *, const char *);
extern Int32T areaClearNetTitle (struct cmsh *, const char *, const char *);


extern void cmdPrintTimestr(Int8T *, Int32T *, time_t);
extern Int32T showIsisDatabase (struct cmsh *, const char *, int);
extern Int32T setLspGenInterval (struct cmsh *, struct isis_area *, uint16_t , int );
extern Int32T validateMetricStyleNarrow (struct cmsh *, struct isis_area *);
extern Int32T setLspMaxLifetime (struct cmsh *, struct isis_area *, uint16_t , int );
extern Int32T setLspRefreshInterval (struct cmsh *, struct isis_area *, uint16_t , int );


#define DEBUG_ADJ_PACKETS                (1<<0)
#define DEBUG_CHECKSUM_ERRORS            (1<<1)
#define DEBUG_LOCAL_UPDATES              (1<<2)
#define DEBUG_PROTOCOL_ERRORS            (1<<3)
#define DEBUG_SNP_PACKETS                (1<<4)
#define DEBUG_UPDATE_PACKETS             (1<<5)
#define DEBUG_SPF_EVENTS                 (1<<6)
#define DEBUG_SPF_STATS                  (1<<7)
#define DEBUG_SPF_TRIGGERS               (1<<8)
#define DEBUG_RTE_EVENTS                 (1<<9)
#define DEBUG_EVENTS                     (1<<10)
#define DEBUG_ZEBRA                      (1<<11)
#define DEBUG_PACKET_DUMP                (1<<12)

#endif /* ISISD_H */
