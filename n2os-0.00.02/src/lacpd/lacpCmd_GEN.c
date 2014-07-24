/*
 * @file      :  ../../lacpd/lacpCmd_GEN.c  
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
cmdFuncLacpTimeout(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncLacpNoTimeout(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncLacpPortChannel(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncLacpNoPortChannel(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncLacpShowCliPortChannel(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncLacpShowPifPortChannel(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncLacpShowPortChannelConfig(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncLacpShowPortChannelState(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncNoLacpDebug(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncLacpDebug(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
void
cmdFuncGlobalInstall(struct cmsh *cmsh)
{
	cmdFuncInstall(cmsh, CMD_FUNC_LACPCMD_0_ID, *cmdFuncLacpTimeout);
	cmdFuncInstall(cmsh, CMD_FUNC_LACPCMD_1_ID, *cmdFuncLacpNoTimeout);
	cmdFuncInstall(cmsh, CMD_FUNC_LACPCMD_2_ID, *cmdFuncLacpPortChannel);
	cmdFuncInstall(cmsh, CMD_FUNC_LACPCMD_3_ID, *cmdFuncLacpNoPortChannel);
	cmdFuncInstall(cmsh, CMD_FUNC_LACPCMD_4_ID, *cmdFuncLacpShowCliPortChannel);
	cmdFuncInstall(cmsh, CMD_FUNC_LACPCMD_5_ID, *cmdFuncLacpShowPifPortChannel);
	cmdFuncInstall(cmsh, CMD_FUNC_LACPCMD_6_ID, *cmdFuncLacpShowPortChannelConfig);
	cmdFuncInstall(cmsh, CMD_FUNC_LACPCMD_7_ID, *cmdFuncLacpShowPortChannelState);
	cmdFuncInstall(cmsh, CMD_FUNC_LACPCMD_8_ID, *cmdFuncNoLacpDebug);
	cmdFuncInstall(cmsh, CMD_FUNC_LACPCMD_9_ID, *cmdFuncLacpDebug);
}
