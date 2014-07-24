/* Copyright (C) 2004 IP Infusion, Inc. All Rights Reserved.
  
LAYER 2 BRIDGE HAL
  
This module defines the platform abstraction layer to the 
Linux hal.

*/

#include <sys/ioctl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <net/route.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <errno.h>
#include <stdio.h>

#include "hal_incl.h"

static hal_sock_handle_t hal_sock = -1;
/* Generic ioctl handler */

int
hal_ioctl (unsigned long arg0, unsigned long arg1, unsigned long arg2,
          unsigned long arg3, unsigned long arg4, unsigned long arg5,
		  unsigned long arg6)
{
  unsigned long arg[6];
  
  arg[0] = arg0;
  arg[1] = arg1;
  arg[2] = arg2;
  arg[3] = arg3;
  arg[4] = arg4;
  arg[5] = arg5;
  arg[6] = arg6;

  printf ("***********************************\n"); 
  printf ("HAL_IOCTL:\n"); 
  printf ("hal_sock=%d, type=%d, cmd=%d\n", (int)hal_sock, SIOCPROTOPRIVATE, (int)arg0); 
  printf ("***********************************\n"); 

  return ioctl (hal_sock, SIOCPROTOPRIVATE, arg);
}

int
hal_ioctl_is_port_up(struct ifreq *ifr)
{
  return ioctl(hal_sock, SIOCGIFFLAGS, ifr);
}

/* Stubs for hal linux hal_init */
int
hal_init ()
{
  int errnum;
  
  hal_sock = socket (AF_HAL, SOCK_RAW, 0);
  errnum = errno;
  
  if (hal_sock < 0)
    {
	  printf("%d \n", errnum);
      printf ("***********************************\n"); 
      printf ("HAL_INIT: FAIL !!!\n"); 
      printf ("errno=%d\n", errno); 
      printf ("***********************************\n"); 
      return -errno;
    }
  printf ("***********************************\n"); 
  printf ("HAL_INIT: SUCCESS\n"); 
  printf ("hal_sock=%d\n", hal_sock); 
  printf ("***********************************\n"); 
  return hal_sock;
}
                                                                          
int
hal_deinit ()
{
  return close(hal_sock);
}

