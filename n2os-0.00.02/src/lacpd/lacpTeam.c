/*
 *   teamd.c - Network team device daemon
 *   Copyright (C) 2011-2013 Jiri Pirko <jiri@resnulli.us>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "nnTypes.h"
#include "nnStr.h"

#include "nnCmdLink.h"
#include "nnCmdCommon.h"
#include "nnCmdCmsh.h"
#include "nnCmdNode.h"
#include "nnCmdMsg.h"
#include "nnCmdInstall.h"
#include "nosLib.h"
#include "nnCmdDefines.h"
#include "nnUtility.h"
#include "nnList.h"

#include "nnBuffer.h"
#include "nnPrefix.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <linux/netdevice.h>
#include <sys/syslog.h>
#include <private/list.h>
#include <private/misc.h>
#include <team.h>

#include "config.h"
#include "teamd.h"
#include "teamd_config.h"

#include "teamd_state.h"
#include "teamd_phys_port_check.h"

#include "lacpDef.h"

/*
 * Description :
 */
static void lacpDevTimerEvent (Int32T fd, Int16T event, void *loopCallback)
{ 
	DEBUGPRINT("%s %p\n", __FUNCTION__, loopCallback);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	lacpDevEventCallback (fd, loopCallback);
} 

#if	1
void *lacpDevTimerRegister (void *loopCallback, struct timeval *tv)
{
	void *timerTask;

	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	timerTask = taskTimerSet(lacpDevTimerEvent, *tv, TASK_PERSIST, loopCallback);
	DEBUGPRINT("%s ev=%p %p (tv=%ld.%06ld)\n", __FUNCTION__, timerTask, loopCallback, tv->tv_sec, tv->tv_usec);
	return timerTask;
}
#else
void *lacpDevTimerRegister (void *loopCallback, struct timeval *tv, Int32T persist)
{
	void *timerTask;

	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	timerTask = taskTimerSet(lacpDevTimerEvent, *tv, persist ? TASK_PERSIST : 0, loopCallback);
	DEBUGPRINT("%s ev=%p %p/%d (tv=%ld.%06ld)\n", __FUNCTION__, timerTask, loopCallback, persist, tv->tv_sec, tv->tv_usec);
	return timerTask;
}
#endif

void *lacpDevTimerUpdate (void *loopCallback, struct timeval *tv, void *timerTask)
{
	void *timerNewTask;
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	timerNewTask = taskTimerUpdate (lacpDevTimerEvent, timerTask, *tv, loopCallback);
	DEBUGPRINT("%s ev=%p->%p %p (tv=%ld.%06ld)\n", __FUNCTION__, timerTask, timerNewTask, loopCallback, tv->tv_sec, tv->tv_usec);
	return timerNewTask;
}

void lacpDevTimerUnregister (void *timerTask)
{
	DEBUGPRINT("%s ev=%p\n", __FUNCTION__, timerTask);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	taskTimerDel(timerTask);
}

static void lacpDevFdEvent (Int32T fd, Int16T event, void *loopCallback)
{
	DEBUGPRINT("%s %p\n", __FUNCTION__, loopCallback);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	lacpDevEventCallback (fd, loopCallback);
}

void *lacpDevFdRegister (Int32T fd, Int32T flag, void *loopCallback)
{
	void *fdTask;

	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	fdTask = taskFdSet (lacpDevFdEvent, fd, flag, TASK_PRI_MIDDLE, loopCallback);
	DEBUGPRINT("%s ev=%p %p/%x\n", __FUNCTION__, fdTask, loopCallback, flag);
	return fdTask;
}

void *lacpDevFdUpdate (void *loopCallback, void *fdTask)
{
	void *fdNewTask;

	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	fdNewTask = taskFdUpdate (lacpDevFdEvent, fdTask, loopCallback);
	DEBUGPRINT("%s ev=%p->%p %p\n", __FUNCTION__, fdTask, fdNewTask, loopCallback);
	return fdNewTask;
}

void lacpDevFdUnregister (void *fdTask)
{
	DEBUGPRINT("%s ev=%p\n", __FUNCTION__, fdTask);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	taskFdDel(fdTask);
}

static const struct teamd_runner *teamd_runner_list[] = {
	&teamd_runner_lacp,
};

#define TEAMD_RUNNER_LIST_SIZE ARRAY_SIZE(teamd_runner_list)

static const struct teamd_runner *teamdFindRunner(const char * runnerName)
{
	Int32T i;

	for (i = 0; i < TEAMD_RUNNER_LIST_SIZE; i++) {
		if (strcmp(teamd_runner_list[i]->name, runnerName) == 0)
			return teamd_runner_list[i];
	}
	return NULL;
}

#define TEAMD_DEFAULT_RUNNER_NAME "lacp"
#define TEAMD_DEFAULT_DEVNAME_PREFIX "team"

static Int32T teamdFlushPorts(struct teamd_context *ctx)
{
	struct teamd_port *tdPort;
	teamd_for_each_tdport(tdPort, ctx) {
		teamd_port_remove(ctx, tdPort->ifname);
	}
	return 0;
}

struct teamd_loop_callback {
	struct list_item list;
	char * name;
	void *priv;
	teamd_loop_callback_func_t func;
	Int32T fd;
	Int32T fdEvent;
	bool isPeriod;
	bool enabled;
	void *registerEvent;
	void *ctx;
	struct timeval tv;
	Int32T persist;
};

