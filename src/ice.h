/*#################################################################
  #                                              |                #
  #  ******* **    **  ******  **    **  ******  |                #
  # ******** ***  *** ******** **    ** ******** |    \\._.//     #
  # **       ******** **    ** **    ** **       |    (0...0)     #
  # *******  ******** ******** **    ** **  **** |     ).:.(      #
  #  ******* ** ** ** ******** **    ** **  **** |     {o o}      #
  #       ** **    ** **    ** **    ** **    ** |    / ' ' \     #
  # ******** **    ** **    ** ******** ******** | -^^.VxvxV.^^-  #
  # *******  **    ** **    **  ******   ******  |                #
  #                                              |                #
  # ------------------------------------------------------------- #
  # [S]imulated [M]edieval [A]dventure Multi[U]ser [G]ame         #
  # ------------------------------------------------------------- #
  # SMAUG 1.4 © 1994, 1995, 1996, 1998  by Derek Snider           #
  # ------------------------------------------------------------- #
  # SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,         #
  # Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,         #
  # Tricops, Fireblade, Edmond, Conran                            #
  # ------------------------------------------------------------- #
  # Merc 2.1 Diku Mud improvments copyright © 1992, 1993 by       #
  # Michael Chastain, Michael Quan, and Mitchell Tse.             #
  # Original Diku Mud copyright © 1990, 1991 by Sebastian Hammer, #
  # Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja #
  # Nyboe. Win32 port by Nick Gammon                              #
  # ------------------------------------------------------------- #
  # --{smaug}-- 1.8.x © 2014-2015 by Antonio Cao @(burzumishi)    #
  # ------------------------------------------------------------- #
  #             IMC-channel-extensions common defs                #
  #################################################################*/

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
