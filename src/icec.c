/*
 * IMC2 - an inter-mud communications protocol
 *
 * icec.c: IMC-channel-extensions (ICE) client code
 *
 * Copyright (C) 1997 Oliver Jowett <oliver@jowett.manawatu.planet.co.nz>
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

#include "imc.h"
#include "icec.h"

void ev_icec_timeout(void *data);

ice_channel *icec_channel_list;

ice_channel *icec_findchannel(const char *name)
{
  ice_channel *c;

  for (c=icec_channel_list; c; c=c->next)
    if (!strcasecmp(c->name, name))
      return c;

  return NULL;
}

int (*icec_recv_chain)(const imc_packet *p, int bcast);
void icec_shutdown(void)
{
	ice_channel *ic;

	while((ic=icec_channel_list))
	{
		icec_localfree(ic);
		imc_strfree(ic->name);
		imc_strfree(ic->owner);
		imc_strfree(ic->operators);
		imc_strfree(ic->invited);
		imc_strfree(ic->excluded);
		icec_channel_list=ic->next;
		imc_free(ic,sizeof(ic));
	}
}
int icec_recv(const imc_packet *p, int bcast)
{
  /* redirected msg */
  if (!strcasecmp(p->type, "ice-msg-r"))
  {
    icec_recv_msg_r(p->from,
		    imc_getkey(&p->data, "realfrom", ""),
		    imc_getkey(&p->data, "channel", ""),
		    imc_getkey(&p->data, "text", ""),
		    imc_getkeyi(&p->data, "emote", 0));
    return 1;
  }
  else if (!strcasecmp(p->type, "ice-msg-b"))
  {
    icec_recv_msg_b(p->from,
		    imc_getkey(&p->data, "channel", ""),
		    imc_getkey(&p->data, "text", ""),
		    imc_getkeyi(&p->data, "emote", 0));
    return 1;
    
  }
  else if (!strcasecmp(p->type, "ice-update"))
  {
    icec_recv_update(p->from,
		     imc_getkey(&p->data, "channel", ""),
		     imc_getkey(&p->data, "owner", ""),
		     imc_getkey(&p->data, "operators", ""),
                     imc_getkey(&p->data, "policy", ""),
		     imc_getkey(&p->data, "invited", "" ),
		     imc_getkey(&p->data, "excluded", ""));
    return 1;
  }
  else if (!strcasecmp(p->type, "ice-destroy"))
  {
    icec_recv_destroy(p->from, imc_getkey(&p->data, "channel", ""));
    return 1;
  }
  else
    return 0;
}

void icec_recv_msg_r(const char *from,
		     const char *realfrom,
		     const char *chan,
		     const char *txt,
		     int emote)
{
  ice_channel *c;
  const char *mud;

  mud=imc_mudof(from);

  /* forged? */
  if (!strchr(chan, ':') ||
      strcasecmp(mud, ice_mudof(chan)))
    return;

  c=icec_findchannel(chan);
  if (!c)
    return;

  if (!c->local || c->policy!=ICE_PRIVATE)
    return;

  /*  we assume that anything redirected is automatically audible - since we
   *  trust the ICEd
   */

  icec_showchannel(c, realfrom, txt, emote);
}

void icec_recv_msg_b(const char *from,
		     const char *chan,
		     const char *txt,
		     int emote)
{
  ice_channel *c;

  c=icec_findchannel(chan);
  if (!c)
    return;

  if (!c->local || c->policy==ICE_PRIVATE)
    return;
  
  if (!ice_audible(c, from))
    return;
  
  icec_showchannel(c, from, txt, emote);
}

void icec_recv_update(const char *from,
		      const char *chan,
		      const char *owner,
		      const char *operators,
		      const char *policy,
		      const char *invited,
		      const char *excluded)
{
  ice_channel *c;
  const char *mud;

  mud=imc_mudof(from);

  /* forged? */
  if (!strchr(chan, ':') ||
      strcasecmp(mud, ice_mudof(chan)))
    return;

  c=icec_findchannel(chan);
  if (!c)
  {
    c=imc_malloc(sizeof(*c));
    c->name=imc_strdup(chan);
    c->owner=imc_strdup(owner);
    c->operators=imc_strdup(operators);
    c->invited=imc_strdup(invited);
    c->excluded=imc_strdup(excluded);
    c->local=NULL;
    c->active=NULL;

    c->next=icec_channel_list;
    icec_channel_list=c;
  }
  else
  {
    imc_strfree(c->owner);
    imc_strfree(c->operators);
    imc_strfree(c->invited);
    imc_strfree(c->excluded);
    c->name=imc_strdup(chan);
    c->owner=imc_strdup(owner);
    c->operators=imc_strdup(operators);
    c->invited=imc_strdup(invited);
    c->excluded=imc_strdup(excluded);
  }
    
  if (!strcasecmp(policy, "open"))
    c->policy=ICE_OPEN;
  else if (!strcasecmp(policy, "closed"))
    c->policy=ICE_CLOSED;
  else
    c->policy=ICE_PRIVATE;

  if (c->local && !ice_audible(c, imc_name))
    icec_localfree(c);

  icec_notify_update(c);

  imc_cancel_event(ev_icec_timeout, c);
  imc_add_event(ICEC_TIMEOUT, ev_icec_timeout, c, 0);
}

