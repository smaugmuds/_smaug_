/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * Win32 port by Nick Gammon                                                *
 * ------------------------------------------------------------------------ *
 *                          Undertaker Module                               *
 * ------------------------------------------------------------------------ *
 * Part of this code is from act_wiz.c : do_owhere(). Structure follows     *
 * that of the ROM healer port by Desden, el Chaman Tibetano.               *
 * Brought together by Cyrus & Robcon (ROC2). Please send suggestions to    *
 *    cyrus@cx.net                                                          *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mud.h"

void do_corpse( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj, *outer_obj;
    CHAR_DATA *undertaker;
    bool found = FALSE;
    int cost = 0;

    /* Avoids the potential for filling the room with hundreds of mob corpses */
    if( IS_NPC(ch) )
    {
	send_to_char( "Mobs cannot retreive corpses.\n\r", ch );
	return;
    }

		for( undertaker = ch->in_room->first_person; undertaker; undertaker = undertaker->next_in_room )
			if( IS_NPC( undertaker ) && xIS_SET( undertaker->act, ACT_UNDERTAKER ) )
				break;

		set_char_color( AT_GREEN, ch );
		if( !undertaker )
    {
        send_to_char( "There's no undertaker here!\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        act (AT_PLAIN, "$N says 'Ooo Yesss ... I can helpss you.'", ch, NULL, undertaker, TO_CHAR);
        send_to_char ("  retrieve: Retrieves your corpse worths 20 coins per level.\n\r", ch);
        send_to_char (" Type corpse <type> for the service.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "retrieve"))
        cost  = 20 * ch->level;
    else
    {
        act (AT_PLAIN,"$N says ' Type 'corpse' for help on what I do.'", ch, NULL, undertaker, TO_CHAR);
        return;
    }

#ifdef ENABLE_GOLD_SILVER_COPPER
    if (cost > ch->copper )
    {
        act(AT_PLAIN,"$N says 'Pah! You do not have enough copper for my services!'", ch, NULL, undertaker, TO_CHAR);
#else
    if (cost > ch->gold )
    {
        act(AT_PLAIN,"$N says 'Pah! You do not have enough gold for my services!'", ch, NULL, undertaker, TO_CHAR);
#endif
        return;
    }

    strcpy( buf, "the corpse of " );
    strcat( buf, ch->name ); 			/* Bug fix here by Samson 12-21-00 See below */
    for ( obj = first_object; obj; obj = obj->next )
    {
        if ( !nifty_is_name( buf, obj->short_descr ) ) /* Fix here - Samson 1-26-01 */
              continue;

	  /* This will prevent NPC corpses from being retreived if the person has a mob's name */
	  if ( obj->item_type == ITEM_CORPSE_NPC )
		continue;

        found = TRUE;
        
        /* Could be carried by act_scavengers, or other idiots so ... */
        outer_obj = obj;
        while ( outer_obj->in_obj )
              outer_obj = outer_obj->in_obj;

        separate_obj( outer_obj );
        obj_from_room( outer_obj );
        obj_to_room( outer_obj, ch->in_room );

#ifdef ENABLE_GOLD_SILVER_COPPER
        ch->copper -= cost;
#else
        ch->gold -= cost;
#endif
        act (AT_PLAIN, "$N creepily carts in your corpse.", ch, NULL, undertaker, TO_CHAR);
        act (AT_PLAIN, "$n creepily carts in the $T.", undertaker, NULL, buf, TO_ROOM);
    }

    /* Could've been extracted, so do this */
    if ( !found )
        act (AT_PLAIN, "$N says 'Sorry I can't find your corpse. There's nothing more I can do.'", ch, NULL, undertaker, TO_CHAR);

    return;
}
