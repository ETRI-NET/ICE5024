/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : 각 컴포넌트에서 사용하는 공통 routemap 관련 기능을 제공한다.
 * - Block Name : riblib
 * - Process Name : rib library
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : nnRouteMap.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "nnTypes.h"
#include "nnStr.h"
#include "nnList.h"
#include "nnVector.h"
#include "nnPrefix.h"
#include "nnRoutemap.h"
#include "nosLib.h"

/* Vector for route match rules. */
static VectorT pRouteMatchVec;

/* Vector for route set rules. */
static VectorT pRouteSetVec;

/* Route map rule. This rule has both `match' rule and `set' rule. */
struct RouteMapRule
{
  /* Rule type. */
  struct RouteMapRuleCmd *cmd;

  /* For pretty printing. */
  StringT ruleStr;

  /* Pre-compiled match rule. */
  void *value;

  /* Linked list. */
  struct RouteMapRule *next;
  struct RouteMapRule *prev;
};
typedef struct RouteMapRule RouteMapRuleT;

/* Making route map list. */
struct RouteMapList
{
  struct RouteMap *head;
  struct RouteMap *tail;

  void (*addHook) (const char *);
  void (*deleteHook) (const char *);
  void (*eventHook) (eRouteMapEventT, const char *); 
};
typedef struct RouteMapList RouteMapListT;


/* Master list of route map. */
static RouteMapListT routeMapMaster = { NULL, NULL, NULL, NULL };

static void
routeMapRuleDelete (RouteMapRuleListT *, RouteMapRuleT *);

static void
routeMapIndexDelete (RouteMapIndexT *, Int32T);

/* New route map allocation. Please note route map's name must be
   specified. */
static RouteMapT *
routeMapNew (const StringT name)
{
  RouteMapT *pNew = NULL;

  pNew =  NNMALLOC (MEM_ROUTEMAP, sizeof (RouteMapT));
  pNew->name = nnStrDup (name, MEM_ROUTEMAP_NAME);
  return pNew;
}

/* Add new name to routeMap. */
static RouteMapT *
routeMapAdd (const StringT name)
{
  RouteMapT *pMap = NULL;
  RouteMapListT *list = NULL;

  pMap = routeMapNew (name);
  list = &routeMapMaster;
    
  pMap->next = NULL;
  pMap->prev = list->tail;
  if (list->tail)
    list->tail->next = pMap;
  else
    list->head = pMap;
  list->tail = pMap;

  /* Execute hook. */
  if (routeMapMaster.addHook)
    (*routeMapMaster.addHook) (name);

  return pMap;
}

/* Route map delete from list. */
static void
routeMapDelete (RouteMapT *pMap)
{
  RouteMapListT *list = NULL;
  RouteMapIndexT *index = NULL;
  StringT name;
  
  while ((index = pMap->head) != NULL)
    routeMapIndexDelete (index, 0);

  name = pMap->name;

  list = &routeMapMaster;

  if (pMap->next)
    pMap->next->prev = pMap->prev;
  else
    list->tail = pMap->prev;

  if (pMap->prev)
    pMap->prev->next = pMap->next;
  else
    list->head = pMap->next;

  NNFREE (MEM_ROUTEMAP, pMap);

  /* Execute deletion hook. */
  if (routeMapMaster.deleteHook)
    (*routeMapMaster.deleteHook) (name);

  if (name)
    NNFREE (MEM_ROUTEMAP_NAME, name);
}

/* Lookup route map by route map name string. */
RouteMapT *
routeMapLookupByName (const StringT name)
{
  RouteMapT *pMap = NULL;

  for (pMap = routeMapMaster.head; pMap; pMap = pMap->next)
    if (strcmp (pMap->name, name) == 0)
      return pMap;
  return NULL;
}

/* Lookup route map.  If there isn't route map create one and return
   it. */
static RouteMapT *
routeMapGet (const StringT name)
{
  RouteMapT *pMap;

  pMap = routeMapLookupByName (name);
  if (pMap == NULL)
    pMap = routeMapAdd (name);
  return pMap;
}

/* Return route map's type string. */
static const StringT 
routeMapTypeStr (eRouteMapType type)
{
  switch (type)
    {
    case RMAP_PERMIT:
      return "permit";
      break;
    case RMAP_DENY:
      return "deny";
      break;
    default:
      return "";
      break;
    }
}

static Int32T
routeMapEmpty (RouteMapT *pMap)
{
  if (pMap->head == NULL && pMap->tail == NULL)
    return 1;
  else
    return 0;
}

