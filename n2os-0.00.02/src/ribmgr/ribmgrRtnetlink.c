/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**
 * @brief       : RIB Manager에서 Kernel의 라우팅 테이블 및 인터페이스 테이블로 부터
 * 각각의 정보를 읽거나 제어하기 위한 Netlink 소켓을 핸들링 하는 함수들로 이루어진
 * 파일임.
 *
 * - Block Name : RIB Manager
 * - Process Name : ribmgr
 * - Creator : Suncheul Kim
 * - Initial Date : 2013/10/10
 */

/**
 * @file        : ribmgrRtnetlink.c
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#include <unistd.h>
#include <arpa/inet.h>

#include "nnRibDefines.h"
#include "nnIf.h"
#include "nnPrefix.h"
#include "nnTable.h"
#include "nosLib.h"

#include "ribmgrInit.h"
#include "ribmgrRib.h"
#include "ribmgrConnected.h"
#include "ribmgrRt.h"
#include "ribmgrRedistribute.h"
#include "ribmgrInterface.h"
#include "ribmgrDebug.h"
#include "ribmgrUtil.h"


/* Hack for GNU libc version 2. */
#ifndef MSG_TRUNC
#define MSG_TRUNC      0x20
#endif /* MSG_TRUNC */

/*
 * Socket interface to kernel.
 */
#ifdef HAVE_NETLINK
/* Receive buffer size for netlink socket */
Uint32T nlRcvBuffSize = 0;
#endif /* HAVE_NETLINK */


void kernelInit (void);

#define MULTIPATH_NUM 1


/* Message structure. */
typedef struct message
{
  Int32T      key;
  const char *str;
}MessageT;


/*
 * Description : 에러 문자열을 반환하는 함수.
 * (Wrapper around strerror to handle case where it returns NULL)
 *
 * param [in] errnum : 에러 코드
 *
 * retval : 에러 문자열 포인터
 */
const char *
safeStrError(Int32T errnum)
{
  const char *s = strerror(errnum);
  return (s != NULL) ? s : "Unknown error";
}


/*
 * Description : KEY를 기반으로 문자열을 반환하는 함수.
 *
 * param [in] mess : 문자열 포인터
 * param [in] key : KEY
 *
 * retval : NULL if 일치하는 것이 없으면,
             문자열 포인터 if 일치하는 내용이 존재하면
 */
char *
lookup (const MessageT *mess, Int32T key)
{
  MessageT *pnt;

  for (pnt = (MessageT *)mess; pnt->key != 0; pnt++)
    if (pnt->key == key)
      return (char *)pnt->str;

  return "";
}


static const MessageT NL_MSG_STR[] = {
  {RTM_NEWROUTE, "RTM_NEWROUTE"},
  {RTM_DELROUTE, "RTM_DELROUTE"},
  {RTM_GETROUTE, "RTM_GETROUTE"},
  {RTM_NEWLINK,  "RTM_NEWLINK"},
  {RTM_DELLINK,  "RTM_DELLINK"},
  {RTM_GETLINK,  "RTM_GETLINK"},
  {RTM_NEWADDR,  "RTM_NEWADDR"},
  {RTM_DELADDR,  "RTM_DELADDR"},
  {RTM_GETADDR,  "RTM_GETADDR"},
  {0, NULL}
};

static const char * NH_TYPES_DESC[] =
{
  "none",
  "Directly connected",
  "Interface route",
  "IPv4 nexthop",
  "IPv4 nexthop with ifindex",
  "IPv4 nexthop with ifname",
  "IPv6 nexthop",
  "IPv6 nexthop with ifindex",
  "IPv6 nexthop with ifname",
  "Null0 nexthop",
};


/*
 * Description : Netlink 기능을 초기화 하는 함수.
 *
 * retval : 0 always
 */
Int32T 
netlinkInit()
{
  /* Netlink Socket Init. */
  kernelInit ();

  /* Read Interface Netlink. */
  interfaceLookupNetlink ();

  /* Read Route Netlink. */
  routeReadNetlink ();

  return 0;
}


/*
 * Description : 인터페이스 인덱스를 설정하는 함수
 * (Note: on netlink systems, there should be a 1-to-1 mapping between 
 *  interface names and ifindex values.)
 *
 * param [in] pIf : interface 자료구조 포인터
 * param [in] ifiIndex : Interface 인덱스
 */
static void
setIfindex(InterfaceT *pIf, Uint32T ifiIndex)
{
  InterfaceT *pOIf = NULL;

  NNLOG (LOG_DEBUG, "%s : ifiIndex = %d\n", __func__, ifiIndex);

  if (((pOIf = ifLookupByIndex(ifiIndex)) != NULL) && (pOIf != pIf))
  {
    if (ifiIndex == IFINDEX_INTERNAL)
    {
        NNLOG (LOG_ERR, 
              "Err : Netlink is setting interface %s ifindex to reserved "
              "internal value %u\n", pIf->name, ifiIndex);
    }
    else
    {
      if (IS_RIBMGR_DEBUG_KERNEL)
        NNLOG (LOG_DEBUG, 
              "interface index %d was renamed from %s to %s\n",
              ifiIndex, pOIf->name, pIf->name);

	  if (ifIsUp(pOIf))
      {
        NNLOG (LOG_ERR, 
              "Err : interface rename detected on up interface: index %d "
              "was renamed from %s to %s, results are uncertain!\n", 
              ifiIndex, pOIf->name, pIf->name);
      }

      ifDeleteUpdate(pOIf);
    }
  }
  pIf->ifIndex = ifiIndex;
}


/*
 * Description : Netlink 소켓의 수신 버퍼 크기를 설정하는 함수.
 *
 * param [in] nl : NLSocketT 자료구조 포인터
 * param [in] newsize : 버퍼 크기
 *
 * retval : -1 if 실패
 *           0 if 성공
 */
static Int32T
netlinkRecvBuff (NLSocketT *nl, Uint32T newsize)
{
  Uint32T oldsize = 0;
  Int32T ret = 0;
  socklen_t newlen = sizeof(newsize);
  socklen_t oldlen = sizeof(oldsize);

  ret = getsockopt(nl->sock, SOL_SOCKET, SO_RCVBUF, &oldsize, &oldlen);
  if (ret < 0)
  {
    NNLOG (LOG_ERR, "Can't get %s receive buffer size: %s\n", nl->name,
	      safeStrError (errno));
    return -1;
  }

  ret = setsockopt(nl->sock, SOL_SOCKET, SO_RCVBUF, &nlRcvBuffSize,
                   sizeof(nlRcvBuffSize));
  if (ret < 0)
  {
    NNLOG (LOG_ERR, "Can't set %s receive buffer size: %s\n", nl->name,
          safeStrError (errno));
    return -1;
  }

  ret = getsockopt(nl->sock, SOL_SOCKET, SO_RCVBUF, &newsize, &newlen);
  if (ret < 0)
  {
    NNLOG (LOG_ERR, "Can't get %s receive buffer size: %s\n", nl->name,
          safeStrError (errno));
    return -1;
  }

  if (IS_RIBMGR_DEBUG_KERNEL)
    NNLOG (LOG_DEBUG, 
          "Setting netlink socket receive buffer size: %u -> %u\n",
          oldsize, newsize);
  return 0;
}


/*
 * Description : Netlink 소켓을 생성하는 함수.
 *
 * param [in] nl : NLSocketT 자료구조 포인터
 * param [in] groups : Netlink 그룹정보
 *
 * retval : -1 if 실패
 *           0 if 성공
 */
static Int32T
netlinkSocket (NLSocketT *nl, Uint32T groups)
{
  Int32T ret = 0;
  struct sockaddr_nl snl = {0,};
  Int32T sock = 0;
  Int32T nameLen = 0;
  Int32T saveErrNo = 0;

  sock = socket (AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock < 0)
  {
    NNLOG (LOG_ERR, "Can't open %s socket: %s\n", 
          nl->name, safeStrError (errno));
    return -1;
  }

  memset (&snl, 0, sizeof snl);
  snl.nl_family = AF_NETLINK;
  snl.nl_groups = groups;

  ret = bind (sock, (struct sockaddr *) &snl, sizeof snl);
  saveErrNo = errno;

  if (ret < 0)
  {
    NNLOG (LOG_ERR, "Can't bind %s socket to group 0x%x: %s\n",
          nl->name, snl.nl_groups, safeStrError (saveErrNo));
    close (sock);
    return -1;
  }

  /* multiple netlink sockets will have different nl_pid */
  nameLen = sizeof snl;
  ret = getsockname (sock, (struct sockaddr *) &snl, (socklen_t *) &nameLen);
  if (ret < 0 || nameLen != sizeof snl)
  {
    NNLOG (LOG_ERR, "Can't get %s socket name: %s\n", 
          nl->name, safeStrError (errno));
    close (sock);
    return -1;
  }

  nl->snl = snl;
  nl->sock = sock;
  return ret;
}


/*
 * Description : Netlink 소켓으로 커널에 정보를 요청하는 함수.
 *
 * param [in] family : 네트워크 패밀리
 * param [in] type : 요청정보 타입
 * param [in] nl : NLSocketT 자료구조 포인터
 *
 * retval : -1 if 실패
 *           0 if 성공
 */
static Int32T
netlinkRequest (Int32T family, Int32T type, NLSocketT *nl)
{
  Int32T ret = 0;
  Int32T saveErrNo = 0;
  struct sockaddr_nl snl = {0,};

  struct
  {
    struct nlmsghdr nlh;
    struct rtgenmsg g;
  } req;

  /* Check netlink socket. */
  if (nl->sock < 0)
  {
    NNLOG (LOG_ERR, "%s socket isn't active.\n", nl->name);
    return -1;
  }

  memset (&snl, 0, sizeof snl);
  snl.nl_family = AF_NETLINK;

  memset (&req, 0, sizeof req);
  req.nlh.nlmsg_len = sizeof req;
  req.nlh.nlmsg_type = type;
  req.nlh.nlmsg_flags = NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST;
  req.nlh.nlmsg_pid = nl->snl.nl_pid;
  req.nlh.nlmsg_seq = ++nl->seq;
  req.g.rtgen_family = family;

  ret = sendto (nl->sock, (void *) &req, sizeof req, 0,
                (struct sockaddr *) &snl, sizeof snl);
  saveErrNo = errno;

  if (ret < 0)
  {
    NNLOG (LOG_ERR, "%s sendto failed: %s\n", nl->name, safeStrError (saveErrNo));
    return -1;
  }

  return 0;
}


