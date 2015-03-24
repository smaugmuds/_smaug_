/*
 * IMC2 - an inter-mud communications protocol
 *
 * imc-interp.c: packet interpretation code
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
#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include "imc.h"
#include "iced.h"

/* rignore'd people */
imc_ignore_data *imc_ignore_list;
/* prefixes for all data files */
char *imc_prefix;

int mud_has_hub=0; /* for ping control - shogar */
time_t time_of_last_ping, time_since_last_ping=300; /* ping istat - shogar */
time_t time_of_last_ralive, time_since_last_ralive=300; /* ping istat - shogar */
time_t time_of_last_salive, time_since_last_salive=300; /* ping istat - shogar */

/* called when a keepalive has been received */
void imc_recv_keepalive(const char *from, const char *version, const char *flags)
{
  imc_reminfo *p;

  if (!strcasecmp(from, imc_name))
    return;
  
  /*  this should never fail, imc.c should create an
   *  entry if one doesn't exist (in the path update
   *  code)
   */
  p=imc_find_reminfo(from, 0);
  if (!p)		    /* boggle */
    return;

  if(!time_of_last_ralive)
  {
	time_of_last_ralive=time(NULL);
  }
  else
  {
        time_since_last_ralive = time(NULL) - time_of_last_ralive;
	time_of_last_ralive =time(NULL);
  }

  if (imc_hasname(flags, "hide"))
    p->hide=1;
  else
    p->hide=0;
  
  /* lower-level code has already updated p->alive */

  if (strcasecmp(version, p->version))    /* remote version has changed? */
  {
    imc_strfree(p->version);              /* if so, update it */
    p->version=imc_strdup(version);
  }

  /* Only routers should ping - and even then, only directly connected muds */
  /* and only if there is an open connection - shogar */
  if (imc_is_router && imc_getinfo(from) && 
	imc_getinfo(from)->connection && imc_getinfo(from)->connection->state==IMC_CONNECTED)
  {
    struct timeval tv;

    gettimeofday(&tv, NULL);

    imc_send_ping(from, tv.tv_sec, tv.tv_usec);
  }
  /* or muds acting as a hub to another mud - shogar */
  if (!imc_is_router && imc_getinfo(from) && imc_getinfo(from)->connection
    && !(imc_getinfo(from)->flags & IMC_HUB) && mud_has_hub)
  {
    struct timeval tv;

    gettimeofday(&tv, NULL);

    imc_send_ping(from, tv.tv_sec, tv.tv_usec);
  }
}

/* called when a ping request is received */
void imc_recv_ping(const char *from, int time_s, int time_u, const char *path)
{
  /* need a way to monitor the pinging for istat - shogar */
  if(!time_of_last_ping)
  {
	time_of_last_ping=time(NULL);
  }
  else
  {
        time_since_last_ping = time(NULL) - time_of_last_ping;
	time_of_last_ping =time(NULL);
  }

  /* ping 'em back */
  imc_send_pingreply(from, time_s, time_u, path);
  /* if only hubs can ping - or a mud acting as a hub ... then ...
     this should do 2 things, enforce the IMC_HUB flag is properly set
     and stop two muds from playing the i ping you , you ping me, i ping you
     game - shogar */

     if(!mud_has_hub)
     {
     	mud_has_hub=1;
     }
     if(imc_getinfo(from))
      imc_getinfo(from)->flags |= IMC_HUB;

   /* here is the way I meant it to work. The hub pings all direct connects.
      the first imc_send_ping above for muds acting as hubs does not 
      execute because mud_has_hub is false, so we set it here. On the second
      recieved ping the mud pings all the directly connected muds it
      has. They don't have the mud_has_hub flag on, so it stops there. On the
      third keepalive sent from the hub , the muds connected to first mud now 
      ping any muds that they are connected to, ad infinitum.
   */
}

/* called when a ping reply is received */
void imc_recv_pingreply(const char *from, int time_s, int time_u, const char *pathto, const char *pathfrom)
{
  imc_reminfo *p;
  struct timeval tv;

  p = imc_find_reminfo(from, 0);   /* should always exist */
  if (!p)			   /* boggle */
    return;

  gettimeofday(&tv, NULL);      /* grab the exact time now and calc RTT */
  p->ping = (tv.tv_sec - time_s) * 1000 + (tv.tv_usec - time_u) / 1000;

  /* check for pending traceroutes */
  imc_traceroute(p->ping, pathto, pathfrom);
}

