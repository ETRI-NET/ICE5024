/* Copyright (C) 2001-2003 IP Infusion, Inc. All Rights Reserved. */

#include "pal.h"
#include <errno.h>
#include <string.h>

/* Proc filesystem one line buffer. */
#define PROCBUFSIZ                  1024

/* Path to device proc file system. */
#ifndef _PATH_PROC_NET_DEV
#define _PATH_PROC_NET_DEV        "/proc/net/dev"
#endif /* _PATH_PROC_NET_DEV */

/* Return statistics data pointer. */
static char *
interface_name_cut (char *buf, char **name)
{
  char *stats;

  /* Skip white space.  Line will include header spaces. */
  while (*buf == ' ')
    buf++;
  *name = buf;

  /* Cut interface name. */
  stats = strrchr (buf, ':');
  *stats++ = '\0';

  return stats;
}

#ifdef HAVE_PROC_NET_DEV
/* Fetch each statistics field. */
static int
ifstat_dev_fields (int version, char *buf, struct interface *ifp)
{
  switch (version) 
    {
    case 3:
      sscanf(buf,
	     "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
	     &ifp->stats.rx_bytes,
	     &ifp->stats.rx_packets,
	     &ifp->stats.rx_errors,
	     &ifp->stats.rx_dropped,
	     &ifp->stats.rx_fifo_errors,
	     &ifp->stats.rx_frame_errors,
	     &ifp->stats.rx_compressed,
	     &ifp->stats.rx_multicast,
	     &ifp->stats.tx_bytes,
	     &ifp->stats.tx_packets,
	     &ifp->stats.tx_errors,
	     &ifp->stats.tx_dropped,
	     &ifp->stats.tx_fifo_errors,
	     &ifp->stats.collisions,
	     &ifp->stats.tx_carrier_errors,
	     &ifp->stats.tx_compressed);
      break;
    case 2:
      sscanf(buf, "%d %d %d %d %d %d %d %d %d %d %d %d %d",
	     &ifp->stats.rx_bytes,
	     &ifp->stats.rx_packets,
	     &ifp->stats.rx_errors,
	     &ifp->stats.rx_dropped,
	     &ifp->stats.rx_fifo_errors,
	     &ifp->stats.rx_frame_errors,
	     &ifp->stats.tx_bytes,
	     &ifp->stats.tx_packets,
	     &ifp->stats.tx_errors,
	     &ifp->stats.tx_dropped,
	     &ifp->stats.tx_fifo_errors,
	     &ifp->stats.collisions,
	     &ifp->stats.tx_carrier_errors);
      ifp->stats.rx_multicast = 0;
      break;
    case 1:
      sscanf(buf, "%d %d %d %d %d %d %d %d %d %d %d",
	     &ifp->stats.rx_packets,
	     &ifp->stats.rx_errors,
	     &ifp->stats.rx_dropped,
	     &ifp->stats.rx_fifo_errors,
	     &ifp->stats.rx_frame_errors,
	     &ifp->stats.tx_packets,
	     &ifp->stats.tx_errors,
	     &ifp->stats.tx_dropped,
	     &ifp->stats.tx_fifo_errors,
	     &ifp->stats.collisions,
	     &ifp->stats.tx_carrier_errors);
      ifp->stats.rx_bytes = 0;
      ifp->stats.tx_bytes = 0;
      ifp->stats.rx_multicast = 0;
      break;
    }
  return 0;
}
#endif

#ifdef HAVE_PROC_NET_DEV
/* Update interface's statistics. */
result_t
pal_if_stat_update ()
{
  FILE *fp;
  char buf[PROCBUFSIZ];
  int version;
  struct interface *ifp;
  char *stat;
  char *name;

  /* Open /proc/net/dev. */
  fp = fopen (_PATH_PROC_NET_DEV, "r");
  if (fp == NULL)
    {
      NNLOG (LOG_DEBUG, "Can't open proc file %s: %s",
		 _PATH_PROC_NET_DEV, strerror (errno));
      return -1;
    }

  /* Drop header lines. */
  fgets (buf, PROCBUFSIZ, fp);
  fgets (buf, PROCBUFSIZ, fp);

  /* To detect proc format veresion, parse second line. */
  if (strstr (buf, "compressed"))
    version = 3;
  else if (strstr (buf, "bytes"))
    version = 2;
  else
    version = 1;

  /* Update each interface's statistics. */
  while (fgets (buf, PROCBUFSIZ, fp) != NULL)
    {
      stat = interface_name_cut (buf, &name);
      ifp = ifg_get_by_name (&NSM_ZG->ifg, name);
      ifstat_dev_fields (version, stat, ifp);
    }

  fclose(fp);

  return 0;
}
#else
result_t
pal_if_stat_update(void)
{
  return 0;
}
#endif /* HAVE_PROC_NET_DEV */

/* Interface structure allocation by proc filesystem. */
int
interface_list_proc ()
{
  FILE *fp;
  char buf[PROCBUFSIZ];
  //struct interface *ifp;
  char *name;

  /* Open /proc/net/dev. */
  fp = fopen (_PATH_PROC_NET_DEV, "r");
  if (fp == NULL)
    {
      NNLOG (LOG_DEBUG, "Can't open proc file %s: %s",
		 _PATH_PROC_NET_DEV, strerror (errno));
      return -1;
    }

  /* Drop header lines. */
  fgets (buf, PROCBUFSIZ, fp);
  fgets (buf, PROCBUFSIZ, fp);

  /* Only allocate interface structure.  Other jobs will be done in
     if_ioctl.c. */
  while (fgets (buf, PROCBUFSIZ, fp) != NULL)
    {
      interface_name_cut (buf, &name);
      ifGetByName (name);
      //ifp = ifGetByName (name);
    }

  fclose (fp);

  return 0;
}

#if defined(HAVE_IPV6) && defined(HAVE_PROC_NET_IF_INET6)

#ifndef _PATH_PROC_NET_IF_INET6
#define _PATH_PROC_NET_IF_INET6          "/proc/net/if_inet6"
#endif /* _PATH_PROC_NET_IF_INET6 */

#include "nnPrefix.h"

int
ifaddr_proc_ipv6 ()
{
  FILE *fp;
  char buf[PROCBUFSIZ];
  int n;
  char addr[33];
  char ifname[20];
  int ifindex, plen, scope, status;
  struct interface *ifp;
  struct in6_addr p;

  /* Open proc file system. */
  fp = fopen (_PATH_PROC_NET_IF_INET6, "r");
  if (fp == NULL)
  {
      NNLOG (LOG_DEBUG, "Can't open proc file %s: %s",
		 _PATH_PROC_NET_IF_INET6, strerror (errno));
      return -1;
  }
  
  /* Get interface's IPv6 address. */
  while (fgets (buf, PROCBUFSIZ, fp) != NULL)
  {
      n = sscanf (buf, "%32s %02x %02x %02x %02x %20s", 
		  addr, &ifindex, &plen, &scope, &status, ifname);
      if (n != 6)
	continue;

      ifp = ifGetByName (ifname);

      /* Fetch interface's IPv6 address. */
      nnCnvStringtoAddr6 (&p, addr);
      plen;

      connectedAddIpv6 (ifp, 0, &p, plen, NULL, 0, 0);
  }

  fclose (fp);

  return 0;
}
#endif /* HAVE_IPV6 && HAVE_PROC_NET_IF_INET6 */
