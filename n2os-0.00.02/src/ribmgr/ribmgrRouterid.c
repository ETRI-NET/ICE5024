/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : ribmgr에서 인터페이스 주소 또는 사용자에 의한 RouterID 설정
 * 값을 기반으로 각 프로토콜로 정보를 제공하는 기능을 하는 파일임.
 * RouterID 값이 운용자에 의해 입력되지 않는 경우에는 PIF Manager로 부터 제공
 * 받은 인터페이스들 중에서 주소 값이 가장 큰 값을 기본적으로 RouterID로 사용.
 * 운용자에 의해 RouterID 값을 입력받는 경우는 입력된 값을 RouterID로 사용함.
 * Rib Manager에서 RouterID를 결정하는 매 순간마다 EVENT_ROUTER_ID 이벤트를
 * 전송한다.
 *
 * - Block Name : RIB Manager
 * - Process Name : ribmgr
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : ribmgrRouterid.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include "nnRibDefines.h"
#include "nnIf.h"
#include "nnPrefix.h"
#include "nnTable.h"
#include "nnBuffer.h"
#include "nosLib.h"

#include "ribmgrInit.h"
#include "ribmgrRib.h"
#include "ribmgrConnected.h"
#include "ribmgrRouterid.h"
#include "ribmgrRedistribute.h"
#include "ribmgrUtil.h"


static void
printRouterIdList()
{
  ListNodeT * pNode = NULL;
  Prefix4T  * pData = NULL;

  /* Print pRidLoSortedList */
  NNLOG(LOG_DEBUG, "pRidLoSortedList \n");
  for(pNode = pRibmgr->pRidLoSortedList->pHead; pNode != NULL; pNode = pNode->pNext)
  {
    pData = pNode->pData;
    NNLOG(LOG_DEBUG, "\tprefix/len = %s/%d \n", inet_ntoa(pData->prefix), pData->prefixLen);
  }
  
  /* Print pRidAllSortedList */
  NNLOG(LOG_DEBUG, "pRidAllSortedList \n");
  for(pNode = pRibmgr->pRidAllSortedList->pHead; pNode != NULL; pNode = pNode->pNext)
  {
    pData = pNode->pData;
    NNLOG(LOG_DEBUG, "\tprefix/len = %s/%d \n", inet_ntoa(pData->prefix), pData->prefixLen);
  }
}


/**
 * Description : RouterID 이벤트 메시지 구성 함수.
 *
 * @param [in] apiBuff : 이벤트 메시지 버퍼
 * @param [in] p : Prefix4T 자료구조 포인터
 *
 * @retval : 이벤트 메시지 버퍼 길이
 *
 * Message format
 *
 *  0 1 2 3 4 5 6 7 8 9 A B C D E F 0 1 2 3 4 5 6 7 8 9 A B C D E F
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      N2OS IPC Header                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     Family     |
 * +-+-+-+-+-+-+-+-+-
 * |   PrefixLen    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         Ipv4 Address                          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
Int32T buildRouterId(nnBufferT * apiBuff, Prefix4T * pPrefix4)
{
  /* Set Prefix Family */
  nnBufferSetInt8T(apiBuff, pPrefix4->family); 

  /* Set Prefix Length */
  nnBufferSetInt8T(apiBuff, pPrefix4->prefixLen);

  /* Set Prefix Address */
  nnBufferSetInaddr(apiBuff, pPrefix4->prefix);

  return nnBufferGetLength(apiBuff);
}


/*
 * Description : Router ID로 사용할 IPv4 주소 값이 동일한라를 체크하는 함수.
 *
 * param [in] pP1 : Prefix4T
 * param [in] pP2 : Prefix4T
 *
 * retval : TRUE  if 주소값이 동일하면,
 *          FALSE if 주소값이 다르면
 */
static Int32T 
routerIdPrefix4Same(Prefix4T * pP1, Prefix4T * pP2)
{
  if (pP1->family == pP2->family && pP1->prefixLen == pP2->prefixLen)
  {   
    if (pP1->family == AF_INET)
    {
      if (memcmp(&pP1->prefix, &pP2->prefix, sizeof(struct in_addr)) == 0)
      {
        return TRUE;
      }
    }
  }

  return FALSE;  
}


