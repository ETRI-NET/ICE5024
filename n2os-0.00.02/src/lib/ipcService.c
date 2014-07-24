/*******************************************************************************
 *            Electronics and Telecommunications Research Institute
 * Copyright: 2013 Electronics and Telecommunications Research Institute.
 *            All rights reserved.
 *            No part of this software shall be reproduced, stored in a
 *            retrieval system, or transmitted by any means, electronic,
 *            mechanical, photocopying, recording, or otherwise, without
 *            written permission from ETRI.
*******************************************************************************/
/**
 * @brief : IPC Service
 *  - Block Name : IPC Service Library
 *  - Process Name :
 *  - Creator : JaeSu Han
 *  - Initial Date : 2013/7/15
*/

/**
 * @file : ipcService.c
 *
 * $Id:
 * $Author:
 * $Date:
 * $Revision:
 * $LastChangedBy:
 **/

/*******************************************************************************
 *                        CONSTANTS / LITERALS / TYPES
 ******************************************************************************/

#include <unistd.h>
#include <errno.h>
#include "taskManager.h"

/*******************************************************************************
 *                              GLOBAL VARIABLES
 ******************************************************************************/


/*******************************************************************************
 *                         GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 *                         LOCAL CONSTANTS/LITERALS/TYPES
 ******************************************************************************/

// N2OS Component String (Ref. nnDefines.h)
const Int8T sIpcProcessString[][24] = {
  "IPC MANAGER",
  "PROCESS MANAGER",
  "PORT INTERFACE MANAGER",
  "RIB MANAGER",
  "POLICY MANAGER",
  "COMMAND MANAGER",
  "MULTICAST RIB MANAGER",
  "LIB MANAGER",
  "CHECKPOINT MANAGER",
  "LACP",
  "MSTP",
  "GVRP",
  "IGMP",
  "RIP",
  "ISIS",
  "OSPF",
  "BGP",
  "PIM",
  "RSVP",
  "LDP",
  "RIB_TESTER",
  "DLU_TESTER"

};


/*******************************************************************************
*                               LOCAL VARIABLES
*******************************************************************************/


/*******************************************************************************
 *                            LOCAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/

static void ipcProcessConnectListInit(void);
static Int8T ipcProcessConnectListCheck(Uint32T process);
static IpcMsgHeaderT * ipcRecvMsgSync (Int32T sock);
static Int32T ipcReadnSync (Int32T sock, StringT buf, Int32T readBytes);

/*******************************************************************************
 *                                   CODE
 ******************************************************************************/

/*******************************************************************************
 * Function: <ipcProcessConnectListInit>
 *
 * Description: <Initialize the process connect List>
 *
 * Parameters: none
 *
 * Returns: none
*******************************************************************************/

static void ipcProcessConnectListInit(void)
{
  Int32T i;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "Process Connect Info List Init Start\n");

  for (i=0; i<MAX_PROCESS_CNT; i++)
  {
    // IPC Common Management Structure Init.
    gIpcCommMgt[i].acceptFd = -1;
    gIpcCommMgt[i].bev = NULL;
    memset(&gIpcCommMgt[i].connectInfo, 0x00, sizeof(struct sockaddr));
    memset(&gIpcCommMgt[i].allocateInfo, 0x00, sizeof(struct sockaddr));
    gIpcCommMgt[i].bufSize = 0;
    gIpcCommMgt[i].buf = NULL;


    // Accept Process list init.
    gIpcAcceptMgt[i].fd = -1;
    memset(&gIpcAcceptMgt[i].addr, 0x00, sizeof(struct sockaddr));

    nLOGDEBUG(LOG_DEBUG, "gIpcCommMgt[%d].acceptFd = %d\n",
                          i, gIpcCommMgt[i].acceptFd);
    nLOGDEBUG(LOG_DEBUG, "gIpcCommMgt[%d].bev = %d\n",
                          i, gIpcCommMgt[i].bev);
    nLOGDEBUG(LOG_DEBUG, "gIpcCommMgt[%d].connectInfo = %u.%u.%u.%u, %d\n",
                          i,
                          IP_PRINT
                          (((struct sockaddr_in*)&gIpcCommMgt[i].connectInfo)
                             ->sin_addr.s_addr),
                           ((struct sockaddr_in*)&gIpcCommMgt[i].connectInfo)
                             ->sin_port);
    nLOGDEBUG(LOG_DEBUG, "gIpcCommMgt[%d].allocateInfo = %u.%u.%u.%u, %d\n",
                          i,
                          IP_PRINT
                          (((struct sockaddr_in*)&gIpcCommMgt[i].allocateInfo)
                             ->sin_addr.s_addr),
                           ((struct sockaddr_in*)&gIpcCommMgt[i].allocateInfo)
                             ->sin_port);
    nLOGDEBUG(LOG_DEBUG, "gIpcCommMgt[%d].bufSize = %d\n",
                          i, gIpcCommMgt[i].bufSize);
    nLOGDEBUG(LOG_DEBUG, "gIpcCommMgt[%d].buf = %d\n",
                          i, gIpcCommMgt[i].buf);
    nLOGDEBUG(LOG_DEBUG, "gIpcAcceptMgt[%d].fd = %d\n",
                          i, gIpcAcceptMgt[i].fd);
    nLOGDEBUG(LOG_DEBUG, "gIpcAcceptMgt[%d].addr = %u.%u.%u.%u, %d\n",
                          i,
                          IP_PRINT
                          (((struct sockaddr_in*)&gIpcAcceptMgt[i].addr)
                             ->sin_addr.s_addr),
                           ((struct sockaddr_in*)&gIpcAcceptMgt[i].addr)
                             ->sin_port);
  }


  nLOGDEBUG(LOG_INFO, "Process Connect Info List Init Complete\n");
  nLOGDEBUG(LOG_DEBUG, "################################\n");

}


/*
 * Description: Process 연결정보를 조회하는 함수
 *
 * @param [in] process : 조회할 Process
 * @param [out] processInfo : 조회된 Info 값
 *
 * @retval : SUCESS(0) : 해당 Process 정보가 있음
 *           FAILURE(-1) : 해당 Process 정보가 없음
 */

Int32T _ipcProcessInfoGet(Int32T process, struct sockaddr *processInfo)
{
    if(gIpcCommMgt[process].acceptFd == -1)
    {
        nLOGDEBUG(LOG_INFO, "Not Exist (Process Info : %s\n",
                             sIpcProcessString[process]);
        return FAILURE;
    }

    memcpy(processInfo, 
           &gIpcCommMgt[process].allocateInfo, 
           sizeof(struct sockaddr));

    nLOGDEBUG(LOG_DEBUG, 
              "allocateInfo : %u.%u.%u.%u, %d\n",
               IP_PRINT 
               (((struct sockaddr_in*)&gIpcCommMgt[process].allocateInfo)
                  ->sin_addr.s_addr),
                ((struct sockaddr_in*)&gIpcCommMgt[process].allocateInfo)
                  ->sin_port);

    nLOGDEBUG(LOG_DEBUG,
              "processInfo : %u.%u.%u.%u, %d\n",
               IP_PRINT
               (((struct sockaddr_in*)processInfo)->sin_addr.s_addr),
                ((struct sockaddr_in*)processInfo)->sin_port);

    return SUCCESS;
}


/*
 * Description: Process 연결정보 리스트에 Process 연결정보를 삭제하는 함수
 *
 * @param [in] fd : 삭제할 Process의 fd
 *
 * @retval : SUCCESS(0) : Process 삭제 성공,
 *           FAILURE(-1) : 해당 Process 정보가 없음
 */

