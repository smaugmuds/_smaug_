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
 *         Dragonflight Module for use with Samson's Overland Module        *
 *                       Created by Ymris of Terahoun                       *
 ****************************************************************************/

/*                                                                          *
 * A 1000x1000 wilderness can be pretty daunting on foot, even with no mobs *  
 *  in it to slow you down.  So this is a dragonflight module to speed it   *
 *  up a little.                                                            *
 *                                                                          */

/* I've made this only work on the Overland for now. Working with calls from
 * inside areas will require a bit more tweaking than the original code
 * was aware of - Samson
 */

#include <math.h>
#include "mud.h"

LANDING_DATA *first_landing;
LANDING_DATA *last_landing;

/* Used to determine safe landing spots when flying */
sh_int get_terrain( int map, int x, int y );

/* Needed to guide the dragon in flight to specific areas, otherwise he takes the REALLY long way around */
double distance( int chX, int chY, int lmX, int lmY );
int calc_angle( int chX, int chY, int lmX, int lmY, int *ipDistan );

/*
 * Dragon landing Function
 */
void land_dragon( CHAR_DATA *ch, CHAR_DATA *dragon, bool arrived )
{
   char buf[MSL];
   int terrain;

	terrain = get_terrain( ch->map, dragon->dcoordx, dragon->dcoordy );

	if( dragon->backtracking )
	{
		do_look( ch, "auto" );
		terrain = get_terrain( ch->map, dragon->x, dragon->y );
	}

	/* If it's safe, then land. */

	if( terrain != SECT_WATER_SWIM 
	 && terrain != SECT_WATER_NOSWIM 
	 && terrain != SECT_AIR 
	 && terrain != SECT_UNDERWATER 
	 && terrain != SECT_RIVER 
	 && terrain != SECT_OCEANFLOOR 
	 && terrain != SECT_UNDERGROUND 
	 && terrain != SECT_OCEAN 
	 && terrain != SECT_LAVA 
	 && terrain != SECT_TREE 
	 && terrain != SECT_STONE 
	 && terrain != SECT_QUICKSAND 
	 && terrain != SECT_WALL 
	 && terrain != SECT_GLACIER 
	 && terrain != SECT_EXIT )
	{
		if( dragon->backtracking )
		{
			arrived = TRUE;
			dragon->dcoordx = dragon->x;
			dragon->dcoordy = dragon->y;
		}

		if( !IS_NPC(ch) && arrived )
		{
			ch->x = dragon->dcoordx;
			ch->y = dragon->dcoordy;
			ch->dragonflight = FALSE;
		}
	
		xTOGGLE_BIT( dragon->act, ACT_NOWANDER );	/* Keeps him from running around so much */
		dragon->map = dragon->my_rider->map;
		dragon->x = dragon->my_rider->x;
		dragon->y = dragon->my_rider->y;
		dragon->backtracking = FALSE;
		dragon->dragonflight = FALSE;
		
		if( IS_ACT_FLAG( dragon, ACT_MOUNTED ) )
		{
			do_look( ch, "auto" );
			act( AT_ACTION, "$n flares sharply and touches down.", dragon, NULL, NULL, TO_ROOM );
		      return;
		} 
		else 
		{
			act( AT_ACTION, "$n banks in a spiral, touching down on the ground in front of you.", dragon, NULL, NULL, TO_ROOM );
			return;
		}
	}
	else 
	{
	            /* else go back one step, try to land again */
			sprintf( buf, "%s surveys the terrain below and decides to look for a safer landing site.\r\n", dragon->short_descr );
			send_to_char( buf, ch );
			dragon->backtracking = TRUE;
			dragon->dcoordx = dragon->lcoordx;
			dragon->dcoordy = dragon->lcoordy;
			return;
	}
	return;
}

/*
 * Dragonflight Function
 */
