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
  FEA - Forwarding plane APIs.
*/

#include <stdio.h>
#include "pal.h"
#include "hal_incl.h"

#include "pif.h"
#include "pifFeaApi.h"



/*
   NSM FEA lacp init.
*/
int pif_fea_lacp_init ()
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  /* currently empty  */
#endif  
  return 0;
}


/*
   NSM FEA lacp deinit.
*/
int pif_fea_lacp_deinit ()
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  /* currently empty  */
#endif  
  return 0;
}


/* 
   Name: hal_lacp_add_aggregator

   Description:
   This API adds a aggregator with the specified name and mac address.

   Parameters:
   IN -> name - aggregator interface name
   IN -> mac  - mac address of aggregator
   IN -> agg_type - aggregator type (L2/L3)

   Returns:
   HAL_ERR_LACP_EXISTS
   HAL_SUCCESS

   HAL: int hal_lacp_add_aggregator (char *name, unsigned char mac[], int agg_type);
*/
int pif_fea_lacp_add_aggregator (char *name, unsigned char mac[], int agg_type )
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_lacp_add_aggregator(name, mac, agg_type);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail(%d) to set HAL interface api \n", ret);
  }
#endif  
  return 0;
}




/* 
   Name: hal_lacp_delete_aggregator

   Description:
   This API deletes a aggregator.

   Parameters:
   IN -> name - aggregator interface name
   IN -> ifindex - aggregator ifindex

   Returns:
   HAL_ERR_LACP_NOT_EXISTS
   HAL_SUCCESS

   HAL: int hal_lacp_delete_aggregator (char *name, unsigned int ifindex);
*/
int pif_fea_lacp_delete_aggregator (char *name)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_lacp_delete_aggregator(name, 0);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail(%d) to set HAL interface api \n", ret);
  }
#endif  
  return 0;
}




/* 
   Name: hal_lacp_attach_mux_to_aggregator

   Description:
   This API adds a port to a aggregator.

   Parameters:
   IN -> agg_name - aggregator name
   IN -> agg_ifindex - aggregator ifindex
   IN -> port_name - port name
   IN -> port_ifindex - port ifindex

   Returns:
   < 0 on error
   HAL_SUCCESS

   HAL: int hal_lacp_attach_mux_to_aggregator (char *agg_name, unsigned int agg_ifindex, 
				   char *port_name, unsigned int port_ifindex);
*/
int pif_fea_lacp_attach_mux_to_aggregator (char *name, char *port_name, 
											unsigned int port_ifindex)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_lacp_attach_mux_to_aggregator(name, 0, port_name, port_ifindex);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail(%d) to set HAL interface api \n", ret);
  }
#endif  
  return 0;
}



/* 
   Name: hal_lacp_detach_mux_from_aggregator

   Description:
   This API deletes a port from a aggregator.

   Parameters:
   IN -> agg_name - aggregator name
   IN -> agg_ifindex - aggregator ifindex
   IN -> port_name - port name
   IN -> port_ifindex - port ifindex

   Returns:
   < 0 on error
   HAL_SUCCESS
   HAL: int hal_lacp_detach_mux_from_aggregator (char *agg_name, unsigned int agg_ifindex, 
				     char *port_name, unsigned int port_ifindex);
*/

int pif_fea_lacp_detach_mux_from_aggregator (char *name, char *port_name, 
											unsigned int port_ifindex)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_lacp_detach_mux_from_aggregator(name, 0, port_name, port_ifindex);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail(%d) to set HAL interface api \n", ret);
  }
#endif  
  return 0;
}






