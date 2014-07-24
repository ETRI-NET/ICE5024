#if !defined(_pifVlanMgr_h)
#define _pifVlanMgr_h
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
 *                 VLAN Table
 *
 **********************************************************/
typedef struct VlanTable {
	AvlTreeLongT *tree;
} VlanTableT;

VlanTableT* createVlanTable();
VlanTableT* restartVlanTable(VlanTableT* table);
void 	removeVlanTable();
void 	clearVlanTable();
Int32T 	getSizeVlanTable();

void 	insertVlanTable(Uint64T key, VlanT* pData);
void 	deleteVlanTable(Uint64T key);

VlanT* 	findVlanTable(Uint64T key);
VlanT* 	findNameVlanTable(StringT name);

void 	printVlanTable(AvlNodeLongT* treeNode);
AvlNodeLongT* getRootVlanTable();

/**********************************************************
 *
 *                 Static MAC Table
 *
 **********************************************************/
typedef struct MacTable {
	AvlTreeLongT *tree;
} MacTableT;

MacTableT* createMacTable();
void removeMacTable(Uint32T vid);
void clearMacTable(Uint32T vid);

Int32T getSizeMacTable(Uint32T vid);
AvlNodeLongT* getRootMacTable(Uint32T vid);

void insertMacTable(Uint32T vid, Uint64T key, StaticMacT* pData);
void deleteMacTable(Uint32T vid, Uint64T key);

StaticMacT* findMacTable(Uint32T vid, Uint64T key);

Int64T getMacKey(StringT mac);

Int32T getCurrentAgingTime();
Int32T getDefaultAgingTime();

/**********************************************************
 *
 *                 VLAN Manager
 *
 **********************************************************/

typedef struct VlanDatabase
{   
	Uint32T defaultVid;
	Uint32T defaultMetric;
	Uint32T defaultAging;
	Uint32T currentAging;
	Uint32T defaultBridge;
	Int8T   defaultBridgeName[IF_NAME_SIZE];

    /* Table for VlanT (key = vid)*/
    VlanTableT*      pifVlanTable;
    
    /* Table for StaticMacT of each Vid */
    MacTableT*       pifMacTable[PIF_VLAN_MAX+1];;

} VlanDatabaseT;

/*************************************************************
 * VlanMgr start/restart/stop
 ************************************************************/
VlanDatabaseT* pifVlanMgrCreate();
VlanDatabaseT* pifVlanMgrRestart(VlanDatabaseT* db);
void pifVlanMgrInitialize();

/*************************************************************
 * VlanMgr add/delete vlan
 ************************************************************/
Int32T pifVlanMgrAddVlan(Uint32T vid, StringT name, Uint32T mtu);
Int32T pifVlanMgrDeleteVlan(Uint32T vid);
Int32T pifVlanMgrAddUpdate(Uint32T vid, StringT name, Uint32T mtu);

/*************************************************************
 * API of VLAN Manager
 ************************************************************/
Uint32T pifVlanMgrGetDefaultBridge();
StringT pifVlanMgrGetDefaultBridgeName();
Uint32T pifVlanMgrGetDefaultVid();
Int32T pifVlanMgrCheckValid(Uint32T vid);

VlanT* pifVlanMgrGetVlan(Uint32T vid);

/* set port mode (access/trunk) */
Int32T pifVlanMgrSetPortMode(void* port, VlanPortModeT vlanMode);

/* add/delete vlan access port member  */
Int32T pifVlanMgrAddAccessPortMember(void* port, Uint32T vid);
Int32T pifVlanMgrDeleteAccessPortMember(void* port, Uint32T vid);

/* add/delete vlan trunk port member  */
Int32T pifVlanMgrAddTrunkPortMember(void* port, Uint32T vid);
Int32T pifVlanMgrDeleteTrunkPortMember(void* port, Uint32T vid);
Int32T pifVlanMgrAddTrunkPortMemberAll(void* port);
Int32T pifVlanMgrDeleteTrunkPortMemberAll(void* port);


/*************************************************************
 * VlanMgr add/delete static mac address
 ************************************************************/
Int32T pifMacAddressTableSetAgingTime(Uint32T agingTime);
Int32T pifMacAddressTableGetAgingTime();
Int32T pifMacAddressTableDelAgingTime();

Int32T pifMacAddressTableAdd(StringT mac, Uint32T type, Uint32T vid, PhysicalPortT *port);
Int32T pifMacAddressTableDelete(StringT mac, Uint32T type, Uint32T vid, PhysicalPortT *port);
Int32T pifMacAddressTableClear();





#endif   /* _pifVlanMgr_h */

