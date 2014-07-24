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
Int32T  sNosCompName         = IPC_MANAGER;
StringT sNosCompModuleVer    = "0.0.1";
StringT sNosLogFile          = "ipcmgr";
Int32T  sNosLogSize          = 10;
Int32T  sNosLogLocation      = LOG_FILE;
Int32T  sNosLogLevel         = LOG_DEBUG;

/* register component control process (SHOULD SET) */
void ipcmgrInitProcess(void);
void ipcmgrTermProcess(void);
void ipcmgrRestartProcess(void);
void ipcmgrHoldProcess(void);
NosInitProcessT     sNosInitProcess    = ipcmgrInitProcess;
NosTermProcessT     sNosTermProcess    = ipcmgrTermProcess;
NosRestartProcessT  sNosRestartProcess = ipcmgrRestartProcess;
NosHoldProcessT     sNosHoldProcess    = ipcmgrHoldProcess;

/* register message(Ipc/Event/Signal) process (SHOULD SET) */
void ipcmgrIpcProcess(Int32T msgId, void * data, Uint32T dataLen);
void ipcmgrEventProcess(Int32T msgId, void * data, Uint32T dataLen);
void ipcmgrSignalProcess(Int32T sig);
NosIpcProcessT    sNosIpcProcess    = ipcmgrIpcProcess;
NosEventProcessT  sNosEventProcess  = ipcmgrEventProcess;
NosSignalProcessT sNosSignalProcess = ipcmgrSignalProcess;

void ipcmgrCmshIpcProcess(Int32T sockId, void *message, Uint32T size);
void ipcmgrCmIpcProcess(Int32T sockId, void *message, Uint32T size);
NosCmIpcProcessT sNosCmshIpcProcess = ipcmgrCmshIpcProcess;  /** Channel Process CMSH  */
NosCmIpcProcessT sNosCompIpcProcess = ipcmgrCmIpcProcess;  /** Channel Process COMP  */

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
