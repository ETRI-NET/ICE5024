/**************************************************************************************************** 
*                      Electronics and Telecommunications Research Institute
* Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.
*           No part of this software shall be reproduced, stored in a retrieval system, or
*           transmitted by any means, electronic, mechanical, photocopying, recording,
*           or otherwise, without written permission from ETRI.
****************************************************************************************************/

/**
 * @brief       : CLI Command of pifmgr 
 * - Block Name : Command Manager
 * - Process Name : cm
 * - Creator : Seungwoo Hong
 * - Initial Date : 2014/05/20
 */

/**
 * @file        : 
 *
 * $Id: 
 * $Author: 
 * $Date: 
 * $Revision: 
 * $LastChangedBy: 
 */

#include <stdlib.h>
#include "nnTypes.h"
#include "nnStr.h"

#include "nnCmdLink.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"
#include "nnCmdInstall.h"
#include "nosLib.h"
#include "nnCmdDefines.h"
//#include "nnUtility.h"

#include "nnBuffer.h"
#include "nnPrefix.h"

#include "pif.h"
#include "pifDataTypes.h"
#include "pifCmdApi.h"

#include "pifInterfaceMgr.h"
#include "pifPortMgr.h"
#include "pifAggMgr.h"
#include "pifVlanMgr.h"

#include "hal_incl.h"

/** @name Command Type
 */
/**@{*/
#define         CMD_DELETE              0                 
#define         CMD_ADD                 1
/**@}*/

static Int32T cmdDebugFlag = 1;

#define uargc   uargc1
#define uargv   uargv1
#define CMDDEBUG() \
	if(cmdDebugFlag){ \
		Int32T i; \
		pifLOG(LOG_DEBUG, "Enter \n"); \
		cmdPrint(cmsh,"uargc=%d", cargc); \
		for(i = 0; i < uargc; i++) \
		{ \
			cmdPrint(cmsh," %s", uargv[i]);\
		} \
		cmdPrint(cmsh," \n"); \
		cmdPrint(cmsh,"cargc=%d", cargc); \
		for(i = 0; i < cargc; i++) \
		{ \
			cmdPrint(cmsh," %s", cargv[i]);\
		} \
		cmdPrint(cmsh," \n"); \
	} 


/**
 * Description : Interface no shutdown command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncIfAdminSet,
	CMD_NODE_INTERFACE,
    IPC_PIF_MGR,
	"no shutdown",
	"Negate a command", 
	"Shutdown the interface")
{
	CMDDEBUG();

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, uargv[1]);

	/* check interface name */
	Int32T pifRlt = pifCmiIfSet(ifName);
	if(pifRlt != PIF_CMI_RETURN_OK){
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}

	/* request to process configuation */
	if(pifCmiIfAdminSet() != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}


/**
 * Description : Interface shutdown command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncIfAdminUnset,
	CMD_NODE_INTERFACE,
    IPC_PIF_MGR,
	"shutdown",
	"Shutdown the interface")
{
	CMDDEBUG();

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, uargv[1]);

	/* check interface name */
	Int32T pifRlt = pifCmiIfSet(ifName);
	if(pifRlt != PIF_CMI_RETURN_OK){
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}

	/* request to process configuation */
	if(pifCmiIfAdminUnset() != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}





/**
 * Description : Interface speed command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncIfSpeedSet,
	CMD_NODE_INTERFACE,
    IPC_PIF_MGR,
	"speed (10|100|1000|auto)",
	"Interface speed", 
	"10 Mbps", 
	"100 Mbps", 
	"1000 Mbps", 
	"Auto speed")
{
	CMDDEBUG();

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, uargv[1]);

	/* check interface name */
	Int32T pifRlt = pifCmiIfSet(ifName);
	if(pifRlt != PIF_CMI_RETURN_OK){
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}

	/* get speed value */
	PortSpeedT speed;
	if (strcmp("10", cargv[1]) == 0){
		speed = PORT_SPEED_10;
	}
	else if(strcmp("100", cargv[1]) == 0){
		speed = PORT_SPEED_100;
	}
	else if(strcmp("1000", cargv[1]) == 0){
		speed = PORT_SPEED_1000;
	}
	else if(strcmp("auto", cargv[1]) == 0){
		speed = PORT_SPEED_AUTO;
	}
	else {
		speed = PORT_SPEED_AUTO;
	}

	/* request to process configuation */
	if(pifCmiIfSpeedSet(speed) != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}


/**
 * Description : Interface duplex command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncIfDuplexSet,
	CMD_NODE_INTERFACE,
    IPC_PIF_MGR,
	"duplex (full|half|auto)",
	"Interface duplex", 
	"Set full-duplex", 
	"Set half-duplex", 
	"Set auto-negotiate")
{
	CMDDEBUG();

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, uargv[1]);

	/* check interface name */
	Int32T pifRlt = pifCmiIfSet(ifName);
	if(pifRlt != PIF_CMI_RETURN_OK){
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}

	/* get duplex value */
	PortDuplexT duplex;
	if (strcmp("full", cargv[1]) == 0){
		duplex = PORT_DUPLEX_FULL;
	}
	else if(strcmp("half", cargv[1]) == 0){
		duplex = PORT_DUPLEX_HALF;
	}
	else if(strcmp("auto", cargv[1]) == 0){
		duplex = PORT_DUPLEX_AUTO;
	}
	else {
		duplex = PORT_DUPLEX_AUTO;
	}

	/* request to process configuation */
	if(pifCmiIfDuplexSet(duplex) != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}


/**
 * Description : Interface IP address command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncIfAddressSet,
	CMD_NODE_INTERFACE,
    IPC_PIF_MGR,
	"ip address A.B.C.D/M",
	"Interface IP command", 
	"IP address", 
	"IP address format (e.g. 10.1.1.1/24)")
{
	CMDDEBUG();

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, uargv[1]);

	/* check interface name */
	Int32T pifRlt = pifCmiIfSet(ifName);
	if(pifRlt != PIF_CMI_RETURN_OK){
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}

	/* request to process configuation */
	if(pifCmiIfAddressSet((StringT)cargv[2]) != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}


