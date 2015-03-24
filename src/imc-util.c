/*
 * IMC2 - an inter-mud communications protocol
 *
 * imc-util.c: misc utility functions for IMC
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
 * in general: anything which is called from more than one file and is not
 * obviously an interface function is a candidate for here.
 *
 * specifically:
 * - general string manipulation functions
 * - flag and state lookup functions
 * - error logging
 * - imc_reminfo creation/lookup/deletion
 * - imc_info lookup
 * - connection naming
 * - reconnect setup
 * - static buffer allocation
 */

char imc_lasterror[IMC_DATA_LENGTH];	  /* last error reported */

/*
 *  Static buffer allocation - greatly reduces IMC's memory footprint
 */

/* reserves about 64k for static buffers */
#define ssize (IMC_DATA_LENGTH * 4)
static char sspace[ssize];
static int soffset;
static char *lastalloc;

char *imc_getsbuf(int len)
{
  char *buf;
  
  if (soffset >= ssize-len)
    soffset=0;
  
  buf=&sspace[soffset];
  soffset = (soffset + len) % ssize;

  buf[0]=0;
  lastalloc=buf;
  return buf;
}

void imc_shrinksbuf(char *buf)
{
  int offset;

  if (!buf || buf!=lastalloc)
    return;

  offset=buf-sspace;
  soffset=offset+strlen(buf)+1;
}

/*
 * Key/value manipulation
 */

/* clone packet data */

void imc_clonedata(const imc_data *p, imc_data *n)
{
  int i;

  for (i=0; i<IMC_MAX_KEYS; i++)
  {
    if (p->key[i])
      n->key[i]=imc_strdup(p->key[i]);
    else
      n->key[i]=NULL;
    
    if (p->value[i])
      n->value[i]=imc_strdup(p->value[i]);
    else
      n->value[i]=NULL;
  }
}

/* get the value of "key" from "p"; if it isn't present, return "def" */
const char *imc_getkey(const imc_data * p, const char *key, const char *def)
{
  int i;

  for (i=0; i<IMC_MAX_KEYS; i++)
    if (p->key[i] && !strcasecmp(p->key[i], key))
      return p->value[i];

  return def;
}

/* identical to imc_getkey, except get the integer value of the key */
int imc_getkeyi(const imc_data *p, const char *key, int def)
{
  int i;

  for (i=0; i<IMC_MAX_KEYS; i++)
    if (p->key[i] && !strcasecmp(p->key[i], key))
      return atoi(p->value[i]);

  return def;
}

/* add "key=value" to "p" */
void imc_addkey(imc_data *p, const char *key, const char *value)
{
  int i;

  for (i=0; i<IMC_MAX_KEYS; i++)
    if (p->key[i] && !strcasecmp(key, p->key[i]))
    {
      imc_strfree(p->key[i]);
      imc_strfree(p->value[i]);
      p->key[i]   = NULL;
      p->value[i] = NULL;
      break;
    }

  if (!value)
    return;

  for (i=0; i<IMC_MAX_KEYS; i++)
    if (!p->key[i])
    {
      p->key[i]   = imc_strdup(key);
      p->value[i] = imc_strdup(value);
      return;
    }
}

/* add "key=value" for an integer value */
void imc_addkeyi(imc_data *p, const char *key, int value)
{
  char temp[20];
  sprintf(temp, "%d", value);
  imc_addkey(p, key, temp);
}

/* clear all keys in "p" */
void imc_initdata(imc_data *p)
{
  int i;

  for (i=0; i<IMC_MAX_KEYS; i++)
  {
    p->key[i]   = NULL;
    p->value[i] = NULL;
  }
}

/* free all the keys in "p" */
void imc_freedata(imc_data * p)
{
  int i;

  for (i=0; i<IMC_MAX_KEYS; i++)
  {
    if (p->key[i])
      imc_strfree(p->key[i]);
    if (p->value[i])
      imc_strfree(p->value[i]);
  }
}

/*
 *  Error logging
 */

/* log a string */
void imc_logstring(const char *format, ...)
{
  char buf[IMC_DATA_LENGTH];
  va_list ap;

  va_start(ap, format);
  vsnprintf(buf, IMC_DATA_LENGTH - 1, format, ap);
  va_end(ap);

  imc_log(buf);
}

/* log an error (log string and copy to lasterror) */
void imc_logerror(const char *format,...)
{
  va_list ap;

  va_start(ap, format);
  vsnprintf(imc_lasterror, IMC_DATA_LENGTH - 1, format, ap);
  va_end(ap);

  imc_log(imc_lasterror);
}

/* log an error quietly (just copy to lasterror) */
void imc_qerror(const char *format,...)
{
  va_list ap;

  va_start(ap, format);
  vsnprintf(imc_lasterror, IMC_DATA_LENGTH - 1, format, ap);
  va_end(ap);
}

/* log a system error (log string, ": ", string representing errno)   */
/* this is particularly broken on SunOS (which doesn't have strerror) */
void imc_lerror(const char *format,...)
{
  va_list ap;

  va_start(ap, format);
  vsnprintf(imc_lasterror, IMC_DATA_LENGTH - 1, format, ap);
  strcat(imc_lasterror, ": ");
  strcat(imc_lasterror, strerror(errno));

  imc_log(imc_lasterror);
}

