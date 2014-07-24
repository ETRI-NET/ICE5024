#if !defined(_pifInterfaceMgr_h)
#define _pifInterfaceMgr_h
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

#include "nnTypes.h"
#include "nnAvl.h"
#include "nnAvlLong.h"
#include "pifDataTypes.h"
#include "pifMsgEvent.h"

/**********************************************************
 *
 *                 IPInterface Table
 *
 **********************************************************/
typedef struct InterfaceTable {
	AvlTreeLongT *tree;
} InterfaceTableT;

InterfaceTableT* createInterfaceTable();
InterfaceTableT* restartInterfaceTable(InterfaceTableT* table);
void removeInterfaceTable();
void clearInterfaceTable();
Int32T getSizeInterfaceTable();

void insertInterfaceTable(Uint64T key, IPInterfaceT* pData);
void deleteInterfaceTable(Uint64T key);

IPInterfaceT* findInterfaceTable(Uint64T key);
IPInterfaceT* findNameInterfaceTable(StringT name);
IPInterfaceT* findIfIdxInterfaceTable(Uint32T idx);

AvlNodeLongT* getRootInterfaceTable();

void printInterfaceTable(AvlNodeLongT* treeNode);
void printInterfaceAll();


/**********************************************************
 *
 *                 IPInterface Manager
 *
 **********************************************************/

typedef struct InterfaceDatabase
{   
	Uint32T defaultMtu;
	Uint32T defaultMetric;
	Uint32T defaultVid;

    /* Table for IPInterfaceT (key = ifindex, name)*/
    InterfaceTableT*      pifInterfaceTable;
    
} InterfaceDatabaseT;

/*************************************************************
 * InterfaceMgr start/restart/stop
 ************************************************************/

InterfaceDatabaseT* pifInterfaceMgrCreate();
InterfaceDatabaseT* pifInterfaceMgrRestart(InterfaceDatabaseT* db);
void pifInterfaceMgrInitialize();

/*************************************************************
 * Event From FEA Interface
 ************************************************************/

Int32T pifInterfaceMgrAddInterface(IPInterfaceT* obj);
Int32T pifInterfaceMgrDelInterface(StringT ifName, Uint32T iid);

Int32T pifInterfaceMgrAddAddress(StringT ifName, Uint32T iid, ConnectedAddressT *conAddr);
Int32T pifInterfaceMgrDelAddress(StringT ifName, Uint32T iid, PrefixT *addr);

Int32T pifInterfaceMgrUpInterface(StringT ifName, Uint32T ifIndex, Uint32T iid);
Int32T pifInterfaceMgrDownInterface(StringT ifName, Uint32T ifIndex, Uint32T iid);


/*************************************************************
 * API of IPInterface Manager
 ************************************************************/

/* get interface and it's attributes */
IPInterfaceT* pifInterfaceMgrGetInterface(Uint32T iid);
ConnectedAddressT* pifInterfaceMgrGetConnectedAddr(IPInterfaceT* ifp, PrefixT* addr);

/* set interface and it's attributes */
void pifInterfaceMgrSetPortMode(IPInterfaceT* ifp, PortConfigModeT mode);


/* config interface address */
Int32T pifInterfaceMgrConfigAddAddress(IPInterfaceT* ifp, PrefixT* addr);
Int32T pifInterfaceMgrConfigDelAddress(IPInterfaceT* ifp, PrefixT* addr);
Int32T pifInterfaceMgrConfigClearAddress(IPInterfaceT* ifp);

/* set port no shutdown/shutdown */
Int32T pifInterfaceMgrEnableIf(IPInterfaceT* ifp);
Int32T pifInterfaceMgrDisableIf(IPInterfaceT* ifp);

/* set port as switchport or routedport */
Int32T pifInterfaceMgrSetRoutedPort(IPInterfaceT* ifp);
Int32T pifInterfaceMgrClearRoutedPort(IPInterfaceT* ifp);

Int32T pifInterfaceMgrSetSwitchPort(IPInterfaceT* ifp);
Int32T pifInterfaceMgrClearSwitchPort(IPInterfaceT* ifp);




#endif   /* _pifInterfaceMgr_h */