/*
 * Description : Netlink 소켓으로 부터 수신된 메시지를 파싱하는 함수.
 * 입력 변수인 filter 콜백 함수에서 처리하도록 되어 있다.
 *
 * param [in] filter : 콜백함수 포인터
 * param [in] nl : NLSocketT 자료구조 포인터
 *
 * retval : -1 if 실패
 *           0 if 성공
 */
static Int32T
netlinkParseInfo (Int32T (*filter) (struct sockaddr_nl *, 
                  struct nlmsghdr *), NLSocketT *nl)
{
  Int32T status = 0, ret = 0, error = 0;

  while (1)
  {
    char buf[4096] = {};
    struct iovec iov = { buf, sizeof buf };
    struct sockaddr_nl snl = {0,};
    struct msghdr msg = { (void *) &snl, sizeof snl, &iov, 1, NULL, 0, 0 };
    struct nlmsghdr *h = NULL;


    status = recvmsg (nl->sock, &msg, 0);
    if (status < 0)
    {
      if (errno == EINTR)
      {
        continue;
      }

      if (errno == EWOULDBLOCK || errno == EAGAIN)
      {
        break;
      }

      NNLOG (LOG_ERR, "%s recvmsg overrun: %s\n", 
            nl->name, safeStrError(errno));
      continue;
    }

    if (status == 0)
    {
      NNLOG (LOG_ERR, "%s EOF\n", nl->name);
      return -1;
    }

    if (msg.msg_namelen != sizeof snl)
    {
      NNLOG (LOG_ERR, "%s sender address length error: length %d \n",
            nl->name, msg.msg_namelen);
      return -1;
    }
      
    for (h = (struct nlmsghdr *) buf; NLMSG_OK (h, (Uint32T) status);
         h = NLMSG_NEXT (h, status))
    {
      /* Finish of reading. */
      if (h->nlmsg_type == NLMSG_DONE)
      {
        return ret;
      }

      /* Error handling. */
      if (h->nlmsg_type == NLMSG_ERROR)
      {
        struct nlmsgerr *err = (struct nlmsgerr *) NLMSG_DATA (h);
	    Int32T errnum = err->error;
	    Int32T msg_type = err->msg.nlmsg_type;

        /* If the error field is zero, then this is an ACK */
        if (err->error == 0)
        {
          if (IS_RIBMGR_DEBUG_KERNEL)
            NNLOG (LOG_DEBUG, "%s: %s ACK: type=%s(%u), seq=%u, pid=%u\n",
                  __FUNCTION__, nl->name,
                  lookup (NL_MSG_STR, err->msg.nlmsg_type),
                  err->msg.nlmsg_type, err->msg.nlmsg_seq,
                  err->msg.nlmsg_pid);

          /* return if not a multipart message, otherwise continue */
          if (!(h->nlmsg_flags & NLM_F_MULTI))
          {
            return 0;
          }
          continue;
        }

        if (h->nlmsg_len < NLMSG_LENGTH (sizeof (struct nlmsgerr)))
        {
          NNLOG (LOG_ERR, "%s error: message truncated\n", nl->name);
          return -1;
        }

        /* Deal with errors that occur because of races in link handling */
	    if (nl == &pRibmgr->NETLINK_CMD && 
            ((msg_type == RTM_DELROUTE &&
		     (-errnum == ENODEV || -errnum == ESRCH)) || 
            (msg_type == RTM_NEWROUTE && -errnum == EEXIST)))
        {
          if (IS_RIBMGR_DEBUG_KERNEL)
            NNLOG (LOG_DEBUG, "%s: error: %s type=%s(%u), seq=%u, pid=%u\n",
				  nl->name, safeStrError (-errnum),
				  lookup (NL_MSG_STR, msg_type),
				  msg_type, err->msg.nlmsg_seq, err->msg.nlmsg_pid);
          return 0;
        }

	    NNLOG (LOG_ERR, "%s error: %s, type=%s(%u), seq=%u, pid=%u\n",
              nl->name, safeStrError (-errnum),
              lookup (NL_MSG_STR, msg_type),
              msg_type, err->msg.nlmsg_seq, err->msg.nlmsg_pid);
        return -1;
      }

      /* OK we got netlink message. */
      if (IS_RIBMGR_DEBUG_KERNEL)
        NNLOG (LOG_DEBUG, "netlinkParseInfo: %s type %s(%u), seq=%u, pid=%u\n",
              nl->name,
              lookup (NL_MSG_STR, h->nlmsg_type), h->nlmsg_type,
              h->nlmsg_seq, h->nlmsg_pid);

      /* skip unsolicited messages originating from command socket */
      if (nl != &pRibmgr->NETLINK_CMD && 
          h->nlmsg_pid == pRibmgr->NETLINK_CMD.snl.nl_pid)
      {
        if (IS_RIBMGR_DEBUG_KERNEL)
          NNLOG (LOG_DEBUG, "netlinkParseInfo: %s packet comes from %s\n",
                pRibmgr->NETLINK_CMD.name, nl->name);
        continue;
      }
      error = (*filter) (&snl, h);
      if (error < 0)
      {
        NNLOG (LOG_ERR, "%s filter function error\n", nl->name);
        ret = error;
      }
    }

    /* After error care. */
    if (msg.msg_flags & MSG_TRUNC)
    {
      NNLOG (LOG_ERR, "%s error: message truncated\n", nl->name);
      continue;
    }

    if (status)
    {
      NNLOG(LOG_ERR, "%s error: data remnant size %d\n", nl->name, status);
      return -1;
    }
  } // while
  return ret;
}


/*
 * Description : rtattr 구조체 정보를 처리하는 유틸리티 함수.
 *
 * param [in] tb : rtattr로 이루어진 테이블(이중포인터)
 * param [in] max : rtattr 테이블 최대 크기
 * param [in] rta : rtattr 자료구조 포인터
 * param [in] len : rta 길이
 *
 * retval : -1 if 실패
 *           0 if 성공
 */
static void
netlinkParseRtattr (struct rtattr **tb, Int32T max, struct rtattr *rta,
                    Int32T len)
{
  while (RTA_OK (rta, len))
  {
    if (rta->rta_type <= max)
    {
      tb[rta->rta_type] = rta;
    }

    rta = RTA_NEXT (rta, len);
  }
}


/*
 * Description : Netlink 메시지 중 인터페이스 정보를 처리하는 함수.
 * interfaceLookupNetlink()로부터 호출되며, RIB Manager 초기화 시점에서만 사용됨.
 *
 * param [in] snl : sockaddr_nl 자료구조 포인터
 * param [in] h : nlmsghdr 자료구조 포인터
 *
 * retval : -1 if 실패
 *           0 if 성공
 */
static Int32T
netlinkInterface (struct sockaddr_nl *snl, struct nlmsghdr *h)
{
  Int32T i = 0, len = 0;
  struct rtattr *tb[IFLA_MAX + 1];
  struct ifinfomsg *ifi = NULL;
  InterfaceT *pIf = NULL;
  char *name = NULL;

  ifi = NLMSG_DATA (h);

  if (h->nlmsg_type != RTM_NEWLINK)
  {
    return 0;
  }

  len = h->nlmsg_len - NLMSG_LENGTH (sizeof (struct ifinfomsg));
  if (len < 0)
  {
    return -1;
  }

  /* Looking up interface name. */
  memset (tb, 0, sizeof tb);
  netlinkParseRtattr (tb, IFLA_MAX, IFLA_RTA (ifi), len);
  
#ifdef IFLA_WIRELESS
  /* check for wireless messages to ignore */
  if ((tb[IFLA_WIRELESS] != NULL) && (ifi->ifi_change == 0))
  {
    if (IS_RIBMGR_DEBUG_KERNEL)
      NNLOG (LOG_DEBUG, "%s: ignoring IFLA_WIRELESS message\n", __func__);
    return 0;
  }
#endif /* IFLA_WIRELESS */

  if (tb[IFLA_IFNAME] == NULL)
  {
    return -1;
  }
  name = (char *) RTA_DATA (tb[IFLA_IFNAME]);

  /* Add interface. */
  pIf = ifGetByName (name);

  if(pIf == NULL)
  {
    NNLOG (LOG_DEBUG, "If Name is NULL\n");
  }
  else
  {
    NNLOG (LOG_DEBUG, "If Name is %s\n", name);
  }

  setIfindex(pIf, ifi->ifi_index);
  pIf->flags = ifi->ifi_flags & 0x0000fffff;
  pIf->mtu6 = pIf->mtu = *(Uint32T *) RTA_DATA (tb[IFLA_MTU]);
  pIf->metric = 1;


  /* Hardware type and address. */
  pIf->hwType = ifi->ifi_type;

  if (tb[IFLA_ADDRESS])
  {
    Int32T hwAddrLen;

    hwAddrLen = RTA_PAYLOAD (tb[IFLA_ADDRESS]);

    if (hwAddrLen > INTERFACE_HWADDR_MAX)
    {
      NNLOG (LOG_WARNING, "Hardware address is too large: %d\n", hwAddrLen);
    }
    else
    {
      pIf->hwAddrLen = hwAddrLen;
      memcpy (pIf->hwAddr, RTA_DATA (tb[IFLA_ADDRESS]), hwAddrLen);

      for (i = 0; i < hwAddrLen; i++)
        if (pIf->hwAddr[i] != 0)
        {
          break;
        }

      if (i == hwAddrLen)
      {
        pIf->hwAddrLen = 0;
      }
      else
      {
        pIf->hwAddrLen = hwAddrLen;
      }
    }
  }

  ifAddUpdate (pIf);

  return 0;
}


/*
 * Description : Netlink 메시지 중 인터페이스 주소(IPv4IPv/6) 정보를 처리하는 함수.
 *
 * param [in] snl : sockaddr_nl 자료구조 포인터
 * param [in] h : nlmsghdr 자료구조 포인터
 *
 * retval : -1 if 실패
 *           0 if 성공
 */
