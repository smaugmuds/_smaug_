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
 *			     Spell handling module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#ifdef sun
  #include <strings.h>
#endif
#include <time.h>
#include "mud.h"



/*
 * Local functions.
 */
void	say_spell	args( ( CHAR_DATA *ch, int sn ) );
/*
CHAR_DATA *make_poly_mob args( (CHAR_DATA *ch, int vnum) );
*/
ch_ret	spell_affect	args( ( int sn, int level, CHAR_DATA *ch, void *vo ) );
ch_ret	spell_affectchar args( ( int sn, int level, CHAR_DATA *ch, void *vo ) );
int dispel_casting(AFFECT_DATA *paf,CHAR_DATA *ch,CHAR_DATA*victim,int affect,bool dispel);
bool can_charm ( CHAR_DATA *ch );

/*
 * Is immune to a damage type
 */
bool is_immune( CHAR_DATA *ch, sh_int damtype )
{
    switch( damtype )
    {
	case SD_FIRE:	     if (IS_SET(ch->immune, RIS_FIRE))	 return TRUE;
	case SD_COLD:	     if (IS_SET(ch->immune, RIS_COLD))	 return TRUE;
	case SD_ELECTRICITY: if (IS_SET(ch->immune, RIS_ELECTRICITY)) return TRUE;
	case SD_ENERGY:	     if (IS_SET(ch->immune, RIS_ENERGY)) return TRUE;
	case SD_ACID:	     if (IS_SET(ch->immune, RIS_ACID))	 return TRUE;
	case SD_POISON:	     if (IS_SET(ch->immune, RIS_POISON)) return TRUE;
	case SD_DRAIN:	     if (IS_SET(ch->immune, RIS_DRAIN))	 return TRUE;
    }
    return FALSE;
}

                            

bool can_astral_room( ROOM_INDEX_DATA *room )
{
    if ( xIS_SET(room->room_flags, ROOM_PRIVATE)
    ||   xIS_SET(room->room_flags, ROOM_SOLITARY)
    ||   xIS_SET(room->room_flags, ROOM_NO_ASTRAL)
    ||   xIS_SET(room->room_flags, ROOM_DEATH)
    ||   xIS_SET(room->room_flags, ROOM_NO_RECALL)
    ||   xIS_SET(room->room_flags, ROOM_PROTOTYPE)
    ||   IS_SET(room->area->flags, AFLAG_NOPORTALIN)
    ||   IS_SET(room->area->flags, AFLAG_NOTELEPORT) )
    return FALSE;

    
    return TRUE;
}

/*
 * Lookup a skill by name, only stopping at skills the player has.
 */
int ch_slookup( CHAR_DATA *ch, const char *name )
{
    int sn;

    if ( IS_NPC(ch) )
	return skill_lookup( name );
    for ( sn = 0; sn < top_sn; sn++ )
    {
	if ( !skill_table[sn]->name )
	    break;
	if ( ch->pcdata->learned[sn] > 0
	&&   ch->level >= skill_table[sn]->skill_level[ch->class]
	&&   LOWER(name[0]) == LOWER(skill_table[sn]->name[0])
	&&  !str_prefix( name, skill_table[sn]->name ) )
	    return sn;
    }

    return -1;
}

/*
 * Lookup an herb by name.
 */
int herb_lookup( const char *name )
{
    int sn;

    for ( sn = 0; sn < top_herb; sn++ )
    {
	if ( !herb_table[sn] || !herb_table[sn]->name )
	    return -1;
	if ( LOWER(name[0]) == LOWER(herb_table[sn]->name[0])
	&&  !str_prefix( name, herb_table[sn]->name ) )
	    return sn;
    }
    return -1;
}

/*
 * Lookup a personal skill
 * Unused for now.  In place to allow a player to have a custom spell/skill.
 * When this is put in make sure you put in cleanup code if you do any
 * sort of allocating memory in free_char --Shaddai
 */
int personal_lookup( CHAR_DATA *ch, const char *name )
{
    int sn;

    if ( !ch->pcdata )
	return -1;
    for ( sn = 0; sn < MAX_PERSONAL; sn++ )
    {
	if ( !ch->pcdata->special_skills[sn]
	||   !ch->pcdata->special_skills[sn]->name )
	    return -1;
	if ( LOWER(name[0]) == LOWER(ch->pcdata->special_skills[sn]->name[0])
	&&  !str_prefix( name, ch->pcdata->special_skills[sn]->name ) )
	    return sn;
    }
    return -1;
}

/*
 * Lookup a skill by name.
 */
int skill_lookup( const char *name )
{
    int sn;

    if ( (sn=bsearch_skill_exact(name, gsn_first_spell, gsn_first_skill-1)) == -1 )
      if ( (sn=bsearch_skill_exact(name, gsn_first_skill, gsn_first_weapon-1)) == -1 )
	if ( (sn=bsearch_skill_exact(name, gsn_first_weapon, gsn_first_tongue-1)) == -1 )
	  if ( (sn=bsearch_skill_exact(name, gsn_first_tongue, gsn_top_sn-1)) == -1 )
	    if ( (sn=bsearch_skill_prefix(name, gsn_first_spell, gsn_first_skill-1)) == -1 )
	      if ( (sn=bsearch_skill_prefix(name, gsn_first_skill, gsn_first_weapon-1)) == -1 )
		if ( (sn=bsearch_skill_prefix(name, gsn_first_weapon, gsn_first_tongue-1)) == -1 )
		  if ( (sn=bsearch_skill_prefix(name, gsn_first_tongue, gsn_top_sn-1)) == -1
		  &&    gsn_top_sn < top_sn )
		  {
		      for ( sn = gsn_top_sn; sn < top_sn; sn++ )
		      {
			  if ( !skill_table[sn] || !skill_table[sn]->name )
			    return -1;
			  if ( LOWER(name[0]) == LOWER(skill_table[sn]->name[0])
			  &&  !str_prefix( name, skill_table[sn]->name ) )
			    return sn;
		      }
		      return -1;
		  }
    return sn;
}

/*
 * Return a skilltype pointer based on sn			-Thoric
 * Returns NULL if bad, unused or personal sn.
 */
SKILLTYPE *get_skilltype( int sn )
{
    if ( sn >= TYPE_PERSONAL )
	return NULL;
    if ( sn >= TYPE_HERB )
	return IS_VALID_HERB(sn-TYPE_HERB) ? herb_table[sn-TYPE_HERB] : NULL;
    if ( sn >= TYPE_HIT )
	return NULL;
    return IS_VALID_SN(sn) ? skill_table[sn] : NULL;
}

/*
 * Perform a binary search on a section of the skill table	-Thoric
 * Each different section of the skill table is sorted alphabetically
 *
 * Check for prefix matches
 */
int bsearch_skill_prefix( const char *name, int first, int top )
{
    int sn;

    for (;;)
    {
	sn = (first + top) >> 1;
	if (!IS_VALID_SN(sn))
	    return -1;
	if (LOWER(name[0]) == LOWER(skill_table[sn]->name[0])
	&&  !str_prefix(name, skill_table[sn]->name) )
	    return sn;
	if (first >= top)
	    return -1;
    	if (strcmp(name, skill_table[sn]->name) < 1)
	    top = sn - 1;
    	else
	    first = sn + 1;
    }
    return -1;
}

/*
 * Perform a binary search on a section of the skill table	-Thoric
 * Each different section of the skill table is sorted alphabetically
 *
 * Check for exact matches only
 */
int bsearch_skill_exact( const char *name, int first, int top )
{
    int sn;

    for (;;)
    {
	sn = (first + top) >> 1;
	if (!IS_VALID_SN(sn))
	    return -1;
	if (!str_cmp(name, skill_table[sn]->name) )
	    return sn;
	if (first >= top)
	    return -1;
    	if (strcmp(name, skill_table[sn]->name) < 1)
	    top = sn - 1;
    	else
	    first = sn + 1;
    }
    return -1;
}

/*
 * Perform a binary search on a section of the skill table	-Thoric
 * Each different section of the skill table is sorted alphabetically
 *
 * Check exact match first, then a prefix match
 */
int bsearch_skill( const char *name, int first, int top )
{
    int sn = bsearch_skill_exact(name, first, top);

    return (sn == -1) ? bsearch_skill_prefix(name, first, top) : sn;
}

/*
 * Perform a binary search on a section of the skill table
 * Each different section of the skill table is sorted alphabetically
 * Only match skills player knows				-Thoric
 */
int ch_bsearch_skill_prefix( CHAR_DATA *ch, const char *name, int first, int top )
{
    int sn;

    for (;;)
    {
	sn = (first + top) >> 1;

	if ( LOWER(name[0]) == LOWER(skill_table[sn]->name[0])
	&&  (!str_prefix(name, skill_table[sn]->name)
	||  (sysdata.magichell && ch->level > 35 && number_percent() < 30 ))
	&&   ch->pcdata->learned[sn] > 0
	&&   ch->level >= skill_table[sn]->skill_level[ch->class] )
		return sn;
	if (first >= top)
	    return -1;
    	if (strcmp( name, skill_table[sn]->name) < 1)
	    top = sn - 1;
    	else
	    first = sn + 1;
    }
    return -1;
}

int ch_bsearch_skill_exact( CHAR_DATA *ch, const char *name, int first, int top )
{
    int sn;

    for (;;)
    {
	sn = (first + top) >> 1;

	if ((!str_cmp(name, skill_table[sn]->name)
	||   (sysdata.magichell && ch->level > 35 && number_percent() < 30 ))
	&&   ch->pcdata->learned[sn] > 0
	&&   ch->level >= skill_table[sn]->skill_level[ch->class] )
		return sn;
	if (first >= top)
	    return -1;
    	if (strcmp(name, skill_table[sn]->name) < 1)
	    top = sn - 1;
    	else
	    first = sn + 1;
    }
    return -1;
}

int ch_bsearch_skill( CHAR_DATA *ch, const char *name, int first, int top )
{
    int sn = ch_bsearch_skill_exact(ch, name, first, top);

    return (sn == -1) ? ch_bsearch_skill_prefix(ch, name, first, top) : sn;
}

int find_spell( CHAR_DATA *ch, const char *name, bool know )
{
    if ( IS_NPC(ch) || !know )
	return bsearch_skill( name, gsn_first_spell, gsn_first_skill-1 );
    else
	return ch_bsearch_skill( ch, name, gsn_first_spell, gsn_first_skill-1 );
}

int find_skill( CHAR_DATA *ch, const char *name, bool know )
{
    if ( IS_NPC(ch) || !know )
	return bsearch_skill( name, gsn_first_skill, gsn_first_weapon-1 );
    else
	return ch_bsearch_skill( ch, name, gsn_first_skill, gsn_first_weapon-1 );
}

int find_weapon( CHAR_DATA *ch, const char *name, bool know )
{
    if ( IS_NPC(ch) || !know )
	return bsearch_skill( name, gsn_first_weapon, gsn_first_tongue-1 );
    else
	return ch_bsearch_skill( ch, name, gsn_first_weapon, gsn_first_tongue-1 );
}

int find_tongue( CHAR_DATA *ch, const char *name, bool know )
{
    if ( IS_NPC(ch) || !know )
	return bsearch_skill( name, gsn_first_tongue, gsn_top_sn-1 );
    else
	return ch_bsearch_skill( ch, name, gsn_first_tongue, gsn_top_sn-1 );
}


/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int slot_lookup( int slot )
{
    extern bool fBootDb;
    int sn;

    if ( slot <= 0 )
	return -1;

    for ( sn = 0; sn < top_sn; sn++ )
	if ( slot == skill_table[sn]->slot )
	    return sn;

    if ( fBootDb )
    {
	bug( "Slot_lookup: bad slot %d.", slot );
	abort( );
    }

    return -1;
}

/*
 * Handler to tell the victim which spell is being affected.
 * Shaddai
 */
int
dispel_casting (AFFECT_DATA *paf,CHAR_DATA *ch,CHAR_DATA *victim,int affect, bool dispel)
{
        char      buf[MAX_STRING_LENGTH];
        char      *spell;
        SKILLTYPE *sktmp;
        bool is_mage = FALSE, has_detect = FALSE;
	EXT_BV ext_bv = meb(affect);

        if ( IS_NPC ( ch ) || ch->class == CLASS_MAGE )
                is_mage = TRUE;
        if ( IS_AFFECTED (ch, AFF_DETECT_MAGIC) )
                has_detect = TRUE;

        if ( paf )
        {
                if ((sktmp = get_skilltype (paf->type)) == NULL)
                        return 0;
                spell = sktmp->name;
        }
        else
                spell = affect_bit_name ( &ext_bv );

        set_char_color ( AT_MAGIC, ch );
        set_char_color ( AT_HITME, victim );

        if ( !can_see ( ch, victim ) )
           strcpy( buf, "Someone" );
        else
        {
           strcpy(buf,(IS_NPC(victim)?victim->short_descr:victim->name));
           buf[0] = toupper( buf[0] );
        }

        if ( dispel )
        {
                ch_printf (victim, "Your %s vanishes.\n\r", spell );
                if ( is_mage && has_detect )
                        ch_printf ( ch, "%s's %s vanishes.\n\r",
                            buf, spell );
                else
                    return 0; /* So we give the default Ok. Message */
        }
        else
        {
                if ( is_mage && has_detect )
                        ch_printf (ch, "%s's %s wavers but holds.\n\r",
                            buf, spell );
                else
                   return 0; /* The wonderful Failed. Message */
        }
        return 1;
}

/*
 * Fancy message handling for a successful casting		-Thoric
 */
void successful_casting( SKILLTYPE *skill, CHAR_DATA *ch,
			 CHAR_DATA *victim, OBJ_DATA *obj )
{
    sh_int chitroom = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_ACTION);
    sh_int chit	    = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_HIT);
    sh_int chitme   = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_HITME);

    if ( skill->target != TAR_CHAR_OFFENSIVE )
    {
	chit = chitroom;
	chitme = chitroom;
    }

    if ( ch && ch != victim )
    {
	if ( skill->hit_char && skill->hit_char[0] != '\0' )
	{
	    if ( str_cmp(skill->hit_char, SPELL_SILENT_MARKER) )
		act( AT_COLORIZE, skill->hit_char, ch, obj, victim, TO_CHAR );
	}
	else
	if ( skill->type == SKILL_SPELL )
            act( AT_COLORIZE, "Ok.", ch, NULL, NULL, TO_CHAR );
    }
    if ( ch && skill->hit_room && skill->hit_room[0] != '\0' &&
         str_cmp(skill->hit_room, SPELL_SILENT_MARKER) )
	act( AT_COLORIZE, skill->hit_room, ch, obj, victim, TO_NOTVICT );
    if ( ch && victim && skill->hit_vict && skill->hit_vict[0] != '\0' )
    {
	if ( str_cmp(skill->hit_vict, SPELL_SILENT_MARKER) )
	{
	    if ( ch != victim )
		act( AT_COLORIZE, skill->hit_vict, ch, obj, victim, TO_VICT );
	    else
		act( AT_COLORIZE, skill->hit_vict, ch, obj, victim, TO_CHAR );
	}
    }
    else
    if ( ch && ch == victim && skill->type == SKILL_SPELL )
	act( AT_COLORIZE, "Ok.", ch, NULL, NULL, TO_CHAR );
}

/*
 * Fancy message handling for a failed casting			-Thoric
 */
void failed_casting( SKILLTYPE *skill, CHAR_DATA *ch,
		     CHAR_DATA *victim, OBJ_DATA *obj )
{
    sh_int chitroom = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_ACTION);
    sh_int chit	    = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_HIT);
    sh_int chitme   = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_HITME);

    if ( skill->target != TAR_CHAR_OFFENSIVE )
    {
	chit = chitroom;
	chitme = chitroom;
    }

    if ( ch && ch != victim )
    {
	if ( skill->miss_char && skill->miss_char[0] != '\0' )
	{
	    if ( str_cmp(skill->miss_char, SPELL_SILENT_MARKER) )
		act( AT_COLORIZE, skill->miss_char, ch, obj, victim, TO_CHAR );
	}
	else
	if ( skill->type == SKILL_SPELL )
            act( chitme, "You failed.", ch, NULL, NULL, TO_CHAR );
    }
    if ( ch && skill->miss_room && skill->miss_room[0] != '\0' &&
         str_cmp(skill->miss_room, SPELL_SILENT_MARKER)  &&
         str_cmp(skill->miss_room, "supress") ) /* Back Compat -- Alty */
	act( AT_COLORIZE, skill->miss_room, ch, obj, victim, TO_NOTVICT );
    if ( ch && victim && skill->miss_vict && skill->miss_vict[0] != '\0' )
    {
	if ( str_cmp(skill->miss_vict, SPELL_SILENT_MARKER) )
	{
	    if ( ch != victim )
		act( AT_COLORIZE, skill->miss_vict, ch, obj, victim, TO_VICT );
	    else
		act( AT_COLORIZE, skill->miss_vict, ch, obj, victim, TO_CHAR );
	}
    }
    else
    if ( ch && ch == victim )
    {
	if ( skill->miss_char && skill->miss_char[0] != '\0' )
	{
	    if ( str_cmp(skill->miss_char, SPELL_SILENT_MARKER) )
		act( AT_COLORIZE, skill->miss_char, ch, obj, victim, TO_CHAR );
	}
	else
	if ( skill->type == SKILL_SPELL )
	    act( chitme, "You failed.", ch, NULL, NULL, TO_CHAR );
    }
}

/*
 * Fancy message handling for being immune to something		-Thoric
 */
void immune_casting( SKILLTYPE *skill, CHAR_DATA *ch,
		     CHAR_DATA *victim, OBJ_DATA *obj )
{
    sh_int chitroom = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_ACTION);
    sh_int chit	    = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_HIT);
    sh_int chitme   = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_HITME);

    if ( skill->target != TAR_CHAR_OFFENSIVE )
    {
	chit = chitroom;
	chitme = chitroom;
    }

    if ( ch && ch != victim )
    {
	if ( skill->imm_char && skill->imm_char[0] != '\0' )
	{
	    if ( str_cmp(skill->imm_char, SPELL_SILENT_MARKER) )
		act( AT_COLORIZE, skill->imm_char, ch, obj, victim, TO_CHAR );
	}
	else
	if ( skill->miss_char && skill->miss_char[0] != '\0' )
	{
	    if ( str_cmp(skill->miss_char, SPELL_SILENT_MARKER) )
		act( AT_COLORIZE, skill->hit_char, ch, obj, victim, TO_CHAR );
	}
	else
	if ( skill->type == SKILL_SPELL || skill->type == SKILL_SKILL )
            act( chit, "That appears to have no effect.", ch, NULL, NULL, TO_CHAR );
    }
    if ( ch && skill->imm_room && skill->imm_room[0] != '\0' )
    {
	if ( str_cmp(skill->imm_room, SPELL_SILENT_MARKER) )
	    act( AT_COLORIZE, skill->imm_room, ch, obj, victim, TO_NOTVICT );
    }
    else
    if ( ch && skill->miss_room && skill->miss_room[0] != '\0' )
    {
	if ( str_cmp(skill->miss_room, SPELL_SILENT_MARKER) )
	    act( AT_COLORIZE, skill->miss_room, ch, obj, victim, TO_NOTVICT );
    }
    if ( ch && victim && skill->imm_vict && skill->imm_vict[0] != '\0' )
    {
	if ( str_cmp(skill->imm_vict, SPELL_SILENT_MARKER) )
	{
	    if ( ch != victim )
		act( AT_COLORIZE, skill->imm_vict, ch, obj, victim, TO_VICT );
	    else
		act( AT_COLORIZE, skill->imm_vict, ch, obj, victim, TO_CHAR );
	}
    }
    else
    if ( ch && victim && skill->miss_vict && skill->miss_vict[0] != '\0' )
    {
	if ( str_cmp(skill->miss_vict, SPELL_SILENT_MARKER) )
	{
	    if ( ch != victim )
		act( AT_COLORIZE, skill->miss_vict, ch, obj, victim, TO_VICT );
	    else
		act( AT_COLORIZE, skill->miss_vict, ch, obj, victim, TO_CHAR );
	}
    }
    else
    if ( ch && ch == victim )
    {
	if ( skill->imm_char && skill->imm_char[0] != '\0' )
	{
	    if ( str_cmp(skill->imm_char, SPELL_SILENT_MARKER) )
		act( AT_COLORIZE, skill->imm_char, ch, obj, victim, TO_CHAR );
	}
	else
	if ( skill->miss_char && skill->miss_char[0] != '\0' )
	{
	    if ( str_cmp(skill->hit_char, SPELL_SILENT_MARKER) )
		act( AT_COLORIZE, skill->hit_char, ch, obj, victim, TO_CHAR );
	}
	else
	if ( skill->type == SKILL_SPELL || skill->type == SKILL_SKILL )
            act( chit, "That appears to have no affect.", ch, NULL, NULL, TO_CHAR );
    }
}


