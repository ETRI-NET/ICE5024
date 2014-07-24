// -----------------------------------------------------------------
//      Electronics and Telecommunications Research Institute 
//
// Filename:     $Source: /ribmgr/test/ripMain.c
//               $Id: riprMain.c,v 0.01 09/05/13, by suncheul kim
//
// Subsystem:    RIP Test
//
// Overview:     This file contains the main function.
//
// Creator:      $Author: suncheul kim 
//
// Owner:        suncheul kim
//
// Copyright:    2013 ETRI. All Rights Reserved.
//               No part of this software shall be reproduced,
//               stored in a retrieval system, or transmitted by
//               any means, electronic, mechanical, photocopying,
//               recording, or otherwise, without written 
//               permission from ETRI Inc.
//
// Modificaiton History:
// ====================
// 11/10/13    suncheul kim    Initial Creation.
//
// -----------------------------------------------------------------

#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "nnTypes.h"
#include "nnRibDefines.h"
#include "ribmgrRib.h"

#include "nosLib.h"
#include "nnPrefix.h"
#include "nnBuffer.h"

#include "ribClient.h"
#include "nnList.h"


char * pStrTimeBegin;
char * pStrTimeEnd;

// Global Defination
ListT * pRouteList = NULL;
struct in_addr gNextHop;

// External Defination
extern char * time2Str(void);

Int8T
routeListCmpFunc(void  * pOldData, void * pNewData)
{
  if ((((Prefix4T *)pOldData)->prefix.s_addr) > (((Prefix4T *)pNewData)->prefix.s_addr))
  {
     return 0; 
  }
  else if ((((Prefix4T *)pOldData)->prefix.s_addr) < (((Prefix4T *)pNewData)->prefix.s_addr))
  {
    return 1;
  }
  /* same data */
  else
  {
    return -1;
  }
}

struct in_addr
combineIp(Uint8T a, Uint8T b, Uint8T c, Uint8T d)
{
  char buff[16] = {};
  struct in_addr ipAddr;

  snprintf(buff, 16, "%d.%d.%d.%d", a, b, c, d);
  ipAddr.s_addr = inet_addr(buff); 

  return ipAddr;
}

void 
createRouteList(Uint32T routeNum)
{
  struct in_addr tmpAddr;
  Uint32T totalNum = 0;
  Uint8T i,j;

  Uint8T a = 1; // don't touch
  Uint8T b = 1; // this value increase
  Uint8T c = 1; // this value increase
  Uint8T d = 0; // don't touch 

  for(i = 1 ; i < 255 ; i++, b++)
  {
    for(j = 1, c = 1 ; j < 255; j++, c++)
    {
      tmpAddr = combineIp(a, b, c, d);
      //printf("totalNum = %d, combined ip = %s\n", totalNum, inet_ntoa(tmpAddr));

      // Add to List
      Prefix4T * pPrefix = NNMALLOC (MEM_ROUTE_RIB, sizeof(Prefix4T));
      pPrefix->family    = AF_INET;
      pPrefix->prefixLen = 24;
      memcpy(&pPrefix->prefix, &tmpAddr, sizeof(struct in_addr));

      //printf("\tprefix/prefixlen = %s/%d \n", 
      //        inet_ntoa(pPrefix->prefix), pPrefix->prefixLen);

      if(nnListAddNode(pRouteList, pPrefix) < 0)
      {
        printf("[Error] Fail Add List...\n");
        return ;
      }

      totalNum ++;
      if (totalNum >= routeNum)
        break;
    }

    if (totalNum >= routeNum)
      break;
  }
}

void
showRouteList()
{
  printf("Show Route List Data all...\n");
  ListNodeT * pNode = NULL;
  Prefix4T  * pData = NULL;
  for(pNode = pRouteList->pHead; pNode != NULL; pNode = pNode->pNext)
  {
    pData = pNode->pData;
    printf("\tprefix/len = %s/%d \n", inet_ntoa(pData->prefix), pData->prefixLen);
  }
}


