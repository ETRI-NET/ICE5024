/*
 *
 * net/drivers/team/team.c - Network team device driver
 * Copyright (c) 2011 Jiri Pirko <jpirko@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/rcupdate.h>
#include <linux/errno.h>
#include <linux/ctype.h>
#include <linux/notifier.h>
#include <linux/netdevice.h>
#include <linux/if_vlan.h>
#include <linux/if_arp.h>
#include <linux/socket.h>
#include <linux/etherdevice.h>
#include <linux/rtnetlink.h>
#include <net/rtnetlink.h>
#include <net/genetlink.h>
#include <net/netlink.h>
#ifdef	N2OS
#include "include/linux/compat.h"
#include "include/linux/u64_stats_sync.h"
#include "include/linux/if_team.h"
#include "include/linux/team_hwapi.h"
#else
#include <linux/if_team.h>
#endif

#define DRV_NAME "team"

int	debug = 0;
module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "Set debug options(default:0)");

/**********
 * Helpers
 **********/

#ifdef	N2OS	//refer. bonding
#define team_port_exists(dev) (dev->priv_flags & IFF_SLAVE)
#else
#define team_port_exists(dev) (dev->priv_flags & IFF_TEAM_PORT)
#endif

#ifdef	N2OS_HWAPI
/*****************
 * Hardware abstraction: called by Hardware driver
 *****************/
static int (* team_aggregation_link_enable) (struct net_device *dev, bool up) = NULL;

void set_team_aggregation_link_enable (int (* real_link_enable_func) (struct net_device *dev, bool up))
{
	team_aggregation_link_enable = real_link_enable_func;
}
EXPORT_SYMBOL(set_team_aggregation_link_enable);

void reset_team_aggregation_link_enable (void)
{
	team_aggregation_link_enable = NULL;
}
EXPORT_SYMBOL(reset_team_aggregation_link_enable);

void team_aggregation_link_up_by_dev (struct net_device *dev)
{
	if (team_port_exists (dev))
		netif_carrier_on(dev);
}
EXPORT_SYMBOL(team_aggregation_link_up_by_dev);

void team_aggregation_link_down_by_dev (struct net_device *dev)
{
	if (team_port_exists (dev))
		netif_carrier_off(dev);
}
EXPORT_SYMBOL(team_aggregation_link_down_by_dev);

void team_aggregation_link_up_by_index (int index)
{
	struct net_device *dev = dev_get_by_index (&init_net, index);
	if (dev == NULL)
		return;
	if (team_port_exists (dev))
		netif_carrier_on(dev);
	dev_put (dev);
}
EXPORT_SYMBOL(team_aggregation_link_up_by_index);

void team_aggregation_link_down_by_index (int index)
{
	struct net_device *dev = dev_get_by_index (&init_net, index);
	if (dev == NULL)
		return;
	if (team_port_exists (dev))
		netif_carrier_off(dev);
	dev_put (dev);
}
EXPORT_SYMBOL(team_aggregation_link_down_by_index);

void team_aggregation_link_up_by_name (char *ifname)
{
	struct net_device *dev = dev_get_by_name (&init_net, ifname);
	if (dev == NULL)
		return;
	if (team_port_exists (dev))
		netif_carrier_on(dev);
	dev_put (dev);
}
EXPORT_SYMBOL(team_aggregation_link_up_by_name);

void team_aggregation_link_down_by_name (char *ifname)
{
	struct net_device *dev = dev_get_by_name (&init_net, ifname);
	if (dev == NULL)
		return;
	if (team_port_exists (dev))
		netif_carrier_off(dev);
	dev_put (dev);
}
EXPORT_SYMBOL(team_aggregation_link_down_by_name);
#endif

#ifdef	N2OS	//refer. bonding
void vlan_vids_add_by_dev(struct team *team, struct net_device *port_dev);
void vlan_vids_del_by_dev(struct team *team, struct net_device *port_dev);
static LIST_HEAD(team_port_local_list);
static DEFINE_SPINLOCK(team_port_local_lock);

struct team_port_local {
	struct list_head list;
	struct team_port *port;
};

static int team_port_local_register(struct team_port *port)
{
	struct team_port_local *p;

	PRINTK ("%s: %s dev=0x%p\n", __FUNCTION__, port->dev->name, port->dev);
	p = kmalloc (sizeof(struct team_port_local), GFP_ATOMIC);
	if (p == NULL)
		return -1;

	p->port = port;
	spin_lock_bh (&team_port_local_lock);
	list_add_tail (&p->list, &team_port_local_list);
	spin_unlock_bh (&team_port_local_lock);
	PRINTK ("%s: %s end port=0x%p\n", __FUNCTION__, port->dev->name, port);
	return 0;
}

static int team_port_local_unregister(struct team_port *port)
{
	int err = -1;
	struct team_port_local *p;
	struct team_port_local *tmp;

	PRINTK ("%s: %s dev=0x%p\n", __FUNCTION__, port->dev->name, port->dev);
	spin_lock_bh (&team_port_local_lock);
	list_for_each_entry_safe(p, tmp, &team_port_local_list, list)
	{
		if (p->port == port) {
			list_del (&p->list);
			PRINTK ("%s: %s port=0x%p\n", __FUNCTION__, p->port->dev->name, p->port);
			kfree (p);
			err = 0;
			break;
		}
	}
	spin_unlock_bh (&team_port_local_lock);
	PRINTK ("%s: \n", __FUNCTION__);
	return err;
}

static struct team_port *team_port_local_get(struct net_device *dev)
{
	struct team_port *port = NULL;
	struct team_port_local *p;
	struct team_port_local *tmp;

	PRINTK ("%s: %s dev=0x%p\n", __FUNCTION__, dev->name, dev);
	spin_lock_bh (&team_port_local_lock);
	list_for_each_entry_safe(p, tmp, &team_port_local_list, list)
	{
		PRINTK ("%s: %s local dev=0x%p\n", __FUNCTION__, p->port->dev->name, p->port->dev);
		if (dev == p->port->dev) {
			PRINTK ("%s: %s found\n", __FUNCTION__, dev->name);
			port = team_port_exists(dev) ? p->port : NULL;
			break;
		}
	}
	spin_unlock_bh (&team_port_local_lock);
	PRINTK ("%s: %s end 0x%p\n", __FUNCTION__, dev->name, port);
	return port;
}
#else
static struct team_port *team_port_get_rcu(const struct net_device *dev)
{
	struct team_port *port = rcu_dereference(dev->rx_handler_data);

	return team_port_exists(dev) ? port : NULL;
}

static struct team_port *team_port_get_rtnl(const struct net_device *dev)
{
	struct team_port *port = rtnl_dereference(dev->rx_handler_data);

	return team_port_exists(dev) ? port : NULL;
}
#endif

/*
 * Since the ability to change mac address for open port device is tested in
 * team_port_add, this function can be called without control of return value
 */
static int __set_port_mac(struct net_device *port_dev,
			  const unsigned char *dev_addr)
{
	struct sockaddr addr;

	memcpy(addr.sa_data, dev_addr, ETH_ALEN);
	addr.sa_family = ARPHRD_ETHER;
	return dev_set_mac_address(port_dev, &addr);
}

int team_port_set_orig_mac(struct team_port *port)
{
	PRINTK ("%s: %s\n", __FUNCTION__, port->dev->name);
	return __set_port_mac(port->dev, port->orig.dev_addr);
}

int team_port_set_team_mac(struct team_port *port)
{
	PRINTK ("%s: %s\n", __FUNCTION__, port->dev->name);
	return __set_port_mac(port->dev, port->team->dev->dev_addr);
}
EXPORT_SYMBOL(team_port_set_team_mac);


/*******************
 * Options handling
 *******************/

struct team_option *__team_find_option(struct team *team, const char *opt_name)
{
	struct team_option *option;

	list_for_each_entry(option, &team->option_list, list) {
		if (strcmp(option->name, opt_name) == 0)
			return option;
	}
	return NULL;
}

int __team_options_register(struct team *team,
			    const struct team_option *option,
			    size_t option_count)
{
	int i;
	struct team_option **dst_opts;
	int err;

	dst_opts = kzalloc(sizeof(struct team_option *) * option_count,
			   GFP_KERNEL);
	if (!dst_opts)
		return -ENOMEM;
	for (i = 0; i < option_count; i++, option++) {
		if (__team_find_option(team, option->name)) {
			err = -EEXIST;
			goto rollback;
		}
		dst_opts[i] = kmemdup(option, sizeof(*option), GFP_KERNEL);
		if (!dst_opts[i]) {
			err = -ENOMEM;
			goto rollback;
		}
		PRINTK ("%s: %s\n", __FUNCTION__, option->name);
	}

	for (i = 0; i < option_count; i++) {
		dst_opts[i]->changed = true;
		dst_opts[i]->removed = false;
		list_add_tail(&dst_opts[i]->list, &team->option_list);
	}

	kfree(dst_opts);
	return 0;

rollback:
	for (i = 0; i < option_count; i++)
		kfree(dst_opts[i]);

	kfree(dst_opts);
	return err;
}

static void __team_options_mark_removed(struct team *team,
					const struct team_option *option,
					size_t option_count)
{
	int i;

	for (i = 0; i < option_count; i++, option++) {
		struct team_option *del_opt;

		del_opt = __team_find_option(team, option->name);
		if (del_opt) {
			del_opt->changed = true;
			del_opt->removed = true;
		}
	}
}

static void __team_options_unregister(struct team *team,
				      const struct team_option *option,
				      size_t option_count)
{
	int i;

	for (i = 0; i < option_count; i++, option++) {
		struct team_option *del_opt;

		del_opt = __team_find_option(team, option->name);
		if (del_opt) {
			list_del(&del_opt->list);
			kfree(del_opt);
		}
	}
}

static void __team_options_change_check(struct team *team);

int team_options_register(struct team *team,
			  const struct team_option *option,
			  size_t option_count)
{
	int err;

	PRINTK ("%s: %s\n", __FUNCTION__, team->dev->name);
	err = __team_options_register(team, option, option_count);
	if (err)
		return err;
	__team_options_change_check(team);
	PRINTK ("%s: %s end\n", __FUNCTION__, team->dev->name);
	return 0;
}
EXPORT_SYMBOL(team_options_register);

void team_options_unregister(struct team *team,
			     const struct team_option *option,
			     size_t option_count)
{
	PRINTK ("%s: %s\n", __FUNCTION__, team->dev->name);
	__team_options_mark_removed(team, option, option_count);
	__team_options_change_check(team);
	__team_options_unregister(team, option, option_count);
}
EXPORT_SYMBOL(team_options_unregister);

