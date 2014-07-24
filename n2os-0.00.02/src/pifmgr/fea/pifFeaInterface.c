/*345678901234567890123456789012345678901234567890123456789012345678901234567890*/
/*********************************************************************************
 *               Electronics and Telecommunications Research Institute
 * Filename : <myFileName>
 * Blockname: <PIF Manager>
 * Overview : <PIF Manager S/W block manages Port/Interface & L2 MAC/VLAN>
 * Creator  : <Seungwoo Hong>
 * Owner    : <Seungwoo Hong>
 * Copyright: 2013 Electronics and Telecommunications Research Institute. 
 *            All rights reserved. No part of this software shall be reproduced, 
 *            stored in a retrieval system, or transmitted by any means, 
 *            electronic, mechanical, photocopying, recording, or otherwise, 
 *            without written permission from ETRI.
 *********************************************************************************/
    
/*********************************************************************************
 *                        VERSION CONTROL  INFORMATION
 * $Author$
 * $Date$
 * $Revision
 * $Log$ 
 *********************************************************************************/

/*
  NSM - Forwarding plane APIs.
*/

#include <stdio.h>
#include "pal.h"
#include "hal_incl.h"

#include "pif.h"
#include "pifFeaApi.h"



/*
  Update the interface information from the forwarding layer.
*/
int
pif_fea_if_update (void)
{
  PAL_DBUG(0, "enter\n");
  kernelIfUpdate ();
  return 0;
}

/*
  Set flags for a interface. 
*/
int
pif_fea_if_flags_set (struct interface *ifp, u_int32_t flags)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  hal_if_flags_set (ifp->name, ifp->ifIndex, flags);
#endif /* ENABLE_HAL_PATH */

#ifdef ENABLE_PAL_PATH 
  kernelIfFlagsSet (ifp, flags);
#endif /* ENABLE_PAL_PATH */

  return 0;
}

/*
  Unset flags for a interface. 
*/
int
pif_fea_if_flags_unset (struct interface *ifp, u_int32_t flags)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  hal_if_flags_unset (ifp->name, ifp->ifIndex, flags);
#endif /* ENABLE_HAL_PATH */

#ifdef ENABLE_PAL_PATH
  kernelIfFlagsUnset (ifp, flags);
#endif /* ENABLE_PAL_PATH */

  return 0;
}

/*
  Get flags for a interface.
*/
extern result_t kernel_if_flags_get(InterfaceT *ifp);
int
pif_fea_if_flags_get (struct interface *ifp)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_PAL_PATH
  return kernel_if_flags_get (ifp);
#elif defined (ENABLE_HAL_PATH)
  {
    u_int32_t flags;
    int ret;
    
    ret = kernelIfFlagsGet (ifp->name, ifp->ifIndex, &flags);
    if (ret < 0)
      return ret;
    
    ifp->flags = flags;
  }
#endif /* ENABLE_PAL_PATH */
  return 0;
}

/*
  Set MTU for a interface.
*/
int
pif_fea_if_set_mtu (struct interface *ifp, int mtu)
{
  PAL_DBUG(0, "enter\n");
#ifdef ENABLE_HAL_PATH
  hal_if_set_mtu (ifp->name, ifp->ifIndex, mtu);
#endif /* ENABLE_HAL_PATH */

#ifdef ENABLE_PAL_PATH
  kernelIfSetMtu (ifp, mtu);
#endif /* ENABLE_PAL_PATH */
  return 0;
}

int
pif_if_proxy_arp_set (struct interface *ifp, int proxy_arp)
{
  PAL_DBUG(0, "enter\n");
  int ret = 0;
  ret = kernelIfSetproxyArp (ifp, proxy_arp);
  return ret;
}


/*
  Get ARP AGEING TIMEOUT for a interface.
*/
int
pif_fea_if_get_arp_ageing_timeout (struct interface *ifp)
{
  PAL_DBUG(0, "enter\n");
  int ret = 0;
#ifdef ENABLE_HAL_PATH
  int arp_ageing_timeout;

  ret = hal_if_get_arp_ageing_timeout (ifp->name, ifp->ifIndex, &arp_ageing_timeout);
  if (ret < 0)
    return ret;

  ifp->arp_ageing_timeout = arp_ageing_timeout;
#endif  /* ENABLE_HAL_PATH */
  return ret;
}

