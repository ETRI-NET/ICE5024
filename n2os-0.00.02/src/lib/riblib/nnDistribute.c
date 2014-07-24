/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : 각 컴포넌트에서 사용하는 공통 distribute 관련 기능을 제공한다.
 * - Block Name : riblib
 * - Process Name : rib library
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : nnDistribute.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include "hash.h"
#include "nnStr.h"
#include "nnIf.h"
#include "nnFilter.h"
#include "nnDistribute.h"
#include "nosLib.h"

#include "nnCmdDefines.h"
#include "nnCompProcess.h"
#include "nnCmdCmsh.h"

/* Hash of distribute list. */
struct hash *pDistHash;

/* Hook functions. */
void (*distributeAddHook) (DistributeT *);
void (*distributeDeleteHook) (DistributeT *);

static DistributeT *
distributeNew (void)
{
  return NNMALLOC (MEM_DISTRIBUTE, sizeof (DistributeT));
}

/* Free distribute object. */
static void
distributeFree (DistributeT *pDist)
{
  if (pDist->ifName)
    NNFREE (MEM_IF_NAME, pDist->ifName);

  if (pDist->filterName[DISTRIBUTE_IN])
    NNFREE (MEM_FILTER_NAME, pDist->filterName[DISTRIBUTE_IN]);
  if (pDist->filterName[DISTRIBUTE_OUT])
    NNFREE (MEM_FILTER_NAME, pDist->filterName[DISTRIBUTE_OUT]);

  if (pDist->prefixName[DISTRIBUTE_IN])
    NNFREE (MEM_PREFIX_NAME, pDist->prefixName[DISTRIBUTE_IN]);
  if (pDist->prefixName[DISTRIBUTE_OUT])
    NNFREE (MEM_PREFIX_NAME, pDist->prefixName[DISTRIBUTE_OUT]);

  NNFREE (MEM_DISTRIBUTE, pDist);
}

/* Lookup interface's distribute list. */
DistributeT *
distributeLookup (const StringT ifName)
{
  DistributeT key = {0,};
  DistributeT *pDist = NULL;

  /* temporary reference */
  key.ifName = ifName;

  pDist = hash_lookup (pDistHash, &key);
  
  return pDist;
}

void
distributeListAddHook (void (*func) (DistributeT *))
{
  distributeAddHook = func;
}

void
distributeListDeleteHook (void (*func) (DistributeT *))
{
  distributeDeleteHook = func;
}

static void *
distributeHashAlloc (DistributeT *pArg)
{
  DistributeT *pDist = NULL;

  pDist = distributeNew ();
  if (pArg->ifName)
    pDist->ifName = nnStrDup (pArg->ifName, MEM_IF_NAME);
  else
    pDist->ifName = NULL;
  return pDist;
}

/* Make new distribute list and push into hash. */
static DistributeT *
distributeGet (const StringT ifName)
{
  DistributeT key = {0,};

  /* temporary reference */
  key.ifName = ifName;
  
  return hash_get (pDistHash, &key, (void * (*) (void *))distributeHashAlloc);
}

static Uint32T
distributeHashMake (void * arg)
{
  const DistributeT * pDist = arg;

  return pDist->ifName ? string_hash_make (pDist->ifName) : 0;
}

/* If two distribute-list have same value then return 1 else return
   0. This function is used by hash package. */
static Int32T
distributeCmp (const DistributeT *pDist1, const DistributeT *pDist2)
{
  if (pDist1->ifName && pDist2->ifName)
    if (strcmp (pDist1->ifName, pDist2->ifName) == 0)
      return 1;
  if (! pDist1->ifName && ! pDist2->ifName)
    return 1;
  return 0;
}

