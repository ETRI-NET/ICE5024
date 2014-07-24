/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : ribmgr에서 사용하는 Interface 정보를 저장 관리 하는 기능을 수행한다.
 * - Block Name : RIB Manager
 * - Process Name : ribmgr
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : ribmgrInterface.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include <net/if.h>

#include "nnRibDefines.h"
#include "nnTypes.h"
#include "nnList.h"
#include "nnIf.h"
#include "nnPrefix.h"
#include "nnTable.h"
#include "nosLib.h"

#include "ribmgrInit.h"
#include "ribmgrConnected.h"
#include "ribmgrInterface.h"
#include "ribmgrRib.h"
#include "ribmgrRouterid.h"
#include "ribmgrRedistribute.h"
#include "ribmgrDebug.h"



/*
 * Description : 인터페이스에 플래그 값을 설정하는 함수.
 *
 * @param [in] pIf : 인터페이스 포인터
 * @param [in] flags : 인터페이스 포인터
 *
 * @retval : 0 always
 */
Int32T
ifSetFlags (InterfaceT *pIf, uint64_t flags)
{
  NNLOG (LOG_DEBUG, "INTERFACE : %s called. This is null function.\n", __func__);

  SET_FLAG (pIf->flags, flags);

  return 0;
}


/*
 * Description : 인터페이스에 플래그 값을 삭제하는 함수.
 *
 * @param [in] pIf : 인터페이스 포인터
 * @param [in] flags : 인터페이스 포인터
 *
 * @retval : 0 always
 */
Int32T
ifUnsetFlags (InterfaceT *pIf, uint64_t flags)
{
  NNLOG (LOG_DEBUG, "%s called. This is null function.\n", __func__);
  return 0;
}


/*
 * Description : 인터페이스에 플래그 값을 얻는 함수.
 *
 * @param [in] pIf : 인터페이스 포인터
 */
void
ifGetFlags (InterfaceT *pIf)
{
  NNLOG (LOG_DEBUG, "%s called. This is null function.\n", __func__);
}


/*
 * Description : 인터페이스에 프리픽스 값을 설정하는 함수.
 *
 * @param [in] pIf : 인터페이스 포인터
 * @param [in] pIfc : connected 포인터
 *
 * @retval : 0 always
 */
Int32T 
ifSetPrefix (InterfaceT *pIf, ConnectedT *pIfc)
{
  NNLOG (LOG_DEBUG, "%s called. This is null function.\n", __func__);
  return 0;
}

#ifdef HAVE_IPV6
/*
 * Description : 인터페이스에 IPv6 주소를 설정하는 함수.
 *
 * @param [in] pIf : 인터페이스 포인터
 * @param [in] pIfc : connected 포인터
 *
 * @retval : 0 always
 */
Int32T
ifPrefixAddIpv6 (InterfaceT *pIf, ConnectedT *pIfc)
{
  NNLOG (LOG_DEBUG, "%s called. This is null function.\n", __func__);
  return 0;
}
#endif //HAVE_IPV6


/* Description : 인터페이스 정보를 후킹 설정하는 함수. */
Int32T
ifRibmgrNewHook (InterfaceT *pIf)
{
  NNLOG (LOG_DEBUG, "Testing : %s Called \n", __func__);
  return 0;
}


/* Description : 인터페이스 정보를 후킹 해제하는 함수. */
Int32T
ifRibmgrDeleteHook (InterfaceT *pIf)
{
  NNLOG(LOG_DEBUG,  "Testing : %s Called \n", __FUNCTION__);
  return 0;
}


/* Description : 인터페이스에 서브 네트워크를 추가하는 함수. */
Int32T
ifSubnetAdd (InterfaceT *pIf, ConnectedT *pIfc)
{
  PrefixT cp = {0,};
  RouteNodeT * pRNode = NULL;
  RibmgrIfT  * pRIf = NULL;
  ListT * pAddrList = NULL;

  assert (pIf);
  assert (pIf->pInfo);
  assert (pIfc);
  assert (pIf && pIf->pInfo && pIfc);
  pRIf = pIf->pInfo;

  /* Get address derived subnet node and associated address list, while marking
     address secondary attribute appropriately. */
  cp = *pIfc->pAddress;
  nnApplyNetmasktoPrefix (&cp, &cp.prefixLen);
  pRNode = nnRouteNodeGet (pRIf->ipv4Subnets, &cp);

  if ((pAddrList = pRNode->pInfo))
  {
    SET_FLAG (pIfc->flags, RIBMGR_IFA_SECONDARY);
  }
  else
  {
    UNSET_FLAG (pIfc->flags, RIBMGR_IFA_SECONDARY);
    pRNode->pInfo = pAddrList = nnListInit(NULL, MEM_CONNECTED);
    nnRouteNodeLock (pRNode);
  }

  /* Tie address at the tail of address list. */
  nnListAddNode(pAddrList, pIfc);
  
  /* Return list element count. */
  return nnListCount(pAddrList);
}


