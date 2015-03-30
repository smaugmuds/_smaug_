/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops, Fireblade, Edmond, Conran                         |             *
 ****************************************************************************
 *  The MUDprograms are heavily based on the original MOBprogram code that  *
 *  was written by N'Atas-ha.						    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

char *	mprog_type_to_name	args( ( int type ) );
ch_ret	simple_damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt );

char *mprog_type_to_name( int type )
{
    switch ( type )
    {
	case IN_FILE_PROG:      return "in_file_prog";
	case ACT_PROG:          return "act_prog";
	case SPEECH_PROG:       return "speech_prog";
	case RAND_PROG:         return "rand_prog";
	case FIGHT_PROG:        return "fight_prog";
	case HITPRCNT_PROG:     return "hitprcnt_prog";
	case DEATH_PROG:        return "death_prog";
	case ENTRY_PROG:        return "entry_prog";
	case GREET_PROG:        return "greet_prog";
	case ALL_GREET_PROG:    return "all_greet_prog";
        case GREET_IN_FIGHT_PROG:return"greet_in_fight_prog";
	case GIVE_PROG:         return "give_prog";
	case BRIBE_PROG:        return "bribe_prog";
	case HOUR_PROG:		return "hour_prog";
	case TIME_PROG:		return "time_prog";
	case WEAR_PROG:         return "wear_prog";
	case REMOVE_PROG:       return "remove_prog";
	case SAC_PROG :         return "sac_prog";
	case LOOK_PROG:         return "look_prog";
	case EXA_PROG:          return "exa_prog";
	case ZAP_PROG:          return "zap_prog";
	case GET_PROG:          return "get_prog";
	case DROP_PROG:         return "drop_prog";
	case REPAIR_PROG:       return "repair_prog";
	case DAMAGE_PROG:       return "damage_prog";
	case PULL_PROG:         return "pull_prog";
	case PUSH_PROG:         return "push_prog";
	case SCRIPT_PROG:	return "script_prog";
	case SLEEP_PROG:        return "sleep_prog";
	case REST_PROG:         return "rest_prog";
	case LEAVE_PROG:        return "leave_prog";
	case USE_PROG:          return "use_prog";
	case LOAD_PROG:		return "load_prog";
        case LOGIN_PROG:        return "login_prog";
        case VOID_PROG:         return "void_prog";
	case IMMINFO_PROG:	return "imminfo_prog";
	case CMD_PROG:		return "cmd_prog";
	case SELL_PROG:		return "sell_prog";
	case TELL_PROG:		return "tell_prog";
	default:                return "ERROR_PROG";
    }
}

/* A trivial rehack of do_mstat.  This doesnt show all the data, but just
 * enough to identify the mob and give its basic condition.  It does however,
 * show the MUDprograms which are set.
 */
void do_mpstat( CHAR_DATA *ch, char *argument )
{
    char        arg[MAX_INPUT_LENGTH];
    MPROG_DATA *mprg;
    CHAR_DATA  *victim;
	sh_int cnt = 0;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "MProg stat whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC( victim ) )
    {
	send_to_char( "Only Mobiles can have MobPrograms!\n\r", ch);
	return;
    }
    if ( get_trust( ch ) < LEVEL_GREATER
    &&  xIS_SET( victim->act, ACT_STATSHIELD ) )
    {
	set_pager_color( AT_IMMORT, ch );
	send_to_pager( "Their godly glow prevents you from getting a good look.\n\r", ch );
	return;
    }
    if ( xIS_EMPTY(victim->pIndexData->progtypes) )
    {
	ch_printf( ch, "No programs on mobile:  %s - #%d\n\r",
		victim->name,
		victim->pIndexData->vnum );
	return;
    }

    ch_printf( ch, "Name: %s.  Vnum: %d.\n\r",
	victim->name, victim->pIndexData->vnum );

    ch_printf( ch, "Short description: %s.\n\rLong  description: %s",
	    victim->short_descr,
	    victim->long_descr[0] != '\0' ?
	    victim->long_descr : "(none).\n\r" );

    ch_printf( ch, "Hp: %d/%d.  Mana: %d/%d.  Move: %d/%d. \n\r",
	victim->hit,         victim->max_hit,
	victim->mana,        victim->max_mana,
	victim->move,        victim->max_move );

    ch_printf( ch,
	"Lv: %d.  Class: %d.  Align: %d.  AC: %d.  Gold: %d.  Exp: %d.\n\r",
	victim->level,       victim->class,        victim->alignment,
	GET_AC( victim ),    victim->gold,         victim->exp );

    for ( mprg = victim->pIndexData->mudprogs; mprg; mprg = mprg->next )
	ch_printf( ch, "%d >%s %s\n\r%s\n\r",
		++cnt, 
		mprog_type_to_name( mprg->type ),
		mprg->arglist,
		mprg->comlist );
    return;
}

/* Opstat - Scryn 8/12*/
void do_opstat( CHAR_DATA *ch, char *argument )
{
    char        arg[MAX_INPUT_LENGTH];
    MPROG_DATA *mprg;
    OBJ_DATA   *obj;
	sh_int	cnt = 0;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "OProg stat what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, arg ) ) == NULL )
    {
	send_to_char( "You cannot find that.\n\r", ch );
	return;
    }

    if ( xIS_EMPTY(obj->pIndexData->progtypes) )
    {
	send_to_char( "That object has no programs set.\n\r", ch);
	return;
    }

    ch_printf( ch, "Name: %s.  Vnum: %d.\n\r",
	obj->name, obj->pIndexData->vnum );

    ch_printf( ch, "Short description: %s.\n\r",
	    obj->short_descr );

    for ( mprg = obj->pIndexData->mudprogs; mprg; mprg = mprg->next )
	ch_printf( ch, "%d >%s %s\n\r%s\n\r",
		++cnt,
		mprog_type_to_name( mprg->type ),
		mprg->arglist,
		mprg->comlist );

    return;

}

/* Rpstat - Scryn 8/12 */
void do_rpstat( CHAR_DATA *ch, char *argument )
{
    MPROG_DATA *mprg;
	sh_int cnt = 0;

    if ( xIS_EMPTY(ch->in_room->progtypes) )
    {
	send_to_char( "This room has no programs set.\n\r", ch);
	return;
    }

    ch_printf( ch, "Name: %s.  Vnum: %d.\n\r",
	ch->in_room->name, ch->in_room->vnum );

    for ( mprg = ch->in_room->mudprogs; mprg; mprg = mprg->next )
	ch_printf( ch, "%d >%s %s\n\r%s\n\r",
		++cnt,
		mprog_type_to_name( mprg->type ),
		mprg->arglist,
		mprg->comlist );
    return;
}

/* Woowoo - Blodkai, November 1997 */
void do_mpasupress( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    int rnds;
 
    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' ) {
       send_to_char( "Mpasupress who?\n\r", ch );
       progbug( "Mpasupress:  invalid (nonexistent?) argument", ch );
       return;
    }
    if ( arg2[0] == '\0' ) {
       send_to_char( "Supress their attacks for how many rounds?\n\r", ch );
       progbug( "Mpasupress:  invalid (nonexistent?) argument", ch );
       return;
    }
    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL ) {
        send_to_char( "No such victim in the room.\n\r", ch );
        progbug( "Mpasupress:  victim not present", ch );
        return;
    }
    rnds = atoi( arg2 );
    if ( rnds < 0 || rnds > 32000 ) {
       send_to_char( "Invalid number of rounds to supress attacks.\n\r", ch );
       progbug( "Mpsupress:  invalid (nonexistent?) argument", ch );
       return;
    }
    add_timer( victim, TIMER_ASUPRESSED, rnds, NULL, 0 );   
    return;
}

/* lets the mobile kill any player or mobile without murder*/
void do_mpkill( CHAR_DATA *ch, char *argument )
{
    char      arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;

    if ( !ch )
    {
	bug( "Nonexistent ch in do_mpkill!", 0 );
	return;
    }

    if ( !IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) )
    {
         send_to_char( "Huh?\n\r", ch );
         return;
    }
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	progbug( "MpKill - no argument", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	progbug( "MpKill - Victim not in room", ch );
	return;
    }

    if ( victim == ch )
    {
	progbug( "MpKill - Bad victim to attack", ch );
	return;
    }

    if ( ch->position == POS_FIGHTING
    ||   ch->position ==  POS_EVASIVE
    ||   ch->position ==  POS_DEFENSIVE
    ||   ch->position ==  POS_AGGRESSIVE
    ||   ch->position ==  POS_BERSERK )
    {
	progbug( "MpKill - Already fighting", ch );
	return;
    }

    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}


/* lets the mobile destroy an object in its inventory
   it can also destroy a worn object and it can destroy
   items using all.xxxxx or just plain all of them */

void do_mpjunk( CHAR_DATA *ch, char *argument )
{
    char      arg[ MAX_INPUT_LENGTH ];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0')
    {
	progbug( "Mpjunk - No argument", ch );
	return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
	if ( ( obj = get_obj_wear( ch, arg ) ) != NULL )
	{
	   unequip_char( ch, obj );
	   extract_obj( obj );
	   return;
	}
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	   return;
	extract_obj( obj );
    }
    else
    for ( obj = ch->first_carrying; obj; obj = obj_next )
    {
	obj_next = obj->next_content;
	if ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
	{
	    if ( obj->wear_loc != WEAR_NONE)
		unequip_char( ch, obj );
	    extract_obj( obj );
	}
    }

    return;

}

/*
 * This function examines a text string to see if the first "word" is a
 * color indicator (e.g. _red, _whi_, _blu).  -  Gorog
 */
int get_color(char *argument)    /* get color code from command string */ 
{ 
   char color[MAX_INPUT_LENGTH]; 
   char *cptr; 
   static char const * color_list= 
         "_bla_red_dgr_bro_dbl_pur_cya_cha_dch_ora_gre_yel_blu_pin_lbl_whi"; 
   static char const * blink_list= 
         "*bla*red*dgr*bro*dbl*pur*cya*cha*dch*ora*gre*yel*blu*pin*lbl*whi"; 
        
   one_argument (argument, color); 
   if (color[0]!='_' && color[0]!='*') return 0;
   if ( (cptr = strstr(color_list, color)) ) 
     return (cptr - color_list) / 4; 
   if ( (cptr = strstr(blink_list, color)) ) 
     return (cptr - blink_list) / 4 + AT_BLINK; 
   return 0; 
} 

