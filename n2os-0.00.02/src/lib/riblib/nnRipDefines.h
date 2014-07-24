/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the rip protocol related definitions.
 *  - Block Name : riblib
 *  - Process Name : rib library
 *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : nnRipDefines.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_nnRipDefines_h)
#define _nnRipDefines_h

/* Definition of IPC Command */
#define RIP_ROUTER_RIP                             1
#define RIP_VERSION                                2
#define RIP_ROUTE                                  3
#define RIP_DEFAULT_METRIC                         4
#define RIP_TIMERS_BASIC                           5
#define RIP_DISTANCE                               6
#define RIP_SHOW_IP_RIP                            7
#define RIP_SHOW_IP_RIP_STATUS                     8
#define RIP_NETWORK                                9
#define RIP_NEIGHBOR                              10
#define RIP_IP_RIP_RECEIVE_VERSION                11
#define RIP_IP_RIP_SEND_VERSION                   12
#define RIP_IP_RIP_AUTHENTICATION_MODE            13
#define RIP_IP_RIP_AUTHENTICATION_STRING          14
#define RIP_IP_RIP_AUTHENTICATION_KEYCHAIN        15
#define RIP_IP_RIP_SPLIT_HORIZON                  16
#define RIP_PASSIVE_INTERFACE                     17
#define RIP_REDISTRIBUTE_RIP                      18
#define RIP_DEFAULT_INFORMATION_ORIGINATE         19

/* Definition of RIP CLI COMMAND Flags */
#define RIP_CLI_COMMAND_TYPE_UNSET                 0
#define RIP_CLI_COMMAND_TYPE_SET                   1

/* Definition of RIP Distance Flags */
#define RIP_DISTANCE_FLAG_NETWORK_MASK          0x01
#define RIP_DISTANCE_FLAG_WORD_MASK             0x02

/* Definition of IP RIP RECEIVE VERSION Flags */
#define RIP_IP_RIP_RECEIVE_VERSION_MASK         0x01
#define RIP_IP_RIP_RECEIVE_VERSION_BOTH_MASK    0x02

/* Definition of IP RIP SEND VERSION Flags */
#define RIP_IP_RIP_SEND_VERSION_MASK            0x01
#define RIP_IP_RIP_SEND_VERSION_BOTH_MASK       0x02

/* Definition of IP RIP SEND VERSION Flags */
#define RIP_NETWORK_PREFIX_MASK                 0x01
#define RIP_NETWORK_IFNAME_MASK              0x02

/* Definition of IP RIP SEND VERSION Flags */
#define RIP_IP_RIP_SPLIT_HORIZON_POISONED_REVERSE_MASK     0x01


#endif /* _nnRipDefines_h */
