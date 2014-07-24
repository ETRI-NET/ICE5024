/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : 각 컴포넌트에서 사용하는 공통 md5 key chain 관련 기능을 
 *                제공한다.
 * - Block Name : riblib
 * - Process Name : rib library
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : nnKeychain.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "nnDefines.h"
#include "nnStr.h"
#include "nnList.h"
#include "nnKeychain.h"
#include "nosLib.h"

#include "nnCmdLink.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"
#include "nnCmdInstall.h"
#include "nosLib.h"
#include "nnCmdDefines.h"
#include "nnUtility.h"

#define CMSH_GET_LONG(NAME,V,STR) \
do { \
  char *endptr = NULL; \
  (V) = strtoul ((STR), &endptr, 10); \
  if (*endptr != '\0' || (V) == ULONG_MAX) \
    { \
      cmdPrint (cmsh, "%% Invalid %s value", NAME); \
      return CMD_IPC_WARNING; \
    } \
} while (0)

#define CMSH_GET_INTEGER_RANGE(NAME,V,STR,MIN,MAX) \
do { \
  unsigned long tmpl; \
  CMSH_GET_LONG(NAME, tmpl, STR); \
  if ( (tmpl < (MIN)) || (tmpl > (MAX))) \
    { \
      cmdPrint (cmsh, "%% Invalid %s value", NAME); \
      return CMD_IPC_WARNING; \
    } \
  (V) = tmpl; \
} while (0)

#define CMSH_GET_INTEGER(NAME,V,STR) \
  CMSH_GET_INTEGER_RANGE(NAME,V,STR,0U,UINT32_MAX)


/* Master list of key chain. */
ListT * pKeychainList;

static KeychainT *
keychainNew (void)
{
  return NNMALLOC (MEM_KEYCHAIN, sizeof (KeychainT));
}

static void
keychainFree (KeychainT *pKeychain)
{
  NNFREE (MEM_KEYCHAIN, pKeychain);
}

static KeyT *
keyNew (void)
{
  return NNMALLOC (MEM_KEY, sizeof (KeyT));
}

static void
keyFree (KeyT *pKey)
{
  NNFREE (MEM_KEY, pKey);
}

KeychainT *
keychainLookup (const StringT name)
{
  ListNodeT * pNode = NULL;
  KeychainT * pKeychain = NULL;

  if (name == NULL)
    return NULL;

  for (pNode = pKeychainList->pHead ; pNode != NULL ; pNode = pNode->pNext)
  {
    pKeychain = pNode->pData;

    if (strcmp (pKeychain->name, name) == 0)
      return pKeychain;
  }
  return NULL;
}

static Int8T
keyCmpFunc (void *pArg1, void *pArg2)
{
  const KeyT *k1 = pArg1;
  const KeyT *k2 = pArg2;
  
  if (k1->index > k2->index)
    return 1;
  if (k1->index < k2->index)
    return -1;
  return 0;
}

#if 0
static void
key_delete_func (KeyT *key)
{
  if (key->string)
    NNFREE (MEM_KEY_NAME, key->string);

  keyFree (key);
}
#endif

KeychainT *
keychainGet (const StringT name)
{
  KeychainT * pKeychain = NULL;

  /* Lookup keychain from keychain list. */
  pKeychain = keychainLookup (name);

  if (pKeychain)
    return pKeychain;

  pKeychain = keychainNew ();
  pKeychain->name = nnStrDup (name, MEM_KEYCHAIN_NAME);
  pKeychain->pKeyList = nnListInit (keyCmpFunc, MEM_KEY);

/* bellow cmp & del field is not used in nnList */
//  pKeychain->key->cmp = (Int32T (*)(void *, void *)) keyCmpFunc;
//  pKeychain->key->del = (void (*)(void *)) key_delete_func;

  /* Add keychain to keychain list. */
  nnListAddNode (pKeychainList, pKeychain);

  return pKeychain;
}

void
keychainDelete (KeychainT *pKeychain)
{
  ListNodeT * pNode = NULL;
  KeyT * pKey = NULL;

  /* Free all string of all key node. */
  for (pNode = pKeychain->pKeyList->pHead; 
       pNode != NULL; 
       pNode = pNode->pNext)
  {
    pKey = pNode->pData;
    if (pKey->string)
      NNFREE (MEM_KEY_NAME, pKey->string); 
  }

  /* Free all key node of key list. */
  nnListFree (pKeychain->pKeyList);

  /* Check and free name string of keychain. */
  if (pKeychain->name)
    NNFREE (MEM_KEYCHAIN_NAME, pKeychain->name);

  /* Free keychain node of keychain list. */
  nnListDeleteNode (pKeychainList, pKeychain);

  /* Blocked because autofree in nnListDeleteNode function. */
  //keychainFree (pKeychain);
}

