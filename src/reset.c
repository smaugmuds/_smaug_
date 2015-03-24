/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops, Fireblade, Edmond, Conran                         |             *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *			 Online Reset Editing Module			    *
 ****************************************************************************/

/*
 * This file relies heavily on the fact that your linked lists are correct,
 * and that pArea->reset_first is the first reset in pArea.  Likewise,
 * pArea->reset_last *MUST* be the last reset in pArea.  Weird and
 * wonderful things will happen if any of your lists are messed up, none
 * of them good.  The most important are your pRoom->contents,
 * pRoom->people, rch->carrying, obj->contains, and pArea->reset_first ..
 * pArea->reset_last.  -- Altrag
 */
 
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


/* Externals */
extern	int	top_reset;
char *		sprint_reset	args( ( CHAR_DATA *ch, RESET_DATA *pReset,
					sh_int num, bool rlist ) );
RESET_DATA *	parse_reset	args( ( AREA_DATA *tarea, char *argument,
					CHAR_DATA *ch ) );
int		get_wearloc	args( ( char *type ) );
int		get_trapflag	args( ( char *flag ) );
int		get_exflag	args( ( char *flag ) );
int		get_rflag	args( ( char *flag ) );
extern	char *	const		wear_locs[];
extern	char *	const		ex_flags[];

bool is_room_reset  args( ( RESET_DATA *pReset, ROOM_INDEX_DATA *aRoom,
                            AREA_DATA *pArea ) );
void add_obj_reset  args( ( AREA_DATA *pArea, char cm, OBJ_DATA *obj,
                            int v2, int v3 ) );
void delete_reset   args( ( AREA_DATA *pArea, RESET_DATA *pReset ) );
void instaroom      args( ( AREA_DATA *pArea, ROOM_INDEX_DATA *pRoom,
			    bool dodoors ) );
#define RID ROOM_INDEX_DATA
RID *find_room      args( ( CHAR_DATA *ch, char *argument,
                            ROOM_INDEX_DATA *pRoom ) );
#undef RID
void edit_reset     args( ( CHAR_DATA *ch, char *argument, AREA_DATA *pArea,
                            ROOM_INDEX_DATA *aRoom ) );
#define RD RESET_DATA
RD *find_reset      args( ( AREA_DATA *pArea, ROOM_INDEX_DATA *pRoom,
			    int num ) );
#undef RD
void list_resets    args( ( CHAR_DATA *ch, AREA_DATA *pArea,
			    ROOM_INDEX_DATA *pRoom, int start, int end, char extraarg ) );



RESET_DATA *find_reset(AREA_DATA *pArea, ROOM_INDEX_DATA *pRoom, int numb)
{
  RESET_DATA *pReset;
  int num = 0;
  
  for ( pReset = pArea->first_reset; pReset; pReset = pReset->next )
    if ( is_room_reset(pReset, pRoom, pArea) && ++num >= numb )
      return pReset;
  return NULL;
}

/* This is one loopy function.  Ugh. -- Altrag */
bool is_room_reset( RESET_DATA *pReset, ROOM_INDEX_DATA *aRoom,
                    AREA_DATA *pArea )
{
  ROOM_INDEX_DATA *pRoom;
  RESET_DATA *reset;
  int pr;
  
  if ( !aRoom )
    return TRUE;
  switch( pReset->command )
  {
  case 'M':
  case 'O':
    pRoom = get_room_index( pReset->arg3 );
    if ( !pRoom || pRoom != aRoom )
      return FALSE;
    return TRUE;
  case 'P':
  case 'T':
  case 'H':
    if ( pReset->command == 'H' )
      pr = pReset->arg1;
    else
      pr = pReset->arg3;
    for ( reset = pReset->prev; reset; reset = reset->prev )
      if ( (reset->command == 'O' || reset->command == 'P' ||
            reset->command == 'G' || reset->command == 'E') &&
           (!pr || pr == reset->arg1) && get_obj_index(reset->arg1) )
        break;
    if ( reset && is_room_reset(reset, aRoom, pArea) )
      return TRUE;
    return FALSE;
  case 'B':
    switch(pReset->arg2 & BIT_RESET_TYPE_MASK)
    {
    case BIT_RESET_DOOR:
    case BIT_RESET_ROOM:
      return (aRoom->vnum == pReset->arg1);
    case BIT_RESET_MOBILE:
      for ( reset = pReset->prev; reset; reset = reset->prev )
        if ( reset->command == 'M' && get_mob_index(reset->arg1) )
          break;
      if ( reset && is_room_reset(reset, aRoom, pArea) )
        return TRUE;
      return FALSE;
    case BIT_RESET_OBJECT:
      for ( reset = pReset->prev; reset; reset = reset->prev )
        if ( (reset->command == 'O' || reset->command == 'P' ||
              reset->command == 'G' || reset->command == 'E') &&
             (!pReset->arg1 || pReset->arg1 == reset->arg1) &&
              get_obj_index(reset->arg1) )
          break;
      if ( reset && is_room_reset(reset, aRoom, pArea) )
        return TRUE;
      return FALSE;
    }
    return FALSE;
  case 'G':
  case 'E':
    for ( reset = pReset->prev; reset; reset = reset->prev )
      if ( reset->command == 'M' && get_mob_index(reset->arg1) )
        break;
    if ( reset && is_room_reset(reset, aRoom, pArea) )
      return TRUE;
    return FALSE;
  case 'D':
  case 'R':
    pRoom = get_room_index( pReset->arg1 );
    if ( !pRoom || pRoom->area != pArea || (aRoom && pRoom != aRoom) )
      return FALSE;
    return TRUE;
  default:
    return FALSE;
  }
  return FALSE;
}

ROOM_INDEX_DATA *find_room( CHAR_DATA *ch, char *argument,
                            ROOM_INDEX_DATA *pRoom )
{
  char arg[MAX_INPUT_LENGTH];
  
  if ( pRoom )
    return pRoom;
  one_argument(argument, arg);
  if ( !is_number(arg) && arg[0] != '\0' )
  {
    send_to_char( "Reset to which room?\n\r", ch );
    return NULL;
  }
  if ( arg[0] == '\0' )
    pRoom = ch->in_room;
  else
    pRoom = get_room_index(atoi(arg));
  if ( !pRoom )
  {
    send_to_char( "Room does not exist.\n\r", ch );
    return NULL;
  }
  return pRoom;
}

/* Separate function for recursive purposes */
#define DEL_RESET(area, reset, rprev) \
do { \
  rprev = reset->prev; \
  delete_reset(area, reset); \
  reset = rprev; \
  continue; \
} while(0)
void delete_reset( AREA_DATA *pArea, RESET_DATA *pReset )
{
  RESET_DATA *reset;
  RESET_DATA *reset_prev;

  if ( pReset->command == 'M' )
  {
    for ( reset = pReset->next; reset; reset = reset->next )
    {
      /* Break when a new mob found */
      if ( reset->command == 'M' )
        break;
      /* Delete anything mob is holding */
      if ( reset->command == 'G' || reset->command == 'E' )
        DEL_RESET(pArea, reset, reset_prev);
      if ( reset->command == 'B' &&
          (reset->arg2 & BIT_RESET_TYPE_MASK) == BIT_RESET_MOBILE &&
          (!reset->arg1 || reset->arg1 == pReset->arg1) )
        DEL_RESET(pArea, reset, reset_prev);
    }
  }
  else if ( pReset->command == 'O' || pReset->command == 'P' ||
            pReset->command == 'G' || pReset->command == 'E' )
  {
    for ( reset = pReset->next; reset; reset = reset->next )
    {
      if ( reset->command == 'T' &&
          (!reset->arg3 || reset->arg3 == pReset->arg1) )
        DEL_RESET(pArea, reset, reset_prev);
      if ( reset->command == 'H' &&
          (!reset->arg1 || reset->arg1 == pReset->arg1) )
        DEL_RESET(pArea, reset, reset_prev);
      /* Delete nested objects, even if they are the same object. */
      if ( reset->command == 'P' && (reset->arg3 > 0 ||
           pReset->command != 'P' || reset->extra-1 == pReset->extra) &&
          (!reset->arg3 || reset->arg3 == pReset->arg1) )
        DEL_RESET(pArea, reset, reset_prev);
      if ( reset->command == 'B' &&
          (reset->arg2 & BIT_RESET_TYPE_MASK) == BIT_RESET_OBJECT &&
          (!reset->arg1 || reset->arg1 == pReset->arg1) )
        DEL_RESET(pArea, reset, reset_prev);

      /* Break when a new object of same type is found */
      if ( (reset->command == 'O' || reset->command == 'P' ||
            reset->command == 'G' || reset->command == 'E') &&
           reset->arg1 == pReset->arg1 )
        break;
    }
  }
  if ( pReset == pArea->last_mob_reset )
    pArea->last_mob_reset = NULL;
  if ( pReset == pArea->last_obj_reset )
    pArea->last_obj_reset = NULL;
  UNLINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
  DISPOSE(pReset);
  return;
}
#undef DEL_RESET

RESET_DATA *find_oreset(CHAR_DATA *ch, AREA_DATA *pArea,
			ROOM_INDEX_DATA *pRoom, char *name)
{
  RESET_DATA *reset;
  
  if ( !*name )
  {
    for ( reset = pArea->last_reset; reset; reset = reset->prev )
    {
      if ( !is_room_reset(reset, pRoom, pArea) )
        continue;
      switch(reset->command)
      {
      default:
        continue;
      case 'O': case 'E': case 'G': case 'P':
        break;
      }
      break;
    }
    if ( !reset )
      send_to_char( "No object resets in list.\n\r", ch );
    return reset;
  }
  else
  {
    char arg[MAX_INPUT_LENGTH];
    int cnt = 0, num = number_argument(name, arg);
    OBJ_INDEX_DATA *pObjTo = NULL;
    
    for ( reset = pArea->first_reset; reset; reset = reset->next )
    {
      if ( !is_room_reset(reset, pRoom, pArea) )
        continue;
      switch(reset->command)
      {
      default:
        continue;
      case 'O': case 'E': case 'G': case 'P':
        break;
      }
      if ( (pObjTo = get_obj_index(reset->arg1)) &&
            is_name(arg, pObjTo->name) && ++cnt == num )
        break;
    }
    if ( !pObjTo || !reset )
    {
      send_to_char( "To object not in reset list.\n\r", ch );
      return NULL;
    }
  }
  return reset;
}