/* send a standard 'you are being ignored' rtell */
void imc_sendignore(const char *to)
{
  char buf[IMC_DATA_LENGTH];

  if (strcmp(imc_nameof(to), "*"))
  {
    sprintf(buf, "%s is ignoring you.", imc_name);
    imc_send_tell(NULL, to, buf, 1);
  }
}

/* imc_char_data representation:
 *
 *  Levels are simplified: >0 is a mortal, <0 is an immortal. The 'see' and
 *  'invis' fields are no longer used.
 *
 *  d->wizi  is -1 if the character is invisible to mortals (hidden/invis or
 *           wizi)
 *  d->level is the level of the character (-1=imm, 1=mortal)
 *
 *  also checks rignores for a 'notrust' flag which makes that person a
 *  level 1 mortal for the purposes of wizi visibility checks, etc
 *
 *  Default behavior is now: trusted.
 *  If there's a notrust flag, untrusted. If there's also a trust flag, trusted
 */

/* convert from the char data in 'p' to an internal representation in 'd' */
static void getdata(const imc_packet *p, imc_char_data *d)
{
  int trust=1;

  if (imc_findignore(p->from, IMC_NOTRUST))
    trust=0;
  if (imc_findignore(p->from, IMC_TRUST))
    trust=1;

  strcpy(d->name, p->from);
  d->wizi = trust ? imc_getkeyi(&p->data, "wizi", 0) : 0;
  d->level = trust ? imc_getkeyi(&p->data, "level", 0) : 0;
  d->invis = 0;
}

/* convert back from 'd' to 'p' */
static void setdata(imc_packet *p, const imc_char_data *d)
{
  imc_initdata(&p->data);

  if (!d)
  {
    strcpy(p->from, "*");
    imc_addkeyi(&p->data, "level", -1);
    return;
  }

  strcpy(p->from, d->name);

  if (d->wizi)
    imc_addkeyi(&p->data, "wizi", d->wizi);
  imc_addkeyi(&p->data, "level", d->level);
}

