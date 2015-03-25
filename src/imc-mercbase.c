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
  #    IMC integrated interface code for Merc-derived codebases   #
  #################################################################*/

/*
 * IMC2 - an inter-mud communications protocol
 *
 * imc-mercbase.c: integrated interface code for Merc-derived codebases.
 *                 Now also includes patches for Circle.
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

/* Modifications by:
 *  Erwin S. Andreasen <erwin@pip.dknet.dk>    (Envy changes)
 *  Erin Moeller <altrag@game.org>             (Merc, SMAUG changes)
 *  Stephen Zepp <zenithar@fullnet.net>        (Ack! changes)
 *  Trevor Man <tman@iname.com>                (Circle changes)
 * 
 * integration by Oliver Jowett
 */

#if !defined(CIRCLE)
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#endif

#include "imc.h"

#include "imc-mercdefs.h"
#include "imc-mercbase.h"

extern int imc_log_on;

/* memory allocation hooks */
#ifdef LOCAL_LOG_ROUTINE
  void imc_local_log(const char * string);
#endif
/* void ev_iced_refresh(void *dummy) */
 /*dummy catch cause muds has no iced.c-shogar */
/*
{
}
*/

void *imc_malloc(int size)
{
#if defined(SMAUG) || defined(CIRCLE)
  /* Have to use char because CREATE uses a sizeof, which doesnt like void */
  char *mem;

  CREATE(mem, char, size);
  return (void *)mem;
#elif defined(ACK)
  return getmem(size);
#else
  return alloc_mem(size);
#endif
}

void IMC_free(void *block, int size)
{
  if(!block)
	return;
#ifdef SMAUG
  DISPOSE(block);
#elif defined(ACK)
  dispose(block, size);
#elif defined(CIRCLE)
  free(block);
#else
  free_mem(block, size);
#endif
}

char *imc_strdup(const char *src)
{
  return str_dup(src);
}

void IMC_strfree(char *str)
{
  if(!str)
	return;
#ifdef SMAUG
  DISPOSE(str);
#elif defined(CIRCLE)
  free(str);
#else
  free_string(str);
#endif
}

/* config hooks */

int imc_readconfighook(const char *word, const char *value)
{
  return 0;
}

void imc_saveconfighook(FILE *fp)
{
}

/* call markfn for every chunk of memory allocated by IMC */
void imc_markmemory(void (*markfn)(void *))
{
  imc_reminfo *r;
  imc_mail *mailp;
  imc_qnode *queuep;
  imc_mailid *idp;
  imc_info *infop;
  imc_connect *cp;
  imc_event *ep;

  for (infop=imc_info_list; infop; infop=infop->next)
    markfn(infop);

  for (cp=imc_connect_list; cp; cp=cp->next)
  {
    markfn(cp->inbuf);
    markfn(cp->outbuf);
    markfn(cp);
  }

  for (r=imc_reminfo_list; r; r=r->next)
    markfn(r);

  for (mailp=imc_ml_head; mailp; mailp=mailp->next)
    markfn(mailp);

  for (queuep=imc_mq_head; queuep; queuep=queuep->next)
    markfn(queuep);

  for (idp=imc_idlist; idp; idp=idp->next)
    markfn(idp);

  for (ep=imc_event_list; ep; ep=ep->next)
    markfn(ep);

  for (ep=imc_event_free; ep; ep=ep->next)
    markfn(ep);
}

/* call markfn for every string allocated by IMC */
void imc_markstrings(void (*markfn)(char *))
{
  int i;
  imc_reminfo *r;
  imc_mail *mailp;
  imc_qnode *queuep;
  imc_mailid *idp;
  imc_info *infop;
  imc_ignore_data *ign;

  markfn(imc_name);
  markfn(imc_prefix);

  for (infop=imc_info_list; infop; infop=infop->next)
  {
    markfn(infop->name);
    markfn(infop->host);
    markfn(infop->serverpw);
    markfn(infop->clientpw);
  }

  for (r=imc_reminfo_list; r; r=r->next)
  {
    markfn(r->name);
    markfn(r->version);
    markfn(r->route);
  }

  for (ign=imc_ignore_list; ign; ign=ign->next)
    markfn(ign->name);

  for (i=0; i<IMC_MEMORY; i++)
    if (imc_memory[i].from)
      markfn(imc_memory[i].from);

  for (mailp=imc_ml_head; mailp; mailp=mailp->next)
  {
    markfn(mailp->from);
    markfn(mailp->to);
    markfn(mailp->date);
    markfn(mailp->text);
    markfn(mailp->subject);
    markfn(mailp->id);
  }
  
  for (queuep=imc_mq_head; queuep; queuep=queuep->next)
    markfn(queuep->tomud);
  
  for (idp=imc_idlist; idp; idp=idp->next)
    markfn(idp->id);
}


/* Color configuration (more tables to configure.. sowwy)
 *
 * The various #defines here are used within the IMC code to format 
 * things like rtells themselves (not the text of the message, but the
 * "a@b rtells you 'blah'" structure)
 *
 * Each C_? #define maps an IMC color code (the ?) to one of your color codes.
 *
 * The various FORMATs define colors for rbeep, etc
 *
 * this may seem ugly, but it makes things expandable - and it will probably
 * go away in the near future anyway
 *
 */

#ifdef ACK
#define RTELL_FORMAT_1 COL(W) "You rtell " COL(Y) "%s " COL(W) "'" COL(w) "%s" COL(W) "'\n\r" COL(d)
#define RTELL_FORMAT_2 COL(Y) "%s " COL(W) "rtells you '" COL(w) "%s" COL(W) "'\n\r" COL(d)
#define RBEEP_FORMAT_1 COL(R) "You rbeep " COL(Y) "%s" COL(R) ".\n\r" COL(d)
#define RBEEP_FORMAT_2 COL(Y) "\a%s " COL(R) "rbeeps you.\n\r" COL(d)
#else /* custom formats */
#define RTELL_FORMAT_1 "You rtell %s '%s" COL(d) "'.\n\r"
#define RTELL_FORMAT_2 "%s rtells you '%s" COL(d) "'.\n\r"
#define RBEEP_FORMAT_1 "You rbeep %s.\n\r"
#define RBEEP_FORMAT_2 "\a%s rbeeps you.\n\r"
#endif

#ifdef ACK
/* Ack! colors */
#define COL(x) C_##x
#define C_b "@@B"
#define C_g "@@G"
#define C_r "@@R"
#define C_y "@@b"
#define C_m "@@m"
#define C_c "@@c"
#define C_w "@@g"
#define C_D "@@d"
#define C_B "@@l"
#define C_G "@@r"
#define C_R "@@e"
#define C_Y "@@y"
#define C_M "@@p"
#define C_C "@@a"
#define C_W "@@W"
#define C_x "@@N"
#define C_d "@@N"

#elif defined(SMAUG)
/* SMAUG colors */
#define COL(x) C_##x
#define C_b "&b"
#define C_g "&g"
#define C_r "&r"
#define C_y "&O"
#define C_m "&p"
#define C_c "&c"
#define C_w "&w"
#define C_D "&z"
#define C_B "&B"
#define C_G "&G"
#define C_R "&R"
#define C_Y "&Y"
#define C_M "&P"
#define C_C "&C"
#define C_W "&W"
#define C_x "&w"
#define C_d "&w"

#elif defined(CIRCLE_COLOR)
/* Circle colors - Easy Colour 2.2 */
#define COL(x) C_##x
#define C_b "&b"
#define C_g "&g"
#define C_r "&r"
#define C_y "&y"
#define C_m "&m"
#define C_c "&c"
#define C_w "&w"
#define C_D "&K"
#define C_B "&B"
#define C_G "&G"
#define C_R "&R"
#define C_Y "&Y"
#define C_M "&M"
#define C_C "&C"
#define C_W "&W"
#define C_x "&n"
#define C_d "&n"

#elif defined(IMC_COLOR)
/* custom color configuration */
#define COL(x) C_##x
#define C_b "{b"
#define C_g "{g"
#define C_r "{r"
#define C_y "{y"
#define C_m "{m"
#define C_c "{c"
#define C_w "{w"
#define C_D "{D"
#define C_B "{B"
#define C_G "{G"
#define C_R "{R"
#define C_Y "{Y"
#define C_M "{M"
#define C_C "{C"
#define C_W "{W"
#define C_x "{x"
#define C_d "{d"

#else
/* no color */
#define COL(x) ""

#endif

/*  maps IMC standard -> mud local color codes
 *  let's be unique, noone uses ~ :>
 */

/* Most of this table will be generated from the #defines above;
 * you will just need to add the exceptions.
 */

