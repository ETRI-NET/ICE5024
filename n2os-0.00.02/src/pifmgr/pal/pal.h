/*=============================================================================
**
** Copyright (C) 2002-2003 IP Infusion, Inc.  All Rights Reserved.
**
** pal.h -- ZebOS PAL common definitions
**          for Linux
*/
#ifndef _PAL_H
#define _PAL_H

/*-----------------------------------------------------------------------------
**
** Include files
*/

/* removed 
#include "config.h"
#include "plat_incl.h"
#include "pal_types.h"
#include "pal_posix.h"
#include "pal_assert.h"
#include "pal_memory.h"
#include "pal_file.h"
#include "pal_daemon.h"
#include "pal_debug.h"
#include "pal_stdlib.h"
#include "pal_string.h"
#include "pal_log.h"
#include "pal_time.h"
#include "pal_math.h"
#include "pal_regex.h"

#include "pal_socket.h"
#include "pal_sock_raw.h"
#include "pal_sock_ll.h"
#include "pal_sock_udp.h"
#include "pal_inet.h"
#include "pal_np.h"
#include "pal_pipe.h"
#include "pal_term.h"
#include "pal_signal.h"
*/

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <net/route.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <linux/ethtool.h>
#include <linux/sockios.h>
/*-----------------------------------------------------------------------------
** Types
*/
typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned long int u64;

typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned int u_int32_t;
typedef int int32_t;


/* pal reference header files */
#include "interface.h"
#include "connected.h"
#include "palKernelIfTypes.h"
#include "palKernelIfStats.h"
#include "palKernelIfDefault.h"
#include "palKernel.h"

/* nos reference header files */
#include "nnTypes.h"
#include "nnPrefix.h"
#include "nnDefines.h"

#include "nosLib.h"


#define PAL_DBUG(level, format, ...) printf("[%s] %s: " format, __FILE__, __FUNCTION__, ##__VA_ARGS__)
#define klog(level, format, ...) printf("[%s] %s: " format, __FILE__, __FUNCTION__, ##__VA_ARGS__)

/*-----------------------------------------------------------------------------
**
** Constants and enumerations
*/

/*-----------------------------------------------------------------------------
**
** Functions
*/

/*-----------------------------------------------------------------------------
**
** Done
*/
#endif /* _PAL_H */
