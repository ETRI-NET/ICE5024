/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : RIP Protocol의 인터페이스 설정을 제어하는 화일
 *
 * - Block Name : RIP Protocol
 * - Process Name : ripd
 * - Creator : Geontae Park
 * - Initial Date : 2014/02/20
 */

/**
 * @file        : ripInterface.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include <assert.h>

#include "nnTypes.h"
#include "nnRibDefines.h"
#include "nnStr.h"
#include "nnIf.h"
#include "nnPrefix.h"
#include "nnTable.h"
#include "nnList.h"
#include "nnVector.h"
#include "nnSocketOption.h"
#include "nosLib.h"

#include "nnCmdCommon.h"

#include "ripd.h"
#include "ripInterface.h"
#include "ripRibmgr.h"
#include "ripUtil.h"
#include "ripDebug.h"


/* static prototypes */
static void ripEnableApply (InterfaceT *);
static void ripPassiveInterfaceApply (InterfaceT *);
static Int32T ripIfDown(InterfaceT *);
static Int32T ripEnableIfLookup (const char *);
static Int32T ripEnableNetworkLookup2 (ConnectedT *);
static void ripEnableApplyAll (void);

struct message ri_version_msg[] = 
{
  {RI_RIP_VERSION_1,       "1"},
  {RI_RIP_VERSION_2,       "2"},
  {RI_RIP_VERSION_1_AND_2, "1 2"},
};


/* external data structure */

/* Join to the RIP version 2 multicast group. */
static Int32T
ipv4MulticastJoin (Int32T sock,
                   struct in_addr group,
                   struct in_addr ifAddr,
                   Uint32T ifIndex)
{
  Int32T ret = 0;

  NNLOG (LOG_DEBUG, "MULTICAST JOIN :: %s, group=%s, ifa=%s, ifindex=%d\n", 
         __func__, inet_ntoa(group), inet_ntoa(ifAddr), ifIndex);

  ret = setSockOptMulticastIpv4 (sock, IP_ADD_MEMBERSHIP, ifAddr, 
                                 group.s_addr, ifIndex); 

  if (ret < 0) 
    NNLOG(LOG_DEBUG,"can't setsockopt IP_ADD_MEMBERSHIP %d\n", errno);

  return ret;
}

/* Leave from the RIP version 2 multicast group. */
static Int32T
ipv4MulticastLeave (Int32T sock, 
		      struct in_addr group, 
		      struct in_addr ifAddr,
		      Uint32T ifIndex)
{  
  Int32T ret = 0;

  NNLOG(LOG_DEBUG,"Fun[%s] Line[%d]\n", __FUNCTION__, __LINE__);
  
  ret = setSockOptMulticastIpv4 (sock, IP_DROP_MEMBERSHIP, ifAddr, 
                                 group.s_addr, ifIndex);

  if (ret < 0) 
    NNLOG(LOG_DEBUG,"can't setsockopt IP_DROP_MEMBERSHIP\n");

  return ret;
}


/* Allocate new RIP's interface configuration. */
static RipInterfaceT *
ripInterfaceNew (void)
{
  RipInterfaceT * pRIf = NULL;

  NNLOG(LOG_DEBUG,"Fun[%s] Line[%d]\n", __FUNCTION__, __LINE__);

  pRIf = NNMALLOC (MEM_IF, sizeof (RipInterfaceT));
  /* Default authentication type is simple password for Cisco
     compatibility. */
  pRIf->authType = RIP_NO_AUTH;
  pRIf->md5AuthLen = RIP_AUTH_MD5_COMPAT_SIZE;

  /* Set default split-horizon behavior.  If the interface is Frame
     Relay or SMDS is enabled, the default value for split-horizon is
     off.  But currently Zebra does detect Frame Relay or SMDS
     interface.  So all interface is set to split horizon.  */
  pRIf->splitHorizonDefault = RIP_SPLIT_HORIZON;
  pRIf->splitHorizon = pRIf->splitHorizonDefault;

  return pRIf;
}

void
ripInterfaceMulticastSet (Int32T sock, ConnectedT *pConnected)
{
  struct in_addr addr;
  
  assert (pConnected != NULL);
  
  addr = CONNECTED_ID(pConnected)->u.prefix4;

  /* set socket option to do not receive looped packet */
  Int32T val = 0;
  Int32T len = sizeof(val); 
  if(setsockopt (sock, IPPROTO_IP, IP_MULTICAST_LOOP, (void *)&val, len) < 0)
  {
    NNLOG(LOG_DEBUG,"Can't setsockopt IP_MULTICAST_LOOP on fd %d to \n"
          "source address %s for interface %s",
          sock, inet_ntoa(addr), pConnected->pIf->name);
  }

  /* set socket option to enable multicast interface */
  if (setSockOptMulticastIpv4 (sock, IP_MULTICAST_IF, addr, 0, 
                               pConnected->pIf->ifIndex) < 0) 
  {
    NNLOG(LOG_DEBUG,"Can't setsockopt IP_MULTICAST_IF on fd %d to \n"
          "source address %s for interface %s",
          sock, inet_ntoa(addr), pConnected->pIf->name);
  }

  return;
}