KeyT *
keyLookup (const KeychainT *pKeychain, Uint32T index)
{
  ListNodeT * pNode = NULL;
  KeyT * pKey = NULL;

  for (pNode = pKeychain->pKeyList->pHead; 
       pNode != NULL; 
       pNode = pNode->pNext)
  {
    pKey = pNode->pData;

    if (pKey->index == index)
      return pKey;
  }

  return NULL;
}

KeyT *
keyLookupForAccept (const KeychainT *pKeychain, Uint32T index)
{
  ListNodeT * pNode = NULL;
  KeyT * pKey = NULL;
  time_t now;

  now = time (NULL);

  for (pNode = pKeychain->pKeyList->pHead ; 
       pNode != NULL ; 
       pNode = pNode->pNext)
  {
    pKey = pNode->pData;

    if (pKey->index >= index)
    {
      if (pKey->accept.start == 0)
        return pKey;

      if (pKey->accept.start <= now)
        if (pKey->accept.end >= now || pKey->accept.end == -1)
          return pKey;
    }
  }

  return NULL;
}

KeyT *
keyMatchForAccept (const KeychainT *pKeychain, const StringT strAuth)
{
  ListNodeT * pNode = NULL;
  KeyT * pKey = NULL;
  time_t now;

  now = time (NULL);

  for (pNode = pKeychain->pKeyList->pHead ; 
       pNode != NULL ; 
       pNode = pNode->pNext)
  {
    pKey = pNode->pData;

    if (pKey->accept.start == 0 ||
       (pKey->accept.start <= now &&
       (pKey->accept.end >= now || pKey->accept.end == -1)))
    {
      if (strncmp (pKey->string, strAuth, 16) == 0)
        return pKey;
    }
  }

  return NULL;
}

KeyT *
keyLookupForSend (const KeychainT *pKeychain)
{
  ListNodeT * pNode = NULL;
  KeyT * pKey = NULL;
  time_t now;

  now = time (NULL);

  for (pNode = pKeychain->pKeyList->pHead ; 
       pNode != NULL ; 
       pNode = pNode->pNext)
  {
    pKey = pNode->pData;

    if (pKey->send.start == 0)
      return pKey;

    if (pKey->send.start <= now)
      if (pKey->send.end >= now || pKey->send.end == -1)
        return pKey;
  }

  return NULL;
}

KeyT *
keyGet (const KeychainT *pKeychain, Uint32T index)
{
  KeyT * pKey = NULL;

  pKey = keyLookup (pKeychain, index);

  if (pKey)
    return pKey;

  pKey = keyNew ();
  pKey->index = index;
  nnListAddNodeSort (pKeychain->pKeyList, pKey);

  return pKey;
}

void
keyDelete (KeychainT *pKeychain, KeyT *pKey)
{
  nnListDeleteNode (pKeychain->pKeyList, pKey);

  if (pKey->string)
    NNFREE (MEM_KEY_NAME, pKey->string);

  /* Blocked because autofree in nnListDeleteNode function. */
  //keyFree (pKey);
}



/* Convert HH:MM:SS MON DAY YEAR to time_t value.  -1 is returned when
   given string is malformed. */
