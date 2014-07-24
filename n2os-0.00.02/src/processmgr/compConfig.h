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
Int32T  sNosCompName         = PROCESS_MANAGER;
StringT sNosCompModuleVer    = "0.0.1";
StringT sNosLogFile          = "procLog";
Int32T  sNosLogSize          = 10;
Int32T  sNosLogLocation      = LOG_FILE;
Int32T  sNosLogLevel         = LOG_DEBUG;

/* register component control process (SHOULD SET) */
void procInitProcess(void);
void procTermProcess(void);
void procRestartProcess(void);
void procHoldProcess(void);
NosInitProcessT     sNosInitProcess    = procInitProcess;
NosTermProcessT     sNosTermProcess    = procTermProcess;
NosRestartProcessT  sNosRestartProcess = procRestartProcess;
NosHoldProcessT     sNosHoldProcess    = procHoldProcess;

/* register message(Ipc/Event/Signal) process (SHOULD SET) */
void procIpcProcess(Int32T msgId, void * data, Uint32T dataLen);
void procEventProcess(Int32T msgId, void * data, Uint32T dataLen);
void procSignalProcess(Int32T sig);
NosIpcProcessT    sNosIpcProcess    = procIpcProcess;
NosEventProcessT  sNosEventProcess  = procEventProcess;
NosSignalProcessT sNosSignalProcess = procSignalProcess;

void procCmshIpcProcess(Int32T sockId, void *message, Uint32T size);
void procCmIpcProcess(Int32T sockId, void *message, Uint32T size);
NosCmIpcProcessT sNosCmshIpcProcess = procCmshIpcProcess;  /** Channel Process CMSH  */
NosCmIpcProcessT sNosCompIpcProcess = procCmIpcProcess;  /** Channel Process COMP  */

/* nos event register & receive process (SHOULD SET) */
Int32T sNosComponentEventNum = 2;
struct NosEventRegister
{
	Int32T eventId;
	Int32T priority;
} sNosEventRegister[] __attribute__ ((unused)) = 
{
	{EVENT_LCS_COMPONENT_ERROR_OCCURRED, EVENT_PRI_MIDDLE},
	{EVENT_LCS_COMPONENT_SERVICE_STATUS, EVENT_PRI_MIDDLE}
};

#endif   /* _compConfig */