struct {
  char *imc;        /* IMC code to convert */
  char *mud;        /* Equivalent mud code */
}
trans_table[]=
{
  /* common color definitions, derived from the #defines above */

  { "~!", COL(x) },  /* reset */
  { "~d", COL(d) },  /* default */

  { "~b", COL(b) },  /* dark blue */
  { "~g", COL(g) },  /* dark green */
  { "~r", COL(r) },  /* dark red */
  { "~y", COL(y) },  /* dark yellow / brown */
  { "~m", COL(m) },  /* dark magenta / purple */
  { "~c", COL(c) },  /* cyan */
  { "~w", COL(w) },  /* white */

  { "~D", COL(D) },  /* dark grey / "z" ? */
  { "~B", COL(B) },  /* light blue */
  { "~G", COL(G) },  /* light green */
  { "~R", COL(R) },  /* light red */
  { "~Y", COL(Y) },  /* light yellow */
  { "~M", COL(M) },  /* light magenta / purple */
  { "~C", COL(C) },  /* light cyan */
  { "~W", COL(W) },  /* bright white */

#ifdef SMAUG
  /* built-in SMAUG color */

  { "&",  "&&" },
  { "^",  "^^" },

  { "~~", "&-" },  /* note ordering here.. */
  { "~~", "~"  },

  /* background colors - no support for these in IMC */

  { "",   "^b" },
  { "",   "^x" },
  { "",   "^g" },
  { "",   "^r" },
  { "",   "^O" },
  { "",   "^p" },
  { "",   "^c" },
  { "",   "^w" },
  { "",   "^z" },
  { "",   "^B" },
  { "",   "^G" },
  { "",   "^R" },
  { "",   "^Y" },
  { "",   "^P" },
  { "",   "^C" },
  { "",   "^W" },

#elif defined(ACK)
  /* built-in Ack! color */

  { "~~", "~"   },  /* escape raw tildes */

  { "@@", "@ @" },  /* escape our color character */
  
  { "~!", "@@x" },  /* catch commonly used (but incorrect) codes */
  { "~!", "@@f" },  /*  ditto  */
  { "~!", "@@!" },  /*  ditto  */
  { "~!", "@@." },  /*  ditto  */
  { "~!", "@@n" },  /*  ditto  */
  { "~!", "@@i" },  /*  ditto  */
  { "~D", "@@k" },  /* black -> grey */

#elif defined(IMC_COLOR)
  /* generic color; change as needed */

  { "~~", "~"  },  /* escape raw tildes */
  { "{",  "{{" },  /* escape our color character */

#else
  /* no color support */

  { "~~", "~"  },  /* escape raw tildes */

#endif
};

#define numtrans (sizeof(trans_table)/sizeof(trans_table[0]))

/* convert from imc color -> mud color */

const char *color_itom(const char *s)
{
  char *buf=imc_getsbuf(IMC_DATA_LENGTH);
  const char *current;
  char *out;
  int i, l;
  int count;

  for (current=s, out=buf, count=0; *current && count<IMC_DATA_LENGTH; )
  {
    if (*current=='~' || *current==IMC_COLORCHAR || *current==IMC_COLORCHAR2)
    {
      for (i=0; i<numtrans; i++)
      {
	l=strlen(trans_table[i].imc);
	if (l && !strncmp(current, trans_table[i].imc, l))
	  break;
      }
      
      if (i!=numtrans)       /* match */
  
	  {
	int len;

	len=strlen(trans_table[i].mud);
	count+=len;
	if (count >= IMC_DATA_LENGTH)
	  break;
	memcpy(out, trans_table[i].mud, len);
	out+=len;
	current+=l;
	continue;
      }
    }

    *out++=*current++;
    count++;
  }
  
  *out=0;
  imc_shrinksbuf(buf);
  return buf;
}

/* convert from mud color -> imc color */

const char *color_mtoi(const char *s)
{
  char *buf=imc_getsbuf(IMC_DATA_LENGTH);
  const char *current;
  char *out;
  int i, l;
  int count;

  for (current=s, out=buf, count=0; *current && count<IMC_DATA_LENGTH; )
  {
    if (*current=='~' || *current==IMC_COLORCHAR || *current==IMC_COLORCHAR2)
    {
      for (i=0; i<numtrans; i++)
      {
	l=strlen(trans_table[i].mud);
	if (l && !strncmp(current, trans_table[i].mud, l))
	  break;
      }
      
      if (i!=numtrans)      /* match */
      {
	int len;

	len=strlen(trans_table[i].imc);
	count+=len;
	if (count>=IMC_DATA_LENGTH)
	  break;
	memcpy(out, trans_table[i].imc, len);
	out+=len;
	current+=l;
	continue;
      }
    }

    *out++=*current++;
    count++;
  }
  
  *out=0;
  imc_shrinksbuf(buf);
  return buf;
}

/* #ifdefs here are ugly, but will eventually disappear */

static struct {
  int number;
  const char *name;
  const char *chatstr;
  const char *emotestr;
  long flag;
  int minlevel;
  char *to;
} imc_channels[]=
{
#ifdef ACK
  /* Ack!-specific formatting - ick */
  
  {
    2,
    "RInfo",
    "@@W[@@aRINFO@@W] @@a%s@@W:'@@g%s@@W'@@N\n\r",
    "@@W[@@aRINFO@@W] @@a%s@@W %s@@N\n\r",
    IMC_RINFO,
    L_SUP,
    "*",
  },

  /* ICE status channel */
  {
    15,
    "ICE",
    "@@W[@@aICE@@W] @@a%s@@W %s@@N\n\r",
    "@@W[@@aICE@@W] @@a%s@@W %s@@N\n\r",
    IMC_ICE,
    L_SUP,
    "*"
  },

#else /* non-Ack! */

  {
    2,
    "RInfo",
    /* why didn't I think of this? :) */
    "[Rinfo] %s announces '%s" COL(d) "'\n\r",
    "[RInfo] %s %s\n\r" COL(d),
    IMC_RINFO,
    LEVEL_IMMORTAL,
    "*",
  },

  {
    15,
    "ICE",
    "[ICE] %s %s" COL(d) "\n\r",
    "[ICE] %s %s" COL(d) "\n\r",
    IMC_ICE,
    LEVEL_IMMORTAL,
    "*"
  }
#endif /* #ifdef ACK */

  /*
   *  An example of how to set up a channel that is only distributed between
   *  the three muds 'mud1', 'mud2', and 'mud3'.

  {
    76,                            -- channel number to use, change this!
    "LocalChannel",                -- the channel name
    "%s localchannels '%s'\n\r",   -- format for a normal message
    "[LocalChannel] %s %s\n\r",    -- format for an emote
    IMC_LOCALCHANNEL,              -- flag in imc_deaf, etc
    LEVEL_IMMORTAL,                -- min. level for channel
    "mud1 mud2 mud3"               -- muds to distribute to (include your own)
  }

   * (see the interack channel in #ifdef ACK, above, for an example)
   *
   */
};

#define numchannels (sizeof(imc_channels)/sizeof(imc_channels[0]))

static int getlevel(int l)
{
  if (l<LEVEL_HERO)
    return 1;
  else
    return -1;
}

#ifdef CIRCLE
void bug(char *format, int tmp)
{
  sprintf(buf, format, tmp);
  log(buf);
  
  return;
}
#endif

#if defined(CIRCLE) && defined(NEED_STR_PREFIX)
bool str_prefix(const char *astr, const char *bstr)
{
  if (!astr) {
    log("Strn_cmp: null astr.");
    return TRUE;
  }
  if (!bstr) {
    log("Strn_cmp: null astr.");
    return TRUE;
  }
  for(; *astr; astr++, bstr++) {
    if(LOWER(*astr) != LOWER(*bstr)) return TRUE;
  }
  return FALSE;
}
#endif

const imc_char_data *imc_getdata(CHAR_DATA *ch)
{
  static imc_char_data d;

  if (!ch) /* fake system character */
  {
    d.wizi=0;
    d.level=-1;
    d.invis=0;
    strcpy(d.name, "*");
    return &d;
  }

  if (IS_RINVIS(ch))
    d.invis=1;
  else
    d.invis=0;

#ifdef ROM
  d.wizi=(ch->invis_level || ch->incog_level ||
	  IS_AFFECTED(ch, AFF_HIDE) || IS_AFFECTED(ch, AFF_SNEAK) ||
#ifdef ROT
          IS_SHIELDED(ch, SHD_INVISIBLE)) ? -1 : 0;
#else
	  IS_AFFECTED(ch, AFF_INVISIBLE)) ? -1 : 0;
#endif

#elif defined(SMAUG14) || defined(SMAUG18)
  d.wizi=(xIS_SET(ch->act, PLR_WIZINVIS) ||
	  IS_AFFECTED(ch, AFF_HIDE) || IS_AFFECTED(ch, AFF_SNEAK) ||
	  IS_AFFECTED(ch, AFF_INVISIBLE)) ? -1 : 0;
#elif defined(ACK)
  d.wizi=(IS_SET(ch->act, PLR_WIZINVIS) ||
	  IS_AFFECTED(ch, AFF_HIDE) || IS_AFFECTED(ch, AFF_SNEAK) ||
	  IS_AFFECTED(ch, AFF_INVISIBLE)) ? -1 : 0;
#elif defined(CIRCLE)
  d.wizi=GET_INVIS_LEV(ch);
#else
  d.wizi=(IS_SET(ch->act, PLR_WIZINVIS) ||
	  IS_AFFECTED(ch, AFF_HIDE) || IS_AFFECTED(ch, AFF_SNEAK) ||
	  IS_AFFECTED(ch, AFF_INVISIBLE)) ? -1 : 0;
#endif

  d.level=getlevel(get_trust(ch));
  strcpy(d.name, GET_NAME(ch));

  return &d;
}

int imc_visible(const imc_char_data *viewer, const imc_char_data *viewed)
{
  /* this is now a lot simpler */

  return
    !viewed->invis &&
    (viewer->level < 0 ||
     viewed->wizi >= 0);
}

