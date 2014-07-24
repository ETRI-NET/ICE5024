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
 * - Block Name : PIF Manager
 * - Process Name : pifmgr
 * - Creator : Suncheul Kim, ported by Seungwoo Hong
 * - Initial Date : 2014/03/17
 */

/**
 * @file        : 
 *
 * $Author: $
 * $Date:  $
 * $Revision:  $
 * $LastChangedBy: $
 */

#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include <linux/rtnetlink.h>
#include <linux/filter.h>
#include "palKernelNetlink.h"
#include "palKernel.h"
#include "interface.h"
#include "connected.h"


/* Hack for GNU libc version 2. */
#ifndef MSG_TRUNC
#define MSG_TRUNC      0x20
#endif /* MSG_TRUNC */


/* Receive buffer size for netlink socket */
Uint32T nlRcvBuffSize = 0;

#define MULTIPATH_NUM 1

/* Socket interface to kernel */
struct nlsock
{
  Int32T sock;
  Int32T seq;
  struct sockaddr_nl snl;
  const char *name;
} NETLINK      = { -1, 0, {0}, "netlink-listen"},     /* kernel messages */
  NETLINK_CMD  = { -1, 0, {0}, "netlink-cmd"};        /* command channel */


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


/*
 * Description : 문자열을 버퍼 크기만큼 복사하는 함수.
 *
 * param [in] d : 문자열 포인터
 * param [in] s : 문자열 포인터
 * param [in] bufsize : 버퍼 길이
 *
 * retval : 복사한 문자열 길이
 */
size_t
strlcpy(char *d, const char *s, size_t bufsize)
{
  size_t len = strlen(s);
  size_t ret = len;
  if (bufsize > 0) {
    if (len >= bufsize)
      len = bufsize-1;
    memcpy(d, s, len);
    d[len] = 0;
  }
  return ret;
}


/*
 * Description : 두 문자열을 합치는 함수.
 *
 * param [in] d : 문자열 포인터
 * param [in] s : 문자열 포인터
 * param [in] bufsize : 문자열 버퍼 길이
 *
 * retval : 복사한 문자열 길이
 */
