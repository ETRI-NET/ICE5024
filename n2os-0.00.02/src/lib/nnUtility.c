/**
* @file : nnUtility.c
* @brief : N2OS Common Library - Utility 관련
*
* $Id:
* $Author:
* $Date:
* $Log$
* $Revision:
* $LastChangedBy:
* $LastChanged$
*
*            Electronics and Telecommunications Research Institute
* Copyright: 2013 Electronics and Telecommunications Research Institute.
*            All rights reserved.
*            No part of this software shall be reproduced, stored in a
*            retrieval system, or transmitted by any means, electronic,
*            mechanical, photocopying, recording, or otherwise, without
*            written permission from ETRI.
**/

/*******************************************************************************
 *                               INCLUDE FILES
 ******************************************************************************/

#include "nnUtility.h"
#include "nnDefines.h"
#include "nosLib.h"

/*******************************************************************************
 *                              GLOBAL VARIABLES
 ******************************************************************************/


/*******************************************************************************
 *                         GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 *                         LOCAL CONSTANTS/LITERALS/TYPES
 ******************************************************************************/


/*******************************************************************************
*                               LOCAL VARIABLES
*******************************************************************************/


/*******************************************************************************
 *                                   CODE
 ******************************************************************************/

const TypeDescTableT gProcessTypes[] =
{
    NN_DESC_ENTRY (IPC_MANAGER, "IPC_MANAGER"),
    NN_DESC_ENTRY (PROCESS_MANAGER, "PROCESS_MANAGER"),
    NN_DESC_ENTRY (PORT_INTERFACE_MANAGER, "PORT_INTERFACE_MANAGER"),
    NN_DESC_ENTRY (RIB_MANAGER, "RIB_MANAGER"),
    NN_DESC_ENTRY (POLICY_MANAGER, "POLICY_MANAGER"),
    NN_DESC_ENTRY (COMMAND_MANAGER, "COMMAND_MANAGER"),
    NN_DESC_ENTRY (MULTICAST_RIB_MANAGER, "MULTICAST_RIB_MANAGER"),
    NN_DESC_ENTRY (LIB_MANAGER, "LIB_MANAGER"),
    NN_DESC_ENTRY (CHECKPOINT_MANAGER, "CHECKPOINT_MANAGER"),
    NN_DESC_ENTRY (LACP, "LACP"),
    NN_DESC_ENTRY (MSTP, "MSTP"),
    NN_DESC_ENTRY (GVRP, "GVRP"),
    NN_DESC_ENTRY (IGMP, "IGMP"),
    NN_DESC_ENTRY (RIP, "RIP"),
    NN_DESC_ENTRY (ISIS, "ISIS"),
    NN_DESC_ENTRY (OSPF, "OSPF"),
    NN_DESC_ENTRY (BGP, "BGP"),
    NN_DESC_ENTRY (PIM, "PIM"),
    NN_DESC_ENTRY (RSVP, "RSVP"),
    NN_DESC_ENTRY (LDP, "LDP")
};


void nnPrintHexf(StringT p, Int32T len, Int32T logPri)
{
    Int8T buff[65535];
    StringT cp = p;
    Int32T hcnt = 0;

    memset(buff, 0x00, sizeof(buff));
    NNLOG(logPri, "nnPrintHexf Start\n");

    while (len > 0)
    {
        if (hcnt == 0)
        {
            sprintf(buff, "x%2.21lx: ", (long unsigned int)(cp-p));
        }
        if ((hcnt%4) == 0)
        {
            sprintf(buff, "%s ", buff);
        }

        sprintf(buff, "%s%2.2x", buff, (0xff&*cp++));

        len--;
        hcnt++;

        if (hcnt >= 16 || len == 0)
        {
            NNLOG(logPri, "%s\n", buff);
            memset(buff, 0x00, sizeof(buff));
            hcnt = 0;
        }
    }

    NNLOG(logPri, "nnPrintHexf End\n");
}
/*
struct routeDescTable
{
  Uint32T type;
  const char * string;
  char chr;
};
typedef struct routeDescTable RouteDescTableT;
*/

#define DESC_ENTRY(T,S,C) [(T)] = { (T), (S), (C) }
static const RouteDescTableT routeTypes[] = { 
  DESC_ENTRY    (RIB_ROUTE_TYPE_SYSTEM,    "system",       'X' ),
  DESC_ENTRY    (RIB_ROUTE_TYPE_KERNEL,    "kernel",       'K' ),
  DESC_ENTRY    (RIB_ROUTE_TYPE_CONNECT,   "connected",    'C' ),
  DESC_ENTRY    (RIB_ROUTE_TYPE_STATIC,    "static",       'S' ),
  DESC_ENTRY    (RIB_ROUTE_TYPE_RIP,       "rip",          'R' ),
  DESC_ENTRY    (RIB_ROUTE_TYPE_RIPNG,     "ripng",        'R' ),
  DESC_ENTRY    (RIB_ROUTE_TYPE_OSPF,      "ospf",         'O' ),
  DESC_ENTRY    (RIB_ROUTE_TYPE_OSPF6,     "ospf6",        'O' ),
  DESC_ENTRY    (RIB_ROUTE_TYPE_ISIS,      "isis",         'I' ),
  DESC_ENTRY    (RIB_ROUTE_TYPE_BGP,       "bgp",          'B' ),
  DESC_ENTRY    (RIB_ROUTE_TYPE_HSLS,      "hsls",         'H' ),
};
#undef DESC_ENTRY

static const RouteDescTableT unknown = { 0, "unknown", '?' };

/*
 * Description : 루트 타입에 따른 routeDescTable Entry를 찾는 함수. 
 *
 * param [in] routeNum : 루트타입
 *
 * retval : ribRouteDesTable Entry의 주소
 */
static const RouteDescTableT * nnRouteLookup(u_int routeNum)
{
  u_int i;

  if (routeNum >= sizeof(routeTypes)/sizeof(routeTypes[0]))
  {
    NNLOG(LOG_ERR, "unknown ribmgr route type: %u\n", routeNum);
    return &unknown;
  }

  if (routeNum == routeTypes[routeNum].type)
  {
    return &routeTypes[routeNum];
  }

  for (i = 0; i < sizeof(routeTypes)/sizeof(routeTypes[0]); i++)
  {
    if (routeNum == routeTypes[i].type)
    {   
      NNLOG(LOG_ERR, "internal error: route type table out of order "
                "while searching for %u, please notify developers\n", routeNum);
      return &routeTypes[i];
    }   
  }
  NNLOG(LOG_ERR, "internal error: cannot find route type %u in table!\n", routeNum);
  return &unknown;
}


/* Description : 루트 타입에 따른 문자열값을 제공하는 함수. */
const StringT nnRoute2String(u_int routeNum)
{
  return nnRouteLookup(routeNum)->string;
}


/* Description : 루트 타입에 따른 문자값을 제공하는 함수. */
Int8T nnRoute2Char(u_int routeNum)
{
  return nnRouteLookup(routeNum)->chr;
}