#if 0
/* show route-map */
static void
vty_show_route_map_entry (struct vty *vty, RouteMapT *pMap)
{
  RouteMapIndexT *pIndex = NULL;
  RouteMapRuleT *pRule = NULL;

  /* Print the name of the protocol */
  if (zlog_default)
    vty_out (vty, "%s:%s", zlog_proto_names[zlog_default->protocol],
             VTY_NEWLINE);

  for (pIndex = pMap->head; pIndex; pIndex = pIndex->next)
    {
      vty_out (vty, "route-map %s, %s, sequence %d%s",
               pMap->name, routeMapTypeStr (pIndex->type),
               pIndex->pref, VTY_NEWLINE);

      /* Description */
      if (pIndex->description)
	vty_out (vty, "  Description:%s    %s%s", VTY_NEWLINE,
		 pIndex->description, VTY_NEWLINE);
      
      /* Match clauses */
      vty_out (vty, "  Match clauses:%s", VTY_NEWLINE);
      for (pRule = pIndex->matchList.head; pRule; pRule = pRule->next)
        vty_out (vty, "    %s %s%s", 
                 pRule->cmd->str, pRule->ruleStr, VTY_NEWLINE);
      
      vty_out (vty, "  Set clauses:%s", VTY_NEWLINE);
      for (pRule = pIndex->setList.head; pRule; pRule = pRule->next)
        vty_out (vty, "    %s %s%s",
                 pRule->cmd->str, pRule->ruleStr, VTY_NEWLINE);
      
      /* Call clause */
      vty_out (vty, "  Call clause:%s", VTY_NEWLINE);
      if (pIndex->nextrm)
        vty_out (vty, "    Call %s%s", pIndex->nextrm, VTY_NEWLINE);
      
      /* Exit Policy */
      vty_out (vty, "  Action:%s", VTY_NEWLINE);
      if (pIndex->exitpolicy == RMAP_GOTO)
        vty_out (vty, "    Goto %d%s", pIndex->nextpref, VTY_NEWLINE);
      else if (pIndex->exitpolicy == RMAP_NEXT)
        vty_out (vty, "    Continue to next entry%s", VTY_NEWLINE);
      else if (pIndex->exitpolicy == RMAP_EXIT)
        vty_out (vty, "    Exit routemap%s", VTY_NEWLINE);
    }
}
#endif

#if 0
static Int32T
vty_show_route_map (struct vty *vty, const char *name)
{
  RouteMapT *pMap = NULL;

  if (name)
    {
      pMap = routeMapLookupByName (name);

      if (pMap)
        {
          vty_show_route_map_entry (vty, pMap);
          return CMD_SUCCESS;
        }
      else
        {
          vty_out (vty, "%%route-map %s not found%s", name, VTY_NEWLINE);
          return CMD_WARNING;
        }
    }
  else
    {
      for (pMap = routeMapMaster.head; pMap; pMap = pMap->next)
	vty_show_route_map_entry (vty, pMap);
    }
  return CMD_SUCCESS;
}
#endif

/* New route map allocation. Please note route map's name must be
   specified. */
static RouteMapIndexT *
routeMapIndexNew (void)
{
  RouteMapIndexT *pNew = NULL;

  pNew =  NNMALLOC (MEM_ROUTEMAP_INDEX, sizeof (RouteMapIndexT));
  pNew->exitpolicy = RMAP_EXIT; /* Default to Cisco-style */

  return pNew;
}

/* Free route map index. */
static void
routeMapIndexDelete (RouteMapIndexT *pIndex, Int32T notify)
{
  RouteMapRuleT *pRule = NULL;

  /* Free route match. */
  while ((pRule = pIndex->matchList.head) != NULL)
    routeMapRuleDelete (&pIndex->matchList, pRule);

  /* Free route set. */
  while ((pRule = pIndex->setList.head) != NULL)
    routeMapRuleDelete (&pIndex->setList, pRule);

  /* Remove pIndex from route map list. */
  if (pIndex->next)
    pIndex->next->prev = pIndex->prev;
  else
    pIndex->map->tail = pIndex->prev;

  if (pIndex->prev)
    pIndex->prev->next = pIndex->next;
  else
    pIndex->map->head = pIndex->next;

  /* Free 'char *nextrm' if not NULL */
  if (pIndex->nextrm)
    NNFREE (MEM_OTHER, pIndex->nextrm);

    /* Execute event hook. */
  if (routeMapMaster.eventHook && notify)
    (*routeMapMaster.eventHook) (RMAP_EVENT_INDEX_DELETED,
				    pIndex->map->name);

  NNFREE (MEM_ROUTEMAP_INDEX, pIndex);
}

/* Lookup index from route map. */
static RouteMapIndexT *
routeMapIndexLookup (RouteMapT *pMap, eRouteMapType type, Int32T pref)
{
  RouteMapIndexT *pIndex;

  for (pIndex = pMap->head; pIndex; pIndex = pIndex->next)
    if ((pIndex->type == type || type == RMAP_ANY) && pIndex->pref == pref)
      return pIndex;

  return NULL;
}

