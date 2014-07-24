/* Copyright (C) 2003 IP Infusion, Inc. All Rights Reserved.
  
LAYER 2 BRIDGE HAL
  
This module defines the platform abstraction layer to the 
Linux layer 2 bridging.

*/

#include "hal_incl.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <linux/if.h>
#include <net/if.h>
#include <errno.h>

//#include "pal.h"
//#include "lib.h"
//#include "filter.h"
//#include "hal_ipifwd.h"
//#ifdef HAVE_IGMP_SNOOP
//#include "hal_igmp_snoop.h"
//#endif /* HAVE_IGMP_SNOOP */

int
hal_igmp_snooping_add_entry (char *bridge_name,
                             struct hal_in4_addr *src,
                             struct hal_in4_addr *group,
                             char is_exclude,
                             int vid,
                             int count,
                             unsigned int *ifindexes)
{
  u_int8_t mac [HAL_HW_LENGTH];

  if (! bridge_name || ! src || ! group || !ifindexes)
    return HAL_ERR_INVALID_ARG;

  HAL_CONVERT_IPV4MCADDR_TO_MAC (group, mac);

  return hal_l2_add_fdb (bridge_name, ifindexes [0], mac, HAL_HW_LENGTH,
                         vid, HAL_L2_FDB_STATIC, PAL_TRUE);
}

int
hal_igmp_snooping_delete_entry (char *bridge_name,
                                struct hal_in4_addr *src,
                                struct hal_in4_addr *group,
                                char is_exclude,
                                int vid,
                                int count,
                                unsigned int *ifindexes)
{
  u_int8_t mac [HAL_HW_LENGTH];

  if (! bridge_name || ! src || ! group || !ifindexes)
    return HAL_ERR_INVALID_ARG;

  HAL_CONVERT_IPV4MCADDR_TO_MAC (group, mac);

  return hal_l2_del_fdb (bridge_name, ifindexes [0], mac, HAL_HW_LENGTH,
                         vid, HAL_L2_FDB_STATIC);
}

/*
int
hal_mld_snooping_delete_entry (char *bridge_name,
                               struct hal_in6_addr *src,
                               struct hal_in6_addr *group,
                               char is_exclude, 
                               int vid,
                               int count,
                               unsigned int *ifindexes)
{
  return 0;
}

int
hal_mld_snooping_add_entry (char *bridge_name,
                            struct hal_in6_addr *src,
                            struct hal_in6_addr *group,
                            char is_exclude,
                            int vid,
                            int count,
                            unsigned int *ifindexes)
{
  return 0;
}
*/

/* Create a bridge instance */
int
hal_bridge_add (char *name, unsigned int is_vlan_aware, int protocol)
{
  int ret;
  char str[HAL_BRIDGE_NAME_LEN];

  memset (str, '\0', HAL_BRIDGE_NAME_LEN);
  pal_strncpy (str, name, HAL_BRIDGE_NAME_LEN);
  ret = hal_ioctl (IPIBR_ADD_BRIDGE, (unsigned long)str, is_vlan_aware, 
                      protocol, 0, 0);
  if (ret < 0)
    {
      return -errno;
    }

  return ret;
}

/* Delete a bridge instance */

int
hal_bridge_delete (char *name)
{
  int ret;
  char str[HAL_BRIDGE_NAME_LEN];

  memset (str, '\0', HAL_BRIDGE_NAME_LEN);
  pal_strncpy (str, name, HAL_BRIDGE_NAME_LEN);


  ret = hal_ioctl (IPIBR_DEL_BRIDGE, (unsigned long)str, 0, 0, 0, 0);
  if (ret < 0)
    return -errno;

  return HAL_SUCCESS;
}


/* Set the bridge dynamic ageing time (in seconds) */

int
hal_bridge_set_ageing_time (char *name, u_int32_t ageing_time)
{
  int ret;

  ret = hal_ioctl (IPIBR_SET_AGEING_TIME, (unsigned long)name, ageing_time, 
                      0, 0, 0);
  if (ret < 0)
    return -errno;

  return HAL_SUCCESS;
}

/* Set the bridge learning interval (in seconds). */