static const char *getname(CHAR_DATA *ch, const imc_char_data *vict)
{
  const char *mud, *name;
  const imc_char_data *chdata;

  mud=imc_mudof(vict->name);
  name=imc_nameof(vict->name);
  chdata=imc_getdata(ch);

#ifdef CIRCLE
  if (!str_cmp((char *)mud, imc_name))
#else
  if (!str_cmp(mud, imc_name))
#endif
#ifdef ACK
    return imc_visible(chdata,vict) ? imc_nameof(name)
      : (chdata->wizi ? "A Mystical Being" : "someone");
#else
    return imc_visible(imc_getdata(ch),vict) ? imc_nameof(name) : "someone";
#endif
  else
  {
    if (imc_visible(imc_getdata(ch),vict))
      return vict->name;
    else
    {
      char *buf=imc_getsbuf(IMC_NAME_LENGTH);
#ifdef ACK
      sprintf(buf, "%s@%s",
	      (imc_getdata(ch))->wizi ? "A Mystical Being" : "someone", mud);
#else
      sprintf(buf, "someone@%s", mud);
#endif
      imc_shrinksbuf(buf);
      return buf;
    }
  }
}

static void send_rchannel(CHAR_DATA *ch, char *argument, int number)
{
  char arg[MAX_STRING_LENGTH];
  char *arg2;
  int chan;

  CHECKIMC(ch);

#ifdef CIRCLE
  skip_spaces(&argument);
#endif

  for (chan=0; chan<numchannels; chan++)
    if (imc_channels[chan].number==number)
      break;

  if (chan==numchannels)
  {
    imc_to_char("That channel doesn't seem to exist!\n\r", ch);
    bug("IMC channel %d doesn't exist?!", number);
    return; /* oops */
  }

  if (!CAN(ch, imc_channels[chan].flag, imc_channels[chan].minlevel))
  {
    sprintf(arg, "You are not authorised to use %s.\n\r",
	    imc_channels[chan].name);
    imc_to_char(arg, ch);
    return;
  }

  if (!argument[0])
  {
    if (IS_SET(IMC_DEAF_FLAGS(ch), imc_channels[chan].flag))
    {
      sprintf(arg, "%s channel is now ON.\n\r", imc_channels[chan].name);
      imc_to_char(arg, ch);
      REMOVE_BIT(IMC_DEAF_FLAGS(ch), imc_channels[chan].flag);
      return;
    }
    
    sprintf(arg, "%s channel is now OFF.\n\r", imc_channels[chan].name);
    imc_to_char(arg, ch);

    SET_BIT(IMC_DEAF_FLAGS(ch), imc_channels[chan].flag);
    return;
  }

  if (IS_NOCHAN(ch))
  {
    imc_to_char("The gods have revoked your channel priviliges.\n\r", ch);
    return;
  }

  if (IS_QUIET(ch))
  {
    imc_to_char("You must turn off quiet mode first.\n\r", ch);
    return;
  }

  if (IS_SILENT(ch))
  {
    imc_to_char ("You can't seem to break the silence.\n\r",ch);
    return;
  }

  if (IS_RINVIS(ch))
  {
    imc_to_char ("You cannot use IMC channels while invisible to IMC.\n\r", ch);
    return;
  }
  
  REMOVE_BIT(IMC_DEAF_FLAGS(ch), imc_channels[chan].flag);

  /* NargoRoth's bugfix here */
  arg2=one_argument(argument, arg);

#ifdef CIRCLE
  skip_spaces(&arg2);
#endif

  if (!str_cmp(arg, ",") || !str_cmp(arg, "emote"))
    imc_send_emote(imc_getdata(ch), number, color_mtoi(arg2), imc_channels[chan].to);
  else
    imc_send_chat(imc_getdata(ch), number, color_mtoi(argument),
                  imc_channels[chan].to);
}

DEFINE_DO_FUN(do_rchat)
{
  send_rchannel(ch, argument, 0);
}

DEFINE_DO_FUN(do_rimm)
{
  send_rchannel(ch, argument, 1);
}

DEFINE_DO_FUN(do_rinfo)
{
  send_rchannel(ch, argument, 2);
}

DEFINE_DO_FUN(do_rcode)
{
  send_rchannel(ch, argument, 3);
}

#ifdef ACK
DEFINE_DO_FUN(do_interack)
{
  send_rchannel(ch, argument, 243);
}
#endif

DEFINE_DO_FUN(do_rtell)
{
  char buf[MAX_STRING_LENGTH], buf1[MAX_STRING_LENGTH];

  CHECKIMC(ch);

  argument=one_argument(argument, buf);

#ifdef CIRCLE
  skip_spaces(&argument);
#endif

  if (!buf[0] || !strchr(buf, '@') || !argument[0])
  {
    imc_to_char("rtell who@where what?\n\r", ch);
    return;
  }

  if (!CAN(ch, IMC_RTELL, IMC_LEVEL_RTELL))
  {
    imc_to_char("You are not authorised to use rtell.\n\r", ch);
    return;
  }

  if (IS_NOTELL(ch))
  {
    imc_to_char("You cannot tell!\n\r", ch);
    return;
  }

  if (IS_RINVIS(ch))
  {
    imc_to_char ("You cannot rtell while invisible to IMC.\n\r", ch);
    return;
  }

  if (IS_SET(IMC_DEAF_FLAGS(ch), IMC_RTELL))
  {
    imc_to_char("Enable incoming rtells first ('rchannel +rtell').\n\r", ch);
    return;
  }

#ifdef ACK
  if (IS_SET(ch->pcdata->pflags, PFLAG_AFK))
  {
    imc_to_char("Turn off AFK mode first.\n\r", ch);
    return;
  }
#endif

#ifdef ROM
  if (IS_SET(ch->comm, COMM_AFK))
  {
    imc_to_char("Turn off AFK mode first.\n\r", ch);
    return;
  }

  if (IS_SET(ch->comm, COMM_DEAF))
  {
    imc_to_char("You must turn off deaf mode first.\n\r", ch);
    return;
  }
#endif

  if (IS_QUIET(ch))
  {
    imc_to_char("You must turn off quiet mode first.\n\r", ch);
    return;
  }
  
  CHECKMUDOF(ch, buf);
  imc_send_tell(imc_getdata(ch), buf, color_mtoi(argument), 0);
  
  sprintf(buf1,
          RTELL_FORMAT_1,
	  buf, argument);
  imc_to_char(buf1, ch);
}

DEFINE_DO_FUN(do_rreply)
{
  char buf1[MAX_STRING_LENGTH];

  CHECKIMC(ch);

  if (!IMC_RREPLY(ch))
  {
    imc_to_char("rreply to who?\n\r", ch);
    return;
  }

#ifdef CIRCLE
  skip_spaces(&argument);
#endif

  if (!argument[0])
  {
    imc_to_char("rreply what?\n\r", ch);
    return;
  }

  /* just check for deny, since you can rtell someone < IMC_LEVEL_RTELL */
  if (IS_SET(IMC_DENY_FLAGS(ch), IMC_RTELL))
  {
    imc_to_char("You are not authorised to use rreply.\n\r", ch);
    return;
  }

  if (IS_NOTELL(ch))
  {
    imc_to_char("You cannot tell!\n\r", ch);
    return;
  }

  if (IS_RINVIS(ch))
  {
    imc_to_char ("You cannot rreply while invisible to IMC.\n\r", ch);
    return;
  }

  if (IS_SET(IMC_DEAF_FLAGS(ch), IMC_RTELL))
  {
    imc_to_char("Enable incoming rtells first ('rchannel +rtell').\n\r", ch);
    return;
  }

#ifdef ACK
  if (IS_SET(ch->pcdata->pflags, PFLAG_AFK))
  {
    imc_to_char("Turn off AFK mode first.\n\r", ch);
    return;
  }
#endif

#ifdef ROM
  if (IS_SET(ch->comm, COMM_AFK))
  {
    imc_to_char("Turn off AFK mode first.\n\r", ch);
    return;
  }

  if (IS_SET(ch->comm, COMM_DEAF))
  {
    imc_to_char("You must turn off deaf mode first.\n\r",ch);
    return;
  }
#endif

  if (IS_QUIET(ch))
  {
    imc_to_char("You must turn off quiet mode first.\n\r", ch);
    return;
  }
  
  CHECKMUDOF(ch, IMC_RREPLY(ch));
  imc_send_tell(imc_getdata(ch), IMC_RREPLY(ch), color_mtoi(argument), 1);

  sprintf(buf1,
          RTELL_FORMAT_1,
	  IMC_RREPLY_NAME(ch), argument);
  imc_to_char(buf1, ch);
} 

DEFINE_DO_FUN(do_rwho)
{
  char arg[MAX_STRING_LENGTH];

  CHECKIMC(ch);

  argument=one_argument(argument, arg);

#ifdef CIRCLE
  skip_spaces(&argument);
#endif

  if (!arg[0])
  {
    imc_to_char("rwho where?\n\r", ch);
    return;
  }
  
  CHECKMUD(ch, arg);
  imc_send_who(imc_getdata(ch), arg, argument[0] ? argument : "who");
}