/* Send RIP request packet to specified interface. */
static void
ripRequestInterfaceSend (InterfaceT * pIf, Uint8T version)
{
  struct sockaddr_in to;

  /* RIPv2 support multicast. */
  if (version == RIPv2 && ifIsMulticast (pIf))
  {
    if (IS_RIP_DEBUG_EVENT)
      NNLOG(LOG_DEBUG,"multicast request on %s\n", pIf->name);

    ripRequestSend (NULL, pIf, version, NULL);
    return;
  }

  /* RIPv1 and non multicast interface. */
  if (ifIsPointPoint (pIf) || ifIsBroadcast (pIf))
  {
    ConnectedT *connected;

    if (IS_RIP_DEBUG_EVENT)
      NNLOG(LOG_DEBUG,"broadcast request to %s\n", pIf->name);

    ListT * pConnectedList = pIf->pConnected;
    ListNodeT * pConnectedNode;

    for (pConnectedNode = pConnectedList->pHead;
         pConnectedNode != NULL;
         pConnectedNode = pConnectedNode->pNext)
    {
      connected = pConnectedNode->pData;

      if (connected->pAddress->family == AF_INET)
      {
        memset (&to, 0, sizeof (struct sockaddr_in));
        to.sin_port = htons (RIP_PORT_DEFAULT);
        if (connected->pDestination)
          /* use specified broadcast or peer destination addr */
          to.sin_addr = connected->pDestination->u.prefix4;
        else if (connected->pAddress->prefixLen < NN_IPV4_MAX_PREFIXLEN){
          /* calculate the appropriate broadcast address */
          nnGetBroadcastfromAddr(&to.sin_addr, 
                                 &connected->pAddress->u.prefix4,
                                 &connected->pAddress->prefixLen);

        }
        else
          /* do not know where to send the packet */
          continue;

        if (IS_RIP_DEBUG_EVENT)
          NNLOG(LOG_DEBUG,"SEND request to %s\n", inet_ntoa (to.sin_addr));

        ripRequestSend (&to, pIf, version, connected);
      }
    }
  }
}

/* This will be executed when interface goes up. */
static void
ripRequestInterface (InterfaceT *pIf)
{
  RipInterfaceT *ri = NULL;

  /* In default ripd doesn't send RIP_REQUEST to the loopback interface. */
  if (ifIsLoopback (pIf))
    return;

  /* If interface is down, don't send RIP packet. */
  if (! ifIsOperative (pIf))
    return;

  /* Fetch RIP interface information. */
  ri = pIf->pInfo;


  /* If there is no version configuration in the interface,
     use rip's version setting. */
  Int32T vsend = ((ri->riSend == RI_RIP_UNSPEC) ?  
                                 pRip->versionSend : ri->riSend);
    
  if (vsend & RIPv1)
  {
    ripRequestInterfaceSend (pIf, RIPv1);
  }
  if (vsend & RIPv2)
  {
    ripRequestInterfaceSend (pIf, RIPv2);
  }
}

/* Send RIP request to the neighbor. */
static void
ripRequestNeighbor (struct in_addr addr)
{
  struct sockaddr_in to;

  memset (&to, 0, sizeof (struct sockaddr_in));
  to.sin_port = htons (RIP_PORT_DEFAULT);
  to.sin_addr = addr;

  ripRequestSend (&to, NULL, pRip->versionSend, NULL);
}

/* Request routes at all interfaces. */
static void
ripRequestNeighborAll (void)
{
  RouteNodeT *pRNode = NULL;

  /* Check global rip data structure pointer. */
  if (!pRip)
    return;

  /* Check neighbor table pointer. */
  if (!pRip->pNeighbor)
    return;

  if (IS_RIP_DEBUG_EVENT)
    NNLOG(LOG_DEBUG, "request to the all neighbor\n");

  /* Send request to all neighbor. */
  for (pRNode = nnRouteTop (pRip->pNeighbor); 
       pRNode; 
       pRNode = nnRouteNext (pRNode))
  {
    if (pRNode->pInfo)
      ripRequestNeighbor (pRNode->p.u.prefix4);
  }
}

/* Multicast packet receive socket. */
static Int32T
ripMulticastJoin (InterfaceT * pIf, Int32T sock) // gtp, rip multicast group ?? join
{
  ConnectedT *pIfc = NULL;

  if (ifIsOperative (pIf) && ifIsMulticast (pIf))
  {
    if (IS_RIP_DEBUG_EVENT)
      NNLOG(LOG_DEBUG,"multicast join at %s\n", pIf->name);

    ListT * pConnectedList = pIf->pConnected;
    ListNodeT * pConnectedNode;

    for (pConnectedNode = pConnectedList->pHead;
         pConnectedNode != NULL;
         pConnectedNode = pConnectedNode->pNext)
    {
      Prefix4T *p;
      struct in_addr group;

      pIfc = pConnectedNode->pData;
	      
      p = (Prefix4T *) pIfc->pAddress;
      
      if (p->family != AF_INET)
        continue;
      
      group.s_addr = htonl (INADDR_RIP_GROUP);
      if (ipv4MulticastJoin (sock, group, p->prefix, pIf->ifIndex) < 0)
      {
        return -1;
      }
      else
      {
        return 0;
      }
    }

  }

  return 0;
}

/* Leave from multicast group. */
static void
ripMulticastLeave (InterfaceT * pIf, Int32T sock)
{
  ConnectedT * pConnected = NULL;

  if (ifIsUp (pIf) && ifIsMulticast (pIf))
  {
    if (IS_RIP_DEBUG_EVENT)
      NNLOG(LOG_DEBUG,"multicast leave from %s\n", pIf->name);

    ListT * pConnectedList = pIf->pConnected;;
    ListNodeT * pConnectedNode;

    for(pConnectedNode = pConnectedList->pHead;
        pConnectedNode != NULL;
        pConnectedNode = pConnectedNode->pNext)
    {
      Prefix4T *p;
      struct in_addr group;

      pConnected = pConnectedNode->pData;
          
      p = (Prefix4T *) pConnected->pAddress;
	  
      if (p->family != AF_INET)
        continue;
    
      group.s_addr = htonl (INADDR_RIP_GROUP);
      if (ipv4MulticastLeave (sock, group, p->prefix, pIf->ifIndex) == 0)
      {
        return;
      }
    }
  }

}

/* Is there and address on interface that I could use ? */
static Int32T
ripIfIpv4AddressCheck (InterfaceT * pIf)
{
  ConnectedT * pConnected = NULL;
  Int32T count = 0;

  ListT * pConnectedList = pIf->pConnected;
  ListNodeT * pConnectedNode;
  
  for(pConnectedNode = pConnectedList->pHead;
      pConnectedNode != NULL;
      pConnectedNode = pConnectedNode->pNext)
  {
    pConnected = pConnectedNode->pData;

    PrefixT *p;
    p = pConnected->pAddress;

    if (p->family == AF_INET)
      count++;
  }

  return count;
}
						
						
						

