#include <string.h>
#include <stdio.h>

#include "nnTypes.h"
#include "nosLib.h"

#if !defined(_processmgrApi_h)
#define _processmgrApi_h

/*******************************************************************************
 *            Electronics and Telecommunications Research Institute
 * Copyright: 2013 Electronics and Telecommunications Research Institute.
 *            All rights reserved.
 *            No part of this software shall be reproduced, stored in a
 *            retrieval system, or transmitted by any means, electronic,
 *            mechanical, photocopying, recording, or otherwise, without
 *            written permission from ETRI.
*******************************************************************************/

/**
 * @briefa : Process Manager 가 지원하는 API 를 포함
 *  - Block Name : Process Manager
 *  - Process Name : procmgr
 *  - Creator : Changwoo Lee
 *  - Initial Date : 2014/4/15
*/

/**
 * @file : processmgrApi.h
 *
 * $Id:
 * $Author:
 * $Date:
 * $Revision:
 * $LastChangedBy:
 **/

/*******************************************************************************
 *                        CONSTANTS / LITERALS / TYPES
 ******************************************************************************/
/** Recovery 를 위한 값 */
typedef enum
{
    LCS_NO_RECOMMENDED,
    LCS_COMPONENT_RESTART,
    LCS_NODE_FAILOVER,
    LCS_NODE_SWITCHOVER,
    LCS_NODE_FAILFAST
} LcsRecoveryT;

/** Role 을 위한 값 */
typedef enum
{
    LCS_HA_NONE,
    LCS_HA_SIMPLEX,
    LCS_HA_RPR_ACTIVE,
    LCS_HA_RPR_STANDBY,
    LCS_HA_SSO_ACTIVE,
    LCS_HA_SSO_STANDBY,
    LCS_HA_QUIESCING,
    LCS_HA_QUIESCED
} LcsHaStateT;

/** Process 의 동작 상태를 알리기 위한 값 */
typedef enum
{
    LCS_IPC_MANAGER = 0,
    LCS_PROCESS_MANAGER = 1,
    LCS_RIB_MANAGER = 2,
    LCS_PORT_INTERFACE_MANAGER = 3,
    LCS_POLICY_MANAGER = 4,
    LCS_COMMAND_MANAGER = 5,
    LCS_MULTICAST_RIB_MANAGER = 6,
    LCS_LIB_MANAGER = 7,
    LCS_MSTP = 8,
    LCS_LACP = 9,
    LCS_GVRP = 10,
    LCS_IGMP = 11,
    LCS_PIM = 12,
    LCS_RIP = 13,
    LCS_ISIS = 14,
    LCS_OSPF = 15,
    LCS_BGP = 16,
    LCS_RSVP = 17,
    LCS_LDP = 18,
    LCS_MAX_PROCESS_CNT = 63
} LcsProcessTypeT;

/** Attribute 에 대한 값 */
typedef enum
{
    ATTRIBUTE_SET,
    ATTRIBUTE_REQUEST,
    ATTRIBUTE_RESPONSE
} LcsAttributeOperationT;

/** Error 에 대한 값 */
typedef enum
{
    LCS_OK = 1,
    LCS_ERR_LIBRARY = 2,
    LCS_ERR_VERSION = 3,
    LCS_ERR_INIT = 4,
    LCS_ERR_TIMEOUT = 5,
    LCS_ERR_TRY_AGAIN = 6,
    LCS_ERR_INVALID_PARAM = 7,
    LCS_ERR_NO_MEMORY = 8,
    LCS_ERR_BAD_HANDLE = 9,
    LCS_ERR_BUSY = 10,
    LCS_ERR_ACCESS = 11,
    LCS_ERR_NOT_EXIST = 12,
    LCS_ERR_NAME_TOO_LONG = 13,
    LCS_ERR_EXIST = 14,
    LCS_ERR_NO_SPACE = 15,
    LCS_ERR_INTERRUPT = 16,
    LCS_ERR_NO_RESOURCES = 18,
    LCS_ERR_NOT_SUPPORTED = 19,
    LCS_ERR_BAD_OPERATION = 20,
    LCS_ERR_FAILED_OPERATION = 20,
    LCS_ERR_MESSAGE_ERROR = 22,
    LCS_ERR_QUEUE_FULL = 23,
    LCS_ERR_QUEUE_NOT_AVAILABLE = 24,
    LCS_ERR_BAD_FALGS = 25,
    LCS_ERR_TOO_BIG = 26,
    LCS_ERR_NO_SECTIONS = 27,
    LCS_ERR_NO_OP = 28,
    LCS_ERR_REPAIR_PENDING = 29,
    LCS_ERR_NO_BINDINGS = 30,
    LCS_ERR_UNAVAILABLE = 31,
    LCS_ERR_CAMPAIGN_ERROR_DETECTED = 32,
    LCS_ERR_CAMPAIGN_PROC_FAILED = 33,
    LCS_ERR_CAMPAIGN_CANCELED = 34,
    LCS_ERR_CAMPAIGN_FAILED = 35,
    LCS_ERR_CAMPAIGN_SUSPENDED = 36,
    LCS_ERR_CAMPAIGN_SUSPENDING = 37,
    LCS_ERR_ACCESS_DENIED = 38,
    LCS_ERR_NOT_READY = 39,
    LCS_ERR_DEPLOYMENT = 40
} LcsErrorT;

