#ifndef __CMD_NODE_H__
#define __CMD_NODE_H__
/**
 * @brief Overview :
 * @brief Creator: Thanh Nguyen Ba, Hai Tu
 * @file      : cmdNode.h
 *
 * $Id: cmdNode.h 932 2014-03-04 09:09:11Z thanh $
 * $Author: thanh $
 * $Date: 2014-03-04 04:09:11 -0500 (Tue, 04 Mar 2014) $
 * $Log$
 * $Revision: 932 $
 * $LastChangedBy: thanh $
 * $LastChanged$
 *
 *                      Electronics and Telecommunications Research Institute
 * Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.
 *           No part of this software shall be reproduced, stored in a retrieval system, or
 *           transmitted by any means, electronic, mechanical, photocopying, recording,
 *           or otherwise, without written permission from ETRI.
 **/

/********************************************************************************
 *                                  INCLUDE FILES
 ********************************************************************************/
#include "nnDefines.h"

enum
{
  CMD_NODE_NULL = 0,
  CMD_NODE_VIEW,
  CMD_NODE_EXEC,
  CMD_NODE_CONFIG,
  CMD_NODE_INTERFACE,
  CMD_NODE_VLAN,
  CMD_NODE_VTY,
  CMD_NODE_CONSOLE,
  CMD_NODE_CONFIG_ROUTE_MAP,
  CMD_NODE_CONFIG_KEYCHAIN,
  CMD_NODE_CONFIG_KEYCHAIN_KEY,
  CMD_NODE_CONFIG_RIP,
  CMD_NODE_CONFIG_ISIS,
  CMD_NODE_CONFIG_OSPF,
  CMD_NODE_CONFIG_USER,
  CMD_NODE_CONFIG_SERVICE,
  CMD_NODE_CONFIG_TEST,	//j1 added
  CMD_NODE_CONFIG_TEST_MUL, //hai added
  CMD_NODE_MAX
};

enum
{
  PRIVILEGE_MIN = 0,      /** Min priv  */
  PRIVILEGE_EXEC,         /** Enable Node Priv  */
  PRIVILEGE_CONF,         /** Config Node Priv  */
  PRIVILEGE_INFD,         /** Interface Node Priv */
  PRIVILEGE_VLAN,         /** Vlan Node Priv */
  PRIVILEGE_VTY,          /** VTY Priv  */
  PRIVILEGE_CONSOLE,      /** Console priv  */
  PRIVILEGE_CONFIG_ROUTE_MAP, /** Router Map node priv  */
  PRIVILEGE_CONFIG_KEYCHAIN,
  PRIVILEGE_CONFIG_KEYCHAIN_KEY,
  PRIVILEGE_CONFIG_RIP,
  PRIVILEGE_CONFIG_ISIS,
  PRIVILEGE_CONFIG_OSPF,
  PRIVILEGE_CONFIG_USER,
  PRIVILEGE_CONFIG_SERVICE,
  PRIVILEGE_CONFIG_TEST,	//j1 added
  PRIVILEGE_CONFIG_TEST_MUL,	//hai added
  PRIVILEGE_MAX           /** Max Priv  */
};

#define CMD_TOKEN_LENGTH    128 /** Max Token Length  */

struct cmdTreeNode
{
  /** Token type */
  enum cmdToken
  {
    CMD_TOKEN_NULL,
    CMD_TOKEN_PAREN_OPEN,
    CMD_TOKEN_PAREN_CLOSE,
    CMD_TOKEN_CBRACE_OPEN,
    CMD_TOKEN_CBRACE_CLOSE,
    CMD_TOKEN_BRACE_OPEN,
    CMD_TOKEN_BRACE_CLOSE,
    CMD_TOKEN_IFNAME_OPEN,
    CMD_TOKEN_IFNAME_CLOSE,
    CMD_TOKEN_SEPARATOR,
    CMD_TOKEN_PIPE,
    CMD_TOKEN_REDIRECT,
    CMD_TOKEN_DOT,
    CMD_TOKEN_QUESTION,
    CMD_TOKEN_RANGE,
    CMD_TOKEN_KEYWORD,
    CMD_TOKEN_ALIAS,
    CMD_TOKEN_LINE,
    CMD_TOKEN_WORD,
    CMD_TOKEN_IPV4,
    CMD_TOKEN_IPV4_PREFIX,
    CMD_TOKEN_IPV6,
    CMD_TOKEN_IPV6_PREFIX,
    CMD_TOKEN_TIME,
    CMD_TOKEN_COMMUNITY,
    CMD_TOKEN_MAC_ADDRESS,
    CMD_TOKEN_IFNAME,
    CMD_TOKEN_UNKNOWN
  } type;

  /** String for this token */
  Int8T *tokenStr;

  struct cmdList *cHelpTable; /** New Version */

  /** Max and min for range type token. */
  Uint32T max;
  Uint32T min;

  struct cmdList *cCelTable;  /** New Version */

  /**  List Chidren node */
  struct cmdList *cNodeList;

  Uint16T flags;

  /** Reference count.  */
  Uint32T refCnt;
};

struct cmdElement
{
    /** Command line string.  */
    Int8T *str;

    /** Function to execute this command.  */
    Int32T funcID;       ///ID for callback function (ex. CMD_FUNC_ROUTER_OSPF)

    /** Help strings array. */
    Int8T **help;

    Int32T helpNum;

    /** Unique key. */
    Int32T cmdKey;

    /** Flags of the commands.  */
#define MAX_BYTES_FLAGS 64
    Uint8T flags[MAX_BYTES_FLAGS];    /** 00000000 00000000 00000000...  */
    /** Change to other node (If it is needed) */
    Int32T nextNode;
};

typedef Int32T (*CMD_NODE_WRITE_FUNC)(Int8T **);
typedef Int32T (*CMD_OUT_FUNC)(void *, Int8T *, ...);

struct cmdTree
{
    /**  Struct Handler CMSH */
    struct cmsh *cmsh;

    /** Parsing tree for each modes. */
    struct cmdTreeNode modeTree[CMD_NODE_MAX];

    /** Prompt string for each modes */
    Int8T* prompt[CMD_NODE_MAX];

    /** Privilege level for each modes */
    Uint8T nodePrivilege[CMD_NODE_MAX];

    /** configuration output function for each modes */
    CMD_NODE_WRITE_FUNC configWrite[CMD_NODE_MAX];
};

Int32T cmdBuildRecursive(struct cmdTreeNode *, struct cmdElement *, Int8T *, Int32T );
Int32T cmdGetTokenNumHelp(Int8T* );
Int8T *cmdGetToken (Int8T *str, enum cmdToken *type, Int32T *flags, Int8T **val);
Int32T cmdParseHelp (struct cmsh *cmsh);
Int8T **cmdParseAutoCompletion (struct cmsh *cmsh);
Int32T cmdParseCommand(struct cmsh *cmsh);

#endif