/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA *ch, int sn )
{
    char buf  [MAX_STRING_LENGTH];
    char buf2 [MAX_STRING_LENGTH];
    CHAR_DATA *rch;
    char *pName;
    int iSyl;
    int length;
    SKILLTYPE *skill = get_skilltype( sn );

    struct syl_type
    {
	char *	old;
	char *	new;
    };

    static const struct syl_type syl_table[] =
    {
	{ " ",		" "		},
	{ "ar",		"abra"		},
	{ "au",		"kada"		},
	{ "bless",	"fido"		},
	{ "blind",	"nose"		},
	{ "bur",	"mosa"		},
	{ "cu",		"judi"		},
	{ "de",		"oculo"		},
	{ "en",		"unso"		},
	{ "light",	"dies"		},
	{ "lo",		"hi"		},
	{ "mor",	"zak"		},
	{ "move",	"sido"		},
	{ "ness",	"lacri"		},
	{ "ning",	"illa"		},
	{ "per",	"duda"		},
	{ "polymorph",  "iaddahs"	},
	{ "ra",		"gru"		},
	{ "re",		"candus"	},
	{ "son",	"sabru"		},
	{ "tect",	"infra"		},
	{ "tri",	"cula"		},
	{ "ven",	"nofo"		},
	{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
	{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
	{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
	{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
	{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
	{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
	{ "y", "l" }, { "z", "k" },
	{ "", "" }
    };

    buf[0]	= '\0';
    for ( pName = skill->name; *pName != '\0'; pName += length )
    {
	for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++ )
	{
	    if ( !str_prefix( syl_table[iSyl].old, pName ) )
	    {
		strcat( buf, syl_table[iSyl].new );
		break;
	    }
	}

	if ( length == 0 )
	    length = 1;
    }

    sprintf( buf2, "$n utters the words, '%s'.", buf );
    sprintf( buf,  "$n utters the words, '%s'.", skill->name );

    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
    {
	if ( rch != ch )
	act( AT_MAGIC, ch->class==rch->class ? buf : buf2,
	     ch, NULL, rch, TO_VICT );
    }

    return;
}


/*
 * Make adjustments to saving throw based in RIS		-Thoric
 */
int ris_save( CHAR_DATA *ch, int chance, int ris )
{
   sh_int modifier;

   modifier = 10;
   if (IS_SET (ch->immune, ris) || IS_SET (ch->stance_immune, ris))
     modifier -= 10;
   if (IS_SET (ch->resistant, ris) || IS_SET (ch->stance_immune, ris))
     modifier -= 2;
   if ( IS_SET(ch->susceptible, ris ) || IS_SET ( ch->stance_immune, ris))
   {
	if ( IS_NPC( ch )
	&&   IS_SET( ch->immune, ris ) )
	  modifier += 0;
	else
	  modifier += 2;
   }
   if ( modifier <= 0 )
	return 1000;
   if ( modifier == 10 )
	return chance;
   return (chance * modifier) / 10;
}


/*								    -Thoric
 * Fancy dice expression parsing complete with order of operations,
 * simple exponent support, dice support as well as a few extra
 * variables: L = level, H = hp, M = mana, V = move, S = str, X = dex
 *            I = int, W = wis, C = con, A = cha, U = luck, A = age
 *
 * Used for spell dice parsing, ie: 3d8+L-6
 *
 */
int rd_parse(CHAR_DATA *ch, int level, char *exp)
{
    int x, lop = 0, gop = 0, eop = 0;
    char operation;
    char *sexp[2];
    int total = 0, len = 0;

    /* take care of nulls coming in */
    if (!exp || !strlen(exp))
	return 0;

    /* get rid of brackets if they surround the entire expresion */
    if ( (*exp == '(') && !index(exp+1,'(') && exp[strlen(exp)-1] == ')' )
    {
	exp[strlen(exp)-1] = '\0';
	exp++;
    }

    /* check if the expresion is just a number */
    len = strlen(exp);
    if ( len == 1 && isalpha(exp[0]) )
    {
	switch(exp[0])
	{
	    case 'L': case 'l':	return level;
	    case 'H': case 'h':	return ch->hit;
	    case 'M': case 'm':	return ch->mana;
	    case 'V': case 'v':	return ch->move;
	    case 'S': case 's':	return get_curr_str(ch);
	    case 'I': case 'i':	return get_curr_int(ch);
	    case 'W': case 'w':	return get_curr_wis(ch);
	    case 'X': case 'x':	return get_curr_dex(ch);
	    case 'C': case 'c':	return get_curr_con(ch);
	    case 'A': case 'a':	return get_curr_cha(ch);
	    case 'U': case 'u':	return get_curr_lck(ch);
	    case 'Y': case 'y':	return get_age(ch);
	}
    }

    for (x = 0; x < len; ++x)
	if (!isdigit(exp[x]) && !isspace(exp[x]))
	    break;
    if (x == len)
	return atoi(exp);
  
    /* break it into 2 parts */
    for (x = 0; x < strlen(exp); ++x)
	switch(exp[x])
	{
	    case '^':
	      if (!total)
		eop = x;
	      break;
	    case '-': case '+':
	      if (!total) 
		lop = x;
	      break;
	    case '*': case '/': case '%': case 'd': case 'D':
	    case '<': case '>': case '{': case '}': case '=':
	      if (!total) 
		gop =  x;
	      break;
	    case '(':
	      ++total;
	      break;
	    case ')':
	      --total;
	      break;
	}
    if (lop)
	x = lop;
    else
    if (gop)
	x = gop;
    else
	x = eop;
    operation = exp[x];
    exp[x] = '\0';
    sexp[0] = exp;
    sexp[1] = (char *)(exp+x+1);

    /* work it out */
    total = rd_parse(ch, level, sexp[0]);
    switch(operation)
    {
	case '-':		total -= rd_parse(ch, level, sexp[1]);	break;
	case '+':		total += rd_parse(ch, level, sexp[1]);	break;
	case '*':		total *= rd_parse(ch, level, sexp[1]);	break;
	case '/':		total /= rd_parse(ch, level, sexp[1]);	break;
	case '%':		total %= rd_parse(ch, level, sexp[1]);	break;
	case 'd': case 'D':	total = dice( total, rd_parse(ch, level, sexp[1]) );	break;
	case '<':		total = (total < rd_parse(ch, level, sexp[1]));		break;
	case '>':		total = (total > rd_parse(ch, level, sexp[1]));		break;
	case '=':		total = (total == rd_parse(ch, level, sexp[1]));	break;
	case '{':		total = UMIN( total, rd_parse(ch, level, sexp[1]) );	break;
	case '}':		total = UMAX( total, rd_parse(ch, level, sexp[1]) );	break;

	case '^':
	{
	    int y = rd_parse(ch, level, sexp[1]), z = total;

	    for (x = 1; x < y; ++x, z *= total);
	    total = z;
	    break;
	}
    }
    return total;
}

/* wrapper function so as not to destroy exp */
int dice_parse(CHAR_DATA *ch, int level, char *exp)
{
    char buf[MAX_INPUT_LENGTH];

    strcpy( buf, exp );
    return rd_parse(ch, level, buf);
}

/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_poison_death( int level, CHAR_DATA *victim )
{
    int save;

    save = 50 + ( victim->level - level - victim->saving_poison_death ) * 5;
    save = URANGE( 5, save, 95 );
    return chance( victim, save );
}
bool saves_wands( int level, CHAR_DATA *victim )
{
    int save;

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
      return TRUE;

    save = 50 + ( victim->level - level - victim->saving_wand ) * 5;
    save = URANGE( 5, save, 95 );
    return chance( victim, save );
}
bool saves_para_petri( int level, CHAR_DATA *victim )
{
    int save;

    save = 50 + ( victim->level - level - victim->saving_para_petri ) * 5;
    save = URANGE( 5, save, 95 );
    return chance( victim, save );
}
bool saves_breath( int level, CHAR_DATA *victim )
{
    int save;

    save = 50 + ( victim->level - level - victim->saving_breath ) * 5;
    save = URANGE( 5, save, 95 );
    return chance( victim, save );
}
bool saves_spell_staff( int level, CHAR_DATA *victim )
{
    int save;

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
      return TRUE;

    if ( IS_NPC( victim ) && level > 10 )
      level -= 5;
    save = 50 + ( victim->level - level - victim->saving_spell_staff ) * 5;
    save = URANGE( 5, save, 95 );
    return chance( victim, save );
}


/*
 * Process the spell's required components, if any		-Thoric
 * -----------------------------------------------
 * T###		check for item of type ###
 * V#####	check for item of vnum #####
 * Kword	check for item with keyword 'word'
 * G#####	check if player has ##### amount of gold
 * H####	check if player has #### amount of hitpoints
 *
 * Special operators:
 * ! spell fails if player has this
 * + don't consume this component
 * @ decrease component's value[0], and extract if it reaches 0
 * # decrease component's value[1], and extract if it reaches 0
 * $ decrease component's value[2], and extract if it reaches 0
 * % decrease component's value[3], and extract if it reaches 0
 * ^ decrease component's value[4], and extract if it reaches 0
 * & decrease component's value[5], and extract if it reaches 0
 */
bool process_spell_components( CHAR_DATA *ch, int sn )
{
     SKILLTYPE *skill	= get_skilltype(sn);
     char *comp		= skill->components;
     char *check;
     char arg[MAX_INPUT_LENGTH];
     bool consume, fail, found;
     int  val, value;
     OBJ_DATA *obj;

     /* if no components necessary, then everything is cool */
     if ( !comp || comp[0] == '\0' )
	return TRUE;

     while ( comp[0] != '\0' )
     {
	comp = one_argument( comp, arg );
	consume = TRUE;
	fail = found = FALSE;
	val = -1;
	switch( arg[1] )
	{
	    default:	check = arg+1;				break;
	    case '!':	check = arg+2;	fail = TRUE;		break;
	    case '+':	check = arg+2;	consume = FALSE;	break;
	    case '@':	check = arg+2;	val = 0;		break;
	    case '#':	check = arg+2;	val = 1;		break;
	    case '$':	check = arg+2;	val = 2;		break;
	    case '%':	check = arg+2;	val = 3;		break;
	    case '^':	check = arg+2;	val = 4;		break;
	    case '&':	check = arg+2;	val = 5;		break;
	    /*   reserve '*', '(' and ')' for v6, v7 and v8   */
	}
	value = atoi(check);
	obj = NULL;
	switch( UPPER(arg[0]) )
	{
	    case 'T':
		for ( obj = ch->first_carrying; obj; obj = obj->next_content )
		   if ( obj->item_type == value )
		   {
			if ( fail )
			{
			  send_to_char( "Something disrupts the casting of this spell...\n\r", ch );
			  return FALSE;
			}
			found = TRUE;
			break;
		   }
		break;
	    case 'V':
		for ( obj = ch->first_carrying; obj; obj = obj->next_content )
		   if ( obj->pIndexData->vnum == value )
		   {
			if ( fail )
			{
			  send_to_char( "Something disrupts the casting of this spell...\n\r", ch );
			  return FALSE;
			}
			found = TRUE;
			break;
		   }
		break;
	    case 'K':
		for ( obj = ch->first_carrying; obj; obj = obj->next_content )
		   if ( nifty_is_name( check, obj->name ) )
		   {
			if ( fail )
			{
			  send_to_char( "Something disrupts the casting of this spell...\n\r", ch );
			  return FALSE;
			}
			found = TRUE;
			break;
		   }
		break;
	    case 'G':
	    	if ( ch->gold >= value )
			{
		  if ( fail )
		  {
		    send_to_char( "Something disrupts the casting of this spell...\n\r", ch );
		    return FALSE;
		  }
		  else
		  {
		    if ( consume )
		    {
			set_char_color( AT_GOLD, ch );
			send_to_char( "You feel a little lighter...\n\r", ch );
		 	ch->gold -= value;
		    }
		    continue;
		  }
			}
	    	break;
	    case 'H':
		if ( ch->hit >= value )
		{
		  if ( fail )
		  {
		    send_to_char( "Something disrupts the casting of this spell...\n\r", ch );
		    return FALSE;
		  }
		  else
		  {
		    if ( consume )
		    {
			set_char_color( AT_BLOOD, ch );
			send_to_char( "You feel a little weaker...\n\r", ch );
			ch->hit -= value;
			update_pos( ch );
		    }
		    continue;
		  }
		}
		break;
	}
	/* having this component would make the spell fail... if we get
	   here, then the caster didn't have that component */
	if ( fail )
	    continue;
	if ( !found )
	{
	    send_to_char( "Something is missing...\n\r", ch );
	    return FALSE;
	}
	if ( obj )
	{
	    if ( val >=0 && val < 6 )
	    {
		separate_obj(obj);
		if ( obj->value[val] <= 0 )
		{
		    act( AT_MAGIC, "$p disappears in a puff of smoke!", ch, obj, NULL, TO_CHAR );
		    act( AT_MAGIC, "$p disappears in a puff of smoke!", ch, obj, NULL, TO_ROOM );
		    extract_obj( obj );
		    return FALSE;
		}
		else
		if ( --obj->value[val] == 0 )
		{
		    act( AT_MAGIC, "$p glows briefly, then disappears in a puff of smoke!", ch, obj, NULL, TO_CHAR );
		    act( AT_MAGIC, "$p glows briefly, then disappears in a puff of smoke!", ch, obj, NULL, TO_ROOM );
		    extract_obj( obj );
		}
		else
		    act( AT_MAGIC, "$p glows briefly and a whisp of smoke rises from it.", ch, obj, NULL, TO_CHAR );
	    }
	    else
	    if ( consume )
	    {
		separate_obj(obj);
		act( AT_MAGIC, "$p glows brightly, then disappears in a puff of smoke!", ch, obj, NULL, TO_CHAR );
		act( AT_MAGIC, "$p glows brightly, then disappears in a puff of smoke!", ch, obj, NULL, TO_ROOM );
		extract_obj( obj );
	    }
	    else
	    {
		int count = obj->count;

		obj->count = 1;
		act( AT_MAGIC, "$p glows briefly.", ch, obj, NULL, TO_CHAR );
		obj->count = count;
	    }
	}
     }
     return TRUE;
}






int pAbort;

/*
 * Locate targets.
 */
/* Turn off annoying message and just abort if needed */
bool silence_locate_targets;

void *locate_targets( CHAR_DATA *ch, char *arg, int sn,
		      CHAR_DATA **victim, OBJ_DATA **obj )
{
    SKILLTYPE *skill = get_skilltype( sn );
    void *vo	= NULL;

    *victim	= NULL;
    *obj	= NULL;
      
    switch ( skill->target )
    {
	default:
	  bug( "Do_cast: bad target for sn %d.", sn );
	  return &pAbort;

	case TAR_IGNORE:
	  break;

	case TAR_CHAR_OFFENSIVE:
	  {
	    if ( arg[0] == '\0' )
	    {
		if ( ( *victim = who_fighting( ch ) ) == NULL )
		{
		    if (!silence_locate_targets)
			send_to_char( "Cast the spell on whom?\n\r", ch );
		    return &pAbort;
		}
	    }
	    else
	    {
		if ( ( *victim = get_char_room( ch, arg ) ) == NULL )
		{
		    if (!silence_locate_targets)
			send_to_char( "They aren't here.\n\r", ch );
		    return &pAbort;
		}
	    }
	  }

	  /* Offensive spells will choose the ch up to 92% of the time
	   * if the nuisance flag is set -- Shaddai 
	   */
	  if ( !IS_NPC(ch) && ch->pcdata->nuisance && 
			ch->pcdata->nuisance->flags > 5
		&& number_percent() < (((ch->pcdata->nuisance->flags-5)*8)+
		ch->pcdata->nuisance->power*6))
		*victim = ch;

	  if ( sysdata.magichell && ch->level > 35 && number_percent() < 30 )
		*victim = ch;

	  if ( is_safe(ch, *victim, TRUE) )
		return &pAbort;

	  if ( ch == *victim )
	  {
	        if ( SPELL_FLAG(get_skilltype(sn), SF_NOSELF))
	        {
		  if (!silence_locate_targets)
		    send_to_char( "You can't cast this on yourself!\n\r", ch);
		  return &pAbort;
		}
		if (!silence_locate_targets)
		  send_to_char( "Cast this on yourself?  Okay...\n\r", ch );
		/*
		send_to_char( "You can't do that to yourself.\n\r", ch );
		return &pAbort;
		*/
	  }

	  if ( !IS_NPC(ch) )
	  {
		if ( !IS_NPC(*victim) )
		{
		    if ( get_timer( ch, TIMER_PKILLED ) > 0 )
		    {
			if (!silence_locate_targets)
			    send_to_char( "You have been killed in the last 5 minutes.\n\r", ch);
			return &pAbort;
		    }

		    if ( get_timer( *victim, TIMER_PKILLED ) > 0 )
		    {
			if (!silence_locate_targets)
			    send_to_char( "This player has been killed in the last 5 minutes.\n\r", ch );
			return &pAbort;
		    }	
		    if ( xIS_SET(ch->act, PLR_NICE) && ch != *victim )
		    {
			if (!silence_locate_targets)
			    send_to_char("You are too nice to attack another player.\n\r", ch );
			return &pAbort;
		    } 
		    if ( *victim != ch)
		    {
			if (!silence_locate_targets)
		            send_to_char( "You really shouldn't do this to another player...\n\r", ch );
		        else if (who_fighting(*victim) != ch)
		        {
		            /* Only auto-attack those that are hitting you. */
		            return &pAbort;
		        }
		    }
		}

		if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == *victim )
		{
		    if (!silence_locate_targets)
			send_to_char( "You can't do that on your own follower.\n\r", ch );
		    return &pAbort;
		}
	  }

	  check_illegal_pk( ch, *victim );
	  vo = (void *) *victim;
	  break;

	case TAR_CHAR_DEFENSIVE:
	  {
	    if ( arg[0] == '\0' )
		*victim = ch;
	    else
	    {
		if ( ( *victim = get_char_room( ch, arg ) ) == NULL )
		{
		    if (!silence_locate_targets)
			send_to_char( "They aren't here.\n\r", ch );
		    return &pAbort;
		}
	    }
	  }

	  if ( SPELL_FLAG( skill, SF_NOMOB )
	  &&   IS_NPC( *victim ) )
	  {
		send_to_char( "Your magic fails to take hold.\n\r", ch );
		return &pAbort;
	  }


	  /* Nuisance flag will pick who you are fighting for defensive
	   * spells up to 36% of the time -- Shaddai
	   */

	  if ( !IS_NPC(ch) && ch->fighting && ch->pcdata->nuisance &&
		ch->pcdata->nuisance->flags > 5
		&& number_percent() < (((ch->pcdata->nuisance->flags-5)*8) +
		6*ch->pcdata->nuisance->power))
		*victim = who_fighting( ch );

	  if ( ch == *victim && 
			SPELL_FLAG(get_skilltype(sn), SF_NOSELF))
	  {
	    if (!silence_locate_targets)
	      send_to_char( "You can't cast this on yourself!\n\r", ch);
	    return &pAbort;
	  }

	  vo = (void *) *victim;
	  break;

	case TAR_CHAR_SELF:
	  if ( arg[0] != '\0' && !nifty_is_name( arg, ch->name ) )
	  {
		if (!silence_locate_targets)
		    send_to_char( "You cannot cast this spell on another.\n\r", ch );
		return &pAbort;
	  }

	  vo = (void *) ch;
	  break;

	case TAR_OBJ_INV:
	  {
	    if ( arg[0] == '\0' )
	    {
		if (!silence_locate_targets)
		    send_to_char( "What should the spell be cast upon?\n\r", ch );
		return &pAbort;
	    }

	    if ( ( *obj = get_obj_carry( ch, arg ) ) == NULL )
	    {
		if (!silence_locate_targets)
		    send_to_char( "You are not carrying that.\n\r", ch );
		return &pAbort;
	    }
	  }

	  vo = (void *) *obj;
	  break;
    }

    return vo;
}






/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;
char *ranged_target_name = NULL;


/*
 * Cast a spell.  Multi-caster and component support by Thoric
 */
void do_cast( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    static char staticbuf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    void *vo = NULL;
    int mana;
    int blood;
    int sn;
    ch_ret retcode;
    bool dont_wait = FALSE;
    SKILLTYPE *skill = NULL;
    struct timeval time_used;

    retcode = rNONE;

    switch( ch->substate )
    {
      default:
	/* no ordering charmed mobs to cast spells */

	if ( IS_NPC(ch) 
	&& ( IS_AFFECTED( ch, AFF_CHARM ) || IS_AFFECTED( ch, AFF_POSSESS ) ) )
	{
	    send_to_char( "You can't seem to do that right now...\n\r", ch );
	    return;
	}

	if ( xIS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) || IS_SET( ch->in_room->area->flags, AFLAG_NOMAGIC ) )
	{
	    set_char_color( AT_MAGIC, ch );
	    send_to_char( "You failed.\n\r", ch );
	    return;
	}

	target_name = one_argument( argument, arg1 );
	one_argument( target_name, arg2 );
	if ( ranged_target_name )
		DISPOSE( ranged_target_name );
	ranged_target_name = str_dup( target_name );

	if ( arg1[0] == '\0' )
	{
	    send_to_char( "Cast which what where?\n\r", ch );
	    return;
	}

	/* Regular mortal spell casting */
	if ( get_trust(ch) < LEVEL_GOD )
	{
	    if ( ( sn = find_spell( ch, arg1, TRUE ) ) < 0
	    || ( !IS_NPC(ch) && ch->level < skill_table[sn]->skill_level[ch->class] ) )
	    {
		send_to_char( "You can't do that.\n\r", ch );
		return;
	    }
	    if ( (skill=get_skilltype(sn)) == NULL )
	    {
		send_to_char( "You can't do that right now...\n\r", ch );
		return;
	    }
	}
	else
	/*
	 * Godly "spell builder" spell casting with debugging messages
	 */
	{
	    if ( (sn=skill_lookup(arg1)) < 0 )
	    {
		send_to_char( "We didn't create that yet...\n\r", ch );
		return;
	    }
	    if ( sn >= MAX_SKILL )
	    {
		send_to_char( "Hmm... that might hurt.\n\r", ch );
		return;
	    }
	    if ( (skill=get_skilltype(sn)) == NULL )
	    {
		send_to_char( "Something is severely wrong with that one...\n\r", ch );
		return;
	    }
	    if ( skill->type != SKILL_SPELL )
	    {
		send_to_char( "That isn't a spell.\n\r", ch );
		return;
	    }
	    if ( !skill->spell_fun )
	    {
		send_to_char( "We didn't finish that one yet...\n\r", ch );
		return;
	    }
	}

	/*
	 * Something else removed by Merc			-Thoric
	 */
	/* Band-aid alert!  !IS_NPC check -- Blod */
	if ( ch->position < skill->minimum_position && !IS_NPC(ch) )
	{
	    switch( ch->position )
	    {
	      default:
		send_to_char( "You can't concentrate enough.\n\r", ch );
		break;
	      case POS_SITTING:
		send_to_char( "You can't summon enough energy sitting down.\n\r", ch );
		break;
	      case POS_RESTING:
		send_to_char( "You're too relaxed to cast that spell.\n\r", ch );
		break;
	      case POS_FIGHTING:
		if(skill->minimum_position<=POS_EVASIVE){
		  send_to_char( "This fighting style is too demanding for that!\n\r", ch);
		} else {
		  send_to_char( "No way!  You are still fighting!\n\r", ch);
		}
                break;
	      case POS_DEFENSIVE:
		if(skill->minimum_position<=POS_EVASIVE){
		  send_to_char( "This fighting style is too demanding for that!\n\r", ch);
		} else {
		  send_to_char( "No way!  You are still fighting!\n\r", ch);
		}
                break;
	      case POS_AGGRESSIVE:
		if(skill->minimum_position<=POS_EVASIVE){
		  send_to_char( "This fighting style is too demanding for that!\n\r", ch);
		} else {
		  send_to_char( "No way!  You are still fighting!\n\r", ch);
		}
                break;
	      case POS_BERSERK:
		if(skill->minimum_position<=POS_EVASIVE){
		  send_to_char( "This fighting style is too demanding for that!\n\r", ch);
		} else {
		  send_to_char( "No way!  You are still fighting!\n\r", ch);
		}
                break;
	      case POS_EVASIVE:
		send_to_char( "No way!  You are still fighting!\n\r", ch);
		break;
	      case POS_SLEEPING:
		send_to_char( "You dream about great feats of magic.\n\r", ch );
		break;
	    }
	    return;
	}

	if ( skill->spell_fun == spell_null )
	{
	    send_to_char( "That's not a spell!\n\r", ch );
	    return;
	}

	if ( !skill->spell_fun )
	{
	    send_to_char( "You cannot cast that... yet.\n\r", ch );
	    return;
	}

	if ( !IS_NPC(ch)			/* fixed by Thoric */
	&&   !IS_IMMORTAL(ch) 
	&&    skill->guild != CLASS_NONE 
	&&  (!ch->pcdata->clan
	|| skill->guild != ch->pcdata->clan->class) )
	{
	    send_to_char( "That is only available to members of a certain guild.\n\r", ch);
	    return;
	}

	/* Mystaric, 980908 - Added checks for spell sector type */
	if ( !ch->in_room || ( skill->spell_sector && 
	     !IS_SET( skill->spell_sector, (1<<ch->in_room->sector_type))))
   	{
     	  send_to_char("You can not cast that here.\n\r", ch );
     	  return;
   	}

	if ( !sysdata.magichell )
	mana = IS_NPC(ch) ? 0 : UMAX(skill->min_mana,
	   100 / ( 2 + ch->level - skill->skill_level[ch->class] ) );
	else
	mana = IS_NPC(ch) ? 0 : UMAX(skill->min_mana * number_range(5,10)/5,
	   (100 / (2 + ch->level - skill->skill_level[ch->class] ) ) * number_range(5,10)/5 );

	/*
	 * Locate targets.
	 */
	vo = locate_targets( ch, arg2, sn, &victim, &obj );
	if ( vo == &pAbort )
	    return;

	if ( !IS_NPC( ch ) && victim && !IS_NPC( victim ) 
	&&    CAN_PKILL( victim ) && !CAN_PKILL( ch )
	&&   !in_arena( ch ) && !in_arena( victim ) )
	{
	    set_char_color( AT_MAGIC, ch );
	    send_to_char( "The gods will not permit you to cast spells on that character.\n\r", ch );
	    return;
	}


	/*
	 * Vampire spell casting				-Thoric
	 */
	blood = UMAX(1, (mana+4) / 8);      /* NPCs don't have PCDatas. -- Altrag */
	if ( IS_VAMPIRE(ch) )
	{
	    if (ch->pcdata->condition[COND_BLOODTHIRST] < blood)
	    {
		send_to_char( "You don't have enough blood power.\n\r", ch );
		return;
	    }
	}
	else
	if ( !IS_NPC(ch) && ch->mana < mana )
	{
	    send_to_char( "You don't have enough mana.\n\r", ch );
	    return;
	}

	if ( skill->participants <= 1 )
	   break;

	/* multi-participant spells			-Thoric */
	add_timer( ch, TIMER_DO_FUN, UMIN(skill->beats / 10, 3),
		do_cast, 1 );
	act( AT_MAGIC, "You begin to chant...", ch, NULL, NULL, TO_CHAR );
	act( AT_MAGIC, "$n begins to chant...", ch, NULL, NULL, TO_ROOM );
	sprintf( staticbuf, "%s %s", arg2, target_name );
	ch->alloc_ptr = str_dup( staticbuf );
	ch->tempnum = sn;
	return;	
      case SUB_TIMER_DO_ABORT:
        DISPOSE( ch->alloc_ptr );
	if ( IS_VALID_SN((sn = ch->tempnum)) )
	{
	    if ( (skill=get_skilltype(sn)) == NULL )
	    {
		send_to_char( "Something went wrong...\n\r", ch );
		bug( "do_cast: SUB_TIMER_DO_ABORT: bad sn %d", sn );
		return;
	    }
	    mana = IS_NPC(ch) ? 0 : UMAX(skill->min_mana,
	       100 / ( 2 + ch->level - skill->skill_level[ch->class] ) );
	    blood = UMAX(1, (mana+4) / 8);
	    if ( IS_VAMPIRE(ch) )
	      gain_condition( ch, COND_BLOODTHIRST, - UMAX(1, blood / 3) );
	    else
	    if (ch->level < LEVEL_IMMORTAL)    /* so imms dont lose mana */
	      ch->mana -= mana / 3;
	}
	set_char_color( AT_MAGIC, ch );
	send_to_char( "You stop chanting...\n\r", ch );
	/* should add chance of backfire here */
        return;
      case 1:
	sn = ch->tempnum;
	if ( (skill=get_skilltype(sn)) == NULL )
	{
	    send_to_char( "Something went wrong...\n\r", ch );
	    bug( "do_cast: substate 1: bad sn %d", sn );
	    return;
	}
	if ( !ch->alloc_ptr || !IS_VALID_SN(sn) || skill->type != SKILL_SPELL )
	{
	    send_to_char( "Something cancels out the spell!\n\r", ch );
	    bug( "do_cast: ch->alloc_ptr NULL or bad sn (%d)", sn );
	    return;
	}
	mana = IS_NPC(ch) ? 0 : UMAX(skill->min_mana,
	   100 / ( 2 + ch->level - skill->skill_level[ch->class] ) );
	blood = UMAX(1, (mana+4) / 8);
	strcpy( staticbuf, ch->alloc_ptr );
	target_name = one_argument(staticbuf, arg2);
	DISPOSE( ch->alloc_ptr );
	ch->substate = SUB_NONE;
	if ( skill->participants > 1 )
	{
	    int cnt = 1;
	    CHAR_DATA *tmp;
	    TIMER *t;

	    for ( tmp = ch->in_room->first_person; tmp; tmp = tmp->next_in_room )
		if (  tmp != ch
		&&   (t = get_timerptr( tmp, TIMER_DO_FUN )) != NULL
		&&    t->count >= 1 && t->do_fun == do_cast
		&&    tmp->tempnum == sn && tmp->alloc_ptr
		&&   !str_cmp( tmp->alloc_ptr, staticbuf ) )
		  ++cnt;
	    if ( cnt >= skill->participants )
	    {
		for ( tmp = ch->in_room->first_person; tmp; tmp = tmp->next_in_room )
		    if (  tmp != ch
		    &&   (t = get_timerptr( tmp, TIMER_DO_FUN )) != NULL
		    &&    t->count >= 1 && t->do_fun == do_cast
		    &&    tmp->tempnum == sn && tmp->alloc_ptr
		    &&   !str_cmp( tmp->alloc_ptr, staticbuf ) )
		{
		    extract_timer( tmp, t );
		    act( AT_MAGIC, "Channeling your energy into $n, you help cast the spell!", ch, NULL, tmp, TO_VICT );
		    act( AT_MAGIC, "$N channels $S energy into you!", ch, NULL, tmp, TO_CHAR );
		    act( AT_MAGIC, "$N channels $S energy into $n!", ch, NULL, tmp, TO_NOTVICT );
		    learn_from_success( tmp, sn );
		    if ( IS_VAMPIRE(ch) )
			gain_condition( tmp, COND_BLOODTHIRST, - blood );
		    else
			tmp->mana -= mana;
		    tmp->substate = SUB_NONE;
		    tmp->tempnum = -1;
		    DISPOSE( tmp->alloc_ptr );
		}
		dont_wait = TRUE;
		send_to_char( "You concentrate all the energy into a burst of mystical words!\n\r", ch );
		vo = locate_targets( ch, arg2, sn, &victim, &obj );
		if ( vo == &pAbort )
		  return;
	    }
	    else
	    {
		set_char_color( AT_MAGIC, ch );
		send_to_char( "There was not enough power for the spell to succeed...\n\r", ch );
		if ( IS_VAMPIRE(ch) )
		  gain_condition( ch, COND_BLOODTHIRST, - UMAX(1, blood / 2) );
		else
		if (ch->level < LEVEL_IMMORTAL)    /* so imms dont lose mana */
		  ch->mana -= mana / 2;
		learn_from_failure( ch, sn );
		return;
	    }
	}
    }

    /* uttering those magic words unless casting "ventriloquate" */
    if ( str_cmp( skill->name, "ventriloquate" ) )
	say_spell( ch, sn );

    if ( !dont_wait )
	WAIT_STATE( ch, skill->beats );

    /*
     * Getting ready to cast... check for spell components	-Thoric
     */
    if ( !process_spell_components( ch, sn ) )
    {
	if ( IS_VAMPIRE(ch) )
	  gain_condition( ch, COND_BLOODTHIRST, - UMAX(1, blood / 2) );
	else
	if (ch->level < LEVEL_IMMORTAL)    /* so imms dont lose mana */
	  ch->mana -= mana / 2;
	learn_from_failure( ch, sn );
	return;
    }

    if ( !IS_NPC(ch)
    &&   (number_percent( ) + skill->difficulty * 5) > ch->pcdata->learned[sn] )
    {
	/* Some more interesting loss of concentration messages  -Thoric */
	switch( number_bits(2) )
	{
	    case 0:	/* too busy */
		if ( ch->fighting )
		  send_to_char( "This round of battle is too hectic to concentrate properly.\n\r", ch );
		else
		  send_to_char( "You lost your concentration.\n\r", ch );
		break;
	    case 1:	/* irritation */
		if ( number_bits(2) == 0 )
		{
		  switch( number_bits(2) )
		  {
		     case 0: send_to_char( "A tickle in your nose prevents you from keeping your concentration.\n\r", ch ); break;
		     case 1: send_to_char( "An itch on your leg keeps you from properly casting your spell.\n\r", ch ); break;
		     case 2: send_to_char( "Something in your throat prevents you from uttering the proper phrase.\n\r", ch ); break;
		     case 3: send_to_char( "A twitch in your eye disrupts your concentration for a moment.\n\r", ch ); break;
		  }
		}
		else
		  send_to_char( "Something distracts you, and you lose your concentration.\n\r", ch );
		break;
	    case 2:	/* not enough time */
		if ( ch->fighting )
		  send_to_char( "There wasn't enough time this round to complete the casting.\n\r", ch );
		else
		  send_to_char( "You lost your concentration.\n\r", ch );
		break;
	    case 3:
		send_to_char( "You get a mental block mid-way through the casting.\n\r", ch );
		break;
	}
	if ( IS_VAMPIRE(ch) )
	  gain_condition( ch, COND_BLOODTHIRST, - UMAX(1, blood / 2) );
	else
	if (ch->level < LEVEL_IMMORTAL)    /* so imms dont lose mana */
	  ch->mana -= mana / 2;
	learn_from_failure( ch, sn );
	return;
    }
    else
    {
	if ( IS_VAMPIRE(ch) )
	  gain_condition( ch, COND_BLOODTHIRST, - blood );
	else
	  ch->mana -= mana;

	/*
	 * check for immunity to magic if victim is known...
	 * and it is a TAR_CHAR_DEFENSIVE/SELF spell
	 * otherwise spells will have to check themselves
	 */
	if ( ((skill->target == TAR_CHAR_DEFENSIVE
	||    skill->target == TAR_CHAR_SELF)
	&&    victim && IS_SET(victim->immune, RIS_MAGIC)) )
	{
	   immune_casting( skill, ch, victim, NULL );
	   retcode = rSPELL_FAILED;
	}
	else
	{
	   start_timer(&time_used);
	   retcode = (*skill->spell_fun) ( sn, ch->level, ch, vo );
	   end_timer(&time_used);
	   update_userec(&time_used, &skill->userec);
	}
    }

    if ( ch->in_room && IS_SET( ch->in_room->area->flags, AFLAG_SPELLLIMIT ) )
    	ch->in_room->area->curr_spell_count++;

    if ( retcode == rCHAR_DIED || retcode == rERROR || char_died(ch) )
	return;

    /* learning */
    if ( retcode != rSPELL_FAILED )
	learn_from_success( ch, sn );
    else
	learn_from_failure( ch, sn );


    /* favor adjustments */
    if ( victim && victim != ch && !IS_NPC(victim)
    &&   skill->target == TAR_CHAR_DEFENSIVE )
	adjust_favor( ch, 7, 1 );

    if ( victim && victim != ch && !IS_NPC(ch)
    &&   skill->target == TAR_CHAR_DEFENSIVE )
	adjust_favor( victim, 13, 1 );

    if ( victim && victim != ch && !IS_NPC(ch)
    &&   skill->target == TAR_CHAR_OFFENSIVE )
	adjust_favor( ch, 4, 1 );

    /*
     * Fixed up a weird mess here, and added double safeguards	-Thoric
     */
    if ( skill->target == TAR_CHAR_OFFENSIVE
    &&   victim
    &&  !char_died(victim)
    &&	 victim != ch )
    {
	CHAR_DATA *vch, *vch_next;
	
	for ( vch = ch->in_room->first_person; vch; vch = vch_next )
	{
    	   vch_next = vch->next_in_room;
    	   
    	   if ( vch == victim )
    	   {
		if ( vch->master != ch
		&&  !vch->fighting )
    		  retcode = multi_hit( vch, ch, TYPE_UNDEFINED );
		break;
	   }
	}
    }
    
    return;
}


/*
 * Cast spells at targets using a magical object.
 */
ch_ret obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
    void *vo;
    ch_ret retcode = rNONE;
    int levdiff = ch->level - level;
    SKILLTYPE *skill = get_skilltype( sn );
    struct timeval time_used;

    if ( sn == -1 )
	return retcode;
    if ( !skill || !skill->spell_fun )
    {
	bug( "Obj_cast_spell: bad sn %d.", sn );
	return rERROR;
    }

    if ( !ch->in_room || xIS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) )
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "Nothing seems to happen...\n\r", ch );
	return rNONE;
    }

    if ( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) && 
	 skill->target == TAR_CHAR_OFFENSIVE)
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "Nothing seems to happen...\n\r", ch );
	return rNONE;
    }

    /*
     * Basically this was added to cut down on level 5 players using level
     * 40 scrolls in battle too often ;)		-Thoric
     */
    if ( ch != victim
	&& 	  (skill->target == TAR_CHAR_OFFENSIVE
    ||    number_bits(7) == 1)	/* 1/128 chance if non-offensive */
    &&    skill->type != SKILL_HERB
    &&   !chance( ch, 95 + levdiff ) )
    {
	switch( number_bits(2) )
	{
	   case 0: failed_casting( skill, ch, victim, NULL );	break;
	   case 1:
		act( AT_MAGIC, "The $t spell backfires!", ch, skill->name, victim, TO_CHAR );
		if ( victim )
		  act( AT_MAGIC, "$n's $t spell backfires!", ch, skill->name, victim, TO_VICT );
		act( AT_MAGIC, "$n's $t spell backfires!", ch, skill->name, victim, TO_NOTVICT );
		return damage( ch, ch, number_range( 1, level ), TYPE_UNDEFINED );
	   case 2: failed_casting( skill, ch, victim, NULL );	break;
	   case 3:
		act( AT_MAGIC, "The $t spell backfires!", ch, skill->name, victim, TO_CHAR );
		if ( victim )
		  act( AT_MAGIC, "$n's $t spell backfires!", ch, skill->name, victim, TO_VICT );
		act( AT_MAGIC, "$n's $t spell backfires!", ch, skill->name, victim, TO_NOTVICT );
		return damage( ch, ch, number_range( 1, level ), TYPE_UNDEFINED );
	}
	return rNONE;
    }

    target_name = "";
    switch ( skill->target )
    {
    default:
	bug( "Obj_cast_spell: bad target for sn %d.", sn );
	return rERROR;

    case TAR_IGNORE:
	vo = NULL;
	if ( victim )
	    target_name = victim->name;
	else
	if ( obj )
	    target_name = obj->name;
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( victim != ch )
	{
	  if ( !victim )
	      victim = who_fighting( ch );
	  if ( !victim || (!IS_NPC(victim) && !in_arena(victim)) )
	  {
	      send_to_char( "You can't do that.\n\r", ch );
	      return rNONE;
	  }
	}
	if ( ch != victim && is_safe( ch, victim, TRUE ) )
	  return rNONE;
	vo = (void *) victim;
	break;

    case TAR_CHAR_DEFENSIVE:
	if ( victim == NULL )
	    victim = ch;
	vo = (void *) victim;
        if ( SPELL_FLAG( skill, SF_NOMOB )
        &&   IS_NPC( victim ) )
        {
              send_to_char( "Your magic fails to take hold.\n\r", ch );
              return rNONE;
        }
	if ( skill->type != SKILL_HERB
	&&   IS_SET(victim->immune, RIS_MAGIC ) )
	{
	    immune_casting( skill, ch, victim, NULL );
	    return rNONE;
	}
	break;

    case TAR_CHAR_SELF:
	vo = (void *) ch;
	if ( skill->type != SKILL_HERB
	&&   IS_SET(ch->immune, RIS_MAGIC ) )
	{
	    immune_casting( skill, ch, victim, NULL );
	    return rNONE;
	}
	break;

    case TAR_OBJ_INV:
	if ( obj == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return rNONE;
	}
	vo = (void *) obj;
	break;
    }

    start_timer(&time_used);
    retcode = (*skill->spell_fun) ( sn, level, ch, vo );
    end_timer(&time_used);
    update_userec(&time_used, &skill->userec);

    if ( retcode == rSPELL_FAILED )
	retcode = rNONE;

    if ( retcode == rCHAR_DIED || retcode == rERROR )
	return retcode;

    if ( char_died(ch) )
	return rCHAR_DIED;


    if ( skill->target == TAR_CHAR_OFFENSIVE
    &&   victim != ch
    &&  !char_died(victim) )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	for ( vch = ch->in_room->first_person; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && !vch->fighting && vch->master != ch )
	    {
		retcode = multi_hit( vch, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }

    return retcode;
}