int
hal_bridge_set_learning (char *name, int learning)
{
  int ret;

  /* The FID dependancy is not implemented in the software forwarder Yet.
     so ignore it. */
  ret = hal_ioctl (IPIBR_SET_BRIDGE_LEARNING, (unsigned long)name, learning, 
                      0, 0, 0);
  if (ret < 0)
    return -errno;

  return HAL_SUCCESS;
}

/* Add a static mac entry if it does not already exist for the specified port. */

int
hal_l2_add_fdb (const char * const name, unsigned int ifindex, 
                const unsigned char * const mac, int len,
                unsigned short vid, unsigned char flags, bool_t is_forward)
{
  int ret;

  //int port = if_nametoindex ((const char *)ifName);
  if (ifindex < 0)
    return -errno;

  if ( flags & HAL_L2_FDB_STATIC )
    ret = hal_ioctl (IPIBR_ADD_STATFDB_ENTRY, (unsigned long)name, ifindex, 
                        (unsigned long)mac, is_forward, vid);
  else
    ret = hal_ioctl (IPIBR_ADD_DYNAMIC_FDB_ENTRY, (unsigned long)name,
                     (unsigned long)ifindex, (unsigned long)vid,
                     (unsigned long)mac, is_forward);
    
  if (ret < 0)
    return -errno;

  return HAL_SUCCESS;
}

/* Remove a static mac entry if it exists. */

int
hal_l2_del_fdb (const char * const name, unsigned int ifindex, 
		const unsigned char * const mac, int len,
                const unsigned short vid, unsigned char flags)
{
  int ret;

  if (flags & HAL_L2_FDB_STATIC)
    ret = hal_ioctl (IPIBR_DEL_STATFDB_ENTRY, (unsigned long)name, ifindex, 
                        (unsigned long)mac, vid, 0);
  else
    ret = hal_ioctl (IPIBR_DEL_DYNAMIC_FDB_ENTRY, (unsigned long)name,
                        (unsigned long)ifindex, (unsigned long)vid,
                        (unsigned long)mac, vid);
  if (ret < 0)
    return -errno;

  return HAL_SUCCESS;
}

/* Read the dynamic fdb entries. Requires that fdbs point to
   an array of cardinality HAL_BRIDGE_MAX_DYN_FDB_ENTRIES. */

int
hal_bridge_read_fdb (const char * const name, struct hal_fdb_entry *fdbs)
{
  int ret;
  memset (fdbs, '\0', 
          sizeof (struct hal_fdb_entry) * HAL_BRIDGE_MAX_DYN_FDB_ENTRIES);

  ret = hal_ioctl (IPIBR_GET_DYNFDB_ENTRIES, (unsigned long)name,
		      (unsigned long)fdbs, HAL_BRIDGE_MAX_DYN_FDB_ENTRIES, 
                      0, 0);
  if (ret < 0)
    {
      return -errno;
    }
  return ret;
}


int
hal_bridge_read_fdb_all (const char * const name, struct fdb_entry *fdbs)
{
  int ret;
  memset (fdbs, '\0', 
          sizeof (struct fdb_entry) * HAL_BRIDGE_MAX_DYN_FDB_ENTRIES);

  ret = hal_ioctl (IPIBR_GET_DYNFDB_ENTRIES, (unsigned long)name,
		      (unsigned long)fdbs, HAL_BRIDGE_MAX_DYN_FDB_ENTRIES, 
                      0, 0);
  if (ret < 0)
  {
      return -errno;
  }
  return ret;
}



/* Read the static fdb entries. Requires that fdbs argument
   point to an array of cardinality HAL_BRIDGE_MAX_STATIC_FDB_ENTRIES! */

int
hal_bridge_read_statfdb (const char * const name, struct hal_fdb_entry *fdbs)
{
  int ret;
  memset (fdbs, '\0', 
          sizeof (struct hal_fdb_entry) * HAL_BRIDGE_MAX_STATIC_FDB_ENTRIES);

  ret = hal_ioctl (IPIBR_GET_STATFDB_ENTRIES, (unsigned long)name,
		      (unsigned long)fdbs, HAL_BRIDGE_MAX_STATIC_FDB_ENTRIES, 
                      0, 0);
  if (ret < 0) 
    {
      return -errno;
    }
  return ret;
}

