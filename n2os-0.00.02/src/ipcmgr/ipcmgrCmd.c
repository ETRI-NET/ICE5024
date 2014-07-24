/*******************************************************************************
 *            Electronics and Telecommunications Research Institute
 * Copyright: 2013 Electronics and Telecommunications Research Institute.
 *            All rights reserved.
 *            No part of this software shall be reproduced, stored in a
 *            retrieval system, or transmitted by any means, electronic,
 *            mechanical, photocopying, recording, or otherwise, without
 *            written permission from ETRI.
*******************************************************************************/

/**
 * @brief :  IPC Manager ?? CLI ???ɾ?
 * - Block Name : IPC Manager
 * - Process Name : ipcmgr
 * - Creator : Jaesu Han
 * - Initial Date : 2014/5/30
 */

/**
* @file : ipcmgrCmd.c
*
* $Id:
* $Author:
* $Date:
* $Revision:
* $LastChangedBy:
**/

/*******************************************************************************
 *                               INCLUDE FILES
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "nnTypes.h"
#include "nnList.h"

#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdInstall.h"

#include "ipcmgrInit.h"


/*******************************************************************************
 *                              GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 *                         GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/

/*******************************************************************************
 *                         LOCAL CONSTANTS/LITERALS/TYPES
 ******************************************************************************/

// N2OS Component String (Ref. nnDefines.h)
const Int8T sProcessString[][24] = {
    "IPC MANAGER",
    "PROCESS MANAGER",
    "PORT INTERFACE MANAGER",
    "RIB MANAGER",
    "POLICY MANAGER",
    "COMMAND MANAGER",
    "MULTICAST RIB MANAGER",
    "LIB MANAGER",
    "CHECKPOINT MANAGER",
    "LACP",
    "MSTP",
    "GVRP",
    "IGMP",
    "RIP",
    "ISIS",
    "OSPF",
    "BGP",
    "PIM",
    "RSVP",
    "LDP",
    "RIB_TESTER",
    "DLU_TESTER"

};

const Int8T sProcessName[][16] = {
    "ipcmgr",
    "procmgr",
    "pifmgr",
    "ribmgr",
    "polmgr",
    "cmd",
    " ",
    "libmgr",
    " ",
    "lacp",
    "mstp",
    "gvrp",
    "igmp",
    "rip",
    "isis",
    "ospf",
    "bgp",
    "pim",
    "rsvp",
    "ldp",
    "ribtester",
    "dlutester"

};


// N2OS Event String (Ref. nnDefines.h)
const Int8T sEventString[][32] = {
    "EVENT_0",
    "LCS_COMPONENT_ERROR_OCCURRED",
    "LCS_COMPONENT_SERVICE_STATUS",
    "ROUTER_ID",
    "INTERFACE_ADD",
    "INTERFACE_DELETE",
    "INTERFACE_UP",
    "INTERFACE_DOWN",
    "INTERFACE_ADDRESS_ADD",
    "INTERFACE_ADDRESS_DELETE",
    "IPV4_ROUTE_ADD",
    "IPV4_ROUTE_DELETE",
    "IPV6_ROUTE_ADD",
    "IPV6_ROUTE_DELETE",
    "EVENT_2",
    "EVENT_3"

};

// N2OS Event Priority String (Ref. nnDefines.h)
const Int8T sEventPriString[][16] = {
    "PRI_HIGH",
    "PRI_MIDDLE",
    "PRI_LOW"

};

/*******************************************************************************
*                               LOCAL VARIABLES
*******************************************************************************/

/*******************************************************************************
 *                                   CODE
 ******************************************************************************/

