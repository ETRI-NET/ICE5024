#ifndef __COMPSTATIC_H__
#define __COMPSTATIC_H__
/**
 * @brief Overview : Process function for cmsh module
 * @brief Creator: Thanh Nguyen Ba
 * @file      : compStatic
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
void nosCmCmdInit(Int32T cmshID, Int32T cmID, void *nosCmshProcess, void *nosCompProcess);
void nosCompCmdInit(Int32T compID, Int32T cmID, void *nosCmshProcess, void *nosCompProcess);
void nosMainServiceFree(void);
#endif