DEFINE_DO_FUN(do_rwhois)
{
  char arg[MAX_STRING_LENGTH];

  CHECKIMC(ch);

  argument=one_argument(argument, arg);

#ifdef CIRCLE
  skip_spaces(&argument);
#endif

  if (!arg[0])
  {
    imc_to_char("rwho whom?\n\r", ch);
    return;
  }

  imc_send_whois(imc_getdata(ch), arg);
}

DEFINE_DO_FUN(do_rfinger)
{
  char arg[MAX_STRING_LENGTH];
  char name[MAX_STRING_LENGTH];

  CHECKIMC(ch);

  argument=one_argument(argument, arg);

#ifdef CIRCLE
  skip_spaces(&argument);
#endif

  if (!arg[0] || !strchr(arg, '@'))
  {
    imc_to_char("rfinger who@where?\n\r", ch);
    return;
  }

  CHECKMUD(ch, imc_mudof(arg));
  sprintf(name, "finger %s", imc_nameof(arg));
  imc_send_who(imc_getdata(ch), imc_mudof(arg), name);
}

DEFINE_DO_FUN(do_rquery)
{
  char arg[MAX_STRING_LENGTH];

  CHECKIMC(ch);

  argument=one_argument(argument, arg);

#ifdef CIRCLE
  skip_spaces(&argument);
#endif

  if (!arg[0])
  {
    imc_to_char("rquery where?\n\r", ch);
    return;
  }

  CHECKMUD(ch, arg);
  imc_send_who(imc_getdata(ch), arg, argument[0] ? argument : "help");
}

DEFINE_DO_FUN(do_rbeep)
{
  char buf[MAX_STRING_LENGTH];

  CHECKIMC(ch);

#ifdef CIRCLE
  skip_spaces(&argument);
#endif

  if (!argument[0] || !strchr(argument, '@'))
  {
    imc_to_char("rbeep who@where?\n\r", ch);
    return;
  }

  if (!CAN(ch, IMC_RBEEP, IMC_LEVEL_RBEEP))
  {
    imc_to_char("You are not authorised to rbeep.\n\r", ch);
    return;
  }

  if (IS_NOTELL(ch))
  {
    imc_to_char("You cannot beep!\n\r", ch);
    return;
  }

  if (IS_RINVIS(ch))
  {
    imc_to_char ("You cannot rbeep while invisible to IMC.\n\r", ch);
    return;
  }

  if (IS_QUIET(ch))
  {
    imc_to_char("You must turn off quiet mode first.\n\r", ch);
    return;
  }
  
#ifdef ROM
  if (IS_SET(ch->comm, COMM_DEAF))
  {
    imc_to_char("You must turn off deaf mode first.\n\r", ch);
    return;
  }
#endif

  CHECKMUDOF(ch, argument);
  imc_send_beep(imc_getdata(ch), argument);
  sprintf(buf, RBEEP_FORMAT_1, argument);
  imc_to_char(buf, ch);
   
#ifdef CIRCLE
  WAIT_STATE(ch, PASSES_PER_SEC * 5);
#else
  WAIT_STATE(ch, PULSE_PER_SECOND * 5 );
#endif
}

DEFINE_DO_FUN(do_imclist)
{

#ifdef CIRCLE
  skip_spaces(&argument);
#endif

#ifdef SMAUG
  set_pager_color(AT_PLAIN,ch);
#endif
  if (!argument[0])
    imc_to_pager(imc_list(3), ch);
  else if (!str_prefix(argument, "all"))
    imc_to_pager(imc_list(6), ch);
  else if (!str_prefix(argument, "routes"))
  {
	char buf[IMC_DATA_LENGTH];
	imc_reminfo *i;

	*buf=0;
  	for (i=imc_reminfo_list; i; i=i->next)
        {
                if(i->hide)
			strcat(buf,"[H]");
		sprintf(buf + strlen(buf),"%s:",i->name);
            	if(i->path)
			sprintf(buf + strlen(buf),"%s\n\r",i->path);
		else
			sprintf(buf + strlen(buf),"\n\r");
	}
	imc_to_pager(buf,ch);
  }
		
  else if (!str_prefix(argument, "direct"))
  {
    if (get_trust(ch)>=MAX_LEVEL-1)
      imc_to_pager(imc_list(2), ch);
    else if (IS_IMMORTAL(ch))
      imc_to_pager(imc_list(1), ch);
    else
      imc_to_pager(imc_list(0), ch);
  }
  else if (!str_prefix(argument, "config"))
  {
    if (IS_IMMORTAL(ch))
      imc_to_pager(imc_list(5), ch);
    else
      imc_to_pager(imc_list(4), ch);
  }
  else
    imc_to_pager("Unknown option.", ch);

  imc_to_pager("\n\r", ch);
}

DEFINE_DO_FUN(do_rsockets)
{
  CHECKIMC(ch);

  imc_to_pager(imc_sockets(), ch);
}

DEFINE_DO_FUN(do_imcpoll) {
	imc_add_event(5, ev_imc_optimize, NULL, 1);
	imc_to_char("Polling for hubs in 5 seconds.\n\r", ch);
}

DEFINE_DO_FUN(do_imc)
{
  int r;

#ifdef CIRCLE
  skip_spaces(&argument);
#endif

  r=imc_command(argument);

  if (r>0)
  {
    imc_to_char("Ok.\n\r", ch);
    return;
  }
  else if (r==0)
  {
    imc_to_char("Syntax:  imc add <mudname>\n\r"
		 "         imc delete <mudname>\n\r"
		 "         imc set <mudname> all <host> <port> <clientpw> <serverpw>\n\r"
		 "                 <rcvstamp> <noforward> <flags>\n\r"
		 "         imc set <mudname> host|port|clientpw|serverpw|rcvstamp|\n\r"
		 "                           noforward|flags <value>\n\r"
		 "         imc rename <oldmudname> <newmudname>\n\r"
		 "         imc localname <mudname>\n\r"
		 "         imc localport <portnum>\n\r"
		 "         imc log (toggles all log messages)\n\r"
		 "         imc hubswitch (toggles hubswitching - for firewalls)\n\r"
		 "         imc info name <LocalName>\n\r"
                 "         imc info host <InfoHost> (server ip addy or dns name)\n\r"
		 "         imc info imail <admins imc mail address>\n\r"
		 "         imc info email <admins internet mail address>\n\r"
                 "         imc info www <URL for webpage>\n\r"
                 "         imc info details <text>\n\r"
		 "         imc reload\n\r",
		 ch);
    return;
  }

#ifdef CIRCLE
  imc_to_char((char *)imc_error(), ch);
#else
  imc_to_char(imc_error(), ch);
#endif
  imc_to_char("\n\r", ch);
}

DEFINE_DO_FUN(do_rignore)
{
#ifdef CIRCLE
  skip_spaces(&argument);
#endif

#ifdef CIRCLE
  imc_to_char((char *)imc_ignore(argument), ch);
#else
  imc_to_char(imc_ignore(argument), ch);
#endif
  imc_to_char("\n\r", ch);
}

DEFINE_DO_FUN(do_rconnect)
{
  CHECKIMC(ch);

#ifdef CIRCLE
  skip_spaces(&argument);
#endif

  if (!argument[0])
  {
    imc_to_char("rconnect to where?\n\r", ch);
    return;
  }

  if (imc_connect_to(argument))
  {
    imc_to_char("Ok.\n\r", ch);
    return;
  }

#ifdef CIRCLE
  imc_to_char((char *)imc_error(), ch);
#else
  imc_to_char(imc_error(), ch);
#endif
  imc_to_char("\n\r", ch);
}

DEFINE_DO_FUN(do_rdisconnect)
{
  CHECKIMC(ch);

#ifdef CIRCLE
  skip_spaces(&argument);
#endif

  if (!argument[0])
  {
    imc_to_char("rdisconnect where?\n\r", ch);
    return;
  }

  if (imc_disconnect(argument))
  {
    imc_to_char("Ok.\n\r", ch);
    return;
  }

#ifdef CIRCLE
  imc_to_char((char *)imc_error(), ch);
#else
  imc_to_char(imc_error(), ch);
#endif
  imc_to_char("\n\r", ch);
}

DEFINE_DO_FUN(do_mailqueue)
{
  CHECKIMC(ch);

  imc_to_pager(imc_mail_showqueue(), ch);
}

DEFINE_DO_FUN(do_istats)
{
  CHECKIMC(ch);

#ifdef CIRCLE
  imc_to_char((char *)imc_getstats(), ch);
#else
  imc_to_char(imc_getstats(), ch);
#endif
}

DEFINE_DO_FUN(do_rchannels)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];
  bool toggle;
  int i;
  const char *name;
  long flag;
  
  if (IS_NPC(ch))
  {
    imc_to_char("NPCs cannot use IMC.\n\r", ch);
    return;
  }

  one_argument(argument, arg);

#ifdef CIRCLE
  skip_spaces(&argument);
