/*
 * @file      :  ../../processmgr/processmgrCmd_GEN.c  
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
cmdProcShowProcess(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdProcRedundancyStates(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdProcReloadAt(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdProcShowReload(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdProcReloadCancelAllPeerPower(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdProcReloadAllPeerPowerNow(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdProcNoReload(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdProShowReloadCause(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdProcShowVersion(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdProcReloadIn(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdProcLiveupdateQuestion(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdProcLiveupdateProcess(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
void
cmdFuncGlobalInstall(struct cmsh *cmsh)
{
	cmdFuncInstall(cmsh, CMD_FUNC_PROCESSMGRCMD_0_ID, *cmdProcShowProcess);
	cmdFuncInstall(cmsh, CMD_FUNC_PROCESSMGRCMD_1_ID, *cmdProcRedundancyStates);
	cmdFuncInstall(cmsh, CMD_FUNC_PROCESSMGRCMD_2_ID, *cmdProcReloadAt);
	cmdFuncInstall(cmsh, CMD_FUNC_PROCESSMGRCMD_3_ID, *cmdProcShowReload);
	cmdFuncInstall(cmsh, CMD_FUNC_PROCESSMGRCMD_4_ID, *cmdProcReloadCancelAllPeerPower);
	cmdFuncInstall(cmsh, CMD_FUNC_PROCESSMGRCMD_5_ID, *cmdProcReloadAllPeerPowerNow);
	cmdFuncInstall(cmsh, CMD_FUNC_PROCESSMGRCMD_6_ID, *cmdProcNoReload);
	cmdFuncInstall(cmsh, CMD_FUNC_PROCESSMGRCMD_7_ID, *cmdProShowReloadCause);
	cmdFuncInstall(cmsh, CMD_FUNC_PROCESSMGRCMD_8_ID, *cmdProcShowVersion);
	cmdFuncInstall(cmsh, CMD_FUNC_PROCESSMGRCMD_9_ID, *cmdProcReloadIn);
	cmdFuncInstall(cmsh, CMD_FUNC_PROCESSMGRCMD_10_ID, *cmdProcLiveupdateQuestion);
	cmdFuncInstall(cmsh, CMD_FUNC_PROCESSMGRCMD_11_ID, *cmdProcLiveupdateProcess);
}