void fly_dragon( CHAR_DATA *ch, CHAR_DATA *dragon )
{
	CHAR_DATA *pair;
	int speed;		/* Speed dragon moves measured in rooms.  Salt to taste. */
	int dist, angle;

	/* Reset the boredom counter */
	dragon->zzzzz = 0;
	
	/* Define if dragon is mounted, or incoming */
	if( ch && IS_ACT_FLAG(dragon, ACT_MOUNTED) )
	{
		pair = ch;
		speed = 10;		/* Dragon with player flies this fast */
	}
	else
	{
		pair = dragon;
		speed = 50;		/* empty dragon flies 5x faster *shrug*  */
	}

	/* If dragon is close to the landing site... */
	if((( dragon->y - dragon->dcoordy ) <= speed ) 
		&& ( dragon->y - dragon->dcoordy ) >= -speed
		&& (( dragon->x - dragon->dcoordx ) <= speed ) 
		&& ( dragon->x - dragon->dcoordx ) >= -speed )
	{
		land_dragon( pair, dragon, TRUE );
		return;
    	}

	/* up up and away */

	dist = distance( dragon->x, dragon->y, dragon->dcoordx, dragon->dcoordy );
	angle = calc_angle( dragon->x, dragon->y, dragon->dcoordx, dragon->dcoordy, &dist );

	if( angle == -1 )
	   dragon->heading = -1;
	else if( angle >= 360 )
	   dragon->heading = DIR_NORTH;
	else if( angle >= 315 )
		dragon->heading = DIR_NORTHWEST;
	else if( angle >= 270 )
		dragon->heading = DIR_WEST;
	else if( angle >= 225 )
		dragon->heading = DIR_SOUTHWEST;
	else if( angle >= 180 )
		dragon->heading = DIR_SOUTH;
	else if( angle >= 135 )
		dragon->heading = DIR_SOUTHEAST;
	else if( angle >= 90 )
		dragon->heading = DIR_EAST;
	else if( angle >= 45 )
		dragon->heading = DIR_NORTHEAST;
	else if( angle >= 0 )
		dragon->heading = DIR_NORTH;

	/* move towards dest in steps of "speed" rooms  (salt to taste) */
      switch( dragon->heading )
      {
           	case DIR_NORTH:
				pair->y = pair->y - speed;
				if( pair == ch )
				{
					dragon->y = dragon->y - speed;
				}
			 	break;
           	case DIR_EAST:
				pair->x = pair->x + speed; 
				if( pair == ch )
				{
					dragon->x = dragon->x + speed;
				}
				break;
           	case DIR_SOUTH:
				pair->y = pair->y + speed; 
				if( pair == ch )
				{
					dragon->y = dragon->y + speed;
				}
				break;
           	case DIR_WEST:
				pair->x = pair->x - speed; 
				if( pair == ch )
				{
					dragon->x = dragon->x - speed;
				}
				break;

           	case DIR_NORTHEAST:
                	      pair->x = pair->x + speed; 
                	      pair->y = pair->y - speed; 
				if( pair == ch )
				{
					dragon->x = dragon->x + speed;
					dragon->y = dragon->y - speed;
				}
				break;
           	case DIR_NORTHWEST:
                	      pair->x = pair->x - speed; 
                	      pair->y = pair->y - speed; 
				if( pair == ch )
				{
					dragon->x = dragon->x - speed;
					dragon->y = dragon->y - speed;
				}
				break;
           	case DIR_SOUTHEAST:
                	      pair->x = pair->x + speed; 
                	      pair->y = pair->y + speed; 
				if( pair == ch )
				{
					dragon->x = dragon->x + speed;
					dragon->y = dragon->y + speed;
				}
				break;
           	case DIR_SOUTHWEST:
                	      pair->x = pair->x - speed; 
                	      pair->y = pair->y + speed; 
				if( pair == ch )
				{
					dragon->x = dragon->x - speed;
					dragon->y = dragon->y + speed;
				}
				break;
           	default:
               	break;
      } 

	/* Reversed the order of these calls because of how the Overland clears the screen - Samson */
	if( IS_ACT_FLAG( dragon, ACT_MOUNTED ) )
	  do_look( pair, "auto" );

	/* If dragon is looking for a better spot....*/
	if( dragon->backtracking )
	{
		land_dragon( pair, dragon, FALSE );
	}

	return;
}

/*
 * Create a Dragon 
 */