/* Prints the argument to all the rooms around the mobile */
void do_mpasound( CHAR_DATA *ch, char *argument )
{
    char                arg1[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA     *was_in_room;
    EXIT_DATA           *pexit;
    sh_int              color;
    EXT_BV              actflags;

    if (!ch ) {
        bug("Nonexistent ch in do_mpasound!",0);
        return;
    }
    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    if ( argument[0] == '\0' ) {
        progbug( "Mpasound - No argument", ch );
        return;
    }
    actflags = ch->act;
    xREMOVE_BIT(ch->act, ACT_SECRETIVE);
    if ( ( color = get_color(argument) ) )
        argument = one_argument(argument, arg1);
    was_in_room = ch->in_room;
    for ( pexit = was_in_room->first_exit; pexit; pexit = pexit->next )
    {
        if ( pexit->to_room
        &&   pexit->to_room != was_in_room )
        {
           ch->in_room = pexit->to_room;
           MOBtrigger  = FALSE;
           if ( color )
                act( color, argument, ch, NULL, NULL, TO_ROOM );
            else
                act( AT_SAY, argument, ch, NULL, NULL, TO_ROOM );
        }
    }
    ch->act = actflags;
    ch->in_room = was_in_room;
    return;
} 
 
/* prints the message to all in the room other than the mob and victim */
void do_mpechoaround( CHAR_DATA *ch, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    EXT_BV     actflags;
    sh_int     color;
 
    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
	progbug( "Mpechoaround - No argument", ch );
	return;
    }
 
    if ( !( victim=get_char_room( ch, arg ) ) )
    {
	progbug( "Mpechoaround - victim does not exist", ch );
	return;
    }
 
    actflags = ch->act;
    xREMOVE_BIT(ch->act, ACT_SECRETIVE);

    /* DONT_UPPER prevents argument[0] from being captilized. --Shaddai */
    DONT_UPPER = TRUE;
    if ( (color = get_color(argument)) )
    {
	argument = one_argument( argument, arg );
	act( color, argument, ch, NULL, victim, TO_NOTVICT );
    }
    else
	act( AT_ACTION, argument, ch, NULL, victim, TO_NOTVICT );

    DONT_UPPER = FALSE;  /* Always set it back to false */
    ch->act = actflags;
}


/* prints message only to victim */
 
void do_mpechoat( CHAR_DATA *ch, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    EXT_BV     actflags;
    sh_int     color;
 
    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
	progbug( "Mpechoat - No argument", ch );
	return;
    }
 
    if ( !( victim = get_char_room( ch, arg ) ) )
    {
	progbug( "Mpechoat - victim does not exist", ch );
	return;
    }
 
    actflags = ch->act;
    xREMOVE_BIT(ch->act, ACT_SECRETIVE);

    DONT_UPPER = TRUE;
    if ( argument[0] == '\0' )
    	act( AT_ACTION, " ", ch, NULL, victim, TO_VICT );
    else if ( (color = get_color(argument)) )
    {
	argument = one_argument( argument, arg );
	act( color, argument, ch, NULL, victim, TO_VICT );
    }
    else
	act( AT_ACTION, argument, ch, NULL, victim, TO_VICT );

    DONT_UPPER = FALSE;

    ch->act = actflags;
}
 

/* prints message to room at large. */

void do_mpecho( CHAR_DATA *ch, char *argument )
{
    char       arg1 [MAX_INPUT_LENGTH];
    sh_int     color;
    EXT_BV     actflags;
 
    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    actflags = ch->act;
    xREMOVE_BIT(ch->act, ACT_SECRETIVE);

    DONT_UPPER = TRUE;
    if ( argument[0] == '\0' )
 	act( AT_ACTION, " ", ch, NULL, NULL, TO_ROOM );
    else if ( (color = get_color(argument)) )
    {
	argument = one_argument ( argument, arg1 );
	act( color, argument, ch, NULL, NULL, TO_ROOM );
    }
    else
	act( AT_ACTION, argument, ch, NULL, NULL, TO_ROOM );
    DONT_UPPER = FALSE;
    ch->act = actflags;
}
/* sound support -haus */

void do_mpsoundaround( CHAR_DATA *ch, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    char       sound[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    EXT_BV     actflags;
 
    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

 
    argument = one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
	progbug( "Mpsoundaround - No argument", ch );
	return;
    }
 
    if ( !( victim=get_char_room( ch, arg ) ) )
    {
	progbug( "Mpsoundaround - victim does not exist", ch );
	return;
    }
 
    actflags = ch->act;
    xREMOVE_BIT(ch->act, ACT_SECRETIVE);

    sprintf(sound,"!!SOUND(%s)\n", argument);
    act( AT_ACTION, sound, ch, NULL, victim, TO_NOTVICT );

    ch->act = actflags;
}


/* prints message only to victim */
 
void do_mpsoundat( CHAR_DATA *ch, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    char       sound [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    EXT_BV     actflags;
 
    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

 
    argument = one_argument( argument, arg );
 
    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	progbug( "Mpsoundat - No argument", ch );
	return;
    }
 
    if ( !( victim = get_char_room( ch, arg ) ) )
    {
	progbug( "Mpsoundat - victim does not exist", ch );
	return;
    }
 
    actflags = ch->act;
    xREMOVE_BIT(ch->act, ACT_SECRETIVE);

    sprintf(sound,"!!SOUND(%s)\n", argument);
    act( AT_ACTION, sound, ch, NULL, victim, TO_VICT );

    ch->act = actflags;
}
 

/* prints message to room at large. */

void do_mpsound( CHAR_DATA *ch, char *argument )
{
    char	sound [MAX_INPUT_LENGTH];
    EXT_BV	actflags;
 
    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

 
    if ( argument[0] == '\0' )
    {
	progbug( "Mpsound - called w/o argument", ch );
	return;
    }
 
    actflags = ch->act;
    xREMOVE_BIT(ch->act, ACT_SECRETIVE);

    sprintf(sound,"!!SOUND(%s)\n", argument);
    act( AT_ACTION, sound, ch, NULL, NULL, TO_ROOM );

    ch->act = actflags;
}
/* end sound stuff ----------------------------------------*/

/* Music stuff, same as above, at zMUD coders' request -- Blodkai */
void do_mpmusicaround( CHAR_DATA *ch, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    char       music[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    EXT_BV     actflags;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' ) {
        progbug( "Mpmusicaround - No argument", ch );
       return;
    }
    if ( !( victim=get_char_room( ch, arg ) ) ) {
        progbug( "Mpmusicaround - victim does not exist", ch );
        return;
    }
    actflags = ch->act;
    xREMOVE_BIT(ch->act, ACT_SECRETIVE);
    sprintf(music,"!!MUSIC(%s)\n", argument);
    act( AT_ACTION, music, ch, NULL, victim, TO_NOTVICT );
    ch->act = actflags;
    return;
}
void do_mpmusic( CHAR_DATA *ch, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    char       music[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    EXT_BV     actflags;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' ) {
        progbug( "Mpmusic - No argument", ch );
       return;
    }
    if ( !( victim=get_char_room( ch, arg ) ) ) {
        progbug( "Mpmusic - victim does not exist", ch );
        return;
    }
    actflags = ch->act;
    xREMOVE_BIT(ch->act, ACT_SECRETIVE);
    sprintf(music,"!!MUSIC(%s)\n", argument);
    act( AT_ACTION, music, ch, NULL, victim, TO_ROOM );
    ch->act = actflags;
    return;
}
void do_mpmusicat( CHAR_DATA *ch, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    char       music[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    EXT_BV     actflags;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' ) {
        progbug( "Mpmusicat - No argument", ch );
       return;
    }
    if ( !( victim=get_char_room( ch, arg ) ) ) {
        progbug( "Mpmusicat - victim does not exist", ch );
        return;
    }
    actflags = ch->act;
    xREMOVE_BIT(ch->act, ACT_SECRETIVE);
    sprintf(music,"!!MUSIC(%s)\n", argument);
    act( AT_ACTION, music, ch, NULL, victim, TO_VICT );
    ch->act = actflags;
    return;
}

/* lets the mobile load an item or mobile.  All items
are loaded into inventory.  you can specify a level with
the load object portion as well. */
void do_mpmload( CHAR_DATA *ch, char *argument )
{
    char            arg[ MAX_INPUT_LENGTH ];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA      *victim;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
	progbug( "Mpmload - Bad vnum as arg", ch );
	return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
	progbug( "Mpmload - Bad mob vnum", ch );
	return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    return;
}

void do_mpoload( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA       *obj;
    int             level;
    int		    timer = 0;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }


    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
	progbug( "Mpoload - Bad syntax", ch );
	return;
    }

    if ( arg2[0] == '\0' )
	level = get_trust( ch );
    else
    {
	/*
	 * New feature from Alander.
	 */
	if ( !is_number( arg2 ) )
	{
	    progbug( "Mpoload - Bad level syntax", ch );
	    return;
	}
	level = atoi( arg2 );
	if ( level < 0 || level > get_trust( ch ) )
	{
	    progbug( "Mpoload - Bad level", ch );
	    return;
	}

	/*
	 * New feature from Thoric.
	 */
	timer = atoi( argument );
	if ( timer < 0 )
	{
	   progbug( "Mpoload - Bad timer", ch );
	   return;
	}
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	progbug( "Mpoload - Bad vnum arg", ch );
	return;
    }

    obj = create_object( pObjIndex, level );
    obj->timer = timer;
    if ( CAN_WEAR(obj, ITEM_TAKE) )
	obj_to_char( obj, ch );
    else
	obj_to_room( obj, ch->in_room );

    return;
}

