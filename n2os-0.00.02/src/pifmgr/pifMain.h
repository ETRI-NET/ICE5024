#if !defined(_pifMain_h)
#define _pifMain_h
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
#include "nosLib.h"
#include "pifPortMgr.h"
#include "pifAggMgr.h"
#include "pifInterfaceMgr.h"
#include "pifVlanMgr.h"

/*
 * PIF database definition
 */
typedef struct PifDataBase
{
	/* Port Database */
	PortDatabaseT*       	PORT_DB;

	/* Aggregator Database */
	AggDatabaseT*	     	AGG_DB;

	/* Interface Database */
	InterfaceDatabaseT*     INTERFACE_DB;

	/* VLAN Database */
	VlanDatabaseT*       	VLAN_DB;
	
	/* Command manager interface */
	void * pCmshGlobal;
	
} PifDataBaseT;


void initPifDatabase();
void restartPifDatabase();
void pifInitProcess(void);
void pifTermProcess(void);
void pifRestartProcess(void);
void pifHoldProcess(void);
void pifSingalProcess(Int32T sig);

void pifCmIpcProcess(Int32T sockId, void *message, Uint32T size);
void pifCmshIpcProcess (Int32T sockId, void *message, Uint32T size);



#endif   /* _pifMain_h */

