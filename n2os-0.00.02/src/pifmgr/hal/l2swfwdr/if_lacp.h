/* Copyright (C) 2003, IP Infusion, Inc. All Rights Reserved. 

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version
    2 of the License, or (at your option) any later version.

*/
#ifndef __IF_LACP_H__
#define __IF_LACP_H__

/* This module declares the API for sockets of type ETH_P_PAE.
   These sockets are used to send/recv LACP PDUs for an authenticator
   per the 802.1x specification. */

#define ETH_P_LACP 0x8809


#define IPI_LACP_VERSION  0x0610

/* Ioctl calls accepted on an ETH_P_LACP socket */

#define IPILACP_BASE          300
#define IPILACP_GET_VERSION   (IPILACP_BASE + 1)
#define IPILACP_ADD_AGG       (IPILACP_BASE + 2)
#define IPILACP_DEL_AGG       (IPILACP_BASE + 3)
#define IPILACP_AGG_LINK      (IPILACP_BASE + 4)
#define IPILACP_DEAGG_LINK    (IPILACP_BASE + 5)
#define IPILACP_SET_MACADDR   (IPILACP_BASE + 6)

#define LACP_NAME "IPI LACP 6.1"


#endif