Int32T ipcProcessInfoDelete(Int32T fd)
{
  Int32T i;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "Process Infomation Delete\n");
  nLOGDEBUG(LOG_INFO, "Input Fd : %d\n", fd);

  for(i=0; i<MAX_PROCESS_CNT; i++)
  {
    if(fd == gIpcCommMgt[i].acceptFd)
    {
      nLOGDEBUG(LOG_DEBUG, "[Before Information : %s]\n", sIpcProcessString[i]);
      nLOGDEBUG(LOG_DEBUG, "gIpcCommMgt[%d].acceptFd = %d\n",
                            i, gIpcCommMgt[i].acceptFd);
      nLOGDEBUG(LOG_DEBUG, "connectInfo : %u.%u.%u.%u, %d\n",
                            IP_PRINT
                            (((struct sockaddr_in*)&gIpcCommMgt[i].connectInfo)
                               ->sin_addr.s_addr),
                             ((struct sockaddr_in*)&gIpcCommMgt[i].connectInfo)
                               ->sin_port);
      nLOGDEBUG(LOG_DEBUG, "allocateInfo : %u.%u.%u.%u, %d\n",
                            IP_PRINT
                            (((struct sockaddr_in*)&gIpcCommMgt[i].allocateInfo)
                               ->sin_addr.s_addr),
                             ((struct sockaddr_in*)&gIpcCommMgt[i].allocateInfo)
                              ->sin_port);
      gIpcCommMgt[i].acceptFd = -1;

      if(gIpcCommMgt[i].bev != NULL)
      {
        bufferevent_free(gIpcCommMgt[i].bev);
        gIpcCommMgt[i].bev = NULL;
      }

      gIpcCommMgt[i].bufSize = 0;

      if(gIpcCommMgt[i].buf != NULL)
      {
        nFREE(IPC_MESSAGE, gIpcCommMgt[i].buf);
      }

      memset(&gIpcCommMgt[i].connectInfo, 0x00, sizeof(struct sockaddr));
      memset(&gIpcCommMgt[i].allocateInfo, 0x00, sizeof(struct sockaddr));

      nLOGDEBUG(LOG_DEBUG, "[After Information : %s]\n", sIpcProcessString[i]);
      nLOGDEBUG(LOG_DEBUG, "gIpcCommMgt[%d].acceptFd = %d\n",
                            i, gIpcCommMgt[i].acceptFd);
      nLOGDEBUG(LOG_DEBUG, "gIpcCommMgt[%d].bev = %d\n",
                             i, gIpcCommMgt[i].bev);
      nLOGDEBUG(LOG_DEBUG,"gIpcCommMgt[%d].connectInfo = %u.%u.%u.%u, %d\n",
                           i,
                           IP_PRINT
                           (((struct sockaddr_in*)&gIpcCommMgt[i].connectInfo)
                              ->sin_addr.s_addr),
                            ((struct sockaddr_in*)&gIpcCommMgt[i].connectInfo)
                              ->sin_port);
      nLOGDEBUG(LOG_DEBUG, "gIpcCommMgt[%d].allocateInfo = %u.%u.%u.%u, %d\n",
                            i,
                            IP_PRINT
                            (((struct sockaddr_in*)&gIpcCommMgt[i].allocateInfo)
                                ->sin_addr.s_addr),
                             ((struct sockaddr_in*)&gIpcCommMgt[i].allocateInfo)
                                ->sin_port);
      nLOGDEBUG(LOG_DEBUG, "gIpcCommMgt[%d].bufSize = %d\n",
                            i, gIpcCommMgt[i].bufSize);
      nLOGDEBUG(LOG_DEBUG, "gIpcCommMgt[%d].buf = %d\n",
                            i, gIpcCommMgt[i].buf);

      nLOGDEBUG(LOG_INFO, "Process Info Delete : Success\n");
      nLOGDEBUG(LOG_DEBUG, "################################\n");

      return SUCCESS;
    }
  }

  nLOGDEBUG(LOG_ERR, "Process Info Delete : Failure\n");
  nLOGDEBUG(LOG_DEBUG, "################################\n");

  return FAILURE;
}


/*******************************************************************************
 * Function: <ipcGetAcceptPoint>
 *
 * Description: <get Accepte process information>
 *
 * Parameters:
 *
 * Returns: Accepted Process Number,
 *          -1 : none process
*******************************************************************************/

Int32T ipcGetAcceptPoint(void)
{
  Int32T i;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "Accpet Point Check\n");

  for (i=0; i<MAX_PROCESS_CNT; i++)
  {
    if(gIpcAcceptMgt[i].fd == -1)
    {
      nLOGDEBUG(LOG_INFO, "gIpcAcceptMgt[%d] is Accepte Point\n", i);
      nLOGDEBUG(LOG_DEBUG, "################################\n");
      return i;
    }
  }

  nLOGDEBUG(LOG_ERR, "Full Accepte Point\n");
  nLOGDEBUG(LOG_DEBUG, "################################\n");
    
  return FAILURE;
}

/*******************************************************************************
 * Function: <ipcProcessConnectListCheck>
 *
 * Description: <Make sure that the process connetion information>
 *
 * Parameters: process : check process
 *
 * Returns:  0 : In connection information 
 *          -1 : No connection information
*******************************************************************************/

static Int8T ipcProcessConnectListCheck(Uint32T process)
{
  Int32T port
   = (((struct sockaddr_in*)&gIpcCommMgt[process].allocateInfo)->sin_port);

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "Process Info Check\n");
  nLOGDEBUG(LOG_INFO, "Process : %d, port : %d\n", process, port);

  if(port <= 0)
  {
    nLOGDEBUG(LOG_INFO, "Port <= 0\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return FAILURE;
  }
  else
  {
    nLOGDEBUG(LOG_INFO, "Port > 0\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return SUCCESS;
  }
}

/*******************************************************************************
 * Function: <ipcProcessConnectListUpdate>
 *
 * Description: <Updating the process connection list>
 *
 * Parameters: recvList : update the information
 *
 * Returns:  0 : Success
*******************************************************************************/

Int32T ipcProcessConnectListUpdate(struct sockaddr * recvList)
{
  Int32T i;
  static Int32T cnt = 0;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "Process Connect List Update\n");
  nLOGDEBUG(LOG_INFO, "Update Count (Before) : %d\n", cnt);
  cnt++;
  nLOGDEBUG(LOG_INFO, "Update Count (After - Current Cnt) : %d\n", cnt);

  for (i=0; i<MAX_PROCESS_CNT; i++)
  {
    if(((struct sockaddr_in*)&gIpcCommMgt[i].allocateInfo)->sin_port
          !=
       (((struct sockaddr_in*)recvList + i)->sin_port))
    {
      Int32T port;

      memcpy(&gIpcCommMgt[i].allocateInfo, 
             recvList + i, 
             sizeof(struct sockaddr));

      port = ((struct sockaddr_in*)&gIpcCommMgt[i].allocateInfo)->sin_port;

      nLOGDEBUG(LOG_INFO, "Update Process[%d] : %u.%u.%u.%u, %d\n",
                           i,
                           IP_PRINT
                           (((struct sockaddr_in*)&gIpcCommMgt[i].allocateInfo)
                             ->sin_addr.s_addr),
                            ((struct sockaddr_in*)&gIpcCommMgt[i].allocateInfo)
                             ->sin_port);

      if(port == 0)
      {
        nLOGDEBUG(LOG_INFO, "%s(%d) Connect List Del Start\n", 
                             sIpcProcessString[i], i);
        nLOGDEBUG(LOG_DEBUG, "(Before) gIpcCommMgt[%d].acceptFd = %d\n",
                              i, gIpcCommMgt[i].acceptFd);
        gIpcCommMgt[i].acceptFd = -1;
        nLOGDEBUG(LOG_DEBUG, "(After) gIpcCommMgt[%d].acceptFd = %d\n",
                              i, gIpcCommMgt[i].acceptFd);

        if(gIpcCommMgt[i].bev != NULL)
        {
          nLOGDEBUG(LOG_DEBUG, "(Before) gIpcCommMgt[%d].bev = %p\n",
                                i, gIpcCommMgt[i].bev);
          bufferevent_free(gIpcCommMgt[i].bev);
          gIpcCommMgt[i].bev = NULL;
          nLOGDEBUG(LOG_DEBUG, "(After) gIpcCommMgt[%d].bev = %p\n",
                                i, gIpcCommMgt[i].bev);
        }

        nLOGDEBUG(LOG_DEBUG, "(Before) gIpcCommMgt[%d].bufSize = %d\n",
                              i, gIpcCommMgt[i].bufSize);
        gIpcCommMgt[i].bufSize = 0;
        nLOGDEBUG(LOG_DEBUG, "(After) gIpcCommMgt[%d].bufSize = %d\n",
                              i, gIpcCommMgt[i].bufSize);

        if(gIpcCommMgt[i].buf != NULL)
        {
          nLOGDEBUG(LOG_DEBUG, "(Before) gIpcCommMgt[%d].buf = %p\n",
                                i, gIpcCommMgt[i].buf);
          nFREE(IPC_MESSAGE, gIpcCommMgt[i].buf);
          gIpcCommMgt[i].buf = NULL;
          nLOGDEBUG(LOG_DEBUG, "(After) gIpcCommMgt[%d].buf = %p\n",
                                i, gIpcCommMgt[i].buf);
        }

        nLOGDEBUG(LOG_INFO, "%s(%d) Connect List Del End\n", 
                             sIpcProcessString[i], i);           
      }
    }
  }

  nLOGDEBUG(LOG_INFO, "Process Connect List Update End\n");
  nLOGDEBUG(LOG_DEBUG, "################################\n");

  return SUCCESS;
}