void
routeAddMultipleTest()
{
  Int32T isRight;
  char buff[32]={};
  char nexthopBuff[32] = {};
  Uint32T routeNum = 0;
  routeApi4T rApi;

TEST_AGAIN : 

  printf("\t Number of Route Testing = ");  
  fgets(buff, 32, stdin);
  sscanf(buff, "%u", &routeNum);

  printf("\t nexthop = ");   
  fgets(buff, 32, stdin);
  sscanf(buff, "%s", nexthopBuff);

  printf("\n");
  printf("\t : Number of Route is %d\n", routeNum);
  printf("\t : nexthop  = %s\n", nexthopBuff);
  printf("\n");

  printf("\n");
  printf("\tinput values are right ? [yes=1/no=0]");
  fgets(buff, 32, stdin);
  sscanf(buff, "%d", &isRight);

  if(isRight != 1)
    goto TEST_AGAIN;

  /////////////////////////////////////////////////////////////
  // Make List
  /////////////////////////////////////////////////////////////
  createRouteList(routeNum);

  /////////////////////////////////////////////////////////////
  // Show List
  /////////////////////////////////////////////////////////////
  //showRouteList();

  /////////////////////////////////////////////////////////////
  // nexthop format
  /////////////////////////////////////////////////////////////
  if(nnCnvStringtoAddr(&rApi.nextHop, nexthopBuff) != SUCCESS)
  {
    printf("\t Wrong Nexthop Address Type\n");
    goto TEST_AGAIN;
  }

  /////////////////////////////////////////////////////////////
  // Copy Nexthop to Reuse at Delete Function
  /////////////////////////////////////////////////////////////
  memcpy(&gNextHop, &rApi.nextHop, sizeof(struct in_addr));

  /////////////////////////////////////////////////////////////
  // Set Value for API Structure
  /////////////////////////////////////////////////////////////
  rApi.flags = 0;
  rApi.message |= RIB_MESSAGE_NEXTHOP;
  rApi.message |= RIB_MESSAGE_DISTANCE;
  rApi.message |= RIB_MESSAGE_METRIC;
  rApi.distance = 100;
  rApi.metric = 100;

  /////////////////////////////////////////////////////////////
  // Call Add Route to Rib Manager
  /////////////////////////////////////////////////////////////
  ListNodeT * pNode = NULL;
  Prefix4T  * pData = NULL;
  for(pNode = pRouteList->pHead; pNode != NULL; pNode = pNode->pNext)
  {
    pData = pNode->pData;
    memcpy(&rApi.prefix, pData, sizeof(Prefix4T));
    printf("\tprefix/len = %s/%d \n", inet_ntoa(pData->prefix), pData->prefixLen);
    /////////////////////////////////////////////////////////////
    // Request Route Add Function
    /////////////////////////////////////////////////////////////
    routeAddIpv4(RIB_ROUTE_TYPE_RIP, rApi); 
  }
}

void
routeDeleteMultipleTest()
{
  routeApi4T rApi;

  rApi.flags = 0;
  rApi.message |= RIB_MESSAGE_NEXTHOP;
  rApi.message |= RIB_MESSAGE_DISTANCE;
  rApi.message |= RIB_MESSAGE_METRIC;
  memcpy(&rApi.nextHop, &gNextHop, sizeof(struct in_addr));
  rApi.distance = 100;
  rApi.metric = 100;

  /////////////////////////////////////////////////////////////
  // Call Add Route to Rib Manager
  /////////////////////////////////////////////////////////////
  ListNodeT * pNode = NULL;
  Prefix4T  * pData = NULL;
  for(pNode = pRouteList->pHead; pNode != NULL; pNode = pNode->pNext)
  {
    pData = pNode->pData;
    memcpy(&rApi.prefix, pData, sizeof(Prefix4T));
    printf("\tprefix/len = %s/%d \n", inet_ntoa(pData->prefix), pData->prefixLen);
    /////////////////////////////////////////////////////////////
    // Request Route Delete Function
    /////////////////////////////////////////////////////////////
    routeDeleteIpv4(RIB_ROUTE_TYPE_RIP, rApi);
  }

  nnListDeleteAllNode(pRouteList);
}


