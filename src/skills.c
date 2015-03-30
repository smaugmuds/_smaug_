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
 *			     Player skills module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"



extern  char *  const           sec_flags[];
extern	int			get_secflag( char *flag );
extern  void                    remove_bexit_flag( EXIT_DATA *exit, int flag );

char * const spell_flag[] =
{ "water", "earth", "air", "astral", "area", "distant", "reverse",
"noself", "_unused2_", "accumulative", "recastable", "noscribe",
"nobrew", "group", "object", "character", "secretskill", "pksensitive",
"stoponfail", "nofight", "nodispel", "randomtarget", "nomob", "r3", "r4",
"r5", "r6", "r7", "r8", "r9", "r10", "r11"
};

char * const spell_saves[] =
{ "none", "poison_death", "wands", "para_petri", "breath", "spell_staff" };

char * const spell_save_effect[] =
{ "none", "negate", "eightdam", "quarterdam", "halfdam", "3qtrdam",
"reflect", "absorb" };

char * const spell_damage[] =
#ifdef NEWSPELLS
{ "none", "fire", "cold", "electricity", "energy", "acid", "poison", "drain", "holy", "unholy" };
#else
{ "none", "fire", "cold", "electricity", "energy", "acid", "poison", "drain" };
#endif

char * const spell_action[] =
{ "none", "create", "destroy", "resist", "suscept", "divinate", "obscure",
"change" };

char * const spell_power[] =
{ "none", "minor", "greater", "major" };

char * const spell_class[] =
{ "none", "lunar", "solar", "travel", "summon", "life", "death", "illusion" };

char * const target_type[] =
{ "ignore", "offensive", "defensive", "self", "objinv" };


void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch );
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, 
	bool fShowN );

int ris_save( CHAR_DATA *ch, int chance, int ris );
bool check_illegal_psteal( CHAR_DATA *ch, CHAR_DATA *victim );

/* from magic.c */
void failed_casting( struct skill_type *skill, CHAR_DATA *ch,
		     CHAR_DATA *victim, OBJ_DATA *obj );


/*
 * Dummy function
 */
void skill_notfound( CHAR_DATA *ch, char *argument )
{
    send_to_char( "Huh?\n\r", ch );
    return;
}


int get_ssave( char *name )
{
    int x;

    for ( x = 0; x < sizeof(spell_saves) / sizeof(spell_saves[0]); x++ )
      if ( !str_cmp( name, spell_saves[x] ) )
        return x;
    return -1;
}

int get_starget( char *name )
{
    int x;

    for ( x = 0; x < sizeof(target_type) / sizeof(target_type[0]); x++ )
      if ( !str_cmp( name, target_type[x] ) )
        return x;
    return -1;
}

int get_sflag( char *name )
{
    int x;

    for ( x = 0; x < sizeof(spell_flag) / sizeof(spell_flag[0]); x++ )
      if ( !str_cmp( name, spell_flag[x] ) )
        return x;
    return -1;
}

int get_sdamage( char *name )
{
    int x;

    for ( x = 0; x < sizeof(spell_damage) / sizeof(spell_damage[0]); x++ )
      if ( !str_cmp( name, spell_damage[x] ) )
        return x;
    return -1;
}

int get_saction( char *name )
{
    int x;

    for ( x = 0; x < sizeof(spell_action) / sizeof(spell_action[0]); x++ )
      if ( !str_cmp( name, spell_action[x] ) )
        return x;
    return -1;
}

int get_ssave_effect( char *name )
{
    int x;

    for ( x = 0; x < sizeof(spell_save_effect) / sizeof(spell_save_effect[0]); x++ )
      if ( !str_cmp( name, spell_save_effect[x] ) )
        return x;
    return -1;
}

int get_spower( char *name )
{
    int x;

    for ( x = 0; x < sizeof(spell_power) / sizeof(spell_power[0]); x++ )
      if ( !str_cmp( name, spell_power[x] ) )
        return x;
    return -1;
}

int get_sclass( char *name )
{
    int x;

    for ( x = 0; x < sizeof(spell_class) / sizeof(spell_class[0]); x++ )
      if ( !str_cmp( name, spell_class[x] ) )
        return x;
    return -1;
}

bool is_legal_kill(CHAR_DATA *ch, CHAR_DATA *vch)
{
  if ( IS_NPC(ch) || IS_NPC(vch) )
    return TRUE;
  if ( !IS_PKILL(ch) || !IS_PKILL(vch) )
    return FALSE;
  if ( ch->pcdata->clan && ch->pcdata->clan == vch->pcdata->clan )
    return FALSE;
  return TRUE;
}


extern char *target_name;	/* from magic.c */

/*
 * Perform a binary search on a section of the skill table
 * Each different section of the skill table is sorted alphabetically
 * Only match skills player knows				-Thoric
 */
bool check_skill( CHAR_DATA *ch, char *command, char *argument )
{
    int sn;
    int first = gsn_first_skill;
    int top   = gsn_first_weapon-1;
    int mana, blood;
    struct timeval time_used;

    /* bsearch for the skill */
    for (;;)
    {
	sn = (first + top) >> 1;

	if ( LOWER(command[0]) == LOWER(skill_table[sn]->name[0])
	&&  !str_prefix(command, skill_table[sn]->name)
	&&  (skill_table[sn]->skill_fun || skill_table[sn]->spell_fun != spell_null)
        && ( can_use_skill(ch, 0, sn ) ) )
/*
	&&  (IS_NPC(ch)
	||  (ch->pcdata->learned[sn] > 0
	&&   ch->level >= skill_table[sn]->skill_level[ch->class])) )
*/
		break;
	if (first >= top)
	    return FALSE;
    	if (strcmp( command, skill_table[sn]->name) < 1)
	    top = sn - 1;
    	else
	    first = sn + 1;
    }

    if ( !check_pos( ch, skill_table[sn]->minimum_position ) )
	return TRUE;

    if ( IS_NPC(ch)
    &&  (IS_AFFECTED( ch, AFF_CHARM ) || IS_AFFECTED( ch, AFF_POSSESS )) )
    {
	send_to_char( "For some reason, you seem unable to perform that...\n\r", ch );
	act( AT_GREY,"$n wanders around aimlessly.", ch, NULL, NULL, TO_ROOM );
	return TRUE;
    }

    /* check if mana is required */
    if ( skill_table[sn]->min_mana )
    {
	mana = IS_NPC(ch) ? 0 : UMAX(skill_table[sn]->min_mana,
	   100 / ( 2 + ch->level - skill_table[sn]->skill_level[ch->class] ) );
	blood = UMAX(1, (mana+4) / 8);      /* NPCs don't have PCDatas. -- Altrag */
	if ( IS_VAMPIRE(ch) )
	{
	    if (ch->pcdata->condition[COND_BLOODTHIRST] < blood)
	    {
		send_to_char( "You don't have enough blood power.\n\r", ch );
		return TRUE;
	    }
	}
	else
	if ( !IS_NPC(ch) && ch->mana < mana )
	{
	    send_to_char( "You don't have enough mana.\n\r", ch );
	    return TRUE;
	}
    }
    else
    {
	mana = 0;
	blood = 0;
    }
    
    /*
     * Is this a real do-fun, or a really a spell?
     */
    if ( !skill_table[sn]->skill_fun )
    {
	ch_ret retcode = rNONE;
	void *vo = NULL;
	CHAR_DATA *victim = NULL;
	OBJ_DATA *obj = NULL;

	target_name = "";

	switch ( skill_table[sn]->target )
	{
	default:
	    bug( "Check_skill: bad target for sn %d.", sn );
	    send_to_char( "Something went wrong...\n\r", ch );
	    return TRUE;

	case TAR_IGNORE:
	    vo = NULL;
	    if ( argument[0] == '\0' )
	    {
		if ( (victim=who_fighting(ch)) != NULL )
		    target_name = victim->name;
	    }
	    else
		target_name = argument;
	    break;

	case TAR_CHAR_OFFENSIVE:
	    {
	      if ( argument[0] == '\0'
	      &&  (victim=who_fighting(ch)) == NULL )
	      {
		ch_printf( ch, "Confusion overcomes you as your '%s' has no target.\n\r", skill_table[sn]->name );
		return TRUE;
	      }
	      else
	      if ( argument[0] != '\0'
	      &&  (victim=get_char_room(ch, argument)) == NULL )
	      {
		send_to_char( "They aren't here.\n\r", ch );
		return TRUE;
	      }
	    }
	    if ( is_safe( ch, victim, TRUE ) )
		return TRUE;

	    if ( ch == victim && SPELL_FLAG(skill_table[sn], SF_NOSELF))
            {
                send_to_char( "You can't target yourself!\n\r", ch);
                return TRUE;
            }

	    if ( !IS_NPC(ch) )
	    {
		if ( !IS_NPC(victim) )
		{
		    /*  Sheesh! can't do anything
		    send_to_char( "You can't do that on a player.\n\r", ch );
		    return TRUE;
	            */	
/*
		    if ( xIS_SET(victim->act, PLR_PK))
*/
		    if ( get_timer( ch, TIMER_PKILLED ) > 0 )
		    {
			send_to_char( "You have been killed in the last 5 minutes.\n\r", ch);
			return TRUE;
		    }
		    if ( get_timer( victim, TIMER_PKILLED ) > 0 )
		    {
			send_to_char( "This player has been killed in the last 5 minutes.\n\r", ch );
			return TRUE;
		    }	
		    /* Too many illegal pk attempts with cuffs, swats etc lately. - Luc */
		    if ( xIS_SET( ch->act, PLR_NICE ) )
		    {
			send_to_char( "You feel too nice to do that.\n\r", ch );
			return TRUE;
		    }
		    if ( victim != ch)
		        send_to_char( "You really shouldn't do this to another player...\n\r", ch );
		}

		if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
		{
		    send_to_char( "You can't do that on your own follower.\n\r", ch );
		    return TRUE;
		}
	    }

	    check_illegal_pk( ch, victim );
	    vo = (void *) victim;
	    break;

	case TAR_CHAR_DEFENSIVE:
	    {
	      if ( argument[0] != '\0'
	      &&  (victim=get_char_room(ch, argument)) == NULL )
	      {
		send_to_char( "They aren't here.\n\r", ch );
		return TRUE;
	      }
	      if ( !victim )
		victim = ch;
	    }

	    if ( ch == victim && SPELL_FLAG(skill_table[sn], SF_NOSELF))
            {
                send_to_char( "You can't target yourself!\n\r", ch);
                return TRUE;
            }

	    vo = (void *) victim;
	    break;

	case TAR_CHAR_SELF:
	    vo = (void *) ch;
	    break;

	case TAR_OBJ_INV:
	    {
	      if ( (obj=get_obj_carry(ch, argument)) == NULL )
	      {
		send_to_char( "You can't find that.\n\r", ch );
		return TRUE;
	      }
	    }
	    vo = (void *) obj;
	    break;
	}

	/* waitstate */
	WAIT_STATE( ch, skill_table[sn]->beats );
	/* check for failure */
	if ( (number_percent( ) + skill_table[sn]->difficulty * 5)
	   > (IS_NPC(ch) ? 75 : LEARNED(ch, sn)) )
	{
	    failed_casting( skill_table[sn], ch, vo, obj );
	    learn_from_failure( ch, sn );
	    if ( mana )
	    {
		if ( IS_VAMPIRE(ch) )
		  gain_condition( ch, COND_BLOODTHIRST, - blood/2 );
		else
		  ch->mana -= mana/2;
	    }
	    return TRUE;
	}
	if ( mana )
	{
	    if ( IS_VAMPIRE(ch) )
		gain_condition( ch, COND_BLOODTHIRST, - blood );
	    else
		ch->mana -= mana;
	}
	start_timer(&time_used);
	retcode = (*skill_table[sn]->spell_fun) ( sn, ch->level, ch, vo );
	end_timer(&time_used);
	update_userec(&time_used, &skill_table[sn]->userec);
	
	if ( retcode == rCHAR_DIED || retcode == rERROR )
	    return TRUE;

	if ( char_died(ch) )
	    return TRUE;

	if ( retcode == rSPELL_FAILED )
	{
	    learn_from_failure( ch, sn );
	    retcode = rNONE;
	}
	else
	    learn_from_success( ch, sn );

	if ( skill_table[sn]->target == TAR_CHAR_OFFENSIVE
	&&   victim != ch
	&&  !char_died(victim) )
	{
	    CHAR_DATA *vch;
	    CHAR_DATA *vch_next;

	    for ( vch = ch->in_room->first_person; vch; vch = vch_next )
	    {
		vch_next = vch->next_in_room;
		if ( victim == vch && !victim->fighting && victim->master != ch )
		{
		    retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
		    break;
		}
	    }
	}
	return TRUE;
    }

    if ( mana )
    {
	if ( IS_VAMPIRE(ch) )
	  gain_condition( ch, COND_BLOODTHIRST, - blood );
	else
	  ch->mana -= mana;
    }
    ch->prev_cmd = ch->last_cmd;    /* haus, for automapping */
    ch->last_cmd = skill_table[sn]->skill_fun;
    start_timer(&time_used);
    (*skill_table[sn]->skill_fun) ( ch, argument );
    end_timer(&time_used);
    update_userec(&time_used, &skill_table[sn]->userec);

    tail_chain( );
    return TRUE;
}

void do_skin( CHAR_DATA *ch, char *argument)
{
    OBJ_INDEX_DATA *korps;
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *skin;
    bool found;
    char *name;
    char buf[MAX_STRING_LENGTH];
    found = FALSE;
 
    if ( !IS_PKILL(ch) && !IS_IMMORTAL(ch) )
    {
        send_to_char( "Leave the hideous defilings to the killers!\n", ch );
        return;
    }
    if ( argument[0] == '\0' )
    { 
        send_to_char( "Whose corpse do you wish to skin?\n\r", ch );
        return;
    }
    if ( (corpse=get_obj_here(ch, argument)) == NULL )
    {
	send_to_char( "You cannot find that here.\n\r", ch );
	return;
    }
    if ( (obj=get_eq_char(ch, WEAR_WIELD)) == NULL )
    {
        send_to_char( "You have no weapon with which to perform this deed.\n\r", ch );
        return;
    }
    if ( !IS_IMMORTAL(ch) && !IS_OBJ_STAT(corpse, ITEM_CLANCORPSE ) )
    {
        send_to_char( "You can only skin the bodies of player characters.\n\r", ch);
        return;
    }
    if ( (obj->value[3] != 1
    &&   obj->value[3] != 2
    &&   obj->value[3] != 3
    &&   obj->value[3] != 11) ||
    IS_OBJ_STAT( corpse, ITEM_SKINNED) )
    {
        send_to_char( "There is nothing you can do with this corpse.\n\r", ch );
        return;
    }
    if ( !IS_IMMORTAL(ch) )
    {
        if ( IS_OBJ_STAT( corpse, ITEM_CLANCORPSE ) )
        {
	      if ( corpse->action_desc
              && is_name( ch->name, corpse->action_desc ) );
	      else 
	      {
	        send_to_char( "There is nothing you can do with this corpse.\n\r", ch);
        	return;
    	   }
	    }
	    else 
	    {
    		send_to_char( "There is nothing you can do with this corpse.\n\r", ch);
            return;
	    }
    }
    if ( get_obj_index( OBJ_VNUM_SKIN ) == NULL )
    {
      bug( "Vnum 23 (OBJ_VNUM_SKIN) not found for do_skin!", 0);
      return;
    }
    korps               = get_obj_index(OBJ_VNUM_CORPSE_PC);
    skin                = create_object( get_obj_index(OBJ_VNUM_SKIN), 0 );
    name                = IS_NPC(ch) ? korps->short_descr : corpse->short_descr;
    sprintf( buf, skin->short_descr, name );
    STRFREE( skin->short_descr );
    skin->short_descr = STRALLOC( buf );
    sprintf( buf, skin->description, name );
    STRFREE( skin->description );
    skin->description = STRALLOC( buf );
    act( AT_BLOOD, "$n strips the skin from $p.", ch, corpse, NULL, TO_ROOM);
    act( AT_BLOOD, "You strip the skin from $p.", ch, corpse, NULL, TO_CHAR);
    separate_obj(corpse);
    xSET_BIT( corpse->extra_flags, ITEM_SKINNED );
    obj_to_char( skin, ch );
    return;
}

/*
 * Lookup a skills information
 * High god command
 */
void do_slookup( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int sn;
    int iClass, iRace;
    SKILLTYPE *skill = NULL;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Slookup what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	for ( sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++ )
	    pager_printf( ch, "Sn: %4d Slot: %4d Skill/spell: '%-20s' Damtype: %s\n\r",
		sn, skill_table[sn]->slot, skill_table[sn]->name,
		spell_damage[SPELL_DAMAGE( skill_table[sn] )] );
    }
    else
    if ( !str_cmp( arg, "herbs" ) )
    {
	for ( sn = 0; sn < top_herb && herb_table[sn] && herb_table[sn]->name; sn++ )
	   pager_printf( ch, "%d) %s\n\r", sn, herb_table[sn]->name );
    }
    else
    {
	SMAUG_AFF *aff;
	int cnt = 0;

	if ( arg[0] == 'h' && is_number(arg+1) )
	{
	    sn = atoi(arg+1);
	    if ( !IS_VALID_HERB(sn) )
	    {
		send_to_char( "Invalid herb.\n\r", ch );
		return;
	    }
	    skill = herb_table[sn];
	}
	else
	if ( is_number(arg) )
	{
	    sn = atoi(arg);
	    if ( (skill=get_skilltype(sn)) == NULL )
	    {
		send_to_char( "Invalid sn.\n\r", ch );
		return;
	    }
	    sn %= 1000;
	}
	else
	if ( ( sn = skill_lookup( arg ) ) >= 0 )
	    skill = skill_table[sn];
	else
	if ( ( sn = herb_lookup( arg ) ) >= 0 )
	    skill = herb_table[sn];
	else
	{
	    send_to_char( "No such skill, spell, proficiency or tongue.\n\r", ch );
	    return;
	}
	if ( !skill )
	{
	    send_to_char( "Not created yet.\n\r", ch );
	    return;
	}

	ch_printf( ch, "Sn: %4d Slot: %4d %s: '%-20s'\n\r",
	    sn, skill->slot, skill_tname[skill->type], skill->name );
	if ( skill->info )
	    ch_printf( ch, "DamType: %s  ActType: %s   ClassType: %s   PowerType: %s\n\r",
		spell_damage[SPELL_DAMAGE(skill)],
		spell_action[SPELL_ACTION(skill)],
		spell_class[SPELL_CLASS(skill)],
		spell_power[SPELL_POWER(skill)] );
	if ( skill->flags )
	{
	    int x;

	    strcpy( buf, "Flags:" );
	    for ( x = 0; x < 32; x++ )
	      if ( SPELL_FLAG( skill, 1 << x ) )
	      {
		strcat( buf, " " );
		strcat( buf, spell_flag[x] );
	      }
	    strcat( buf, "\n\r" );
	    send_to_char( buf, ch );
	}
	ch_printf( ch, "Saves: %s  SaveEffect: %s\n\r",
	    spell_saves[(int) skill->saves],
	    spell_save_effect[SPELL_SAVE(skill)] );

	if ( skill->difficulty != '\0' )
	    ch_printf( ch, "Difficulty: %d\n\r", (int) skill->difficulty );

	ch_printf( ch, "Type: %s  Target: %s  Minpos: %d  Mana: %d  Beats: %d  Range: %d\n\r",
		skill_tname[skill->type],
		target_type[URANGE(TAR_IGNORE, skill->target, TAR_OBJ_INV)],
		skill->minimum_position,
		skill->min_mana, skill->beats, skill->range );
	ch_printf( ch, "Flags: %d  Guild: %d  Value: %d  Info: %d  Code: %s\n\r",
		skill->flags,
		skill->guild,
		skill->value,
		skill->info,
		skill->skill_fun ? skill_name(skill->skill_fun)
					   : spell_name(skill->spell_fun));
        ch_printf( ch, "Sectors Allowed: %s\n",
		skill->spell_sector?flag_string(skill->spell_sector,sec_flags):
			"All");
	ch_printf( ch, "Dammsg: %s\n\rWearoff: %s\n",
		skill->noun_damage,
		skill->msg_off ? skill->msg_off : "(none set)" );
	if ( skill->dice && skill->dice[0] != '\0' )
	    ch_printf( ch, "Dice: %s\n\r", skill->dice );
	if ( skill->teachers && skill->teachers[0] != '\0' )
	    ch_printf( ch, "Teachers: %s\n\r", skill->teachers );
	if ( skill->components && skill->components[0] != '\0' )
	    ch_printf( ch, "Components: %s\n\r", skill->components );
	if ( skill->participants )
	    ch_printf( ch, "Participants: %d\n\r", (int) skill->participants );
	if ( skill->userec.num_uses )
	    send_timer(&skill->userec, ch);
	for ( aff = skill->affects; aff; aff = aff->next )
	{
	    if ( aff == skill->affects )
	      send_to_char( "\n\r", ch );
	    sprintf( buf, "Affect %d", ++cnt );
	    if ( aff->location )
	    {
		strcat( buf, " modifies " );
		strcat( buf, a_types[aff->location % REVERSE_APPLY] );
		strcat( buf, " by '" );
		strcat( buf, aff->modifier );
		if ( aff->bitvector != -1 )
		  strcat( buf, "' and" );
		else
		  strcat( buf, "'" );
	    }
	    if ( aff->bitvector != -1 )
	    {
		strcat( buf, " applies " );
		strcat( buf, a_flags[aff->bitvector] );
	    }
	    if ( aff->duration[0] != '\0' && aff->duration[0] != '0' )
	    {
		strcat( buf, " for '" );
		strcat( buf, aff->duration );
		strcat( buf, "' rounds" );
	    }
	    if ( aff->location >= REVERSE_APPLY )
		strcat( buf, " (affects caster only)" );
	    strcat( buf, "\n\r" );
	    send_to_char( buf, ch );
	    
	    if ( !aff->next )
	      send_to_char( "\n\r", ch );
	}
	if ( skill->hit_char && skill->hit_char[0] != '\0' )
	    ch_printf( ch, "Hitchar   : %s\n\r", skill->hit_char );
	if ( skill->hit_vict && skill->hit_vict[0] != '\0' )
	    ch_printf( ch, "Hitvict   : %s\n\r", skill->hit_vict );
	if ( skill->hit_room && skill->hit_room[0] != '\0' )
	    ch_printf( ch, "Hitroom   : %s\n\r", skill->hit_room );
	if ( skill->hit_dest && skill->hit_dest[0] != '\0' )
	    ch_printf( ch, "Hitdest   : %s\n\r", skill->hit_dest );
	if ( skill->miss_char && skill->miss_char[0] != '\0' )
	    ch_printf( ch, "Misschar  : %s\n\r", skill->miss_char );
	if ( skill->miss_vict && skill->miss_vict[0] != '\0' )
	    ch_printf( ch, "Missvict  : %s\n\r", skill->miss_vict );
	if ( skill->miss_room && skill->miss_room[0] != '\0' )
	    ch_printf( ch, "Missroom  : %s\n\r", skill->miss_room );
	if ( skill->die_char && skill->die_char[0] != '\0' )
	    ch_printf( ch, "Diechar   : %s\n\r", skill->die_char );
	if ( skill->die_vict && skill->die_vict[0] != '\0' )
	    ch_printf( ch, "Dievict   : %s\n\r", skill->die_vict );
	if ( skill->die_room && skill->die_room[0] != '\0' )
	    ch_printf( ch, "Dieroom   : %s\n\r", skill->die_room );
	if ( skill->imm_char && skill->imm_char[0] != '\0' )
	    ch_printf( ch, "Immchar   : %s\n\r", skill->imm_char );
	if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
	    ch_printf( ch, "Immvict   : %s\n\r", skill->imm_vict );
	if ( skill->imm_room && skill->imm_room[0] != '\0' )
	    ch_printf( ch, "Immroom   : %s\n\r", skill->imm_room );
	if ( skill->type != SKILL_HERB )
	{
            if(skill->type!=SKILL_RACIAL) 
            {
               send_to_char( "--------------------------[CLASS USE]--------------------------\n\r",ch);
	       for ( iClass = 0; iClass < MAX_PC_CLASS; iClass++ )
	       {
		   strcpy( buf, class_table[iClass]->who_name );
		   sprintf(buf+3, ") lvl: %3d max: %2d%%",
				   skill->skill_level[iClass],
				   skill->skill_adept[iClass] );
		   if ( iClass % 3 == 2 )
			   strcat(buf, "\n\r" );
		   else
			   strcat(buf, "  " );
		   send_to_char( buf, ch );
	       }
            } else
            {
                 send_to_char( "\n\r--------------------------[RACE USE]--------------------------\n\r",ch);
	         for ( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
	         {
		         sprintf(buf, "%8.8s) lvl: %3d max: %2d%%",
                            race_table[iRace]->race_name,
		            skill->race_level[iRace],
		            skill->race_adept[iRace] );
                         if( !strcmp(race_table[iRace]->race_name,"unused") )
                            sprintf(buf,"                           ");
		         if ( (iRace>0) && (iRace % 2 == 1 ))
		            strcat(buf, "\n\r" );
		         else
		            strcat(buf, "  " );
		     send_to_char( buf, ch );
	         }
	     }

         }
	 send_to_char( "\n\r", ch );
           
    }

    return;
}