const char *imc_error(void)
{
  return imc_lasterror;
}


/*
 *  String manipulation functions, mostly exported
 */

/* lowercase what */
void imc_slower(char *what)
{
  char *p=what;
  while (*p)
  {
    *p=tolower(*p);
    p++;
  }
}

/* copy src->dest, max count, null-terminate */
void imc_sncpy(char *dest, const char *src, int count)
{
  strncpy(dest, src, count-1);
  dest[count-1] = 0;
}

/* return 'mud' from 'player@mud' */
const char *imc_mudof(const char *fullname)
{
  char *buf=imc_getsbuf(IMC_MNAME_LENGTH);
  char *where;

  where=strchr(fullname, '@');
  if (!where)
    imc_sncpy(buf, fullname, IMC_MNAME_LENGTH);
  else
    imc_sncpy(buf, where+1, IMC_MNAME_LENGTH);

  imc_shrinksbuf(buf);
  return buf;
}

/* return 'player' from 'player@mud' */
const char *imc_nameof(const char *fullname)
{
  char *buf=imc_getsbuf(IMC_PNAME_LENGTH);
  char *where=buf;
  int count=0;

  while (*fullname && *fullname != '@' && count < IMC_PNAME_LENGTH-1)
    *where++=*fullname++, count++;

  *where = 0;
  imc_shrinksbuf(buf);
  return buf;
}

/* return 'player@mud' from 'player' and 'mud' */
const char *imc_makename(const char *player, const char *mud)
{
  char *buf=imc_getsbuf(IMC_NAME_LENGTH);

  imc_sncpy(buf, player, IMC_PNAME_LENGTH);
  strcat(buf, "@");
  imc_sncpy(buf + strlen(buf), mud, IMC_MNAME_LENGTH);
  imc_shrinksbuf(buf);
  return buf;
}

/* return 'e' from 'a!b!c!d!e' */
const char *imc_lastinpath(const char *path)
{
  const char *where;
  char *buf=imc_getsbuf(IMC_NAME_LENGTH);

  where=path + strlen(path)-1;
  while (*where != '!' && where >= path)
    where--;

  imc_sncpy(buf, where+1, IMC_NAME_LENGTH);
  imc_shrinksbuf(buf);
  return buf;
}

/* return 'a' from 'a!b!c!d!e' */
const char *imc_firstinpath(const char *path)
{
  char *buf=imc_getsbuf(IMC_NAME_LENGTH);
  char *p;

  for (p=buf; *path && *path != '!'; *p++=*path++)
    ;

  *p=0;
  imc_shrinksbuf(buf);
  return buf;
}

/*  imc_getarg: extract a single argument (with given max length) from
 *  argument to arg; if arg==NULL, just skip an arg, don't copy it out
 */
const char *imc_getarg(const char *argument, char *arg, int length)
{
  int len = 0;

  while (*argument && isspace(*argument))
    argument++;

  if (arg)
    while (*argument && !isspace(*argument) && len < length-1)
      *arg++=*argument++, len++;
  else
    while (*argument && !isspace(*argument))
      argument++;

  while (*argument && !isspace(*argument))
    argument++;

  while (*argument && isspace(*argument))
    argument++;

  if (arg)
    *arg = 0;

  return argument;
}

/* Check for a name in a list */
int imc_hasname(const char *list, const char *name)
{
    const char *p;
    char arg[IMC_NAME_LENGTH];

    if(!list)
	return(0);

    p=imc_getarg(list, arg, IMC_NAME_LENGTH);
    while (arg[0])
    {
      if (!strcasecmp(name, arg))
        return 1;
      p=imc_getarg(p, arg, IMC_NAME_LENGTH);
    }

    return 0;
}

/* Add a name to a list */
void imc_addname(char **list, const char *name)
{
  char buf[IMC_DATA_LENGTH];

  if (imc_hasname(*list, name))
    return;

  if ((*list)[0])
    sprintf(buf, "%s %s", *list, name);
  else
    strcpy(buf, name);
  
  imc_strfree(*list);
  *list=imc_strdup(buf);
}

/* Remove a name from a list */
void imc_removename(char **list, const char *name)
{
  char buf[1000];
  char arg[IMC_NAME_LENGTH];
  const char *p;
  
  buf[0]=0;
  p=imc_getarg(*list, arg, IMC_NAME_LENGTH);
  while (arg[0])
  {
    if (strcasecmp(arg, name))
    {
      if (buf[0])
	strcat(buf, " ");
      strcat(buf, arg);
    }
    p=imc_getarg(p, arg, IMC_NAME_LENGTH);
  }

  imc_strfree(*list);
  *list=imc_strdup(buf);
}

/*
 *  Flag interpretation
 */

/* look up a value in a table */
const char *imc_statename(int value, const imc_flag_type *table)
{
  int i;

  for (i=0; table[i].name; i++)
    if (value==table[i].value)
      return table[i].name;

  return "unknown";
}

