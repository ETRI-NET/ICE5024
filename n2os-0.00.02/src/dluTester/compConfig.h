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
Int32T  sNosCompName         = DLU_TESTER;
StringT sNosCompModuleVer    = "0.0.1";
StringT sNosLogFile          = "dluTester";
Int32T  sNosLogSize          = 10;
Int32T  sNosLogLocation      = LOG_FILE;
Int32T  sNosLogLevel         = LOG_DEBUG;

/* register component control process (SHOULD SET) */
void dluTesterInitProcess(void);
void dluTesterTermProcess(void);
void dluTesterRestartProcess(void);
void dluTesterHoldProcess(void);
NosInitProcessT     sNosInitProcess    = dluTesterInitProcess;
NosTermProcessT     sNosTermProcess    = dluTesterTermProcess;
NosRestartProcessT  sNosRestartProcess = dluTesterRestartProcess;
NosHoldProcessT     sNosHoldProcess    = dluTesterHoldProcess;

/* register message(Ipc/Event/Signal) process (SHOULD SET) */
void dluTesterIpcProcess(Int32T msgId, void * data, Uint32T dataLen);
void dluTesterEventProcess(Int32T msgId, void * data, Uint32T dataLen);
void dluTesterSignalProcess(Int32T sig);
NosIpcProcessT    sNosIpcProcess    = dluTesterIpcProcess;
NosEventProcessT  sNosEventProcess  = dluTesterEventProcess;
NosSignalProcessT sNosSignalProcess = dluTesterSignalProcess;

void dluTesterCmshIpcProcess(Int32T sockId, void *message, Uint32T size);
void dluTesterCmIpcProcess(Int32T sockId, void *message, Uint32T size);
NosCmIpcProcessT sNosCmshIpcProcess = dluTesterCmshIpcProcess;  /** Channel Process CMSH  */
NosCmIpcProcessT sNosCompIpcProcess = dluTesterCmIpcProcess;  /** Channel Process COMP  */

/* nos event register & receive process (SHOULD SET) */
Int32T sNosComponentEventNum = 0;
struct NosEventRegister
{
	Int32T eventId;
	Int32T priority;
} sNosEventRegister[] __attribute__ ((unused)) = 
{
};

#endif   /* _compConfig */
