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
 *			    Battle & death module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"



extern char		lastplayercmd[MAX_INPUT_LENGTH];
extern CHAR_DATA *	gch_prev;

OBJ_DATA *used_weapon;   /* Used to figure out which weapon later */

/*
 * Local functions.
 */
void	new_dam_message	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			    int dt, OBJ_DATA *obj ) );
void	death_cry	args( ( CHAR_DATA *ch ) );
void	group_gain	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int	xp_compute	args( ( CHAR_DATA *gch, CHAR_DATA *victim ) );
int	align_compute	args( ( CHAR_DATA *gch, CHAR_DATA *victim ) );
ch_ret	one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
int	obj_hitroll	args( ( OBJ_DATA *obj ) );
void    show_condition  args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );


/*
 * Check to see if player's attacks are (still?) suppressed
 * #ifdef TRI
 */
bool is_attack_supressed( CHAR_DATA *ch )
{
  TIMER *timer;

  if (IS_NPC(ch))
    return FALSE;

  if ( IS_AFFECTED( ch, AFF_GRAPPLE ) )
    return TRUE;

  timer = get_timerptr( ch, TIMER_ASUPRESSED );

  if ( !timer )
    return FALSE;

  /* perma-supression -- bard? (can be reset at end of fight, or spell, etc) */
  if ( timer->value == -1 )
    return TRUE;

  /* this is for timed supressions */
  if ( timer->count >= 1 )
    return TRUE;

  return FALSE;
}
  
/*
 * Check to see if weapon is poisoned.
 */
bool is_wielding_poisoned( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ( !used_weapon )
    	return FALSE;

    if ( (obj=get_eq_char(ch, WEAR_WIELD)) != NULL
    &&    used_weapon == obj
    &&    IS_OBJ_STAT(obj, ITEM_POISONED) )
	return TRUE;
    if ( (obj=get_eq_char(ch, WEAR_DUAL_WIELD)) != NULL
    &&    used_weapon == obj
    &&	  IS_OBJ_STAT(obj, ITEM_POISONED) )
    	return TRUE;

    return FALSE;
}

/*
 * hunting, hating and fearing code				-Thoric
 */
bool is_hunting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->hunting || ch->hunting->who != victim )
      return FALSE;
    
    return TRUE;    
}

bool is_hating( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->hating || ch->hating->who != victim )
      return FALSE;
    
    return TRUE;    
}

bool is_fearing( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->fearing || ch->fearing->who != victim )
      return FALSE;
    
    return TRUE;    
}

void stop_hunting( CHAR_DATA *ch )
{
    if ( ch->hunting )
    {
	STRFREE( ch->hunting->name );
	DISPOSE( ch->hunting );
	ch->hunting = NULL;
    }
    return;
}

void strip_grapple( CHAR_DATA *ch )
{
    if ( IS_NPC( ch ) )
	return;

    if ( IS_AFFECTED( ch, AFF_GRAPPLE ) )
    {
        affect_strip( ch, gsn_grapple );
        set_char_color( AT_WEAROFF, ch );
        send_to_char( skill_table[gsn_grapple]->msg_off, ch );
        send_to_char( "\n\r", ch );
	if ( get_timer( ch, TIMER_PKILLED ) > 0 )
	  remove_timer( ch, TIMER_ASUPRESSED );
    }
    if ( who_fighting( ch ) )
    {
	if ( who_fighting( ch->fighting->who ) == ch 
	&&   IS_AFFECTED( who_fighting( ch ), AFF_GRAPPLE ) )
	{
	  affect_strip( who_fighting( ch ), gsn_grapple );
          set_char_color( AT_WEAROFF, who_fighting( ch ) );
          send_to_char( skill_table[gsn_grapple]->msg_off, who_fighting( ch ) );
          send_to_char( "\n\r", who_fighting( ch ) );
          if ( get_timer( who_fighting( ch ), TIMER_PKILLED ) > 0 )
	   remove_timer( who_fighting( ch ), TIMER_ASUPRESSED );
	}
    }
    return;
}

void stop_hating( CHAR_DATA *ch )
{
    if ( ch->hating )
    {
	STRFREE( ch->hating->name );
	DISPOSE( ch->hating );
	ch->hating = NULL;
    }
    return;
}

void stop_fearing( CHAR_DATA *ch )
{
    if ( ch->fearing )
    {
	STRFREE( ch->fearing->name );
	DISPOSE( ch->fearing );
	ch->fearing = NULL;
    }
    return;
}

void start_hunting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->hunting )
      stop_hunting( ch );

    CREATE( ch->hunting, HHF_DATA, 1 );
    ch->hunting->name = QUICKLINK( victim->name );
    ch->hunting->who  = victim;
    return;
}

void start_hating( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->hating )
      stop_hating( ch );

    CREATE( ch->hating, HHF_DATA, 1 );
    ch->hating->name = QUICKLINK( victim->name );
    ch->hating->who  = victim;
    return;
}

void start_fearing( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->fearing )
      stop_fearing( ch );

    CREATE( ch->fearing, HHF_DATA, 1 );
    ch->fearing->name = QUICKLINK( victim->name );
    ch->fearing->who  = victim;
    return;
}

/*
 * Get the current armor class for a vampire based on time of day
 */
sh_int VAMP_AC( CHAR_DATA * ch )
{
  if ( IS_VAMPIRE( ch ) && IS_OUTSIDE( ch ) )
  {
    switch(time_info.sunlight)
    {
    case SUN_DARK:
      return -10;
    case SUN_RISE:
      return 5;
    case SUN_LIGHT:
      return 10;
    case SUN_SET:
      return 2;
    default:
      return 0;
    }
  }
  else
    return 0;
}

int max_fight( CHAR_DATA *ch )
{
    return 8;
}






/*
 * Control the fights going on.
 * Called periodically by update_handler.
 * Many hours spent fixing bugs in here by Thoric, as noted by residual
 * debugging checks.  If you never get any of these error messages again
 * in your logs... then you can comment out some of the checks without
 * worry.
 *
 * Note:  This function also handles some non-violence updates.
 */
void violence_update( void )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *ch;
    CHAR_DATA *lst_ch;
    CHAR_DATA *victim;
    CHAR_DATA *rch, *rch_next;
    AFFECT_DATA *paf, *paf_next;
    TIMER	*timer, *timer_next;
    ch_ret	retcode;
    int		attacktype, cnt;
    SKILLTYPE	*skill;
    static int	pulse = 0;

    lst_ch = NULL;
    pulse = (pulse+1) % 100;

    for ( ch = last_char; ch; lst_ch = ch, ch = gch_prev )
    {
	set_cur_char( ch );

	if ( ch == first_char && ch->prev )
	{
	   bug( "ERROR: first_char->prev != NULL, fixing...", 0 );
	   ch->prev = NULL;
	}

	gch_prev	= ch->prev;

	if ( gch_prev && gch_prev->next != ch )
	{
	    sprintf( buf, "FATAL: violence_update: %s->prev->next doesn't point to ch.",
		ch->name );
	    bug( buf, 0 );	    
	    bug( "Short-cutting here", 0 );
	    ch->prev = NULL;
	    gch_prev = NULL;
	    do_shout( ch, "Thoric says, 'Prepare for the worst!'" );
	}

	/*
	 * See if we got a pointer to someone who recently died...
	 * if so, either the pointer is bad... or it's a player who
	 * "died", and is back at the healer...
	 * Since he/she's in the char_list, it's likely to be the later...
	 * and should not already be in another fight already
	 */
	if ( char_died(ch) )
	    continue;

	/*
	 * See if we got a pointer to some bad looking data...
	 */
	if ( !ch->in_room || !ch->name )
	{
	    log_string( "violence_update: bad ch record!  (Shortcutting.)" );
	    sprintf( buf, "ch: %d  ch->in_room: %d  ch->prev: %d  ch->next: %d",
	    	(int) ch, (int) ch->in_room, (int) ch->prev, (int) ch->next );
	    log_string( buf );
	    log_string( lastplayercmd );
	    if ( lst_ch )
	      sprintf( buf, "lst_ch: %d  lst_ch->prev: %d  lst_ch->next: %d",
	      		(int) lst_ch, (int) lst_ch->prev, (int) lst_ch->next );
	    else
	      strcpy( buf, "lst_ch: NULL" );
	    log_string( buf );
	    gch_prev = NULL;
	    continue;
	}

        /*
         * Experience gained during battle deceases as battle drags on
         */
	if ( ch->fighting )
	  if ( (++ch->fighting->duration % 24) == 0 )
	    ch->fighting->xp = ((ch->fighting->xp * 9) / 10);


	for ( timer = ch->first_timer; timer; timer = timer_next )
	{
	    timer_next = timer->next;
		
		if ( ch->fighting && timer->type == TIMER_DO_FUN )
		{
		    int tempsub;
		    
		    tempsub = ch->substate;
		    ch->substate = SUB_TIMER_DO_ABORT;
		    (timer->do_fun)(ch,"");
		    if ( char_died(ch) )
		        break;
	        ch->substate = tempsub;
    	    extract_timer( ch, timer );
			continue;
   		}

	    if ( --timer->count <= 0 )
	    {
		if ( timer->type == TIMER_ASUPRESSED )
		{
		    if ( timer->value == -1 )
		    {
			timer->count = 1000;
			continue;
		    }
		}
		if ( timer->type == TIMER_NUISANCE )
		{
		    DISPOSE( ch->pcdata->nuisance );
		}

		if ( timer->type == TIMER_DO_FUN )
		{
		    int tempsub;

		    tempsub = ch->substate;
		    ch->substate = timer->value;
		    (timer->do_fun)( ch, "" );
		    if ( char_died(ch) )
			break;
		    ch->substate = tempsub;
            if ( timer->count > 0 )
               continue;
		}
		extract_timer( ch, timer );
	    }
	}

	if ( char_died(ch) )
	    continue;

	/*
	 * We need spells that have shorter durations than an hour.
	 * So a melee round sounds good to me... -Thoric
	 */
	for ( paf = ch->first_affect; paf; paf = paf_next )
	{
	    paf_next	= paf->next;
	    if ( paf->duration > 0 )
		paf->duration--;
	    else
	    if ( paf->duration < 0 )
		;
	    else
	    {
		if ( !paf_next
		||    paf_next->type != paf->type
		||    paf_next->duration > 0 )
		{
		    skill = get_skilltype(paf->type);
		    if ( paf->type > 0 && skill && skill->msg_off )
		    {
			set_char_color( AT_WEAROFF, ch );
			send_to_char( skill->msg_off, ch );
			send_to_char( "\n\r", ch );
		    }
		}
		if (paf->type == gsn_possess)
	        {
		    ch->desc->character = ch->desc->original;
		    ch->desc->original  = NULL;
		    ch->desc->character->desc = ch->desc;
		    ch->desc->character->switched = NULL;
		    ch->desc            = NULL;
		}
		affect_remove( ch, paf );
	    }
	}
	
	if ( char_died(ch) )
	    continue;

	/* check for exits moving players around */
	if ( (retcode=pullcheck(ch, pulse)) == rCHAR_DIED || char_died(ch) )
	    continue;

	/* Let the battle begin! */

	if ( ( victim = who_fighting( ch ) ) == NULL
	||   IS_AFFECTED( ch, AFF_PARALYSIS ) )
	    continue;

        retcode = rNONE;

	if ( xIS_SET(ch->in_room->room_flags, ROOM_SAFE ) )
	{
	   sprintf( buf, "violence_update: %s fighting %s in a SAFE room.",
	   	ch->name, victim->name );
	   log_string( buf );
	   stop_fighting( ch, TRUE );
	}
	else
	if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
	    retcode = multi_hit( ch, victim, TYPE_UNDEFINED );
	else
	    stop_fighting( ch, FALSE );

	if ( char_died(ch) )
	    continue;

	if ( retcode == rCHAR_DIED
	|| ( victim = who_fighting( ch ) ) == NULL )
	    continue;

/* Zednakk patch - remove next check */
	if ( !str_cmp( ch->name, "zednakk" ) )
	{
	    act( AT_DGREEN, "$N rips your head clean from your shoulders.  As your spinal cord dangles limply in the breeze, the old adage echoes in your dying thoughts 'Be careful what you wish for!'", ch, NULL, victim, TO_CHAR );
	    act( AT_DGREEN, "Spurts of thick green sarcasm erupt from Zednakk as his head is sliced clean from his shoulders.",  ch, NULL, victim, TO_NOTVICT );
	    do_mp_slay( victim, "zednakk" );
	    if ( char_died( ch ) || char_died( victim ) )
	      continue;
	}

	/*
	 *  Mob triggers
	 *  -- Added some victim death checks, because it IS possible.. -- Alty
	 */
	rprog_rfight_trigger( ch );
	if ( char_died(ch) || char_died(victim) )
	    continue;
	mprog_hitprcnt_trigger( ch, victim );
	if ( char_died(ch) || char_died(victim) )
	    continue;
	mprog_fight_trigger( ch, victim );
	if ( char_died(ch) || char_died(victim) )
	    continue;

	/*
	 * NPC special attack flags				-Thoric
	 */
	if ( IS_NPC(ch) ) 
	{
	    if ( !xIS_EMPTY(ch->attacks) )
	    {
		attacktype = -1;
		if ( 30 + (ch->level/4) >= number_percent( ) )
		{
		    cnt = 0;
		    for ( ;; )
		    {
			if ( cnt++ > 10 )
			{
			    attacktype = -1;
			    break;
			}
			attacktype = number_range( 7, MAX_ATTACK_TYPE-1 );
			if ( xIS_SET( ch->attacks, attacktype ) )
			    break;
		    }
		    switch( attacktype )
		    {
			case ATCK_BASH:
			    do_bash( ch, "" );
			    retcode = global_retcode;
			    break;
			case ATCK_STUN:
			    do_stun( ch, "" ); 
			    retcode = global_retcode;
			    break;
			case ATCK_GOUGE:
			    do_gouge( ch, "" );
			    retcode = global_retcode;
			    break;
			case ATCK_FEED:
			    do_gouge( ch, "" );
			    retcode = global_retcode;
			    break;
			case ATCK_DRAIN:
			    retcode = spell_energy_drain( skill_lookup( "energy drain" ), ch->level, ch, victim );
			    break;
			case ATCK_FIREBREATH:
			    retcode = spell_fire_breath( skill_lookup( "fire breath" ), ch->level, ch, victim );
			    break;
			case ATCK_FROSTBREATH:
			    retcode = spell_frost_breath( skill_lookup( "frost breath" ), ch->level, ch, victim );
			    break;
			case ATCK_ACIDBREATH:
			    retcode = spell_acid_breath( skill_lookup( "acid breath" ), ch->level, ch, victim );
			    break;
			case ATCK_LIGHTNBREATH:
			    retcode = spell_lightning_breath( skill_lookup( "lightning breath" ), ch->level, ch, victim );
			    break;
			case ATCK_GASBREATH:
			    retcode = spell_gas_breath( skill_lookup( "gas breath" ), ch->level, ch, victim );
			    break;
			case ATCK_SPIRALBLAST:         
			    retcode = spell_spiral_blast( skill_lookup( "spiral blast" ),
				ch->level, ch, victim );
			    break;
			case ATCK_POISON:
			    retcode = spell_poison( gsn_poison, ch->level, ch, victim );
			    break;
			case ATCK_NASTYPOISON:
			    /*
			    retcode = spell_nasty_poison( skill_lookup( "nasty poison" ), ch->level, ch, victim );
			    */
			    break;
			case ATCK_GAZE:
			    /*
			    retcode = spell_gaze( skill_lookup( "gaze" ), ch->level, ch, victim );
			    */
			    break;
			case ATCK_BLINDNESS:
			    retcode = spell_blindness( gsn_blindness, ch->level, ch, victim );
			    break;
			case ATCK_CAUSESERIOUS:
			    retcode = spell_cause_serious( skill_lookup( "cause serious" ), ch->level, ch, victim );
			    break;
			case ATCK_EARTHQUAKE:
			    retcode = spell_earthquake( skill_lookup( "earthquake" ), ch->level, ch, victim );
			    break;
			case ATCK_CAUSECRITICAL:
			    retcode = spell_cause_critical( skill_lookup( "cause critical" ), ch->level, ch, victim );
			    break;
			case ATCK_CURSE:
			    retcode = spell_curse( skill_lookup( "curse" ), ch->level, ch, victim );
			    break;
			case ATCK_FLAMESTRIKE:
			    retcode = spell_flamestrike( skill_lookup( "flamestrike" ), ch->level, ch, victim );
			    break;
			case ATCK_HARM:
			    retcode = spell_harm( skill_lookup( "harm" ), ch->level, ch, victim );
			    break;
			case ATCK_FIREBALL:
			    retcode = spell_fireball( skill_lookup( "fireball" ), ch->level, ch, victim );
			    break;
			case ATCK_COLORSPRAY:
			    retcode = spell_colour_spray( skill_lookup( "colour spray" ), ch->level, ch, victim );
			    break;
			case ATCK_WEAKEN:
			    retcode = spell_weaken( skill_lookup( "weaken" ), ch->level, ch, victim );
			    break;
		    }
		    if ( attacktype != -1 && (retcode == rCHAR_DIED || char_died(ch)) )
			continue;
		}
	    }
	    /*
	     * NPC special defense flags				-Thoric
	     */
	    if ( !xIS_EMPTY(ch->defenses) )
	    {
	    	/* Fix for character not here bugs --Shaddai */
	        if ( char_died(ch) || char_died(victim) )
	    	  continue;

		attacktype = -1;
		if ( 50 + (ch->level/4) > number_percent( ) )
		{
		    cnt = 0;
		    for ( ;; )
		    {
			if ( cnt++ > 10 )
			{
			    attacktype = -1;
			    break;
			}
			attacktype = number_range( 2, MAX_DEFENSE_TYPE-1 );
			if ( xIS_SET( ch->defenses, attacktype ) )
			    break;
		    }

		    switch( attacktype )
		    {
			case DFND_CURELIGHT:
			    /* A few quick checks in the cure ones so that a) less spam and
			       b) we don't have mobs looking stupider than normal by healing
			       themselves when they aren't even being hit (although that
			       doesn't happen TOO often */
			    if ( ch->hit < ch->max_hit )
			    {
				act( AT_MAGIC, "$n mutters a few incantations...and looks a little better.", ch, NULL, NULL, TO_ROOM );
				retcode = spell_smaug( skill_lookup( "cure light" ), ch->level, ch, ch );
			    }
			    break;
			case DFND_CURESERIOUS:
			    if ( ch->hit < ch->max_hit )
			    {
				act( AT_MAGIC, "$n mutters a few incantations...and looks a bit better.", ch, NULL, NULL, TO_ROOM );
				retcode = spell_smaug( skill_lookup( "cure serious" ), ch->level, ch, ch );
			    }
			    break;
			case DFND_CURECRITICAL:
			    if ( ch->hit < ch->max_hit )
			    {
				act( AT_MAGIC, "$n mutters a few incantations...and looks healthier.", ch, NULL, NULL, TO_ROOM );
				retcode = spell_smaug( skill_lookup( "cure critical" ), ch->level, ch, ch );
			    }
			    break;
			case DFND_HEAL:
			    if ( ch->hit < ch->max_hit )
			    {
				act( AT_MAGIC, "$n mutters a few incantations...and looks much healthier.", ch, NULL, NULL, TO_ROOM );
				retcode = spell_smaug( skill_lookup( "heal" ), ch->level, ch, ch );
			    }
			    break;
			case DFND_DISPELMAGIC:
			    if ( victim->first_affect )
			    {
				act( AT_MAGIC, "$n utters an incantation...", ch, NULL, NULL, TO_ROOM );
				retcode = spell_dispel_magic( skill_lookup( "dispel magic" ), ch->level, ch, victim );
			    }
			    break;
			case DFND_DISPELEVIL:
			    act( AT_MAGIC, "$n utters an incantation...", ch, NULL, NULL, TO_ROOM );
			    retcode = spell_dispel_evil( skill_lookup( "dispel evil" ), ch->level, ch, victim );
			    break;
			case DFND_TELEPORT:
			    retcode = spell_teleport( skill_lookup( "teleport"), ch->level, ch, ch );
			    break;
			case DFND_SHOCKSHIELD:
			    if ( !IS_AFFECTED( ch, AFF_SHOCKSHIELD ) )
			    {
				act( AT_MAGIC, "$n utters a few incantations...", ch, NULL, NULL, TO_ROOM );
				retcode = spell_smaug( skill_lookup( "shockshield" ), ch->level, ch, ch );
			    }
			    else
				retcode = rNONE;
			    break;
                       case DFND_VENOMSHIELD:
                            if ( !IS_AFFECTED( ch, AFF_VENOMSHIELD ) )
                            {
                                act( AT_MAGIC, "$n utters a few incantations ...", ch, NULL, NULL, TO_ROOM );
                                retcode = spell_smaug( skill_lookup( "venomshield" ), ch->level, ch, ch );
                            }
                            else
                                retcode = rNONE;
                            break;
			case DFND_ACIDMIST:
			    if ( !IS_AFFECTED( ch, AFF_ACIDMIST ) )
			    {
				act( AT_MAGIC, "$n utters a few incantations ...", ch, NULL, NULL, TO_ROOM );
				retcode = spell_smaug( skill_lookup( "acidmist" ), ch->level, ch, ch );
			    }
			    else
				retcode = rNONE;
			    break;
			case DFND_FIRESHIELD:
			    if ( !IS_AFFECTED( ch, AFF_FIRESHIELD ) )
			    {
				act( AT_MAGIC, "$n utters a few incantations...", ch, NULL, NULL, TO_ROOM );
				retcode = spell_smaug( skill_lookup( "fireshield" ), ch->level, ch, ch );
			    }
			    else
				retcode = rNONE;
			    break;
			case DFND_ICESHIELD:
			    if ( !IS_AFFECTED( ch, AFF_ICESHIELD ) )
			    {
				act( AT_MAGIC, "$n utters a few incantations...", ch, NULL, NULL, TO_ROOM );
				retcode = spell_smaug( skill_lookup( "iceshield" ), ch->level, ch, ch );
			    }
			    else
				retcode = rNONE;
			    break;
			case DFND_TRUESIGHT:
			    if ( !IS_AFFECTED( ch, AFF_TRUESIGHT ) )
				retcode = spell_smaug( skill_lookup( "true" ), ch->level, ch, ch );
			    else
				retcode = rNONE;
			    break;
			case DFND_SANCTUARY:
			    if ( !IS_AFFECTED( ch, AFF_SANCTUARY ) )
			    {
				act( AT_MAGIC, "$n utters a few incantations...", ch, NULL, NULL, TO_ROOM );
				retcode = spell_smaug( skill_lookup( "sanctuary" ), ch->level, ch, ch );
			    }
			    else
				retcode = rNONE;
			    break;
		    }
		    if ( attacktype != -1 && (retcode == rCHAR_DIED || char_died(ch)) )
			continue;
		}
	    }
	}

	/*
	 * Fun for the whole family!
	 */
	for ( rch = ch->in_room->first_person; rch; rch = rch_next )
	{
	    rch_next = rch->next_in_room;

            /*
             *   Group Fighting Styles Support:
             *   If ch is tanking
             *   If rch is using a more aggressive style than ch
             *   Then rch is the new tank   -h
             */
            /* &&( is_same_group(ch, rch)      ) */

            if( ( !IS_NPC(ch) && !IS_NPC(rch) )
              &&( rch!=ch                     )
              &&( rch->fighting               )
              &&( who_fighting(rch->fighting->who) == ch    )
              &&( !xIS_SET( rch->fighting->who->act, ACT_AUTONOMOUS ) )
              &&( rch->style < ch->style      )
              )
            {
                 rch->fighting->who->fighting->who = rch; 

            }

	    if ( IS_AWAKE(rch) && !rch->fighting )
	    {
		/*
		 * PC's auto-assist others in their group.
		 */
		if ( !IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) )
		{
		    if ( ( ( !IS_NPC(rch) && rch->desc )
		    ||        IS_AFFECTED(rch, AFF_CHARM) )
		    && is_same_group(ch, rch)
		    && !is_safe( rch, victim, TRUE) )
			multi_hit( rch, victim, TYPE_UNDEFINED );
		    continue;
		}

		/*
		 * NPC's assist NPC's of same type or 12.5% chance regardless.
		 */
		if ( IS_NPC(rch) && !IS_AFFECTED(rch, AFF_CHARM)
		&&  !xIS_SET(rch->act, ACT_NOASSIST) )
		{
		    if ( char_died(ch) )
			break;
		    if ( rch->pIndexData == ch->pIndexData
		    ||   number_bits( 3 ) == 0 )
		    {
			CHAR_DATA *vch;
			CHAR_DATA *target;
			int number;

			target = NULL;
			number = 0;			
                        for ( vch = ch->in_room->first_person; vch; vch = vch->next )
			{
			    if ( can_see( rch, vch )
			    &&   is_same_group( vch, victim )
			    &&   number_range( 0, number ) == 0 )
			    {
				if ( vch->mount && vch->mount == rch )
				  target = NULL;
				else
				{
				  target = vch;
				  number++;
				}
			    }
			}

			if ( target )
			    multi_hit( rch, target, TYPE_UNDEFINED );
		    }
		}
	    }
	}
    }

    return;
}