/* Does this address belongs to me ? */
Int32T
ifCheckAddress (struct in_addr addr)
{
  ListNodeT * pNode = NULL;
  InterfaceT *pIf = NULL;

  for(pNode = pRip->pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pIf = pNode->pData;

    ConnectedT * pConnected = NULL;
    ListT * pConnectedList = pIf->pConnected;
    ListNodeT * pConnectedNode;

    for(pConnectedNode = pConnectedList->pHead;
        pConnectedNode != NULL;
        pConnectedNode = pConnectedNode->pNext)
    {
      pConnected = pConnectedNode->pData;
      
      Prefix4T *p;
      p = (Prefix4T *) pConnected->pAddress;

      if (p->family != AF_INET)
        continue;

      if (PREFIX_IPV4_ADDR_CMP (&p->prefix, &addr) == 0)
      {
        return 1;
      }
	}
  }

  return 0;
}



/* Inteface link down message processing. */
Int32T
ripInterfaceDown (nnBufferT * msgBuff)
{
  InterfaceT * pIf = NULL;

  /* pifInterfaceStateRead() updates interface structure in pIflist. */
  pIf = pifInterfaceStateRead(msgBuff);

  if (pIf == NULL)
  {
    NNLOG (LOG_ERR, "Error] Interface not exist. %s \n", __func__);
    return 0;
  }

  ripIfDown(pIf);
 
  if (IS_RIP_DEBUG_RIBMGR)
    NNLOG(LOG_DEBUG,"interface %s index %d flags %llx metric %d mtu %d is down\n",
          pIf->name, pIf->ifIndex, (unsigned long long)pIf->flags,
          pIf->metric, pIf->mtu);

  return 0;
}

/* Inteface link up message processing */
Int32T
ripInterfaceUp (nnBufferT *msgBuff)
{
  InterfaceT * pIf = NULL;

  /* pifInterfaceStateRead () updates interface structure in pIflist.*/
  pIf = pifInterfaceStateRead (msgBuff);

  if (pIf == NULL)
    return 0;

  if (IS_RIP_DEBUG_RIBMGR)
    NNLOG(LOG_DEBUG,"interface %s index %d flags %#llx metric %d mtu %d is up\n",
	       pIf->name, pIf->ifIndex, (unsigned long long) pIf->flags,
	       pIf->metric, pIf->mtu);

  /* Check if this interface is RIP enabled or not.*/
  ripEnableApply (pIf);
 
  /* Check for a passive interface */
  ripPassiveInterfaceApply (pIf);

  /* Apply distribute list to the all interface. */
  ripDistributeUpdateInterface (pIf);

  return 0;
}

/* Inteface addition message from pifManager. */
Int32T
ripInterfaceAdd (nnBufferT * msgBuff)
{
  InterfaceT *pIf = NULL;

  pIf = pifInterfaceAddRead (msgBuff);

  if (IS_RIP_DEBUG_RIBMGR)
    NNLOG(LOG_DEBUG,"interface add %s index %d flags %#llx metric %d mtu %d\n",
          pIf->name, pIf->ifIndex, (unsigned long long) pIf->flags,
          pIf->metric, pIf->mtu);

  /* Check if this interface is RIP enabled or not.*/
  ripEnableApply (pIf);
 
  /* Check for a passive interface */
  ripPassiveInterfaceApply (pIf);

  /* Apply distribute list to the all interface. */
  ripDistributeUpdateInterface (pIf);

  /* ripRequestNeighborAll (); */

  /* Check interface routemap. */
  ripIfRmapUpdateInterface (pIf);

  return 0;
}

Int32T
ripInterfaceDelete (nnBufferT * msgBuff)
{
  InterfaceT *pIf = NULL;

  /* pifInterfaceStateRead() updates interface structure in pIflist */
  pIf = pifInterfaceStateRead(msgBuff);

  if (pIf == NULL)
    return 0;

  if (ifIsUp (pIf)) 
  {
    ripIfDown(pIf);
  } 
  
  NNLOG(LOG_DEBUG,"interface delete %s index %d flags %#llx metric %d mtu %d\n",
        pIf->name, pIf->ifIndex, (unsigned long long) pIf->flags,
        pIf->metric, pIf->mtu);
  
  /* To support pseudo interface do not free interface structure.  */
  pIf->ifIndex = IFINDEX_INTERNAL;

  return 0;
}


void
ripInterfaceReset (void)
{
  ListNodeT * pNode = NULL;
  InterfaceT * pIf = NULL;
  RipInterfaceT * pRIf = NULL;

  /* print interface. */
  NNLOG (LOG_DEBUG, "#################### ifDump(1) #####################\n");
  ifDumpAll();

  for(pNode = pRip->pIfList->pHead;
      pNode != NULL;
      pNode = pNode->pNext)
  {
    pIf = pNode->pData;

    pRIf = pIf->pInfo;
    pRIf->enableNetwork = 0;
    pRIf->enableInterface = 0;
    pRIf->running = 0;

    pRIf->riSend = RI_RIP_UNSPEC;
    pRIf->riReceive = RI_RIP_UNSPEC;

    pRIf->authType = RIP_NO_AUTH;

    if (pRIf->authStr)
    {
      NNFREE (MEM_AUTHENTICATION_NAME, pRIf->authStr);
      pRIf->authStr = NULL;
    }
    if (pRIf->keyChain)
    {
      NNFREE (MEM_KEYCHAIN, pRIf->keyChain);
      pRIf->keyChain = NULL;
    }

    pRIf->splitHorizon = RIP_NO_SPLIT_HORIZON;
    pRIf->splitHorizonDefault = RIP_NO_SPLIT_HORIZON;

    pRIf->pAccessList[RIP_FILTER_IN] = NULL;
    pRIf->pAccessList[RIP_FILTER_OUT] = NULL;

    pRIf->pPrefixList[RIP_FILTER_IN] = NULL;
    pRIf->pPrefixList[RIP_FILTER_OUT] = NULL;

    pRIf->recvBadPackets = 0;
    pRIf->recvBadRoutes = 0;
    pRIf->sentUpdates = 0;

    pRIf->passive = 0;
  }

  /* print interface. */
  NNLOG (LOG_DEBUG, "#################### ifDump(2) #####################\n");
  ifDumpAll();
}


