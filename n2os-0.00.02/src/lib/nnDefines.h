#if !defined(_nnDefines_h)
#define _nnDefines_h

#include "nnTypes.h"

/*******************************************************************************
 *            Electronics and Telecommunications Research Institute
 *                      (Co-developer of ARCHYS Co., Ltd.)
 *
 * Filename: <nnDefines.h>
 *
 * Blockname: <N2OS Common Library : User-Defined Type>
 *
 * Overview: <User-Defined Type>
 *
 * Creator: <Changwoo Lee, JaeSu Han>
 *
 * Owner: <Changwoo Lee, Jaesu Han> (The person to contact regarding this file)
 *
 * Copyright: 2013 Electronics and Telecommunications Research Institute.
 *           All rights reserved.
 *           No part of this software shall be reproduced,
 *           stored in a retrieval system, or transmitted by any means,
 *           electronic, mechanical, photocopying, recording,
 *           or otherwise, without written permission from ETRI.
 ******************************************************************************/


/*******************************************************************************
 *                        VERSION CONTROL  INFORMATION
 * $Author: jshan $     Jaesu Han
 * $Date: 2014-03-28 17:34:36 +0900 (Fri, 28 Mar 2014) $       2013.11.13
 * $Revision    0.1
 * $Log$        Develop draft
 ******************************************************************************/


/*******************************************************************************
 *                        CONSTANTS / LITERALS / TYPES
 ******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// N2OS Component LIST 
///////////////////////////////////////////////////////////////////////////////

enum {

    IPC_MANAGER = 0,		
    PROCESS_MANAGER,	
    PORT_INTERFACE_MANAGER,	
    RIB_MANAGER,		
    POLICY_MANAGER,
    COMMAND_MANAGER,
    MULTICAST_RIB_MANAGER,	
    LIB_MANAGER,		
    CHECKPOINT_MANAGER,		
    LACP,			
    MSTP,			
    GVRP,			
    IGMP,		
    RIP,		
    ISIS,
    OSPF,			
    BGP,			
    PIM,			
    RSVP,			
    LDP,			
    RIB_TESTER,
    DLU_TESTER,
    MAX_PROCESS_CNT

};

///////////////////////////////////////////////////////////////////////////////
// N2OS USER MEMORY TYPE LIST 
///////////////////////////////////////////////////////////////////////////////

enum {
   
    MEM_USER_START = 0,
   
  /* PROCESS_MANAGER MEMORY TYPE */
    MEM_PROCESS_REGISTER = 0,
	MEM_PROCESS_TIMER,

  /* Routing RELATED MEMORY TYPE */
    MEM_GLOBAL, /* Component's global data struture */
    MEM_VECTOR,
    MEM_VECTOR_INDEX,
    MEM_VRF,
    MEM_VRF_NAME,
    MEM_IF_LIST,
    MEM_IF,
    MEM_IF_NAME,
    MEM_IF_LABEL,
    MEM_PREFIX,
    MEM_PREFIX_IPV4,
    MEM_PREFIX_IPV6,
    MEM_STATIC_IPV4,
    MEM_STATIC_IPV6,
    MEM_NEXTHOP,
    MEM_CONNECTED,
    MEM_ROUTE_TABLE,
    MEM_ROUTE_NODE,
    MEM_ROUTE_RIB,
    MEM_AUTHENTICATION,
    MEM_AUTHENTICATION_NAME,
    MEM_KEY_LIST,
    MEM_KEY,
    MEM_KEY_NAME,
    MEM_KEYCHAIN,
    MEM_KEYCHAIN_NAME,
    MEM_FILTER,
    MEM_FILTER_NAME,
    MEM_ACCESSLIST,
    MEM_ACCESSLIST_NAME,
    MEM_ACCESSLIST_REMARK,
    MEM_PREFIXLIST,
    MEM_PREFIXLIST_NAME,
    MEM_PREFIXLIST_DESC,
    MEM_PREFIXLIST_ENTRY,
    MEM_PREFIX_NAME,
    MEM_DISTRIBUTE,
    MEM_ROUTEMAP,
    MEM_ROUTEMAP_INDEX,
    MEM_ROUTEMAP_NAME,
    MEM_ROUTEMAP_DESC,
    MEM_ROUTEMAP_RULE,
    MEM_ROUTEMAP_RULE_NAME,
    MEM_ROUTEMAP_IN,
    MEM_ROUTEMAP_OUT,
    MEM_ROUTEMAP_COMPILED,
    MEM_OFFSET,
    MEM_OFFSET_IN,
    MEM_OFFSET_OUT,
    MEM_METRIC,
    MEM_SOCKET_UNION,
    MEM_RIP_PEER,
    MEM_RIP_INFO,
    MEM_RIP_DISTANCE,

 /* POLICY_MANAGER MEMORY TYPE */
    MEM_POLICY_STD_ACL,
    MEM_POLICY_EXT_ACL,

    MEM_PORT,

    MEM_OTHER,
    MEM_MAX_USER_TYPE

} memUserTypeT;