#endif
  
  if (!arg[0])
  {
    /* list channel status */

    bool found=FALSE;
    
    strcpy(buf,
	   "channel      status\n\r"
	   "-------------------\n\r");

    for (i=0; i<numchannels; i++)
    {
      if (IS_SET(IMC_DENY_FLAGS(ch), imc_channels[i].flag))
      {
	sprintf(buf+strlen(buf),
		"%-10s : you are denied from this channel.\n\r",
		imc_channels[i].name);
	found=TRUE;
      }
      else if (CAN(ch, imc_channels[i].flag, imc_channels[i].minlevel))
      {
	sprintf(buf+strlen(buf),
		"%-10s : %s\n\r",
		imc_channels[i].name,
		(IS_SET(IMC_DEAF_FLAGS(ch), imc_channels[i].flag) ?
		 "OFF" : "ON"));
	found=TRUE;
      }
    }

    if (IS_SET(IMC_DENY_FLAGS(ch), IMC_RTELL))
    {
      strcat(buf, "RTells     : you cannot rtell.\n\r");
      found=TRUE;
    }
    else if (CAN(ch, IMC_RTELL, IMC_LEVEL_RTELL))
    {
      sprintf(buf+strlen(buf), "%-10s : %s\n\r", "RTell",
	      IS_SET(IMC_DEAF_FLAGS(ch), IMC_RTELL) ? "OFF" : "ON");
      found=TRUE;
    }

    if (IS_SET(IMC_DENY_FLAGS(ch), IMC_RBEEP))
    {
      strcat(buf, "RBeeps     : you cannot rbeep.\n\r");
      found=TRUE;
    }
    else if (CAN(ch, IMC_RBEEP, IMC_LEVEL_RBEEP))
    {
      sprintf(buf+strlen(buf), "%-10s : %s\n\r", "RBeep",
	      IS_SET(IMC_DEAF_FLAGS(ch), IMC_RBEEP) ? "OFF" : "ON");
      found=TRUE;
    }

    if (IS_SET(IMC_DEAF_FLAGS(ch), IMC_RINVIS))
      sprintf(buf+strlen(buf), "You are not visible to others over IMC.\n\r");

    if (!found)
      imc_to_char("You have access to no IMC channels.\n\r", ch);
    else
      imc_to_char(buf, ch);
      
    return;
  }

  /* turn some things on or off */

  while((argument=one_argument(argument, arg)), arg[0])
  {
    if (arg[0]=='-')
      toggle=FALSE;
    else if (arg[0]=='+')
      toggle=TRUE;
    else
    {
      imc_to_char("Syntax: rchannels          displays current IMC channels\n\r"
		   "        rchannels +chan    turn on a channel\n\r"
		   "        rchannels -chan    turn off a channel\n\r"
                   "chan may be an IMC channel name, 'rbeep', 'rtell', 'rinvis', or 'all'.\n\r"
		   "Multiple settings may be given in one command.\n\r", ch);
      return;
    }

    if (!str_cmp(arg+1, "all"))
    {
      if (toggle)
      {
	imc_to_char("ALL available IMC channels are now on.\n\r", ch);
	IMC_DEAF_FLAGS(ch)=0;
      }
      else
      {
	imc_to_char("ALL available IMC channels are now off.\n\r", ch);
	IMC_DEAF_FLAGS(ch)=-1L;
      }

      continue;
    }
    else if (!str_cmp(arg+1, "rtell") && CAN(ch, IMC_RTELL, IMC_LEVEL_RTELL))
    {
      name="RTell";
      flag=IMC_RTELL;
    }
    else if (!str_cmp(arg+1, "rbeep") && CAN(ch, IMC_RBEEP, IMC_LEVEL_RBEEP))
    {
      name="RBeep";
      flag=IMC_RBEEP;
    }
    else if (!str_cmp(arg+1, "rinvis"))
    {
      name="RInvis";
      flag=IMC_RINVIS;
      toggle=!toggle;
    }
    else
    {
      for (i=0; i<numchannels; i++)
      {
	if (CAN(ch, imc_channels[i].flag, imc_channels[i].minlevel) &&
#ifdef CIRCLE
	    !str_cmp((char *)imc_channels[i].name, arg+1))
#else
	    !str_cmp(imc_channels[i].name, arg+1))
#endif
	  break;
      }

      if (i==numchannels)
      {
	sprintf(buf, "You don't have access to an IMC channel called \"%s\".\n\r", arg);
	imc_to_char(buf, ch);
        continue;
      }

      name=imc_channels[i].name;
      flag=imc_channels[i].flag;
    }
  
    if (toggle && !IS_SET(IMC_DEAF_FLAGS(ch), flag))
    {
      if (flag==IMC_RINVIS)
	strcpy(buf, "You are already visible on IMC.\n\r");
      else
	sprintf(buf, "%s is already on.\n\r", name);
      imc_to_char(buf, ch);
      continue;
    }

    if (!toggle && IS_SET(IMC_DEAF_FLAGS(ch), flag))
    {
      if (flag==IMC_RINVIS)
	strcpy(buf, "You are already invisible on IMC.\n\r");
      else
	sprintf(buf, "%s is already off.\n\r", name);
      imc_to_char(buf, ch);
      continue;
    }

    if (toggle)
    {
      if (flag==IMC_RINVIS)
	strcpy(buf, "You are no longer invisible to IMC.\n\r");
      else
	sprintf(buf, "%s is now ON.\n\r", name);
      REMOVE_BIT(IMC_DEAF_FLAGS(ch), flag);
    }
    else
    {
      if (flag==IMC_RINVIS)
        strcpy(buf, "You are now invisible to rwho, rtell and rbeep on IMC.\n\r");
      else
	sprintf(buf, "%s is now OFF.\n\r", name);
      SET_BIT(IMC_DEAF_FLAGS(ch), flag);
    }

    imc_to_char(buf, ch);
  }
}