Int32T
ripIfDown(InterfaceT *pIf)
{
  RouteNodeT * pRNode = NULL;
  RipInfoT * pRInfo = NULL;
  RipInterfaceT * pRIf = NULL;

  if (pRip)
  {
    for (pRNode = nnRouteTop (pRip->pRipRibTable); pRNode; pRNode = nnRouteNext (pRNode))
      if ((pRInfo = pRNode->pInfo) != NULL)
      {
        /* Routes got through this interface. */
        if (pRInfo->ifIndex == pIf->ifIndex &&
        pRInfo->type == RIB_ROUTE_TYPE_RIP &&
        pRInfo->subType == RIP_ROUTE_RTE)
        {
          ripRibmgrIpv4Delete ((Prefix4T *) &pRNode->p,
                               &pRInfo->nexthop,

                               pRInfo->metric);
          printf("+++++++++++++> %s called. \n", __func__);
          ripRedistributeDelete (pRInfo->type, pRInfo->subType,
                                 (Prefix4T *)&pRNode->p,
                                 pRInfo->ifIndex);
        }
        else
        {
          /* All redistributed routes but static and system */
          if ((pRInfo->ifIndex == pIf->ifIndex) &&
           /* (pRInfo->type != RIB_ROUTE_TYPE_STATIC) && */
              (pRInfo->type != RIB_ROUTE_TYPE_SYSTEM))
          {
            ripRedistributeDelete (pRInfo->type, pRInfo->subType,
                                   (Prefix4T *)&pRNode->p,
                                   pRInfo->ifIndex);
          }
        }
      }
  }
	    
  pRIf = pIf->pInfo;
  
  if (pRIf->running)
  {
    if (IS_RIP_DEBUG_EVENT)
      NNLOG(LOG_DEBUG,"turn off %s\n", pIf->name);

    /* Leave from multicast group. */
    ripMulticastLeave (pIf, pRip->sock);

    pRIf->running = 0;
  }

  return 0;
}

/* Needed for stop RIP process. */
void
ripIfDownAll ()
{
  InterfaceT * pIf = NULL;
  ListNodeT * pNode = NULL;

  for(pNode = pRip->pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pIf = pNode->pData;
    ripIfDown (pIf);
  }
}

static void
ripApplyAddressAdd (ConnectedT *pIfc)
{
  Prefix4T address = {0,};
  PrefixT *p = NULL;

  if (!pRip)
  {
    return;
  }

  if (! ifIsUp(pIfc->pIf))
  {
    return;
  }

  p = pIfc->pAddress;

  memset (&address, 0, sizeof (address));
  address.family = p->family;
  address.prefix = p->u.prefix4;
  address.prefixLen = p->prefixLen;
  nnApplyNetmasktoPrefix4(&address, &address.prefixLen); // gtp, apply_mask_ipv4(&address) 의 변경함수

  /* Check if this interface is RIP enabled or not
     or  Check if this address's prefix is RIP enabled */
  if ((ripEnableIfLookup(pIfc->pIf->name) >= 0) ||
      (ripEnableNetworkLookup2(pIfc) >= 0))
    ripRedistributeAdd(RIB_ROUTE_TYPE_CONNECT, RIP_ROUTE_INTERFACE,
                         &address, pIfc->pIf->ifIndex, NULL, 0, 0);
}

Int32T
ripInterfaceAddressAdd (nnBufferT * msgBuff)
{
  ConnectedT * pIfc = NULL;
  PrefixT * p = NULL;

  pIfc = pifInterfaceAddressRead (EVENT_INTERFACE_ADDRESS_ADD,  msgBuff);

  if (pIfc == NULL)
    return 0;

  p = pIfc->pAddress;

  if (p->family == AF_INET)
  {
    if (IS_RIP_DEBUG_RIBMGR)
      NNLOG(LOG_DEBUG,"connected address %s/%d is added\n",
            inet_ntoa (p->u.prefix4), p->prefixLen);

    ripEnableApply(pIfc->pIf);

    /* Check if this prefix needs to be redistributed */
    ripApplyAddressAdd(pIfc);

#ifdef HAVE_SNMP
    ripIfaddrAdd (pIfc->ifp, pIfc);
#endif /* HAVE_SNMP */
  }

  ifDumpAll(); // add by sckim

  return 0;
}

static void
ripApplyAddressDel (ConnectedT * pIfc) 
{
  Prefix4T address = {0,};
  PrefixT * p = NULL;


  if (!pRip)
  {
    return;
  }

  if (!pIfc)
  {
    return;
  }

  if (!pIfc->pIf)
  {
    return;
  }

  if (! ifIsUp(pIfc->pIf))
  {
    return;
  }

  p = pIfc->pAddress;

  memset (&address, 0, sizeof (address));
  address.family = p->family;
  address.prefix = p->u.prefix4;
  address.prefixLen = p->prefixLen;
  nnApplyNetmasktoPrefix4(&address, &address.prefixLen);

  ripRedistributeDelete (RIB_ROUTE_TYPE_CONNECT, RIP_ROUTE_INTERFACE,
                         &address, pIfc->pIf->ifIndex);
}

Int32T
ripInterfaceAddressDelete (nnBufferT * msgBuff)
{
  ConnectedT * pIfc = NULL;
  PrefixT * p = NULL;

  pIfc = pifInterfaceAddressRead (EVENT_INTERFACE_ADDRESS_DELETE, msgBuff);
  
  if (pIfc)
  {
    p = pIfc->pAddress;
    if (p->family == AF_INET)
    {
      if (IS_RIP_DEBUG_RIBMGR)
        NNLOG(LOG_DEBUG,"connected address %s/%d is deleted\n",
                        inet_ntoa (p->u.prefix4), p->prefixLen);

#ifdef HAVE_SNMP
      ripIfaddrDelete (pIfc->pIf, pIfc);
#endif /* HAVE_SNMP */

      /* Chech wether this prefix needs to be removed */
      ripApplyAddressDel(pIfc);

    }

    connectedFree (pIfc);
  }

  return 0;
}