static struct teamd_loop_callback *__GetLcb(struct teamd_context *ctx,
					     const char * cbName, void *priv,
					     struct teamd_loop_callback *last)
{
	struct teamd_loop_callback *lcb;
	bool lastFound;

	lastFound = last == NULL ? true: false;
	list_for_each_node_entry(lcb, &ctx->run_loop.callback_list, list)
	{
		if (!lastFound) {
			if (lcb == last)
				lastFound = true;
			continue;
		}
		if (cbName && strcmp(lcb->name, cbName))
			continue;
		if (priv && lcb->priv != priv)
			continue;
		return lcb;
	}
	return NULL;
}

static struct teamd_loop_callback *GetLcb(struct teamd_context *ctx,
					   const char * cbName, void *priv)
{
	return __GetLcb(ctx, cbName, priv, NULL);
}

static struct teamd_loop_callback *getLcbMulti(struct teamd_context *ctx,
						 const char * cbName,
						 void *priv,
						 struct teamd_loop_callback *last)
{
	return __GetLcb(ctx, cbName, priv, last);
}

#define ForEachLcbMultiMatch(lcb, ctx, cbName, priv)		\
	for (lcb = getLcbMulti(ctx, cbName, priv, NULL); lcb;	\
	     lcb = getLcbMulti(ctx, cbName, priv, lcb))

#define ForEachLcbMultiMatchSafe(lcb, tmp, ctx, cbName, priv)	\
	for (lcb = getLcbMulti(ctx, cbName, priv, NULL),		\
	     tmp = getLcbMulti(ctx, cbName, priv, lcb);		\
	     lcb;							\
	     lcb = tmp,							\
	     tmp = getLcbMulti(ctx, cbName, priv, lcb))

static Int32T __teamdLoopCallbackFdAdd(struct teamd_context *ctx,
					const char * cbName, void *priv,
					teamd_loop_callback_func_t func,
					Int32T fd, Int32T fdEvent, bool tail)
{
	Int32T err;
	struct teamd_loop_callback *lcb;

	if (!cbName || !priv)
		return -EINVAL;
	if (GetLcb(ctx, cbName, priv)) {
		teamd_log_err("Callback named \"%s\" is already registered.",
			      cbName);
		return -EEXIST;
	}
	lcb = myzalloc(sizeof(*lcb));
	if (!lcb) {
		teamd_log_err("Failed alloc memory for callback.");
		return -ENOMEM;
	}
	lcb->name = strdup(cbName);
	if (!lcb->name) {
		err = -ENOMEM;
		goto lcb_free;
	}
	lcb->priv = priv;
	lcb->func = func;
	lcb->fd = fd;
	lcb->registerEvent = NULL;
	lcb->ctx = ctx;
	lcb->persist = 0;
	lcb->fdEvent = fdEvent & TEAMD_LOOP_FD_EVENT_MASK;
	if (tail)
		list_add_tail(&ctx->run_loop.callback_list, &lcb->list);
	else
		list_add(&ctx->run_loop.callback_list, &lcb->list);
	teamd_log_dbg("Added loop callback: %s, %p", lcb->name, lcb->priv);
	return 0;

lcb_free:
	free(lcb);
	return err;
}

Int32T teamd_loop_callback_fd_add(struct teamd_context *ctx,
			       const char * cbName, void *priv,
			       teamd_loop_callback_func_t func,
			       Int32T fd, Int32T fdEvent)
{
	return __teamdLoopCallbackFdAdd(ctx, cbName, priv, func,
					    fd, fdEvent, false);
}

Int32T teamd_loop_callback_fd_add_tail(struct teamd_context *ctx,
				    const char * cbName, void *priv,
				    teamd_loop_callback_func_t func,
				    Int32T fd, Int32T fdEvent)
{
	return __teamdLoopCallbackFdAdd(ctx, cbName, priv, func,
					    fd, fdEvent, true);
}

static Int32T teamdSetTime(struct timespec *interval, struct timespec *initial, struct timeval *tv)
{
	if (interval) {
		tv->tv_sec = interval->tv_sec;
		tv->tv_usec = interval->tv_nsec / 1000;
		return 1;
	}
	if (initial) {
		tv->tv_sec = initial->tv_sec;
		tv->tv_usec = initial->tv_nsec / 1000;
	}
	else {
		tv->tv_sec = 0;
		tv->tv_usec = 100000; /* to enable that */
	}
	return 0;
}

Int32T teamd_loop_callback_timer_add_set(struct teamd_context *ctx,
				      const char * cbName, void *priv,
				      teamd_loop_callback_func_t func,
				      struct timespec *interval,
				      struct timespec *initial)
{
	Int32T err;
	Int32T fd = 0;
	struct teamd_loop_callback *lcb;

	err = teamd_loop_callback_fd_add(ctx, cbName, priv, func, fd,
					 TEAMD_LOOP_FD_EVENT_READ);
	if (err) {
		close(fd);
		return err;
	}
	lcb = GetLcb(ctx, cbName, priv);
	if (!lcb) {
		teamd_log_err("Callback named \"%s\" not found.", cbName);
		return -ENOENT;
	}
	lcb->isPeriod = true;
	lcb->persist = teamdSetTime(interval, initial, &lcb->tv);
	teamd_log_dbg("Add & Set timer : %s, %p (tv=%ld.%06ld) set persist=%d",
		cbName, priv, lcb->tv.tv_sec, lcb->tv.tv_usec, lcb->persist);
	return 0;
}

