#ifndef __NOS_SHA512_H__
#define __NOS_SHA512_H__
/**
 * @brief Overview : 
 * @brief Creator: Thanh Nguyen Ba
 * @file      : nnCmdSha4.h
 *
 * $Id: nnCmdSha4.h 725 2014-01-17 09:11:34Z hyryu $
 * $Author: hyryu $
 * $Date: 2014-01-17 04:11:34 -0500 (Fri, 17 Jan 2014) $
 * $Log$
 * $Revision: 725 $
 * $LastChangedBy: hyryu $
 * $LastChanged$
 *
 *                      Electronics and Telecommunications Research Institute
 * Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.
 *           No part of this software shall be reproduced, stored in a retrieval system, or
 *           transmitted by any means, electronic, mechanical, photocopying, recording,
 *           or otherwise, without written permission from ETRI.
 **/

#if defined(_MSC_VER) || defined(__WATCOMC__)
  #define UL64(x) x##ui64
  typedef unsigned __int64 uint64_t;
#else
  #include <inttypes.h>
  #define UL64(x) x##ULL
#endif
typedef struct SHA512_Context_s
{
  uint64_t  total[2];
  uint64_t  state[8];
  uint8_t   buffer[128];
  uint8_t   ipad[128];
  uint8_t   opad[128];
}SHA512_Context_T;

#define SHA512_DIGEST_LENGTH 64 
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void SHA512_Init(SHA512_Context_T *ctx);
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void SHA512_Update(SHA512_Context_T *ctx, uint8_t *input, size_t ilen);
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void SHA512_Finish(SHA512_Context_T *ctx, uint8_t output[64]);
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void SHA512_Process(SHA512_Context_T *ctx, const uint8_t data[128]);
#endif