/*
 * Description : List에서 Router ID를 찾는 함수
 *
 * param [in] pList : RouterID 정보를 저장하고 있는 List
 * param [in] pPrefix4 : Prefix4T
 *
 * retval : ListNodeT *  if 찾았으면,
 *          NULL if 찾지 못했으면
 */
static ListNodeT *
routerIdFindNode (ListT * pList, Prefix4T * pPrefix4)
{
  return (ListNodeT *) nnListSearchNode(pList, pPrefix4);
}


/*
 * Description : 입력된 주소가 RouterID로 사용가능한지를 판단하는 함수.
 *
 * param [in] pPrefix4 : Prefix4T
 *
 * retval : TRUE  if 사용가능하면,
 *          FALSE if 사용가능하지 않으면
 */
static Int32T
routerIdBadAddress (Prefix4T * pPrefix4)
{
  if (pPrefix4->family != AF_INET)
  {
    return TRUE;
  }
  
  /* non-redistributable addresses shouldn't be used for RIDs either */
  if (!ribCheckAddr4 (pPrefix4))
  {
    return TRUE;
  }
  
  return FALSE;
}


/* Description : 현재 설정된 RouterID값을 획득하는 함수. */
void
routerIdGet (Prefix4T * pPrefix4)
{
  ListNodeT * pNode = NULL;
  Prefix4T  * pP2 = NULL;

  pPrefix4->prefix.s_addr = 0;
  pPrefix4->family = AF_INET;
  pPrefix4->prefixLen = 32;

  if (pRibmgr->ridUserAssigned.prefix.s_addr)
  {
    pPrefix4->prefix.s_addr = pRibmgr->ridUserAssigned.prefix.s_addr;
  }
  else if (nnListCount (pRibmgr->pRidLoSortedList) > 0)
  {
    NNLOG(LOG_DEBUG, "pRidLoSortedList count = %d\n", 
          (int)nnListCount (pRibmgr->pRidLoSortedList));
    pNode = nnListGetTailNode (pRibmgr->pRidLoSortedList);
    pP2 = pNode->pData;
    pPrefix4->prefix.s_addr = pP2->prefix.s_addr;
  }
  else if (nnListCount (pRibmgr->pRidAllSortedList) > 0)
  {
    NNLOG(LOG_DEBUG, "pRidAllSortedList count = %d\n", 
          (int)nnListCount (pRibmgr->pRidAllSortedList));
    pNode = nnListGetTailNode (pRibmgr->pRidAllSortedList);
    pP2 = pNode->pData;
    pPrefix4->prefix.s_addr = pP2->prefix.s_addr;
  }
  else
  {
    NNLOG(LOG_WARNING, "Not exist interface for router-id\n");
  }
}

/* Description : 운용자에 의해 RouterID가 설정/삭제되는 경우에 호출되는 함수. */
void
routerIdSet (Prefix4T * pPrefix4)
{
  Prefix4T p2 = {0,};
  nnBufferT msgBuff= {0,};

  /* Assigned Router ID installed by Operator */
  pRibmgr->ridUserAssigned.prefix.s_addr = pPrefix4->prefix.s_addr;

  /* Select RouterID */
  routerIdGet (&p2);

  /* Buffer Reset */
  nnBufferReset(&msgBuff);

  /* Building Event Message for RouterId */
  buildRouterId(&msgBuff, pPrefix4);

  /* Send Event Message */
  eventPublish(EVENT_ROUTER_ID, msgBuff.data, msgBuff.length);

  NNLOG(LOG_DEBUG, "++++++++++++++++++++++++++++++++++++++++++++++++\n");
  NNLOG(LOG_DEBUG, "++++ Sending Router ID Event +++++++++++++++++++\n");
  NNLOG(LOG_DEBUG, "++++++++++++++++++++++++++++++++++++++++++++++++\n");
  NNLOG(LOG_DEBUG, "family=%d, prefixLen=%d, prefix=%s\n", 
                    pPrefix4->family, pPrefix4->prefixLen, inet_ntoa(pPrefix4->prefix));  
  NNLOG(LOG_DEBUG, "++++++++++++++++++++++++++++++++++++++++++++++++\n");
}


