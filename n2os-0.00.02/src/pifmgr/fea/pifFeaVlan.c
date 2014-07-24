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
#include "pifFeaVlan.h"
#include "pifFeaBridge.h"




/*
   NSM FEA vlan init.
*/
int pif_fea_vlan_init ()
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
#if 0
  /* create default vlan 1 */
  int ret = hal_vlan_add(PIF_HAL_BRIDGE_NAME, 0, 0, PIF_HAL_DEFAULT_VID);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }

  /* enable vlan 1 */
  ret = hal_vlan_enable(PIF_HAL_BRIDGE_NAME, 0, PIF_HAL_DEFAULT_VID);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif

#endif  
  return 0;
}


/*
   NSM FEA vlan deinit.
*/
int pif_fea_vlan_deinit ()
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  /* currently empty  */
#endif  
  return 0;
}


/* 
   Name: pif_fea_vlan_add 

   Description: 
   This API adds a VLAN.

   Parameters:
   IN -> name - bridge name
   IN -> vid - VLAN id

   Returns:
   HAL_ERR_VLAN_EXISTS 
   HAL_SUCCESS 
   
   HAL : int hal_vlan_add (char *name, unsigned short vid);
*/
int pif_fea_vlan_add (char *name, unsigned short vid)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  enum hal_vlan_type type = 0;
  enum hal_evc_type evc_type = 0;

  int ret = hal_vlan_add(name, type, evc_type, vid);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}


/* 
   Name: pif_fea_vlan_delete

   Description:
   This API deletes a VLAN.

   Parameters:
   IN -> name - bridge name
   IN -> vid - VLAN id

   Returns:
   HAL_ERR_VLAN_NOT_EXISTS
   HAL_SUCCESS

   HAL : int hal_vlan_delete (char *name, unsigned short vid);
*/
int pif_fea_vlan_delete (char *name, unsigned short vid)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  enum hal_vlan_type type = 0;

  int ret = hal_vlan_delete(name, type, vid);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}



int pif_fea_vlan_enable (char *name, unsigned short vid)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  enum hal_vlan_type type = 0;
  int ret = hal_vlan_enable(name, type, vid);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}


int pif_fea_vlan_disable (char *name, unsigned short vid)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  enum hal_vlan_type type = 0;
  int ret = hal_vlan_disable(name, type, vid);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}











/* 
   Name: pif_fea_vlan_add_vid_to_port

   Description:
   This API adds a VLAN to a port.

   Parameters:
   IN -> name - bridge name
   IN -> ifindex - interface index
   IN -> vid - VLAN id
   IN -> egress - egress tagged/untagged

   Returns:
   HAL_ERR_VLAN_NOT_EXISTS
   HAL_ERR_VLAN_PORT_NOT_EXISTS
   HAL_SUCCESS

   HAL : int hal_vlan_add_vid_to_port (char *name, unsigned int ifindex,
			  unsigned short vid, 
			  enum hal_vlan_egress_type egress);
*/
int pif_fea_vlan_add_vid_to_port (char *name, 
								  unsigned int ifindex,
								  unsigned short vid,
								  enum hal_vlan_egress_type egress)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_vlan_add_vid_to_port(name, ifindex, vid, egress);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}

/* 
   Name: pif_fea_vlan_delete_vid_from_port

   Description:
   This API deletes a VLAN from a port.

   Parameters:
   IN -> name - bridge name
   IN -> ifindex - interface index
   IN -> vid - VLAN id

   Returns:
   HAL_ERR_VLAN_PORT_NOT_EXISTS
   HAL_ERR_VLAN_NOT_EXISTS
   HAL_SUCCESS

   HAL : int hal_vlan_delete_vid_from_port (char *name, unsigned int ifindex,
			       unsigned short vid);
*/
int pif_fea_vlan_delete_vid_from_port (char *name, unsigned int ifindex, unsigned short vid)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_vlan_delete_vid_from_port(name, ifindex, vid);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}



/* 
   Name: pif_fea_vlan_set_port_type

   Description:
   This API sets the acceptable frame type for a port.

   Parameters:
   IN -> name - bridge name
   IN -> ifindex - interface index
   IN -> port_type - trunk, access or hybrid
   IN -> acceptable_frame_type - acceptable frame type
   IN -> enable_ingress_filter - enable ingress filtering

   Returns:
   HAL_ERR_VLAN_FRAME_TYPE
   HAL_SUCCESS

   HAL : int hal_vlan_set_port_type (char *name, 
			unsigned int ifindex, 
			enum hal_vlan_port_type port_type,
			enum hal_vlan_acceptable_frame_type acceptable_frame_types,
			unsigned short enable_ingress_filter);
*/
int pif_fea_vlan_set_port_type (char *name, 
		                        unsigned int ifindex,
								enum hal_vlan_port_type port_type,
								enum hal_vlan_acceptable_frame_type acceptable_frame_types,
								 unsigned short enable_ingress_filter)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  enum hal_vlan_port_type sub_port_type = port_type;
  int ret = hal_vlan_set_port_type(name, ifindex, port_type, sub_port_type, 
		                           acceptable_frame_types,
		 	                       enable_ingress_filter);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}




/* 
   Name: pif_fea_vlan_set_default_pvid

   Description:
   This API sets the default PVID for a port.

   Parameters:
   IN -> name - bridge name
   IN -> ifindex - interface index
   IN -> pvid - default PVID
   IN -> egress - egress tagged/untagged

   Returns:
   HAL_ERR_VLAN_NOT_EXISTS
   HAL_SUCCESS

   HAL : int hal_vlan_set_default_pvid (char *name, unsigned int ifindex,
			   unsigned short pvid, 
			   enum hal_vlan_egress_type egress);
*/
int pif_fea_vlan_set_default_pvid (char *name, 
		                        unsigned int ifindex,
								unsigned short pvid,
								enum hal_vlan_egress_type egress)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_vlan_set_default_pvid(name, ifindex, pvid, egress); 
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}



/*
   Name: pif_fea_vlan_set_native_vid

   Description:
   This API sets the native VID for a port.

   Parameters:
   IN -> name - bridge name
   IN -> ifindex - interface index
   IN -> vid - native VID

   Returns:
   HAL_ERR_VLAN_NOT_EXISTS
   HAL_SUCCESS

   HAL : int hal_vlan_set_native_vid (char *name, unsigned int ifindex,
                         unsigned short vid);
*/
int pif_fea_vlan_set_native_vid (char *name, 
		                        unsigned int ifindex,
								unsigned short vid)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_vlan_set_native_vid(name, ifindex, vid); 
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}