/* Add new index to route map. */
static RouteMapIndexT *
routeMapIndexAdd (RouteMapT *pMap, eRouteMapType type, Int32T pref)
{
  RouteMapIndexT *pIndex = NULL;
  RouteMapIndexT *pPoint = NULL;

  /* Allocate new route map inex. */
  pIndex = routeMapIndexNew ();
  pIndex->map = pMap;
  pIndex->type = type;
  pIndex->pref = pref;
  
  /* Compare preference. */
  for (pPoint = pMap->head; pPoint; pPoint = pPoint->next)
    if (pPoint->pref >= pref)
      break;

  if (pMap->head == NULL)
  {
      pMap->head = pMap->tail = pIndex;
  }
  else if (pPoint == NULL)
  {
      pIndex->prev = pMap->tail;
      pMap->tail->next = pIndex;
      pMap->tail = pIndex;
  }
  else if (pPoint == pMap->head)
  {
      pIndex->next = pMap->head;
      pMap->head->prev = pIndex;
      pMap->head = pIndex;
  }
  else
  {
      pIndex->next = pPoint;
      pIndex->prev = pPoint->prev;
      if (pPoint->prev)
	pPoint->prev->next = pIndex;
      pPoint->prev = pIndex;
  }

  /* Execute event hook. */
  if (routeMapMaster.eventHook)
    (*routeMapMaster.eventHook) (RMAP_EVENT_INDEX_ADDED, pMap->name);

  return pIndex;
}

/* Get route map pIndex. */
static RouteMapIndexT *
routeMapIndexGet (RouteMapT *pMap, eRouteMapType type, Int32T pref)
{
  RouteMapIndexT *pIndex = NULL;

  pIndex = routeMapIndexLookup (pMap, RMAP_ANY, pref);
  if (pIndex && pIndex->type != type)
  {
      /* Delete pIndex from route map. */
      routeMapIndexDelete (pIndex, 1);
      pIndex = NULL;
  }
  if (pIndex == NULL)
    pIndex = routeMapIndexAdd (pMap, type, pref);

  return pIndex;
}

/* New route map rule */
static RouteMapRuleT *
routeMapRuleNew (void)
{
  RouteMapRuleT *pNew = NULL;

  pNew = NNMALLOC (MEM_ROUTEMAP_RULE, sizeof (RouteMapRuleT));

  return pNew;
}

/* Install rule command to the match list. */
void
routeMapInstallMatch (RouteMapRuleCmdT *cmd)
{
  vectorSet (pRouteMatchVec, cmd);
}

/* Install rule command to the set list. */
void
routeMapInstallSet (RouteMapRuleCmdT *cmd)
{
  vectorSet (pRouteSetVec, cmd);
}

/* Lookup rule command from match list. */
static RouteMapRuleCmdT *
routeMapLookupMatch (const StringT name)
{
  Uint32T i = 0;
  RouteMapRuleCmdT *pRule = NULL;

  for (i = 0; i < vectorActive (pRouteMatchVec); i++)
    if ((pRule = vectorSlot (pRouteMatchVec, i)) != NULL)
      if (strcmp (pRule->str, name) == 0)
        return pRule;

  return NULL;
}

/* Lookup rule command from set list. */
static RouteMapRuleCmdT *
routeMapLookupSet (const StringT name)
{
  Uint32T i;
  RouteMapRuleCmdT *pRule = NULL;

  for (i = 0; i < vectorActive (pRouteSetVec); i++)
    if ((pRule = vectorSlot (pRouteSetVec, i)) != NULL)
      if (strcmp (pRule->str, name) == 0)
        return pRule;

  return NULL;
}

/* Add match and set rule to rule list. */
static void
routeMapRuleAdd (RouteMapRuleListT *pList, RouteMapRuleT *pRule)
{
  pRule->next = NULL;
  pRule->prev = pList->tail;
  if (pList->tail)
    pList->tail->next = pRule;
  else
    pList->head = pRule;
  pList->tail = pRule;
}

/* Delete rule from rule list. */
static void
routeMapRuleDelete (RouteMapRuleListT *pList, RouteMapRuleT *pRule)
{
  if (pRule->cmd->funcFree)
    (*pRule->cmd->funcFree) (pRule->value);

  if (pRule->ruleStr)
    NNFREE (MEM_ROUTEMAP_RULE_NAME, pRule->ruleStr);

  if (pRule->next)
    pRule->next->prev = pRule->prev;
  else
    pList->tail = pRule->prev;
  if (pRule->prev)
    pRule->prev->next = pRule->next;
  else
    pList->head = pRule->next;

  NNFREE (MEM_ROUTEMAP_RULE, pRule);
}

/* strcmp wrapper function which don't crush even argument is NULL. */
static Int32T
rulecmp (const StringT dst, const StringT src)
{
  if (dst == NULL)
  {
    if (src ==  NULL)
      return 0;
    else
      return 1;
  }
  else
  {
    if (src == NULL)
      return 1;
    else
      return strcmp (dst, src);
  }
  return 1;
}

