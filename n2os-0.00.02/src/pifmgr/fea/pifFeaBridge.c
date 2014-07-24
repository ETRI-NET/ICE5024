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

#include "pifFeaBridge.h"
/*
   NSM FEA bridge init.
*/
int pif_fea_bridge_init ()
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  /* 
   * create default bridge 
   */
  /*
  int ret = hal_bridge_add(PIF_HAL_BRIDGE_NAME, 
		  				   PIF_HAL_BRIDGE_VLAN_AWARE, 
						   PIF_PROTOCOL_MSTP,
						   0, 0,0 NULL);
  */
  int ret = hal_bridge_add(PIF_HAL_BRIDGE_NAME, 
		  				   PIF_HAL_BRIDGE_VLAN_AWARE, 
						   PIF_PROTOCOL_RSTP,
						   0, 0, NULL);

  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail(%d) to create default bridge \n", ret);
  }

  /* 
   * set default ageing time
   */
  /*
  ret = pif_fea_bridge_set_ageing_time(PIF_HAL_BRIDGE_NAME, PIF_AGEING_DEFAULT);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail(%d) to set bridge ageing time \n", ret);
  }
  */
#endif  
  return 0;
}


/*
   NSM FEA bridge deinit.
*/
int pif_fea_bridge_deinit ()
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_bridge_delete(PIF_HAL_BRIDGE_NAME);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail(%d) to create default bridge \n", ret);
  }
#endif  
  return 0;
}



/* 
   Name: pif_fea_bridge_add 

   Description: 
   This API adds a bridge instance.

   Parameters:
   IN -> name - bridge name

   Returns:
   HAL_ERR_BRIDGE_EXISTS 
   HAL_ERR_BRIDGE_ADD_ERR 
   HAL_SUCCESS 

   HAL: int hal_bridge_add (char *brname, unsigned int is_vlan_aware, int protocol) 
*/
int pif_fea_bridge_add (char *name, unsigned int is_vlan_aware, int protocol)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_bridge_add(name, is_vlan_aware, protocol, 0, 0, NULL);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}


/* 
   Name: pif_fea_bridge_delete

   Description:
   This API deletes a bridge instance.

   Parameters:
   IN -> name - bridge name

   Returns:
   HAL_ERR_BRIDGE_NOT_EXISTS 
   HAL_ERR_BRIDGE_DELETE_ERR
   HAL_SUCCESS

   HAL: int hal_bridge_delete (char *brname)
*/
int pif_fea_bridge_delete (char *name)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_bridge_delete(name);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}



int pif_fea_bridge_change_protocol (char *name, int protocol)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_bridge_change_vlan_type(name, PIF_HAL_BRIDGE_VLAN_AWARE, (unsigned char)protocol);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}



/* 
   Name: hal_bridge_set_ageing_time

   Description:
   This API sets the ageing time for a bridge.

   Parameters:
   IN -> name - bridge name
   IN -> ageing_time - Ageing time in seconds.

   Returns:
   HAL_ERR_BRIDGE_NOT_EXISTS
   HAL_SUCCESS

   HAL: int hal_bridge_set_ageing_time (char *name, u_int32_t ageing_time)
*/
int pif_fea_bridge_set_ageing_time (char *name, u_int32_t ageing_time)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_bridge_set_ageing_time(name, ageing_time);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}


int pif_fea_bridge_disable_ageing (char *name)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_bridge_disable_ageing(name);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}


/* 
   Name: hal_bridge_set_iearning

   Description:
   This API sets the learning for a bridge.

   Parameters:
   IN -> name - bridge name

   Returns:
   HAL_ERR_BRIDGE_NOT_EXISTS
   HAL_SUCCESS

   HAL : int hal_bridge_set_learning (char *name, int learning(FALSE/TRUE));

*/
int pif_fea_bridge_set_set_learning (char *name, int learning)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_bridge_set_learning(name, learning);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}



/* 
   Name: hal_bridge_add_port

   Description:
   This API adds a port to a bridge. 
   
   Parameters:
   IN -> name - bridge name
   IN -> ifindex - interface index of port

   Returns:
   HAL_ERR_BRIDGE_PORT_EXISTS
   HAL_SUCCESS

   HAL : int hal_bridge_add_port (char *brname, unsigned int ifindex);
*/
int pif_fea_bridge_add_port (char *name, unsigned int ifindex)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_bridge_add_port(name, ifindex);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}



/* 
   Name: hal_bridge_delete_port

   Description:
   This API deletes a port from a bridge. 

   Parameters:
   IN -> name - bridge name
   IN -> ifindex - interface index of port

   Returns:
   HAL_ERR_BRIDGE_PORT_NOT_EXISTS
   HAL_SUCCESS

   HAL : int hal_bridge_delete_port (char *name, int ifindex);
*/
int pif_fea_bridge_delete_port (char *name, unsigned int ifindex)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_bridge_delete_port(name, ifindex);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}


/* 
   Name: hal_bridge_add_instance

   Description:
   This API adds a instance to a bridge. 

   Parameters:
   IN -> name - bridge name
   IN -> instance - instance number

   Returns:
   HAL_ERR_BRIDGE_INSTANCE_EXISTS
   HAL_SUCCESS

   HAL : int hal_bridge_add_instance (char * name, int instance);
*/
int pif_fea_bridge_add_instance (char *name, int instance)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_bridge_add_instance(name, instance);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}


