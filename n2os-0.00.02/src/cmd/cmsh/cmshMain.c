/*3456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*********************************.*********************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute.
*           All rights reserved. No part of this software shall be reproduced,
*           stored in a retrieval system, or transmitted by any means,
*           electronic, mechanical, photocopying, recording, or otherwise,
*           without written permission from ETRI.
*******************************************************************************/
 
/**
 * @brief : This file defines main function for cmshicy manager.
 *  - Block Name : Policy Manager
 *  - Process Name : cmshmgr
 *  - Creator : PyungKoo Park
 *  - Initial Date : 2014/03/03
 */
 
/**
 * @file        :
 *
 * $Author: $
 * $Date: $
 * $Revision: $
 * $LastChangedBy: $
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <stdlib.h>
#include <time.h> 
#include "editline/readline.h"
#include "histedit.h"


#include "taskManager.h"
#include "nnTypes.h"
#include "nnCmdCommon.h"
#include "nnCmdLink.h"
#include "nnCmdCmsh.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"
#include "nnCmdDefines.h"

extern Int8T *cmshPromptStr(void);
extern Int32T cmshMainInit(Int32T sock);
extern Int32T cmshParseCmd(struct cmsh *cmsh);
extern void cmshMainFree(void);
extern void cmshMainExit(void);
extern Int32T cmdMsgLogin(Int32T sockfd, Int32T counter);
extern Int32T cmdSockConn(Int32T gIndex);
extern Int32T cmdSockClose(Int32T sock);
extern Uint32T nodeBackup;
fd_set gMasterFds;  /** For Hook  */
Int32T gFdMaxNumber;
Int32T gSockCmManager;
Int32T gCmshBlockHook;  /** Status Comand for hook  */

time_t gRealTime;	/**	Current timeout check	*/

Int32T isWaitingCommandLine = 0;

struct cmdList *cmshOutTable = NULL;

extern struct cmsh *gCmshPtr;

static void
usage (Int32T status, Int8T *progname)
{
    if (status != 0)
    fprintf (stderr, "Try `%s --help' for more information.\n", progname);
    else
    {
      printf ("Usage : %s [OPTION...]\n\n\
-p               Request Password Login\n\
-h               Hostname Remote Login \n\
-f               Default Configure File\n\
\n", progname);
    }
    exit (status);
}

void
cmshCheckOption(Int32T argc, Int8T **argv, Int32T *sFlags, Int8T **rValue)
{
  Int8T szBuffer[1024];
  Int32T c;
  /** Set umask before anything for security.  */
  umask (0027);
  *sFlags = 0;
  /** Preserve name of myself.  */
  while ((c = getopt (argc, argv, "ph:f:")) != -1)
  {
    switch(c)
    {
      case 'h':
        sprintf(szBuffer, "%s", optarg);
        *rValue = szBuffer;
      case 'p':
        *sFlags = 1;
      break;
      case 'f':
      break;
      default:
        usage (0, argv[0]);
      break;
    }
  }
}
sigjmp_buf jmpbuf;
Int32T jmpflag = 0;
void
cmshResetTerminal(Int32T mode)
{
	if(mode == 0)
    rl_reset_terminal(NULL);
	else
	{
		rl_set_prompt(cmshPromptStr());
		rl_forced_update_display();
	}
}
void *
cmshSetSignal(Int32T signo, void (*func) (Int32T))
{
    struct sigaction sig;
    struct sigaction osig;
    sig.sa_handler = func;
    sigemptyset (&sig.sa_mask);
    sig.sa_flags = 0;
#ifdef SA_RESTART
    sig.sa_flags |= SA_RESTART;
#endif /** SA_RESTART */
    if (sigaction (signo, &sig, &osig) < 0) {
        return (SIG_ERR);
    } else {
        return (osig.sa_handler);
    }
}