/* mpplace by Edmond 010731 */
void do_mpplace( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char arg3[MAX_STRING_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *pRoomIndex;
    AREA_DATA *tarea;
    bool found = FALSE;
    int lo_vnum, hi_vnum;

    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Mpplace what?\n\r", ch );
	progbug( "Mpplace: no argument", ch );
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
        send_to_char( "You cannot mpplace that item.\n\r", ch );
	progbug( "Mpplace: not carrying such object", ch);
        return;
    }

	if ( arg2[0] == '\0' )
	{
		lo_vnum = 101;
		hi_vnum = MAX_VNUM;
	}
	else
	if ( !strcmp( arg2, "area" ) )
	{
	    for ( tarea = first_area; tarea; tarea = tarea->next )
	    {
		if (!str_cmp( arg3, tarea->filename ) )
		{
			found=TRUE;
			break;
		}
	    }
	    if (!found)
	     for ( tarea = first_build; tarea; tarea = tarea->next )
	     {
		if (!str_cmp( arg2, tarea->filename ) )
		{
		  found = TRUE;
		  break;
		}
	     }
            if ( !found )
	    {
		send_to_char( "Invalid area - you cannot mpplace there.\n\r", ch );
		progbug( "Mpplace: no such area", ch);
		return;
	    }
	    else
	    {
		lo_vnum = tarea->low_r_vnum;
		hi_vnum = tarea->hi_r_vnum;
	    }
	}
	else
	if ( is_number(arg2) && is_number(arg3) )
	{
	   lo_vnum = atoi(arg2);
	   hi_vnum = atoi(arg3);
	   if ( hi_vnum <= lo_vnum )
	   {
	     send_to_char( "Invalid vnum range, hi vnum lower than/equal to  low vnum.\n\r", ch );
	     progbug( "Mpplace: invalid vnum range", ch );
	    return;
           }
        }
	else
	{
	   send_to_char( "Invalid syntax.\n\r", ch);
	   progbug( "Mpplace: Invalid syntax", ch);
	   return;
	}

	for ( ;; )
	{
	    pRoomIndex = get_room_index( number_range( lo_vnum, hi_vnum ) );
	    if ( pRoomIndex )
		if ( !pRoomIndex->first_person
		    || xIS_SET(pRoomIndex->room_flags, ROOM_DEATH ) )
	    break;
	}

	sprintf( buf, "%d drop %s", pRoomIndex->vnum, arg1 );
	do_mpat( ch, buf );
	return;
}

/* Just a hack of do_pardon from act_wiz.c -- Blodkai, 6/15/97 */
void do_mppardon( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
 
    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

 
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        progbug( "Mppardon:  missing argument", ch );
        send_to_char( "Mppardon who for what?\n\r", ch );
        return;
    }
    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        progbug( "Mppardon: offender not present", ch );
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
    if ( IS_NPC( victim ) )
    {
        progbug( "Mppardon:  trying to pardon NPC", ch );
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }
    if ( !str_cmp( arg2, "attacker" ) )
    {
        if ( xIS_SET(victim->act, PLR_ATTACKER) )
        {
            xREMOVE_BIT( victim->act, PLR_ATTACKER );
            send_to_char( "Attacker flag removed.\n\r", ch );
            send_to_char( "Your crime of attack has been pardoned.\n\r", victim );
        }
        return;
    }
    if ( !str_cmp( arg2, "killer" ) )
    {
        if ( xIS_SET(victim->act, PLR_KILLER) )
        {
            xREMOVE_BIT( victim->act, PLR_KILLER );
            send_to_char( "Killer flag removed.\n\r", ch );
            send_to_char( "Your crime of murder has been pardoned.\n\r", victim );
        }
        return;
    }
    if ( !str_cmp( arg2, "litterbug" ) )
    {
	if ( xIS_SET(victim->act, PLR_LITTERBUG) )
	{
	    xREMOVE_BIT( victim->act, PLR_LITTERBUG );
	    send_to_char( "Litterbug flag removed.\n\r", ch );
	    send_to_char( "Your crime of littering has been pardoned./n/r", victim );
	}
    return;
    }
    if ( !str_cmp( arg2, "thief" ) )
    {
        if ( xIS_SET(victim->act, PLR_THIEF) )
        {
            xREMOVE_BIT( victim->act, PLR_THIEF );
            send_to_char( "Thief flag removed.\n\r", ch );
            send_to_char( "Your crime of theft has been pardoned.\n\r", victim );
        }
        return;
    }
    send_to_char( "Pardon who for what?\n\r", ch );
    progbug( "Mppardon: Invalid argument", ch );
    return;
}

/* lets the mobile purge all objects and other npcs in the room,
   or purge a specified object or mob in the room.  It can purge
   itself, but this had best be the last command in the MUDprogram
   otherwise ugly stuff will happen */
void do_mppurge( CHAR_DATA *ch, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }


    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	/* 'purge' */
	CHAR_DATA *vnext;

	for ( victim = ch->in_room->first_person; victim; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( IS_NPC( victim ) && victim != ch )
	      extract_char( victim, TRUE );
	}
	while ( ch->in_room->first_content )
	   extract_obj( ch->in_room->first_content );

	return;
    }

    if ( (victim = get_char_room( ch, arg )) == NULL )
    {
	if ( (obj = get_obj_here( ch, arg )) != NULL )
	    extract_obj( obj );
	else
	    progbug( "Mppurge - Bad argument", ch );
	return;
    }

    if ( !IS_NPC( victim ) )
    {
	progbug( "Mppurge - Trying to purge a PC", ch );
	return;
    }

    if ( victim == ch )
    {
    	progbug( "Mppurge - Trying to purge oneself", ch );
    	return;
    }

    if ( IS_NPC( victim ) && victim->pIndexData->vnum == 3 )
    {
        progbug( "Mppurge: trying to purge supermob", ch );
	return;
    }
    
    extract_char( victim, TRUE );
    return;
}


/* Allow mobiles to go wizinvis with programs -- SB */
 
void do_mpinvis( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int level;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }


    argument = one_argument( argument, arg );
    if ( arg && arg[0] != '\0' )
    {
        if ( !is_number( arg ) )
        {
           progbug( "Mpinvis - Non numeric argument ", ch );
           return;
        }       
        level = atoi( arg );
        if ( level < 2 || level > LEVEL_IMMORTAL )
        {
            progbug( "MPinvis - Invalid level ", ch );
            return;
        }
 
	ch->mobinvis = level;
	ch_printf( ch, "Mobinvis level set to %d.\n\r", level );
	return;
    }
 
    if ( ch->mobinvis < 2 )
      ch->mobinvis = ch->level;
 
    if ( xIS_SET(ch->act, ACT_MOBINVIS) )
    {
        xREMOVE_BIT(ch->act, ACT_MOBINVIS);
	act(AT_IMMORT, "$n slowly fades into existence.", ch, NULL, NULL,TO_ROOM );
	send_to_char( "You slowly fade back into existence.\n\r", ch );       
    }
    else
    {
        xSET_BIT(ch->act, ACT_MOBINVIS);
	act( AT_IMMORT, "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
        send_to_char( "You slowly vanish into thin air.\n\r", ch );
    }
    return;
}

/* lets the mobile goto any location it wishes that is not private */
/* Mounted chars follow their mobiles now - Blod, 11/97 */
void do_mpgoto( CHAR_DATA *ch, char *argument )
{
    char             arg[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *location;
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }


    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	progbug( "Mpgoto - No argument", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	progbug( "Mpgoto - No such location", ch );
	return;
    }

    in_room = ch->in_room;
    if ( ch->fighting )
	stop_fighting( ch, TRUE );
    char_from_room( ch );
    char_to_room( ch, location );
    for ( fch = in_room->first_person; fch; fch = fch_next )
    {
        fch_next = fch->next_in_room;
	if ( fch->mount && fch->mount == ch )
	{
	  char_from_room( fch );
	  char_to_room( fch, location );
	}
    }
    return;
}

/* lets the mobile do a command at another location. Very useful */

void do_mpat( CHAR_DATA *ch, char *argument )
{
    char             arg[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }


    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	progbug( "Mpat - Bad argument", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	progbug( "Mpat - No such location", ch );
	return;
    }

    original = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    if ( !char_died(ch) )
    {
      char_from_room( ch );
      char_to_room( ch, original );
    }

    return;
}

/* allow a mobile to advance a player's level... very dangerous */
void do_mpadvance( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;
    int iLevel;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }


    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	progbug( "Mpadvance - Bad syntax", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	progbug( "Mpadvance - Victim not there", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	progbug( "Mpadvance - Victim is NPC", ch );
	return;
    }

    if ( victim->level >= LEVEL_AVATAR )
      return;

	level = victim->level + 1;

	if ( victim->level > ch->level )
	{
	  act( AT_TELL, "$n tells you, 'Sorry... you must seek someone more powerful than I.'",
	       ch, NULL, victim, TO_VICT );
	  return;
	}

	if (victim->level >= LEVEL_AVATAR)
	{
	  set_char_color( AT_IMMORT, victim );
	  act( AT_IMMORT, "$n makes some arcane gestures with $s hands, then points $s fingers at you!",
	       ch, NULL, victim, TO_VICT );
	  act( AT_IMMORT, "$n makes some arcane gestures with $s hands, then points $s fingers at $N!",
	       ch, NULL, victim, TO_NOTVICT );
	  set_char_color( AT_WHITE, victim );
	  send_to_char( "You suddenly feel very strange...\n\r\n\r", victim );
	  set_char_color( AT_LBLUE, victim );
	}

	switch(level)
	{
	default:
	  send_to_char( "You feel more powerful!\n\r", victim );
	  break;
	case LEVEL_IMMORTAL:
	  do_help(victim, "M_GODLVL1_" );
	  set_char_color( AT_WHITE, victim );
	  send_to_char( "You awake... all your possessions are gone.\n\r", victim );

	  while ( victim->first_carrying )
	    extract_obj( victim->first_carrying );
	  break;
	case LEVEL_ACOLYTE:
	  do_help(victim, "M_GODLVL2_" );
	  break;
	case LEVEL_CREATOR:
	  do_help(victim, "M_GODLVL3_" );
	  break;
	case LEVEL_SAVIOR:
	  do_help(victim, "M_GODLVL4_" );
	  break;
	case LEVEL_DEMI:
	  do_help(victim, "M_GODLVL5_" );
	  break;
	case LEVEL_TRUEIMM:
	  do_help(victim, "M_GODLVL6_" );
	  break;
	case LEVEL_LESSER:
	  do_help(victim, "M_GODLVL7_" );
	  break;
	case LEVEL_GOD:
	  do_help(victim, "M_GODLVL8_" );
	  break;
	case LEVEL_GREATER:
	  do_help(victim, "M_GODLVL9_" );
	  break;
	case LEVEL_ASCENDANT:
	  do_help(victim, "M_GODLVL10_" );
	  break;
	case LEVEL_SUB_IMPLEM:
	  do_help(victim, "M_GODLVL11_" );
	  break;
	case LEVEL_IMPLEMENTOR:
	  do_help(victim, "M_GODLVL12_" );
	  break;
	case LEVEL_ETERNAL:
	  do_help(victim, "M_GODLVL13_" );
	  break;
	case LEVEL_INFINITE:
	  do_help(victim, "M_GODLVL14_" );
	  break;
	case LEVEL_SUPREME:
	  do_help(victim, "M_GODLVL15_" );
	}

    for ( iLevel = victim->level ; iLevel < level; iLevel++ )
    {
	if (level < LEVEL_IMMORTAL)
	  send_to_char( "You raise a level!!  ", victim );
	victim->level += 1;
	advance_level( victim );
    }
    victim->exp   = 1000 * UMAX( 1, victim->level );
    victim->trust = 0;
    return;
}