RESET_DATA *find_mreset(CHAR_DATA *ch, AREA_DATA *pArea,
			ROOM_INDEX_DATA *pRoom, char *name)
{
  RESET_DATA *reset;
  
  if ( !*name )
  {
    for ( reset = pArea->last_reset; reset; reset = reset->prev )
    {
      if ( !is_room_reset(reset, pRoom, pArea) )
        continue;
      switch(reset->command)
      {
      default:
        continue;
      case 'M':
        break;
      }
      break;
    }
    if ( !reset )
      send_to_char( "No mobile resets in list.\n\r", ch );
    return reset;
  }
  else
  {
    char arg[MAX_INPUT_LENGTH];
    int cnt = 0, num = number_argument(name, arg);
    MOB_INDEX_DATA *pMob = NULL;
    
    for ( reset = pArea->first_reset; reset; reset = reset->next )
    {
      if ( !is_room_reset(reset, pRoom, pArea) )
        continue;
      switch(reset->command)
      {
      default:
        continue;
      case 'M':
        break;
      }
      if ( (pMob = get_mob_index(reset->arg1)) &&
            is_name(arg, pMob->player_name) && ++cnt == num )
        break;
    }
    if ( !pMob || !reset )
    {
      send_to_char( "Mobile not in reset list.\n\r", ch );
      return NULL;
    }
  }
  return reset;
}

