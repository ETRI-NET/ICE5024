/*
 * IS-IS Rout(e)ing protocol - isis_dynhn.c
 *                             Dynamic hostname cache
 * Copyright (C) 2001,2002   Sampo Saaristo
 *                           Tampere University of Technology      
 *                           Institute of Communications Engineering
 *
 * This program is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU General Public Licenseas published by the Free 
 * Software Foundation; either version 2 of the License, or (at your option) 
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
 * more details.
 *
 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//#include <zebra.h>

#include "vty.h"
#include "linklist.h"
#include "memory.h"
#include "log.h"
#include "stream.h"
#include "command.h"
#include "if.h"

#include "nosLib.h"

#include "dict.h"
#include "isis_constants.h"
#include "isis_common.h"
#include "isis_flags.h"
#include "isis_circuit.h"
#include "isisd.h"
#include "isis_dynhn.h"
#include "isis_misc.h"
#include "isis_constants.h"

#include "nnDefines.h"

struct list *dyn_cache = NULL;


void
dyn_cache_init (void)
{
  struct timeval tv = {0,0};

  if (dyn_cache == NULL)
    dyn_cache = list_new ();

  tv.tv_sec = 120;
  isis->pTimerDyncClean = taskTimerSet(dynCacheCleanup, tv, 0, NULL);

  return;
}

void
dynCacheCleanup (Int32T fd, Int16T event, void * arg)
{
  struct listnode *node, *nnode;
  struct isis_dynhn *dyn;
  time_t now = time (NULL);
  struct timeval tv = {0,0};

  isis->pTimerDyncClean = NULL;

  for (ALL_LIST_ELEMENTS (dyn_cache, node, nnode, dyn))
    {
      if ((now - dyn->refresh) < MAX_LSP_LIFETIME)
        continue;

      list_delete_node (dyn_cache, node);
      XFREE (MTYPE_ISIS_DYNHN, dyn);
    }

  tv.tv_sec = 120;
  isis->pTimerDyncClean = taskTimerSet(dynCacheCleanup, tv, 0, NULL);
}

struct isis_dynhn *
dynhn_find_by_id (u_char * id)
{
  struct listnode *node = NULL;
  struct isis_dynhn *dyn = NULL;

  for (ALL_LIST_ELEMENTS_RO (dyn_cache, node, dyn))
    if (memcmp (dyn->id, id, ISIS_SYS_ID_LEN) == 0)
      return dyn;

  return NULL;
}

struct isis_dynhn *
dynhn_find_by_name (const char *hostname)
{
  struct listnode *node = NULL;
  struct isis_dynhn *dyn = NULL;

  for (ALL_LIST_ELEMENTS_RO (dyn_cache, node, dyn))
    if (strncmp ((char *)dyn->name.name, hostname, 255) == 0)
      return dyn;

  return NULL;
}

void
isis_dynhn_insert (u_char * id, struct hostname *hostname, int level)
{
  struct isis_dynhn *dyn;

  dyn = dynhn_find_by_id (id);
  if (dyn)
    {
      memcpy (&dyn->name, hostname, hostname->namelen + 1);
      memcpy (dyn->id, id, ISIS_SYS_ID_LEN);
      dyn->refresh = time (NULL);
      return;
    }
  dyn = XCALLOC (MTYPE_ISIS_DYNHN, sizeof (struct isis_dynhn));
  if (!dyn)
    {
      zlog_warn ("isis_dynhn_insert(): out of memory!");
      return;
    }

  /* we also copy the length */
  memcpy (&dyn->name, hostname, hostname->namelen + 1);
  memcpy (dyn->id, id, ISIS_SYS_ID_LEN);
  dyn->refresh = time (NULL);
  dyn->level = level;

  listnode_add (dyn_cache, dyn);

  return;
}

void
isis_dynhn_remove (u_char * id)
{
  struct isis_dynhn *dyn;

  dyn = dynhn_find_by_id (id);
  if (!dyn)
    return;
  listnode_delete (dyn_cache, dyn);
  XFREE (MTYPE_ISIS_DYNHN, dyn);
  return;
}

/*
 * Level  System ID      Dynamic Hostname  (notag)
 *  2     0000.0000.0001 foo-gw
 *  2     0000.0000.0002 bar-gw
 *      * 0000.0000.0004 this-gw
 */
void
dynhnPrintAll (struct cmsh *cmsh)
{
	struct listnode *node;
	struct isis_dynhn *dyn;

	cmdPrint(cmsh, "Level  System ID      Dynamic Hostname");
	for (ALL_LIST_ELEMENTS_RO(dyn_cache, node, dyn)) {
		cmdPrint(cmsh, "%-7d%-15s%-15s",
				dyn->level, sysid_print(dyn->id), dyn->name.name);
	}

	cmdPrint(cmsh, "     * %s %s", sysid_print(isis->sysid), unix_hostname());

	return;
}