void pop_dragon( CHAR_DATA *ch, char *location )
{
	char buf[MSL];
	MOB_INDEX_DATA *vdragon = NULL;
	CHAR_DATA *dragon;
	int dnum = 0;
	
	/* Set this to the range of vnums assigned to your dragons */
   	dnum = number_range( 11072, 11078 );
	vdragon = get_mob_index( dnum );

	if( !vdragon )
	{
	   bug( "pop_dragon: Vnum not found for dragon %d" );
	   return;
	}
	dragon = create_mobile( vdragon );
	
	/* pick a random set of coordinates */
	/* and create dragon at the coords */
      /* Dragon can be safely spawned using the same room as the PC calling it - Samson */
	char_to_room( dragon, ch->in_room );
	SET_ACT_FLAG( dragon, ACT_ONMAP );
	dragon->dragonflight = TRUE;
	dragon->heading = -1;
	dragon->map = ch->map;
	dragon->x = number_range( 0, MAX_X );
	dragon->y = number_range( 0, MAX_Y );

	sprintf( buf, "Dragon spawned on %s at coords: %dX, %dY.", map_names[dragon->map], dragon->x, dragon->y );
	log_string( buf );

	/* Bond the player and dragon together */
	ch->my_dragon = dragon;
	dragon->my_rider = ch;

	/* Set the launch coords for backtracking, if needed later */
	dragon->lcoordx = ch->x;
	dragon->lcoordy = ch->y;

	/* fly dragon to player location */
	dragon->dcoordx = ch->x;
	dragon->dcoordy = ch->y;

	fly_dragon( NULL, dragon );
	
	return;
}

/*
 * Remove a Dragon when it is no longer needed
 */
void purge_dragon( CHAR_DATA *ch, CHAR_DATA *dragon )
{
	int four_corners = 0;
	
	/* Bye-bye-birdie!  Pick random coords */
	
	four_corners = number_range( 1,4 );
	switch( four_corners )
	{
		case 1:	dragon->dcoordx = 0; dragon->dcoordy = 0;
		case 2:	dragon->dcoordx = 0; dragon->dcoordy = MAX_Y;
		case 3:	dragon->dcoordx = MAX_X; dragon->dcoordy = 0;
		case 4:	dragon->dcoordx = MAX_X; dragon->dcoordy = MAX_Y;
	}

	act( AT_ACTION, "$n grows restless and takes to the sky in a gust of wind.\r\n", dragon, NULL, NULL, TO_ROOM );

	/* Release the player from the dragon */
	if( ch->mount )
	   do_dismount( ch, "" );
	ch->has_dragon = FALSE;
	ch->my_dragon = NULL;

	/* fly_dragon to one of the four corners */
	fly_dragon( NULL, dragon );
	
	/* poof the magic dragon (lived by the sea)  *cough* */
	dragon->timer = 10; /* When timer expires, dragon will be extracted - Samson */
	
	return;
}

/*
 * Call a Dragon 
 */
void do_call( CHAR_DATA *ch, char *argument )
{
	char buf[MSL];
	char site[MSL];
	int dragon_dough = 0;
	int terrain = 0;
	
	site[0] = '\0';

	/* Sanity checks Reasons why a dragon wouldn't want to answer*/
	/* You a smelly mobbie?? */
	if( IS_NPC(ch) )
	{
		send_to_char( "Sorry, mobs cannot use dragons to get around.\r\n", ch );
		return;

	}

	/* Simplifies things to keep dragons on the Overland only - Samson */
      if( !IS_PLR_FLAG( ch, PLR_ONMAP ) )
	{
	   send_to_char( "Dragons may only be called to the Overland.\r\n", ch );
	   return;
	}

	/* already has a dragon */
	if( ch->my_dragon )
	{
		send_to_char( "A dragon has already claimed you, and dragons are jealous creatures.\r\n", ch );
		return;
	}

	/* Inaccessible Sectors for dragons */
      	
	terrain = get_terrain( ch->map, ch->x, ch->y );

	if(terrain == SECT_WATER_SWIM 
	|| terrain == SECT_WATER_NOSWIM 
	|| terrain == SECT_UNDERWATER 
	|| terrain == SECT_RIVER 
	|| terrain == SECT_OCEANFLOOR 
	|| terrain == SECT_OCEAN)
	{
		send_to_char( "Dragons do not enjoy swimming enough to answer your call.\r\n", ch );
		return;
	}

	if ( terrain == SECT_UNDERGROUND )
	{
		send_to_char( "Dragons do not like such cramped areas.\r\n", ch );
		return;
	}

	/* Calculate cost of a dragon   salt to taste*/
	dragon_dough = ch->level * 1000;
	
	/* Check if player has the cash */
	if( ch->gold < dragon_dough )
	{
		send_to_char( "You do not have the price of dragonfare with you.\r\n", ch );
		return;
	}
	
	/* Where is the player? */
	sprintf( buf, "%d %d %d", ch->map, ch->x, ch->y );

	/* Debit the player */
	ch->gold = ch->gold - dragon_dough;
	act( AT_ACTION, "Your purse feels lighter but you feel confident a dragon is on the way.", ch, NULL, NULL, TO_CHAR );
	act( AT_ACTION, "$n calls a dragon.", ch, NULL, NULL, TO_ROOM );

	/* call pop_dragon */
	pop_dragon( ch, buf );

      return;
}