void edit_reset( CHAR_DATA *ch, char *argument, AREA_DATA *pArea,
		 ROOM_INDEX_DATA *aRoom )
{
  char arg[MAX_INPUT_LENGTH];
  RESET_DATA *pReset = NULL;
  RESET_DATA *reset = NULL;
  MOB_INDEX_DATA *pMob = NULL;
  ROOM_INDEX_DATA *pRoom;
  OBJ_INDEX_DATA *pObj;
  int num = 0;
  int vnum;
  char *origarg = argument;
  char extraarg = '\0';
  
  argument = one_argument(argument, arg);
  if ( !*arg || !str_cmp(arg, "?") )
  {
    char *nm = (ch->substate == SUB_REPEATCMD ? "" : (aRoom ? "rreset "
    		: "reset "));
    char *rn = (aRoom ? "" : " [room#]");
    ch_printf(ch, "Syntax: %s<list|edit|delete|add|insert|place%s>\n\r",
        nm, (aRoom ? "" : "|area"));
    ch_printf( ch, "Syntax: %sremove <#>\n\r", nm );
    ch_printf( ch, "Syntax: %smobile <mob#> [limit]%s\n\r", nm, rn );
    ch_printf( ch, "Syntax: %sobject <obj#> [limit [room%s]]\n\r", nm, rn );
    ch_printf( ch, "Syntax: %sobject <obj#> give <mob name> [limit]\n\r", nm );
    ch_printf( ch, "Syntax: %sobject <obj#> equip <mob name> <location> "
        "[limit]\n\r", nm );
    ch_printf( ch, "Syntax: %sobject <obj#> put <to_obj name> [limit]\n\r",
        nm );
    ch_printf( ch, "Syntax: %shide <obj name>\n\r", nm );
    ch_printf( ch, "Syntax: %strap <obj name> <type> <charges> <flags>\n\r",
        nm );
    ch_printf( ch, "Syntax: %strap room <type> <charges> <flags>\n\r", nm );
    ch_printf( ch, "Syntax: %sbit <set|toggle|remove> door%s <dir> "
        "<exit flags>\n\r", nm, rn );
    ch_printf( ch, "Syntax: %sbit <set|toggle|remove> object <obj name> "
        "<extra flags>\n\r", nm );
    ch_printf( ch, "Syntax: %sbit <set|toggle|remove> mobile <mob name> "
        "<affect flags>\n\r", nm );
    ch_printf( ch, "Syntax: %sbit <set|toggle|remove> room%s <room flags>"
        "\n\r", nm, rn );
    ch_printf( ch, "Syntax: %srandom <last dir>%s\n\r", nm, rn );
    if ( !aRoom )
    {
      send_to_char( "\n\r[room#] will default to the room you are in, "
          "if unspecified.\n\r", ch );
    }
    return;
  }
  if ( !str_cmp(arg, "on") )
  {
    ch->substate = SUB_REPEATCMD;
    ch->dest_buf = (aRoom ? (void *)aRoom : (void *)pArea);
    send_to_char( "Reset mode on.\n\r", ch );
    return;
  }
  if ( !aRoom && !str_cmp(arg, "area") )
  {
    if ( !pArea->first_reset )
    {
      send_to_char( "You don't have any resets defined.\n\r", ch );
      return;
    }
    num = pArea->nplayer;
    pArea->nplayer = 0;
    reset_area(pArea);
    pArea->nplayer = num;
    send_to_char( "Done.\n\r", ch );
    return;
  }
  
  if ( !str_cmp(arg, "list") )
  {
    int start, end, nNumber;
    start = -1;
    end = -1;
    nNumber = 0;

    for ( ;; )
    {
  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' )
      break;
  if ( is_number( arg ) )
  {
      switch ( ++nNumber )
      {
      case 1: start = atoi( arg ); break;
      case 2: end = atoi( arg ); break;
      default:
    send_to_char( "Only two number parameters allowed.\n\r", ch );
    return;
   }
  }
  else
  {
/* Tiki - March 2002
Check for reset type from below
  A = all objects (E, G, O, and P)
  B = bit                      D = door
  E = object worn by mob       G = object in mob's inventory
  H = hide flag on object      M = mob
  O = object placed in room    P = object placed in container
  R = randomized rooms (maze)  T = trap
*/
   extraarg = UPPER(arg[0]);
   switch( extraarg )
   {
   default:
    send_to_char("Reset list parameter must be one of the following:\n\r", ch);
    send_to_char("  A = all objects (E, G, O, and P)\n\r", ch);
    send_to_char("  B = bit                      D = door\n\r", ch);
    send_to_char("  E = object worn by mob       G = object in mob's inventory\n\r", ch);
    send_to_char("  H = hide flag on object      M = mob\n\r", ch);
    send_to_char("  O = object placed in room    P = object placed in container\n\r", ch);
    send_to_char("  R = randomized rooms (maze)  T = trap\n\r", ch);
    return;
   case 'A': case 'B': case 'D': case 'E': case 'G': case 'H':  
   case 'M': case 'O': case 'P': case 'R': case 'T':
    break;
   }
  }
 }
 list_resets(ch, pArea, aRoom, start, end, extraarg);
 return;
  }


  if ( !str_cmp(arg, "edit") )
  {
    argument = one_argument(argument, arg);
    if ( !*arg || !is_number(arg) )
    {
      send_to_char( "Usage: reset edit <number> <command>\n\r", ch );
      return;
    }
    num = atoi(arg);
    if ( !(pReset = find_reset(pArea, aRoom, num)) )
    {
      send_to_char( "Reset not found.\n\r", ch );
      return;
    }
    if ( !(reset = parse_reset(pArea, argument, ch)) )
    {
      send_to_char( "Error in reset.  Reset not changed.\n\r", ch );
      return;
    }
    reset->prev = pReset->prev;
    reset->next = pReset->next;
    if ( !pReset->prev )
      pArea->first_reset = reset;
    else
      pReset->prev->next = reset;
    if ( !pReset->next )
      pArea->last_reset  = reset;
    else
      pReset->next->prev = reset;
    DISPOSE(pReset);
    send_to_char( "Done.\n\r", ch );
    return;
  }
  if ( !str_cmp(arg, "add") )
  {
    if ( (pReset = parse_reset(pArea, argument, ch)) == NULL )
    {
      send_to_char( "Error in reset.  Reset not added.\n\r", ch );
      return;
    }
    add_reset(pArea, pReset->command, pReset->extra, pReset->arg1,
        pReset->arg2, pReset->arg3);
    DISPOSE(pReset);
    send_to_char( "Done.\n\r", ch );
    return;
  }
  if ( !str_cmp(arg, "place") )
  {
    if ( (pReset = parse_reset(pArea, argument, ch)) == NULL )
    {
      send_to_char( "Error in reset.  Reset not added.\n\r", ch );
      return;
    }
    place_reset(pArea, pReset->command, pReset->extra, pReset->arg1,
        pReset->arg2, pReset->arg3);
    DISPOSE(pReset);
    send_to_char( "Done.\n\r", ch );
    return;
  }
  if ( !str_cmp(arg, "insert") )
  {
    argument = one_argument(argument, arg);
    if ( !*arg || !is_number(arg) )
    {
      send_to_char( "Usage: reset insert <number> <command>\n\r", ch );
      return;
    }
    num = atoi(arg);
    if ( (reset = find_reset(pArea, aRoom, num)) == NULL )
    {
      send_to_char( "Reset not found.\n\r", ch );
      return;
    }
    if ( (pReset = parse_reset(pArea, argument, ch)) == NULL )
    {
      send_to_char( "Error in reset.  Reset not inserted.\n\r", ch );
      return;
    }
    INSERT(pReset, reset, pArea->first_reset, next, prev);
    send_to_char( "Done.\n\r", ch );
    return;
  }
  if ( !str_cmp(arg, "delete") )
  {
    int start, end;
    bool found;
    
    if ( !*argument )
    {
      send_to_char( "Usage: reset delete <start> [end]\n\r", ch );
      return;
    }
    argument = one_argument(argument, arg);
    start = is_number(arg) ? atoi(arg) : -1;
    end   = is_number(arg) ? atoi(arg) : -1;
    num = 0; found = FALSE;
    for ( pReset = pArea->first_reset; pReset; pReset = reset )
    {
      reset = pReset->next;
      if ( !is_room_reset(pReset, aRoom, pArea) )
        continue;
      if ( start > ++num )
        continue;
      if ( (end != -1 && num > end) || (end == -1 && found) )
        return;
      UNLINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
      if ( pReset == pArea->last_mob_reset )
        pArea->last_mob_reset = NULL;
      DISPOSE(pReset);
      top_reset--;
      found = TRUE;
    }
    if ( !found )
      send_to_char( "Reset not found.\n\r", ch );
    else
      send_to_char( "Done.\n\r", ch );
    return;
  }
  
  if ( !str_cmp(arg, "remove") )
  {
    int iarg;
    
    argument = one_argument(argument, arg);
    if ( arg[0] == '\0' || !is_number(arg) )
    {
      send_to_char( "Delete which reset?\n\r", ch );
      return;
    }
    iarg = atoi(arg);
    for ( pReset = pArea->first_reset; pReset; pReset = pReset->next )
    {
      if ( is_room_reset( pReset, aRoom, pArea ) && ++num == iarg )
        break;
    }
    if ( !pReset )
    {
      send_to_char( "Reset does not exist.\n\r", ch );
      return;
    }
    delete_reset( pArea, pReset );
    send_to_char( "Reset deleted.\n\r", ch );
    return;
  }
  if ( !str_prefix( arg, "mobile" ) )
  {
    argument = one_argument(argument, arg);
    if ( arg[0] == '\0' || !is_number(arg) )
    {
      send_to_char( "Reset which mobile vnum?\n\r", ch );
      return;
    }
    if ( !(pMob = get_mob_index(atoi(arg))) )
    {
      send_to_char( "Mobile does not exist.\n\r", ch );
      return;
    }
    argument = one_argument(argument, arg);
    if ( arg[0] == '\0' )
      num = 1;
    else if ( !is_number(arg) )
    {
      send_to_char( "Reset how many mobiles?\n\r", ch );
      return;
    }
    else
      num = atoi(arg);
    if ( !(pRoom = find_room(ch, argument, aRoom)) )
      return;
    pReset = make_reset('M', 0, pMob->vnum, num, pRoom->vnum);
    LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
    send_to_char( "Mobile reset added.\n\r", ch );
    return;
  }
  if ( !str_prefix(arg, "object") )
  {
    argument = one_argument(argument, arg);
    if ( arg[0] == '\0' || !is_number(arg) )
    {
      send_to_char( "Reset which object vnum?\n\r", ch );
      return;
    }
    if ( !(pObj = get_obj_index(atoi(arg))) )
    {
      send_to_char( "Object does not exist.\n\r", ch );
      return;
    }
    argument = one_argument(argument, arg);
    if ( arg[0] == '\0' )
      strcpy(arg, "room");
    if ( !str_prefix( arg, "put" ) )
    {
      argument = one_argument(argument, arg);
      if ( !(reset = find_oreset(ch, pArea, aRoom, arg)) )
        return;
      pReset = reset;
      /* Put in_objects after hide and trap resets */
      while ( reset->next && (reset->next->command == 'H' ||
              reset->next->command == 'T' ||
             (reset->next->command == 'B' &&
              (reset->next->arg2 & BIT_RESET_TYPE_MASK) == BIT_RESET_OBJECT &&
              (!reset->next->arg1 || reset->next->arg1 == pReset->arg1))) )
        reset = reset->next;
/*      pReset = make_reset('P', 1, pObj->vnum, num, reset->arg1);*/
      argument = one_argument(argument, arg);
      if ( (vnum = atoi(arg)) < 1 )
        vnum = 1;
      pReset = make_reset('P', reset->extra+1, pObj->vnum, vnum, 0);
      /* Grumble.. insert puts pReset before reset, and we need it after,
         so we make a hackup and reverse all the list params.. :P.. */
      INSERT(pReset, reset, pArea->last_reset, prev, next);
      send_to_char( "Object reset in object created.\n\r", ch );
      return;
    }
    if ( !str_prefix( arg, "give" ) )
    {
      argument = one_argument(argument, arg);
      if ( !(reset = find_mreset(ch, pArea, aRoom, arg)) )
        return;
      pReset = reset;
      while ( reset->next && reset->next->command == 'B' &&
      	     (reset->next->arg2 & BIT_RESET_TYPE_MASK) == BIT_RESET_OBJECT &&
      	     (!reset->next->arg1 || reset->next->arg1 == pReset->arg1) )
        reset = reset->next;
      argument = one_argument(argument, arg);
      if ( (vnum = atoi(arg)) < 1 )
        vnum = 1;
      pReset = make_reset('G', 1, pObj->vnum, vnum, 0);
      INSERT(pReset, reset, pArea->last_reset, prev, next);
      send_to_char( "Object reset to mobile created.\n\r", ch );
      return;
    }
    if ( !str_prefix( arg, "equip" ) )
    {
      argument = one_argument(argument, arg);
      if ( !(reset = find_mreset(ch, pArea, aRoom, arg)) )
        return;
      pReset = reset;
      while ( reset->next && reset->next->command == 'B' &&
      	     (reset->next->arg2 & BIT_RESET_TYPE_MASK) == BIT_RESET_OBJECT &&
      	     (!reset->next->arg1 || reset->next->arg1 == pReset->arg1) )
        reset = reset->next;
      num = get_wearloc(argument);
      if ( num < 0 )
      {
        send_to_char( "Reset object to which location?\n\r", ch );
        return;
      }
      for ( pReset = reset->next; pReset; pReset = pReset->next )
      {
        if ( pReset->command == 'M' )
          break;
        if ( pReset->command == 'E' && pReset->arg3 == num )
        {
          send_to_char( "Mobile already has an item equipped there.\n\r", ch);
          return;
        }
      }
      argument = one_argument(argument, arg);
      if ( (vnum = atoi(arg)) < 1 )
        vnum = 1;
      pReset = make_reset('E', 1, pObj->vnum, vnum, num);
      INSERT(pReset, reset, pArea->last_reset, prev, next);
      send_to_char( "Object reset equipped by mobile created.\n\r", ch );
      return;
    }
    if ( arg[0] == '\0' || !(num = (int)str_cmp(arg, "room")) ||
         is_number(arg) )
    {
      if ( !(bool)num )
        argument = one_argument(argument, arg);
      if ( !(pRoom = find_room(ch, argument, aRoom)) )
        return;
      if ( pRoom->area != pArea )
      {
        send_to_char( "Cannot reset objects to other areas.\n\r", ch );
        return;
      }
      if ( (vnum = atoi(arg)) < 1 )
        vnum = 1;
      pReset = make_reset('O', 0, pObj->vnum, vnum, pRoom->vnum);
      LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
      send_to_char( "Object reset added.\n\r", ch );
      return;
    }
    send_to_char( "Reset object to where?\n\r", ch );
    return;
  }
  if ( !str_cmp(arg, "random") )
  {
    argument = one_argument(argument, arg);
    vnum = get_dir( arg );
    if ( vnum < 0 || vnum > 9 )
    {
      send_to_char( "Reset which random doors?\n\r", ch );
      return;
    }
    if ( vnum == 0 )
    {
      send_to_char( "There is no point in randomizing one door.\n\r", ch );
      return;
    }
    pRoom = find_room(ch, argument, aRoom);
    if ( pRoom->area != pArea )
    {
      send_to_char( "Cannot randomize doors in other areas.\n\r", ch );
      return;
    }
    pReset = make_reset('R', 0, pRoom->vnum, vnum, 0);
    LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
    send_to_char( "Reset random doors created.\n\r", ch );
    return;
  }
  if ( !str_cmp(arg, "trap") )
  {
    char oname[MAX_INPUT_LENGTH];
    int chrg, value, extra = 0;
    bool isobj;
    
    argument = one_argument(argument, oname);
    argument = one_argument(argument, arg);
    num = is_number(arg) ? atoi(arg) : -1;
    argument = one_argument(argument, arg);
    chrg = is_number(arg) ? atoi(arg) : -1;
    isobj = is_name(argument, "obj");
    if ( isobj == is_name(argument, "room") )
    {
      send_to_char( "Reset: TRAP: Must specify ROOM or OBJECT\n\r", ch );
      return;
    }
    if ( !str_cmp(oname, "room") && !isobj )
    {
      vnum = (aRoom ? aRoom->vnum : ch->in_room->vnum);
      extra = TRAP_ROOM;
    }
    else
    {
      if ( is_number(oname) && !isobj )
      {
        vnum = atoi(oname);
        if ( !get_room_index(vnum) )
        {
          send_to_char( "Reset: TRAP: no such room\n\r", ch );
          return;
        }
        reset = NULL;
        extra = TRAP_ROOM;
      }
      else
      {
        if ( !(reset = find_oreset(ch, pArea, aRoom, oname)) )
          return;
/*        vnum = reset->arg1;*/
        vnum = 0;
        extra = TRAP_OBJ;
      }
    }
    if ( num < 1 || num > MAX_TRAPTYPE )
    {
      send_to_char( "Reset: TRAP: invalid trap type\n\r", ch );
      return;
    }
    if ( chrg < 0 || chrg > 10000 )
    {
      send_to_char( "Reset: TRAP: invalid trap charges\n\r", ch );
      return;
    }
    while ( *argument )
    {
      argument = one_argument(argument, arg);
      value = get_trapflag(arg);
      if ( value < 0 || value > 31 )
      {
        send_to_char( "Reset: TRAP: bad flag\n\r", ch );
        return;
      }
      SET_BIT(extra, 1 << value);
    }
    pReset = make_reset('T', extra, num, chrg, vnum);
    if ( reset )
      INSERT(pReset, reset, pArea->last_reset, prev, next);
    else
      LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
    send_to_char( "Trap created.\n\r", ch );
    return;
  }
  if ( !str_cmp(arg, "bit") )
  {
    int (*flfunc)(char *type);
    int flags = 0;
    char option[MAX_INPUT_LENGTH];
    char *parg;
    bool ext_bv = FALSE;
    
    argument = one_argument(argument, option);
    if ( !*option )
    {
      send_to_char( "You must specify SET, REMOVE, or TOGGLE.\n\r", ch );
      return;
    }
    num = 0;
    if ( !str_prefix(option, "set") )
      SET_BIT(num, BIT_RESET_SET);
    else if ( !str_prefix(option, "toggle") )
      SET_BIT(num, BIT_RESET_TOGGLE);
    else if ( str_prefix(option, "remove") )
    {
      send_to_char( "You must specify SET, REMOVE, or TOGGLE.\n\r", ch );
      return;
    }
    argument = one_argument(argument, option);
    parg = argument;
    argument = one_argument(argument, arg);
    if ( !*option )
    {
      send_to_char( "Must specify OBJECT, MOBILE, ROOM, or DOOR.\n\r", ch );
      return;
    }
    if ( !str_prefix(option, "door") )
    {
      SET_BIT(num, BIT_RESET_DOOR);
      if ( aRoom )
      {
        pRoom = aRoom;
        argument = parg;
      }
      else if ( !is_number(arg) )
      {
        pRoom = ch->in_room;
        argument = parg;
      }
      else if ( !(pRoom = find_room(ch, arg, aRoom)) )
        return;
      argument = one_argument(argument, arg);
      if ( !*arg )
      {
        send_to_char( "Must specify direction.\n\r", ch );
        return;
      }
      vnum = get_dir(arg);
      SET_BIT(num, vnum << BIT_RESET_DOOR_THRESHOLD);
      vnum = pRoom->vnum;
      flfunc = &get_exflag;
      reset = NULL;
    }
    else if ( !str_prefix(option, "object") )
    {
      SET_BIT(num, BIT_RESET_OBJECT);
      vnum = 0;
      flfunc = &get_oflag;
      if ( !(reset = find_oreset(ch, pArea, aRoom, arg)) )
        return;
      ext_bv = TRUE;
    }
    else if ( !str_prefix(option, "mobile") )
    {
      SET_BIT(num, BIT_RESET_MOBILE);
      vnum = 0;
      flfunc = &get_aflag;
      if ( !(reset = find_mreset(ch, pArea, aRoom, arg)) )
        return;
      ext_bv = TRUE;
    }
    else if ( !str_prefix(option, "room") )
    {
      SET_BIT(num, BIT_RESET_ROOM);
      if ( aRoom )
      {
        pRoom = aRoom;
        argument = parg;
      }
      else if ( !is_number(arg) )
      {
        pRoom = ch->in_room;
        argument = parg;
      }
      else if ( !(pRoom = find_room(ch, arg, aRoom)) )
        return;
      vnum = pRoom->vnum;
      flfunc = &get_rflag;
      reset = NULL;
    }
    else
    {
      send_to_char( "Must specify OBJECT, MOBILE, ROOM, or DOOR.\n\r", ch );
      return;
    }
    while ( *argument )
    {
      int value;
      argument = one_argument(argument, arg);
      value = (*flfunc)(arg);
      if ( value < 0 || (!ext_bv && value > 31) )
      {
        send_to_char( "Reset: BIT: bad flag\n\r", ch );
        return;
      }
      if (ext_bv)	/* one per flag for extendeds */
      {
        pReset = make_reset('B', 1, vnum, num, flags);
        if (reset)
        {
          INSERT(pReset, reset, pArea->last_reset, prev, next);
          reset = pReset;
        }
        else
          LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
      }
      else
        SET_BIT(flags, 1 << value);
    }
    if ( !flags )
    {
      send_to_char( "Set which flags?\n\r", ch );
      return;
    }
    if (!ext_bv)
    {
      pReset = make_reset('B', 1, vnum, num, flags);
      if ( reset )
        INSERT(pReset, reset, pArea->last_reset, prev, next);
      else
        LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
    }
    send_to_char( "Bitvector reset created.\n\r", ch );
    return;
  }
  if ( !str_cmp(arg, "hide") )
  {
    argument = one_argument(argument, arg);
    if ( !(reset = find_oreset(ch, pArea, aRoom, arg)) )
      return;
/*    pReset = make_reset('H', 1, reset->arg1, 0, 0);*/
    pReset = make_reset('H', 1, 0, 0, 0);
    INSERT(pReset, reset, pArea->last_reset, prev, next);
    send_to_char( "Object hide reset created.\n\r", ch );
    return;
  }
  if ( ch->substate == SUB_REPEATCMD )
  {
    ch->substate = SUB_NONE;
    interpret(ch, origarg);
    ch->substate = SUB_REPEATCMD;
    ch->last_cmd = (aRoom ? do_rreset : do_reset);
  }
  else
    edit_reset(ch, "", pArea, aRoom);
  return;
}

