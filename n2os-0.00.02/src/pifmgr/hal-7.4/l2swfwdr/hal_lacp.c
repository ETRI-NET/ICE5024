/* Copyright (C) 2003 IP Infusion, Inc. All Rights Reserved. */

/* LINK AGGREGATION */

/* This module defines the interface to the aggregators for LACP. */

#define HAVE_LACPD	1


#ifdef HAVE_LACPD
#include "hal_incl.h"
#include "if_lacp.h"

int 
hal_lacp_init ()
{
  return HAL_SUCCESS;
}


int
hal_lacp_deinit ()
{
   return HAL_SUCCESS;
}
  

/* This function adds an aggregator */

/*
extern int
hal_lacp_add_aggregator (char *name, u_char *mac_address, int agg_type)
*/
int
hal_lacp_add_aggregator (char *name, unsigned char *mac_address, int agg_type)
{
  int ret;
  ret = hal_ioctl (IPILACP_ADD_AGG, 
		    (long) name, (long) mac_address, (long) agg_type, 0, 0);
  if (ret < 0)
    return -ret;

  return HAL_SUCCESS;
}

/* This function deletes an aggregator */

int
hal_lacp_delete_aggregator (char *name, unsigned int ifindex)
{
  int ret;
  ret = hal_ioctl (IPILACP_DEL_AGG, (long) name, 0, 0, 0, 0);
  if (ret < 0)
    return -ret;
    /* return -errno; */
  return HAL_SUCCESS;
}


/* These functions are described in 43.4.9 */

int
hal_lacp_attach_mux_to_aggregator (char *name, unsigned int agg_ifindex,
				   char *port, unsigned int port_ifindex)
{
  int ret;

  ret = hal_ioctl (IPILACP_AGG_LINK, (long) name, (long)port, 0, 0, 0);
  if (ret < 0)
    return -ret;
  return HAL_SUCCESS;
}

int
hal_lacp_detach_mux_from_aggregator (char *name, unsigned int agg_ifindex,
				     char *port, unsigned int port_ifindex)
{
  int ret;

  ret = hal_ioctl (IPILACP_DEAGG_LINK, (long) name, (long)port, 0, 0, 0);
  if (ret < 0)
    return -ret;
  return HAL_SUCCESS;
}


int
hal_lacp_psc_set (unsigned int ifIndex, int psc)
{
  return HAL_SUCCESS;
}

int
hal_lacp_enable_collecting (const char * const name, const char * const port)
{
  return HAL_SUCCESS;
}



int
hal_lacp_disable_collecting (const char * const name, const char * const port)
{
  return HAL_SUCCESS;
}

int
hal_lacp_enable_distributing (const char * const name, const char * const port)
{
  return HAL_SUCCESS;
}

int
hal_lacp_disable_distributing (const char * const name, const char * const port)
{
  return HAL_SUCCESS;
}

int
hal_lacp_enable_collecting_distributing (const char * const name, const char * const port)
{
  return HAL_SUCCESS;
}

int
hal_lacp_disable_collecting_distributing (const char * const name, const char * const port)
{
  return HAL_SUCCESS;
}
  
#endif /* HAVE_LACPD */
