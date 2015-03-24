/*
 * IMC2 - an inter-mud communications protocol
 *
 * iced.c: IMC-channel-extensions (ICE) daemon code
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
#include "iced.h"

ice_channel *iced_channel_list;
static ice_channel *next_refresh, *next_list;

imc_char_data iced_char =
{
  "ICE", /* name */
  0,     /* invis */
  -1,    /* level */
  0      /* wizi */
};

static void fixactive(ice_channel *c)
{
  char arg[IMC_NAME_LENGTH];
  const char *p;
  char buf[IMC_DATA_LENGTH];
  
  if (c->policy != ICE_PRIVATE)
  {
    imc_strfree(c->active);
    c->active=imc_strdup("");
    return;
  }

  buf[0]=0;
  p=imc_getarg(c->invited, arg, IMC_NAME_LENGTH);
  while (arg[0])
  {
    const char *mud=imc_mudof(arg);

    if (!imc_hasname(buf, mud))
    {
      if (buf[0])
	strcat(buf, " ");

      strcat(buf, mud);
    }

    p=imc_getarg(p, arg, IMC_NAME_LENGTH);
  }

  p=imc_getarg(c->operators, arg, IMC_NAME_LENGTH);
  while (arg[0])
  {
    const char *mud=imc_mudof(arg);

    if (!imc_hasname(buf, mud))
    {
      if (buf[0])
	strcat(buf, " ");

      strcat(buf, mud);
    }

    p=imc_getarg(p, arg, IMC_NAME_LENGTH);
  }

  if (!imc_hasname(buf, imc_mudof(c->owner)))
  {
    if (buf[0])
      strcat(buf, " ");

    strcat(buf, imc_mudof(c->owner));
  }
  
  imc_strfree(c->active);
  c->active=imc_strdup(buf);
}

void iced_save_channels(void)
{
  ice_channel *c;
  FILE *fp;
  char name[IMC_DATA_LENGTH];

  strcpy(name, imc_prefix);
  strcat(name, "iced");

  fp=fopen(name, "w");
  if (!fp)
  {
    imc_logerror("Can't write to %s", name);
    return;
  }
  
  for (c=iced_channel_list; c; c=c->next)
  {
    /* save */
    fprintf(fp,
	    "%s %s %d\n"
	    "op %s\n"
	    "invite %s\n"
	    "exclude %s\n",
	    c->name, c->owner, c->policy,
	    c->operators[0] ? c->operators : "none",
	    c->invited[0] ? c->invited : "none",
	    c->excluded[0] ? c->excluded : "none");
  }

  fclose(fp);
}

void iced_load_channels(void)
{
  FILE *fp;
  char name[IMC_DATA_LENGTH];
  char buf1[IMC_DATA_LENGTH];
  char buf2[IMC_DATA_LENGTH];
  char buf3[IMC_DATA_LENGTH];
  char buf4[IMC_DATA_LENGTH];
  char buf5[IMC_DATA_LENGTH];
  int p;

  strcpy(name, imc_prefix);
  strcat(name, "iced");

  fp=fopen(name, "r");
  if (!fp)
  {
    imc_logerror("Can't open %s", name);
    return;
  }

  while (fscanf(fp,
		"%s %s %d\n"
		"op %[^\n]\n"
		"invite %[^\n]\n"
		"exclude %[^\n]\n", buf1, buf2, &p, buf3, buf4, buf5) == 6)
  {
    ice_channel *c=imc_malloc(sizeof(*c));

    if (!strcmp(buf3, "none"))
      buf3[0]=0;
    if (!strcmp(buf4, "none"))
      buf4[0]=0;
    if (!strcmp(buf5, "none"))
      buf5[0]=0;
    
    c->local=NULL;
    c->name=imc_strdup(buf1);
    c->owner=imc_strdup(buf2);
    c->operators=imc_strdup(buf3);
    c->invited=imc_strdup(buf4);
    c->excluded=imc_strdup(buf5);
    c->active=imc_strdup("");
    c->policy=p;
    
    c->next=iced_channel_list;
    iced_channel_list=c;

    fixactive(c);

    imc_logstring("ICEd: loaded %s (%s) owned by %s",
		  c->name,
		  c->policy == ICE_OPEN ? "open" :
		  c->policy == ICE_CLOSED ? "closed" : "private",
                  c->owner);
  }

  fclose(fp);
}

