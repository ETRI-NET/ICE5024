/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : 각 컴포넌트에서 사용하는 공통 interface routemap 관련 기능을 
 *                제공한다.
 * - Block Name : riblib
 * - Process Name : rib library
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : nnIfRouteMap.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */
#include <assert.h>
#include <string.h>

#include "hash.h"
#include "nnStr.h"
#include "nnIf.h"
#include "nnIfRouteMap.h"
#include "nosLib.h"

struct hash *pIfRouteMapHash;

/* Hook functions. */
static void (*ifRouteMapAddHook) (IfRouteMapT *) = NULL;
static void (*ifRouteMapDeleteHook) (IfRouteMapT *) = NULL;

static IfRouteMapT *
ifRouteMapNew (void)
{
  IfRouteMapT *pNew;

  pNew = NNMALLOC (MEM_ROUTEMAP, sizeof (IfRouteMapT));

  return pNew;
}

static void
ifRouteMapFree (IfRouteMapT *pIfRouteMap)
{
  if (pIfRouteMap->ifName)
    NNFREE (MEM_IF_NAME, pIfRouteMap->ifName);

  if (pIfRouteMap->routeMap[IF_RMAP_IN])
    NNFREE (MEM_ROUTEMAP_IN, pIfRouteMap->routeMap[IF_RMAP_IN]);

  if (pIfRouteMap->routeMap[IF_RMAP_OUT])
    NNFREE (MEM_ROUTEMAP_OUT, pIfRouteMap->routeMap[IF_RMAP_OUT]);

  NNFREE (MEM_ROUTEMAP, pIfRouteMap);
}

IfRouteMapT *
ifRouteMapLookup (const StringT ifName)
{
  IfRouteMapT key = {0,};
  IfRouteMapT *pIfRouteMap = NULL;

  /* temporary copy */
  key.ifName = ifName;

  pIfRouteMap = hash_lookup (pIfRouteMapHash, &key);
  
  return pIfRouteMap;
}

void
ifRouteMapHookAdd (void (*func) (IfRouteMapT *))
{
  ifRouteMapAddHook = func;
}

void
ifRouteMapHookDelete (void (*func) (IfRouteMapT *))
{
  ifRouteMapDeleteHook = func;
}

static void *
ifRouteMapHashAlloc (void *arg)
{
  IfRouteMapT *ifarg = arg;
  IfRouteMapT *pIfRouteMap = NULL;

  pIfRouteMap = ifRouteMapNew ();
  pIfRouteMap->ifName = nnStrDup (ifarg->ifName, MEM_IF_NAME);

  return pIfRouteMap;
}

static IfRouteMapT *
ifRouteMapGet (const StringT ifName)
{
  IfRouteMapT key = {0,};

  /* temporary copy */
  key.ifName = ifName;

  return (IfRouteMapT *) hash_get (pIfRouteMapHash, &key, ifRouteMapHashAlloc);
}

static Uint32T
ifRouteMapHashMake (void *data)
{
  IfRouteMapT *pIfRouteMap = data;

  return string_hash_make (pIfRouteMap->ifName);
}

static Int32T
ifRouteMapHashCmp (const void *arg1, const void* arg2)
{
  const IfRouteMapT *pIfRouteMap1 = arg1;
  const IfRouteMapT *pIfRouteMap2 = arg2;

  return strcmp (pIfRouteMap1->ifName, pIfRouteMap2->ifName) == 0;
}

static IfRouteMapT *
ifRouteMapSet (const StringT ifName, eIfRouteMapTypeT type, 
               const StringT strRouteMapName)
{
  IfRouteMapT *pIfRouteMap = NULL;

  pIfRouteMap = ifRouteMapGet (ifName);

  if (type == IF_RMAP_IN)
  {
    if (pIfRouteMap->routeMap[IF_RMAP_IN])
      NNFREE (MEM_ROUTEMAP_IN, pIfRouteMap->routeMap[IF_RMAP_IN]);
    pIfRouteMap->routeMap[IF_RMAP_IN] = nnStrDup (strRouteMapName, MEM_ROUTEMAP_IN);
  }
  if (type == IF_RMAP_OUT)
  {
    if (pIfRouteMap->routeMap[IF_RMAP_OUT])
      NNFREE (MEM_ROUTEMAP_OUT, pIfRouteMap->routeMap[IF_RMAP_OUT]);
    pIfRouteMap->routeMap[IF_RMAP_OUT] = nnStrDup (strRouteMapName, MEM_ROUTEMAP_OUT);
  }

  if (ifRouteMapAddHook)
    (*ifRouteMapAddHook) (pIfRouteMap);
  
  return pIfRouteMap;
}

