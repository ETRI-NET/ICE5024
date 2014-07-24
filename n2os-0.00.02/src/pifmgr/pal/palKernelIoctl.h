/* Copyright (C) 2002-2003 IP Infusion, Inc.  All Rights Reserved. */
#ifndef _PAL_KERNEL_IOCTL_DEF
#define _PAL_KERNEL_IOCTL_DEF

/* Functions.  */

/* Lookup all interface information. */ 
result_t kernel_if_scan(void);
/* Interface looking up using infamous SIOCGIFCONF. */
int interface_list_ioctl (int arbiter);
/* Fetch interface information via ioctl(). */ 

/* Get interface's index by ioctl. */ 
result_t kernel_if_get_index(InterfaceT *ifp);

/* get interface metric */
result_t kernel_if_get_metric (InterfaceT *ifp);

/* set interface MTU */ 
result_t kernel_if_set_mtu (InterfaceT *ifp, u_int32_t mtu_size);

/* get interface MTU */ 
result_t kernel_if_get_mtu (InterfaceT *ifp);

/* Get interface bandwidth. */
result_t kernel_if_get_bw (InterfaceT *ifp);

int kernel_if_get_hwaddr (InterfaceT *ifp);

 /* get interface flags */ 
result_t kernel_if_flags_get(InterfaceT *ifp);

/* Set interface flags */
result_t kernel_if_flags_set (InterfaceT *ifp,  u_int32_t flag);

/* Unset interface's flag. */
result_t kernel_if_flags_unset (InterfaceT *ifp, u_int32_t flag);

/* Scan interfaces by ioctl. */
int if_getaddrs (int arbiter);

/* Interface address lookup by ioctl. */
int if_get_addr (InterfaceT *ifp, int arbiter);

/* Add interface address */
/* currently, use netlink api */
//result_t kernel_if_ipv4_address_add (InterfaceT *ifp, ConnectedT *ifc);
//result_t kernel_if_ipv4_address_delete(InterfaceT *ifp, ConnectedT *ifc);

/* set interface proxy arp. (Empty)*/
result_t kernel_if_set_proxy_arp (InterfaceT *ifp, u_int32_t proxy_arp);

#endif /* _PAL_KERNEL_IOCTL_DEF */