size_t
strlcat(char *d, const char *s, size_t bufsize)
{
  size_t len1 = strlen(d);
  size_t len2 = strlen(s);
  size_t ret = len1 + len2;

  if (len1 < bufsize - 1) 
  {
    if (len2 >= bufsize - len1)
      len2 = bufsize - len1 - 1;
    memcpy(d+len1, s, len2);
    d[len1+len2] = 0;
  }
  return ret;
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

/*
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
*/



/*
 * Description : 인터페이스 인덱스를 설정하는 함수
 * (Note: on netlink systems, there should be a 1-to-1 mapping between 
 *  interface names and ifindex values.)
 *
 * param [in] ifp : interface 자료구조 포인터
 * param [in] ifc : connected 자료구조 포인터
 */
static void
setIfindex(InterfaceT *ifp, Uint32T ifi_index)
{
  InterfaceT *oifp;

  klog(KER_DEBUG, "%s : ifi_index = %d\n", __func__, ifi_index);

  if (((oifp = ifLookupByIndex(ifi_index)) != NULL) && (oifp != ifp))
  {
    if (ifi_index == IFINDEX_INTERNAL)
        klog(KER_DEBUG, 
              "Err : Netlink is setting interface %s ifindex to reserved "
              "internal value %u\n", ifp->name, ifi_index);
    else
    {
      klog(KER_DEBUG, 
            "interface index %d was renamed from %s to %s\n",
            ifi_index, oifp->name, ifp->name);
	  if (ifIsUp(oifp))
        klog(KER_DEBUG, 
              "Err : interface rename detected on up interface: index %d "
              "was renamed from %s to %s, results are uncertain!\n", 
              ifi_index, oifp->name, ifp->name);

      kernelEventIfDel(oifp);
    }
  }
  ifp->ifIndex = ifi_index;
}


/*
 * Description : Netlink 소켓의 수신 버퍼 크기를 설정하는 함수.
 *
 * param [in] nl : nlsock 자료구조 포인터
 * param [in] newsize : 버퍼 크기
 *
 * retval : -1 if 실패
 *           0 if 성공
 */
static Int32T
netlinkRecvBuff (struct nlsock *nl, Uint32T newsize)
{
  Uint32T oldsize;
  socklen_t newlen = sizeof(newsize);
  socklen_t oldlen = sizeof(oldsize);
  Int32T ret;

  ret = getsockopt(nl->sock, SOL_SOCKET, SO_RCVBUF, &oldsize, &oldlen);
  if (ret < 0)
    {
      klog(KER_DEBUG, "Can't get %s receive buffer size: %s\n", nl->name,
	    safeStrError (errno));
      return -1;
    }

  ret = setsockopt(nl->sock, SOL_SOCKET, SO_RCVBUF, &nlRcvBuffSize,
		   sizeof(nlRcvBuffSize));
  if (ret < 0)
    {
      klog(KER_DEBUG, "Can't set %s receive buffer size: %s\n", nl->name,
	    safeStrError (errno));
      return -1;
    }

  ret = getsockopt(nl->sock, SOL_SOCKET, SO_RCVBUF, &newsize, &newlen);
  if (ret < 0)
    {
      klog(KER_DEBUG, "Can't get %s receive buffer size: %s\n", nl->name,
	    safeStrError (errno));
      return -1;
    }

  klog(KER_DEBUG, 
        "Setting netlink socket receive buffer size: %u -> %u\n",
        oldsize, newsize);
  return 0;
}


/*
 * Description : Netlink 소켓을 생성하는 함수.
 *
 * param [in] nl : nlsock 자료구조 포인터
 * param [in] groups : Netlink 그룹정보
 *
 * retval : -1 if 실패
 *           0 if 성공
 */
static Int32T
netlinkSocket (struct nlsock *nl, Uint32T groups)
{
  Int32T ret;
  struct sockaddr_nl snl;
  Int32T sock;
  Int32T namelen;
  Int32T save_errno;

  sock = socket (AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock < 0)
  {
    klog(KER_DEBUG, "Can't open %s socket: %s\n", 
          nl->name, safeStrError (errno));
    return -1;
  }

  memset (&snl, 0, sizeof snl);
  snl.nl_family = AF_NETLINK;
  snl.nl_groups = groups;

  ret = bind (sock, (struct sockaddr *) &snl, sizeof snl);
  save_errno = errno;

  if (ret < 0)
  {
    klog(KER_DEBUG, "Can't bind %s socket to group 0x%x: %s\n",
          nl->name, snl.nl_groups, safeStrError (save_errno));
    close (sock);
    return -1;
  }

  /* multiple netlink sockets will have different nl_pid */
  namelen = sizeof snl;
  ret = getsockname (sock, (struct sockaddr *) &snl, (socklen_t *) &namelen);
  if (ret < 0 || namelen != sizeof snl)
  {
    klog(KER_DEBUG, "Can't get %s socket name: %s\n", 
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
 * param [in] nl : nlsock 자료구조 포인터
 *
 * retval : -1 if 실패
 *           0 if 성공
 */
static Int32T
netlinkRequest (Int32T family, Int32T type, struct nlsock *nl)
{
  Int32T ret;
  struct sockaddr_nl snl;
  Int32T save_errno;

  struct
  {
    struct nlmsghdr nlh;
    struct rtgenmsg g;
  } req;

  /* Check netlink socket. */
  if (nl->sock < 0)
  {
    klog(KER_DEBUG, "%s socket isn't active.\n", nl->name);
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
  save_errno = errno;

  if (ret < 0)
  {
    klog(KER_DEBUG, "%s sendto failed: %s\n", nl->name, safeStrError (save_errno));
    return -1;
  }

  return 0;
}


/*
 * Description : Netlink 소켓으로 부터 수신된 메시지를 파싱하는 함수.
 * 입력 변수인 filter 콜백 함수에서 처리하도록 되어 있다.
 *
 * param [in] filter : 콜백함수 포인터
 * param [in] nl : nlsock 자료구조 포인터
 *
 * retval : -1 if 실패
 *           0 if 성공
 */
static Int32T
netlinkParseInfo (Int32T (*filter) (struct sockaddr_nl *, struct nlmsghdr *),
                    struct nlsock *nl)
{
  Int32T status;
  Int32T ret = 0;
  Int32T error;

  while (1)
  {
    char buf[4096];
    struct iovec iov = { buf, sizeof buf };
    struct sockaddr_nl snl;
    struct msghdr msg = { (void *) &snl, sizeof snl, &iov, 1, NULL, 0, 0 };
    struct nlmsghdr *h;


    status = recvmsg (nl->sock, &msg, 0);
    //klog(KER_DEBUG, "NNNN recvmsg length = %d\n", status);

    if (status < 0)
    {
      if (errno == EINTR)
        continue;

      if (errno == EWOULDBLOCK || errno == EAGAIN)
        break;

      klog(KER_DEBUG, "NNNN %s recvmsg overrun: %s\n", nl->name, safeStrError(errno));
      continue;
    }

    if (status == 0)
    {
      klog(KER_DEBUG, "NNNN %s EOF\n", nl->name);
      return -1;
    }

    if (msg.msg_namelen != sizeof snl)
    {
      klog(KER_DEBUG, "NNNN %s sender address length error: length %d \n",
                nl->name, msg.msg_namelen);
      return -1;
    }
      
    for (h = (struct nlmsghdr *) buf; NLMSG_OK (h, (Uint32T) status);
         h = NLMSG_NEXT (h, status))
    {
      /* Finish of reading. */
      if (h->nlmsg_type == NLMSG_DONE)
        return ret;

      /* Error handling. */
      if (h->nlmsg_type == NLMSG_ERROR)
      {
        struct nlmsgerr *err = (struct nlmsgerr *) NLMSG_DATA (h);
	    Int32T errnum = err->error;
	    Int32T msg_type = err->msg.nlmsg_type;

        /* If the error field is zero, then this is an ACK */
        if (err->error == 0)
        {
          klog(KER_DEBUG, "NNNN %s: %s ACK: type=%s(%u), seq=%u, pid=%u\n",
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
          klog(KER_DEBUG, "NNNN %s error: message truncated\n", nl->name);
          return -1;
        }

        /* Deal with errors that occur because of races in link handling */
	    if (nl == &NETLINK_CMD && 
            ((msg_type == RTM_DELROUTE &&
		     (-errnum == ENODEV || -errnum == ESRCH)) || 
            (msg_type == RTM_NEWROUTE && -errnum == EEXIST)))
        {
          klog(KER_DEBUG, "NNNN %s: error: %s type=%s(%u), seq=%u, pid=%u\n",
				nl->name, safeStrError (-errnum),
				lookup (NL_MSG_STR, msg_type),
				msg_type, err->msg.nlmsg_seq, err->msg.nlmsg_pid);
          return 0;
        }

	    klog(KER_DEBUG, "NNNN %s error: %s, type=%s(%u), seq=%u, pid=%u\n",
              nl->name, safeStrError (-errnum),
              lookup (NL_MSG_STR, msg_type),
              msg_type, err->msg.nlmsg_seq, err->msg.nlmsg_pid);
        return -1;
      }

      /* OK we got netlink message. */
	  /*
      klog(KER_DEBUG, "netlink message: %s=%s(%u), seq=%u, pid=%u\n",
                       nl->name,
                       lookup (NL_MSG_STR, h->nlmsg_type), h->nlmsg_type,
                       h->nlmsg_seq, h->nlmsg_pid);
	  */

      /* skip unsolicited messages originating from command socket */
      if (nl != &NETLINK_CMD && h->nlmsg_pid == NETLINK_CMD.snl.nl_pid)
      {
        klog(KER_DEBUG, "NNNN netlinkParseInfo: %s packet comes from %s\n",
                            NETLINK_CMD.name, nl->name);
        continue;
      }
      error = (*filter) (&snl, h);
      if (error < 0)
      {
        klog(KER_DEBUG, "NNNN %s filter function error\n", nl->name);
        ret = error;
      }
    }
	// for each messages

    /* After error care. */
    if (msg.msg_flags & MSG_TRUNC)
    {
      klog(KER_DEBUG, "NNNN %s error: message truncated\n", nl->name);
      continue;
    }
    if (status)
    {
      klog(KER_DEBUG, "NNNN %s error: data remnant size %d\n", nl->name, status);
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
        tb[rta->rta_type] = rta;
      rta = RTA_NEXT (rta, len);
    }
}


/*
 * Description : Netlink 메시지 중 인터페이스 정보를 처리하는 함수.
 * param [in] snl : sockaddr_nl 자료구조 포인터
 * param [in] h : nlmsghdr 자료구조 포인터
 *
 * retval : -1 if 실패
 *           0 if 성공
 */
extern Int32T kernel_if_get_bw(InterfaceT *ifp);
static Int32T
netlinkInterface (struct sockaddr_nl *snl, struct nlmsghdr *h)
{
  Int32T len;
  struct ifinfomsg *ifi;
  struct rtattr *tb[IFLA_MAX + 1];
  InterfaceT *ifp;
  char *name;
  Int32T i;

  ifi = NLMSG_DATA (h);

  if (h->nlmsg_type != RTM_NEWLINK)
    return 0;

  len = h->nlmsg_len - NLMSG_LENGTH (sizeof (struct ifinfomsg));
  if (len < 0)
    return -1;

  /* Looking up interface name. */
  memset (tb, 0, sizeof tb);
  netlinkParseRtattr (tb, IFLA_MAX, IFLA_RTA (ifi), len);
  
#ifdef IFLA_WIRELESS
  /* check for wireless messages to ignore */
  if ((tb[IFLA_WIRELESS] != NULL) && (ifi->ifi_change == 0))
    {
      klog (KER_DEBUG, "%s: ignoring IFLA_WIRELESS message\n", __func__);
      return 0;
    }
#endif /* IFLA_WIRELESS */

  if (tb[IFLA_IFNAME] == NULL)
    return -1;
  name = (char *) RTA_DATA (tb[IFLA_IFNAME]);

  /* Add interface. */
  ifp = ifGetByName (name);

  if(ifp == NULL)
  {
    klog(KER_DEBUG, "If Name is NULL\n");
  }else
  {
    klog(KER_DEBUG, "If Name is %s\n", name);
  }

  //setIfindex(ifp, ifi->ifi_index);
  /* Base interface info */
  ifp->ifIndex = ifi->ifi_index;
  ifp->flags = ifi->ifi_flags & 0x0000fffff;
  ifp->mtu6 = ifp->mtu = *(Uint32T *) RTA_DATA (tb[IFLA_MTU]);
  ifp->metric = 1;

  /* Hardware type */
  ifp->hwType = pal_if_type(ifi->ifi_type);

  /* Hardware address */
  if (tb[IFLA_ADDRESS])
  {
    Int32T hwAddrLen;
    hwAddrLen = RTA_PAYLOAD (tb[IFLA_ADDRESS]);

    if (hwAddrLen > INTERFACE_HWADDR_MAX)
      klog(KER_DEBUG, "Hardware address is too large: %d\n", hwAddrLen);
    else
    {
      ifp->hwAddrLen = hwAddrLen;
      memcpy (ifp->hwAddr, RTA_DATA (tb[IFLA_ADDRESS]), hwAddrLen);

      for (i = 0; i < hwAddrLen; i++)
        if (ifp->hwAddr[i] != 0)
          break;

      if (i == hwAddrLen)
        ifp->hwAddrLen = 0;
      else
        ifp->hwAddrLen = hwAddrLen;
    }
  }

  /* bandwidth */
  kernel_if_get_bw(ifp);

  /* Notice PIF a new interface.  */
  kernelEventIfAdd (ifp);

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
  Int32T len;
  struct ifaddrmsg *ifa;
  struct rtattr *tb[IFA_MAX + 1];
  InterfaceT *ifp;
  void *addr;
  void *broad;
  u_char flags = 0;
  char *label = NULL;

  /*
  klog(KER_DEBUG, "-----------------------------------------\n");
  klog(KER_DEBUG, "NETLINK:: Interface Address \n");
  */
  klog(KER_DEBUG, "-----------------------------------------\n");

  ifa = NLMSG_DATA (h);

  if (ifa->ifa_family != AF_INET
#ifdef HAVE_IPV6
      && ifa->ifa_family != AF_INET6
#endif /* HAVE_IPV6 */
    )
    return 0;

  if (h->nlmsg_type != RTM_NEWADDR && h->nlmsg_type != RTM_DELADDR)
  {
    klog(KER_DEBUG, "invalid RTM Types %d\n", h->nlmsg_type);
    return 0;
  }

  len = h->nlmsg_len - NLMSG_LENGTH (sizeof (struct ifaddrmsg));
  if (len < 0)
  {
    klog(KER_DEBUG, "invalid NLMSG len %d\n", h->nlmsg_len);
    return -1;
  }

  memset (tb, 0, sizeof tb);
  netlinkParseRtattr (tb, IFA_MAX, IFA_RTA (ifa), len);

  ifp = ifLookupByIndex (ifa->ifa_index);
  if (ifp == NULL)
  {
    klog(KER_DEBUG, "netlinkInterfaceAddr can't find interface by index %d\n",
          ifa->ifa_index);
    return -1;
  }

  /*
  klog(KER_DEBUG, "lookup interface by idx(%d)->name(%s)\n", ifa->ifa_index, ifp->name);
  */

  char buf[BUFSIZ];
  klog(KER_DEBUG, "%s if-%s:\n",
        lookup (NL_MSG_STR, h->nlmsg_type), ifp->name);
  klog(KER_DEBUG, "-----------------------------------------\n");
  if (tb[IFA_LOCAL])
    klog(KER_DEBUG, "  IFA_LOCAL     %s/%d\n",
          inet_ntop (ifa->ifa_family, RTA_DATA (tb[IFA_LOCAL]),
          buf, BUFSIZ), ifa->ifa_prefixlen);
  if (tb[IFA_ADDRESS])
    klog(KER_DEBUG, "  IFA_ADDRESS   %s/%d\n",
          inet_ntop (ifa->ifa_family, RTA_DATA (tb[IFA_ADDRESS]),
                               buf, BUFSIZ), ifa->ifa_prefixlen);
  if (tb[IFA_BROADCAST])
    klog(KER_DEBUG, "  IFA_BROADCAST %s/%d\n",
          inet_ntop (ifa->ifa_family, RTA_DATA (tb[IFA_BROADCAST]),
          buf, BUFSIZ), ifa->ifa_prefixlen);
  if (tb[IFA_LABEL] && strcmp (ifp->name, RTA_DATA (tb[IFA_LABEL])))
    klog(KER_DEBUG, "  IFA_LABEL     %s\n", (char *)RTA_DATA (tb[IFA_LABEL]));
      
  if (tb[IFA_CACHEINFO])
  {
    struct ifa_cacheinfo *ci = RTA_DATA (tb[IFA_CACHEINFO]);
    klog(KER_DEBUG, "  IFA_CACHEINFO pref %d, valid %d\n",
          ci->ifa_prefered, ci->ifa_valid);
  }
  klog(KER_DEBUG, "-----------------------------------------\n");
  
  /* logic copied from iproute2/ip/ipaddress.c:print_addrinfo() */
  if (tb[IFA_LOCAL] == NULL)
    tb[IFA_LOCAL] = tb[IFA_ADDRESS];
  if (tb[IFA_ADDRESS] == NULL)
    tb[IFA_ADDRESS] = tb[IFA_LOCAL];
  
  /* local interface address */
  addr = (tb[IFA_LOCAL] ? RTA_DATA(tb[IFA_LOCAL]) : NULL);

  /* is there a peer address? */
  if (tb[IFA_ADDRESS] &&
      memcmp(RTA_DATA(tb[IFA_ADDRESS]), RTA_DATA(tb[IFA_LOCAL]), RTA_PAYLOAD(tb[IFA_ADDRESS])))
  {
      broad = RTA_DATA(tb[IFA_ADDRESS]);
      SET_FLAG (flags, IFA_PEER);
  }
  else
    /* seeking a broadcast address */
    broad = (tb[IFA_BROADCAST] ? RTA_DATA(tb[IFA_BROADCAST]) : NULL);

  /* addr is primary key, SOL if we don't have one */
  if (addr == NULL)
  {
    klog (KER_DEBUG, "%s: NULL address\n", __func__);
    return -1;
  }

  /* Flags. */
  if (ifa->ifa_flags & IFA_F_SECONDARY)
  {
    klog(KER_DEBUG, "Secondary Flag Set\n");
    SET_FLAG (flags, IFA_SECONDARY);
  }

  /* Label */
  if (tb[IFA_LABEL])
    label = (char *) RTA_DATA (tb[IFA_LABEL]);

  if (ifp && label && strcmp (ifp->name, label) == 0)
    label = NULL;

  /* Register interface address to the interface. */
  if (ifa->ifa_family == AF_INET)
  {
    if (h->nlmsg_type == RTM_NEWADDR)
      connectedAddIpv4 (ifp, flags,
                        (struct in_addr *) addr, ifa->ifa_prefixlen,
                        (struct in_addr *) broad, label);
    else
      connectedDeleteIpv4 (ifp, flags,
                           (struct in_addr *) addr, ifa->ifa_prefixlen,
                           (struct in_addr *) broad);
  }
#ifdef HAVE_IPV6
  if (ifa->ifa_family == AF_INET6)
  {
    if (h->nlmsg_type == RTM_NEWADDR)
      connectedAddIpv6 (ifp, flags,
                        (struct in6_addr *) addr, ifa->ifa_prefixlen,
                        (struct in6_addr *) broad, label);
    else
      connectedDeleteIpv6 (ifp,
                           (struct in6_addr *) addr, ifa->ifa_prefixlen,
                           (struct in6_addr *) broad, lablel);
  }
#endif /* HAVE_IPV6 */

  // display interface list
  //ifDumpAll();

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
 * Description : Kernel에서 인터페이스가 변경되었을 경우 호출되는 함수.
 *
 * param [in] snl : sockaddr_nl 자료구조 포인터
 * param [in] h : nlmsghdr 자료구조 포인터
 *
 * retval : -1 if 실패
 *           0 if 성공
 */
extern const char * ifTypeDump (Uint16T type);
extern const char * ifMacDump (Uint8T* mac);
static Int32T
netlinkLinkChange (struct sockaddr_nl *snl, struct nlmsghdr *h)
{
  Int32T len;
  struct ifinfomsg *ifi;
  struct rtattr *tb[IFLA_MAX + 1];
  InterfaceT *ifp;
  char *name;
 
  klog(KER_DEBUG, "------------------------------------\n");
  klog(KER_DEBUG, "%s called\n", __func__);
  klog(KER_DEBUG, "------------------------------------\n");

  ifi = NLMSG_DATA (h);

  if (!(h->nlmsg_type == RTM_NEWLINK || h->nlmsg_type == RTM_DELLINK))
  {
      /* If this is not link add/delete message so print warning. */
      klog(KER_DEBUG, "netlinkLinkChange: wrong kernel message %d\n",
                 h->nlmsg_type);
      return 0;
  }

  len = h->nlmsg_len - NLMSG_LENGTH (sizeof (struct ifinfomsg));
  if (len < 0)
    return -1;

  /* Looking up interface name. */
  memset (tb, 0, sizeof tb);
  netlinkParseRtattr (tb, IFLA_MAX, IFLA_RTA (ifi), len);

#ifdef IFLA_WIRELESS
  /* check for wireless messages to ignore */
  if ((tb[IFLA_WIRELESS] != NULL) && (ifi->ifi_change == 0))
  {
      klog(KER_DEBUG, "%s: ignoring IFLA_WIRELESS message\n", __func__);
      return 0;
  }
#endif /* IFLA_WIRELESS */
  
  if (tb[IFLA_IFNAME] == NULL)
    return -1;
  name = (char *) RTA_DATA (tb[IFLA_IFNAME]);

  klog(KER_DEBUG, "IF NAME = %s\n", name);

  /* Add interface. */
  if (h->nlmsg_type == RTM_NEWLINK)
  {
    ifp = ifLookupByName (name);

    // if (ifp == NULL || !CHECK_FLAG (ifp->status, INTERFACE_ACTIVE))
	
    /* Create New interface. */
    if (ifp == NULL)
    {
      klog(KER_DEBUG, "RTM_NEWLINK: create new interface: %s\n", name);
      if (ifp == NULL)
        ifp = ifGetByName (name);

      setIfindex(ifp, ifi->ifi_index);
      ifp->flags = ifi->ifi_flags & 0x0000fffff;
      ifp->mtu6 = ifp->mtu = *(Int32T *) RTA_DATA (tb[IFLA_MTU]);
      ifp->metric = 1;
	  ifp->hwType = pal_if_type(ifi->ifi_type);
	  ifp->hwAddrLen = RTA_PAYLOAD (tb[IFLA_ADDRESS]);
	  memcpy (ifp->hwAddr, RTA_DATA (tb[IFLA_ADDRESS]), ifp->hwAddrLen);
	  kernel_if_get_bw(ifp);

      klog(KER_DEBUG, "IF Flag = %s\n",  ifFlagDump(ifp->flags));
      klog(KER_DEBUG, "IF mtu  = %d\n",  ifp->mtu);
      klog(KER_DEBUG, "IF metr = %d\n",  ifp->metric);
      klog(KER_DEBUG, "IF type = %s\n",  ifTypeDump(ifp->hwType));
      klog(KER_DEBUG, "IF bw   = %d\n",  ifp->bandwidth);
      klog(KER_DEBUG, "IF addr = %s\n",  ifp->hwAddr);
      klog(KER_DEBUG, "------------------------------------\n");

      /* EVENT-IF-ADD */
      kernelEventIfAdd (ifp);

    }
    /* Interface state change */
    else
    {
      klog(KER_DEBUG, "RTM_NEWLINK: interface update(down/up): %s\n", name);
      /* Interface status change. */
      setIfindex(ifp, ifi->ifi_index);
      ifp->mtu6 = ifp->mtu = *(Int32T *) RTA_DATA (tb[IFLA_MTU]);
      ifp->metric = 1;

      if (ifIsOperative (ifp))
      {
        ifp->flags = ifi->ifi_flags & 0x0000fffff;
        if (!ifIsOperative (ifp))

          /* EVENT-IF-DOWN */
          kernelEventIfDown (ifp);
      }
      else
      {
        ifp->flags = ifi->ifi_flags & 0x0000fffff;
        if (ifIsOperative (ifp))

          /* EVENT-IF-UP */
          kernelEventIfUp (ifp);
      }
    }
  }

  /* Delete interface */
  else
  {
    // RTM_DELLINK.
    ifp = ifLookupByName (name);

    if (ifp == NULL)
    {
      klog(KER_DEBUG, "interface %s is deleted but can't find\n", name);
      return 0;
    }

    /* EVENT-IF-DELETE */
    kernelEventIfDel (ifp);

	ifDelete(ifp);
  }

  // display interface list
  //ifDumpAll();


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
  /*
  klog(KER_DEBUG, "%s, %d called...\n", __FUNCTION__, __LINE__);
  */

  /* JF: Ignore messages that aren't from the kernel */
  if ( snl->nl_pid != 0 )
  {
    klog(KER_DEBUG, "Ignoring message from pid %u\n", snl->nl_pid );
    return -1;
  }

  switch (h->nlmsg_type)
  {
    case RTM_NEWROUTE:
      //return netlinkRouteChange (snl, h);
      break;
    case RTM_DELROUTE:
      //return netlinkRouteChange (snl, h);
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
      klog(KER_DEBUG, "Unknown netlink nlmsg_type %d\n", h->nlmsg_type);
      break;
  }
  return 0;
}



/*
 * rtattr 처리하는 유틸리티 함수.
 */
static Int32T
addattr_l (struct nlmsghdr *n, Int32T maxlen, Int32T type, void *data, Int32T alen)
{
  Int32T len;
  struct rtattr *rta;

  len = RTA_LENGTH (alen);

  if (NLMSG_ALIGN (n->nlmsg_len) + len > maxlen)
    return -1;

  rta = (struct rtattr *) (((char *) n) + NLMSG_ALIGN (n->nlmsg_len));
  rta->rta_type = type;
  rta->rta_len = len;
  memcpy (RTA_DATA (rta), data, alen);
  n->nlmsg_len = NLMSG_ALIGN (n->nlmsg_len) + len;

  return 0;
}


/*
 * rtattr 처리하는 유틸리티 함수.
static Int32T
rta_addattr_l (struct rtattr *rta, Int32T maxlen, Int32T type, void *data, Int32T alen)
{
  Int32T len;
  struct rtattr *subrta;

  len = RTA_LENGTH (alen);

  if (RTA_ALIGN (rta->rta_len) + len > maxlen)
    return -1;

  subrta = (struct rtattr *) (((char *) rta) + RTA_ALIGN (rta->rta_len));
  subrta->rta_type = type;
  subrta->rta_len = len;
  memcpy (RTA_DATA (subrta), data, alen);
  rta->rta_len = NLMSG_ALIGN (rta->rta_len) + len;

  return 0;
}
 */

/*
 * rtattr 처리하는 유틸리티 함수.
static Int32T
addattr32 (struct nlmsghdr *n, Int32T maxlen, Int32T type, Int32T data)
{
  Int32T len;
  struct rtattr *rta;

  len = RTA_LENGTH (4);

  if (NLMSG_ALIGN (n->nlmsg_len) + len > maxlen)
    return -1;

  rta = (struct rtattr *) (((char *) n) + NLMSG_ALIGN (n->nlmsg_len));
  rta->rta_type = type;
  rta->rta_len = len;
  memcpy (RTA_DATA (rta), &data, 4);
  n->nlmsg_len = NLMSG_ALIGN (n->nlmsg_len) + len;

  return 0;
}
 */


static Int32T
netlinkTalkFilter (struct sockaddr_nl *snl, struct nlmsghdr *h)
{
  klog(KER_DEBUG, "netlinkTalk: ignoring message type 0x%04x \n", h->nlmsg_type);
  return 0;
}


/*
 * Description : Netlink 소켓으로 Kernel로 메시지 전송 하는 함수.
 * 이후 recvmsg()로 커널로부터 정보를 수신한다.
 *
 * param [in] n : nlmsghdr 자료구조 포인터
 * param [in] nl : nlsock 자료구조 포인터
 * 
 * retval : -1 if 실패
 *           0 if 성공
 */
static Int32T
netlinkTalk (struct nlmsghdr *n, struct nlsock *nl)
{
  Int32T status;
  struct sockaddr_nl snl;
  struct iovec iov = { (void *) n, n->nlmsg_len };
  struct msghdr msg = { (void *) &snl, sizeof snl, &iov, 1, NULL, 0, 0 };
  Int32T save_errno;

  memset (&snl, 0, sizeof snl);
  snl.nl_family = AF_NETLINK;

  n->nlmsg_seq = ++nl->seq;

  /* Request an acknowledgement by setting NLM_F_ACK */
  n->nlmsg_flags |= NLM_F_ACK;

  klog(KER_DEBUG, "netlinkTalk: %s type %s(%u), seq=%u \n", nl->name,
               lookup (NL_MSG_STR, n->nlmsg_type), n->nlmsg_type,
               n->nlmsg_seq);

  /* Send message to netlink interface. */
  status = sendmsg (nl->sock, &msg, 0);
  save_errno = errno;

  if (status < 0)
  {
    klog(KER_DEBUG, "netlinkTalk sendmsg() error: %s \n",
          safeStrError (save_errno));
    return -1;
  }

  /* 
   * Get reply from netlink socket. 
   * The reply should either be an acknowlegement or an error.
   */
  return netlinkParseInfo (netlinkTalkFilter, nl);
}


/*
 * Description : 인터페이스 주소 수정
 *
 * param [in] cmd : 명령타입
 * param [in] family : 네트워크 패밀리
 * param [in] ifp : interface 자료구조 포인터
 * param [in] ifc : connected 자료구조 포인터
 *
 * retval : -1 if 실패,
 *           0 if 성공
 */
static Int32T
netlinkAddress (Int32T cmd, Int32T family, InterfaceT *ifp,
                 ConnectedT *ifc)
{
  klog(KER_DEBUG, "called (cmd=%d) \n", cmd);

  Int32T bytelen;
  PrefixT *p;

  struct
  {
    struct nlmsghdr n;
    struct ifaddrmsg ifa;
    char buf[1024];
  } req;

  p = ifc->address;
  memset (&req, 0, sizeof req);

  bytelen = (family == AF_INET ? 4 : 16);

  req.n.nlmsg_len = NLMSG_LENGTH (sizeof (struct ifaddrmsg));
  req.n.nlmsg_flags = NLM_F_REQUEST;
  req.n.nlmsg_type = cmd;
  req.ifa.ifa_family = family;

  req.ifa.ifa_index = ifp->ifIndex;
  req.ifa.ifa_prefixlen = p->prefixLen;

  if (family == AF_INET)
    addattr_l (&req.n, sizeof req, IFA_LOCAL, &p->u.prefix4, bytelen);
#ifdef HAVE_IPV6
  else if (family == AF_INET6)
    addattr_l (&req.n, sizeof req, IFA_LOCAL, &p->u.prefix6, bytelen);
#endif

  if (family == AF_INET && cmd == RTM_NEWADDR)
  {
    if (!CONNECTED_PEER(ifc) && ifc->destination)
    {
      p = ifc->destination;
      addattr_l (&req.n, sizeof req, IFA_BROADCAST, &p->u.prefix4,
                 bytelen);
    }
  }

  if (CHECK_FLAG (ifc->flags, IFA_SECONDARY))
    SET_FLAG (req.ifa.ifa_flags, IFA_F_SECONDARY);

  if (ifc->label)
    addattr_l (&req.n, sizeof req, IFA_LABEL, ifc->label,
               strlen (ifc->label) + 1);

  return netlinkTalk (&req.n, &NETLINK_CMD);
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
    klog(KER_DEBUG, "Can't install socket filter: %s\n", safeStrError(errno));
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
  /*
  klog(KER_DEBUG, "%s called \n", __FUNCTION__);
  */

  if(netlinkParseInfo (netlinkInformationFetch, &NETLINK) < 0)
  {
    klog(KER_DEBUG, "Error] netlinkParseInfo retrun value\n");
  }
}



/* Description : RIB Manager의 초기화 시점에 Kernel로부터 인터페이스 정보를 
 * 요청하는 함수. */
Int32T
netlinkUpdateIfAll (void)
{
  Int32T ret;

  klog(KER_DEBUG, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
  klog(KER_DEBUG, "INTERFACE SCANE: start  \n");
  klog(KER_DEBUG, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

  /* Get interface information. */
  ret = netlinkRequest (AF_PACKET, RTM_GETLINK, &NETLINK_CMD);
  if (ret < 0)
    return ret;
  ret = netlinkParseInfo (netlinkInterface, &NETLINK_CMD);
  if (ret < 0)
    return ret;

  klog(KER_DEBUG, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
  klog(KER_DEBUG, "ADDRESS SCANE :  start \n");
  klog(KER_DEBUG, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

  /* Get IPv4 address of the interfaces. */
  ret = netlinkRequest (AF_INET, RTM_GETADDR, &NETLINK_CMD);
  if (ret < 0)
    return ret;
  ret = netlinkParseInfo (netlinkInterfaceAddr, &NETLINK_CMD);
  if (ret < 0)
    return ret;

#ifdef HAVE_IPV6
  /* Get IPv6 address of the interfaces. */
  ret = netlinkRequest (AF_INET6, RTM_GETADDR, &NETLINK_CMD);
  if (ret < 0)
    return ret;
  ret = netlinkParseInfo (netlinkInterfaceAddr, &NETLINK_CMD);
  if (ret < 0)
    return ret;
#endif /* HAVE_IPV6 */

  // display interface and connected address
  //ifDumpAll();

  return 0;
}


/*
 * Description : Kernel과 통신을 위한 Netlink 소켓을 생성 하고, 
 * 콜백함수를 등록하는 함수.
 */
void
netlinkInit (void)
{
  Uint32T groups;

  klog(KER_DEBUG, "%s %s called\n", __FILE__, __FUNCTION__);

  // Create Netlink Falgs
  groups = RTMGRP_LINK | RTMGRP_IPV4_ROUTE | RTMGRP_IPV4_IFADDR;
#ifdef HAVE_IPV6
  groups |= RTMGRP_IPV6_ROUTE | RTMGRP_IPV6_IFADDR;
#endif /* HAVE_IPV6 */

  // Create Netlink Socket
  netlinkSocket (&NETLINK, groups);
  netlinkSocket (&NETLINK_CMD, 0);

  // Register kernel socket.
  if (NETLINK.sock > 0)
  {
    // Only want non-blocking on the netlink event socket
#if 1
    if (fcntl (NETLINK.sock, F_SETFL, O_NONBLOCK) < 0)
      klog(KER_DEBUG, "Error] Can't set %s socket flags: %s \n", NETLINK.name,
		safeStrError (errno));
#endif

    // Set receive buffer size if it's set from command line
    if (nlRcvBuffSize)
      netlinkRecvBuff (&NETLINK, nlRcvBuffSize);

    // Set Netlink Filter
    netlinkInstallFilter (NETLINK.sock, NETLINK_CMD.snl.nl_pid);
   
    // This Add Socket File Discriptor, and register Callback Function
	kernelRequestCB(readNetlinkCB, NETLINK.sock);
  }
  else
  {
    klog(KER_DEBUG, "Error] netlink.sock=%d\n", NETLINK.sock);
  }
}

/* Description : Netlink 소켓을 이용하여 Kernel로 IPv4 주소 설정을 요청하는 함수. */
Int32T
netlinkAddressAddIpv4 (InterfaceT *ifp, ConnectedT *ifc)
{
  return netlinkAddress (RTM_NEWADDR, AF_INET, ifp, ifc);
}


/* Description : Netlink 소켓을 이용하여 Kernel로 IPv4 주소 삭제를 요청하는 함수. */
Int32T
netlinkAddressDeleteIpv4 (InterfaceT *ifp, ConnectedT *ifc)
{
  return netlinkAddress (RTM_DELADDR, AF_INET, ifp, ifc);
}




void 
netlinkkernelInit()
{
  PAL_DBUG(0, "enter \n");
  /* Netlink Socket Init */
  netlinkInit();
}

void
netlinkkernelIfScan (void)
{
  netlinkUpdateIfAll (); 
}


void
netlinkkernelIfUpdate (void)
{
  netlinkUpdateIfAll (); 
}

void 
netlinkkernelStart()
{
  /* Netlink Socket Init */
  netlinkInit();

  /* Netlink interface scan */
  netlinkUpdateIfAll();
}

void
netlinkkernelStop (void)
{
  /* tbd */
}