void do_reset( CHAR_DATA *ch, char *argument )
{
  AREA_DATA *pArea = NULL;
  char arg[MAX_INPUT_LENGTH];
  char *parg;
  
  /*
   * Can't have NPC's doing this.  Bug report sent in by Cronel
   * -- Shaddai
   */
  if ( IS_NPC( ch ) )	return;

  parg = one_argument(argument, arg);
  if ( ch->substate == SUB_REPEATCMD )
  {
    pArea = ch->dest_buf;
    if ( pArea && pArea != ch->pcdata->area && pArea != ch->in_room->area )
    {
      AREA_DATA *tmp;
      
      for ( tmp = first_build; tmp; tmp = tmp->next )
        if ( tmp == pArea )
          break;
      if ( !tmp )
        for ( tmp = first_area; tmp; tmp = tmp->next )
          if ( tmp == pArea )
            break;
      if ( !tmp )
      {
        send_to_char("Your area pointer got lost.  Reset mode off.\n\r", ch);
        bug("do_reset: %s's dest_buf points to invalid area",
		ch->name);	/* why was this cast to an int? */
        ch->substate = SUB_NONE;
        ch->dest_buf = NULL;
        return;
      }
    }
    if ( !*arg )
    {
      ch_printf(ch, "Editing resets for area: %s\n\r", pArea->name);
      return;
    }
    if ( !str_cmp(arg, "done") || !str_cmp(arg, "off") )
    {
      send_to_char( "Reset mode off.\n\r", ch );
      ch->substate = SUB_NONE;
      ch->dest_buf = NULL;
      return;
    }
  }
  if ( !pArea && get_trust(ch) > LEVEL_GOD )
  {
    char fname[80];
    
    sprintf(fname, "%s.are", capitalize(arg));
    for ( pArea = first_build; pArea; pArea = pArea->next )
      if ( !str_cmp(fname, pArea->filename) )
      {
        argument = parg;
        break;
      }
    if ( !pArea )
      pArea = ch->pcdata->area;
    if ( !pArea )
      pArea = ch->in_room->area;
  }
  else
    pArea = ch->pcdata->area;
  if ( !pArea )
  {
    send_to_char( "You do not have an assigned area.\n\r", ch );
    return;
  }
  edit_reset(ch, argument, pArea, NULL);
  return;
}

void do_rreset( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA *pRoom;
  
  if ( ch->substate == SUB_REPEATCMD )
  {
    pRoom = ch->dest_buf;
    if ( !pRoom )
    {
      send_to_char( "Your room pointer got lost.  Reset mode off.\n\r", ch);
      bug("do_rreset: %s's dest_buf points to invalid room", (int)ch->name);
    }
    ch->substate = SUB_NONE;
    ch->dest_buf = NULL;
    return;
  }
  else
    pRoom = ch->in_room;
  if ( !can_rmodify(ch, pRoom) )
    return;
  edit_reset(ch, argument, pRoom->area, pRoom);
  return;
}

void add_obj_reset( AREA_DATA *pArea, char cm, OBJ_DATA *obj, int v2, int v3 )
{
  OBJ_DATA *inobj;
  static int iNest;
  
  if ( (cm == 'O' || cm == 'P') && obj->pIndexData->vnum == OBJ_VNUM_TRAP )
  {
    if ( cm == 'O' )
      add_reset(pArea, 'T', obj->value[3], obj->value[1], obj->value[0], v3);
    return;
  }
  add_reset( pArea, cm, (cm == 'P' ? iNest : 1), obj->pIndexData->vnum,
  	     v2, v3 );
  /* Only add hide for in-room objects that are hidden and cant be moved, as
     hide is an update reset, not a load-only reset. */
  if ( cm == 'O' && IS_OBJ_STAT(obj, ITEM_HIDDEN) &&
      !IS_SET(obj->wear_flags, ITEM_TAKE) )
    add_reset(pArea, 'H', 1, 0, 0, 0);
  for ( inobj = obj->first_content; inobj; inobj = inobj->next_content )
    if ( inobj->pIndexData->vnum == OBJ_VNUM_TRAP )
      add_obj_reset(pArea, 'O', inobj, 0, 0);
  if ( cm == 'P' )
    iNest++;
  for ( inobj = obj->first_content; inobj; inobj = inobj->next_content )
    add_obj_reset( pArea, 'P', inobj, 1, 0 );
  if ( cm == 'P' )
    iNest--;
  return;
}

