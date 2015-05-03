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
     |           -*- Calendar Handler/Seasonal Updates Module -*-          |
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

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include "mud.h"

HOLIDAY_DATA *first_holiday;
HOLIDAY_DATA *last_holiday;

HOLIDAY_DATA *get_holiday( short month, short day )
{
   HOLIDAY_DATA *holiday;
   for( holiday = first_holiday; holiday; holiday = holiday->next )
      if( month + 1 == holiday->month && day + 1 == holiday->day )
         return holiday;
   return NULL;
}

void free_holiday( HOLIDAY_DATA * day )
{
   UNLINK( day, first_holiday, last_holiday, next, prev );
   DISPOSE( day->announce );
   DISPOSE( day->name );
   DISPOSE( day );
   return;
}

void free_holidays( void )
{
   HOLIDAY_DATA *day, *day_next;

   for( day = first_holiday; day; day = day_next )
   {
      day_next = day->next;
      free_holiday( day );
   }
   return;
}

void
do_holidays( CHAR_DATA *ch, char *argument)
{
   HOLIDAY_DATA *day;

   send_to_pager( _("&RHoliday		       &YMonth	        &GDay\n"), ch );
   send_to_pager( "&g----------------------+----------------+---------------\r\n", ch );

   for( day = first_holiday; day; day = day->next )
      pager_printf( ch, "&G%-21s	&g%-11s	%-2d\r\n", day->name, month_name[day->month - 1], day->day );

   return;
}

/* Read in an individual holiday */
void fread_day( HOLIDAY_DATA * day, FILE * fp )
{
   const char *word;
   bool fMatch;

   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case 'A':
            KEY( "Announce", day->announce, fread_string_nohash( fp ) );
            break;

         case 'D':
            KEY( "Day", day->day, fread_number( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !day->announce )
                  day->announce = str_dup( _("Today is a holiday, but who the hell knows which one.") );
               return;
            }
            break;

         case 'M':
            KEY( "Month", day->month, fread_number( fp ) );
            break;

         case 'N':
            KEY( "Name", day->name, fread_string_nohash( fp ) );
            break;
      }

      if( !fMatch )
         bug( "fread_day: no match: %s", word );
   }
}

/* Load the holiday file */
void load_holidays( void )
{
   char filename[256];
   HOLIDAY_DATA *day;
   FILE *fp;
   short daycount;

   first_holiday = NULL;
   last_holiday = NULL;

   snprintf( filename, 256, "%s", HOLIDAY_FILE );

   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {
      daycount = 0;
      for( ;; )
      {
         char letter;
         char *word;

         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }

         if( letter != '#' )
         {
            bug( "%s", "load_holidays: # not found." );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "HOLIDAY" ) )
         {
            if( daycount >= sysdata.maxholiday )
            {
               bug( _("load_holidays: more holidays than %d, increase Max Holiday in cset."), sysdata.maxholiday );
               fclose( fp );
               return;
            }
            CREATE( day, HOLIDAY_DATA, 1 );
            fread_day( day, fp );
            daycount++;
            LINK( day, first_holiday, last_holiday, next, prev );
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "load_holidays: bad section: %s.", word );
            continue;
         }
      }
      fclose( fp );
   }

   return;
}

/* Save the holidays to disk - Samson 5-6-99 */
void save_holidays( void )
{
   HOLIDAY_DATA *day;
   FILE *fp;
   char filename[256];

   snprintf( filename, 256, "%s", HOLIDAY_FILE );

   if( !( fp = fopen( filename, "w" ) ) )
   {
      bug( "%s", "save_holidays: fopen" );
      perror( filename );
   }
   else
   {
      for( day = first_holiday; day; day = day->next )
      {
         fprintf( fp, "%s", "#HOLIDAY\n" );
         fprintf( fp, "Name		%s~\n", day->name );
         fprintf( fp, "Announce	%s~\n", day->announce );
         fprintf( fp, "Month		%d\n", day->month );
         fprintf( fp, "Day		%d\n", day->day );
         fprintf( fp, "%s", "End\n\n" );
      }
      fprintf( fp, "%s", "#END\n" );
      fclose( fp );
   }
   return;
}

void
do_saveholiday( CHAR_DATA *ch, char *argument)
{
   save_holidays(  );
   send_to_char( _("Holiday chart saved.\n"), ch );
   return;
}