static int team_option_get(struct team *team, struct team_option *option,
			   void *arg)
{
	PRINTK ("%s: %s name=%s\n", __FUNCTION__, team->dev->name, option->name);
	return option->getter(team, arg);
}

static int team_option_set(struct team *team, struct team_option *option,
			   void *arg)
{
	int err;

	PRINTK ("%s: %s name=%s\n", __FUNCTION__, team->dev->name, option->name);
	err = option->setter(team, arg);
	if (err)
		return err;

	option->changed = true;
	__team_options_change_check(team);
	return err;
}

/****************
 * Mode handling
 ****************/

static LIST_HEAD(mode_list);
static DEFINE_SPINLOCK(mode_list_lock);

static struct team_mode *__find_mode(const char *kind)
{
	struct team_mode *mode;

	list_for_each_entry(mode, &mode_list, list) {
		if (strcmp(mode->kind, kind) == 0)
			return mode;
	}
	return NULL;
}

static bool is_good_mode_name(const char *name)
{
	while (*name != '\0') {
		if (!isalpha(*name) && !isdigit(*name) && *name != '_')
			return false;
		name++;
	}
	return true;
}

int team_mode_register(struct team_mode *mode)
{
	int err = 0;

	PRINTK ("%s: %s\n", __FUNCTION__, mode->kind);
	if (!is_good_mode_name(mode->kind) ||
	    mode->priv_size > TEAM_MODE_PRIV_SIZE)
		return -EINVAL;
	spin_lock(&mode_list_lock);
	if (__find_mode(mode->kind)) {
		err = -EEXIST;
		goto unlock;
	}
	list_add_tail(&mode->list, &mode_list);
unlock:
	spin_unlock(&mode_list_lock);
	return err;
}
EXPORT_SYMBOL(team_mode_register);

int team_mode_unregister(struct team_mode *mode)
{
	PRINTK ("%s: %s\n", __FUNCTION__, mode->kind);
	spin_lock(&mode_list_lock);
	list_del_init(&mode->list);
	spin_unlock(&mode_list_lock);
	return 0;
}
EXPORT_SYMBOL(team_mode_unregister);

static struct team_mode *team_mode_get(const char *kind)
{
	struct team_mode *mode;

	spin_lock(&mode_list_lock);
	mode = __find_mode(kind);
	if (!mode) {
		spin_unlock(&mode_list_lock);
		request_module("team-mode-%s", kind);
		spin_lock(&mode_list_lock);
		mode = __find_mode(kind);
	}
	if (mode)
		if (!try_module_get(mode->owner))
			mode = NULL;

	spin_unlock(&mode_list_lock);
	return mode;
}

static void team_mode_put(const struct team_mode *mode)
{
	module_put(mode->owner);
}

static bool team_dummy_transmit(struct team *team, struct sk_buff *skb)
{
	dev_kfree_skb_any(skb);
	return false;
}

#ifndef	N2OS
rx_handler_result_t team_dummy_receive(struct team *team,
				       struct team_port *port,
				       struct sk_buff *skb)
{
	return RX_HANDLER_ANOTHER;
}
#endif

static void team_adjust_ops(struct team *team)
{
	/*
	 * To avoid checks in rx/tx skb paths, ensure here that non-null and
	 * correct ops are always set.
	 */

	if (list_empty(&team->port_list) ||
	    !team->mode || !team->mode->ops->transmit)
		team->ops.transmit = team_dummy_transmit;
	else
		team->ops.transmit = team->mode->ops->transmit;

#ifndef	N2OS
	if (list_empty(&team->port_list) ||
	    !team->mode || !team->mode->ops->receive)
		team->ops.receive = team_dummy_receive;
	else
		team->ops.receive = team->mode->ops->receive;
#endif
}

/*
 * We can benefit from the fact that it's ensured no port is present
 * at the time of mode change. Therefore no packets are in fly so there's no
 * need to set mode operations in any special way.
 */
static int __team_change_mode(struct team *team,
			      const struct team_mode *new_mode)
{
	/* Check if mode was previously set and do cleanup if so */
	if (team->mode) {
		void (*exit_op)(struct team *team) = team->ops.exit;

		/* Clear ops area so no callback is called any longer */
		memset(&team->ops, 0, sizeof(struct team_mode_ops));
		team_adjust_ops(team);

		if (exit_op)
			exit_op(team);
		team_mode_put(team->mode);
		team->mode = NULL;
		/* zero private data area */
		memset(&team->mode_priv, 0,
		       sizeof(struct team) - offsetof(struct team, mode_priv));
	}

	if (!new_mode)
		return 0;

	if (new_mode->ops->init) {
		int err;

		err = new_mode->ops->init(team);
		if (err)
			return err;
	}

	team->mode = new_mode;
	memcpy(&team->ops, new_mode->ops, sizeof(struct team_mode_ops));
	team_adjust_ops(team);

	return 0;
}

static int team_change_mode(struct team *team, const char *kind)
{
	struct team_mode *new_mode;
	struct net_device *dev = team->dev;
	int err;

	if (!list_empty(&team->port_list)) {
		netdev_err(dev, "No ports can be present during mode change\n");
		return -EBUSY;
	}

	if (team->mode && strcmp(team->mode->kind, kind) == 0) {
		netdev_err(dev, "Unable to change to the same mode the team is in\n");
		return -EINVAL;
	}

	new_mode = team_mode_get(kind);
	if (!new_mode) {
		netdev_err(dev, "Mode \"%s\" not found\n", kind);
		return -EINVAL;
	}

	err = __team_change_mode(team, new_mode);
	if (err) {
		netdev_err(dev, "Failed to change to mode \"%s\"\n", kind);
		team_mode_put(new_mode);
		return err;
	}

	netdev_info(dev, "Mode changed to \"%s\"\n", kind);
	return 0;
}


/************************
 * Rx path frame handler
 ************************/

#ifdef	N2OS//_PLUS_IP
static inline void team_eth_type_trans(struct sk_buff *skb, struct net_device *team_dev)
{
	PRINTK ("%s: %s\n", __FUNCTION__, team_dev->name);
	if (compare_ether_addr_64bits(eth_hdr(skb)->h_dest, team_dev->dev_addr) == 0)
		skb->pkt_type = PACKET_HOST;
}

static int team_rcv(struct sk_buff *skb, struct net_device *dev,
               struct packet_type *pt, struct net_device *orig_dev)
{
	if ((skb->pkt_type == PACKET_OTHERHOST) && !(dev->priv_flags & IFF_SLAVE)) {
		PRINTK ("%s: %s (%s)\n", __FUNCTION__, dev->name, orig_dev->name);
		team_eth_type_trans (skb, dev);
	}
	kfree_skb (skb);
	return 0;
}

static struct packet_type team_packet_type __read_mostly = {
    .type = cpu_to_be16(ETH_P_ALL),
    .func = team_rcv,
};
#endif

/* note: already called with rcu_read_lock */
#ifndef	N2OS
static rx_handler_result_t team_handle_frame(struct sk_buff **pskb)
{
	struct sk_buff *skb = *pskb;
	struct team_port *port;
	struct team *team;
	rx_handler_result_t res;

	skb = skb_share_check(skb, GFP_ATOMIC);
	if (!skb)
		return RX_HANDLER_CONSUMED;

	*pskb = skb;

	port = team_port_get_rcu(skb->dev);
	team = port->team;

	res = team->ops.receive(team, port, skb);
	if (res == RX_HANDLER_ANOTHER) {
		struct team_pcpu_stats *pcpu_stats;

		pcpu_stats = this_cpu_ptr(team->pcpu_stats);
		u64_stats_update_begin(&pcpu_stats->syncp);
		pcpu_stats->rx_packets++;
		pcpu_stats->rx_bytes += skb->len;
		if (skb->pkt_type == PACKET_MULTICAST)
			pcpu_stats->rx_multicast++;
		u64_stats_update_end(&pcpu_stats->syncp);

		skb->dev = team->dev;
	} else {
		this_cpu_inc(team->pcpu_stats->rx_dropped);
	}

	return res;
}
#endif

/****************
 * Port handling
 ****************/

static bool team_port_find(const struct team *team,
			   const struct team_port *port)
{
	struct team_port *cur;

	list_for_each_entry(cur, &team->port_list, list)
		if (cur == port)
			return true;
	return false;
}

/*
 * Add/delete port to the team port list. Write guarded by rtnl_lock.
 * Takes care of correct port->index setup (might be racy).
 */
static void team_port_list_add_port(struct team *team,
				    struct team_port *port)
{
	PRINTK ("%s: %s@%s\n", __FUNCTION__, team->dev->name, port->dev->name);
	port->index = team->port_count++;
	hlist_add_head_rcu(&port->hlist,
			   team_port_index_hash(team, port->index));
	list_add_tail_rcu(&port->list, &team->port_list);
	PRINTK ("%s: %s@%s end\n", __FUNCTION__, team->dev->name, port->dev->name);
}

static void __reconstruct_port_hlist(struct team *team, int rm_index)
{
	int i;
	struct team_port *port;

	for (i = rm_index + 1; i < team->port_count; i++) {
		port = team_get_port_by_index(team, i);
		hlist_del_rcu(&port->hlist);
		port->index--;
		hlist_add_head_rcu(&port->hlist,
				   team_port_index_hash(team, port->index));
	}
}

static void team_port_list_del_port(struct team *team,
				   struct team_port *port)
{
	int rm_index = port->index;

	hlist_del_rcu(&port->hlist);
	list_del_rcu(&port->list);
	__reconstruct_port_hlist(team, rm_index);
	team->port_count--;
}

#ifdef	N2OS
#define TEAM_VLAN_FEATURES (NETIF_F_ALL_CSUM | NETIF_F_SG | \
			    NETIF_F_FRAGLIST | \
			    NETIF_F_HIGHDMA | NETIF_F_LRO)
#else
#define TEAM_VLAN_FEATURES (NETIF_F_ALL_CSUM | NETIF_F_SG | \
			    NETIF_F_FRAGLIST | NETIF_F_ALL_TSO | \
			    NETIF_F_HIGHDMA | NETIF_F_LRO)
#endif

static void __team_compute_features(struct team *team)
{
	struct team_port *port;
	u32 vlan_features = TEAM_VLAN_FEATURES;
	unsigned short max_hard_header_len = ETH_HLEN;

	PRINTK ("%s: %s\n", __FUNCTION__, team->dev->name);
	list_for_each_entry(port, &team->port_list, list) {
		vlan_features = netdev_increment_features(vlan_features,
					port->dev->vlan_features,
					TEAM_VLAN_FEATURES);

		if (port->dev->hard_header_len > max_hard_header_len)
			max_hard_header_len = port->dev->hard_header_len;
	}

	PRINTK ("%s: %s middle\n", __FUNCTION__, team->dev->name);
	team->dev->vlan_features = vlan_features;
	team->dev->hard_header_len = max_hard_header_len;

#ifdef	N2OS	//refer. bonding
	team->dev->vlan_features = netdev_fix_features(vlan_features, NULL);
#else
	netdev_change_features(team->dev);
#endif
	PRINTK ("%s: %s end\n", __FUNCTION__, team->dev->name);
}