/* handle a packet destined for us, or a broadcast */
void imc_recv(const imc_packet *p)
{
  imc_char_data d;
  int bcast;
  imc_reminfo *i;

  bcast=!strcmp(imc_mudof(p->i.to), "*") ? 1 : 0;
  
  getdata(p, &d);

  if(!imc_is_router)
  {
	i=imc_find_reminfo(imc_mudof(p->from),0);
	if(i)
	{
		if(i->path)
			imc_strfree(i->path);
		i->path=imc_strdup(p->i.path);
		i->ping=0;
                i->type=0;
	}
  }
  
  /* chat: message to a channel (broadcast) */
  if (!strcasecmp(p->type, "chat") && !imc_isignored(p->from))
    imc_recv_chat(&d, imc_getkeyi(&p->data, "channel", 0),
		  imc_getkey(&p->data, "text", ""));

  /* emote: emote to a channel (broadcast) */
  else if (!strcasecmp(p->type, "emote") && !imc_isignored(p->from))
    imc_recv_emote(&d, imc_getkeyi(&p->data, "channel", 0),
		   imc_getkey(&p->data, "text", ""));

  /* tell: tell a player here something */
  else if (!strcasecmp(p->type, "tell"))
  {
    if (imc_isignored(p->from))
    {
      imc_sendignore(p->from);
    }
    else
    {
      imc_recv_tell(&d, p->to, imc_getkey(&p->data, "text", ""),
		    imc_getkeyi(&p->data, "isreply", 0));
    }
  }

  /* who-reply: receive a who response */
  else if (!strcasecmp(p->type, "who-reply"))
    imc_recv_whoreply(p->to, imc_getkey(&p->data, "text", ""),
		      imc_getkeyi(&p->data, "sequence", -1));

  /* who: receive a who request */
  else if (!strcasecmp(p->type, "who"))
  {
    if (imc_isignored(p->from))
    {
      imc_sendignore(p->from);
    }
    else
    {
      imc_recv_who(&d, imc_getkey(&p->data, "type", "who"));
    }
  }

  /* whois-reply: receive a whois response */
  else if (!strcasecmp(p->type, "whois-reply"))
    imc_recv_whoisreply(p->to, imc_getkey(&p->data, "text", ""));

  /* whois: receive a whois request */
  else if (!strcasecmp(p->type, "whois"))
    imc_recv_whois(&d, p->to);

  /* beep: beep a player */
  else if (!strcasecmp(p->type, "beep"))
  {
    if (imc_isignored(p->from))
    {
      imc_sendignore(p->from);
    }
    else
    {
      imc_recv_beep(&d, p->to);
    }
  }

  /* is-alive: receive a keepalive (broadcast) */
  else if (!strcasecmp(p->type, "is-alive"))
    imc_recv_keepalive(imc_mudof(p->from),
		       imc_getkey(&p->data, "versionid", "unknown"),
		       imc_getkey(&p->data, "flags", ""));

  /* ping: receive a ping request */
  else if (!strcasecmp(p->type, "ping"))
    imc_recv_ping(imc_mudof(p->from), imc_getkeyi(&p->data, "time-s", 0),
		  imc_getkeyi(&p->data, "time-us", 0), p->i.path);

  /* ping-reply: receive a ping reply */
  else if (!strcasecmp(p->type, "ping-reply"))
    imc_recv_pingreply(imc_mudof(p->from), imc_getkeyi(&p->data, "time-s", 0),
		       imc_getkeyi(&p->data, "time-us", 0),
		       imc_getkey(&p->data, "path", NULL), p->i.path);

  /* mail: mail something to a local player */
  else if (!strcasecmp(p->type, "mail"))
    imc_recv_mail(imc_getkey(&p->data, "from", "error@hell"),
		  imc_getkey(&p->data, "to", "error@hell"),
		  imc_getkey(&p->data, "date", "(IMC error: bad date)"),
		  imc_getkey(&p->data, "subject", "no subject"),
		  imc_getkey(&p->data, "id", "bad_id"),
		  imc_getkey(&p->data, "text", ""));

  /* mail-ok: remote confirmed that they got the mail ok */
  else if (!strcasecmp(p->type, "mail-ok"))
    imc_recv_mailok(p->from, imc_getkey(&p->data, "id", "bad_id"));

  /* mail-reject: remote rejected our mail, bounce it */
  else if (!strcasecmp(p->type, "mail-reject"))
    imc_recv_mailrej(p->from, imc_getkey(&p->data, "id", "bad_id"),
		     imc_getkey(&p->data, "reason",
				"(IMC error: no reason supplied"));

/* handle keepalive requests - shogar */
  else if (!strcasecmp(p->type, "keepalive-request"))
  {
        if(imc_is_router)
        	imc_logstring("Recieved keepalive request from %s", p->from);
	imc_send_keepalive();
  }
/* expire closed hubs - shogar */
  else if (!strcasecmp(p->type, "close-notify"))
  {
        imc_reminfo *r;
	char fake[90];
    	struct timeval tv;

        if(imc_is_router)
  		imc_logstring("%s reports %s closed.", p->from,
        	imc_getkey(&p->data, "host","unknown"));
        if(imc_is_router)
		return;

	r = imc_find_reminfo(imc_getkey(&p->data,"host","unknown"),0);
        if(r)
	{
		r->type = IMC_REMINFO_EXPIRED;
		for(r=imc_reminfo_list;r;r=r->next)
		{
			char *sf;
			sprintf(fake,"!%s",imc_getkey(&p->data,"host","___unknown"));
			if(r->name 
			&& r->path 
			&& (sf=strstr(r->path,fake))
			&& sf 
			&& (*(sf+strlen(fake))=='!' || *(sf+strlen(fake)) == 0) 
			)
			{

//				imc_logstring("Expiring %s",r->path);
				r->type = IMC_REMINFO_EXPIRED;
    				gettimeofday(&tv, NULL);
    				imc_send_ping(r->name, tv.tv_sec, tv.tv_usec);
			}
		}
				
	}
  }

/* These 4 added by me for the auto-hub-swap -- Scion 1/9/99 */
  else if (!strcasecmp(p->type, "inforequest")) /* Request connection info from all 1.00a hubs -- Scion */
    imc_recv_inforequest(p->from); /* Had to use inforequest because 0.10 responds to info-request :) */
  else if (!strcasecmp(p->type, "info-reply")) /* receive the reply from the inforequest :) -- Scion */
	imc_recv_info_reply(p->from, imc_getkey(&p->data, "hub", "no"), imc_getkeyi(&p->data, "direct", -1));
  
  else if (!strcasecmp(p->type, "switch-reply")) /* hub confirmed that it added a connection to us. -- Scion */
	  imc_autoconnect_reply_accept(    /* Add a connection back to the hub */
	  p->from,      
	  imc_getkey(&p->data, "host", "!!!"), 
	  imc_getkeyi(&p->data, "port", -1),
	  imc_getkey(&p->data, "clientpw", "password"), 
	  imc_getkey(&p->data, "serverpw", "password"), 
	  imc_getkeyi(&p->data, "rcvstamp", 0), 
	  imc_getkeyi(&p->data, "noforward", 0), 
	  imc_getkey(&p->data, "flags", "none"),
	  imc_getkey(&p->data, "localname", "!!!"),
	  imc_getkey(&p->data, "confirm", "not accepted")
	  );
  else if (!strcasecmp(p->type, "imc-switch")) /* hub receives request to add a connection -- Scion */
	  imc_recv_autoconnect(
	  p->from, 
	  imc_getkey(&p->data, "host", "!!!"), 
	  imc_getkeyi(&p->data, "port", -1),
	  imc_getkey(&p->data, "clientpw", "password"), 
	  imc_getkey(&p->data, "serverpw", "password"), 
	  imc_getkeyi(&p->data, "rcvstamp", 0), 
	  imc_getkeyi(&p->data, "noforward", 0), 
	  imc_getkey(&p->data, "flags", "none"),
	  imc_getkey(&p->data, "localname", "!!!")
	  );
/* call catch-all fn if present */
  else
  {
    imc_packet out;

	if (imc_recv_hook)
      if ((*imc_recv_hook)(p, bcast))
	return;

    if (bcast || !strcasecmp(p->type, "reject"))
      return;
    
    /* reject packet */
      if (!imc_is_router)
    { 
    strcpy(out.type, "reject");
    strcpy(out.to, p->from);
    strcpy(out.from, p->to);

    imc_clonedata(&p->data, &out.data);
    imc_addkey(&out.data, "old-type", p->type);

    imc_send(&out);
    imc_freedata(&out.data);
    }
  }
}


