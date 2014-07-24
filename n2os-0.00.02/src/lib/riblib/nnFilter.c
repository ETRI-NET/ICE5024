/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : 각 프로토콜에서 사용하는 access list 관련 기능을 제공한다.
 * - Block Name : riblib
 * - Process Name : riblib
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : nnFilter.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include <assert.h>
#include <ctype.h>

#include "nnStr.h"
#include "nnPrefix.h"
#include "nnFilter.h"
#include "nnList.h"
#include "nosLib.h"


/* Static structure for IPv4 AccessList's master. */
static AccessMasterT gAccessMasterIpv4 = 
{ 
  {NULL, NULL},
  {NULL, NULL},
  NULL,
  NULL,
};

#ifdef HAVE_IPV6
/* Static structure for IPv6 AccessList's master. */
static AccessMasterT gAccessMasterIpv6 = 
{ 
  {NULL, NULL},
  {NULL, NULL},
  NULL,
  NULL,
};
#endif /* HAVE_IPV6 */

static AccessMasterT *
accessMasterGet (afi_t afi)
{
  if (afi == AFI_IP)
    return &gAccessMasterIpv4;
#ifdef HAVE_IPV6
  else if (afi == AFI_IP6)
    return &gAccessMasterIpv6;
#endif /* HAVE_IPV6 */
  return NULL;
}

/* Allocate new filter structure. */
static FilterT *
filterNew (void)
{
  return (FilterT *) NNMALLOC(MEM_FILTER, sizeof (FilterT));
}

static void
filterFree (FilterT *pFilter)
{
  NNFREE (MEM_FILTER, pFilter);
}

/* Return string of filterType. */
static const StringT 
filterTypeStr (FilterT *pFilter)
{
  switch (pFilter->type)
  {
    case FILTER_PERMIT:
      return "permit";
      break;
    case FILTER_DENY:
      return "deny";
      break;
    case FILTER_DYNAMIC:
      return "dynamic";
      break;
    default:
      return "";
      break;
  }
}

/* If filter match to the prefix then return 1. */
static Int32T
filterMatchCisco (FilterT *mfilter, PrefixT *pPrefix)
{
  FilterCiscoT *pFilter = NULL;
  struct in_addr mask = {0,};
  Uint32T checkAddr = 0;
  Uint32T checkMask = 0;

  pFilter = &mfilter->u.ciscoFilter;
  checkAddr = pPrefix->u.prefix4.s_addr & ~pFilter->addrMask.s_addr;

  if (pFilter->extended)
  {
    nnCnvMasklentoIp (pPrefix->prefixLen, &mask);
    checkMask = mask.s_addr & ~pFilter->maskMask.s_addr;

    if (memcmp (&checkAddr, &pFilter->addr.s_addr, 4) == 0 && 
        memcmp (&checkMask, &pFilter->mask.s_addr, 4) == 0)
      return 1;
  }
  else if (memcmp (&checkAddr, &pFilter->addr.s_addr, 4) == 0)
    return 1;

  return 0;
}

/* If filter match to the prefix then return 1. */
static Int32T
filterMatchNos (FilterT *mfilter, PrefixT *pPrefix)
{
  FilterNosT *filter = NULL;

  filter = &mfilter->u.nosFilter;

  if (filter->prefix.family == pPrefix->family)
  {
    if (filter->exact)
    {
      if (filter->prefix.prefixLen == pPrefix->prefixLen)
        return nnPrefixMatch (&filter->prefix, pPrefix);
      else
        return 0;
    }
    else
      return nnPrefixMatch (&filter->prefix, pPrefix);
  }
  else
    return 0;
}

/* Allocate new access list structure. */
static AccessListT *
accessListNew (void)
{
  return (AccessListT *) NNMALLOC (MEM_ACCESSLIST, sizeof (AccessListT));
}

/* Free allocated access_list. */
static void
accessListFree (AccessListT *pAccess)
{
  NNFREE (MEM_ACCESSLIST, pAccess);
}

/* Delete access_list from access_master and free it. */
static void
accessListDelete (AccessListT *pAccess)
{
  FilterT *pFilter = NULL;
  FilterT *pNext = NULL;
  AccessListListT *pList = NULL;
  AccessMasterT *pMaster = NULL;

  for (pFilter = pAccess->head; pFilter; pFilter = pNext)
  {
    pNext = pFilter->next;
    filterFree (pFilter);
  }

  pMaster = pAccess->master;

  if (pAccess->type == ACCESS_TYPE_NUMBER)
    pList = &pMaster->num;
  else
    pList = &pMaster->str;

  if (pAccess->next)
    pAccess->next->prev = pAccess->prev;
  else
    pList->tail = pAccess->prev;

  if (pAccess->prev)
    pAccess->prev->next = pAccess->next;
  else
    pList->head = pAccess->next;

  if (pAccess->name)
    NNFREE (MEM_ACCESSLIST_NAME, pAccess->name);

  if (pAccess->remark)
    NNFREE (MEM_ACCESSLIST_REMARK, pAccess->remark);

  accessListFree (pAccess);
}

/* Insert new access list to list of access_list.  Each acceess_list
   is sorted by the name. */
static AccessListT *
accessListInsert (afi_t afi, const StringT name)
{
  Uint32T i = 0;
  long number = 0;
  AccessListT *pAccess = NULL;
  AccessListT *pPoint = NULL;
  AccessListListT *pAList = NULL;
  AccessMasterT *pMaster = NULL;

  pMaster = accessMasterGet (afi);
  if (pMaster == NULL)
    return NULL;

  /* Allocate new access_list and copy given name. */
  pAccess = accessListNew ();
  pAccess->name = nnStrDup (name, MEM_ACCESSLIST_NAME);
  pAccess->master = pMaster;

  /* If name is made by all digit character.  We treat it as
     number. */
  for (number = 0, i = 0; i < strlen (name); i++)
  {
    if (isdigit ((Int32T) name[i]))
      number = (number * 10) + (name[i] - '0');
    else
      break;
  }

  /* In case of name is all digit character */
  if (i == strlen (name))
  {
    pAccess->type = ACCESS_TYPE_NUMBER;

    /* Set access_list to number list. */
    pAList = &pMaster->num;

    for (pPoint = pAList->head; pPoint; pPoint = pPoint->next)
      if (atol (pPoint->name) >= number)
        break;
  }
  else
  {
    pAccess->type = ACCESS_TYPE_STRING;

    /* Set access_list to string list. */
    pAList = &pMaster->str;
  
    /* Set pPoint to insertion pPoint. */
    for (pPoint = pAList->head; pPoint; pPoint = pPoint->next)
      if (strcmp (pPoint->name, name) >= 0)
        break;
  }

  /* In case of this is the first element of pMaster. */
  if (pAList->head == NULL)
  {
    pAList->head = pAList->tail = pAccess;
    return pAccess;
  }

  /* In case of insertion is made at the tail of access_list. */
  if (pPoint == NULL)
  {
    pAccess->prev = pAList->tail;
    pAList->tail->next = pAccess;
    pAList->tail = pAccess;
    return pAccess;
  }

  /* In case of insertion is made at the head of access_list. */
  if (pPoint == pAList->head)
  {
    pAccess->next = pAList->head;
    pAList->head->prev = pAccess;
    pAList->head = pAccess;
    return pAccess;
  }

  /* Insertion is made at middle of the access_list. */
  pAccess->next = pPoint;
  pAccess->prev = pPoint->prev;

  if (pPoint->prev)
    pPoint->prev->next = pAccess;
  pPoint->prev = pAccess;

  return pAccess;
}

