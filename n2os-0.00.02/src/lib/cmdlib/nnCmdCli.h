#ifndef __NN_CMD_MGR_CLI_H__
#define __NN_CMD_MGR_CLI_H__

/**
 * @brief Overview : Client for CMSHs and COMPs
 * @brief Creator: Thanh Nguyen Ba
 * @file      : nnCmdCli.h
 *
 * $Id: nnCmdMd5.h 725 2014-01-17 09:11:34Z hyryu $
 * $Author: hyryu $
 * $Date: 2014-01-17 04:11:34 -0500 (Fri, 17 Jan 2014) $
 * $Log$
 * $Revision: 725 $
 * $LastChangedBy: thanh $
 * $LastChanged$
 *
 *                      Electronics and Telecommunications Research Institute
 * Copyright: 2013 Electronics and Telecommunications Research Institute. All rights reserved.
 *           No part of this software shall be reproduced, stored in a retrieval system, or
 *           transmitted by any means, electronic, mechanical, photocopying, recording,
 *           or otherwise, without written permission from ETRI.
 **/
/**
 * Description: 
 *
 * @param [in]  
 * @param [out] 
 *
 * @retval : code
 */
Int32T cmdCmshRegister(struct cmsh *cmsh);
Int32T cmdCmshDeRegister(struct cmsh *cmsh);
#endif