int
hal_bridge_read_statfdb_all (const char * const name, struct fdb_entry *fdbs)
{
  int ret;
  memset (fdbs, '\0', 
          sizeof (struct fdb_entry) * HAL_BRIDGE_MAX_STATIC_FDB_ENTRIES);

  ret = hal_ioctl (IPIBR_GET_STATFDB_ENTRIES, (unsigned long)name,
		      (unsigned long)fdbs, HAL_BRIDGE_MAX_STATIC_FDB_ENTRIES, 
                      0, 0);
  if (ret < 0) 
    {
      return -errno;
    }
  return ret;
}



/* Add an port to a bridge instance. */

int
hal_bridge_add_port (char *name, unsigned int ifindex)
{

  if (hal_ioctl (IPIBR_ADD_IF, (unsigned long)name, (int)ifindex, 
                    0, 0, 0) < 0)
    {
      return -errno;
    }
  return HAL_SUCCESS;
}

/* Delete an port from a bridge instance */

int
hal_bridge_delete_port (char *name, int ifindex)
{
  if (hal_ioctl (IPIBR_DEL_IF, (unsigned long)name, (int)ifindex, 
                    0, 0, 0) < 0)
    {
      return -errno;
    }

  return HAL_SUCCESS;
}

extern int hal_ioctl_is_port_up(struct ifreq *ifr);

int
hal_bridge_is_port_up (const int ifindex)
{
  struct ifreq  ifr;
  char ifname[HAL_IFNAME_LEN];

  if_indextoname (ifindex, ifname);
  pal_strncpy (ifr.ifr_name, ifname, HAL_IFNAME_LEN);

  if (hal_ioctl_is_port_up (&ifr) < 0)
    return PAL_FALSE;
  if (ifr.ifr_flags & IFF_UP)
    return PAL_TRUE;
  return PAL_FALSE;
}

int
hal_bridge_set_port_state (char *bridge_name,
			   int ifindex, int instance, int state)
{
  if (hal_ioctl (IPIBR_SET_PORT_STATE, (unsigned long)bridge_name,
		    ifindex, instance, state, 0) < 0)
    {
      return -errno;
    }
 
  return HAL_SUCCESS;
}

int
hal_bridge_flush_fdb_by_port (char *bridge_name,
			      unsigned int ifindex,
                              unsigned short vid)
{
  int instance = 0;
  if (hal_ioctl (IPIBR_FLUSH_FDB_BY_PORT, (unsigned long)bridge_name, 
                    ifindex, instance, vid, 0) < 0)
    {
      //perror ("pal: flush_fdb");
      return -errno;
    }

  return HAL_SUCCESS;
}

/* Enable auth-mac on the port */
int
hal_bridge_auth_mac_enabled_on_port (char *bridge_name,
                                     unsigned int ifindex,
                                     unsigned short vid)
{
  int instance = 0;
  if (hal_ioctl (IPIBR_FLUSH_FDB_BY_PORT, (unsigned long)bridge_name,
                    ifindex, instance, vid, 0) < 0)
    {
      //perror ("pal: flush_fdb");
      return -errno;
    }

  return HAL_SUCCESS;
}

int
hal_bridge_set_learn_fwd (const char *const bridge_name,const int ifindex, 
			  const int instance, const int learn, const int forward)
{
  if (hal_ioctl (IPIBR_SET_PORT_FWDER_FLAGS, (unsigned long)bridge_name, 
                    ifindex, instance, learn, forward) < 0)
    {
      return -errno;
    }

  return HAL_SUCCESS;
}

/* L2 QOS Api */
int hal_l2_qos_init (void)
{
  return HAL_SUCCESS;
}

int hal_l2_qos_deinit (void)
{
  return HAL_SUCCESS;
}

int hal_l2_qos_default_user_priority_set (unsigned int ifindex,
				      unsigned char user_priority)
{
  return HAL_SUCCESS;
}

int hal_l2_qos_default_user_priority_get (unsigned int ifindex,
				      unsigned char *user_priority)
{
  *user_priority = 0;
  return HAL_SUCCESS;
}

