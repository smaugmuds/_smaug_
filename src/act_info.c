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
 *			     Informational module			    *
 ****************************************************************************/


#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "mud.h"


/* Had to add unknowns because someone added new positions and didn't
 * update them.  Just a band-aid till I have time to fix it right.
 * This was found thanks to mud@mini.axcomp.com pointing it out :)
 * --Shaddai
 */

char *  const   imm_badge   [15] =
{
    "&G&r(&wSupreme Entity&r)",
    "&G&r(&wInfinite&G&r)",
    "&G&r(&wEternal&G&r)",
    "&G&r(&wAncient&G&r)",
    "&G&r(&wExalted God&G&r)",
    "&G(&wAscendant God&G)",
    "&G(&wGreater God&G)",
    "&G(&wGod&G)", 
    "&G(&wLesser God&G)",
    "&G(&wImmortal&G)",
    "&G(&wDemi God&G)",
    "&G(&WSavior&G)",
    "&G(&WCreator&G)",
    "&G(&WAcolyte&G)",
    "&G&W(Neophyte)"
};

char *	const	where_name	[] =
{
    "<used as light>      ",
    "<worn on finger>     ",
    "<worn on finger>     ",
    "<worn around neck>   ",
    "<worn around neck>   ",
    "<worn on body>       ",
    "<worn on head>       ",
    "<worn on legs>       ",
    "<worn on feet>       ",
    "<worn on hands>      ",
    "<worn on arms>       ",
    "<worn as shield>     ",
    "<worn about body>    ",
    "<worn about waist>   ",
    "<worn around wrist>  ",
    "<worn around wrist>  ",
    "<wielded>            ",
    "<held>               ",
    "<dual wielded>       ",
    "<worn on ears>       ",
    "<worn on eyes>       ",
    "<missile wielded>    ",
    "<worn on back>       ",
    "<worn over face>     ",
    "<worn around ankle>  ",
    "<worn around ankle>  ",
    "<BUG Inform Nivek>  ",
    "<BUG Inform Nivek>  ",
    "<BUG Inform Nivek>  "
};


/*
 * Local functions.
 */
void	show_char_to_char_0	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char_1	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char	args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
bool	check_blind		args( ( CHAR_DATA *ch ) );
void    show_condition          args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

	//Similar Helpfile Snippet Declarations
        sh_int str_similarity( const char *astr, const char *bstr );
        sh_int str_prefix_level( const char *astr, const char *bstr );
        void similar_help_files(CHAR_DATA *ch, char *argument);

extern 	bool	in_same_house	args( ( CHAR_DATA *ch, CHAR_DATA *vch ) );



char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
    static char buf[MAX_STRING_LENGTH];
    bool glowsee = FALSE;

    /* can see glowing invis items in the dark */
    if ( IS_OBJ_STAT(obj, ITEM_GLOW) && IS_OBJ_STAT(obj, ITEM_INVIS)
    &&  !IS_AFFECTED(ch, AFF_TRUESIGHT) && !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
	glowsee = TRUE;

    buf[0] = '\0';
    if ( IS_OBJ_STAT(obj, ITEM_INVIS)     )   strcat( buf, "(Invis) "     );
    if (  (IS_AFFECTED(ch, AFF_DETECT_EVIL) || ch->class==CLASS_PALADIN)
	 && IS_OBJ_STAT(obj, ITEM_EVIL)   )   strcat( buf, "(Red Aura) "  );

    if ( ch->class==CLASS_PALADIN
	 && ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && !IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && !IS_OBJ_STAT(obj, ITEM_ANTI_GOOD))   )
          strcat( buf, "(Flaming Red) "  );
    if ( ch->class==CLASS_PALADIN
	 && ( !IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && !IS_OBJ_STAT(obj, ITEM_ANTI_GOOD))   )
          strcat( buf, "(Flaming Grey) "  );
    if ( ch->class==CLASS_PALADIN
	 && (!IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && !IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_OBJ_STAT(obj, ITEM_ANTI_GOOD))   )
          strcat( buf, "(Flaming White) "  );


    if ( ch->class==CLASS_PALADIN
	 && ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && !IS_OBJ_STAT(obj, ITEM_ANTI_GOOD))   )
          strcat( buf, "(Smouldering Red-Grey) "  );
    if ( ch->class==CLASS_PALADIN
	 && ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && !IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_OBJ_STAT(obj, ITEM_ANTI_GOOD))   )
          strcat( buf, "(Smouldering Red-White) "  );
    if ( ch->class==CLASS_PALADIN
	 && ( !IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_OBJ_STAT(obj, ITEM_ANTI_GOOD))   )
          strcat( buf, "(Smouldering Grey-White) "  );

    if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC)
	 && IS_OBJ_STAT(obj, ITEM_MAGIC)  )   strcat( buf, "(Magical) "   );
    if ( !glowsee && IS_OBJ_STAT(obj, ITEM_GLOW) )   strcat( buf, "(Glowing) "   );
    if ( IS_OBJ_STAT(obj, ITEM_HUM)       )   strcat( buf, "(Humming) "   );
    if ( IS_OBJ_STAT(obj, ITEM_HIDDEN)	  )   strcat( buf, "(Hidden) "	  );
    if ( IS_OBJ_STAT(obj, ITEM_BURIED)	  )   strcat( buf, "(Buried) "	  );
    if ( IS_IMMORTAL(ch)
	 && IS_OBJ_STAT(obj, ITEM_PROTOTYPE) ) strcat( buf, "(PROTO) "	  );
    if ( ( IS_AFFECTED(ch, AFF_DETECTTRAPS) ||  xIS_SET(ch->act, PLR_HOLYLIGHT) )
	 && is_trapped(obj)   )   strcat( buf, "(Trap) "  );

    if ( fShort )
    {
	if ( glowsee && !IS_IMMORTAL(ch) )
	    strcat( buf, "the faint glow of something" );
	else
	if ( obj->short_descr )
	    strcat( buf, obj->short_descr );
    }
    else
    {
	if ( glowsee )
	    strcat( buf, "You see the faint glow of something nearby." );
	else
	if ( obj->description )
	    strcat( buf, obj->description );
    }

    return buf;
}


/*
 * Some increasingly freaky hallucinated objects		-Thoric
 * (Hats off to Albert Hoffman's "problem child")
 */
char *hallucinated_object( int ms, bool fShort )
{
    int sms = URANGE( 1, (ms+10)/5, 20 );

    if ( fShort )
    switch( number_range( 6-URANGE(1,sms/2,5), sms ) )
    {
	case  1: return "a sword";
	case  2: return "a stick";
	case  3: return "something shiny";
	case  4: return "something";
	case  5: return "something interesting";
	case  6: return "something colorful";
	case  7: return "something that looks cool";
	case  8: return "a nifty thing";
	case  9: return "a cloak of flowing colors";
	case 10: return "a mystical flaming sword";
	case 11: return "a swarm of insects";
	case 12: return "a deathbane";
	case 13: return "a figment of your imagination";
	case 14: return "your gravestone";
	case 15: return "the long lost boots of Ranger Thoric";
	case 16: return "a glowing tome of arcane knowledge";
	case 17: return "a long sought secret";
	case 18: return "the meaning of it all";
	case 19: return "the answer";
	case 20: return "the key to life, the universe and everything";
    }
    switch( number_range( 6-URANGE(1,sms/2,5), sms ) )
    {
	case  1: return "A nice looking sword catches your eye.";
	case  2: return "The ground is covered in small sticks.";
	case  3: return "Something shiny catches your eye.";
	case  4: return "Something catches your attention.";
	case  5: return "Something interesting catches your eye.";
	case  6: return "Something colorful flows by.";
	case  7: return "Something that looks cool calls out to you.";
	case  8: return "A nifty thing of great importance stands here.";
	case  9: return "A cloak of flowing colors asks you to wear it.";
	case 10: return "A mystical flaming sword awaits your grasp.";
	case 11: return "A swarm of insects buzzes in your face!";
	case 12: return "The extremely rare Deathbane lies at your feet.";
	case 13: return "A figment of your imagination is at your command.";
	case 14: return "You notice a gravestone here... upon closer examination, it reads your name.";
	case 15: return "The long lost boots of Ranger Thoric lie off to the side.";
	case 16: return "A glowing tome of arcane knowledge hovers in the air before you.";
	case 17: return "A long sought secret of all mankind is now clear to you.";
	case 18: return "The meaning of it all, so simple, so clear... of course!";
	case 19: return "The answer.  One.  It's always been One.";
	case 20: return "The key to life, the universe and everything awaits your hand.";
    }
    return "Whoa!!!";
}


/* This is the punct snippet from Desden el Chaman Tibetano - Nov 1998
   Email: jlalbatros@mx2.redestb.es
*  Memory leak reported by robet@wirehead.com - fixed --Shaddai
*/
char *num_punct(int foo)
{
    int index, index_new, rest;
    char buf[16];
    static char buf_new[16];

    sprintf(buf,"%d",foo);
    rest = strlen(buf)%3;

    for (index=index_new=0;index<strlen(buf);index++,index_new++)
    {
	if (index!=0 && (index-rest)%3==0 )
	{
	    buf_new[index_new]=',';
	    index_new++;
	    buf_new[index_new]=buf[index];
        }
        else
	    buf_new[index_new] = buf[index];
    }
    buf_new[index_new]='\0';
    return buf_new;
}


/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
    char **prgpstrShow;
    int *prgnShow;
    int *pitShow;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count, offcount, tmp, ms, cnt;
    bool fCombine;

    if ( !ch->desc )
	return;

    /*
     * if there's no list... then don't do all this crap!  -Thoric
     */
    if ( !list )
    {
    	if ( fShowNothing )
    	{
	   if ( IS_NPC(ch) || xIS_SET(ch->act, PLR_COMBINE) )
	      send_to_char( "     ", ch );
	   set_char_color( AT_OBJECT, ch );
	   send_to_char( "Nothing.\n\r", ch );
	}
	return;
    }
    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( obj = list; obj; obj = obj->next_content )
	count++;

    ms  = (ch->mental_state ? ch->mental_state : 1)
	* (IS_NPC(ch) ? 1 : (ch->pcdata->condition[COND_DRUNK] ? (ch->pcdata->condition[COND_DRUNK]/12) : 1));

    /*
     * If not mentally stable...
     */
    if ( abs(ms) > 40 )
    {
	offcount = URANGE( -(count), (count * ms) / 100, count*2 );
	if ( offcount < 0 )
	  offcount += number_range(0, abs(offcount));
	else
	if ( offcount > 0 )
	  offcount -= number_range(0, offcount);
    }
    else
	offcount = 0;

    if ( count + offcount <= 0 )
    {
    	if ( fShowNothing )
    	{
	   if ( IS_NPC(ch) || xIS_SET(ch->act, PLR_COMBINE) )
	      send_to_char( "     ", ch );
	   set_char_color( AT_OBJECT, ch );
	   send_to_char( "Nothing.\n\r", ch );
	}
	return;
    }

    CREATE( prgpstrShow,	char*,	count + ((offcount > 0) ? offcount : 0) );
    CREATE( prgnShow,		int,	count + ((offcount > 0) ? offcount : 0) );
    CREATE( pitShow,		int,	count + ((offcount > 0) ? offcount : 0) );
    nShow	= 0;
    tmp		= (offcount > 0) ? offcount : 0;
    cnt		= 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj; obj = obj->next_content )
    {
	if ( offcount < 0 && ++cnt > (count + offcount) )
	    break;
	if ( tmp > 0 && number_bits(1) == 0 )
	{
	    prgpstrShow [nShow] = str_dup( hallucinated_object(ms, fShort) );
	    prgnShow	[nShow] = 1;
	    pitShow	[nShow] = number_range( ITEM_LIGHT, ITEM_BOOK );
	    nShow++;
	    --tmp;
	}
	if ( obj->wear_loc == WEAR_NONE
	&& can_see_obj( ch, obj )
	&& (obj->item_type != ITEM_TRAP || IS_AFFECTED(ch, AFF_DETECTTRAPS) ) )
	{
	    pstrShow = format_obj_to_char( obj, ch, fShort );
	    fCombine = FALSE;

	    if ( IS_NPC(ch) || xIS_SET(ch->act, PLR_COMBINE) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow] += obj->count;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    pitShow[nShow] = obj->item_type;
	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = obj->count;
		nShow++;
	    }
	}
    }
    if ( tmp > 0 )
    {
	int x;
	for ( x = 0; x < tmp; x++ )
	{
	    prgpstrShow [nShow] = str_dup( hallucinated_object(ms, fShort) );
	    prgnShow	[nShow] = 1;
	    pitShow	[nShow] = number_range( ITEM_LIGHT, ITEM_BOOK );
	    nShow++;
	}
    }

    /*
     * Output the formatted list.		-Color support by Thoric
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	switch(pitShow[iShow]) {
	default:
	  set_char_color( AT_OBJECT, ch );
	  break;
	case ITEM_BLOOD:
	  set_char_color( AT_BLOOD, ch );
	  break;
	case ITEM_CORPSE_PC:
	case ITEM_CORPSE_NPC:
	  set_char_color( AT_ORANGE, ch );
	  break;
	case ITEM_MONEY:
	case ITEM_TREASURE:
	  set_char_color( AT_YELLOW, ch );
	  break;
	case ITEM_COOK:
	case ITEM_FOOD:
	  set_char_color( AT_HUNGRY, ch );
	  break;
	case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
	case ITEM_PUDDLE:
	  set_char_color( AT_THIRSTY, ch );
	  break;
	case ITEM_FIRE:
	  set_char_color( AT_FIRE, ch );
	  break;
	case ITEM_SCROLL:
	case ITEM_WAND:
	case ITEM_STAFF:
	  set_char_color( AT_MAGIC, ch );
	  break;
	}
	if ( fShowNothing )
	    send_to_char( "     ", ch );
	send_to_char( prgpstrShow[iShow], ch );
/*	if ( IS_NPC(ch) || xIS_SET(ch->act, PLR_COMBINE) ) */
	{
	    if ( prgnShow[iShow] != 1 )
		ch_printf( ch, " (%d)", prgnShow[iShow] );
	}

	send_to_char( "\n\r", ch );
	DISPOSE( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC(ch) || xIS_SET(ch->act, PLR_COMBINE) )
	    send_to_char( "     ", ch );
	set_char_color( AT_OBJECT, ch );
	send_to_char( "Nothing.\n\r", ch );
    }

    /*
     * Clean up.
     */
    DISPOSE( prgpstrShow );
    DISPOSE( prgnShow	 );
    DISPOSE( pitShow	 );
    return;
}


/*
 * Show fancy descriptions for certain spell affects		-Thoric
 */
void show_visible_affects_to_char( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    char name[MAX_STRING_LENGTH];

    if ( IS_NPC( victim ) )
      strcpy( name, victim->short_descr );
    else
      strcpy( name, victim->name);
    name[0] = toupper(name[0]);

    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
    {
	set_char_color( AT_WHITE, ch );
        if ( IS_GOOD(victim) )
            ch_printf( ch, "%s glows with an aura of divine radiance.\n\r", name );
        else if ( IS_EVIL(victim) )
            ch_printf( ch, "%s shimmers beneath an aura of dark energy.\n\r", name );
        else
            ch_printf( ch, "%s is shrouded in flowing shadow and light.\n\r", name );
    }
    if ( IS_AFFECTED(victim, AFF_FIRESHIELD) )
    {
        set_char_color( AT_FIRE, ch );
        ch_printf( ch, "%s is engulfed within a blaze of mystical flame.\n\r", name );
    }
    if ( IS_AFFECTED(victim, AFF_SHOCKSHIELD) )
    {
        set_char_color( AT_BLUE, ch );
	ch_printf( ch, "%s is surrounded by cascading torrents of energy.\n\r", name );
    }
    if ( IS_AFFECTED(victim, AFF_ACIDMIST) )
    {
	set_char_color( AT_GREEN, ch );
	ch_printf( ch, "%s is visible through a cloud of churning mist.\n\r", name );
    }
/*Scryn 8/13*/
    if ( IS_AFFECTED(victim, AFF_ICESHIELD) )
    {
        set_char_color( AT_LBLUE, ch );
        ch_printf( ch, "%s is ensphered by shards of glistening ice.\n\r", name );
    }
    if ( IS_AFFECTED(victim, AFF_CHARM)       )
    {
	set_char_color( AT_MAGIC, ch );
	ch_printf( ch, "%s wanders in a dazed, zombie-like state.\n\r", name );
    }
    if ( !IS_NPC(victim) && !victim->desc
    &&    victim->switched && IS_AFFECTED(victim->switched, AFF_POSSESS) )
    {
	set_char_color( AT_MAGIC, ch );
	strcpy( buf, PERS( victim, ch ) );
	strcat( buf, " appears to be in a deep trance...\n\r" );
    }
}

void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    TIMER *timer;
    
    buf[0] = '\0';

    set_char_color( AT_PERSON, ch );
    if ( !IS_NPC(victim) && !victim->desc )
    {
	if ( !victim->switched )         send_to_char_color( "&P[(Link Dead)] ", ch );
	else if ( !IS_AFFECTED(victim, AFF_POSSESS) )
						strcat( buf, "(Switched) " );
    }
    if ( IS_NPC(victim) && IS_AFFECTED(victim, AFF_POSSESS) && IS_IMMORTAL(ch)
	 && victim->desc )
    {
			sprintf( buf1, "(%s)",victim->desc->original->name );
			strcat( buf, buf1 );
    }
    if ( !IS_NPC(victim)
    &&   xIS_SET(victim->act, PLR_AFK) )	strcat( buf, "[AFK] ");        

    if ( (!IS_NPC(victim) && xIS_SET(victim->act, PLR_WIZINVIS))
      || (IS_NPC(victim) && xIS_SET(victim->act, ACT_MOBINVIS)) ) 
    {
        if (!IS_NPC(victim))
	sprintf( buf1,"(Invis %d) ", victim->pcdata->wizinvis );
        else sprintf( buf1,"(Mobinvis %d) ", victim->mobinvis);
	strcat( buf, buf1 );
    }
 

    if ( !IS_NPC( victim ) )
    {
	int ilevel = victim->level;
	switch(ilevel)
	{
	  case MAX_LEVEL - 0 :
	    send_to_char_color( "&G&r(&wSupreme Entity&G&r)&P ", ch );
	    break;
  	  case MAX_LEVEL - 1 :
	    send_to_char_color( "&G&r(&wInfinite&G&r)&P ", ch );
	    break;
	  case MAX_LEVEL - 2 :
	    send_to_char_color( "&G&r(&wEternal&G&r)&P ", ch );
	    break;
	  case MAX_LEVEL - 3 :
	    send_to_char_color( "&G&r(&wAncient&G&r)&P ", ch );
	    break;
	  case MAX_LEVEL - 4 :
	    if ( victim->sex == 2 )
		send_to_char_color( "&G&r(&wExalted Goddess&G&r)&P ", ch );
	    else
		send_to_char_color( "&G&r(&wExalted God&G&r)&P ", ch );
	    break;
	  case MAX_LEVEL - 5 :
            if ( victim->sex == 2 )
                send_to_char_color( "&G(&wAscendant Goddess&G)&P ", ch );
            else
		send_to_char_color( "&G(&wAscendant God&G)&P ", ch );
	    break;
	  case MAX_LEVEL - 6 :
            if ( victim->sex == 2 )
                send_to_char_color( "&G(&wGreater Goddess&G)&P ", ch );
            else
		send_to_char_color( "&G(&wGreater God&G)&P ", ch );
	    break;
	  case MAX_LEVEL - 7 :
            if ( victim->sex == 2 )
                send_to_char_color( "&G(&wGoddess&G)&P ", ch );
            else
		send_to_char_color( "&G(&wGod&G)&P ", ch );
	    break;
	  case MAX_LEVEL - 8 :
            if ( victim->sex == 2 )
                send_to_char_color( "&G(&wLesser Goddess&G)&P ", ch );
            else
		send_to_char_color( "&G(&wLesser God&G)&P ", ch );
            break;
	  case MAX_LEVEL - 9 :
            send_to_char_color( "&G(&wImmortal&G)&P ", ch );
            break;
	  case MAX_LEVEL -10 :
            if ( victim->sex == 2 )
                send_to_char_color( "&G(&wDemi Goddess&G)&P ", ch );
            else
		send_to_char_color( "&G(&wDemi God&G)&P ", ch );
            break;
	  case MAX_LEVEL -11 :
            send_to_char_color( "&G(&WSavior&G)&P ", ch );
            break;
	  case MAX_LEVEL -12 :
            send_to_char_color( "&G(&WCreator&G)&P ", ch );
            break;
	  case MAX_LEVEL -13 :
            send_to_char_color( "&G(&WAcolyte&G)&P ", ch );
            break;
	  case MAX_LEVEL -14 :
	    send_to_char_color( "&G&W(Neophyte)&P ", ch );
	    break;
	  default:
	    break;
	}

/*
	if ( !IS_IMMORTAL( victim ) && !IS_NPC( victim ) && IS_IMMORTAL( ch ) )
	{
	    if ( xIS_SET( victim->act, PLR_SILENCE )
	    ||   xIS_SET( victim->act, PLR_FREEZE )
	    ||    IS_SET( victim->pcdata->flags, PCFLAG_NOBIO )
	    ||    IS_SET( victim->pcdata->flags, PCFLAG_NODESC )
	    ||    IS_SET( victim->pcdata->flags, PCFLAG_NOBECKON )
	    ||   xIS_SET( victim->act, PLR_NO_EMOTE ) )
		ch_printf_color( ch, "&G&P(Mulct) " );
	}
*/
	if ( !IS_IMMORTAL( victim )
	&&    ch->level < 26
	&&    victim->pcdata->council
	&&   !str_cmp( victim->pcdata->council->name, "Newbie Council" ) )
					ch_printf_color( ch, "&P&W<&PNew Player Advisor&W>&P " );
	if ( victim->pcdata->clan
        &&   IS_SET( victim->pcdata->flags, PCFLAG_DEADLY )
	&&   victim->pcdata->clan->badge
	&& ( victim->pcdata->clan->clan_type != CLAN_ORDER
	&&   victim->pcdata->clan->clan_type != CLAN_GUILD ) )
                                        ch_printf_color( ch, "&P%s ", victim->pcdata->clan->badge );
	else if ( CAN_PKILL( victim ) && victim->level < 51 )
      				         send_to_char_color( "&P(&wUnclanned&P) ", ch );
    }

    set_char_color( AT_PERSON, ch );

    if ( IS_AFFECTED(victim, AFF_INVISIBLE)   ) strcat( buf, "(Invis) "      );
    if ( IS_AFFECTED(victim, AFF_HIDE)        ) strcat( buf, "(Hide) "       );
    if ( IS_AFFECTED(victim, AFF_PASS_DOOR)   ) strcat( buf, "(Translucent) ");
    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) strcat( buf, "(Pink Aura) "  );
    if ( IS_EVIL(victim)
    &&   (  IS_AFFECTED(ch, AFF_DETECT_EVIL) || ch->class==CLASS_PALADIN)     ) strcat( buf, "(Red Aura) "   );
    if ( IS_NEUTRAL(victim)
    &&   ch->class==CLASS_PALADIN  ) strcat( buf, "(Grey Aura) "   );
    if ( IS_GOOD(victim)
    &&   ch->class==CLASS_PALADIN  ) strcat( buf, "(White Aura) "   );


    if ( IS_AFFECTED(victim, AFF_BERSERK)     ) strcat( buf, "(Wild-eyed) "  );
    if ( IS_AFFECTED(victim, AFF_GRAPPLE) )     strcat( buf, "(Grappling) "  );
    if ( !IS_NPC(victim) && xIS_SET(victim->act, PLR_ATTACKER ) )
						strcat( buf, "(ATTACKER) "   );
    if ( !IS_NPC(victim) && xIS_SET(victim->act, PLR_KILLER ) )
						strcat( buf, "(KILLER) "     );
    if ( !IS_NPC(victim) && xIS_SET(victim->act, PLR_THIEF  ) )
						strcat( buf, "(THIEF) "      );
    if ( !IS_NPC(victim) && xIS_SET(victim->act, PLR_LITTERBUG  ) )
						strcat( buf, "(LITTERBUG) "  );
    if ( IS_NPC(victim) && IS_IMMORTAL(ch)
	 && xIS_SET(victim->act, ACT_PROTOTYPE) )strcat( buf, "(PROTO) "      );
    if ( IS_NPC(victim) && ch->mount && ch->mount == victim
         && ch->in_room == ch->mount->in_room ) strcat( buf, "(Mount) "      );
    if ( victim->desc && victim->desc->connected == CON_EDITING )
						strcat( buf, "(Writing) "    );
    if ( victim->morph != NULL )
      strcat (buf, "(Morphed) ");

    set_char_color( AT_PERSON, ch );
  if ((victim->position == victim->defposition && victim->long_descr[0] != '\0')
      || ( victim->morph && victim->morph->morph &&
           victim->morph->morph->defpos == victim->position ) )
    {
      if ( victim->morph != NULL )
      {
        if ( !IS_IMMORTAL(ch) )
        {
          if ( victim->morph->morph != NULL)
                strcat ( buf, victim->morph->morph->long_desc );
          else
                strcat ( buf, victim->long_descr );
        }
        else
	{
           strcat (buf, PERS(victim, ch) );
    	   if ( !IS_NPC(victim) && !xIS_SET(ch->act, PLR_BRIEF) )
		strcat( buf, victim->pcdata->title );
	   strcat( buf, ".\n\r" );
	}
      }
      else
           strcat (buf, victim->long_descr);
	send_to_char_color( buf, ch ); /* Blod color */
	show_visible_affects_to_char( victim, ch );
	return;
    }
    else {
    if ( victim->morph != NULL && victim->morph->morph != NULL &&
	 !IS_IMMORTAL( ch ) )
	strcat( buf, MORPHPERS( victim, ch ) );
    else 
    	strcat( buf, PERS( victim, ch ) );
    }

    if ( !IS_NPC(victim) && !xIS_SET(ch->act, PLR_BRIEF) )
	strcat( buf, victim->pcdata->title );

    if ( ( timer = get_timerptr( victim, TIMER_DO_FUN ) ) != NULL )
    {
       if ( timer->do_fun == do_meditate )   	strcat( buf, " is in a meditative state." ); 
	else if  (timer->do_fun == do_cast )		strcat( buf, " is here chanting.");
       else if (timer->do_fun == do_dig )	strcat( buf, " is here digging.");
	   else if (timer->do_fun == do_trance )	strcat( buf, " is in a deep trance.");
	   else if (timer->do_fun == do_search )	strcat( buf, " is searching the area for something.");
	   else if (timer->do_fun == do_detrap )	strcat( buf, " is working with the trap here.");
	   else strcat( buf, " is looking rather lost.");
	}
    else
    {
    switch ( victim->position )
    {
    case POS_DEAD:     strcat( buf, " is DEAD!!" );			break;
    case POS_MORTAL:   strcat( buf, " is mortally wounded." );		break;
    case POS_INCAP:    strcat( buf, " is incapacitated." );		break;
    case POS_STUNNED:  strcat( buf, " is lying here stunned." );	break;
    case POS_SLEEPING:
        if (ch->position == POS_SITTING
        ||  ch->position == POS_RESTING )
            strcat( buf, " is sleeping nearby." );
	else
            strcat( buf, " is deep in slumber here." );
        break;
    case POS_RESTING:
        if (ch->position == POS_RESTING)
            strcat ( buf, " is sprawled out alongside you." );
        else
	if (ch->position == POS_MOUNTED)
	    strcat ( buf, " is sprawled out at the foot of your mount." );
	else
            strcat (buf, " is sprawled out here." );
        break;
    case POS_SITTING:
        if (ch->position == POS_SITTING)
            strcat( buf, " sits here with you." );
        else
        if (ch->position == POS_RESTING)
            strcat( buf, " sits nearby as you lie around." );
        else
            strcat( buf, " sits upright here." );
        break;
    case POS_STANDING:
	if ( IS_IMMORTAL( victim ) )
	    strcat( buf, " radiates with a godly light." );
	else
        if ( ( victim->in_room->sector_type == SECT_UNDERWATER )
        && !IS_AFFECTED(victim, AFF_AQUA_BREATH) && !IS_NPC(victim) )
            strcat( buf, " is drowning here." );
	else
	if ( victim->in_room->sector_type == SECT_UNDERWATER )
            strcat( buf, " is here in the water." );
	else
	if ( ( victim->in_room->sector_type == SECT_OCEANFLOOR )
	&& !IS_AFFECTED(victim, AFF_AQUA_BREATH) && !IS_NPC(victim) )
	    strcat( buf, " is drowning here." );
	else
	if ( victim->in_room->sector_type == SECT_OCEANFLOOR )
	    strcat( buf, " is standing here in the water." );
	else
	if ( IS_AFFECTED(victim, AFF_FLOATING)
        || IS_AFFECTED(victim, AFF_FLYING) ) 
        if (victim->stance > STANCE_NONE)
          {
            strcat (buf, " is hovering here in a ");
            strcat (buf, get_stance_name (victim->stance));
            strcat (buf, " stance.");
          }
        else
          strcat (buf, " is hovering here.");
      else if (victim->stance > STANCE_NONE)
        {
          strcat (buf, " is standing here in a ");
          strcat (buf, get_stance_name (victim->stance));
          strcat (buf, " stance.");
        }
      else
        strcat (buf, " is standing here.");
        break;
    case POS_SHOVE:    strcat( buf, " is being shoved around." );	break;
    case POS_DRAG:     strcat( buf, " is being dragged around." );	break;
    case POS_MOUNTED:
	strcat( buf, " is here, upon " );
	if ( !victim->mount )
	    strcat( buf, "thin air???" );
	else
	if ( victim->mount == ch )
	    strcat( buf, "your back." );
	else
	if ( victim->in_room == victim->mount->in_room )
	{
	    strcat( buf, PERS( victim->mount, ch ) );
	    strcat( buf, "." );
	}
	else
	    strcat( buf, "someone who left??" );
	break;
    case POS_FIGHTING:
    case POS_EVASIVE:
    case POS_DEFENSIVE:
    case POS_AGGRESSIVE:
    case POS_BERSERK:
	strcat( buf, " is here, fighting " );
	if ( !victim->fighting ) 
        {
	    strcat( buf, "thin air???" );

            /* some bug somewhere.... kinda hackey fix -h */
            if(! victim->mount)
               victim->position = POS_STANDING;
            else
               victim->position = POS_MOUNTED;
        }
	else if ( who_fighting( victim ) == ch )
	    strcat( buf, "YOU!" );
	else if ( victim->in_room == victim->fighting->who->in_room )
	{
	    strcat( buf, PERS( victim->fighting->who, ch ) );
	    strcat( buf, "." );
	}
	else
	    strcat( buf, "someone who left??" );
	break;
    }
	}
    strcat( buf, "\n\r" );
    buf[0] = UPPER(buf[0]);
    send_to_char_color( buf, ch ); /* Blod color */
    show_visible_affects_to_char( victim, ch );
    return;
}



