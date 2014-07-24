#if !defined(_ribmgrInit_h)
#define _ribmgrInit_h

/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the Global data structure definitions.
 *  - Block Name : RIB Manager
 *  - Process Name : ribmgr
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : ribmgrInit.h
 *
 * $Author: sckim007 $
 * $Date: 2014-03-11 16:30:27 +0900 (Tue, 11 Mar 2014) $
 * $Revision: 967 $
 * $LastChangedBy: sckim007 $
 */
#include <bits/sockaddr.h>
#include <linux/netlink.h>

#include "nnTypes.h"
#include "nnVector.h"
#include "nnList.h"
#include "nnPrefix.h"

#include "nnRibDefines.h"

struct NLSocket
{
  Int32T sock;
  Int32T seq;
  struct sockaddr_nl snl;
  const char *name;
};
typedef struct NLSocket NLSocketT;


/*
 * Clinet data structure.
 */
struct Client 
{
  /* Client component id. Definded in nnDefines.h. */
  Uint8T componentId;

  /* Routing protocol's type. Defined in nnRibDefines.h. */
  Uint8T routeType;

  /* Redistribute default route flag. */
  Uint8T redistributeDefault;

  /* Redistribute information of each clients. */
  Uint8T redistribute[RIB_ROUTE_TYPE_MAX];
};
typedef struct Client ClientT;


struct Ribmgr {
  /* Global datas tructure declaration for debugging.
   * Operation refer ribmgrDebug.h and ribmgrDebug.c 
   */
  Uint32T ribmgrDebugEvent;
  Uint32T ribmgrDebugPacket;
  Uint32T ribmgrDebugKernel;
  Uint32T ribmgrDebugRib;

  /* Global datas tructure declaration for rib .
   * Operation refer ribmgrRib.h and ribmgrRib.c 
   */
  /* Vector for routing table.  */
  VectorT pVrfVector;


  /* Default Routing Table Number (0~255).
   * Can be set by user (table command)
   */
  Uint8T rtmTableDefault;

  /*
   * RouterID global data structure.
   */
  ListT * pRidAllSortedList;
  ListT * pRidLoSortedList;
  Prefix4T ridUserAssigned;

  /*
   * List for interface list.
   */
  ListT * pIfList;

  /*
   * List for client list.
   */
  ListT * pClientList;

  /*
   * Distribute 2 dimension array.
   *  - First index is requesting protocol id.
   *  - Second index is requested route type. 
   */
//  Uint8T Redistribute[RIB_ROUTE_TYPE_MAX][RIB_ROUTE_TYPE_MAX];
  Uint8T Redistribute[MAX_PROCESS_CNT][RIB_ROUTE_TYPE_MAX];

  /*
   * Socket related data structure.
   */
  NLSocketT NETLINK; /* kernel messages */
  NLSocketT  NETLINK_CMD; /* command channel */
  void * pNetlinkFdEvent; /* fd's event pointer */

  /* Command related pointer. */
  void * pCmshGlobal;
};
typedef struct Ribmgr RibmgrT;

extern RibmgrT * pRibmgr;

extern void ribInitProcess();
extern void ribTermProcess ();
extern void ribRestartProcess ();
extern void ribHoldProcess ();
extern void ribTimerProcess (Int32T fd, Int16T event, void * arg);
extern void ribTimerProcessOne (Int32T fd, Int16T event, void * arg);
extern void ribTimerProcessTen (Int32T fd, Int16T event, void * arg);

extern void ribSignalProcess (Int32T signalType);
//extern void initCompletCallback();
extern void ribIpcProcess (Int32T msgId, void * data, Uint32T dataLen);
extern void ribEventProcess (Int32T msgId, void * data, Uint32T dataLen);

#endif