int hal_l2_qos_regen_user_priority_set (unsigned int ifindex, 
				    unsigned char recvd_user_priority,
				    unsigned char regen_user_priority)
{
  return HAL_SUCCESS;
}

int hal_l2_qos_regen_user_priority_get (unsigned int ifindex,
				    unsigned char *regen_user_priority)
{
  *regen_user_priority = 0;
  return HAL_SUCCESS;
}

int hal_l2_qos_traffic_class_set (unsigned int ifindex,
			      unsigned char user_priority,
			      unsigned char traffic_class,
			      unsigned char traffic_class_value)
{
  return HAL_SUCCESS;
}

int hal_l2_qos_traffic_class_get (unsigned int ifindex,
			      unsigned char user_priority,
			      unsigned char traffic_class,
			      unsigned char traffic_class_value)
{
  return HAL_SUCCESS;
}

/* Vlan related pal functions */
int
hal_vlan_add_old (char *name, unsigned short vid)
{
  int ret;

  ret = hal_ioctl (IPI_VLAN_ADD, (unsigned long)name, 
      (unsigned long)vid, 0, 0, 0);

  if (ret < 0)
    {
      return -errno;
    }

  return HAL_SUCCESS;
}

int
hal_vlan_delete_old (char *bridge_name, unsigned short vid)
{
  int ret;

  ret = hal_ioctl (IPI_VLAN_DEL, (unsigned long)bridge_name,
		      (unsigned long)vid, 0, 0, 0);

  if (ret < 0)
    {
      return -errno;
    }

  return HAL_SUCCESS;
}




int
hal_vlan_set_port_type (char * const bridge_name, 
                        unsigned int ifindex,
                        enum hal_vlan_port_type port_type,
                        enum hal_vlan_acceptable_frame_type acceptable_frame_types,
                       const unsigned short enable_ingress_filter)
{
  int ret;
  
  ret = hal_ioctl (IPI_VLAN_SET_PORT_TYPE, (unsigned long)bridge_name,
		      (unsigned long)ifindex, (unsigned long)port_type,
		      (unsigned long)acceptable_frame_types, 
		      (unsigned long)enable_ingress_filter);

  if (ret < 0)
    {
      return -errno;
    }

  return HAL_SUCCESS;
}

int
hal_vlan_set_default_pvid (char *name,
                           unsigned int ifindex,
                           unsigned short pvid,
                           enum hal_vlan_egress_type egress_tagged)
{
  int ret;

  ret = hal_ioctl (IPI_VLAN_SET_DEFAULT_PVID, (unsigned long)name,
		      (unsigned long)ifindex, (unsigned long)pvid, 
		      (unsigned long)egress_tagged, 0);
  if (ret < 0)
    {
      return -errno;
    }

  return HAL_SUCCESS;
}

int
hal_vlan_set_native_vid (char *name,
                         unsigned int ifindex,
                         unsigned short native_vid)
{
  int ret;

  ret = hal_ioctl (IPI_VLAN_SET_NATIVE_VID, (unsigned long)name,
                      (unsigned long)ifindex, (unsigned long)native_vid,
                       0, 0);
  if (ret < 0)
    {
      return -errno;
    }

  return HAL_SUCCESS;
}

int
hal_vlan_add_vid_to_port (char *name,
                          unsigned int ifindex,
                          unsigned short vid,
                          enum hal_vlan_egress_type egress_tagged)
{
  int ret;

  ret = hal_ioctl (IPI_VLAN_ADD_VID_TO_PORT, (unsigned long)name,
		      (unsigned long)ifindex, (unsigned long)vid,
		      (unsigned long)egress_tagged, 0);

  if (ret < 0)
    {
      return -errno;
    }

  return HAL_SUCCESS;
}

int
hal_vlan_delete_vid_from_port (char *name,
                               unsigned int ifindex,
                               unsigned short vid)
                     
{
  int ret;

  ret = hal_ioctl (IPI_VLAN_DEL_VID_FROM_PORT, (unsigned long)name,
		      (unsigned long)ifindex, (unsigned long)vid, 0, 0); 

  if (ret < 0)
    {
      return -errno;
    }

  return HAL_SUCCESS;
}