/* Description : 인터페이스에 서브 네트워크를 삭제하는 함수. */
Int32T
ifSubnetDelete (InterfaceT *pIf, ConnectedT *pIfc)
{
  RouteNodeT *pRNode = NULL;
  RibmgrIfT * pRIf = NULL;
  ListT * pAddrList = NULL;


  assert (pIf && pIfc);

  if(!pIf->pInfo)
  {
    NNLOG (LOG_ERR, "pIf->info is NULL. so return -1. \n");
    return -1;
  }

  pRIf = pIf->pInfo;

  /* Get address derived subnet node. */
  pRNode = nnRouteNodeLookup (pRIf->ipv4Subnets, pIfc->pAddress);
  if (! (pRNode && pRNode->pInfo))
  {
    return -1;
  }
  nnRouteNodeUnlock (pRNode);
  
  /* Untie address from subnet's address list. */
  pAddrList = pRNode->pInfo;
  nnListDeleteNode (pAddrList, pIfc);
  nnRouteNodeUnlock (pRNode);

  /* Return list element count, if not empty. */
  if (nnListCount(pAddrList))
  {
    /* If deleted address is primary, mark subsequent one as such and distribute. */
    if (! CHECK_FLAG (pIfc->flags, RIBMGR_IFA_SECONDARY))
    {
      pIfc = pAddrList->pHead->pData;

      // add by sckim for router-id
      routerIdDelAddress(pIfc);
      routerIdAddAddress(pIfc);

      ribmgrInterfaceAddressDeleteUpdate (pIf, pIfc);
      UNSET_FLAG (pIfc->flags, RIBMGR_IFA_SECONDARY);
      ribmgrInterfaceAddressAddUpdate (pIf, pIfc);
    }
      
    return nnListCount(pAddrList);
  }
  
  /* Otherwise, free list and route node. */
  nnListFree (pAddrList);
  pRNode->pInfo = NULL;
  nnRouteNodeUnlock (pRNode);

  return 0;
}

/* ifFlagsMangle: A place for hacks that require mangling
 * or tweaking the interface flags.
 *
 * ******************** Solaris flags hacks **************************
 *
 * Solaris IFF_UP flag reflects only the primary interface as the
 * routing socket only sends IFINFO for the primary interface.  Hence  
 * ~IFF_UP does not per se imply all the logical interfaces are also   
 * down - which we only know of as addresses. Instead we must determine
 * whether the interface really is up or not according to how many   
 * addresses are still attached. (Solaris always sends RTM_DELADDR if
 * an interface, logical or not, goes ~IFF_UP).
 *
 * Ie, we mangle IFF_UP to *additionally* reflect whether or not there
 * are addresses left in ConnectedT, not just the actual underlying
 * IFF_UP flag.
 *
 * We must hence remember the real state of IFF_UP, which we do in
 * RibmgrIfT.primary_state.
 *
 * Setting IFF_UP within ribmgr to administratively shutdown the
 * interface will affect only the primary interface/address on Solaris.
 ************************End Solaris flags hacks ***********************
 */
#if 0
static inline void
ifFlagsMangle (InterfaceT *pIf, uint64_t *newflags)
{
#ifdef SUNOS_5
  RibmgrIfT *rIf = pIf->info;
  
  rIf->primary_state = *newflags & (IFF_UP & 0xff);
  
  if (CHECK_FLAG (rIf->primary_state, IFF_UP)
      || listcount(pIf->connected) > 0)
    SET_FLAG (*newflags, IFF_UP);
  else
    UNSET_FLAG (*newflags, IFF_UP);
#endif /* SUNOS_5 */
}

/* Update the flags field of the pIf with the new flag set provided.
 * Take whatever actions are required for any changes in flags we care
 * about.
 *
 * newflags should be the raw value, as obtained from the OS.
 */
