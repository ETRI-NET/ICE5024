/**
 * @file    	:  <cmdRip.c>
 * @brief       :  <Define command line and process command line for Rip module>
 * $Id: cmdRip.c 1282 2014-04-25 05:54:40Z sckim007 $			
 * $Author: sckim007 $ 
 * $Date: 2014-04-25 14:54:40 +0900 (Fri, 25 Apr 2014) $
 * $Log$
 * $Revision: 1282 $
 * $LastChangedBy: sckim007 $
 * $LastChanged$
 * 
 *                      Electronics and Telecommunications Research Institute
 * Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.
 *           No part of this software shall be reproduced, stored in a retrieval system, or
 *           transmitted by any means, electronic, mechanical, photocopying, recording,
 *           or otherwise, without written permission from ETRI.
 **/ 

/********************************************************************************
 *                                  INCLUDE FILES 
 * ********************************************************************************/
#include "nnTypes.h"
#include "nnStr.h"

#include "nnCmdLink.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"
#include "nnCmdInstall.h"
#include "nosLib.h"
#include "nnCmdDefines.h"
#include "nnUtility.h"

#include "nnRibDefines.h"

#include "nnBuffer.h"
#include "nnPrefix.h"
#include "nnList.h"
#include "nnKeychain.h"
#include "nnDistribute.h"

#include "ripd.h"
#include "ripPeer.h"
#include "ripInterface.h"
#include "ripUtil.h"
#include "ripRibmgr.h"
#include "ripDebug.h"

#define INTERFACE_NAME_MAX       20
#define RULE_NAME_MAX           128


DENODEC(cmdFuncEnterRip,
        CMD_NODE_CONFIG_RIP, 
        IPC_RIP)
{
  if (pRip && 
      (pRip->isConfigured == RIP_CONFIG_UNSET))
  {
    /* Initialize rip configuration. */
    ripInit ();
  }

  /* Set rip configuration. */
  pRip->isConfigured = RIP_CONFIG_SET;

  return (CMD_IPC_OK);
}


/**< This is CM side callbackfunc	*/
DECMD(cmdFuncRipNoRouterRip,
    CMD_NODE_CONFIG,
    IPC_RIP,
    "no router rip",
    "Negate a command or set its defaults",
    "Disable a routing process",
    "Routing Information Protocol (RIP)")
{
  if (pRip)
  {
    /* Clean rip configuration. */
    ripClean();
  }

  /* Unset rip configuration. */
  pRip->isConfigured = RIP_CONFIG_UNSET;

  return (CMD_IPC_OK);
}