void do_release( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *dragon;

	if( !ch->my_dragon )
	{
		send_to_char( "You have no dragon summoned.\r\n", ch );
		return;
	}

	if( ch->dragonflight )
	{
		send_to_char( "Dragons are not equipped with parachutes.  Wait until you land.\r\n", ch );
		return;
	}

	if( ch->mount )
	   do_dismount( ch, "" );

	dragon = ch->my_dragon;
	act( AT_ACTION, "Your thank the dragon for its service and dismiss it.", ch, NULL, NULL, TO_CHAR );
	act( AT_ACTION, "$n dismisses $s dragon.", ch, NULL, NULL, TO_ROOM );

	ch->my_dragon = NULL;
	purge_dragon( ch, dragon );

	return;
}

/*
 * Command to Fly a Dragon 
 */
void do_fly( CHAR_DATA *ch, char *argument )
{
	char command[MIL];
	CHAR_DATA *dragon = NULL;
	LANDING_DATA *landing;
	char areaname[MIL];

    	one_argument( argument, command );
	areaname[0] = '\0';

      if( !IS_PLR_FLAG( ch, PLR_ONMAP ) )
      {
	   send_to_char( "This command can only be used from the Overland.\r\n", ch );
	   return;
	}

      if( !ch->my_dragon )
	{
	   send_to_char( "You have not called for a dragon, how do you expect to take flight?\r\n", ch );
	   return;
	}

	if( !ch->mount )
	{
		send_to_char( "You aren't mounted on a dragon.\r\n", ch );
		return;
	}

      if( ch->mount && ch->mount != ch->my_dragon )
	{
	   send_to_char( "You are mounted upon the wrong creature! Dismount first.\r\n", ch );
	   return;
	}

	dragon = ch->mount;
	dragon->backtracking = FALSE;

	if( !ch->dragonflight )
	{
		act( AT_ACTION, "You hold tight to the dragon's scaly hide as it leaps into the sky.", ch, NULL, NULL, TO_CHAR );
		act( AT_ACTION, "$n holds on tight as $s dragon leaps into the sky.", ch, NULL, NULL, TO_ROOM );

	     /* Removed the whole chunk dealing with removing you from a regular zone.
		* It's far less complex to keep this all on the overland anyway - Samson
		*/
      }
      else 
      {
		act( AT_ACTION, "You lean against the dragon's flank and he banks, changing course.", ch, NULL, NULL, TO_CHAR );
	}

    if( !str_cmp( command, "" ) ) /* No argument passed */
    {
	act( AT_ACTION, "You circle high above the landscape.", ch, NULL, NULL, TO_CHAR );
	act( AT_ACTION, "$n riding a $s circles high overhead.", ch, NULL, NULL, TO_ROOM );

	dragon->dcoordx = ch->x;
	dragon->dcoordy = ch->y;
	ch->dragonflight = TRUE;
	dragon->heading = -1;
	return;
    }

	else if( !str_cmp( command, "north" ) || !str_cmp( command, "n" ) )
	{
		dragon->dcoordx = dragon->x;
		dragon->dcoordy = 0;
		ch->dragonflight = TRUE;
		dragon->heading = DIR_NORTH;
		fly_dragon( ch, dragon );
		return;
	}
	else if( !str_cmp( command, "northeast" ) || !str_cmp( command, "ne" ) )
	{
		dragon->dcoordx = MAX_X - 1;
		dragon->dcoordy = 0;
		ch->dragonflight = TRUE;
		dragon->heading = DIR_NORTHEAST;
		fly_dragon( ch, dragon );
		return;
	}
	else if( !str_cmp( command, "east" ) || !str_cmp( command, "e" ) )
	{
		dragon->dcoordx = MAX_X - 1;
		dragon->dcoordy = dragon->y;
		ch->dragonflight = TRUE;
		dragon->heading = DIR_EAST;
		fly_dragon( ch, dragon );
		return;
	}
	else if( !str_cmp( command, "south" ) || !str_cmp( command, "s" ) )
	{
		dragon->dcoordx = dragon->x;
		dragon->dcoordy = MAX_Y - 1;
		ch->dragonflight = TRUE;
		dragon->heading = DIR_SOUTH;
		fly_dragon( ch, dragon );
		return;
	}
	else if( !str_cmp( command, "west" ) || !str_cmp( command, "w" ) )
	{
		dragon->dcoordx = 0;
		dragon->dcoordy = dragon->y;
		ch->dragonflight = TRUE;
		dragon->heading = DIR_WEST;
		fly_dragon( ch, dragon );
		return;
	}
      else 
	{
	      for( landing = first_landing; landing; landing = landing->next )
		{
			if( !str_prefix( command,landing->area ) )
			{
				if( landing->map != ch->map )
				{
				   /* Simplifies things. Especially since it would look funny to see alien terrain below you - Samson */
				   send_to_char( "The dragon refuses to fly you to another continent.\r\n", ch );
				   return;
				}
				dragon->dcoordx = landing->x;
				dragon->dcoordy = landing->y;
				ch->dragonflight = TRUE;
				fly_dragon( ch, dragon );
				return;
			}
		}
		send_to_char( "That location is not known to the dragon.\r\n", ch );
		dragon->dcoordx = ch->x;
		dragon->dcoordy = ch->y;
	}
}

