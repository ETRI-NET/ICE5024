#ifndef __NN_CMD_DEFINES_H__
#define __NN_CMD_DEFINES_H__

/**
 * @brief Overview :
 * @brief Creator: Thanh Nguyen Ba
 * @file      : nnCmdDefines.h
 *
 * $Id: nnCmdDefines.h 725 2014-01-17 09:11:34Z hyryu $
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

enum
{
  IPC_SHOW_MGR = 0,
  IPC_CMSH_MGR,
  IPC_IPC_MGR,
  IPC_PRO_MGR,
  IPC_PIF_MGR,
  IPC_RIB_MGR,
  IPC_POL_MGR,
  IPC_CM_MGR,
  IPC_LACP,
  IPC_MSTP,
  IPC_RIP,
  IPC_ISIS,
  IPC_OSPF,
  IPC_RIB_TESTER,
  IPC_DLU_TESTER,
  IPC_MAX_MGR
};

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX    108
#endif

#define NOS_TELNET_DEFAULT_PORT_ID  62000
#define NOS_SSH_DEFAULT_PORT_ID     62001
#define NOS_INETD_CONFIG_FILE_PATH  "/etc/inetd.nos.conf"

#ifndef _NOS_CFG_FILE_
#define _NOS_CFG_FILE_ "/etc/nos.conf"
#define _NOS_CFG_MAX_BUFF_SIZE 1024
#endif
#endif