DECMD(cmdFuncRipVersion,
    CMD_NODE_CONFIG_RIP,
    IPC_RIP,
    "version <1-2>",
    "Set routing protocol version", 
    "version") 
{
  Int32T version = 0;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint(cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  /* Get and set version value. */
  version = atoi (cargv[1]); 
  if (version != RIPv1 && version != RIPv2)
  {    
    NNLOG (LOG_DEBUG, "Error. wrong parameter!!!\n");
  }    
  pRip->versionSend = version;
  pRip->versionRecv = version;

  return (CMD_IPC_OK);
}


DECMD(cmdFuncRipNoVersion,
    CMD_NODE_CONFIG_RIP,
    IPC_RIP,
    "no version",
    "Negate a command or set its defaults", 
    "Set routing protocol version") 
{
  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint(cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  /* Set default version values. */
  pRip->versionSend = RI_RIP_VERSION_2;
  pRip->versionRecv = RI_RIP_VERSION_1_AND_2;

  return (CMD_IPC_OK);
}

DECMD(cmdFuncRipDefaultMetric,
    CMD_NODE_CONFIG_RIP,
    IPC_RIP,
    "default-metric <1-16>",
    "Set a metric of redistribute routes", 
    "Default metric") 
{
  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint(cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  /* Set default metric. */
  pRip->defaultMetric = atoi (cargv[1]);

  return (CMD_IPC_OK);
}

DECMD(cmdFuncRipNoDefaultMetric,
    CMD_NODE_CONFIG_RIP,
    IPC_RIP,
    "no default-metric",
    "Negate a command or set its defaults",
    "Set a metric of redistribute routes") 
{
  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint(cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  /* Unset default metric. */
  pRip->defaultMetric = RIP_DEFAULT_METRIC_DEFAULT;

  return (CMD_IPC_OK);
}

DECMD(cmdFuncRipTimersBasic,
    CMD_NODE_CONFIG_RIP,
    IPC_RIP,
    "timers basic <5-2147483647> <5-2147483647> <5-2147483647>",
    "Routing timers",
    "Basic routing protocol update timers",
    "Routing table update timer value in second. Default is 30.",
    "Routing information timeout timer. Default is 180.",
    "Garbage collection timer. Default is 120.") 
{
  Uint32T updateTimer = 0;
  Uint32T timeoutTimer = 0;
  Uint32T garbageTimer = 0;
  char *endptr = NULL;
  Uint32T RIP_TIMER_MAX = 2147483647;
  Uint32T RIP_TIMER_MIN = 5;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint(cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  updateTimer = strtoul (cargv[2], &endptr, 10); 
  if (updateTimer > RIP_TIMER_MAX || updateTimer < RIP_TIMER_MIN || *endptr != '\0')  
  {    
    cmdPrint (cmsh, "Warning. update timer value error\n");
    return (CMD_IPC_OK);
  }    
  
  timeoutTimer = strtoul (cargv[3], &endptr, 10); 
  if (timeoutTimer > RIP_TIMER_MAX || timeoutTimer < RIP_TIMER_MIN || *endptr != '\0') 
  {    
    cmdPrint (cmsh, "Warning. timeout timer value error\n");
    return CMD_IPC_OK;
  }    
  
  garbageTimer = strtoul (cargv[4], &endptr, 10); 
  if (garbageTimer > RIP_TIMER_MAX || garbageTimer < RIP_TIMER_MIN || *endptr != '\0') 
  {    
    cmdPrint (cmsh, "Warning. garbage timer value error\n");
    return CMD_IPC_OK;
  }

  /* Set each timer value. */
  pRip->tvUpdateTime.tv_sec = updateTimer;
  pRip->tvTimeoutTime.tv_sec = timeoutTimer;
  pRip->tvGarbageTime.tv_sec = garbageTimer;

  /* Change timers basic values. */
  ripTimerChange ();
 
  return (CMD_IPC_OK);
}


DECMD(cmdFuncRipNoTimersBasic,
    CMD_NODE_CONFIG_RIP,
    IPC_RIP,
    "no timers basic",
    "Negate a command or set its defaults",
    "Routing timers",
    "Basic routing protocol update timers")
{
  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint(cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  /* Set deault timer values. */
  pRip->tvUpdateTime.tv_sec = RIP_UPDATE_TIMER_DEFAULT;
  pRip->tvTimeoutTime.tv_sec = RIP_TIMEOUT_TIMER_DEFAULT;
  pRip->tvGarbageTime.tv_sec = RIP_GARBAGE_TIMER_DEFAULT;

  return (CMD_IPC_OK);
}


DECMD(cmdFuncRipRoute, 
    CMD_NODE_CONFIG_RIP,
    IPC_RIP,
    "route A.B.C.D/M",
    "RIP static route configuration", 
    "IP prefix <network>/<length>") 
{
  PrefixT prefix = {0,};
  Prefix4T prefix4 = {0,};
  RouteNodeT *pNode = NULL;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint(cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  /* Convert string to prefix. */
  if (nnCnvStringtoPrefix (&prefix, cargv[1]) != SUCCESS)
  {
    cmdPrint(cmsh, "Warning. Wrong prefix format. \n");
    return (CMD_IPC_OK);
  }

   /* Check rip is installed or not. */
  if (!pRip->pStaticRoute)
  {
    cmdPrint(cmsh, "Warning. Not installed rip's static route.\n");
    return (CMD_IPC_OK);
  } 

  /* Allocate route node. */
  pNode = nnRouteNodeGet (pRip->pStaticRoute, (PrefixT *) &prefix);
  if (pNode->pInfo)
  {     
    cmdPrint(cmsh, "Warning. There is already same rip static route.\n");
    nnRouteNodeUnlock (pNode);
    return (CMD_IPC_OK);
  }   
  pNode->pInfo = (char *)"static";
  nnCnvPrefixTtoPrefix4T(&prefix4, &prefix);

  ripRedistributeAdd (RIB_ROUTE_TYPE_RIP, RIP_ROUTE_STATIC, 
                      &prefix4, 0, NULL, 0, 0);  

  return (CMD_IPC_OK);
}

DECMD(cmdFuncRipNoRoute,
    CMD_NODE_CONFIG_RIP,
    IPC_RIP,
    "no route A.B.C.D/M",
    "Negate a command or set its defaults",
    "RIP static route configuration",
    "IP prefix <network>/<length>") 
{
  PrefixT prefix = {0,};
  Prefix4T prefix4 = {0,};
  RouteNodeT *pNode = NULL;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint(cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  /* Convert string to prefix. */
  if (nnCnvStringtoPrefix (&prefix, cargv[2]) != SUCCESS)
  {
    cmdPrint(cmsh, "Warning. Wrong prefix format. \n");
    return (CMD_IPC_OK);
  }

   /* Check rip is installed or not. */
  if (!pRip->pStaticRoute)
  {
    cmdPrint(cmsh, "Not installed rip's static route.\n");
    return (CMD_IPC_OK);
  } 

  pNode = nnRouteNodeLookup (pRip->pStaticRoute, (PrefixT *) &prefix);
  if (!pNode)
  {
    cmdPrint(cmsh, "Warning. Can't find rip static route.\n");
    return (CMD_IPC_OK);
  }
  nnCnvPrefixTtoPrefix4T(&prefix4, &prefix);
  ripRedistributeDelete (RIB_ROUTE_TYPE_RIP, RIP_ROUTE_STATIC, &prefix4, 0);
  nnRouteNodeUnlock (pNode);

  pNode->pInfo = NULL;
  nnRouteNodeUnlock (pNode);

  return (CMD_IPC_OK);
} 

DECMD(cmdFuncRipDistance,
    CMD_NODE_CONFIG_RIP,
    IPC_RIP,
    "distance <1-255>",
    "Administrative distance",
    "Distance value") 
{
  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint(cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  /* distance <1-255> */
  if (cargc == 2)
  {
    pRip->distance = atoi (cargv[1]);
  }
  /* distance <1-255> A.B.C.D/M */
  else if (cargc == 3)
  {
    PrefixT prefix = {0,};
    Int32T ret = 0;

    ret = nnCnvStringtoPrefix (&prefix, cargv[2]);
    if (ret != SUCCESS)
    {
      cmdPrint (cmsh, "Wrong prefix format. \n");
      return (CMD_IPC_OK);
    }
    
    ripDistanceSet (atoi(cargv[1]), &prefix, NULL);
  }
  /* distance <1-255> A.B.C.D/M WORD */
  else if (cargc == 4)
  {
    PrefixT prefix = {0,};
    Int32T ret = 0;

    ret = nnCnvStringtoPrefix (&prefix, cargv[2]);
    if (ret != SUCCESS)
    {
      cmdPrint (cmsh, "Wrong prefix format. \n");
      return (CMD_IPC_OK);
    }
    
    ripDistanceSet (atoi(cargv[1]), &prefix, cargv[3]);
  }
  else
  {
    cmdPrint (cmsh, "Error wrong command. cargc = %d\n", cargc);
    return (CMD_IPC_OK);
  }

  return (CMD_IPC_OK);
}
ALICMD(cmdFuncRipDistance,
    CMD_NODE_CONFIG_RIP,
    IPC_RIP,
    "distance <1-255> A.B.C.D/M",
    "Administrative distance",
    "Distance value",
    "IP source prefix");
ALICMD(cmdFuncRipDistance,
    CMD_NODE_CONFIG_RIP,
    IPC_RIP,
    "distance <1-255> A.B.C.D/M WORD",
    "Administrative distance",
    "Distance value",
    "IP source prefix",
    "Access list name");


DECMD(cmdFuncRipNoDistance,
    CMD_NODE_CONFIG_RIP,
    IPC_RIP,
    "no distance <1-255>",
    "Negate a command or set its defaults",
    "Administrative distance",
    "Distance value") 
{
  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint(cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  /* distance <1-255> */
  if (cargc == 3)
  {
    pRip->distance = 0;
  }
  /* distance <1-255> A.B.C.D/M */
  else if (cargc == 4)
  {
    PrefixT prefix = {0,};
    Int32T ret = 0;

    ret = nnCnvStringtoPrefix (&prefix, cargv[3]);
    if (ret != SUCCESS)
    {
      cmdPrint (cmsh, "Wrong prefix format. \n");
      return (CMD_IPC_OK);
    }
    
    ripDistanceUnset (atoi(cargv[2]), &prefix, NULL);
  }
  /* distance <1-255> A.B.C.D/M WORD */
  else if (cargc == 5)
  {
    PrefixT prefix = {0,};
    Int32T ret = 0;

    ret = nnCnvStringtoPrefix (&prefix, cargv[3]);
    if (ret != SUCCESS)
    {
      cmdPrint (cmsh, "Wrong prefix format. \n");
      return (CMD_IPC_OK);
    }
    
    ripDistanceUnset (atoi(cargv[2]), &prefix, cargv[4]);
  }
  else
  {
    cmdPrint (cmsh, "Error wrong command. cargc = %d\n", cargc);
    return (CMD_IPC_OK);
  }

  return (CMD_IPC_OK);
}
ALICMD(cmdFuncRipNoDistance,
    CMD_NODE_CONFIG_RIP,
    IPC_RIP,
    "no distance <1-255> A.B.C.D/M",
    "Negate a command or set its defaults",
    "Administrative distance",
    "Distance value",
    "IP source prefix");
ALICMD(cmdFuncRipNoDistance,
    CMD_NODE_CONFIG_RIP,
    IPC_RIP,
    "no distance <1-255> A.B.C.D/M WORD",
    "Negate a command or set its defaults",
    "Administrative distance",
    "Distance value",
    "IP source prefix",
    "Access list name");


DECMD(cmdFuncRipNetwork,
    CMD_NODE_CONFIG_RIP,
    IPC_RIP,
    "network (A.B.C.D/M|WORD)",
    "Enable routing on an IP network", 
    "IP prefix <network>/<length> (e.g) 10.0.0.0/8",
    "Interface name (e.g) eth0") 
{
  PrefixT prefix = {0, };
  Int32T ret = 0;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint(cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  /* convert arguments to varibales */
  ret = nnCnvStringtoPrefix (&prefix, cargv[1]);
  /* Ipv4 Address Type */
  if (ret == SUCCESS)
  {
    ret = nnPrefixCheckIpv4Any((Prefix4T *)&prefix);
    if(ret == PREFIX_ERR_NULL)
    {
      cmdPrint (cmsh, "Error] nnPrefixCheckIpv4Any, ret=NULL !!!\n");
      return (CMD_IPC_OK);
    }
    else if(ret == TRUE)
    {
      cmdPrint (cmsh, "Error] nnPrefixCheckIpv4Any, ret=TRUE !!!\n");
      return (CMD_IPC_OK);
    }

    /* Set values */
    ripEnableNetworkAdd(&prefix);
  }
  else
  {
    ripEnableIfAdd(cargv[1]);
  }
  
  return (CMD_IPC_OK);
} 


DECMD(cmdFuncRipNoNetwork,
    CMD_NODE_CONFIG_RIP,
    IPC_RIP,
    "no network (A.B.C.D/M|WORD)",
    "Negate a command or set its defaults",
    "Enable routing on an IP network", 
    "IP prefix <network>/<length> (e.g) 10.0.0.0/8",
    "Interface name (e.g) eth0") 
{
  PrefixT prefix = {0, };
  Int8T ret = 0;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Warning. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint(cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  ret = nnCnvStringtoPrefix (&prefix, cargv[2]);
  /* Ipv4 Address Type */
  if (ret == SUCCESS)
  {
    ret = nnPrefixCheckIpv4Any((Prefix4T *)&prefix);
    if(ret == PREFIX_ERR_NULL)
    {
      cmdPrint (cmsh, "Error] nnPrefixCheckIpv4Any, ret=NULL !!!\n");
      return CMD_IPC_OK;
    }
    else if(ret == TRUE)
    {
      cmdPrint (cmsh,"Error] nnPrefixCheckIpv4Any, ret=TRUE !!!\n");
      return CMD_IPC_OK;
    }

    /* Delete network configuration. */
    ripEnableNetworkDelete(&prefix);
  }
  else
  {
    /* Delete network configuration. */
    ripEnableIfDelete(cargv[2]);
  }


  return CMD_IPC_OK;
} 


DECMD(cmdFuncRipNeighbor,
    CMD_NODE_CONFIG_RIP,
    IPC_RIP,
    "neighbor A.B.C.D",
    "Specify a neighbor router", 
    "Neighbor address (e.g) 10.0.0.1") 
{
  PrefixT prefix = {0,};
  Prefix4T prefix4 = {0,};
  Int32T ret = 0;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint(cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  /* Convert string to prefix format */  
  ret = nnCnvStringtoPrefix (&prefix, cargv[1]);
  if (ret != SUCCESS)
  {
    cmdPrint (cmsh, "Wrong address format.\n");
    return (CMD_IPC_OK);
  }

  /* Convert prefix to prefix4. */
  nnCnvPrefixTtoPrefix4T(&prefix4, &prefix);
 
  /* Add neighbor address. */ 
  ripNeighborAdd (&prefix4);

  return (CMD_IPC_OK);
}


DECMD(cmdFuncRipNoNeighbor,
    CMD_NODE_CONFIG_RIP,
    IPC_RIP,
    "no neighbor A.B.C.D",
    "Negate a command or set its defaults",
    "Specify a neighbor router", 
    "Neighbor address (e.g) 10.0.0.1") 
{
  PrefixT prefix = {0,};
  Prefix4T prefix4 = {0,};
  Int32T ret = 0;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint(cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  /* Convert string to prefix format */  
  ret = nnCnvStringtoPrefix (&prefix, cargv[2]);
  if (ret != SUCCESS)
  {
    cmdPrint (cmsh, "Wrong address format.\n");
    return (CMD_IPC_OK);
  }

  /* Convert prefix to prefix4. */
  nnCnvPrefixTtoPrefix4T(&prefix4, &prefix);

  /* Delete neighbor address. */
  ripNeighborDelete (&prefix4);

  return (CMD_IPC_OK);
} 


DECMD(cmdFuncRipPassiveInterface,
    CMD_NODE_CONFIG_RIP,
    IPC_RIP,
    "passive-interface (WORD|default)",
    "Suppress routing updates on an interface",
    "Interface name (e.g) eth0", 
    "default for all interfaces")
{
  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint(cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  if (!strcmp(cargv[1], "default"))
  {   
    pRip->passiveDefault = 1;
    ripPassiveNonDefaultClean ();
    return (CMD_IPC_OK);
  }     

  if (pRip->passiveDefault)
  {   
    ripPassiveNonDefaultUnset (cargv[1]);
  } 
  else
  {
    ripPassiveNonDefaultSet (cargv[1]);
  }

  return (CMD_IPC_OK);
}


DECMD(cmdFuncRipNoPassiveInterface,
    CMD_NODE_CONFIG_RIP,
    IPC_RIP,
    "no passive-interface (WORD|default)",
    "Negate a command or set its defaults",
    "Suppress routing updates on an interface",
    "Interface name (e.g) eth0", 
    "default for all interfaces")
{
  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint(cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  if (!strcmp(cargv[2], "default"))
  {
    pRip->passiveDefault = 0;
    ripPassiveNonDefaultClean ();

    return (CMD_IPC_OK);
  }

  if (pRip->passiveDefault)
  {
    ripPassiveNonDefaultSet (cargv[2]);
  }
  else
  {
    ripPassiveNonDefaultUnset (cargv[2]);
  }

  return (CMD_IPC_OK);
}


DECMD(cmdFuncRipDefaultInformationOriginate,
      CMD_NODE_CONFIG_RIP,
      IPC_RIP,
      "default-information originate",
      "Control distribution of default route",
      "Distribute a default route")
{
  Prefix4T prefix4 = {0,};

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint(cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  if (cargc == 2)
  {
    if (!pRip->defaultInformation)
    {
      memset (&prefix4, 0, sizeof (Prefix4T));
      prefix4.family = AF_INET;

      pRip->defaultInformation = 1;
  
      ripRedistributeAdd (RIB_ROUTE_TYPE_RIP, RIP_ROUTE_DEFAULT,
                          &prefix4, 0, NULL, 0, 0);
    }
  }
  else if (cargc == 3)
  {
    if (pRip->defaultInformation)              
    {   
      memset (&prefix4, 0, sizeof (Prefix4T));
      prefix4.family = AF_INET;

      pRip->defaultInformation = 0;

      ripRedistributeDelete (RIB_ROUTE_TYPE_RIP, RIP_ROUTE_DEFAULT,
                             &prefix4, 0);
    }
  }
  else
  {
    cmdPrint (cmsh, "Error wrong command arg count.\n");
    return (CMD_IPC_OK);
  }

  return (CMD_IPC_OK);
}
ALICMD(cmdFuncRipDefaultInformationOriginate,
       CMD_NODE_CONFIG_RIP,
       IPC_RIP,
       "no default-information originate",
       "Negate a command or set its defaults",
       "Control distribution of default route",
       "Distribute a default route"); 

/*
 * redistribute command.
 */
DECMD (cmdFuncRipRedistributeType,
       CMD_NODE_CONFIG_RIP,
       IPC_RIP,
       "redistribute (kernel|connected|static|ospf|isis|bgp)",
       "Redistribute information from another routing protocol",
       "Kernel routes (not installed via the zebra RIB)",
       "Connected routes (directly attached subnet or host)",
       "Statically configured routes",
       "Open Shortest Path First (OSPFv2)",
       "Intermediate System to Intermediate System (IS-IS)",
       "Border Gateway Protocol (BGP)")
{
  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint(cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  return ripRibmgrRedistribute(cmsh, IPC_RIB_REDISTRIBUTE_ADD, cargv[1]);
}

DECMD (cmdFuncRipNoRedistributeType,
       CMD_NODE_CONFIG_RIP,
       IPC_RIP,
       "no redistribute (kernel|connected|static|ospf|isis|bgp)",
       "Negate a command or set its defaults"
       "Redistribute information from another routing protocol",
       "Kernel routes (not installed via the zebra RIB)",
       "Connected routes (directly attached subnet or host)",
       "Statically configured routes",
       "Open Shortest Path First (OSPFv2)",
       "Intermediate System to Intermediate System (IS-IS)",
       "Border Gateway Protocol (BGP)")
       
{
  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint(cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  return ripRibmgrRedistribute(cmsh, IPC_RIB_REDISTRIBUTE_DELETE, cargv[2]);
}

DECMD (cmdFuncRipRedistributeTypeMetric,
       CMD_NODE_CONFIG_RIP,
       IPC_RIP,
       "redistribute (kernel|connected|static|ospf|isis|bgp) metric <0-16>",
       "Redistribute information from another routing protocol",
       "Kernel routes (not installed via the zebra RIB)",
       "Connected routes (directly attached subnet or host)",
       "Statically configured routes",
       "Open Shortest Path First (OSPFv2)",
       "Intermediate System to Intermediate System (IS-IS)",
       "Border Gateway Protocol (BGP)",
       "Metric",
       "Metric value")
{
  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint(cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  /* Set metric value. */
  ripRedistributeMetricSet (cargv[1], atoi(cargv[3]));

  /* Request redistribute add to ribmgr. */
  return ripRibmgrRedistribute(cmsh, IPC_RIB_REDISTRIBUTE_ADD, cargv[1]);
}

DECMD (cmdFuncRipNoRedistributeTypeMetric,
       CMD_NODE_CONFIG_RIP,
       IPC_RIP,
       "no redistribute (kernel|connected|static|ospf|isis|bgp) metric <0-16>",
       "Negate a command or set its defaults"
       "Redistribute information from another routing protocol",
       "Kernel routes (not installed via the zebra RIB)",
       "Connected routes (directly attached subnet or host)",
       "Statically configured routes",
       "Open Shortest Path First (OSPFv2)",
       "Intermediate System to Intermediate System (IS-IS)",
       "Border Gateway Protocol (BGP)",
       "Metric",
       "Metric value")
       
{
  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint(cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  /* Set metric value. */
  if (ripRedistributeMetricUnSet (cargv[2], atoi(cargv[4])) != SUCCESS)
  {
    printf ("Wrong metric value = %d\n", atoi(cargv[4]));
    cmdPrint (cmsh, "Wrong metric value = %d\n", atoi(cargv[4]));
    return (CMD_IPC_OK);
  }
  
  /* Request redistribute delete to ribmgr. */
  return ripRibmgrRedistribute(cmsh, IPC_RIB_REDISTRIBUTE_DELETE, cargv[2]);
}


/*
 * rip show command.
 */
DECMD(cmdFuncRipShowIpRip,
    CMD_NODE_EXEC,
    IPC_RIP|IPC_SHOW_MGR,
    "show ip rip",
    "show",
    "ip",
    "Routing Information Protocol (RIP)")
{
  RouteNodeT *pRNode = NULL;
  RipInfoT *pRInfo = NULL;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint (cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  cmdPrint (cmsh, "Codes: R - RIP, C - connected, S - Static, O - OSPF, B - BGP\n"
           "Sub-codes:\n"
           "      (n) - normal, (s) - static, (d) - default, (r) - redistribute,\n"
           "      (i) - interface\n"
           "     Network            Next Hop         Metric From            Tag Time\n");

  for (pRNode = nnRouteTop (pRip->pRipRibTable); 
       pRNode; 
       pRNode = nnRouteNext (pRNode))
    if ((pRInfo = pRNode->pInfo) != NULL)
    {
      Int32T len = 0;
      Int32T idx = 0;
      Int8T showBuff[RIP_BUFFER_MAX_SIZE] = {};

      len = sprintf (showBuff, "%c(%s) %s/%d",
                      nnRoute2Char(pRInfo->type),
                      ripRouteTypePrint (pRInfo->subType),
                      inet_ntoa (pRNode->p.u.prefix4), pRNode->p.prefixLen);
      idx = len;

      len = 24 - len;
      if (len > 0)
      {
        idx += sprintf (showBuff + idx, "%*s", len, " ");
      }
      

      if (pRInfo->nexthop.s_addr)
      {
        idx += sprintf (showBuff + idx, "%-20s %2d ", 
                        inet_ntoa (pRInfo->nexthop), pRInfo->metric);
      }
      else
      {
        idx += sprintf (showBuff + idx, "0.0.0.0              %2d ", 
                        pRInfo->metric);
      }

      /* Route which exist in kernel routing table. */
      if ((pRInfo->type == RIB_ROUTE_TYPE_RIP) &&
           (pRInfo->subType == RIP_ROUTE_RTE))
      {
        idx += sprintf (showBuff + idx, "%-15s ", inet_ntoa (pRInfo->from));
        idx += sprintf (showBuff + idx, "%3d ", pRInfo->tag);
        idx += ripCmshUptime (showBuff, idx, pRInfo);
      }
      else if (pRInfo->metric == RIP_METRIC_INFINITY)
      {
        idx += sprintf (showBuff + idx, "self            ");
        idx += sprintf (showBuff + idx, "%3d ", pRInfo->tag);
        idx += ripCmshUptime (showBuff, idx, pRInfo);
      }
      else
      {
        if (pRInfo->externalMetric)
        {
          memset (showBuff, 0, RIP_BUFFER_MAX_SIZE);
          len = sprintf (showBuff + idx, "self (%s:%d)",
                         (StringT)nnRoute2String(pRInfo->type),
                         pRInfo->externalMetric);
          idx += len;
          len = 16 - len;
          if (len > 0)
          {
            idx += sprintf (showBuff + idx, "%*s", len, " ");
          }
        }
        else
        {
          idx += sprintf (showBuff + idx, "self            ");
        }

       idx += sprintf (showBuff + idx, "%3d", pRInfo->tag);
      }
      cmdPrint (cmsh, "%s", showBuff); 
    }

  return (CMD_IPC_OK);
} 


DECMD(cmdFuncRipShowIpRipStatus,
    CMD_NODE_EXEC,
    IPC_RIP|IPC_SHOW_MGR,
    "show ip rip status",
    "show",
    "ip",
    "Routing Information Protocol (RIP)",
    "status")
{
  InterfaceT *pIf = NULL;
  RipInterfaceT *pRIf = NULL;
  extern const struct message ri_version_msg[];
  const char *SendVersion;
  const char *receiveVersion;
  char strBuff[RIP_BUFFER_MAX_SIZE] = {};
  Int32T idx = 0;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Error. Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check no router rip status. */
  if (pRip->isConfigured == RIP_CONFIG_UNSET)
  {
    cmdPrint (cmsh, "Warning. no router rip status.\n");
    return (CMD_IPC_OK);
  }

  /* Display show. */
  cmdPrint (cmsh, "Routing Protocol is \"rip\"\n");
  idx = sprintf (strBuff, 
                 "  Sending updates every %d seconds with +/-50%%,",
                 (Int32T)pRip->tvUpdateTime.tv_sec);
  sprintf (strBuff + idx, " next due in %d seconds\n", 
           ripCmshUpdateTimerRemain());
  cmdPrint (cmsh, "%s", strBuff);
  
  memset (strBuff, 0, sizeof (strBuff));
  idx = sprintf (strBuff, "  Timeout after %d seconds,", 
                 (Int32T)pRip->tvTimeoutTime.tv_sec);
  sprintf (strBuff + idx, " garbage collect after %d seconds\n", 
           (Int32T)pRip->tvGarbageTime.tv_sec);
  cmdPrint (cmsh, "%s", strBuff);

  /* Filtering status show. */
  configShowDistribute (cmsh);

  /* Default metric information. */
  cmdPrint (cmsh, 
            "  Default redistribution metric is %d\n", 
            pRip->defaultMetric);

  /* Redistribute information. */
  cmdPrint (cmsh, "  Redistributing:");
  configWriteRipRedistribute (cmsh, 0);

  memset (strBuff, 0, sizeof (strBuff));
  idx = sprintf (strBuff, 
                 "  Default version control: send version %s,",
                 lookupStr (ri_version_msg, pRip->versionSend));
  if (pRip->versionRecv == RI_RIP_VERSION_1_AND_2)
  {
    sprintf (strBuff + idx, " receive any version \n");
  }
  else
  {
    sprintf (strBuff + idx, " receive version %s \n",
              lookupStr (ri_version_msg, pRip->versionRecv));
  }
  cmdPrint (cmsh, "%s", strBuff);

  cmdPrint (cmsh, "    Interface        Send  Recv   Key-chain\n");

  ListNodeT * pNode = NULL;
  for(pNode = pRip->pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
  {
    pIf = pNode->pData;
    pRIf = pIf->pInfo;

    if (!pRIf->running)
      continue;

    if (pRIf->enableNetwork || pRIf->enableInterface)
    {
      if (pRIf->riSend == RI_RIP_UNSPEC)
        SendVersion = lookupStr (ri_version_msg, pRip->versionSend);
      else
        SendVersion = lookupStr (ri_version_msg, pRIf->riSend);

      if (pRIf->riReceive == RI_RIP_UNSPEC)
        receiveVersion = lookupStr (ri_version_msg, pRip->versionRecv);
      else
        receiveVersion = lookupStr (ri_version_msg, pRIf->riReceive);

      cmdPrint (cmsh, "    %-17s%-3s   %-3s    %s\n", pIf->name,
                SendVersion, receiveVersion,
                pRIf->keyChain ? pRIf->keyChain : "");
    }
  }

  cmdPrint (cmsh, "  Routing for Networks:\n");
  ripNetworkDisplay (cmsh);
  {
    int found_passive = 0;
    pNode = NULL;
    for(pNode = pRip->pIfList->pHead;pNode != NULL;pNode = pNode->pNext)
    {
      pIf = pNode->pData;
      pRIf = pIf->pInfo;

      if ((pRIf->enableNetwork || pRIf->enableInterface) && pRIf->passive)
      {
        if (!found_passive)
        {
          cmdPrint (cmsh, "  Passive Interface(s):\n");
          found_passive = 1;
        }
        cmdPrint (cmsh, "    %s\n", pIf->name);
      }
    }
  }

  cmdPrint (cmsh, "  Routing Information Sources:\n");
  cmdPrint (cmsh, "    Gateway          BadPackets BadRoutes  Distance Last Update\n");
  ripPeerDisplay (cmsh);

  ripDistanceDisplay (cmsh);

  return (CMD_IPC_OK);
} 


DECMD(cmdFuncRipIpRipReceiveVersion,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "ip rip receive version (1|2)",
    "ip",
    "Routing Information Protocol (RIP)",
    "Advertisement reception",
    "Version control",
    "Version 1",
    "Version 2")
{
#if 0
  Int32T i;
  for(i = 0; i < uargc1; i++) 
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++) 
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }

  for(i = 0; i < cargc; i++) 
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  InterfaceT * pIf = NULL;
  RipInterfaceT * pRIf = NULL;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check interface name is null or not. */
  if (uargv1[1] == NULL)
  {
    cmdPrint (cmsh, "Interface name is Null.\n");
    return (CMD_IPC_OK);
  }

  /* Lookup interface pointer. */
  pIf = ifGetByNameLen (uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));

  /* Check pointer. */
  assert (pIf);
  assert (pIf->pInfo);

  /* Assign RipInterface pointer. */
  pRIf = pIf->pInfo;

  /* Set configuration. */
  if (atoi(cargv[4]) == 1)
  {
    pRIf->riReceive =  RI_RIP_VERSION_1;
  }
  else if (atoi(cargv[4]) == 2)
  {
    pRIf->riReceive =  RI_RIP_VERSION_2;
  }
  else
  {
    cmdPrint (cmsh, "Error wrong version[%d]\n", atoi(cargv[4]));
    return (CMD_IPC_OK);
  }

  return (CMD_IPC_OK);
}

DECMD(cmdFuncRipIpRipReceiveVersion12,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "ip rip receive version 1 2",
    "ip",
    "Routing Information Protocol (RIP)",
    "Advertisement reception",
    "Version control",
    "Version 1",
    "Version 2")
{
#if 0
  Int32T i;
  for(i = 0; i < uargc1; i++) 
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++) 
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }

  for(i = 0; i < cargc; i++) 
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  InterfaceT * pIf = NULL;
  RipInterfaceT * pRIf = NULL;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check interface name is null or not. */
  if (uargv1[1] == NULL)
  {
    cmdPrint (cmsh, "Interface name is Null.\n");
    return (CMD_IPC_OK);
  }

  /* Lookup interface pointer. */
  pIf = ifGetByNameLen (uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));

  /* Check pointer. */
  assert (pIf);
  assert (pIf->pInfo);

  /* Assign RipInterface pointer. */
  pRIf = pIf->pInfo;

  /* Set configuration. */
  pRIf->riReceive = RI_RIP_VERSION_1_AND_2;

  return (CMD_IPC_OK);
}
ALICMD(cmdFuncRipIpRipReceiveVersion12,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "ip rip receive version 2 1",
    "ip",
    "Routing Information Protocol (RIP)",
    "Advertisement reception",
    "Version control",
    "Version 2",
    "Version 1");


DECMD(cmdFuncRipNoIpRipReceiveVersion,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "no ip rip receive version",
    "Negate a command or set its defaults",
    "ip",
    "Routing Information Protocol (RIP)",
    "Advertisement reception",
    "Version control")
{
  InterfaceT * pIf = NULL;
  RipInterfaceT * pRIf = NULL;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check interface name is null or not. */
  if (uargv1[1] == NULL)
  {
    cmdPrint (cmsh, "Interface name is Null.\n");
    return (CMD_IPC_OK);
  }

  /* Lookup interface pointer. */
  pIf = ifGetByNameLen (uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));

  /* Check pointer. */
  assert (pIf);
  assert (pIf->pInfo);

  /* Assign RipInterface pointer. */
  pRIf = pIf->pInfo;

  /* Set configuration. */
  pRIf->riReceive = RI_RIP_UNSPEC;
  
  return (CMD_IPC_OK);
}
ALICMD(cmdFuncRipNoIpRipReceiveVersion,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "no ip rip receive version (1|2)",
    "Negate a command or set its defaults",
    "ip",
    "Routing Information Protocol (RIP)",
    "Advertisement reception",
    "Version control",
    "Version 1",
    "Version 2");


DECMD(cmdFuncRipIpRipSendVersion,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "ip rip send version (1|2)",
    "ip",
    "Routing Information Protocol (RIP)",
    "Advertisement transmission",
    "Version control",
    "Version 1",
    "Version 2") 
{
  InterfaceT * pIf = NULL;
  RipInterfaceT * pRIf = NULL;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check interface name is null or not. */
  if (uargv1[1] == NULL)
  {
    cmdPrint (cmsh, "Interface name is Null.\n");
    return (CMD_IPC_OK);
  }

  /* Lookup interface pointer. */
  pIf = ifGetByNameLen (uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));

  /* Check pointer. */
  assert (pIf);
  assert (pIf->pInfo);

  /* Assign RipInterface pointer. */
  pRIf = pIf->pInfo;

  /* Set configuration. */
  if (atoi(cargv[4]) == 1)
  {
    pRIf->riSend =  RI_RIP_VERSION_1;
  }
  else if (atoi(cargv[4]) == 2)
  {
    pRIf->riSend =  RI_RIP_VERSION_2;
  }
  else
  {
    cmdPrint (cmsh, "Error wrong version[%d]\n", atoi(cargv[4]));
    return (CMD_IPC_OK);
  }

  return (CMD_IPC_OK);
} 


DECMD(cmdFuncRipIpRipSendVersion12,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "ip rip send version 1 2",
    "ip",
    "Routing Information Protocol (RIP)",
    "Advertisement transmission",
    "Version control",
    "Version 1",
    "Version 2")
{
  InterfaceT * pIf = NULL;
  RipInterfaceT * pRIf = NULL;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check interface name is null or not. */
  if (uargv1[1] == NULL)
  {
    cmdPrint (cmsh, "Interface name is Null.\n");
    return (CMD_IPC_OK);
  }

  /* Lookup interface pointer. */
  pIf = ifGetByNameLen (uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));

  /* Check pointer. */
  assert (pIf);
  assert (pIf->pInfo);

  /* Assign RipInterface pointer. */
  pRIf = pIf->pInfo;

  /* Set configuration. */
  pRIf->riSend = RI_RIP_VERSION_1_AND_2;

  return (CMD_IPC_OK);
}
ALICMD(cmdFuncRipIpRipSendVersion12,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "ip rip send version 2 1",
    "ip",
    "Routing Information Protocol (RIP)",
    "Advertisement transmission",
    "Version control",
    "Version 2",
    "Version 1");


DECMD(cmdFuncRipNoIpRipSendVersion,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "no ip rip send version",
    "Negate a command or set its defaults",
    "ip",
    "Routing Information Protocol (RIP)",
    "Advertisement transmission",
    "Version control")
{
  InterfaceT * pIf = NULL;
  RipInterfaceT * pRIf = NULL;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check interface name is null or not. */
  if (uargv1[1] == NULL)
  {
    cmdPrint (cmsh, "Interface name is Null.\n");
    return (CMD_IPC_OK);
  }

  /* Lookup interface pointer. */
  pIf = ifGetByNameLen (uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));

  /* Check pointer. */
  assert (pIf);
  assert (pIf->pInfo);

  /* Assign RipInterface pointer. */
  pRIf = pIf->pInfo;

  /* Set configuration. */
  pRIf->riSend = RI_RIP_UNSPEC;

  return (CMD_IPC_OK);
}
ALICMD(cmdFuncRipNoIpRipSendVersion,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "no ip rip send version (1|2)",
    "Negate a command or set its defaults",
    "ip",
    "Routing Information Protocol (RIP)",
    "Advertisement transmission",
    "Version control",
    "Version 1",
    "Version 2"); 


DECMD(cmdFuncRipIpRipAuthenticationMode,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "ip rip authentication mode (md5|text)",
    "ip",
    "Routing Information Protocol (RIP)",
    "Authentication control",
    "Authentication mode",
    "Keyed message digest",
    "Clear text authentication")
{
  InterfaceT * pIf = NULL;
  RipInterfaceT * pRIf = NULL;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check interface name is null or not. */
  if (uargv1[1] == NULL)
  {
    cmdPrint (cmsh, "Interface name is Null.\n");
    return (CMD_IPC_OK);
  }

  /* Lookup interface pointer. */
  pIf = ifGetByNameLen (uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));

  /* Check pointer. */
  assert (pIf);
  assert (pIf->pInfo);

  /* Assign RipInterface pointer. */
  pRIf = pIf->pInfo;

  /* Set configuration. */
  if (strncmp ("md5", cargv[4], strlen(cargv[4])) == 0)
  {
    pRIf->authType = RIP_AUTH_MD5;
  }
  else if (strncmp ("text", cargv[4], strlen(cargv[4])) == 0)
  {
    pRIf->authType = RIP_AUTH_SIMPLE_PASSWORD;
  }
  else
  {
    cmdPrint (cmsh, "Error mode should be md5 or text\n");
    return (CMD_IPC_OK);
  }

  return (CMD_IPC_OK);
}

DECMD(cmdFuncRipNoIpRipAuthenticationMode,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "no ip rip authentication mode",
    "Negate a command or set its defaults",
    "ip",
    "Routing Information Protocol (RIP)",
    "Authentication control",
    "Authentication mode")
{
  InterfaceT * pIf = NULL;
  RipInterfaceT * pRIf = NULL;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check interface name is null or not. */
  if (uargv1[1] == NULL)
  {
    cmdPrint (cmsh, "Interface name is Null.\n");
    return (CMD_IPC_OK);
  }

  /* Lookup interface pointer. */
  pIf = ifGetByNameLen (uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));

  /* Check pointer. */
  assert (pIf);
  assert (pIf->pInfo);

  /* Assign RipInterface pointer. */
  pRIf = pIf->pInfo;

  /* Set configuration. */
  pRIf->authType = RIP_NO_AUTH;
  pRIf->md5AuthLen = RIP_AUTH_MD5_COMPAT_SIZE;

  return (CMD_IPC_OK);
}
ALICMD(cmdFuncRipNoIpRipAuthenticationMode,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "no ip rip authentication mode (md5|text)",
    "Negate a command or set its defaults",
    "ip",
    "Routing Information Protocol (RIP)",
    "Authentication control",
    "Authentication mode",
    "Keyed message digest",
    "Clear text authentication");


DECMD(cmdFuncRipIpRipAuthenticationString,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "ip rip authentication string WORD",
    "ip",
    "Routing Information Protocol (RIP)",
    "Authentication control",
    "Authentication string",
    "Authentication string")
{
  InterfaceT * pIf = NULL;
  RipInterfaceT * pRIf = NULL;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check interface name is null or not. */
  if (uargv1[1] == NULL)
  {
    cmdPrint (cmsh, "Interface name is Null.\n");
    return (CMD_IPC_OK);
  }

  /* Check string length. */
  if (strlen(cargv[4]) > 16)
  {
    cmdPrint(cmsh, "RIPv2 authentication string must be shorter than 16\n");
    return CMD_IPC_OK;
  }

  /* Lookup interface pointer. */
  pIf = ifGetByNameLen (uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));

  /* Check pointer. */
  assert (pIf);
  assert (pIf->pInfo);

  /* Assign RipInterface pointer. */
  pRIf = pIf->pInfo;

  /* Set configuration. */
  if (pRIf->keyChain)
  {
    cmdPrint(cmsh, "Warning. key-chain configuration exists.\n");
    return (CMD_IPC_OK);
  }

  if (pRIf->authStr)
  {
    NNFREE (MEM_AUTHENTICATION_NAME, pRIf->authStr);
  }

  pRIf->authStr = nnStrDup (cargv[4], MEM_AUTHENTICATION_NAME);

  return (CMD_IPC_OK);
}


DECMD(cmdFuncRipNoIpRipAuthenticationString,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "no ip rip authentication string",
    "Negate a command or set its defaults",
    "ip",
    "Routing Information Protocol (RIP)",
    "authentication control",
    "authentication string") 
{
  InterfaceT * pIf = NULL;
  RipInterfaceT * pRIf = NULL;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check interface name is null or not. */
  if (uargv1[1] == NULL)
  {
    cmdPrint (cmsh, "Interface name is Null.\n");
    return (CMD_IPC_OK);
  }

  /* Lookup interface pointer. */
  pIf = ifGetByNameLen (uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));

  /* Check pointer. */
  assert (pIf);
  assert (pIf->pInfo);

  /* Assign RipInterface pointer. */
  pRIf = pIf->pInfo;

  /* Set configuration. */
  if (pRIf->authStr)
  {
    NNFREE (MEM_AUTHENTICATION_NAME, pRIf->authStr);
  }

  pRIf->authStr = NULL;
 
  return (CMD_IPC_OK);
}
ALICMD(cmdFuncRipNoIpRipAuthenticationString,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "no ip rip authentication string WORD",
    "Negate a command or set its defaults",
    "ip",
    "Routing Information Protocol (RIP)",
    "Authentication control",
    "Authentication string",
    "Keyed message string");




DECMD(cmdFuncRipIpRipAuthenticationKeyChain,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "ip rip authentication key-chain WORD",
    "ip",
    "Routing Information Protocol (RIP)",
    "Authentication control",
    "Authentication key-chain",
    "name of key-chain")
{
  InterfaceT * pIf = NULL;
  RipInterfaceT * pRIf = NULL;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check interface name is null or not. */
  if (uargv1[1] == NULL)
  {
    cmdPrint (cmsh, "Interface name is Null.\n");
    return (CMD_IPC_OK);
  }

  /* Lookup interface pointer. */
  pIf = ifGetByNameLen (uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));

  /* Check pointer. */
  assert (pIf);
  assert (pIf->pInfo);

  /* Assign RipInterface pointer. */
  pRIf = pIf->pInfo;

  /* Set configuration. */
  if (pRIf->authStr)
  {
    cmdPrint (cmsh, "Warning. authentication string configuration exists\n");
    return (CMD_IPC_OK);
  }

  if (pRIf->keyChain)
  {
    NNFREE (MEM_KEYCHAIN, pRIf->keyChain);
  }

  pRIf->keyChain = nnStrDup (cargv[4], MEM_KEYCHAIN_NAME);

  return (CMD_IPC_OK);
}

DECMD(cmdFuncRipNoIpRipAuthenticationKeyChain,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "no ip rip authentication key-chain",
    "Negate a command or set its defaults",
    "ip",
    "Routing Information Protocol (RIP)",
    "authentication control",
    "authentication key-chain")
{
  InterfaceT * pIf = NULL;
  RipInterfaceT * pRIf = NULL;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check interface name is null or not. */
  if (uargv1[1] == NULL)
  {
    cmdPrint (cmsh, "Interface name is Null.\n");
    return (CMD_IPC_OK);
  }

  /* Lookup interface pointer. */
  pIf = ifGetByNameLen (uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));

  /* Check pointer. */
  assert (pIf);
  assert (pIf->pInfo);

  /* Assign RipInterface pointer. */
  pRIf = pIf->pInfo;

  /* Set configuration. */
  if (pRIf->keyChain)
  {
    NNFREE (MEM_KEYCHAIN, pRIf->keyChain);
  }

  pRIf->keyChain = NULL;

  return (CMD_IPC_OK);
}
ALICMD(cmdFuncRipNoIpRipAuthenticationKeyChain,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "no ip rip authentication key-chain WORD",
    "Negate a command or set its defaults",
    "ip",
    "Routing Information Protocol (RIP)",
    "authentication control",
    "authentication key-chain",
    "authentication key-chain");


DECMD(cmdFuncRipIpRipSplitHorizon,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "ip rip split-horizon",
    "ip",
    "Routing Information Protocol (RIP)",
    "Perform split-horizon")
{
  InterfaceT * pIf = NULL;
  RipInterfaceT * pRIf = NULL;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check interface name is null or not. */
  if (uargv1[1] == NULL)
  {
    cmdPrint (cmsh, "Interface name is Null.\n");
    return (CMD_IPC_OK);
  }

  /* Lookup interface pointer. */
  pIf = ifGetByNameLen (uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));

  /* Check pointer. */
  assert (pIf);
  assert (pIf->pInfo);

  /* Assign RipInterface pointer. */
  pRIf = pIf->pInfo;

  /* Set configuration. */
  if (cargc == 3)
  {
    pRIf->splitHorizon = RIP_SPLIT_HORIZON;
  }
  else if (cargc == 4)
  {
    pRIf->splitHorizon = RIP_SPLIT_HORIZON_POISONED_REVERSE;
  }
  else
  {
    cmdPrint (cmsh, "Error wrong command argc count.\n");
    return (CMD_IPC_OK);
  }

  return (CMD_IPC_OK);
}
ALICMD(cmdFuncRipIpRipSplitHorizon,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "ip rip split-horizon poisoned-reverse",
    "ip",
    "Routing Information Protocol (RIP)",
    "Perform split-horizon",
    "With poisoned-reverse");

DECMD(cmdFuncRipNoIpRipSplitHorizon,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "no ip rip split-horizon",
    "Negate a command or set its defaults",
    "ip",
    "Routing Information Protocol (RIP)",
    "Perform split-horizon")
{
  InterfaceT * pIf = NULL;
  RipInterfaceT * pRIf = NULL;

  /* Check rip is installed or not. */
  if (!pRip)
  {
    cmdPrint(cmsh, "Not installed rip.\n");
    return (CMD_IPC_OK);
  }

  /* Check interface name is null or not. */
  if (uargv1[1] == NULL)
  {
    cmdPrint (cmsh, "Interface name is Null.\n");
    return (CMD_IPC_OK);
  }

  /* Lookup interface pointer. */
  pIf = ifGetByNameLen (uargv1[1], strnlen(uargv1[1], INTERFACE_NAMSIZ));

  /* Check pointer. */
  assert (pIf);
  assert (pIf->pInfo);

  /* Assign RipInterface pointer. */
  pRIf = pIf->pInfo;

  /* Set configuration. */
  if (cargc == 4)
  {
    pRIf->splitHorizon = RIP_NO_SPLIT_HORIZON;
  }
  else if (cargc == 5)
  {
    pRIf->splitHorizon = RIP_SPLIT_HORIZON;
  }
  else
  {
    cmdPrint (cmsh, "Error wrong command argc count.\n");
    return (CMD_IPC_OK);
  }

  return (CMD_IPC_OK);
}
ALICMD(cmdFuncRipNoIpRipSplitHorizon,
    CMD_NODE_INTERFACE,
    IPC_RIP,
    "no ip rip split-horizon poisoned-reverse",
    "Negate a command or set its defaults",
    "ip",
    "Routing Information Protocol (RIP)",
    "Perform split-horizon",
    "With poison-reverse"); 



DECMD (cmdFuncRipDebugEvents,
       CMD_NODE_CONFIG,
       IPC_RIP,
       "debug rip events",
       "Debugging functions",
       "Routing Information Protocol (RIP)",
       "RIP events")
{
  pRip->ripDebugEvent = RIP_DEBUG_EVENT;

  return CMD_IPC_OK;
}

DECMD (cmdFuncNoRipDebugEvents,
       CMD_NODE_CONFIG,
       IPC_RIP,
       "no debug rip events",
       "Negate a command or set its defaults",
       "Debugging functions",
       "Routing Information Protocol (RIP)",
       "RIP events")
{
  pRip->ripDebugEvent = 0;

  return CMD_IPC_OK;
}


DECMD (cmdFuncRipDebugPacket,
       CMD_NODE_CONFIG,
       IPC_RIP,
       "debug rip packet",
       "Debugging functions",
       "Routing Information Protocol (RIP)",
       "RIP packet")
{
  pRip->ripDebugPacket = RIP_DEBUG_PACKET;

  return CMD_IPC_OK;
}

DECMD (cmdFuncNoRipDebugPacket,
       CMD_NODE_CONFIG,
       IPC_RIP,
       "no debug rip packet",
       "Negate a command or set its defaults",
       "Debugging functions",
       "Routing Information Protocol (RIP)",
       "RIP packet")
{
  pRip->ripDebugPacket = 0;

  return CMD_IPC_OK;
}


DECMD (cmdFuncRipDebugPacketDirection,
       CMD_NODE_CONFIG,
       IPC_RIP,
       "debug rip packet (recv|send)",
       "Debugging functions",
       "Routing Information Protocol (RIP)",
       "RIP receive packet",
       "RIP send packet")
{
  pRip->ripDebugPacket |= RIP_DEBUG_PACKET;

  if (strncmp ("send", cargv[3], strlen (cargv[3])) == 0)
  {
    pRip->ripDebugPacket |= RIP_DEBUG_SEND;
    if (IS_RIP_DEBUG_SEND_DETAIL)
    {
      pRip->ripDebugPacket &= ~RIP_DEBUG_SEND_DETAIL;
    }
  }

  if (strncmp ("recv", cargv[3], strlen (cargv[3])) == 0)
  {
    pRip->ripDebugPacket |= RIP_DEBUG_RECV;
    if (IS_RIP_DEBUG_RECV_DETAIL)
    {
      pRip->ripDebugPacket &= ~RIP_DEBUG_RECV_DETAIL;
    }
  }

  return CMD_IPC_OK;
}

DECMD (cmdFuncNoRipDebugPacketDirection,
       CMD_NODE_CONFIG,
       IPC_RIP,
       "no debug rip packet (recv|send)",
       "Negate a command or set its defaults",
       "Debugging functions",
       "Routing Information Protocol (RIP)",
       "RIP receive packet",
       "RIP send packet")
{
  if (strncmp ("send", cargv[4], strlen (cargv[4])) == 0)
  {
    if (IS_RIP_DEBUG_RECV)
    {
      pRip->ripDebugPacket &= ~RIP_DEBUG_SEND;
      pRip->ripDebugPacket &= ~RIP_DEBUG_SEND_DETAIL;
    }
    else
      pRip->ripDebugPacket = 0;
  }

  if (strncmp ("recv", cargv[4], strlen (cargv[4])) == 0)
  {
    if (IS_RIP_DEBUG_SEND)
    {
      pRip->ripDebugPacket &= ~RIP_DEBUG_RECV;
      pRip->ripDebugPacket &= ~RIP_DEBUG_RECV_DETAIL;
    }
    else
      pRip->ripDebugPacket = 0;
  }

  return CMD_IPC_OK;
}


DECMD (cmdFuncRipDebugPacketDirectionDetail,
       CMD_NODE_CONFIG,
       IPC_RIP,
       "debug rip packet (recv|send) detail",
       "Debugging functions",
       "Routing Information Protocol (RIP)",
       "RIP receive packet",
       "RIP send packet",
       "Detailed information display")
{
  pRip->ripDebugPacket |= RIP_DEBUG_PACKET;
  if (strncmp ("send", cargv[3], strlen (cargv[3])) == 0)
  {
    pRip->ripDebugPacket |= RIP_DEBUG_SEND;
    pRip->ripDebugPacket |= RIP_DEBUG_SEND_DETAIL;
  }

  if (strncmp ("recv", cargv[3], strlen (cargv[3])) == 0)
  {
    pRip->ripDebugPacket |= RIP_DEBUG_RECV;
    pRip->ripDebugPacket |= RIP_DEBUG_RECV_DETAIL;
  }

  return CMD_IPC_OK;
}



DECMD (cmdFuncRipDebugRibmgr,
       CMD_NODE_CONFIG,
       IPC_RIP,
       "debug rip ribmgr",
       "Debugging functions",
       "Routing Information Protocol (RIP)",
       "RIP and RIBMGR communication")
{
  pRip->ripDebugRibmgr = RIP_DEBUG_RIBMGR;

  return CMD_IPC_OK;
}


DECMD (cmdFuncNoRipDebugRibmgr,
       CMD_NODE_CONFIG,
       IPC_RIP,
       "no debug rip ribmgr",
       "Negate a command or set its defaults",
       "Debugging functions",
       "Routing Information Protocol (RIP)",
       "RIP and RIBMGR communication")
{
  pRip->ripDebugRibmgr = 0;

  return CMD_IPC_OK;
}


/*
 * Below codes should move to polmgr.
 */
DENODEC(cmdFuncEnterKeychain,
        CMD_NODE_CONFIG_KEYCHAIN, 
        IPC_RIP)
{
#if 0
  Int32T i;
  cmdPrint (cmsh, "uargc1[%d]\n", uargc1);
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint (cmsh, "uargv1[%d] = [%s]\n", i, uargv1[i]);
  }
  cmdPrint (cmsh, "uargc2[%d]\n", uargc2);
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint (cmsh, "uargv2[%d] = [%s]\n", i, uargv2[i]);
  }
  for(i = 0; i < cargc; i++)
  {
    cmdPrint (cmsh, "cargv[%d] = [%s]\n", i, cargv[i]);
  }
#endif

  /* Get or create key chain node. */
  KeychainT * pKeychain = NULL;
  pKeychain = keychainGet (cargv[2]);
  /* Check key chain node. */
  if (!pKeychain)
  {
    cmsh->outputStr = strdup("Error] not exist or cannot create keychain-node\n");
    return (CMD_IPC_OK);
  }

  return (CMD_IPC_OK);
}


DECMD (cmdFuncNoKeychain,
       CMD_NODE_CONFIG,
       IPC_RIP,
       "no key chain WORD",
       "Negate a command or set its defaults",
       "Authentication key management",
       "Key-chain management",
       "Key-chain name")
{
#if 0
  Int32T i;
  cmdPrint (cmsh, "uargc1[%d]\n", uargc1);
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint (cmsh, "uargv1[%d] = [%s]\n", i, uargv1[i]);
  }
  cmdPrint (cmsh, "uargc2[%d]\n", uargc2);
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint (cmsh, "uargv2[%d] = [%s]\n", i, uargv2[i]);
  }
  cmdPrint (cmsh, "cargc[%d]\n", cargc);
  for(i = 0; i < cargc; i++)
  {
    cmdPrint (cmsh, "cargv[%d] = [%s]\n", i, cargv[i]);
  }
#endif

  /* Get or create key chain node. */
  KeychainT *pKeychain = NULL;
  pKeychain = keychainLookup (cargv[3]);
  if (! pKeychain)
  {
    cmdPrint (cmsh, "Warning. Can't find keychain %s", cargv[3]);
    return CMD_IPC_OK;
  }

  keychainDelete (pKeychain);

  return (CMD_IPC_OK);
}


DENODEC(cmdFuncEnterKeychainKey,
        CMD_NODE_CONFIG_KEYCHAIN_KEY, 
        IPC_RIP)
{
#if 0
  Int32T i;
  cmdPrint (cmsh, "uargc1[%d]\n", uargc1);
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint (cmsh, "uargv1[%d] = [%s]\n", i, uargv1[i]);
  }
  cmdPrint (cmsh, "uargc2[%d]\n", uargc2);
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint (cmsh, "uargv2[%d] = [%s]\n", i, uargv2[i]);
  }
  cmdPrint (cmsh, "cargc[%d]\n", cargc);
  for(i = 0; i < cargc; i++)
  {
    cmdPrint (cmsh, "cargv[%d] = [%s]\n", i, cargv[i]);
  }
#endif

  KeychainT * pKeychain = NULL;
  KeyT *pKey = NULL;
  Int32T idx = 0;

  /* Get keychain pointer. */
  pKeychain = keychainGet (uargv1[2]);
  if (!pKeychain)
  {
    cmsh->outputStr = strdup("Error] not exist or cannot create keychain-node\n");
    return (CMD_IPC_OK);
  }

  /* Get index. */
  idx = atoi (cargv[1]);

  /* Get key pointer. */
  pKey = keyGet (pKeychain, idx);
  if (!pKey)
  {
    cmsh->outputStr = strdup("Error] not exist or cannot create key node\n");
    return (CMD_IPC_OK);
  }

  return (CMD_IPC_OK);
}



