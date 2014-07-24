/*
 * ospfRibmgr.c
 *
 *  Created on: 2014. 6. 10.
 *      Author: root
 */

#include "prefix.h"
#include "vty.h"

#include "nnTypes.h"
#include "nnStr.h"
#include "nnPrefix.h"
#include "nnBuffer.h"
#include "nnRibDefines.h"
#include "nosLib.h"
#include "nnUtility.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"

#include "ospfd.h"
#include "ospfRibmgr.h"
#include "ospf_asbr.h"
#include "ospf_lsa.h"
#include "ospf_zebra.h"

/*
 * Description : 루트 삭제 메시지 버퍼를 반드는 함수.
 *
 * param [in] apiBuff : 메시지 버퍼
 * param [in] rCount : 루트 개수
 * param [in] rType : 루트 타입
 * param [in] rApi : 각 입력변수를 담는 데이터 스트럭처
 *
 * @retval : 메시지 버퍼 길이
 */
void
ospfRibmgrInit ()
{
  /* Buffer Reset */
  nnBufferT msgBuff;
  nnBufferReset(&msgBuff);

  /* Component ID. Defined in nnDefines.h. */
  nnBufferSetInt8T(&msgBuff, OSPF);

  /* Routing component type. Defined in nnRibDefines.h. */
  nnBufferSetInt8T(&msgBuff, RIB_ROUTE_TYPE_OSPF);

  /* Send ipc message to ribmgr */
  ipcSendAsync(RIB_MANAGER,
               IPC_RIB_CLIENT_INIT, msgBuff.data, msgBuff.length);
}

void
ospfRibmgrClose ()
{
	  nnBufferT sendBuff;
	  nnBufferT recvBuff;

	  /* Buffer Reset */
	  nnBufferReset(&sendBuff);
	  nnBufferReset(&recvBuff);

	  /* Component ID. Defined in nnDefines.h. */
	  nnBufferSetInt8T(&sendBuff, OSPF);

	  /* Routing component type. Defined in nnRibDefines.h. */
	  nnBufferSetInt8T(&sendBuff, RIB_ROUTE_TYPE_OSPF);

	  NNLOG (LOG_DEBUG,
	         "%s called. send IPC_RIB_CLIENT_CLOSE to ribmgr\n", __func__);
	  /* Send ipc message to ribmgr */
	  ipcSendSync(RIB_MANAGER,
	              IPC_RIB_CLIENT_CLOSE, IPC_RIB_CLIENT_CLOSE,
	              sendBuff.data, sendBuff.length, recvBuff.data, &recvBuff.length);

	  ipcProcPendingMsg ();

}