Int32T teamd_loop_callback_timer_add(struct teamd_context *ctx,
				  const char * cbName, void *priv,
				  teamd_loop_callback_func_t func)
{
	return teamd_loop_callback_timer_add_set(ctx, cbName, priv, func,
						 NULL, NULL);
}

Int32T teamd_loop_callback_timer_set(struct teamd_context *ctx,
				  const char * cbName,
				  void *priv,
				  struct timespec *interval,
				  struct timespec *initial)
{
	struct teamd_loop_callback *lcb;

	if (!cbName || !priv)
		return -EINVAL;
	lcb = GetLcb(ctx, cbName, priv);
	if (!lcb) {
		teamd_log_err("Callback named \"%s\" not found.", cbName);
		return -ENOENT;
	}
	if (!lcb->isPeriod) {
		teamd_log_err("Can't reset non-periodic callback.");
		return -EINVAL;
	}
	if (lcb->persist == 0) {
#if 1
		if (lcb->registerEvent) {
			lacpDevTimerUnregister (lcb->registerEvent);
		}
#endif
		lcb->registerEvent = NULL;
	}
	lcb->persist = teamdSetTime(interval, initial, &lcb->tv);
	if (lcb->registerEvent) {
		lacpDevTimerUnregister (lcb->registerEvent);
#if 1
		lcb->registerEvent = lacpDevTimerRegister (lcb, &lcb->tv);
#else
		lcb->registerEvent = lacpDevTimerRegister (lcb, &lcb->tv, lcb->persist);
#endif
	}
	teamd_log_dbg("Set timer : %s, %p (tv=%ld.%06ld) set persist=%d",
		cbName, priv, lcb->tv.tv_sec, lcb->tv.tv_usec, lcb->persist);
	return 0;
}

void teamd_loop_callback_del(struct teamd_context *ctx, const char * cbName,
			     void *priv)
{
	struct teamd_loop_callback *lcb;
	struct teamd_loop_callback *tmp;
	bool found = false;

	ForEachLcbMultiMatchSafe(lcb, tmp, ctx, cbName, priv) {
		list_del(&lcb->list);
		if (lcb->registerEvent) {
			if (lcb->isPeriod) {
#if 1
				lacpDevTimerUnregister (lcb->registerEvent);
#else
				if (lcb->persist)
					lacpDevTimerUnregister (lcb->registerEvent);
#endif
			}
			else
				lacpDevFdUnregister (lcb->registerEvent);
		}
		teamd_log_dbg("Removed loop callback: %s, %p", lcb->name, lcb->priv);
		free(lcb->name);
		free(lcb);
		found = true;
		break;
	}
	if (!found)
		teamd_log_dbg("Callback named \"%s\" not found.", cbName);
}

Int32T teamd_loop_callback_enable(struct teamd_context *ctx, const char * cbName,
			       void *priv)
{
	struct teamd_loop_callback *lcb;
	bool found = false;

	ForEachLcbMultiMatch(lcb, ctx, cbName, priv)
	{
		lcb->enabled = true;
		found = true;
		break;
	}
	if (!found) {
		teamd_log_dbg("Callback named \"%s\" not found.", cbName);
		return -ENOENT;
	}
	if (lcb->registerEvent) {
		if (lcb->isPeriod) {
#if 1
			lacpDevTimerUnregister (lcb->registerEvent);
#else
			if (lcb->persist)
				lacpDevTimerUnregister (lcb->registerEvent);
#endif
		}
		else {
			teamd_log_dbg("Ignored loop callback: %s, %p", cbName, priv);
			return 0;
		}
	}
	if (lcb->isPeriod)
#if 1
		lcb->registerEvent = lacpDevTimerRegister (lcb, &lcb->tv);
#else
		lcb->registerEvent = lacpDevTimerRegister (lcb, &lcb->tv, lcb->persist);
#endif
	else {
		Int32T flag = TASK_PERSIST;
		if (lcb->fdEvent & TEAMD_LOOP_FD_EVENT_READ)
			flag |= TASK_READ;
		if (lcb->fdEvent & TEAMD_LOOP_FD_EVENT_WRITE)
			flag |= TASK_WRITE;
		lcb->registerEvent = lacpDevFdRegister (lcb->fd, flag, lcb);
	}
	teamd_log_dbg("Enabled loop callback: %s, %p", cbName, priv);
	return 0;
}

Int32T teamd_loop_callback_disable(struct teamd_context *ctx, const char * cbName,
				void *priv)
{
	struct teamd_loop_callback *lcb;
	bool found = false;

	ForEachLcbMultiMatch(lcb, ctx, cbName, priv) {
		lcb->enabled = false;
		found = true;
		break;
	}
	if (!found)
		return -ENOENT;

	if (lcb->registerEvent) {
		if (lcb->isPeriod) {
#if 1
			lacpDevTimerUnregister (lcb->registerEvent);
#else
			if (lcb->persist)
				lacpDevTimerUnregister (lcb->registerEvent);
#endif
		}
		else
			lacpDevFdUnregister (lcb->registerEvent);
		lcb->registerEvent = NULL;
		lcb->ctx = NULL;
		lcb->persist = 0;
	}
	teamd_log_dbg("Disabled loop callback: %s, %p", cbName, priv);

	return 0;
}

static Int32T callbackLibteamEvent(struct teamd_context *ctx, Int32T events,
				  void *priv)
{
	return team_handle_events(ctx->th);
}

