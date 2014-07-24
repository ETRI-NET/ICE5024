/*
 * IS-IS Rout(e)ing protocol - isis_dr.c
 *                             IS-IS designated router related routines   
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


#include <sys/time.h>
#include <unistd.h>

#include "log.h"
#include "hash.h"
#include "linklist.h"
#include "vty.h"
#include "stream.h"
#include "if.h"

#include "nosLib.h"

#include "dict.h"
#include "isis_constants.h"
#include "isis_common.h"
#include "isis_misc.h"
#include "isis_flags.h"
#include "isis_circuit.h"
#include "isisd.h"
#include "isis_adjacency.h"
#include "isis_constants.h"
#include "isis_pdu.h"
#include "isis_tlv.h"
#include "isis_lsp.h"
#include "isis_dr.h"
#include "isis_events.h"

const char *
isis_disflag2string (int disflag)
{

  switch (disflag)
    {
    case ISIS_IS_NOT_DIS:
      return "is not DIS";
    case ISIS_IS_DIS:
      return "is DIS";
    case ISIS_WAS_DIS:
      return "was DIS";
    default:
      return "unknown DIS state";
    }
  return NULL;			/* not reached */
}

void
isisRunDrL1 (Int32T fd, Int16T event, void * arg)
{
  struct isis_circuit *circuit;

  circuit = (struct isis_circuit *)arg;

  assert (circuit);

  if (circuit->u.bc.run_dr_elect[0])
    zlog_warn ("isis_run_dr(): run_dr_elect already set for l1");

  circuit->u.bc.pTimerRunDr[0] = NULL;

  circuit->u.bc.run_dr_elect[0] = 1;

}

void
isisRunDrL2 (Int32T fd, Int16T event, void * arg)
{
  struct isis_circuit *circuit;

  circuit = (struct isis_circuit *)arg;
  assert (circuit);

  if (circuit->u.bc.run_dr_elect[1])
    zlog_warn ("isis_run_dr(): run_dr_elect already set for l2");


  circuit->u.bc.pTimerRunDr[1] = NULL;

  circuit->u.bc.run_dr_elect[1] = 1;

}

static int
isis_check_dr_change (struct isis_adjacency *adj, int level)
{
  int i;

  if (adj->dis_record[level - 1].dis !=
      adj->dis_record[(1 * ISIS_LEVELS) + level - 1].dis)
    /* was there a DIS state transition ? */
    {
      adj->dischanges[level - 1]++;
      /* ok rotate the history list through */
      for (i = DIS_RECORDS - 1; i > 0; i--)
	{
	  adj->dis_record[(i * ISIS_LEVELS) + level - 1].dis =
	    adj->dis_record[((i - 1) * ISIS_LEVELS) + level - 1].dis;
	  adj->dis_record[(i * ISIS_LEVELS) + level - 1].last_dis_change =
	    adj->dis_record[((i - 1) * ISIS_LEVELS) + level -
			    1].last_dis_change;
	}
    }
  return ISIS_OK;
}

