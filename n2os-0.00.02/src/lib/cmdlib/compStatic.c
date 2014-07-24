#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>


#include "nnLog.h"
#include "nnMemmgr.h"
#include "taskManager.h"
#include "nnBuffer.h"
#include "nnCmdCommon.h"
#include "nnCmdDefines.h"
#include "nnCmdMsg.h"

struct nnUnixCompServ_s
{
  Int32T compId;
  Int8T   compUnixPath[UNIX_PATH_MAX];
} gzUnixCompServ[IPC_MAX_MGR]  __attribute__ ((unused)) =
{
  {IPC_CMSH_MGR,  "/var/run/cmsh.sock"},
  {IPC_IPC_MGR,   "/var/run/ipcmgr.sock"},
  {IPC_PRO_MGR,   "/var/run/procmgr.sock"},
  {IPC_PIF_MGR,   "/var/run/pifmgr.sock"},
  {IPC_RIB_MGR,   "/var/run/ribmgr.sock"},
  {IPC_POL_MGR,   "/var/run/polmgr.sock"},
  {IPC_CM_MGR,    "/var/run/cm.sock"},
  {IPC_LACP,      "/var/run/lacp.sock"},
  {IPC_MSTP,      "/var/run/mstp.sock"},
  {IPC_RIP,       "/var/run/rip.sock"},
  {IPC_ISIS,      "/var/run/isis.sock"},
  {IPC_OSPF,      "/var/run/ospf.sock"},
  {IPC_DLU_TESTER, "/var/run/ribtester.sock"},
  {IPC_DLU_TESTER, "/var/run/dlutester.sock"}
};

typedef void   (*CMIPC)   (Int32T sockId, void *message, Uint32T size);

void *gNosCmshProcess = NULL; /** Process from cmsh */
void *gNosCompProcess = NULL; /** Process from comp */

#define MAX_TASK_NUMBER 10240

struct cmTaskTable_T
{
  void    *pData;
} sCmTaskTable[MAX_TASK_NUMBER] __attribute__ ((unused));

void
nosCmdAddTask(void *taskCb, Int32T taskFd)
{
  sCmTaskTable[taskFd].pData = _taskFdSet(taskCb, taskFd, TASK_READ|TASK_PERSIST, TASK_PRI_MIDDLE, NULL);
}
void
nosCmdDelTask(Int32T taskFd)
{
  close(taskFd);
  _taskFdDel(sCmTaskTable[taskFd].pData);
}
void
nosMainServiceFree(void)
{
        struct dirent *dirP;
        DIR *dirPtr;
        FILE *cmdLine;
        Int8T dirStr[1024], dirStat[1024], cmdStr[1024];
        sprintf(dirStr, "/proc/");
        if((dirPtr = opendir(dirStr)))
        {
                while((dirP = readdir(dirPtr)) != NULL)
                {
                        if(!isdigit(dirP->d_name[0]))
                                continue;
                        sprintf(dirStat, "/proc/%s/cmdline", dirP->d_name);
                        if((cmdLine = fopen(dirStat, "rb")) == NULL)
                                continue;
                        Int8T *argv = 0;
                        size_t size = 0;
                        while(getdelim(&argv, &size, 0, cmdLine) != -1)
                        {
                                if(!strcmp(argv, NOS_INETD_CONFIG_FILE_PATH))
                                {
                                        /**     Kill Here       */
                                        sprintf(cmdStr, "kill -9 %s", dirP->d_name);
                                        system(cmdStr);
                                }
                        }
                        free(argv);
                        fclose(cmdLine);
                }
        }
}