/* Commands called by the interface layer */

 /* This function was rendered useless when everything was packed into
 rquery, and the rquery info command returned that dumb little #define instead
 of this info. I swapped this with the rquery info function, and now I'm salvaging
 this function to use for something entirely different. 1/5/99 -- Scion
 */
void imc_recv_inforequest(const char *from) {
	/* What we're going to do here is to reply to a type of packet
	labelled as "inforequest". We'll reply with two things: 
	1) Are we a hub or not?
	2) The number of direct connections we have.
	
	  What will happen when the MUD receives this info is that it will compare
	  each of the available hubs and find the one with the least number of direct
	  connections. It will then switch its hub connection to the hub with less
	  connections, hopefully averaging the number of connections per hub. You will
	  be able to prevent this auto-swap by #defining NO_OPTIMIZE in imc-config.h -- Scion */
  imc_packet reply;
  int num_direct=0;
  imc_info *i;
  
  extern imc_info *imc_info_list;

  strcpy(reply.to, from);
  strcpy(reply.from, "*");

  imc_initdata(&reply.data);

  if (imc_isignored(from)) /* Don't reply if they're ignored */
  {
    strcpy(reply.type, "info-unavailable");
    imc_send(&reply);
  }
  strcpy(reply.type, "info-reply");
    
  if (imc_is_router) 
  	/* don't even send a reply if we're not a hub -- Scion */
  {
	  imc_addkey(&reply.data, "hub",    "yes");
	  for (i=imc_info_list; i; i=i->next)
              if(i->connection && i->connection->state & IMC_CONNECTED) /* only the live ones - shogar */
			num_direct++;
	  imc_addkeyi(&reply.data, "direct",    
		imc_siteinfo.maxguests - num_direct);
  
        /*  don't send if num_direct exceeds maxguest - shogar */
        if(num_direct <= imc_siteinfo.maxguests)
        {
  		imc_send(&reply);
  		imc_logstring("Sent info-reply to %s.", from);
	}
  }
  imc_freedata(&reply.data);
}

/* Take the data from an info-reply packet and process it. We shouldn't get packets from MUDs, only
from hubs. -- Scion */
void imc_recv_info_reply(const char *from, const char *hub, int direct) {
	
/* Globals needed: hubname, directnum */

if (direct > global_directnum) 
{
	imc_info *i;

	/* spin infolist check for noswitch flag - shogar */
	  for (i=imc_info_list; i; i=i->next)
              if(i->flags & IMC_NOSWITCH && !strcasecmp(imc_mudof(from),i->name))
	      {
		imc_logstring("Rejecting info-reply(%d) from %s", direct,from);
			return;
	      }
		imc_sncpy(global_hubname, imc_mudof(from),  IMC_MNAME_LENGTH);
		global_directnum=direct;
	}
	imc_logstring("Received info-reply(%d) from %s", direct,from);
}