ice_channel *iced_findchannel(const char *name)
{
  ice_channel *c;

  for (c=iced_channel_list; c; c=c->next)
    if (!strcasecmp(c->name, name))
      return c;

  return NULL;
}

void iced_gannounce(const char *fmt, ...)
{
  char buf[IMC_DATA_LENGTH];
  va_list ap;

  strcpy(buf, "announces: ");
  
  va_start(ap, fmt);
  vsnprintf(buf+strlen(buf), IMC_DATA_LENGTH - 1, fmt, ap);
  va_end(ap);

  imc_send_emote(&iced_char, 15, buf, "*");
}

void iced_privmsg(ice_channel *c, imc_packet *out, const char *exclude)
{
  const char *p;
  char arg[IMC_NAME_LENGTH];
  
  p=imc_getarg(c->active, arg, IMC_NAME_LENGTH);
  while(arg[0])
  {
    if (!exclude || strcasecmp(arg, exclude))
    {
      sprintf(out->to, "*@%s", arg);
      imc_send(out);
    }

    p=imc_getarg(p, arg, IMC_NAME_LENGTH);
  }
}

void iced_announce(ice_channel *c, const char *fmt, ...)
{
  va_list ap;
  char buf[IMC_DATA_LENGTH];

  strcpy(buf, "announces: ");
  
  va_start(ap, fmt);
  vsnprintf(buf+strlen(buf), (IMC_DATA_LENGTH-strlen(buf)) - 1, fmt, ap);
  va_end(ap);

  if (c->policy == ICE_PRIVATE)
  {
    imc_packet out;
    
    strcpy(out.from, "ICE");
    strcpy(out.to, "*");
    strcpy(out.type, "ice-msg-r");

    imc_initdata(&out.data);
    imc_addkey(&out.data, "realfrom", imc_makename("ICE", imc_name));
    imc_addkey(&out.data, "text", buf);
    imc_addkey(&out.data, "channel", c->name);
    imc_addkeyi(&out.data, "emote", 1);

    iced_privmsg(c, &out, NULL);

    imc_freedata(&out.data);
  }
  else
  {
    imc_packet out;

    strcpy(out.from, "ICE");
    strcpy(out.to, "*");
    strcpy(out.type, "ice-msg-b");

    imc_initdata(&out.data);
    imc_addkey(&out.data, "text", buf);
    imc_addkey(&out.data, "channel", c->name);
    imc_addkeyi(&out.data, "emote", 1);

    imc_send(&out);
    imc_freedata(&out.data);
  }
}

int (*iced_recv_chain)(const imc_packet *, int);

/* channel daemon hook */
int iced_recv(const imc_packet *p, int bcast)
{
  /* commands */
  if (!strcasecmp(p->type, "ice-cmd"))
  {
    iced_recv_command(p->from,
		      imc_getkey(&p->data, "channel", ""),
		      imc_getkey(&p->data, "command", ""),
		      imc_getkey(&p->data, "data", ""), 0);
    return 1;
  }
  else if (!strcasecmp(p->type, "ice-msg-p"))
  {
    /* private message to be forwarded */
    iced_recv_msg_p(p->from,
		    imc_getkey(&p->data, "channel", ""),
		    imc_getkey(&p->data, "text", ""),
		    imc_getkeyi(&p->data, "emote", 0));
    return 1;
  }
  else if (!strcasecmp(p->type, "ice-msg-b"))
  {
    /* check for misdirection */
    iced_recv_msg_b(p->from,
		    imc_getkey(&p->data, "channel", ""));
    return 1;
  }
  else if (!strcasecmp(p->type, "ice-refresh"))
  {
    iced_recv_refresh(p->from, imc_getkey(&p->data, "channel", "*"));
    return 1;
  }
  else if (!strcasecmp(p->type, "ice-join-request"))
  {
    iced_recv_join(p->from, imc_getkey(&p->data, "channel", ""));
    return 1;
  }
  else if (!strcasecmp(p->type, "ice-leave-request"))
  {
    iced_recv_leave(p->from, imc_getkey(&p->data, "channel", ""));
    return 1;
  }
  
  if (iced_recv_chain)
    return (*iced_recv_chain)(p, bcast);
  else
    return 0;
}