void
ifFlagsUpdate (InterfaceT *pIf, uint64_t newflags)
{
  ifFlagsMangle (pIf, &newflags);
    
  if (ifIsOperative (pIf))
    {
      /* operative -> inoperative? */
      pIf->flags = newflags;
      if (!ifIsOperative (pIf))
        ifStateDown (pIf);
    }
  else
    {
      /* inoperative -> operative? */
      pIf->flags = newflags;
      if (ifIsOperative (pIf))
        ifStateUp (pIf);
    }
}
#endif


/*
 * Description : 인터페이스에 설정된 주소가 현재 커널에 설정된 주소가 아닌
 * 경우, 운용자에 의해 설정된 주소를 활성화 하는 함수.
 * Wake up configured address if it is not in current kernel address.
 *
 * param [in] pIf : 인터페이스 포인터
 */
static void
ifAddrWakeup (InterfaceT *pIf)
{
  ConnectedT *pIfc = NULL;
  PrefixT * pPrefix = NULL;
  Int32T ret = 0;

  NNLOG(LOG_DEBUG, "+--------------------------------\n");
  NNLOG(LOG_DEBUG, "| %s called.\n", __func__);
  NNLOG(LOG_DEBUG, "+--------------------------------\n");

  ListT * pConnectedList = pIf->pConnected;
  ListNodeT * pConnectedNode = NULL;

  for (pConnectedNode = pConnectedList->pHead;
       pConnectedNode != NULL;
       pConnectedNode = pConnectedNode->pNext)
  {
    pIfc = pConnectedNode->pData;
    pPrefix = pIfc->pAddress;

    if (CHECK_FLAG (pIfc->conf, RIBMGR_IFC_CONFIGURED) && 
        ! CHECK_FLAG (pIfc->conf, RIBMGR_IFC_REAL))
    {
      /* Address check. */
      if (pPrefix->family == AF_INET)
      {
        if (! ifIsUp (pIf))
        {
          /* XXX: WTF is it trying to set flags here?
          * caller has just gotten a new interface, has been
          * handed the flags already. This code has no business
          * trying to override administrative status of the interface.
          * The only call path to here which doesn't originate from
          * kernel event is irdp - what on earth is it trying to do?
          *
          * further RUNNING is not a settable flag on any system
          * I (paulj) am aware of.
          */
          ifSetFlags (pIf, IFF_UP | IFF_RUNNING);
          ifRefresh (pIf);
        }

        ret = ifSetPrefix (pIf, pIfc);
        if (ret < 0)
        {
          NNLOG(LOG_DEBUG,"Error] Can't set interface's address: %s\n", __func__);
          continue;
        }

        /* Add to subnet chain list. */
        ifSubnetAdd (pIf, pIfc);

        SET_FLAG (pIfc->conf, RIBMGR_IFC_REAL);

        ribmgrInterfaceAddressAddUpdate (pIf, pIfc);

        if (ifIsOperative(pIf))
        {
          connectedUpIpv4 (pIf, pIfc);
        }
      }
#ifdef HAVE_IPV6
      if (pPrefix->family == AF_INET6)
      {
        if (! ifIsUp (pIf))
        {
          /* XXX: See long comment above */
          ifSetFlags (pIf, IFF_UP | IFF_RUNNING);
          ifRefresh (pIf);
        }

        ret = ifPrefixAddIpv6 (pIf, pIfc);
        if (ret < 0)
        {
          NNLOG(LOG_DEBUG,"Error] Can't set interface's address: %s\n", __func__);
          continue;
        }
        SET_FLAG (pIfc->conf, RIBMGR_IFC_REAL);

        ribmgrInterfaceAddressAddUpdate (pIf, pIfc);

        if (ifIsOperative(pIf))
        {
          connectedUpIpv6 (pIf, pIfc);
        }
      }
#endif /* HAVE_IPV6 */
	}
  }

}



