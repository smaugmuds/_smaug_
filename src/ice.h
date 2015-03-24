/*
 * IMC2 - an inter-mud communications protocol
 *
 * ice.h: IMC-channel-extensions common defs
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

#ifndef ICE_H
#define ICE_H

/* channel policy */

#define ICE_OPEN    1
#define ICE_CLOSED  2
#define ICE_PRIVATE 3

/* defined in icec-mercbase.h, etc */
struct _icec_lchannel;
typedef struct _icec_lchannel icec_lchannel;

typedef struct _ice_channel
{
  char *name;         /* name of channel */
  char *owner;        /* owner (singular) of channel */
  char *operators;    /* current operators of channel */

  int policy;

  char *invited;
  char *excluded;
  char *active;
  
  icec_lchannel *local;
  
  struct _ice_channel *next;
} ice_channel;

int ice_audible(ice_channel *c, const char *who);
const char *ice_nameof(const char *fullname);
const char *ice_mudof(const char *fullname);

#endif
