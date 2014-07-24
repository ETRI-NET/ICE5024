#if !defined(_ripDluConfig_h)
#define _ripDluConfig_h

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
Int32T  sNosCompName         = RIP;
StringT sNosCompNameStr      = "rip";
StringT sNosCompModuleVer    = "0.0.1";
StringT sNosLogFile          = "rip";
Int32T  sNosLogSize          = 10;
Int32T  sNosLogLocation      = LOG_FILE;
Int32T  sNosLogLevel         = LOG_DEBUG;

/* register component control process (SHOULD SET) */
void ripInitProcess(void);
void ripTermProcess(void);
void ripRestartProcess(void);
void ripHoldProcess(void);
NosInitProcessT     sNosInitProcess    = ripInitProcess;
NosTermProcessT     sNosTermProcess    = ripTermProcess;
NosRestartProcessT  sNosRestartProcess = ripRestartProcess;
NosHoldProcessT     sNosHoldProcess    = ripHoldProcess;

/* register message(Ipc/Event/Signal) process (SHOULD SET) */
void ripIpcProcess(Int32T msgId, void * data, Uint32T dataLen);
void ripEventProcess(Int32T msgId, void * data, Uint32T dataLen);
void ripSignalProcess(Int32T sig);
NosIpcProcessT    sNosIpcProcess    = ripIpcProcess;
NosEventProcessT  sNosEventProcess  = ripEventProcess;
NosSignalProcessT sNosSignalProcess = ripSignalProcess;

/* register CM/CMSH process (SHOULD SET) */
void ripCmshIpcProcess(Int32T sockId, void *message, Uint32T size);
void ripCmIpcProcess(Int32T sockId, void *message, Uint32T size);
NosCmIpcProcessT sNosCmshIpcProcess = ripCmshIpcProcess;
NosCmIpcProcessT sNosCompIpcProcess = ripCmIpcProcess;

/* nos event register & receive process (SHOULD SET) */
Int32T sNosComponentEventNum = 8;
struct NosEventRegister
{
	Int32T eventId;
	Int32T priority;
} sNosEventRegister[] __attribute__ ((unused)) = 
{
    /* Process Manager Event. */
    {EVENT_LCS_COMPONENT_ERROR_OCCURRED, EVENT_PRI_MIDDLE},
    {EVENT_LCS_COMPONENT_SERVICE_STATUS, EVENT_PRI_MIDDLE},
    /* Interface evnets. */
	{EVENT_INTERFACE_ADD,         EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_DELETE,      EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_UP,          EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_DOWN,        EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_ADDRESS_ADD, EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_ADDRESS_DELETE, EVENT_PRI_MIDDLE}
};

#endif   /* _ripDluConfig */