static time_t 
keyStr2Time (const char *strTime, const char *strDay, const char *strMonth,
             const char *strYear)
{
  Int32T i = 0;
  char *colon;
  struct tm tm;
  time_t time;
  Uint32T sec, min, hour;
  Uint32T day, month, year;

  const char *month_name[] = 
  {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December",
    NULL
  };

#define GET_LONG_RANGE(V,STR,MIN,MAX) \
{ \
  Uint32T tmpl; \
  char *endptr = NULL; \
  tmpl = strtoul ((STR), &endptr, 10); \
  if (*endptr != '\0' || tmpl == ULONG_MAX) \
    return -1; \
  if ( tmpl < (MIN) || tmpl > (MAX)) \
    return -1; \
  (V) = tmpl; \
}
      
  /* Check hour field of strTime. */
  colon = strchr (strTime, ':');
  if (colon == NULL)
    return -1;
  *colon = '\0';

  /* Hour must be between 0 and 23. */
  GET_LONG_RANGE (hour, strTime, 0, 23);

  /* Check min field of strTime. */
  strTime = colon + 1;
  colon = strchr (strTime, ':');
  if (*strTime == '\0' || colon == NULL)
    return -1;
  *colon = '\0';

  /* Min must be between 0 and 59. */
  GET_LONG_RANGE (min, strTime, 0, 59);

  /* Check sec field of strTime. */
  strTime = colon + 1;
  if (*strTime == '\0')
    return -1;
  
  /* Sec must be between 0 and 59. */
  GET_LONG_RANGE (sec, strTime, 0, 59);
  
  /* Check strDay.  Day must be <1-31>. */
  GET_LONG_RANGE (day, strDay, 1, 31);

  /* Check strMonth.  Month must match month_name. */
  month = 0;
  if (strlen (strMonth) >= 3)
    for (i = 0; month_name[i]; i++)
      if (strncmp (strMonth, month_name[i], strlen (strMonth)) == 0)
	{
	  month = i;
	  break;
	}
  if (! month_name[i])
    return -1;

  /* Check strYear.  Year must be <1993-2035>. */
  GET_LONG_RANGE (year, strYear, 1993, 2035);
  
  memset (&tm, 0, sizeof (struct tm));
  tm.tm_sec = sec;
  tm.tm_min = min;
  tm.tm_hour = hour;
  tm.tm_mon = month;
  tm.tm_mday = day;
  tm.tm_year = year - 1900;
    
  time = mktime (&tm);
  
  return time;
#undef GET_LONG_RANGE
}

Int32T
keyLifetimeSet (struct cmsh *cmsh, KeyrangeT *pKeyRange,
                const char *strSTime, const char *strSDay,
                const char *strSMonth, const char *strSYear,
                const char *strETime, const char *strEDay,
                const char *strEMonth, const char *strEYear)
{
  time_t startTime;
  time_t endTime;
  
  startTime = keyStr2Time (strSTime, strSDay, strSMonth, strSYear);
  if (startTime < 0)
    {
      cmdPrint (cmsh, "Malformed time value");
      return CMD_IPC_WARNING;
    }
  endTime = keyStr2Time (strETime, strEDay, strEMonth, strEYear);

  if (endTime < 0)
    {
      cmdPrint (cmsh, "Malformed time value");
      return CMD_IPC_WARNING;
    }

  if (endTime <= startTime)
    {
      cmdPrint (cmsh, "Expire time is not later than start time");
      return CMD_IPC_WARNING;
    }

  pKeyRange->start = startTime;
  pKeyRange->end = endTime;

  return (CMD_IPC_OK);
}


Int32T
keyLifetimeDurationSet (struct cmsh *cmsh, KeyrangeT *pKeyRange,
                        const char *strSTime, const char *strSDay,
                        const char *strSMonth, const char *strSYear,
                        const char *strDuration)
{
  time_t startTime;
  Uint32T duration;
    
  startTime = keyStr2Time (strSTime, strSDay, strSMonth, strSYear);
  if (startTime < 0)
    {
      cmdPrint (cmsh, "Malformed time value");
      return CMD_IPC_WARNING;
    }
  pKeyRange->start = startTime;

  CMSH_GET_INTEGER ("duration", duration, strDuration);
  pKeyRange->duration = 1;
  pKeyRange->end = startTime + duration;

  return (CMD_IPC_OK);
}


Int32T
keyLifetimeInfiniteSet (struct cmsh *cmsh, KeyrangeT *pKeyRange,
                        const char *strSTime, const char *strSDay,
                        const char *strSMonth, const char *strSYear)
{
  time_t startTime;
    
  startTime = keyStr2Time (strSTime, strSDay, strSMonth, strSYear);
  if (startTime < 0)
    {
      cmdPrint (cmsh, "Malformed time value");
      return CMD_IPC_WARNING;
    }
  pKeyRange->start = startTime;

  pKeyRange->end = -1;

  return (CMD_IPC_OK);
}


static Int32T
keychainStrftime (char *buf, Int32T bufsiz, time_t *time)
{
  struct tm *tm;
  size_t len;

  tm = localtime (time);

  len = strftime (buf, bufsiz, "%T %b %d %Y", tm);

  return len;
}


