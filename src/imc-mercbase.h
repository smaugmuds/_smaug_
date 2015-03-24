/*
 * IMC2 - an inter-mud communications protocol
 *
 * imc-mercbase.h: integrated interface defs for Merc-derived codebases.
 *                 Now also includes Circle defs.
 *
 * Copyright (C) 1999 Peter Keeler <Scion@usa.net> and Gregor Moody <moodyg@erols.com>
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

#ifndef IMC_MERCBASE_H
#define IMC_MERCBASE_H

#include "imc-mercdefs.h"

/* Stick your mud ad in this #define. */
#define IMC_MUD_INFO "No MUD information has been defined.\n\r"

/* #define CIRCLE */
/* for Ack! : you need to include the board vnums for the IMC immortal-only and
 * public boards here
 */

#ifdef ACK
#define IMC_BOARD_PUBLIC   1234
#define IMC_BOARD_IMMORTAL 5678
#endif

#ifdef CIRCLE
#define DECLARE_DO_FUN ACMD
#define DEFINE_DO_FUN ACMD   
#define CHAR_DATA struct char_data
#else
#define DEFINE_DO_FUN(x) void x (CHAR_DATA *ch, char *argument)
#endif

/* Color configuration.
 * SMAUG and Ack! -based muds should not need to touch anything here.
 */

#ifdef ACK
#define IMC_COLORCHAR '@'
#define IMC_COLORCHAR2 '@'
#define IMC_COLOR

#elif defined(SMAUG)

#define IMC_COLORCHAR '&'
#define IMC_COLORCHAR2 '^'
#define IMC_COLOR

#elif defined(CIRCLE)

#define IMC_COLORCHAR '&'
#define IMC_COLORCHAR2 '&'

#else

/**  this section is for non-(SMAUG/Ack!) muds  **/

/* uncomment this if your mud supports color */
/* #define IMC_COLOR */

/* change this if your mud supports color to whatever the first character
 * in your color sequences is. There are two possible matches (in case your
 * mud uses more than one prefix, such as SMAUG does)
 */

#define IMC_COLORCHAR '{'
#define IMC_COLORCHAR2 '{'

#endif

/* end of color config */




/* default levels for rtell, rbeep */
#ifdef ACK
#define IMC_LEVEL_RTELL 30
#define IMC_LEVEL_RBEEP 80
#else
#define IMC_LEVEL_RTELL 5
#define IMC_LEVEL_RBEEP 20
#endif



/** no user-servicable parts below here **/

#define IMC_RINFO   0x04
#define IMC_ICE     0x10

#ifdef ACK
#define IMC_INTERACK 0x0100
#endif

#define IMC_RINVIS  0x1000
#define IMC_RBEEP   0x2000
#define IMC_RTELL   0x4000


#ifdef ROM
#define WIZ_IMC       (Y)
#define WIZ_IMCDEBUG  (Z)
#endif

DECLARE_DO_FUN(do_rinfo);

DECLARE_DO_FUN(do_rtell);
DECLARE_DO_FUN(do_rreply);
DECLARE_DO_FUN(do_rwho);
DECLARE_DO_FUN(do_rwhois);
DECLARE_DO_FUN(do_rquery);
DECLARE_DO_FUN(do_rbeep);
DECLARE_DO_FUN(do_rfinger);

DECLARE_DO_FUN(do_imclist);
DECLARE_DO_FUN(do_rsockets);
DECLARE_DO_FUN(do_imc);
DECLARE_DO_FUN(do_imcpoll);
DECLARE_DO_FUN(do_rignore);
DECLARE_DO_FUN(do_rconnect);
DECLARE_DO_FUN(do_rdisconnect);

DECLARE_DO_FUN(do_mailqueue);
DECLARE_DO_FUN(do_istats);

DECLARE_DO_FUN(do_rchannels);
DECLARE_DO_FUN(do_rchanset);

DECLARE_DO_FUN(do_rping);

/* for memory marking (leak checking in conjunction with SAM) */
void imc_markmemory(void (*markfn)(void *));

/* string checking (with SAM+hacks) */
void imc_markstrings(void (*markfn)(char *));

/* color conversion functions, mud->imc and imc->mud */
const char *color_mtoi(const char *);
const char *color_itom(const char *);

/* mail shell */
void imc_post_mail(CHAR_DATA *from, const char *sender, const char *to_list,
                   const char *subject, const char *date, const char *text);

#endif
#ifndef IMC_H
#include "imc.h"
#endif
const imc_char_data *imc_getdata(CHAR_DATA *ch);
int imc_visible(const imc_char_data *viewer, const imc_char_data *viewed);
void process_rwho(const imc_char_data *from, const char *argument);
void process_rfinger(const imc_char_data *from, const char *argument);

