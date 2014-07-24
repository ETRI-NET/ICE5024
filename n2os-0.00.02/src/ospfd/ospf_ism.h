/*
 * OSPF version 2  Interface State Machine.
 *   From RFC2328 [OSPF Version 2]
 * Copyright (C) 1999 Toshiaki Takada
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _ZEBRA_OSPF_ISM_H
#define _ZEBRA_OSPF_ISM_H


/* OSPF Interface State Machine Status. */
#define ISM_DependUpon                    0
#define ISM_Down                          1
#define ISM_Loopback                      2
#define ISM_Waiting                       3
#define ISM_PointToPoint                  4
#define ISM_DROther                       5
#define ISM_Backup                        6
#define ISM_DR                            7
#define OSPF_ISM_STATE_MAX   	          8

/* Because DR/DROther values are exhanged wrt RFC */
#define ISM_SNMP(x) (((x) == ISM_DROther) ? ISM_DR : \
                     ((x) == ISM_DR) ? ISM_DROther : (x))

/* OSPF Interface State Machine Event. */
#define ISM_NoEvent                       0
#define ISM_InterfaceUp                   1
#define ISM_WaitTimer                     2
#define ISM_BackupSeen                    3
#define ISM_NeighborChange                4
#define ISM_LoopInd                       5
#define ISM_UnloopInd                     6
#define ISM_InterfaceDown                 7
#define OSPF_ISM_EVENT_MAX                8

#define OSPF_ISM_WRITE_ON(O)                                                  \
      do                                                                      \
        {                                                                     \
          if (oi->on_write_q == 0)                                            \
	    {                                                                 \
              listnode_add ((O)->oi_write_q, oi);                             \
	      oi->on_write_q = 1;                                             \
	    }                                                                 \
	  if ((O)->pSockFdWriteEvent == NULL)                                           \
	  	  (O)->pSockFdWriteEvent = taskFdSet(ospfWrite, (O)->fd,											\
    						TASK_WRITE, TASK_PRI_MIDDLE, (O)); \
        } while (0)


/* Macro for OSPF ISM timer turn on. */
#define OSPF_ISM_TIMER_ON(T,F,V) \
  do { \
	  struct timeval tv; \
	  tv.tv_sec = (V);	\
	  tv.tv_usec = 0;	\
    if (!(T)) \
      (T) = taskTimerSet((F), tv, 0, oi); \
  } while (0)
#define OSPF_ISM_TIMER_MSEC_ON(T,F,V) \
  do { \
	  struct timeval tv; \
	  tv.tv_sec = (V) / 1000;	\
	  tv.tv_usec = 1000*((V) % 1000);	\
    if (!(T)) \
      (T) = taskTimerSet((F), tv, 0, oi); \
  } while (0)


/* convenience macro to set hello timer correctly, according to
 * whether fast-hello is set or not
 */
#define OSPF_HELLO_TIMER_ON(O) \
  do { \
    if (OSPF_IF_PARAM ((O), fast_hello)){ \
        OSPF_ISM_TIMER_MSEC_ON ((O)->pTimerHello, ospfHelloTimer, \
                                1000 / OSPF_IF_PARAM ((O), fast_hello)); \
         (O)->tvUpdateHello = ospfGetUpdateTime(0, 1000 / OSPF_IF_PARAM ((O), fast_hello));}\
    else{ \
        OSPF_ISM_TIMER_ON ((O)->pTimerHello, ospfHelloTimer, \
                                OSPF_IF_PARAM ((O), v_hello)); \
        (O)->tvUpdateHello = ospfGetUpdateTime(OSPF_IF_PARAM ((O), v_hello), 0);} \
  } while (0)

/* Macro for OSPF ISM timer turn off. */
#define OSPF_ISM_TIMER_OFF(X) \
  do { \
    if (X) \
      { \
	thread_cancel (X); \
	(X) = NULL; \
      } \
  } while (0)

/* Macro for OSPF schedule event. */
#define OSPF_ISM_EVENT_SCHEDULE(I,E) \
	do { \
		struct timeval tv;	\
		struct ospfEventArg *ev;													\
		ev = (struct ospfEventArg *)malloc(sizeof(struct ospfEventArg));\
		ev->arg1 = (I); ev->arg2 = (int)(E);									\
		tv.tv_sec = 0; tv.tv_usec = 0; \
		taskTimerSet(ospfIsmEvent, tv, 0, ev); \
	} while(0)

/* Macro for OSPF execute event. */
#define OSPF_ISM_EVENT_EXECUTE(I,E) \
	do { \
		struct ospfEventArg *ev;													\
		ev = (struct ospfEventArg *)malloc(sizeof(struct ospfEventArg));\
		ev->arg1 = (I); ev->arg2 = (int)(E);									\
		ospfIsmEvent(0, 0, ev);	\
  } while(0)

/* Prototypes. */
extern void ospfIsmEvent (Int32T fd, Int16T event, void * arg);
extern void ism_change_status (struct ospf_interface *, int);
extern void ospfHelloTimer (Int32T fd, Int16T event, void * arg);

extern void ospfWaitTimer (Int32T fd, Int16T event, void * arg);

#endif /* _ZEBRA_OSPF_ISM_H */
