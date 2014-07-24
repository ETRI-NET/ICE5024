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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "nosLib.h"
#include "pif.h"
#include "pifDataTypes.h"

#define IIDSTRSIZE	100

static char sIidString[IIDSTRSIZE];

StringT getIidStr(Int32T id)
{
	memset(sIidString, 0, IIDSTRSIZE);
	InterfaceIdT iid;
	iid.idx = id;
	sprintf(sIidString, "%d/%d/%d/%d-%s/%d", iid.Shelf, iid.Slot, iid.Port, 
						iid.Type, getIfTypeStr(iid.Type), iid.Id);

	return sIidString;
}

Int32T getIid(Int32T shelf, Int32T slot, Int32T port, Int32T type, Int32T id)
{
	InterfaceIdT iid;
	iid.Shelf = shelf;
	iid.Slot = slot;
	iid.Port = port;
	iid.Type = type;
	iid.Id = id;

	pifLOG(LOG_DEBUG, "IID :: Shelf(%d), Slot(%d), Port(%d), Type(%d), Id(%d) \n", 
			shelf, slot, port, type, id);
	return iid.idx;
}

Int32T getIidType(Int32T id)
{
	InterfaceIdT iid;
	iid.idx = id;
	return iid.Type;
}

Int32T getIidLogicalId(Int32T id)
{
	InterfaceIdT iid;
	iid.idx = id;
	return iid.Id;
}

Int32T getIidPhysicalId(Int32T id)
{
	InterfaceIdT iid;
	iid.idx = id;
	iid.Id = 0;
	return iid.idx;
}


#define PIF_IF_NAME_LEN 11

#define PARSE_STOP 	    0
#define PARSE_ID 	    1
#define PARSE_HW 	    2
#define PARSE_PORT 	    3

#define PARSE_HW_ATTR   3

static const struct
{
	Int8T  name[IF_NAME_SIZE];
	Int32T type;
	Int32T action;
	Int32T value;
} pifIfNamePaser[PIF_IF_NAME_LEN] __attribute__ ((unused)) =
{
	{"fastethernet", TYPE_ETHERNET,  PARSE_HW,   0},
	{"eth",          TYPE_ETHERNET,  PARSE_PORT, 0},
	{"lo",           TYPE_LOOPBACK,  PARSE_STOP, 0},
	{"sit",          TYPE_IPV6IP,    PARSE_ID,   0},
	{"fftun",        TYPE_IPIP,      PARSE_STOP, 0},
	{"xgtun",        TYPE_IPIP,      PARSE_STOP, 1},
	{"vlan0.",       TYPE_VLAN,      PARSE_ID,   0},
	{"vlan",         TYPE_VLAN,      PARSE_ID,   0},
	{"port-channel", TYPE_AGGREGATE, PARSE_PORT, 0},
	{"po", 	         TYPE_AGGREGATE, PARSE_PORT, 0},
	{"null",         TYPE_MANAGE,    PARSE_ID,   0}
};


Int32T getIidbyName(StringT name)
{
	InterfaceIdT iid;

	Int32T i, type, action, value;
	Int32T shelf, slot, port, id;

	char nameStr[PIF_IF_NAME_LEN]; 
	char *strStr, *str, *tr;

	memcpy(nameStr, name, PIF_IF_NAME_LEN);

	/* find interface name type */
	type = 0;
	for(i=0; i<PIF_IF_NAME_LEN; i++){
		strStr = strstr(nameStr, pifIfNamePaser[i].name);
		if(strStr != NULL) {
			type   = pifIfNamePaser[i].type;
			action = pifIfNamePaser[i].action;
			value  = pifIfNamePaser[i].value;
			str = nameStr + strlen(pifIfNamePaser[i].name);
			break;
		}
	}//for

	if(type == 0) {
		pifLOG(LOG_DEBUG, "Invalid if name (%s) \n", name);
		return -1;
	}

	/* skip space character */
	for(i=0; i<PIF_IF_NAME_LEN; i++){
		if(str[0] == ' ') str = str+1;
		else break;
	}

	if(action == PARSE_STOP) {
		/* parse nothing (ex. fftun) */
		id = value;
		shelf = slot = port = 0;
	}
	else if(action == PARSE_ID) {
		/* parse id (ex. vlan 0)*/
		/* parse id (ex. vlan0)*/
		/* parse id (ex. vlan0.1)*/
		/* parse id (ex. vlan0.10)*/
		pifLOG(LOG_DEBUG, "parse-id (%s) \n", str);
		shelf = slot = port = 0;
		id = atoi(str);
	}
	else if(action == PARSE_PORT) {
		/* parse id (ex. eth0, eth1) */
		pifLOG(LOG_DEBUG, "parse-port (%s) \n", str);
		shelf = slot = id = 0;
		port = atoi(str);
	}
	else if(action == PARSE_HW) {
		/* parse hw 0/1.0 */
		id = 0;
		/* slot number */
		tr = strchr(str, '/');
		if(tr == NULL) {
			pifLOG(LOG_DEBUG, "Invalid if name (%s) \n", name);
			return -1;
		}
		tr[0] = '\0';
		tr = tr+1;
		slot = atoi(str);
		
		/* port number */
		str = tr;
		tr = strchr(str, '.');
		if(tr == NULL) {
			port = atoi(str);
		}
		else {
			/* id number */
			tr[0] = '\0';
			tr = tr+1;
			port = atoi(str);
			id = atoi(tr);
		}
	}

	iid.Shelf = shelf;
	iid.Slot = slot;
	iid.Port = port;
	iid.Type = type;
	iid.Id = id;

	pifLOG(LOG_DEBUG, "IID(%s-->%08x)::Shelf(%d),Slot(%d),Port(%d),Type(%d),Id(%d) \n", 
		   name, iid.idx, shelf, slot, port, type, id);
	return iid.idx;
}







