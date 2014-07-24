/*
 * drivers/net/team/team_mode_loadbalance.c - Load-balancing mode for team
 * Copyright (c) 2012 Jiri Pirko <jpirko@redhat.com>
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
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/filter.h>
#ifdef	N2OS
#include <linux/rtnetlink.h>
#include <net/sock.h>
#include "include/linux/compat.h"
#include "include/linux/u64_stats_sync.h"
#include "include/linux/if_team.h"
#else
#include <linux/if_team.h>
#endif

int	debug = 0;
module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "Set debug options(default:0)");

struct lb_priv {
	struct sk_filter __rcu *fp;
	struct sock_fprog *orig_fprog;
};

static struct lb_priv *lb_priv(struct team *team)
{
	return (struct lb_priv *) &team->mode_priv;
}

static bool lb_transmit(struct team *team, struct sk_buff *skb)
{
#ifndef	N2OS
	struct sk_filter *fp;
#endif
	struct team_port *port;
	unsigned int hash;
	int port_index;

#ifdef	N2OS
	int port_count;
	int port_index_real = 0;
	int port_enabled_count = 0;

	u16 *d = (u16 *) &skb->data[ETH_ALEN];
	hash = *d ^ *(d+1) ^ *(d+2);

	rcu_read_lock();
	list_for_each_entry_rcu(port, &team->port_list, list) {
		port_index_real++;
		if (port->enabled)
			port_enabled_count++;
	}

	if (port_enabled_count == 0) {
		rcu_read_unlock();
		netdev_warn(team->dev, "Failed to select a path of device %s: enabled-device not found\n", team->dev->name);
		goto drop;
	}
	port_index = hash % port_enabled_count;

	port_index_real = 0;
	port_count = 0;
	list_for_each_entry_rcu(port, &team->port_list, list) {
		if (port->enabled) {
			if (port_count == port_index) {
				port_index = port_index_real;
				break;
			}
			port_count++;
		}
		port_index_real++;
	}
	rcu_read_unlock();
#else
	fp = rcu_dereference(lb_priv(team)->fp);
	if (unlikely(!fp))
		goto drop;
	hash = SK_RUN_FILTER(fp, skb);
	port_index = hash % team->en_port_count;
#endif

	port = team_get_port_by_index_rcu(team, port_index);
	if (unlikely(!port)) {
		netdev_warn(team->dev, "Failed to select the path of device %s: %s not found\n", team->dev->name, port->dev->name);
		goto drop;
	}
	skb->dev = port->dev;

	PRINTK ("%s: %s@%s\n", __FUNCTION__, skb->dev->name, team->dev->name);

	dev_queue_xmit(skb);
	return true;

drop:
	dev_kfree_skb_any(skb);
	return false;
}

#ifdef	N2OS	//ref: kernel-3.13.6
static inline unsigned int sk_filter_size(unsigned int proglen)
{
    return max(sizeof(struct sk_filter),
           offsetof(struct sk_filter, insns[proglen]));
}

/**
 *	sk_unattached_filter_create - create an unattached filter
 *	@fprog: the filter program
 *	@pfp: the unattached filter that is created
 *
 * Create a filter independent of any socket. We first run some
 * sanity checks on it to make sure it does not explode on us later.
 * If an error occurs or there is insufficient memory for the filter
 * a negative errno code is returned. On success the return is zero.
 */
int sk_unattached_filter_create(struct sk_filter **pfp,
				struct sock_fprog *fprog)
{
	struct sk_filter *fp;
	unsigned int fsize = sizeof(struct sock_filter) * fprog->len;
	int err;

	/* Make sure new filter is there and in the right amounts. */
	if (fprog->filter == NULL)
		return -EINVAL;

	fp = kmalloc(sk_filter_size(fprog->len), GFP_KERNEL);
	if (!fp)
		return -ENOMEM;
	memcpy(fp->insns, fprog->filter, fsize);

	atomic_set(&fp->refcnt, 1);
	fp->len = fprog->len;

	err = sk_chk_filter(fp->insns, fp->len);
	if (err)
		goto free_mem;

	*pfp = fp;
	return 0;

free_mem:
	printk ("%s: err\n", __FUNCTION__);
	kfree(fp);
	return err;
}
EXPORT_SYMBOL_GPL(sk_unattached_filter_create);

void sk_unattached_filter_destroy(struct sk_filter *fp)
{
	sk_filter_release(fp);
}
EXPORT_SYMBOL_GPL(sk_unattached_filter_destroy);
#endif