/**
 * Description : Interface IP address command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncIfAddressUnset,
	CMD_NODE_INTERFACE,
    IPC_PIF_MGR,
	"no ip address A.B.C.D/M",
	"Negate a command", 
	"Interface IP command", 
	"IP address", 
	"IP address format (e.g. 10.1.1.1/24)")
{
	CMDDEBUG();

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, uargv[1]);

	/* check interface name */
	Int32T pifRlt = pifCmiIfSet(ifName);
	if(pifRlt != PIF_CMI_RETURN_OK){
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}

	/* request to process configuation */
	if(pifCmiIfAddressUnset((StringT)cargv[3]) != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}


/**
 * Description : Interface no switchport command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncIfSwportUnset,
	CMD_NODE_INTERFACE,
    IPC_PIF_MGR,
	"no switchport",
	"Negate a command", 
	"switchport the interface")
{
	CMDDEBUG();

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, uargv[1]);

	/* check interface name */
	Int32T pifRlt = pifCmiIfSet(ifName);
	if(pifRlt != PIF_CMI_RETURN_OK){
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}

	/* request to process configuation */
	if(pifCmiIfSwportUnset() != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}


/**
 * Description : Interface switchport command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncIfSwportSet,
	CMD_NODE_INTERFACE,
    IPC_PIF_MGR,
	"switchport",
	"switchport the interface")
{
	CMDDEBUG();

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, uargv[1]);

	/* check interface name */
	Int32T pifRlt = pifCmiIfSet(ifName);
	if(pifRlt != PIF_CMI_RETURN_OK){
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}

	/* request to process configuation */
	if(pifCmiIfSwportset() != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}

/**
 * Description : Vlan access/trunk mode command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncVlanPortModeSet,
	CMD_NODE_INTERFACE,
    IPC_PIF_MGR,
	"switchport mode (access|trunk)",
	"Vlan port mode of L2 interface",
	"Mode of vlan", 
	"Vlan access port mode", 
	"Vlan trunk port mode")
{
	CMDDEBUG();

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, uargv[1]);

	/* check interface name */
	Int32T pifRlt = pifCmiIfSet(ifName);
	if(pifRlt != PIF_CMI_RETURN_OK){
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}

	/* get vlanMode */
	VlanPortModeT mode = VLAN_NOT_CONFIGURED;
	if (strcmp("access", cargv[2]) == 0){
		mode = VLAN_MODE_ACCESS;
	}
	else if(strcmp("trunk", cargv[2]) == 0){
		mode = VLAN_MODE_TRUNK;
	}

	/* request to process configuation */
	if(pifCmiVlanPortModeSet(mode) != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}



/**
 * Description : Vlan access default vid command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncVlanAccessVidDefault,
	CMD_NODE_INTERFACE,
    IPC_PIF_MGR,
	"switchport access",
	"Vlan port mode of L2 interface",
	"Vlan access port vid")
{
	CMDDEBUG();

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, uargv[1]);

	/* check interface name */
	Int32T pifRlt = pifCmiIfSet(ifName);
	if(pifRlt != PIF_CMI_RETURN_OK){
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}

	/* set vid as NOS_DEFAULT_VID */
	Uint32T vid = NOS_DEFAULT_VID;

	/* request to process configuation */
	if(pifCmiVlanAccessVidAdd(vid) != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}


/**
 * Description : Vlan access user vid command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncVlanAccessVidAdd,
	CMD_NODE_INTERFACE,
    IPC_PIF_MGR,
	"switchport access vlan <1-4095>",
	"Vlan port mode of L2 interface",
	"Vlan access port vid",
	"Vlan ID config",
	"VID <1-4095>")
{
	CMDDEBUG();

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, uargv[1]);

	/* check interface name */
	Int32T pifRlt = pifCmiIfSet(ifName);
	if(pifRlt != PIF_CMI_RETURN_OK){
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}

	/* get vid*/
	Uint32T vid = atoi(cargv[3]);

	/* request to process configuation */
	if(pifCmiVlanAccessVidAdd(vid) != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}


/**
 * Description : Vlan clear access vid command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncVlanAccessVidDel,
	CMD_NODE_INTERFACE,
    IPC_PIF_MGR,
	"no switchport access vlan",
	"Negate a command", 
	"Vlan port mode of L2 interface",
	"Vlan access port vid",
	"Vlan ID config")
{
	CMDDEBUG();

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, uargv[1]);

	/* check interface name */
	Int32T pifRlt = pifCmiIfSet(ifName);
	if(pifRlt != PIF_CMI_RETURN_OK){
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}

	/* request to process configuation */
	if(pifCmiVlanAccessVidDel() != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}


/**
 * Description : Vlan Trunk allowed vid(all/none) command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncVlanTrunkVidAllowed,
	CMD_NODE_INTERFACE,
    IPC_PIF_MGR,
	"switchport trunk allowed vlan (all|none)",
	"Vlan port mode of L2 interface",
	"Vlan trunk port",
	"Set allowed vlan id",
	"Vlan configured",
	"Allow all vlans on the switchport",
	"Allow no vlan on the swithport")
{
	CMDDEBUG();

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, uargv[1]);

	/* check interface name */
	Int32T pifRlt = pifCmiIfSet(ifName);
	if(pifRlt != PIF_CMI_RETURN_OK){
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}


	/* get allowed vids */
	Uint32T vid = NOS_NONE_VID;
	if (strcmp("all", cargv[4]) == 0){
		vid = NOS_ALL_VID;
	}
	else if(strcmp("none", cargv[4]) == 0){
		vid = NOS_NONE_VID;
	}

	/* request to process configuation */
	if(pifCmiVlanTrunkVidAdd(vid) != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}



