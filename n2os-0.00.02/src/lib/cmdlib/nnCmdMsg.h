#ifndef __CMD_MSG_H__
#define __CMD_MSG_H__
/**
 * @brief Overview : Process packec IPC
 * @brief Creator: Thanh Nguyen Ba, Hai Tu
 * @file    	: cmdMsg.h 
 * 
 * $Id: cmdMsg.h 822 2014-02-13 08:18:37Z thanh $			
 * $Author: thanh $ 
 * $Date: 2014-02-13 03:18:37 -0500 (Thu, 13 Feb 2014) $
 * $Log$
 * $Revision: 822 $
 * $LastChangedBy: thanh $
 * $LastChanged$
 * 
 *                      Electronics and Telecommunications Research Institute
 * Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.
 *           No part of this software shall be reproduced, stored in a retrieval system, or
 *           transmitted by any means, electronic, mechanical, photocopying, recording,
 *           or otherwise, without written permission from ETRI.
 **/ 

/*******************************************************************************************************
 *                             LOCAL CONSTANTS/LITERALS/TYPES 
 *******************************************************************************************************/

#define CMD_IPC_MAXLEN      2048	/** Max CM IPC Message length	*/
#define CMD_IPC_HDR_LEN      12		/** Header length	*/
#define CMD_IPC_BODY_LEN        (CMD_IPC_MAXLEN - CMD_IPC_HDR_LEN)	/** Body of message CM IPC	*/

#pragma pack(1)
typedef struct cmIpcMsg
{
    /**	LENGTH          */
    Uint16T   length;

    /**	Request Key     */
    Uint16T   requestKey;
/*******************************************************************************************************
 *                             LOCAL CONSTANTS/LITERALS/TYPES 
 *******************************************************************************************************/
#define CMD_IPC_KEY_NULL                            0	/** Default IPC KEY	*/

    /**  Code            */
    Uint8T    code;
/*******************************************************************************************************
 *                             LOCAL CONSTANTS/LITERALS/TYPES 
 *******************************************************************************************************/
	
#define CMD_IPC_CODE_CLIENT_REG_REQ                 0	/** Request register	*/
#define CMD_IPC_CODE_CLIENT_REG_RES_OK              1	/** Response register	*/
#define CMD_IPC_CODE_CLIENT_DEREG_REQ               2	/** Request deregister	*/
#define CMD_IPC_CODE_CLIENT_DEREG_RES_OK            3	/** Response deregister	*/
#define CMD_IPC_CODE_AUTH_ENABLE_REQ                4	/** Enable node request password	*/
#define CMD_IPC_CODE_AUTH_ENABLE_RES_OK             5	/** Enable node reponse is ok	*/
#define CMD_IPC_CODE_AUTH_ENABLE_RES_NOK            6	/** Enable Node reponse is not ok	*/
#define CMD_IPC_CODE_AUTH_ENABLE_REQ_PASSWD         7	/** Enable Node request password	*/
#define CMD_IPC_CODE_AUTH_CONFIGURE_REQ             8	/** Config Node request	*/
#define CMD_IPC_CODE_AUTH_CONFIGURE_RES_NOK         9	/** Config Node response	*/
#define CMD_IPC_CODE_AUTH_CONFIGURE_RES_EXIST       10	/** Config Node Response has another in this mode	*/
#define CMD_IPC_CODE_AUTH_CONFIGURE_RES_PASSWD      11	/** Config Node Request Password	*/
#define CMD_IPC_CODE_AUTH_CONFIGURE_RES_OK          12	/** Config Node response is ok	*/
#define CMD_IPC_CODE_AUTH_CONFIGURE_REQ_PASSWD      13	/** Config Node Request Password	*/
/**	Show	*/
#define CMD_IPC_CODE_FUNC_SHOW						14	/** CMSH request do function show	*/
/**	End		*/
#define CMD_IPC_CODE_FUNC_REQ                       15	/** CMSH request do function	*/
#define CMD_IPC_CODE_FUNC_RES_OK                    16	/** CM response do function success	*/
/**  hai add */
#define CMD_IPC_CODE_FUNC_EXIT                      17	/** Do function exit and back to parrent node	*/
/**  thanh add */
#define CMD_IPC_CODE_UPDATE_NODE                    18	/** Update node 	*/
/**  wowook add */
#define CMD_IPC_CODE_FUNC_RES_WAIT                  19	/** Response from CM for waiting funtion	*/
#define CMD_IPC_CODE_FUNC_REQ_PASSWD				        20	/** Request Password From CMSH	*/
#define CMD_IPC_CODE_FUNC_RES_PASSWD				        21	/** Request Password From CMSH	*/

#define CMD_IPC_CODE_STARTUP_CONFIG                 22  /** Show all running config*/ 

/** thanh@ add for change node  */
#define CMD_IPC_CODE_CHANGE_NODE_REQ                247
#define CMD_IPC_CODE_CHANGE_NODE_RES_OK             248
#define CMD_IPC_CODE_CHANGE_NODE_RES_NOT_OK         249

/** thanh@ add for OAM message  */
#define CMD_IPC_OAM_DATA                            250
/** thanh@ add for cmdOutput    */
#define CMD_IPC_OUTPUT_DATA                         251

/** thanh@ add for login */
#define CMD_IPC_CODE_CLIENT_REG_LOGIN               252 /** Request Login For Telnet and ssh  */
#define CMD_IPC_CODE_CLIENT_RES_LOGIN_NOT_OK        253 /** Response Not success  */
#define CMD_IPC_CODE_CLIENT_RES_LOG_SUCCESS         254 /** Response Success      */
    /**  Cli Type        */
    Uint8T    clientType;
/*******************************************************************************************************
 *                             LOCAL CONSTANTS/LITERALS/TYPES 
 *******************************************************************************************************/
#define CMD_IPC_TYPE_CLI            0	/** CM IPC TYPE CLI	*/
#define CMD_IPC_TYPE_WEB            1	/** CM IPC TYPE WEB	*/
#define CMD_IPC_TYPE_SSH            2	/** CM IPC TYPE SSH	*/

    /**  Mode            */
    Uint8T    mode;

    /**  User Key        */
    Uint8T    userKey;

    /**  Src ID          */
    Uint16T   srcID;

    /**  Dst ID          */
    Uint16T   dstID;
} cmdIpcMsg_T;