/*
 * Spell functions.
 */
ch_ret spell_acid_blast( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 6 );
    if ( saves_spell_staff( level, victim ) )
	dam /= 2;
    return damage( ch, victim, dam, sn );
}




ch_ret spell_blindness( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int tmp;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( SPELL_FLAG(skill, SF_PKSENSITIVE)
    &&  !IS_NPC(ch) && !IS_NPC(victim) )
	tmp = level/2;
    else
	tmp = level;

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
	immune_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }
    if ( IS_AFFECTED(victim, AFF_BLIND) || saves_spell_staff( tmp, victim ) )
    {
	failed_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    af.type      = sn;
    af.location  = APPLY_HITROLL;
    af.modifier  = -4;
    af.duration  = (1 + (level / 3)) * DUR_CONV;
    af.bitvector = meb(AFF_BLIND);
    affect_to_char( victim, &af );
    set_char_color( AT_MAGIC, victim );
    send_to_char( "You are blinded!\n\r", victim );
    if ( ch != victim )
    {
	act( AT_MAGIC, "You weave a spell of blindness around $N.", ch, NULL, victim, TO_CHAR );
	act( AT_MAGIC, "$n weaves a spell of blindness about $N.", ch, NULL, victim, TO_NOTVICT );
    }
    return rNONE;
}


ch_ret spell_burning_hands( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] =
    {
	 0,
	 0,  0,  0,  0,	14,	17, 20, 23, 26, 29,
	29, 29, 30, 30,	31,	31, 32, 32, 33, 33,
	34, 34, 35, 35,	36,	36, 37, 37, 38, 38,
	39, 39, 40, 40,	41,	41, 42, 42, 43, 43,
    44, 44, 45, 45, 46, 46, 47, 47, 48, 48,
    49, 49, 50, 50, 51, 51, 52, 52, 53, 53,
    54, 54, 55, 55, 56, 56, 57, 57, 58, 58
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell_staff( level, victim ) )
	dam /= 2;
    return damage( ch, victim, dam, sn );
}



ch_ret spell_call_lightning( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    bool ch_died;
    ch_ret retcode = rNONE;

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You must be out of doors.\n\r", ch );
	return rSPELL_FAILED;
    }

    if ( ch->in_room->area->weather->precip <= 0 )
    {
	send_to_char( "You need bad weather.\n\r", ch );
	return rSPELL_FAILED;
    }

    dam = dice(level/2, 8);

    set_char_color( AT_MAGIC, ch );
    send_to_char( "God's lightning strikes your foes!\n\r", ch );
    act( AT_MAGIC, "$n calls God's lightning to strike $s foes!",
	ch, NULL, NULL, TO_ROOM );

    ch_died = FALSE;
    for ( vch = first_char; vch; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( !vch->in_room )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( IS_NPC( vch ) && xIS_SET( vch->act, ACT_MOBINVIS ) )
		continue;
            if ( !IS_NPC( vch ) && xIS_SET( vch->act, PLR_WIZINVIS )
	    &&    vch->pcdata->wizinvis >= LEVEL_IMMORTAL )
		continue;

	    if ( vch != ch && ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) ) )
		retcode = damage( ch, vch, saves_spell_staff( level, vch ) ? dam/2 : dam, sn );
	    if ( retcode == rCHAR_DIED || char_died(ch) )
		ch_died = TRUE;
	    continue;
	}

	if ( !ch_died
	&&   vch->in_room->area == ch->in_room->area
	&&   IS_OUTSIDE(vch)
	&&   IS_AWAKE(vch) ) {
	    if ( number_bits( 3 ) == 0 )
	      send_to_char_color( "&BLightning flashes in the sky.\n\r", vch );
	}
    }

    if ( ch_died )
      return rCHAR_DIED;
    else
      return rNONE;
}



ch_ret spell_cause_light( int sn, int level, CHAR_DATA *ch, void *vo )
{
    return damage( ch, (CHAR_DATA *) vo, dice(1, 8) + level / 3, sn );
}



ch_ret spell_cause_critical( int sn, int level, CHAR_DATA *ch, void *vo )
{
    return damage( ch, (CHAR_DATA *) vo, dice(3, 8) + level - 6, sn );
}



ch_ret spell_cause_serious( int sn, int level, CHAR_DATA *ch, void *vo )
{
    return damage( ch, (CHAR_DATA *) vo, dice(2, 8) + level / 2, sn );
}


ch_ret spell_change_sex( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
	immune_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }
    if ( is_affected( victim, sn ) )
    {
        failed_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }
    af.type      = sn;
    af.duration  = 10 * level * DUR_CONV;
    af.location  = APPLY_SEX;
    do
    {
	af.modifier  = number_range( 0, 2 ) - victim->sex;
    }
    while ( af.modifier == 0 );
    xCLEAR_BITS(af.bitvector);
    affect_to_char( victim, &af );
/*    set_char_color( AT_MAGIC, victim );
    send_to_char( "You feel different.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );*/
    successful_casting( skill, ch, victim, NULL );
    return rNONE;
}

bool can_charm( CHAR_DATA *ch )
{
  if ( IS_NPC(ch) || IS_IMMORTAL(ch) )
  	return TRUE;
  if ( ((get_curr_cha(ch)/3)+1) > ch->pcdata->charmies )
  	return TRUE;
  return FALSE;
}

ch_ret spell_charm_person( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int chance;
    char buf[MAX_STRING_LENGTH];
    SKILLTYPE *skill = get_skilltype(sn);

    if ( victim == ch )
    {
	send_to_char( "You like yourself even better!\n\r", ch );
	return rSPELL_FAILED;
    }

    if ( get_timer( ch, TIMER_PKILLED) > 0 )
    {
        send_to_char( "You have been killed in the past five minutes.\n\r", ch );
        return rSPELL_FAILED;
    }

    if ( IS_SET( victim->immune, RIS_MAGIC )
    ||   IS_SET( victim->immune, RIS_CHARM ) )
    {
	immune_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    if ( !IS_NPC( victim ) && !IS_NPC( ch ) )
    {
	send_to_char( "I don't think so...\n\r", ch );
	send_to_char( "You feel charmed...\n\r", victim );
	return rSPELL_FAILED;
    }

    chance = ris_save( victim, level, RIS_CHARM );

    if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   chance == 1000
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||   level < victim->level
    ||	 circle_follow( victim, ch )
    ||   !can_charm( ch )
    ||   saves_spell_staff( chance, victim ) )
    {
	failed_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

/* cannot charm mobs while having just been killed/killer */
if (get_timer(ch, TIMER_PKILLED) > 0)
{
   send_to_char( "You are still too weak..\n\r", ch );
   return rSPELL_FAILED;
}

    if ( victim->master )
	stop_follower( victim );
    add_follower( victim, ch );
    af.type      = sn;
    af.duration  = (number_fuzzy( (level + 1) / 5 ) + 1) * DUR_CONV;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = meb(AFF_CHARM);
    affect_to_char( victim, &af );
/*    act( AT_MAGIC, "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
    act( AT_MAGIC, "$N's eyes glaze over...", ch, NULL, victim, TO_ROOM );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );*/
    successful_casting( skill, ch, victim, NULL );

    sprintf( buf, "%s has charmed %s.", ch->name, victim->name);
    log_string_plus( buf, LOG_NORMAL, ch->level );
    if ( !IS_NPC(ch) )
    	ch->pcdata->charmies++;
    if ( IS_NPC( victim ) )
    {
      start_hating( victim, ch );
      start_hunting( victim, ch );
    }
/*
    to_channel( buf, CHANNEL_MONITOR, "Monitor", UMAX( LEVEL_IMMORTAL, ch->level ) );
*/
    return rNONE; 
}


ch_ret spell_chill_touch( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] =
    {
	 0,
	 0,  0,  6,  7,  8,	 9, 12, 13, 13, 13,
	14, 14, 14, 15, 15,	15, 16, 16, 16, 17,
	17, 17, 18, 18, 18,	19, 19, 19, 20, 20,
	20, 21, 21, 21, 22,	22, 22, 23, 23, 23,
    24, 24, 24, 25, 25, 25, 26, 26, 26, 27,
    27, 28, 28, 29, 29, 30, 30, 31, 31, 32,
    32, 33, 34, 34, 35, 35, 36, 37, 37, 38
    };
    AFFECT_DATA af;
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( !saves_spell_staff( level, victim ) )
    {
	af.type      = sn;
	af.duration  = 14;
	af.location  = APPLY_STR;
	af.modifier  = -1;
	xCLEAR_BITS(af.bitvector);
	affect_join( victim, &af );
    }
    else
    {
	dam /= 2;
    }

    return damage( ch, victim, dam, sn );
}



ch_ret spell_colour_spray( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] =
    {
	 0,
	 0,  0,  0,  0,  0,	 0,  0,  0,  0,  0,
	30, 35, 40, 45, 50,	55, 55, 55, 56, 57,
	58, 58, 59, 60, 61,	61, 62, 63, 64, 64,
	65, 66, 67, 67, 68,	69, 70, 70, 71, 72,
    73, 73, 74, 75, 76, 76, 77, 78, 79, 79,
    80, 80, 81, 82, 82, 83, 83, 84, 85, 85,
    86, 86, 87, 88, 88, 89, 89, 90, 91, 91
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2,  dam_each[level] * 2 );
    if ( saves_spell_staff( level, victim ) )
	dam /= 2;

    return damage( ch, victim, dam, sn );
}


ch_ret spell_control_weather( int sn, int level, CHAR_DATA *ch, void *vo )
{
    SKILLTYPE *skill = get_skilltype(sn);
    WEATHER_DATA *weath;
    int change;    
    weath = ch->in_room->area->weather;
    
    change = number_range(-rand_factor, rand_factor) +
    	(ch->level*3)/(2*max_vector);
    
    if ( !str_cmp( target_name, "warmer" ) )
	weath->temp_vector += change;
    else if ( !str_cmp( target_name, "colder" ) )
	weath->temp_vector -= change;
    else if(!str_cmp(target_name, "wetter"))
    	weath->precip_vector += change;
    else if(!str_cmp(target_name, "drier"))
    	weath->precip_vector -= change;
    else if(!str_cmp(target_name, "windier"))
    	weath->wind_vector += change;
    else if(!str_cmp(target_name, "calmer"))
    	weath->wind_vector -= change;
    else
    {
	send_to_char ("Do you want it to get warmer, colder, wetter, "
		"drier, windier, or calmer?\n\r", ch );
	return rSPELL_FAILED;
    }
    
    weath->temp_vector = URANGE(-max_vector,
    	weath->temp_vector, max_vector);
    weath->precip_vector = URANGE(-max_vector,
    	weath->precip_vector, max_vector);
    weath->wind_vector = URANGE(-max_vector,
    	weath->wind_vector, max_vector);
    
    successful_casting( skill, ch, NULL, NULL );
    return rNONE;
}


ch_ret spell_create_food( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *mushroom;

    mushroom = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ), 0 );
    mushroom->value[0] = 5 + level;
    act( AT_MAGIC, "$p suddenly appears.", ch, mushroom, NULL, TO_ROOM );
    act( AT_MAGIC, "$p suddenly appears.", ch, mushroom, NULL, TO_CHAR );
    mushroom = obj_to_room( mushroom, ch->in_room );
    return rNONE;
}


ch_ret spell_create_water( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    WEATHER_DATA *weath;
    int water;

    if ( obj->item_type != ITEM_DRINK_CON )
    {
	send_to_char( "It is unable to hold water.\n\r", ch );
	return rSPELL_FAILED;
    }

    if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
    {
	send_to_char( "It contains some other liquid.\n\r", ch );
	return rSPELL_FAILED;
    }
    
    weath = ch->in_room->area->weather;

    water = UMIN(
		level * (weath >= 0 ? 4 : 2),
		obj->value[0] - obj->value[1]
		);

    if ( water > 0 )
    {
	separate_obj(obj);
	obj->value[2] = LIQ_WATER;
	obj->value[1] += water;
	if ( !is_name( "water", obj->name ) )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "%s water", obj->name );
	    STRFREE( obj->name );
	    obj->name = STRALLOC( buf );
	}
	act( AT_MAGIC, "$p is filled.", ch, obj, NULL, TO_CHAR );
    }

    return rNONE;
}

ch_ret spell_cure_blindness( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    SKILLTYPE *skill = get_skilltype(sn);

    set_char_color( AT_MAGIC, ch );
    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
	immune_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    if ( !is_affected( victim, gsn_blindness ) )
    {
	if ( ch != victim )
	  send_to_char( "You work your cure, but it has no apparent effect.\n\r", ch );
	else
	  send_to_char( "You don't seem to be blind.\n\r", ch );
	return rSPELL_FAILED;
    }
    affect_strip( victim, gsn_blindness );
    set_char_color( AT_MAGIC, victim);
    send_to_char( "Your vision returns!\n\r", victim );
    if ( ch != victim )
	send_to_char( "You work your cure, restoring vision.\n\r", ch );
    return rNONE;
}

/* Next 3 are Paladin spells, until we update smaugspells -- Blodkai, 4/97 */

ch_ret spell_sacral_divinity( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;
    SKILLTYPE *skill = get_skilltype(sn);
 
    if ( ch->alignment < 350 ) {
      act( AT_MAGIC, "Your prayer goes unanswered.", ch, NULL, NULL, TO_CHAR );
      return rSPELL_FAILED;
    }
    if ( IS_SET( ch->immune, RIS_MAGIC ) ) {
      immune_casting( skill, ch, NULL, NULL );
      return rSPELL_FAILED;
    }
    if ( IS_AFFECTED( ch, AFF_SANCTUARY ) )
      return rSPELL_FAILED;
    af.type      = sn;
    af.duration  = level * 3;
    af.location  = APPLY_AFFECT;
    af.modifier  = 0;
    af.bitvector = meb(AFF_SANCTUARY);
    affect_to_char( ch, &af );
    act( AT_MAGIC, "A shroud of glittering light slowly wraps itself about $n.", ch, NULL, NULL, TO_ROOM);
    act( AT_MAGIC, "A shroud of glittering light slowly wraps itself around you.", ch, NULL, NULL, TO_CHAR );
    return rNONE;
}

ch_ret spell_expurgation( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    SKILLTYPE *skill = get_skilltype(sn);
 
    if ( IS_SET( victim->immune, RIS_MAGIC ) ) {
        immune_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }
    if ( !is_affected( victim, gsn_poison ) )
        return rSPELL_FAILED;
    affect_strip( victim, gsn_poison );
    act( AT_MAGIC, "You speak an ancient prayer, begging your god for purification.", ch, NULL, NULL, TO_CHAR );
    act( AT_MAGIC, "$n speaks an ancient prayer begging $s god for purification.", ch, NULL, NULL, TO_ROOM );
    return rNONE;
}

ch_ret spell_bethsaidean_touch( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    SKILLTYPE *skill = get_skilltype(sn);
 
    if ( IS_SET( victim->immune, RIS_MAGIC ) ) {
        immune_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }
    if ( !is_affected( victim, gsn_blindness ) )
        return rSPELL_FAILED;
    affect_strip( victim, gsn_blindness );
    set_char_color( AT_MAGIC, victim );
    send_to_char( "Your sight is restored!\n\r", victim );
    if ( ch != victim ) {
      act( AT_MAGIC, "$n lays $s hands over your eyes and concentrates...", ch, NULL, victim, TO_VICT );
      act( AT_MAGIC, "$n lays $s hands over $N's eyes and concentrates...", ch, NULL, victim, TO_NOTVICT );
      act( AT_MAGIC, "Laying your hands on $N's eyes, you pray to lift $S blindness.", ch, NULL, victim, TO_CHAR );
    }
    return rNONE;
}


ch_ret spell_cure_poison( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    SKILLTYPE *skill = get_skilltype(sn);
    int x=0;

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
	immune_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    if ( is_affected( victim, gsn_poison ) )
    {

	affect_strip( victim, gsn_poison );
	set_char_color( AT_MAGIC, victim);
	send_to_char( "A warm feeling runs through your body.\n\r", victim );
	x = victim->mental_state < 0 ? -x : x;
	victim->mental_state = URANGE( -25, victim->mental_state, 25 );
	if ( ch != victim )
	{
	  act( AT_MAGIC, "A flush of health washes over $N.", ch, NULL, victim, TO_NOTVICT );
	  act( AT_MAGIC, "You lift the poison from $N's body.", ch, NULL, victim, TO_CHAR );
	}
	return rNONE;
    }
    else
    {
	set_char_color( AT_MAGIC, ch );
	if ( ch != victim )
	  send_to_char( "You work your cure, but it has no apparent effect.\n\r", ch );
	else
	  send_to_char( "You don't seem to be poisoned.\n\r", ch );
	return rSPELL_FAILED;
    }
}

ch_ret spell_curse( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
	immune_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    if ( IS_AFFECTED(victim, AFF_CURSE) || saves_spell_staff( level, victim ) )
    {
	failed_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }
    af.type      = sn;
    af.duration  = (4*level) * DUR_CONV;
    af.location  = APPLY_HITROLL;
    af.modifier  = -1;
    af.bitvector = meb(AFF_CURSE);
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = 1;
    affect_to_char( victim, &af );

    set_char_color( AT_MAGIC, victim);
    send_to_char( "You feel unclean.\n\r", victim );
    if ( ch != victim )
    {
      act( AT_MAGIC, "You utter a curse upon $N.", ch, NULL, victim, TO_CHAR );
      act( AT_MAGIC, "$n utters a curse upon $N.", ch, NULL, victim, TO_NOTVICT );
    }
    return rNONE;
}


ch_ret spell_detect_poison( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    set_char_color( AT_MAGIC, ch);
    if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD 
	 || obj->item_type == ITEM_COOK)
    {
	if ( obj->item_type == ITEM_COOK && obj->value[2] == 0 )
	    send_to_char( "It looks undercooked.\n\r", ch );
	else if ( obj->value[3] != 0 )
	    send_to_char( "You smell poisonous fumes.\n\r", ch );
	else
	    send_to_char( "It looks very delicious.\n\r", ch );
    }
    else
    {
	send_to_char( "It doesn't look poisoned.\n\r", ch );
    }

    return rNONE;
}


