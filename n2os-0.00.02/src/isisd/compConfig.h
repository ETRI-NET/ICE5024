#if !defined(_isisDluConfig_h)
#define _isisDluConfig_h

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
Int32T  sNosCompName         = ISIS;
StringT sNosCompNameStr      = "isis";
StringT sNosCompModuleVer    = "0.0.1";
StringT sNosLogFile          = "isis";
Int32T  sNosLogSize          = 10;
Int32T  sNosLogLocation      = LOG_FILE;
Int32T  sNosLogLevel         = LOG_DEBUG;

/* register component control process (SHOULD SET) */
void isisInitProcess(void);
void isisTermProcess(void);
void isisRestartProcess(void);
void isisHoldProcess(void);
NosInitProcessT     sNosInitProcess    = isisInitProcess;
NosTermProcessT     sNosTermProcess    = isisTermProcess;
NosRestartProcessT  sNosRestartProcess = isisRestartProcess;
NosHoldProcessT     sNosHoldProcess    = isisHoldProcess;

/* register message(Ipc/Event/Signal) process (SHOULD SET) */
void isisIpcProcess(Int32T msgId, void * data, Uint32T dataLen);
void isisEventProcess(Int32T msgId, void * data, Uint32T dataLen);
void isisSignalProcess(Int32T sig);
NosIpcProcessT    sNosIpcProcess    = isisIpcProcess;
NosEventProcessT  sNosEventProcess  = isisEventProcess;
NosSignalProcessT sNosSignalProcess = isisSignalProcess;

/* register CM/CMSH process (SHOULD SET) */
void isisCmshIpcProcess(Int32T sockId, void *message, Uint32T size);
void isisCmIpcProcess(Int32T sockId, void *message, Uint32T size);
NosCmIpcProcessT sNosCmshIpcProcess = isisCmshIpcProcess;
NosCmIpcProcessT sNosCompIpcProcess = isisCmIpcProcess;

/* nos event register & receive process (SHOULD SET) */
Int32T sNosComponentEventNum = 7;
struct NosEventRegister
{
	Int32T eventId;
	Int32T priority;
} sNosEventRegister[] __attribute__ ((unused)) = 
{
	{EVENT_ROUTER_ID,       		  EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_ADD,         EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_DELETE,      EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_UP,          EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_DOWN,        EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_ADDRESS_ADD, EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_ADDRESS_DELETE, EVENT_PRI_MIDDLE}
};

#endif   /* _isisDluConfig */