static void team_compute_features(struct team *team)
{
	mutex_lock(&team->lock);
	__team_compute_features(team);
	mutex_unlock(&team->lock);
}

static int team_port_enter(struct team *team, struct team_port *port)
{
	int err = 0;

	dev_hold(team->dev);
#ifdef	N2OS	//refer. bonding
	port->dev->priv_flags |= IFF_SLAVE;
#else
	port->dev->priv_flags |= IFF_TEAM_PORT;
#endif
	if (team->ops.port_enter) {
		err = team->ops.port_enter(team, port);
		if (err) {
			netdev_err(team->dev, "Device %s failed to enter team mode\n",
				   port->dev->name);
			goto err_port_enter;
		}
	}

	return 0;

err_port_enter:
#ifdef	N2OS	//refer. bonding
	port->dev->priv_flags &= ~IFF_SLAVE;
#else
	port->dev->priv_flags &= ~IFF_TEAM_PORT;
#endif
	dev_put(team->dev);

	return err;
}

static void team_port_leave(struct team *team, struct team_port *port)
{
	if (team->ops.port_leave)
		team->ops.port_leave(team, port);
#ifdef	N2OS	//refer. bonding
	port->dev->priv_flags &= ~IFF_SLAVE;
#else
	port->dev->priv_flags &= ~IFF_TEAM_PORT;
#endif
	dev_put(team->dev);
}

static void __team_port_change_check(struct team_port *port, bool linkup);

static int team_port_add(struct team *team, struct net_device *port_dev)
{
	struct net_device *dev = team->dev;
	struct team_port *port;
	char *portname = port_dev->name;
	int err;

	PRINTK ("%s: %s@%s\n", __FUNCTION__, team->dev->name, port_dev->name);
	if (port_dev->flags & IFF_LOOPBACK ||
	    port_dev->type != ARPHRD_ETHER) {
		netdev_err(dev, "Device %s is of an unsupported type\n",
			   portname);
		return -EINVAL;
	}

	if (team_port_exists(port_dev)) {
		netdev_err(dev, "Device %s is already a port "
				"of a team device\n", portname);
		return -EBUSY;
	}

	if (port_dev->flags & IFF_UP) {
		netdev_err(dev, "Device %s is up. Set it down before adding it as a team port\n",
			   portname);
		return -EBUSY;
	}

	port = kzalloc(sizeof(struct team_port), GFP_KERNEL);
	if (!port)
		return -ENOMEM;

	port->dev = port_dev;
	port->team = team;

	port->orig.mtu = port_dev->mtu;
	err = dev_set_mtu(port_dev, dev->mtu);
	if (err) {
		netdev_dbg(dev, "Error %d calling dev_set_mtu\n", err);
		goto err_set_mtu;
	}

	memcpy(port->orig.dev_addr, port_dev->dev_addr, ETH_ALEN);

	err = team_port_enter(team, port);
	if (err) {
		netdev_err(dev, "Device %s failed to enter team mode\n",
			   portname);
		goto err_port_enter;
	}

	err = dev_open(port_dev);
	if (err) {
		netdev_dbg(dev, "Device %s opening failed\n",
			   portname);
		goto err_dev_open;
	}

#ifdef	N2OS	//refer. bonding
	vlan_vids_add_by_dev(team, port_dev);
#else
	err = vlan_vids_add_by_dev(port_dev, dev);
	if (err) {
		netdev_err(dev, "Failed to add vlan ids to device %s\n",
				portname);
		goto err_vids_add;
	}
#endif

	err = netdev_set_master(port_dev, dev);
	if (err) {
		netdev_err(dev, "Device %s failed to set master\n", portname);
		goto err_set_master;
	}

#ifdef	N2OS
	team_port_local_register (port);
#else
	err = netdev_rx_handler_register(port_dev, team_handle_frame,
					 port);
	if (err) {
		netdev_err(dev, "Device %s failed to register rx_handler\n",
			   portname);
		goto err_handler_register;
	}
#endif

	team_port_list_add_port(team, port);
	team_adjust_ops(team);
	__team_compute_features(team);
	__team_port_change_check(port, !!netif_carrier_ok(port_dev));
#ifdef	N2OS
	port->enabled = 1;
	port->user_linkup = 1;
	port->priority = 0;
#if 0	//FFS
	if (team_aggregation_link_enable)
		team_aggregation_link_enable (port_dev, false);
#endif
#endif

	netdev_info(dev, "Port device %s added\n", portname);

	return 0;

#ifndef	N2OS
err_handler_register:
#endif
	netdev_set_master(port_dev, NULL);

err_set_master:
#ifdef	N2OS	//refer. bonding
	vlan_vids_del_by_dev(team, port_dev);
#else
	vlan_vids_del_by_dev(port_dev, dev);
#endif

#ifndef	N2OS
err_vids_add:
#endif
	dev_close(port_dev);

err_dev_open:
	team_port_leave(team, port);
	team_port_set_orig_mac(port);

err_port_enter:
	dev_set_mtu(port_dev, port->orig.mtu);

err_set_mtu:
	kfree(port);

	return err;
}

static int team_port_del(struct team *team, struct net_device *port_dev)
{
	struct net_device *dev = team->dev;
	struct team_port *port;
	char *portname = port_dev->name;

	PRINTK ("%s: %s@%s\n", __FUNCTION__, team->dev->name, port_dev->name);

#ifdef	N2OS
	port = team_port_local_get(port_dev);
#else
	port = team_port_get_rtnl(port_dev);
#endif
	if (!port || !team_port_find(team, port)) {
		netdev_err(dev, "Device %s does not act as a port of this team\n",
			   portname);
		return -ENOENT;
	}

#ifdef	N2OS
	port->enabled = 0;
	port->user_linkup = 0;
#if 0	//FFS
	if (team_aggregation_link_enable)
		team_aggregation_link_enable (port_dev, false);
#endif
#endif
	port->removed = true;
	__team_port_change_check(port, false);
	team_port_list_del_port(team, port);
	team_adjust_ops(team);
#ifdef	N2OS
	team_port_local_unregister (port);
#else
	netdev_rx_handler_unregister(port_dev);
#endif
	netdev_set_master(port_dev, NULL);
#ifdef	N2OS	//refer. bonding
	vlan_vids_del_by_dev(team, port_dev);
#else
	vlan_vids_del_by_dev(port_dev, dev);
#endif
	dev_close(port_dev);
	team_port_leave(team, port);
	team_port_set_orig_mac(port);
	dev_set_mtu(port_dev, port->orig.mtu);
	synchronize_rcu();
	kfree(port);
	netdev_info(dev, "Port device %s removed\n", portname);
	__team_compute_features(team);

	return 0;
}

/*****************
 * Net device ops
 *****************/

static const char team_no_mode_kind[] = "*NOMODE*";

static int team_mode_option_get(struct team *team, void *arg)
{
	const char **str = arg;

	*str = team->mode ? team->mode->kind : team_no_mode_kind;
	PRINTK ("%s: %s (ptr=%p)\n", __FUNCTION__, team->dev->name, *str);
	return 0;
}

static int team_mode_option_set(struct team *team, void *arg)
{
	const char **str = arg;

	PRINTK ("%s: %s\n", __FUNCTION__, team->dev->name);
	return team_change_mode(team, *str);
}

static const struct team_option team_options[] = {
	{
		.name = "mode",
		.type = TEAM_OPTION_TYPE_STRING,
		.getter = team_mode_option_get,
		.setter = team_mode_option_set,
	},
};

static int team_init(struct net_device *dev)
{
	struct team *team = netdev_priv(dev);
	int i;
	int err;

	PRINTK ("%s: %s\n", __FUNCTION__, dev->name);
	team->dev = dev;
	mutex_init(&team->lock);

#ifdef	N2OS	//refer. bonding
	INIT_LIST_HEAD(&team->vlan_list);
#endif
	team->pcpu_stats = alloc_percpu(struct team_pcpu_stats);
	if (!team->pcpu_stats)
		return -ENOMEM;

	for (i = 0; i < TEAM_PORT_HASHENTRIES; i++)
		INIT_HLIST_HEAD(&team->port_hlist[i]);
	INIT_LIST_HEAD(&team->port_list);

	team_adjust_ops(team);

	INIT_LIST_HEAD(&team->option_list);
	err = team_options_register(team, team_options, ARRAY_SIZE(team_options));
	if (err)
		goto err_options_register;
	netif_carrier_off(dev);

	PRINTK ("%s: %s end\n", __FUNCTION__, dev->name);
	return 0;

err_options_register:
	free_percpu(team->pcpu_stats);

	return err;
}

static void team_uninit(struct net_device *dev)
{
	struct team *team = netdev_priv(dev);
	struct team_port *port;
	struct team_port *tmp;

	PRINTK ("%s: %s\n", __FUNCTION__, dev->name);
	mutex_lock(&team->lock);
	list_for_each_entry_safe(port, tmp, &team->port_list, list)
		team_port_del(team, port->dev);

	__team_change_mode(team, NULL); /* cleanup */
	__team_options_unregister(team, team_options, ARRAY_SIZE(team_options));
	mutex_unlock(&team->lock);
}

static void team_destructor(struct net_device *dev)
{
	struct team *team = netdev_priv(dev);

	PRINTK ("%s: %s\n", __FUNCTION__, dev->name);
	free_percpu(team->pcpu_stats);
	free_netdev(dev);
}

static int team_open(struct net_device *dev)
{
	PRINTK ("%s: %s\n", __FUNCTION__, dev->name);
	netif_carrier_on(dev);
	return 0;
}

static int team_close(struct net_device *dev)
{
	PRINTK ("%s: %s\n", __FUNCTION__, dev->name);
	netif_carrier_off(dev);
	return 0;
}

#ifndef this_cpu_inc
# define this_cpu_inc(pcp)      this_cpu_add((pcp), 1)
#endif

/*
 * note: already called with rcu_read_lock
 */
static netdev_tx_t team_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct team *team = netdev_priv(dev);
	bool tx_success = false;
	unsigned int len = skb->len;
	struct team_pcpu_stats *pcpu_stats;

	PRINTK ("%s: %s\n", __FUNCTION__, dev->name);
	tx_success = team->ops.transmit(team, skb);

