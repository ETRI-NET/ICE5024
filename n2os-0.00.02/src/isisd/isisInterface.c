/*
 * isisInterface.c
 *
 *  Created on: 2014. 4. 23.
 *      Author: root
 */

#include "if.h"
#include "vty.h"

#include "nnIf.h"
#include "nnCmdDefines.h"
#include "nnCompProcess.h"
#include "nnCmdCmsh.h"

#include "isisInterface.h"
#include "isisd.h"
#include "isis_csm.h"
#include "isis_circuit.h"
#include "isisUtil.h"

Int32T
isisInterfaceDown (nnBufferT *msgBuff)
{
	struct interface *ifp;
	struct isis_circuit *circuit;

	ifp = pifInterfaceStateRead(msgBuff);

	if (ifp == NULL)
	{
		return 0;
	}

	circuit = isis_csm_state_change(IF_DOWN_FROM_Z, circuit_scan_by_ifp(ifp),
			ifp);
	if (circuit)
		SET_FLAG(circuit->flags, ISIS_CIRCUIT_FLAPPED_AFTER_SPF);

	return 0;

}

Int32T
isisInterfaceUp (nnBufferT *msgBuff)
{
	struct interface *ifp;

	ifp = pifInterfaceStateRead (msgBuff);

	if (ifp == NULL)
		return 0;

	isis_csm_state_change(IF_UP_FROM_Z, circuit_scan_by_ifp(ifp), ifp);

	return 0;
}

Int32T
isisInterfaceAdd (nnBufferT *msgBuff)
{
	struct interface *ifp;

	ifp = pifInterfaceAddRead (msgBuff);

//	if (isis->debugs & DEBUG_ZEBRA)
		zlog_debug("Zebra I/F add: %s index %d flags %ld metric %d mtu %d",
				ifp->name, ifp->ifindex, (long) ifp->flags, ifp->metric,
				ifp->mtu);

	if (if_is_operative(ifp))
		isis_csm_state_change(IF_UP_FROM_Z, circuit_scan_by_ifp(ifp), ifp);

	return 0;
}

Int32T
isisInterfaceDelete (nnBufferT *msgBuff)
{
	struct interface *ifp;

	ifp = pifInterfaceAddRead (msgBuff);

	if (!ifp)
		return 0;

	if (if_is_operative(ifp))
		zlog_warn("Zebra: got delete of %s, but interface is still up",
				ifp->name);

	//  if (isis->debugs & DEBUG_ZEBRA)
	zlog_debug("Zebra I/F delete: %s index %d flags %ld metric %d mtu %d",
			ifp->name, ifp->ifindex, (long) ifp->flags, ifp->metric, ifp->mtu);

	isis_csm_state_change(IF_DOWN_FROM_Z, circuit_scan_by_ifp(ifp), ifp);

	/* Cannot call if_delete because we should retain the pseudo interface
	 in case there is configuration info attached to it. */
	if_delete_retain(ifp);

	ifp->ifindex = IFINDEX_INTERNAL;

	return 0;
}

Int32T
isisInterfaceAddressAdd (nnBufferT *msgBuff)
{
	struct connected *pIfc;

	struct prefix *p;
	char buf[BUFSIZ];

	pIfc = pifInterfaceAddressRead (EVENT_INTERFACE_ADDRESS_ADD,  msgBuff);

	if (pIfc == NULL)
		return 0;

	p = pIfc->address;

	prefix2str(p, buf, BUFSIZ);
#ifdef EXTREME_DEBUG
	if (p->family == AF_INET)
	zlog_debug ("connected IP address %s", buf);
#ifdef HAVE_IPV6
	if (p->family == AF_INET6)
	zlog_debug ("connected IPv6 address %s", buf);
#endif /* HAVE_IPV6 */
#endif /* EXTREME_DEBUG */
	if (if_is_operative(pIfc->ifp))
		isis_circuit_add_addr(circuit_scan_by_ifp(pIfc->ifp), pIfc);


	return 0;
}

Int32T
isisInterfaceAddressDelete (nnBufferT *msgBuff)
{
	struct connected *pIfc;
	  struct interface *ifp;
	#ifdef EXTREME_DEBUG
	  struct prefix *p;
	  u_char buf[BUFSIZ];
	#endif /* EXTREME_DEBUG */

	  pIfc = pifInterfaceAddressRead (EVENT_INTERFACE_ADDRESS_DELETE, msgBuff);

	  if (pIfc == NULL)
	    return 0;

	  ifp = pIfc->ifp;

	#ifdef EXTREME_DEBUG
	  p = c->address;
	  prefix2str (p, buf, BUFSIZ);

	  if (p->family == AF_INET)
	    zlog_debug ("disconnected IP address %s", buf);
	#ifdef HAVE_IPV6
	  if (p->family == AF_INET6)
	    zlog_debug ("disconnected IPv6 address %s", buf);
	#endif /* HAVE_IPV6 */
	#endif /* EXTREME_DEBUG */

	  if (if_is_operative (ifp))
	    isis_circuit_del_addr (circuit_scan_by_ifp (ifp), pIfc);
	  connected_free (pIfc);

	return 0;
}




//Int32T
//configWriteIsisNetwork(struct cmsh *, Int32T)
//{
//
//}