/* Lookup access_list from list of access_list by name. */
AccessListT *
accessListLookup (afi_t afi, const StringT name)
{
  AccessListT *pAccess = NULL;
  AccessMasterT *pMaster = NULL;

  if (name == NULL)
    return NULL;

  pMaster = accessMasterGet (afi);
  if (pMaster == NULL)
    return NULL;

  for (pAccess = pMaster->num.head; pAccess; pAccess = pAccess->next)
    if (strcmp (pAccess->name, name) == 0)
      return pAccess;

  for (pAccess = pMaster->str.head; pAccess; pAccess = pAccess->next)
    if (strcmp (pAccess->name, name) == 0)
      return pAccess;

  return NULL;
}

/* Get access list from list of access_list.  If there isn't matched
   access_list create new one and return it. */
static AccessListT *
accessListGet (afi_t afi, const StringT name)
{
  AccessListT *access = NULL;

  access = accessListLookup (afi, name);
  if (access == NULL)
    access = accessListInsert (afi, name);
  return access;
}

/* Apply access list to object (which should be PrefixT *). */
eFilterTypeT
accessListApply (AccessListT *pAccess, void *pObject)
{
  FilterT *pFilter = NULL;
  PrefixT *pPrefix = NULL;

  pPrefix = (PrefixT *) pObject;

  if (pAccess == NULL)
    return FILTER_DENY;

  for (pFilter = pAccess->head; pFilter; pFilter = pFilter->next)
  {
    if (pFilter->cisco)
    {
      if (filterMatchCisco (pFilter, pPrefix))
        return pFilter->type;
    }
    else
    {
      if (filterMatchNos (pFilter, pPrefix))
        return pFilter->type;
    }
  }

  return FILTER_DENY;
}

/* Add hook function. */
void
accessListAddHook (void (*func) (AccessListT *pAccess))
{
  gAccessMasterIpv4.addHook = func;
#ifdef HAVE_IPV6
  gAccessMasterIpv6.addHook = func;
#endif /* HAVE_IPV6 */
}

/* Delete hook function. */
void
accessListDeleteHook (void (*func) (AccessListT *pAccess))
{
  gAccessMasterIpv4.deleteHook = func;
#ifdef HAVE_IPV6
  gAccessMasterIpv6.deleteHook = func;
#endif /* HAVE_IPV6 */
}

/* Add new filter to the end of specified access_list. */
static void
accessListFilterAdd (AccessListT *pAccess, FilterT *pFilter)
{
  pFilter->next = NULL;
  pFilter->prev = pAccess->tail;

  if (pAccess->tail)
    pAccess->tail->next = pFilter;
  else
    pAccess->head = pFilter;
  pAccess->tail = pFilter;

  /* Run hook function. */
  if (pAccess->master->addHook)
    (*pAccess->master->addHook) (pAccess);
}

/* If access_list has no filter then return 1. */
static Int32T
accessListEmpty (AccessListT *pAccess)
{
  if (pAccess->head == NULL && pAccess->tail == NULL)
    return 1;
  else
    return 0;
}

/* Delete filter from specified access_list.  If there is hook
   function execute it. */
static void
accessListFilterDelete (AccessListT *pAccess, FilterT *pFilter)
{
  AccessMasterT *pMaster = NULL;

  pMaster = pAccess->master;

  if (pFilter->next)
    pFilter->next->prev = pFilter->prev;
  else
    pAccess->tail = pFilter->prev;

  if (pFilter->prev)
    pFilter->prev->next = pFilter->next;
  else
    pAccess->head = pFilter->next;

  filterFree (pFilter);

  /* If access_list becomes empty delete it from access_master. */
  if (accessListEmpty (pAccess))
    accessListDelete (pAccess);

  /* Run hook function. */
  if (pMaster->deleteHook)
    (*pMaster->deleteHook) (pAccess);
}

/*
  deny    Specify packets to reject
  permit  Specify packets to forward
  dynamic ?
*/

/*
  Hostname or A.B.C.D  Address to match
  any                  Any source host
  host                 A single host address
*/

static FilterT *
filterLookupCisco (AccessListT *pAccess, FilterT *pMNew)
{
  FilterT *pMFilter = NULL;
  FilterCiscoT *pFilter = NULL;
  FilterCiscoT *pNew = NULL;

  pNew = &pMNew->u.ciscoFilter;

  for (pMFilter = pAccess->head; pMFilter; pMFilter = pMFilter->next)
  {
    pFilter = &pMFilter->u.ciscoFilter;

    if (pFilter->extended)
    {
      if (pMFilter->type == pMNew->type && 
          pFilter->addr.s_addr == pNew->addr.s_addr && 
          pFilter->addrMask.s_addr == pNew->addrMask.s_addr && 
          pFilter->mask.s_addr == pNew->mask.s_addr && 
          pFilter->maskMask.s_addr == pNew->maskMask.s_addr)
        return pMFilter;
    }
    else
    {
      if (pMFilter->type == pMNew->type && 
          pFilter->addr.s_addr == pNew->addr.s_addr && 
          pFilter->addrMask.s_addr == pNew->addrMask.s_addr)
	    return pMFilter;
    }
  }

  return NULL;
}

static FilterT *
filterLookupNos (AccessListT *pAccess, FilterT *pMNew)
{
  FilterT *pMFilter = NULL;
  FilterNosT *pFilter = NULL;
  FilterNosT *pNew = NULL;

  pNew = &pMNew->u.nosFilter;

  for (pMFilter = pAccess->head; pMFilter; pMFilter = pMFilter->next)
  {
    pFilter = &pMFilter->u.nosFilter;

    if (pFilter->exact == pNew->exact && 
        pMFilter->type == pMNew->type && 
        nnPrefixSame (&pFilter->prefix, &pNew->prefix))
      return pMFilter;
  }
  return NULL;
}

#if 0
static Int32T
vtyAccessListRemarkUnset (struct vty *vty, afi_t afi, const StringT name)
{
  AccessListT *pAccess = NULL;

  pAccess = accessListLookup (afi, name);
  if (! pAccess)
  {
      vty_out (vty, "%% access-list %s doesn't exist%s", name,
	       VTY_NEWLINE);
      return CMD_WARNING;
  }

  if (pAccess->remark)
  {
      NNFREE (MEM_ACCESSLIST_REMARK, pAccess->remark);
      pAccess->remark = NULL;
  }

  if (pAccess->head == NULL && pAccess->tail == NULL && pAccess->remark == NULL)
    accessListDelete (pAccess);

  return CMD_SUCCESS;
}
#endif