/*
int
hal_vlan_add_all_vids_to_port (const char * const bridge_name,
                               int ifindex,
                               unsigned short egress_tagged)
{
  int ret;

  ret = hal_ioctl (IPI_VLAN_ADD_ALL_VIDS_TO_PORT,
		      (unsigned long)bridge_name, (unsigned long)ifindex, 
		      (unsigned long)egress_tagged, 0, 0);

  if (ret < 0)
    {
      return -errno;
    }
  
  return HAL_SUCCESS;
}

int
hal_vlan_delete_all_vids_from_port (const char * const bridge_name,
                                    int ifindex)
{
  int ret;

  ret = hal_ioctl (IPI_VLAN_DEL_ALL_VIDS_FROM_PORT, 
		      (unsigned long)bridge_name, (unsigned long)ifindex, 
		      0, 0, 0);
    
  if (ret < 0)
    {
      return -errno;
    }

  return HAL_SUCCESS;
}
*/

#ifdef HAVE_IGMP_SNOOP
int
hal_igmp_snooping_enable (char* bridge_name)
{
  int ret;
 
  ret = hal_ioctl (IPIBR_ENABLE_IGMP_SNOOPING, (unsigned long)bridge_name, 0,0,0,0);

  if (ret < 0)
    {
      return -errno;
    }
  return HAL_SUCCESS;
}

int
hal_igmp_snooping_disable (char* bridge_name)
{
  int ret;

  ret = hal_ioctl (IPIBR_DISABLE_IGMP_SNOOPING, (unsigned long)bridge_name,0,0,0,0);

  if (ret < 0)
    {
      return -errno;
    }

  return HAL_SUCCESS;
}
#endif /* HAVE_IGMP_SNOOP */

#ifdef HAVE_MLD_SNOOP
int
hal_mld_snooping_enable (char* bridge_name)
{
  int ret;

  ret = hal_ioctl (IPIBR_ENABLE_MLD_SNOOPING, (unsigned long)bridge_name, 0,0,0,0);

  if (ret < 0)
    {
      return -errno;
    }
  return HAL_SUCCESS;
}

int
hal_mld_snooping_disable (char* bridge_name)
{
  int ret;

  ret = hal_ioctl (IPIBR_DISABLE_MLD_SNOOPING, (unsigned long)bridge_name,0,0,0,0);

  if (ret < 0)
    {
      return -errno;
    }

  return HAL_SUCCESS;
}
#endif /* HAVE_MLD_SNOOP */

/* This function notifies the forwarder about the mapping 
   of the VLAN id to the instance.
*/
int
hal_bridge_add_vlan_to_instance (char *bridge_name, int instance_id,
    unsigned short vlanid)
{ 

  int ret;

  ret = hal_ioctl (IPIBR_ADD_VLAN_TO_INST, (unsigned long )bridge_name,
                      (unsigned long)instance_id, (unsigned long)vlanid,
                      0, 0);
  
  if (ret < 0)
    {
      return -errno;
    }

  return HAL_SUCCESS;

}

/* Delete VLAN from instance. */
int
hal_bridge_delete_vlan_from_instance (char *bridge_name, int instance_id,
    unsigned short vlanid)
{
#if 0
  int ret;

  ret = hal_ioctl (IPIBR_DELETE_VLAN_FROM_INST, (unsigned long )bridge_name,
                      (unsigned long)instance_id, (unsigned long)vlanid,
                      0, 0);
  
  if (ret < 0)
    {
      return -errno;
    }
#endif

  return HAL_SUCCESS;
}

int
hal_vlan_stacked_enable (const char * const bridge_name,
                         const int ifindex,
                         const unsigned short vid)
{
  return HAL_SUCCESS;
}

int
hal_vlan_stacked_disable (const char * const bridge_name,
                          const int ifindex)
{
  return HAL_SUCCESS;
}

/* This function is for colleting port HC stats 
 */
struct port_HC_stats * 
hal_get_port_HC_stats(const int ifindex, const int vlanid, const char * const
		      bridge_name)
{ 

  return NULL;

}

/* This function is for colleting port stats 
 */
struct port_stats * 
hal_get_port_stats(const int ifindex, const int vlanid, const char * const
		   bridge_name)
{ 

  return NULL;

}