/* Add match statement to route map. */
Int32T
routeMapAddMatch (RouteMapIndexT *pIndex, const StringT matchName,
                  const StringT matchArg)
{
  RouteMapRuleT *pRule = NULL;
  RouteMapRuleT *pNext = NULL;
  RouteMapRuleCmdT *pCmd = NULL;
  void *pCompile;
  Int32T replaced = 0;

  /* First lookup rule for add match statement. */
  pCmd = routeMapLookupMatch (matchName);
  if (pCmd == NULL)
    return RMAP_RULE_MISSING;

  /* Next call pCompile function for this match statement. */
  if (pCmd->funcCompile)
  {
    pCompile= (*pCmd->funcCompile)(matchArg);
    if (pCompile == NULL)
      return RMAP_COMPILE_ERROR;
  }
  else
    pCompile = NULL;

  /* If argument is completely same ignore it. */
  for (pRule = pIndex->matchList.head; pRule; pRule = pNext)
  {
    pNext = pRule->next;
    if (pRule->cmd == pCmd)
    {	
      routeMapRuleDelete (&pIndex->matchList, pRule);
      replaced = 1;
    }
  }

  /* Add new route map match rule. */
  pRule = routeMapRuleNew ();
  pRule->cmd = pCmd;
  pRule->value = pCompile;
  if (matchArg)
    pRule->ruleStr = nnStrDup (matchArg, MEM_ROUTEMAP_RULE_NAME);
  else
    pRule->ruleStr = NULL;

  /* Add new route match rule to linked list. */
  routeMapRuleAdd (&pIndex->matchList, pRule);

  /* Execute event hook. */
  if (routeMapMaster.eventHook)
    (*routeMapMaster.eventHook) (replaced ?
                                  RMAP_EVENT_MATCH_REPLACED:
                                  RMAP_EVENT_MATCH_ADDED,
                                  pIndex->map->name);

  return 0;
}

/* Delete specified route match rule. */
Int32T
routeMapDeleteMatch (RouteMapIndexT *pIndex, const StringT matchName,
                     const StringT matchArg)
{
  RouteMapRuleT *pRule = NULL;
  RouteMapRuleCmdT *pCmd = NULL;

  pCmd = routeMapLookupMatch (matchName);
  if (pCmd == NULL)
    return 1;
  
  for (pRule = pIndex->matchList.head; pRule; pRule = pRule->next)
    if (pRule->cmd == pCmd && 
        (rulecmp (pRule->ruleStr, matchArg) == 0 || matchArg == NULL))
    {
      routeMapRuleDelete (&pIndex->matchList, pRule);
      /* Execute event hook. */
      if (routeMapMaster.eventHook)
        (*routeMapMaster.eventHook) (RMAP_EVENT_MATCH_DELETED, pIndex->map->name);

      return 0;
    }

  /* Can't find matched rule. */
  return 1;
}

/* Add route-map set statement to the route map. */
Int32T
routeMapAddSet (RouteMapIndexT *pIndex, const StringT setName,
                const StringT setArg)
{
  RouteMapRuleT *pRule = NULL;
  RouteMapRuleT *pNext = NULL;
  RouteMapRuleCmdT *pCmd = NULL;
  void *compile;
  Int32T replaced = 0;

  pCmd = routeMapLookupSet (setName);
  if (pCmd == NULL)
    return RMAP_RULE_MISSING;

  /* Next call compile function for this match statement. */
  if (pCmd->funcCompile)
  {
    compile= (*pCmd->funcCompile)(setArg);
    if (compile == NULL)
      return RMAP_COMPILE_ERROR;
  }
  else
    compile = NULL;

 /* Add by WJL. if old set command of same kind exist, delete it first
    to ensure only one set command of same kind exist under a
    routeMapIndex. */
  for (pRule = pIndex->setList.head; pRule; pRule = pNext)
  {
    pNext = pRule->next;
    if (pRule->cmd == pCmd)
    {
      routeMapRuleDelete (&pIndex->setList, pRule);
      replaced = 1;
    }
  }

  /* Add new route map match rule. */
  pRule = routeMapRuleNew ();
  pRule->cmd = pCmd;
  pRule->value = compile;
  if (setArg)
    pRule->ruleStr = nnStrDup (setArg, MEM_ROUTEMAP_RULE_NAME);
  else
    pRule->ruleStr = NULL;

  /* Add new route match rule to linked list. */
  routeMapRuleAdd (&pIndex->setList, pRule);

  /* Execute event hook. */
  if (routeMapMaster.eventHook)
    (*routeMapMaster.eventHook) (replaced ?
                                  RMAP_EVENT_SET_REPLACED:
                                  RMAP_EVENT_SET_ADDED,
                                  pIndex->map->name);
  return 0;
}

/* Delete route map set rule. */
Int32T
routeMapDeleteSet (RouteMapIndexT *pIndex, const StringT setName,
                   const StringT setArg)
{
  RouteMapRuleT *pRule = NULL;
  RouteMapRuleCmdT *pCmd = NULL;

  pCmd = routeMapLookupSet (setName);
  if (pCmd == NULL)
    return 1;
  
  for (pRule = pIndex->setList.head; pRule; pRule = pRule->next)
    if ((pRule->cmd == pCmd) &&
        (rulecmp (pRule->ruleStr, setArg) == 0 || setArg == NULL))
    {
      routeMapRuleDelete (&pIndex->setList, pRule);
      /* Execute event hook. */
      if (routeMapMaster.eventHook)
        (*routeMapMaster.eventHook) (RMAP_EVENT_SET_DELETED,
                                      pIndex->map->name);
      return 0;
    }

  /* Can't find matched rule. */
  return 1;
}

