/* Copyright (C) 2004 IP Infusion, Inc. All Rights Reserved. */

#ifndef _HAL_IF_H_
#define _HAL_IF_H_

/* Port Mirroring direction enumeration. */
enum hal_port_mirror_direction
  {
    HAL_PORT_MIRROR_DIRECTION_RECEIVE      = (1 << 0),
    HAL_PORT_MIRROR_DIRECTION_TRANSMIT     = (1 << 1),
    HAL_PORT_MIRROR_DIRECTION_BOTH         = (HAL_PORT_MIRROR_DIRECTION_RECEIVE | HAL_PORT_MIRROR_DIRECTION_TRANSMIT)
  };


typedef union {
        u_int8_t  c[8];
        u_int16_t s[4];
        u_int32_t l[2];
} ut_int64_t;      /* 64 bit unsigned integer */


/*
   Interface counters.
 */
struct  hal_if_counters
  {
    ut_int64_t good_octets_rcv;
    ut_int64_t bad_octets_rcv;
    ut_int64_t mac_transmit_err;
    ut_int64_t good_pkts_rcv;
    ut_int64_t bad_pkts_rcv;
    ut_int64_t brdc_pkts_rcv;
    ut_int64_t mc_pkts_rcv;
    ut_int64_t pkts_64_octets;
    ut_int64_t pkts_65_127_octets;
    ut_int64_t pkts_128_255_octets;
    ut_int64_t pkts_256_511_octets;
    ut_int64_t pkts_512_1023_octets;
    ut_int64_t pkts_1024_max_octets;
    ut_int64_t good_octets_sent;
    ut_int64_t good_pkts_sent;
    ut_int64_t excessive_collisions;
    ut_int64_t mc_pkts_sent;
    ut_int64_t brdc_pkts_sent;
    ut_int64_t unrecog_mac_cntr_rcv;
    ut_int64_t fc_sent;
    ut_int64_t good_fc_rcv;
    ut_int64_t drop_events;
    ut_int64_t undersize_pkts;
    ut_int64_t fragments_pkts;
    ut_int64_t oversize_pkts;
    ut_int64_t jabber_pkts;
    ut_int64_t mac_rcv_error;
    ut_int64_t bad_crc;
    ut_int64_t collisions;
    ut_int64_t late_collisions;
    ut_int64_t bad_fc_rcv;
  };

struct hal_msg_if_stat
{
   struct hal_if_counters cntrs;
   unsigned int ifindex;
};
/* 
   Port type. 
*/
enum hal_if_port_type
  {
    HAL_IF_SWITCH_PORT,
    HAL_IF_ROUTER_PORT
  };

/* 
   Name: hal_if_get_metric 

   Description: 
   This API gets the metric for a interface.

   Parameters:
   IN -> ifname - interface name
   IN -> ifindex - interface ifindex
   OUT -> metric - metric

   Returns:
   < 0 on error 
   HAL_SUCCESS
*/
int
hal_if_get_metric (char *ifname, unsigned int ifindex, int *metric);

/* 
   Name: hal_if_get_mtu 

   Description:
   This API get the mtu for a interface.

   Parameters:
   IN -> ifname - interface name
   IN -> ifindex - interface index
   OUT -> mtu - mtu

   Returns:
   < 0 on error
   HAL_SUCCESS
*/
int
hal_if_get_mtu (char *ifname, unsigned int ifindex, int *metric);

/* 
   Name: hal_if_set_mtu

   Description:
   This API set the MTU for a interface.

   Parameters:
   IN -> ifname - interface name
   IN -> ifindex - interface index
   IN -> mtu - mtu

   Returns:
   < 0 on error
   HAL_SUCCESS
*/
int
hal_if_set_mtu (char *ifname, unsigned int ifindex, int mtu);

/*
   Name: hal_if_get_arp_ageing_timeout

   Description:
   This API set the arp ageing timeout for a interface.

   Parameters:
   IN -> ifname - interface name
   IN -> ifindex - interface index
   IN -> arp_ageing_timeout - arp ageing timeout value

   Returns:
   < 0 on error
   HAL_SUCCESS
*/
int
hal_if_set_arp_ageing_timeout (char *ifname, unsigned int ifindex, int arp_ageing_timeout);

/*
   Name: hal_if_get_arp_ageing_timeout

   Description:
   This API get the arp ageing timeout for a interface.

   Parameters:
   IN -> ifname - interface name
   IN -> ifindex - interface index
   OUT -> arp_ageing_timeout - arp ageing timeout value

   Returns:
   < 0 on error
   HAL_SUCCESS
*/
int
hal_if_get_arp_ageing_timeout (char *ifname, unsigned int ifindex, int *arp_ageing_timeout);

/*
   Name: hal_if_set_duplex

   Description:
   This API set the DUPLEX for a interface.

   Parameters:
   IN -> ifname - interface name
   IN -> ifindex - interface index
   IN -> duplex - duplex

   Returns:
   < 0 on error
   HAL_SUCCESS
*/
int
hal_if_set_duplex (char *ifname, unsigned int ifindex, int duplex);

