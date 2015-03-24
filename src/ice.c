/*
 * IMC2 - an inter-mud communications protocol
 *
 * ice.c: IMC-channel-extensions (ICE) common code
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
#include "ice.h"

/* see if someone can talk on a channel - lots of string stuff here! */
int ice_audible(ice_channel *c, const char *who)
{
  /* checking a mud? */
  if (!strchr(who, '@'))
  {
    char arg[IMC_DATA_LENGTH];
    const char *p;
    int invited=0;

    if (c->policy != ICE_PRIVATE)
      return 1;

    if (!strcasecmp(ice_mudof(c->name), imc_name))
      return 1;

    /* Private channel - can only hear if someone with the right mud name is
     * there.
     */
    
    p=imc_getarg(c->invited, arg, IMC_NAME_LENGTH);
    while (arg[0])
    {
      if (!strcasecmp(who, arg) ||
	  !strcasecmp(who, imc_mudof(arg)))
      {
	invited=1;
	break;
      }
      
      p=imc_getarg(p, arg, IMC_NAME_LENGTH);
    }

    if (!invited)
      return 0;

    p=imc_getarg(c->excluded, arg, IMC_NAME_LENGTH);
    while (arg[0])
    {
      if (!strcasecmp(who, arg) ||
	  !strcasecmp(who, imc_mudof(arg)))
        return 0;
	
      p=imc_getarg(p, arg, IMC_NAME_LENGTH);
    }

    return 1;
  }

  /* owners and operators always can */
  if (!strcasecmp(c->owner, who) ||
      imc_hasname(c->operators, who))
    return 1;

  /* ICE locally can use any channel */
  if (!strcasecmp(imc_nameof(who), "ICE") &&
      !strcasecmp(imc_mudof(who), imc_name))
    return 1;
  
  if (c->policy == ICE_OPEN)
  {
    /* open policy. default yes. override with excludes, then invites */
    
    if ((imc_hasname(c->excluded, who) ||
	 imc_hasname(c->excluded, imc_mudof(who))) &&
	!imc_hasname(c->invited, who) &&
	!imc_hasname(c->invited, imc_mudof(who)))
      return 0;
    else
      return 1;
  }

  /* closed or private. default no, override with invites, then excludes */
  
  if ((imc_hasname(c->invited, who) ||
       imc_hasname(c->invited, imc_mudof(who))) &&
      !imc_hasname(c->excluded, who) &&
      !imc_hasname(c->excluded, imc_mudof(who)))
    return 1;
  else
    return 0;
}

const char *ice_mudof(const char *fullname)
{
  char *buf=imc_getsbuf(IMC_PNAME_LENGTH);
  char *where=buf;
  int count=0;

  while (*fullname && *fullname != ':' && count < IMC_PNAME_LENGTH-1)
    *where++=*fullname++, count++;

  *where = 0;
  imc_shrinksbuf(buf);
  return buf;
}

const char *ice_nameof(const char *fullname)
{
  char *buf=imc_getsbuf(IMC_MNAME_LENGTH);
  char *where;

  where=strchr(fullname, ':');
  if (!where)
    imc_sncpy(buf, fullname, IMC_MNAME_LENGTH);
  else
    imc_sncpy(buf, where+1, IMC_MNAME_LENGTH);

  imc_shrinksbuf(buf);
  return buf;
}