#define LIBTEAM_EVENTS_CB_NAME "libteam_events"

static Int32T teamdRunLoopInit(struct teamd_context *ctx)
{
	Int32T err;

	list_init(&ctx->run_loop.callback_list);
	err = teamd_loop_callback_fd_add(ctx, LIBTEAM_EVENTS_CB_NAME, ctx,
					 callbackLibteamEvent,
					 team_get_event_fd(ctx->th),
					 TEAMD_LOOP_FD_EVENT_READ);
	if (err) {
		teamd_log_err("Failed to add libteam event loop callback");
		return err;
	}

	teamd_loop_callback_enable(ctx, LIBTEAM_EVENTS_CB_NAME, ctx);

	return 0;

}

static void teamdRunLoopFini(struct teamd_context *ctx)
{
	teamd_loop_callback_del(ctx, LIBTEAM_EVENTS_CB_NAME, NULL);
}

void lacpDevEventCallback (Int32T fd, void *loopCallback)
{
	Int32T err;
	struct teamd_loop_callback *lcb = loopCallback;

	teamd_log_dbg("Loop callback: %s, %p", lcb->name, lcb->priv);
#if 1
	if (lcb->isPeriod == true && lcb->persist == 0) {
		lacpDevTimerUnregister (lcb->registerEvent);
		lcb->registerEvent = NULL;
	}
#endif
	err = lcb->func(lcb->ctx, lcb->fdEvent, lcb->priv);
	if (err) {
		teamd_log_dbg("Failed loop callback: %s, %p", lcb->name, lcb->priv);
	}
}

static Int32T teamdParseHwaddr(const char * hwAddrStr, char **pHwAddr,
			Uint32T *pLen)
{
	const char * pos = hwAddrStr;
	Uint32T byteCount = 0;
	Uint32T tmp;
	Int32T err;
	char * hwAddr = NULL;
	char * newHwAddr;
	char * endPtr;

	while (true) {
		errno = 0;
		tmp = strtoul(pos, &endPtr, 16);
		if (errno != 0 || tmp > 0xFF) {
			err = -EINVAL;
			goto err_out;
		}
		byteCount++;
		newHwAddr = realloc(hwAddr, sizeof(char) * byteCount);
		if (!newHwAddr) {
			err = -ENOMEM;
			goto err_out;
		}
		hwAddr = newHwAddr;
		hwAddr[byteCount - 1] = (char) tmp;
		while (isspace(endPtr[0]) && endPtr[0] != '\0')
			endPtr++;
		if (endPtr[0] == ':') {
			pos = endPtr + 1;
		} else if (endPtr[0] == '\0') {
			break;
		} else {
			err = -EINVAL;
			goto err_out;
		}
	}
	*pHwAddr = hwAddr;
	*pLen = byteCount;
	return 0;
err_out:
	free(hwAddr);
	return err;
}

static Int32T teamdSetHwaddr(struct teamd_context *ctx)
{
	Int32T err;
	const char * hwAddrStr;
	char * hwAddr;
	Uint32T hwAddrLen;

	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	err = teamd_config_string_get(ctx, &hwAddrStr, "$.hwaddr");
	if (err)
		return 0; /* addr is not defined in config, no change needed */

	teamd_log_dbg("Hwaddr string: \"%s\".", hwAddrStr);
	err = teamdParseHwaddr(hwAddrStr, &hwAddr, &hwAddrLen);
	if (err) {
		teamd_log_err("Failed to parse hardware address.");
		return err;
	}

	if (hwAddrLen != ctx->hwaddr_len) {
		teamd_log_err("Passed hardware address has different length (%d) than team device has (%d).",
			      hwAddrLen, ctx->hwaddr_len);
		err = -EINVAL;
		goto free_hwaddr;
	}
	err = team_hwaddr_set(ctx->th, ctx->ifindex, hwAddr, hwAddrLen);
	if (!err)
		ctx->hwaddr_explicit = true;
free_hwaddr:
	free(hwAddr);
	return err;
}

Int32T teamd_port_add(struct teamd_context *ctx, const char * port_name)
{
	Int32T err;
	Uint32T ifIndex;
	bool state;

	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	ifIndex = team_ifname2ifindex(ctx->th, port_name);
	teamd_log_dbg("%s: Adding port (found ifindex \"%d\").",
		      port_name, ifIndex);
	team_port_set_promiscuous(ctx->th, port_name);
	err = team_port_down(ctx->th, port_name, &state);
	if (err)
		teamd_log_err("%s: Failed to down port (%s).", port_name, strerror(-err));
	if (err == 0)
		err = team_port_add(ctx->th, ifIndex);
	if (err)
		teamd_log_err("%s: Failed to add port (%s).", port_name, strerror(-err));
	return err;
}

Int32T teamd_port_remove(struct teamd_context *ctx, const char * port_name)
{
	Int32T err;
	Uint32T ifIndex;

	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	ifIndex = team_ifname2ifindex(ctx->th, port_name);
	teamd_log_dbg("%s: Removing port (found ifindex \"%d\").",
		      port_name, ifIndex);
	err = team_port_remove(ctx->th, ifIndex);
	if (err)
		teamd_log_err("%s: Failed to remove port.", port_name);
	return err;
}

static Int32T teamdAddPorts(struct teamd_context *ctx)
{
	Int32T err;
	const char * key;

	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	if (ctx->init_no_ports)
		return 0;

	teamd_config_for_each_key(key, ctx, "$.ports") {
		err = teamd_port_add(ctx, key);
		if (err)
			return err;
	}
	return 0;
}

