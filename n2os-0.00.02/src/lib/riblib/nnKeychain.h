/*******************************************************************************
* Copyright: 2013 Electronics and Telecommunications Research Institute. 
*           All rights reserved. No part of this software shall be reproduced, 
*           stored in a retrieval system, or transmitted by any means, 
*           electronic, mechanical, photocopying, recording, or otherwise, 
*           without written permission from ETRI.
*******************************************************************************/

/**   
 * @brief : This file defines the md5 keychain related definitions.
 *  - Block Name : riblib
 *  - Process Name : rib library *  - Creator : Suncheul Kim
 *  - Initial Date : 2013/10/10
 */

/**
 * @file : nnKeychain.h
 *
 * $Author$
 * $Date$
 * $Revision$
 * $LastChangedBy$
 */

#if !defined(_nnKeychain_h)
#define _nnKeychain_h
#include <time.h>
#include "nnTypes.h"

#include "nnCmdCmsh.h"

struct Keychain
{
  char * name;
  ListT * pKeyList;
};
typedef struct Keychain KeychainT;

struct Keyrange
{
  time_t start;
  time_t end;

  Uint8T duration;
};
typedef struct Keyrange KeyrangeT;

struct Key
{
  Uint32T index;

  StringT string;

  KeyrangeT send;
  KeyrangeT accept;
};
typedef struct Key KeyT;

extern KeychainT * keychainGet (const StringT);
extern KeyT * keyGet (const KeychainT *, Uint32T);
extern void keychainDelete (KeychainT *);
extern KeyT * keyLookup (const KeychainT *, Uint32T);
extern void keyDelete (KeychainT *, KeyT *);

extern Int32T keyLifetimeSet (struct cmsh *, KeyrangeT *, const char *, 
              const char *, const char *, const char *, const char *, 
              const char *, const char *, const char *);
extern Int32T keyLifetimeDurationSet (struct cmsh *, KeyrangeT *,
              const char *, const char *, const char *, const char *,
              const char *);
extern Int32T keyLifetimeInfiniteSet (struct cmsh *, KeyrangeT *, 
              const char *, const char *, const char *, const char *);

extern void keychainInit (void);
extern void keychainReset (void);
extern ListT * keychainGetPtr ();
extern void keychainVersionUpdate (ListT *);
extern KeychainT *keychainLookup (const StringT);
extern KeyT *keyLookupForAccept (const KeychainT *, Uint32T);
extern KeyT *keyMatchForAccept (const KeychainT *, const StringT);
extern KeyT *keyLookupForSend (const KeychainT *);

extern Int32T configWriteKeychain (struct cmsh * cmsh);

#endif /* _nnKeychain_h */