#if	LINUX_VERSION_CODE == KERNEL_VERSION (2,6,32)
	pcpu_stats = per_cpu_ptr(team->pcpu_stats, 0);
#endif
#if	LINUX_VERSION_CODE == KERNEL_VERSION (2,6,35)
	pcpu_stats = this_cpu_ptr(team->pcpu_stats);
#endif
	u64_stats_update_begin(&pcpu_stats->syncp);
	if (tx_success) {
		pcpu_stats->tx_packets++;
		pcpu_stats->tx_bytes += len;
	} else {
		pcpu_stats->tx_dropped++;
	}
	u64_stats_update_end(&pcpu_stats->syncp);

	return NETDEV_TX_OK;
}

static void team_change_rx_flags(struct net_device *dev, int change)
{
	struct team *team = netdev_priv(dev);
	struct team_port *port;
	int inc;

	PRINTK ("%s: %s\n", __FUNCTION__, dev->name);
	rcu_read_lock();
	list_for_each_entry_rcu(port, &team->port_list, list) {
		if (change & IFF_PROMISC) {
			inc = dev->flags & IFF_PROMISC ? 1 : -1;
			dev_set_promiscuity(port->dev, inc);
		}
		if (change & IFF_ALLMULTI) {
			inc = dev->flags & IFF_ALLMULTI ? 1 : -1;
			dev_set_allmulti(port->dev, inc);
		}
	}
	rcu_read_unlock();
}

static void team_set_rx_mode(struct net_device *dev)
{
	struct team *team = netdev_priv(dev);
	struct team_port *port;

	PRINTK ("%s: %s\n", __FUNCTION__, dev->name);
	rcu_read_lock();
	list_for_each_entry_rcu(port, &team->port_list, list) {
#if	LINUX_VERSION_CODE == KERNEL_VERSION (2,6,32)
		dev_unicast_sync(port->dev, dev);
#endif
#if	LINUX_VERSION_CODE == KERNEL_VERSION (2,6,35)
		dev_uc_sync(port->dev, dev);
#endif
		dev_mc_sync(port->dev, dev);
	}
	rcu_read_unlock();
}

static int team_set_mac_address(struct net_device *dev, void *p)
{
	struct team *team = netdev_priv(dev);
	struct team_port *port;
	struct sockaddr *addr = p;

	PRINTK ("%s: %s\n", __FUNCTION__, dev->name);
#ifndef	N2OS
	dev->addr_assign_type &= ~NET_ADDR_RANDOM;
#endif
	memcpy(dev->dev_addr, addr->sa_data, ETH_ALEN);
	rcu_read_lock();
	list_for_each_entry_rcu(port, &team->port_list, list)
		if (team->ops.port_change_mac)
			team->ops.port_change_mac(team, port);
	rcu_read_unlock();
	return 0;
}

static int team_change_mtu(struct net_device *dev, int new_mtu)
{
	struct team *team = netdev_priv(dev);
	struct team_port *port;
	int err;

	PRINTK ("%s: %s\n", __FUNCTION__, dev->name);
	/*
	 * Alhough this is reader, it's guarded by team lock. It's not possible
	 * to traverse list in reverse under rcu_read_lock
	 */
	mutex_lock(&team->lock);
	list_for_each_entry(port, &team->port_list, list) {
		err = dev_set_mtu(port->dev, new_mtu);
		if (err) {
			netdev_err(dev, "Device %s failed to change mtu",
				   port->dev->name);
			goto unwind;
		}
	}
	mutex_unlock(&team->lock);

	dev->mtu = new_mtu;

	return 0;

unwind:
	list_for_each_entry_continue_reverse(port, &team->port_list, list)
		dev_set_mtu(port->dev, dev->mtu);
	mutex_unlock(&team->lock);

	return err;
}

#ifdef	N2OS
static struct net_device_stats *team_get_stats(struct net_device *dev)
{
	struct net_device_stats *stats, total = {0};
	struct team *team = netdev_priv(dev);
	struct team_port *port;

	stats = &dev->stats;

	memset(&total, 0, sizeof(struct net_device_stats));

	mutex_lock(&team->lock);

	list_for_each_entry(port, &team->port_list, list) {
		const struct net_device_stats *sstats = dev_get_stats(port->dev);

		total.rx_packets += sstats->rx_packets;
		total.rx_bytes += sstats->rx_bytes;
		total.rx_errors += sstats->rx_errors;
		total.rx_dropped += sstats->rx_dropped;

		total.tx_packets += sstats->tx_packets;
		total.tx_bytes += sstats->tx_bytes;
		total.tx_errors += sstats->tx_errors;
		total.tx_dropped += sstats->tx_dropped;

		total.multicast += sstats->multicast;
		total.collisions += sstats->collisions;

		total.rx_length_errors += sstats->rx_length_errors;
		total.rx_over_errors += sstats->rx_over_errors;
		total.rx_crc_errors += sstats->rx_crc_errors;
		total.rx_frame_errors += sstats->rx_frame_errors;
		total.rx_fifo_errors += sstats->rx_fifo_errors;
		total.rx_missed_errors += sstats->rx_missed_errors;

		total.tx_aborted_errors += sstats->tx_aborted_errors;
		total.tx_carrier_errors += sstats->tx_carrier_errors;
		total.tx_fifo_errors += sstats->tx_fifo_errors;
		total.tx_heartbeat_errors += sstats->tx_heartbeat_errors;
		total.tx_window_errors += sstats->tx_window_errors;
	}

	mutex_unlock(&team->lock);

	memcpy (stats, &total, sizeof(struct net_device_stats));

	return stats;
}

#else
static struct rtnl_link_stats64 *
team_get_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	struct team *team = netdev_priv(dev);
	struct team_pcpu_stats *p;
	u64 rx_packets, rx_bytes, rx_multicast, tx_packets, tx_bytes;
	u32 rx_dropped = 0, tx_dropped = 0;
	unsigned int start;
	int i;

	for_each_possible_cpu(i) {
		p = per_cpu_ptr(team->pcpu_stats, i);
		do {
			start = u64_stats_fetch_begin_bh(&p->syncp);
			rx_packets	= p->rx_packets;
			rx_bytes	= p->rx_bytes;
			rx_multicast	= p->rx_multicast;
			tx_packets	= p->tx_packets;
			tx_bytes	= p->tx_bytes;
		} while (u64_stats_fetch_retry_bh(&p->syncp, start));

		stats->rx_packets	+= rx_packets;
		stats->rx_bytes		+= rx_bytes;
		stats->multicast	+= rx_multicast;
		stats->tx_packets	+= tx_packets;
		stats->tx_bytes		+= tx_bytes;
		/*
		 * rx_dropped & tx_dropped are u32, updated
		 * without syncp protection.
		 */
		rx_dropped	+= p->rx_dropped;
		tx_dropped	+= p->tx_dropped;
	}
	stats->rx_dropped	= rx_dropped;
	stats->tx_dropped	= tx_dropped;
	return stats;
}
#endif

#ifdef	N2OS	//refer. bonding
struct vlan_entry {
    struct list_head vlan_list;
    unsigned short vlan_id;
#if 0
    __be32 vlan_ip;
#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
    struct in6_addr vlan_ipv6;
#endif
#endif
};

static void team_vlan_rx_register(struct net_device *dev, struct vlan_group *vlan_group)
{
    struct team *team = netdev_priv(dev);
	struct team_port *port;

	PRINTK ("%s: %s \n", __FUNCTION__, dev->name);
	mutex_lock(&team->lock);
    team->vlan_group = vlan_group;
	list_for_each_entry(port, &team->port_list, list) {
		struct net_device *port_dev = port->dev;
        const struct net_device_ops *port_dev_ops = port_dev->netdev_ops;

        if ((port_dev->features & NETIF_F_HW_VLAN_RX) &&
            port_dev_ops->ndo_vlan_rx_register) {
            port_dev_ops->ndo_vlan_rx_register(port_dev, vlan_group);
        }
    }
	mutex_unlock(&team->lock);
}

static void team_vlan_rx_add_vid(struct net_device *dev, uint16_t vid)
{
	struct team *team = netdev_priv(dev);
	struct team_port *port;

	PRINTK ("%s: %s vid 0x%x\n", __FUNCTION__, dev->name, vid);
	/*
	 * Alhough this is reader, it's guarded by team lock. It's not possible
	 * to traverse list in reverse under rcu_read_lock
	 */
	mutex_lock(&team->lock);
	list_for_each_entry(port, &team->port_list, list) {
		struct net_device *port_dev = port->dev;
        const struct net_device_ops *port_dev_ops = port_dev->netdev_ops;

        if ((port_dev->features & NETIF_F_HW_VLAN_RX) &&
            port_dev_ops->ndo_vlan_rx_add_vid) {
            port_dev_ops->ndo_vlan_rx_add_vid(port_dev, vid);
        }
    }
	mutex_unlock(&team->lock);
}

static void team_vlan_rx_kill_vid(struct net_device *dev, uint16_t vid)
{
	struct team *team = netdev_priv(dev);
	struct team_port *port;
	struct net_device *vlan_dev;

	PRINTK ("%s: %s vid 0x%x\n", __FUNCTION__, dev->name, vid);
	mutex_lock(&team->lock);
	list_for_each_entry(port, &team->port_list, list) {
		struct net_device *port_dev = port->dev;
        const struct net_device_ops *port_dev_ops = port_dev->netdev_ops;

        if ((port_dev->features & NETIF_F_HW_VLAN_RX) &&
            port_dev_ops->ndo_vlan_rx_kill_vid) {
            vlan_dev = vlan_group_get_device(team->vlan_group, vid);
            port_dev_ops->ndo_vlan_rx_kill_vid(port_dev, vid);
            vlan_group_set_device(team->vlan_group, vid, vlan_dev);
        }
    }
	mutex_unlock(&team->lock);
}

void vlan_vids_add_by_dev(struct team *team, struct net_device *port_dev)
{
	struct vlan_entry *vlan;
	const struct net_device_ops *port_dev_ops = port_dev->netdev_ops;

	PRINTK ("%s: %s-%s\n", __FUNCTION__, team->dev->name, port_dev->name);
    if (list_empty(&team->vlan_list))
        goto out;

    if ((port_dev->features & NETIF_F_HW_VLAN_RX) &&
        port_dev_ops->ndo_vlan_rx_register)
        port_dev_ops->ndo_vlan_rx_register(port_dev, team->vlan_group);

    if (!(port_dev->features & NETIF_F_HW_VLAN_FILTER) ||
        !(port_dev_ops->ndo_vlan_rx_add_vid))
        goto out;

    list_for_each_entry(vlan, &team->vlan_list, vlan_list)
        port_dev_ops->ndo_vlan_rx_add_vid(port_dev, vlan->vlan_id);

out:
	PRINTK ("%s: %s-%s end\n", __FUNCTION__, team->dev->name, port_dev->name);
}

