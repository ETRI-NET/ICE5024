/*
 * IS-IS Rout(e)ing protocol - isis_main.c
 *
 * Copyright (C) 2001,2002   Sampo Saaristo
 *                           Tampere University of Technology      
 *                           Institute of Communications Engineering
 *
 * This program is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU General Public Licenseas published by the Free 
 * Software Foundation; either version 2 of the License, or (at your option) 
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
 * more details.

 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#include "dict.h"
#include "include-netbsd/iso.h"
#include "isis_constants.h"
#include "isis_common.h"
#include "isis_flags.h"
#include "isis_circuit.h"
#include "isisd.h"
#include "isis_dynhn.h"
#include "isis_spf.h"
#include "isis_route.h"
#include "isis_zebra.h"

#include "nnMemmgr.h"
#include "nnTypes.h"
#include "nnDefines.h"
#include "isisInit.h"
#include "taskManager.h"
#include "isisIpc.h"
//#include "nosLib.h"

/*
 * Main routine of isisd. Parse arguments and handle IS-IS state machine.
 */
Int32T
main (Int32T argc, char **argv)
{

	/* Initialize Memmory Manager             */
	/* RIB_MANAGER defined in nnTypes.h file. */
	if (memInit(ISIS) == FAILURE) {
		return -1;
	}

	/* Initialize Log */
	if (nnLogInit(ISIS) != SUCCESS) {
		return -1;
	}

	/* Max Log FileName size : 20 */
	nnLogSetFile("isisd", 10);
	nnLogSetFlag(LOG_FILE);

	/* Signal Registration */
	taskCreate(isisInitProcess);

	/* IPC Callback Function Registration */
	ipcChannelOpen(ISIS, (void *) isisIpcProcess);

	/* Event Callback Function Registration */
	eventOpen((void *) isisEventProcess);

	/* Event Registration */
	isisEventSubscribe();

	/* Signal Registration */
	taskSignalSetAggr(isisSignalProcess);

	/* Task Dispatch */
	taskDispatch();
}
