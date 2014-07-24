/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : RIP Protocol의 Offset 설정을 제어하는 화일
 *
 * - Block Name : RIP Protocol
 * - Process Name : ripd
 * - Creator : Geontae Park
 * - Initial Date : 2014/02/20
 */

/**
 * @file        : ripOffset.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include <assert.h>

#include "nnTypes.h"
#include "nnStr.h"
#include "nnPrefix.h"
#include "nnList.h"
#include "nnRibDefines.h"
#include "nosLib.h"

#include "nnCmdDefines.h"
#include "nnCompProcess.h"
#include "nnCmdCmsh.h"

#include "nnFilter.h"
#include "ripd.h"

#define RIP_OFFSET_LIST_IN  0
#define RIP_OFFSET_LIST_OUT 1
#define RIP_OFFSET_LIST_MAX 2

struct ripOffset
{
  char *ifName;

  struct 
  {
    char *alistName;
    /* AccessListT *alist; */
    Int32T metric;
  } direct[RIP_OFFSET_LIST_MAX];
};
typedef struct ripOffset RipOffsetT;

static Int32T
strcmpSafe (const char *s1, const char *s2)
{
  if (s1 == NULL && s2 == NULL)
    return 0;
  if (s1 == NULL)
    return -1;
  if (s2 == NULL)
    return 1;
  return strcmp (s1, s2);
}

static RipOffsetT *
ripOffsetListNew (void)
{
  return NNMALLOC (MEM_OFFSET, sizeof (RipOffsetT));
}

static void
ripOffsetListFree (RipOffsetT *offset)
{
  NNFREE (MEM_OFFSET, offset);
}

static RipOffsetT *
ripOffsetListLookup (const char *ifName)
{
  ListNodeT * pNode = NULL;
  RipOffsetT * pOffset = NULL;

  for(pNode = (pRip->pRipOffsetList)->pHead;pNode != NULL;pNode = pNode->pNext) 
  {
    pOffset = pNode->pData;
    if (strcmpSafe (pOffset->ifName, ifName) == 0)
    {
		return pOffset;
    }
  }

  return NULL;
}

static RipOffsetT *
ripOffsetListGet (const StringT ifName)
{
  RipOffsetT * pOffset = NULL;

  pOffset = ripOffsetListLookup (ifName);
  if (pOffset)
  {
    return pOffset;
  }

  pOffset = ripOffsetListNew ();

  if (ifName)
    pOffset->ifName = nnStrDup (ifName, MEM_IF_NAME);

  nnListAddNodeSort (pRip->pRipOffsetList, pOffset);

  return pOffset;
}

#if 0
static Int32T
ripOffsetListSet (struct vty *vty, const char *alist, const char *direct_str,
		     const char *metric_str, const char *ifName)
{
  Int32T direct;
  Int32T metric;
  RipOffsetT *offset;
  NNLOG(LOG_DEBUG,"gtpark start : %s %d", __FUNCTION__, __LINE__);

  /* Check direction. */
  if (strncmp (direct_str, "i", 1) == 0)
    direct = RIP_OFFSET_LIST_IN;
  else if (strncmp (direct_str, "o", 1) == 0)
    direct = RIP_OFFSET_LIST_OUT;
  else
    {
      vty_out (vty, "Invalid direction: %s%s", direct_str, VTY_NEWLINE);
	NNLOG(LOG_DEBUG,"gtpark end : %s %d", __FUNCTION__, __LINE__);
      return CMD_WARNING;
    }

  /* Check metric. */
  metric = atoi (metric_str);
  if (metric < 0 || metric > 16)
    {
      vty_out (vty, "Invalid metric: %s%s", metric_str, VTY_NEWLINE);
	NNLOG(LOG_DEBUG,"gtpark end : %s %d", __FUNCTION__, __LINE__);
      return CMD_WARNING;
    }

  /* Get offset-list structure with interface name. */
  offset = ripOffsetListGet (ifName);

  if (offset->direct[direct].alistName)
    NNFREE (MEM_ACCESSLIST_NAME, offset->direct[direct].alistName);
  offset->direct[direct].alistName = nnStrDup (alist, MEM_ACCESSLIST_NAME);
  offset->direct[direct].metric = metric;

  NNLOG(LOG_DEBUG,"gtpark end : %s %d", __FUNCTION__, __LINE__);
  return CMD_SUCCESS;
}
#endif

