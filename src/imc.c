/*
 * IMC2 - an inter-mud communications protocol
 *
 * imc.c: the core protocol code
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
#include <sys/file.h>

/* if freeBSD , use flock instead of lockf - shogar */
#ifndef F_TLOCK
#define F_TLOCK LOCK_EX
#define F_ULOCK LOCK_UN
#define lockf(a,b,c) flock(a,b)
#endif

#include "imc.h"
#include "icec.h"

/*
 *  Local declarations + some global stuff from imc.h
 */

/* decls of vars from imc.h */

imc_connect *imc_connect_list;
imc_info    *imc_info_list;
imc_reminfo *imc_reminfo_list;

imc_statistics imc_stats;

/* imc_active now has more states:
 *
 * 0:  nothing done yet
 * 1:  configuration loaded, but IMC not active. imc_name is not set.
 * 2:  configuration loaded, but IMC not active. imc_name is valid.
 * 3:  imc_name and configuration loaded, network active, port disabled.
 * 4:  everything active.
 *
 */
int imc_active;
time_t imc_now;          /* current time */
time_t imc_boot;          /* current time */
int imc_lock;
int imc_is_router;       /* run as a router (ie. ping stuff) */
int imc_lock_file=-1;

char global_hubname[IMC_MNAME_LENGTH] = "NULL";
int global_directnum=-1;
extern int imc_hubswitch;

/* control socket for accepting connections */
static int control;

/* sequence memory */
_imc_memory imc_memory[IMC_MEMORY];

unsigned long imc_sequencenumber;	  /* sequence# for outgoing packets */

char *imc_name;			      /* our IMC name */
unsigned short imc_port;              /* our port; 0=disabled */
unsigned long imc_bind;               /* IP to bind to */

/* imc flag/state tables */

/* flags for connections */
const imc_flag_type imc_connection_flags[] =
{
  { "noauto",    IMC_NOAUTO    },
  { "client",    IMC_CLIENT    },
  { "reconnect", IMC_RECONNECT },
  { "broadcast", IMC_BROADCAST },
  { "deny",      IMC_DENY      },
  { "quiet",     IMC_QUIET     },
  { "hub",       IMC_HUB       }, /* SPAM fix - shogar && currently used hub (unless main hub.. confused yet?) ;) - Scion */
  { "main_hub",  IMC_MAIN_HUB  }, /* main hub, same as hub, but first to try - Scion */
  { "old_hub",   IMC_OLD_HUB   }, /* backup hub just in case - Scion */
  { "dead_hub",  IMC_DEAD_HUB  }, /* defunct hub, 3 connection attempts - Scion */
  { "new",       IMC_NEW_HUB  }, /* mark new hubs so dont save - shogar */
  { "noswitch",  IMC_NOSWITCH  }, /* might be reasons to never switch to them - shogar */
  { NULL, 0 },
};

/* flags for rignore entries */
const imc_flag_type imc_ignore_types[] =
{
  { "ignore",    IMC_IGNORE               },
  { "notrust",   IMC_NOTRUST              },
  { "trust",     IMC_TRUST                },
  
  /* for old config files */
  { "1",         IMC_IGNORE               },
  { "2",         IMC_NOTRUST              },
  { NULL, 0 }
};

/* states that state in imc_connect can take */
const imc_flag_type imc_state_names[] =
{
  { "closed",      IMC_CLOSED     },
  { "connecting",  IMC_CONNECTING },
  { "wait1",       IMC_WAIT1      },
  { "wait2",       IMC_WAIT2      },
  { "connected",   IMC_CONNECTED  },
  { NULL, 0 }
};

/* states that imc_active can take */
const imc_flag_type imc_active_names[] =
{
  { "inactive - not initialized",                         IA_NONE      },
  { "inactive - config loaded, local name not set",       IA_CONFIG1   },
  { "inactive - config loaded, local name set",           IA_CONFIG2   },
  { "active - not accepting connections",                 IA_UP        },
  { "active - accepting connections",                     IA_LISTENING },
  { NULL, 0 }
};

/* set up a new imc_connect struct, and link it into imc_connect_list */
imc_connect *imc_new_connect(void)
{
  imc_connect *c;

  c=imc_malloc(sizeof(*c));
  c->state    = IMC_CLOSED;
  c->desc     = -1;
  c->insize   = IMC_MINBUF;
  c->inbuf    = imc_malloc(c->insize);
  c->outsize  = IMC_MINBUF;
  c->outbuf   = imc_malloc(c->outsize);
  c->inbuf[0] = c->outbuf[0] = 0;
  c->info     = NULL;
  c->spamcounter1=0;
  c->spamcounter2=0;
  c->spamtime1=0;
  c->spamtime2=0;
  c->newoutput=0;

  c->next=imc_connect_list;
  imc_connect_list=c;

  return c;
}

/*  free buffers and extract 'c' from imc_connect_list
 *  called from imc_idle_select when we're done with a connection with
 *  c->state==IMC_CLOSED
 */
void imc_extract_connect(imc_connect *c)
{
  imc_connect *c_find;

  if (c->state!=IMC_CLOSED)
  {
    imc_logerror("imc_extract_connect: non-closed connection");
    return;
  }
  
  imc_free(c->inbuf, c->insize);
  imc_free(c->outbuf, c->outsize);

  if (c==imc_connect_list)
    imc_connect_list=c->next;
  else
  {
    for (c_find=imc_connect_list; c_find && c_find->next!=c;
	 c_find=c_find->next)
      ;
    
    if (!c_find)
      imc_logerror("imc_extract_connect: not in imc_connect_list");
    else
      c_find->next=c->next;
  }

  imc_cancel_event(NULL, c);

  imc_free(c, sizeof(*c));
}

/* update our routing table based on a packet received with path "path" */
static void updateroutes(const char *path)
{
  imc_reminfo *p;
  const char *sender, *last;
  const char *temp;

  /* loop through each item in the path, and update routes to there */

  last = imc_lastinpath(path);
  temp = path;
  while (temp && temp[0])
  {
    sender=imc_firstinpath(temp);

    if (strcasecmp(sender, imc_name))
    {
      /* not from us */
      /* check if its in the list already */

      p = imc_find_reminfo(sender, 1);
      if (!p)			/* not in list yet, create a new entry */
      {
	p=imc_new_reminfo();

	p->name    = imc_strdup(sender);
	p->ping    = 0;
	p->alive   = imc_now;
	p->route   = imc_strdup(last);
	p->version = imc_strdup("unknown");
	p->type    = IMC_REMINFO_NORMAL;
      }
      else
      {				/* already in list, update the entry */
	if (strcasecmp(last, p->route))
	{
	  imc_strfree(p->route);
	  p->route=imc_strdup(last);
	}
	p->alive=imc_now;
	p->type = IMC_REMINFO_NORMAL;
      }
    }

    /* get the next item in the path */

    temp=strchr(temp, '!');
    if (temp)
      temp++;			/* skip to just after the next '!' */
  }
}