/*
 * Do one group of attacks.
 */
ch_ret multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    int     chance;
    int	    dual_bonus;
    ch_ret  retcode;

    /* add timer to pkillers */
    if ( !IS_NPC(ch) && !IS_NPC(victim) )
    {
        if ( xIS_SET(ch->act, PLR_NICE) ) return rNONE;
        add_timer( ch,     TIMER_RECENTFIGHT, 11, NULL, 0 );
        add_timer( victim, TIMER_RECENTFIGHT, 11, NULL, 0 );
    }

    if ( is_attack_supressed( ch ) )
      return rNONE;

    if ( IS_NPC(ch) && xIS_SET(ch->act, ACT_NOATTACK) )
      return rNONE;

    if ( (retcode = one_hit( ch, victim, dt )) != rNONE )
      return retcode;

    if ( who_fighting( ch ) != victim || dt == gsn_backstab
    ||   dt == gsn_circle || dt == gsn_pounce )
	return rNONE;
	
    /* Very high chance of hitting compared to chance of going berserk */
    /* 40% or higher is always hit.. don't learn anything here though. */
    /* -- Altrag */
    chance = IS_NPC(ch) ? 100 : (LEARNED(ch, gsn_berserk)*5/2);
    if ( IS_AFFECTED(ch, AFF_BERSERK) && number_percent() < chance )
      if ( (retcode = one_hit( ch, victim, dt )) != rNONE ||
            who_fighting( ch ) != victim )
        return retcode;

    if ( get_eq_char( ch, WEAR_DUAL_WIELD ) )
    {
      dual_bonus = IS_NPC(ch) ? (ch->level / 10) : (LEARNED(ch, gsn_dual_wield) / 10);
      chance = IS_NPC(ch) ? ch->level : LEARNED(ch, gsn_dual_wield);
      if ( number_percent( ) < chance )
      {
	learn_from_success( ch, gsn_dual_wield );
	retcode = one_hit( ch, victim, dt );
	if ( retcode != rNONE || who_fighting( ch ) != victim )
	    return retcode;
      }
      else
	learn_from_failure( ch, gsn_dual_wield );
    }
    else
      dual_bonus = 0;

    if ( ch->move < 10 )
      dual_bonus = -20;

    /*
     * NPC predetermined number of attacks			-Thoric
     */
    if ( IS_NPC(ch) && ch->numattacks > 0 )
    {
      int temp_attacks;

      /* We could put a penalty on mobs if their stances aren't GM but
       * Why? :)    SHADDAI
       */

      temp_attacks = ch->numattacks;
      if (victim->stance != STANCE_MONKEY && ch->stance != STANCE_MONKEY)
        temp_attacks += stance_index[ch->stance].num_attacks;
      for (chance = 0; chance < temp_attacks; chance++)
        {
          retcode = one_hit (ch, victim, dt);
          if (retcode != rNONE || who_fighting (ch) != victim)
            return retcode;
        }
      return retcode;
    }


  /* Support added for stances on chars.  Only give them the extra bonus
   * attacks if their stance is GM.   SHADDAI
   */
  if (!IS_NPC (ch) && ch->stance != STANCE_MONKEY &&
      victim->stance != STANCE_MONKEY)
    {
      if (ch->pcdata->stances[ch->stance] >= STANCE_GRAND_MASTER)
        for (chance = 0; chance < stance_index[ch->stance].num_attacks;chance++)
          {
            retcode = one_hit (ch, victim, dt);
            if (retcode != rNONE || who_fighting (ch) != victim)
              return retcode;
          }
    }

    chance = IS_NPC(ch) ? ch->level
	   : (int) ((LEARNED(ch, gsn_second_attack)+dual_bonus)/1.5);
    if ( number_percent( ) < chance )
    {
	learn_from_success( ch, gsn_second_attack );
	retcode = one_hit( ch, victim, dt );
	if ( retcode != rNONE || who_fighting( ch ) != victim )
	    return retcode;
    }
    else
	learn_from_failure( ch, gsn_second_attack );

    chance = IS_NPC(ch) ? ch->level
	   : (int) ((LEARNED(ch, gsn_third_attack)+(dual_bonus*1.5))/2);
    if ( number_percent( ) < chance )
    {
	learn_from_success( ch, gsn_third_attack );
	retcode = one_hit( ch, victim, dt );
	if ( retcode != rNONE || who_fighting( ch ) != victim )
	    return retcode;
    }
    else
	learn_from_failure( ch, gsn_third_attack );

    chance = IS_NPC(ch) ? ch->level
	   : (int) ((LEARNED(ch, gsn_fourth_attack)+(dual_bonus*2))/3);
    if ( number_percent( ) < chance )
    {
	learn_from_success( ch, gsn_fourth_attack );
	retcode = one_hit( ch, victim, dt );
	if ( retcode != rNONE || who_fighting( ch ) != victim )
	    return retcode;
    }
    else
	learn_from_failure( ch, gsn_fourth_attack );

    chance = IS_NPC(ch) ? ch->level
	   : (int) ((LEARNED(ch, gsn_fifth_attack)+(dual_bonus*3))/4);
    if ( number_percent( ) < chance )
    {
	learn_from_success( ch, gsn_fifth_attack );
	retcode = one_hit( ch, victim, dt );
	if ( retcode != rNONE || who_fighting( ch ) != victim )
	    return retcode;
    }
    else
	learn_from_failure( ch, gsn_fifth_attack );

    retcode = rNONE;

    chance = IS_NPC(ch) ? (int) (ch->level / 2) : 0;
    if ( number_percent( ) < chance )
	retcode = one_hit( ch, victim, dt );

    if ( retcode == rNONE )
    {
	int move;

	if ( !IS_AFFECTED(ch, AFF_FLYING)
	&&   !IS_AFFECTED(ch, AFF_FLOATING) )
	  move = encumbrance( ch, movement_loss[UMIN(SECT_MAX-1, ch->in_room->sector_type)] );
	else
	  move = encumbrance( ch, 1 );
	if ( ch->move )
	  ch->move = UMAX( 0, ch->move - move );
    }
    return retcode;
}


/*
 * Weapon types, haus
 */
int weapon_prof_bonus_check( CHAR_DATA *ch, OBJ_DATA *wield, int *gsn_ptr )
{
    int bonus;

    bonus = 0;	*gsn_ptr = -1;
    if ( !IS_NPC(ch) && ch->level > 5 && wield )   
    {
	switch(wield->value[3])
	{
	   default:		*gsn_ptr = -1;			break;
           case DAM_HIT:
	   case DAM_SUCTION:
	   case DAM_BITE:
	   case DAM_BLAST:	*gsn_ptr = gsn_pugilism;	break;
           case DAM_SLASH:
           case DAM_SLICE:	*gsn_ptr = gsn_long_blades;	break;
           case DAM_PIERCE:
           case DAM_STAB:	*gsn_ptr = gsn_short_blades;	break;
           case DAM_WHIP:	*gsn_ptr = gsn_flexible_arms;	break;
           case DAM_CLAW:	*gsn_ptr = gsn_talonous_arms;	break;
           case DAM_POUND:
           case DAM_CRUSH:	*gsn_ptr = gsn_bludgeons;	break;
	   case DAM_BOLT:
	   case DAM_ARROW:
	   case DAM_DART:
	   case DAM_STONE:
	   case DAM_PEA:	*gsn_ptr = gsn_missile_weapons;	break;

	}
	if ( *gsn_ptr != -1 )
	  bonus = (int) ((LEARNED(ch, *gsn_ptr) -50)/10);

       /* Reduce weapon bonuses for misaligned clannies.
       if ( IS_CLANNED(ch) )
       {
          bonus = bonus * 1000 / 
          ( 1 + abs( ch->alignment - ch->pcdata->clan->alignment ) );
       }*/

	if ( IS_DEVOTED(ch) )
	   bonus -= ch->pcdata->favor / -400 ;

    }
    return bonus;
}

/*
 * Calculate the tohit bonus on the object and return RIS values.
 * -- Altrag
 */
int obj_hitroll( OBJ_DATA *obj )
{
	int tohit = 0;
	AFFECT_DATA *paf;
	
	for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
		if ( paf->location == APPLY_HITROLL )
			tohit += paf->modifier;
	for ( paf = obj->first_affect; paf; paf = paf->next )
		if ( paf->location == APPLY_HITROLL )
			tohit += paf->modifier;
	return tohit;
}

/*
 * Offensive shield level modifier
 */