#if 0
static Int32T
ripOffsetListUnset (struct vty *vty, const char *alist,
		       const char *direct_str, const char *metric_str,
		       const char *ifName)
{
  Int32T direct;
  Int32T metric;
  RipOffsetT *offset;
  NNLOG(LOG_DEBUG,"gtpark start : %s %d", __FUNCTION__, __LINE__);

  /* Check direction. */
  if (strncmp (direct_str, "i", 1) == 0)
    direct = RIP_OFFSET_LIST_IN;
  else if (strncmp (direct_str, "o", 1) == 0)
    direct = RIP_OFFSET_LIST_OUT;
  else
    {
      vty_out (vty, "Invalid direction: %s%s", direct_str, VTY_NEWLINE);
	NNLOG(LOG_DEBUG,"gtpark end : %s %d", __FUNCTION__, __LINE__);
      return CMD_WARNING;
    }

  /* Check metric. */
  metric = atoi (metric_str);
  if (metric < 0 || metric > 16)
    {
      vty_out (vty, "Invalid metric: %s%s", metric_str, VTY_NEWLINE);
	NNLOG(LOG_DEBUG,"gtpark end : %s %d", __FUNCTION__, __LINE__);
      return CMD_WARNING;
    }

  /* Get offset-list structure with interface name. */
  offset = ripOffsetListLookup (ifName);

  if (offset)
    {
      if (offset->direct[direct].alistName)
	NNFREE (MEM_ACCESSLIST_NAME, offset->direct[direct].alistName);
      offset->direct[direct].alistName = NULL;

      if (offset->direct[RIP_OFFSET_LIST_IN].alistName == NULL &&
	  offset->direct[RIP_OFFSET_LIST_OUT].alistName == NULL)
	{
	  listnode_delete (pRip->pRipOffsetList, offset);
	  if (offset->ifName)
	    NNFREE (offset->ifName, MEM_IF_NAME);
	  ripOffsetListFree (offset);
	}
    }
  else
    {
      vty_out (vty, "Can't find offset-list%s", VTY_NEWLINE);
	NNLOG(LOG_DEBUG,"gtpark end : %s %d", __FUNCTION__, __LINE__);
	return CMD_WARNING;
    }

  NNLOG(LOG_DEBUG,"gtpark end : %s %d", __FUNCTION__, __LINE__);
  return CMD_SUCCESS;
}
#endif

#define OFFSET_LIST_IN_NAME(O)  ((O)->direct[RIP_OFFSET_LIST_IN].alistName)
#define OFFSET_LIST_IN_METRIC(O)  ((O)->direct[RIP_OFFSET_LIST_IN].metric)

#define OFFSET_LIST_OUT_NAME(O)  ((O)->direct[RIP_OFFSET_LIST_OUT].alistName)
#define OFFSET_LIST_OUT_METRIC(O)  ((O)->direct[RIP_OFFSET_LIST_OUT].metric)

/* If metric is modifed return 1. */
Int32T
ripOffsetListApplyIn (Prefix4T * p, InterfaceT * pIf, Uint32T * metric)
{
  RipOffsetT * pOffset = NULL;
  AccessListT * alist = NULL;

  /* Look up offset-list with interface name. */
  pOffset = ripOffsetListLookup (pIf->name);
  if (pOffset && OFFSET_LIST_IN_NAME (pOffset))
  {
    alist = accessListLookup (AFI_IP, OFFSET_LIST_IN_NAME (pOffset));

    if (alist && 
        accessListApply (alist, (PrefixT *)p) == FILTER_PERMIT)
    {
      *metric += OFFSET_LIST_IN_METRIC (pOffset);
      return 1;
    }
    return 0;
  }
  /* Look up offset-list without interface name. */
  pOffset = ripOffsetListLookup (NULL);
  if (pOffset && OFFSET_LIST_IN_NAME (pOffset))
  {
    alist = accessListLookup (AFI_IP, OFFSET_LIST_IN_NAME (pOffset));

    if (alist && 
        accessListApply (alist, (PrefixT *)p) == FILTER_PERMIT)
    {
      *metric += OFFSET_LIST_IN_METRIC (pOffset);
      return 1;
    }
    return 0;
  }
  return 0;
}

/* If metric is modifed return 1. */
Int32T
ripOffsetListApplyOut (Prefix4T * p, InterfaceT * pIf, Uint32T * metric)
{
  RipOffsetT * pOffset = NULL;
  AccessListT * alist = NULL;

  /* Look up offset-list with interface name. */
  pOffset = ripOffsetListLookup (pIf->name);
  if (pOffset && OFFSET_LIST_OUT_NAME (pOffset))
  {
    alist = accessListLookup (AFI_IP, OFFSET_LIST_OUT_NAME (pOffset));

    if (alist && 
        accessListApply (alist, (PrefixT *)p) == FILTER_PERMIT)
    {
      *metric += OFFSET_LIST_OUT_METRIC (pOffset);
      return 1;
    }
    return 0;
  }

  /* Look up offset-list without interface name. */
  pOffset = ripOffsetListLookup (NULL);
  if (pOffset && OFFSET_LIST_OUT_NAME (pOffset))
  {
    alist = accessListLookup (AFI_IP, OFFSET_LIST_OUT_NAME (pOffset));

    if (alist && 
        accessListApply (alist, (PrefixT *)p) == FILTER_PERMIT)
    {
      *metric += OFFSET_LIST_OUT_METRIC (pOffset);
      return 1;
    }
    return 0;
  }
  return 0;
}

#if 0
DEFUN (rip_offset_list,
       rip_offset_list_cmd,
       "offset-list WORD (in|out) <0-16>",
       "Modify RIP metric\n"
       "Access-list name\n"
       "For incoming updates\n"
       "For outgoing updates\n"
       "Metric value\n")
{
  return ripOffsetListSet (vty, argv[0], argv[1], argv[2], NULL);
}

