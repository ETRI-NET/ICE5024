/* Copyright (C) 2001-2003 IP Infusion, Inc. All Rights Reserved. */

#include "pal.h"
#include "palKernelIoctl.h"
#include "palKernelNetlink.h"

#ifndef SO_VR
#define SO_VR 0x1020
#endif /* SO_VR */


int32_t kernelStart (void)
{
  PAL_DBUG(0, "enter \n");
  /* Initialize rt_netlink socket */
  netlinkkernelInit();

  /* scan kernel interfaces */
  kernelIfScan();
  return 0;
}

int32_t kernelStop (void)
{
  netlinkkernelStop();
  return 0;
}

/* Interface information read by netlink. */
int32_t kernelIfScan (void)
{
  PAL_DBUG(0, "enter \n");
  netlinkUpdateIfAll ();
  return RESULT_OK;
}

int32_t kernelIfUpdate (void)
{
  netlinkUpdateIfAll ();
  return RESULT_OK;
}


int32_t kernelIfIpv4AddressAdd (InterfaceT *ifp, ConnectedT *ifc)
{
  PAL_DBUG(0, "enter \n");
  int ret;
  ret = netlinkAddressAddIpv4 (ifp, ifc);
  if (ret < 0)
    return -1;
  return RESULT_OK;
}

int32_t kernelIfIpv4AddressDelete (InterfaceT *ifp, ConnectedT *ifc)
{
  int ret;
  ret = netlinkAddressDeleteIpv4 (ifp, ifc);
  if (ret < 0)
    return -1;
  return RESULT_OK;
}

int32_t kernelIfIpv4AddressDeleteAll (InterfaceT *ifp)
{
  /* need to make list loop
  if (ifc != NULL)
  {
      if (ifc->next)
         kernelIfIpv4AddressDeleteAll (ifp, ifc->next);

	    // /usr/include/linux/in.h
      if (!IN_LOOPBACK (ntohl (ifc->address->u.prefix4.s_addr)))
         netlinkAddressDeleteIpv4 (ifp, ifc);
  }
  */
  return RESULT_OK;
}

#if 0
int32_t kernelIfIpv4AddressUpdate (InterfaceT *ifp,
				   ConnectedT *ifc_old,
				   ConnectedT *ifc_new)
{
  ConnectedT *ifc;

  /* Delete all addresses from Interface. */
  kernelIfIpv4AddressDeleteAll (ifp, ifp->ifc_ipv4);
  netlinkAddressDeleteIpv4 (ifp, ifc_old);

  /* Add all addresses to Interface. */
  netlinkAddressAddIpv4 (ifp, ifc_new);
  for (ifc = ifp->ifc_ipv4; ifc; ifc = ifc->next)
    netlinkAddressAddIpv4 (ifp, ifc);

  return RESULT_OK;
}
#endif 

int32_t kernelIfIpv4AddressSecondaryAdd (InterfaceT *ifp, ConnectedT *ifc)
{
  int ret;
  ret = netlinkAddressAddIpv4 (ifp, ifc);
  if (ret < 0)
    return -1;
  return RESULT_OK;
}

int32_t
kernelIfIpv4AddressSecondaryDelete (InterfaceT *ifp, ConnectedT *ifc)
{
  int ret;
  ret = netlinkAddressDeleteIpv4 (ifp, ifc);
  if (ret < 0)
    return -1;
  return RESULT_OK;
}

/* Ret interface proxy arp. */
int32_t
kernelIfSetProxyArp (InterfaceT *ifp,
                             u_int32_t proxy_arp)
{
  return 0;
}



int32_t kernelIfGetIndex(InterfaceT *ifp)
{
  return kernel_if_get_index(ifp);
}

int32_t kernelIfGetMetric (InterfaceT *ifp)
{
  return kernel_if_get_metric (ifp);
}

int32_t kernelIfSetMtu (InterfaceT *ifp, u_int32_t mtu_size)
{
  return kernel_if_set_mtu (ifp, mtu_size);
}

int32_t kernelIfGetMtn (InterfaceT *ifp)
{
  return kernel_if_get_mtu (ifp);
}

int32_t kernelIfGetBw (InterfaceT *ifp)
{
  return kernel_if_get_bw (ifp);
}

int32_t kernelIfGetHwaddr (InterfaceT *ifp)
{
  return kernel_if_get_hwaddr (ifp);
}

int32_t kernelIfFlagsGet(InterfaceT *ifp)
{
  return kernel_if_flags_get(ifp);
}

int32_t kernelIfFlagsSet (InterfaceT *ifp,  u_int32_t flag)
{
  return kernel_if_flags_set (ifp, flag);
}

int32_t kernelIfFlagsUnset (InterfaceT *ifp, u_int32_t flag)
{
  return kernel_if_flags_unset (ifp, flag);
}

int32_t kernelIfSetproxyArp (InterfaceT *ifp, u_int32_t proxy_arp)
{
  return kernel_if_set_proxy_arp (ifp, proxy_arp);
}

#include "pifFea.h"
void kernelEventIfAdd(InterfaceT *ifp)
{
	pif_fea_if_add(ifp);
}

void kernelEventIfDel(InterfaceT *ifp)
{
	pif_fea_if_del(ifp);
}

void kernelEventIfUp(InterfaceT *ifp)
{
	pif_fea_if_up(ifp);
}

void kernelEventIfDown(InterfaceT *ifp)
{
	pif_fea_if_down(ifp);
}

void kernelEventIfAddrAdd(InterfaceT *ifp, ConnectedT *ifc)
{
	pif_fea_if_addr_add(ifp, ifc);
}

void kernelEventIfAddrDel(InterfaceT *ifp, ConnectedT *ifc)
{
	pif_fea_if_addr_del(ifp, ifc);
}

/* link to Nos Task library */ 
#include "taskManager.h"
void kernelRequestCB(void *cbFunc, int32_t sock)
{
	taskFdSet(cbFunc, sock, TASK_READ | TASK_PERSIST, TASK_PRI_MIDDLE, 0);
}




#ifdef HAVE_IPV6
/* Interface's address add/delete functions. */
int32_t 
kernelIfIpv6AddressAdd (InterfaceT *ifp, 
				ConnectedT *ifc)
{
  int ret;

  /* Linux kernel returns EOPNOTSUPP/EAGAIN errno
     when the IPv6 address already exists.  */
  //ret = kernel_address_add_ipv6 (ifp, ifc);
  if (ret < 0 && errno != EOPNOTSUPP && errno != EAGAIN)
    return -1;

  return RESULT_OK;
}

int32_t
kernelIfIpv6AddressDelete (InterfaceT *ifp, 
				   ConnectedT *ifc)
{
  int ret;

  //ret = kernel_address_delete_ipv6 (ifp, ifc);
  if (ret < 0)
    return -1;

  return RESULT_OK;
}

#endif /* HAVE_IPV6 */