static Int32T
netlinkInterfaceAddr (struct sockaddr_nl *snl, struct nlmsghdr *h)
{
  Int32T len = 0;
  Uint8T flags = 0;
  struct rtattr *tb[IFA_MAX + 1];
  struct ifaddrmsg *pIfAddr = NULL;
  InterfaceT *pIf = NULL;
  void *addr = NULL;
  void *broad = NULL;
  char *label = NULL;

  pIfAddr = NLMSG_DATA (h);

  if (pIfAddr->ifa_family != AF_INET
#ifdef HAVE_IPV6
      && pIfAddr->ifa_family != AF_INET6
#endif /* HAVE_IPV6 */
    )
  {
    return 0;
  }

  if (h->nlmsg_type != RTM_NEWADDR && h->nlmsg_type != RTM_DELADDR)
  {
    return 0;
  }

  len = h->nlmsg_len - NLMSG_LENGTH (sizeof (struct ifaddrmsg));
  if (len < 0)
  {
    return -1;
  }

  memset (tb, 0, sizeof tb);
  netlinkParseRtattr (tb, IFA_MAX, IFA_RTA (pIfAddr), len);

  pIf = ifLookupByIndex (pIfAddr->ifa_index);
  if (pIf == NULL)
  {
    NNLOG (LOG_ERR, "netlinkInterfaceAddr can't find interface by index %d\n",
          pIfAddr->ifa_index);
    return -1;
  }

  if (IS_RIBMGR_DEBUG_KERNEL)
  {
    char buf[BUFSIZ];
    NNLOG (LOG_DEBUG, "netlinkInterfaceAddr %s %s:\n",
          lookup (NL_MSG_STR, h->nlmsg_type), pIf->name);
    if (tb[IFA_LOCAL])
    {
      NNLOG (LOG_DEBUG, "  IFA_LOCAL     %s/%d\n",
            inet_ntop (pIfAddr->ifa_family, RTA_DATA (tb[IFA_LOCAL]),
            buf, BUFSIZ), pIfAddr->ifa_prefixlen);
    }
    if (tb[IFA_ADDRESS])
    {
      NNLOG (LOG_DEBUG, "  IFA_ADDRESS   %s/%d\n",
            inet_ntop (pIfAddr->ifa_family, RTA_DATA (tb[IFA_ADDRESS]),
                       buf, BUFSIZ), pIfAddr->ifa_prefixlen);
    }
    if (tb[IFA_BROADCAST])
    {
      NNLOG (LOG_DEBUG, "  IFA_BROADCAST %s/%d\n",
            inet_ntop (pIfAddr->ifa_family, RTA_DATA (tb[IFA_BROADCAST]),
            buf, BUFSIZ), pIfAddr->ifa_prefixlen);
    }
    if (tb[IFA_LABEL] && strcmp (pIf->name, RTA_DATA (tb[IFA_LABEL])))
    {
      NNLOG (LOG_DEBUG, "  IFA_LABEL     %s\n", (char *)RTA_DATA (tb[IFA_LABEL]));
    }
      
    if (tb[IFA_CACHEINFO])
    {
      struct ifa_cacheinfo *ci = RTA_DATA (tb[IFA_CACHEINFO]);
      NNLOG (LOG_DEBUG, "  IFA_CACHEINFO pref %d, valid %d\n",
            ci->ifa_prefered, ci->ifa_valid);
    }
  }
  
  /* logic copied from iproute2/ip/ipaddress.c:print_addrinfo() */
  if (tb[IFA_LOCAL] == NULL)
  {
    tb[IFA_LOCAL] = tb[IFA_ADDRESS];
  }
  if (tb[IFA_ADDRESS] == NULL)
  {
    tb[IFA_ADDRESS] = tb[IFA_LOCAL];
  }
  
  /* local interface address */
  addr = (tb[IFA_LOCAL] ? RTA_DATA(tb[IFA_LOCAL]) : NULL);

  /* is there a peer address? */
  if (tb[IFA_ADDRESS] &&
      memcmp(RTA_DATA(tb[IFA_ADDRESS]), RTA_DATA(tb[IFA_LOCAL]), RTA_PAYLOAD(tb[IFA_ADDRESS])))
  {
    broad = RTA_DATA(tb[IFA_ADDRESS]);
    SET_FLAG (flags, RIBMGR_IFA_PEER);
  }
  else
  {
    /* seeking a broadcast address */
    broad = (tb[IFA_BROADCAST] ? RTA_DATA(tb[IFA_BROADCAST]) : NULL);
  }

  /* addr is primary key, SOL if we don't have one */
  if (addr == NULL)
  {
    NNLOG (LOG_ERR, "%s: NULL address\n", __func__);
    return -1;
  }

  /* Flags. */
  if (pIfAddr->ifa_flags & IFA_F_SECONDARY)
  {
    if (IS_RIBMGR_DEBUG_KERNEL)
      NNLOG (LOG_DEBUG, "Secondary Flag Set\n");
    SET_FLAG (flags, RIBMGR_IFA_SECONDARY);
  }

  /* Label */
  if (tb[IFA_LABEL])
  {
    label = (char *) RTA_DATA (tb[IFA_LABEL]);
  }

  if (pIf && label && strcmp (pIf->name, label) == 0)
  {
    label = NULL;
  }

  /* Register interface address to the interface. */
  if (pIfAddr->ifa_family == AF_INET)
  {
    if (h->nlmsg_type == RTM_NEWADDR)
    {
      connectedAddIpv4 (pIf, flags,
                        (struct in_addr *) addr, pIfAddr->ifa_prefixlen,
                        (struct in_addr *) broad, label);
    }
    else
    {
      connectedDeleteIpv4 (pIf, flags,
                           (struct in_addr *) addr, pIfAddr->ifa_prefixlen,
                           (struct in_addr *) broad);
    }
  }
#ifdef HAVE_IPV6
  if (pIfAddr->ifa_family == AF_INET6)
  {
    if (h->nlmsg_type == RTM_NEWADDR)
    {
      connectedAddIpv6 (pIf, flags,
                        (struct in6_addr *) addr, pIfAddr->ifa_prefixlen,
                        (struct in6_addr *) broad, label);
    }
    else
    {
      connectedDeleteIpv6 (pIf,
                           (struct in6_addr *) addr, pIfAddr->ifa_prefixlen,
                           (struct in6_addr *) broad);
    }
  }
#endif /* HAVE_IPV6 */

  // display interface list
  ifDumpAll();

  return 0;
}

static const MessageT RT_PROTO_STR[] = {
  {RTPROT_REDIRECT, "redirect"},
  {RTPROT_KERNEL,   "kernel"},
  {RTPROT_BOOT,     "boot"},
  {RTPROT_STATIC,   "static"},
  {RTPROT_GATED,    "GateD"},
  {RTPROT_RA,       "router advertisement"},
  {RTPROT_MRT,      "MRT"},
  {RTPROT_ZEBRA,    "Zebra"},
#ifdef RTPROT_BIRD
  {RTPROT_BIRD,     "BIRD"},
#endif /* RTPROT_BIRD */
  {0,               NULL}
};


/*
 * Description : Netlink 메시지 중 루트 정보를 처리하는 함수.
 *
 * param [in] snl : sockaddr_nl 자료구조 포인터
 * param [in] h : nlmsghdr 자료구조 포인터
 *
 * retval : -1 if 실패
 *           0 if 성공
 */
static Int32T
netlinkRoutingTable (struct sockaddr_nl *snl, struct nlmsghdr *h)
{
  struct rtmsg *rtm = NULL;
  struct rtattr *tb[RTA_MAX + 1];
  char anyaddr[16] = { 0 };
  u_char flags = 0;
  Int32T idx = 0, table = 0, metric = 0, len = 0; 
  void *pDest = NULL, *pGate = NULL, *pSrc = NULL;

  rtm = NLMSG_DATA (h);

  if (IS_RIBMGR_DEBUG_KERNEL)
    NNLOG (LOG_DEBUG, "%s %s %s proto %s\n",
          h->nlmsg_type ==
          RTM_NEWROUTE ? "RTM_NEWROUTE" : "RTM_DELROUTE",
          rtm->rtm_family == AF_INET ? "ipv4" : "ipv6",
          rtm->rtm_type == RTN_UNICAST ? "unicast" : "multicast",
          lookup (RT_PROTO_STR, rtm->rtm_protocol));

  if (h->nlmsg_type != RTM_NEWROUTE)
  {
    return 0;
  }

  if (rtm->rtm_type != RTN_UNICAST)
  {
    return 0;
  }

  table = rtm->rtm_table;

  len = h->nlmsg_len - NLMSG_LENGTH (sizeof (struct rtmsg));
  if (len < 0)
  {
    return -1;
  }

  memset (tb, 0, sizeof tb);
  netlinkParseRtattr (tb, RTA_MAX, RTM_RTA (rtm), len);

  if (rtm->rtm_flags & RTM_F_CLONED)
  {
    return 0;
  }
  if (rtm->rtm_protocol == RTPROT_REDIRECT)
  {
    return 0;
  }
  if (rtm->rtm_protocol == RTPROT_KERNEL)
  {
    return 0;
  }

  if (rtm->rtm_src_len != 0)
  {
    return 0;
  }

  /* Route which inserted by Zebra. */
  if (rtm->rtm_protocol == RTPROT_ZEBRA)
  {
    flags |= RIB_FLAG_SELFROUTE;
  }


  if (tb[RTA_OIF])
  {
    idx = *(int *) RTA_DATA (tb[RTA_OIF]);
  }

  if (tb[RTA_DST])
  {
    pDest = RTA_DATA (tb[RTA_DST]);
  }
  else
  {
    pDest = anyaddr;
  }

  if (tb[RTA_PREFSRC])
  {
    pSrc = RTA_DATA (tb[RTA_PREFSRC]);
  }

  /* Multipath treatment is needed. */
  if (tb[RTA_GATEWAY])
  {
    pGate = RTA_DATA (tb[RTA_GATEWAY]);
  }

  if (tb[RTA_PRIORITY])
  {
    metric = *(Int32T *) RTA_DATA(tb[RTA_PRIORITY]);
  }


  if (rtm->rtm_family == AF_INET)
  {
    Prefix4T p;
    p.family = AF_INET;
    memcpy (&p.prefix, pDest, 4);
    p.prefixLen = rtm->rtm_dst_len;

    ribAddIpv4 (RIB_ROUTE_TYPE_KERNEL, 
                  flags, &p, pGate, pSrc, idx, table, metric, 0);
  }
#ifdef HAVE_IPV6
  if (rtm->rtm_family == AF_INET6)
  {
    Prefix6T prefix6;
    prefix6.family = AF_INET6;
    memcpy (&prefix6.prefix, pDest, 16);
    prefix6.prefixLen = rtm->rtm_dst_len;

    ribAddIpv6 (RIB_ROUTE_TYPE_KERNEL, 
                flags, &prefix6, pGate, idx, table, metric, 0);
  }
#endif /* HAVE_IPV6 */

  return 0;
}


