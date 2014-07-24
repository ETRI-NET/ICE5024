/*345678901234567890123456789012345678901234567890123456789012345678901234567890*/
/*********************************************************************************
 *               Electronics and Telecommunications Research Institute
 * Filename : <myFileName>
 * Blockname: <PIF Manager>
 * Overview : <PIF Manager S/W block manages Port/Interface & L2 MAC/VLAN>
 * Creator  : <Seungwoo Hong>
 * Owner    : <Seungwoo Hong>
 * Copyright: 2013 Electronics and Telecommunications Research Institute. 
 *            All rights reserved. No part of this software shall be reproduced, 
 *            stored in a retrieval system, or transmitted by any means, 
 *            electronic, mechanical, photocopying, recording, or otherwise, 
 *            without written permission from ETRI.
 *********************************************************************************/

/*********************************************************************************
 *                        VERSION CONTROL  INFORMATION
 * $Author$
 * $Date$
 * $Revision
 * $Log$ 
 *********************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "nosLib.h"
#include "pifIf.h"
#include "nnTime.h"

extern StringT sNosCompModuleVer;

// FD Accept
void pifFdAcceptProcess(Int32T fd, Int16T event, void * arg)
{
    PifFdInterfaceT* ifData = pifFdInterfaceConfigGetLU(NULL);
    Int32T clifd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    if(ifData == NULL)
    {
		printf("IF data NULL \n");
		return;
	}

    clifd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
    ifData->readEvt 
     = taskFdSet(pifFdRead, clifd, TASK_READ|TASK_PERSIST, TASK_PRI_MIDDLE);

    printf("Accepted connection from %s\n", inet_ntoa(client_addr.sin_addr));

}

// FD Read
void pifFdRead(Int32T fd, Int16T event, void * arg)
{
    Int8T buf[1024] = {0, };
    Int32T len;
    PifFdInterfaceT* ifData = pifFdInterfaceConfigGetLU(NULL);

    len = read(fd, buf, sizeof(buf));

    if(len == 0)
    {
        printf("client disconnect");
        close(fd);
        taskFdDel(ifData->readEvt);
    }
    else if(len < 0)
    {
        printf("Socket failure, disconnecting client\n");
        close(fd);
        taskFdDel(ifData->readEvt);
    }
    else
    {
        //do anything
        Int8T date[32] = {0, };
        printf("recv msg : %s\n", buf);
        memset(buf, 0x00, sizeof(buf));

        nnTimeGetCurrent(date);
        sprintf(buf, "[%s] reply msg Before : %s\n", date, sNosCompModuleVer);
        len = write(fd, buf, strlen(buf));
    }
}
