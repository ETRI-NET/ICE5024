#include "nnRibDefines.h"
#include "nnTypes.h"
#include "nnMemmgr.h"
#include "taskManager.h"
#include "nnLog.h"
#include "nnBuffer.h"

///////////////////////////////////////////////////////////////////////////////
// Global Definitions is here
///////////////////////////////////////////////////////////////////////////////
MemT * pMemMgr = NULL;
char * pStrTimeBegin;
char * pStrTimeEnd;

///////////////////////////////////////////////////////////////////////////////
// External Definitions are here
///////////////////////////////////////////////////////////////////////////////
extern Int32T initIpcChannel();

void
procMsgId201(void * data, Uint32T dataLen)
{
  short a;
  int b;
  nnBufferT msgBuff;
  nnBufferReset(&msgBuff);
  nnBufferAssign (&msgBuff, data, dataLen);
  a = nnBufferGetInt16T(&msgBuff);
  b = nnBufferGetInt32T(&msgBuff);
  printf("procMsgId201 e=%d f=%d msg_length=%d\n",a,b,msgBuff.length); 
}

void
procMsgId202()
{
  PrefixT p;
  nnBufferT msgBuff;
  nnBufferReset(&msgBuff);
  char *s = "1.2.3.4";

  inet_aton(s,&p.u.prefix4);
  p.prefixLen = 24;

  nnBufferSetInaddr(&msgBuff,p.u.prefix4);
  nnBufferSetInt8T(&msgBuff,p.prefixLen);
  printf("procMsgId202, msg_length= %d\n", msgBuff.length); 
  ipcSendAsync(RIP, 102, msgBuff.data, msgBuff.length);
}

void
procMsgId203()
{
  char command = 2;
  char version = 2;
  short allzero1 = 0;
  short address_family = 2;
  short route_tag = 1;
  struct in_addr network_address;
  int subnet_mask = 24;
  struct in_addr nexthop_address;
  int metric=1;

  char *s = "1.2.3.4";
  char *r = "11.22.33.44";

  inet_aton(s,&network_address);
  inet_aton(r,&nexthop_address);

  // buffering
  nnBufferT msgBuff;
  nnBufferReset(&msgBuff);

  nnBufferSetInt8T(&msgBuff, command);
  nnBufferSetInt8T(&msgBuff, version);
  nnBufferSetInt16T(&msgBuff, allzero1);
  nnBufferSetInt16T(&msgBuff, address_family);
  nnBufferSetInt16T(&msgBuff, route_tag);
  nnBufferSetInaddr(&msgBuff, network_address);
  nnBufferSetInt32T(&msgBuff, subnet_mask);
  nnBufferSetInaddr(&msgBuff, nexthop_address);
  nnBufferSetInt32T(&msgBuff, metric);
  
//  ipcSendAsync(RIP, 104, msgBuff.data, msgBuff.length);
  printf("$ RIPv2 Packet sent (msg_length= %d)\n", msgBuff.length);
}

void
procRIPv1Msg(void * data, Uint32T dataLen)
{
  char command ;
  char version ;
  short allzero1 ;
  short address_family ;
  short allzero2 ;
  struct in_addr network_address ;
  int allzero3 ;
  int allzero4 ;
  int metric ;
  PrefixT p ;
 
  nnBufferT msgBuff;
  nnBufferReset(&msgBuff);
  nnBufferAssign (&msgBuff, data, dataLen);
 
  // ����Ÿ ����
  command         = nnBufferGetInt8T(&msgBuff);
  version         = nnBufferGetInt8T(&msgBuff);
  allzero1        = nnBufferGetInt16T(&msgBuff);
  address_family  = nnBufferGetInt16T(&msgBuff);
  allzero2        = nnBufferGetInt16T(&msgBuff);
  network_address = nnBufferGetInaddr(&msgBuff);
  allzero3        = nnBufferGetInt32T(&msgBuff);
  allzero4        = nnBufferGetInt32T(&msgBuff);
  metric          = nnBufferGetInt32T(&msgBuff);

  printf("----------------------------------------\n");
  printf("@ RIPv1 Packet received (msg_length= %d)\n", msgBuff.length);
  printf("----------------------------------------\n");
  printf("@ command= %d\n",command);
  printf("@ version= %d\n",version);
  printf("@ allzero1= %d\n",allzero1);
  printf("@ address_family= %d\n",address_family);
  printf("@ allzero2= %d\n",allzero2);
  printf("@ network_address= %s\n",inet_ntoa(network_address));
  printf("@ allzero3= %d\n",allzero3);
  printf("@ allzero4= %d\n",allzero4);
  printf("@ metric= %d\n",metric);
}

