#if !defined(_testerDluConfig_h)
#define _testerDluConfig_h

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
Int32T  sNosCompName         = RIB_TESTER;
StringT sNosCompNameStr      = "ribTester";
StringT sNosCompModuleVer    = "0.0.1";
StringT sNosLogFile          = "ribTester";
Int32T  sNosLogSize          = 10;
Int32T  sNosLogLocation      = LOG_FILE;
Int32T  sNosLogLevel         = LOG_DEBUG;

/* register component control process (SHOULD SET) */
void testerInitProcess(void);
void testerTermProcess(void);
void testerRestartProcess(void);
void testerHoldProcess(void);
NosInitProcessT     sNosInitProcess    = testerInitProcess;
NosTermProcessT     sNosTermProcess    = testerTermProcess;
NosRestartProcessT  sNosRestartProcess = testerRestartProcess;
NosHoldProcessT     sNosHoldProcess    = testerHoldProcess;

/* register message(Ipc/Event/Signal) process (SHOULD SET) */
void testerIpcProcess(Int32T msgId, void * data, Uint32T dataLen);
void testerEventProcess(Int32T msgId, void * data, Uint32T dataLen);
void testerSignalProcess(Int32T sig);
NosIpcProcessT    sNosIpcProcess    = testerIpcProcess;
NosEventProcessT  sNosEventProcess  = testerEventProcess;
NosSignalProcessT sNosSignalProcess = testerSignalProcess;

/* register CM/CMSH process (SHOULD SET) */
void testerCmshIpcProcess(Int32T sockId, void *message, Uint32T size);
void testerCmIpcProcess(Int32T sockId, void *message, Uint32T size);
NosCmIpcProcessT sNosCmshIpcProcess = testerCmshIpcProcess;
NosCmIpcProcessT sNosCompIpcProcess = testerCmIpcProcess;

/* nos event register & receive process (SHOULD SET) */
Int32T sNosComponentEventNum = 6;
struct NosEventRegister
{
	Int32T eventId;
	Int32T priority;
} sNosEventRegister[] __attribute__ ((unused)) = 
{
	{EVENT_INTERFACE_ADD,         EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_DELETE,      EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_UP,          EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_DOWN,        EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_ADDRESS_ADD, EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_DELETE,     EVENT_PRI_MIDDLE}
};

#endif   /* _testerDluConfig */
