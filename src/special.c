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
 *			   "Special procedure" module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

void	wear_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, sh_int wear_bit ) );




/*
 * The following special functions are available for mobiles.
 */
DECLARE_SPEC_FUN(	spec_breath_any		);
DECLARE_SPEC_FUN(	spec_breath_acid	);
DECLARE_SPEC_FUN(	spec_breath_fire	);
DECLARE_SPEC_FUN(	spec_breath_frost	);
DECLARE_SPEC_FUN(	spec_breath_gas		);
DECLARE_SPEC_FUN(	spec_breath_lightning	);
DECLARE_SPEC_FUN(	spec_cast_adept		);
DECLARE_SPEC_FUN(	spec_cast_cleric	);
DECLARE_SPEC_FUN(	spec_cast_mage		);
DECLARE_SPEC_FUN(	spec_cast_undead	);
DECLARE_SPEC_FUN(	spec_executioner	);
DECLARE_SPEC_FUN(	spec_fido		);
DECLARE_SPEC_FUN(	spec_guard		);
DECLARE_SPEC_FUN(	spec_janitor		);
DECLARE_SPEC_FUN(	spec_mayor		);
DECLARE_SPEC_FUN(	spec_poison		);
DECLARE_SPEC_FUN(	spec_thief		);
DECLARE_SPEC_FUN(	spec_wanderer		);



/*
 * Given a name, return the appropriate spec fun.
 */
SPEC_FUN *spec_lookup( const char *name )
{
    if ( !str_cmp( name, "spec_breath_any"	  ) ) return spec_breath_any;
    if ( !str_cmp( name, "spec_breath_acid"	  ) ) return spec_breath_acid;
    if ( !str_cmp( name, "spec_breath_fire"	  ) ) return spec_breath_fire;
    if ( !str_cmp( name, "spec_breath_frost"	  ) ) return spec_breath_frost;
    if ( !str_cmp( name, "spec_breath_gas"	  ) ) return spec_breath_gas;
    if ( !str_cmp( name, "spec_breath_lightning"  ) ) return
							spec_breath_lightning;
    if ( !str_cmp( name, "spec_cast_adept"	  ) ) return spec_cast_adept;
    if ( !str_cmp( name, "spec_cast_cleric"	  ) ) return spec_cast_cleric;
    if ( !str_cmp( name, "spec_cast_mage"	  ) ) return spec_cast_mage;
    if ( !str_cmp( name, "spec_cast_undead"	  ) ) return spec_cast_undead;
    if ( !str_cmp( name, "spec_executioner"	  ) ) return spec_executioner;
    if ( !str_cmp( name, "spec_fido"		  ) ) return spec_fido;
    if ( !str_cmp( name, "spec_guard"		  ) ) return spec_guard;
    if ( !str_cmp( name, "spec_janitor"		  ) ) return spec_janitor;
    if ( !str_cmp( name, "spec_mayor"		  ) ) return spec_mayor;
    if ( !str_cmp( name, "spec_poison"		  ) ) return spec_poison;
    if ( !str_cmp( name, "spec_thief"		  ) ) return spec_thief;
    if ( !str_cmp( name, "spec_wanderer"	  ) ) return spec_wanderer;
    return 0;
}

/*
 * Given a pointer, return the appropriate spec fun text.
 */
char *lookup_spec( SPEC_FUN *special )
{
    if ( special == spec_breath_any	)	return "spec_breath_any";
    
    if ( special == spec_breath_acid	)	return "spec_breath_acid";
    if ( special == spec_breath_fire	) 	return "spec_breath_fire";
    if ( special == spec_breath_frost	) 	return "spec_breath_frost";
    if ( special == spec_breath_gas	) 	return "spec_breath_gas";
    if ( special == spec_breath_lightning )	return "spec_breath_lightning";
    if ( special == spec_cast_adept	)	return "spec_cast_adept";
    if ( special == spec_cast_cleric	)	return "spec_cast_cleric";
    if ( special == spec_cast_mage	)	return "spec_cast_mage";
    if ( special == spec_cast_undead	)	return "spec_cast_undead";
    if ( special == spec_executioner	)	return "spec_executioner";
    if ( special == spec_fido		)	return "spec_fido";
    if ( special == spec_guard		)	return "spec_guard";
    if ( special == spec_janitor	)	return "spec_janitor";
    if ( special == spec_mayor		)	return "spec_mayor";
    if ( special == spec_poison		)	return "spec_poison";
    if ( special == spec_thief		)	return "spec_thief";
    if ( special == spec_wanderer	)	return "spec_wanderer";
    return "";
}


