#ifndef __NN_CMD_INSTALL_H__
#define __NN_CMD_INSTALL_H__
/**
 * @brief Overview :
 * @brief Creator: Thanh Nguyen Ba
 * @file      : nnCmdInstall.h
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

#define DECMD(funcname, node, flags, cmdstr, ...)      \
Int32T funcname(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv)

#define ALICMD(funcname, node, flags, cmdstr, ...)

#define DENODE(funcname, node, pnode, prompt, priv, cmdstr, ...)      \
Int32T funcname(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv)

#define DENODEWR(nodeid, funcname)                      \
Int32T funcname(char** nodeFile)


#define DENODEC(funcname, nodeID, compID)      \
Int32T funcname(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv)

struct cmshNodeCmTree
{
  Int32T nodeID;
  Int8T *nodeStr;
};

Int32T cmdCommandInstall(struct cmsh *cmsh, Int32T node,
    Int32T *sFlags, Int32T numFlags, struct cmdElement *cel);

Int32T cmdNodeInstall(struct cmsh *cmsh, Int32T pnode, Int32T cnode, Int32T priv,
    Int8T *str, struct cmdElement *mCel, struct cmdElement *eCel);

Int32T cmdNodeRunningInstall(struct cmdList *table, Int32T node, Int8T *str);

Int32T cmdUpdateMode(struct cmsh* cmsh, Int32T argc, char **argv);
Int32T cmdChangeNodeReq(struct cmsh* cmsh, Int32T nodeID, Int32T funcID, Int32T compID, Int32T argc, char **argv);
Int32T cmdChangeNodeRes(struct cmsh* cmsh, Int32T compID, Int32T outData, Int8T *outString);

Int32T cmdFuncInstall(struct cmsh *cmsh, Int32T funcID, Int32T (*funcName)(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv));
Int32T cmdWriteConfigInstall(struct cmsh *cmsh, Int32T (*funcName)(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv));
Int32T cmdWriteConfigUpdate(struct cmsh *cmsh, Int32T (*funcName)(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv));

#endif