/* Apply route map's each pIndex to the object.

   The matrix for a route-map looks like this:
   (note, this includes the description for the "NEXT"
   and "GOTO" frobs now
  
              Match   |   No Match
                      |
    permit    action  |     cont
                      |
    ------------------+---------------
                      |
    deny      deny    |     cont
                      |
  
   action)
      -Apply Set statements, accept route
      -If Call statement is present jump to the specified route-map, if it
         denies the route we finish.
      -If NEXT is specified, goto NEXT statement
      -If GOTO is specified, goto the first clause where pref > nextpref
      -If nothing is specified, do as Cisco and finish
   deny)
      -Route is denied by route-map.
   cont)
      -Goto Next index
  
   If we get no matches after we've processed all updates, then the route
   is dropped too.
  
   Some notes on the new "CALL", "NEXT" and "GOTO"
     call WORD        - If this clause is matched, then the set statements
                        are executed and then we jump to route-map 'WORD'. If
                        this route-map denies the route, we finish, in other case we
                        do whatever the exit policy (EXIT, NEXT or GOTO) tells.
     on-match next    - If this clause is matched, then the set statements
                        are executed and then we drop through to the next clause
     on-match goto n  - If this clause is matched, then the set statments
                        are executed and then we goto the nth clause, or the
                        first clause greater than this. In order to ensure
                        route-maps *always* exit, you cannot jump backwards.
                        Sorry ;)
  
   We need to make sure our route-map processing matches the above
*/

static eRouteMapResultT
routeMapApplyMatch (RouteMapRuleListT *pMatchList,
                    PrefixT *pPrefix, eRouteMapObjectT type,
                    void *pObject)
{
  eRouteMapResultT ret = RMAP_NOMATCH;
  RouteMapRuleT *match = NULL;


  /* Check all match rule and if there is no match rule, go to the
     set statement. */
  if (!pMatchList->head)
    ret = RMAP_MATCH;
  else
  {
    for (match = pMatchList->head; match; match = match->next)
    {
      /* Try each match statement in turn, If any do not return
         RMAP_MATCH, return, otherwise continue on to next match 
         statement. All match statements must match for end-result
         to be a match. */
      ret = (*match->cmd->func_apply) (match->value, pPrefix, type, pObject);
      if (ret != RMAP_MATCH)
        return ret;
    }
  }
  return ret;
}

/* Apply route map to the object. */
eRouteMapResultT
routeMapApply (RouteMapT *map, PrefixT *pPrefix,
               eRouteMapObjectT type, void *pObject)
{
  static Int32T recursion = 0;
  Int32T ret = 0;
  RouteMapIndexT *pIndex = NULL;
  RouteMapRuleT *set = NULL;

  if (recursion > RMAP_RECURSION_LIMIT)
  {
    NNLOG (LOG_DEBUG, "route-map recursion limit (%d) reached, discarding route\n",
            RMAP_RECURSION_LIMIT);
    recursion = 0;
    return RMAP_DENYMATCH;
  }

  if (map == NULL)
    return RMAP_DENYMATCH;

  for (pIndex = map->head; pIndex; pIndex = pIndex->next)
  {
    /* Apply this pIndex. */
    ret = routeMapApplyMatch (&pIndex->matchList, pPrefix, type, pObject);

    /* Now we apply the matrix from above */
    if (ret == RMAP_NOMATCH)
      /* 'cont' from matrix - continue to next route-map sequence */
      continue;
    else if (ret == RMAP_MATCH)
    {
      if (pIndex->type == RMAP_PERMIT)
      /* 'action' */
      {
        /* permit+match must execute sets */
        for (set = pIndex->setList.head; set; set = set->next)
          ret = (*set->cmd->func_apply) (set->value, pPrefix, type, pObject);

        /* Call another route-map if available */
        if (pIndex->nextrm)
        {
          RouteMapT *nextrm = routeMapLookupByName (pIndex->nextrm);

          if (nextrm) /* Target route-map found, jump to it */
          {
            recursion++;
            ret = routeMapApply (nextrm, pPrefix, type, pObject);
            recursion--;
          }

          /* If nextrm returned 'deny', finish. */
          if (ret == RMAP_DENYMATCH)
            return ret;
        }
                
        switch (pIndex->exitpolicy)
        {
          case RMAP_EXIT:
            return ret;
          case RMAP_NEXT:
            continue;
          case RMAP_GOTO:
          {
            /* Find the next clause to jump to */
            RouteMapIndexT *next = pIndex->next;
            Int32T nextpref = pIndex->nextpref;

            while (next && next->pref < nextpref)
            {
              pIndex = next;
              next = next->next;
            }
            if (next == NULL)
            {
              /* No clauses match! */
              return ret;
            }
          }
        }
      }
      else if (pIndex->type == RMAP_DENY)
      /* 'deny' */
      {
        return RMAP_DENYMATCH;
      }
    }
  }

  /* Finally route-map does not match at all. */
  return RMAP_DENYMATCH;
}