/*
 * Set a skill's attributes or what skills a player has.
 * High god command, with support for creating skills/spells/herbs/etc
 */
void do_sset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn,i;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: sset <victim> <skill> <value>\n\r",	ch );
	send_to_char( "or:     sset <victim> all     <value>\n\r",	ch );
	if ( get_trust(ch) >= LEVEL_ASCENDANT )
	{
	  send_to_char( "or:     sset save skill table\n\r",		ch );
	  send_to_char( "or:     sset save herb table\n\r",		ch );
	  send_to_char( "or:     sset create skill 'new skill'\n\r",	ch );
	  send_to_char( "or:     sset create herb 'new herb'\n\r",	ch );
	  send_to_char( "or:     sset create ability 'new ability'\n\r",ch );
	}
	if ( get_trust(ch) > LEVEL_GREATER )
	{
	  send_to_char( "or:     sset <sn>     <field> <value>\n\r",	ch );
	  send_to_char( "\n\rField being one of:\n\r",			ch );
	  send_to_char( "  name code target minpos slot mana beats dammsg wearoff guild minlevel\n\r", ch );
	  send_to_char( "  type damtype acttype classtype powertype seffect flag dice value difficulty\n\r", ch );
	  send_to_char( "  affect rmaffect level adept hit miss die imm (char/vict/room)\n\r", ch );
	  send_to_char( "  components teachers racelevel raceadept\n\r",ch );
	  send_to_char( "  sector\n\r", ch );
	  send_to_char( "Affect having the fields: <location> <modfifier> [duration] [bitvector]\n\r", ch );
	  send_to_char( "(See AFFECTTYPES for location, and AFFECTED_BY for bitvector)\n\r", ch );
	}
	send_to_char( "Skill being any skill or spell.\n\r",		ch );
	return;
    }

    if ( get_trust(ch) >= LEVEL_ASCENDANT
    &&  !str_cmp( arg1, "save" )
    &&	!str_cmp( argument, "table" ) )
    {
	if ( !str_cmp( arg2, "skill" ) )
	{
	    send_to_char( "Saving skill table...\n\r", ch );
	    save_skill_table();
	    save_classes();
	    /* save_races(); */
	    return;
	}
	if ( !str_cmp( arg2, "herb" ) )
	{
	    send_to_char( "Saving herb table...\n\r", ch );
	    save_herb_table();
	    return;
	}
    }
    if ( get_trust(ch) >= LEVEL_ASCENDANT
    &&  !str_cmp( arg1, "create" )
    && (!str_cmp( arg2, "skill" ) || !str_cmp( arg2, "herb" ) || !str_cmp( arg2, "ability" )) )
    {
	struct skill_type *skill;
	sh_int type = SKILL_UNKNOWN;

	if ( !str_cmp( arg2, "herb" ) )
	{
	    type = SKILL_HERB;
	    if ( top_herb >= MAX_HERB )
	    {
		ch_printf( ch, "The current top herb is %d, which is the maximum.  "
			   "To add more herbs,\n\rMAX_HERB will have to be "
			   "raised in mud.h, and the mud recompiled.\n\r",
			   top_herb );
		return;
	    }
	}
	else
	if ( top_sn >= MAX_SKILL )
	{
	    ch_printf( ch, "The current top sn is %d, which is the maximum.  "
			   "To add more skills,\n\rMAX_SKILL will have to be "
			   "raised in mud.h, and the mud recompiled.\n\r",
			   top_sn );
	    return;
	}
	CREATE( skill, struct skill_type, 1 );
	skill->slot = 0;
	if ( type == SKILL_HERB )
	{
	    int max, x;

	    herb_table[top_herb++] = skill;
	    for ( max = x = 0; x < top_herb-1; x++ )
		if ( herb_table[x] && herb_table[x]->slot > max )
		    max = herb_table[x]->slot;
	    skill->slot = max+1;
	}
	else
	    skill_table[top_sn++] = skill;
	skill->min_mana = 0;
	skill->name = str_dup( argument );
	skill->noun_damage = str_dup( "" );
	skill->msg_off = str_dup( "" );
	skill->spell_fun = spell_smaug;
	skill->type = type;
	skill->spell_sector = 0;
	skill->guild = -1;
        if (!str_cmp( arg2, "ability" ) )
          skill->type  = SKILL_RACIAL;

        for(i=0;i<MAX_PC_CLASS;i++)
        {
             skill->skill_level[i]= LEVEL_IMMORTAL;
             skill->skill_adept[i]= 95;
        }
        for(i=0;i<MAX_PC_RACE;i++)
        {
             skill->race_level[i]= LEVEL_IMMORTAL;
             skill->race_adept[i]= 95;
        }

	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( arg1[0] == 'h' )
	sn = atoi( arg1+1 );
    else
	sn = atoi( arg1 );
    if ( get_trust(ch) > LEVEL_GREATER
    && ((arg1[0] == 'h' && is_number(arg1+1) && (sn=atoi(arg1+1))>=0)
    ||  (is_number(arg1) && (sn=atoi(arg1)) >= 0)) )
    {
	struct skill_type *skill;

	if ( arg1[0] == 'h' )
	{
	    if ( sn >= top_herb )
	    {
		send_to_char( "Herb number out of range.\n\r", ch );
		return;
	    }
	    skill = herb_table[sn];
	}
	else
	{
	    if ( (skill=get_skilltype(sn)) == NULL )
	    {
		send_to_char( "Skill number out of range.\n\r", ch );
		return;
	    }
	    sn %= 1000;
	}

	if ( !str_cmp( arg2, "difficulty" ) )
	{
	    skill->difficulty = atoi( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "participants" ) )
	{
	    skill->participants = atoi( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "damtype" ) )
	{
	    int x = get_sdamage( argument );

	    if ( x == -1 )
		send_to_char( "Not a spell damage type.\n\r", ch );
	    else
	    {
		SET_SDAM( skill, x );
		send_to_char( "Ok.\n\r", ch );
	    }
	    return;
	}
	if ( !str_cmp( arg2, "acttype" ) )
	{
	    int x = get_saction( argument );

	    if ( x == -1 )
		send_to_char( "Not a spell action type.\n\r", ch );
	    else
	    {
		SET_SACT( skill, x );
		send_to_char( "Ok.\n\r", ch );
	    }
	    return;
	}
	if ( !str_cmp( arg2, "classtype" ) )
	{
	    int x = get_sclass( argument );

	    if ( x == -1 )
		send_to_char( "Not a spell class type.\n\r", ch );
	    else
	    {
		SET_SCLA( skill, x );
		send_to_char( "Ok.\n\r", ch );
	    }
	    return;
	}
	if ( !str_cmp( arg2, "powertype" ) )
	{
	    int x = get_spower( argument );

	    if ( x == -1 )
		send_to_char( "Not a spell power type.\n\r", ch );
	    else
	    {
		SET_SPOW( skill, x );
		send_to_char( "Ok.\n\r", ch );
	    }
	    return;
	}
	if ( !str_cmp( arg2, "seffect" ) )
	{
	    int x = get_ssave_effect( argument );

	    if ( x == -1 )
		send_to_char( "Not a spell save effect type.\n\r", ch );
	    else
	    {
		SET_SSAV( skill, x );
		send_to_char( "Ok.\n\r", ch );
	    }
	    return;
	}
	if ( !str_cmp( arg2, "flag" ) )
	{
	    int x = get_sflag( argument );

	    if ( x == -1 )
		send_to_char( "Not a spell flag.\n\r", ch );
	    else
	    {
		TOGGLE_BIT( skill->flags, 1 << x );
		send_to_char( "Ok.\n\r", ch );
	    }
	    return;
	}
	if ( !str_cmp( arg2, "saves" ) )
	{
	    int x = get_ssave( argument );

	    if ( x == -1 )
		send_to_char( "Not a saving type.\n\r", ch );
	    else
	    {
		skill->saves = x;
		send_to_char( "Ok.\n\r", ch );
	    }
	    return;
	}

	if ( !str_cmp( arg2, "code" ) )
	{
	    SPELL_FUN *spellfun;
	    DO_FUN    *dofun;
		
	    if ( (spellfun=spell_function(argument)) != spell_notfound )
	    {
		skill->spell_fun = spellfun;
		skill->skill_fun = NULL;
	    }
	    else
	    if ( (dofun=skill_function(argument)) != skill_notfound )
	    {
		skill->skill_fun = dofun;
		skill->spell_fun = NULL;
	    }
	    else
	    {
		send_to_char( "Not a spell or skill.\n\r", ch );
		return;
	    }
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}

	if ( !str_cmp( arg2, "target" ) )
	{
	    int x = get_starget( argument );

	    if ( x == -1 )
		send_to_char( "Not a valid target type.\n\r", ch );
	    else
	    {
		skill->target = x;
		send_to_char( "Ok.\n\r", ch );
	    }
	    return;
	}
	if ( !str_cmp( arg2, "minpos" ) )
	{
	    skill->minimum_position = URANGE( POS_DEAD, atoi( argument ), POS_DRAG );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "minlevel" ) )
	{
	    skill->min_level = URANGE( 1, atoi( argument ), MAX_LEVEL );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "sector") )
	{
	    char tmp_arg[MAX_STRING_LENGTH];

	    while ( argument[0] != '\0' )
	    {
	       argument = one_argument( argument, tmp_arg );
	       value = get_secflag( tmp_arg );
	       if ( value < 0 || value > MAX_SECFLAG )
	          ch_printf( ch, "Unknown flag: %s\n\r", tmp_arg );
	       else
	          TOGGLE_BIT( skill->spell_sector, (1 << value) );
            }
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "slot" ) )
	{
	    skill->slot = URANGE( 0, atoi( argument ), 30000 );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "mana" ) )
	{
	    skill->min_mana = URANGE( 0, atoi( argument ), 2000 );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "beats" ) )
	{
	    skill->beats = URANGE( 0, atoi( argument ), 120 );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "range" ) )
	{
	    skill->range = URANGE( 0, atoi( argument ), 20 );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "guild" ) )
	{
	    skill->guild = atoi( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "value" ) )
	{
	    skill->value = atoi( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "type" ) )
	{
	    skill->type = get_skill( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "rmaffect" ) )
	{
	    SMAUG_AFF *aff = skill->affects;
	    SMAUG_AFF *aff_next;
	    int num = atoi( argument );
	    int cnt = 1;

	    if ( !aff )
	    {
		send_to_char( "This spell has no special affects to remove.\n\r", ch );
		return;
	    }
	    if ( num == 1 )
	    {
		skill->affects = aff->next;
		DISPOSE( aff->duration );
		DISPOSE( aff->modifier );
		DISPOSE( aff );
		send_to_char( "Removed.\n\r", ch );
		return;
	    }
	    for ( ; aff; aff = aff->next )
	    {
		if ( ++cnt == num && (aff_next=aff->next) != NULL )
		{
		    aff->next = aff_next->next;
		    DISPOSE( aff_next->duration );
		    DISPOSE( aff_next->modifier );
		    DISPOSE( aff_next );
		    send_to_char( "Removed.\n\r", ch );
		    return;
		}
	    }
	    send_to_char( "Not found.\n\r", ch );
	    return;
	}
	/*
	 * affect <location> <modifier> <duration> <bitvector>
	 */
	if ( !str_cmp( arg2, "affect" ) )
	{
	    char location[MAX_INPUT_LENGTH];
	    char modifier[MAX_INPUT_LENGTH];
	    char duration[MAX_INPUT_LENGTH];
/*	    char bitvector[MAX_INPUT_LENGTH];	*/
	    int loc, bit, tmpbit;
	    SMAUG_AFF *aff;

	    argument = one_argument( argument, location );
	    argument = one_argument( argument, modifier );
	    argument = one_argument( argument, duration );
	    
	    if ( location[0] == '!' )
		loc = get_atype( location+1 ) + REVERSE_APPLY;
	    else
		loc = get_atype( location );
	    if ( (loc % REVERSE_APPLY) < 0
	    ||   (loc % REVERSE_APPLY) >= MAX_APPLY_TYPE )
	    {
		send_to_char( "Unknown affect location.  See AFFECTTYPES.\n\r", ch );
		return;
	    }
	    bit = -1;
	    if ( argument[0] != '\0' )
	    {
		if ( (tmpbit=get_aflag(argument)) == -1 )
		  ch_printf( ch, "Unknown bitvector: %s.  See AFFECTED_BY\n\r", argument );
		else
		  bit = tmpbit;
	    }
	    CREATE( aff, SMAUG_AFF, 1 );
	    if ( !str_cmp( duration, "0" ) )
	      duration[0] = '\0';
	    if ( !str_cmp( modifier, "0" ) )
	      modifier[0] = '\0';
	    aff->duration = str_dup( duration );
	    aff->location = loc;
	    aff->modifier = str_dup( modifier );
	    aff->bitvector = bit;
	    aff->next = skill->affects;
	    skill->affects = aff;
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "level" ) )
	{
	    char arg3[MAX_INPUT_LENGTH];
	    int class;

	    argument = one_argument( argument, arg3 );
	    class = atoi( arg3 );
	    if ( class >= MAX_PC_CLASS || class < 0 )
		send_to_char( "Not a valid class.\n\r", ch );
	    else
		skill->skill_level[class] =
			URANGE(0, atoi(argument), MAX_LEVEL);
	    return;
        }
	if ( !str_cmp( arg2, "racelevel" ) )
	{
	    char arg3[MAX_INPUT_LENGTH];
	    int race;

	    argument = one_argument( argument, arg3 );
	    race = atoi( arg3 );
	    if ( race >= MAX_PC_RACE || race < 0 )
		send_to_char( "Not a valid race.\n\r", ch );
	    else
		skill->race_level[race] =
			URANGE(0, atoi(argument), MAX_LEVEL);
	    return;
	}
	if ( !str_cmp( arg2, "adept" ) )
	{
	    char arg3[MAX_INPUT_LENGTH];
	    int class;

	    argument = one_argument( argument, arg3 );
	    class = atoi( arg3 );
	    if ( class >= MAX_PC_CLASS || class < 0 )
		send_to_char( "Not a valid class.\n\r", ch );
	    else
		skill->skill_adept[class] =
			URANGE(0, atoi(argument), 100);
	    return;
	}
	if ( !str_cmp( arg2, "raceadept" ) )
	{
	    char arg3[MAX_INPUT_LENGTH];
	    int race;

	    argument = one_argument( argument, arg3 );
	    race = atoi( arg3 );
	    if ( race >= MAX_PC_RACE || race < 0 )
		send_to_char( "Not a valid race.\n\r", ch );
	    else
		skill->race_adept[race] =
			URANGE(0, atoi(argument), 100);
	    return;
	}


	if ( !str_cmp( arg2, "name" ) )
	{
	    DISPOSE(skill->name);
	    skill->name = str_dup( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "dammsg" ) )
	{
	    DISPOSE(skill->noun_damage);
	    if ( !str_cmp( argument, "clear" ) )
	      skill->noun_damage = str_dup( "" );
	    else
	      skill->noun_damage = str_dup( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "wearoff" ) )
	{
	    DISPOSE(skill->msg_off);
	    if ( str_cmp( argument, "clear" ) )
	      skill->msg_off = str_dup( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "hitchar" ) )
	{
	    if ( skill->hit_char )
	      DISPOSE(skill->hit_char);
	    if ( str_cmp( argument, "clear" ) )
	      skill->hit_char = str_dup( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "hitvict" ) )
	{
	    if ( skill->hit_vict )
	      DISPOSE(skill->hit_vict);
	    if ( str_cmp( argument, "clear" ) )
	      skill->hit_vict = str_dup( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "hitroom" ) )
	{
	    if ( skill->hit_room )
	      DISPOSE(skill->hit_room);
	    if ( str_cmp( argument, "clear" ) )
	      skill->hit_room = str_dup( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "hitdest" ) )
	{
	    if ( skill->hit_dest )
	      DISPOSE(skill->hit_dest);
	    if ( str_cmp( argument, "clear" ) )
	      skill->hit_dest = str_dup( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "misschar" ) )
	{
	    if ( skill->miss_char )
	      DISPOSE(skill->miss_char);
	    if ( str_cmp( argument, "clear" ) )
	      skill->miss_char = str_dup( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "missvict" ) )
	{
	    if ( skill->miss_vict )
	      DISPOSE(skill->miss_vict);
	    if ( str_cmp( argument, "clear" ) )
	      skill->miss_vict = str_dup( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "missroom" ) )
	{
	    if ( skill->miss_room )
	      DISPOSE(skill->miss_room);
	    if ( str_cmp( argument, "clear" ) )
	      skill->miss_room = str_dup( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "diechar" ) )
	{
	    if ( skill->die_char )
	      DISPOSE(skill->die_char);
	    if ( str_cmp( argument, "clear" ) )
	      skill->die_char = str_dup( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "dievict" ) )
	{
	    if ( skill->die_vict )
	      DISPOSE(skill->die_vict);
	    if ( str_cmp( argument, "clear" ) )
	      skill->die_vict = str_dup( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "dieroom" ) )
	{
	    if ( skill->die_room )
	      DISPOSE(skill->die_room);
	    if ( str_cmp( argument, "clear" ) )
	      skill->die_room = str_dup( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "immchar" ) )
	{
	    if ( skill->imm_char )
	      DISPOSE(skill->imm_char);
	    if ( str_cmp( argument, "clear" ) )
	      skill->imm_char = str_dup( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "immvict" ) )
	{
	    if ( skill->imm_vict )
	      DISPOSE(skill->imm_vict);
	    if ( str_cmp( argument, "clear" ) )
	      skill->imm_vict = str_dup( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "immroom" ) )
	{
	    if ( skill->imm_room )
	      DISPOSE(skill->imm_room);
	    if ( str_cmp( argument, "clear" ) )
	      skill->imm_room = str_dup( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "dice" ) )
	{
	    if ( skill->dice )
	      DISPOSE(skill->dice);
	    if ( str_cmp( argument, "clear" ) )
	      skill->dice = str_dup( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "components" ) )
	{
	    if ( skill->components )
	      DISPOSE(skill->components);
	    if ( str_cmp( argument, "clear" ) )
	      skill->components = str_dup( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg2, "teachers" ) )
	{
	    if ( skill->teachers )
	      DISPOSE(skill->teachers);
	    if ( str_cmp( argument, "clear" ) )
	      skill->teachers = str_dup( argument );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
	do_sset( ch, "" );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	if ( (sn = skill_lookup(arg1)) >= 0 )
	{
	    sprintf(arg1, "%d %s %s", sn, arg2, argument);
	    do_sset(ch, arg1);
	}
	else
	    send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
	send_to_char( "No such skill or spell.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( argument ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }

    value = atoi( argument );
    if ( value < 0 || value > 100 )
    {
	send_to_char( "Value range is 0 to 100.\n\r", ch );
	return;
    }

    if ( fAll )
    {
	for ( sn = 0; sn < top_sn; sn++ )
	{
            /* Fix by Narn to prevent ssetting skills the player shouldn't have. */ 
	    if ( skill_table[sn]->name 
            && ( victim->level >= skill_table[sn]->skill_level[victim->class] 
                      || value == 0 ) )
	    {
		if ( value == 100 && !IS_IMMORTAL( victim ) )
		  victim->pcdata->learned[sn] = GET_ADEPT( victim, sn );
		else
		  victim->pcdata->learned[sn] = value;
	    }
	}
    }
    else
	victim->pcdata->learned[sn] = value;

    return;
}


void learn_from_success( CHAR_DATA *ch, int sn )
{
    int adept, gain, sklvl, learn, percent, chance;

    if ( IS_NPC(ch) || ch->pcdata->learned[sn] <= 0 )
	return;
    adept = GET_ADEPT(ch,sn);
    sklvl = skill_table[sn]->skill_level[ch->class];
    if ( sklvl == 0 )
	sklvl = ch->level;
    if ( ch->pcdata->learned[sn] < adept )
    {
	chance = ch->pcdata->learned[sn] + (5 * skill_table[sn]->difficulty);
	percent = number_percent();
	if ( percent >= chance )
	  learn = 2;
	else
	if ( chance - percent > 25 )
	  return;
	else
	  learn = 1;
	ch->pcdata->learned[sn] = UMIN( adept, ch->pcdata->learned[sn] + learn );
	if ( ch->pcdata->learned[sn] == adept )	 /* fully learned! */
	{
	    gain = 1000 * sklvl;
            if ( ch->class==CLASS_MAGE ) gain *= 5;	/* h, mage upgrade */
            if ( ch->class==CLASS_CLERIC ) gain *= 2;	/* h, mage upgrade */
	    set_char_color( AT_WHITE, ch );
	    ch_printf( ch, "You are now an adept of %s!  You gain %d bonus experience!\n\r",
		skill_table[sn]->name, gain );
	}
	else
	{
	    gain = 20 * sklvl;
            if ( ch->class==CLASS_MAGE ) gain *= 6;	/* h, mage upgrade */
            if ( ch->class==CLASS_CLERIC ) gain *= 3;	/* h, mage upgrade */
	    if ( !ch->fighting && sn != gsn_hide && sn != gsn_sneak )
	    {
		set_char_color( AT_WHITE, ch );
		ch_printf( ch, "You gain %d experience points from your success!\n\r", gain );
	    }
	}
	gain_exp( ch, gain );
    }
}


void learn_from_failure( CHAR_DATA *ch, int sn )
{
    int adept, chance;

    if ( IS_NPC(ch) || ch->pcdata->learned[sn] <= 0 )
	return;
    chance = ch->pcdata->learned[sn] + (5 * skill_table[sn]->difficulty);
    if ( chance - number_percent() > 25 )
	return;
    adept = GET_ADEPT(ch, sn);
    if ( ch->pcdata->learned[sn] < (adept-1) )
	ch->pcdata->learned[sn] = UMIN( adept, ch->pcdata->learned[sn] + 1 );
}

void do_grapple( CHAR_DATA *ch, char *argument )
{

    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    sh_int percent;
    AFFECT_DATA af;

    if ( !IS_PKILL( ch ) && !IS_IMMORTAL( ch ) )
	return;

    if ( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't do that right now.\n\r", ch );
	return;
    }

    if ( ch->mount )
    {
        send_to_char( "You can't get close enough while mounted.\n\r", ch );
        return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
      one_argument( argument, arg );
      if ( arg[0] == '\0' )
      {
	  act( AT_ACTION, "You move in a circle looking for someone to grapple.", ch, NULL, NULL, TO_CHAR );
	  act( AT_ACTION, "$n moves in a circle looking for someone to grapple.", ch, NULL, NULL, TO_ROOM );
          return;
      }

      if ( ( victim = get_char_room( ch, arg ) ) == NULL )
      {
        send_to_char( "They aren't here.\n\r", ch );
        return;
      }

      if ( victim == ch )
      {
        send_to_char( "How can you sneak up on yourself?\n\r", ch );
        return;
      }
    }

    if ( IS_NPC( victim ) )
    {
	send_to_char( "Stick to wrestling players.\n\r", ch );
	return;
    }
  
    if ( IS_AFFECTED( victim, AFF_GRAPPLE) )
	return;

    if ( is_safe( ch, victim, TRUE ) )
    {
        send_to_char( "A magical force prevents you from attacking.\n\r", ch );
        return;
    }

    if ( who_fighting( ch ) && who_fighting( ch ) != victim )
    {
	send_to_char( "You're fighting someone else!\n\r", ch );
	return;
    }

    if ( who_fighting( victim ) && who_fighting( victim ) != ch )
    {
	send_to_char( "You can't get close enough.\n\r", ch );
	return;
    }

    percent = LEARNED( ch, gsn_grapple );
    WAIT_STATE( ch, skill_table[gsn_grapple]->beats );

    if ( !chance( ch, percent ) )
    {
	send_to_char( "You lost your balance.\n\r", ch);
	act( AT_ACTION, "$n tries to grapple you but can't get close enough.", ch, NULL, victim, TO_VICT );
/*
	act( AT_ACTION, "$n tries to grapple with $N.", ch, NULL, victim, TO_NOTVICT );
*/
	learn_from_failure( ch, gsn_grapple );
	check_illegal_pk( ch, victim );
	return;
    }

    af.type	 = gsn_grapple;
    af.duration  = 2;
    af.location  = APPLY_DEX;
    af.modifier  = -2;
    af.bitvector = meb( AFF_GRAPPLE );
    affect_to_char( victim, &af);
    af.type	 = gsn_grapple;
    af.duration	 = 2;
    af.location	 = APPLY_DEX;
    af.modifier  = -2;
    af.bitvector = meb( AFF_GRAPPLE );
    affect_to_char( ch, &af );
    ch_printf( ch, "You manage to grab hold of %s!\n\r", capitalize( victim->name ) );
    act( AT_ACTION, "$n grabs hold of you!", ch, NULL, victim, TO_VICT );
    act( AT_ACTION, "$n begins grappling with $N!", ch, NULL, victim, TO_NOTVICT );
    learn_from_success( ch, gsn_grapple );
    check_illegal_pk( ch, victim );
    if ( !ch->fighting && victim->in_room == ch->in_room )
	set_fighting( ch, victim );
    if ( !victim->fighting && ch->in_room == victim->in_room)
        set_fighting( victim, ch );
    return;
}

void do_gouge( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    sh_int dam;
    int chance;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    if ( !can_use_skill(ch, 0, gsn_gouge ) )
    {
	send_to_char("You do not yet know of this skill.\n\r", ch );
	return;
    }

    if ( ch->mount )
    {
	send_to_char( "You can't get close enough while mounted.\n\r", ch );
	return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    chance = (( get_curr_dex(victim) - get_curr_dex(ch)) * 10) + 10;
    if ( !IS_NPC(ch) && !IS_NPC(victim) )
	chance += sysdata.gouge_plr_vs_plr;
    if ( victim->fighting && victim->fighting->who != ch )
	chance += sysdata.gouge_nontank;
    if ( can_use_skill( ch, (number_percent () + chance), gsn_gouge ) )
    {
	dam = number_range( 5, ch->level );
	global_retcode = damage( ch, victim, dam, gsn_gouge );
	if ( global_retcode == rNONE )
	{
		if ( !IS_AFFECTED( victim, AFF_BLIND ) )
		{
		  af.type      = gsn_blindness;
		  af.location  = APPLY_HITROLL;
		  af.modifier  = -6;
		  if ( !IS_NPC( victim ) && !IS_NPC( ch ) )
		    af.duration = ( ch->level+10 ) / get_curr_con( victim );
		  else
		    af.duration  = 3 + (ch->level/15);
		  af.bitvector = meb(AFF_BLIND);
		  affect_to_char( victim, &af );
		  act( AT_SKILL, "You can't see a thing!", victim, NULL, NULL, TO_CHAR );
		}
		WAIT_STATE( ch,     PULSE_VIOLENCE );
		if ( !IS_NPC( ch ) && !IS_NPC( victim ) )
		{
		  if ( number_bits( 1 ) == 0 )
		  {
		    ch_printf( ch, "%s looks momentarily dazed.\n\r",
			victim->name );
		    send_to_char( "You are momentarily dazed ...\n\r",
			victim );
		    WAIT_STATE( victim, PULSE_VIOLENCE );
		  }
		}
		else
		  WAIT_STATE( victim, PULSE_VIOLENCE );
		/* Taken out by request - put back in by Thoric
		 * This is how it was designed.  You'd be a tad stunned
		 * if someone gouged you in the eye.
		 * Mildly modified by Blodkai, Feb 1998 at request of
		 * of pkill Conclave (peaceful use remains the same)
		 */
	}
	else
	if ( global_retcode == rVICT_DIED )
	{
	    act( AT_BLOOD, "Your fingers plunge into your victim's brain, causing immediate death!",
		ch, NULL, NULL, TO_CHAR );
	}
	if ( global_retcode != rCHAR_DIED && global_retcode != rBOTH_DIED )
	    learn_from_success( ch, gsn_gouge );
    }
    else
    {
	WAIT_STATE( ch, skill_table[gsn_gouge]->beats );
	global_retcode = damage( ch, victim, 0, gsn_gouge );
	learn_from_failure( ch, gsn_gouge );
    }

    return;
}

void do_detrap( CHAR_DATA *ch, char *argument )
{
    char arg  [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *trap;
    int percent;
    bool found = FALSE;

    switch( ch->substate )
    {
	default:
	    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
	    {
		send_to_char( "You can't concentrate enough for that.\n\r", ch );
		return;
	    }
	    argument = one_argument( argument, arg );
            if ( !can_use_skill(ch, 0, gsn_detrap ) )
	    {
		send_to_char("You do not yet know of this skill.\n\r", ch );
		return;
	    }
	    if ( arg[0] == '\0' )
	    {
		send_to_char( "Detrap what?\n\r", ch );
		return;
	    }
	    if ( ms_find_obj(ch) )
		return;
	    found = FALSE;
	    if ( ch->mount )
	    {
		send_to_char( "You can't do that while mounted.\n\r", ch );
		return;
	    }
	    if ( !ch->in_room->first_content )
	    {
	       send_to_char( "You can't find that here.\n\r", ch );
	       return;
	    }
	    for ( obj = ch->in_room->first_content; obj; obj = obj->next_content )
	    {
	       if ( can_see_obj( ch, obj ) && nifty_is_name( arg, obj->name ) )
	       {
		  found = TRUE;
		  break;
	       }
	    }
	    if ( !found )
	    {
	       send_to_char( "You can't find that here.\n\r", ch );
	       return;
	    }
	    act( AT_ACTION, "You carefully begin your attempt to remove a trap from $p...", ch, obj, NULL, TO_CHAR );
	    act( AT_ACTION, "$n carefully attempts to remove a trap from $p...", ch, obj, NULL, TO_ROOM );
	    ch->alloc_ptr = str_dup( obj->name );
	    add_timer( ch, TIMER_DO_FUN, 3, do_detrap, 1 );
/*	    WAIT_STATE( ch, skill_table[gsn_detrap]->beats ); */
	    return;
	case 1:
	    if ( !ch->alloc_ptr )
	    {
		send_to_char( "Your detrapping was interrupted!\n\r", ch );
		bug( "do_detrap: ch->alloc_ptr NULL!", 0 );
		return;
	    }
	    strcpy( arg, ch->alloc_ptr );
	    DISPOSE( ch->alloc_ptr );
	    ch->alloc_ptr = NULL;
	    ch->substate = SUB_NONE;
	    break;
	case SUB_TIMER_DO_ABORT:
	    DISPOSE(ch->alloc_ptr);
	    ch->substate = SUB_NONE;
	    send_to_char( "You carefully stop what you were doing.\n\r", ch );
	    return;
    }

    if ( !ch->in_room->first_content )
    {
       send_to_char( "You can't find that here.\n\r", ch );
       return;
    }
    for ( obj = ch->in_room->first_content; obj; obj = obj->next_content )
    {
       if ( can_see_obj( ch, obj ) && nifty_is_name( arg, obj->name ) )
       {
	  found = TRUE;
	  break;
       }
    }
    if ( !found )
    {
       send_to_char( "You can't find that here.\n\r", ch );
       return;
    }
    if ( (trap = get_trap( obj )) == NULL )
    {
       send_to_char( "You find no trap on that.\n\r", ch );
       return;
    }

    percent  = number_percent( ) - ( ch->level / 15 ) 
               - (get_curr_lck(ch) - 16);

    separate_obj(obj);
    if ( !can_use_skill( ch, percent, gsn_detrap ) )
    {
	send_to_char( "Ooops!\n\r", ch );
	spring_trap( ch, trap );
	learn_from_failure( ch, gsn_detrap );
	return;
    }

    extract_obj( trap );

    send_to_char( "You successfully remove a trap.\n\r", ch );
    learn_from_success( ch, gsn_detrap );
    return;
}

void do_dig( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *startobj;
    bool found, shovel;
    EXIT_DATA *pexit;
   
    switch( ch->substate )
    {
	default:
	  if ( IS_NPC(ch)  && IS_AFFECTED( ch, AFF_CHARM ) )
	  {
	    send_to_char( "You can't concentrate enough for that.\n\r", ch );
	    return;
	  }
          if ( ch->mount )
	  {
	    send_to_char( "You can't do that while mounted.\n\r", ch );
	    return;
	  }
	  one_argument( argument, arg );
	  if ( arg[0] != '\0' )
	  {
	      if ( ( pexit = find_door( ch, arg, TRUE ) ) == NULL
	      &&     get_dir(arg) == -1 )
	      {
		  send_to_char( "What direction is that?\n\r", ch );
		  return;
	      }
	      if ( pexit )
	      {
		  if ( !IS_SET(pexit->exit_info, EX_DIG)
		  &&   !IS_SET(pexit->exit_info, EX_CLOSED) )
		  {
		      send_to_char( "There is no need to dig out that exit.\n\r", ch );
		      return;
		  }
	      }
	  }
	  else
	  {
	      switch( ch->in_room->sector_type )
	      {
		  case SECT_CITY:
		  case SECT_INSIDE:
		    send_to_char( "The floor is too hard to dig through.\n\r", ch );
		    return;
		  case SECT_WATER_SWIM:
		  case SECT_WATER_NOSWIM:
		  case SECT_UNDERWATER:
		    send_to_char( "You cannot dig here.\n\r", ch );
		    return;
		  case SECT_AIR:
		    send_to_char( "What?  In the air?!\n\r", ch );
		    return;
	      }
	  }
	  add_timer( ch, TIMER_DO_FUN, UMIN(skill_table[gsn_dig]->beats / 10, 3), 	
		do_dig, 1);
	  ch->alloc_ptr = str_dup( arg );
	  send_to_char( "You begin digging...\n\r", ch );
 	  act( AT_PLAIN, "$n begins digging...", ch, NULL, NULL, TO_ROOM );
	  return;

	case 1:
	  if ( !ch->alloc_ptr )
	  {
	      send_to_char( "Your digging was interrupted!\n\r", ch );
	      act( AT_PLAIN, "$n's digging was interrupted!", ch, NULL, NULL, TO_ROOM );
	      bug( "do_dig: alloc_ptr NULL", 0 );
	      return;
	  }
	  strcpy( arg, ch->alloc_ptr );  
	  DISPOSE( ch->alloc_ptr );	
	  break;

	case SUB_TIMER_DO_ABORT:
	  DISPOSE( ch->alloc_ptr );
	  ch->substate = SUB_NONE;
	  send_to_char( "You stop digging...\n\r", ch );
	  act( AT_PLAIN, "$n stops digging...", ch, NULL, NULL, TO_ROOM );
	  return;
    }

    ch->substate = SUB_NONE;

    /* not having a shovel makes it harder to succeed */
    shovel = FALSE;
    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
      if ( obj->item_type == ITEM_SHOVEL )
      {
	  shovel = TRUE;
	  break;
      }

    /* dig out an EX_DIG exit... */
    if ( arg[0] != '\0' )
    {
	if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL
	&&     IS_SET( pexit->exit_info, EX_DIG )
	&&     IS_SET( pexit->exit_info, EX_CLOSED ) )
	{
	    /* 4 times harder to dig open a passage without a shovel */
/*
	    if ( (number_percent() * (shovel ? 1 : 4)) <
		 LEARNED(ch, gsn_dig) )
*/
            if ( can_use_skill( ch, (number_percent() * (shovel ? 1 : 4)),
			gsn_dig ) )
	    {
                if ( !IS_SET( pexit->exit_info, EX_ISDOOR ) )
		{
		remove_bexit_flag( pexit, EX_CLOSED );
		send_to_char( "You dig open a passageway!\n\r", ch );
		act( AT_PLAIN, "$n digs open a passageway!", ch, NULL, NULL, TO_ROOM );
		}
		else
		{
		REMOVE_BIT( pexit->exit_info, EX_DIG );
		send_to_char( "You uncover a doorway!\n\r", ch );
		act( AT_PLAIN, "$n uncovers a doorway!", ch, NULL, NULL, TO_ROOM );
		}
		learn_from_success( ch, gsn_dig );
		return;
	    }
	}
	learn_from_failure( ch, gsn_dig );
	send_to_char( "Your dig did not discover any exit...\n\r", ch );
	act( AT_PLAIN, "$n's dig did not discover any exit...", ch, NULL, NULL, TO_ROOM );
	return;
    }

    startobj = ch->in_room->first_content;
    found = FALSE;
    
    for ( obj = startobj; obj; obj = obj->next_content )
    {
	/* twice as hard to find something without a shovel */
	if ( IS_OBJ_STAT( obj, ITEM_BURIED )
          && ( can_use_skill( ch, (number_percent () * (shovel ? 1 : 2)),
                gsn_dig ) ) )
/*
	&&  (number_percent() * (shovel ? 1 : 2)) <
	    (IS_NPC(ch) ? 80 : ch->pcdata->learned[gsn_dig]) )
*/
	{
	  found = TRUE;
	  break;
	}
    }

    if ( !found )
    {
	send_to_char( "Your dig uncovered nothing.\n\r", ch );
	act( AT_PLAIN, "$n's dig uncovered nothing.", ch, NULL, NULL, TO_ROOM );
	learn_from_failure( ch, gsn_dig );
	return;
    }

    separate_obj(obj);
    xREMOVE_BIT( obj->extra_flags, ITEM_BURIED );
    act( AT_SKILL, "Your dig uncovered $p!", ch, obj, NULL, TO_CHAR );
    act( AT_SKILL, "$n's dig uncovered $p!", ch, obj, NULL, TO_ROOM );
    learn_from_success( ch, gsn_dig );
    if ( obj->item_type == ITEM_CORPSE_PC
    ||   obj->item_type == ITEM_CORPSE_NPC )
	adjust_favor( ch, 14, 1 );

    return;
} 

    
void do_search( CHAR_DATA *ch, char *argument )
{
    char arg  [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *container;
    OBJ_DATA *startobj;
    int percent, door;

    door = -1;
    switch( ch->substate )
    {
	default:
	    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
	    {
		send_to_char( "You can't concentrate enough for that.\n\r", ch );
		return;
	    }
	    if ( ch->mount )
	    {
		send_to_char( "You can't do that while mounted.\n\r", ch );
		return;
	    }
	    argument = one_argument( argument, arg );
	    if ( arg[0] != '\0' && (door = get_door( arg )) == -1 )
	    {
		container = get_obj_here( ch, arg );
		if ( !container )
		{
		  send_to_char( "You can't find that here.\n\r", ch );
		  return;
		}
		if ( container->item_type != ITEM_CONTAINER )
		{
		  send_to_char( "You can't search in that!\n\r", ch );
		  return;
		}
		if ( IS_SET(container->value[1], CONT_CLOSED) )
		{
		  send_to_char( "It is closed.\n\r", ch );
		  return;
		}
	    }
	    add_timer( ch, TIMER_DO_FUN, UMIN(skill_table[gsn_search]->beats / 10, 3),
		       do_search, 1 );
	    send_to_char( "You begin your search...\n\r", ch );
	    ch->alloc_ptr = str_dup( arg );
	    return;

	case 1:
	    if ( !ch->alloc_ptr )
	    {
		send_to_char( "Your search was interrupted!\n\r", ch );
		bug( "do_search: alloc_ptr NULL", 0 );
		return;
	    }
	    strcpy( arg, ch->alloc_ptr );
	    DISPOSE( ch->alloc_ptr );
	    break;
	case SUB_TIMER_DO_ABORT:
	    DISPOSE( ch->alloc_ptr );
	    ch->substate = SUB_NONE;
	    send_to_char( "You stop your search...\n\r", ch );
	    return;
    }
    ch->substate = SUB_NONE;
    if ( arg[0] == '\0' )
	startobj = ch->in_room->first_content;
    else
    {
	if ( (door = get_door( arg )) != -1 )
	  startobj = NULL;
	else
	{
	    container = get_obj_here( ch, arg );
	    if ( !container )
	    {
		send_to_char( "You can't find that here.\n\r", ch );
		return;
	    }
	    startobj = container->first_content;
	}
    }

    if ( (!startobj && door == -1) || IS_NPC(ch) )
    {
	send_to_char( "You find nothing.\n\r", ch );
	learn_from_failure( ch, gsn_search );
	return;
    }

    percent  = number_percent( ) + number_percent( ) - ( ch->level / 10 );

    if ( door != -1 )
    {
	EXIT_DATA *pexit;
	
	if ( (pexit = get_exit( ch->in_room, door )) != NULL
	&&   IS_SET( pexit->exit_info, EX_SECRET )
	&&   IS_SET( pexit->exit_info, EX_xSEARCHABLE )
        &&   can_use_skill( ch, percent, gsn_search ) )
	{
	    act( AT_SKILL, "Your search reveals the $d!", ch, NULL, pexit->keyword, TO_CHAR );
	    act( AT_SKILL, "$n finds the $d!", ch, NULL, pexit->keyword, TO_ROOM );
	    REMOVE_BIT( pexit->exit_info, EX_SECRET );
	    learn_from_success( ch, gsn_search );
	    return;
	}
    }
    else
    for ( obj = startobj; obj; obj = obj->next_content )
    {
       if ( IS_OBJ_STAT( obj, ITEM_HIDDEN )
            && can_use_skill(ch, percent, gsn_search ) )
       {
	    separate_obj(obj);
	    xREMOVE_BIT( obj->extra_flags, ITEM_HIDDEN );
	    act( AT_SKILL, "Your search reveals $p!", ch, obj, NULL, TO_CHAR );
	    act( AT_SKILL, "$n finds $p!", ch, obj, NULL, TO_ROOM );
	    learn_from_success( ch, gsn_search );
	    return;
       }
    }

    send_to_char( "You find nothing.\n\r", ch );
    learn_from_failure( ch, gsn_search );
    return;
}


void do_steal( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim, *mst;
    OBJ_DATA *obj;
    int percent;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ch->mount )
    {
	send_to_char( "You can't do that while mounted.\n\r", ch );
	return;
    }

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Steal what from whom?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "That's pointless.\n\r", ch );
	return;
    }

    if ( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "A magical force interrupts you.\n\r", ch );
	return;
    }


