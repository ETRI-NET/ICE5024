/* Copyright (C) 2004 IP Infusion, Inc. All Rights Reserved. */

#ifndef _HAL_H_
#define _HAL_H_

#define HAL_DBUG(level, format, ...) printf("[%s] %s: " format, __FILE__, __FUNCTION__, ##__VA_ARGS__)

/* 
   Name: hal_init

   Description: 
   Initialize the HAL component. 

   Parameters:
   None

   Returns:
   < 0 on error 
   HAL_SUCCESS
*/
int
hal_init ();

/* 
   Name: hal_deinit

   Description:
   Deinitialize the HAL component.

   Parameters:
   None

   Returns:
   < 0 on error
   HAL_SUCCESS
*/
int
hal_deinit ();

#endif /* _HAL_H_ */