/* if a spell casting mob is hating someone... try and summon them */
void summon_if_hating( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    char name[MAX_INPUT_LENGTH];
    bool found = FALSE;

    if ( ch->position <= POS_SLEEPING )
	return;

    if ( ch->fighting || ch->fearing
    ||  !ch->hating || xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
      return;

    /* if player is close enough to hunt... don't summon */
    if ( ch->hunting )
      return;

    one_argument( ch->hating->name, name );

    /* make sure the char exists - works even if player quits */
    for (victim = first_char;
   	 victim;
   	 victim = victim->next)
    {
	if ( !str_cmp( ch->hating->name, victim->name ) )
	{
           found = TRUE;
	   break;
	}
    }

    if ( !found )
      return;
    if ( ch->in_room == victim->in_room )
      return;
    if ( !IS_NPC( victim ) )
      sprintf( buf, "summon 0.%s", name );
     else
      sprintf( buf, "summon %s", name );
    do_cast( ch, buf );
    return;
}






/*
 * Core procedure for dragons.
 */
bool dragon( CHAR_DATA *ch, char *spell_name )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    int sn;

    if ( ch->position != POS_FIGHTING
       && ch->position !=  POS_EVASIVE
       && ch->position !=  POS_DEFENSIVE
       && ch->position !=  POS_AGGRESSIVE
       && ch->position !=  POS_BERSERK
    )
	return FALSE;

    for ( victim = ch->in_room->first_person; victim; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( who_fighting( victim ) == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( !victim )
	return FALSE;

    if ( ( sn = skill_lookup( spell_name ) ) < 0 )
	return FALSE;
    (*skill_table[sn]->spell_fun) ( sn, ch->level, ch, victim );
    return TRUE;
}



/*
 * Special procedures for mobiles.
 */
bool spec_breath_any( CHAR_DATA *ch )
{
    if ( ch->position != POS_FIGHTING
       && ch->position !=  POS_EVASIVE
       && ch->position !=  POS_DEFENSIVE
       && ch->position !=  POS_AGGRESSIVE
       && ch->position !=  POS_BERSERK
    )
	return FALSE;

    switch ( number_bits( 3 ) )
    {
    case 0: return spec_breath_fire		( ch );
    case 1:
    case 2: return spec_breath_lightning	( ch );
    case 3: return spec_breath_gas		( ch );
    case 4: return spec_breath_acid		( ch );
    case 5:
    case 6:
    case 7: return spec_breath_frost		( ch );
    }

    return FALSE;
}



bool spec_breath_acid( CHAR_DATA *ch )
{
    return dragon( ch, "acid breath" );
}



bool spec_breath_fire( CHAR_DATA *ch )
{
    return dragon( ch, "fire breath" );
}



bool spec_breath_frost( CHAR_DATA *ch )
{
    return dragon( ch, "frost breath" );
}



bool spec_breath_gas( CHAR_DATA *ch )
{
    int sn;

    if ( ch->position != POS_FIGHTING
       && ch->position !=  POS_EVASIVE
       && ch->position !=  POS_DEFENSIVE
       && ch->position !=  POS_AGGRESSIVE
       && ch->position !=  POS_BERSERK
    )
	return FALSE;

    if ( ( sn = skill_lookup( "gas breath" ) ) < 0 )
	return FALSE;
    (*skill_table[sn]->spell_fun) ( sn, ch->level, ch, NULL );
    return TRUE;
}



bool spec_breath_lightning( CHAR_DATA *ch )
{
    return dragon( ch, "lightning breath" );
}



bool spec_cast_adept( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;

    if ( !IS_AWAKE(ch) || ch->fighting )
        return FALSE;

    for ( victim = ch->in_room->first_person; victim; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim != ch && can_see( ch, victim ) && number_bits( 1 ) == 0 )
	    break;
    }

    if ( !victim )
	return FALSE;

    switch ( number_bits( 3 ) )
    {
    case 0:
    act( AT_MAGIC, "$n utters the word 'ciroht'.", ch, NULL, NULL, TO_ROOM );
	spell_smaug( skill_lookup( "armor" ), ch->level, ch, victim );
	return TRUE;

    case 1:
    act( AT_MAGIC, "$n utters the word 'sunimod'.", ch, NULL, NULL, TO_ROOM );
	spell_smaug( skill_lookup( "bless" ), ch->level, ch, victim );
	return TRUE;

    case 2:
    act( AT_MAGIC, "$n utters the word 'suah'.", ch, NULL, NULL, TO_ROOM );
	spell_cure_blindness( skill_lookup( "cure blindness" ),
	    ch->level, ch, victim );
	return TRUE;

    case 3:
    act( AT_MAGIC, "$n utters the word 'nran'.", ch, NULL, NULL, TO_ROOM );
	spell_smaug( skill_lookup( "cure light" ),
	    ch->level, ch, victim );
	return TRUE;

    case 4:
    act( AT_MAGIC, "$n utters the word 'nyrcs'.", ch, NULL, NULL, TO_ROOM );
	spell_cure_poison( skill_lookup( "cure poison" ),
	    ch->level, ch, victim );
	return TRUE;

    case 5:
    act( AT_MAGIC, "$n utters the word 'gartla'.", ch, NULL, NULL, TO_ROOM );
	spell_smaug( skill_lookup( "refresh" ), ch->level, ch, victim );
	return TRUE;

    case 6:
    act( AT_MAGIC, "$n utters the word 'naimad'.", ch, NULL, NULL, TO_ROOM );
        spell_smaug( skill_lookup( "cure serious" ), ch->level, ch, victim );
        return TRUE;

    case 7:
    act( AT_MAGIC, "$n utters the word 'gorog'.", ch, NULL, NULL, TO_ROOM );
        spell_remove_curse( skill_lookup( "remove curse" ), ch->level, ch, victim );
        return TRUE;

    }

    return FALSE;
}



bool spec_cast_cleric( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    summon_if_hating( ch );

    if ( ch->position != POS_FIGHTING
       && ch->position !=  POS_EVASIVE
       && ch->position !=  POS_DEFENSIVE
       && ch->position !=  POS_AGGRESSIVE
       && ch->position !=  POS_BERSERK
    )
	return FALSE;

    for ( victim = ch->in_room->first_person; victim; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( who_fighting( victim ) == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( !victim || victim == ch )
	return FALSE;

    for ( ;; )
    {
	int min_level;

	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "cause light";    break;
	case  1: min_level =  3; spell = "cause serious";  break;
	case  2: min_level =  6; spell = "earthquake";     break;
	case  3: min_level =  7; spell = "blindness";	   break;
	case  4: min_level =  9; spell = "cause critical"; break;
	case  5: min_level = 10; spell = "dispel evil";    break;
	case  6: min_level = 12; spell = "curse";          break;
	case  7: min_level = 13; spell = "flamestrike";    break;
	case  8: 
	case  9:
	case 10: min_level = 15; spell = "harm";           break;
	default: min_level = 16; spell = "dispel magic";   break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn]->spell_fun) ( sn, ch->level, ch, victim );
    return TRUE;
}



bool spec_cast_mage( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    summon_if_hating( ch );

    if ( ch->position != POS_FIGHTING
       && ch->position !=  POS_EVASIVE
       && ch->position !=  POS_DEFENSIVE
       && ch->position !=  POS_AGGRESSIVE
       && ch->position !=  POS_BERSERK
    )
	return FALSE;

    for ( victim = ch->in_room->first_person; victim; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( who_fighting( victim ) && number_bits( 2 ) == 0 )
	    break;
    }

    if ( !victim || victim == ch )
	return FALSE;

    for ( ;; )
    {
	int min_level;

	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "black hand";	   break;
	case  1: min_level =  3; spell = "chill touch";    break;
	case  2: min_level =  7; spell = "weaken";         break;
	case  3: min_level =  8; spell = "galvanic whip";  break;
	case  4: min_level = 11; spell = "colour spray";   break;
	case  5: min_level = 12; spell = "weaken";	   break;
	case  6: min_level = 13; spell = "energy drain";   break;
	case  7: min_level = 14; spell = "spectral furor"; break;
	case  8:
	case  9: min_level = 15; spell = "fireball";       break;
	default: min_level = 20; spell = "acid blast";     break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn]->spell_fun) ( sn, ch->level, ch, victim );
    return TRUE;
}



bool spec_cast_undead( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    summon_if_hating( ch );

    if ( ch->position != POS_FIGHTING
       && ch->position !=  POS_EVASIVE
       && ch->position !=  POS_DEFENSIVE
       && ch->position !=  POS_AGGRESSIVE
       && ch->position !=  POS_BERSERK
    )
	return FALSE;

    for ( victim = ch->in_room->first_person; victim; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( who_fighting( victim ) == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( !victim || victim == ch )
	return FALSE;

    for ( ;; )
    {
	int min_level;

	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "chill touch";    break;
	case  1: min_level = 11; spell = "weaken";         break;
	case  2: min_level = 12; spell = "curse";          break;
	case  3: min_level = 13; spell = "blindness";      break;
	case  4: min_level = 14; spell = "poison";         break;
	case  5: min_level = 15; spell = "energy drain";   break;
	case  6: min_level = 18; spell = "harm";           break;
	default: min_level = 40; spell = "gate";           break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn]->spell_fun) ( sn, ch->level, ch, victim );
    return TRUE;
}



bool spec_executioner( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    MOB_INDEX_DATA *cityguard;
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *crime;

    if ( !IS_AWAKE(ch) || ch->fighting )
	return FALSE;

    crime = "";
    for ( victim = ch->in_room->first_person; victim; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( !IS_NPC(victim) && xIS_SET(victim->act, PLR_KILLER) )
	    { crime = "KILLER"; break; }

	if ( !IS_NPC(victim) && xIS_SET(victim->act, PLR_THIEF) )
	    { crime = "THIEF"; break; }
    }

    if ( !victim )
	return FALSE;

    if ( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
	sprintf( buf, "%s is a %s!  As well as a COWARD!",
		victim->name, crime );
	do_yell( ch, buf );
	return TRUE;
    }

    sprintf( buf, "%s is a %s!  PROTECT THE INNOCENT!  MORE BLOOOOD!!!",
	victim->name, crime );
    do_yell( ch, buf );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    if ( char_died(ch) )
      return TRUE;

    /* Added log in case of missing cityguard -- Tri */

    cityguard = get_mob_index( MOB_VNUM_CITYGUARD );

    if ( !cityguard )
    {
      sprintf( buf, "Missing Cityguard - Vnum:[%d]", MOB_VNUM_CITYGUARD );
      bug( buf, 0 );
      return TRUE;
    }

    char_to_room( create_mobile( cityguard ), ch->in_room );
    char_to_room( create_mobile( cityguard ), ch->in_room );
    return TRUE;
}



bool spec_fido( CHAR_DATA *ch )
{
    OBJ_DATA *corpse;
    OBJ_DATA *c_next;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( corpse = ch->in_room->first_content; corpse; corpse = c_next )
    {
	c_next = corpse->next_content;
	if ( corpse->item_type != ITEM_CORPSE_NPC )
	    continue;

    act( AT_ACTION, "$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM );
	for ( obj = corpse->first_content; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    obj_from_obj( obj );
	    obj_to_room( obj, ch->in_room );
	}
	extract_obj( corpse );
	return TRUE;
    }

    return FALSE;
}



bool spec_guard( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    CHAR_DATA *ech;
    char *crime;
    int max_evil;

    if ( !IS_AWAKE(ch) || ch->fighting )
	return FALSE;

    max_evil = 300;
    ech      = NULL;
    crime    = "";

    for ( victim = ch->in_room->first_person; victim; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( !IS_NPC(victim) && xIS_SET(victim->act, PLR_KILLER) )
	    { crime = "KILLER"; break; }

	if ( !IS_NPC(victim) && xIS_SET(victim->act, PLR_THIEF) )
	    { crime = "THIEF"; break; }

	if ( victim->fighting
	&&   who_fighting( victim ) != ch
	&&   victim->alignment < max_evil )
	{
	    max_evil = victim->alignment;
	    ech      = victim;
	}
    }

    if ( victim && xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
	sprintf( buf, "%s is a %s!  As well as a COWARD!",
		victim->name, crime );
	do_yell( ch, buf );
	return TRUE;
    }

    if ( victim )
    {
	sprintf( buf, "%s is a %s!  PROTECT THE INNOCENT!!  BANZAI!!",
		victim->name, crime );
	do_yell( ch, buf );
	multi_hit( ch, victim, TYPE_UNDEFINED );
	return TRUE;
    }

    if ( ech )
    {
    act( AT_YELL, "$n screams 'PROTECT THE INNOCENT!!  BANZAI!!",
	    ch, NULL, NULL, TO_ROOM );
	multi_hit( ch, ech, TYPE_UNDEFINED );
	return TRUE;
    }

    return FALSE;
}



bool spec_janitor( CHAR_DATA *ch )
{
    OBJ_DATA *trash;
    OBJ_DATA *trash_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( trash = ch->in_room->first_content; trash; trash = trash_next )
    {
	trash_next = trash->next_content;
	if ( !IS_SET( trash->wear_flags, ITEM_TAKE )
	||    IS_OBJ_STAT( trash, ITEM_BURIED ) )
	    continue;
	if ( trash->item_type == ITEM_DRINK_CON
	||   trash->item_type == ITEM_TRASH
	||   trash->cost < 10
	||  (trash->pIndexData->vnum == OBJ_VNUM_SHOPPING_BAG
	&&  !trash->first_content) )
	{
	    act( AT_ACTION, "$n picks up some trash.", ch, NULL, NULL, TO_ROOM );
	    obj_from_room( trash );
	    obj_to_char( trash, ch );
	    return TRUE;
	}
    }

    return FALSE;
}



bool spec_mayor( CHAR_DATA *ch )
{
    static const char open_path[] =
	"W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

    static const char close_path[] =
	"W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

    static const char *path;
    static int pos;
    static bool move;

    if ( !move )
    {
	if ( time_info.hour ==  6 )
	{
	    path = open_path;
	    move = TRUE;
	    pos  = 0;
	}

	if ( time_info.hour == 20 )
	{
	    path = close_path;
	    move = TRUE;
	    pos  = 0;
	}
    }

    if ( ch->fighting )
	return spec_cast_cleric( ch );
    if ( !move || ch->position < POS_SLEEPING )
	return FALSE;

    switch ( path[pos] )
    {
    case '0':
    case '1':
    case '2':
    case '3':
	move_char( ch, get_exit( ch->in_room, path[pos] - '0' ), 0 );
	break;

    case 'W':
	ch->position = POS_STANDING;
    act( AT_ACTION, "$n awakens and groans loudly.", ch, NULL, NULL, TO_ROOM );
	break;

    case 'S':
	ch->position = POS_SLEEPING;
    act( AT_ACTION, "$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM );
	break;

    case 'a':
    act( AT_SAY, "$n says 'Hello Honey!'", ch, NULL, NULL, TO_ROOM );
	break;

    case 'b':
    act( AT_SAY, "$n says 'What a view!  I must do something about that dump!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'c':
    act( AT_SAY, "$n says 'Vandals!  Youngsters have no respect for anything!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'd':
    act( AT_SAY, "$n says 'Good day, citizens!'", ch, NULL, NULL, TO_ROOM );
	break;

    case 'e':
    act( AT_SAY, "$n says 'I hereby declare the town of Darkhaven open!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'E':
    act( AT_SAY, "$n says 'I hereby declare the town of Darkhaven closed!'",
	    ch, NULL, NULL, TO_ROOM );
	break;

    case 'O':
	do_unlock( ch, "gate" );
	do_open( ch, "gate" );
	break;

    case 'C':
	do_close( ch, "gate" );
	do_lock( ch, "gate" );
	break;

    case '.' :
	move = FALSE;
	break;
    }

    pos++;
    return FALSE;
}



bool spec_poison( CHAR_DATA *ch )
{
    CHAR_DATA *victim;

    if ( ch->position != POS_FIGHTING
       && ch->position !=  POS_EVASIVE
       && ch->position !=  POS_DEFENSIVE
       && ch->position !=  POS_AGGRESSIVE
       && ch->position !=  POS_BERSERK
    )
	return FALSE;

   if ( ( victim = who_fighting( ch ) ) == NULL
    ||   number_percent( ) > 2 * ch->level )
	return FALSE;

    act( AT_HIT, "You bite $N!",  ch, NULL, victim, TO_CHAR    );
    act( AT_ACTION, "$n bites $N!",  ch, NULL, victim, TO_NOTVICT );
    act( AT_POISON, "$n bites you!", ch, NULL, victim, TO_VICT    );
    spell_poison( gsn_poison, ch->level, ch, victim );
    return TRUE;
}



bool spec_thief( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    int gold, maxgold;

    if ( ch->position != POS_STANDING )
	return FALSE;

    for ( victim = ch->in_room->first_person; victim; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( IS_NPC(victim)
	||   victim->level >= LEVEL_IMMORTAL
	||   number_bits( 2 ) != 0
	||   !can_see( ch, victim ) )	/* Thx Glop */
	    continue;

	if ( IS_AWAKE(victim) && number_range( 0, ch->level ) == 0 )
	{
	    act( AT_ACTION, "You discover $n's hands in your sack of gold!",
		ch, NULL, victim, TO_VICT );
	    act( AT_ACTION, "$N discovers $n's hands in $S sack of gold!",
		ch, NULL, victim, TO_NOTVICT );
	    return TRUE;
	}
	else
	{
	    maxgold = ch->level * ch->level * 1000;
	    gold = victim->gold
	    	 * number_range( 1, URANGE(2, ch->level/4, 10) ) / 100;
	    ch->gold     += 9 * gold / 10;
	    victim->gold -= gold;
	    if ( ch->gold > maxgold )
	    {
		boost_economy( ch->in_room->area, ch->gold - maxgold/2 );
		ch->gold = maxgold/2;
	    }
	    return TRUE;
	}
    }

    return FALSE;
}


/*****
 * A wanderer (nomad) that arm's itself and discards what it doesnt.. Kinda fun
 * to watch. I wrote this for an area that repops alot of armor on the ground
 * Adds a little spice to the area. Basically a modified janitor. Started off as
 * an easy project. Thanks to Mark Zagorski <mwz0615@ksu.edu> for pointing out I 
 * had a reduntancy if I had two mobs that in rooms that only led to each other
 * they would throw the piece of armor back and forth. He also suggested 
 * damaging the armor when thrown. 
 *****/
 
bool spec_wanderer( CHAR_DATA *ch )
{
    OBJ_DATA *trash;
    OBJ_DATA *trash_next;
    OBJ_DATA *obj2;
    ROOM_INDEX_DATA *was_in_room;
    EXIT_DATA *pexit = NULL;
    CHAR_DATA *vch;
    int door;
    int chance=50;
    bool found = FALSE; /* Valid direction */
    bool thrown = FALSE;/* Whether to be thrown or not */
    bool noexit = TRUE; /* Assume there is no valid exits */
    
    
    was_in_room = ch->in_room;
    if ( !IS_AWAKE(ch) )
	return FALSE;

    if((pexit = ch->in_room->first_exit) !=NULL)
			noexit=FALSE;
    
    if(chance>number_percent()){
    /****
     * Look for objects on the ground and pick it up
     ****/
    
    for ( trash = ch->in_room->first_content; trash; trash = trash_next )
    {
	trash_next = trash->next_content;
	if ( !IS_SET( trash->wear_flags, ITEM_TAKE )
	||    IS_OBJ_STAT( trash, ITEM_BURIED ) )
	    continue;
	
	if ( trash->item_type == ITEM_WEAPON
	|| 	 trash->item_type == ITEM_ARMOR
	||   trash->item_type == ITEM_LIGHT ) {
	separate_obj( trash );  /* So there is no 'sword <6>' gets only one object off ground */
	act( AT_ACTION, "$n leans over and gets $p.", ch, trash, NULL, TO_ROOM );	
	obj_from_room( trash );
	trash = obj_to_char( trash, ch );
	
	/*****
	 * If object is too high a level throw it away.
	 *****/
    if ( ch->level < trash->level )
    {
	act( AT_ACTION, "$n tries to use $p, but is too inexperienced.",ch, trash, NULL, TO_ROOM );
        thrown = TRUE;
    }
    
	/*****
	 * Wear the object if it is not to be thrown. The FALSE is passed
	 * so that the mob wont remove a piece of armor already there
	 * if it is not worn it is assumed that they can't use it or 
	 * they already are wearing something.
	 *****/
	
	if(!thrown)
	wear_obj( ch, trash, FALSE, -1 );
	
	/*****
	 * Look for an object in the inventory that is not being worn
	 * then throw it away...
	 *****/
	found=FALSE;
	if(!thrown)
        for ( obj2 = ch->first_carrying; obj2; obj2 = obj2->next_content ){
			if (obj2->wear_loc == WEAR_NONE){
			do_say(ch,"Hmm, I can't use this.");			
			trash=obj2; 
			thrown=TRUE;
	 }
	}
	/*****
	 * Ugly bit of code..
	 * Checks if the object is to be thrown & there is a valid exit, 
	 * randomly pick a direction to throw it, and check to make sure no other
	 * spec_wanderer mobs are in that room.
	 *****/
	if(thrown && !noexit)
	  while(!found && !noexit){
		door=number_door();
		if((pexit = get_exit(ch->in_room,door) ) != NULL
		&&   pexit->to_room
		&&   !IS_SET(pexit->exit_info, EX_CLOSED)
		&&	!xIS_SET(pexit->to_room->room_flags, ROOM_NODROP)){
		  if( (vch = pexit->to_room->first_person) !=NULL)
			for( vch = pexit->to_room->first_person; vch; vch = vch->next_in_room){
			  if (!str_cmp(lookup_spec(vch->spec_fun), "spec_wanderer")){
			  noexit = TRUE;
			  /*act( AT_CYAN,"$n spec_wanderer inroom $T", ch, NULL,dir_name[pexit->vdir], TO_ROOM);*/
			  return FALSE;
			  }
			}
			found = TRUE;
		}
	  }
		if (!noexit && thrown){
		/*if (trash->value*/
		set_cur_obj( trash );
		if( damage_obj( trash ) != rOBJ_SCRAPPED){
		separate_obj(trash);
		act( AT_ACTION, "$n growls and throws $p $T.", ch, trash, dir_name[pexit->vdir], TO_ROOM );
		obj_from_char( trash );
		obj_to_room( trash, pexit->to_room );
		char_from_room(ch);
		char_to_room(ch, pexit->to_room);
		act( AT_CYAN,"$p thrown by $n lands in the room.",ch, trash, ch, TO_ROOM);
		char_from_room(ch);
		char_to_room(ch, was_in_room);
		}
		else{
		do_say(ch,"This thing is junk!");
		act( AT_ACTION, "$n growls and breaks $p.",ch, trash, NULL, TO_ROOM);
		}
	
	return TRUE;
	}
	return TRUE;
    }
   } /* get next obj */
    return FALSE; /* No objects :< */
   }
   return FALSE;
}