/* Disabled stealing among players because of complaints naked avatars were 
   running around stealing eq from equipped pkillers. -- Narn
*/
/*    if ( check_illegal_psteal( ch, victim ) )
    {
	send_to_char( "You can't steal from that player.\n\r", ch );
	return;
    }
*/

    if ( !IS_NPC( ch ) && !IS_NPC( victim ) )
    {
	set_char_color( AT_IMMORT, ch );
	send_to_char( "The gods forbid theft between players.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_PACIFIST)  ) /* Gorog */
    {
        send_to_char( "They are a pacifist - Shame on you!\n\r", ch);
        return;
    }


    WAIT_STATE( ch, skill_table[gsn_steal]->beats );
    percent  = number_percent( ) + ( IS_AWAKE(victim) ? 10 : -50 )
	       - (get_curr_lck(ch) - 15) + (get_curr_lck(victim) - 13);

    /* Changed the level check, made it 10 levels instead of five and made the 
       victim not attack in the case of a too high level difference.  This is 
       to allow mobprogs where the mob steals eq without having to put level 
       checks into the progs.  Also gave the mobs a 10% chance of failure.
    */
    if( ch->level + 10 < victim->level )
    {
	send_to_char( "You really don't want to try that!\n\r", ch );
        return;
    } 

    if ( victim->position == POS_FIGHTING
      || !can_use_skill( ch, percent, gsn_steal )
      || (IS_NPC(victim) && xIS_SET(victim->act, ACT_NOSTEAL) ) )
    {
	/*
	 * Failure.
	 */
	send_to_char( "Oops...\n\r", ch );
	act( AT_ACTION, "$n tried to steal from you!\n\r", ch, NULL, victim, TO_VICT    );
	act( AT_ACTION, "$n tried to steal from $N.\n\r",  ch, NULL, victim, TO_NOTVICT );

	sprintf( buf, "%s is a bloody thief!", ch->name );
	do_yell( victim, buf );

	learn_from_failure( ch, gsn_steal );
	if ( !IS_NPC(ch) )
	{
	    if ( legal_loot( ch, victim ) )
	    {
		global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
	    }
	    else
	    {
		/* log_string( buf ); */
		if ( IS_NPC( ch ) )
		{
		  if ( (mst = ch->master) == NULL )
		    return;
		}
		else
		  mst = ch;
		if ( IS_NPC( mst ) )
		  return;
		if ( !xIS_SET(mst->act, PLR_THIEF) )
		{
		    xSET_BIT(mst->act, PLR_THIEF);
		    set_char_color( AT_WHITE, ch );
		    send_to_char( "A strange feeling grows deep inside you, and a tingle goes up your spine...\n\r", ch );
		    set_char_color( AT_IMMORT, ch );
		    send_to_char( "A deep voice booms inside your head, 'Thou shall now be known as a lowly thief!'\n\r", ch );
		    set_char_color( AT_WHITE, ch );
		    send_to_char( "You feel as if your soul has been revealed for all to see.\n\r", ch );
		    save_char_obj( mst );
		}
	    }
	}

	return;
    }

    if ( !str_cmp( arg1, "coin"  )
    ||   !str_cmp( arg1, "coins" )
    ||   !str_cmp( arg1, "gold"  ) )
    {
	int amount;

	amount = (int) (victim->gold * number_range(1, 10) / 100);
	if ( amount <= 0 )
	{
	    send_to_char( "You couldn't get any gold.\n\r", ch );
	    learn_from_failure( ch, gsn_steal );
	    return;
	}

	ch->gold     += amount;
	victim->gold -= amount;
	ch_printf( ch, "Aha!  You got %d gold coins.\n\r", amount );
	learn_from_success( ch, gsn_steal );
	return;
    }

    if ( ( obj = get_obj_carry( victim, arg1 ) ) == NULL )
    {
	send_to_char( "You can't seem to find it.\n\r", ch );
	learn_from_failure( ch, gsn_steal );
	return;
    }

    if ( !can_drop_obj( ch, obj )
    ||   IS_OBJ_STAT(obj, ITEM_INVENTORY)
    ||	 IS_OBJ_STAT(obj, ITEM_PROTOTYPE)
    ||   obj->level > ch->level )
    {
	send_to_char( "You can't manage to pry it away.\n\r", ch );
	learn_from_failure( ch, gsn_steal );
	return;
    }

    if ( ch->carry_number + (get_obj_number(obj)/obj->count) > can_carry_n( ch ) )
    {
	send_to_char( "You have your hands full.\n\r", ch );
	learn_from_failure( ch, gsn_steal );
	return;
    }

    if ( ch->carry_weight + (get_obj_weight(obj)/obj->count) > can_carry_w( ch ) )
    {
	send_to_char( "You can't carry that much weight.\n\r", ch );
	learn_from_failure( ch, gsn_steal );
	return;
    }

    separate_obj( obj );
    obj_from_char( obj );
    obj_to_char( obj, ch );
    send_to_char( "Ok.\n\r", ch );
    learn_from_success( ch, gsn_steal );
    adjust_favor( ch, 9, 1 );
    return;
}


void do_pounce( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent;

    if ( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't do that right now.\n\r", ch );
	return;
    }

    one_argument( argument, arg );

    if ( ch->mount )
    {
	send_to_char( "You can't get close enough while mounted.\n\r", ch );
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Pounce on whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Pounce on yourself?\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim, TRUE ) )
      return;

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
    ||   ( obj->value[3] !=  1
        && obj->value[3] !=  2
        && obj->value[3] !=  3
	&& obj->value[3] !=  5
        && obj->value[3] != 10
        && obj->value[3] != 11 ) )
    {
	send_to_char( "You are not wielding an appropriate weapon type to effectively pounce.\n\r", ch );
	return;
    }

    if ( victim->fighting )
    {
	send_to_char( "You cannot pounce on someone who is in combat.\n\r", ch );
	return;
    }

    if ( victim->hit < victim->max_hit && IS_AWAKE( victim ) )
    {
	act( AT_PLAIN, "$N is hurt and suspicious ... you can't sneak up.", ch, NULL, victim, TO_CHAR );
	return;
    }

    percent = number_percent( ) - ( get_curr_lck( ch ) - 14 )
	      + ( get_curr_lck( victim ) - 13 );

    check_attacker( ch, victim );

    WAIT_STATE( ch, skill_table[gsn_pounce]->beats );

    if ( !IS_AWAKE( victim )
    ||    can_use_skill( ch, percent, gsn_pounce ) )
    {
	learn_from_success( ch, gsn_pounce );
	global_retcode = multi_hit( ch, victim, gsn_pounce );
        check_illegal_pk( ch, victim );
    }
    else
    {
	learn_from_failure( ch, gsn_pounce );
	global_retcode = damage( ch, victim, 0, gsn_pounce );
        check_illegal_pk( ch, victim );
    }
    return;
}

