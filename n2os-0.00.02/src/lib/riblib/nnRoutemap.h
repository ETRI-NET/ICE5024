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
 * @brief : This file defines the Routemap related definitions.
 *  - Block Name : riblib
 *  - Process Name : rib library
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2014/03/13
 */

/**
 * @file : nnRouteMap.h
 *
 * $Id:$
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_nnRoutemap_h)
#define _nnRoutemap_h

/* Route map's type. */
typedef enum eRouteMapType
{
  RMAP_PERMIT,
  RMAP_DENY,
  RMAP_ANY
}eRouteMapType;

typedef enum eRouteMapResult
{
  RMAP_MATCH,
  RMAP_DENYMATCH,
  RMAP_NOMATCH,
  RMAP_ERROR,
  RMAP_OKAY
} eRouteMapResultT;

typedef enum eRouteMapObject
{
  RMAP_RIP,
  RMAP_RIPNG,
  RMAP_OSPF,
  RMAP_OSPF6,
  RMAP_BGP,
  RMAP_ZEBRA
} eRouteMapObjectT;

typedef enum eRouteMapEnd
{
  RMAP_EXIT,
  RMAP_GOTO,
  RMAP_NEXT
} eRouteMapEndT;

typedef enum eRouteMapEvent
{
  RMAP_EVENT_SET_ADDED,
  RMAP_EVENT_SET_DELETED,
  RMAP_EVENT_SET_REPLACED,
  RMAP_EVENT_MATCH_ADDED,
  RMAP_EVENT_MATCH_DELETED,
  RMAP_EVENT_MATCH_REPLACED,
  RMAP_EVENT_INDEX_ADDED,
  RMAP_EVENT_INDEX_DELETED
} eRouteMapEventT;

/* Depth limit in RMAP recursion using RMAP_CALL. */
#define RMAP_RECURSION_LIMIT      10

/* Route map rule structure for matching and setting. */
struct RouteMapRuleCmd
{
  /* Route map rule name (e.g. as-path, metric) */
  const StringT str;

  /* Function for value set or match. */
  eRouteMapResultT (*func_apply)(void *, PrefixT *, 
				                 eRouteMapObjectT, void *);

  /* Compile argument and return result as void *. */
  void *(*funcCompile)(const StringT);

  /* Free allocated value by funcCompile (). */
  void (*funcFree)(void *);
};
typedef struct RouteMapRuleCmd RouteMapRuleCmdT;

/* Route map apply error. */
enum
{
  /* Route map rule is missing. */
  RMAP_RULE_MISSING = 1,

  /* Route map rule can't compile */
  RMAP_COMPILE_ERROR
};

/* Route map rule list. */
struct RouteMapRuleList
{
  struct RouteMapRule *head;
  struct RouteMapRule *tail;
};
typedef struct RouteMapRuleList RouteMapRuleListT;

/* Route map index structure. */
struct RouteMapIndex
{
  struct RouteMap *map;
  StringT description;

  /* Preference of this route map rule. */
  Int32T pref;

  /* Route map type permit or deny. */
  eRouteMapType type;			

  /* Do we follow old rules, or hop forward? */
  eRouteMapEndT exitpolicy;

  /* If we're using "GOTO", to where do we go? */
  Int32T nextpref;

  /* If we're using "CALL", to which route-map do ew go? */
  StringT nextrm;

  /* Matching rule list. */
  struct RouteMapRuleList matchList;
  struct RouteMapRuleList setList;

  /* Make linked list. */
  struct RouteMapIndex *next;
  struct RouteMapIndex *prev;
};
typedef struct RouteMapIndex RouteMapIndexT;

/* Route map list structure. */
struct RouteMap
{
  /* Name of route map. */
  StringT name;

  /* Route map's rule. */
  struct RouteMapIndex *head;
  struct RouteMapIndex *tail;

  /* Make linked list. */
  struct RouteMap *next;
  struct RouteMap *prev;
};
typedef struct RouteMap RouteMapT;

/* Prototypes. */
extern void routeMapInit (void);
extern VectorT routeMapMatchVecGetPtr ();
extern VectorT routeMapSetVecGetPtr ();
extern void routeMapUpdate (VectorT, VectorT);
extern void routeMapInitVty (void);
extern void routeMapFinish (void);

/* Add match statement to route map. */
extern Int32T routeMapAddMatch (RouteMapIndexT *,
                                const StringT, const StringT);

/* Delete specified route match rule. */
extern Int32T routeMapDeleteMatch (RouteMapIndexT *,
                                   const StringT, const StringT);

/* Add route-map set statement to the route map. */
extern Int32T routeMapAddSet (RouteMapIndexT *, 
                              const StringT, const StringT);

/* Delete route map set rule. */
extern Int32T routeMapDeleteSet (RouteMapIndexT *,
                                 const StringT, const StringT);

/* Install rule command to the match list. */
extern void routeMapInstallMatch (RouteMapRuleCmdT *);

/* Install rule command to the set list. */
extern void routeMapInstallSet (RouteMapRuleCmdT *);

/* Lookup route map by name. */
extern RouteMapT * routeMapLookupByName (const StringT);

/* Apply route map to the object. */
extern eRouteMapResultT routeMapApply (RouteMapT *, PrefixT *,
                                       eRouteMapObjectT, void *);

extern void routeMapAddHook (void (*func) (const char *));
extern void routeMapDeleteHook (void (*func) (const char *));
extern void routeMapEventHook (void (*func) (eRouteMapEventT, const char *));

#endif /* _nnRoutemap_h */