void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    int iWear;
    bool found;

    if ( can_see( victim, ch ) && !IS_NPC( ch ) && !xIS_SET( ch->act, PLR_WIZINVIS ) )
    {
      act( AT_ACTION, "$n looks at you.", ch, NULL, victim, TO_VICT    );
      if ( victim != ch )
	act( AT_ACTION, "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
      else
        act( AT_ACTION, "$n looks at $mself.", ch, NULL, victim, TO_NOTVICT );
    }

    if ( victim->description[0] != '\0' )
    {
      if ( victim->morph != NULL && victim->morph->morph != NULL)
        send_to_char ( victim->morph->morph->description , ch );
      else
        send_to_char (victim->description, ch);
    }
    else
    {
        if ( victim->morph != NULL && victim->morph->morph != NULL)
           send_to_char ( victim->morph->morph->description , ch );
	else if ( IS_NPC( victim ) )
	  act( AT_PLAIN, "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
	else if ( ch != victim )
	  act( AT_PLAIN, "$E isn't much to look at...", ch, NULL, victim, TO_CHAR );
	else
	  act( AT_PLAIN, "You're not much to look at...", ch, NULL, NULL, TO_CHAR );
    }

    /* Show stance they are in if someone looks at them.  SHADDAI */
    if (victim->stance > STANCE_NORMAL)
    {
      ch_printf (ch, "\n\r%s is in a %s fighting stance.\n\r",
                 capitalize (PERS (victim, ch)),
                 get_stance_name (victim->stance));
    }

    show_race_line( ch, victim );
    show_condition( ch, victim );

    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
	&&   can_see_obj( ch, obj ) )
	{
	    if ( !found )
	    {
		send_to_char( "\n\r", ch );
		if ( victim != ch )
		  act( AT_PLAIN, "$N is using:", ch, NULL, victim, TO_CHAR );
		else
		  act( AT_PLAIN, "You are using:", ch, NULL, NULL, TO_CHAR );
		found = TRUE;
	    }
            if( (!IS_NPC(victim)) && (victim->race>0) && (victim->race<MAX_PC_RACE))
		send_to_char(race_table[victim->race]->where_name[iWear], ch);
	    else
	        send_to_char( where_name[iWear], ch );
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
    }

    /*
     * Crash fix here by Thoric
     */
    if ( IS_NPC(ch) || victim == ch )
      return;

    if ( IS_IMMORTAL( ch ) )
    {
      if ( IS_NPC( victim ) )
	ch_printf( ch, "\n\rMobile #%d '%s' ",
	    victim->pIndexData->vnum,
	    victim->name );
	else
	  ch_printf( ch, "\n\r%s ", victim->name );
      ch_printf( ch, "is a level %d %s %s.\n\r",
	victim->level,
	IS_NPC(victim)?victim->race<MAX_NPC_RACE&&victim->race>=0?
	npc_race[victim->race]:"unknown":victim->race<MAX_PC_RACE&&
	race_table[victim->race]->race_name&&
	race_table[victim->race]->race_name[0] != '\0'?
	race_table[victim->race]->race_name:"unknown",
	IS_NPC(victim)?victim->class<MAX_NPC_CLASS&&victim->class>=0?
	npc_class[victim->class] : "unknown":victim->class<MAX_PC_CLASS&&
	class_table[victim->class]->who_name&&
	class_table[victim->class]->who_name[0] != '\0'?
	class_table[victim->class]->who_name:"unknown");
/* Fix so it shows what is in class table
	victim->race<MAX_NPC_RACE&&victim->race>=0?npc_race[victim->race] : "unknown",
	victim->class<MAX_NPC_CLASS&&victim->class>=0?npc_class[victim->class] : "unknown" );
*/
    }

    if ( number_percent( ) < LEARNED(ch, gsn_peek) )
    {
	ch_printf( ch, "\n\rYou peek at %s inventory:\n\r",
	  victim->sex == 1 ? "his" : victim->sex == 2 ? "her" : "its" );
	show_list_to_char( victim->first_carrying, ch, TRUE, TRUE );
	learn_from_success( ch, gsn_peek );
    }
    else
      if ( ch->pcdata->learned[gsn_peek] > 0 )
        learn_from_failure( ch, gsn_peek );

    return;
}


void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
    CHAR_DATA *rch;

    for ( rch = list; rch; rch = rch->next_in_room )
    {
	if ( rch == ch )
	    continue;

	if ( can_see( ch, rch ) )
	{
	    show_char_to_char_0( rch, ch );
	}
	else if ( room_is_dark( ch->in_room )
	&&        IS_AFFECTED(rch, AFF_INFRARED ) 
        && !(!IS_NPC(rch) && IS_IMMORTAL(rch)) )
	{
	    set_char_color( AT_BLOOD, ch );
	    send_to_char( "The red form of a living creature is here.\n\r", ch );
	}
    }

    return;
}



bool check_blind( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && xIS_SET(ch->act, PLR_HOLYLIGHT) )
	return TRUE;
	
    if ( IS_AFFECTED(ch, AFF_TRUESIGHT) )
      return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    {
	send_to_char( "You can't see a thing!\n\r", ch );
	return FALSE;
    }

    return TRUE;
}

/*
 * Returns classical DIKU door direction based on text in arg	-Thoric
 */
