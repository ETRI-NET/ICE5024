/**
 * @file      :  nnCmdNode.c
 * @brief       :  Install node and process function for node
 * 
 * $Id: cmdNode.c 933 2014-03-05 02:45:50Z thanh $      
 * $Author: thanh $ 
 * $Date: 2014-03-04 21:45:50 -0500 (Tue, 04 Mar 2014) $
 * $Log$
 * $Revision: 933 $
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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h> 
#include <termios.h>

#include "nnLog.h"
#include "nnTypes.h"
#include "nnStr.h"

#include "nnCmdLink.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"

Int8T nnCmdStrCheckIPv4Type(StringT str)
{
    Int8T strLen = 0;
    Int32T gIndex = 0;
    Int32T num = 0;
    Int32T dotCount = 0;

    strLen = strlen(str);

    /* Check Length */
    if (!(strLen >= MIN_IPV4_LEN && strLen <= MAX_IPV4_LEN))
    {
        return FALSE;
    }

    /* Check IPV4 one by one */
    for (gIndex = 0; gIndex < strLen; ++gIndex)
    {
        if ((isdigit(str[gIndex])))
        {
            num = num * 10 + (str[gIndex] - 48);
        }
        else if ((str[gIndex] == '.'))
        {
            if (num > 255)
            {
                return FALSE;
            }
            num = 0;
            ++dotCount;

            if (dotCount > MAX_IPV4_DOT_COUNT)
            {
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }
    return TRUE;
}

Int8T nnCmdStrCheckIPv4PrefixType(StringT str)
{
    Int8T strLen = 0;
    Int8T gIndex = 0;

    StringT slash = NULL;
    Int8T addr[MAX_IPV4_LEN + 1] = {0,};
    Int8T addrLen = 0;
    Int8T prefixLen = 0;
    Int32T num = 0;

    strLen = strlen(str);

    if ((slash = strchr(str, '/')) != NULL)
    {
        addrLen = (slash - str);
        prefixLen = strLen - (slash - str) - 1;

        if (!(prefixLen >= MIN_IPV4_PREFIX_LEN &&
            prefixLen <= MAX_IPV4_PREFIX_LEN))
        {
            return FALSE;
        }

        strncpy(addr, str, addrLen);

        // Check Prefix
        for (gIndex = 0; gIndex < prefixLen; ++gIndex)
        {
            if(!(isdigit(str[strLen - prefixLen + gIndex])))
            {
                return FALSE;
            }
            else
            {
                num = num * 10 + (str[strLen - prefixLen + gIndex] - 48);
            }
        }

        if (num > MAX_IPV4_PREFIX_NUM)
        {
            return FALSE;
        }
        // Check IPV4
        if (nnCmdStrCheckIPv4Type(addr) == FALSE)
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

Int8T nnCmdStrCheckIPv6Type(StringT str)
{
    Int8T strLen = 0;
    Int32T gIndex = 0;

    Int8T reduce = 0;
    Int8T numCount = 0;
    Int8T colonCount = 0;
    StringT tempStr = NULL;

    tempStr = str;

    if (!(strncmp("::", str, 2)))
    {
        if (strlen(tempStr) == 2)
        {
            return TRUE;
        }

        reduce = 1;

        tempStr += 2;
    }

    strLen = strlen(tempStr);

    if (strLen > MAX_IPV6_LEN - (reduce * 4))
    {
        return FALSE;
    }
    for (gIndex = 0; gIndex < strLen; ++gIndex)
    {
        if (!(isxdigit(tempStr[gIndex])))
        {
            if (tempStr[gIndex] != ':')
            {
                return FALSE;
            }
            else
            {
                if (numCount == 0)
                {
                    if (reduce == 1)
                    {
                        return FALSE;
                    }
                    else
                    {
                        reduce = 1;
                    }
                }
                numCount = 0;
                ++colonCount;
            }
        }
        else
        {
            ++numCount;

            if (numCount > 4)
            {
                return FALSE;
            }
        }
    }

    if (colonCount > MAX_IPV6_COLON_COUNT)
    {
        if (strncmp("::", (tempStr + gIndex - 2), 2))
        {
            return FALSE;
        }
    }
    else if (numCount == 0 &&
            !(tempStr[gIndex - 1] == ':' &&
            tempStr[gIndex - 2] == ':'))
    {
        return FALSE;
    }

    return TRUE;
}

Int8T nnCmdStrCheckIPv6PrefixType(StringT str)
{
    Int8T strLen = 0;
    Int8T gIndex = 0;

    StringT slash = NULL;
    Int8T addr[MAX_IPV6_LEN + 1] = {0,};
    Int8T addrLen = 0;
    Int8T prefixLen = 0;
    Int32T num = 0;

    strLen = strlen(str);

    if ((slash = strchr(str, '/')) != NULL)
    {
        addrLen = (slash - str);
        prefixLen = strLen - (slash - str) - 1;

        if (!(prefixLen >= MIN_IPV6_PREFIX_LEN &&
            prefixLen <= MAX_IPV6_PREFIX_LEN))
        {
           return FALSE;
        }

        strncpy(addr, str, addrLen);

        // Check Prefix
        for (gIndex = 0; gIndex < prefixLen; ++gIndex)
        {
            if(!(isdigit(str[strLen - prefixLen + gIndex])))
            {
                return FALSE;
            }
            else
            {
                num = num * 10 + (str[strLen - prefixLen + gIndex] - 48);
            }
        }
        if (num > MAX_IPV6_PREFIX_NUM)
        {
            return FALSE;
        }

        // Check IPv6
        if (nnCmdStrCheckIPv6Type(addr) == FALSE)
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

Int8T nnCmdStrCheckTime(StringT str)
{
    Int8T strLen = 0;
    Int32T gIndex = 0;
    strLen = strlen(str);

    if(strLen == 8)
    {
        for(gIndex = 0; gIndex < 8; ++gIndex)
        {
            if ((gIndex % 3) == 2)
            {
                if (str[gIndex] != ':')
                {
                    return FALSE;
                }
            }
            else
            {
                if(!isdigit(str[gIndex]))
                {
                    return FALSE;
                }
            }
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}
Int8T nnCmdStrCheckMacAddressType(StringT str)
{
    Int8T strLen = 0;
    Int32T gIndex = 0;

    strLen = strlen(str);

    if (strLen == 17)
    {
        for(gIndex = 0; gIndex < 17; ++gIndex)
        {
            if ((gIndex % 3) == 2)
            {
                if (str[gIndex] != ':')
                {
                    return FALSE;
                }
            }
            else
            {
                if(!(isxdigit(str[gIndex])))
                {
                    return FALSE;
                }
            }
        }
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

/**
 * Description: Function cut a first token from inputted command string
 *
 * @param [in]   str : Command string input
 * @param [out]  type : Token type output
 * @param [out]  val: Command string output
 *
 * @retval : cp
 */
Int8T *
cmdGetToken (Int8T *str, enum cmdToken *type, Int32T * flags, Int8T **val)
{
  Int32T i = 0;
  Int8T *cp = str;
  *flags = 0;
  static Int8T buf[CMD_TOKEN_LENGTH];
  bzero(&buf, CMD_TOKEN_LENGTH);
  if(!str)
  {
    *type = CMD_TOKEN_NULL;
    return NULL;
  }
  /** Skip white spaces.  */
  while (isspace ((Int32T) *cp) && *cp != '\0')
    cp++, str++;

  /** Only white spaces, return NULL.  */
  if (*cp == '\0')
  {
    *type = CMD_TOKEN_NULL;
    return NULL;
  }
  /** Check special character.  */
  switch (*cp)
  {
    case '(':
      *type = CMD_TOKEN_PAREN_OPEN;
      return ++cp;
    case ')':
      *type = CMD_TOKEN_PAREN_CLOSE;
      return ++cp;

    case '{':
      *type = CMD_TOKEN_CBRACE_OPEN;
      return ++cp;

    case '}':
      *type = CMD_TOKEN_CBRACE_CLOSE;
      return ++cp;

    case '[':
      *type = CMD_TOKEN_BRACE_OPEN;
      return ++cp;

    case ']':
      *type = CMD_TOKEN_BRACE_CLOSE;
      return ++cp;

    case '`':
      *type = CMD_TOKEN_IFNAME_OPEN;
      return ++cp;

    case '\'':
      *type = CMD_TOKEN_IFNAME_CLOSE;
      return ++cp;

    case '|':
      /** '||' treated as keyword '|' for output modifier pipe.  */
      if (*(cp + 1) == '|')
      {
        buf[0] = '|';
        buf[1] = '\0';
        *val = buf;
        *type = CMD_TOKEN_PIPE;
        return (cp + 2);
      } else {
        *type = CMD_TOKEN_SEPARATOR;
        return ++cp;
      }

    case '>':
      *type = CMD_TOKEN_REDIRECT;
      buf[0] = '>';
      buf[1] = '\0';
      *val = buf;
      return ++cp;

    case '*':
      buf[0] = '*';
      buf[1] = '\0';
      *val = buf;
      *type = CMD_TOKEN_KEYWORD;
      return ++cp;

    case '.':
      *type = CMD_TOKEN_DOT;
      return ++cp;

    case '?':
      *type = CMD_TOKEN_QUESTION;
      return ++cp;
  }
  /** Range value.  */
  if (*cp == '<')
  {
    while (*cp == '<' || *cp == '>' || *cp == '-' || *cp == '+' || *cp == '/'|| isalnum ((Int32T)*cp))
    {
      buf[i++] = *cp++;
    }
    buf[i] = '\0';
    *val = buf;
    *type = CMD_TOKEN_RANGE;
    return cp;
  }

  /** Special words.  */
  if (isupper ((Int32T)*cp))
  {
    while (isalnum ((Int32T)*cp) || *cp == '.' || *cp == '/' || *cp == ':' || *cp == '_' || *cp == '-')
    {
      buf[i++] = *cp++;
    }
    buf[i] = '\0';
    *val = buf;
    if (strcmp (buf, "LINE") == 0)
      *type = CMD_TOKEN_LINE;
    else if (strcmp (buf, "WORD") == 0)
      *type = CMD_TOKEN_WORD;
    else if (strcmp (buf, "IFNAME") == 0)
      *type = CMD_TOKEN_IFNAME;
    else if (strcmp (buf, "A.B.C.D") == 0)
      *type = CMD_TOKEN_IPV4;
    else if (strcmp (buf, "A.B.C.D/M") == 0)
      *type = CMD_TOKEN_IPV4_PREFIX;
    else if (strcmp (buf, "X:X::X:X") == 0)
      *type = CMD_TOKEN_IPV6;
    else if (strcmp (buf, "X:X::X:X/M") == 0)
      *type = CMD_TOKEN_IPV6_PREFIX;
    else if (strcmp (buf, "HH:MM:SS") == 0)
      *type = CMD_TOKEN_TIME;
    else if (strcmp (buf, "AA:NN") == 0)
      *type = CMD_TOKEN_COMMUNITY;
    else if (strcmp (buf, "XX:XX:XX:XX:XX:XX") == 0)
      *type = CMD_TOKEN_MAC_ADDRESS;
    else
      *type = CMD_TOKEN_WORD;
    return cp;
  }
  /** Keyword.  */
  if (isalnum ((Int32T)*cp))
  {

    while (isalnum ((Int32T)*cp) || *cp == '.' || *cp == '/' || *cp == ':' || *cp == '-' || *cp == '@') 
    {
      buf[i++] = *cp++;
    }
    buf[i] = '\0';
    *val = buf;
    *type = CMD_TOKEN_KEYWORD;
    return cp;
  }
   /** Keyword HIRE  */
  if(*cp == '&' && *(cp+1) == '&'){
	 *flags = 1;
     cp = cp + 2;
    while (isalnum ((Int32T)*cp) || *cp == '.' || *cp == '/' || *cp == ':' || *cp == '-' || *cp == '@')
    {
      buf[i++] = *cp++;
    }
    buf[i] = '\0';
    *val = buf;
    *type = CMD_TOKEN_KEYWORD;
    return cp;	
  }
  *type = CMD_TOKEN_NULL;
  return NULL;
}

/**
 * Description: Function returns number of token in inputted command string
 *
 * @param [in]  cmdStr: Input string 
 * @param [out] none
 *
 * @retval : tokenCount
 */
Int32T
cmdGetTokenNum(char* cmdStr)
{
  Int32T tokenCount = 0, flags;
  char* remainStr = cmdStr;
  char* temp;
  enum cmdToken type;
  while((remainStr = cmdGetToken(remainStr, &type, &flags, &temp)))
  {
    tokenCount++;
  }
  return tokenCount;
}
/**
 * Description: Function returns number of token which has a help string
 *
 * @param [in]  cmdStr: Input string help
 * @param [out] none
 *
 * @retval : tokenCount
 */
Int32T
cmdGetTokenNumHelp(char* cmdStr)
{
  Int32T tokenCount = 0, flags = 0;
  char* remainStr = cmdStr;
  char* temp;
  enum cmdToken type;
  while((remainStr = cmdGetToken(remainStr, &type, &flags, &temp)) != NULL)
  {
    if( (type == CMD_TOKEN_PAREN_OPEN)    ||
        (type == CMD_TOKEN_PAREN_CLOSE)   ||
        (type == CMD_TOKEN_CBRACE_OPEN)   ||
        (type == CMD_TOKEN_CBRACE_CLOSE)  ||
        (type == CMD_TOKEN_BRACE_OPEN)    ||
        (type == CMD_TOKEN_BRACE_CLOSE)   ||
        (type == CMD_TOKEN_IFNAME_OPEN)   ||
        (type == CMD_TOKEN_IFNAME_CLOSE)  ||
        (type == CMD_TOKEN_SEPARATOR)     ||
        (type == CMD_TOKEN_PIPE)          ||
        (type == CMD_TOKEN_REDIRECT)      ||
        (type == CMD_TOKEN_DOT)           ||
        (type == CMD_TOKEN_QUESTION))
    {
      continue;
    }
    tokenCount++;
  }
  return tokenCount;
}
/**
 * Description: Get Token Help Index
 *
 * @param [in]  cel  : struct cmdElement input
 * @param [in]  type  : type Token
 * @param [out] tokenStr  : string token
 *
 * @retval : none
 */
Int32T
cmdGetTokenHelpIndex(struct cmdElement *cel, enum cmdToken type, Int8T *tokenStr)
{
  enum cmdToken tokenType;
  Int8T *tmpStr = cel->str;
  Int8T *temp;
  Int8T *val = strdup(tokenStr);
  Int32T zIndex = 0, flags = 0;
  while((tmpStr = cmdGetToken(tmpStr, &tokenType, &flags, &temp)) != NULL)
  {
    switch(type)
    {
      case CMD_TOKEN_PAREN_OPEN:
      case CMD_TOKEN_PAREN_CLOSE:
      case CMD_TOKEN_CBRACE_OPEN:
      case CMD_TOKEN_CBRACE_CLOSE:
      case CMD_TOKEN_BRACE_OPEN:
      case CMD_TOKEN_BRACE_CLOSE:
      case CMD_TOKEN_IFNAME_OPEN:
      case CMD_TOKEN_IFNAME_CLOSE:
      case CMD_TOKEN_SEPARATOR:
      case CMD_TOKEN_PIPE:
      case CMD_TOKEN_REDIRECT:
      case CMD_TOKEN_DOT:
      case CMD_TOKEN_QUESTION:
      break;
      default:
        if((type == tokenType) &&
          (!strcmp(val, temp)))
        {
          return zIndex;
        }
        zIndex++;
      break;
    }
  }
  return -1;
}


/**
 * Description: cmdNodeLookup
 *
 * @param [in]  currentNode  : Node curent
 * @param [in]  cmdStr  : string input
 * @param [out] none 
 *
 * @retval : cmdTreeNode structure
 */
struct cmdTreeNode *
cmdNodeLookup(struct cmdTreeNode *currentNode, Int8T *cmdStr)
{
  struct cmdTreeNode *node;
  struct cmdListNode *nn;
  CMD_MANAGER_LIST_LOOP(currentNode->cNodeList, node, nn)
  {
    if(!strcmp(node->tokenStr, cmdStr))
    {
      return node;
    }
  }
  return NULL;
}
/**
 * Description: Add new node for command to existing parsing tree
 *
 * @param [in]  currentNode  : Node curent
 * @param [in]  type  : type token
 * @param [in]  tokenStr: string token
 * @param [out] none 
 *
 * @retval : cmdTreeNode node new
 */
struct cmdTreeNode *
cmdAddNode(struct cmdTreeNode *currentNode, enum cmdToken type, Int32T flags, Int8T *tokenStr)
{
  struct cmdTreeNode *node;
  if((node = cmdNodeLookup(currentNode, tokenStr)) == NULL)
  {
    node = malloc(sizeof(*node));
    node->type = type;
	node->flags = flags;
    node->tokenStr = strdup(tokenStr);
    node->cNodeList = cmdListNew();
    /** New Version */
    node->cCelTable = cmdListNew();
    node->cHelpTable = cmdListNew();
    /** END         */
    currentNode->refCnt++;
    cmdListAddNode(currentNode->cNodeList, node);
  }
  return node;
}
/** New Version */
void
cmdAddHelpStr(struct cmdTreeNode *currentNode, Int8T *helpStr)
{
  struct cmdListNode *nn;
  Int8T *hStr;
  Int8T *szHStr = strdup(helpStr);
  CMD_MANAGER_LIST_LOOP(currentNode->cHelpTable, hStr, nn)
  {
    if(!strcmp(hStr, szHStr))
    {
      return;
    }
  }
  cmdListAddNode(currentNode->cHelpTable, szHStr);
}

void
cmdAddCel(struct cmdTreeNode *currentNode, struct cmdElement *cel)
{
  cmdListAddNode(currentNode->cCelTable, cel);
}
/** End */
/**
 * Description: Function to add new branch for command to existing parsing tree and connect cmdElement
 *
 * @param [in]  currentNode  : Current tree node of recursive traversing
 * @param [in]  cel  : New command structure
 * @param [in]  cmdStr : Remained command string
 * @param [in] depth : Recursive depth
 * @param [out] none 
 *
 * @retval : code
 */
Int32T
cmdBuildRecursive(struct cmdTreeNode *currentNode, 
  struct cmdElement *cel, Int8T *cmdStr, Int32T depth)
{
  struct cmdTreeNode *tmpNode = currentNode;
  Int8T *tokenStr;
  enum cmdToken type;
  Int32T flags = 0;
  	
  cmdStr = cmdGetToken(cmdStr, &type, &flags, &tokenStr);
  switch(type)
  {
    case CMD_TOKEN_PAREN_OPEN:
    case CMD_TOKEN_CBRACE_OPEN:
    case CMD_TOKEN_BRACE_OPEN:
    case CMD_TOKEN_IFNAME_OPEN:
    {
      Int32T i, nodeNumber = 0;
      struct cmdTreeNode *nListNode[1024];
      while((cmdStr = cmdGetToken(cmdStr, &type, &flags, &tokenStr)) != NULL)
      {
        if(type == CMD_TOKEN_PAREN_CLOSE ||
           type == CMD_TOKEN_CBRACE_CLOSE||
           type == CMD_TOKEN_BRACE_CLOSE||
           type == CMD_TOKEN_IFNAME_CLOSE)
        {
          break;
        }
        if(type != CMD_TOKEN_SEPARATOR)
        {
          nListNode[nodeNumber] = cmdAddNode(tmpNode, type, flags, tokenStr);
          cmdAddHelpStr(nListNode[nodeNumber], cel->help[depth+nodeNumber]);  /** New Version */
          nodeNumber++;
        }
      }
      for(i = 0 ; i < nodeNumber; i++)
      {
        cmdBuildRecursive(nListNode[i], cel, cmdStr, depth + nodeNumber);
      }
    }
    break;
    case CMD_TOKEN_PAREN_CLOSE:
    case CMD_TOKEN_CBRACE_CLOSE:
    case CMD_TOKEN_BRACE_CLOSE:
    case CMD_TOKEN_IFNAME_CLOSE:
    case CMD_TOKEN_SEPARATOR:
    break;
    case CMD_TOKEN_NULL:
      cmdAddCel(tmpNode, cel);  /** New Version */
    break;
    default:
      tmpNode = cmdAddNode(tmpNode, type, flags, tokenStr);
      cmdAddHelpStr(tmpNode, cel->help[depth]); /** New Version */
      /**  Install Range Here  */
      return cmdBuildRecursive(tmpNode, cel, cmdStr, depth + 1);
    break;
  }
  return CMD_IPC_OK;
}

/**
 * Description: Convert string to int
 *
 * @param [in]  tr: string input
 * @param [out] value: value Int32T output
 *
 * @retval : TRUE: success, FALSE: false
 */
static Int32T
cmdAtoi(Int8T *str, Uint32T *value)
{
  Int32T i;
  for(i = 0;  i < strlen(str); i++)
  {
    if(!isdigit(str[i]))
    {
      return FALSE;
    }
  }
  *value = atoi(str);
  return TRUE;
}
/**
 * Description: Check value of range value
 *
 * @param [in]  s: min value
 * @param [in]  d: max value  
 * @param [out] none
 *
 * @retval : TRUE: success, FALSE: false
 */
static Int32T
cmdMatchRangeCheck(Int8T *s, Int8T *d)
{
    Uint32T start, end, value;
    if(sscanf(s, "<%u-%u>", &start, &end) == 2)
    {
      if(cmdAtoi(d, &value) == TRUE)
      {
        if((value <= end) && (value >= start))
        {
          return TRUE;
        }
      }
    }
    return FALSE;
}
/**
 * Description: cmdNodeShortenCommandIndex
 *
 * @param [in]  currentNode  : Current tree node of recursive traversing
 * @param [in]  cmdStr  : string input
 * @param [out] zIndex  : zIndex output
 *
 * @retval : prevNode
 */
struct cmdTreeNode *
cmdNodeShortenCommandIndex(struct cmdTreeNode *currentNode, Int8T *cmdStr, Int32T *zIndex)
{
  struct cmdTreeNode *node = NULL, *prevNode = NULL;
  struct cmdListNode *nn;
  Int32T zIndexNumber = 0;
  
  CMD_MANAGER_LIST_LOOP(currentNode->cNodeList, node, nn)
  {
    if(!strncmp(node->tokenStr, cmdStr, strlen(cmdStr)))
    {
      prevNode = node;
      zIndexNumber++;
    }
  }
  *zIndex = zIndexNumber;
  return prevNode;
}
/**
 * Description: cmdMatchCheck
 *
 * @param [in]  cmsh  : cmsh structure
 * @param [in]  currentNode  : Current tree node of recursive traversing
 * @param [in]  tokenStr : string token
 * @param [in]  mode : mode check
 * @param [out] none
 *
 * @retval : node
 */
struct cmdTreeNode *
cmdMatchCheck(struct cmsh *cmsh, struct cmdTreeNode *currentNode, Int8T *tokenStr, Int32T mode)
{
  struct cmdTreeNode *node = NULL, *prevNode = NULL;
  struct cmdListNode *nn;
  Int32T iNumToken = 0;
  CMD_MANAGER_LIST_LOOP(currentNode->cNodeList, node, nn)
  {
    switch(node->type)
    {
      case CMD_TOKEN_KEYWORD:
        if(!strcmp(node->tokenStr, tokenStr))
        {
          return node;
        } else if(!strncmp(node->tokenStr, tokenStr, strlen(tokenStr)) && mode == 0) { /**  For Help and Auto Completion */
          iNumToken++;
          prevNode = node;
        }
      break;
      default:
      break;
    }
  }
  if(iNumToken == 1 && prevNode != NULL && mode == 0)
  {
    return prevNode;
  }

  CMD_MANAGER_LIST_LOOP(currentNode->cNodeList, node, nn)
  {
    switch(node->type)
    {
      case CMD_TOKEN_NULL:
      break;
      case CMD_TOKEN_PIPE:
      break;
      case CMD_TOKEN_REDIRECT:
      break;
      case CMD_TOKEN_DOT:
      break;
      case CMD_TOKEN_QUESTION:
      break;
      case CMD_TOKEN_RANGE:
        if(cmdMatchRangeCheck(node->tokenStr, tokenStr) == TRUE)
        {
          return node;
        }
      break;
      case CMD_TOKEN_ALIAS:
      break;
      case CMD_TOKEN_LINE:
        return node;
      break;
      case CMD_TOKEN_WORD:
        return node;
      break;
      case CMD_TOKEN_IPV4:
        if(nnCmdStrCheckIPv4Type(tokenStr) == TRUE)
        {
          return node;
        }
      break;
      case CMD_TOKEN_IPV4_PREFIX:
        if(nnCmdStrCheckIPv4PrefixType(tokenStr) == TRUE)
        {
          return node;
        }
      break;
      case CMD_TOKEN_IPV6:
        if(nnCmdStrCheckIPv6Type(tokenStr) == TRUE)
        {
          return node;
        }
      break;
      case CMD_TOKEN_IPV6_PREFIX:
        if(nnCmdStrCheckIPv6PrefixType(tokenStr) == TRUE)
        {
          return node;
        }
      break;
      case CMD_TOKEN_TIME:
        if(nnCmdStrCheckTime(tokenStr) == TRUE)
        {
          Int32T hh, mm, ss;
          if(sscanf(tokenStr, "%d:%d:%d", &hh, &mm, &ss) == 3)
          {
            if((hh >= 0 && hh <= 23) && 
                (mm >=0 && mm <= 59) && 
                (ss >=0 && ss <= 59))
            {
              return node;
            }
          }
        }
      break;
      case CMD_TOKEN_COMMUNITY:
      break;
      case CMD_TOKEN_MAC_ADDRESS:
        if(nnCmdStrCheckMacAddressType(tokenStr) == TRUE)
        {
          return node;
        }
      break;
      case CMD_TOKEN_IFNAME:
      break;
      case CMD_TOKEN_UNKNOWN:
      break;
    default:
    break;
    }
  }
  if(iNumToken == 1 && prevNode != NULL && mode == 0)
  {
    return prevNode;
  }
  cmsh->invalidNum = iNumToken;
  return NULL;
}
/**
 * Description: Function to do parsing tree traversing
 *
 * @param [in]  cmsh: Command structure
 * @param [in]  currentNode: Tree node which starting poInt32T of the parsing
 * @param [in]  remainStr: Command string for parsing
 * @param [in]  depth:  recursive depth
 * @param [out] none
 *
 * @retval : none
 */
Int32T
cmdParseRecursive(struct cmsh *cmsh, struct cmdTreeNode *currentNode, Int8T *remainStr, Int32T depth)
{
  enum cmdToken tokenType;
  Int8T *tokenStr;
  Int32T mode, flags;
  Int8T *tmpStr = remainStr;
  Int8T *tmpLineStr = tmpStr;
  struct cmdTreeNode *tmpNode = currentNode;
  cmsh->cnode = currentNode;
  if(strlen(tmpStr) == 0)
  {
    return CMD_PARSE_NULL;
  }
  cmsh->invalidIndex = 0;
  while((tmpStr = cmdGetToken(tmpStr, &tokenType, &flags, &tokenStr)) != NULL)
  {

    if(cmsh->typeParse)
    {
      if(tmpStr[0] == ' ')
      {
        mode = 0;
      }
      else
      {
        mode = 1;
      }
    }
    else
    {
      mode = 0;
    }
    tmpNode = cmdMatchCheck(cmsh, tmpNode, tokenStr, mode);
    if(tmpNode == NULL)
    {
      cmsh->invalid = tokenStr;
      break;
    }

    cmsh->invalidIndex += strlen(tokenStr) + 1;

    cmsh->cnode = tmpNode;
    if(tmpNode->type == CMD_TOKEN_LINE)
    {
      cmsh->argv[cmsh->argc] = strdup(tmpLineStr);
      cmsh->argc++;
      break;
    }
    switch (tmpNode->type)
    {
        case CMD_TOKEN_RANGE:       /** 1-65535 */  ///J1.Choi: removed ArrowHead around Number 1-65535.
        case CMD_TOKEN_IPV4:        /** A.B.C.D */
        case CMD_TOKEN_IPV4_PREFIX:     /** A.B.C.D/M */
        case CMD_TOKEN_IPV6:        /** X:X::X:X  */
        case CMD_TOKEN_IPV6_PREFIX:     /** X:X::X:X/M  */
        case CMD_TOKEN_MAC_ADDRESS:     /** XX:XX:XX:XX:XX:XX */
        case CMD_TOKEN_IFNAME:  /** IFNAME  */
        case CMD_TOKEN_WORD:
        case CMD_TOKEN_TIME:
            cmsh->argv[cmsh->argc] = strdup(tokenStr);
            cmsh->argc++;
        break;
        case CMD_TOKEN_KEYWORD:
            cmsh->argv[cmsh->argc] = strdup(tmpNode->tokenStr);
            cmsh->argc++;
        break;
    default:
    break;
    }
    tmpLineStr = tmpStr;
  }
  if(cmsh->cnode->type == CMD_TOKEN_LINE)
  {
    return CMD_PARSE_SUCCESS;
  }
  if((cmsh->cnode != NULL) && (tmpStr == NULL) && (CMD_MANAGER_LIST_COUNT(cmsh->cnode->cCelTable) == 0))
  {
    cmsh->pMode = mode;
    return CMD_PARSE_INCOMPLETE;
  }
  else if((cmsh->cnode != NULL) && (tmpStr == NULL) && (CMD_MANAGER_LIST_COUNT(cmsh->cnode->cCelTable) > 0))
  {
      return CMD_PARSE_SUCCESS;
  }
  else 
  {
    return CMD_PARSE_FAIL;
  }
  return CMD_PARSE_NULL;
}
/**
 * Description: Parse Command line
 *
 * @param [in]  cmsh:  User’s input command (cmsh.inputStr), Comamnd parsing tree (cmsh.ctree), User’s mode (cmsh.mode) 
 * @param [out] none
 *
 * @retval : code
 */
Int32T
cmdParseCommand(struct cmsh *cmsh)
{
  Int8T *cmdStr = cmsh->inputStr;
  Int32T rv = CMD_PARSE_NULL;
  struct cmdTreeNode *startNode = &cmsh->ctree->modeTree[cmsh->modeTrace[cmsh->currentDepth].mode];
  cmsh->argc = 0;
  cmsh->typeParse = 0;
  if(cmdStr == NULL)
    return CMD_PARSE_SUCCESS; 
  rv = cmdParseRecursive(cmsh, startNode, cmdStr, 0);
  switch(rv)
  {
    case CMD_PARSE_INCOMPLETE:
    {
      cmdPrint(cmsh, " %% Incomple Command Line.\n");
      break;
    }
    case CMD_PARSE_FAIL:
    {
      if(cmsh->hostName != NULL)
      {
        Int32T pSize = strlen(cmsh->hostName);
        if(cmsh->currentDepth > 1)  /**  View Depth  */
        {
          pSize +=2;
        } else {
          pSize -= strlen(cmsh->prompt);
        }
        if(cmsh->prompt != NULL)
        {
          cmdPrint(cmsh, "%*c^\n", pSize + strlen(cmsh->prompt) + cmsh->invalidIndex + 1, ' ');
        }
        cmdPrint(cmsh, "%% Invalid command at '^'\n\n");
      }
    }
    break;
  }
	return rv;
}
#define CMD_SPACE_NUMBER   5  /** Number space for align help and token */
/**
 * Description: use cmdParseRecursive function to traverse parsing tree. The result of parsing 
 *        is two cases. One is the case that user complete a token and pressed tab. 
 *        In this case, find that token’s childs and show them as a possible next token
 *
 * @param [in]  cmsh: User input command (cmsh.inputStr), Command parsing tree (cmsh.ctree), 
 *            User mode (cmsh.mode)
 * @param [out] none
 *
 * @retval : Int8T string
 */
Int8T **
cmdParseAutoCompletion (struct cmsh *cmsh)
{
  Int32T rv, nodeIndex = cmsh->modeTrace[cmsh->currentDepth].mode;
  struct cmdTreeNode *node = &cmsh->ctree->modeTree[nodeIndex];
  struct cmdTreeNode *sNode;
  struct cmdListNode *nn;
  cmsh->matchNum = 0;
  cmsh->typeParse = 1;
  rv = cmdParseRecursive(cmsh, node, cmsh->inputStr, 0);
  switch(rv)
  {
    case CMD_PARSE_SUCCESS:
      if(CMD_MANAGER_LIST_COUNT(cmsh->cnode->cNodeList) != 0)
      {
        CMD_MANAGER_LIST_LOOP(cmsh->cnode->cNodeList, sNode, nn)
        {
			if(cmsh->cnode->flags == 0)
              cmdPrint(cmsh,"\n [%s]", sNode->tokenStr);
        }
      }
	  cmdPrint(cmsh,"\n  <cr>\n"); 
      break;
    case CMD_PARSE_INCOMPLETE:
      if(CMD_MANAGER_LIST_COUNT(cmsh->cnode->cNodeList) == 1)
      {
          if(cmsh->pMode == 0)
          {
            CMD_MANAGER_LIST_LOOP(cmsh->cnode->cNodeList, sNode, nn)
            {
              if(sNode->type == CMD_TOKEN_KEYWORD && sNode->flags == 0)
              {
                cmsh->matchStr[cmsh->matchNum++] = strdup(sNode->tokenStr);
              }else {
				if(sNode->flags == 0)
                	cmdPrint(cmsh,"\n %s", sNode->tokenStr);
              }
            }
          }else{
            if(cmsh->cnode->type == CMD_TOKEN_KEYWORD && cmsh->cnode->flags == 0)
            {
              cmsh->matchStr[cmsh->matchNum++] = strdup(cmsh->cnode->tokenStr);
            }
          }
      }
      else
      {
        cmdPrint(cmsh,"\n");
        CMD_MANAGER_LIST_LOOP(cmsh->cnode->cNodeList, sNode, nn)
        {
			if(sNode->flags == 0)
            	cmdPrint(cmsh," %s", sNode->tokenStr);
        }
      }
      break;
    case CMD_PARSE_FAIL:
      if(cmsh->invalidNum == 0)
      {
        CMD_MANAGER_LIST_LOOP(cmsh->cnode->cNodeList, sNode, nn)
        {
          if(sNode->type == CMD_TOKEN_KEYWORD && sNode->flags == 0)
          {
            if(!strncmp(sNode->tokenStr, cmsh->invalid, strlen(cmsh->invalid)))
            {
              cmsh->matchStr[cmsh->matchNum++] = strdup(sNode->tokenStr);
            }
          }
        }
        if(cmsh->matchNum == 0)
        {
          cmdPrint(cmsh,"\n");
          CMD_MANAGER_LIST_LOOP(cmsh->cnode->cNodeList, sNode, nn)
          {
			  if(sNode->flags == 0)
              	cmdPrint(cmsh," %s", sNode->tokenStr);
          }
        }
      }
      break;
  }
  cmsh->matchStr[cmsh->matchNum] = NULL;
  return cmsh->matchStr;
}
/**
 * Description: Show help function
 *
 * @param [in]  cmsh: User inputted command (cmsh.inputStr), Command parsing tree (cmsh.ctree), 
 *        User mode (cmsh.mode) 
 * @param [out] none
 *
 * @retval : code
 */
Int32T
cmdParseHelp (struct cmsh *cmsh)
{
  Int32T rv, nodeIndex = cmsh->modeTrace[cmsh->currentDepth].mode;
  struct cmdTreeNode *node = &cmsh->ctree->modeTree[nodeIndex];
  struct cmdTreeNode *sNode;
  struct cmdListNode *nn, *hNN;
  Int8T *szHelpStr;
  Int32T szIndex, numMaxSpace = 0;
  cmsh->typeParse = 1;
  rv = cmdParseRecursive(cmsh, node, cmsh->inputStr, 0);
  switch(rv)
  {
    case CMD_PARSE_SUCCESS:
      if(CMD_MANAGER_LIST_COUNT(cmsh->cnode->cNodeList) != 0)
      {
        CMD_MANAGER_LIST_LOOP(cmsh->cnode->cNodeList, sNode, nn)
        {
          numMaxSpace = (numMaxSpace > strlen(sNode->tokenStr)) ? numMaxSpace : strlen(sNode->tokenStr);
        }
        cmdPrint(cmsh,"\n");
        CMD_MANAGER_LIST_LOOP(cmsh->cnode->cNodeList, sNode, nn)
        {
			if(sNode->flags == 0){
			  cmdPrint(cmsh, "  %s", sNode->tokenStr);
			  szIndex = 0;
			  CMD_MANAGER_LIST_LOOP(sNode->cHelpTable, szHelpStr, hNN)
			  {
				if(szIndex == 0)
				{
				  cmdPrint(cmsh, "%*c", numMaxSpace - strlen(sNode->tokenStr) + 5, ' ');
				} else {
				  cmdPrint(cmsh, "%*c", numMaxSpace + 7, ' ');
				}
				cmdPrint(cmsh, "%s\n", szHelpStr);
				szIndex++;
			  }
			}
		}
      } else {
          cmdPrint(cmsh,"\n");
      }
      if(cmsh->cel )
      {
            cmdPrint(cmsh,"  <cr>\n");
      }
      break;
    case CMD_PARSE_INCOMPLETE:
      cmdPrint(cmsh,"\n");
      CMD_MANAGER_LIST_LOOP(cmsh->cnode->cNodeList, sNode, nn)
      {
        numMaxSpace = (numMaxSpace > strlen(sNode->tokenStr)) ? numMaxSpace : strlen(sNode->tokenStr);
      }
      CMD_MANAGER_LIST_LOOP(cmsh->cnode->cNodeList, sNode, nn)
      {
		if(sNode->flags == 0){
			  cmdPrint(cmsh, "  %s", sNode->tokenStr);
			  szIndex = 0;
			  CMD_MANAGER_LIST_LOOP(sNode->cHelpTable, szHelpStr, hNN)
			  {
				if(szIndex == 0)
				{
				  cmdPrint(cmsh, "%*c", numMaxSpace - strlen(sNode->tokenStr) + 5, ' ');
				} else {
				  cmdPrint(cmsh, "%*c", numMaxSpace + 7, ' ');
				}
				cmdPrint(cmsh, "%s\n", szHelpStr);
				szIndex++;
			  }
		}
      }
      break;
    case CMD_PARSE_FAIL:
      cmdPrint(cmsh,"\n");
      if(cmsh->invalidNum == 0)
      {
        CMD_MANAGER_LIST_LOOP(cmsh->cnode->cNodeList, sNode, nn)
        {
          numMaxSpace = (numMaxSpace > strlen(sNode->tokenStr)) ? numMaxSpace : strlen(sNode->tokenStr);
        }
        CMD_MANAGER_LIST_LOOP(cmsh->cnode->cNodeList, sNode, nn)
        {
          if(!strncmp(sNode->tokenStr, cmsh->invalid, strlen(cmsh->invalid)) && sNode->flags == 0)
          {
            cmdPrint(cmsh, "  %s", sNode->tokenStr);
            szIndex = 0;
            CMD_MANAGER_LIST_LOOP(sNode->cHelpTable, szHelpStr, hNN)
            {
              if(szIndex == 0)
              {
                cmdPrint(cmsh, "%*c", numMaxSpace - strlen(sNode->tokenStr) + 5, ' ');
              } else {
                cmdPrint(cmsh, "%*c", numMaxSpace + 7, ' ');
              }
              cmdPrint(cmsh, "%s\n", szHelpStr);
            }
            szIndex++;
          }
        }
      }
      break;
    default:
      cmdPrint(cmsh,"\n");
      CMD_MANAGER_LIST_LOOP(cmsh->cnode->cNodeList, sNode, nn)
      {
        numMaxSpace = (numMaxSpace > strlen(sNode->tokenStr)) ? numMaxSpace : strlen(sNode->tokenStr);
      }
      CMD_MANAGER_LIST_LOOP(cmsh->cnode->cNodeList, sNode, nn)
      {
		if(sNode->flags == 0)
      	{
			cmdPrint(cmsh, "  %s", sNode->tokenStr);
			szIndex = 0;
			CMD_MANAGER_LIST_LOOP(sNode->cHelpTable, szHelpStr, hNN)
			{
			  if(szIndex == 0)
			  {
				cmdPrint(cmsh, "%*c", numMaxSpace - strlen(sNode->tokenStr) + 5, ' ');
			  } else {
				cmdPrint(cmsh, "%*c", numMaxSpace + 7, ' ');
			  }
			  cmdPrint(cmsh, "%s\n", szHelpStr);
			  szIndex++;
			}
       }
	  }
      break;
  }
  return CMD_IPC_OK;
}