/* Description : 인터페이스에서 주소를 RouterID에 설정하는 함수 */
void
routerIdAddAddress (ConnectedT *pIfc)
{
  ListT *pList = NULL;
  Prefix4T before = {0,};
  Prefix4T after = {0,};
  Prefix4T * tmpPrefix = NULL;
  nnBufferT msgBuff= {0,};

  NNLOG(LOG_DEBUG, "++++++++++++++++++%s++++++++++++++++++\n", __func__);
  NNLOG(LOG_DEBUG, "  ifName = %s\n", pIfc->pIf->name);
  NNLOG(LOG_DEBUG, "++++++++++++++++++%s++++++++++++++++++\n", __func__);
  
  /* Copy Prefix Address */
  tmpPrefix = (Prefix4T *)NNMALLOC(MEM_PREFIX, sizeof(Prefix4T));
  tmpPrefix->family = AF_INET;
  tmpPrefix->prefixLen = 32;
  memcpy(&tmpPrefix->prefix, &pIfc->pAddress->u.prefix4, sizeof(struct in_addr));

  /* Check Address */
  if (routerIdBadAddress (tmpPrefix))
  {
    NNLOG(LOG_DEBUG, "Warning .... routerIdBadAddress......\n");
    printRouterIdList();
    return;
  }

  /* Select RouterID */
  routerIdGet (&before);

  /* Check Address Type : lo address is first... */
  if (!strncmp (pIfc->pIf->name, "lo", 2) || 
      !strncmp (pIfc->pIf->name, "dummy", 5))
  {
    pList = pRibmgr->pRidLoSortedList;
  }
  else
  {
    pList = pRibmgr->pRidAllSortedList;
  }
  
  /* Add Address to the Selected List */
  if (!routerIdFindNode (pList, tmpPrefix))
  {
    nnListAddNodeSort (pList, tmpPrefix);

    NNLOG(LOG_DEBUG, "No Same Node......\n");
    printRouterIdList();
  }

  /* Select RouterID */
  routerIdGet (&after);

  /* Check RouterID, If Same, Then just return */
  if (routerIdPrefix4Same (&before, &after))
  {
    NNLOG(LOG_DEBUG, "Same Router ID\n");
    return;
  }

  /* Buffer Reset */
  nnBufferReset(&msgBuff);

  /* Building Event Message for RouterId */
  buildRouterId(&msgBuff, &after);

  /* Send Event Message */
  eventPublish(EVENT_ROUTER_ID, msgBuff.data, msgBuff.length);
  
  NNLOG(LOG_DEBUG, "++++++++++++++++++++++++++++++++++++++++++++++++\n");
  NNLOG(LOG_DEBUG, "++++ Sending Router ID Event +++++++++++++++++++\n");
  NNLOG(LOG_DEBUG, "++++++++++++++++++++++++++++++++++++++++++++++++\n");
  NNLOG(LOG_DEBUG, "++++ family=%d, prefixLen=%d, prefix=%s\n", 
                    after.family, after.prefixLen, inet_ntoa(after.prefix));  
  NNLOG(LOG_DEBUG, "++++++++++++++++++++++++++++++++++++++++++++++++\n");

  printRouterIdList();
}