typedef struct cmdIpcMsgTlv
{
    /**  Type        */
    Uint16T type;
/*******************************************************************************************************
 *                             LOCAL CONSTANTS/LITERALS/TYPES 
 *******************************************************************************************************/
	
#define CMD_IPC_TLV_NULL            0  	/** NULL TLV	*/
#define CMD_IPC_TLV_USERID          1	/** USERID TLV	*/
#define CMD_IPC_TLV_PRIVILEGE       2	/** Pri mode TLV	*/
#define CMD_IPC_TLV_PASSWD          3	/** Password TLV	*/
#define CMD_IPC_TLV_CALLBACK_FUNC   4	/** Callback Func TLV	*/
#define CMD_IPC_TLV_CALLBACK_ARGSTR 5	/** Callback parameter TLV	*/
#define CMD_IPC_TLV_CALLBACK_INDEX  6	/** Callback parameter TLV	*/
#define CMD_IPC_TLV_RET_STR         7	/** Response string TLV	*/
#define CMD_IPC_TLV_MODE            8	/** Mode TLV	*/
#define CMD_IPC_TLV_MODE_ARGC       9	/** Mode argc TLV	*/
#define CMD_IPC_TLV_MODE_ARGV       10	/** Mode argc TLV	*/
#define CMD_IPC_TLV_MODE_ARGV1       11/** Mode argv TLV	*/
#define CMD_IPC_TLV_MODE_ARGV2       12/** Mode argv TLV	*/
#define CMD_IPC_TLV_MODE_ARGV3       13/** Mode argv TLV	*/
#define CMD_IPC_TLV_MODE_ARGV4       14/** Mode argv TLV	*/
#define CMD_IPC_TLV_MODE_ARGV5       15/** Mode argv TLV	*/
#define CMD_IPC_TLV_OUTPUT_TYPE     16
#define CMD_IPC_TLV_OUTPUT_DATA     17

#define CMD_IPC_TLV_OAM_TYPE        18  /** OAM Type      */
#define CMD_IPC_TLV_OAM_DATA        19  /** OAM Data      */
    /**  Length      */
    Uint16T length;
    /**  Data        */
    void    *data;
} cmdIpcMsgTlv_T;
#pragma pack()

/**
 * Description: function which used for CM IPC message making
 *
 * @param [in]  msg: Message buffer for CM IPC, Header structure to add 
 * @param [out] msgBuf: Updated (header is added) message buffer
 *
 * @retval : code
 */
Int32T cmdIpcAddHdr(Int8T *, cmdIpcMsg_T *);
/**
 * Description: used to unpack message header
 *
 * @param [in]  msgBuf: Message buffer of CM IPC to extract header 
 * @param [out] msg: Extracted header structure 
 *
 * @retval : code
 */
Int32T cmdIpcUnpackHdr(Int8T *, cmdIpcMsg_T *);
/**
 * Description: function which used for CM IPC message making
 *
 * @param [in]  data: Message buffer for CM IPC, TLV(type, length, value) to add.
 * @param [in]  type: type TLV
 * @param [in]  length: length TLV
 * @param [out] msgBuf: Updated (TLV is added) message buffer 
 *
 * @retval : code
 */
Int32T cmdIpcAddTlv(Int8T *, Int32T, Int32T  , void *);
/**
 * Description: used to unpack message TLV
 *
 * @param [in]  msgBuf  : Message buffer of CM IPC to extract header
 * @param [in]  type: type TLV
 * @param [out] size: size of data
 * @param [out] data  : Extracted header structure
 *
 * @retval : code
 */
Int32T cmdIpcUnpackTlv(Int8T *, Int32T, Int32T *, void *);
/**
 * Description: send request
 *
 * @param [in]  sock  :  socket fd
 * @param [in]  msgBuf  : CM IPC message to send
 * @param [out] none
 *
 * @retval : code
 */
Int32T cmdIpcSend(Int32T sock, Int8T *msgBuf);
/**
 * Description: receive response
 *
 * @param [in]  sock  :  socket fd 
 * @param [out] msgBuf : Received CM IPC message
 *
 * @retval : code
 */
Int32T cmdIpcRecv(Int32T sock, Int8T *msgBuf);
#endif
