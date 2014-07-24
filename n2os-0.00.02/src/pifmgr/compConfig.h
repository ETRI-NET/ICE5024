#if !defined(_compConfig_h)
#define _compConfig_h

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
Int32T  sNosCompName         = PORT_INTERFACE_MANAGER;
StringT sNosCompNameStr      = "pifmgr";
StringT sNosCompModuleVer    = "0.0.1";
StringT sNosLogFile          = "pifmgr";
Int32T  sNosLogSize          = 10;
Int32T  sNosLogLocation      = LOG_FILE;
Int32T  sNosLogLevel         = LOG_DEBUG;

/* register component control process (SHOULD SET) */
void pifInitProcess(void);
void pifTermProcess(void);
void pifRestartProcess(void);
void pifHoldProcess(void);
NosInitProcessT     sNosInitProcess    = pifInitProcess;
NosTermProcessT     sNosTermProcess    = pifTermProcess;
NosRestartProcessT  sNosRestartProcess = pifRestartProcess;
NosHoldProcessT     sNosHoldProcess    = pifHoldProcess;

/* register message(Ipc/Event/Signal) process (SHOULD SET) */
void pifSignalProcess(Int32T sig);
void pifIpcProcess(Int32T msgId, void * data, Uint32T dataLen);
void pifEventProcess(Int32T msgId, void * data, Uint32T dataLen);
NosIpcProcessT    sNosIpcProcess    = pifIpcProcess;
NosEventProcessT  sNosEventProcess  = pifEventProcess;
NosSignalProcessT sNosSignalProcess = pifSignalProcess;

/* register CM/CMSH channel (SHOULD SET) */
void pifCmshIpcProcess(Int32T sockId, void *message, Uint32T size);
void pifCmIpcProcess(Int32T sockId, void *message, Uint32T size);
NosCmIpcProcessT sNosCmshIpcProcess = pifCmshIpcProcess;
NosCmIpcProcessT sNosCompIpcProcess = pifCmIpcProcess;

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

#endif   /* _compConfig */