sh_int off_shld_lvl( CHAR_DATA *ch, CHAR_DATA *victim )
{
    sh_int lvl;

    if ( !IS_NPC(ch) )          /* players get much less effect */
    {
        lvl = UMAX( 1, (ch->level - 10) / 2 );
        if ( number_percent() + (victim->level - lvl) < 40 )
	{
	  if ( CAN_PKILL( ch ) && CAN_PKILL( victim ) )
	    return ch->level;
	  else
	    return lvl;
	}
        else
          return 0;
    }
    else
    {
	lvl = ch->level / 2;
	if ( number_percent() + (victim->level - lvl) < 70 )
	  return lvl;
	else
	  return 0;
    }
}

/*
 * Hit one guy once.
 */
ch_ret one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    OBJ_DATA *wield;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int plusris;
    int dam;
    int diceroll;
    int attacktype, cnt;
    int	prof_bonus;
    int	prof_gsn = -1;
    ch_ret retcode = rNONE;
    static bool dual_flip = FALSE;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return rVICT_DIED;

    used_weapon = NULL;
    /*
     * Figure out the weapon doing the damage			-Thoric
     * Dual wield support -- switch weapons each attack
     */
    if ( (wield = get_eq_char( ch, WEAR_DUAL_WIELD )) != NULL )
    {
	if ( dual_flip == FALSE )
	{
	    dual_flip = TRUE;
	    wield = get_eq_char( ch, WEAR_WIELD );
	}
	else
	    dual_flip = FALSE;
    }
    else
	wield = get_eq_char( ch, WEAR_WIELD );

    used_weapon = wield;

    if ( wield )
	prof_bonus = weapon_prof_bonus_check( ch, wield, &prof_gsn );
    else
	prof_bonus = 0;

    if ( ch->fighting		/* make sure fight is already started */
    &&   dt == TYPE_UNDEFINED
    &&   IS_NPC(ch)
    &&  !xIS_EMPTY(ch->attacks) )
    {
	cnt = 0;
	for ( ;; )
	{
	    attacktype = number_range( 0, 6 );
	    if ( xIS_SET( ch->attacks, attacktype ) )
		break;
	    if ( cnt++ > 16 )
 	    {
		attacktype = -1;
		break;
	    }
	}
	if ( attacktype == ATCK_BACKSTAB )
	    attacktype = -1;
	if ( wield && number_percent( ) > 25 )
	    attacktype = -1;
	if ( !wield && number_percent( ) > 50 )
	    attacktype = -1;

	switch ( attacktype )
	{
	    default:
		break;
	    case ATCK_BITE:
		do_bite( ch, "" );
		retcode = global_retcode;
		break;
	    case ATCK_CLAWS:
		do_claw( ch, "" );
		retcode = global_retcode;
		break;
	    case ATCK_TAIL:
		do_tail( ch, "" );
		retcode = global_retcode;
		break;
	    case ATCK_STING:
		do_sting( ch, "" );
		retcode = global_retcode;
		break;
	    case ATCK_PUNCH:
		do_punch( ch, "" );
		retcode = global_retcode;
		break;
	    case ATCK_KICK:
		do_kick( ch, "" );
		retcode = global_retcode;
		break;
	    case ATCK_TRIP:
		attacktype = 0;
		break;
	}
	if ( attacktype >= 0 )
	    return retcode;
    }

    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
    }

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if ( IS_NPC(ch) )
    {
	thac0_00 = ch->mobthac0;
	thac0_32 =  0;
    }
    else
    {
	thac0_00 = class_table[ch->class]->thac0_00;
	thac0_32 = class_table[ch->class]->thac0_32;
    }
    thac0     = interpolate( ch->level, thac0_00, thac0_32 ) - GET_HITROLL(ch);
    victim_ac = UMAX( -19, (int) (GET_AC(victim) / 10) );

    /* if you can't see what's coming... */
    if ( wield && !can_see_obj( victim, wield) )
	victim_ac += 1;
    if ( !can_see( ch, victim ) )
	victim_ac -= 4;

    /*
     * "learning" between combatants.  Takes the intelligence difference,
     * and multiplies by the times killed to make up a learning bonus
     * given to whoever is more intelligent		-Thoric
     * (basically the more intelligent one "learns" the other's fighting style)
     */
    if ( ch->fighting && ch->fighting->who == victim )
    {
	sh_int times = ch->fighting->timeskilled;

	if ( times )
	{
	    sh_int intdiff = get_curr_int(ch) - get_curr_int(victim);

	    if ( intdiff != 0 )
		victim_ac += (intdiff*times)/10;
	}
    }

    /* Weapon proficiency bonus */
    victim_ac += prof_bonus;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
	;

    if ( diceroll == 0
    || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
	/* Miss. */
	if ( prof_gsn != -1 )
	    learn_from_failure( ch, prof_gsn );
	damage( ch, victim, 0, dt );
	tail_chain( );
	return rNONE;
    }

    /*
     * Hit.
     * Calc damage.
     */

    if ( !wield )       /* bare hand dice formula fixed by Thoric */
	dam = number_range( ch->barenumdie, ch->baresizedie * ch->barenumdie );
    else
	dam = number_range( wield->value[1], wield->value[2] );

    /*
     * Bonuses.
     */
    dam += GET_DAMROLL(ch);

    if ( prof_bonus )
	dam += prof_bonus / 4;

    /*
     * Calculate Damage Modifiers from Victim's Fighting Style
     */
    if ( victim->position == POS_BERSERK )
       dam = 1.2 * dam;
    else if ( victim->position == POS_AGGRESSIVE )
       dam = 1.1 * dam;
    else if ( victim->position == POS_DEFENSIVE )
       dam = .85 * dam;
    else if ( victim->position == POS_EVASIVE )
       dam = .8 * dam;

    /*
     * Calculate Damage Modifiers from Attacker's Fighting Style
     */
    if ( ch->position == POS_BERSERK )
       dam = 1.2 * dam;
    else if ( ch->position == POS_AGGRESSIVE )
       dam = 1.1 * dam;
    else if ( ch->position == POS_DEFENSIVE )
       dam = .85 * dam;
    else if ( ch->position == POS_EVASIVE )
       dam = .8 * dam;

    if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_enhanced_damage] > 0 )
    {
	dam += (int) (dam * LEARNED(ch, gsn_enhanced_damage) / 120);
	learn_from_success( ch, gsn_enhanced_damage );
    }

    if ( !IS_AWAKE(victim) )
	dam *= 2;

    if ( dt == gsn_backstab )
	dam *= (2 + URANGE( 2, ch->level - (victim->level/4), 30 ) / 8);

   if ( dt == gsn_pounce )
        dam *= (2 + URANGE( 2, ch->level - (victim->level/4), 30 ) / 11);
 
    if ( dt == gsn_circle )
 	dam *= (2 + URANGE( 2, ch->level - (victim->level/4), 30 ) / 16); 

    if ( dam <= 0 )
	dam = 1;

    plusris = 0;

    if ( wield )
    {
	if ( IS_OBJ_STAT(wield, ITEM_MAGIC) )
 	    dam = ris_damage( victim, dam, RIS_MAGIC );
	else
	    dam = ris_damage( victim, dam, RIS_NONMAGIC );

	/*
	 * Handle PLUS1 - PLUS6 ris bits vs. weapon hitroll	-Thoric
	 */
	plusris = obj_hitroll( wield );
    }
    else
	dam = ris_damage( victim, dam, RIS_NONMAGIC );

    /* check for RIS_PLUSx 					-Thoric */
    if ( dam )
    {
	int x, res, imm, sus, mod;

	if ( plusris )
	   plusris = RIS_PLUS1 << UMIN(plusris, 7);

	/* initialize values to handle a zero plusris */
	imm = res = -1;  sus = 1;

	/* find high ris */
	for ( x = RIS_PLUS1; x <= RIS_PLUS6; x <<= 1 )
	{
	    if ( IS_SET( victim->immune, x ) )
		imm = x;
	    if ( IS_SET( victim->resistant, x ) )
		res = x;
	    if ( IS_SET( victim->susceptible, x ) )
		sus = x;
	}
	mod = 10;
	if ( imm >= plusris )
	    mod -= 10;
	if ( res >= plusris )
	    mod -= 2;
	if ( sus <= plusris )
	    mod += 2;

	/* check if immune */
	if ( mod <= 0 )
	    dam = -1;
	if ( mod != 10 )
	    dam = (dam * mod) / 10;
    }
    if ( prof_gsn != -1 )
    {
	if ( dam > 0 )
	    learn_from_success( ch, prof_gsn );
	else
	    learn_from_failure( ch, prof_gsn );
    }

    /* immune to damage */
    if ( dam == -1 )
    {
	if ( dt >= 0 && dt < top_sn )
	{
	    SKILLTYPE *skill = skill_table[dt];
	    bool found = FALSE;

	    if ( skill->imm_char && skill->imm_char[0] != '\0' )
	    {
		act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
		found = TRUE;
	    }
	    if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
	    {
		act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
		found = TRUE;
	    }
	    if ( skill->imm_room && skill->imm_room[0] != '\0' )
	    {
		act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT );
		found = TRUE;
	    }
	    if ( found )
	      return rNONE;
	}
	dam = 0;
    }
    if ( (retcode = damage( ch, victim, dam, dt )) != rNONE )
	return retcode;
    if ( char_died(ch) )
	return rCHAR_DIED;
    if ( char_died(victim) )
	return rVICT_DIED;

    retcode = rNONE;
    if ( dam == 0 )
	return retcode;

    /*
     * Weapon spell support				-Thoric
     * Each successful hit casts a spell
     */
    if ( wield
    &&  !IS_SET(victim->immune, RIS_MAGIC)
    &&  !xIS_SET(victim->in_room->room_flags, ROOM_NO_MAGIC) )
    {
	AFFECT_DATA *aff;
	
	for ( aff = wield->pIndexData->first_affect; aff; aff = aff->next )
	   if ( aff->location == APPLY_WEAPONSPELL
	   &&   IS_VALID_SN(aff->modifier)
	   &&   skill_table[aff->modifier]->spell_fun )
		retcode = (*skill_table[aff->modifier]->spell_fun) ( aff->modifier, (wield->level+3)/3, ch, victim );
	if ( retcode != rNONE || char_died(ch) || char_died(victim) )
		return retcode;
	for ( aff = wield->first_affect; aff; aff = aff->next )
	   if ( aff->location == APPLY_WEAPONSPELL
	   &&   IS_VALID_SN(aff->modifier)
	   &&   skill_table[aff->modifier]->spell_fun )
		retcode = (*skill_table[aff->modifier]->spell_fun) ( aff->modifier, (wield->level+3)/3, ch, victim );
	if ( retcode != rNONE || char_died(ch) || char_died(victim) )
		return retcode;
    }

    /*
     * magic shields that retaliate				-Thoric
     */
    if ( IS_AFFECTED( victim, AFF_FIRESHIELD )
    &&  !IS_AFFECTED( ch, AFF_FIRESHIELD ) )
	retcode = spell_smaug( skill_lookup( "flare" ), off_shld_lvl(victim, ch), victim, ch );
    if ( retcode != rNONE || char_died(ch) || char_died(victim) )
      return retcode;

    if ( IS_AFFECTED( victim, AFF_ICESHIELD )
    &&  !IS_AFFECTED( ch, AFF_ICESHIELD ) )
         retcode = spell_smaug( skill_lookup( "iceshard" ), off_shld_lvl(victim, ch), victim, ch );
    if ( retcode != rNONE || char_died(ch) || char_died(victim) )
      return retcode;

    if ( IS_AFFECTED( victim, AFF_SHOCKSHIELD )
    &&  !IS_AFFECTED( ch, AFF_SHOCKSHIELD ) )
	retcode = spell_smaug( skill_lookup( "torrent" ), off_shld_lvl(victim, ch), victim, ch );
    if ( retcode != rNONE || char_died(ch) || char_died(victim) )
      return retcode;

    if ( IS_AFFECTED( victim, AFF_ACIDMIST )
    &&  !IS_AFFECTED( ch, AFF_ACIDMIST ) )
	retcode = spell_smaug( skill_lookup( "acidshot" ), off_shld_lvl(victim, ch), victim, ch );
    if ( retcode != rNONE || char_died(ch) || char_died(victim) )
	return retcode;

    if ( IS_AFFECTED( victim, AFF_VENOMSHIELD )
    &&  !IS_AFFECTED( ch, AFF_VENOMSHIELD ) )
        retcode = spell_smaug( skill_lookup( "venomshot" ), off_shld_lvl(victim, ch), victim, ch );
    if ( retcode != rNONE || char_died(ch) || char_died(victim) )
        return retcode;    

    tail_chain( );
    return retcode;
}

/*
 * Hit one guy with a projectile.
 * Handles use of missile weapons (wield = missile weapon)
 * or thrown items/weapons
 */
ch_ret projectile_hit( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield,
		       OBJ_DATA *projectile, sh_int dist )
{
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int plusris;
    int dam;
    int diceroll;
    int	prof_bonus;
    int	prof_gsn = -1;
    int proj_bonus;
    int dt;
    ch_ret retcode;

    if ( !projectile )
	return rNONE;

    if ( projectile->item_type == ITEM_PROJECTILE 
    ||   projectile->item_type == ITEM_WEAPON )
    {
	dt = TYPE_HIT + projectile->value[3];
	proj_bonus = number_range(projectile->value[1], projectile->value[2]);
    }
    else
    {
	dt = TYPE_UNDEFINED;
	proj_bonus = number_range(1, URANGE(2, get_obj_weight(projectile), 100) );
    }

    /*
     * Can't beat a dead char!
     */
    if ( victim->position == POS_DEAD || char_died(victim) )
    {
	extract_obj(projectile);
	return rVICT_DIED;
    }

    if ( wield )
	prof_bonus = weapon_prof_bonus_check( ch, wield, &prof_gsn );
    else
	prof_bonus = 0;

    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield && wield->item_type == ITEM_MISSILE_WEAPON )
	    dt += wield->value[3];
    }

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if ( IS_NPC(ch) )
    {
	thac0_00 = ch->mobthac0;
	thac0_32 =  0;
    }
    else
    {
	thac0_00 = class_table[ch->class]->thac0_00;
	thac0_32 = class_table[ch->class]->thac0_32;
    }
    thac0     = interpolate( ch->level, thac0_00, thac0_32 ) - GET_HITROLL(ch) + (dist*2);
    victim_ac = UMAX( -19, (int) (GET_AC(victim) / 10) );

    /* if you can't see what's coming... */
    if ( !can_see_obj( victim, projectile) )
	victim_ac += 1;
    if ( !can_see( ch, victim ) )
	victim_ac -= 4;

    /* Weapon proficiency bonus */
    victim_ac += prof_bonus;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
	;

    if ( diceroll == 0
    || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
	/* Miss. */
	if ( prof_gsn != -1 )
	    learn_from_failure( ch, prof_gsn );

	/* Do something with the projectile */
	if ( number_percent() < 50 )
	    extract_obj(projectile);
	else
	{
	    if ( projectile->in_obj )
		obj_from_obj(projectile);
	    if ( projectile->carried_by )
		obj_from_char(projectile);
	    obj_to_room(projectile, victim->in_room);
	}
	damage( ch, victim, 0, dt );
	tail_chain( );
	return rNONE;
    }

    /*
     * Hit.
     * Calc damage.
     */

    if ( !wield )       /* dice formula fixed by Thoric */
	dam = proj_bonus;
    else
	dam = number_range(wield->value[1], wield->value[2]) + (proj_bonus / 10);

    /*
     * Bonuses.
     */
    dam += GET_DAMROLL(ch);

    if ( prof_bonus )
	dam += prof_bonus / 4;

    /*
     * Calculate Damage Modifiers from Victim's Fighting Style
     */
    if ( victim->position == POS_BERSERK )
       dam = 1.2 * dam;
    else if ( victim->position == POS_AGGRESSIVE )
       dam = 1.1 * dam;
    else if ( victim->position == POS_DEFENSIVE )
       dam = .85 * dam;
    else if ( victim->position == POS_EVASIVE )
       dam = .8 * dam;

    if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_enhanced_damage] > 0 )
    {
	dam += (int) (dam * LEARNED(ch, gsn_enhanced_damage) / 120);
	learn_from_success( ch, gsn_enhanced_damage );
    }

    if ( !IS_AWAKE(victim) )
	dam *= 2;

    if ( dam <= 0 )
	dam = 1;

    plusris = 0;

    if ( IS_OBJ_STAT(projectile, ITEM_MAGIC) )
	dam = ris_damage( victim, dam, RIS_MAGIC );
    else
	dam = ris_damage( victim, dam, RIS_NONMAGIC );

    /*
     * Handle PLUS1 - PLUS6 ris bits vs. weapon hitroll	-Thoric
     */
    if ( wield )
	plusris = obj_hitroll( wield );

    /* check for RIS_PLUSx 					-Thoric */
    if ( dam )
    {
	int x, res, imm, sus, mod;

	if ( plusris )
	   plusris = RIS_PLUS1 << UMIN(plusris, 7);

	/* initialize values to handle a zero plusris */
	imm = res = -1;  sus = 1;

	/* find high ris */
	for ( x = RIS_PLUS1; x <= RIS_PLUS6; x <<= 1 )
	{
	   if ( IS_SET( victim->immune, x ) )
		imm = x;
	   if ( IS_SET( victim->resistant, x ) )
		res = x;
	   if ( IS_SET( victim->susceptible, x ) )
		sus = x;
	}
	mod = 10;
	if ( imm >= plusris )
	  mod -= 10;
	if ( res >= plusris )
	  mod -= 2;
	if ( sus <= plusris )
	  mod += 2;

	/* check if immune */
	if ( mod <= 0 )
	  dam = -1;
	if ( mod != 10 )
	  dam = (dam * mod) / 10;
    }
    if ( prof_gsn != -1 )
    {
      if ( dam > 0 )
        learn_from_success( ch, prof_gsn );
      else
        learn_from_failure( ch, prof_gsn );
    }

    /* immune to damage */
    if ( dam == -1 )
    {
	if ( dt >= 0 && dt < top_sn )
	{
	    SKILLTYPE *skill = skill_table[dt];
	    bool found = FALSE;

	    if ( skill->imm_char && skill->imm_char[0] != '\0' )
	    {
		act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
		found = TRUE;
	    }
	    if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
	    {
		act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
		found = TRUE;
	    }
	    if ( skill->imm_room && skill->imm_room[0] != '\0' )
	    {
		act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT );
		found = TRUE;
	    }
	    if ( found )
	    {
		if ( number_percent() < 50 )
		    extract_obj(projectile);
		else
		{
		    if ( projectile->in_obj )
			obj_from_obj(projectile);
		    if ( projectile->carried_by )
			obj_from_char(projectile);
		    obj_to_room(projectile, victim->in_room);
		}
		return rNONE;
	    }
	}
	dam = 0;
    }
    if ( (retcode = damage( ch, victim, dam, dt )) != rNONE )
    {
	extract_obj(projectile);
	return retcode;
    }
    if ( char_died(ch) )
    {
	extract_obj(projectile);
	return rCHAR_DIED;
    }
    if ( char_died(victim) )
    {
	extract_obj(projectile);
	return rVICT_DIED;
    }

    retcode = rNONE;
    if ( dam == 0 )
    {
	if ( number_percent() < 50 )
	    extract_obj(projectile);
	else
	{
	    if ( projectile->in_obj )
		obj_from_obj(projectile);
	    if ( projectile->carried_by )
		obj_from_char(projectile);
	    obj_to_room(projectile, victim->in_room);
	}
	return retcode;
    }