struct {
  char *name;
  int level;  /* 0=anyone, 1=op only, 2=owner only */
  void (*cmdfn)(ice_channel *c, const char *cname, const char *from, const char *data);
  int needchan;
} iced_cmdtable[] =
{
  { "destroy",    2, iced_destroy,   1 },
  { "policy",     2, iced_policy,    1 },
  { "addop",      2, iced_addop,     1 },
  { "removeop",   2, iced_removeop,  1 },
  { "invite",     1, iced_invite,    1 },
  { "uninvite",   1, iced_uninvite,  1 },
  { "exclude",    1, iced_exclude,   1 },
  { "unexclude",  1, iced_unexclude, 1 },
  { "create",     0, iced_create,    0 },
  { "refresh",    0, iced_refresh,   0 },
  { "list",       0, iced_list,      0 },
  { NULL,         0, NULL              }
};

int iced_getaccess(ice_channel *c, const char *from)
{
  if (!c)
    return 0;
  else if (!strcasecmp(from, c->owner))
    return 2;
  else if (imc_hasname(c->operators, from))
    return 1;
  else
    return 0;
}

void iced_recv_command(const char *from, const char *chan, const char *cmd,
		       const char *data, int override)
{
  ice_channel *c;
  int i;

  if (imc_isignored(from))
  {
    imc_sendignore(from);
    return;
  }
  
  for (i=0; iced_cmdtable[i].name; i++)
    if (!strcasecmp(iced_cmdtable[i].name, cmd))
      break;

  if (!iced_cmdtable[i].name)
  {
    imc_send_tell(&iced_char, from, "Unknown command. Send LIST for a list.", 1);
    return;
  }

  c=iced_findchannel(chan);
  if (!c && iced_cmdtable[i].needchan)
  {
    imc_send_tell(NULL, from, "No such channel. Syntax: icommand <command> <channel> [<data..>]", 1);
    return;
  }

  if (!override && (iced_getaccess(c, from) < iced_cmdtable[i].level))
  {
    imc_send_tell(&iced_char, from, "Insufficient security to do that.", 1);
    return;
  }

  (*iced_cmdtable[i].cmdfn)(c, chan, from, data);
  iced_save_channels();
}

/* list commands */
void iced_list(ice_channel *c, const char *cname, const char *from, const char *data)
{
  char out[IMC_DATA_LENGTH];
  int i;
  int access;

  strcpy(out,
	 "Available commands:\n"
	 "Lvl Ok? Name\n");

  access=iced_getaccess(c, from);
  
  for (i=0; iced_cmdtable[i].name; i++)
  {
    sprintf(out+strlen(out),
	    " %d %s %s\n",
	    iced_cmdtable[i].level,
	    access >= iced_cmdtable[i].level ? "Yes" : "No ",
	    iced_cmdtable[i].name);
  }

  imc_send_tell(&iced_char, from, out, 1);
}

void iced_send_destroy(const char *cname, const char *to)
{
  imc_packet out;

  strcpy(out.from, "ICE");
  strcpy(out.to, to ? to : "*");
  strcpy(out.type, "ice-destroy");
  imc_initdata(&out.data);
  imc_addkey(&out.data, "channel", cname);
  imc_send(&out);
  imc_freedata(&out.data);
}