/* To do.
   USER memory type add
   (e.g.)
   enum {
        ...
        MEM_NAME, // Add Type 

		MEM_MAX_USER_TYPE
	};
*/

///////////////////////////////////////////////////////////////////////////////
// N2OS SYSTEM MEMORY TYPE LIST 
///////////////////////////////////////////////////////////////////////////////

enum {

    MEM_SYS_START = 0xFF00,
    CQUEUE_HEAD = 0xFF00,   /**< Cqueue Head Memory Type */
    CQUEUE_NODE,            /**< Cqueue Node Memory Type */
    LIST_HEAD,              /**< Linked List Head Memory Type */
    LIST_NODE,              /**< Linked List Node Memory Type */
    LIST_SUB_HEAD,          /**< Linked List Sub Head Memory Type */
    LIST_SUB_NODE,          /**< Linked List Sub Node Memory Type */
    HASH_HEAD,              /**< Hash Head Memory Type */
    HASH_BUCKET,            /**< Hash Bucket Memory Type */
    AVL_TREE_HEAD,          /**< Avl Tree Head Memory Type */
    AVL_TREE_NODE,          /**< Avl Tree Node Memory Type */
    MESSAGE_HEADER,         /**< IPC/EVENT Message Header Memory Type */
    IPC_MESSAGE,            /**< IPC/EVENT Message Body Memory Type */
    PROCESS_SINGLE_INFO,    /**< Process Single Info. Memory Type */
    EVENT_SUB_MESSAGE,      /**< EVENT Subscribe Message Memory Type */
    EVENT_SUB_PROCESS,      /**< EVENT Subscribe Process Memory Type */
    LOG,                    /**< LOG Service Memory Type */
    DLU_SEND_MESSAGE,       /**< DLU Service Send. Memory Type */
    MEM_MAX_SYS_TYPE

} MemSystemTypeT;


/** Memory Management Service Error Code */

#define MEMMGR_ERR_ALLOC -1          /**< Memory 동적 할당 실패 */
#define MEMMGR_ERR_REALLOC -1        /**< Memory 동적 재할당 실패 */
#define MEMMGR_ERR_DEBUG_ALLOC -2    /**< Memory Debug 구조체 동적 할당 실패 */
#define MEMMGR_ERR_DEBUG_ENQUEUE -3  /**< Memory Debug 관리 Enqueue 실패 */
#define MEMMGR_ERR_ARGUMENT -4       /**< 잘못된 인자 값 */


///////////////////////////////////////////////////////////////////////////////
// N2OS EVENT TYPE LIST 
///////////////////////////////////////////////////////////////////////////////

enum {

	EVENT_0 = 0,
	EVENT_LCS_COMPONENT_ERROR_OCCURRED,
	EVENT_LCS_COMPONENT_SERVICE_STATUS,
	EVENT_ROUTER_ID,
	EVENT_INTERFACE_ADD,
	EVENT_INTERFACE_DELETE,
	EVENT_INTERFACE_UP,
	EVENT_INTERFACE_DOWN,
	EVENT_INTERFACE_ADDRESS_ADD,
	EVENT_INTERFACE_ADDRESS_DELETE,
	EVENT_IPV4_ROUTE_ADD,
	EVENT_IPV4_ROUTE_DELETE,
	EVENT_IPV6_ROUTE_ADD,
	EVENT_IPV6_ROUTE_DELETE,
	EVENT_2,
	EVENT_3,
	EVENT_PIF_L2_LINK,
	EVENT_PIF_L2_VLAN,
	EVENT_PIF_L3_INTERFACE,
	MAX_EVENT_CNT

} EventListT;

