/* Copyright (C) 2003-2011 IP Infusion, Inc. All Rights Reserved.

    Interface to VLAN Aware forwarder
  
    Authors: McLendon

 */

#ifndef _HAL_IPIFWD_H
#define _HAL_IPIFWD_H

#include "hal_types.h"

#define ZEBOS_LAYER2_VERSION  101

/* Bridge control message identifiers */
#define IPIBR_GET_VERSION                           1
#define IPIBR_GET_BRIDGES                           2
#define IPIBR_ADD_BRIDGE                            3
#define IPIBR_DEL_BRIDGE                            4
#define IPIBR_ADD_IF                                5
#define IPIBR_DEL_IF                                6
#define IPIBR_GET_BRIDGE_INFO                       7
#define IPIBR_GET_PORT_LIST                         8
#define IPIBR_SET_AGEING_TIME                       9
#define IPIBR_SET_DYNAMIC_AGEING_INTERVAL           10
#define IPIBR_GET_PORT_INFO                         11
#define IPIBR_SET_BRIDGE_LEARNING                   12
#define IPIBR_GET_DYNFDB_ENTRIES                    13
#define IPIBR_GET_STATFDB_ENTRIES                   14
#define IPIBR_ADD_STATFDB_ENTRY                     15
#define IPIBR_DEL_STATFDB_ENTRY                     16
#define IPIBR_GET_DEVADDR                           17
#define IPIBR_GET_PORT_STATE                        18
#define IPIBR_SET_PORT_STATE                        19
#define IPIBR_SET_PORT_FWDER_FLAGS                  20
#define IPI_VLAN_ADD                                21
#define IPI_VLAN_DEL                                22
#define IPI_VLAN_SET_PORT_TYPE                      23
#define IPI_VLAN_SET_DEFAULT_PVID                   24
#define IPI_VLAN_ADD_VID_TO_PORT                    25
#define IPI_VLAN_DEL_VID_FROM_PORT                  26
#define IPIBR_FLUSH_FDB_BY_PORT                     27
#define IPIBR_ADD_DYNAMIC_FDB_ENTRY                 28
#define IPIBR_DEL_DYNAMIC_FDB_ENTRY                 29
#define IPIBR_ADD_VLAN_TO_INST                      30
#define IPIBR_ENABLE_IGMP_SNOOPING                  31
#define IPIBR_DISABLE_IGMP_SNOOPING                 32
#define IPI_VLAN_SET_NATIVE_VID                     33
#define IPI_VLAN_SET_MTU                            34
#define IPIBR_GET_UNICAST_ENTRIES                   35
#define IPIBR_GET_MULTICAST_ENTRIES                 36
#define IPIBR_CLEAR_FDB_BY_MAC                      37
#define IPIBR_GARP_SET_BRIDGE_TYPE                  38
#define IPIBR_ADD_GMRP_SERVICE_REQ                  39
#define IPIBR_SET_EXT_FILTER                        40
#define IPIBR_SET_PVLAN_TYPE                        41
#define IPIBR_SET_PVLAN_ASSOCIATE                   42
#define IPIBR_SET_PVLAN_PORT_MODE                   43
#define IPIBR_SET_PVLAN_HOST_ASSOCIATION            44
#define IPIBR_ADD_CVLAN_REG_ENTRY                   45
#define IPIBR_DEL_CVLAN_REG_ENTRY                   46
#define IPIBR_ADD_VLAN_TRANS_ENTRY                  47
#define IPIBR_DEL_VLAN_TRANS_ENTRY                  48
#define IPIBR_SET_PROTO_PROCESS                     49
#define IPI_VLAN_ADD_PRO_EDGE_PORT                  50
#define IPI_VLAN_DEL_PRO_EDGE_PORT                  51
#define IPI_VLAN_SET_PRO_EDGE_DEFAULT_PVID          52
#define IPI_VLAN_SET_PRO_EDGE_UNTAGGED_VID          53
#define IPIBR_CHANGE_VLAN_TYPE                      54
#define IPIBR_GET_IFINDEX_BY_MAC_VID                55
#define IPI_VLAN_DISABLE                            56
#define IPI_VLAN_ENABLE                             57
#define IPIBR_DISABLE_AGEING                        58
#define IPIBR_MAX_CMD                               59
#define IPI_UNI_ADD                                 60

/* Maximum Values for bridge data */
#define HAL_BRIDGE_MAX_DYN_FDB_ENTRIES              256
#define HAL_BRIDGE_MAX_STATIC_FDB_ENTRIES           256
#define HAL_MAX_BRIDGES                             32
#define HAL_BRIDGE_MAX_PORTS                        32

