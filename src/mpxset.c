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
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


int	get_otype	args( ( char *type ) );
int	get_aflag	args( ( char *flag ) );
int	get_npc_race	args( ( char *type ) );
int	get_actflag	args( ( char *flag ) );
int	get_risflag	args( ( char *flag ) );
int	get_partflag	args( ( char *flag ) );
int	get_attackflag	args( ( char *flag ) );
int	get_defenseflag	args( ( char *flag ) );
int	get_langflag	args( ( char *flag ) );
int	get_langnum	args( ( char *flag ) );
int	get_trigflag	args( ( char *flag ) );

extern	int	top_affect;


void do_mpmset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf  [MAX_STRING_LENGTH];
    char outbuf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int value, v2;
    int minattr, maxattr;

    /* A desc means switched.. too many loopholes if we allow that.. */
    if ( !IS_NPC( ch ) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc )
    {
	send_to_char( "Huh?\n\r", ch );
	return;    
    }

    smash_tilde( argument );

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( !*arg1 )
    {
	progbug("MpMset: no args", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
	progbug("MpMset: no victim", ch);
	return;
    }

    if ( IS_IMMORTAL(victim) )
    {
	send_to_char( "You can't do that!\n\r", ch );
	return;
    }
    
    if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE) )
    {
	progbug("MpMset: victim is proto", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	minattr = 1;
	maxattr = 25;
    }
    else
    {
	minattr = 3;
	maxattr = 18;
    }

    value = is_number( arg3 ) ? atoi( arg3 ) : -1;
    if ( atoi(arg3) < -1 && value == -1 )
      value = atoi(arg3);

    if ( !str_cmp( arg2, "str" ) )
    {
	if ( value < minattr || value > maxattr )
	{
	    progbug("MpMset: Invalid str", ch);
	    return;
	}
	victim->perm_str = value;
	return;
    }

    if ( !str_cmp( arg2, "int" ) )
    {
	if ( value < minattr || value > maxattr )
	{
	    progbug("MpMset: Invalid int", ch);
	    return;
	}
	victim->perm_int = value;
	return;
    }

    if ( !str_cmp( arg2, "wis" ) )
    {
	if ( value < minattr || value > maxattr )
	{
	    progbug("MpMset: Invalid wis", ch);
	    return;
	}
	victim->perm_wis = value;
	return;
    }

    if ( !str_cmp( arg2, "dex" ) )
    {
	if ( value < minattr || value > maxattr )
	{
	    progbug("MpMset: Invalid dex", ch);
	    return;
	}
	victim->perm_dex = value;
	return;
    }

    if ( !str_cmp( arg2, "con" ) )
    {
	if ( value < minattr || value > maxattr )
	{
	    progbug("MpMset: Invalid con", ch);
	    return;
	}
	victim->perm_con = value;
	return;
    }

    if ( !str_cmp( arg2, "cha" ) )
    {
	if ( value < minattr || value > maxattr )
	{
	    progbug("MpMset: Invalid cha", ch);
	    return;
	}
	victim->perm_cha = value;
	return;
    }

    if ( !str_cmp( arg2, "lck" ) )
    {
	if ( value < minattr || value > maxattr )
	{
	    progbug("MpMset: Invalid lck", ch);
	    return;
	}
	victim->perm_lck = value;
	return;
    }

    if ( !str_cmp( arg2, "sav1" ) )
    {
	if ( value < -30 || value > 30 )
	{
	    progbug("MpMset: Invalid sav1", ch);
	    return;
	}
	victim->saving_poison_death = value;
	return;
    }

    if ( !str_cmp( arg2, "sav2" ) )
    {
	if ( value < -30 || value > 30 )
	{
	    progbug("MpMset: Invalid sav2", ch);
	    return;
	}
	victim->saving_wand = value;
	return;
    }

    if ( !str_cmp( arg2, "sav3" ) )
    {
	if ( value < -30 || value > 30 )
	{
	    progbug("MpMset: Invalid sav3", ch);
	    return;
	}
	victim->saving_para_petri = value;
	return;
    }

    if ( !str_cmp( arg2, "sav4" ) )
    {
	if ( value < -30 || value > 30 )
	{
	    progbug("MpMset: Invalid sav4", ch);
	    return;
	}
	victim->saving_breath = value;
	return;
    }

    if ( !str_cmp( arg2, "sav5" ) )
    {
	if ( value < -30 || value > 30 )
	{
	    progbug("MpMset: Invalid sav5", ch);
	    return;
	}
	victim->saving_spell_staff = value;
	return;
    }

    if ( !str_cmp( arg2, "sex" ) )
    {
	if ( value < 0 || value > 2 )
	{
	    progbug("MpMset: Invalid sex", ch);
	    return;
	}
	victim->sex = value;
	return;
    }

    if ( !str_cmp( arg2, "class" ) )
    {
        if( IS_NPC(victim) )   /* Broken by Haus... fixed by Thoric */
	{
          if ( value >= MAX_NPC_CLASS || value < 0 )
	  {
		progbug("MpMset: Invalid npc class", ch);
	        return;
	  }
	  victim->class = value;
	  return;
	}
	progbug("MpMset: can't set pc class", ch);
    }

    if ( !str_cmp( arg2, "race" ) )
    {
	value = get_npc_race( arg3 );
	if ( value < 0 )
	  value = atoi( arg3 );
	if ( !IS_NPC(victim) )
	{
	  progbug("MpMset: can't set pc race", ch);
	  return;
	}
	if ( value < 0 || value >= MAX_NPC_RACE )
	{
	    progbug("MpMset: Invalid npc race", ch);
	    return;
	}
	victim->race = value;
	return;
    }

    if ( !str_cmp( arg2, "armor" ) )
    {
	if ( value < -300 || value > 300 )
	{
	    send_to_char( "AC range is -300 to 300.\n\r", ch );
	    return;
	}
	victim->armor = value;
	return;
    }

    if ( !str_cmp( arg2, "level" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    progbug("MpMset: can't set pc level", ch);
	    return;
	}

	if ( value < 0 || value > LEVEL_AVATAR + 5 )
	{
	    progbug("MpMset: Invalid npc level", ch);
	    return;
	}
	victim->level = value;
	return;
    }

    if ( !str_cmp( arg2, "numattacks" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    progbug("MpMset: can't set pc numattacks", ch);
	    return;
	}

	if ( value < 0 || value > 20 )
	{
	    progbug("MpMset: Invalid npc numattacks", ch);
	    return;
	}
	victim->numattacks = value;
	return;
    }

    if ( !str_cmp( arg2, "gold" ) )
    {
	victim->gold = value;
	return;
    }

    if ( !str_cmp( arg2, "hitroll" ) )
    {
	victim->hitroll = URANGE(0, value, 85);
	return;
    }

    if ( !str_cmp( arg2, "damroll" ) )
    {
	victim->damroll = URANGE(0, value, 65);
	return;
    }

    if ( !str_cmp( arg2, "hp" ) )
    {
	if ( value < 1 || value > 32700 )
	{
	    progbug("MpMset: Invalid hp", ch);
	    return;
	}
	victim->max_hit = value;
	return;
    }

    if ( !str_cmp( arg2, "mana" ) )
    {
	if ( value < 0 || value > 30000 )
	{
	    progbug("MpMset: Invalid mana", ch);
	    return;
	}
	victim->max_mana = value;
	return;
    }

    if ( !str_cmp( arg2, "move" ) )
    {
	if ( value < 0 || value > 30000 )
	{
	    progbug("MpMset: Invalid move", ch);
	    return;
	}
	victim->max_move = value;
	return;
    }

    if ( !str_cmp( arg2, "practice" ) )
    {
	if ( value < 0 || value > 100 )
	{
	    progbug("MpMset: Invalid practice", ch);
	    return;
	}
	victim->practice = value;
	return;
    }

    if ( !str_cmp( arg2, "align" ) )
    {
	if ( value < -1000 || value > 1000 )
	{
	    progbug("MpMset: Invalid align", ch);
	    return;
	}
	victim->alignment = value;
	return;
    }

/* non-functional for now -- Blod
    if ( !str_cmp( arg2, "quest" ) )
    {
	if ( IS_NPC(victim) )
	{
	    progbug("MpMset: can't set npc quest", ch);
	    return;
	}

	if ( value < 0 || value > 500 )
	{
	    progbug("MpMset: Invalid pc quest", ch);
	    return;
	}

	victim->pcdata->quest = value;
	return;
    }
*/

    if ( !str_cmp( arg2, "questplus" ) )
    {
	if ( IS_NPC(victim) )
	{
	    progbug("MpMset: can't set npc qp", ch);
	    return;
	}

	if ( value < 0 || value > 5000 )
	{
	    progbug("MpMset: Invalid pc qp", ch);
	    return;
	}
	sprintf( log_buf, "%s raising glory of %s by %d ...",
	  ch->name, victim->name, value );
        log_string( log_buf );
	victim->pcdata->quest_curr  += value;
	victim->pcdata->quest_accum += value;
	return;
    }

    if ( !str_cmp( arg2, "favor" ) )    
    {
	if ( IS_NPC( victim ) )
	{
	   progbug("MpMset: can't set npc favor", ch);
	   return;
	}

	if ( value < -2500 || value > 2500 )
	{
	   progbug("MpMset: Invalid pc favor", ch);
	   return;
	}

	victim->pcdata->favor = value;
	return;
    }	

    if ( !str_cmp( arg2, "mentalstate" ) )
    {
	if ( value < -100 || value > 100 )
	{
	    progbug("MpMset: Invalid mentalstate", ch);
	    return;
	}
	victim->mental_state = value;
	return;
    }

    if ( !str_cmp( arg2, "emotion" ) )
    {
	if ( value < -100 || value > 100 )
	{
	    progbug("MpMset: Invalid emotion", ch);
	    return;
	}
	victim->emotional_state = value;
	return;
    }

    if ( !str_cmp( arg2, "thirst" ) )
    {
	if ( IS_NPC(victim) )
	{
	    progbug("MpMset: can't set npc thirst", ch);
	    return;
	}

	if ( value < 0 || value > 100 )
	{
	    progbug("MpMset: Invalid pc thirst", ch);
	    return;
	}

	victim->pcdata->condition[COND_THIRST] = value;
	return;
    }

    if ( !str_cmp( arg2, "drunk" ) )
    {
	if ( IS_NPC(victim) )
	{
	    progbug("MpMset: can't set npc drunk", ch);
	    return;
	}

	if ( value < 0 || value > 100 )
	{
	    progbug("MpMset: Invalid pc drunk", ch);
	    return;
	}

	victim->pcdata->condition[COND_DRUNK] = value;
	return;
    }

    if ( !str_cmp( arg2, "full" ) )
    {
	if ( IS_NPC(victim) )
	{
	    progbug("MpMset: can't set npc full", ch);
	    return;
	}

	if ( value < 0 || value > 100 )
	{
	    progbug("MpMset: Invalid pc full", ch);
	    return;
	}

	victim->pcdata->condition[COND_FULL] = value;
	return;
    }

    if ( !str_cmp( arg2, "blood" ) )
    {
	if ( IS_NPC(victim) )
	{
	    progbug("MpMset: can't set npc blood", ch);
	    return;
	}

	if ( value < 0 || value > MAX_LEVEL+10 )
	{
	    progbug("MpMset: Invalid pc blood", ch);
	    return;
	}

	victim->pcdata->condition[COND_BLOODTHIRST] = value;
	return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    progbug("MpMset: can't set pc name", ch);
	    return;
	}

	STRFREE( victim->name );
	victim->name = STRALLOC( arg3 );
	return;
    }

    if ( !str_cmp( arg2, "deity" ))
    {
	DEITY_DATA *deity;
	
        if ( IS_NPC(victim) )
        {
            progbug("MpMset: can't set npc deity", ch);
            return;
        }
 
        if ( !arg3 || arg3[0] == '\0' )
        {
            STRFREE( victim->pcdata->deity_name );
            victim->pcdata->deity_name        = STRALLOC( "" );
            victim->pcdata->deity             = NULL;
            return;
        }
 
        deity = get_deity( arg3 );
        if ( !deity )
        {
           progbug("MpMset: Invalid deity", ch);
           return;
        }
        STRFREE( victim->pcdata->deity_name );
        victim->pcdata->deity_name = QUICKLINK( deity->name );
        victim->pcdata->deity = deity;
        return;
    }

    if ( !str_cmp( arg2, "short" ) )
    {
	STRFREE( victim->short_descr );
	victim->short_descr = STRALLOC( arg3 );
	return;
    }

    if ( !str_cmp( arg2, "long" ) )
    {
	STRFREE( victim->long_descr );
	strcpy( buf, arg3 );
	strcat( buf, "\n\r" );
	victim->long_descr = STRALLOC( buf );
	return;
    }

    if ( !str_cmp( arg2, "title" ) )
    {
	if ( IS_NPC(victim) )
	{
	    progbug("MpMset: can't set npc title", ch);
	    return;
	}

	set_title( victim, arg3 );
	return;
    }

    if ( !str_cmp( arg2, "spec" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    progbug("MpMset: can't set pc spec", ch);
	    return;
	}

        if ( !str_cmp( arg3, "none" ) )
        {
          victim->spec_fun = NULL;
	  return;
        }

	if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
	{
	    progbug("MpMset: Invalid spec", ch);
	    return;
	}
	return;
    }

    if ( !str_cmp( arg2, "flags" ) )
    {
	if ( !IS_NPC( victim ) )
	{
	    progbug("MpMset: can't set pc flags", ch);
	    return;
	}

	if ( !argument || argument[0] == '\0' )
	{
	   progbug("MpMset: no flags", ch);
	   return;
	}
	while ( argument[0] != '\0' )
	{
	   argument = one_argument( argument, arg3 );
	   value = get_actflag( arg3 );
	   if ( value < 0 || value > MAX_BITS )
	     progbug("MpMset: Invalid flag", ch);
	   else
	   {
	     if ( value == ACT_PROTOTYPE )
		progbug("MpMset: can't set prototype flag", ch);
	     else if ( value == ACT_IS_NPC )
		progbug("MpMset: can't remove npc flag", ch);
/*
	     else if ( value == ACT_POLYMORPHED )
		progbug("MpMset: can't change polymorphed flag", ch);
*/
	     else
		xTOGGLE_BIT( victim->act, value );
	   }
	}
	return;
    }

    if ( !str_cmp( arg2, "affected" ) )
    {
	if ( !IS_NPC( victim ) )
	{
	    progbug("MpMset: can't modify pc affected", ch);
	    return;
	}

	if ( !argument || argument[0] == '\0' )
	{
	   progbug("MpMset: no affected", ch);
	   return;
	}
	while ( argument[0] != '\0' )
	{
	   argument = one_argument( argument, arg3 );
	   value = get_aflag( arg3 );
	   if ( value < 0 || value > MAX_BITS )
		progbug("MpMset: Invalid affected", ch);
	   else
		xTOGGLE_BIT( victim->affected_by, value );
	}
	return;
    }

    /*
     * save some more finger-leather for setting RIS stuff
     * Why there's can_modify checks here AND in the called function, Ill
     * never know, so I removed them.. -- Alty
     */
    if ( !str_cmp( arg2, "r" ) )
    {
	sprintf(outbuf,"%s resistant %s", arg1, arg3);
        do_mpmset( ch, outbuf );
        return;
    }
    if ( !str_cmp( arg2, "i" ) )
    {
	sprintf(outbuf, "%s immune %s", arg1, arg3);
        do_mpmset( ch, outbuf );
        return;
    }
    if ( !str_cmp( arg2, "s" ) )
    {
	sprintf(outbuf, "%s susceptible %s", arg1, arg3);
        do_mpmset( ch, outbuf );
        return;
    }
    if ( !str_cmp( arg2, "ri" ) )
    {
	sprintf(outbuf, "%s resistant %s", arg1, arg3);
        do_mpmset( ch, outbuf );
	sprintf(outbuf, "%s immune %s", arg1, arg3);
        do_mpmset( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "rs" ) )
    {
	sprintf(outbuf, "%s resistant %s", arg1, arg3);
        do_mpmset( ch, outbuf );
	sprintf(outbuf, "%s susceptible %s", arg1, arg3);
        do_mpmset( ch, outbuf );
        return;
    }
    if ( !str_cmp( arg2, "is" ) )
    {
	sprintf(outbuf, "%s immune %s", arg1, arg3);
        do_mpmset( ch, outbuf );
	sprintf(outbuf, "%s susceptible %s", arg1, arg3);
        do_mpmset( ch, outbuf );
        return;
    }
    if ( !str_cmp( arg2, "ris" ) )
    {
	sprintf(outbuf, "%s resistant %s", arg1, arg3);
        do_mpmset( ch, outbuf );
	sprintf(outbuf, "%s immune %s", arg1, arg3);
        do_mpmset( ch, outbuf );
	sprintf(outbuf,"%s susceptible %s",arg1, arg3);
        do_mpmset( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "resistant" ) )
    {
	if ( !IS_NPC( victim ) )
	{
	    progbug("MpMset: can't set pc resistant", ch);
	    return;
	}
	if ( !argument || argument[0] == '\0' )
	{
	   progbug("MpMset: no resistant", ch);
	   return;
	}
	while ( argument[0] != '\0' )
	{
	   argument = one_argument( argument, arg3 );
	   value = get_risflag( arg3 );
	   if ( value < 0 || value > 31 )
		progbug("MpMset: Invalid resistant", ch);
	   else
		TOGGLE_BIT( victim->resistant, 1 << value );
	}
	return;
    }

    if ( !str_cmp( arg2, "immune" ) )
    {
	if ( !IS_NPC( victim ) )
	{
	    progbug("MpMset: can't set pc immune", ch);
	    return;
	}
	if ( !argument || argument[0] == '\0' )
	{
	    progbug("MpMset: no immune", ch);
	    return;
	}
	while ( argument[0] != '\0' )
	{
	    argument = one_argument( argument, arg3 );
	    value = get_risflag( arg3 );
	    if ( value < 0 || value > 31 )
		progbug("MpMset: Invalid immune", ch);
	    else
		TOGGLE_BIT( victim->immune, 1 << value );
	}
	return;
    }

    if ( !str_cmp( arg2, "susceptible" ) )
    {
	if ( !IS_NPC( victim ) )
	{
	    progbug("MpMset: can't set pc susceptible", ch);
	    return;
	}
	if ( !argument || argument[0] == '\0' )
	{
	    progbug("MpMset: no susceptible", ch);
	    return;
	}
	while ( argument[0] != '\0' )
	{
	    argument = one_argument( argument, arg3 );
	    value = get_risflag( arg3 );
	    if ( value < 0 || value > 31 )
		progbug("MpMset: Invalid susceptible", ch);
	    else
		TOGGLE_BIT( victim->susceptible, 1 << value );
	}
	return;
    }

    if ( !str_cmp( arg2, "part" ) )
    {
	if ( !IS_NPC( victim ) )
	{
	    progbug("MpMset: can't set pc part", ch);
	    return;
	}
	if ( !argument || argument[0] == '\0' )
	{
	    progbug("MpMset: no part", ch);
	    return;
	}
	while ( argument[0] != '\0' )
	{
	    argument = one_argument( argument, arg3 );
	    value = get_partflag( arg3 );
	    if ( value < 0 || value > 31 )
		progbug("MpMset: Invalid part", ch);
	    else
		TOGGLE_BIT( victim->xflags, 1 << value );
	}
	return;
    }

    if ( !str_cmp( arg2, "attack" ) )
    {
	if ( !IS_NPC( victim ) )
	{
	    progbug("MpMset: can't set pc attack", ch);
	    return;
	}
	if ( !argument || argument[0] == '\0' )
	{
	    progbug("MpMset: no attack", ch);
	    return;
	}
	while ( argument[0] != '\0' )
	{
	    argument = one_argument( argument, arg3 );
	    value = get_attackflag( arg3 );
	    if ( value < 0 )
		progbug("MpMset: Invalid attack", ch);
	    else
		xTOGGLE_BIT( victim->attacks, value );
	}
	return;
    }

    if ( !str_cmp( arg2, "defense" ) )
    {
	if ( !IS_NPC( victim ) )
	{
	    progbug("MpMset: can't set pc defense", ch);
	    return;
	}
	if ( !argument || argument[0] == '\0' )
	{
	    progbug("MpMset: no defense", ch);
	    return;
	}
	while ( argument[0] != '\0' )
	{
	    argument = one_argument( argument, arg3 );
	    value = get_defenseflag( arg3 );
	    if ( value < 0 || value > MAX_BITS )
		progbug("MpMset: Invalid defense", ch);
	    else
		xTOGGLE_BIT( victim->defenses, value );
	}
	return;
    }

    if ( !str_cmp( arg2, "pos" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    progbug("MpMset: can't set pc pos", ch);
	    return;
	}
	if ( value < 0 || value > POS_STANDING )
	{
	    progbug("MpMset: Invalid pos", ch);
	    return;
	}
	victim->position = value;
	return;
    }

    if ( !str_cmp( arg2, "defpos" ) )
    {
	if ( !IS_NPC(victim) )
	{
	    progbug("MpMset: can't set pc defpos", ch);
	    return;
	}
	if ( value < 0 || value > POS_STANDING )
	{
	    progbug("MpMset: Invalid defpos", ch);
	    return;
	}
	victim->defposition = value;
	return;
    }

    if ( !str_cmp( arg2, "speaks" ) )
    {
    	if ( !argument || argument[0] == '\0' )
    	{
    	    progbug("MpMset: no speaks", ch);
    	    return;
    	}
    	while ( argument[0] != '\0' )
    	{
	    argument = one_argument( argument, arg3 );
	    value = get_langflag( arg3 );
            v2 = get_langnum( arg3);
	    if ( value == LANG_UNKNOWN )
		progbug("MpMset: Invalid speaks", ch);
	    else
	    if ( !IS_NPC( victim ) )
	    {
    		int valid_langs = LANG_COMMON | LANG_ELVEN | LANG_DWARVEN | LANG_PIXIE
    				| LANG_OGRE | LANG_ORCISH | LANG_TROLLISH | LANG_GOBLIN
    				| LANG_HALFLING;

	    	if ( !(value &= valid_langs) )
		{
		    progbug("MpMset: Invalid player language", ch);
		    continue;
		}    		
                if ( v2 == -1 )
                    ch_printf( ch, "Unknown language: %s\n\r", arg3 );
                else
                     TOGGLE_BIT( victim->speaks, 1 << v2 );
	    }
	    else {
                if ( v2 == -1 )
                    ch_printf( ch, "Unknown language: %s\n\r", arg3 );
                else
                     TOGGLE_BIT( victim->speaks, 1 << v2 );
            }  
	}
	if ( !IS_NPC( victim ) )
	{
	    REMOVE_BIT( victim->speaks, race_table[victim->race]->language );
	    if ( !knows_language( victim, victim->speaking, victim ) )
		victim->speaking = race_table[victim->race]->language;
	}
	return;
    }

    if ( !str_cmp( arg2, "speaking" ) )
    {
	if ( !IS_NPC( victim ) )
	{
	    progbug("MpMset: can't set pc speaking", ch);
	    return;
	}
	if ( !argument || argument[0] == '\0' )
	{
	    progbug("MpMset: no speaking", ch);
	    return;
	}
	while ( argument[0] != '\0' )
	{
	    argument = one_argument( argument, arg3 );
	    value = get_langflag( arg3 );
	    if ( value == LANG_UNKNOWN )
		progbug("MpMset: Invalid speaking", ch);
	    else {
                v2 = get_langnum( arg3);
                if ( v2 == -1 )
                    ch_printf( ch, "Unknown language: %s\n\r", arg3 );
                else
                     TOGGLE_BIT( victim->speaks, 1 << v2 );
            }
	}
	return;
    }

    progbug("MpMset: Invalid field", ch);
    return;
}