int
isis_dr_elect (struct isis_circuit *circuit, int level)
{
  struct list *adjdb;
  struct listnode *node;
  struct isis_adjacency *adj, *adj_dr = NULL;
  struct list *list = list_new ();
  u_char own_prio;
  int biggest_prio = -1;
  int cmp_res, retval = ISIS_OK;

  own_prio = circuit->priority[level - 1];
  adjdb = circuit->u.bc.adjdb[level - 1];

  if (!adjdb)
    {
      zlog_warn ("isis_dr_elect() adjdb == NULL");
      list_delete (list);
      return ISIS_WARNING;
    }
  isis_adj_build_up_list (adjdb, list);

  /*
   * Loop the adjacencies and find the one with the biggest priority
   */
  for (ALL_LIST_ELEMENTS_RO (list, node, adj))
    {
      /* clear flag for show output */
      adj->dis_record[level - 1].dis = ISIS_IS_NOT_DIS;
      adj->dis_record[level - 1].last_dis_change = time (NULL);

      if (adj->prio[level - 1] > biggest_prio)
	{
	  biggest_prio = adj->prio[level - 1];
	  adj_dr = adj;
	}
      else if (adj->prio[level - 1] == biggest_prio)
	{
	  /*
	   * Comparison of MACs breaks a tie
	   */
	  if (adj_dr)
	    {
	      cmp_res = memcmp (adj_dr->snpa, adj->snpa, ETH_ALEN);
	      if (cmp_res < 0)
		{
		  adj_dr = adj;
		}
	      if (cmp_res == 0)
		zlog_warn
		  ("isis_dr_elect(): multiple adjacencies with same SNPA");
	    }
	  else
	    {
	      adj_dr = adj;
	    }
	}
    }

  if (!adj_dr)
    {
      /*
       * Could not find the DR - means we are alone. Resign if we were DR.
       */
      if (circuit->u.bc.is_dr[level - 1])
        retval = isis_dr_resign (circuit, level);
      list_delete (list);
      return retval;
    }

  /*
   * Now we have the DR adjacency, compare it to self
   */
  if (adj_dr->prio[level - 1] < own_prio ||
      (adj_dr->prio[level - 1] == own_prio &&
       memcmp (adj_dr->snpa, circuit->u.bc.snpa, ETH_ALEN) < 0))
    {
      adj_dr->dis_record[level - 1].dis = ISIS_IS_NOT_DIS;
      adj_dr->dis_record[level - 1].last_dis_change = time (NULL);

      /* rotate the history log */
      for (ALL_LIST_ELEMENTS_RO (list, node, adj))
        isis_check_dr_change (adj, level);

      /* We are the DR, commence DR */
      if (circuit->u.bc.is_dr[level - 1] == 0 && listcount (list) > 0)
        retval = isis_dr_commence (circuit, level);
    }
  else
    {
      /* ok we have found the DIS - lets mark the adjacency */
      /* set flag for show output */
      adj_dr->dis_record[level - 1].dis = ISIS_IS_DIS;
      adj_dr->dis_record[level - 1].last_dis_change = time (NULL);

      /* now loop through a second time to check if there has been a DIS change
       * if yes rotate the history log
       */

      for (ALL_LIST_ELEMENTS_RO (list, node, adj))
        isis_check_dr_change (adj, level);

      /*
       * We are not DR - if we were -> resign
       */
      if (circuit->u.bc.is_dr[level - 1])
        retval = isis_dr_resign (circuit, level);
    }
  list_delete (list);
  return retval;
}

int
isis_dr_resign (struct isis_circuit *circuit, int level)
{
  u_char id[ISIS_SYS_ID_LEN + 2];
  struct timeval tv = {0,0};

  zlog_debug ("isis_dr_resign l%d", level);

  circuit->u.bc.is_dr[level - 1] = 0;
  circuit->u.bc.run_dr_elect[level - 1] = 0;

  if(circuit->u.bc.pTimerRunDr[level - 1])
  {
	  taskTimerDel(circuit->u.bc.pTimerRunDr[level - 1]);
	  circuit->u.bc.pTimerRunDr[level - 1] = NULL;
  }

  if(circuit->u.bc.pTimerRefreshPseudoLsp[level - 1])
  {
	  taskTimerDel(circuit->u.bc.pTimerRefreshPseudoLsp[level - 1]);
	  circuit->u.bc.pTimerRefreshPseudoLsp[level - 1] = NULL;
  }

  circuit->lsp_regenerate_pending[level - 1] = 0;

  memcpy (id, isis->sysid, ISIS_SYS_ID_LEN);
  LSP_PSEUDO_ID (id) = circuit->circuit_id;
  LSP_FRAGMENT (id) = 0;
  lsp_purge_pseudo (id, circuit, level);

  if (level == 1)
    {
      memset (circuit->u.bc.l1_desig_is, 0, ISIS_SYS_ID_LEN + 1);

      if(circuit->pTimerSendCsnp[0])
      {
    	  taskTimerDel(circuit->pTimerSendCsnp[0]);
    	  circuit->pTimerSendCsnp[0] = NULL;
      }

      tv.tv_sec = 2 * circuit->hello_interval[0];
      circuit->u.bc.pTimerRunDr[0] = taskTimerSet(isisRunDrL1, tv, 0, (void *)circuit);

      tv.tv_sec = circuit->psnp_interval[level - 1];
      circuit->pTimerSendPsnp[0] = taskTimerSet(sendL1Psnp, tv, 0, (void *)circuit);
    }
  else
    {
      memset (circuit->u.bc.l2_desig_is, 0, ISIS_SYS_ID_LEN + 1);

      if(circuit->pTimerSendCsnp[1])
      {
    	  taskTimerDel(circuit->pTimerSendCsnp[1]);
    	  circuit->pTimerSendCsnp[1] = NULL;
      }

      tv.tv_sec = 2 * circuit->hello_interval[1];
      circuit->u.bc.pTimerRunDr[1] = taskTimerSet(isisRunDrL2, tv, 0, (void *)circuit);

      tv.tv_sec = circuit->psnp_interval[level - 1];
      circuit->pTimerSendPsnp[1] = taskTimerSet(sendL2Psnp, tv, 0, (void *)circuit);
    }

  tv.tv_sec = 0;
  taskTimerSet(isisEventDisStatusChange, tv, 0, (void *)circuit);

  return ISIS_OK;
}

