/*
 * include/linux/if_team.h - Network team device driver header
 * Copyright (c) 2011 Jiri Pirko <jpirko@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef _LINUX_IF_TEAM_COMPAT_H_
#define _LINUX_IF_TEAM_COMPAT_H_

#ifdef __KERNEL__

#ifdef	N2OS
#define	PRINTK if (debug) printk
#undef	netdev_emerg
#undef	netdev_alert
#undef	netdev_crit
#undef	netdev_err
#undef	netdev_warn
#undef	netdev_notice
#undef	netdev_info
#undef	netdev_dbg

#define netdev_emerg(dev, format, args...)          \
    printk(KERN_EMERG "%s: " format, dev->name, ##args)
#define netdev_alert(dev, format, args...)          \
    printk(KERN_ALERT "%s: " format, dev->name, ##args)
#define netdev_crit(dev, format, args...)           \
    printk(KERN_CRIT "%s: " format, dev->name, ##args)
#define netdev_err(dev, format, args...)            \
    printk(KERN_ERR "%s: " format, dev->name, ##args)
#define netdev_warn(dev, format, args...)           \
    printk(KERN_WARNING "%s: " format, dev->name, ##args)
#define netdev_notice(dev, format, args...)         \
    printk(KERN_NOTICE "%s: " format, dev->name, ##args)
#define netdev_info(dev, format, args...)           \
    printk(KERN_INFO "%s: " format, dev->name, ##args)
#define netdev_dbg(dev, format, args...)           \
    printk(KERN_DEBUG "%s: " format, dev->name, ##args)

#define	NLA_S32	(NLA_BINARY+3)

#define	__rcu
typedef u64 netdev_features_t;

enum rx_handler_result {
    RX_HANDLER_CONSUMED,
    RX_HANDLER_ANOTHER,
    RX_HANDLER_EXACT,
    RX_HANDLER_PASS,
};
typedef enum rx_handler_result rx_handler_result_t;

#if 0
#define IFF_TEAM_PORT   0x40000
#endif

int __ethtool_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
    ASSERT_RTNL();

    if (!dev->ethtool_ops || !dev->ethtool_ops->get_settings)
        return -EOPNOTSUPP;

    memset(cmd, 0, sizeof(struct ethtool_cmd));
    cmd->cmd = ETHTOOL_GSET;
    return dev->ethtool_ops->get_settings(dev, cmd);
}
#endif

#endif /* __KERNEL__ */

#endif // _LINUX_IF_TEAM_COMPAT_H_