void do_mposet( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf  [MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    char outbuf  [MAX_STRING_LENGTH];
    int value, tmp;

    /* A desc means switched.. too many loopholes if we allow that.. */
    if ( !IS_NPC( ch ) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc )
    {
	send_to_char( "Huh?\n\r", ch );
	return;    
    }

    smash_tilde( argument );

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( !*arg1 )
    {
	progbug("MpOset: no args", ch);
	return;
    }

    if ( ( obj = get_obj_here( ch, arg1 ) ) == NULL )
    {
	progbug("MpOset: no object", ch);
	return;
    }

    if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
    {
	progbug("MpOset: can't set prototype items", ch);
	return;
    }
    separate_obj( obj );
    value = atoi( arg3 );

    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
	obj->value[0] = value;
	return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
	obj->value[1] = value;
	return;
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
	obj->value[2] = value;
	return;
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
	obj->value[3] = value;
	return;
    }

    if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
    {
	obj->value[4] = value;
	return;
    }

    if ( !str_cmp( arg2, "value5" ) || !str_cmp( arg2, "v5" ) )
    {
	obj->value[5] = value;
	return;
    }

    if ( !str_cmp( arg2, "type" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	   progbug("MpOset: no type", ch);
	   return;
	}
	value = get_otype( argument );
	if ( value < 1 )
	{
	     progbug("MpOset: Invalid type", ch);
	     return;	
	}
	obj->item_type = (sh_int) value;
	return;	
    }

    if ( !str_cmp( arg2, "flags" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	   progbug("MpOset: no flags", ch);
	   return;
	}
	while ( argument[0] != '\0' )
	{
	   argument = one_argument( argument, arg3 );
	   value = get_oflag( arg3 );
	   if ( value < 0 || value > MAX_BITS )
	     progbug("MpOset: Invalid flag", ch);
	   else
	   {
	     if ( value == ITEM_PROTOTYPE )
		progbug("MpOset: can't set prototype flag", ch);
	     else
		xTOGGLE_BIT(obj->extra_flags, value);
	   }
	}
	return;
    }

    if ( !str_cmp( arg2, "wear" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	   progbug("MpOset: no wear", ch);
	   return;
	}
	while ( argument[0] != '\0' )
	{
	   argument = one_argument( argument, arg3 );
	   value = get_wflag( arg3 );
	   if ( value < 0 || value > 31 )
	     progbug("MpOset: Invalid wear", ch);
	   else
	     TOGGLE_BIT( obj->wear_flags, 1 << value );
	}

	return;
    }

    if ( !str_cmp( arg2, "level" ) )
    {
	obj->level = value;
	return;
    }

    if ( !str_cmp( arg2, "weight" ) )
    {
	obj->weight = value;
	return;
    }

    if ( !str_cmp( arg2, "cost" ) )
    {
	obj->cost = value;
	return;
    }

    if ( !str_cmp( arg2, "timer" ) )
    {
	obj->timer = value;
	return;
    }
	
    if ( !str_cmp( arg2, "name" ) )
    {
	STRFREE( obj->name );
	obj->name = STRALLOC( arg3 );
	return;
    }

    if ( !str_cmp( arg2, "short" ) )
    {
	STRFREE( obj->short_descr );
	obj->short_descr = STRALLOC( arg3 );
        /* Feature added by Narn, Apr/96 
	 * If the item is not proto, add the word 'rename' to the keywords
	 * if it is not already there.
	 */
        if ( str_infix( "mprename", obj->name ) )
        {
          sprintf( buf, "%s %s", obj->name, "mprename" );
	  STRFREE( obj->name );
	  obj->name = STRALLOC( buf );
        }
	return;
    }

    if ( !str_cmp( arg2, "long" ) )
    {
        STRFREE( obj->description );
        strcpy( buf, arg3 );
        obj->description = STRALLOC( buf );
        return;
    }

    if ( !str_cmp( arg2, "actiondesc" ) )
    {
	if ( strstr( arg3, "%n" )
	||   strstr( arg3, "%d" )
	||   strstr( arg3, "%l" ) )
	{
	   progbug("MpOset: Illegal actiondesc", ch);
	   return;
	}
	STRFREE( obj->action_desc );
	obj->action_desc = STRALLOC( arg3 );
	return;
    }

    if ( !str_cmp( arg2, "affect" ) )
    {
	AFFECT_DATA *paf;
	sh_int loc;
	int bitv;

	argument = one_argument( argument, arg2 );
	if ( arg2[0] == '\0' || !argument || argument[0] == 0 )
	{
	   progbug("MpOset: Bad affect syntax", ch);
	   send_to_char( "Usage: oset <object> affect <field> <value>\n\r", ch );
	   return;
	}
	loc = get_atype( arg2 );
	if ( loc < 1 )
	{
	     progbug("MpOset: Invalid affect field", ch);
	     return;	
	}
	if ( loc >= APPLY_AFFECT && loc < APPLY_WEAPONSPELL )
	{
	   bitv = 0;
	   while ( argument[0] != '\0' )
	   {
		argument = one_argument( argument, arg3 );
		if ( loc == APPLY_AFFECT )
		  value = get_aflag( arg3 );
		else
		  value = get_risflag( arg3 );
		if ( value < 0 || value > 31 )
		  progbug("MpOset: bad affect flag", ch);
		else
		  SET_BIT( bitv, 1 << value );
	   }
	   if ( !bitv )
	     return;
	   value = bitv;
	}
	else
	{
	   argument = one_argument( argument, arg3 );
	   value = atoi( arg3 );
	}
	CREATE( paf, AFFECT_DATA, 1 );
	paf->type		= -1;
	paf->duration		= -1;
	paf->location		= loc;
	paf->modifier		= value;
	xCLEAR_BITS(paf->bitvector);
	paf->next		= NULL;
	LINK( paf, obj->first_affect, obj->last_affect, next, prev );
	++top_affect;
	return;
    }

    if ( !str_cmp( arg2, "rmaffect" ) )
    {
	AFFECT_DATA *paf;
	sh_int loc, count;
	
	if ( !argument || argument[0] == '\0' )
	{
	   progbug("MpOset: no rmaffect", ch);
	   return;
	}
	loc = atoi( argument );
	if ( loc < 1 )
	{
	     progbug("MpOset: Invalid rmaffect", ch);
	     return;	
	}

	count = 0;
	
	for ( paf = obj->first_affect; paf; paf = paf->next )
	{
	  if ( ++count == loc )
	  {
	    UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
	    DISPOSE( paf );
	    send_to_char( "Removed.\n\r", ch );
	    --top_affect;
	    return;
	  }
	}
	progbug("MpOset: rmaffect not found", ch);
	return;
    }

    /*
     * save some finger-leather
     */
    if ( !str_cmp( arg2, "ris" ) )
    {
	sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
        do_mposet( ch, outbuf );
	sprintf(outbuf, "%s affect immune %s", arg1, arg3);
        do_mposet( ch, outbuf );
	sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
        do_mposet( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "r" ) )
    {
	sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
        do_mposet( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "i" ) )
    {
	sprintf(outbuf, "%s affect immune %s", arg1, arg3);
        do_mposet( ch, outbuf );
        return;
    }
    if ( !str_cmp( arg2, "s" ) )
    {
	sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
        do_mposet( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "ri" ) )
    {
	sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
        do_mposet( ch, outbuf );
	sprintf(outbuf, "%s affect immune %s", arg1, arg3);
        do_mposet( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "rs" ) )
    {
	sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
        do_mposet( ch, outbuf );
	sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
        do_mposet( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "is" ) )
    {
	sprintf(outbuf, "%s affect immune %s", arg1, arg3);
        do_mposet( ch, outbuf );
	sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
        do_mposet( ch, outbuf );
        return;
    }

    /*
     * Make it easier to set special object values by name than number
     * 						-Thoric
     */
    tmp = -1;
    switch( obj->item_type )
    {
	case ITEM_WEAPON:
	    if ( !str_cmp( arg2, "weapontype" ) )
	    {
		int x;

		value = -1;
		for ( x = 0; x < sizeof( attack_table ) / sizeof( attack_table[0] ); x++ )
		  if ( !str_cmp( arg3, attack_table[x] ) )
		    value = x;
		if ( value < 0 )
		{
		    progbug("MpOset: Invalid weapon type", ch);
		    return;
		}
		tmp = 3;
		break;
	    }
	    if ( !str_cmp( arg2, "condition" ) )	tmp = 0;
	    break;
	case ITEM_ARMOR:
	    if ( !str_cmp( arg2, "condition" ) )	tmp = 3;
	    if ( !str_cmp( arg2, "ac" )	)		tmp = 1;
	    break;
	case ITEM_SALVE:
	    if ( !str_cmp( arg2, "slevel"   ) )		tmp = 0;
	    if ( !str_cmp( arg2, "maxdoses" ) )		tmp = 1;
	    if ( !str_cmp( arg2, "doses"    ) )		tmp = 2;
	    if ( !str_cmp( arg2, "delay"    ) )		tmp = 3;
	    if ( !str_cmp( arg2, "spell1"   ) )		tmp = 4;
	    if ( !str_cmp( arg2, "spell2"   ) )		tmp = 5;
	    if ( tmp >=4 && tmp <= 5 )			value = skill_lookup(arg3);
	    break;
	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
	    if ( !str_cmp( arg2, "slevel" ) )		tmp = 0;
	    if ( !str_cmp( arg2, "spell1" ) )		tmp = 1;
	    if ( !str_cmp( arg2, "spell2" ) )		tmp = 2;
	    if ( !str_cmp( arg2, "spell3" ) )		tmp = 3;
	    if ( tmp >=1 && tmp <= 3 )			value = skill_lookup(arg3);
	    break;
	case ITEM_STAFF:
	case ITEM_WAND:
	    if ( !str_cmp( arg2, "slevel" ) )		tmp = 0;
	    if ( !str_cmp( arg2, "spell" ) )
	    {
	    	tmp = 3;
		value = skill_lookup(arg3);
	    }
	    if ( !str_cmp( arg2, "maxcharges" )	)	tmp = 1;
	    if ( !str_cmp( arg2, "charges" ) )		tmp = 2;
	    break;
	case ITEM_CONTAINER:
	    if ( !str_cmp( arg2, "capacity" ) )		tmp = 0;
	    if ( !str_cmp( arg2, "cflags" ) )		tmp = 1;
	    if ( !str_cmp( arg2, "key" ) )		tmp = 2;
	    break;
	case ITEM_SWITCH:
	case ITEM_LEVER:
	case ITEM_PULLCHAIN:
	case ITEM_BUTTON:
	    if ( !str_cmp( arg2, "tflags" ) )
	    {
	    	tmp = 0;
		value = get_trigflag(arg3);
	    }
	    break;
    }
    if ( tmp >= 0 && tmp <= 3 )
    {
	obj->value[tmp] = value;
	return;
    }

    progbug("MpOset: Invalid field", ch);
    return;
}