ch_ret spell_dispel_evil( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( !IS_NPC(ch) && IS_EVIL(ch) )
	victim = ch;

    if ( IS_GOOD(victim) )
    {
	act( AT_MAGIC, "Thoric protects $N.", ch, NULL, victim, TO_ROOM );
	return rSPELL_FAILED;
    }

    if ( IS_NEUTRAL(victim) )
    {
	act( AT_MAGIC, "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
	return rSPELL_FAILED;
    }

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
	immune_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    dam = dice( level, 4 );
    if ( saves_spell_staff( level, victim ) )
	dam /= 2;
    return damage( ch, victim, dam, sn );
}


/*
 * New version of dispel magic fixes alot of bugs, and allows players
 * to not lose thie affects if they have the spell and the affect.
 * Also prints a message to the victim, and does various other things :)
 * Shaddai
 */

ch_ret
spell_dispel_magic (int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int cnt = 0, affect_num, affected_by = 0, times = 0;
  int chance;
  SKILLTYPE *skill = get_skilltype (sn);
  AFFECT_DATA *paf;
  bool found = FALSE, twice = FALSE, three = FALSE;
  bool is_mage = FALSE;

  set_char_color ( AT_MAGIC, ch );

  chance = ( get_curr_int ( ch ) - get_curr_int ( victim ) );

  if ( IS_SET ( victim->immune, RIS_MAGIC ) )
    {
      immune_casting ( skill, ch, victim, NULL );
      return rSPELL_FAILED;
    }

  if ( IS_NPC(ch) || ch->class == CLASS_MAGE )
        is_mage = TRUE;

  if ( is_mage )
        chance += 5;
  else 
	chance -= 15;

/* Bug Fix to prevent possesed mobs from being dispelled -Shaddai */
  if ( IS_NPC(victim) && IS_AFFECTED( victim, AFF_POSSESS) )
  {
      immune_casting ( skill, ch, victim, NULL );
      return rVICT_IMMUNE;
  }

  if ( ch == victim )
    {
      if ( ch->first_affect )
        {
          send_to_char ( "You pass your hands around your body...\n\r", ch );
          while ( ch->first_affect )
            affect_remove ( ch, ch->first_affect );
          if ( !IS_NPC ( ch ) ) /* Stop the NPC bug  Shaddai */
                update_aris( victim );
          return rNONE;
        }
      else
        {
          send_to_char ( "You pass your hands around your body...\n\r", ch );
          return rNONE;
        }
    }
  if ( !is_mage && !IS_AFFECTED ( ch, AFF_DETECT_MAGIC ))
  {
        send_to_char ("You don't sense a magical aura to dispel.\n\r", ch );
        return rERROR; /* You don't cast it so don't attack */
  }

  if ( number_percent() > ( 75 - chance ) )
  {
        twice = TRUE;
        if ( number_percent () > ( 75 - chance ) )
                three = TRUE;
  }

start_loop:

  /* Grab affected_by from mobs first */
  if ( IS_NPC( victim ) && !xIS_EMPTY(victim->affected_by) )
  {
        for( ;; ) {
                affected_by = number_range(0, MAX_AFFECTED_BY-1);
                if ( xIS_SET(victim->affected_by, affected_by) )
                {
                        found = TRUE;
                        break;
                }
                if ( cnt++ > 30 )
                {
                        found = FALSE;
                        break;
                }
        }
        if ( found ) /* Ok lets see if it is a spell */
        {
          for ( paf = victim->first_affect; paf; paf = paf->next )
                if ( xIS_SET(paf->bitvector, affected_by) )
                        break;
          if ( paf ) /*It is a spell lets remove the spell too */
          {

                if (level < victim->level || saves_spell_staff (level, victim))
                {
                    if ( !dispel_casting ( paf, ch, victim, FALSE, FALSE ) )
                        failed_casting ( skill, ch, victim, NULL );
                    return rSPELL_FAILED;
                }
                if ( SPELL_FLAG ( get_skilltype (paf->type), SF_NODISPEL))
                {
                    if ( !dispel_casting ( paf, ch, victim, FALSE, FALSE ) )
                        failed_casting ( skill, ch, victim, NULL );
                    return rSPELL_FAILED;
                }
                if ( !dispel_casting ( paf, ch, victim, FALSE, TRUE )
                     && times == 0 )
                        successful_casting( skill, ch, victim, NULL );
                affect_remove ( victim, paf );
                if ( (twice && times < 1) || (three && times < 2 ) )
                {
                        times++;
                        goto start_loop;
                }
                return rNONE;
          }
          else  /* Nope not a spell just remove the bit *For Mobs Only* */
          {
                if (level < victim->level || saves_spell_staff (level, victim))
                {
                    if ( !dispel_casting (NULL, ch, victim, affected_by,FALSE))
                        failed_casting ( skill, ch, victim, NULL );
                    return rSPELL_FAILED;
                }
                if ( !dispel_casting ( NULL, ch, victim, affected_by, TRUE )
                        && times == 0 )
                        successful_casting( skill, ch, victim, NULL );
		xREMOVE_BIT(victim->affected_by, affected_by);
                if ( (twice && times < 1) || (three && times < 2 ) )
                {
                        times++;
                        goto start_loop;
                }
                return rNONE;
          }
        }
  }

  /* Ok mob has no affected_by's or we didn't catch them lets go to
   * first_affect. SHADDAI
   */

  if ( !victim->first_affect )
    {
      failed_casting ( skill, ch, victim, NULL );
      return rSPELL_FAILED;
    }

  cnt = 0;

  /*
   * Need to randomize the affects, yes you have to loop on average 1.5 times
   * but dispel magic only takes at worst case 256 uSecs so who cares :)
   * Shaddai
   */

  for ( paf = victim->first_affect; paf; paf = paf->next )
        cnt++;

  paf = victim->first_affect;

  for ( affect_num = number_range (0, (cnt - 1));affect_num > 0; affect_num-- )
        paf = paf->next;

  if ( level < victim->level || saves_spell_staff ( level, victim ) )
    {
      if ( !dispel_casting ( paf, ch, victim, FALSE, FALSE ) )
         failed_casting ( skill, ch, victim, NULL );
      return rSPELL_FAILED;
    }

  /* Need to make sure we have an affect and it isn't no dispel */
  if ( !paf || SPELL_FLAG ( get_skilltype (paf->type), SF_NODISPEL))
  {
        if ( !dispel_casting ( paf, ch, victim, FALSE, FALSE ) )
             failed_casting ( skill, ch, victim, NULL );
        return rSPELL_FAILED;
  }
  if ( !dispel_casting ( paf, ch, victim, FALSE, TRUE )
        && times == 0 )
        successful_casting ( skill, ch, victim, NULL );
  affect_remove ( victim, paf );
  if ( (twice && times < 1) || (three && times < 2 ) )
  {
        times++;
        goto start_loop;
  }

 /* Have to reset victim affects */

 if ( !IS_NPC ( victim ) )
        update_aris( victim );
 return rNONE;
}



ch_ret spell_polymorph ( int sn, int level, CHAR_DATA *ch, void *vo )
{
  MORPH_DATA *morph;
  SKILLTYPE *skill = get_skilltype(sn);

  morph = find_morph( ch, target_name, TRUE );
  if ( !morph )
  {
    send_to_char("You can't morph into anything like that!\n\r", ch );
    return rSPELL_FAILED;
  }
  if ( !do_morph_char(ch, morph) )
  {
  	failed_casting( skill, ch, NULL, NULL );
	return rSPELL_FAILED;
  }
  return rNONE;
}

ch_ret spell_earthquake( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    bool ch_died;
    ch_ret retcode;
    SKILLTYPE *skill = get_skilltype(sn);

    ch_died = FALSE;
    retcode = rNONE;
    
    if ( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
	failed_casting( skill, ch, NULL, NULL );
	return rSPELL_FAILED;
    }

    act( AT_MAGIC, "The earth trembles beneath your feet!", ch, NULL, NULL, TO_CHAR );
    act( AT_MAGIC, "$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM );

    for ( vch = first_char; vch; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( !vch->in_room )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
           if ( IS_NPC( vch ) && xIS_SET( vch->act, ACT_MOBINVIS ) )
              continue;
            if ( !IS_NPC( vch ) && xIS_SET( vch->act, PLR_WIZINVIS )
                 && vch->pcdata->wizinvis >= LEVEL_IMMORTAL )
              continue;

	    if ( vch != ch && ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
	    &&  !IS_AFFECTED( vch, AFF_FLYING )
	    &&  !IS_AFFECTED( vch, AFF_FLOATING ) )
		retcode = damage( ch, vch, level + dice(2, 8), sn );
	    if ( retcode == rCHAR_DIED || char_died(ch) )
	    {
		ch_died = TRUE;
		continue;
	    }
	    if ( char_died(vch) )
	    	continue;
	}

	if ( !ch_died && vch->in_room->area == ch->in_room->area )
	{
            if ( number_bits( 3 ) == 0 )
	        send_to_char_color( "&BThe earth trembles and shivers.\n\r", vch );
	}
    }

    if ( ch_died )
      return rCHAR_DIED;
    else
      return rNONE;
}


ch_ret spell_enchant_weapon( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;

    if ( obj->item_type != ITEM_WEAPON
    ||   IS_OBJ_STAT(obj, ITEM_MAGIC)
    ||   obj->first_affect )
    {
      act( AT_MAGIC, "Your magic twists and winds around $p but cannot take hold.", ch, obj, NULL, TO_CHAR );
      act( AT_MAGIC, "$n's magic twists and winds around $p but cannot take hold.", ch, obj, NULL, TO_NOTVICT );
      return rSPELL_FAILED;
    }

    /* Bug fix here. -- Alty */
    separate_obj(obj);
    CREATE( paf, AFFECT_DATA, 1 );
    paf->type		= -1;
    paf->duration	= -1;
    paf->location	= APPLY_HITROLL;
    paf->modifier	= level / 15;
    xCLEAR_BITS(paf->bitvector);
    LINK( paf, obj->first_affect, obj->last_affect, next, prev );

    CREATE( paf, AFFECT_DATA, 1 );
    paf->type		= -1;
    paf->duration	= -1;
    paf->location	= APPLY_DAMROLL;
    paf->modifier	= level / 15;
    xCLEAR_BITS(paf->bitvector);
    LINK( paf, obj->first_affect, obj->last_affect, next, prev );

    if ( IS_GOOD(ch) )
    {
	xSET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
	act( AT_BLUE, "$p gleams with flecks of blue energy.", ch, obj, NULL, TO_ROOM );
	act( AT_BLUE, "$p gleams with flecks of blue energy.", ch, obj, NULL, TO_CHAR );
    }
    else if ( IS_EVIL(ch) )
    {
	xSET_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
	act( AT_BLOOD, "A crimson stain flows slowly over $p.", ch, obj, NULL, TO_CHAR );
	act( AT_BLOOD, "A crimson stain flows slowly over $p.", ch, obj, NULL, TO_ROOM );
    }
    else
    {
	xSET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
	xSET_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
	act( AT_YELLOW, "$p glows with a disquieting light.", ch, obj, NULL, TO_ROOM );
	act( AT_YELLOW, "$p glows with a disquieting light.", ch, obj, NULL, TO_CHAR );
    }
    return rNONE;
}

ch_ret spell_disenchant_weapon( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;

    if ( obj->item_type != ITEM_WEAPON )
    {
        send_to_char("You can only disenchant weapons.",ch);
	return rSPELL_FAILED;
    }

    if ( !IS_OBJ_STAT(obj, ITEM_MAGIC) || !obj->first_affect )
    {
        send_to_char("This weapon appears to have no enchantments on it.",ch);
	return rSPELL_FAILED;
    }

    if ( xIS_SET(obj->pIndexData->extra_flags, ITEM_MAGIC) )
    {
        send_to_char("You can't disenchant a weapon that's inherently magical.",ch);
	return rSPELL_FAILED;
    }

    if ( xIS_SET(obj->pIndexData->extra_flags, ITEM_ANTI_GOOD) 
    ||   xIS_SET(obj->pIndexData->extra_flags, ITEM_ANTI_EVIL) )
    {
        send_to_char("You can't disenchant a weapon that's inherently good or evil.",ch);
	return rSPELL_FAILED;
    }

    separate_obj(obj);
    for ( paf = obj->first_affect; paf; paf = paf->next)
    {
	if ( paf->location == APPLY_HITROLL
	||   paf->location == APPLY_DAMROLL )
	{
	    UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
	}
    }

    if ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) )
    {
	xREMOVE_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
	act( AT_BLUE, "$p momentarily absorbs all blue light around it.", ch, obj, NULL, TO_CHAR );
    }
    if ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) )
    {
	xREMOVE_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
	act( AT_RED, "$p momentarily absorbs all red light around it.", ch, obj, NULL, TO_CHAR );
    }

/*    send_to_char( "Ok.\n\r", ch );*/
    successful_casting( get_skilltype(sn), ch, NULL, obj );
    return rNONE;
}



/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
ch_ret spell_energy_drain( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    int chance;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
	immune_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    chance = ris_save( victim, victim->level, RIS_DRAIN );
    if ( chance == 1000 || saves_spell_staff( chance, victim ) ) 
    {
	failed_casting( skill, ch, victim, NULL ); /* SB */    
	return rSPELL_FAILED;
    }

    ch->alignment = UMAX(-1000, ch->alignment - 200);
    if ( victim->level <= 2 )
	dam		 = ch->hit + 1;
    else
    {
	gain_exp( victim, 0 - number_range( level / 2, 3 * level / 2 ) );
	victim->mana	/= 2;
	victim->move	/= 2;
	dam		 = dice(1, level);
	ch->hit		+= dam;
    }

    if ( ch->hit > ch->max_hit )
	ch->hit = ch->max_hit;
    return damage( ch, victim, dam, sn );
}



ch_ret spell_fireball( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] =
    {
	  0,
	  0,   0,   0,   0,   0,	  0,   0,   0,   0,   0,
	  0,   0,   0,   0,  30,	 35,  40,  45,  50,  55,
	 60,  65,  70,  75,  80,	 82,  84,  86,  88,  90,
	 92,  94,  96,  98, 100,	102, 104, 106, 108, 110,
    112, 114, 116, 118, 120,    122, 124, 126, 128, 130,
    132, 134, 136, 138, 140,    142, 144, 146, 148, 150,
    152, 154, 156, 158, 160,    162, 164, 166, 168, 170
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell_staff( level, victim ) )
	dam /= 2;
    return damage( ch, victim, dam, sn );
}



ch_ret spell_flamestrike( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(6, 8);
    if ( saves_spell_staff( level, victim ) )
	dam /= 2;
    return damage( ch, victim, dam, sn );
}



ch_ret spell_faerie_fire( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
	immune_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) )
    {
	failed_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }
    af.type      = sn;
    af.duration  = level * DUR_CONV;
    af.location  = APPLY_AC;
    af.modifier  = 2 * level;
    af.bitvector = meb(AFF_FAERIE_FIRE);
    affect_to_char( victim, &af );
    act( AT_PINK, "You are surrounded by a pink outline.", victim, NULL, NULL, TO_CHAR );
    act( AT_PINK, "$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM );
    return rNONE;
}



