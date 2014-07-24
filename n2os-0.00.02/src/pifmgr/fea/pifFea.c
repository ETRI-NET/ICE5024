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

/*
  NSM - Forwarding plane APIs.
*/

#include <stdio.h>
#include "pal.h"
#include "hal_incl.h"

#include "pif.h"
#include "pifFeaApi.h"

#include "pifFea.h"


/*
   NSM FEA init.
*/
int
pif_fea_init ()
{
  PAL_DBUG(0, "enter\n");

  /* PAL Interface data structure initialize */
  ifInit();

  /* Netlink init & scan linux interface */
  kernelStart ();

#ifdef ENABLE_HAL_PATH
  /* HAL socket init */
  hal_init ();
#endif

  /* init L2 bridge */
  pif_fea_bridge_init();

  /* init vlan */
  pif_fea_vlan_init();

  /* init lacp */
  pif_fea_lacp_init();

  PAL_DBUG(0, "end");
  return 0;
}

/*
   NSM FEA deinit.
*/
int
pif_fea_deinit ()
{
  PAL_DBUG(0, "enter\n");

  /* PAL Netlink stop */
  kernelStop ();

#ifdef ENABLE_HAL_PATH
  hal_deinit ();
#endif

  /* deinit L2 bridge */
  pif_fea_bridge_deinit();

  /* deinit vlan */
  pif_fea_vlan_deinit();

  /* deinit lacp */
  pif_fea_lacp_deinit();

  PAL_DBUG(0, "end");
  return 0;
}

