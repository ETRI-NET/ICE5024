#ifndef __CMD_MANAGER_COMMON_H__
#define __CMD_MANAGER_COMMON_H__
/**
 * @brief Overview : Define structure and library commom
 * @brief Creator: Thanh Nguyen Ba
 * @file      : nnCmdCommon.h
 *
 * $Id: nos.h 770 2014-01-23 14:11:47Z thanh $
 * $Author: thanh $
 * $Date: 2014-01-23 09:11:47 -0500 (Thu, 23 Jan 2014) $
 * $Log$
 * $Revision: 770 $
 * $LastChangedBy: thanh $
 * $LastChanged$
 *
 *                      Electronics and Telecommunications Research Institute
 * Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.
 *           No part of this software shall be reproduced, stored in a retrieval system, or
 *           transmitted by any means, electronic, mechanical, photocopying, recording,
 *           or otherwise, without written permission from ETRI.
 **/

#define MAX_REASON_SIZE                       256 /** Max Reasons   */

#pragma pack(push, 1)
struct cmdSetResT
{
  Uint32T        cmdFd;
  Uint32T        cmdKey;
  Uint32T        result;
  char           reason[MAX_REASON_SIZE];
};
#pragma pack(pop)

typedef struct cmdSetResT cmdSetResT;

enum {
  CMD_SET_OK,
  CMD_SET_WARNING,
  CMD_SET_NOK,
  CMD_SHOW,
  CMD_SHOW_END
};


#define UNUSED(x) (void)(x) /** Disable Unused parameter  */

/********************************************************************************
 *                                  Define Variable
 ********************************************************************************/
#ifndef MAX
#define MAX(a,b)    ((a) > (b) ? (a) : (b)) /** Max Value */
#endif /** !MAX */
#ifndef MIN
#define MIN(a,b)    ((a) < (b) ? (a) : (b)) /** Min value */
#endif /** !MIN */

/********************************************************************************
 *                                  Define Buffer of CM IPC
 ********************************************************************************/
#define SO_RCV_BUF_SIZE_MIN     (48*1024)  /** min. rcv socket buf size       */
#define SO_RCV_BUF_SIZE_MAX     (256*1024) /** desired rcv socket buf size    */
#define SO_SND_BUF_SIZE_MIN     (48*1024)  /** min. snd socket buf size       */
#define SO_SND_BUF_SIZE_MAX     (256*1024) /** desired snd socket buf size    */

/********************************************************************************
 *                                  Define Return Value
 ********************************************************************************/
#define CMD_IPC_OK        0 /** Success */
#define CMD_IPC_WAIT      1 /** Waiting */
#define CMD_IPC_PASS_WAIT 2 /** Password Require  */
#define CMD_IPC_PASS_SHOW   3  /** Show value from cm hai add*/
#define CMD_IPC_ERROR     -1  /** Error   */
#define CMD_IPC_WARNING   -2  /** Warning */

/** TODO: Add more socket here  */
/********************************************************************************
 *                                  Define Service
 ********************************************************************************/
#define CMD_MANAGER_INETD_CONFIG_FILE     "/etc/inetd.nos.conf"   /**  Default Config File            */
#define CMD_MANAGER_DEFAULT_TELNET_PORT   65000                   /**  Default Telnetd Port for NOS   */
#define CMD_MANAGER_DEFAULT_SSHD_PORT     64000                   /**  Default Sshd Port for NOS      */

#define CMD_USERID_MAX  128     /** Max string length of username  */
#define CMD_PASSWD_MAX  1024      /** Max string length of password  */

enum
{
  CMD_IPC_CMSH_CLI = 0,
  CMD_IPC_COMP_CLI    ,
  CMD_IPC_MAX_CLI
};

enum
{
  CMD_PARSE_NULL = 0,
  CMD_PARSE_FAIL,
  CMD_PARSE_SUCCESS,
  CMD_PARSE_INCOMPLETE
};
/********************************************************************************
 *                                  Define Function for Lib
 ********************************************************************************/
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
Uint32T cmdDaemonize(Uint16T, Uint16T);
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void cmdCallbackShellEv(Int8T *argv[]);
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void cmdCallbackShellPipeEv(Int8T *argv1[], Int8T *argv2[]);
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
Int32T cmdCallbackShellCmd(Int8T *argv[]); /** Without diable stdout */
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
Int32T cmdCallbackShellSrv(Int8T *argv);
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void *cmdSetSignal(Int32T, void (*func)(Int32T));
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
Uint32T cmdGetTerm(Int32T sock, Uint16T *row, Uint16T *col);
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
Uint32T cmdSetTerm(Int32T sock, Uint16T row, Uint16T col);

Uint32T cmdCompBitAdd(Uint8T value[], Uint32T nIndex);
Uint32T cmdCompBitCheck(Uint8T value[], Uint32T nIndex);
#endif