DECMD (cmdFuncNoKeychainKey,
       CMD_NODE_CONFIG_KEYCHAIN,
       IPC_RIP,
       "no key <0-2147483647>",
       "Negate a command or set its defaults",
       "Delete a key",
       "Key identifier number")
{
#if 0
  Int32T i;
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint (cmsh, "Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint (cmsh, "Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }

  for(i = 0; i < cargc; i++)
  {
    cmdPrint (cmsh, "Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  KeychainT * pKeychain = NULL;
  KeyT * pKey = NULL;
  Uint32T idx = 0;

  /* Lookup Keychain. */  
  pKeychain = keychainLookup (uargv1[2]);
  if (!pKeychain)
  {
    cmsh->outputStr = strdup("Error] not exist or cannot create key-chain node\n");
    return (CMD_IPC_OK);
  }

  idx = atoi (cargv[2]);

  pKey = keyLookup (pKeychain, idx);
  if (! pKey)
  {
    cmdPrint (cmsh, "Warning. Can't find key %d", idx);
    return CMD_IPC_OK;
  }

  keyDelete (pKeychain, pKey);

  return (CMD_IPC_OK);
}


DECMD (cmdFuncKeyString,
       CMD_NODE_CONFIG_KEYCHAIN_KEY,
       IPC_RIP,
       "key-string WORD",
       "Set key string",
       "The key")
{
#if 0
  Int32T i;
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }

  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  KeychainT * pKeychain = NULL;
  KeyT *pKey = NULL;
  Int32T idx = 0;

  /* Get keychain pointer. */
  pKeychain = keychainGet (uargv1[2]);

  /* Get index. */
  idx = atoi (uargv2[1]);

  /* Get key pointer. */
  pKey = keyGet (pKeychain, idx);

  if (pKey->string)
    NNFREE (MEM_KEY_NAME, pKey->string);

  pKey->string = nnStrDup (cargv[1], MEM_KEY_NAME);

  return (CMD_IPC_OK);
}


DECMD (cmdFuncNoKeyString,
       CMD_NODE_CONFIG_KEYCHAIN_KEY,
       IPC_RIP,
       "no key-string",
       "Negate a command or set its defaults",
       "Unset key string",
       "The key")
{
#if 0
  Int32T i;
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }

  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  KeychainT * pKeychain = NULL;
  KeyT *pKey = NULL;
  Int32T idx = 0;

  /* Get keychain pointer. */
  pKeychain = keychainGet (uargv1[2]);

  /* Get index. */
  idx = atoi (uargv2[1]);

  /* Get key pointer. */
  pKey = keyGet (pKeychain, idx);

  /* Check string & free memory. */
  if (pKey->string)
  {
    NNFREE (MEM_KEY_NAME, pKey->string);
    pKey->string = NULL;
  }

  return (CMD_IPC_OK);
}



