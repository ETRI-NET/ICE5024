
#ifdef	N2OS_HWAPI
extern void team_aggregation_link_up_by_dev (struct net_device *dev);
extern void team_aggregation_link_down_by_dev (struct net_device *dev);
extern void team_aggregation_link_up_by_index (int index);
extern void team_aggregation_link_down_by_index (int index);
extern void team_aggregation_link_up_by_name (char *ifname);
extern void team_aggregation_link_down_by_name (char *ifname);

extern void set_team_aggregation_link_enable (int (* real_link_enable_func) (struct net_device *dev, bool up));
extern void reset_team_aggregation_link_enable (void);
#endif