Int32T
ospfRibmgrReadIpv4 (Int32T command,  nnBufferT *msgBuff, Uint16T length)
{

	Int8T type = 0;
	Int8T flags = 0;
	Int8T messages = 0;
	Prefix4T prefix = { 0, };
	Int8T nhCnt = 0;
	struct in_addr nextHop = { 0, };
	Int8T ifCnt = 0;
	Uint32T ifIndex = 0;
	Int8T distance = 0;
	Uint32T metric = 0;
	struct ospf *ospf;

	struct external_info *ei;
	struct prefix_ipv4 pi4;

	/* Type, flags, message. */
	type = nnBufferGetInt8T(msgBuff);
	flags = nnBufferGetInt8T(msgBuff);
	messages = nnBufferGetInt8T(msgBuff);

	/* IPv4 prefix. */
	memset(&prefix, 0, sizeof(Prefix4T));
	prefix.family = AF_INET;
	prefix.prefixLen = nnBufferGetInt8T(msgBuff);
	prefix.prefix = nnBufferGetInaddr(msgBuff);

	if (IPV4_NET127(ntohl(prefix.prefix.s_addr)))
		return 0;

	/* Nexthop, ifindex, distance, metric. */
	if (CHECK_FLAG(messages, RIB_MESSAGE_NEXTHOP)) {
		nhCnt = nnBufferGetInt8T(msgBuff);
		nextHop = nnBufferGetInaddr(msgBuff);
	}
	if (CHECK_FLAG(messages, RIB_MESSAGE_IFINDEX)) {
		ifCnt = nnBufferGetInt8T(msgBuff);
		ifIndex = nnBufferGetInt32T(msgBuff);
	}
	/* Get distance. */
	if (CHECK_FLAG(messages, RIB_MESSAGE_DISTANCE)) {
		distance = nnBufferGetInt8T(msgBuff);
	}
	/* Get metric. */
	if (CHECK_FLAG(messages, RIB_MESSAGE_METRIC)) {
		metric = nnBufferGetInt32T(msgBuff);
	} else {
		metric = 0;
	}

//	  if (IS_RIP_DEBUG_RIBMGR)
	{
		NNLOG(LOG_DEBUG, "%s\n", __func__);
		if (command == IPC_RIB_IPV4_ROUTE_ADD)
			NNLOG(LOG_DEBUG, "IPC_RIB_IPV4_ROUTE_ADD\n");
		else
			NNLOG(LOG_DEBUG, "IPC_RIB_IPV4_ROUTE_DELETE\n");
		NNLOG(LOG_DEBUG, "\t prefix/length=%s/%d\n", inet_ntoa(prefix.prefix),
				prefix.prefixLen);
		NNLOG(LOG_DEBUG, "\t nexthop count=%d, address=%s\n", nhCnt,
				inet_ntoa(nextHop));
		NNLOG(LOG_DEBUG, "\t intf count = %d, intf index = %d\n", ifCnt,
				ifIndex);
		NNLOG(LOG_DEBUG, "\t distance = %d, metric = %d\n", distance, metric);
		NNLOG(LOG_DEBUG, "\t type = %d, flags = %d, messages = %d\n", type,
				flags, messages);
	}

	ospf = ospf_lookup();
	if (ospf == NULL)
		return 0;

	pi4.family = prefix.family;
	pi4.prefix = prefix.prefix;
	pi4.prefixlen = prefix.prefixLen;

	/* Then fetch IPv4 prefixes. */
	if (command == IPC_RIB_IPV4_ROUTE_ADD) {
		/* XXX|HACK|TODO|FIXME:
		 * Maybe we should ignore reject/blackhole routes? Testing shows that
		 * there is no problems though and this is only way to "summarize"
		 * routes in ASBR at the moment. Maybe we need just a better generalised
		 * solution for these types?
		 *
		 * if ( CHECK_FLAG (api.flags, ZEBRA_FLAG_BLACKHOLE)
		 *     || CHECK_FLAG (api.flags, ZEBRA_FLAG_REJECT))
		 * return 0;
		 */
		ei = ospf_external_info_add(type, pi4, ifIndex, nextHop);

		if (ospf->router_id.s_addr == 0)
			/* Set flags to generate AS-external-LSA originate event
			 for each redistributed protocols later. */
			ospf->external_origin |= (1 << type);
		else {
			if (ei) {
				if (is_prefix_default(&pi4))
					ospf_external_lsa_refresh_default(ospf);
				else {
					struct ospf_lsa *current;

					current = ospf_external_info_find_lsa(ospf, &ei->p);
					if (!current)
						ospf_external_lsa_originate(ospf, ei);
					else if (IS_LSA_MAXAGE(current))
						ospf_external_lsa_refresh(ospf, current, ei,
								LSA_REFRESH_FORCE);
					else
						zlog_warn("ospf_zebra_read_ipv4() : %s already exists",
								inet_ntoa(pi4.prefix));
				}
			}
		}
	}
	else                          /* if (command == ZEBRA_IPV4_ROUTE_DELETE) */
	    {
	      ospf_external_info_delete (type, pi4);
	      if (is_prefix_default (&pi4))
	        ospf_external_lsa_refresh_default (ospf);
	      else
	        ospf_external_lsa_flush (ospf, type, &pi4, ifIndex /*, nexthop */);
	    }

	return 0;

}

/* Redistribution types */
static struct {
  Int32T type;
  Int32T str_min_len;
  const char *str;
} redist_type[] = {
  {RIB_ROUTE_TYPE_KERNEL,  1, "kernel"},
  {RIB_ROUTE_TYPE_CONNECT, 1, "connected"},
  {RIB_ROUTE_TYPE_STATIC,  1, "static"},
  {RIB_ROUTE_TYPE_RIP,     1, "rip"},
  {RIB_ROUTE_TYPE_ISIS,    1, "isis"},
  {RIB_ROUTE_TYPE_BGP,     1, "bgp"},
  {0, 0, NULL}
};