/* Check interface is enabled by network statement. */
/* Check wether the interface has at least a connected prefix that
 * is within the ripng_enable_network table. */
static Int32T
ripEnableNetworkLookupIf (InterfaceT * pIf)
{
  ConnectedT * pConnected = NULL;
  Prefix4T address = {0,};

  ListT * pConnectedList = pIf->pConnected;
  ListNodeT * pConnectedNode;
  for(pConnectedNode = pConnectedList->pHead;
      pConnectedNode != NULL;
      pConnectedNode = pConnectedNode->pNext)
  {
    PrefixT *p; 
    RouteNodeT *node;

    pConnected = pConnectedNode->pData;

    p = pConnected->pAddress;

    if (p->family == AF_INET)
    {
      address.family = AF_INET;
      address.prefix = p->u.prefix4;
      address.prefixLen = NN_IPV4_MAX_BITLEN;
          
      node = nnRouteNodeMatch (pRip->pRipEnableNetwork, (PrefixT *)&address);
      if (node)
      {
        nnRouteNodeUnlock (node);
        return 1;
      }
    }
  }

  return -1;
}

/* Check wether connected is within the ripng_enable_network table. */
Int32T
ripEnableNetworkLookup2 (ConnectedT * pConnected)
{
  Prefix4T address = {0,};
  PrefixT * p = NULL;

  p = pConnected->pAddress;

  if (p->family == AF_INET) 
  {
    RouteNodeT *node;

    address.family = p->family;
    address.prefix = p->u.prefix4;
    address.prefixLen = NN_IPV4_MAX_BITLEN;

    node = nnRouteNodeMatch (pRip->pRipEnableNetwork, (PrefixT *)&address);

    if (node) 
    {
      nnRouteNodeUnlock (node);
      return 1;
    }
  }

  return -1;
}

/* Add RIP enable network. */
Int32T
ripEnableNetworkAdd (PrefixT *p)
{
  RouteNodeT * pRNode;

  pRNode = nnRouteNodeGet (pRip->pRipEnableNetwork, p); 
  if (pRNode->pInfo)
  {
    nnRouteNodeUnlock (pRNode);
    return -1;
  }
  else
    pRNode->pInfo = (char *) "enabled";

  /* XXX: One should find a better solution than a generic one */
  ripEnableApplyAll();

  return 1;
}

/* Delete RIP enable network. */
Int32T
ripEnableNetworkDelete (PrefixT *p)
{
  RouteNodeT * pRNode = NULL;

  pRNode = nnRouteNodeLookup (pRip->pRipEnableNetwork, p);
  if (pRNode)
  {
    pRNode->pInfo = NULL;

    /* Unlock info lock. */
    nnRouteNodeUnlock (pRNode);

    /* Unlock lookup lock. */
    nnRouteNodeUnlock (pRNode);

    /* XXX: One should find a better solution than a generic one */
    ripEnableApplyAll ();

    return 1;
  }

  return -1;
}

/* Check interface is enabled by ifname statement. */
static Int32T
ripEnableIfLookup (const char * ifName)
{
  Uint32T i = 0;
  char *str;

  for (i = 0; i < vectorActive (pRip->ripEnableInterface); i++)
    if ((str = vectorSlot (pRip->ripEnableInterface, i)) != NULL)
      if (strcmp (str, ifName) == 0)
      {
		return i;
      }

  return -1;
}

/* Add interface to rip_enable_if. */
Int32T
ripEnableIfAdd (const StringT ifName)
{
  Int32T ret = 0;

  ret = ripEnableIfLookup (ifName);
  if (ret >= 0)
    return -1;

  vectorSet (pRip->ripEnableInterface, nnStrDup (ifName, MEM_IF_NAME));

  ripEnableApplyAll(); /* TODOVJ */

  return 1;
}

/* Delete interface from rip_enable_if. */
Int32T
ripEnableIfDelete (const StringT ifName)
{
  Int32T ifIndex;
  char * str;

  ifIndex = ripEnableIfLookup (ifName);
  if (ifIndex < 0)
  {
    return -1;
  }

  str = vectorSlot (pRip->ripEnableInterface, ifIndex);
  NNFREE (MEM_IF_NAME, str);
  vectorUnset (pRip->ripEnableInterface, ifIndex);

  ripEnableApplyAll(); /* TODOVJ */
  
  return 1;
}

/* Join to multicast group and send request to the interface. */
static Int32T
ripInterfaceWakeup (InterfaceT * pIf)
{
  RipInterfaceT * pRIf = NULL;

  pRIf = pIf->pInfo;

  /* Join to multicast group. */
  if (ripMulticastJoin (pIf, pRip->sock) < 0)
  {
    return 0;
  }

  /* Set running flag. */
  pRIf->running = 1;

  /* Send RIP request to the interface. */
  ripRequestInterface (pIf);

  return 0;
}

static void
ripConnectSet (InterfaceT * pIf, Int32T set)
{
  ConnectedT * pConnected = NULL;
  Prefix4T address = {0,};

  ListT * pConnectedList = pIf->pConnected;
  ListNodeT * pConnectedNode;
  for (pConnectedNode = pConnectedList->pHead;
       pConnectedNode != NULL;
       pConnectedNode = pConnectedNode->pNext)
  {
    pConnected = pConnectedNode->pData;

    PrefixT *p; 
    p = pConnected->pAddress;

    if (p->family != AF_INET)
      continue;

    address.family = AF_INET;
    address.prefix = p->u.prefix4;
    address.prefixLen = p->prefixLen;
    nnApplyNetmasktoPrefix4(&address, &address.prefixLen);

    if (set) 
    {
      /* Check once more wether this prefix is within a "network IF_OR_PREF" one */
      if ((ripEnableIfLookup(pConnected->pIf->name) >= 0) ||
          (ripEnableNetworkLookup2(pConnected) >= 0))
        ripRedistributeAdd (RIB_ROUTE_TYPE_CONNECT, RIP_ROUTE_INTERFACE,
                            &address, pConnected->pIf->ifIndex, 
                            NULL, 0, 0);
    } 
    else
    {
      ripRedistributeDelete (RIB_ROUTE_TYPE_CONNECT, RIP_ROUTE_INTERFACE,
                             &address, pConnected->pIf->ifIndex);
      if (ripRedistributeCheck (RIB_ROUTE_TYPE_CONNECT))
        ripRedistributeAdd (RIB_ROUTE_TYPE_CONNECT, RIP_ROUTE_REDISTRIBUTE,
                            &address, pConnected->pIf->ifIndex,
                            NULL, 0, 0);
    }
  }

}