/* 
   Name: hal_bridge_delete_instance

   Description:
   This API deletes the instance from the bridge.

   Parameters:
   IN -> name - bridge name
   IN -> instance - instance number

   Returns:
   HAL_ERR_BRIDGE_INSTANCE_NOT_EXISTS
   HAL_SUCCESS

   HAL : int hal_bridge_delete_instance (char * name, int instance);
*/
int pif_fea_bridge_delete_instance (char *name, int instance)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_bridge_delete_instance(name, instance);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}


/* 
   Name: hal_bridge_add_vlan_to_instance

   Description:
   This API adds a VLAN to a instance in a bridge.

   Parametes:
   IN -> name - bridge name
   IN -> instance - instance number
   IN -> vid - VLAN id

   Returns:
   HAL_ERR_BRIDGE_INSTANCE_NOT_EXISTS
   HAL_ERR_BRIDGE_VLAN_NOT_FOUND
   HAL_SUCCESS

   HAL : int hal_bridge_add_vlan_to_instance (char * name, int instance, 
				 unsigned short vid);
*/
int pif_fea_bridge_add_vlan_to_instance (char *name, int instance, 
										unsigned short vid)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_bridge_add_vlan_to_instance(name, instance, vid);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}


/* 
   Name: hal_bridge_delete_vlan_from_instance

   Description:
   This API deletes a VLAN from a instance in a bridge. 

   Parameters:
   IN -> name - bridge name
   IN -> instance - instance number
   IN -> vid - VLAN id

   Returns:
   HAL_ERR_BRIDGE_INSTANCE_NOT_EXISTS
   HAL_ERR_BRIDGE_VLAN_NOT_FOUND
   HAL_SUCCESS

   HAL : int hal_bridge_delete_vlan_from_instance (char * name, int instance, 
				      unsigned short vid);
*/
int pif_fea_bridge_add_delete_from_instance (char *name, int instance, 
										unsigned short vid)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_bridge_delete_vlan_from_instance(name, instance, vid);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}


/* 
   Name: hal_bridge_set_port_state

   Description:
   This API sets port state of a bridge port. 

   Parameters:
   IN -> name - bridge name
   IN -> ifindex - interface index of port
   IN -> instance - 
   IN -> state    - port state

   Returns:
   HAL_ERR_BRIDGE_PORT_NOT_EXISTS
   HAL_SUCCESS
   
   HAL : int hal_bridge_set_port_state (char *bridge_name,
			   int ifindex, int instance, int state);
*/
int pif_fea_bridge_set_port_state (char *name,
								int ifindex,
								int instance,
								int state)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_bridge_set_port_state(name, ifindex, instance, state);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}



/*
   Name: hal_bridge_set_learn_fwd

   Description:
   This API sets the learn and forwarding flag for a port.

   Parameters:
   IN -> bridge_name - bridge name
   IN -> ifindex  - interface index of the port.
   IN -> instance - instance number
   IN -> learn - learning to be enabled
   IN -> forward - forwarding to be enabled

   Returns:
   HAL_ERR_BRIDGE_INSTANCE_NOT_EXISTS
   HAL_ERR_BRIDGE_VLAN_NOT_FOUND
   HAL_SUCCESS

   HAL : int hal_bridge_set_learn_fwd (const char *const bridge_name,const int ifindex,
                          const int instance, const int learn, 
                          const int forward);
*/
int pif_fea_bridge_set_learn_fwd (char *name, 
								  int ifindex, 
								  int instance,
								  int learn, 
								  int forward)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_bridge_set_learn_fwd(name, ifindex, instance, learn, forward);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}


/*
int hal_set_acl_for_access_group (struct access_list *access,
                              struct hal_ip_access_grp *access_grp);

int hal_ip_set_access_group (struct hal_ip_access_grp access_grp,
                         char *ifname,
                         int action, int dir);

int hal_ip_set_access_group_interface (struct hal_ip_access_grp access_grp,
                                   char *vifname, char *ifname,
                                   int action, int dir);
*/



/* mac address table */
int pif_fea_bridge_add_mac_static (char *name, char* mac, int ifindex, unsigned short vid)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_l2_add_fdb(name, ifindex, (unsigned char *)mac, HAL_HW_LENGTH, 
		  					vid, 0, HAL_L2_FDB_STATIC, PAL_TRUE);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}


int pif_fea_bridge_del_mac_static (char *name, char* mac, int ifindex, unsigned short vid)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_l2_del_fdb(name, ifindex, (unsigned char *)mac, HAL_HW_LENGTH, 
		  				   vid, 0, HAL_L2_FDB_STATIC);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}


int pif_fea_bridge_add_mac_dynamic (char *name, char* mac, int ifindex, unsigned short vid)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_l2_add_fdb(name, ifindex, (unsigned char *)mac, HAL_HW_LENGTH, 
		  	               vid, 0, HAL_L2_FDB_DYNAMIC, PAL_TRUE);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}


int pif_fea_bridge_del_mac_dynamic (char *name, char* mac, int ifindex, unsigned short vid)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_l2_del_fdb(name, ifindex, (unsigned char *)mac, HAL_HW_LENGTH, 
		  				   vid, 0, HAL_L2_FDB_DYNAMIC);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}


int pif_fea_bridge_clear_mac_dynamic (char *name, int ifindex, unsigned short vid)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_bridge_flush_fdb_by_port(name, ifindex, 0, vid, 0);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}


int pif_fea_bridge_clear_mac_dynamic_by_mac (char *name, char* mac)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  int ret = hal_bridge_flush_dynamic_fdb_by_mac(name, (unsigned char *)mac, HAL_HW_LENGTH);
  if(ret != HAL_SUCCESS) {
  	PAL_DBUG(0, "Fail to set HAL interface api \n");
  }
#endif  
  return 0;
}






