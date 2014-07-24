/*
 * ospfInit.c
 *
 *  Created on: 2014. 5. 30.
 *      Author: root
 */

#include "version.h"
#include "getopt.h"
#include "prefix.h"
#include "linklist.h"
#include "if.h"
#include "vector.h"
#include "vty.h"
#include "command.h"
#include "filter.h"
#include "plist.h"
#include "stream.h"
#include "log.h"
#include "memory.h"
#include "privs.h"
#include "sigevent.h"
#include "zclient.h"

#include "nnTypes.h"
#include "nnStr.h"
#include "nnVector.h"
#include "nnList.h"
#include "nnPrefix.h"
#include "nnBuffer.h"
#include "nosLib.h"
#include "nnRibDefines.h"

#include "nnCmdDefines.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCompProcess.h"
#include "lcsService.h"

#include "ospfd.h"
#include "ospf_interface.h"
#include "ospf_asbr.h"
#include "ospf_lsa.h"
#include "ospf_lsdb.h"
#include "ospf_neighbor.h"
#include "ospf_dump.h"
#include "ospf_zebra.h"
#include "ospf_vty.h"
#include "ospfRibmgr.h"
#include "ospf_opaque.h"
#include "ospfInit.h"
#include "ospfIpc.h"






/*
 * External Definitions for Global Data Structure
 */
extern void ** gCompData;

void
ospfCmIpcProcess(Int32T sockId, void *message, Uint32T size)
{
  printf("==>> %s %d...\n", __func__, __LINE__);
  compCmdIpcProcess(om->pCmshGlobal, sockId, message, size);
}


/**
 * Description : Command shell channel
 *
 * @retval : none
 */
void
ospfCmshIpcProcess (Int32T sockId, void *message, Uint32T size)
{
  printf("==>> %s %d...\n", __func__, __LINE__);
  compCmdIpcProcess(om->pCmshGlobal, sockId, message, size);
}

/*
 * Description : When we display running config, this function will be called.
 */
Int32T
ospfWriteConfCB(struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv)
{
	/* Check global ospf pointer. */
	if (!om) {
		return CMD_IPC_OK;
	}

	/* Ospf debug's configure. */
	config_write_debug(cmsh);

	/* Interface Node's configure. */
	configWriteOspfInterface(cmsh);

	/* Rip node's configure. */
	configWriteOspf(cmsh);

	return CMD_IPC_OK;
}

void
ospfInitProcess()
{
	/*
	 * Process Manager 가 Process 를 관리할 때 사용할 속성들을 담은 구조체로
	 * 변경을 수행하지 않을 땐 값을 0 으로 사용
	 */
	LcsAttributeT lcsAttribute = { 0, };

	/* OSPF master init. */
	ospf_master_init();

	/* Command Init. */
	om->pCmshGlobal = compCmdInit(IPC_OSPF, ospfWriteConfCB);

	accessListInit();
	accessListAddHook(ospf_filter_update);
	accessListDeleteHook(ospf_filter_update);
	om->accessMasterIpv4 = accessListGetMaster4();

	prefixListInit();
	prefixListAddHook(ospf_prefix_list_update);
	prefixListDeleteHook(ospf_prefix_list_update);
	om->prefixMasterIpv4 = prefixListGetMaster4();

	/* OSPFd inits. */
	ospf_if_init();

	ospf_route_map_init();
#ifdef HAVE_SNMP
	ospf_snmp_init ();
#endif /* HAVE_SNMP */
#ifdef HAVE_OPAQUE_LSA
	ospf_opaque_init();
#endif /* HAVE_OPAQUE_LSA */

	/* Assign shared memory pointer to global memory pointer. */
	(*gCompData) = (void *) om;

	/* Registrate to Ribmgr. */
	ospfRibmgrInit();

	/* 초기화 작업 이후 Process Manager 에게 Register Message 전송 */
	lcsRegister(LCS_OSPF, OSPF, lcsAttribute);
}

void
ospfTermProcess ()
{
	NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);

	ospfSignalProcess(0);
}

