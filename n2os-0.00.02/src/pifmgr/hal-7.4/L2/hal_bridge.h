/* Copyright (C) 2004 IP Infusion, Inc. All Rights Reserved. */

#ifndef _HAL_BRIDGE_H_
#define _HAL_BRIDGE_H_

#define HAL_PROTO_STP  1
#define HAL_PROTO_RSTP 2
#define HAL_PROTO_MSTP 3
#define HAL_PROTO_MAX  4

/* 
   Name: hal_bridge_init

   Description:
   This API initializes the bridging hardware layer component.

   Parameters:
   None

   Returns:
   HAL_ERR_BRIDGE_INIT
   HAL_SUCCESS
*/
int 
hal_bridge_init (void);

/* 
   Name: hal_bridge_deinit

   Description:
   This API deinitializes the bridging hardware layer component.

   Parameters:
   None

   Returns:
   HAL_ERR_BRIDGE_DEINIT
   HAL_SUCCESS
*/
int
hal_bridge_deinit (void);

/* 
   Name: hal_bridge_add 

   Description: 
   This API adds a bridge instance.

   Parameters:
   IN -> name - bridge name

   Returns:
   HAL_ERR_BRIDGE_EXISTS 
   HAL_ERR_BRIDGE_ADD_ERR 
   HAL_SUCCESS 
*/
int
hal_bridge_add (char *name, unsigned int is_vlan_aware, int protocol);

/* 
   Name: hal_bridge_delete

   Description:
   This API deletes a bridge instance.

   Parameters:
   IN -> name - bridge name

   Returns:
   HAL_ERR_BRIDGE_NOT_EXISTS 
   HAL_ERR_BRIDGE_DELETE_ERR
   HAL_SUCCESS
*/
int
hal_bridge_delete (char *name);

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
*/
int
hal_bridge_set_ageing_time (char *name, u_int32_t ageing_time);

/* 
   Name: hal_bridge_set_iearning

   Description:
   This API sets the learning for a bridge.

   Parameters:
   IN -> name - bridge name

   Returns:
   HAL_ERR_BRIDGE_NOT_EXISTS
   HAL_SUCCESS
*/
int
hal_bridge_set_learning (char *name, int learning);

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
*/
int
hal_bridge_add_port (char *name, unsigned int ifindex);

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
*/
int
hal_bridge_delete_port (char *name, int ifindex);

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
*/
int
hal_bridge_set_port_state (char *bridge_name,
			   int ifindex, int instance, int state);

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
*/
int
hal_bridge_add_instance (char * name, int instance);

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
*/
int
hal_bridge_delete_instance (char * name, int instance);

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
*/
int
hal_bridge_add_vlan_to_instance (char * name, int instance, 
				 unsigned short vid);

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
*/
int
hal_bridge_delete_vlan_from_instance (char * name, int instance, 
				      unsigned short vid);

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
*/
int
hal_bridge_set_learn_fwd (const char *const bridge_name,const int ifindex,
                          const int instance, const int learn, 
                          const int forward);

/*
int
hal_set_acl_for_access_group (struct access_list *access,
                              struct hal_ip_access_grp *access_grp);
*/

int
hal_ip_set_access_group (struct hal_ip_access_grp access_grp,
                         char *ifname,
                         int action, int dir);
int
hal_ip_set_access_group_interface (struct hal_ip_access_grp access_grp,
                                   char *vifname, char *ifname,
                                   int action, int dir);


#include "hal_ipifwd.h"
int       
hal_bridge_read_fdb (const char * const name, struct hal_fdb_entry *fdbs);

int
hal_bridge_read_fdb_all (const char * const name, struct fdb_entry *fdbs);

int
hal_bridge_read_statfdb (const char * const name, struct hal_fdb_entry *fdbs);

int
hal_bridge_read_statfdb_all (const char * const name, struct fdb_entry *fdbs);


int hal_get_bridge_info(char* name, struct bridge_info *info);

int hal_get_bridge_port_list(char* name, unsigned int *port_list);

int hal_get_bridge_port_state(char* name, unsigned int ifindex, unsigned int instance, unsigned int *state);













#endif