/*
  Set ARP AGEING TIMEOUT for a interface.
*/
int
pif_fea_if_set_arp_ageing_timeout (struct interface *ifp, int arp_ageing_timeout)
{
  PAL_DBUG(0, "enter\n");
  int ret = 0;

#ifdef ENABLE_HAL_PATH
  ret = hal_if_set_arp_ageing_timeout (ifp->name, ifp->ifIndex, arp_ageing_timeout);
#endif /* ENABLE_HAL_PATH */

  return ret;
}

/*
  Get DUPLEX for a interface.
*/
int
pif_fea_if_get_duplex (struct interface *ifp)
{
  PAL_DBUG(0, "enter\n");
  int ret = 0;
#ifdef ENABLE_HAL_PATH
  int duplex;

  ret = hal_if_get_duplex (ifp->name, ifp->ifIndex, &duplex);
  if (ret < 0)
    return ret;

  ifp->duplex = duplex;
#endif  /* ENABLE_HAL_PATH */
  return ret;
}

/*
  Set DUPLEX for a interface.
*/
int
pif_fea_if_set_duplex (struct interface *ifp, int duplex)
{
  PAL_DBUG(0, "enter\n");
  int ret = 0;

#ifdef ENABLE_HAL_PATH
  ret = hal_if_set_duplex (ifp->name, ifp->ifIndex, duplex);
#endif  /* ENABLE_HAL_PATH */
  return ret;
}

/*
  Set AUTO-NEGOTIATE for a interface.
*/
int 
pif_fea_if_set_autonego (struct interface *ifp, int autonego)
{
  PAL_DBUG(0, "enter\n");
  int ret = 0;

#ifdef ENABLE_HAL_PATH
  ret = hal_if_set_autonego (ifp->name, ifp->ifIndex, autonego);
#endif  /* ENABLE_HAL_PATH */
  return ret;
} 

/*
  Get BANDWIDTH for a interface.
*/
int
pif_fea_if_get_bandwidth (struct interface *ifp)
{
  PAL_DBUG(0, "enter\n");
  int ret = 0;
#ifdef ENABLE_HAL_PATH
  unsigned int bandwidth;

  ret = hal_if_get_bw (ifp->name, ifp->ifIndex, &bandwidth);
  if (ret < 0)
    return ret;

  ifp->bandwidth = bandwidth;
#endif  /* ENABLE_HAL_PATH */
  return ret;
}

/*
  Set BANDWIDTH for a interface.
*/
int
pif_fea_if_set_bandwidth (struct interface *ifp, float bandwidth)
{
  PAL_DBUG(0, "enter\n");
  int ret = 0;

#ifdef ENABLE_HAL_PATH
  ret = hal_if_set_bw (ifp->name, ifp->ifIndex, bandwidth);
#endif  /* ENABLE_HAL_PATH */
  return ret;
}


/*
  Set PORT-TYPE(switchport/routedport) for a interface.
*/
int
pif_fea_if_set_port_type (struct interface *ifp, int portType)
{
  PAL_DBUG(0, "enter\n");
  int ret = 0;

  enum hal_if_port_type type; 
  if(portType == SWITCH_PORT) type = HAL_IF_SWITCH_PORT;
  else type = HAL_IF_ROUTER_PORT;

#ifdef ENABLE_HAL_PATH
  unsigned int retifIndex;
  ret = hal_if_set_port_type (ifp->name, ifp->ifIndex, type, &retifIndex);
#endif  /* ENABLE_HAL_PATH */
  return ret;
}



#ifdef HAVE_IPV6
/*
  Get IPv6 forwarding enable flag. 
*/
int
pif_fea_ipv6_forwarding_get (int *ipforward)
{
  PAL_DBUG(0, "enter\n");
#ifdef HAVE_L3
#ifdef ENABLE_PAL_PATH
  return pal_kernel_ipv6_forwarding_get (ipforward);
#else
  *ipforward = 1;
  return 0;
#endif /* ENABLE_PAL_PATH */
#endif /* HAVE_L3 */
}

