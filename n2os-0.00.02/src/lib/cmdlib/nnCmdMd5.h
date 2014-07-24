#ifndef __NOS_MD5_H__
#define __NOS_MD5_H__
/**
 * @brief Overview : 
 * @brief Creator: Thanh Nguyen Ba
 * @file      : nnCmdMd5.h
 *
 * $Id: nnCmdMd5.h 725 2014-01-17 09:11:34Z hyryu $
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

 /** Data structure for MD5 (Message Digest) computation */
typedef struct 
{
  uint32_t i[2];                   /** number of _bits_ handled mod 2^64 */
  uint32_t buf[4];                                    /** scratch buffer */
  uint8_t  in[64];                              /** input buffer */
  uint8_t  digest[16];     /** actual digest after MD5Final call */
} MD5_CTX;

/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void MD5Init(MD5_CTX *mdContext);
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void MD5Update(MD5_CTX *mdContext, uint8_t *inBuf, uint32_t inLen);
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
void MD5Final(MD5_CTX *mdContext);
#endif