/**
 * Description : Vlan Trunk allowed vid add command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncVlanTrunkVidAdd,
	CMD_NODE_INTERFACE,
    IPC_PIF_MGR,
	"switchport trunk allowed vlan add <1-4095>",
	"Vlan port mode of L2 interface",
	"Vlan trunk port",
	"Allowed vlan list",
	"Set allowed vlan id",
	"Add vlan id",
	"vid 1-4095")
{
	CMDDEBUG();

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, uargv[1]);

	/* check interface name */
	Int32T pifRlt = pifCmiIfSet(ifName);
	if(pifRlt != PIF_CMI_RETURN_OK){
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}

	/* get allowed vid */
	Uint32T vid = atoi(cargv[5]);;

	/* request to process configuation */
	if(pifCmiVlanTrunkVidAdd(vid) != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}


/**
 * Description : Vlan Trunk allowed vid remove command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncVlanTrunkVidDel,
	CMD_NODE_INTERFACE,
    IPC_PIF_MGR,
	"switchport trunk allowed vlan remove <1-4095>",
	"Vlan port mode of L2 interface",
	"Vlan trunk port",
	"Allowed vlan list",
	"Set allowed vlan id",
	"Remove vlan id",
	"vid 1-4095")
{
	CMDDEBUG();

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, uargv[1]);

	/* check interface name */
	Int32T pifRlt = pifCmiIfSet(ifName);
	if(pifRlt != PIF_CMI_RETURN_OK){
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}

	/* get allowed vid */
	Uint32T vid = atoi(cargv[5]);;

	/* request to process configuation */
	if(pifCmiVlanTrunkVidDel(vid) != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}


/**
 * Description : Vlan Trunk allowed vid except command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncVlanTrunkVidExcept,
	CMD_NODE_INTERFACE,
    IPC_PIF_MGR,
	"switchport trunk allowed vlan except <1-4095>",
	"Vlan port mode of L2 interface",
	"Vlan trunk port",
	"Allowed vlan list",
	"Set allowed vlan id",
	"Except vlan id",
	"vid 1-4095")
{
	CMDDEBUG();

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, uargv[1]);

	/* check interface name */
	Int32T pifRlt = pifCmiIfSet(ifName);
	if(pifRlt != PIF_CMI_RETURN_OK){
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}

	/* get allowed vid */
	Uint32T vid = atoi(cargv[5]);;

	/* request to process configuation */
	if(pifCmiVlanTrunkVidExept(vid) != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}


/**
 * Description : Vlan Trunk Port  clear command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncVlanTrunkVidClear,
	CMD_NODE_INTERFACE,
    IPC_PIF_MGR,
	"no switchport trunk",
	"Negate a command", 
	"Vlan port mode of L2 interface",
	"Vlan trunk port")
{
	CMDDEBUG();

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, uargv[1]);

	/* check interface name */
	Int32T pifRlt = pifCmiIfSet(ifName);
	if(pifRlt != PIF_CMI_RETURN_OK){
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}

	/* request to process configuation */
	if(pifCmiVlanTrunkClear() != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}


/**
 * Description : Vlan Access clear command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncVlanAccessClear,
	CMD_NODE_INTERFACE,
    IPC_PIF_MGR,
	"no switchport access",
	"Negate a command", 
	"Vlan port mode of L2 interface",
	"Vlan trunk port")
{
	CMDDEBUG();

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, uargv[1]);

	/* check interface name */
	Int32T pifRlt = pifCmiIfSet(ifName);
	if(pifRlt != PIF_CMI_RETURN_OK){
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}

	/* request to process configuation */
	if(pifCmiVlanAccessClear() != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}







/**
 * Description : Vlan vid add command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncVlanAdd,
	CMD_NODE_VLAN,
    IPC_PIF_MGR,
	"vlan <1-4095> ",
	"vlan config command", 
	"vid 1-4095")
{
	CMDDEBUG();

	Uint32T vid = 0;
	StringT vlanName = NULL;
	Uint32T mtu = 0;

	/* set vid */
	vid = atoi(cargv[1]);

	/* set name */
	if(cargc > 2){
		vlanName = (StringT)cargv[3];
	}

	/* set mtu */
	if(cargc > 4){
		mtu = atoi(cargv[5]);
	}

	/* request to process configuation */
	if(pifCmiVlanDatabaseAdd(vid, vlanName, mtu) != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}
ALICMD(cmdFuncVlanAdd,
	CMD_NODE_VLAN,
    IPC_PIF_MGR,
	"vlan <1-4095> name WORD ",
	"vlan config command", 
	"vid 1-4095",
	"vlan name keyword",
	"vlan name string");
ALICMD(cmdFuncVlanAdd,
	CMD_NODE_VLAN,
    IPC_PIF_MGR,
	"vlan <1-4095> name WORD mtu <1-2000>  ",
	"vlan config command", 
	"vid 1-4095",
	"vlan name keyword",
	"vlan name string",
	"mtu size keyword", 
	"mtu size number");


/**
 * Description : Vlan vid delete command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncVlanDel,
	CMD_NODE_VLAN,
    IPC_PIF_MGR,
	"no vlan <1-4095> ",
	"Negate a command", 
	"vlan config command", 
	"vid 1-4095")
{
	CMDDEBUG();

	Uint32T vid = 0;

	/* set vid */
	vid = atoi(cargv[2]);

	/* request to process configuation */
	if(pifCmiVlanDatabaseDel(vid) != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}






/**
 * Description : mac-address-table aging-time set
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncMacTableAgingeTime,
	CMD_NODE_CONFIG,
    IPC_PIF_MGR,
	"mac-address-table aging-time <1-10000> ",
	"MAC table command", 
	"aging-time paramter", 
	"time value <1-10000> ")
{
	CMDDEBUG();

	Uint32T agingTime = 0;

	/* set time */
	agingTime = atoi(cargv[2]);

	/* request to process configuation */
	if(pifMacAddressTableSetAgingTime(agingTime) != PIF_RTN_OK) {
		cmdPrint(cmsh,"fail to set aging-time\n");
		return CMD_IPC_OK;
	}

	return CMD_IPC_OK;
}