void do_backstab( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't do that right now.\n\r", ch );
	return;
    }

    one_argument( argument, arg );

    if ( ch->mount )
    {
	send_to_char( "You can't get close enough while mounted.\n\r", ch );
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Backstab whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you sneak up on yourself?\n\r", ch );
	return;
    }

    if ( xIS_SET(ch->act, PLR_NICE ) && !IS_NPC( victim ) )
    {
	send_to_char( "You feel too nice to do that!\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim, TRUE ) )
      return;


    /* Added stabbing weapon. -Narn */
    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
    ||   ( obj->value[3] != 11 && obj->value[3] != 2 ) )
    {
	send_to_char( "You need to wield a piercing or stabbing weapon.\n\r", ch );
	return;
    }

    if ( victim->fighting )
    {
	send_to_char( "You can't backstab someone who is in combat.\n\r", ch );
	return;
    }

    /* Can backstab a char even if it's hurt as long as it's sleeping. -Narn */
    if ( victim->hit < victim->max_hit && IS_AWAKE( victim ) )
    {
    act( AT_PLAIN, "$N is hurt and suspicious ... you can't sneak up.",
	    ch, NULL, victim, TO_CHAR );
	return;
    }

    percent = number_percent( ) - (get_curr_lck(ch) - 14) 
	      + (get_curr_lck(victim) - 13);

    check_attacker( ch, victim );
    WAIT_STATE( ch, skill_table[gsn_backstab]->beats );
    if ( !IS_AWAKE(victim)
    || can_use_skill( ch, percent, gsn_backstab ) )
    {
	learn_from_success( ch, gsn_backstab );
	global_retcode = multi_hit( ch, victim, gsn_backstab );
	adjust_favor( ch, 10, 1 );
        check_illegal_pk( ch, victim );

    }
    else
    {
	learn_from_failure( ch, gsn_backstab );
	global_retcode = damage( ch, victim, 0, gsn_backstab );
        check_illegal_pk( ch, victim );
    }
    return;
}


void do_rescue( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *fch;
    int percent;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_BERSERK ) )
    {
	send_to_char( "You aren't thinking clearly...\n\r", ch );
	return;
    }

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Rescue whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "How about fleeing instead?\n\r", ch );
	return;
    }

    if ( ch->mount )
    {
	send_to_char( "You can't do that while mounted.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
	send_to_char( "They don't need your help!\n\r", ch );
	return;
    }

    if ( !ch->fighting )
    {
	send_to_char( "Too late...\n\r", ch );
	return;
    }

    if ( ( fch = who_fighting( victim) ) == NULL )
    {
	send_to_char( "They are not fighting right now.\n\r", ch );
	return;
    }

    if ( who_fighting( victim ) == ch )
    {
	send_to_char( "Just running away would be better...\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( victim, AFF_BERSERK ) )
    {
	send_to_char( "Stepping in front of a berserker would not be an intelligent decision.\n\r", ch );
	return;
    }

    percent = number_percent( ) - (get_curr_lck(ch) - 14) 
	      - (get_curr_lck(victim) - 16);

    WAIT_STATE( ch, skill_table[gsn_rescue]->beats );
    if (!can_use_skill( ch, percent, gsn_rescue ) )
    {
	send_to_char( "You fail the rescue.\n\r", ch );
	act( AT_SKILL, "$n tries to rescue you!", ch, NULL, victim, TO_VICT   );
	act( AT_SKILL, "$n tries to rescue $N!", ch, NULL, victim, TO_NOTVICT );
	learn_from_failure( ch, gsn_rescue );
	return;
    }

    act( AT_SKILL, "You rescue $N!",  ch, NULL, victim, TO_CHAR    );
    act( AT_SKILL, "$n rescues you!", ch, NULL, victim, TO_VICT    );
    act( AT_SKILL, "$n moves in front of $N!",  ch, NULL, victim, TO_NOTVICT );

    learn_from_success( ch, gsn_rescue );
    adjust_favor( ch, 8, 1 );  
    stop_fighting( fch, FALSE );
    stop_fighting( victim, FALSE );
    if ( ch->fighting )
      stop_fighting( ch, FALSE );

    set_fighting( ch, fch );
    set_fighting( fch, ch );
    return;
}

void do_meditate( CHAR_DATA *ch, char *argument )
{
    char *arg;
    int percent;
    int managain = (ch->class == CLASS_DRUID ? 0 : 22);

    switch( ch->substate )
    {
    default:
      if ( IS_NPC(ch) ||  IS_AFFECTED( ch, AFF_CHARM ) )
      {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
      }
      send_to_char_color( "&BYou enter into a meditative state, hoping to collect mana from the cosmos.\n\r", ch );
      add_timer( ch, TIMER_DO_FUN, UMAX(2,(skill_table[gsn_meditate]->beats/8)), do_meditate, 1);
      ch->alloc_ptr = str_dup( argument );
      return;

    case 1:
      if ( !ch->alloc_ptr )
      {
          send_to_char_color( "&BYour meditation was interrupted.\n\r", ch );
          bug( "do_meditate: alloc_ptr NULL or not numeric", 0 );
          return;
      }
      arg = str_dup( ch->alloc_ptr );
      DISPOSE( ch->alloc_ptr );
      break;

    case SUB_TIMER_DO_ABORT:
      ch->substate = SUB_NONE;
      send_to_char_color(  "&BYou stop meditating.\n\r", ch );
      DISPOSE( ch->alloc_ptr );
      return;
    }

	switch ( ch->in_room->sector_type )
	{
	case    SECT_INSIDE:
	case	SECT_DUNNO:
	case 	SECT_UNDERGROUND:
	default:
		break;
	case SECT_FIELD:
	case SECT_HILLS:
	case SECT_FOREST:
	case SECT_MOUNTAIN:
		if (ch->class == CLASS_DRUID)
			managain=24;
		else
			managain +=2;
		break;
	case SECT_WATER_SWIM:
	case SECT_WATER_NOSWIM:
	case SECT_UNDERWATER:
	case SECT_OCEANFLOOR:
	    if ( ch->race == RACE_SEA_ELF )
		   managain+=3;
	    else
		  if (!IS_AFFECTED( ch, AFF_AQUA_BREATH ))
		    managain-=2;
		break;
	case SECT_AIR:
		if ( ch->race == RACE_PIXIE )
			managain+=3;
		else
			if ( !IS_AFFECTED( ch, AFF_FLYING ) )			
		    managain-=2;
		break;
	case	SECT_DESERT:
	case	SECT_LAVA:
	case	SECT_SWAMP:
		managain-=2;
		break;
	}

    percent = UMAX(1, number_percent() + get_curr_wis(ch) - 10);
    if ( can_use_skill(ch, percent ,gsn_meditate ) )
    {
      learn_from_success( ch, gsn_meditate );
      send_to_char_color( "&BYou meditate peacefully, collecting mana from the cosmos.\n\r", ch );
      ch->mana = UMIN(ch->max_mana, ch->mana+managain);
    }
    else
	{
      send_to_char_color( "&BYou spend several minutes in deep concentration, but fail to collect any mana.\n\r", ch );
      learn_from_failure( ch, gsn_meditate );
    }
        if ( ch->mana<ch->max_mana)
        {
           TIMER *timer;
           timer = get_timerptr( ch, TIMER_DO_FUN );
           timer->count += UMAX(2,(skill_table[gsn_meditate]->beats/8));
           ch->alloc_ptr = str_dup( arg );
        }
        else
        {
           send_to_char_color(  "&BYou complete your meditations.\n\r", ch );
           ch->substate = SUB_NONE;
        }
    return;
}

void do_trance( CHAR_DATA *ch, char *argument )
{
    char *arg;
    int percent;
    int managain= (ch->class == CLASS_DRUID ? 0 : 50);

    switch( ch->substate )
    {
    default:
      if ( IS_NPC(ch) ||  IS_AFFECTED( ch, AFF_CHARM ) )
      {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
      }
      send_to_char_color( "&BYou enter a peaceful trance, collecting mana from the cosmos.\n\r", ch );
      add_timer( ch, TIMER_DO_FUN, UMAX(2,(skill_table[gsn_trance]->beats/8)), do_trance, 1);
      ch->alloc_ptr = str_dup( argument );
      return;

    case 1:
      if ( !ch->alloc_ptr )
      {
          send_to_char_color( "&BYour trance was interrupted.\n\r", ch );
          bug( "do_trance: alloc_ptr NULL or not numeric", 0 );
          return;
      }
      arg = str_dup ( ch->alloc_ptr );
      DISPOSE( ch->alloc_ptr );
      break;

    case SUB_TIMER_DO_ABORT:
      ch->substate = SUB_NONE;
      send_to_char_color(  "&BYou come out of your trance.\n\r", ch );
      DISPOSE( ch->alloc_ptr );
      return;
    }

	switch ( ch->in_room->sector_type )
	{
	case	SECT_DUNNO:
	case 	SECT_UNDERGROUND:
	default:
		break;
	case SECT_INSIDE:
	case SECT_FIELD:
	case SECT_HILLS:
	case SECT_FOREST:
	case SECT_MOUNTAIN:
		if ( ch->class == CLASS_DRUID )
			managain = 50;
		else
			managain +=2;
		break;
	case SECT_WATER_SWIM:
	case SECT_WATER_NOSWIM:
	case SECT_UNDERWATER:
	case SECT_OCEANFLOOR:
	    if ( ch->race == RACE_SEA_ELF )
		   managain+=3;
	    else
		  if (!IS_AFFECTED( ch, AFF_AQUA_BREATH ))
		    managain-=2;
		break;
	case SECT_AIR:
	    if ( ch->race == RACE_PIXIE )
			managain+=3;
		else
			if ( !IS_AFFECTED( ch, AFF_FLYING ) )			
		    managain-=2;
		break;
	case	SECT_DESERT:
	case	SECT_LAVA:
	case	SECT_SWAMP:
		managain-=2;
		break;
	}


    percent = UMAX(1, number_percent() + get_curr_wis(ch) - 10);
    if ( can_use_skill(ch, percent, gsn_trance ) )
    {
      learn_from_success( ch, gsn_trance );
      send_to_char_color( "&BIn your peaceful trance, you collect mana from the cosmos.\n\r", ch );
      ch->mana = UMIN(ch->max_mana, ch->mana+managain);
    }
    else
	{
      send_to_char_color( "&BYou spend several minutes in a deep trance, but fail to collect any mana.\n\r", ch );
      learn_from_failure( ch, gsn_trance );
	}
        if ( ch->mana<ch->max_mana)
        {
           TIMER *timer;
           timer = get_timerptr( ch, TIMER_DO_FUN );
           timer->count += UMAX(2,(skill_table[gsn_trance]->beats/8));
           ch->alloc_ptr = str_dup( arg );
        }
        else
        {
           send_to_char_color( "&BYou complete your trance.\n\r", ch );
           ch->substate = SUB_NONE;
        }
    return;
}


void do_kick( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_kick]->skill_level[ch->class] )
    {
	send_to_char(
	    "You better leave the martial arts to fighters.\n\r", ch );
	return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_kick]->beats );
    if ( can_use_skill(ch, number_percent(),gsn_kick ) )
    {
	learn_from_success( ch, gsn_kick );
	global_retcode = damage( ch, victim, number_range( 1, ch->level ), gsn_kick );
    }
    else
    {
	learn_from_failure( ch, gsn_kick );
	global_retcode = damage( ch, victim, 0, gsn_kick );
    }
    return;
}

void do_punch( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_punch]->skill_level[ch->class] )
    {
	send_to_char(
	    "You better leave the martial arts to fighters.\n\r", ch );
	return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_punch]->beats );
    if ( can_use_skill( ch, number_percent(), gsn_punch ) )
    {
	learn_from_success( ch, gsn_punch );
	global_retcode = damage( ch, victim, number_range( 1, ch->level ), gsn_punch );
    }
    else
    {
	learn_from_failure( ch, gsn_punch );
	global_retcode = damage( ch, victim, 0, gsn_punch );
    }
    return;
}


void do_bite( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_bite]->skill_level[ch->class] )
    {
	send_to_char(
	    "That isn't quite one of your natural skills.\n\r", ch );
	return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_bite]->beats );
    if ( can_use_skill( ch, number_percent(), gsn_bite ) )
    {
	learn_from_success( ch, gsn_bite );
	global_retcode = damage( ch, victim, number_range( 1, ch->level ), gsn_bite );
    }
    else
    {
	learn_from_failure( ch, gsn_bite );
	global_retcode = damage( ch, victim, 0, gsn_bite );
    }
    return;
}


void do_claw( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_claw]->skill_level[ch->class] )
    {
	send_to_char(
	    "That isn't quite one of your natural skills.\n\r", ch );
	return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_claw]->beats );
    if ( can_use_skill( ch, number_percent(), gsn_claw ) )
    {
	learn_from_success( ch, gsn_claw );
	global_retcode = damage( ch, victim, number_range( 1, ch->level ), gsn_claw );
    }
    else
    {
	learn_from_failure( ch, gsn_claw );
	global_retcode = damage( ch, victim, 0, gsn_claw );
    }
    return;
}



void do_sting( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_sting]->skill_level[ch->class] )
    {
	send_to_char(
	    "That isn't quite one of your natural skills.\n\r", ch );
	return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_sting]->beats );
    if ( can_use_skill( ch, number_percent(), gsn_sting ) )
    {
	learn_from_success( ch, gsn_sting );
	global_retcode = damage( ch, victim, number_range( 1, ch->level ), gsn_sting );
    }
    else
    {
	learn_from_failure( ch, gsn_sting );
	global_retcode = damage( ch, victim, 0, gsn_sting );
    }
    return;
}


void do_tail( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_tail]->skill_level[ch->class] )
    {
	send_to_char(
	    "That isn't quite one of your natural skills.\n\r", ch );
	return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_tail]->beats );
  if ( can_use_skill( ch, number_percent(), gsn_tail ) )
    {
	learn_from_success( ch, gsn_tail );
	global_retcode = damage( ch, victim, number_range( 1, ch->level ), gsn_tail );
    }
    else
    {
	learn_from_failure( ch, gsn_tail );
	global_retcode = damage( ch, victim, 0, gsn_tail );
    }
    return;
}


void do_bash( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int chance;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_bash]->skill_level[ch->class] )
    {
	send_to_char(
	    "You better leave the martial arts to fighters.\n\r", ch );
	return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

   chance = (((get_curr_dex(victim) + get_curr_str(victim))
	   -  (get_curr_dex(ch)     + get_curr_str(ch))) * 10) + 10;
    if ( !IS_NPC(ch) && !IS_NPC(victim) )
      chance += sysdata.bash_plr_vs_plr;
    if ( victim->fighting && victim->fighting->who != ch )
      chance += sysdata.bash_nontank;
    WAIT_STATE( ch, skill_table[gsn_bash]->beats );
  if ( can_use_skill(ch, (number_percent () + chance), gsn_bash ) )
    {
	learn_from_success( ch, gsn_bash );
	/* do not change anything here!  -Thoric */
	WAIT_STATE( victim, 2 * PULSE_VIOLENCE );
	victim->position = POS_SITTING;
	global_retcode = damage( ch, victim, number_range( 1, ch->level ), gsn_bash );
    }
    else
    {
	learn_from_failure( ch, gsn_bash );
	global_retcode = damage( ch, victim, 0, gsn_bash );
    }
    return;
}