/* weapon spells	-Thoric */
    if ( wield
    &&  !IS_SET(victim->immune, RIS_MAGIC)
    &&  !xIS_SET(victim->in_room->room_flags, ROOM_NO_MAGIC) )
    {
	AFFECT_DATA *aff;
	
	for ( aff = wield->pIndexData->first_affect; aff; aff = aff->next )
	   if ( aff->location == APPLY_WEAPONSPELL
	   &&   IS_VALID_SN(aff->modifier)
	   &&   skill_table[aff->modifier]->spell_fun )
		retcode = (*skill_table[aff->modifier]->spell_fun) ( aff->modifier, (wield->level+3)/3, ch, victim );
	if ( retcode != rNONE || char_died(ch) || char_died(victim) )
	{
	    extract_obj(projectile);
	    return retcode;
	}
	for ( aff = wield->first_affect; aff; aff = aff->next )
	   if ( aff->location == APPLY_WEAPONSPELL
	   &&   IS_VALID_SN(aff->modifier)
	   &&   skill_table[aff->modifier]->spell_fun )
		retcode = (*skill_table[aff->modifier]->spell_fun) ( aff->modifier, (wield->level+3)/3, ch, victim );
	if ( retcode != rNONE || char_died(ch) || char_died(victim) )
	{
	    extract_obj(projectile);
	    return retcode;
	}
    }

    extract_obj(projectile);

    tail_chain( );
    return retcode;
}

/*
 * Calculate damage based on resistances, immunities and suceptibilities
 *					-Thoric
 */
sh_int ris_damage( CHAR_DATA *ch, sh_int dam, int ris )
{
   sh_int modifier;

   modifier = 10;
   if ( (IS_SET(ch->immune, ris ) || IS_SET (ch->stance_immune, ris))
        && !IS_SET(ch->no_immune, ris) )
     modifier -= 10;
   if ( (IS_SET(ch->resistant, ris ) || IS_SET (ch->stance_resistant, ris))
        && !IS_SET(ch->no_resistant, ris) )
     modifier -= 2;
   if ( (IS_SET(ch->susceptible, ris ) || IS_SET (ch->stance_susceptible, ris))
        && !IS_SET(ch->no_susceptible, ris) )
   {
     if ( IS_NPC( ch )
     &&   IS_SET( ch->immune, ris ) )
	modifier += 0;
     else
	modifier += 2;
   }
   if ( modifier <= 0 )
     return -1;
   if ( modifier == 10 )
     return dam;
   return (dam * modifier) / 10;
}


/*
 * Inflict damage from a hit.   This is one damn big function.
 */