/* destroy a channel */
void iced_destroy(ice_channel *c, const char *cname, const char *from, const char *data)
{
  ice_channel *p;
  
  if (strcasecmp(data, "destroy"))
  {
    imc_send_tell(&iced_char, from, "Use 'destroy <channel> destroy' to confirm.", 1);
    return;
  }

  /* remove/free the channel from our list */
  
  if (c==iced_channel_list)
    iced_channel_list=c->next;
  else
  {
    for (p=iced_channel_list; p; p=p->next)
      if (p->next == c)
	break;

    if (!p)
      imc_logerror("%s not in channel list?!", c->name);
    else
      p->next=c->next;
  }

  if (c==next_refresh)
    next_refresh=c->next;
  if (c==next_list)
    next_list=c->next;

  imc_logstring("%s destroys channel %s", from, c->name);
  iced_gannounce("Channel %s has been destroyed by %s.", c->name, from);
  
  imc_strfree(c->name);
  imc_strfree(c->owner);
  imc_strfree(c->operators);
  imc_strfree(c->invited);
  imc_strfree(c->excluded);
  imc_strfree(c->active);
  imc_free(c, sizeof(*c));

  imc_send_tell(&iced_char, from, "Done.", 1);

  /* send destroy notification */
  iced_send_destroy(c->name, NULL);
}

/* set channel policy */
void iced_policy(ice_channel *c, const char *cname, const char *from, const char *data)
{
  if (!strcasecmp(data, "open"))
  {
    c->policy=ICE_OPEN;
    iced_announce(c, "Channel policy is now: open.");
    iced_gannounce("%s is now policy: open.", c->name);
    iced_update(c, NULL);
  }
  else if (!strcasecmp(data, "closed"))
  {
    c->policy=ICE_CLOSED;
    iced_announce(c, "Channel policy is now: closed.");
    iced_gannounce("%s is now policy: closed.", c->name);
    iced_update(c, NULL);
  }
  else if (!strcasecmp(data, "private"))
  {
    c->policy=ICE_PRIVATE;
    iced_announce(c, "Channel policy is now: private.");
    iced_gannounce("%s is now policy: private.", c->name);
    iced_update(c, NULL);
  }
  else
    imc_send_tell(&iced_char, from, "Syntax: <channel> policy [open|closed|private]", 1);
}

/* add operator */
void iced_addop(ice_channel *c, const char *cname, const char *from, const char *data)
{
  char buf[1000];
  char arg[IMC_NAME_LENGTH];

  if (!data[0])
  {
    imc_send_tell(&iced_char, from, "Syntax: addop <channel> <user@mud>", 1);
    return;
  }
  
  if (imc_hasname(c->operators, data))
  {
    imc_send_tell(&iced_char, from, "They are already an operator.", 1);
    return;
  }

  imc_getarg(data, arg, IMC_NAME_LENGTH);

  if (!strchr(arg, '@'))
  {
    imc_send_tell(&iced_char, from, "Need a full user@mud name to add.", 1);
    return;
  }
  
  imc_addname(&c->operators, arg);

  sprintf(buf, "%s is now an operator of %s.", arg, c->name);
  imc_send_tell(&iced_char, from, buf, 1);

  iced_announce(c, "%s is now an operator.", arg);
  iced_update(c, NULL);
}

/* remove operator */
void iced_removeop(ice_channel *c, const char *cname, const char *from, const char *data)
{
  char buf[1000];
  char arg[IMC_NAME_LENGTH];
  
  imc_getarg(data, arg, IMC_NAME_LENGTH);

  if (!arg[0])
  {
    imc_send_tell(&iced_char, from, "Syntax: removeop <channel> <user@mud>", 1);
    return;
  }
  
  if (!imc_hasname(c->operators, arg))
  {
    imc_send_tell(&iced_char, from, "They are not an operator.", 1);
    return;
  }

  imc_removename(&c->operators, arg);

  sprintf(buf, "%s is no longer an operator of %s.", arg, c->name);
  imc_send_tell(&iced_char, from, buf, 1);

  iced_announce(c, "%s is no longer an operator.", arg);
  iced_update(c, NULL);
}