/*
 * Command to Land a Dragon 
 */
void do_land( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *dragon = ch->my_dragon;

	if( !dragon || !ch->mount )
	{
		send_to_char( "You aren't flying on a dragon.\r\n", ch );
		return;
	}

	act( AT_ACTION, "You press your heels into the dragons back and he banks sharply, descending to the ground.", ch, NULL, NULL, TO_CHAR );
	act( AT_ACTION, "$n, riding a dragon, banks sharply overhead and descends to the ground.", ch, NULL, NULL, TO_ROOM );

	dragon->dcoordx = ch->x;
	dragon->dcoordy = ch->y;
	land_dragon( ch, dragon, TRUE );

	return;
}

/* 
 * Timing function used by update.c
 */
void dflight_update( void )
{
	CHAR_DATA *ch;

    /* Safer to scan the list from start to end */
    for ( ch = first_char; ch; ch = ch->next )
    {
		if( IS_NPC(ch) && ch->dragonflight )
		    fly_dragon( ch->my_rider, ch );

		if( !IS_NPC(ch) && ch->dragonflight )
		    fly_dragon( ch, ch->my_dragon );

 		/* Dragon idling Function*/
		if( IS_NPC(ch) && ch->my_rider && !ch->dragonflight )
		{
			ch->zzzzz++;

			/* First boredom warning   salt to taste */
			if( ch->zzzzz == 100 )
			{
				act( AT_ACTION, "$n shakes its massive shoulders, quivering out to its wingtips.\r\n", ch, NULL, NULL, TO_ROOM );
			}
		
			/* Second boredom warning   salt to taste */
			if( ch->zzzzz == 200 )
			{
				act( AT_ACTION, "$n snorts loudly, sending a cloud of smoke into the air.\r\n", ch, NULL, NULL, TO_ROOM );
			}
		
			/* Third strike, yeeeer out!   salt to taste */
			if( ch->zzzzz == 300 )
			   purge_dragon( ch->my_rider, ch );
		}
    }
}

/*
 * Landing Site Stuff
 *   I take no credit for originality here on down.  This is 
 *   DIRECTLY "ADAPTED" from Samson's landmark code in overland.c
 */
#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )				\
				if ( !str_cmp( word, literal ) )	\
				{									\
				      field = value;				\
				      fMatch = TRUE;				\
				      break;						\
				}