#ifdef	N2OS
static int lb_bpf_func_get(struct team *team, void *arg)
#else
static int lb_bpf_func_get(struct team *team, struct team_gsetter_ctx *ctx)
#endif
{
#ifdef	N2OS
	struct team_gsetter_ctx *ctx = (struct team_gsetter_ctx *) arg;
#endif
	if (!lb_priv(team)->orig_fprog) {
		ctx->data.bin_val.len = 0;
		ctx->data.bin_val.ptr = NULL;
		PRINTK ("%s: %s l=0 p=0\n", __FUNCTION__, team->dev->name);
		return 0;
	}
	ctx->data.bin_val.len = lb_priv(team)->orig_fprog->len *
				sizeof(struct sock_filter);
	ctx->data.bin_val.ptr = lb_priv(team)->orig_fprog->filter;
	PRINTK ("%s: %s l=%u p=0x%p\n", __FUNCTION__, team->dev->name, ctx->data.bin_val.len, ctx->data.bin_val.ptr);
	return 0;
}

static int __fprog_create(struct sock_fprog **pfprog, u32 data_len,
			  const void *data)
{
	struct sock_fprog *fprog;
	struct sock_filter *filter = (struct sock_filter *) data;

	if (data_len % sizeof(struct sock_filter))
		return -EINVAL;
	fprog = kmalloc(sizeof(struct sock_fprog), GFP_KERNEL);
	if (!fprog)
		return -ENOMEM;
	fprog->filter = kmemdup(filter, data_len, GFP_KERNEL);
	if (!fprog->filter) {
		kfree(fprog);
		return -ENOMEM;
	}
	fprog->len = data_len / sizeof(struct sock_filter);
	*pfprog = fprog;
	return 0;
}

static void __fprog_destroy(struct sock_fprog *fprog)
{
	kfree(fprog->filter);
	kfree(fprog);
}

#ifdef	N2OS
static int lb_bpf_func_set(struct team *team, void *arg)
#else
static int lb_bpf_func_set(struct team *team, struct team_gsetter_ctx *ctx)
#endif
{
	struct sk_filter *fp = NULL;
	struct sock_fprog *fprog = NULL;
	int err;
#ifdef	N2OS
	struct team_gsetter_ctx *ctx =
		(struct team_gsetter_ctx *) (*(long*)arg);
#endif

	PRINTK ("%s: %s l=%u p=0x%p\n", __FUNCTION__, team->dev->name, ctx->data.bin_val.len, ctx->data.bin_val.ptr);
	if (ctx->data.bin_val.len) {
		err = __fprog_create(&fprog, ctx->data.bin_val.len,
				     ctx->data.bin_val.ptr);
		if (err)
			return err;
		err = sk_unattached_filter_create(&fp, fprog);
		if (err) {
			__fprog_destroy(fprog);
			return err;
		}
	}

	if (lb_priv(team)->orig_fprog) {
		/* Clear old filter data */
		__fprog_destroy(lb_priv(team)->orig_fprog);
		sk_unattached_filter_destroy(lb_priv(team)->fp);
	}

	rcu_assign_pointer(lb_priv(team)->fp, fp);
	lb_priv(team)->orig_fprog = fprog;
	return 0;
}

static const struct team_option lb_options[] = {
	{
		.name = "bpf_hash_func",
		.type = TEAM_OPTION_TYPE_BINARY,
		.getter = lb_bpf_func_get,
		.setter = lb_bpf_func_set,
	},
};

static int lb_init(struct team *team)
{
	PRINTK ("%s: %s\n", __FUNCTION__, team->dev->name);
	return team_options_register(team, lb_options,
				     ARRAY_SIZE(lb_options));
}

static void lb_exit(struct team *team)
{
	PRINTK ("%s: %s\n", __FUNCTION__, team->dev->name);
	team_options_unregister(team, lb_options,
				ARRAY_SIZE(lb_options));
}

static const struct team_mode_ops lb_mode_ops = {
	.init			= lb_init,
	.exit			= lb_exit,
	.transmit		= lb_transmit,
};

static struct team_mode lb_mode = {
	.kind		= "loadbalance",
	.owner		= THIS_MODULE,
	.priv_size	= sizeof(struct lb_priv),
	.ops		= &lb_mode_ops,
};

static int __init lb_init_module(void)
{
	return team_mode_register(&lb_mode);
}

static void __exit lb_cleanup_module(void)
{
	team_mode_unregister(&lb_mode);
}

module_init(lb_init_module);
module_exit(lb_cleanup_module);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Jiri Pirko <jpirko@redhat.com>, modified by N2OS");
MODULE_DESCRIPTION("Load-balancing mode for team by N2OS");
MODULE_ALIAS("team-mode-loadbalance");
