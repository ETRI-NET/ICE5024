#include <stdio.h>
#include "nnTypes.h"
#include "nosLib.h"

#include "nnCmdDefines.h"
#include "nnCompProcess.h"

#include "ipcmgrInit.h"

/*
 * External Definitions for Global Data Structure
 */
extern void ** gCompData;


/*
 * Ipc manager's data structure using in shared code.
 */

IpcmgrT * pIpcmgr = NULL;

extern Int32T ipcmgrWriteConfCB
 (struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1, Int32T uargc2, Int8T **uargv2,
  Int32T uargc3, Int8T **uargv3, Int32T uargc4, Int8T **uargv4,
  Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);

void 
ipcmgrInitProcess(void) //initialize component
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);

  /** Component Global Variable Memory Allocate */
  pIpcmgr = NNMALLOC(MEM_GLOBAL, sizeof(IpcmgrT));

  pIpcmgr->pCmshGlobal = NULL;

  /** Component Command Init **/

  /** TODO : Add more here **/
  eventSubInfoInit();


  /* Assign shared memory pointer to global memory pointer. */
  (*gCompData) = (void *)pIpcmgr;
}


void 
ipcmgrTermProcess(void) // terminate component
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);

  /** Component Command Close */
  compCmdFree(pIpcmgr->pCmshGlobal);
 
  /** TODO : Add more here **/

  eventSubInfoClose();


  /** Component Global Variable Memory Free */
  NNFREE(MEM_GLOBAL, pIpcmgr);

  /** System Library Close */
//  taskClose();
  nnLogClose();
  memClose();
}


void ipcmgrHoldProcess(void)  // hold component
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);  

  /** TODO : Add more here **/


}

void ipcmgrRestartProcess(void) // restart component
{
  fprintf(stdout, "enter %s\n", __FUNCTION__);

  /* Re-assign shared memory pointer to global memory pointer. */
  pIpcmgr = (IpcmgrT *)(*gCompData);

  /** Component Command Restart **/
  compCmdUpdate(pIpcmgr->pCmshGlobal, ipcmgrWriteConfCB);

  
  /** TODO : Add more here **/

}

void ipcmgrSignalProcess(Int32T sigType)  // signal catch proces
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);
  NNLOG(LOG_DEBUG, "Signal(%d) Occured !! \n", sigType);

  /** TODO : Add more here **/

  ipcmgrTermProcess();
}