void fread_landing_sites( LANDING_DATA *landing, FILE *fp )
{
     char buf[MSL];
     char *word;
     bool fMatch;

     for ( ; ; )
     {
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
		return;
	    }
	    break;

	case 'C':
	    if( !str_cmp( word, "Coordinates" ) )
	    {
		landing->map = fread_number( fp );
		landing->x = fread_number( fp );
		landing->y = fread_number( fp );
	    fMatch = TRUE;
		break;
	    }
	    break;

	case 'A':
	    KEY("Area", landing->area, fread_string(fp)); 
	    fMatch = TRUE;
	    break;
	}

	if ( !fMatch )
	{
	    sprintf( buf, "Fread_landing: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}

void load_landing_sites( void )
{
     char filename[32];
     LANDING_DATA *landing;
     FILE *fp;
     
     first_landing = NULL;
     last_landing  = NULL;

     sprintf( filename, "%s%s", MAP_DIR, LANDING_SITE_FILE );

     if ( ( fp = fopen( filename, "r" ) ) != NULL )
     {
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
		bug( "Load_landing_sites: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "LANDING_SITE" ) )
	    {
		CREATE( landing, LANDING_DATA, 1 );
		fread_landing_sites( landing, fp );
		LINK( landing, first_landing, last_landing, next, prev );
		continue;
	    }
	    else
            if ( !str_cmp( word, "END" ) )
	        break;
	    else
	    {
		bug( "Load_landing_sites: bad section: %s.", word );
		continue;
	    }
	}
	FCLOSE( fp );
    }

    return;
}

void save_landing_sites( void )
{
   LANDING_DATA *landing;
   FILE *fp;
   char filename[32];

   sprintf( filename, "%s%s", MAP_DIR, LANDING_SITE_FILE );

   FCLOSE( fpReserve );
   if ( ( fp = fopen( filename, "w" ) ) == NULL )
   {
	bug( "save_landing_sites: fopen", 0 );
	perror( filename );
   }
   else
   {
	for ( landing = first_landing; landing; landing = landing->next )
      {
	  fprintf( fp, "#LANDING_SITE\n" );
	  fprintf( fp, "Coordinates	%d %d %d\n", landing->map, landing->x, landing->y );
	  fprintf( fp, "Area	%s~\n", landing->area );
	  fprintf( fp, "End\n\n" );
	}
	fprintf( fp, "#END\n" );
	FCLOSE(fp);
   }
   fpReserve = fopen ( NULL_FILE, "r" );
   return;
}

LANDING_DATA *check_landing_site( int map, int x, int y )
{
   LANDING_DATA *landing;

   for( landing = first_landing; landing; landing = landing->next )
   {
	if( landing->map == map )
	{
	   if( landing->x == x && landing->y == y )
		return landing;
	}
   }
   return NULL;
}

void add_landing( int map, int x, int y )
{
   LANDING_DATA *landing;

   CREATE( landing, LANDING_DATA, 1 );
   LINK( landing, first_landing, last_landing, next, prev );
   landing->map = map;
   landing->x = x;
   landing->y = y;
   landing->area = STRALLOC( "" );

   save_landing_sites();

   return;
}

void delete_landing_site( LANDING_DATA *landing )
{
   if( !landing )
   {
	bug( "delete_landing_site: Trying to delete NULL landing site.", 0 );
	return;
   }

   UNLINK( landing, first_landing, last_landing, next, prev );
   STRFREE( landing->area );
   DISPOSE( landing );

   save_landing_sites();

   return;
}

/* Support command to list all landing sites currently loaded */
void do_landing_sites( CHAR_DATA *ch, char *argument )
{
   LANDING_DATA *landing;

   if( !first_landing )
   {
	send_to_char( "No landing sites defined.\r\n", ch );
	return;
   }

   send_to_pager( "Continent | Coordinates | Area\r\n", ch );
   send_to_pager( "--------------------------------------------\r\n", ch );

   for( landing = first_landing; landing; landing = landing->next )
   {
	pager_printf( ch, "%-10s  %-4dX %-4dY   %s   \r\n", 
	   map_names[landing->map], landing->x, landing->y, landing->area );
   }
   return;
}

/* OLC command to add/delete/edit landing site information */
void do_setlanding( CHAR_DATA *ch, char *argument )
{
   LANDING_DATA *landing = NULL;
   char arg[MIL];

   if( IS_NPC(ch) )
   {
	send_to_char( "Sorry, NPCs have to walk.\r\n", ch );
	return;
   }

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' || !str_cmp( arg, "help" ) )
   {
	send_to_char( "Usage: setlanding add\r\n", ch );
	send_to_char( "Usage: setlanding delete\r\n", ch );
	send_to_char( "Usage: setlanding area <area name>\r\n", ch );
	return;
   }

   landing = check_landing_site( ch->map, ch->x, ch->y );

   if( !str_cmp( arg, "add" ) )
   {
	if( landing )
	{
	   send_to_char( "There's already a landing site at this location.\r\n", ch );
	   return;
	}
	add_landing( ch->map, ch->x, ch->y );
	send_to_char( "Landing site added.\r\n", ch );
	return;
   }

   if( !landing )
   {
      send_to_char( "There is no landing site here.\r\n", ch );
	return;
   }

   if( !str_cmp( arg, "delete" ) )
   {
	delete_landing_site( landing );
	send_to_char( "Landing site deleted.\r\n", ch );
	return;
   }

   if( !str_cmp( arg, "area" ) )
   {
	STRFREE( landing->area );
	landing->area = STRALLOC( argument );
	save_landing_sites();
	send_to_char( "Area set.\r\n", ch );
	return;
   }
 
   do_setlanding( ch, "" );
   return;
}
