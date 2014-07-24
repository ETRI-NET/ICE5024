#if !defined(_pifAggMgr_h)
#define _pifAggMgr_h
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


/***********************************************************
 *
 *       PIF Agg Table
 *
 **********************************************************/

typedef struct AggTable {
	AvlTreeLongT *tree;
} AggTableT;

AggTableT* createAggTable();
AggTableT* restartAggTable(AggTableT* table);
void 	removeAggTable();
void 	clearAggTable();
Int32T 	getSizeAggTable();

void 	insertAggTable(Int64T key, AggGroupT* pData);
void 	deleteAggTable(Uint64T key);

AggGroupT* 	findAggTable(Uint64T key);
AggGroupT* 	findNameAggTable(StringT name);

void 	printAggTable(AvlNodeLongT* treeNode);
AvlNodeLongT* getRootAggTable();
#define AGG_TABLE_DBUG() printAggTable(getRootAggTable()) 


/***********************************************************
 *
 *       PIF Agg Manager
 *
 **********************************************************/

typedef struct AggDatabase
{
    Int32T           defaultVid;

    /* Table for AggGroupT */
    AggTableT*      pifAggTable;

} AggDatabaseT;


/*************************************************************
 * PIF Agg Manager start/restart/stop 
 ************************************************************/
AggDatabaseT* pifAggMgrCreate();
AggDatabaseT* pifAggMgrRestart(AggDatabaseT* db);
void pifAggMgrInitialize();

/*************************************************************
 * PIF Agg Manager External API
 ************************************************************/
AggGroupT* pifAggMgrGetGroup(Uint32T aggId);
Int32T pifAggMgrGetPortCount(AggGroupT* agg);
void pifAggMgrSetAggName(StringT name, Int32T aggId);
void pifAggMgrSetMacAddr(StringT mac, Int32T aggId);

/*************************************************************
 * PIF Agg Manager add/delete group
 ************************************************************/
Int32T pifAggMgrAddGroup(StringT name, Uint32T id, Int8T* mac, Int16T mode);
Int32T pifAggMgrDeleteGroup(StringT name);

Int32T pifAggMgrAddStaticGroup(Uint32T aggId, Int8T* mac);
Int32T pifAggMgrDeleteStaticGroup(Uint32T aggId);

/*************************************************************
 * PIF Agg Manager add/delete port to/from group
 ************************************************************/
Int32T pifAggMgrAddPortMember(StringT name, Uint32T id, Uint32T portIfIndex);
Int32T pifAggMgrDeletePortMember(StringT name, Uint32T id, Uint32T portIfIndex);












#endif   /* _pifAggMgr_h */