Int32T
routeAddTest()
{
  char buff[32]={};
  char prefixBuff[32] = {};
  char nexthopBuff[32] = {};
  Int32T  isRight;
  
  routeApi4T rApi;

TEST_AGAIN :
 
  printf("Route Add Testing Menu\n");

  printf("\t prefix[ex> 1.1.1.0/24] = ");    
  fgets(buff, 32, stdin );
  sscanf(buff, "%s", prefixBuff);

  printf("\t nexthop = ");   
  fgets(buff, 32, stdin);
  sscanf(buff, "%s", nexthopBuff);
  rApi.message |= RIB_MESSAGE_NEXTHOP;

  printf("\t distance = ");  
  fgets(buff, 32, stdin);
  sscanf(buff, "%hhu", &rApi.distance);
  rApi.message |= RIB_MESSAGE_DISTANCE;

  printf("\t metric = ");
  fgets(buff, 32, stdin);
  sscanf(buff, "%d", &rApi.metric);
  rApi.message |= RIB_MESSAGE_METRIC;
  
  
  printf("\n");
  printf("\t input valus are\n");
  printf("\t : prefix/prefixlen = %s\n", prefixBuff);
  printf("\t : nexthop  = %s\n", nexthopBuff);
  printf("\t : distance = %d\n", rApi.distance);
  printf("\t : metric   = %d\n", rApi.metric);

  printf("\n");
  printf("\tinput values are right ? [yes=1/no=0]");
  fgets(buff, 32, stdin);
  sscanf(buff, "%d", &isRight);

  if(isRight != 1)
    goto TEST_AGAIN;

  printf(" let's write down for route test....\n");

  /////////////////////////////////////////////////////////////
  // check route and mask format
  /////////////////////////////////////////////////////////////
  if(nnCnvStringtoPrefix4(&rApi.prefix, prefixBuff) != SUCCESS)
  {
    printf("\t Wrong Prefix Type....\n");
    goto TEST_AGAIN;
  } 
  printf("prefix = %s/%d\n", inet_ntoa(rApi.prefix.prefix), rApi.prefix.prefixLen);
  
  /////////////////////////////////////////////////////////////
  // nexthop format
  /////////////////////////////////////////////////////////////
  if(nnCnvStringtoAddr(&rApi.nextHop, nexthopBuff) != SUCCESS)
  {
    printf("\t Wrong Nexthop Address Type\n");
    goto TEST_AGAIN;
  }
  printf("nexthop = %s\n", inet_ntoa(rApi.nextHop));

  /////////////////////////////////////////////////////////////
  // distance
  /////////////////////////////////////////////////////////////
  if((rApi.distance < 0) || (rApi.distance > 255))
  {
    printf("\t Wrong Distance Value\n");
  }
  printf("distance = %d\n", rApi.distance);

  /////////////////////////////////////////////////////////////
  // metric
  /////////////////////////////////////////////////////////////
  if((rApi.metric < 0) || (rApi.metric > 65535))
  {
    printf("\t Wrong Distance Value\n");
  }
  printf("metric = %d\n", rApi.metric);

  /////////////////////////////////////////////////////////////
  // Request Route Add Function
  /////////////////////////////////////////////////////////////
  routeAddIpv4(RIB_ROUTE_TYPE_RIP, rApi); 
 
  return 0;
}