/* Set access-list name to the distribute list. */
static DistributeT *
distributeListSet (const StringT ifName, eDistributeTypeT type, 
                   const StringT aListName)
{
  DistributeT *pDist = NULL;

  pDist = distributeGet (ifName);

  if (type == DISTRIBUTE_IN)
  {
    if (pDist->filterName[DISTRIBUTE_IN])
      NNFREE (MEM_FILTER_NAME, pDist->filterName[DISTRIBUTE_IN]);
    pDist->filterName[DISTRIBUTE_IN] = nnStrDup (aListName, MEM_FILTER_NAME);
  }
  if (type == DISTRIBUTE_OUT)
  {
    if (pDist->filterName[DISTRIBUTE_OUT])
      NNFREE (MEM_FILTER_NAME, pDist->filterName[DISTRIBUTE_OUT]);
    pDist->filterName[DISTRIBUTE_OUT] = nnStrDup (aListName, MEM_FILTER_NAME);
  }

  /* Apply this distribute-list to the interface. */
  (*distributeAddHook) (pDist);
  
  return pDist;
}

/* Unset distribute-list.  If matched distribute-list exist then
   return 1. */
static Int32T
distributeListUnset (const StringT ifName, eDistributeTypeT type, 
                     const StringT aListName)
{
  DistributeT *pDist = NULL;

  pDist = distributeLookup (ifName);
  if (!pDist)
    return 0;

  if (type == DISTRIBUTE_IN)
  {
    if (!pDist->filterName[DISTRIBUTE_IN])
      return 0;
    if (strcmp (pDist->filterName[DISTRIBUTE_IN], aListName) != 0)
      return 0;

    NNFREE (MEM_FILTER_NAME, pDist->filterName[DISTRIBUTE_IN]);
    pDist->filterName[DISTRIBUTE_IN] = NULL;      
  }

  if (type == DISTRIBUTE_OUT)
  {
    if (!pDist->filterName[DISTRIBUTE_OUT])
      return 0;
    if (strcmp (pDist->filterName[DISTRIBUTE_OUT], aListName) != 0)
      return 0;

    NNFREE (MEM_FILTER_NAME, pDist->filterName[DISTRIBUTE_OUT]);
    pDist->filterName[DISTRIBUTE_OUT] = NULL;      
  }

  /* Apply this distribute-list to the interface. */
  (*distributeDeleteHook) (pDist);

  /* If both out and in is NULL then free distribute list. */
  if (pDist->filterName[DISTRIBUTE_IN] == NULL &&
      pDist->filterName[DISTRIBUTE_OUT] == NULL &&
      pDist->prefixName[DISTRIBUTE_IN] == NULL &&
      pDist->prefixName[DISTRIBUTE_OUT] == NULL)
  {
    hash_release (pDistHash, pDist);
    distributeFree (pDist);
  }

  return 1;
}

/* Set access-list name to the distribute list. */
static DistributeT *
distributeListPrefixSet (const StringT ifName, eDistributeTypeT type,
                         const StringT listName)
{
  DistributeT *pDist = NULL;

  pDist = distributeGet (ifName);

  if (type == DISTRIBUTE_IN)
  {
    if (pDist->prefixName[DISTRIBUTE_IN])
      NNFREE (MEM_PREFIX_NAME, pDist->prefixName[DISTRIBUTE_IN]);
    pDist->prefixName[DISTRIBUTE_IN] = nnStrDup (listName, MEM_PREFIX_NAME);
  }
  if (type == DISTRIBUTE_OUT)
  {
    if (pDist->prefixName[DISTRIBUTE_OUT])
      NNFREE (MEM_PREFIX_NAME, pDist->prefixName[DISTRIBUTE_OUT]);
    pDist->prefixName[DISTRIBUTE_OUT] = nnStrDup (listName, MEM_PREFIX_NAME);
  }

  /* Apply this distribute-list to the interface. */
  (*distributeAddHook) (pDist);
  
  return pDist;
}

/* Unset distribute-list.  If matched distribute-list exist then
   return 1. */
