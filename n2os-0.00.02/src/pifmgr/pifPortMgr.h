#if !defined(_pifPortMgr_h)
#define _pifPortMgr_h
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

/*************************************************************
 *
 *        PIF Port Table (Key: Iid)
 *
 *************************************************************/
typedef struct PortTable {
	AvlTreeLongT *tree;
} PortTableT;

PortTableT* createPortTable();
PortTableT* restartPortTable(PortTableT* table);
void 	removePortTable();
void 	clearPortTable();
Int32T 	getSizePortTable();

void 	insertPortTable(Int64T key, PhysicalPortT* pData);
void 	deletePortTable(Uint64T key);

PhysicalPortT* 	findPortTable(Uint64T key);
PhysicalPortT* 	findNamePortTable(StringT name);
PhysicalPortT*  findIfIdxPortTable(Uint32T idx);

AvlNodeLongT* getRootPortTable();

void 	printPortTable(AvlNodeLongT* treeNode);
void 	printPortAll();


#define PORT_TABLE_DBUG() printPortTable(getRootPortTable()) 


/*************************************************************
 *
 *        PIF Port Manager
 *
 *************************************************************/
typedef struct PortDatabase
{   

    PortSpeedT       defaultPortSpeed;
    PortDuplexT      defaultDuplex;
    PortFlowControlT defaultFlowControl;
    Int32T           defaultVid;
    PortConfigModeT  defaultPortMode;

    /* Table for PhysicalPortT (key = ifindex, name)*/
    PortTableT*      pifPortTable;
    
} PortDatabaseT;

/* PIF Port Manager start/restart/stop */
PortDatabaseT* pifPortMgrCreate();
PortDatabaseT* pifPortMgrRestart(PortDatabaseT* db);
void pifPortMgrInitialize();


/* Event From FEA Interface */
Int32T pifPortMgrAddPort(PhysicalPortT* obj);
Int32T pifPortMgrDelPort(StringT ifName, Uint32T ifIndex, Uint32T iid);

Int32T pifPortMgrUpPort(StringT ifName, Uint32T ifIndex, Uint32T iid);
Int32T pifPortMgrDownPort(StringT ifName, Uint32T ifIndex, Uint32T iid);


/*************************************************************
 * API of Port Manager
 ************************************************************/

Int32T pifPortMgrCheckPort(Uint32T iid);
PhysicalPortT* pifPortMgrGetPort(Uint32T iid);
IPInterfaceT* pifPortMgrGetSubInterface(Uint32T iid);

void pifPortMgrSetPortMode(PhysicalPortT* port, PortConfigModeT mode);
void pifPortMgrSetPortFwdState(PhysicalPortT* port, Uint32T instance, PortStateT state);


/* set port no shutdown/shutdown */
Int32T pifPortMgrEnablePort(PhysicalPortT* port);
Int32T pifPortMgrDisablePort(PhysicalPortT* port);

/* set port mode switchport/routedport */
Int32T pifPortMgrSetSwitchPort(PhysicalPortT* port);
Int32T pifPortMgrSetRoutedPort(PhysicalPortT* port);

Int32T pifPortMgrClearSwitchPort(PhysicalPortT* port);
Int32T pifPortMgrClearRoutedPort(PhysicalPortT* port);


/* set port speed/duplex/flow control */
Int32T pifPortMgrSetSpeed(PhysicalPortT* port, PortSpeedT speed);
Int32T pifPortMgrSetDuplex(PhysicalPortT* port, PortDuplexT duplex);
Int32T pifPortMgrSetFlow(PhysicalPortT* port, Int8T dir, PortFlowCtrlT ctrl);

/* set vlan mode(VLAN_MODE_ACCESS/VLAN_MODE_TRUNK) */
Int32T pifPortMgrSetVlanAccess(PhysicalPortT* port);
Int32T pifPortMgrSetVlanTrunk(PhysicalPortT* port);

/* set switchport access vid */
Int32T pifPortMgrAddAccessVid(PhysicalPortT* port, Uint32T vid);
Int32T pifPortMgrDeleteAccessVid(PhysicalPortT* port);

/* set switchport trunck vid */
Int32T pifPortMgrAddTrunkVidAll(PhysicalPortT* port);
Int32T pifPortMgrAddTrunkVidNone(PhysicalPortT* port);
Int32T pifPortMgrAddTrunkVid(PhysicalPortT* port, Uint32T vid);

Int32T pifPortMgrDeleteTrunkVid(PhysicalPortT* port, Uint32T vid);
Int32T pifPortMgrClearTrunk(PhysicalPortT* port);
Int32T pifPortMgrApplyTrunkVidAll(Uint32T vid);

/* set switchport clear */
Int32T pifPortMgrClearTrunk(PhysicalPortT* port);
Int32T pifPortMgrClearAccess(PhysicalPortT* port);



#endif   /* _pifPortMgr_h */

