/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the filter related definitions.
 *  - Block Name : riblib
 *  - Process Name : rib library
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : nnFilter.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_nnFilter_h)
#define _nnFilter_h

/**   
 * @brief : This file defines the access list related definitions.
 *  - Block Name : riblib
 *  - Process Name : riblib
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2014/04/21
 */

/**
 * @file : nnFilter.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include "nnIf.h"
#include "nnRibDefines.h"

/* Filter type is made by `permit', `deny' and `dynamic'. */
typedef enum eFilterType 
{
  FILTER_DENY,
  FILTER_PERMIT,
  FILTER_DYNAMIC
}eFilterTypeT;

typedef enum eAccessType
{
  ACCESS_TYPE_STRING,
  ACCESS_TYPE_NUMBER
}eAccessTypeT;

/* Access list */
struct AccessList
{
  StringT name;
  char *remark;

  struct AccessMaster *master;

  eAccessTypeT type;

  struct AccessList *next;
  struct AccessList *prev;

  struct Filter *head;
  struct Filter *tail;
};
typedef struct AccessList AccessListT;

//
struct FilterCisco
{
  /* Cisco access-list */
  int extended;
  struct in_addr addr;
  struct in_addr addrMask;
  struct in_addr mask;
  struct in_addr maskMask;
};
typedef struct FilterCisco FilterCiscoT;

struct FilterNos
{
  /* If this filter is "exact" match then this flag is set. */
  int exact;

  /* Prefix information. */
  PrefixT prefix;
};
typedef struct FilterNos FilterNosT;


/* Filter element of access list */
struct Filter
{
  /* For doubly linked list. */
  struct Filter *next;
  struct Filter *prev;

  /* Filter type information. */
  eFilterTypeT type;

  /* Cisco access-list */
  int cisco;

  union
  {
    FilterCiscoT ciscoFilter;
    FilterNosT nosFilter;
  } u; 
};
typedef struct Filter FilterT;


/* List of AccessList. */
struct AccessListList
{
  AccessListT *head;
  AccessListT *tail;
};
typedef struct AccessListList AccessListListT;


/* Master structure of AccessList. */
struct AccessMaster
{
  /* List of AccessList which name is number. */
  struct AccessListList num; 

  /* List of AccessList which name is string. */
  struct AccessListList str; 

  /* Hook function which is executed when new AccessList is added. */
  void (*addHook) (struct AccessList *);

  /* Hook function which is executed when AccessList is deleted. */
  void (*deleteHook) (struct AccessList *);
};
typedef struct AccessMaster AccessMasterT;
//

/* Prototypes for access-list. */
extern void accessListInit (void);
extern AccessMasterT accessListGetMaster4 ();
extern void accessListVersionUpdate4 (AccessMasterT);
#ifdef HAVE_IPV6
extern AccessMasterT accessListGetMaster6 ();
extern void accessListVersionUpdate6 (AccessMasterT);
#endif
extern void accessListReset (void);
extern void accessListAddHook (void (*func)(AccessListT *));
extern void accessListDeleteHook (void (*func)(AccessListT *));
extern AccessListT *accessListLookup (afi_t, const StringT);
extern eFilterTypeT accessListApply (AccessListT *, void *);

#endif /* _nnFilter_h */