DECMD (cmdFuncAcceptLifetimeDayMonthDayMonth,
       CMD_NODE_CONFIG_KEYCHAIN_KEY,
       IPC_RIP,
       "accept-lifetime HH:MM:SS <1-31> MONTH <1993-2035> HH:MM:SS <1-31> MONTH <1993-2035>",
       "Set accept lifetime of the key",
       "Time to start",
       "Day of th month to start",
       "Month of the year to start",
       "Year to start",
       "Time to expire",
       "Day of th month to expire",
       "Month of the year to expire",
       "Year to expire")
{
#if 0
  Int32T i;
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif


  KeychainT * pKeychain = NULL;
  KeyT *pKey = NULL;
  Int32T idx = 0;

  /* Get keychain pointer. */
  pKeychain = keychainGet (uargv1[2]);

  /* Get index. */
  idx = atoi (uargv2[1]);

  /* Get key pointer. */
  pKey = keyGet (pKeychain, idx);

  /* Set command value. */
  return keyLifetimeSet (cmsh, &pKey->accept, cargv[1], cargv[2], cargv[3],
                         cargv[4], cargv[5], cargv[6], cargv[7], cargv[8]);
}


DECMD (cmdFuncAcceptLifetimeDayMonthMonthDay,
       CMD_NODE_CONFIG_KEYCHAIN_KEY,
       IPC_RIP,
       "accept-lifetime HH:MM:SS <1-31> MONTH <1993-2035> HH:MM:SS MONTH <1-31> <1993-2035>",
       "Set accept lifetime of the key",
       "Time to start",
       "Day of th month to start",
       "Month of the year to start",
       "Year to start",
       "Time to expire",
       "Month of the year to expire",
       "Day of th month to expire",
       "Year to expire")
{
#if 0
  Int32T i;
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  KeychainT * pKeychain = NULL;
  KeyT *pKey = NULL;
  Int32T idx = 0;

  /* Get keychain pointer. */
  pKeychain = keychainGet (uargv1[2]);

  /* Get index. */
  idx = atoi (uargv2[1]);

  /* Get key pointer. */
  pKey = keyGet (pKeychain, idx);

  /* Set command value. */
  return keyLifetimeSet (cmsh, &pKey->accept, cargv[1], cargv[2], cargv[3],
                         cargv[4], cargv[5], cargv[7], cargv[6], cargv[8]);
}