/* ospfd to ribmgr command interface. */
static void
ospfRibmgrRedistributeSet (Int32T cmd, Int32T routeType)
{
  /* Buffer Reset */
  nnBufferT msgBuff;
  nnBufferReset(&msgBuff);

  /* Add commponent id. */
  nnBufferSetInt8T(&msgBuff, OSPF); /* OSPF component id */

  /* Add requested route type. */
  nnBufferSetInt8T(&msgBuff, routeType);

//  if (IS_RIP_DEBUG_RIBMGR)
//  {
//    NNLOG (LOG_DEBUG, "%s\n", __func__);
//
//    if (cmd == IPC_RIB_REDISTRIBUTE_ADD)
//    {
//      NNLOG (LOG_DEBUG, "IPC_RIB_REDISTRIBUTE_ADD\n");
//    }
//    else
//    {
//      NNLOG (LOG_DEBUG, "IPC_RIB_REDISTRIBUTE_DELETE\n");
//    }
//
//    NNLOG (LOG_DEBUG, "\tcompId = %d, protoId = %d, requestRoute=%d\n",
//           RIP, RIB_ROUTE_TYPE_RIP, routeType);
//  }

  /* send ipc message to ribmgr */
  ipcSendAsync(RIB_MANAGER, cmd, msgBuff.data, msgBuff.length);
}

static void
ospfRibmgrRedistributeDefaultSet (Int32T cmd)
{
  /* Buffer Reset */
  nnBufferT msgBuff;
  nnBufferReset(&msgBuff);

  /* Add commponent id. */
  nnBufferSetInt8T(&msgBuff, OSPF); /* OSPF component id */

  /* send ipc message to ribmgr */
  ipcSendAsync(RIB_MANAGER, cmd, msgBuff.data, msgBuff.length);
}

static int
ospfRibmgrRedistribute (struct cmsh *cmsh, int cmd, int type)
{
	Int32T i = 0;

	/* Check route string. */
	for (i = 0; redist_type[i].type; i++) {
		if (redist_type[i].type == type) {
			/* Request routemap to ribmgr. */
			if (cmd == IPC_RIB_REDISTRIBUTE_ADD) {
				if (om->ospfRedistribute[redist_type[i].type]) {
					cmdPrint(cmsh, "Already set redistribute = %s\n", redist_type[i].str);
					return (CMD_IPC_OK);
				}

				/* Set flag of redistribute. */
				cmdPrint(cmsh, "Set redistribute = %s\n", redist_type[i].str);
				om->ospfRedistribute[redist_type[i].type] = 1;

				/* Send request message to ribmgr. */
				ospfRibmgrRedistributeSet(cmd, redist_type[i].type);
			} else if (cmd == IPC_RIB_REDISTRIBUTE_DELETE) {
				if (!om->ospfRedistribute[redist_type[i].type]) {
					cmdPrint(cmsh, "Already no set redistribute = %s\n",
							redist_type[i].str);
					return (CMD_IPC_OK);
				}

				/* Set flag of redistribute. */
				cmdPrint(cmsh, "no Set redistribute = %s\n", redist_type[i].str);
				om->ospfRedistribute[redist_type[i].type] = 0;

				/* Send request message to ribmgr. */
				ospfRibmgrRedistributeSet(cmd, redist_type[i].type);
			} else if (cmd == IPC_RIB_REDISTRIBUTE_CLEAR) {
				cmdPrint(cmsh, "Not implemented yet !!!\n");
			} else {
				cmdPrint(cmsh, "Wrong command = %d\n", cmd);
				return (CMD_IPC_ERROR);
			}

			return (CMD_IPC_OK);
		}
	}

	cmdPrint(cmsh, "Wrong route string = %s\n", redist_type[i].str);
	return (CMD_IPC_ERROR);
}