/**
 * Description : mac-address-table aging-time del
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncMacTableNoAgingeTime,
	CMD_NODE_CONFIG,
    IPC_PIF_MGR,
	"no mac-address-table aging-time ",
	"Negate a command", 
	"MAC table command", 
	"aging-time paramter" )
{
	CMDDEBUG();

	/* request to process configuation */
	if(pifMacAddressTableDelAgingTime() != PIF_RTN_OK) {
		cmdPrint(cmsh,"fail to set aging-time\n");
		return CMD_IPC_OK;
	}

	return CMD_IPC_OK;
}







/**
 * Description : mac-address-table command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncMacTableStaticAdd,
	CMD_NODE_CONFIG,
    IPC_PIF_MGR,
	"mac-address-table static WORD interface WORD",
	"MAC table command", 
	"static mac address", 
	"mac address eg. 00:30:48:b0:fc:cf", 
	"interface parameter", 
	"interface name eg. eth0")
{
	CMDDEBUG();

	/* set default vid */
	Uint32T vid = 1;

	/* set mac address */
	Int8T macAddr[ETH_ADDR_LEN];
	memset(&macAddr[0], 0, ETH_ADDR_LEN);

	/* check mac address validation */
	//checkMacAddress(cargv[2]);
	
	/* copy mac address */
	memcpy(&macAddr[0], str2mac(cargv[2]), ETH_ADDR_LEN); 

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, cargv[4]);

	/* verify interface name */
	Uint32T iid = getIidbyName(ifName);
	if(iid < 0) {
		cmdPrint(cmsh,"invalid interface name %s \n", ifName);
		return CMD_IPC_OK;
	}

	/* get port instance */
	PhysicalPortT *port = pifPortMgrGetPort(iid);
	if(port == NULL) {
		cmdPrint(cmsh,"interface %s not exist\n", ifName);
		return CMD_IPC_OK;
	}

	/* request to process configuation */
	Int32T rlt = pifMacAddressTableAdd(macAddr, PIF_MAC_TYPE_CLI, vid, port);
	if(rlt != PIF_RTN_OK){
		cmdPrint(cmsh,"fail to set static mac %s \n", mac2str(macAddr));
		return CMD_IPC_OK;
	}

	return CMD_IPC_OK;
}


/**
 * Description : no mac-address-table command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncMacTableStaticDel,
	CMD_NODE_CONFIG,
    IPC_PIF_MGR,
	"no mac-address-table static WORD interface WORD",
	"Negate a command", 
	"MAC table command", 
	"static mac address", 
	"mac address eg. 00:30:48:b0:fc:cf", 
	"interface parameter", 
	"interface name eg. eth0")
{
	CMDDEBUG();

	/* set default vid */
	Uint32T vid = 1;

	/* set mac address */
	Int8T macAddr[ETH_ADDR_LEN];
	memset(&macAddr[0], 0, ETH_ADDR_LEN);

	/* check mac address validation */
	//checkMacAddress(cargv[2]);
	
	/* copy mac address */
	memcpy(&macAddr[0], str2mac(cargv[3]), ETH_ADDR_LEN); 

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, cargv[5]);

	/* verify interface name */
	Uint32T iid = getIidbyName(ifName);
	if(iid < 0) {
		cmdPrint(cmsh,"invalid interface name %s \n", ifName);
		return CMD_IPC_OK;
	}

	/* get port instance */
	PhysicalPortT *port = pifPortMgrGetPort(iid);
	if(port == NULL) {
		cmdPrint(cmsh,"interface %s not exist\n", ifName);
		return CMD_IPC_OK;
	}

	/* request to process configuation */
	Int32T rlt = pifMacAddressTableDelete(macAddr, PIF_MAC_TYPE_CLI, vid, port);
	if(rlt != PIF_RTN_OK){
		cmdPrint(cmsh,"fail to set static mac %s \n", mac2str(macAddr));
		return CMD_IPC_OK;
	}

	return CMD_IPC_OK;
}





/**
 * Description : mac-address-table with vid command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncMacTableStaticAddWithVid,
	CMD_NODE_CONFIG,
    IPC_PIF_MGR,
	"mac-address-table static WORD interface WORD vlan <1-4095> ",
	"MAC table command", 
	"static mac address", 
	"mac address eg. 00:30:48:b0:fc:cf", 
	"interface parameter", 
	"interface name eg. eth0", 
	"mac target vlan ", 
	"vid 1-4095")
{
	CMDDEBUG();

	/* set vid */
	Uint32T vid =  atoi(cargv[6]);

	/* set mac address */
	Int8T macAddr[ETH_ADDR_LEN];
	memset(&macAddr[0], 0, ETH_ADDR_LEN);

	/* check mac address validation */
	//checkMacAddress(cargv[2]);
	
	/* copy mac address */
	memcpy(&macAddr[0], str2mac(cargv[2]), ETH_ADDR_LEN); 

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, cargv[4]);

	/* verify interface name */
	Uint32T iid = getIidbyName(ifName);
	if(iid < 0) {
		cmdPrint(cmsh,"invalid interface name %s \n", ifName);
		return CMD_IPC_OK;
	}

	/* get port instance */
	PhysicalPortT *port = pifPortMgrGetPort(iid);
	if(port == NULL) {
		cmdPrint(cmsh,"interface %s not exist\n", ifName);
		return CMD_IPC_OK;
	}

	/* request to process configuation */
	Int32T rlt = pifMacAddressTableAdd(macAddr, PIF_MAC_TYPE_CLI, vid, port);
	if(rlt != PIF_RTN_OK){
		cmdPrint(cmsh,"fail to set static mac %s \n", mac2str(macAddr));
		return CMD_IPC_OK;
	}

	return CMD_IPC_OK;
}



