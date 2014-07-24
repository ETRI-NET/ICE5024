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
Int32T  sNosCompName         = POLICY_MANAGER;
StringT sNosCompModuleVer    = "0.0.1";
StringT sNosLogFile          = "polmgr";
Int32T  sNosLogSize          = 10;
Int32T  sNosLogLocation      = LOG_FILE;
Int32T  sNosLogLevel         = LOG_DEBUG;

/* register component control process (SHOULD SET) */
void polInitProcess(void);
void polTermProcess(void);
void polRestartProcess(void);
void polHoldProcess(void);
NosInitProcessT     sNosInitProcess    = polInitProcess;
NosTermProcessT     sNosTermProcess    = polTermProcess;
NosRestartProcessT  sNosRestartProcess = polRestartProcess;
NosHoldProcessT     sNosHoldProcess    = polHoldProcess;

/* register message(Ipc/Event/Signal) process (SHOULD SET) */
void polIpcProcess(Int32T msgId, void * data, Uint32T dataLen);
void polEventProcess(Int32T msgId, void * data, Uint32T dataLen);
void polSignalProcess(Int32T sig);
NosIpcProcessT    sNosIpcProcess    = polIpcProcess;
NosEventProcessT  sNosEventProcess  = polEventProcess;
NosSignalProcessT sNosSignalProcess = polSignalProcess;

void polCmshIpcProcess(Int32T sockId, void *message, Uint32T size);
void polCmIpcProcess(Int32T sockId, void *message, Uint32T size);
NosCmIpcProcessT sNosCmshIpcProcess = polCmshIpcProcess;  /** Channel Process CMSH  */
NosCmIpcProcessT sNosCompIpcProcess = polCmIpcProcess;  /** Channel Process COMP  */

/* nos event register & receive process (SHOULD SET) */
Int32T sNosComponentEventNum = 6;
struct NosEventRegister
{
	Int32T eventId;
	Int32T priority;
} sNosEventRegister[] __attribute__ ((unused)) = 
{
	{EVENT_INTERFACE_ADD,     EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_DELETE,  EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_UP,      EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_DOWN,    EVENT_PRI_MIDDLE},

        // Process Manager Event
        {EVENT_LCS_COMPONENT_ERROR_OCCURRED, EVENT_PRI_MIDDLE},
        {EVENT_LCS_COMPONENT_SERVICE_STATUS, EVENT_PRI_MIDDLE}
};

#endif   /* _compConfig */
