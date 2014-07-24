/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : 각 컴포넌트에서 사용하는 공통 Prefix list 관련 기능을 제공한다.
 * - Block Name : riblib
 * - Process Name : rib library
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : nnPlist.c
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
#include "nnPlist.h"
#include "nnBuffer.h"
#include "nosLib.h"


/* Static structure of IPv4 prefix_list's master. */
static PrefixMasterT gPrefixMasterIpv4 = 
{ 
  {NULL, NULL},
  {NULL, NULL},
  1,
  NULL,
  NULL,
};

#ifdef HAVE_IPV6
/* Static structure of IPv6 prefix-list's master. */
static PrefixMasterT gPrefixMasterIpv6 = 
{ 
  {NULL, NULL},
  {NULL, NULL},
  1,
  NULL,
  NULL,
};
#endif /* HAVE_IPV6*/

/* Static structure of BGP ORF prefix_list's master. */
static PrefixMasterT gPrefixMasterOrf = 
{ 
  {NULL, NULL},
  {NULL, NULL},
  1,
  NULL,
  NULL,
};

static PrefixMasterT *
prefixMasterGet (afi_t afi)
{
  if (afi == AFI_IP)
    return &gPrefixMasterIpv4;
#ifdef HAVE_IPV6
  else if (afi == AFI_IP6)
    return &gPrefixMasterIpv6;
#endif /* HAVE_IPV6 */
  else if (afi == AFI_ORF_PREFIX)
    return &gPrefixMasterOrf;
  return NULL;
}

/* Lookup prefix_list from list of prefix_list by name. */
PrefixListT *
prefixListLookup (afi_t afi, const StringT name)
{
  PrefixListT *pPlist = NULL;
  PrefixMasterT *pMaster = NULL;

  if (name == NULL)
    return NULL;

  pMaster = prefixMasterGet (afi);
  if (pMaster == NULL)
    return NULL;

  for (pPlist = pMaster->num.head; pPlist; pPlist = pPlist->next)
    if (strcmp (pPlist->name, name) == 0)
      return pPlist;

  for (pPlist = pMaster->str.head; pPlist; pPlist = pPlist->next)
    if (strcmp (pPlist->name, name) == 0)
      return pPlist;

  return NULL;
}

static PrefixListT *
prefixListNew (void)
{
  PrefixListT *pNew = NULL;

  pNew = NNMALLOC (MEM_PREFIXLIST, sizeof (PrefixListT));

  return pNew;
}

static void
prefixListFree (PrefixListT *pPlist)
{
  NNFREE (MEM_PREFIXLIST, pPlist);
}

static PrefixListEntryT *
prefixListEntryNew (void)
{
  PrefixListEntryT *pNew;

  pNew = NNMALLOC (MEM_PREFIXLIST_ENTRY, sizeof (PrefixListEntryT));
  return pNew;
}

static void
prefixListEntryFree (PrefixListEntryT *pPentry)
{
  NNFREE (MEM_PREFIXLIST_ENTRY, pPentry);
}

/* Insert new prefix list to list of prefix_list.  Each prefix_list
   is sorted by the name. */
static PrefixListT *
prefixListInsert (afi_t afi, const StringT name)
{
  Uint32T i = 0;
  Uint32T number = 0;
  PrefixListT *pPlist = NULL;
  PrefixListT *point = NULL;
  PrefixListListT *pPlistList = NULL;
  PrefixMasterT *pMaster = NULL;

  pMaster = prefixMasterGet (afi);
  if (pMaster == NULL)
    return NULL;

  /* Allocate new prefix_list and copy given name. */
  pPlist = prefixListNew ();
  pPlist->name = nnStrDup (name, MEM_PREFIXLIST_NAME);
  pPlist->master = pMaster;

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
    pPlist->type = PREFIX_TYPE_NUMBER;

    /* Set prefix_list to number list. */
    pPlistList = &pMaster->num;

    for (point = pPlistList->head; point; point = point->next)
      if (atol (point->name) >= number)
        break;
  }
  else
  {
    pPlist->type = PREFIX_TYPE_STRING;

    /* Set prefix_list to string list. */
    pPlistList = &pMaster->str;
  
    /* Set point to insertion point. */
    for (point = pPlistList->head; point; point = point->next)
      if (strcmp (point->name, name) >= 0)
        break;
  }

  /* In case of this is the first element of pMaster. */
  if (pPlistList->head == NULL)
  {
    pPlistList->head = pPlistList->tail = pPlist;
    return pPlist;
  }

  /* In case of insertion is made at the tail of access_list. */
  if (point == NULL)
  {
    pPlist->prev = pPlistList->tail;
    pPlistList->tail->next = pPlist;
    pPlistList->tail = pPlist;
    return pPlist;
  }

  /* In case of insertion is made at the head of access_list. */
  if (point == pPlistList->head)
  {
    pPlist->next = pPlistList->head;
    pPlistList->head->prev = pPlist;
    pPlistList->head = pPlist;
    return pPlist;
  }

  /* Insertion is made at middle of the access_list. */
  pPlist->next = point;
  pPlist->prev = point->prev;

  if (point->prev)
    point->prev->next = pPlist;
  point->prev = pPlist;

  return pPlist;
}

static PrefixListT *
prefixListGet (afi_t afi, const StringT name)
{
  PrefixListT *pPList = NULL;

  pPList = prefixListLookup (afi, name);

  if (pPList == NULL)
    pPList = prefixListInsert (afi, name);
  return pPList;
}

/* Delete prefixList from prefixListMaster and free it. */
static void
prefixListDelete (PrefixListT *pPlist)
{
  PrefixListListT *pPlistList = NULL;
  PrefixMasterT *pMaster = NULL;
  PrefixListEntryT *pPentry = NULL;
  PrefixListEntryT *pNext = NULL;

  /* If prefix-list contain prefix_list_entry free all of it. */
  for (pPentry = pPlist->head; pPentry; pPentry = pNext)
  {
    pNext = pPentry->next;
    prefixListEntryFree (pPentry);
    pPlist->count--;
  }

  pMaster = pPlist->master;

  if (pPlist->type == PREFIX_TYPE_NUMBER)
    pPlistList = &pMaster->num;
  else
    pPlistList = &pMaster->str;

  if (pPlist->next)
    pPlist->next->prev = pPlist->prev;
  else
    pPlistList->tail = pPlist->prev;

  if (pPlist->prev)
    pPlist->prev->next = pPlist->next;
  else
    pPlistList->head = pPlist->next;

  if (pPlist->desc)
    NNFREE (MEM_PREFIXLIST_DESC, pPlist->desc);

  /* Make sure pMaster's recent changed prefix-list information is
     cleared. */
  pMaster->pRecent = NULL;

  if (pPlist->name)
    NNFREE (MEM_PREFIXLIST_NAME, pPlist->name);
  
  prefixListFree (pPlist);
  
  if (pMaster->deleteHook)
    (*pMaster->deleteHook) (NULL);
}

static PrefixListEntryT *
prefixListEntryMake (PrefixT *pPrefix, ePrefixListTypeT type,
                     Int32T seq, Int32T le, Int32T ge, Int32T any)
{
  PrefixListEntryT *pPentry = NULL;

  pPentry = prefixListEntryNew ();

  if (any)
    pPentry->any = 1;

  nnPrefixCopy (&pPentry->prefix, pPrefix);
  pPentry->type = type;
  pPentry->seq = seq;
  pPentry->le = le;
  pPentry->ge = ge;

  return pPentry;
}

/* Add hook function. */
void
prefixListAddHook (void (*func) (PrefixListT *pPlist))
{
  gPrefixMasterIpv4.addHook = func;
#ifdef HAVE_IPV6
  gPrefixMasterIpv6.addHook = func;
#endif /* HAVE_IPV6 */
}

/* Delete hook function. */
void
prefixListDeleteHook (void (*func) (PrefixListT *pPlist))
{
  gPrefixMasterIpv4.deleteHook = func;
#ifdef HAVE_IPV6
  gPrefixMasterIpv6.deleteHook = func;
#endif /* HAVE_IPVt6 */
}

/* Calculate new sequential number. */
static Int32T
prefixNewSeqGet (PrefixListT *pPlist)
{
  Int32T maxseq = 0;
  Int32T newseq = 0;
  PrefixListEntryT *pPentry = NULL;

  maxseq = newseq = 0;

  for (pPentry = pPlist->head; pPentry; pPentry = pPentry->next)
    {
      if (maxseq < pPentry->seq)
	maxseq = pPentry->seq;
    }

  newseq = ((maxseq / 5) * 5) + 5;
  
  return newseq;
}

/* Return prefix list entry which has same seq number. */
static PrefixListEntryT *
prefixSeqCheck (PrefixListT *pPlist, Int32T seq)
{
  PrefixListEntryT *pPentry = NULL;

  for (pPentry = pPlist->head; pPentry; pPentry = pPentry->next)
    if (pPentry->seq == seq)
      return pPentry;
  return NULL;
}

static PrefixListEntryT *
prefixListEntryLookup (PrefixListT *pPlist, PrefixT *pPrefix,
                       ePrefixListTypeT type, 
                       Int32T seq, Int32T le, Int32T ge)
{
  PrefixListEntryT *pPentry;

  for (pPentry = pPlist->head; pPentry; pPentry = pPentry->next)
    if (nnPrefixSame (&pPentry->prefix, pPrefix) && pPentry->type == type)
    {
      if (seq >= 0 && pPentry->seq != seq)
        continue;

      if (pPentry->le != le)
        continue;
      if (pPentry->ge != ge)
        continue;

      return pPentry;
    }

  return NULL;
}