/**
 * Description : no mac-address-table with vid command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncMacTableStaticDelWithVid,
	CMD_NODE_CONFIG,
    IPC_PIF_MGR,
	"no mac-address-table static WORD interface WORD vlan <1-4095> ",
	"Negate a command", 
	"MAC table command", 
	"static mac address", 
	"mac address eg. 00:30:48:b0:fc:cf", 
	"interface parameter", 
	"interface name eg. eth0", 
	"mac target vlan ", 
	"vid 1-4095")
{
	CMDDEBUG();

	/* set vid */
	Uint32T vid = atoi(cargv[7]);

	/* set mac address */
	Int8T macAddr[ETH_ADDR_LEN];
	memset(&macAddr[0], 0, ETH_ADDR_LEN);

	/* check mac address validation */
	//checkMacAddress(cargv[2]);
	
	/* copy mac address */
	memcpy(&macAddr[0], str2mac(cargv[3]), ETH_ADDR_LEN); 

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, cargv[5]);

	/* verify interface name */
	Uint32T iid = getIidbyName(ifName);
	if(iid < 0) {
		cmdPrint(cmsh,"invalid interface name %s \n", ifName);
		return CMD_IPC_OK;
	}

	/* get port instance */
	PhysicalPortT *port = pifPortMgrGetPort(iid);
	if(port == NULL) {
		cmdPrint(cmsh,"interface %s not exist\n", ifName);
		return CMD_IPC_OK;
	}

	/* request to process configuation */
	Int32T rlt = pifMacAddressTableDelete(macAddr, PIF_MAC_TYPE_CLI, vid, port);
	if(rlt != PIF_RTN_OK){
		cmdPrint(cmsh,"fail to set static mac %s \n", mac2str(macAddr));
		return CMD_IPC_OK;
	}

	return CMD_IPC_OK;
}


/**
 * Description : mac-address-table clear dynamic
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
extern void feaApiMacTableClearByPort(Uint32T ifIndex);
DECMD(cmdFuncMacTableClearDynamic,
	CMD_NODE_EXEC,
    IPC_PIF_MGR,
	"clear mac-address-table dynamic interface WORD",
	"clear command", 
	"MAC table parameter", 
	"dynamic mac address only", 
	"interface parameter", 
	"interface name string eg.eth0 ")
{
	CMDDEBUG();

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, cargv[4]);

	/* verify interface name */
	Uint32T iid = getIidbyName(ifName);
	if(iid < 0) {
		cmdPrint(cmsh,"invalid interface name %s \n", ifName);
		return CMD_IPC_OK;
	}

	/* get port instance */
	PhysicalPortT *port = pifPortMgrGetPort(iid);
	if(port == NULL) {
		cmdPrint(cmsh,"interface %s not exist\n", ifName);
		return CMD_IPC_OK;
	}

	/* direct call to fea interface */
	feaApiMacTableClearByPort(port->ifIndex);

	return CMD_IPC_OK;
}


/**
 * Description : mac-address-table show dynamic
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncMacTableShowDynamic,
	CMD_NODE_EXEC,
    IPC_PIF_MGR,
	"show mac-address-table dynamic",
	"show command", 
	"MAC table parameter", 
	"dynamic mac address only ")
{
	CMDDEBUG();

	/* get bridge name */
	StringT brName = pifVlanMgrGetDefaultBridgeName();

	/* prepare read data */
	struct fdb_entry fdbs[HAL_BRIDGE_MAX_DYN_FDB_ENTRIES];

	/* request to HAL */
	Int32T num = hal_bridge_read_fdb_all(brName, &(fdbs[0]));
	if(num < 0){
		pifLOG(LOG_DEBUG, "fail(%d) to get hal data \n", num);
		cmdPrint(cmsh,"bridge name: %s\n", brName);
		cmdPrint(cmsh,"\tfail to get fdb data \n");
		return CMD_IPC_OK;
	}

	cmdPrint(cmsh,"bridge name: %s\n", brName);
	cmdPrint(cmsh,"%-4s %-8s %-17s %3s %4s\n", "vid", "port-no", "mac-address", "fwd", "time");
	cmdPrint(cmsh,"%-4s %-8s %-17s %3s %4s\n", "---", "-------", "-----------------", "---", "----");

	int i;
	for(i=0; i<num; i++) {
		cmdPrint(cmsh,"%4d %8d %-17s %3d %4d\n",
			   	 fdbs[i].vid,
				 fdbs[i].port_no,
				 mac2str((char*)&(fdbs[i].mac_addr[0])),
				 fdbs[i].is_fwd,
				 fdbs[i].ageing_timer_value
				 );
	}

	cmdPrint(cmsh,"\n");

	return CMD_IPC_OK;
}



/**
 * Description : mac-address-table show static
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
void showStaticMac(struct cmsh *cmsh, AvlNodeLongT* treeNode);
DECMD(cmdFuncMacTableShowStatic,
	CMD_NODE_EXEC,
    IPC_PIF_MGR,
	"show mac-address-table static vlan <1-4095>",
	"show command", 
	"MAC table parameter", 
	"static address only", 
	"vlan parameter", 
	"vlan id 1-4095 ")
{
	CMDDEBUG();
	cmdPrint(cmsh,"\n");

	/* set vid */
	Uint32T vid = atoi(cargv[4]);

	/* get bridge name */
	StringT brName = pifVlanMgrGetDefaultBridgeName();
	cmdPrint(cmsh,"bridge name: %s\n", brName);
	cmdPrint(cmsh,"%18s %7s %4s \n", "mac-address", "vlan-id", "port");
	cmdPrint(cmsh,"%18s %7s %4s \n", "------------------", "-------", "----");

	AvlNodeLongT *treeRoot = getRootMacTable(vid);
	if(treeRoot == NULL){
		cmdPrint(cmsh,"vid %d: no entry \n", vid);
		return CMD_IPC_OK;
	}
	
	showStaticMac(cmsh, treeRoot);

	cmdPrint(cmsh,"\n");

	return CMD_IPC_OK;
}