/*
  Set IPv6 forwarding enable flag. 
*/
int
pif_fea_ipv6_forwarding_set (int ipforward)
{
  PAL_DBUG(0, "enter\n");
#ifdef HAVE_L3
#ifdef ENABLE_PAL_PATH
  return pal_kernel_ipv6_forwarding_set (ipforward);
#endif /* ENABLE_PAL_PATH */
#endif /* HAVE_L3 */
  return 0;
}

#endif /* HAVE_IPV6 */

/*
  Add a IPv4 address to a interface.
*/
int
pif_fea_if_ipv4_address_primary_add (struct interface *ifp,
				     struct connected *ifc)
{
  PAL_DBUG(0, "enter\n");
#ifdef HAVE_L3
#ifdef ENABLE_HAL_PATH
  {
    PrefixT *p;
    unsigned int retifIndex = ifp->ifIndex;
    int ret = 0;
     
    p = ifc->address;
    if ((ifp->ifIndex <= 0) &&
        (ifp->hwType == IF_TYPE_VLAN))
      {
        /* Create the SVI in hardware first */
        ret = hal_if_svi_create (ifp->name, &retifIndex);
        if ((ret < 0) || (retifIndex <= 0))
          return -1;
      }    
    //ret = hal_if_ipv4_address_add (ifp->name, retifIndex, &p->u.prefix4, p->prefixlen);
    if (ret < 0) return -1;
  }
#endif /* ENABLE_HAL_PATH */

#ifdef ENABLE_PAL_PATH
  kernelIfIpv4AddressAdd (ifp, ifc);
#endif /* ENABLE_PAL_PATH */
#endif /* HAVE_L3 */
  return 0;
}

/*
  Delete a IPv4 address to a interface.
*/
int
pif_fea_if_ipv4_address_primary_delete (struct interface *ifp,
					struct connected *ifc)
{
  PAL_DBUG(0, "enter\n");
#ifdef HAVE_L3
#ifdef ENABLE_HAL_PATH
  {
	/*
    struct prefix *p;
    
    p = ifc->address;
    
    hal_if_ipv4_address_delete (ifp->name, ifp->ifIndex, &p->u.prefix4, p->prefixlen);
	*/
  }
#endif /* ENABLE_HAL_PATH */

#ifdef ENABLE_PAL_PATH
  kernelIfIpv4AddressDelete (ifp, ifc);
#endif /* ENABLE_PAL_PATH */
#endif /* HAVE_L3 */
  return 0;
}

/*
  Add secondary IPv4 address.
*/
int
pif_fea_if_ipv4_address_secondary_add (struct interface *ifp,
				       struct connected *ifc)
{
  PAL_DBUG(0, "enter\n");
#ifdef HAVE_L3

#ifdef ENABLE_PAL_PATH
  kernelIfIpv4AddressSecondaryAdd (ifp, ifc);
#endif /* ENABLE_PAL_PATH */
#endif /* HAVE_L3 */
  return 0;
}

/*
  Delete secondary IPv4 address.
*/
int
pif_fea_if_ipv4_address_secondary_delete (struct interface *ifp,
					  struct connected *ifc)
{
  PAL_DBUG(0, "enter\n");
#ifdef HAVE_L3

#ifdef ENABLE_PAL_PATH
  kernelIfIpv4AddressSecondaryDelete (ifp, ifc);
#endif /* ENABLE_PAL_PATH */
#endif /* HAVE_L3 */
  return 0;
}

int
pif_fea_if_ipv4_address_add (struct interface *ifp, struct connected *ifc)
{
  PAL_DBUG(0, "enter\n");
  if (CHECK_FLAG (ifc->flags, IFA_SECONDARY))
    return pif_fea_if_ipv4_address_secondary_add (ifp, ifc);
  else
    return pif_fea_if_ipv4_address_primary_add (ifp, ifc);
}