/* Update interface status. */
void
ripEnableApply (InterfaceT * pIf)
{
  Int32T ret = 0;
  RipInterfaceT *pRIf = NULL;

  /* Check interface. */
  if (! ifIsOperative (pIf))
  {
    return;
  }

  pRIf = pIf->pInfo;

  /* Check network configuration. */
  ret = ripEnableNetworkLookupIf (pIf);

  /* If the interface is matched. */
  if (ret > 0)
  {
    pRIf->enableNetwork = 1;
  }
  else
  {
    pRIf->enableNetwork = 0;
  }

  /* Check interface name configuration. */
  ret = ripEnableIfLookup (pIf->name);
  if (ret >= 0)
  {
    pRIf->enableInterface = 1;
  }
  else
  {
    pRIf->enableInterface = 0;
  }

  /* any interface MUST have an IPv4 address */
  if ( ! ripIfIpv4AddressCheck (pIf) )
  {
    pRIf->enableNetwork = 0;
    pRIf->enableInterface = 0;
  }

  /* Update running status of the interface. */
  if (pRIf->enableNetwork || pRIf->enableInterface)
  {
	{
	if (IS_RIP_DEBUG_EVENT)
      NNLOG(LOG_DEBUG,"turn on %s\n", pIf->name);

    /* Set interface wake up and request message */
    ripInterfaceWakeup(pIf); // add by sckim

    ripConnectSet (pIf, 1);
	}
  }
  else
  {
    if (pRIf->running)
    {
	  /* Might as well clean up the route table as well
	   * ripIfDown sets to 0 pRIf->running, and displays "turn off %s"
	   **/ 
      ripIfDown(pIf);

      ripConnectSet (pIf, 0);
    }
  }
}

/* Apply network configuration to all interface. */
void
ripEnableApplyAll ()
{
  ListNodeT * pNode = NULL;
  InterfaceT * pIf = NULL;

  /* Check each interface. */
  for(pNode = pRip->pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pIf = pNode->pData;
    ripEnableApply (pIf);
  }
}

Int32T
ripNeighborLookup (struct sockaddr_in *pFrom)
{
  Prefix4T p = {0,};
  RouteNodeT * pRNode = NULL;

  /* Check global rip data structure pointer. */
  if (!pRip)
    return 0;

  /* Check neighbor table pointer. */
  if (!pRip->pNeighbor)
    return 0;

  memset (&p, 0, sizeof (Prefix4T));
  p.family = AF_INET;
  p.prefix = pFrom->sin_addr;
  p.prefixLen = NN_IPV4_MAX_BITLEN;

  pRNode = nnRouteNodeLookup (pRip->pNeighbor, (PrefixT *) &p);
  if (pRNode)
  {
    nnRouteNodeUnlock (pRNode);
    return 1;
  }

  return 0;
}

/* Add new RIP neighbor to the neighbor tree. */
Int32T
ripNeighborAdd (Prefix4T *pPrefix4)
{
  RouteNodeT * pNode = NULL;
  PrefixT prefix = {0,};

  /* Check global rip data structure pointer. */
  if (!pRip)
    return 0;

  /* Check neighbor table pointer. */
  if (!pRip->pNeighbor)
    return 0;

  /* Convert Prefix4T to PrefixT data structure. */
  nnCnvPrefix4TtoPrefixT (&prefix, pPrefix4); 
  
  /* Look up node & lock node. */
  pNode = nnRouteNodeGet (pRip->pNeighbor, &prefix);

  if (pNode->pInfo)
  {
    return -1;
  }

  pNode->pInfo = pRip->pNeighbor;

  return 0;
}

/* Delete RIP neighbor from the neighbor tree. */
Int32T
ripNeighborDelete (Prefix4T *pPrefix4)
{
  RouteNodeT * pNode = NULL;
  PrefixT prefix = {0,};

  /* Check global rip data structure pointer. */
  if (!pRip)
    return 0;

  /* Check neighbor table pointer. */
  if (!pRip->pNeighbor)
    return 0;

  /* Convert Prefix4T to PrefixT data structure. */
  nnCnvPrefix4TtoPrefixT (&prefix, pPrefix4); 

  /* Look up node & lock node. */
  pNode = nnRouteNodeLookup (pRip->pNeighbor, &prefix);
  if (! pNode)
  {
    return -1;
  }
  
  pNode->pInfo = NULL;

  /* Unlock lookup lock. */
  nnRouteNodeUnlock (pNode);

  /* Unlock real neighbor information lock. */
  nnRouteNodeUnlock (pNode);

  return 0;
}