void showStaticMac(struct cmsh *cmsh, AvlNodeLongT* treeNode)
{
	if(treeNode == NULL) return;

	/* print each vlan node */
	StaticMacT* mac = (StaticMacT*)treeNode->pData;
	cmdPrint(cmsh,"%18s %7d %4s \n", mac2str(mac->macAddr), mac->vid, mac->ifName);

	showStaticMac(cmsh, treeNode->pLeft);
	showStaticMac(cmsh, treeNode->pRight);
}






/**
 * Description : mac-address-table ageing time show
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
void showVlanAgingTime(struct cmsh *cmsh, AvlNodeLongT* treeNode, Int32T aging);
DECMD(cmdFuncMacTableShowAgeing,
	CMD_NODE_EXEC,
    IPC_PIF_MGR|IPC_SHOW_MGR,
	"show mac-address-table aging-time",
	"show command",
	"MAC table parameter", 
	"aging-time in seconds")
{
	CMDDEBUG();

	Int32T agingTime = pifMacAddressTableGetAgingTime();

	cmdPrint(cmsh,"\n");
	cmdPrint(cmsh,"%5s %12s\n", "Vlan", "Aging Time");
	cmdPrint(cmsh,"%5s %12s\n", "----", "----------");

	AvlNodeLongT *treeRoot = getRootVlanTable();
	showVlanAgingTime(cmsh, treeRoot, agingTime);
	cmdPrint(cmsh,"\n");

	return CMD_IPC_OK;
}

void showVlanAgingTime(struct cmsh *cmsh, AvlNodeLongT* treeNode, Int32T aging)
{
	if(treeNode == NULL) return;

	/* print each vlan node */
	VlanT* vlan = (VlanT*)treeNode->pData;
	cmdPrint(cmsh,"%5d %12d", vlan->vid, aging);

	showVlanAgingTime(cmsh, treeNode->pLeft, aging);
	showVlanAgingTime(cmsh, treeNode->pRight, aging);
}










/**
 * Description : static-port-channel add command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncStaticAggAdd,
	CMD_NODE_INTERFACE,
    IPC_PIF_MGR,
	"static-channel-group <1-4>",
	"Interface static channel group", 
	"group number 1-4")
{
	CMDDEBUG();

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, uargv[1]);

	/* check interface name */
	Int32T pifRlt = pifCmiIfSet(ifName);
	if(pifRlt != PIF_CMI_RETURN_OK){
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}

	/* set group id */
	Uint32T aggNumber = atoi(cargv[1]);

	/* request to process configuation */
	if(pifCmiPortChannelAdd(aggNumber) != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}


/**
 * Description : static-port-channel delete command
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
DECMD(cmdFuncStaticAggDel,
	CMD_NODE_INTERFACE,
    IPC_PIF_MGR,
	"no static-channel-group",
	"Negate a command", 
	"Interface static channel group")
{
	CMDDEBUG();

	/* get interface name */
	Int8T  ifName[IF_NAME_SIZE];
	strcpy(ifName, uargv[1]);

	/* check interface name */
	Int32T pifRlt = pifCmiIfSet(ifName);
	if(pifRlt != PIF_CMI_RETURN_OK){
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}

	/* request to process configuation */
	if(pifCmiPortChannelDel() != PIF_CMI_RETURN_OK) {
		cmdPrint(cmsh,"%s \n", pifCmiGetErrorStr());
		return CMD_IPC_OK;
	}
	
	return CMD_IPC_OK;
}



/**
 * Description : Interface show
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
void showInterface(struct cmsh *cmsh, AvlNodeLongT* treeNode);
DECMD(cmdFuncShowInterface,
	CMD_NODE_EXEC,
    IPC_PIF_MGR|IPC_SHOW_MGR,
	"show interface",
	"Show command", 
	"show interface command")
{
	CMDDEBUG();

	AvlNodeLongT *treeRoot = getRootInterfaceTable();
	showInterface(cmsh, treeRoot);
	cmdPrint(cmsh,"\n");

	return CMD_IPC_OK;
}

#define STR_BUFF_SIZE	2048
void showInterface(struct cmsh *cmsh, AvlNodeLongT* treeNode)
{
	Int8T strBuff[STR_BUFF_SIZE] = {};

	if(treeNode == NULL) return;

	/* print each interface node */
	printStrIPInterface(strBuff, (IPInterfaceT*)treeNode->pData); 
	cmdPrint(cmsh,"%s", strBuff);
	memset(strBuff, 0, STR_BUFF_SIZE);

	showInterface(cmsh, treeNode->pLeft);
	showInterface(cmsh, treeNode->pRight);
}




/**
 * Description : Interface show brief
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
void showInterfaceBrief(struct cmsh *cmsh, AvlNodeLongT* treeNode);
DECMD(cmdFuncShowInterfaceBrief,
	CMD_NODE_EXEC,
    IPC_PIF_MGR|IPC_SHOW_MGR,
	"show interface brief",
	"Show command", 
	"interface", 
	"brief view")
{
	CMDDEBUG();

	AvlNodeLongT *treeRoot = getRootInterfaceTable();
	showInterfaceBrief(cmsh, treeRoot);
	cmdPrint(cmsh,"\n");

	return CMD_IPC_OK;
}

#define STR_BUFF_SIZE	2048
void showInterfaceBrief(struct cmsh *cmsh, AvlNodeLongT* treeNode)
{
	Int8T strBuff[STR_BUFF_SIZE] = {};

	if(treeNode == NULL) return;

	/* print each interface node */
	printStrIPInterfaceBrief(strBuff, (IPInterfaceT*)treeNode->pData); 
	cmdPrint(cmsh,"%s", strBuff);
	memset(strBuff, 0, STR_BUFF_SIZE);

	showInterfaceBrief(cmsh, treeNode->pLeft);
	showInterfaceBrief(cmsh, treeNode->pRight);
}




