/*
 * IS-IS Rout(e)ing protocol - isis_events.h   
 *
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

 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
//#include <zebra.h>
#include <sys/time.h>
#include <unistd.h>

#include "log.h"
#include "memory.h"
#include "if.h"
#include "linklist.h"
#include "command.h"
#include "hash.h"
#include "prefix.h"
#include "stream.h"
#include "table.h"

#include "nosLib.h"

#include "dict.h"
#include "include-netbsd/iso.h"
#include "isis_constants.h"
#include "isis_common.h"
#include "isis_flags.h"
#include "isis_circuit.h"
#include "isis_tlv.h"
#include "isis_lsp.h"
#include "isis_pdu.h"
#include "isis_network.h"
#include "isis_misc.h"
#include "isis_constants.h"
#include "isis_adjacency.h"
#include "isis_dr.h"
#include "isisd.h"
#include "isis_csm.h"
#include "isis_events.h"
#include "isis_spf.h"

/* debug isis-spf spf-events 
 4w4d: ISIS-Spf (tlt): L2 SPF needed, new adjacency, from 0x609229F4
 4w4d: ISIS-Spf (tlt): L2, 0000.0000.0042.01-00 TLV contents changed, code 0x2
 4w4d: ISIS-Spf (tlt): L2, new LSP 0 DEAD.BEEF.0043.00-00
 4w5d: ISIS-Spf (tlt): L1 SPF needed, periodic SPF, from 0x6091C844
 4w5d: ISIS-Spf (tlt): L2 SPF needed, periodic SPF, from 0x6091C844
*/

void
isis_event_circuit_state_change (struct isis_circuit *circuit,
                                 struct isis_area *area, int up)
{
  area->circuit_state_changes++;

  if (isis->debugs & DEBUG_EVENTS)
    zlog_debug ("ISIS-Evt (%s) circuit %s", area->area_tag,
                up ? "up" : "down");

  /*
   * Regenerate LSPs this affects
   */
  lsp_regenerate_schedule (area, IS_LEVEL_1 | IS_LEVEL_2, 0);

  return;
}

static void
area_resign_level (struct isis_area *area, int level)
{
  if (area->lspdb[level - 1])
    {
      lsp_db_destroy (area->lspdb[level - 1]);
      area->lspdb[level - 1] = NULL;
    }
  if (area->spftree[level - 1])
    {
      isis_spftree_del (area->spftree[level - 1]);
      area->spftree[level - 1] = NULL;
    }
#ifdef HAVE_IPV6
  if (area->spftree6[level - 1])
    {
      isis_spftree_del (area->spftree6[level - 1]);
      area->spftree6[level - 1] = NULL;
    }
#endif
  if (area->route_table[level - 1])
    {
      route_table_finish (area->route_table[level - 1]);
      area->route_table[level - 1] = NULL;
    }
#ifdef HAVE_IPV6
  if (area->route_table6[level - 1])
    {
      route_table_finish (area->route_table6[level - 1]);
      area->route_table6[level - 1] = NULL;
    }
#endif /* HAVE_IPV6 */

  if(area->pTimerLspRefresh[level - 1])
  {
	  taskTimerDel(area->pTimerLspRefresh[level - 1]);
	  area->pTimerLspRefresh[level - 1] = NULL;
  }
}

