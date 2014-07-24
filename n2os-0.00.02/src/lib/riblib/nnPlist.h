/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the prefix list related definitions.
 *  - Block Name : riblib
 *  - Process Name : rib library
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : nnPlist.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_nnPlist_h)
#define _nnPlist_h

#include "nnTypes.h"
#include "nnPrefix.h"
#include "nnBuffer.h"
#include "nnRibDefines.h"


#define AFI_ORF_PREFIX 65535


typedef enum ePrefixListType 
{
  PREFIX_DENY,
  PREFIX_PERMIT,
}ePrefixListTypeT;

typedef enum ePrefixNameType
{
  PREFIX_TYPE_STRING,
  PREFIX_TYPE_NUMBER
}ePrefixNameTypeT;

struct PrefixList
{
  StringT name;
  StringT desc;

  struct PrefixMaster *master;

  ePrefixNameTypeT type;

  Int32T count;
  Int32T rangecount;

  struct PrefixListEntry *head;
  struct PrefixListEntry *tail;

  struct PrefixList *next;
  struct PrefixList *prev;
};
typedef struct PrefixList PrefixListT;

struct OrfPrefix
{
  Uint32T seq;
  Uint8T ge;
  Uint8T le;
  PrefixT p;
};
typedef struct OrfPrefix OrfPrefixT;

/* Each prefix-list's entry. */
struct PrefixListEntry
{
  Int32T seq; 

  Int32T le;
  Int32T ge;

  ePrefixListTypeT type;

  Int32T any; 
  PrefixT prefix;

  Uint32T refCount;
  Uint32T hitCount;

  struct PrefixListEntry *next;
  struct PrefixListEntry *prev;
};
typedef struct PrefixListEntry PrefixListEntryT;

/* List of PrefixListT. */
struct PrefixListList
{
  PrefixListT *head;
  PrefixListT *tail;
};
typedef struct PrefixListList PrefixListListT;

/* Master structure of prefix_list. */
struct PrefixMaster
{
  /* List of prefix_list which name is number. */
  PrefixListListT num; 

  /* List of prefix_list which name is string. */
  PrefixListListT str; 

  /* Whether sequential number is used. */
  Int32T seqNum;

  /* The latest update. */
  PrefixListT *pRecent;

  /* Hook function which is executed when new prefix_list is added. */
  void (*addHook) (PrefixListT *);

  /* Hook function which is executed when prefix_list is deleted. */
  void (*deleteHook) (PrefixListT *);
};
typedef struct PrefixMaster PrefixMasterT;

/* Prototypes. */
extern void prefixListInit (void);
extern PrefixMasterT prefixListGetMaster4 ();
extern void prefixListVersionUpdate4 (PrefixMasterT);
#ifdef HAVE_IPV6
extern PrefixMasterT prefixListGetMaster6 ();
extern void prefixListVersionUpdate6 (PrefixMasterT);
#endif
extern void prefixListReset (void);
extern void prefixListAddHook (void (*func) (PrefixListT *));
extern void prefixListDeleteHook (void (*func) (PrefixListT *));

extern PrefixListT *prefixListLookup (afi_t, const StringT);
extern ePrefixListTypeT prefixListApply (PrefixListT *, void *);

extern nnBufferT * prefixBgpOrfEntry (nnBufferT *, PrefixListT *,
                                      Uint8T, Uint8T, Uint8T);
extern int prefixBgpOrfSet (char *, afi_t, OrfPrefixT *, Int32T, Int32T);
extern void prefixBgpOrfRemoveAll (char *);
//extern int prefixBgpShowPrefixList (struct vty *, afi_t, char *);

#endif /* _nnPlist_h */
