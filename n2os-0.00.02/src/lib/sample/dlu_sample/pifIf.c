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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "pifIf.h"
#include "nosLib.h"

PifCmiInterfaceT* mgrIfData = NULL;
PifTimerInterfaceT* mgrTimerData = NULL; 
PifFdInterfaceT* mgrFdData = NULL;
Int32T localCount = 0;

Int32T pifInterfaceConfigLU(StringT inIf) 
{
	sprintf(&(mgrIfData->ifName[0]), "%s", inIf);
	localCount++;
	return PIF_IF_CMI_SUCCESS;
}

PifCmiInterfaceT* pifInterfaceConfigGetLU(StringT inIf) 
{
	return mgrIfData;
}

PifTimerInterfaceT* pifTimerInterfaceConfigGetLU(StringT inIf)
{
    return mgrTimerData;
}

PifFdInterfaceT* pifFdInterfaceConfigGetLU(StringT inIf)
{
    return mgrFdData;
}

Int32T pifInterfaceGetCount()
{
	return localCount;
}

Int32T pifInterfaceConfig(StringT ifName) 
{
	return 0;
}

void* pifInterfaceInit(void* data)
{
	mgrIfData = (PifCmiInterfaceT*)NNMALLOC(MEM_TYPE_IF, sizeof(PifCmiInterfaceT));
	mgrIfData->version = 0;
	mgrIfData->totalCount = 0;
	sprintf(mgrIfData->ifName, "%s", "Not_set");

	printf("%s IF-Data(%p) \n",__FUNCTION__, mgrIfData);

	return (void *)mgrIfData;
}

void * pifTimerInterfaceInit(void* data)
{
    struct timeval tv1;
    struct timeval tv2;

    mgrTimerData 
     = (PifTimerInterfaceT*)NNMALLOC(MEM_TYPE_IF, sizeof(PifTimerInterfaceT));

    mgrTimerData->timerEvt1 = NULL;
    mgrTimerData->timerEvt2 = NULL;

	printf("%s IF-Data(%p) \n",__FUNCTION__, mgrTimerData);

    // Timer Setting
    tv1.tv_sec = 3;
    tv1.tv_usec = 0;

    tv2.tv_sec = 5;
    tv2.tv_usec = 0;

    mgrTimerData->timerEvt1
     = taskTimerSet(pifTimerProcess3Sec, tv1, TASK_PERSIST, NULL);

    mgrTimerData->timerEvt2 
     = taskTimerSet(pifTimerProcess5Sec, tv2, TASK_PERSIST, NULL); 

    printf("Timer1 : %s IF-Data(%p) \n",__FUNCTION__, mgrTimerData->timerEvt1);
    printf("Timer2 : %s IF-Data(%p) \n",__FUNCTION__, mgrTimerData->timerEvt2);
 
    return (void *)mgrTimerData;
}

void* pifFdInterfaceInit(void* data)
{
    Int32T sock;
    struct sockaddr_in server_addr;

    mgrFdData = (PifFdInterfaceT*)NNMALLOC(MEM_TYPE_IF, sizeof(PifFdInterfaceT));
    mgrFdData->actEvt = NULL;
    mgrFdData->readEvt = NULL;
   
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1)
    {
        printf("socket() error\n");
        exit(1);
    } 

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(50000);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
    {
        printf("bind error\n");
        exit(1);
    }

    if (listen(sock, 8) < 0) 
    {
        printf("listen error\n");
        exit(1);
    }

    
    mgrFdData->actEvt
     = taskFdSet(pifFdAcceptProcess, sock, TASK_READ|TASK_PERSIST, TASK_PRI_MIDDLE); 

    printf("%s IF-Data(%p) \n",__FUNCTION__, mgrFdData);

    return (void *)mgrFdData;
}

void pifInterfaceRestart(void* data)
{
	mgrIfData = (PifCmiInterfaceT*)data;
	printf("%s IF-Data(%p) \n", __FUNCTION__, mgrIfData);
}

void pifTimerInterfaceRestart(void* data)
{
    struct timeval tv1;
    struct timeval tv2;
    mgrTimerData = (PifTimerInterfaceT*)data;
    
	printf("%s IF-Data(%p) \n",__FUNCTION__, mgrTimerData);

    // Timer Setting
    tv1.tv_sec = 3;
    tv1.tv_usec = 0;

    tv2.tv_sec = 7;
    tv2.tv_usec = 0;

    mgrTimerData->timerEvt1
     = taskTimerUpdate(pifTimerProcess3Sec, mgrTimerData->timerEvt1, tv1, NULL);

    mgrTimerData->timerEvt2 
     = taskTimerUpdate(pifTimerProcess5Sec, mgrTimerData->timerEvt2, tv2, NULL); 

    printf("Timer1 : %s IF-Data(%p) \n",__FUNCTION__, mgrTimerData->timerEvt1);
    printf("Timer2 : %s IF-Data(%p) \n",__FUNCTION__, mgrTimerData->timerEvt2);
 
}

void pifFdInterfaceRestart(void* data)
{
    mgrFdData = (PifFdInterfaceT*)data;
    
    mgrFdData->actEvt = 
     taskFdUpdate(pifFdAcceptProcess, mgrFdData->actEvt);

    mgrFdData->readEvt =
     taskFdUpdate(pifFdRead, mgrFdData->readEvt);

    printf("%s IF-Data(%p) \n",__FUNCTION__, mgrFdData);
}

void pifInterfaceStr(StringT str, Int32T rlt)
{
	switch(rlt)
	{
		case PIF_IF_CMI_SUCCESS:
			sprintf(str, "command ok"); 
			break;
		case PIF_IF_CMI_NOT_EXIST:
			sprintf(str, "interface not exist"); 
			break;
		case PIF_IF_CMI_INVALID_NAME:
			sprintf(str, "invalid interface name"); 
			break;
		default: 
			sprintf(str, "unknown command"); 
	}
}