#define HAL_VLAN_DEFAULT_VID              1
#define HAL_VLAN_NAME_SIZE                32
#define HAL_VLAN_SUSPEND_STATE            0
#define HAL_VLAN_ACTIVE_STATE             1
#define HAL_VLAN_ACCESS_PORT              0
#define HAL_VLAN_HYBRID_PORT              1
#define HAL_VLAN_TRUNK_PORT               2
#define HAL_VLAN_STACKED_TRUNK_PORT       3
#define HAL_VLAN_EGRESS_UNTAGGED          0
#define HAL_VLAN_EGRESS_TAGGED            1
#define HAL_VLAN_ACCEPT_ALL_FRAMES_TYPES  0
#define HAL_VLAN_TAGGED_FRAMES_ONLY       1
#define HAL_VLAN_DISABLE_INGRESS_FILTER   0
#define HAL_VLAN_ENABLE_INGRESS_FILTER    1

/* Bridge VLAN types */
#define HAL_NO_VLAN_BRIDGE                0
#define HAL_VLAN_BRIDGE                   1
#define HAL_SHARED_VLAN_BRIDGE            2

#define HAL_BITS_PER_VLANMAP_ENTRY        8

#define HAL_VLANMAP_SIZE\
        ((HAL_MAX_VLAN_ID + 2)/HAL_BITS_PER_VLANMAP_ENTRY)

#define HAL_VLAN_GET_VLAN_BITMAP(vid, vlan_bitmap) \
        (((vlan_bitmap)[(vid)/(HAL_BITS_PER_VLANMAP_ENTRY)]) \
         & (1 << ((vid) % (HAL_BITS_PER_VLANMAP_ENTRY))))

#define HAL_VLAN_SET_VLAN_BITMAP(vid, vlan_bitmap) \
        (((vlan_bitmap)[(vid)/(HAL_BITS_PER_VLANMAP_ENTRY)]) \
         | (1 << ((vid) % (HAL_BITS_PER_VLANMAP_ENTRY))))

#define HAL_PORT_MIRROR_DIRECTION_BOTH          0x11
#define HAL_PORT_MIRROR_DIRECTION_TRANSMIT      0x10
#define HAL_PORT_MIRROR_DIRECTION_RECEIVE       0x01

/* Flag values for the bridge */

#define IPIBR_UP                  0x01
#define IPIBR_LEARNING_ENABLED    0x02

/* Bridge default values */

#define BRIDGE_TIMER_DEFAULT_FWD_DELAY                    15
#define BRIDGE_TIMER_DEFAULT_HELLO_TIME                   2
#define BRIDGE_TIMER_DEFAULT_MAX_AGE                      20
#define BRIDGE_TIMER_DEFAULT_AGEING_TIME                  300
#define BRIDGE_TIMER_DEFAULT_AGEING_INTERVAL              4
#define BRIDGE_TIMER_DISABLING_AGEING_TIME		  0

#define BRIDGE_DEFAULT_PRIORITY           32768
#define CE_BRIDGE_DEFAULT_PRIORITY        61440
#define BRIDGE_DEFAULT_PORT_PRIORITY      128

struct vlan_summary
{
  unsigned short vid;
  unsigned short fid;
  unsigned char  vlan_name[HAL_VLAN_NAME_LEN];
  unsigned short vlan_state;
  unsigned char  config_ports[HAL_BRIDGE_MAX_PORTS];
};

struct vlan_port_summary
{
  int port_no;
  unsigned char port_type;
  unsigned char ingress_filter;
  unsigned char acceptable_frame_types;
  unsigned char enable_vid_swap;
  unsigned short stacked_vid;
  unsigned short default_pvid;
  unsigned char config_vids[HAL_VLANMAP_SIZE];
  unsigned char egress_tagged[HAL_VLANMAP_SIZE];
};

struct port_stats
{
  unsigned int inframes;
  unsigned int outframes;
  unsigned int discards;
  unsigned int in_overflow_frames;
  unsigned int out_overflow_frames;
  unsigned int overflow_discards;
};

struct port_HC_stats
{
  unsigned int inframes;
  unsigned int outframes;
  unsigned int discards;
};


struct bridge_info
{
  unsigned char       up;
  unsigned char       learning_enabled;
  unsigned int        ageing_time;
  unsigned int        dynamic_ageing_period;
  unsigned int        dynamic_ageing_timer_value;
};

struct port_info
{
  unsigned short      port_id;
  unsigned char       state;
};


#endif