/* return 1 if 'name' is a part of 'path'  (internal) */
static int inpath(const char *path, const char *name)
{
  char buf[IMC_MNAME_LENGTH+3];
  char tempn[IMC_MNAME_LENGTH], tempp[IMC_PATH_LENGTH];

  imc_sncpy(tempn, name, IMC_MNAME_LENGTH);
  imc_sncpy(tempp, path, IMC_PATH_LENGTH);
  imc_slower(tempn);
  imc_slower(tempp);

  if (!strcmp(tempp, tempn))
    return 1;

  sprintf(buf, "%s!", tempn);
  if (!strncmp(tempp, buf, strlen(buf)))
    return 1;

  sprintf(buf, "!%s", tempn);
  if (strlen(buf) < strlen(tempp) &&
      !strcmp(tempp + strlen(tempp) - strlen(buf), buf))
    return 1;

  sprintf(buf, "!%s!", tempn);
  if (strstr(tempp, buf))
    return 1;

  return 0;
}

/*
 *  Core functions (all internal)
 */

/* accept a connection on the control port */
static void do_accept(void)
{
  int d;
  imc_connect *c;
  struct sockaddr_in sa;
  int size = sizeof(sa);
  int r;

  d=accept(control, (struct sockaddr *) &sa, &size);
  if (d<0)
  {
    imc_lerror("accept");
    return;
  }

  r=fcntl(d, F_GETFL, 0);
  if (r<0 || fcntl(d, F_SETFL, O_NONBLOCK | r)<0)
  {
    imc_lerror("do_accept: fcntl");
    close(d);
    return;
  }

  c=imc_new_connect();
  c->state    = IMC_WAIT1;
  c->desc     = d;

  imc_add_event(IMC_LOGIN_TIMEOUT, ev_login_timeout, c, 1);
  imc_logstring("connection from %s:%d on descriptor %d",
		inet_ntoa(sa.sin_addr), ntohs(sa.sin_port), d);
}
/* notify everyone of the closure - shogar */
void imc_close_notify(const char *host)
{
  imc_packet out;
  char shorthost[80];

  if (imc_active<IA_UP)
    return;

  strcpy(shorthost,host);
  if(strchr(shorthost,'['))
    *(strchr(shorthost,'['))=0;

  imc_initdata(&out.data);
  strcpy(out.type, "close-notify");
  strcpy(out.from, "*");
  strcpy(out.to, "*@*");
  imc_addkey(&out.data, "versionid", IMC_VERSIONID);
  if (imc_siteinfo.flags[0])
    imc_addkey(&out.data, "flags", imc_siteinfo.flags);
  imc_addkey(&out.data, "host", shorthost);
  imc_send(&out);
  imc_freedata(&out.data);
}

/* close given connection */
static void do_close(imc_connect *c)
{
  const char *name;
  imc_reminfo *r;

  if (c->info) /* if our switched hub, get a new one right away - shogar */
  {
    if (!imc_is_router && (c->info->flags & IMC_NEW_HUB) && c->info == imc_info_list)
    {
	imc_cancel_event(ev_imc_pollforhub,NULL);
	imc_cancel_event(ev_imc_optimize,NULL);
  	imc_add_event(10, ev_imc_pollforhub, NULL, 1);
  	imc_add_event(70, ev_imc_optimize, NULL, 1);
    }
  }
  
  if (c->state==IMC_CLOSED)
    return;

  name=imc_getconnectname(c);
  if(name && c->state == IMC_CONNECTED) /* dont send if never connected */
  {
  	imc_close_notify(name);
        close(c->desc); /* dont close if closed */
  }
  if (c->state == IMC_CONNECTED)
    c->info->connection=NULL;

  /* handle reconnects */
  if (c->info)
    if ((c->info->flags & IMC_RECONNECT) &&
	!(c->info->flags & IMC_DENY) &&
	!(c->info->flags & IMC_CLIENT))
    {
      imc_setup_reconnect(c->info);
    }

  c->state=IMC_CLOSED;

  /* only log after we've set the state, in case imc_logstring
   * sends packets itself (problems with eg. output buffer overflow).
   */
  if (!c->info || !(c->info->flags & IMC_QUIET))
  {
  	name=imc_getconnectname(c);
    	if(name) imc_logstring("%s: closing link", name);
  }

  if (c->info)
  {
    r=imc_find_reminfo(c->info->name, 1);
    if (r)
      imc_delete_reminfo(r);
  }
  if(c->desc)
	close(c->desc);
  c->desc=0;
  c->inbuf[0]=0;
  c->outbuf[0]=0;
  
}

/* time out a login */
void ev_login_timeout(void *data)
{
  imc_connect *c=(imc_connect *)data;

  if (!c->info || !(c->info->flags & IMC_QUIET))
    imc_logstring("%s: login timeout", imc_getconnectname(c));
  do_close(c);
}

/* read waiting data from descriptor.
 * read to a temp buffer to avoid repeated allocations
 */
static void do_read(imc_connect *c)
{
  int size;
  int r;
  char temp[IMC_MAXBUF];
  char *newbuf;
  int newsize;

  r=read(c->desc, temp, IMC_MAXBUF-1);
  if (!r || (r<0 && errno != EAGAIN && errno != EWOULDBLOCK))
  {
    if (!c->info || !(c->info->flags & IMC_QUIET))
    {
      if (r<0)                    /* read error */
      {
        imc_lerror("%s: read", imc_getconnectname(c));
      }
      else                        /* socket was closed */
      {
        imc_logerror("%s: read: EOF", imc_getconnectname(c));
      }
    }
    do_close(c);
    return;
  }
  
  if (r<0)			/* EAGAIN error */
    return;

  temp[r]=0;

  size=strlen(c->inbuf)+r+1;

  if (size>=c->insize)
  {

#ifdef SHOW_OVERFLOW
	/* not an error anymore, expected and handled - shogar */
    if (size>IMC_MAXBUF)
    {
        imc_logerror("%s: input buffer overflow", imc_getconnectname(c));
        imc_logerror("%d: was allocated", c->insize);
//      do_close(c);
//      imc_free(c->inbuf,c->insize);
//      c->insize=IMC_MINBUF;
//      c->inbuf= imc_malloc(c->insize);
//      size = r + 1;
//      return;
    }
      
#endif
    newsize=c->insize;
    while(newsize<size)
      newsize*=2;

    newbuf=imc_malloc(newsize);
    strcpy(newbuf, c->inbuf);
    imc_free(c->inbuf, c->insize);
    c->inbuf=newbuf;
    c->insize=newsize;
  }
  
  if (size>c->insize/2)
  {
    imc_cancel_event(ev_shrink_input, c);
    imc_add_event(IMC_SHRINKTIME, ev_shrink_input, c, 0);
  }
  if (size<c->insize/2 && size >= IMC_MINBUF)
  {
    newsize=c->insize;
    newsize/=2;

    newbuf=imc_malloc(newsize);
    strcpy(newbuf, c->inbuf);
    imc_free(c->inbuf, c->insize);
    c->inbuf=newbuf;
    c->insize=newsize;
  }

  strcat(c->inbuf, temp);

  imc_stats.rx_bytes += r;
}