DEFINE_DO_FUN(do_rchanset)
{
  char arg[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int flag, i;
  const char *name;
  int fn;
  bool found=FALSE;
  char buf[MAX_STRING_LENGTH];

  argument=one_argument(argument, arg);

#ifdef CIRCLE
  skip_spaces(&argument);
#endif

  if (!arg[0])
  {
    imc_to_char("Syntax: rchanset <char>            - check flag settings\n\r"
		 "        rchanset <char> +<channel> - set allow flag\n\r"
		 "        rchanset <char> -<channel> - set deny flag\n\r"
		 "        rchanset <char> =<channel> - reset allow/deny\n\r",
		 ch);
    return;
  }

  if (!(victim=get_char_world(ch, arg)))
  {
    imc_to_char("They're not here.\n\r", ch);
    return;
  }

  if (IS_NPC(victim))
  {
    imc_to_char("Can't set rchannel status on NPCs.\n\r", ch);
    return;
  }

  if (get_trust(victim) >= get_trust(ch))
  {
    imc_to_char("You failed.\n\r", ch);
    return;
  }

  if (!argument[0])
  {
    sprintf(buf,
	    "%s's IMC channel flags:\n\r"
	    "------------------------------\n\r",
	    GET_NAME(victim));

    for (i=0; i<numchannels; i++)
    {
      if (IS_SET(IMC_ALLOW_FLAGS(victim), imc_channels[i].flag))
      {
	sprintf(buf+strlen(buf), "%-10s : allow flag set.\n\r",
		imc_channels[i].name);
	found=TRUE;
      }

      if (IS_SET(IMC_DENY_FLAGS(victim), imc_channels[i].flag))
      {
	sprintf(buf+strlen(buf), "%-10s : deny flag set.\n\r",
		imc_channels[i].name);
	found=TRUE;
      }
    }

    if (IS_SET(IMC_ALLOW_FLAGS(victim), IMC_RTELL))
    {
      sprintf(buf+strlen(buf), "%-10s : allow flag set.\n\r",
	      "RTell");
      found=TRUE;
    }

    if (IS_SET(IMC_DENY_FLAGS(victim), IMC_RTELL))
    {
      sprintf(buf+strlen(buf), "%-10s : deny flag set.\n\r",
	      "RTell");
      found=TRUE;
    }

    if (IS_SET(IMC_ALLOW_FLAGS(victim), IMC_RBEEP))
    {
      sprintf(buf+strlen(buf), "%-10s : allow flag set.\n\r",
	      "RBeep");
      found=TRUE;
    }

    if (IS_SET(IMC_DENY_FLAGS(victim), IMC_RBEEP))
    {
      sprintf(buf+strlen(buf), "%-10s : deny flag set.\n\r",
	      "RBeep");
      found=TRUE;
    }

    if (!found)
      sprintf(buf, "%s has no IMC flags set.\n\r", GET_NAME(victim));

    imc_to_char(buf, ch);
    return;
  }
  
  if (argument[0]=='-')
    fn=0;
  else if (argument[0]=='+')
    fn=1;
  else if (argument[0]=='=')
    fn=2;
  else
  {
    imc_to_char("Channel name must be preceeded by +, -, or =.\n\r", ch);
    return;
  }

  argument++;

  if (!str_cmp(argument, "rtell"))
  {
    flag=IMC_RTELL;
    name="RTell";
  }
  else if (!str_cmp(argument, "rbeep"))
  {
    flag=IMC_RBEEP;
    name="RBeep";
  }
  else
  {
    for (i=0; i<numchannels; i++)
#ifdef CIRCLE
      if (!str_cmp((char *)imc_channels[i].name, argument))
#else
      if (!str_cmp(imc_channels[i].name, argument))
#endif
	break;

    if (i==numchannels)
    {
      imc_to_char("No such channel.\n\r", ch);
      return;
    }

    flag=imc_channels[i].flag;
    name=imc_channels[i].name;
  }

  switch(fn)
  {
  case 0: /* set deny flag */
    if (IS_SET(IMC_DENY_FLAGS(victim), flag))
    {
      imc_to_char("Deny flag already set.\n\r", ch);
      return;
    }

    SET_BIT(IMC_DENY_FLAGS(victim), flag);
    REMOVE_BIT(IMC_ALLOW_FLAGS(victim), flag);
    sprintf(buf, "The gods have revoked your %s priviliges.\n\r", name);
    imc_to_char(buf, victim);
    imc_to_char("Ok.\n\r", ch);
#ifdef ROM
    sprintf(buf, "$N revokes %s's access to %s.", GET_NAME(victim), name);
    wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
#endif
    return;

  case 1: /* set allow flag */
    if (IS_SET(IMC_ALLOW_FLAGS(victim), flag))
    {
      imc_to_char("Allow flag already set.\n\r", ch);
      return;
    }

    SET_BIT(IMC_ALLOW_FLAGS(victim), flag);
    REMOVE_BIT(IMC_DENY_FLAGS(victim), flag);
    sprintf(buf, "The gods have allowed you access to %s.\n\r", name);
    imc_to_char(buf, victim);
    imc_to_char("Ok.\n\r", ch);
#ifdef ROM
    sprintf(buf, "$N allows %s access to %s.", GET_NAME(victim), name);
    wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
#endif
    return;

  case 2: /* clears flags */
    if (IS_SET(IMC_ALLOW_FLAGS(victim), flag))
    {
      REMOVE_BIT(IMC_ALLOW_FLAGS(victim), flag);
      sprintf(buf, "The gods have removed your special access to %s.\n\r",
	      name);
      imc_to_char(buf, victim);
#ifdef ROM
      sprintf(buf, "$N removes %s's access to %s.", GET_NAME(victim), name);
      wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
#endif
      found=TRUE;
    }

    if (IS_SET(IMC_DENY_FLAGS(victim), flag))
    {
      REMOVE_BIT(IMC_DENY_FLAGS(victim), flag);
      sprintf(buf, "The gods have restored your %s priviliges.\n\r", name);
      imc_to_char(buf, victim);
#ifdef ROM
      sprintf(buf, "$N restores %s to %s.", name, GET_NAME(victim));
      wiznet(buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
#endif
      found=TRUE;
    }
    
    if (!found)
      imc_to_char("They have no flags to clear.\n\r", ch);
    else
      imc_to_char("Ok.\n\r", ch);
    return;
  }
}

/* renamed from do_rchannel */
static void do_imcchannel(const imc_char_data *from, int number,
			  const char *argument, int emote)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;
  const char *str;
  const char *arg;
  char buf[IMC_DATA_LENGTH];
  int chan;

  for (chan=0; chan<numchannels; chan++)
    if (imc_channels[chan].number==number)
      break;

  if (chan==numchannels)
    return;

  str=emote ? imc_channels[chan].emotestr : imc_channels[chan].chatstr;
  arg=color_itom(argument);

  for (d=descriptor_list; d; d=d->next)
  {
    if (d->connected==CON_PLAYING &&
	(victim=d->original ? d->original : d->character)!=NULL &&
	!IS_QUIET(victim) &&
	!IS_SILENT(victim) &&
	!IS_SET(IMC_DEAF_FLAGS(victim), imc_channels[chan].flag) &&
	CAN(victim, imc_channels[chan].flag, imc_channels[chan].minlevel))
    {
#if NO_VSNPRINTF
      sprintf(buf, str, getname(victim, from), arg);
#else
      snprintf(buf, IMC_DATA_LENGTH, str, getname(victim, from), arg);
#endif
      imc_to_char(buf, victim);
    }
  }
}

/*  Traceroute and ping.
 *
 *  Be lazy - only remember the last traceroute
 */

static char lastping[IMC_MNAME_LENGTH];
static char pinger[100];

DEFINE_DO_FUN(do_rping)
{
  char arg[MAX_STRING_LENGTH];
  struct timeval tv;
  
  CHECKIMC(ch);
  
#ifdef CIRCLE
  skip_spaces(&argument);
#endif

  one_argument(argument, arg);
  
  if (!arg[0])
  {
    imc_to_char("Ping which mud?\n\r", ch);
    return;
  }

  CHECKMUD(ch, arg);

  gettimeofday(&tv, NULL);
  strcpy(lastping, arg);
#ifdef CIRCLE
  strcpy(pinger, ch->player.name);
#else
  strcpy(pinger, ch->name );
#endif
  imc_send_ping(arg, tv.tv_sec, tv.tv_usec);
}

void imc_traceroute(int ping, const char *pathto, const char *pathfrom)
{
  if (!str_cmp(imc_firstinpath(pathfrom), lastping))
  {
    DESCRIPTOR_DATA *d;
    CHAR_DATA *ch=NULL;
    char buf[MAX_STRING_LENGTH];

    for (d=descriptor_list; d; d=d->next)
    {
      if (d->connected == CON_PLAYING &&
	  (ch =d->original ? d->original : d->character)!=NULL &&
#ifdef CIRCLE
          !str_cmp(ch->player.name, pinger))
#else
	  !str_cmp(ch->name, pinger))
#endif
	break;
    }
    
    if (!ch)
      return;

    sprintf(buf,
	    "%s: %dms round-trip-time.\n\r"
	    "Return path: %s\n\r"
	    "Send path:   %s\n\r",
	    imc_firstinpath(pathfrom),
	    ping,
	    pathfrom,
	    pathto ? pathto : "unknown");

    imc_to_char(buf, ch);
  }
}

void imc_recv_chat(const imc_char_data *from, int channel,
		   const char *argument)
{
  do_imcchannel(from, channel, argument, 0);
}

void imc_recv_emote(const imc_char_data *from, int channel,
		    const char *argument)
{
  do_imcchannel(from, channel, argument, 1);
}

/* new imc_whoreply_* support, see imc-interp.c */

/* expanded for minimal mud-specific code. I really don't want to replicate
 * stock in-game who displays here, since it's one of the most commonly
 * changed pieces of code. shrug.
 */
#ifndef LOCAL_RWHO_REPLY_ROUTINE
void process_rwho(const imc_char_data *from, const char *argument)
{
  CHAR_DATA *victim;
  int count=0;
  char output[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;

  imc_whoreply_start(from->name);
  
#ifdef ACK
  imc_whoreply_add(color_mtoi("@@RWho's Playing " mudnamecolor ":\n\r"));
#endif

  for (d = descriptor_list; d; d = d->next)
  {
    victim = ( d->original ) ? d->original : d->character;

    if (d->connected != CON_PLAYING || !imc_visible(from, imc_getdata(victim)) ||
        IS_NPC(victim))
      continue;

    count++;
    
#ifdef ACK
    sprintf(output,
	    "@@W[@@p%s@@W] @@G%s@@N%s@@N\n\r",
	    ((IS_IMMORTAL(victim) ||
	      (victim->adept_level>0)) ? victim->pcdata->who_name
	     : is_remort(victim) ? "    @@mREMORT@@N    "
	     : "    @@cMORTAL@@N    "),
	    GET_NAME(victim), GET_TITLE(victim));
#elif defined(CIRCLE)
    sprintf(output, "%s %s\r\n",
            GET_NAME(victim),
            GET_TITLE(victim));
#else
    sprintf(output, "%s%s\n\r",
	    GET_NAME(victim),
	    GET_TITLE(victim));
#endif

    imc_whoreply_add(color_mtoi(output));
  }

  sprintf(output, "\n\rRWHO for %s [%d players found]\n\r", imc_name, count);
  imc_whoreply_add(output);
  imc_whoreply_end();
}
#endif
/* edit this if you want to support rfinger */
#ifndef LOCAL_RFINGER_ROUTINE
void process_rfinger(const imc_char_data *from, const char *argument)
{
  imc_send_whoreply(from->name,
		    "Sorry, no information is available of that type.\n\r", -1);
}
#endif

void imc_recv_who(const imc_char_data *from, const char *type)
{
  char arg[MAX_STRING_LENGTH];
  char output[MAX_STRING_LENGTH];

  type=imc_getarg(type, arg, MAX_STRING_LENGTH);

  if (!str_cmp(arg, "who"))
  {
    process_rwho(from, type);
    return;
  }
  else if (!str_cmp(arg, "finger"))
  {
    process_rfinger(from, type);
    return;
  }
  else if (!str_cmp(arg, "info"))
    sprintf(output, "Site Information --\r\nSite Name: %s\r\nSite Host: %s\r\nAdmin Email: %s\r\nAdmin MUDMail: %s\r\nWeb Site: %s\r\nIMC Information --\r\nIMC Version: %s\r\nIMC Details: %s\r\nIMC Flags: %s\r\nMUD Information:\r\n%s\r\n",
	imc_siteinfo.name,
    imc_siteinfo.host,
    imc_siteinfo.email,
    imc_siteinfo.imail,
    imc_siteinfo.www,
    IMC_VERSIONID,
    imc_siteinfo.details,
    imc_siteinfo.flags,
	IMC_MUD_INFO);
  else if (!str_cmp(arg, "list"))
    strcpy(output, imc_list(3));
  else if (!str_cmp(arg, "config"))
    strcpy(output, imc_list(4));
  else if (!str_cmp(arg, "direct"))
    strcpy(output, imc_list(0));
  else if (!str_cmp(arg, "istats"))
    imc_send_whoreply(from->name, imc_getstats(), -1);
  else if (!str_cmp(arg, "options") ||
	   !str_cmp(arg, "services") ||
	   !str_cmp(arg, "help"))
    strcpy(output,
	   "Available rquery types:\n\r"
	   "help       - this list\n\r"
	   "who        - who listing\n\r"
	   "info       - mud information\n\r"
	   "list       - active IMC connections\n\r"
	   "direct     - direct IMC connections\n\r"
	   "config     - local IMC configuration\n\r"
	   "istats     - network traffic statistics\n\r"
	   /* uncomment if you support rfinger */
	   /* "finger xxx - finger player xxx\n\r" */
	   );
  else
    strcpy(output, "Sorry, no information is available of that type.\n\r");

  imc_send_whoreply(from->name, color_mtoi(output), -1);
}

void imc_recv_whoreply(const char *to, const char *text, int sequence)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;

  for (d=descriptor_list; d; d=d->next)
  {
    if (d->connected==CON_PLAYING &&
	(victim=d->original ? d->original : d->character)!=NULL &&
        is_name((char *)to, GET_NAME(victim)))
    {
      imc_to_pager(color_itom(text), victim);
      return;
    }
  }
}

