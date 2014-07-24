#ifndef _NSM_FEA_BR_H_
#define _NSM_FEA_BR_H_

/*345678901234567890123456789012345678901234567890123456789012345678901234567890*/
/*********************************************************************************
 *               Electronics and Telecommunications Research Institute
 * Filename : <myFileName>
 * Blockname: <PIF Manager>
 * Overview : <PIF Manager S/W block manages Port/Interface & L2 MAC/VLAN>
 * Creator  : <Seungwoo Hong>
 * Owner    : <Seungwoo Hong>
 * Copyright: 2013 Electronics and Telecommunications Research Institute. 
 *            All rights reserved. No part of this software shall be reproduced, 
 *            stored in a retrieval system, or transmitted by any means, 
 *            electronic, mechanical, photocopying, recording, or otherwise, 
 *            without written permission from ETRI.
 *********************************************************************************/
/*********************************************************************************
 *                        VERSION CONTROL  INFORMATION
 * $Author$
 * $Date$
 * $Revision
 * $Log$ 
 *********************************************************************************/

#include "interface.h"

#define PIF_HAL_BRIDGE_NAME			"0"
#define PIF_HAL_BRIDGE_VLAN_AWARE	1

/* HAL_PROTO_STP/HAL_PROTO_RSTP/HAL_PROTO_MSTP */
#define PIF_PROTOCOL_STP		HAL_PROTO_STP
#define PIF_PROTOCOL_RSTP		HAL_PROTO_RSTP
#define PIF_PROTOCOL_MSTP		HAL_PROTO_MSTP

#define PIF_AGEING_DEFAULT		300


/**************************************************************
 *
 *   ZebOS L2 Forwarder Brideg HAL API
 *
 *************************************************************/

/* Bridge Init/Deinit */
int pif_fea_bridge_init ();
int pif_fea_bridge_deinit ();

/* Bridge set vlan port type */

/* Bridge add/delete */
int pif_fea_bridge_add (char *name, unsigned int is_vlan_aware, int protocol);
int pif_fea_bridge_delete (char *name);

/* Bridge port add/delete */
int pif_fea_bridge_add_port (char *name, unsigned int ifindex);
int pif_fea_bridge_delete_port (char *name, unsigned int ifindex);

/* Bridge change protocol (STP/RSTP/MSTP) */
int pif_fea_bridge_change_protocol (char *name, int protocol);

/* Bridge port state (for STP) */
int pif_fea_bridge_set_port_state (char *name, int ifindex, int instance, int state);

/* Bridge instance & vlan add/delete (for MSTP) */
int pif_fea_bridge_add_instance (char *name, int instance);
int pif_fea_bridge_delete_instance (char *name, int instance);
int pif_fea_bridge_add_vlan_to_instance (char *name, int instance, unsigned short vid);
int pif_fea_bridge_add_delete_from_instance (char *name, int instance, unsigned short vid);

/* Bridge learn and forward */
int pif_fea_bridge_set_learn_fwd (char *name, int ifindex, int instance, int learn, int forward);

/* Bridge ageing time/mac learning */
int pif_fea_bridge_set_ageing_time (char *name, u_int32_t ageing_time);
//int pif_fea_bridge_disable_ageing (char *name);
int pif_fea_bridge_set_set_learning (char *name, int learning);

/* mac address table */
int pif_fea_bridge_add_mac_static (char *name, char* mac, int ifindex, unsigned short vid);
int pif_fea_bridge_del_mac_static (char *name, char* mac, int ifindex, unsigned short vid);

int pif_fea_bridge_add_mac_dynamic (char *name, char* mac, int ifindex, unsigned short vid);
int pif_fea_bridge_del_mac_dynamic (char *name, char* mac, int ifindex, unsigned short vid);

int pif_fea_bridge_clear_mac_dynamic (char *name, int ifindex, unsigned short vid);
int pif_fea_bridge_clear_mac_dynamic_by_mac (char *name, char* mac);






#endif /* _NSM_FEA_BR_H */