/* write to descriptor */
static void do_write(imc_connect *c)
{
  int size, w;

  if (c->state==IMC_CONNECTING)
  {
    /* Wait for server password */
    c->state=IMC_WAIT2;
    return;
  }

  size = strlen(c->outbuf);
  if (!size)			/* nothing to write */
    return;

  w=write(c->desc, c->outbuf, size);
  if (!w || (w<0 && errno != EAGAIN && errno != EWOULDBLOCK))
  {
    if (!c->info || !(c->info->flags & IMC_QUIET))
    {
      if (w<0)			/* write error */
      {
        imc_lerror("%s: write", imc_getconnectname(c));
      }
      else			/* socket was closed */
      {
        imc_logerror("%s: write: EOF", imc_getconnectname(c));
      }
    }
    do_close(c);
    return;
  }

  if (w<0)			/* EAGAIN */
    return;

  /* throw away data we wrote */
//  memmove(c->outbuf, c->outbuf+w, size-w+1);
  strcpy(c->outbuf,c->outbuf+w);

  imc_stats.tx_bytes += w;
}

/* put a line onto descriptors output buffer */
static void do_send(imc_connect *c, const char *line)
{
  int len;
  char *newbuf;
  int newsize=c->outsize;

  if (c->state==IMC_CLOSED)
    return;

//  imc_debug(c, 1, line);	/* log outgoing traffic */

  if (!c->outbuf[0])
    c->newoutput=1;

  len=strlen(c->outbuf)+strlen(line)+3;

  if (len > c->outsize)
  {

#ifdef SHOW_OVERFLOW
	/* not an error anymore, expected and handled - shogar */
    if (len > IMC_MAXBUF)
    {
      if (!c->info || !(c->info->flags & IMC_QUIET))
      imc_logerror("%s: output buffer overflow", imc_getconnectname(c));
      imc_logerror("%d: was allocated", c->outsize);
//      imc_logerror("current buf: %s", c->outbuf);
//    do_close(c);
//      imc_free(c->outbuf,c->outsize);
//      c->outsize=IMC_MINBUF;
//      c->outbuf= imc_malloc(c->outsize);
//      len=strlen(line)+3;
//    return;
  }
#endif
    while(newsize < len)
      newsize*=2;

    newbuf=imc_malloc(newsize);
    strcpy(newbuf, c->outbuf);
    imc_free(c->outbuf, c->outsize);
    c->outbuf=newbuf;
    c->outsize=newsize;
  }
  if (len<c->outsize/2 && len >= IMC_MINBUF)
  {
    newsize=c->outsize/2;

    newbuf=imc_malloc(newsize);
    strcpy(newbuf, c->outbuf);
    imc_free(c->outbuf, c->outsize);
    c->outbuf=newbuf;
    c->outsize=newsize;
  }

  strcat(c->outbuf, line);
  strcat(c->outbuf, "\n\r");

  if (strlen(c->outbuf)>=c->outsize/2)
  {
    imc_cancel_event(ev_shrink_output, c);
    imc_add_event(IMC_SHRINKTIME, ev_shrink_output, c, 0);
  }
}

/*  try to read a line from the input buffer, NULL if none ready
 *  all lines are \n\r terminated in theory, but take other combinations
 */
static const char *imc_getline(char *buffer)
{
  int i;
  char *buf=imc_getsbuf(IMC_PACKET_LENGTH);

  /* copy until \n, \r, end of buffer, or out of space */
  for (i=0; buffer[i] && buffer[i] != '\n' && buffer[i] != '\r' &&
       i+1 < IMC_PACKET_LENGTH; i++)
    buf[i] = buffer[i];

  /* end of buffer and we haven't hit the maximum line length */
  if (!buffer[i] && i+1 < IMC_PACKET_LENGTH)
  {
    buf[0]=0;
    imc_shrinksbuf(buf);
    return NULL;		/* so no line available */
  }

  /* terminate return string */
  buf[i]=0;

  /* strip off extra control codes */
  while (buffer[i] && (buffer[i] == '\n' || buffer[i] == '\r'))
    i++;

  /* remove the line from the input buffer */
//  memmove(buffer, buffer+i, strlen(buffer+i) + 1);
  strcpy(buffer,buffer+i);

  imc_shrinksbuf(buf);
  return buf;
}

static int memory_head; /* next entry in memory table to use, wrapping */

/* checkrepeat: check for repeats in the memory table */
static int checkrepeat(const char *mud, unsigned long seq)
{
  int i;

  for (i=0; i<IMC_MEMORY; i++)
    if (imc_memory[i].from && 
        !strcasecmp(mud, imc_memory[i].from) &&
        seq == imc_memory[i].sequence)
      return 1;

  /* not a repeat, so log it */

  if (imc_memory[memory_head].from)
    imc_strfree(imc_memory[memory_head].from);

  imc_memory[memory_head].from     = imc_strdup(mud);
  imc_memory[memory_head].sequence = seq;
  
  memory_head++;
  if (memory_head==IMC_MEMORY)
    memory_head=0;

  return 0;
}

#ifdef IMC_NOTIFY
static void do_notify(void)
{
  /* tell the central server that we're using IMC.
   *
   * This isn't related to your IMC connections, it's just so I can keep track
   * of how many people are using IMC, with what versions, and where.
   *
   * This gets done once a day (and on startup), but only when packets are
   * actually being forwarded. This means that muds not connected
   * to anything won't notify. The notification is a single UDP packet to a
   * hardcoded IP, containing the version ID of IMC being used, and your IMC
   * name.
   *
   * If it bugs you, comment out the #define IMC_NOTIFY in imc.h and it won't
   * notify the central server.
   */

  struct sockaddr_in sa;
  int s;
  char buf[100];

  sprintf(buf, "name=%s\nversion=%s\nemail=%s\n",
	  imc_name ? imc_name : "unset",
	  IMC_VERSIONID,
	  imc_siteinfo.email);

  s=socket(AF_INET, SOCK_DGRAM, 0);
  if (s<0)
    return;

  /* we won't do a lookup here.. if the IP changes, such is life.
   * 209.51.169.2 is toof.net
   */
  sa.sin_family=AF_INET;
  sa.sin_addr.s_addr=inet_addr("209.51.169.2");
  sa.sin_port=htons(9000);

  sendto(s, buf, 100, 0, (struct sockaddr *)&sa, sizeof(sa));

  close(s);
}
#endif  

/* send a packet to a mud using the right version */
static void do_send_packet(imc_connect *c, const imc_packet *p)
{
  const char *output;
  int v;
#ifdef IMC_NOTIFY
  static time_t last_notify;

  if (last_notify + 3600*24 < imc_now)
  {
    last_notify = imc_now;
    do_notify();
  }
#endif

  v=c->version;
  if (v>IMC_VERSION)
    v=IMC_VERSION;

  output=(*imc_vinfo[v].generate)(p);

  if (output)
  {
    imc_stats.tx_pkts++;
    if (strlen(output) > imc_stats.max_pkt)
      imc_stats.max_pkt=strlen(output);
    do_send(c, output);
  }
}