void do_stun( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    int chance;
    bool fail;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_stun]->skill_level[ch->class] )
    {
	send_to_char(
	    "You better leave the martial arts to fighters.\n\r", ch );
	return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) && ch->move < ch->max_move/10 )
    {
	set_char_color( AT_SKILL, ch );
	send_to_char( "You are far too tired to do that.\n\r", ch );
	return;		/* missing return fixed March 11/96 */
    }

    WAIT_STATE( ch, skill_table[gsn_stun]->beats );
    fail = FALSE;
    chance = ris_save( victim, ch->level, RIS_PARALYSIS );
    if ( chance == 1000 )
      fail = TRUE;
    else
      fail = saves_para_petri( chance, victim );

    chance = (((get_curr_dex(victim) + get_curr_str(victim))
	   -   (get_curr_dex(ch)     + get_curr_str(ch))) * 10) + 10;
    /* harder for player to stun another player */
    if ( !IS_NPC(ch) && !IS_NPC(victim) )
      chance += sysdata.stun_plr_vs_plr;
    else
      chance += sysdata.stun_regular;
  if (!fail && can_use_skill(ch, (number_percent () + chance), gsn_stun ) )
    {
	learn_from_success( ch, gsn_stun );
	/*    DO *NOT* CHANGE!    -Thoric    */
	if ( !IS_NPC(ch) )
  	  ch->move -= ch->max_move/10;
	WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
	WAIT_STATE( victim, PULSE_VIOLENCE );
	act( AT_SKILL, "$N smashes into you, leaving you stunned!", victim, NULL, ch, TO_CHAR );
	act( AT_SKILL, "You smash into $N, leaving $M stunned!", ch, NULL, victim, TO_CHAR );
	act( AT_SKILL, "$n smashes into $N, leaving $M stunned!", ch, NULL, victim, TO_NOTVICT );
	if ( !IS_AFFECTED( victim, AFF_PARALYSIS ) )
	{
	  af.type      = gsn_stun;
	  af.location  = APPLY_AC;
	  af.modifier  = 20;
	  af.duration  = 3;
	  af.bitvector = meb(AFF_PARALYSIS);
	  affect_to_char( victim, &af );
	  update_pos( victim );
	}
    }
    else
    {
	WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
	if ( !IS_NPC(ch) )
	  ch->move -= ch->max_move/15;
	learn_from_failure( ch, gsn_stun );
	act( AT_SKILL, "$n charges at you screaming, but you dodge out of the way.", ch, NULL, victim, TO_VICT );
	act( AT_SKILL, "You try to stun $N, but $E dodges out of the way.", ch, NULL, victim, TO_CHAR );
	act( AT_SKILL, "$n charges screaming at $N, but keeps going right on past.", ch, NULL, victim, TO_NOTVICT );
    }
    return;
}

void do_bloodlet( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA  *obj;
 
    if ( IS_NPC(ch) || !IS_VAMPIRE(ch) )
	return;
 
    if ( ch->fighting )
    {
	send_to_char( "You're too busy fighting...\n\r", ch );
	return;
    }
    if ( ch->pcdata->condition[COND_BLOODTHIRST] < 10 )
    {
	send_to_char( "You are too drained to offer any blood...\n\r", ch );
	return;
    }
 
    WAIT_STATE( ch, PULSE_VIOLENCE );
    if ( can_use_skill(ch, number_percent(), gsn_bloodlet ) )
    {
	gain_condition( ch, COND_BLOODTHIRST, -7 );
	act( AT_BLOOD, "Tracing a sharp nail over your skin, you let your blood spill.", ch, NULL, NULL, TO_CHAR );
	act( AT_BLOOD, "$n traces a sharp nail over $s skin, spilling a quantity of blood to the ground.", ch, NULL, NULL, TO_ROOM );
	learn_from_success( ch, gsn_bloodlet );
	obj           = create_object( get_obj_index( OBJ_VNUM_BLOODLET ), 0 );
	obj->timer    = 1;
	obj->value[1] = 6;
	obj_to_room( obj, ch->in_room );
	damage( ch, ch, ch->level/5, gsn_bloodlet );
    }
    else
    {
	act( AT_BLOOD, "You cannot manage to draw much blood...", ch, NULL, NULL, TO_CHAR );
	act( AT_BLOOD, "$n slices open $s skin, but no blood is spilled...", ch, NULL, NULL, TO_ROOM );
	learn_from_failure( ch, gsn_bloodlet );
    }
    return;
}

void do_feed( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	sh_int dam;
	sh_int temphit;

	if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
	{
	  send_to_char( "You can't concentrate enough for that.\n\r", ch );
	  return;
	}

	if ( !IS_NPC(ch)
	&&   !IS_VAMPIRE(ch) )
	{
	  send_to_char( "It is not of your nature to feed on living creatures.\n\r", ch );
	  return;
	}
        if ( !can_use_skill(ch, 0, gsn_feed ) )
	{
	  send_to_char( "You have not yet practiced your new teeth.\n\r", ch );
	  return;
	}

	if ( ( victim = who_fighting( ch ) ) == NULL )
	{
	  send_to_char( "You aren't fighting anyone.\n\r", ch );
	  return;
	}

	if ( ch->mount )
	{
	  send_to_char( "You can't do that while mounted.\n\r", ch );
	  return;
	}

	WAIT_STATE( ch, skill_table[gsn_feed]->beats );
        if ( can_use_skill( ch, number_percent(), gsn_feed ) )
	{
	  dam = number_range( 1, ch->level );
	  global_retcode = damage( ch, victim, dam, gsn_feed );
	  if ( global_retcode == rNONE && !IS_NPC(ch) && dam
	  &&  ch->fighting
	  &&  ch->pcdata->condition[COND_BLOODTHIRST] < (10 + ch->level) )
	  {
	    gain_condition( ch, COND_BLOODTHIRST,
	     UMIN( number_range(1, (ch->level+victim->level / 20) + 3 ),
	     (10 + ch->level) - ch->pcdata->condition[COND_BLOODTHIRST] ) );
            if ( ch->pcdata->condition[COND_FULL] <= 37 )
	      gain_condition( ch, COND_FULL, 2);
	    gain_condition( ch, COND_THIRST, 2);
	    act( AT_BLOOD, "You manage to suck a little life out of $N.", ch, NULL,
		 victim, TO_CHAR );
	    act( AT_BLOOD, "$n sucks some of your blood!", ch, NULL, victim, TO_VICT );
	    learn_from_success( ch, gsn_feed );
	    temphit = ch->hit;
            ch->hit += 1 + ch->level / 5;
            if ( ch->hit > ch->max_hit )
                ch->hit = ch->max_hit;
            if ( ch->hit < temphit )
                ch->hit = temphit;
	  }
	}
	else
	{
	  global_retcode = damage( ch, victim, 0, gsn_feed );
	  if ( global_retcode == rNONE && !IS_NPC(ch)
	  &&  ch->fighting
	  &&  ch->pcdata->condition[COND_BLOODTHIRST] < (10 + ch->level) )
	  {
	    act( AT_BLOOD, "The smell of $N's blood is driving you insane!",
		ch, NULL, victim, TO_CHAR );
	    act( AT_BLOOD, "$n is lusting after your blood!", ch, NULL, victim, TO_VICT );
	    learn_from_failure( ch, gsn_feed );
	  }
	}
	return;
}


/*
 * Disarm a creature.
 * Caller must check for successful attack.
 * Check for loyalty flag (weapon disarms to inventory) for pkillers -Blodkai
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{

    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj, *tmpobj;

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
	return;

    if ( ( tmpobj = get_eq_char( victim, WEAR_DUAL_WIELD ) ) != NULL
    &&     number_bits( 1 ) == 0 )
	obj = tmpobj;

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL && number_bits( 1 ) == 0 )
    {
	learn_from_failure( ch, gsn_disarm );
	return;
    }

    if ( IS_NPC( ch ) && !can_see_obj( ch, obj ) && number_bits( 1 ) == 0)
    {
	learn_from_failure( ch, gsn_disarm );
	return;
    }
 
    if ( check_grip( ch, victim ) )
    {
	learn_from_failure( ch, gsn_disarm );
	return;
    }

    act( AT_SKILL, "$n DISARMS you!", ch, NULL, victim, TO_VICT    );
    act( AT_SKILL, "You disarm $N!",  ch, NULL, victim, TO_CHAR    );
    act( AT_SKILL, "$n disarms $N!",  ch, NULL, victim, TO_NOTVICT );
    learn_from_success( ch, gsn_disarm );

    if ( obj == get_eq_char( victim, WEAR_WIELD )
    &&  (tmpobj = get_eq_char( victim, WEAR_DUAL_WIELD)) != NULL )
       tmpobj->wear_loc = WEAR_WIELD;

    obj_from_char( obj );
    if ( !IS_NPC( victim ) && CAN_PKILL( victim ) )
    /* && !IS_OBJ_STAT( obj, ITEM_LOYAL ) ) -- why is this here? */
    {
	SET_BIT( obj->magic_flags, ITEM_PKDISARMED );
	sprintf( buf, "%s %s", ch->name, victim->name );
	STRFREE( obj->action_desc );
	obj->action_desc = STRALLOC( buf );
    }
    if ( IS_NPC(victim)
    || ( IS_OBJ_STAT(obj, ITEM_LOYAL) && IS_PKILL(victim) && !IS_NPC(ch) ) )
    {
	obj_to_char( obj, victim );
	STRFREE( obj->action_desc ); /* Rather do this kludgy stuff than try to test all circumstances */
	obj->action_desc = STRALLOC( "" );
    }
    else if ( in_arena( victim ) )
    {
	obj_to_char( obj, victim );
	STRFREE( obj->action_desc );
	obj->action_desc = STRALLOC( "" );
    }
    else
	obj_to_room( obj, victim->in_room );
    return;
}


void do_disarm( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_disarm]->skill_level[ch->class] )
    {
	send_to_char( "You don't know how to disarm opponents.\n\r", ch );
	return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL )
    {
	send_to_char( "You must wield a weapon to disarm.\n\r", ch );
	return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
    {
	send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_disarm]->beats );
    percent = number_percent( ) + victim->level - ch->level
	      - (get_curr_lck(ch) - 15) + (get_curr_lck(victim) - 15);
    if ( !can_see_obj( ch, obj ) )
      percent += 10;
    if ( can_use_skill(ch, (percent*3/2), gsn_disarm ) )
	disarm( ch, victim );
    else
    {
	send_to_char( "You failed.\n\r", ch );
	learn_from_failure( ch, gsn_disarm );
    }
    return;
}


/*
 * Trip a creature.
 * Caller must check for successful attack.
 */
void trip( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( IS_AFFECTED( victim, AFF_FLYING )
    ||   IS_AFFECTED( victim, AFF_FLOATING ) )
      return;
    if ( victim->mount )
    {
	if ( IS_AFFECTED( victim->mount, AFF_FLYING )
	||   IS_AFFECTED( victim->mount, AFF_FLOATING ) )
	  return;
	act( AT_SKILL, "$n trips your mount and you fall off!", ch, NULL, victim, TO_VICT    );
	act( AT_SKILL, "You trip $N's mount and $N falls off!", ch, NULL, victim, TO_CHAR    );
	act( AT_SKILL, "$n trips $N's mount and $N falls off!", ch, NULL, victim, TO_NOTVICT );
	xREMOVE_BIT( victim->mount->act, ACT_MOUNTED );
	victim->mount = NULL;
	WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
	WAIT_STATE( victim, 2 * PULSE_VIOLENCE );
	victim->position = POS_RESTING;
	return;		
    }
    if ( victim->wait == 0 )
    {
	act( AT_SKILL, "$n trips you and you go down!", ch, NULL, victim, TO_VICT    );
	act( AT_SKILL, "You trip $N and $N goes down!", ch, NULL, victim, TO_CHAR    );
	act( AT_SKILL, "$n trips $N and $N goes down!", ch, NULL, victim, TO_NOTVICT );

	WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
	WAIT_STATE( victim, 2 * PULSE_VIOLENCE );
	victim->position = POS_RESTING;
    }

    return;
}


/* Shargate, May 2002 */
void do_cleave( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int dam = 0;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "A clear mind is required to use that skill.\n\r", ch );
	return;
    }
    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_cleave]->skill_level[ch->class] )
    {
	send_to_char("You can't seem to summon the strength.\n\r", ch );
	return;
    }
    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
    ||   ( obj->value[3] != 1 && obj->value[3] != 3 && obj->value[3] != 5) )
    {
	send_to_char( "You need a slashing weapon.\n\r", ch );
	return;
    }
    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }
    if ( can_use_skill( ch, number_percent(), gsn_cleave ) )
    {
	WAIT_STATE( ch, skill_table[gsn_cleave]->beats );
    	if ( number_percent() <= 20)
	{
	   set_char_color( AT_WHITE, ch );
	   send_to_char( "You deal a devastating blow!\n\r", ch );
	   dam = ( number_range( 11, 22 ) * get_curr_str( ch ) )+30;
	   WAIT_STATE( victim, 2);
	}
	else
	{
	   dam = ( number_range( 9,18 ) * get_curr_str( ch ) )+30;
	}
	learn_from_success( ch, gsn_cleave );
	global_retcode = damage( ch, victim, dam, gsn_cleave );
    }
    else
    {
	learn_from_failure( ch, gsn_cleave );
	global_retcode = damage( ch, victim, 0, gsn_cleave );
    }
    return;
}


/* Converted to function well as a skill for vampires -- Blodkai */
void do_mistwalk( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    bool allowday;
 
    set_char_color( AT_DGREEN, ch );
    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) ) {
      send_to_char( "You can't do that right now.\n\r", ch );
      return;
    }
    if ( ch->mount ) {
        send_to_char( "And scare your mount to death?\n\r", ch );
        return;
    }
    one_argument( argument, arg );
    if ( arg[0] == '\0' ) {
        send_to_char( "Who will be your victim?\n\r", ch );
        return;
    }
    WAIT_STATE( ch, skill_table[gsn_mistwalk]->beats );
    if ( ( victim = get_char_world( ch, arg ) ) == NULL || victim == ch ) {
        send_to_char( "You are unable to sense your victim.\n\r", ch );
        return;
    }
    if ( IS_PKILL( ch ) )
      allowday = TRUE;
    else
      allowday = FALSE;
    if ( (time_info.hour < 21 && time_info.hour > 5 && !allowday )
    ||   !victim->in_room
    ||   xIS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_NO_ASTRAL)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_DEATH)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_PROTOTYPE)
    ||   xIS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= ch->level + 15
    ||  (IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE))
    ||  (IS_NPC(victim) && saves_spell_staff( ch->level, victim ))
    ||  (CAN_PKILL(victim) && !IS_NPC(ch) && !CAN_PKILL(ch))
    ||  !in_hard_range( ch, victim->in_room->area )
    ||  (IS_SET(victim->in_room->area->flags, AFLAG_NOPKILL) && IS_PKILL(ch)))
    {
        send_to_char( "You are unable to sense your victim.\n\r", ch );
        learn_from_failure( ch, gsn_mistwalk );
        return;
    }
    /* Subtract 22 extra bp for mist walk from 0500 to 2100 SB
    if  ( time_info.hour < 21 && time_info.hour > 5 && !IS_NPC(ch) )
      gain_condition( ch, COND_BLOODTHIRST, - 22 );
    */
    act( AT_DGREEN, "Your surroundings blur as you assume a form of churning mist!", ch, NULL, NULL, TO_CHAR );
    act( AT_DGREEN, "$n dissolves into a cloud of glowing mist, then vanishes!", ch, NULL, NULL, TO_ROOM );
    learn_from_success( ch, gsn_mistwalk );
    char_from_room( ch );
    char_to_room( ch, victim->in_room );
    act( AT_DGREEN, "A cloud of glowing mist engulfs you, then withdraws to unveil $n!", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    return;
}

void do_broach( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
 
    set_char_color(AT_DGREEN, ch);
 
    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) ) {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }
    one_argument( argument, arg );
    if ( arg[0] == '\0' ) {
        send_to_char( "Attempt this in which direction?\n\r", ch );
        return;
    }
    if ( ch->mount ) {
        send_to_char( "You should really dismount first.\n\r", ch );
        return;
    }
    WAIT_STATE( ch, skill_table[gsn_broach]->beats );
    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL ) {
        EXIT_DATA *pexit_rev;
        if ( !IS_SET(pexit->exit_info, EX_CLOSED)
        ||   !IS_SET(pexit->exit_info, EX_LOCKED)
        ||    IS_SET(pexit->exit_info, EX_PICKPROOF)
        || can_use_skill(ch, number_percent(), gsn_broach ) )
        {
            send_to_char( "Your attempt fails.\n\r", ch );
            learn_from_failure( ch, gsn_broach );
            check_room_for_traps( ch, TRAP_PICK | trap_door[pexit->vdir] );
            return;
        }
        REMOVE_BIT(pexit->exit_info, EX_LOCKED);
        send_to_char( "You successfully broach the exit...\n\r", ch );
        learn_from_success( ch, gsn_broach );
        adjust_favor( ch, 9, 1 );
        if ( ( pexit_rev = pexit->rexit ) != NULL && pexit_rev->to_room == ch->in_room )
        {
            REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
        }
        check_room_for_traps( ch, TRAP_PICK | trap_door[pexit->vdir] );
        return;
    }
    send_to_char( "Your attempt fails.\n\r", ch );
    return;
}

