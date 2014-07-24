/*
 * isisInit.c
 *
 *  Created on: 2014. 4. 23.
 *      Author: root
 */

#include "vty.h"
#include "if.h"

#include <linux/netlink.h>
#include "nnTypes.h"
#include "nnStr.h"
#include "nnVector.h"
#include "nnList.h"
#include "nnPrefix.h"
#include "nnBuffer.h"
#include "nosLib.h"
#include "nnDefines.h"
#include "nnRibDefines.h"

#include "lcsService.h"

#include "nnCmdDefines.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCompProcess.h"

#include "isisInit.h"
#include "isisd.h"
#include "isisInterface.h"
#include "isis_circuit.h"
#include "isisRibmgr.h"
#include "isisDebug.h"
#include "isis_route.h"
#include "isis_zebra.h"
#include "isisIpc.h"

/*
 * External Definitions for Global Data Structure
 */
extern void ** gCompData;

void
isisCmIpcProcess(Int32T sockId, void *message, Uint32T size)
{
  printf("==>> %s %d...\n", __func__, __LINE__);
  compCmdIpcProcess(isis->pCmshGlobal, sockId, message, size);
}


/**
 * Description : Command shell channel
 *
 * @retval : none
 */
void
isisCmshIpcProcess (Int32T sockId, void *message, Uint32T size)
{
  printf("==>> %s %d...\n", __func__, __LINE__);
  compCmdIpcProcess(isis->pCmshGlobal, sockId, message, size);
}


void
isisTermProcess ()
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);

  isisSignalProcess (0);
}

void
isisRestartProcess()
{
	NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);

	/* Assign global memory pointer to shared memory pointer. */
	isis = (struct isis *)(*gCompData);

	/* Assign each data pointer. */
	isisVersionUpdate();
}

void
isisHoldProcess()
{
	NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);
}

/*
 * Description : When we display running config, this function will be called.
 */
Int32T
isisWriteConfCB(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv)
{
	/* Check global isis pointer. */
	if (!isis) {
		return CMD_IPC_OK;
	}

	/* Isis debug's configure. */
   configWriteIsisDebug (cmsh);

	/* Interface Node's configure. */
	configWriteIsisInterface(cmsh);

	/* Isis node's configure. */
	configWriteIsis(cmsh);

	return CMD_IPC_OK;
}


void
isisInitProcess( void )
{
	NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);

	/*
	 * Process Manager 가 Process 를 관리할 때 사용할 속성들을 담은 구조체로
	 * 변경을 수행하지 않을 땐 값을 0 으로 사용
	 */
	LcsAttributeT lcsAttribute = { 0, };

	isisInit();

	/* Assign shared memory pointer to global memory pointer. */
	(*gCompData) = (void *) isis;

	/* 초기화 작업 이후 Process Manager 에게 Register Message 전송 */
	lcsRegister(LCS_ISIS, ISIS, lcsAttribute);

}

void
isisEventProcess (Int32T msgId, void * data, Uint32T dataLen)
{
	printf("############################\n");
	printf("## %s called !!!\n", __func__);
	printf("############################\n");

	/* Buffer Reset & Assign */
	nnBufferT msgBuff;
	nnBufferReset(&msgBuff);
	nnBufferAssign(&msgBuff, data, dataLen);

	switch (msgId) {
	/* Process Manager Event. */
	case EVENT_LCS_COMPONENT_ERROR_OCCURRED:
		isisLcsEventComponentErrorOccured(data, dataLen);
		break;
	case EVENT_LCS_COMPONENT_SERVICE_STATUS:
		isisLcsEventComponentServiceStatus(data, dataLen);
		break;

	case EVENT_ROUTER_ID:
		printf("EVENT_ROUTER_ID\n");
		isisRouterIdUpdate(&msgBuff);
		break;
	case EVENT_INTERFACE_ADD:
		printf("EVENT_INTERFACE_ADD\n");
		isisInterfaceAdd(&msgBuff);
		break;

	case EVENT_INTERFACE_DELETE:
		printf("EVENT_INTERFACE_DELETE\n");
		isisInterfaceDelete(&msgBuff);
		break;

	case EVENT_INTERFACE_ADDRESS_ADD:
		printf("EVENT_INTERFACE_ADDRESS_ADD\n");
		isisInterfaceAddressAdd(&msgBuff);
		break;

	case EVENT_INTERFACE_ADDRESS_DELETE:
		printf("EVENT_INTERFACE_ADDRESS_DELETE\n");
		isisInterfaceAddressDelete(&msgBuff);
		break;

	case EVENT_INTERFACE_UP:
		printf("EVENT_INTERFACE_UP\n");
		isisInterfaceUp(&msgBuff);
		break;

	case EVENT_INTERFACE_DOWN:
		printf("EVENT_INTERFACE_DOWN\n");
		isisInterfaceDown(&msgBuff);
		break;
	default:
		printf("EVENT_UNKNOWN....\n");
		break;
	}
}

void
isisSignalProcess(Int32T signalType)
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);

//  isisRibmgrClose (); /* Close request to ribmgr. */

  /* Component Command Close */
    compCmdFree(isis->pCmshGlobal);


//  ifClose(); /* Free interface's memory */

//  NNFREE (MEM_GLOBAL, pRip); /* Free Global Ribmgr Memory */

  taskClose();
  nnLogClose();
  memClose();

//  exit(1);
}


void
isisIpcProcess (Int32T msgId, void * data, Uint32T dataLen)
{
	NNLOG(LOG_DEBUG, "ipcReadCallback msgId: %d\n", msgId);

	/* Buffer Reset & Assign. */
	nnBufferT msgBuff;
	nnBufferReset(&msgBuff);
	nnBufferAssign(&msgBuff, data, dataLen);
	switch (msgId) {
	/* ProcessManager Interface IPC message. */
	case IPC_LCS_PM2C_SETROLE:
		isisLcsSetRole(data, dataLen);
		break;
	case IPC_LCS_PM2C_TERMINATE:
		isisLcsTerminate(data, dataLen);
		break;
	case IPC_LCS_PM2C_HEALTHCHECK:
		isisLcsHealthcheck(data, dataLen);
		break;

		/* Ribmgr Interface IPC message. */
	case IPC_RIB_ROUTER_ID_SET:
		isisRouterIdUpdate(&msgBuff);
		break;

	case IPC_RIB_IPV4_ROUTE_ADD:
		isisRibmgrReadIpv4(msgId, &msgBuff, dataLen);
		break;

	case IPC_RIB_IPV4_ROUTE_DELETE:
		isisRibmgrReadIpv4(msgId, &msgBuff, dataLen);
		break;

	case IPC_INTERFACE_ADD:
		isisInterfaceAdd(&msgBuff);
		break;

	case IPC_INTERFACE_UP:
		isisInterfaceUp(&msgBuff);
		break;

	case IPC_INTERFACE_ADDRESS_ADD:
		isisInterfaceAddressAdd(&msgBuff);
		break;

	default:
		break;
	}
}