void vlan_vids_del_by_dev(struct team *team, struct net_device *port_dev)
{
	struct net_device *vlan_dev;
	struct vlan_entry *vlan;
	const struct net_device_ops *port_dev_ops = port_dev->netdev_ops;

	PRINTK ("%s: %s-%s\n", __FUNCTION__, team->dev->name, port_dev->name);
    if (list_empty(&team->vlan_list))
        goto out;

    if (!(port_dev->features & NETIF_F_HW_VLAN_FILTER) ||
        !(port_dev_ops->ndo_vlan_rx_kill_vid))
        goto unreg;

    list_for_each_entry(vlan, &team->vlan_list, vlan_list) {
        /* Save and then restore vlan_dev in the vlan_group array,
 *          * since the port_dev's driver might clear it.
 *                   */
        vlan_dev = vlan_group_get_device(team->vlan_group, vlan->vlan_id);
        port_dev_ops->ndo_vlan_rx_kill_vid(port_dev, vlan->vlan_id);
        vlan_group_set_device(team->vlan_group, vlan->vlan_id, vlan_dev);
    }

unreg:
    if ((port_dev->features & NETIF_F_HW_VLAN_RX) &&
        port_dev_ops->ndo_vlan_rx_register)
        port_dev_ops->ndo_vlan_rx_register(port_dev, NULL);

out:
	PRINTK ("%s: %s-%s end\n", __FUNCTION__, team->dev->name, port_dev->name);
}
#else
static int team_vlan_rx_add_vid(struct net_device *dev, uint16_t vid)
{
	struct team *team = netdev_priv(dev);
	struct team_port *port;
	int err;

	/*
	 * Alhough this is reader, it's guarded by team lock. It's not possible
	 * to traverse list in reverse under rcu_read_lock
	 */
	mutex_lock(&team->lock);
	list_for_each_entry(port, &team->port_list, list) {
		err = vlan_vid_add(port->dev, vid);
		if (err)
			goto unwind;
	}
	mutex_unlock(&team->lock);

	return 0;

unwind:
	list_for_each_entry_continue_reverse(port, &team->port_list, list)
		vlan_vid_del(port->dev, vid);
	mutex_unlock(&team->lock);

	return err;
}
static int team_vlan_rx_kill_vid(struct net_device *dev, uint16_t vid)
{
	struct team *team = netdev_priv(dev);
	struct team_port *port;

	PRINTK ("%s: %s vid 0x%x\n", __FUNCTION__, dev->name, vid);
	rcu_read_lock();
	list_for_each_entry_rcu(port, &team->port_list, list)
		vlan_vid_del(port->dev, vid);
	rcu_read_unlock();

	return 0;
}
#endif

static int team_add_slave(struct net_device *dev, struct net_device *port_dev)
{
	struct team *team = netdev_priv(dev);
	int err;

	PRINTK ("%s: %s\n", __FUNCTION__, dev->name);
	mutex_lock(&team->lock);
	err = team_port_add(team, port_dev);
	mutex_unlock(&team->lock);
	return err;
}

static int team_del_slave(struct net_device *dev, struct net_device *port_dev)
{
	struct team *team = netdev_priv(dev);
	int err;

	PRINTK ("%s: %s\n", __FUNCTION__, dev->name);
	mutex_lock(&team->lock);
	err = team_port_del(team, port_dev);
	mutex_unlock(&team->lock);
	return err;
}

#ifdef	N2OS
static int team_enable_slave(struct net_device *dev, struct net_device *port_dev)
{
	int err = -ENODEV;
	struct team_port *port;
	struct team *team = netdev_priv(dev);
	PRINTK ("%s: %s\n", __FUNCTION__, dev->name);

	mutex_lock(&team->lock);
	list_for_each_entry(port, &team->port_list, list) {
		if (port_dev == port->dev) {
			port->enabled = 1;
			netdev_info(dev, "%s: enable enabled state @ slave %s\n", dev->name, port_dev->name);
			err = 0;
			break;
		}
	}
	mutex_unlock(&team->lock);
	return err;
}

static int team_disable_slave(struct net_device *dev, struct net_device *port_dev)
{
	int err = -ENODEV;
	struct team_port *port;
	struct team *team = netdev_priv(dev);
	PRINTK ("%s: %s\n", __FUNCTION__, dev->name);
	mutex_lock(&team->lock);
	list_for_each_entry(port, &team->port_list, list) {
		if (port_dev == port->dev) {
			port->enabled = 0;
			err = 0;
			netdev_info(dev, "%s: disable enabled state @ slave %s\n", dev->name, port_dev->name);
			break;
		}
	}
	mutex_unlock(&team->lock);
	return err;
}

static int team_get_slave_enabled(struct net_device *dev, struct net_device *port_dev, int *flag)
{
	int err = -ENODEV;
	struct team_port *port;
	struct team *team = netdev_priv(dev);
	PRINTK ("%s: %s\n", __FUNCTION__, dev->name);

	mutex_lock(&team->lock);
	list_for_each_entry(port, &team->port_list, list) {
		if (port_dev == port->dev) {
			*flag = port->enabled ? 1 : 0;
			err = 0;
			netdev_info(dev, "%s: get enabled state \"%s\" @ slave %s\n", dev->name, *flag ? "enable" : "disable", port_dev->name);
			break;
		}
	}
	mutex_unlock(&team->lock);
	return err;
}

static int team_set_port_user_linkup(struct net_device *dev, struct net_device *port_dev)
{
	int err = -ENODEV;
	struct team_port *port;
	struct team *team = netdev_priv(dev);
	PRINTK ("%s: %s\n", __FUNCTION__, dev->name);
	mutex_lock(&team->lock);
	list_for_each_entry(port, &team->port_list, list) {
		if (port_dev == port->dev) {
			port->user_linkup = 1;
			err = 0;
#if 0	//FFS
			if (team_aggregation_link_enable)
				team_aggregation_link_enable (port_dev, true);
#endif
			netdev_info(dev, "%s: set user_linkup state @ slave %s\n", dev->name, port_dev->name);
			break;
		}
	}
	mutex_unlock(&team->lock);
	return err;
}

static int team_reset_port_user_linkup(struct net_device *dev, struct net_device *port_dev)
{
	int err = -ENODEV;
	struct team_port *port;
	struct team *team = netdev_priv(dev);
	PRINTK ("%s: %s\n", __FUNCTION__, dev->name);
	mutex_lock(&team->lock);
	list_for_each_entry(port, &team->port_list, list) {
		if (port_dev == port->dev) {
			port->user_linkup = 0;
			err = 0;
#if 0	//FFS
			if (team_aggregation_link_enable)
				team_aggregation_link_enable (port_dev, false);
#endif
			netdev_info(dev, "%s: reset user_linkup state @ slave %s\n", dev->name, port_dev->name);
			break;
		}
	}
	mutex_unlock(&team->lock);
	return err;
}

static int team_get_port_user_linkup(struct net_device *dev, struct net_device *port_dev, int *flag)
{
	int err = -ENODEV;
	struct team_port *port;
	struct team *team = netdev_priv(dev);
	PRINTK ("%s: %s\n", __FUNCTION__, dev->name);

	mutex_lock(&team->lock);
	list_for_each_entry(port, &team->port_list, list) {
		if (port_dev == port->dev) {
			*flag = port->user_linkup ? 1 : 0;
			err = 0;
			netdev_info(dev, "%s: get user_linkup state %s @ slave %s\n", dev->name, *flag ? "up" : "down", port_dev->name);
			break;
		}
	}
	mutex_unlock(&team->lock);
	return err;
}

struct ifr_port_priority {
	int ifindex;
	uint32_t priority;
};

static int team_set_port_priority(struct net_device *dev, struct net_device *port_dev, int priority)
{
	int err = -ENODEV;
	struct team_port *port;
	struct team *team = netdev_priv(dev);
	PRINTK ("%s: %s\n", __FUNCTION__, dev->name);
	mutex_lock(&team->lock);
	list_for_each_entry(port, &team->port_list, list) {
		if (port_dev == port->dev) {
			port->priority = priority;
			err = 0;
			netdev_info(dev, "%s: set priority %d @ slave %s\n", dev->name, priority, port_dev->name);
			break;
		}
	}
	mutex_unlock(&team->lock);
	return err;
}

static int team_get_port_priority(struct net_device *dev, struct net_device *port_dev, int *priority)
{
	int err = -ENODEV;
	struct team_port *port;
	struct team *team = netdev_priv(dev);
	PRINTK ("%s: %s\n", __FUNCTION__, dev->name);

	mutex_lock(&team->lock);
	list_for_each_entry(port, &team->port_list, list) {
		if (port_dev == port->dev) {
			*priority = port->priority;
			err = 0;
			netdev_info(dev, "%s: get priority %d @ slave %s\n", dev->name, *priority, port_dev->name);
			break;
		}
	}
	mutex_unlock(&team->lock);
	return err;
}

#define SIOCSTEAMADDSLAVE	(SIOCDEVPRIVATE + 0)
#define SIOCSTEAMDELSLAVE	(SIOCDEVPRIVATE + 1)
#define SIOCSTEAMENAPORT	(SIOCDEVPRIVATE + 2)
#define SIOCSTEAMDISPORT	(SIOCDEVPRIVATE + 3)
#define SIOCGTEAMGETPORT	(SIOCDEVPRIVATE + 4)
#define SIOCSTEAMENAUSER	(SIOCDEVPRIVATE + 5)
#define SIOCSTEAMDISUSER	(SIOCDEVPRIVATE + 6)
#define SIOCGTEAMGETUSER	(SIOCDEVPRIVATE + 7)
#define SIOCSTEAMPORTPRIO	(SIOCDEVPRIVATE + 8)
#define SIOCGTEAMPORTPRIO	(SIOCDEVPRIVATE + 9)