/* Description : 인터페이스에서 주소를 RouterID에 삭제하는 함수 */
void
routerIdDelAddress (ConnectedT *pIfc)
{
  ListT * pList = NULL;
  ListNodeT * pNode = NULL;
  Prefix4T after = {0,};
  Prefix4T before = {0,};
  Prefix4T * pPrefixTmp = NULL;
  nnBufferT msgBuff= {0,};

  /* Copy Prefix Address */
  pPrefixTmp = (Prefix4T *)NNMALLOC(MEM_PREFIX, sizeof(Prefix4T));
  pPrefixTmp->family = AF_INET;
  pPrefixTmp->prefixLen = 32;
  memcpy(&pPrefixTmp->prefix, &pIfc->pAddress->u.prefix4, sizeof(struct in_addr));

  /* Check Address */
  if (routerIdBadAddress (pPrefixTmp))
  {
    return;
  }

  /* Select RouterID */
  routerIdGet (&before);

  /* Check Address Type : lo address is first... */
  if (!strncmp (pIfc->pIf->name, "lo", 2) || 
      !strncmp (pIfc->pIf->name, "dummy", 5))
  {
    pList = pRibmgr->pRidLoSortedList ;
  }
  else
  {
    pList = pRibmgr->pRidAllSortedList;
  }

  /* Add Address to the Selected List */
  if ((pNode = routerIdFindNode (pList, pPrefixTmp)))
  {
    nnListDeleteNode (pList, pNode);
  }

  /* Select RouterID */
  routerIdGet (&after);

  /* Check RouterID, If Same, Then just return */
  if (routerIdPrefix4Same (&before, &after))
  {
    return;
  }

  /* Buffer Reset */
  nnBufferReset(&msgBuff);

  /* Building Event Message for RouterId */
  buildRouterId(&msgBuff, &after);

  /* Send Event Message */
  eventPublish(EVENT_ROUTER_ID, msgBuff.data, msgBuff.length);
  
  NNLOG(LOG_DEBUG, "++++++++++++++++++++++++++++++++++++++++++++++++\n");
  NNLOG(LOG_DEBUG, "++++ Sending Router ID Event +++++++++++++++++++\n");
  NNLOG(LOG_DEBUG, "++++++++++++++++++++++++++++++++++++++++++++++++\n");
  NNLOG(LOG_DEBUG, "++++ family=%d, prefixLen=%d, prefix=%s\n", 
                    after.family, after.prefixLen, inet_ntoa(after.prefix));  
  NNLOG(LOG_DEBUG, "++++++++++++++++++++++++++++++++++++++++++++++++\n");

  printRouterIdList();
}


/*
 * Description : List에서 RouterID 값을 기반으로 소트하여 정렬하기 위한 비교함수.
 *
 * param [in] pOldData : ListNode의 포인터
 * param [in] pNewData : ListNode의 포인터
 *
 * retval : 0 if 기존 ListNode의 데이터가 새로운 ListNode의 데이터 보다 큰경우
 *          1 if 기존 ListNode의 데이터가 새로운 ListNode의 데이터 보다 작은경우.
 */
static Int8T 
routerIdCmp (void *pOldData, void *pNewData)
{
  if ((((Prefix4T *)pOldData)->prefix.s_addr) > (((Prefix4T *)pNewData)->prefix.s_addr))
  {
     return 0;  
  }
  else if ((((Prefix4T *)pOldData)->prefix.s_addr) < (((Prefix4T *)pNewData)->prefix.s_addr))
  {
    return 1;
  }
  /* same data */
  else
  {
    return -1; 
  }
}


/* Description : RouterID의 자료구조를 초기화 하는 함수. */
void
routerIdInit (void)
{
  pRibmgr->pRidAllSortedList = nnListInit (routerIdCmp, MEM_PREFIX);
  pRibmgr->pRidLoSortedList  = nnListInit (routerIdCmp, MEM_PREFIX);

  memset (&pRibmgr->ridUserAssigned, 0, sizeof (pRibmgr->ridUserAssigned));
  pRibmgr->ridUserAssigned.family = AF_INET;
  pRibmgr->ridUserAssigned.prefixLen = 32;
}


/* Description : RouterID List의 자료구조 비교함수 연결하는 함수. */
void
routerIdUpdate (void)
{
  nnListSetNodeCompFunc (pRibmgr->pRidAllSortedList, routerIdCmp);
  nnListSetNodeCompFunc (pRibmgr->pRidLoSortedList, routerIdCmp);
}

/* Description : RouterID의 자료구조의 메모리를 해제하는 함수. */
void
routerIdClose (void)
{
  nnListFree (pRibmgr->pRidAllSortedList);
  nnListFree (pRibmgr->pRidLoSortedList);
}


