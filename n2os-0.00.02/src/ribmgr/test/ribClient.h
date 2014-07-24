#if !defined(_ribClient_h)
#define _ribClient_h


/**************************************************************************************************** 
*                      Electronics and Telecommunications Research Institute
* Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.
*           No part of this software shall be reproduced, stored in a retrieval system, or
*           transmitted by any means, electronic, mechanical, photocopying, recording,
*           or otherwise, without written permission from ETRI.
****************************************************************************************************/


/**   
 * @brief : This file defines ribClient Function Test code.
 *  - Block Name : ripd
 *  - Process Name : ripd
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : ribClient.h
 *
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include "nnPrefix.h"
#include "nnBuffer.h"

/* Routing API Structure */
struct routeApi4 {
  //Uint8T         routeType;
  Uint8T         flags;
  Uint8T         message;

  Prefix4T       prefix;

  Uint8T         nextHopNum;
  struct in_addr nextHop;

  Uint8T         ifIndexNum;
  Uint32T        ifIndex;

  Uint8T         distance;
  Int32T         metric;
};
typedef struct routeApi4 routeApi4T;


void routeAddIpv4 (Uint8T rType, routeApi4T rApi);
void routeDeleteIpv4 (Uint8T rType, routeApi4T rApi);
void ipcReadCallback (Int32T msgId, void * data, Uint32T dataLen);
void eventReadCallback (Int32T msgId, void * data, Uint32T dataLen);

#endif /* _ribClient_h */