void
cmshSignalInitCb(Int32T signo)
{
  if(gCmshBlockHook)
  {
    gCmshBlockHook = 0;   /** Unblocking Hook */
    if(gCmshPtr->inputMode)
    {
      gCmshPtr->currentDepth = nodeBackup;
    }
    isWaitingCommandLine = 0;
    cmshResetTerminal(1);
  }

	cmshSetSignal(SIGINT ,  cmshSignalInitCb);
  cmshResetTerminal(0);
	if (jmpflag)
	{
		jmpflag = 0;
		siglongjmp (jmpbuf, 1);
	}
}
void
cmshSignalStipCb(Int32T signo)
{
	cmshSetSignal(SIGTSTP,  cmshSignalStipCb);
  cmshResetTerminal(0);
	if (jmpflag)
	{
		jmpflag = 0;
		siglongjmp (jmpbuf, 1);
	}
}
void
cmshExit(Int32T gIndex)
{
  cmshMainExit();
}
void
cmshSignalTermCb(Int32T signo)
{
	cmshExit(0);
}
void
cmshSignalInit(void)
{
	cmshSetSignal(SIGINT ,	cmshSignalInitCb);
	cmshSetSignal(SIGTSTP,	cmshSignalStipCb);
	cmshSetSignal(SIGALRM,    SIG_IGN);
	cmshSetSignal(SIGPIPE,    SIG_IGN);
	cmshSetSignal(SIGQUIT,    SIG_IGN);
	cmshSetSignal(SIGTTIN,    SIG_IGN);
	cmshSetSignal(SIGTTOU,    SIG_IGN);
	cmshSetSignal(SIGTERM,	cmshSignalTermCb);
}

Int8T *inLine = NULL;
extern EditLine *e;
Int8T *
cmshGetStr(Int8T *prompt)
{
    if(inLine)
    {
        free(inLine);
        inLine = NULL;
    }
    inLine = readline(prompt);
    if(inLine && *inLine)
    {
        add_history(inLine);
    }
    if(inLine == NULL)
    {
        printf("\n");
        inLine = strdup("exit\n");
    }
    return inLine;
}

Int32T
cmshParseCommand(Int8T *msg, Int32T mode)
{
    if(msg != NULL)
    {
      gCmshPtr->inputStr = strdup(msg);
    }
    else
    {
      gCmshPtr->inputStr = strdup(inLine);
    }
    gCmshPtr->inputMode = mode;
    gCmshPtr->cmshFuncIndex = 0;
    return cmshParseCmd(gCmshPtr);
}

Int32T
cmshHelpFunction(Int32T a, Int32T b)
{
    const LineInfo *li;
    li = el_line(e);
    /** User input.  */
    rl_line_buffer = (Int8T *) li->buffer;
    rl_end = li->lastchar - li->buffer;
    rl_line_buffer[rl_end] = '\0';
    gCmshPtr->inputStr = (Int8T *)rl_line_buffer;
    cmdParseHelp(gCmshPtr);
    return 0;
}

Int32T complete_status;

Int8T *
cmshAutoFunction_matches (const Int8T *text, Int32T state)
{
    static Int8T **matched = NULL;
    static Int32T gindex = 0;
    const LineInfo *li;
    UNUSED(text);
    li = el_line(e);

    rl_line_buffer = (Int8T *) li->buffer;
    rl_end = li->lastchar - li->buffer;
    rl_line_buffer[rl_end] = '\0';

    if (li->cursor != li->lastchar)
    {
        return NULL;
    }
    /** First call. */
    if (! state)
    {
        gindex = 0;
        gCmshPtr->inputStr = (Int8T *)rl_line_buffer;
        matched = cmdParseAutoCompletion(gCmshPtr);
        /** Update complete status.  */
        if (matched && matched[0] && matched[1] == NULL) {
            complete_status = 1;
        } else {
            complete_status = 0;
        }
    }
    /** This function is called until this function returns NULL.  */
    if (matched && matched[gindex])
    {
        return matched[gindex++];
    }
    return NULL;
}
Int8T **
cmshAutoFunction (Int8T *text, Int32T start, Int32T end)
{
    Int8T **matches;
    UNUSED(start);
    UNUSED(end);
    matches = completion_matches (text, cmshAutoFunction_matches);
    return matches;
}


