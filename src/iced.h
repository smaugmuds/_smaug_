/*
 * IMC2 - an inter-mud communications protocol
 *
 * iced.h: IMC-channel-extensions daemon defs
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

#ifndef ICED_H
#define ICED_H

#include "ice.h"

/* who is allowed to create channels? */
#define ICED_CREATORS "shogar@drgnbane ntanel@ucmm shogar@ucode ntanel@ucode scion@ucmm scion@apn"

/* refresh timeout */
#define ICED_REFRESH_TIME 300

extern ice_channel *iced_channel_list;

ice_channel *iced_find_channel(const char *name);

/* main packet dispatcher */
int iced_recv(const imc_packet *p, int bcast);
/* ice-command dispatcher */
void iced_recv_command(const char *from, const char *chan, const char *cmd, const char *data, int override);
/* ice-command subcommands */
void iced_policy   (ice_channel *c, const char *cname, const char *from, const char *data);
void iced_addop    (ice_channel *c, const char *cname, const char *from, const char *data);
void iced_removeop (ice_channel *c, const char *cname, const char *from, const char *data);
void iced_invite   (ice_channel *c, const char *cname, const char *from, const char *data);
void iced_uninvite (ice_channel *c, const char *cname, const char *from, const char *data);
void iced_exclude  (ice_channel *c, const char *cname, const char *from, const char *data);
void iced_unexclude(ice_channel *c, const char *cname, const char *from, const char *data);
void iced_create   (ice_channel *c, const char *cname, const char *from, const char *data);
void iced_destroy  (ice_channel *c, const char *cname, const char *from, const char *data);
void iced_refresh  (ice_channel *c, const char *cname, const char *from, const char *data);
void iced_list     (ice_channel *c, const char *cname, const char *from, const char *data);

void iced_recv_msg_p(const char *from, const char *chan, const char *txt, int emote);
void iced_recv_msg_b(const char *from, const char *chan);
void iced_recv_refresh(const char *from, const char *chan);
void iced_recv_join(const char *from, const char *chan);
void iced_recv_leave(const char *from, const char *chan);
void iced_update(ice_channel *c, const char *to);

void ev_iced_chanlist(void *data);
void ev_iced_refresh(void *data);

void iced_init(void);

void iced_gannounce(const char *fmt, ...) __attribute__((format(printf,1,2)));
void iced_announce(ice_channel *c, const char *fmt, ...) __attribute__((format(printf,2,3)));

#endif