Int32T
routeDeleteTest()
{
  char buff[32]={};
  char prefixBuff[32] = {};
  char nexthopBuff[32] = {};
  Int32T  isRight;

  routeApi4T rApi;

TEST_AGAIN :
 
  printf("Route Delete Testing Menu\n");

  printf("\t prefix[ex> 1.1.1.0/24] = ");    
  fgets(buff, 32, stdin );
  sscanf(buff, "%s", prefixBuff);

  printf("\t nexthop = ");   
  fgets(buff, 32, stdin);
  sscanf(buff, "%s", nexthopBuff);
  rApi.message |= RIB_MESSAGE_NEXTHOP;

  printf("\t metric = ");    
  fgets(buff, 32, stdin);
  sscanf(buff, "%u", &rApi.metric);
  rApi.message |= RIB_MESSAGE_METRIC;
  
  
  printf("\n");
  printf("\t input valus are\n");
  printf("\t : prefix/prefixlen = %s\n", prefixBuff);
  printf("\t : nexthop  = %s\n", nexthopBuff);
  printf("\t : metric   = %d\n", rApi.metric);

  printf("\n");
  printf("\tinput values are right ? [yes=1/no=0]");
  fgets(buff, 32, stdin);
  sscanf(buff, "%d", &isRight);

  if(isRight != 1)
    goto TEST_AGAIN;

  printf(" let's write down for route test....\n");

  /////////////////////////////////////////////////////////////
  // check route and mask format
  /////////////////////////////////////////////////////////////
  if(nnCnvStringtoPrefix4(&rApi.prefix, prefixBuff) != SUCCESS)
  {
    printf("\t Wrong Prefix Type....\n");
    goto TEST_AGAIN;
  } 
  printf("prefix = %s/%d\n", inet_ntoa(rApi.prefix.prefix), rApi.prefix.prefixLen);
  
  /////////////////////////////////////////////////////////////
  // nexthop format
  /////////////////////////////////////////////////////////////
  if(nnCnvStringtoAddr(&rApi.nextHop, nexthopBuff) != SUCCESS)
  {
    printf("\t Wrong Nexthop Address Type\n");
    goto TEST_AGAIN;
  }
  printf("nexthop = %s\n", inet_ntoa(rApi.nextHop));

  /////////////////////////////////////////////////////////////
  // metric
  /////////////////////////////////////////////////////////////
  if((rApi.metric < 0) || (rApi.metric > 65535))
  {
    printf("\t Wrong Metric Value\n");
  }
  printf("metric = %d\n", rApi.metric);

  /////////////////////////////////////////////////////////////
  // Flags
  /////////////////////////////////////////////////////////////
  rApi.flags = 0;

  /////////////////////////////////////////////////////////////
  // Request Route Delete Function
  /////////////////////////////////////////////////////////////
  routeDeleteIpv4(RIB_ROUTE_TYPE_RIP, rApi);

  return 0;
}


Int32T routeLookupIpv4Test()
{
  char buff[32] ={};
  char nexthopBuff[32] ={};
  struct in_addr nextHop;
  Int32T  isRight =0;

TEST_AGAIN :

  printf("Route Lookup Testing Menu\n");

  printf("\t nexthop = ");
  fgets(buff, 32, stdin);
  sscanf(buff, "%s", nexthopBuff);
 
  /////////////////////////////////////////////////////////////
  // nexthop format
  /////////////////////////////////////////////////////////////
  nextHop.s_addr = inet_addr(nexthopBuff);

  printf("nexthop = %s\n", inet_ntoa(nextHop));

  printf("\n");
  printf("\tinput values are right ? [yes=1/no=0]");
  fgets(buff, 32, stdin);
  sscanf(buff, "%d", &isRight);

  
  /////////////////////////////////////////////////////////////////////////////
  // Send Message to RIB Manager
  /////////////////////////////////////////////////////////////////////////////
  routeLookupIpv4(RIP,  nextHop);
  
  return 0;
}


Int32T
redistributeAddTest()
{
  return 0;
}


Int32T
redistributeDeleteTest()
{
  return 0;
}


Int32T ribUpgradeTest()
{
  char buff[32] ={};
  char buffVer[32] ={};
  char buff2[32] ={};
  struct in_addr nextHop;
  Int32T  isRight =0;

TEST_AGAIN :

  printf("Upgrade Testing Menu\n");

  printf("\t version = ");
  fgets(buff, 32, stdin);
  sscanf(buff, "%s", buffVer);
 
  printf("\n\t version = %s\n", buffVer);

  printf("\n");
  printf("\tinput values are right ? [yes=1/no=0]");
  fgets(buff2, 32, stdin);
  sscanf(buff2, "%d", &isRight);

  if(isRight)
  {
    printf("buffering. \n");
    /* Build nnBufferT */
    nnBufferT apiBuff;
    nnBufferReset (&apiBuff);

    nnBufferSetInt32T (&apiBuff, 0); /* Set cmdFd */
    nnBufferSetInt32T (&apiBuff, 0); /* Set cmdKey */
    nnBufferSetInt32T (&apiBuff, 0); /* Set cmdResult */
    nnBufferSetInt8T  (&apiBuff, strlen(buffVer)); /* Set version string length */
    nnBufferSetString (&apiBuff, buff, strlen(buffVer)); /* Set version string */ 

    nnBufferPrint(&apiBuff);

    /* send ipc message to ribmgr */
    printf("send ipc message. \n");
    ipcSendAsync(RIB_MANAGER,
                 CMD_NOS_DYNAMC_UPGRADE, apiBuff.data, apiBuff.length);
    
  }

  return 0;
}