/* Description : 인터페이스가 추가를 처리하는 함수. */
void
ifAddUpdate (InterfaceT *pIf)
{
  NNLOG(LOG_DEBUG, "+--------------------------------\n");
  NNLOG(LOG_DEBUG, "| %s called.\n", __func__);
  NNLOG(LOG_DEBUG, "+--------------------------------\n");

#if 0
  RibmgrIfT * ifData;

  ifData = pIf->info;
  if (ifData->multicast == IF_RIBMGR_MULTICAST_ON)
  {
    ifSetFlags (pIf, IFF_MULTICAST);
  }
  else if (ifData->multicast == IF_RIBMGR_MULTICAST_OFF)
  {
    ifUnsetFlags (pIf, IFF_MULTICAST);
  }
  else
  {
    NNLOG(LOG_ERR, "Wrong multicast field = %d\n", ifData->multicast);
  }
#endif

  ribmgrInterfaceAddUpdate (pIf);

  if (! CHECK_FLAG (pIf->status, RIBMGR_INTERFACE_ACTIVE))
  {
    NNLOG (LOG_DEBUG, "interface %s index %d becomes active.\n", 
           pIf->name, pIf->ifIndex);

    SET_FLAG (pIf->status, RIBMGR_INTERFACE_ACTIVE);
    ifAddrWakeup (pIf);

    if (IS_RIBMGR_DEBUG_KERNEL)
    {
      NNLOG (LOG_DEBUG, "interface %s index %d becomes active.\n", 
             pIf->name, pIf->ifIndex);
    }
  }
  else
  {
    NNLOG (LOG_DEBUG, "interface %s index %d is added..\n", 
           pIf->name, pIf->ifIndex);
    if (IS_RIBMGR_DEBUG_KERNEL)
    {
      NNLOG (LOG_DEBUG, "interface %s index %d is added.\n", 
             pIf->name, pIf->ifIndex);
    }
  }
}


/* Description : 인터페이스가 삭제 이벤트를 처리하는 함수. */
void 
ifDeleteUpdate (InterfaceT *pIf)
{
  ConnectedT *pIfc = NULL;
  RouteNodeT *pRNode = NULL;
  RibmgrIfT * pRIf = NULL;
  PrefixT * pPrefix = NULL;

  NNLOG(LOG_DEBUG, "+--------------------------------\n");
  NNLOG(LOG_DEBUG, "| %s called.\n", __func__);
  NNLOG(LOG_DEBUG, "+--------------------------------\n");

  pRIf = pIf->pInfo;

  if (ifIsUp(pIf))
  {
    NNLOG (LOG_ERR, "interface %s index %d is still up while being deleted.\n",
	       pIf->name, pIf->ifIndex);
    return;
  }

  /* Mark interface as inactive */
  UNSET_FLAG (pIf->status, RIBMGR_INTERFACE_ACTIVE);
  
  if (IS_RIBMGR_DEBUG_KERNEL)
  {
    NNLOG (LOG_DEBUG, "interface %s index %d is now inactive.\n", 
           pIf->name, pIf->ifIndex);
  }

  /* Delete connected routes from the kernel. */
  if (pIf->pConnected)
  {
    ListT * pConnectedList = pIf->pConnected;
    ListNodeT * pConnectedNode;
    ListNodeT * pLastNode = NULL;

    while ((pConnectedNode = (pLastNode ? 
                              pLastNode->pNext : pConnectedList->pHead)))
	{
	  pIfc = pConnectedNode->pData;
	  pPrefix = pIfc->pAddress;
	  
      if ( pPrefix->family == AF_INET && 
          (pRNode = nnRouteNodeLookup (pRIf->ipv4Subnets, pPrefix)))
      {
        ListNodeT * pNode = NULL;
        ListNodeT * pNextNode = NULL;
        ListNodeT * pFirstNode = NULL;
        ListT * pAddrList = NULL;
	      
        nnRouteNodeUnlock (pRNode);
        pAddrList = (ListT *) pRNode->pInfo;
	      
        /* Remove addresses, secondaries first. */
        pFirstNode = pAddrList->pHead;
        for (pNode = pFirstNode->pNext; pNode || pFirstNode; pNode = pNextNode)
        {
          if (!pNode)
          {
            pNode = pFirstNode;
            pFirstNode = NULL;
          }
          pNextNode = pNode->pNext;

          pIfc = pNode->pData;
          pPrefix = pIfc->pAddress;

          connectedDownIpv4 (pIf, pIfc);

          ribmgrInterfaceAddressDeleteUpdate (pIf, pIfc);

          UNSET_FLAG (pIfc->conf, RIBMGR_IFC_REAL);

          /* Remove from subnet chain. */
          nnListDeleteNode(pAddrList, pNode);
          nnRouteNodeUnlock (pRNode);
		  
		  /* Remove from interface address list (unconditionally). */
          if (!CHECK_FLAG (pIfc->conf, RIBMGR_IFC_CONFIGURED))
          {
            nnListDeleteNode(pIf->pConnected, pIfc);
            connectedFree (pIfc);
          }
          else
          {
            pLastNode = pConnectedNode;
          }
        } /* for loop */

        /* Free chain list and respective route node. */
        nnListFree (pAddrList);
        pRNode->pInfo = NULL;
        nnRouteNodeUnlock (pRNode);
      }
#ifdef HAVE_IPV6
      else if (pPrefix->family == AF_INET6)
      {
        connectedDownIpv6 (pIf, pIfc);

        ribmgrInterfaceAddressDeleteUpdate (pIf, pIfc);

        UNSET_FLAG (pIfc->conf, RIBMGR_IFC_REAL);

        if (CHECK_FLAG (pIfc->conf, RIBMGR_IFC_CONFIGURED))
        {
          pLastNode = pConnectedNode;
        }
        else
        {
          nnListDeleteNode (pIf->pConnected, pIfc);
          connectedFree (pIfc);
        }
      }
#endif /* HAVE_IPV6 */
	  else
      {
        pLastNode = pConnectedNode;
      }
    }
  }

  ribmgrInterfaceDeleteUpdate (pIf);

  /* Update ifIndex after distributing the delete message.  This is in
     case any client needs to have the old value of ifIndex available
     while processing the deletion.  Each client daemon is responsible
     for setting ifIndex to IFINDEX_INTERNAL after processing the
     interface deletion message. */
  pIf->ifIndex = IFINDEX_INTERNAL;
}


