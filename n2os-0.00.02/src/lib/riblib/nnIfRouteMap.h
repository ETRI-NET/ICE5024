/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the interface routemap definitions.
 *  - Block Name : riblib
 *  - Process Name : rib library
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : nnIfRouteMap.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_nnIfRouteMap_h)
#define _nnIfRouteMap_h

#include "hash.h"

#include "nnCmdCmsh.h"

typedef enum eIfRouteMapType
{
  IF_RMAP_IN,
  IF_RMAP_OUT,
  IF_RMAP_MAX
} eIfRouteMapTypeT;

struct IfRouteMap
{
  /* Name of the interface. */
  StringT ifName;

  StringT routeMap[IF_RMAP_MAX];
};
typedef struct IfRouteMap IfRouteMapT;

extern void ifRouteMapInit (Int32T);
extern struct hash * ifRouteMapGetPtr ();
extern void ifRouteMapVersionUpdate (Int32T, struct hash *);
extern void ifRouteMapReset (void);
extern void ifRouteMapHookAdd (void (*) (IfRouteMapT *));
extern void ifRouteMapHookDelete (void (*) (IfRouteMapT *));
extern IfRouteMapT *ifRouteMapLookup (const StringT);
extern int configWriteIfRouteMap (struct cmsh *);

#endif /* _nnIfRouteMap_h */