/* lets the mobile transfer people.  the all argument transfers
   everyone in the current room to the specified location 
   the area argument transfers everyone in the current area to the
   specified location */
/*
 * Bug fixes for transfer all by Gianfranco Finell    -- Shaddai
 */
void do_mptransfer( CHAR_DATA *ch, char *argument )
{
    char             arg1[ MAX_INPUT_LENGTH ];
    char             arg2[ MAX_INPUT_LENGTH ];
    char 	     buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *destination;
    CHAR_DATA       *victim;
    CHAR_DATA       *ChNext;
    CHAR_DATA	    *ChList;
    CHAR_DATA       *ListIdx;
    DESCRIPTOR_DATA *d;


    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( !ch->in_room ) {
    	progbug( "Mptransfer - ch in NULL room", ch );
	return;
    }

    if ( arg1[0] == '\0' )
    {
	progbug( "Mptransfer - Bad syntax", ch );
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */

    if ( arg2[0] == '\0' ) {
    	destination = ch->in_room;
	if ( !destination ) {
		progbug( "MpTransfer - Null room destination", ch );
		return;
	}
    }
    else {
    	destination = find_location( ch, arg2 );
	if ( !destination ) {
		progbug( "MPtransfer - Unable to find destination room",ch);
		return;
	}
	if ( room_is_private( destination ) ) {
		progbug( "Mptransfer - Destination is a private room", ch );
		return;
	}
     }

    ChList = NULL;
    /* Put in the variable nextinroom to make this work right. -Narn */
    if ( !str_cmp( arg1, "all" ) )
    {
	for ( victim = ch->in_room->last_person; victim; victim = ChNext )
	{
            ChNext = victim->prev_in_room;

	    if ( victim == ch
	    ||	 NOT_AUTHED(victim)
	    ||   !can_see( ch, victim) 
	    ||   !in_hard_range( victim, destination->area) )
		continue;
	
	    if ( victim->fighting )
	    	stop_fighting( victim, TRUE );
	    char_from_room( victim );
	    victim->next_in_room = ChList;
	    ChList = victim;
	}
    }
    /* This will only transfer PC's in the area not Mobs --Shaddai */
    else if ( !str_cmp( arg1, "area" ) )
    {
	for ( d = first_descriptor; d; d = d->next )
	{
		if ( !d->character 
		|| (d->connected != CON_PLAYING && d->connected != CON_EDITING) 
		|| !can_see(ch, d->character)
		|| !d->character->in_room
	        || ch->in_room->area != d->character->in_room->area 
		|| NOT_AUTHED(d->character) 
		|| !in_hard_range(d->character, destination->area))
			continue;
		if ( d->character->fighting )
			stop_fighting( d->character, TRUE );
		char_from_room( d->character );
		d->character->next_in_room = ChList;
		ChList = d->character;
        }
    }
    else {
       if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
       {
	   progbug( "Mptransfer - No such person", ch );
	   return;
       }

       if ( !victim->in_room )
       {
	   progbug( "Mptransfer - Victim in Limbo", ch );
	   return;
       }

       if (NOT_AUTHED(victim) && destination->area != victim->in_room->area)
       {
	   sprintf( buf, "Mptransfer - unauthed char (%s)", victim->name );
 	   progbug( buf, ch);
	   return;
       }

       /* If victim not in area's level range, do not transfer */
       if ( !in_hard_range( victim, destination->area ) 
       &&   !xIS_SET( destination->room_flags, ROOM_PROTOTYPE ) )
      	   return;

       if ( victim->fighting )
	   stop_fighting( victim, TRUE );

       char_from_room( victim );
       victim->next_in_room = ChList;
       ChList = victim;
    }
    for ( ListIdx = ChList; ListIdx; ListIdx = ListIdx->next_in_room ) {
      for (victim = ListIdx->was_in_room->last_person;victim; victim = ChNext){
         ChNext = victim->prev_in_room;
	 if ( IS_NPC(victim) || get_trust(victim) <= LEVEL_AVATAR ||
	      victim->master != ListIdx )
	      continue;
	 if ( victim->fighting )
	 	stop_fighting( victim, TRUE );
	 char_from_room( victim );
	 victim->next_in_room = ChList;
	 ChList = victim;
      }
    }

    for ( ; ChList; ChList = ChNext ) {
    	ChNext = ChList->next_in_room;
	ChList->next_in_room = NULL;
	char_to_room( ChList, destination );
    }

    return;
}

/* lets the mobile force someone to do something.  must be mortal level
   and the all argument only affects those in the room with the mobile */

void do_mpforce( CHAR_DATA *ch, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];
	sh_int drunk = 0, mst;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }


    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	progbug( "Mpforce - Bad syntax", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	CHAR_DATA *vch;

	for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
	    if ( !IS_IMMORTAL(vch) && can_see( ch, vch ) )
	{
		mst = vch->mental_state;
		vch->mental_state = 0;
		if (!IS_NPC( vch ))
		{
		  drunk = vch->pcdata->condition[COND_DRUNK];
		  vch->pcdata->condition[COND_DRUNK] = 0;
		}		
		interpret( vch, argument );
        vch->mental_state = mst;
        if (!IS_NPC( vch ))
          vch->pcdata->condition[COND_DRUNK] = drunk;
	}
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    progbug( "Mpforce - No such victim", ch );
	    return;
	}

	if ( victim == ch )
	{
	    progbug( "Mpforce - Forcing oneself", ch );
	    return;
	}

	if ( !IS_NPC( victim )
	&& ( !victim->desc )
	&& IS_IMMORTAL( victim ) )
	{
	    progbug( "Mpforce - Attempting to force link dead immortal", ch );
	    return;
	}

    mst = victim->mental_state;
    victim->mental_state = 0;
    if (!IS_NPC( victim ))
    {
      drunk = victim->pcdata->condition[COND_DRUNK];
      victim->pcdata->condition[COND_DRUNK] = 0;
    }
	interpret( victim, argument );
    victim->mental_state = mst;
    if (!IS_NPC( victim ))
      victim->pcdata->condition[COND_DRUNK] = drunk;

    }

    return;
}


/*
 * mpnuisance mpunnuisance just incase we need them later --Shaddai
 */

void do_mpnuisance( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg1[MAX_STRING_LENGTH];
    struct tm *now_time;

    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

   argument = one_argument( argument, arg1 );

   if ( arg1[0] == '\0' )
   {
        progbug( "Mpnuisance - called w/o enough argument(s)", ch );
        return;
   }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Victim must be in room.\n\r", ch );
        progbug( "Mpnuisance: victim not in room", ch );
        return;
    }
    if ( IS_NPC(victim) )
    {
	progbug( "Mpnuisance: victim is a mob", ch );
	return;
    }
    if ( IS_IMMORTAL(victim) )
    {
	progbug( "Mpnuisance: not allowed on immortals", ch );
	return;
    }
    if ( victim->pcdata->nuisance )
    {
	progbug( "Mpnuisance: victim is already nuisanced", ch );
	return;
    }
    CREATE ( victim->pcdata->nuisance, NUISANCE_DATA, 1 );
    victim->pcdata->nuisance->time = current_time;
    victim->pcdata->nuisance->flags = 1;
    victim->pcdata->nuisance->power = 2;
    now_time = localtime(&current_time);
    now_time->tm_mday+= 1;
    victim->pcdata->nuisance->max_time = mktime(now_time);
    add_timer( victim, TIMER_NUISANCE, (28800*2), NULL, 0 );
    return;
}

void do_mpunnuisance( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    TIMER *timer, *timer_next;
    char arg1[MAX_STRING_LENGTH];

    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

   argument = one_argument( argument, arg1 );

   if ( arg1[0] == '\0' )
   {
        progbug( "Mpunnuisance - called w/o enough argument(s)", ch );
        return;
   }

   if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
        send_to_char( "Victim must be in room.\n\r", ch );
        progbug( "Mpunnuisance: victim not in room", ch );
        return;
   }

   if ( IS_NPC(victim ) )
   {
	progbug( "Mpunnuisance: victim was a mob", ch );
	return;
   }

   if ( IS_IMMORTAL( victim ) )
   {
	progbug( "Mpunnuisance: victim was an immortal", ch );
	return;
   }

   if ( !ch->pcdata->nuisance )
   {
	progbug( "Mpunnuisance: victim is not nuisanced", ch );
	return;
   }
   for (timer = victim->first_timer; timer; timer = timer_next)
   {
      timer_next = timer->next;
      if ( timer->type == TIMER_NUISANCE )
	extract_timer (victim, timer);
   }
   DISPOSE ( victim->pcdata->nuisance );
   return;
}

/*
 * mpbodybag for mobs to do cr's  --Shaddai
 */
