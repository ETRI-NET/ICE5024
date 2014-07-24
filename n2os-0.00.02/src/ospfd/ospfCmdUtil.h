/*
 * ospfCmdUtil.h
 *
 *  Created on: 2014. 6. 13.
 *      Author: root
 */

#ifndef OSPFCMDUTIL_H_
#define OSPFCMDUTIL_H_

#include <sys/types.h>
#include "if.h"
#include "prefix.h"
#include "table.h"


#include "ospfd.h"

extern const char *ospf_abr_type_descr_str[];

extern const char *ospf_shortcut_mode_descr_str[];

extern void ospf_passive_interface_default (struct ospf *, u_char );
extern void ospf_passive_interface_update (struct ospf *, struct interface *,
        										struct in_addr , struct ospf_if_params *, u_char );
extern void ospf_vl_config_data_init (struct ospf_vl_config_data *, struct cmsh *);
extern int ospf_vl_set (struct ospf *ospf, struct ospf_vl_config_data *vl_config);
extern int ospf_area_nssa_cmd_handler (struct cmsh *cmsh, int argc, Int8T *argv[],
												int nosum);
extern int ospf_timers_spf_set (struct cmsh *, unsigned int , unsigned int , unsigned int );

extern void show_ip_ospf_area (struct cmsh *, struct ospf_area *);
extern void show_ip_ospf_interface_sub (struct cmsh *, struct ospf *, struct interface *);
extern void show_ip_ospf_neighbour_header (struct cmsh *);
extern void show_ip_ospf_neighbor_sub (struct cmsh *, struct ospf_interface *);
extern void show_ip_ospf_nbr_nbma_detail_sub (struct cmsh *, struct ospf_interface *,
														struct ospf_nbr_nbma *);
extern void show_ip_ospf_neighbor_detail_sub (struct cmsh *, struct ospf_interface *,
				  	  	  	  	  	  	  	  	  	  struct ospf_neighbor *);
extern void show_lsa_detail (struct cmsh *, struct ospf *, int ,
										struct in_addr *, struct in_addr *);
extern void show_lsa_detail_adv_router (struct cmsh *, struct ospf *, int , struct in_addr *);
extern void show_ip_ospf_database_summary (struct cmsh *, struct ospf *, int );
extern void show_ip_ospf_database_maxage (struct cmsh *, struct ospf *);
extern void show_ip_ospf_route_network (struct cmsh *, struct route_table *);
extern void show_ip_ospf_route_router (struct cmsh *, struct route_table *);
extern void show_ip_ospf_route_external (struct cmsh *, struct route_table *);
extern void show_mpls_te_link_sub (struct cmsh *, struct interface *);

extern void ospf_nbr_timer_update (struct ospf_interface *);
extern int ospf_vty_dead_interval_set (struct interface *, struct cmsh *,
												const char *, const char *,const char *);

extern int str2metric (const char *, int *);
extern int str2metric_type (const char *, int *);

#endif /* OSPFCMDUTIL_H_ */