static int can_forward(const imc_packet *p)
{
  if (!strcasecmp(p->type, "chat") ||
      !strcasecmp(p->type, "emote"))
  {
    int chan=imc_getkeyi(&p->data, "channel", 0);
    
    if (chan==0 || chan==1 || chan==3)
      return 0;
  }

  return 1;
}

/* forward a packet - main routing function, all packets pass through here */
static void forward(imc_packet *p)
{
  imc_info *i;
  int broadcast, isbroadcast;
  const char *to;
  imc_reminfo *route;
  imc_info *direct;
  imc_connect *c;
  char recievedfrom[IMC_MAXBUF]; /* SPAM fix - shogar */
  imc_connect *rf; /* SPAM fix - shogar */

  /* check for duplication, and register the packet in the sequence memory */
  
  if (p->i.sequence && checkrepeat(imc_mudof(p->i.from), p->i.sequence))
    return;

  /* check for packets we've already forwarded */

  if (inpath(p->i.path, imc_name))
    return;

  /* check for really old packets */

  route=imc_find_reminfo(imc_mudof(p->i.from), 1);
  if (route)
  {
    if ((p->i.sequence+IMC_PACKET_LIFETIME) < route->top_sequence)
    {
      imc_stats.sequence_drops++;
#ifdef LOG_LATE_PACKETS 
	/* kind of spammy, but late packets are natural when the path
           is broken between sender and reciever */
      if(imc_is_router) /* spare the muds from seeing this - shogar */
         imc_logstring("sequence drop: %s (seq=%ld, top=%ld)",
		    p->i.path, p->i.sequence, route->top_sequence);
#endif
      return;
    }
    if (p->i.sequence > route->top_sequence)
      route->top_sequence=p->i.sequence;
  }

  /* update our routing info */

  updateroutes(p->i.path);

  /* forward to our mud if it's for us */

  if (!strcmp(imc_mudof(p->i.to), "*") ||
      !strcasecmp(imc_mudof(p->i.to), imc_name))
  {
    strcpy(p->to, imc_nameof(p->i.to));    /* strip the name from the 'to' */
    strcpy(p->from, p->i.from);

    imc_recv(p);
  }

  /* if its only to us (ie. not broadcast) don't forward it */
  if (!strcasecmp(imc_mudof(p->to), imc_name))
    return;

  /* check if we should just drop it (policy rules) */
  if (!can_forward(p))
    return;
  
  /* convert a specific destination to a broadcast in some cases */

  to=imc_mudof(p->i.to);

  isbroadcast=!strcmp(to, "*");	  /* broadcasts are, well, broadcasts */
  broadcast=1;		          /* unless we know better, flood packets */
  i=0;  			  /* make gcc happy */
  direct=NULL;			  /* no direct connection to send on */
  
  /* convert 'to' fields that we have a route for to a hop along the route */
  
  if (!isbroadcast &&
      (route=imc_find_reminfo(to, 0)) != NULL &&
      route->route != NULL &&
      !inpath(p->i.path, route->route))	/* avoid circular routing */
  {
    /*  check for a direct connection: if we find it, and the route isn't
     *  to it, then the route is a little suspect.. also send it direct
     */
    if (strcasecmp(to, route->route) &&
	(i=imc_getinfo(to))!=NULL &&
	i->connection)
      direct=i;
    to=route->route;
  }
  
  /* check for a direct connection */
  
  if (!isbroadcast &&
      (i=imc_getinfo(to)) != NULL &&
      i->connection &&
      !(i->flags & IMC_BROADCAST))
    broadcast=0;

  if (broadcast)
  {				/* need to forward a packet */
    /* SPAM fix - hubcnt and who just gave us the packet- shogar */
    int hubcnt,fromhub;
    hubcnt=0;
    fromhub=0;
    strcpy(recievedfrom,imc_lastinpath(p->i.path)); 
    for (rf=imc_connect_list; rf; rf=rf->next)
    {
	if(rf->info && rf->info->name && !strcmp(recievedfrom,rf->info->name))
        {
        	if(rf->info->flags & IMC_HUB) 
			fromhub=1;
	}
    }
    /* end SPAM fix */

    for (c=imc_connect_list; c; c=c->next)
      if (c->state==IMC_CONNECTED)
      {
	/* don't forward to sites that have already received it,
	 * or sites that don't need this packet
	 */
	if (inpath(p->i.path, c->info->name) ||
	    (p->i.stamp & c->info->noforward)!=0)
	  continue;
        /* SPAM fix - shogar */
        if(c->info->flags & IMC_HUB) 
        {
	    	if(!imc_is_router)
            	{
			if (fromhub)
				continue;
			if(hubcnt)
			{
				continue;
			}
			else
			{
				hubcnt=1;
			}
		}
/* if for imc3 we need to do this - shogar */
/*
        	if (imc_getkeyi(&p->data,"channel",0) == 2)
			continue;
*/
	}
	/* end SPAM fix */
	do_send_packet(c, p);
      }
  }
  else
    /* forwarding to a specific connection */
  {
    /* but only if they haven't seen it (sanity check) */
    if (i->connection && !inpath(p->i.path, i->name))
      do_send_packet(i->connection, p);

    /* send on direct connection, if we have one */
    if (direct && direct!=i && direct->connection &&
	!inpath(p->i.path, direct->name))
      do_send_packet(direct->connection, p);
  }
}

/* handle a password from a client */
static void clientpassword(imc_connect *c, const char *argument)
{
  char arg1[3], name[IMC_MNAME_LENGTH], pw[IMC_PW_LENGTH], version[20];
  imc_info *i;
  char response[IMC_PACKET_LENGTH];

  argument=imc_getarg(argument, arg1, 4);      /* packet type (has to be PW) */
  argument=imc_getarg(argument, name, IMC_MNAME_LENGTH);  /* remote mud name */
  argument=imc_getarg(argument, pw, IMC_PW_LENGTH);	         /* password */
  argument=imc_getarg(argument, version, 20);	/* optional version=n string */

  if (strcasecmp(arg1, "PW"))
  {
    imc_logstring("%s: non-PW password packet", imc_getconnectname(c));
    do_close(c);
    return;
  }

  /* do we know them, and do they have the right password? */
  i=imc_getinfo(name);
  if (!i || strcmp(i->clientpw, pw))
  {
    if (!i || !(i->flags & IMC_QUIET))
    imc_logstring("%s: password failure for %s", imc_getconnectname(c), name);
    do_close(c);
    return;
  }

  /* deny access if deny flag is set (good for eg. muds that start crashing
   * on rwho)
   */
  if (i->flags & IMC_DENY)
  {
    if (!(i->flags & IMC_QUIET))
      imc_logstring("%s: denying connection", name);
    do_close(c);
    return;
  }

  if (i->connection)	                      /* kill old connections */
    do_close(i->connection);

  /* register them */
  i->connection     = c;

  c->state          = IMC_CONNECTED;
  c->info           = i;
  c->spamcounter1   = 0;
  c->spamcounter2   = 0;

  /* check for a version string (assume version 0 if not present) */
  if (sscanf(version, "version=%hu", &c->version)!=1)
    c->version=0;

  /* check for generator/interpreter */
  if (!imc_vinfo[c->version].generate ||
      !imc_vinfo[c->version].interpret)
  {
    if (!(i->flags & IMC_QUIET))
    imc_logstring("%s: unsupported version %d",
		  imc_getconnectname(c), c->version);
    do_close(c);
    return;
  }

  /* send our response */

  sprintf(response, "PW %s %s version=%d",
	  imc_name, i->serverpw, IMC_VERSION);
  do_send(c, response);

  if (!(i->flags & IMC_QUIET))
    imc_logstring("%s: connected (version %d)",
		  imc_getconnectname(c), c->version);

  c->info->timer_duration=IMC_MIN_RECONNECT_TIME;
  c->info->last_connected=imc_now;
  imc_cancel_event(ev_login_timeout, c);
  imc_cancel_event(ev_reconnect, c->info);
}