void do_mpbodybag( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char arg[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char buf4[MAX_STRING_LENGTH];


    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

   argument = one_argument( argument, arg );

   if ( arg[0] == '\0' )
   {
        progbug( "Mpbodybag - called w/o enough argument(s)", ch );
        return;
   }

   if ( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
        send_to_char( "Victim must be in room.\n\r", ch );
        progbug( "Mpbodybag: victim not in room", ch );
        return;
   }
   if ( IS_NPC( victim ) )
   {
	progbug( "Mpbodybag: bodybagging a npc corpse", ch );
	return;
   }
   sprintf (buf3, " ");
   sprintf (buf2, "the corpse of %s", arg);
   for (obj = first_object; obj; obj = obj->next)
   {
     if (obj->in_room
         && !str_cmp (buf2, obj->short_descr)
         && (obj->pIndexData->vnum == 11))
       {
	 obj_from_room (obj);
  	 obj = obj_to_char (obj, ch);
         obj->timer = -1;
       }
   }
   /* Maybe should just make the command logged... Shrug I am not sure
    * --Shaddai
    */
   sprintf(buf4, "Mpbodybag: Grabbed %s", buf2 );
   progbug( buf4, ch );
   return;
}

/*
 * mpmorph and mpunmorph for morphing people with mobs. --Shaddai
 */

void do_mpmorph( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    MORPH_DATA *morph;
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];

    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if ( arg1[0] == '\0' || arg2[0] == '\0' )
   {
        progbug( "Mpmorph - called w/o enough argument(s)", ch );
        return;
   }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Victim must be in room.\n\r", ch );
        progbug( "Mpmorph: victim not in room", ch );
        return;
    }


   if ( !is_number(arg2) )
	morph = get_morph( arg2 );
   else
	morph = get_morph_vnum( atoi(arg2) );
   if ( !morph )
   {
        progbug( "Mpmorph - unknown morph", ch );
        return;
   }
   if ( victim->morph )
   {
        progbug( "Mpmorph - victim already morphed", ch );
        return;
   }
   do_morph_char ( victim, morph );
   return;
}

void do_mpunmorph( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char arg[MAX_STRING_LENGTH];

  if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
  {
        send_to_char( "Huh?\n\r", ch );
        return;
  }

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  { 
        progbug( "Mpmorph - called w/o an argument", ch );
        return;
  }

  if ( ( victim = get_char_room( ch, arg ) ) == NULL )
  {
      send_to_char( "Victim must be in room.\n\r", ch );
      progbug( "Mpunmorph: victim not in room", ch );
      return;
  }
  if ( !victim->morph )
  {
	progbug( "Mpunmorph: victim not morphed", ch );
	return;
  }
  do_unmorph_char( victim );
  return;
}

void do_mpechozone( CHAR_DATA *ch, char *argument ) /* Blod, late 97 */
{
    char       arg1[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    sh_int     color;
    EXT_BV     actflags;
 
    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    actflags = ch->act;
    xREMOVE_BIT(ch->act, ACT_SECRETIVE);
    if ( ( color = get_color(argument) ) )
	argument = one_argument(argument, arg1);
    DONT_UPPER = TRUE;
    for ( vch = first_char; vch; vch = vch_next )
    {
        vch_next = vch->next;
        if ( vch->in_room->area == ch->in_room->area
	&&  !IS_NPC(vch)
        &&   IS_AWAKE(vch) )
        {
	    if ( argument[0] == '\0' )
	     	act( AT_ACTION, " ", vch, NULL, NULL, TO_CHAR );
            else if ( color )
                act( color, argument, vch, NULL, NULL, TO_CHAR );
            else
              act( AT_ACTION, argument, vch, NULL, NULL, TO_CHAR );
        }
    }
    DONT_UPPER = FALSE;
    ch->act = actflags;
}

/*
 *  Haus' toys follow:
 */

/*
 * syntax:  mppractice victim spell_name max%
 *
 */
void do_mp_practice( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char arg3[ MAX_INPUT_LENGTH ];
    char buf[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    int sn, max, tmp, adept;
    char *skill_name;

    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg1 );  
    argument = one_argument( argument, arg2 );  
    argument = one_argument( argument, arg3 );  

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Mppractice: bad syntax", ch );
	progbug( "Mppractice - Bad syntax", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
       send_to_char("Mppractice: Student not in room? Invis?", ch);
       progbug( "Mppractice: Invalid student not in room", ch );
       return;
    }

    if ( ( sn = skill_lookup( arg2 ) ) < 0 )
    {
       send_to_char("Mppractice: Invalid spell/skill name", ch);
       progbug( "Mppractice: Invalid spell/skill name", ch );
       return;
    }


    if(IS_NPC(victim))
    {
       send_to_char("Mppractice: Can't train a mob", ch);
       progbug("Mppractice: Can't train a mob", ch );
       return;
    }

    skill_name = skill_table[sn]->name;

    max = atoi( arg3 );
    if( (max<0) || (max>100) )
    {
	sprintf( log_buf, "mp_practice: Invalid maxpercent: %d", max );
	send_to_char( log_buf, ch);
	progbug( log_buf, ch );
	return;
    }

    if(victim->level < skill_table[sn]->skill_level[victim->class] )
    {
	sprintf(buf,"$n attempts to tutor you in %s, but it's beyond your comprehension.",skill_name);
	act( AT_TELL, buf, ch, NULL, victim, TO_VICT );
	return;
    }

    /* adept is how high the player can learn it */
    /* adept = class_table[ch->class]->skill_adept; */
    adept = GET_ADEPT(victim,sn);

    if ( (victim->pcdata->learned[sn] >= adept )
    ||   (victim->pcdata->learned[sn] >= max   ) )
    {
	sprintf(buf,"$n shows some knowledge of %s, but yours is clearly superior.",skill_name);
	act( AT_TELL, buf, ch, NULL, victim, TO_VICT );
	return;
    }
    

    /* past here, victim learns something */
    tmp = UMIN(victim->pcdata->learned[sn] + int_app[get_curr_int(victim)].learn, max);
    act( AT_ACTION, "$N demonstrates $t to you.  You feel more learned in this subject.", victim, skill_table[sn]->name, ch,TO_CHAR );

    victim->pcdata->learned[sn] = max; 


    if ( victim->pcdata->learned[sn] >= adept )
    {
	victim->pcdata->learned[sn] = adept;
	act( AT_TELL, "$n tells you, 'You have learned all I know on this subject...'",
		ch, NULL, victim, TO_VICT );
    }
    return;

}
/*
void do_mpstrew( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj_next;
    OBJ_DATA *obj_lose;
    ROOM_INDEX_DATA *pRoomIndex;
    int low_vnum, high_vnum, rvnum;

    set_char_color( AT_IMMORT, ch );
 
    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' ) {
     send_to_char( "Mpstrew whom?\n\r", ch );
     progbug( "Mpstrew: invalid (nonexistent?) argument", ch );
     return;
    }
    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL ) {
      send_to_char( "Victim must be in room.\n\r", ch );
      progbug( "Mpstrew: victim not in room", ch );
      return;
    }
    if ( IS_IMMORTAL( victim ) && get_trust( victim ) >= get_trust( ch ) ) {
      send_to_char( "You haven't the power to succeed against this victim.\n\r", ch );
      progbug( "Mpstrew: victim level too high", ch );
      return;
    }
    if ( !str_cmp( arg2, "coins"  ) ) {
      if ( victim->gold < 1) {
        send_to_char( "Drat, this one's got no gold to start with.\n\r", ch );
        return;
      }
      victim->gold = 0;
      return;
    }
    if (arg2[0] == '\0') {
      send_to_char( "You must specify a low vnum.\n\r", ch );
      progbug( "Mpstrew:  missing low vnum", ch );
      return;
    }
    if (argument[0] == '\0') {
      send_to_char( "You must specify a high vnum.\n\r", ch );
      progbug( "Mpstrew:  missing high vnum", ch );
      return;
    }
   low_vnum = atoi( arg2 ); high_vnum = atoi( argument );
    if ( low_vnum < 1 || high_vnum < low_vnum || low_vnum > high_vnum || low_vnum == high_vnum || high_vnum > MAX_VNUM ) {
        send_to_char( "Invalid range.\n\r", ch );
        progbug( "Mpstrew:  invalid range", ch );
        return;
    }
    for ( ; ; ) {
      rvnum = number_range( low_vnum, high_vnum );
      pRoomIndex = get_room_index( rvnum );
      if ( pRoomIndex )
        break;
    }
    if ( !str_cmp( arg2, "inventory" ) ) {
      for ( obj_lose=victim->first_carrying; obj_lose; obj_lose=obj_next ) {
        obj_next = obj_lose->next_content;
        obj_from_char( obj_lose );
        obj_to_room( obj_lose, rvnum );
        pager_printf_color( ch, "\t&w%s sent to %d\n\r",
 	  capitalize(obj_lose->short_descr), pRoomIndex->vnum );
      }
      return;
    }
    send_to_char( "Strew their coins or inventory?\n\r", ch );
    progbug( "Mpstrew:  no arguments", ch );
    return;
}
*/
void do_mpscatter( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *pRoomIndex;
    int low_vnum, high_vnum, rvnum;
 
    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' ) {
     send_to_char( "Mpscatter whom?\n\r", ch );
     progbug( "Mpscatter: invalid (nonexistent?) argument", ch );
     return;
    }
    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL ) {
      send_to_char( "Victim must be in room.\n\r", ch );
      progbug( "Mpscatter: victim not in room", ch );
      return;
    }
    if ( IS_IMMORTAL( victim ) && get_trust( victim ) >= get_trust( ch ) ) {
      send_to_char( "You haven't the power to succeed against this victim.\n\r", ch );
      progbug( "Mpscatter: victim level too high", ch );
      return;
    }
    if (arg2[0] == '\0') {
      send_to_char( "You must specify a low vnum.\n\r", ch );
      progbug( "Mpscatter:  missing low vnum", ch );
      return;
    }
    if (argument[0] == '\0') {
      send_to_char( "You must specify a high vnum.\n\r", ch );
      progbug( "Mpscatter:  missing high vnum", ch );
      return;
    }
    low_vnum = atoi( arg2 ); high_vnum = atoi( argument );
    if ( low_vnum < 1 || high_vnum < low_vnum || low_vnum > high_vnum || low_vnum == high_vnum || high_vnum > MAX_VNUM ) {
        send_to_char( "Invalid range.\n\r", ch );
        progbug( "Mpscatter:  invalid range", ch );
        return;
    }
    while (1) {
      rvnum = number_range( low_vnum, high_vnum );
      pRoomIndex = get_room_index( rvnum );  
/*    sprintf( log_buf, "Scattering.  Checking room %d..", rvnum);
      log_string( log_buf ); */
      if ( pRoomIndex ) 
/*      if ( !xIS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)
      &&   !xIS_SET(pRoomIndex->room_flags, ROOM_SOLITARY)
      &&   !xIS_SET(pRoomIndex->room_flags, ROOM_NO_ASTRAL)
      &&   !xIS_SET(pRoomIndex->room_flags, ROOM_PROTOTYPE) )
      -- still causing problems if every room in range matches
         these flags, removed for now, flag checks aren't necessary
	 for this right now anyway */
        break;
    }
    if ( victim->fighting ) stop_fighting( victim, TRUE );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    victim->position = POS_RESTING;
    do_look( victim, "auto" );
    return;
}