ch_ret damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char filename[256]; 
    char filename2[256];
    sh_int dameq;
    sh_int maxdam;
    bool npcvict;
    bool loot;
    int  xp_gain;
    OBJ_DATA *damobj;
    ch_ret retcode;
    sh_int dampmod;
    CHAR_DATA *gch /*, *lch */;
    int init_gold, new_gold, gold_diff;
    sh_int anopc = 0;  /* # of (non-pkill) pc in a (ch) */
    sh_int bnopc = 0;  /* # of (non-pkill) pc in b (victim) */


    retcode = rNONE;

    if ( !ch )
    {
	bug( "Damage: null ch!", 0 );
	return rERROR;
    }
    if ( !victim )
    {
	bug( "Damage: null victim!", 0 );
	return rVICT_DIED;
    }

    if ( victim->position == POS_DEAD )
	return rVICT_DIED;

    npcvict = IS_NPC(victim);

    /*
     * Check damage types for RIS				-Thoric
     */
    if ( dam && dt != TYPE_UNDEFINED )
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
	if ( IS_DRAIN(dt) )
	  dam = ris_damage(victim, dam, RIS_DRAIN);
	else
	if ( dt == gsn_poison || IS_POISON(dt) )
	  dam = ris_damage(victim, dam, RIS_POISON);
	else
	if ( dt == (TYPE_HIT + DAM_POUND) || dt == (TYPE_HIT + DAM_CRUSH)
	||   dt == (TYPE_HIT + DAM_STONE) || dt == (TYPE_HIT + DAM_PEA) )
	  dam = ris_damage(victim, dam, RIS_BLUNT);
	else
	if ( dt == (TYPE_HIT + DAM_STAB) || dt == (TYPE_HIT + DAM_PIERCE)
	||   dt == (TYPE_HIT + DAM_BITE) || dt == (TYPE_HIT + DAM_BOLT)
	||   dt == (TYPE_HIT + DAM_DART) || dt == (TYPE_HIT + DAM_ARROW) )
	  dam = ris_damage(victim, dam, RIS_PIERCE);
	else
	if ( dt == (TYPE_HIT + DAM_SLICE) || dt == (TYPE_HIT + DAM_SLASH)
	||   dt == (TYPE_HIT + DAM_WHIP)  || dt == (TYPE_HIT + DAM_CLAW) )
	  dam = ris_damage(victim, dam, RIS_SLASH);

	if ( dam == -1 )
	{
	    if ( dt >= 0 && dt < top_sn )
	    {
		bool found = FALSE;
		SKILLTYPE *skill = skill_table[dt];

		if ( skill->imm_char && skill->imm_char[0] != '\0' )
		{
		   act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
		   found = TRUE;
		}
		if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
		{
		   act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
		   found = TRUE;
		}
		if ( skill->imm_room && skill->imm_room[0] != '\0' )
		{
		   act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT );
		   found = TRUE;
		}
		if ( found )
		   return rNONE;
	    }
	    dam = 0;
	}
    }

    /*
     * Precautionary step mainly to prevent people in Hell from finding
     * a way out. --Shaddai
     */
    if ( xIS_SET( victim->in_room->room_flags, ROOM_SAFE )
    &&  !str_cmp( victim->in_room->area->filename, "limbo.are" ) )
	dam = 0;

    if ( dam && npcvict && ch != victim )
    {
	if ( !xIS_SET( victim->act, ACT_SENTINEL ) )
 	{
	    if ( victim->hunting )
	    {
		if ( victim->hunting->who != ch )	
		{
		    STRFREE( victim->hunting->name );
		    victim->hunting->name = QUICKLINK( ch->name );
		    victim->hunting->who  = ch;
		}
            }
	    else
            if ( !xIS_SET(victim->act, ACT_PACIFIST)  ) /* Gorog */
		start_hunting( victim, ch );
	}

	if ( victim->hating )
	{
   	    if ( victim->hating->who != ch )
 	    {
		STRFREE( victim->hating->name );
		victim->hating->name = QUICKLINK( ch->name );
		victim->hating->who  = ch;
	    }
	}
	else
	if ( !xIS_SET(victim->act, ACT_PACIFIST)  ) /* Gorog */
	    start_hating( victim, ch );
    }

    /*
     * Stop up any residual loopholes.
     */
    if ( dt == gsn_backstab )
      maxdam = ch->level * 80;
    else
      maxdam = ch->level * 40;


    if ( dam > maxdam )
    {
	sprintf( buf, "Damage: %d more than %d points!", dam, maxdam );
	bug( buf, dam );
	sprintf(buf, "** %s (lvl %d) -> %s **",
		ch->name,
		ch->level,
	        victim->name);
	bug( buf, 0 );
	dam = maxdam;
    }

    if (victim != ch)
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if ( victim == supermob ) /* Stop any damage */
	    return rNONE;

	if (is_safe(ch, victim, TRUE))
	    return rNONE;
	check_attacker(ch, victim);

	if (victim->position > POS_STUNNED)
	{
	    if (!victim->fighting && victim->in_room == ch->in_room)
		set_fighting(victim, ch);

	    /*
	       vwas: victim->position = POS_FIGHTING; 
	     */
	    if ( IS_NPC(victim) && victim->fighting )
		victim->position = POS_FIGHTING;
	    else if (victim->fighting)
	    {
		switch (victim->style)
		{
		    case (STYLE_EVASIVE):
			victim->position = POS_EVASIVE;
			break;
		    case (STYLE_DEFENSIVE):
			victim->position = POS_DEFENSIVE;
			break;
		    case (STYLE_AGGRESSIVE):
			victim->position = POS_AGGRESSIVE;
			break;
		    case (STYLE_BERSERK):
			victim->position = POS_BERSERK;
			break;
		    default:
			victim->position = POS_FIGHTING;
		}

	    }

	}

	if (victim->position > POS_STUNNED)
	{
	    if (!ch->fighting && victim->in_room == ch->in_room)
		set_fighting(ch, victim);

	    /*
	     * If victim is charmed, ch might attack victim's master.
	     */
	    if (IS_NPC(ch)
		&& npcvict
		&& IS_AFFECTED(victim, AFF_CHARM)
		&& victim->master
		&& victim->master->in_room == ch->in_room
		&& number_bits(3) == 0)
	    {
		stop_fighting(ch, FALSE);
		retcode = multi_hit(ch, victim->master, TYPE_UNDEFINED);
		return retcode;
	    }
	}


	/*
	 * More charm stuff.
	 */
	if (victim->master == ch)
	    stop_follower(victim);

	/*
	 * Pkill stuff.  If a deadly attacks another deadly or is attacked by
	 * one, then ungroup any nondealies.  Disabled untill I can figure out
	 * the right way to do it.
	 */


	/*
	   count the # of non-pkill pc in a ( not including == ch ) 
	 */
	for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
	    if (is_same_group(ch, gch) && !IS_NPC(gch)
		&& !IS_PKILL(gch) && (ch != gch))
		anopc++;

	/*
	   count the # of non-pkill pc in b ( not including == victim ) 
	 */
	for (gch = victim->in_room->first_person; gch; gch = gch->next_in_room)
	    if (is_same_group(victim, gch) && !IS_NPC(gch)
		&& !IS_PKILL(gch) && (victim != gch))
		bnopc++;


	/*
	   only consider disbanding if both groups have 1(+) non-pk pc,
	   or when one participant is pc, and the other group has 1(+)
	   pk pc's (in the case that participant is only pk pc in group) 
	 */
	if ((bnopc > 0 && anopc > 0)
	    || (bnopc > 0 && !IS_NPC(ch))
	    || (anopc > 0 && !IS_NPC(victim)))
	{
	    /*
	       Disband from same group first 
	     */
	    if (is_same_group(ch, victim))
	    {
		/*
		   Messages to char and master handled in stop_follower 
		 */
		act(AT_ACTION, "$n disbands from $N's group.",
		    (ch->leader == victim) ? victim : ch, NULL,
		    (ch->leader == victim) ? victim->master : ch->master,
		    TO_NOTVICT);
		if (ch->leader == victim)
		    stop_follower(victim);
		else
		    stop_follower(ch);
	    }
	    /*
	       if leader isnt pkill, leave the group and disband ch 
	     */
	    if (ch->leader != NULL && !IS_NPC(ch->leader) &&
		!IS_PKILL(ch->leader))
	    {
		act(AT_ACTION, "$n disbands from $N's group.", ch, NULL,
		    ch->master, TO_NOTVICT);
		stop_follower(ch);
	    }
	    else
	    {
		for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
		    if (is_same_group(gch, ch) && !IS_NPC(gch) &&
			!IS_PKILL(gch) && gch != ch)
		    {
			act(AT_ACTION, "$n disbands from $N's group.", ch, NULL,
			    gch->master, TO_NOTVICT);
			stop_follower(gch);
		    }
	    }
	    /*
	       if leader isnt pkill, leave the group and disband victim 
	     */
	    if (victim->leader != NULL && !IS_NPC(victim->leader) &&
		!IS_PKILL(victim->leader))
	    {
		act(AT_ACTION, "$n disbands from $N's group.", victim, NULL,
		    victim->master, TO_NOTVICT);
		stop_follower(victim);
	    }
	    else
	    {
		for (gch = victim->in_room->first_person; gch; gch = gch->next_in_room)
		    if (is_same_group(gch, victim) && !IS_NPC(gch) &&
			!IS_PKILL(gch) && gch != victim)
		    {
			act(AT_ACTION, "$n disbands from $N's group.", gch, NULL,
			    gch->master, TO_NOTVICT);
			stop_follower(gch);
		    }
	    }
	}

	/*
	 * Inviso attacks ... not.
	 */
	if (IS_AFFECTED(ch, AFF_INVISIBLE))
	{
	    affect_strip(ch, gsn_invis);
	    affect_strip(ch, gsn_mass_invis);
	    xREMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
	    act(AT_MAGIC, "$n fades into existence.", ch, NULL, NULL, TO_ROOM);
	}

	/* Take away Hide */
	if (IS_AFFECTED(ch, AFF_HIDE))
	    xREMOVE_BIT(ch->affected_by, AFF_HIDE);
	/*
	 * Damage modifiers.
	 */
	if (IS_AFFECTED(victim, AFF_SANCTUARY))
	    dam /= 2;

	if (IS_AFFECTED(victim, AFF_PROTECT) && IS_EVIL(ch))
	    dam -= (int) (dam / 4);

      /*
       * Support for stance damage modifiers.   SHADDAI
       * If anyone thinks of a quicker easier way to do this
       * Please tell me!  Maybe calculate some variable off of how well
       * They know the stance and only recalculate that when they
       * change stances or get better in the stance they are in.
       */
      if (ch->stance != STANCE_MONKEY && victim->stance != STANCE_MONKEY)
        {
          if (ch->stance > STANCE_NONE && stance_index[ch->stance].dam_done > 0)
            {
              float temp_dam;
              if (IS_NPC (ch))
                {
                  dam *= (stance_index[ch->stance].dam_done / 100.0);
                  temp_dam = (ch->pIndexData->stances[ch->stance] / 200.0);
                  if (temp_dam < .5)
                    temp_dam = .5;
                }
              else
                {
                  dam *= (stance_index[ch->stance].dam_done / 100.0);
                  temp_dam = (ch->pcdata->stances[ch->stance] / 200.0);
                  if (temp_dam < .5)
                    temp_dam = .5;
                }
              dam *= temp_dam;
            }

          if (victim->stance > STANCE_NONE && stance_index[victim->stance].dam_taken > 0)
            {
              float temp_dam;
              if (IS_NPC (victim))
                {
                  dam *= (stance_index[victim->stance].dam_taken / 100.0);
                  temp_dam = (victim->pIndexData->stances[victim->stance]/200.0);
                  if (temp_dam < .5)
                    temp_dam = .5;
                }
              else
                {
                  dam *= (stance_index[victim->stance].dam_taken / 100.0);
                  temp_dam = (victim->pcdata->stances[victim->stance] / 200.0);
                  if (temp_dam < .5)
                    temp_dam = .5;
                }
              dam /= temp_dam;
            }
        }

	if (dam < 0)
	    dam = 0;


	/*
	 * Check for disarm, trip, parry, dodge and tumble.
	 */
	if (dt >= TYPE_HIT && ch->in_room == victim->in_room)
	{
	    if (IS_NPC(ch)
		&& xIS_SET(ch->defenses, DFND_DISARM)
		&& ch->level > 9
		&& number_percent() < ch->level / 3) /* Was 2 try this --Shaddai*/
		disarm(ch, victim);

	    if (IS_NPC(ch)
		&& xIS_SET(ch->attacks, ATCK_TRIP)
		&& ch->level > 5
		&& number_percent() < ch->level / 2)
		trip(ch, victim);

	    if (check_parry(ch, victim))
		return rNONE;
	    if (check_dodge(ch, victim))
		return rNONE;
	    if (check_tumble(ch, victim))
		return rNONE;
	}

	/*
	 * Check control panel settings and modify damage
	 */
	if (IS_NPC(ch))
	{
	    if (npcvict)
		dampmod = sysdata.dam_mob_vs_mob;
	    else
	    {
		if ( victim->level < 50 )
		  dampmod = sysdata.dam_mob_vs_nonav;
		else
		  dampmod = sysdata.dam_mob_vs_plr;
	    }
	}
	else
	{
	    if (npcvict)
	    {
		if ( ch->level < 50 )
		  dampmod = sysdata.dam_nonav_vs_mob;
		else
		  dampmod = sysdata.dam_plr_vs_mob;
	    }	
	    else
		dampmod = sysdata.dam_plr_vs_plr;
	}

	if (dampmod > 0)
	    dam = (dam * dampmod) / 100;

	dam_message(ch, victim, dam, dt);
    }

    /*
     * Code to handle equipment getting damaged, and also support  -Thoric
     * bonuses/penalties for having or not having equipment where hit
     */
    if (dam > 10 && dt != TYPE_UNDEFINED)
    {
	/* get a random body eq part */
	dameq  = number_range(WEAR_LIGHT, WEAR_EYES);
	damobj = get_eq_char(victim, dameq);
	if ( damobj )
	{
	    if ( dam > get_obj_resistance(damobj)
	    &&   number_bits(1) == 0 )
	    {
		set_cur_obj(damobj);
		damage_obj(damobj);
	    }
	    dam -= 5;  /* add a bonus for having something to block the blow */
	}
	else
	    dam += 5;  /* add penalty for bare skin! */
    }

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;
/*  ch_printf( ch, "(%d damage)\n\r", dam ); */

    /*
     * Get experience based on % of damage done			-Thoric
     */
    if ( dam && ch != victim
    &&  !IS_NPC(ch) && ch->fighting && ch->fighting->xp )
    {
	if ( ch->fighting->who == victim )
	    xp_gain = (int) (ch->fighting->xp * dam) / victim->max_hit;
	else
	    xp_gain = (int) (xp_compute( ch, victim ) * 0.85 * dam) / victim->max_hit;
	if ( dt == gsn_backstab || dt == gsn_circle )
	    xp_gain = (int) (xp_gain*0.05);
	gain_exp( ch, xp_gain );
    }

    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
       victim->hit = 1;

    /* Make sure newbies dont die */

    if (!IS_NPC(victim) && NOT_AUTHED(victim) && victim->hit < 1)
	victim->hit = 1;

    if ( dam > 0 && dt > TYPE_HIT
    &&  !IS_AFFECTED( victim, AFF_POISON )
    &&   is_wielding_poisoned( ch )
    &&  !IS_SET( victim->immune, RIS_POISON )
    &&  !saves_poison_death( ch->level, victim ) )
    {
	AFFECT_DATA af;

	af.type      = gsn_poison;
	af.duration  = 20;
	af.location  = APPLY_STR;
	af.modifier  = -2;
	af.bitvector = meb(AFF_POISON);
	affect_join( victim, &af );
	victim->mental_state = URANGE( 20, ch->mental_state + (IS_PKILL(ch) ? 1 : 2), 100 );
    }

    /*
     * Vampire self preservation				-Thoric
     */
    if ( IS_VAMPIRE(victim) )
    {
      if ( dam >= (victim->max_hit / 10) )	/* get hit hard, lose blood */
	gain_condition(victim, COND_BLOODTHIRST, -1 - (victim->level / 20));
      if ( victim->hit <= (victim->max_hit / 8)
      && victim->pcdata->condition[COND_BLOODTHIRST]>5 )
      {
	gain_condition(victim, COND_BLOODTHIRST,
		-URANGE(3, victim->level / 10, 8) );
	victim->hit += URANGE( 4, (victim->max_hit / 30), 15);
	set_char_color(AT_BLOOD, victim);
	send_to_char("You howl with rage as the beast within stirs!\n\r", victim);
      }
    }

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
	if ( dt >= 0 && dt < top_sn )
	{
	    SKILLTYPE *skill = skill_table[dt];

	    if ( skill->die_char && skill->die_char[0] != '\0' )
	      act( AT_DEAD, skill->die_char, ch, NULL, victim, TO_CHAR );
	    if ( skill->die_vict && skill->die_vict[0] != '\0' )
	      act( AT_DEAD, skill->die_vict, ch, NULL, victim, TO_VICT );
	    if ( skill->die_room && skill->die_room[0] != '\0' )
	      act( AT_DEAD, skill->die_room, ch, NULL, victim, TO_NOTVICT );
	}
	act( AT_DEAD, "$n is DEAD!!", victim, 0, 0, TO_ROOM );
	act( AT_DEAD, "You have been KILLED!!\n\r", victim, 0, 0, TO_CHAR );
	break;

    default:
	/*
	 * Victim mentalstate affected, not attacker -- oops ;)
	 * Thanks to gfinello@mail.karmanet.it for finding this bug
	 */
	if ( dam > victim->max_hit / 4 )
	{
	   act( AT_HURT, "That really did HURT!", victim, 0, 0, TO_CHAR );
	   if ( number_bits(3) == 0 )
		worsen_mental_state( victim, 1 );
	}
	if ( victim->hit < victim->max_hit / 4 )

	{
	   act( AT_DANGER, "You wish that your wounds would stop BLEEDING so much!",
		victim, 0, 0, TO_CHAR );
	   if ( number_bits(2) == 0 )
		worsen_mental_state( victim, 1 );
	}
	break;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if ( !IS_AWAKE(victim)		/* lets make NPC's not slaughter PC's */
    &&   !IS_AFFECTED( victim, AFF_PARALYSIS ) )
    {
	if ( victim->fighting
	&&   victim->fighting->who->hunting
	&&   victim->fighting->who->hunting->who == victim )
	   stop_hunting( victim->fighting->who );

	if ( victim->fighting
	&&   victim->fighting->who->hating
	&&   victim->fighting->who->hating->who == victim )
	   stop_hating( victim->fighting->who );

	if (!npcvict && IS_NPC(ch))
	  stop_fighting( victim, TRUE );
	else
	  stop_fighting( victim, FALSE );
    }

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
	group_gain( ch, victim );

	if ( !npcvict )
	{
	    if ( !victim->desc )
        add_loginmsg( victim->name, 17, (IS_NPC(ch) ? ch->short_descr : ch->name) );
	    sprintf( log_buf, "%s (%d) killed by %s at %d",
		victim->name,
		victim->level,
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		victim->in_room->vnum );
	    log_string_plus( log_buf, LOG_ALL, 65 );
            to_channel( log_buf, CHANNEL_DEATH, "Death", LEVEL_IMMORTAL );
            if (!IS_NPC( ch ) && !IS_IMMORTAL( ch ) && ch->pcdata->clan
	    &&   ch->pcdata->clan->clan_type != CLAN_ORDER
            &&   ch->pcdata->clan->clan_type != CLAN_GUILD
            &&   victim != ch )
            {
                sprintf( filename, "%s%s.record", CLAN_DIR, ch->pcdata->clan->name );
                sprintf( log_buf, "&P(%2d) %-12s &wvs &P(%2d) %s &P%s ... &w%s",
		  ch->level,
                  ch->name,
		  victim->level,
		  !CAN_PKILL( victim ) ? "&W<Peaceful>" :
		    victim->pcdata->clan ? victim->pcdata->clan->badge :
		      "&P(&WUnclanned&P)",
		  victim->name,
                  ch->in_room->area->name );
		if ( victim->pcdata
		&&   victim->pcdata->clan
		&&   victim->pcdata->clan->name == ch->pcdata->clan->name )
		;
		else
		  append_to_file( filename, log_buf );
	    }
            if ( !IS_NPC( victim ) && !IS_IMMORTAL( victim ) && victim->pcdata->clan
            &&   victim->pcdata->clan->clan_type != CLAN_ORDER
            &&   victim->pcdata->clan->clan_type != CLAN_GUILD
            &&   ch != victim
	    &&   !IS_NPC( ch ) )
	    {
		sprintf( filename2, "%s%s.defeats", CLAN_DIR, victim->pcdata->clan->name );
                sprintf( log_buf, "&P(%2d) %-12s &wdefeated by &P(%2d) %s &P%s ... &w%s",
		  victim->level,
                  victim->name,
		  ch->level,
		  !CAN_PKILL( ch ) ? "&W<Peaceful>" :
		    ch->pcdata->clan ? ch->pcdata->clan->badge :
		      "&P(&WUnclanned&P)",
		  ch->name,
                  victim->in_room->area->name );
		if ( ch->pcdata
		&&   ch->pcdata->clan
		&&   ch->pcdata->clan->name == victim->pcdata->clan->name )
		;
		else
		  append_to_file( filename2, log_buf );
            }


	    /*
	     * Dying penalty:
	     * 1/2 way back to previous level.
	     */
	    if ( !IS_PKILL( victim ) ) /* August, 2000 */
	    {
	    if ( victim->exp > exp_level(victim, victim->level) )
		gain_exp( victim, (exp_level(victim, victim->level) - victim->exp)/2 );
	    }

	    /*
	     * New penalty... go back to the beginning of current level.
	     victim->exp = exp_level( victim, victim->level );
	     */
	}
	else
	if ( !IS_NPC(ch) )		/* keep track of mob vnum killed */
	    add_kill( ch, victim );

	check_killer( ch, victim );

	if ( !IS_NPC( ch ) && ch->pcdata->clan )
	   update_member( ch );
        if ( !IS_NPC( victim ) && victim->pcdata->clan )
           update_member( victim );

	if ( ch->in_room == victim->in_room )
	    loot = legal_loot( ch, victim );
	else
	    loot = FALSE;

	set_cur_char(victim);
	raw_kill( ch, victim );
	victim = NULL;

	if ( !IS_NPC(ch) && loot )
	{
	   /* Autogold by Scryn 8/12 */
	    if ( xIS_SET(ch->act, PLR_AUTOGOLD) )
	    {
		init_gold = ch->gold;
		do_get( ch, "coins corpse" );
		new_gold = ch->gold;
		gold_diff = (new_gold - init_gold);
		if (gold_diff > 0)
                {
                  sprintf(buf1,"%d",gold_diff);
		  do_split( ch, buf1 );
		} 
	    }
	    if ( xIS_SET(ch->act, PLR_AUTOLOOT)
	    &&   victim != ch )  /* prevent nasty obj problems -- Blodkai */
		do_get( ch, "all corpse" );
	    else
		do_look( ch, "in corpse" );

	    if ( xIS_SET(ch->act, PLR_AUTOSAC) )
		do_sacrifice( ch, "corpse" );
	}

	if ( IS_SET( sysdata.save_flags, SV_KILL ) )
	   save_char_obj( ch );
	return rVICT_DIED;
    }

    if ( victim == ch )
	return rNONE;

    /*
     * Take care of link dead people.
     */
    if ( !npcvict && !victim->desc  
    && !IS_SET( victim->pcdata->flags, PCFLAG_NORECALL ) )
    {
	if ( number_range( 0, victim->wait ) == 0)
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


/*
 * Changed is_safe to have the show_messg boolian.  This is so if you don't
 * want to show why you can't kill someone you can't turn it off.  This is
 * useful for things like area attacks.  --Shaddai
 */
bool is_safe( CHAR_DATA *ch, CHAR_DATA *victim, bool show_messg )
{
    if ( char_died(victim) || char_died(ch) )
    	return TRUE;

    /* Thx Josh! */
    if ( who_fighting( ch ) == ch )
	return FALSE;

    if ( !victim ) /*Gonna find this is_safe crash bug -Blod*/
    {
        bug( "Is_safe: %s opponent does not exist!", ch->name );
        return TRUE;
    }
    if ( !victim->in_room )
    {
	bug( "Is_safe: %s has no physical location!", victim->name );
	return TRUE;
    }

    if ( xIS_SET( victim->in_room->room_flags, ROOM_SAFE ) )
    {
        if ( show_messg ) {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "A magical force prevents you from attacking.\n\r", ch );
	}
	return TRUE;
    }
    
    if(IS_PACIFIST(ch)) /* Fireblade */
    {
        if ( show_messg ) {
    	set_char_color(AT_MAGIC, ch);
    	ch_printf(ch, "You are a pacifist and will not fight.\n\r");
	}
	return TRUE;
    }

    if ( IS_PACIFIST(victim) ) /* Gorog */
    {
        char buf[MAX_STRING_LENGTH];
        if ( show_messg ) {
        sprintf(buf, "%s is a pacifist and will not fight.\n\r",
                capitalize(victim->short_descr));
        set_char_color( AT_MAGIC, ch );
        send_to_char( buf, ch);
	}
        return TRUE;
    }

    if ( !IS_NPC( ch ) && ch->level >= LEVEL_IMMORTAL )
        return FALSE;

    if( !IS_NPC( ch ) && !IS_NPC( victim )
    &&   ch != victim 
    &&   IS_SET( victim->in_room->area->flags, AFLAG_NOPKILL ) ) 
    {
        if ( show_messg ) {
        set_char_color( AT_IMMORT, ch );
        send_to_char( "The gods have forbidden player killing in this area.\n\r", ch );
	}
        return TRUE;
    }
 
    if ( IS_NPC(ch) || IS_NPC(victim) )
	return FALSE;

    if ( get_age( ch ) < 18 || ch->level < 5 )
    {
        if ( show_messg ) {
	set_char_color( AT_WHITE, ch );	
	send_to_char( "You are not yet ready, needing age or experience, if not both. \n\r", ch );
	}
	return TRUE;
    }

    if ( get_age( victim ) < 18 || victim->level < 5 )
    {
        if ( show_messg ) {
	set_char_color( AT_WHITE, ch );
	send_to_char( "They are yet too young to die.\n\r", ch );
	}
	return TRUE;
    }

    if ( ch->level - victim->level > 5 
    ||   victim->level - ch->level > 5 )
    {
        if ( show_messg ) {
	set_char_color( AT_IMMORT, ch );
	send_to_char( "The gods do not allow murder when there is such a difference in level.\n\r", ch );
	}
	return TRUE;
    }

    if ( get_timer(victim, TIMER_PKILLED) > 0 )
    {
        if ( show_messg ) {
	set_char_color( AT_GREEN, ch );
        send_to_char( "That character has died within the last 5 minutes.\n\r", ch);
	}
        return TRUE;
    }
  
    if ( get_timer(ch, TIMER_PKILLED) > 0 )
    {
        if ( show_messg ) {
	set_char_color( AT_GREEN, ch );
        send_to_char( "You have been killed within the last 5 minutes.\n\r", ch );
	}
        return TRUE;
    }

    return FALSE;
}

/*
 * just verify that a corpse looting is legal
 */
bool legal_loot( CHAR_DATA *ch, CHAR_DATA *victim )
{
    /* anyone can loot mobs */
    if ( IS_NPC(victim) )
      return TRUE;
    /* non-charmed mobs can loot anything */
    if ( IS_NPC(ch) && !ch->master )
      return TRUE;
    /* members of different clans can loot too! -Thoric */
    if ( !IS_NPC(ch) && !IS_NPC(victim)
    &&    IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) 
    &&    IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ) ) 
	return TRUE;
    return FALSE;
}

