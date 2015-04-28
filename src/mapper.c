/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.8 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
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
 *                           Room Mapper Module                             *
 ****************************************************************************/

/**************************************************************************
*  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
*  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
*                                                                         *
*  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
*  Chastain, Michael Quan, and Mitchell Tse.                              *
*                                                                         *
*  In order to use any part of this Merc Diku Mud, you must comply with   *
*  both the original Diku license in 'license.doc' as well the Merc       *
*  license in 'license.txt'.  In particular, you may not remove either of *
*  these copyright notices.                                               *
*                                                                         *
*  Dystopia Mud improvements copyright (C) 2000, 2001 by Brian Graversen  *
*                                                                         *
*  Much time and thought has gone into this software and you are          *
*  benefitting.  We hope that you share your changes too.  What goes      *
*  around, comes around.                                                  *
***************************************************************************
*  Converted for AFKMud 1.64 by Zarius (jeff@mindcloud.com)               *
*  Downloaded from http://www.mindcloud.com                               *
*  If you like the snippet let me know                                    *
***************************************************************************/
/**************************************************************************
 * 	                       Version History                              *
 **************************************************************************
 *  (v1.0) - Converted Automapper to AFKMud 1.64 and added additional     *
 *           directions and removed room desc code into a sep func        *
 **************************************************************************/

/*
   TO DO
   -----

   1. Add a way of displaying up and down directions effectively
 */
#include <stdio.h>
#include <string.h>
#include "mud.h"
#include "mapper.h"

bool check_blind( CHAR_DATA * ch );

/* The map itself */
MAP_TYPE dmap[MAPX + 1][MAPY + 1];

/* Take care of some repetitive code for later */
void get_exit_dir( int dir, int *x, int *y, int xorig, int yorig )
{
   /*
    * Get the next coord based on direction 
    */
   switch ( dir )
   {
      case DIR_NORTH:  /* North */
         *x = xorig;
         *y = yorig - 1;
         break;

      case DIR_EAST:   /* East */
         *x = xorig + 1;
         *y = yorig;
         break;

      case DIR_SOUTH:  /* South */
         *x = xorig;
         *y = yorig + 1;
         break;

      case DIR_WEST:   /* West */
         *x = xorig - 1;
         *y = yorig;
         break;

      case DIR_UP:  /* UP */
         break;

      case DIR_DOWN:   /* DOWN */
         break;

      case DIR_NORTHEAST: /* NE */
         *x = xorig + 1;
         *y = yorig - 1;
         break;

      case DIR_NORTHWEST: /* NW */
         *x = xorig - 1;
         *y = yorig - 1;
         break;

      case DIR_SOUTHEAST: /* SE */
         *x = xorig + 1;
         *y = yorig + 1;
         break;

      case DIR_SOUTHWEST: /* SW */
         *x = xorig - 1;
         *y = yorig + 1;
         break;

      default:
         *x = -1;
         *y = -1;
         break;
   }
}