/*
 * syntax: mpslay (character)
 */
void do_mp_slay( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg1 );  
    if ( arg1[0] == '\0' )
    {
       send_to_char( "mpslay whom?\n\r", ch );
       progbug( "Mpslay: invalid (nonexistent?) argument", ch );
       return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Victim must be in room.\n\r", ch );
        progbug( "Mpslay: victim not in room", ch );
	return;
    }

    if ( victim == ch )
    {
        send_to_char( "You try to slay yourself.  You fail.\n\r", ch );
        progbug( "Mpslay: trying to slay self", ch );
	return;
    }

    if ( IS_NPC( victim ) && victim->pIndexData->vnum == 3 )
    {
        send_to_char( "You cannot slay supermob!\n\r", ch );
        progbug( "Mpslay: trying to slay supermob", ch );
	return;
    }

    if( victim->level < LEVEL_IMMORTAL)
    {
       act( AT_IMMORT, "You slay $M in cold blood!",  ch, NULL, victim, TO_CHAR);
       act( AT_IMMORT, "$n slays you in cold blood!", ch, NULL, victim, TO_VICT);
       act( AT_IMMORT, "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT);
       set_cur_char(victim);
       raw_kill( ch, victim );
       stop_fighting( ch, FALSE );
       stop_hating( ch );
       stop_fearing( ch );
       stop_hunting( ch );
    } 
    else 
    {
       act( AT_IMMORT, "You attempt to slay $M and fail!",  ch, NULL, victim, TO_CHAR);
       act( AT_IMMORT, "$n attempts to slay you.  What a kneebiter!", ch, NULL, victim, TO_VICT);
       act( AT_IMMORT, "$n attempts to slay $N.  Needless to say $e fails.",  ch, NULL, victim, TO_NOTVICT);
    }
    return;
}

/*
 * syntax: mpdamage (character) (#hps)
 */
void do_mp_damage( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *nextinroom;
    int dam;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg1 );  
    argument = one_argument( argument, arg2 );  
    if ( arg1[0] == '\0' )
    {
       send_to_char( "mpdamage whom?\n\r", ch );
       progbug( "Mpdamage: invalid argument1", ch );
       return;
    }
/* Am I asking for trouble here or what?  But I need it. -- Blodkai */
    if ( !str_cmp( arg1, "all" ) )
    {
        for ( victim = ch->in_room->first_person; victim; victim = nextinroom )
        {
            nextinroom = victim->next_in_room;
            if ( victim != ch
            &&   can_see( ch, victim ) ) /* Could go either way */
            {
                sprintf( buf, "'%s' %s", victim->name, arg2 );
                do_mp_damage( ch, buf );
            }
        }
        return;
    }
    if ( arg2[0] == '\0' )
    {
       send_to_char( "mpdamage inflict how many hps?\n\r", ch );
       progbug( "Mpdamage: invalid argument2", ch );
       return;
    }
    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Victim must be in room.\n\r", ch );
        progbug( "Mpdamage: victim not in room", ch );
	return;
    }
    if ( victim == ch )
    {
        send_to_char( "You can't mpdamage yourself.\n\r", ch );
        progbug( "Mpdamage: trying to damage self", ch );
	return;
    }
    dam = atoi(arg2);
    if( (dam<0) || (dam>32000) )
    {
       send_to_char( "Mpdamage how much?\n\r", ch );
       progbug( "Mpdamage: invalid (nonexistent?) argument", ch );
       return;
    }
   /* this is kinda begging for trouble        */
   /*
    * Note from Thoric to whoever put this in...
    * Wouldn't it be better to call damage(ch, ch, dam, dt)?
    * I hate redundant code
    */
    if ( simple_damage(ch, victim, dam, TYPE_UNDEFINED ) == rVICT_DIED ) 
    {
	stop_fighting( ch, FALSE );
	stop_hating( ch );
	stop_fearing( ch );
	stop_hunting( ch );
    }
    return;
}

void do_mp_log( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_STRING_LENGTH];
    struct   tm *t = localtime(&current_time);

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    if ( argument[0] == '\0' ) {
        progbug( "Mp_log:  non-existent entry", ch );
        return;
    }
    sprintf( buf, "&p%-2.2d/%-2.2d | %-2.2d:%-2.2d  &P%s:  &p%s",
	t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min,
	ch->short_descr,
	argument );
    append_to_file( MOBLOG_FILE, buf );
    return;
}

/*
 * syntax: mprestore (character) (#hps)                Gorog
 */
void do_mp_restore( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    int hp;

    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg1 );  
    argument = one_argument( argument, arg2 );  

    if ( arg1[0] == '\0' )
    {
       send_to_char( "mprestore whom?\n\r", ch );
       progbug( "Mprestore: invalid argument1", ch );
       return;
    }

    if ( arg2[0] == '\0' )
    {
       send_to_char( "mprestore how many hps?\n\r", ch );
       progbug( "Mprestore: invalid argument2", ch );
       return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Victim must be in room.\n\r", ch );
        progbug( "Mprestore: victim not in room", ch );
	return;
    }

    hp = atoi(arg2);

    if( (hp<0) || (hp>32000) )
    {
       send_to_char( "Mprestore how much?\n\r", ch );
       progbug( "Mprestore: invalid (nonexistent?) argument", ch );
       return;
    }
    hp += victim->hit;
    victim->hit = (hp > 32000 || hp < 0 || hp > victim->max_hit) ?
                  victim->max_hit : hp;
}

/*
 * Syntax mpfavor target number
 * Raise a player's favor in progs.
 */
void  do_mpfavor( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    int favor;
    char *tmp;
    bool plus = FALSE, minus = FALSE;
 
    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
       send_to_char( "mpfavor whom?\n\r", ch );
       progbug( "Mpfavor: invalid argument1", ch );
       return;
    }
 
    if ( arg2[0] == '\0' )
    {
       send_to_char( "mpfavor how much favor?\n\r", ch );
       progbug( "Mpfavor: invalid argument2", ch );
       return;
    }

    tmp = arg2;
    if ( tmp[0] == '+' )
    {
      plus = TRUE;
      tmp++;
      if ( tmp[0] == '\0' )
      {
         send_to_char( "mpfavor how much favor?\n\r", ch );
         progbug( "Mpfavor: invalid argument2", ch );
         return;
      }
    }
    else if ( tmp[0] == '-' )
    {
      minus = TRUE;
      tmp++;
      if ( tmp[0] == '\0' )
      {
         send_to_char( "mpfavor how much favor?\n\r", ch );
         progbug( "Mpfavor: invalid argument2", ch );
         return;
      }
    }
    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Victim must be in room.\n\r", ch );
        progbug( "Mpfavor: victim not in room", ch );
        return;
    }
 
    favor = atoi(tmp);
    if ( plus )
    	victim->pcdata->favor = URANGE( -2500, victim->pcdata->favor + favor, 2500 );
    else if ( minus )
    	victim->pcdata->favor = URANGE( -2500, victim->pcdata->favor - favor, 2500 );
    else
    	victim->pcdata->favor = URANGE( -2500, favor, 2500 );
} 

/*
 * Syntax mp_open_passage x y z
 *
 * opens a 1-way passage from room x to room y in direction z
 *
 *  won't mess with existing exits
 */
void do_mp_open_passage( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char arg3[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *targetRoom, *fromRoom;
    int targetRoomVnum, fromRoomVnum, exit_num;
    EXIT_DATA *pexit;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg1 );  
    argument = one_argument( argument, arg2 );  
    argument = one_argument( argument, arg3 );  

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	progbug( "MpOpenPassage - Bad syntax", ch );
	return;
    }

    if( !is_number(arg1) )
    {
	progbug( "MpOpenPassage - Bad syntax", ch );
	return;
    }

    fromRoomVnum = atoi(arg1);
    if(  (fromRoom = get_room_index( fromRoomVnum ) )  ==NULL)
    {
	progbug( "MpOpenPassage - Bad syntax", ch );
	return;
    }

    if( !is_number(arg2) )
    {
	progbug( "MpOpenPassage - Bad syntax", ch );
	return;
    }

    targetRoomVnum = atoi(arg2);
    if(  (targetRoom = get_room_index( targetRoomVnum ) )  ==NULL)
    {
	progbug( "MpOpenPassage - Bad syntax", ch );
	return;
    }

    if( !is_number(arg3) )
    {
	progbug( "MpOpenPassage - Bad syntax", ch );
	return;
    }

    exit_num = atoi(arg3);
    if( (exit_num < 0) || (exit_num > MAX_DIR) )
    {
	progbug( "MpOpenPassage - Bad syntax", ch );
	return;
    }

    if( (pexit = get_exit( fromRoom, exit_num )) != NULL )
    {
	if( !IS_SET( pexit->exit_info, EX_PASSAGE) )
	  return;
	progbug( "MpOpenPassage - Exit exists", ch );
	return;
    }

    pexit = make_exit( fromRoom, targetRoom, exit_num );
    pexit->keyword 		= STRALLOC( "" );
    pexit->description		= STRALLOC( "" );
    pexit->key     		= -1;
    pexit->exit_info		= EX_PASSAGE;

    /* act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_CHAR ); */
    /* act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_ROOM ); */

    return;
}


/*
 * Syntax mp_fillin x
 * Simply closes the door
 */
void do_mp_fill_in( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  EXIT_DATA *pexit;
  
    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ) || ch->desc )
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

  one_argument( argument, arg );
  
  if ( (pexit = find_door( ch, arg, TRUE )) == NULL  )
  {
	progbug( "MpFillIn - Exit does not exist", ch );
	return;
  }
  SET_BIT( pexit->exit_info, EX_CLOSED );
  return;
}

/*
 * Syntax mp_close_passage x y 
 *
 * closes a passage in room x leading in direction y
 *
 * the exit must have EX_PASSAGE set
 */