/*
 * Description : Kernel에서 루트가 변경되었을 경우 호출되는 함수.
 *
 * param [in] snl : sockaddr_nl 자료구조 포인터
 * param [in] h : nlmsghdr 자료구조 포인터
 *
 * retval : -1 if 실패
 *           0 if 성공
 */
static Int32T
netlinkRouteChange (struct sockaddr_nl *snl, struct nlmsghdr *h)
{
  struct rtmsg *rtm = NULL;
  struct rtattr *tb[RTA_MAX + 1];
  char anyaddr[16] = { 0 };
  Int32T idx = 0, table = 0, len = 0;
  void *pDest = NULL, *pGate = NULL, *pSrc = NULL;

  if (IS_RIBMGR_DEBUG_KERNEL)
    NNLOG (LOG_DEBUG, "%s called\n", __func__);

  rtm = NLMSG_DATA (h);

  if (!(h->nlmsg_type == RTM_NEWROUTE || h->nlmsg_type == RTM_DELROUTE))
  {
    /* If this is not route add/delete message print warning. */
    NNLOG (LOG_DEBUG, "Kernel message: %d\n", h->nlmsg_type);
    return 0;
  }

  /* Connected route. */
  if (IS_RIBMGR_DEBUG_KERNEL)
    NNLOG (LOG_DEBUG, "%s %s %s proto %s\n",
          h->nlmsg_type ==
          RTM_NEWROUTE ? "RTM_NEWROUTE" : "RTM_DELROUTE",
          rtm->rtm_family == AF_INET ? "ipv4" : "ipv6",
          rtm->rtm_type == RTN_UNICAST ? "unicast" : "multicast",
          lookup (RT_PROTO_STR, rtm->rtm_protocol));

  if (rtm->rtm_type != RTN_UNICAST)
  {
    return 0;
  }

  table = rtm->rtm_table;

  len = h->nlmsg_len - NLMSG_LENGTH (sizeof (struct rtmsg));
  if (len < 0)
  {
    return -1;
  }

  memset (tb, 0, sizeof tb);
  netlinkParseRtattr (tb, RTA_MAX, RTM_RTA (rtm), len);

  if (rtm->rtm_flags & RTM_F_CLONED)
  {
    return 0;
  }
  if (rtm->rtm_protocol == RTPROT_REDIRECT)
  {
    return 0;
  }
  if (rtm->rtm_protocol == RTPROT_KERNEL)
  {
    return 0;
  }

  if (rtm->rtm_src_len != 0)
  {
    NNLOG (LOG_WARNING, "netlinkRouteChange(): no src len\n");
    return 0;
  }

  if (tb[RTA_OIF])
  {
    idx = *(Int32T *) RTA_DATA (tb[RTA_OIF]);
  }

  if (tb[RTA_DST])
  {
    pDest = RTA_DATA (tb[RTA_DST]);
  }
  else
  {
    pDest = anyaddr;
  }

  if (tb[RTA_GATEWAY])
  {
    pGate = RTA_DATA (tb[RTA_GATEWAY]);
  }

  if (tb[RTA_PREFSRC])
  {
    pSrc = RTA_DATA (tb[RTA_PREFSRC]);
  }

  if (rtm->rtm_family == AF_INET)
  {
    Prefix4T p;
    p.family = AF_INET;
    memcpy (&p.prefix, pDest, 4);
    p.prefixLen = rtm->rtm_dst_len;

    if (h->nlmsg_type == RTM_NEWROUTE)
    {
      NNLOG (LOG_DEBUG, "RTM_NEWROUTE %s/%d\n", inet_ntoa (p.prefix), p.prefixLen);
    }
    else
    {
      NNLOG (LOG_DEBUG, "RTM_DELROUTE %s/%d\n", inet_ntoa (p.prefix), p.prefixLen);
    }

    if (h->nlmsg_type == RTM_NEWROUTE)
    {
      ribAddIpv4(RIB_ROUTE_TYPE_KERNEL, 0, &p, pGate, pSrc, idx, table, 0, 0);
    }
    else
    {
      ribDeleteIpv4(RIB_ROUTE_TYPE_KERNEL, 0, &p, pGate, idx, table);
    }
  }

#ifdef HAVE_IPV6
  if (rtm->rtm_family == AF_INET6)
  {
    Prefix6T p = {0,};
    char buf[BUFSIZ] = {};

    p.family = AF_INET6;
    memcpy (&p.prefix, pDest, 16);
    p.prefixLen = rtm->rtm_dst_len;

    if (h->nlmsg_type == RTM_NEWROUTE)
    {
      NNLOG (LOG_DEBUG, "RTM_NEWROUTE %s/%d\n",
            inet_ntop (AF_INET6, &p.prefix, buf, BUFSIZ),
            p.prefixLen);
    }
    else
    {
      NNLOG (LOG_DEBUG, "RTM_DELROUTE %s/%d\n",
            inet_ntop (AF_INET6, &p.prefix, buf, BUFSIZ),
            p.prefixLen);
    }

    if (h->nlmsg_type == RTM_NEWROUTE)
    {
      ribAddIpv6 (RIB_ROUTE_TYPE_KERNEL, 0, &p, pGate, idx, table, 0, 0);
    }
    else
    {
      ribDeleteIpv6 (RIB_ROUTE_TYPE_KERNEL, 0, &p, pGate, idx, table);
    }
  }
#endif /* HAVE_IPV6 */

  return 0;
}


/*
 * Description : Kernel에서 인터페이스가 변경되었을 경우 호출되는 함수.
 *
 * param [in] snl : sockaddr_nl 자료구조 포인터
 * param [in] h : nlmsghdr 자료구조 포인터
 *
 * retval : -1 if 실패
 *           0 if 성공
 */
static Int32T
netlinkLinkChange (struct sockaddr_nl *snl, struct nlmsghdr *h)
{
  Int32T len = 0;
  struct ifinfomsg *pIfi = NULL;
  struct rtattr *tb[IFLA_MAX + 1];
  InterfaceT *pIf = NULL;
  char *name = NULL;

  if (IS_RIBMGR_DEBUG_KERNEL) 
    NNLOG (LOG_DEBUG, "%s called\n", __func__);

  pIfi = NLMSG_DATA (h);

  if (!(h->nlmsg_type == RTM_NEWLINK || h->nlmsg_type == RTM_DELLINK))
  {
    /* If this is not link add/delete message so print warning. */
    NNLOG (LOG_WARNING, "netlinkLinkChange: wrong kernel message %d\n",
          h->nlmsg_type);
    return 0;
  }

  len = h->nlmsg_len - NLMSG_LENGTH (sizeof (struct ifinfomsg));
  if (len < 0)
  {
    return -1;
  }

  /* Looking up interface name. */
  memset (tb, 0, sizeof tb);
  netlinkParseRtattr (tb, IFLA_MAX, IFLA_RTA (pIfi), len);

#ifdef IFLA_WIRELESS
  /* check for wireless messages to ignore */
  if ((tb[IFLA_WIRELESS] != NULL) && (pIfi->ifi_change == 0))
  {
    if (IS_RIBMGR_DEBUG_KERNEL)
      NNLOG (LOG_DEBUG, "%s: ignoring IFLA_WIRELESS message\n", __func__);
    return 0;
  }
#endif /* IFLA_WIRELESS */
  
  if (tb[IFLA_IFNAME] == NULL)
  {
    return -1;
  }
  name = (char *) RTA_DATA (tb[IFLA_IFNAME]);

  if (IS_RIBMGR_DEBUG_KERNEL)
    NNLOG (LOG_DEBUG, "IF NAME = %s\n", name);

  /* Add interface. */
  if (h->nlmsg_type == RTM_NEWLINK)
  {
    pIf = ifLookupByName (name);

    if (pIf == NULL || !CHECK_FLAG (pIf->status, RIBMGR_INTERFACE_ACTIVE))
    {
      if (pIf == NULL)
      {
        pIf = ifGetByName (name);
      }

      setIfindex(pIf, pIfi->ifi_index);
      pIf->flags = pIfi->ifi_flags & 0x0000fffff;
      pIf->mtu6 = pIf->mtu = *(Int32T *) RTA_DATA (tb[IFLA_MTU]);
      pIf->metric = 1;

      /* If new link is added. */
      ifAddUpdate (pIf);
    }
    else
    {
      /* Interface status change. */
      setIfindex(pIf, pIfi->ifi_index);
      pIf->mtu6 = pIf->mtu = *(Int32T *) RTA_DATA (tb[IFLA_MTU]);
      pIf->metric = 1;

      if (ifIsOperative (pIf))
      {
        pIf->flags = pIfi->ifi_flags & 0x0000fffff;
        if (!ifIsOperative (pIf))
        {
          ifStateDown (pIf);
        }
      }
      else
      {
        pIf->flags = pIfi->ifi_flags & 0x0000fffff;
        if (ifIsOperative (pIf))
        {
          ifStateUp (pIf);
        }
      }
    }
  }
  /* Interface Delete. */
  else
  {
    /* RTM_DELLINK. */
    pIf = ifLookupByName (name);

    if (pIf == NULL)
    {
      NNLOG (LOG_WARNING, "interface %s is deleted but can't find\n", name);
      return 0;
    }

    ifDeleteUpdate (pIf);
  }

  /* display interface list */
  ifDumpAll();

  return 0;
}