/* send a blank packet with a type of "inforequest" to everyone. The reply is automatic
so we don't need to send any data except who we are. Those with less than version 1.0a will
reply with a reject packet. */
void imc_send_info_request(void)
{
  imc_packet out;

  if (imc_active<IA_UP)
    return;
  global_directnum=-1;
  if(!time_of_last_salive) /* ping istat monitor - shogar */
  {
	time_of_last_salive=time(NULL);
  }
  else
  {
        time_since_last_salive = time(NULL) - time_of_last_salive;
	time_of_last_salive =time(NULL);
  }

  imc_initdata(&out.data);
  strcpy(out.type, "inforequest");
  strcpy(out.from, "*");
  strcpy(out.to, "*@*");

  imc_send(&out);
  imc_freedata(&out.data);
  imc_logstring("Polling for fast hubs.");
}


/* send a message out on a channel */
void imc_send_chat(const imc_char_data *from, int channel,
		   const char *argument, const char *to)
{
  imc_packet out;
  char tobuf[IMC_MNAME_LENGTH];

  if (imc_active<IA_UP)
    return;

  setdata(&out, from);

  strcpy(out.type, "chat");
  strcpy(out.to, "*@*");
  imc_addkey(&out.data, "text", argument);
  imc_addkeyi(&out.data, "channel", channel);

  to=imc_getarg(to, tobuf, IMC_MNAME_LENGTH);
  while (tobuf[0])
  {
    if (!strcmp(tobuf, "*") || !strcasecmp(tobuf, imc_name) ||
	imc_find_reminfo(tobuf, 0))
    {
      strcpy(out.to, "*@");
      strcat(out.to, tobuf);
      imc_send(&out);
    }

    to=imc_getarg(to, tobuf, IMC_MNAME_LENGTH);
  }

  imc_freedata(&out.data);
}

void imc_send_direct(const imc_char_data *from, int channel,
					 const char *argument, const char *to) {
	/* This sends a message out only to those MUDs directly connected to the one running this code.
	Added 12/26/98 to cut down on UCMM IMC network traffic -- Scion */
	imc_packet out;
  char tobuf[IMC_MNAME_LENGTH];
    imc_connect *c;

  if (imc_active<IA_UP)
    return;

  setdata(&out, from);

  strcpy(out.type, "chat"); /* emulate a broadcast packet so we don't have to add a imc_recv_direct() */
  strcpy(out.to, "*@!direct!"); /* But we're not sending to *@* this time. */
  imc_addkey(&out.data, "text", argument);
  imc_addkeyi(&out.data, "channel", channel);

  for (c=imc_connect_list; c; c=c->next) /* Go through the list of direct connections */
      if (c->state==IMC_CONNECTED) {
		  strcpy(tobuf, c->info->name);
		  strcpy(out.to, "*@");
		  strcat(out.to, tobuf);
          imc_send(&out); /* And send them what they think is a broadcast packet. */
	  }  

  imc_freedata(&out.data);
}

/* send an emote out on a channel */
void imc_send_emote(const imc_char_data *from, int channel,
		    const char *argument, const char *to)
{
  imc_packet out;
  char tobuf[IMC_MNAME_LENGTH];

  if (imc_active<IA_UP)
    return;

  setdata(&out, from);

  strcpy(out.type, "emote");
  imc_addkeyi(&out.data, "channel", channel);
  imc_addkey(&out.data, "text", argument);

  to=imc_getarg(to, tobuf, IMC_MNAME_LENGTH);
  while (tobuf[0])
  {
    if (!strcmp(tobuf, "*") || !strcasecmp(tobuf, imc_name) ||
	imc_find_reminfo(tobuf, 0))
    {
      strcpy(out.to, "*@");
      strcat(out.to, tobuf);
      imc_send(&out);
    }

    to=imc_getarg(to, tobuf, IMC_MNAME_LENGTH);
  }

  imc_freedata(&out.data);
}

/* send a tell to a remote player */
void imc_send_tell(const imc_char_data *from, const char *to,
		   const char *argument, int isreply)
{
  imc_packet out;

  if (imc_active<IA_UP)
    return;

  if (!strcmp(imc_mudof(to), "*"))
    return; /* don't let them do this */

  setdata(&out, from);

  imc_sncpy(out.to, to, IMC_NAME_LENGTH);
  strcpy(out.type, "tell");
  imc_addkey(&out.data, "text", argument);
  if (isreply)
    imc_addkeyi(&out.data, "isreply", isreply);

  imc_send(&out);
  imc_freedata(&out.data);
}