static Int32T
distributeListPrefixUnset (const StringT ifName, eDistributeTypeT type,
                           const StringT listName)
{
  DistributeT *pDist = NULL;

  pDist = distributeLookup (ifName);
  if (!pDist)
    return 0;

  if (type == DISTRIBUTE_IN)
  {
    if (!pDist->prefixName[DISTRIBUTE_IN])
      return 0;
    if (strcmp (pDist->prefixName[DISTRIBUTE_IN], listName) != 0)
      return 0;

    NNFREE (MEM_PREFIX_NAME, pDist->prefixName[DISTRIBUTE_IN]);
    pDist->prefixName[DISTRIBUTE_IN] = NULL;      
  }

  if (type == DISTRIBUTE_OUT)
  {
    if (!pDist->prefixName[DISTRIBUTE_OUT])
      return 0;
    if (strcmp (pDist->prefixName[DISTRIBUTE_OUT], listName) != 0)
      return 0;

    NNFREE (MEM_PREFIX_NAME, pDist->prefixName[DISTRIBUTE_OUT]);
    pDist->prefixName[DISTRIBUTE_OUT] = NULL;      
  }

  /* Apply this distribute-list to the interface. */
  (*distributeDeleteHook) (pDist);

  /* If both out and in is NULL then free distribute list. */
  if (pDist->filterName[DISTRIBUTE_IN] == NULL &&
      pDist->filterName[DISTRIBUTE_OUT] == NULL &&
      pDist->prefixName[DISTRIBUTE_IN] == NULL &&
      pDist->prefixName[DISTRIBUTE_OUT] == NULL)
  {
    hash_release (pDistHash, pDist);
    distributeFree (pDist);
  }

  return 1;
}

#if 0
DEFUN (distribute_list_all,
       distribute_list_all_cmd,
       "distribute-list WORD (in|out)",
       "Filter networks in routing updates\n"
       "Access-list name\n"
       "Filter incoming routing updates\n"
       "Filter outgoing routing updates\n")
{
  eDistributeTypeT type;

  /* Check of distribute list type. */
  if (strncmp (argv[1], "i", 1) == 0)
    type = DISTRIBUTE_IN;
  else if (strncmp (argv[1], "o", 1) == 0)
    type = DISTRIBUTE_OUT;
  else
  {
    vty_out (vty, "distribute list direction must be [in|out]%s", VTY_NEWLINE);
    return CMD_WARNING;
  }

  /* Get interface name corresponding distribute list. */
  distributeListSet (NULL, type, argv[0]);

  return CMD_SUCCESS;
}

ALIAS (distribute_list_all,
       ipv6_distribute_list_all_cmd,
       "distribute-list WORD (in|out)",
       "Filter networks in routing updates\n"
       "Access-list name\n"
       "Filter incoming routing updates\n"
       "Filter outgoing routing updates\n")

