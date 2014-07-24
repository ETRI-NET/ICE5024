#ifndef __NOS_SHA256_H__
#define __NOS_SHA256_H__
/**
 * @brief Overview : 
 * @brief Creator: Thanh Nguyen Ba
 * @file      : nnCmdSha2.h
 *
 * $Id: nnCmdSha2.h 725 2014-01-17 09:11:34Z hyryu $
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

typedef struct SHA256_Context_s
{
  uint32_t  total[2];
  uint32_t  state[8];
  uint8_t   buffer[64];
  uint8_t   ipad[64];
  uint8_t   opad[64];
} SHA256_Context_T;

#define SHA256_DIGEST_LENGTH    32
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void SHA256_Init(SHA256_Context_T *ctx);
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void SHA256_Update(SHA256_Context_T *ctx, uint8_t *input, size_t ilen);
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void SHA256_Finish(SHA256_Context_T *ctx, uint8_t output[32]);
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void SHA256_Process(SHA256_Context_T *ctx, const uint8_t data[64]);
#endif