/* invite mud or player */
void iced_invite(ice_channel *c, const char *cname, const char *from, const char *data)
{
  char buf[1000];
  char arg[IMC_NAME_LENGTH];
  
  imc_getarg(data, arg, IMC_NAME_LENGTH);

  if (!arg[0])
  {
    imc_send_tell(&iced_char, from, "Syntax: invite <channel> <user@mud>", 1);
    return;
  }
  
  if (imc_hasname(c->invited, arg))
  {
    imc_send_tell(&iced_char, from, "They are already on the invite list.", 1);
    return;
  }

  imc_addname(&c->invited, arg);

  sprintf(buf, "%s is now invited to %s.", arg, c->name);
  imc_send_tell(&iced_char, from, buf, 1);

  iced_announce(c, "%s invites %s.", from, arg);
  iced_update(c, NULL);
}

/* uninvite mud or player */
void iced_uninvite(ice_channel *c, const char *cname, const char *from, const char *data)
{
  char buf[1000];
  char arg[IMC_NAME_LENGTH];
  
  imc_getarg(data, arg, IMC_NAME_LENGTH);

  if (!arg[0])
  {
    imc_send_tell(&iced_char, from, "Syntax: uninvite <channel> <user@mud>", 1);
    return;
  }
  
  if (!imc_hasname(c->invited, arg))
  {
    imc_send_tell(&iced_char, from, "They are not currently on the invite list.", 1);
    return;
  }

  imc_removename(&c->invited, arg);

  sprintf(buf, "%s is no longer invited on %s.", arg, c->name);
  imc_send_tell(&iced_char, from, buf, 1);

  iced_announce(c, "%s uninvites %s.", from, arg);
  iced_update(c, NULL);
}

/* exclude mud or player */
void iced_exclude(ice_channel *c, const char *cname, const char *from, const char *data)
{
  char buf[1000];
  char arg[IMC_NAME_LENGTH];
  
  imc_getarg(data, arg, IMC_NAME_LENGTH);

  if (!arg[0])
  {
    imc_send_tell(&iced_char, from, "Syntax: exclude <channel> <user@mud>", 1);
    return;
  }
  
  if (imc_hasname(c->excluded, arg))
  {
    imc_send_tell(&iced_char, from, "They are already on the exclude list.", 1);
    return;
  }

  imc_addname(&c->excluded, arg);

  sprintf(buf, "%s is now excluded from %s.", arg, c->name);
  imc_send_tell(&iced_char, from, buf, 1);

  iced_announce(c, "%s excludes %s.", from, arg);
  iced_update(c, NULL);
}

/* unexclude mud or player */
void iced_unexclude(ice_channel *c, const char *cname, const char *from, const char *data)
{
  char buf[1000];
  char arg[IMC_NAME_LENGTH];
  
  imc_getarg(data, arg, IMC_NAME_LENGTH);

  if (!arg[0])
  {
    imc_send_tell(&iced_char, from, "Syntax: unexclude <channel> <user@mud>", 1);
    return;
  }
  
  if (!imc_hasname(c->excluded, arg))
  {
    imc_send_tell(&iced_char, from, "They are not on the exclude list.", 1);
    return;
  }

  imc_removename(&c->excluded, arg);

  sprintf(buf, "%s is no longer excluded from %s.", arg, c->name);
  imc_send_tell(&iced_char, from, buf, 1);

  iced_announce(c, "%s unexcludes %s.", from, arg);
  iced_update(c, NULL);
}