ch_ret spell_faerie_fog( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *ich;

    act( AT_MAGIC, "$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM );
    act( AT_MAGIC, "You conjure a cloud of purple smoke.", ch, NULL, NULL, TO_CHAR );

    for ( ich = ch->in_room->first_person; ich; ich = ich->next_in_room )
    {
        if ( IS_NPC( ich ) && xIS_SET( ich->act, ACT_MOBINVIS ) )
            continue;

	if ( !IS_NPC(ich) && xIS_SET(ich->act, PLR_WIZINVIS) )
	    continue;

	if ( ich == ch || saves_spell_staff( level, ich ) )
	    continue;

	affect_strip ( ich, gsn_invis			);
	affect_strip ( ich, gsn_mass_invis		);
	affect_strip ( ich, gsn_sneak			);
	xREMOVE_BIT   ( ich->affected_by, AFF_HIDE	);
	xREMOVE_BIT   ( ich->affected_by, AFF_INVISIBLE	);
	xREMOVE_BIT   ( ich->affected_by, AFF_SNEAK	);
	act( AT_MAGIC, "$n is revealed!", ich, NULL, NULL, TO_ROOM );
	act( AT_MAGIC, "You are revealed!", ich, NULL, NULL, TO_CHAR );
    }
    return rNONE;
}


ch_ret spell_gate( int sn, int level, CHAR_DATA *ch, void *vo )
{
    MOB_INDEX_DATA *temp;
    if ( (temp = get_mob_index( MOB_VNUM_VAMPIRE ) ) == NULL )
    {
	bug ("Spell_gate: Vampire vnum %d doesn't exist.", MOB_VNUM_VAMPIRE);
	return rSPELL_FAILED;
    }
    char_to_room( create_mobile( temp ),
	ch->in_room );
    return rNONE;
}


ch_ret spell_harm( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
	immune_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    dam = UMAX(  20, victim->hit - dice(1,4) );
    if ( saves_spell_staff( level, victim ) )
	dam = UMIN( 50, dam / 4 );
    dam = UMIN( 100, dam );
    return damage( ch, victim, dam, sn );
}


ch_ret spell_identify( int sn, int level, CHAR_DATA *ch, void *vo )
{
/* Modified by Scryn to work on mobs/players/objs */
/* Made it show short descrs instead of keywords, seeing as you need
   to know the keyword anyways, we may as well make it look nice -- Alty */
    OBJ_DATA *obj;
    CHAR_DATA *victim;
    AFFECT_DATA *paf;
    SKILLTYPE *sktmp;
    SKILLTYPE *skill = get_skilltype(sn);
    char *name;

    if ( target_name[0] == '\0' )
    {
      send_to_char( "What should the spell be cast upon?\n\r", ch );
      return rSPELL_FAILED;
    }

    if ( ( obj = get_obj_carry( ch, target_name ) ) != NULL )
    {
	set_char_color( AT_LBLUE, ch );
	ch_printf( ch,
	"\n\rObject '%s' is %s",
/*		obj->name,*/
		obj->short_descr,
		aoran( item_type_name( obj ) ) );
	if ( obj->item_type != ITEM_LIGHT && obj->wear_flags-1 > 0 )
  	  ch_printf( ch, ", with wear location:  %s\n\r",
		flag_string(obj->wear_flags-1, w_flags) );
	else
	  send_to_char( ".\n\r", ch );
	ch_printf( ch,
	"Special properties:  %s\n\rIts weight is %d, value is %d, and level is %d.\n\r",
		extra_bit_name( &obj->extra_flags ),
	/*	magic_bit_name( obj->magic_flags ), -- unused for now */
		obj->weight,
		obj->cost,
		obj->level );
	if ( obj->owner[0] != '\0' )
		ch_printf_color( ch, "&cOwner: &Y%s\n\r", obj->owner);
	set_char_color( AT_MAGIC, ch );

    switch ( obj->item_type )
    {
    case ITEM_CONTAINER:
	ch_printf( ch, "%s appears to be %s.\n\r", capitalize(obj->short_descr),
		obj->value[0] < 76  ? "of a small capacity"           :
		obj->value[0] < 150 ? "of a small to medium capacity" :
		obj->value[0] < 300 ? "of a medium capacity"          :
		obj->value[0] < 550 ? "of a medium to large capacity" :
		obj->value[0] < 751 ? "of a large capacity"           :
				      "of a giant capacity" );
	break;	

    case ITEM_PILL:
    case ITEM_SCROLL:
    case ITEM_POTION:
	ch_printf( ch, "Level %d spells of:", obj->value[0] );

	if ( obj->value[1] >= 0 && (sktmp=get_skilltype(obj->value[1])) != NULL )
	{
	    send_to_char( " '", ch );
	    send_to_char( sktmp->name, ch );
	    send_to_char( "'", ch );
	}

	if ( obj->value[2] >= 0 && (sktmp=get_skilltype(obj->value[2])) != NULL )
	{
	    send_to_char( " '", ch );
	    send_to_char( sktmp->name, ch );
	    send_to_char( "'", ch );
	}

	if ( obj->value[3] >= 0 && (sktmp=get_skilltype(obj->value[3])) != NULL )
	{
	    send_to_char( " '", ch );
	    send_to_char( sktmp->name, ch );
	    send_to_char( "'", ch );
	}

	send_to_char( ".\n\r", ch );
	break;

    case ITEM_SALVE:
	ch_printf( ch, "Has %d(%d) applications of level %d",
	  obj->value[1], obj->value[2], obj->value[0] );
	if ( obj->value[4] >= 0 && (sktmp=get_skilltype(obj->value[4])) != NULL )
        {
            send_to_char( " '", ch );
            send_to_char( sktmp->name, ch );
            send_to_char( "'", ch );
        }
	if ( obj->value[5] >= 0 && (sktmp=get_skilltype(obj->value[5])) != NULL )
        {
            send_to_char( " '", ch );
            send_to_char( sktmp->name, ch );
            send_to_char( "'", ch );
        }
	send_to_char( ".\n\r", ch );
	break;
                      
    case ITEM_WAND:
    case ITEM_STAFF:
	ch_printf( ch, "Has %d(%d) charges of level %d",
	    obj->value[1], obj->value[2], obj->value[0] );

	if ( obj->value[3] >= 0 && (sktmp=get_skilltype(obj->value[3])) != NULL )
	{
	    send_to_char( " '", ch );
	    send_to_char( sktmp->name, ch );
	    send_to_char( "'", ch );
	}

	send_to_char( ".\n\r", ch );
	break;

    case ITEM_WEAPON:
	ch_printf( ch, "Damage is %d to %d (average %d)%s\n\r",
	    obj->value[1], obj->value[2],
	    ( obj->value[1] + obj->value[2] ) / 2,
	    IS_OBJ_STAT( obj, ITEM_POISONED) ? ", and is poisonous." :
	    "." );
	break;

    case ITEM_ARMOR:
	ch_printf( ch, "Armor class is %d.\n\r", obj->value[0] );
	break;
    }

    for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
	showaffect( ch, paf );

    for ( paf = obj->first_affect; paf; paf = paf->next )
	showaffect( ch, paf );

    return rNONE;
    }

    else if ( ( victim = get_char_room( ch, target_name ) ) != NULL )
    {

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
	immune_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    /* If they are morphed or a NPC use the appropriate short_desc otherwise
     * use their name -- Shaddai
     */

    if ( victim->morph && victim->morph->morph )
	name = capitalize(victim->morph->morph->short_desc);
    else if ( IS_NPC(victim) )
    	name = capitalize(victim->short_descr);
    else
    	name = victim->name;
   
    ch_printf(ch, "%s appears to be between level %d and %d.\n\r",
	name,
	victim->level - (victim->level % 5), 
        victim->level - (victim->level % 5) + 5);
 
    if ( IS_NPC(victim) && victim->morph )
	ch_printf(ch,"%s appears to truly be %s.\n\r",
	name, 
	(ch->level > victim->level + 10) 
	 ? victim->name : "someone else");

    ch_printf(ch,"%s looks like %s, and follows the ways of the %s.\n\r",
     	name, aoran(get_race(victim)), get_class(victim));

    if ( (chance(ch, 50) && ch->level >= victim->level + 10 )
    ||    IS_IMMORTAL(ch) )
    {
    ch_printf(ch, "%s appears to be affected by: ", name);

    if (!victim->first_affect)
    {
      send_to_char( "nothing.\n\r", ch );
      return rNONE;
    }

      for ( paf = victim->first_affect; paf; paf = paf->next )
      {
        if (victim->first_affect != victim->last_affect)
        {
          if( paf != victim->last_affect && (sktmp=get_skilltype(paf->type)) != NULL )
            ch_printf( ch, "%s, ", sktmp->name );

	  if( paf == victim->last_affect && (sktmp=get_skilltype(paf->type)) != NULL )
	  {
	    ch_printf( ch, "and %s.\n\r", sktmp->name );
	    return rNONE;
	  }
        }
        else
	{
	  if ( (sktmp=get_skilltype(paf->type)) != NULL )
	    ch_printf( ch, "%s.\n\r", sktmp->name );
	  else
	    send_to_char( "\n\r", ch );
	  return rNONE;
	}
      }
    }
  }
  
  else
  {
    ch_printf(ch, "You can't find %s!\n\r", target_name );
    return rSPELL_FAILED;
  }
  return rNONE;
}



ch_ret spell_invis( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    SKILLTYPE *skill = get_skilltype(sn);

/* Modifications on 1/2/96 to work on player/object - Scryn */

    if (target_name[0] == '\0')
	victim = ch;
    else
	victim = get_char_room(ch, target_name);

    if( victim )
    {
	AFFECT_DATA af;

	if ( IS_SET( victim->immune, RIS_MAGIC ) )
	{
	    immune_casting( skill, ch, victim, NULL );
	    return rSPELL_FAILED;
	}

	if ( IS_AFFECTED(victim, AFF_INVISIBLE) )
	{
	    failed_casting( skill, ch, victim, NULL );
	    return rSPELL_FAILED;
	}

	act( AT_MAGIC, "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );
	af.type      = sn;
	af.duration  = ((level / 4) + 12) * DUR_CONV;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = meb(AFF_INVISIBLE);
	affect_to_char( victim, &af );
	act( AT_MAGIC, "You fade out of existence.", victim, NULL, NULL, TO_CHAR );
	return rNONE;
    }
    else
    {
	OBJ_DATA *obj;

	obj = get_obj_carry( ch, target_name );

	if (obj)
	{
	    separate_obj(obj); /* Fix multi-invis bug --Blod */
	    if ( IS_OBJ_STAT(obj, ITEM_INVIS) 
	    ||   chance(ch, 40 + level / 10))
            {
		failed_casting( skill, ch, NULL, NULL );
		return rSPELL_FAILED;
	    }

	    xSET_BIT( obj->extra_flags, ITEM_INVIS );
	    act( AT_MAGIC, "$p fades out of existence.", ch, obj, NULL, TO_CHAR );
	    return rNONE;
	}
    }
    ch_printf(ch, "You can't find %s!\n\r", target_name);
    return rSPELL_FAILED;
}



ch_ret spell_know_alignment( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char *msg;
    int ap;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( !victim )
    {
	failed_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
	immune_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    ap = victim->alignment;

	 if ( ap >  700 ) msg = "$N has an aura as white as the driven snow.";
    else if ( ap >  350 ) msg = "$N is of excellent moral character.";
    else if ( ap >  100 ) msg = "$N is often kind and thoughtful.";
    else if ( ap > -100 ) msg = "$N doesn't have a firm moral commitment.";
    else if ( ap > -350 ) msg = "$N lies to $S friends.";
    else if ( ap > -700 ) msg = "$N would just as soon kill you as look at you.";
    else msg = "I'd rather just not say anything at all about $N.";

    act( AT_MAGIC, msg, ch, NULL, victim, TO_CHAR );
    return rNONE;
}


ch_ret spell_lightning_bolt( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] =
    {
	 0,
	 0,  0,  0,  0,  0,	 0,  0,  0, 25, 28,
	31, 34, 37, 40, 40,	41, 42, 42, 43, 44,
	44, 45, 46, 46, 47,	48, 48, 49, 50, 50,
	51, 52, 52, 53, 54,	54, 55, 56, 56, 57,
    58, 58, 59, 60, 60, 61, 62, 62, 63, 64,
    64, 65, 65, 66, 66, 67, 68, 68, 69, 69,
    70, 71, 71, 72, 72, 73, 73, 74, 75, 75
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell_staff( level, victim ) )
	dam /= 2;
    return damage( ch, victim, dam, sn );
}

/*

ch_ret spell_locate_object( int sn, int level, CHAR_DATA *ch, void *vo )
{
    char buf[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    int cnt, found = 0;

    for ( obj = first_object; obj; obj = obj->next )
    {
	if ( !can_see_obj( ch, obj ) || !nifty_is_name( target_name, obj->name ) )
	    continue;
	if ( (IS_OBJ_STAT( obj, ITEM_PROTOTYPE )
	||    IS_OBJ_STAT( obj, ITEM_NOLOCATE  ) )
	&&   !IS_IMMORTAL(ch) )
	    continue;

	found++;

	for ( cnt = 0, in_obj = obj;
	      in_obj->in_obj && cnt < 100;
	      in_obj = in_obj->in_obj, ++cnt )
	    ;
	if ( cnt >= MAX_NEST )
	{
	    sprintf( buf, "spell_locate_obj: object [%d] %s is nested more than %d times!",
		obj->pIndexData->vnum, obj->short_descr, MAX_NEST );
	    bug( buf, 0 );
	    continue;
	}

	if ( in_obj->carried_by )
	{
	    if ( IS_IMMORTAL( in_obj->carried_by )
	    &&  !IS_NPC( in_obj->carried_by ) )
 	    && ( get_trust( ch ) < in_obj->carried_by->pcdata->wizinvis )
            &&  xIS_SET( in_obj->carried_by->act, PLR_WIZINVIS ) )
	     {
		found--;
	      	continue;
	     }

	    sprintf( buf, "%s carried by %s.\n\r",
		   obj_short(obj), PERS(in_obj->carried_by, ch) );
	}
	else
	{
	    sprintf( buf, "%s in %s.\n\r",
		obj_short(obj), in_obj->in_room == NULL
		    ? "somewhere" : in_obj->in_room->name );
	}

	buf[0] = UPPER(buf[0]);
        set_char_color( AT_MAGIC, ch );
        send_to_char( buf, ch );
    }

    if ( !found )
    {
	send_to_char( "Nothing like that exists.\n\r", ch );
	return rSPELL_FAILED;
    }
    return rNONE;
}
*/


bool can_locate_obj( CHAR_DATA *ch, OBJ_DATA *obj ) {
   if ( IS_OBJ_STAT( obj, ITEM_NOLOCATE ) )
      return FALSE;
   if ( obj->item_type != ITEM_CORPSE_PC && !CAN_WEAR( obj, ITEM_TAKE )
        && ch->level < sysdata.level_getobjnotake )
      return FALSE;
   if ( !IS_IMMORTAL( ch ) && IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
      return FALSE;
   if ( xIS_SET( ch->act, PLR_HOLYLIGHT ) )
      return TRUE;
   if ( IS_OBJ_STAT( obj, ITEM_BURIED ) || IS_OBJ_STAT( obj, ITEM_HIDDEN ) )
      return FALSE;
   if ( IS_AFFECTED( ch, AFF_TRUESIGHT ) )
      return TRUE;
   if ( IS_AFFECTED( ch, AFF_BLIND ) )
      return FALSE;
   if ( IS_OBJ_STAT( obj, ITEM_INVIS ) && !IS_AFFECTED( ch, AFF_DETECT_INVIS ) )
      return FALSE;
   return TRUE;
   }

bool can_locate_mob( CHAR_DATA *ch, CHAR_DATA *victim ) {
   bool ch_immort;

   /* to remove if we want caster to not locate hmself */
   if ( ch == victim )
      return TRUE;
   ch_immort = IS_IMMORTAL( ch );
   if ( IS_NPC( victim ) ) {
      if ( xIS_SET( victim->act, ACT_MOBINVIS ) && get_trust( ch ) < victim->mobinvis )
         return FALSE;
      if ( xIS_SET( victim->act, ACT_NOLOCATE ) )
         return FALSE;
      }
   else {
      if ( xIS_SET( victim->act, PLR_WIZINVIS ) && get_trust( ch ) < victim->pcdata->wizinvis )
         return FALSE;
      if ( victim->timer > 1 && !IS_NPC( ch ) && IS_PKILL( victim ) && !ch_immort && !victim->desc )
         return FALSE;
      if ( IS_IMMORTAL( victim ) && !ch_immort )
         return FALSE;
      }
   if ( xIS_SET( ch->act, PLR_HOLYLIGHT ) )
      return TRUE;
   if ( !IS_AFFECTED( ch, AFF_TRUESIGHT ) ) {
      if ( IS_AFFECTED( ch, AFF_BLIND) )
         return FALSE;
      /* if ( room_is_dark( victim->in_room ) && !IS_AFFECTED(ch, AFF_INFRARED) )
         return FALSE; */
      if ( IS_AFFECTED( victim, AFF_INVISIBLE ) && !IS_AFFECTED( ch, AFF_DETECT_INVIS ) )
         return FALSE;
      if ( IS_AFFECTED( victim, AFF_HIDE )
      && !IS_AFFECTED( ch, AFF_DETECT_HIDDEN )
      && ( IS_NPC( ch ) != IS_NPC( victim ) ) )
         return FALSE;
      }
   if( NOT_AUTHED( victim ) && !ch_immort )
      return FALSE;
   return TRUE;
   }

typedef enum { CTN_ROOM, CTN_MOB } ct_type;

typedef struct keyw_node KEYW_NODE;
struct keyw_node {
   KEYW_NODE *next;
   char *key;
   bool ok;
   };

KEYW_NODE *keyw_hash[1024];
KEYW_NODE *keyw_vec;
int keyw_num;

typedef struct pairs_node PAIRS_NODE;
struct pairs_node {
   PAIRS_NODE *next;
   char *desc;
   char *cont;
   int num;
   ct_type ct;
   };

PAIRS_NODE *pairs_hash[512];
PAIRS_NODE *pairs_vec;
int pairs_num;
int pair_overflow;

typedef struct print_node PRINT_NODE;
struct print_node {
   PRINT_NODE *next;
   PAIRS_NODE *obj;
   };

PRINT_NODE *print_hash[512];
PRINT_NODE *print_vec;
int print_num;

extern int top_obj_index;

char * cont_descr( void * cont, ct_type t ) {
   CHAR_DATA * mob;

   if ( !cont )
      return NULL;
   if ( t == CTN_ROOM )
      return ( ( ROOM_INDEX_DATA * ) cont )->name;
   mob = cont;
   return IS_NPC( mob ) ? mob->short_descr : mob->name;
   }

/* a _should_ be a 32 bits integer. Returns a 10 bits key */
#define iKEYW( a )   ( ( ( ( int ) a >> 2 ) ^ ( ( int ) a >> 12 ) ^ ( ( int ) a >> 22 ) ) & 1023 )
/* Like iKEYW, but returns a 9 bits key */
#define iKEYP( a )   ( ( ( ( int ) a >> 2 ) ^ ( ( int ) a >> 11 ) ^ ( ( int ) a >> 20 ) ) & 511 )

#define MAX_PAIRS 2000

void build_pairs( CHAR_DATA *ch, OBJ_DATA *obj, void *ctn, ct_type ctp, char *ct_name, int nest ) {
   KEYW_NODE *keyw;
   PAIRS_NODE *pair;
   int idx, pkey;
   
   if ( nest > 100 ) {
      log_string( "Pairs nest level exceeded." );
      return;
      }
   for ( ; obj ; obj = obj->next_content ) {
      if ( can_locate_obj( ch, obj ) ) {
         idx = iKEYW( obj->name );
         for ( keyw = keyw_hash[idx] ; keyw ; keyw = keyw->next )
            if ( keyw->key == obj->name )
               break;
         if ( !keyw && keyw_num < top_obj_index ) {
            keyw = keyw_vec + keyw_num++;
            keyw->key = obj->name;
            keyw->ok = nifty_is_name( target_name, obj->name );
            keyw->next = keyw_hash[idx];
            keyw_hash[idx] = keyw;
            }
         if ( keyw ? keyw->ok : nifty_is_name( target_name, obj->name ) ) {
            pkey = ( int ) ( obj->short_descr ) ^ ( int ) ct_name ^ ( ctp << 2 );
            idx = iKEYP( pkey );
            for ( pair = pairs_hash[idx] ; pair ; pair = pair->next )
               if ( pair->desc == obj->short_descr && pair->cont == ct_name && pair->ct == ctp )
                  break;
            if ( !pair && pairs_num < MAX_PAIRS ) {
               pair = pairs_vec + pairs_num++;
               pair->desc = obj->short_descr;
               pair->cont = ct_name;
               pair->ct = ctp;
               pair->num = obj->count;
               pair->next = pairs_hash[idx];
               pairs_hash[idx] = pair;
               }
            else if ( pair )
               pair->num += obj->count;
            else
               pair_overflow++;
            }
         if ( obj->first_content )
            build_pairs( ch, obj->first_content, ctn, ctp, ct_name, nest + 1 );
         }  
      }
   }

ch_ret spell_locate_object( int sn, int level, CHAR_DATA *ch, void *vo ) {
   extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
   ROOM_INDEX_DATA *room;
   CHAR_DATA *mob;
   PRINT_NODE *objprn;
   PAIRS_NODE *locprn, *nextpair;
   int x, objnum, hitnum, idx;
   char msg[2048];

   for ( x = 0 ; x < 1024 ; x++ )
      keyw_hash[x] = NULL;
   keyw_vec = malloc( top_obj_index * sizeof( KEYW_NODE ) );
   keyw_num = 0;
   for ( x = 0 ; x < 512 ; x++ )
      pairs_hash[x] = NULL;
   pairs_vec = malloc( MAX_PAIRS * sizeof( PAIRS_NODE ) );
   pairs_num = 0;
   if ( !keyw_vec || !pairs_vec ) {
      if ( keyw_vec )
         free( keyw_vec );
      if ( pairs_vec )
         free( pairs_vec );
      set_char_color( AT_MAGIC, ch );
      send_to_char( "You failed.\n\r", ch );
      sprintf( msg, "Locate object low on memory for %s.", ch->name );
      log_string( msg );
      return rSPELL_FAILED;
      }
   pair_overflow = 0;
   for ( x = 0; x < MAX_KEY_HASH; x++ ) {
      for ( room = room_index_hash[x] ; room ; room = room->next ) {
         if ( xIS_SET( room->room_flags, ROOM_NOLOCATE ) 
			|| xIS_SET( room->room_flags, ROOM_CLANSTOREROOM )
			|| xIS_SET( room->room_flags, ROOM_HOUSE ) )
            continue;
         build_pairs( ch, room->first_content, room, CTN_ROOM, room->name, 1 );
         for ( mob = room->first_person ; mob ; mob = mob->next_in_room )
            if ( can_locate_mob( ch, mob ) )
               build_pairs( ch, mob->first_carrying, mob, CTN_MOB, cont_descr( mob, CTN_MOB ), 1 );
         }
      }
   if ( pair_overflow ) {
      sprintf( msg, "Pair overflowed by %d locating %s.", pair_overflow, target_name );
      log_string( msg );
      }
   free( keyw_vec );
   for ( x = 0 ; x < 512 ; x++ )
      print_hash[x] = NULL;
   print_vec = malloc( MAX_PAIRS * sizeof( PRINT_NODE ) );
   print_num = 0;
   if ( !print_vec ) {
      free( pairs_vec );
      set_char_color( AT_MAGIC, ch );
      send_to_char( "You failed.\n\r", ch );
      sprintf( msg, "Locate object low on memory for %s.", ch->name );
      log_string( msg );
      return rSPELL_FAILED;
      }
   for ( x = 0 ; x < 512 ; x++ )
      for ( locprn = pairs_hash[x] ; locprn ; locprn = nextpair ) {
         nextpair = locprn->next;
         idx = iKEYP( locprn->desc );
         for ( objprn = print_hash[idx] ; objprn ; objprn = objprn->next )
            if ( objprn->obj->desc == locprn->desc )
               break;
         if ( !objprn ) {
            objprn = print_vec + print_num++;
            objprn->next = print_hash[idx];
            print_hash[idx] = objprn;
            locprn->next = NULL;
            objprn->obj = locprn;
            }
         else {
            locprn->next = objprn->obj;
            objprn->obj = locprn;
            }
         }
   objnum = hitnum = 0;
   set_char_color( AT_MAGIC, ch );
   send_to_char( "For a brief moment, your awareness expands to encompass the whole world...\n\r", ch );
   for ( x = 0 ; x < 512 ; x++ )
      for ( objprn = print_hash[x] ; objprn ; objprn = objprn->next ) {
         bool multi;

         if ( !objnum )
            send_to_char( "...and you find to be in existence:\n\r\n\r", ch );
         send_to_char( objprn->obj->desc, ch );
         multi = FALSE;
         for ( locprn = objprn->obj ; locprn ; locprn = locprn->next ) {
            if ( locprn == objprn->obj && locprn->next ) {
               send_to_char( ":\n\r", ch );
               multi = TRUE;
               }
            ch_printf( ch,
               /* multi ? "  %3d%s%s\n\r" : " (%d)%s%s\n\r",
               locprn->num, */
               multi ? "  %s%s\n\r" : "%s%s\n\r",
               locprn->ct == CTN_ROOM ? " in " : " on ",
               locprn->cont );
               hitnum++;
            }
         objnum++;
         }
   free( print_vec );
   free( pairs_vec );
   if ( !objnum ) {
      send_to_char( "...but nothing like that exists.\n\r", ch );
      return rSPELL_FAILED;
      }
   /* Locate is eating too much cpu time. */
   if ( IS_IMMORTAL( ch ) )
      ch_printf( ch, "\n\rFound %d instances of %d objects.\n\r", hitnum, objnum );
   else if ( hitnum ) {
      x = ( hitnum * 3 ) / ( get_curr_wis( ch ) + get_curr_int( ch ) );
      if ( x ) {
         if ( x < 3 )
            ;
         else if ( x < 10 )
            send_to_char( "\n\r... your mind is unable to cope with this much information...\n\r", ch );
         else if ( x < 20 )
            send_to_char( "\n\r...your mind is SHATTERED by the flood of information!\n\r", ch );
         else {
            send_to_char( "\n\r...ENOUGH!  Your mortal mind was not meant for omniscience.\n\r", ch );
            send_to_char( "What is left of it may need time to recover from this ordeal...\n\r", ch );
            }
	x = ch->mental_state < 0 ? -x : x;
        ch->mental_state = URANGE( -ch->level, ch->mental_state + x/5, ch->level );
         }
      }
   return rNONE;
   }





ch_ret spell_magic_missile( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 3,  3,  4,  4,  5,	 6,  6,  6,  6,  6,
	 7,  7,  7,  7,  7,	 8,  8,  8,  8,  8,
	 9,  9,  9,  9,  9,	10, 10, 10, 10, 10,
	11, 11, 11, 11, 11,	12, 12, 12, 12, 12,
    13, 13, 13, 13, 13, 14, 14, 14, 14, 14,
    15, 15, 15, 15, 15, 16, 16, 16, 16, 16,
    17, 17, 17, 17, 17, 18, 18, 18, 18, 18
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    /*  What's this?  You can't save vs. magic missile!		-Thoric
    if ( saves_spell( level, victim ) )
	dam /= 2;
    */
    return damage( ch, victim, dam, sn );
}




ch_ret spell_pass_door( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
	immune_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    if ( IS_AFFECTED(victim, AFF_PASS_DOOR) )
    {
	failed_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }
    af.type      = sn;
    af.duration  = number_fuzzy( level / 4 ) * DUR_CONV;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = meb(AFF_PASS_DOOR);
    affect_to_char( victim, &af );
    act( AT_MAGIC, "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
    act( AT_MAGIC, "You turn translucent.", victim, NULL, NULL, TO_CHAR );
    return rNONE;
}



ch_ret spell_poison( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int chance;
    bool first = TRUE;

    chance = ris_save( victim, level, RIS_POISON );
    if ( chance == 1000 || saves_poison_death( chance, victim ) )
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "Your magic fails to take hold.\n\r", ch );
	return rSPELL_FAILED;
    }
    if ( IS_AFFECTED( victim, AFF_POISON ) )
	first = FALSE;
    /* af.type      = sn; -Only till I fix SN's */
    af.type	 = gsn_poison;
    af.duration  = level * DUR_CONV;
    af.location  = APPLY_STR;
    af.modifier  = -2;
    af.bitvector = meb(AFF_POISON);
    affect_join( victim, &af );
    set_char_color( AT_GREEN, victim );
    send_to_char( "You feel very sick.\n\r", victim );
    if ( IS_PKILL( victim) )
      victim->mental_state = URANGE( 10, victim->mental_state
			   + (first ? 5 : 0), 100 );
    else
      victim->mental_state = URANGE( 20, victim->mental_state
  			   + (first ? 5 : 0), 100 );
    if ( ch != victim )
    {
      act( AT_GREEN, "$N shivers as your poison spreads through $S body.", ch, NULL, victim, TO_CHAR );
      act( AT_GREEN, "$N shivers as $n's poison spreads through $S body.", ch, NULL, victim, TO_NOTVICT );
    }
    return rNONE;
}


ch_ret spell_remove_curse( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
	immune_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    if ( is_affected( victim, gsn_curse ) )
    {
	affect_strip( victim, gsn_curse );
	set_char_color( AT_MAGIC, victim );
	send_to_char( "The weight of your curse is lifted.\n\r", victim );
	if ( ch != victim )
	{
	    act( AT_MAGIC, "You dispel the curses afflicting $N.", ch, NULL, victim, TO_CHAR );
	    act( AT_MAGIC, "$n's dispels the curses afflicting $N.", ch, NULL, victim, TO_NOTVICT );
	}
    }
    else
    if ( victim->first_carrying )
    {
	for ( obj = victim->first_carrying; obj; obj = obj->next_content )
	   if ( !obj->in_obj
	   && (IS_OBJ_STAT( obj, ITEM_NOREMOVE )
	    || IS_OBJ_STAT( obj, ITEM_NODROP ) ) )
	   {
	      if ( IS_OBJ_STAT( obj, ITEM_NOREMOVE ) )
		xREMOVE_BIT( obj->extra_flags, ITEM_NOREMOVE );
	      if ( IS_OBJ_STAT( obj, ITEM_NODROP ) )
		xREMOVE_BIT( obj->extra_flags, ITEM_NODROP );
	      set_char_color( AT_MAGIC, victim );
	      send_to_char( "You feel a burden released.\n\r", victim );
	      if ( ch != victim )
	      {
		act( AT_MAGIC, "You dispel the curses afflicting $N.", ch, NULL, victim, TO_CHAR );
		act( AT_MAGIC, "$n's dispels the curses afflicting $N.", ch, NULL, victim, TO_NOTVICT );
	      }
	      return rNONE;
	   }
    }
    return rNONE;
}

ch_ret spell_remove_trap( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj;
    OBJ_DATA *trap;
    bool found;
    int retcode;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( !target_name || target_name[0] == '\0' )
    {
       send_to_char( "Remove trap on what?\n\r", ch );
       return rSPELL_FAILED;
    }

    found = FALSE;

    if ( !ch->in_room->first_content )
    {
       send_to_char( "You can't find that here.\n\r", ch );
       return rNONE;
    }

    for ( obj = ch->in_room->first_content; obj; obj = obj->next_content )
       if ( can_see_obj( ch, obj ) && nifty_is_name( target_name, obj->name ) )
       {
	  found = TRUE;
	  break;
       }

    if ( !found )
    {
       send_to_char( "You can't find that here.\n\r", ch );
       return rSPELL_FAILED;
    }

    if ( (trap = get_trap( obj )) == NULL )
    {
	failed_casting( skill, ch, NULL, NULL );
	return rSPELL_FAILED;
    }


    if ( !chance(ch, 70 + get_curr_wis(ch)) )
    {
       send_to_char( "Ooops!\n\r", ch );
       retcode = spring_trap(ch, trap);
       if ( retcode == rNONE )
         retcode = rSPELL_FAILED;
       return retcode;
    }

    extract_obj( trap );

    successful_casting( skill, ch, NULL, NULL );
    return rNONE;
}


ch_ret spell_shocking_grasp( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const int dam_each[] =
    {
	 0,
	 0,  0,  0,  0,  0,	 0, 20, 25, 29, 33,
	36, 39, 39, 39, 40,	40, 41, 41, 42, 42,
	43, 43, 44, 44, 45,	45, 46, 46, 47, 47,
	48, 48, 49, 49, 50,	50, 51, 51, 52, 52,
    53, 53, 54, 54, 55, 55, 56, 56, 57, 57,
    58, 58, 59, 59, 60, 60, 61, 61, 62, 62,
    63, 63, 64, 64, 65, 65, 66, 66, 67, 67
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell_staff( level, victim ) )
	dam /= 2;
    return damage( ch, victim, dam, sn );
}



ch_ret spell_sleep( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;
    int retcode;
    int chance;
    int tmp;
    CHAR_DATA *victim;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return rSPELL_FAILED;
    }

    if ( !IS_NPC(victim) && victim->fighting )
    {
	send_to_char( "You cannot sleep a fighting player.\n\r", ch );
	return rSPELL_FAILED;
    }

    if ( is_safe(ch, victim, TRUE) )
        return rSPELL_FAILED;

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
	immune_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    if ( SPELL_FLAG(skill, SF_PKSENSITIVE)
    &&  !IS_NPC(ch) && !IS_NPC(victim) )
	tmp = level/2;
    else
	tmp = level;

    if ( IS_AFFECTED(victim, AFF_SLEEP)
    ||	(chance=ris_save(victim, tmp, RIS_SLEEP)) == 1000
    ||   level < victim->level
    ||  (victim != ch && xIS_SET(victim->in_room->room_flags, ROOM_SAFE))
    ||   saves_spell_staff( chance, victim ) )
    {
	failed_casting( skill, ch, victim, NULL );
	if ( ch == victim )
	  return rSPELL_FAILED;
	if ( !victim->fighting )
	{
	  retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
	  if ( retcode == rNONE )
	    retcode = rSPELL_FAILED;
	  return retcode;
	}
    }
    af.type      = sn;
    af.duration  = (4 + level) * DUR_CONV;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = meb(AFF_SLEEP);
    affect_join( victim, &af );

    /* Added by Narn at the request of Dominus. */
    if ( !IS_NPC( victim ) )
    {
	sprintf( log_buf, "%s has cast sleep on %s.", ch->name, victim->name );
	log_string_plus( log_buf, LOG_NORMAL, ch->level );
	to_channel( log_buf, CHANNEL_MONITOR, "Monitor", UMAX( LEVEL_IMMORTAL, ch->level ) );
    }

    if ( IS_AWAKE(victim) )
    {
	act( AT_MAGIC, "You feel very sleepy ..... zzzzzz.", victim, NULL, NULL, TO_CHAR );
	act( AT_MAGIC, "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
	victim->position = POS_SLEEPING;
    }
    if ( IS_NPC( victim ) )
      start_hating( victim, ch );

    return rNONE;
}



ch_ret spell_summon( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    SKILLTYPE *skill = get_skilltype(sn);

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   !victim->in_room
    ||   xIS_SET(ch->in_room->room_flags,     ROOM_NO_ASTRAL)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_NO_SUMMON)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 3
    ||   victim->fighting
    ||   victim->in_room == ch->in_room
    ||  (IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE))
    ||  (IS_NPC(victim) && xIS_SET(victim->act, ACT_AGGRESSIVE))
    ||  (IS_NPC(victim) && saves_spell_staff( level, victim )) 
    ||   !in_hard_range( victim, ch->in_room->area )
    ||  ( !IS_NPC( ch ) && !CAN_PKILL( ch ) && IS_PKILL( victim ) )
    ||  (IS_SET(ch->in_room->area->flags, AFLAG_NOPKILL) && IS_PKILL(victim))
    ||  ( !IS_NPC(ch) && !IS_NPC(victim) && IS_SET(victim->pcdata->flags, PCFLAG_NOSUMMON) ) )
    {
	failed_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    if ( ch->in_room->area != victim->in_room->area )
    {
	if ( ( (IS_NPC(ch) != IS_NPC(victim)) && chance(ch, 30) )
	||   ( (IS_NPC(ch) == IS_NPC(victim)) && chance(ch, 60) ) )
	{
	    failed_casting( skill, ch, victim, NULL );
	    set_char_color( AT_MAGIC, victim );
	    send_to_char( "You feel a strange pulling sensation...\n\r", victim );
	    return rSPELL_FAILED;
	}
    }

    if ( !IS_NPC( ch ) )
    {
	act( AT_MAGIC, "You feel a wave of nausea overcome you...", ch, NULL,
	     NULL, TO_CHAR );
	act( AT_MAGIC, "$n collapses, stunned!", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STUNNED;
    
	sprintf( buf, "%s summoned %s to room %d.", ch->name,
					     victim->name,
					     ch->in_room->vnum );
	log_string_plus( buf, LOG_NORMAL, ch->level );
	to_channel( buf, CHANNEL_MONITOR, "Monitor", UMAX( LEVEL_IMMORTAL, ch->level ) );
    }
 
    act( AT_MAGIC, "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    if ( sysdata.magichell && ch->level > 35 )
    {
        ROOM_INDEX_DATA *troom;

    for ( ;; )
    {
        troom = get_room_index( number_range( 1300, 100000 ) );
        if ( troom )
        if ( !troom->first_person
            && !xIS_SET(troom->room_flags, ROOM_DEATH ) )
        break;
    }

    if ( troom && can_astral_room(troom) )
    char_to_room( victim, troom );
    else
    char_to_room( victim, ch->in_room );
    }
    else
    char_to_room( victim, ch->in_room );
    act( AT_MAGIC, "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM );
    act( AT_MAGIC, "$N has summoned you!", victim, NULL, ch,   TO_CHAR );
    do_look( victim, "auto" );
    return rNONE;
}

/*
 * Travel via the astral plains to quickly travel to desired location
 *	-Thoric
 *
 * Uses SMAUG spell messages is available to allow use as a SMAUG spell
 */
ch_ret spell_astral_walk( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    struct skill_type *skill = get_skilltype(sn);

    if ( (victim = get_char_world(ch, target_name)) == NULL
    ||   !can_astral(ch, victim)
    ||   !in_hard_range(ch, victim->in_room->area) )
    {
	failed_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    if ( skill->hit_char && skill->hit_char[0] != '\0' )
	act( AT_COLORIZE, skill->hit_char, ch, NULL, victim, TO_CHAR );
    if ( skill->hit_vict && skill->hit_vict[0] != '\0' )
	act( AT_COLORIZE, skill->hit_vict, ch, NULL, victim, TO_VICT );

    if ( skill->hit_room && skill->hit_room[0] != '\0' )
	act( AT_COLORIZE, skill->hit_room, ch, NULL, victim, TO_NOTVICT );
    else
	act( AT_MAGIC, "$n disappears in a flash of light!", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    if ( sysdata.magichell && ch->level > 35 )
    {
        ROOM_INDEX_DATA *troom;

    for ( ;; )
    {
        troom = get_room_index( number_range( 1300, 100000 ) );
        if ( troom )
        if ( !troom->first_person
            && !xIS_SET(troom->room_flags, ROOM_DEATH ) )
        break;
    }

    if ( troom && can_astral_room(troom) )
    char_to_room( ch, troom );
    else
    char_to_room( ch, victim->in_room );
    }
    else
    char_to_room( ch, victim->in_room );
    if ( skill->hit_dest && skill->hit_dest[0] != '\0' )
	act( AT_COLORIZE, skill->hit_dest, ch, NULL, victim, TO_NOTVICT );
    else
	act( AT_MAGIC, "$n appears in a flash of light!", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    return rNONE;
}



ch_ret spell_teleport( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *pRoomIndex;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( !victim->in_room
    ||   xIS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    || ( !IS_NPC(ch) && victim->fighting )
    || ( victim != ch
    && ( saves_spell_staff( level, victim ) || saves_wands( level, victim ) ) ) )
    {
	failed_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    for ( ;; )
    {
	pRoomIndex = get_room_index( number_range( 0, MAX_VNUM ) );
	if ( pRoomIndex )
	if ( !xIS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)
	&&   !xIS_SET(pRoomIndex->room_flags, ROOM_SOLITARY)
	&&   !xIS_SET(pRoomIndex->room_flags, ROOM_NO_ASTRAL)
	&&   !IS_SET(pRoomIndex->area->flags, AFLAG_NOTELEPORT)
	&&   !xIS_SET(pRoomIndex->room_flags, ROOM_PROTOTYPE)
	&&   !xIS_SET(pRoomIndex->room_flags, ROOM_NO_RECALL) 
        &&   in_hard_range( ch, pRoomIndex->area ) )
	    break;
    }

    act( AT_MAGIC, "$n slowly fades out of view.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    if ( sysdata.magichell && ch->level > 35 )
    {
        ROOM_INDEX_DATA *troom;

    for ( ;; )
    {
        troom = get_room_index( number_range( 1300, 100000 ) );
        if ( troom )
        if ( !troom->first_person
            && !xIS_SET(troom->room_flags, ROOM_DEATH ) )
        break;
    }

    if ( troom && can_astral_room(troom) )
    char_to_room( victim, troom );
    else
    char_to_room( victim, pRoomIndex );
    }
    else
    char_to_room( victim, pRoomIndex );
    if ( !IS_NPC( victim ) )
	act( AT_MAGIC, "$n slowly fades into view.", victim, NULL, NULL, TO_ROOM );
    do_look( victim, "auto" );
    return rNONE;
}

#ifdef MYSTARIC
/*
 * Group Teleport Spell
 * Mystaric Sept. 15, 99
 *
 */

/*  Returns the number ot people within a grouped party */
int number_in_group( CHAR_DATA *ch )
{
  CHAR_DATA *nch, *nch_next;
  ROOM_INDEX_DATA *start = ch->in_room;
  int number = 0;

  for ( nch = start->first_person; nch; nch = nch_next )
  {
    nch_next = nch->next_in_room;
    if(is_same_group(nch, ch))
    number += 1;
  }
  return number;
}                  

ch_ret spell_group_teleport( int sn, int level, CHAR_DATA *ch, void *vo )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *nch, *nch_next;
  ROOM_INDEX_DATA *start = ch->in_room, *pRoomIndex;
  SKILLTYPE *skill = get_skilltype(sn);
  int mana;

  if ( !victim )
  	victim = ch;

  if ( !victim->in_room
    ||   xIS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    || ( !IS_NPC(ch) && victim->fighting )
    || ( victim != ch
    && ( saves_spell_staff( level, victim ) || saves_wands( level, victim ) ) ))
    {
      failed_casting( skill, ch, victim, NULL );
      return rSPELL_FAILED;
    }

    for ( ;; )
    {
      pRoomIndex = get_room_index( number_range( 0, MAX_VNUM ) );
      if ( pRoomIndex )
        if ( pRoomIndex != start )
          if ( !xIS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)
            &&   !xIS_SET(pRoomIndex->room_flags, ROOM_SOLITARY)
            &&   !xIS_SET(pRoomIndex->room_flags, ROOM_NO_ASTRAL)
            &&   !IS_SET(pRoomIndex->area->flags, AFLAG_NOTELEPORT)
            &&   !xIS_SET(pRoomIndex->room_flags, ROOM_PROTOTYPE)
            &&   !xIS_SET(pRoomIndex->room_flags, ROOM_NO_RECALL)
            &&   in_hard_range( ch, pRoomIndex->area ) )
      break;
    }

  mana = number_in_group( victim ) * 100;
  if ( !IS_NPC(ch) && ch->mana < mana )
  {
    send_to_char( "You don't have enough mana.\n\r", ch );
    return rSPELL_FAILED;
  }
  
  act( AT_MAGIC, "$n's arms rise towards the sky, a tornado engulfs the area.", 
                  victim, NULL, NULL, TO_ROOM );
  act( AT_MAGIC, "$n's group is sucked into the raging tornadoes.", 
                  victim, NULL, NULL, TO_ROOM );

  char_from_room( victim );
    if ( sysdata.magichell && ch->level > 35 )
    {
        ROOM_INDEX_DATA *troom;

    for ( ;; )
    {
        troom = get_room_index( number_range( 1300, 100000 ) );
        if ( troom )
        if ( !troom->first_person
            && !xIS_SET(troom->room_flags, ROOM_DEATH ) )
        break;
    }

    if ( troom && can_astral_room(troom) )
    char_to_room( victim, troom );
    else
    char_to_room( victim, pRoomIndex );
    }
    else
  char_to_room( victim, pRoomIndex );
  do_look( victim, "auto" );

  for ( nch = start->first_person; nch; nch = nch_next )
  {
    nch_next = nch->next_in_room;
    if(is_same_group(nch, victim) && (in_hard_range( nch, pRoomIndex->area ) ) )
    {
      char_from_room( nch );
      char_to_room( nch, pRoomIndex );
      if ( !IS_NPC( nch ) )
      do_look( nch, "auto" );
    }
  }                                                 
  victim->mana -= mana;
  return rNONE;
}    
#endif

/*
ch_ret spell_ventriloquate( int sn, int level, CHAR_DATA *ch, void *vo )
{
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char speaker[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;

    target_name = one_argument( target_name, speaker );

    sprintf( buf1, "%s says '%s'.\n\r",              speaker, target_name );
    sprintf( buf2, "Someone makes %s say '%s'.\n\r", speaker, target_name );
    buf1[0] = UPPER(buf1[0]);

    for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
    {
	if ( !is_name( speaker, vch->name ) ) {
	    set_char_color( AT_SAY, vch );
	    send_to_char( saves_spell_staff( level, vch ) ? buf2 : buf1, vch );
	}
    }

    return rNONE;
}
*/

ch_ret spell_ventriloquate( int sn, int level, CHAR_DATA *ch, void *vo )
{
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char speaker[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;

    target_name = one_argument( target_name, speaker );
    if ( speaker[0] == '\0' )
    {
	send_to_char( "Your voice sputters as you try to cast it in random directions.\n\r", ch );
    	return rSPELL_FAILED;
    }

    sprintf( buf1, "%s says '%s'.\n\r",              speaker, target_name );
    sprintf( buf2, "Someone makes %s say '%s'.\n\r", speaker, target_name );
    sprintf( buf3, "%s makes %s say '%s'.\n\r", capitalize( ch->name ), speaker, target_name);
    buf1[0] = UPPER(buf1[0]);

    for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
    {
	if ( !is_name( speaker, vch->name ) ) {
	    set_char_color( AT_SAY, vch );
	    if (get_trust(vch) >= LEVEL_IMMORTAL)
		send_to_char(buf3, vch);
            else
	    	send_to_char( saves_spell_staff( level, vch ) ? buf2 : buf1, vch );
	}
    }

    return rNONE;
}


ch_ret spell_weaken( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    SKILLTYPE *skill = get_skilltype(sn);

    set_char_color( AT_MAGIC, ch );
    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
	immune_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }
    if ( is_affected( victim, sn ) || saves_wands( level, victim ) )
    {
	send_to_char( "Your magic fails to take hold.\n\r", ch );
	return rSPELL_FAILED;
    }
    af.type      = sn;
    af.duration  = level / 2 * DUR_CONV;
    af.location  = APPLY_STR;
    af.modifier  = -2;
    xCLEAR_BITS(af.bitvector);
    affect_to_char( victim, &af );
    set_char_color( AT_MAGIC, victim );
    send_to_char( "Your muscles seem to atrophy!\n\r", victim );
    if ( ch != victim )
    {
      if ( ( ( ( !IS_NPC(victim) && class_table[victim->class]->attr_prime == APPLY_STR )
      || IS_NPC(victim) )
      && get_curr_str(victim) < 25 )
      || get_curr_str(victim) < 20 )
      {
        act( AT_MAGIC, "$N labors weakly as your spell atrophies $S muscles.", ch, NULL, victim, TO_CHAR );
        act( AT_MAGIC, "$N labors weakly as $n's spell atrophies $S muscles.", ch, NULL, victim, TO_NOTVICT );
      }
      else
      {
        act( AT_MAGIC, "You induce a mild atrophy in $N's muscles.", ch, NULL, victim, TO_CHAR );
        act( AT_MAGIC, "$n induces a mild atrophy in $N's muscles.", ch, NULL, victim, TO_NOTVICT );
      }
    }
    return rNONE;
}



/*
 * A spell as it should be				-Thoric
 */
ch_ret spell_word_of_recall( int sn, int level, CHAR_DATA *ch, void *vo )
{
    do_recall( (CHAR_DATA *) vo, "" );
    return rNONE;
}


/*
 * NPC spells.
 */
ch_ret spell_acid_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_lose;
    OBJ_DATA *obj_next;
    int dam;
    int hpch;

    if ( chance(ch, 2 * level) && !saves_breath( level, victim ) )
    {
	for ( obj_lose = victim->first_carrying; obj_lose; obj_lose = obj_next )
	{
	    int iWear;

	    obj_next = obj_lose->next_content;

	    if ( number_bits( 2 ) != 0 )
		continue;

	    switch ( obj_lose->item_type )
	    {
	    case ITEM_ARMOR:
		if ( obj_lose->value[0] > 0 )
		{
		    separate_obj(obj_lose);
		    act( AT_DAMAGE, "$p is pitted and etched!",
			victim, obj_lose, NULL, TO_CHAR );
		    if ( ( iWear = obj_lose->wear_loc ) != WEAR_NONE )
			victim->armor -= apply_ac( obj_lose, iWear );
		    obj_lose->value[0] -= 1;
		    obj_lose->cost      = 0;
		    if ( iWear != WEAR_NONE )
			victim->armor += apply_ac( obj_lose, iWear );
		}
		break;

	    case ITEM_CONTAINER:
		separate_obj( obj_lose );
		act( AT_DAMAGE, "$p fumes and dissolves!",
		    victim, obj_lose, NULL, TO_CHAR );
		act( AT_OBJECT, "The contents of $p held by $N spill onto the ground.",
		   victim, obj_lose, victim, TO_ROOM );
		act( AT_OBJECT, "The contents of $p spill out onto the ground!",
		   victim, obj_lose, NULL, TO_CHAR );
		empty_obj( obj_lose, NULL, victim->in_room );
		extract_obj( obj_lose );
		break;
	    }
	}
    }

    hpch = UMAX( 10, ch->hit );
    dam  = number_range( hpch/16+1, hpch/8 );
    if ( saves_breath( level, victim ) )
	dam /= 2;
    return damage( ch, victim, dam, sn );
}



ch_ret spell_fire_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_lose;
    OBJ_DATA *obj_next;
    int dam;
    int hpch;

    if ( chance(ch, 2 * level) && !saves_breath( level, victim ) )
    {
	for ( obj_lose = victim->first_carrying; obj_lose;
	      obj_lose = obj_next )
	{
	    char *msg;

	    obj_next = obj_lose->next_content;
	    if ( number_bits( 2 ) != 0 )
		continue;

	    switch ( obj_lose->item_type )
	    {
	    default:             continue;
	    case ITEM_CONTAINER: msg = "$p ignites and burns!";   break;
	    case ITEM_POTION:    msg = "$p bubbles and boils!";   break;
	    case ITEM_SCROLL:    msg = "$p crackles and burns!";  break;
	    case ITEM_STAFF:     msg = "$p smokes and chars!";    break;
	    case ITEM_WAND:      msg = "$p sparks and sputters!"; break;
	    case ITEM_COOK:
	    case ITEM_FOOD:      msg = "$p blackens and crisps!"; break;
	    case ITEM_PILL:      msg = "$p melts and drips!";     break;
	    }

	    separate_obj( obj_lose );
	    act( AT_DAMAGE, msg, victim, obj_lose, NULL, TO_CHAR );
	    if ( obj_lose->item_type == ITEM_CONTAINER )
	    {
		act( AT_OBJECT, "The contents of $p held by $N spill onto the ground.",
		   victim, obj_lose, victim, TO_ROOM );
		act( AT_OBJECT, "The contents of $p spill out onto the ground!",
		   victim, obj_lose, NULL, TO_CHAR );
		empty_obj( obj_lose, NULL, victim->in_room );
	    }
	    extract_obj( obj_lose );
	}
    }

    hpch = UMAX( 10, ch->hit );
    dam  = number_range( hpch/16+1, hpch/8 );
    if ( saves_breath( level, victim ) )
	dam /= 2;
    return damage( ch, victim, dam, sn );
}



ch_ret spell_frost_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_lose;
    OBJ_DATA *obj_next;
    int dam;
    int hpch;

    if ( chance( ch, 2 * level ) && !saves_breath( level, victim ) )
    {
	for ( obj_lose = victim->first_carrying; obj_lose;
	obj_lose = obj_next )
	{
	    char *msg;

	    obj_next = obj_lose->next_content;
	    if ( number_bits( 2 ) != 0 )
		continue;

	    switch ( obj_lose->item_type )
	    {
	    default:            continue;
	    case ITEM_CONTAINER:
	    case ITEM_DRINK_CON:
	    case ITEM_POTION:   msg = "$p freezes and shatters!"; break;
	    }

	    separate_obj( obj_lose );
	    act( AT_DAMAGE, msg, victim, obj_lose, NULL, TO_CHAR );
	    if ( obj_lose->item_type == ITEM_CONTAINER )
	    {
		act( AT_OBJECT, "The contents of $p held by $N spill onto the ground.",
		   victim, obj_lose, victim, TO_ROOM );
		act( AT_OBJECT, "The contents of $p spill out onto the ground!",
		   victim, obj_lose, NULL, TO_CHAR );
		empty_obj( obj_lose, NULL, victim->in_room );
	    }
	    extract_obj( obj_lose );
	}
    }

    hpch = UMAX( 10, ch->hit );
    dam  = number_range( hpch/16+1, hpch/8 );
    if ( saves_breath( level, victim ) )
	dam /= 2;
    return damage( ch, victim, dam, sn );
}



ch_ret spell_gas_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    int hpch;
    bool ch_died;

    ch_died = FALSE;

    if ( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "You fail to breathe.\n\r", ch );
	return rNONE;
    }

    for ( vch = ch->in_room->first_person; vch; vch = vch_next )
    {
	vch_next = vch->next_in_room;
        if ( IS_NPC( vch ) && xIS_SET( vch->act, ACT_MOBINVIS ) )
          continue;

        if ( !IS_NPC( vch ) && xIS_SET( vch->act, PLR_WIZINVIS ) 
             && vch->pcdata->wizinvis >= LEVEL_IMMORTAL )
          continue;

	if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
	{
	    hpch = UMAX( 10, ch->hit );
	    dam  = number_range( hpch/16+1, hpch/8 );
	    if ( saves_breath( level, vch ) )
		dam /= 2;
	    if ( damage( ch, vch, dam, sn ) == rCHAR_DIED || char_died(ch) )
	      ch_died = TRUE;
	}
    }
    if ( ch_died )
      return rCHAR_DIED;
    else
      return rNONE;
}