/* Clear all network and neighbor configuration. */
void
ripNetworkClean ()
{
  Uint32T i = 0;
  char *str;
  RouteNodeT *pRNode;

  /* Check rip global data structure pointer. */
  if (!pRip)
    return;

  /* pRip->pRipEnableNetwork. */
  if (pRip->pRipEnableNetwork)
  {
    for (pRNode = nnRouteTop (pRip->pRipEnableNetwork);
         pRNode;
         pRNode = nnRouteNext (pRNode))
    {
      if (pRNode->pInfo)
      {
        pRNode->pInfo = NULL;
        nnRouteNodeUnlock (pRNode);
      }
    }

    /* Network table free. */
    nnRouteTableFinish (pRip->pRipEnableNetwork);
    pRip->pRipEnableNetwork = NULL;
  }


  /* pRip->ripEnableInterface. */
  if (pRip->ripEnableInterface)
  {
    for (i = 0; i < vectorActive (pRip->ripEnableInterface); i++)
    {
      if ((str = vectorSlot (pRip->ripEnableInterface, i)) != NULL)
      {
        NNFREE (MEM_IF_NAME, str);
        vectorSlot (pRip->ripEnableInterface, i) = NULL;
      }
    }

    /* Interface vector free. */
    vectorFree (pRip->ripEnableInterface);
    pRip->ripEnableInterface = NULL;
  }
}

/* Utility function for looking up passive interface settings. */
static Int32T
ripPassiveNonDefaultLookup (const char * ifName)
{
  Uint32T i = 0;
  char * str;

  for (i = 0; i < vectorActive (pRip->vripPassiveNonDefault); i++)
    if ((str = vectorSlot (pRip->vripPassiveNonDefault, i)) != NULL)
      if (strcmp (str, ifName) == 0)
      {
        return i;
      }

  return -1;
}

void
ripPassiveInterfaceApply (InterfaceT * pIf)
{
  RipInterfaceT * pRIf = NULL;

  pRIf = pIf->pInfo;

  pRIf->passive = ((ripPassiveNonDefaultLookup (pIf->name) < 0) ?
		 pRip->passiveDefault : !pRip->passiveDefault);

  if (IS_RIP_DEBUG_RIBMGR)
    NNLOG(LOG_DEBUG,"interface %s: passive = %d", pIf->name, pRIf->passive);
}

static void
ripPassiveInterfaceApplyAll (void)
{
  ListNodeT * pNode = NULL;
  InterfaceT *pIf = NULL;

  for(pNode = pRip->pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pIf = pNode->pData;
    ripPassiveInterfaceApply (pIf);
  }
}

/* Passive interface. */
Int32T
ripPassiveNonDefaultSet (const StringT ifName)
{

  if (ripPassiveNonDefaultLookup (ifName) >= 0)
  {
    return FAILURE;
  }

  vectorSet (pRip->vripPassiveNonDefault, nnStrDup (ifName, MEM_IF_NAME));

  ripPassiveInterfaceApplyAll ();

  return SUCCESS;
}

Int32T
ripPassiveNonDefaultUnset (const StringT ifName)
{
  Int32T i = 0;
  char *str;

  i = ripPassiveNonDefaultLookup (ifName);
  if (i < 0)
  {
    return FAILURE;
  }

  str = vectorSlot (pRip->vripPassiveNonDefault, i);
  NNFREE (MEM_IF_NAME, str);
  vectorUnset (pRip->vripPassiveNonDefault, i);

  ripPassiveInterfaceApplyAll ();

  return SUCCESS;
}

/* Free all configured RIP passive-interface settings. */
void
ripPassiveNonDefaultClean (void)
{
  Uint32T i = 0;
  char * str;

  if (!pRip)
    return;

  if (!pRip->vripPassiveNonDefault)
    return;
  
  for (i = 0; i < vectorActive (pRip->vripPassiveNonDefault); i++)
  {
    if ((str = vectorSlot (pRip->vripPassiveNonDefault, i)) != NULL)
    {
      NNFREE (MEM_IF_NAME, str);
      vectorSlot (pRip->vripPassiveNonDefault, i) = NULL;
    }
  }

  ripPassiveInterfaceApplyAll ();

  vectorFree (pRip->vripPassiveNonDefault);
  pRip->vripPassiveNonDefault = NULL;
}

/* Write rip configuration of each interface. */
Int32T
configWriteRipInterface (struct cmsh *cmsh)
{
  ListNodeT * pNode = NULL;
  InterfaceT *pIf = NULL;

  for(pNode = pRip->pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    RipInterfaceT *pRIf = NULL;
 
    pIf = pNode->pData; 
    pRIf = pIf->pInfo;

    /* Do not display the interface if there is no
     * configuration about it.
     **/
    if ((!pIf->desc)                                  &&
        (pRIf->splitHorizon == pRIf->splitHorizonDefault) &&
        (pRIf->riSend == RI_RIP_UNSPEC)                 &&
        (pRIf->riReceive == RI_RIP_UNSPEC)              &&
        (pRIf->authType != RIP_AUTH_MD5)                &&
        (pRIf->md5AuthLen != RIP_AUTH_MD5_SIZE)         &&
        (!pRIf->authStr)                                &&
        (!pRIf->keyChain)                               )
      continue;

    cmdPrint (cmsh, "!");
    cmdPrint (cmsh, "interface %s", pIf->name);

    if (pIf->desc)
    {
      cmdPrint (cmsh, " description %s", pIf->desc);
    }

    /* Split horizon. */
    if (pRIf->splitHorizon != pRIf->splitHorizonDefault)
    {
      switch (pRIf->splitHorizon)
      {
        case RIP_SPLIT_HORIZON:
          cmdPrint (cmsh, " ip rip split-horizon");
          break;
        case RIP_SPLIT_HORIZON_POISONED_REVERSE:
          cmdPrint (cmsh, " ip rip split-horizon poisoned-reverse");
          break;
        case RIP_NO_SPLIT_HORIZON:
        default:
          cmdPrint (cmsh, " no ip rip split-horizon");
          break;
      }
    }

    /* RIP version setting. */
    if (pRIf->riSend != RI_RIP_UNSPEC)
    {
      cmdPrint (cmsh, " ip rip send version %s",
		       lookupStr (ri_version_msg, pRIf->riSend));
    }

    if (pRIf->riReceive != RI_RIP_UNSPEC)
    {
      cmdPrint (cmsh, " ip rip receive version %s",
               lookupStr (ri_version_msg, pRIf->riReceive));
    }

    /* RIP authentication. */
    if (pRIf->authType == RIP_AUTH_SIMPLE_PASSWORD)
    {
      cmdPrint (cmsh, " ip rip authentication mode text");
    }

    if (pRIf->authType == RIP_AUTH_MD5)
    {
      Int8T strBuff[RIP_BUFFER_MAX_SIZE] = {};
      Int32T len = 0;
      len = sprintf (strBuff, " ip rip authentication mode md5");
      if (pRIf->md5AuthLen == RIP_AUTH_MD5_COMPAT_SIZE)
      {
        sprintf (strBuff + len, " auth-length old-ripd");
      }
      else 
      {
        sprintf (strBuff + len, " auth-length rfc");
      }

      cmdPrint (cmsh, "%s", strBuff);
    }

    if (pRIf->authStr)
    {
      cmdPrint (cmsh, " ip rip authentication string %s", pRIf->authStr);
    }

    if (pRIf->keyChain)
    {
      cmdPrint (cmsh, " ip rip authentication key-chain %s", pRIf->keyChain);
    }
  }

  return 0;
}