/*
 * Description : Kernel에서 인터페이스가 변경되었을 경우 해당 처리루틴트로 분기
 * 해주는 함수.
 *
 * param [in] snl : sockaddr_nl 자료구조 포인터
 * param [in] h : nlmsghdr 자료구조 포인터
 *
 * retval : -1 if 실패
 *           0 if 성공
 */
static Int32T
netlinkInformationFetch (struct sockaddr_nl *snl, struct nlmsghdr *h)
{
  /* JF: Ignore messages that aren't from the kernel */
  if ( snl->nl_pid != 0 )
  {
    NNLOG (LOG_ERR, "Ignoring message from pid %u\n", snl->nl_pid );
    return -1;
  }

  switch (h->nlmsg_type)
  {
    case RTM_NEWROUTE:
      return netlinkRouteChange (snl, h);
      break;
    case RTM_DELROUTE:
      return netlinkRouteChange (snl, h);
      break;
    case RTM_NEWLINK:
      return netlinkLinkChange (snl, h);
      break;
    case RTM_DELLINK:
      return netlinkLinkChange (snl, h);
      break;
    case RTM_NEWADDR:
      return netlinkInterfaceAddr (snl, h);
      break;
    case RTM_DELADDR:
      return netlinkInterfaceAddr (snl, h);
      break;
    default:
      NNLOG (LOG_WARNING, "Unknown netlink nlmsg_type %d\n", h->nlmsg_type);
      break;
  }
  return 0;
}


/* Description : RIB Manager의 초기화 시점에 Kernel로부터 인터페이스 정보를 
 * 요청하는 함수. */
Int32T
interfaceLookupNetlink (void)
{
  Int32T ret = 0;

  /* Get interface information. */
  ret = netlinkRequest (AF_PACKET, RTM_GETLINK, &pRibmgr->NETLINK_CMD);
  if (ret < 0)
  {
    return ret;
  }
  ret = netlinkParseInfo (netlinkInterface, &pRibmgr->NETLINK_CMD);
  if (ret < 0)
  {
    return ret;
  }

  /* Get IPv4 address of the interfaces. */
  ret = netlinkRequest (AF_INET, RTM_GETADDR, &pRibmgr->NETLINK_CMD);
  if (ret < 0)
  {
    return ret;
  }
  ret = netlinkParseInfo (netlinkInterfaceAddr, &pRibmgr->NETLINK_CMD);
  if (ret < 0)
  {
    return ret;
  }

#ifdef HAVE_IPV6
  /* Get IPv6 address of the interfaces. */
  ret = netlinkRequest (AF_INET6, RTM_GETADDR, &pRibmgr->NETLINK_CMD);
  if (ret < 0)
  {
    return ret;
  }
  ret = netlinkParseInfo (netlinkInterfaceAddr, &pRibmgr->NETLINK_CMD);
  if (ret < 0)
  {
    return ret;
  }
#endif /* HAVE_IPV6 */

  // display interface and connected address
  ifDumpAll();

  return 0;
}


/* Description : RIB Manager의 초기화 시점에 Kernel로부터 루트 정보를 
 * 요청하는 함수. */
Int32T
routeReadNetlink  (void)
{
  Int32T ret = 0;

  /* Get IPv4 routing table. */
  ret = netlinkRequest (AF_INET, RTM_GETROUTE, &pRibmgr->NETLINK_CMD);
  if (ret < 0)
  {
    return ret;
  }
  ret = netlinkParseInfo (netlinkRoutingTable, &pRibmgr->NETLINK_CMD);
  if (ret < 0)
  {
    return ret;
  }

#ifdef HAVE_IPV6
  /* Get IPv6 routing table. */
  ret = netlinkRequest (AF_INET6, RTM_GETROUTE, &pRibmgr->NETLINK_CMD);
  if (ret < 0)
  {
    return ret;
  }
  ret = netlinkParseInfo (netlinkRoutingTable, &pRibmgr->NETLINK_CMD);
  if (ret < 0)
  {
    return ret;
  }
#endif /* HAVE_IPV6 */

  return 0;
}


/*
 * rtattr 처리하는 유틸리티 함수.
 */
static Int32T
addattr_l (struct nlmsghdr *n, Int32T maxLen, Int32T type, void *pData, Int32T alen)
{
  Int32T len = 0;
  struct rtattr *rta = NULL;

  len = RTA_LENGTH (alen);

  if (NLMSG_ALIGN (n->nlmsg_len) + len > maxLen)
  {
    return -1;
  }

  rta = (struct rtattr *) (((char *) n) + NLMSG_ALIGN (n->nlmsg_len));
  rta->rta_type = type;
  rta->rta_len = len;
  memcpy (RTA_DATA (rta), pData, alen);
  n->nlmsg_len = NLMSG_ALIGN (n->nlmsg_len) + len;

  return 0;
}


/*
 * rtattr 처리하는 유틸리티 함수.
 */
static Int32T
rta_addattr_l (struct rtattr *rta, Int32T maxLen, Int32T type, void *pData, Int32T alen)
{
  Int32T len = 0;
  struct rtattr *subrta = NULL;

  len = RTA_LENGTH (alen);

  if (RTA_ALIGN (rta->rta_len) + len > maxLen)
  {
    return -1;
  }

  subrta = (struct rtattr *) (((char *) rta) + RTA_ALIGN (rta->rta_len));
  subrta->rta_type = type;
  subrta->rta_len = len;
  memcpy (RTA_DATA (subrta), pData, alen);
  rta->rta_len = NLMSG_ALIGN (rta->rta_len) + len;

  return 0;
}


/*
 * rtattr 처리하는 유틸리티 함수.
 */
static Int32T
addattr32 (struct nlmsghdr *n, Int32T maxLen, Int32T type, Int32T data)
{
  Int32T len = 0;
  struct rtattr *rta = NULL;

  len = RTA_LENGTH (4);

  if (NLMSG_ALIGN (n->nlmsg_len) + len > maxLen)
  {
    return -1;
  }

  rta = (struct rtattr *) (((char *) n) + NLMSG_ALIGN (n->nlmsg_len));
  rta->rta_type = type;
  rta->rta_len = len;
  memcpy (RTA_DATA (rta), &data, 4);
  n->nlmsg_len = NLMSG_ALIGN (n->nlmsg_len) + len;

  return 0;
}


static Int32T
netlinkTalkFilter (struct sockaddr_nl *snl, struct nlmsghdr *h)
{
  NNLOG (LOG_DEBUG, "netlinkTalk: ignoring message type 0x%04x \n", h->nlmsg_type);
  return 0;
}


/*
 * Description : Netlink 소켓으로 Kernel로 메시지 전송 하는 함수.
 * 이후 recvmsg()로 커널로부터 정보를 수신한다.
 *
 * param [in] n : nlmsghdr 자료구조 포인터
 * param [in] nl : NLSocketT 자료구조 포인터
 * 
 * retval : -1 if 실패
 *           0 if 성공
 */
static Int32T
netlinkTalk (struct nlmsghdr *n, NLSocketT *nl)
{
  Int32T status = 0, saveErrNo = 0;
  struct sockaddr_nl snl = {0,};
  struct iovec iov = { (void *) n, n->nlmsg_len };
  struct msghdr msg = { (void *) &snl, sizeof snl, &iov, 1, NULL, 0, 0 };

  memset (&snl, 0, sizeof snl);
  snl.nl_family = AF_NETLINK;

  n->nlmsg_seq = ++nl->seq;

  /* Request an acknowledgement by setting NLM_F_ACK */
  n->nlmsg_flags |= NLM_F_ACK;

  if (IS_RIBMGR_DEBUG_KERNEL)
    NNLOG (LOG_DEBUG, "netlinkTalk: %s type %s(%u), seq=%u \n", nl->name,
          lookup (NL_MSG_STR, n->nlmsg_type), n->nlmsg_type,
          n->nlmsg_seq);

  /* Send message to netlink interface. */
  status = sendmsg (nl->sock, &msg, 0);
  saveErrNo = errno;

  if (status < 0)
  {
    NNLOG (LOG_ERR, "netlinkTalk sendmsg() error: %s \n",
          safeStrError (saveErrNo));
    return -1;
  }

  /* 
   * Get reply from netlink socket. 
   * The reply should either be an acknowlegement or an error.
   */
  return netlinkParseInfo (netlinkTalkFilter, nl);
}


/*
 * Description : Netlink 소켓으로 Kernel로 루트 요청 메시지를 전송하는 함수.
 * 
 * param [in] cmd : 명령 타입
 * param [in] family : 네트워크 패킬리 
 * param [in] dest : IP 목적지 주소
 * param [in] length : IP 목적지 주소 길이
 * param [in] gate : nexthop gateway 주소 
 * param [in] idx : 인덱스
 * param [in] ribmgrFlags : 플래그
 * param [in] table : 루트 테이블 번호
 *
 * retval : -1 if 실패
 *           0 if 성공
 */
#ifdef HAVE_IPV6
static Int32T
netlinkRoute (Int32T cmd, Int32T family, void *pDest, Int32T length, void *pGate,
              Int32T idx, Int32T ribmgrFlags, Int32T table)
{
  Int32T ret =0, byteLen = 0, discard = 0;
  struct sockaddr_nl snl = {0,};

  struct
  {
    struct nlmsghdr n;
    struct rtmsg r;
    char buf[1024];
  } req;

  memset (&req, 0, sizeof req);

  byteLen = (family == AF_INET ? 4 : 16);

  req.n.nlmsg_len = NLMSG_LENGTH (sizeof (struct rtmsg));
  req.n.nlmsg_flags = NLM_F_CREATE | NLM_F_REQUEST;
  req.n.nlmsg_type = cmd;
  req.r.rtm_family = family;
  req.r.rtm_table = table;
  req.r.rtm_dst_len = length;
  req.r.rtm_protocol = RTPROT_ZEBRA;
  req.r.rtm_scope = RT_SCOPE_UNIVERSE;

  if ((ribmgrFlags & RIB_FLAG_NULL0) || (ribmgrFlags & RIB_FLAG_REJECT))
  {
    discard = 1;
  }
  else
  {
    discard = 0;
  }

  if (cmd == RTM_NEWROUTE)
  {
    if (discard)
    {
      if (ribmgrFlags & RIB_FLAG_NULL0)
      {
        req.r.rtm_type = RTN_BLACKHOLE;
      }
      else if (ribmgrFlags & RIB_FLAG_REJECT)
      {
        req.r.rtm_type = RTN_UNREACHABLE;
      }
      else
      {
        assert (RTN_BLACKHOLE != RTN_UNREACHABLE);  /* false */
      }
    }
    else
    {
      req.r.rtm_type = RTN_UNICAST;
    }
  }

  if (pDest)
  {
    addattr_l (&req.n, sizeof req, RTA_DST, pDest, byteLen);
  }

  if (!discard)
  {
    if (pGate)
    {
      addattr_l (&req.n, sizeof req, RTA_GATEWAY, pGate, byteLen);
    }
    if (idx > 0)
    {
      addattr32 (&req.n, sizeof req, RTA_OIF, idx);
    }
  }

  /* Destination netlink address. */
  memset (&snl, 0, sizeof snl);
  snl.nl_family = AF_NETLINK;

  /* Talk to netlink socket. */
  ret = netlinkTalk (&req.n, &pRibmgr->NETLINK_CMD);
  if (ret < 0)
  {
    return -1;
  }

  return 0;
}
#endif /* HAVE_IPV6 */