/* create a channel */
void iced_create(ice_channel *c, const char *cname, const char *from, const char *data)
{
  ice_channel *p;

  if (!imc_hasname(ICED_CREATORS, "*") &&
      !imc_hasname(ICED_CREATORS, from) &&
      !imc_hasname(ICED_CREATORS, imc_mudof(from)))
  {
    imc_send_tell(&iced_char, from, "You don't have permission to create channels here.", 1);
    return;
  }

  if (!cname[0])
  {
    char buf[IMC_DATA_LENGTH];
    sprintf(buf, "Syntax: create %s:<channel name>", imc_name);
    imc_send_tell(&iced_char, from, buf, 1);
    return;
  }
  
  if (!strchr(cname, ':') ||
      strcasecmp(ice_mudof(cname), imc_name))
  {
    char buf[IMC_DATA_LENGTH];
    sprintf(buf, "Channels created here must begin with %s:", imc_name);
    imc_send_tell(&iced_char, from, buf, 1);
    return;
  }
  
  if (c)
  {
    imc_send_tell(&iced_char, from, "A channel by that name already exists.", 1);
    return;
  }

  p=imc_malloc(sizeof(*p));
  p->name=imc_strdup(cname);
  p->owner=imc_strdup(from);
  p->operators=imc_strdup("");
  p->invited=imc_strdup("");
  p->excluded=imc_strdup("");
  p->active=imc_strdup("");
  p->policy=ICE_CLOSED;

  /* start refreshes as needed */
  if (!iced_channel_list)
    imc_add_event(ICED_REFRESH_TIME, ev_iced_refresh, NULL, 1);
  
  p->next=iced_channel_list;
  iced_channel_list=p;

  iced_update(p, NULL);

  imc_logstring("%s creates channel %s", from, p->name);
  iced_gannounce("Channel %s created by %s.", p->name, from);
  imc_send_tell(&iced_char, from, "Channel created.", 1);
}

/* refresh a channel */
void iced_refresh(ice_channel *c, const char *cname, const char *from, const char *data)
{
  if (!c)
  {
    if (!strcmp(cname, "*"))
    {
      if (!next_list)
      {
	next_list=iced_channel_list;
	imc_add_event(5, ev_iced_chanlist, (void *)from, 1);
      }
      else
      {
	imc_send_tell(&iced_char, from, "Refresh already in progress - try again later.", 1);
      }
    }
    else
    {
      iced_send_destroy(cname, imc_mudof(from));
      imc_send_tell(&iced_char, from, "No such channel. Destroy message sent.", 1);
    }

    return;
  }

  iced_update(c, imc_mudof(from));
  imc_send_tell(&iced_char, from, "Channel refreshed.", 1);
}

/* private message - for forwarding */
void iced_recv_msg_p(const char *from, const char *chan, const char *txt,
		     int emote)
{
  ice_channel *c;
  imc_packet out;
  char temp[IMC_NAME_LENGTH];
  
  if (imc_isignored(from))
  {
    imc_sendignore(from);
    return;
  }

  c=iced_findchannel(chan);
  if (!c)
  {
    imc_send_tell(&iced_char, from, "You're trying to talk on a nonexistant channel.", 1);
    iced_send_destroy(chan, imc_mudof(from));
    return;
  }

  if (!ice_audible(c, from))
  {
    imc_send_tell(&iced_char, from, "You're trying to talk on a channel that you don't have access to.", 1);
    iced_update(c, imc_mudof(from));
    return;
  }

  if (c->policy != ICE_PRIVATE)
  {
    imc_send_tell(&iced_char, from, "Misconfiguration, sending PtP message on nonprivate channel. Try again.", 1);
    iced_update(c, imc_mudof(from));
  }

  strcpy(out.from, "ICE");
  strcpy(out.type, "ice-msg-r"); /* redirect */

  imc_initdata(&out.data);
  imc_addkey(&out.data, "realfrom", from);
  imc_addkey(&out.data, "channel", chan);
  imc_addkey(&out.data, "text", txt);
  imc_addkeyi(&out.data, "emote", emote);

  strcpy(temp, imc_mudof(from)); /* since we do several imc_sends */
  iced_privmsg(c, &out, temp);

  imc_freedata(&out.data);
}

