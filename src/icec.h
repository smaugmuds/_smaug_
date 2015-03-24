/*
 * IMC2 - an inter-mud communications protocol
 *
 * icec.h: IMC-channel-extensions client defs
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

#ifndef ICEC_H
#define ICEC_H

#include "ice.h"

extern ice_channel *icec_channel_list;

ice_channel *icec_findchannel(const char *name);

void icec_sendmessage(ice_channel *c, const char *name, const char *text, int emote);
const char *icec_command(const char *from, const char *arg);

void icec_recv_msg_r(const char *from, const char *realfrom, const char *chan, const char *txt, int emote);
void icec_recv_msg_b(const char *from, const char *chan, const char *txt, int emote);
void icec_recv_update(const char *from, const char *chan, const char *owner, const char *operators, const char *policy, const char *invited, const char *excluded);
void icec_recv_destroy(const char *from, const char *channel);

/* defined somewhere else */
void icec_showchannel(ice_channel *c, const char *from, const char *text, int emote);
void icec_localfree(ice_channel *c);
ice_channel *icec_findlchannel(const char *name);
void icec_notify_update(ice_channel *c);
void icec_load_channels(void);

void icec_init(void);
void icec_shutdown(void);

#define ICEC_TIMEOUT 650

#endif