/* Rate Limit Interface */
int
hal_ratelimit_init ()
{
  return HAL_SUCCESS;
}

int
hal_ratelimit_deinit ()
{
  return HAL_SUCCESS;
}

int
hal_l2_ratelimit_bcast (unsigned int ifindex, 
                        unsigned char level, unsigned char fraction)
{
  return HAL_SUCCESS;
}

int
hal_l2_bcast_discards_get (unsigned int ifindex, unsigned int *discards)
{
  return HAL_SUCCESS;
}

int
hal_l2_ratelimit_mcast (unsigned int ifindex, 
                        unsigned char level, unsigned char fraction)
{
  return HAL_SUCCESS;
}

int
hal_l2_mcast_discards_get (unsigned int ifindex, unsigned int *discards)
{
  return HAL_SUCCESS;
}

int
hal_l2_ratelimit_dlf_bcast (unsigned int ifindex,
                            unsigned char level, unsigned char fraction)
{
  return HAL_SUCCESS;
}

int
hal_l2_dlf_bcast_discards_get (unsigned int ifindex, unsigned int *discards)
{
  return HAL_SUCCESS;
}

/* This function mirror one port to another port 
 */
int
hal_port_mirror_set(unsigned int to_ifindex,
		    unsigned int from_ifindex,
		    enum hal_port_mirror_direction direction)
{ 
  return HAL_ERR_PMIRROR_SET;
}

/* This function mirror one port to another port 
 */
int
hal_port_mirror_unset(unsigned int to_ifindex,
		      unsigned int from_ifindex,
		      enum hal_port_mirror_direction direction)
{ 
  return HAL_ERR_PMIRROR_UNSET;
}

int
hal_flow_control_set( unsigned int ifindex,
		      unsigned char direction)
{ 
  return HAL_ERR_FLOW_CONTROL_SET;
}

void
hal_flow_control_statictics(unsigned int ifindex, 
                            unsigned char *direction,
                            int *rxpause,
                            int *txpause)
{
  *direction = 0;
  *rxpause = 0;
  *txpause = 0;
}

int
hal_flow_control_init ()
{
  return HAL_SUCCESS;
}

int
hal_flow_control_statistics (unsigned int ifindex, unsigned char *direction, 
                             int *rxpause, int *txpause)
{
  return HAL_SUCCESS;
}

int
hal_flow_control_deinit ()
{
  return HAL_SUCCESS;
}

int
hal_vlan_classifier_init()
{
  return HAL_SUCCESS;
}

int
hal_vlan_classifier_deinit()
{
  return HAL_SUCCESS;
}

int
hal_vlan_classifier_add (struct hal_vlan_classifier_rule *rule_ptr,u_int32_t ifindex, u_int32_t refcount)

{
  return HAL_SUCCESS;
}

int
hal_vlan_classifier_del (struct hal_vlan_classifier_rule *rule_ptr,u_int32_t ifindex, u_int32_t refcount)
{
  return HAL_SUCCESS;
}

int
hal_vlan_set_classification_group (const char * const bridge_name,
                             const int ifindex, const unsigned short pvid,
                             const int group)
{
  return HAL_SUCCESS;
}

int
hal_vlan_clr_classification_group (const char * const bridge_name,
                             const int ifindex, const unsigned short pvid,
                             const int group)
{
  return HAL_SUCCESS;
}

#ifdef HAVE_VLAN_STACK
int
hal_vlan_stacking_enable (u_int32_t ifindex, u_int16_t ethtype
                          u_int16_t stackmode)
{
  return HAL_SUCCESS;
}


int
hal_vlan_stacking_disable (u_int32_t ifindex)
{
  return HAL_SUCCESS;
}
#endif /* HAVE_VLAN_STACK */

#ifdef HAVE_L2LERN
int
hal_mac_set_access_grp( struct hal_mac_access_grp *hal_macc_grp,
                        int ifindex,
                        int action,
                        int dir)
{
  return HAL_SUCCESS;
}

#ifdef HAVE_VLAN
int
hal_vlan_set_access_map( struct hal_mac_access_grp *hal_macc_grp,
                        int vid,
                        int action)
{
  return HAL_SUCCESS;
}
#endif /* HAVE_VLAN */
#endif /* HAVE_L2LERN */