void
routeMapAddHook (void (*func) (const char *))
{
  routeMapMaster.addHook = func;
}

void
routeMapDeleteHook (void (*func) (const char *))
{
  routeMapMaster.deleteHook = func;
}

void
routeMapEventHook (void (*func) (eRouteMapEventT, const char *))
{
  routeMapMaster.eventHook = func;
}

void
routeMapInit (void)
{
  /* Make vector for match and set. */
  pRouteMatchVec = vectorInit (1);
  pRouteSetVec = vectorInit (1);

  NNLOG (LOG_DEBUG, "pRouteMatchVec=%p, pRouteSetVec=%p\n", 
         pRouteMatchVec, pRouteSetVec);
}

VectorT
routeMapMatchVecGetPtr ()
{
  assert (pRouteMatchVec);

  /* Mapping vector pointer for match and set. */
  return pRouteMatchVec;
}

VectorT
routeMapSetVecGetPtr ()
{
  assert (pRouteSetVec);

  /* Mapping vector pointer for match and set. */
  return pRouteSetVec;
}


void
routeMapUpdate (VectorT matchVec, VectorT setVec)
{
  /* Make vector for match and set. */
  pRouteMatchVec = matchVec;
  pRouteSetVec = setVec;

  NNLOG (LOG_DEBUG, "pRouteMatchVec=%p, pRouteSetVec=%p\n", 
         pRouteMatchVec, pRouteSetVec);
}

void
routeMapFinish (void)
{
  vectorFree (pRouteMatchVec);
  pRouteMatchVec = NULL;

  vectorFree (pRouteSetVec);
  pRouteSetVec = NULL;
}

#if 0
/* VTY related functions. */
DEFUN (route_map,
       route_map_cmd,
       "route-map WORD (deny|permit) <1-65535>",
       "Create route-map or enter route-map command mode\n"
       "Route map tag\n"
       "Route map denies set operations\n"
       "Route map permits set operations\n"
       "Sequence to insert to/delete from existing route-map entry\n")
{
  Int32T permit;
  Uint32T pref;
  RouteMapT *map = NULL;
  RouteMapIndexT *pIndex = NULL;
  StringT endptr = NULL;

  /* Permit check. */
  if (strncmp (argv[1], "permit", strlen (argv[1])) == 0)
    permit = RMAP_PERMIT;
  else if (strncmp (argv[1], "deny", strlen (argv[1])) == 0)
    permit = RMAP_DENY;
  else
  {
    vty_out (vty, "the third field must be [permit|deny]%s", VTY_NEWLINE);
    return CMD_WARNING;
  }

  /* Preference check. */
  pref = strtoul (argv[2], &endptr, 10);
  if (pref == ULONG_MAX || *endptr != '\0')
  {
    vty_out (vty, "the fourth field must be positive integer%s",
             VTY_NEWLINE);
    return CMD_WARNING;
  }
  if (pref == 0 || pref > 65535)
  {
    vty_out (vty, "the fourth field must be <1-65535>%s", VTY_NEWLINE);
    return CMD_WARNING;
  }

  /* Get route map. */
  map = routeMapGet (argv[0]);
  pIndex = routeMapIndexGet (map, permit, pref);

  vty->index = pIndex;
  vty->node = RMAP_NODE;
  return CMD_SUCCESS;
}

DEFUN (no_route_map_all,
       no_route_map_all_cmd,
       "no route-map WORD",
       NO_STR
       "Create route-map or enter route-map command mode\n"
       "Route map tag\n")
{
  RouteMapT *map;

  map = routeMapLookupByName (argv[0]);
  if (map == NULL)
  {
    vty_out (vty, "%% Could not find route-map %s%s",
             argv[0], VTY_NEWLINE);
    return CMD_WARNING;
  }

  routeMapDelete (map);

  return CMD_SUCCESS;
}

DEFUN (no_route_map,
       no_route_map_cmd,
       "no route-map WORD (deny|permit) <1-65535>",
       NO_STR
       "Create route-map or enter route-map command mode\n"
       "Route map tag\n"
       "Route map denies set operations\n"
       "Route map permits set operations\n"
       "Sequence to insert to/delete from existing route-map entry\n")
{
  Int32T permit;
  Uint32T pref;
  RouteMapT *map = NULL;
  RouteMapIndexT *pIndex = NULL;
  StringT endptr = NULL;

  /* Permit check. */
  if (strncmp (argv[1], "permit", strlen (argv[1])) == 0)
    permit = RMAP_PERMIT;
  else if (strncmp (argv[1], "deny", strlen (argv[1])) == 0)
    permit = RMAP_DENY;
  else
  {
    vty_out (vty, "the third field must be [permit|deny]%s", VTY_NEWLINE);
    return CMD_WARNING;
  }

  /* Preference. */
  pref = strtoul (argv[2], &endptr, 10);
  if (pref == ULONG_MAX || *endptr != '\0')
  {
    vty_out (vty, "the fourth field must be positive integer%s",
             VTY_NEWLINE);
    return CMD_WARNING;
  }
  if (pref == 0 || pref > 65535)
  {
    vty_out (vty, "the fourth field must be <1-65535>%s", VTY_NEWLINE);
    return CMD_WARNING;
  }

  /* Existence check. */
  map = routeMapLookupByName (argv[0]);
  if (map == NULL)
  {
    vty_out (vty, "%% Could not find route-map %s%s",
             argv[0], VTY_NEWLINE);
    return CMD_WARNING;
  }

  /* Lookup route map pIndex. */
  pIndex = routeMapIndexLookup (map, permit, pref);
  if (pIndex == NULL)
  {
    vty_out (vty, "%% Could not find route-map entry %s %s%s", 
             argv[0], argv[2], VTY_NEWLINE);
    return CMD_WARNING;
  }

  /* Delete pIndex from route map. */
  routeMapIndexDelete (pIndex, 1);

  /* If this route rule is the last one, delete route map itself. */
  if (routeMapEmpty (map))
    routeMapDelete (map);

  return CMD_SUCCESS;
}