void
ospfRestartProcess ()
{
	/* Check rip global pointer. */
	if (!om)
		return;

	/* Assign global memory pointer to shared memory pointr. */
	om = (struct ospf_master *) (*gCompData);

	/* Assign each data pointer. */
	ospfVersionUpdate();
}

void
ospfHoldProcess ()
{

}

void
ospfSignalProcess (Int32T signalType)
{

	NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);

//	ospfRibmgrClose (); /* Close request to ribmgr. */

	/* Component Command Close */
	compCmdFree(om->pCmshGlobal);

	//  ifClose(); /* Free interface's memory */
	if_terminate();

	//  NNFREE (MEM_GLOBAL, pRip); /* Free Global Ribmgr Memory */

	taskClose();
	nnLogClose();
	memClose();

	exit(1);

}

void
ospfIpcProcess (Int32T msgId, void * data, Uint32T dataLen)
{
	NNLOG(LOG_DEBUG, "ipcReadCallback msgId: %d\n", msgId);

	/* Buffer Reset & Assign. */
	nnBufferT msgBuff;
	nnBufferReset(&msgBuff);
	nnBufferAssign(&msgBuff, data, dataLen);

	switch (msgId) {
	/* ProcessManager Interface IPC message. */
	case IPC_LCS_PM2C_SETROLE:
		ospfLcsSetRole(data, dataLen);
		break;
	case IPC_LCS_PM2C_TERMINATE:
		ospfLcsTerminate(data, dataLen);
		break;
	case IPC_LCS_PM2C_HEALTHCHECK:
		ospfLcsHealthcheck(data, dataLen);
		break;

	case IPC_RIB_ROUTER_ID_SET:
		ospfRouterIdUpdate(&msgBuff);
		break;

	case IPC_RIB_IPV4_ROUTE_ADD:
		ospfRibmgrReadIpv4(msgId, &msgBuff, dataLen);
		break;

	case IPC_RIB_IPV4_ROUTE_DELETE:
		ospfRibmgrReadIpv4(msgId, &msgBuff, dataLen);
		break;

	case IPC_INTERFACE_ADD:
		ospfInterfaceAdd(&msgBuff);
		break;

	case IPC_INTERFACE_UP:
		ospfInterfaceStateUp(&msgBuff);
		break;

	case IPC_INTERFACE_ADDRESS_ADD:
		ospfInterfaceAddressAdd(&msgBuff);
		break;

	default:
		break;
	}
}

void
ospfEventProcess (Int32T msgId, void * data, Uint32T dataLen)
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
		ospfLcsEventComponentErrorOccured(data, dataLen);
		break;
	case EVENT_LCS_COMPONENT_SERVICE_STATUS:
		ospfLcsEventComponentServiceStatus(data, dataLen);
		break;

	case EVENT_ROUTER_ID:
		printf("EVENT_ROUTER_ID\n");
		ospfRouterIdUpdate(&msgBuff);
		break;

	case EVENT_INTERFACE_ADD:
		printf("EVENT_INTERFACE_ADD\n");
		ospfInterfaceAdd(&msgBuff);
		break;

	case EVENT_INTERFACE_DELETE:
		printf("EVENT_INTERFACE_DELETE\n");
		ospfInterfaceDelete(&msgBuff);
		break;

	case EVENT_INTERFACE_ADDRESS_ADD:
		printf("EVENT_INTERFACE_ADDRESS_ADD\n");
		ospfInterfaceAddressAdd(&msgBuff);
		break;

	case EVENT_INTERFACE_ADDRESS_DELETE:
		printf("EVENT_INTERFACE_ADDRESS_DELETE\n");
		ospfInterfaceAddressDelete(&msgBuff);
		break;

	case EVENT_INTERFACE_UP:
		printf("EVENT_INTERFACE_UP\n");
		ospfInterfaceStateUp(&msgBuff);
		break;

	case EVENT_INTERFACE_DOWN:
		printf("EVENT_INTERFACE_DOWN\n");
		ospfInterfaceStateDown(&msgBuff);
		break;
	default:
		printf("EVENT_UNKNOWN....\n");
		break;
	}

}