Int32T
nosCmdNonBlocking(Int32T sock)
{
  Int32T on = 1;
  Int32T flags;
  setsockopt(sock, SOL_SOCKET,  SO_REUSEADDR,
                   (char *)&on, sizeof(on));
  if((flags = fcntl(sock, F_GETFL, 0)) < 0)
  {
    return 0;
  }
  return fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

Int32T
cmdSockBind(Int32T gIndex)
{
  Int32T i, sock;
  struct sockaddr_un addr;
  sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if(sock < 0)
  {
    fprintf(stderr,"Create socket error [%s]\n", strerror (errno));
    return sock;
  }
  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  for(i = 0; i < IPC_MAX_MGR; i++)
  {
    if(gzUnixCompServ[i].compId == gIndex)
    {
      unlink(gzUnixCompServ[i].compUnixPath);
      strncpy(addr.sun_path, gzUnixCompServ[i].compUnixPath, strlen(gzUnixCompServ[i].compUnixPath));
      break;
    }
  }
  nosCmdNonBlocking(sock);
  if(bind(sock, (struct sockaddr *)&addr,  sizeof (addr.sun_family) + strlen(addr.sun_path)) < 0)
  {
    fprintf(stderr,"Bind Socket error [%s]\n", strerror (errno));
    return -1;
  }
  if(listen(sock, SOMAXCONN) < 0)
  {
    fprintf(stderr,"Bind Socket error [%s]\n", strerror (errno));
    return -1;
  }
  return sock;
}

Int32T
cmdSockConn(Int32T gIndex)
{
  Int32T i, sock;
  struct sockaddr_un addr;
  sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if(sock < 0)
  {
    fprintf(stderr,"Create socket error [%s]\n", strerror (errno));
    return sock;
  }
  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  for(i = 0; i < IPC_MAX_MGR; i++)
  {
    if(gzUnixCompServ[i].compId == gIndex)
    {
      strncpy(addr.sun_path, gzUnixCompServ[i].compUnixPath, strlen(gzUnixCompServ[i].compUnixPath));
      break;
    }
  }
  if(connect(sock, (struct sockaddr *)&addr,  sizeof (addr.sun_family) + strlen(addr.sun_path)) < 0)
  {
    fprintf(stderr,"Connect Socket error [%s]\n", strerror (errno));
    return -1;
  }
  nosCmdNonBlocking(sock);
  return sock;
}
Int32T
cmdSockClose(Int32T sock)
{
  return close(sock);
}


void
nosCmshReadFuncCb(Int32T sock, Int16T event, void *argv)
{
  char msgBuff[SO_RCV_BUF_SIZE_MAX];
  ssize_t nBytes;
  while (1)
  {
    bzero(&msgBuff, sizeof(msgBuff));
    if ((nBytes = recv(sock, msgBuff, sizeof(msgBuff), 0)) <= 0) {
      break;
    }
    if(gNosCmshProcess != NULL)
    {
      CMIPC tmpFunc = (CMIPC)(gNosCmshProcess);
      tmpFunc(sock, msgBuff, nBytes);
    }
  }
  if(nBytes == 0) {
    nosCmdDelTask(sock);
  } else if(nBytes < 0) {
    if (errno == EAGAIN)
      return;
    nosCmdDelTask(sock);
  }
}

void
nosCompReadFuncCb(Int32T sock, Int16T event, void *argv)
{
  char msgBuff[SO_RCV_BUF_SIZE_MAX];
  ssize_t nBytes;
  while (1)
  {
    bzero(&msgBuff, sizeof(msgBuff));
    if ((nBytes = recv(sock, msgBuff, sizeof(msgBuff), 0)) <= 0) {
      break;
    }
    if(gNosCompProcess != NULL)
    {
      CMIPC tmpFunc = (CMIPC)(gNosCompProcess);
      tmpFunc(sock, msgBuff, nBytes);
    }
  }
  if(nBytes == 0) {
    nosCmdDelTask(sock);
  } else if(nBytes < 0) {
    if (errno == EAGAIN)
      return;
    nosCmdDelTask(sock);
  }
}

void
nosCmshAcceptFuncCb(Int32T sock, Int16T event, void *argv)
{
  struct sockaddr_storage ss;
  socklen_t slen = sizeof(ss);
  Int32T fd;
  if((fd = accept(sock, (struct sockaddr*)&ss, &slen)) < 0)
  {
    perror("Accept");
  } else if (fd > FD_SETSIZE) {
    close(fd);
  }
  else
  {
    nosCmdNonBlocking(fd);
    nosCmdAddTask(nosCmshReadFuncCb, fd);
  }
}

void
nosCompAcceptFuncCb(Int32T sock, Int16T event, void *argv)
{
  struct sockaddr_storage ss;
  socklen_t slen = sizeof(ss);
  Int32T fd;
  if((fd = accept(sock, (struct sockaddr*)&ss, &slen)) < 0)
  {
    perror("Accept");
  } else if (fd > FD_SETSIZE) {
    close(fd);
  }
  else
  {
    nosCmdNonBlocking(fd);
    nosCmdAddTask(nosCompReadFuncCb, fd);
  }
}

void
comCmdRegister(Int32T sockId, Int32T compId, Int32T cmId)
{
  Int8T szBuffer[CMD_IPC_MAXLEN];
  cmdIpcMsg_T *node = (cmdIpcMsg_T *)(szBuffer);
  node->length = 0;
  /** Register Request  */
  node->code = CMD_IPC_CODE_CLIENT_REG_REQ;
  node->srcID = compId;
  node->dstID = cmId;
  if(send(sockId, szBuffer, CMD_IPC_HDR_LEN, 0) < 0)
  {
    perror("comCmdRegister");
  }
  /** Startup Request   */
  node->code = CMD_IPC_CODE_STARTUP_CONFIG;
  if(send(sockId, szBuffer, CMD_IPC_HDR_LEN, 0) < 0)
  {
    perror("comCmdRegister");
  }
}

void
nosCmCmdInit(Int32T cmshID, Int32T cmID, void *nosCmshProcess, void *nosCompProcess)
{
  gNosCmshProcess = nosCmshProcess;
  gNosCompProcess = nosCompProcess;
  //register COMPs & CMSHs IPC Process
  Int32T sCmsh = cmdSockBind(cmshID);
  Int32T sComp = cmdSockBind(cmID);
  /** Init Task List  */
  nosCmdAddTask(nosCmshAcceptFuncCb, sCmsh);
  nosCmdAddTask(nosCompAcceptFuncCb, sComp);
}

void
nosCompCmdInit(Int32T compID, Int32T cmID, void *nosCmshProcess, void *nosCompProcess)
{
  Int32T sockCm;
  gNosCmshProcess = nosCmshProcess;
  gNosCompProcess = nosCompProcess;
	//register CMSHs IPC Process (Show Func)
	Int32T sCmsh = cmdSockBind(compID);
	nosCmdAddTask(nosCmshAcceptFuncCb, sCmsh);
	//connect to cm manager
  do
  {
	  if((sockCm = cmdSockConn(cmID)) > 0)  /** Connect to CM Manager */
    {
      break;
    }
	 sleep(1); // this is the code that the library developer had added
  }while(1);
	
	nosCmdAddTask(nosCompReadFuncCb, sockCm);
  comCmdRegister(sockCm, compID, cmID);  /** Register Components  */
}
