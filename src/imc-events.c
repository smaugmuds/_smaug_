/*
 * IMC2 - an inter-mud communications protocol
 *
 * imc-events.c: IMC event handling
 *
 * Copyright (C) 1996,1997 Oliver Jowett <oliver@jowett.manawatu.planet.co.nz>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, write to the
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>

#include "imc.h"

/*
 * I needed to split up imc.c (2600 lines and counting..) so this file now
 * contains:
 *
 * - event support functions
 * - event callbacks
 */


/*
 *  event support functions
 */

/* we use size-limited recycle lists, since events will be being
 * queued/unqueued quite often
 */

imc_event *imc_event_list, *imc_event_free;
static int event_freecount;

imc_event *imc_new_event(void)
{
  imc_event *p;

  if (imc_event_free)
  {
    p=imc_event_free;
    imc_event_free=p->next;
    event_freecount--;
  }
  else
    p=imc_malloc(sizeof(imc_event));

  p->when=0;
  p->callback=NULL;
  p->data=NULL;
  p->next=NULL;
  p->timed=0;

  return p;
}

void imc_free_event(imc_event *p)
{
  if (event_freecount>10) /* pick a number, any number */
  {
    imc_free(p, sizeof(imc_event));
  }
  else
  {
    p->next=imc_event_free;
    imc_event_free=p;
    event_freecount++;
  }
}

void imc_add_event(int when, void (*callback)(void *), void *data, int timed)
{
  imc_event *p, *scan, **last;

  p=imc_new_event();
  p->when=imc_now+when;
  p->callback=callback;
  p->data=data;
  p->timed=timed;

  for (last=&imc_event_list, scan=*last;
       scan;
       last=&scan->next, scan=*last)
    if (scan->when >= p->when)
      break;

  p->next=scan;
  *last=p;
}

void imc_cancel_event(void (*callback)(void *), void *data)
{
  imc_event *p, *p_next, **last;

  for (last=&imc_event_list, p=*last; p; p=p_next)
  {
    p_next=p->next;

    if ((!callback || p->callback==callback) && p->data==data)
    {
      *last=p_next;
      imc_free_event(p);
    }
    else
      last=&p->next;
  }
}

void imc_run_events(time_t newtime)
{
  imc_event *p;
  void (*callback)(void *);
  void *data;

  while(imc_event_list)
  {
    p=imc_event_list;

    if (p->when > newtime)
      break;

    imc_event_list=p->next;
    callback=p->callback;
    data=p->data;
    imc_now=p->when;

    imc_free_event(p);

    if (callback)
      (*callback)(data);
    else
      imc_logerror("imc_run_events: NULL callback");
  }

  imc_now=newtime;
}

int imc_next_event(void (*callback)(void *), void *data)
{
  imc_event *p;

  for (p=imc_event_list; p; p=p->next)
    if (p->callback==callback && p->data==data)
      return p->when - imc_now;

  return -1;
}



/*
 *  Events
 */

void ev_imc_optimize(void *data) 
{

	/* This one's mine! This will do the switch to the less trafficky hub, once
	each time imc reloads, and then each 24 hours. -- Scion */
	
	if (!imc_is_router)  /* don't wanna do it if we -are- a hub :) */
	{
		imc_logstring("Auto-optimizing hub connections.");
		
		if (strcmp(global_hubname, "NULL")) { /* we got a hub, let's connect to it. */
            		imc_send_autoconnect(global_hubname);
			strcpy(global_hubname, "NULL"); /* erase the record of the hub so we can start over */
		}
	}
}
/* need optimize poll as seperate event - shogar */
void ev_imc_pollforhub(void *data)
{
	if (!imc_is_router)  /* don't wanna do it if we -are- a hub :) */
	{
		imc_send_info_request();
		imc_add_event(3600, ev_imc_pollforhub, NULL, 1); /* every 12 hours */
		imc_add_event(3660, ev_imc_optimize, NULL, 1); /* give em a minutes to reply */
	}
}

/* expire the imc_reminfo entry pointed at by data */
void ev_expire_reminfo(void *data)
{
  imc_reminfo *r=(imc_reminfo *)data;

  r->type=IMC_REMINFO_EXPIRED;
}

/* drop the imc_reminfo entry */
void ev_drop_reminfo(void *data)
{
  imc_cancel_event(NULL, data);
  imc_delete_reminfo((imc_reminfo *)data);
}

