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
 *                     Housing Module Source File                           *
 ****************************************************************************
 * Author : Senir                                                           *
 * E-Mail : oldgaijin@yahoo.com                                             *
 * Rewrite for RoD - Edmond - during 10/01 and 11/01                        *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include "mud.h"

extern void            shutdown_mud     args ( ( char *reason ) );
void   boot_log        args( ( const char *str, ... ) );

bool in_same_house( CHAR_DATA *ch, CHAR_DATA *vch )
{
	HOME_DATA 	*home;
	int 		x,y;

	if ( IS_IMMORTAL(vch) )
	return TRUE;

	for ( home = first_home;home;home = home->next )
		for ( x = 0; x < MAX_HOUSE_ROOMS; x++ )
			if ( home->vnum[x] == ch->in_room->vnum )
			{
				for ( y = 0;y < MAX_HOUSE_ROOMS; y++ )
				  if ( home->vnum[y] == vch->in_room->vnum )
					return TRUE;

				return FALSE;
			}

	return FALSE;
}		

void do_house( CHAR_DATA *ch, char *argument )
{	bool apt = FALSE;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	ROOM_INDEX_DATA *location = NULL;
	HOME_DATA *homedata;
	sh_int i;
	
	set_char_color( AT_GREEN, ch );
	
	switch( ch->substate )
	{	default:
			break;
		case SUB_ROOM_DESC:
			location = ch->dest_buf;
			if ( !location )
			{ 	bug("house: sub_room_desc: NULL ch->dest_buf", 0 );
				location = ch->in_room;	}
			STRFREE( location->description );
			location->description = copy_buffer( ch );
			stop_editing( ch );
			ch->substate = ch->tempnum;
			save_residence( location );
			send_to_char( "House room description set.\n\r", ch );
			return;	}
	
	if ( !argument || argument[0] == '\0' )
	{	set_char_color( AT_GREEN, ch );
		
		send_to_char( "Syntax:\n\r", ch);
		send_to_char( " house <argument>\n\r", ch);
		send_to_char( "Where the argument is one of:\n\r", ch);
		send_to_char( " name <Desired Title of Room>\n\r desc\n\r", ch);
		send_to_char( " addroom <direction>\n\r", ch);
		send_to_char( " givekey\n\r", ch);
		
		if (IS_IMMORTAL(ch))
		{	set_char_color( AT_IMMORT, ch );
			send_to_char( "\n\rImmortal Arguments:\n\r", ch);
			send_to_char( " set <character name> <vnum> (apartment)\n\r", ch);
			send_to_char( " set <character name> addroom <current house vnum> <direction>\n\r", ch);
			send_to_char( " remove <character name>\n\r", ch);
			send_to_char( " givekey <character name>\n\r", ch);	}
		return;	}
	
	if ( IS_NPC( ch ) )
	{
		send_to_char( "Mobiles do not have houses.\n\r", ch );
		return;
	}
	
	for ( homedata=first_home; homedata; homedata=homedata->next )
	  if ( !str_cmp( homedata->name, ch->name ) )
		{ apt = homedata->apartment;  break; }
	
	if ( !homedata && get_trust( ch ) < LEVEL_DEMI )
	{ 
		send_to_char( "You do not own a residence.\n\r", ch );
		return;
	}
	
	argument = one_argument( argument, arg );
	
	if ( !str_cmp( arg, "givekey" ) )
	{
		if ( get_trust(ch) < LEVEL_DEMI )
		{
			if ( ch->gold >= DEFAULT_MOB_PRICE )
				victim = ch;
			else
			{
				send_to_char( "You do not have the necessary gold to make a key.\n\r", ch);
				return;
			}
		}
		else
		{
			if ( !argument || argument[0] == '\0' )
			{
				send_to_char( "Syntax:  house givekey <character name>\n\r", ch );
				return;
			}
			if ( ( victim = get_char_world( ch, argument ) ) == NULL )
			{
				send_to_char( "That character is not online.\n\r", ch);
				return;
			}
		
			for ( homedata = first_home; homedata; homedata = homedata->next )
				if ( !str_cmp( homedata->name, victim->name ) )
					break;
			
			if ( !homedata )
			{
				send_to_char( "They do not own a home.\n\r", ch );
				return;
			}
		}
		
		if ( !give_key( ch, homedata->vnum[0], homedata->name, homedata->apartment ) )
		{
			send_to_char( "A key was not able to be made.\n\r", ch );
			return;
		}
		
		if ( ch == victim )
			ch->gold -= DEFAULT_MOB_PRICE;
		
		send_to_char( "The requested key has been made.\n\r", ch );
		return;
	}
	
	if ( homedata )
	{
		for ( i=0; i<MAX_HOUSE_ROOMS; i++ )
			if ( homedata->vnum[i] == ch->in_room->vnum )
			{
				location = ch->in_room;
				break;
			}
		
		if ( !location  ) 
			if ( !IS_IMMORTAL( ch ) || ( location = get_room_index( homedata->vnum[0] ) ) == NULL )
			{
				send_to_char( "You must be in your house to make changes to it.\n\r", ch );
				return;
			}
		
		if ( !str_cmp( arg, "desc" ) )
		{
			ch->tempnum = SUB_NONE;
			ch->substate = SUB_ROOM_DESC;
			ch->dest_buf = location;
			sprintf( buf, "%s: house desc", capitalize( ch->name ) );
			log_string( buf );
			start_editing( ch, location->description );
			return;
		}
		
		if ( !argument || argument[0] == '\0' )
		{
			do_house( ch, "" );
			return;
		}

		if ( !str_cmp( arg, "name" ) )
		{
			STRFREE( location->name );
			location->name = STRALLOC( argument );
			save_residence( location );
			send_to_char( "Room title set.\n\r", ch );
			sprintf( buf, "%s: house %s %s", capitalize( ch->name ), arg, argument );
			log_string( buf );
			return;
		}
		
		if ( !str_cmp( arg, "addroom" ) && !apt )
		{
			if ( ch->gold < ADDITIONAL_ROOM_COST )
			{
				send_to_char( "You do not have enough money for an additional room.\n\r", ch );
				return;
			}
			
			for ( i = 0; i < MAX_HOUSE_ROOMS; i++ )
				if ( homedata->vnum[i] == 0 )
					break;
			
			if ( i == MAX_HOUSE_ROOMS )
			{
				send_to_char( "You currently have the maximum number of additional rooms.\n\r", ch );
				return;
			}
			
			if (( get_exit( location, get_dir( argument ))) != NULL )
			{
				send_to_char( "An exit already exists in that direction.\n\r", ch );
				return;
			}
			
			if ( !add_room( homedata, location, argument ) )
			{
				send_to_char( "An exit could not be created.  You may need to contact an immortal.\n\r", ch );
				return;
			}
			
			ch->gold -= ADDITIONAL_ROOM_COST;
			sprintf( buf, "%s: house %s %s", capitalize( ch->name ), arg, argument );
			log_string( buf );
			send_to_char( "Additional room added.\n\r", ch );
			return;
		}
	}
			
	if ( get_trust( ch ) < LEVEL_GREATER )
	{	do_house( ch, "");
		return;	}

	set_char_color( AT_IMMORT, ch );
	sprintf( buf, "%s: house %s %s", capitalize( ch->name ), arg, argument );
	log_string( buf );

	if ( !str_cmp( arg, "set" ) )
	{	char 	arg2[MAX_INPUT_LENGTH];
		char 	arg3[MAX_INPUT_LENGTH];
		sh_int 	i;

		argument = one_argument( argument, arg2 );
		argument = one_argument( argument, arg3 );

		if ( !arg2 || arg2[0] == '\0'
		||	 !arg3 || arg3[0] == '\0' )
		{	
			send_to_char( "Syntax: house set <character name> <vnum> (apartment)\n\r", ch );
			send_to_char( "        house set <character name> addroom <current vnum> <direction>\n\r", ch );
			return; 
		}

		if ( ( victim = get_char_world( ch, arg2 ) ) == NULL )
		{
			send_to_char( "That character is not online.\n\r", ch );
			return;
		}

		if ( IS_NPC( victim ) )
		{
			send_to_char( "Mobiles can not have residences.\n\r", ch );
			return;
		}

		for ( homedata = first_home; homedata; homedata = homedata->next )
			if ( !str_cmp( homedata->name, victim->name ) )
			{
				apt = homedata->apartment;
				break;
			}

		if ( !str_cmp( arg3, "addroom" ) )
		{
			bool found = FALSE;

			argument = one_argument( argument, arg3 );
			if ( !argument || argument[0] == '\0'
				|| !arg3 || arg3[0] == '\0' || !is_number( arg3 ) )
			{
				send_to_char( "Syntax: house set <character name> addroom <current vnum> <direction>\n\r", ch );
				return;
			}

			if ( !homedata )
			{
				send_to_char( "They do not have a house.\n\r", ch );
				return;
			}
			else if ( apt )
			{
				send_to_char( "Apartments cannot have additional rooms.\n\r", ch );
				return;
			}

			if (( location = get_room_index( atoi(arg3) ) ) == NULL )
			{
				send_to_char( "That location does not exist.\n\r", ch );
				return;
			}

			for ( i=0;i < MAX_HOUSE_ROOMS; i++)
			{
				if ( homedata->vnum[i] == atoi(arg3) )
					found=TRUE;
				if ( homedata->vnum[i] == 0 )
					break;
			}

			if ( !found )
			{
				send_to_char( "That vnum is not part of their house.\n\r", ch );
				return;
			}

			if ( i == MAX_HOUSE_ROOMS )
			{
				send_to_char( "That character already has the maximum number of additional rooms.\n\r", ch );
				return;
			}

			if (( get_exit( location, get_dir( argument ))) != NULL )
			{
				send_to_char( "An exit already exists in that direction.\n\r", ch );
				return;
			}

			if ( !add_room( homedata, location, argument ))
			{
				send_to_char( "Error: Room could not be created.\n\r", ch );
				return;
			}

			send_to_char( "Additional room added to that house.\n\r", ch );
			return;
		}

		if ( homedata )
		{
			send_to_char( "They already have a house.\n\r", ch );
			return;
		}

		if ( ( location = get_room_index( atoi(arg3) ) ) == NULL )
		{
			send_to_char( "That room does not exist.\n\r", ch );
			return;
		}

		if ( argument && argument[0] != '\0' && !str_cmp( argument, "apartment" ) )
			apt = TRUE;

		if ( !set_house( victim, atoi(arg3), apt ) )
		{
			send_to_char( "That residence could not be created.\n\r", ch );
			return;
		}

		send_to_char( "Residence created successfully.\n\r", ch );
		return;
	}
	
	if ( !str_cmp( arg, "remove" ) )
	{
		if ( !argument || argument[0] == '\0' )
		{
			send_to_char( "Syntax: house remove <character name>\n\r", ch );
			return;
		}
		
		if ( ( victim = get_char_world( ch, argument ) ) == NULL )
		{
			send_to_char( "That character is not online.\n\r", ch );
			return;
		}
		
		if ( IS_NPC( victim ) )
		{
			send_to_char( "Moblies cannot have residences.\n\r", ch );
			return;
		}
		
		for ( homedata = first_home; homedata; homedata = homedata->next )
			if (!str_cmp(homedata->name, victim->name ) )
				break;
		
		if ( !homedata )
		{
			send_to_char( "That character does not appear to have a residence.\n\r", ch );
			return;
		}
		
		if ( !remove_house( victim ) )
		{
			send_to_char( "The residence could not be removed.\n\r", ch );
			return;
		}
		
		send_to_char( "Residence removed successfully.\n\r", ch );
		return;
	}
	
	do_house( ch, "" );
	return;
}


