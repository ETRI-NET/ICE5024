#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "../taskManager.h"

static void ipc_register_callback(int msgId, void * data, Uint32T dataLen)
{
    printf("msgId : %d, dataLen : %d\n", msgId, dataLen);

}

static void event_register_callback(int eventId, void * data, Uint32T dataLen)
{
    printf("eventId : %d, dataLen : %d\n", eventId, dataLen);
}


int main()
{
	memInit(5);
	memSetDebug();
	
	taskCreate();

	ipcChannelOpen(5, ipc_register_callback);

        eventOpen(event_register_callback);
	eventSubscribe(1, 4);
        
	
	//eventUnsubscribe(1);

	taskDispatch();

	memClose();

	return 0;
}