static int team_ioctl (struct net_device *dev, struct ifreq *ifr, int cmd)
{
	int	err ;
	struct ifr_port_priority *port_priority = NULL;
	struct net_device *port_dev;
	int flag = 0;
	int ifindex;

	PRINTK ("%s: %s (cmd=0x%x)\n", __FUNCTION__, dev->name, cmd);

	switch (cmd) {
	case SIOCSTEAMADDSLAVE:
	case SIOCSTEAMDELSLAVE:
	case SIOCSTEAMENAPORT:
	case SIOCSTEAMDISPORT:
	case SIOCGTEAMGETPORT:
	case SIOCSTEAMENAUSER:
	case SIOCSTEAMDISUSER:
	case SIOCGTEAMGETUSER:
		ifindex = ifr->ifr_ifindex;
		break;
	case SIOCSTEAMPORTPRIO:
	case SIOCGTEAMPORTPRIO:
		port_priority = (struct ifr_port_priority *) ifr->ifr_data;
		ifindex = port_priority->ifindex;
		break;
	default:
		return -EINVAL;
	}

	port_dev = dev_get_by_index (&init_net, ifindex);
	if (port_dev == NULL)
		return -ENODEV;

	switch (cmd)
	{
	case SIOCSTEAMADDSLAVE:
		err = team_add_slave(dev, port_dev);
		break;
	case SIOCSTEAMDELSLAVE:
		err = team_del_slave(dev, port_dev);
		break;
	case SIOCSTEAMENAPORT:
		err = team_enable_slave(dev, port_dev);
		break;
	case SIOCSTEAMDISPORT:
		err = team_disable_slave(dev, port_dev);
		break;
	case SIOCGTEAMGETPORT:
		err = team_get_slave_enabled(dev, port_dev, &flag);
		ifr->ifr_ifru.ifru_ivalue = flag;
		break;
	case SIOCSTEAMENAUSER:
		err = team_set_port_user_linkup(dev, port_dev);
		break;
	case SIOCSTEAMDISUSER:
		err = team_reset_port_user_linkup(dev, port_dev);
		break;
	case SIOCGTEAMGETUSER:
		err = team_get_port_user_linkup(dev, port_dev, &flag);
		ifr->ifr_ifru.ifru_ivalue = flag;
		break;
	case SIOCSTEAMPORTPRIO:
		err = team_set_port_priority(dev, port_dev, port_priority->priority);
		break;
	case SIOCGTEAMPORTPRIO:
		err = team_get_port_priority(dev, port_dev, &port_priority->priority);
		break;
	default:
		return -EINVAL;
	}

	dev_put (port_dev);
	PRINTK ("%s: %s index=%u (cmd=0x%x) err=%d\n", __FUNCTION__, dev->name, ifr->ifr_ifindex, cmd, err);
	return err;
}
#else
static netdev_features_t team_fix_features(struct net_device *dev,
					   netdev_features_t features)
{
	struct team_port *port;
	struct team *team = netdev_priv(dev);
	netdev_features_t mask;

	mask = features;
	features &= ~NETIF_F_ONE_FOR_ALL;
#ifndef	N2OS
	features |= NETIF_F_ALL_FOR_ALL;
#endif

	rcu_read_lock();
	list_for_each_entry_rcu(port, &team->port_list, list) {
		features = netdev_increment_features(features,
						     port->dev->features,
						     mask);
	}
	rcu_read_unlock();
	return features;
}
#endif

static const struct net_device_ops team_netdev_ops = {
	.ndo_init		= team_init,
	.ndo_uninit		= team_uninit,
	.ndo_open		= team_open,
	.ndo_stop		= team_close,
	.ndo_start_xmit		= team_xmit,
	.ndo_change_rx_flags	= team_change_rx_flags,
	.ndo_set_rx_mode	= team_set_rx_mode,
	.ndo_set_mac_address	= team_set_mac_address,
	.ndo_change_mtu		= team_change_mtu,
#ifdef	N2OS
	.ndo_get_stats	= team_get_stats,
#else
	.ndo_get_stats64	= team_get_stats64,
#endif
#ifdef	N2OS	//refer. bonding
	.ndo_vlan_rx_register	= team_vlan_rx_register,
#endif
	.ndo_vlan_rx_add_vid	= team_vlan_rx_add_vid,
	.ndo_vlan_rx_kill_vid	= team_vlan_rx_kill_vid,
#ifdef	N2OS
	.ndo_do_ioctl		= team_ioctl,
#else
	.ndo_add_slave		= team_add_slave,
	.ndo_del_slave		= team_del_slave,
	.ndo_fix_features	= team_fix_features,
#endif
};


/***********************
 * rt netlink interface
 ***********************/

static void team_setup(struct net_device *dev)
{
	PRINTK ("%s: %s\n", __FUNCTION__, dev->name);
	ether_setup(dev);

	dev->netdev_ops = &team_netdev_ops;
	dev->destructor	= team_destructor;
	dev->tx_queue_len = 0;
	dev->flags |= IFF_MULTICAST;
#ifdef	N2OS
#if 0
	dev->dev_addr[0] = 0x00;
	dev->dev_addr[1] = 0x08;
	dev->dev_addr[2] = 0x06;
	dev->dev_addr[3] = ;
	dev->dev_addr[4] = ;
	dev->dev_addr[5] = ;
	dev->priv_flags &= ~(IFF_XMIT_DST_RELEASE);
#endif
#else
	dev->priv_flags &= ~(IFF_XMIT_DST_RELEASE | IFF_TX_SKB_SHARING);
#endif

	/*
	 * Indicate we support unicast address filtering. That way core won't
	 * bring us to promisc mode in case a unicast addr is added.
	 * Let this up to underlay drivers.
	 */
#ifndef	N2OS
	dev->priv_flags |= IFF_UNICAST_FLT;
#endif

	dev->features |= NETIF_F_LLTX;
	dev->features |= NETIF_F_GRO;
#ifdef	N2OS
	dev->features |= NETIF_F_HW_VLAN_TX |
			   NETIF_F_HW_VLAN_RX |
			   NETIF_F_HW_VLAN_FILTER;
#else
	dev->hw_features = NETIF_F_HW_VLAN_TX |
			   NETIF_F_HW_VLAN_RX |
			   NETIF_F_HW_VLAN_FILTER;

	dev->features |= dev->hw_features;
#endif
}

#ifdef	N2OS
static inline void eth_random_addr(u8 *addr)
{
    get_random_bytes(addr, ETH_ALEN);
    addr[0] &= 0xfe;    /* clear multicast bit */
    addr[0] |= 0x02;    /* set local assignment bit (IEEE802) */
}
#endif

#if	LINUX_VERSION_CODE == KERNEL_VERSION (2,6,32)
static int team_newlink(struct net_device *dev,
			struct nlattr *tb[], struct nlattr *data[])
#endif
#if	LINUX_VERSION_CODE == KERNEL_VERSION (2,6,35)
static int team_newlink(struct net *src_net, struct net_device *dev,
			struct nlattr *tb[], struct nlattr *data[])
#endif
{
	int err;

#ifdef	N2OS
	eth_random_addr(dev->dev_addr);
#else
	if (tb[IFLA_ADDRESS] == NULL)
		eth_hw_addr_random(dev);
#endif

	err = register_netdevice(dev);
	if (err)
		return err;

	return 0;
}

static int team_validate(struct nlattr *tb[], struct nlattr *data[])
{
	if (tb[IFLA_ADDRESS]) {
		if (nla_len(tb[IFLA_ADDRESS]) != ETH_ALEN)
			return -EINVAL;
		if (!is_valid_ether_addr(nla_data(tb[IFLA_ADDRESS])))
			return -EADDRNOTAVAIL;
	}
	return 0;
}

static struct rtnl_link_ops team_link_ops __read_mostly = {
	.kind		= DRV_NAME,
	.priv_size	= sizeof(struct team),
	.setup		= team_setup,
	.newlink	= team_newlink,
	.validate	= team_validate,
};


/***********************************
 * Generic netlink custom interface
 ***********************************/

static struct genl_family team_nl_family = {
	.id		= GENL_ID_GENERATE,
	.name		= TEAM_GENL_NAME,
	.version	= TEAM_GENL_VERSION,
	.maxattr	= TEAM_ATTR_MAX,
	.netnsok	= true,
};

static const struct nla_policy team_nl_policy[TEAM_ATTR_MAX + 1] = {
	[TEAM_ATTR_UNSPEC]			= { .type = NLA_UNSPEC, },
	[TEAM_ATTR_TEAM_IFINDEX]		= { .type = NLA_U32 },
	[TEAM_ATTR_LIST_OPTION]			= { .type = NLA_NESTED },
	[TEAM_ATTR_LIST_PORT]			= { .type = NLA_NESTED },
};

static const struct nla_policy
team_nl_option_policy[TEAM_ATTR_OPTION_MAX + 1] = {
	[TEAM_ATTR_OPTION_UNSPEC]		= { .type = NLA_UNSPEC, },
	[TEAM_ATTR_OPTION_NAME] = {
		.type = NLA_STRING,
		.len = TEAM_STRING_MAX_LEN,
	},
	[TEAM_ATTR_OPTION_CHANGED]		= { .type = NLA_FLAG },
	[TEAM_ATTR_OPTION_TYPE]			= { .type = NLA_U8 },
	[TEAM_ATTR_OPTION_DATA] = {
		.type = NLA_BINARY,
#ifndef	N2OS
		.len = TEAM_STRING_MAX_LEN,
#endif
	},
};

static int team_nl_cmd_noop(struct sk_buff *skb, struct genl_info *info)
{
	struct sk_buff *msg;
	void *hdr;
	int err;

	PRINTK ("%s:\n", __FUNCTION__);
	msg = nlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!msg)
		return -ENOMEM;

	hdr = genlmsg_put(msg, info->snd_pid, info->snd_seq,
			  &team_nl_family, 0, TEAM_CMD_NOOP);
	if (IS_ERR(hdr)) {
		err = PTR_ERR(hdr);
		goto err_msg_put;
	}

	genlmsg_end(msg, hdr);

	PRINTK ("%s: reply\n", __FUNCTION__);
	return genlmsg_unicast(genl_info_net(info), msg, info->snd_pid);

err_msg_put:
	nlmsg_free(msg);

	return err;
}

/*
 * Netlink cmd functions should be locked by following two functions.
 * Since dev gets held here, that ensures dev won't disappear in between.
 */
static struct team *team_nl_team_get(struct genl_info *info)
{
	struct net *net = genl_info_net(info);
	int ifindex;
	struct net_device *dev;
	struct team *team;

	PRINTK ("%s: \n", __FUNCTION__);
	if (!info->attrs[TEAM_ATTR_TEAM_IFINDEX])
		return NULL;

	ifindex = nla_get_u32(info->attrs[TEAM_ATTR_TEAM_IFINDEX]);
	dev = dev_get_by_index(net, ifindex);
	if (!dev || dev->netdev_ops != &team_netdev_ops) {
		if (dev)
			dev_put(dev);
		return NULL;
	}