void do_gohome( CHAR_DATA *ch, char *argument )
{
	HOME_DATA *home;
	ROOM_INDEX_DATA *room;
	char buf[MAX_STRING_LENGTH];
	
	if ( IS_NPC( ch ) || !ch->desc )
		return;
	
	for ( home=first_home; home; home = home->next )
		if ( !str_cmp( home->name, ch->name ) )
			break;

	if ( !home )
	{
		send_to_char( "You do not own a home.\n\r", ch );
		return;
	}

	if ( (room=get_room_index(home->vnum[0]) ) == NULL )
	{
		send_to_char( "Your home no longer exists.\n\r", ch );
		return;
	}
	
	if ( xIS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )
	||   xIS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC  ) )
	{
		send_to_char( "A magical force prevents you from returning home.\n\r", ch );
		return;
	}
	
	switch ( ch->position )
	{
		default:
			break;
		case POS_DEAD:
		case POS_MORTAL:
		case POS_INCAP:
		case POS_STUNNED:
			send_to_char_color( "&RYou are hurt far too badly for that!", ch );
			return;
		case POS_SLEEPING:
		case POS_RESTING:
		case POS_SITTING:
			do_stand( ch, "" );
			break;
		case POS_BERSERK: 
		case POS_AGGRESSIVE:
		case POS_FIGHTING:
		case POS_DEFENSIVE:
		case POS_EVASIVE:
		case POS_SHOVE:
		case POS_DRAG:
			send_to_char_color( "&RNo way! You are too busy!", ch );
			return;
	}
	
	sprintf( buf, "%s disappears in a vortex of luminescent light.", ch->name );
	act( AT_GREY, buf, ch, NULL, ch, TO_ROOM );
	
	char_from_room( ch );
	if ( ch->mount )
	{
		char_from_room( ch->mount );
		char_to_room( ch->mount, room );
	}
	char_to_room( ch, room );
	
	sprintf( buf, "As a vortex of luminescent light forms, %s slides gracefully into their home.\n\r", ch->name );
	act( AT_GREY, buf, ch, NULL, ch, TO_ROOM );
	
	do_look( ch, "" );
	return;
}

void do_residence( CHAR_DATA *ch, char *argument )
{
	HOME_DATA *home;
	ROOM_INDEX_DATA *room;
	sh_int i, z=1;
	
	if ( !first_home )
	{
		send_to_char( "There are currently no residences.\n\r", ch );
		return;
	}
	
	send_to_char_color( "&G                               Current Residences\n\r", ch );
	send_to_char_color( "&O###&G|   &OPlayer Name   &G|         &OHousing Area         &G|&OType&G| &ORooms &G| &OStart Vnum &G|\n\r", ch );
	send_to_char_color( "&G-------------------------------------------------------------------------------\n\r", ch );
	
	for ( home= first_home; home; home = home->next )
	{
		if (( room = get_room_index( home->vnum[0] )) == NULL )
		{
			char buf[MAX_STRING_LENGTH];
			
			sprintf( buf, "Residence: NULL start room vnum - C:%s V:%d", home->name, home->vnum[0] );
			bug( buf, 0 );
			
			continue;
		}
		
		for ( i = 0; i < MAX_HOUSE_ROOMS; i++ )
			if ( home->vnum[i] <= 0 )
				break;
		
		ch_printf_color( ch, "&w%3d&G| &w%-15.15s &G| &w%-28.28s &G|  &w%s &G| &w%5d &G| &w%10d &G|\n\r"
			, z
			, home->name
			, room->area->name
			, home->apartment ? "A" : "H"
			, i
			, home->vnum[0] );
		
		z++;
	}
	
	send_to_char_color( "&G-------------------------------------------------------------------------------\n\r", ch );
	return;
}