/* description : 인터페이스가 상태의 up 이벤트를 처리하는 함수. */
void
ifStateUp (InterfaceT *pIf)
{
  ConnectedT *pIfc = NULL;
  ListNodeT * pConnectedNode = NULL;
  PrefixT * pPrefix = NULL;

  NNLOG(LOG_DEBUG, "+--------------------------------\n");
  NNLOG(LOG_DEBUG, "| %s called.\n", __func__);
  NNLOG(LOG_DEBUG, "+--------------------------------\n");

  /* Notify the protocol daemons. */
  ribmgrInterfaceUpUpdate (pIf);

  /* Install connected routes to the kernel. */
  if (pIf->pConnected)
  {
    ListT * pConnectedList = pIf->pConnected;
    for (pConnectedNode = pConnectedList->pHead;
         pConnectedNode != NULL;
         pConnectedNode = pConnectedNode->pNext)
    {
      pIfc = pConnectedNode->pData;
      pPrefix = pIfc->pAddress;

      if (pPrefix->family == AF_INET)
      {
        connectedUpIpv4 (pIf, pIfc);
      }
#ifdef HAVE_IPV6
      else if (pPrefix->family == AF_INET6)
      {
        connectedUpIpv6 (pIf, pIfc);
      }
#endif /* HAVE_IPV6 */
      else
      {
        NNLOG(LOG_ERR, "Wrong family = %d\n", pPrefix->family);
      }
    } /* for loop */
  } /* if */

  /* Examine all static routes. */
  ribUpdate ();
}


/* description : 인터페이스가 상태의 DOWN 이벤트를 처리하는 함수. */
void
ifStateDown (InterfaceT *pIf)
{
  ConnectedT *pIfc = NULL;
  ListNodeT * pConnectedNode = NULL;
  PrefixT * pPrefix = NULL;

  NNLOG(LOG_DEBUG, "+--------------------------------\n");
  NNLOG(LOG_DEBUG, "| %s called.\n", __func__);
  NNLOG(LOG_DEBUG, "+--------------------------------\n");

  /* Notify to the protocol daemons. */
  ribmgrInterfaceDownUpdate (pIf);

  /* Delete connected routes from the kernel. */
  if (pIf->pConnected)
  {
    ListT * pConnectedList = pIf->pConnected;
    for (pConnectedNode = pConnectedList->pHead;
         pConnectedNode != NULL;
         pConnectedNode = pConnectedNode->pNext)
    {
      pIfc = pConnectedNode->pData;
      pPrefix = pIfc->pAddress;

      if (pPrefix->family == AF_INET)
      {
        connectedDownIpv4 (pIf, pIfc);
      }
#ifdef HAVE_IPV6
      else if (pPrefix->family == AF_INET6)
      {
        connectedDownIpv6 (pIf, pIfc);
      }
#endif /* HAVE_IPV6 */
      else
      {
        NNLOG(LOG_ERR, "Wrong family = %d\n", pPrefix->family);
      }
    } /* for loop */
  } /* if */

  /* Examine all static routes which direct to the interface. */
  ribUpdate ();
}


/**
 * description : 인터페이스의 플래그를 갱신하는 함수.
 *
 * @param [in] pIf : 인터페이스 포인터
 */
void
ifRefresh (InterfaceT *pIf)
{
  ifGetFlags (pIf);
}