	team = netdev_priv(dev);
	PRINTK ("%s: %s\n", __FUNCTION__, team->dev->name);
	mutex_lock(&team->lock);
	return team;
}

static void team_nl_team_put(struct team *team)
{
	mutex_unlock(&team->lock);
	PRINTK ("%s: %s\n", __FUNCTION__, team->dev->name);
	dev_put(team->dev);
}

static int team_nl_send_generic(struct genl_info *info, struct team *team,
				int (*fill_func)(struct sk_buff *skb,
						 struct genl_info *info,
						 int flags, struct team *team))
{
	struct sk_buff *skb;
	int err;

	PRINTK ("%s: %s\n", __FUNCTION__, team->dev->name);
	skb = nlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;

	err = fill_func(skb, info, NLM_F_ACK, team);
	if (err < 0)
		goto err_fill;

	err = genlmsg_unicast(genl_info_net(info), skb, info->snd_pid);
	return err;

err_fill:
	nlmsg_free(skb);
	return err;
}

static int team_nl_fill_options_get(struct sk_buff *skb,
				    u32 pid, u32 seq, int flags,
				    struct team *team, bool fillall)
{
	struct nlattr *option_list;
	void *hdr;
	struct team_option *option;

	PRINTK ("%s: %s\n", __FUNCTION__, team->dev->name);
	hdr = genlmsg_put(skb, pid, seq, &team_nl_family, flags,
			  TEAM_CMD_OPTIONS_GET);
	if (IS_ERR(hdr))
		return PTR_ERR(hdr);

	NLA_PUT_U32(skb, TEAM_ATTR_TEAM_IFINDEX, team->dev->ifindex);
	option_list = nla_nest_start(skb, TEAM_ATTR_LIST_OPTION);
	if (!option_list)
		return -EMSGSIZE;

	list_for_each_entry(option, &team->option_list, list) {
		struct nlattr *option_item;
		long arg;
#ifdef	N2OS
		struct team_gsetter_ctx ctx;
#endif

		/* Include only changed options if fill all mode is not on */
		if (!fillall && !option->changed)
			continue;
		option_item = nla_nest_start(skb, TEAM_ATTR_ITEM_OPTION);
		if (!option_item)
			goto nla_put_failure;
		NLA_PUT_STRING(skb, TEAM_ATTR_OPTION_NAME, option->name);
		if (option->changed) {
			NLA_PUT_FLAG(skb, TEAM_ATTR_OPTION_CHANGED);
			option->changed = false;
		}
		if (option->removed)
			NLA_PUT_FLAG(skb, TEAM_ATTR_OPTION_REMOVED);
		PRINTK ("%s: %s type=0x%x name=%s\n", __FUNCTION__, team->dev->name, option->type, option->name);

		switch (option->type) {
		case TEAM_OPTION_TYPE_U32:
			NLA_PUT_U8(skb, TEAM_ATTR_OPTION_TYPE, NLA_U32);
			team_option_get(team, option, &arg);
			NLA_PUT_U32(skb, TEAM_ATTR_OPTION_DATA, arg);
			break;
		case TEAM_OPTION_TYPE_STRING:
			NLA_PUT_U8(skb, TEAM_ATTR_OPTION_TYPE, NLA_STRING);
			team_option_get(team, option, &arg);
			NLA_PUT_STRING(skb, TEAM_ATTR_OPTION_DATA,
				       (char *) arg);
			break;
#ifdef	N2OS
		case TEAM_OPTION_TYPE_BINARY:
			{
				NLA_PUT_U8(skb, TEAM_ATTR_OPTION_TYPE, NLA_BINARY);
				team_option_get(team, option, &ctx);
				NLA_PUT(skb, TEAM_ATTR_OPTION_DATA,
					ctx.data.bin_val.len,
					ctx.data.bin_val.ptr);
			}
			break;
		case TEAM_OPTION_TYPE_BOOL:
			{
				NLA_PUT_U8(skb, TEAM_ATTR_OPTION_TYPE, NLA_FLAG);
				team_option_get(team, option, &arg);
				if (arg == true)
					NLA_PUT_FLAG(skb, TEAM_ATTR_OPTION_DATA);
			}
			break;
		case TEAM_OPTION_TYPE_S32:
			{
				NLA_PUT_U8(skb, TEAM_ATTR_OPTION_TYPE, NLA_S32);
				team_option_get(team, option, &arg);
				NLA_PUT_U32(skb, TEAM_ATTR_OPTION_DATA,(int)arg);
			}
			break;
#endif
		default:
			PRINTK ("%s: %s type=0x%x BUG?\n", __FUNCTION__, team->dev->name, option->type);
			BUG();
		}
		nla_nest_end(skb, option_item);
	}

	nla_nest_end(skb, option_list);
	PRINTK ("%s: %s end\n", __FUNCTION__, team->dev->name);
	return genlmsg_end(skb, hdr);

nla_put_failure:
	genlmsg_cancel(skb, hdr);
	return -EMSGSIZE;
}

static int team_nl_fill_options_get_all(struct sk_buff *skb,
					struct genl_info *info, int flags,
					struct team *team)
{
	PRINTK ("%s: %s\n", __FUNCTION__, team->dev->name);
	return team_nl_fill_options_get(skb, info->snd_pid,
					info->snd_seq, NLM_F_ACK,
					team, true);
}

static int team_nl_cmd_options_get(struct sk_buff *skb, struct genl_info *info)
{
	struct team *team;
	int err;

	PRINTK ("%s:\n", __FUNCTION__);
	team = team_nl_team_get(info);
	if (!team)
		return -EINVAL;

	PRINTK ("%s: %s\n", __FUNCTION__, team->dev->name);
	err = team_nl_send_generic(info, team, team_nl_fill_options_get_all);

	team_nl_team_put(team);

	PRINTK ("%s: end\n", __FUNCTION__);
	return err;
}

static int team_nl_cmd_options_set(struct sk_buff *skb, struct genl_info *info)
{
	struct team *team;
	int err = 0;
	int i;
	struct nlattr *nl_option;

	PRINTK ("%s: \n", __FUNCTION__);
	team = team_nl_team_get(info);
	if (!team)
		return -EINVAL;

	PRINTK ("%s: %s\n", __FUNCTION__, team->dev->name);
	err = -EINVAL;
	if (!info->attrs[TEAM_ATTR_LIST_OPTION]) {
		err = -EINVAL;
		PRINTK ("%s: %s 1\n", __FUNCTION__, team->dev->name);
		goto team_put;
	}

	nla_for_each_nested(nl_option, info->attrs[TEAM_ATTR_LIST_OPTION], i) {
		struct nlattr *mode_attrs[TEAM_ATTR_OPTION_MAX + 1];
		enum team_option_type opt_type;
		struct team_option *option;
		char *opt_name;
		bool opt_found = false;

		if (nla_type(nl_option) != TEAM_ATTR_ITEM_OPTION) {
			PRINTK ("%s: %s 2\n", __FUNCTION__, team->dev->name);
			err = -EINVAL;
			goto team_put;
		}
		err = nla_parse_nested(mode_attrs, TEAM_ATTR_OPTION_MAX,
				       nl_option, team_nl_option_policy);
		if (err) {
			PRINTK ("%s: %s 3 error=%d\n", __FUNCTION__, team->dev->name, err);
			goto team_put;
		}
		if (!mode_attrs[TEAM_ATTR_OPTION_NAME] ||
		    !mode_attrs[TEAM_ATTR_OPTION_TYPE] ||
		    !mode_attrs[TEAM_ATTR_OPTION_DATA]) {
			PRINTK ("%s: %s 4\n", __FUNCTION__, team->dev->name);
			err = -EINVAL;
			goto team_put;
		}
		switch (nla_get_u8(mode_attrs[TEAM_ATTR_OPTION_TYPE])) {
		case NLA_U32:
			opt_type = TEAM_OPTION_TYPE_U32;
			break;
		case NLA_STRING:
			opt_type = TEAM_OPTION_TYPE_STRING;
			break;
#ifdef	N2OS
		case NLA_BINARY:
			opt_type = TEAM_OPTION_TYPE_BINARY;
			break;
		case NLA_FLAG:
			opt_type = TEAM_OPTION_TYPE_BOOL;
			break;
		case NLA_S32:
			opt_type = TEAM_OPTION_TYPE_S32;
			break;
#endif
		default:
			PRINTK ("%s: %s 5\n", __FUNCTION__, team->dev->name);
			goto team_put;
		}

		opt_name = nla_data(mode_attrs[TEAM_ATTR_OPTION_NAME]);
		list_for_each_entry(option, &team->option_list, list) {
			long arg;
			struct nlattr *opt_data_attr;

			if (option->type != opt_type ||
			    strcmp(option->name, opt_name))
				continue;
			opt_found = true;
			opt_data_attr = mode_attrs[TEAM_ATTR_OPTION_DATA];
			PRINTK ("%s: %s type=0x%x\n", __FUNCTION__, team->dev->name, opt_type);
			switch (opt_type) {
			case TEAM_OPTION_TYPE_U32:
				arg = nla_get_u32(opt_data_attr);
				break;
			case TEAM_OPTION_TYPE_STRING:
				arg = (long) nla_data(opt_data_attr);
				break;
#ifdef	N2OS
			case TEAM_OPTION_TYPE_BINARY:
				{
					struct team_gsetter_ctx ctx;
					ctx.data.bin_val.len = nla_len(opt_data_attr);
					ctx.data.bin_val.ptr = nla_data(opt_data_attr);
					arg = (long) &ctx;
				}
				break;
			case TEAM_OPTION_TYPE_BOOL:
				arg = opt_data_attr ? true : false;
				break;
			case TEAM_OPTION_TYPE_S32:
				arg = nla_get_u32(opt_data_attr);
				break;
#endif
			default:
				PRINTK ("%s: %s type=0x%x BUG?\n", __FUNCTION__, team->dev->name, opt_type);
				BUG();
			}
			err = team_option_set(team, option, &arg);
			if (err)
				goto team_put;
		}
		if (!opt_found) {
			PRINTK ("%s: %s 6\n", __FUNCTION__, team->dev->name);
			err = -ENOENT;
			goto team_put;
		}
	}

team_put:
	team_nl_team_put(team);

	PRINTK ("%s: end\n", __FUNCTION__);
	return err;
}