/* send a who-request to a remote mud */
void imc_send_who(const imc_char_data *from, const char *to, const char *type)
{
  imc_packet out;

  if (imc_active<IA_UP)
    return;

  if (!strcmp(imc_mudof(to), "*"))
    return; /* don't let them do this */

  setdata(&out, from);

  sprintf(out.to, "*@%s", to);
  strcpy(out.type, "who");

  imc_addkey(&out.data, "type", type);

  imc_send(&out);
  imc_freedata(&out.data);
}

/* respond to a who request with the given data */
void imc_send_whoreply(const char *to, const char *data, int sequence)
{
  imc_packet out;

  if (imc_active<IA_UP)
    return;

  if (!strcmp(imc_mudof(to), "*"))
    return; /* don't let them do this */

  imc_initdata(&out.data);

  imc_sncpy(out.to, to, IMC_NAME_LENGTH);
  strcpy(out.type, "who-reply");
  strcpy(out.from, "*");
  imc_addkey(&out.data, "text", data);
  if (sequence!=-1)
    imc_addkeyi(&out.data, "sequence", sequence);
  
  imc_send(&out);
  imc_freedata(&out.data);
}

/* special handling of whoreply construction for sequencing */
static char *wr_to;
static char *wr_buf;
static int wr_sequence;

void imc_whoreply_start(const char *to)
{
  wr_sequence=0;
  wr_to=imc_strdup(to);
  wr_buf=imc_getsbuf(IMC_DATA_LENGTH);
}

void imc_whoreply_add(const char *text)
{
  /* give a bit of a margin for error here */
  if (strlen(wr_to) + strlen(text) >= IMC_DATA_LENGTH-500)
  {
    imc_send_whoreply(wr_to, wr_buf, wr_sequence);
    wr_sequence++;
    imc_sncpy(wr_buf, text, IMC_DATA_LENGTH);
    return;
  }

  strcat(wr_buf, text);
}

void imc_whoreply_end(void)
{
  imc_send_whoreply(wr_to, wr_buf, -(wr_sequence+1));
  imc_strfree(wr_to);
  wr_buf[0]=0;
  imc_shrinksbuf(wr_buf);
}

/* send a whois-request to a remote mud */
void imc_send_whois(const imc_char_data *from, const char *to)
{
  imc_packet out;

  if (imc_active<IA_UP)
    return;

  if (strchr(to, '@'))
    return;

  setdata(&out, from);

  sprintf(out.to, "%s@*", to);
  strcpy(out.type, "whois");

  imc_send(&out);
  imc_freedata(&out.data);
}

/* respond with a whois-reply */
void imc_send_whoisreply(const char *to, const char *data)
{
  imc_packet out;

  if (imc_active<IA_UP)
    return;

  if (!strcmp(imc_mudof(to), "*"))
    return; /* don't let them do this */

  imc_initdata(&out.data);

  imc_sncpy(out.to, to, IMC_NAME_LENGTH);
  strcpy(out.type, "whois-reply");
  strcpy(out.from, "*");
  imc_addkey(&out.data, "text", data);

  imc_send(&out);
  imc_freedata(&out.data);
}

/* beep a remote player */
void imc_send_beep(const imc_char_data *from, const char *to)
{
  imc_packet out;

  if (imc_active<IA_UP)
    return;

  if (!strcmp(imc_mudof(to), "*"))
    return; /* don't let them do this */

  setdata(&out, from);
  strcpy(out.type, "beep");
  imc_sncpy(out.to, to, IMC_NAME_LENGTH);

  imc_send(&out);
  imc_freedata(&out.data);
}

/* send a keepalive to everyone */
void imc_send_keepalive(void)
{
  imc_packet out;

  if (imc_active<IA_UP)
    return;

  imc_initdata(&out.data);
  strcpy(out.type, "is-alive");
  strcpy(out.from, "*");
  strcpy(out.to, "*@*");
  imc_addkey(&out.data, "versionid", IMC_VERSIONID);
  if (imc_siteinfo.flags[0])
    imc_addkey(&out.data, "flags", imc_siteinfo.flags);

  imc_send(&out);
  imc_freedata(&out.data);
  imc_add_event(ICED_REFRESH_TIME, ev_iced_refresh, NULL, 1);
}
/* send a keepalive request to everyone - shogar */
void imc_request_keepalive(void)
{
  imc_packet out;
/*
  if (imc_active<IA_UP)
    return;
*/

  if(!time_of_last_salive)
  {
	time_of_last_salive=time(NULL);
  }
  else
  {
        time_since_last_salive = time(NULL) - time_of_last_salive;
	time_of_last_salive =time(NULL);
  }
  imc_initdata(&out.data);
  strcpy(out.type, "keepalive-request");
  strcpy(out.from, "*");
  strcpy(out.to, "*@*");
  imc_addkey(&out.data, "versionid", IMC_VERSIONID);
  if (imc_siteinfo.flags[0])
    imc_addkey(&out.data, "flags", imc_siteinfo.flags);

  imc_send(&out);
  imc_freedata(&out.data);
}