Int32T cmdUpgradeTest()
{
  char buff[32] ={};
  char buffVer[32] ={};
  char buff2[32] ={};
  struct in_addr nextHop;
  Int32T  isRight =0;

TEST_AGAIN :

  printf("Upgrade Testing Menu\n");

  printf("\t version = ");
  fgets(buff, 32, stdin);
  sscanf(buff, "%s", buffVer);
 
  printf("\n\t version = %s\n", buffVer);

  printf("\n");
  printf("\tinput values are right ? [yes=1/no=0]");
  fgets(buff2, 32, stdin);
  sscanf(buff2, "%d", &isRight);

  if(isRight)
  {
    printf("buffering. \n");
    /* Build nnBufferT */
    nnBufferT apiBuff;
    nnBufferReset (&apiBuff);

    nnBufferSetInt32T (&apiBuff, 0); /* Set cmdFd */
    nnBufferSetInt32T (&apiBuff, 0); /* Set cmdKey */
    nnBufferSetInt32T (&apiBuff, 0); /* Set cmdResult */
    nnBufferSetInt8T  (&apiBuff, strlen(buffVer)); /* Set version string length */
    nnBufferSetString (&apiBuff, buff, strlen(buffVer)); /* Set version string */ 

    nnBufferPrint(&apiBuff);

    /* send ipc message to ribmgr */
    printf("send ipc message. \n");
    ipcSendAsync(COMMAND_MANAGER_NEW,
                 CMD_NOS_DYNAMC_UPGRADE, apiBuff.data, apiBuff.length);
    
  }

  return 0;
}


Int32T
displayMenu()
{
  Int32T select;
  char buff[4096];
  
  fflush(stdin);

  //system("clear");
  printf("Select Testing Menu\n");
  printf("\t 1. Route Add Function\n");
  printf("\t 2. Route Delete Function\n");
  printf("\t 3. Route Lookup Function\n");
  printf("\t 4. Redistribute Add Function\n");
  printf("\t 5. Redistribute Delete Function\n");
  printf("\t 11.Route Multiple Add Function\n");
  printf("\t 12.Route Multiple Delete Function\n");
  printf("\t 20.RIBMgr Upgrade\n");
  printf("\t 21.CMDMgr Upgrade\n");
  printf("\t 100. exit\n");
  printf("\t ==> ");
  fgets(buff, 4096, stdin);
  sscanf(buff, "%d", &select);
  fflush(stdin);

  return select;
}
  
void *
procMenu()
{
  Int32T select;

  while(1)
  {
    select = displayMenu();
    if((select >= 1) && (select <= 100))
    {
      switch(select)
      {
        case 1 :
          routeAddTest();
          break;
  
        case 2 :
          routeDeleteTest();
          break;

        case 3 :
          routeLookupIpv4Test();
          break;

        case 4 :
          redistributeAddTest();
          break;

        case 5 :
          redistributeDeleteTest();
          break;

        case 11 :
          routeAddMultipleTest();
          break;

        case 12 :
          routeDeleteMultipleTest();
          break;

        case 20 :
          ribUpgradeTest();
          break;

        case 21 :
          cmdUpgradeTest();
          break;

        case 100 :
          taskClose();
          nnLogClose();
          memClose();
          exit(0);
          break;

        default :
          break;
      }//switch
    }//if
  }//while
}


void 
initMenuThread()
{
  pthread_t thid;
  if(pthread_create(&thid, NULL, procMenu, NULL))
  {
        perror("packet_recv_thread: ");
        return ; 
  }
}