void do_pick( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    EXIT_DATA *pexit;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Pick what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( ch->mount )
    {
	send_to_char( "You can't do that while mounted.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_pick_lock]->beats );

    /* look for guards */
    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
	if ( IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level )
	{
	act( AT_PLAIN, "$N is standing too close to the lock.",
		ch, NULL, gch, TO_CHAR );
	    return;
	}
    }

  if ( !can_use_skill(ch, number_percent(), gsn_pick_lock ) )
    {
	send_to_char( "You failed.\n\r", ch);
	learn_from_failure( ch, gsn_pick_lock );
/*        for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
        {
          if ( IS_NPC(gch) && IS_AWAKE(gch) && xIS_SET(gch->act, ACT_GUARDIAN ) )
            multi_hit( gch, ch, TYPE_UNDEFINED );
        }
*/
	return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
	/* 'pick door' */
/*	ROOM_INDEX_DATA *to_room; */ /* Unused */
	EXIT_DATA *pexit_rev;

	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be picked.\n\r",     ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_PICKPROOF) )
	{
	   send_to_char( "You failed.\n\r", ch );
	   learn_from_failure( ch, gsn_pick_lock );
	   check_room_for_traps( ch, TRAP_PICK | trap_door[pexit->vdir] );
	   return;
	}

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( AT_ACTION, "$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	learn_from_success( ch, gsn_pick_lock );
	adjust_favor( ch, 9, 1 );
	/* pick the other side */
	if ( ( pexit_rev = pexit->rexit ) != NULL
	&&   pexit_rev->to_room == ch->in_room )
	{
	    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	}
	check_room_for_traps( ch, TRAP_PICK | trap_door[pexit->vdir] );
        return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'pick object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
	if ( IS_SET(obj->value[1], CONT_PICKPROOF) )
	{
	   send_to_char( "You failed.\n\r", ch );
	   learn_from_failure( ch, gsn_pick_lock );
	   check_for_trap( ch, obj, TRAP_PICK );
	   return;
	}

	separate_obj( obj );
	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( AT_ACTION, "$n picks $p.", ch, obj, NULL, TO_ROOM );
	learn_from_success( ch, gsn_pick_lock );
	adjust_favor( ch, 9, 1 );
	check_for_trap( ch, obj, TRAP_PICK );
	return;
    }

    ch_printf( ch, "You see no %s here.\n\r", arg );
    return;
}



void do_sneak( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    if ( ch->mount )
    {
	send_to_char( "You can't do that while mounted.\n\r", ch );
	return;
    }

    send_to_char( "You attempt to move silently.\n\r", ch );
    affect_strip( ch, gsn_sneak );

    if ( can_use_skill(ch, number_percent(), gsn_sneak ) )
    {
	af.type      = gsn_sneak;
	af.duration  = ch->level * DUR_CONV;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = meb(AFF_SNEAK);
	affect_to_char( ch, &af );
	learn_from_success( ch, gsn_sneak );
    }
    else
	learn_from_failure( ch, gsn_sneak );

    return;
}



void do_hide( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    if ( ch->mount )
    {
	send_to_char( "You can't do that while mounted.\n\r", ch );
	return;
    }

    send_to_char( "You attempt to hide.\n\r", ch );

    if ( IS_AFFECTED(ch, AFF_HIDE) )
	xREMOVE_BIT(ch->affected_by, AFF_HIDE);

    if ( can_use_skill(ch, number_percent(), gsn_hide ) )
    {
	xSET_BIT(ch->affected_by, AFF_HIDE);
	learn_from_success( ch, gsn_hide );
    }
    else
	learn_from_failure( ch, gsn_hide );
    return;
}



/*
 * Contributed by Alander.
 */
void do_visible( CHAR_DATA *ch, char *argument )
{
    affect_strip ( ch, gsn_invis			);
    affect_strip ( ch, gsn_mass_invis			);
    affect_strip ( ch, gsn_sneak			);
    xREMOVE_BIT  ( ch->affected_by, AFF_HIDE		);
    xREMOVE_BIT  ( ch->affected_by, AFF_INVISIBLE	);
    xREMOVE_BIT  ( ch->affected_by, AFF_SNEAK		);
    send_to_char( "Ok.\n\r", ch );
    return;
}


void do_recall( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *opponent;

    location = NULL;

    if ( !IS_NPC(ch) && ch->pcdata->clan )
      location = get_room_index( ch->pcdata->clan->recall );

    if ( !IS_NPC( ch ) && !location && ch->level >= 5
    &&   IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
       location = get_room_index( 3009 );

    /* 1998-01-02, h */
    if ( !location
    &&   !IS_NPC( ch ) ) /* Obscure crash bug */
       location = get_room_index(race_table[ch->race]->race_recall);

    if ( !location )
       location = get_room_index( ROOM_VNUM_TEMPLE ); 

    if ( !location )
    {
	send_to_char( "You are completely lost.\n\r", ch );
	return;
    }

    if ( ch->in_room == location )
	return;

    if ( xIS_SET(ch->in_room->room_flags, ROOM_NO_RECALL) )
    {
	send_to_char( "For some strange reason... nothing happens.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CURSE) )
    {
        send_to_char("You are cursed and cannot recall!\n\r", ch );
        return;
    }

    if ( ( opponent = who_fighting( ch ) ) != NULL )
    {
	int lose;

	if ( number_bits( 1 ) == 0 || ( !IS_NPC( opponent ) && number_bits( 3 ) > 1 ) )
	{
	    WAIT_STATE( ch, 4 );
	    lose = (exp_level(ch, ch->level+1) - exp_level(ch, ch->level)) * 0.1;
	    if ( ch->desc )
	      lose /= 2;
	    gain_exp( ch, 0 - lose );
	    ch_printf( ch, "You failed!  You lose %d exps.\n\r", lose );
	    return;
	}

	lose = (exp_level(ch, ch->level+1) - exp_level(ch, ch->level)) * 0.2;
	if ( ch->desc )
	  lose /= 2;
	gain_exp( ch, 0 - lose );
	ch_printf( ch, "You recall from combat!  You lose %d exps.\n\r", lose );
	stop_fighting( ch, TRUE );
    }

    act( AT_ACTION, "$n disappears in a swirl of smoke.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, location );
    if ( ch->mount )
    {
	char_from_room( ch->mount );
	char_to_room( ch->mount, location );
    }
    act( AT_ACTION, "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );

    return;
}


void do_aid( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int percent;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Aid whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )                        /* Gorog */
    {
	send_to_char( "Not on mobs.\n\r", ch );
	return;
    }

    if ( ch->mount )
    {
	send_to_char( "You can't do that while mounted.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Aid yourself?\n\r", ch );
	return;
    }

    if ( victim->position > POS_STUNNED )
    {
	act( AT_PLAIN, "$N doesn't need your help.", ch, NULL, victim,
	     TO_CHAR);
	return;
    }

    if ( victim->hit <= -6 )
    {
	act( AT_PLAIN, "$N's condition is beyond your aiding ability.", ch,
	     NULL, victim, TO_CHAR);
	return;
    }

    percent = number_percent( ) - (get_curr_lck(ch) - 13);
    WAIT_STATE( ch, skill_table[gsn_aid]->beats );
    if ( !can_use_skill( ch, percent, gsn_aid ) )
    {
	send_to_char( "You fail.\n\r", ch );
	learn_from_failure( ch, gsn_aid );
	return;
    }

    act( AT_SKILL, "You aid $N!",  ch, NULL, victim, TO_CHAR    );
    act( AT_SKILL, "$n aids $N!",  ch, NULL, victim, TO_NOTVICT );
    learn_from_success( ch, gsn_aid );
    adjust_favor( ch, 8, 1 );
    if ( victim->hit < 1 )
      victim->hit = 1;

    update_pos( victim );
    act( AT_SKILL, "$n aids you!", ch, NULL, victim, TO_VICT    );
    return;
}


void do_mount( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_mount]->skill_level[ch->class] )
    {
	send_to_char(
	    "I don't think that would be a good idea...\n\r", ch );
	return;
    }

    if ( ch->mount )
    {
	send_to_char( "You're already mounted!\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, argument ) ) == NULL )
    {
	send_to_char( "You can't find that here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) || !xIS_SET(victim->act, ACT_MOUNTABLE ) )
    {
	send_to_char( "You can't mount that!\n\r", ch );
	return;
    }

    if ( xIS_SET(victim->act, ACT_MOUNTED ) )
    {
	send_to_char( "That mount already has a rider.\n\r", ch );
	return;
    }

    if ( victim->position < POS_STANDING )
    {
	send_to_char( "Your mount must be standing.\n\r", ch );
	return;
    }

    if ( victim->position == POS_FIGHTING || victim->fighting )
    {
	send_to_char( "Your mount is moving around too much.\n\r", ch );
	return;
    }
    if (ch->stance > STANCE_NONE)
      do_stance (ch, "");
    WAIT_STATE( ch, skill_table[gsn_mount]->beats );
    if ( can_use_skill(ch, number_percent(), gsn_mount ) )
    {
	xSET_BIT(victim->act, ACT_MOUNTED );
	ch->mount = victim;
	act( AT_SKILL, "You mount $N.", ch, NULL, victim, TO_CHAR );
	act( AT_SKILL, "$n skillfully mounts $N.", ch, NULL, victim, TO_NOTVICT );
	act( AT_SKILL, "$n mounts you.", ch, NULL, victim, TO_VICT );
	learn_from_success( ch, gsn_mount );
	ch->position = POS_MOUNTED;
    }
    else
    {
	act( AT_SKILL, "You unsuccessfully try to mount $N.", ch, NULL, victim, TO_CHAR );
	act( AT_SKILL, "$n unsuccessfully attempts to mount $N.", ch, NULL, victim, TO_NOTVICT );
	act( AT_SKILL, "$n tries to mount you.", ch, NULL, victim, TO_VICT );
	learn_from_failure( ch, gsn_mount );
    }
    return;
}


void do_dismount( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( (victim = ch->mount) == NULL )
    {
	send_to_char( "You're not mounted.\n\r", ch );
	return;	
    }

    WAIT_STATE( ch, skill_table[gsn_mount]->beats );
   if ( can_use_skill(ch, number_percent(), gsn_mount ) )
    {
	act( AT_SKILL, "You dismount $N.", ch, NULL, victim, TO_CHAR );
	act( AT_SKILL, "$n skillfully dismounts $N.", ch, NULL, victim, TO_NOTVICT );
	act( AT_SKILL, "$n dismounts you.  Whew!", ch, NULL, victim, TO_VICT );
	xREMOVE_BIT( victim->act, ACT_MOUNTED );
	ch->mount = NULL;
	ch->position = POS_STANDING;
	learn_from_success( ch, gsn_mount );
    }
    else
    {
	act( AT_SKILL, "You fall off while dismounting $N.  Ouch!", ch, NULL, victim, TO_CHAR );
	act( AT_SKILL, "$n falls off of $N while dismounting.", ch, NULL, victim, TO_NOTVICT );
	act( AT_SKILL, "$n falls off your back.", ch, NULL, victim, TO_VICT );
	learn_from_failure( ch, gsn_mount );
	xREMOVE_BIT( victim->act, ACT_MOUNTED );
	ch->mount = NULL;
	ch->position = POS_SITTING;
	global_retcode = damage( ch, ch, 1, TYPE_UNDEFINED );
    }
    return;
}


/**************************************************************************/


/*
 * Check for parry.
 */
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chances;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( IS_NPC(victim) && !xIS_SET(victim->defenses, DFND_PARRY) )
      return FALSE;

  /* Put support in for bonus to parry.  If you have your stance 75% of the
   * way to GM, then you get the full parry benefit/penalty otherwise you only
   * get half of it.   SHADDAI
   */

    if ( IS_NPC(victim) )
    {
	chances	= UMIN( 60, 2 * victim->level );
      if (victim->pIndexData->stances[victim->stance]>(STANCE_GRAND_MASTER*.75))
        chances += stance_index[victim->stance].parry;
      else
        chances += (stance_index[victim->stance].parry / 2);
    }
    else
    {
	if ( get_eq_char(victim, WEAR_WIELD) == NULL )
	    return FALSE;
	chances	= (int) (LEARNED(victim, gsn_parry) / sysdata.parry_mod);
        if(victim->pcdata->stances[victim->stance]>(STANCE_GRAND_MASTER * .75))
           chances += stance_index[victim->stance].parry;
        else
           chances += (stance_index[victim->stance].parry / 2);
    }

    /* Put in the call to chance() to allow penalties for misaligned
       clannies.  */ 
    if ( chances != 0 && victim->morph )
	chances+=victim->morph->parry;

    if ( !chance( victim, chances + victim->level - ch->level ) )
    {
	learn_from_failure( victim, gsn_parry );
	return FALSE;
    }
    if ( !IS_NPC(victim)
         && !IS_SET( victim->pcdata->flags, PCFLAG_GAG) ) /*SB*/
    act( AT_SKILL, "You parry $n's attack.",  ch, NULL, victim, TO_VICT    );

    if ( !IS_NPC(ch)
         && !IS_SET( ch->pcdata->flags, PCFLAG_GAG) )  /* SB */   
    act( AT_SKILL, "$N parries your attack.", ch, NULL, victim, TO_CHAR    );

    learn_from_success( victim, gsn_parry );
    return TRUE;
}



/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chances;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( IS_NPC(victim) && !xIS_SET(victim->defenses, DFND_DODGE) )
      return FALSE;

    /* Support for stances have been added.  If you have your stance at least
     * half-way to Grand-Master you get the dodge benefit, other-wise you get
     * half of the benefit.     SHADDAI
     */
    if ( IS_NPC(victim) )
    {
      chances = UMIN (60, 2 * victim->level);
      if (victim->pIndexData->stances[victim->stance] > (STANCE_GRAND_MASTER/2))
        chances += stance_index[victim->stance].dodge;
      else
        chances += (stance_index[victim->stance].dodge / 2);
    }
    else
    {
        chances  = (int) (LEARNED(victim, gsn_dodge) / sysdata.dodge_mod );
        if (victim->pcdata->stances[ch->stance] > (STANCE_GRAND_MASTER / 2))
          chances += stance_index[victim->stance].dodge;
        else
          chances += (stance_index[victim->stance].dodge / 2);
    }

    if ( chances != 0 && victim->morph != NULL )
	chances+=victim->morph->dodge;

    /* Consider luck as a factor */
    if ( !chance( victim, chances + victim->level - ch->level ) )
    {
	learn_from_failure( victim, gsn_dodge );
        return FALSE;
    }

    if ( !IS_NPC(victim) && !IS_SET( victim->pcdata->flags, PCFLAG_GAG) )    
    act( AT_SKILL, "You dodge $n's attack.", ch, NULL, victim, TO_VICT    );
    
    if ( !IS_NPC(ch) && !IS_SET( ch->pcdata->flags, PCFLAG_GAG) ) 
    act( AT_SKILL, "$N dodges your attack.", ch, NULL, victim, TO_CHAR    );

    learn_from_success( victim, gsn_dodge );
    return TRUE;
}

bool check_tumble( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chances;
    int mod_tumble_by;
 
    if ( victim->class != CLASS_THIEF || !IS_AWAKE( victim ) )
      return FALSE;
    if ( !IS_NPC( victim )
    &&   !victim->pcdata->learned[gsn_tumble] > 0 )
      return FALSE;
    if ( IS_PKILL( victim ) )
	mod_tumble_by = sysdata.tumble_pk;
    else
	mod_tumble_by = sysdata.tumble_mod;
    if ( IS_NPC( victim ) )
      chances = UMIN( 60, 2 * victim->level );
    else
      chances = (int) ( LEARNED(victim, gsn_tumble) / mod_tumble_by
			+ ( get_curr_dex( victim ) - 13 ) );
    if ( chances != 0 && victim->morph )
      chances+=victim->morph->tumble;
    if ( !chance( victim, chances + victim->level - ch->level ) )
      return FALSE;
    if ( !IS_NPC(victim) && !IS_SET( victim->pcdata->flags, PCFLAG_GAG ) )
      act( AT_SKILL, "You tumble away from $n's attack.", ch, NULL, victim, TO_VICT );
    if ( !IS_NPC(ch) && !IS_SET( ch->pcdata->flags, PCFLAG_GAG ) )
      act( AT_SKILL, "$N tumbles away from your attack.", ch, NULL, victim, TO_CHAR );
    learn_from_success( victim, gsn_tumble );
    return TRUE;
}

void do_poison_weapon( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *obj;
  OBJ_DATA *pobj;
  OBJ_DATA *wobj;
  char      arg [ MAX_INPUT_LENGTH ];
  int       percent;

  if ( !IS_NPC( ch )
  && ch->level < skill_table[gsn_poison_weapon]->skill_level[ch->class] )
    {
    send_to_char( "What do you think you are, a thief?\n\r", ch );
    return;
    }

  one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "What are you trying to poison?\n\r",    ch );
	return;
    }
    if ( ch->fighting )
    {
	send_to_char( "While you're fighting?  Nice try.\n\r", ch );
	return;
    }
    if ( ms_find_obj(ch) )
	return;

    if ( !( obj = get_obj_carry( ch, arg ) ) )
    {
	send_to_char( "You do not have that weapon.\n\r",      ch );
	return;
    }
    if ( obj->item_type != ITEM_WEAPON )
    {
	send_to_char( "That item is not a weapon.\n\r",        ch );
	return;
    }
    if ( IS_OBJ_STAT( obj, ITEM_POISONED ) )
    {
	send_to_char( "That weapon is already poisoned.\n\r",  ch );
	return;
    }
    if ( IS_OBJ_STAT( obj, ITEM_CLANOBJECT ) )
    {
	send_to_char( "It doesn't appear to be fashioned of a poisonable material.\n\r", ch );
	return;
    }
    /* Now we have a valid weapon...check to see if we have the powder. */
    for ( pobj = ch->first_carrying; pobj; pobj = pobj->next_content )
    {
	if ( pobj->pIndexData->vnum == OBJ_VNUM_BLACK_POWDER )
	break;
    }
    if ( !pobj )
    {
	send_to_char( "You do not have the black poison powder.\n\r", ch );
	return;
    }
    /* Okay, we have the powder...do we have water? */
    for ( wobj = ch->first_carrying; wobj; wobj = wobj->next_content )
    {
	if ( wobj->item_type == ITEM_DRINK_CON
	&& wobj->value[1]  >  0
	&& wobj->value[2]  == 0 )
	break;
    }
    if ( !wobj )
    {
	send_to_char( "You have no water to mix with the powder.\n\r", ch );
	return;
    }
    /* Great, we have the ingredients...but is the thief smart enough? */
    if ( !IS_NPC( ch ) && get_curr_wis( ch ) < 16 )
    {
	send_to_char( "You can't quite remember what to do...\n\r", ch );
	return;
    }
    /* And does the thief have steady enough hands? */
    if ( !IS_NPC( ch )
    && ( (get_curr_dex( ch ) < 17) || ch->pcdata->condition[COND_DRUNK] > 0 ) )
    {
	send_to_char("Your hands aren't steady enough to properly mix the poison.\n\r", ch );
	return;
    }
    WAIT_STATE( ch, skill_table[gsn_poison_weapon]->beats );
  
    percent = (number_percent( ) - get_curr_lck(ch) - 14);

    /* Check the skill percentage */
    separate_obj( pobj );
    separate_obj( wobj );
    if (!can_use_skill(ch, percent, gsn_poison_weapon ) )
    {
	set_char_color( AT_RED, ch );
	send_to_char( "You failed and spill some on yourself.  Ouch!\n\r", ch );
	set_char_color( AT_GREY, ch );
	damage( ch, ch, ch->level, gsn_poison_weapon );
	act(AT_RED, "$n spills the poison all over!", ch, NULL, NULL, TO_ROOM );
	extract_obj( pobj );
	extract_obj( wobj );
	learn_from_failure( ch, gsn_poison_weapon );
	return;
    }
    separate_obj( obj );
    /* Well, I'm tired of waiting.  Are you? */
    act(AT_RED, "You mix $p in $P, creating a deadly poison!", ch, pobj, wobj, TO_CHAR );
    act(AT_RED, "$n mixes $p in $P, creating a deadly poison!",ch, pobj, wobj, TO_ROOM );
    act(AT_GREEN, "You pour the poison over $p, which glistens wickedly!",ch, obj, NULL, TO_CHAR  );
    act(AT_GREEN, "$n pours the poison over $p, which glistens wickedly!",ch, obj, NULL, TO_ROOM  );
    xSET_BIT( obj->extra_flags, ITEM_POISONED );
    obj->cost *= 2;
    /* Set an object timer.  Don't want proliferation of poisoned weapons */
    obj->timer = UMIN( obj->level, ch->level );

    if ( IS_OBJ_STAT( obj, ITEM_BLESS ) )
    obj->timer *= 2;

    if ( IS_OBJ_STAT( obj, ITEM_MAGIC ) )
    obj->timer *= 2;

    /* WHAT?  All of that, just for that one bit?  How lame. ;) */
   act(AT_BLUE, "The remainder of the poison eats through $p.", ch, wobj, NULL, TO_CHAR );
   act(AT_BLUE, "The remainder of the poison eats through $p.", ch, wobj, NULL, TO_ROOM );
   extract_obj( pobj );
   extract_obj( wobj );
   learn_from_success( ch, gsn_poison_weapon );
   return;
}

void do_scribe( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *scroll;
    int sn;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    int mana;

    if ( IS_NPC(ch) )
        return;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_scribe]->skill_level[ch->class] )
    {
	send_to_char( "A skill such as this requires more magical ability than that of your class.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' || !str_cmp(argument, "") )
    {
	send_to_char( "Scribe what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( (sn = find_spell( ch, argument, TRUE )) < 0 )
    {
         send_to_char( "You have not learned that spell.\n\r", ch );
         return;
    }

    if ( skill_table[sn]->spell_fun == spell_null )
    {
        send_to_char( "That's not a spell!\n\r", ch );
        return;
    }

    if ( SPELL_FLAG(skill_table[sn], SF_NOSCRIBE) )
    {
        send_to_char( "You cannot scribe that spell.\n\r", ch );
        return;
    }

    mana = IS_NPC(ch) ? 0 : UMAX(skill_table[sn]->min_mana,
     100 / ( 2 + ch->level - skill_table[sn]->skill_level[ch->class] ) );

    mana *=5;

    if ( !IS_NPC(ch) && ch->mana < mana )
    {
        send_to_char( "You don't have enough mana.\n\r", ch );
        return;
    }

     if ( ( scroll = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
     {
	send_to_char( "You must be holding a blank scroll to scribe it.\n\r", ch );
	return;
     }

     if( scroll->pIndexData->vnum != OBJ_VNUM_SCROLL_SCRIBING )
     {
	send_to_char( "You must be holding a blank scroll to scribe it.\n\r", ch );
	return;
     }
    
     if ( ( scroll->value[1] != -1 )
     && ( scroll->pIndexData->vnum == OBJ_VNUM_SCROLL_SCRIBING ) )
     {
	send_to_char( "That scroll has already been inscribed.\n\r", ch);
	return;
     }

     if ( !process_spell_components( ch, sn ) )
     {
	learn_from_failure( ch, gsn_scribe );
	ch->mana -= (mana / 2);
	return;
     }

     if ( !can_use_skill(ch, number_percent(), gsn_scribe ) )
     {
       set_char_color ( AT_MAGIC, ch );
       send_to_char("You failed.\n\r", ch);
       learn_from_failure( ch, gsn_scribe );
       ch->mana -= (mana / 2);
       return;
     }
     
     scroll->value[1] = sn;
     scroll->value[0] = ch->level;
     sprintf(buf1, "%s scroll", skill_table[sn]->name);
     STRFREE(scroll->short_descr);
     scroll->short_descr = STRALLOC( aoran(buf1) );

     sprintf(buf2, "A glowing scroll inscribed '%s' lies in the dust.", 
                                              skill_table[sn]->name);

     STRFREE(scroll->description);
     scroll->description = STRALLOC(buf2);

     sprintf(buf3, "scroll scribing %s", skill_table[sn]->name);
     STRFREE(scroll->name);
     scroll->name = STRALLOC(buf3);

     act( AT_MAGIC, "$n magically scribes $p.",   ch,scroll, NULL, TO_ROOM );
     act( AT_MAGIC, "You magically scribe $p.",   ch,scroll, NULL, TO_CHAR );
     WAIT_STATE( ch, skill_table[gsn_scribe]->beats );
     learn_from_success( ch, gsn_scribe );
    
     ch->mana -= mana;
     
}

void do_brew( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *potion;
    OBJ_DATA *fire;
    int sn;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    int mana;
    bool found;

    if ( IS_NPC(ch) )
        return;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_brew]->skill_level[ch->class] )
    {
        send_to_char( "A skill such as this requires more magical ability than that of your class.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' || !str_cmp(argument, "") )
    {
	send_to_char( "Brew what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( (sn = find_spell( ch, argument, TRUE )) < 0 )
    {
         send_to_char( "You have not learned that spell.\n\r", ch );
         return;
    }

    if ( skill_table[sn]->spell_fun == spell_null )
    {
        send_to_char( "That's not a spell!\n\r", ch );
        return;
    }

    if ( SPELL_FLAG(skill_table[sn], SF_NOBREW) )
    {
        send_to_char( "You cannot brew that spell.\n\r", ch );
        return;
    }

    mana = IS_NPC(ch) ? 0 : UMAX(skill_table[sn]->min_mana,
     100 / ( 2 + ch->level - skill_table[sn]->skill_level[ch->class] ) );

    mana *=4;

    if ( !IS_NPC(ch) && ch->mana < mana )
    {
        send_to_char( "You don't have enough mana.\n\r", ch );
        return;
    }
  
    found = FALSE;

    for ( fire = ch->in_room->first_content; fire; 
          fire = fire->next_content )
    {
       if( fire->item_type == ITEM_FIRE)
       {
	  found = TRUE;
	  break;
       }
    }

     if ( !found )
     {
        send_to_char(
        "There must be a fire in the room to brew a potion.\n\r", ch );
        return;
     }

     if ( ( potion = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
     {
        send_to_char(
        "You must be holding an empty flask to brew a potion.\n\r", ch );
        return;
     }

     if( potion->pIndexData->vnum != OBJ_VNUM_FLASK_BREWING )
     {
	send_to_char( "You must be holding an empty flask to brew a potion.\n\r", ch );
	return;
     }

     if ( ( potion->value[1] != -1 )
     && ( potion->pIndexData->vnum == OBJ_VNUM_FLASK_BREWING ) )
     {
	send_to_char( "That's not an empty flask.\n\r", ch);
	return;
     }

     if ( !process_spell_components( ch, sn ) )
     {
	learn_from_failure( ch, gsn_brew );
	ch->mana -= (mana / 2);
	return;
     }

     if ( !can_use_skill(ch, number_percent(), gsn_brew ) )
     {
       set_char_color ( AT_MAGIC, ch );
       send_to_char("You failed.\n\r", ch);
       learn_from_failure( ch, gsn_brew );
       ch->mana -= (mana / 2);
       return;
     }

     potion->value[1] = sn;
     potion->value[0] = ch->level;
     sprintf(buf1, "%s potion", skill_table[sn]->name);
     STRFREE(potion->short_descr);
     potion->short_descr = STRALLOC( aoran(buf1) );

     sprintf(buf2, "A strange potion labelled '%s' sizzles in a glass flask.",
                                              skill_table[sn]->name);

     STRFREE(potion->description);
     potion->description = STRALLOC(buf2);

     sprintf(buf3, "flask potion %s", skill_table[sn]->name);
     STRFREE(potion->name);
     potion->name = STRALLOC(buf3);

     act( AT_MAGIC, "$n brews up $p.",   ch,potion, NULL, TO_ROOM );
     act( AT_MAGIC, "You brew up $p.",   ch,potion, NULL, TO_CHAR );
     WAIT_STATE( ch, skill_table[gsn_brew]->beats );
     learn_from_success( ch, gsn_brew );
    
     ch->mana -= mana;
     
}

bool check_grip( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    if ( IS_NPC(victim) && !xIS_SET(victim->defenses, DFND_GRIP) )
      return FALSE;

    if ( IS_NPC(victim) )
	chance  = UMIN( 60, 2 * victim->level );
    else
        chance  = (int) (LEARNED(victim, gsn_grip) / 2);

    /* Consider luck as a factor */
    chance += (2 * (get_curr_lck(victim) - 13 ) );

    if ( number_percent( ) >= chance + victim->level - ch->level )
    {
	learn_from_failure( victim, gsn_grip );
        return FALSE;
    }
    act( AT_SKILL, "You evade $n's attempt to disarm you.", ch, NULL, victim, TO_VICT    );
    act( AT_SKILL, "$N holds $S weapon strongly, and is not disarmed.", 
  	 ch, NULL, victim, TO_CHAR    );
    learn_from_success( victim, gsn_grip );
    return TRUE;
}

void do_circle( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You can't concentrate enough for that.\n\r", ch );
	return;
    }

    one_argument( argument, arg );

    if ( ch->mount )
    {
	send_to_char( "You can't circle while mounted.\n\r", ch );
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Circle around whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you sneak up on yourself?\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim, TRUE ) )
      return;

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
    ||   ( obj->value[3] != 11 && obj->value[3] != 2 ) )
    {
	send_to_char( "You need to wield a piercing or stabbing weapon.\n\r", ch );
	return;
    }

    if ( !ch->fighting )
    { 
	send_to_char( "You can't circle when you aren't fighting.\n\r", ch);
	return;
    }

    if ( !victim->fighting )
    {
	send_to_char( "You can't circle around a person who is not fighting.\n\r", ch );
	return;
    }

    if ( victim->num_fighting < 2 )
    {
    act( AT_PLAIN, "You can't circle around them without a distraction.",
	    ch, NULL, victim, TO_CHAR );
	return;
    }

    percent = number_percent( ) - (get_curr_lck(ch) - 16) 
	      + (get_curr_lck(victim) - 13);

    check_attacker( ch, victim );
    WAIT_STATE( ch, skill_table[gsn_circle]->beats );
    if ( can_use_skill( ch, percent, gsn_circle ) )
    {
	learn_from_success( ch, gsn_circle );
	WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
	global_retcode = multi_hit( ch, victim, gsn_circle );
	adjust_favor( ch, 10, 1 );	
        check_illegal_pk( ch, victim );
    }
    else
    {
	learn_from_failure( ch, gsn_circle );
	WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
	global_retcode = damage( ch, victim, 0, gsn_circle );
    }
    return;
}

/* Berserk and HitAll. -- Altrag */
void do_berserk( CHAR_DATA *ch, char *argument )
{
  sh_int percent;
  AFFECT_DATA af;
  
  if ( !ch->fighting )
  {
    send_to_char( "But you aren't fighting!\n\r", ch );
    return;
  }
  
  if ( IS_AFFECTED(ch, AFF_BERSERK) )
  {
    send_to_char( "Your rage is already at its peak!\n\r", ch );
    return;
  }
  
  percent = LEARNED(ch, gsn_berserk);
  WAIT_STATE(ch, skill_table[gsn_berserk]->beats);
  if ( !chance(ch, percent) )
  {
    send_to_char( "You couldn't build up enough rage.\n\r", ch);
    learn_from_failure(ch, gsn_berserk);
    return;
  }
  af.type = gsn_berserk;
  /* Hmmm.. 10-20 combat rounds at level 50.. good enough for most mobs,
     and if not they can always go berserk again.. shrug.. maybe even
     too high. -- Altrag */
  af.duration = number_range(ch->level/5, ch->level*2/5);
  /* Hmm.. you get stronger when yer really enraged.. mind over matter
     type thing.. */
  af.location = APPLY_STR;
  af.modifier = 1;
  af.bitvector = meb(AFF_BERSERK);
  affect_to_char(ch, &af);
  send_to_char( "You start to lose control..\n\r", ch );
  learn_from_success(ch, gsn_berserk);
  return;
}

/* External from fight.c */
ch_ret one_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void do_hitall( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  sh_int nvict = 0;
  sh_int nhit = 0;
  sh_int percent;
  
  if ( xIS_SET(ch->in_room->room_flags, ROOM_SAFE) )
  {
    send_to_char_color( "&BA godly force prevents you.\n\r", ch);
    return;
  }

  if ( !ch->in_room->first_person )
  {
    send_to_char( "There's no one else here!\n\r", ch );
    return;
  }
  percent = LEARNED(ch, gsn_hitall);
  for ( vch = ch->in_room->first_person; vch; vch = vch_next )
  {
    vch_next = vch->next_in_room;
    if ( is_same_group(ch, vch) || !is_legal_kill(ch, vch) ||
        !can_see(ch, vch) || is_safe(ch, vch, TRUE) )
      continue;
    if ( ++nvict > ch->level / 5 )
      break;
    check_illegal_pk(ch, vch);
    if ( chance(ch, percent) )
    {
      nhit++;
      global_retcode = one_hit(ch, vch, TYPE_UNDEFINED);
    }
    else
      global_retcode = damage(ch, vch, 0, TYPE_UNDEFINED);
    /* Fireshield, etc. could kill ch too.. :>.. -- Altrag */
    if ( global_retcode == rCHAR_DIED || global_retcode == rBOTH_DIED
    ||   char_died(ch) )
      return;
  }
  if ( !nvict )
  {
    send_to_char( "There's no one else here!\n\r", ch );
    return;
  }
  ch->move = UMAX(0, ch->move-nvict*3+nhit);
  if ( nhit )
    learn_from_success(ch, gsn_hitall);
  else
    learn_from_failure(ch, gsn_hitall);
  return;
}
    
  

bool check_illegal_psteal( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if (!IS_NPC (victim) && !IS_NPC(ch))
    {
	if ( ( !IS_SET( victim->pcdata->flags, PCFLAG_DEADLY )
	|| ch->level - victim->level > 10
	|| !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
	&& ( ch->in_room->vnum < 29 || ch->in_room->vnum > 43 )
	&& ch != victim )
	{
	    /* 
	    sprintf( log_buf, "%s illegally stealing from %s at %d",
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		victim->name,
		victim->in_room->vnum );
	    log_string( log_buf );
	    to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
	    */
	    return TRUE;
	}
    }
    return FALSE;
}

void do_scan( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in_room;
    EXIT_DATA *pexit;
    sh_int dir = -1;
    sh_int dist;
    sh_int max_dist = 8;

    set_char_color( AT_ACTION, ch );

    if ( IS_AFFECTED( ch, AFF_BLIND ) )
    {
	send_to_char( "Not very effective when you're blind...\n\r", ch );
	return;
    }
 
    if ( argument[0] == '\0' )
    {
	send_to_char( "Scan in a direction...\n\r", ch );
	return;
    }

    if ( ( dir = get_door( argument ) ) == -1 )
    {
	send_to_char( "Scan in WHAT direction?\n\r", ch );
	return;
    }

    was_in_room = ch->in_room;
    act( AT_GREY, "Scanning $t...", ch, dir_name[dir], NULL, TO_CHAR );
    act( AT_GREY, "$n scans $t.", ch, dir_name[dir], NULL, TO_ROOM );

    if ( !can_use_skill(ch, number_percent(), gsn_scan ) )
    {
	act( AT_GREY, "You stop scanning $t as your vision blurs.", ch,
	    dir_name[dir], NULL, TO_CHAR );
	learn_from_failure( ch, gsn_scan );
	return;
    }

    if ( IS_VAMPIRE( ch ) )
    {
	if ( time_info.hour < 21 && time_info.hour > 5 )
	{
	    send_to_char( "You have trouble seeing clearly through all the "
		"light.\n\r", ch );
	    max_dist = 1;
	}
    }

    if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
    {
	act( AT_GREY, "You can't see $t.", ch, dir_name[dir], NULL, TO_CHAR );
	return;
    }

    if ( ch->level < 50 ) --max_dist;
    if ( ch->level < 40 ) --max_dist;
    if ( ch->level < 30 ) --max_dist;

    for ( dist = 1; dist <= max_dist; )
    {
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    if ( IS_SET(pexit->exit_info, EX_SECRET)
	    ||   IS_SET(pexit->exit_info, EX_DIG) )
		act( AT_GREY, "Your view $t is blocked by a wall.", ch,
		    dir_name[dir], NULL, TO_CHAR );
	    else
		act( AT_GREY, "Your view $t is blocked by a door.", ch, 
		    dir_name[dir], NULL, TO_CHAR );
	    break;
	}
	if ( room_is_private( pexit->to_room )
	&&   ch->level < LEVEL_GREATER )
	{
	    act( AT_GREY, "Your view $t is blocked by a private room.", ch, 
		dir_name[dir], NULL, TO_CHAR );
	    break;
	}
	char_from_room( ch );
	char_to_room( ch, pexit->to_room );    
	set_char_color( AT_RMNAME, ch );
	if ( xIS_SET( ch->in_room->room_flags, ROOM_COLOR) )
	  send_to_char_color( ch->in_room->name, ch );
	else
	  send_to_char( ch->in_room->name, ch );
	send_to_char( "\n\r", ch );
	show_list_to_char( ch->in_room->first_content, ch, FALSE, FALSE );
	show_char_to_char( ch->in_room->first_person, ch );

	switch( ch->in_room->sector_type )
	{
	    default: dist++; break;
	    case SECT_AIR:
		if ( number_percent() < 80 ) dist++; break;
	    case SECT_INSIDE:
	    case SECT_FIELD:
	    case SECT_UNDERGROUND:
		dist++; break;
	    case SECT_FOREST:
	    case SECT_CITY:
	    case SECT_DESERT:
	    case SECT_HILLS:
		dist += 2; break;
	    case SECT_WATER_SWIM:
	    case SECT_WATER_NOSWIM:
		dist += 3; break;
	    case SECT_MOUNTAIN:
	    case SECT_UNDERWATER:
	    case SECT_OCEANFLOOR:
		dist += 4; break;
	}

	if ( dist >= max_dist )
	{
	    act( AT_GREY, "Your vision blurs with distance and you see no "
		"farther $t.", ch, dir_name[dir], NULL, TO_CHAR );
	    break;
	}
	if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
	{
	    act( AT_GREY, "Your view $t is blocked by a wall.", ch, 
		dir_name[dir], NULL, TO_CHAR );
	    break;
	}
    }
  
    char_from_room( ch );
    char_to_room( ch, was_in_room );
    learn_from_success( ch, gsn_scan );

    return;
}


/*
 * Basically the same guts as do_scan() from above (please keep them in
 * sync) used to find the victim we're firing at.	-Thoric
 */
CHAR_DATA *scan_for_victim( CHAR_DATA *ch, EXIT_DATA *pexit, char *name )
{
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *was_in_room;
    sh_int dist, dir;
    sh_int max_dist = 8;

    if ( IS_AFFECTED(ch, AFF_BLIND) || !pexit )
	return NULL;
 
    was_in_room = ch->in_room;
    if ( IS_VAMPIRE(ch) && time_info.hour < 21 && time_info.hour > 5 )
	max_dist = 1;

    if ( ch->level < 50 ) --max_dist;
    if ( ch->level < 40 ) --max_dist;
    if ( ch->level < 30 ) --max_dist;

    for ( dist = 1; dist <= max_dist; )
    {
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	    break;

	if ( room_is_private( pexit->to_room )
	&&   ch->level < LEVEL_GREATER )
	    break;

	char_from_room( ch );
	char_to_room( ch, pexit->to_room );    

	if ( (victim=get_char_room(ch, name)) != NULL )
	{
	    char_from_room(ch);
	    char_to_room(ch, was_in_room);
	    return victim;
	}

	switch( ch->in_room->sector_type )
	{
	    default: dist++; break;
	    case SECT_AIR:
		if ( number_percent() < 80 ) dist++; break;
	    case SECT_INSIDE:
	    case SECT_FIELD:
	    case SECT_UNDERGROUND:
		dist++; break;
	    case SECT_FOREST:
	    case SECT_CITY:
	    case SECT_DESERT:
	    case SECT_HILLS:
		dist += 2; break;
	    case SECT_WATER_SWIM:
	    case SECT_WATER_NOSWIM:
		dist += 3; break;
	    case SECT_MOUNTAIN:
	    case SECT_UNDERWATER:
	    case SECT_OCEANFLOOR:
		dist += 4; break;
	}

	if ( dist >= max_dist )
	    break;

	dir = pexit->vdir;
	if ( (pexit=get_exit(ch->in_room, dir)) == NULL )
	    break;
    }
  
    char_from_room(ch);
    char_to_room(ch, was_in_room);

    return NULL;
}

/*
 * Search inventory for an appropriate projectile to fire.
 * Also search open quivers.					-Thoric
 */
OBJ_DATA *find_projectile( CHAR_DATA *ch, int type )
{
    OBJ_DATA *obj, *obj2;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
	if ( can_see_obj(ch, obj) )
	{
	    if ( obj->item_type == ITEM_QUIVER && !IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		for ( obj2 = obj->last_content; obj2; obj2 = obj2->prev_content )
		{
		    if ( obj2->item_type == ITEM_PROJECTILE
		    &&   obj2->value[3] == type )
			return obj2;
		}
	    }
	    if ( obj->item_type == ITEM_PROJECTILE && obj->value[3] == type )
		return obj;
	}
    }

    return NULL;
}


ch_ret spell_attack( int, int, CHAR_DATA *, void * );

/*
 * Perform the actual attack on a victim			-Thoric
 */
ch_ret ranged_got_target( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *weapon,
	OBJ_DATA *projectile, sh_int dist, sh_int dt, char *stxt, sh_int color )
{
    if ( xIS_SET(ch->in_room->room_flags, ROOM_SAFE) )
    {
	/* safe room, bubye projectile */
	if ( projectile )
	{
	    ch_printf(ch, "Your %s is blasted from existance by a godly presense.",
		myobj(projectile) );
	    act( color, "A godly presence smites $p!", ch, projectile, NULL, TO_ROOM );
	    extract_obj(projectile);
	}
	else
	{
	    ch_printf(ch, "Your %s is blasted from existance by a godly presense.",
		stxt );
	    act( color, "A godly presence smites $t!", ch, aoran(stxt), NULL, TO_ROOM );
	}
	return rNONE;
    }

    if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_SENTINEL)
    &&   ch->in_room != victim->in_room )
    {
	/*
	 * letsee, if they're high enough.. attack back with fireballs
	 * long distance or maybe some minions... go herne! heh..
	 *
	 * For now, just always miss if not in same room  -Thoric
	 */

	if ( projectile )
	{
	    learn_from_failure( ch, gsn_missile_weapons );

	    /* 50% chance of projectile getting lost */
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
	}
	return damage( ch, victim, 0, dt );
    }

    if ( number_percent() > 50 || (projectile && weapon
    &&   can_use_skill(ch, number_percent(), gsn_missile_weapons)) )
    {
	if ( projectile )
	    global_retcode = projectile_hit(ch, victim, weapon, projectile, dist );
	else
	    global_retcode = spell_attack( dt, ch->level, ch, victim );
    }
    else
    {
	learn_from_failure( ch, gsn_missile_weapons );
	global_retcode = damage( ch, victim, 0, dt );

	if ( projectile )
	{
	    /* 50% chance of getting lost */
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
	}
    }
    return global_retcode;
}

/*
 * Generic use ranged attack function			-Thoric & Tricops
 */
ch_ret ranged_attack( CHAR_DATA *ch, char *argument, OBJ_DATA *weapon,
		      OBJ_DATA *projectile, sh_int dt, sh_int range )
{
    CHAR_DATA *victim, *vch;
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *was_in_room;
    char arg[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char temp[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    SKILLTYPE *skill = NULL;
    sh_int dir = -1, dist = 0, color = AT_GREY;
    char *dtxt = "somewhere";
    char *stxt = "burst of energy";
    int count;


    if ( argument && argument[0] != '\0' && argument[0] == '\''){
      one_argument( argument, temp );
      argument = temp;
    }

    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg1);

    if ( arg[0] == '\0' )
    {
	send_to_char( "Where?  At who?\n\r", ch );
	return rNONE;
    }

    victim = NULL;

    /* get an exit or a victim */
    if ( (pexit = find_door(ch, arg, TRUE)) == NULL )
    {
	if ( (victim=get_char_room(ch, arg)) == NULL )
	{
	    send_to_char( "Aim in what direction?\n\r", ch );
	    return rNONE;
	}
	else
	{
	    if ( who_fighting(ch) == victim )
	    {
		send_to_char( "They are too close to release that type of attack!\n\r", ch );
		return rNONE;
	    }
	    /* Taken out because of waitstate 
	    if ( !IS_NPC(ch) && !IS_NPC(victim) )
	    {
	    	send_to_char("Pkill like a real pkiller.\n\r", ch );
		return rNONE;
	    }
	    */
	}
    }
    else
	dir = pexit->vdir;

    /* check for ranged attacks from private rooms, etc */
    if ( !victim )
    {
	if ( xIS_SET(ch->in_room->room_flags, ROOM_PRIVATE)
	||   xIS_SET(ch->in_room->room_flags, ROOM_SOLITARY) )
	{
	    send_to_char( "You cannot perform a ranged attack from a private room.\n\r", ch );
	    return rNONE;
	}
	if ( ch->in_room->tunnel > 0 )
	{
	    count = 0;
	    for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
		++count;
	    if ( count >= ch->in_room->tunnel )
	    {
		send_to_char( "This room is too cramped to perform such an attack.\n\r", ch );
		return rNONE;
	    }
	}
    }

    if ( IS_VALID_SN(dt) )
	skill = skill_table[dt];

    if ( pexit && !pexit->to_room )
    {
	send_to_char( "Are you expecting to fire through a wall!?\n\r", ch );
	return rNONE;
    }

    /* Check for obstruction */
    if ( pexit && IS_SET(pexit->exit_info, EX_CLOSED) )
    {
	if ( IS_SET(pexit->exit_info, EX_SECRET)
	||   IS_SET(pexit->exit_info, EX_DIG) )
	    send_to_char( "Are you expecting to fire through a wall!?\n\r", ch );
	else
	    send_to_char( "Are you expecting to fire through a door!?\n\r", ch );
	return rNONE;
    }

    vch = NULL;
    if ( pexit && arg1[0] != '\0' )
    {
	if ( (vch=scan_for_victim(ch, pexit, arg1)) == NULL )
	{
	    send_to_char( "You cannot see your target.\n\r", ch );
	    return rNONE;
	}

	/* don't allow attacks on mobs stuck in another area?
	if ( IS_NPC(vch) && xIS_SET(vch->act, ACT_STAY_AREA)
	&&   ch->in_room->area != vch->in_room->area) )
	{
	}
	*/
	/*don't allow attacks on mobs that are in a no-missile room --Shaddai */
	if ( xIS_SET(vch->in_room->room_flags, ROOM_NOMISSILE) )
	{
	    send_to_char( "You can't get a clean shot off.\n\r", ch );
	    return rNONE;
	}
	/* Taken out cause of wait state
	if ( !IS_NPC(ch) && !IS_NPC(vch) )
	{
	    send_to_char("Pkill like a real pkiller.\n\r", ch );
	    return rNONE;
	}
	*/

	/* can't properly target someone heavily in battle */
	if ( vch->num_fighting > max_fight(vch) )
	{
	    send_to_char( "There is too much activity there for you to get a clear shot.\n\r", ch );
	    return rNONE;
	}
    }
    if ( vch ) {
    if ( !IS_NPC( vch ) && !IS_NPC( ch ) &&
	 xIS_SET(ch->act, PLR_NICE ) )
    {
	send_to_char( "You're too nice to do that!\n\r", ch );
	return rNONE;
    }
    if ( vch && is_safe(ch, vch, TRUE) )
	    return rNONE;
    }
    was_in_room = ch->in_room;

    if ( projectile )
    {
	separate_obj(projectile);
	if ( pexit )
	{
	    if ( weapon )
	    {
		act( AT_GREY, "You fire $p $T.", ch, projectile, dir_name[dir], TO_CHAR );
		act( AT_GREY, "$n fires $p $T.", ch, projectile, dir_name[dir], TO_ROOM );
	    }
	    else
	    {
		act( AT_GREY, "You throw $p $T.", ch, projectile, dir_name[dir], TO_CHAR );
		act( AT_GREY, "$n throw $p $T.", ch, projectile, dir_name[dir], TO_ROOM );
	    }
	}
	else
	{
	    if ( weapon )
	    {
		act( AT_GREY, "You fire $p at $N.", ch, projectile, victim, TO_CHAR );
		act( AT_GREY, "$n fires $p at $N.", ch, projectile, victim, TO_NOTVICT );
		act( AT_GREY, "$n fires $p at you!", ch, projectile, victim, TO_VICT );
	    }
	    else
	    {
		act( AT_GREY, "You throw $p at $N.", ch, projectile, victim, TO_CHAR );
		act( AT_GREY, "$n throws $p at $N.", ch, projectile, victim, TO_NOTVICT );
		act( AT_GREY, "$n throws $p at you!", ch, projectile, victim, TO_VICT );
	    }
	}
    }
    else
    if ( skill )
    {
	if ( skill->noun_damage && skill->noun_damage[0] != '\0' )
	    stxt = skill->noun_damage;
	else
	    stxt = skill->name;
	/* a plain "spell" flying around seems boring */
	if ( !str_cmp(stxt, "spell") )
	    stxt = "magical burst of energy";
	if ( skill->type == SKILL_SPELL )
	{
	    color = AT_MAGIC;
	    if ( pexit )
	    {
		act( AT_MAGIC, "You release $t $T.", ch, aoran(stxt), dir_name[dir], TO_CHAR );
		act( AT_MAGIC, "$n releases $s $t $T.", ch, stxt, dir_name[dir], TO_ROOM );
	    }
	    else
	    {
		act( AT_MAGIC, "You release $t at $N.", ch, aoran(stxt), victim, TO_CHAR );
		act( AT_MAGIC, "$n releases $s $t at $N.", ch, stxt, victim, TO_NOTVICT );
		act( AT_MAGIC, "$n releases $s $t at you!", ch, stxt, victim, TO_VICT );
	    }
	}
    }
    else
    {
	bug( "Ranged_attack: no projectile, no skill dt %d", dt );
	return rNONE;
    }
  
    /* victim in same room */
    if ( victim )
    {
    	check_illegal_pk( ch, victim );
	check_attacker( ch, victim );
	return ranged_got_target( ch, victim, weapon, projectile,
		0, dt, stxt, color );
    }

    /* assign scanned victim */
    victim = vch;

    /* reverse direction text from move_char */
    dtxt = rev_exit(pexit->vdir);

    while ( dist <= range )
    {
	char_from_room(ch);
	char_to_room(ch, pexit->to_room);

	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    /* whadoyahknow, the door's closed */
	    if ( projectile )
		sprintf(buf,"You see your %s pierce a door in the distance to the %s.", 
		    myobj(projectile), dir_name[dir] );
	    else
		sprintf(buf, "You see your %s hit a door in the distance to the %s.",
		    stxt, dir_name[dir] );
	    act( color, buf, ch, NULL, NULL, TO_CHAR );
	    if ( projectile )
	    {
		sprintf(buf,"$p flies in from %s and implants itself solidly in the %sern door.",
		    dtxt, dir_name[dir] );
		act( color, buf, ch, projectile, NULL, TO_ROOM );
	    }
	    else
	    {
		sprintf(buf, "%s flies in from %s and implants itself solidly in the %sern door.",
		    aoran(stxt), dtxt, dir_name[dir] );
		buf[0] = UPPER(buf[0]);
		act( color, buf, ch, NULL, NULL, TO_ROOM );
	    }
	    break; 
	}


	/* no victim? pick a random one */
	if ( !victim )
	{
	    for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
	    {
		if ( ((IS_NPC(ch) && !IS_NPC(vch))
		||   (!IS_NPC(ch) &&  IS_NPC(vch)))
		&&    number_bits(1) == 0 )
		{
		    victim = vch;
		    break;
		}
	    }
	    if ( victim && is_safe(ch, victim, TRUE) )
	    {
	        char_from_room(ch);
	        char_to_room(ch, was_in_room);
		return rNONE;
	    }
	}

	/* In the same room as our victim? */
	if ( victim && ch->in_room == victim->in_room )
	{
	    if ( projectile )
		act( color, "$p flies in from $T.", ch, projectile, dtxt, TO_ROOM );
	    else
		act( color, "$t flies in from $T.", ch, aoran(stxt), dtxt, TO_ROOM );

	    /* get back before the action starts */
	    char_from_room(ch);
	    char_to_room(ch, was_in_room);

	    check_illegal_pk( ch, victim );
	    check_attacker( ch, victim );
	    return ranged_got_target( ch, victim, weapon, projectile,
					dist, dt, stxt, color );
 	}

	if ( dist == range ) 
	{
	    if ( projectile )
	    {
		act( color, "Your $t falls harmlessly to the $T.", 
		    ch, myobj(projectile), dir_name[dir], TO_CHAR );
		act( color, "$p flies in from $T and falls harmlessly to the ground.",
		    ch, projectile, dtxt, TO_ROOM );
		if ( projectile->in_obj )
		    obj_from_obj(projectile);
		if ( projectile->carried_by )
		    obj_from_char(projectile);
		obj_to_room(projectile, ch->in_room);
	    }
	    else
	    {
		act( color, "Your $t fizzles out harmlessly to the $T.", ch, stxt, dir_name[dir], TO_CHAR );
		act( color, "$t flies in from $T and fizzles out harmlessly.",
		    ch, aoran(stxt), dtxt, TO_ROOM );
	    }
	    break;
	}

	if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
	{
	    if ( projectile )
	    {
		act( color, "Your $t hits a wall and bounces harmlessly to the $T.", 
		    ch, myobj(projectile), dir_name[dir], TO_CHAR );
		act( color, "$p falls harmlessly to the ground.",
		    ch, projectile, dir_name[dir], TO_ROOM );
		if ( projectile->in_obj )
		    obj_from_obj(projectile);
		if ( projectile->carried_by )
		    obj_from_char(projectile);
		obj_to_room(projectile, ch->in_room);
	    }
	    else
	    {
		act( color, "Your $t harmlessly hits a wall to the $T.", 
		    ch, stxt, dir_name[dir], TO_CHAR );
		act( color, "$t strikes the $Tern wall and falls harmlessly to the ground.",
		    ch, aoran(stxt), dir_name[dir], TO_ROOM );
	    }
	    break;
	}
	if ( projectile )
	    act( color, "$p flies in from $T.", ch, projectile, dtxt, TO_ROOM );
	else
	    act( color, "$t flies in from $T.", ch, aoran(stxt), dtxt, TO_ROOM );
	dist++;
    }

    char_from_room( ch );
    char_to_room( ch, was_in_room );

    return rNONE;
}

/*
 * Fire <direction> <target>
 *
 * Fire a projectile from a missile weapon (bow, crossbow, etc)
 *
 * Design by Thoric, coding by Thoric and Tricops.
 *
 * Support code (see projectile_hit(), quiver support, other changes to
 * fight.c, etc by Thoric.
 */
void do_fire( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *arrow;
    OBJ_DATA *bow;
    sh_int max_dist;

    if ( argument[0] == '\0' || !str_cmp(argument, " ") )
    {
	send_to_char( "Fire where at who?\n\r", ch );
	return;
    }

    if ( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "A magical force prevents you from attacking.\n\r", ch );
	return;
    }

    /*
     * Find the projectile weapon
     */
    if ( (bow=get_eq_char(ch, WEAR_MISSILE_WIELD)) != NULL )
	if ( !(bow->item_type == ITEM_MISSILE_WEAPON) )
	    bow = NULL;

    if ( !bow )
    {
	send_to_char( "You are not wielding a missile weapon.\n\r", ch );
	return;
    }

    /* modify maximum distance based on bow-type and ch's class/str/etc */
    max_dist = URANGE( 1, bow->value[4], 10 );

    if ( (arrow=find_projectile(ch, bow->value[3])) == NULL )
    {
	char *msg = "You have nothing to fire...\n\r";

	switch( bow->value[3] )
	{
	    case DAM_BOLT:	msg = "You have no bolts...\n\r";	break;
	    case DAM_ARROW:	msg = "You have no arrows...\n\r";	break;
	    case DAM_DART:	msg = "You have no darts...\n\r";	break;
	    case DAM_STONE:	msg = "You have no slingstones...\n\r";	break;
	    case DAM_PEA:	msg = "You have no peas...\n\r";	break;
	}
	send_to_char( msg, ch );
	return;
    }

    /* Add wait state to fire for pkill, etc... */
    WAIT_STATE( ch, 6 );

    /* handle the ranged attack */
    ranged_attack( ch, argument, bow, arrow, TYPE_HIT + arrow->value[3], max_dist );

    return;
}

/*
 * Attempt to fire at a victim.
 * Returns FALSE if no attempt was made
 */
bool mob_fire( CHAR_DATA *ch, char *name )
{
    OBJ_DATA *arrow;
    OBJ_DATA *bow;
    sh_int max_dist;

    if ( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
	return FALSE;

    if ( (bow=get_eq_char(ch, WEAR_MISSILE_WIELD)) != NULL )
	if ( !(bow->item_type == ITEM_MISSILE_WEAPON) )
	    bow = NULL;

    if ( !bow )
	return FALSE;

    /* modify maximum distance based on bow-type and ch's class/str/etc */
    max_dist = URANGE( 1, bow->value[4], 10 );

    if ( (arrow=find_projectile(ch, bow->value[3])) == NULL )
	return FALSE;

    ranged_attack( ch, name, bow, arrow, TYPE_HIT + arrow->value[3], max_dist );

    return TRUE;
}

/* -- working on -- 
 * Syntaxes: throw object  (assumed already fighting)
 *	     throw object direction target  (all needed args for distance 
 *	          throwing)
 *	     throw object  (assumed same room throw)

void do_throw( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA *was_in_room;
  CHAR_DATA *victim;
  OBJ_DATA *throw_obj;
  EXIT_DATA *pexit;
  sh_int dir;
  sh_int dist;
  sh_int max_dist = 3;
  char arg[MAX_INPUT_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  for ( throw_obj = ch->last_carrying; throw_obj;
	throw_obj = throw_obj=>prev_content )
  {
---    if ( can_see_obj( ch, throw_obj )
	&& ( throw_obj->wear_loc == WEAR_HELD || throw_obj->wear_loc == 
	WEAR_WIELDED || throw_obj->wear_loc == WEAR_DUAL_WIELDED )
	&& nifty_is_name( arg, throw_obj->name ) )
      break;
 ----
    if ( can_see_obj( ch, throw_obj ) && nifty_is_name( arg, throw_obj->name )
      break;
  }

  if ( !throw_obj )
  {
    send_to_char( "You aren't holding or wielding anything like that.\n\r", ch );
    return;
  }

----
  if ( ( throw_obj->item_type != ITEM_WEAPON)
  {
    send_to_char("You can only throw weapons.\n\r", ch );
    return;
  }
----

  if (get_obj_weight( throw_obj ) - ( 3 * (get_curr_str(ch) - 15) ) > 0)
  {
    send_to_char("That is too heavy for you to throw.\n\r", ch);
    if (!number_range(0,10))
      learn_from_failure( ch, gsn_throw );
    return;
  }

  if ( ch->fighting )
    victim = ch->fighting;
   else
    {
      if ( ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
  	&& ( arg2[0] == '\0' ) )
      {
        act( AT_GREY, "Throw $t at whom?", ch, obj->short_descr, NULL,  
	  TO_CHAR );
        return;
      }
    }
}*/
  
void do_slice( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA *corpse;
  OBJ_DATA *obj;
  OBJ_DATA *slice;
  bool found;
  MOB_INDEX_DATA *pMobIndex;
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  found = FALSE;


  /*
   * Noticed that it was checking gsn_kick.  Bug report by Li'l Lukey
   */

  if ( !IS_NPC(ch) && !IS_IMMORTAL(ch)
  &&   ch->level < skill_table[gsn_slice]->skill_level[ch->class] )
  {
    send_to_char("You are not learned in this skill.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' )
  { 
    send_to_char("From what do you wish to slice meat?\n\r", ch);
    return;
  }


  if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
  ||   ( obj->value[3] != 1 && obj->value[3] != 2 && obj->value[3] != 3
      && obj->value[3] != 11) )
  {
    send_to_char( "You need to wield a sharp weapon.\n\r", ch);
    return;
  }

  if ( (corpse = get_obj_here( ch, argument )) == NULL)
  {  
    send_to_char("You can't find that here.\n\r", ch);
    return;
  }

  if (corpse->item_type != ITEM_CORPSE_NPC || corpse->value[3] < 75)
  {
    send_to_char("That is not a suitable source of meat.\n\r", ch);
    return;
  }

  if ( (pMobIndex = get_mob_index( -(corpse->value[2] ))) == NULL )   
  {
        bug("Can not find mob for value[2] of corpse, do_slice", 0);
        return;
  }

  if ( get_obj_index(OBJ_VNUM_SLICE) == NULL )
  {
    bug("Vnum 24 not found for do_slice!", 0);
    return;
  }

  if ( !can_use_skill(ch, number_percent(), gsn_slice ) && !IS_IMMORTAL(ch))
  { 
    send_to_char("You fail to slice the meat properly.\n\r", ch);
    learn_from_failure(ch, gsn_slice); /* Just in case they die :> */
    if ( number_percent() + (get_curr_dex(ch) - 13) < 10)   
    {
      act(AT_BLOOD, "You cut yourself!", ch, NULL, NULL, TO_CHAR);
      damage(ch, ch, ch->level, gsn_slice);
    }
    return;
  }

  slice = create_object( get_obj_index(OBJ_VNUM_SLICE), 0 );

  sprintf(buf, "meat fresh slice %s", pMobIndex->player_name);
  STRFREE(slice->name);
  slice->name = STRALLOC(buf);

  sprintf(buf, "a slice of raw meat from %s", pMobIndex->short_descr);
  STRFREE(slice->short_descr);
  slice->short_descr = STRALLOC(buf);

  sprintf(buf1, "A slice of raw meat from %s lies on the ground.", pMobIndex->short_descr);
  STRFREE(slice->description);
  slice->description = STRALLOC(buf1);
    
  act( AT_BLOOD, "$n cuts a slice of meat from $p.", ch, corpse, NULL, TO_ROOM);
  act( AT_BLOOD, "You cut a slice of meat from $p.", ch, corpse, NULL, TO_CHAR);

  obj_to_char(slice, ch);
  corpse->value[3] -= 25;
  learn_from_success(ch, gsn_slice);
  return;
}

/*------------------------------------------------------------ 
 *  Fighting Styles - haus
 */void do_style( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
/*  char buf[MAX_INPUT_LENGTH];
    int percent; */

    if ( IS_NPC(ch) )
	return;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	ch_printf_color( ch, "&wAdopt which fighting style?  (current:  %s&w)\n\r",
          ch->style == STYLE_BERSERK    ? "&Rberserk"    :
          ch->style == STYLE_AGGRESSIVE ? "&Raggressive" :
          ch->style == STYLE_DEFENSIVE  ? "&Ydefensive"  :
          ch->style == STYLE_EVASIVE    ? "&Yevasive"    :
                                          "standard"     );
	return;
    }

    if( !str_prefix(arg, "evasive") ){
      if( ch->level < skill_table[gsn_style_evasive]->skill_level[ch->class])
      {
         send_to_char( "You have not yet learned enough to fight evasively.\n\r",ch);
         return;
      }
      WAIT_STATE( ch, skill_table[gsn_style_evasive]->beats );
      if( number_percent() < LEARNED(ch, gsn_style_evasive) ){
          /* success */
	  if(ch->fighting){
             ch->position = POS_EVASIVE;
             learn_from_success(ch,gsn_style_evasive);
	     if ( IS_PKILL( ch ) )
		act( AT_ACTION, "$n falls back into an evasive stance.",
                  ch, NULL, NULL, TO_ROOM );
          }
          ch->style = STYLE_EVASIVE;
          send_to_char( "You adopt an evasive fighting style.\n\r",ch);
          return;
      } else {
          /* failure */ 
          send_to_char( "You nearly trip in a lame attempt to adopt an evasive fighting style.\n\r",ch);
	  return;
      }
    } else if( !str_prefix(arg, "defensive")){
      if( ch->level < skill_table[gsn_style_defensive]->skill_level[ch->class])
      {
         send_to_char( "You have not yet learned enough to fight defensively.\n\r",ch);
         return;
      }
      WAIT_STATE( ch, skill_table[gsn_style_defensive]->beats );
      if( number_percent() < LEARNED(ch, gsn_style_defensive) ){
          /* success */
	  if(ch->fighting) {
             ch->position = POS_DEFENSIVE;
             learn_from_success(ch,gsn_style_defensive);
	     if ( IS_PKILL( ch ) )
		act( AT_ACTION, "$n moves into a defensive posture.",
                  ch, NULL, NULL, TO_ROOM );
          }
          ch->style = STYLE_DEFENSIVE;
          send_to_char( "You adopt a defensive fighting style.\n\r",ch);
          return;
      } else {
          /* failure */ 
          send_to_char( "You nearly trip in a lame attempt to adopt a defensive fighting style.\n\r",ch);
	  return;
      }
    } else if( !str_prefix(arg,"standard")){
      if( ch->level < skill_table[gsn_style_standard]->skill_level[ch->class])
      {
         send_to_char( "You have not yet learned enough to fight in the standard style.\n\r",ch);
         return;
      }
      WAIT_STATE( ch, skill_table[gsn_style_standard]->beats );
      if( number_percent() < LEARNED(ch, gsn_style_standard) ){
          /* success */
	  if(ch->fighting) {
             ch->position = POS_FIGHTING;
             learn_from_success(ch,gsn_style_standard);
	      if ( IS_PKILL( ch ) )
		act( AT_ACTION, "$n switches to a standard fighting style.",
                  ch, NULL, NULL, TO_ROOM );
          }
          ch->style = STYLE_FIGHTING;
          send_to_char( "You adopt a standard fighting style.\n\r",ch);
          return;
      } else {
          /* failure */ 
          send_to_char( "You nearly trip in a lame attempt to adopt a standard fighting style.\n\r",ch);
	  return;
      }
    } else if( !str_prefix(arg,"aggressive")){
      if( ch->level < skill_table[gsn_style_aggressive]->skill_level[ch->class])
      {
         send_to_char( "You have not yet learned enough to fight aggressively.\n\r",ch);
         return;
      }
      WAIT_STATE( ch, skill_table[gsn_style_aggressive]->beats );
      if( number_percent() < LEARNED(ch, gsn_style_aggressive) ){
          /* success */
	  if(ch->fighting) {
             ch->position = POS_AGGRESSIVE;
             learn_from_success(ch,gsn_style_aggressive);
	      if ( IS_PKILL( ch ) )
		act( AT_ACTION, "$n assumes an aggressive stance.",
                  ch, NULL, NULL, TO_ROOM );
          }
          ch->style = STYLE_AGGRESSIVE;
          send_to_char( "You adopt an aggressive fighting style.\n\r",ch);
          return;
      } else {
          /* failure */ 
          send_to_char( "You nearly trip in a lame attempt to adopt an aggressive fighting style.\n\r",ch);
	  return;
      }
    } else if( !str_prefix(arg,"berserk")){
      if( ch->level < skill_table[gsn_style_berserk]->skill_level[ch->class])
      {
         send_to_char( "You have not yet learned enough to fight as a berserker.\n\r",ch);
         return;
      }
      WAIT_STATE( ch, skill_table[gsn_style_berserk]->beats );
      if( number_percent() < LEARNED(ch, gsn_style_berserk) ){
          /* success */
	  if(ch->fighting) {
             ch->position = POS_BERSERK;
             learn_from_success(ch,gsn_style_berserk);
	      if ( IS_PKILL( ch ) )
		act( AT_ACTION, "$n enters a wildly aggressive style.",
                  ch, NULL, NULL, TO_ROOM );
          }
          ch->style = STYLE_BERSERK;
          send_to_char( "You adopt a berserk fighting style.\n\r",ch);
          return;
      } else {
          /* failure */ 
          send_to_char( "You nearly trip in a lame attempt to adopt a berserk fighting style.\n\r",ch);
	  return;
      }
    }

    send_to_char( "Adopt which fighting style?\n\r",ch);

    return;
}

/*  New check to see if you can use skills to support morphs --Shaddai */
bool can_use_skill( CHAR_DATA *ch, int percent, int gsn )
{
   bool check = FALSE;
   if ( IS_NPC(ch) && percent < 85 ) check = TRUE;
   else if ( !IS_NPC(ch) && percent < LEARNED(ch, gsn)  ) check = TRUE;
   else if ( ch->morph && ch->morph->morph && ch->morph->morph->skills &&
             ch->morph->morph->skills[0] != '\0' &&
             is_name( skill_table[gsn]->name, ch->morph->morph->skills ) && 
	     percent < 85 )
                check = TRUE;
   if ( ch->morph && ch->morph->morph && ch->morph->morph->no_skills &&
             ch->morph->morph->no_skills[0] != '\0' &&
             is_name( skill_table[gsn]->name, ch->morph->morph->no_skills ) )
                check = FALSE;
   if ( skill_table[gsn]->guild == 99
   &&  !IS_NPC( ch )
   &&  !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
	check = FALSE;

   return check;
}

/*
 * Cook was coded by Blackmane and heavily modified by Shaddai
 */
void do_cook ( CHAR_DATA *ch, char *argument )
{
   OBJ_DATA *food, *fire;
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];

   one_argument( argument, arg );
   if ( IS_NPC(ch) || ch->level < skill_table[gsn_cook]->skill_level[ch->class])
   {
	send_to_char("That skill is beyond your understanding.\n\r", ch );
	return;
   }
   if ( arg[0] == '\0' )
   {	
	send_to_char("Cook what?\n\r", ch );
	return;
   }

   if ( ms_find_obj(ch) )
            return;

   if ( ( food = get_obj_carry( ch, arg ) ) == NULL )
   {
	send_to_char("You do not have that item.\n\r", ch );
	return;
   }
   if ( food->item_type != ITEM_COOK )
   {
	send_to_char("How can you cook that?\n\r", ch );
	return;
   }
   if ( food->value[2] > 2 )
   {
	send_to_char("That is already burnt to a crisp.\n\r", ch );
	return;
   }
   for ( fire = ch->in_room->first_content; fire; fire = fire->next_content )
   {
	if ( fire->item_type == ITEM_FIRE )
	   break;
   }
   if ( !fire )
   {
	send_to_char("There is no fire here!\n\r", ch );
	return;
   }
   separate_obj(food);	/* Bug catch by Tchaika from SMAUG list */
   if ( number_percent() > LEARNED(ch, gsn_cook)  )
   {
        food->timer = food->timer/2;
        food->value[0] = 0;
	food->value[2] = 3;
	act( AT_MAGIC, "$p catches on fire burning it to a crisp!\n\r",
		ch, food, NULL, TO_CHAR );
        act( AT_MAGIC, "$n catches $p on fire burning it to a crisp.",
		ch, food, NULL, TO_ROOM);
	sprintf( buf, "a burnt %s", food->pIndexData->name );
	STRFREE(food->short_descr);
	food->short_descr = STRALLOC(buf);
	sprintf( buf, "A burnt %s.", food->pIndexData->name);
        STRFREE(food->description);
        food->description = STRALLOC(buf);
        return;
   }

   if ( number_percent() > 85 )
   {
        food->timer = food->timer*3;
	food->value[2]+=2;
	act( AT_MAGIC, "$n overcooks $p.",ch, food, NULL, TO_ROOM);
        act( AT_MAGIC, "You overcook $p.",ch, food, NULL, TO_CHAR);
	sprintf( buf, "an overcooked %s", food->pIndexData->name );
	STRFREE(food->short_descr);
	food->short_descr = STRALLOC(buf);
	sprintf( buf, "An overcooked %s.", food->pIndexData->name);
        STRFREE(food->description);
        food->description = STRALLOC(buf);
   }
   else
   {
        food->timer = food->timer*4;
	food->value[0] *= 2;
        act( AT_MAGIC, "$n roasts $p.",ch, food, NULL, TO_ROOM);
        act( AT_MAGIC, "You roast $p.",ch, food, NULL, TO_CHAR);
	sprintf( buf, "a roasted %s", food->pIndexData->name );
	STRFREE(food->short_descr);
	food->short_descr = STRALLOC(buf);
	sprintf( buf, "A roasted %s.", food->pIndexData->name);
        STRFREE(food->description);
        food->description = STRALLOC(buf);
	food->value[2]++;
   }
   learn_from_success(ch, gsn_cook);
}