/* To do.
   N2OS Event type add
   (e.g.)
   enum {
        ...
        EVENT_NAME, // Add Type
        
        MAX_EVENT_CNT
   };
*/ 

///////////////////////////////////////////////////////////////////////////////
// N2OS System Library Define
///////////////////////////////////////////////////////////////////////////////

/* Log Flag (Output Location) */
#define LOG_NOLOG    0x00    /* 로깅을 수행하지 않음 */
#define LOG_STDOUT   0x01    /* STDOUT 에 로깅을 수행 */
#define LOG_SYSTEM   0x02    /* syslog 에 로깅을 수행 */
#define LOG_FILE     0x04    /* File 에 로깅을 수행 */
#define LOG_ALL      0x07    /* 로깅 설정 시 검사를 위해 사용 */

/* Log Level  */
#define LOG_EMERG      0   /* system is unusable */
#define LOG_ALERT      1   /* action must be taken immediately */
#define LOG_CRIT       2   /* critical conditions */
#define LOG_ERR        3   /* error conditions */
#define LOG_WARNING    4   /* warning conditions */
#define LOG_NOTICE     5   /* normal but significant condition */
#define LOG_INFO       6   /* informational */
#define LOG_DEBUG      7   /* debug-level messages */

/* Task Option Flag */
#define TASK_TIMEOUT    0x01
#define TASK_READ       0x02
#define TASK_WRITE      0x04
#define TASK_SIGNAL     0x08
#define TASK_PERSIST    0x10

/* Task Priority */
#define TASK_PRI_HIGH      0
#define TASK_PRI_MIDDLE    1
#define TASK_PRI_LOW       2

#define EVENT_PRI_CNT      3

/* Event priority */
#define EVENT_PRI_CNT      3

#define EVENT_PRI_HIGH     0
#define EVENT_PRI_MIDDLE   1
#define EVENT_PRI_LOW      2

///////////////////////////////////////////////////////////////////////////////
// N2OS Component using IPC Message Number
///////////////////////////////////////////////////////////////////////////////

#define IPC_NUM_IPC_MANAGER_START            1000
#define IPC_NUM_IPC_MANAGER_END              1199
#define IPC_DYNAMIC_UPGRADE_TEST_SEND        1001          
#define IPC_DYNAMIC_UPGRADE_TEST_RESPONSE    1002

#define IPC_NUM_PROCESS_MANAGER_START        1200
#define IPC_NUM_PROCESS_MANAGER_END          1399

#define IPC_NUM_RIB_MANAGER_START            1400
#define IPC_NUM_RIB_MANAGER_END              1599

#define IPC_NUM_PIF_MANAGER_START            1600
#define IPC_NUM_PIF_MANAGER_END              1799

#define IPC_NUM_POLICY_MANAGER_START         1800
#define IPC_NUM_POLICY_MANAGER_END           1999

#define IPC_NUM_LIB_MANAGER_START            2000
#define IPC_NUM_LIB_MANAGER_END              2199

#define IPC_NUM_CHECKPOINT_MANAGER_START     2200
#define IPC_NUM_CHECKPOINT_MANAGER_END       2399

#define IPC_NUM_STPM_START                   2400
#define IPC_NUM_STPM_END                     2599

#define IPC_NUM_LACP_START                   2600
#define IPC_NUM_LACP_END                     2799

#define IPC_NUM_GVRP_START                   2800
#define IPC_NUM_GVRP_END                     2999

#define IPC_NUM_IGMP_START                   3000
#define IPC_NUM_IGMP_END                     3199

#define IPC_NUM_PIM_START                    3200
#define IPC_NUM_PIM_END                      3399

#define IPC_NUM_RIP_START                    3400
#define IPC_NUM_RIP_END                      3599

#define IPC_NUM_OSPF_START                   3600
#define IPC_NUM_OSPF_END                     3799

#define IPC_NUM_ISIS_START                   3800
#define IPC_NUM_ISIS_END                     3999

#define IPC_NUM_BGP_START                    4000
#define IPC_NUM_BGP_END                      4199

#define IPC_NUM_RSVP_START                   4200
#define IPC_NUM_RSVP_END                     4399