/*******************************************************************************
 * Function: <ipcProcessConnectListUpdate>
 *
 * Description: <Updating fd of the process connection list>
 *
 * Parameters: fd : update the fd
 *             process : process
 *
 * Returns:  0 : Success
*******************************************************************************/

Int32T ipcProcessConnectListUpdateFd(Int32T fd, Uint32T process)
{
  Uint32T i;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "Process Connect List FD Update Start\n");
  nLOGDEBUG(LOG_INFO, "Input Process : %s(%d), fd : %d\n", 
                       sIpcProcessString[process], process, fd);

  gIpcCommMgt[process].acceptFd = fd;
 
  for(i=0; i<MAX_PROCESS_CNT; i++)
  {
    if(gIpcCommMgt[i].acceptFd != -1)
    {
      nLOGDEBUG(LOG_DEBUG, 
                "Current List : gIpcCommMgt[%d].acceptFd = %d\n",
                 i, gIpcCommMgt[i].acceptFd);
    }

    if(gIpcAcceptMgt[i].fd == fd)
    {
      nLOGDEBUG(LOG_DEBUG, "gIpcAcceptMgt[%d] List Init\n", i);
      gIpcAcceptMgt[i].fd = -1;
      memset(&gIpcAcceptMgt[i].addr, 0x00, sizeof(struct sockaddr));
    }
  }

  nLOGDEBUG(LOG_INFO, "Process Connect List FD Update End\n");
  nLOGDEBUG(LOG_DEBUG, "################################\n");

  return SUCCESS;
}

/*******************************************************************************
 * Function: <ipcGetProcess>
 *
 * Description: <get process id based on fd>
 *
 * Parameters: fd : Based on fd
 *
 * Returns: Process Id,
 *          -1 : none
*******************************************************************************/

Int32T ipcGetProcess(Int32T fd)
{
  Int32T i;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "Get Process\n");
  nLOGDEBUG(LOG_INFO, "Input fd : %d\n", fd);

  for(i=0; i<MAX_PROCESS_CNT; i++)
  {
    if (gIpcCommMgt[i].acceptFd == fd)
    {
      nLOGDEBUG(LOG_INFO, "Process : %s(%d)\n", sIpcProcessString[i], i);
      nLOGDEBUG(LOG_DEBUG, "################################\n");
      return i;
    }
  }

  nLOGDEBUG(LOG_ERR, "Get Process Failure\n");
  nLOGDEBUG(LOG_DEBUG, "################################\n");

  return FAILURE;
}

/*******************************************************************************
 * Function: <ipcGetFd>
 *
 * Description: <get fd based on process id>
 *
 * Parameters: process : Based on process id
 *
 * Returns: fd,
 *          -1 : none
*******************************************************************************/

Int32T ipcGetFd(Uint32T process)
{
  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "Get Fd\n");
  nLOGDEBUG(LOG_INFO, "Input Process : %s(%d)\n", 
                       sIpcProcessString[process], process);

  nLOGDEBUG(LOG_INFO, "Return Fd : %d\n", gIpcCommMgt[process].acceptFd);
  nLOGDEBUG(LOG_DEBUG, "################################\n");

  return gIpcCommMgt[process].acceptFd;
}

/**
 * Description: IPC Serivce Channel을 Open 하는 함수
 *
 * @param [in] process : 사용하려는 Process
 * @param [in] recvFunc : IPC Message 가 수신되었을 때, Callback 되는 함수
 *
 * @retval : SUCCESS(0) : 성공,
 *           IPC_ERR_ARGUMENT(-1) : 잘못된 인자 값,
 *           IPC_ERR_MGR_CONN(-2) : IPC Manager 가 연결되어 있지 않음,
 *           IPC_ERR_PENDING_INIT(-3) : Message Pending List 초기화 실패,
 *           IPC_ERR_MSG_REGIST(-10) : Process 등록 실패
 * @bug
 *  반드시, taskCreate()을 먼저 실행해야 한다.
 *
 * Example Code Usage
 * @code
 *  void * ipcCallback (int msgId, void * data, Uint32T dataLen)
 *  {
 *    ... // IPC MsgId에 따라 Message 처리
 *  }
 *
 *  Int main()
 *  {
 *    Int32T ret = 0;
 *
 *    ...
 *
 *    ret = _ipcChannelOpen(RIB_MANAGER, ipcCallback);
 *
 *    ...
 *  }
 * @endcode
 */

Int32T _ipcChannelOpen(Uint32T process, pIpcRegFunc recvFunc)
{
//    Int32T ret;
  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "Input Process : %s(%d)\n", 
                       sIpcProcessString[process], process);

  // 1. Arguement Check & Init

  if (recvFunc == NULL || process <= 0)
  {
    nLOGDEBUG(LOG_ERR, "recvFunc == NULL or Process(%d) <= 0\n", process);
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return IPC_ERR_ARGUMENT; // -1
  }

  gIpcCb = recvFunc;       // IPC Message Callback Func.
  gUserProcess = process;  // Use Process
  gListenerCheck = -1;     // Listener Check Flag

  nLOGDEBUG(LOG_INFO, 
           "IPC Message Callback Func, Use Process Set Success\n");

  gIpcMsgPendingList  // Message Pending List Init.
     = queueInit(IPC_MAX_PENDING_SIZE, CQUEUE_NOT_DUPLICATED, IPC_MESSAGE);
  if (gIpcMsgPendingList < 0)
  {
    nLOGDEBUG(LOG_ERR, "Message Pending List Init Failure : ret(%d)\n",
                        gIpcMsgPendingList);
    nLOGDEBUG(LOG_DEBUG, "################################\n");       
    return IPC_ERR_PENDING_INIT;  // -3
  }

  nLOGDEBUG(LOG_INFO, "Message Pending List Init Success\n");

  /* Message Send Sync Mode Setting END */
  ipcProcessConnectListInit();
  nLOGDEBUG(LOG_INFO, "IPC Connect List Init End\n");

  // 2. IPC Manager Connect & Registration

  if (ipcProcessConnect(IPC_MANAGER, IPC_MANAGER_IP, IPC_MANAGER_PORT) < 0)
  {
    nLOGDEBUG(LOG_ERR, "IPC Manager Connect Failure\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return IPC_ERR_MGR_CONN; // -2
  }

  nLOGDEBUG(LOG_INFO, "IPC Manager Connect Success\n");
/* 
    ret = ipcConnectRegist(gUserProcess, IPC_MANAGER);
//    ret = ipcConnectRegistSync
//           (gUserProcess, bufferevent_getfd(gIpcCommMgt[IPC_MANAGER].bev));
    if(ret < 0)
    {
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        nLOGDEBUG(LOG_ERR, "Send Regist Message (To. IPC Manager) Failure\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return IPC_ERR_MSG_REGIST; // -10
    }
 
    nLOGDEBUG(LOG_DEBUG, "################################\n"); 
    nLOGDEBUG(LOG_INFO, "Send Regist Message (To. IPC Manager) Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
 */
  return SUCCESS;
}

Int32T _ipcChannelUpdate(pIpcRegFunc recvFunc)
{
  gIpcCb = recvFunc;
  return SUCCESS;
}

/*******************************************************************************
 * Function: <ipcProcessConnect>  
 *  
 * Description: <Connect Process(Socket Connect)>  
 *  
 * Parameters: process : connect process  
 *             ip      : connect ip  
 *             port    : connect port  
 *  
 * Returns:  0 : SUCCESS,  
 *          -1 : FAILURE  
*******************************************************************************/

Int32T ipcProcessConnect(Uint32T process, StringT ip, Uint32T port)
{
  struct sockaddr_in ipcSin;
  Int32T ret;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "Process Connect Start\n");
  nLOGDEBUG(LOG_INFO, "Connect Process : %s(%d), ip : %s, port : %d\n", 
                       sIpcProcessString[process], process, ip, port);

  // 1. Connect info. Registration

  memset(&ipcSin, 0x00, sizeof(ipcSin));
  ipcSin.sin_family = AF_INET;// CASE : TCP
  ipcSin.sin_addr.s_addr = inet_addr(ip);
  ipcSin.sin_port = htons(port);

  nLOGDEBUG(LOG_INFO, "Connect info. Registration Success\n");


  // 2. new libevent socket create

  gIpcCommMgt[process].bev =
        bufferevent_socket_new(gTaskService, -1, BEV_OPT_CLOSE_ON_FREE);

  nLOGDEBUG(LOG_INFO, "new socket create Success\n");


  // 3. Socket register (read or event cb)

  bufferevent_setcb(gIpcCommMgt[process].bev,
                    ipcInterReadCb, NULL, ipcInterEventCb, NULL);
  bufferevent_enable(gIpcCommMgt[process].bev, EV_READ);

  nLOGDEBUG(LOG_INFO, "Socket register Success\n");

  while(1)
  {
    ret = bufferevent_socket_connect
         (gIpcCommMgt[process].bev,(struct sockaddr *)&ipcSin, sizeof(ipcSin));

    nLOGDEBUG(LOG_DEBUG, "ret : %d\n", ret);

    if(ret == SUCCESS)//FAILURE)
    {
      break;
    }
  }

  nLOGDEBUG(LOG_INFO, "ipcProcessConnect - ret : %d\n", ret);
  nLOGDEBUG(LOG_INFO, "socket : %d\n", 
                       bufferevent_getfd(gIpcCommMgt[process].bev));
  nLOGDEBUG(LOG_INFO, "Process Connect End\n");
  nLOGDEBUG(LOG_DEBUG, "################################\n");

  return SUCCESS;
}