void do_accessories( CHAR_DATA *ch, char *argument )
{
	ACCESSORIES_DATA *acc;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	sh_int i=0;
	
	set_char_color( AT_GREEN, ch );
	
	if ( IS_NPC( ch ) )
	{
		send_to_char( "Mobs to not own homes to furnish.\n\r", ch );
		return;
	}
	
	argument = one_argument( argument, arg );
	
	if ( !arg || arg[0] == '\0' )
	{
		send_to_char( "Syntax:\n\r", ch );
		send_to_char( "  accessories <argument>\n\r", ch );
		send_to_char( "\n\rWhere <argument> is one of:\n\r", ch );
		send_to_char( "  list\n\r", ch );
		send_to_char( "  buy <accessory #>\n\r", ch );
		send_to_char( "  show <accessory #>\n\r", ch );
		if ( get_trust( ch ) >= LEVEL_GREATER )
		{
			set_char_color ( AT_IMMORT, ch );
			
			send_to_char( "\n\rImmortal Arguments:\n\r", ch );
			send_to_char( "  add <vnum>\n\r", ch );
			send_to_char( "  add <vnum> (mob)\n\r", ch );
			send_to_char( "  remove <accessory #>\n\r", ch );
			send_to_char( "  setprice <accessory #> <cost>\n\r", ch );
		}
		return;
	}
	
	if ( !str_cmp( arg, "list" ) )
	{
		OBJ_INDEX_DATA *obj;
		MOB_INDEX_DATA *mob;
		char *name;
		char buf[MAX_INPUT_LENGTH];
				
		if ( !first_accessory )
		{
			send_to_char( "There are currently no accessories.\n\r", ch );
			return;
		}
		
		i=1;
		
		send_to_char_color( "&G                House Accessories\n\r", ch );
		if ( !IS_IMMORTAL(ch) )
		{
			send_to_char_color( "&g## &G|        &gAccessory Name        &G|   &gPrice    &G|\n\r", ch );
			send_to_char_color( "&G------------------------------------------------\n\r", ch );
		}
		else
		{
			send_to_char_color( "&g## &G|        &gAccessory Name        &G|   &gPrice    &G|    &gVnum    &G|\n\r", ch );
			send_to_char_color( "&G-------------------------------------------------------------\n\r", ch );
		}
		
		for (acc = first_accessory;acc;acc=acc->next )
		{
			if ( !acc->mob )
			{
				if ( ( obj = get_obj_index( acc->vnum )) == NULL )
				{
					sprintf( buf, "Accessories list: object vnum %d does not exist.", acc->vnum );
					bug( buf, 0 );
					ch_printf_color( ch, "&g%3d&G| &RAccessory currently unavailable.\n\r", i );
					i++;
					continue;

				}
				name = str_dup( obj->short_descr );
			}
			else
			{
				if (( mob = get_mob_index( acc->vnum ) ) == NULL )
				{
					sprintf( buf, "Accessories list: mob vnum %d does not exist.", acc->vnum );
					bug( buf, 0 );
					ch_printf_color( ch, "&g%3d&G| &RAccessory currently unavailable.\n\r", i );
					i++;
					continue;
				}
				
				name = str_dup( mob->short_descr );
			}
			
			if ( !IS_IMMORTAL( ch ) )
			{
				ch_printf_color( ch, "&g%3d&G| &g%-28.28s &G| &g%10d &G|\n\r"
					, i
					, name
					, acc->price );
			}
			else
			{
				ch_printf_color( ch, "&g%3d&G| &g%-28.28s &G| &g%10d &G| &g%10d &G|\n\r"
					, i
					, name
					, acc->price
					, acc->vnum );
			}
			
			i++;
		}
		
		if ( !IS_IMMORTAL( ch ) )
			send_to_char_color( "&G------------------------------------------------\n\r", ch );
		else
			send_to_char_color( "&G-------------------------------------------------------------\n\r", ch );

		return;
	}

	if ( !argument || argument[0] == '\0' )
	{
		do_accessories( ch, "" );
		return;
	}

	if ( str_cmp( arg, "show" ) )
	{
		sprintf( buf, "%s: accessories %s %s", capitalize( ch->name ), arg, argument );
		log_string( buf );
	}
	
	argument = one_argument( argument, arg2 );
	
	if ( !str_cmp( arg, "add" ) && ( get_trust( ch ) >= LEVEL_GREATER ) )
	{
		OBJ_INDEX_DATA *obj = NULL;
		MOB_INDEX_DATA *mob = NULL;
		int vnum;
		ACCESSORIES_DATA *newacc;

		if ( !arg2 || arg2[0] == '\0' || !is_number(arg2) )
		{
			send_to_char( "Syntax: accessories add <vnum> (mob)\n\r", ch );
			return;
		}

		vnum = atoi( arg2 );
		
		if ( argument && !str_cmp( argument, "mob" ) )
		{
			if ( (mob=get_mob_index( vnum )) == NULL )
			{
				send_to_char( "That mob does not exist.\n\r", ch );
				return;
			}
		}
		
		if ( !mob && (obj = get_obj_index( vnum ) ) == NULL )
		{
			send_to_char( "That object does not exist.\n\r", ch );
			return;
		}
		
		for ( acc = first_accessory;acc;acc = acc->next )
		{
			if ( acc->vnum == vnum )
			{
				if ( mob && acc->mob )
				{
					send_to_char( "That mob is already an accessory.\n\r", ch );
					return;
				}
			}
			else
			{
				if ( !mob && !acc->mob )
				{
					send_to_char( "That object is already an accessory.\n\r", ch );
					return;
				}
			}
		}
		
		CREATE( newacc, ACCESSORIES_DATA, 1 );
		LINK( newacc, first_accessory, last_accessory, next, prev );
		
		newacc->vnum = vnum;
		if ( mob )
		{
			newacc->price = DEFAULT_MOB_PRICE;
			newacc->mob = TRUE;
		}
		else
		{
			newacc->price = obj->cost;
			newacc->mob = FALSE;
		}
		
		save_accessories();
		send_to_char( "Accessory added.\n\r", ch );
		return;
	}
	
	for ( acc = first_accessory; acc; acc = acc->next )
		if ( is_number( arg2 ) &&  ++i == atoi( arg2 ) )
			break;
	
	if ( !acc )
	{
		send_to_char( "That is not a valid accessory.\n\r", ch );
		return;
	}
	
	if ( !str_cmp( arg, "buy" ) )
	{
		HOME_DATA *home;
		MOB_INDEX_DATA *mobindex;
		OBJ_INDEX_DATA *objindex;
		ROOM_INDEX_DATA *location = NULL;
		
		for ( home = first_home; home; home = home->next )
			if ( !str_cmp( home->name, ch->name ) )
				break;
		
		if ( !home || home->apartment ) 
		{
			send_to_char( "You need a house to buy accessories.\n\r", ch );
			return;
		}
		
		if ( ch->gold < acc->price )
		{
			send_to_char( "You do not have enough gold.\n\r", ch );
			return;
		}
		
		for ( i=0; i<MAX_HOUSE_ROOMS; i++ )
		{
			if ( ch->in_room->vnum == home->vnum[i] )
			{
				location = ch->in_room;
				break;
			}
		}
		
		if ( !location )
		{
			send_to_char( "You must be in your house to furnish it.\n\r", ch );
			return;
		}
		
		if ( acc->mob && ( mobindex = get_mob_index( acc->vnum )) != NULL )
		{
			CHAR_DATA *mob;
			
			mob = create_mobile( mobindex );
			char_to_room( mob, location );
			add_reset( location->area, 'M', 1, mobindex->vnum, mobindex->count, location->vnum );
			save_residence( location );
		}
		else if ( !acc->mob && ( objindex = get_obj_index( acc->vnum )) != NULL )
		{
			OBJ_DATA *obj;
				
			obj = create_object( objindex, objindex->level );
			obj_to_room( obj, location );
		}
		else
		{
			char buf[MAX_STRING_LENGTH];
			
			sprintf( buf, "Accessory buy: accessory does not exist - V: %d  #: %d"
			, acc->vnum
			, i );
			bug( buf, 0 );
			return;
		}
		
		ch->gold -= acc->price;
		fwrite_house( home );
		send_to_char( "Ok.\n\r", ch );
		return;
	}
	
	if ( !str_cmp( arg, "show" ) )
	{
		OBJ_INDEX_DATA *obj;
		MOB_INDEX_DATA *mob;
		char buf[MAX_STRING_LENGTH];
		
		if ( acc->mob && ( mob = get_mob_index( acc->vnum ) ) != NULL )
		{
			send_to_char_color( "&cMob Accessory Information\n\r", ch);
			ch_printf_color( ch, "&gName: &G%s\n\r", mob->short_descr);
			if ( get_trust( ch ) > LEVEL_CREATOR )
				ch_printf_color( ch, "&gVnum: &G%d\n\r", mob->vnum);
			ch_printf_color( ch, "&gRace: &G%s\n\r", 
				(mob->race < MAX_NPC_RACE && mob->race >= 0?
				npc_race[mob->race] : "unknown"));
			ch_printf_color( ch, "&gClass: &G%s\n\r", 
				(mob->class < MAX_NPC_CLASS && mob->class >= 0?
				npc_class[mob->class] : "unknown"));
			ch_printf_color( ch, "&gAverage Hp: &G%d\n\r", 
				( !mob->hitnodice ? (mob->level * 8 + number_range( mob->level * mob->level / 4, 
				mob->level * mob->level)) :(mob->hitnodice * number_range(1, mob->hitsizedice ) 
				+ mob->hitplus)));
			ch_printf_color( ch, "&gAverage AC: &G%d\n\r", 
				(mob->ac ? mob->ac : interpolate( mob->level, 100, -100 )));
			ch_printf_color( ch, "&gNumber of Attacks(Per Round): &G%d\n\r", mob->numattacks);

			return;
		}
		else if ( !acc->mob && ( obj = get_obj_index( acc->vnum )) != NULL )
		{
			send_to_char_color( "&C&cObject Accessory Information\n\r", ch);
			ch_printf_color( ch, "&C&gName: &G%s\n\r", obj->short_descr);
			if ( get_trust( ch ) > LEVEL_CREATOR )
				ch_printf_color( ch, "&gVnum: &G%d", obj->vnum );
			ch_printf_color( ch, "&C&gType: &G%s\n\r", o_types[obj->item_type]);
			ch_printf_color( ch, "&C&gFlags: &G%s\n\r", ext_flag_string(&obj->extra_flags, o_flags));
			ch_printf_color( ch, "&C&gMagical Flags: &G%s\n\r", magic_bit_name( obj->magic_flags ));
			ch_printf_color( ch, "&C&gWeight: &G%d\n\r", obj->weight);
			return;
		}
		else
		{
			sprintf( buf, "Accessory show: accessory does not exist - V: %d  #: %d"
				, acc->vnum
				, i );
			bug( buf, 0 );
			send_to_char( "Error: Accessory doesn't exist.\n\r", ch);
			return;
		}
	}
	
	if ( get_trust( ch ) < LEVEL_GREATER )
	{
		do_accessories( ch, "" );
		return;
	}
	
	if ( !str_cmp( arg, "remove" ) )
	{
		UNLINK( acc, first_accessory, last_accessory, next, prev );
		
		acc->vnum = 0; acc->price = 0; acc->mob = FALSE;
		DISPOSE( acc );
		save_accessories();
		send_to_char( "Ok.\n\r", ch );
		return;
	}
	
	if ( !str_cmp( arg, "setprice" ) )
	{
		if ( !argument || argument[0] == '\0' || !is_number( argument ) )
		{
			send_to_char( "Syntax: accessories setprice <#> <cost>\n\r", ch );
			return;
		}
		
		acc->price = atoi( argument );
		save_accessories();
		send_to_char( "Ok.\n\r", ch );
		return;
	}
	
	do_accessories(ch, "" );
	return;
}
void do_homebuy( CHAR_DATA *ch, char *argument )
{
	HOMEBUY_DATA *homeb;
	HOME_DATA *home;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int i = 0;
	int bidinc;
	
	set_char_color( AT_GREEN, ch );
	
	if ( IS_NPC( ch ))
	{
		send_to_char( "Mobs cannot purchase residences.\n\r", ch );
		return;
	}
	
	if ( !argument || argument[0] == '\0' || !str_cmp( argument, "list" ) )
	{
		ROOM_INDEX_DATA *location = NULL;
		int days;
		char name[MAX_INPUT_LENGTH];
		char area[MAX_INPUT_LENGTH];
		
		if ( !first_homebuy )
		{
			send_to_char( "No residences are currently on auction.\n\r", ch );
			return;
		}

		send_to_char_color( "&G                 Residence Auction\n\r", ch );
		send_to_char_color( "&c##&C| &cResidence Name       &C| &cHousing Area   &C|&cCurr. Bid &C| &cBid Inc. &C|&cRemaining Time&C|\n\r", ch );
		send_to_char_color( "&C--------------------------------------------------------------------------------\n\r", ch );
		
		i=0;
		
		for ( homeb = first_homebuy; homeb; homeb = homeb->next )
		{
			i++;
			
			if (( location = get_room_index( homeb->vnum )) == NULL )
			{
				char buf[MAX_INPUT_LENGTH];
				
				sprintf( buf, "Homebuy list: location does not exist.  -V:%d  #:%d"
				, homeb->vnum
				, i );
				bug(buf, 0);
				ch_printf_color( ch, "&c%2d&C| &RThis residence not currently available.", i );
				continue;
			}
		
			strcpy( area, location->area->name );
			if ( homeb->apartment )
				sprintf( name, "(A) %s", location->name );
			else
				strcpy( name, location->name );
			
			bidinc = homeb->incpercent * (homeb->bid/100);
			days = homeb->endtime/48;
			
			ch_printf_color( ch, "&c%2d&C|&c%-22.22s&C|&c%-16.16s&C|&c%10d&C|&c%10d&C|&c%2d &CDays &c%2d &CHrs|\n\r"
			,i
			,name
			,area
			,homeb->bid
			,bidinc
			,days
			,( (homeb->endtime-(days*48))/2) );
			
		}
		
		send_to_char_color( "&C--------------------------------------------------------------------------------\n\r", ch );
		return;
	}
	
	argument = one_argument( argument, arg );
	argument = one_argument( argument, arg2 );
	
	if ( !str_cmp( arg, "syntax" ) )
	{
		send_to_char( "Syntax:\n\r", ch);
		send_to_char( " homebuy <argument>\n\r", ch);
		send_to_char( "Where the argument is one of:\n\r", ch);
		send_to_char( " list\n\r", ch);
		send_to_char( " syntax\n\r", ch );
		send_to_char( " show <# of auction item>\n\r", ch);
		send_to_char( " bid <# of auction item> <amount>\n\r", ch);
		
		if (IS_IMMORTAL(ch))
		{
			set_char_color( AT_IMMORT, ch );
			send_to_char( "\n\rImmortal Arguments:\n\r", ch);
			send_to_char( " add <residence vnum> (apartment)\n\r", ch);
			send_to_char( " remove <# of auction item> \n\r", ch);
			send_to_char( " setbid <# of auction item> <amount>\n\r", ch);
			send_to_char( " bidincrement <# of auction item>  <percent>\n\r", ch);
			send_to_char( " timeremainder <# of auction item> <days> (<hours>)\n\r", ch);
			send_to_char( " clearbidder <# of auction item>\n\r", ch);

		}
		return;
	}
	
	if ( !arg2 || arg2[0] == '\0' || !is_number( arg2 ) )
	{
		do_homebuy( ch, "syntax" );
		return;
	}
	
	if ( !str_cmp( arg, "add" ) && get_trust( ch ) >= LEVEL_GREATER )
	{
		int vnum = atoi(arg2);
		ROOM_INDEX_DATA *location;
		bool apartment = FALSE;
		
		if ( argument[0] != '\0' && !str_cmp( argument, "apartment" ) )
			apartment = TRUE;
		
		if (( location = get_room_index( vnum )) != NULL )
		{
			if ( xIS_SET( location->room_flags, ROOM_HOUSE ) )
			{
				send_to_char( "That room is already a residence.  Use \"sellhouse\" or first remove that player's house.\n\r", ch );
				return;
			}
		}
	
		if ( !add_homebuy( ch, vnum, apartment, 0 ) )
		{
			send_to_char( "Vnum could not be added to the auction.\n\r"
						  "Vnum may already be on auction or vnum does not exist.\n\r", ch);
			return;
		}
		
		send_to_char( "Residence added to auction successfully.\n\r", ch );
		return;
	}
	
	for ( homeb = first_homebuy; homeb; homeb = homeb->next )
		if ( ++i == atoi( arg2 ) )
			break;

	if ( !homeb )
	{
		send_to_char( "That is not a valid auction #.\n\r", ch );
		return;
	}

	bidinc = homeb->incpercent * ( homeb->bid/100 );
		
	if ( !str_cmp( arg, "show" ) )
	{
		ROOM_INDEX_DATA *location;
		int days;
		
		if (( location = get_room_index( homeb->vnum )) == NULL )
		{
			char buf[MAX_INPUT_LENGTH];
			
			sprintf( buf, "Homebuy show: location does not exist. -V: %d, #: %d", homeb->vnum, i );
			bug( buf, 0 );
			send_to_char( "That residence does not exist.  Please contact an immortal.\n\r", ch );
			return;
		}
		
		days = homeb->endtime / 48;
		send_to_char_color( "&GHome Auction Residence Information\n\r", ch);
		ch_printf_color( ch, "&CName: &c%s\n\r", location->name);
		ch_printf_color( ch, "&CType: &c%s\n\r", (homeb->apartment ? "Apartment" : "House"));
		ch_printf_color( ch, "&CHousing Area: &c%s\n\r", location->area->name);
		ch_printf_color( ch, "&CCurrent Bid: &c%d\n\r", homeb->bid);
		ch_printf_color( ch, "&CBid Increment: &c%d\n\r", bidinc);
		ch_printf_color( ch, "&CTime Left: &c%d &CDays &c%d &CHours\n\r",
			days, ((homeb->endtime - (days * 48)) / 2));
		if (IS_IMMORTAL(ch))
		{
			ch_printf_color( ch, "&CSeller: &c%s\n\r", homeb->seller);
			ch_printf_color( ch, "&CBidder: &c%s\n\r", homeb->bidder);
			ch_printf_color( ch, "&CVnum: &c%d\n\r", homeb->vnum);
		}
		return;
	}
	
	if ( !str_cmp( arg, "bid" ) )
	{
		HOMEBUY_DATA *checkhome;
		int bid;
		
		if ( !argument || argument[0] == '\0' || !is_number( argument ) )
		{
			send_to_char( "Syntax: homebuy bid <# of auction> <amount>\n\r", ch );
			return;
		}
		
		bid = atoi( argument );
		
		for ( home = first_home; home; home=home->next )
			if ( !str_cmp( home->name, ch->name ) )
			{
				send_to_char( "You already own a home. You must sell that one first.\n\r", ch );
				return;
			}
		
		for ( checkhome = first_homebuy; checkhome; checkhome=checkhome->next )
			if ( !str_cmp( checkhome->bidder, ch->name ) )
			{
				send_to_char( "You are already bidding on a home.\n\r", ch );
				return;
			}
		
		if ( !str_cmp( homeb->seller, ch->name ) )
		{
			send_to_char( "You cannot bid on your own home.\n\r", ch );
			return;
		}

		if ( ( homeb->bid + bidinc ) < 0 )
		{
			send_to_char( "You cannot bid more than the current bid.\n\r", ch );
			return;
		}
		
		if ( bid < homeb->bid || bid < (homeb->bid + bidinc) )
		{
			send_to_char( "You must bid more than the current bid plus the bid increment.\n\r", ch );
			return;
		}
		
		if ( ch->gold < bid )
		{
			send_to_char( "You do not have that much gold.\n\r", ch );
			return;
		}
		
		STRFREE( homeb->bidder );
		homeb->bidder = STRALLOC( ch->name );
		homeb->bid = bid;
		save_homebuy();
		send_to_char( "You have successfully placed a bid.\n\r", ch );
		return;
	}
	
	if ( get_trust( ch ) < LEVEL_GREATER )
	{
		do_homebuy( ch, "syntax" );
		return;
	}
	
	if ( !str_cmp( arg, "remove" ) )
	{
		if ( !remove_homebuy( homeb ) )
		{
			send_to_char( "Residence could not be removed from auction.\n\r", ch );
			return;
		}
		send_to_char( "Residence removed from auction successfully.\n\r", ch );
		return;
	}
	
	if ( !str_cmp( arg, "clearbidder" ) )
	{
		STRFREE( homeb->bidder );
		homeb->bidder = STRALLOC( "None" );
		save_homebuy();
		send_to_char( "Bidder cleared.\n\r", ch );
		return;
	}
	
	if ( !str_cmp( arg, "timeremainder" ) )
	{
		char arg3[MAX_INPUT_LENGTH];
		sh_int hours, days;
		
		argument = one_argument( argument, arg3 );
		
		if (!arg3 || arg3[0] == '\0' || !is_number( arg3 )
		|| !argument || argument[0] == '\0' || !is_number( argument ) )
		{
			do_homebuy( ch, "syntax" );
			return;
		}
		
		hours = atoi( argument); days = atoi ( arg3 );
		if ( days > 31 || days < 0 )
		{
			send_to_char( "Days can be from 0 to 31.\n\r", ch );
			return;
		}
		
		if ( hours < 0 || hours > 23 )
		{
			send_to_char( "Hours can be from 0 to 23.\n\r" , ch );
			return;
		}
		
		homeb->endtime = ( days*48 ) + ( hours * 2 );
		save_homebuy();
		send_to_char( "Time remainder set.\n\r", ch );
		return;
	}
	
	if ( !argument || argument[0] == '\0' || !is_number( argument ) )
	{
		do_homebuy( ch, "syntax" );
		return;
	}
	
	if ( !str_cmp( arg, "setbid" ) )
	{
		homeb->bid = atoi( argument );
		save_homebuy();
		send_to_char( "Bid set.\n\r", ch );
		return;
	}
	
	if ( !str_cmp( arg, "bidincrement" ) )
	{
		homeb->incpercent = atoi( argument );
		save_homebuy();
		send_to_char( "Bid increment precentage set.\n\r", ch );
		return;
	}
	
	do_homebuy( ch, "syntax" );
	return;
}


