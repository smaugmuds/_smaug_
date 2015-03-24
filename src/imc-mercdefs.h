/*
 * IMC2 - an inter-mud communications protocol
 *
 * imc-mercbase.h: integrated macro defs for Merc-derived codebases.
 *                 Now also includes Circle defs.
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

#ifndef IMC_MERCDEFS_H
#define IMC_MERCDEFS_H

/* LOCAL_LOG_ROUTINE -
If you wish to handle the log messages in your own way, uncomment the
#define below and create a function (in comm.c suggested) called
void imc_local_log(const char* string) and put any log processing you wish
in this function. Like putting the log channels on a different channel.
    - shogar
*/

/* #define LOCAL_LOG_ROUTINE */
/* #define LOCAL_RWHO_REPLY_ROUTINE */
/* #define LOCAL_RFINGER_ROUTINE */


/* mud type configuration.
 * Either uncomment one of these #defines, or add an appropriate
 * -DROM / -DMERC / etc flag to your Makefile
 */

//#define ROM
//#define ROT
//#define MERC
//#define SMAUG /* 102 users */
#define SMAUG14
//#define ENVY  
//#define ACK   
//#define CIRCLE

#ifdef SMAUG14
#ifndef SMAUG
#define SMAUG
#endif
#endif
#ifdef ROT
#ifndef ROM
#define ROM 
#endif
#endif

#if !defined(ROM) && !defined(MERC) && !defined(ENVY) && !defined(SMAUG) && !defined(ACK) && !defined(CIRCLE)
#error You must #define one of ROM, ROT, MERC, SMAUG, ENVY, ACK or CIRCLE
#endif

#ifdef IN_IMC
#ifdef SMAUG
#include "mud.h"
#elif defined(CIRCLE)
#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "interpreter.h"
#include "utils.h"
#include "handler.h"
#include "comm.h"
#include "db.h"
#include "mail.h"

#else
#include "merc.h"
#endif
#endif

#ifdef ROM
#include "recycle.h"
#define IS_NOCHAN(ch) (IS_SET((ch)->comm, COMM_NOCHANNELS))
#define IS_NOTELL(ch) (IS_SET((ch)->comm, COMM_NOTELL))
#define IS_SILENT(ch) (0)
#define IS_QUIET(ch)  (IS_SET((ch)->comm, COMM_QUIET))
/* from note.c: */
void append_note(NOTE_DATA *pnote);
#define imc_to_pager send_to_char
#define imc_to_char send_to_char
#endif /* ROM */


#ifdef MERC
#define IS_NOCHAN(ch) (IS_SET((ch)->act, PLR_SILENCE))
#define IS_NOTELL(ch) (IS_SET((ch)->act, PLR_NO_TELL) || IS_NOCHAN(ch))
#define IS_SILENT(ch) (0)
#define IS_QUIET(ch) (0)
#define imc_to_pager send_to_char
#define imc_to_char send_to_char
#endif /* Merc */


#ifdef SMAUG
#ifdef SMAUG14
#define IS_NOCHAN(ch) (xIS_SET((ch)->act, PLR_SILENCE))
#define IS_SILENT(ch) ((!IS_NPC(ch) && xIS_SET(ch->act,PLR_SILENCE)) || \
		       xIS_SET(ch->in_room->room_flags, ROOM_SILENCE))
#define IS_NOTELL(ch) (xIS_SET((ch)->act, PLR_NO_TELL) || IS_NOCHAN(ch))
#else
#define IS_NOCHAN(ch) (IS_SET((ch)->act, PLR_SILENCE))
#define IS_SILENT(ch) ((!IS_NPC(ch) && IS_SET(ch->act,PLR_SILENCE)) || \
		       xIS_SET(ch->in_room->room_flags, ROOM_SILENCE))
#define IS_NOTELL(ch) (IS_SET((ch)->act, PLR_NO_TELL) || IS_NOCHAN(ch))
#endif
#define IS_QUIET(ch) (0)
#define imc_to_pager send_to_pager_color
#define imc_to_char send_to_char_color
#define descriptor_list first_descriptor
#endif /* SMAUG */


#ifdef ENVY
/* There are 2 versions of the IS_SILENT macro: one that checks the race_table
 * and one that doesn't. the RACE_MUTE flag appeared in one of the Envy 2.0
 * patches, but not everyone has it. Uncomment as needed.
 */
#define IS_NOCHAN(ch) (IS_SET((ch)->act, PLR_SILENCE))
#define IS_NOTELL(ch) (IS_SET((ch)->act, PLR_NO_TELL) || IS_NOCHAN(ch))
#define IS_SILENT(ch) ((IS_AFFECTED((ch),AFF_MUTE) || \
	  /* IS_SET(race_table[(ch)->race].race_abilities, RACE_MUTE) || */ \
             xIS_SET((ch)->in_room->room_flags, ROOM_CONE_OF_SILENCE)))
