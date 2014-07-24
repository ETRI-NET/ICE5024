#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "../taskManager.h"


void * ret;

static void ipc_register_callback(int msgId, void * data, Uint32T dataLen)
{
    printf("msgId : %d, dataLen : %d\n", msgId, dataLen);

}

static void event_register_callback(int eventId, void * data, Uint32T dataLen)
{
    printf("eventId : %d, dataLen : %d\n", eventId, dataLen);
}


void timeCallback(Int32T fd, Int16T event, void * arg)
{
    static Int32T i;
    Int8T time[50];

    nnTimeGetCurrent(time);
    printf("Timer1 : %s  i = %d, fd = %d, event = %d\n", time, i++, fd, event);

//    eventPublish(1);
   
    if(i == 4)
    {
      //  printf("arg : %p \n", arg);
        ipcSendAsync(IPC_MANAGER, 999, NULL, 0);
//	eventUnsubscribe(1);
    }

    else if(i == 7)
    {
        StringT data = malloc(4084);
        StringT data2 = malloc(3000);
        StringT data3 = malloc(4000);
    	eventPublish(1, NULL, 0);
        ipcSendAsync(3, 1111, data, 4084);
        ipcSendAsync(3, 1112, data2, 3000);
        eventPublish(1, NULL, 0);
        ipcSendAsync(3, 1113, data3, 4000);
	
//.         ipcSendSync(5, 3333, 3334, NULL, 0);
    }
    else if(i == 8)
    {
	StringT data = malloc(4084);
        StringT data2 = malloc(3000);
        StringT data3 = malloc(4000);
	StringT data4 = malloc(4500);
        StringT data5 = malloc(4500);
        eventPublish(1, NULL, 0);
        ipcSendAsync(3, 1111, data, 4084);
        ipcSendAsync(3, 1112, data2, 3000);
        ipcSendAsync(3, 1113, data3, 4000);
        ipcSendAsync(3, 1113, data4, 4500);
	ipcSendAsync(3, 1113, data5, 4500);

//        ipcSendSync(5, 3333, 3334, NULL, 0);
    }

    else if(i == 10)
    {
         eventUnsubscribe(1);
         eventPublish(1, NULL, 0);
    }
    else if(i == 15)
    {
        taskTimerDel(arg);
    }


}

void signalIntCallback(Int32T fd, Int16T event, void * arg)
{
    struct timeval delay = {1, 0};    

  //  printf("fd = %d, event = %d\n", fd, event);

   // printf("arg : %p\n", arg);
 
//    taskSignalDel(arg);
    taskClose();
}

void timeCallback2(Int32T fd, Int16T event, void * arg)
{
    static Int32T i;
    Int8T time[50];
    
    nnTimeGetCurrent(time);
   printf("Timer2 : %s  i = %d, fd = %d, event = %d\n", time, i++, fd, event);

    if(i == 5)
    {
        printf("arg : %p \n", arg);
        taskSignalDel(ret);
        taskTimerDel(arg);
    }

}

int main()
{
	struct timeval tv;
	struct timeval tv2;

	tv.tv_sec = 1;
	tv.tv_usec = 9;

	tv2.tv_sec = 1;
	tv2.tv_usec = 0;

	memInit(10);
	memSetDebug();
	
	taskCreate();
	ipcChannelOpen(10, ipc_register_callback);

        eventOpen(event_register_callback);

	eventSubscribe(1, 22);
	//eventUnsubscribe(1);

        taskTimerSet(timeCallback, tv, TASK_PERSIST);
	ret = taskSignalSet(SIGINT, signalIntCallback);
	printf("ret : %d\n", ret);
//        taskTimerSet(timeCallback2, tv2, TASK_PERSIST);
	taskDispatch();

	memClose();

	return 0;
}