DEFUN (rmap_onmatch_next,
       rmap_onmatch_next_cmd,
       "on-match next",
       "Exit policy on matches\n"
       "Next clause\n")
{
  RouteMapIndexT *pIndex = NULL;

  pIndex = vty->index;

  if (pIndex)
    pIndex->exitpolicy = RMAP_NEXT;

  return CMD_SUCCESS;
}

DEFUN (no_rmap_onmatch_next,
       no_rmap_onmatch_next_cmd,
       "no on-match next",
       NO_STR
       "Exit policy on matches\n"
       "Next clause\n")
{
  RouteMapIndexT *pIndex = NULL;

  pIndex = vty->index;
  
  if (pIndex)
    pIndex->exitpolicy = RMAP_EXIT;

  return CMD_SUCCESS;
}

DEFUN (rmap_onmatch_goto,
       rmap_onmatch_goto_cmd,
       "on-match goto <1-65535>",
       "Exit policy on matches\n"
       "Goto Clause number\n"
       "Number\n")
{
  RouteMapIndexT *pIndex = vty->index;
  Int32T d = 0;

  if (pIndex)
  {
    if (argc == 1 && argv[0])
      VTY_GET_INTEGER_RANGE("route-map index", d, argv[0], 1, 65536);
    else
      d = pIndex->pref + 1;
      
    if (d <= pIndex->pref)
    {
      /* Can't allow you to do that, Dave */
      vty_out (vty, "can't jump backwards in route-maps%s", 
               VTY_NEWLINE);
      return CMD_WARNING;
    }
    else
    {
      pIndex->exitpolicy = RMAP_GOTO;
      pIndex->nextpref = d;
    }
  }
  return CMD_SUCCESS;
}

DEFUN (no_rmap_onmatch_goto,
       no_rmap_onmatch_goto_cmd,
       "no on-match goto",
       NO_STR
       "Exit policy on matches\n"
       "Goto Clause number\n")
{
  RouteMapIndexT *pIndex = NULL;

  pIndex = vty->index;

  if (pIndex)
    pIndex->exitpolicy = RMAP_EXIT;
  
  return CMD_SUCCESS;
}

/* Cisco/GNU Zebra compatible ALIASes for on-match next */
ALIAS (rmap_onmatch_goto,
       rmap_continue_cmd,
       "continue",
       "Continue on a different entry within the route-map\n")

ALIAS (no_rmap_onmatch_goto,
       no_rmap_continue_cmd,
       "no continue",
       NO_STR
       "Continue on a different entry within the route-map\n")

/* GNU Zebra compatible */
ALIAS (rmap_onmatch_goto,
       rmap_continue_seq_cmd,
       "continue <1-65535>",
       "Continue on a different entry within the route-map\n"
       "Route-map entry sequence number\n")

ALIAS (no_rmap_onmatch_goto,
       no_rmap_continue_seq,
       "no continue <1-65535>",
       NO_STR
       "Continue on a different entry within the route-map\n"
       "Route-map entry sequence number\n")

DEFUN (rmap_show_name,
       rmap_show_name_cmd,
       "show route-map [WORD]",
       SHOW_STR
       "route-map information\n"
       "route-map name\n")
{
    const StringT name = NULL;
    if (argc)
      name = argv[0];
    return vty_show_route_map (vty, name);
}

ALIAS (rmap_onmatch_goto,
      rmap_continue_index_cmd,
      "continue <1-65536>",
      "Exit policy on matches\n"
      "Goto Clause number\n")

DEFUN (rmap_call,
       rmap_call_cmd,
       "call WORD",
       "Jump to another Route-Map after match+set\n"
       "Target route-map name\n")
{
  RouteMapIndexT *pIndex = NULL;

  pIndex = vty->index;
  if (pIndex)
  {
    if (pIndex->nextrm)
      NNFREE (MEM_OTHER, pIndex->nextrm);
    pIndex->nextrm = nnStrDup (argv[0], MEM_OTHER);
  }
  return CMD_SUCCESS;
}