/* handle a password response from a server */
static void serverpassword(imc_connect *c, const char *argument)
{
  char arg1[3], name[IMC_MNAME_LENGTH], pw[IMC_PW_LENGTH], version[20];
  imc_info *i;

  argument=imc_getarg(argument, arg1, 4);	/* has to be PW */
  argument=imc_getarg(argument, name, IMC_MNAME_LENGTH);
  argument=imc_getarg(argument, pw, IMC_PW_LENGTH);
  argument=imc_getarg(argument, version, 20);

  if (strcasecmp(arg1, "PW"))
  {
    imc_logstring("%s: non-PW password packet", imc_getconnectname(c));
    do_close(c);
    return;
  }

  i=imc_getinfo(name);
  if (!i || strcmp(i->serverpw, pw) ||
      i != c->info)
  {
    if ((!i || !(i->flags & IMC_QUIET)) && !(c->info->flags & IMC_QUIET))
    imc_logstring("%s: password failure for %s", imc_getconnectname(c), name);
    do_close(c);
    return;
  }

  if (i->connection)	/* kill old connections */
    do_close(i->connection);

  i->connection         = c;

  c->state              = IMC_CONNECTED;
  c->spamcounter1       = 0;
  c->spamcounter2       = 0;

  /* check for a version string (assume version 0 if not present) */
  if (sscanf(version, "version=%hu", &c->version)!=1)
    c->version=0;

  /* check for generator/interpreter */
  if (!imc_vinfo[c->version].generate ||
      !imc_vinfo[c->version].interpret)
  {
    if (!(i->flags & IMC_QUIET))
      imc_logstring("%s: unsupported version %d",
		    imc_getconnectname(c), c->version);
    do_close(c);
    return;
  }

  if (!(i->flags & IMC_QUIET))
    imc_logstring("%s: connected (version %d)",
		  imc_getconnectname(c), c->version);
  c->info->timer_duration=IMC_MIN_RECONNECT_TIME;
  c->info->last_connected=imc_now;
  imc_cancel_event(ev_login_timeout, c);
  imc_cancel_event(ev_reconnect, c->info);
}

/* start up listening port */
void imc_startup_port(void)
{
  int i;
  struct sockaddr_in sa;

  if (imc_active!=IA_UP)
  {
    imc_logerror("imc_startup_port: called with imc_active=%d", imc_active);
    return;
  }

  if (imc_port==0)
  {
    imc_logerror("imc_startup_port: called with imc_port=0");
    return;
  }
  
  imc_logstring("binding port %d for incoming connections", imc_port);
      
  control = socket(AF_INET, SOCK_STREAM, 0);
  if (control<0)
  {
    imc_lerror("imc_startup_port: socket");
    return;
  }
    
  i=1;
  if (setsockopt(control, SOL_SOCKET, SO_REUSEADDR, (void *)&i,
		 sizeof(i))<0)
  {
    imc_lerror("imc_startup_port: SO_REUSEADDR");
    close(control);
    return;
  }
  
  if ((i=fcntl(control, F_GETFL, 0))<0)
  {
    imc_lerror("imc_startup_port: fcntl(F_GETFL)");
    close(control);
    return;
  }

  if (fcntl(control, F_SETFL, i | O_NONBLOCK)<0)
  {
    imc_lerror("imc_startup_port: fcntl(F_SETFL)");
    close(control);
    return;
  }

  sa.sin_family      = AF_INET;
  sa.sin_port        = htons(imc_port);
  sa.sin_addr.s_addr = imc_bind; /* already in network order */
  
  if (bind(control, (struct sockaddr *)&sa, sizeof(sa))<0)
  {
    imc_lerror("imc_startup_port: bind");
    close(control);
    return;
  }
  
  if (listen(control, 1)<0)
  {
    imc_lerror("imc_startup_port: listen");
    close(control);
    return;
  }
  
  imc_active=IA_LISTENING;
}

/* shut down listening port */
void imc_shutdown_port(void)
{
  if (imc_active!=IA_LISTENING)
  {
    imc_logerror("imc_shutdown_port: called with imc_active=%d", imc_active);
    return;
  }

  imc_logstring("closing listen port");
  close(control);
  imc_active=IA_UP;
}

#ifdef USEIOCTL
/*  this is an ugly hack to generate the send-queue size for an empty queue.
 *  SO_SNDBUF is only supported in some places, and seems to cause problems
 *  under SunOS
 */

/*  connect to the local discard server, and look at the queue size for an
 *  empty socket.
 */
static int getsndbuf(void)
{
  struct sockaddr_in sa;
  int s, queue;

  if ((s=socket(AF_INET, SOCK_STREAM, 0))<0)
    return 0;

  sa.sin_family      = AF_INET;
  sa.sin_addr.s_addr = inet_addr("127.0.0.1");	/* connect to localhost */
  sa.sin_port        = htons(9);                /* 'discard' service */

  if (connect(s, (struct sockaddr *)&sa, sizeof(sa))<0)
  {
    close(s);
    return 0;
  }

  if (ioctl(s, TIOCOUTQ, &queue)<0)
  {
    close(s);
    return 0;
  }

  close(s);
  return queue;
}
#endif

static int lock_prefix(void)
{
  char lockfile[1000];

  sprintf(lockfile, "%slock", imc_prefix);
  imc_lock_file=open(lockfile, O_CREAT|O_EXCL|O_RDWR, 0644);
  if (imc_lock_file<0)
    imc_lock_file=open(lockfile, O_RDWR, 0644);
  if (imc_lock_file<0)
  {
    imc_lerror("lock_prefix: open %s", lockfile);
    return 0;
  }

  //if (lockf(imc_lock_file, F_TLOCK, 1)<0)
  if (imc_lock_file<0) {
    close(imc_lock_file);
    imc_lock_file=-1;
    return 0;
  }

  return 1;
}