Int32T teamd_port_modify(struct teamd_context *ctx, const char * port_name)
{
	Uint32T ifIndex;
	struct teamd_port *tdPort;
	Int32T err;

	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	ifIndex = team_ifname2ifindex(ctx->th, port_name);
	teamd_log_dbg("%s: Modifying port (found ifindex \"%d\").",
		      port_name, ifIndex);
	if (ifIndex == 0) {
		teamd_log_err("%s: Failed to find port.", port_name);
		return -1;
	}

	teamd_for_each_tdport(tdPort, ctx) {
		if (tdPort->ifindex == ifIndex) {
			if (ctx->runner->modify) {
				err = ctx->runner->modify(ctx, ctx->runner_priv, tdPort);
				if (err) {
					teamd_log_err("%s: Failed to modify port.", port_name);
					return err;
				}
			}
			break;
		}
	}
	return 0;
}

static Int32T teamdHwaddrCheckRange(struct teamd_context *ctx,
				     struct teamd_port *tdPort)
{
	const char * hwAddr;
	unsigned char hwAddrLen;
	Int32T err;

	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	if (ctx->port_obj_list_count != 1 || ctx->hwaddr_explicit)
		return 0;
	hwAddr = team_get_ifinfo_orig_hwaddr(tdPort->team_ifinfo);
	hwAddrLen = team_get_ifinfo_orig_hwaddr_len(tdPort->team_ifinfo);
	if (hwAddrLen != ctx->hwaddr_len) {
		teamd_log_err("%s: Port original hardware address has different length (%d) than team device has (%d).",
			      tdPort->ifname, hwAddrLen, ctx->hwaddr_len);
		return -EINVAL;
	}
	err = team_hwaddr_set(ctx->th, ctx->ifindex, hwAddr, hwAddrLen);
	if (err) {
		teamd_log_err("Failed to set team device hardware address.");
		return err;
	}
	return 0;
}

static Int32T teamdWatchEventPortAdded(struct teamd_context *ctx,
					struct teamd_port *tdPort, void *priv)
{
	Int32T err;
	Int32T tmp;

	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	err = teamdHwaddrCheckRange(ctx, tdPort);
	if (err)
		return err;

	err = teamd_config_int_get(ctx, &tmp, "$.ports.%s.queue_id",
				   tdPort->ifname);
	if (!err) {
		Uint32T queue_id;

		if (tmp < 0) {
			teamd_log_err("%s: \"queue_id\" must not be negative number.",
				      tdPort->ifname);
			return -EINVAL;
		}
		queue_id = tmp;
		err = team_set_port_queue_id(ctx->th, tdPort->ifindex,
					     queue_id);
		if (err) {
			teamd_log_err("%s: Failed to set \"queue_id\".",
				      tdPort->ifname);
			return err;
		}
	}
	err = teamd_config_int_get(ctx, &tmp, "$.ports.%s.prio",
				   tdPort->ifname);
	if (err)
		tmp = 0;
	err = team_set_port_priority(ctx->th, tdPort->ifindex, tmp);
	if (err) {
		teamd_log_err("%s: Failed to set \"priority\".",
			      tdPort->ifname);
		return err;
	}
	return 0;
}

static const struct teamd_event_watch_ops teamdPortWatchOps = {
	.port_added = teamdWatchEventPortAdded,
};

static Int32T teamdPortWatchInit(struct teamd_context *ctx)
{
	return teamd_event_watch_register(ctx, &teamdPortWatchOps, NULL);
}

static void teamdPortWatchFini(struct teamd_context *ctx)
{
	teamd_event_watch_unregister(ctx, &teamdPortWatchOps, NULL);
}

static Int32T teamdRunnerInit(struct teamd_context *ctx)
{
	DEBUGPRINT("%s\n", __FUNCTION__);
	Int32T err;
	const char * runnerName;

	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	err = teamd_config_string_get(ctx, &runnerName, "$.runner.name");
	if (err) {
		teamd_log_dbg("Failed to get team runner name from config.");
		runnerName = TEAMD_DEFAULT_RUNNER_NAME;
		err = teamd_config_string_set(ctx, runnerName, "$.runner.name");
		if (err) {
			teamd_log_err("Failed to set default team runner name in config.");
			return err;
		}
		teamd_log_dbg("Using default team runner \"%s\".", runnerName);
	} else {
		teamd_log_dbg("Using team runner \"%s\".", runnerName);
	}
	ctx->runner = teamdFindRunner(runnerName);
	if (!ctx->runner) {
		teamd_log_err("No runner named \"%s\" available.", runnerName);
		return -EINVAL;
	}

	if (ctx->runner->team_mode_name) {
		char * cur_mode;
		const char * new_mode = ctx->runner->team_mode_name;

		err = team_get_mode_name(ctx->th, &cur_mode);
		if (err) {
			teamd_log_err("Failed to det team mode.");
			return err;
		}
		if (strcmp(cur_mode, new_mode)) {
			err = team_set_mode_name(ctx->th, new_mode);
			if (err) {
				teamd_log_err("Failed to set team mode \"%s\".",
					      new_mode);
				return err;
			}
		}
	} else {
		teamd_log_warn("Note \"%s\" runner does not select team mode resulting in no functionality!",
			       runnerName);
	}

	if (ctx->runner->priv_size) {
		ctx->runner_priv = myzalloc(ctx->runner->priv_size);
		if (!ctx->runner_priv)
			return -ENOMEM;
	}

	if (ctx->runner->init) {
		err = ctx->runner->init(ctx, ctx->runner_priv);
		if (err)
			goto free_runner_priv;
	}
	return 0;

free_runner_priv:
	free(ctx->runner_priv);
	return err;
}