///////////////////////////////////////////////////////////////////////////////
// Initialize step is completed, This Callback is called.
///////////////////////////////////////////////////////////////////////////////
void
initCompletCallback()
{
#if 0
  nnBufferT msgBuff;

  short a=100;
  int b=200;
  nnBufferReset(&msgBuff);
  nnBufferSetInt16T(&msgBuff, a);
  nnBufferSetInt32T(&msgBuff, b);
  ipcSendAsync(RIP, 101, msgBuff.data, msgBuff.length);
#endif
//  procMsgId202();
//  procMsgId203();
}

////////////////////////////////////////////////////////////////////////////
// Signal comes up, This Callback is called.
////////////////////////////////////////////////////////////////////////////
void
signalCallback(Int32T signalType)
{
  taskClose();
  nnLogClose();
  memClose();
}

/////////////////////////////////////////////////////////////////////////////
// Callback Function When Timer occured
/////////////////////////////////////////////////////////////////////////////
void
timerCallback (Int32T fd, Int16T event, void * arg)
{
  // procMsgId203();
}
 
void
eventReadCallback (Int32T msgId, void * data, Uint32T dataLen)
{
  NNLOG(LOG_DEBUG, "gtpark eventReadCallback");
}

void
ipcReadCallback (Int32T msgId, void * data, Uint32T dataLen)
{
  // printf("## %s called..\n", __func__);
  NNLOG(LOG_DEBUG, 0, "receive data : msgId = %d len=%d\n", msgId, dataLen);
  // message id check
  switch(msgId)
  {
      case 201 :
          procMsgId201(data, dataLen);
          break;
      case 202 :
          procMsgId202(data, dataLen);
          break;
      case 203 :
          procMsgId203(data, dataLen);
          break;
      case 204 :
          procRIPv1Msg(data, dataLen);
          break;
      default :
          break;
  }
  return;
}

/* Main routine of ripd. */
int
main (int argc, char **argv)
{
  /////////////////////////////////////////////////////////////////////////////
  //   Initialize Memmory Manager
  //   RIB_MANAGER defined in nnTypes.h file.
  /////////////////////////////////////////////////////////////////////////////
  if(memInit(OSPF) == FAILURE)
  {
    printf("Could not Initialize Memory Manager !!\n");
    return -1; 
  }

  /////////////////////////////////////////////////////////////////////////////
  // Initialize Log
  /////////////////////////////////////////////////////////////////////////////
  if(nnLogInit(OSPF) != SUCCESS)
  {
    printf("Could not Initialize Log Service !!\n");
    return -1; 
  }

  /////////////////////////////////////////////////////////////////////////////
  // Max Log FileName size : 20
  /////////////////////////////////////////////////////////////////////////////
  nnLogSetFile("logRIP", 10);
  nnLogSetFlag(LOG_FILE);

  /////////////////////////////////////////////////////////////////////////////
  // Signal Registration
  /////////////////////////////////////////////////////////////////////////////
  taskCreate(initCompletCallback);

  ////////////////////////////////////////////////////////////////////////////
  // IPC Callback Function Registration
  ////////////////////////////////////////////////////////////////////////////
  ipcChannelOpen(OSPF, (void *)ipcReadCallback);
 
  /////////////////////////////////////////////////////////////////////////////
  // Timer Registration
  /////////////////////////////////////////////////////////////////////////////
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 5000;
  taskTimerSet(timerCallback, tv, TASK_PERSIST);

  /////////////////////////////////////////////////////////////////////////////
  // Signal Registration
  /////////////////////////////////////////////////////////////////////////////
  taskSignalSetAggr(signalCallback);
  
  /////////////////////////////////////////////////////////////////////////////
  // Task Dispatch
  /////////////////////////////////////////////////////////////////////////////
  taskDispatch();

  return (0);
}