static void unlock_prefix(void)
{
  if (imc_lock_file<0)
    return;

  //lockf(imc_lock_file, F_ULOCK, 1);
  close(imc_lock_file);
  imc_lock_file=-1;
}

/* start up IMC */
void imc_startup_network(void)
{
    imc_info *info;
	int toggle=0; /* This is to tell after we've done reconnects if we've connected to a hub or not -- Scion */

  if (imc_active != IA_CONFIG2)
  {
    imc_logerror("imc_startup_network: called with imc_active==%d",
		 imc_active);
    return;
  }

  if (!imc_siteinfo.name[0])
  {
    imc_logerror("InfoName not set, not initializing");
    return;
  }

  if (!imc_siteinfo.email[0])
  {
    imc_logerror("InfoEmail not set, not initializing");
    return;
  }
  
  imc_logstring("network initializing");

  imc_active=IA_UP;

  control=-1;

  if (imc_port)
    imc_startup_port();
  imc_stats.start    = imc_now;
  imc_stats.rx_pkts  = 0;
  imc_stats.tx_pkts  = 0;
  imc_stats.rx_bytes = 0;
  imc_stats.tx_bytes = 0;
  imc_stats.sequence_drops = 0;

  imc_add_event(20, ev_keepalive, NULL, 1);
  /* fill my imclist please - shogar */
  imc_add_event(30, ev_request_keepalive, NULL, 1);

  imc_mail_startup();		/* start up the mailer */

  if (!lock_prefix())
  {
    imc_logstring("another process is using the same config prefix, not autoconnecting.");
    return;
  }

  /* do autoconnects */
  for (info=imc_info_list; info; info=info->next)
    if (!(info->flags & IMC_NOAUTO) && !(info->flags & IMC_CLIENT) 
		&& !(info->flags & IMC_DENY) 
 /*               && !(info->flags & IMC_OLD_HUB)  not used anymore - shogar
                && !(info->flags & IMC_DEAD_HUB)
*/
		&& !(info->flags & IMC_HUB)) {
      if (imc_connect_to(info->name) && ((info->flags & IMC_MAIN_HUB) || (info->flags & IMC_HUB)))
		  toggle=1;
	}

  
	/* Setup optimization to run, it won't if we're a hub -- Scion */
   if(imc_hubswitch && !imc_is_router)
   {
   	imc_add_event(360, ev_imc_pollforhub, NULL, 1);
   	imc_add_event(420, ev_imc_optimize, NULL, 1);

   	imc_logstring("Setting auto-optimize to run in 6 minutes.");
   }

   /* Are we connected to any hubs, btw? -- Scion */

   if (!toggle) { /* No hubs connected! */
	   for (info=imc_info_list; info; info=info->next)
		   if ((info->flags & IMC_HUB) && (!toggle))
			   if (imc_connect_to(info->name))
				   toggle=0;
   }
   /* We're out of ideas, notify the admins -- Scion */
/*
   if (!toggle)
	   imc_logstring("No hubs connected! Check the configuration!");
*/
}

void imc_startup(const char *prefix)
{
  if (imc_active!=IA_NONE)
  {
    imc_logstring("imc_startup: called with imc_active=%d", imc_active);
    return;
  }

  imc_now=time(NULL);                  /* start our clock */
  imc_boot=imc_now;

  imc_logstring("%s initializing", IMC_VERSIONID);

#ifdef USEIOCTL
  outqsize = getsndbuf();
  imc_logstring("found TIOCOUTQ=%d", outqsize);
#endif

  imc_prefix=imc_strdup(prefix);

  imc_sequencenumber=imc_now;
  strcpy(imc_lasterror, "no error");

  imc_readconfig();
  imc_readignores();

  imc_active = imc_name ? IA_CONFIG2 : IA_CONFIG1;

  if (imc_active==IA_CONFIG2)
    imc_startup_network();
}

void imc_shutdown_network(void)
{
  imc_event *ev, *ev_next;
  imc_connect *c, *c_next;
  imc_reminfo *p, *pnext;

  if (imc_active < IA_UP)
  {
    imc_logerror("imc_shutdown_network: called with imc_active==%d",
		 imc_active);
    return;
  }

  if (imc_lock)
  {
    imc_logerror("imc_shutdown_network: called from within imc_idle_select");
    return;
  }

  imc_logstring("shutting down network");

  if (imc_active == IA_LISTENING)
    imc_shutdown_port();

  imc_logstring("rx %ld packets, %ld bytes (%ld/second)",
		imc_stats.rx_pkts,
		imc_stats.rx_bytes,
		(imc_now == imc_stats.start) ? 0 :
		imc_stats.rx_bytes / (imc_now - imc_stats.start));
  imc_logstring("tx %ld packets, %ld bytes (%ld/second)",
		imc_stats.tx_pkts,
		imc_stats.tx_bytes,
		(imc_now == imc_stats.start) ? 0 :
		imc_stats.tx_bytes / (imc_now - imc_stats.start));
  imc_logstring("largest packet %d bytes", imc_stats.max_pkt);
  imc_logstring("dropped %d packets by sequence number",
                imc_stats.sequence_drops);

  imc_mail_shutdown();

  for (c=imc_connect_list; c; c=c_next)
  {
    c_next=c->next;
    do_close(c);
    imc_extract_connect(c);
  }
  imc_connect_list=NULL; 
  if(!imc_is_router)
  	icec_shutdown();
  for (p=imc_reminfo_list; p; p=pnext)
  {
    pnext=p->next;
    imc_strfree(p->version);
    imc_strfree(p->name);
    if(p->path) imc_strfree(p->path); 
    imc_free(p, sizeof(imc_reminfo));
  }
  imc_reminfo_list=NULL;

  for (ev=imc_event_list; ev; ev=ev_next)
  {
    ev_next=ev->next;
    imc_free(ev, sizeof(imc_event));
  }
  for (ev=imc_event_free; ev; ev=ev_next)
  {
    ev_next=ev->next;
    imc_free(ev, sizeof(imc_event));
  }
  imc_event_list=imc_event_free=NULL;

  unlock_prefix();

  imc_active=IA_CONFIG2;
}

/* close down imc */
void imc_shutdown(void)
{
  imc_ignore_data *ign, *ign_next;
  imc_info *info, *info_next;

  if (imc_active==IA_NONE)
  {
    imc_logerror("imc_shutdown: called with imc_active==0");
    return;
  }

  if (imc_active>=IA_UP)
    imc_shutdown_network();

  for (ign=imc_ignore_list; ign; ign=ign_next)
  {
    ign_next=ign->next;
    imc_freeignore(ign);
  }
  imc_ignore_list=NULL;

  for (info=imc_info_list; info; info=info_next)
  {
    info_next=info->next;
    imc_delete_info(info);
  }
  imc_info_list=NULL;

  if (imc_active >= IA_UP)
    imc_shutdown_network();

  imc_strfree(imc_prefix);
  imc_prefix=NULL;

  if (imc_active >= IA_CONFIG2)
    imc_strfree(imc_name);

  imc_name=NULL;
  imc_active=IA_NONE;
}

