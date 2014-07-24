#if !defined(_pifInterface_h)
#define _pifInterface_h
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
#include "nosLib.h"

typedef enum {
    PIF_IF_CMI_SUCCESS = 0,
    PIF_IF_CMI_NOT_EXIST,
    PIF_IF_CMI_INVALID_NAME,
    PIF_IF_CMI_ERROR_UNKNOWN
} PifCmiIfResultT;

typedef struct
{
	Int8T  ifName[32];
	Int32T version;
	Int32T totalCount;
} PifCmiInterfaceT;

void*  pifInterfaceInit();
void  pifInterfaceRestart();
Int32T pifInterfaceConfig(StringT ifName);
Int32T pifInterfaceConfigLU(StringT ifName);

Int32T pifInterfaceGetCount();
void pifInterfaceStr(StringT str, Int32T rlt);
PifCmiInterfaceT* pifInterfaceConfigGetLU(StringT ifName);

/* Pif Timer Interface */
typedef struct
{
    void * timerEvt1;
    void * timerEvt2;

} PifTimerInterfaceT;

void * pifTimerInterfaceInit();
void pifTimerInterfaceRestart();
void pifTimerProcess3Sec(Int32T fd, Int16T event, void * arg);
void pifTimerProcess5Sec(Int32T fd, Int16T event, void * arg);
PifTimerInterfaceT* pifTimerInterfaceConfigGetLU(StringT ifName);


/* Pif Fd Interface */
typedef struct
{
    void * actEvt;
    void * readEvt;

} PifFdInterfaceT;

void * pifFdInterfaceInit();
void pifFdInterfaceRestart();
void pifFdAcceptProcess(Int32T fd, Int16T event, void * arg);
void pifFdRead(Int32T fd, Int16T event, void * arg);
PifFdInterfaceT* pifFdInterfaceConfigGetLU(StringT ifName);


#endif   /* _pifInterface_h */

