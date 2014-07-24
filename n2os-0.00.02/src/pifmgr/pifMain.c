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

#include <stdio.h>
#include <stdlib.h>
#include "pif.h"
#include "pifMain.h"
#include "nosLib.h"
#include "pifFeaApi.h"

#include "nnCmdDefines.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCompProcess.h"


/*
 * External Definitions for Global Data Structure 
 */
extern void** gCompData;

/*
 * PIF database reference
 */
static PifDataBaseT* gPifDataGlobal = NULL;



void initPifDatabase()
{
	printf("%s\n", __FUNCTION__);

	gPifDataGlobal = (PifDataBaseT*)NNMALLOC(MEM_GLOBAL, sizeof(PifDataBaseT));

	gPifDataGlobal->PORT_DB      = NULL;
	gPifDataGlobal->AGG_DB       = NULL;
	gPifDataGlobal->INTERFACE_DB = NULL;
	gPifDataGlobal->VLAN_DB      = NULL;

	printf("%s(database = %p)\n", __FUNCTION__, (*gCompData));

	/* Initailize each database */
	gPifDataGlobal->PORT_DB      = pifPortMgrCreate();
	gPifDataGlobal->AGG_DB       = pifAggMgrCreate();
	gPifDataGlobal->INTERFACE_DB = pifInterfaceMgrCreate();
	gPifDataGlobal->VLAN_DB      = pifVlanMgrCreate();

	printf("%s end\n", __FUNCTION__);
}

void initPifDataManagers()
{
	printf("%s\n", __FUNCTION__);
	pifPortMgrInitialize();
	pifAggMgrInitialize();
	pifInterfaceMgrInitialize();
	pifVlanMgrInitialize();
}




void restartPifDatabase()
{
	pifPortMgrRestart(gPifDataGlobal->PORT_DB);
	pifAggMgrRestart(gPifDataGlobal->AGG_DB);
	pifInterfaceMgrRestart(gPifDataGlobal->INTERFACE_DB);
	pifVlanMgrRestart(gPifDataGlobal->VLAN_DB);
}

/* initialize pif manager's data structure, and build interface
 * information from h/w and os kernels */

Int32T pifWriteConfCB(struct cmsh *cmsh, 
		Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
		Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
		Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);

void pifInitProcess(void)
{
	printf("%s\n", __FUNCTION__);

	/* create pif data structure */
	initPifDatabase();

	/* Command Init. */
	gPifDataGlobal->pCmshGlobal = compCmdInit(IPC_PIF_MGR, pifWriteConfCB);

	/* Save componet database in global reference */
	(*gCompData) = (void*)gPifDataGlobal;

	/* Read current system h/w and os kernel configuration */
	feaInit();

	/* Initialize pif data managers */
	initPifDataManagers();

	printf("%s end\n", __FUNCTION__);
}

//terminate component
void pifTermProcess(void)
{
	//do something such as free database
}

//restart component
void pifRestartProcess(void)
{
	pifLOG(LOG_DEBUG, "PIF-RESTART ");

	/* initailize pif data structure */
	restartPifDatabase();
}

//hold component
void pifHoldProcess(void)
{
	pifLOG(LOG_DEBUG, "PIF-HOLD ");

	//do something such as free database
}

//signal catch proces
void pifSignalProcess(Int32T sig)
{
	pifLOG(LOG_DEBUG, "\nPIF-SIGNAL \n");

	pifLOG(LOG_DEBUG,"=================================\n");
	pifLOG(LOG_DEBUG," Catch siganl: %d\n", sig);
	pifLOG(LOG_DEBUG,"=================================\n");

	//do something you need
	
	exit(1);
}


/**
 * Description : Command manager channel
 *
 * @retval : none
 */
void pifCmIpcProcess(Int32T sockId, void *message, Uint32T size)
{
	pifLOG(LOG_DEBUG, "called pifCmIpcProcess \n");
	compCmdIpcProcess(gPifDataGlobal->pCmshGlobal, sockId, message, size);
}


/**
 * Description : Command shell channel
 *
 * @retval : none
 */
void pifCmshIpcProcess (Int32T sockId, void *message, Uint32T size)
{
	pifLOG(LOG_DEBUG, "called pifCmshIpcProcess \n");
	compCmdIpcProcess(gPifDataGlobal->pCmshGlobal, sockId, message, size);
}


/*
 * Description : write running config
 */
void interfaceConfigWrite(struct cmsh *cmsh, AvlNodeLongT* treeNode);
void vlanConfigWrite(struct cmsh *cmsh, AvlNodeLongT* treeNode);

