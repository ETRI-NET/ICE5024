#if !defined(_ospfDluConfig_h)
#define _ospfDluConfig_h

#include "nosLib.h"

//nos work callback functions definition
typedef void (*NosInitProcessT)(void);
typedef void (*NosTermProcessT)(void);
typedef void (*NosRestartProcessT)(void);
typedef void (*NosHoldProcessT)(void);
typedef void (*NosSignalProcessT)(Int32T sig);
typedef void (*NosIpcProcessT)(Int32T msgId, void * data, Uint32T dataLen);
typedef void (*NosEventProcessT)(Int32T msgId, void * data, Uint32T dataLen);
typedef void (*NosCmIpcProcessT)(Int32T sockId, void *message, Uint32T size);

/**************************************************************************
 *                      NOS Component Configuration 
 *************************************************************************/
/* Nos Component Configuration (SHOULD SET) */
Int32T  sNosCompName         = OSPF;
StringT sNosCompNameStr      = "ospf";
StringT sNosCompModuleVer    = "0.0.1";
StringT sNosLogFile          = "ospf";
Int32T  sNosLogSize          = 10;
Int32T  sNosLogLocation      = LOG_FILE;
Int32T  sNosLogLevel         = LOG_DEBUG;

/* register component control process (SHOULD SET) */
void ospfInitProcess(void);
void ospfTermProcess(void);
void ospfRestartProcess(void);
void ospfHoldProcess(void);
NosInitProcessT     sNosInitProcess    = ospfInitProcess;
NosTermProcessT     sNosTermProcess    = ospfTermProcess;
NosRestartProcessT  sNosRestartProcess = ospfRestartProcess;
NosHoldProcessT     sNosHoldProcess    = ospfHoldProcess;

/* register message(Ipc/Event/Signal) process (SHOULD SET) */
void ospfIpcProcess(Int32T msgId, void * data, Uint32T dataLen);
void ospfEventProcess(Int32T msgId, void * data, Uint32T dataLen);
void ospfSignalProcess(Int32T sig);
NosIpcProcessT    sNosIpcProcess    = ospfIpcProcess;
NosEventProcessT  sNosEventProcess  = ospfEventProcess;
NosSignalProcessT sNosSignalProcess = ospfSignalProcess;

/* register CM/CMSH process (SHOULD SET) */
void ospfCmshIpcProcess(Int32T sockId, void *message, Uint32T size);
void ospfCmIpcProcess(Int32T sockId, void *message, Uint32T size);
NosCmIpcProcessT sNosCmshIpcProcess = ospfCmshIpcProcess;
NosCmIpcProcessT sNosCompIpcProcess = ospfCmIpcProcess;

/* nos event register & receive process (SHOULD SET) */
Int32T sNosComponentEventNum = 7;
struct NosEventRegister
{
	Int32T eventId;
	Int32T priority;
} sNosEventRegister[] __attribute__ ((unused)) = 
{
	{EVENT_ROUTER_ID,		         EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_ADD,         EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_DELETE,      EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_UP,          EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_DOWN,        EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_ADDRESS_ADD, EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_ADDRESS_DELETE, EVENT_PRI_MIDDLE}
};

#endif   /* _ospfDluConfig */