/**
 * Description : Interface show switchport
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
void showInterfaceSwitchport(struct cmsh *cmsh, AvlNodeLongT* treeNode);
DECMD(cmdFuncShowInterfaceSwitchport,
	CMD_NODE_EXEC,
    IPC_PIF_MGR|IPC_SHOW_MGR,
	"show interface switchport",
	"Show command", 
	"interface", 
	"l2 switchport view")
{
	CMDDEBUG();

	AvlNodeLongT *treeRoot = getRootPortTable();
	showInterfaceSwitchport(cmsh, treeRoot);
	cmdPrint(cmsh,"\n");

	return CMD_IPC_OK;
}

void showInterfaceSwitchport(struct cmsh *cmsh, AvlNodeLongT* treeNode)
{
	Int8T strBuff[STR_BUFF_SIZE] = {};

	if(treeNode == NULL) return;

	/* print each interface node */
	printStrPhysicalPortSwitchPort(strBuff, (PhysicalPortT*)treeNode->pData); 
	cmdPrint(cmsh,"%s", strBuff);
	memset(strBuff, 0, STR_BUFF_SIZE);

	showInterfaceSwitchport(cmsh, treeNode->pLeft);
	showInterfaceSwitchport(cmsh, treeNode->pRight);
}


















/**
 * Description : Vlan show
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
void showVlan(struct cmsh *cmsh, AvlNodeLongT* treeNode);
DECMD(cmdFuncShowVlan,
	CMD_NODE_EXEC,
    IPC_PIF_MGR|IPC_SHOW_MGR,
	"show vlan",
	"Show command", 
	"show vlan command")
{
	CMDDEBUG();

	AvlNodeLongT *treeRoot = getRootVlanTable();
	showVlan(cmsh, treeRoot);
	cmdPrint(cmsh,"\n");

	return CMD_IPC_OK;
}

void showVlan(struct cmsh *cmsh, AvlNodeLongT* treeNode)
{
	Int8T strBuff[2048] = {};

	if(treeNode == NULL) return;

	/* print each vlan node */
	printStrVlan(strBuff, (VlanT*)treeNode->pData); 
	cmdPrint(cmsh,"%s", strBuff);

	showVlan(cmsh, treeNode->pLeft);
	showVlan(cmsh, treeNode->pRight);
}




/**
 * Description : static-channe-group show
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
void showAggGroup(struct cmsh *cmsh, AvlNodeLongT* treeNode);
DECMD(cmdFuncShowAggGroup,
	CMD_NODE_EXEC,
    IPC_PIF_MGR|IPC_SHOW_MGR,
	"show static-channel-group",
	"Show command", 
	"show static channel group command")
{
	CMDDEBUG();

	AvlNodeLongT *treeRoot = getRootAggTable();
	showAggGroup(cmsh, treeRoot);
	cmdPrint(cmsh,"\n");

	return CMD_IPC_OK;
}

void showAggGroup(struct cmsh *cmsh, AvlNodeLongT* treeNode)
{
	Int8T strBuff[2048] = {};

	if(treeNode == NULL) return;

	/* print each vlan node */
	printStrAggGroup(strBuff, (AggGroupT*)treeNode->pData); 
	cmdPrint(cmsh,"%s", strBuff);

	showAggGroup(cmsh, treeNode->pLeft);
	showAggGroup(cmsh, treeNode->pRight);
}




/**
 * Description : Layer2 forwarder show
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
StringT pifVlanMgrGetDefaultBridgeName();

DECMD(cmdFuncShowBridgeInfo,
	CMD_NODE_EXEC,
    IPC_PIF_MGR|IPC_SHOW_MGR,
	"show bridge info",
	"Show command", 
	"layer2 bridge", 
	"show layer2 forwader bridge information")
{
	CMDDEBUG();

	struct bridge_info info;
	memset(&info, 0, sizeof(struct bridge_info));

	StringT brName = pifVlanMgrGetDefaultBridgeName();

	Int32T ret = hal_get_bridge_info(brName, &info);
	if(ret != 0){
		pifLOG(LOG_DEBUG, "fail(%d) to get hal data \n", ret);
		cmdPrint(cmsh,"bridge name: %s\n", brName);
		cmdPrint(cmsh,"\tfail to get fdb data \n");
		return CMD_IPC_OK;
	}

	cmdPrint(cmsh,"bridge name: %s\n", brName);
	cmdPrint(cmsh,"\trunning= %d\n", info.up);
	cmdPrint(cmsh,"\tlearning enabled= %d\n", info.learning_enabled);
	cmdPrint(cmsh,"\tdynamic ageing period= %d\n", info.dynamic_ageing_period);
	cmdPrint(cmsh,"\tdynamic ageing value = %d\n", info.dynamic_ageing_timer_value);

	return CMD_IPC_OK;
}


DECMD(cmdFuncShowBridgePorts,
	CMD_NODE_EXEC,
    IPC_PIF_MGR|IPC_SHOW_MGR,
	"show bridge port-list",
	"Show command", 
	"layer2 bridge", 
	"show layer2 forwader bridge information")
{
	CMDDEBUG();

	Uint32T port_list[HAL_BRIDGE_MAX_PORTS];
	memset(port_list, 0, HAL_BRIDGE_MAX_PORTS * sizeof(Uint32T));

	StringT brName = pifVlanMgrGetDefaultBridgeName();

	Int32T num = hal_get_bridge_port_list(brName, port_list);
	if(num <= 0){
		pifLOG(LOG_DEBUG, "fail(%d) to get hal data \n", num);
		cmdPrint(cmsh,"bridge name: %s\n", brName);
		cmdPrint(cmsh,"\tfail to get hal data \n");
		return CMD_IPC_OK;
	}

	cmdPrint(cmsh,"bridge name: %s\n", brName);

	int i;
	for(i=0; i<num; i++) {
		if(port_list[i]) {
			cmdPrint(cmsh,"\tifIndex= %d\n", port_list[i]);
		}
	}
	cmdPrint(cmsh,"\n");

	return CMD_IPC_OK;
}



void showPortState(struct cmsh *cmsh, AvlNodeLongT* treeNode, StringT brName);
DECMD(cmdFuncShowBridgePortState,
	CMD_NODE_EXEC,
    IPC_PIF_MGR|IPC_SHOW_MGR,
	"show bridge port-state",
	"Show command", 
	"layer2 bridge", 
	"show layer2 forwader bridge information")
{
	CMDDEBUG();

	Uint32T port_list[HAL_BRIDGE_MAX_PORTS];
	memset(port_list, 0, HAL_BRIDGE_MAX_PORTS * sizeof(Uint32T));

	StringT brName = pifVlanMgrGetDefaultBridgeName();

	cmdPrint(cmsh,"bridge name: %s\n", brName);

	AvlNodeLongT *treeRoot = getRootPortTable();
	showPortState(cmsh, treeRoot, brName);

	cmdPrint(cmsh,"\n");

	return CMD_IPC_OK;
}

void showPortState(struct cmsh *cmsh, AvlNodeLongT* treeNode, StringT brName)
{
	if(treeNode == NULL) return;

	/* print each port */
	PhysicalPortT *port = (PhysicalPortT*)treeNode->pData;
	Uint32T ifIndex = port->ifIndex;
	Uint32T state;

	Int32T ret = hal_get_bridge_port_state(brName, ifIndex, 0, &state);
	if(ret != 0){
		cmdPrint(cmsh,"\t%s not included in bridge 1", port->name);
	}
	else {
		cmdPrint(cmsh,"\t%s state= %d", port->name, state);
	}

	showPortState(cmsh, treeNode->pLeft, brName);
	showPortState(cmsh, treeNode->pRight, brName);
}