static Int32T
ifRouteMapUnset (const StringT ifName, eIfRouteMapTypeT type, 
                 const StringT strRouteMapName)
{
  IfRouteMapT *pIfRouteMap = NULL;

  pIfRouteMap = ifRouteMapLookup (ifName);
  if (!pIfRouteMap)
    return 0;

  if (type == IF_RMAP_IN)
  {
    if (!pIfRouteMap->routeMap[IF_RMAP_IN])
      return 0;
    if (strcmp (pIfRouteMap->routeMap[IF_RMAP_IN], strRouteMapName) != 0)
      return 0;

    NNFREE (MEM_ROUTEMAP_IN,  pIfRouteMap->routeMap[IF_RMAP_IN]);
    pIfRouteMap->routeMap[IF_RMAP_IN] = NULL;      
  }

  if (type == IF_RMAP_OUT)
  {
    if (!pIfRouteMap->routeMap[IF_RMAP_OUT])
      return 0;
    if (strcmp (pIfRouteMap->routeMap[IF_RMAP_OUT], strRouteMapName) != 0)
      return 0;

    NNFREE (MEM_ROUTEMAP_OUT, pIfRouteMap->routeMap[IF_RMAP_OUT]);
    pIfRouteMap->routeMap[IF_RMAP_OUT] = NULL;      
  }

  if (ifRouteMapDeleteHook)
    (*ifRouteMapDeleteHook) (pIfRouteMap);

  if (pIfRouteMap->routeMap[IF_RMAP_IN] == NULL &&
      pIfRouteMap->routeMap[IF_RMAP_OUT] == NULL)
  {
    hash_release (pIfRouteMapHash, pIfRouteMap);
    ifRouteMapFree (pIfRouteMap);
  }

  return 1;
}

#if 0
DEFUN (if_rmap,
       if_rmap_cmd,
       "route-map RMAP_NAME (in|out) IFNAME",
       "Route map set\n"
       "Route map name\n"
       "Route map set for input filtering\n"
       "Route map set for output filtering\n"
       "Route map interface name\n")
{
  eIfRouteMapTypeT type;

  if (strncmp (argv[1], "i", 1) == 0)
    type = IF_RMAP_IN;
  else if (strncmp (argv[1], "o", 1) == 0)
    type = IF_RMAP_OUT;
  else
    {
      vty_out (vty, "route-map direction must be [in|out]%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  ifRouteMapSet (argv[2], type, argv[0]);

  return CMD_SUCCESS;
}      

ALIAS (if_rmap,
       if_ipv6_rmap_cmd,
       "route-map RMAP_NAME (in|out) IFNAME",
       "Route map set\n"
       "Route map name\n"
       "Route map set for input filtering\n"
       "Route map set for output filtering\n"
       "Route map interface name\n")

DEFUN (no_if_rmap,
       no_if_rmap_cmd,
       "no route-map ROUTEMAP_NAME (in|out) IFNAME",
       NO_STR
       "Route map unset\n"
       "Route map name\n"
       "Route map for input filtering\n"
       "Route map for output filtering\n"
       "Route map interface name\n")
{
  Int32T ret;
  eIfRouteMapTypeT type;

  if (strncmp (argv[1], "i", 1) == 0)
    type = IF_RMAP_IN;
  else if (strncmp (argv[1], "o", 1) == 0)
    type = IF_RMAP_OUT;
  else
    {
      vty_out (vty, "route-map direction must be [in|out]%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  ret = ifRouteMapUnset (argv[2], type, argv[0]);
  if (! ret)
    {
      vty_out (vty, "route-map doesn't exist%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  return CMD_SUCCESS;
}      

ALIAS (no_if_rmap,
       no_if_ipv6_rmap_cmd,
       "no route-map ROUTEMAP_NAME (in|out) IFNAME",
       NO_STR
       "Route map unset\n"
       "Route map name\n"
       "Route map for input filtering\n"
       "Route map for output filtering\n"
       "Route map interface name\n")
#endif 


/* Configuration write function. */
Int32T
configWriteIfRouteMap (struct cmsh *cmsh)
{
  Uint32T i = 0;
  struct hash_backet *mp = NULL;

  for (i = 0; i < pIfRouteMapHash->size; i++)
    for (mp = pIfRouteMapHash->index[i]; mp; mp = mp->next)
    {
      IfRouteMapT *ifRmap = NULL;

      ifRmap = mp->data;
      if (ifRmap->routeMap[IF_RMAP_IN])
      {
        cmdPrint (cmsh, " route-map %s in %s", 
                  ifRmap->routeMap[IF_RMAP_IN], ifRmap->ifName);
      }

      if (ifRmap->routeMap[IF_RMAP_OUT])
      {
        cmdPrint (cmsh, " route-map %s out %s", 
                  ifRmap->routeMap[IF_RMAP_OUT], ifRmap->ifName);
      }
    }
  return 0;
}


void
ifRouteMapReset ()
{
  hash_clean (pIfRouteMapHash, (void (*) (void *)) ifRouteMapFree);
}

void
ifRouteMapInit (Int32T node)
{
  pIfRouteMapHash = hash_create (ifRouteMapHashMake, ifRouteMapHashCmp);
#if 0
  if (node == RIPNG_NODE) {
    install_element (RIPNG_NODE, &if_ipv6_rmap_cmd);
    install_element (RIPNG_NODE, &no_if_ipv6_rmap_cmd);
  } else if (node == RIP_NODE) {
    install_element (RIP_NODE, &if_rmap_cmd);
    install_element (RIP_NODE, &no_if_rmap_cmd);
  }
#endif
}

/* Get interface route map hash pointer. */
struct hash * 
ifRouteMapGetPtr ()
{
  assert (pIfRouteMapHash);

  return pIfRouteMapHash; 
}


/* Update distribue list related hash. */
void
ifRouteMapVersionUpdate (Int32T node, struct hash * pHash)
{
  assert (pHash);

  /* Assign global pointer to pDistHash. */
  pIfRouteMapHash = pHash;

  /* Set callback function. */
  pIfRouteMapHash->hash_key = ifRouteMapHashMake;
  pIfRouteMapHash->hash_cmp = ifRouteMapHashCmp; 
}