/* 
   Name: hal_if_get_duplex 

   Description:
   This API get the duplex for a interface.

   Parameters:
   IN -> ifname - interface name
   IN -> ifindex - interface index
   OUT -> duplex - duplex

   Returns:
   < 0 on error
   HAL_SUCCESS
*/
int
hal_if_get_duplex (char *ifname, unsigned int ifindex, int *duplex);

/* 
   Name: hal_if_get_bw

   Description:
   This API gets the bandwidth for the interface. This API should
   return the value in bytes per second.

   Parameters:
   IN -> ifname - interface name
   IN -> ifindex - interface ifindex
   OUT -> bandwidth - interface bandwidth

   Returns:
   < 0 for error
   HAL_SUCCESS
*/
int
hal_if_get_bw (char *ifname, unsigned int ifindex, unsigned int *bandwidth);

/*
   Name: hal_if_set_bw

   Description:
   This API set the bandwidth for a interface.

   Parameters:
   IN -> ifname - interface name
   IN -> ifindex - interface index
   IN -> bandwidth - bandwidth

   Returns:
   < 0 on error
   HAL_SUCCESS
*/
int
hal_if_set_bw (char *ifname, unsigned int ifindex, unsigned int bandwidth);

/*
   Name: hal_if_set_autonego

   Description:
   This API set the DUPLEX with auto-negotiate for a interface.

   Parameters:
   IN -> ifname - interface name
   IN -> ifindex - interface index
   IN -> autonego - autonego 

   Returns:
   < 0 on error
   HAL_SUCCESS
*/
int
hal_if_set_autonego (char *ifname, unsigned int ifindex, int autonego);

/*
  Name: hal_if_get_hwaddr

  Description:
  This API gets the hardware address for a interface. This is the MAC 
  address in case of ethernet. The caller has to provide a buffer large
  enough to hold the address.

  Parameters:
  IN -> ifname - interface name
  IN -> ifindex - interface ifindex
  OUT -> hwaddr - hardware address
  OUT -> hwaddr_len - hardware address length

  Returns:
  < 0 on error
  HAL_SUCCESS
*/
int
hal_if_get_hwaddr (char *ifname, unsigned int ifindex, 
		   unsigned char *hwaddr, int *hwaddr_len);

/*
  Name: hal_if_set_hwaddr

  Description:
  This API sets the hardware address for a interface. This is the MAC 
  address in case of ethernet.

  Parameters:
  IN -> ifname - interface name
  IN -> ifindex - interface ifindex
  IN -> hwaddr - hardware address
  IN -> hwlen - hardware address length

  Returns:
  < 0 on error
  HAL_SUCCESS
*/
int
hal_if_set_hwaddr (char *ifname, unsigned int ifindex,
		   unsigned char *hwaddr, int hwlen);

/*
  Name: hal_if_sec_hwaddrs_set

  Description:
  This API sets the list of secondary MAC addresses for a 
  interface.

  Parameters:
  IN -> ifname  - interface name
  IN -> ifindex - interface index
  IN -> hw_addr_len - length of address
  IN -> nAddr s - number of MAC addresses
  IN -> addresses - array of MAC addresses

  Returns:
  < 0 on error
  HAL_SUCCESS;
*/
int
hal_if_sec_hwaddrs_set (char *ifname, unsigned int ifindex,
			int hw_addr_len, int nAddrs, unsigned char **addresses);
  
/*
  Name: hal_if_sec_hwaddrs_add

  Description:
  This API adds the secondary hardware addresses to the list of MAC addresses
  for a interface.

  Parameters:
  IN -> ifname  - interface name
  IN -> ifindex - interface index
  IN -> hw_addr_len - length of address
  IN -> nAddr s - number of MAC addresses
  IN -> addresses - array of MAC addresses

  Returns:
  < 0 on error
  HAL_SUCCESS;
*/
int
hal_if_sec_hwaddrs_add (char *ifname, unsigned int ifindex,
			int hw_addr_len, int nAddrs, unsigned char **addresses);

/*
  Name: hal_if_sec_hwaddrs_delete

  Description:
  This API deletes the secondary hardware addresses from the
  list of receive MAC addresses for a interface.

  Parameters:
  IN -> ifname  - interface name
  IN -> ifindex - interface index
  IN -> hw_addr_len - length of address
  IN -> nAddr s - number of MAC addresses
  IN -> addresses - array of MAC addresses

  Returns:
  < 0 on error
  HAL_SUCCESS;
*/
int
hal_if_sec_hwaddrs__delete (char *ifname, unsigned int ifindex,
			    int hw_addr_len, int nAddrs, unsigned char **addresses);
  
/* 
   Name: hal_if_flags_get

   Description:
   This API gets the flags for a interface. The flags are
   IFF_RUNNING
   IFF_UP
   IFF_BROADCAST
   IFF_LOOPBACK

   Parameters:
   IN -> ifname - interface name
   IN -> ifindex - interface ifindex
   OUT -> flags - flags
   
   Returns:
   < 0 on error
   HAL_SUCCES
*/
int
hal_if_flags_get (char *ifname, unsigned int ifindex, unsigned int *flags);