ch_ret spell_lightning_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    int hpch;

    hpch = UMAX( 10, ch->hit );
    dam = number_range( hpch/16+1, hpch/8 );
    if ( saves_breath( level, victim ) )
	dam /= 2;
    return damage( ch, victim, dam, sn );
}

ch_ret spell_null( int sn, int level, CHAR_DATA *ch, void *vo )
{
    send_to_char( "That's not a spell!\n\r", ch );
    return rNONE;
}

/* don't remove, may look redundant, but is important */
ch_ret spell_notfound( int sn, int level, CHAR_DATA *ch, void *vo )
{
    send_to_char( "That's not a spell!\n\r", ch );
    return rNONE;
}


/*
 *   Haus' Spell Additions
 *
 */

/* to do: portal           (like mpcreatepassage)
 *        enchant armour?  (say -1/-2/-3 ac )
 *        sharpness        (makes weapon of caster's level)
 *        repair           (repairs armor)
 *        blood burn       (offensive)  * name: net book of spells *
 *        spirit scream    (offensive)  * name: net book of spells *
 *        something about saltpeter or brimstone
 */

/* Working on DM's transport eq suggestion - Scryn 8/13 */
ch_ret spell_transport( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char arg3[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    SKILLTYPE *skill = get_skilltype(sn);

    target_name = one_argument(target_name, arg3 );

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   xIS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_NO_ASTRAL)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_DEATH)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_PROTOTYPE)
    ||   xIS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 15
    ||	(IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE))
    ||  (IS_NPC(victim) && saves_spell_staff( level, victim )) )
    {
	failed_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }


    if (victim->in_room == ch->in_room)
    {
	send_to_char("They are right beside you!", ch);
	return rSPELL_FAILED;
    }

    if ( (obj = get_obj_carry( ch, arg3 ) ) == NULL 
    || ( victim->carry_weight + get_obj_weight ( obj ) ) > can_carry_w(victim) 
    ||	(IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE)))
    {
	failed_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    separate_obj(obj);  /* altrag shoots, haus alley-oops! */

    if ( IS_OBJ_STAT(obj, ITEM_NODROP) || IS_OBJ_STAT(obj, ITEM_PERMANENT) )
    {
	send_to_char( "You can't seem to let go of it.\n\r", ch );
	return rSPELL_FAILED;   /* nice catch, caine */
    }

    if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE )
    &&   get_trust( victim ) < LEVEL_IMMORTAL )
    {
	send_to_char( "That item is not for mortal hands to touch!\n\r", ch );
	return rSPELL_FAILED;   /* Thoric */
    }

    act( AT_MAGIC, "$p slowly dematerializes...", ch, obj, NULL, TO_CHAR );
    act( AT_MAGIC, "$p slowly dematerializes from $n's hands..", ch, obj, NULL, TO_ROOM );
    obj_from_char( obj );
    obj_to_char( obj, victim );
    act( AT_MAGIC, "$p from $n appears in your hands!", ch, obj, victim, TO_VICT );
    act( AT_MAGIC, "$p appears in $n's hands!", victim, obj, NULL, TO_ROOM );
    save_char_obj(ch);
    save_char_obj(victim);
    return rNONE;
}

ch_ret spell_close_portal( int sn, int level, CHAR_DATA *ch, void *vo )
{

    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    for ( obj = ch->in_room->first_content; obj; obj = obj_next )
    {
        obj_next = obj->next_content;
	if ( obj->item_type == ITEM_PORTAL )
	{
	    remove_portal( obj );
	    obj->item_type = ITEM_TRASH;
	    extract_obj( obj );
	    act( AT_MAGIC, "As $n utters a lengthy incantation, the portal spins into nothingness.", ch, NULL, NULL, TO_ROOM );
	    act( AT_MAGIC, "As you utter a lengthy incantation, the portal spins into nothingness.", ch, NULL, NULL, TO_CHAR );
	}	
    }
    return rNONE;
}

ch_ret spell_portal( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *targetRoom, *fromRoom;
    int targetRoomVnum;
    OBJ_DATA *portalObj;
    EXIT_DATA *pexit;
    char buf[MAX_STRING_LENGTH];
    SKILLTYPE *skill = get_skilltype(sn);

    /* No go if all kinds of things aren't just right, including the caster
       and victim are not both pkill or both peaceful. -- Narn
    */
    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   !victim->in_room
    ||   xIS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_NO_ASTRAL)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_DEATH)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_PROTOTYPE)
    ||   xIS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   xIS_SET(ch->in_room->room_flags, ROOM_NO_ASTRAL)
    ||   IS_SET(ch->in_room->area->flags, AFLAG_NOPORTALOUT)
    ||   IS_SET(victim->in_room->area->flags, AFLAG_NOPORTALIN)
	||	 IS_SET( victim->in_room->area->flags, AFLAG_NOASTRAL )
	||	 IS_SET( ch->in_room->area->flags, AFLAG_NOASTRAL )
    ||   victim->level >= level + 15
    ||	(IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE))
    ||  (IS_NPC(victim) && saves_spell_staff( level, victim )) 
    ||  (!IS_NPC(victim) && CAN_PKILL(ch) != CAN_PKILL(victim)) )
    {
	failed_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    if (victim->in_room == ch->in_room)
    {
	send_to_char("They are right beside you!", ch);
	return rSPELL_FAILED;
    }

    
    targetRoomVnum = victim->in_room->vnum;
    fromRoom = ch->in_room;
    targetRoom = victim->in_room;

    /* Check if there already is a portal in either room. */
    for ( pexit = fromRoom->first_exit; pexit; pexit = pexit->next )
    {
	if ( IS_SET( pexit->exit_info, EX_PORTAL ) ) 
	{
	    send_to_char("There is already a portal in this room.\n\r",ch);
	    return rSPELL_FAILED;
	}
 
	if ( pexit->vdir == DIR_PORTAL )
	{
	    send_to_char("You may not create a portal in this room.\n\r",ch);
	    return rSPELL_FAILED;
	}
    }

    for ( pexit = targetRoom->first_exit; pexit; pexit = pexit->next )
	if ( pexit->vdir == DIR_PORTAL )
	{
	    failed_casting( skill, ch, victim, NULL );
            return rSPELL_FAILED;
	}

    pexit = make_exit( fromRoom, targetRoom, DIR_PORTAL ); 
    pexit->keyword 	= STRALLOC( "portal" );
    pexit->description	= STRALLOC( "You gaze into the shimmering portal...\n\r" );
    pexit->key     	= -1;
    pexit->exit_info	= EX_PORTAL | EX_xENTER | EX_HIDDEN | EX_xLOOK;
    pexit->vnum    	= targetRoomVnum;

    portalObj = create_object( get_obj_index( OBJ_VNUM_PORTAL ), 0 );
    portalObj->timer = 3;
    sprintf( buf, "a portal created by %s", ch->name );
    STRFREE( portalObj->short_descr );
    portalObj->short_descr = STRALLOC( buf ); 

    /* support for new casting messages */
    if ( !skill->hit_char || skill->hit_char[0] == '\0' )
    {
	set_char_color( AT_MAGIC, ch );
	send_to_char("You utter an incantation, and a portal forms in front of you!\n\r", ch);
    }
    else
	act( AT_COLORIZE, skill->hit_char, ch, NULL, victim, TO_CHAR );
    if ( !skill->hit_room || skill->hit_room[0] == '\0' )
	act( AT_COLORIZE, "$n utters an incantation, and a portal forms in front of you!", ch, NULL, NULL, TO_ROOM );
    else
	act( AT_COLORIZE, skill->hit_room, ch, NULL, victim, TO_ROOM );
    if ( !skill->hit_vict || skill->hit_vict[0] == '\0' )
	act( AT_MAGIC, "A shimmering portal forms in front of you!", victim, NULL, NULL, TO_ROOM );
    else
	act( AT_COLORIZE, skill->hit_vict, victim, NULL, victim, TO_ROOM );
    portalObj = obj_to_room( portalObj, ch->in_room );

    pexit = make_exit( targetRoom, fromRoom, DIR_PORTAL );
    pexit->keyword 	= STRALLOC( "portal" );
    pexit->description	= STRALLOC( "You gaze into the shimmering portal...\n\r" );
    pexit->key          = -1;
    pexit->exit_info    = EX_PORTAL | EX_xENTER | EX_HIDDEN;
    pexit->vnum         = targetRoomVnum;

    portalObj = create_object( get_obj_index( OBJ_VNUM_PORTAL ), 0 );
    portalObj->timer = 3;
    STRFREE( portalObj->short_descr );
    portalObj->short_descr = STRALLOC( buf ); 
    portalObj = obj_to_room( portalObj, targetRoom );
/*
    sprintf( buf, "%s has made a portal from room %d to room %d.", 
             ch->name, fromRoom->vnum, targetRoomVnum );
    log_string_plus( buf, LOG_NORMAL, ch->level );
    to_channel( buf, CHANNEL_MONITOR, "Monitor", UMAX( LEVEL_IMMORTAL, ch->level)  );
*/
    return rNONE;
}

ch_ret spell_farsight( int sn, int level, CHAR_DATA *ch, void *vo )
{
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA *victim;
    SKILLTYPE *skill = get_skilltype(sn);

    /* The spell fails if the victim isn't playing, the victim is the caster,
       the target room has private, solitary, noastral, death or proto flags,
       the caster's room is norecall, the victim is too high in level, the 
       victim is a proto mob, the victim makes the saving throw or the pkill 
       flag on the caster is not the same as on the victim.  Got it?
    */
    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   !victim->in_room
    ||   xIS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_NO_ASTRAL)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_DEATH)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_PROTOTYPE)
    ||   xIS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 15
    ||	(IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE))
    ||  (IS_NPC(victim) && saves_spell_staff( level, victim )) 
    ||  (!IS_NPC(victim) && CAN_PKILL(victim) && !CAN_PKILL(ch) ) )
    {
	failed_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    location = victim->in_room;
    if (!location)
    {
        failed_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }
    successful_casting( skill, ch, victim, NULL );
    original = ch->in_room;
    char_from_room( ch );
    if ( sysdata.magichell && ch->level > 35 )
    {
        ROOM_INDEX_DATA *troom;

    for ( ;; )
    {
        troom = get_room_index( number_range( 1300, 100000 ) );
        if ( troom )
        if ( !troom->first_person
            && !xIS_SET(troom->room_flags, ROOM_DEATH ) )
        break;
    }

    if ( troom && can_astral_room(troom) )
    char_to_room( ch, troom );
    else
    char_to_room( ch, location );
    }
    else
    char_to_room( ch, location );
    do_look( ch, "auto" );
    char_from_room( ch );
    char_to_room( ch, original );

    if ( chance_attrib( victim, 20, get_curr_wis( victim ) )
    &&  !IS_PKILL( ch ) )
        send_to_char( "You get an uneasy feeling that you are being watched.\n\r", victim );
    return rNONE;
}

ch_ret spell_recharge( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if ( obj->item_type == ITEM_STAFF 
    ||   obj->item_type == ITEM_WAND) 
    {
	separate_obj(obj);
	if ( obj->value[2] == obj->value[1]
	||   obj->value[1] > (obj->pIndexData->value[1] * 4) )
	{
	    act( AT_FIRE, "$p bursts into flames, injuring you!", ch, obj, NULL, TO_CHAR ); 
	    act( AT_FIRE, "$p bursts into flames, charring $n!", ch, obj, NULL, TO_ROOM);
	    extract_obj(obj);
	    if ( damage(ch, ch, obj->level * 2, TYPE_UNDEFINED) == rCHAR_DIED
	    ||   char_died(ch) )
		return rCHAR_DIED;
	    else
		return rSPELL_FAILED;
	}

	if ( chance(ch, 2) )
	{
	    act( AT_YELLOW, "$p glows with a blinding magical luminescence.", 
		ch, obj, NULL, TO_CHAR);
	    obj->value[1] *= 2;
	    obj->value[2] = obj->value[1];
	    return rNONE;
	}
	else
	if ( chance(ch, 5) )
	{
	    act( AT_YELLOW, "$p glows brightly for a few seconds...", 
		ch, obj, NULL, TO_CHAR);
	    obj->value[2] = obj->value[1];
	    return rNONE;
	}
	else
	if ( chance(ch, 10) )
	{
	    act( AT_WHITE, "$p disintegrates into a void.", ch, obj, NULL, TO_CHAR);
	    act( AT_WHITE, "$n's attempt at recharging fails, and $p disintegrates.", 
		ch, obj, NULL, TO_ROOM);
	    extract_obj(obj);
	    return rSPELL_FAILED;
	}
	else
	if ( chance(ch, 50 - (ch->level/2) ) )
	{
	    send_to_char("Nothing happens.\n\r", ch);
	    return rSPELL_FAILED;
	}
	else
	{
	    act( AT_MAGIC, "$p feels warm to the touch.", ch, obj, NULL, TO_CHAR);
	    --obj->value[1];
	    obj->value[2] = obj->value[1];
	    return rNONE;
	}
    }
    else
    {
	send_to_char( "You can't recharge that!\n\r", ch);
	return rSPELL_FAILED;
    }    
}

/*
 * Idea from AD&D 2nd edition player's handbook (c)1989 TSR Hobbies Inc.
 * -Thoric
 */
