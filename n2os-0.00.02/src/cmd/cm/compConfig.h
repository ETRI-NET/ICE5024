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
Int32T  sNosCompName         = COMMAND_MANAGER;
StringT sNosCompModuleVer    = "0.0.1";
StringT sNosLogFile          = "cmLog";
Int32T  sNosLogSize          = 10;
Int32T  sNosLogLocation      = LOG_FILE;
Int32T  sNosLogLevel         = LOG_DEBUG;

/* register component control process (SHOULD SET) */
void cmInitProcess(void);
void cmTermProcess(void);
void cmRestartProcess(void);
void cmHoldProcess(void);
NosInitProcessT     sNosInitProcess    = cmInitProcess;
NosTermProcessT     sNosTermProcess    = cmTermProcess;
NosRestartProcessT  sNosRestartProcess = cmRestartProcess;
NosHoldProcessT     sNosHoldProcess    = cmHoldProcess;

/* register message(Ipc/Event/Signal) process (SHOULD SET) */
void cmIpcProcess(Int32T msgId, void * data, Uint32T dataLen);
void cmEventProcess(Int32T msgId, void * data, Uint32T dataLen);
void cmSignalProcess(Int32T sig);
NosIpcProcessT    sNosIpcProcess    = cmIpcProcess;
NosEventProcessT  sNosEventProcess  = cmEventProcess;
NosSignalProcessT sNosSignalProcess = cmSignalProcess;

void cmCmshProcess(Int32T sockId, void *message, Uint32T size);
void cmCompProcess(Int32T sockId, void *message, Uint32T size);
NosCmIpcProcessT sNosCmshIpcProcess = cmCmshProcess;  /** Channel Process CMSH  */
NosCmIpcProcessT sNosCompIpcProcess = cmCompProcess;  /** Channel Process COMP  */

/* nos event register & receive process (SHOULD SET) */
Int32T sNosComponentEventNum = 4;
struct NosEventRegister
{
	Int32T eventId;
	Int32T priority;
} sNosEventRegister[] __attribute__ ((unused)) = 
{
	{EVENT_INTERFACE_ADD,     EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_DELETE,  EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_UP,      EVENT_PRI_MIDDLE},
	{EVENT_INTERFACE_DOWN,    EVENT_PRI_MIDDLE}
};

#endif   /* _compConfig */