Int32T
configWriteKeychain (struct cmsh * cmsh)
{
  ListNodeT * pNode = NULL;
  ListNodeT * pKNode = NULL;
  KeychainT * pKeychain = NULL;
  KeyT * pKey = NULL;
  char strBuff[BUFSIZ] = {};
  Int32T idx = 0;

  if (!pKeychainList)
  {
    printf ("pKeychainList is null. \n");
    return 0;
  }

  for (pNode = pKeychainList->pHead ; pNode != NULL ; pNode = pNode->pNext)
  {
    pKeychain = pNode->pData;

    cmdPrint (cmsh, "!");

    cmdPrint (cmsh, "key chain %s", pKeychain->name);

    for (pKNode = pKeychain->pKeyList->pHead ; pKNode != NULL ; pKNode = pKNode->pNext)
    {
      pKey = pKNode->pData;

      cmdPrint (cmsh, " key %d", pKey->index);

      if (pKey->string)
      {
        cmdPrint (cmsh, "  key-string %s", pKey->string);
      }

      memset (strBuff, 0, BUFSIZ);
      idx = 0;
      if (pKey->accept.start)
      {
        idx = sprintf (strBuff, "%s", "  accept-lifetime ");
        idx += keychainStrftime (strBuff + idx, BUFSIZ, &pKey->accept.start);

        if (pKey->accept.end == -1)
        {
          idx += sprintf (strBuff + idx, "%s", " infinite");
        }
        else if (pKey->accept.duration)
        {
          idx += sprintf (strBuff + idx, " duration %ld", 
                          (long)(pKey->accept.end - pKey->accept.start));
        }
        else
        {
          idx += keychainStrftime (strBuff + idx, BUFSIZ, &pKey->accept.end);
        }
        cmdPrint (cmsh, "%s", strBuff);
      }

      memset (strBuff, 0, BUFSIZ);
      idx = 0;
      if (pKey->send.start)
      {
        idx = sprintf (strBuff, "%s", "  send-lifetime ");
        idx += keychainStrftime (strBuff + idx, BUFSIZ, &pKey->send.start);

        if (pKey->send.end == -1)
        {
          idx += sprintf (strBuff, "%s", " infinite");
        }
        else if (pKey->send.duration)
        {
          idx += sprintf (strBuff + idx, " duration %ld", (long)(pKey->send.end - pKey->send.start));
        }
        else
        {
          idx += keychainStrftime (strBuff + idx, BUFSIZ, &pKey->send.end);
        }
        cmdPrint (cmsh, "%s", strBuff);
      }
      cmdPrint (cmsh, "  exit");
    }
  }

  return 0;
}


void
keychainInit ()
{
  pKeychainList = nnListInit (NULL, MEM_KEYCHAIN);
}

void
keychainReset ()
{
  ListNodeT * pNode = NULL;
  KeychainT * pKeychain = NULL;

  for (pNode = pKeychainList->pHead ; 
       pNode != NULL ; 
       pNode = pNode->pNext)
  {
    pKeychain = pNode->pData;

    /* Free KeyT list. */
    nnListFree (pKeychain->pKeyList);
    pKeychain->pKeyList = NULL;
  } 

  /* Free KeychainT list. */
  nnListFree (pKeychainList);
  pKeychainList = NULL;
}


ListT *
keychainGetPtr ()
{
  /* Assign pKeyChainList pointer to keychainList pointer. */
  NNLOG (LOG_DEBUG, "pKeychainList=%p\n", pKeychainList);

  return pKeychainList;
}


void
keychainKeyCompFuncUpdate()
{
  ListNodeT * pNode = NULL;
  KeychainT * pKeychain = NULL;

  for (pNode = pKeychainList->pHead ; pNode != NULL ; pNode = pNode->pNext)
  {
    pKeychain = pNode->pData;

    if (pKeychain)
      nnListSetNodeCompFunc (pKeychain->pKeyList, keyCmpFunc);
  }
}


void
keychainVersionUpdate (ListT * keyChainList)
{
  assert (keyChainList);

  /* Update keyChainList pointer to pKeychainList pointer. */
  pKeychainList = keyChainList;

  NNLOG (LOG_DEBUG, "pKeychainList=%p, keyChainList=%p\n", 
         pKeychainList, keyChainList);

  /* Update compare function */
  keychainKeyCompFuncUpdate();
}