/* interpret an incoming packet using the right version */
static imc_packet *do_interpret_packet(imc_connect *c, const char *line)
{
  int v;
  imc_packet *p;

  if (!line[0])
    return NULL;

  v=c->version;
  if (v>IMC_VERSION)
    v=IMC_VERSION;

  p=(*imc_vinfo[v].interpret)(line);
  if (p)
  {
    if (c->info)
    {
      p->i.stamp=c->info->rcvstamp;
    }
    else
    {
      p->i.stamp=0;
    }
  }

  return p;
}

int imc_fill_fdsets(int maxfd, fd_set *read, fd_set *write, fd_set *exc)
{
  imc_connect *c;

  if (imc_active<IA_UP)
    return maxfd;

  /* set up fd_sets for select */

  if (imc_active>=IA_LISTENING)
  {
    if (maxfd < control)
      maxfd = control;
    FD_SET(control, read);
  }

  for (c=imc_connect_list; c; c=c->next)
  {
    if (maxfd < c->desc)
      maxfd = c->desc;

    switch (c->state)
    {
    case IMC_CONNECTING:	/* connected/error when writable */
      FD_SET(c->desc, write);
      break;
    case IMC_CONNECTED:
    case IMC_WAIT1:
    case IMC_WAIT2:
      FD_SET(c->desc, read);
      if (c->outbuf[0])
	FD_SET(c->desc, write);
      break;
    }
  }

  return maxfd;
}

int imc_get_max_timeout(void)
{
  imc_event *p;

  for (p=imc_event_list; p; p=p->next)
    if (p->timed)
      return p->when - imc_now;

  return 60; /* make sure we don't get too backlogged with events */
}

/* shell around imc_idle_select */
void imc_idle(int s, int us)
{
  fd_set read, write, exc;
  int maxfd;
  struct timeval timeout;
  int i;

  FD_ZERO(&read);
  FD_ZERO(&write);
  FD_ZERO(&exc);

  maxfd=imc_fill_fdsets(0, &read, &write, &exc);
  timeout.tv_sec = s;
  timeout.tv_usec = us;

  if (maxfd)
    while ((i=select(maxfd+1, &read, &write, &exc, &timeout)) < 0 &&
	   errno == EINTR)	/* loop, ignoring signals */
      ;
  else
    while ((i=select(0, NULL, NULL, NULL, &timeout)) < 0 &&
	   errno == EINTR)
      ;
    

  if (i<0)
  {
    imc_lerror("imc_idle: select");
    imc_shutdown_network();
    return;
  }

  imc_idle_select(&read, &write, &exc, time(NULL));
}

/* low-level idle function: read/write buffers as needed, etc */
void imc_idle_select(fd_set *read, fd_set *write, fd_set *exc, time_t now)
{
  const char *command;
  imc_packet *p;
  imc_connect *c, *c_next ;

  if (imc_active<IA_CONFIG1)
    return;

  if (imc_lock)
  {
    imc_logerror("imc_idle_select: recursive call");
    return;
  }

  imc_lock=1;

  if (imc_sequencenumber < (unsigned long)imc_now)
    imc_sequencenumber=(unsigned long)imc_now;

  imc_run_events(now);

  if (imc_active<IA_UP)
  {
    imc_lock=0;
    return;
  }

  /* handle results of the select */

  if (imc_active >= IA_LISTENING &&
      FD_ISSET(control, read))
    do_accept();

  for (c=imc_connect_list; c; c=c_next)
  {
    c_next=c->next;

    if (c->state!=IMC_CLOSED && FD_ISSET(c->desc, exc))
      do_close(c);

    if (c->state!=IMC_CLOSED && FD_ISSET(c->desc, read))
      do_read(c);

    while (c->state!=IMC_CLOSED &&
//	   (c->spamtime1>=0 || c->spamcounter1<=IMC_SPAM1MAX) &&
//	   (c->spamtime2>=0 || c->spamcounter2<=IMC_SPAM2MAX) &&
	   (command = imc_getline(c->inbuf)) != NULL)
    {
      if (strlen(command) > imc_stats.max_pkt)
	imc_stats.max_pkt=strlen(command);

//      imc_debug(c, 0, command);	/* log incoming packets */

      switch (c->state)
      {
      case IMC_CLOSED:
	break;
      case IMC_WAIT1:
	clientpassword(c, command);
	break;
      case IMC_WAIT2:
	serverpassword(c, command);
	break;
      case IMC_CONNECTED:
	p = do_interpret_packet(c, command);
	if (p)
	{
#ifdef IMC_PARANOIA
	  /* paranoia: check the last entry in the path is the same as the
	   * sending mud. Also check the first entry to see that it matches
	   * the sender.
	   */

	  imc_stats.rx_pkts++;

	  if (strcasecmp(c->info->name,
			 imc_lastinpath(p->i.path)))
	    imc_logerror("PARANOIA: packet from %s allegedly from %s",
			 c->info->name,
			 imc_lastinpath(p->i.path));
	  else if (strcasecmp(imc_mudof(p->i.from), imc_firstinpath(p->i.path)))
	    imc_logerror("PARANOIA: packet from %s has firstinpath %s",
			 p->i.from,
			 imc_firstinpath(p->i.path));
	  else
	    forward(p);		/* only forward if its a valid packet! */
#else
	  imc_stats.rx_pkts++;
	  forward(p);
#endif
#ifdef SPAMPROT
  if (!strcasecmp(p->type, "chat") ||
      !strcasecmp(p->type, "tell") ||
      !strcasecmp(p->type, "emote") || 1)
  {
      if (!c->spamcounter1 && !c->spamtime1)
	imc_add_event(IMC_SPAM1INTERVAL, ev_spam1, c, 0);
      c->spamcounter1++;

      if (!c->spamcounter2 && !c->spamtime2)
	imc_add_event(IMC_SPAM2INTERVAL, ev_spam2, c, 0);
      c->spamcounter2++;
  }
#endif
	  imc_freedata(&p->data);
	}
	break;
      }
    }
  }

  for (c=imc_connect_list; c; c=c_next)
  {
    c_next=c->next;
    
    if (c->state!=IMC_CLOSED &&
	(FD_ISSET(c->desc, write) || c->newoutput))
    {
//      c->newoutput=0;
      do_write(c);
      c->newoutput=c->outbuf[0];
    }
  }

  for (c=imc_connect_list; c; c=c_next)
  {
    c_next=c->next;

    if (c->state==IMC_CLOSED)
      imc_extract_connect(c);
  }

  imc_lock=0;
}