/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
    /*
     * NPC's are fair game.
     */
    if ( IS_NPC(victim) )
    {
	if ( !IS_NPC( ch ) )
	{
	  int level_ratio;
	  level_ratio = URANGE( 1, ch->level / victim->level, 50);
	  if ( ch->pcdata->clan )
	    ch->pcdata->clan->mkills++;
	  ch->pcdata->mkills++;
	  ch->in_room->area->mkills++;
	  if ( ch->pcdata->deity ) 
	  {
	    if ( victim->race == ch->pcdata->deity->npcrace )
	      adjust_favor( ch, 3, level_ratio );
	    else
	      if ( victim->race == ch->pcdata->deity->npcfoe )
		adjust_favor( ch, 17, level_ratio );
	      else
                adjust_favor( ch, 2, level_ratio );
	  }
	}
	return;
    }



    /*
     * If you kill yourself nothing happens.
     */

    if ( ch == victim || ch->level >= LEVEL_IMMORTAL )
      return;

    /*
     * Checks to see if character is in an area flagged FREEKILL and then checks
     * whether combatants are within 10 honour of each other. This is all for
     * the purpose of awarding honour for pkills and the pkill data.
     * -- Raltaris
     */

    if ( IS_SET(ch->in_room->area->flags, AFLAG_FREEKILL )
    && abs( ch->pcdata->honour - victim->pcdata->honour ) <= 10 )
    {
		set_char_color( AT_IMMORT, ch );
		/* If loser/victim has no honour don't award or take any away. */
		if ( victim->pcdata->honour == 0 )
		  send_to_char( "You gain no honour for this kill. \n\r", ch );
		else
		{
	        /* Take care of the winner/ch, award honour, award pkill */
		  send_to_char( "You gain 1 honour for triumph in combat. \n\r", ch );
		  ch->pcdata->honour++;
		  send_to_char( "You lose 1 honour for defeat in combat. \n\r", victim );
		  victim->pcdata->honour--;
		}
		/* Now check to see if the winner has gained a new rank */
		if ( ch->pcdata->honour == 5
          || ch->pcdata->honour == 10
          || ch->pcdata->honour == 20
          || ch->pcdata->honour == 40
          || ch->pcdata->honour == 65
          || ch->pcdata->honour == 100 )
	      send_to_char( "Congratulations. You have obtained a new rank of honour. \n\r", ch );
    }

    /*
     * Any character in the arena is ok to kill, so are killers and thieves
     * Added pdeath and pkills here
     */
    if ( in_arena( ch ) 
	|| xIS_SET(victim->act, PLR_KILLER)
	|| xIS_SET(victim->act, PLR_THIEF) )
    {
    	if ( !IS_NPC(ch) && !IS_NPC(victim) )
		{
		  ch->pcdata->pkills++;
	  	  victim->pcdata->pdeaths++;
		}
      return;
    }


    /* clan checks					-Thoric */
    if ( !IS_NPC(ch) && !IS_NPC(victim)
    &&    IS_SET( ch->pcdata->flags, PCFLAG_DEADLY )
    &&    IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ) )
    {
      /* not of same clan? Go ahead and kill!!! */
      if ( !ch->pcdata->clan
      ||   !victim->pcdata->clan 
      ||   ( ch->pcdata->clan->clan_type != CLAN_NOKILL
      &&   victim->pcdata->clan->clan_type != CLAN_NOKILL
      &&   ch->pcdata->clan != victim->pcdata->clan ) )
      {
	if ( ch->pcdata->clan ) 
	{
            if ( victim->level < 10 )
              ch->pcdata->clan->pkills[0]++;
            else if ( victim->level < 15 )
              ch->pcdata->clan->pkills[1]++;
            else if ( victim->level < 20 )
              ch->pcdata->clan->pkills[2]++;
            else if ( victim->level < 30 )
              ch->pcdata->clan->pkills[3]++;
            else if ( victim->level < 40 )
              ch->pcdata->clan->pkills[4]++;
            else if ( victim->level < 50 )
              ch->pcdata->clan->pkills[5]++;
            else
              ch->pcdata->clan->pkills[6]++;
	}
	  ch->pcdata->pkills++;
	  ch->hit = ch->max_hit;
	  ch->mana = ch->max_mana;
	  ch->move = ch->max_move;
	if ( ch->pcdata )
	  ch->pcdata->condition[COND_BLOODTHIRST] = (10 + ch->level);
	update_pos(victim);
	if ( victim != ch )
	{
	  act( AT_MAGIC, "Bolts of blue energy rise from the corpse, seeping into $n.", ch, victim->name, NULL, TO_ROOM );
	  act( AT_MAGIC, "Bolts of blue energy rise from the corpse, seeping into you.", ch, victim->name, NULL, TO_CHAR ); 
	}
	if ( victim->pcdata->clan )
	{
            if ( ch->level < 10 )
              victim->pcdata->clan->pdeaths[0]++;
            else if ( ch->level < 15 )
              victim->pcdata->clan->pdeaths[1]++;
            else if ( ch->level < 20 )
              victim->pcdata->clan->pdeaths[2]++;
            else if ( ch->level < 30 )
              victim->pcdata->clan->pdeaths[3]++;
            else if ( ch->level < 40 )
              victim->pcdata->clan->pdeaths[4]++;
            else if ( ch->level < 50 )
              victim->pcdata->clan->pdeaths[5]++;
            else
              victim->pcdata->clan->pdeaths[6]++;
	}
	victim->pcdata->pdeaths++;   
	adjust_favor( victim, 11, 1 );
	adjust_favor( ch, 2, 1 );
	add_timer( victim, TIMER_PKILLED, 115, NULL, 0 );
	WAIT_STATE( victim, 3 * PULSE_VIOLENCE );
	/* xSET_BIT(victim->act, PLR_PK); */
	return;
      }
    }

    /*
     * Charm-o-rama.
     */
    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
	if ( !ch->master )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "Check_killer: %s bad AFF_CHARM",
		IS_NPC(ch) ? ch->short_descr : ch->name );
	    bug( buf, 0 );
	    affect_strip( ch, gsn_charm_person );
	    xREMOVE_BIT( ch->affected_by, AFF_CHARM );
	    return;
	}

	/* stop_follower( ch ); */
	if ( ch->master )
	  check_killer( ch->master, victim );
	return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * So is being immortal (Alander's idea).
     * And current killers stay as they are.
     */
    if ( IS_NPC(ch) )
    {
      if ( !IS_NPC(victim) )
      {
	int level_ratio;
        if ( victim->pcdata->clan )
          victim->pcdata->clan->mdeaths++;
        victim->pcdata->mdeaths++;
	victim->in_room->area->mdeaths++;
	level_ratio = URANGE( 1, ch->level / victim->level, 50 );
	if ( victim->pcdata->deity )
	{
	  if ( ch->race == victim->pcdata->deity->npcrace )
	    adjust_favor( victim, 12, level_ratio );
	  else
	    if ( ch->race == victim->pcdata->deity->npcfoe )
		adjust_favor( victim, 15, level_ratio );
	    else
	        adjust_favor( victim, 11, level_ratio );
	}
      }
      return;
    }

      
    if ( !IS_NPC(ch) )
    {
      if ( ch->pcdata->clan )
        ch->pcdata->clan->illegal_pk++;
      ch->pcdata->illegal_pk++;
      ch->in_room->area->illegal_pk++;
    }
    if ( !IS_NPC(victim) )
    {
      if ( victim->pcdata->clan )
      {
            if ( ch->level < 10 )
              victim->pcdata->clan->pdeaths[0]++;
            else if ( ch->level < 15 )
              victim->pcdata->clan->pdeaths[1]++;
            else if ( ch->level < 20 )
              victim->pcdata->clan->pdeaths[2]++;
            else if ( ch->level < 30 )
              victim->pcdata->clan->pdeaths[3]++;
            else if ( ch->level < 40 )
              victim->pcdata->clan->pdeaths[4]++;
            else if ( ch->level < 50 )
              victim->pcdata->clan->pdeaths[5]++;
            else
              victim->pcdata->clan->pdeaths[6]++;
      }
      victim->pcdata->pdeaths++;
      victim->in_room->area->pdeaths++;
    }

    if ( xIS_SET(ch->act, PLR_KILLER) )
      return;

    set_char_color( AT_WHITE, ch );
    send_to_char( "A strange feeling grows deep inside you, and a tingle goes up your spine...\n\r", ch );
    set_char_color( AT_IMMORT, ch );
    send_to_char( "A deep voice booms inside your head, 'Thou shall now be known as a deadly murderer!!!'\n\r", ch );
    set_char_color( AT_WHITE, ch );
    send_to_char( "You feel as if your soul has been revealed for all to see.\n\r", ch );
    xSET_BIT(ch->act, PLR_KILLER);
    if ( xIS_SET( ch->act, PLR_ATTACKER) )
      xREMOVE_BIT(ch->act, PLR_ATTACKER);
    save_char_obj( ch );
    return;
}

/*
 * See if an attack justifies a ATTACKER flag.
 */
void check_attacker( CHAR_DATA *ch, CHAR_DATA *victim )
{

/* 
 * Made some changes to this function Apr 6/96 to reduce the prolifiration
 * of attacker flags in the realms. -Narn
 */
    /*
     * NPC's are fair game.
     * So are killers and thieves.
     */
    if ( IS_NPC(victim)
    ||  xIS_SET(victim->act, PLR_KILLER)
    ||  xIS_SET(victim->act, PLR_THIEF) )
	return;

    /* deadly char check */
    if ( !IS_NPC(ch) && !IS_NPC(victim)
         && CAN_PKILL( ch ) && CAN_PKILL( victim ) )
	return;

/* Pkiller versus pkiller will no longer ever make an attacker flag
    { if ( !(ch->pcdata->clan && victim->pcdata->clan
      && ch->pcdata->clan == victim->pcdata->clan ) )  return; }
*/

    /*
     * Charm-o-rama.
     */
    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
	if ( !ch->master )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "Check_attacker: %s bad AFF_CHARM",
		IS_NPC(ch) ? ch->short_descr : ch->name );
	    bug( buf, 0 );
	    affect_strip( ch, gsn_charm_person );
	    xREMOVE_BIT( ch->affected_by, AFF_CHARM );
	    return;
	}

        /* Won't have charmed mobs fighting give the master an attacker 
           flag.  The killer flag stays in, and I'll put something in 
           do_murder. -Narn */
	/* xSET_BIT(ch->master->act, PLR_ATTACKER);*/
	/* stop_follower( ch ); */
	return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * So is being immortal (Alander's idea).
     * And current killers stay as they are.
     */
    if ( IS_NPC(ch)
    ||   ch == victim
    ||   ch->level >= LEVEL_IMMORTAL
    ||   xIS_SET(ch->act, PLR_ATTACKER)
    ||   xIS_SET(ch->act, PLR_KILLER) )
	return;

    xSET_BIT(ch->act, PLR_ATTACKER);
    save_char_obj( ch );
    return;
}


/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( !victim )
    {
      bug( "update_pos: null victim", 0 );
      return;
    }

    if ( victim->hit > 0 )
    {
	if ( victim->position <= POS_STUNNED 
  	     && !IS_AFFECTED( victim, AFF_PARALYSIS ))
        {
        int gm_stance = FALSE;
        victim->position = POS_STANDING;
        if ( ( get_stance_mastery ( victim ) >= 200 ) )
                gm_stance = TRUE;
        if ( ((victim->max_hit/4) > victim->hit) || !gm_stance )
        {
          act ( AT_ACTION, "$n clambers back to $s feet.", victim, NULL, NULL, TO_ROOM);
          act( AT_ACTION, "You clamber back to your feet.", victim, NULL, NULL, TO_CHAR);
        }
        else
        {
          act( AT_ACTION, "$n flips back up to $s feet.", victim, NULL, NULL, TO_ROOM);
          act( AT_ACTION, "You flip back up to your feet.", victim, NULL, NULL, TO_CHAR);
        }
      }
      if ( IS_AFFECTED( victim, AFF_PARALYSIS ) )
	  victim->position = POS_STUNNED;
	return;
    }

    if ( IS_NPC(victim) || victim->hit <= -11 )
    {
	if ( victim->mount )
	{
	  act( AT_ACTION, "$n falls from $N.",
		victim, NULL, victim->mount, TO_ROOM );
	  xREMOVE_BIT( victim->mount->act, ACT_MOUNTED );
	  victim->mount = NULL;
	}
	victim->position = POS_DEAD;
	return;
    }

	 if ( victim->hit <= -6 ) victim->position = POS_MORTAL;
    else if ( victim->hit <= -3 ) victim->position = POS_INCAP;
    else                          victim->position = POS_STUNNED;

    if ( victim->position > POS_STUNNED
    &&   IS_AFFECTED( victim, AFF_PARALYSIS ) )
      victim->position = POS_STUNNED;

    if ( victim->mount )
    {
	act( AT_ACTION, "$n falls unconscious from $N.",
		victim, NULL, victim->mount, TO_ROOM );
	xREMOVE_BIT( victim->mount->act, ACT_MOUNTED );
	victim->mount = NULL;
    }
    return;
}


/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    FIGHT_DATA *fight;

    if ( ch->fighting )
    {
	/* Spam!  Spammity spam!  Spammity spam spamalicious spam!
	char buf[MAX_STRING_LENGTH];

	sprintf( buf, "Set_fighting: %s -> %s (already fighting %s)",
		ch->name, victim->name, ch->fighting->who->name );
	bug( buf, 0 );
	*/
	return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
	affect_strip( ch, gsn_sleep );

    /* Limit attackers -Thoric */
    if ( victim->num_fighting > max_fight(victim) )
    {
	send_to_char( "There are too many people fighting for you to join in.\n\r", ch );
	return;
    }

    CREATE( fight, FIGHT_DATA, 1 );
    fight->who	 = victim;
    fight->xp	 = (int) xp_compute( ch, victim ) * 0.85;
    fight->align = align_compute( ch, victim );
    if ( !IS_NPC(ch) && IS_NPC(victim) )
      fight->timeskilled = times_killed(ch, victim);
    ch->num_fighting = 1;
    ch->fighting = fight;
    /* ch->position = POS_FIGHTING; */
	if ( IS_NPC(ch) )
		ch->position = POS_FIGHTING;
	else
	switch(ch->style)
	{
		case(STYLE_EVASIVE): 
			ch->position = POS_EVASIVE;
			break;
		case(STYLE_DEFENSIVE): 
			ch->position = POS_DEFENSIVE;
			break;
		case(STYLE_AGGRESSIVE): 
			ch->position = POS_AGGRESSIVE;
			break;
		case(STYLE_BERSERK): 
			ch->position = POS_BERSERK;
			break;
		default: ch->position = POS_FIGHTING;
	}
    victim->num_fighting++;
#ifdef SHADDAI
    if (IS_NPC (ch) && ch->stance < STANCE_NORMAL)
     if (ch->pIndexData->stances[0] > STANCE_NONE)
      do_stance (ch, get_stance_name (ch->pIndexData->stances[0]));
    if (IS_NPC (victim) && victim->stance < STANCE_NORMAL)
     if (victim->pIndexData->stances[0] > STANCE_NONE)
      do_stance (victim, get_stance_name (victim->pIndexData->stances[0]));
#endif
    if (victim->switched && IS_AFFECTED (victim->switched, AFF_POSSESS))
    {
      send_to_char ("You are disturbed!\n\r", victim->switched);
      do_return (victim->switched, "");
    }
    return;
}


CHAR_DATA *who_fighting( CHAR_DATA *ch )
{
    if ( !ch )
    {
	bug( "who_fighting: null ch", 0 );
	return NULL;
    }
    if ( !ch->fighting )
      return NULL;
    return ch->fighting->who;
}

void free_fight( CHAR_DATA *ch )
{
   if ( !ch )
   {
	bug( "Free_fight: null ch!", 0 );
	return;
   }
   if ( ch->fighting )
   {
     if ( !char_died(ch->fighting->who) )
       --ch->fighting->who->num_fighting;
     DISPOSE( ch->fighting );
   }
   ch->fighting = NULL;
   if ( ch->mount )
     ch->position = POS_MOUNTED;
   else
     ch->position = POS_STANDING;
   /* Berserk wears off after combat. -- Altrag */
   if ( IS_AFFECTED(ch, AFF_BERSERK) )
   {
     affect_strip(ch, gsn_berserk);
     set_char_color(AT_WEAROFF, ch);
     send_to_char(skill_table[gsn_berserk]->msg_off, ch);
     send_to_char("\n\r", ch);
   }
/* 
   if ( IS_AFFECTED( ch, AFF_GRAPPLE ) )
   {
     affect_strip( ch, gsn_grapple );
     set_char_color( AT_WEAROFF, ch );
     send_to_char( skill_table[gsn_grapple]->msg_off, ch );
     send_to_char( "\n\r", ch );
   }
*/
   return;
}


/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;

    strip_grapple( ch );
    free_fight( ch );
    update_pos( ch );

    if ( !fBoth )   /* major short cut here by Thoric */
      return;

    for ( fch = first_char; fch; fch = fch->next )
    {
	if ( who_fighting( fch ) == ch )
	{
	    free_fight( fch );
	    update_pos( fch );
	}
    }
    return;
}