/*******************************************************************************
 * Function: <ipcConnectRegist>  
 *  
 * Description: <Registration process at IPC Manager or Process (Async)>  
 *  
 * Parameters: process: registration process  
 *             target : target process  
 *  
 * Returns:   0 : SUCCESS,  
 *           -4 : IPC_ERR_MSG_CREATE,  
 *          -11 : IPC_ERR_MSG_SEND  
*******************************************************************************/

Int32T ipcConnectRegist (Uint32T process, Uint32T target)
{
  IpcMsgHeaderT *pMsg = (IpcMsgHeaderT *) 0;
  Int32T msgLen = IPC_MSG_HEADER_SIZE;
  IpcRegMsgT *pData = NULL;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "Send Regist Message (Async Mode)\n");
  nLOGDEBUG(LOG_INFO, "%s(%d) -> (%s)%d\n", 
                       sIpcProcessString[process], process, 
                       sIpcProcessString[target], target);

  // 1. Process Registration Message Header, Message init.

  msgLen = msgLen + sizeof(EventSubMsgT);
  nLOGDEBUG(LOG_DEBUG, "Regist Message Len : %d (Header(%d) + body(%d))\n",
                        msgLen, IPC_MSG_HEADER_SIZE, sizeof(EventSubMsgT));

  pMsg = (IpcMsgHeaderT *)nMALLOC(MESSAGE_HEADER, msgLen);
  if (pMsg <= 0)
  {
      nLOGDEBUG(LOG_ERR, "Message Header Create Failure\n");
      nLOGDEBUG(LOG_DEBUG, "################################\n");
      return IPC_ERR_MSG_CREATE;
  }
  nLOGDEBUG(LOG_DEBUG, "Message Header Create Success\n");

  pData = (IpcRegMsgT *)nMALLOC(IPC_PROCESS_REG, sizeof(IpcRegMsgT));
  if (pData <= 0)
  {
      nLOGDEBUG(LOG_ERR, "Message Body Create Failure\n");
      nLOGDEBUG(LOG_DEBUG, "################################\n");

      nFREE(MESSAGE_HEADER, pMsg);
      return IPC_ERR_MSG_CREATE;
  }
  nLOGDEBUG(LOG_DEBUG, "Message Body Create Success\n");


  // 2. Message context setting

  pMsg->type = IPC_PROCESS_REG;
  pMsg->msgId = -1;
  pMsg->length = msgLen;

  nLOGDEBUG(LOG_DEBUG, "Message Header Information\n");
  nLOGDEBUG(LOG_DEBUG, 
            "pMsg->type : %d, pMsg->msgId : %d, pMsg->length : %d\n",
             pMsg->type, pMsg->msgId, pMsg->length);

  pData->process = process;
  
  nLOGDEBUG(LOG_DEBUG, "Message Body Infomation\n");
  nLOGDEBUG(LOG_DEBUG, "source process : %s(%d)\n", 
                        sIpcProcessString[process], process);

  memcpy (pMsg->data, pData, sizeof(IpcRegMsgT));
  nLOGDEBUG(LOG_DEBUG, "Message Context Setting Success\n");


  // 3. Message send and delete

  if(gIpcCommMgt[target].bev != NULL)
  {
    bufferevent_write(gIpcCommMgt[target].bev, pMsg, pMsg->length);
    nLOGDEBUG(LOG_INFO, "Regist Message Send Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    nFREE(MESSAGE_HEADER, pMsg);
    nFREE(IPC_PROCESS_REG, pData);
    return SUCCESS;
  }
  else
  {
    nLOGDEBUG(LOG_ERR, "Send Failure - %s(%d) Not Connect\n", 
                        sIpcProcessString[target], target);
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    nFREE(MESSAGE_HEADER, pMsg);
    nFREE(IPC_PROCESS_REG, pData);
    return IPC_ERR_MSG_SEND;
  }
}

/*******************************************************************************
 * Function: <ipcConnectRegistSync>
 *
 * Description: <Registration process at IPC Manager or Process (Sync)>
 *
 * Parameters: process: registration process
 *             socket : target process socket
 *
 * Returns:  0 : SUCCESS,
 *          -4 : IPC_ERR_MSG_CREATE,
 *          -8 : IPC_ERR_MSG_SYNC_WRIT
*******************************************************************************/

Int32T ipcConnectRegistSync (Uint32T process, Int32T socket)
{
  IpcRegMsgT *pData = (IpcRegMsgT *) 0;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "Send Regist Message (Sync Mode)\n");
  nLOGDEBUG(LOG_INFO, "%s(%d) -> Regist Target socket : %d\n", 
                       sIpcProcessString[process], process, socket);


  // 1. Process Registration Message Message init.

  pData = (IpcRegMsgT *)nMALLOC(IPC_PROCESS_REG, sizeof(IpcRegMsgT));
  if (pData <= 0)
  {
    nLOGDEBUG(LOG_ERR, "Message Body Create Failure\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return IPC_ERR_MSG_CREATE;
  }
  nLOGDEBUG(LOG_DEBUG, "Message Body Create Success\n");


  // 2. Message context setting

  pData->process = process;

  nLOGDEBUG(LOG_DEBUG, "Message Body Infomation\n");
  nLOGDEBUG(LOG_DEBUG, "source process : %d\n", process);    


  // 3. Message Sync Mode Send
  
  if(ipcSendMsgSync
      (socket, IPC_PROCESS_REG, -1, pData, sizeof(IpcRegMsgT)) < 0)
  {
    nLOGDEBUG(LOG_ERR, "Send Failure - sock(%d)\n", socket);
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    nFREE(IPC_PROCESS_REG, pData);
    return IPC_ERR_MSG_SYNC_WRITE;
  }
   
  nLOGDEBUG(LOG_INFO, "Regist Message Send Success\n");
  nLOGDEBUG(LOG_DEBUG, "################################\n");

  nFREE(IPC_PROCESS_REG, pData);

  return SUCCESS;
}

/*******************************************************************************
 * Function: <ipcChannelConnect>  
 *  
 * Description: <IPC Channel Connect Open(assigned address)>  
 *  
 * Parameters: ip   : assigned ip  
 *             port : assigned port  
 *  
 * Returns:   0 : SUCCESS,  
 *           -1 : FAILURE,
 *          -11 : IPC_ERR_MSG_SEND  
*******************************************************************************/