/* send a ping with a given timestamp */
void imc_send_ping(const char *to, int time_s, int time_u)
{
  imc_packet out;

  if (imc_active<IA_UP)
    return;

  imc_initdata(&out.data);
  strcpy(out.type, "ping");
  strcpy(out.from, "*");
  strcpy(out.to, "*@");
  imc_sncpy(out.to+2, to, IMC_MNAME_LENGTH-2);
  imc_addkeyi(&out.data, "time-s", time_s);
  imc_addkeyi(&out.data, "time-us", time_u);

  imc_send(&out);
  imc_freedata(&out.data);
}

/* send a pingreply with the given timestamp */
void imc_send_pingreply(const char *to, int time_s, int time_u, const char *path)
{
  imc_packet out;

  if (imc_active<IA_UP)
    return;

  imc_initdata(&out.data);
  strcpy(out.type, "ping-reply");
  strcpy(out.from, "*");
  strcpy(out.to, "*@");
  imc_sncpy(out.to+2, to, IMC_MNAME_LENGTH-2);
  imc_addkeyi(&out.data, "time-s", time_s);
  imc_addkeyi(&out.data, "time-us", time_u);
  imc_addkey(&out.data, "path", path);

  imc_send(&out);
  imc_freedata(&out.data);
}

/* send connection info to new hub for optimization */
void imc_send_autoconnect(const char *to)
{
  imc_packet out;
  imc_info *i;
  char no_port[IMC_MNAME_LENGTH];

  int a=0,b=0;

  if (imc_active<IA_UP)
    return;

  i=imc_getinfo(to);
  if(i && i == imc_info_list && i->connection) /* not if primary hub - shogar */
    return;

     /* Send all this info: -- Scion 
	 imc set <mudname> all <host> <port> <clientpw> <serverpw> <rcvstamp> <noforward> <flags>
		*/
  imc_initdata(&out.data);
  strcpy(out.type, "imc-switch");
  strcpy(out.from, "*");
  strcpy(out.to, "*@");
  imc_sncpy(out.to+2, to, IMC_MNAME_LENGTH-2);

  /* Get that pesky port out of the host name */
  for (a=0;a<strlen(imc_siteinfo.host); a++) {
	  if (imc_siteinfo.host[a]==':' 
	  || imc_siteinfo.host[a]==' ') 
	  {
		  break;
	  }
	  no_port[b++]=imc_siteinfo.host[a];
	  no_port[b]='\0';
  }
  imc_addkey(&out.data, "host", no_port);
  imc_addkeyi(&out.data, "port", (int)imc_port);
  
  if(i) /* don't assign new ones, if we have passwords */
  {
  	imc_addkey(&out.data, "clientpw", i->clientpw);
  	imc_addkey(&out.data, "serverpw", i->serverpw);
  }
  else 
  {
  	imc_addkey(&out.data, "clientpw", imc_make_password());/* these are random */
  	imc_addkey(&out.data, "serverpw", imc_make_password());
  }
  imc_addkeyi(&out.data, "rcvstamp", 0);
  imc_addkeyi(&out.data, "noforward", 0);
  imc_addkey(&out.data, "flags", "none");
  imc_addkey(&out.data, "localname", imc_name);

  imc_send(&out);
  imc_freedata(&out.data);
  imc_logstring("Autoconnect sent to %s", to);
}