/* return the name of a particular set of flags */
const char *imc_flagname(int value, const imc_flag_type *table)
{
  char *buf=imc_getsbuf(100);
  int i;

  buf[0]=0;

  for (i=0; table[i].name; i++)
    if ((value & table[i].value) == table[i].value)
    {
      strcat(buf, table[i].name);
      strcat(buf, " ");
      value &= ~table[i].value;
    }

  if (buf[0])
    buf[strlen(buf)-1] = 0;
  else
    strcpy(buf, "none");

  imc_shrinksbuf(buf);
  return buf;
}

/* return the value corresponding to a set of names */
int imc_flagvalue(const char *name, const imc_flag_type *table)
{
  char buf[20];
  int i;
  int value = 0;

  while (1)
  {
    name=imc_getarg(name, buf, 20);
    if (!buf[0])
      return value;

    for (i=0; table[i].name; i++)
      if (!strcasecmp(table[i].name, buf))
	value |= table[i].value;
  }
}

/* return the value corresponding to a name */
int imc_statevalue(const char *name, const imc_flag_type *table)
{
  int i;
  char buf[20];

  imc_getarg(name, buf, 20);

  for (i=0; table[i].name; i++)
    if (!strcasecmp(table[i].name, buf))
      return table[i].value;

  return -1;
}

/*
 *  imc_reminfo handling
 */

/* find an info entry for "name" */
imc_reminfo *imc_find_reminfo(const char *name, int type)
{
  imc_reminfo *p;

  for (p = imc_reminfo_list; p; p = p->next)
  {
//    if (p->type == IMC_REMINFO_EXPIRED && !type)
//      continue;
    if (!strcasecmp(name, p->name))
      return p;
  }

  return NULL;
}

/* create a new info entry, insert into list */
imc_reminfo *imc_new_reminfo(void)
{
  imc_reminfo *p;

  p=imc_malloc(sizeof(imc_reminfo));

  p->name    = NULL;
  p->version = NULL;
  p->route   = NULL;
  p->path   = NULL;
  p->alive   = 0;
  p->ping    = 0;
  p->top_sequence = 0;
  p->next    = imc_reminfo_list;
  p->type    = IMC_REMINFO_NORMAL;

  imc_reminfo_list=p;
  return p;
}

/* delete the info entry "p" */
void imc_delete_reminfo(imc_reminfo *p)
{
  imc_reminfo *last;

  if (!imc_reminfo_list || !p)
    return;

  if (p == imc_reminfo_list)
    imc_reminfo_list = p->next;
  else
  {
    for (last=imc_reminfo_list; last && last->next != p; last=last->next)
      ;
    if (!last)
      return;
    last->next=p->next;
  }

  imc_strfree(p->name);
  imc_strfree(p->version);
  imc_strfree(p->route);

  imc_cancel_event(NULL, p);
  imc_free(p, sizeof(*p));
}

/* get info struct for given mud */
imc_info *imc_getinfo(const char *mud)
{
  imc_info *p;

  for (p=imc_info_list; p; p=p->next)
    if (!strcasecmp(mud, p->name))
      return p;

  return NULL;
}

/* get name of a connection */
const char *imc_getconnectname(const imc_connect *c)
{
  char *buf=imc_getsbuf(IMC_NAME_LENGTH);
  const char *n;

  if (c->info)
    n=c->info->name;
  else
    n="unknown";

  sprintf(buf, "%s[%d]", n, c->desc);
  imc_shrinksbuf(buf);
  return buf;
 }

/* set up for a reconnect */
void imc_setup_reconnect(imc_info *i)
{
  time_t temp;
  int t;

  /*  add a bit of randomness so we don't get too many simultaneous
   *  reconnects
   */
  temp=i->timer_duration + (rand()%21) - 20;
  t=imc_next_event(ev_reconnect, i);

  if (t >= 0 && t < temp)
    return;

      /* routers should never do reconnects to muds - shogar */
      /* and limit RECONNECTs to 3 attempts until successful connect */
      /* or they connect to us until we reboot, then we give them a second */
      /* chance */
      	if(!imc_is_router || (i->flags & IMC_HUB)) 
     	{ 
		if(i->connect_attempts < 3)
		{
  			i->timer_duration*=2;
  			if (i->timer_duration > IMC_MAX_RECONNECT_TIME)
    				i->timer_duration = IMC_MAX_RECONNECT_TIME;
			imc_add_event(temp, ev_reconnect, i, 1);
			i->connect_attempts += 1; /* count the attempts - shogar */
		}
	}
}

const char *imc_make_password(void) {
	/* Generate random passwords for the auto-reconnect feature -- Scion */
	time_t temp;
	int a=0;
	char letters[] = "abcdefghijklmnopqrstuvwxyz"; /* You can change this if you want */
	static char buf[9];
	char *pass=NULL;

	
    
	for (a=0; a<9; a++) {
		
		temp=(rand()%(strlen(letters)-1)+1);
		if (temp<0 || temp>strlen(letters)-1)
			temp=0;
		/* We'll make 8 character passwords */
		buf[a]=letters[temp]; 
	}
	buf[8]='\0';

	pass=buf;
	return pass;
}