void do_sellhouse( CHAR_DATA *ch, char *argument )
{
	HOME_DATA *home;
	int bid;
	
	set_char_color( AT_GREEN, ch );
	
	if ( IS_NPC( ch ) )
	{
		send_to_char( "Mobs cannot sell homes.\n\r", ch );
		return;
	}
	
	for ( home=first_home;home;home=home->next )
		if ( !str_cmp( home->name, ch->name ) )
		break;
		
	if ( !home )
	{
		send_to_char( "You do not have a home to sell.\n\r", ch );
		return;
	}
	
	if ( !argument || argument [0] == '\0' )
	{
		send_to_char( "Options for Sellhouse Command:\n\r", ch);
		send_to_char( "  Sellhouse yes\n\r", ch);
		send_to_char( "    Places your residence on auction with default starting bid.\n\r", ch);
		send_to_char( "  Sellhouse <amount>\n\r", ch);
		send_to_char( "   Places your residence on auction with the given amount as starting bid.\n\r", ch);
		return;
	}

	bid = ( is_number( argument ) ? atoi(argument) : 0 );
	
	if ( str_cmp( argument, "yes" ) )
	{
		if ( ( home->apartment && bid < MIN_APARTMENT_BID ) || bid < MIN_HOUSE_BID )
		{
			send_to_char( "Bid amount must be equal to or greater than the minimum bid for the residence type.\n\r", ch );
			return;
		}
	}
	
	if ( !add_homebuy( ch, home->vnum[0], home->apartment, bid ) )
	{
		send_to_char( "Your residence could not be added to the auction.\n\r"
					  "Please contact an immortal for assistance.\n\r", ch );
		return;
	}
	
	send_to_char( "You have successfully placed your residence on auction for the given starting bid.\n\r"
			"Profit from the sale should occur when your house has been bought on auction.\n\r"
			"Loss of your residence should also occur at that time.\n\r", ch);
	return;
}