void icec_recv_destroy(const char *from, const char *channel)
{
  ice_channel *c;
  const char *mud;

  mud=imc_mudof(from);

  if (!strchr(channel, ':') ||
      strcasecmp(mud, ice_mudof(channel)))
    return;

  c=icec_findchannel(channel);
  if (!c)
    return;

  if (c==icec_channel_list)
    icec_channel_list=c->next;
  else
  {
    ice_channel *p;

    for (p=icec_channel_list; p; p=p->next)
      if (p->next == c)
	break;

    if (p)
      p->next=c->next;
  }

  icec_localfree(c);
  
  imc_strfree(c->name);
  imc_strfree(c->owner);
  imc_strfree(c->operators);
  imc_strfree(c->invited);
  imc_strfree(c->excluded);
}

const char *icec_command(const char *from, const char *arg)
{
  char cmd[IMC_NAME_LENGTH];
  char chan[IMC_NAME_LENGTH];
  char data[IMC_DATA_LENGTH];
  const char *p;
  imc_packet out;
  ice_channel *c;
  
  p=imc_getarg(arg, cmd, IMC_NAME_LENGTH);
  p=imc_getarg(p, chan, IMC_NAME_LENGTH);
  strcpy(data, p);

  if (!cmd[0] || !chan[0])
    return "Syntax: icommand <command> <node:channel> [<data..>]";

  p=strchr(chan, ':');
  if (!p)
  {
    c=icec_findlchannel(chan);
    if (c)
      strcpy(chan, c->name);
  }

  sprintf(out.to, "ICE@%s", ice_mudof(chan));
  strcpy(out.type, "ice-cmd");
  strcpy(out.from, from);
  imc_initdata(&out.data);
  imc_addkey(&out.data, "channel", chan);
  imc_addkey(&out.data, "command", cmd);
  imc_addkey(&out.data, "data", data);

  imc_send(&out);
  imc_freedata(&out.data);

  return "Command sent.";
}

void icec_sendmessage(ice_channel *c, const char *name, const char *text, int emote)
{
  imc_packet out;
  
  strcpy(out.from, name);
  imc_initdata(&out.data);
  imc_addkey(&out.data, "channel", c->name);
  imc_addkey(&out.data, "text", text);
  imc_addkeyi(&out.data, "emote", emote);
  
  /* send a message out on a channel */

  if (c->policy == ICE_PRIVATE)
  {
    /* send to the daemon to distribute */
    /* send locally */
    icec_showchannel(c, imc_makename(name, imc_name), text, emote);
    
    strcpy(out.type, "ice-msg-p");
    sprintf(out.to, "ICE@%s", ice_mudof(c->name));
  }
  else
  {
    /* broadcast */
    strcpy(out.type, "ice-msg-b");
    strcpy(out.to, "*@*");
  }

  imc_send(&out);
  imc_freedata(&out.data);
}

void ev_icec_firstrefresh(void *dummy)
{
  imc_packet out;

  if (imc_active < IA_UP)
    return;
  
  strcpy(out.from, "*");
  strcpy(out.to, "ICE@*");
  strcpy(out.type, "ice-refresh");
  imc_initdata(&out.data);
  imc_addkey(&out.data, "channel", "*");
  imc_send(&out);
  imc_freedata(&out.data);
}

void ev_icec_timeout(void *data)
{
  ice_channel *c=data;

  if (c==icec_channel_list)
    icec_channel_list=c->next;
  else
  {
    ice_channel *p;

    for (p=icec_channel_list; p; p=p->next)
      if (p->next == c)
	break;

    if (p)
      p->next=c->next;
  }

  icec_localfree(c);
  
  imc_strfree(c->name);
  imc_strfree(c->owner);
  imc_strfree(c->operators);
  imc_strfree(c->invited);
  imc_strfree(c->excluded);
}

/* global init */
void icec_init(void)
{
  imc_logstring("ICE client starting.");

  icec_recv_chain=imc_recv_hook;
  imc_recv_hook=icec_recv;

  imc_add_event(60, ev_icec_firstrefresh, NULL, 1);

  icec_load_channels();
}