/* add the mud to our list if we're a hub, and reply with our info. -- Scion*/
void imc_recv_autoconnect(const char *to, const char *rhost, int rport, 
	const char *rclientpw, const char *rserverpw, 
  	int rrcvstamp, int rnoforward, const char *rflags, 
	const char *remotename)
{
  imc_packet out;
  char no_port[IMC_MNAME_LENGTH];

  int a=0,b=0;

  imc_info *i,*o;

   if (imc_active<IA_UP)
    return;


  /* Receive all this info: -- Scion 
	 imc set <mudname> all <host> <port> <clientpw> <serverpw> <rcvstamp> <noforward> <flags>
		*/
#ifdef NO_OPTIMIZE
   imc_logstring("Autoconnect refused because NO_OPTIMIZE is defined.");
   return;
#endif
  
  if (imc_name && !strcasecmp(remotename, imc_name)) {
	  imc_logstring("Autoconnect attempt allegedly from this server! Ignoring.");
      return; /* Hey! That's our name! */
  }
	  
  /* Get that pesky port out of the host name */
  for (a=0;a<strlen(imc_siteinfo.host); a++) {
	  if (imc_siteinfo.host[a]==':'
	  || imc_siteinfo.host[a]==' ') 
 	  {
		  break;
	  }
	  no_port[b++]=imc_siteinfo.host[a];
	  no_port[b]='\0';
  }
  /* Send our information back to the MUD */
  imc_initdata(&out.data);
  strcpy(out.type, "switch-reply");
  strcpy(out.from, "*");
  strcpy(out.to, "*@");
  imc_sncpy(out.to+2, remotename, IMC_MNAME_LENGTH-2);
  
  imc_addkey(&out.data, "confirm", "connection added");
  imc_addkey(&out.data, "host", no_port); /* don't put the port number in from imc_siteinfo.host b/c it's the game port */
  imc_addkeyi(&out.data, "port", (int)imc_port);
  imc_addkey(&out.data, "clientpw", rclientpw);
  imc_addkey(&out.data, "serverpw", rserverpw);
  imc_addkeyi(&out.data, "rcvstamp", 0);
  imc_addkeyi(&out.data, "noforward", 0);
  imc_addkey(&out.data, "flags", "none");
  imc_addkey(&out.data, "localname", imc_name);
  
  imc_send(&out);
  
  imc_freedata(&out.data); 
  
  /* Setup the new connection... */
  if((o=imc_getinfo(remotename))) /* remove old entries - shogar */
  {
  	i=imc_insert_info();
        i->connection=o->connection;
        i->flags=o->flags;
        if (i->connection) i->connection->info=i;
  	imc_cancel_info(o); 
  }
  else
  {
  	i=imc_insert_info();
        i->flags = imc_flagvalue("new", imc_connection_flags); /* Hubs don't need specific flags really */
  }


  i->name       = imc_strdup(remotename);
  i->host       = imc_strdup(rhost);
  i->port       = (unsigned short)rport;
  i->clientpw   = imc_strdup(rclientpw);
  i->serverpw   = imc_strdup(rserverpw);
  i->rcvstamp   = rrcvstamp;
  i->noforward  = rnoforward;
  i->connect_attempts = 0;


  imc_saveconfig();
  /* All done! */
  
  imc_logstring("Autoconnect accepted from %s", to);


}

void imc_autoconnect_reply_accept(const char *from, const char *rhost, int rport, const char *rclientpw, const char *rserverpw, 
						  int rrcvstamp, int rnoforward, const char *rflags, const char *remotename, const char *confirm) {
	/* Wow.. what a function name! This gets the packet that says "connection added"
	in it from the hub, then adds the appropriate connection here. -- Scion */

	imc_info *i,*o;

	imc_logstring("Received autoconnect reply from %s.", remotename);
	if (strcasecmp(confirm, "connection added")) { /* Don't change this, or it won't work. */
		imc_logstring("Confirmation refused: %s", confirm);
		return;
	}

	if (imc_name && !strcasecmp(remotename, imc_name)) {
	  imc_logstring("Autoconnect attempt allegedly from this host! Ignoring.");
      return; /* Hey! That's our name! */
  }
  else if (!strcasecmp(rhost, "!!!")) {
	  imc_logstring("Unknown host name for %s. Aborting.", remotename);
	  return;
  } else {
	/* Setup the new connection... */
  if((o=imc_getinfo(remotename))) /* remove old entries - shogar */
  {
  	i=imc_insert_info();
        i->connection=o->connection;
        i->flags=o->flags; 
        if(i->connection) i->connection->info=i;
  	imc_cancel_info(o); 
  }
  else
  {
  	i=imc_insert_info();
//        i->flags      = imc_flagvalue("hub reconnect new", imc_connection_flags);
        i->flags      = imc_flagvalue("hub new", imc_connection_flags);
  }

      i->name       = imc_strdup(remotename);
      i->host       = imc_strdup(rhost);
      i->port       = (unsigned short)rport;
      i->clientpw   = imc_strdup(rclientpw);
      i->serverpw   = imc_strdup(rserverpw);
      i->rcvstamp   = rrcvstamp;
      i->noforward  = rnoforward;
      i->connect_attempts = 0;

      imc_saveconfig();

	  imc_connect_to(remotename);
	  /* All done! */
	  imc_logstring("Added connection to %s.", remotename);

  }
  imc_send_keepalive();

}