static void teamdRunnerFini(struct teamd_context *ctx)
{
	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	if (ctx->runner->fini)
		ctx->runner->fini(ctx, ctx->runner_priv);
	free(ctx->runner_priv);
	ctx->runner = NULL;
}

static void debugLogPortList(struct teamd_context *ctx)
{
	struct team_port *port;
	char buf[120];
	bool trunc;

	teamd_log_dbg("<port_list>");
	team_for_each_port(port, ctx->th) {
		trunc = team_port_str(port, buf, sizeof(buf));
		teamd_log_dbg("%s %s", buf, trunc ? "<trunc>" : "");
	}
	teamd_log_dbg("</port_list>");
}

static void debugLogOptionList(struct teamd_context *ctx)
{
	struct team_option *option;
	char buf[120];
	bool trunc;

	teamd_log_dbgx(ctx, 2, "<changed_option_list>");
	team_for_each_option(option, ctx->th) {
		if (!team_is_option_changed(option) ||
		    team_is_option_changed_locally(option))
			continue;
		trunc = team_option_str(ctx->th, option, buf, sizeof(buf));
		teamd_log_dbgx(ctx, 2, "%s %s", buf, trunc ? "<trunc>" : "");
	}
	teamd_log_dbgx(ctx, 2, "</changed_option_list>");
}

static void debugLogIfinfoList(struct teamd_context *ctx)
{
	struct team_ifinfo *ifInfo;
	char buf[120];
	bool trunc;

	teamd_log_dbg("<ifinfo_list>");
	team_for_each_ifinfo(ifInfo, ctx->th) {
		trunc = team_ifinfo_str(ifInfo, buf, sizeof(buf));
		teamd_log_dbg("%s %s", buf, trunc ? "<trunc>" : "");
	}
	teamd_log_dbg("</ifinfo_list>");
}

static Int32T debugChangeHandlerFunc(struct team_handle *th, void *priv,
				     team_change_type_mask_t type_mask)
{
	struct teamd_context *ctx = priv;

	if (type_mask & TEAM_PORT_CHANGE)
		debugLogPortList(ctx);
	if (type_mask & TEAM_OPTION_CHANGE)
		debugLogOptionList(ctx);
	if (type_mask & TEAM_IFINFO_CHANGE)
		debugLogIfinfoList(ctx);
	return 0;
}

static const struct team_change_handler debugChangeHandler = {
	.func = debugChangeHandlerFunc,
	.type_mask = TEAM_PORT_CHANGE | TEAM_OPTION_CHANGE | TEAM_IFINFO_CHANGE,
};

static Int32T teamdRegisterDefaultHandlers(struct teamd_context *ctx)
{
	if (!ctx->debug)
		return 0;
	return team_change_handler_register(ctx->th,
					    &debugChangeHandler, ctx);
}

static void teamdUnregisterDefaultHandlers(struct teamd_context *ctx)
{
	if (!ctx->debug)
		return;
	team_change_handler_unregister(ctx->th, &debugChangeHandler, ctx);
}

