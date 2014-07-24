/*
 * ospfUtil.c
 *
 *  Created on: 2014. 4. 22.
 *      Author: root
 */

#include "stream.h"
#include "if.h"
#include "vty.h"

#include "nnIf.h"
#include "nnPrefix.h"
#include "nnLog.h"
#include "nosLib.h"

#include "ospfUtil.h"
#include "ospfd.h"
#include "ospf_asbr.h"
#include "ospf_lsa.h"

void
interfaceIfSetValue (nnBufferT *msgBuff , struct interface *pIf)
{
	/* Interface index */
	pIf->ifindex = nnBufferGetInt32T(msgBuff);

	/* Status */
	pIf->status = nnBufferGetInt8T(msgBuff);

	/* Flags */
	pIf->flags = nnBufferGetInt64T(msgBuff);

	/* Metric */
	pIf->metric = nnBufferGetInt32T(msgBuff);

	/* MTU */
	pIf->mtu = nnBufferGetInt32T(msgBuff);

	/* MTU6 */
	pIf->mtu6 = nnBufferGetInt32T(msgBuff);

	/* Bandwidth */
	pIf->bandwidth = nnBufferGetInt32T(msgBuff);

	/* HW Type. */
	pIf->hw_type = nnBufferGetInt16T(msgBuff);

	/* HW Address Length. */
	pIf->hw_addr_len = nnBufferGetInt32T(msgBuff);

	if (pIf->hw_addr_len > 0) {
		/* HW Address Length. */
		nnBufferGetString(msgBuff, (char *) pIf->hw_addr, pIf->hw_addr_len);

		printf("+++++++++++>> hwType[%d] hwAddrLen[%d]\n", pIf->hw_type,
				pIf->hw_addr_len);
		printf("+++++++++++>> hwAddress : ");
		int i;
		for (i = 0; i < pIf->hw_addr_len; i++) {
			printf("%02x:", pIf->hw_addr[i]);
		}
		printf("\n");
	}
}

struct interface *
pifInterfaceStateRead(nnBufferT * msgBuff) {
	struct interface *pIf;

	char ifName[INTERFACE_NAMSIZ];

	/* Name Length, String */
	Uint8T nameLength = nnBufferGetInt8T(msgBuff);
	nnBufferGetString(msgBuff, ifName, nameLength);

	/* Lookup this by interface index. */
//	  pIf = ifLookupByNameLen (ifName,
//	                               strnlen(ifName, INTERFACE_NAMSIZ));
	pIf = if_lookup_by_name_len(ifName, strnlen(ifName, INTERFACE_NAMSIZ));

	/* Check interface exist */
	if (!pIf)
		return NULL;

	interfaceIfSetValue(msgBuff, pIf);

	return pIf;
}

struct interface *
pifInterfaceAddRead (nnBufferT * msgBuff)
{
	struct interface * pIf = NULL;
	char ifName[INTERFACE_NAMSIZ] = { };

	/* Name Length, String */
	Uint8T nameLength = nnBufferGetInt8T(msgBuff);
	nnBufferGetString(msgBuff, ifName, nameLength);

	/* Lookup/create interface by name. */
//	pIf = ifGetByNameLen(ifName, strnlen(ifName, INTERFACE_NAMSIZ));
	pIf = if_get_by_name_len(ifName, strnlen(ifName, INTERFACE_NAMSIZ));

	interfaceIfSetValue(msgBuff, pIf);

	return pIf;

}

static Int32T
memConstant(const void *s, Int32T c, size_t n)
{
  const Uint8T *p = s;

  while (n-- > 0)
    if (*p++ != c)
      return 0;
  return 1;
}

struct connected *
pifInterfaceAddressRead (Int32T type, nnBufferT * msgBuff)
{
  Uint32T ifIndex = 0, plen = 0;
  Uint8T ifcFlags = 0;
  struct interface * pIf = NULL;
  struct connected * pIfc = NULL;
  struct prefix p, d;

  memset (&p, 0, sizeof(p));
  memset (&d, 0, sizeof(d));

  /* Get interface index. */
  ifIndex = nnBufferGetInt32T(msgBuff);

  nnBufferPrint(msgBuff);

  /* Lookup index. */
//  pIf = ifLookupByIndex (ifIndex);
  pIf = if_lookup_by_index (ifIndex);
  if (pIf == NULL)
  {
    NNLOG (LOG_DEBUG, "pifInterfaceAddressRead(%s): "
                      "Can't find interface by ifindex: %d ",
                 (type == EVENT_INTERFACE_ADDRESS_ADD? "ADD" : "DELETE"),
                 ifIndex);
    return NULL;
  }

  /* Fetch flag. */
  ifcFlags = nnBufferGetInt8T(msgBuff);

  /* Fetch interface address. */
  p.family = nnBufferGetInt8T(msgBuff);
  if (p.family == AF_INET)
  {
    /* Prefix Address, Length */
    p.u.prefix4 = nnBufferGetInaddr (msgBuff);
    p.prefixlen = nnBufferGetInt8T (msgBuff);

    /* Destination */
    d.u.prefix4 = nnBufferGetInaddr(msgBuff);
  }
#ifdef HAVE_IPV6
  else if (p.family == AF_INET6)
  {
    /* Prefix Address, Length */
//    p.u.prefix6 = nnBufferGetIn6addr (msgBuff);
    p.prefixLen = nnBufferGetInt8T (msgBuff);

    /* Destination */
//    d.u.prefix6 = nnBufferGetIn6addr(msgBuff);
  }
#endif

  if (type == EVENT_INTERFACE_ADDRESS_ADD)
  {
    /* N.B. NULL destination pointers are encoded as all zeroes */
//    pIfc = connectedAddByPrefix(pIf, &p,(memConstant(&d.u.prefix,0,plen) ?
//                          NULL : &d));
	  pIfc = connected_add_by_prefix(pIf, &p,(memConstant(&d.u.prefix,0,plen) ?
		      NULL : &d));
    if (pIfc != NULL)
    {
      pIfc->flags = ifcFlags;
      if (pIfc->destination)
      {
        pIfc->destination->prefixlen = pIfc->address->prefixlen;
      }
    }
  }
  else
  {
    assert (type == EVENT_INTERFACE_ADDRESS_DELETE);
//    pIfc = connectedDeleteByPrefix(pIf, &p);
    pIfc = connected_delete_by_prefix(pIf, &p);
  }

  return pIfc;
}

struct timeval
ospfGetUpdateTime(long sec, long usec)
{
	struct timeval tv;

	tv.tv_sec = ospfRecentRelativeTime().tv_sec + sec;
	tv.tv_usec = ospfRecentRelativeTime().tv_usec + usec * 1000;

	return tv_adjust(tv);
}

struct timeval
ospfRecentRelativeTime (void)
{
	struct timespec tspec;
	struct timeval tv;

	clock_gettime(CLOCK_MONOTONIC, &tspec);

	tv.tv_sec = tspec.tv_sec;
	tv.tv_usec = tspec.tv_nsec / 1000;

	return tv;

}