void save_residence(ROOM_INDEX_DATA *location)
{
	AREA_DATA *tarea = NULL;
	char filename[256];

	if ( !location ) 
	{
		bug ( "Save_residence: NULL Location", 0);
		return;
	}

	for ( tarea = first_area; tarea; tarea = tarea->next )
	{
		if ( tarea == location->area )
		{
			fold_area( location->area, location->area->filename, FALSE );
			return;
		}
    }

	for ( tarea = first_build; tarea; tarea = tarea->next )
	{
		if ( tarea == location->area )
		{
			sprintf( filename, "%s%s", BUILD_DIR, location->area->filename );
			fold_area( location->area, filename, FALSE );
			return;
		}
	}

	bug( "Save_residence: Location doesn't have an area.", 0);
	return;
}

bool set_house( CHAR_DATA *ch, int vnum, bool apartment)
{
   HOME_DATA *tmphome, *shome = NULL;
   char buf[MAX_STRING_LENGTH];
   ROOM_INDEX_DATA *location=NULL;
   EXIT_DATA *pexit=NULL;
   OBJ_INDEX_DATA *obj, *key = NULL;
   
   if (!ch || !ch->name || vnum <= 0)
	   return FALSE;

    CREATE(tmphome, HOME_DATA, 1);

	tmphome->name = STRALLOC(ch->name);
	tmphome->apartment = apartment;
	tmphome->vnum[0] = vnum;

	if (first_home)
	{
		bool found = FALSE;

		for ( shome = first_home; shome; shome = shome->next )
		{
			if ( strcmp(tmphome->name, shome->name) < 0 )
			{
				INSERT( tmphome, shome, first_home, next, prev );
				found = TRUE;
				break;
			}
		}

		if (!found)
			LINK( tmphome, first_home, last_home, next, prev );
	}
	else
	{
		LINK( tmphome, first_home, last_home, next, prev );
	}

	update_house_list();
	fwrite_house(tmphome);

	if ((location = get_room_index(vnum)) == NULL)
		return FALSE;

    if (apartment)
		sprintf( buf, "%s's Apartment", tmphome->name);
	else
		sprintf( buf, "%s's House", tmphome->name);

	STRFREE( location->name );
	location->name = STRALLOC( buf );
	STRFREE( location->description );
	if (apartment)
		location->description = STRALLOC( "A room surrounds you, filled with the dirt and filth of previous tenants.\n\rA spray painted frame of the last tenant can be seen on the floor in a \n\rfar corner.\n\r");
	else
	    location->description = STRALLOC( "This is your desc. You can edit this with HOUSE DESC.\n\r");
	
	location->sector_type = 0;
	location->max_weight = 2000;
        xSET_BIT( location->room_flags, ROOM_NO_SUMMON );
        xSET_BIT( location->room_flags, ROOM_NO_ASTRAL );
        xSET_BIT( location->room_flags, ROOM_INDOORS );
        xSET_BIT( location->room_flags, ROOM_HOUSE );
        xREMOVE_BIT( location->room_flags, ROOM_PROTOTYPE );

	for ( pexit = location->first_exit; pexit; pexit = pexit->next )
	{
		SET_BIT( pexit->exit_info, EX_ISDOOR );
		SET_BIT( pexit->exit_info, EX_CLOSED);
		SET_BIT( pexit->exit_info, EX_LOCKED);
		SET_BIT( pexit->exit_info, EX_NOPASSDOOR);
		SET_BIT( pexit->exit_info, EX_PICKPROOF);
		SET_BIT( pexit->exit_info, EX_BASHPROOF);

		pexit->key = location->vnum;
		add_reset( location->area, 'D', 0, location->vnum, pexit->vdir, 2 );                                                                                                                                             

		if (pexit->rexit)
		{
		   ROOM_INDEX_DATA *revroom;
		   
			SET_BIT( pexit->rexit->exit_info, EX_ISDOOR );
		    SET_BIT( pexit->rexit->exit_info, EX_CLOSED);
		    SET_BIT( pexit->rexit->exit_info, EX_LOCKED);
			SET_BIT( pexit->rexit->exit_info, EX_NOPASSDOOR);
		    SET_BIT( pexit->rexit->exit_info, EX_PICKPROOF);
		    SET_BIT( pexit->rexit->exit_info, EX_BASHPROOF);

			pexit->rexit->key = location->vnum;

		   if ( ( revroom = get_room_index( pexit->vnum ) ) )
			{
				add_reset( revroom->area, 'D', 0, revroom->vnum, pexit->rexit->vdir, 2 );                                                                                                                                             
				fold_area( revroom->area, revroom->area->filename, FALSE );  
			}
		      
		}

	}
		
/*	if (apartment)
		return TRUE; */

	if ( (obj = get_obj_index(location->vnum)) != NULL)
	{
		delete_obj(obj);
		save_residence(location);
	}

	sprintf( buf, "%s %s key", tmphome->name, (apartment ? "apartment" : "house") );
    key = make_object( location->vnum, 0, buf);
	key->value[0] = location->vnum;
	key->item_type = ITEM_HOUSEKEY;
    key->level = 1;
	sprintf( buf, "%s's %s Key", tmphome->name, (apartment ? "Apartment" : "House") );
    STRFREE( key->short_descr );
	key->short_descr = STRALLOC( buf );
        STRFREE( key->description );
        sprintf( buf, "A small golden key is here." );
        key->description = STRALLOC( buf );
	SET_BIT( key->wear_flags, ITEM_TAKE );
	SET_BIT( key->wear_flags, ITEM_HOLD );
 
	xREMOVE_BIT(key->extra_flags, ITEM_PROTOTYPE);
	
	save_residence(location);

	return TRUE;
}

bool remove_house( CHAR_DATA *ch )
{
	HOME_DATA *home;
	ROOM_INDEX_DATA *location;
	OBJ_INDEX_DATA *key;
	AREA_DATA *area;
	EXIT_DATA *pexit;
	RESET_DATA *reset;
	sh_int i;
	char filename[256];
	
	for ( home = first_home; home; home= home->next )
		if ( !str_cmp( home->name, ch->name ) )
			break;
	
	if ( !home )
		return FALSE;
	
	if ( ( location = get_room_index( home->vnum[0] ) ) == NULL )
		return FALSE;
	
	sprintf( filename, "%s%s", HOUSE_DIR, capitalize( home->name ) );
	remove( filename );
	
	if ( home->vnum[1] > 0 )
	{
		ROOM_INDEX_DATA *addloc;
		
		for ( i=1; i < MAX_HOUSE_ROOMS; i++)
		{
			if ( home->vnum[i] <= 0 )
				continue;
			
			if ( ( addloc = get_room_index( home->vnum[i] ) ) == NULL )
			{
				home->vnum[i]=0;
				continue;
			}
			
			area = addloc->area;
			
			for ( pexit=addloc->first_exit;pexit;pexit = pexit->next )
			{
				if ( pexit->rexit )
				extract_exit( pexit->to_room, pexit->rexit );
			}
			
			for ( reset = area->first_reset; reset; reset = reset->next )
			{
				if ( is_room_reset( reset, addloc, addloc->area ) )
				{
					delete_reset( area, reset );
					reset = area->first_reset;
				}
			}
			
			home->vnum[i] = 0;
			
			delete_room( addloc );
			fold_area( area, area->filename, FALSE );
		}
	}
	
	if ( ( key = get_obj_index( location->vnum ) ) != NULL )
	{
		delete_obj( key );
	}
	
	STRFREE( location->name );
	location->name = STRALLOC( "Vacant Residence" );
	STRFREE( location->description );
	location->description = STRALLOC( "" );
	
     xREMOVE_BIT( location->room_flags, ROOM_NO_SUMMON );
     xREMOVE_BIT( location->room_flags, ROOM_NO_ASTRAL );
     xREMOVE_BIT( location->room_flags, ROOM_INDOORS );
     xREMOVE_BIT( location->room_flags, ROOM_HOUSE );
     xSET_BIT( location->room_flags, ROOM_PROTOTYPE );

	 for ( pexit = location->first_exit; pexit; pexit = pexit->next )
	 {
		 pexit->key=-1;
		 REMOVE_BIT( pexit->exit_info, EX_ISDOOR );
		 REMOVE_BIT( pexit->exit_info, EX_CLOSED);
		 REMOVE_BIT( pexit->exit_info, EX_LOCKED);
		 REMOVE_BIT( pexit->exit_info, EX_NOPASSDOOR);
		 REMOVE_BIT( pexit->exit_info, EX_PICKPROOF);
		 REMOVE_BIT( pexit->exit_info, EX_BASHPROOF);
		 
		 if(pexit->rexit)
		 {
			 pexit->rexit->key=-1;
			 REMOVE_BIT( pexit->rexit->exit_info, EX_ISDOOR );
			 REMOVE_BIT( pexit->rexit->exit_info, EX_CLOSED);
			 REMOVE_BIT( pexit->rexit->exit_info, EX_LOCKED);
			 REMOVE_BIT( pexit->rexit->exit_info, EX_NOPASSDOOR);
			 REMOVE_BIT( pexit->rexit->exit_info, EX_PICKPROOF);
			 REMOVE_BIT( pexit->rexit->exit_info, EX_BASHPROOF);
		 }
	 }

	 for ( reset = location->area->first_reset; reset; reset = reset->next )
	 {

		if ( is_room_reset( reset, location, location->area ) )
		{
			delete_reset( location->area, reset );
			reset = location->area->first_reset;
		}
	 }

	save_residence(location);
	UNLINK(home, first_home, last_home, next, prev);
	DISPOSE(home);
    update_house_list();

	return TRUE;
}

