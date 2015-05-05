/*
                     R E A L M S    O F    D E S P A I R  !
   ___________________________________________________________________________
  //            /                                                            \\
 [|_____________\   ********   *        *   ********   *        *   *******   |]
 [|   \\._.//   /  **********  **      **  **********  **      **  *********  |]
 [|   (0...0)   \  **********  ***    ***  **********  ***    ***  *********  |]
 [|    ).:.(    /  ***         ****  ****  ***    ***  ***    ***  ***        |]
 [|    {o o}    \  *********   **********  **********  ***    ***  *** ****   |]
 [|   / ' ' \   /   *********  *** ** ***  **********  ***    ***  ***  ****  |]
 [|-'- /   \ -`-\         ***  ***    ***  ***    ***  ***    ***  ***   ***  |]
 [|   .VxvxV.   /   *********  ***    ***  ***    ***  **********  *********  |]
 [|_____________\  **********  **      **  **      **  **********  *********  |]
 [|             /  *********   *        *  *        *   ********    *******   |]
  \\____________\____________________________________________________________//
     |                                                                     |
     |    --{ [S]imulated [M]edieval [A]dventure Multi[U]ser [G]ame }--    |
     |_____________________________________________________________________|
     |                                                                     |
     |                     -*- Room editing Module -*-                     |
     |_____________________________________________________________________|
     |                                                                     |
     |     OasisOLC II for Smaug 1.40 written by Evan Cortens(Tagith)      |
     |                                                                     |
     |   Based on OasisOLC for CircleMUD3.0bpl9 written by Harvey Gilpin   |
     |_____________________________________________________________________|
    //                                                                     \\
   [|  SMAUG 2.0 © 2014-2015 Antonio Cao (@burzumishi)                      |]
   [|                                                                       |]
   [|  AFKMud Copyright 1997-2007 by Roger Libiez (Samson),                 |]
   [|  Levi Beckerson (Whir), Michael Ward (Tarl), Erik Wolfe (Dwip),       |]
   [|  Cameron Carroll (Cam), Cyberfox, Karangi, Rathian, Raine,            |]
   [|  Xorith, and Adjani.                                                  |]
   [|  All Rights Reserved. External contributions from Remcon, Quixadhal,  |]
   [|  Zarius and many others.                                              |]
   [|                                                                       |]
   [|  SMAUG 1.4 © 1994-1998 Thoric/Altrag/Blodkai/Narn/Haus/Scryn/Rennard  |]
   [|  Swordbearer/Gorog/Grishnakh/Nivek/Tricops/Fireblade/Edmond/Conran    |]
   [|                                                                       |]
   [|  Merc 2.1 Diku Mud improvments © 1992-1993 Michael Chastain, Michael  |]
   [|  Quan, and Mitchell Tse. Original Diku Mud © 1990-1991 by Sebastian   |]
   [|  Hammer, Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, Katja    |]
   [|  Nyboe. Win32 port Nick Gammon.                                       |]
    \\_____________________________________________________________________//
*/

#include <stdio.h>
#include <string.h>
#include "mud.h"
#include "olc.h"

/*------------------------------------------------------------------------*/
/* function prototypes */

int get_rflag				args( ( char *flag ) );

DECLARE_DO_FUN( do_redit_reset );
void redit_disp_extradesc_menu		args( ( DESCRIPTOR_DATA *d ) );
void redit_disp_exit_menu		args( ( DESCRIPTOR_DATA *d ) );
void redit_disp_exit_flag_menu		args( ( DESCRIPTOR_DATA *d ) );
void redit_disp_flag_menu		args( ( DESCRIPTOR_DATA *d ) );
void redit_disp_sector_menu		args( ( DESCRIPTOR_DATA *d ) );
void redit_disp_menu			args( ( DESCRIPTOR_DATA *d ) );

void redit_parse			args( ( DESCRIPTOR_DATA *d, char *arg ) );
void redit_setup_new			args( ( DESCRIPTOR_DATA *d ) );
void free_room				args( ( ROOM_INDEX_DATA *room ) );

/*-----------------------------------------------------------------------*/
/* Global variable declarations/externals */
/* EXIT_DATA *get_exit_number( ROOM_INDEX_DATA *room, int xit ); */
void oedit_disp_extra_choice		args( ( DESCRIPTOR_DATA *d ) );
extern  char *  const        ex_flags[];
char * const sector_names[];
extern int top_ed;

/*-----------------------------------------------------------------------*/

