/*
 * ospf_routermap.h
 *
 *  Created on: 2014. 6. 19.
 *      Author: root
 */

#ifndef OSPF_ROUTERMAP_H_
#define OSPF_ROUTERMAP_H_

extern struct route_map_index *OspfGetRoutemap (struct cmsh *,
									const StringT , const StringT , const StringT );

extern int ospf_route_match_add (struct cmsh *, struct route_map_index *,
										const char *, const StringT );
extern int ospf_route_match_delete (struct cmsh *, struct route_map_index *,
			 	 	 	 	 	 	 	 	 const char *, const StringT );
extern int ospf_route_set_add (struct cmsh *, struct route_map_index *,
		    							const char *, const StringT );
extern int ospf_route_set_delete (struct cmsh *, struct route_map_index *,
		       	   	   	   	   	   const char *, const StringT );



#endif /* OSPF_ROUTERMAP_H_ */