/*
int
hal_set_acl_for_access_group (struct access_list *access,
                              struct hal_ip_access_grp *access_grp)
{
  return HAL_SUCCESS;
}

int
hal_ip_set_access_group (struct hal_ip_access_grp access_grp,
                         char *ifname, int action, int dir)
{
  return HAL_SUCCESS;
}

int
hal_ip_set_access_group_interface (struct hal_ip_access_grp access_grp,
                                   char *vifname, char *ifname,
                                   int action, int dir)
{
  return HAL_SUCCESS;
}
*/

int 
hal_get_master_cpu_entry (unsigned char *cpu_entry);



int hal_get_bridge_info(char* name, struct bridge_info *info)
{
	int ret;
	char brname[HAL_BRIDGE_NAME_LEN];

	memset (brname, '\0', HAL_BRIDGE_NAME_LEN);
	pal_strncpy (brname, name, HAL_BRIDGE_NAME_LEN);

	ret = hal_ioctl (IPIBR_GET_BRIDGE_INFO, (unsigned long)brname, (unsigned long)info, 0, 0, 0);
	if (ret < 0)
	{
    	return -errno;
	}

	return ret;
}


int hal_get_bridge_port_list(char* name, unsigned int *port_list)
{
	int ret;
	char brname[HAL_BRIDGE_NAME_LEN];

	memset (brname, '\0', HAL_BRIDGE_NAME_LEN);
	pal_strncpy (brname, name, HAL_BRIDGE_NAME_LEN);

	ret = hal_ioctl (IPIBR_GET_PORT_LIST, (unsigned long)brname, (unsigned long)port_list, 
				     HAL_BRIDGE_MAX_PORTS, 0, 0);
	if (ret < 0)
	{
    	return -errno;
	}

	return ret;
}


int hal_get_bridge_port_state(char* name, unsigned int ifindex, unsigned int instance, unsigned int *state)
{
	int ret;
	char brname[HAL_BRIDGE_NAME_LEN];

	memset (brname, '\0', HAL_BRIDGE_NAME_LEN);
	pal_strncpy (brname, name, HAL_BRIDGE_NAME_LEN);

	ret = hal_ioctl (IPIBR_GET_PORT_STATE, (unsigned long)brname, (unsigned long)ifindex, 
					(unsigned long)instance, (unsigned long)state, 0);
	if (ret < 0)
	{
    	return -errno;
	}

	return ret;
}



/*********************************************************************
 *
 * ZebOS 7.6 API
 *
 * ******************************************************************/
int
hal_vlan_add (char *name, enum hal_vlan_type type, enum hal_evc_type evc_type, unsigned short vid)
{
  int ret;

  ret = hal_ioctl (IPI_VLAN_ADD, (unsigned long)name, type, evc_type,
                   (unsigned long)vid, 0, 0);

  if (ret < 0)
    { 
      return -errno;
    }    
          
  return HAL_SUCCESS;
}       


int
hal_vlan_delete (char *bridge_name, enum hal_vlan_type type, unsigned short vid)
{
  int ret;

  ret = hal_ioctl (IPI_VLAN_DEL, (unsigned long)bridge_name, type,
                   (unsigned long)vid, 0, 0, 0);

  if (ret < 0)
    {
      return -errno;
    }

  return HAL_SUCCESS;
}



int
hal_vlan_disable (char *bridge_name, enum hal_vlan_type type, unsigned short vid)
{
  int ret;

  ret = hal_ioctl (IPI_VLAN_DISABLE, (unsigned long)bridge_name, type,
                   (unsigned long)vid, 0, 0, 0);

  if (ret < 0)
    {
      return -errno;
    }

  return HAL_SUCCESS;
}

int
hal_vlan_enable (char *bridge_name, enum hal_vlan_type type, unsigned short vid)
{
  int ret;

  ret = hal_ioctl (IPI_VLAN_ENABLE, (unsigned long)bridge_name, type,
                   (unsigned long)vid, 0, 0, 0);

  if (ret < 0)
    {
      return -errno;
    }

  return HAL_SUCCESS;
}