void
isis_event_system_type_change (struct isis_area *area, int newtype)
{
  struct listnode *node;
  struct isis_circuit *circuit;

  if (isis->debugs & DEBUG_EVENTS)
    zlog_debug ("ISIS-Evt (%s) system type change %s -> %s", area->area_tag,
	       circuit_t2string (area->is_type), circuit_t2string (newtype));

  if (area->is_type == newtype)
    return;			/* No change */

  switch (area->is_type)
  {
    case IS_LEVEL_1:
      if (newtype == IS_LEVEL_2)
        area_resign_level (area, IS_LEVEL_1);

      if (area->lspdb[1] == NULL)
        area->lspdb[1] = lsp_db_init ();
      if (area->route_table[1] == NULL)
        area->route_table[1] = route_table_init ();
#ifdef HAVE_IPV6
      if (area->route_table6[1] == NULL)
        area->route_table6[1] = route_table_init ();
#endif /* HAVE_IPV6 */
      break;

    case IS_LEVEL_1_AND_2:
      if (newtype == IS_LEVEL_1)
        area_resign_level (area, IS_LEVEL_2);
      else
        area_resign_level (area, IS_LEVEL_1);
      break;

    case IS_LEVEL_2:
      if (newtype == IS_LEVEL_1)
        area_resign_level (area, IS_LEVEL_2);

      if (area->lspdb[0] == NULL)
        area->lspdb[0] = lsp_db_init ();
      if (area->route_table[0] == NULL)
        area->route_table[0] = route_table_init ();
#ifdef HAVE_IPV6
      if (area->route_table6[0] == NULL)
        area->route_table6[0] = route_table_init ();
#endif /* HAVE_IPV6 */
      break;

    default:
      break;
  }

  area->is_type = newtype;

  /* override circuit's is_type */
  if (area->is_type != IS_LEVEL_1_AND_2)
  {
    for (ALL_LIST_ELEMENTS_RO (area->circuit_list, node, circuit))
      isis_event_circuit_type_change (circuit, newtype);
  }

  spftree_area_init (area);

  if (newtype & IS_LEVEL_1)
    lsp_generate (area, IS_LEVEL_1);
  if (newtype & IS_LEVEL_2)
    lsp_generate (area, IS_LEVEL_2);
  lsp_regenerate_schedule (area, IS_LEVEL_1 | IS_LEVEL_2, 1);

  return;
}

static void
circuit_commence_level (struct isis_circuit *circuit, int level)
{
  struct timeval tv = {0,0};
  if (level == 1)
    {
      if (! circuit->is_passive){
    	  tv.tv_sec = circuit->psnp_interval[0];
    	  circuit->pTimerSendPsnp[0] = taskTimerSet(sendL1Psnp, tv, 0, (void *)circuit);
      }

      if (circuit->circ_type == CIRCUIT_T_BROADCAST)
	{
      tv.tv_sec = 2 * circuit->hello_interval[0];
      circuit->u.bc.pTimerRunDr[0] = taskTimerSet(isisRunDrL1, tv, 0, (void *)circuit);

      tv.tv_sec = circuit->hello_interval[0];
      circuit->u.bc.pTimerSendLanHello[0] = taskTimerSet(sendLanL1Hello, tv, 0, (void *)circuit);

	  circuit->u.bc.lan_neighs[0] = list_new ();
	}
    }
  else
    {
      if (! circuit->is_passive)
      {
    	  tv.tv_sec = circuit->psnp_interval[1];
    	  circuit->pTimerSendPsnp[1] = taskTimerSet(sendL2Psnp, tv, 0, (void *)circuit);
      }

      if (circuit->circ_type == CIRCUIT_T_BROADCAST)
	{
      tv.tv_sec = 2 * circuit->hello_interval[1];
      circuit->u.bc.pTimerRunDr[1] = taskTimerSet(isisRunDrL2, tv, 0, (void *)circuit);

      tv.tv_sec = circuit->hello_interval[1];
      circuit->u.bc.pTimerSendLanHello[1] = taskTimerSet(sendLanL2Hello, tv, 0, (void *)circuit);

	  circuit->u.bc.lan_neighs[1] = list_new ();
	}
    }

  return;
}

static void
circuit_resign_level (struct isis_circuit *circuit, int level)
{
  int idx = level - 1;

  if(circuit->pTimerSendCsnp[idx])
  {
	  taskTimerDel(circuit->pTimerSendCsnp[idx]);
	  circuit->pTimerSendCsnp[idx] = NULL;
  }
  if(circuit->pTimerSendPsnp[idx])
  {
	  taskTimerDel(circuit->pTimerSendPsnp[idx]);
	  circuit->pTimerSendPsnp[idx] = NULL;
  }

  if (circuit->circ_type == CIRCUIT_T_BROADCAST)
    {
	  if(circuit->u.bc.pTimerSendLanHello[idx])
	  {
		  taskTimerDel(circuit->u.bc.pTimerSendLanHello[idx]);
		  circuit->u.bc.pTimerSendLanHello[idx] = NULL;
	  }

      if(circuit->u.bc.pTimerRunDr[idx])
      {
    	  taskTimerDel(circuit->u.bc.pTimerRunDr[idx]);
    	  circuit->u.bc.pTimerRunDr[idx] = NULL;
      }

      if(circuit->u.bc.pTimerRefreshPseudoLsp[idx])
      {
    	  taskTimerDel(circuit->u.bc.pTimerRefreshPseudoLsp[idx]);
    	  circuit->u.bc.pTimerRefreshPseudoLsp[idx] = NULL;
      }

      circuit->u.bc.run_dr_elect[idx] = 0;
      list_delete (circuit->u.bc.lan_neighs[idx]);
      circuit->u.bc.lan_neighs[idx] = NULL;
    }

  return;
}

