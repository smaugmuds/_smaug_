
/*
 * IMC2 - an inter-mud communications protocol
 *
 * imc-version.c: packet generation/interpretation for various protocol
 *                versions
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
#include <ctype.h>
#include <string.h>

#define IMC_INTERNALS
#include "imc.h"

static const char *generate2(const imc_packet *);
static imc_packet *interpret2(const char *);

_imc_vinfo imc_vinfo[] =
{
  { 0, NULL, NULL },
  { 1, NULL, NULL },
  { 2, generate2, interpret2 }
};

/* escape2: escape " -> \", \ -> \\, CR -> \r, LF -> \n */

static const char *escape2(const char *data)
{
  char *buf=imc_getsbuf(IMC_DATA_LENGTH);
  char *p;

  for (p=buf; *data && (p-buf < IMC_DATA_LENGTH-1); data++, p++)
  {
    if (*data == '\n')
    {
      *p++='\\';
      *p='n';
    }
    else if (*data == '\r')
    {
      *p++='\\';
      *p='r';
    }
    else if (*data == '\\')
    {
      *p++='\\';
      *p='\\';
    }
    else if (*data == '"')
    {
      *p++='\\';
      *p='"';
    }
    else
      *p=*data;
  }

  *p=0;

  imc_shrinksbuf(buf);
  return buf;
}

/* printkeys: print key-value pairs, escaping values */
static const char *printkeys(const imc_data * data)
{
  char *buf=imc_getsbuf(IMC_DATA_LENGTH);
  char temp[IMC_DATA_LENGTH];
  int len=0;
  int i;

  buf[0]=0;

  for (i=0; i<IMC_MAX_KEYS; i++)
  {
    if (!data->key[i])
      continue;
    imc_sncpy(buf+len, data->key[i], IMC_DATA_LENGTH-len-1);
    strcat(buf, "=");
    len = strlen(buf);

    if (!strchr(data->value[i], ' '))
      imc_sncpy(temp, escape2(data->value[i]), IMC_DATA_LENGTH-1);
    else
    {
      temp[0]='"';
      imc_sncpy(temp+1, escape2(data->value[i]), IMC_DATA_LENGTH-3);
      strcat(temp, "\"");
    }

    strcat(temp, " ");
    imc_sncpy(buf+len, temp, IMC_DATA_LENGTH-len);
    len = strlen(buf);
  }

  imc_shrinksbuf(buf);
  return buf;
}

/* parsekeys: extract keys from string */

static void parsekeys(const char *string, imc_data * data)
{
  const char *p1;
  char *p2;
  char k[IMC_DATA_LENGTH], v[IMC_DATA_LENGTH];
  int quote;

  p1 = string;

  while (*p1)
  {
    while (*p1 && isspace(*p1))
      p1++;

    p2 = k;
    while (*p1 && *p1 != '=' && p2-k < IMC_DATA_LENGTH-1)
      *p2++=*p1++;
    *p2=0;

    if (!k[0] || !*p1)		/* no more keys? */
      break;

    p1++;			/* skip the '=' */

    if (*p1 == '"')
    {
      p1++;
      quote = 1;
    }
    else
      quote = 0;

    p2=v;
    while (*p1 && (!quote || *p1 != '"') && (quote || *p1 != ' ') &&
	   p2-v < IMC_DATA_LENGTH+1)
    {
      if (*p1 == '\\')
      {
	switch (*(++p1))
	{
	case '\\':
	  *p2++='\\';
	  break;
	case 'n':
	  *p2++='\n';
	  break;
	case 'r':
	  *p2++='\r';
	  break;
	case '"':
	  *p2++='"';
	  break;
	default:
	  *p2++=*p1;
	  break;
	}
	if (*p1)
	  p1++;
      }
      else
	*p2++=*p1++;
    }

    *p2=0;

    if (!v[0])
      continue;

    imc_addkey(data, k, v);

    if (quote && *p1)
      p1++;
  }
}

static const char *generate2(const imc_packet * p)
{
  char *temp;
  char newpath[IMC_PATH_LENGTH];

  if (!p->type[0] || !p->i.from[0] || !p->i.to[0])
  {
    imc_logerror("BUG: generate2: bad packet!");
    imc_logerror("type: %s from: %s to: %s",p->type,p->i.from,p->i.to);
    imc_logerror("path: %s data:",p->i.path,printkeys(&p->data));
    return NULL;		/* catch bad packets here */
  }

  if (!p->i.path[0])
    strcpy(newpath, imc_name);
  else
    sprintf(newpath, "%s!%s", p->i.path, imc_name);

  temp=imc_getsbuf(IMC_PACKET_LENGTH);
  sprintf(temp, "%s %lu %s %s %s %s",
	  p->i.from, p->i.sequence, newpath, p->type, p->i.to,
	  printkeys(&p->data));
  imc_shrinksbuf(temp);
  return temp;
}

static imc_packet *interpret2(const char *argument)
{
  char seq[20];
  static imc_packet out;

  imc_initdata(&out.data);
  argument=imc_getarg(argument, out.i.from, IMC_NAME_LENGTH );
  argument=imc_getarg(argument, seq, 20);
  argument=imc_getarg(argument, out.i.path, IMC_PATH_LENGTH );
  argument=imc_getarg(argument, out.type, IMC_TYPE_LENGTH );
  argument=imc_getarg(argument, out.i.to, IMC_NAME_LENGTH );

  if (!out.i.from[0] || !seq[0] || !out.i.path[0] || !out.type[0] || !out.i.to[0])
  {
    imc_logerror("interpret2: bad packet received, discarding");
    return NULL;
  }

  parsekeys(argument, &out.data);

  out.i.sequence=strtoul(seq, NULL, 10);
  return &out;
}