void instaroom( AREA_DATA *pArea, ROOM_INDEX_DATA *pRoom, bool dodoors )
{
  CHAR_DATA *rch;
  OBJ_DATA *obj;
  
  for ( rch = pRoom->first_person; rch; rch = rch->next_in_room )
  {
    if ( !IS_NPC(rch) )
      continue;
    add_reset( pArea, 'M', 1, rch->pIndexData->vnum, rch->pIndexData->count,
               pRoom->vnum );
    for ( obj = rch->first_carrying; obj; obj = obj->next_content )
    {
      if ( obj->wear_loc == WEAR_NONE )
        add_obj_reset( pArea, 'G', obj, 1, 0 );
      else
        add_obj_reset( pArea, 'E', obj, 1, obj->wear_loc );
    }
  }
  for ( obj = pRoom->first_content; obj; obj = obj->next_content )
  {
    add_obj_reset( pArea, 'O', obj, 1, pRoom->vnum );
  }
  if ( dodoors )
  {
    EXIT_DATA *pexit;

    for ( pexit = pRoom->first_exit; pexit; pexit = pexit->next )
    {
      int state = 0;
    
      if ( !IS_SET( pexit->exit_info, EX_ISDOOR ) )
        continue;
      if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
      {
        if ( IS_SET( pexit->exit_info, EX_LOCKED ) )
          state = 2;
        else
          state = 1;
      }
      add_reset( pArea, 'D', 0, pRoom->vnum, pexit->vdir, state );
    }
  }
  return;
}

void wipe_resets( AREA_DATA *pArea, ROOM_INDEX_DATA *pRoom )
{
  RESET_DATA *pReset;
  
  for ( pReset = pArea->first_reset; pReset; )
  {
    if ( pReset->command != 'R' && is_room_reset( pReset, pRoom, pArea ) )
    {
      /* Resets always go forward, so we can safely use the previous reset,
         providing it exists, or first_reset if it doesnt.  -- Altrag */
      RESET_DATA *prev = pReset->prev;
      
      delete_reset(pArea, pReset);
      pReset = (prev ? prev->next : pArea->first_reset);
    }
    else
      pReset = pReset->next;
  }
  return;
}

void do_instaroom( CHAR_DATA *ch, char *argument )
{
  AREA_DATA *pArea;
  ROOM_INDEX_DATA *pRoom;
  bool dodoors;
  char arg[MAX_INPUT_LENGTH];

  if ( IS_NPC(ch) || get_trust(ch) < LEVEL_SAVIOR || !ch->pcdata ||
      !ch->pcdata->area )
  {
    send_to_char( "You don't have an assigned area to create resets for.\n\r",
        ch );
    return;
  }
  argument = one_argument(argument, arg);
  if ( !str_cmp(argument, "nodoors") )
    dodoors = FALSE;
  else
    dodoors = TRUE;
  pArea = ch->pcdata->area;
  if ( !(pRoom = find_room(ch, arg, NULL)) )
  {
    send_to_char( "Room doesn't exist.\n\r", ch );
    return;
  }
  if ( !can_rmodify(ch, pRoom) )
    return;
  if ( pRoom->area != pArea && get_trust(ch) < LEVEL_GREATER )
  {
    send_to_char( "You cannot reset that room.\n\r", ch );
    return;
  }
  if ( pArea->first_reset )
    wipe_resets(pArea, pRoom);
  instaroom(pArea, pRoom, dodoors);
  send_to_char( "Room resets installed.\n\r", ch );
}

void do_instazone( CHAR_DATA *ch, char *argument )
{
  AREA_DATA *pArea;
  int vnum;
  ROOM_INDEX_DATA *pRoom;
  bool dodoors;

  if ( IS_NPC(ch) || get_trust(ch) < LEVEL_SAVIOR || !ch->pcdata ||
      !ch->pcdata->area )
  {
    send_to_char( "You don't have an assigned area to create resets for.\n\r",
        ch );
    return;
  }
  if ( !str_cmp(argument, "nodoors") )
    dodoors = FALSE;
  else
    dodoors = TRUE;
  pArea = ch->pcdata->area;
  if ( pArea->first_reset )
    wipe_resets(pArea, NULL);
  for ( vnum = pArea->low_r_vnum; vnum <= pArea->hi_r_vnum; vnum++ )
  {
    if ( !(pRoom = get_room_index(vnum)) || pRoom->area != pArea )
      continue;
    instaroom( pArea, pRoom, dodoors );
  }
  send_to_char( "Area resets installed.\n\r", ch );
  return;
}

int generate_itemlevel( AREA_DATA *pArea, OBJ_INDEX_DATA *pObjIndex )
{
    int olevel;
    int min = UMAX(pArea->low_soft_range, 1);
    int max = UMIN(pArea->hi_soft_range, min + 15);

    if ( pObjIndex->level > 0 )
	olevel = UMIN(pObjIndex->level, MAX_LEVEL);
    else
	switch ( pObjIndex->item_type )
	{
	    default:		olevel = 0;				break;
	    case ITEM_PILL:	olevel = number_range(  min, max );	break;
	    case ITEM_POTION:	olevel = number_range(  min, max );	break;
	    case ITEM_SCROLL:	olevel = pObjIndex->value[0];		break;
	    case ITEM_WAND:	olevel = number_range( min+4, max+1 );	break;
	    case ITEM_STAFF:	olevel = number_range( min+9, max+5 );	break;
	    case ITEM_ARMOR:	olevel = number_range( min+4, max+1 );	break;
	    case ITEM_WEAPON:	olevel = number_range( min+4, max+1 );	break;
	}
    return olevel;
}

/*
 * Reset one area.
 */
