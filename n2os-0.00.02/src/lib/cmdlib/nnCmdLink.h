#ifndef __NN_CMD_MANAGER_LINK_H__
#define __NN_CMD_MANAGER_LINK_H__

/**
 * @brief Overview :
 * @brief Creator: Thanh Nguyen Ba
 * @file      : nnCmdService.h
 *
 * $Id: nnCmdMd5.h 725 2014-01-17 09:11:34Z hyryu $
 * $Author: hyryu $
 * $Date: 2014-01-17 04:11:34 -0500 (Fri, 17 Jan 2014) $
 * $Log$
 * $Revision: 725 $
 * $LastChangedBy: thanh $
 * $LastChanged$
 *
 *                      Electronics and Telecommunications Research Institute
 * Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.
 *           No part of this software shall be reproduced, stored in a retrieval system, or
 *           transmitted by any means, electronic, mechanical, photocopying, recording,
 *           or otherwise, without written permission from ETRI.
 **/
#define CMD_MANAGER_LIST_LOOP(L,V,N) \
  for ((N) = (L)->pHead; (N); (N) = (N)->pNext) \
    if (((V) = (N)->pData) != NULL)

#define CMD_MANAGER_LISTNODE_ADD(L,N) \
  do { \
    (N)->pNext = NULL; \
    (N)->pPrev = (L)->pTail; \
    if ((L)->pHead == NULL) \
      (L)->pHead = (N); \
    else \
      (L)->pTail->pNext = (N); \
    (L)->pTail = (N); \
  } while (0)

#define CMD_MANAGER_LISTNODE_DELETE(L,N) \
  do { \
    if ((N)->pPrev) \
      (N)->pPrev->pNext = (N)->pNext; \
    else \
      (L)->pHead = (N)->pNext; \
    if ((N)->pNext) \
      (N)->pNext->pPrev = (N)->pPrev; \
    else \
      (L)->pTail = (N)->pPrev; \
  } while (0)

#define CMD_MANAGER_LIST_COUNT(X) (((X) != NULL) ? ((X)->pCount) : 0) /** Count of Linlist  */

struct cmdListNode
{
  struct cmdListNode *pNext;
  struct cmdListNode *pPrev;
  void *pData;
};
struct cmdList
{
  struct cmdListNode *pHead;
  struct cmdListNode *pTail;
  Int32T pCount;
};

/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
struct cmdList *cmdListNew(void);
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
struct cmdListNode *cmdListAddNode(struct cmdList *, void *);
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void cmdListFree(struct cmdList *);
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void cmdListDelNode(struct cmdList *, struct cmdListNode *);
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void cmdListNodeDel(struct cmdList *, void *);
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void cmdListDel(struct cmdList *);
#endif