void imc_recv_whois(const imc_char_data *from, const char *to)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];

  for (d=descriptor_list; d; d = d->next)
  {
    if (d->connected==CON_PLAYING &&
	(victim = d->original ? d->original : d->character)!=NULL &&
	!IS_NPC(victim) &&
	imc_visible(from, imc_getdata(victim)) &&
#ifdef CIRCLE
	(!str_cmp((char *)to, GET_NAME(victim)) ||
#else
	(!str_cmp(to, GET_NAME(victim)) ||
#endif
	 (strlen(to)>3 && !str_prefix(to, GET_NAME(victim)))))
    {
      sprintf(buf, "rwhois %s : %s@%s is online.\n\r", to,
	      GET_NAME(victim), imc_name);
      imc_send_whoisreply(from->name, buf);
    }
  }
}

void imc_recv_whoisreply(const char *to, const char *text)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;

  for (d=descriptor_list; d; d=d->next)
  {
    if (d->connected==CON_PLAYING &&
	(victim=d->original ? d->original : d->character)!=NULL &&
        !str_cmp((char *)to, GET_NAME(victim)))
    {
#ifdef CIRCLE
      imc_to_char((char *)color_itom(text), victim);
#else
      imc_to_char(color_itom(text), victim);
#endif
      return;
    }
  }
}

void imc_recv_tell(const imc_char_data *from, const char *to,
		   const char *argument, int isreply)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim, *vch;
  char buf[IMC_DATA_LENGTH];

  if (!strcmp(to, "*")) /* ignore messages to system */
    return;

  victim=NULL;
  for (d=descriptor_list; d; d=d->next)
  {
    if (d->connected==CON_PLAYING &&
	(vch=d->original ? d->original : d->character)!=NULL &&
	!IS_NPC(vch) &&  /* sanity check */
        !IS_RINVIS(vch) &&
        (isreply || imc_visible(from, imc_getdata(vch))))
    {
      if (!str_cmp((char *)to, GET_NAME(vch)))
      {
	victim=vch;
	break;
      }
      if (is_name((char *)to, GET_NAME(vch)))
	victim=vch;
    }
  }

  if (victim)
  {
    if (IS_SET(IMC_DEAF_FLAGS(victim), IMC_RTELL) ||
	IS_QUIET(victim) ||
#ifdef ROM    
	IS_SET(victim->comm, COMM_DEAF) ||
#endif
#ifdef ACK
	IS_SET(victim->pcdata->pflags, PFLAG_AFK) ||
#endif
	IS_SILENT(victim) ||
	IS_SET(IMC_DENY_FLAGS(victim), IMC_RTELL))
    {
#ifdef CIRCLE
      if (str_cmp((char *)imc_nameof(from->name), "*"))
#else
      if (str_cmp(imc_nameof(from->name), "*"))
#endif
      {
	sprintf(buf, "%s is not receiving tells.", to);
	imc_send_tell(NULL, from->name, buf, 1);
      }
      return;
    }
    
#ifdef ROM
    if (IS_SET(victim->comm, COMM_AFK) &&
	str_cmp(imc_nameof(from->name), "*"))
    {
      sprintf(buf, "%s is AFK, but your tell will go through when they "
	      "return.", to);
      imc_send_tell(NULL, from->name, buf, 1);
    }
    else if (!victim->desc &&
	     str_cmp(imc_nameof(from->name), "*"))
    {
      sprintf(buf, "%s is switched, but your tell will go through when "
	      "they return.", to);
      imc_send_tell(NULL, from->name, buf, 1);
    }
#endif

#ifdef CIRCLE
    if (str_cmp((char *)imc_nameof(from->name), "*"))     /* not a system message */
#else
    if (str_cmp(imc_nameof(from->name), "*"))     /* not a system message */
#endif
    {
      if (IMC_RREPLY(victim))
	imc_strfree(IMC_RREPLY(victim));
      if (IMC_RREPLY_NAME(victim))
	imc_strfree(IMC_RREPLY_NAME(victim));
      
      IMC_RREPLY(victim)=imc_strdup(from->name);
      IMC_RREPLY_NAME(victim)=imc_strdup(getname(victim, from));
    }	
    
#if NO_VSNPRINTF
    sprintf(buf,
            RTELL_FORMAT_2,
	    getname(victim, from),
	    color_itom(argument));
#else
    snprintf(buf, IMC_DATA_LENGTH,
	     RTELL_FORMAT_2,
	     getname(victim, from), color_itom(argument));
#endif

#ifdef ROM
    if (IS_SET(victim->comm, COMM_AFK) || !victim->desc)
      add_buf(victim->pcdata->buffer, buf);
    else
      imc_to_pager(buf, victim);
#else
    imc_to_pager(buf, victim);
#endif
  }
  else
  {
    sprintf(buf, "%s is not here.", to);
    imc_send_tell(NULL, from->name, buf, 1);
  }
}

void imc_recv_beep(const imc_char_data *from, const char *to)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim, *vch;
  char buf[IMC_DATA_LENGTH];

  if (!strcmp(to, "*")) /* ignore messages to system */
    return;

  victim=NULL;
  for (d=descriptor_list; d; d=d->next)
  {
    if (d->connected==CON_PLAYING &&
	(vch=d->original ? d->original : d->character)!=NULL &&
	imc_visible(from, imc_getdata(vch)))
    {
      if (!str_cmp((char *)to, GET_NAME(vch)))
      {
	victim=vch;
	break;
      }
      if (is_name((char *)to, GET_NAME(vch)))
	victim=vch;
    }
  }

  if (victim)
  {
    if (IS_SET(IMC_DEAF_FLAGS(victim), IMC_RBEEP) ||
	IS_SILENT(victim) ||
	!CAN(victim, IMC_RTELL, IMC_LEVEL_RTELL))
    {
      sprintf(buf, "%s is not receiving beeps.", to);
      imc_send_tell(NULL, from->name, buf, 1);
      return;
    }
    
    /* always display the true name here */
    sprintf(buf, RBEEP_FORMAT_2, from->name);
    imc_to_char(buf, victim);
  }
  else
  {
    sprintf(buf, "%s is not here", to);
    imc_send_tell(NULL, from->name, buf, 1);
  }
}



/**  IMC MAIL  **/

/* shell for sending IMC mail */

void imc_post_mail(CHAR_DATA *from,
		   const char *sender,
		   const char *to_list,
		   const char *subject,
		   const char *date,
		   const char *text)
{
  char arg[MAX_STRING_LENGTH];
  const char *temp;

  CHECKIMC(from);

  if (!strchr(to_list, '@'))
    return;

  for (temp=one_argument((char *)to_list, arg); arg[0]; /* blech */
       temp=one_argument((char *)temp, arg))
    if (strchr(arg, '@'))
      CHECKMUD(from, imc_mudof(arg));

  imc_send_mail(sender, to_list, subject, date, color_mtoi(text));
}

/* Since there are half-a-dozen different ways that people do this, I'm
 * sticking to the stock code, plus Erwin's board system.
 */

#ifdef SMAUG

/*
 * Find the IMC mail board.  It SHOULD exist.. :)
 */
BOARD_DATA *imc_find_board(void)
{
  BOARD_DATA *board;
  
  for (board = first_board; board; board = board->next)
    if (board->board_obj == sysdata.imc_mail_vnum)
      return board;
  return NULL;
}