static Int32T teamdInit(struct teamd_context *ctx)
{
	Int32T err;

	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	ctx->th = team_alloc();
	if (!ctx->th) {
		teamd_log_err("Team alloc failed.");
		return -ENOMEM;
	}

	ctx->ifindex = team_ifname2ifindex(ctx->th, ctx->team_devname);
	if (ctx->ifindex && ctx->take_over)
		goto skip_create;

	if (ctx->force_recreate)
		err = team_recreate(ctx->th, ctx->team_devname);
	else
		err = team_create(ctx->th, ctx->team_devname);
	if (err) {
		teamd_log_err("Failed to create team device (%s).", ctx->team_devname);
		goto team_free;
	}

	ctx->ifindex = team_ifname2ifindex(ctx->th, ctx->team_devname);
	if (!ctx->ifindex) {
		teamd_log_err("Netdevice \"%s\" not found.", ctx->team_devname);
		err = -ENODEV;
		goto team_destroy;
	}
skip_create:

	err = team_init(ctx->th, ctx->ifindex);
	if (err) {
		teamd_log_err("Team init failed.");
		goto team_destroy;
	}

	ctx->ifinfo = team_get_ifinfo(ctx->th);
	ctx->hwaddr = team_get_ifinfo_hwaddr(ctx->ifinfo);
	ctx->hwaddr_len = team_get_ifinfo_hwaddr_len(ctx->ifinfo);

	err = teamdSetHwaddr(ctx);
	if (err) {
		teamd_log_err("Hardware address set failed.");
		goto team_destroy;
	}

	err = teamdRunLoopInit(ctx);
	if (err) {
		teamd_log_err("Failed to init run loop.");
		goto team_destroy;
	}

	err = teamdRegisterDefaultHandlers(ctx);
	if (err) {
		teamd_log_err("Failed to register debug event handlers.");
		goto run_loop_fini;
	}

	err = teamd_events_init(ctx);
	if (err) {
		teamd_log_err("Failed to init events infrastructure.");
		goto team_unreg_debug_handlers;
	}

	err = teamd_option_watch_init(ctx);
	if (err) {
		teamd_log_err("Failed to init option watches.");
		goto events_fini;
	}

	err = teamd_ifinfo_watch_init(ctx);
	if (err) {
		teamd_log_err("Failed to init ifinfo watches.");
		goto option_watch_fini;
	}

	err = teamdPortWatchInit(ctx);
	if (err) {
		teamd_log_err("Failed to init port watch.");
		goto ifinfo_watch_fini;
	}

	err = teamd_state_init(ctx);
	if (err) {
		teamd_log_err("Failed to init state json infrastructure.");
		goto port_watch_fini;
	}

	err = teamd_per_port_init(ctx);
	if (err) {
		teamd_log_err("Failed to init per-port.");
		goto state_fini;
	}

	err = teamd_link_watch_init(ctx);
	if (err) {
		teamd_log_err("Failed to init link watch.");
		goto per_port_fini;
	}

	err = teamdRunnerInit(ctx);
	if (err) {
		teamd_log_err("Failed to init runner.");
		goto link_watch_fini;
	}

	err = teamd_state_basics_init(ctx);
	if (err) {
		teamd_log_err("Failed to init state json basics.");
		goto runner_fini;
	}

	err = teamd_phys_port_check_init(ctx);
	if (err) {
		teamd_log_err("Failed to init SR-IOV support.");
		goto state_basics_fini;
	}

	err = teamdAddPorts(ctx);
	if (err) {
		teamd_log_err("Failed to add ports.");
		goto phys_port_check_fini;
	}

	return 0;

phys_port_check_fini:
	teamd_phys_port_check_fini(ctx);
state_basics_fini:
	teamd_state_basics_fini(ctx);
runner_fini:
	teamdRunnerFini(ctx);
link_watch_fini:
	teamd_link_watch_fini(ctx);
per_port_fini:
	teamd_per_port_fini(ctx);
state_fini:
	teamd_state_fini(ctx);
port_watch_fini:
	teamdPortWatchFini(ctx);
ifinfo_watch_fini:
	teamd_ifinfo_watch_fini(ctx);
option_watch_fini:
	teamd_option_watch_fini(ctx);
events_fini:
	teamd_events_fini(ctx);
team_unreg_debug_handlers:
	teamdUnregisterDefaultHandlers(ctx);
run_loop_fini:
	teamdRunLoopFini(ctx);
team_destroy:
	if (!ctx->take_over)
		team_destroy(ctx->th);
team_free:
	team_free(ctx->th);
	return err;
}

static void teamdFini(struct teamd_context *ctx)
{
	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	teamd_phys_port_check_fini(ctx);
	teamd_state_basics_fini(ctx);
	teamdRunnerFini(ctx);
	teamd_link_watch_fini(ctx);
	teamd_per_port_fini(ctx);
	teamd_state_fini(ctx);
	teamd_ifinfo_watch_fini(ctx);
	teamd_option_watch_fini(ctx);
	teamd_events_fini(ctx);
	teamdUnregisterDefaultHandlers(ctx);
	teamdRunLoopFini(ctx);
	if (!ctx->take_over)
		team_destroy(ctx->th);
	team_free(ctx->th);
}

static Int32T teamdStart(struct teamd_context *ctx)
{
	Int32T err = 0;

	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	err = teamdInit(ctx);
	if (err)
		return err;

	teamd_log_info("Successfully started %p.", ctx);

	return err;
}

static Int32T teamdGenerateDevName(struct teamd_context *ctx)
{
	char buf[IFNAMSIZ];
	Int32T i = 0;
	Uint32T ifIndex;
	Int32T ret;
	Int32T err;

	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	do {
		ret = snprintf(buf, sizeof(buf),
			       TEAMD_DEFAULT_DEVNAME_PREFIX "%d", i++);
		if (ret >= sizeof(buf))
			return -EINVAL;
		err = ifname2ifindex(&ifIndex, buf);
		if (err)
			return err;
	} while (ifIndex);
	teamd_log_dbg("Generated team device name \"%s\".", buf);

	ctx->team_devname = strdup(buf);
	if (!ctx->team_devname)
		return -ENOMEM;
	return 0;
}

static Int32T teamdGetDevName(struct teamd_context *ctx, bool generate_enabled)
{
	Int32T err;

	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	if (!ctx->team_devname) {
		const char * team_name;

		err = teamd_config_string_get(ctx, &team_name, "$.device");
		if (!err) {
			ctx->team_devname = strdup(team_name);
			if (!ctx->team_devname) {
				teamd_log_err("Failed allocate memory for device name.");
				return -ENOMEM;
			}
			goto skip_set;
		} else {
			teamd_log_dbg("Failed to get team device name from config.");
			if (generate_enabled) {
				err = teamdGenerateDevName(ctx);
				if (err) {
					teamd_log_err("Failed to generate team device name.");
					return err;
				}
			} else {
				teamd_log_err("Team device name not specified.");
				return -EINVAL;
			}
		}
	}
	err = teamd_config_string_set(ctx, ctx->team_devname, "$.device");
	if (err) {
		teamd_log_err("Failed to set team device name in config (%s).", strerror(err));
		return err;
	}

skip_set:
	teamd_log_dbg("Using team device \"%s\".", ctx->team_devname);

	return 0;
}

