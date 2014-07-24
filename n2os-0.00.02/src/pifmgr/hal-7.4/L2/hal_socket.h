/* Copyright (C) 2004 IP Infusion, Inc. All Rights Reserved. */

#ifndef _HAL_SOCKET_H_
#define _HAL_SOCKET_H_

/* Supported address families */
#define AF_LACP                40
#define AF_EAPOL               41
#define AF_STP                 42
#define AF_HAL                 43
#define AF_GARP                44
#define AF_IGMP_SNOOP          45
#define AF_MLD_SNOOP           46
#define AF_PROTO_STP           1
#define AF_PROTO_RSTP          2
#define AF_PROTO_MSTP          3

typedef int hal_sock_handle_t;

typedef struct sockaddr_l2 sockaddr_stp_t;
typedef struct sockaddr_l2 sockaddr_lacp_t;
typedef struct sockaddr_l2 sockaddr_eapol_t;
typedef struct sockaddr_l2 sockaddr_garp_t;
typedef struct sockaddr_l2 sockaddr_pae_t;
typedef struct sockaddr_igs sockaddr_igs_t;

#endif /* _HAL_SOCKET_H_ */
