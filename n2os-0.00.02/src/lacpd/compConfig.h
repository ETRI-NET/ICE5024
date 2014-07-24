#if !defined(_lacpDluConfig_h)
#define _lacpDluConfig_h

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
Int32T  sNosCompName         = LACP;
StringT sNosCompNameStr      = "lacp";
StringT sNosCompModuleVer    = "0.0.1";
StringT sNosLogFile          = "lacp";
Int32T  sNosLogSize          = 10;
Int32T  sNosLogLocation      = LOG_FILE;
Int32T  sNosLogLevel         = LOG_DEBUG;

/* register component control process (SHOULD SET) */
void lacpInitProcess(void);
void lacpTermProcess(void);
void lacpRestartProcess(void);
void lacpHoldProcess(void);
NosInitProcessT     sNosInitProcess    = lacpInitProcess;
NosTermProcessT     sNosTermProcess    = lacpTermProcess;
NosRestartProcessT  sNosRestartProcess = lacpRestartProcess;
NosHoldProcessT     sNosHoldProcess    = lacpHoldProcess;

/* register message(Ipc/Event/Signal) process (SHOULD SET) */
void lacpIpcProcess(Int32T msgId, void * data, Uint32T dataLen);
void lacpEventProcess(Int32T msgId, void * data, Uint32T dataLen);
void lacpSignalProcess(Int32T sig);
NosIpcProcessT    sNosIpcProcess    = lacpIpcProcess;
NosEventProcessT  sNosEventProcess  = lacpEventProcess;
NosSignalProcessT sNosSignalProcess = lacpSignalProcess;

/* register CM/CMSH process (SHOULD SET) */
void lacpCmshIpcProcess(Int32T sockId, void *message, Uint32T size);
void lacpCmIpcProcess(Int32T sockId, void *message, Uint32T size);
NosCmIpcProcessT sNosCmshIpcProcess = lacpCmshIpcProcess;
NosCmIpcProcessT sNosCompIpcProcess = lacpCmIpcProcess;

/* nos event register & receive process (SHOULD SET) */
Int32T sNosComponentEventNum = 3;
struct NosEventRegister
{
	Int32T eventId;
	Int32T priority;
} sNosEventRegister[] __attribute__ ((unused)) = 
{
    /* Process Manager Event. */
    {EVENT_LCS_COMPONENT_ERROR_OCCURRED, EVENT_PRI_MIDDLE},
    {EVENT_LCS_COMPONENT_SERVICE_STATUS, EVENT_PRI_MIDDLE},
    /* Port Interface evnets. */
	{EVENT_PIF_L2_LINK,	EVENT_PRI_MIDDLE},
};

#endif   /* _lacpDluConfig */