ch_ret spell_plant_pass( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   !victim->in_room
    ||   xIS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_NO_ASTRAL)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_DEATH)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_PROTOTYPE)
	||	 IS_SET( victim->in_room->area->flags, AFLAG_NOASTRAL )
	||	 IS_SET( ch->in_room->area->flags, AFLAG_NOASTRAL )
    ||  (victim->in_room->sector_type != SECT_FOREST
    &&   victim->in_room->sector_type != SECT_FIELD)
    ||  (ch->in_room->sector_type     != SECT_FOREST
    &&   ch->in_room->sector_type     != SECT_FIELD)
    ||   xIS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 15
    ||  (CAN_PKILL(victim) && !IS_NPC(ch) && !IS_PKILL(ch))
    ||	(IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE))
    ||  (IS_NPC(victim) && saves_spell_staff( level, victim )) 
    ||  !in_hard_range( ch, victim->in_room->area )
    ||  (IS_SET(victim->in_room->area->flags, AFLAG_NOPKILL) && IS_PKILL(ch)))
    {
	failed_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    if ( ch->in_room->sector_type == SECT_FOREST )
	act( AT_MAGIC, "$n melds into a nearby tree!", ch, NULL, NULL, TO_ROOM );
    else
	act( AT_MAGIC, "$n melds into the grass!", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    if ( sysdata.magichell && ch->level > 35 )
    {
        ROOM_INDEX_DATA *troom;

    for ( ;; )
    {
        troom = get_room_index( number_range( 1300, 100000 ) );
        if ( troom )
        if ( !troom->first_person
            && !xIS_SET(troom->room_flags, ROOM_DEATH ) )
        break;
    }

    if ( troom && can_astral_room(troom) )
    char_to_room( ch, troom );
    else
    char_to_room( ch, victim->in_room );
    }
    else
    char_to_room( ch, victim->in_room );
    if ( ch->in_room->sector_type == SECT_FOREST )
	act( AT_MAGIC, "$n appears from behind a nearby tree!", ch, NULL, NULL, TO_ROOM );
    else
	act( AT_MAGIC, "$n grows up from the grass!", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    return rNONE;
}

/*
 * Vampire version of astral_walk				-Thoric
 */
ch_ret spell_mist_walk( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    bool allowday;
    SKILLTYPE *skill = get_skilltype(sn);

    set_char_color( AT_DGREEN, ch );

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch )
    {
	failed_casting( skill, ch, victim, NULL );
	send_to_char( "You cannot sense your victim...", ch );
	return rSPELL_FAILED;
    }

    if ( IS_PKILL(ch)
    &&   ch->pcdata->condition[COND_BLOODTHIRST] > 22 )
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
	||	 IS_SET( victim->in_room->area->flags, AFLAG_NOASTRAL )
	||	 IS_SET( ch->in_room->area->flags, AFLAG_NOASTRAL )
    ||   xIS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= level + 15
    ||  (CAN_PKILL(victim) && !IS_NPC(ch) && !IS_PKILL(ch))
    ||	(IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE))
    ||  (IS_NPC(victim) && saves_spell_staff( level, victim )) 
    ||  !in_hard_range( ch, victim->in_room->area )
    ||  (IS_SET(victim->in_room->area->flags, AFLAG_NOPKILL) && IS_PKILL(ch)))
    {
	failed_casting( skill, ch, victim, NULL );
	send_to_char( "You cannot sense your victim...", ch);
	return rSPELL_FAILED;
    }
    
    /* Subtract 22 extra bp for mist walk from 0500 to 2100 SB */
    if  ( time_info.hour < 21 && time_info.hour > 5 && !IS_NPC(ch) )
	gain_condition( ch, COND_BLOODTHIRST, - 22 );

    act( AT_DGREEN, "$n dissolves into a cloud of glowing mist, then vanishes!", 
	ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    if ( sysdata.magichell && ch->level > 35 )
    {
        ROOM_INDEX_DATA *troom;

    for ( ;; )
    {
        troom = get_room_index( number_range( 1300, 100000 ) );
        if ( troom )
        if ( !troom->first_person
            && !xIS_SET(troom->room_flags, ROOM_DEATH ) )
        break;
    }

    if ( troom && can_astral_room(troom) )
    char_to_room( ch, troom );
    else
    char_to_room( ch, victim->in_room );
    }
    else
    char_to_room( ch, victim->in_room );
    act( AT_DGREEN, "A cloud of glowing mist engulfs you, then withdraws to unveil $n!", 
	ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    return rNONE;
}

/*
 * Cleric version of astral_walk				-Thoric
 */
ch_ret spell_solar_flight( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    SKILLTYPE *skill = get_skilltype(sn);
    WEATHER_DATA *weath = ch->in_room->area->weather;

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||  (time_info.hour > 18 || time_info.hour < 8)
    ||   !victim->in_room
    ||  !IS_OUTSIDE(ch)
    ||  !IS_OUTSIDE(victim)
    ||   weath->precip >= 0
    ||   xIS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_NO_ASTRAL)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_DEATH)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_PROTOTYPE)
    ||   xIS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
	||	 IS_SET( victim->in_room->area->flags, AFLAG_NOASTRAL )
	||	 IS_SET( ch->in_room->area->flags, AFLAG_NOASTRAL )
    ||   victim->level >= level + 15
    ||  (CAN_PKILL(victim) && !IS_NPC(ch) && !IS_PKILL(ch))
    ||	(IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE))
    ||  (IS_NPC(victim) && saves_spell_staff( level, victim )) 
    ||  !in_hard_range( ch, victim->in_room->area )
    ||  (IS_SET(victim->in_room->area->flags, AFLAG_NOPKILL) && IS_PKILL(ch)))
    {
	failed_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    act( AT_MAGIC, "$n disappears in a blinding flash of light!",
	ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    if ( sysdata.magichell && ch->level > 35 )
    {
        ROOM_INDEX_DATA *troom;

    for ( ;; )
    {
        troom = get_room_index( number_range( 1300, 100000 ) );
        if ( troom )
        if ( !troom->first_person
            && !xIS_SET(troom->room_flags, ROOM_DEATH ) )
        break;
    }

    if ( troom && can_astral_room(troom) )
    char_to_room( ch, troom );
    else
    char_to_room( ch, victim->in_room );
    }
    else
    char_to_room( ch, victim->in_room );
    act( AT_COLORIZE, "$n appears in a blinding flash of light!", 
	ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    return rNONE;
}

/* Scryn 2/2/96 */
ch_ret spell_remove_invis( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( target_name[0] == '\0' )
    {
	send_to_char( "What should the spell be cast upon?\n\r", ch );
	return rSPELL_FAILED;
    }

    obj = get_obj_carry( ch, target_name );


    if ( obj )
    {
	separate_obj( obj );

	if ( !IS_OBJ_STAT(obj, ITEM_INVIS) )
	{
	    send_to_char( "Its not invisible!\n\r", ch );
	    return rSPELL_FAILED;
	}

	xREMOVE_BIT(obj->extra_flags, ITEM_INVIS);
	act( AT_MAGIC, "$p becomes visible again.", ch, obj, NULL, TO_CHAR );
    
	send_to_char( "Ok.\n\r", ch );
	return rNONE;
    }
    else
    {
	CHAR_DATA *victim;

	victim = get_char_room(ch, target_name);

	if (victim)
	{
	    if(!can_see(ch, victim))
	    {
		ch_printf(ch, "You don't see %s!\n\r", target_name);
		return rSPELL_FAILED;
	    }
	
	    if( !IS_AFFECTED(victim, AFF_INVISIBLE) )
	    {
		send_to_char("They are not invisible!\n\r", ch);
		return rSPELL_FAILED;
	    }

	    if ( is_safe(ch, victim, TRUE) )
	    {
		failed_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
	    }

	    if ( IS_SET( victim->immune, RIS_MAGIC ) )
	    {
		immune_casting( skill, ch, victim, NULL );
		return rSPELL_FAILED;
 	    }
	    if ( !IS_NPC(victim) )
	    {
		if ( chance(ch, 50) && ch->level + 10 < victim->level )
		{
		    failed_casting( skill, ch, victim, NULL );
		    return rSPELL_FAILED;
		}
		else
		    check_illegal_pk(ch, victim);
	    }
	    else
	    {
		if ( chance(ch, 50) && ch->level + 15 < victim->level )
		{
		    failed_casting( skill, ch, victim, NULL );
		    return rSPELL_FAILED;
		}
	    }

	    affect_strip ( victim, gsn_invis                        );
	    affect_strip ( victim, gsn_mass_invis                   );
	    xREMOVE_BIT  ( victim->affected_by, AFF_INVISIBLE       );
/*	    send_to_char( "Ok.\n\r", ch );*/
	    successful_casting( skill, ch, victim, NULL );
	    return rNONE;
	}

	ch_printf(ch, "You can't find %s!\n\r", target_name);
	return rSPELL_FAILED;
    }	   
}	

/*
 * Animate Dead: Scryn 3/2/96
 * Modifications by Altrag 16/2/96
 */
ch_ret spell_animate_dead( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *mob;
    OBJ_DATA  *corpse;
    OBJ_DATA  *corpse_next;
    OBJ_DATA  *obj;
    OBJ_DATA  *obj_next;
    bool      found;
    MOB_INDEX_DATA *pMobIndex;
    AFFECT_DATA af;
    char       buf[MAX_STRING_LENGTH];
    SKILLTYPE *skill = get_skilltype(sn);

    found = FALSE;

    for (corpse = ch->in_room->first_content; corpse; corpse = corpse_next)
    {
	corpse_next = corpse->next_content;

	if (corpse->item_type == ITEM_CORPSE_NPC && corpse->cost != -5)
	{
	   found = TRUE;
	   break;
	}
    }
  
    if ( !found )
    {
	send_to_char("You cannot find a suitable corpse here.\n\r", ch);
	return rSPELL_FAILED;
    }

    if ( get_mob_index(MOB_VNUM_ANIMATED_CORPSE) == NULL )
    {
	bug("Vnum 5 not found for spell_animate_dead!", 0);
	return rNONE;
    }
 

    if ( (pMobIndex = get_mob_index((int) abs(corpse->cost) )) == NULL )
    {
	bug("Can not find mob for cost of corpse, spell_animate_dead", 0);
	return rSPELL_FAILED;
    }

    if ( pMobIndex == get_mob_index( MOB_VNUM_DEITY ) )
    {
    	send_to_char("You can't animate the corpse of a Deity's Avatar.\n\r", ch );
	return rSPELL_FAILED;
    }
    
    if ( !IS_NPC(ch) )
    {
      if ( IS_VAMPIRE(ch) )
      {
        if ( !IS_IMMORTAL(ch) && ch->pcdata->condition[COND_BLOODTHIRST] -
            (pMobIndex->level/3) < 0 )
        {
          send_to_char("You do not have enough blood power to reanimate this"
                      " corpse.\n\r", ch );
          return rSPELL_FAILED;
        }
        gain_condition(ch, COND_BLOODTHIRST, pMobIndex->level/3);
      }
      else if ( ch->mana - (pMobIndex->level*4) < 0 )
      {
  	send_to_char("You do not have enough mana to reanimate this "
  	            "corpse.\n\r", ch);
	return rSPELL_FAILED;
      }
      else
        ch->mana -= (pMobIndex->level*4);
    }



    if ( IS_IMMORTAL(ch) || ( chance(ch, 75) && pMobIndex->level - ch->level < 10 ) )
    { 
	mob = create_mobile( get_mob_index(MOB_VNUM_ANIMATED_CORPSE) );
	char_to_room( mob, ch->in_room );
/* this used to base off level of mob, but plevelers were having a hay day */
/* If someone comes up with a better way than making level 20, 200 hp mobs */
/* feel free to change the level formula back */
	mob->level 	 = number_fuzzy( 5 );
	mob->race  	 = pMobIndex->race;	/* should be undead */

        /* Fix so mobs wont have 0 hps and crash mud - Scryn 2/20/96 */
  	if (!pMobIndex->hitnodice)
	  mob->max_hit      = pMobIndex->level * 8 + number_range(
                              pMobIndex->level * pMobIndex->level / 4,
                              pMobIndex->level * pMobIndex->level );        
	else
	mob->max_hit     = dice(pMobIndex->hitnodice, pMobIndex->hitsizedice)
	                 + pMobIndex->hitplus;
	mob->max_hit	 = UMAX( URANGE( mob->max_hit / 4,
	                          (mob->max_hit * corpse->value[3]) / 100,
				   ch->level * dice(20,10)), 1 );

       
	mob->hit       = mob->max_hit;
	mob->damroll   = ch->level / 8;
	mob->hitroll   = ch->level / 6;
	mob->alignment = ch->alignment;

	act(AT_MAGIC, "$n makes $T rise from the grave!", ch, NULL, pMobIndex->short_descr, TO_ROOM);
	act(AT_MAGIC, "You make $T rise from the grave!", ch, NULL, pMobIndex->short_descr, TO_CHAR);

	sprintf(buf, "animated corpse %s", pMobIndex->player_name);
	STRFREE(mob->name);
	mob->name = STRALLOC(buf);

	sprintf(buf, "The animated corpse of %s", pMobIndex->short_descr);
	STRFREE(mob->short_descr);
	mob->short_descr = STRALLOC(buf);

	sprintf(buf, "An animated corpse of %s struggles with the horror of its undeath.\n\r", pMobIndex->short_descr);
	STRFREE(mob->long_descr);
	mob->long_descr = STRALLOC(buf);
	add_follower( mob, ch );
	af.type      = sn;
	af.duration  = (number_fuzzy( (level + 1) / 4 ) + 1) * DUR_CONV;
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = meb(AFF_CHARM);
	affect_to_char( mob, &af );

	if (corpse->first_content)
	    for( obj = corpse->first_content; obj; obj = obj_next)
	    {
		obj_next = obj->next_content;
		obj_from_obj(obj);
		obj_to_room(obj, corpse->in_room);
	    }

	separate_obj(corpse);
	extract_obj(corpse);
	return rNONE;
    }
    else
    {
	failed_casting( skill, ch, NULL, NULL );
	return rSPELL_FAILED;
    }
}

/* Works now.. -- Altrag */
ch_ret spell_possess( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA af;
    SKILLTYPE *skill = get_skilltype(sn);
    int try;

    if (!ch->desc || ch->desc->original)
    {
	send_to_char("You are not in your original state.\n\r", ch);
	return rSPELL_FAILED;
    }

    if ( (victim = get_char_room( ch, target_name ) ) == NULL)
    {
	send_to_char("They aren't here!\n\r", ch);
	return rSPELL_FAILED;
    }

    if (victim == ch)
    {
	send_to_char("You can't possess yourself!\n\r", ch);
	return rSPELL_FAILED;
    }

    if (!IS_NPC(victim))
    {
	send_to_char("You can't possess another player!\n\r", ch);
	return rSPELL_FAILED;
    }

    if (victim->desc)
    {
	ch_printf(ch, "%s is already possessed.\n\r", victim->short_descr);
	return rSPELL_FAILED;
    }

    if ( IS_PACIFIST( victim ) )
    {
	immune_casting( skill, ch, victim, NULL );
	return rVICT_IMMUNE;
    }

    if ( IS_SET( victim->immune, RIS_MAGIC )
    ||   IS_SET( victim->immune, RIS_CHARM ) )
    {
	immune_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    try = ris_save( victim, level, RIS_CHARM );

    if ( IS_AFFECTED(victim, AFF_POSSESS)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||   level < victim->level 
    ||  victim->desc
    ||   saves_spell_staff( try, victim ) 
    ||  !chance(ch, 25) )
    {
	failed_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    af.type      = sn;
    af.duration  = 20 + (ch->level - victim->level) / 2;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = meb(AFF_POSSESS);
    affect_to_char( victim, &af );

    sprintf(buf, "You have possessed %s!\n\r", victim->short_descr);

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    ch->switched        = victim;
    send_to_char( buf, victim );

    return rNONE;
}

/* Ignores pickproofs, but can't unlock containers. -- Altrag 17/2/96 */
ch_ret spell_knock( int sn, int level, CHAR_DATA *ch, void *vo )
{
  EXIT_DATA *pexit;
    SKILLTYPE *skill = get_skilltype(sn);

  set_char_color(AT_MAGIC, ch);
  /*
   * shouldn't know why it didn't work, and shouldn't work on pickproof
   * exits.  -Thoric
   */
  if ( !(pexit=find_door(ch, target_name, FALSE))
  ||   !IS_SET(pexit->exit_info, EX_CLOSED)
  ||   !IS_SET(pexit->exit_info, EX_LOCKED)
  ||    IS_SET(pexit->exit_info, EX_PICKPROOF) )
  {
	failed_casting( skill, ch, NULL, NULL );
	return rSPELL_FAILED;
  }
  REMOVE_BIT(pexit->exit_info, EX_LOCKED);
  send_to_char( "*Click*\n\r", ch );
  if ( pexit->rexit && pexit->rexit->to_room == ch->in_room )
    REMOVE_BIT( pexit->rexit->exit_info, EX_LOCKED );
  check_room_for_traps( ch, TRAP_UNLOCK | trap_door[pexit->vdir] );
  return rNONE;
}

/* Tells to sleepers in area. -- Altrag 17/2/96 */
ch_ret spell_dream( int sn, int level, CHAR_DATA *ch, void *vo )
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  
  target_name = one_argument(target_name, arg);
  set_char_color(AT_MAGIC, ch);
  if ( !(victim = get_char_world(ch, arg)) ||
      victim->in_room->area != ch->in_room->area )
  {
    send_to_char("They aren't here.\n\r", ch);
    return rSPELL_FAILED;
  }
  if ( victim->position != POS_SLEEPING )
  {
    send_to_char("They aren't asleep.\n\r", ch);
    return rSPELL_FAILED;
  }
  if ( !target_name )
  {
    send_to_char("What do you want them to dream about?\n\r", ch );
    return rSPELL_FAILED;
  }

  set_char_color(AT_TELL, victim);
  ch_printf(victim, "You have dreams about %s telling you '%s'.\n\r",
	 PERS(ch, victim), target_name);
  successful_casting( get_skilltype(sn), ch, victim, NULL );
/*  send_to_char("Ok.\n\r", ch);*/
  return rNONE;
}


/* Added spells spiral_blast, scorching surge,
    nostrum, and astral   by SB for Augurer class 
7/10/96 */
ch_ret spell_spiral_blast( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    int hpch;
    bool ch_died;
 
    ch_died = FALSE;

    if ( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )   
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "You fail to breathe.\n\r", ch );
        return rNONE;
    }
 
    for ( vch = ch->in_room->first_person; vch; vch = vch_next )
    {
        vch_next = vch->next_in_room;
        if ( IS_NPC( vch ) && xIS_SET( vch->act, ACT_MOBINVIS ) )
          continue;
	if ( !IS_NPC( vch ) && xIS_SET( vch->act, PLR_WIZINVIS )       
        && vch->pcdata->wizinvis >= LEVEL_IMMORTAL )
          continue;
 
        if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
        {
	    act( AT_MAGIC, "Swirling colours radiate from $n"
                        ", encompassing $N.",  
	          ch, ch, vch, TO_ROOM );
            act( AT_MAGIC, "Swirling colours radiate from you,"
                        " encompassing $N", 
	          ch, ch, vch , TO_CHAR );

            hpch = UMAX( 10, ch->hit );
            dam  = number_range( hpch/14+1, hpch/7 );
            if ( saves_breath( level, vch ) )
                dam /= 2;
            if ( damage( ch, vch, dam, sn ) == rCHAR_DIED || 
		char_died(ch) )
              ch_died = TRUE;
        }
    }

    if ( ch_died )
	return rCHAR_DIED;
    else
	return rNONE;  
}

ch_ret spell_scorching_surge( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] =
    {
          0,
          0,   0,   0,   0,   0,          0,   0,   0,   0,   0,
          0,   0,   0,   0,  30,         35,  40,  45,  50,  55,
         60,  65,  70,  75,  80,         82,  84,  86,  88,  90,
         92,  94,  96,  98, 100,   	102, 104, 106, 108, 110,
 	 112, 114, 116, 118, 120,       122, 124, 126, 128, 130,  
   	 132, 134, 136, 138, 140,  	142, 144, 146, 148, 150,
   	 152, 154, 156, 158, 160,   	162, 164, 166, 168, 170
    };
    int dam;
 
    level       = UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level       = UMAX(0, level);
    dam         = number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell_staff( level, victim ) )
    dam /= 2;
    act( AT_MAGIC, "A fiery current lashes through $n's body!",  
        ch, NULL, NULL, TO_ROOM );
    act( AT_MAGIC, "A fiery current lashes through your body!",
        ch, NULL, NULL, TO_CHAR );   
    return damage( ch, victim, (dam*1.4), sn );
}
 

ch_ret spell_helical_flow( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    SKILLTYPE *skill = get_skilltype(sn);
 
    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   !victim->in_room
    ||   xIS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_NO_ASTRAL)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_DEATH)
    ||   xIS_SET(victim->in_room->room_flags, ROOM_PROTOTYPE)
    ||   xIS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
	||	 IS_SET( victim->in_room->area->flags, AFLAG_NOASTRAL )
	||	 IS_SET( ch->in_room->area->flags, AFLAG_NOASTRAL )
    ||   victim->level >= level + 15
    ||  (CAN_PKILL(victim) && !IS_NPC(ch) && !IS_PKILL(ch))
    ||  (IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE))
    ||  (IS_NPC(victim) && saves_spell_staff( level, victim ))
    ||  !in_hard_range( ch, victim->in_room->area )
    ||  (IS_SET(victim->in_room->area->flags, AFLAG_NOPKILL) && IS_PKILL(ch)))
    {
        failed_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }
 
    act( AT_MAGIC, "$n coils into an ascending column of colour,"
                    " vanishing into thin air.", ch, NULL, NULL, 
		    TO_ROOM );
    char_from_room( ch );
    if ( sysdata.magichell && ch->level > 35 )
    {
        ROOM_INDEX_DATA *troom;

    for ( ;; )
    {
        troom = get_room_index( number_range( 1300, 100000 ) );
        if ( troom )
        if ( !troom->first_person
            && !xIS_SET(troom->room_flags, ROOM_DEATH ) )
        break;
    }

    if ( troom && can_astral_room(troom) )
    char_to_room( ch, troom );
    else
    char_to_room( ch, victim->in_room );
    }
    else
    char_to_room( ch, victim->in_room );
    act( AT_MAGIC, "A coil of colours descends from above, "
                   "revealing $n as it dissipates.", ch, NULL, 
	 	   NULL, TO_ROOM );
    do_look( ch, "auto" );
    return rNONE;
}


	/*******************************************************
	 * Everything after this point is part of SMAUG SPELLS *
	 *******************************************************/

/*
 * saving throw check						-Thoric
 */
bool check_save( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim )
{
    SKILLTYPE *skill = get_skilltype(sn);
    bool saved = FALSE;

    if ( SPELL_FLAG(skill, SF_PKSENSITIVE)
    &&  !IS_NPC(ch) && !IS_NPC(victim) )
	level /= 2;

    if ( skill->saves )
	switch( skill->saves )
	{
	  case SS_POISON_DEATH:
	    saved = saves_poison_death(level, victim);	break;
	  case SS_ROD_WANDS:
	    saved = saves_wands(level, victim);		break;
	  case SS_PARA_PETRI:
	    saved = saves_para_petri(level, victim);	break;
	  case SS_BREATH:
	    saved = saves_breath(level, victim);	break;
	  case SS_SPELL_STAFF:
	    saved = saves_spell_staff(level, victim);	break;
 	}
    return saved;
}

/*
 * Generic offensive spell damage attack			-Thoric
 */
ch_ret spell_attack( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    SKILLTYPE *skill = get_skilltype(sn);
    bool saved = check_save( sn, level, ch, victim );
    int dam;
    ch_ret retcode = rNONE;

    if ( saved && SPELL_SAVE(skill) == SE_NEGATE )
    {
	failed_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }
    if ( skill->dice )
	dam = UMAX( 0, dice_parse( ch, level, skill->dice ) );
    else
	dam = dice( 1, level/2 );
    if ( saved )
    {
	switch( SPELL_SAVE(skill) )
	{
	    case SE_3QTRDAM:	dam = (dam * 3) / 4;	break;
	    case SE_HALFDAM:	dam >>= 1;		break;
	    case SE_QUARTERDAM:	dam >>= 2;		break;
	    case SE_EIGHTHDAM:	dam >>= 3;		break;

	    case SE_ABSORB:	/* victim absorbs spell for hp's */
		act( AT_MAGIC, "$N absorbs your $t!", ch, skill->noun_damage, victim, TO_CHAR );
		act( AT_MAGIC, "You absorb $N's $t!", victim, skill->noun_damage, ch, TO_CHAR );
		act( AT_MAGIC, "$N absorbs $n's $t!", ch, skill->noun_damage, victim, TO_NOTVICT );
		victim->hit = URANGE( 0, victim->hit + dam, victim->max_hit );
		update_pos( victim );
		if ( (dam > 0 && ch->fighting && ch->fighting->who == victim)
		||   (dam > 0 && victim->fighting && victim->fighting->who == ch) )
		{
		    int xp = ch->fighting ? ch->fighting->xp : victim->fighting->xp;
		    int xp_gain = (int) (xp * dam * 2) / victim->max_hit;

		    gain_exp( ch, 0 - xp_gain );
		}
		if ( skill->affects )
		    retcode = spell_affectchar( sn, level, ch, victim );
		return retcode;

	    case SE_REFLECT:	/* reflect the spell to the caster */
	    	return spell_attack( sn, level, victim, ch );
	}
    }
    retcode = damage( ch, victim, dam, sn );
    if ( retcode == rNONE && skill->affects
    &&  !char_died(ch) && !char_died(victim)
    && (!is_affected(victim, sn)
    ||  SPELL_FLAG(skill, SF_ACCUMULATIVE)
    ||  SPELL_FLAG(skill, SF_RECASTABLE)) )
	retcode = spell_affectchar( sn, level, ch, victim );
    return retcode;
}

/*
 * Generic area attack						-Thoric
 */
ch_ret spell_area_attack( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch, *vch_next;
    SKILLTYPE *skill = get_skilltype(sn);
    bool saved;
    bool affects;
    int dam;
    bool ch_died = FALSE;
    ch_ret retcode = rNONE;

    if ( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
	failed_casting( skill, ch, NULL, NULL );
	return rSPELL_FAILED;
    }

    affects = (skill->affects ? TRUE : FALSE);
    if ( skill->hit_char && skill->hit_char[0] != '\0' )
	act( AT_COLORIZE, skill->hit_char, ch, NULL, NULL, TO_CHAR );
    if ( skill->hit_room && skill->hit_room[0] != '\0' )
	act( AT_COLORIZE, skill->hit_room, ch, NULL, NULL, TO_ROOM );

    for ( vch = ch->in_room->first_person; vch; vch = vch_next )
    {
	vch_next = vch->next_in_room;

        if ( IS_NPC( vch ) && xIS_SET( vch->act, ACT_MOBINVIS ) )
          continue;

	if ( !IS_NPC(vch) && xIS_SET(vch->act, PLR_WIZINVIS)
	&&    vch->pcdata->wizinvis >= LEVEL_IMMORTAL )
	   continue;

        if ( vch == ch )
	   continue;

        if ( is_safe( ch, vch, FALSE) )
	   continue;

	if ( !IS_NPC(ch) && !IS_NPC(vch) && !in_arena(ch) && ( !IS_PKILL( ch ) 
		|| !IS_PKILL( vch ) ) )
	   continue;

	saved = check_save( sn, level, ch, vch );
	if ( saved && SPELL_SAVE(skill) == SE_NEGATE )
	{
	    failed_casting( skill, ch, vch, NULL );
	    continue;
	}
	else
	if ( skill->dice )
	    dam = dice_parse(ch, level, skill->dice);
	else
	    dam = dice( 1, level/2 );
	if ( saved )
	{
	    switch( SPELL_SAVE(skill) )
	    {
		case SE_3QTRDAM:	dam = (dam * 3) / 4;	break;
		case SE_HALFDAM:	dam >>= 1;		break;
		case SE_QUARTERDAM:	dam >>= 2;		break;
		case SE_EIGHTHDAM:	dam >>= 3;		break;

		case SE_ABSORB:	/* victim absorbs spell for hp's */
		    act( AT_MAGIC, "$N absorbs your $t!", ch, skill->noun_damage, vch, TO_CHAR );
		    act( AT_MAGIC, "You absorb $N's $t!", vch, skill->noun_damage, ch, TO_CHAR );
		    act( AT_MAGIC, "$N absorbs $n's $t!", ch, skill->noun_damage, vch, TO_NOTVICT );
		    vch->hit = URANGE( 0, vch->hit + dam, vch->max_hit );
		    update_pos( vch );
		    if ( (dam > 0 && ch->fighting && ch->fighting->who == vch)
		    ||   (dam > 0 && vch->fighting && vch->fighting->who == ch) )
		    {
			int xp = ch->fighting ? ch->fighting->xp : vch->fighting->xp;
			int xp_gain = (int) (xp * dam * 2) / vch->max_hit;

			gain_exp( ch, 0 - xp_gain );
		     }
		     continue;

		case SE_REFLECT:	/* reflect the spell to the caster */
	    	    retcode = spell_attack( sn, level, vch, ch );
		    if ( char_died(ch) )
		    {
			ch_died = TRUE;
			break;
		    }
		    continue;
	    }
	}
	retcode = damage( ch, vch, dam, sn );
	if ( retcode == rNONE && affects && !char_died(ch) && !char_died(vch)
	&& (!is_affected(vch, sn)
	||  SPELL_FLAG(skill, SF_ACCUMULATIVE)
	||  SPELL_FLAG(skill, SF_RECASTABLE)) )
	    retcode = spell_affectchar( sn, level, ch, vch );
	if ( retcode == rCHAR_DIED || char_died(ch) )
	{
	    ch_died = TRUE;
	    break;
	}
    }
    return retcode;
}


ch_ret spell_affectchar( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;
    SMAUG_AFF *saf;
    SKILLTYPE *skill = get_skilltype(sn);
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int chance;
    ch_ret retcode = rNONE;

    if ( SPELL_FLAG( skill, SF_RECASTABLE ) )
	affect_strip( victim, sn );
    for ( saf = skill->affects; saf; saf = saf->next )
    {
	if ( saf->location >= REVERSE_APPLY )
	    victim = ch;
	else
	    victim = (CHAR_DATA *) vo;
	/* Check if char has this bitvector already */
	af.bitvector = meb(saf->bitvector);
	if ( saf->bitvector >= 0
	&&   xIS_SET(victim->affected_by, saf->bitvector)
	&&  !SPELL_FLAG(skill, SF_ACCUMULATIVE) )
	   continue;
        /*
         * necessary for affect_strip to work properly...
         */
        switch ( saf->bitvector )
        {
	    default:		af.type = sn;			break;
	    case AFF_POISON:	af.type = gsn_poison;
		chance = ris_save( victim, level, RIS_POISON );
		if ( chance == 1000 )
		{
		    retcode = rVICT_IMMUNE;
		    if ( SPELL_FLAG(skill, SF_STOPONFAIL) )
			return retcode;
		    continue;
		}
		if ( saves_poison_death( chance, victim ) )
		{
		    if ( SPELL_FLAG(skill, SF_STOPONFAIL) )
			return retcode;
		    continue;
		}
		victim->mental_state = URANGE( 30, victim->mental_state + 2, 100 );
		break;
	    case AFF_BLIND:	af.type = gsn_blindness;	break;
	    case AFF_CURSE:	af.type = gsn_curse;		break;
	    case AFF_INVISIBLE:	af.type = gsn_invis;		break;
	    case AFF_SLEEP:	af.type = gsn_sleep;
		chance = ris_save( victim, level, RIS_SLEEP );
		if ( chance == 1000 )
		{
		    retcode = rVICT_IMMUNE;
		    if ( SPELL_FLAG(skill, SF_STOPONFAIL) )
			return retcode;
		    continue;
		}
		break;
	    case AFF_CHARM:		af.type = gsn_charm_person;
		chance = ris_save( victim, level, RIS_CHARM );
		if ( chance == 1000 )
		{
		    retcode = rVICT_IMMUNE;
		    if ( SPELL_FLAG(skill, SF_STOPONFAIL) )
			return retcode;
		    continue;
		}
		break;
	    case AFF_POSSESS:	af.type = gsn_possess;		break;
	}
	af.duration  = dice_parse(ch, level, saf->duration);
	af.modifier  = dice_parse(ch, level, saf->modifier);
	af.location  = saf->location % REVERSE_APPLY;


	if ( af.duration == 0 )
	{
	    int xp_gain;

	    switch( af.location )
	    {
		case APPLY_HIT:

		    victim->hit = URANGE( 0, victim->hit + (!IS_NPC( ch ) && IS_NPC( victim ) ? af.modifier/4 : af.modifier), victim->max_hit );
/*
Blod - August, 2000
		    victim->hit = URANGE( 0, victim->hit + af.modifier, victim->max_hit );
*/
		    update_pos( victim );
		    if ( (af.modifier > 0 && ch->fighting && ch->fighting->who == victim)
		    ||   (af.modifier > 0 && victim->fighting && victim->fighting->who == ch) )
		    {
			int xp = ch->fighting ? ch->fighting->xp : victim->fighting->xp;

			xp_gain = (int) (xp * af.modifier*2) / victim->max_hit;
			gain_exp( ch, 0 - xp_gain );
		    }
		    if ( IS_NPC(victim) && victim->hit <= 0 )
		    	damage( ch, victim, 5, TYPE_UNDEFINED );
		    break;
		case APPLY_MANA:
		    victim->mana = URANGE( 0, victim->mana + af.modifier, victim->max_mana );
		    update_pos( victim );
		    break;
		case APPLY_MOVE:
		    victim->move = URANGE( 0, victim->move + af.modifier, victim->max_move );
		    update_pos( victim );
		    break;
		default:
		    affect_modify( victim, &af, TRUE );
		    break;
	    }
	}
	else
	if ( SPELL_FLAG( skill, SF_ACCUMULATIVE ) )
	    affect_join( victim, &af );
	else
	    affect_to_char( victim, &af );
    }
    update_pos( victim );
    return retcode;
}


/*
 * Generic spell affect						-Thoric
 */