DEFUN (no_rmap_call,
       no_rmap_call_cmd,
       "no call",
       NO_STR
       "Jump to another Route-Map after match+set\n")
{
  RouteMapIndexT *pIndex = NULL;

  pIndex = vty->index;

  if (pIndex->nextrm)
  {
    NNFREE (MEM_OTHER, pIndex->nextrm);
    pIndex->nextrm = NULL;
  }

  return CMD_SUCCESS;
}

DEFUN (rmap_description,
       rmap_description_cmd,
       "description .LINE",
       "Route-map comment\n"
       "Comment describing this route-map rule\n")
{
  RouteMapIndexT *pIndex = NULL;

  pIndex = vty->index;
  if (pIndex)
  {
    if (pIndex->description)
      NNFREE (MEM_ROUTEMAP_DESC, pIndex->description);
    pIndex->description = argv_concat (argv, argc, 0);
  }
  return CMD_SUCCESS;
}

DEFUN (no_rmap_description,
       no_rmap_description_cmd,
       "no description",
       NO_STR
       "Route-map comment\n")
{
  RouteMapIndexT *pIndex = NULL;

  pIndex = vty->index;
  if (pIndex)
  {
    if (pIndex->description)
      NNFREE (MEM_ROUTEMAP_DESC, pIndex->description);
    pIndex->description = NULL;
  }
  return CMD_SUCCESS;
}
#endif

#if 0
/* Configuration write function. */
static Int32T
route_map_config_write (struct vty *vty)
{
  RouteMapT *pMap = NULL;
  RouteMapIndexT *pIndex = NULL;
  RouteMapRuleT *pRule = NULL;
  Int32T first = 1;
  Int32T write = 0;

  for (pMap = routeMapMaster.head; pMap; pMap = pMap->next)
    for (pIndex = pMap->head; pIndex; pIndex = pIndex->next)
    {
      if (!first)
        vty_out (vty, "!%s", VTY_NEWLINE);
      else
        first = 0;

      vty_out (vty, "route-map %s %s %d%s", 
               pMap->name,
               routeMapTypeStr (pIndex->type),
               pIndex->pref, VTY_NEWLINE);

      if (pIndex->description)
        vty_out (vty, " description %s%s", pIndex->description, VTY_NEWLINE);

      for (pRule = pIndex->matchList.head; pRule; pRule = pRule->next)
        vty_out (vty, " match %s %s%s", pRule->cmd->str, 
                 pRule->ruleStr ? pRule->ruleStr : "",
                 VTY_NEWLINE);

      for (pRule = pIndex->setList.head; pRule; pRule = pRule->next)
        vty_out (vty, " set %s %s%s", pRule->cmd->str,
                 pRule->ruleStr ? pRule->ruleStr : "",
                 VTY_NEWLINE);
      if (pIndex->nextrm)
        vty_out (vty, " call %s%s", pIndex->nextrm, VTY_NEWLINE);
      if (pIndex->exitpolicy == RMAP_GOTO)
        vty_out (vty, " on-match goto %d%s", pIndex->nextpref, VTY_NEWLINE);
      if (pIndex->exitpolicy == RMAP_NEXT)
        vty_out (vty," on-match next%s", VTY_NEWLINE);
	
      write++;
    }
  return write;
}
#endif

#if 0
/* Route map node structure. */
static struct cmd_node rmap_node =
{
  RMAP_NODE,
  "%s(config-route-map)# ",
  1
};
#endif

/* Initialization of route map vector. */
void
routeMapInitVty (void)
{
#if 0
  /* Install route map top node. */
  install_node (&rmap_node, route_map_config_write);

  /* Install route map commands. */
  install_default (RMAP_NODE);
  install_element (CONFIG_NODE, &route_map_cmd);
  install_element (CONFIG_NODE, &no_route_map_cmd);
  install_element (CONFIG_NODE, &no_route_map_all_cmd);

  /* Install the on-match stuff */
  install_element (RMAP_NODE, &route_map_cmd);
  install_element (RMAP_NODE, &rmap_onmatch_next_cmd);
  install_element (RMAP_NODE, &no_rmap_onmatch_next_cmd);
  install_element (RMAP_NODE, &rmap_onmatch_goto_cmd);
  install_element (RMAP_NODE, &no_rmap_onmatch_goto_cmd);
  
  /* Install the continue stuff (ALIAS of on-match). */
  install_element (RMAP_NODE, &rmap_continue_cmd);
  install_element (RMAP_NODE, &no_rmap_continue_cmd);
  install_element (RMAP_NODE, &rmap_continue_index_cmd);
  
  /* Install the call stuff. */
  install_element (RMAP_NODE, &rmap_call_cmd);
  install_element (RMAP_NODE, &no_rmap_call_cmd);

  /* Install description commands. */
  install_element (RMAP_NODE, &rmap_description_cmd);
  install_element (RMAP_NODE, &no_rmap_description_cmd);
   
  /* Install show command */
  install_element (ENABLE_NODE, &rmap_show_name_cmd);
#endif
}