void reset_area( AREA_DATA *pArea )
{
  RESET_DATA *pReset;
  CHAR_DATA *mob;
  OBJ_DATA *obj;
  OBJ_DATA *lastobj;
  ROOM_INDEX_DATA *pRoomIndex;
  MOB_INDEX_DATA *pMobIndex;
  OBJ_INDEX_DATA *pObjIndex;
  OBJ_INDEX_DATA *pObjToIndex;
  EXIT_DATA *pexit;
  OBJ_DATA *to_obj;
  char buf[MAX_STRING_LENGTH];
  int level = 0;
  void *plc = NULL;
  bool ext_bv = FALSE;
  
  if ( !pArea )
  {
    bug( "reset_area: NULL pArea", 0 );
    return;
  }
  
  mob = NULL;
  obj = NULL;
  lastobj = NULL;
  if ( !pArea->first_reset )
  {
    bug( "%s: reset_area: no resets", (int)pArea->filename );
    return;
  }
  level = 0;
  for ( pReset = pArea->first_reset; pReset; pReset = pReset->next )
  {
    switch( pReset->command )
    {
    default:
      sprintf( buf, "%s Reset_area: bad command %c.", pArea->filename,
	pReset->command );
      bug ( buf, 0 );
      break;
    
    case 'M':
      if ( !(pMobIndex = get_mob_index(pReset->arg1)) )
      {
	sprintf( buf, "%s Reset_area: 'M': bad mob vnum %d.", pArea->filename,
	  pReset->arg1 );
	bug( buf, 0 );
        continue;
      }
      if ( !(pRoomIndex = get_room_index(pReset->arg3)) )
      {
	sprintf( buf, "%s Reset_area: 'M': bad room vnum %d.", pArea->filename,
	  pReset->arg3 );
	bug ( buf, 0 );
        continue;
      }
      if ( pMobIndex->count >= pReset->arg2 )
      {
        mob = NULL;
        break;
      }
      mob = create_mobile(pMobIndex);
      {
	ROOM_INDEX_DATA *pRoomPrev = get_room_index(pReset->arg3 - 1);
        
	if ( pRoomPrev && xIS_SET(pRoomPrev->room_flags, ROOM_PET_SHOP) )
	  xSET_BIT(mob->act, ACT_PET);
      }
      if ( room_is_dark(pRoomIndex) )
	  xSET_BIT(mob->affected_by, AFF_INFRARED);
      char_to_room(mob, pRoomIndex);
      economize_mobgold(mob);
      level = URANGE(0, mob->level - 2, LEVEL_AVATAR);
      if ( HAS_PROG(mob->pIndexData, LOAD_PROG) )
	mprog_percent_check( mob, NULL, NULL, NULL, NULL, LOAD_PROG );
      break;
    
    case 'G':
    case 'E':
      if ( !(pObjIndex = get_obj_index(pReset->arg1)) )
      {
	sprintf (buf, "%s Reset_area: 'E' or 'G': bad obj vnum %d.", 
	   pArea->filename, pReset->arg1 );
	bug ( buf, 0 );
        continue;
      }
      if ( !mob )
      {
        lastobj = NULL;
        break;
      }
      if ( mob->pIndexData->pShop )
      {
	int olevel =  UMIN(generate_itemlevel( pArea, pObjIndex ),level);
	obj = create_object(pObjIndex, olevel);
	xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
      }
      else
      obj = create_object(pObjIndex, number_fuzzy(level));
      obj->level = URANGE(0, obj->level, LEVEL_AVATAR);
      obj = obj_to_char(obj, mob);
      if ( pReset->command == 'E' )
        equip_char(mob, obj, pReset->arg3);
      lastobj = obj;
      break;
    
    case 'O':
      if ( !(pObjIndex = get_obj_index(pReset->arg1)) )
      {
/*
	sprintf (buf, "%s Reset_area: 'O': bad obj vnum %d.",
		pArea->filename, pReset->arg1 );
	bug ( buf, 0 );
*/
        continue;
      }
      if ( !(pRoomIndex = get_room_index(pReset->arg3)) )
      {
/*
	sprintf ( buf, "%s Reset_area: 'O': bad room vnum %d.", pArea->filename,
	   pReset->arg3 );
	bug ( buf, 0 );
*/
        continue;
      }
      if ( pArea->nplayer > 0 ||
           count_obj_list(pObjIndex, pRoomIndex->first_content) > 0 )
      {
        obj = NULL;
        lastobj = NULL;
        break;
      }
      obj = create_object(pObjIndex, number_fuzzy(generate_itemlevel(pArea, pObjIndex)));
      obj->level = UMIN(obj->level, LEVEL_AVATAR);
      obj->cost = 0;
      obj_to_room(obj, pRoomIndex);
      lastobj = obj;
      break;
    
    case 'P':
      if ( !(pObjIndex = get_obj_index(pReset->arg1)) )
      {
/*
	sprintf ( buf, "%s Reset_area: 'P': bad obj vnum %d.", pArea->filename,
	   pReset->arg1 );
	bug ( buf, 0 );
*/
        continue;
      }
      if ( pReset->arg3 > 0 )
      {
        if ( !(pObjToIndex = get_obj_index(pReset->arg3)) )
        {
/*
	  sprintf(buf,"%s Reset_area: 'P': bad objto vnum %d.",pArea->filename,
		pReset->arg3 );
	  bug( buf, 0 );
*/
          continue;
        }
        if ( pArea->nplayer > 0 ||
           !(to_obj = get_obj_type(pObjToIndex)) ||
            !to_obj->in_room ||
             count_obj_list(pObjIndex, to_obj->first_content) > 0 )
        {
          obj = NULL;
          break;
        }
        lastobj = to_obj;
      }
      else
      {
        int iNest;
        
        if ( !lastobj )
          break;
        to_obj = lastobj;
        for ( iNest = 0; iNest < pReset->extra; iNest++ )
          if ( !(to_obj = to_obj->last_content) )
          {
/*
	    sprintf(buf,"%s Reset_area: 'P': Invalid nesting obj %d."
		,pArea->filename, pReset->arg1 );
	    bug( buf, 0 );
*/
            iNest = -1;
            break;
          }
        if ( iNest < 0 )
          continue;
      }
      obj = create_object(pObjIndex, number_fuzzy(UMAX(generate_itemlevel(pArea, pObjIndex),to_obj->level)));
      obj->level = UMIN(obj->level, LEVEL_AVATAR);
      obj_to_obj(obj, to_obj);
      break;
    
    case 'T':
      if ( IS_SET(pReset->extra, TRAP_OBJ) )
      {
        /* We need to preserve obj for future 'T' and 'H' checks */
        OBJ_DATA *pobj;
        
        if ( pReset->arg3 > 0 )
        {
          if ( !(pObjToIndex = get_obj_index(pReset->arg3)) )
          {
/*
	    sprintf (buf,"%s Reset_area: 'T': bad objto vnum %d."
		,pArea->filename, pReset->arg3 );
	    bug ( buf, 0 );
*/
            continue;
          }
          if ( pArea->nplayer > 0 ||
             !(to_obj = get_obj_type(pObjToIndex)) ||
              (to_obj->carried_by && !IS_NPC(to_obj->carried_by)) ||
               is_trapped(to_obj) )
            break;
        }
        else
        {
          if ( !lastobj || !obj )
            break;
          to_obj = obj;
        }
        pobj = make_trap( pReset->arg2, pReset->arg1,
        		  number_fuzzy(to_obj->level), pReset->extra );
        obj_to_obj(pobj, to_obj);
      }
      else
      {
        if ( !(pRoomIndex = get_room_index(pReset->arg3)) )
        {
/*
	  sprintf(buf,"%s Reset_area: 'T': bad room %d.", pArea->filename,
		pReset->arg3 );
	  bug( buf, 0 );
*/
          continue;
        }
        if ( pArea->nplayer > 0 ||
             count_obj_list(get_obj_index(OBJ_VNUM_TRAP),
               pRoomIndex->first_content) > 0 )
          break;
        to_obj = make_trap(pReset->arg2, pReset->arg1, 10, pReset->extra);
        obj_to_room(to_obj, pRoomIndex);
      }
      break;
    
    case 'H':
      if ( pReset->arg1 > 0 )
      {
        if ( !(pObjToIndex = get_obj_index(pReset->arg1)) )
        {
/*
	  sprintf(buf,"%s Reset_area: 'H': bad objto vnum %d.",pArea->filename,
		pReset->arg1 );
	  bug( buf, 0 );
*/
          continue;
        }
        if ( pArea->nplayer > 0 ||
           !(to_obj = get_obj_type(pObjToIndex)) ||
            !to_obj->in_room ||
             to_obj->in_room->area != pArea ||
             IS_OBJ_STAT(to_obj, ITEM_HIDDEN) )
          break;
      }
      else
      {
        if ( !lastobj || !obj )
          break;
        to_obj = obj;
      }
      xSET_BIT(to_obj->extra_flags, ITEM_HIDDEN);
      break;
    
    case 'B':
      switch(pReset->arg2 & BIT_RESET_TYPE_MASK)
      {
      case BIT_RESET_DOOR:
        {
        int doornum;
        
        if ( !(pRoomIndex = get_room_index(pReset->arg1)) )
        {
/*
	  sprintf(buf,"%s Reset_area: 'B': door: bad room vnum %d.",
		pArea->filename, pReset->arg1 );
	  bug( buf, 0 );
*/
          continue;
        }
        doornum = (pReset->arg2 & BIT_RESET_DOOR_MASK)
                >> BIT_RESET_DOOR_THRESHOLD;
        if ( !(pexit = get_exit(pRoomIndex, doornum)) )
          break;
        plc = &pexit->exit_info;
        }
        break;
      case BIT_RESET_ROOM:
        if ( !(pRoomIndex = get_room_index(pReset->arg1)) )
        {
/*
	  sprintf(buf,"%s Reset_area: 'B': room: bad room vnum %d.",
		pArea->filename, pReset->arg1 );
	  bug(buf, 0);
*/
          continue;
        }
        plc = &pRoomIndex->room_flags;
        break;
      case BIT_RESET_OBJECT:
        if ( pReset->arg1 > 0 )
        {
          if ( !(pObjToIndex = get_obj_index(pReset->arg1)) )
          {
/*
	    sprintf(buf,"%s Reset_area: 'B': object: bad objto vnum %d.",
		pArea->filename, pReset->arg1 );
	    bug( buf, 0 );
*/
            continue;
          }
          if ( !(to_obj = get_obj_type(pObjToIndex)) ||
                !to_obj->in_room ||
                 to_obj->in_room->area != pArea )
            continue;
        }
        else
        {
          if ( !lastobj || !obj )
            continue;
          to_obj = obj;
        }
        plc = &to_obj->extra_flags;
	ext_bv = TRUE;
        break;
      case BIT_RESET_MOBILE:
        if ( !mob )
          continue;
        plc = &mob->affected_by;
	ext_bv = TRUE;
        break;
      default:
/*
	sprintf(buf, "%s Reset_area: 'B': bad options %d.",
		pArea->filename, pReset->arg2 );
	bug( buf, 0 );
*/
        continue;
      }
      if ( IS_SET(pReset->arg2, BIT_RESET_SET) && plc != NULL)
      {
	if ( ext_bv )
	    xSET_BIT(*(EXT_BV *)plc, pReset->arg3);
	else
	    SET_BIT(*(int *)plc, pReset->arg3);
      }
      else if ( IS_SET(pReset->arg2, BIT_RESET_TOGGLE) )
      {
	if ( ext_bv )
	    xTOGGLE_BIT(*(EXT_BV *)plc, pReset->arg3);
	else
	    TOGGLE_BIT(*(int *)plc, pReset->arg3);
      }
      else
      {
	if ( ext_bv )
	    xREMOVE_BIT(*(EXT_BV *)plc, pReset->arg3);
	else
	    REMOVE_BIT(*(int *)plc, pReset->arg3);
      }
      break;
    
    case 'D':
      if ( !(pRoomIndex = get_room_index(pReset->arg1)) )
      {
/*
	sprintf(buf, "%s Reset_area: 'D': bad room vnum %d.",
		pArea->filename, pReset->arg1 );
	bug(buf, 0);
*/
        continue;
      }
      if ( !(pexit = get_exit(pRoomIndex, pReset->arg2)) )
        break;
      switch( pReset->arg3 )
      {
      case 0:
        REMOVE_BIT(pexit->exit_info, EX_CLOSED);
        REMOVE_BIT(pexit->exit_info, EX_LOCKED);
        break;
      case 1:
        SET_BIT(   pexit->exit_info, EX_CLOSED);
        REMOVE_BIT(pexit->exit_info, EX_LOCKED);
        if ( IS_SET(pexit->exit_info, EX_xSEARCHABLE) )
          SET_BIT( pexit->exit_info, EX_SECRET);
        break;
      case 2:
        SET_BIT(   pexit->exit_info, EX_CLOSED);
        SET_BIT(   pexit->exit_info, EX_LOCKED);
        if ( IS_SET(pexit->exit_info, EX_xSEARCHABLE) )
          SET_BIT( pexit->exit_info, EX_SECRET);
        break;
      }
      break;
    
    case 'R':
      if ( !(pRoomIndex = get_room_index(pReset->arg1)) )
      {
/*
	sprintf(buf,"%s Reset_area: 'R': bad room vnum %d.",
		pArea->filename, pReset->arg1 );
	bug(buf, 0);
*/
        continue;
      }
      randomize_exits(pRoomIndex, pReset->arg2-1);
      break;
    }
  }
  return;
}