Int32T
configWriteRipNetwork (struct cmsh *cmsh, Int32T configMode)
{
  Uint32T i;
  char *ifName;
  RouteNodeT *pRNode;

  /* Network type RIP enable interface statement. */
  if (pRip->pRipEnableNetwork)
  {
    for (pRNode = nnRouteTop (pRip->pRipEnableNetwork); 
         pRNode; 
         pRNode = nnRouteNext (pRNode))
    {
      if (pRNode->pInfo)
      {
        cmdPrint (cmsh, "%s%s/%d", 
                  configMode ? " network " : "    ",
                  inet_ntoa (pRNode->p.u.prefix4), pRNode->p.prefixLen);
      }
    }
  }

  /* Interface name RIP enable statement. */
  if (pRip->ripEnableInterface)
  {
    for (i = 0; i < vectorActive (pRip->ripEnableInterface); i++)
    {
      if ((ifName = vectorSlot (pRip->ripEnableInterface, i)) != NULL)
      {
        cmdPrint (cmsh, "%s%s", configMode ? " network " : "    ", ifName);
      }
    }
  }

  /* RIP neighbors listing. */
  if (pRip->pNeighbor)
  {
    for (pRNode = nnRouteTop (pRip->pNeighbor); 
         pRNode; 
         pRNode = nnRouteNext (pRNode))
    {
      if (pRNode->pInfo)
      {
        cmdPrint (cmsh, "%s%s", 
                 configMode ? " neighbor " : "    ",
                 inet_ntoa (pRNode->p.u.prefix4));
      }
    }
  }

  /* RIP passive interface listing. */
  if (configMode) 
  {
    if (pRip->passiveDefault)
    {
      cmdPrint (cmsh, " passive-interface default");
    }

    if (pRip->vripPassiveNonDefault)
    {
      for (i = 0; i < vectorActive (pRip->vripPassiveNonDefault); i++)
      {
        if ((ifName = vectorSlot (pRip->vripPassiveNonDefault, i)) != NULL)
        {
          cmdPrint (cmsh, " %spassive-interface %s",
		            (pRip->passiveDefault ? "no " : ""), ifName);
        }
      }
    }
  }

  return 0;
}


Int32T
ripNetworkDisplay (struct cmsh * cmsh)
{
  Uint32T i = 0;
  char * ifName = NULL;
  RouteNodeT * pRNode = NULL;

  /* Network type RIP enable interface statement. */
  if (pRip->pRipEnableNetwork)
  {
    for (pRNode = nnRouteTop (pRip->pRipEnableNetwork); 
         pRNode; 
         pRNode = nnRouteNext (pRNode))
    {
      if (pRNode->pInfo)
        cmdPrint (cmsh, "%s%s/%d\n", 
	         "    ", inet_ntoa (pRNode->p.u.prefix4), pRNode->p.prefixLen);
    }
  }

  /* Interface name RIP enable statement. */
  if (pRip->ripEnableInterface)
  {
    for (i = 0; i < vectorActive (pRip->ripEnableInterface); i++)
    {
      if ((ifName = vectorSlot (pRip->ripEnableInterface, i)) != NULL)
        cmdPrint (cmsh, "%s%s\n", "    ", ifName);
    }
  }

  /* RIP neighbors listing. */
  if (pRip->pNeighbor)
  {
    for (pRNode = nnRouteTop (pRip->pNeighbor); 
         pRNode; 
         pRNode = nnRouteNext (pRNode))
    {
      if (pRNode->pInfo)
        cmdPrint (cmsh, "%s%s\n", "    ", inet_ntoa (pRNode->p.u.prefix4));
    }
  }

  return 0;
}


/* Called when interface structure allocated. */
static Int32T
ripInterfaceNewHook (InterfaceT * pIf)
{
  pIf->pInfo = ripInterfaceNew ();

  return 0;
}

/* Called when interface structure deleted. */
static Int32T
ripInterfaceDeleteHook (InterfaceT * pIf)
{
  NNFREE (MEM_IF, pIf->pInfo);
  pIf->pInfo = NULL;

  return 0;
}

/* Allocate and initialize interface vector. */
void
ripIfInit (void)
{
  /* Default initial size of interface vector. */
  if (!pRip->pIfList)
  {
    pRip->pIfList = ifInit();

    /* Hooking callback functions. */
    ifAddHook (IF_NEW_HOOK, ripInterfaceNewHook);
    ifAddHook (IF_DELETE_HOOK, ripInterfaceDeleteHook);
  }
  
  /* RIP network init. */
  pRip->ripEnableInterface = vectorInit (1);
  pRip->pRipEnableNetwork = nnRouteTableInit ();

  /* RIP passive interface. */
  pRip->vripPassiveNonDefault = vectorInit (1);
}


/* Allocate and initialize interface vector. */
void
ripIfVersionUpdate (void)
{
  /* Assign interface list pointer. */
  ifUpdate (pRip->pIfList);

  /* Hooking callback functions. */
  ifAddHook (IF_NEW_HOOK, ripInterfaceNewHook);
  ifAddHook (IF_DELETE_HOOK, ripInterfaceDeleteHook);
}
