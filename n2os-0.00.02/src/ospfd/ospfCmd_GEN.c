/*
 * @file      :  ../../ospfd/ospfCmd_GEN.c  
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
cmdFuncEnterOspf(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoRouterOspf(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfRouterId(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoRouterId(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfPassiveInterface(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoPassiveInterface(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNetworkArea(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoNetworkArea(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfAreaRange(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfAreaRangeNotAdvertise(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoAreaRange(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfAreaRangeSubstitute(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfno_ospf_area_range_substitute(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfAreaVlink(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoAreaVlink(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfAreaShortcut(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoOspfAreaShortcut(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfAreaStub(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfAreaStubNoSummary(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoAreaStub(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoAreaStubNoSummary(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfAreaNssaTranslateNoSummary(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfAreaNssaTranslate(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfAreaNssa(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfAreaNssaNoSummary(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoAareaNssa(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoAreaNssaNoSummary(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfAreaDefaultCost(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoAreaDefaultCost(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfAreaExportList(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoAreaExportList(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfAreaImportList(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoAreaImportList(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfAreaFilterList(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoAreaFilterList(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfAreaAuthenticationMessageDigest(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfAreaAuthentication(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoAreaAuthentication(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfAbrType(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoAbrType(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfLogAdjacencyChanges(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfLogAdjacencyChangesDetail(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoLogAdjacencyChanges(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoLogAdjacencyChangesDetail(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfCompatibleRfc1583(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoCompatibleRfc1583(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfTimersThrottleSpf(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfTimersSpf(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoTimersThrottleSpf(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNeighbor(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNeighborPollInterval(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfno_ospf_neighbor(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfRefreshTimer(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoOspfRefreshTimer(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfAutoCostReferenceBandwidth(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoAutoCostReferenceBandwidth(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfShowIp(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfShowIpInterface(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfShowIpNeighbor(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfShowIpNeighborAll(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfShowIpNeighborInt(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfShowIpNeighborId(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfShowIpNeighborDetail(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfShowIpNeighborDetailAll(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfShowIpNeighborIntDetail(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfShowIpDatabase(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfShowIpDatabaseTypeAdvRouter(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfIpAuthenticationArgs(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfIpAuthentication(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoIpAuthentication(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfIpAuthenticationKey(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoIpAuthenticationKey(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfIpMessageDigestKey(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoIpMessageDigestKey(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfIpCost(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoIpCost(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoIpCost2(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfIpDeadInterval(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfIpDeadIntervalMinimal(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoIpDeadInterval(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfip_ospf_hello_interval(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoIpHelloInterval(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfIpNetwork(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoIpNetwork(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfIpPriority(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoIpPriority(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfIpRetransmitInterval(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoIpRetransmitInterval(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfIpTransmitDelay(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoIpTransmitDelay(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfRedistributeSourceMetricType(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfRedistributeSourceTypeMetric(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfRedistributeSourceMetricRoutemap(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfRedistributeSourceTypeRoutemap(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfRedistributeSourceRoutemap(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoRedistributeSource(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDistributeListOut(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoDistributeListOut(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDefaultInformationOriginateMetricTypeRoutemap(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDefaultInformationOriginateMetricRoutemap(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDefaultInformationOriginateRoutemap(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDefaultInformationOriginateTypeMetricRoutemap(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDefaultInformationOriginateTypeRoutemap(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDefaultInformationOriginateAlwaysMetricTypeRoutemap(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDefaultInformationOriginateAlwaysMetricRoutemap(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDefaultInformationOriginateAlwaysRoutemap(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDefaultInformationOriginateAlwaysTypeMetricRoutemap(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDefaultInformationOriginateAlwaysTypeRoutemap(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoDefaultInformationOriginate(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDefaultMetric(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoDefaultMetric(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDistance(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoDistance(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoDistanceOspf(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDistanceOspfIntra(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDistanceOspfIntraInter(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDistanceOspfIntraExternal(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDistanceOspfIntraInterExternal(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDistanceOspfIntraExternalInter(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDistanceOspfInter(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDistanceOspfInterIntra(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDistanceOspfInterExternal(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDistanceOspfInterIntraExternal(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDistanceOspfInterExternalIntra(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDistanceOspfExternal(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDistanceOspfExternalIntra(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDistanceOspfExternalInter(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDistanceOspfExternalIntraInter(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDistanceOspfExternalInterIntra(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDistanceSource(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoDistanceSource(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDistanceSourceAccessList(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoDistanceSourceAccessList(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfIpMtuIgnore(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoIpMtuIgnore(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfMaxMetricRouterLsaAdmin(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoMaxMetricRouterLsaAdmin(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfMaxMetricRouterLsaStartup(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoMaxMetricRouterLsaStartup(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfMaxMetricRouterLsaShutdown(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoMaxMetricRouterLsaShutdown(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfShowIpBorderRouters(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfShowIpRoute(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfCapabilityOpaque(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoCapabilityOpaque(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfMplsTe(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoMplsTe(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfMplsTeRouterAddr(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfMplsTeLinkMetric(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfMplsTeLinkMaxbw(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfMplsTeLinkMaxRsvBw(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfMplsTeLinkUnrsvBw(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfMplsTeLinkRscClsclr(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfShowMplsTeRouter(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfShowMplsTeLink(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDebugPacket(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoDebugPacket(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfdebug_ospf_ism(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoDebugIsm(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDebugNsm(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoDebugNsm(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDebugLsa(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoDebugLsa(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDebugZebra(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoDebugZebra(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDebugEvent(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoDebugEvent(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfDebugNssa(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoDebugNssa(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfMatchIpNexthop(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoMatchIpNexthop(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfMatchIpNextHopPrefixList(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoMatchIpNextHopPrefixList(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfMatchIpAddress(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoMatchIpAddress(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfMatchIpAddressPrefixList(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoMatchIpAddressPrefixList(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfMatchInterface(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoMatchInterface(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfSetMetric(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoSetMetric(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfSetMetricType(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
extern Int32T
cmdFuncOspfNoSetMetricType(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,
				Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, 
				Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);
void
cmdFuncGlobalInstall(struct cmsh *cmsh)
{
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_0_ID, *cmdFuncEnterOspf);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_1_ID, *cmdFuncOspfNoRouterOspf);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_2_ID, *cmdFuncOspfRouterId);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_3_ID, *cmdFuncOspfNoRouterId);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_4_ID, *cmdFuncOspfPassiveInterface);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_5_ID, *cmdFuncOspfNoPassiveInterface);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_6_ID, *cmdFuncOspfNetworkArea);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_7_ID, *cmdFuncOspfNoNetworkArea);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_8_ID, *cmdFuncOspfAreaRange);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_9_ID, *cmdFuncOspfAreaRangeNotAdvertise);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_10_ID, *cmdFuncOspfNoAreaRange);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_11_ID, *cmdFuncOspfAreaRangeSubstitute);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_12_ID, *cmdFuncOspfno_ospf_area_range_substitute);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_13_ID, *cmdFuncOspfAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_14_ID, *cmdFuncOspfNoAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_15_ID, *cmdFuncOspfAreaShortcut);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_16_ID, *cmdFuncOspfNoOspfAreaShortcut);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_17_ID, *cmdFuncOspfAreaStub);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_18_ID, *cmdFuncOspfAreaStubNoSummary);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_19_ID, *cmdFuncOspfNoAreaStub);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_20_ID, *cmdFuncOspfNoAreaStubNoSummary);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_21_ID, *cmdFuncOspfAreaNssaTranslateNoSummary);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_22_ID, *cmdFuncOspfAreaNssaTranslate);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_23_ID, *cmdFuncOspfAreaNssa);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_24_ID, *cmdFuncOspfAreaNssaNoSummary);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_25_ID, *cmdFuncOspfNoAareaNssa);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_26_ID, *cmdFuncOspfNoAreaNssaNoSummary);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_27_ID, *cmdFuncOspfAreaDefaultCost);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_28_ID, *cmdFuncOspfNoAreaDefaultCost);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_29_ID, *cmdFuncOspfAreaExportList);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_30_ID, *cmdFuncOspfNoAreaExportList);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_31_ID, *cmdFuncOspfAreaImportList);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_32_ID, *cmdFuncOspfNoAreaImportList);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_33_ID, *cmdFuncOspfAreaFilterList);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_34_ID, *cmdFuncOspfNoAreaFilterList);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_35_ID, *cmdFuncOspfAreaAuthenticationMessageDigest);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_36_ID, *cmdFuncOspfAreaAuthentication);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_37_ID, *cmdFuncOspfNoAreaAuthentication);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_38_ID, *cmdFuncOspfAbrType);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_39_ID, *cmdFuncOspfNoAbrType);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_40_ID, *cmdFuncOspfLogAdjacencyChanges);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_41_ID, *cmdFuncOspfLogAdjacencyChangesDetail);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_42_ID, *cmdFuncOspfNoLogAdjacencyChanges);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_43_ID, *cmdFuncOspfNoLogAdjacencyChangesDetail);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_44_ID, *cmdFuncOspfCompatibleRfc1583);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_45_ID, *cmdFuncOspfNoCompatibleRfc1583);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_46_ID, *cmdFuncOspfTimersThrottleSpf);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_47_ID, *cmdFuncOspfTimersSpf);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_48_ID, *cmdFuncOspfNoTimersThrottleSpf);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_49_ID, *cmdFuncOspfNeighbor);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_50_ID, *cmdFuncOspfNeighborPollInterval);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_51_ID, *cmdFuncOspfno_ospf_neighbor);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_52_ID, *cmdFuncOspfRefreshTimer);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_53_ID, *cmdFuncOspfNoOspfRefreshTimer);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_54_ID, *cmdFuncOspfAutoCostReferenceBandwidth);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_55_ID, *cmdFuncOspfNoAutoCostReferenceBandwidth);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_56_ID, *cmdFuncOspfShowIp);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_57_ID, *cmdFuncOspfShowIpInterface);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_58_ID, *cmdFuncOspfShowIpNeighbor);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_59_ID, *cmdFuncOspfShowIpNeighborAll);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_60_ID, *cmdFuncOspfShowIpNeighborInt);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_61_ID, *cmdFuncOspfShowIpNeighborId);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_62_ID, *cmdFuncOspfShowIpNeighborDetail);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_63_ID, *cmdFuncOspfShowIpNeighborDetailAll);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_64_ID, *cmdFuncOspfShowIpNeighborIntDetail);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_65_ID, *cmdFuncOspfShowIpDatabase);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_66_ID, *cmdFuncOspfShowIpDatabaseTypeAdvRouter);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_67_ID, *cmdFuncOspfIpAuthenticationArgs);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_68_ID, *cmdFuncOspfIpAuthentication);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_69_ID, *cmdFuncOspfNoIpAuthentication);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_70_ID, *cmdFuncOspfIpAuthenticationKey);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_71_ID, *cmdFuncOspfNoIpAuthenticationKey);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_72_ID, *cmdFuncOspfIpMessageDigestKey);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_73_ID, *cmdFuncOspfNoIpMessageDigestKey);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_74_ID, *cmdFuncOspfIpCost);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_75_ID, *cmdFuncOspfNoIpCost);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_76_ID, *cmdFuncOspfNoIpCost2);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_77_ID, *cmdFuncOspfIpDeadInterval);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_78_ID, *cmdFuncOspfIpDeadIntervalMinimal);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_79_ID, *cmdFuncOspfNoIpDeadInterval);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_80_ID, *cmdFuncOspfip_ospf_hello_interval);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_81_ID, *cmdFuncOspfNoIpHelloInterval);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_82_ID, *cmdFuncOspfIpNetwork);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_83_ID, *cmdFuncOspfNoIpNetwork);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_84_ID, *cmdFuncOspfIpPriority);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_85_ID, *cmdFuncOspfNoIpPriority);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_86_ID, *cmdFuncOspfIpRetransmitInterval);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_87_ID, *cmdFuncOspfNoIpRetransmitInterval);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_88_ID, *cmdFuncOspfIpTransmitDelay);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_89_ID, *cmdFuncOspfNoIpTransmitDelay);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_90_ID, *cmdFuncOspfRedistributeSourceMetricType);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_91_ID, *cmdFuncOspfRedistributeSourceTypeMetric);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_92_ID, *cmdFuncOspfRedistributeSourceMetricRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_93_ID, *cmdFuncOspfRedistributeSourceTypeRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_94_ID, *cmdFuncOspfRedistributeSourceRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_95_ID, *cmdFuncOspfNoRedistributeSource);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_96_ID, *cmdFuncOspfDistributeListOut);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_97_ID, *cmdFuncOspfNoDistributeListOut);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_98_ID, *cmdFuncOspfDefaultInformationOriginateMetricTypeRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_99_ID, *cmdFuncOspfDefaultInformationOriginateMetricRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_100_ID, *cmdFuncOspfDefaultInformationOriginateRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_101_ID, *cmdFuncOspfDefaultInformationOriginateTypeMetricRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_102_ID, *cmdFuncOspfDefaultInformationOriginateTypeRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_103_ID, *cmdFuncOspfDefaultInformationOriginateAlwaysMetricTypeRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_104_ID, *cmdFuncOspfDefaultInformationOriginateAlwaysMetricRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_105_ID, *cmdFuncOspfDefaultInformationOriginateAlwaysRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_106_ID, *cmdFuncOspfDefaultInformationOriginateAlwaysTypeMetricRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_107_ID, *cmdFuncOspfDefaultInformationOriginateAlwaysTypeRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_108_ID, *cmdFuncOspfNoDefaultInformationOriginate);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_109_ID, *cmdFuncOspfDefaultMetric);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_110_ID, *cmdFuncOspfNoDefaultMetric);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_111_ID, *cmdFuncOspfDistance);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_112_ID, *cmdFuncOspfNoDistance);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_113_ID, *cmdFuncOspfNoDistanceOspf);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_114_ID, *cmdFuncOspfDistanceOspfIntra);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_115_ID, *cmdFuncOspfDistanceOspfIntraInter);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_116_ID, *cmdFuncOspfDistanceOspfIntraExternal);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_117_ID, *cmdFuncOspfDistanceOspfIntraInterExternal);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_118_ID, *cmdFuncOspfDistanceOspfIntraExternalInter);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_119_ID, *cmdFuncOspfDistanceOspfInter);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_120_ID, *cmdFuncOspfDistanceOspfInterIntra);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_121_ID, *cmdFuncOspfDistanceOspfInterExternal);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_122_ID, *cmdFuncOspfDistanceOspfInterIntraExternal);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_123_ID, *cmdFuncOspfDistanceOspfInterExternalIntra);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_124_ID, *cmdFuncOspfDistanceOspfExternal);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_125_ID, *cmdFuncOspfDistanceOspfExternalIntra);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_126_ID, *cmdFuncOspfDistanceOspfExternalInter);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_127_ID, *cmdFuncOspfDistanceOspfExternalIntraInter);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_128_ID, *cmdFuncOspfDistanceOspfExternalInterIntra);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_129_ID, *cmdFuncOspfDistanceSource);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_130_ID, *cmdFuncOspfNoDistanceSource);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_131_ID, *cmdFuncOspfDistanceSourceAccessList);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_132_ID, *cmdFuncOspfNoDistanceSourceAccessList);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_133_ID, *cmdFuncOspfIpMtuIgnore);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_134_ID, *cmdFuncOspfNoIpMtuIgnore);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_135_ID, *cmdFuncOspfMaxMetricRouterLsaAdmin);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_136_ID, *cmdFuncOspfNoMaxMetricRouterLsaAdmin);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_137_ID, *cmdFuncOspfMaxMetricRouterLsaStartup);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_138_ID, *cmdFuncOspfNoMaxMetricRouterLsaStartup);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_139_ID, *cmdFuncOspfMaxMetricRouterLsaShutdown);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_140_ID, *cmdFuncOspfNoMaxMetricRouterLsaShutdown);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_141_ID, *cmdFuncOspfShowIpBorderRouters);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_142_ID, *cmdFuncOspfShowIpRoute);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_143_ID, *cmdFuncOspfCapabilityOpaque);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_144_ID, *cmdFuncOspfNoCapabilityOpaque);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_145_ID, *cmdFuncOspfMplsTe);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_146_ID, *cmdFuncOspfNoMplsTe);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_147_ID, *cmdFuncOspfMplsTeRouterAddr);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_148_ID, *cmdFuncOspfMplsTeLinkMetric);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_149_ID, *cmdFuncOspfMplsTeLinkMaxbw);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_150_ID, *cmdFuncOspfMplsTeLinkMaxRsvBw);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_151_ID, *cmdFuncOspfMplsTeLinkUnrsvBw);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_152_ID, *cmdFuncOspfMplsTeLinkRscClsclr);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_153_ID, *cmdFuncOspfShowMplsTeRouter);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_154_ID, *cmdFuncOspfShowMplsTeLink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_155_ID, *cmdFuncOspfDebugPacket);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_156_ID, *cmdFuncOspfNoDebugPacket);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_157_ID, *cmdFuncOspfdebug_ospf_ism);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_158_ID, *cmdFuncOspfNoDebugIsm);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_159_ID, *cmdFuncOspfDebugNsm);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_160_ID, *cmdFuncOspfNoDebugNsm);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_161_ID, *cmdFuncOspfDebugLsa);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_162_ID, *cmdFuncOspfNoDebugLsa);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_163_ID, *cmdFuncOspfDebugZebra);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_164_ID, *cmdFuncOspfNoDebugZebra);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_165_ID, *cmdFuncOspfDebugEvent);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_166_ID, *cmdFuncOspfNoDebugEvent);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_167_ID, *cmdFuncOspfDebugNssa);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_168_ID, *cmdFuncOspfNoDebugNssa);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_169_ID, *cmdFuncOspfMatchIpNexthop);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_170_ID, *cmdFuncOspfNoMatchIpNexthop);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_171_ID, *cmdFuncOspfMatchIpNextHopPrefixList);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_172_ID, *cmdFuncOspfNoMatchIpNextHopPrefixList);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_173_ID, *cmdFuncOspfMatchIpAddress);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_174_ID, *cmdFuncOspfNoMatchIpAddress);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_175_ID, *cmdFuncOspfMatchIpAddressPrefixList);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_176_ID, *cmdFuncOspfNoMatchIpAddressPrefixList);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_177_ID, *cmdFuncOspfMatchInterface);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_178_ID, *cmdFuncOspfNoMatchInterface);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_179_ID, *cmdFuncOspfSetMetric);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_180_ID, *cmdFuncOspfNoSetMetric);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_181_ID, *cmdFuncOspfSetMetricType);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_182_ID, *cmdFuncOspfNoSetMetricType);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_183_ID, *cmdFuncOspfRouterId);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_184_ID, *cmdFuncOspfNoRouterId);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_185_ID, *cmdFuncOspfPassiveInterface);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_186_ID, *cmdFuncOspfPassiveInterface);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_187_ID, *cmdFuncOspfNoPassiveInterface);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_188_ID, *cmdFuncOspfNoPassiveInterface);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_189_ID, *cmdFuncOspfAreaRange);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_190_ID, *cmdFuncOspfAreaRange);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_191_ID, *cmdFuncOspfAreaRange);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_192_ID, *cmdFuncOspfNoAreaRange);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_193_ID, *cmdFuncOspfNoAreaRange);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_194_ID, *cmdFuncOspfNoAreaRange);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_195_ID, *cmdFuncOspfAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_196_ID, *cmdFuncOspfNoAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_197_ID, *cmdFuncOspfAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_198_ID, *cmdFuncOspfNoAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_199_ID, *cmdFuncOspfAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_200_ID, *cmdFuncOspfNoAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_201_ID, *cmdFuncOspfAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_202_ID, *cmdFuncOspfNoAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_203_ID, *cmdFuncOspfAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_204_ID, *cmdFuncOspfAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_205_ID, *cmdFuncOspfNoAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_206_ID, *cmdFuncOspfAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_207_ID, *cmdFuncOspfNoAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_208_ID, *cmdFuncOspfAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_209_ID, *cmdFuncOspfNoAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_210_ID, *cmdFuncOspfAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_211_ID, *cmdFuncOspfAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_212_ID, *cmdFuncOspfNoAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_213_ID, *cmdFuncOspfAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_214_ID, *cmdFuncOspfAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_215_ID, *cmdFuncOspfNoAreaVlink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_216_ID, *cmdFuncOspfCompatibleRfc1583);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_217_ID, *cmdFuncOspfNoCompatibleRfc1583);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_218_ID, *cmdFuncOspfNoTimersThrottleSpf);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_219_ID, *cmdFuncOspfNeighbor);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_220_ID, *cmdFuncOspfNeighbor);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_221_ID, *cmdFuncOspfNeighborPollInterval);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_222_ID, *cmdFuncOspfno_ospf_neighbor);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_223_ID, *cmdFuncOspfno_ospf_neighbor);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_224_ID, *cmdFuncOspfno_ospf_neighbor);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_225_ID, *cmdFuncOspfNoOspfRefreshTimer);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_226_ID, *cmdFuncOspfShowIpInterface);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_227_ID, *cmdFuncOspfShowIpDatabase);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_228_ID, *cmdFuncOspfShowIpDatabase);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_229_ID, *cmdFuncOspfShowIpDatabase);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_230_ID, *cmdFuncOspfShowIpDatabase);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_231_ID, *cmdFuncOspfShowIpDatabaseTypeAdvRouter);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_232_ID, *cmdFuncOspfIpAuthenticationArgs);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_233_ID, *cmdFuncOspfIpAuthentication);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_234_ID, *cmdFuncOspfNoIpAuthentication);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_235_ID, *cmdFuncOspfIpAuthenticationKey);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_236_ID, *cmdFuncOspfIpAuthenticationKey);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_237_ID, *cmdFuncOspfNoIpAuthenticationKey);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_238_ID, *cmdFuncOspfNoIpAuthenticationKey);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_239_ID, *cmdFuncOspfIpMessageDigestKey);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_240_ID, *cmdFuncOspfIpMessageDigestKey);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_241_ID, *cmdFuncOspfNoIpMessageDigestKey);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_242_ID, *cmdFuncOspfNoIpMessageDigestKey);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_243_ID, *cmdFuncOspfIpCost);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_244_ID, *cmdFuncOspfIpCost);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_245_ID, *cmdFuncOspfIpCost);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_246_ID, *cmdFuncOspfNoIpCost);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_247_ID, *cmdFuncOspfNoIpCost);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_248_ID, *cmdFuncOspfNoIpCost);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_249_ID, *cmdFuncOspfNoIpCost2);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_250_ID, *cmdFuncOspfNoIpCost2);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_251_ID, *cmdFuncOspfNoIpCost2);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_252_ID, *cmdFuncOspfIpDeadInterval);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_253_ID, *cmdFuncOspfIpDeadInterval);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_254_ID, *cmdFuncOspfIpDeadIntervalMinimal);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_255_ID, *cmdFuncOspfNoIpDeadInterval);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_256_ID, *cmdFuncOspfNoIpDeadInterval);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_257_ID, *cmdFuncOspfip_ospf_hello_interval);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_258_ID, *cmdFuncOspfip_ospf_hello_interval);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_259_ID, *cmdFuncOspfNoIpHelloInterval);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_260_ID, *cmdFuncOspfNoIpHelloInterval);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_261_ID, *cmdFuncOspfIpNetwork);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_262_ID, *cmdFuncOspfNoIpNetwork);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_263_ID, *cmdFuncOspfIpPriority);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_264_ID, *cmdFuncOspfIpPriority);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_265_ID, *cmdFuncOspfNoIpPriority);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_266_ID, *cmdFuncOspfNoIpPriority);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_267_ID, *cmdFuncOspfIpRetransmitInterval);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_268_ID, *cmdFuncOspfIpRetransmitInterval);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_269_ID, *cmdFuncOspfNoIpRetransmitInterval);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_270_ID, *cmdFuncOspfNoIpRetransmitInterval);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_271_ID, *cmdFuncOspfIpTransmitDelay);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_272_ID, *cmdFuncOspfIpTransmitDelay);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_273_ID, *cmdFuncOspfNoIpTransmitDelay);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_274_ID, *cmdFuncOspfNoIpTransmitDelay);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_275_ID, *cmdFuncOspfRedistributeSourceMetricType);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_276_ID, *cmdFuncOspfRedistributeSourceMetricType);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_277_ID, *cmdFuncOspfRedistributeSourceTypeMetric);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_278_ID, *cmdFuncOspfRedistributeSourceTypeMetric);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_279_ID, *cmdFuncOspfRedistributeSourceTypeMetric);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_280_ID, *cmdFuncOspfDefaultInformationOriginateMetricTypeRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_281_ID, *cmdFuncOspfDefaultInformationOriginateMetricTypeRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_282_ID, *cmdFuncOspfDefaultInformationOriginateMetricTypeRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_283_ID, *cmdFuncOspfDefaultInformationOriginateTypeMetricRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_284_ID, *cmdFuncOspfDefaultInformationOriginateTypeMetricRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_285_ID, *cmdFuncOspfDefaultInformationOriginateAlwaysMetricTypeRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_286_ID, *cmdFuncOspfDefaultInformationOriginateAlwaysMetricTypeRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_287_ID, *cmdFuncOspfDefaultInformationOriginateAlwaysMetricTypeRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_288_ID, *cmdFuncOspfDefaultInformationOriginateAlwaysTypeMetricRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_289_ID, *cmdFuncOspfDefaultInformationOriginateAlwaysTypeMetricRoutemap);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_290_ID, *cmdFuncOspfNoDefaultMetric);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_291_ID, *cmdFuncOspfIpMtuIgnore);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_292_ID, *cmdFuncOspfNoIpMtuIgnore);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_293_ID, *cmdFuncOspfCapabilityOpaque);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_294_ID, *cmdFuncOspfNoCapabilityOpaque);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_295_ID, *cmdFuncOspfMplsTe);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_296_ID, *cmdFuncOspfShowMplsTeLink);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_297_ID, *cmdFuncOspfDebugPacket);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_298_ID, *cmdFuncOspfDebugPacket);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_299_ID, *cmdFuncOspfNoDebugPacket);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_300_ID, *cmdFuncOspfNoDebugPacket);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_301_ID, *cmdFuncOspfdebug_ospf_ism);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_302_ID, *cmdFuncOspfNoDebugIsm);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_303_ID, *cmdFuncOspfDebugNsm);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_304_ID, *cmdFuncOspfNoDebugNsm);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_305_ID, *cmdFuncOspfDebugLsa);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_306_ID, *cmdFuncOspfNoDebugLsa);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_307_ID, *cmdFuncOspfDebugZebra);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_308_ID, *cmdFuncOspfNoDebugZebra);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_309_ID, *cmdFuncOspfNoMatchIpNexthop);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_310_ID, *cmdFuncOspfNoMatchIpNextHopPrefixList);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_311_ID, *cmdFuncOspfNoMatchIpAddress);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_312_ID, *cmdFuncOspfNoMatchIpAddressPrefixList);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_313_ID, *cmdFuncOspfNoMatchInterface);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_314_ID, *cmdFuncOspfNoSetMetric);
	cmdFuncInstall(cmsh, CMD_FUNC_OSPFCMD_315_ID, *cmdFuncOspfNoSetMetricType);
}