/* broadcast message - complain if its a private channel */
void iced_recv_msg_b(const char *from, const char *chan)
{
  ice_channel *c;

  if (!strchr(chan, ':') ||
      strcasecmp(ice_mudof(chan), imc_name))
    return;
  
  c=iced_findchannel(chan);
  if (!c)
  {
    imc_send_tell(&iced_char, from, "You're trying to talk on a nonexistant channel.", 1);
    iced_send_destroy(chan, imc_mudof(from));
    return;
  }

  if (!ice_audible(c, from))
  {
    imc_send_tell(&iced_char, from, "You're trying to talk on a channel that you don't have access to.", 1);
    iced_update(c, imc_mudof(from));
    return;
  }
  
  if (c->policy == ICE_PRIVATE)
  {
    imc_send_tell(&iced_char, from, "Misconfiguration, sending broadcast message on private channel. Try again.", 1);
    iced_update(c, imc_mudof(from));
  }

  /* do nothing otherwise */
}

/* refresh request */
void iced_recv_refresh(const char *from, const char *chan)
{
  if (!strcmp(chan, "*"))
  {
    if (!next_list)
    {
      next_list=iced_channel_list;
      imc_add_event(5, ev_iced_chanlist, (void *)from, 1);
    }
    else
      return; /* try again later */
  }
  else
  {
    ice_channel *c=iced_findchannel(chan);
    if (!c)
    {
      iced_send_destroy(chan, imc_mudof(from));
      return; /* no such channel */
    }

    iced_update(c, imc_mudof(from));
  }
}

void iced_recv_join(const char *from, const char *chan)
{
  ice_channel *c;
  const char *mud;

  c=iced_findchannel(chan);
  if (!c)
    return;
    
  mud=imc_mudof(from);
  
  if (!ice_audible(c, mud))
    return;
    
  if (imc_hasname(c->active, mud))
    return;
    
  imc_addname(&c->active, mud);
}

void iced_recv_leave(const char *from, const char *chan)
{
  ice_channel *c;
  const char *mud;

  c=iced_findchannel(chan);
  if (!c)
    return;
    
  mud=imc_mudof(from);
  
  if (!imc_hasname(c->active, mud))
    return;

  imc_removename(&c->active, mud);
}
    
/* update a channel */
void iced_update(ice_channel *c, const char *to)
{
  imc_packet out;

  fixactive(c);
  
  strcpy(out.from, "ICE");
  strcpy(out.to, to ? to : "*@*");
  strcpy(out.type, "ice-update");

  imc_initdata(&out.data);
  imc_addkey(&out.data, "channel", c->name);
  imc_addkey(&out.data, "owner", c->owner);
  imc_addkey(&out.data, "operators", c->operators);
  imc_addkey(&out.data, "policy", c->policy == ICE_OPEN ? "open" :
			 c->policy == ICE_CLOSED ? "closed" : "private");
  imc_addkey(&out.data, "invited", c->invited);
  imc_addkey(&out.data, "excluded", c->excluded);

  imc_send(&out);
  imc_freedata(&out.data);
}

/* spam out all channel updates at once */
void ev_iced_chanlist(void *data)
{
  char *to=data;

  if (!next_list)
    return;

  iced_update(next_list, to);
  imc_add_event(5, ev_iced_chanlist, data, 1);

  next_list=next_list->next;
}
  
/* generate a channel listing */
void ev_iced_refresh(void *dummy)
{

  if (imc_active < IA_UP)
    return;
  
  if (!next_refresh)
    next_refresh=iced_channel_list;

  if (!next_refresh)
    return;

  iced_update(next_refresh, NULL);
  next_refresh=next_refresh->next;
  imc_add_event(ICED_REFRESH_TIME, ev_iced_refresh, NULL, 1);
}

/* global init */
void iced_init(void)
{
  imc_logstring("ICE daemon starting.");

  iced_recv_chain=imc_recv_hook;
  imc_recv_hook=iced_recv;

  imc_add_event(ICED_REFRESH_TIME, ev_iced_refresh, NULL, 1);

  iced_load_channels();
}
