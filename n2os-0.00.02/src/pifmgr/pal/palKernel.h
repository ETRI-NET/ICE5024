/* Copyright (C) 2002-2003 IP Infusion, Inc.  All Rights Reserved. */
#ifndef _PAL_KERNEL_DEF
#define _PAL_KERNEL_DEF

/* PAL kernel API.  */
#define RESULT_OK 0
#define result_t  int32_t


/* Functions.  */

/* kernel basic */
int32_t kernelStart (void);
int32_t kernelStop (void);
int32_t kernelIfScan (void);
int32_t kernelIfUpdate (void);
//int32_t kernel_if_info (InterfaceT *ifp);
    
/* kernel interface address */
int32_t kernelIfIpv4AddressAdd (InterfaceT *ifp, ConnectedT *ifc);
int32_t kernelIfIpv4AddressDelete (InterfaceT *ifp, ConnectedT *ifc);
int32_t kernelIfIpv4AddressDeleteAll (InterfaceT *ifp);
//int32_t kernelIfIpv4AddressUpdate (InterfaceT *ifp, ConnectedT *ifc_old, ConnectedT *ifc_new);
int32_t kernelIfIpv4AddressSecondaryAdd (InterfaceT *ifp, ConnectedT *ifc);
int32_t kernelIfIpv4AddressSecondaryDelete (InterfaceT *ifp, ConnectedT *ifc);

/* kernel interface parameters */
int32_t kernelIfGetIndex(InterfaceT *ifp);
int32_t kernelIfGetMetric (InterfaceT *ifp);
int32_t kernelIfSetMtu (InterfaceT *ifp, u_int32_t mtu_size);
int32_t kernelIfGetMtn (InterfaceT *ifp);
int32_t kernelIfGetBw (InterfaceT *ifp);
int32_t kernelIfGetHwaddr (InterfaceT *ifp);
int32_t kernelIfFlagsGet(InterfaceT *ifp);
int32_t kernelIfFlagsSet (InterfaceT *ifp,  u_int32_t flag);
int32_t kernelIfFlagsUnset (InterfaceT *ifp, u_int32_t flag);
int32_t kernelIfSetproxyArp (InterfaceT *ifp, u_int32_t proxy_arp);

#ifdef HAVE_IPV6
int32_t kernelIfIpv6AddressAdd (InterfaceT *ifp, ConnectedT *ifc);
int32_t kernelIfIpv6AddressDelete (InterfaceT *ifp, ConnectedT *ifc);
#endif /* HAVE_IPV6 */

/* kernel interface event */
void kernelEventIfAdd(InterfaceT *ifp);
void kernelEventIfDel(InterfaceT *ifp);

void kernelEventIfUp(InterfaceT *ifp);
void kernelEventIfDown(InterfaceT *ifp);

void kernelEventIfAddrAdd(InterfaceT *ifp, ConnectedT *ifc);
void kernelEventIfAddrDel(InterfaceT *ifp, ConnectedT *ifc);

/* N2OS Libarary for kernel functions */
void kernelRequestCB(void *cbFunc, int32_t sock);

/* N2OS Log  */
#define KER_EMERG              LOG_EMERG                 /* 0 system is unusable */
#define KER_ALERT              LOG_ALERT                 /* 1 action must be taken immediately */
#define KER_CRIT               LOG_CRIT                  /* 2 critical conditions */
#define KER_ERR                LOG_ERR                   /* 3 error conditions */
#define KER_WARNING            LOG_WARNING               /* 4 warning conditions */
#define KER_NOTICE             LOG_NOTICE                /* 5 normal but significant condition */
#define KER_INFO               LOG_INFO                  /* 6 informational */
#define KER_DEBUG              LOG_DEBUG                 /* 7 debug-level messages */



/*
void ifAddUpdate (InterfaceT *ifp);
void ifDeleteUpdate (InterfaceT *);
void ifStateUp (InterfaceT *);
void ifStateDown (InterfaceT *);
void connectedAddIpv4 (InterfaceT *ifp, int32_t flags, struct in_addr *addr, Uint8T prefixLen, struct in_addr *broad, const char * label);
void connectedDeleteIpv4 (InterfaceT *ifp, int32_t flags, struct in_addr *addr, Uint8T prefixLen, struct in_addr *broad);
*/


#endif /* _PAL_KERNEL_DEF */
