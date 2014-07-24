#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef IS_SHARED_LIBRARY
#include "nosLib.h"
#else
#include "taskManager.h"
#include "nnLog.h"
#include "nnMemmgr.h"
#endif

#include "pifIf.h"


#ifdef IS_SHARED_LIBRARY

/* External Definitions for Global Data Structure */
extern void** gCompData;

#endif

typedef struct pifataBaseTmp 
{
	void* portData;
	void* portGroupData;
	void* vlanData;
	void* interfaceData;
    void* cmdData;
    void* timerData;
    void* fdData;
} pifDataBaseTmpT;

pifDataBaseTmpT* pifDataBase;

//initialize component
void pifInitProcess(void)
{
	NNLOG(LOG_DEBUG, "enter %s\n", __FUNCTION__);

	//initailize pif database
	pifDataBase = (pifDataBaseTmpT*)NNMALLOC(MEM_TYPE_IF, sizeof(pifDataBaseTmpT));
	memset(pifDataBase, 0, sizeof(pifDataBaseTmpT));

#ifdef IS_SHARED_LIBRARY
	//save componet database in global reference
	(*gCompData) = (void*)pifDataBase;
#endif

	//initailize each data
	pifDataBase->interfaceData = pifInterfaceInit(pifDataBase->interfaceData);
    pifDataBase->cmdData = pifCmdInit(pifDataBase->cmdData);
    pifDataBase->timerData = pifTimerInterfaceInit(pifDataBase->timerData);
    pifDataBase->fdData = pifFdInterfaceInit(pifDataBase->fdData);
}

//terminate component
void pifTermProcess(void)
{
	NNLOG(LOG_DEBUG, "enter %s\n", __FUNCTION__);

	//do something such as free database

    NNFREE(MEM_TYPE_IF, pifDataBase->interfaceData);
    NNFREE(MEM_TYPE_IF, pifDataBase->timerData);
    NNFREE(MEM_TYPE_IF, pifDataBase->fdData);
    NNFREE(MEM_TYPE_IF, pifDataBase);
}

#ifdef IS_SHARED_LIBRARY
//restart component
void pifRestartProcess(void)
{
	NNLOG(LOG_DEBUG, "enter %s\n", __FUNCTION__);

	//renewal pif database from global reference
	pifDataBase = (pifDataBaseTmpT*)(*gCompData);

	pifInterfaceRestart((void*)pifDataBase->interfaceData);
    pifTimerInterfaceRestart((void*)pifDataBase->timerData);
    pifFdInterfaceRestart((void*)pifDataBase->fdData);
}
#endif

#ifdef IS_SHARED_LIBRARY
//hold component
void pifHoldProcess(void)
{
	NNLOG(LOG_DEBUG, "enter %s\n", __FUNCTION__);

	//do something you need
}
#endif

//signal catch proces
void pifSignalProcess(Int32T sig)
{
	NNLOG(LOG_DEBUG, "enter %s\n", __FUNCTION__);
	NNLOG(LOG_ERR, "Catch siganl: %d\n", sig);

	//do something you need

    pifTermProcess();
}

