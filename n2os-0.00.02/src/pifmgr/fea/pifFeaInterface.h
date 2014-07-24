#ifndef _NSM_FEA__IF_H_
#define _NSM_FEA__IF_H_
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

/**************************************************************
 *
 *    Forwarding plane APIs.
 *
 *************************************************************/

/* Interface scan update */
int pif_fea_if_update (void);

/* Interface configuration */
int pif_fea_if_flags_set (InterfaceT *ifp, u_int32_t flags);
int pif_fea_if_flags_unset (InterfaceT *ifp, u_int32_t flags);
int pif_fea_if_flags_get (InterfaceT *ifp);
int pif_fea_if_set_mtu (InterfaceT *ifp, int mtu);
int pif_fea_if_set_duplex (InterfaceT *ifp, int duplex);
int pif_fea_if_set_autonego (InterfaceT *ifp, int autonego);
int pif_fea_if_get_bandwidth (InterfaceT *ifp);
int pif_fea_if_set_bandwidth (InterfaceT *ifp, float bandwidth);
int pif_if_proxy_arp_set (InterfaceT *ifp, int proxy_arp);
int pif_fea_if_set_arp_ageing_timeout (InterfaceT *ifp, int arp_ageing_timeout); 
int pif_fea_if_set_port_type (InterfaceT *ifp, int type);

/* Interface Address configuration */
int pif_fea_if_ipv4_address_add (InterfaceT *ifp, struct connected *ifc);
int pif_fea_if_ipv4_address_delete (InterfaceT *ifp, struct connected *ifc);
int pif_fea_if_ipv4_address_delete_all (InterfaceT *ifp, struct connected *ifc);
int pif_fea_if_ipv4_address_update (InterfaceT *ifp, struct connected *ifc_old,
				    struct connected *ifc_new);





#ifdef HAVE_IPV6
int pif_fea_ipv6_forwarding_get (int *ipforward);
int pif_fea_ipv6_forwarding_set (int ipforward);
int pif_fea_if_ipv6_address_add (InterfaceT *ifp, struct connected *ifc);
int pif_fea_if_ipv6_address_delete (InterfaceT *ifp, struct connected *ifc);
#endif /* HAVE_IPV6 */

/**************************************************************
 *
 *    OS Forwarding plane Events.
 *
 *************************************************************/
void pif_fea_if_add(InterfaceT *ifp);
void pif_fea_if_del(InterfaceT *ifp);

void pif_fea_if_up(InterfaceT *ifp);
void pif_fea_if_down(InterfaceT *ifp);

void pif_fea_if_addr_add(InterfaceT *ifp, ConnectedT *ifc);
void pif_fea_if_addr_del(InterfaceT *ifp, ConnectedT *ifc);

#endif /* _NSM_FEA_IF_H */