/* Holiday OLC command - (c)Andrew Wilkie May-20-2005*/
/* Calendar code (c)The Alsherok Team*/
void
do_setholiday( CHAR_DATA *ch, char *argument)
{
   HOLIDAY_DATA *day, *newday;
   int count = 0;
   int x = 0;
   char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH];

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );

   if( arg1 == '\0' || !str_cmp( arg1, " " ) )
   {
      send_to_char( _("Syntax : setholiday <name> <field> <argument>\n"), ch );
      send_to_char( _("Field can be : day name create announce save delete\n"), ch );
      return;
   }

/* Save em all.. saves the work of saving individual 
   holidays when mass-creating or editing*/
   if( !str_cmp( arg1, "save" ) )
   {
      save_holidays(  );
      send_to_char( _("Holiday chart saved.\n"), ch );
      return;
   }

/* Create a new holiday by name arg1 */
   if( !str_cmp( arg2, "create" ) )
   {
      for( day = first_holiday; day; day = day->next )
         count++;
      for( day = first_holiday; day; day = day->next )
      {
         if( !str_cmp( arg1, day->name ) )
         {
            send_to_char( _("A holiday with that name exists already!\n"), ch );
            return;
         }
      }

      if( count >= sysdata.maxholiday )
      {
         send_to_char( _("There are already too many holidays!\n"), ch );
         return;
      }

      CREATE( newday, HOLIDAY_DATA, 1 );
      newday->name = str_dup( arg1 );
      newday->day = time_info.day;
      newday->month = time_info.month;
      newday->announce = str_dup( _("Today is the holiday of when some moron forgot to set the announcement for this one!") );
      LINK( newday, first_holiday, last_holiday, next, prev );
      send_to_char( _("Holiday created.\n"), ch );
      return;
   }

/* Now... let's find that holiday */

   for( day = first_holiday; day; day = day->next )
   {
      if( !str_cmp( day->name, arg1 ) )
         break;
   }

/* Anything match? */
   if( !day )
   {
      send_to_char( _("Which holiday was that?\n"), ch );
      return;
   }

/* Set the day */

   if( !str_cmp( arg2, "day" ) )
   {
      if( arg3 == '\0' || !is_number( arg3 ) || atoi( arg3 ) > sysdata.dayspermonth || atoi( arg3 ) <= 1 )
      {
         ch_printf( ch, _("You must specify a numeric value : %d - %d"), 1, sysdata.dayspermonth );
         return;
      }

/* What day is it?... FRIDAY!.. oh... no... it's.. arg3? */
      day->day = atoi( arg3 );
      send_to_char( _("Day changed.\n"), ch );
      return;
   }

   if( !str_cmp( arg2, "month" ) )
   {
/* Go through the months and find arg3 */

      if( arg3 == '\0' || !is_number( arg3 ) || atoi( arg3 ) > sysdata.monthsperyear || atoi( arg3 ) <= 1 )
      {
         send_to_char( _("You must specify a valid month number:\n"), ch );

/* List all the months with a counter next to them*/
         count = 1;
         while( month_name[x] != '\0' && str_cmp(month_name[x], " ") && x < sysdata.monthsperyear)

         {
            ch_printf( ch, "&R(&W%d&R)&Y%s\r\n", count, month_name[x] );
            x++;
            count++;
         }
return;
      }


/* What's the month? */
      day->month = atoi( arg3 );
      send_to_char( _("Month changed.\n"), ch );
      return;
   }

   if( !str_cmp( arg2, "announce" ) )
   {
      if( arg3 == '\0' || !str_cmp( arg3, " " ) || is_number( arg3 ) )
      {
         send_to_char( _("Set the annoucement to what?\n"), ch );
         return;
      }

/* Set the announcement */
      DISPOSE( day->announce );
      day->announce = str_dup( arg3 );
      send_to_char( _("Announcement changed.\n"), ch );
      return;
   }

/* Change the name */
   if( !str_cmp( arg2, "name" ) )
   {
      if( arg3 == '\0' || !str_cmp( arg3, " " ) || is_number( arg3 ) )
      {
         send_to_char( _("Set the name to what?\n"), ch );
         return;
      }

/* Release the good...err... name */
      DISPOSE( day->name );
      day->name = str_dup( arg3 );
      send_to_char( _("Name changed.\n"), ch );
      return;
   }

   if( !str_cmp( arg2, "delete" ) )
   {
      if( str_cmp( arg3, "yes" ) )
      {
         send_to_char( _("If you are sure, use 'delete yes'.\n"), ch );
         return;
      }

      free_holiday( day );
      send_to_char( _("&RHoliday deleted.\n"), ch );
      return;
   }

   send_to_char( _("Syntax: setholiday <name> <field> <argument>\n"), ch );
   send_to_char( _("Field can be: day name create announce save delete\n"), ch );
   return;
}