// IPC Connect Info CMD
DECMD(cmdIpcmgrShowIpcConnectList,
  CMD_NODE_VIEW,
  IPC_IPC_MGR | IPC_SHOW_MGR,
  "show ipc info",
  "Show information",
  "ipc",
  "connect info")
{
  Int32T i;

//  cmdPrint(cmsh, "IPC Connect Info (N2OS IPC Channel : TCP)\n");
  cmdPrint(cmsh, "%-24s %-15s %8s\n", 
                 "Process", "IP", "PORT");

  cmdPrint(cmsh, "---------------------------------------------------\n");
 
  cmdPrint(cmsh, "%-24s %3s.%3s.%3s.%3s %8d\n",
                 sProcessString[IPC_MANAGER],
                 "127","0","0","1", 10000);                
 
  for(i=1; i<MAX_PROCESS_CNT; i++)
  {
      struct sockaddr processInfo;
      memset(&processInfo, 0x00, sizeof(struct sockaddr));

      if(ipcProcessInfoGet(i, &processInfo) == SUCCESS)
      {
          cmdPrint(cmsh, "%-24s %3u.%3u.%3u.%3u %8d\n",
                          sProcessString[i],
                          IP_PRINT
                          (((struct sockaddr_in*)&processInfo)->sin_addr.s_addr),
                           ((struct sockaddr_in*)&processInfo)->sin_port);
      }
  }

  cmdPrint(cmsh, "---------------------------------------------------\n");
 
  return CMD_IPC_OK;
}

// EVENT Subscribe CMD
DECMD(cmdIpcmgrShowEventSubscribeListAll,
  CMD_NODE_VIEW,
  IPC_IPC_MGR | IPC_SHOW_MGR,
  "show event subscribe all",
  "Show information",
  "event",
  "subscribe list",
  "All")
{
  Int32T i;

  cmdPrint(cmsh, "Event Subscribe Process List\n");
  cmdPrint(cmsh, "%-32s %-24s %-16s\n", 
                 "EVENT Name", "Process Name", "Priority");

  for(i=0; i<MAX_EVENT_CNT; i++)
  {
      ListNodeT *pTempNode = NULL;
            
      for(pTempNode = pIpcmgr->gEventSubList[i].subProcess->pHead;
          pTempNode != NULL;
          pTempNode = pTempNode->pNext)
      {
        if(pTempNode == NULL)
          break;
              
        cmdPrint(cmsh,"%-32s %-24s %-16s\n",
          sEventString[i],
          sProcessString[((EventSubProcessT *)pTempNode->pData)->process],
          sEventPriString[((EventSubProcessT *)pTempNode->pData)->priority]);
      }
  }

  return CMD_IPC_OK;
}

DECMD(cmdIpcmgrShowEventSubscribeListProcess,
  CMD_NODE_VIEW,
  IPC_IPC_MGR | IPC_SHOW_MGR,
  "show event subscribe WORD",
  "Show information",
  "event",
  "subscribe list",
  "Process Name")
{
  Int32T i, processId, cnt = 0;
  Int8T word[32] = {0, };

  memcpy(word, cargv[3], strlen(cargv[3]));

  for(i=0; i<MAX_PROCESS_CNT; i++)
  {
    if(strcmp(word, sProcessName[i]) == 0)
    {
        processId = i;
        break;
    }
  }

  if(i == MAX_PROCESS_CNT)
  {
    cmdPrint(cmsh, "None Process : %s\n", word);
    return CMD_IPC_OK;
  }

  cmdPrint(cmsh, "%s Subscribe Event List\n", sProcessString[processId]);
  cmdPrint(cmsh, "%-32s %-16s\n",
                 "EVENT Name", "Priority");


  for(i=0; i<MAX_EVENT_CNT; i++)
  {
    ListNodeT *pTempNode = NULL;

    for(pTempNode = pIpcmgr->gEventSubList[i].subProcess->pHead;
        pTempNode != NULL;
        pTempNode = pTempNode->pNext)
    {
      if(pTempNode == NULL)
        break;

      if(processId == ((EventSubProcessT *)pTempNode->pData)->process)
      {
        cnt++;
        cmdPrint(cmsh,"%-32s %-16s\n",
          sEventString[i],
          sEventPriString[((EventSubProcessT *)pTempNode->pData)->priority]);
      }
    }
  }

  if(cnt == 0)
  {
    cmdPrint(cmsh, "%s : None Subscibe Event\n", sProcessString[processId]);
  }

  return CMD_IPC_OK;
}