DECMD (cmdFuncAcceptLifetimeMonthDayDayMonth,
       CMD_NODE_CONFIG_KEYCHAIN_KEY,
       IPC_RIP,
       "accept-lifetime HH:MM:SS MONTH <1-31> <1993-2035> HH:MM:SS <1-31> MONTH <1993-2035>",
       "Set accept lifetime of the key",
       "Time to start",
       "Month of the year to start",
       "Day of th month to start",
       "Year to start",
       "Time to expire",
       "Day of th month to expire",
       "Month of the year to expire",
       "Year to expire")
{
#if 0
  Int32T i;
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  KeychainT * pKeychain = NULL;
  KeyT *pKey = NULL;
  Int32T idx = 0;

  /* Get keychain pointer. */
  pKeychain = keychainGet (uargv1[2]);

  /* Get index. */
  idx = atoi (uargv2[1]);

  /* Get key pointer. */
  pKey = keyGet (pKeychain, idx);

  /* Set command value. */
  return keyLifetimeSet (cmsh, &pKey->accept, cargv[1], cargv[3], cargv[2],
                         cargv[4], cargv[5], cargv[6], cargv[7], cargv[8]);
}

DECMD (cmdFuncAcceptLifetimeMonthDayMonthDay,
       CMD_NODE_CONFIG_KEYCHAIN_KEY,
       IPC_RIP,
       "accept-lifetime HH:MM:SS MONTH <1-31> <1993-2035> HH:MM:SS MONTH <1-31> <1993-2035>",
       "Set accept lifetime of the key",
       "Time to start",
       "Month of the year to start",
       "Day of th month to start",
       "Year to start",
       "Time to expire",
       "Month of the year to expire",
       "Day of th month to expire",
       "Year to expire")
{
#if 0
  Int32T i;
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  KeychainT * pKeychain = NULL;
  KeyT *pKey = NULL;
  Int32T idx = 0;

  /* Get keychain pointer. */
  pKeychain = keychainGet (uargv1[2]);

  /* Get index. */
  idx = atoi (uargv2[1]);

  /* Get key pointer. */
  pKey = keyGet (pKeychain, idx);

  /* Set command value. */
  return keyLifetimeSet (cmsh, &pKey->accept, cargv[1], cargv[3], cargv[2],
                         cargv[4], cargv[5], cargv[7], cargv[6], cargv[8]);
}