void list_resets( CHAR_DATA *ch, AREA_DATA *pArea, ROOM_INDEX_DATA *pRoom,
		  int start, int end, char extraarg )
{
  RESET_DATA *pReset;
  ROOM_INDEX_DATA *room;
  MOB_INDEX_DATA *mob;
  OBJ_INDEX_DATA *obj, *obj2;
  OBJ_INDEX_DATA *lastobj;
  RESET_DATA *lo_reset;
  bool found;
  int num = 0;
  const char *rname = "???", *mname = "???", *oname = "???";
  char buf[256];
  char *pbuf;
  bool show = FALSE;
  
  if ( !ch || !pArea )
    return;
  room = NULL;
  mob = NULL;
  obj = NULL;
  lastobj = NULL;
  lo_reset = NULL;
  found = FALSE;
  
  for ( pReset = pArea->first_reset; pReset; pReset = pReset->next )
  {
	  show = FALSE;
    if ( !is_room_reset(pReset, pRoom, pArea) )
      continue;
    ++num;
    if ( !extraarg || extraarg == '\0' || extraarg == pReset->command)
      show = TRUE;
    else /*Check for 'A', which displays all objects (E, G, O, and P)*/
    {
      if ( extraarg == 'A' &&
      ( pReset->command == 'E' ||
        pReset->command == 'G' ||
    	pReset->command == 'O' ||
    	pReset->command == 'P') )
   		show = TRUE;
 	}

    sprintf(buf, "%3d) ", num);
    pbuf = buf+strlen(buf);
    switch( pReset->command )
    {
    default:
      sprintf(pbuf, "*** BAD RESET: %c %d %d %d %d ***\n\r",
          pReset->command, pReset->extra, pReset->arg1, pReset->arg2,
          pReset->arg3);
      break;
    case 'M':
      if ( !(mob = get_mob_index(pReset->arg1)) )
        mname = "Mobile: *BAD VNUM*";
      else
        mname = mob->player_name;
      if ( !(room = get_room_index(pReset->arg3)) )
        rname = "Room: *BAD VNUM*";
      else
        rname = room->name;
      sprintf( pbuf, "%s (%d) -> %s (%d) [%d]", mname, pReset->arg1, rname,
          pReset->arg3, pReset->arg2 );
      if ( !room )
        mob = NULL;
      if ( (room = get_room_index(pReset->arg3-1)) &&
            xIS_SET(room->room_flags, ROOM_PET_SHOP) )
        strcat( buf, " (pet)\n\r" );
      else
        strcat( buf, "\n\r" );
      break;
    case 'G':
    case 'E':
      if ( !mob )
        mname = "* ERROR: NO MOBILE! *";
      if ( !(obj = get_obj_index(pReset->arg1)) )
        oname = "Object: *BAD VNUM*";
      else
        oname = obj->name;
      sprintf( pbuf, "%s (%d) -> %s (%s) [%d]", oname, pReset->arg1, mname,
          (pReset->command == 'G' ? "carry" : wear_locs[pReset->arg3]),
          pReset->arg2 );
      if ( mob && mob->pShop )
        strcat( buf, " (shop)\n\r" );
      else
        strcat( buf, "\n\r" );
      lastobj = obj;
      lo_reset = pReset;
      break;
    case 'O':
      if ( !(obj = get_obj_index(pReset->arg1)) )
        oname = "Object: *BAD VNUM*";
      else
        oname = obj->name;
      if ( !(room = get_room_index(pReset->arg3)) )
        rname = "Room: *BAD VNUM*";
      else
        rname = room->name;
      sprintf( pbuf, "(object) %s (%d) -> %s (%d) [%d]\n\r", oname,
          pReset->arg1, rname, pReset->arg3, pReset->arg2 );
      if ( !room )
        obj = NULL;
      lastobj = obj;
      lo_reset = pReset;
      break;
    case 'P':
      if ( !(obj = get_obj_index(pReset->arg1)) )
        oname = "Object1: *BAD VNUM*";
      else
        oname = obj->name;
      obj2 = NULL;
      if ( pReset->arg3 > 0 )
      {
        obj2 = get_obj_index(pReset->arg3);
        rname = (obj2 ? obj2->name : "Object2: *BAD VNUM*");
        lastobj = obj2;
      }
      else if ( !lastobj )
        rname = "Object2: *NULL obj*";
      else if ( pReset->extra == 0 )
      {
        rname = lastobj->name;
        obj2 = lastobj;
      }
      else
      {
        int iNest;
        RESET_DATA *reset;
        
        reset = lo_reset->next;
        for ( iNest = 0; iNest < pReset->extra; iNest++ )
        {
          for ( ; reset; reset = reset->next )
            if ( reset->command == 'O' || reset->command == 'G' ||
                 reset->command == 'E' || (reset->command == 'P' &&
                !reset->arg3 && reset->extra == iNest) )
              break;
          if ( !reset || reset->command != 'P' )
            break;
        }
        if ( !reset )
          rname = "Object2: *BAD NESTING*";
        else if ( !(obj2 = get_obj_index(reset->arg1)) )
          rname = "Object2: *NESTED BAD VNUM*";
        else
          rname = obj2->name;
      }
      sprintf( pbuf, "(Put) %s (%d) -> %s (%d) [%d] {nest %d}\n\r", oname,
          pReset->arg1, rname, (obj2 ? obj2->vnum : pReset->arg3),
          pReset->arg2, pReset->extra );
      break;
    case 'T':
      sprintf(pbuf, "TRAP: %d %d %d %d (%s)\n\r", pReset->extra, pReset->arg1,
          pReset->arg2, pReset->arg3, flag_string(pReset->extra, trap_flags));
      break;
    case 'H':
      if ( pReset->arg1 > 0 )
        if ( !(obj2 = get_obj_index(pReset->arg1)) )
          rname = "Object: *BAD VNUM*";
        else
          rname = obj2->name;
      else if ( !obj )
        rname = "Object: *NULL obj*";
      else
        rname = oname;
      sprintf(pbuf, "Hide %s (%d)\n\r", rname,
          (pReset->arg1 > 0 ? pReset->arg1 : obj ? obj->vnum : 0));
      break;
    case 'B':
      {
      char * const *flagarray;
      bool ext_bv = FALSE;
      
      strcpy(pbuf, "BIT: ");
      pbuf += 5;
      if ( IS_SET(pReset->arg2, BIT_RESET_SET) )
      {
        strcpy(pbuf, "Set: ");
        pbuf += 5;
      }
      else if ( IS_SET(pReset->arg2, BIT_RESET_TOGGLE) )
      {
        strcpy(pbuf, "Toggle: ");
        pbuf += 8;
      }
      else
      {
        strcpy(pbuf, "Remove: ");
        pbuf += 8;
      }
      switch(pReset->arg2 & BIT_RESET_TYPE_MASK)
      {
      case BIT_RESET_DOOR:
        {
        int door;
        
        if ( !(room = get_room_index(pReset->arg1)) )
          rname = "Room: *BAD VNUM*";
        else
          rname = room->name;
        door = (pReset->arg2 & BIT_RESET_DOOR_MASK)
             >> BIT_RESET_DOOR_THRESHOLD;
        door = URANGE(0, door, MAX_DIR+1);
        sprintf(pbuf, "Exit %s%s (%d), Room %s (%d)", dir_name[door],
            (room && get_exit(room, door) ? "" : " (NO EXIT!)"), door,
            rname, pReset->arg1);
        }
        flagarray = ex_flags;
        break;
      case BIT_RESET_ROOM:
        if ( !(room = get_room_index(pReset->arg1)) )
          rname = "Room: *BAD VNUM*";
        else
          rname = room->name;
        sprintf(pbuf, "Room %s (%d)", rname, pReset->arg1);
        flagarray = r_flags;
        break;
      case BIT_RESET_OBJECT:
        if ( pReset->arg1 > 0 )
          if ( !(obj2 = get_obj_index(pReset->arg1)) )
            rname = "Object: *BAD VNUM*";
          else
            rname = obj2->name;
        else if ( !obj )
          rname = "Object: *NULL obj*";
        else
          rname = oname;
        sprintf(pbuf, "Object %s (%d)", rname,
            (pReset->arg1 > 0 ? pReset->arg1 : obj ? obj->vnum : 0));
        flagarray = o_flags;
        ext_bv = TRUE;
        break;
      case BIT_RESET_MOBILE:
        if ( pReset->arg1 > 0 )
        {
          MOB_INDEX_DATA *mob2;
          
          if ( !(mob2 = get_mob_index(pReset->arg1)) )
            rname = "Mobile: *BAD VNUM*";
          else
            rname = mob2->player_name;
        }
        else if ( !mob )
          rname = "Mobile: *NULL mob*";
        else
          rname = mname;
        sprintf(pbuf, "Mobile %s (%d)", rname,
            (pReset->arg1 > 0 ? pReset->arg1 : mob ? mob->vnum : 0));
        flagarray = a_flags;
        ext_bv = TRUE;
        break;
      default:
        sprintf(pbuf, "bad type %d", pReset->arg2 & BIT_RESET_TYPE_MASK);
        flagarray = NULL;
        break;
      }
      pbuf += strlen(pbuf);
      if ( flagarray )
      {
        if (ext_bv)
        {
          EXT_BV tmp;
          
          tmp = meb(pReset->arg3);
          sprintf(pbuf, "; flags: %s [%d]\n\r",
          	ext_flag_string(&tmp, flagarray), pReset->arg3);
        }
        else
          sprintf(pbuf, "; flags: %s [%d]\n\r",
          	flag_string(pReset->arg3, flagarray), pReset->arg3);
      }
      else
        sprintf(pbuf, "; flags %d\n\r", pReset->arg3);
      }
      break;
    case 'D':
      {
      char *ef_name;
      
      pReset->arg2 = URANGE(0, pReset->arg2, MAX_DIR+1);
      if ( !(room = get_room_index(pReset->arg1)) )
        rname = "Room: *BAD VNUM*";
      else
        rname = room->name;
      switch(pReset->arg3)
      {
      default:	ef_name = "(* ERROR *)";	break;
      case 0:	ef_name = "Open";		break;
      case 1:	ef_name = "Close";		break;
      case 2:	ef_name = "Close and lock";	break;
      }
      sprintf(pbuf, "%s [%d] the %s%s [%d] door %s (%d)\n\r", ef_name,
          pReset->arg3, dir_name[pReset->arg2],
          (room && get_exit(room, pReset->arg2) ? "" : " (NO EXIT!)"),
          pReset->arg2, rname, pReset->arg1);
      }
      break;
    case 'R':
      if ( !(room = get_room_index(pReset->arg1)) )
        rname = "Room: *BAD VNUM*";
      else
        rname = room->name;
      sprintf(pbuf, "Randomize exits 0 to %d -> %s (%d)\n\r", pReset->arg2,
          rname, pReset->arg1);
      break;
    }
    if ( ( start == -1 || num >= start ) && show )
      send_to_char( buf, ch );
    if ( end != -1 && num >= end )
      break;
  }
  if ( num == 0 )
    send_to_char( "You don't have any resets defined.\n\r", ch );
  return;
}