int
isis_dr_commence (struct isis_circuit *circuit, int level)
{
  u_char old_dr[ISIS_SYS_ID_LEN + 2];
  struct timeval tv = {0,0};

  if (isis->debugs & DEBUG_EVENTS)
    zlog_debug ("isis_dr_commence l%d", level);

  /* Lets keep a pause in DR election */
  circuit->u.bc.run_dr_elect[level - 1] = 0;
  if (level == 1)
  {
		tv.tv_sec = 2 * circuit->hello_interval[0];
		circuit->u.bc.pTimerRunDr[0] = taskTimerSet(isisRunDrL1, tv, 0, (void *)circuit);
	}
  else
  {
    tv.tv_sec = 2 * circuit->hello_interval[1];
    circuit->u.bc.pTimerRunDr[1] = taskTimerSet(isisRunDrL2, tv, 0, (void *)circuit);
  }
  circuit->u.bc.is_dr[level - 1] = 1;

  if (level == 1)
    {
      memcpy (old_dr, circuit->u.bc.l1_desig_is, ISIS_SYS_ID_LEN + 1);
      LSP_FRAGMENT (old_dr) = 0;
      if (LSP_PSEUDO_ID (old_dr))
	{
	  /* there was a dr elected, purge its LSPs from the db */
	  lsp_purge_pseudo (old_dr, circuit, level);
	}
      memcpy (circuit->u.bc.l1_desig_is, isis->sysid, ISIS_SYS_ID_LEN);
      *(circuit->u.bc.l1_desig_is + ISIS_SYS_ID_LEN) = circuit->circuit_id;

      assert (circuit->circuit_id);	/* must be non-zero */
      /*    if (circuit->t_send_l1_psnp)
         thread_cancel (circuit->t_send_l1_psnp); */
      lsp_generate_pseudo (circuit, 1);

      if(circuit->u.bc.pTimerRunDr[0])
      {
    	  taskTimerDel(circuit->u.bc.pTimerRunDr[0]);
    	  circuit->u.bc.pTimerRunDr[0] = NULL;
      }

      tv.tv_sec = 2 * circuit->hello_interval[0];
      circuit->u.bc.pTimerRunDr[0] = taskTimerSet(isisRunDrL1, tv, 0, (void *)circuit);

      tv.tv_sec = circuit->csnp_interval[level - 1];
      circuit->pTimerSendCsnp[0] = taskTimerSet(sendL1Csnp, tv, 0, (void *)circuit);

    }
  else
    {
      memcpy (old_dr, circuit->u.bc.l2_desig_is, ISIS_SYS_ID_LEN + 1);
      LSP_FRAGMENT (old_dr) = 0;
      if (LSP_PSEUDO_ID (old_dr))
	{
	  /* there was a dr elected, purge its LSPs from the db */
	  lsp_purge_pseudo (old_dr, circuit, level);
	}
      memcpy (circuit->u.bc.l2_desig_is, isis->sysid, ISIS_SYS_ID_LEN);
      *(circuit->u.bc.l2_desig_is + ISIS_SYS_ID_LEN) = circuit->circuit_id;

      assert (circuit->circuit_id);	/* must be non-zero */
      /*    if (circuit->t_send_l1_psnp)
         thread_cancel (circuit->t_send_l1_psnp); */
      lsp_generate_pseudo (circuit, 2);

      if(circuit->u.bc.pTimerRunDr[0])
      {
    	  taskTimerDel(circuit->u.bc.pTimerRunDr[1]);
    	  circuit->u.bc.pTimerRunDr[0] = NULL;
      }

      tv.tv_sec = 2 * circuit->hello_interval[1];
      circuit->u.bc.pTimerRunDr[1] = taskTimerSet(isisRunDrL2, tv, 0, (void *)circuit);

      tv.tv_sec = circuit->csnp_interval[level - 1];
      circuit->pTimerSendCsnp[1] = taskTimerSet(sendL2Csnp, tv, 0, (void *)circuit);
    }

  tv.tv_sec = 0;
  taskTimerSet(isisEventDisStatusChange, tv, 0, (void *)circuit);

  return ISIS_OK;
}