/*
  Name: hal_if_flags_set
  
  Description:
  This API sets the flags for a interface. The flags are
  IFF_RUNNING
  IFF_UP
  IFF_BROADCAST
  IFF_LOOPBACK
  
   Parameters:
   IN -> ifname - interface name
   IN -> ifindex - interface ifindex
   IN -> flags - flags to set

   Returns:
   < 0 for error
   HAL_SUCCESS
*/
int
hal_if_flags_set (char *ifname, unsigned int ifindex, unsigned int flags);

/* 
   Name: hal_if_flags_unset

   Description:
   This API unsets the flags for a interface. The flags are
   IFF_RUNNING
   IFF_UP
   IFF_BROADCAST
   IFF_LOOPBACK
   
   Parameters:
   IN -> ifname - interface name
   IN -> ifindex - interface ifindex
   IN -> flags - flags to unset

   Returns:
   < 0 for error
   HAL_SUCCESS
*/
int
hal_if_flags_unset (char *ifname, unsigned int ifindex, unsigned int flags);

/*
  Name: hal_if_set_port_type 

  Description:
  This API set the port type i.e. ROUTER or SWITCH port for a interface.

  Parameters:
  IN -> name - name of the interface
  IN -> ifindex - ifindex
  IN -> type - the port type
  OUT -> retifindex - the ifindex of the new type of interface

  Returns:
  < 0 on error
  HAL_SUCCESS
*/
int
hal_if_set_port_type (char *ifname, unsigned int ifindex, 
		      enum hal_if_port_type type, unsigned int *retifindex);

#ifdef HAVE_L3
/* 
   Name: hal_if_bind_fib

   Description:
   This API is called to bind a interface to a FIB fib_id in the forwarding 
   plane. 

   Parameters:
   IN -> ifindex - ifindex of interface
   IN -> fib     - fib id

   Results:
   HAL_SUCCESS
   < 0 on error
*/
int hal_if_bind_fib (u_int32_t ifindex, u_int32_t fib);

/* 
   Name: hal_if_unbind_fib

   Description:
   This API is called to unbind an interface from  FIB fib_id in the forwarding 
   plane. 

   Parameters:
   IN -> ifindex - ifindex of interface
   IN -> fib     - fib id

   Results:
   HAL_SUCCESS
   < 0 on error
*/
int hal_if_unbind_fib (u_int32_t ifindex, u_int32_t fib);
#endif /* HAVE_L3 */

/*
  Name: hal_if_svi_create

  Description:
  This API creates a SVI(Switch Virtual Interface) for a specific VLAN. The
  the VLAN information is embedded in the name of the interface.

  Parameters:
  IN -> name - interface name
  OUT -> ifindex - ifindex for the interface

  Returns:
  < 0 on error
  HAL_SUCCESS
*/
int
hal_if_svi_create (char *ifname, unsigned int *ifindex);

/*
  Name: hal_if_svi_delete

  Description:
  This API deletes the SVI(Switch Virtual Interface) for a specific VLAN. 

  Parameters:
  IN -> name - interface name
  IN -> ifindex - ifindex

  Returns:
  < 0 on error
  HAL_SUCCESS
*/
int
hal_if_svi_delete (char *ifname, unsigned int ifindex);

/* 
   Name: hal_if_get_counters

   Description: This API gets interface statistics for specific  
   ifindex.

   Parameters:
   IN -> ifindex - Interface index.
   OUT ->if_stats - the array of counters for interface. 

   Returns:
  Returns:
  < 0 on error
  HAL_SUCCESS
*/
int
hal_if_get_counters(unsigned int ifindex, struct hal_if_counters *if_stats);

/*
   Name: hal_if_get_list
                                                                                                                             
   Description:
   This API gets the list of interfaces from the interface manager.
                                                                                                                             
   Parameters:
   None
                                                                                                                             
   Returns:
   < 0 on error
   HAL_SUCCESS
*/
int
hal_if_get_list (void);
#define HAL_IP_ACL_NAME_SIZE               20
#define HAL_IP_MAX_ACL_FILTER              30

#define HAL_IP_ACL_DIRECTION_INGRESS       1   /* Incoming */
#define HAL_IP_ACL_DIRECTION_EGRESS        2   /* Outgoing */


/* Definition for ip access group structure for communicating with HSL */
struct hal_filter_common
{
  /* Common access-list */
  int extended;
  u_int32_t  addr;
  u_int32_t addr_mask;
  u_int32_t mask;
  u_int32_t mask_mask;
};


struct hal_ip_filter_list
{
    int type;    /* permit/deny*/
    union 
    {
	struct hal_filter_common     ipfilter;
    }ace;
};

struct hal_ip_access_grp
{
    char name[HAL_IP_ACL_NAME_SIZE];
    int ace_number;
    struct hal_ip_filter_list hfilter[HAL_IP_MAX_ACL_FILTER];
};

int
hal_ip_set_access_group (struct hal_ip_access_grp access_grp,
                         char *ifname, int action, int dir);

int
hal_ip_set_access_group_interface (struct hal_ip_access_grp access_grp,
                                   char *vifname, char *ifname,
                                   int action, int dir);

#endif /* _HAL_IF_H_ */