/* send a keepalive, and queue the next keepalive event */
void ev_keepalive(void *data)
{
  imc_send_keepalive();
}
void ev_request_keepalive(void *data)
{
	imc_request_keepalive();
}

/* maybe shrink the input buffer of the connection 'data' */
void ev_shrink_input(void *data)
{
  imc_connect *c=(imc_connect *)data;
  int len;
  int newsize;
  void *newbuf;

  len=strlen(c->inbuf)+1;
  newsize=c->insize/2;

  if (len<=newsize)
  {
    if (newsize<IMC_MINBUF) /* huh? should never happen.. */
      return;

    /* shrink the buffer one step */
    newbuf=imc_malloc(newsize);
    strcpy(newbuf, c->inbuf);
    imc_free(c->inbuf, c->insize);
    c->inbuf=newbuf;
    c->insize=newsize;

    if (newsize<=IMC_MINBUF)
      return;
  }
  
  imc_add_event(IMC_SHRINKTIME, ev_shrink_input, c, 0);
}

/* maybe shrink the output buffer of the connection 'data' */
void ev_shrink_output(void *data)
{
  imc_connect *c=(imc_connect *)data;
  int len;
  int newsize;
  void *newbuf;

  len=strlen(c->outbuf)+1;
  newsize=c->outsize/2;

  if (len<=newsize)
  {
    if (newsize<IMC_MINBUF) /* huh? should never happen.. */
      return;

    /* shrink the buffer one step */
    newbuf=imc_malloc(newsize);
    strcpy(newbuf, c->outbuf);
    imc_free(c->outbuf, c->outsize);
    c->outbuf=newbuf;
    c->outsize=newsize;

    if (newsize<=IMC_MINBUF)
      return;
  }
  
  imc_add_event(IMC_SHRINKTIME, ev_shrink_output, c, 0);
}

/* try a reconnect to the given imc_info */
void ev_reconnect(void *data)
{
  imc_info *info=(imc_info *)data;

  if (!info->connection) {
	  if ((info->flags & IMC_RECONNECT) || (info->flags & IMC_HUB) || (info->flags & IMC_MAIN_HUB)) {
		  if (!imc_connect_to(info->name)) {
      /* routers should never do reconnects to muds - shogar */
      /* and limit RECONNECTs to 5 attempts until successful connect */
      /* or they connect to us until we reboot, then we give them a second */
      /* chance - shogar */
      	
			  if(!imc_is_router || (info->flags & IMC_HUB)) { 
				  if(info->connect_attempts < 4)
					  imc_setup_reconnect(info);
				  else { /* Set them to "dead_hub" so they get deleted at next reboot. -- Scion */
					info->flags=imc_flagvalue("dead_hub", imc_connection_flags);
			  
				  }		  
			  }	  
		  }  
	  }
  }
}

/* handle a spam event for counter 1 */
void ev_spam1(void *data)
{
  imc_connect *c=(imc_connect *)data;

  if (c->spamcounter1 > IMC_SPAM1MAX)
  {
    if (c->spamtime1 < 0 || ++c->spamtime1 >= IMC_SPAM1TIME)
      c->spamtime1=-IMC_SPAM1TIME;
  }
  else
  {
    if (c->spamtime1<0)
      c->spamtime1++;
  }

  c->spamcounter1=0;
  if (c->spamtime1>0)
    imc_add_event(IMC_SPAM1INTERVAL, ev_spam1, c, 0);
  else if (c->spamtime1<0)
    imc_add_event(IMC_SPAM1INTERVAL, ev_spam1, c, 1);
}

/* handle a spam event for counter 2 */
void ev_spam2(void *data)
{
  imc_connect *c=(imc_connect *)data;

  if (c->spamcounter2 > IMC_SPAM2MAX)
  {
    if (c->spamtime2 < 0 || ++c->spamtime2 >= IMC_SPAM2TIME)
      c->spamtime2=-IMC_SPAM2TIME;
  }
  else
    if (c->spamtime2<0)
      c->spamtime2++;

  c->spamcounter2=0;
  if (c->spamtime2>0)
    imc_add_event(IMC_SPAM1INTERVAL, ev_spam2, c, 0);
  else if (c->spamtime2<0)
    imc_add_event(IMC_SPAM1INTERVAL, ev_spam2, c, 1);
}