bool add_room( HOME_DATA *home, ROOM_INDEX_DATA *location, char *argument )
{
	ROOM_INDEX_DATA *addloc = NULL;
	AREA_DATA *pArea;
	EXIT_DATA *pexit, *rexit;
	char buf[MAX_STRING_LENGTH];
	int i;
	
	if ( !location || !home || !argument || argument[0] == '\0' )
	{
		bug( "add_room: passed null information", 0 );
		return FALSE;
	}
	
	for ( pArea = first_area; pArea; pArea = pArea->next )
	{
		if ( !str_cmp( pArea->filename, ADDED_ROOM_HOUSING_AREA ) )
		break;
	}
	
	if ( !pArea)
	{	
		bug( "add_room: housing area file not found in installed areas.", 0 );
		return FALSE;
	}
	
	for ( i=pArea->low_r_vnum; i<pArea->hi_r_vnum; i++ )
	{
		if (( get_room_index(i)) == NULL )
		{
			if (( addloc = make_room(i)) == NULL )
			{
				bug( "add_room: make_room unable to complete.", 0 );
				return FALSE;
			}
		break;
		}
	}
	
	if ( !addloc )
	{
		bug( "add_room: housing area appears to be full.", 0 );
		return FALSE;
	}
	
	if ( ( pexit = make_exit( location, addloc, get_dir( argument ) ) ) == NULL 
		 || (rexit = make_exit( addloc, location, rev_dir[get_dir( argument )] ) ) == NULL )
	{
		delete_room( addloc );
		bug( "add_room: exits not able to be created for room.  location deleted.", 0);
		return FALSE;
	}
	
	pexit->keyword                = STRALLOC( "" );
    pexit->description            = STRALLOC( "" );
    pexit->key                    = -1;
    pexit->exit_info              = 0;
    rexit->keyword                = STRALLOC( "" );
    rexit->description            = STRALLOC( "" );
    rexit->key                    = -1;
    rexit->exit_info              = 0;
    pexit->rexit                  = rexit;
    rexit->rexit                  = pexit;

	sprintf( buf, "%s\'s Additional House Room", capitalize(home->name) );
    STRFREE( addloc->name );
    addloc->name = STRALLOC( buf );
    STRFREE( addloc->description );
    addloc->description = STRALLOC( "This is your desc. You can edit this with HOUSE DESC.\n\r");
    addloc->sector_type = 0;
	addloc->max_weight = 2000;
	xSET_BIT( addloc->room_flags, ROOM_NO_SUMMON );
	xSET_BIT( addloc->room_flags, ROOM_NO_ASTRAL );
    xSET_BIT( addloc->room_flags, ROOM_INDOORS );
    xSET_BIT( addloc->room_flags, ROOM_HOUSE );
	xREMOVE_BIT( addloc->room_flags, ROOM_PROTOTYPE);

	addloc->area = pArea;
	
	for ( i=0; i < MAX_HOUSE_ROOMS; i++)
	{
		if ( home->vnum[i] <= 0)
		{
			home->vnum[i] = addloc->vnum;
			break;
		}
	}
	
	fwrite_house( home );
	save_residence( addloc );
	
	if ( addloc->area != location->area )
		save_residence( location );
	
	return TRUE;
}

bool give_key( CHAR_DATA *ch, int vnum, char *name, bool apt )
{
	OBJ_INDEX_DATA *keyindex = NULL;
	OBJ_DATA *key = NULL;

	if( !ch || vnum <= 0)
		return FALSE;
	if ( ( keyindex = get_obj_index( vnum ) ) == NULL )
	{
	if ( get_trust( ch ) >= LEVEL_GREATER )
	{
     char buf[MAX_STRING_LENGTH];

     sprintf( buf, "%s %s key", name, (apt ? "apartment" : "house") );
     keyindex = make_object( vnum, 0, buf);
     keyindex->value[0] = vnum;
     keyindex->item_type = ITEM_HOUSEKEY;
     keyindex->level = 1;
     sprintf( buf, "%s's %s Key", name, (apt ? "Apartment" : "House") );
     STRFREE( keyindex->short_descr );
     keyindex->short_descr = STRALLOC( buf );
     STRFREE( keyindex->description );
     keyindex->description = STRALLOC( buf );
     SET_BIT( keyindex->wear_flags, ITEM_TAKE );
     SET_BIT( keyindex->wear_flags, ITEM_HOLD );
     xREMOVE_BIT(keyindex->extra_flags, ITEM_PROTOTYPE);

	 send_to_char( "Key previously not created.  You must fold houses.are to save this key.\n\r", ch );
	}
	else	
	return FALSE;
	}
		
	if (( key = create_object( keyindex, 1 )) == NULL)
		return FALSE;

	STRFREE( key->owner );
	key->owner = STRALLOC( name );
	obj_to_char( key, ch );

	send_to_char( "A house key appears in your inventory.\n\r", ch);

	return TRUE;
}

void fwrite_house(HOME_DATA *homedata)
{
   FILE *fpout;
   ROOM_INDEX_DATA *location = NULL;
   OBJ_DATA *obj=NULL;
   sh_int i, j =0;
   char buf[MAX_STRING_LENGTH];
   char strsave[MAX_INPUT_LENGTH];

    if (!homedata || !homedata->name || homedata->name[0] == '\0')
    {
		bug( "Fwrite_house: NULL homedata", 0);
		return;
	}

    sprintf( strsave, "%s%s", HOUSE_DIR, capitalize( homedata->name ) );

    if ( (fpout=fopen( strsave, "w" )) == NULL )
    {
        sprintf( buf, "Fwrite_house: Cannot open %s for writing.", strsave);
		bug( buf, 0 );
		return;
    }

	fprintf( fpout, "#HOME\n" );
	fprintf( fpout, "Name        %s~\n", homedata->name        );

	for (i=0; i < MAX_HOUSE_ROOMS; i++)
	{
		if (homedata->vnum[i] > 0)
			fprintf( fpout, "Vnum    %d\n", homedata->vnum[i]);
	}
	
	fprintf( fpout, "Apartment   %d\n", homedata->apartment   );
	fprintf( fpout, "End\n\n" );

	for (i=0; i < MAX_HOUSE_ROOMS; i++)
	{
		if (homedata->vnum[i] > 0 
			&& (location = get_room_index(homedata->vnum[i])) != NULL)
		{
			obj = location->last_content;
			j=supermob->level;
			supermob->level=MAX_LEVEL;

			if (obj)
				fwrite_obj(supermob, obj, fpout, 0, OS_VAULT );
			supermob->level=j;
		}
	}

    fprintf( fpout, "#END\n" );
    fclose( fpout );
	fpout = NULL;
}

void load_homedata()
{
   FILE *fpList;
   char *filename;
   char list[256];
   char buf[MAX_STRING_LENGTH];
    
    first_home	= NULL;
    last_home	= NULL;

    sprintf( list, "%s%s", HOUSE_DIR, HOUSE_LIST );
    fclose( fpReserve );
	fpReserve = NULL;

	if ( ( fpList = fopen( list, "r" ) ) == NULL )
	{
		log_string ( "Cannot open house.lst for reading. Home loading aborted.");
		shutdown_mud( "No house.lst" );
		exit(1);
        }

	for ( ; ; )
	{
		filename = feof( fpList ) ? "$" : fread_word( fpList );
	
		if ( filename[0] == '$' )
			break;

		if ( !load_house_file( filename ) )
		{
			sprintf( buf, "Cannot load house file: %s", filename );
			bug( buf, 0 );
		}

    }

    fclose( fpList );
	fpList = NULL;
    fpReserve = fopen( NULL_FILE, "r" );
    return;

}

#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];

