#if !defined(_pifFeaApi_h)
#define _pifFeaApi_h
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

#include "nnPrefix.h"
#include "pifDataTypes.h"

/*************************************************************
 *
 * Init/Deinit FEA OS & H/W
 *
 ************************************************************/
void feaInit();


/*************************************************************
 *
 * Event from FEA OS & H/W
 *
 ************************************************************/
/* Port Event */
void feaEventPortAdd (PhysicalPortT* port);
void feaEventPortDelete (StringT ifName, Uint32T ifIndex, Uint32T iid);
void feaEventPortDown (StringT ifName, Uint32T ifIndex, Uint32T iid);
void feaEventPortUp (StringT ifName, Uint32T ifIndex, Uint32T iid);

/* IPInterface Event */
void feaEventIPInterfaceAdd (IPInterfaceT* ifIp);
void feaEventIPInterfaceDelete (StringT ifName, Uint32T ifIndex, Uint32T iid);
void feaEventIPInterfaceUp (StringT ifName, Uint32T ifIndex, Uint32T iid);
void feaEventIPInterfaceDown (StringT ifName, Uint32T ifIndex, Uint32T iid);

/* IPInterface Address Event */
void feaEventIPAddressAdd (StringT ifName, Uint32T iid, ConnectedAddressT *conAddr);
void feaEventIPAddressDelete (StringT ifName, Uint32T iid, PrefixT *addr);


/*************************************************************
 *
 * FEA control API (Interface)
 *
 ************************************************************/
/* Interface up/down */
void feaApiInterfaceUp (StringT ifName, Uint32T ifIndex, Uint32T iid, Uint32T flags);
void feaApiInterfaceDown (StringT ifName, Uint32T ifIndex, Uint32T iid, Uint32T flags);

/* Interface address */
void feaApiInterfaceAddressAdd (StringT ifName, Uint32T ifIndex, ConnectedAddressT* conAddr);
void feaApiInterfaceAddressDel (StringT ifName, Uint32T ifIndex, ConnectedAddressT* conAddr);


/* Interface port type (switchport/routedport) */
void feaApiInterfacePortType (StringT ifName, Uint32T ifIndex, Uint32T portType);


/*************************************************************
 *
 * FEA control API (Vlan)
 *
 ************************************************************/
/* L2 vlan add/delete */
void feaApiVlanAdd (Uint32T vid);
void feaApiVlanDelete (Uint32T vid);

/* L2 vlan enable/disable */
void feaApiVlanEnable (Uint32T vid);
void feaApiVlanDisable (Uint32T vid);

/* L2 vlan default pvid */
void feaApiVlanDefaultPVid (StringT ifName, Uint32T ifIndex, Uint32T pvid, Uint32T vlanMode);

/* L2 vlan port  mode (access/trunk) */
void feaApiVlanPortMode (StringT ifName, Uint32T ifIndex, Uint32T vlanMode);

/* L2 vlan add/delete to/from port */
void feaApiVlanAddToPort (StringT ifName, Uint32T ifIndex, Uint32T vid, Uint32T vlanMode);
void feaApiVlanDeleteFromPort (StringT ifName, Uint32T ifIndex, Uint32T vid);


/*************************************************************
 *
 * FEA control API (Bridge)
 *
 ************************************************************/

void feaApiBridgeAdd(StringT brName, Uint32T isVlan, Uint32T protocol);

void feaApiBridgeDelete(StringT brName);

void feaApiBridgeAddPort(StringT ifName, Uint32T ifIndex);

void feaApiBridgeDeletePort(StringT ifName, Uint32T ifIndex);

void feaApiBridgeAddInstance(Uint32T instance);

void feaApiBridgeDeleteInstance(StringT ifName, Uint32T instance);

void feaApiBridgeAddVlanToInstance(Uint32T vid, Uint32T instance);

void feaApiBridgeDeleteVlanFromInstance(Uint32T vid, Uint32T instance);

void feaApiBridgeSetPortState(StringT ifName, Uint32T ifIndex, Uint32T instance, Int32T state);

void feaApiBridgeSetLearning(Int32T flag);

/* mac address table */
void feaApiMacTableSetAgingTime(Uint32T timeVal);

void feaApiMacTableAddStatic(StringT mac, Uint32T ifIndex, Uint32T vid);
void feaApiMacTableDeleteStatic(StringT mac, Uint32T ifIndex, Uint32T vid);

void feaApiMacTableAddDynamic(StringT mac, Uint32T ifIndex, Uint32T vid);
void feaApiMacTableDeleteDynamic(StringT mac, Uint32T ifIndex, Uint32T vid);

void feaApiMacTableClearByPort(Uint32T ifIndex);
void feaApiMacTableClearByVlan(Uint32T vid);
void feaApiMacTableClearByMac(StringT mac);

/*************************************************************
 *
 * FEA control API (Link Aggregation)
 *
 ************************************************************/

/* Aggregator add/delete */
void feaApiLacpAddAggregator(StringT aggName, StringT mac, PortConfigModeT mode);
void feaApiLacpDeleteAggregator(StringT aggName);

/* Aggregator add/delete port */
void feaApiLacpAddPortToAggregator(StringT aggName, StringT ifName, Uint32T ifIndex);
void feaApiLacpDeletePortToAggregator(StringT aggName, StringT ifName, Uint32T ifIndex);



#endif   /* _pifFeaApi_h */