/* Vnums for the various bodyparts */
int part_vnums[] =
{	12,	/* Head */
	14,	/* arms */
	15,	/* legs */
	13,	/* heart */
	44,	/* brains */
	16,	/* guts */
	45,	/* hands */
	46,	/* feet */
	47,	/* fingers */
	48,	/* ear */
	49,	/* eye */
	50,	/* long_tongue */
	51,	/* eyestalks */
	52,	/* tentacles */
	53,	/* fins */
	54,	/* wings */
	55,	/* tail */
	56,	/* scales */
	59,	/* claws */
	87,	/* fangs */
	58,	/* horns */
	57,	/* tusks */
	55,	/* tailattack */
	85,	/* sharpscales */
	84,	/* beak */
	86,	/* haunches */
	83,	/* hooves */
	82,	/* paws */
	81,	/* forelegs */
	80,	/* feathers */
	120050,	/* husk_shell */
	0	/* r2 */
};

/* Messages for flinging off the various bodyparts */
char* part_messages[] =
{
	"$n's severed head plops from its neck.",
	"$n's arm is sliced from $s dead body.",
	"$n's leg is sliced from $s dead body.",
	"$n's heart is torn from $s chest.",
	"$n's brains spill grotesquely from $s head.",
	"$n's guts spill grotesquely from $s torso.",
	"$n's hand is sliced from $s dead body.",
	"$n's foot is sliced from $s dead body.",
	"A finger is sliced from $n's dead body.",
	"$n's ear is sliced from $s dead body.",
	"$n's eye is gouged from its socket.",
	"$n's tongue is torn from $s mouth.",
	"An eyestalk is sliced from $n's dead body.",
	"A tentacle is severed from $n's dead body.",
	"A fin is sliced from $n's dead body.",
	"A wing is severed from $n's dead body.",
	"$n's tail is sliced from $s dead body.",
	"A scale falls from the body of $n.",
	"A claw is torn from $n's dead body.",
	"$n's fangs are torn from $s mouth.",
	"A horn is wrenched from the body of $n.",
	"$n's tusk is torn from $s dead body.",
	"$n's tail is sliced from $s dead body.",
	"A ridged scale falls from the body of $n.",
	"$n's beak is sliced from $s dead body.",
	"$n's haunches are sliced from $s dead body.",
	"A hoof is sliced from $n's dead body.",
	"A paw is sliced from $n's dead body.",
	"$n's foreleg is sliced from $s dead body.",
	"Some feathers fall from $n's dead body.",
	"$n's shell remains.",
	"r2 message."
};
			
/*
 * Improved Death_cry contributed by Diavolo.
 * Additional improvement by Thoric (and removal of turds... sheesh!)  
 * Support for additional bodyparts by Fireblade
 */
void death_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    EXIT_DATA *pexit;
    int vnum, shift, index, i;

    if ( !ch )
    {
      bug( "DEATH_CRY: null ch!", 0 );
      return;
    }

    vnum = 0;
    msg = NULL;
    
    switch ( number_range(0, 5) )
    {
    default: msg  = "You hear $n's death cry.";				break;
    case  0:
      msg = "$n screams furiously as $e falls to the ground in a heap!"; break;
    case  1:
      msg  = "$n hits the ground ... DEAD.";			        break;
    case  2:
      msg = "$n catches $s guts in $s hands as they pour through $s fatal"
            " wound!";                                                  break;
    case  3: msg  = "$n splatters blood on your armor.";		break;
    case  4: msg  = "$n gasps $s last breath and blood spurts out of $s "
                    "mouth and ears.";                                  break;
    case  5:
    	shift = number_range(0, 31);
    	index = 1 << shift;
    	
       	for(i = 0;i < 32 && ch->xflags;i++)
    	{
    		if(HAS_BODYPART(ch, index))
    		{
    			msg = part_messages[shift];
    			vnum = part_vnums[shift];
    			break;
    		}
    		else
    		{
    			shift = number_range(0, 31);
    			index = 1 << shift;
    		}
    	}
    	
    	if(!msg)
    		msg = "You hear $n's death cry.";
    	break;
    }

    act( AT_CARNAGE, msg, ch, NULL, NULL, TO_ROOM );

    if ( vnum )
    {
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	char *name;

	if(!get_obj_index(vnum))
	{
		bug("death_cry: invalid vnum", 0);
		return;
	}

	name		= IS_NPC(ch) ? ch->short_descr : ch->name;
	obj		= create_object( get_obj_index( vnum ), 0 );
	obj->timer	= number_range( 4, 7 );
	if ( IS_AFFECTED( ch, AFF_POISON ) )
	  obj->value[3] = 10;

	sprintf( buf, obj->short_descr, name );
	STRFREE( obj->short_descr );
	obj->short_descr = STRALLOC( buf );

	sprintf( buf, obj->description, name );
	STRFREE( obj->description );
	obj->description = STRALLOC( buf );

	obj = obj_to_room( obj, ch->in_room );
    }

    if ( IS_NPC(ch) )
	msg = "You hear something's death cry.";
    else
	msg = "You hear someone's death cry.";

    was_in_room = ch->in_room;
    for ( pexit = was_in_room->first_exit; pexit; pexit = pexit->next )
    {
	if ( pexit->to_room
	&&   pexit->to_room != was_in_room )
	{
	    ch->in_room = pexit->to_room;
	    act( AT_CARNAGE, msg, ch, NULL, NULL, TO_ROOM );
	}
    }
    ch->in_room = was_in_room;

    return;
}



void raw_kill( CHAR_DATA *ch, CHAR_DATA *victim )
{

    if ( !victim )
    {
      bug( "raw_kill: null victim!", 0 );
      return;
    }
/* backup in case hp goes below 1 */
    if (NOT_AUTHED(victim))
    {
      bug( "raw_kill: killing unauthed", 0 );
      return;
    }

    stop_fighting( victim, TRUE );

    /* Take care of morphed characters */
    if(victim->morph)
    {
      do_unmorph_char( victim );
      raw_kill(ch, victim);
      return;
    }

    mprog_death_trigger( ch, victim );
    if ( char_died(victim) )
      return;
 /* death_cry( victim ); */

    rprog_death_trigger( ch, victim );
    if ( char_died(victim) )
      return;

    make_corpse( victim, ch );
    if ( victim->in_room->sector_type == SECT_OCEANFLOOR
    ||   victim->in_room->sector_type == SECT_UNDERWATER
    ||   victim->in_room->sector_type == SECT_WATER_SWIM
    ||   victim->in_room->sector_type == SECT_WATER_NOSWIM )
      act( AT_BLOOD, "$n's blood slowly clouds the surrounding water.", victim, NULL, NULL, TO_ROOM );
    else if ( victim->in_room->sector_type == SECT_AIR )
      act( AT_BLOOD, "$n's blood sprays wildly through the air.", victim, NULL, NULL, TO_ROOM );
    else
      make_blood( victim );

    if ( IS_NPC(victim) )
    {
        #ifdef MYSTARIC
        /* Bug fix below since was using pIndexData on non NPC's -Shaddai */
  	if( victim->pIndexData->pCasino )
   	{
      	   CASINO_DATA *casino;
      	   casino = victim->pIndexData->pCasino;
      	   UNLINK( casino, first_casino, last_casino, next, prev );
   	}
	#endif
	
	victim->pIndexData->killed++;
	extract_char( victim, TRUE );
	victim = NULL;
	return;
    }

    set_char_color( AT_DIEMSG, victim );
    if ( victim->pcdata->mdeaths + victim->pcdata->pdeaths < 3 )
      do_help( victim, "new_death" );
    else
      do_help( victim, "_DIEMSG_" );

    extract_char( victim, FALSE );
    if ( !victim )
    {
      bug( "oops! raw_kill: extract_char destroyed pc char", 0 );
      return;
    }
    while ( victim->first_affect )
	affect_remove( victim, victim->first_affect );
    victim->affected_by	= race_table[victim->race]->affected;
    victim->resistant   = 0;
    victim->susceptible = 0;
    victim->immune      = 0;
    victim->carry_weight= 0;
    victim->armor	= 100;
    victim->armor	+= race_table[victim->race]->ac_plus;
    victim->attacks	= race_table[victim->race]->attacks;
    victim->defenses	= race_table[victim->race]->defenses;
    victim->mod_str	= 0;
    victim->mod_dex	= 0;
    victim->mod_wis	= 0;
    victim->mod_int	= 0;
    victim->mod_con	= 0;
    victim->mod_cha	= 0;
    victim->mod_lck   	= 0;
    victim->damroll	= 0;
    victim->hitroll	= 0;
    victim->mental_state = -10;
    victim->alignment	= URANGE( -1000, victim->alignment, 1000 );
/*  victim->alignment		= race_table[victim->race]->alignment;
-- switched lines just for now to prevent mortals from building up
days of bellyaching about their angelic or satanic humans becoming
neutral when they die given the difficulting of changing align */

    victim->saving_poison_death = race_table[victim->race]->saving_poison_death;
    victim->saving_wand 	= race_table[victim->race]->saving_wand;
    victim->saving_para_petri 	= race_table[victim->race]->saving_para_petri;
    victim->saving_breath 	= race_table[victim->race]->saving_breath;
    victim->saving_spell_staff 	= race_table[victim->race]->saving_spell_staff;
    victim->position	= POS_RESTING;
    victim->hit		= UMAX( 1, victim->hit  );
    /* Shut down some of those naked spammer killers - Blodkai */
    if ( victim->level < 50 )
	victim->mana	= UMAX( 1, victim->mana );
    else
	victim->mana	= 1;
    victim->move	= UMAX( 1, victim->move );

    /*
     * Pardon crimes...						-Thoric
     */
    if ( xIS_SET( victim->act, PLR_KILLER) )
    {
      xREMOVE_BIT( victim->act, PLR_KILLER);
      send_to_char("The gods have pardoned you for your murderous acts.\n\r",victim);
    }
    if ( xIS_SET( victim->act, PLR_THIEF) )
    {
      xREMOVE_BIT( victim->act, PLR_THIEF);
      send_to_char("The gods have pardoned you for your thievery.\n\r",victim);
    }
    victim->pcdata->condition[COND_FULL]   = 12;
    victim->pcdata->condition[COND_THIRST] = 12;
    if ( IS_VAMPIRE( victim ) )
      victim->pcdata->condition[COND_BLOODTHIRST] = (victim->level / 2);

    if ( IS_SET( sysdata.save_flags, SV_DEATH ) )
	save_char_obj( victim );
    return;
}



void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    int xp;
    int members;

    /*
     * Monsters don't get kill xp's or alignment changes.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if ( IS_NPC(ch) || victim == ch )
	return;

    members = 0;
    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) )
	    members++;
    }

    if ( members == 0 )
    {
	bug( "Group_gain: members.", members );
	members = 1;
    }

    lch = ch->leader ? ch->leader : ch;

    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if ( !is_same_group( gch, ch ) )
	    continue;

	if ( gch->level - lch->level >  8 )
	{
	    send_to_char( "You are too high for this group.\n\r", gch );
	    continue;
	}

	if ( gch->level - lch->level < -8 )
	{
	    send_to_char( "You are too low for this group.\n\r", gch );
	    continue;
	}

	xp = (int) (xp_compute( gch, victim ) * 0.1765) / members;
	if ( !gch->fighting )
	  xp /= 2;
	gch->alignment = align_compute( gch, victim );
	sprintf( buf, "You receive %d experience points.\n\r", xp );
	send_to_char( buf, gch );
	gain_exp( gch, xp );

	for ( obj = ch->first_carrying; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE )
		continue;

	    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
	    {
		act( AT_MAGIC, "You are zapped by $p.", ch, obj, NULL, TO_CHAR );
		act( AT_MAGIC, "$n is zapped by $p.",   ch, obj, NULL, TO_ROOM );
       		obj_from_char( obj );
		if ( in_arena( ch ) )
		  obj = obj_to_char( obj, ch );
		else
		  obj = obj_to_room( obj, ch->in_room );
		oprog_zap_trigger(ch, obj);  /* mudprogs */
		if ( char_died(ch) )
		  return;
	    }
	}
    }

    return;
}


int align_compute( CHAR_DATA *gch, CHAR_DATA *victim )
{
    int align, newalign, divalign;

    align = gch->alignment - victim->alignment;

    /* slowed movement in good & evil ranges by a factor of 5, h */
    /* Added divalign to keep neutral chars shifting faster -- Blodkai */
    /* This is obviously gonna take a lot more thought */

    if ( gch->alignment > -350 && gch->alignment < 350 )
	divalign = 4;
    else
	divalign = 20;
    
    if ( align >  500 )
	newalign  = UMIN( gch->alignment + (align-500)/divalign,  1000 );
    else
    if ( align < -500 )
	newalign  = UMAX( gch->alignment + (align+500)/divalign, -1000 );
    else
	newalign  = gch->alignment - (int) (gch->alignment/divalign);

    return newalign;
}


/*
 * Calculate how much XP gch should gain for killing victim
 * Lots of redesigning for new exp system by Thoric
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim )
{
    int align;
    int xp;
    int xp_ratio;
    int gchlev = gch->level;

    xp	  = (get_exp_worth( victim )
    	  *  URANGE( 0, (victim->level - gchlev) + 10, 13 )) / 10;
    align = gch->alignment - victim->alignment;

    /* bonus for attacking opposite alignment */
    if ( align >  990 || align < -990 )
	xp = (xp*5) >> 2;
    else
    /* penalty for good attacking same alignment */
    if ( gch->alignment > 300 && align < 250 )
	xp = (xp*3) >> 2;

    xp = number_range( (xp*3) >> 2, (xp*5) >> 2 );

    /* get 1/4 exp for players					-Thoric */
    if ( !IS_NPC( victim ) )
      xp /= 4;
    else
    /* reduce exp for killing the same mob repeatedly		-Thoric */
    if ( !IS_NPC( gch ) )
    {
	int times = times_killed( gch, victim );

	if ( times >= 20 )
	   xp = 0;
	else
	if ( times )
	{
	   xp = (xp * (20-times)) / 20;
	   if ( times > 15 )
	     xp /= 3;
	   else
	   if ( times > 10 )
	     xp >>= 1;
	}
    }

    /*
     * semi-intelligent experienced player vs. novice player xp gain
     * "bell curve"ish xp mod by Thoric
     * based on time played vs. level
     */
    if ( !IS_NPC( gch ) && gchlev > 5 )
    {
	xp_ratio = (int) gch->played / gchlev;
	if ( xp_ratio > 20000 )		/* 5/4 */
	    xp = (xp*5) >> 2;
	else
	if ( xp_ratio < 16000 )		/* 3/4 */
	    xp = (xp*3) >> 2;
	else
	if ( xp_ratio < 10000 )		/* 1/2 */
	    xp >>= 1;
	else
	if ( xp_ratio < 5000 )		/* 1/4 */
	    xp >>= 2;
	else
	if ( xp_ratio < 3500 )		/* 1/8 */
	    xp >>= 3;
	else
	if ( xp_ratio < 2000 )		/* 1/16 */
	    xp >>= 4;
    }

    /*
     * Level based experience gain cap.  Cannot get more experience for
     * a kill than the amount for your current experience level   -Thoric
     */
    return URANGE(0, xp, exp_level(gch, gchlev+1) - exp_level(gch, gchlev) );
}


/*
 * Revamped by Thoric to be more realistic
 * Added code to produce different messages based on weapon type - FB
 * Added better bug message so you can track down the bad dt's -Shaddai
 */