#define PAGERPROMPT " --More-- "
#define BACKOVERPROMPT  "\b\b\b\b\b\b\b\b\b\b          \b\b\b\b\b\b\b\b\b\b"
void
cmshPrintfPage(struct cmdList *list)
{
  Int8T *cmshStrLine, c;
  Int32T i = 0;
  Uint16T row, col;
  cmdGetTerm(0, &row, &col);
  struct cmdListNode *nn;
  CMD_MANAGER_LIST_LOOP(list, cmshStrLine, nn)
  {
    if(i == (row -2))
    {
      cmdPrint(gCmshPtr, PAGERPROMPT);
      fflush(0);
      do
      {
        c = getchar();
      }while((c != 'q') && (c != 'Q') && (c != ' ') && (c != '\r') && (c != '\n'));
      cmdPrint(gCmshPtr, BACKOVERPROMPT);
      if(c == 'q' || c == 'Q')
        break;
      if(c == '\r' || c == '\n')
        i--;
      else
        i = 0;
    }
    cmdPrint(gCmshPtr, "%s\n", cmshStrLine);
    i++;
  }
}

void
cmshRecvCallback(Int32T sock, void *message, Int32T size)
{
  Int8T *szBuffer = (Int8T *)message;
  Int8T  szTmpBuffer[CMD_IPC_MAXLEN];
  Int8T *pch = NULL;
  Uint32T nMsgSize, nTotalBytes = 0;
  cmdIpcMsg_T cmIpcHdr;
  while(nTotalBytes < size)
  {
    cmdIpcUnpackHdr(szBuffer, &cmIpcHdr);
    nMsgSize = cmIpcHdr.length + CMD_IPC_HDR_LEN;
    bzero(&szTmpBuffer, CMD_IPC_MAXLEN);
    memcpy(&szTmpBuffer, szBuffer, nMsgSize);
    switch(cmIpcHdr.code)
    {
      case CMD_IPC_CODE_CLIENT_DEREG_RES_OK:
      {
        cmshMainFree();
        cmshResetTerminal(0);
        exit(0);
      }
      break;
      case CMD_IPC_CODE_CLIENT_REG_RES_OK:
      {
        Uint8T tlvPrivilege;
        Int32T tlvPrivilegeLen;
        cmdIpcUnpackTlv(szBuffer, CMD_IPC_TLV_PRIVILEGE, &tlvPrivilegeLen, &tlvPrivilege);
        gCmshPtr->userPrivilege = tlvPrivilege;
      }
      break;
      case CMD_IPC_OUTPUT_DATA:
      {
        Int8T szBufferTlv[CMD_IPC_MAXLEN];
        Int32T tlvLengthType, tlvLengthData, outTypeTlv;
        bzero(&szBufferTlv, CMD_IPC_MAXLEN);
        cmdIpcUnpackTlv(szBuffer, CMD_IPC_TLV_OUTPUT_TYPE, &tlvLengthType, &outTypeTlv);
        cmdIpcUnpackTlv(szBuffer, CMD_IPC_TLV_OUTPUT_DATA, &tlvLengthData, szBufferTlv);
        switch(outTypeTlv) {
          case CMD_OUTMOD_BEGIN:
            if(cmshOutTable != NULL)
            {
              cmdListDel(cmshOutTable);
              cmshOutTable = NULL;
            }
            cmshOutTable = cmdListNew();
            pch = strtok (szBufferTlv,"\n");
            while (pch != NULL)
            {
              cmdListAddNode(cmshOutTable, strdup(pch));
              pch = strtok (NULL, "\n");
            }

          break;
          case CMD_OUTMOD_INCLUDE:
            if(cmshOutTable == NULL)
            {
              cmshOutTable = cmdListNew();
            }
            pch = strtok (szBufferTlv,"\n");
            while (pch != NULL)
            {
              cmdListAddNode(cmshOutTable, strdup(pch));
              pch = strtok (NULL, "\n");
            }
          break;
          case CMD_OUTMOD_EXCLUDE:
            if(cmshOutTable == NULL)
            {
              cmshOutTable = cmdListNew();
            }
            pch = strtok (szBufferTlv,"\n");
            while (pch != NULL)
            {
              cmdListAddNode(cmshOutTable, strdup(pch));
              pch = strtok (NULL, "\n");
            }
            if(sock != gSockCmManager) 
            {
              cmdSockClose(sock);
              FD_CLR(sock, &gMasterFds);
              gFdMaxNumber = gSockCmManager;
            }

            cmshPrintfPage(cmshOutTable);
            if(cmshOutTable != NULL)
            {
              cmdListDel(cmshOutTable);
              cmshOutTable = NULL;
            }
						gCmshBlockHook--;   /** Unblocking Hook */
            if(gCmshPtr->inputMode)
            {
              gCmshPtr->currentDepth = nodeBackup;//CMD_NODE_CONFIG;
            }
            isWaitingCommandLine = 0;
						cmshResetTerminal(1);
          break;
          default:
            cmdPrint(gCmshPtr, "\nUnknown OutPutType[%d]\n", outTypeTlv);
          break;
        }
        gRealTime = time(NULL);  /** Reset Timer */
      }
      break;
      case CMD_IPC_OAM_DATA:
      {
        Int8T szBufferTlv[CMD_IPC_MAXLEN];
        Int32T tlvLengthData;
        bzero(&szBufferTlv, CMD_IPC_MAXLEN);
        cmdIpcUnpackTlv(szBuffer, CMD_IPC_TLV_OAM_DATA, &tlvLengthData, szBufferTlv);
        cmdPrint(gCmshPtr, "\n%s\n", szBufferTlv);
        cmshResetTerminal(1);
      }
      break;
      case CMD_IPC_CODE_FUNC_REQ_PASSWD:
      /** Request Password From CM or Components  */
      {
        Int8T msgBuf[CMD_IPC_MAXLEN];
        Int8T argv[CMSH_ARGC_MAX][CMSH_ARGV_MAX_LEN];
        Int32T i, sSize, funcIndex, funcId, argc = 0;
        Int8T *passBuf = getpass ("Password: ");
        cmdIpcUnpackTlv(szBuffer, CMD_IPC_TLV_CALLBACK_INDEX, &sSize, &funcIndex);
        cmdIpcUnpackTlv(szBuffer, CMD_IPC_TLV_CALLBACK_FUNC, &sSize, &funcId);
        memset(argv, 0, CMSH_ARGC_MAX*CMSH_ARGV_MAX_LEN);
        while(cmdIpcUnpackTlv(szBuffer, CMD_IPC_TLV_CALLBACK_ARGSTR, &sSize, argv[argc]) == CMD_IPC_OK)
        {
            argc++;
        }
        cmIpcHdr.code = CMD_IPC_CODE_FUNC_RES_PASSWD;
        cmIpcHdr.requestKey = sock;
        cmIpcHdr.dstID = cmIpcHdr.srcID;
        cmIpcHdr.srcID = IPC_CMSH_MGR;
        cmdIpcAddHdr(msgBuf, &cmIpcHdr);
        funcIndex++;/** Increase  */
        cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_INDEX, sizeof (Int32T), &funcIndex);
        cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_FUNC, sizeof (Int32T), &funcId);
        for(i = 0; i < argc; i++)
        {
          cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_ARGSTR, strlen(argv[i]), argv[i]);
        }
        cmdIpcAddTlv(msgBuf, CMD_IPC_TLV_CALLBACK_ARGSTR, strlen(passBuf), passBuf);
        cmdIpcSend(gCmshPtr->cmfd, msgBuf);
        gRealTime = time(NULL);  /** Reset Timer */
      }
      break;
      case CMD_IPC_CODE_FUNC_RES_WAIT:
      {
        isWaitingCommandLine = 1;
        gCmshPtr->cmshFuncIndex++;
        gRealTime = time(NULL);
      }
      break;
      default:
        cmdPrint(gCmshPtr, "\nUnknown code [%d]\n", cmIpcHdr.code);
      break;
    }
    nTotalBytes += nMsgSize;
    szBuffer += nMsgSize;
  }
}
Int32T
cmshDiffTime(void)
{
	return time (NULL) - gRealTime;
}
void
cmshEventHookCb(void)
{
  Int8T buf[SO_RCV_BUF_SIZE_MAX];
  Int32T i, nbytes, rv  = 0;
  struct timeval tv;
  fd_set read_fds = gMasterFds;
  tv.tv_sec = 0;
  tv.tv_usec = 5000;
  if((rv = select(gFdMaxNumber+1, &read_fds, NULL, NULL, &tv)) > 0)
  {
    for(i = 0; i <= gFdMaxNumber; i++)
    {
      if(FD_ISSET(i, &read_fds))
      {
        /** Recv Message  */
        nbytes = cmdIpcRecv(i, buf);
        if(nbytes == 0)
        {
          cmdPrint(gCmshPtr,"\nSocket close...\n");
          cmdSockClose(i);
          FD_CLR(i, &gMasterFds);
          if(i == gSockCmManager) 
          {
            cmshMainFree();
            cmshResetTerminal(0);
            exit(0);
          }
        }
        cmshRecvCallback(i, buf, nbytes);
      }
    }
  }
  if(gCmshBlockHook) 
  {
    if(cmshDiffTime() >= 5)	/**	Timeout 5s	*/
    {
      gCmshBlockHook = 0;		/**	Unblocking Hook	*/
      if(gCmshPtr->inputMode)
      {
        gCmshPtr->currentDepth = nodeBackup;//CMD_NODE_CONFIG;
      }
      isWaitingCommandLine = 0;
      cmdPrint(gCmshPtr, "Timeout...\n");
      cmshResetTerminal(1);
    }
    else
    {
      if((isWaitingCommandLine) && cmshDiffTime() >= 1)
      {
        gCmshBlockHook = cmshParseCmd(gCmshPtr);
        gRealTime = time(NULL);
      }
    }
  }
}