bool load_house_file( char *name )
{
   FILE *fp;
   char filename[256];
   char buf[MAX_STRING_LENGTH];
   int iNest, vnum = 0;
   OBJ_DATA *obj = NULL;
   ROOM_INDEX_DATA *pRoom = NULL;

    sprintf( filename, "%s%s", HOUSE_DIR, name);

    if ( ( fp = fopen( filename, "r" ) ) == NULL )
		return FALSE;

	
	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
		rgObjNest[iNest] = NULL;

	for ( ; ; )
	{
		char letter;
	    char *word;

	    letter = fread_letter( fp );

		if ( letter == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if ( letter != '#' )
		{

			sprintf( buf, "Load_house_file: # not found in %s.", filename);
			boot_log( buf );
			break;
		}

		word = fread_word( fp );
		
		if ( !str_cmp( word, "HOME"	) )
		{
			pRoom = get_room_index( vnum = fread_house( fp ) );
			if ( !pRoom )
			{
				sprintf( buf, "Load_house_file: bad house vnum in %s.  Vnum %d", filename, vnum );
				boot_log( buf );
				return FALSE;
			}
			rset_supermob(pRoom);
			continue;
		}
		else if ( !str_cmp( word, "OBJECT" ) )
		{
			fread_obj( supermob, fp, OS_VAULT );
			continue;
		}
		else if ( !str_cmp( word, "END"	) )
		{
			if ( !pRoom )
				return FALSE;
			break;
		}
		else
		{
		sprintf( buf, "Load_house_file: bad section: %s in file %s.", word, filename );
		boot_log( buf );
		break;
	    }
	}

	fclose( fp );
	fp = NULL;
    
	while ( (obj = supermob->first_carrying) != NULL)
	{
		obj_from_char( obj );
		obj_to_room( obj, pRoom );
	}

	release_supermob();

	return TRUE;
}

void update_house_list( )
{
    HOME_DATA *tmphome;
    FILE *fpout;
    char filename[256];

    sprintf( filename, "%s%s", HOUSE_DIR, HOUSE_LIST );

    if ( (fpout = fopen( filename, "w" )) == NULL )
    {
		bug( "FATAL: cannot open house.lst for writing!\n\r", 0 );
		return;
	}
	
	for ( tmphome = first_home; tmphome; tmphome = tmphome->next )
		fprintf( fpout, "%s\n", capitalize(tmphome->name) );
	
	fprintf( fpout, "$\n" );
    fclose( fpout );
	fpout = NULL;
}

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

int fread_house( FILE *fp )
{
   sh_int i = 0;
   HOME_DATA *homedata = NULL;
   char buf[MAX_STRING_LENGTH];

    CREATE( homedata, HOME_DATA, 1 );

	for ( ;; )
    {
		char *word;
		bool fMatch;
		
		word   = feof( fp ) ? "End" : fread_word( fp );
		fMatch = FALSE;

		switch ( UPPER(word[0]) )
		{
		
		case '*':
			fMatch = TRUE;
			fread_to_eol( fp );
			break;

		case 'E':
			if ( !str_cmp( word, "End" ) )
			{
				if ( !homedata->name )
				{
					boot_log( "Fread_house: NULL Name" );
                    DISPOSE(homedata);
					return -1;
				}

				LINK( homedata, first_home, last_home, next, prev );

				return(homedata->vnum[0] <= 0 ? 2 : homedata->vnum[0]);
			}

			break;

		case 'A':
			KEY( "Apartment",   homedata->apartment,        fread_number(fp) );
			break;

		case 'N':
			KEY( "Name",        homedata->name,             fread_string(fp) );
	    break;

        case 'V':
		if ( !str_cmp( word, "Vnum" ) )
		{
			int vnum;

			fMatch = TRUE;
			vnum = fread_number( fp );
			if ( !get_room_index( vnum ) )
			{
				DISPOSE( homedata );
				return -1;
			}
				homedata->vnum[i++] = vnum;
		}
			break;
	}
	
		if ( !fMatch )
		{
			sprintf( buf, "Fread_house: no match: %s", word );
			bug( buf, 0 );
		}
	}
}

#undef KEY

void save_house_by_vnum(int vnum)
{
   HOME_DATA *tmphome = NULL;
   sh_int i = 0;

    for (tmphome = first_home; tmphome; tmphome = tmphome->next)
	if ( !tmphome->apartment )
	   for (i = 0; i < MAX_HOUSE_ROOMS; i++)
			if (tmphome->vnum[i] == vnum)
				fwrite_house(tmphome);

	return;
}

void save_accessories()
{
	FILE *fpout;
    ACCESSORIES_DATA *tmpacc = NULL;
	OBJ_INDEX_DATA *obj = NULL;
	MOB_INDEX_DATA *mob = NULL;

	if ( (fpout = fopen( ACCESSORIES_FILE, "w" ) ) == NULL )
    {
        bug( "Cannot open homeaccessories.dat for writing", 0 );
        perror( ACCESSORIES_FILE );
		return;
    }

	for ( tmpacc = first_accessory; tmpacc; tmpacc = tmpacc->next)
	{
		if (!tmpacc->mob)
		{
			if ( (obj = get_obj_index(tmpacc->vnum)) == NULL)
				continue;
		}
		else
		{
			if ( (mob = get_mob_index( tmpacc->vnum ) ) == NULL )
				continue;
		}

		fprintf( fpout, "#ACCESSORIES\n" );
		fprintf( fpout, "Vnum        %d\n",  tmpacc->vnum        );
		fprintf( fpout, "Price       %d\n",  tmpacc->price       );
		fprintf( fpout, "Mob         %d\n",  tmpacc->mob         );
	    fprintf( fpout, "End\n\n" );
    }

    fprintf( fpout, "#END\n" );
    fclose( fpout );
	fpout = NULL;
}

void load_accessories()
{
    FILE *fp;

    if ( ( fp = fopen( ACCESSORIES_FILE, "r" ) ) == NULL )
    {
		boot_log( "Cannot open homeaccessories.dat. Aborting loadup of accessories data." ); 
		return;
	}

	for ( ;; )
	{
		char letter;
	    char *word;

	    letter = fread_letter( fp );

	    if ( letter == '*' )
	    {
			fread_to_eol( fp );
			continue;
	    }

	    if ( letter != '#' )
	    {
			bug( "Load_accessories: # not found.", 0 );
			break;
	    }

	    word = fread_word( fp );

		if ( !str_cmp( word, "ACCESSORIES"      ) )
	    {
			fread_accessories( fp );
	    	continue;
	    }
	    else if ( !str_cmp( word, "END"	) )
			break;
	    else
	    {
			bug( "Load_accessories: bad section.", 0 );
			continue;
	    }
	}
	
	fclose( fp );
	fp = NULL;
}

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void fread_accessories( FILE *fp )
{ 
   ACCESSORIES_DATA *newacc = NULL;
   char buf[MAX_STRING_LENGTH];

    CREATE( newacc, ACCESSORIES_DATA, 1 );

	for ( ;; )
    {
		char *word;
		bool fMatch;
		
		word   = feof( fp ) ? "End" : fread_word( fp );
		fMatch = FALSE;

		switch ( UPPER(word[0]) )
		{
		
		case '*':
			fMatch = TRUE;
			fread_to_eol( fp );
			break;

		case 'E':
			if ( !str_cmp( word, "End" ) )
			{
				if ( (newacc->mob && (get_mob_index(newacc->vnum)) == NULL)
			     || (!newacc->mob && (get_obj_index(newacc->vnum)) == NULL))
				{
					bug( "Fread_accessories: Accessory doesn't exist", 0 );
                    DISPOSE(newacc);
					return;
				}

				LINK( newacc, first_accessory, last_accessory, next, prev );

				return;
			}

			break;

		case 'M':
			KEY( "Mob",        newacc->mob,             fread_number(fp) );
	    break;

		
		case 'P':
			KEY( "Price",      newacc->price,             fread_number(fp) );
	    break;

        case 'V':
            KEY( "Vnum",       newacc->vnum,             fread_number(fp) );
            break;
		}
	
		if ( !fMatch )
		{
			sprintf( buf, "Fread_accessories: no match: %s", word );
			bug( buf, 0 );
		}
	}
}

#undef KEY

bool add_homebuy( CHAR_DATA *seller, int vnum, bool apartment, int price)
{
   HOMEBUY_DATA *tmphome = NULL;

	if ( vnum <= 0 || !seller || IS_NPC(seller))
		return FALSE;

	for ( tmphome = first_homebuy; tmphome; tmphome = tmphome->next)
	{
		if ( tmphome->vnum == vnum)
			break;
	}

	if (tmphome)
		return FALSE;

	if ( (get_room_index( vnum)) == NULL)
		return FALSE;

	CREATE(tmphome, HOMEBUY_DATA, 1);
	LINK(tmphome, first_homebuy, last_homebuy, next, prev);

	tmphome->vnum = vnum;
	tmphome->apartment = apartment;
	tmphome->endtime = ( 7 * 48);
	tmphome->incpercent = DEFAULT_BID_INCREMENT_PERCENTAGE;
	tmphome->bidder = STRALLOC("None");
	tmphome->seller = STRALLOC(seller->name);

	if (price <= 0)
	{
		if (apartment)
			tmphome->bid = MIN_APARTMENT_BID;
		else
			tmphome->bid = MIN_HOUSE_BID;
	}
	else
	{
		tmphome->bid = price;
	}

	save_homebuy();

	return TRUE;
}

bool remove_homebuy(HOMEBUY_DATA *tmphome)
{
	if (!tmphome)
		return FALSE;

	STRFREE( tmphome->bidder);
	STRFREE( tmphome->seller);
	tmphome->incpercent = 0;
	tmphome->apartment = FALSE;
	tmphome->bid = 0;
	tmphome->vnum = 0;
	tmphome->endtime = 0;

	UNLINK(tmphome, first_homebuy, last_homebuy, next, prev);

	DISPOSE(tmphome);

	save_homebuy();

	return TRUE;
}

void save_homebuy()
{
	FILE *fpout;
    HOMEBUY_DATA *tmphome = NULL;

	if ( (fpout = fopen( HOMEBUY_FILE, "w" ) ) == NULL )
    {
		boot_log( "Cannot open homebuy.dat for writing", 0);
        perror( HOMEBUY_FILE );
		return;
    }

	for ( tmphome = first_homebuy; tmphome; tmphome = tmphome->next)
	{
		if ( (get_room_index( tmphome->vnum)) == NULL)
			continue;

		fprintf( fpout, "#HOMEBUY\n" );
		fprintf( fpout, "Apartment   %d\n",  tmphome->apartment  );
		fprintf( fpout, "Bid         %d\n",  tmphome->bid        );
		fprintf( fpout, "Bidder      %s~\n", tmphome->bidder     );
		fprintf( fpout, "BidIncPerc  %d\n",  tmphome->incpercent );
		fprintf( fpout, "Endtime     %d\n",  tmphome->endtime    );
		fprintf( fpout, "Seller      %s~\n", tmphome->seller     );
		fprintf( fpout, "Vnum        %d\n",  tmphome->vnum       );
	    fprintf( fpout, "End\n\n" );
    }

    fprintf( fpout, "#END\n" );
    fclose( fpout );
	fpout = NULL;
}

void load_homebuy()
{
    FILE *fp;

    if ( ( fp = fopen( HOMEBUY_FILE, "r" ) ) == NULL )
    {
		boot_log( "Cannot open homebuy.dat. Aborting loadup of home auction data." );
		return;
	}

	for ( ;; )
	{
		char letter;
	    char *word;

	    letter = fread_letter( fp );

	    if ( letter == '*' )
	    {
			fread_to_eol( fp );
			continue;
	    }

	    if ( letter != '#' )
	    {
			bug( "Load_accessories: # not found.", 0 );
			break;
	    }

	    word = fread_word( fp );

		if ( !str_cmp( word, "HOMEBUY"      ) )
	    {
			fread_homebuy( fp );
	    	continue;
	    }
	    else if ( !str_cmp( word, "END"	) )
			break;
	    else
	    {
			boot_log( "Load_homebuy: bad section." );
			continue;
	    }
	}
	
	fclose( fp );
	fp = NULL;
}

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void fread_homebuy( FILE *fp )
{ 
   HOMEBUY_DATA *newhome = NULL;
   char buf[MAX_STRING_LENGTH];

    CREATE( newhome, HOMEBUY_DATA, 1 );

	for ( ;; )
    {
		char *word;
		bool fMatch;
		
		word   = feof( fp ) ? "End" : fread_word( fp );
		fMatch = FALSE;

		switch ( UPPER(word[0]) )
		{
		
		case '*':
			fMatch = TRUE;
			fread_to_eol( fp );
			break;

		case 'A':
			KEY( "Apartment",  newhome->apartment,       fread_number(fp) );
		break;

		case 'B':
			KEY( "Bid",        newhome->bid,             fread_number(fp) );
			KEY( "Bidder",     newhome->bidder,          fread_string(fp) );
			KEY( "BidIncPerc", newhome->incpercent,      fread_number(fp) );
		break;

		case 'E':

			KEY( "Endtime",    newhome->endtime,         fread_number(fp) );

			if ( !str_cmp( word, "End" ) )
			{
				if ( (get_room_index(newhome->vnum)) == NULL)
				{
					bug( "Fread_homebuy: Residence doesn't exist", 0 );
                    DISPOSE(newhome);
					return;
				}

				LINK( newhome, first_homebuy, last_homebuy, next, prev );

				return;
			}

			break;
		
		case 'S':
			KEY( "Seller",     newhome->seller,         fread_string(fp) );
	    break;

        case 'V':
            KEY( "Vnum",       newhome->vnum,           fread_number(fp) );
        break;

		}
	
		if ( !fMatch )
		{
			sprintf( buf, "Fread_homebuy: no match: %s", word );
			bug( buf, 0 );
		}
	}
}

#undef KEY

void homebuy_update()
{
	HOMEBUY_DATA *home, *nexthome;
	CHAR_DATA *seller = NULL, *bidder = NULL;
	char buf[MAX_STRING_LENGTH];
	
	for ( home = first_homebuy; home; home = nexthome )
	{
		nexthome = home->next;
		
		if ( --home->endtime > 0 && ( ( home->bid + (home->incpercent * (home->bid/100)) ) > 0 ) )
		{
			save_homebuy();
			continue;
		}
		
		if ( home->bidder[0] != '\0' && str_cmp( home->bidder, "None" )
			&& ( bidder = load_player( home->bidder ) ) != NULL )
		{
			if ( bidder->gold < home->bid )
			{
				if (( bidder->gold -= ( home->bid/100*PENALTY_PERCENTAGE ) )<0 )
					bidder->gold = 0;
				home->endtime = 144;
				STRFREE( home->bidder );
				home->bidder = STRALLOC( "None" );
				
				if ( bidder->desc )
					send_to_char( "You did not have enough money for the residence you bid on.\n\r"
								  "It has been readded to the auction and you have been penalized.\n\r", bidder );
				else
					add_loginmsg(bidder->name, 1, NULL);
				
				save_homebuy();
				logoff( bidder );
				continue;
			}
		}
		
		if ( home->seller[0] == '\0' || ( seller = load_player( home->seller )) == NULL )
		{
			sprintf( buf, "homebuy_update: Seller of residence with vnum %d could not be found.", home->vnum );
			bug( buf, 0 );
		
			if ( !remove_homebuy( home ) )
			{ bug( "this is an error", 0 );
			return;
			}
		
			if ( bidder->desc )
				send_to_char( "There was an error while looking up the seller for the residence you had\n\r"
							  "bid on.  That residence was removed and no interaction has taken place.\n\r", bidder );
			else
				add_loginmsg( bidder->name, 2, NULL );
			
			continue;
		}
		
		if ( !bidder )
		{
			if (( seller->gold -= home->bid/100*PENALTY_PERCENTAGE ) < 0 )
				seller->gold = 0;
			
			if ( seller->desc )
				send_to_char( "There was no bid on your residence.  It has been removed from auction\n\r"
							  "and you have been penalized.\n\r", seller );
			else
				add_loginmsg( seller->name, 3, NULL );
			
			remove_homebuy( home );
			logoff( seller );
			continue;
		}
		
		if ( get_trust( seller ) < LEVEL_GREATER )
		if ( !remove_house( seller ) )
		{
				bug( "homebuy_update: residence could not be removed from seller after sale.", 0 );
				sprintf( buf, "homebuy_update: Vnum: %d  Seller: %s   Bidder: %s"
					, home->vnum
					, seller->name
					, bidder->name );
				bug( buf, 0 );
				logoff( bidder );
				logoff( seller );
				continue;
		}
			
		if ( !set_house( bidder, home->vnum, home->apartment ))
		{
				bug( "homebuy_update: residence could not be setup after sale.", 0 );
				sprintf( buf, "homebuy_update: Vnum: %d  Seller: %s   Bidder: %s"
					, home->vnum
					, seller->name
					, bidder->name );
				bug( buf, 0 );
				logoff( bidder );
				logoff( seller );
				continue;
		}
		
		seller->gold += home->bid;
		bidder->gold -= home->bid;

		sprintf( buf, "Homebuy_update: Home successfully sold B:%s S:%s V:%d", 
				bidder->name,
				seller->name,
				home->vnum );
		log_string( buf );

		if ( !give_key( bidder, home->vnum, bidder->name, home->apartment ))
		{
			sprintf( buf, "Homebuy_update: Key for residence with vnum %d could not be given.", home->vnum);
			bug( buf, 0);
		}

		remove_homebuy( home );
		
		if ( bidder->desc )
			send_to_char( "You have successfully received your new residence.\n\r", bidder ); 
		else
			add_loginmsg( bidder->name, 4, NULL );
		
		if ( seller->desc )
			send_to_char( "You have successfully sold your residence.\n\r", seller );
		else
			add_loginmsg( seller->name, 5, NULL );
		
		logoff( bidder );
		logoff( seller );
	}
	
	return;
}

CHAR_DATA *load_player( char *name )
{
   CHAR_DATA *onlinechar = NULL;
   DESCRIPTOR_DATA *d = NULL;
   struct stat fst;
   char buf[MAX_STRING_LENGTH];
   int oldvnum = 0;
   
    sprintf( buf, "%s%c/%s", PLAYER_DIR, LOWER(name[0]),
		capitalize(name) );

	for ( onlinechar = first_char; onlinechar; onlinechar = onlinechar->next )
	{

		if ( IS_NPC(onlinechar) ) 
			continue;

		if ( !str_cmp( onlinechar->name, name ) )
			return onlinechar;

	}

	if ( !check_parse_name( name, FALSE) )
		return NULL;

	if ( stat( buf, &fst ) == -1 )
		return NULL;

	CREATE( d, DESCRIPTOR_DATA, 1 );
	d->next = NULL;
	d->prev = NULL;
	d->connected = CON_GET_NAME;
	d->outsize = 2000;
	CREATE( d->outbuf, char, d->outsize );
	load_char_obj( d, name, FALSE );
	add_char( d->character );


	oldvnum = ((get_room_index(d->character->in_room->vnum)) != NULL) ? d->character->in_room->vnum
		: ROOM_VNUM_LIMBO;

	
	char_to_room( d->character, get_room_index(oldvnum));

	d->character->retran = oldvnum;

	d->character->desc	= NULL;
    if ( d->character->pcdata->prev_site )
        d->character->pcdata->recent_site = STRALLOC( d->character->pcdata->prev_site );
	d->character		= NULL;	

	DISPOSE( d->outbuf );

	DISPOSE( d );

	for ( onlinechar = first_char; onlinechar; onlinechar = onlinechar->next )
	{

		if ( IS_NPC(onlinechar) ) 
			continue;

		if ( !str_cmp( onlinechar->name, name ) )
			return onlinechar;

	}

	return NULL;
}

void logoff( CHAR_DATA *ch )
{
   int x, y;
   
    x = y = 0;

	if (!ch || IS_NPC(ch) || ch->desc != NULL || ch->switched != NULL)
		return;

	if ( ch->position == POS_MOUNTED )
		do_dismount( ch, "" );
        
	set_char_color( AT_GREY, ch);

	if ( !ch->in_room || (get_room_index(ch->in_room->vnum)) == NULL)
		ch->in_room = get_room_index( ROOM_VNUM_LIMBO);

	quitting_char = ch;
	save_char_obj( ch );
        
    if ( sysdata.save_pets && ch->pcdata && ch->pcdata->pet )
		extract_char( ch->pcdata->pet, TRUE );

	if ( ch->pcdata && ch->pcdata->clan )
		save_clan( ch->pcdata->clan );

	saving_char = NULL;
	
	extract_char( ch, TRUE );

	for ( x = 0; x < MAX_WEAR; x++ )
        for ( y = 0; y < MAX_LAYERS; y++ )
			save_equipment[x][y] = NULL;

	return;
}

