#if !defined(_pifMsgEvent_h)
#define _pifMsgEvent_h
/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/
  
/**   
 * @brief : This file defines the interface related definitions.
 *  - Block Name : PIF Manager
 *  - Process Name : pifmgr
 *  - Creator : Seungwoo Hong
 *  - Initial Date : 2013/10/10
 */
  
/**
 * @file : 
 *
 * $Author: $
 * $Date: $
 * $Revision: $
 * $LastChangedBy: $
 */

#include "nosLib.h"
#include "pifMsgEventData.h"
#include "pifDataTypes.h"

void pifEventPost(void* obj, Uint32T mgrId, Uint32T msgId);
void pifPostEventIPInterface(IPInterfaceT* ifp, Uint32T msgId);
void pifPostEventIPAddr(IPInterfaceT* ifp, ConnectedAddressT* conAddr, Uint32T msgId);
void pifPostEventAgg(AggGroupT* agg, Uint32T msgId);
void pifPostEventVlan(VlanT* vlan, void* port, Uint32T msgId);
void pifPostEventPort(PhysicalPortT* port, Uint32T msgId);
void dumpIPInterfaceEvent(PifInterfaceEventMsgT* m);
void dumpIPInterfaceAddrEvent(PifInterfaceAddrEventMsgT* m);


#endif   /* _pifMsgEvent_h */

