/* Copyright (C) 2004 IP Infusion, Inc. All Rights Reserved. */

#ifndef _HAL_L2_
#define _HAL_L2_

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN	6
#endif

/* Bridge information structure. */
struct hal_bridge_info
{
  unsigned char       up;
  unsigned char       learning_enabled;
  unsigned int        ageing_time;
  unsigned int        dynamic_ageing_period;
  unsigned int        dynamic_ageing_timer_value;
};

/* Bridge port information. */                                                  
struct hal_port_info
{
  unsigned short      port_id;
  unsigned char       state;
};

/* Bridge Port states */
#define HAL_BR_PORT_STATE_DISABLED   0
#define HAL_BR_PORT_STATE_LISTENING  1
#define HAL_BR_PORT_STATE_LEARNING   2
#define HAL_BR_PORT_STATE_FORWARDING 3
#define HAL_BR_PORT_STATE_BLOCKING   4
#define HAL_BR_PORT_STATE_MAX        5

/* Flow control. */
#define HAL_FLOW_CONTROL_OFF                            0
#define HAL_FLOW_CONTROL_SEND                           (1 << 0)
#define HAL_FLOW_CONTROL_RECEIVE                        (1 << 1)

/* FDB entry flags. */
#define HAL_L2_FDB_DYNAMIC                     0
#define HAL_L2_FDB_STATIC                      (1 << 0)

/* Max Number of fdb entries */
#define HAL_MAX_L2_FDB_ENTRIES                 100

/* FDB entry structure. */
struct hal_fdb_entry
{
  unsigned short vid;
  unsigned int ageing_timer_value;
  unsigned char mac_addr[ETHER_ADDR_LEN];
  int num;
  char is_local;
  unsigned char is_static;
  unsigned char is_forward;
  unsigned int port;
};

#if 0
typedef struct fdb_entry
{
  int        ageing_timer_value;   /* Entry timeout value  */
  u_char       mac_addr[6];        /* Ethernet address     */
  short        port_no;            /* Physical port number */   
  short        vid;                /* Vlan id              */
  short        is_static;          /* Static entry         */
  u_char       is_local;           /* Local address        */
  u_char       is_fwd;             /* Drop/Forward flag    */ 
  u_char       snmp_status;
  u_char       _unused[2];
} fdb_entry_t;
#endif 

/* Port Selection Criteria for Link Aggregation */
#define HAL_LACP_PSC_DST_MAC          1
#define HAL_LACP_PSC_SRC_MAC          2
#define HAL_LACP_PSC_SRC_DST_MAC      3
#define HAL_LACP_PSC_SRC_IP           4
#define HAL_LACP_PSC_DST_IP           5
#define HAL_LACP_PSC_SRC_DST_IP       6
#define HAL_LACP_PSC_SRC_PORT         7
#define HAL_LACP_PSC_DST_PORT         8
#define HAL_LACP_PSC_SRC_DST_PORT     9

/* Egress tagged enumeration. */
enum hal_vlan_egress_type
  {
    HAL_VLAN_EGRESS_UNTAGGED = 0,
    HAL_VLAN_EGRESS_TAGGED   = 1
  };


/* Acceptable Frame Types enumeration. */
enum hal_vlan_acceptable_frame_type
  {
    HAL_VLAN_ACCEPTABLE_FRAME_TYPE_ALL    = 0,
    HAL_VLAN_ACCEPTABLE_FRAME_TYPE_TAGGED = 1
  };

/* Port type. */
enum hal_vlan_port_type
  {
    HAL_VLAN_ACCESS_PORT = 0,
    HAL_VLAN_HYBRID_PORT = 1,
    HAL_VLAN_TRUNK_PORT
  };

/* Vlan stack mode */
#define HAL_VLAN_STACK_MODE_NONE      0 /* disable vlan stacking */
#define HAL_VLAN_STACK_MODE_INTERNAL  1 /* Use internal/service provider tag */
#define HAL_VLAN_STACK_MODE_EXTERNAL  2 /* Use external/customer tag */

/* Filter type */
#define HAL_VLAN_CLASSIFIER_MAC       1  /* filter on source MAC */
#define HAL_VLAN_CLASSIFIER_PROTOCOL  2  /* filter on protocol */
#define HAL_VLAN_CLASSIFIER_IPV4      4  /* filter on src IPv4 subnet */
                                                                                
/* Encapsulation */
#define HAL_VLAN_CLASSIFIER_ETH        0x00020000  /* Ethernet v2 */
#define HAL_VLAN_CLASSIFIER_NOSNAP_LLC 0x00020001  /* No snap LLC */
#define HAL_VLAN_CLASSIFIER_SNAP_LLC   0x00020002  /* Snap LLC*/


struct hal_vlan_classifier_rule
{
  int type;                       /* Type of classifier: Protocol/Mac/Subnet */
  unsigned short vlan_id;         /* Destination vlan_id                     */
  u_int32_t rule_id;              /* Rule identification number.             */

  union                           /* Rule criteria.                          */
  {
    unsigned char mac[ETHER_ADDR_LEN];  /* Mac address.                      */

    struct 
    {
      unsigned int addr;
      unsigned char masklen;
    }ipv4;
 

    struct
    {
      unsigned short ether_type;  /* Protocol value                          */
      unsigned int   encaps;      /* Packet L2 encapsulation.                */
    } protocol;

  } u;

  struct avl_tree *group_tree;     /* Groups rule attached to.  */
};


#endif /* _HAL_L2_ */