DECMD (cmdFuncAcceptLifetimeInfiniteDayMonth,
       CMD_NODE_CONFIG_KEYCHAIN_KEY,
       IPC_RIP,
       "accept-lifetime HH:MM:SS <1-31> MONTH <1993-2035> infinite",
       "Set accept lifetime of the key",
       "Time to start",
       "Day of th month to start",
       "Month of the year to start",
       "Year to start",
       "Never expires")
{
#if 0
  Int32T i;
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  KeychainT * pKeychain = NULL;
  KeyT *pKey = NULL;
  Int32T idx = 0;

  /* Get keychain pointer. */
  pKeychain = keychainGet (uargv1[2]);

  /* Get index. */
  idx = atoi (uargv2[1]);

  /* Get key pointer. */
  pKey = keyGet (pKeychain, idx);

  /* Set command value. */
  return keyLifetimeInfiniteSet (cmsh, &pKey->accept, cargv[1], cargv[2],
                                 cargv[3], cargv[4]);
}

DECMD (cmdFuncAcceptLifetimeInfiniteMonthDay,
       CMD_NODE_CONFIG_KEYCHAIN_KEY,
       IPC_RIP,
       "accept-lifetime HH:MM:SS MONTH <1-31> <1993-2035> infinite",
       "Set accept lifetime of the key",
       "Time to start",
       "Month of the year to start",
       "Day of th month to start",
       "Year to start",
       "Never expires")
{
#if 0
  Int32T i;
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  KeychainT * pKeychain = NULL;
  KeyT *pKey = NULL;
  Int32T idx = 0;

  /* Get keychain pointer. */
  pKeychain = keychainGet (uargv1[2]);

  /* Get index. */
  idx = atoi (uargv2[1]);

  /* Get key pointer. */
  pKey = keyGet (pKeychain, idx);

  /* Set command value. */
  return keyLifetimeInfiniteSet (cmsh, &pKey->accept, cargv[1], cargv[3],
                                 cargv[2], cargv[4]);
}