int
pif_fea_if_ipv4_address_delete (struct interface *ifp, struct connected *ifc)
{
  PAL_DBUG(0, "enter\n");
  if (CHECK_FLAG (ifc->flags, IFA_SECONDARY))
    return pif_fea_if_ipv4_address_secondary_delete (ifp, ifc);
  else
    return pif_fea_if_ipv4_address_primary_delete (ifp, ifc);
}

/*
  Delete all IPv4 address configured on a interface.
*/
int
pif_fea_if_ipv4_address_delete_all (struct interface *ifp, struct connected *ifc)
{
  PAL_DBUG(0, "enter\n");
#ifdef HAVE_L3

#ifdef ENABLE_PAL_PATH
  kernelIfIpv4AddressDeleteAll (ifp);
#endif /* ENABLE_PAL_PATH */
#endif /* HAVE_L3 */
  return 0;
}

/*
  Update IPv4 address on a interface.
*/
int
pif_fea_if_ipv4_address_update (struct interface *ifp, struct connected *ifc_old,
				struct connected *ifc_new)
{
#if 0


#ifdef HAVE_L3
#ifdef ENABLE_HAL_PATH
  {
    struct prefix *p;
    struct connected *ifc;
    
    /* Delete all addresses from interface. */
    pif_fea_if_ipv4_address_delete_all (ifp, ifp->ifc_ipv4);
    p = ifc_old->address;
    hal_if_ipv4_address_delete (ifp->name, ifp->ifIndex, &p->u.prefix4, p->prefixlen);

    /* Add all addresses to interface. */
    p = ifc_new->address;
    hal_if_ipv4_address_add (ifp->name, ifp->ifIndex, &p->u.prefix4, p->prefixlen);
    for (ifc = ifp->ifc_ipv4; ifc; ifc = ifc->next)
      {
	p = ifc->address;
	hal_if_ipv4_address_add (ifp->name, ifp->ifIndex, &p->u.prefix4, p->prefixlen);
      }
    return 0;
  }
#endif /* ENABLE_HAL_PATH */

#ifdef ENABLE_PAL_PATH
  pal_kernel_if_ipv4_address_update (ifp, ifc_old, ifc_new);
#endif /* ENABLE_PAL_PATH */
#endif /* HAVE_L3 */

#endif

  return 0;
}

#ifdef HAVE_IPV6
/* 
   Add IPV6 address on a interface. 
*/
int
pif_fea_if_ipv6_address_add (struct interface *ifp, struct connected *ifc)
{
#ifdef HAVE_L3
#ifdef ENABLE_HAL_PATH
  {
    struct prefix *p;
    int retifIndex = ifp->ifIndex;
    int ret;

    p = ifc->address;
    if ((ifp->ifIndex <= 0) &&
        (ifp->hw_type == IF_TYPE_VLAN))
      {
        /* Create the SVI in hardware first */
        ret = hal_if_svi_create (ifp->name, &retifIndex);
        if ((ret < 0) || (retifIndex <= 0))
          return -1;
      }
    ret = hal_if_ipv6_address_add (ifp->name, retifIndex, &p->u.prefix6, p->prefixlen, ifc->flags);
    if (ret < 0)
      return -1;
  }
#endif /* ENABLE_HAL_PATH */

#ifdef ENABLE_PAL_PATH
  pal_kernel_if_ipv6_address_add (ifp, ifc);
#endif /* ENABLE_PAL_PATH */
#endif /* HAVE_L3 */
  return 0;
}