int get_door( char *arg )
{
    int door;

	 if ( !str_cmp( arg, "n"  ) || !str_cmp( arg, "north"	  ) ) door = 0;
    else if ( !str_cmp( arg, "e"  ) || !str_cmp( arg, "east"	  ) ) door = 1;
    else if ( !str_cmp( arg, "s"  ) || !str_cmp( arg, "south"	  ) ) door = 2;
    else if ( !str_cmp( arg, "w"  ) || !str_cmp( arg, "west"	  ) ) door = 3;
    else if ( !str_cmp( arg, "u"  ) || !str_cmp( arg, "up"	  ) ) door = 4;
    else if ( !str_cmp( arg, "d"  ) || !str_cmp( arg, "down"	  ) ) door = 5;
    else if ( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast" ) ) door = 6;
    else if ( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest" ) ) door = 7;
    else if ( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast" ) ) door = 8;
    else if ( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest" ) ) door = 9;
    else door = -1;
    return door;
}

void do_look( CHAR_DATA *ch, char *argument )
{
    char arg  [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char dir_n[50];
    char dir_e[50];
    char dir_s[50];
    char dir_w[50];
    char dir_u[50];
    char dir_d[50];
    char dir_ne[50];
    char dir_nw[50];
    char dir_se[50];
    char dir_sw[50];
    char dir_sm[50];
    char *exitcolor;
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *original;
    char *pdesc;
    bool doexaprog; 
    sh_int door;
    int number, cnt;
    int len = colorlen( ch->in_room->name, 50);

    if ( !ch->desc )
	return;

    if ( ch->position < POS_SLEEPING )
    {
	send_to_char( "You can't see anything but stars!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
	return;
    }

    if ( !check_blind( ch ) )
	return;

    if ( !IS_NPC(ch)
    &&   !xIS_SET(ch->act, PLR_HOLYLIGHT)
    &&   !IS_AFFECTED(ch, AFF_TRUESIGHT)
    &&   room_is_dark( ch->in_room ) )
    {
	set_char_color( AT_DGREY, ch );
	send_to_char( "It is pitch black ... \n\r", ch );
	show_char_to_char( ch->in_room->first_person, ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    doexaprog = str_cmp( "noprog", arg2 ) && str_cmp( "noprog", arg3 );

    if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
    {
	switch ( ch->inter_page )    /* rmenu */
	{ 
	   case ROOM_PAGE_A : do_rmenu(ch,"a");
			      break;
	   case ROOM_PAGE_B : do_rmenu(ch,"b");
			      break;
	   case ROOM_PAGE_C : do_rmenu(ch,"c");
			      break;
        }
	if( xIS_SET( ch->act, PLR_COMPASS ) )
	{
		strcpy( dir_n, "&z-" );
		strcpy( dir_e, "&z-" );
		strcpy( dir_s, "&z-" );
		strcpy( dir_w, "&z-" );
		strcpy( dir_u, "&z-" );
		strcpy( dir_d, "&z-" );
		strcpy( dir_ne, "&z \\" );
		strcpy( dir_nw, "&z/ " );
		strcpy( dir_se, "&z /" );
		strcpy( dir_sw, "&z\\ " );
		strcpy( dir_sm, "&z-" );

		for( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
		{
			if( ( pexit->to_room
			&& !IS_SET (pexit->exit_info, EX_HIDDEN)
			&& !IS_SET (pexit->exit_info, EX_SECRET)) )

/*			|| (IS_SET (pexit->exit_info, EX_SECRET)
			&& !IS_SET (pexit->exit_info, EX_CLOSED)))
*/
			{
				exitcolor = "&Y&G";
				if( IS_SET( pexit->exit_info, EX_WINDOW ) )
					exitcolor = "&C";
				if( IS_SET( pexit->exit_info, EX_SECRET ) )
					exitcolor = "&b";
				if( IS_SET( pexit->exit_info, EX_CLOSED ) )
					exitcolor = "&g";
				if( IS_SET( pexit->exit_info, EX_LOCKED ) )
					exitcolor = "&r";
				if( pexit->vdir == DIR_NORTH )
					sprintf( dir_n, "%sN", exitcolor );
				if( pexit->vdir == DIR_EAST )
					sprintf( dir_e, "%sE", exitcolor );
				if( pexit->vdir == DIR_SOUTH )
					sprintf( dir_s, "%sS", exitcolor );
				if( pexit->vdir == DIR_WEST )
					sprintf( dir_w, "%sW", exitcolor );
				if( pexit->vdir == DIR_UP )
					sprintf( dir_u, "%sU", exitcolor );
				if( pexit->vdir == DIR_DOWN )
					sprintf( dir_d, "%sD", exitcolor );
				if( pexit->vdir == DIR_NORTHEAST )
					sprintf( dir_ne, "%sNE", exitcolor );
				if( pexit->vdir == DIR_NORTHWEST )
					sprintf( dir_nw, "%sNW", exitcolor );
				if( pexit->vdir == DIR_SOUTHEAST )
					sprintf( dir_se, "%sSE", exitcolor );
				if( pexit->vdir == DIR_SOUTHWEST )
					sprintf( dir_sw, "%sSW", exitcolor );
			}
		}

		send_to_char( "\n\r", ch );
		set_char_color( AT_RMNAME, ch );
/*		ch_printf( ch, "%-50.50s", ch->in_room->name ); */
	        if ( xIS_SET( ch->in_room->room_flags, ROOM_COLOR) )
		  ch_printf_color( ch, "%-*.*s", len, len,
			 ch->in_room->name );
		else
		  ch_printf( ch, "%-50.50s", ch->in_room->name );
		strcat( dir_nw, "  " );
		ch_printf_color( ch, "        %s ", dir_nw );
		strcat( dir_n, "  " );
		ch_printf_color( ch, "  %s", dir_n );
		ch_printf_color( ch, "   %s\n\r", dir_ne );
		send_to_char_color( "&z------------------------------------------------------- ", ch );
		strcat( dir_w, "" );
		ch_printf_color( ch, "   %s", dir_w );
		send_to_char_color( "&z -", ch );
		strcat( dir_u, "&z-&g(&Y&G+&g)&z" );
		ch_printf_color( ch, "%s", dir_u );
		strcat( dir_d, "&z-" );
		send_to_char_color( "&z-", ch );
		ch_printf_color( ch, "%s", dir_d );
		send_to_char_color( "&z ", ch );
		ch_printf_color( ch, "%s\n\r", dir_e );
		send_to_char( "                                                        ", ch );
		strcat( dir_sw, "  " );
		ch_printf_color( ch, "  %s ", dir_sw );
		strcat( dir_s, "  " );
		ch_printf_color( ch, "  %s", dir_s );
		ch_printf_color( ch, "   %s", dir_se );
		send_to_char( "\n\r", ch );

	}
	else
	{
		set_char_color( AT_RMNAME, ch );
		if ( xIS_SET( ch->in_room->room_flags, ROOM_COLOR ) )
			send_to_char_color( ch->in_room->name, ch );
		else
			send_to_char( ch->in_room->name, ch );
		send_to_char( "\n\r", ch );		
	}

	set_char_color( AT_RMDESC, ch );
	if ( arg1[0] == '\0'
	|| ( !IS_NPC(ch) && !xIS_SET(ch->act, PLR_BRIEF) ) )
	{
	    if ( xIS_SET(ch->in_room->room_flags, ROOM_COLOR) )
		send_to_char_color( ch->in_room->description, ch );
	    else
		send_to_char( ch->in_room->description, ch );
	}

/*
	if ( !IS_NPC(ch) && xIS_SET(ch->act, PLR_AUTOMAP) )
	{
	    if(ch->in_room->map != NULL)
	    {
	       do_lookmap(ch, NULL);
	    }
	}
*/

	/* Testing automapper snippet - 2002 */
	if ( !IS_NPC(ch) && xIS_SET(ch->act, PLR_AUTOMAP) )
	{
	    ch_printf_color( ch, "&w_____________________________________________________________________________\n\r" );
	    if ( IS_IMMORTAL( ch ) )
		do_lookmap(ch, "auto");
            ch_printf_color( ch, "&w_____________________________________________________________________________\n\r" );

	}


	if ( !IS_NPC(ch) && xIS_SET(ch->act, PLR_AUTOEXIT) )
	    do_exits( ch, "auto" );


	show_list_to_char( ch->in_room->first_content, ch, FALSE, FALSE );
	show_char_to_char( ch->in_room->first_person,  ch );
	return;
    }

    if ( !str_cmp( arg1, "sky" ) )
    {
      if ( !IS_OUTSIDE(ch) )
      {
        send_to_char( "You can't see the sky indoors.\n\r", ch );
        return;
      }
      else
      {
        look_sky(ch);
        return;
      }
    }

    if ( !str_cmp( arg1, "under" ) )
    {
	int count;

	/* 'look under' */
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Look beneath what?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not see that here.\n\r", ch );
	    return;
	}
	if ( !CAN_WEAR( obj, ITEM_TAKE ) && ch->level < sysdata.level_getobjnotake )
	{
	    send_to_char( "You can't seem to get a grip on it.\n\r", ch );
	    return;
	}
	if ( ch->carry_weight + obj->weight > can_carry_w( ch ) )
	{
	    send_to_char( "It's too heavy for you to look under.\n\r", ch );
	    return;
	}
	count = obj->count;
	obj->count = 1;
	act( AT_PLAIN, "You lift $p and look beneath it:", ch, obj, NULL, TO_CHAR );
	act( AT_PLAIN, "$n lifts $p and looks beneath it:", ch, obj, NULL, TO_ROOM );
	obj->count = count;
	if ( IS_OBJ_STAT( obj, ITEM_COVERING ) )
	   show_list_to_char( obj->first_content, ch, TRUE, TRUE );
	else
	   send_to_char( "Nothing.\n\r", ch );
	if ( doexaprog ) oprog_examine_trigger( ch, obj );
	return;
    }

    if ( !str_cmp( arg1, "i" ) || !str_cmp( arg1, "in" ) )
    {
	int count;

	/* 'look in' */
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Look in what?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not see that here.\n\r", ch );
	    return;
	}

	switch ( obj->item_type )
	{
	default:
	    send_to_char( "That is not a container.\n\r", ch );
	    break;

	case ITEM_DRINK_CON:
	    if ( obj->value[1] <= 0 )
	    {
		send_to_char( "It is empty.\n\r", ch );
	        if ( doexaprog ) oprog_examine_trigger( ch, obj );
		break;
	    }

	    ch_printf( ch, "It's %s full of a %s liquid.\n\r",
		obj->value[1] <     obj->value[0] / 4
		    ? "less than" :
		obj->value[1] < 3 * obj->value[0] / 4
		    ? "about"     : "more than",
		( obj->value[2] >= LIQ_MAX ? "clear" : liq_table[obj->value[2]].liq_color )
		 );

	    if ( doexaprog ) oprog_examine_trigger( ch, obj );
	    break;

	case ITEM_PORTAL:
	    for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
	    {
		if ( pexit->vdir == DIR_PORTAL
		&&   IS_SET(pexit->exit_info, EX_PORTAL) )
		{
		    if ( room_is_private( pexit->to_room )
		    &&   get_trust(ch) < sysdata.level_override_private )
		    {
			set_char_color( AT_WHITE, ch );
			send_to_char( "That room is private buster!\n\r", ch );
			return;
		    }
		    original = ch->in_room;
		    char_from_room( ch );
		    char_to_room( ch, pexit->to_room );
		    do_look( ch, "auto" );
		    char_from_room( ch );
		    char_to_room( ch, original );
		    return;
		}
	    }
	    send_to_char( "You see swirling chaos...\n\r", ch );
	    break;
	case ITEM_CONTAINER:
	case ITEM_QUIVER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		send_to_char( "It is closed.\n\r", ch );
		break;
	    }

	case ITEM_KEYRING:
	    count = obj->count;
	    obj->count = 1;
	    if ( obj->item_type == ITEM_CONTAINER )
		act( AT_PLAIN, "$p contains:", ch, obj, NULL, TO_CHAR );
	    else
		act( AT_PLAIN, "$p holds:", ch, obj, NULL, TO_CHAR );
	    obj->count = count;
	    show_list_to_char( obj->first_content, ch, TRUE, TRUE );
	    if ( doexaprog ) oprog_examine_trigger( ch, obj );
	    break;
	}
	return;
    }

    if ( (pdesc=get_extra_descr(arg1, ch->in_room->first_extradesc)) != NULL )
    {
	send_to_char_color( pdesc, ch );
	return;
    }

    door = get_door(arg1);
    if ( (pexit=find_door(ch, arg1, TRUE)) != NULL )
    {
	if ( IS_SET(pexit->exit_info, EX_CLOSED)
	&&  !IS_SET(pexit->exit_info, EX_WINDOW) )
	{
	    if ( (IS_SET(pexit->exit_info, EX_SECRET)
	    ||    IS_SET(pexit->exit_info, EX_DIG)) && door != -1 )
		send_to_char( "Nothing special there.\n\r", ch );
	    else
            {
                if ( pexit->keyword[strlen( pexit->keyword )-1] == 's'
                || ( pexit->keyword[strlen( pexit->keyword )-1] == '\''
                && pexit->keyword[strlen( pexit->keyword ) -2] == 's' ) )
                act( AT_RED, "The $d are closed.", ch, NULL, pexit->keyword, TO_CHAR );
                else
                act( AT_RED, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
            }
            return;
        }
        if ( IS_SET( pexit->exit_info, EX_BASHED ) )
        {
                if ( pexit->keyword[strlen( pexit->keyword )-1] == 's'
                || ( pexit->keyword[strlen( pexit->keyword )-1] == '\''
                && pexit->keyword[strlen( pexit->keyword ) -2] == 's' ) )
                act( AT_RED, "The $d have been bashed from their hinges.", ch, NULL, pexit->keyword, TO_CHAR );
                else
                act( AT_RED, "The $d has been bashed from its hinges.", ch, NULL, pexit->keyword, TO_CHAR );
        }                                                                                                                                       

	if ( pexit->description && pexit->description[0] != '\0' )
	    send_to_char( pexit->description, ch );
	else
	    send_to_char( "Nothing special there.\n\r", ch );

	/*
	 * Ability to look into the next room			-Thoric
	 */
	if ( pexit->to_room
	&& ( IS_AFFECTED( ch, AFF_SCRYING )
	||   ch->class == CLASS_THIEF
	||   IS_SET( pexit->exit_info, EX_xLOOK )
	||   get_trust(ch) >= LEVEL_IMMORTAL ) )
	{
	    if ( !IS_SET( pexit->exit_info, EX_xLOOK )
	    &&    get_trust( ch ) < LEVEL_IMMORTAL )
	    {
		set_char_color( AT_MAGIC, ch );
		send_to_char( "You attempt to scry...\n\r", ch );
		/*
		 * Change by Narn, Sept 96 to allow characters who don't have the
		 * scry spell to benefit from objects that are affected by scry.
		 */
		if (!IS_NPC(ch) )
		{
		    int percent = LEARNED(ch, skill_lookup("scry") );
		    if ( !percent )
		    {
			if ( ch->class == CLASS_THIEF )
			    percent = 95;
			else
			    percent = 55;	/* 95 was too good -Thoric */
		    }
 
		    if ( number_percent( ) > percent ) 
		    {
			send_to_char( "You fail.\n\r", ch );
			return;
		    }
		}
	    }
	    if ( room_is_private( pexit->to_room )
	    &&   get_trust(ch) < sysdata.level_override_private )
	    {
		set_char_color( AT_WHITE, ch );
		send_to_char( "That room is private buster!\n\r", ch );
		return;
	    }
	    original = ch->in_room;
	    char_from_room( ch );
	    char_to_room( ch, pexit->to_room );
	    do_look( ch, "auto" );
	    char_from_room( ch );
	    char_to_room( ch, original );
	}
	return;
    }
    else
    if ( door != -1 )
    {
	send_to_char( "Nothing special there.\n\r", ch );
	return;
    }

    if ( (victim = get_char_room(ch, arg1)) != NULL )
    {
	show_char_to_char_1( victim, ch );
	return;
    }


    /* finally fixed the annoying look 2.obj desc bug	-Thoric */
    number = number_argument( arg1, arg );
    for ( cnt = 0, obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    if ( (pdesc=get_extra_descr(arg, obj->first_extradesc)) != NULL )
	    {
		if ( (cnt += obj->count) < number )
		  continue;
		send_to_char_color( pdesc, ch );
	        if ( doexaprog ) oprog_examine_trigger( ch, obj );
		return;
	    }

	    if ( (pdesc=get_extra_descr(arg, obj->pIndexData->first_extradesc)) != NULL )
	    {
		if ( (cnt += obj->count) < number )
		  continue;
		send_to_char_color( pdesc, ch );
	        if ( doexaprog ) oprog_examine_trigger( ch, obj );
		return;
	    }
	    if ( nifty_is_name_prefix( arg, obj->name ) )
	    {
		if ( (cnt += obj->count) < number )
		  continue;
		pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );
		if ( !pdesc )
		  pdesc = get_extra_descr( obj->name, obj->first_extradesc );
		if ( !pdesc )
		  send_to_char_color( "You see nothing special.\r\n", ch );
		else
		  send_to_char_color( pdesc, ch );
		if (obj->item_type == ITEM_PUDDLE)
		  ch_printf( ch, "It's a puddle of %s liquid.\n\r",
		( obj->value[2] >= LIQ_MAX ? "clear" : liq_table[obj->value[2]].liq_color ) );
		if ( doexaprog ) oprog_examine_trigger( ch, obj );
		  return;
	    }
	}
    }

    for ( obj = ch->in_room->last_content; obj; obj = obj->prev_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    if ( (pdesc=get_extra_descr(arg, obj->first_extradesc)) != NULL )
	    {
		if ( (cnt += obj->count) < number )
		  continue;
		send_to_char_color( pdesc, ch );
	        if ( doexaprog ) oprog_examine_trigger( ch, obj );
		return;
	    }

	    if ( (pdesc=get_extra_descr(arg, obj->pIndexData->first_extradesc)) != NULL )
	    {
		if ( (cnt += obj->count) < number )
		  continue;
		send_to_char_color( pdesc, ch );
	        if ( doexaprog ) oprog_examine_trigger( ch, obj );
		return;
	    }
	    if ( nifty_is_name_prefix( arg, obj->name ) )
	    {
		if ( (cnt += obj->count) < number )
		  continue;
		pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );
		if ( !pdesc )
		  pdesc = get_extra_descr( obj->name, obj->first_extradesc );
		if ( !pdesc )
		  send_to_char( "You see nothing special.\r\n", ch );
		else
		  send_to_char_color( pdesc, ch );
		if (obj->item_type == ITEM_PUDDLE)
		  ch_printf( ch, "It's a puddle of %s liquid.\n\r",
		( obj->value[2] >= LIQ_MAX ? "clear" : liq_table[obj->value[2]].liq_color ) );
		if ( doexaprog ) oprog_examine_trigger( ch, obj );
		  return;
	    }
	}
    }

    send_to_char( "You do not see that here.\n\r", ch );
    return;
}

void show_race_line( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    int feet, inches;


	if ( !IS_NPC(victim) && (victim != ch) )
	{
		feet =  victim->height / 12;
		inches = victim->height % 12;
		sprintf( buf, "%s is %d'%d\" and weighs %d pounds.\n\r", PERS(victim, ch), feet, inches, victim->weight );
		send_to_char( buf, ch);
                return;
        } 
	if ( !IS_NPC(victim) && (victim == ch) )
	{
		feet =  victim->height / 12;
		inches = victim->height % 12;
		sprintf( buf, "You are %d'%d\" and weigh %d pounds.\n\r",  feet, inches, victim->weight );
		send_to_char( buf, ch);
                return;
        } 

}


void show_condition( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    int percent;

    if ( victim->max_hit > 0 )
        percent = ( 100 * victim->hit ) / victim->max_hit;
    else
        percent = -1;


    if ( victim != ch )
    {
	strcpy( buf, PERS(victim, ch) );
             if ( percent >= 100 ) strcat( buf, " is in perfect health.\n\r" );
	else if ( percent >=  90 ) strcat( buf, " is slightly scratched.\n\r" );
        else if ( percent >=  80 ) strcat( buf, " has a few bruises.\n\r"     );
	else if ( percent >=  70 ) strcat( buf, " has some cuts.\n\r"         );
	else if ( percent >=  60 ) strcat( buf, " has several wounds.\n\r"    );
	else if ( percent >=  50 ) strcat( buf, " has many nasty wounds.\n\r" );
	else if ( percent >=  40 ) strcat( buf, " is bleeding freely.\n\r"    );
	else if ( percent >=  30 ) strcat( buf, " is covered in blood.\n\r"   );
	else if ( percent >=  20 ) strcat( buf, " is leaking guts.\n\r"       );
	else if ( percent >=  10 ) strcat( buf, " is almost dead.\n\r"        );
	else                       strcat( buf, " is DYING.\n\r"              );
    }
    else
    {
	strcpy( buf, "You" );
	     if ( percent >= 100 ) strcat( buf, " are in perfect health.\n\r" );
	else if ( percent >=  90 ) strcat( buf, " are slightly scratched.\n\r");
        else if ( percent >=  80 ) strcat( buf, " have a few bruises.\n\r"    );
        else if ( percent >=  70 ) strcat( buf, " have some cuts.\n\r"        );
        else if ( percent >=  60 ) strcat( buf, " have several wounds.\n\r"   );
        else if ( percent >=  50 ) strcat( buf, " have many nasty wounds.\n\r");
	else if ( percent >=  40 ) strcat( buf, " are bleeding freely.\n\r"   );
	else if ( percent >=  30 ) strcat( buf, " are covered in blood.\n\r"  );
	else if ( percent >=  20 ) strcat( buf, " are leaking guts.\n\r"      );
	else if ( percent >=  10 ) strcat( buf, " are almost dead.\n\r"       );
	else                       strcat( buf, " are DYING.\n\r"             );
    }

    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );
    return;
}

/* A much simpler version of look, this function will show you only
the condition of a mob or pc, or if used without an argument, the
same you would see if you enter the room and have config +brief.
-- Narn, winter '96
*/
void do_glance( CHAR_DATA *ch, char *argument )
{
  char arg1 [MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  bool brief;
 
  if ( !ch->desc )
    return;
 
  if ( ch->position < POS_SLEEPING )
  {
    send_to_char( "You can't see anything but stars!\n\r", ch );
    return;
  }
 
  if ( ch->position == POS_SLEEPING )
  {
    send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
    return;
  }
 
  if ( !check_blind( ch ) )
    return;

  set_char_color( AT_ACTION, ch ); 
  argument = one_argument( argument, arg1 );
 
  if ( arg1[0] == '\0' )
  {
    if ( xIS_SET(ch->act, PLR_BRIEF) )
	brief = TRUE;
    else
	brief = FALSE;
    xSET_BIT( ch->act, PLR_BRIEF );
    do_look( ch, "auto" );
    if ( !brief )
	xREMOVE_BIT(ch->act, PLR_BRIEF);
    return;
  }
 
  if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
  {
    send_to_char( "They're not here.\n\r", ch );
    return;
  }
  else
  {
    if ( can_see( victim, ch ) )
    {
	act( AT_ACTION, "$n glances at you.", ch, NULL, victim, TO_VICT );
	act( AT_ACTION, "$n glances at $N.",  ch, NULL, victim, TO_NOTVICT );
    }
    if ( IS_IMMORTAL( ch ) && victim != ch )
    {
	if ( IS_NPC( victim ) )
	    ch_printf( ch, "Mobile #%d '%s' ",
		victim->pIndexData->vnum, victim->name );
	else
	    ch_printf( ch, "%s ", victim->name );
	ch_printf( ch, "is a level %d %s %s.\n\r",
	    victim->level,
	    IS_NPC(victim)?victim->race<MAX_NPC_RACE&&victim->race>=0?
            npc_race[victim->race] : "unknown":victim->race<MAX_PC_RACE&&
            race_table[victim->race]->race_name&&
            race_table[victim->race]->race_name[0] != '\0'?
            race_table[victim->race]->race_name:"unknown",
            IS_NPC(victim)?victim->class<MAX_NPC_CLASS&&victim->class>=0?
            npc_class[victim->class] : "unknown":victim->class<MAX_PC_CLASS&&
            class_table[victim->class]->who_name&&
            class_table[victim->class]->who_name[0] != '\0'?
            class_table[victim->class]->who_name:"unknown");
/* New Change
	    victim->race<MAX_NPC_RACE&&victim->race>=0?npc_race[victim->race] : "unknown",
	    victim->class<MAX_NPC_CLASS&&victim->class>=0?npc_class[victim->class] : "unknown" );
*/
    }
    show_condition( ch, victim );

    return;
  }

  return;
}


void do_examine( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    BOARD_DATA *board;
    sh_int dam;

    if ( !argument )
    {
	bug( "do_examine: null argument.", 0);
	return;
    }

    if ( !ch )
    {
	bug( "do_examine: null ch.", 0);
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Examine what?\n\r", ch );
	return;
    }

    sprintf( buf, "%s noprog", arg );
    do_look( ch, buf );

    /*
     * Support for looking at boards, checking equipment conditions,
     * and support for trigger positions by Thoric
     */
    if ( (obj = get_obj_here(ch, arg)) != NULL )
    {
	if ( (board = get_board(obj)) != NULL )
	{
	    if ( board->num_posts )
		ch_printf( ch, "There are about %d notes posted here.  Type 'note list' to list them.\n\r", board->num_posts );
	    else
		send_to_char( "There aren't any notes posted here.\n\r", ch );
	}

	switch ( obj->item_type )
	{
	default:
	    break;

	case ITEM_ARMOR:
	    if ( obj->value[1] == 0 )
	      obj->value[1] = obj->value[0];
	    if ( obj->value[1] == 0 )
	      obj->value[1] = 1;
	    dam = (sh_int) ((obj->value[0] * 10) / obj->value[1]);
	    strcpy( buf, "As you look more closely, you notice that it is ");
	    if (dam >= 10) strcat( buf, "in superb condition.");
       else if (dam ==  9) strcat( buf, "in very good condition.");
       else if (dam ==  8) strcat( buf, "in good shape.");
       else if (dam ==  7) strcat( buf, "showing a bit of wear.");
       else if (dam ==  6) strcat( buf, "a little run down.");
       else if (dam ==  5) strcat( buf, "in need of repair.");
       else if (dam ==  4) strcat( buf, "in great need of repair.");
       else if (dam ==  3) strcat( buf, "in dire need of repair.");
       else if (dam ==  2) strcat( buf, "very badly worn.");
       else if (dam ==  1) strcat( buf, "practically worthless.");
       else if (dam <=  0) strcat( buf, "broken.");
	    strcat( buf, "\n\r" );
	    send_to_char( buf, ch );
	    break;

         case ITEM_JOURNAL:
         {
                sh_int count = 0;
                EXTRA_DESCR_DATA *ed;

                for ( ed = obj->first_extradesc; ed; ed = ed->next )
                count++;

                ch_printf( ch, "%s has %d %s written in out of a possible %d.\n\r",
                   obj->short_descr,
                   count,
                   count == 1 ? "page" : "pages",
                   obj->value[0] );

                break;
        }


	case ITEM_WEAPON:
	    dam = INIT_WEAPON_CONDITION - obj->value[0];
	    strcpy( buf, "As you look more closely, you notice that it is ");
	    if (dam <=  0) strcat( buf, "in superb condition.");
       else if (dam ==  1) strcat( buf, "in excellent condition.");
       else if (dam ==  2) strcat( buf, "in very good condition.");
       else if (dam ==  3) strcat( buf, "in good shape.");
       else if (dam ==  4) strcat( buf, "showing a bit of wear.");
       else if (dam ==  5) strcat( buf, "a little run down.");
       else if (dam ==  6) strcat( buf, "in need of repair.");
       else if (dam ==  7) strcat( buf, "in great need of repair.");
       else if (dam ==  8) strcat( buf, "in dire need of repair.");
       else if (dam ==  9) strcat( buf, "very badly worn.");
       else if (dam == 10) strcat( buf, "practically worthless.");
       else if (dam == 11) strcat( buf, "almost broken.");
       else if (dam == 12) strcat( buf, "broken.");
	    strcat( buf, "\n\r" );
	    send_to_char( buf, ch );
	    break;

	case ITEM_COOK:
	    strcpy( buf, "As you examine it carefully you notice that it " );
	    dam = obj->value[2];
	     if (dam >= 3) strcat( buf, "is burned to a crisp.");
	     /* Changed to 2, bug caught by Keith Howell */
	else if (dam == 2) strcat( buf, "is a little over cooked.");
	else if (dam == 1) strcat( buf, "is perfectly roasted.");
	else		   strcat( buf, "is raw.");
	    strcat( buf, "\n\r" );
	    send_to_char( buf, ch );
	case ITEM_FOOD:
	    if ( obj->timer > 0 && obj->value[1] > 0 )
	      dam = (obj->timer * 10) / obj->value[1];
	    else
	      dam = 10;
	    if ( obj->item_type == ITEM_FOOD )	
	      strcpy( buf, "As you examine it carefully you notice that it " );
	    else
	      strcpy( buf, "Also it " );
	    if (dam >= 10) strcat( buf, "is fresh.");
       else if (dam ==  9) strcat( buf, "is nearly fresh.");
       else if (dam ==  8) strcat( buf, "is perfectly fine.");
       else if (dam ==  7) strcat( buf, "looks good.");
       else if (dam ==  6) strcat( buf, "looks ok.");
       else if (dam ==  5) strcat( buf, "is a little stale.");
       else if (dam ==  4) strcat( buf, "is a bit stale.");
       else if (dam ==  3) strcat( buf, "smells slightly off.");
       else if (dam ==  2) strcat( buf, "smells quite rank.");
       else if (dam ==  1) strcat( buf, "smells revolting!");
       else if (dam <=  0) strcat( buf, "is crawling with maggots!");
	    strcat( buf, "\n\r" );
	    send_to_char( buf, ch );
	    break;


	case ITEM_SWITCH:
	case ITEM_LEVER:
	case ITEM_PULLCHAIN:
	    if ( IS_SET( obj->value[0], TRIG_UP ) )
		send_to_char( "You notice that it is in the up position.\n\r", ch );
	    else
		send_to_char( "You notice that it is in the down position.\n\r", ch );
	    break;
	case ITEM_BUTTON:
	    if ( IS_SET( obj->value[0], TRIG_UP ) )
		send_to_char( "You notice that it is depressed.\n\r", ch );
	    else
		send_to_char( "You notice that it is not depressed.\n\r", ch );
	    break;

/* Not needed due to check in do_look already
	case ITEM_PORTAL:
	    sprintf( buf, "in %s noprog", arg );
	    do_look( ch, buf );
	    break;
*/

        case ITEM_CORPSE_PC:
	case ITEM_CORPSE_NPC:
            {
		sh_int timerfrac = obj->timer;
		if ( obj->item_type == ITEM_CORPSE_PC )
		timerfrac = (int)obj->timer / 8 + 1; 

		switch (timerfrac)
		{
		    default:
			send_to_char( "This corpse has recently been slain.\n\r", ch );
			break;
		    case 4:
			send_to_char( "This corpse was slain a little while ago.\n\r", ch );
			break;
		    case 3:
			send_to_char( "A foul smell rises from the corpse, and it is covered in flies.\n\r", ch );
			break;
		    case 2:
			send_to_char( "A writhing mass of maggots and decay, you can barely go near this corpse.\n\r", ch );
			break;
		    case 1:
		    case 0:
			send_to_char( "Little more than bones, there isn't much left of this corpse.\n\r", ch );
			break;
		}
            }
	case ITEM_CONTAINER:
	    if ( IS_OBJ_STAT( obj, ITEM_COVERING ) )
		break;
	case ITEM_DRINK_CON:
	case ITEM_QUIVER:
	    send_to_char( "When you look inside, you see:\n\r", ch );
	case ITEM_KEYRING:
	    sprintf( buf, "in %s noprog", arg );
	    do_look( ch, buf );
	    break;
	}
	if ( IS_OBJ_STAT( obj, ITEM_COVERING ) )
	{
	    sprintf( buf, "under %s noprog", arg );
	    do_look( ch, buf );
	}
	oprog_examine_trigger( ch, obj );
	if ( char_died(ch) || obj_extracted(obj) )
	    return;

	check_for_trap( ch, obj, TRAP_EXAMINE );
    }
    return;
}

void do_exits( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    bool fAuto;

    buf[0] = '\0';
    fAuto  = !str_cmp( argument, "auto" );

    if ( !check_blind(ch) )
	return;

    set_char_color( AT_EXITS, ch );
    ch_printf_color( ch, "%s", fAuto ? "Exits: " : "Obvious exits:\n\r" );
    found = FALSE;
    for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
    {
	if ( pexit->to_room
/*	&&  !IS_SET(pexit->exit_info, EX_CLOSED) */
	&& (!IS_SET(pexit->exit_info, EX_WINDOW)
	||   IS_SET(pexit->exit_info, EX_ISDOOR))
	&&  !IS_SET(pexit->exit_info, EX_SECRET)
	&&  !IS_SET(pexit->exit_info, EX_HIDDEN)
	&&  !IS_SET(pexit->exit_info, EX_DIG) )
	{
	    found = TRUE;
	    if ( fAuto )
	    {
		if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
		{
		  if ( pexit->keyword
		  && ( !str_cmp( "door", pexit->keyword )
		  ||   !str_cmp( "gate", pexit->keyword )
		  ||    pexit->keyword[0] == '\0' ) )
		  {
			strcat( buf, "[" );
			strcat( buf, dir_name[pexit->vdir] );
			strcat( buf, "] " );
		  }
		}
		else
		{
		  strcat( buf, dir_name[pexit->vdir] );
		  strcat( buf, " " );
		}
	    }
	    else
	    {
		sprintf( buf + strlen(buf), "%-5s - %s\n\r",
		    capitalize( dir_name[pexit->vdir] ),
		    room_is_dark( pexit->to_room )
			?  "Too dark to tell"
			: pexit->to_room->name
		    );
	    }
	}
    }

    if ( !found )
	strcat( buf, fAuto ? "none\n\r" : "None\n\r" );
    else
    if ( fAuto )
	strcat( buf, "\n\r" );
    send_to_char_color( buf, ch );
    return;
}


char *	const	day_name	[] =
{
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

char *	const	month_name	[] =
{
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time( CHAR_DATA *ch, char *argument )
{
    extern char str_boot_time[];
    extern char reboot_time[];
    char *suf;
    int day;

    day     = time_info.day + 1;

	 if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

    set_char_color( AT_YELLOW, ch );
    ch_printf( ch,
	"It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n\r"  
        "The mud started up at:    %s\r"
        "The system time (E.S.T.): %s\r"
        "Next Reboot is set for:   %s\r",

	(time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
	time_info.hour >= 12 ? "pm" : "am",
	day_name[day % 7],
	day, suf,
	month_name[time_info.month],
	str_boot_time,
	(char *) ctime( &current_time ),
	reboot_time
	);

    return;
}

/*
void do_weather( CHAR_DATA *ch, char *argument )
{
    static char * const sky_look[4] =
    {
	"cloudless",
	"cloudy",
	"rainy",
	"lit by flashes of lightning"
    };

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You can't see the sky from here.\n\r", ch );
	return;
    }

    set_char_color( AT_BLUE, ch );
    ch_printf( ch, "The sky is %s and %s.\n\r",
	sky_look[weather_info.sky],
	weather_info.change >= 0
	? "a warm southerly breeze blows"
	: "a cold northern gust blows"
	);
    return;
}
*/

/*
 * Produce a description of the weather based on area weather using
 * the following sentence format:
 *		<combo-phrase> and <single-phrase>.
 * Where the combo-phrase describes either the precipitation and
 * temperature or the wind and temperature. The single-phrase
 * describes either the wind or precipitation depending upon the
 * combo-phrase.
 * Last Modified: July 31, 1997
 * Fireblade - Under Construction
 */
void do_weather(CHAR_DATA *ch, char *argument)
{
	char *combo, *single;
	char buf[MAX_INPUT_LENGTH];
	int temp, precip, wind;	

	if ( !IS_OUTSIDE(ch) )
	{
	    ch_printf(ch, "You can't see the sky from here.\n\r");
	    return;
	}

	temp = (ch->in_room->area->weather->temp + 3*weath_unit - 1)/
		weath_unit;
	precip = (ch->in_room->area->weather->precip + 3*weath_unit - 1)/
		weath_unit;
	wind = (ch->in_room->area->weather->wind + 3*weath_unit - 1)/
		weath_unit;
	
	if ( precip >= 3 )
	{
	    combo = preciptemp_msg[precip][temp];
	    single = wind_msg[wind];
	}
	else
	{
	    combo = windtemp_msg[wind][temp];
	    single = precip_msg[precip];
	}
	
	sprintf(buf, "%s and %s.\n\r", combo, single);
	
	set_char_color(AT_BLUE, ch);
	
	ch_printf(ch, buf);
}

/*
 * Moved into a separate function so it can be used for other things
 * ie: online help editing				-Thoric
 */
HELP_DATA *get_help( CHAR_DATA *ch, char *argument )
{
    char argall[MAX_INPUT_LENGTH];
    char argone[MAX_INPUT_LENGTH];
    char argnew[MAX_INPUT_LENGTH];
    HELP_DATA *pHelp;
    int lev;

    if ( argument[0] == '\0' )
	argument = "summary";

    if ( isdigit(argument[0]) )
    {
	lev = number_argument( argument, argnew );
	if ( strcmp(argument, argnew) )
	  argument = argnew;
	else
	  lev = -2;
    }
    else
	lev = -2;
    /*
     * Tricky argument handling so 'help a b' doesn't match a.
     */
    argall[0] = '\0';
    while ( argument[0] != '\0' )
    {
	argument = one_argument( argument, argone );
	if ( argall[0] != '\0' )
	    strcat( argall, " " );
	strcat( argall, argone );
    }

    for ( pHelp = first_help; pHelp; pHelp = pHelp->next )
    {
	if ( pHelp->level > get_trust( ch ) )
	    continue;
	if ( lev != -2 && pHelp->level != lev )
	    continue;

	if ( is_name( argall, pHelp->keyword ) )
	    return pHelp;
    }

    return NULL;
}

void do_nanny_help( CHAR_DATA *ch, char *argument )
{
   HELP_DATA *pHelp;
   char       buf[MAX_STRING_LENGTH];

   sprintf( buf, "nanny_%s", argument );

   if ( ( pHelp = get_help( ch, buf ) ) == NULL )
        return;

   if ( pHelp->level >= 0 )
   {
      send_to_pager( "\n\r", ch );
      set_pager_color( AT_GREY, ch );
   }

   send_to_pager_color( pHelp->text + ( pHelp->text[0] == '.' ? 1 : 0 ) , ch );
   return;
}

/*
 * LAWS command
 */
void do_laws( CHAR_DATA *ch, char *argument )
{
    char buf[1024];

    if ( argument == NULL)
	do_help( ch, "laws" );
    else
    {
	sprintf( buf, "law %s", argument );
	do_help( ch, buf );
    }
}

//  Ranks by number of matches between two whole words. Coded for the Similar Helpfiles
//  Snippet by Senir.
sh_int str_similarity( const char *astr, const char *bstr )
{
   sh_int matches=0;

    if (!astr || !bstr)
       return matches;

    for ( ; *astr; astr++)
    {
        if ( LOWER(*astr) == LOWER(*bstr) )
           matches++;
        
        if (++bstr == '\0')
           return matches;                
    }
    
    return matches;
}

//  Ranks by number of matches until there's a nonmatching character between two words.
//  Coded for the Similar Helpfiles Snippet by Senir.
sh_int str_prefix_level( const char *astr, const char *bstr )
{
   sh_int matches=0;

    if (!astr || !bstr)
       return matches;

    for ( ; *astr; astr++)
    {
        if ( LOWER(*astr) == LOWER(*bstr) )
           matches++;
        else
           return matches;

        if (++bstr == '\0')
        return matches;
    }

    return matches;
}

// Main function of Similar Helpfiles Snippet by Senir. It loops through all of the
// helpfiles, using the string matching function defined to find the closest matching
// helpfiles to the argument. It then checks for singles. Then, if matching helpfiles
// are found at all, it loops through and prints out the closest matching helpfiles.
// If its a single(there's only one), it opens the helpfile.

void similar_help_files(CHAR_DATA *ch, char *argument)
{
   HELP_DATA *pHelp=NULL;
   char buf[MAX_STRING_LENGTH];
   char *extension;
   sh_int lvl=0;
   bool single=FALSE;
    
        
    send_to_pager_color( "&C&RSimilar Help Files:\n\r", ch);
        
    for ( pHelp = first_help; pHelp; pHelp=pHelp->next)
    {
        buf[0]='\0';      
        extension=pHelp->keyword;
        
        if (pHelp->level > get_trust(ch))
           continue;

        while ( extension[0] != '\0' )
        {
              extension= one_argument(extension, buf);
              
              if ( str_similarity(argument, buf) > lvl)
              {
                 lvl=str_similarity(argument, buf);
                 single=TRUE;
              }        
              else if ( str_similarity(argument, buf) == lvl && lvl > 0)
              {
                 single=FALSE;
              }
        }
    }
        
    if (lvl==0)
    {
       send_to_pager_color( "&C&RNo similar help files.\n\r", ch);   
       return;
    }

    for ( pHelp = first_help; pHelp; pHelp=pHelp->next)
    {
        buf[0]='\0';      
        extension=pHelp->keyword;

        while ( extension[0] != '\0' )
        {
              extension=one_argument(extension, buf);

              if ( str_similarity(argument, buf) >= lvl
                   && pHelp->level <= get_trust(ch))
              {
                 if (single)
                 {
                    send_to_pager_color( "&C&WOpening only similar helpfile.&C\n\r", ch);
                    do_help( ch, buf);
                    return;
                 }

                 pager_printf_color(ch, "&C&W   %s\n\r", pHelp->keyword);
                 break;

              }

        }
    }
    return;
}



/* Mods by Luc to alleviate some pager color bleeding */
void do_help( CHAR_DATA *ch, char *argument )
{
   HELP_DATA *pHelp;

   if ( (pHelp = get_help( ch, argument )) == NULL ) {
      /*
       *  Write this to a file so we can see what people
       * are looking for.  --Shaddai
       */
      char buf[MAX_STRING_LENGTH];
      sprintf( buf, "%s : %s", argument, ch->name);
      append_to_file(NOHELP_FILE, buf );

      //  Looks better printing out the missed argument before going to similar
      //  helpfiles. - Senir
      pager_printf_color( ch, "&C&RNo help on \'%s\' found.\n\r", argument );
      similar_help_files(ch, argument);
      return;
      }
   /* Make newbies do a help start. --Shaddai */
   if ( !IS_NPC(ch) && !str_cmp( argument, "start" ) )
      SET_BIT(ch->pcdata->flags, PCFLAG_HELPSTART);
   if ( pHelp->level >= 0 && str_cmp( argument, "imotd" ) ) {
      set_pager_color( AT_RED, ch );
      send_to_pager( pHelp->keyword, ch );
      send_to_pager( "\n\r", ch );
      set_pager_color( AT_GREY, ch );
      }
   /* Strip leading '.' to allow initial blanks. */
   send_to_pager_color( pHelp->text + ( pHelp->text[0] == '.' ? 1 : 0 ) , ch );
}

void do_changes( CHAR_DATA *ch, char *argument )
{

    if ( IS_NPC( ch ) )
        return;
    send_to_char_color( "\n\r&gChanges to the Realms ...\n\r", ch );
    show_file( ch, CHANGE_FILE );
    send_to_char_color( "&G&g... type &G'&gnews&G'&g for recent updates.\n\r", ch );
    if ( IS_IMMORTAL( ch ) )
	send_to_char_color( "&G&gImmortals should also check &G'&gimmnews&G'&g.\n\r", ch );
    return;
}

/*  Replaced with extended news snippet
void do_news( CHAR_DATA *ch, char *argument )
{

    if ( IS_NPC( ch ) )
	return;
    send_to_char_color( "\n\r&gNews of the Realms ...\n\r", ch );
    show_file( ch, NEWS_FILE );
    send_to_char_color( "&G&g... type &G'&gchanges&G'&g for other or older updates.\n\r", ch );
    if ( IS_IMMORTAL( ch ) )
	send_to_char_color( "&G&gImmortals should also check &G'&gimmnews&G'&g.\n\r", ch );
    return;
}
*/


extern char * help_greeting;	/* so we can edit the greeting online */

/*
 * Help editor							-Thoric
 */
void do_hedit( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;

    if ( !ch->desc )
    {
	send_to_char( "You have no descriptor.\n\r", ch );
	return;
    }

    switch( ch->substate )
    {
	default:
	  break;
	case SUB_HELP_EDIT:
	  if ( (pHelp = ch->dest_buf) == NULL )
	  {
		bug( "hedit: sub_help_edit: NULL ch->dest_buf", 0 );
		stop_editing( ch );
		return;
	  }
	  if ( help_greeting == pHelp->text )
		help_greeting = NULL;
	  STRFREE( pHelp->text );
	  pHelp->text = copy_buffer( ch );
	  if ( !help_greeting )
		help_greeting = pHelp->text;
	  stop_editing( ch );
	  return;
    }
    if ( (pHelp = get_help(ch, argument)) == NULL )     /* new help */
    {
        HELP_DATA *tHelp;
        char argnew[MAX_INPUT_LENGTH];
        int lev;
        bool new_help = TRUE;

        for ( tHelp=first_help; tHelp; tHelp = tHelp->next )
           if ( !str_cmp( argument, tHelp->keyword) )
           {
                pHelp = tHelp;
                new_help = FALSE;
                break;
           }
        if ( new_help ) {
        if ( isdigit(argument[0]) )
        {
            lev = number_argument( argument, argnew );
            argument = argnew;
        }
        else
            lev = get_trust(ch);
        CREATE( pHelp, HELP_DATA, 1 );
        pHelp->keyword = STRALLOC( strupper(argument) );
        pHelp->text    = STRALLOC( "" );
        pHelp->level   = lev;
        add_help( pHelp );
        }
    }

    ch->substate = SUB_HELP_EDIT;
    ch->dest_buf = pHelp;
    start_editing( ch, pHelp->text );
}

/*
 * Stupid leading space muncher fix				-Thoric
 */
char *help_fix( char *text )
{
    char *fixed;

    if ( !text )
	return "";
    fixed = strip_cr(text);
    if ( fixed[0] == ' ' )
	fixed[0] = '.';
    return fixed;
}

void do_hset( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: hset <field> [value] [help page]\n\r",	ch );
	send_to_char( "\n\r",						ch );
	send_to_char( "Field being one of:\n\r",			ch );
	send_to_char( "  level keyword remove save\n\r",		ch );
	return;
    }

    if ( !str_cmp( arg1, "save" ) )
    {
	FILE *fpout;

	log_string_plus( "Saving help.are...", LOG_NORMAL, LEVEL_GREATER );

	rename( "help.are", "help.are.bak" );
	fclose( fpReserve );
	if ( ( fpout = fopen( "help.are", "w" ) ) == NULL )
	{
	   bug( "hset save: fopen", 0 );
	   perror( "help.are" );
	   fpReserve = fopen( NULL_FILE, "r" );
	   return;
	}
    
	fprintf( fpout, "#HELPS\n\n" );
	for ( pHelp = first_help; pHelp; pHelp = pHelp->next )
	    fprintf( fpout, "%d %s~\n%s~\n\n",
		pHelp->level, pHelp->keyword, help_fix(pHelp->text) );

	fprintf( fpout, "0 $~\n\n\n#$\n" );
	fclose( fpout );
	fpReserve = fopen( NULL_FILE, "r" );
	send_to_char( "Saved.\n\r", ch );
	return;
    }
    if ( str_cmp( arg1, "remove" ) )
	argument = one_argument( argument, arg2 );

    if ( (pHelp = get_help(ch, argument)) == NULL )
    {
	send_to_char( "Cannot find help on that subject.\n\r", ch );
	return;
    }
    if ( !str_cmp( arg1, "remove" ) )
    {
	UNLINK( pHelp, first_help, last_help, next, prev );
	STRFREE( pHelp->text );
	STRFREE( pHelp->keyword );
	DISPOSE( pHelp );
	send_to_char( "Removed.\n\r", ch );
	return;
    }
    if ( !str_cmp( arg1, "level" ) )
    {
	pHelp->level = atoi( arg2 );
	send_to_char( "Done.\n\r", ch );
	return;
    }
    if ( !str_cmp( arg1, "keyword" ) )
    {
	STRFREE( pHelp->keyword );
	pHelp->keyword = STRALLOC( strupper(arg2) );
	send_to_char( "Done.\n\r", ch );
	return;
    }

    do_hset( ch, "" );
}

void do_hl( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to use HLIST, spell it out.\n\r", ch );
    return;
}

/*
 * Show help topics in a level range				-Thoric
 * Idea suggested by Gorog
 * prefix keyword indexing added by Fireblade
 */
void do_hlist( CHAR_DATA *ch, char *argument )
{
    int min, max, minlimit, maxlimit, cnt;
    char arg[MAX_INPUT_LENGTH];
    HELP_DATA *help;
    bool minfound, maxfound;
    char *idx;

    maxlimit = get_trust(ch);
    minlimit = maxlimit >= LEVEL_GREATER ? -1 : 0;
    
    min = minlimit;
    max  = maxlimit;
    
    idx = NULL;
    minfound = FALSE;
    maxfound = FALSE;
    
    for ( argument = one_argument(argument, arg); arg[0] != '\0';
	  argument = one_argument(argument, arg))
    {
	if( !isdigit(arg[0]) )
	{
 	    if ( idx )
	    {
		set_char_color(AT_GREEN, ch);
		ch_printf(ch, "You may only use a single keyword to index the list.\n\r");
		return;
	    }
	    idx = STRALLOC(arg);
    	}
	else
	{
	    if ( !minfound )
	    {
		min = URANGE(minlimit, atoi(arg), maxlimit);
		minfound = TRUE;
	    }
	    else
	    if ( !maxfound )
	    {
		max = URANGE(minlimit, atoi(arg), maxlimit);
		maxfound = TRUE;
	    }
	    else
	    {
		set_char_color(AT_GREEN, ch);
		ch_printf(ch, "You may only use two level limits.\n\r");
		return;
	    }
	}
    }
    
    if ( min > max )
    {
	int temp = min;

	min = max;
	max = temp;
    }
    
    set_pager_color( AT_GREEN, ch );
    pager_printf( ch, "Help Topics in level range %d to %d:\n\r\n\r", min, max );
    for ( cnt = 0, help = first_help; help; help = help->next )
	if ( help->level >= min && help->level <= max
	&&  (!idx || nifty_is_name_prefix(idx, help->keyword)) )
	{
	    pager_printf( ch, "  %3d %s\n\r", help->level, help->keyword );
	    ++cnt;
	}
    if ( cnt )
	pager_printf( ch, "\n\r%d pages found.\n\r", cnt );
    else
	send_to_char( "None found.\n\r", ch );

    if ( idx )
    	STRFREE(idx);
    
    return;
}


/* 
 * New do_who with WHO REQUEST, clan, race and homepage support.  -Thoric
 *
 * Latest version of do_who eliminates redundant code by using linked lists.
 * Shows imms separately, indicates guest and retired immortals.
 * Narn, Oct/96
 *
 * Who group by Altrag, Feb 28/97
 */
struct whogr_s
{
  struct whogr_s *next;
  struct whogr_s *follower;
  struct whogr_s *l_follow;
  DESCRIPTOR_DATA *d;
  int indent;
} *first_whogr, *last_whogr;

struct whogr_s *find_whogr(DESCRIPTOR_DATA *d, struct whogr_s *first)
{
  struct whogr_s *whogr, *whogr_t;
  
  for (whogr = first; whogr; whogr = whogr->next)
    if (whogr->d == d)
      return whogr;
    else if (whogr->follower && (whogr_t = find_whogr(d, whogr->follower)))
      return whogr_t;
  return NULL;
}

void indent_whogr(CHAR_DATA *looker, struct whogr_s *whogr, int ilev)
{
  for ( ; whogr; whogr = whogr->next )
  {
    if (whogr->follower)
    {
      int nlev = ilev;
      CHAR_DATA *wch =
          (whogr->d->original ? whogr->d->original : whogr->d->character);
      
      if (can_see(looker, wch) && !IS_IMMORTAL(wch))
        nlev += 3;
      indent_whogr(looker, whogr->follower, nlev);
    }
    whogr->indent = ilev;
  }
}

/* This a great big mess to backwards-structure the ->leader character
   fields */
void create_whogr(CHAR_DATA *looker)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *wch;
  struct whogr_s *whogr, *whogr_t;
  int dc = 0, wc = 0;
  
  while ((whogr = first_whogr) != NULL)
  {
    first_whogr = whogr->next;
    DISPOSE(whogr);
  }
  first_whogr = last_whogr = NULL;
  /* Link in the ones without leaders first */
  for (d = last_descriptor; d; d = d->prev)
  {
    if (d->connected != CON_PLAYING && d->connected != CON_EDITING)
      continue;
    ++dc;
    wch = (d->original ? d->original : d->character);
    if (!wch->leader || wch->leader == wch || !wch->leader->desc ||
         IS_NPC(wch->leader) || IS_IMMORTAL(wch) || IS_IMMORTAL(wch->leader))
    {
      CREATE(whogr, struct whogr_s, 1);
      if (!last_whogr)
        first_whogr = last_whogr = whogr;
      else
      {
        last_whogr->next = whogr;
        last_whogr = whogr;
      }
      whogr->next = NULL;
      whogr->follower = whogr->l_follow = NULL;
      whogr->d = d;
      whogr->indent = 0;
      ++wc;
    }
  }
  /* Now for those who have leaders.. */
  while (wc < dc)
    for (d = last_descriptor; d; d = d->prev)
    {
      if (d->connected != CON_PLAYING && d->connected != CON_EDITING)
        continue;
      if (find_whogr(d, first_whogr))
        continue;
      wch = (d->original ? d->original : d->character);
      if (wch->leader && wch->leader != wch && wch->leader->desc &&
         !IS_NPC(wch->leader) && !IS_IMMORTAL(wch) &&
         !IS_IMMORTAL(wch->leader) &&
         (whogr_t = find_whogr(wch->leader->desc, first_whogr)))
      {
        CREATE(whogr, struct whogr_s, 1);
        if (!whogr_t->l_follow)
          whogr_t->follower = whogr_t->l_follow = whogr;
        else
        {
          whogr_t->l_follow->next = whogr;
          whogr_t->l_follow = whogr;
        }
        whogr->next = NULL;
        whogr->follower = whogr->l_follow = NULL;
        whogr->d = d;
        whogr->indent = 0;
        ++wc;
      }
    }
  /* Set up indentation levels */
  indent_whogr(looker, first_whogr, 0);
  
  /* And now to linear link them.. */
  for (whogr_t = NULL, whogr = first_whogr; whogr; )
    if (whogr->l_follow)
    {
      whogr->l_follow->next = whogr;
      whogr->l_follow = NULL;
      if (whogr_t)
        whogr_t->next = whogr = whogr->follower;
      else
        first_whogr = whogr = whogr->follower;
    }
    else
    {
      whogr_t = whogr;
      whogr = whogr->next;
    }
}

void do_who( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char clan_name[MAX_INPUT_LENGTH];
    char council_name[MAX_INPUT_LENGTH];
    char invis_str[MAX_INPUT_LENGTH];
    char char_name[MAX_INPUT_LENGTH];
    char *extra_title;
    char class_text[MAX_INPUT_LENGTH];
    struct whogr_s *whogr, *whogr_p;
    DESCRIPTOR_DATA *d;
    int iClass, iRace;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    bool rgfClass[MAX_CLASS];
    bool rgfRace[MAX_RACE];
    bool fClassRestrict;
    bool fRaceRestrict;
    bool fImmortalOnly;
    bool fLeader;
    bool fPkill;
    bool fShowHomepage;
    bool fRetired = FALSE;
    bool fClanMatch; /* SB who clan (order),who guild, and who council */
    bool fCouncilMatch;
    bool fDeityMatch;
    bool fGroup;
    CLAN_DATA *pClan = NULL;
    COUNCIL_DATA *pCouncil = NULL;
    DEITY_DATA *pDeity = NULL;
    FILE *whoout = NULL;

    /*
    #define WT_IMM    0;
    #define WT_MORTAL 1;
    #define WT_DEADLY 2;
    */

    WHO_DATA *cur_who = NULL;
    WHO_DATA *next_who = NULL;
    WHO_DATA *first_mortal = NULL;
    WHO_DATA *first_imm = NULL;
    WHO_DATA *first_deadly  = NULL;
    WHO_DATA *first_grouped = NULL;
    WHO_DATA *first_groupwho = NULL;


    /*
     * Set default arguments.
     */
    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL;
    fClassRestrict = FALSE;
    fRaceRestrict  = FALSE;
    fImmortalOnly  = FALSE;
    fPkill         = FALSE;
    fShowHomepage  = FALSE;
    fClanMatch	   = FALSE; /* SB who clan (order), who guild, who council */
    fCouncilMatch  = FALSE;
    fDeityMatch    = FALSE;
    fGroup	   = FALSE; /* Alty who group */
    fLeader	   = FALSE;
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	rgfClass[iClass] = FALSE;
    for ( iRace = 0; iRace < MAX_RACE; iRace++ )
	rgfRace[iRace] = FALSE;

#ifdef REQWHOARG
    /*
     * The who command must have at least one argument because we often
     * have up to 500 players on. Too much spam if a player accidentally
     * types "who" with no arguments.           --Gorog
     */
    if ( ch && argument[0] == '\0' )
    {
	send_to_char_color( "\n\r&GYou must specify at least one argument.\n\r", ch );
	send_to_char_color( "Use 'who 1' to view the entire who list.  Other arguments:\n\r", ch );
	send_to_char_color( " ... nc = new player advisors\n\r ... deadly = deadly players\n\r ... group = grouped players\n\r", ch);
	send_to_char_color( "\n\rFor additional arguments or help, type 'help who'.\n\r", ch );
        return;
    }
#endif
	
    /*
     * Parse arguments.
     */
    nNumber = 0;
    for ( ;; )
    {
	char arg[MAX_STRING_LENGTH];

	argument = one_argument( argument, arg );
	if ( arg[0] == '\0' )
	    break;

	if ( is_number( arg ) )
	{
	    switch ( ++nNumber )
	    {
	    case 1: iLevelLower = atoi( arg ); break;
	    case 2: iLevelUpper = atoi( arg ); break;
	    default:
		send_to_char( "Only two level numbers allowed.\n\r", ch );
		return;
	    }
	}
	else
	{
	    if ( strlen(arg) < 2 )
	    {
		send_to_char( "Arguments must be longer than that.\n\r", ch );
		return;
	    }

	    if ( !str_cmp( arg, "nc" ) || !str_cmp( arg, "newbie" ) || !str_cmp( arg, "advisor" ) )
	    {
		do_who( ch, "\'newbie council\'" );
		return;
	    }

	    /*
	     * Look for classes to turn on.
	     */
            if ( !str_cmp( arg, "deadly" ) || !str_cmp( arg, "pkill" ) )
              fPkill = TRUE;
            else
	    if ( !str_cmp( arg, "imm" ) || !str_cmp( arg, "gods" ) )
		fImmortalOnly = TRUE;
	    else
	    if ( !str_cmp( arg, "leader" ) )
	    	fLeader = TRUE;
	    else
/*
	    if ( !str_cmp( arg, "nc" ) || !str_cmp( arg, "advisor" ) || !str_cmp( arg, "newbie" ) )
		fShowAdvisor = TRUE;
	    else
*/
	    if ( !str_cmp( arg, "www" ) )
		fShowHomepage = TRUE;
	    else
		if ( !str_cmp( arg, "retired" ) )
		fRetired = TRUE;
		else
	    if ( !str_cmp( arg, "group" ) && ch )
		fGroup = TRUE;
            else		 /* SB who clan (order), guild, council */
             if  ( ( pClan = get_clan (arg) ) )
	   	fClanMatch = TRUE;
            else
             if ( ( pCouncil = get_council (arg) ) )
                fCouncilMatch = TRUE;
	    else
	     if ( ( pDeity = get_deity (arg) ) )
		fDeityMatch = TRUE;
	    else
	    {
		for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
		{
		    if ( !str_cmp( arg, class_table[iClass]->who_name ) )
		    {
			rgfClass[iClass] = TRUE;
			break;
		    }
		}
		if ( iClass != MAX_CLASS )
		  fClassRestrict = TRUE;

		for ( iRace = 0; iRace < MAX_RACE; iRace++ )
		{
		    if ( !str_cmp( arg, race_table[iRace]->race_name ) )
		    {
			rgfRace[iRace] = TRUE;
			break;
		    }
		}
		if ( iRace != MAX_RACE )
		  fRaceRestrict = TRUE;

		if ( iClass == MAX_CLASS && iRace == MAX_RACE 
 		 && fClanMatch == FALSE 
		 && fRetired == FALSE
         && fCouncilMatch == FALSE
		 && fDeityMatch == FALSE )
		{
		 send_to_char( "That\'s not a class, race, order, guild,"
			" council or deity.\n\r", ch );
		    return;
		}
	    }
	}
    }

    /*
     * Now find matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    if ( ch )
	set_pager_color( AT_GREEN, ch );
    else
    {
	if ( fShowHomepage )
	  whoout = fopen( WEBWHO_FILE, "w" );
	else
	  whoout = fopen( WHO_FILE, "w" );
	if ( !whoout )
	{
	  bug( "do_who: cannot open who file!" );
	  return;
	}
    }

/* start from last to first to get it in the proper order */
    if (fGroup)
    {
      create_whogr(ch);
      whogr = first_whogr;
      d = whogr->d;
    }
    else
    {
      whogr = NULL;
      d = last_descriptor;
    }
    whogr_p = NULL;
    for ( ; d; whogr_p = whogr, whogr = (fGroup ? whogr->next : NULL),
    			d = (fGroup ? (whogr ? whogr->d : NULL) : d->prev))
    {
	CHAR_DATA *wch;
	char const *class;

	if ( (d->connected != CON_PLAYING && d->connected != CON_EDITING)
	||   !can_see( ch, d->character ) || d->original)
	    continue;
	wch   = d->original ? d->original : d->character;
	if ( wch->level < iLevelLower
	||   wch->level > iLevelUpper
	|| ( fPkill && !CAN_PKILL( wch ) )
/*
	|| ( fShowAdvisor && wch->pcdata->council && !str_cmp( "Newbie Council", wch->pcdata->council->name ) )
*/
    || ( fRetired && !IS_RETIRED( wch ) )
	|| ( fImmortalOnly  && wch->level < LEVEL_IMMORTAL )
	|| ( fClassRestrict && !rgfClass[wch->class] )
	|| ( fRaceRestrict && !rgfRace[wch->race] )
	|| ( fClanMatch && ( pClan != wch->pcdata->clan ))  /* SB */
        || ( fCouncilMatch && ( pCouncil != wch->pcdata->council )) /* SB */ 
	|| ( fDeityMatch && ( pDeity != wch->pcdata->deity )) )
	    continue;
	if ( fLeader && !(wch->pcdata->council && 
	     ((wch->pcdata->council->head2 &&
	     !str_cmp(wch->pcdata->council->head2, wch->name)) ||
	     (wch->pcdata->council->head &&
	     !str_cmp(wch->pcdata->council->head, wch->name)))) &&
	     !(wch->pcdata->clan && ((wch->pcdata->clan->deity &&
	     !str_cmp(wch->pcdata->clan->deity,wch->name) )
	     || (wch->pcdata->clan->leader
	     && !str_cmp(wch->pcdata->clan->leader, wch->name ) )
	     || (wch->pcdata->clan->number1
	     && !str_cmp(wch->pcdata->clan->number1, wch->name ) )
	     || (wch->pcdata->clan->number2 
	     && !str_cmp(wch->pcdata->clan->number2, wch->name )))))
	     continue;

	if (fGroup && !wch->leader &&
	   !IS_SET(wch->pcdata->flags, PCFLAG_GROUPWHO) &&
	   (!whogr_p || !whogr_p->indent))
	  continue;

	nMatch++;

	if ( fShowHomepage
	&&   wch->pcdata->homepage
	&&   wch->pcdata->homepage[0] != '\0' )
	  sprintf( char_name, "<A HREF=\"%s\">%s</A>",
		show_tilde( wch->pcdata->homepage ), wch->name );
	else
	  strcpy( char_name, wch->name );

	sprintf( class_text, "%s%2d %s", NOT_AUTHED(wch) ? "N" : " ", wch->level, class_table[wch->class]->who_name );
	class = class_text;
	switch ( wch->level )
	{
	default: break;
	case MAX_LEVEL -  0: class = "Supreme Entity";	break;
	case MAX_LEVEL -  1: class = "Infinite";	break;
	case MAX_LEVEL -  2: class = "Eternal";		break;
	case MAX_LEVEL -  3: class = "Ancient";		break;
	case MAX_LEVEL -  4: class = "Exalted God";	break;
	case MAX_LEVEL -  5: class = "Ascendant God";	break;
	case MAX_LEVEL -  6: class = "Greater God";	break;
	case MAX_LEVEL -  7: class = "God";		break;
	case MAX_LEVEL -  8: class = "Lesser God";	break;
	case MAX_LEVEL -  9: class = "Immortal";	break;
	case MAX_LEVEL - 10: class = "Demi God";	break;
	case MAX_LEVEL - 11: class = "Savior";		break;
	case MAX_LEVEL - 12: class = "Creator";		break;
	case MAX_LEVEL - 13: class = "Acolyte";		break;
	case MAX_LEVEL - 14: class = "Neophyte";	break;
	case MAX_LEVEL - 15: class = "Avatar";		break;
	}

        if ( !str_cmp( wch->name, sysdata.guild_overseer ) )
          extra_title = " [Overseer of Guilds]";
        else if ( !str_cmp( wch->name, sysdata.guild_advisor ) )
          extra_title = " [Advisor to Guilds]";
        else
          extra_title = "";
 
        if ( IS_GUEST( wch ) )
          class = "Guest";
	else if ( wch->pcdata->clan
             &&  !str_cmp( wch->name, wch->pcdata->clan->leader )
             &&   wch->pcdata->clan->leadrank[0] != '\0' )
                class = wch->pcdata->clan->leadrank;
        else if ( wch->pcdata->clan
	     &&  !str_cmp( wch->name, wch->pcdata->clan->number1 )
             &&   wch->pcdata->clan->onerank[0] != '\0' )
                class = wch->pcdata->clan->onerank;
	else if ( wch->pcdata->clan
             &&  !str_cmp( wch->name, wch->pcdata->clan->number2 )
             &&   wch->pcdata->clan->tworank[0] != '\0' )
                class = wch->pcdata->clan->tworank;
  	else if ( wch->pcdata->rank && wch->pcdata->rank[0] != '\0' )
	  class = wch->pcdata->rank;
	else if ( IS_RETIRED( wch ) )
	  class = "Retired";

	if ( wch->pcdata->clan )
	{
          CLAN_DATA *pclan = wch->pcdata->clan;
	  if ( pclan->clan_type == CLAN_GUILD )
	    strcpy( clan_name, " <" );
	  else
	    strcpy( clan_name, " (" );

          if ( pclan->clan_type == CLAN_ORDER )
          {
	    if ( !str_cmp( wch->name, pclan->deity ) )
	      strcat( clan_name, "Deity, Order of " );
            else
	    if ( !str_cmp( wch->name, pclan->leader ) )
	      strcat( clan_name, "Leader, Order of " );
	    else
	    if ( !str_cmp( wch->name, pclan->number1 ) )
	      strcat( clan_name, "Number One, Order of " );
	    else
	    if ( !str_cmp( wch->name, pclan->number2 ) )
	      strcat( clan_name, "Number Two, Order of " );
	    else
	      strcat( clan_name, "Order of " );
          }
          else
	  if ( pclan->clan_type == CLAN_GUILD )
	  {
	    if ( !str_cmp( wch->name, pclan->leader ) )
              strcat( clan_name, "Leader, " );
            if ( !str_cmp( wch->name, pclan->number1 ) )
              strcat( clan_name, "First, " );
            if ( !str_cmp( wch->name, pclan->number2 ) )
              strcat( clan_name, "Second, " );
	  }
	  else
          {
	    if ( !str_cmp( wch->name, pclan->deity ) )
	      strcat( clan_name, "Deity of " );
	    else
	    if ( !str_cmp( wch->name, pclan->leader ) )
	      strcat( clan_name, "Leader of " );
	    else
	    if ( !str_cmp( wch->name, pclan->number1 ) )
	      strcat( clan_name, "Number One " );
	    else
	    if ( !str_cmp( wch->name, pclan->number2 ) )
	      strcat( clan_name, "Number Two " );
          } 
	  strcat( clan_name, pclan->name );
	  if ( pclan->clan_type == CLAN_GUILD )
	    strcat( clan_name, ">" );
	  else
	    strcat( clan_name, ")" );
	}
	else
	  clan_name[0] = '\0';

	if ( wch->pcdata->council )
	{
          strcpy( council_name, " [" );
          if (  wch->pcdata->council->head2 == NULL )
          {
             if (!str_cmp (wch->name, wch->pcdata->council->head))
                strcat (council_name, "Head of ");
          }
          else
          {
             if (!str_cmp (wch->name, wch->pcdata->council->head)
                 || !str_cmp ( wch->name, wch->pcdata->council->head2) )
                strcat (council_name, "Co-Head of ");
          }
          strcat( council_name, wch->pcdata->council_name );
          strcat( council_name, "]" );
	}
	else
          council_name[0] = '\0';

	if ( xIS_SET(wch->act, PLR_WIZINVIS) )
	  sprintf( invis_str, "(%d) ", wch->pcdata->wizinvis );
	else
	  invis_str[0] = '\0';
	sprintf( buf, "%*s%-15s %s%s%s%s%s%s%s%s.%s%s%s\n\r",
	  (fGroup ? whogr->indent : 0), "",
	  class,
	  invis_str,
          (wch->desc && wch->desc->connected) ? "[WRITING] " : "",
	  xIS_SET(wch->act, PLR_AFK) ? "[AFK] " : "",
	  xIS_SET(wch->act, PLR_ATTACKER) ? "(ATTACKER) " : "",
	  xIS_SET(wch->act, PLR_KILLER) ? "(KILLER) " : "",
	  xIS_SET(wch->act, PLR_THIEF)  ? "(THIEF) "  : "",
	  char_name,
	  wch->pcdata->title,
          extra_title,
	  clan_name,
	  council_name );

          /*  
           * This is where the old code would display the found player to the ch.
           * What we do instead is put the found data into a linked list
           */ 

          /* First make the structure. */
          CREATE( cur_who, WHO_DATA, 1 );
          cur_who->text = str_dup( buf );
          if ( wch->level > 50 && IS_IMMORTAL( wch ) )
            cur_who->type = WT_IMM;
          else if ( fGroup )
            if ( wch->leader || (whogr_p && whogr_p->indent) )
              cur_who->type = WT_GROUPED;
            else
              cur_who->type = WT_GROUPWHO;
          else if ( CAN_PKILL( wch ) ) 
            cur_who->type = WT_DEADLY;
          else
            cur_who->type = WT_MORTAL;

          /* Then put it into the appropriate list. */
          switch ( cur_who->type )
          {
            case WT_MORTAL:
              cur_who->next = first_mortal;
              first_mortal = cur_who;
              break;
            case WT_DEADLY:
              cur_who->next = first_deadly;
              first_deadly = cur_who;
              break;
            case WT_GROUPED:
              cur_who->next = first_grouped;
              first_grouped = cur_who;
              break;
            case WT_GROUPWHO:
              cur_who->next = first_groupwho;
              first_groupwho = cur_who;
              break;
            case WT_IMM:
              cur_who->next = first_imm;
              first_imm = cur_who;
              break;
          }

    }


    /* Ok, now we have three separate linked lists and what remains is to 
     * display the information and clean up.
     */
    /*
     * Two extras now for grouped and groupwho (wanting group). -- Alty
     */

    for ( cur_who = first_mortal; cur_who; cur_who = next_who )
    {
      if ( !ch )
        fprintf( whoout, cur_who->text );
      else
        send_to_pager( cur_who->text, ch );
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      DISPOSE( cur_who ); 
    } 

    if ( first_deadly )
    {
      if ( !ch )
        fprintf( whoout, "\n\r-------------------------------[ DEADLY CHARACTERS ]-------------------------\n\r\n\r" );
      else
       send_to_pager_color( "\n\r-------------------------------[ &GDEADLY CHARACTERS&g ]--------------------------\n\r\n\r", ch );
    }

    for ( cur_who = first_deadly; cur_who; cur_who = next_who )
    {
      if ( !ch )
        fprintf( whoout, cur_who->text );
      else
        send_to_pager( cur_who->text, ch );
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      DISPOSE( cur_who ); 
    }
    
    if (first_grouped)
    {
/*      if ( !ch )
        fprintf( whoout, "\n\r-----------------------------[ GROUPED CHARACTERS ]---------------------------\n\r\n\r" );
      else*/
       send_to_pager_color( "\n\r-----------------------------[ &GGROUPED CHARACTERS&g ]---------------------------\n\r\n\r", ch );
    }
    for ( cur_who = first_grouped; cur_who; cur_who = next_who )
    {
/*      if ( !ch )
        fprintf( whoout, cur_who->text );
      else*/
        send_to_pager( cur_who->text, ch );
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      DISPOSE( cur_who ); 
    }

    if (first_groupwho)
    {
      if ( !ch )
        fprintf( whoout, "\n\r-------------------------------[ WANTING GROUP ]------------------------------\n\r\n\r" );
      else
       send_to_pager_color( "\n\r-------------------------------[&G WANTING GROUP&g ]------------------------------\n\r\n\r", ch );
    }
    for ( cur_who = first_groupwho; cur_who; cur_who = next_who )
    {
      if ( !ch )
        fprintf( whoout, cur_who->text );
      else
        send_to_pager( cur_who->text, ch );
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      DISPOSE( cur_who ); 
    }

    if ( first_imm )
    {
      if ( !ch )
        fprintf( whoout, "\n\r-----------------------------------[ IMMORTALS ]-----------------------------\n\r\n\r" );
      else
       send_to_pager_color( "\n\r-----------------------------------[&G IMMORTALS&g ]------------------------------\n\r\n\r", ch );
    }

    for ( cur_who = first_imm; cur_who; cur_who = next_who )
    {
      if ( !ch )
        fprintf( whoout, cur_who->text );
      else
        send_to_pager( cur_who->text, ch );
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      DISPOSE( cur_who ); 
    } 

    if ( !ch )
    {
	fprintf( whoout, "%d player%s.\n\r", nMatch, nMatch == 1 ? "" : "s" );
	fclose( whoout );
	return;
    }

    set_char_color( AT_YELLOW, ch );
    ch_printf( ch, "%d player%s.\n\r", nMatch, nMatch == 1 ? "" : "s" );
    return;
}


void do_cwho( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char clan_name[MAX_INPUT_LENGTH];
    char council_name[MAX_INPUT_LENGTH];
    char invis_str[MAX_INPUT_LENGTH];
    char char_name[MAX_INPUT_LENGTH];
    char *extra_title;
    char class_text[MAX_INPUT_LENGTH];
    struct whogr_s *whogr, *whogr_p;
    DESCRIPTOR_DATA *d;
    int iClass, iRace;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    bool rgfClass[MAX_CLASS];
    bool rgfRace[MAX_RACE];
    bool fClassRestrict;
    bool fRaceRestrict;
    bool fImmortalOnly;
    bool fLeader;
    bool fPkill;
    bool fShowHomepage;
    bool fClanMatch; /* SB who clan (order),who guild, and who council */
    bool fCouncilMatch;
    bool fDeityMatch;
    bool fGroup;
    bool fRetired;
    CLAN_DATA *pClan = NULL;
    COUNCIL_DATA *pCouncil = NULL;
    DEITY_DATA *pDeity = NULL;
    FILE *whoout = NULL;

    /*
    #define WT_IMM    0;
    #define WT_MORTAL 1;
    #define WT_DEADLY 2;
    */

    WHO_DATA *cur_who = NULL;
    WHO_DATA *next_who = NULL;
    WHO_DATA *first_mortal = NULL;
    WHO_DATA *first_imm = NULL;
    WHO_DATA *first_deadly  = NULL;
    WHO_DATA *first_grouped = NULL;
    WHO_DATA *first_groupwho = NULL;


    /*
     * Set default arguments.
     */
    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL;
    fClassRestrict = FALSE;
    fRaceRestrict  = FALSE;
    fImmortalOnly  = FALSE;
    fPkill         = FALSE;
    fShowHomepage  = FALSE;
    fClanMatch	   = FALSE; /* SB who clan (order), who guild, who council */
    fCouncilMatch  = FALSE;
    fDeityMatch    = FALSE;
    fRetired	   = FALSE; 
   fGroup	   = FALSE; /* Alty who group */
    fLeader	   = FALSE;
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	rgfClass[iClass] = FALSE;
    for ( iRace = 0; iRace < MAX_RACE; iRace++ )
	rgfRace[iRace] = FALSE;

#ifdef REQWHOARG
    /*
     * The who command must have at least one argument because we often
     * have up to 500 players on. Too much spam if a player accidentally
     * types "who" with no arguments.           --Gorog
     */
    if ( ch && argument[0] == '\0' )
    {
	send_to_char_color( "\n\r&GYou must specify at least one argument.\n\r", ch );
	send_to_char_color( "Use 'who 1' to view the entire who list.  Other arguments:\n\r", ch );
	send_to_char_color( " ... nc = new player advisors\n\r ... deadly = deadly players\n\r ... group = grouped players\n\r", ch);
	send_to_char_color( "\n\rFor additional arguments or help, type 'help who'.\n\r", ch );
        return;
    }
#endif
	
    /*
     * Parse arguments.
     */
    nNumber = 0;
    for ( ;; )
    {
	char arg[MAX_STRING_LENGTH];

	argument = one_argument( argument, arg );
	if ( arg[0] == '\0' )
	    break;

	if ( is_number( arg ) )
	{
	    switch ( ++nNumber )
	    {
	    case 1: iLevelLower = atoi( arg ); break;
	    case 2: iLevelUpper = atoi( arg ); break;
	    default:
		send_to_char( "Only two level numbers allowed.\n\r", ch );
		return;
	    }
	}
	else
	{
	    if ( strlen(arg) < 2 )
	    {
		send_to_char( "Arguments must be longer than that.\n\r", ch );
		return;
	    }
/*
	    if ( !str_cmp( arg, "nc" ) || !str_cmp( arg, "newbie" ) || !str_cmp( arg, "advisor" ) )
	    {
		do_who( ch, "\'newbie council\'" );
		return;
	    }
*/
	    /*
	     * Look for classes to turn on.
	     */
            if ( !str_cmp( arg, "deadly" ) || !str_cmp( arg, "pkill" ) )
              fPkill = TRUE;
            else
	    if ( !str_cmp( arg, "imm" ) || !str_cmp( arg, "gods" ) )
		fImmortalOnly = TRUE;
	    else
	    if ( !str_cmp( arg, "leader" ) )
	    	fLeader = TRUE;
	    else
/*
	    if ( !str_cmp( arg, "nc" ) || !str_cmp( arg, "advisor" ) || !str_cmp( arg, "newbie" ) )
		fShowAdvisor = TRUE;
	    else
*/
	    if ( !str_cmp( arg, "www" ) )
		fShowHomepage = TRUE;
	    else
		if ( !str_cmp( arg, "retired" ) )
		fRetired = TRUE;
		else
	    if ( !str_cmp( arg, "group" ) && ch )
		fGroup = TRUE;
            else		 /* SB who clan (order), guild, council */
             if  ( ( pClan = get_clan (arg) ) )
	   	fClanMatch = TRUE;
            else
             if ( ( pCouncil = get_council (arg) ) )
                fCouncilMatch = TRUE;
	    else
	     if ( ( pDeity = get_deity (arg) ) )
		fDeityMatch = TRUE;
	    else
	    {
		for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
		{
		    if ( !str_cmp( arg, class_table[iClass]->who_name ) )
		    {
			rgfClass[iClass] = TRUE;
			break;
		    }
		}
		if ( iClass != MAX_CLASS )
		  fClassRestrict = TRUE;

		for ( iRace = 0; iRace < MAX_RACE; iRace++ )
		{
		    if ( !str_cmp( arg, race_table[iRace]->race_name ) )
		    {
			rgfRace[iRace] = TRUE;
			break;
		    }
		}
		if ( iRace != MAX_RACE )
		  fRaceRestrict = TRUE;

		if ( iClass == MAX_CLASS && iRace == MAX_RACE 
 		 && fClanMatch == FALSE 
         && fRetired == FALSE
		 && fCouncilMatch == FALSE
		 && fDeityMatch == FALSE )
		{
		 send_to_char( "That's not a class, race, order, guild,"
			" council or deity.\n\r", ch );
		    return;
		}
	    }
	}
    }

    /*
     * Now find matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    if ( ch )
	set_pager_color( AT_GREEN, ch );
    else
    {
	if ( fShowHomepage )
	  whoout = fopen( WEBWHO_FILE, "w" );
	else
	  whoout = fopen( WHO_FILE, "w" );
	if ( !whoout )
	{
	  bug( "do_who: cannot open who file!" );
	  return;
	}
    }

/* start from last to first to get it in the proper order */
    if (fGroup)
    {
      create_whogr(ch);
      whogr = first_whogr;
      d = whogr->d;
    }
    else
    {
      whogr = NULL;
      d = last_descriptor;
    }
    whogr_p = NULL;
    for ( ; d; whogr_p = whogr, whogr = (fGroup ? whogr->next : NULL),
    			d = (fGroup ? (whogr ? whogr->d : NULL) : d->prev))
    {
	CHAR_DATA *wch;
	char const *class;

	if ( (d->connected != CON_PLAYING && d->connected != CON_EDITING)
	||   !can_see( ch, d->character ) || d->original)
	    continue;
	wch   = d->original ? d->original : d->character;
	if ( wch->level < iLevelLower
	||   wch->level > iLevelUpper
	|| ( fPkill && !CAN_PKILL( wch ) )
/*
	|| ( fShowAdvisor && wch->pcdata->council && !str_cmp( "Newbie Council", wch->pcdata->council->name ) )
*/
	|| ( fRetired && !IS_RETIRED( wch ) )
	|| ( fImmortalOnly  && wch->level < LEVEL_IMMORTAL )
	|| ( fClassRestrict && !rgfClass[wch->class] )
	|| ( fRaceRestrict && !rgfRace[wch->race] )
	|| ( fClanMatch && ( pClan != wch->pcdata->clan ))  /* SB */
        || ( fCouncilMatch && ( pCouncil != wch->pcdata->council )) /* SB */ 
	|| ( fDeityMatch && ( pDeity != wch->pcdata->deity )) )
	    continue;
	if ( fLeader && !(wch->pcdata->council && 
	     ((wch->pcdata->council->head2 &&
	     !str_cmp(wch->pcdata->council->head2, wch->name)) ||
	     (wch->pcdata->council->head &&
	     !str_cmp(wch->pcdata->council->head, wch->name)))) &&
	     !(wch->pcdata->clan && ((wch->pcdata->clan->deity &&
	     !str_cmp(wch->pcdata->clan->deity,wch->name) )
	     || (wch->pcdata->clan->leader
	     && !str_cmp(wch->pcdata->clan->leader, wch->name ) )
	     || (wch->pcdata->clan->number1
	     && !str_cmp(wch->pcdata->clan->number1, wch->name ) )
	     || (wch->pcdata->clan->number2 
	     && !str_cmp(wch->pcdata->clan->number2, wch->name )))))
	     continue;

	if (fGroup && !wch->leader &&
	   !IS_SET(wch->pcdata->flags, PCFLAG_GROUPWHO) &&
	   (!whogr_p || !whogr_p->indent))
	  continue;

	nMatch++;

	if ( fShowHomepage
	&&   wch->pcdata->homepage
	&&   wch->pcdata->homepage[0] != '\0' )
	  sprintf( char_name, "<A HREF=\"%s\">%s</A>",
		show_tilde( wch->pcdata->homepage ), wch->name );
	else
	  strcpy( char_name, wch->name );

	sprintf( class_text, "%s%2d %s", NOT_AUTHED(wch) ? "N" : " ", wch->level, class_table[wch->class]->who_name );
	class = class_text;
	switch ( wch->level )
	{
	default: break;
	case MAX_LEVEL -  0: class = "Supreme Entity";	break;
	case MAX_LEVEL -  1: class = "Infinite";	break;
	case MAX_LEVEL -  2: class = "Eternal";		break;
	case MAX_LEVEL -  3: class = "Ancient";		break;
	case MAX_LEVEL -  4: class = "Exalted God";	break;
	case MAX_LEVEL -  5: class = "Ascendant God";	break;
	case MAX_LEVEL -  6: class = "Greater God";	break;
	case MAX_LEVEL -  7: class = "God";		break;
	case MAX_LEVEL -  8: class = "Lesser God";	break;
	case MAX_LEVEL -  9: class = "Immortal";	break;
	case MAX_LEVEL - 10: class = "Demi God";	break;
	case MAX_LEVEL - 11: class = "Savior";		break;
	case MAX_LEVEL - 12: class = "Creator";		break;
	case MAX_LEVEL - 13: class = "Acolyte";		break;
	case MAX_LEVEL - 14: class = "Neophyte";	break;
	case MAX_LEVEL - 15: class = "Avatar";		break;
	}

        if ( !str_cmp( wch->name, sysdata.guild_overseer ) )
          extra_title = " &c[&WOverseer of Guilds&c]&G";
        else if ( !str_cmp( wch->name, sysdata.guild_advisor ) )
          extra_title = " &c[&WAdvisor to Guilds&c]&G";
        else
          extra_title = "";
 
        if ( IS_GUEST( wch ) )
          class = "Guest";
	else if ( wch->pcdata->clan
             &&  !str_cmp( wch->name, wch->pcdata->clan->leader )
             &&   wch->pcdata->clan->leadrank[0] != '\0' )
                class = wch->pcdata->clan->leadrank;
        else if ( wch->pcdata->clan
	     &&  !str_cmp( wch->name, wch->pcdata->clan->number1 )
             &&   wch->pcdata->clan->onerank[0] != '\0' )
                class = wch->pcdata->clan->onerank;
	else if ( wch->pcdata->clan
             &&  !str_cmp( wch->name, wch->pcdata->clan->number2 )
             &&   wch->pcdata->clan->tworank[0] != '\0' )
                class = wch->pcdata->clan->tworank;
  	else if ( wch->pcdata->rank && wch->pcdata->rank[0] != '\0' )
	  class = wch->pcdata->rank;
	else if ( IS_RETIRED( wch ) )
	  class = "Retired";

	if ( wch->pcdata->clan )
	{
          CLAN_DATA *pclan = wch->pcdata->clan;
	  if ( pclan->clan_type == CLAN_GUILD )
	    strcpy( clan_name, " &c<&W" );
	  else
	    strcpy( clan_name, " &c(&W" );

          if ( pclan->clan_type == CLAN_ORDER )
          {
	    if ( !str_cmp( wch->name, pclan->deity ) )
	      strcat( clan_name, "Deity of " );
            else
	    if ( !str_cmp( wch->name, pclan->leader ) )
	      strcat( clan_name, "Leader of " );
	    else
	    if ( !str_cmp( wch->name, pclan->number1 ) )
	      strcat( clan_name, "Number One, " );
	    else
	    if ( !str_cmp( wch->name, pclan->number2 ) )
	      strcat( clan_name, "Number Two, " );
	    else
	      strcat( clan_name, "" );
          }
          else
	  if ( pclan->clan_type == CLAN_GUILD )
	  {
	    if ( !str_cmp( wch->name, pclan->leader ) )
              strcat( clan_name, "Leader, " );
            if ( !str_cmp( wch->name, pclan->number1 ) )
              strcat( clan_name, "First, " );
            if ( !str_cmp( wch->name, pclan->number2 ) )
              strcat( clan_name, "Second, " );
	  }
	  else
          {
	    if ( !str_cmp( wch->name, pclan->deity ) )
	      strcat( clan_name, "Deity of " );
	    else
	    if ( !str_cmp( wch->name, pclan->leader ) )
	      strcat( clan_name, "Leader of " );
	    else
	    if ( !str_cmp( wch->name, pclan->number1 ) )
	      strcat( clan_name, "Number One, " );
	    else
	    if ( !str_cmp( wch->name, pclan->number2 ) )
	      strcat( clan_name, "Number Two, " );
          } 
	  strcat( clan_name, pclan->name );
	  if ( pclan->clan_type == CLAN_GUILD )
	    strcat( clan_name, "&c>&G" );
	  else
	    strcat( clan_name, "&c)&G" );
	}
	else
	  clan_name[0] = '\0';

	if ( wch->pcdata->council )
	{
          strcpy( council_name, " &c[&W" );
          if (  wch->pcdata->council->head2 == NULL )
          {
             if (!str_cmp (wch->name, wch->pcdata->council->head))
                strcat (council_name, "Head of ");
          }
          else
          {
             if (!str_cmp (wch->name, wch->pcdata->council->head)
                 || !str_cmp ( wch->name, wch->pcdata->council->head2) )
                strcat (council_name, "Co-Head of ");
          }
          strcat( council_name, wch->pcdata->council_name );
          strcat( council_name, "&c]&G" );
	}
	else
          council_name[0] = '\0';

	if ( xIS_SET(wch->act, PLR_WIZINVIS) )
	  sprintf( invis_str, "&g(&G%d&g)&G ", wch->pcdata->wizinvis );
	else
	  invis_str[0] = '\0';
	sprintf( buf, "&g%*s%-15s&G %s%s%s%s%s%s%s%s.%s%s%s\n\r",
	  (fGroup ? whogr->indent : 0), "",
	  class,
	  invis_str,
          (wch->desc && wch->desc->connected) ? "[WRITING] " : "",
	  xIS_SET(wch->act, PLR_AFK) ? "[AFK] " : "",
	  xIS_SET(wch->act, PLR_ATTACKER) ? "(ATTACKER) " : "",
	  xIS_SET(wch->act, PLR_KILLER) ? "(KILLER) " : "",
	  xIS_SET(wch->act, PLR_THIEF)  ? "(THIEF) "  : "",
	  char_name,
	  wch->pcdata->title,
          extra_title,
	  clan_name,
	  council_name );

          /*  
           * This is where the old code would display the found player to the ch.
           * What we do instead is put the found data into a linked list
           */ 

          /* First make the structure. */
          CREATE( cur_who, WHO_DATA, 1 );
          cur_who->text = str_dup( buf );
          if ( wch->level > 50 && IS_IMMORTAL( wch ) )
            cur_who->type = WT_IMM;
          else if ( fGroup )
            if ( wch->leader || (whogr_p && whogr_p->indent) )
              cur_who->type = WT_GROUPED;
            else
              cur_who->type = WT_GROUPWHO;
          else if ( CAN_PKILL( wch ) ) 
            cur_who->type = WT_DEADLY;
          else
            cur_who->type = WT_MORTAL;

          /* Then put it into the appropriate list. */
          switch ( cur_who->type )
          {
            case WT_MORTAL:
              cur_who->next = first_mortal;
              first_mortal = cur_who;
              break;
            case WT_DEADLY:
              cur_who->next = first_deadly;
              first_deadly = cur_who;
              break;
            case WT_GROUPED:
              cur_who->next = first_grouped;
              first_grouped = cur_who;
              break;
            case WT_GROUPWHO:
              cur_who->next = first_groupwho;
              first_groupwho = cur_who;
              break;
            case WT_IMM:
              cur_who->next = first_imm;
              first_imm = cur_who;
              break;
          }

    }


    /* Ok, now we have three separate linked lists and what remains is to 
     * display the information and clean up.
     */
    /*
     * Two extras now for grouped and groupwho (wanting group). -- Alty
     */

    for ( cur_who = first_mortal; cur_who; cur_who = next_who )
    {
      if ( !ch )
        fprintf( whoout, cur_who->text );
      else
        send_to_pager_color( cur_who->text, ch );
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      DISPOSE( cur_who ); 
    } 

    if ( first_deadly )
    {
      if ( !ch )
        fprintf( whoout, "\n\r-------------------------------[ DEADLY CHARACTERS ]-------------------------\n\r\n\r" );
      else
       send_to_pager_color( "\n\r&g-------------------------------[&G DEADLY CHARACTERS &g]--------------------------\n\r\n\r", ch );
    }

    for ( cur_who = first_deadly; cur_who; cur_who = next_who )
    {
      if ( !ch )
        fprintf( whoout, cur_who->text );
      else
        send_to_pager_color( cur_who->text, ch );
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      DISPOSE( cur_who ); 
    }
    
    if (first_grouped)
    {
/*      if ( !ch )
        fprintf( whoout, "\n\r-----------------------------[ GROUPED CHARACTERS ]---------------------------\n\r\n\r" );
      else*/
       send_to_pager_color( "\n\r&g-----------------------------[ GROUPED CHARACTERS ]---------------------------\n\r\n\r", ch );
    }
    for ( cur_who = first_grouped; cur_who; cur_who = next_who )
    {
/*      if ( !ch )
        fprintf( whoout, cur_who->text );
      else*/
        send_to_pager_color( cur_who->text, ch );
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      DISPOSE( cur_who ); 
    }

    if (first_groupwho)
    {
      if ( !ch )
        fprintf( whoout, "\n\r-------------------------------[ WANTING GROUP ]------------------------------\n\r\n\r" );
      else
       send_to_pager_color( "\n\r&g-------------------------------[ WANTING GROUP ]------------------------------\n\r\n\r", ch );
    }
    for ( cur_who = first_groupwho; cur_who; cur_who = next_who )
    {
      if ( !ch )
        fprintf( whoout, cur_who->text );
      else
        send_to_pager_color( cur_who->text, ch );
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      DISPOSE( cur_who ); 
    }

    if ( first_imm )
    {
      if ( !ch )
        fprintf( whoout, "\n\r-----------------------------------[ IMMORTALS ]-----------------------------\n\r\n\r" );
      else
       send_to_pager_color( "\n\r&g-----------------------------------[&G IMMORTALS&g ]------------------------------\n\r\n\r", ch );
    }

    for ( cur_who = first_imm; cur_who; cur_who = next_who )
    {
      if ( !ch )
        fprintf( whoout, cur_who->text );
      else
        send_to_pager_color( cur_who->text, ch );
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      DISPOSE( cur_who ); 
    } 

    if ( !ch )
    {
	fprintf( whoout, "%d player%s.\n\r", nMatch, nMatch == 1 ? "" : "s" );
	fclose( whoout );
	return;
    }

    set_char_color( AT_NOTE, ch );
    ch_printf( ch, "%d player%s.\n\r", nMatch, nMatch == 1 ? "" : "s" );
    return;
}


void do_compare( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Compare what to what?\n\r", ch );
	return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	for ( obj2 = ch->first_carrying; obj2; obj2 = obj2->next_content )
	{
	    if ( obj2->wear_loc != WEAR_NONE
	    &&   can_see_obj( ch, obj2 )
	    &&   obj1->item_type == obj2->item_type
	    && ( obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
		break;
	}

	if ( !obj2 )
	{
	    send_to_char( "You aren't wearing anything comparable.\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( obj2 = get_obj_carry( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not have that item.\n\r", ch );
	    return;
	}
    }

    msg		= NULL;
    value1	= 0;
    value2	= 0;

    if ( obj1 == obj2 )
    {
	msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
	msg = "You can't compare $p and $P.";
    }
    else
    {
	switch ( obj1->item_type )
	{
	default:
	    msg = "You can't compare $p and $P.";
	    break;

	case ITEM_ARMOR:
	    value1 = obj1->value[0];
	    value2 = obj2->value[0];
	    break;

	case ITEM_WEAPON:
	    value1 = obj1->value[1] + obj1->value[2];
	    value2 = obj2->value[1] + obj2->value[2];
	    break;
	}
    }

    if ( !msg )
    {
	     if ( value1 == value2 ) msg = "$p and $P look about the same.";
	else if ( value1  > value2 ) msg = "$p looks better than $P.";
	else                         msg = "$p looks worse than $P.";
    }

    act( AT_PLAIN, msg, ch, obj1, obj2, TO_CHAR );
    return;
}

void do_where( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found;
    bool PK_WHERE = FALSE;
	
    one_argument( argument, arg );

    if ( arg[0]!='\0'
    && ( victim = get_char_world( ch, arg ) )
    &&  !IS_NPC( victim )
    &&   IS_SET( victim->pcdata->flags, PCFLAG_DND )
    &&   get_trust( ch ) < get_trust( victim ) )
    {
	pager_printf_color( ch, "&GYou didn't find any %s.\n\r",
	  capitalize( argument ) );
	return;
    }

    if ( !str_cmp( arg, "pk" ) )
	PK_WHERE = TRUE;

    set_pager_color( AT_GREEN, ch );

    if ( arg[0] == '\0' || PK_WHERE )
    {
	pager_printf_color( ch, "\n\r&G&gPlayers near you in %s:\n\r", ch->in_room->area->name );
	found = FALSE;
	for ( d = first_descriptor; d; d = d->next )
        {
	    if ( ( d->connected == CON_PLAYING || d->connected == CON_EDITING )
	    &&   ( victim = d->character ) != NULL
	    &&    !IS_NPC( victim )
	    &&     victim->in_room
	    &&   ( ( victim->in_room->area == ch->in_room->area )
			  && ( !xIS_SET( victim->in_room->room_flags, ROOM_HOUSE ) 
			  || in_same_house( victim, ch ) ) )
	    &&     can_see( ch, victim )
	    &&	 ( victim->in_room == ch->in_room 
		  	  || IS_IMMORTAL( ch ) 
	          || ( !IS_SET( ch->in_room->area->flags, AFLAG_NOWHERE) 
			  && !xIS_SET( victim->in_room->room_flags, ROOM_NOWHERE ) ) )
        &&   ( get_trust( ch ) >= get_trust( victim )
              ||    !IS_SET( victim->pcdata->flags, PCFLAG_DND ) ) )
                /* if victim has the DND flag ch must outrank them */ 
	    {
		found = TRUE;
		if ( PK_WHERE && !IS_PKILL( victim ) )
		  continue;
		pager_printf_color( ch, "&g| &G%-13s&w  ", victim->name );
		if ( IS_IMMORTAL( victim ) && victim->level > 50
		&&   victim->level <= 65 )
		{
		  if ( imm_badge[MAX_LEVEL-victim->level] )
		  {
		    sprintf( buf, "%-25.25s\t",
			imm_badge[MAX_LEVEL-victim->level] );
		    send_to_pager_color( buf, ch );
		  }
		  else
		  {
		    send_to_pager_color( "&W(Immortal)\t\t", ch );
		  }
		}
		else if ( CAN_PKILL( victim )
		     &&   victim->pcdata->clan
		     &&   victim->pcdata->clan->clan_type != CLAN_ORDER
		     &&   victim->pcdata->clan->clan_type != CLAN_GUILD )
		     {
			int len = colorlen( victim->pcdata->clan->badge, 18);
			pager_printf_color( ch, "&G%-*.*s\t", len, len, victim->pcdata->clan->badge );
		     }
		else if ( CAN_PKILL( victim ) )
		  send_to_pager_color( "&W(Unclanned)\t\t", ch );
		else
		  send_to_pager( "\t\t\t", ch );
		pager_printf_color( ch, "&g| &w%s&w\n\r",
		  victim->in_room->name );
	    }
	}
	if ( !found )
	  send_to_char_color( "&GNone\n\r", ch );
    }
    else
    {
	found = FALSE;
	for ( victim = first_char; victim; victim = victim->next )
        {
	  if ( victim->in_room
	  &&   victim->in_room->area == ch->in_room->area
	  &&  !IS_AFFECTED( victim, AFF_HIDE )
	  &&  (( !IS_SET( ch->in_room->area->flags, AFLAG_NOWHERE )
		&& !xIS_SET( victim->in_room->room_flags, ROOM_NOWHERE ) )
	    || IS_IMMORTAL( ch )
	    || ch->in_room == victim->in_room )
	  &&  !IS_AFFECTED( victim, AFF_SNEAK )
	  &&   can_see( ch, victim )
	  &&   is_name( arg, victim->name ) )
	  {
		found = TRUE;
		pager_printf_color( ch, "&g| &G%s is currently at &w%s&w\n\r",
		  PERS( victim, ch ), victim->in_room->name );
		break;
	  }
	}
	if ( !found )
	  pager_printf_color( ch, "You didn't find any %s.\n\r",
	    capitalize( argument ) );
    }
    return;
}

void do_consider( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char *msg;
    int diff;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Consider killing whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They're not here.\n\r", ch );
	return;
    }
    if ( victim == ch )
    {
	send_to_char( "You decide you're pretty sure you could take yourself in a fight.\n\r", ch );
	return;
    }
    diff = victim->level - ch->level;

	 if ( diff <= -10 ) msg = "You are far more experienced than $N.";
    else if ( diff <=  -5 ) msg = "$N is not nearly as experienced as you.";
    else if ( diff <=  -2 ) msg = "You are more experienced than $N.";
    else if ( diff <=   1 ) msg = "You are just about as experienced as $N.";
    else if ( diff <=   4 ) msg = "You are not nearly as experienced as $N.";
    else if ( diff <=   9 ) msg = "$N is far more experienced than you!";
    else                    msg = "$N would make a great teacher for you!";
    act( AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR );

    diff = (int) (victim->max_hit - ch->max_hit) / 6;

	 if ( diff <= -200) msg = "$N looks like a feather!";
    else if ( diff <= -150) msg = "You could kill $N with your hands tied!";
    else if ( diff <= -100) msg = "Hey! Where'd $N go?";
    else if ( diff <=  -50) msg = "$N is a wimp.";
    else if ( diff <=    0) msg = "$N looks weaker than you.";
    else if ( diff <=   50) msg = "$N looks about as strong as you.";
    else if ( diff <=  100) msg = "It would take a bit of luck...";
    else if ( diff <=  150) msg = "It would take a lot of luck, and equipment!";
    else if ( diff <=  200) msg = "Why don't you dig a grave for yourself first?";
    else                    msg = "$N is built like a TANK!";
    act( AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR );

    return;
}



/*
 * Place any skill types you don't want them to be able to practice
 * normally in this list.  Separate each with a space.
 * (Uses an is_name check). -- Altrag
 */
#define CANT_PRAC "Tongue"

void do_practice( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int sn;

    if ( IS_NPC(ch) )
	return;

/*
    if ( ch->level < 2 )
    {
	send_to_char(
	"You must be second level to practice.  Seek out monsters to kill!\n\r",
	    ch );
	return;
    }
*/
    if ( argument[0] == '\0' )
    {
	int	col;
	sh_int	lasttype, cnt;

	col = cnt = 0;	lasttype = SKILL_SPELL;
	set_pager_color( AT_MAGIC, ch );
	for ( sn = 0; sn < top_sn; sn++ )
	{
	    if ( !skill_table[sn]->name )
		break;


	    if ( strcmp(skill_table[sn]->name, "reserved") == 0
	    && ( IS_IMMORTAL(ch) || CAN_CAST(ch) ) )
	    {
		if ( col % 3 != 0 )
		    send_to_pager( "\n\r", ch );
	      set_pager_color( AT_MAGIC, ch );
	      send_to_pager_color(
" ----------------------------------[&CSpells&B]----------------------------------\n\r", ch);
		col = 0;
	    }
	    if ( skill_table[sn]->type != lasttype )
	    {
		if ( !cnt )
		    send_to_pager( "                                   (none)\n\r", ch );
		else
		if ( col % 3 != 0 )
		    send_to_pager( "\n\r", ch );
		set_pager_color( AT_MAGIC, ch );
		pager_printf_color( ch,
" ----------------------------------&C%ss&B----------------------------------\n\r",
			 skill_tname[skill_table[sn]->type]);
		col = cnt = 0;
	    }
	    lasttype = skill_table[sn]->type;

/* Pkill only skill */
	    if ( skill_table[sn]->guild != -1 && !IS_IMMORTAL( ch ) )
	    {
		if ( skill_table[sn]->guild == 99
		&&  !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
		  continue;
		else if ( skill_table[sn]->guild != 99
		     &&   ( !IS_GUILDED( ch )
		     ||   (  ch->pcdata->clan->class != skill_table[sn]->guild ) ) )
			continue;
	    }

		if ( sysdata.magichell  && ch->level > 35 && number_percent() < 30 )
			continue;

	    if ( ch->level < skill_table[sn]->skill_level[ch->class]
	    || (!IS_IMMORTAL(ch) && skill_table[sn]->skill_level[ch->class] == 0) )
 	        continue;

	    if ( ch->pcdata->learned[sn] <= 0
	    &&   SPELL_FLAG(skill_table[sn], SF_SECRETSKILL) )
		continue;

	    ++cnt;
	    set_pager_color( AT_MAGIC, ch );
	    pager_printf( ch, "%20.20s", skill_table[sn]->name );
	    if ( ch->pcdata->learned[sn] > 0 )
		set_pager_color( AT_SCORE, ch );
	    pager_printf( ch, " %3d%% ", ch->pcdata->learned[sn] );
	    if ( ++col % 3 == 0 )
		send_to_pager( "\n\r", ch );
	}

	if ( col % 3 != 0 )
	    send_to_pager( "\n\r", ch );
        set_pager_color( AT_MAGIC, ch );
	pager_printf( ch, "You have %d practice sessions left.\n\r",
	    ch->practice );
    }
    else
    {
	CHAR_DATA *mob;
	char arg[MAX_INPUT_LENGTH];
	int adept;
	bool can_prac = TRUE;

	if ( !IS_AWAKE(ch) )
	{
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    return;
	}

	for ( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
	    if ( IS_NPC(mob) && xIS_SET(mob->act, ACT_PRACTICE) )
		break;

	if ( !mob )
	{
	    send_to_char( "You can't do that here.\n\r", ch );
	    return;
	}

	if ( ch->practice <= 0 )
	{
	    act( AT_TELL, "$n tells you 'You must earn some more practice sessions.'",
		mob, NULL, ch, TO_VICT );
	    return;
	}
	
	argument = one_argument(argument, arg);
	sn = skill_lookup( arg );

	if ( can_prac 
          && ( ( sn == -1 )
       	       || ( !IS_NPC(ch)
	            &&  ch->level < skill_table[sn]->skill_level[ch->class] 
/* OUT FOR THIS PORT -SHADDAI
                    &&  ch->level < skill_table[sn]->race_level[ch->race]  
*/
                  ) 
             ) 
        )
	{
	    act( AT_TELL, "$n tells you 'You're not ready to learn that yet...'",
		mob, NULL, ch, TO_VICT );
	    return;
	}

	if ( is_name( skill_tname[skill_table[sn]->type], CANT_PRAC ) )
	{
	    act( AT_TELL, "$n tells you 'I do not know how to teach that.'",  
		  mob, NULL, ch, TO_VICT );
	    return;
	}

	/*
	 * Skill requires a special teacher
	 */
	if ( skill_table[sn]->teachers && skill_table[sn]->teachers[0] != '\0' )
	{
	    sprintf( buf, "%d", mob->pIndexData->vnum );
	    if ( !is_name( buf, skill_table[sn]->teachers ) )
	    {
		act( AT_TELL, "$n tells you, 'I know not know how to teach that.'",
		    mob, NULL, ch, TO_VICT );
		return;
	    }
	}

/*
 * Guild checks - right now, cant practice guild skills - done on 
 * induct/outcast
 */
/*	
	if ( !IS_NPC(ch) && !IS_GUILDED(ch)
	&&    skill_table[sn]->guild != CLASS_NONE)
	{
	    act( AT_TELL, "$n tells you 'Only guild members can use that..'"
		mob, NULL, ch, TO_VICT );
	    return;
	}
	{
	    act( AT_TELL, "$n tells you 'That can not be used by your guild.'"
		mob, NULL, ch, TO_VICT );
	    return;
	}
*/
	if ( !IS_NPC(ch) && skill_table[sn]->guild != CLASS_NONE
	&&    skill_table[sn]->guild != 99 )
	{
	    act( AT_TELL, "$n tells you 'That is only for members of guilds...'",
		mob, NULL, ch, TO_VICT );
	    return;
	}

	if ( !IS_NPC( ch ) && skill_table[sn]->guild == 99 && !IS_PKILL( ch ) )
	{
	    act( AT_TELL, "$n tells you 'Only deadlies may use that skill ...'", mob, NULL, ch, TO_VICT );
	    return;
        }
	/*
	 * Disabled for now
	if ( mob->level < skill_table[sn]->skill_level[ch->class]
	||   mob->level < skill_table[sn]->skill_level[mob->class] )
	{
	    act( AT_TELL, "$n tells you 'You must seek another to teach you that...'",
		mob, NULL, ch, TO_VICT );
	    return;
	}
	 */

	adept = class_table[ch->class]->skill_adept * 0.2;

	if ( ch->pcdata->learned[sn] >= adept )
	{
	    sprintf( buf, "$n tells you, 'I've taught you everything I can about %s.'",
		skill_table[sn]->name );
	    act( AT_TELL, buf, mob, NULL, ch, TO_VICT );
	    act( AT_TELL, "$n tells you, 'You'll have to practice it on your own now...'",
		mob, NULL, ch, TO_VICT );
	}
	else
	{
	    ch->practice--;
	    ch->pcdata->learned[sn] += int_app[get_curr_int(ch)].learn;
	    act( AT_ACTION, "You practice $T.",
		    ch, NULL, skill_table[sn]->name, TO_CHAR );
	    act( AT_ACTION, "$n practices $T.",
		    ch, NULL, skill_table[sn]->name, TO_ROOM );
	    if ( ch->pcdata->learned[sn] >= adept )
	    {
		ch->pcdata->learned[sn] = adept;
		act( AT_TELL,
		 "$n tells you. 'You'll have to practice it on your own now...'",
		 mob, NULL, ch, TO_VICT );
	    }
	}
    }
    return;
}


void do_wimpy( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int wimpy;

    set_char_color( AT_YELLOW, ch );
    one_argument( argument, arg );
    if ( !str_cmp( arg, "max" ) )
    {
      if ( IS_PKILL( ch ) )
	wimpy = (int) ch->max_hit / 2.25;
      else
	wimpy = (int) ch->max_hit / 1.2;
    }
    else    
    if ( arg[0] == '\0' )
      wimpy = (int) ch->max_hit / 5;
    else
      wimpy = atoi( arg );

    if ( wimpy < 0 ) {
	send_to_char( "Your courage exceeds your wisdom.\n\r", ch );
	return;
    }
    if ( IS_PKILL( ch ) && wimpy > (int) ch->max_hit / 2.25 )
    {
	send_to_char( "Such cowardice ill becomes you.\n\r", ch );
	return;
    }
    else if ( wimpy > (int) ch->max_hit / 1.2 )
    {
	send_to_char( "Such cowardice ill becomes you.\n\r", ch );
	return;
    }
    ch->wimpy	= wimpy;
    ch_printf( ch, "Wimpy set to %d hit points.\n\r", wimpy );
    return;
}



void do_password( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char log_buf[MAX_STRING_LENGTH];
  /*char *pArg;*/
    char *pwdnew;
    char *p;
  /*char cEnd;*/

    if ( IS_NPC(ch) )
	return;

#if 0
    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';
#endif

    argument = case_argument( argument, arg1 );
    argument = case_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: password <new> <again>.\n\r", ch );
	send_to_char( "Syntax: password <new> <again>.\n\r", ch );
	return;
    }

/*
    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
	WAIT_STATE( ch, 40 );
	send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
	return;
    }
*/

/* This should stop all the mistyped password problems --Shaddai */
    if ( strcmp( arg1, arg2 ))
    {
	send_to_char("Passwords don't match try again.\n\r", ch );
	return;
    }
    if ( strlen(arg2) < 5 )
    {
	send_to_char(
	    "New password must be at least five characters long.\n\r", ch );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
	    send_to_char(
		"New password not acceptable, try again.\n\r", ch );
	    return;
	}
    }

    DISPOSE( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    if ( IS_SET(sysdata.save_flags, SV_PASSCHG) )
	save_char_obj( ch );
    if ( ch->desc && ch->desc->host[0]  != '\0' )
       sprintf(log_buf, "%s changing password from site %s\n", ch->name, 
       		ch->desc->host );
    else
       sprintf(log_buf, "%s changing thier password with no descriptor!", ch->name);
    log_string( log_buf );
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_socials( CHAR_DATA *ch, char *argument )
{
    int iHash;
    int col = 0;
    SOCIALTYPE *social;

    set_pager_color( AT_PLAIN, ch );
    for ( iHash = 0; iHash < 27; iHash++ )
	for ( social = social_index[iHash]; social; social = social->next )
	{
	    pager_printf( ch, "%-12s", social->name );
	    if ( ++col % 6 == 0 )
		send_to_pager( "\n\r", ch );
	}

    if ( col % 6 != 0 )
	send_to_pager( "\n\r", ch );
    return;
}


void do_commands( CHAR_DATA *ch, char *argument )
{
    int col;
    bool found;
    int hash;
    CMDTYPE *command;

    col = 0;
    set_pager_color( AT_PLAIN, ch );
    if ( argument[0] == '\0' )
    {
	for ( hash = 0; hash < 126; hash++ )
	    for ( command = command_hash[hash]; command; command = command->next )
		if ( command->level <  LEVEL_HERO
		&&   command->level <= get_trust( ch )
		&&  (command->name[0] != 'm'
		||   command->name[1] != 'p') )
		{
		    pager_printf( ch, "%-12s", command->name );
		    if ( ++col % 6 == 0 )
			send_to_pager( "\n\r", ch );
		}
	if ( col % 6 != 0 )
	    send_to_pager( "\n\r", ch );
    }
    else
    {
	found = FALSE;
	for ( hash = 0; hash < 126; hash++ )
	    for ( command = command_hash[hash]; command; command = command->next )
		if ( command->level <  LEVEL_HERO
		&&   command->level <= get_trust( ch )
		&&  !str_prefix(argument, command->name)
		&&  (command->name[0] != 'm'
		||   command->name[1] != 'p') )
		{
		    pager_printf( ch, "%-12s", command->name );
		    found = TRUE;
		    if ( ++col % 6 == 0 )
			send_to_pager( "\n\r", ch );
		}

	if ( col % 6 != 0 )
	    send_to_pager( "\n\r", ch );
	if ( !found )
	    ch_printf( ch, "No command found under %s.\n\r", argument);
    }
    return;
}

void do_channels( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    one_argument( argument, arg );

    if ( IS_NPC( ch ) )
	return;

    if ( arg[0] == '\0' )
    {
        if ( !IS_NPC(ch) && xIS_SET(ch->act, PLR_SILENCE) )
        {
	    set_char_color( AT_GREEN, ch );
            send_to_char( "You are silenced.\n\r", ch );
            return;
        }
    
	/* Channels everyone sees regardless of affiliation --Blodkai */
        send_to_char_color( "\n\r &gPublic channels  (severe penalties for abuse)&G:\n\r  ", ch );
	ch_printf_color( ch, "%s",   !xIS_SET( ch->deaf, CHANNEL_RACETALK )?
						" &G+RACETALK" :
						" &g-racetalk" );
        ch_printf_color( ch, "%s",   !xIS_SET( ch->deaf, CHANNEL_CHAT )    ?
				                " &G+CHAT" 	          :
						" &g-chat" );
        if ( get_trust( ch ) > 4 && !NOT_AUTHED( ch ) )
          ch_printf_color( ch, "%s", !xIS_SET( ch->deaf, CHANNEL_AUCTION ) ?
				                " &G+AUCTION" 	          :
						" &g-auction" );
	ch_printf_color( ch, "%s",   !xIS_SET( ch->deaf, CHANNEL_TRAFFIC ) ?
						" &G+TRAFFIC"		  :
						" &g-traffic" );
        ch_printf_color( ch, "%s",   !xIS_SET( ch->deaf, CHANNEL_QUEST )   ?
				                " &G+QUEST" 	          :
						" &g-quest" );
        ch_printf_color( ch, "%s",   !xIS_SET( ch->deaf, CHANNEL_WARTALK ) ?
				                " &G+WARTALK" 	          :
						" &g-wartalk" );
        if ( IS_HERO( ch ) )
          ch_printf_color( ch, "%s", !xIS_SET( ch->deaf, CHANNEL_AVTALK )  ?
				                " &G+AVATAR"	          :
						" &g-avatar" );
        ch_printf_color( ch, "%s",   !xIS_SET( ch->deaf, CHANNEL_MUSIC )   ?
				                " &G+MUSIC" 	          :
						" &g-music" );
        ch_printf_color( ch, "%s",   !xIS_SET( ch->deaf, CHANNEL_ASK )     ?
					        " &G+ASK"                 :
						" &g-ask" );
        ch_printf_color( ch, "%s",   !xIS_SET( ch->deaf, CHANNEL_YELL )    ?
				                " &G+YELL"	          :
						" &g-yell" );
  
	/* For organization channels (orders, clans, guilds, councils) */
        send_to_char_color( "\n\r &gPrivate channels (severe penalties for abuse)&G:\n\r ", ch );
        ch_printf_color( ch, "%s",   !xIS_SET( ch->deaf, CHANNEL_TELLS )   ?
                                                " &G+TELLS"               :
                                                " &g-tells" );
        ch_printf_color( ch, "%s", !xIS_SET( ch->deaf, CHANNEL_WHISPER )   ?
						" &G+WHISPER"		  :
						" &g-whisper" );
        if ( !IS_NPC( ch ) && ch->pcdata->clan )
        {
          if ( ch->pcdata->clan->clan_type == CLAN_ORDER )
	    send_to_char_color( !xIS_SET( ch->deaf, CHANNEL_ORDER ) ?
		" &G+ORDER"	:	" &g-order", ch );

          else if ( ch->pcdata->clan->clan_type == CLAN_GUILD )
	    send_to_char_color( !xIS_SET( ch->deaf, CHANNEL_GUILD ) ?
		" &G+GUILD"	:	" &g-guild", ch );
          else
	    send_to_char_color( !xIS_SET( ch->deaf, CHANNEL_CLAN )  ?
		" &G+CLAN"	:	" &g-clan", ch );
        }
        if ( IS_IMMORTAL(ch) || ( ch->pcdata->council
	&&   !str_cmp( ch->pcdata->council->name, "Newbie Council" ) ) )
          ch_printf_color( ch, "%s",   !xIS_SET( ch->deaf, CHANNEL_NEWBIE) ?
    				 	        " &G+NEWBIE"	     	  :
						" &g-newbie" );
        if ( !IS_NPC( ch ) && ch->pcdata->council )
          ch_printf_color( ch, "%s",   !xIS_SET( ch->deaf, CHANNEL_COUNCIL)?
				                " &G+COUNCIL"	     	  :
						" &g-council" );
	if ( !IS_NPC( ch ) && IS_SET(ch->pcdata->flags, PCFLAG_RETIRED) )
	  ch_printf_color( ch, "%s",   !xIS_SET( ch->deaf, CHANNEL_RETIRED)?
	  					" &G+RETIRED"		  :
						" &g-retired" );

	/* Immortal channels */
        if ( IS_IMMORTAL( ch ) )
        {
            send_to_char_color( "\n\r &gImmortal Channels&G:\n\r  ", ch );
            send_to_char_color( !xIS_SET( ch->deaf, CHANNEL_IMMTALK )    ?
		" &G+IMMTALK"	:	" &g-immtalk", ch );
/*          send_to_char_color( !xIS_SET( ch->deaf, CHANNEL_PRAY )       ?
		" &G+PRAY"	:	" &g-pray", ch ); */
            if ( get_trust( ch ) >= sysdata.muse_level )
              send_to_char_color( !xIS_SET( ch->deaf, CHANNEL_HIGHGOD )  ?
		" &G+MUSE"	:	" &g-muse", ch );
            send_to_char_color( !xIS_SET( ch->deaf, CHANNEL_MONITOR )    ?
		" &G+MONITOR"	:	" &g-monitor", ch );
            send_to_char_color( !xIS_SET( ch->deaf, CHANNEL_DEATH )      ?
               " &G+DEATH"      :       " &g-death", ch );      
	    send_to_char_color( !xIS_SET( ch->deaf, CHANNEL_AUTH )	?
	       " &G+AUTH"	:	" &g-auth", ch );	
	    send_to_char_color( !xIS_SET( ch->deaf, CHANNEL_RETIRED)     ?
	       " &G+RETIRED"	:	" &g-retired", ch );
        }
        if ( get_trust( ch ) >= sysdata.log_level )
        {
            send_to_char_color( !xIS_SET( ch->deaf, CHANNEL_LOG ) 	?
		" &G+LOG"	:	" &g-log", ch);
            send_to_char_color( !xIS_SET( ch->deaf, CHANNEL_BUILD)       ?
		" &G+BUILD"	:	" &g-build", ch );
            send_to_char_color( !xIS_SET( ch->deaf, CHANNEL_COMM ) 	?
		" &G+COMM"	:	" &g-comm", ch );
            send_to_char_color( !xIS_SET (ch->deaf, CHANNEL_WARN)	?
		" &G+WARN"	:	" &g-warn", ch);
/*
	    send_to_char_color( !xIS_SET(ch->deaf, CHANNEL_BUG)		?
		" &G+BUG"	:	" &g-bug", ch);
*/
            if ( get_trust( ch ) >= sysdata.think_level )
              send_to_char_color( !xIS_SET( ch->deaf, CHANNEL_HIGH ) 	?
		" &G+HIGH"	:	" &g-high", ch );
	    if ( get_trust( ch ) >= 57 )
	      send_to_char_color( !xIS_SET( ch->deaf, CHANNEL_BUG )	?
		" &G+BUG"	:	" &g-bug", ch );
        }
        send_to_char( "\n\r", ch );   
    }
    else
    {
	bool fClear;
	bool ClearAll;
	int bit;

        bit=0;
        ClearAll = FALSE;

	     if ( arg[0] == '+' ) fClear = TRUE;
	else if ( arg[0] == '-' ) fClear = FALSE;
	else
	{
	    send_to_char( "Channels -channel or +channel?\n\r", ch );
	    return;
	}

	     if ( !str_cmp( arg+1, "auction"  ) ) bit = CHANNEL_AUCTION;
	else if ( !str_cmp( arg+1, "traffic"  ) ) bit = CHANNEL_TRAFFIC;
	else if ( !str_cmp( arg+1, "chat"     ) ) bit = CHANNEL_CHAT;
	else if ( !str_cmp( arg+1, "clan"     ) ) bit = CHANNEL_CLAN;
	else if ( !str_cmp( arg+1, "council"  ) ) bit = CHANNEL_COUNCIL;
        else if ( !str_cmp( arg+1, "guild"    ) ) bit = CHANNEL_GUILD;  
	else if ( !str_cmp( arg+1, "quest"    ) ) bit = CHANNEL_QUEST;
	else if ( !str_cmp( arg+1, "tells"    ) ) bit = CHANNEL_TELLS;
	else if ( !str_cmp( arg+1, "immtalk"  ) ) bit = CHANNEL_IMMTALK;
	else if ( !str_cmp( arg+1, "log"      ) ) bit = CHANNEL_LOG;
	else if ( !str_cmp( arg+1, "build"    ) ) bit = CHANNEL_BUILD;
	else if ( !str_cmp( arg+1, "high"     ) ) bit = CHANNEL_HIGH;
	else if ( !str_cmp( arg+1, "pray"     ) ) bit = CHANNEL_PRAY;
	else if ( !str_cmp( arg+1, "avatar"   ) ) bit = CHANNEL_AVTALK;
	else if ( !str_cmp( arg+1, "monitor"  ) ) bit = CHANNEL_MONITOR;
        else if ( !str_cmp( arg+1, "death"    ) ) bit = CHANNEL_DEATH;
	else if ( !str_cmp( arg+1, "auth"     ) ) bit = CHANNEL_AUTH;
	else if ( !str_cmp( arg+1, "newbie"   ) ) bit = CHANNEL_NEWBIE;
	else if ( !str_cmp( arg+1, "music"    ) ) bit = CHANNEL_MUSIC;
	else if ( !str_cmp( arg+1, "muse"     ) ) bit = CHANNEL_HIGHGOD;
	else if ( !str_cmp( arg+1, "ask"      ) ) bit = CHANNEL_ASK;
	else if ( !str_cmp( arg+1, "yell"     ) ) bit = CHANNEL_YELL;
	else if ( !str_cmp( arg+1, "comm"     ) ) bit = CHANNEL_COMM;
        else if ( !str_cmp (arg+1, "warn"     ) ) bit = CHANNEL_WARN;
	else if ( !str_cmp( arg+1, "bug"      ) ) bit = CHANNEL_BUG;
	else if ( !str_cmp( arg+1, "order"    ) ) bit = CHANNEL_ORDER;
        else if ( !str_cmp( arg+1, "wartalk"  ) ) bit = CHANNEL_WARTALK;
	else if ( !str_cmp( arg+1, "whisper"  ) ) bit = CHANNEL_WHISPER;
	else if ( !str_cmp( arg+1, "racetalk" ) ) bit = CHANNEL_RACETALK;
	else if ( !str_cmp( arg+1, "retired"  ) ) bit = CHANNEL_RETIRED;
	else if ( !str_cmp( arg+1, "all"      ) ) ClearAll = TRUE;
	else
	{
	    send_to_char( "Set or clear which channel?\n\r", ch );
	    return;
	}

	if (( fClear ) && ( ClearAll ))
	{
	    xREMOVE_BIT (ch->deaf, CHANNEL_RACETALK);
            xREMOVE_BIT (ch->deaf, CHANNEL_AUCTION);
            xREMOVE_BIT (ch->deaf, CHANNEL_CHAT);
            xREMOVE_BIT (ch->deaf, CHANNEL_QUEST);
            xREMOVE_BIT (ch->deaf, CHANNEL_WARTALK);
            xREMOVE_BIT (ch->deaf, CHANNEL_PRAY);
	    xREMOVE_BIT (ch->deaf, CHANNEL_TRAFFIC);
            xREMOVE_BIT (ch->deaf, CHANNEL_MUSIC);
            xREMOVE_BIT (ch->deaf, CHANNEL_ASK);
            xREMOVE_BIT (ch->deaf, CHANNEL_YELL);

       /*     if (ch->pcdata->clan)
              xREMOVE_BIT (ch->deaf, CHANNEL_CLAN);

	    if (ch->pcdata->council)
	      xREMOVE_BIT (ch->deaf, CHANNEL_COUNCIL);

            if (ch->pcdata->guild)
              xREMOVE_BIT (ch->deaf, CHANNEL_GUILD);
       */
            if (ch->level >= LEVEL_IMMORTAL)
              xREMOVE_BIT (ch->deaf, CHANNEL_AVTALK);
 
	    /*
	    if (ch->level >= sysdata.log_level )
	      xREMOVE_BIT (ch->deaf, CHANNEL_COMM);
	    */

        } else if ((!fClear) && (ClearAll))
        {
	    xSET_BIT (ch->deaf, CHANNEL_RACETALK);
            xSET_BIT (ch->deaf, CHANNEL_AUCTION);
	    xSET_BIT (ch->deaf, CHANNEL_TRAFFIC);
            xSET_BIT (ch->deaf, CHANNEL_CHAT);
            xSET_BIT (ch->deaf, CHANNEL_QUEST);
            xSET_BIT (ch->deaf, CHANNEL_PRAY);
            xSET_BIT (ch->deaf, CHANNEL_MUSIC);
            xSET_BIT (ch->deaf, CHANNEL_ASK);
	    xSET_BIT (ch->deaf, CHANNEL_WARTALK);
            xSET_BIT (ch->deaf, CHANNEL_YELL);
          
       /*     if (ch->pcdata->clan)
              xSET_BIT (ch->deaf, CHANNEL_CLAN);

	    if (ch->pcdata->council)
	      xSET_BIT (ch->deaf, CHANNEL_COUNCIL);

            if ( IS_GUILDED(ch) )
              xSET_BIT (ch->deaf, CHANNEL_GUILD);
       */
            if (ch->level >= LEVEL_IMMORTAL)
              xSET_BIT (ch->deaf, CHANNEL_AVTALK);

	    /*
	    if (ch->level >= sysdata.log_level)
	      xSET_BIT (ch->deaf, CHANNEL_COMM);
	    */

         } else if (fClear)
         {
	    xREMOVE_BIT (ch->deaf, bit);
         } else
         {
	    xSET_BIT    (ch->deaf, bit);
         }

	  send_to_char( "Ok.\n\r", ch );
    }

    return;
}


/*
 * display RETRIEDLIST file		-Shaddai
 */
void do_retiredlist( CHAR_DATA *ch, char *argument )
{
    set_pager_color( AT_IMMORT, ch );
    show_file( ch, RETIREDLIST_FILE );
}

/*
 * display WIZLIST file						-Thoric
 */
void do_wizlist( CHAR_DATA *ch, char *argument )
{
    set_pager_color( AT_IMMORT, ch );
    show_file( ch, WIZLIST_FILE );
}

/*
 * Contributed by Grodyn.
 * Display completely overhauled, 2/97 -- Blodkai
 */
void do_config( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
 
    if ( IS_NPC(ch) )
        return;
 
    one_argument( argument, arg );
 
    set_char_color( AT_GREEN, ch );
 
    if ( arg[0] == '\0' )
    {
      set_char_color( AT_DGREEN, ch );
      send_to_char( "\n\rConfigurations ", ch );
      set_char_color( AT_GREEN, ch );
      send_to_char( "(use 'config +/- <keyword>' to toggle, see 'help config')\n\r\n\r", ch );
      set_char_color( AT_DGREEN, ch );
      send_to_char( "Display:   ", ch );
      set_char_color( AT_GREY, ch );
      ch_printf( ch, "%-12s   %-12s   %-12s   %-12s\n\r           %-12s   %-12s   %-12s   %-12s\n\r           %-12s",
        IS_SET( ch->pcdata->flags, PCFLAG_PAGERON ) 	? "[+] PAGER"
                                                    	: "[-] pager",
        IS_SET( ch->pcdata->flags, PCFLAG_GAG )     	? "[+] GAG"
                                                    	: "[-] gag",
        xIS_SET(ch->act, PLR_BRIEF )                	? "[+] BRIEF"
                                                    	: "[-] brief",
        xIS_SET(ch->act, PLR_COMBINE )              	? "[+] COMBINE"
                                                    	: "[-] combine",
        xIS_SET(ch->act, PLR_BLANK )                	? "[+] BLANK"
                                                    	: "[-] blank",
        xIS_SET(ch->act, PLR_PROMPT )               	? "[+] PROMPT"
                                                    	: "[-] prompt",
        xIS_SET(ch->act, PLR_ANSI )                 	? "[+] ANSI"
                                                    	: "[-] ansi",
        xIS_SET(ch->act, PLR_RIP )                  	? "[+] RIP"
                                                    	: "[-] rip",
	  xIS_SET(ch->act, PLR_COMPASS )			? "[+] COMPASS"
									: "[-] compass" );
      set_char_color( AT_DGREEN, ch );
      send_to_char( "\n\r\n\rAuto:      ", ch );
      set_char_color( AT_GREY, ch );
      ch_printf( ch, "%-12s   %-12s   %-12s   %-12s",
        xIS_SET(ch->act, PLR_AUTOSAC  )             	? "[+] AUTOSAC"
                                                    	: "[-] autosac",
        xIS_SET(ch->act, PLR_AUTOGOLD )             	? "[+] AUTOGOLD"
                                                    	: "[-] autogold",
        xIS_SET(ch->act, PLR_AUTOLOOT )             	? "[+] AUTOLOOT"
                                                    	: "[-] autoloot",
        xIS_SET(ch->act, PLR_AUTOEXIT )             	? "[+] AUTOEXIT"
                                                    	: "[-] autoexit" );
 
      set_char_color( AT_DGREEN, ch );
      send_to_char( "\n\r\n\rSafeties:  ", ch );
      set_char_color( AT_GREY, ch );
      ch_printf( ch, "%-12s   %-12s",
        IS_SET( ch->pcdata->flags, PCFLAG_NORECALL ) 	? "[+] NORECALL"
                                                     	: "[-] norecall",
        IS_SET( ch->pcdata->flags, PCFLAG_NOSUMMON ) 	? "[+] NOSUMMON"
                                                     	: "[-] nosummon" );
 
      if ( !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
        ch_printf( ch, "   %-12s   %-12s",
           xIS_SET(ch->act, PLR_SHOVEDRAG )             ? "[+] DRAG"
                                                        : "[-] drag",
           xIS_SET(ch->act, PLR_NICE )               	? "[+] NICE"
                                                     	: "[-] nice" );
      else
	ch_printf( ch, "   %-12s",
	    IS_SET( ch->pcdata->flags, PCFLAG_NOEXP )	? "[+] NOEXP"
							: "[-] noexp" );
 
      set_char_color( AT_DGREEN, ch );
      send_to_char( "\n\r\n\rMisc:      ", ch );
      set_char_color( AT_GREY, ch );
      ch_printf( ch, "%-12s   %-12s   %-12s   %-12s",
	   xIS_SET(ch->act, PLR_TELNET_GA )		? "[+] TELNETGA"
                                                        : "[-] telnetga",
           IS_SET( ch->pcdata->flags, PCFLAG_GROUPWHO ) ? "[+] GROUPWHO"
                                                        : "[-] groupwho",
           IS_SET( ch->pcdata->flags, PCFLAG_NOINTRO )  ? "[+] NOINTRO"
                                                        : "[-] nointro",
	   IS_SET( ch->pcdata->flags, PCFLAG_BECKON )   ? "[+] BECKON"
							: "[-] beckon" );
 
      set_char_color( AT_DGREEN, ch );
      send_to_char( "\n\r\n\rSettings:  ", ch );
      set_char_color( AT_GREY, ch );
      ch_printf_color( ch, "Pager Length (%d)    Wimpy (&W%d&w)   Hints (%s)",
							ch->pcdata->pagerlen,
						        ch->wimpy,
							IS_SET( ch->pcdata->flags, PCFLAG_HINTS )
							  ? "ON"
							  : "off" );
 
      if ( IS_IMMORTAL( ch ) )
      {
	set_char_color( AT_DGREEN, ch );
        send_to_char( "\n\r\n\rImmortal toggles:  ", ch );
        set_char_color( AT_GREY, ch );
        ch_printf( ch, "Roomvnum [%s]    Automap [%s]",
	  xIS_SET(ch->act, PLR_ROOMVNUM ) 		? "+"
							: " ",
          xIS_SET(ch->act, PLR_AUTOMAP  ) 		? "+"
							: " " );
      }

      set_char_color( AT_DGREEN, ch );
      send_to_char( "\n\r\n\rSentences imposed on you (if any):", ch );
      set_char_color( AT_YELLOW, ch );
      ch_printf( ch, "\n\r%s%s%s%s%s%s%s%s",
          xIS_SET(ch->act, PLR_SILENCE )  ?
            " For your abuse of channels, you are currently silenced.\n\r" : "",
          xIS_SET(ch->act, PLR_NO_EMOTE ) ?
            " The gods have removed your emotes.\n\r"                      : "", 
          xIS_SET(ch->act, PLR_NO_TELL )  ?
            " You are not permitted to send 'tells' to others.\n\r"        : "", 
          xIS_SET(ch->act, PLR_LITTERBUG )?
            " A convicted litterbug.  You cannot drop anything.\n\r"       : "", 
          xIS_SET(ch->act, PLR_THIEF )    ?
            " A proven thief, you will be hunted by the authorities.\n\r"  : "", 
          xIS_SET(ch->act, PLR_KILLER )   ?
            " For the crime of murder you are sentenced to death...\n\r"   : "",
	  xIS_SET(ch->act, PLR_NOHOMEPAGE ) ?
	    " You are not permitted to set your homepage.\n\r"		   : "",
	   IS_SET(ch->pcdata->flags, PCFLAG_NODESC ) ?
	    " You are not permitted to set your description.\n\r"	   : "" );
    }
    else
    {
	bool fSet;
	int bit = 0;

	     if ( arg[0] == '+' ) fSet = TRUE;
	else if ( arg[0] == '-' ) fSet = FALSE;
	else
	{
	    send_to_char( "Config -option or +option?\n\r", ch );
	    return;
	}

	     if ( !str_prefix( arg+1, "autoexit" ) ) bit = PLR_AUTOEXIT;
	else if ( !str_prefix( arg+1, "autoloot" ) ) bit = PLR_AUTOLOOT;
	else if ( !str_prefix( arg+1, "autosac"  ) ) bit = PLR_AUTOSAC;
	else if ( !str_prefix( arg+1, "autogold" ) ) bit = PLR_AUTOGOLD;
	else if ( !str_prefix( arg+1, "blank"    ) ) bit = PLR_BLANK;
	else if ( !str_prefix( arg+1, "brief"    ) ) bit = PLR_BRIEF;
	else if ( !str_prefix( arg+1, "combine"  ) ) bit = PLR_COMBINE;
	else if ( !str_prefix( arg+1, "prompt"   ) ) bit = PLR_PROMPT;
	else if ( !str_prefix( arg+1, "telnetga" ) ) bit = PLR_TELNET_GA;
	else if ( !str_prefix( arg+1, "ansi"     ) ) bit = PLR_ANSI;
	else if ( !str_prefix( arg+1, "compass"  ) ) bit = PLR_COMPASS;
	else if ( !str_prefix( arg+1, "rip"      ) ) bit = PLR_RIP;
/*	else if ( !str_prefix( arg+1, "flee"     ) ) bit = PLR_FLEE; */
	else if ( !str_prefix( arg+1, "nice"     ) ) bit = PLR_NICE;
	else if ( !str_prefix( arg+1, "drag"     ) ) bit = PLR_SHOVEDRAG;
	else if ( IS_IMMORTAL( ch )
	     &&   !str_prefix( arg+1, "vnum"     ) ) bit = PLR_ROOMVNUM;
	else if ( IS_IMMORTAL( ch )
	     &&   !str_prefix( arg+1, "map"      ) ) bit = PLR_AUTOMAP;     /* maps */

	if (bit)
        {
  	  if ( (   bit == PLR_FLEE
		|| bit == PLR_NICE
		|| bit == PLR_SHOVEDRAG )
	  &&  IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
          {
	    send_to_char( "Pkill characters can not config that option.\n\r", ch );
	    return;
          }
          
	  if ( fSet )
	    xSET_BIT   (ch->act, bit);
	  else
	    xREMOVE_BIT(ch->act, bit);
	  send_to_char( "Ok.\n\r", ch );
          return;
        }
        else
        {
	       if ( !str_prefix( arg+1, "norecall" ) ) bit = PCFLAG_NORECALL;
	  else if ( !str_prefix( arg+1, "nointro"  ) ) bit = PCFLAG_NOINTRO;
	  else if ( !str_prefix( arg+1, "beckon"   ) ) bit = PCFLAG_BECKON;
	  else if ( !str_prefix( arg+1, "nosummon" ) ) bit = PCFLAG_NOSUMMON;
          else if ( !str_prefix( arg+1, "gag"      ) ) bit = PCFLAG_GAG; 
          else if ( !str_prefix( arg+1, "pager"    ) ) bit = PCFLAG_PAGERON;
          else if ( !str_prefix( arg+1, "groupwho" ) ) bit = PCFLAG_GROUPWHO;
	  else if ( !str_prefix( arg+1, "noexp"	   ) ) bit = PCFLAG_NOEXP;
	  else if ( !str_prefix( arg+1, "hints"	   ) ) bit = PCFLAG_HINTS;
	  else if ( !str_prefix( arg+1, "@hgflag_" ) ) bit = PCFLAG_HIGHGAG;
          else
	  {
	    send_to_char( "Config which option?\n\r", ch );
	    return;
   	  }
	  if (bit)
          {
            if ( ( bit == PCFLAG_NOSUMMON )
            &&     IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
            {
              send_to_char( "Pkill characters can not config that option.\n\r", ch );
              return;
            }

            if ( fSet )
  	      SET_BIT    (ch->pcdata->flags, bit);
  	    else
  	      REMOVE_BIT (ch->pcdata->flags, bit);

	    send_to_char( "Ok.\n\r", ch );
            return;
	  }
        }
    }

    return;
}


void do_credits( CHAR_DATA *ch, char *argument )
{
  do_help( ch, "credits" );
}


extern int top_area;

/*
void do_areas( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea1;
    AREA_DATA *pArea2;
    int iArea;
    int iAreaHalf;

    iAreaHalf = (top_area + 1) / 2;
    pArea1    = first_area;
    pArea2    = first_area;
    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
	pArea2 = pArea2->next;

    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
    {
	ch_printf( ch, "%-39s%-39s\n\r",
	    pArea1->name, pArea2 ? pArea2->name : "" );
	pArea1 = pArea1->next;
	if ( pArea2 )
	    pArea2 = pArea2->next;
    }

    return;
}
*/

/* 
 * New do_areas with soft/hard level ranges 
void do_areas( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;

    set_pager_color( AT_PLAIN, ch );
    send_to_pager("\n\r   Author    |             Area                     | Recommended |  Enforced\n\r", ch);
    send_to_pager("-------------+--------------------------------------+-------------+-----------\n\r", ch);

    for ( pArea = first_area; pArea; pArea = pArea->next )
	pager_printf(ch, "%-12s | %-36s | %4d - %-4d | %3d - %-3d \n\r", 
	 	pArea->author, pArea->name, pArea->low_soft_range, 
		pArea->hi_soft_range, pArea->low_hard_range, 
		pArea->hi_hard_range);
    return;
}
*/

/*
 * New do_areas, written by Fireblade, last modified - 4/27/97
 *
 *   Syntax: area            ->      lists areas in alphanumeric order
 *           area <a>        ->      lists areas with soft max less than
 *                                                    parameter a
 *           area <a> <b>    ->      lists areas with soft max bewteen
 *                                                    numbers a and b
 *           area old        ->      list areas in order loaded
 *
 */
void do_areas( CHAR_DATA *ch, char *argument )
{ 
    char *header_string1 = "\n\r   &c&GAuthor          &Y|&G              Area"
                                    "                     &Y| "
                                    "&GRecommended &Y|\n\r";
    char *header_string2 = "-------------------+------------------"
                                    "---------------------+----"
                                    "---------+&w&G\n\r";
    char *print_string = "&c&G%-18.18s &Y| &z%-37s &Y|&W %4d - %-4d &Y|\n\r";
 
    AREA_DATA *pArea;
    int lower_bound = 0;
    int upper_bound = MAX_LEVEL + 1;
    /* make sure is to init. > max area level */
    char arg[MAX_STRING_LENGTH];
	char *org = argument;
   
    argument = one_argument(argument,arg);
        
    if(arg[0] != '\0')
    {
      if(!is_number(arg))
      {
        if(!strcmp(arg,"old"))
        {
          set_pager_color( AT_PLAIN, ch );
          send_to_pager_color(header_string1, ch);
          send_to_pager_color(header_string2, ch);
          for (pArea = first_area; pArea;pArea = pArea->next)
          {   
            if ( IS_SET ( pArea->flags, AFLAG_HIDDEN ) )
              continue; /* Blod, 2000 */
            pager_printf_color(ch, print_string,
              pArea->author, pArea->name,
              pArea->low_soft_range,
              pArea->hi_soft_range );
          }  
          return;
       }
       else
       {
       for (pArea = first_area_name; pArea; pArea = pArea->next_sort_name)
       {
           if ( !str_cmp( pArea->name, org ) )
           {
               pager_printf_color( ch, "&CArea:  &Y%s\n\r", pArea->name );
               pager_printf_color( ch, "&CAuthor:  &Y%s\n\r", pArea->author );
               pager_printf_color( ch, "&CCredits: &Y%s\n\r", pArea->credits );
               pager_printf_color( ch, "&CLevel Range:  &Y%d - %d\n\r\n\r", pArea->low_soft_range, pArea->hi_soft_range );
                 do_help( ch, pArea->name );
                 return;
             }
         }
       }

           send_to_char("Area may only be followed by numbers, 'old', or an area name.\n\r", ch);
           return;
      }
                                 
      upper_bound = atoi(arg);
      lower_bound = upper_bound;
                                                
      argument = one_argument(argument,arg);
                                                
      if(arg[0] != '\0')
      {
        if(!is_number(arg))
        {
          send_to_char("Area may only be followed by numbers.\n\r", ch);
          return;
        }
                                
        upper_bound = atoi(arg);
                 
        argument = one_argument(argument,arg);
        if(arg[0] != '\0')
        {
          send_to_char("Only two level numbers allowed.\n\r",ch);
          return;
        }
      }
    }
                         
    if(lower_bound > upper_bound)
    {
      int swap = lower_bound;
      lower_bound = upper_bound;
      upper_bound = swap;
    }
 
    set_pager_color( AT_PLAIN, ch );
    send_to_pager_color(header_string1, ch);
    send_to_pager_color(header_string2, ch);
                                
    for (pArea = first_area_name; pArea; pArea = pArea->next_sort_name)
    {
      if (pArea->hi_soft_range >= lower_bound
      &&  pArea->low_soft_range <= upper_bound)
      {
        if ( IS_SET( pArea->flags, AFLAG_HIDDEN ) )
          continue; /* Blod, 2000 */
        pager_printf_color(ch, print_string,
          pArea->author, pArea->name,
          pArea->low_soft_range,
          pArea->hi_soft_range );
      }
    }
    return;
}

void do_afk( CHAR_DATA *ch, char *argument )
{
     if ( IS_NPC(ch) )
     return;
     
     if xIS_SET(ch->act, PLR_AFK)
     {
    	xREMOVE_BIT(ch->act, PLR_AFK);
	send_to_char( "You are no longer afk.\n\r", ch );
/*	act(AT_GREY,"$n is no longer afk.", ch, NULL, NULL, TO_ROOM);*/
	act(AT_GREY,"$n is no longer afk.", ch, NULL, NULL, TO_CANSEE);
     }
     else
     {
	xSET_BIT(ch->act, PLR_AFK);
	send_to_char( "You are now afk.\n\r", ch );
/*	act(AT_GREY,"$n is now afk.", ch, NULL, NULL, TO_ROOM);*/
	act(AT_GREY,"$n is now afk.", ch, NULL, NULL, TO_CANSEE);
	return;
     }
         
}

void do_slist( CHAR_DATA *ch, char *argument )
{
   int sn, i, lFound;
   char skn[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH]; 
   int lowlev, hilev;
   sh_int lasttype = SKILL_SPELL;

   if ( IS_NPC(ch) )
     return;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   lowlev=1;
   hilev=LEVEL_AVATAR;

   if (arg1[0]!='\0')
      lowlev=atoi(arg1);

   if ((lowlev<1) || (lowlev>LEVEL_IMMORTAL))
      lowlev=1;

   if (arg2[0]!='\0')
      hilev=atoi(arg2);

   if ((hilev<0) || (hilev>=LEVEL_IMMORTAL))
      hilev=LEVEL_HERO;

   if(lowlev>hilev)
      lowlev=hilev;

   set_pager_color( AT_MAGIC, ch );
   send_to_pager("SPELL & SKILL LIST\n\r",ch);
   send_to_pager("------------------\n\r",ch);

   for (i=lowlev; i <= hilev; i++)
   {
	lFound= 0;
	sprintf(skn,"Spell");   
	for ( sn = 0; sn < top_sn; sn++ )
	{
	    if ( !skill_table[sn]->name )
		break;

	    if ( skill_table[sn]->type != lasttype )
	    {
		lasttype = skill_table[sn]->type;
		strcpy( skn, skill_tname[lasttype] );
	    }

            if ( skill_table[sn]->guild != -1 && !IS_IMMORTAL( ch ) )
            {
                if ( skill_table[sn]->guild == 99
                &&  !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
                  continue;
                else if ( skill_table[sn]->guild != 99
                     &&   ( !IS_GUILDED( ch )
                     ||   (  ch->pcdata->clan->class != skill_table[sn]->guild ) ) )
                        continue;
            }

	    if ( ch->pcdata->learned[sn] <= 0
	    &&   SPELL_FLAG(skill_table[sn], SF_SECRETSKILL) )
		continue;

		if ( sysdata.magichell  && ch->level > 35 && number_percent() < 30 )
			continue;

	    if(i==skill_table[sn]->skill_level[ch->class]  )
	    {
		if( !lFound )
		{
		    lFound=1;
		    pager_printf( ch, "Level %d\n\r", i );
		}
                switch (skill_table[sn]->minimum_position)
                {
                    case POS_DEAD:
                            sprintf(buf, "any");
                            break;
                    case POS_MORTAL:
                            sprintf(buf, "mortally wounded");
                            break;
                    case POS_INCAP:
                            sprintf(buf, "incapacitated");
                            break;
                    case POS_STUNNED:
                            sprintf(buf, "stunned");
                            break;
                    case POS_SLEEPING:
                            sprintf(buf, "sleeping");
                            break;
                    case POS_RESTING:
                            sprintf(buf, "resting");
                            break;
                    case POS_STANDING:
                            sprintf(buf, "standing");
                            break;
                    case POS_FIGHTING:
                            sprintf(buf, "fighting");
                            break;
                    case POS_EVASIVE:
                            sprintf(buf, "fighting (evasive)");   /* Fighting style support -haus */
                            break;
                    case POS_DEFENSIVE:
                            sprintf(buf, "fighting (defensive)");
                            break;
                    case POS_AGGRESSIVE:
                            sprintf(buf, "fighting (aggressive)");
                            break;
                    case POS_BERSERK:
                            sprintf(buf, "fighting (berserk)");
                            break;
                    case POS_MOUNTED:
                            sprintf(buf, "mounted");
                            break;
                    case POS_SITTING:
                            sprintf(buf, "sitting");
                            break;
                }

		pager_printf(ch, "%7s: %20.20s \t Current: %-3d Max: %-3d  MinPos: %s \n\r",
                        skn, skill_table[sn]->name, 
                        ch->pcdata->learned[sn],
                        skill_table[sn]->skill_adept[ch->class],
			buf );
	    }
	}
   }
   return;
}

void do_whois( CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  CLAN_DATA *pclan;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  buf[0] = '\0';

  if(IS_NPC(ch))
    return;

  set_pager_color( AT_GREEN, ch );

  if(argument[0] == '\0')
  {
    send_to_pager("You must input the name of an online character.\n\r", ch);
    return;
  }

  strcat(buf, "0.");
  strcat(buf, argument);
  if( ( ( victim = get_char_world(ch, buf) ) == NULL ))
  {
    send_to_pager("No such character online.\n\r", ch);
    return;
  }

  if(IS_NPC(victim))
  {
    send_to_pager("That's not a player!\n\r", ch);
    return;
  }

  set_pager_color( AT_GREY, ch );
  pager_printf(ch, "\n\r'%s%s.'\n\r %s is a %s level %d %s %s, %d years of age.\n\r",
	victim->name,
	victim->pcdata->title,
        victim->sex == SEX_MALE ? "He" :
        victim->sex == SEX_FEMALE ? "She" : "It",
	victim->sex == SEX_MALE ? "male" : 
	victim->sex == SEX_FEMALE ? "female" : "neutral",
	victim->level, 
	capitalize(race_table[victim->race]->race_name),
	class_table[victim->class]->who_name,
	get_age(victim) );

  pager_printf_color(ch, " %s is a %sdeadly player",
	victim->sex == SEX_MALE ? "He" :
	victim->sex == SEX_FEMALE ? "She" : "It",
	IS_SET(victim->pcdata->flags, PCFLAG_DEADLY) ? "" : "non-");

   if ( ( pclan = victim->pcdata->clan ) != NULL )
   {
    switch ( pclan->clan_type )
    {
    default:
            if ( !str_cmp( victim->name, pclan->deity ) )
              send_to_pager( ", and is Deity of ", ch );
            else if ( !str_cmp( victim->name, pclan->leader ) )
              send_to_pager( ", and is Leader of ", ch );
            else if ( !str_cmp( victim->name, pclan->number1 ) )
              send_to_pager( ", and is Number One of ", ch );
            else if ( !str_cmp( victim->name, pclan->number2 ) )
              send_to_pager( ", and is Number Two of ", ch );
            else
              send_to_pager( ", and belongs to Clan ", ch );
            break;
    case CLAN_ORDER:
           if ( !str_cmp( victim->name, pclan->deity ) )
              send_to_pager( ", and is Deity of the Order of ", ch );
           else if ( !str_cmp( victim->name, pclan->leader ) )
              send_to_pager( ", and is Leader of the Order of ", ch );
           else if ( !str_cmp( victim->name, pclan->number1 ) )
              send_to_pager( ", and is Number One of the Order of ", ch );
           else if ( !str_cmp( victim->name, pclan->number2 ) )
              send_to_pager( ", and is Number Two of the Order of ", ch );
           else
              send_to_pager( ", and belongs to the Order of ", ch );
           break;
    case CLAN_GUILD:
           if ( !str_cmp( victim->name, pclan->leader ) )
              pager_printf( ch, ", and is the %s of the ",
                victim->sex == SEX_FEMALE ? "Guildmistress" : "Guildmaster" );
           else if ( !str_cmp( victim->name, pclan->number1 ) )
              send_to_pager( ", and is First of the ", ch );
           else if ( !str_cmp( victim->name, pclan->number2 ) )
              send_to_pager( ", and is Second of the ", ch );
           else
              send_to_pager( ", and belongs to the ", ch );
           break;
    }
    send_to_pager( pclan->name, ch );
   }
  send_to_pager( ".\n\r", ch );

 if(victim->pcdata->council)
  {
    if ( !str_cmp( victim->name, victim->pcdata->council->head ))
      pager_printf( ch, " %s is the %s of:  %s\n\r",
          victim->sex == SEX_MALE ? "He" :
          victim->sex == SEX_FEMALE ? "She" : "It",
          victim->pcdata->council->head2 == NULL ? "Head" : "Co-Head",
          victim->pcdata->council->name );
    else if ( victim->pcdata->council->head2 && !str_cmp ( victim->name, victim->pcdata->council->head2) )
          pager_printf(ch, " %s is the Co-Head of:  %s\n\r",
          victim->sex == SEX_MALE ? "He" :
          victim->sex == SEX_FEMALE ? "She" : "It",
          victim->pcdata->council->name );
    else
          pager_printf(ch, " %s holds a seat on:  %s\n\r",
          victim->sex == SEX_MALE ? "He" :
          victim->sex == SEX_FEMALE ? "She" : "It",
          victim->pcdata->council->name );
  }

    
    pager_printf(ch, " %s has accumulated %d glory in %s lifetime.\n\r",
        victim->name,
	victim->pcdata->quest_accum,
        victim->sex == SEX_MALE ? "his" :
        victim->sex == SEX_FEMALE ? "her" : "its");

  if ( victim->level >= 50 )
    pager_printf(ch, " %s has %d honour and %s holds the rank of: %s\n\r",
	victim->name,
	victim->pcdata->honour,
        victim->sex == SEX_MALE ? "he" :
        victim->sex == SEX_FEMALE ? "she" : "it",
	get_honour ( victim ) );

  if(victim->pcdata->deity)
    pager_printf(ch, " %s has found succor in the deity %s.\n\r",
        victim->sex == SEX_MALE ? "He" :
        victim->sex == SEX_FEMALE ? "She" : "It",
	victim->pcdata->deity->name);
  
  if(victim->pcdata->homepage && victim->pcdata->homepage[0] != '\0')
    pager_printf(ch, " %s homepage can be found at %s\n\r", 
        victim->sex == SEX_MALE ? "His" :
        victim->sex == SEX_FEMALE ? "Her" : "Its",
	show_tilde( victim->pcdata->homepage ) );

  if(victim->pcdata->email && victim->pcdata->email[0] != '\0')
    pager_printf(ch, " %s email is: %s\n\r", 
        victim->sex == SEX_MALE ? "His" :
        victim->sex == SEX_FEMALE ? "Her" : "Its",
	show_tilde( victim->pcdata->email ) );

  if(victim->pcdata->icq && victim->pcdata->icq[0] != '\0')
    pager_printf(ch, " %s icq is %s\n\r", 
        victim->sex == SEX_MALE ? "His" :
        victim->sex == SEX_FEMALE ? "Her" : "Its",
	show_tilde( victim->pcdata->icq ) );

  if(victim->pcdata->bio && victim->pcdata->bio[0] != '\0')
    pager_printf(ch, " %s's personal bio:\n\r%s",
	victim->name,
	victim->pcdata->bio);
  else
    pager_printf(ch, " %s has yet to create a bio.\n\r",
	victim->name );

  if(IS_IMMORTAL(ch))
  {  
    send_to_pager("-------------------\n\r", ch);
    send_to_pager("Info for immortals:\n\r", ch);

    if ( get_trust( ch ) > LEVEL_GOD ){
       if ( victim->pcdata->authed_by && victim->pcdata->authed_by[0] != '\0' )
            pager_printf(ch, "%s was authorized by %s.\n\r",
                victim->name, victim->pcdata->authed_by );
    }


    pager_printf(ch, "%s has killed %d mobiles, and been killed by a mobile %d times.\n\r",
		victim->name, victim->pcdata->mkills, victim->pcdata->mdeaths );
    if ( victim->pcdata->pkills || victim->pcdata->pdeaths )
	pager_printf(ch, "%s has killed %d players, and been killed by a player %d times.\n\r",
		victim->name, victim->pcdata->pkills, victim->pcdata->pdeaths );
    if ( victim->pcdata->illegal_pk )
	pager_printf(ch, "%s has committed %d illegal player kills.\n\r",
		victim->name, victim->pcdata->illegal_pk );

    pager_printf(ch, "%s is %shelled at the moment.\n\r",
	victim->name, 
	(victim->pcdata->release_date == 0) ? "not " : "");

    if (victim->pcdata->nuisance )
    {
      pager_printf_color( ch, "&RNuisance   &cStage: (&R%d&c/%d)  Power:  &w%d  &cTime:  &w%s.\n\r", victim->pcdata->nuisance->flags,
                  MAX_NUISANCE_STAGE, victim->pcdata->nuisance->power,
		              ctime(&victim->pcdata->nuisance->time));
    }
    if(victim->pcdata->release_date != 0)
      pager_printf(ch, "%s was helled by %s, and will be released on %24.24s.\n\r",
	victim->sex == SEX_MALE ? "He" :
	victim->sex == SEX_FEMALE ? "She" : "It",
        victim->pcdata->helled_by,
	ctime(&victim->pcdata->release_date));

    if(get_trust(victim) < get_trust(ch))
    {
      sprintf(buf2, "list %s", buf);
      do_comment(ch, buf2);
    }

    if(xIS_SET(victim->act, PLR_SILENCE) || xIS_SET(victim->act, PLR_NO_EMOTE) 
    || xIS_SET(victim->act, PLR_NO_TELL) || xIS_SET(victim->act, PLR_THIEF) 
    || xIS_SET(victim->act, PLR_KILLER)  ||  IS_SET(victim->pcdata->flags, PCFLAG_NOBECKON)
    ||  IS_SET(victim->pcdata->flags, PCFLAG_NODESC)
    ||  IS_SET(victim->pcdata->flags, PCFLAG_NOBIO)
    ||  IS_SET(victim->pcdata->flags, PCFLAG_NOTITLE)
    ||  IS_SET(victim->pcdata->flags, PCFLAG_NOHTTP)
    || xIS_SET(victim->act, PLR_NOHOMEPAGE)
    || xIS_SET(victim->act, PLR_LITTERBUG) 
	|| xIS_SET(victim->act, PLR_FREEZE) )
    {
      sprintf(buf2, "&GThis character has the following sanctions: &Y");
      if(xIS_SET(victim->act, PLR_SILENCE)) 
        strcat(buf2, " silence");
	  if(xIS_SET(victim->act, PLR_FREEZE))
		strcat(buf2, " frozen");
      if(xIS_SET(victim->act, PLR_NO_EMOTE)) 
        strcat(buf2, " noemote");
      if(xIS_SET(victim->act, PLR_NO_TELL) )
        strcat(buf2, " notell");
      if(xIS_SET(victim->act, PLR_THIEF) )
        strcat(buf2, " thief");
      if(xIS_SET(victim->act, PLR_KILLER) )
        strcat(buf2, " killer");
      if(xIS_SET(victim->act, PLR_LITTERBUG) )
	strcat(buf2, " litterbug");
      if( IS_SET(victim->pcdata->flags, PCFLAG_NOBIO) )
	strcat(buf2, " nobio");
      if( IS_SET(victim->pcdata->flags, PCFLAG_NODESC) )
        strcat(buf2, " nodesc");
      if( IS_SET(victim->pcdata->flags, PCFLAG_NOBECKON) )
        strcat(buf2, " nobeckon");
      if(xIS_SET(victim->act, PLR_NOHOMEPAGE) )
	strcat(buf2, " nohomepage");
      if ( IS_SET( victim->pcdata->flags, PCFLAG_NOTITLE ) )
        strcat(buf2, " notitle");
      if ( IS_SET( victim->pcdata->flags, PCFLAG_NOHTTP ) )
        strcat(buf2, " nohttp");
      strcat(buf2, ".\n\r");
      send_to_pager_color(buf2, ch);
    }
     set_char_color( AT_GREEN, ch );
     if ( victim->desc && victim->desc->host[0]!='\0' )   /* added by Gorog */
     {
     sprintf (buf2, "%s's IP info: %s ", victim->name, victim->desc->host);
     if (get_trust(ch) >= LEVEL_GOD)
        strcat (buf2, victim->desc->user);
     strcat (buf2, "\n\r");
     send_to_pager(buf2, ch);
     }
  else
    if ( victim->pcdata->recent_site )
    {
        sprintf( buf2, "%s's most recent IP: %s \n\r ", victim->name, victim->pcdata->recent_site);
        send_to_pager( buf2, ch );
    }
  }
}

void do_pager( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  
  if ( IS_NPC(ch) )
    return;
  set_char_color( AT_NOTE, ch );
  argument = one_argument(argument, arg);
  if ( !*arg )
  {
    if ( IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) )
    {
        send_to_char( "Pager disabled.\n\r", ch );
        do_config(ch, "-pager");
    } else {
        ch_printf( ch, "Pager is now enabled at %d lines.\n\r", ch->pcdata->pagerlen );
        do_config(ch, "+pager");
    }
    return;
  }
  if ( !is_number(arg) )
  {
    send_to_char( "Set page pausing to how many lines?\n\r", ch );
    return;
  }
  ch->pcdata->pagerlen = atoi(arg);
  if ( ch->pcdata->pagerlen < 5 )
    ch->pcdata->pagerlen = 5;
  ch_printf( ch, "Page pausing set to %d lines.\n\r", ch->pcdata->pagerlen );
  return;
}

/*
 * The ignore command allows players to ignore up to MAX_IGN
 * other players. Players may ignore other characters whether
 * they are online or not. This is to prevent people from
 * spamming someone and then logging off quickly to evade
 * being ignored.
 * Syntax:
 *	ignore		-	lists players currently ignored
 *	ignore none	-	sets it so no players are ignored
 *	ignore <player>	-	start ignoring player if not already
 *				ignored otherwise stop ignoring player
 *	ignore reply	-	start ignoring last player to send a
 *				tell to ch, to deal with invis spammers
 * Last Modified: June 26, 1997
 * - Fireblade
 */
void do_ignore(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	IGNORE_DATA *temp, *next;
	char fname[1024];
	struct stat fst;
	CHAR_DATA *victim;
	
	if(IS_NPC(ch))
		return;
	
	argument = one_argument(argument, arg);
	
	sprintf(fname, "%s%c/%s", PLAYER_DIR,
		tolower(arg[0]), capitalize(arg));
	
	victim = NULL;
	
	/* If no arguements, then list players currently ignored */
	if(arg[0] == '\0')
	{
		set_char_color(AT_DIVIDER, ch);
		ch_printf(ch, "\n\r----------------------------------------\n\r");
		set_char_color(AT_DGREEN, ch);
		ch_printf(ch, "You are currently ignoring:\n\r");
		set_char_color(AT_DIVIDER, ch);
		ch_printf(ch, "----------------------------------------\n\r");
		set_char_color(AT_IGNORE, ch);
		
		if(!ch->pcdata->first_ignored)
		{
			ch_printf(ch, "\t    no one\n\r");
			return;
		}
		
		for(temp = ch->pcdata->first_ignored; temp;
				temp = temp->next)
		{
			ch_printf(ch,"\t  - %s\n\r",temp->name);
		}
		
		return;
	}
	/* Clear players ignored if given arg "none" */
	else if(!strcmp(arg, "none"))
	{
		for(temp = ch->pcdata->first_ignored; temp; temp = next)
		{
			next = temp->next;
			UNLINK(temp, ch->pcdata->first_ignored,
					ch->pcdata->last_ignored,
					next, prev);
			STRFREE(temp->name);
			DISPOSE(temp);
		}
		
		set_char_color(AT_IGNORE, ch);
		ch_printf(ch, "You now ignore no one.\n\r");
		
		return;
	}
	/* Prevent someone from ignoring themself... */
	else if(!strcmp(arg, "self") || nifty_is_name(arg, ch->name))
	{
		set_char_color(AT_IGNORE, ch);
		ch_printf(ch, "Did you type something?\n\r");
		return;
	}
	else
	{
		int i;
		
		/* get the name of the char who last sent tell to ch */		
		if(!strcmp(arg, "reply"))
		{
			if(!ch->reply)
			{
				set_char_color(AT_IGNORE, ch);
				ch_printf(ch, "They're not here.\n\r");
				return;
			}
			else
			{
				strcpy(arg, ch->reply->name);
			}
		}
		
		/* Loop through the linked list of ignored players */
		/* 	keep track of how many are being ignored     */
		for(temp = ch->pcdata->first_ignored, i = 0; temp;
				temp = temp->next, i++)
		{
			/* If the argument matches a name in list remove it */
			if(!strcmp(temp->name, capitalize(arg)))
			{
				UNLINK(temp, ch->pcdata->first_ignored,
					ch->pcdata->last_ignored,
					next, prev);
				set_char_color(AT_IGNORE, ch);
				ch_printf(ch,"You no longer ignore %s.\n\r",
					temp->name);
				STRFREE(temp->name);
				DISPOSE(temp);
				return;
			}
		}
		
		/* if there wasn't a match check to see if the name   */
		/* is valid. This if-statement may seem like overkill */
		/* but it is intended to prevent people from doing the*/
		/* spam and log thing while still allowing ya to      */
		/* ignore new chars without pfiles yet...             */
		if( stat(fname, &fst) == -1 &&
			(!(victim = get_char_world(ch, arg)) ||
			IS_NPC(victim) ||
			strcmp(capitalize(arg),victim->name) != 0))
		{
			set_char_color(AT_IGNORE, ch);
			ch_printf(ch,"No player exists by that"
				" name.\n\r");
			return;
		}
		
		if(victim)
		{
			strcpy(capitalize(arg),victim->name);
		}
		
		/* If its valid and the list size limit has not been */
		/* reached create a node and at it to the list	     */
		if(i < MAX_IGN)
		{
			IGNORE_DATA *new;
			CREATE(new, IGNORE_DATA, 1);
			new->name = STRALLOC(capitalize(arg));
			new->next = NULL;
			new->prev = NULL;
			LINK(new, ch->pcdata->first_ignored,
				ch->pcdata->last_ignored, next, prev);
			set_char_color(AT_IGNORE, ch);
			ch_printf(ch,"You now ignore %s.\n\r", new->name);
			return;
		}
		else
		{
			set_char_color(AT_IGNORE, ch);
			ch_printf(ch,"You may only ignore %d players.\n\r",
				MAX_IGN);
			return;
		}
	}
}

/*
 * This function simply checks to see if ch is ignoring ign_ch.
 * Last Modified: October 10, 1997
 * - Fireblade
 */
bool is_ignoring(CHAR_DATA *ch, CHAR_DATA *ign_ch)
{
	IGNORE_DATA *temp;
	
	if(IS_NPC(ch) || IS_NPC(ign_ch))
		return FALSE;
	
	for(temp = ch->pcdata->first_ignored; temp; temp = temp->next)
	{
		if(nifty_is_name(temp->name, ign_ch->name))
			return TRUE;
	}
	
	return FALSE;
}

/* Version info -- Scryn */

void do_version(CHAR_DATA* ch, char* argument)
{
	if(IS_NPC(ch))
	  return;

	set_char_color(AT_YELLOW, ch);
	ch_printf(ch, "SMAUG %s.%s\n\r", SMAUG_VERSION_MAJOR, SMAUG_VERSION_MINOR);

	if(IS_IMMORTAL(ch))
	  ch_printf(ch, "Compiled on %s at %s.\n\r", __DATE__, __TIME__);

	return;
}