static int
ospfRibmgrRedistributeDefault (struct cmsh *cmsh, int cmd)
{
	if (cmd == IPC_RIB_REDISTRIBUTE_DEFAULT_ADD) {
		if (om->ospfDefaultInformation) {
			cmdPrint(cmsh, "Already set redistribute default \n");
			return (CMD_IPC_OK);
		}

		/* Set flag of redistribute. */
		cmdPrint(cmsh, "Set redistribute default \n");
		om->ospfDefaultInformation = 1;

		/* Send request message to ribmgr. */
		ospfRibmgrRedistributeDefaultSet(cmd);
	} else if (cmd == IPC_RIB_REDISTRIBUTE_DEFAULT_DELETE) {
		if (!om->ospfDefaultInformation) {
			cmdPrint(cmsh, "Already no set redistribute default\n");
			return (CMD_IPC_OK);
		}

		/* Set flag of redistribute. */
		cmdPrint(cmsh, "no Set redistribute default\n");
		om->ospfDefaultInformation = 0;

		/* Send request message to ribmgr. */
		ospfRibmgrRedistributeDefaultSet(cmd);
	} else if (cmd == IPC_RIB_REDISTRIBUTE_CLEAR) {
		cmdPrint(cmsh, "Not implemented yet !!!\n");
	} else {
		cmdPrint(cmsh, "Wrong command = %d\n", cmd);
		return (CMD_IPC_ERROR);
	}
	return (CMD_IPC_OK);
}

int
ospfRedistributeSet (struct cmsh *cmsh, struct ospf *ospf, int type, int mtype, int mvalue)
{
	  int force = 0;

	fprintf(stderr,"xxxx metric : %d \n", mvalue);

	  if (ospf_is_type_redistributed (type))
	    {
	      if (mtype != ospf->dmetric[type].type)
	        {
	          ospf->dmetric[type].type = mtype;
	          force = LSA_REFRESH_FORCE;
	        }
	      if (mvalue != ospf->dmetric[type].value)
	        {
	          ospf->dmetric[type].value = mvalue;
	          force = LSA_REFRESH_FORCE;
	        }

	      ospf_external_lsa_refresh_type (ospf, type, force);

	        zlog_debug ("Redistribute[%s]: Refresh  Type[%d], Metric[%d]",
	                   ospf_redist_string(type),
	                   metric_type (ospf, type), metric_value (ospf, type));

	      return CMD_IPC_OK;
	    }

	  ospf->dmetric[type].type = mtype;
	  ospf->dmetric[type].value = mvalue;

	  ospfRibmgrRedistribute (cmsh, IPC_RIB_REDISTRIBUTE_ADD, type);

//	  if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
	    zlog_debug ("Redistribute[%s]: Start  Type[%d], Metric[%d]",
	               ospf_redist_string(type),
	               metric_type (ospf, type), metric_value (ospf, type));

	  ospf_asbr_status_update (ospf, ++ospf->redistribute);

	  return CMD_IPC_OK;
}

int
ospf_redistribute_unset (struct cmsh *cmsh, struct ospf *ospf, int type)
{
  if (type == om->ospfRedistributeDefault)
    return CMD_IPC_OK;

  if (!ospf_is_type_redistributed (type))
    return CMD_IPC_OK;

  ospfRibmgrRedistribute (cmsh, IPC_RIB_REDISTRIBUTE_DELETE, type);

//  if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
    zlog_debug ("Redistribute[%s]: Stop",
               ospf_redist_string(type));

  ospf->dmetric[type].type = -1;
  ospf->dmetric[type].value = -1;

  /* Remove the routes from OSPF table. */
  ospf_redistribute_withdraw (ospf, type);

  ospf_asbr_status_update (ospf, --ospf->redistribute);

  return CMD_IPC_OK;
}