DECMD (cmdFuncAcceptLifetimeDurationDayMonth,
       CMD_NODE_CONFIG_KEYCHAIN_KEY,
       IPC_RIP,
       "accept-lifetime HH:MM:SS <1-31> MONTH <1993-2035> duration <1-2147483646>",
       "Set accept lifetime of the key",
       "Time to start",
       "Day of th month to start",
       "Month of the year to start",
       "Year to start",
       "Duration of the key",
       "Duration seconds")
{
#if 0 
  Int32T i;
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  KeychainT * pKeychain = NULL;
  KeyT *pKey = NULL;
  Int32T idx = 0;

  /* Get keychain pointer. */
  pKeychain = keychainGet (uargv1[2]);

  /* Get index. */
  idx = atoi (uargv2[1]);

  /* Get key pointer. */
  pKey = keyGet (pKeychain, idx);

  /* Set command value. */
  return keyLifetimeDurationSet (cmsh, &pKey->accept, cargv[1], cargv[2],
                                 cargv[3], cargv[4], cargv[5]);
}

DECMD (cmdFuncAcceptLifetimeDurationMonthDay,
       CMD_NODE_CONFIG_KEYCHAIN_KEY,
       IPC_RIP,
       "accept-lifetime HH:MM:SS MONTH <1-31> <1993-2035> duration <1-2147483646>",
       "Set accept lifetime of the key",
       "Time to start",
       "Month of the year to start",
       "Day of th month to start",
       "Year to start",
       "Duration of the key",
       "Duration seconds")
{
#if 0
  Int32T i;
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif


  KeychainT * pKeychain = NULL;
  KeyT *pKey = NULL;
  Int32T idx = 0;

  /* Get keychain pointer. */
  pKeychain = keychainGet (uargv1[2]);

  /* Get index. */
  idx = atoi (uargv2[1]);

  /* Get key pointer. */
  pKey = keyGet (pKeychain, idx);

  /* Set command value. */
  return keyLifetimeDurationSet (cmsh, &pKey->accept, cargv[1], cargv[3],
                                 cargv[2], cargv[4], cargv[5]);
}