/*
  Delete IPv6 address from a interface.
*/
int
pif_fea_if_ipv6_address_delete (struct interface *ifp, struct connected *ifc)
{
#ifdef HAVE_L3
#ifdef ENABLE_HAL_PATH
  {
    struct prefix *p;

    p = ifc->address;

    hal_if_ipv6_address_delete (ifp->name, ifp->ifIndex, &p->u.prefix6, p->prefixlen);
  }
#endif /* ENABLE_HAL_PATH */
#endif /* HAVE_L3 */

#endif /* ENABLE_IPV6 */


PhysicalPortT* pif_fea_new_port(InterfaceT *ifp, Uint32T iid)
{
	/* create new PhysicalPort */
	PhysicalPortT* port = newPhysicalPort();

	/* port name */
	strncpy(port->name, ifp->name, strlen(ifp->name));

	/* port interface id  */
	port->iid.idx = iid;

	/* port linux ifIndex */
	port->ifIndex = ifp->ifIndex;

	/* port adminState, initial state is DISABLED */
	port->adminState = ifIsUp(ifp) ? ADMIN_UP : ADMIN_DOWN;
	port->operState = ifIsOperative(ifp) ? STATE_UP : STATE_DOWN;;
		
	/* port speed, need to check if use linux bandwidth */
	// port->speed = portSpeedMap(ifp->bandwidth);
	port->speed = ifp->bandwidth;
	port->bandwidth = ifp->bandwidth;

	/* port HW type */
	if(ifp->hwType == IF_TYPE_ETHERNET){
		port->hwType = (port->speed > 100)?FAST_ETHERNET:GIGA_ETHERNET;
	}
	else if(ifp->hwType == IF_TYPE_AGGREGATE){
		port->hwType = AGG_ETHERNET;
	}

	/* port HW address */
	memcpy(port->hwAddr, ifp->hwAddr, ETH_ADDR_LEN);

	/* encapType, duplex, flowCtrl */
	port->encapType = ENCAP_ARPA;
	port->duplex    = PORT_DUPLEX_AUTO;
	port->flowCtrl.send    = FLOWCTL_DESIRED;
	port->flowCtrl.receive = FLOWCTL_DESIRED;

	/* port mode  */
	port->portMode = PIF_DEFAULT_PORTMODE;

	return port;
}


AggGroupT* pif_fea_new_agg_port(InterfaceT *ifp, Uint32T iid)
{
	/* create new PhysicalPort */
	AggGroupT* port = newAggGroup();

	/* port name */
	strncpy(port->name, ifp->name, strlen(ifp->name));

	/* port interface id */
	port->iid.idx = iid;

	/* port linux ifIndex */
	port->ifIndex = ifp->ifIndex;

	port->adminState = ifIsUp(ifp) ? ADMIN_UP : ADMIN_DOWN;
	port->operState = ifIsOperative(ifp) ? STATE_UP : STATE_DOWN;;
		
	port->speed = ifp->bandwidth;
	port->bandwidth = ifp->bandwidth;

	/* port HW type */
	port->hwType = AGG_ETHERNET;

	/* port HW address */
	memcpy(port->hwAddr, ifp->hwAddr, ETH_ADDR_LEN);

	/* encapType, duplex, flowCtrl */
	port->encapType = ENCAP_ARPA;
	port->duplex    = PORT_DUPLEX_AUTO;

	port->portMode = PIF_DEFAULT_PORTMODE;

	return port;
}


IPInterfaceT* pif_fea_new_interface(InterfaceT *ifp, Uint32T iid)
{
	/* Create new logical IPInterface  */
	IPInterfaceT* ipIf = newIPInterface();

	/* interface name */
	strncpy(ipIf->name, ifp->name, strlen(ifp->name));

	/* ipIf interface id  */
	ipIf->iid.idx  = iid;

	/* ipIf linux ifIndex */
	ipIf->ifIndex = ifp->ifIndex;

	/* ipIf adminState, operState */
	ipIf->adminState = ifIsUp(ifp) ? ADMIN_UP : ADMIN_DOWN;
	ipIf->operState = ifIsOperative(ifp) ? STATE_UP : STATE_DOWN;;
		
	/* ipIf HW type */
	ipIf->ifType = ifp->hwType;

	/* ipIf mtu */
	ipIf->mtu = ifp->mtu;

	/* ipIf metric */
	ipIf->metric = ifp->metric; 

	/* ipIf bandwidth */
	ipIf->bandwidth = ifp->bandwidth;

	/* ipIf linux interface flags */
	ipIf->flags = ifp->flags;

	/* interface port mode */
	ipIf->ifMode = ROUTED_PORT;

	return ipIf;
}



void pif_fea_if_add(InterfaceT *ifp)
{
	/* Create new PhysicalPort and new IPInterface */
	PAL_DBUG(0, "enter(%s: strlen%d)\n", ifp->name, (int)strlen(ifp->name));

	/* prepare new objects */
	PhysicalPortT* port = NULL;
	IPInterfaceT* ipIf = NULL;

	/* get IID */
	Uint32T iid = getIidbyName(ifp->name);
	Int32T type = getIidType(iid);

	/* create new IPInterface */ 
	ipIf = pif_fea_new_interface(ifp, iid);

	/* create new PhysicalPort. if type is ethernet/aggregate */ 
	if((ifp->hwType == IF_TYPE_ETHERNET) || (ifp->hwType == IF_TYPE_AGGREGATE)) 
	{
		if(type != TYPE_VLAN) 
		{
			port = pif_fea_new_port(ifp, iid);
			port->routedPort.connectedIpIf = ipIf;
			ipIf->ifMode = PIF_DEFAULT_PORTMODE;

			/* notify PhysicalPort creation to user FEA_INTERFACE */
			feaEventPortAdd(port);
		}
	}

	/* notify IPInterface creation to user FEA_INTERFACE */
	ipIf->attachedPort = port;
	feaEventIPInterfaceAdd(ipIf);
}


void pif_fea_if_del(InterfaceT *ifp)
{
	/* Delete PhysicalPort and IPInterface */
	PAL_DBUG(0, "enter\n");

	/* get IID */
	Uint32T iid = getIidbyName(ifp->name);

	Int8T ifName[INTERFACE_NAMSIZ + 1];
	Uint32T ifIndex = ifp->ifIndex;
	memcpy(ifName, ifp->name, INTERFACE_NAMSIZ);

	/* Delete PhysicalPort first */
	if( (ifp->hwType == IF_TYPE_ETHERNET) ||
	    (ifp->hwType == IF_TYPE_AGGREGATE) )
	{
		feaEventPortDelete(ifName, ifIndex, iid);
	}

	/* Delete IPInterface */
	feaEventIPInterfaceDelete(ifName, ifIndex, iid);
}

void pif_fea_if_up(InterfaceT *ifp)
{
	PAL_DBUG(0, "enter\n");

	/* get IID */
	Uint32T iid = getIidbyName(ifp->name);

	Int8T ifName[INTERFACE_NAMSIZ + 1];
	Uint32T ifIndex = ifp->ifIndex;
	memcpy(ifName, ifp->name, INTERFACE_NAMSIZ);

	if(ifp->hwType == IF_TYPE_ETHERNET){
		feaEventPortUp(ifName, ifIndex, iid);
	}

	feaEventIPInterfaceUp(ifName, ifIndex, iid);
}

void pif_fea_if_down(InterfaceT *ifp)
{
	PAL_DBUG(0, "enter\n");

	/* get IID */
	Uint32T iid = getIidbyName(ifp->name);

	Int8T ifName[INTERFACE_NAMSIZ + 1];
	Uint32T ifIndex = ifp->ifIndex;
	memcpy(ifName, ifp->name, INTERFACE_NAMSIZ);

	if(ifp->hwType == IF_TYPE_ETHERNET){
		feaEventPortDown(ifName, ifIndex, iid);
	}

	feaEventIPInterfaceDown(ifName, ifIndex, iid);
}



void pif_fea_if_addr_add(InterfaceT *ifp, ConnectedT *ifc)
{
	PAL_DBUG(0, "enter\n");

	/* get IID */
	Uint32T iid = getIidbyName(ifp->name);

	/* new connected address */
	ConnectedAddressT* conAddr = newConnectedAddress();

	/* address flags */
	conAddr->flags = ifc->flags;

	/* local address */
	nnPrefixCopy(&(conAddr->address), ifc->address);

	/* broadcast address */
	if(ifc->destination) {
		nnPrefixCopy(&(conAddr->broadcast), ifc->destination);
	}

	feaEventIPAddressAdd(ifp->name, iid, conAddr);
}

void pif_fea_if_addr_del(InterfaceT *ifp, ConnectedT *ifc)
{
	PAL_DBUG(0, "enter\n");

	/* get IID */
	Uint32T iid = getIidbyName(ifp->name);

	feaEventIPAddressDelete(ifp->name, iid, ifc->address);
}