void
isis_event_circuit_type_change (struct isis_circuit *circuit, int newtype)
{
  if (circuit->state != C_STATE_UP)
  {
    circuit->is_type = newtype;
    return;
  }

  if (isis->debugs & DEBUG_EVENTS)
    zlog_debug ("ISIS-Evt (%s) circuit type change %s -> %s",
	       circuit->area->area_tag,
	       circuit_t2string (circuit->is_type),
	       circuit_t2string (newtype));

  if (circuit->is_type == newtype)
    return;			/* No change */

  if (!(newtype & circuit->area->is_type))
    {
      zlog_err ("ISIS-Evt (%s) circuit type change - invalid level %s because"
		" area is %s", circuit->area->area_tag,
		circuit_t2string (newtype),
		circuit_t2string (circuit->area->is_type));
      return;
    }

  switch (circuit->is_type)
    {
    case IS_LEVEL_1:
      if (newtype == IS_LEVEL_2)
	circuit_resign_level (circuit, 1);
      circuit_commence_level (circuit, 2);
      break;
    case IS_LEVEL_1_AND_2:
      if (newtype == IS_LEVEL_1)
	circuit_resign_level (circuit, 2);
      else
	circuit_resign_level (circuit, 1);
      break;
    case IS_LEVEL_2:
      if (newtype == IS_LEVEL_1)
	circuit_resign_level (circuit, 2);
      circuit_commence_level (circuit, 1);
      break;
    default:
      break;
    }

  circuit->is_type = newtype;
  lsp_regenerate_schedule (circuit->area, IS_LEVEL_1 | IS_LEVEL_2, 0);

  return;
}

 /* 04/18/2002 by Gwak. */
 /**************************************************************************
  *
  * EVENTS for LSP generation
  *
  * 1) an Adajacency or Circuit Up/Down event
  * 2) a chnage in Circuit metric
  * 3) a change in Reachable Address metric
  * 4) a change in manualAreaAddresses
  * 5) a change in systemID
  * 6) a change in DIS status
  * 7) a chnage in the waiting status
  *
  * ***********************************************************************
  *
  * current support event
  *
  * 1) Adjacency Up/Down event
  * 6) a change in DIS status
  *
  * ***********************************************************************/

void
isis_event_adjacency_state_change (struct isis_adjacency *adj, int newstate)
{
  /* adjacency state change event. 
   * - the only proto-type was supported */

  /* invalid arguments */
  if (!adj || !adj->circuit || !adj->circuit->area)
    return;

  if (isis->debugs & DEBUG_EVENTS)
    zlog_debug ("ISIS-Evt (%s) Adjacency State change",
		adj->circuit->area->area_tag);

  /* LSP generation again */
  lsp_regenerate_schedule (adj->circuit->area, IS_LEVEL_1 | IS_LEVEL_2, 0);

  return;
}

/* events supporting code */
void
isisEventDisStatusChange (Int32T fd, Int16T event, void * arg)
{
  struct isis_circuit *circuit = NULL;

  circuit = (struct isis_circuit *)arg;

  /* invalid arguments */
  if (!circuit || !circuit->area)
    return;
  if (isis->debugs & DEBUG_EVENTS)
    zlog_debug ("ISIS-Evt (%s) DIS status change", circuit->area->area_tag);

  /* LSP generation again */
  lsp_regenerate_schedule (circuit->area, IS_LEVEL_1 | IS_LEVEL_2, 0);

}

void
isis_event_auth_failure (char *area_tag, const char *error_string, u_char *sysid)
{
  if (isis->debugs & DEBUG_EVENTS)
    zlog_debug ("ISIS-Evt (%s) Authentication failure %s from %s",
		area_tag, error_string, sysid_print (sysid));

  return;
}