void
cmshLogin(Int32T sockfd)
{
  if(sockfd  > 0)
  {
    if(cmdMsgLogin(sockfd, 5) == CMD_IPC_OK)
    {
      printf("Success Authen...\n");
    }
    else
    {
      printf("Not Success....\n");
      exit(0);
    }
  }
}

void
cmshReadLineInit(Int32T sock)
{
  FD_ZERO(&gMasterFds);
  gFdMaxNumber = 0;
  FD_SET(sock, &gMasterFds);
  gFdMaxNumber = MAX(gFdMaxNumber, sock);
  rl_help_function = cmshHelpFunction;
  rl_attempted_completion_function = (CPPFunction *)cmshAutoFunction;
  rl_event_hook = cmshEventHookCb;
}


Int32T main(Int32T argc, char **argv)
{
  Int8T *sLogin;
  Int32T sFlags;
  /** Init Socket Here  */
  gSockCmManager = cmdSockConn(IPC_CMSH_MGR);
  if(gSockCmManager <= 0)
  {
    return 0;
  }
  cmshSignalInit(); /** Init Signal */
  cmshCheckOption(argc, argv, &sFlags, &sLogin);
  if((sFlags != 0) && (sLogin != NULL))
  {
    printf("Login to CM manager...\n");
    /** Login */
    cmshLogin(gSockCmManager);
  }
  cmshReadLineInit(gSockCmManager);
  /** Init Tree */
	if(cmshMainInit(gSockCmManager) != CMD_IPC_OK)
	{
		cmdPrint(gCmshPtr,"Can't Init cmdTree ...\n");
		return 0;
	}
  if((gCmshBlockHook = cmshParseCommand("show banner", 0)))
  {
    gRealTime = time(NULL);  /** Get Current time start command line */
  }
  /** CTRL + C  */
  sigsetjmp (jmpbuf, 1);
  jmpflag = 1;
  cmdPrint(gCmshPtr, "\n");
  while(cmshGetStr(cmshPromptStr()))  
  {
    if(gCmshBlockHook == 0)
    {
      isWaitingCommandLine = 0;
      if((gCmshBlockHook = cmshParseCommand(NULL, 0)))
      {
        gRealTime = time(NULL);	/**	Get Current time start command line	*/
      }
    }
  }
  return(0);
}