DEFUN (no_distribute_list_all,
       no_distribute_list_all_cmd,
       "no distribute-list WORD (in|out)",
       NO_STR
       "Filter networks in routing updates\n"
       "Access-list name\n"
       "Filter incoming routing updates\n"
       "Filter outgoing routing updates\n")
{
  Int32T ret;
  eDistributeTypeT type;

  /* Check of distribute list type. */
  if (strncmp (argv[1], "i", 1) == 0)
    type = DISTRIBUTE_IN;
  else if (strncmp (argv[1], "o", 1) == 0)
    type = DISTRIBUTE_OUT;
  else
    {
      vty_out (vty, "distribute list direction must be [in|out]%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  ret = distributeListUnset (NULL, type, argv[0]);
  if (! ret)
    {
      vty_out (vty, "distribute list doesn't exist%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  return CMD_SUCCESS;
}

ALIAS (no_distribute_list_all,
       no_ipv6_distribute_list_all_cmd,
       "no distribute-list WORD (in|out)",
       NO_STR
       "Filter networks in routing updates\n"
       "Access-list name\n"
       "Filter incoming routing updates\n"
       "Filter outgoing routing updates\n")

DEFUN (distribute_list,
       distribute_list_cmd,
       "distribute-list WORD (in|out) WORD",
       "Filter networks in routing updates\n"
       "Access-list name\n"
       "Filter incoming routing updates\n"
       "Filter outgoing routing updates\n"
       "Interface name\n")
{
  eDistributeTypeT type;

  /* Check of distribute list type. */
  if (strncmp (argv[1], "i", 1) == 0)
    type = DISTRIBUTE_IN;
  else if (strncmp (argv[1], "o", 1) == 0)
    type = DISTRIBUTE_OUT;
  else
  {
    vty_out (vty, "distribute list direction must be [in|out]%s", VTY_NEWLINE);
    return CMD_WARNING;
  }

  /* Get interface name corresponding distribute list. */
  distributeListSet (argv[2], type, argv[0]);

  return CMD_SUCCESS;
}       

ALIAS (distribute_list,
       ipv6_distribute_list_cmd,
       "distribute-list WORD (in|out) WORD",
       "Filter networks in routing updates\n"
       "Access-list name\n"
       "Filter incoming routing updates\n"
       "Filter outgoing routing updates\n"
       "Interface name\n")

DEFUN (no_districute_list, no_distribute_list_cmd,
       "no distribute-list WORD (in|out) WORD",
       NO_STR
       "Filter networks in routing updates\n"
       "Access-list name\n"
       "Filter incoming routing updates\n"
       "Filter outgoing routing updates\n"
       "Interface name\n")
{
  Int32T ret;
  eDistributeTypeT type;

  /* Check of distribute list type. */
  if (strncmp (argv[1], "i", 1) == 0)
    type = DISTRIBUTE_IN;
  else if (strncmp (argv[1], "o", 1) == 0)
    type = DISTRIBUTE_OUT;
  else
    {
      vty_out (vty, "distribute list direction must be [in|out]%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  ret = distributeListUnset (argv[2], type, argv[0]);
  if (! ret)
    {
      vty_out (vty, "distribute list doesn't exist%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  return CMD_SUCCESS;
}       

ALIAS (no_districute_list, no_ipv6_distribute_list_cmd,
       "no distribute-list WORD (in|out) WORD",
       NO_STR
       "Filter networks in routing updates\n"
       "Access-list name\n"
       "Filter incoming routing updates\n"
       "Filter outgoing routing updates\n"
       "Interface name\n")

DEFUN (districute_list_prefix_all,
       distribute_list_prefix_all_cmd,
       "distribute-list prefix WORD (in|out)",
       "Filter networks in routing updates\n"
       "Filter prefixes in routing updates\n"
       "Name of an IP prefix-list\n"
       "Filter incoming routing updates\n"
       "Filter outgoing routing updates\n")
{
  eDistributeTypeT type;

  /* Check of distribute list type. */
  if (strncmp (argv[1], "i", 1) == 0)
    type = DISTRIBUTE_IN;
  else if (strncmp (argv[1], "o", 1) == 0)
    type = DISTRIBUTE_OUT;
  else
    {
      vty_out (vty, "distribute list direction must be [in|out]%s", 
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Get interface name corresponding distribute list. */
  distributeListPrefixSet (NULL, type, argv[0]);

  return CMD_SUCCESS;
}       

ALIAS (districute_list_prefix_all,
       ipv6_distribute_list_prefix_all_cmd,
       "distribute-list prefix WORD (in|out)",
       "Filter networks in routing updates\n"
       "Filter prefixes in routing updates\n"
       "Name of an IP prefix-list\n"
       "Filter incoming routing updates\n"
       "Filter outgoing routing updates\n")

DEFUN (no_districute_list_prefix_all,
       no_distribute_list_prefix_all_cmd,
       "no distribute-list prefix WORD (in|out)",
       NO_STR
       "Filter networks in routing updates\n"
       "Filter prefixes in routing updates\n"
       "Name of an IP prefix-list\n"
       "Filter incoming routing updates\n"
       "Filter outgoing routing updates\n")
{
  Int32T ret;
  eDistributeTypeT type;

  /* Check of distribute list type. */
  if (strncmp (argv[1], "i", 1) == 0)
    type = DISTRIBUTE_IN;
  else if (strncmp (argv[1], "o", 1) == 0)
    type = DISTRIBUTE_OUT;
  else
    {
      vty_out (vty, "distribute list direction must be [in|out]%s", 
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  ret = distributeListPrefixUnset (NULL, type, argv[0]);
  if (! ret)
    {
      vty_out (vty, "distribute list doesn't exist%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  return CMD_SUCCESS;
}       

ALIAS (no_districute_list_prefix_all,
       no_ipv6_distribute_list_prefix_all_cmd,
       "no distribute-list prefix WORD (in|out)",
       NO_STR
       "Filter networks in routing updates\n"
       "Filter prefixes in routing updates\n"
       "Name of an IP prefix-list\n"
       "Filter incoming routing updates\n"
       "Filter outgoing routing updates\n")

DEFUN (districute_list_prefix, distribute_list_prefix_cmd,
       "distribute-list prefix WORD (in|out) WORD",
       "Filter networks in routing updates\n"
       "Filter prefixes in routing updates\n"
       "Name of an IP prefix-list\n"
       "Filter incoming routing updates\n"
       "Filter outgoing routing updates\n"
       "Interface name\n")
{
  eDistributeTypeT type;

  /* Check of distribute list type. */
  if (strncmp (argv[1], "i", 1) == 0)
    type = DISTRIBUTE_IN;
  else if (strncmp (argv[1], "o", 1) == 0)
    type = DISTRIBUTE_OUT;
  else
    {
      vty_out (vty, "distribute list direction must be [in|out]%s", 
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Get interface name corresponding distribute list. */
  distributeListPrefixSet (argv[2], type, argv[0]);

  return CMD_SUCCESS;
}       

ALIAS (districute_list_prefix, ipv6_distribute_list_prefix_cmd,
       "distribute-list prefix WORD (in|out) WORD",
       "Filter networks in routing updates\n"
       "Filter prefixes in routing updates\n"
       "Name of an IP prefix-list\n"
       "Filter incoming routing updates\n"
       "Filter outgoing routing updates\n"
       "Interface name\n")

DEFUN (no_districute_list_prefix, no_distribute_list_prefix_cmd,
       "no distribute-list prefix WORD (in|out) WORD",
       NO_STR
       "Filter networks in routing updates\n"
       "Filter prefixes in routing updates\n"
       "Name of an IP prefix-list\n"
       "Filter incoming routing updates\n"
       "Filter outgoing routing updates\n"
       "Interface name\n")
{
  Int32T ret;
  eDistributeTypeT type;

  /* Check of distribute list type. */
  if (strncmp (argv[1], "i", 1) == 0)
    type = DISTRIBUTE_IN;
  else if (strncmp (argv[1], "o", 1) == 0)
    type = DISTRIBUTE_OUT;
  else
    {
      vty_out (vty, "distribute list direction must be [in|out]%s", 
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  ret = distributeListPrefixUnset (argv[2], type, argv[0]);
  if (! ret)
    {
      vty_out (vty, "distribute list doesn't exist%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  return CMD_SUCCESS;
}       

ALIAS (no_districute_list_prefix, no_ipv6_distribute_list_prefix_cmd,
       "no distribute-list prefix WORD (in|out) WORD",
       NO_STR
       "Filter networks in routing updates\n"
       "Filter prefixes in routing updates\n"
       "Name of an IP prefix-list\n"
       "Filter incoming routing updates\n"
       "Filter outgoing routing updates\n"
       "Interface name\n")
#endif


Int32T
configShowDistribute (struct cmsh *cmsh)
{
  Uint32T i = 0;
  struct hash_backet *pMp = NULL;
  DistributeT *pDist = NULL;
  char strBuff[1024] = {};
  Int32T idx = 0;

  /* Output filter configuration. */
  pDist = distributeLookup (NULL);
  if (pDist && (pDist->filterName[DISTRIBUTE_OUT] || 
      pDist->prefixName[DISTRIBUTE_OUT]))
  {
    idx = sprintf (strBuff, 
                   "  Outgoing update filter list for all interface is");
    if (pDist->filterName[DISTRIBUTE_OUT])
    {
      idx += sprintf (strBuff + idx, 
                      " %s", pDist->filterName[DISTRIBUTE_OUT]);
    }

    if (pDist->prefixName[DISTRIBUTE_OUT])
    {
      idx += sprintf (strBuff, "%s (prefix-list) %s",
                      pDist->filterName[DISTRIBUTE_OUT] ? "," : "",
                      pDist->prefixName[DISTRIBUTE_OUT]);
    }
    cmdPrint (cmsh, "%s", strBuff);
  }
  else
  {
    cmdPrint (cmsh, "  Outgoing update filter list for all interface is not set");
  }

  for (i = 0; i < pDistHash->size; i++)
  {
    for (pMp = pDistHash->index[i]; pMp; pMp = pMp->next)
    {
      pDist = pMp->data;
      if (pDist->ifName)
      {
        if (pDist->filterName[DISTRIBUTE_OUT] || 
            pDist->prefixName[DISTRIBUTE_OUT])
        {
          memset (strBuff, 0, sizeof(strBuff));
          idx = sprintf (strBuff, "    %s filtered by", pDist->ifName);
          if (pDist->filterName[DISTRIBUTE_OUT])
          {
            idx += sprintf (strBuff + idx, 
                            " %s", pDist->filterName[DISTRIBUTE_OUT]);
          }
          if (pDist->prefixName[DISTRIBUTE_OUT])
          {
            idx += sprintf (strBuff + idx, "%s (prefix-list) %s",
                            pDist->filterName[DISTRIBUTE_OUT] ? "," : "",
                            pDist->prefixName[DISTRIBUTE_OUT]);
          }
	      cmdPrint (cmsh, "%s", strBuff);
        }
      }
    }
  }


  /* Input filter configuration. */
  pDist = distributeLookup (NULL);
  if (pDist && (pDist->filterName[DISTRIBUTE_IN] || 
      pDist->prefixName[DISTRIBUTE_IN]))
  {
    memset (strBuff, 0, sizeof (strBuff));
    idx = sprintf (strBuff, 
                   "  Incoming update filter list for all interface is");
    if (pDist->filterName[DISTRIBUTE_IN])
    {
      idx += sprintf (strBuff + idx, 
                      " %s", pDist->filterName[DISTRIBUTE_IN]);
    }
    if (pDist->prefixName[DISTRIBUTE_IN])
    {
      idx += sprintf (strBuff, "%s (prefix-list) %s",
                      pDist->filterName[DISTRIBUTE_IN] ? "," : "",
                      pDist->prefixName[DISTRIBUTE_IN]);
    }
    cmdPrint (cmsh, "%s", strBuff);
  }
  else
  {
    cmdPrint (cmsh, 
              "  Incoming update filter list for all interface is not set");
  }

  for (i = 0; i < pDistHash->size; i++)
  {
    for (pMp = pDistHash->index[i]; pMp; pMp = pMp->next)
    {
      pDist = pMp->data;
      if (pDist->ifName)
      {
        if (pDist->filterName[DISTRIBUTE_IN] || pDist->prefixName[DISTRIBUTE_IN])
	    {
          memset (strBuff, 0, sizeof (strBuff));
          idx = sprintf (strBuff, "    %s filtered by", pDist->ifName);
          if (pDist->filterName[DISTRIBUTE_IN])
          {
            idx += sprintf (strBuff, " %s", pDist->filterName[DISTRIBUTE_IN]);
          }
          if (pDist->prefixName[DISTRIBUTE_IN])
          {
            idx += sprintf (strBuff, "%s (prefix-list) %s",
                            pDist->filterName[DISTRIBUTE_IN] ? "," : "",
                            pDist->prefixName[DISTRIBUTE_IN]);
          }
          cmdPrint (cmsh, "%s", strBuff);
        }
      }
    }
  }

  return 0;
}


/* Configuration write function. */
Int32T
configWriteDistribute (struct cmsh *cmsh)
{
  Uint32T i = 0;
  struct hash_backet *mp = NULL;

  for (i = 0; i < pDistHash->size; i++)
    for (mp = pDistHash->index[i]; mp; mp = mp->next)
    {
      DistributeT *pDist = NULL;

      pDist = mp->data;
      if (pDist->filterName[DISTRIBUTE_IN])
      {
        cmdPrint (cmsh, " distribute-list %s in %s", 
                  pDist->filterName[DISTRIBUTE_IN],
                  pDist->ifName ? pDist->ifName : "");
      }

      if (pDist->filterName[DISTRIBUTE_OUT])
      {
        cmdPrint (cmsh, " distribute-list %s out %s", 
                  pDist->filterName[DISTRIBUTE_OUT],
                  pDist->ifName ? pDist->ifName : "");
      }

      if (pDist->prefixName[DISTRIBUTE_IN])
      {
        cmdPrint (cmsh, " distribute-list prefix %s in %s",
                  pDist->prefixName[DISTRIBUTE_IN],
                  pDist->ifName ? pDist->ifName : "");
      }

      if (pDist->prefixName[DISTRIBUTE_OUT])
      {
        cmdPrint (cmsh, " distribute-list prefix %s out %s",
                  pDist->prefixName[DISTRIBUTE_OUT],
                  pDist->ifName ? pDist->ifName : "");
      }
    }

  return SUCCESS;
}


/* Clear all distribute list. */
void
distributeListReset ()
{
  hash_clean (pDistHash, (void (*) (void *)) distributeFree);
}

/* Initialize distribute list related hash. */
void
distributeListInit (Int32T node)
{
  pDistHash = hash_create (distributeHashMake,
                          (Int32T (*) (const void *, const void *)) distributeCmp);
#if 0
  if(node==RIP_NODE) {
    install_element (RIP_NODE, &distribute_list_all_cmd);
    install_element (RIP_NODE, &no_distribute_list_all_cmd);
    install_element (RIP_NODE, &distribute_list_cmd);
    install_element (RIP_NODE, &no_distribute_list_cmd);
    install_element (RIP_NODE, &distribute_list_prefix_all_cmd);
    install_element (RIP_NODE, &no_distribute_list_prefix_all_cmd);
    install_element (RIP_NODE, &distribute_list_prefix_cmd);
    install_element (RIP_NODE, &no_distribute_list_prefix_cmd);
  } else {
    install_element (RIPNG_NODE, &ipv6_distribute_list_all_cmd);
    install_element (RIPNG_NODE, &no_ipv6_distribute_list_all_cmd);
    install_element (RIPNG_NODE, &ipv6_distribute_list_cmd);
    install_element (RIPNG_NODE, &no_ipv6_distribute_list_cmd);
    install_element (RIPNG_NODE, &ipv6_distribute_list_prefix_all_cmd);
    install_element (RIPNG_NODE, &no_ipv6_distribute_list_prefix_all_cmd);
    install_element (RIPNG_NODE, &ipv6_distribute_list_prefix_cmd);
    install_element (RIPNG_NODE, &no_ipv6_distribute_list_prefix_cmd);
  }
#endif

}


/* Get distribute list hash pointer. */
struct hash * 
distributeListGetPtr ()
{
  assert (pDistHash);

  return pDistHash; 
}


/* Update distribue list related hash. */
void
distributeListVersionUpdate (Int32T node, struct hash * pHash)
{
  assert (pHash);

  /* Assign global pointer to pDistHash. */
  pDistHash = pHash;

  /* Set callback function. */
  pDistHash->hash_key = (Uint32T (*) (void *)) distributeHashMake;
  pDistHash->hash_cmp = (Int32T (*) (const void *, const void *)) distributeCmp; 
}