DECMD(cmdFuncShowBridgeMacDynamic,
	CMD_NODE_EXEC,
    IPC_PIF_MGR|IPC_SHOW_MGR,
	"show bridge mac-dynamic",
	"Show command", 
	"layer2 bridge", 
	"show layer2 forwader bridge information")
{
	CMDDEBUG();

	/* get bridge name */
	StringT brName = pifVlanMgrGetDefaultBridgeName();

	/* prepare read data */
	struct fdb_entry fdbs[HAL_BRIDGE_MAX_DYN_FDB_ENTRIES];
	//struct hal_fdb_entry fdbs[HAL_BRIDGE_MAX_DYN_FDB_ENTRIES];

	/* request to HAL */
	Int32T num = hal_bridge_read_fdb_all(brName, &(fdbs[0]));
	if(num < 0){
		pifLOG(LOG_DEBUG, "fail(%d) to get hal data \n", num);
		cmdPrint(cmsh,"bridge name: %s\n", brName);
		cmdPrint(cmsh,"\tfail to get fdb data \n");
		return CMD_IPC_OK;
	}

	cmdPrint(cmsh,"bridge name: %s\n", brName);
	cmdPrint(cmsh,"%-4s %-8s  %s %s  %s\n", "vid", "port-no", "mac", "fwd", "time");

	int i;
	for(i=0; i<num; i++) {
		cmdPrint(cmsh,"%-4d %-8d  %s %1d  %d\n",
			   	 fdbs[i].vid,
				 fdbs[i].port_no,
				 mac2str((char*)&(fdbs[i].mac_addr[0])),
				 fdbs[i].is_fwd,
				 fdbs[i].ageing_timer_value
				 );
	}

	cmdPrint(cmsh,"\n");

	return CMD_IPC_OK;
}


DECMD(cmdFuncShowBridgeMacStatic,
	CMD_NODE_EXEC,
    IPC_PIF_MGR|IPC_SHOW_MGR,
	"show bridge mac-static",
	"Show command", 
	"layer2 bridge", 
	"show layer2 forwader bridge information")
{
	CMDDEBUG();

	/* get bridge name */
	StringT brName = pifVlanMgrGetDefaultBridgeName();

	/* prepare read data */
	struct fdb_entry fdbs[HAL_BRIDGE_MAX_DYN_FDB_ENTRIES];

	/* request to HAL */
	Int32T num = hal_bridge_read_statfdb_all(brName, &(fdbs[0]));
	if(num < 0){
		pifLOG(LOG_DEBUG, "fail(%d) to get hal data \n", num);
		cmdPrint(cmsh,"bridge name: %s\n", brName);
		cmdPrint(cmsh,"\tfail to get fdb data \n");
		return CMD_IPC_OK;
	}

	cmdPrint(cmsh,"bridge name: %s\n", brName);

	int i;
	for(i=0; i<num; i++) {
		cmdPrint(cmsh,"%-4d %-8d  %s %1d    %d\n",
			   	 fdbs[i].vid,
				 fdbs[i].port_no,
				 mac2str((char*)&(fdbs[i].mac_addr[0])),
				 fdbs[i].is_fwd,
				 fdbs[i].ageing_timer_value
				 );
	}

	cmdPrint(cmsh,"\n");

	return CMD_IPC_OK;
}



















/**
 * Description : dynamic software upgrading
 *
 * @param [in] cmsh : cmsh reference
 * @param [in] cargc : arguments number
 * @param [in] cargv : arguments array
 *
 * @retval : CMD_IPC_ERROR
 *           CMD_IPC_WARNING
 *           CMD_IPC_OK
 */
/*
DECMD(cmdFuncUpgradePifmgr,
	CMD_NODE_EXEC,
    IPC_PIF_MGR,
	"upgrade pifmgr version WORD",
	"upgrade", 
	"component name",
	"version",
	"version number" )
{
	CMDDEBUG();

	return CMD_IPC_OK;
}
*/