/*
 * Description : Netlink 소켓을 이용하여 Kernel로 루트 설정/삭제 요청 메시지를 
 * 전송하는 함수.
 *
 * param [in] cmd : 명령 타입
 * param [in] p : PrefixT 자료구조 포인터
 * param [in] rib : rib 자료구조 포인터
 * param [in] failiy : 네트워크 패밀리
 *
 * retval : -1 if 실패,
 *           0 if 성공
 */
static Int32T
netlinkRouteMultipath (Int32T cmd, PrefixT *pPrefix, RibT *pRib, Int32T family)
{
  Int32T byteLen = 0, nextHopNum = 0, discard = 0;
  struct sockaddr_nl snl = {0,};
  NexthopT *pNextHop = NULL;

  struct
  {
    struct nlmsghdr n;
    struct rtmsg r;
    char buf[1024];
  } req;

  memset (&req, 0, sizeof req);

  byteLen = (family == AF_INET ? 4 : 16);

  req.n.nlmsg_len = NLMSG_LENGTH (sizeof (struct rtmsg));
  req.n.nlmsg_flags = NLM_F_CREATE | NLM_F_REQUEST;
  req.n.nlmsg_type = cmd;
  req.r.rtm_family = family;
  req.r.rtm_table = pRib->table;
  req.r.rtm_dst_len = pPrefix->prefixLen;
  req.r.rtm_protocol = RTPROT_ZEBRA;
  req.r.rtm_scope = RT_SCOPE_UNIVERSE;

  if ((pRib->flags & RIB_FLAG_NULL0) || (pRib->flags & RIB_FLAG_REJECT))
  {
    discard = 1;
  }
  else
  {
    discard = 0;
  }

  if (cmd == RTM_NEWROUTE)
  {
    if (discard)
    {
      if (pRib->flags & RIB_FLAG_NULL0)
      {
        req.r.rtm_type = RTN_BLACKHOLE;
      }
      else if (pRib->flags & RIB_FLAG_REJECT)
      {
        req.r.rtm_type = RTN_UNREACHABLE;
      }
      else
      {
        assert (RTN_BLACKHOLE != RTN_UNREACHABLE);  /* false */
      }
    }
    else
    {
      req.r.rtm_type = RTN_UNICAST;
    }
  }

  if (pPrefix->family == AF_INET)
  {
    addattr_l (&req.n, sizeof req, RTA_DST, &pPrefix->u.prefix4, byteLen);
  }
#ifdef HAVE_IPV6
  else if (pPrefix->family == AF_INET6)
  {
    addattr_l (&req.n, sizeof req, RTA_DST, &pPrefix->u.prefix6, byteLen);
  }
#endif
  else
  {
    NNLOG (LOG_ERR, "Wrong family = %d\n", pPrefix->family);
    return 0;
  }

  /* Metric. */
  addattr32 (&req.n, sizeof req, RTA_PRIORITY, pRib->metric);

  if (discard)
  {
    if (cmd == RTM_NEWROUTE)
      for (pNextHop = pRib->pNexthop; pNextHop; pNextHop = pNextHop->next)
        SET_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB);
    goto skip;
  }

  /* Multipath case. */
  if (pRib->nexthopActiveNum == 1 || MULTIPATH_NUM == 1)
  {
    for (pNextHop = pRib->pNexthop; pNextHop; pNextHop = pNextHop->next)
    {

      if ((cmd == RTM_NEWROUTE && 
           CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE)) || 
          (cmd == RTM_DELROUTE && 
           CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB)))
      {

        if (CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_RECURSIVE))
        {
          if (IS_RIBMGR_DEBUG_KERNEL)
            NNLOG (LOG_DEBUG,
                  "netlinkRouteMultipath() (recursive, 1 hop): "
                  "%s %s/%d, type %s \n", lookup (NL_MSG_STR, cmd),
#ifdef HAVE_IPV6
                 (family == AF_INET) ? inet_ntoa (pPrefix->u.prefix4) :
                 inet6_ntoa (pPrefix->u.prefix6),
#else
                 inet_ntoa (pPrefix->u.prefix4),
#endif /* HAVE_IPV6 */
                 pPrefix->prefixLen, NH_TYPES_DESC[pNextHop->rType]);

          if (pNextHop->rType == NEXTHOP_TYPE_IPV4 || 
              pNextHop->rType == NEXTHOP_TYPE_IPV4_IFINDEX)
          {
            addattr_l (&req.n, sizeof req, RTA_GATEWAY,
                       &pNextHop->rGate.ipv4, byteLen);

            if (pNextHop->src.ipv4.s_addr)
            {
              addattr_l(&req.n, sizeof req, RTA_PREFSRC,
                        &pNextHop->src.ipv4, byteLen);
            }
            NNLOG (LOG_DEBUG, "netlinkRouteMultipath() (recursive, "
                  "1 hop): nexthop via %s if %u \n",
                  inet_ntoa (pNextHop->rGate.ipv4), pNextHop->rifIndex);
          }
#ifdef HAVE_IPV6
          if (pNextHop->rType == NEXTHOP_TYPE_IPV6 || 
              pNextHop->rType == NEXTHOP_TYPE_IPV6_IFINDEX || 
              pNextHop->rType == NEXTHOP_TYPE_IPV6_IFNAME)
          {
            addattr_l (&req.n, sizeof req, RTA_GATEWAY,
                       &pNextHop->rGate.ipv6, byteLen);
            if (IS_RIBMGR_DEBUG_KERNEL)
              NNLOG (LOG_DEBUG, "netlinkRouteMultipath() (recursive, "
                    "1 hop): nexthop via %s if %u \n",
                    inet6_ntoa (pNextHop->rGate.ipv6), pNextHop->rifIndex);
          }
#endif /* HAVE_IPV6 */
          if (pNextHop->rType == NEXTHOP_TYPE_IFINDEX || 
              pNextHop->rType == NEXTHOP_TYPE_IFNAME || 
              pNextHop->rType == NEXTHOP_TYPE_IPV4_IFINDEX || 
              pNextHop->rType == NEXTHOP_TYPE_IPV6_IFINDEX || 
              pNextHop->rType == NEXTHOP_TYPE_IPV6_IFNAME)
          {
            addattr32 (&req.n, sizeof req, RTA_OIF,
                       pNextHop->rifIndex);
            if ((pNextHop->rType == NEXTHOP_TYPE_IPV4_IFINDEX || 
                 pNextHop->rType == NEXTHOP_TYPE_IFINDEX) && 
                pNextHop->src.ipv4.s_addr)
            {
              addattr_l (&req.n, sizeof req, RTA_PREFSRC,
                         &pNextHop->src.ipv4, byteLen);
            }

            NNLOG (LOG_DEBUG, "netlinkRouteMultipath() (recursive, "
                  "1 hop): nexthop via if %u\n", pNextHop->rifIndex);
          }
        }
        else
        {
          if (IS_RIBMGR_DEBUG_KERNEL)
            NNLOG (LOG_DEBUG,
                  "netlinkRouteMultipath() (single hop): "
                  "%s %s/%d, type %s \n", lookup (NL_MSG_STR, cmd),
#ifdef HAVE_IPV6
                (family == AF_INET) ? inet_ntoa (pPrefix->u.prefix4) :
                inet6_ntoa (pPrefix->u.prefix6),
#else
                inet_ntoa (pPrefix->u.prefix4),
#endif /* HAVE_IPV6 */
                pPrefix->prefixLen, NH_TYPES_DESC[pNextHop->type]);

          if (pNextHop->type == NEXTHOP_TYPE_IPV4 || 
              pNextHop->type == NEXTHOP_TYPE_IPV4_IFINDEX)
          {
            addattr_l (&req.n, sizeof req, RTA_GATEWAY,
                       &pNextHop->gate.ipv4, byteLen);
            if (pNextHop->src.ipv4.s_addr)
            {
              addattr_l (&req.n, sizeof req, RTA_PREFSRC,
                         &pNextHop->src.ipv4, byteLen);
            }

            if (IS_RIBMGR_DEBUG_KERNEL)
              NNLOG (LOG_DEBUG, "netlinkRouteMultipath() (single hop): "
                    "nexthop via %s if %u \n",
                  inet_ntoa (pNextHop->gate.ipv4), pNextHop->ifIndex);
          }
#ifdef HAVE_IPV6
          if (pNextHop->type == NEXTHOP_TYPE_IPV6 || 
              pNextHop->type == NEXTHOP_TYPE_IPV6_IFNAME || 
              pNextHop->type == NEXTHOP_TYPE_IPV6_IFINDEX)
          {
            addattr_l (&req.n, sizeof req, RTA_GATEWAY,
                       &pNextHop->gate.ipv6, byteLen);

            if (IS_RIBMGR_DEBUG_KERNEL)
              NNLOG (LOG_DEBUG, "netlinkRouteMultipath() (single hop): "
                    "nexthop via %s if %u \n",
                    inet6_ntoa (pNextHop->gate.ipv6), pNextHop->ifIndex);
          }
#endif /* HAVE_IPV6 */
          if (pNextHop->type == NEXTHOP_TYPE_IFINDEX || 
              pNextHop->type == NEXTHOP_TYPE_IFNAME || 
              pNextHop->type == NEXTHOP_TYPE_IPV4_IFINDEX)
          {
		    addattr32 (&req.n, sizeof req, RTA_OIF, pNextHop->ifIndex);

            if (pNextHop->src.ipv4.s_addr)
            {
              addattr_l (&req.n, sizeof req, RTA_PREFSRC,
                         &pNextHop->src.ipv4, byteLen);
            }

            if (IS_RIBMGR_DEBUG_KERNEL)
              NNLOG (LOG_DEBUG, "netlinkRouteMultipath() (single hop): "
                    "nexthop via if %u \n", pNextHop->ifIndex);
          }
          else if (pNextHop->type == NEXTHOP_TYPE_IPV6_IFINDEX || 
                   pNextHop->type == NEXTHOP_TYPE_IPV6_IFNAME)
          {
            addattr32 (&req.n, sizeof req, RTA_OIF, pNextHop->ifIndex);

            if (IS_RIBMGR_DEBUG_KERNEL)
              NNLOG (LOG_DEBUG, "netlinkRouteMultipath() (single hop): "
                    "nexthop via if %u \n", pNextHop->ifIndex);
          }
          else
          {
            NNLOG (LOG_WARNING, "Unknown case happened.\n");
          }
        }

        if (cmd == RTM_NEWROUTE)
        {
          SET_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB);
        }

        nextHopNum++;
        break;
      }
    }
  }
  else
  {
    char buf[1024] = {};
    struct rtattr *rta = (void *) buf;
    struct rtnexthop *rtnh = NULL;
    uGwAddrT *src = NULL;

    rta->rta_type = RTA_MULTIPATH;
    rta->rta_len = RTA_LENGTH (0);
    rtnh = RTA_DATA (rta);

    nextHopNum = 0;
    for (pNextHop = pRib->pNexthop;
         pNextHop && (MULTIPATH_NUM == 0 || nextHopNum < MULTIPATH_NUM);
         pNextHop = pNextHop->next)
    {
      if ((cmd == RTM_NEWROUTE && 
           CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_ACTIVE)) || 
          (cmd == RTM_DELROUTE && 
           CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB)))
      {
        nextHopNum++;

        rtnh->rtnh_len = sizeof (*rtnh);
        rtnh->rtnh_flags = 0;
        rtnh->rtnh_hops = 0;
        rta->rta_len += rtnh->rtnh_len;

        if (CHECK_FLAG (pNextHop->flags, NEXTHOP_FLAG_RECURSIVE))
        {
          if (IS_RIBMGR_DEBUG_KERNEL)
            NNLOG (LOG_DEBUG, "netlinkRouteMultipath() "
                  "(recursive, multihop): %s %s/%d type %s \n",
                  lookup (NL_MSG_STR, cmd),
#ifdef HAVE_IPV6
                (family == AF_INET) ? inet_ntoa (pPrefix->u.prefix4) :
                inet6_ntoa (pPrefix->u.prefix6),
#else
                inet_ntoa (pPrefix->u.prefix4),
#endif /* HAVE_IPV6 */
                pPrefix->prefixLen, NH_TYPES_DESC[pNextHop->rType]);
          if (pNextHop->rType == NEXTHOP_TYPE_IPV4 || 
              pNextHop->rType == NEXTHOP_TYPE_IPV4_IFINDEX)
          {
            rta_addattr_l (rta, 4096, RTA_GATEWAY,
                           &pNextHop->rGate.ipv4, byteLen);
            rtnh->rtnh_len += sizeof (struct rtattr) + 4;

            if (pNextHop->src.ipv4.s_addr)
            {
              src = &pNextHop->src;
            }

            if (IS_RIBMGR_DEBUG_KERNEL)
              NNLOG (LOG_DEBUG, "netlinkRouteMultipath() (recursive, "
                    "multihop): nexthop via %s if %u \n",
                    inet_ntoa (pNextHop->rGate.ipv4), pNextHop->rifIndex);
          }
#ifdef HAVE_IPV6
          if (pNextHop->rType == NEXTHOP_TYPE_IPV6 || 
              pNextHop->rType == NEXTHOP_TYPE_IPV6_IFNAME || 
              pNextHop->rType == NEXTHOP_TYPE_IPV6_IFINDEX)
          {
            rta_addattr_l (rta, 4096, RTA_GATEWAY,
                           &pNextHop->rGate.ipv6, byteLen);

            if (IS_RIBMGR_DEBUG_KERNEL)
              NNLOG (LOG_DEBUG, "netlinkRouteMultipath() (recursive, "
                    "multihop): nexthop via %s if %u \n",
                    inet6_ntoa (pNextHop->rGate.ipv6), pNextHop->rifIndex);
          }
#endif /* HAVE_IPV6 */
          /* ifindex */
          if (pNextHop->rType == NEXTHOP_TYPE_IPV4_IFINDEX || 
              pNextHop->rType == NEXTHOP_TYPE_IFINDEX || 
              pNextHop->rType == NEXTHOP_TYPE_IFNAME)
          {
            rtnh->rtnh_ifindex = pNextHop->rifIndex;
            if (pNextHop->src.ipv4.s_addr)
            {
              src = &pNextHop->src;
            }

            if (IS_RIBMGR_DEBUG_KERNEL)
              NNLOG (LOG_DEBUG, "netlinkRouteMultipath() (recursive, "
                    "multihop): nexthop via if %u \n",
                    pNextHop->rifIndex);
          }
          else if (pNextHop->rType == NEXTHOP_TYPE_IPV6_IFINDEX || 
                   pNextHop->rType == NEXTHOP_TYPE_IPV6_IFNAME)
          {
            rtnh->rtnh_ifindex = pNextHop->rifIndex;

            if (IS_RIBMGR_DEBUG_KERNEL)
              NNLOG (LOG_DEBUG, "netlinkRouteMultipath() (recursive, "
                    "multihop): nexthop via if %u \n", pNextHop->rifIndex);
          }
          else
		  {
            rtnh->rtnh_ifindex = 0;
		  }
        }
        else
        {
          if (IS_RIBMGR_DEBUG_KERNEL)
            NNLOG (LOG_DEBUG, "netlinkRouteMultipath() (multihop): "
                  "%s %s/%d, type %s \n", lookup (NL_MSG_STR, cmd),
#ifdef HAVE_IPV6
                  (family == AF_INET) ? inet_ntoa (pPrefix->u.prefix4) :
                  inet6_ntoa (pPrefix->u.prefix6),
#else
                  inet_ntoa (pPrefix->u.prefix4),
#endif /* HAVE_IPV6 */
                  pPrefix->prefixLen, NH_TYPES_DESC[pNextHop->type]);

          if (pNextHop->type == NEXTHOP_TYPE_IPV4 || 
              pNextHop->type == NEXTHOP_TYPE_IPV4_IFINDEX)
          {
            rta_addattr_l (rta, 4096, RTA_GATEWAY,
                           &pNextHop->gate.ipv4, byteLen);
            rtnh->rtnh_len += sizeof (struct rtattr) + 4;

            if (pNextHop->src.ipv4.s_addr)
            {
              src = &pNextHop->src;
            }

            if (IS_RIBMGR_DEBUG_KERNEL)
              NNLOG (LOG_DEBUG, "netlinkRouteMultipath() (multihop): "
                    "nexthop via %s if %u \n",
                    inet_ntoa (pNextHop->gate.ipv4), pNextHop->ifIndex);
          }
#ifdef HAVE_IPV6
          if (pNextHop->type == NEXTHOP_TYPE_IPV6 || 
              pNextHop->type == NEXTHOP_TYPE_IPV6_IFNAME || 
              pNextHop->type == NEXTHOP_TYPE_IPV6_IFINDEX)
          { 
            rta_addattr_l (rta, 4096, RTA_GATEWAY,
                           &pNextHop->gate.ipv6, byteLen);

            if (IS_RIBMGR_DEBUG_KERNEL)
              NNLOG (LOG_DEBUG, "netlinkRouteMultipath() (multihop): "
                    "nexthop via %s if %u \n",
                    inet6_ntoa (pNextHop->gate.ipv6), pNextHop->ifIndex);
          }
#endif /* HAVE_IPV6 */
          /* ifindex */
          if (pNextHop->type == NEXTHOP_TYPE_IPV4_IFINDEX || 
              pNextHop->type == NEXTHOP_TYPE_IFINDEX || 
              pNextHop->type == NEXTHOP_TYPE_IFNAME)
          {
            rtnh->rtnh_ifindex = pNextHop->ifIndex;
            if (pNextHop->src.ipv4.s_addr)
            {
              src = &pNextHop->src;
            }

            if (IS_RIBMGR_DEBUG_KERNEL)
              NNLOG (LOG_DEBUG, "netlinkRouteMultipath() (multihop): "
                    "nexthop via if %u \n", pNextHop->ifIndex);
          }
          else if (pNextHop->type == NEXTHOP_TYPE_IPV6_IFNAME || 
                   pNextHop->type == NEXTHOP_TYPE_IPV6_IFINDEX)
          {
            rtnh->rtnh_ifindex = pNextHop->ifIndex;

            if (IS_RIBMGR_DEBUG_KERNEL)
              NNLOG (LOG_DEBUG, "netlinkRouteMultipath() (multihop): "
                    "nexthop via if %u \n", pNextHop->ifIndex);
          }
          else
          {
            rtnh->rtnh_ifindex = 0;
          }
        }
        rtnh = RTNH_NEXT (rtnh);

        if (cmd == RTM_NEWROUTE)
        {
          SET_FLAG (pNextHop->flags, NEXTHOP_FLAG_FIB);
        }
      }
    }
    if (src)
    {
      addattr_l (&req.n, sizeof req, RTA_PREFSRC, &src->ipv4, byteLen);
    }

    if (rta->rta_len > RTA_LENGTH (0))
    {
      addattr_l (&req.n, 1024, RTA_MULTIPATH, RTA_DATA (rta),
                 RTA_PAYLOAD (rta));
    }
  }

  /* If there is no useful nexthop then return. */
  if (nextHopNum == 0)
  {
    if (IS_RIBMGR_DEBUG_KERNEL)
      NNLOG (LOG_DEBUG, "netlinkRouteMultipath(): No useful nexthop.\n");

    return 0;
  }

