/*
 * @file      :  ../../polmgr/polMgrCmd_GEN.c  
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
cmdPolFuncRipOamSwitch(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdPollShowTest(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdPolFuncRipNoRouterRip1(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdPolFuncRipNoRouterRip2(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdPolFuncRipNoRouterRip3(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
void
cmdFuncGlobalInstall(struct cmsh *cmsh)
{
	cmdFuncInstall(cmsh, CMD_FUNC_POLMGRCMD_1_ID, *cmdPolFuncRipOamSwitch);
	cmdFuncInstall(cmsh, CMD_FUNC_POLMGRCMD_2_ID, *cmdPollShowTest);
	cmdFuncInstall(cmsh, CMD_FUNC_POLMGRCMD_3_ID, *cmdPolFuncRipNoRouterRip1);
	cmdFuncInstall(cmsh, CMD_FUNC_POLMGRCMD_4_ID, *cmdPolFuncRipNoRouterRip2);
	cmdFuncInstall(cmsh, CMD_FUNC_POLMGRCMD_5_ID, *cmdPolFuncRipNoRouterRip3);
}
