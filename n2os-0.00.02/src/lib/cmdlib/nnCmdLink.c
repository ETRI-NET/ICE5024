/**
 * @file      : nnCmdService.c
 * @brief       :
 *
 * $Id: nnCmdMd5.c 725 2014-01-17 09:11:34Z hyryu $
 * $Author: hyryu $
 * $Date: 2014-01-17 04:11:34 -0500 (Fri, 17 Jan 2014) $
 * $Log$
 * $Revision: 725 $
 * $LastChangedBy: hyryu $
 * $LastChanged$
 *
 *                      Electronics and Telecommunications Research Institute
 * Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.
 *           No part of this software shall be reproduced, stored in a retrieval system, or
 *           transmitted by any means, electronic, mechanical, photocopying, recording,
 *           or otherwise, without written permission from ETRI.
 **/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "nnTypes.h"
#include "nnCmdLink.h"
/*******************************************************************************************************
 *                                       CODE
 *******************************************************************************************************/
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
struct cmdList *
cmdListNew(void)
{
  struct cmdList *new = (struct cmdList *)malloc(sizeof(*new));
  memset(new, 0, sizeof(*new));
  return new;
}
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void
cmdListFree(struct cmdList *nnList)
{
  if(nnList)
  {
    free(nnList);
  }
}
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
struct cmdListNode *
cmdListNodeNew(void)
{
  struct cmdListNode *node = (struct cmdListNode *)malloc(sizeof(*node));
  memset(node, 0, sizeof(*node));
  return node;
}
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void
cmdListNodeFree(struct cmdListNode *node)
{
  if(node)
  {
    free(node);
  }
}

/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
struct cmdListNode *
cmdListAddNode(struct cmdList *list, void *val)
{
  struct cmdListNode *node;
  if ( (!list) || (!val) )
  {
    return NULL;
  }
  node = cmdListNodeNew();
  if ( !node )
  {
    return NULL;
  }
  node->pPrev = list->pTail;
  node->pData = val;

  if (list->pHead == NULL)
  {
    list->pHead = node;
  } else {
    list->pTail->pNext = node;
  }
  list->pTail = node;
  list->pCount++;
  return node;
}
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void 
cmdListDelNode(struct cmdList *list, struct cmdListNode *node)
{
  if (node->pPrev)
  {
    node->pPrev->pNext = node->pNext;
  } else {
    list->pHead = node->pNext;
  }
  if (node->pNext)
  {
    node->pNext->pPrev = node->pPrev;
  } else {
    list->pTail = node->pPrev;
  }
  list->pCount--;
  cmdListNodeFree(node);
}
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void 
cmdListNodeDel(struct cmdList *list, void *val)
{
  struct cmdListNode *node;

  if ( (!list) || (!val) )
  {
    return;
  }
  for (node = list->pHead; node; node = node->pNext)
  {
    if (node->pData == val)
    {
      if (node->pPrev)
      {
        node->pPrev->pNext = node->pNext;
      } else {
        list->pHead = node->pNext;
      }
      if (node->pNext)
      {
        node->pNext->pPrev = node->pPrev;
      } else {
        list->pTail = node->pPrev;
      }
      list->pCount--;
      cmdListNodeFree(node);
      return;
    }
  }
}
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void 
cmdListDel(struct cmdList *list)
{
  struct cmdListNode *node, *next;
  for (node = list->pHead; node; node = next)
  {
    next = node->pNext;
    cmdListNodeFree(node);
  }
  cmdListFree(list);
}