DEFUN (rip_offset_list_ifname,
       rip_offset_list_ifname_cmd,
       "offset-list WORD (in|out) <0-16> IFNAME",
       "Modify RIP metric\n"
       "Access-list name\n"
       "For incoming updates\n"
       "For outgoing updates\n"
       "Metric value\n"
       "Interface to match\n")
{
  return ripOffsetListSet (vty, argv[0], argv[1], argv[2], argv[3]);
}

DEFUN (no_rip_offset_list,
       no_rip_offset_list_cmd,
       "no offset-list WORD (in|out) <0-16>",
       NO_STR
       "Modify RIP metric\n"
       "Access-list name\n"
       "For incoming updates\n"
       "For outgoing updates\n"
       "Metric value\n")
{
  return ripOffsetListUnset (vty, argv[0], argv[1], argv[2], NULL);
}

DEFUN (no_rip_offset_list_ifname,
       no_rip_offset_list_ifname_cmd,
       "no offset-list WORD (in|out) <0-16> IFNAME",
       NO_STR
       "Modify RIP metric\n"
       "Access-list name\n"
       "For incoming updates\n"
       "For outgoing updates\n"
       "Metric value\n"
       "Interface to match\n")
{
  return ripOffsetListUnset (vty, argv[0], argv[1], argv[2], argv[3]);
}
#endif


/*
 * Description : List에서 Peer IP Address 값을 기반으로 소트하여 정렬하기 위한 비교함수.
 *
 * param [in] pOldData : ListNode의 포인터
 * param [in] pNewData : ListNode의 포인터
 *
 * retval : 0 if 기존 ListNode의 데이터가 새로운 ListNode의 데이터 보다 큰경우
 *          1 if 기존 ListNode의 데이터가 새로운 ListNode의 데이터 보다 작은경우.
 */
Int8T 
offsetListCmp (void *pOldData, void *pNewData)
{
  return strcmpSafe (((RipOffsetT *)pOldData)->ifName,
                      ((RipOffsetT *)pNewData)->ifName);
}


static void
offsetListDel (RipOffsetT *pOffset)
{

  if (OFFSET_LIST_IN_NAME (pOffset))
    NNFREE (MEM_OFFSET_IN, OFFSET_LIST_IN_NAME (pOffset));

  if (OFFSET_LIST_OUT_NAME (pOffset))
    NNFREE (MEM_OFFSET_OUT, OFFSET_LIST_OUT_NAME (pOffset));

  if (pOffset->ifName)
    NNFREE (MEM_IF_NAME, pOffset->ifName);

  ripOffsetListFree (pOffset);
}

void
ripOffsetInit ()
{
  pRip->pRipOffsetList = nnListInit (offsetListCmp, MEM_OFFSET);
}

void
ripOffsetClean ()
{
  /* Check rip global data structure pointer. */
  if (!pRip)
    return;

  /* Check rip offset list. */
  if (!pRip->pRipOffsetList)
    return;

  /* Delete & free ListNode and Memory frist... */
  nnListFree(pRip->pRipOffsetList);
  pRip->pRipOffsetList = NULL;
}


Int32T
configWriteRipOffsetList (struct cmsh *cmsh)
{
  ListNodeT * pNode = NULL;
  RipOffsetT * pOffset = NULL;

  for(pNode = pRip->pRipOffsetList->pHead;
      pNode != NULL;
      pNode = pNode->pNext)
  {
    pOffset = pNode->pData;
    if (! pOffset->ifName)
    {
      if (pOffset->direct[RIP_OFFSET_LIST_IN].alistName)
      {
        cmdPrint (cmsh, " offset-list %s in %d",
                  pOffset->direct[RIP_OFFSET_LIST_IN].alistName,
                  pOffset->direct[RIP_OFFSET_LIST_IN].metric);
      }
      if (pOffset->direct[RIP_OFFSET_LIST_OUT].alistName)
      {
        cmdPrint (cmsh, " offset-list %s out %d",
                  pOffset->direct[RIP_OFFSET_LIST_OUT].alistName,
                  pOffset->direct[RIP_OFFSET_LIST_OUT].metric);
      }
	}
    else
    {
	  if (pOffset->direct[RIP_OFFSET_LIST_IN].alistName)
      {
        cmdPrint (cmsh, " offset-list %s in %d %s",
                  pOffset->direct[RIP_OFFSET_LIST_IN].alistName,
                  pOffset->direct[RIP_OFFSET_LIST_IN].metric,
                  pOffset->ifName);
      }
	  if (pOffset->direct[RIP_OFFSET_LIST_OUT].alistName)
      {
        cmdPrint (cmsh, " offset-list %s out %d %s",
                  pOffset->direct[RIP_OFFSET_LIST_OUT].alistName,
                  pOffset->direct[RIP_OFFSET_LIST_OUT].metric,
                  pOffset->ifName);
      }
    }
  }
  return SUCCESS;
}
