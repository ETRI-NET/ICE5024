#ifndef _NSM_FEA_H_
#define _NSM_FEA_H_
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

/* Enable PAL/HAL api */
#define ENABLE_HAL_PATH     1
#define ENABLE_PAL_PATH     1
#define HAVE_L3             1

#include "pifFeaInterface.h"
#include "pifFeaBridge.h"
#include "pifFeaVlan.h"
#include "pifFeaLacp.h"

/* Fea Init/Deinit */
int pif_fea_init ();
int pif_fea_deinit ();
int pif_fea_if_update (void);


#endif /* _NSM_FEA_H */