static void
prefixListEntryDelete (PrefixListT *pPlist, PrefixListEntryT *pPentry,
                       Int32T updateList)
{
  if (pPlist == NULL || pPentry == NULL)
    return;
  if (pPentry->prev)
    pPentry->prev->next = pPentry->next;
  else
    pPlist->head = pPentry->next;
  if (pPentry->next)
    pPentry->next->prev = pPentry->prev;
  else
    pPlist->tail = pPentry->prev;

  prefixListEntryFree (pPentry);

  pPlist->count--;

  if (updateList)
  {
    if (pPlist->master->deleteHook)
      (*pPlist->master->deleteHook) (pPlist);

    if (pPlist->head == NULL && 
        pPlist->tail == NULL && 
        pPlist->desc == NULL)
      prefixListDelete (pPlist);
    else
      pPlist->master->pRecent = pPlist;
  }
}

static void
prefixListEntryAdd (PrefixListT *pPlist, PrefixListEntryT *pPentry)
{
  PrefixListEntryT *pReplace = NULL;
  PrefixListEntryT *point = NULL;

  /* Automatic asignment of seq no. */
  if (pPentry->seq == -1)
    pPentry->seq = prefixNewSeqGet (pPlist);

  /* Is there any same seq prefix list entry? */
  pReplace = prefixSeqCheck (pPlist, pPentry->seq);
  if (pReplace)
    prefixListEntryDelete (pPlist, pReplace, 0);

  /* Check insert point. */
  for (point = pPlist->head; point; point = point->next)
    if (point->seq >= pPentry->seq)
      break;

  /* In case of this is the first element of the list. */
  pPentry->next = point;

  if (point)
  {
    if (point->prev)
      point->prev->next = pPentry;
    else
      pPlist->head = pPentry;

    pPentry->prev = point->prev;
    point->prev = pPentry;
  }
  else
  {
    if (pPlist->tail)
      pPlist->tail->next = pPentry;
    else
      pPlist->head = pPentry;

    pPentry->prev = pPlist->tail;
    pPlist->tail = pPentry;
  }

  /* Increment count. */
  pPlist->count++;

  /* Run hook function. */
  if (pPlist->master->addHook)
    (*pPlist->master->addHook) (pPlist);

  pPlist->master->pRecent = pPlist;
}

/* Return string of prefix_list_type. */
static const StringT 
prefixListTypeStr (PrefixListEntryT *pPentry)
{
  switch (pPentry->type)
    {
    case PREFIX_PERMIT:
      return "permit";
    case PREFIX_DENY:
      return "deny";
    default:
      return "";
    }
}

static Int32T
prefixListEntryMatch (PrefixListEntryT *pPentry, PrefixT *pPrefix)
{
  Int32T ret = 0;

  ret = nnPrefixMatch (&pPentry->prefix, pPrefix);
  if (! ret)
    return 0;
  
  /* In case of le nor ge is specified, exact match is performed. */
  if (! pPentry->le && ! pPentry->ge)
  {
    if (pPentry->prefix.prefixLen != pPrefix->prefixLen)
      return 0;
  }
  else
  {  
    if (pPentry->le)
      if (pPrefix->prefixLen > pPentry->le)
        return 0;

    if (pPentry->ge)
      if (pPrefix->prefixLen < pPentry->ge)
        return 0;
  }
  return 1;
}

ePrefixListTypeT
prefixListApply (PrefixListT *pPlist, void *pObject)
{
  PrefixListEntryT *pPentry = NULL;
  PrefixT *pPrefix = NULL;

  pPrefix = (PrefixT *) pObject;

  if (pPlist == NULL)
    return PREFIX_DENY;

  if (pPlist->count == 0)
    return PREFIX_PERMIT;

  for (pPentry = pPlist->head; pPentry; pPentry = pPentry->next)
  {
    pPentry->refCount++;
    if (prefixListEntryMatch (pPentry, pPrefix))
    {
      pPentry->hitCount++;
      return pPentry->type;
    }
  }

  return PREFIX_DENY;
}


static void __attribute__ ((unused))
prefixListPrint (PrefixListT *pPlist)
{
  PrefixListEntryT *pPentry = NULL;

  if (pPlist == NULL)
    return;

  NNLOG (LOG_DEBUG, "ip prefix-list %s: %d entries\n", pPlist->name, pPlist->count);

  for (pPentry = pPlist->head; pPentry; pPentry = pPentry->next)
  {
    if (pPentry->any)
    {
      NNLOG (LOG_DEBUG, "any %s\n", prefixListTypeStr (pPentry));
    }
    else
    {
      PrefixT *pPrefix = {0,};
      char buf[BUFSIZ]={};
	  
      pPrefix = &pPentry->prefix;
	  
      NNLOG (LOG_DEBUG, "  seq %d %s %s/%d", 
              pPentry->seq, prefixListTypeStr (pPentry),
              inet_ntop (pPrefix->family, &pPrefix->u.prefix, buf, BUFSIZ),
              pPrefix->prefixLen);
      if (pPentry->ge)
        NNLOG (LOG_DEBUG, " ge %d", pPentry->ge);
      if (pPentry->le)
        NNLOG (LOG_DEBUG, " le %d", pPentry->le);
      NNLOG (LOG_DEBUG, "\n");
    }
  }
}


/* Retrun 1 when pPlist already include pentry policy. */
static PrefixListEntryT *
prefixEntryDupCheck (PrefixListT *pPlist, PrefixListEntryT *pNew)
{
  PrefixListEntryT *pPentry = NULL;
  Int32T seq = 0;

  if (pNew->seq == -1)
    seq = prefixNewSeqGet (pPlist);
  else
    seq = pNew->seq;

  for (pPentry = pPlist->head; pPentry; pPentry = pPentry->next)
  {
    if (nnPrefixSame (&pPentry->prefix, &pNew->prefix) && 
        pPentry->type == pNew->type && pPentry->le == pNew->le && 
        pPentry->ge == pNew->ge && pPentry->seq != seq)
      return pPentry;
  }
  return NULL;
}

#if 0
static Int32T
vtyInvalidPrefixRange (struct vty *vty, const StringT strPrefix)
{
  vty_out (vty, "%% Invalid prefix range for %s, make sure: len < ge-value <= le-value%s",
           strPrefix, VTY_NEWLINE);
  return CMD_WARNING;
}
#endif