/** Attribute 를 담는 구조체 */
typedef struct
{
    LcsRecoveryT defaultRR;
    Uint32T      healthcheckInterval;
    Uint32T      healthcheckResponseTimeout;
    Uint32T      requestResponseTimeout;
    LcsHaStateT  haState;
    Uint32T      memoryThreshold;
    Uint32T      cpuThreshold;
} LcsAttributeT;

/** Process 의 동작상태를 담는 구조체 */
typedef struct
{
    Uint64T serviceStatus;
} LcsServiceStatusEventT;

/** Process 등록을 위한 정보를 담는 구조체 */
typedef struct
{
    LcsProcessTypeT processType;
    Int32T          ipcType;
    LcsAttributeT   attribute;
} LcsRegisterMsgT;

/** Process 종료를 위한 정보를 담는 구조체 */
typedef struct
{
    LcsProcessTypeT processType;
} LcsFinalizeMsgT;

/** Attribute 메시지에 사용되는 구조체 */
typedef struct
{
    LcsProcessTypeT        processType;
    LcsAttributeOperationT operation;
    LcsAttributeT          attribute;
} LcsAttributeMsgT;

/** Error Report 에 사용되는 구조체 */
typedef struct
{
    LcsProcessTypeT processType;
    LcsErrorT       error;
    LcsRecoveryT    recommendedRecovery;
} LcsErrorReportMsgT;

/** Process 의 Quiscing 완료시에 사용되는 구조체 */
typedef struct
{
    LcsProcessTypeT processType;
    Uint32T         invocationId;
    LcsErrorT       error;
} LcsRoleQuiescingCompleteMsgT;

/** Process 의 응답에 사용되는 구조체 */
typedef struct
{
    LcsProcessTypeT processType;
    Uint32T         invocationId;
} LcsResponseMsgT;

/** Process 의 종료시에 사용되는 구조체 */
typedef struct
{
    LcsProcessTypeT processType;
    Uint32T         invocationId;
} LcsTerminateMsgT;

/** Process 의 Role 설정시에 사용되는 구조체 */
typedef struct
{
    LcsProcessTypeT processType;
    Uint32T         invocationId;
    LcsHaStateT     haState;
} LcsSetRoleMsgT;

/** Process 의 HealthCheck 시에 사용되는 구조체 */
typedef struct
{
    LcsProcessTypeT processType;
    Uint32T         invocationId;
} LcsHealthcheckRequestMsgT;

/** Error 발생시에 사용되는 구조체 */
typedef struct
{
    LcsProcessTypeT processType;
    LcsErrorT       error;
    LcsRecoveryT    recoveryAction;
} LcsErrorOccurredEventT;

/*******************************************************************************
 *                                GLOBALS VARIABLES
 ******************************************************************************/

/*******************************************************************************
 *                           GLOBAL FUNCTIONS/PROTOTYPES
 ******************************************************************************/
LcsErrorT lcsRegister(LcsProcessTypeT process, Int32T ipc,
                    LcsAttributeT lcsAttribute);
LcsErrorT lcsFinalize(LcsProcessTypeT process);
LcsErrorT lcsAttributeSet(LcsProcessTypeT process, LcsAttributeT lcsAttribute);
LcsErrorT lcsAttributeGet(LcsProcessTypeT process, LcsAttributeT *lcsAttribute);
LcsErrorT lcsErrorReport(LcsProcessTypeT process, LcsErrorT error,
            LcsRecoveryT recommendedRecovery);
LcsErrorT lcsRoleQuiescingComplete(LcsProcessTypeT process, Uint32T invocation,
            LcsErrorT error);
LcsErrorT lcsResponse(LcsProcessTypeT process, Uint32T invocation);

#endif /* _processmgrApi_h */