static Int32T teamdContextInit(struct teamd_context **pctx)
{
	struct teamd_context *ctx;

	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	ctx = NNMALLOC (MEM_GLOBAL, sizeof(*ctx));
	memset (ctx, 0, sizeof(*ctx));
	if (!ctx)
		return -ENOMEM;

	ctx->force_recreate = 1;

	*pctx = ctx;

	return 0;
}

static void teamdContextFini(struct teamd_context *ctx)
{
	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	free(ctx->team_devname);
	free(ctx->config_text);
	free(ctx->config_file);
	NNFREE (MEM_GLOBAL, ctx);
}

void lacpDevAggregatorFini(struct teamd_context *ctx)
{
	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	teamdFini(ctx);
	teamd_config_free(ctx);
	teamdContextFini(ctx);
}

void lacpDevAggregatorFiniTimer(Int32T fd, Int32T event, void *context);

Int32T lacpDevAggregatorFiniBusy(struct teamd_context *ctx, struct timeval *tv, int val)
{
	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	if (teamd_has_ports(ctx)) {
		lacpDevFiniStart (ctx, tv, lacpDevAggregatorFiniTimer);
		return 1;
	}
	return 0;
}
	
void lacpDevAggregatorFiniTimer(Int32T fd, Int32T event, void *context)
{
	struct timeval tv;
	struct teamd_context *ctx = context;

	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	tv.tv_sec = 0;
	tv.tv_usec = 100000;
	if (lacpDevAggregatorFiniBusy (ctx, &tv, 1))
		return;
	lacpDevAggregatorFini(ctx);
	lacpDevFiniEnd (context);
}

void lacpDevAggregatorDelete(void *context)
{
	Int32T err;
	struct teamd_context *ctx = context;
	struct timeval tv;

	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	err = teamdGetDevName(ctx, 0);
	if (err)
		return;

	teamdFlushPorts(ctx);
	tv.tv_sec = 0;
	tv.tv_usec = 500000;
	if (lacpDevAggregatorFiniBusy (ctx, &tv, 0))
		return;
	lacpDevAggregatorFini(ctx);
	lacpDevFiniEnd (context);
}

void *lacpDevAggregatorAdd(char * file, Int32T debug)
{
	Int32T err;
	struct teamd_context *ctx;

	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	err = teamdContextInit(&ctx);
	if (err) {
		fprintf(stderr, "Failed to init context\n");
		return NULL;
	}

	ctx->config_file = realpath(file, NULL);
	if (!ctx->config_file) {
		fprintf(stderr, "Failed to get absolute path of \"%s\": %s\n",
			file, strerror(errno));
		free(ctx);
		return NULL;
	}

	ctx->debug = debug;

	err = teamd_config_load(ctx);
	if (err) {
		teamd_log_err("Failed to load config.");
		goto context_fini;
	}

	err = teamdGetDevName(ctx, 1);
	if (err)
		goto config_free;

	if (ctx->config_file)
		teamd_log_dbg("Using config file \"%s\"", ctx->config_file);

	err = teamdStart(ctx);
	if (err == 0)
		return ctx;

	teamd_log_err("Failed: %s", strerror(-err));

config_free:
	teamd_config_free(ctx);
context_fini:
	teamdContextFini(ctx);
	return NULL;
}

void lacpDevPortAdd(void *context, char * ifname)
{
	struct teamd_context *ctx = context;
	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	teamd_config_reload(ctx);
	teamd_port_add(ctx, ifname);
}

void lacpDevPortDelete(void *context, char * ifname)
{
	struct teamd_context *ctx = context;
	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	teamd_port_remove(ctx, ifname);
}

void lacpDevPortModify(void *context, char * ifname)
{
	struct teamd_context *ctx = context;
	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	teamd_config_reload(ctx);
	teamd_port_modify(ctx, ifname);
}

void lacpDevModify(void *context)
{
	Int32T err;
	struct teamd_context *ctx = context;

	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	teamd_config_reload(ctx);
	if (ctx->runner->modify) {
		err = ctx->runner->modify(ctx, ctx->runner_priv, NULL);
		if (err) {
			teamd_log_err("%s: Failed to modify device.", __FUNCTION__);
		}
	}
}

// for DLU
void lacpDevCallbackChange(void *context)
{
	struct teamd_context *ctx = context;
	struct teamd_loop_callback *lcb;

	DEBUGPRINT("%s\n", __FUNCTION__);
	if (pLacp->debug) NNLOG(LOG_DEBUG, "%s\n", __FUNCTION__);
	list_for_each_node_entry(lcb, &ctx->run_loop.callback_list, list)
	{
		if (! lcb->enabled)
			continue;
		if (lcb->registerEvent) {
			if (lcb->isPeriod) {
				lcb->registerEvent = lacpDevTimerUpdate (lcb, &lcb->tv, lcb->registerEvent);
			}
			else {
				lcb->registerEvent = lacpDevFdUpdate (lcb, lcb->registerEvent);
			}
		}
	}
}

char * lacpDevConfigDump(void *context)
{
	struct teamd_context *ctx = context;
    char * cfg;
    Int32T err;

    err = teamd_config_dump(ctx, &cfg);
    if (err) {
        teamd_log_err("Failed to dump config.");
        return NULL;
    }
    return cfg;
}

char * lacpDevStateDump(void *context)
{
	struct teamd_context *ctx = context;
    char * state;
    Int32T err;

    err = teamd_state_dump(ctx, &state);
    if (err) {
        teamd_log_err("Failed to dump state.");
        return NULL;
    }
    return state;
}
