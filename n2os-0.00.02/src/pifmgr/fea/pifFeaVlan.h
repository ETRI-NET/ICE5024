#ifndef _NSM_FEA_VLAN_H_
#define _NSM_FEA_VLAN_H_

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

#include "hal_l2.h"

/**************************************************************
 *
 *   ZebOS L2 Forwarder Brideg HAL API
 *
 *************************************************************/
#define PIF_HAL_DEFAULT_VID         1

/* Vlan Init/Deinit */
int pif_fea_vlan_init ();
int pif_fea_vlan_deinit ();

/* Vlan add/delete */
int pif_fea_vlan_add (char *name, unsigned short vid);
int pif_fea_vlan_delete (char *name, unsigned short vid);

/* Vlan enable/disable */
int pif_fea_vlan_enable (char *name, unsigned short vid);
int pif_fea_vlan_disable (char *name, unsigned short vid);

/* Vlan default pvid */
 int pif_fea_vlan_set_default_pvid (char *name, unsigned int ifindex,
		                            unsigned short pvid, enum hal_vlan_egress_type egress_tagged);

/* Vlan set port type (access or trunk) */
int pif_fea_vlan_set_port_type (char *name, unsigned int ifindex,
		                        enum hal_vlan_port_type port_type,
								enum hal_vlan_acceptable_frame_type acceptable_frame_types,
								 unsigned short enable_ingress_filter);

/* Vlan add/delete to/from port */
int pif_fea_vlan_add_vid_to_port (char *name, unsigned int ifindex, 
			                      unsigned short vid, enum hal_vlan_egress_type egress);
int pif_fea_vlan_delete_vid_from_port (char *name, unsigned int ifindex, 
		                               unsigned short vid);

/* Vlan set trunck port default vid */
int pif_fea_vlan_set_default_pvid (char *name, unsigned int ifindex,
		                           unsigned short pvid,
								   enum hal_vlan_egress_type egress);

/* Vlan set trunck port native vid */
int pif_fea_vlan_set_native_vid (char *name, unsigned int ifindex,  unsigned short vid);


#endif /* _NSM_FEA_VLAN_H */