void new_dam_message( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, OBJ_DATA *obj )
{
    char buf1[256], buf2[256], buf3[256];
    char bugbuf[MAX_STRING_LENGTH];
    const char *vs;
    const char *vp;
    const char *attack;
    char punct;
    sh_int dampc;
    struct skill_type *skill = NULL;
    bool gcflag = FALSE;
    bool gvflag = FALSE;
    int d_index, w_index;
    ROOM_INDEX_DATA *was_in_room;
    
    if ( ! dam )
	dampc = 0;
    else
	dampc = ( (dam * 1000) / victim->max_hit) +
              ( 50 - ((victim->hit * 50) / victim->max_hit) );

    if ( ch->in_room != victim->in_room )
    {
	was_in_room = ch->in_room;
	char_from_room(ch);
	char_to_room(ch, victim->in_room);
    }
    else
	was_in_room = NULL;

    /* Get the weapon index */
    if ( dt > 0 && dt < top_sn )
    {
    	w_index = 0;
    }
    else
    if ( dt >= TYPE_HIT && dt < TYPE_HIT + sizeof(attack_table)/sizeof(attack_table[0]) )
    {
   	w_index = dt - TYPE_HIT;
    }
    else
    {
	sprintf(bugbuf, "Dam_message: bad dt %d from %s in %d.",
		dt, ch->name, ch->in_room->vnum );
	bug( bugbuf, 0);
   	dt = TYPE_HIT;
   	w_index = 0;
    }

    /* get the damage index */
    if(dam == 0)
    	d_index = 0;
    else if(dampc < 0)
    	d_index = 1;
    else if(dampc <= 100)
   	d_index = 1 + dampc/10;
    else if(dampc <= 200)
   	d_index = 11 + (dampc - 100)/20;
    else if(dampc <= 900)
   	d_index = 16 + (dampc - 200)/100;
    else
   	d_index = 23;
   
    /* Lookup the damage message */
    vs = s_message_table[w_index][d_index];
    vp = p_message_table[w_index][d_index];
   
    punct   = (dampc <= 30) ? '.' : '!';

    if ( dam == 0 && (!IS_NPC(ch) && 
       (IS_SET(ch->pcdata->flags, PCFLAG_GAG)))) gcflag = TRUE;

    if ( dam == 0 && (!IS_NPC(victim) &&
       (IS_SET(victim->pcdata->flags, PCFLAG_GAG)))) gvflag = TRUE;

    if ( dt >=0 && dt < top_sn )
	skill = skill_table[dt];
    if ( dt == TYPE_HIT )
    {
	sprintf( buf1, "$n %s $N%c",  vp, punct );
	sprintf( buf2, "You %s $N%c", vs, punct );
	sprintf( buf3, "$n %s you%c", vp, punct );
    }
    else
    if ( dt > TYPE_HIT && is_wielding_poisoned( ch ) )
    {
	if ( dt < TYPE_HIT + sizeof(attack_table)/sizeof(attack_table[0]) )
	    attack	= attack_table[dt - TYPE_HIT];
	else
	{
         sprintf(bugbuf, "Dam_message: bad dt %d from %s in %d.",
                dt, ch->name, ch->in_room->vnum );
         bug( bugbuf, 0);
	    dt  = TYPE_HIT;
	    attack  = attack_table[0];
        }

	sprintf( buf1, "$n's poisoned %s %s $N%c", attack, vp, punct );
	sprintf( buf2, "Your poisoned %s %s $N%c", attack, vp, punct );
	sprintf( buf3, "$n's poisoned %s %s you%c", attack, vp, punct ); 
    }
    else
    {
	if ( skill )
	{
	    attack	= skill->noun_damage;
	    if ( dam == 0 )
	    {
		bool found = FALSE;

		if ( skill->miss_char && skill->miss_char[0] != '\0' )
		{
		   act( AT_HIT, skill->miss_char, ch, NULL, victim, TO_CHAR );
		   found = TRUE;
		}
		if ( skill->miss_vict && skill->miss_vict[0] != '\0' )
		{
		   act( AT_HITME, skill->miss_vict, ch, NULL, victim, TO_VICT );
		   found = TRUE;
		}
		if ( skill->miss_room && skill->miss_room[0] != '\0' )
		{
		   if (strcmp( skill->miss_room,"supress" ) )
			act( AT_ACTION, skill->miss_room, ch, NULL, victim, TO_NOTVICT );
		   found = TRUE;
		}
		if ( found )	/* miss message already sent */
		{
		   if ( was_in_room )
		   {
			char_from_room(ch);
			char_to_room(ch, was_in_room);
		   }
		   return;
		}
	    }
	    else
	    {
		if ( skill->hit_char && skill->hit_char[0] != '\0' )
		  act( AT_HIT, skill->hit_char, ch, NULL, victim, TO_CHAR );
		if ( skill->hit_vict && skill->hit_vict[0] != '\0' )
		  act( AT_HITME, skill->hit_vict, ch, NULL, victim, TO_VICT );
		if ( skill->hit_room && skill->hit_room[0] != '\0' )
		  act( AT_ACTION, skill->hit_room, ch, NULL, victim, TO_NOTVICT );
	    }
	}
	else if ( dt >= TYPE_HIT
	&& dt < TYPE_HIT + sizeof(attack_table)/sizeof(attack_table[0]) )
	{
	    if ( obj )
		attack = obj->short_descr;
	    else
		attack = attack_table[dt - TYPE_HIT];
	}
	else
	{
            sprintf(bugbuf, "Dam_message: bad dt %d from %s in %d.",
                dt, ch->name, ch->in_room->vnum );
            bug( bugbuf, 0);
	    dt  = TYPE_HIT;
	    attack  = attack_table[0];
	}

	sprintf( buf1, "$n's %s %s $N%c",  attack, vp, punct );
	sprintf( buf2, "Your %s %s $N%c",  attack, vp, punct );
	sprintf( buf3, "$n's %s %s you%c", attack, vp, punct );
    }


    act( AT_ACTION, buf1, ch, NULL, victim, TO_NOTVICT );
    if (!gcflag)  act( AT_HIT, buf2, ch, NULL, victim, TO_CHAR );
    if (!gvflag)  act( AT_HITME, buf3, ch, NULL, victim, TO_VICT );

   if ( was_in_room )
   {
	char_from_room(ch);
	char_to_room(ch, was_in_room);
   }
    return;
}

#ifndef dam_message
void dam_message( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
    new_dam_message(ch, victim, dam, dt);
}
#endif

void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Kill whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) && victim->morph)
    {
	send_to_char("This creature appears strange to you.  Look upon it more closely before attempting to kill it.", ch);
	return;
    }

    if ( !IS_NPC(victim) )
    {
	if ( !xIS_SET(victim->act, PLR_KILLER)
	&&   !xIS_SET(victim->act, PLR_THIEF) )
	{
	    send_to_char( "You must MURDER a player.\n\r", ch );
	    return;
	}
    }

   /*
    *
    else
    {
	if ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL )
	{
	    send_to_char( "You must MURDER a charmed creature.\n\r", ch );
	    return;
	}
    }
    *
    */

    if ( victim == ch )
    {
	send_to_char( "You hit yourself.  Ouch!\n\r", ch );
	multi_hit( ch, ch, TYPE_UNDEFINED );
	return;
    }

    if ( is_safe( ch, victim, TRUE ) )
	return;

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
    act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING 
       || ch->position ==  POS_EVASIVE
       || ch->position ==  POS_DEFENSIVE
       || ch->position ==  POS_AGGRESSIVE
       || ch->position ==  POS_BERSERK
       )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    check_attacker( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_murde( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to MURDER, spell it out.\n\r", ch );
    return;
}



void do_murder( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Murder whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim, TRUE ) )
	return;

    if ( IS_AFFECTED(ch, AFF_CHARM) ) 
    {
      if ( ch->master == victim )
      {
        act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
      }
      else
      {
        if ( ch->master )
          xSET_BIT(ch->master->act, PLR_ATTACKER);
      }
    }

    if ( ch->position == POS_FIGHTING 
       || ch->position ==  POS_EVASIVE
       || ch->position ==  POS_DEFENSIVE
       || ch->position ==  POS_AGGRESSIVE
       || ch->position ==  POS_BERSERK
       )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }

    if ( !IS_NPC( victim ) && xIS_SET(ch->act, PLR_NICE ) )
    {
      send_to_char( "You feel too nice to do that!\n\r", ch );
      return;
    }
/*
    if ( !IS_NPC( victim ) && xIS_SET(victim->act, PLR_PK ) )
*/

    if (!IS_NPC (victim))
       {
       sprintf( log_buf, "%s: murder %s.", ch->name, victim->name );
       log_string_plus( log_buf, LOG_ALL, 65 );
       }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    sprintf( buf, "Help!  I am being attacked by %s!", 
                   IS_NPC( ch ) ? ch->short_descr : ch->name );
    if ( IS_PKILL(victim) )
	do_wartalk( victim, buf );
    else
	do_yell( victim, buf );
    check_illegal_pk( ch, victim );
    check_attacker( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}

/*
 * Check to see if the player is in an "Arena".
 */
bool in_arena( CHAR_DATA *ch )
{
    if ( xIS_SET(ch->in_room->room_flags, ROOM_ARENA) )
	return TRUE;
    if ( IS_SET(ch->in_room->area->flags, AFLAG_FREEKILL) )
	return TRUE;
    if ( ch->in_room->vnum >= 29 && ch->in_room->vnum <= 43 )
	return TRUE;
    if ( !str_cmp(ch->in_room->area->filename, "arena.are") )
	return TRUE;

    return FALSE;
}

bool check_illegal_pk( CHAR_DATA *ch, CHAR_DATA *victim )
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  if ( !IS_NPC(victim) && !IS_NPC(ch) )
  {
	if ( ( !IS_SET(victim->pcdata->flags, PCFLAG_DEADLY)
	|| ch->level - victim->level > 10 
	|| !IS_SET(ch->pcdata->flags, PCFLAG_DEADLY) ) 
	&& !in_arena(ch)
	&& ch != victim 
	&& !( IS_IMMORTAL(ch) && IS_IMMORTAL(victim) ) )
	{
	    if ( IS_NPC(ch) )
		sprintf(buf, " (%s)", ch->name);
	    if ( IS_NPC(victim) )
		sprintf(buf2, " (%s)", victim->name);

	    sprintf( log_buf, "&p%s on %s%s in &G&R***&rILLEGAL PKILL&G&R*** &pattempt at %d",
		(lastplayercmd),
		(IS_NPC(victim) ? victim->short_descr : victim->name),
		(IS_NPC(victim) ? buf2 : ""),
		victim->in_room->vnum );
	    last_pkroom = victim->in_room->vnum;
	    log_string(log_buf);
	    to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
	    return TRUE;
	}
    }
    return FALSE;
}


void do_flee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    char buf[MAX_STRING_LENGTH];
    int attempt, los;
    sh_int door;
    EXIT_DATA *pexit;

    if ( !who_fighting(ch) )
    {
	if ( ch->position == POS_FIGHTING
	||   ch->position == POS_EVASIVE
        ||   ch->position == POS_DEFENSIVE
        ||   ch->position == POS_AGGRESSIVE
        ||   ch->position == POS_BERSERK )
	{
	  if ( ch->mount )
	    ch->position = POS_MOUNTED;
	  else
	    ch->position = POS_STANDING;
	}
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }
    if ( IS_AFFECTED( ch, AFF_BERSERK ) ) {
        send_to_char( "Flee while berserking?  You aren't thinking very clearly...\n\r", ch);
        return;
    }
    if ( IS_AFFECTED( ch, AFF_GRAPPLE ) )
    {
	send_to_char( "You're too wrapped up to flee!\n\r", ch );
	return;
    }
    if ( ch->move <= 0 ) {
	send_to_char( "You're too exhausted to flee from combat!\n\r", ch );
	return;
    }
    /* No fleeing while more aggressive than standard or hurt. - Haus */
    if ( !IS_NPC( ch ) && ch->position < POS_FIGHTING ) {
	send_to_char( "You can't flee in an aggressive stance...\n\r", ch );
	return; 
    }
    if ( IS_NPC( ch ) && ch->position <= POS_SLEEPING )
	return;
    was_in = ch->in_room;
    for ( attempt = 0; attempt < 8; attempt++ )
    {
	door = number_door( );
	if (( pexit = get_exit( was_in, door ) ) == NULL
	||    !pexit->to_room
	||   IS_SET( pexit->exit_info, EX_NOFLEE )
	|| ( IS_PKILL( ch )
	&&   xIS_SET( pexit->to_room->room_flags, ROOM_DEATH ) )
	|| ( IS_SET( pexit->exit_info, EX_CLOSED )
	&&  !IS_AFFECTED( ch, AFF_PASS_DOOR ) )
	|| ( IS_NPC( ch )
	&&   xIS_SET( pexit->to_room->room_flags, ROOM_NO_MOB ) ) )
	    continue;
        affect_strip ( ch, gsn_sneak );
        xREMOVE_BIT  ( ch->affected_by, AFF_SNEAK );
	if ( ch->mount && ch->mount->fighting )
	    stop_fighting( ch->mount, TRUE );
	move_char( ch, pexit, 0 );
	if ( ( now_in = ch->in_room ) == was_in )
	    continue;
	ch->in_room = was_in;
	act( AT_FLEE, "$n flees head over heels!", ch, NULL, NULL, TO_ROOM );
	ch->in_room = now_in;
	act( AT_FLEE, "$n glances around for signs of pursuit.", ch, NULL, NULL, TO_ROOM );
	if ( !IS_NPC( ch ) )
	{
	    CHAR_DATA *wf = who_fighting( ch );
	    act( AT_FLEE, "You flee head over heels from combat!", ch, NULL, NULL, TO_CHAR );
	    los = ( exp_level( ch, ch->level+1 ) - exp_level( ch, ch->level ) ) * 0.02;
	    if ( ch->level < 50 )
	    {
	      if ( !IS_PKILL( ch ) )
	      {
		if ( ch->level > 1 )
		{
		  sprintf( buf, "Curse the gods, you've lost %d experience!", los );
		  act( AT_FLEE, buf, ch, NULL, NULL, TO_CHAR );
		  gain_exp( ch, 0 - los );
		}
	      }
	    }
	    if ( wf && ch->pcdata->deity )
	    {
	      int level_ratio = URANGE( 1, wf->level / ch->level, 50 );

	      if ( wf && wf->race == ch->pcdata->deity->npcrace )
		adjust_favor( ch, 1, level_ratio );
   	      else
		if ( wf && wf->race == ch->pcdata->deity->npcfoe )
		  adjust_favor( ch, 16, level_ratio );
		else
		  adjust_favor( ch, 0, level_ratio );
	    }
	}
	stop_fighting( ch, TRUE );
	return;
    }
    los = ( exp_level( ch, ch->level+1 ) - exp_level( ch, ch->level ) ) * 0.01;
    act( AT_FLEE, "You attempt to flee from combat but can't escape!", ch, NULL, NULL, TO_CHAR );
    if ( ch->level < 50 && number_bits( 3 ) == 1 )
    {
      if ( !IS_PKILL( ch ) )
      {
        sprintf( buf, "Curse the gods, you've lost %d experience!\n\r", los );
        act( AT_FLEE, buf, ch, NULL, NULL, TO_CHAR );
        gain_exp( ch, 0 - los );
      }
    }
    return;
}


void do_sla( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
    return;
}

void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );
    one_argument( argument, arg2 );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Slay whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "immolate" ) )
    {
      act( AT_FIRE, "Your fireball turns $N into a blazing inferno.",  ch, NULL, victim, TO_CHAR    );
      act( AT_FIRE, "$n releases a searing fireball in your direction.", ch, NULL, victim, TO_VICT    );
      act( AT_FIRE, "$n points at $N, who bursts into a flaming inferno.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "shatter" ) )
    {
      act( AT_LBLUE, "You freeze $N with a glance and shatter the frozen corpse into tiny shards.",  ch, NULL, victim, TO_CHAR    );
      act( AT_LBLUE, "$n freezes you with a glance and shatters your frozen body into tiny shards.", ch, NULL, victim, TO_VICT    );
      act( AT_LBLUE, "$n freezes $N with a glance and shatters the frozen body into tiny shards.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "demon" ) )
    {
      act( AT_IMMORT, "You gesture, and a slavering demon appears.  With a horrible grin, the",  ch, NULL, victim, TO_CHAR );
      act( AT_IMMORT, "foul creature turns on $N, who screams in panic before being eaten alive.",  ch, NULL, victim, TO_CHAR );
      act( AT_IMMORT, "$n gestures, and a slavering demon appears.  The foul creature turns on",  ch, NULL, victim, TO_VICT );
      act( AT_IMMORT, "you with a horrible grin.   You scream in panic before being eaten alive.",  ch, NULL, victim, TO_VICT );
      act( AT_IMMORT, "$n gestures, and a slavering demon appears.  With a horrible grin, the",  ch, NULL, victim, TO_NOTVICT );
      act( AT_IMMORT, "foul creature turns on $N, who screams in panic before being eaten alive.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "pounce" ) )
    {
      act( AT_BLOOD, "Leaping upon $N with bared fangs, you tear open $S throat and toss the corpse to the ground...",  ch, NULL, victim, TO_CHAR );
      act( AT_BLOOD, "In a heartbeat, $n rips $s fangs through your throat!  Your blood sprays and pours to the ground as your life ends...", ch, NULL, victim, TO_VICT );
      act( AT_BLOOD, "Leaping suddenly, $n sinks $s fangs into $N's throat.  As blood sprays and gushes to the ground, $n tosses $N's dying body away.",  ch, NULL, victim, TO_NOTVICT );
    }
 
    else if ( !str_cmp( arg2, "slit" ) )
    {
      act( AT_BLOOD, "You calmly slit $N's throat.", ch, NULL, victim, TO_CHAR );
      act( AT_BLOOD, "$n reaches out with a clawed finger and calmly slits your throat.", ch, NULL, victim, TO_VICT );
      act( AT_BLOOD, "$n calmly slits $N's throat.", ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "dog" ) )
    {
      act( AT_BLOOD, "You order your dogs to rip $N to shreds.", ch, NULL, victim, TO_CHAR );
      act( AT_BLOOD, "$n orders $s dogs to rip you apart.", ch, NULL, victim, TO_VICT );
      act( AT_BLOOD, "$n orders $s dogs to rip $N to shreds.", ch, NULL, victim, TO_NOTVICT );
    }

    else
    {
      act( AT_IMMORT, "You slay $N in cold blood!",  ch, NULL, victim, TO_CHAR    );
      act( AT_IMMORT, "$n slays you in cold blood!", ch, NULL, victim, TO_VICT    );
      act( AT_IMMORT, "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );
    }

    set_cur_char(victim);
    raw_kill( ch, victim );
    return;
}