/* connect to given mud */
int imc_connect_to(const char *mud)
{
  imc_info *i;
  imc_connect *c;
  int desc;
  struct sockaddr_in sa;
  char buf[IMC_DATA_LENGTH];
  int r;

  if (imc_active == IA_NONE)
  {
    imc_qerror("IMC is not active");
    return 0;
  }
    
  i=imc_getinfo(mud);
  if (!i)
  {
    imc_qerror("%s: unknown mud name", mud);
    return 0;
  }

  if (i->connection)
  {
    imc_qerror("%s: already connected", mud);
    return 0;
  }

  if (i->flags & IMC_CLIENT)
  {
    imc_qerror("%s: client-only flag is set", mud);
    return 0;
  }

  if (i->flags & IMC_DENY)
  {
    imc_qerror("%s: deny flag is set", mud);
    return 0;
  }

  if (!(i->flags & IMC_QUIET))
    imc_logstring("connect to %s", mud);

  /*  warning: this blocks. It would be better to farm the query out to
   *  another process, but that is difficult to do without lots of changes
   *  to the core mud code. You may want to change this code if you have an
   *  existing resolver process running.
   */

  if ((sa.sin_addr.s_addr=inet_addr(i->host)) == -1UL)
  {
    struct hostent *hostinfo;

    if (NULL == (hostinfo=gethostbyname(i->host)))
    {
      imc_logerror("imc_connect: couldn't resolve hostname");
      return 0;
    }

    sa.sin_addr.s_addr = *(unsigned long *) hostinfo->h_addr;
  }

  sa.sin_port   = htons(i->port);
  sa.sin_family = AF_INET;

  desc=socket(AF_INET, SOCK_STREAM, 0);
  if (desc<0)
  {
    imc_lerror("socket");
    return 0;
  }

  r=fcntl(desc, F_GETFL, 0);
  if (r<0 || fcntl(desc, F_SETFL, O_NONBLOCK | r)<0)
  {
    imc_lerror("imc_connect: fcntl");
    close(desc);
    return 0;
  }

  if (connect(desc, (struct sockaddr *)&sa, sizeof(sa))<0)
    if (errno != EINPROGRESS)
    {
      imc_lerror("connect");
      close(desc);
      return 0;
    }

  c=imc_new_connect();

  c->desc     = desc;
  c->state    = IMC_CONNECTING;
  c->info     = i;

  imc_add_event(IMC_LOGIN_TIMEOUT, ev_login_timeout, c, 1);

  sprintf(buf, "PW %s %s version=%d",
	  imc_name,
	  i->clientpw,
	  IMC_VERSION);
  do_send(c, buf);

  return 1;
}

int imc_disconnect(const char *mud)
{
  imc_connect *c;
  imc_info *i;
  int d;

  if (imc_active == IA_NONE)
  {
    imc_qerror("IMC is not active");
    return 0;
  }

  if ((d=atoi(mud))!=0)
  {
    /* disconnect a specific descriptor */

    for (c=imc_connect_list; c; c=c->next)
      if (c->desc==d)
      {
        imc_logstring("disconnect descriptor %s", imc_getconnectname(c));
	do_close(c);
	return 1;
      }

    imc_qerror("%d: no matching descriptor", d);
    return 0;
  }
    
  i=imc_getinfo(mud);
  if (!i)
  {
    if (strcasecmp(mud, "unknown")) /* disconnect all unknown muds */
    {
      imc_qerror("%s: unknown mud", mud);
      return 0;
    }
  }

  imc_logstring("disconnect %s", mud);

  for (c=imc_connect_list; c; c=c->next)
    if (c->info==i)
      do_close(c);

  return 1;
}

void imc_send(imc_packet *p)
{
  if (imc_active < IA_UP)
  {
    imc_logerror("imc_send when not active!");
    return;
  }
  
  /* initialize packet fields that the caller shouldn't/doesn't set */

  p->i.stamp = 0;
  p->i.path[0]  = 0;
  
  p->i.sequence = imc_sequencenumber++;
  if (!imc_sequencenumber)
    imc_sequencenumber++;
  
  imc_sncpy(p->i.to, p->to, IMC_NAME_LENGTH);
  
  imc_sncpy(p->i.from, p->from, IMC_NAME_LENGTH);
  strcat(p->i.from, "@");
  imc_sncpy(p->i.from + strlen(p->i.from), imc_name,
	    IMC_NAME_LENGTH - strlen(p->i.from));

  forward(p);
}

imc_info *imc_new_info()
{
  imc_info *i, *p;

  i=imc_malloc(sizeof(*i));

  i->name       = NULL;
  i->host       = NULL;
  i->port       = 0;
  i->connection = NULL;
  i->clientpw   = NULL;
  i->serverpw   = NULL;
  i->timer_duration = IMC_MIN_RECONNECT_TIME;
  i->rcvstamp   = 0;
  i->noforward  = 0;
  i->flags      = 0;
  i->last_connected = 0;

  /* ugly hack, but Too Bad, I don't want another global floating around */
  i->next=NULL;

  for (p=imc_info_list; p && p->next; p=p->next)
    ;

  if (!p)
    imc_info_list=i;
  else
    p->next=i;

  return i;
}
imc_info *imc_insert_info()
{
  imc_info *i;

  i=imc_malloc(sizeof(*i));

  i->name       = NULL;
  i->host       = NULL;
  i->port       = 0;
  i->connection = NULL;
  i->clientpw   = NULL;
  i->serverpw   = NULL;
  i->timer_duration = IMC_MIN_RECONNECT_TIME;
  i->rcvstamp   = 0;
  i->noforward  = 0;
  i->flags      = 0;
  i->last_connected = 0;

  /* ugly hack, but Too Bad, I don't want another global floating around */
  i->next=imc_info_list;
  imc_info_list=i;
  return i;
}

void imc_delete_info(imc_info *i)
{
  imc_connect *c;
  imc_info *last;

  for (c=imc_connect_list; c; c=c->next)
    if (c->info==i)
      do_close(c);

  if (i==imc_info_list)
    imc_info_list=i->next;
  else
  {
    for (last=imc_info_list; last && last->next!=i; last=last->next)
      ;

    if (!last)
      imc_logerror("imc_delete_info: not in list");
    else
      last->next=i->next;
  }

  if (i->name)
    imc_strfree(i->name);
  if (i->host)
    imc_strfree(i->host);
  if (i->clientpw)
    imc_strfree(i->clientpw);
  if (i->serverpw)
    imc_strfree(i->serverpw);

  imc_cancel_event(NULL, i);

  imc_free(i, sizeof(*i));
}
void imc_cancel_info(imc_info *i)
{
  imc_info *last;

  if (i==imc_info_list)
    imc_info_list=i->next;
  else
  {
    for (last=imc_info_list; last && last->next!=i; last=last->next)
      ;

    if (!last)
      imc_logerror("imc_delete_info: not in list");
    else
      last->next=i->next;
  }

  if (i->name)
    imc_strfree(i->name);
  if (i->host)
    imc_strfree(i->host);
  if (i->clientpw)
    imc_strfree(i->clientpw);
  if (i->serverpw)
    imc_strfree(i->serverpw);

  imc_free(i, sizeof(*i));
}
