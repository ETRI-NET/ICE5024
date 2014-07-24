/* Copyright (C) 2003 IP Infusion, Inc. All Rights Reserved. */

//#include <pal.h>

#include "palKernelIfTypes.h"
#include <net/if_arp.h>

unsigned short 
pal_if_type (int type)
{
  if (type == ARPHRD_ETHER)
    return IF_TYPE_ETHERNET;
  else if (type == ARPHRD_LOOPBACK)
    return IF_TYPE_LOOPBACK;
  else if (type == ARPHRD_HDLC)
    return IF_TYPE_HDLC;
  else if (type == ARPHRD_PPP)
    return IF_TYPE_PPP;
  else if (type == ARPHRD_ATM)
    return IF_TYPE_ATM;
  else if (type == ARPHRD_DLCI)
    return IF_TYPE_FRELAY;
  else if (type == ARPHRD_TUNNEL)
    return IF_TYPE_IPIP;
  else if (type == ARPHRD_IPGRE)
    return IF_TYPE_GREIP;
  else if (type == ARPHRD_SIT)
    return IF_TYPE_IPV6IP;
  else
    return IF_TYPE_UNKNOWN;
}