ch_ret spell_affect( int sn, int level, CHAR_DATA *ch, void *vo )
{
    SMAUG_AFF *saf;
    SKILLTYPE *skill = get_skilltype(sn);
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool groupsp;
    bool areasp;
    bool hitchar = FALSE, hitroom = FALSE, hitvict = FALSE;
    ch_ret retcode;

    if ( !skill->affects )
    {
	bug( "spell_affect has no affects sn %d", sn );
	return rNONE;
    }
    if ( SPELL_FLAG(skill, SF_GROUPSPELL) )
	groupsp = TRUE;
    else
	groupsp = FALSE;

    if ( SPELL_FLAG(skill, SF_AREA ) )
	areasp = TRUE;
    else
	areasp = FALSE;

    if ( SPELL_FLAG( skill, SF_NOMOB )
    &&   victim
    &&   IS_NPC( victim ) )
	return rSPELL_FAILED;

    if ( !groupsp && !areasp )
    {
	/* Can't find a victim */
	if ( !victim )
	{
	    failed_casting( skill, ch, victim, NULL );
	    return rSPELL_FAILED;
	}

	if ( SPELL_FLAG( skill, SF_NOMOB ) && IS_NPC( victim ) )
	    return rSPELL_FAILED;

	if ( (skill->type != SKILL_HERB
	&&    IS_SET( victim->immune, RIS_MAGIC ))
	||    is_immune( victim, SPELL_DAMAGE(skill) ) )
	{
	    immune_casting( skill, ch, victim, NULL );
	    return rSPELL_FAILED;
	}

	/* Spell is already on this guy */
	if ( is_affected( victim, sn )
	&&  !SPELL_FLAG( skill, SF_ACCUMULATIVE )
	&&  !SPELL_FLAG( skill, SF_RECASTABLE ) )
	{
	    failed_casting( skill, ch, victim, NULL );
	    return rSPELL_FAILED;
	}

	if ( (saf = skill->affects) && !saf->next
	&&    saf->location == APPLY_STRIPSN
	&&   !is_affected( victim, dice_parse(ch, level, saf->modifier) ) )
	{
	    failed_casting( skill, ch, victim, NULL );
	    return rSPELL_FAILED;
	}

	if ( check_save( sn, level, ch, victim ) )
	{
	    failed_casting( skill, ch, victim, NULL );
	    return rSPELL_FAILED;
	}
    }
    else
    {
	if ( skill->hit_char && skill->hit_char[0] != '\0' )
	{
	    if ( strstr(skill->hit_char, "$N") )
		hitchar = TRUE;
	    else
		act( AT_COLORIZE, skill->hit_char, ch, NULL, NULL, TO_CHAR );
	}
	if ( skill->hit_room && skill->hit_room[0] != '\0' )
	{
	    if ( strstr(skill->hit_room, "$N") )
		hitroom = TRUE;
	    else
		act( AT_COLORIZE, skill->hit_room, ch, NULL, NULL, TO_ROOM );
	}
	if ( skill->hit_vict && skill->hit_vict[0] != '\0' )
	    hitvict = TRUE;
	if ( victim )
	  victim = victim->in_room->first_person;
	else
	  victim = ch->in_room->first_person;
    }
    if ( !victim )
    {
	bug( "spell_affect: could not find victim: sn %d", sn );
	failed_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    for ( ; victim; victim = victim->next_in_room )
    {
	if ( groupsp || areasp )
	{
	    if ((groupsp && !is_same_group( victim, ch ))
	    ||	 IS_SET( victim->immune, RIS_MAGIC )
	    ||   is_immune( victim, SPELL_DAMAGE(skill) )
	    ||   check_save(sn, level, ch, victim)
	    || ( SPELL_FLAG(skill, SF_NOMOB) && IS_NPC(victim) )
	    || (!SPELL_FLAG(skill, SF_RECASTABLE) && is_affected(victim, sn)))
		continue;

	    if ( hitvict && ch != victim )
	    {
		act( AT_COLORIZE, skill->hit_vict, ch, NULL, victim, TO_VICT );
		if ( hitroom )
		{
		   act( AT_COLORIZE, skill->hit_room, ch, NULL, victim, TO_NOTVICT );
		   act( AT_COLORIZE, skill->hit_room, ch, NULL, victim, TO_CHAR );
		}
	    }
	    else
	    if ( hitroom )
		act( AT_COLORIZE, skill->hit_room, ch, NULL, victim, TO_ROOM );
	    if ( ch == victim )
	    {
		if ( hitvict )
		  act( AT_COLORIZE, skill->hit_vict, ch, NULL, ch, TO_CHAR );
		else
		if ( hitchar )
		  act( AT_COLORIZE, skill->hit_char, ch, NULL, ch, TO_CHAR );
	    }
	    else
	    if ( hitchar )
		act( AT_COLORIZE, skill->hit_char, ch, NULL, victim, TO_CHAR );
	}
	retcode = spell_affectchar( sn, level, ch, victim );
	if ( !groupsp && !areasp )
	{
	    if ( retcode == rVICT_IMMUNE )
		immune_casting( skill, ch, victim, NULL );
	    else
		successful_casting( skill, ch, victim, NULL );
	    break;
	}
    }
    return rNONE;
}

/*
 * Generic inventory object spell				-Thoric
 */
ch_ret spell_obj_inv( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( !obj )
    {
	failed_casting( skill, ch, NULL, NULL );
	return rNONE;
    }

    switch( SPELL_ACTION(skill) )
    {
	default:
	case SA_NONE:
	  return rNONE;

	case SA_CREATE:
	  if ( SPELL_FLAG(skill, SF_WATER) )	/* create water */
	  {
	    int water;
	    WEATHER_DATA *weath = ch->in_room->area->weather;

	    if ( obj->item_type != ITEM_DRINK_CON )
	    {
		send_to_char( "It is unable to hold water.\n\r", ch );
		return rSPELL_FAILED;
	    }

	    if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
	    {
		send_to_char( "It contains some other liquid.\n\r", ch );
		return rSPELL_FAILED;
	    }

	    water = UMIN( (skill->dice ? dice_parse(ch, level, skill->dice) : level)
		       * (weath->precip >= 0 ? 2 : 1),
			obj->value[0] - obj->value[1] );

	    if ( water > 0 )
	    {
		separate_obj(obj);
		obj->value[2] = LIQ_WATER;
		obj->value[1] += water;
		if ( !is_name( "water", obj->name ) )
		{
		    char buf[MAX_STRING_LENGTH];

		    sprintf( buf, "%s water", obj->name );
		    STRFREE( obj->name );
		    obj->name = STRALLOC( buf );
		}
	     }
	     successful_casting( skill, ch, NULL, obj );
	     return rNONE;
	  }
	  if ( SPELL_DAMAGE(skill) == SD_FIRE )	/* burn object */
	  {
	     /* return rNONE; */
	  }
	  if ( SPELL_DAMAGE(skill) == SD_POISON	/* poison object */
	  ||   SPELL_CLASS(skill)  == SC_DEATH )
	  {
	     switch( obj->item_type )
	     {
		default:
		  failed_casting( skill, ch, NULL, obj );
		  break;
	        case ITEM_COOK:
		case ITEM_FOOD:
		case ITEM_DRINK_CON:
		  separate_obj(obj);
		  obj->value[3] = 1;
		  successful_casting( skill, ch, NULL, obj );
		  break;
	     }
	     return rNONE;
	  }
	  if ( SPELL_CLASS(skill) == SC_LIFE	/* purify food/water */
	  &&  (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON
	       || obj->item_type == ITEM_COOK) )
	  {
	     switch( obj->item_type )
	     {
		default:
		  failed_casting( skill, ch, NULL, obj );
		  break;
		case ITEM_COOK:
		case ITEM_FOOD:
		case ITEM_DRINK_CON:
		  separate_obj(obj);
		  obj->value[3] = 0;
		  successful_casting( skill, ch, NULL, obj );
		  break;
	     }
	     return rNONE;
	  }
	  
	  if ( SPELL_CLASS(skill) != SC_NONE )
	  {
	     failed_casting( skill, ch, NULL, obj );
	     return rNONE;
	  }
	  switch( SPELL_POWER(skill) )		/* clone object */
	  {
	     OBJ_DATA *clone;

	     default:
	     case SP_NONE:
		if ( ch->level - obj->level < 10
		||   obj->cost > ch->level * get_curr_int(ch) * get_curr_wis(ch) )
		{
		   failed_casting( skill, ch, NULL, obj );
		   return rNONE;
		}
		break;
	     case SP_MINOR:
		if ( ch->level - obj->level < 20
		||   obj->cost > ch->level * get_curr_int(ch) / 5 )
		{
		   failed_casting( skill, ch, NULL, obj );
		   return rNONE;
		}
		break;
	     case SP_GREATER:
		if ( ch->level - obj->level < 5
		||   obj->cost > ch->level * 10 * get_curr_int(ch) * get_curr_wis(ch) )
		{
		   failed_casting( skill, ch, NULL, obj );
		   return rNONE;
		}
		break;
	     case SP_MAJOR:
		if ( ch->level - obj->level < 0
		||   obj->cost > ch->level * 50 * get_curr_int(ch) * get_curr_wis(ch) )
		{
		   failed_casting( skill, ch, NULL, obj );
		   return rNONE;
		}
		break;
	     clone = clone_object(obj);
	     clone->timer = skill->dice ? dice_parse(ch, level, skill->dice) : 0;
	     obj_to_char( clone, ch );
	     successful_casting( skill, ch, NULL, obj );
	  }
	  return rNONE;

	case SA_DESTROY:
	case SA_RESIST:
	case SA_SUSCEPT:
	case SA_DIVINATE:
	  if ( SPELL_DAMAGE(skill) == SD_POISON ) /* detect poison */
	  {
	     if ( obj->item_type == ITEM_DRINK_CON
	     ||   obj->item_type == ITEM_FOOD 
	     ||   obj->item_type == ITEM_COOK )
	     {
		if ( obj->item_type == ITEM_COOK && obj->value[2] == 0)
		    send_to_char("It looks undercooked.\n\r", ch );
		else if ( obj->value[3] != 0 )
		    send_to_char( "You smell poisonous fumes.\n\r", ch );
		else
		    send_to_char( "It looks very delicious.\n\r", ch );
	     }
	     else
		send_to_char( "It doesn't look poisoned.\n\r", ch );
	     return rNONE;
	  }
	  return rNONE;
	case SA_OBSCURE:			/* make obj invis */
	  if ( IS_OBJ_STAT(obj, ITEM_INVIS) 
	  ||   chance(ch, skill->dice ? dice_parse(ch, level, skill->dice) : 20))
          {
	     failed_casting( skill, ch, NULL, NULL );
     	     return rSPELL_FAILED;
	  }
	  successful_casting( skill, ch, NULL, obj );
	  xSET_BIT(obj->extra_flags, ITEM_INVIS);
	  return rNONE;

	case SA_CHANGE:
	  return rNONE;
    }
    return rNONE;
}

/*
 * Generic object creating spell				-Thoric
 */
ch_ret spell_create_obj( int sn, int level, CHAR_DATA *ch, void *vo )
{
    SKILLTYPE *skill = get_skilltype(sn);
    int lvl;
    int vnum = skill->value;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *oi;

    switch( SPELL_POWER(skill) )
    {
	default:
	case SP_NONE:	 lvl = 10;	break;
	case SP_MINOR:	 lvl = 0;	break;
	case SP_GREATER: lvl = level/2; break;
	case SP_MAJOR:	 lvl = level;	break;
    }

    /*
     * Add predetermined objects here
     */
    if ( vnum == 0 )
    {
	if ( !str_cmp( target_name, "sword" ) )
	  vnum = OBJ_VNUM_SCHOOL_SWORD;
	if ( !str_cmp( target_name, "shield" ) )
	  vnum = OBJ_VNUM_SCHOOL_SHIELD;
    }

    if ( (oi=get_obj_index(vnum)) == NULL
    ||   (obj=create_object(oi, lvl)) == NULL )
    {
	failed_casting( skill, ch, NULL, NULL );
	return rNONE;
    }
    obj->timer = skill->dice ? dice_parse( ch, level, skill->dice ) : 0;
    successful_casting( skill, ch, NULL, obj );
    if ( CAN_WEAR(obj, ITEM_TAKE) )
      obj_to_char( obj, ch );
    else
      obj_to_room( obj, ch->in_room );
    return rNONE;
}

/*
 * Generic mob creating spell					-Thoric
 */
ch_ret spell_create_mob( int sn, int level, CHAR_DATA *ch, void *vo )
{
    SKILLTYPE *skill = get_skilltype(sn);
    int lvl;
    int vnum = skill->value;
    CHAR_DATA *mob;
    MOB_INDEX_DATA *mi;
    AFFECT_DATA af;

    /* set maximum mob level */
    switch( SPELL_POWER(skill) )
    {
	default:
	case SP_NONE:	 lvl = 20;	break;
	case SP_MINOR:	 lvl = 5;	break;
	case SP_GREATER: lvl = level/2; break;
	case SP_MAJOR:	 lvl = level;	break;
    }

    /*
     * Add predetermined mobiles here
     */
    if ( vnum == 0 )
    {
	if ( !str_cmp( target_name, "cityguard" ) )
	  vnum = MOB_VNUM_CITYGUARD;
	if ( !str_cmp( target_name, "vampire" ) )
	  vnum = MOB_VNUM_VAMPIRE;
    }

    if ( (mi=get_mob_index(vnum)) == NULL
    ||   (mob=create_mobile(mi)) == NULL )
    {
	failed_casting( skill, ch, NULL, NULL );
	return rNONE;
    }
    mob->level   = UMIN( lvl, skill->dice ? dice_parse(ch, level, skill->dice) : mob->level );
    mob->armor	 = interpolate( mob->level, 100, -100 );

    mob->max_hit = mob->level * 8 + number_range(
				mob->level * mob->level / 4,
				mob->level * mob->level );
    mob->hit	 = mob->max_hit;
    mob->gold	 = 0;
    successful_casting( skill, ch, mob, NULL );
    char_to_room( mob, ch->in_room );
    add_follower( mob, ch );
    af.type      = sn;
    af.duration  = (number_fuzzy( (level + 1) / 3 ) + 1) * DUR_CONV;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = meb(AFF_CHARM);
    affect_to_char( mob, &af );
    return rNONE;
}

ch_ret ranged_attack( CHAR_DATA *, char *, OBJ_DATA *, OBJ_DATA *, sh_int, sh_int );

/*
 * Generic handler for new "SMAUG" spells			-Thoric
 */
ch_ret spell_smaug( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    struct skill_type *skill = get_skilltype(sn);

    /* Put this check in to prevent crashes from this getting a bad skill */

    if ( !skill )
    {
    	bug ( "spell_smaug: Called with a null skill for sn %d", sn );
	return rERROR;
    }

    switch( skill->target )
    {
	case TAR_IGNORE:

	  /* offensive area spell */
	  if ( SPELL_FLAG(skill, SF_AREA)
	  && ((SPELL_ACTION(skill) == SA_DESTROY
	  &&   SPELL_CLASS(skill) == SC_LIFE)
	  ||  (SPELL_ACTION(skill) == SA_CREATE
	  &&   SPELL_CLASS(skill) == SC_DEATH)) )
		return spell_area_attack( sn, level, ch, vo );

	  if ( SPELL_ACTION(skill) == SA_CREATE )
	  {
		if ( SPELL_FLAG(skill, SF_OBJECT) )	/* create object */
		  return spell_create_obj( sn, level, ch,  vo );
		if ( SPELL_CLASS(skill) == SC_LIFE )	/* create mob */
		  return spell_create_mob( sn, level, ch,  vo );
	  }

	  /* affect a distant player */
	  if ( SPELL_FLAG(skill, SF_DISTANT)
          &&  (victim = get_char_world( ch, target_name))
	  &&  !xIS_SET( victim->in_room->room_flags, ROOM_NO_ASTRAL)
	  &&   SPELL_FLAG(skill, SF_CHARACTER) )
		return spell_affect(sn, level, ch, get_char_world( ch, target_name ));

	  /* affect a player in this room (should have been TAR_CHAR_XXX) */
	  if ( SPELL_FLAG(skill, SF_CHARACTER) )
		return spell_affect(sn, level, ch, get_char_room( ch, target_name ));

	  if ( skill->range > 0 && (
	   		(SPELL_ACTION(skill) == SA_DESTROY
			&& SPELL_CLASS(skill) == SC_LIFE)
			|| (SPELL_ACTION(skill) == SA_CREATE
			&& SPELL_CLASS(skill) == SC_DEATH) ) )
	     return ranged_attack(ch, ranged_target_name, NULL, NULL, sn, skill->range );
	  /* will fail, or be an area/group affect */
	  return spell_affect( sn, level, ch, vo );

	case TAR_CHAR_OFFENSIVE:

	  /* a regular damage inflicting spell attack */
	  if ( (SPELL_ACTION(skill) == SA_DESTROY
	  &&    SPELL_CLASS(skill) == SC_LIFE)
	  ||   (SPELL_ACTION(skill) == SA_CREATE
	  &&    SPELL_CLASS(skill) == SC_DEATH) ) 
			return spell_attack( sn, level, ch, vo );

	  /* a nasty spell affect */
	  return spell_affect( sn, level, ch, vo );

	case TAR_CHAR_DEFENSIVE:
	case TAR_CHAR_SELF:
          if ( SPELL_FLAG(skill, SF_NOFIGHT ) &&
               ( ch->position == POS_FIGHTING
               || ch->position ==  POS_EVASIVE
               || ch->position ==  POS_DEFENSIVE
               || ch->position ==  POS_AGGRESSIVE
               || ch->position ==  POS_BERSERK ) )
          {
              send_to_char( "You can't concentrate enough for that!\n\r", ch );
              return rNONE;
          }

	  if ( vo && SPELL_ACTION(skill) == SA_DESTROY )
	  {
		CHAR_DATA *victim = (CHAR_DATA *) vo;

		/* cure poison */
		if ( SPELL_DAMAGE(skill) == SD_POISON )
		{
		   if ( is_affected( victim, gsn_poison ) )
		   {
			affect_strip( victim, gsn_poison );
			victim->mental_state = URANGE( -100, victim->mental_state, -10 );
			successful_casting( skill, ch, victim, NULL );
			return rNONE;
		   }
		   failed_casting( skill, ch, victim, NULL );
		   return rSPELL_FAILED;
		}
		/* cure blindness */
		if ( SPELL_CLASS(skill) == SC_ILLUSION )
		{
		    if ( is_affected( victim, gsn_blindness ) )
		    {
			affect_strip( victim, gsn_blindness );
			successful_casting( skill, ch, victim, NULL );
			return rNONE;
		   }
		   failed_casting( skill, ch, victim, NULL );
		   return rSPELL_FAILED;
		}
	  }
	  return spell_affect( sn, level, ch, vo );

	case TAR_OBJ_INV:
	  return spell_obj_inv( sn, level, ch, vo );
    }
    return rNONE;
}



/* Haus' new, new mage spells follow */

/*
 *  4 Energy Spells
 */
ch_ret spell_ethereal_fist( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level       = UMAX(0, level);
    level       = UMIN(35, level);
    dam         = 1.3*(level*number_range( 1, 6 ))-31;
    dam         = UMAX(0,dam);

    if ( saves_spell_staff( level, victim ) )
	dam/=4;

    return damage( ch, victim, dam, sn );
}


ch_ret spell_spectral_furor( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level	= UMAX(0, level);
    level	= UMIN(16, level);
    dam		= 1.3*(level*number_range( 1, 7 )+7);
    if ( saves_spell_staff( level, victim ) )
	dam /= 2;
    return damage( ch, victim, dam, sn );
}

ch_ret spell_hand_of_chaos( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level       = UMAX(0, level);
    level       = UMIN(18, level);
    dam         = 1.3*(level*number_range( 1, 7 )+9);

    if ( saves_spell_staff( level, victim ) )
	dam/=4;
    return damage( ch, victim, dam, sn );
}


ch_ret spell_disruption( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level	= UMAX(0, level);
    level	= UMIN(14, level);
    dam		= 1.3*(level*number_range( 1, 6 )+8);

    if ( saves_spell_staff( level, victim ) )
	dam = 0;
    return damage( ch, victim, dam, sn );
}

ch_ret spell_sonic_resonance( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level       = UMAX(0, level);
    level       = UMIN(23, level);
    dam         = 1.3*(level*number_range( 1, 8 ));

    if ( saves_spell_staff( level, victim ) )
	dam = dam*3/4;
    return damage( ch, victim, dam, sn );
}

/*
 * 3 Mentalstate spells
 */
ch_ret spell_mind_wrack( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    /* decrement mentalstate by up to 50 */

    level	= UMAX(0, level);
    dam		= number_range( 0, 0 );
    if ( saves_spell_staff( level, victim ) )
	dam /= 2;
    return damage( ch, victim, dam, sn );
}

ch_ret spell_mind_wrench( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    
    /* increment mentalstate by up to 50 */

    level	= UMAX(0, level);
    dam		= number_range( 0, 0 );
    if ( saves_spell_staff( level, victim ) )
	dam /= 2;
    return damage( ch, victim, dam, sn );
}


/* Non-offensive spell! */
ch_ret spell_revive( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    /* set mentalstate to mentalstate/2 */
    level	= UMAX(0, level);
    dam		= number_range( 0, 0 );
    if ( saves_spell_staff( level, victim ) )
	dam /= 2;
    return damage( ch, victim, dam, sn );
}

/*
 * n Acid Spells
 */
ch_ret spell_sulfurous_spray( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level       = UMAX(0, level);
    level       = UMIN(19, level);
    dam         = 1.3*(2*level*number_range( 1, 7 )+11);

    if ( saves_spell_staff( level, victim ) )
	dam /= 4;
    return damage( ch, victim, dam, sn );
}

ch_ret spell_caustic_fount( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level       = UMAX(0, level);
    level       = UMIN(42, level);
    dam         = 1.3*(2*level*number_range( 1, 6 ))-31;
    dam         = UMAX(0,dam);

    if ( saves_spell_staff( level, victim ) )
	dam = dam*1/2;
    return damage( ch, victim, dam, sn );
}

ch_ret spell_acetum_primus( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level       = UMAX(0, level);
    dam         = 1.3*(2*level*number_range( 1, 4 )+7);

    if ( saves_spell_staff( level, victim ) )
	dam = 3*dam/4;
    return damage( ch, victim, dam, sn );
}

/*
 *  Electrical
 */

ch_ret spell_galvanic_whip( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level	= UMAX(0, level);
    level	= UMIN(10, level);
    dam		= 1.3*(level*number_range( 1, 6 )+5);

    if ( saves_spell_staff( level, victim ) )
	dam /= 2;
    return damage( ch, victim, dam, sn );
}

ch_ret spell_magnetic_thrust( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level       = UMAX(0, level);
    level       = UMIN(29, level);
    dam         = .65* ((5*level*number_range( 1, 6 )) +16);

    if ( saves_spell_staff( level, victim ) )
	dam /= 3;
    return damage( ch, victim, dam, sn );
}

ch_ret spell_quantum_spike( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam,l;

    level       = UMAX(0, level);
    l 		= 2*(level/10);
    dam         = 1.3*(l*number_range( 1,30)+145);

    if ( saves_spell_staff( level, victim ) )
	dam /= 2;
    return damage( ch, victim, dam, sn );
}

/*
 * Black-magicish guys
 */

/* L2 Mage Spell */
ch_ret spell_black_hand( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level	= UMAX(0, level);
    level	= UMIN(5, level);
    dam		= 1.3*(level*number_range( 1, 6 )+3);

    if ( saves_spell_staff( level, victim ) )
	dam /= 4;
    return damage( ch, victim, dam, sn );
}

ch_ret spell_black_fist( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level       = UMAX(0, level);
    level       = UMIN(30, level);
    dam         = 1.3*(level*number_range( 1, 9 )+4);

    if ( saves_spell_staff( level, victim ) )
	dam /= 4;
    return damage( ch, victim, dam, sn );
}

ch_ret spell_black_lightning( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam,l;

    level       = UMAX(0, level);
    l 		= 2*level/10;
    dam         = 1.3*(l*number_range(1,50)+135);

    if ( saves_spell_staff( level, victim ) )
	dam /= 4;
    return damage( ch, victim, dam, sn );
}

ch_ret spell_midas_touch( int sn, int level, CHAR_DATA *ch, void *vo )
{
    char buf [MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH]; 
    char buf3[MAX_STRING_LENGTH];
    CHAR_DATA *victim = NULL;
    int val;
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if ( IS_OBJ_STAT(obj, ITEM_NODROP) || IS_OBJ_STAT(obj, ITEM_PERMANENT) )
    {
	send_to_char( "You can't seem to let go of it.\n\r", ch );
	return rSPELL_FAILED;  
    }

    if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE )
    &&   get_trust( ch ) < LEVEL_IMMORTAL ) /* was victim instead of ch!  Thanks Nick Gammon */
    {
	send_to_char( "That item is not for mortal hands to touch!\n\r", ch );
	return rSPELL_FAILED;   /* Thoric */
    }

    if ( !CAN_WEAR(obj, ITEM_TAKE) 
       || ( obj->item_type == ITEM_CORPSE_NPC )
       || ( obj->item_type == ITEM_CORPSE_PC )
       || ( obj->item_type == ITEM_KEY )
       )
    {
       send_to_char( "You cannot seem to turn this item to gold!\n\r", ch);
       return rNONE;
    }

    separate_obj(obj);  /* nice, alty :) */

    val = obj->cost/3;
    val = UMAX(0, val);

    if(  obj->item_type==ITEM_WEAPON ){
	switch( number_bits(2) )
	{
	   case 0: victim = get_char_world( ch, "shmalnoth");	break;
	   case 1: victim = get_char_world( ch, "midas001"); break;
	   case 2: victim = get_char_world( ch, "midas003"); break;
	   case 3: victim = get_char_world( ch, "shmalnak" ); break;
        }
    } else if (  obj->item_type==ITEM_ARMOR ){
	switch( number_bits(3) )
	{
	   case 0: victim = get_char_world( ch, "shmalnoth");	break;
	   case 1: victim = get_char_world( ch, "shmalnoth");	break;
	   case 2: victim = get_char_world( ch, "midas001"); break;
	   case 3: victim = get_char_world( ch, "midas001"); break;
	   case 4: victim = get_char_world( ch, "midas003"); break;
	   case 5: victim = get_char_world( ch, "midas005"); break;
	   case 6: victim = get_char_world( ch, "crafter" ); break;
	   case 7: victim = get_char_world( ch, "crafter" ); break;
        }
    } else if (  obj->item_type==ITEM_SCROLL ){
	switch( number_bits(2) )
	{
	   case 0: victim = get_char_world( ch, "tatorious");	break;
	   case 1: victim = get_char_world( ch, "midas002");	break;
	   case 2: victim = get_char_world( ch, "midas002");	break;
	   case 3: victim = get_char_world( ch, "midas007");	break;
        }
    } else if (  obj->item_type==ITEM_STAFF ){
	switch( number_bits(2) )
	{
	   case 0: victim = get_char_world( ch, "tatorious");	break;
	   case 1: victim = get_char_world( ch, "midas007");	break;
	   case 2: victim = get_char_world( ch, "midas007");	break;
	   case 3: victim = get_char_world( ch, "midas008");	break;
        }
    } else if (  obj->item_type==ITEM_WAND ){
	switch( number_bits(2) )
	{
	   case 0: victim = get_char_world( ch, "tatorious");	break;
	   case 1: victim = get_char_world( ch, "midas006");	break;
	   case 2: victim = get_char_world( ch, "midas007");	break;
	   case 3: victim = get_char_world( ch, "midas008");	break;
        }
    } else {
       victim = get_char_world( ch, "terror");
       /* victim = NULL; */
    }

    if ( obj->item_type != ITEM_TREASURE )
    {
      if ( get_curr_int( ch ) > 19 && get_curr_wis( ch ) > 19 )
      {
	  if ( number_bits( 3 ) == 1 )
	  {
	    obj_from_char( obj );
	    obj->item_type = ITEM_TREASURE;
	    obj->timer = -1;
	    obj->value[0] = 0;
	    obj->value[1] = 0;
	    obj->value[2] = 0;
	    obj->value[3] = 0;
	    obj->value[4] = 0;
	    obj->value[5] = 0;
 	    obj->cost = obj->cost*2;
	    obj->weight = obj->weight*2;
	    ch_printf_color( ch, "&YA yellowish glow slowly seeps over the item, leaving behind pure gold.\n\r" );
	    sprintf( buf, "(Gold) %s", obj->short_descr );
	    STRFREE( obj->short_descr );
	    obj->short_descr = STRALLOC( buf );
	    sprintf( buf2, "%s %s", obj->name, "midas gold" );
	    STRFREE( obj->name );
	    obj->name = STRALLOC( buf2 );
	    sprintf( buf3, "(Gold) %s", obj->description );
	    STRFREE( obj->description );
	    obj->description = STRALLOC( buf3 );
	    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
	    {
		act( AT_ACTION, "The item is suddenly too heavy for you to carry.", ch, NULL, NULL, TO_CHAR );
		act( AT_ACTION, "You drop $p.", ch, obj, NULL, TO_CHAR );
		act( AT_ACTION, "$n drops $p.", ch, obj, NULL, TO_ROOM );
		obj_to_room( obj, ch->in_room );
	    }
	    else
		obj_to_char( obj, ch );
	    return rNONE;
	  }
      }
      else if ( number_bits( 6 ) == 1 )
      {
	  obj_from_char( obj );
	  obj->item_type = ITEM_TREASURE;
          obj->timer = -1;
          obj->value[0] = 0;
          obj->value[1] = 0;
          obj->value[2] = 0;
          obj->value[3] = 0;
          obj->value[4] = 0;
          obj->value[5] = 0;
 	  obj->cost = obj->cost*2;
	  obj->weight = obj->weight*2;
	  ch_printf_color( ch, "&YA yellowish glow slowly seeps over the item, leaving behind pure gold.\n\r" );
	  sprintf( buf, "(Gold) %s", obj->short_descr );
	  STRFREE( obj->short_descr );
	  obj->short_descr = STRALLOC( buf );
	  sprintf( buf2, "%s %s", obj->name, "midas gold" );
	  STRFREE( obj->name );
	  obj->name = STRALLOC( buf2 );
          sprintf( buf3, "(Gold) %s", obj->description );
          STRFREE( obj->description );
          obj->description = STRALLOC( buf3 );
	  if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
          {
              act( AT_ACTION, "The item is suddenly too heavy for you to carry.", ch, NULL, NULL, TO_CHAR );
              act( AT_ACTION, "You drop $p.", ch, obj, NULL, TO_CHAR );
              act( AT_ACTION, "$n drops $p.", ch, obj, NULL, TO_ROOM );
              obj_to_room( obj, ch->in_room );
          }
          else
              obj_to_char( obj, ch );
	  return rNONE;
      }
    }

    if (  victim == NULL || !IS_NPC( victim ) )
    {
        ch->gold += val;

        if ( obj_extracted(obj) )
          return rNONE;
        if ( cur_obj == obj->serial )
          global_objcode = rOBJ_SACCED;
        extract_obj( obj );
           send_to_char( "You transmogrify the item to gold!\n\r", ch);
           return rNONE;
    }


    if ( ( victim->carry_weight + get_obj_weight ( obj ) ) > can_carry_w(victim) 
    ||	(IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE)))
    {
        ch->gold += val;

        if ( obj_extracted(obj) )
          return rNONE;
        if ( cur_obj == obj->serial )
          global_objcode = rOBJ_SACCED;
        extract_obj( obj );
           send_to_char( "You transmogrify the item to gold!\n\r", ch);
           return rNONE;
    }

    ch->gold += val;
    obj_from_char( obj );
    obj_to_char( obj, victim );

    send_to_char( "You transmogrify the item to gold!\n\r", ch );
    return rNONE;
}