void do_oredit( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    ROOM_INDEX_DATA *room;

    if ( IS_NPC(ch) || !ch->desc )
    {
	send_to_char( "I don't think so...\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );
    if ( argument[0] == '\0' )
        room = ch->in_room;
    else
    {
	if ( is_number( arg ) )
	{
            argument = one_argument( argument, arg );
	    room = get_room_index( atoi( arg ) );
	}
	else
	{
	    send_to_char( "Vnum must be specified in numbers!\n\r", ch );
	    return;
	}
    }

    if ( !room )
    {
	send_to_char( "That room does not exist!\n\r", ch );
	return;
    }
        
    /* Make sure the room isnt already being edited */
    for ( d = first_descriptor; d; d = d->next )
	if ( d->connected == CON_REDIT )
	    if ( d->olc && OLC_VNUM(d) == room->vnum )
	    {
		ch_printf( ch, "That room is currently being edited by %s.\n\r", d->character->name );
		return;
	    }

    if ( !can_rmodify( ch, room ) )
	return;

    d = ch->desc;
    CREATE( d->olc, OLC_DATA, 1 );
    OLC_VNUM(d) = room->vnum;
    OLC_CHANGE(d) = FALSE;
    d->character->dest_buf = room;
    d->connected = CON_REDIT;
    redit_disp_menu( d );

    act( AT_ACTION, "$n starts using OLC.", ch, NULL, NULL, TO_ROOM );
    return;
}

void do_rcopy( CHAR_DATA *ch, char *argument )
{
    return;
}

bool is_inolc( DESCRIPTOR_DATA *d )
{
    /* safeties, not that its necessary really... */
    if ( !d || !d->character )
	return FALSE;

    if ( IS_NPC( d->character ) )
	return FALSE;

    /* objs */
    if ( d->connected == CON_OEDIT )
	return TRUE;

    /* mobs */
    if ( d->connected == CON_MEDIT )
	return TRUE;

    /* rooms */
    if ( d->connected == CON_REDIT )
	return TRUE;

    return FALSE;
}

/*
 * Log all changes to catch those sneaky bastards =)
 */
/* void olc_log( DESCRIPTOR_DATA *d, char *argument ) */
void olc_log( DESCRIPTOR_DATA *d, char *format, ... )
{
    ROOM_INDEX_DATA *room = d->character->dest_buf;
    OBJ_DATA *obj = d->character->dest_buf;
    CHAR_DATA *victim = d->character->dest_buf;
    char logline[MAX_STRING_LENGTH];
    va_list args;

    if ( !d )
    {
        bug( "olc_log: called with null descriptor", 0 );
        return;
    }

    va_start( args, format );
    vsprintf( logline, format, args );
    va_end( args );

    sprintf( log_buf, "Log %s:", d->character->name );
    if ( d->connected == CON_REDIT )
        sprintf( log_buf, "%s ROOM(%d): ", log_buf, room->vnum );
    else if ( d->connected == CON_OEDIT )
        sprintf( log_buf, "%s OBJ(%d): ", log_buf, obj->pIndexData->vnum );
    else if ( d->connected == CON_MEDIT )
    {
        if ( IS_NPC( victim ) )
            sprintf( log_buf, "%s MOB(%d): ", log_buf, victim->pIndexData->vnum );
        else
            sprintf( log_buf, "%s PLR(%s): ", log_buf, victim->name );
    }
    else
    {
	bug( "olc_log: called with a bad connected state", 0 );
	return;
    }
    sprintf( log_buf, "%s%s", log_buf, logline );
    log_string_plus( log_buf, LOG_BUILD, get_trust(d->character) );

    return;
}

/**************************************************************************
  Menu functions 
 **************************************************************************/

/*
 * Nice fancy redone Extra Description stuff :)
 */
void redit_disp_extradesc_prompt_menu( DESCRIPTOR_DATA *d )
{
    char buf[MAX_STRING_LENGTH];
    EXTRA_DESCR_DATA *ed;
    ROOM_INDEX_DATA *room = d->character->dest_buf;
    int counter = 0;

    for ( ed = room->first_extradesc; ed; ed = ed->next )
    {
	sprintf( buf, "&g%2d&w) %-40.40s\n\r", counter++, ed->keyword );
	send_to_char_color( buf, d->character );
    }
    send_to_char( "\n\rWhich extra description do you want to edit? ", d->character );
}

void redit_disp_extradesc_menu( DESCRIPTOR_DATA *d )
{
    ROOM_INDEX_DATA *room = d->character->dest_buf;
    int count = 0;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    if ( room->first_extradesc )
    {
	EXTRA_DESCR_DATA *ed;

	for ( ed = room->first_extradesc; ed; ed = ed->next )
	{
	    ch_printf_color( d->character, "&g%2d&w) Keyword: &O%s\n\r", ++count, ed->keyword );
	}
	send_to_char( "\n\r", d->character );
    }

    ch_printf_color( d->character, "&gA&w) Add a new description\n\r" );
    ch_printf_color( d->character, "&gR&w) Remove a description\n\r" );
    ch_printf_color( d->character, "&gQ&w) Quit\n\r" );
    ch_printf_color( d->character, "\n\rEnter choice: " );

    OLC_MODE(d) = REDIT_EXTRADESC_MENU;
}
	
/* For exits */
void redit_disp_exit_menu( DESCRIPTOR_DATA *d )
{
    /* char buf[MAX_STRING_LENGTH]; */
    ROOM_INDEX_DATA *room = d->character->dest_buf;
    EXIT_DATA *pexit;
    int cnt;

    OLC_MODE(d) = REDIT_EXIT_MENU;
    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for ( cnt = 0, pexit = room->first_exit; pexit; pexit = pexit->next )
    {
	ch_printf_color( d->character,
	/* sprintf( buf, */
	        "&g%2d&w) %-10.10s to %-5d.  Key: %d  Flags: %d  Keywords: %s.\n\r",
		++cnt,
		dir_name[pexit->vdir],
		pexit->to_room ? pexit->to_room->vnum : 0,
		pexit->key,
		pexit->exit_info,
		pexit->keyword[0] != '\0' ? pexit->keyword : "(none)" );
    }

    if ( room->first_exit )
	send_to_char( "\n\r", d->character );
    send_to_char_color( "&gA&w) Add a new exit\n\r", d->character );
    send_to_char_color( "&gR&w) Remove an exit\n\r", d->character );
    send_to_char_color( "&gQ&w) Quit\n\r", d->character );

    send_to_char( "\n\rEnter choice: ", d->character );

    return;
}

void redit_disp_exit_edit( DESCRIPTOR_DATA *d )
{
    /* ROOM_INDEX_DATA *room = d->character->dest_buf; */
    char flags[MAX_STRING_LENGTH];
    EXIT_DATA *pexit = d->character->spare_ptr;
    int i;

    flags[0] = '\0';
    for ( i = 0; i <= MAX_EXFLAG; i++ )
        if ( pexit->exit_info && IS_SET( pexit->exit_info, 1 << i ) )
	{
	    strcat( flags, ex_flags[i] );
	    strcat( flags, " " );
	}

    OLC_MODE(d) = REDIT_EXIT_EDIT;
    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    ch_printf_color( d->character, "&g1&w) Direction  : &c%s\n\r", dir_name[pexit->vdir] );
    ch_printf_color( d->character, "&g2&w) To Vnum    : &c%d\n\r", pexit->to_room ? pexit->to_room->vnum : -1 );
    ch_printf_color( d->character, "&g3&w) Key        : &c%d\n\r", pexit->key );
    ch_printf_color( d->character, "&g4&w) Keyword    : &c%s\n\r", (pexit->keyword && pexit->keyword[0] != '\0') ? pexit->keyword : "(none)" );
    ch_printf_color( d->character, "&g5&w) Flags      : &c%s\n\r", flags[0] != '\0' ? flags : "(none)" );
    ch_printf_color( d->character, "&g6&w) Description: &c%s\n\r", (pexit->description && pexit->description[0] != '\0') ? pexit->description : "(none)" );
    ch_printf_color( d->character, "&gQ&w) Quit\n\r" );
    ch_printf_color( d->character, "\n\rEnter choice: " );

    return;
}

void redit_disp_exit_dirs( DESCRIPTOR_DATA *d )
{
    int i;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for ( i = 0; i <= DIR_SOMEWHERE; i++ )
    {
	ch_printf_color( d->character, "&g%2d&w) %s\n\r", i, dir_name[i] );
    }
    send_to_char( "\n\rChoose a direction: ", d->character );

    return;
}

/* For exit flags */
void redit_disp_exit_flag_menu( DESCRIPTOR_DATA *d )
{
    EXIT_DATA *pexit = d->character->spare_ptr;
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    int i;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for ( i = 0; i <= MAX_EXFLAG; i++ )
    {
	if ( (1 << i == EX_RES1) || (1 << i == EX_RES2) || (1 << i == EX_PORTAL) )
	    continue;
	ch_printf_color( d->character, "&g%2d&w) %-20.20s\n\r", i+1, ex_flags[i] );
    }
    buf1[0] = '\0';
    for ( i = 0; i <= MAX_EXFLAG; i++ )
        if ( IS_SET( pexit->exit_info, 1 << i ) )
        {
	    strcat( buf1, ex_flags[i] );
            strcat( buf1, " " );
	}

    sprintf( buf, "\n\rExit flags: &c%s&w\n\r"
		  "Enter room flags, 0 to quit: ", buf1 );
    send_to_char_color( buf, d->character );
    OLC_MODE(d) = REDIT_EXIT_FLAGS;
}

/* For room flags */
void redit_disp_flag_menu( DESCRIPTOR_DATA *d )
{
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *room = d->character->dest_buf;
    int counter, columns = 0;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for ( counter = 0; counter < MAX_OLC_ITEMS_LIST; counter++ ) 
    {
	if ( counter == MAX_OLC_ITEMS_LIST-3 || counter == MAX_OLC_ITEMS_LIST-2 || counter == MAX_OLC_ITEMS_LIST )
	    continue;

	sprintf( buf, "&g%2d&w) %-20.20s ", counter + 1, r_flags[counter]);

	if( !(++columns % 2) )
	   strcat(buf, "\n\r");
		 send_to_char_color( buf, d->character );
    }
    ch_printf_color( d->character, "\n\rRoom flags: &c%s&w\n\rEnter room flags, 0 to quit : ",
		ext_flag_string( &room->room_flags, r_flags ) );
    OLC_MODE(d) = REDIT_FLAGS;
}

char *	const	sector_names [] =
{
"inside", "city", "field", "forest", "hills", "mountains", "water_swim",
"water_noswim", "underwater", "air", "desert", "???!", "oceanfloor",
"underground"
};

/* for sector type */
void redit_disp_sector_menu( DESCRIPTOR_DATA *d )
{
    char buf[MAX_STRING_LENGTH];
    int counter, columns = 0;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for ( counter = 0; counter < SECT_MAX; counter++ )
    {
	if ( counter == SECT_DUNNO )
	     continue;

	sprintf(buf, "&g%2d&w) %-20.20s ",
	    counter, sector_names[counter] );

	if(!(++columns % 2))
	    strcat( buf, "\n\r" );

	send_to_char_color( buf, d->character );
    }
    send_to_char("\r\nEnter sector type : ", d->character);
    OLC_MODE(d) = REDIT_SECTOR;
}

/* the main menu */
void redit_disp_menu( DESCRIPTOR_DATA *d )
{
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *room = d->character->dest_buf;
    char *sect;


    switch( room->sector_type )
    {
    default:			sect = "???!";		break;
    case SECT_INSIDE:		sect = "Inside";	break;
    case SECT_CITY:		sect = "City";		break;
    case SECT_FIELD:		sect = "Field";		break;
    case SECT_FOREST:		sect = "Forest";	break;
    case SECT_HILLS:		sect = "Hills";		break;
    case SECT_MOUNTAIN:		sect = "Mountains";	break;
    case SECT_WATER_SWIM:	sect = "Swim";		break;
    case SECT_WATER_NOSWIM:	sect = "Noswim";	break;
    case SECT_UNDERWATER:	sect = "Underwater";	break;
    case SECT_AIR:		sect = "Air";		break;
    case SECT_DESERT:		sect = "Desert";	break;
    case SECT_OCEANFLOOR:	sect = "Oceanfloor";	break;
    case SECT_UNDERGROUND:	sect = "Underground";	break;
    }

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    sprintf( buf,
	"&w-- Room number : [&c%d&w]      Room area: [&c%-30.30s&w]\n\r"
	"&g1&w) Name        : &O%s\n\r"
	"&g2&w) Description :\n\r&O%s"
  "&g3&w) Room flags  : &c%s\n\r"
	"&g4&w) Sector type : &c%s\n\r"
	"&g5&w) Tunnel      : &c%d\n\r"
	"&g6&w) TeleDelay   : &c%d\n\r"
	"&g7&w) TeleVnum    : &c%d\n\r"
	"&gA&w) Exit menu\n\r"
  	"&gB&w) Extra descriptions menu\r\n"
  	"&gQ&w) Quit\r\n"
  	"Enter choice : ",

	OLC_NUM(d),
	room->area ? room->area->name : "None????",
	room->name,
	room->description,
	ext_flag_string( &room->room_flags, r_flags),
	sect,
	room->tunnel,
	room->tele_delay,
	room->tele_vnum
  );
  set_char_color( AT_PLAIN, d->character );
  send_to_char_color(buf, d->character);

  OLC_MODE(d) = REDIT_MAIN_MENU;
}

EXTRA_DESCR_DATA *redit_find_extradesc( ROOM_INDEX_DATA *room, int number )
{
    int count = 0;
    EXTRA_DESCR_DATA *ed;

    for ( ed = room->first_extradesc; ed; ed = ed->next )
    {
	if ( ++count == number )
	    return ed;
    }

    return NULL;
}

void do_redit_reset( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *room = ch->dest_buf;
    EXTRA_DESCR_DATA *ed = ch->spare_ptr;

    switch( ch->substate )
    {
    case SUB_ROOM_DESC:
	if ( !ch->dest_buf )
	{
	    /* If theres no dest_buf, theres no object, so stick em back as playing */
	    send_to_char( "Fatal error, report to Tagith.\n\r", ch );
	    bug( "do_redit_reset: sub_obj_extra: NULL ch->dest_buf", 0 );
	    ch->substate = SUB_NONE;
	    ch->desc->connected = CON_PLAYING;
	    return;
	}
	STRFREE( room->description );
	room->description = copy_buffer( ch );
	stop_editing( ch );
	ch->dest_buf = room;
	ch->desc->connected = CON_REDIT;
	ch->substate = SUB_NONE;

	olc_log( ch->desc, "Edited room description" );
	redit_disp_menu( ch->desc );
	return;

    case SUB_ROOM_EXTRA:
	STRFREE( ed->description );
	ed->description = copy_buffer( ch );
	stop_editing( ch );
	ch->dest_buf = room;
	ch->spare_ptr = ed;
	ch->substate = SUB_NONE;
	ch->desc->connected = CON_REDIT;
	oedit_disp_extra_choice( ch->desc );
	OLC_MODE(ch->desc) = REDIT_EXTRADESC_CHOICE;
	olc_log( ch->desc, "Edit description for exdesc %s", ed->keyword );
	
	return;
    }
}

/**************************************************************************
  The main loop
 **************************************************************************/

void redit_parse( DESCRIPTOR_DATA *d , char *arg )
{
    ROOM_INDEX_DATA *room = d->character->dest_buf;
    ROOM_INDEX_DATA *tmp;
    EXIT_DATA *pexit = d->character->spare_ptr;
    EXTRA_DESCR_DATA *ed = d->character->spare_ptr;
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int number = 0;

    switch (OLC_MODE(d))
    {
    case REDIT_CONFIRM_SAVESTRING:
	switch (*arg)
	{
	case 'y':
	case 'Y':
	    /* redit_save_internally(d); */
	    sprintf( log_buf, "OLC: %s edits room %d", d->character->name, OLC_NUM(d));
	    log_string_plus( log_buf, LOG_BUILD, d->character->level );
	    cleanup_olc(d);
	    send_to_char("Room saved to memory.\r\n", d->character);
	    break;
	case 'n':
	case 'N':
	    cleanup_olc(d);
	    break;
	default:
	    send_to_char("Invalid choice!\r\n", d->character);
	    send_to_char("Do you wish to save this room internally? : ", d->character);
	    break;
	}
	return;

    case REDIT_MAIN_MENU:
	switch (*arg)
	{
	case 'q':
	case 'Q':
	    /* if (OLC_CHANGE(d))
	    { *. Something has been modified .*
		send_to_char( "Do you wish to save this room internally? : ", d->character );
		OLC_MODE(d) = REDIT_CONFIRM_SAVESTRING;
	    } 
	    else */
		cleanup_olc( d );
	    return;
	case '1':
	    send_to_char("Enter room name:-\r\n| ", d->character);
	    OLC_MODE(d) = REDIT_NAME;
	    break;
    	case '2':
	    OLC_MODE(d) = REDIT_DESC;
	    d->character->substate = SUB_ROOM_DESC;
	    d->character->last_cmd = do_redit_reset;

	    send_to_char( "Enter room description:-\r\n", d->character );
	    if ( !room->description )
		room->description = STRALLOC( "" );
	    start_editing( d->character, room->description );
            break;
	case '3':
	    redit_disp_flag_menu(d);
	    break;
	case '4':
	    redit_disp_sector_menu(d);
	    break;
	case '5':
	    send_to_char( "How many people can fit in the room? ", d->character );
	    OLC_MODE(d) = REDIT_TUNNEL;
	    break;
	case '6':
	    send_to_char( "How long before people are teleported out? ", d->character );
	    OLC_MODE(d) = REDIT_TELEDELAY;
	    break;
	case '7':
	    send_to_char( "Where are they teleported to? ", d->character );
	    OLC_MODE(d) = REDIT_TELEVNUM;
	    break;
	case 'a':
	case 'A':
	    redit_disp_exit_menu(d);
	    break;
	case 'b':
	case 'B':
	    redit_disp_extradesc_menu(d);
	    break;

	default:
	    send_to_char( "Invalid choice!", d->character );
	    redit_disp_menu(d);
	    break;
	}
    return;

    case REDIT_NAME:
	STRFREE( room->name );
	room->name = STRALLOC( arg );
	olc_log( d, "Changed name to %s", room->name );
	break;

    case REDIT_DESC:
	/* we will NEVER get here */
	bug( "Reached REDIT_DESC case in redit_parse", 0 );
	break;

    case REDIT_FLAGS:
        if ( is_number( arg ) )
        {
            number = atoi( arg );
            if ( number == 0 )
                break;
            else if ( number < 0 || number > 32 )
            {
                send_to_char( "Invalid flag, try again: ", d->character );
                return;
            }
            else
            {
		number -= 1; /* Offset for 0 */
             xTOGGLE_BIT( room->room_flags, 1 << number );
		olc_log( d, "%s the room flag %s", 
      xIS_SET( room->room_flags, 1 << number ) ? "Added" : "Removed",
		    r_flags[number] );
            }
        }
        else
        {
            while ( arg[0] != '\0' )
            {
                arg = one_argument( arg, arg1 );
                number = get_rflag( arg1 );
                if ( number > 0 )
                {
             xTOGGLE_BIT( room->room_flags, 1 << number );
	  	    olc_log( d, "%s the room flag %s", 
			xIS_SET( room->room_flags, 1 << number ) ? "Added" : "Removed",
			r_flags[number] );
		}
            }
        }
        redit_disp_flag_menu(d);
        return;

    case REDIT_SECTOR:
	number = atoi( arg );
	if ( number < 0 || number >= SECT_MAX )
	{
	    send_to_char("Invalid choice!", d->character);
	    redit_disp_sector_menu(d);
	    return;
	}
	else 
	    room->sector_type = number;
	olc_log( d, "Changed sector to %s", sector_names[number] );
	break;

    case REDIT_TUNNEL:
	number = atoi( arg );
	room->tunnel = URANGE( 0, number, 1000 );
	olc_log( d, "Changed tunnel amount to %d", room->tunnel );
	break;

    case REDIT_TELEDELAY:
	number = atoi( arg );
	room->tele_delay = number;
	olc_log( d, "Changed teleportation delay to %d", room->tele_delay );
	break;

    case REDIT_TELEVNUM:
	number = atoi( arg );
	room->tele_vnum = URANGE( 1, number, MAX_VNUM );
	olc_log( d, "Changed teleportation vnum to %d", room->tele_vnum );
	break;

    case REDIT_EXIT_MENU:
	switch ( UPPER( arg[0] ) )
	{
	default:
	    if ( is_number( arg ) )
	    {
		number = atoi( arg );
		pexit = get_exit_num( room, number );
		d->character->spare_ptr = pexit;
		redit_disp_exit_edit(d);
		return;
	    }
	    redit_disp_exit_menu( d );
	    return;
	case 'A':
	    OLC_MODE(d) = REDIT_EXIT_ADD;
	    redit_disp_exit_dirs( d );
	    return;
	case 'R':
	    OLC_MODE(d) = REDIT_EXIT_DELETE;
	    send_to_char( "Delete which exit? ", d->character );
	    return;
	case 'Q':
	    d->character->spare_ptr = NULL;    
	    break;
	}
	break;

    case REDIT_EXIT_EDIT:
	switch ( UPPER( arg[0] ) )
	{
	case 'Q':
	    d->character->spare_ptr = NULL;
	    redit_disp_exit_menu(d);
	    return;
	case '1':
	    /* OLC_MODE(d) = REDIT_EXIT_DIR;
	    redit_disp_exit_dirs(d); */
	    send_to_char( "This option can only be changed by remaking the exit.\n\r", d->character );
	    break;
	case '2':
	    OLC_MODE(d) = REDIT_EXIT_VNUM;
	    send_to_char( "Which room does this exit go to? ", d->character );
	    return;
	case '3':
	    OLC_MODE(d) = REDIT_EXIT_KEY;
	    send_to_char( "What is the vnum of the key to this exit? ", d->character );
	    return;
	case '4':
	    OLC_MODE(d) = REDIT_EXIT_KEYWORD;
	    send_to_char( "What is the keyword to this exit? ", d->character );
	    return;
	case '5':
	    OLC_MODE(d) = REDIT_EXIT_FLAGS;
	    redit_disp_exit_flag_menu(d);
	    return;
	case '6':
	    OLC_MODE(d) = REDIT_EXIT_DESC;
	    send_to_char( "Description:\n\r] ", d->character );
	    return;
	}
	redit_disp_exit_edit(d);
	return;

    case REDIT_EXIT_DESC:
	if ( !arg || arg[0] == '\0' )
	    pexit->description = STRALLOC( "" );
	else
	{
	    sprintf( buf, "%s\n\r", arg );
	    pexit->description = STRALLOC( buf );
	}
	olc_log( d, "Changed %s description to %s", dir_name[pexit->vdir], arg ? arg : "none" );
	redit_disp_exit_edit(d);
	return;

    case REDIT_EXIT_ADD:
	if ( is_number( arg ) )
	{
	    number = atoi( arg );
	    if ( number < DIR_NORTH || number > DIR_SOMEWHERE )
	    {
		send_to_char( "Invalid direction, try again: ", d->character );
		return;
	    }
	    d->character->tempnum = number;
	}
	else
	{
	    number = get_dir( arg );
	    pexit = get_exit( room, number );
	    if ( pexit )
	    {
		send_to_char( "An exit in that direction already exists.\n\r", d->character );
		redit_disp_exit_menu(d);
		return;
	    }
	    d->character->tempnum = number;
	}
	OLC_MODE(d) = REDIT_EXIT_ADD_VNUM;
	send_to_char( "Which room does this exit go to? ", d->character );
	return;

    case REDIT_EXIT_ADD_VNUM:
	number = atoi( arg );
	if ( (tmp = get_room_index( number ) ) == NULL )
	{
	    send_to_char( "Non-existant room.\n\r", d->character );
	    OLC_MODE(d) = REDIT_EXIT_MENU;
	    redit_disp_exit_menu(d);
	    return;
	}
	pexit = make_exit( room, tmp, d->character->tempnum );
	pexit->keyword		= STRALLOC( "" );
	pexit->description	= STRALLOC( "" );
	pexit->key		= -1;
	pexit->exit_info	= 0;
	act( AT_IMMORT, "$n reveals a hidden passage!", d->character, NULL, NULL, TO_ROOM );
	d->character->spare_ptr = pexit;

	olc_log( d, "Added %s exit to %d", dir_name[pexit->vdir], pexit->vnum );

	OLC_MODE(d) = REDIT_EXIT_EDIT;
	redit_disp_exit_edit(d);
	return;

    case REDIT_EXIT_DELETE:
	if ( !is_number( arg ) )
	{
	    send_to_char( "Exit must be specified in a number.\n\r", d->character );
	    redit_disp_exit_menu(d);
	}
	number = atoi( arg );
	pexit = get_exit_num( room, number );
	if ( !pexit )
	{
	    send_to_char( "That exit does not exist.\n\r", d->character );
	    redit_disp_exit_menu(d);
	}
	olc_log( d, "Removed %s exit", dir_name[pexit->vdir] );
	extract_exit( room, pexit );
	redit_disp_exit_menu( d );
	return;

    case REDIT_EXIT_VNUM:
	number = atoi( arg );
	if ( number < 0 || number > MAX_VNUM )
	{
	    send_to_char( "Invalid room number, try again : ", d->character);
	    return;
	}
	if ( get_room_index( number ) == NULL )
	{
	    send_to_char( "That room does not exist, try again: ", d->character );
	    return;
	}
	pexit->vnum = number;
	olc_log( d, "%s exit vnum changed to %d", dir_name[pexit->vdir], pexit->vnum );
	redit_disp_exit_menu(d);
	return;

    case REDIT_EXIT_KEYWORD:
	STRFREE( pexit->keyword );
	pexit->keyword = STRALLOC( arg );
	olc_log( d, "Changed %s keyword to %s", dir_name[pexit->vdir], pexit->keyword );
	redit_disp_exit_edit(d);
	return;

    case REDIT_EXIT_KEY:
	number = atoi( arg );
	if ( number < 0 || number > MAX_VNUM )
	    send_to_char( "Invalid vnum, try again: ", d->character );
	else
	{
	    pexit->key = number;
	    redit_disp_exit_edit(d);
	}
	olc_log( d, "%s key vnum is now %d", dir_name[pexit->vdir], pexit->key );
	return;

    case REDIT_EXIT_FLAGS:
	number = atoi(arg);
	if ( number == 0 )
	{
	    redit_disp_exit_edit(d);
	    return;
	}

	if ( (number < 0) || (number > MAX_EXFLAG+1) 
	|| ( 1 << (number-1) == EX_RES1)
	|| ( 1 << (number-1) == EX_RES2)
	|| ( 1 << (number-1) == EX_PORTAL) )
	{
	    send_to_char("That's not a valid choice!\r\n", d->character);
	    redit_disp_exit_flag_menu(d);
	}
	number -= 1;
	TOGGLE_BIT( pexit->exit_info, 1 << number );
	olc_log( d, "%s %s to %s exit",
	    IS_SET( pexit->exit_info, 1 << number ) ? "Added" : "Removed",
	    ex_flags[number],
	    dir_name[pexit->vdir] ); 
	redit_disp_exit_flag_menu(d);
	return;

    case REDIT_EXTRADESC_DELETE:
	ed = redit_find_extradesc( room, atoi(arg) );
	if ( !ed )
	{
	    send_to_char( "Not found, try again: ", d->character );
	    return;
	}
	olc_log( d, "Deleted exdesc %s", ed->keyword );
	UNLINK( ed, room->first_extradesc, room->last_extradesc, next, prev );
	STRFREE( ed->keyword );
	STRFREE( ed->description );
	DISPOSE( ed );
	top_ed--;
	redit_disp_extradesc_menu(d);
	return;

    case REDIT_EXTRADESC_CHOICE:
	switch ( UPPER( arg[0] ) )
	{
	case 'Q':
	    if ( !ed->keyword || !ed->description )
	    {
		send_to_char( "No keyword and/or description, junking...", d->character );
		UNLINK( ed, room->first_extradesc, room->last_extradesc, next, prev );
		STRFREE( ed->keyword );
		STRFREE( ed->keyword );
		DISPOSE( ed );
		top_ed--;
	    }
	    d->character->spare_ptr = NULL;
	    redit_disp_extradesc_menu(d);
	    return;
	case '1':
	    OLC_MODE(d) = REDIT_EXTRADESC_KEY;
	    send_to_char( "Keywords, seperated by spaces: ", d->character );
	    return;
	case '2':
	    OLC_MODE(d) = REDIT_EXTRADESC_DESCRIPTION;
	    d->character->substate = SUB_ROOM_EXTRA;
	    d->character->last_cmd = do_redit_reset;

	    send_to_char( "Enter new extradesc description: \n\r", d->character );
	    start_editing( d->character, ed->description );
	    return;
	}
	break;

    case REDIT_EXTRADESC_KEY:
	/* if ( SetRExtra( room, arg ) )
	{
	    send_to_char( "A extradesc with that keyword already exists.\n\r", d->character );
	    redit_disp_extradesc_menu(d);
	    return;
	} */
	olc_log( d, "Changed exkey %s to %s", ed->keyword, arg );
	STRFREE( ed->keyword );
	ed->keyword = STRALLOC( arg );
	oedit_disp_extra_choice(d);
	OLC_MODE(d) = REDIT_EXTRADESC_CHOICE;
	return;

    case REDIT_EXTRADESC_MENU:
	switch ( UPPER( arg[0] ) )
	{
	case 'Q':
	    break;
	case 'A':
	    CREATE( ed, EXTRA_DESCR_DATA, 1 );
	    LINK( ed, room->first_extradesc, room->last_extradesc, next, prev );
	    ed->keyword = STRALLOC( "" );
	    ed->description = STRALLOC( "" );
	    top_ed++;
	    d->character->spare_ptr = ed;
	    olc_log( d, "Added new exdesc" );

	    oedit_disp_extra_choice(d);
	    OLC_MODE(d) = REDIT_EXTRADESC_CHOICE;
	    return;
	case 'R':
	    OLC_MODE(d) = REDIT_EXTRADESC_DELETE;
	    send_to_char( "Delete which extra description? ", d->character );
	    return;
	default:
	    if ( is_number( arg ) )
	    {
		ed = redit_find_extradesc( room, atoi( arg ) );
		if ( !ed )
		{
		    send_to_char( "Not found, try again: ", d->character );
		    return;
		}
		d->character->spare_ptr = ed;
		oedit_disp_extra_choice(d);
		OLC_MODE(d) = REDIT_EXTRADESC_CHOICE;
	    }
	    else
		redit_disp_extradesc_menu(d);
	    return;
	}
	break;

    default:
	/* we should never get here */
	bug( "Reached default case in parse_redit", 0 );
	break;
    }
    /* Log the changes, so we can keep track of those sneaky bastards */
    /* Don't log on the flags cause it does that above */
    /* if ( OLC_MODE(d) != REDIT_FLAGS )
	olc_log( d, arg ); */

    /*. If we get this far, something has be changed .*/
    OLC_CHANGE(d) = TRUE;
    redit_disp_menu(d);
}

