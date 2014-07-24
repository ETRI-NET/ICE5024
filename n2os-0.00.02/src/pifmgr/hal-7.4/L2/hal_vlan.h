/* Copyright (C) 2004 IP Infusion, Inc. All Rights Reserved. */

#ifndef _HAL_VLAN_H_
#define _HAL_VLAN_H_

/* 
   Name: hal_vlan_init

   Description:
   This API initializes the VLAN hardware layer component.

   Parameters:
   None

   Returns:
   HAL_ERR_VLAN_INIT
   HAL_SUCCESS
*/
int
hal_vlan_init (void);

/* 
   Name: hal_vlan_deinit

   Description:
   This API deinitializes the VLAN hardware layer component.

   Parameters:
   None

   Returns:
   HAL_ERR_VLAN_DEINIT
   HAL_SUCCESS
*/
int
hal_vlan_deinit (void);

/* 
   Name: hal_vlan_add 

   Description: 
   This API adds a VLAN.

   Parameters:
   IN -> name - bridge name
   IN -> vid - VLAN id

   Returns:
   HAL_ERR_VLAN_EXISTS 
   HAL_SUCCESS 
*/
int
hal_vlan_add (char *name, unsigned short vid);

/* 
   Name: hal_vlan_delete

   Description:
   This API deletes a VLAN.

   Parameters:
   IN -> name - bridge name
   IN -> vid - VLAN id

   Returns:
   HAL_ERR_VLAN_NOT_EXISTS
   HAL_SUCCESS
*/
int
hal_vlan_delete (char *name, unsigned short vid);

/* 
   Name: hal_vlan_set_port_type

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
*/
int
hal_vlan_set_port_type (char *name, 
			unsigned int ifindex, 
			enum hal_vlan_port_type port_type,
			enum hal_vlan_acceptable_frame_type acceptable_frame_types,
			unsigned short enable_ingress_filter);


/* 
   Name: hal_vlan_set_default_pvid

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
*/
int
hal_vlan_set_default_pvid (char *name, unsigned int ifindex,
			   unsigned short pvid, 
			   enum hal_vlan_egress_type egress);

/*
   Name: hal_vlan_set_native_vid

   Description:
   This API sets the native VID for a port.

   Parameters:
   IN -> name - bridge name
   IN -> ifindex - interface index
   IN -> vid - native VID

   Returns:
   HAL_ERR_VLAN_NOT_EXISTS
   HAL_SUCCESS
*/

int
hal_vlan_set_native_vid (char *name, unsigned int ifindex,
                         unsigned short vid);

/* 
   Name: hal_vlan_add_vid_to_port

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
*/
int
hal_vlan_add_vid_to_port (char *name, unsigned int ifindex,
			  unsigned short vid, 
			  enum hal_vlan_egress_type egress);

/* 
   Name: hal_vlan_delete_vid_from_port

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
*/
int
hal_vlan_delete_vid_from_port (char *name, unsigned int ifindex,
			       unsigned short vid);

#endif /* _HAL_VLAN_H_ */