/* Setup put nesting levels, regardless of whether or not the resets will
   actually reset, or if they're bugged. */
void renumber_put_resets( AREA_DATA *pArea )
{
  RESET_DATA *pReset, *lastobj = NULL;
  
  for ( pReset = pArea->first_reset; pReset; pReset = pReset->next )
  {
    switch(pReset->command)
    {
    default:
      break;
    case 'G': case 'E': case 'O':
      lastobj = pReset;
      break;
    case 'P':
      if ( pReset->arg3 == 0 )
      {
        if ( !lastobj )
          pReset->extra = 1000000;
        else if ( lastobj->command != 'P' || lastobj->arg3 > 0 )
          pReset->extra = 0;
        else
          pReset->extra = lastobj->extra+1;
        lastobj = pReset;
      }
    }
  }
  return;
}

/*
 * Create a new reset (for online building)			-Thoric
 */
RESET_DATA *make_reset( char letter, int extra, int arg1, int arg2, int arg3 )
{
	RESET_DATA *pReset;

	CREATE( pReset, RESET_DATA, 1 );
	pReset->command	= letter;
	pReset->extra	= extra;
	pReset->arg1	= arg1;
	pReset->arg2	= arg2;
	pReset->arg3	= arg3;
	top_reset++;	
	return pReset;
}

/*
 * Add a reset to an area				-Thoric
 */
RESET_DATA *add_reset( AREA_DATA *tarea, char letter, int extra, int arg1, int arg2, int arg3 )
{
    RESET_DATA *pReset;

    if ( !tarea )
    {
	bug( "add_reset: NULL area!", 0 );
	return NULL;
    }

    letter = UPPER(letter);
    pReset = make_reset( letter, extra, arg1, arg2, arg3 );
    switch( letter )
    {
	case 'M':  tarea->last_mob_reset = pReset;	break;
	case 'H':  if ( arg1 > 0 )			break;
	case 'E':  case 'G':  case 'P':
	case 'O':  tarea->last_obj_reset = pReset;	break;
	case 'T':
	    if ( IS_SET( extra, TRAP_OBJ ) && arg1 == 0)
		tarea->last_obj_reset = pReset;
	    break;
    }

    LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
    return pReset;
}

/*
 * Place a reset into an area, insert sorting it		-Thoric
 */
RESET_DATA *place_reset( AREA_DATA *tarea, char letter, int extra, int arg1, int arg2, int arg3 )
{
    RESET_DATA *pReset, *tmp, *tmp2;

    if ( !tarea )
    {
	bug( "place_reset: NULL area!", 0 );
	return NULL;
    }

    letter = UPPER(letter);
    pReset = make_reset( letter, extra, arg1, arg2, arg3 );
    if ( letter == 'M' )
	tarea->last_mob_reset = pReset;

    if ( tarea->first_reset )
    {
	switch( letter )
	{
	    default:
		bug( "place_reset: Bad reset type %c", letter );
		return NULL;
	    case 'D':	case 'R':
		for ( tmp = tarea->last_reset; tmp; tmp = tmp->prev )
		    if ( tmp->command == letter )
			break;
		if ( tmp )	/* organize by location */
		    for ( ; tmp && tmp->command == letter && tmp->arg1 > arg1; tmp = tmp->prev );
		if ( tmp )	/* organize by direction */
		    for ( ; tmp && tmp->command == letter && tmp->arg1 == tmp->arg1 && tmp->arg2 > arg2; tmp = tmp->prev );
		if ( tmp )
		    INSERT( pReset, tmp, tarea->first_reset, next, prev );
		else
		    LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
		return pReset;
	    case 'M':	case 'O':
		/* find last reset of same type */
		for ( tmp = tarea->last_reset; tmp; tmp = tmp->prev )
		    if ( tmp->command == letter )
			break;
		tmp2 = tmp ? tmp->next : NULL;
		/* organize by location */
		for ( ; tmp; tmp = tmp->prev )
		    if ( tmp->command == letter && tmp->arg3 <= arg3 )
		    {
			tmp2 = tmp->next;
			/* organize by vnum */
			if ( tmp->arg3 == arg3 )
			  for ( ; tmp; tmp = tmp->prev )
			    if ( tmp->command == letter
			    &&   tmp->arg3 == tmp->arg3
			    &&   tmp->arg1 <= arg1 )
			    {
				tmp2 = tmp->next;
				break;
			    }
			    break;
			}
		/* skip over E or G for that mob */
		if ( tmp2 && letter == 'M' )
		{
		    for ( ; tmp2; tmp2 = tmp2->next )
			if ( tmp2->command != 'E' && tmp2->command != 'G' )
			    break;
		}
		else
		/* skip over P, T or H for that obj */
		if ( tmp2 && letter == 'O' )
		{
		    for ( ; tmp2; tmp2 = tmp2->next )
			if ( tmp2->command != 'P' && tmp2->command != 'T'
			&&   tmp2->command != 'H' )
			    break;
		}
		if ( tmp2 )
		    INSERT( pReset, tmp2, tarea->first_reset, next, prev );
		else
		    LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
		return pReset;
	    case 'G':	case 'E':
		/* find the last mob */
		if ( (tmp=tarea->last_mob_reset) != NULL )
		{
		    /*
		     * See if there are any resets for this mob yet,
		     * put E before G and organize by vnum
		     */
		    if ( tmp->next )
		    {
			tmp = tmp->next;
			if ( tmp && tmp->command == 'E' )
			{
			    if ( letter == 'E' )
				for ( ; tmp && tmp->command == 'E' && tmp->arg1 < arg1; tmp = tmp->next );
			    else
				for ( ; tmp && tmp->command == 'E'; tmp = tmp->next );
			}
			else
			if ( tmp && tmp->command == 'G' && letter == 'G' )
			    for ( ; tmp && tmp->command == 'G' && tmp->arg1 < arg1; tmp = tmp->next );
			if ( tmp )
			    INSERT( pReset, tmp, tarea->first_reset, next, prev );
			else
			    LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
		    }
		    else
			LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
		    return pReset;
		}
		break;
	    case 'P':	case 'T':   case 'H':
		/* find the object in question */
		if ( ((letter == 'P' && arg3 == 0)
		||    (letter == 'T' && IS_SET(extra, TRAP_OBJ) && arg1 == 0)
		||    (letter == 'H' && arg1 == 0))
		&&    (tmp=tarea->last_obj_reset) != NULL )
		{
		    if ( (tmp=tmp->next) != NULL )
		      INSERT( pReset, tmp, tarea->first_reset, next, prev );
		    else
		      LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
		    return pReset;
		}

		for ( tmp = tarea->last_reset; tmp; tmp = tmp->prev )
		   if ( (tmp->command == 'O' || tmp->command == 'G'
		   ||    tmp->command == 'E' || tmp->command == 'P')
		   &&    tmp->arg1 == arg3 )
		   {
			/*
			 * See if there are any resets for this object yet,
			 * put P before H before T and organize by vnum
			 */
			if ( tmp->next )
			{
			    tmp = tmp->next;
			    if ( tmp && tmp->command == 'P' )
			    {
				if ( letter == 'P' && tmp->arg3 == arg3 )
				    for ( ; tmp && tmp->command == 'P' && tmp->arg3 == arg3 && tmp->arg1 < arg1; tmp = tmp->next );
				else
				if ( letter != 'T' )
				    for ( ; tmp && tmp->command == 'P' && tmp->arg3 == arg3; tmp = tmp->next );
			    }
			    else
			    if ( tmp && tmp->command == 'H' )
			    {
				if ( letter == 'H' && tmp->arg3 == arg3 )
				    for ( ; tmp && tmp->command == 'H' && tmp->arg3 == arg3 && tmp->arg1 < arg1; tmp = tmp->next );
				else
				if ( letter != 'H' )
				    for ( ; tmp && tmp->command == 'H' && tmp->arg3 == arg3; tmp = tmp->next );
			    }
			    else
			    if ( tmp && tmp->command == 'T' && letter == 'T' )
				for ( ; tmp && tmp->command == 'T' && tmp->arg3 == arg3 && tmp->arg1 < arg1; tmp = tmp->next );
			    if ( tmp )
				INSERT( pReset, tmp, tarea->first_reset, next, prev );
			    else
				LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
			}
			else
			    LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
			return pReset;
		   }
		break;
	}
	/* likely a bad reset if we get here... add it anyways */
    }
    LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
    return pReset;
}
