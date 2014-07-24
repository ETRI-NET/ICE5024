#ifndef _NSM_FEA_LACP_H_
#define _NSM_FEA_LACP_H_

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
#include "hal_types.h"

/**************************************************************
 *
 *   ZebOS L2 Forwarder LACP HAL API
 *
 *************************************************************/

/* Lacp Init/Deinit */
int pif_fea_lacp_init ();
int pif_fea_lacp_deinit ();

/* Aggregator add/delete */
int pif_fea_lacp_add_aggregator (char *name, unsigned char mac[], int agg_type );
int pif_fea_lacp_delete_aggregator (char *name);

/* Aggregator add/delete port */
int pif_fea_lacp_attach_mux_to_aggregator (char *name, char *port_name, unsigned int port_ifindex);
int pif_fea_lacp_detach_mux_from_aggregator (char *name, char *port_name, unsigned int port_ifindex);


#endif /* _NSM_FEA_LACP_H */