Int32T ipcChannelConnect(StringT ip, Uint32T port)
{
  struct sockaddr_in sin;
  memset(&sin, 0x00, sizeof(sin));

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "IPC Channel listener bind Start\n");
  nLOGDEBUG(LOG_INFO, "ip : %s, port : %d\n", ip, port);

  
  if(gListenerCheck == 1)
  {
    if(ipcBindRegist() < 0)
    {
      nLOGDEBUG(LOG_ERR, "Bind Complete Message Send Failure\n");
      nLOGDEBUG(LOG_DEBUG, "################################\n");
      return IPC_ERR_MSG_SEND;
    }
    
    nLOGDEBUG(LOG_INFO, "Bind Complete Message Send Success\n");
    return SUCCESS;
  }


  // 1. Process connect info. Registration

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = inet_addr(ip);
  sin.sin_port = htons(port);

  nLOGDEBUG(LOG_INFO, "listener bind info. Registration Success\n");


  // 2. Bind and Callback Function Registration

  gListener = evconnlistener_new_bind
              (gTaskService, ipcChannelAccept, NULL,
               LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,
               (struct sockaddr*)&sin, sizeof(sin));
  if (!gListener)
  {
    nLOGDEBUG(LOG_ERR, "Couldn't create listener\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return FAILURE;
  }

  evconnlistener_set_error_cb(gListener, ipcAcceptErrorFunc);

  nLOGDEBUG(LOG_INFO, "ip : %s, port : %d\n", ip, port);
  nLOGDEBUG(LOG_INFO, "IPC Channel Connect (listener bind) Success\n");

  gListenerCheck = 1;
 
  if(ipcBindRegist() < 0)
  {
    nLOGDEBUG(LOG_ERR, "Bind Complete Message Send Failure\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return IPC_ERR_MSG_SEND;   
  }
  else
  {
    nLOGDEBUG(LOG_INFO, "Bind Complete Message Send Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return SUCCESS;
  }
}

/*******************************************************************************
 * Function: <ipcBindRegist>  
 *  
 * Description: <Bind Registration process at IPC Manager>  
 *  
 * Parameters: none  
 *  
 * Returns:   0 : SUCCESS,  
 *           -4 : IPC_ERR_MSG_CREATE,  
 *          -11 : IPC_ERR_MSG_SEND  
*******************************************************************************/

Int32T ipcBindRegist (void)
{
  IpcMsgHeaderT *pMsg = (IpcMsgHeaderT *) 0;
  Int32T msgLen = IPC_MSG_HEADER_SIZE;
  IpcRegMsgT *pData = NULL;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "Send Bind Complete Message\n");


  // 1. Process Registration Message Header, Message init.

  msgLen = msgLen + sizeof(IpcRegMsgT);
  nLOGDEBUG(LOG_DEBUG, "Message Len = %d (Header(%d) + body(%d))\n",
                        msgLen, IPC_MSG_HEADER_SIZE, sizeof(IpcRegMsgT));;

  pMsg = (IpcMsgHeaderT *)nMALLOC(MESSAGE_HEADER, msgLen);
  if (pMsg <= 0)
  {
    nLOGDEBUG(LOG_ERR, "Message Header Create Failure\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return IPC_ERR_MSG_CREATE;
  }
  nLOGDEBUG(LOG_DEBUG, "Message Header Create Success\n");

  pData = (IpcRegMsgT *)nMALLOC(IPC_PROCESS_REG, sizeof(IpcRegMsgT));
  if (pData <= 0)
  {
    nLOGDEBUG(LOG_ERR, "Message Body Create Failure\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    nFREE(MESSAGE_HEADER, pMsg);
    return IPC_ERR_MSG_CREATE;
  }
  nLOGDEBUG(LOG_DEBUG, "Message Body Create Success\n");


  // 2. Message context setting

  pMsg->type = IPC_BIND_COMPLETE;
  pMsg->msgId = -1;
  pMsg->length = msgLen;

  nLOGDEBUG(LOG_DEBUG, "Message Header Information\n");
  nLOGDEBUG(LOG_DEBUG, 
            "pMsg->type : %d, pMsg->msgId : %d, pMsg->length : %d\n",
             pMsg->type, pMsg->msgId, pMsg->length);

  pData->process = gUserProcess;

  nLOGDEBUG(LOG_DEBUG, "Message Body Infomation\n");
  nLOGDEBUG(LOG_DEBUG, "process : %s(%d)", 
                        sIpcProcessString[pData->process], pData->process);

  memcpy (pMsg->data, pData, sizeof(IpcRegMsgT));

  nLOGDEBUG(LOG_DEBUG, "Message Context Setting Success\n");


  // 3. Message send and delete

  if(gIpcCommMgt[IPC_MANAGER].bev != NULL)
  {
    bufferevent_write(gIpcCommMgt[IPC_MANAGER].bev, pMsg, pMsg->length);
    nLOGDEBUG(LOG_INFO, "Bind Complete Message Send Success\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    nFREE(MESSAGE_HEADER, pMsg);
    nFREE(IPC_PROCESS_REG, pData);

    return SUCCESS;
  }
  else
  {
    nLOGDEBUG(LOG_ERR, "Message Send Failure - IPC Manager Not Connect\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
        
    nFREE(MESSAGE_HEADER, pMsg);
    nFREE(IPC_PROCESS_REG, pData);
 
    return IPC_ERR_MSG_SEND;
  }
}

/*******************************************************************************
 * Function: <ipcProcessInfoRequestSync>
 *
 * Description: <Process Info Request(Sync)>
 *
 * Parameters: process: Requeset Process Info 
 *
 * Returns:  0 : SUCCESS,
 *          -2 : IPC_ERR_MGR_CONN,
 *          -4 : IPC_ERR_MSG_CREATE,
 *          -8 : IPC_ERR_MSG_SYNC_WRIT
*******************************************************************************/

Int32T ipcProcessInfoRequestSync (Uint32T process)
{
  IpcRegMsgT *pData = (IpcRegMsgT *) 0;
  Int32T socket = ipcGetFd(IPC_MANAGER);
  Int32T msgType;
  Int32T ret;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "Send Process Info Request (Sync Mode)\n");
  nLOGDEBUG(LOG_INFO, "IPC Manager Socket : %d\n", socket);
  nLOGDEBUG(LOG_INFO, "Request Process : %s(%d)\n", 
                       sIpcProcessString[process], process);

  // 1. IPC Manager Connect Check
   
  if(gIpcCommMgt[IPC_MANAGER].bev == NULL)
  {
    nLOGDEBUG(LOG_ERR, "IPC Manager Not Connected\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return IPC_ERR_MGR_CONN;
  }

  // 2. Process Registration Message Message init.

  pData = (IpcRegMsgT *)nMALLOC(IPC_PROCESS_REG, sizeof(IpcRegMsgT));
  if (pData <= 0)
  {
    nLOGDEBUG(LOG_ERR, "Message Body Create Failure\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return IPC_ERR_MSG_CREATE;
  }
  nLOGDEBUG(LOG_DEBUG, "Message Body Create Success\n");

  // 3. Message context setting

  pData->process = process;

  nLOGDEBUG(LOG_DEBUG, "Message Body Infomation\n");
  nLOGDEBUG(LOG_DEBUG, "source process : %s(%d)\n", 
                        sIpcProcessString[process], process);

  // 4. Message Sync Mode Send

  bufferevent_disable(gIpcCommMgt[IPC_MANAGER].bev, EV_READ);
  ret = ipcSendMsgSync
        (socket, IPC_PROC_SINGAL_INFO_REQ, -1, pData, sizeof(IpcRegMsgT));
  if(ret < 0)
  {
    nLOGDEBUG(LOG_ERR, "Send Failure - sock(%d)\n", socket);
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    nFREE(IPC_PROCESS_REG, pData);
    bufferevent_enable(gIpcCommMgt[IPC_MANAGER].bev, EV_READ);
    return ret;
  }
  nLOGDEBUG(LOG_INFO, "Regist Message Send Success\n");
  nFREE(IPC_PROCESS_REG, pData);


  // 5. Sync Message Receive

  do{
      IpcMsgHeaderT * pRecvMsg = NULL;

      pRecvMsg = ipcRecvMsgSync(socket);
      if(pRecvMsg < 0)
      {
        nLOGDEBUG(LOG_ERR, "Message Received (Sync) Failure\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        bufferevent_enable(gIpcCommMgt[IPC_MANAGER].bev, EV_READ);
        return IPC_ERR_MSG_SYNC_FAILURE;
      }

      msgType = pRecvMsg->type;
      nLOGDEBUG(LOG_INFO, "Received Message Type : %d\n", msgType);

      if(msgType != IPC_PROC_SINGLE_INFO_UPDATE) // not waiting for a Message
      {
        nLOGDEBUG(LOG_INFO, 
                  "msgType(%d) != IPC_PROC_SINGLE_INFO_UPDATE(%d) \n", 
                   msgType, IPC_PROC_SINGLE_INFO_UPDATE);
        ret = queueEnqueue(gIpcMsgPendingList, pRecvMsg);
        if(ret < 0)
        {
          nLOGDEBUG(LOG_ERR, "Message Pending Failure\n");
        }

        nLOGDEBUG(LOG_ERR, "Message(Type : %d) Pending Success\n", msgType);
      }
      else // waiting for a Message
      {
        nLOGDEBUG(LOG_INFO, "Waiting for a Message\n");
          
        nLOGDEBUG(LOG_INFO, "process : %s(%d)\n", 
                            sIpcProcessString[process], process);
        memcpy(&gIpcCommMgt[process].allocateInfo,
                pRecvMsg->data, sizeof(struct sockaddr));

        nLOGDEBUG(LOG_INFO, "Update Process : %u.%u.%u.%u, %d\n",
                  IP_PRINT
                  (((struct sockaddr_in*)&gIpcCommMgt[process].allocateInfo)
                      ->sin_addr.s_addr),
                   ((struct sockaddr_in*)&gIpcCommMgt[process].allocateInfo)
                      ->sin_port);

        nLOGDEBUG(LOG_INFO, "Output Message Copy Complete\n");

        nFREE(IPC_MESSAGE, pRecvMsg);
      }

  } while(msgType != IPC_PROC_SINGLE_INFO_UPDATE);

  bufferevent_enable(gIpcCommMgt[IPC_MANAGER].bev, EV_READ);

  nLOGDEBUG(LOG_INFO, " Process Info Request Success\n");
  nLOGDEBUG(LOG_DEBUG, "################################\n");    

  return SUCCESS;
}


/**
 * Description: IPC Message Async 방식 Send 함수
 *
 * @param [in] process : 전송대상 Process
 * @param [in] messageId : Message ID
 * @param [in] message : 전송하려는 Message Data
 * @param [in] size : Data 크기
 *
 * @retval : SUCCESS(0) : 성공,
 *           IPC_ERR_MSG_CREATE(-4) : Message 생성 실패,
 *           IPC_ERR_MSG_SIZE_BIG(-6) : Message 크기가 큼
 *
 * @bug
 *  반드시, _ipcChannelOpen()을 먼저 실행해야 한다.
 *
 * Example Code Usage
 * @code
 *  void sendMessage(void)
 *  {
 *    Int32T ret;
 *    Int8T sndBuf[4096] = {};
 *    Int32T sndLength;
 *  
 *    ... // send message 설정
 *    
 *    ret = _ipcSendAsync(RIB_MANAGER, RIB_IPV4_ROUTE_ADD, sndBuf, sndLength); 
 *
 *    ...
 *
 *  }
 * @endcode
 */

Int32T _ipcSendAsync
    (Uint32T process, Uint32T messageId, void * message, Uint32T size)
{
  IpcMsgHeaderT *pMsg = (IpcMsgHeaderT *) 0;
  int msgLen = IPC_MSG_HEADER_SIZE + size;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "IPC Send Async Start : %s(%d) -> %s(%d)\n",
                       sIpcProcessString[gUserProcess], gUserProcess,
                       sIpcProcessString[process], process);
  nLOGDEBUG(LOG_INFO, "messageId : %d, data : %p, dataSize : %d\n",
                       messageId, message, size);
  nLOGDEBUG(LOG_DEBUG, "################################\n");


  // 1. Message Create

  if(msgLen > IPC_MAX_MESSAGE_SIZE)
  {
    nLOGDEBUG(LOG_ERR, 
              "Total Message Size : %d > IPC_MAX_MESSAGE_SIZE(%d)\n",
               msgLen, IPC_MAX_MESSAGE_SIZE);
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return IPC_ERR_MSG_SIZE_BIG;
  }

  pMsg = (IpcMsgHeaderT *)nMALLOC(MESSAGE_HEADER, msgLen);
  if(pMsg <= 0)
  {
    nLOGDEBUG(LOG_ERR, "Message Header Create Failure\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return IPC_ERR_MSG_CREATE;
  }
  nLOGDEBUG(LOG_DEBUG, "Message Header Create Success\n");


  // 2. Message Setting

  pMsg->type = IPC_SERVICE;
  pMsg->msgId = messageId;
  pMsg->length = msgLen;

  nLOGDEBUG(LOG_DEBUG, "Message Header Information\n");
  nLOGDEBUG(LOG_DEBUG, 
            "pMsg->type : %d, pMsg->msgId : %d, pMsg->length : %d\n",
             pMsg->type, pMsg->msgId, pMsg->length);

  if (message != NULL && size != 0)
  {
    memcpy(pMsg->data, message, size);
  }
  nLOGDEBUG(LOG_DEBUG, "Message Context Setting Success\n");


  // 3. Process Connect Check

  nLOGDEBUG(LOG_DEBUG, "gIpcCommMgt[%d].bev = %p\n",
                        process, gIpcCommMgt[process].bev);

  if(gIpcCommMgt[process].bev == NULL)
  {
    // Process Disconnected (Not Information)
    nLOGDEBUG(LOG_INFO, "Process Disconnected\n");

    // Request : Process Connect Info.
    if(ipcProcessInfoRequestSync(process) < 0)
    {
      nLOGDEBUG(LOG_INFO, "Process Info Received Failure\n");
      nLOGDEBUG(LOG_DEBUG, "################################\n");

      nFREE(MESSAGE_HEADER, pMsg);           
      return IPC_ERR_MSG_SYNC_FAILURE;        
    }

    // Check : Process Connect Info.
    if(ipcProcessConnectListCheck(process) < 0)
    {
      nLOGDEBUG(LOG_INFO, "Process Not Information\n");
      nLOGDEBUG(LOG_DEBUG, "################################\n");

      nFREE(MESSAGE_HEADER, pMsg);
      return IPC_ERR_PROCESS_NOT_CONN;
    }
    else // Process Connect
    {
      Int8T ip[16] = {0, };
      Int32T port = -1;
      Int32T socket = -1;

      sprintf(ip, "%u.%u.%u.%u",
                   IP_PRINT
                   (((struct sockaddr_in*)&gIpcCommMgt[process].allocateInfo)
                       ->sin_addr.s_addr));

      port = (((struct sockaddr_in*)&gIpcCommMgt[process].allocateInfo)
                 ->sin_port);

      nLOGDEBUG(LOG_INFO, "Process Connect Info - ip : %s, port : %d\n",
                           ip, port);
 
      if(ipcProcessConnect(process, ip, port) < 0)
      {
        nLOGDEBUG(LOG_ERR, "Process Connect Failure\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        nFREE(MESSAGE_HEADER, pMsg);
        return IPC_ERR_PROCESS_NOT_CONN;      
      }

      nLOGDEBUG(LOG_INFO, "Process Connect Success\n");
        
      socket = bufferevent_getfd(gIpcCommMgt[process].bev);
      nLOGDEBUG(LOG_INFO, "Regist Socket : %d\n", socket);
   
      if(ipcConnectRegistSync(gUserProcess, socket) < 0)
      {
        nLOGDEBUG(LOG_ERR, "Regist Message Send Failure\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        nFREE(MESSAGE_HEADER, pMsg);
        return IPC_ERR_PROCESS_NOT_CONN;
      }
    }
  } 

  // 4. Async Message Send and Free

  bufferevent_write(gIpcCommMgt[process].bev, pMsg, pMsg->length);
  nFREE(MESSAGE_HEADER, pMsg);

  nLOGDEBUG(LOG_INFO, "IPC Send Async Success\n");
  nLOGDEBUG(LOG_DEBUG, "################################\n");

  return SUCCESS;
}

/**
 * Description: IPC Message Sync 방식 Send 함수
 *
 * @param [in] process : 전송대상 Process
 * @param [in] messageId : Message ID
 * @param [in] retId : Return Message ID
 * @param [in] message : 전송하려는 Message Data
 * @param [in] size : Data 크기
 * @param [out] outBuff : 전달받은 Message Data를 저장할 공간
 * @param [out] outSize : 전달받은 Message Data의 크기
 *
 * @retval : SUCCESS(0) : 성공,
 *           IPC_ERR_MSG_CREATE(-4) : Message 생성 실패,
 *           IPC_ERR_PROCESS_NOT_CONN(-5) : 전송할 Process가 연결되지 않음,
 *           IPC_ERR_MSG_SYNC_WRITE(-8) : Sync Message 전송 실패,
 *           IPC_ERR_MSG_SYNC_READ(-9) : Response Message 응답 실패
 *
 * @bug
 *  반드시, _ipcChannelOpen()을 먼저 실행해야 한다.
 *
 * Example Code Usage
 * @code
 *  void sendMessage(void)
 *  {
 *    Int32T ret;
 *    Int8T sndBuf[4096] = {};
 *    Int32T sndLength;
 *    Int8T revBuf[4096] = {};   
 *    Int32T revLength;
 *  
 *    ... // send message 설정
 *    
 *    ret = _ipcSendSync
 *          (RIB_MANAGER, RIB_IPV4_ROUTE_ADD, RIB_IPV4_ROUTE_DEL, 
 *           sndBuf, sndLength, rcvBuf, &rcvLength); 
 *
 *    ... // recv message 처리
 *    
 *    _ipcProcessingPendingMsg(); // Pending Message 처리
 *   
 *    ...
 *
 *  }
 * @endcode
 */

Int32T _ipcSendSync
(Uint32T process, Uint32T messageId, Uint32T retId, 
 void * message, Uint32T size, StringT outBuff, Uint32T * outSize)
{
  Int32T socket;
  Int32T ret;
  Uint32T msgId;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "IPC Send Sync Start : %s(%d) -> %s(%d)\n",
                       sIpcProcessString[gUserProcess], gUserProcess, 
                       sIpcProcessString[process], process);
  nLOGDEBUG(LOG_INFO, "messageId : %d, data : %p, dataSize : %d\n",
                       messageId, message, size);
  nLOGDEBUG(LOG_DEBUG, "gIpcCommMgt[%d].bev = %p\n",
                        process, gIpcCommMgt[process].bev);

  if(gIpcCommMgt[process].bev == NULL)
  {
    // Process Disconnected (Not Information)

    nLOGDEBUG(LOG_INFO, "Process Disconnected\n");

    // Request : Process Connect Info.
    if(ipcProcessInfoRequestSync(process) < 0)
    {
      nLOGDEBUG(LOG_INFO, "Process Info Received Failure\n");
      nLOGDEBUG(LOG_DEBUG, "################################\n");

      return IPC_ERR_MSG_SYNC_FAILURE;
    }

    if(ipcProcessConnectListCheck(process) < 0)
    {
      nLOGDEBUG(LOG_INFO, "Process Not Information\n");
      nLOGDEBUG(LOG_DEBUG, "################################\n");

      return IPC_ERR_PROCESS_NOT_CONN;
    }
    else // Process Connect
    {
      Int8T ip[16] = {0, };
      Int32T port = -1;

      sprintf(ip, "%u.%u.%u.%u",
                   IP_PRINT
                   (((struct sockaddr_in*)&gIpcCommMgt[process].allocateInfo)
                       ->sin_addr.s_addr));

      port = (((struct sockaddr_in*)&gIpcCommMgt[process].allocateInfo)
                 ->sin_port);

      nLOGDEBUG(LOG_INFO, "Process Connect Info - ip : %s, port : %d\n",
                           ip, port);
 
      if(ipcProcessConnect(process, ip, port) < 0)
      {
        nLOGDEBUG(LOG_ERR, "Process Connect Failure\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");

        return IPC_ERR_PROCESS_NOT_CONN;
      }
      nLOGDEBUG(LOG_INFO, "Process Connect Success\n");

      socket = bufferevent_getfd(gIpcCommMgt[process].bev);
      nLOGDEBUG(LOG_INFO, "Regist Socket : %d\n", socket);

      if(ipcConnectRegistSync(gUserProcess, socket) < 0)
      {
        nLOGDEBUG(LOG_ERR, "Regist Message Send Failure\n");
        nLOGDEBUG(LOG_DEBUG, "################################\n");
        return IPC_ERR_PROCESS_NOT_CONN;
      }
    }
  }

  socket = bufferevent_getfd(gIpcCommMgt[process].bev);
  nLOGDEBUG(LOG_INFO, "Send Process Socket : %d\n", socket);

  bufferevent_disable(gIpcCommMgt[process].bev, EV_READ);
             
  // 1. Sync Message Send

  ret = ipcSendMsgSync(socket, IPC_SERVICE, messageId, message, size);
  if(ret < 0)
  {
    nLOGDEBUG(LOG_ERR, "Message Send (Sync) Failure\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    bufferevent_enable(gIpcCommMgt[process].bev, EV_READ);
    return ret;
  }

  // 2. Sync Message Receive

  do{
    IpcMsgHeaderT * pRecvMsg = NULL;

    pRecvMsg = ipcRecvMsgSync(socket);
    if(pRecvMsg < 0)
    {
      nLOGDEBUG(LOG_ERR, "Message Received (Sync) Failure\n");
      nLOGDEBUG(LOG_DEBUG, "################################\n");

      bufferevent_enable(gIpcCommMgt[process].bev, EV_READ);
      return IPC_ERR_MSG_SYNC_FAILURE;
    }

    msgId = pRecvMsg->msgId;
    nLOGDEBUG(LOG_INFO, "Received Message Id : %d\n", msgId);

    if(msgId != retId) // not waiting for a Message
    {
      nLOGDEBUG(LOG_INFO, "msgId(%d) != retId(%d)\n", msgId, retId);
      ret = queueEnqueue(gIpcMsgPendingList, pRecvMsg);
      if(ret < 0)
      {
        nLOGDEBUG(LOG_ERR, "Message Pending Failure\n");
      }

      nLOGDEBUG(LOG_ERR, "Message(Id : %d) Pending Success\n", msgId);
    }
    else // waiting for a Message
    {
      nLOGDEBUG(LOG_INFO, "Waiting for a Message\n");
      nLOGDEBUG(LOG_INFO, "msgId(%d) == retId(%d)\n", msgId, retId);
      *outSize = pRecvMsg->length - IPC_MSG_HEADER_SIZE;
      memcpy(outBuff, pRecvMsg->data, *outSize);

      nLOGDEBUG(LOG_INFO, "Output Message Copy Complete\n");
      nLOGDEBUG(LOG_INFO, "Message size = %d\n", *outSize);

      nFREE(IPC_MESSAGE, pRecvMsg);
    }
  } while(msgId != retId);

  bufferevent_enable(gIpcCommMgt[process].bev, EV_READ);

  nLOGDEBUG(LOG_INFO, "IPC Sync Send/Receiv Success\n");
  nLOGDEBUG(LOG_DEBUG, "################################\n");

  return SUCCESS;
}

/**
 * Description: 전달받은 Sync 방식 IPC Message 에 Response Message 전송 함수
 *
 * @param [in] process : 전송대상 Process
 * @param [in] messageId : Response Message ID
 * @param [in] message : 전송하려는 Message Data
 * @param [in] size : Data 크기
 *
 * @retval : SUCCESS(0) : 성공,
 *           IPC_ERR_MSG_CREATE(-4) : Message 생성 실패,
 *           IPC_ERR_PROCESS_NOT_CONN(-5) : 전송할 Process가 연결되지 않음,
 *           IPC_ERR_MSG_SYNC_WRITE(-8) : Sync Message 전송 실패
 *
 * Example Code Usage
 * @code
 *  void responseMessage(void)
 *  {
 *    Int32T ret;
 *    Int8T sndBuf[4096] = {};
 *    Int32T sndLength;
 *  
 *    ... // send message 설정
 *    
 *    ret = _ipcResponseSync(RIP, RIB_IPV4_ROUTE_DEL, sndBuf, sndLength);
 *
 *    ...
 *
 *  }
 * @endcode
 */

Int32T _ipcResponseSync
(Uint32T process, Uint32T messageId, void * message, Uint32T size)
{
  Int32T socket;
  Int32T ret;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, 
            "IPC Response Sync Start : %s(%d) -> %s(%d)\n",
             sIpcProcessString[gUserProcess], gUserProcess, 
             sIpcProcessString[process], process);
  nLOGDEBUG(LOG_INFO, "messageId : %d, data : %p, dataSize : %d\n",
                       messageId, message, size);

  socket = ipcGetFd(process);
  nLOGDEBUG(LOG_INFO, "Sync Message Response. get socket fd : %d\n", socket);


  // Sync Message Send

  ret = ipcSendMsgSync(socket, IPC_SERVICE, messageId, message, size);
  if(ret < 0)
  {
    nLOGDEBUG(LOG_ERR, "Response Message Send (Sync) Failure\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return ret;
  }

  nLOGDEBUG(LOG_INFO, "IPC Response Message Send Success\n");
  nLOGDEBUG(LOG_DEBUG, "################################\n");

  return SUCCESS;
}

/*******************************************************************************
 * Function: <ipcSendMsgSync>  
 *  
 * Description: <IPC Send Message : Sync Mode>  
 *  
 * Parameters: scoket    : Target Process Socket  
 *             type      : Sync Message Type  
 *             messageId : Message ID  
 *             message   : Send Message  
 *             size      : message size  
 *  
 * Returns:  0 : SUCCESS,  
 *          -4 : IPC_ERR_MSG_CREATE,  
 *          -8 : IPC_ERR_MSG_SYNC_WRITE  
*******************************************************************************/

Int32T ipcSendMsgSync
(Int32T socket, Uint16T type, Uint32T messageId, void * message, Uint32T size)
{
  IpcMsgHeaderT *pMsg = (IpcMsgHeaderT *) 0;
  Int32T msgLen = IPC_MSG_HEADER_SIZE + size;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, 
           "IPC Sync Message Send Start : %s(%d) -> socket(%d)\n",
            sIpcProcessString[gUserProcess], gUserProcess, socket);
  nLOGDEBUG(LOG_INFO, "Message Type : %d, Message Id : %d, Data Size : %d\n",
                       type, messageId, size);

  nLOGDEBUG(LOG_INFO, "Message Len (%d) = Header(%d) + Data Size(%d)\n",
                       msgLen, IPC_MSG_HEADER_SIZE, size);


  // 1. Message Creation

  pMsg = (IpcMsgHeaderT *)nMALLOC(MESSAGE_HEADER, msgLen);
  if(pMsg <= 0)
  {
    nLOGDEBUG(LOG_ERR, "Message Header Create Failure\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return IPC_ERR_MSG_CREATE;
  }
  nLOGDEBUG(LOG_DEBUG, "Message Header Create Success\n");

  
  // 2. Message Setting

  pMsg->type = type;
  pMsg->msgId = messageId;
  pMsg->length = msgLen;

  nLOGDEBUG(LOG_DEBUG, "Message Header Information\n");
  nLOGDEBUG(LOG_DEBUG, 
            "pMsg->type : %d, pMsg->msgId : %d, pMsg->length : %d\n",
             pMsg->type, pMsg->msgId, pMsg->length);

  if (message != NULL && size != 0)
  {
    memcpy(pMsg->data, message, size);
  }
  nLOGDEBUG(LOG_DEBUG, "Message Context Setting Success\n");


  // 3. Message Send and free

  if(write(socket, pMsg, pMsg->length) < 0)
  {
    nLOGDEBUG(LOG_ERR, "Message Send Failure - write()\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n"); 

    nFREE(MESSAGE_HEADER, pMsg);
    return IPC_ERR_MSG_SYNC_WRITE;
  }

  nLOGDEBUG(LOG_INFO, "IPC Sync Message Send Send Success\n");
  nLOGDEBUG(LOG_DEBUG, "################################\n"); 

  nFREE(MESSAGE_HEADER, pMsg);
  return SUCCESS;
}

/*******************************************************************************
 * Function: <ipcRecvMsgSync>  
 *  
 * Description: <IPC Receive Message : Sync Mode>  
 *  
 * Parameters: scoket : Send Process Socket  
 *  
 * Returns:  0 : SUCCESS,  
 *          -4 : IPC_ERR_MSG_CREATE,  
 *          -9 : IPC_ERR_MSG_SYNC_READ  
*******************************************************************************/

static IpcMsgHeaderT * ipcRecvMsgSync(Int32T sock)
{
  Int32T nBytes = 0;
  Int32T msgLen = 0;
  StringT msgBuff;
  struct IpcMsgHeaderT * pMsg;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "IPC Message Recv(Sync) Start : socket(%d)\n", socket);

  // 1. Read the Message Header
 
  msgBuff = nMALLOC(IPC_MESSAGE, IPC_MSG_HEADER_SIZE);
  if(msgBuff < 0)
  {
    nLOGDEBUG(LOG_ERR, "Message Header Create Failure\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return (IpcMsgHeaderT *) IPC_ERR_MSG_CREATE;
  }
  nLOGDEBUG(LOG_DEBUG, "Message Header Create Success\n");

  nBytes = ipcReadnSync (sock, msgBuff, IPC_MSG_HEADER_SIZE);
  if(nBytes <= 0)
  {
    nLOGDEBUG(LOG_ERR, "Message Sync Read Failure\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    nFREE(IPC_MESSAGE, msgBuff);
    return (IpcMsgHeaderT *) IPC_ERR_MSG_SYNC_READ;
  }
  nLOGDEBUG(LOG_DEBUG, "Message Header Size Read Success\n");


  // 2. Check the length of the Message

  pMsg = (struct IpcMsgHeaderT *) msgBuff;
  msgLen = pMsg->length;
    
  nLOGDEBUG(LOG_DEBUG, "Message Len : %d\n", msgLen);

  if(msgLen == IPC_MSG_HEADER_SIZE)
  {
      nLOGDEBUG(LOG_INFO, "msgLen == IPC_MSG_HEADER_SIZE\n");
      nLOGDEBUG(LOG_INFO, "Message Received Success\n");
      nLOGDEBUG(LOG_DEBUG, "################################\n");
      return (IpcMsgHeaderT *) msgBuff;
  }


  // 3. Reassigned to the length of the Message

  msgBuff = nREALLOC(IPC_MESSAGE, msgBuff, msgLen);
  if(msgBuff < 0)
  {
    nLOGDEBUG(LOG_ERR, "Message Create Failure\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return (IpcMsgHeaderT *) IPC_ERR_MSG_CREATE;
  }

  // 4. Read the User Data

  nBytes = ipcReadnSync (sock, msgBuff+IPC_MSG_HEADER_SIZE,
                         msgLen - IPC_MSG_HEADER_SIZE);
  if(nBytes <= 0)
  {
    nLOGDEBUG(LOG_ERR, "Message Sync Read Failure\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");

    nFREE(IPC_MESSAGE, msgBuff);
    return (IpcMsgHeaderT *) IPC_ERR_MSG_SYNC_READ;
  }

  nLOGDEBUG(LOG_INFO, "Message Received Success\n");
  nLOGDEBUG(LOG_DEBUG, "################################\n");

  return (IpcMsgHeaderT *) msgBuff;
}

/*******************************************************************************
 * Function: <ipcReadnSync>  
 *  
 * Description: <Receive Message>  
 *  
 * Parameters: scok      : Send Process Socket  
 *             buf       : Read data buffer  
 *             readBytes : Read bytes  
 *  
 * Returns: read bytes   
*******************************************************************************/

static Int32T ipcReadnSync (Int32T sock, StringT buf, Int32T readBytes)
{
  Int32T nReads;
  Int32T nBytes = readBytes;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "Message Readn Sync\n");
  nLOGDEBUG(LOG_INFO, "Socket : %d, readBytes : %d\n", sock, readBytes);

  while(nBytes > 0)
  {
    nReads = read(sock, buf, nBytes);

    if(nReads < 0)
    {
      switch(errno)
      {
        case EINTR:
        case EAGAIN:
        case EINPROGRESS:
       #if(EWOULDBLOCK != EAGAIN)
        case EWOULDBLOCK:
       #endif
        usleep(1);
        continue;
      }

      nLOGDEBUG(LOG_ERR, "nReads is < 0\n");
      return (nReads);
    }
    else
    {
      if(nReads == 0)
      {
        break;
      }

      nBytes -= nReads;
      buf += nReads;
    }
  }

  nLOGDEBUG(LOG_INFO, "readBytes(%d) - nBytes(%d) = %d\n",
                       readBytes, nBytes, readBytes - nBytes);
  nLOGDEBUG(LOG_DEBUG, "################################\n");

  return readBytes - nBytes;
}

/**
 * Description: Pending 되어있는 Message를 실행하는 함수
 *
 * @retval : SUCCESS(0) : 성공,
 *           FAILURE(-1)) : 실패
 * @bug
 *  반드시, _ipcSendSync()을 먼저 실행해야 한다.
 *
 * Example Code Usage
 * @code
 *  void sendMessage(void)
 *  {
 *    Int32T ret;
 *    Int8T sndBuf[4096] = {};
 *    Int32T sndLength;
 *    Int8T revBuf[4096] = {};
 *    Int32T revLength;
 *
 *    ... // send message 설정
 *
 *    ret = _ipcSendSync
 *          (RIB_MANAGER, RIB_IPV4_ROUTE_ADD, RIB_IPV4_ROUTE_DEL,
 *           sndBuf, sndLength, rcvBuf, &rcvLength);
 *
 *    ... // recv message 처리
 *
 *    _ipcProcessingPendingMsg(); // Pending Message 처리
 *
 *    ...
 *
 *  }
 * @endcode
 */

Int32T _ipcProcessingPendingMsg (void)
{
  Int32T i; 
  Int32T dataLen;
  Int32T pendingCnt;

  nLOGDEBUG(LOG_DEBUG, "################################\n");
  nLOGDEBUG(LOG_INFO, "Message Pending List Operation\n");

  if(gIpcMsgPendingList == NULL)
  {
    nLOGDEBUG(LOG_WARNING, "gIpcMsgPendingList is NULL\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return FAILURE;
  }

  if(gIpcMsgPendingList->count == 0)
  {
    nLOGDEBUG(LOG_INFO, "gIpcMsgPendingList->count = 0\n");
    nLOGDEBUG(LOG_INFO, "Message Pending List Operation end\n");
    nLOGDEBUG(LOG_DEBUG, "################################\n");
    return SUCCESS;
  }


  // Stored in the Message processing

  pendingCnt = gIpcMsgPendingList->count;
  nLOGDEBUG(LOG_INFO, "gIpcMsgPendingList->count = %d\n", pendingCnt);

  for(i=0; i<pendingCnt; i++)
  {
    IpcMsgHeaderT * pMsg;
    pMsg = queueDequeue(gIpcMsgPendingList);

    dataLen = pMsg->length - IPC_MSG_HEADER_SIZE;
    nLOGDEBUG(LOG_INFO, "%d, DataLen : %d\n", i, dataLen); 
    gIpcCb(pMsg->msgId, pMsg->data, dataLen);

    nFREE(IPC_MESSAGE, pMsg);
  }

  nLOGDEBUG(LOG_INFO, "Message Pending List Operation end\n");
  nLOGDEBUG(LOG_DEBUG, "################################\n");


  return SUCCESS;
}
 