char *imc_mail_arrived (const char *from, const char *to, const char *date,
                        const char *subject, const char *text)
{
  void write_board(BOARD_DATA *board);
  BOARD_DATA *board;
  NOTE_DATA *pnote;
  char buf[MAX_STRING_LENGTH];
  
  /* don't let people write to 'all' over IMC */
  /* yeesh, why doesn't anyone maintain 'const'-ness any more? */
  
  if (is_name("all", (char *)to))
    return "Notes to 'all' are not accepted.";
  
  if ( !(board = imc_find_board()) )
    return "Could not find the IMC board.";
  
  if ( board->num_posts >= board->max_posts )
    return "There is no room on the IMC board to post your note.";
  
  CREATE( pnote, NOTE_DATA, 1 );
  
  /* Smash all the tildes, just in case
   * This is a complete mess, but smaug's string hasher would do bad things
   * if we smash_tilde'd a STRALLOC'd string..
   */

#define SET_STR(new, old) \
  do { \
    strcpy(buf, old); \
    smash_tilde(buf); \
    pnote->new = STRALLOC(buf); \
  } while(0)

  SET_STR(sender, from);
  SET_STR(to_list, to);
  SET_STR(date, date);
  SET_STR(subject, color_itom(subject));
  SET_STR(text, text);
#undef SET_STR

  pnote->voting = 0;
  pnote->yesvotes = str_dup("");
  pnote->novotes = str_dup("");
  pnote->abstentions = str_dup("");

  LINK(pnote, board->first_note, board->last_note, next, prev);
  board->num_posts++;
  write_board( board );
  
  return NULL;                          /* Errors? What errors? <G> */
}

#endif /* SMAUG */

#if defined(ACK)
/* Added support for ackmud boards.
 * You must configure IMC_BOARD_PUBLIC and IMC_BOARD_IMMORTAL
 * in imc-mercbase.h to suit your mud!
 */
#ifndef IMC_BOARD_PUBLIC
#error You must #define a value for IMC_BOARD_VNUM in imc-mercbase.h!
#endif
#ifndef IMC_BOARD_IMMORTAL
#error You must #define a value for IMC_BOARD_VNUM in imc-mercbase.h!
#endif

char *imc_mail_arrived(const char *from, const char *to, const char *date,
		       const char *subject, const char *text)
{
  BOARD_DATA *board;
  MESSAGE_DATA *msg;
  char buf[MAX_STRING_LENGTH];
  const char *temp=to;
  int vnum;
  int toimm=0;
  char toname[MAX_STRING_LENGTH];

  toname[0]=0;
  while ((temp=imc_getarg(temp, buf, MAX_STRING_LENGTH)), buf[0])
  {
    if (!strchr(buf, '@'))
    {
      if (toname[0])
	return "Sorry, multiple to: names over IMC are not supported here.";
      strcpy(toname, buf);
    }
  }

  if (str_cmp("all", toname))
    return "Notes to 'all' are not accepted.";
  
  if (str_cmp("immortal", toname) || str_cmp("imm", toname) ||
      str_cmp("imms", toname) || str_cmp("immortals", toname))
  {
    vnum=IMC_BOARD_IMMORTAL;
    toimm=1;
  }
  else
    vnum=IMC_BOARD_PUBLIC;

  /* First find the board, and if not there, create one. */
  for (board=first_board; board != NULL; board=board->next)
  {
    if (board->vnum==vnum)
      break;
  }

  if (board==NULL)
  {
    /* uh oh.. */
    OBJ_INDEX_DATA *pObj;
    int i;

    for (i=0; i<MAX_KEY_HASH; i++)
      for (pObj=obj_index_hash[i]; pObj; pObj=pObj->next)
	if (pObj->item_type==ITEM_BOARD && pObj->value[3]==vnum)
          break;

    if (!pObj)
      return "Couldn't find a board to post the note on.";
    
    board=load_board(pObj);
  }

  GET_FREE(msg, message_free);
  msg->datetime=time(NULL);

  /* erk.. this is messy.
   * assume that notes to 'immortal' are public
   * assume that anything else is private
   */

  if (toimm)
    sprintf(buf, "%s @@a%s@@N\n\r", subject, date);
  else
    sprintf(buf, "to: %s @@a%s@@N\n\r", toname, date);
  msg->title=str_dup(buf);
  msg->author=str_dup(from);
  msg->board=board;
  msg->message=str_dup(color_itom(text));
  LINK(msg, board->first_message, board->last_message, next, prev);

  return NULL;
}
#endif /* ACK */

/* #define ERWIN   -- to use the code for Erwin's boards */

#if defined(MERC) || defined(ROM) || defined(ENVY)
#if !defined(ERWIN)

/* post a note; currently all rnotes go to the default note spool */
char *imc_mail_arrived(const char *from, const char *to, const char *date,
		       const char *subject, const char *text)
{
  NOTE_DATA *pnote;
#if defined(MERC) || defined(ENVY)
  NOTE_DATA *prev_note;
  FILE *fp;
#endif

  /* don't let people write to 'all' over IMC */
  /* yeesh, why doesn't anyone maintain 'const'-ness any more? */

  if (is_name("all", (char *)to))
    return "Notes to 'all' are not accepted.";

#ifdef ROM
  pnote=new_note();
  pnote->type=NOTE_NOTE;
#else
  pnote=alloc_perm(sizeof(*pnote));
#endif

  pnote->sender=str_dup(from);
  pnote->to_list=str_dup(to);
  pnote->date=str_dup(date);
  pnote->subject=str_dup(subject);
  pnote->text=str_dup(color_itom(text));
  pnote->date_stamp=current_time;
  pnote->next=NULL;

  /* a bit of paranoia re. tildes here.. */
  smash_tilde(pnote->sender);
  smash_tilde(pnote->to_list);
  smash_tilde(pnote->date);
  smash_tilde(pnote->subject);
  smash_tilde(pnote->text);

#ifdef ROM
  append_note(pnote);
#else
  if (!note_list)
  {
    note_list = pnote;
  }
  else
  {
    for (prev_note = note_list; prev_note->next; prev_note = prev_note->next)
      ;
    prev_note->next = pnote;
  }
  
  fclose (fpReserve);
  if (!(fp = fopen (NOTE_FILE, "a")))
  {
    perror (NOTE_FILE);
    fpReserve = fopen (NULL_FILE, "r");
    return "Couldn't open the note file";
  }
  else
  {
    fprintf (fp, "Sender  %s~\n", pnote->sender);
    fprintf (fp, "Date    %s~\n", pnote->date);
    fprintf (fp, "Stamp   %ld\n", (unsigned long) pnote->date_stamp);
    fprintf (fp, "To      %s~\n", pnote->to_list);
    fprintf (fp, "Subject %s~\n", pnote->subject);
    fprintf (fp, "Text\n%s~\n\n", pnote->text);
    fclose (fp);
  }
  fpReserve = fopen (NULL_FILE, "r");
#endif

  return NULL;                            /* Errors? What errors? <G> */
}

#else

/*
 * If you are using my code (ftp://pip.dknet.dk/pub/pip1773/board-2.tgz)
 * there is another imc_mail_arrived() below - Erwin
 *
 * Note that this assumes that your make_note() does a smash_tilde(), the
 * latest version of the board snippet does, but your may not. Oh well.
 */

char *imc_mail_arrived (const char *from, const char *to, const char *date,
                        const char *subject, const char *text)
{
  /* don't let people write to 'all' over IMC */
  /* yeesh, why doesn't anyone maintain 'const'-ness any more? */

  if (is_name("all", (char *)to))
    return "Notes to 'all' are not accepted.";
  
  make_note ("Personal", from, to, subject, 14, color_itom(text));
  return NULL;
}

#endif /* Erwin's board code */
#endif /* merc/rom/envy */

#ifdef CIRCLE
char *imc_mail_arrived(const char *from, const char *to, const char *date,
                       const char *subject, const char *text)
{
  char buf[MAX_STRING_LENGTH];

  /* don't let people write to 'all' over IMC */
  /* yeesh, why doesn't anyone maintain 'const'-ness any more? */

  if (isname("all", (char *) to))
    return "Notes to 'all' are not accepted.";

  sprintf(buf,
          "-= IMC Header =-\r\n"
          "   From: %s\r\n"
          "     To: %s\r\n"
          "   Date: %s\r\n"
          "Subject: %s\r\n"
          "\r\n%s",
          from,
          to,
          date,
          subject,
          text);

  store_mail(get_id_by_name((char *)to), 0, buf);

  return NULL;                  /* Errors? What errors? <G> */
}
#endif

void imc_log(const char *string)
{
#ifdef LOCAL_LOG_ROUTINE
  imc_local_log(string);
#else
  char buf[MAX_STRING_LENGTH];
 
  sprintf(buf, "imc: %s", string);
  if(imc_log_on) /* configurable log messages */
  {
	  log_string(buf);
#ifdef ACK
  	monitor_chan(buf, MONITOR_CONNECT);
#endif

#ifdef CIRCLE
  	mudlog(buf, BRF, LVL_IMMORT, FALSE); /* FALSE b/c it's already written to file
										this fix to make imc syslogs show up inside
										CircleMUDs. -- Scion */
#endif

#ifdef ROM
  /*  ick, wiznet is a bit broken wrt. passing non-objects to act..
   *  it should really take a void * as its 3rd argument
   */
  	wiznet("imc: $t", NULL, (OBJ_DATA *)string, WIZ_IMC, 0, 0);
#endif
  }
#endif
}

void imc_debug(const imc_connect *c, int out, const char *string)
{
  char buf[MAX_STRING_LENGTH];
  char *dir;

  dir=out ? "<" : ">";

  /* chop to 130 chars (should fit on 2 lines) */
  sprintf(buf, "%s %s %-.130s", imc_getconnectname(c), dir, string);
#ifdef ROM
  wiznet("$t", NULL, (OBJ_DATA *)buf, WIZ_IMCDEBUG, 0, 0);
#endif
#ifdef CIRCLE
  mudlog(buf, CMP, LVL_IMPL, FALSE); /* Scion */
#endif

}