#if 0
static Int32T
filterSetCisco (struct vty *vty, const StringT strName, const StringT strType,
		  const StringT strAddr, const StringT strAddrMask,
		  const StringT strMask, const StringT strMaskMask,
		  Int32T extended, Int32T set)
{
  Int32T ret = 0;
  eFilterTypeT type;
  FilterT *pMFilter = NULL;
  FilterCiscoT *pFilter = NULL;
  AccessListT *pAccess = NULL;
  struct in_addr addr = {0,};
  struct in_addr addrMask = {0,};
  struct in_addr mask = {0,};
  struct in_addr maskMask = {0,};

  /* Check of filter type. */
  if (strncmp (strType, "p", 1) == 0)
    type = FILTER_PERMIT;
  else if (strncmp (strType, "d", 1) == 0)
    type = FILTER_DENY;
  else
    {
      vty_out (vty, "%% filter type must be permit or deny%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  ret = inet_aton (strAddr, &addr);
  if (ret <= 0)
    {
      vty_out (vty, "%%Inconsistent address and mask%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  ret = inet_aton (strAddrMask, &addrMask);
  if (ret <= 0)
    {
      vty_out (vty, "%%Inconsistent address and mask%s",
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  if (extended)
    {
      ret = inet_aton (strMask, &mask);
      if (ret <= 0)
	{
	  vty_out (vty, "%%Inconsistent address and mask%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}

      ret = inet_aton (strMaskMask, &maskMask);
      if (ret <= 0)
	{
	  vty_out (vty, "%%Inconsistent address and mask%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}
    }

  pMFilter = filterNew();
  pMFilter->type = type;
  pMFilter->cisco = 1;
  pFilter = &pMFilter->u.ciscoFilter;
  pFilter->extended = extended;
  pFilter->addr.s_addr = addr.s_addr & ~addrMask.s_addr;
  pFilter->addrMask.s_addr = addrMask.s_addr;

  if (extended)
    {
      pFilter->mask.s_addr = mask.s_addr & ~maskMask.s_addr;
      pFilter->maskMask.s_addr = maskMask.s_addr;
    }

  /* Install new filter to the access_list. */
  pAccess = accessListGet (AFI_IP, strName);

  if (set)
    {
      if (filterLookupCisco (pAccess, pMFilter))
	filterFree (pMFilter);
      else
	accessListFilterAdd (pAccess, pMFilter);
    }
  else
    {
      FilterT *delete_filter;

      delete_filter = filterLookupCisco (pAccess, pMFilter);
      if (delete_filter)
	accessListFilterDelete (pAccess, delete_filter);

      filterFree (pMFilter);
    }

  return CMD_SUCCESS;
}
#endif

#if 0
/* Standard access-list */
DEFUN (access_list_standard,
       access_list_standard_cmd,
       "access-list (<1-99>|<1300-1999>) (deny|permit) A.B.C.D A.B.C.D",
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP standard access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Address to match\n"
       "Wildcard bits\n")
{
  return filterSetCisco (vty, argv[0], argv[1], argv[2], argv[3],
			   NULL, NULL, 0, 1);
}

DEFUN (access_list_standard_nomask,
       access_list_standard_nomask_cmd,
       "access-list (<1-99>|<1300-1999>) (deny|permit) A.B.C.D",
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP standard access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Address to match\n")
{
  return filterSetCisco (vty, argv[0], argv[1], argv[2], "0.0.0.0",
			   NULL, NULL, 0, 1);
}

DEFUN (access_list_standard_host,
       access_list_standard_host_cmd,
       "access-list (<1-99>|<1300-1999>) (deny|permit) host A.B.C.D",
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP standard access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "A single host address\n"
       "Address to match\n")
{
  return filterSetCisco (vty, argv[0], argv[1], argv[2], "0.0.0.0",
			   NULL, NULL, 0, 1);
}

DEFUN (access_list_standard_any,
       access_list_standard_any_cmd,
       "access-list (<1-99>|<1300-1999>) (deny|permit) any",
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP standard access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any source host\n")
{
  return filterSetCisco (vty, argv[0], argv[1], "0.0.0.0",
			   "255.255.255.255", NULL, NULL, 0, 1);
}

DEFUN (no_access_list_standard,
       no_access_list_standard_cmd,
       "no access-list (<1-99>|<1300-1999>) (deny|permit) A.B.C.D A.B.C.D",
       NO_STR
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP standard access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Address to match\n"
       "Wildcard bits\n")
{
  return filterSetCisco (vty, argv[0], argv[1], argv[2], argv[3],
			   NULL, NULL, 0, 0);
}

DEFUN (no_access_list_standard_nomask,
       no_access_list_standard_nomask_cmd,
       "no access-list (<1-99>|<1300-1999>) (deny|permit) A.B.C.D",
       NO_STR
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP standard access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Address to match\n")
{
  return filterSetCisco (vty, argv[0], argv[1], argv[2], "0.0.0.0",
			   NULL, NULL, 0, 0);
}

DEFUN (no_access_list_standard_host,
       no_access_list_standard_host_cmd,
       "no access-list (<1-99>|<1300-1999>) (deny|permit) host A.B.C.D",
       NO_STR
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP standard access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "A single host address\n"
       "Address to match\n")
{
  return filterSetCisco (vty, argv[0], argv[1], argv[2], "0.0.0.0",
			   NULL, NULL, 0, 0);
}

DEFUN (no_access_list_standard_any,
       no_access_list_standard_any_cmd,
       "no access-list (<1-99>|<1300-1999>) (deny|permit) any",
       NO_STR
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP standard access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any source host\n")
{
  return filterSetCisco (vty, argv[0], argv[1], "0.0.0.0",
			   "255.255.255.255", NULL, NULL, 0, 0);
}

/* Extended access-list */
DEFUN (access_list_extended,
       access_list_extended_cmd,
       "access-list (<100-199>|<2000-2699>) (deny|permit) ip A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Source address\n"
       "Source wildcard bits\n"
       "Destination address\n"
       "Destination Wildcard bits\n")
{
  return filterSetCisco (vty, argv[0], argv[1], argv[2],
			   argv[3], argv[4], argv[5], 1 ,1);
}

DEFUN (access_list_extended_mask_any,
       access_list_extended_mask_any_cmd,
       "access-list (<100-199>|<2000-2699>) (deny|permit) ip A.B.C.D A.B.C.D any",
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Source address\n"
       "Source wildcard bits\n"
       "Any destination host\n")
{
  return filterSetCisco (vty, argv[0], argv[1], argv[2],
			   argv[3], "0.0.0.0",
			   "255.255.255.255", 1, 1);
}

DEFUN (access_list_extended_any_mask,
       access_list_extended_any_mask_cmd,
       "access-list (<100-199>|<2000-2699>) (deny|permit) ip any A.B.C.D A.B.C.D",
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Any source host\n"
       "Destination address\n"
       "Destination Wildcard bits\n")
{
  return filterSetCisco (vty, argv[0], argv[1], "0.0.0.0",
			   "255.255.255.255", argv[2],
			   argv[3], 1, 1);
}

DEFUN (access_list_extended_any_any,
       access_list_extended_any_any_cmd,
       "access-list (<100-199>|<2000-2699>) (deny|permit) ip any any",
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Any source host\n"
       "Any destination host\n")
{
  return filterSetCisco (vty, argv[0], argv[1], "0.0.0.0",
			   "255.255.255.255", "0.0.0.0",
			   "255.255.255.255", 1, 1);
}

DEFUN (access_list_extended_mask_host,
       access_list_extended_mask_host_cmd,
       "access-list (<100-199>|<2000-2699>) (deny|permit) ip A.B.C.D A.B.C.D host A.B.C.D",
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Source address\n"
       "Source wildcard bits\n"
       "A single destination host\n"
       "Destination address\n")
{
  return filterSetCisco (vty, argv[0], argv[1], argv[2],
			   argv[3], argv[4],
			   "0.0.0.0", 1, 1);
}

DEFUN (access_list_extended_host_mask,
       access_list_extended_host_mask_cmd,
       "access-list (<100-199>|<2000-2699>) (deny|permit) ip host A.B.C.D A.B.C.D A.B.C.D",
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "A single source host\n"
       "Source address\n"
       "Destination address\n"
       "Destination Wildcard bits\n")
{
  return filterSetCisco (vty, argv[0], argv[1], argv[2],
			   "0.0.0.0", argv[3],
			   argv[4], 1, 1);
}

DEFUN (access_list_extended_host_host,
       access_list_extended_host_host_cmd,
       "access-list (<100-199>|<2000-2699>) (deny|permit) ip host A.B.C.D host A.B.C.D",
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "A single source host\n"
       "Source address\n"
       "A single destination host\n"
       "Destination address\n")
{
  return filterSetCisco (vty, argv[0], argv[1], argv[2],
			   "0.0.0.0", argv[3],
			   "0.0.0.0", 1, 1);
}

DEFUN (access_list_extended_any_host,
       access_list_extended_any_host_cmd,
       "access-list (<100-199>|<2000-2699>) (deny|permit) ip any host A.B.C.D",
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Any source host\n"
       "A single destination host\n"
       "Destination address\n")
{
  return filterSetCisco (vty, argv[0], argv[1], "0.0.0.0",
			   "255.255.255.255", argv[2],
			   "0.0.0.0", 1, 1);
}

DEFUN (access_list_extended_host_any,
       access_list_extended_host_any_cmd,
       "access-list (<100-199>|<2000-2699>) (deny|permit) ip host A.B.C.D any",
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "A single source host\n"
       "Source address\n"
       "Any destination host\n")
{
  return filterSetCisco (vty, argv[0], argv[1], argv[2],
			   "0.0.0.0", "0.0.0.0",
			   "255.255.255.255", 1, 1);
}

DEFUN (no_access_list_extended,
       no_access_list_extended_cmd,
       "no access-list (<100-199>|<2000-2699>) (deny|permit) ip A.B.C.D A.B.C.D A.B.C.D A.B.C.D",
       NO_STR
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Source address\n"
       "Source wildcard bits\n"
       "Destination address\n"
       "Destination Wildcard bits\n")
{
  return filterSetCisco (vty, argv[0], argv[1], argv[2],
			   argv[3], argv[4], argv[5], 1, 0);
}

DEFUN (no_access_list_extended_mask_any,
       no_access_list_extended_mask_any_cmd,
       "no access-list (<100-199>|<2000-2699>) (deny|permit) ip A.B.C.D A.B.C.D any",
       NO_STR
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Source address\n"
       "Source wildcard bits\n"
       "Any destination host\n")
{
  return filterSetCisco (vty, argv[0], argv[1], argv[2],
			   argv[3], "0.0.0.0",
			   "255.255.255.255", 1, 0);
}

DEFUN (no_access_list_extended_any_mask,
       no_access_list_extended_any_mask_cmd,
       "no access-list (<100-199>|<2000-2699>) (deny|permit) ip any A.B.C.D A.B.C.D",
       NO_STR
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Any source host\n"
       "Destination address\n"
       "Destination Wildcard bits\n")
{
  return filterSetCisco (vty, argv[0], argv[1], "0.0.0.0",
			   "255.255.255.255", argv[2],
			   argv[3], 1, 0);
}

DEFUN (no_access_list_extended_any_any,
       no_access_list_extended_any_any_cmd,
       "no access-list (<100-199>|<2000-2699>) (deny|permit) ip any any",
       NO_STR
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Any source host\n"
       "Any destination host\n")
{
  return filterSetCisco (vty, argv[0], argv[1], "0.0.0.0",
			   "255.255.255.255", "0.0.0.0",
			   "255.255.255.255", 1, 0);
}

DEFUN (no_access_list_extended_mask_host,
       no_access_list_extended_mask_host_cmd,
       "no access-list (<100-199>|<2000-2699>) (deny|permit) ip A.B.C.D A.B.C.D host A.B.C.D",
       NO_STR
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Source address\n"
       "Source wildcard bits\n"
       "A single destination host\n"
       "Destination address\n")
{
  return filterSetCisco (vty, argv[0], argv[1], argv[2],
			   argv[3], argv[4],
			   "0.0.0.0", 1, 0);
}

DEFUN (no_access_list_extended_host_mask,
       no_access_list_extended_host_mask_cmd,
       "no access-list (<100-199>|<2000-2699>) (deny|permit) ip host A.B.C.D A.B.C.D A.B.C.D",
       NO_STR
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "A single source host\n"
       "Source address\n"
       "Destination address\n"
       "Destination Wildcard bits\n")
{
  return filterSetCisco (vty, argv[0], argv[1], argv[2],
			   "0.0.0.0", argv[3],
			   argv[4], 1, 0);
}

DEFUN (no_access_list_extended_host_host,
       no_access_list_extended_host_host_cmd,
       "no access-list (<100-199>|<2000-2699>) (deny|permit) ip host A.B.C.D host A.B.C.D",
       NO_STR
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "A single source host\n"
       "Source address\n"
       "A single destination host\n"
       "Destination address\n")
{
  return filterSetCisco (vty, argv[0], argv[1], argv[2],
			   "0.0.0.0", argv[3],
			   "0.0.0.0", 1, 0);
}

DEFUN (no_access_list_extended_any_host,
       no_access_list_extended_any_host_cmd,
       "no access-list (<100-199>|<2000-2699>) (deny|permit) ip any host A.B.C.D",
       NO_STR
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "Any source host\n"
       "A single destination host\n"
       "Destination address\n")
{
  return filterSetCisco (vty, argv[0], argv[1], "0.0.0.0",
			   "255.255.255.255", argv[2],
			   "0.0.0.0", 1, 0);
}

DEFUN (no_access_list_extended_host_any,
       no_access_list_extended_host_any_cmd,
       "no access-list (<100-199>|<2000-2699>) (deny|permit) ip host A.B.C.D any",
       NO_STR
       "Add an access list entry\n"
       "IP extended access list\n"
       "IP extended access list (expanded range)\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any Internet Protocol\n"
       "A single source host\n"
       "Source address\n"
       "Any destination host\n")
{
  return filterSetCisco (vty, argv[0], argv[1], argv[2],
			   "0.0.0.0", "0.0.0.0",
			   "255.255.255.255", 1, 0);
}
#endif

#if 0
static Int32T
filterSetNos (struct vty *vty, const StringT strName, const StringT strType,
              afi_t afi, const StringT strPrefix, Int32T exact, Int32T set)
{
  Int32T ret = 0;
  eFilterTypeT type;
  FilterT *pMFilter = NULL;
  FilterNosT *pFilter = NULL;
  AccessListT *pAccess = NULL;
  PrefixT p = {0,};

  /* Check of filter type. */
  if (strncmp (strType, "p", 1) == 0)
    type = FILTER_PERMIT;
  else if (strncmp (strType, "d", 1) == 0)
    type = FILTER_DENY;
  else
    {
      vty_out (vty, "filter type must be [permit|deny]%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

  /* Check string format of prefix and prefixlen. */
  if (afi == AFI_IP)
    {
      ret = str2prefix_ipv4 (strPrefix, (Prefix4T *)&p);
      if (ret <= 0)
	{
	  vty_out (vty, "IP address prefix/prefixlen is malformed%s",
		   VTY_NEWLINE);
	  return CMD_WARNING;
	}
    }
#ifdef HAVE_IPV6
  else if (afi == AFI_IP6)
    {
      ret = str2prefix_ipv6 (strPrefix, (Prefix6T *) &p);
      if (ret <= 0)
	{
	  vty_out (vty, "IPv6 address prefix/prefixlen is malformed%s",
		   VTY_NEWLINE);
		   return CMD_WARNING;
	}
    }
#endif /* HAVE_IPV6 */
  else
    return CMD_WARNING;

  pMFilter = filterNew ();
  pMFilter->type = type;
  pFilter = &pMFilter->u.nosFilter;
  prefix_copy (&pFilter->prefix, &p);

  /* "exact-match" */
  if (exact)
    filter->exact = 1;

  /* Install new filter to the access_list. */
  pAccess = accessListGet (afi, strName);

  if (set)
    {
      if (filterLookupNos (pAccess, pMFilter))
	filterFree (pMFilter);
      else
	accessListFilterAdd (pAccess, pMFilter);
    }
  else
    {
      FilterT *delete_filter;

      delete_filter = filterLookupNos (pAccess, pMFilter);
      if (delete_filter)
        accessListFilterDelete (pAccess, delete_filter);

      filterFree (pMFilter);
    }

  return CMD_SUCCESS;
}
#endif

#if 0
/* Zebra access-list */
DEFUN (access_list,
       access_list_cmd,
       "access-list WORD (deny|permit) A.B.C.D/M",
       "Add an access list entry\n"
       "IP zebra access-list name\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 10.0.0.0/8\n")
{
  return filterSetNos (vty, argv[0], argv[1], AFI_IP, argv[2], 0, 1);
}

DEFUN (access_list_exact,
       access_list_exact_cmd,
       "access-list WORD (deny|permit) A.B.C.D/M exact-match",
       "Add an access list entry\n"
       "IP zebra access-list name\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 10.0.0.0/8\n"
       "Exact match of the prefixes\n")
{
  return filterSetNos (vty, argv[0], argv[1], AFI_IP, argv[2], 1, 1);
}

DEFUN (access_list_any,
       access_list_any_cmd,
       "access-list WORD (deny|permit) any",
       "Add an access list entry\n"
       "IP zebra access-list name\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 10.0.0.0/8\n")
{
  return filterSetNos (vty, argv[0], argv[1], AFI_IP, "0.0.0.0/0", 0, 1);
}

DEFUN (no_access_list,
       no_access_list_cmd,
       "no access-list WORD (deny|permit) A.B.C.D/M",
       NO_STR
       "Add an access list entry\n"
       "IP zebra access-list name\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 10.0.0.0/8\n")
{
  return filterSetNos (vty, argv[0], argv[1], AFI_IP, argv[2], 0, 0);
}

DEFUN (no_access_list_exact,
       no_access_list_exact_cmd,
       "no access-list WORD (deny|permit) A.B.C.D/M exact-match",
       NO_STR
       "Add an access list entry\n"
       "IP zebra access-list name\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 10.0.0.0/8\n"
       "Exact match of the prefixes\n")
{
  return filterSetNos (vty, argv[0], argv[1], AFI_IP, argv[2], 1, 0);
}

DEFUN (no_access_list_any,
       no_access_list_any_cmd,
       "no access-list WORD (deny|permit) any",
       NO_STR
       "Add an access list entry\n"
       "IP zebra access-list name\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 10.0.0.0/8\n")
{
  return filterSetNos (vty, argv[0], argv[1], AFI_IP, "0.0.0.0/0", 0, 0);
}

DEFUN (no_access_list_all,
       no_access_list_all_cmd,
       "no access-list (<1-99>|<100-199>|<1300-1999>|<2000-2699>|WORD)",
       NO_STR
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP extended access list\n"
       "IP standard access list (expanded range)\n"
       "IP extended access list (expanded range)\n"
       "IP zebra access-list name\n")
{
  AccessListT *pAccess = NULL;
  AccessMasterT *pMaster = NULL;

  /* Looking up access_list. */
  pAccess = accessListLookup (AFI_IP, argv[0]);
  if (pAccess == NULL)
    {
      vty_out (vty, "%% access-list %s doesn't exist%s", argv[0],
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  pMaster = pAccess->master;

  /* Delete all filter from access-list. */
  accessListDelete (pAccess);

  /* Run hook function. */
  if (pMaster->deleteHook)
    (*pMaster->deleteHook) (pAccess);
 
  return CMD_SUCCESS;
}

DEFUN (access_list_remark,
       access_list_remark_cmd,
       "access-list (<1-99>|<100-199>|<1300-1999>|<2000-2699>|WORD) remark .LINE",
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP extended access list\n"
       "IP standard access list (expanded range)\n"
       "IP extended access list (expanded range)\n"
       "IP zebra access-list\n"
       "Access list entry comment\n"
       "Comment up to 100 characters\n")
{
  AccessListT *pAccess;

  pAccess = accessListGet (AFI_IP, argv[0]);

  if (pAccess->remark)
    {
      NNFREE (MEM_ACCESSLIST_REMARK, pAccess->remark);
      pAccess->remark = NULL;
    }
  pAccess->remark = argv_concat(argv, argc, 1);

  return CMD_SUCCESS;
}

DEFUN (no_access_list_remark,
       no_access_list_remark_cmd,
       "no access-list (<1-99>|<100-199>|<1300-1999>|<2000-2699>|WORD) remark",
       NO_STR
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP extended access list\n"
       "IP standard access list (expanded range)\n"
       "IP extended access list (expanded range)\n"
       "IP zebra access-list\n"
       "Access list entry comment\n")
{
  return vtyAccessListRemarkUnset (vty, AFI_IP, argv[0]);
}
	
ALIAS (no_access_list_remark,
       no_access_list_remark_arg_cmd,
       "no access-list (<1-99>|<100-199>|<1300-1999>|<2000-2699>|WORD) remark .LINE",
       NO_STR
       "Add an access list entry\n"
       "IP standard access list\n"
       "IP extended access list\n"
       "IP standard access list (expanded range)\n"
       "IP extended access list (expanded range)\n"
       "IP zebra access-list\n"
       "Access list entry comment\n"
       "Comment up to 100 characters\n")

#ifdef HAVE_IPV6
DEFUN (ipv6_access_list,
       ipv6_access_list_cmd,
       "ipv6 access-list WORD (deny|permit) X:X::X:X/M",
       IPV6_STR
       "Add an access list entry\n"
       "IPv6 zebra access-list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 3ffe:506::/32\n")
{
  return filterSetNos (vty, argv[0], argv[1], AFI_IP6, argv[2], 0, 1);
}

DEFUN (ipv6_access_list_exact,
       ipv6_access_list_exact_cmd,
       "ipv6 access-list WORD (deny|permit) X:X::X:X/M exact-match",
       IPV6_STR
       "Add an access list entry\n"
       "IPv6 zebra access-list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 3ffe:506::/32\n"
       "Exact match of the prefixes\n")
{
  return filterSetNos (vty, argv[0], argv[1], AFI_IP6, argv[2], 1, 1);
}

DEFUN (ipv6_access_list_any,
       ipv6_access_list_any_cmd,
       "ipv6 access-list WORD (deny|permit) any",
       IPV6_STR
       "Add an access list entry\n"
       "IPv6 zebra access-list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any prefixi to match\n")
{
  return filterSetNos (vty, argv[0], argv[1], AFI_IP6, "::/0", 0, 1);
}

DEFUN (no_ipv6_access_list,
       no_ipv6_access_list_cmd,
       "no ipv6 access-list WORD (deny|permit) X:X::X:X/M",
       NO_STR
       IPV6_STR
       "Add an access list entry\n"
       "IPv6 zebra access-list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 3ffe:506::/32\n")
{
  return filterSetNos (vty, argv[0], argv[1], AFI_IP6, argv[2], 0, 0);
}

DEFUN (no_ipv6_access_list_exact,
       no_ipv6_access_list_exact_cmd,
       "no ipv6 access-list WORD (deny|permit) X:X::X:X/M exact-match",
       NO_STR
       IPV6_STR
       "Add an access list entry\n"
       "IPv6 zebra access-list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Prefix to match. e.g. 3ffe:506::/32\n"
       "Exact match of the prefixes\n")
{
  return filterSetNos (vty, argv[0], argv[1], AFI_IP6, argv[2], 1, 0);
}

DEFUN (no_ipv6_access_list_any,
       no_ipv6_access_list_any_cmd,
       "no ipv6 access-list WORD (deny|permit) any",
       NO_STR
       IPV6_STR
       "Add an access list entry\n"
       "IPv6 zebra access-list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "Any prefixi to match\n")
{
  return filterSetNos (vty, argv[0], argv[1], AFI_IP6, "::/0", 0, 0);
}


DEFUN (no_ipv6_access_list_all,
       no_ipv6_access_list_all_cmd,
       "no ipv6 access-list WORD",
       NO_STR
       IPV6_STR
       "Add an access list entry\n"
       "IPv6 zebra access-list\n")
{
  AccessListT *pAccess = NULL;
  AccessMasterT *pMaster = NULL;

  /* Looking up access_list. */
  pAccess = accessListLookup (AFI_IP6, argv[0]);
  if (pAccess == NULL)
    {
      vty_out (vty, "%% access-list %s doesn't exist%s", argv[0],
	       VTY_NEWLINE);
      return CMD_WARNING;
    }

  pMaster = pAccess->master;

  /* Delete all filter from access-list. */
  accessListDelete (pAccess);

  /* Run hook function. */
  if (pMaster->deleteHook)
    (*pMaster->deleteHook) (pAccess);

  return CMD_SUCCESS;
}

DEFUN (ipv6_access_list_remark,
       ipv6_access_list_remark_cmd,
       "ipv6 access-list WORD remark .LINE",
       IPV6_STR
       "Add an access list entry\n"
       "IPv6 zebra access-list\n"
       "Access list entry comment\n"
       "Comment up to 100 characters\n")
{
  AccessListT *pAccess = NULL;

  pAccess = accessListGet (AFI_IP6, argv[0]);

  if (pAccess->remark)
    {
      NNFREE (MEM_ACCESSLIST_REMARK, pAccess->remark);
      pAccess->remark = NULL;
    }
  pAccess->remark = argv_concat(argv, argc, 1);

  return CMD_SUCCESS;
}

DEFUN (no_ipv6_access_list_remark,
       no_ipv6_access_list_remark_cmd,
       "no ipv6 access-list WORD remark",
       NO_STR
       IPV6_STR
       "Add an access list entry\n"
       "IPv6 zebra access-list\n"
       "Access list entry comment\n")
{
  return vtyAccessListRemarkUnset (vty, AFI_IP6, argv[0]);
}
	
ALIAS (no_ipv6_access_list_remark,
       no_ipv6_access_list_remark_arg_cmd,
       "no ipv6 access-list WORD remark .LINE",
       NO_STR
       IPV6_STR
       "Add an access list entry\n"
       "IPv6 zebra access-list\n"
       "Access list entry comment\n"
       "Comment up to 100 characters\n")
#endif /* HAVE_IPV6 */
#endif

#if 0
void configWriteAccessNos (struct vty *, FilterT *);
void configWriteAccessCisco (struct vty *, FilterT *);
#endif

#if 0
/* show access-list command. */
static Int32T
filter_show (struct vty *vty, const StringT name, afi_t afi)
{
  AccessListT *pAccess = NULL;
  AccessMasterT *pMaster = NULL;
  FilterT *pMFilter = NULL;
  FilterCiscoT *pFilter = NULL;
  Int32T write = 0;

  pMaster = accessMasterGet (afi);
  if (pMaster == NULL)
    return 0;

  /* Print the name of the protocol */
  if (zlog_default)
      vty_out (vty, "%s:%s",
      zlog_proto_names[zlog_default->protocol], VTY_NEWLINE);

  for (pAccess = pMaster->num.head; pAccess; pAccess = pAccess->next)
    {
      if (name && strcmp (pAccess->name, name) != 0)
	continue;

      write = 1;

      for (pMFilter = pAccess->head; pMFilter; pMFilter = pMFilter->next)
	{
	  pFilter = &pMFilter->u.ciscoFilter;

	  if (write)
	    {
	      vty_out (vty, "%s IP%s access list %s%s",
		       pMFilter->cisco ? 
		       (pFilter->extended ? "Extended" : "Standard") : "Zebra",
		       afi == AFI_IP6 ? "v6" : "",
		       pAccess->name, VTY_NEWLINE);
	      write = 0;
	    }

	  vty_out (vty, "    %s%s", filterTypeStr (pMFilter),
		   pMFilter->type == FILTER_DENY ? "  " : "");

	  if (! pMFilter->cisco)
	    configWriteAccessNos (vty, pMFilter);
	  else if (pFilter->extended)
	    configWriteAccessCisco (vty, pMFilter);
	  else
	    {
	      if (pFilter->addrMask.s_addr == 0xffffffff)
		vty_out (vty, " any%s", VTY_NEWLINE);
	      else
		{
		  vty_out (vty, " %s", inet_ntoa (pFilter->addr));
		  if (pFilter->addrMask.s_addr != 0)
		    vty_out (vty, ", wildcard bits %s", inet_ntoa (pFilter->addrMask));
		  vty_out (vty, "%s", VTY_NEWLINE);
		}
	    }
	}
    }

  for (pAccess = pMaster->str.head; pAccess; pAccess = pAccess->next)
    {
      if (name && strcmp (pAccess->name, name) != 0)
	continue;

      write = 1;

      for (pMFilter = pAccess->head; pMFilter; pMFilter = pMFilter->next)
	{
	  pFilter = &pMFilter->u.ciscoFilter;

	  if (write)
	    {
	      vty_out (vty, "%s IP%s access list %s%s",
		       pMFilter->cisco ? 
		       (pFilter->extended ? "Extended" : "Standard") : "Zebra",
		       afi == AFI_IP6 ? "v6" : "",
		       pAccess->name, VTY_NEWLINE);
	      write = 0;
	    }

	  vty_out (vty, "    %s%s", filterTypeStr (pMFilter),
		   pMFilter->type == FILTER_DENY ? "  " : "");

	  if (! pMFilter->cisco)
	    configWriteAccessNos (vty, pMFilter);
	  else if (pFilter->extended)
	    configWriteAccessCisco (vty, pMFilter);
	  else
	    {
	      if (pFilter->addrMask.s_addr == 0xffffffff)
		vty_out (vty, " any%s", VTY_NEWLINE);
	      else
		{
		  vty_out (vty, " %s", inet_ntoa (pFilter->addr));
		  if (pFilter->addrMask.s_addr != 0)
		    vty_out (vty, ", wildcard bits %s", inet_ntoa (pFilter->addrMask));
		  vty_out (vty, "%s", VTY_NEWLINE);
		}
	    }
	}
    }
  return CMD_SUCCESS;
}
#endif

#if 0
DEFUN (show_ip_access_list,
       show_ip_access_list_cmd,
       "show ip access-list",
       SHOW_STR
       IP_STR
       "List IP access lists\n")
{
  return filter_show (vty, NULL, AFI_IP);
}

DEFUN (show_ip_access_list_name,
       show_ip_access_list_name_cmd,
       "show ip access-list (<1-99>|<100-199>|<1300-1999>|<2000-2699>|WORD)",
       SHOW_STR
       IP_STR
       "List IP access lists\n"
       "IP standard access list\n"
       "IP extended access list\n"
       "IP standard access list (expanded range)\n"
       "IP extended access list (expanded range)\n"
       "IP zebra access-list\n")
{
  return filter_show (vty, argv[0], AFI_IP);
}

#ifdef HAVE_IPV6
DEFUN (show_ipv6_access_list,
       show_ipv6_access_list_cmd,
       "show ipv6 access-list",
       SHOW_STR
       IPV6_STR
       "List IPv6 access lists\n")
{
  return filter_show (vty, NULL, AFI_IP6);
}

DEFUN (show_ipv6_access_list_name,
       show_ipv6_access_list_name_cmd,
       "show ipv6 access-list WORD",
       SHOW_STR
       IPV6_STR
       "List IPv6 access lists\n"
       "IPv6 zebra access-list\n")
{
  return filter_show (vty, argv[0], AFI_IP6);
}
#endif /* HAVE_IPV6 */
#endif

#if 0
void
configWriteAccessCisco (struct vty *vty, FilterT *pMFilter)
{
  FilterCiscoT *pFilter = NULL;

  pFilter = &pMFilter->u.ciscoFilter;

  if (filter->extended)
    {
      vty_out (vty, " ip");
      if (pFilter->addrMask.s_addr == 0xffffffff)
	vty_out (vty, " any");
      else if (pFilter->addrMask.s_addr == 0)
	vty_out (vty, " host %s", inet_ntoa (pFilter->addr));
      else
	{
	  vty_out (vty, " %s", inet_ntoa (pFilter->addr));
	  vty_out (vty, " %s", inet_ntoa (pFilter->addrMask));
        }

      if (pFilter->maskMask.s_addr == 0xffffffff)
	vty_out (vty, " any");
      else if (pFilter->maskMask.s_addr == 0)
	vty_out (vty, " host %s", inet_ntoa (pFilter->mask));
      else
	{
	  vty_out (vty, " %s", inet_ntoa (pFilter->mask));
	  vty_out (vty, " %s", inet_ntoa (pFilter->maskMask));
	}
      vty_out (vty, "%s", VTY_NEWLINE);
    }
  else
    {
      if (pFilter->addrMask.s_addr == 0xffffffff)
	vty_out (vty, " any%s", VTY_NEWLINE);
      else
	{
	  vty_out (vty, " %s", inet_ntoa (pFilter->addr));
	  if (pFilter->addrMask.s_addr != 0)
	    vty_out (vty, " %s", inet_ntoa (pFilter->addrMask));
	  vty_out (vty, "%s", VTY_NEWLINE);
	}
    }
}
#endif

#if 0
void
configWriteAccessNos (struct vty *vty, FilterT *pMFilter)
{
  FilterNosT *pFilter = NULL;
  PrefixT *pPrefix = NULL;
  char buf[BUFSIZ] = {};

  pFilter = &pMFilter->u.nosFilter;
  pPrefix = &pFilter->prefix;

  if (pPrefix->prefixlen == 0 && ! pFilter->exact)
    vty_out (vty, " any");
  else
    vty_out (vty, " %s/%d%s",
	     inet_ntop (pPrefix->family, &pPrefix->u.prefix, buf, BUFSIZ),
	     pPrefix->prefixlen,
	     pFilter->exact ? " exact-match" : "");

  vty_out (vty, "%s", VTY_NEWLINE);
}
#endif

#if 0
static Int32T
config_write_access (struct vty *vty, afi_t afi)
{
  AccessListT *pAccess = NULL;
  AccessMasterT *pMaster = NULL;
  FilterT *pMFilter = NULL;
  Int32T write = 0;

  pMaster = accessMasterGet (afi);
  if (pMaster == NULL)
    return 0;

  for (pAccess = pMaster->num.head; pAccess; pAccess = pAccess->next)
    {
      if (pAccess->remark)
	{
	  vty_out (vty, "%saccess-list %s remark %s%s",
		   afi == AFI_IP ? "" : "ipv6 ",
		   pAccess->name, pAccess->remark,
		   VTY_NEWLINE);
	  write++;
	}

      for (pMFilter = pAccess->head; pMFilter; pMFilter = pMFilter->next)
	{
	  vty_out (vty, "%saccess-list %s %s",
	     afi == AFI_IP ? "" : "ipv6 ",
	     pAccess->name,
	     filterTypeStr (pMFilter));

	  if (pMFilter->cisco)
	    configWriteAccessCisco (vty, pMFilter);
	  else
	    configWriteAccessNos (vty, pMFilter);

	  write++;
	}
    }

  for (pAccess = pMaster->str.head; pAccess; pAccess = pAccess->next)
    {
      if (pAccess->remark)
	{
	  vty_out (vty, "%saccess-list %s remark %s%s",
		   afi == AFI_IP ? "" : "ipv6 ",
		   pAccess->name, pAccess->remark,
		   VTY_NEWLINE);
	  write++;
	}

      for (pMFilter = pAccess->head; pMFilter; pMFilter = pMFilter->next)
	{
	  vty_out (vty, "%saccess-list %s %s",
	     afi == AFI_IP ? "" : "ipv6 ",
	     pAccess->name,
	     filterTypeStr (pMFilter));

	  if (pMFilter->cisco)
	    configWriteAccessCisco (vty, pMFilter);
	  else
	    configWriteAccessNos (vty, pMFilter);

	  write++;
	}
    }
  return write;
}
#endif

#if 0
/* Access-list node. */
static struct cmd_node access_node =
{
  ACCESS_NODE,
  "",				/* Access list has no interface. */
  1
};
#endif

#if 0
static Int32T
config_write_access_ipv4 (struct vty *vty)
{
  return config_write_access (vty, AFI_IP);
}
#endif

static void
accessListResetIpv4 (void)
{
  AccessListT *pAccess = NULL;
  AccessListT *pNext = NULL;
  AccessMasterT *pMaster = NULL;

  pMaster = accessMasterGet (AFI_IP);
  if (pMaster == NULL)
    return;

  for (pAccess = pMaster->num.head; pAccess; pAccess = pNext)
    {
      pNext = pAccess->next;
      accessListDelete (pAccess);
    }
  for (pAccess = pMaster->str.head; pAccess; pAccess = pNext)
    {
      pNext = pAccess->next;
      accessListDelete (pAccess);
    }

  assert (pMaster->num.head == NULL);
  assert (pMaster->num.tail == NULL);

  assert (pMaster->str.head == NULL);
  assert (pMaster->str.tail == NULL);
}

/* Install vty related command. */
static void
accessListInitIpv4 (void)
{
#if 0
  install_node (&access_node, config_write_access_ipv4);

  install_element (ENABLE_NODE, &show_ip_access_list_cmd);
  install_element (ENABLE_NODE, &show_ip_access_list_name_cmd);

  /* Zebra access-list */
  install_element (CONFIG_NODE, &access_list_cmd);
  install_element (CONFIG_NODE, &access_list_exact_cmd);
  install_element (CONFIG_NODE, &access_list_any_cmd);
  install_element (CONFIG_NODE, &no_access_list_cmd);
  install_element (CONFIG_NODE, &no_access_list_exact_cmd);
  install_element (CONFIG_NODE, &no_access_list_any_cmd);

  /* Standard access-list */
  install_element (CONFIG_NODE, &access_list_standard_cmd);
  install_element (CONFIG_NODE, &access_list_standard_nomask_cmd);
  install_element (CONFIG_NODE, &access_list_standard_host_cmd);
  install_element (CONFIG_NODE, &access_list_standard_any_cmd);
  install_element (CONFIG_NODE, &no_access_list_standard_cmd);
  install_element (CONFIG_NODE, &no_access_list_standard_nomask_cmd);
  install_element (CONFIG_NODE, &no_access_list_standard_host_cmd);
  install_element (CONFIG_NODE, &no_access_list_standard_any_cmd);

  /* Extended access-list */
  install_element (CONFIG_NODE, &access_list_extended_cmd);
  install_element (CONFIG_NODE, &access_list_extended_any_mask_cmd);
  install_element (CONFIG_NODE, &access_list_extended_mask_any_cmd);
  install_element (CONFIG_NODE, &access_list_extended_any_any_cmd);
  install_element (CONFIG_NODE, &access_list_extended_host_mask_cmd);
  install_element (CONFIG_NODE, &access_list_extended_mask_host_cmd);
  install_element (CONFIG_NODE, &access_list_extended_host_host_cmd);
  install_element (CONFIG_NODE, &access_list_extended_any_host_cmd);
  install_element (CONFIG_NODE, &access_list_extended_host_any_cmd);
  install_element (CONFIG_NODE, &no_access_list_extended_cmd);
  install_element (CONFIG_NODE, &no_access_list_extended_any_mask_cmd);
  install_element (CONFIG_NODE, &no_access_list_extended_mask_any_cmd);
  install_element (CONFIG_NODE, &no_access_list_extended_any_any_cmd);
  install_element (CONFIG_NODE, &no_access_list_extended_host_mask_cmd);
  install_element (CONFIG_NODE, &no_access_list_extended_mask_host_cmd);
  install_element (CONFIG_NODE, &no_access_list_extended_host_host_cmd);
  install_element (CONFIG_NODE, &no_access_list_extended_any_host_cmd);
  install_element (CONFIG_NODE, &no_access_list_extended_host_any_cmd);

  install_element (CONFIG_NODE, &access_list_remark_cmd);
  install_element (CONFIG_NODE, &no_access_list_all_cmd);
  install_element (CONFIG_NODE, &no_access_list_remark_cmd);
  install_element (CONFIG_NODE, &no_access_list_remark_arg_cmd);
#endif

}

#ifdef HAVE_IPV6
static struct cmd_node access_ipv6_node =
{
  ACCESS_IPV6_NODE,
  "",
  1
};

#if 0
static Int32T
config_write_access_ipv6 (struct vty *vty)
{
  return config_write_access (vty, AFI_IP6);
}
#endif

static void
accessListResetIpv6 (void)
{
  AccessListT *pAccess = NULL;
  AccessListT *pNext = NULL;
  AccessMasterT *pMaster = NULL;

  pMaster = accessMasterGet (AFI_IP6);
  if (pMaster == NULL)
    return;

  for (pAccess = pMaster->num.head; pAccess; pAccess = pNext)
    {
      pNext = pAccess->next;
      accessListDelete (pAccess);
    }
  for (pAccess = pMaster->str.head; pAccess; pAccess = pNext)
    {
      pNext = pAccess->next;
      accessListDelete (pAccess);
    }

  assert (pMaster->num.head == NULL);
  assert (pMaster->num.tail == NULL);

  assert (pMaster->str.head == NULL);
  assert (pMaster->str.tail == NULL);
}

static void
accessListInitIpv6 (void)
{
#if 0
  install_node (&access_ipv6_node, config_write_access_ipv6);

  install_element (ENABLE_NODE, &show_ipv6_access_list_cmd);
  install_element (ENABLE_NODE, &show_ipv6_access_list_name_cmd);

  install_element (CONFIG_NODE, &ipv6_access_list_cmd);
  install_element (CONFIG_NODE, &ipv6_access_list_exact_cmd);
  install_element (CONFIG_NODE, &ipv6_access_list_any_cmd);
  install_element (CONFIG_NODE, &no_ipv6_access_list_exact_cmd);
  install_element (CONFIG_NODE, &no_ipv6_access_list_cmd);
  install_element (CONFIG_NODE, &no_ipv6_access_list_any_cmd);

  install_element (CONFIG_NODE, &no_ipv6_access_list_all_cmd);
  install_element (CONFIG_NODE, &ipv6_access_list_remark_cmd);
  install_element (CONFIG_NODE, &no_ipv6_access_list_remark_cmd);
  install_element (CONFIG_NODE, &no_ipv6_access_list_remark_arg_cmd);
#endif

}
#endif /* HAVE_IPV6 */

void
accessListInit ()
{
  accessListInitIpv4 ();

#ifdef HAVE_IPV6
  accessListInitIpv6();
#endif /* HAVE_IPV6 */
}

AccessMasterT
accessListGetMaster4 ()
{
  return gAccessMasterIpv4;
}


void
accessListVersionUpdate4 (AccessMasterT ipv4)
{
  gAccessMasterIpv4 = ipv4;
}


#ifdef HAVE_IPV6
AccessMasterT
accessListGetMaster6 ()
{
  return gAccessMasterIpv6;
}

void
accessListVersionUpdate6 (AccessMasterT ipv6)
{
  gAccessMasterIpv6 = ipv6;
}
#endif


void
accessListReset ()
{
  accessListResetIpv4 ();
#ifdef HAVE_IPV6
  accessListResetIpv6();
#endif /* HAVE_IPV6 */
}