static int team_nl_fill_port_list_get(struct sk_buff *skb,
				      u32 pid, u32 seq, int flags,
				      struct team *team,
				      bool fillall)
{
	struct nlattr *port_list;
	void *hdr;
	struct team_port *port;

	PRINTK ("%s: %s\n", __FUNCTION__, team->dev->name);
	hdr = genlmsg_put(skb, pid, seq, &team_nl_family, flags,
			  TEAM_CMD_PORT_LIST_GET);
	if (IS_ERR(hdr))
		return PTR_ERR(hdr);

	NLA_PUT_U32(skb, TEAM_ATTR_TEAM_IFINDEX, team->dev->ifindex);
	port_list = nla_nest_start(skb, TEAM_ATTR_LIST_PORT);
	if (!port_list)
		return -EMSGSIZE;

	list_for_each_entry(port, &team->port_list, list) {
		struct nlattr *port_item;

		/* Include only changed ports if fill all mode is not on */
		if (!fillall && !port->changed)
			continue;
		port_item = nla_nest_start(skb, TEAM_ATTR_ITEM_PORT);
		if (!port_item)
			goto nla_put_failure;
		NLA_PUT_U32(skb, TEAM_ATTR_PORT_IFINDEX, port->dev->ifindex);
		if (port->changed) {
			NLA_PUT_FLAG(skb, TEAM_ATTR_PORT_CHANGED);
			port->changed = false;
		}
		if (port->removed)
			NLA_PUT_FLAG(skb, TEAM_ATTR_PORT_REMOVED);
		if (port->linkup)
			NLA_PUT_FLAG(skb, TEAM_ATTR_PORT_LINKUP);
		NLA_PUT_U32(skb, TEAM_ATTR_PORT_SPEED, port->speed);
		NLA_PUT_U8(skb, TEAM_ATTR_PORT_DUPLEX, port->duplex);
		nla_nest_end(skb, port_item);
	}

	nla_nest_end(skb, port_list);
	return genlmsg_end(skb, hdr);

nla_put_failure:
	genlmsg_cancel(skb, hdr);
	return -EMSGSIZE;
}

static int team_nl_fill_port_list_get_all(struct sk_buff *skb,
					  struct genl_info *info, int flags,
					  struct team *team)
{
	PRINTK ("%s: %s\n", __FUNCTION__, team->dev->name);
	return team_nl_fill_port_list_get(skb, info->snd_pid,
					  info->snd_seq, NLM_F_ACK,
					  team, true);
}

static int team_nl_cmd_port_list_get(struct sk_buff *skb,
				     struct genl_info *info)
{
	struct team *team;
	int err;

	PRINTK ("%s: \n", __FUNCTION__);
	team = team_nl_team_get(info);
	if (!team)
		return -EINVAL;

	PRINTK ("%s: %s\n", __FUNCTION__, team->dev->name);
	err = team_nl_send_generic(info, team, team_nl_fill_port_list_get_all);

	team_nl_team_put(team);

	PRINTK ("%s: end\n", __FUNCTION__);
	return err;
}

static struct genl_ops team_nl_ops[] = {
	{
		.cmd = TEAM_CMD_NOOP,
		.doit = team_nl_cmd_noop,
		.policy = team_nl_policy,
	},
	{
		.cmd = TEAM_CMD_OPTIONS_SET,
		.doit = team_nl_cmd_options_set,
		.policy = team_nl_policy,
		.flags = GENL_ADMIN_PERM,
	},
	{
		.cmd = TEAM_CMD_OPTIONS_GET,
		.doit = team_nl_cmd_options_get,
		.policy = team_nl_policy,
		.flags = GENL_ADMIN_PERM,
	},
	{
		.cmd = TEAM_CMD_PORT_LIST_GET,
		.doit = team_nl_cmd_port_list_get,
		.policy = team_nl_policy,
		.flags = GENL_ADMIN_PERM,
	},
};

static struct genl_multicast_group team_change_event_mcgrp = {
	.name = TEAM_GENL_CHANGE_EVENT_MC_GRP_NAME,
};

static int team_nl_send_event_options_get(struct team *team)
{
	struct sk_buff *skb;
	int err;
	struct net *net = dev_net(team->dev);

	PRINTK ("%s: %s\n", __FUNCTION__, team->dev->name);
	skb = nlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;

	err = team_nl_fill_options_get(skb, 0, 0, 0, team, false);
	if (err < 0)
		goto err_fill;

	PRINTK ("%s: %s 1\n", __FUNCTION__, team->dev->name);

	err = genlmsg_multicast_netns(net, skb, 0, team_change_event_mcgrp.id,
				      GFP_KERNEL);
	if (err)
		PRINTK ("%s: %s error=%d\n", __FUNCTION__, team->dev->name, err);
	return err;

err_fill:
	nlmsg_free(skb);
	return err;
}

static int team_nl_send_event_port_list_get(struct team *team)
{
	struct sk_buff *skb;
	int err;
	struct net *net = dev_net(team->dev);

	PRINTK ("%s: %s\n", __FUNCTION__, team->dev->name);
	skb = nlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;

	err = team_nl_fill_port_list_get(skb, 0, 0, 0, team, false);
	if (err < 0) {
		PRINTK ("%s: %s error1=%d\n", __FUNCTION__, team->dev->name, err);
		goto err_fill;
	}

	err = genlmsg_multicast_netns(net, skb, 0, team_change_event_mcgrp.id,
				      GFP_KERNEL);
	if (err)
		PRINTK ("%s: %s error2=%d\n", __FUNCTION__, team->dev->name, err);
	return err;

err_fill:
	nlmsg_free(skb);
	return err;
}

static int team_nl_init(void)
{
	int err;

	PRINTK ("%s:\n", __FUNCTION__);
	err = genl_register_family_with_ops(&team_nl_family, team_nl_ops,
					    ARRAY_SIZE(team_nl_ops));
	if (err) {
		PRINTK ("%s: error1=%d\n", __FUNCTION__, err);
		return err;
	}

	err = genl_register_mc_group(&team_nl_family, &team_change_event_mcgrp);
	if (err) {
		PRINTK ("%s: error2=%d\n", __FUNCTION__, err);
		goto err_change_event_grp_reg;
	}

	return 0;

err_change_event_grp_reg:
	genl_unregister_family(&team_nl_family);

	return err;
}

static void team_nl_fini(void)
{
	PRINTK ("%s:\n", __FUNCTION__);
	genl_unregister_family(&team_nl_family);
}


/******************
 * Change checkers
 ******************/

static void __team_options_change_check(struct team *team)
{
	int err;

	PRINTK ("%s: %s\n", __FUNCTION__, team->dev->name);
	err = team_nl_send_event_options_get(team);
	if (err)
		netdev_warn(team->dev, "Failed to send options change via netlink\n");
}

/* rtnl lock is held */
static void __team_port_change_check(struct team_port *port, bool linkup)
{
	int err;

	PRINTK ("%s: %s\n", __FUNCTION__, port->dev->name);
	if (!port->removed && port->linkup == linkup)
		return;

	port->changed = true;
	port->linkup = linkup;
	if (linkup) {
		struct ethtool_cmd ecmd;

		err = __ethtool_get_settings(port->dev, &ecmd);
		if (!err) {
			port->speed = ethtool_cmd_speed(&ecmd);
			port->duplex = ecmd.duplex;
			goto send_event;
		}
	}
	port->speed = 0;
	port->duplex = 0;

send_event:
	err = team_nl_send_event_port_list_get(port->team);
	if (err)
		netdev_warn(port->team->dev, "Failed to send port change of device %s via netlink\n",
			    port->dev->name);
}

static void team_port_change_check(struct team_port *port, bool linkup)
{
	struct team *team = port->team;

	mutex_lock(&team->lock);
	__team_port_change_check(port, linkup);
	mutex_unlock(&team->lock);
}

/************************************
 * Net device notifier event handler
 ************************************/

static int team_device_event(struct notifier_block *unused,
			     unsigned long event, void *ptr)
{
	struct net_device *dev = (struct net_device *) ptr;
	struct team_port *port;

	PRINTK ("%s: %s event 0x%lx\n", __FUNCTION__, dev->name, event);
#ifdef	N2OS
	port = team_port_local_get(dev);
#else
	port = team_port_get_rtnl(dev);
#endif
	if (!port)
		return NOTIFY_DONE;

	switch (event) {
	case NETDEV_UP:
		if (netif_carrier_ok(dev))
			team_port_change_check(port, true);
	case NETDEV_DOWN:
		team_port_change_check(port, false);
	case NETDEV_CHANGE:
		if (netif_running(port->dev))
			team_port_change_check(port,
					       !!netif_carrier_ok(port->dev));
		break;
	case NETDEV_UNREGISTER:
		team_del_slave(port->team->dev, dev);
		break;
	case NETDEV_FEAT_CHANGE:
		team_compute_features(port->team);
		break;
	case NETDEV_CHANGEMTU:
		/* Forbid to change mtu of underlaying device */
		return NOTIFY_BAD;
#if	LINUX_VERSION_CODE == KERNEL_VERSION (2,6,35)
	case NETDEV_PRE_TYPE_CHANGE:
		/* Forbid to change type of underlaying device */
		return NOTIFY_BAD;
#endif
	}
	PRINTK ("%s: %s event end\n", __FUNCTION__, dev->name);
	return NOTIFY_DONE;
}

static struct notifier_block team_notifier_block __read_mostly = {
	.notifier_call = team_device_event,
};


/***********************
 * Module init and exit
 ***********************/

#if 0	//FOR TEST
extern long netlink_broadcast_test_bits;
#endif

static int __init team_module_init(void)
{
	int err;

	PRINTK ("%s:\n", __FUNCTION__);

	register_netdevice_notifier(&team_notifier_block);
	err = rtnl_link_register(&team_link_ops);
	if (err)
		goto err_rtnl_reg;

	err = team_nl_init();
	if (err)
		goto err_nl_init;

#ifdef	N2OS//_PLUS_IP
	dev_add_pack(&team_packet_type);
#endif
#if 0	//FOR TEST
	netlink_broadcast_test_bits = 1;
#endif
	return 0;

err_nl_init:
	rtnl_link_unregister(&team_link_ops);

err_rtnl_reg:
	unregister_netdevice_notifier(&team_notifier_block);

	return err;
}

static void __exit team_module_exit(void)
{
	PRINTK ("%s:\n", __FUNCTION__);
#if 0	//FOR TEST
	netlink_broadcast_test_bits = 0;
#endif
#ifdef	N2OS//_PLUS_IP
	dev_remove_pack(&team_packet_type);
#endif
	team_nl_fini();
	rtnl_link_unregister(&team_link_ops);
	unregister_netdevice_notifier(&team_notifier_block);
}

module_init(team_module_init);
module_exit(team_module_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Jiri Pirko <jpirko@redhat.com>, modified by N2OS");
MODULE_DESCRIPTION("Ethernet team device driver for N2OS");
MODULE_ALIAS_RTNL_LINK(DRV_NAME "-N2OS-ioctl");