int
ospf_redistribute_default_set (struct cmsh *cmsh, struct ospf *ospf, int originate,
                               int mtype, int mvalue)
{
  ospf->default_originate = originate;
  ospf->dmetric[DEFAULT_ROUTE].type = mtype;
  ospf->dmetric[DEFAULT_ROUTE].value = mvalue;

  if (ospf_is_type_redistributed (DEFAULT_ROUTE))
    {
      /* if ospf->default_originate changes value, is calling
	 ospf_external_lsa_refresh_default sufficient to implement
	 the change? */
      ospf_external_lsa_refresh_default (ospf);

//      if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
        zlog_debug ("Redistribute[%s]: Refresh  Type[%d], Metric[%d]",
                   ospf_redist_string(DEFAULT_ROUTE),
                   metric_type (ospf, DEFAULT_ROUTE),
                   metric_value (ospf, DEFAULT_ROUTE));
      return (CMD_IPC_OK);
    }

  ospfRibmgrRedistributeDefault (cmsh, IPC_RIB_REDISTRIBUTE_DEFAULT_ADD);

//  if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
    zlog_debug ("Redistribute[DEFAULT]: Start  Type[%d], Metric[%d]",
               metric_type (ospf, DEFAULT_ROUTE),
               metric_value (ospf, DEFAULT_ROUTE));

  if (ospf->router_id.s_addr == 0)
    ospf->external_origin |= (1 << DEFAULT_ROUTE);
  else
  {
	  struct timeval tv = {1,0};
	  taskTimerSet(ospfDefaultOriginateTimer, tv, 0, (void *)ospf);
  }

  ospf_asbr_status_update (ospf, ++ospf->redistribute);

  return (CMD_IPC_OK);
}

int
ospf_redistribute_default_unset (struct cmsh *cmsh, struct ospf *ospf)
{
  if (!ospf_is_type_redistributed (DEFAULT_ROUTE))
    return (CMD_IPC_OK);

  ospf->default_originate = DEFAULT_ORIGINATE_NONE;
  ospf->dmetric[DEFAULT_ROUTE].type = -1;
  ospf->dmetric[DEFAULT_ROUTE].value = -1;

  ospfRibmgrRedistributeDefault (cmsh, IPC_RIB_REDISTRIBUTE_DEFAULT_DELETE);

//  if (IS_DEBUG_OSPF (zebra, ZEBRA_REDISTRIBUTE))
    zlog_debug ("Redistribute[DEFAULT]: Stop");

  ospf_asbr_status_update (ospf, --ospf->redistribute);

  return (CMD_IPC_OK);
}

int
protoRedistNum(int afi, const char *s)
{
  if (! s)
    return -1;

  if (afi == AFI_IP)
    {
      if (strncmp (s, "k", 1) == 0)
	return RIB_ROUTE_TYPE_KERNEL;
      else if (strncmp (s, "c", 1) == 0)
	return RIB_ROUTE_TYPE_CONNECT;
      else if (strncmp (s, "s", 1) == 0)
	return RIB_ROUTE_TYPE_STATIC;
      else if (strncmp (s, "r", 1) == 0)
	return RIB_ROUTE_TYPE_RIP;
      else if (strncmp (s, "o", 1) == 0)
	return RIB_ROUTE_TYPE_OSPF;
      else if (strncmp (s, "i", 1) == 0)
	return RIB_ROUTE_TYPE_ISIS;
      else if (strncmp (s, "b", 1) == 0)
	return RIB_ROUTE_TYPE_BGP;
    }
  if (afi == AFI_IP6)
    {
      if (strncmp (s, "k", 1) == 0)
	return RIB_ROUTE_TYPE_KERNEL;
      else if (strncmp (s, "c", 1) == 0)
	return RIB_ROUTE_TYPE_CONNECT;
      else if (strncmp (s, "s", 1) == 0)
	return RIB_ROUTE_TYPE_STATIC;
      else if (strncmp (s, "r", 1) == 0)
	return RIB_ROUTE_TYPE_RIPNG;
      else if (strncmp (s, "o", 1) == 0)
	return RIB_ROUTE_TYPE_OSPF6;
      else if (strncmp (s, "i", 1) == 0)
	return RIB_ROUTE_TYPE_ISIS;
      else if (strncmp (s, "b", 1) == 0)
	return RIB_ROUTE_TYPE_BGP;
    }
  return -1;
}


//in_addr_t
//ospfGetRouterId(void)
//{
//	Prefix4T *p;
//
//	routerIdGet( p );
//	return p->prefix.s_addr;
//}