void do_mp_close_passage( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char arg3[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *fromRoom;
    int fromRoomVnum, exit_num;
    EXIT_DATA *pexit;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }


    argument = one_argument( argument, arg1 );  
    argument = one_argument( argument, arg2 );  
    argument = one_argument( argument, arg3 );  

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg2[0] == '\0' )
    {
	progbug( "MpClosePassage - Bad syntax", ch );
	return;
    }

    if( !is_number(arg1) )
    {
	progbug( "MpClosePassage - Bad syntax", ch );
	return;
    }

    fromRoomVnum = atoi(arg1);
    if(  (fromRoom = get_room_index( fromRoomVnum ) )  ==NULL)
    {
	progbug( "MpClosePassage - Bad syntax", ch );
	return;
    }

    if( !is_number(arg2) )
    {
	progbug( "MpClosePassage - Bad syntax", ch );
	return;
    }

    exit_num = atoi(arg2);
    if( (exit_num < 0) || (exit_num > MAX_DIR) )
    {
	progbug( "MpClosePassage - Bad syntax", ch );
	return;
    }

    if( ( pexit = get_exit(fromRoom, exit_num) ) == NULL )
    {
	  return;    /* already closed, ignore...  so rand_progs */
		     /*                            can close without spam */
    }

    if( !IS_SET( pexit->exit_info, EX_PASSAGE) )
    {
	progbug( "MpClosePassage - Exit not a passage", ch );
	return;
    }

    extract_exit( fromRoom, pexit );

    /* act( AT_PLAIN, "A passage closes!", ch, NULL, NULL, TO_CHAR ); */
    /* act( AT_PLAIN, "A passage closes!", ch, NULL, NULL, TO_ROOM ); */

    return;
}



/*
 * Does nothing.  Used for scripts.
 */
void do_mpnothing( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    return;
}


/*
 *   Sends a message to sleeping character.  Should be fun
 *    with room sleep_progs
 *
 */
void do_mpdream( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_STRING_LENGTH];
    CHAR_DATA *vict;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }


    argument = one_argument( argument, arg1 );  

    if (  (vict =get_char_world(ch, arg1)) == NULL )
    {
        progbug( "Mpdream: No such character", ch );
        return;
    }
    
    if( vict->position <= POS_SLEEPING)
    {
      send_to_char(argument, vict);
      send_to_char("\n\r",   vict);
    } 
    return;
}

void do_mpapply( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;

    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

  if (argument[0] == '\0')
  {
    progbug("Mpapply - bad syntax", ch );
    return;
  }

  if ( (victim = get_char_room( ch, argument ) ) == NULL )
  {
    progbug("Mpapply - no such player in room.", ch );
    return;
  }

  if ( !victim->desc )
  {
   send_to_char( "Not on linkdeads.\n\r", ch );
   return;
  }

  if( !NOT_AUTHED(victim) )
    return;
  
  if( victim->pcdata->auth_state >= 1 )
    return;

  sprintf( log_buf, "%s@%s new %s %s %s applying...", 
           victim->name, victim->desc->host, 
           race_table[victim->race]->race_name, 
           class_table[victim->class]->who_name,
	   IS_PKILL( victim ) ? "(Deadly)" : "(Peaceful)" );
/*  log_string( log_buf );*/
  to_channel( log_buf, CHANNEL_AUTH, "Auth", LEVEL_IMMORTAL );
  victim->pcdata->auth_state = 1;
  return;
}

void do_mpapplyb( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;


    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

  if (argument[0] == '\0')
  {
    progbug("Mpapplyb - bad syntax", ch );
    return;
  }

  if ( (victim = get_char_room( ch, argument ) ) == NULL )
  {
    progbug("Mpapplyb - no such player in room.", ch );
    return;
  }

  if ( !victim->desc )
  {
   send_to_char( "Not on linkdeads.\n\r", ch );
   return;
  }

  if( !NOT_AUTHED(victim) )
    return;
  
  if ( get_timer(victim, TIMER_APPLIED) >= 1)
    return;

  switch( victim->pcdata->auth_state )
  {
  case 0:
  case 1:   
  default:
  send_to_char( "You attempt to regain the gods' attention.\n\r", victim);
  sprintf( log_buf, "%s@%s new %s %s %s applying...",                      
                    victim->name, victim->desc->host,
                    race_table[victim->race]->race_name, 
                    class_table[victim->class]->who_name,
		    IS_PKILL( victim ) ? "(Deadly)" : "(Peaceful)" );
  log_string( log_buf );
  to_channel( log_buf, CHANNEL_AUTH, "Auth", LEVEL_IMMORTAL );
  add_timer(victim, TIMER_APPLIED, 10, NULL, 0);
  victim->pcdata->auth_state = 1;
  break; 

  case 2:
  send_to_char("Your name has been deemed unsuitable by the gods.  Please choose a more medieval name with the 'name' command.\n\r", victim);
  add_timer(victim, TIMER_APPLIED, 10, NULL, 0);
  break;

  case 3:
  ch_printf( victim, "The gods permit you to enter the %s.\n\r", sysdata.mud_name);
/*  send_to_char( "The gods permit you to enter the Realms of Despair.\n\r", victim);  */
/* Stat stuff that follows from here to name_stamp is to keep stat-cheats
   from abusing the auth system -- Blodkai */
        victim->perm_str                = 13;
        victim->perm_dex                = 13;
        victim->perm_int                = 13;
        victim->perm_wis                = 13;
        victim->perm_cha                = 13;
        victim->perm_con                = 13;
        victim->perm_lck                = 13;
        switch ( class_table[victim->class]->attr_prime )
        {
          case APPLY_STR: victim->perm_str = 16; break;
          case APPLY_INT: victim->perm_int = 16; break;
          case APPLY_WIS: victim->perm_wis = 16; break;
          case APPLY_DEX: victim->perm_dex = 16; break;
          case APPLY_CON: victim->perm_con = 16; break;
          case APPLY_CHA: victim->perm_cha = 16; break;
          case APPLY_LCK: victim->perm_lck = 16; break;
        }
        switch ( class_table[ch->class]->attr_second )
        {
        case APPLY_STR: ch->perm_str = 14; break;
        case APPLY_INT: ch->perm_int = 14; break;
        case APPLY_WIS: ch->perm_wis = 14; break;
        case APPLY_DEX: ch->perm_dex = 14; break;
        case APPLY_CON: ch->perm_con = 14; break;
        case APPLY_CHA: ch->perm_cha = 14; break;
        case APPLY_LCK: ch->perm_lck = 14; break;
        }
        switch ( class_table[ch->class]->attr_deficient )
        {
        case APPLY_STR: ch->perm_str = 9; break;
        case APPLY_INT: ch->perm_int = 9; break;
        case APPLY_WIS: ch->perm_wis = 9; break;
        case APPLY_DEX: ch->perm_dex = 9; break;
        case APPLY_CON: ch->perm_con = 9; break;
        case APPLY_CHA: ch->perm_cha = 9; break;
        case APPLY_LCK: ch->perm_lck = 9; break;
        }


        victim->perm_str         += race_table[victim->race]->str_plus;
        victim->perm_int         += race_table[victim->race]->int_plus;
        victim->perm_wis         += race_table[victim->race]->wis_plus;
        victim->perm_dex         += race_table[victim->race]->dex_plus;
        victim->perm_con         += race_table[victim->race]->con_plus;
        victim->perm_cha         += race_table[victim->race]->cha_plus;
        victim->perm_lck         += race_table[victim->race]->lck_plus;
	name_stamp_stats( victim ); /* Take care of auth cons -- Blodkai */
        REMOVE_BIT(victim->pcdata->flags, PCFLAG_UNAUTHED);
        if ( victim->fighting )
          stop_fighting( victim, TRUE );
        char_from_room(victim);
        char_to_room(victim, get_room_index(ROOM_VNUM_SCHOOL));
        act( AT_WHITE, "$n enters this world from within a column of blinding light!",
            victim, NULL, NULL, TO_ROOM );
	victim->gold = 50000;
        do_look(victim, "auto");
  break;
  }

  return;
}

/*
 * Deposit some gold into the current area's economy		-Thoric
 */
void do_mp_deposit( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_STRING_LENGTH];
    int gold;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    one_argument(argument, arg);

    if ( arg[0] == '\0' )
    {
	progbug("Mpdeposit - bad syntax", ch );
	return;
    }
    gold = atoi( arg );
    if ( gold <= ch->gold && ch->in_room )
    {
	ch->gold -= gold;
	boost_economy( ch->in_room->area, gold );
    }
}


/*
 * Withdraw some gold from the current area's economy		-Thoric
 */
void do_mp_withdraw( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_STRING_LENGTH];
    int gold;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }


    one_argument(argument, arg);

    if ( arg[0] == '\0' )
    {
	progbug("Mpwithdraw - bad syntax", ch );
	return;
    }
    gold = atoi( arg );
    if ( ch->gold < 1000000000 && gold < 1000000000 && ch->in_room
    &&   economy_has( ch->in_room->area, gold ) )
    {
	ch->gold += gold;
	lower_economy( ch->in_room->area, gold );
    }
}

void do_mpdelay( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int delay;
 
    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg );
    if ( !*arg ) {
      send_to_char( "Delay for how many rounds?n\r", ch );
      progbug( "Mpdelay: no duration specified", ch );
      return;
    }
    if ( !( victim = get_char_room( ch, arg ) ) ) {
      send_to_char( "They aren't here.\n\r", ch );
      progbug( "Mpdelay: target not in room", ch );
      return;
    }
    if ( IS_IMMORTAL( victim ) ) {
      send_to_char( "Not against immortals.\n\r", ch );
      progbug( "Mpdelay: target is immortal", ch );
      return;
    }
    argument = one_argument(argument, arg);
    if ( !*arg || !is_number(arg) ) {
      send_to_char( "Delay them for how many rounds?\n\r", ch );
      progbug( "Mpdelay: invalid (nonexistant?) argument", ch );
      return;
    }
    delay = atoi( arg );
    if ( delay < 1 || delay > 30 ) {
      send_to_char( "Argument out of range.\n\r", ch );
      progbug( "Mpdelay:  argument out of range (1 to 30)", ch );
      return;
    }
    WAIT_STATE( victim, delay * PULSE_VIOLENCE );
    send_to_char( "Mpdelay applied.\n\r", ch );
    return;
}