#define IS_QUIET(ch) (0)
#define imc_to_pager send_to_char
#define imc_to_char send_to_char
#endif /* Envy */


#ifdef ACK
#define IS_NOCHAN(ch) (IS_SET((ch)->act, PLR_SILENCE))
#define IS_NOTELL(ch) (IS_SET((ch)->act, PLR_NO_TELL) || IS_NOCHAN(ch))
#define IS_SILENT(ch) (IS_SET(ch->act,PLR_SILENCE) || \
		       xIS_SET(ch->in_room->room_flags, ROOM_QUIET))
#define IS_QUIET(ch) (0)
#define imc_to_pager send_to_char
#define imc_to_char send_to_char
#define descriptor_list first_desc
#endif

#ifdef CIRCLE
#define NEED_STR_PREFIX 1

typedef struct descriptor_data DESCRIPTOR_DATA;

#define AFF_DETECT_HIDDEN AFF_SENSE_LIFE

#define LEVEL_IMMORTAL LVL_IMMORT
#define LEVEL_HERO LVL_IMMORT
#define MAX_LEVEL LVL_IMPL

#define IS_NOCHAN(ch) (PRF_FLAGGED(ch, PRF_DEAF))
#define IS_NOTELL(ch) (PRF_FLAGGED(ch, PRF_NOTELL))
#define IS_SILENT(ch) ((!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_DEAF)) || \
                       ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF))
#define IS_QUIET(ch)  (0)

#define IMC_DENY_FLAGS(ch) ((ch)->player_specials->saved.imc_deny)
#define IMC_ALLOW_FLAGS(ch) ((ch)->player_specials->saved.imc_allow)
#define IMC_DEAF_FLAGS(ch) ((ch)->player_specials->saved.imc_deaf)
#define IMC_RREPLY(ch) ((ch)->player_specials->rreply)
#define IMC_RREPLY_NAME(ch) ((ch)->player_specials->rreply_name)
#define ICE_LISTEN(ch) ((ch)->player_specials->ice_listen)

#define IS_IMMORTAL(ch) (GET_LEVEL(ch) >= LVL_IMMORT)

#define PULSE_PER_SECOND PASSES_PER_SEC

#define imc_to_pager(str, ch) page_string((ch)->desc, (char*) str, 0)
#define imc_to_char send_to_char
#define get_trust(ch) GET_LEVEL(ch)
#define log_string log
#define is_name is_abbrev
#define get_char_world get_char_vis

extern struct descriptor_data *descriptor_list;
extern struct room_data *world;

#else

#define IMC_DENY_FLAGS(ch) ((ch)->pcdata->imc_deny)
#define IMC_ALLOW_FLAGS(ch) ((ch)->pcdata->imc_allow)
#define IMC_DEAF_FLAGS(ch) ((ch)->pcdata->imc_deaf)
#define IMC_RREPLY(ch) ((ch)->pcdata->rreply)
#define IMC_RREPLY_NAME(ch) ((ch)->pcdata->rreply_name)
#define ICE_LISTEN(ch) ((ch)->pcdata->ice_listen)

#define GET_NAME(ch) (ch)->name
#ifdef SMAUG
#define GET_LEVEL(ch) (get_trust(ch))
#else
#define GET_LEVEL(ch) (ch)->level
#endif
#define GET_SEX(ch) (ch)->sex
#define GET_TITLE(ch) (ch)->pcdata->title

#endif

#define IS_RINVIS(ch) (!IS_NPC(ch) && \
  IS_SET(IMC_DEAF_FLAGS(ch), IMC_RINVIS))

#define CAN(ch, x, minlev)                 \
(!IS_NPC(ch) &&                            \
 !IS_SET(IMC_DENY_FLAGS(ch), (x)) &&       \
 (GET_LEVEL(ch) >= (minlev) ||             \
  IS_SET(IMC_ALLOW_FLAGS(ch), (x))))

#define CHECKIMC(ch)                                \
do {                                                \
  if (IS_NPC(ch))                                   \
  {                                                 \
    imc_to_char("NPCs cannot use IMC.\n\r", (ch)); \
    return;                                         \
  }                                                 \
  if (imc_active<IA_UP)                             \
  {                                                 \
    imc_to_char("IMC is not active.\n\r", (ch));   \
    return;                                         \
  }                                                 \
} while(0)

#define CHECKMUD(ch,m)                         \
do {                                           \
  if (strcasecmp(m, imc_name) && !imc_find_reminfo(m,1)) \
  {                                            \
    char _buf[100];                            \
    sprintf(_buf, "Warning: '%s' doesn't seem to be active on IMC.\n\r", (m));\
    imc_to_char(_buf, (ch));                  \
  }                                            \
} while(0)

#define CHECKMUDOF(ch,n) CHECKMUD(ch,imc_mudof(n))

#endif