#if 0
static Int32T
vtyPrefixListInstall (struct vty *vty, afi_t afi, const StringT name, 
                      const StringT seq, const StringT strType,
                      const StringT strPrefix, const StringT ge, const StringT le)
{
  Int32T ret = 0;
  ePrefixListTypeT type = 0;
  PrefixListT *pPlist = NULL;
  PrefixListEntryT *pPentry = NULL;
  PrefixListEntryT *dup = NULL;
  PrefixT p ={0,};
  Int32T any = 0;
  Int32T seqnum = -1;
  Int32T lenum = 0;
  Int32T genum = 0;

  /* Sequential number. */
  if (seq)
    seqnum = atoi (seq);

  /* ge and le number */
  if (ge)
    genum = atoi (ge);
  if (le)
    lenum = atoi (le);

  /* Check filter type. */
  if (strncmp ("permit", strType, 1) == 0)
    type = PREFIX_PERMIT;
  else if (strncmp ("deny", strType, 1) == 0)
    type = PREFIX_DENY;
  else
  {
    vty_out (vty, "%% prefix type must be permit or deny%s", VTY_NEWLINE);
    return CMD_WARNING;
  }

  /* "any" is special token for matching any IPv4 addresses.  */
  if (afi == AFI_IP)
  {
    if (strncmp ("any", strPrefix, strlen (strPrefix)) == 0)
    {
      ret = str2prefix_ipv4 ("0.0.0.0/0", (Prefix4T *) &p);
      genum = 0;
      lenum = IPV4_MAX_BITLEN;
      any = 1;
    }
    else
      ret = str2prefix_ipv4 (strPrefix, (Prefix4T *) &p);

    if (ret <= 0)
    {
      vty_out (vty, "%% Malformed IPv4 prefix%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  }
#ifdef HAVE_IPV6
  else if (afi == AFI_IP6)
  {
    if (strncmp ("any", strPrefix, strlen (strPrefix)) == 0)
    {
      ret = str2prefix_ipv6 ("::/0", (Prefix6T *) &p);
      genum = 0;
      lenum = IPV6_MAX_BITLEN;
      any = 1;
    }
    else
      ret = str2prefix_ipv6 (strPrefix, (Prefix6T *) &p);

    if (ret <= 0)
    {
      vty_out (vty, "%% Malformed IPv6 prefix%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  }
#endif /* HAVE_IPV6 */

  /* ge and le check. */
  if (genum && genum <= p.prefixLen)
    return vtyInvalidPrefixRange (vty, strPrefix);

  if (lenum && lenum <= p.prefixLen)
    return vtyInvalidPrefixRange (vty, strPrefix);

  if (lenum && genum > lenum)
    return vtyInvalidPrefixRange (vty, strPrefix);

  if (genum && lenum == (afi == AFI_IP ? 32 : 128))
    lenum = 0;

  /* Get prefix_list with name. */
  pPlist = prefixListGet (afi, name);

  /* Make prefix entry. */
  pPentry = prefixListEntryMake (&p, type, seqnum, lenum, genum, any);
    
  /* Check same policy. */
  dup = prefixEntryDupCheck (pPlist, pPentry);

  if (dup)
  {
    prefixListEntryFree (pPentry);
    vty_out (vty, "%% Insertion failed - prefix-list entry exists:%s",
             VTY_NEWLINE);
    vty_out (vty, "   seq %d %s %s", dup->seq, strType, strPrefix);
    if (! any && genum)
      vty_out (vty, " ge %d", genum);
    if (! any && lenum)
      vty_out (vty, " le %d", lenum);
    vty_out (vty, "%s", VTY_NEWLINE);
    return CMD_WARNING;
  }

  /* Install new filter to the access_list. */
  prefixListEntryAdd (pPlist, pPentry);

  return 0;
}
#endif

#if 0
static int
vtyPrefixListUninstall (struct vty *vty, afi_t afi, const StringT name, 
                        const StringT seq, const StringT strType,
                        const StringT prefix, const StringT ge, const StringT le)
{
  Int32T ret = 0;
  ePrefixListTypeT type = {0,};
  PrefixListT *pPlist = NULL;
  PrefixListEntryT *pPentry = NULL;
  PrefixT p = {0,};
  Int32T seqnum = -1;
  Int32T lenum = 0;
  Int32T genum = 0;

  /* Check prefix list name. */
  pPlist = prefixListLookup (afi, name);
  if (! pPlist)
  {
    vty_out (vty, "%% Can't find specified prefix-list%s", VTY_NEWLINE);
    return CMD_WARNING;
  }

  /* Only prefix-list name specified, delete the entire prefix-list. */
  if (seq == NULL && strType == NULL && prefix == NULL && 
      ge == NULL && le == NULL)
  {
    prefixListDelete (pPlist);
    return CMD_SUCCESS;
  }

  /* We must have, at a minimum, both the type and prefix here */
  if ((strType == NULL) || (prefix == NULL))
  {
    vty_out (vty, "%% Both prefix and type required%s", VTY_NEWLINE);
    return CMD_WARNING;
  }

  /* Check sequence number. */
  if (seq)
    seqnum = atoi (seq);

  /* ge and le number */
  if (ge)
    genum = atoi (ge);
  if (le)
    lenum = atoi (le);

  /* Check of filter type. */
  if (strncmp ("permit", strType, 1) == 0)
    type = PREFIX_PERMIT;
  else if (strncmp ("deny", strType, 1) == 0)
    type = PREFIX_DENY;
  else
  {
    vty_out (vty, "%% prefix type must be permit or deny%s", VTY_NEWLINE);
    return CMD_WARNING;
  }

  /* "any" is special token for matching any IPv4 addresses.  */
  if (afi == AFI_IP)
  {
    if (strncmp ("any", prefix, strlen (prefix)) == 0)
    {
      ret = str2prefix_ipv4 ("0.0.0.0/0", (Prefix4T *) &p);
      genum = 0;
      lenum = IPV4_MAX_BITLEN;
    }
    else
      ret = str2prefix_ipv4 (prefix, (Prefix4T *) &p);

    if (ret <= 0)
    {
      vty_out (vty, "%% Malformed IPv4 prefix%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  }
#ifdef HAVE_IPV6
  else if (afi == AFI_IP6)
  {
    if (strncmp ("any", prefix, strlen (prefix)) == 0)
    {
      ret = str2prefix_ipv6 ("::/0", (Prefix6T *) &p);
      genum = 0;
      lenum = IPV6_MAX_BITLEN;
    }
    else
      ret = str2prefix_ipv6 (prefix, (Prefix6T *) &p);

    if (ret <= 0)
    {
      vty_out (vty, "%% Malformed IPv6 prefix%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
  }
#endif /* HAVE_IPV6 */

  /* Lookup prefix entry. */
  pPentry = prefixListEntryLookup(pPlist, &p, type, seqnum, lenum, genum);

  if (pPentry == NULL)
  {
    vty_out (vty, "%% Can't find specified prefix-list%s", VTY_NEWLINE);
    return CMD_WARNING;
  }

  /* Install new filter to the access_list. */
  prefixListEntryDelete (pPlist, pPentry, 1);

  return CMD_SUCCESS;
}
#endif

#if 0
static Int32T
vtyPrefixListDescUnset (struct vty *vty, afi_t afi, const StringT name)
{
  PrefixListT *pPlist = NULL;

  pPlist = prefixListLookup (afi, name);
  if (! pPlist)
  {
    vty_out (vty, "%% Can't find specified prefix-list%s", VTY_NEWLINE);
    return CMD_WARNING;
  }

  if (pPlist->desc)
  {
    NNFREE (MEM_PREFIXLIST_DESC, pPlist->desc);
    pPlist->desc = NULL;
  }

  if (pPlist->head == NULL && pPlist->tail == NULL && pPlist->desc == NULL)
    prefixListDelete (pPlist);

  return CMD_SUCCESS;
}
#endif

typedef enum eDisplayType
{
  normal_display,
  summary_display,
  detail_display,
  sequential_display,
  longer_display,
  first_match_display
}eDisplayTypeT;

#if 0
static void
vtyShowPrefixEntry (struct vty *vty, afi_t afi, PrefixListT *pPlist,
                    PrefixMasterT *pMaster, eDisplayTypeT dtype,
                    Int32T seqnum)
{
  PrefixListEntryT *pentry;

  /* Print the name of the protocol */
  if (zlog_default)
      vty_out (vty, "%s: ", zlog_proto_names[zlog_default->protocol]);
                                                                           
  if (dtype == normal_display)
  {
    vty_out (vty, "ip%s prefix-list %s: %d entries%s",
             afi == AFI_IP ? "" : "v6",
             pPlist->name, pPlist->count, VTY_NEWLINE);
    if (pPlist->desc)
      vty_out (vty, "   Description: %s%s", pPlist->desc, VTY_NEWLINE);
  }
  else if (dtype == summary_display || dtype == detail_display)
  {
    vty_out (vty, "ip%s prefix-list %s:%s",
             afi == AFI_IP ? "" : "v6", pPlist->name, VTY_NEWLINE);

    if (pPlist->desc)
      vty_out (vty, "   Description: %s%s", pPlist->desc, VTY_NEWLINE);

    vty_out (vty, "   count: %d, range entries: %d, sequences: %d - %d%s",
             pPlist->count, pPlist->rangecount, 
             pPlist->head ? pPlist->head->seq : 0, 
             pPlist->tail ? pPlist->tail->seq : 0,
             VTY_NEWLINE);
  }

  if (dtype != summary_display)
  {
    for (pentry = pPlist->head; pentry; pentry = pentry->next)
    {
      if (dtype == sequential_display && pentry->seq != seqnum)
        continue;

      vty_out (vty, "   ");

      if (pMaster->seqnum)
        vty_out (vty, "seq %d ", pentry->seq);

      vty_out (vty, "%s ", prefixListTypeStr (pentry));

      if (pentry->any)
        vty_out (vty, "any");
      else
      {
        PrefixT *p = &pentry->prefix;
        char buf[BUFSIZ] = {};

        vty_out (vty, "%s/%d",
                 inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ),
                 p->prefixLen);

        if (pentry->ge)
          vty_out (vty, " ge %d", pentry->ge);
        if (pentry->le)
          vty_out (vty, " le %d", pentry->le);
      }

      if (dtype == detail_display || dtype == sequential_display)
        vty_out (vty, " (hit count: %ld, refcount: %ld)", 
                 pentry->hitcnt, pentry->refcnt);

      vty_out (vty, "%s", VTY_NEWLINE);
    }
  }
}
#endif

#if 0
static Int32T
vtyShowPrefixList (struct vty *vty, afi_t afi, const StringT name,
                   const StringT seq, eDisplayTypeT dtype)
{
  PrefixListT *pPlist = NULL;
  PrefixMasterT *pMaster = NULL;
  Int32T seqnum = 0;

  pMaster = prefixMasterGet (afi);
  if (pMaster == NULL)
    return CMD_WARNING;

  if (seq)
    seqnum = atoi (seq);

  if (name)
  {
    pPlist = prefixListLookup (afi, name);
    if (! pPlist)
    {
      vty_out (vty, "%% Can't find specified prefix-list%s", VTY_NEWLINE);
      return CMD_WARNING;
    }
    vtyShowPrefixEntry (vty, afi, pPlist, pMaster, dtype, seqnum);
  }
  else
  {
    if (dtype == detail_display || dtype == summary_display)
    {
      if (pMaster->recent)
        vty_out (vty, "Prefix-list with the last deletion/insertion: %s%s",
                 pMaster->recent->name, VTY_NEWLINE);
    }

    for (pPlist = pMaster->num.head; pPlist; pPlist = pPlist->next)
      vtyShowPrefixEntry (vty, afi, pPlist, pMaster, dtype, seqnum);

    for (pPlist = pMaster->str.head; pPlist; pPlist = pPlist->next)
      vtyShowPrefixEntry (vty, afi, pPlist, pMaster, dtype, seqnum);
  }

  return CMD_SUCCESS;
}
#endif

#if 0
static Int32T
vtyShowPrefixListPrefix (struct vty *vty, afi_t afi, const StringT name, 
                         const StringT strPrefix, eDisplayTypeT type)
{
  PrefixListT *pPlist = NULL;
  PrefixListEntryT *pPentry = NULL;
  PrefixT p = {0,};
  Int32T ret = 0;
  Int32T match = 0;

  pPlist = prefixListLookup (afi, name);
  if (! pPlist)
  {
    vty_out (vty, "%% Can't find specified prefix-list%s", VTY_NEWLINE);
    return CMD_WARNING;
  }

  ret = str2prefix (strPrefix, &p);
  if (ret <= 0)
  {
    vty_out (vty, "%% prefix is malformed%s", VTY_NEWLINE);
    return CMD_WARNING;
  }

  for (pPentry = pPlist->head; pPentry; pPentry = pPentry->next)
  {
    match = 0;

    if (type == normal_display || type == first_match_display)
      if (nnPrefixSame (&p, &pPentry->prefix))
        match = 1;

    if (type == longer_display)
      if (nnPrefixMatch (&p, &pPentry->prefix))
        match = 1;

    if (match)
    {
      vty_out (vty, "   seq %d %s ", 
               pPentry->seq, prefixListTypeStr (pPentry));

      if (pPentry->any)
        vty_out (vty, "any");
      else
      {
        PrefixT *p = &pPentry->prefix;
        char buf[BUFSIZ] = {};
	      
        vty_out (vty, "%s/%d",
                 inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ),
                 p->prefixLen);

        if (pPentry->ge)
          vty_out (vty, " ge %d", pPentry->ge);
        if (pPentry->le)
          vty_out (vty, " le %d", pPentry->le);
      }
	  
      if (type == normal_display || type == first_match_display)
        vty_out (vty, " (hit count: %ld, refcount: %ld)", 
                 pPentry->hitcnt, pPentry->refcnt);

      vty_out (vty, "%s", VTY_NEWLINE);

      if (type == first_match_display)
        return CMD_SUCCESS;
    }
  }
  return CMD_SUCCESS;
}
#endif

#if 0
static Int32T
vtyClearPrefixList (struct vty *vty, afi_t afi, const StringT name, 
                    const StringT strPrefix)
{
  PrefixMasterT *pMaster = NULL;
  PrefixListT *pPlist = NULL;
  PrefixListEntryT *pPentry = NULL;
  Int32T ret = 0;
  PrefixT p = {0,};

  pMaster = prefixMasterGet (afi);
  if (pMaster == NULL)
    return CMD_WARNING;

  if (name == NULL && prefix == NULL)
  {
    for (pPlist = pMaster->num.head; pPlist; pPlist = pPlist->next)
      for (pPentry = pPlist->head; pPentry; pPentry = pPentry->next)
        pPentry->hitcnt = 0;

    for (pPlist = pMaster->str.head; pPlist; pPlist = pPlist->next)
      for (pPentry = pPlist->head; pPentry; pPentry = pPentry->next)
        pPentry->hitcnt = 0;
  }
  else
  {
    pPlist = prefixListLookup (afi, name);
    if (! pPlist)
    {
      vty_out (vty, "%% Can't find specified prefix-list%s", VTY_NEWLINE);
      return CMD_WARNING;
    }

    if (strPrefix)
    {
      ret = str2prefix (strPrefix, &p);
      if (ret <= 0)
      {
        vty_out (vty, "%% prefix is malformed%s", VTY_NEWLINE);
        return CMD_WARNING;
      }
    }

    for (pPentry = pPlist->head; pPentry; pPentry = pPentry->next)
    {
      if (strPrefix)
      {
        if (nnPrefixMatch (&pPentry->prefix, &p))
          pPentry->hitcnt = 0;
      }
      else
        pPentry->hitcnt = 0;
    }
  }
  return CMD_SUCCESS;
}
#endif

#if 0
DEFUN (ip_prefix_list,
       ip_prefix_list_cmd,
       "ip prefix-list WORD (deny|permit) (A.B.C.D/M|any)",
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Any prefix match. Same as \"0.0.0.0/0 le 32\"\n")
{
  return vtyPrefixListInstall (vty, AFI_IP, argv[0], NULL, 
                               argv[1], argv[2], NULL, NULL);
}

DEFUN (ip_prefix_list_ge,
       ip_prefix_list_ge_cmd,
       "ip prefix-list WORD (deny|permit) A.B.C.D/M ge <0-32>",
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")
{
  return vtyPrefixListInstall (vty, AFI_IP, argv[0], NULL, argv[1], 
                               argv[2], argv[3], NULL);
}

DEFUN (ip_prefix_list_ge_le,
       ip_prefix_list_ge_le_cmd,
       "ip prefix-list WORD (deny|permit) A.B.C.D/M ge <0-32> le <0-32>",
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")
{
  return vtyPrefixListInstall (vty, AFI_IP, argv[0], NULL, argv[1], 
                               argv[2], argv[3], argv[4]);
}

DEFUN (ip_prefix_list_le,
       ip_prefix_list_le_cmd,
       "ip prefix-list WORD (deny|permit) A.B.C.D/M le <0-32>",
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")
{
  return vtyPrefixListInstall (vty, AFI_IP, argv[0], NULL, argv[1],
                               argv[2], NULL, argv[3]);
}

DEFUN (ip_prefix_list_le_ge,
       ip_prefix_list_le_ge_cmd,
       "ip prefix-list WORD (deny|permit) A.B.C.D/M le <0-32> ge <0-32>",
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")
{
  return vtyPrefixListInstall (vty, AFI_IP, argv[0], NULL, argv[1],
                               argv[2], argv[4], argv[3]);
}

DEFUN (ip_prefix_list_seq,
       ip_prefix_list_seq_cmd,
       "ip prefix-list WORD seq <1-4294967295> (deny|permit) (A.B.C.D/M|any)",
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Any prefix match. Same as \"0.0.0.0/0 le 32\"\n")
{
  return vtyPrefixListInstall (vty, AFI_IP, argv[0], argv[1], argv[2],
                               argv[3], NULL, NULL);
}

DEFUN (ip_prefix_list_seq_ge,
       ip_prefix_list_seq_ge_cmd,
       "ip prefix-list WORD seq <1-4294967295> (deny|permit) A.B.C.D/M ge <0-32>",
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")
{
  return vtyPrefixListInstall (vty, AFI_IP, argv[0], argv[1], argv[2],
                               argv[3], argv[4], NULL);
}

DEFUN (ip_prefix_list_seq_ge_le,
       ip_prefix_list_seq_ge_le_cmd,
       "ip prefix-list WORD seq <1-4294967295> (deny|permit) A.B.C.D/M ge <0-32> le <0-32>",
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")
{
  return vtyPrefixListInstall (vty, AFI_IP, argv[0], argv[1], argv[2],
                               argv[3], argv[4], argv[5]);
}

DEFUN (ip_prefix_list_seq_le,
       ip_prefix_list_seq_le_cmd,
       "ip prefix-list WORD seq <1-4294967295> (deny|permit) A.B.C.D/M le <0-32>",
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")
{
  return vtyPrefixListInstall (vty, AFI_IP, argv[0], argv[1], argv[2],
                               argv[3], NULL, argv[4]);
}

DEFUN (ip_prefix_list_seq_le_ge,
       ip_prefix_list_seq_le_ge_cmd,
       "ip prefix-list WORD seq <1-4294967295> (deny|permit) A.B.C.D/M le <0-32> ge <0-32>",
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")
{
  return vtyPrefixListInstall (vty, AFI_IP, argv[0], argv[1], argv[2],
                               argv[3], argv[5], argv[4]);
}

DEFUN (no_ip_prefix_list,
       no_ip_prefix_list_cmd,
       "no ip prefix-list WORD",
       NO_STR
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP, argv[0], NULL, NULL,
                                 NULL, NULL, NULL);
}

DEFUN (no_ip_prefix_list_prefix,
       no_ip_prefix_list_prefix_cmd,
       "no ip prefix-list WORD (deny|permit) (A.B.C.D/M|any)",
       NO_STR
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Any prefix match.  Same as \"0.0.0.0/0 le 32\"\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP, argv[0], NULL, argv[1],
                                 argv[2], NULL, NULL);
}

DEFUN (no_ip_prefix_list_ge,
       no_ip_prefix_list_ge_cmd,
       "no ip prefix-list WORD (deny|permit) A.B.C.D/M ge <0-32>",
       NO_STR
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP, argv[0], NULL, argv[1],
                                 argv[2], argv[3], NULL);
}

DEFUN (no_ip_prefix_list_ge_le,
       no_ip_prefix_list_ge_le_cmd,
       "no ip prefix-list WORD (deny|permit) A.B.C.D/M ge <0-32> le <0-32>",
       NO_STR
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP, argv[0], NULL, argv[1],
                                 argv[2], argv[3], argv[4]);
}

DEFUN (no_ip_prefix_list_le,
       no_ip_prefix_list_le_cmd,
       "no ip prefix-list WORD (deny|permit) A.B.C.D/M le <0-32>",
       NO_STR
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP, argv[0], NULL, argv[1],
                                 argv[2], NULL, argv[3]);
}

DEFUN (no_ip_prefix_list_le_ge,
       no_ip_prefix_list_le_ge_cmd,
       "no ip prefix-list WORD (deny|permit) A.B.C.D/M le <0-32> ge <0-32>",
       NO_STR
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP, argv[0], NULL, argv[1],
                                 argv[2], argv[4], argv[3]);
}

DEFUN (no_ip_prefix_list_seq,
       no_ip_prefix_list_seq_cmd,
       "no ip prefix-list WORD seq <1-4294967295> (deny|permit) (A.B.C.D/M|any)",
       NO_STR
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Any prefix match.  Same as \"0.0.0.0/0 le 32\"\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP, argv[0], argv[1], argv[2],
                                 argv[3], NULL, NULL);
}

DEFUN (no_ip_prefix_list_seq_ge,
       no_ip_prefix_list_seq_ge_cmd,
       "no ip prefix-list WORD seq <1-4294967295> (deny|permit) A.B.C.D/M ge <0-32>",
       NO_STR
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP, argv[0], argv[1], argv[2],
                                 argv[3], argv[4], NULL);
}

DEFUN (no_ip_prefix_list_seq_ge_le,
       no_ip_prefix_list_seq_ge_le_cmd,
       "no ip prefix-list WORD seq <1-4294967295> (deny|permit) A.B.C.D/M ge <0-32> le <0-32>",
       NO_STR
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP, argv[0], argv[1], argv[2],
                                 argv[3], argv[4], argv[5]);
}

DEFUN (no_ip_prefix_list_seq_le,
       no_ip_prefix_list_seq_le_cmd,
       "no ip prefix-list WORD seq <1-4294967295> (deny|permit) A.B.C.D/M le <0-32>",
       NO_STR
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP, argv[0], argv[1], argv[2],
                                 argv[3], NULL, argv[4]);
}

DEFUN (no_ip_prefix_list_seq_le_ge,
       no_ip_prefix_list_seq_le_ge_cmd,
       "no ip prefix-list WORD seq <1-4294967295> (deny|permit) A.B.C.D/M le <0-32> ge <0-32>",
       NO_STR
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP, argv[0], argv[1], argv[2],
                                 argv[3], argv[5], argv[4]);
}

DEFUN (ip_prefix_list_sequence_number,
       ip_prefix_list_sequence_number_cmd,
       "ip prefix-list sequence-number",
       IP_STR
       PREFIX_LIST_STR
       "Include/exclude sequence numbers in NVGEN\n")
{
  gPrefixMasterIpv4.seqnum = 1;
  return CMD_SUCCESS;
}

DEFUN (no_ip_prefix_list_sequence_number,
       no_ip_prefix_list_sequence_number_cmd,
       "no ip prefix-list sequence-number",
       NO_STR
       IP_STR
       PREFIX_LIST_STR
       "Include/exclude sequence numbers in NVGEN\n")
{
  gPrefixMasterIpv4.seqnum = 0;
  return CMD_SUCCESS;
}

DEFUN (ip_prefix_list_description,
       ip_prefix_list_description_cmd,
       "ip prefix-list WORD description .LINE",
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Prefix-list specific description\n"
       "Up to 80 characters describing this prefix-list\n")
{
  PrefixListT *plist;

  plist = prefixListGet (AFI_IP, argv[0]);
  
  if (plist->desc)
  {
    NNFREE (MEM_PREFIXLIST_DESC, plist->desc);
    plist->desc = NULL;
  }
  plist->desc = argv_concat(argv, argc, 1);

  return CMD_SUCCESS;
}       

DEFUN (no_ip_prefix_list_description,
       no_ip_prefix_list_description_cmd,
       "no ip prefix-list WORD description",
       NO_STR
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Prefix-list specific description\n")
{
  return vtyPrefixListDescUnset (vty, AFI_IP, argv[0]);
}

ALIAS (no_ip_prefix_list_description,
       no_ip_prefix_list_description_arg_cmd,
       "no ip prefix-list WORD description .LINE",
       NO_STR
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Prefix-list specific description\n"
       "Up to 80 characters describing this prefix-list\n")

DEFUN (show_ip_prefix_list,
       show_ip_prefix_list_cmd,
       "show ip prefix-list",
       SHOW_STR
       IP_STR
       PREFIX_LIST_STR)
{
  return vtyShowPrefixList (vty, AFI_IP, NULL, NULL, normal_display);
}

DEFUN (show_ip_prefix_list_name,
       show_ip_prefix_list_name_cmd,
       "show ip prefix-list WORD",
       SHOW_STR
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n")
{
  return vtyShowPrefixList (vty, AFI_IP, argv[0], NULL, normal_display);
}

DEFUN (show_ip_prefix_list_name_seq,
       show_ip_prefix_list_name_seq_cmd,
       "show ip prefix-list WORD seq <1-4294967295>",
       SHOW_STR
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n")
{
  return vtyShowPrefixList (vty, AFI_IP, argv[0], argv[1], sequential_display);
}

DEFUN (show_ip_prefix_list_prefix,
       show_ip_prefix_list_prefix_cmd,
       "show ip prefix-list WORD A.B.C.D/M",
       SHOW_STR
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n")
{
  return vtyShowPrefixListPrefix (vty, AFI_IP, argv[0], argv[1], normal_display);
}

DEFUN (show_ip_prefix_list_prefix_longer,
       show_ip_prefix_list_prefix_longer_cmd,
       "show ip prefix-list WORD A.B.C.D/M longer",
       SHOW_STR
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "Lookup longer prefix\n")
{
  return vtyShowPrefixListPrefix (vty, AFI_IP, argv[0], argv[1], longer_display);
}

DEFUN (show_ip_prefix_list_prefix_first_match,
       show_ip_prefix_list_prefix_first_match_cmd,
       "show ip prefix-list WORD A.B.C.D/M first-match",
       SHOW_STR
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n"
       "First matched prefix\n")
{
  return vtyShowPrefixListPrefix (vty, AFI_IP, argv[0], argv[1], first_match_display);
}

DEFUN (show_ip_prefix_list_summary,
       show_ip_prefix_list_summary_cmd,
       "show ip prefix-list summary",
       SHOW_STR
       IP_STR
       PREFIX_LIST_STR
       "Summary of prefix lists\n")
{
  return vtyShowPrefixList (vty, AFI_IP, NULL, NULL, summary_display);
}

DEFUN (show_ip_prefix_list_summary_name,
       show_ip_prefix_list_summary_name_cmd,
       "show ip prefix-list summary WORD",
       SHOW_STR
       IP_STR
       PREFIX_LIST_STR
       "Summary of prefix lists\n"
       "Name of a prefix list\n")
{
  return vtyShowPrefixList (vty, AFI_IP, argv[0], NULL, summary_display);
}


DEFUN (show_ip_prefix_list_detail,
       show_ip_prefix_list_detail_cmd,
       "show ip prefix-list detail",
       SHOW_STR
       IP_STR
       PREFIX_LIST_STR
       "Detail of prefix lists\n")
{
  return vtyShowPrefixList (vty, AFI_IP, NULL, NULL, detail_display);
}

DEFUN (show_ip_prefix_list_detail_name,
       show_ip_prefix_list_detail_name_cmd,
       "show ip prefix-list detail WORD",
       SHOW_STR
       IP_STR
       PREFIX_LIST_STR
       "Detail of prefix lists\n"
       "Name of a prefix list\n")
{
  return vtyShowPrefixList (vty, AFI_IP, argv[0], NULL, detail_display);
}

DEFUN (clear_ip_prefix_list,
       clear_ip_prefix_list_cmd,
       "clear ip prefix-list",
       CLEAR_STR
       IP_STR
       PREFIX_LIST_STR)
{
  return vtyClearPrefixList (vty, AFI_IP, NULL, NULL);
}

DEFUN (clear_ip_prefix_list_name,
       clear_ip_prefix_list_name_cmd,
       "clear ip prefix-list WORD",
       CLEAR_STR
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n")
{
  return vtyClearPrefixList (vty, AFI_IP, argv[0], NULL);
}

DEFUN (clear_ip_prefix_list_name_prefix,
       clear_ip_prefix_list_name_prefix_cmd,
       "clear ip prefix-list WORD A.B.C.D/M",
       CLEAR_STR
       IP_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "IP prefix <network>/<length>, e.g., 35.0.0.0/8\n")
{
  return vtyClearPrefixList (vty, AFI_IP, argv[0], argv[1]);
}

#ifdef HAVE_IPV6
DEFUN (ipv6_prefix_list,
       ipv6_prefix_list_cmd,
       "ipv6 prefix-list WORD (deny|permit) (X:X::X:X/M|any)",
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "Any prefix match.  Same as \"::0/0 le 128\"\n")
{
  return vtyPrefixListInstall (vty, AFI_IP6, argv[0], NULL, 
                               argv[1], argv[2], NULL, NULL);
}

DEFUN (ipv6_prefix_list_ge,
       ipv6_prefix_list_ge_cmd,
       "ipv6 prefix-list WORD (deny|permit) X:X::X:X/M ge <0-128>",
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")
{
  return vtyPrefixListInstall (vty, AFI_IP6, argv[0], NULL, argv[1], 
                               argv[2], argv[3], NULL);
}

DEFUN (ipv6_prefix_list_ge_le,
       ipv6_prefix_list_ge_le_cmd,
       "ipv6 prefix-list WORD (deny|permit) X:X::X:X/M ge <0-128> le <0-128>",
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")

{
  return vtyPrefixListInstall (vty, AFI_IP6, argv[0], NULL, argv[1], 
                               argv[2], argv[3], argv[4]);
}

DEFUN (ipv6_prefix_list_le,
       ipv6_prefix_list_le_cmd,
       "ipv6 prefix-list WORD (deny|permit) X:X::X:X/M le <0-128>",
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")
{
  return vtyPrefixListInstall (vty, AFI_IP6, argv[0], NULL, argv[1],
                               argv[2], NULL, argv[3]);
}

DEFUN (ipv6_prefix_list_le_ge,
       ipv6_prefix_list_le_ge_cmd,
       "ipv6 prefix-list WORD (deny|permit) X:X::X:X/M le <0-128> ge <0-128>",
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")
{
  return vtyPrefixListInstall (vty, AFI_IP6, argv[0], NULL, argv[1],
                               argv[2], argv[4], argv[3]);
}

DEFUN (ipv6_prefix_list_seq,
       ipv6_prefix_list_seq_cmd,
       "ipv6 prefix-list WORD seq <1-4294967295> (deny|permit) (X:X::X:X/M|any)",
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "Any prefix match.  Same as \"::0/0 le 128\"\n")
{
  return vtyPrefixListInstall (vty, AFI_IP6, argv[0], argv[1], argv[2],
                               argv[3], NULL, NULL);
}

DEFUN (ipv6_prefix_list_seq_ge,
       ipv6_prefix_list_seq_ge_cmd,
       "ipv6 prefix-list WORD seq <1-4294967295> (deny|permit) X:X::X:X/M ge <0-128>",
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")
{
  return vtyPrefixListInstall (vty, AFI_IP6, argv[0], argv[1], argv[2],
                               argv[3], argv[4], NULL);
}

DEFUN (ipv6_prefix_list_seq_ge_le,
       ipv6_prefix_list_seq_ge_le_cmd,
       "ipv6 prefix-list WORD seq <1-4294967295> (deny|permit) X:X::X:X/M ge <0-128> le <0-128>",
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")
{
  return vtyPrefixListInstall (vty, AFI_IP6, argv[0], argv[1], argv[2],
                               argv[3], argv[4], argv[5]);
}

DEFUN (ipv6_prefix_list_seq_le,
       ipv6_prefix_list_seq_le_cmd,
       "ipv6 prefix-list WORD seq <1-4294967295> (deny|permit) X:X::X:X/M le <0-128>",
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")
{
  return vtyPrefixListInstall (vty, AFI_IP6, argv[0], argv[1], argv[2],
                               argv[3], NULL, argv[4]);
}

DEFUN (ipv6_prefix_list_seq_le_ge,
       ipv6_prefix_list_seq_le_ge_cmd,
       "ipv6 prefix-list WORD seq <1-4294967295> (deny|permit) X:X::X:X/M le <0-128> ge <0-128>",
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")
{
  return vtyPrefixListInstall (vty, AFI_IP6, argv[0], argv[1], argv[2],
                               argv[3], argv[5], argv[4]);
}

DEFUN (no_ipv6_prefix_list,
       no_ipv6_prefix_list_cmd,
       "no ipv6 prefix-list WORD",
       NO_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP6, argv[0], NULL, NULL,
                                 NULL, NULL, NULL);
}

DEFUN (no_ipv6_prefix_list_prefix,
       no_ipv6_prefix_list_prefix_cmd,
       "no ipv6 prefix-list WORD (deny|permit) (X:X::X:X/M|any)",
       NO_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "Any prefix match.  Same as \"::0/0 le 128\"\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP6, argv[0], NULL, argv[1],
                                 argv[2], NULL, NULL);
}

DEFUN (no_ipv6_prefix_list_ge,
       no_ipv6_prefix_list_ge_cmd,
       "no ipv6 prefix-list WORD (deny|permit) X:X::X:X/M ge <0-128>",
       NO_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP6, argv[0], NULL, argv[1],
                                 argv[2], argv[3], NULL);
}

DEFUN (no_ipv6_prefix_list_ge_le,
       no_ipv6_prefix_list_ge_le_cmd,
       "no ipv6 prefix-list WORD (deny|permit) X:X::X:X/M ge <0-128> le <0-128>",
       NO_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP6, argv[0], NULL, argv[1],
                                 argv[2], argv[3], argv[4]);
}

DEFUN (no_ipv6_prefix_list_le,
       no_ipv6_prefix_list_le_cmd,
       "no ipv6 prefix-list WORD (deny|permit) X:X::X:X/M le <0-128>",
       NO_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP6, argv[0], NULL, argv[1],
                                 argv[2], NULL, argv[3]);
}

DEFUN (no_ipv6_prefix_list_le_ge,
       no_ipv6_prefix_list_le_ge_cmd,
       "no ipv6 prefix-list WORD (deny|permit) X:X::X:X/M le <0-128> ge <0-128>",
       NO_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP6, argv[0], NULL, argv[1],
                                 argv[2], argv[4], argv[3]);
}

DEFUN (no_ipv6_prefix_list_seq,
       no_ipv6_prefix_list_seq_cmd,
       "no ipv6 prefix-list WORD seq <1-4294967295> (deny|permit) (X:X::X:X/M|any)",
       NO_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "Any prefix match.  Same as \"::0/0 le 128\"\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP6, argv[0], argv[1], argv[2],
                                 argv[3], NULL, NULL);
}

DEFUN (no_ipv6_prefix_list_seq_ge,
       no_ipv6_prefix_list_seq_ge_cmd,
       "no ipv6 prefix-list WORD seq <1-4294967295> (deny|permit) X:X::X:X/M ge <0-128>",
       NO_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP6, argv[0], argv[1], argv[2],
                                 argv[3], argv[4], NULL);
}

DEFUN (no_ipv6_prefix_list_seq_ge_le,
       no_ipv6_prefix_list_seq_ge_le_cmd,
       "no ipv6 prefix-list WORD seq <1-4294967295> (deny|permit) X:X::X:X/M ge <0-128> le <0-128>",
       NO_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP6, argv[0], argv[1], argv[2],
                                 argv[3], argv[4], argv[5]);
}

DEFUN (no_ipv6_prefix_list_seq_le,
       no_ipv6_prefix_list_seq_le_cmd,
       "no ipv6 prefix-list WORD seq <1-4294967295> (deny|permit) X:X::X:X/M le <0-128>",
       NO_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP6, argv[0], argv[1], argv[2],
                                 argv[3], NULL, argv[4]);
}

DEFUN (no_ipv6_prefix_list_seq_le_ge,
       no_ipv6_prefix_list_seq_le_ge_cmd,
       "no ipv6 prefix-list WORD seq <1-4294967295> (deny|permit) X:X::X:X/M le <0-128> ge <0-128>",
       NO_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n"
       "Specify packets to reject\n"
       "Specify packets to forward\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "Maximum prefix length to be matched\n"
       "Maximum prefix length\n"
       "Minimum prefix length to be matched\n"
       "Minimum prefix length\n")
{
  return vtyPrefixListUninstall (vty, AFI_IP6, argv[0], argv[1], argv[2],
                                 argv[3], argv[5], argv[4]);
}

DEFUN (ipv6_prefix_list_sequence_number,
       ipv6_prefix_list_sequence_number_cmd,
       "ipv6 prefix-list sequence-number",
       IPV6_STR
       PREFIX_LIST_STR
       "Include/exclude sequence numbers in NVGEN\n")
{
  gPrefixMasterIpv6.seqnum = 1;
  return CMD_SUCCESS;
}

DEFUN (no_ipv6_prefix_list_sequence_number,
       no_ipv6_prefix_list_sequence_number_cmd,
       "no ipv6 prefix-list sequence-number",
       NO_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Include/exclude sequence numbers in NVGEN\n")
{
  gPrefixMasterIpv6.seqnum = 0;
  return CMD_SUCCESS;
}

DEFUN (ipv6_prefix_list_description,
       ipv6_prefix_list_description_cmd,
       "ipv6 prefix-list WORD description .LINE",
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Prefix-list specific description\n"
       "Up to 80 characters describing this prefix-list\n")
{
  PrefixListT *pPList = NULL;

  pPList = prefixListGet (AFI_IP6, argv[0]);
  
  if (pPList->desc)
  {
    XFREE (MTYPE_TMP, pPList->desc);
    pPList->desc = NULL;
  }
  pPList->desc = argv_concat(argv, argc, 1);

  return CMD_SUCCESS;
}       

DEFUN (no_ipv6_prefix_list_description,
       no_ipv6_prefix_list_description_cmd,
       "no ipv6 prefix-list WORD description",
       NO_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Prefix-list specific description\n")
{
  return vtyPrefixListDescUnset (vty, AFI_IP6, argv[0]);
}

ALIAS (no_ipv6_prefix_list_description,
       no_ipv6_prefix_list_description_arg_cmd,
       "no ipv6 prefix-list WORD description .LINE",
       NO_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "Prefix-list specific description\n"
       "Up to 80 characters describing this prefix-list\n")

DEFUN (show_ipv6_prefix_list,
       show_ipv6_prefix_list_cmd,
       "show ipv6 prefix-list",
       SHOW_STR
       IPV6_STR
       PREFIX_LIST_STR)
{
  return vtyShowPrefixList (vty, AFI_IP6, NULL, NULL, normal_display);
}

DEFUN (show_ipv6_prefix_list_name,
       show_ipv6_prefix_list_name_cmd,
       "show ipv6 prefix-list WORD",
       SHOW_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n")
{
  return vtyShowPrefixList (vty, AFI_IP6, argv[0], NULL, normal_display);
}

DEFUN (show_ipv6_prefix_list_name_seq,
       show_ipv6_prefix_list_name_seq_cmd,
       "show ipv6 prefix-list WORD seq <1-4294967295>",
       SHOW_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "sequence number of an entry\n"
       "Sequence number\n")
{
  return vtyShowPrefixList (vty, AFI_IP6, argv[0], argv[1], sequential_display);
}

DEFUN (show_ipv6_prefix_list_prefix,
       show_ipv6_prefix_list_prefix_cmd,
       "show ipv6 prefix-list WORD X:X::X:X/M",
       SHOW_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n")
{
  return vtyShowPrefixListPrefix (vty, AFI_IP6, argv[0], argv[1], normal_display);
}

DEFUN (show_ipv6_prefix_list_prefix_longer,
       show_ipv6_prefix_list_prefix_longer_cmd,
       "show ipv6 prefix-list WORD X:X::X:X/M longer",
       SHOW_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "Lookup longer prefix\n")
{
  return vtyShowPrefixListPrefix (vty, AFI_IP6, argv[0], argv[1], longer_display);
}

DEFUN (show_ipv6_prefix_list_prefix_first_match,
       show_ipv6_prefix_list_prefix_first_match_cmd,
       "show ipv6 prefix-list WORD X:X::X:X/M first-match",
       SHOW_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n"
       "First matched prefix\n")
{
  return vtyShowPrefixListPrefix (vty, AFI_IP6, argv[0], argv[1], first_match_display);
}

DEFUN (show_ipv6_prefix_list_summary,
       show_ipv6_prefix_list_summary_cmd,
       "show ipv6 prefix-list summary",
       SHOW_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Summary of prefix lists\n")
{
  return vtyShowPrefixList (vty, AFI_IP6, NULL, NULL, summary_display);
}

DEFUN (show_ipv6_prefix_list_summary_name,
       show_ipv6_prefix_list_summary_name_cmd,
       "show ipv6 prefix-list summary WORD",
       SHOW_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Summary of prefix lists\n"
       "Name of a prefix list\n")
{
  return vtyShowPrefixList (vty, AFI_IP6, argv[0], NULL, summary_display);
}

DEFUN (show_ipv6_prefix_list_detail,
       show_ipv6_prefix_list_detail_cmd,
       "show ipv6 prefix-list detail",
       SHOW_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Detail of prefix lists\n")
{
  return vtyShowPrefixList (vty, AFI_IP6, NULL, NULL, detail_display);
}

DEFUN (show_ipv6_prefix_list_detail_name,
       show_ipv6_prefix_list_detail_name_cmd,
       "show ipv6 prefix-list detail WORD",
       SHOW_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Detail of prefix lists\n"
       "Name of a prefix list\n")
{
  return vtyShowPrefixList (vty, AFI_IP6, argv[0], NULL, detail_display);
}

DEFUN (clear_ipv6_prefix_list,
       clear_ipv6_prefix_list_cmd,
       "clear ipv6 prefix-list",
       CLEAR_STR
       IPV6_STR
       PREFIX_LIST_STR)
{
  return vtyClearPrefixList (vty, AFI_IP6, NULL, NULL);
}

DEFUN (clear_ipv6_prefix_list_name,
       clear_ipv6_prefix_list_name_cmd,
       "clear ipv6 prefix-list WORD",
       CLEAR_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n")
{
  return vtyClearPrefixList (vty, AFI_IP6, argv[0], NULL);
}

DEFUN (clear_ipv6_prefix_list_name_prefix,
       clear_ipv6_prefix_list_name_prefix_cmd,
       "clear ipv6 prefix-list WORD X:X::X:X/M",
       CLEAR_STR
       IPV6_STR
       PREFIX_LIST_STR
       "Name of a prefix list\n"
       "IPv6 prefix <network>/<length>, e.g., 3ffe::/16\n")
{
  return vtyClearPrefixList (vty, AFI_IP6, argv[0], argv[1]);
}
#endif /* HAVE_IPV6 */
#endif

#if 0
/* Configuration write function. */
static Int32T
configWritePrefixAfi (afi_t afi, struct vty *vty)
{
  PrefixListT *pPlist = NULL;
  PrefixListEntryT *pPentry = NULL;
  PrefixMasterT *pMaster = NULL;
  Int32T write = 0;

  pMaster = prefixMasterGet (afi);
  if (pMaster == NULL)
    return 0;

  if (! pMaster->seqnum)
  {
    vty_out (vty, "no ip%s prefix-list sequence-number%s", 
             afi == AFI_IP ? "" : "v6", VTY_NEWLINE);
    vty_out (vty, "!%s", VTY_NEWLINE);
  }

  for (pPlist = pMaster->num.head; pPlist; pPlist = pPlist->next)
  {
    if (pPlist->desc)
    {
      vty_out (vty, "ip%s prefix-list %s description %s%s",
               afi == AFI_IP ? "" : "v6",
               pPlist->name, pPlist->desc, VTY_NEWLINE);
      write++;
    }

    for (pPentry = pPlist->head; pPentry; pPentry = pPentry->next)
    {
      vty_out (vty, "ip%s prefix-list %s ",
               afi == AFI_IP ? "" : "v6",
               pPlist->name);

	  if (pMaster->seqnum)
        vty_out (vty, "seq %d ", pPentry->seq);
	
      vty_out (vty, "%s ", prefixListTypeStr (pPentry));

      if (pPentry->any)
        vty_out (vty, "any");
      else
      {
        PrefixT *p = &pPentry->prefix;
        char buf[BUFSIZ] = {};

        vty_out (vty, "%s/%d",
                 inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ),
                 p->prefixLen);

        if (pPentry->ge)
          vty_out (vty, " ge %d", pPentry->ge);
        if (pPentry->le)
          vty_out (vty, " le %d", pPentry->le);
      }
      vty_out (vty, "%s", VTY_NEWLINE);
      write++;
    }
    /* vty_out (vty, "!%s", VTY_NEWLINE); */
  }

  for (pPlist = pMaster->str.head; pPlist; pPlist = pPlist->next)
  {
    if (pPlist->desc)
    {
      vty_out (vty, "ip%s prefix-list %s description %s%s",
               afi == AFI_IP ? "" : "v6",
               pPlist->name, pPlist->desc, VTY_NEWLINE);
      write++;
    }

    for (pPentry = pPlist->head; pPentry; pPentry = pPentry->next)
    {
      vty_out (vty, "ip%s prefix-list %s ",
               afi == AFI_IP ? "" : "v6",
               pPlist->name);

      if (pMaster->seqnum)
        vty_out (vty, "seq %d ", pPentry->seq);

      vty_out (vty, "%s", prefixListTypeStr (pPentry));

      if (pPentry->any)
        vty_out (vty, " any");
      else
      {
        PrefixT *p = &pPentry->prefix;
        char buf[BUFSIZ] = {};

        vty_out (vty, " %s/%d",
                 inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ),
                 p->prefixLen);

        if (pPentry->ge)
          vty_out (vty, " ge %d", pPentry->ge);
        if (pPentry->le)
          vty_out (vty, " le %d", pPentry->le);
      }
      vty_out (vty, "%s", VTY_NEWLINE);
      write++;
    }
  }
  
  return write;
}
#endif

nnBufferT *
prefixBgpOrfEntry (nnBufferT * buff, PrefixListT *pPlist,
                   Uint8T initFlag, Uint8T permitFlag, Uint8T denyFlag)
{
  PrefixListEntryT *pPentry = NULL;

  if (! pPlist)
    return buff;

  for (pPentry = pPlist->head; pPentry; pPentry = pPentry->next)
  {
    Uint8T flag = initFlag;
    PrefixT *pPrefix = &pPentry->prefix;

    flag |= (pPentry->type == PREFIX_PERMIT ?  permitFlag : denyFlag);
    nnBufferSetInt8T  (buff, flag);
    nnBufferSetInt32T (buff, (Uint32T)pPentry->seq);
    nnBufferSetInt8T  (buff, (Uint8T)pPentry->ge);
    nnBufferSetInt8T  (buff, (Uint8T)pPentry->le);
    nnBufferSetPrefixT (buff, pPrefix);
  }

  return buff;
}

Int32T
prefixBgpOrfSet (StringT name, afi_t afi, OrfPrefixT *orfp,
                 Int32T permit, Int32T set)
{
  PrefixListT *pPlist = NULL;
  PrefixListEntryT *pPentry = NULL;

  /* ge and le value check */ 
  if (orfp->ge && orfp->ge <= orfp->p.prefixLen)
    return 1;
  if (orfp->le && orfp->le <= orfp->p.prefixLen)
    return 1;
  if (orfp->le && orfp->ge > orfp->le)
    return 1;

  if (orfp->ge && orfp->le == (afi == AFI_IP ? 32 : 128))
    orfp->le = 0;

  pPlist = prefixListGet (AFI_ORF_PREFIX, name);
  if (! pPlist)
    return 1;

  if (set)
  {
    pPentry = prefixListEntryMake (&orfp->p,
				       (permit ? PREFIX_PERMIT : PREFIX_DENY),
				       orfp->seq, orfp->le, orfp->ge, 0);

    if (prefixEntryDupCheck (pPlist, pPentry))
    {
      prefixListEntryFree (pPentry);
      return 1;
    }

    prefixListEntryAdd (pPlist, pPentry);
  }
  else
  {
    pPentry = prefixListEntryLookup (pPlist, &orfp->p,
					 (permit ? PREFIX_PERMIT : PREFIX_DENY),
					 orfp->seq, orfp->le, orfp->ge);

    if (! pPentry)
      return 1;

    prefixListEntryDelete (pPlist, pPentry, 1);
  }

  return 0;
}

void
prefixBgpOrfRemoveAll (StringT name)
{
  PrefixListT *pPlist = NULL;

  pPlist = prefixListLookup (AFI_ORF_PREFIX, name);
  if (pPlist)
    prefixListDelete (pPlist);
}

#if 0
/* return prefix count */
Int32T
prefixBgpShowPrefixList (struct vty *vty, afi_t afi, StringT name)
{
  PrefixListT *pPlist = NULL;
  PrefixListEntryT *pPentry = NULL;

  pPlist = prefixListLookup (AFI_ORF_PREFIX, name);
  if (! pPlist)
    return 0;

  if (! vty)
    return pPlist->count;

  vty_out (vty, "ip%s prefix-list %s: %d entries%s",
	   afi == AFI_IP ? "" : "v6",
	   pPlist->name, pPlist->count, VTY_NEWLINE);

  for (pPentry = pPlist->head; pPentry; pPentry = pPentry->next)
  {
    PrefixT *p = &pPentry->prefix;
    char buf[BUFSIZ];

    vty_out (vty, "   seq %d %s %s/%d", pPentry->seq,
             prefixListTypeStr (pPentry),
             inet_ntop (p->family, &p->u.prefix, buf, BUFSIZ),
             p->prefixLen);

    if (pPentry->ge)
      vty_out (vty, " ge %d", pPentry->ge);
    if (pPentry->le)
      vty_out (vty, " le %d", pPentry->le);

    vty_out (vty, "%s", VTY_NEWLINE);
  }
  return pPlist->count;
}
#endif

static void
prefixListResetOrf (void)
{
  PrefixListT *pPlist = NULL;
  PrefixListT *pNext = NULL;
  PrefixMasterT *pMaster = NULL;

  pMaster = prefixMasterGet (AFI_ORF_PREFIX);
  if (pMaster == NULL)
    return;

  for (pPlist = pMaster->num.head; pPlist; pPlist = pNext)
  {
    pNext = pPlist->next;
    prefixListDelete (pPlist);
  }
  for (pPlist = pMaster->str.head; pPlist; pPlist = pNext)
  {
    pNext = pPlist->next;
    prefixListDelete (pPlist);
  }

  assert (pMaster->num.head == NULL);
  assert (pMaster->num.tail == NULL);

  assert (pMaster->str.head == NULL);
  assert (pMaster->str.tail == NULL);

  pMaster->seqNum = 1;
  pMaster->pRecent = NULL;
}

#if 0
/* Prefix-list node. */
static struct cmd_node prefix_node =
{
  PREFIX_NODE,
  "",				/* Prefix list has no interface. */
  1
};
#endif

#if 0
static Int32T
configWritePrefixIpv4 (struct vty *vty)
{
  return configWritePrefixAfi (AFI_IP, vty);
}
#endif

static void
prefixListResetIpv4 (void)
{
  PrefixListT *pPlist = NULL;
  PrefixListT *pNext = NULL;
  PrefixMasterT *pMaster = NULL;

  pMaster = prefixMasterGet (AFI_IP);
  if (pMaster == NULL)
    return;

  for (pPlist = pMaster->num.head; pPlist; pPlist = pNext)
    {
      pNext = pPlist->next;
      prefixListDelete (pPlist);
    }
  for (pPlist = pMaster->str.head; pPlist; pPlist = pNext)
    {
      pNext = pPlist->next;
      prefixListDelete (pPlist);
    }

  assert (pMaster->num.head == NULL);
  assert (pMaster->num.tail == NULL);

  assert (pMaster->str.head == NULL);
  assert (pMaster->str.tail == NULL);

  pMaster->seqNum = 1;
  pMaster->pRecent = NULL;
}

static void
prefixListInitIpv4 (void)
{
#if 0
  install_node (&prefix_node, configWritePrefixIpv4);

  install_element (CONFIG_NODE, &ip_prefix_list_cmd);
  install_element (CONFIG_NODE, &ip_prefix_list_ge_cmd);
  install_element (CONFIG_NODE, &ip_prefix_list_ge_le_cmd);
  install_element (CONFIG_NODE, &ip_prefix_list_le_cmd);
  install_element (CONFIG_NODE, &ip_prefix_list_le_ge_cmd);
  install_element (CONFIG_NODE, &ip_prefix_list_seq_cmd);
  install_element (CONFIG_NODE, &ip_prefix_list_seq_ge_cmd);
  install_element (CONFIG_NODE, &ip_prefix_list_seq_ge_le_cmd);
  install_element (CONFIG_NODE, &ip_prefix_list_seq_le_cmd);
  install_element (CONFIG_NODE, &ip_prefix_list_seq_le_ge_cmd);

  install_element (CONFIG_NODE, &no_ip_prefix_list_cmd);
  install_element (CONFIG_NODE, &no_ip_prefix_list_prefix_cmd);
  install_element (CONFIG_NODE, &no_ip_prefix_list_ge_cmd);
  install_element (CONFIG_NODE, &no_ip_prefix_list_ge_le_cmd);
  install_element (CONFIG_NODE, &no_ip_prefix_list_le_cmd);
  install_element (CONFIG_NODE, &no_ip_prefix_list_le_ge_cmd);
  install_element (CONFIG_NODE, &no_ip_prefix_list_seq_cmd);
  install_element (CONFIG_NODE, &no_ip_prefix_list_seq_ge_cmd);
  install_element (CONFIG_NODE, &no_ip_prefix_list_seq_ge_le_cmd);
  install_element (CONFIG_NODE, &no_ip_prefix_list_seq_le_cmd);
  install_element (CONFIG_NODE, &no_ip_prefix_list_seq_le_ge_cmd);

  install_element (CONFIG_NODE, &ip_prefix_list_description_cmd);
  install_element (CONFIG_NODE, &no_ip_prefix_list_description_cmd);
  install_element (CONFIG_NODE, &no_ip_prefix_list_description_arg_cmd);

  install_element (CONFIG_NODE, &ip_prefix_list_sequence_number_cmd);
  install_element (CONFIG_NODE, &no_ip_prefix_list_sequence_number_cmd);

  install_element (VIEW_NODE, &show_ip_prefix_list_cmd);
  install_element (VIEW_NODE, &show_ip_prefix_list_name_cmd);
  install_element (VIEW_NODE, &show_ip_prefix_list_name_seq_cmd);
  install_element (VIEW_NODE, &show_ip_prefix_list_prefix_cmd);
  install_element (VIEW_NODE, &show_ip_prefix_list_prefix_longer_cmd);
  install_element (VIEW_NODE, &show_ip_prefix_list_prefix_first_match_cmd);
  install_element (VIEW_NODE, &show_ip_prefix_list_summary_cmd);
  install_element (VIEW_NODE, &show_ip_prefix_list_summary_name_cmd);
  install_element (VIEW_NODE, &show_ip_prefix_list_detail_cmd);
  install_element (VIEW_NODE, &show_ip_prefix_list_detail_name_cmd);

  install_element (ENABLE_NODE, &show_ip_prefix_list_cmd);
  install_element (ENABLE_NODE, &show_ip_prefix_list_name_cmd);
  install_element (ENABLE_NODE, &show_ip_prefix_list_name_seq_cmd);
  install_element (ENABLE_NODE, &show_ip_prefix_list_prefix_cmd);
  install_element (ENABLE_NODE, &show_ip_prefix_list_prefix_longer_cmd);
  install_element (ENABLE_NODE, &show_ip_prefix_list_prefix_first_match_cmd);
  install_element (ENABLE_NODE, &show_ip_prefix_list_summary_cmd);
  install_element (ENABLE_NODE, &show_ip_prefix_list_summary_name_cmd);
  install_element (ENABLE_NODE, &show_ip_prefix_list_detail_cmd);
  install_element (ENABLE_NODE, &show_ip_prefix_list_detail_name_cmd);

  install_element (ENABLE_NODE, &clear_ip_prefix_list_cmd);
  install_element (ENABLE_NODE, &clear_ip_prefix_list_name_cmd);
  install_element (ENABLE_NODE, &clear_ip_prefix_list_name_prefix_cmd);
#endif

}

#ifdef HAVE_IPV6
/* Prefix-list node. */
static struct cmd_node prefix_ipv6_node =
{
  PREFIX_IPV6_NODE,
  "",				/* Prefix list has no interface. */
  1
};

static Int32T
configWritePrefixIpv6 (struct vty *vty)
{
  return configWritePrefixAfi (AFI_IP6, vty);
}

static void
prefixListResetIpv6 (void)
{
  PrefixListT *pPlist = NULL;
  PrefixListT *pNext = NULL;
  PrefixMasterT *pMaster = NULL;

  pMaster = prefixMasterGet (AFI_IP6);
  if (pMaster == NULL)
    return;

  for (pPlist = pMaster->num.head; pPlist; pPlist = pNext)
    {
      pNext = pPlist->next;
      prefixListDelete (pPlist);
    }
  for (pPlist = pMaster->str.head; pPlist; pPlist = pNext)
    {
      pNext = pPlist->next;
      prefixListDelete (pPlist);
    }

  assert (pMaster->num.head == NULL);
  assert (pMaster->num.tail == NULL);

  assert (pMaster->str.head == NULL);
  assert (pMaster->str.tail == NULL);

  pMaster->seqnum = 1;
  pMaster->recent = NULL;
}

static void
prefixListInitIpv6 (void)
{
  install_node (&prefix_ipv6_node, configWritePrefixIpv6);

  install_element (CONFIG_NODE, &ipv6_prefix_list_cmd);
  install_element (CONFIG_NODE, &ipv6_prefix_list_ge_cmd);
  install_element (CONFIG_NODE, &ipv6_prefix_list_ge_le_cmd);
  install_element (CONFIG_NODE, &ipv6_prefix_list_le_cmd);
  install_element (CONFIG_NODE, &ipv6_prefix_list_le_ge_cmd);
  install_element (CONFIG_NODE, &ipv6_prefix_list_seq_cmd);
  install_element (CONFIG_NODE, &ipv6_prefix_list_seq_ge_cmd);
  install_element (CONFIG_NODE, &ipv6_prefix_list_seq_ge_le_cmd);
  install_element (CONFIG_NODE, &ipv6_prefix_list_seq_le_cmd);
  install_element (CONFIG_NODE, &ipv6_prefix_list_seq_le_ge_cmd);

  install_element (CONFIG_NODE, &no_ipv6_prefix_list_cmd);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_prefix_cmd);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_ge_cmd);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_ge_le_cmd);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_le_cmd);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_le_ge_cmd);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_seq_cmd);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_seq_ge_cmd);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_seq_ge_le_cmd);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_seq_le_cmd);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_seq_le_ge_cmd);

  install_element (CONFIG_NODE, &ipv6_prefix_list_description_cmd);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_description_cmd);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_description_arg_cmd);

  install_element (CONFIG_NODE, &ipv6_prefix_list_sequence_number_cmd);
  install_element (CONFIG_NODE, &no_ipv6_prefix_list_sequence_number_cmd);

  install_element (VIEW_NODE, &show_ipv6_prefix_list_cmd);
  install_element (VIEW_NODE, &show_ipv6_prefix_list_name_cmd);
  install_element (VIEW_NODE, &show_ipv6_prefix_list_name_seq_cmd);
  install_element (VIEW_NODE, &show_ipv6_prefix_list_prefix_cmd);
  install_element (VIEW_NODE, &show_ipv6_prefix_list_prefix_longer_cmd);
  install_element (VIEW_NODE, &show_ipv6_prefix_list_prefix_first_match_cmd);
  install_element (VIEW_NODE, &show_ipv6_prefix_list_summary_cmd);
  install_element (VIEW_NODE, &show_ipv6_prefix_list_summary_name_cmd);
  install_element (VIEW_NODE, &show_ipv6_prefix_list_detail_cmd);
  install_element (VIEW_NODE, &show_ipv6_prefix_list_detail_name_cmd);

  install_element (ENABLE_NODE, &show_ipv6_prefix_list_cmd);
  install_element (ENABLE_NODE, &show_ipv6_prefix_list_name_cmd);
  install_element (ENABLE_NODE, &show_ipv6_prefix_list_name_seq_cmd);
  install_element (ENABLE_NODE, &show_ipv6_prefix_list_prefix_cmd);
  install_element (ENABLE_NODE, &show_ipv6_prefix_list_prefix_longer_cmd);
  install_element (ENABLE_NODE, &show_ipv6_prefix_list_prefix_first_match_cmd);
  install_element (ENABLE_NODE, &show_ipv6_prefix_list_summary_cmd);
  install_element (ENABLE_NODE, &show_ipv6_prefix_list_summary_name_cmd);
  install_element (ENABLE_NODE, &show_ipv6_prefix_list_detail_cmd);
  install_element (ENABLE_NODE, &show_ipv6_prefix_list_detail_name_cmd);

  install_element (ENABLE_NODE, &clear_ipv6_prefix_list_cmd);
  install_element (ENABLE_NODE, &clear_ipv6_prefix_list_name_cmd);
  install_element (ENABLE_NODE, &clear_ipv6_prefix_list_name_prefix_cmd);
}
#endif /* HAVE_IPV6 */

void
prefixListInit ()
{
  prefixListInitIpv4 ();
#ifdef HAVE_IPV6
  prefixListInitIpv6 ();
#endif /* HAVE_IPV6 */
}


PrefixMasterT
prefixListGetMaster4 ()
{
  return gPrefixMasterIpv4;
}


void
prefixListVersionUpdate4 (PrefixMasterT ipv4)
{
  gPrefixMasterIpv4 = ipv4;
}


#ifdef HAVE_IPV6
PrefixMasterT
prefixListGetMaster6 ()
{
  return gPrefixMasterIpv6;
}


void
prefixListVersionUpdate6 (PrefixMasterT ipv6)
{
  gPrefixMasterIpv6 = ipv6;
}
#endif



void
prefixListReset ()
{
  prefixListResetIpv4 ();
#ifdef HAVE_IPV6
  prefixListResetIpv6 ();
#endif /* HAVE_IPV6 */
  prefixListResetOrf ();
}