char *get_exits( CHAR_DATA * ch )
{
   static char buf[MAX_STRING_LENGTH];
   EXIT_DATA *pexit;
   bool found = FALSE;

   buf[0] = '\0';

   if( !check_blind( ch ) )
      return buf;

   set_char_color( AT_EXITS, ch );

   mudstrlcpy( buf, "[Exits:", MAX_STRING_LENGTH );

   for( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
   {
      if( IS_IMMORTAL( ch ) )
         /*
          * Immortals see all exits, even secret ones 
          */
      {
         if( pexit->to_room )
         {
            found = TRUE;
            mudstrlcat( buf, " ", MAX_STRING_LENGTH );

            mudstrlcat( buf, capitalize( dir_name[pexit->vdir] ), MAX_STRING_LENGTH );

            /*
             * New code added to display closed, or otherwise invisible exits to immortals 
             * Installed by Samson 1-25-98 
             */
            if( IS_SET( pexit->exit_info, EX_CLOSED ) )
               mudstrlcat( buf, "->(Closed)", MAX_STRING_LENGTH );
            if( IS_SET( pexit->exit_info, EX_DIG ) )
               mudstrlcat( buf, "->(Dig)", MAX_STRING_LENGTH );
            if( IS_SET( pexit->exit_info, EX_WINDOW ) )
               mudstrlcat( buf, "->(Window)", MAX_STRING_LENGTH );
            if( IS_SET( pexit->exit_info, EX_HIDDEN ) )
               mudstrlcat( buf, "->(Hidden)", MAX_STRING_LENGTH );
            if( xIS_SET( pexit->to_room->room_flags, ROOM_DEATH ) )
               mudstrlcat( buf, "->(Deathtrap)", MAX_STRING_LENGTH );
         }
      }
      else
      {
         if( pexit->to_room
             && !IS_SET( pexit->exit_info, EX_SECRET )
             && ( !IS_SET( pexit->exit_info, EX_WINDOW ) || IS_SET( pexit->exit_info, EX_ISDOOR ) )
             && !IS_SET( pexit->exit_info, EX_HIDDEN ) )
         {
            found = TRUE;
            mudstrlcat( buf, " ", MAX_STRING_LENGTH );

            mudstrlcat( buf, capitalize( dir_name[pexit->vdir] ), MAX_STRING_LENGTH );

            if( IS_SET( pexit->exit_info, EX_CLOSED ) )
               mudstrlcat( buf, "->(Closed)", MAX_STRING_LENGTH );
            if( IS_AFFECTED( ch, AFF_DETECTTRAPS ) && xIS_SET( pexit->to_room->room_flags, ROOM_DEATH ) )
               mudstrlcat( buf, "->(Deathtrap)", MAX_STRING_LENGTH );
         }
      }
   }

   if( !found )
      mudstrlcat( buf, " none]", MAX_STRING_LENGTH );
   else
      mudstrlcat( buf, "]", MAX_STRING_LENGTH );
   mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
   return buf;
}

/* Clear one map coord */
void clear_coord( int x, int y )
{
   dmap[x][y].tegn = ' ';
   dmap[x][y].vnum = 0;
   dmap[x][y].depth = 0;
   dmap[x][y].sector = -1;
//   xCLEAR_BITS( dmap[x][y].info );
   dmap[x][y].can_see = TRUE;
}

/* Clear all exits for one room */
void clear_room( int x, int y )
{
   int dir, exitx, exity;

   /*
    * Cycle through the four directions 
    */
   for( dir = 0; dir < 4; ++dir )
   {
      /*
       * Find next coord in this direction 
       */
      get_exit_dir( dir, &exitx, &exity, x, y );

      /*
       * If coord is valid, clear it 
       */
      if( !BOUNDARY( exitx, exity ) )
         clear_coord( exitx, exity );
   }
}

/* This function is recursive, ie it calls itself */
void map_exits( CHAR_DATA * ch, ROOM_INDEX_DATA * pRoom, int x, int y, int depth )
{
   static char map_chars[11] = "|-|-UD/\\\\/";
   int door;
   int exitx = 0, exity = 0;
   int roomx = 0, roomy = 0;
   EXIT_DATA *pExit;

   /*
    * Setup this coord as a room - Change any symbols that can't be displayed here 
    */
   dmap[x][y].sector = pRoom->sector_type;
   switch ( pRoom->sector_type )
   {
      case SECT_INSIDE:
         dmap[x][y].tegn = 'O';
         dmap[x][y].sector = -1;
         break;

      case SECT_CITY:
         dmap[x][y].tegn = ':';
         break;

      case SECT_FIELD:
      case SECT_FOREST:
      case SECT_HILLS:
         dmap[x][y].tegn = '*';
         break;

      case SECT_MOUNTAIN:
         dmap[x][y].tegn = '@';
         break;

      case SECT_WATER_SWIM:
      case SECT_WATER_NOSWIM:
         dmap[x][y].tegn = '=';
         break;

      case SECT_AIR:
         dmap[x][y].tegn = '~';
         break;

      case SECT_DESERT:
         dmap[x][y].tegn = '+';
         break;

      default:
         dmap[x][y].tegn = 'O';
         dmap[x][y].sector = -1;
         bug( "%s: Bad sector type (%d) in room %d.", __func__, pRoom->sector_type, pRoom->vnum );
         break;
   }

   dmap[x][y].vnum = pRoom->vnum;
   dmap[x][y].depth = depth;
//   dmap[x][y].info = pRoom->room_flags;
   dmap[x][y].can_see = room_is_dark( pRoom );

   /*
    * Limit recursion 
    */
   if( depth > MAXDEPTH )
      return;

   /*
    * This room is done, deal with it's exits 
    */
   for( door = 0; door < 10; ++door )
   {
      /*
       * Skip if there is no exit in this direction 
       */
      if( !( pExit = get_exit( pRoom, door ) ) )
         continue;

      /*
       * Skip up and down until I can figure out a good way to display it 
       */
      if( door == 4 || door == 5 )
         continue;

      /*
       * Get the coords for the next exit and room in this direction 
       */
      get_exit_dir( door, &exitx, &exity, x, y );
      get_exit_dir( door, &roomx, &roomy, exitx, exity );

      /*
       * Skip if coords fall outside map 
       */
      if( BOUNDARY( exitx, exity ) || BOUNDARY( roomx, roomy ) )
         continue;

      /*
       * Skip if there is no room beyond this exit 
       */
      if( !pExit->to_room )
         continue;

      /*
       * Ensure there are no clashes with previously defined rooms 
       */
      if( ( dmap[roomx][roomy].vnum != 0 ) && ( dmap[roomx][roomy].vnum != pExit->to_room->vnum ) )
      {
         /*
          * Use the new room if the depth is higher 
          */
         if( dmap[roomx][roomy].depth <= depth )
            continue;

         /*
          * It is so clear the old room 
          */
         clear_room( roomx, roomy );
      }

      /*
       * No exits at MAXDEPTH 
       */
      if( depth == MAXDEPTH )
         continue;

      /*
       * No need for exits that are already mapped 
       */
      if( dmap[exitx][exity].depth > 0 )
         continue;

      /*
       * Fill in exit 
       */
      dmap[exitx][exity].depth = depth;
      dmap[exitx][exity].vnum = pExit->to_room->vnum;
//      dmap[exitx][exity].info = pExit->exit_info;
      dmap[exitx][exity].tegn = map_chars[door];
      dmap[exitx][exity].sector = -1;

      /*
       * More to do? If so we recurse 
       */
      if( depth < MAXDEPTH && ( ( dmap[roomx][roomy].vnum == pExit->to_room->vnum ) || ( dmap[roomx][roomy].vnum == 0 ) ) )
      {
         /*
          * Depth increases by one each time 
          */
         map_exits( ch, pExit->to_room, roomx, roomy, depth + 1 );
      }
   }
}

/* Reformat room descriptions to exclude undesirable characters */
void reformat_desc( char *desc )
{
   /*
    * Index variables to keep track of array/pointer elements 
    */
   unsigned int i = 0;
   int j = 0;
   char buf[MAX_STRING_LENGTH], *p;

   buf[0] = '\0';

   if( !desc )
      return;

   /*
    * Replace all "\n" and "\r" with spaces 
    */
   for( i = 0; i <= strlen( desc ); ++i )
   {
      if( ( desc[i] == '\r' ) || ( desc[i] == '\n' ) )
         desc[i] = ' ';
   }

   /*
    * Remove multiple spaces 
    */
   for( p = desc; *p != '\0'; ++p )
   {
      buf[j] = *p;
      ++j;

      /*
       * Two or more consecutive spaces? 
       */
      if( ( *p == ' ' ) && ( *( p + 1 ) == ' ' ) )
      {
         do
         {
            ++p;
         }
         while( *( p + 1 ) == ' ' );
      }
   }

   buf[j] = '\0';

   /*
    * Copy to desc 
    */
   mudstrlcpy( desc, buf, MAX_STRING_LENGTH );
}

int get_line( char *desc, size_t max_len )
{
   size_t i, j = 0;

   /*
    * Return if it's short enough for one line 
    */
   if( strlen( desc ) <= max_len )
      return 0;

   /*
    * Calculate end point in string without color 
    */
   for( i = 0; i <= strlen( desc ); ++i )
   {
      char dst[20];
      int vislen;

      switch ( desc[i] )
      {
         case '&':  /* NORMAL, Foreground colour */
         case '^':  /* BACKGROUND colour */
         case '}':  /* BLINK Foreground colour */
            *dst = '\0';
            vislen = 0;
            i += colorcode( &desc[i], dst, NULL, 20, &vislen ); /* Skip input token */
            j += vislen; /* Count output token length */
            break;   /* this was missing - if you have issues, remove it */

         default:   /* No conversion, just count */
            ++j;
            break;
      }
      
      if( j > max_len )
         break;
   }

   

   /*
    * End point is now in i, find the nearest space 
    */
   for( j = i; j > 0; --j )
   {
      if( desc[j] == ' ' )
         break;
   }

   /*
    * There could be a problem if there are no spaces on the line 
    */
   return j + 1;
}

char *whatColor( char *str, char *pos )
{
   static char col[2];

   col[0] = '\0';
   while( str != pos )
   {
      if( *str == '&' || *str == '^' || *str == '}' )
      {
         col[0] = *str;

         ++str;
         if( !str )
         {
            col[1] = '\0';
            break;
         }
         col[1] = *str;
      }
      ++str;
   }
   return col;
}

/* Display the map to the player */
void show_map( CHAR_DATA * ch, char *text )
{
   char buf[MAX_STRING_LENGTH * 2];
   int x, y, pos;
   char *p;
   bool alldesc = FALSE;   /* Has desc been fully displayed? */

   if( !text )
      alldesc = TRUE;

   pos = 0;
   p = text;
   buf[0] = '\0';

   /*
    * Show exits 
    */
   if( xIS_SET( ch->act, PLR_AUTOEXIT ) )
      snprintf( buf, MAX_STRING_LENGTH * 2, "%s%s", color_str( AT_EXITS, ch ), get_exits( ch ) );
   else
      mudstrlcpy( buf, "", MAX_STRING_LENGTH * 2 );

   /*
    * Top of map frame 
    */
   mudstrlcat( buf, "&z+-----------+&w ", MAX_STRING_LENGTH * 2 );
   if( !alldesc )
   {
      pos = get_line( p, 63 );
      if( pos > 0 )
      {
         mudstrlcat( buf, color_str( AT_RMDESC, ch ), MAX_STRING_LENGTH * 2 );
         strncat( buf, p, pos );
         p += pos;
      }
      else
      {
         mudstrlcat( buf, color_str( AT_RMDESC, ch ), MAX_STRING_LENGTH * 2 );
         mudstrlcat( buf, p, MAX_STRING_LENGTH * 2 );
         alldesc = TRUE;
      }
   }
   mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH * 2 );

   /*
    * Write out the main map area with text 
    */
   for( y = 0; y <= MAPY; ++y )
   {
      mudstrlcat( buf, "&z|&D", MAX_STRING_LENGTH * 2 );

      for( x = 0; x <= MAPX; ++x )
      {
         switch ( dmap[x][y].tegn )
         {
            case '-':
            case '|':
            case '\\':
            case '/':
               snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH * 2 ) - strlen( buf ), "&O%c&d", dmap[x][y].tegn );
               break;

            case '@':  // Character is standing here
               snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH * 2 ) - strlen( buf ), "&R%c&d", dmap[x][y].tegn );
               break;

            case 'O':  // Indoors
               snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH * 2 ) - strlen( buf ), "&w%c&d", dmap[x][y].tegn );
               break;

            case '=':
               snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH * 2 ) - strlen( buf ), "&B%c&d", dmap[x][y].tegn );
               break;

            case '~':
               snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH * 2 ) - strlen( buf ), "&C%c&d", dmap[x][y].tegn );
               break;

            case '+':
               snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH * 2 ) - strlen( buf ), "&Y%c&d", dmap[x][y].tegn );
               break;

            case '*':
               snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH * 2 ) - strlen( buf ), "&g%c&d", dmap[x][y].tegn );
               break;

            case 'X':
               snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH * 2 ) - strlen( buf ), "&R%c&d", dmap[x][y].tegn );
               break;

            case ':':
               snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH * 2 ) - strlen( buf ), "&Y%c&d", dmap[x][y].tegn );
               break;

            default:   // Empty space
               snprintf( buf + strlen( buf ), ( MAX_STRING_LENGTH * 2 ) - strlen( buf ), "%c", dmap[x][y].tegn );
               break;
         }
      }
      mudstrlcat( buf, "&z|&D ", MAX_STRING_LENGTH * 2 );

      /*
       * Add the text, if necessary 
       */
      if( !alldesc )
      {
         pos = get_line( p, 63 );
         char col[10], c[2];

         strcpy( c, whatColor( text, p ) );
         if( c[0] == '\0' )
            mudstrlcpy( col, color_str( AT_RMDESC, ch ), 10 );
         else
            snprintf( col, 10, "%s", c );

         if( pos > 0 )
         {
            mudstrlcat( buf, col, MAX_STRING_LENGTH * 2 );
            strncat( buf, p, pos );
            p += pos;
         }
         else
         {
            mudstrlcat( buf, col, MAX_STRING_LENGTH * 2 );
            mudstrlcat( buf, p, MAX_STRING_LENGTH * 2 );
            alldesc = TRUE;
         }
      }
      mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH * 2 );
   }

   /*
    * Finish off map area 
    */
   mudstrlcat( buf, "&z+-----------+&D ", MAX_STRING_LENGTH * 2 );
   if( !alldesc )
   {
      char col[10], c[2];
      pos = get_line( p, 63 );

      strcpy( c, whatColor( text, p ) );
      if( c[0] == '\0' )
         mudstrlcpy( col, color_str( AT_RMDESC, ch ), 10 );
      else
         snprintf( col, 10, "%s", c );

      if( pos > 0 )
      {
         mudstrlcat( buf, col, MAX_STRING_LENGTH * 2 );
         strncat( buf, p, pos );
         p += pos;
         mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH * 2 );
      }
      else
      {
         mudstrlcat( buf, col, MAX_STRING_LENGTH * 2 );
         mudstrlcat( buf, p, MAX_STRING_LENGTH * 2 );
         alldesc = TRUE;
      }
   }

   /*
    * Deal with any leftover text 
    */
   if( !alldesc )
   {
      char col[10], c[2];

      do
      {
         /*
          * Note the number - no map to detract from width 
          */
         pos = get_line( p, 78 );

         strcpy( c, whatColor( text, p ) );
         if( c[0] == '\0' )
            mudstrlcpy( col, color_str( AT_RMDESC, ch ), 10 );
         else
            snprintf( col, 10, "%s", c );

         if( pos > 0 )
         {
            mudstrlcat( buf, col, MAX_STRING_LENGTH * 2 );
            strncat( buf, p, pos );
            p += pos;
            mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH * 2 );
         }
         else
         {
            mudstrlcat( buf, col, MAX_STRING_LENGTH * 2 );
            mudstrlcat( buf, p, MAX_STRING_LENGTH * 2 );
            alldesc = TRUE;
         }
      }
      while( !alldesc );
   }
   mudstrlcat( buf, "&D\r\n", MAX_STRING_LENGTH * 2 );
   send_to_char( buf, ch );
}

/* Clear, generate and display the map */
void draw_room_map( CHAR_DATA * ch, const char *desc )
{
   int x, y;
   static char buf[MAX_STRING_LENGTH];

   mudstrlcpy( buf, desc, MAX_STRING_LENGTH );
   /*
    * Remove undesirable characters 
    */
   reformat_desc( buf );

   /*
    * Clear map 
    */
   for( y = 0; y <= MAPY; ++y )
   {
      for( x = 0; x <= MAPX; ++x )
      {
         clear_coord( x, y );
      }
   }

   /*
    * Start with players pos at centre of map 
    */
   x = MAPX / 2;
   y = MAPY / 2;

   dmap[x][y].vnum = ch->in_room->vnum;
   dmap[x][y].depth = 0;

   /*
    * Generate the map 
    */
   map_exits( ch, ch->in_room, x, y, 0 );

   /*
    * Current position should be a "X" 
    */
   dmap[x][y].tegn = '@';
   dmap[x][y].sector = -1;

   /*
    * Send the map 
    */
   show_map( ch, buf );
}