#define IPC_NUM_LDP_START                    4400
#define IPC_NUM_LDP_END                      4599


///////////////////////////////////////////////////////////////////////////////
// N2OS Component using Process Manager IPC Message Number
///////////////////////////////////////////////////////////////////////////////

#define IPC_LCS_C2PM_REGISTER                 1201
#define IPC_LCS_C2PM_FINALIZE                 1202
#define IPC_LCS_C2PM_ERROR_REPORT             1203
#define IPC_LCS_C2PM_ROLE_QUIESCING_COMPLETE  1204
#define IPC_LCS_C2PM_RESPONSE                 1205
#define IPC_LCS_C2PM_ATTRIBUTE_SET            1206
#define IPC_LCS_C2PM_ATTRIBUTE_REQUEST        1207
#define IPC_LCS_PM2C_ATTRIBUTE_RESPONSE       1208
#define IPC_LCS_PM2C_TERMINATE                1209
#define IPC_LCS_PM2C_SETROLE                  1210
#define IPC_LCS_PM2C_HEALTHCHECK              1211
#define IPC_LCS_PM2C_DYNAMIC_UPGRADE          1212
#define IPC_LCS_PM2C_COMMAND_MANAGER_UP       1213
#define IPC_LCS_PM2C_RESPONSE_ACK             1214
#define IPC_LCS_PM2C_DYNAMIC_UPGRADE_RESPONSE 1215

#define IPC_STARTUP_CONFIG_REQUEST            1216
#define IPC_STARTUP_CONFIG_RESPONSE           1217
#define IPC_DYNAMIC_UPGRADE_VERSION           1218

///////////////////////////////////////////////////////////////////////////////
// N2OS Component using Command Message Number
///////////////////////////////////////////////////////////////////////////////

#define CMD_NUM_IPC_MANAGER_START            1000
#define CMD_NUM_IPC_MANAGER_END              1199

#define CMD_NUM_PROCESS_MANAGER_START        1200
#define CMD_NUM_PROCESS_MANAGER_END          1399

#define CMD_NUM_RIB_MANAGER_START            1400
#define CMD_NUM_RIB_MANAGER_END              1599

#define CMD_NUM_PIF_MANAGER_START            1600
#define CMD_NUM_PIF_MANAGER_END              1799

#define CMD_NUM_POLICY_MANAGER_START         1800
#define CMD_NUM_POLICY_MANAGER_END           1999

#define CMD_NUM_LIB_MANAGER_START            2000
#define CMD_NUM_LIB_MANAGER_END              2199

#define CMD_NUM_CHECKPOINT_MANAGER_START     2200
#define CMD_NUM_CHECKPOINT_MANAGER_END       2399

#define CMD_NUM_STPM_START                   2400
#define CMD_NUM_STPM_END                     2599

#define CMD_NUM_LACP_START                   2600
#define CMD_NUM_LACP_END                     2799

#define CMD_NUM_GVRP_START                   2800
#define CMD_NUM_GVRP_END                     2999

#define CMD_NUM_IGMP_START                   3000
#define CMD_NUM_IGMP_END                     3199

#define CMD_NUM_PIM_START                    3200
#define CMD_NUM_PIM_END                      3399

#define CMD_NUM_RIP_START                    3400
#define CMD_NUM_RIP_END                      3599

#define CMD_NUM_OSPF_START                   3600
#define CMD_NUM_OSPF_END                     3799

#define CMD_NUM_ISIS_START                   3800
#define CMD_NUM_ISIS_END                     3999

#define CMD_NUM_BGP_START                    4000
#define CMD_NUM_BGP_END                      4199

#define CMD_NUM_RSVP_START                   4200
#define CMD_NUM_RSVP_END                     4399

#define CMD_NUM_LDP_START                    4400
#define CMD_NUM_LDP_END                      4599


///////////////////////////////////////////////////////////////////////////////
// Flag manipulation macros.
///////////////////////////////////////////////////////////////////////////////
#define CHECK_FLAG(V,F)      ((V) & (F))
#define SET_FLAG(V,F)        (V) |= (F)
#define UNSET_FLAG(V,F)      (V) &= ~(F)

/*******************************************************************************
*                                GLOBALS VARIABLES
*******************************************************************************/

#endif