Int32T pifWriteConfCB(struct cmsh *cmsh, 
		Int32T uargc1, Int8T **uargv1,
		Int32T uargc2, Int8T **uargv2,
		Int32T uargc3, Int8T **uargv3,
		Int32T uargc4, Int8T **uargv4, 
		Int32T uargc5, Int8T **uargv5, 
		Int32T cargc, Int8T **cargv)
{
	pifLOG(LOG_DEBUG, "PIF-CONFIG-WRITE \n");

	AvlNodeLongT *treeRoot = NULL;

	/* from config node */
	cmdPrint(cmsh,"!");

	/* write loop of interface */
	treeRoot = getRootInterfaceTable();
	interfaceConfigWrite(cmsh, treeRoot);

	cmdPrint(cmsh,"!");

	/* write loop of vlan */

	/* check any vlan exist */
	if(getSizeVlanTable() == 0){
		cmdPrint(cmsh,"!");
		return CMD_IPC_OK;
	}

	/* vlan databse node name */
	cmdPrint(cmsh, " vlan database");

	treeRoot = getRootVlanTable();
	vlanConfigWrite(cmsh, treeRoot);

	cmdPrint(cmsh,"!");
	return CMD_IPC_OK;
}


void interfaceConfigWrite(struct cmsh *cmsh, AvlNodeLongT* treeNode)
{
	if(treeNode == NULL) return;

	/* cmd string buffer */
	Int8T str[1024] = {};

	/* print each interface node */
	IPInterfaceT* ifp = (IPInterfaceT*)treeNode->pData;

	/* interface node name */
	sprintf(str," interface %s", ifp->name);
	cmdPrint(cmsh, str);

	/* shutdown/no shutdown */
	if(ifp->adminState == ADMIN_UP){
		sprintf(str,"\t no shutdown");
		cmdPrint(cmsh, str);
	}
	else if(ifp->adminState == ADMIN_DOWN){
		sprintf(str,"\t shutdown");
		cmdPrint(cmsh, str);
	}
	else {
	}

	/* switchport/no switchport */
	if(ifp->ifMode == NOT_CONFIGURED){
	}
	else if(ifp->ifMode == ROUTED_PORT){
		sprintf(str,"\t no switchport");
		cmdPrint(cmsh, str);

		/* ip address in case layer 3 interface */
		ListNodeT * listNode;
		ConnectedAddressT *pAddr;
		Int8T strAddr[100];
		PLIST_LOOP(ifp->connectedAddrs, pAddr, listNode)
		{
			//if(pAddr->address)
			{
				nnCnvPrefixtoString(strAddr, &(pAddr->address));
				sprintf(str,"\t ip address %s", strAddr);
				cmdPrint(cmsh, str);
			}
		}
	}
	else if(ifp->ifMode == SWITCH_PORT){
		sprintf(str,"\t switchport");
		cmdPrint(cmsh, str);

		/* vlan mode in case layer 2 interface */
		SwitchPortT* switchPort = getSwitchPort(ifp->attachedPort);
		if(switchPort->vlanMode == VLAN_NOT_CONFIGURED){
		}
		else if(switchPort->vlanMode == VLAN_MODE_ACCESS){
			sprintf(str,"\t switchport mode access");
			cmdPrint(cmsh, str);

			/* switchport access vid */
			sprintf(str,"\t switchport mode access vlan %d", switchPort->accessVid);
			cmdPrint(cmsh, str);
		}
		else if(switchPort->vlanMode == VLAN_MODE_TRUNK){
			sprintf(str,"\t switchport mode trunk");
			cmdPrint(cmsh, str);

			/* switchport trunk vid */
			Int32T i = 0;
			switch(switchPort->trunkVlanAllowedMode)
			{
				case PIF_VLAN_ALLOWED_NOT_CONFIG:
					break;
				case PIF_VLAN_ALLOWED_ALL:
					sprintf(str,"\t switchport trunk allowed vlan all");
					cmdPrint(cmsh, str);
					break;
				case PIF_VLAN_ALLOWED_NONE:
					sprintf(str,"\t switchport trunk allowed vlan none");
					cmdPrint(cmsh, str);
					break;
				case PIF_VLAN_ALLOWED_CONFIG:
					for(i=1; i<=PIF_VLAN_MAX; i++){
						if(switchPort->trunkAllowedVid[i] == PIF_VLAN_ALLOWED_FLAGE_ENABLE){
							sprintf(str,"\t switchport trunk allowed vlan add %d", i);
							cmdPrint(cmsh, str);
						}
					}
					break;
			}
		} /* end of trunk */
	} /* end of switchport */

	cmdPrint(cmsh,"!");
	/*****************************************
	 * end of interface config write
	 ****************************************/

	interfaceConfigWrite(cmsh, treeNode->pLeft);
	interfaceConfigWrite(cmsh, treeNode->pRight);
}


void vlanConfigWrite(struct cmsh *cmsh, AvlNodeLongT* treeNode)
{
	if(treeNode == NULL) return;

	/* cmd string buffer */
	Int8T str[1024] = {};

	/* print vlan database node */
	VlanT* vlan = (VlanT*)treeNode->pData;

	/* vlan */
	sprintf(str,"\t vlan %d name %s mtu %d", vlan->vid, vlan->name, vlan->mtu);
	cmdPrint(cmsh, str);

	/*****************************************
	 * end of vlan config write
	 ****************************************/

	vlanConfigWrite(cmsh, treeNode->pLeft);
	vlanConfigWrite(cmsh, treeNode->pRight);
}


















