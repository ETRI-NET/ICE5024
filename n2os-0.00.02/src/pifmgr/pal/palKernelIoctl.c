/* Copyright (C) 2001-2003 IP Infusion, Inc. All Rights Reserved. */

#include "pal.h"

#include <string.h>

/*
#include "sockunion.h"
#include "log.h"
#include "prefix.h"
#include "hash.h"
#include "linklist.h"
#include "nsm/nsm_interface.h"
#include "linklist.h"
#include "nsm/nsm_connected.h"
#include "nsm/nsmd.h"
#include "nsm/rib.h"
#include "nsm/nsm_rt.h"
*/

#if defined (SIOCGMIIPHY) && defined (SIOCGMIIREG)
extern int get_link_status_using_mii (char *ifname);
#endif /* SIOCGMIIPHY && SIOCGMIIREG */

/* clear and set interface name string */
void
ifreq_set_name (struct ifreq *ifreq, InterfaceT *ifp)
{
  strncpy (ifreq->ifr_name, ifp->name, IFNAMSIZ);
}

/* call ioctl system call */
int
if_ioctl (int request, caddr_t buffer)
{
  int sock;
  int ret = 0;
  int err = 0;

  sock = socket (AF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
  {
      perror ("socket");
  }

  ret = ioctl (sock, request, buffer);
  if (ret < 0)
  {
	  printf("if_ioctl: ioctl error errno %d \n", errno);
      err = errno;
  }
  close (sock);
  
  if (ret < 0) 
  {
      errno = err;
      return ret;
  }
  return 0;
}


#if 0
/* Call IPNET VR ioctl system call */
int
if_vr_ioctl (int request, caddr_t buffer, unsigned long fib_id)
{
  int sock;
  int ret = 0;
  int err = 0;

  sock = socket (AF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
    {
      perror ("socket");
      return -1;
    }

  if (setsockopt(sock, IP_SOL_SOCKET, IP_SO_X_VR, &fib_id, sizeof(fib_id)) < 0)
    {
        perror("setsockopt(SO_VR) failed");
        return -1;
    }

  ret = ioctl (sock, request, buffer);
  if (ret < 0)
    {
      err = errno;
    }
  close (sock);

  if (ret < 0)
    {
      errno = err;
      return ret;
    }
  return 0;
}
#endif


#ifdef HAVE_IPV6
int
if_ioctl_ipv6 (int request, caddr_t buffer)
{
  int sock;
  int ret = 0;
  int err = 0;

  sock = socket (AF_INET6, SOCK_DGRAM, 0);
  if (sock < 0)
    {
      perror ("socket");

  ret = ioctl (sock, request, buffer);
  if (ret < 0)
    {
      err = errno;
    }
  close (sock);

  if (ret < 0)
    {
      errno = err;
      return ret;
    }
  return 0;
}
#endif /* HAVE_IPV6 */


/* Interface looking up using infamous SIOCGIFCONF. */
int
interface_list_ioctl (int arbiter)
{
  int ret;
  int sock;
#define IFNUM_BASE 32
  int ifnum;
  struct ifreq *ifreq;
  struct ifconf ifconf;
  InterfaceT *ifp;
  int n;
  int update;

  /* Normally SIOCGIFCONF works with AF_INET socket. */
  sock = socket (AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) 
    {
      klog (KER_DEBUG, "Can't make AF_INET socket stream: %s", strerror (errno));
      return -1;
    }

  /* Set initial ifreq count.  This will be double when SIOCGIFCONF
     fail.  Solaris has SIOCGIFNUM. */
#ifdef SIOCGIFNUM
  ret = ioctl (sock, SIOCGIFNUM, &ifnum);
  if (ret < 0)
    ifnum = IFNUM_BASE;
  else
    ifnum++;
#else
  ifnum = IFNUM_BASE;
#endif /* SIOCGIFNUM */

  ifconf.ifc_buf = NULL;

  /* Loop until SIOCGIFCONF success. */
  for (;;) 
    {
      ifconf.ifc_len = sizeof (struct ifreq) * ifnum;
      ifconf.ifc_buf = NNMALLOC (MEM_IF, ifconf.ifc_len);

      ret = ioctl(sock, SIOCGIFCONF, &ifconf);
      if (ret < 0) 
	{
	  klog (KER_DEBUG, "SIOCGIFCONF: %s", strerror(errno));
	  goto end;
	}
      /* When length is same as we prepared, assume it overflowed and
         try again */
      if (ifconf.ifc_len == sizeof (struct ifreq) * ifnum) 
	{
	  ifnum += 10;
	  continue;
	}
      /* Success. */
      break;
    }

  /* Allocate interface. */
  ifreq = ifconf.ifc_req;

  for (n = 0; n < ifconf.ifc_len; n += sizeof(struct ifreq))
  {
      update = 0;
      ifp = ifLookupByName (ifreq->ifr_name);
      if (ifp == NULL)
      {
         ifp = ifGetByName (ifreq->ifr_name);
          update = 1;
      }

      if(arbiter)
         SET_FLAG (ifp->status, INTERFACE_ARBITER);

      if (update)
         kernelEventIfAdd (ifp);
      ifreq++;
  }
 end:
  close (sock);
  /*XFREE (MTYPE_TMP, ifconf.ifc_buf);*/
  NNFREE (MEM_IF, ifconf.ifc_buf);

  return ret;
}

/* Get interface's index by ioctl. */
result_t
kernel_if_get_index(InterfaceT *ifp)
{
  static int if_fake_index = 1;

#ifdef HAVE_BROKEN_ALIASES
  /* Linux 2.2.X does not provide individual interface index for aliases. */
  ifp->ifIndex = if_fake_index++;
  return RESULT_OK;
#else
#ifdef SIOCGIFINDEX
  int ret;
  struct ifreq ifreq;

  ifreq_set_name (&ifreq, ifp);

  ret = if_ioctl (SIOCGIFINDEX, (caddr_t) &ifreq);
  if (ret < 0)
    {
      /* Linux 2.0.X does not have interface index. */
      ifp->ifIndex = if_fake_index++;
      return RESULT_OK;
    }

  /* OK we got interface index. */
#ifdef ifr_ifindex
  ifp->ifIndex = ifreq.ifr_ifindex;
#else
  ifp->ifIndex = ifreq.ifr_index;
#endif
  return RESULT_OK;

#else
  ifp->ifIndex = if_fake_index++;
  return RESULT_OK;
#endif /* SIOCGIFINDEX */
#endif /* HAVE_BROKEN_ALIASES */
}

/*
 * get interface metric
 *   -- if value is not avaliable set -1
 */
result_t
kernel_if_get_metric (InterfaceT *ifp)
{
#ifdef SIOCGIFMETRIC
  struct ifreq ifreq;

  ifreq_set_name (&ifreq, ifp);

  if (if_ioctl (SIOCGIFMETRIC, (caddr_t) &ifreq) < 0) 
    return -1;
  ifp->metric = ifreq.ifr_metric;
  if (ifp->metric == 0)
    ifp->metric = 1;
#else /* SIOCGIFMETRIC */
  ifp->metric = -1;
#endif /* SIOCGIFMETRIC */
  return RESULT_OK;
}

/* set interface MTU */
result_t
kernel_if_set_mtu (InterfaceT *ifp,
		       u_int32_t mtu_size)
{
  struct ifreq ifreq;

  ifreq_set_name (&ifreq, ifp);
  ifreq.ifr_mtu = mtu_size;

#if defined(SIOCSIFMTU)
  if (if_ioctl (SIOCSIFMTU, (caddr_t) & ifreq) < 0) 
  {
      klog (KER_DEBUG, "Can't set mtu by ioctl(SIOCSIFMTU)");
      return -1;
  }

  ifp->mtu = ifreq.ifr_mtu;
#else
  zlog (KER_DEBUG, NULL, ZLOG_INFO, "Can't set mtu on this system");
#endif

  return RESULT_OK;
}


/* get interface MTU */
result_t
kernel_if_get_mtu (InterfaceT *ifp)
{
  struct ifreq ifreq;

  ifreq_set_name (&ifreq, ifp);

#if defined(SIOCGIFMTU)
  if (if_ioctl (SIOCGIFMTU, (caddr_t) & ifreq) < 0) 
    {
      klog (KER_DEBUG, "Can't lookup mtu by ioctl(SIOCGIFMTU)");
      ifp->mtu = -1;
      return -1;
    }

  ifp->mtu = ifreq.ifr_mtu;
#else
  zlog (KER_DEBUG, NULL, ZLOG_INFO, "Can't lookup mtu on this system");
  ifp->mtu = -1;
#endif
  return RESULT_OK;
}


/* Get interface bandwidth. */
/*
result_t
pal_kernel_if_get_bw (InterfaceT *ifp)
{
  ifp->bandwidth = 0;
  return RESULT_OK;
}
*/


int
kernel_if_get_bw (InterfaceT *ifp)
{
  /* The following code seems to work when used with latest redHat Linux
   * versions. In older versions, it generates "Operation not supported"
   * error when used. In any error case, this function retuns a bandwidth
   * value of 0x00.
   */
  struct ethtool_cmd ethcmd;
  struct ifreq       ifreq;
  u_int32_t          err;

  ifp->bandwidth = 0;

  if (ifp->hwType != IF_TYPE_ETHERNET)
    return RESULT_OK;

  ifreq_set_name (&ifreq, ifp);
  ethcmd.cmd = ETHTOOL_GSET;
  ifreq.ifr_data = (caddr_t) &ethcmd;

#ifndef HAVE_IPNET
  err = if_ioctl (SIOCETHTOOL, (caddr_t) &ifreq);
#else
  err = if_vr_ioctl (SIOCETHTOOL, (caddr_t) &ifreq, ifp->vrf ? ifp->vrf->fib_id : 0);
#endif /* HAVE_IPNET */
  if (err < 0)
    {
       klog (KER_DEBUG, "%s \n", strerror(errno));
       return err;
    }

  switch (ethcmd.speed)
    {
    case SPEED_10:
      ifp->bandwidth =  10000000/8;
      klog (KER_DEBUG, "10 MB\n");
      break;
    case SPEED_100:
      ifp->bandwidth = 100000000/8;
      klog (KER_DEBUG, "100 MB \n");
      break;
    case SPEED_1000:
      ifp->bandwidth = 1000000000/8;
      klog (KER_DEBUG, "1000 MB \n");
      break;
    default:
      klog (KER_DEBUG,
     "ioctl() returned illegal value. Setting bandwidth to 0\n");
      break;
    }

  return RESULT_OK;
}


int
kernel_if_get_hwaddr (InterfaceT *ifp)
{
  int ret;
  struct ifreq ifreq;
  int i;

  strncpy (ifreq.ifr_name, ifp->name, IFNAMSIZ);
  ifreq.ifr_addr.sa_family = AF_INET;

  /* Fetch Hardware address if available. */
  ret = if_ioctl (SIOCGIFHWADDR, (caddr_t) &ifreq);
  if (ret < 0)
    ifp->hwAddrLen = 0;
  else
    {
      memcpy (ifp->hwAddr, ifreq.ifr_hwaddr.sa_data, 6);

      for (i = 0; i < 6; i++)
	if (ifp->hwAddr[i] != 0)
	  break;

      if (i == 6)
	ifp->hwAddrLen = 0;
      else
	ifp->hwAddrLen = 6;
    }
  return 0;
}

#include <ifaddrs.h>

/* get interface flags */
result_t
kernel_if_flags_get(InterfaceT *ifp)
{
  int ret;
  struct ifreq ifreq;

  ifreq_set_name (&ifreq, ifp);

#ifndef HAVE_IPNET
  ret = if_ioctl (SIOCGIFFLAGS, (caddr_t) &ifreq);
#else
  ret = if_vr_ioctl (IP_SIOCGIFFLAGS, (caddr_t) &ifreq, ifp->vrf ? ifp->vrf->fib_id : 0);
#endif /* HAVE_IPNET */
  if (ret < 0) 
    return -1;

  ifp->flags = ifreq.ifr_flags & 0x0000ffff;

#if defined (SIOCGMIIPHY) && defined (SIOCGMIIREG)
  int link_status = 0;
  if (ifp->hwType == IF_TYPE_ETHERNET)
    if (CHECK_FLAG (ifp->flags, IFF_UP))
      {
	link_status = get_link_status_using_mii ((char*)ifp->name);
	if (link_status == 1)
	  ifp->flags |= IFF_RUNNING;
	else if (link_status == 0)
	  ifp->flags &= ~(IFF_UP|IFF_RUNNING);
      }
#endif /* SIOCGMIIPHY && SIOCGMIIREG */

  return RESULT_OK;
}

/* Set interface flags */
result_t
kernel_if_flags_set (InterfaceT *ifp, 
			 u_int32_t flag)
{
  int ret;
  struct ifreq ifreq;

  ifreq_set_name (&ifreq, ifp);

  ifreq.ifr_flags = ifp->flags;
  ifreq.ifr_flags |= flag;

  klog(KER_DEBUG, "%s: set if flags %08x\n", __FUNCTION__, ifreq.ifr_flags);

  ret = if_ioctl (SIOCSIFFLAGS, (caddr_t) &ifreq);

  if (ret < 0)
    {
      klog(KER_DEBUG, "error(%d) can't set interface flags \n", ret);
      return ret;
    }
  return RESULT_OK;
}

/* Unset interface's flag. */
result_t
kernel_if_flags_unset (InterfaceT *ifp, 
			   u_int32_t flag)
{
  int ret;
  struct ifreq ifreq;

  ifreq_set_name (&ifreq, ifp);

  ifreq.ifr_flags = ifp->flags;
  ifreq.ifr_flags &= ~flag;

  klog(KER_DEBUG, "%s: set if flags %08x\n", __FUNCTION__, ifreq.ifr_flags);

  ret = if_ioctl (SIOCSIFFLAGS, (caddr_t) &ifreq);

  if (ret < 0)
    {
      klog (KER_DEBUG, "error(%d) can't unset interface flags \n", ret);
      return ret;
    }
  return RESULT_OK;
}

/* Set interface proxy arp. */
result_t 
kernel_if_set_proxy_arp (InterfaceT *ifp, u_int32_t proxy_arp)
{
  return RESULT_OK;
}

#if 0
int
if_getaddrs (int arbiter)
{
  int ret;
  struct ifaddrs *ifap;
  struct ifaddrs *ifapfree;
  InterfaceT *ifp;
  int prefixlen;
  ConnectedT *ifc;

  ret = getifaddrs (&ifap); 
  if (ret != 0)
    {
      klog (KER_DEBUG, "getifaddrs(): %s", strerror (errno));
      return -1;
    }

  for (ifapfree = ifap; ifap; ifap = ifap->ifa_next)
    {
      ifp = ifGetByName (ifap->ifa_name);
      if (ifp == NULL)
	{
	  klog (KER_DEBUG, "if_getaddrs(): Can't lookup interface %s\n",
	    ifap->ifa_name);
	  continue;
	}

      if (ifap->ifa_addr->sa_family == AF_INET)
	{
	  struct sockaddr_in *addr;
	  struct sockaddr_in *mask;
	  struct sockaddr_in *dest;
	  struct in_addr *dest_pnt;

	  addr = (struct sockaddr_in *) ifap->ifa_addr;
	  mask = (struct sockaddr_in *) ifap->ifa_netmask;
	  prefixlen = ip_masklen (mask->sin_addr);

	  dest_pnt = NULL;

	  if (ifap->ifa_flags & IFF_POINTOPOINT) 
	    {
	      dest = (struct sockaddr_in *) ifap->ifa_dstaddr;
	      dest_pnt = &dest->sin_addr;
	    }

	  if (ifap->ifa_flags & IFF_BROADCAST)
	    {
	      dest = (struct sockaddr_in *) ifap->ifa_broadaddr;
	      dest_pnt = &dest->sin_addr;
	    }

	  ifc = nsm_connected_add_ipv4 (ifp, 0, &addr->sin_addr,
					prefixlen, dest_pnt, NULL);
	  if (ifc != NULL)
	    if (arbiter)
	      SET_FLAG (ifc->conf, INTERFACE_ARBITER);
	}
#ifdef HAVE_IPV6
      IF_NSM_CAP_HAVE_IPV6
	{
	  if (ifap->ifa_addr->sa_family == AF_INET6)
	    {
	      struct sockaddr_in6 *addr;
	      struct sockaddr_in6 *mask;
	      struct sockaddr_in6 *dest;
	      struct in6_addr *dest_pnt;

	      addr = (struct sockaddr_in6 *) ifap->ifa_addr;
	      mask = (struct sockaddr_in6 *) ifap->ifa_netmask;
	      prefixlen = ip6_masklen (mask->sin6_addr);

	      dest_pnt = NULL;

	      if (ifap->ifa_flags & IFF_POINTOPOINT) 
		{
		  if (ifap->ifa_dstaddr)
		    {
		      dest = (struct sockaddr_in6 *) ifap->ifa_dstaddr;
		      dest_pnt = &dest->sin6_addr;
		    }
		}

	      if (ifap->ifa_flags & IFF_BROADCAST)
		{
		  if (ifap->ifa_broadaddr)
		    {
		      dest = (struct sockaddr_in6 *) ifap->ifa_broadaddr;
		      dest_pnt = &dest->sin6_addr;
		    }
		}

	      ifc = nsm_connected_add_ipv6 (ifp, &addr->sin6_addr,
					    prefixlen, dest_pnt);
	      if (ifc != NULL)
		if (arbiter)
		  SET_FLAG (ifc->conf, INTERFACE_ARBITER);
	    }
	}
#endif /* HAVE_IPV6 */
    }

  freeifaddrs (ifapfree);

  return 0; 
}
#endif

/* Interface address lookup by ioctl.  This function only looks up
   IPv4 address. */

#if 0
int
if_get_addr (InterfaceT *ifp, int arbiter)
{
  int ret;
  struct ifreq ifreq;
  struct sockaddr_in addr;
  struct sockaddr_in mask;
  struct sockaddr_in dest;
  struct in_addr *dest_pnt;
  u_char prefixlen;
  ConnectedT *ifc;

  /* Interface's name and address family. */
  strncpy (ifreq.ifr_name, ifp->name, IFNAMSIZ);
  ifreq.ifr_addr.sa_family = AF_INET;

  /* Interface's address. */
  ret = if_ioctl (SIOCGIFADDR, (caddr_t) &ifreq);
  if (ret < 0) 
    {
      if (errno != EADDRNOTAVAIL)
	{
	  klog (KER_DEBUG, "SIOCGIFADDR fail: %s", strerror (errno));
	  return ret;
	}
      return 0;
    }
  memcpy (&addr, &ifreq.ifr_addr, sizeof (struct sockaddr_in));

  /* Interface's network mask. */
  ret = if_ioctl (SIOCGIFNETMASK, (caddr_t) &ifreq);
  if (ret < 0) 
    {
      if (errno != EADDRNOTAVAIL) 
	{
	  klog (KER_DEBUG, "SIOCGIFNETMASK fail: %s", strerror (errno));
	  return ret;
	}
      return 0;
    }
#ifdef ifr_netmask
  memcpy (&mask, &ifreq.ifr_netmask, sizeof (struct sockaddr_in));
#else
  memcpy (&mask, &ifreq.ifr_addr, sizeof (struct sockaddr_in));
#endif /* ifr_netmask */
  prefixlen = ip_masklen (mask.sin_addr);

  /* Point to point or borad cast address pointer init. */
  dest_pnt = NULL;

  if (ifp->flags & IFF_POINTOPOINT) 
    {
      ret = if_ioctl (SIOCGIFDSTADDR, (caddr_t) &ifreq);
      if (ret < 0) 
	{
	  if (errno != EADDRNOTAVAIL) 
	    {
	      klog (KER_DEBUG, "SIOCGIFDSTADDR fail: %s", strerror (errno));
	      return ret;
	    }
	  return 0;
	}
      memcpy (&dest, &ifreq.ifr_dstaddr, sizeof (struct sockaddr_in));
      dest_pnt = &dest.sin_addr;
    }
  if (ifp->flags & IFF_BROADCAST)
    {
      ret = if_ioctl (SIOCGIFBRDADDR, (caddr_t) &ifreq);
      if (ret < 0) 
	{
	  if (errno != EADDRNOTAVAIL) 
	    {
	      klog (KER_DEBUG, "SIOCGIFBRDADDR fail: %s", strerror (errno));
	      return ret;
	    }
	  return 0;
	}
      memcpy (&dest, &ifreq.ifr_broadaddr, sizeof (struct sockaddr_in));
      dest_pnt = &dest.sin_addr;
    }


  /* Set address to the interface. */
  ifc = nsm_connected_add_ipv4 (ifp, 0, &addr.sin_addr,
				prefixlen, dest_pnt, NULL);
  if (ifc != NULL)
    if (arbiter)
      SET_FLAG (ifc->conf, INTERFACE_ARBITER);

  return 0;
}
#endif /* Have GETADDR */


/* Fetch interface information via ioctl(). */
#include "nnList.h"
extern ListT * pIfList;
int if_get_addr (InterfaceT *ifp, int arbiter);

static void interface_info_ioctl (int arbiter)
{
  InterfaceT *ifp = NULL;
  ListNodeT *pNode;
 
  for(pNode = pIfList->pHead; pNode!=NULL; pNode=pNode->pNext)
  {
      kernel_if_get_index (ifp);
#ifdef SIOCGIFHWADDR
      kernel_if_get_hwaddr (ifp);
#endif /* SIOCGIFHWADDR */
      kernel_if_flags_get (ifp);
#ifndef HAVE_GETIFADDRS
      //if_get_addr (ifp, arbiter);
#endif /* ! HAVE_GETIFADDRS */
      kernel_if_get_mtu (ifp);
      kernel_if_get_metric (ifp);
      kernel_if_get_bw (ifp);

      /* Update interface information.  */
      kernelEventIfAdd (ifp);
  }
}

/* Lookup all interface information. */
result_t
kernel_if_scan(void)
{
  /* Linux can do both proc & ioctl, ioctl is the only way to get
     interface aliases in 2.2 series kernels. */
#ifdef HAVE_PROC_NET_DEV
  interface_list_proc ();
#endif /* HAVE_PROC_NET_DEV */
  interface_list_ioctl (0);

  /* After listing is done, get index, address, flags and other
     interface's information. */
  interface_info_ioctl (0);

#ifdef HAVE_GETIFADDRS
  if_getaddrs (0);
#endif /* HAVE_GETIFADDRS */

#if defined(HAVE_IPV6) && defined(HAVE_PROC_NET_IF_INET6)
  /* Linux provides interface's IPv6 address via
     /proc/net/if_inet6. */
   ifaddr_proc_ipv6 ();
#endif /* HAVE_IPV6 && HAVE_PROC_NET_IF_INET6 */
  return RESULT_OK;
}

void
pal_kernel_if_update  (void)
{
  /* Linux can do both proc & ioctl, ioctl is the only way to get
     interface aliases in 2.2 series kernels. */
#ifdef HAVE_PROC_NET_DEV
  interface_list_proc ();
#endif /* HAVE_PROC_NET_DEV */
  interface_list_ioctl (1);
  
  /* After listing is done, get index, address, flags and other
     interface's information. */
  interface_info_ioctl (1);

#ifdef HAVE_GETIFADDRS
  if_getaddrs (1);
#endif /* HAVE_GETIFADDRS */

#if defined(HAVE_IPV6) && defined(HAVE_PROC_NET_IF_INET6)
  /* Linux provides interface's IPv6 address via
     /proc/net/if_inet6. */
  IF_NSM_CAP_HAVE_IPV6
    {
      ifaddr_proc_ipv6 ();
    }
#endif /* HAVE_IPV6 && HAVE_PROC_NET_IF_INET6 */
}

#ifdef HAVE_IPV6
int
if_ioctl_ipv6 (int request, caddr_t buffer)
{
  int sock;
  int ret = 0;
  int err = 0;

  sock = socket (AF_INET6, SOCK_DGRAM, 0);
  if (sock < 0)
    {
      perror ("socket");
    }

  ret = ioctl (sock, request, buffer);
  if (ret < 0)
    {
      err = errno;
    }
  close (sock);
  
  if (ret < 0) 
    {
      errno = err;
      return ret;
    }
  return 0;
}
#endif /* HAVE_IPV6 */


#if 0
/* Set up interface's address, netmask (and broadcas? ).  Linux or
   Solaris uses ifname:number semantics to set IP address aliases. */
result_t
kernel_if_ipv4_address_add (InterfaceT *ifp, 
				ConnectedT *ifc)
{
  int ret;
  struct ifreq ifreq;
  struct sockaddr_in addr;
  struct sockaddr_in broad;
  struct sockaddr_in mask;
  struct prefix_ipv4 ifaddr;
  struct prefix_ipv4 *p;

  p = (struct prefix_ipv4 *) ifc->address;

  ifaddr = *p;

  ifreq_set_name (&ifreq, ifp);

  addr.sin_addr = p->prefix;
  addr.sin_family = p->family;
  memcpy (&ifreq.ifr_addr, &addr, sizeof (struct sockaddr_in));
  ret = if_ioctl (SIOCSIFADDR, (caddr_t) &ifreq);
  if (ret < 0)
    return ret;
  
  /* We need mask for make broadcast addr. */
  masklen2ip (p->prefixlen, &mask.sin_addr);

  if (if_is_broadcast (ifp))
    {
      apply_mask_ipv4 (&ifaddr);
      addr.sin_addr = ifaddr.prefix;

      broad.sin_addr.s_addr = (addr.sin_addr.s_addr | ~mask.sin_addr.s_addr);
      broad.sin_family = p->family;

      memcpy (&ifreq.ifr_broadaddr, &broad, sizeof (struct sockaddr_in));
      ret = if_ioctl (SIOCSIFBRDADDR, (caddr_t) &ifreq);
      if (ret < 0)
	return ret;
    }

  mask.sin_family = p->family;

  memcpy (&ifreq.ifr_netmask, &mask, sizeof (struct sockaddr_in));

  ret = if_ioctl (SIOCSIFNETMASK, (caddr_t) &ifreq);
  if (ret < 0)
    return ret;

  /* Linux version before 2.1.0 need to interface route setup. */
#if defined(GNU_LINUX) && LINUX_VERSION_CODE < 131328
  {
    apply_mask_ipv4 (&ifaddr);
    kernel_add_route (&ifaddr, NULL, ifp->ifIndex, 0, 0);
  }
#endif /* ! (GNU_LINUX && LINUX_VERSION_CODE) */

  return RESULT_OK;
}

/* Set up interface's address, netmask (and broadcas? ).  Linux or
   Solaris uses ifname:number semantics to set IP address aliases. */
result_t
kernel_if_ipv4_address_delete (InterfaceT *ifp, 
				   ConnectedT *ifc)
{
  int ret;
  struct ifreq ifreq;
  struct sockaddr_in addr;
  struct prefix_ipv4 *p;

  p = (struct prefix_ipv4 *) ifc->address;

  ifreq_set_name (&ifreq, ifp);

  memset (&addr, 0, sizeof (struct sockaddr_in));
  addr.sin_family = p->family;
  memcpy (&ifreq.ifr_addr, &addr, sizeof (struct sockaddr_in));
  ret = if_ioctl (SIOCSIFADDR, (caddr_t) &ifreq);
  if (ret < 0)
    return ret;

  return RESULT_OK;
}

#ifdef HAVE_IPV6

#ifdef LINUX_IPV6
#ifndef _LINUX_IN6_H
/* linux/include/net/ipv6.h */
struct in6_ifreq 
{
  struct in6_addr ifr6_addr;
  u_int32_t ifr6_prefixlen;
  int ifr6_ifindex;
};
#endif /* _LINUX_IN6_H */

/* Interface's address add/delete functions. */
result_t
kernel_if_ipv6_address_add (InterfaceT *ifp, ConnectedT *ifc)
{
  int ret;
  struct prefix_ipv6 *p;
  struct in6_ifreq ifreq;

  p = (struct prefix_ipv6 *) ifc->address;

  memset (&ifreq, 0, sizeof (struct in6_ifreq));

  memcpy (&ifreq.ifr6_addr, &p->prefix, sizeof (struct in6_addr));
  ifreq.ifr6_ifindex = ifp->ifIndex;
  ifreq.ifr6_prefixlen = p->prefixlen;

  ret = if_ioctl_ipv6 (SIOCSIFADDR, (caddr_t) &ifreq);

  if (ret == -1)
    return -1;
  else 
    return RESULT_OK;
}

result_t
kernel_if_ipv6_address_delete (InterfaceT *ifp, ConnectedT *ifc)
{
  int ret;
  struct prefix_ipv6 *p;
  struct in6_ifreq ifreq;

  p = (struct prefix_ipv6 *) ifc->address;

  memset (&ifreq, 0, sizeof (struct in6_ifreq));

  memcpy (&ifreq.ifr6_addr, &p->prefix, sizeof (struct in6_addr));
  ifreq.ifr6_ifindex = ifp->ifIndex;
  ifreq.ifr6_prefixlen = p->prefixlen;

  ret = if_ioctl_ipv6 (SIOCDIFADDR, (caddr_t) &ifreq);

  if (ret == -1)
    return -1;
  else 
    return RESULT_OK;
}
#else /* LINUX_IPV6 */

#endif /* LINUX_IPV6 */

#endif /* HAVE_IPV6 */

#endif /* end of #IF 0 */



