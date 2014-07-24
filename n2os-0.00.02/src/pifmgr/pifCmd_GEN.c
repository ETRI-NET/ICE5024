/*
 * @file      :  ../../pifmgr/pifCmd_GEN.c  
 * @brief       :  \n * \n * $Id: cmdGlobalCli.c 863 2014-02-18 06:04:16Z thanh 
 * $Author: thanh 
 * $Date: 2014-02-18 01:04:16 -0500 (Tue, 18 Feb 2014) 
 * $Log
 * $Revision: 863 
 * $LastChangedBy: thanh 
 * $LastChangedn * 
 *                      Electronics and Telecommunications Research Institute
 * Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.
 *           No part of this software shall be reproduced, stored in a retrieval system, or
 *           transmitted by any means, electronic, mechanical, photocopying, recording,
 *           or otherwise, without written permission from ETRI.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <termios.h>
#include "nnTypes.h"
#include "nnCmdDefines.h"
#include "nnStr.h"
#include "nnCmdLink.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"
#include "nnCmdInstall.h"
#include "nnGlobalCmd.h"

extern Int32T
cmdFuncIfAdminSet(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncIfAdminUnset(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncIfSpeedSet(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncIfDuplexSet(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncIfAddressSet(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncIfAddressUnset(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncIfSwportUnset(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncIfSwportSet(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncVlanPortModeSet(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncVlanAccessVidDefault(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncVlanAccessVidAdd(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncVlanAccessVidDel(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncVlanTrunkVidAllowed(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncVlanTrunkVidAdd(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncVlanTrunkVidDel(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncVlanTrunkVidExcept(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncVlanTrunkVidClear(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncVlanAccessClear(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncVlanAdd(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncVlanDel(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncStaticAggAdd(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncStaticAggDel(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncShowInterface(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncShowInterfaceBrief(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncShowInterfaceSwitchport(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncShowVlan(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncShowAggGroup(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncShowBridgeInfo(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncShowBridgePorts(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncShowBridgePortState(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncShowBridgeMacDynamic(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncShowBridgeMacStatic(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
void
cmdFuncGlobalInstall(struct cmsh *cmsh)
{
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_0_ID, *cmdFuncIfAdminSet);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_1_ID, *cmdFuncIfAdminUnset);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_2_ID, *cmdFuncIfSpeedSet);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_3_ID, *cmdFuncIfDuplexSet);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_4_ID, *cmdFuncIfAddressSet);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_5_ID, *cmdFuncIfAddressUnset);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_6_ID, *cmdFuncIfSwportUnset);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_7_ID, *cmdFuncIfSwportSet);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_8_ID, *cmdFuncVlanPortModeSet);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_9_ID, *cmdFuncVlanAccessVidDefault);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_10_ID, *cmdFuncVlanAccessVidAdd);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_11_ID, *cmdFuncVlanAccessVidDel);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_12_ID, *cmdFuncVlanTrunkVidAllowed);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_13_ID, *cmdFuncVlanTrunkVidAdd);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_14_ID, *cmdFuncVlanTrunkVidDel);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_15_ID, *cmdFuncVlanTrunkVidExcept);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_16_ID, *cmdFuncVlanTrunkVidClear);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_17_ID, *cmdFuncVlanAccessClear);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_18_ID, *cmdFuncVlanAdd);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_19_ID, *cmdFuncVlanDel);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_20_ID, *cmdFuncStaticAggAdd);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_21_ID, *cmdFuncStaticAggDel);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_22_ID, *cmdFuncShowInterface);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_23_ID, *cmdFuncShowInterfaceBrief);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_24_ID, *cmdFuncShowInterfaceSwitchport);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_25_ID, *cmdFuncShowVlan);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_26_ID, *cmdFuncShowAggGroup);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_27_ID, *cmdFuncShowBridgeInfo);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_28_ID, *cmdFuncShowBridgePorts);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_29_ID, *cmdFuncShowBridgePortState);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_30_ID, *cmdFuncShowBridgeMacDynamic);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_31_ID, *cmdFuncShowBridgeMacStatic);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_32_ID, *cmdFuncVlanAdd);
	cmdFuncInstall(cmsh, CMD_FUNC_PIFCMD_33_ID, *cmdFuncVlanAdd);
}
