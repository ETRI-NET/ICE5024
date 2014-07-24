#include <stdio.h>
#include "nnTypes.h"
#include "nosLib.h"

#include "nnCmdDefines.h"
#include "nnCompProcess.h"

#include "dluTesterMain.h"

extern Int32T dluTesterWriteConfCB (struct cmsh *cmsh, Int32T uargc1, Int8T **uargv1,Int32T uargc2, Int8T **uargv2,Int32T uargc3, Int8T **uargv3,Int32T uargc4, Int8T **uargv4, Int32T uargc5, Int8T **uargv5, Int32T cargc, Int8T **cargv);


/*
 * External Definitions for Global Data Structure
 */
extern void ** gCompData;

dluTestT *gDluTestBase;

void 
dluTesterInitProcess(void) //initialize component
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);

  /** Component Global Variable Memory Allocate */
  gDluTestBase = NNMALLOC(MEM_GLOBAL, sizeof(dluTestT));

  /** Component Command Init **/
  gDluTestBase->gCmshGlobal 
   = compCmdInit(IPC_DLU_TESTER, dluTesterWriteConfCB);

  /* Assign shared memory pointer to global memory pointer. */
  (*gCompData) = (void *)gDluTestBase;
}


void 
dluTesterTermProcess(void) // terminate component
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);

  /** Component Command Close */
  compCmdFree(gDluTestBase->gCmshGlobal);
 
  /** Component Global Variable Memory Free */
  NNFREE(MEM_GLOBAL, gDluTestBase);

  /** System Library Close */
  taskClose();
  nnLogClose();
  memClose();
}


void dluTesterHoldProcess(void)  // hold component
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);  

  /** TODO : Add more here **/


}

extern struct polMgr *gPolIpcBase;
void dluTesterRestartProcess(void) // restart component
{
  fprintf(stdout, "enter %s\n", __FUNCTION__);

  /* Re-assign shared memory pointer to global memory pointer. */
  gDluTestBase = (dluTestT *)(*gCompData);

  /** Component Command Restart **/
  compCmdUpdate(gDluTestBase->gCmshGlobal, dluTesterWriteConfCB);

}

void dluTesterSignalProcess(Int32T sigType)  // signal catch proces
{
  NNLOG(LOG_DEBUG, "PROCESS :: %s\n", __func__);
  NNLOG(LOG_DEBUG, "Signal(%d) Occured !! \n", sigType);

  dluTesterTermProcess();
}

