/* Copyright (C) 2005 IP Infusion, Inc. All Rights Reserved. */
                                                                                
#ifndef _HAL_GARP_H_
#define _HAL_GARP_H_

#define HAL_GMRP_CONFIGURED  0x01
#define HAL_GVRP_CONFIGURED  0x02

/*
   Name: hal_garp_set_bridge_type
                                                                                
   Description:
   This API sets the bridge as gmrp/gvrp enable or disable.
                                                                                
   Parameters:
   bridge name
   type  - gvrp -02
         - gmrp -01
   enable - True
          - False
   Returns:
   Void
*/
void
hal_garp_set_bridge_type (char * bridge_name, unsigned long garp_type, int enable);
                                                                                
#endif /* _HAL_GARP_H_ */