DECMD (cmdFuncSendLifetimeDayMonthDayMonth,
       CMD_NODE_CONFIG_KEYCHAIN_KEY,
       IPC_RIP,
       "send-lifetime HH:MM:SS <1-31> MONTH <1993-2035> HH:MM:SS <1-31> MONTH <1993-2035>",
       "Set send lifetime of the key",
       "Time to start",
       "Day of th month to start",
       "Month of the year to start",
       "Year to start",
       "Time to expire",
       "Day of th month to expire",
       "Month of the year to expire",
       "Year to expire")
{
#if 0
  Int32T i;
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  KeychainT * pKeychain = NULL;
  KeyT *pKey = NULL;
  Int32T idx = 0;

  /* Get keychain pointer. */
  pKeychain = keychainGet (uargv1[2]);

  /* Get index. */
  idx = atoi (uargv2[1]);

  /* Get key pointer. */
  pKey = keyGet (pKeychain, idx);

  /* Set command value. */
  return keyLifetimeSet (cmsh, &pKey->send, cargv[1], cargv[2], cargv[3], 
                         cargv[4], cargv[5], cargv[6], cargv[7], cargv[8]);
}

DECMD (cmdFuncSendLifetimeDayMonthMonthDay,
       CMD_NODE_CONFIG_KEYCHAIN_KEY,
       IPC_RIP,
       "send-lifetime HH:MM:SS <1-31> MONTH <1993-2035> HH:MM:SS MONTH <1-31> <1993-2035>",
       "Set send lifetime of the key",
       "Time to start",
       "Day of th month to start",
       "Month of the year to start",
       "Year to start",
       "Time to expire",
       "Month of the year to expire",
       "Day of th month to expire",
       "Year to expire")
{
#if 0
  Int32T i;
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  KeychainT * pKeychain = NULL;
  KeyT *pKey = NULL;
  Int32T idx = 0;

  /* Get keychain pointer. */
  pKeychain = keychainGet (uargv1[2]);

  /* Get index. */
  idx = atoi (uargv2[1]);

  /* Get key pointer. */
  pKey = keyGet (pKeychain, idx);

  /* Set command value. */
  return keyLifetimeSet (cmsh, &pKey->send, cargv[1], cargv[2], cargv[3], 
                         cargv[4], cargv[5], cargv[7], cargv[6], cargv[8]);
}


DECMD (cmdFuncSendLifetimeMonthDayDayMonth,
       CMD_NODE_CONFIG_KEYCHAIN_KEY,
       IPC_RIP,
       "send-lifetime HH:MM:SS MONTH <1-31> <1993-2035> HH:MM:SS <1-31> MONTH <1993-2035>",
       "Set send lifetime of the key",
       "Time to start",
       "Month of the year to start",
       "Day of th month to start",
       "Year to start",
       "Time to expire",
       "Day of th month to expire",
       "Month of the year to expire",
       "Year to expire")
{
#if 0
  Int32T i;
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  KeychainT * pKeychain = NULL;
  KeyT *pKey = NULL;
  Int32T idx = 0;

  /* Get keychain pointer. */
  pKeychain = keychainGet (uargv1[2]);

  /* Get index. */
  idx = atoi (uargv2[1]);

  /* Get key pointer. */
  pKey = keyGet (pKeychain, idx);

  /* Set command value. */
  return keyLifetimeSet (cmsh, &pKey->send, cargv[1], cargv[3], cargv[2], 
                         cargv[4], cargv[5], cargv[6], cargv[7], cargv[8]);
}


DECMD (cmdFuncSendLifetimeMonthDayMonthDay,
       CMD_NODE_CONFIG_KEYCHAIN_KEY,
       IPC_RIP,
       "send-lifetime HH:MM:SS MONTH <1-31> <1993-2035> HH:MM:SS MONTH <1-31> <1993-2035>",
       "Set send lifetime of the key",
       "Time to start",
       "Month of the year to start",
       "Day of th month to start",
       "Year to start",
       "Time to expire",
       "Month of the year to expire",
       "Day of th month to expire",
       "Year to expire")
{
#if 0
  Int32T i;
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  KeychainT * pKeychain = NULL;
  KeyT *pKey = NULL;
  Int32T idx = 0;

  /* Get keychain pointer. */
  pKeychain = keychainGet (uargv1[2]);

  /* Get index. */
  idx = atoi (uargv2[1]);

  /* Get key pointer. */
  pKey = keyGet (pKeychain, idx);

  /* Set command value. */
  return keyLifetimeSet (cmsh, &pKey->send, cargv[1], cargv[3], cargv[2], 
                         cargv[4], cargv[5], cargv[7], cargv[6], cargv[8]);
}

DECMD (cmdFuncSendLifetimeInfiniteDayMonth,
       CMD_NODE_CONFIG_KEYCHAIN_KEY,
       IPC_RIP,
       "send-lifetime HH:MM:SS <1-31> MONTH <1993-2035> infinite",
       "Set send lifetime of the key",
       "Time to start",
       "Day of th month to start",
       "Month of the year to start",
       "Year to start",
       "Never expires")
{
#if 0
  Int32T i;
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  KeychainT * pKeychain = NULL;
  KeyT *pKey = NULL;
  Int32T idx = 0;

  /* Get keychain pointer. */
  pKeychain = keychainGet (uargv1[2]);

  /* Get index. */
  idx = atoi (uargv2[1]);

  /* Get key pointer. */
  pKey = keyGet (pKeychain, idx);

  /* Set command value. */
  return keyLifetimeInfiniteSet (cmsh, &pKey->send, cargv[1], cargv[2], 
                                 cargv[3], cargv[4]);
}

DECMD (cmdFuncSendLifetimeInfiniteMonthDay,
       CMD_NODE_CONFIG_KEYCHAIN_KEY,
       IPC_RIP,
       "send-lifetime HH:MM:SS MONTH <1-31> <1993-2035> infinite",
       "Set send lifetime of the key",
       "Time to start",
       "Month of the year to start",
       "Day of th month to start",
       "Year to start",
       "Never expires")
{
#if 0
  Int32T i;
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  KeychainT * pKeychain = NULL;
  KeyT *pKey = NULL;
  Int32T idx = 0;

  /* Get keychain pointer. */
  pKeychain = keychainGet (uargv1[2]);

  /* Get index. */
  idx = atoi (uargv2[1]);

  /* Get key pointer. */
  pKey = keyGet (pKeychain, idx);

  /* Set command value. */
  return keyLifetimeInfiniteSet (cmsh, &pKey->send, cargv[1], cargv[3], 
                                 cargv[2], cargv[4]);
}

DECMD (cmdFuncSendLifetimeDurationDayMonth,
       CMD_NODE_CONFIG_KEYCHAIN_KEY,
       IPC_RIP,
       "send-lifetime HH:MM:SS <1-31> MONTH <1993-2035> duration <1-2147483646>",
       "Set send lifetime of the key",
       "Time to start",
       "Day of th month to start",
       "Month of the year to start",
       "Year to start",
       "Duration of the key",
       "Duration seconds")
{
#if 0
  Int32T i;
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  KeychainT * pKeychain = NULL;
  KeyT *pKey = NULL;
  Int32T idx = 0;

  /* Get keychain pointer. */
  pKeychain = keychainGet (uargv1[2]);

  /* Get index. */
  idx = atoi (uargv2[1]);

  /* Get key pointer. */
  pKey = keyGet (pKeychain, idx);

  /* Set command value. */
  return keyLifetimeDurationSet (cmsh, &pKey->send, cargv[1], cargv[2], 
                                 cargv[3], cargv[4], cargv[5]);
}

DECMD (cmdFuncSendLifetimeDurationMonthDay,
       CMD_NODE_CONFIG_KEYCHAIN_KEY,
       IPC_RIP,
       "send-lifetime HH:MM:SS MONTH <1-31> <1993-2035> duration <1-2147483646>",
       "Set send lifetime of the key",
       "Time to start",
       "Month of the year to start",
       "Day of th month to start",
       "Year to start",
       "Duration of the key",
       "Duration seconds")
{
#if 0
  Int32T i;
  for(i = 0; i < uargc1; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv1[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv1[i]);
  }
  for(i = 0; i < uargc2; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Parent Node uargv2[%d] = %s\n", __FILE__, __func__, __LINE__, i, uargv2[i]);
  }
  for(i = 0; i < cargc; i++)
  {
    cmdPrint(cmsh,"Enter [%s][%s][%d] Command's cargv[%d] = %s\n", __FILE__, __func__, __LINE__, i, cargv[i]);
  }
#endif

  KeychainT * pKeychain = NULL;
  KeyT *pKey = NULL;
  Int32T idx = 0;

  /* Get keychain pointer. */
  pKeychain = keychainGet (uargv1[2]);

  /* Get index. */
  idx = atoi (uargv2[1]);

  /* Get key pointer. */
  pKey = keyGet (pKeychain, idx);

  /* Set command value. */
  return keyLifetimeDurationSet (cmsh, &pKey->send, cargv[1], cargv[3], 
                                 cargv[2], cargv[4], cargv[5]);
}