skip:

  /* Destination netlink address. */
  memset (&snl, 0, sizeof snl);
  snl.nl_family = AF_NETLINK;

  /* Talk to netlink socket. */
  return netlinkTalk (&req.n, &pRibmgr->NETLINK_CMD);
}


/* Description : Netlink 소켓을 이용하여 Kernel로 IPv4 루트 설정을 요청하는 함수. */
Int32T
kernelAddIpv4 (PrefixT *pPrefix, RibT *pRib)
{
  return netlinkRouteMultipath (RTM_NEWROUTE, pPrefix, pRib, AF_INET);
}


/* Description : Netlink 소켓을 이용하여 Kernel로 IPv4 루트 삭제를 요청하는 함수. */
Int32T
kernelDeleteIpv4 (PrefixT *pPrefix, RibT *pRib)
{
  return netlinkRouteMultipath (RTM_DELROUTE, pPrefix, pRib, AF_INET);
}


#ifdef HAVE_IPV6
/* Description : Netlink 소켓을 이용하여 Kernel로 IPv6 루트 설정을 요청하는 함수. */
Int32T
kernelAddIpv6 (PrefixT *pPrefix, RibT *pRib)
{
  return netlinkRouteMultipath (RTM_NEWROUTE, pPrefix, pRib, AF_INET6);
}

