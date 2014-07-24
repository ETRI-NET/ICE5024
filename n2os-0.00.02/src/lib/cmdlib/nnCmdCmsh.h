#ifndef __CMSH_H__
#define __CMSH_H__
/**
 * @brief Overview : Process function for cmsh module
 * @brief Creator: Thanh Nguyen Ba, Hai Tu
 * @file      : cmsh.h
 * 
 * $Id: cmsh.h 824 2014-02-13 09:19:49Z thanh $     
 * $Author: thanh $ 
 * $Date: 2014-02-13 04:19:49 -0500 (Thu, 13 Feb 2014) $
 * $Log$
 * $Revision: 824 $
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
#include <stdarg.h>
struct cmsh;

typedef Int32T (*funcNameCb)(struct cmsh *, Int32T,  Int8T **, Int32T,  Int8T **, Int32T,  Int8T **, Int32T,  Int8T **, Int32T,  Int8T **, Int32T,  Int8T **);

struct cmshCallbackMap
{
    Uint32T funcID;
    funcNameCb func;
};

enum
{
  CMD_OUTMOD_NULL = 0,
  CMD_OUTMOD_BEGIN,   /** Begin for Output    */
  CMD_OUTMOD_INCLUDE, /** Include for Output  */
  CMD_OUTMOD_EXCLUDE  /** End for Output      */
};

struct modeInfo
{
    Uint32T mode;           /** ex. CMD_NODE_INTERFACE  */
    Uint32T modeArgc;       /** number of mode arguments  */
#define CMD_MAXLEN_MODEARG  64    /** Max Number mode node  */
    Int8T *modeArgv[CMD_MAXLEN_MODEARG];         /** MAX_LEN=64  */
};

#define CMSH_ARGC_MAX               128 /** Max Argc of cmsh over parser  */
#define CMSH_ARGV_MAX_LEN           256 /** Max Length of each input parameter  */

struct cmsh
{
    /** Current is cmsh or comp */
    Int32T gTypes; /** srcID  IPC_CMSH_MGR | OSPF | ...  */
    Int32T zTypes; /** dstID  IPC_CMSH_MGR | OSPF | ...  */

    /** CMD TREE  */
    struct cmdTree *ctree;

    /**  FLAGS   */
    Uint32T flags;
    /** User input string & parsing result */
    Int32T inputMode;
    Int8T *inputStr;
    Uint32T parseResult;        /** parsing return value (CMD_PARSE_OK, CMD_PARSE_INCOMPLETE, ...)  */

    /**  User output String */
    Int8T *outputStr;
    Uint32T  outputSize;

    /** Parse success. Information for calling callback func. */
    struct cmdElement *cel;      /** used in command execution  */
    struct cmdTreeNode *cnode;   /** used in command auto-completion and help */

    Uint32T argc;
    Int8T *argv[CMSH_ARGC_MAX];

#define CMSH_MAX_MATCH_NUMBER 1024  /** Max match number when help or autocompletion  */
    Int8T  *matchStr[CMSH_MAX_MATCH_NUMBER];
    Uint32T   matchNum;

    /**  Type Parse  */
    Uint32T typeParse;  /**  0 if command 1 if auto or help  */
    Uint32T pMode;
    /** Parse failed. Character pointer to show invalid input. */
    Int8T *invalid; 

    Uint32T invalidNum;

    Uint32T invalidIndex;
    /** Parse success for Output Modifier */
    Uint32T outMod; /** CMD_OUTMOD_NULL/BEGIN/INCLUDE/EXCLUDE */
    Int8T *outModArg;

    /** Output function & fd (see cmdPrint()) */
    Int32T (*outFunc)(void *, Int8T *, ...);
    Int32T (*oamFunc)(void *, Int8T *, ...);
    void *outFd;
    void *oamFd;

    /** requestKey as socket of cmsh  */
    Uint16T  requestKey;

    /** User session information */
    Int8T *userID; 
    Uint32T userKey;    /** unique number for user. (If thereâ€™re multiple user using same ID, this value identifies them)  */
    Uint8T userPrivilege;

    Uint32T lineType;       /** ex. CMD_NODE_LINE_VTY */
    Uint32T lineMin;      /** ex. 0 */
    Uint32T lineMax;      /** ex. 15  */

    /** Current Node     */
#define CMD_MAXNUM_MODEDEPTH    10  /** Max Depth of tree */
    struct modeInfo modeTrace[CMD_MAXNUM_MODEDEPTH];    /** MAX_DEPTH=10  */
    Uint32T currentDepth;

    Int8T *prompt;      /** prompt for current mode */
    Uint32T prevMode;   /** used for        */
    Int8T *hostName;
    Int8T *proStr;      /** Prompt string Return  */
    /** Current CMSH status.  */
    enum {
        CMSH_NORMAL,
        CMSH_CLOSE,
        CMSH_WAITING_RSP,   /** waiting response from other modules */
        CMSH_MORE,        /** prUint32T out waiting (â€œ|moreâ€)     */
    } status;
    /** Number Index of Functions */
    Int32T cmshFuncIndex; /** First Time from cmsh -> cm (0) */

    struct cmdList  *callbackList; /** Callback Function Link List  */

    /** CM IPC variable: fd connected to CM */
    Int32T cmfd;

    /** OAM Socket  */
    Int32T oamSock;

    /** Terminal length (width).  */
    Uint32T width;
};

struct cmsh *cmdCmshInit(Int32T gIndex);
struct cmsh *cmdCmshBaseInit(Int32T gIndex,Int32T zIndex);
void cmdCmshBaseFree(struct cmsh *cmsh);
void cmdCmshFree(struct cmsh *cmsh);

Int32T cmdOutPuts(void *outFd, Int8T *outStr,...);
Int32T cmdOamPuts(void *outFd, Int8T *outStr,...);

#define cmdPrint(cli, ...)                                                     \
  if((cli)->outFunc != NULL)                                                   \
  (*(cli)->outFunc)((cli)->outFd, __VA_ARGS__)

#define cmdOamPrint(...)                                 \
    cmdOamPuts((gCmshData->cmsh)->oamFd, __VA_ARGS__)
#endif
