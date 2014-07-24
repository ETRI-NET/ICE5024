#ifndef __CMD_COMP_PROCESS_H__
#define __CMD_COMP_PROCESS_H__
/**
 * @brief Overview :
 * @brief Creator: Thanh Nguyen Ba
 * @file      : nnCmdService.h
 *
 * $Id: nnCmdMd5.h 725 2014-01-17 09:11:34Z hyryu $
 * $Author: hyryu $
 * $Date: 2014-01-17 04:11:34 -0500 (Fri, 17 Jan 2014) $
 * $Log$
 * $Revision: 725 $
 * $LastChangedBy: thanh $
 * $LastChanged$
 *
 *                      Electronics and Telecommunications Research Institute
 * Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.
 *           No part of this software shall be reproduced, stored in a retrieval system, or
 *           transmitted by any means, electronic, mechanical, photocopying, recording,
 *           or otherwise, without written permission from ETRI.
 **/

typedef struct compCmshGlobal_s
{
	Int32T compId;
	struct cmsh *cmsh;
}compCmshGlobal_T;

extern compCmshGlobal_T *gCmshData;

void *compCmdInit(Int32T compId, Int32T (*funcName)(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv));
void compCmdIpcProcess(void *gCmshGlobal, Int32T sockId, void *message, Uint32T size);
void compCmdUpdate(compCmshGlobal_T *cmshData, Int32T (*funcName)(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv));
void compCmdFree(void *gCmsh);
#endif