/* Description : Netlink 소켓을 이용하여 Kernel로 IPv6 루트 삭제를 요청하는 함수. */
Int32T
kernelDeleteIpv6 (PrefixT *pPrefix, RibT *pRib)
{
  return netlinkRouteMultipath (RTM_DELROUTE, pPrefix, pRib, AF_INET6);
}


/* Description : Netlink 소켓을 이용하여 Kernel로 IPv6 루트 삭제를 요청하는 함수. */
Int32T
kernelDeleteIpv6Old (Prefix6T *pDest, struct in6_addr *pGate,
                        Uint32T idx, Int32T flags, Int32T table)
{
  return netlinkRoute (RTM_DELROUTE, AF_INET6, &pDest->prefix,
                       pDest->prefixLen, pGate, idx, flags, table);
}
#endif /* HAVE_IPV6 */


/*
 * Description : 인터페이스 주소 수정
 *
 * param [in] cmd : 명령타입
 * param [in] family : 네트워크 패밀리
 * param [in] pIf : interface 자료구조 포인터
 * param [in] pIfc : connected 자료구조 포인터
 *
 * retval : -1 if 실패,
 *           0 if 성공
 */
static Int32T
netlinkAddress (Int32T cmd, Int32T family, InterfaceT *pIf,
                ConnectedT *pIfc)
{
  Int32T byteLen = 0;
  PrefixT *pPrefix = NULL;

  struct
  {
    struct nlmsghdr n;
    struct ifaddrmsg ifa;
    char buf[1024];
  } req;

  pPrefix = pIfc->pAddress;
  memset (&req, 0, sizeof req);

  byteLen = (family == AF_INET ? 4 : 16);

  req.n.nlmsg_len = NLMSG_LENGTH (sizeof (struct ifaddrmsg));
  req.n.nlmsg_flags = NLM_F_REQUEST;
  req.n.nlmsg_type = cmd;
  req.ifa.ifa_family = family;

  req.ifa.ifa_index = pIf->ifIndex;
  req.ifa.ifa_prefixlen = pPrefix->prefixLen;

  if (family == AF_INET)
  {
    addattr_l (&req.n, sizeof req, IFA_LOCAL, &pPrefix->u.prefix4, byteLen);
  }
#ifdef HAVE_IPV6
  else if (family == AF_INET6)
  {
    addattr_l (&req.n, sizeof req, IFA_LOCAL, &pPrefix->u.prefix6, byteLen);
  }
#endif
  else
  {
    NNLOG (LOG_ERR, "Wrong family = %d\n", family);
    return -1;
  }

  if (family == AF_INET && cmd == RTM_NEWADDR)
  {
    if (!CONNECTED_PEER(pIfc) && pIfc->pDestination)
    {
      pPrefix = pIfc->pDestination;
      addattr_l (&req.n, sizeof req, IFA_BROADCAST, &pPrefix->u.prefix4,
                 byteLen);
    }
  }

  if (CHECK_FLAG (pIfc->flags, RIBMGR_IFA_SECONDARY))
  {
    SET_FLAG (req.ifa.ifa_flags, IFA_F_SECONDARY);
  }

  if (pIfc->label)
  {
    addattr_l (&req.n, sizeof req, IFA_LABEL, pIfc->label,
               strlen (pIfc->label) + 1);
  }

  return netlinkTalk (&req.n, &pRibmgr->NETLINK_CMD);
}


/* Description : Netlink 소켓을 이용하여 Kernel로 IPv4 주소 설정을 요청하는 함수. */
Int32T
kernelAddressAddIpv4 (InterfaceT *pIf, ConnectedT *pIfc)
{
  return netlinkAddress (RTM_NEWADDR, AF_INET, pIf, pIfc);
}


/* Description : Netlink 소켓을 이용하여 Kernel로 IPv4 주소 삭제를 요청하는 함수. */
Int32T
kernelAddressDeleteIpv4 (InterfaceT *pIf, ConnectedT *pIfc)
{
  return netlinkAddress (RTM_DELADDR, AF_INET, pIf, pIfc);
}



/*
 * Description : Netlink 소켓 필터
 * (Filter out messages from self that occur on listener socket,
 *  caused by our actions on the command socket)
 *
 * param [in] sock : 소켓번호
 * param [in] pid : 프로세스 ID
 */
static void 
netlinkInstallFilter (Int32T sock, __u32 pid)
{
  struct sock_filter filter[] = {
    /* 0: ldh [4]	          */
    BPF_STMT(BPF_LD|BPF_ABS|BPF_H, offsetof(struct nlmsghdr, nlmsg_type)),
    /* 1: jeq 0x18 jt 3 jf 6  */
    BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, htons(RTM_NEWROUTE), 1, 0),
    /* 2: jeq 0x19 jt 3 jf 6  */
    BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, htons(RTM_DELROUTE), 0, 3),
    /* 3: ldw [12]		  */
    BPF_STMT(BPF_LD|BPF_ABS|BPF_W, offsetof(struct nlmsghdr, nlmsg_pid)),
    /* 4: jeq XX  jt 5 jf 6   */
    BPF_JUMP(BPF_JMP|BPF_JEQ|BPF_K, htonl(pid), 0, 1),
    /* 5: ret 0    (skip)     */
    BPF_STMT(BPF_RET|BPF_K, 0),
    /* 6: ret 0xffff (keep)   */
    BPF_STMT(BPF_RET|BPF_K, 0xffff),
  };

  struct sock_fprog prog = {
    .len = sizeof(filter) / sizeof(filter[0]),
    .filter = filter,
  };

  if (setsockopt(sock, SOL_SOCKET, SO_ATTACH_FILTER, &prog, sizeof(prog)) < 0)
  {
    NNLOG (LOG_WARNING, "Can't install socket filter: %s\n", safeStrError(errno));
  }
}


/*
 * Description : Netlink 소켓으로 커널에서 메시지 수신시 호출되는 콜백 함수.
 *
 * param [in] x : dummy
 * param [in] y : dummy
 * param [in] y : argument
 *
 * retval : none
 */
void readNetlinkCB(Int32T x, Int16T y, void *pargs)
{
  if(netlinkParseInfo (netlinkInformationFetch, &pRibmgr->NETLINK) < 0)
  {
    NNLOG (LOG_ERR, "Error] netlinkParseInfo retrun value\n");
  }
}


/*
 * Description : Kernel과 통신을 위한 Netlink 소켓을 생성 하고, 
 * 콜백함수를 등록하는 함수.
 */
void
kernelInit (void)
{
  Uint32T groups;

  /* Create Netlink Falgs. */
  groups = RTMGRP_LINK | RTMGRP_IPV4_ROUTE | RTMGRP_IPV4_IFADDR;
#ifdef HAVE_IPV6
  groups |= RTMGRP_IPV6_ROUTE | RTMGRP_IPV6_IFADDR;
#endif /* HAVE_IPV6 */

  /* Create Netlink Socket. */
  netlinkSocket (&pRibmgr->NETLINK, groups);
  netlinkSocket (&pRibmgr->NETLINK_CMD, 0);

  /* Register kernel socket. */
  if (pRibmgr->NETLINK.sock > 0)
  {
    /* Only want non-blocking on the netlink event socket. */
    if (fcntl (pRibmgr->NETLINK.sock, F_SETFL, O_NONBLOCK) < 0)
    {
      NNLOG (LOG_ERR, "Error] Can't set %s socket flags: %s \n", 
            pRibmgr->NETLINK.name,
      safeStrError (errno));
    }

    /* Set receive buffer size if it's set from command line. */
    if (nlRcvBuffSize)
    {
      netlinkRecvBuff (&pRibmgr->NETLINK, nlRcvBuffSize);
    }

    /* Set Netlink Filter. */
    netlinkInstallFilter (pRibmgr->NETLINK.sock, 
                          pRibmgr->NETLINK_CMD.snl.nl_pid);
   
    /* This Add Socket File Discriptor, and register Callback Function. */
    pRibmgr->pNetlinkFdEvent = taskFdSet(readNetlinkCB, 
                                         pRibmgr->NETLINK.sock, 
                                         TASK_READ | TASK_PERSIST, 
                                         TASK_PRI_MIDDLE, NULL);
  }
  else
  {
    NNLOG (LOG_ERR, "Error] netlink.sock=%d\n", pRibmgr->NETLINK.sock);
  }
}

/*
 * Description : Kernel과 통신을 위한 기존에 생성한 Netlink 소켓에, 
 * 콜백함수를 갱신하는 함수.
 */
void
kernelUpdate (void)
{
  /* This Add Socket File Discriptor, and register Callback Function. */
  taskFdUpdate(readNetlinkCB, pRibmgr->pNetlinkFdEvent, NULL);
}

