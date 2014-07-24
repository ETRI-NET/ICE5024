/*345678901234567890123456789012345678901234567890123456789012345678901234567890*/
/*********************************************************************************
 *               Electronics and Telecommunications Research Institute
 * Filename : <myFileName>
 * Blockname: <PIF Manager>
 * Overview : <PIF Manager S/W block manages Port/Interface & L2 MAC/VLAN>
 * Creator  : <Seungwoo Hong>
 * Owner    : <Seungwoo Hong>
 * Copyright: 2013 Electronics and Telecommunications Research Institute. 
 *            All rights reserved. No part of this software shall be reproduced, 
 *            stored in a retrieval system, or transmitted by any means, 
 *            electronic, mechanical, photocopying, recording, or otherwise, 
 *            without written permission from ETRI.
 *********************************************************************************/

/*********************************************************************************
 *                        VERSION CONTROL  INFORMATION
 * $Author$
 * $Date$
 * $Revision
 * $Log$ 
 *********************************************************************************/

#include <stdio.h>
#include "nosLib.h"
#include "pifIf.h"


extern StringT sNosCompModuleVer;

void pifTimerProcess(Int32T fd, Int16T event, void * arg)
{
    PifCmiInterfaceT* ifData =  pifInterfaceConfigGetLU(NULL);
	if(ifData == NULL) {
		printf("IF data NULL \n");
		return;
	}
    ifData->totalCount += 5;
    printf("IF-Name: %s, Version: %s,  Count: %d\n",
            ifData->ifName, sNosCompModuleVer, ifData->totalCount);
}

void pifTimerProcess3Sec(Int32T fd, Int16T event, void * arg)
{
	//do anything
    PifTimerInterfaceT *ifData = pifTimerInterfaceConfigGetLU(NULL);
	if(ifData == NULL) {
		printf("IF data NULL \n");
		return;
	}

    printf("%s : (version %s) \n",  __FUNCTION__, sNosCompModuleVer);
}

void pifTimerProcess5Sec(Int32T fd, Int16T event, void * arg)
{
	//do anything
    PifTimerInterfaceT *ifData = pifTimerInterfaceConfigGetLU(NULL);
	if(ifData == NULL) {
		printf("IF data NULL \n");
		return;
	}

    printf("%s : (version %s) \n",  __FUNCTION__, sNosCompModuleVer);
}