void do_mppeace( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    CHAR_DATA *victim;
 
    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg );
    if ( !*arg ) {
      send_to_char( "Who do you want to mppeace?\n\r", ch );
      progbug( "Mppeace: invalid (nonexistent?) argument", ch );
      return;
    }
    if ( !str_cmp( arg, "all" ) ) {
      for ( rch=ch->in_room->first_person; rch; rch=rch->next_in_room ) {
        if ( rch->fighting ) {
          stop_fighting( rch, TRUE );
          do_sit( rch, "" );
        }
        stop_hating( rch );
        stop_hunting( rch );
        stop_fearing( rch );
      }
      send_to_char( "Ok.\n\r", ch );
      return;
    }
    if ( ( victim = get_char_room( ch, arg ) ) == NULL ) {
      send_to_char( "They must be in the room.n\r", ch );
      progbug( "Mppeace: target not in room", ch );
      return;
    }
    if ( victim->fighting )
      stop_fighting( victim, TRUE );
    stop_hating( ch ); stop_hunting( ch ); stop_fearing( ch );
    stop_hating( victim ); stop_hunting( victim ); stop_fearing( victim );
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_mppkset( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char arg[MAX_STRING_LENGTH];

    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

  argument = one_argument(argument, arg);

  if (argument[0] == '\0' || arg[0] == '\0')
  {
    progbug("Mppkset - bad syntax", ch );
    return;
  }

  if ( (victim = get_char_room( ch, arg ) ) == NULL )
  {
    progbug("Mppkset - no such player in room.", ch );
    return;
  }

  if (!str_cmp(argument, "yes") || !str_cmp(argument, "y"))
  {
    if(!IS_SET(victim->pcdata->flags, PCFLAG_DEADLY))
      SET_BIT(victim->pcdata->flags, PCFLAG_DEADLY);
  }
  else if (!str_cmp(argument, "no") || !str_cmp(argument, "n"))
  {
    if(IS_SET(victim->pcdata->flags, PCFLAG_DEADLY))
      REMOVE_BIT(victim->pcdata->flags, PCFLAG_DEADLY);
  }
  else
  {
    progbug("Mppkset - bad syntax", ch);
    return;
  }
  return;
}

/*
 * Make an object owned by a char. --Shaddai
 */

void do_mpoowner( CHAR_DATA *ch, char *argument )
 {
   OBJ_DATA *obj;
   CHAR_DATA *victim=NULL;
   char arg1[MAX_STRING_LENGTH];
   char arg2[MAX_STRING_LENGTH];

    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if ( arg1[0] == '\0' || arg2[0] == '\0' )
   {
        send_to_char("Syntax: oowner <object> <player>\n\r", ch );
        return;
   }

   if ( str_cmp( arg2, "none") &&(victim = get_char_room( ch, arg2 ) ) == NULL )
   {
        send_to_char("No such player is in the room.\n\r", ch );
        return;
   }

   if ( ( obj = get_obj_here( ch, arg1) ) == NULL )
   {
        send_to_char("No such object exists.\n\r", ch );
        return;
   }

   separate_obj( obj );

   if ( !str_cmp( "none", arg2) )
   {
     STRFREE( obj->owner );
     obj->owner = STRALLOC("");
     send_to_char("Done.\n\r", ch );
     return;
   }

   if ( IS_NPC(victim) )
   {
        send_to_char("A mob can't be an owner of an item.\n\r", ch );
        return;
   }
   STRFREE( obj->owner );
   obj->owner = STRALLOC( victim->name );
   send_to_char("Done.\n\r", ch );
   return;
}


/*
 * Inflict damage from a mudprogram
 *
 *  note: should be careful about using victim afterwards
 */
ch_ret simple_damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
    sh_int dameq;
    bool npcvict;
    OBJ_DATA *damobj;
    ch_ret retcode;


    retcode = rNONE;

    if ( !ch )
    {
	bug( "Damage: null ch!", 0 );
	return rERROR;
    }
    if ( !victim )
    {
	progbug( "Damage: null victim!", ch );
	return rVICT_DIED;
    }

    if ( victim->position == POS_DEAD )
    {
	return rVICT_DIED;
    }

    npcvict = IS_NPC(victim);

    if ( dam )
    {
	if ( IS_FIRE(dt) )
	  dam = ris_damage(victim, dam, RIS_FIRE);
	else
	if ( IS_COLD(dt) )
	  dam = ris_damage(victim, dam, RIS_COLD);
	else
	if ( IS_ACID(dt) )
	  dam = ris_damage(victim, dam, RIS_ACID);
	else
	if ( IS_ELECTRICITY(dt) )
	  dam = ris_damage(victim, dam, RIS_ELECTRICITY);
	else
	if ( IS_ENERGY(dt) )
	  dam = ris_damage(victim, dam, RIS_ENERGY);
	else
	if ( dt == gsn_poison )
	  dam = ris_damage(victim, dam, RIS_POISON);
	else
	if ( dt == (TYPE_HIT + 7) || dt == (TYPE_HIT + 8) )
	  dam = ris_damage(victim, dam, RIS_BLUNT);
	else
	if ( dt == (TYPE_HIT + 2) || dt == (TYPE_HIT + 11) )
	  dam = ris_damage(victim, dam, RIS_PIERCE);
	else
	if ( dt == (TYPE_HIT + 1) || dt == (TYPE_HIT + 3) )
	  dam = ris_damage(victim, dam, RIS_SLASH);
	if ( dam < 0 )
	  dam = 0;
    }

    if ( victim != ch )
    {
	/*
	 * Damage modifiers.
	 */
	if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
	    dam /= 2;

	if ( IS_AFFECTED(victim, AFF_PROTECT) && IS_EVIL(ch) )
	    dam -= (int) (dam / 4);

	if ( dam < 0 )
	    dam = 0;

	/* dam_message( ch, victim, dam, dt ); */
    }

    /*
     * Check for EQ damage.... ;)
     */

    if (dam > 10)
    {
	/* get a random body eq part */
	dameq  = number_range(WEAR_LIGHT, WEAR_EYES);
	damobj = get_eq_char(victim, dameq);
	if ( damobj )
	{
	  if ( dam > get_obj_resistance(damobj) )
	  {
	     set_cur_obj(damobj);
	     damage_obj(damobj);
	  }
	}
    }

   /*
    * Hurt the victim.
    * Inform the victim of his new state.
    */
    victim->hit -= dam;
    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
    victim->hit = 1;

    if ( !npcvict
    &&   get_trust(victim) >= LEVEL_IMMORTAL
    &&	 get_trust(ch)	   >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
	victim->hit = 1;
    update_pos( victim );

    switch( victim->position )
    {
    case POS_MORTAL:
    act( AT_DYING, "$n is mortally wounded, and will die soon, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
    act( AT_DANGER, "You are mortally wounded, and will die soon, if not aided.",
	victim, NULL, NULL, TO_CHAR );
	break;

    case POS_INCAP:
    act( AT_DYING, "$n is incapacitated and will slowly die, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
    act( AT_DANGER, "You are incapacitated and will slowly die, if not aided.",
	victim, NULL, NULL, TO_CHAR );
	break;

    case POS_STUNNED:
        if ( !IS_AFFECTED( victim, AFF_PARALYSIS ) )
        {
    act( AT_ACTION, "$n is stunned, but will probably recover.",
	    victim, NULL, NULL, TO_ROOM );
    act( AT_HURT, "You are stunned, but will probably recover.",
	victim, NULL, NULL, TO_CHAR );
	}
	break;

    case POS_DEAD:
    act( AT_DEAD, "$n is DEAD!!", victim, 0, 0, TO_ROOM );
    act( AT_DEAD, "You have been KILLED!!\n\r", victim, 0, 0, TO_CHAR );
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	act( AT_HURT, "That really did HURT!", victim, 0, 0, TO_CHAR );
	if ( victim->hit < victim->max_hit / 4 )
	act( AT_DANGER, "You wish that your wounds would stop BLEEDING so much!",
	 victim, 0, 0, TO_CHAR );
	break;
    }

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
	if ( !npcvict )
	{
	    sprintf( log_buf, "%s (%d) killed by %s at %d",
		victim->name,
		victim->level,
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		victim->in_room->vnum );
	    log_string_plus( log_buf, LOG_ALL, 65 );
/*
            append_to_file( LOG_FILE, log_buf );
*/
            to_channel( log_buf, CHANNEL_DEATH, "Death", LEVEL_IMMORTAL );
	    /*
	     * Dying penalty:
	     * 1/2 way back to previous level.
	     */
	    if ( victim->exp > exp_level(victim, victim->level) )
		gain_exp( victim, (exp_level(victim, victim->level) - victim->exp)/2 );

	    /*
	     * New penalty... go back to the beginning of current level.
	     victim->exp = exp_level( victim, victim->level );
	     */
	}
	set_cur_char(victim);
	raw_kill( ch, victim );
	victim = NULL;

	return rVICT_DIED;
    }

    if ( victim == ch )
	return rNONE;

    /*
     * Take care of link dead people.
     */
    if ( !npcvict && !victim->desc )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    do_recall( victim, "" );
	    return rNONE;
	}
    }

    /*
     * Wimp out?
     */
    if ( npcvict && dam > 0 )
    {
	if ( ( xIS_SET(victim->act, ACT_WIMPY) && number_bits( 1 ) == 0
	&&   victim->hit < victim->max_hit / 2 )
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master
	&&     victim->master->in_room != victim->in_room ) )
	{
	    start_fearing( victim, ch );
	    stop_hunting( victim );
	    do_flee( victim, "" );
	}
    }

    if ( !npcvict
    &&   victim->hit > 0
    &&   victim->hit <= victim->wimpy
    &&   victim->wait == 0 )
	do_flee( victim, "" );
    else
    if ( !npcvict && xIS_SET( victim->act, PLR_FLEE ) )
	do_flee( victim, "" );

    tail_chain( );
    return rNONE;
}

/* mphunt/mphate - by Leart 09/01 */
void do_mphunt( CHAR_DATA *ch, char *argument)
{
     char arg[MAX_INPUT_LENGTH];
     CHAR_DATA *victim;


    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
       send_to_char( "Which player to hunt?\n\r", ch );
       progbug( "Mphunt: invalid argument", ch );
       return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
     progbug( "Mphunt - No such person", ch);
     return;
    }


    start_hunting( ch, victim );
    return;
}

void do_mphate( CHAR_DATA *ch, char *argument)
{
     char arg[MAX_INPUT_LENGTH];
     CHAR_DATA *victim;

    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
       send_to_char( "Which player to hate?\n\r", ch );
       progbug( "Mphate: invalid argument", ch );
       return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
     progbug( "Mphate - No such person", ch);
     return;
    }

    start_hating( ch, victim );
    return;
}
