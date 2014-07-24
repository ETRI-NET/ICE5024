/*
 * ospfRibmgr.h
 *
 *  Created on: 2014. 6. 10.
 *      Author: root
 */

#ifndef OSPFRIBMGR_H_
#define OSPFRIBMGR_H_

extern void ospfRibmgrInit ();
extern void ospfRibmgrClose ();
extern Int32T ospfRibmgrReadIpv4 (Int32T ,  nnBufferT *, Uint16T );

//extern in_addr_t ospfGetRouterId(void);

extern int ospfRedistributeSet (struct cmsh *, struct ospf *, int , int , int );
extern int ospf_redistribute_unset (struct cmsh *cmsh, struct ospf *ospf, int type);
extern int protoRedistNum(int afi, const char *s);
extern int ospf_redistribute_default_set (struct cmsh *, struct ospf *, int , int , int );
extern int ospf_redistribute_default_unset (struct cmsh *, struct ospf *ospf);
#endif /* OSPFRIBMGR_H_ */
