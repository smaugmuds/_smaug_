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

#define MAX_TZONE   25

extern time_t board_expire_time_t;
extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];

struct tzone_type
{
   char *name; /* Name of the time zone */
   char *zone; /* Cities or Zones in zone crossing */
   int gmt_offset;   /* Difference in hours from Greenwich Mean Time */
   int dst_offset;   /* Day Light Savings Time offset, Not used but left it in anyway */
};

struct tzone_type tzone_table[MAX_TZONE] = {
   {"GMT-12", "Eniwetok", -12, 0},
   {"GMT-11", "Samoa", -11, 0},
   {"GMT-10", "Hawaii", -10, 0},
   {"GMT-9", "Alaska", -9, 0},
   {"GMT-8", "Pacific US", -8, -7},
   {"GMT-7", "Mountain US", -7, -6},
   {"GMT-6", "Central US", -6, -5},
   {"GMT-5", "Eastern US", -5, -4},
   {"GMT-4", "Atlantic, Canada", -4, 0},
   {"GMT-3", "Brazilia, Buenos Aries", -3, 0},
   {"GMT-2", "Mid-Atlantic", -2, 0},
   {"GMT-1", "Cape Verdes", -1, 0},
   {"GMT", "Greenwich Mean Time, Greenwich", 0, 0},
   {"GMT+1", "Berlin, Rome, Madrid", 1, 0},
   {"GMT+2", "Israel, Cairo", 2, 0},
   {"GMT+3", "Moscow, Kuwait", 3, 0},
   {"GMT+4", "Abu Dhabi, Muscat", 4, 0},
   {"GMT+5", "Islamabad, Karachi", 5, 0},
   {"GMT+6", "Almaty, Dhaka", 6, 0},
   {"GMT+7", "Bangkok, Jakarta", 7, 0},
   {"GMT+8", "Hong Kong, Beijing", 8, 0},
   {"GMT+9", "Tokyo, Osaka", 9, 0},
   {"GMT+10", "Sydney, Melbourne, Guam", 10, 0},
   {"GMT+11", "Magadan, Soloman Is.", 11, 0},
   {"GMT+12", "Fiji, Wellington, Auckland", 12, 0},
};

/* Time values modified to Alsherok calendar - Samson 5-6-99 */
/* Time Values Modified to Smaug Calendar - Kayle 10-17-07 */
char *const day_name[] = {
   ___("the Moon"), ___("the Bull"), ___("Deception"), ___("Thunder"), ___("Freedom"),
   ___("the Great Gods"), ___("the Sun")
};

char *const month_name[] = {
   ___("Winter"), ___("the Winter Wolf"), ___("the Frost Giant"), ___("the Old Forces"),
   ___("the Grand Struggle"), ___("the Spring"), ___("Nature"), ___("Futility"), ___("the Dragon"),
   ___("the Sun"), ___("the Heat"), ___("the Battle"), ___("the Dark Shades"), ___("the Shadows"),
   ___("the Long Shadows"), ___("the Ancient Darkness"), ___("the Great Evil")
};

char *const season_name[] = {
   ___("&gspring"), ___("&Ysummer"), ___("&Oautumn"), ___("&Cwinter")
};

void update_calendar( void )
{
   sysdata.daysperyear = sysdata.dayspermonth * sysdata.monthsperyear;
   sysdata.hoursunrise = sysdata.hoursperday / 4;
   sysdata.hourdaybegin = sysdata.hoursunrise + 1;
   sysdata.hournoon = sysdata.hoursperday / 2;
   sysdata.hoursunset = ( ( sysdata.hoursperday / 4 ) * 3 );
   sysdata.hournightbegin = sysdata.hoursunset + 1;
   sysdata.hourmidnight = sysdata.hoursperday;
   calc_season(  );
}

void update_timers( void )
{
   sysdata.pulsetick = sysdata.secpertick * sysdata.pulsepersec;
   sysdata.pulseviolence = 3 * sysdata.pulsepersec;
   sysdata.pulsemobile = 4 * sysdata.pulsepersec;
   sysdata.pulsecalendar = 4 * sysdata.pulsetick;
}

int tzone_lookup( char *arg )
{
   int i;

   for( i = 0; i < MAX_TZONE; i++ )
   {
      if( !str_cmp( arg, tzone_table[i].name ) )
         return i;
   }

   for( i = 0; i < MAX_TZONE; i++ )
   {
      if( is_name( arg, tzone_table[i].zone ) )
         return i;
   }
   return -1;
}

void
do_timezone( CHAR_DATA *ch, char *argument)
{
   int i;

   if( IS_NPC( ch ) )
      return;

   if( !argument || argument[0] == '\0' )
   {
      ch_printf( ch, "%-6s %-30s (%s)\r\n", _("Name"), _("City/Zone Crosses"), _("Time") );
      send_to_char( "-------------------------------------------------------------------------\r\n", ch );
      for( i = 0; i < MAX_TZONE; i++ )
      {
         ch_printf( ch, "%-6s %-30s (%s)\r\n", tzone_table[i].name, tzone_table[i].zone, c_time( current_time, i ) );
      }
      send_to_char( "-------------------------------------------------------------------------\r\n", ch );
      return;
   }

   i = tzone_lookup( argument );

   if( i == -1 )
   {
      send_to_char( _("That time zone does not exists. Make sure to use the exact name.\n"), ch );
      return;
   }

   ch->pcdata->timezone = i;
   ch_printf( ch, _("Your time zone is now %s %s (%s)\n"), tzone_table[i].name,
              tzone_table[i].zone, c_time( current_time, i ) );
}

/* Ever so slightly modified version of "friendly_ctime" provided by Aurora.
 * Merged with the Timezone snippet by Ryan Jennings (Markanth) r-jenn@shaw.ca
 */
char *c_time( time_t curtime, int tz )
{
   static const char *day[] = { ___("Sun"), ___("Mon"), ___("Tue"), ___("Wed"), ___("Thu"), ___("Fri"), ___("Sat") };
   static const char *month[] = { ___("Jan"), ___("Feb"), ___("Mar"), ___("Apr"), ___("May"), ___("Jun"), ___("Jul"), ___("Aug"), ___("Sep"), ___("Oct"), ___("Nov"), ___("Dec") };
   static char strtime[128];
   struct tm *ptime;
   char tzonename[50];

   if( curtime <= 0 )
      curtime = current_time;

   if( tz > -1 && tz < MAX_TZONE )
   {
      mudstrlcpy( tzonename, tzone_table[tz].zone, 50 );
#if defined(__CYGWIN__)
      curtime += ( time_t ) timezone;
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
      /*
       * Hopefully this works 
       */
      ptime = localtime( &curtime );
      curtime += ptime->tm_gmtoff;
#else
      curtime += timezone; /* timezone external variable in time.h holds the 
                            * difference in seconds to GMT. */
#endif
      curtime += ( 60 * 60 * tzone_table[tz].gmt_offset );  /* Add the offset hours */
   }
   ptime = localtime( &curtime );
   if( tz < 0 || tz >= MAX_TZONE )
#if defined(__CYGWIN__)
      mudstrlcpy( tzonename, tzname[ptime->tm_isdst], 50 );
#else
      mudstrlcpy( tzonename, ptime->tm_zone, 50 );
#endif

   snprintf( strtime, 128, "%3s %3s %d, %d %d:%02d:%02d %cM %s",
             day[ptime->tm_wday], month[ptime->tm_mon], ptime->tm_mday, ptime->tm_year + 1900,
             ptime->tm_hour == 0 ? 12 : ptime->tm_hour > 12 ? ptime->tm_hour - 12 : ptime->tm_hour,
             ptime->tm_min, ptime->tm_sec, ptime->tm_hour < 12 ? 'A' : 'P', tzonename );
   return strtime;
}

/* timeZone is not shown as it's a bit .. long.. but it is respected -- Xorith */
char *mini_c_time( time_t curtime, int tz )
{
   static char strtime[128];
   struct tm *ptime;
   char tzonename[50];

   if( curtime <= 0 )
      curtime = current_time;

   if( tz > -1 && tz < MAX_TZONE )
   {
      mudstrlcpy( tzonename, tzone_table[tz].zone, 50 );
#if defined(__CYGWIN__)
      curtime += ( time_t ) timezone;
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
      /*
       * Hopefully this works 
       */
      ptime = localtime( &curtime );
      curtime += ptime->tm_gmtoff;
#else
      curtime += timezone; /* timezone external variable in time.h holds the
                            * difference in seconds to GMT. */
#endif
      curtime += ( 60 * 60 * tzone_table[tz].gmt_offset );  /* Add the offset hours */
   }
   ptime = localtime( &curtime );
   if( tz < 0 || tz >= MAX_TZONE )
#if defined(__CYGWIN__)
      mudstrlcpy( tzonename, tzname[ptime->tm_isdst], 50 );
#else
      mudstrlcpy( tzonename, ptime->tm_zone, 50 );
#endif

   snprintf( strtime, 128, "%02d/%02d/%02d %02d:%02d%c", ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_year - 100,
             ptime->tm_hour == 0 ? 12 : ptime->tm_hour > 12 ? ptime->tm_hour - 12 : ptime->tm_hour, ptime->tm_min,
             ptime->tm_hour < 12 ? 'A' : 'P' );
   return strtime;
}

/* Calling function must insure tstr buffer is large enough.
 * Returns the address of the buffer passed, allowing things like
 * this printf example: 123456secs = 1day 10hrs 17mins 36secs
 *   time_t tsecs = 123456;
 *   char   buff[MAX_STRING_LENGTH];
 *
 *   printf( "Duration is %s\n", duration( tsecs, buff ) );
 */

#define  DUR_SCMN       ( 60 )
#define  DUR_MNHR       ( 60 )
#define  DUR_HRDY       ( 24 )
#define  DUR_DYWK       (  7 )
#define  DUR_ADDS( t )  ( (t) == 1 ? '\0' : 's' )

char *sec_to_hms( time_t loctime, char *tstr )
{
   time_t t_rem;
   int sc, mn, hr, dy, wk;
   int sflg = 0;
   char buff[MAX_STRING_LENGTH];

   if( loctime < 1 )
   {
      mudstrlcat( tstr, _("no time at all"), MAX_STRING_LENGTH );
      return ( tstr );
   }

   sc = loctime % DUR_SCMN;
   t_rem = loctime - sc;

   if( t_rem > 0 )
   {
      t_rem /= DUR_SCMN;
      mn = t_rem % DUR_MNHR;
      t_rem -= mn;

      if( t_rem > 0 )
      {
         t_rem /= DUR_MNHR;
         hr = t_rem % DUR_HRDY;
         t_rem -= hr;

         if( t_rem > 0 )
         {
            t_rem /= DUR_HRDY;
            dy = t_rem % DUR_DYWK;
            t_rem -= dy;

            if( t_rem > 0 )
            {
               wk = t_rem / DUR_DYWK;

               if( wk )
               {
                  sflg = 1;
                  snprintf( buff, MAX_STRING_LENGTH, _("%d week%c"), wk, DUR_ADDS( wk ) );
                  mudstrlcat( tstr, buff, MAX_STRING_LENGTH );
               }
            }
            if( dy )
            {
               if( sflg == 1 )
                  mudstrlcat( tstr, " ", MAX_STRING_LENGTH );
               sflg = 1;
               snprintf( buff, MAX_STRING_LENGTH, _("%d day%c"), dy, DUR_ADDS( dy ) );
               mudstrlcat( tstr, buff, MAX_STRING_LENGTH );
            }
         }
         if( hr )
         {
            if( sflg == 1 )
               mudstrlcat( tstr, " ", MAX_STRING_LENGTH );
            sflg = 1;
            snprintf( buff, MAX_STRING_LENGTH, _("%d hour%c"), hr, DUR_ADDS( hr ) );
            mudstrlcat( tstr, buff, MAX_STRING_LENGTH );
         }
      }
      if( mn )
      {
         if( sflg == 1 )
            mudstrlcat( tstr, " ", MAX_STRING_LENGTH );
         sflg = 1;
         snprintf( buff, MAX_STRING_LENGTH, _("%d minute%c"), mn, DUR_ADDS( mn ) );
         mudstrlcat( tstr, buff, MAX_STRING_LENGTH );
      }
   }
   if( sc )
   {
      if( sflg == 1 )
         mudstrlcat( tstr, " ", MAX_STRING_LENGTH );
      snprintf( buff, MAX_STRING_LENGTH, _("%d second%c"), sc, DUR_ADDS( sc ) );
      mudstrlcat( tstr, buff, MAX_STRING_LENGTH );
   }
   return ( tstr );
}

/* Reads the actual time file from disk - Samson 1-21-99 */
void fread_timedata( FILE * fp )
{
   const char *word;
   bool fMatch = FALSE;

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

         case 'E':
            if( !str_cmp( word, "End" ) )
               return;
            break;

         case 'M':
            KEY( "Mhour", time_info.hour, fread_number( fp ) );
            KEY( "Mday", time_info.day, fread_number( fp ) );
            KEY( "Mmonth", time_info.month, fread_number( fp ) );
            KEY( "Myear", time_info.year, fread_number( fp ) );
            break;
         /* Uncomment if you have Samson's Pfile Cleanup Snippet installed.
		 case 'P':
            KEY( "Purgetime", new_pfile_time_t, fread_number( fp ) );
            break; */
      }

      if( !fMatch )
      {
         bug( "Fread_timedata: no match: %s", word );
         fread_to_eol( fp );
      }
   }
}

/* Load time information from saved file - Samson 1-21-99 */
bool load_timedata( void )
{
   char filename[256];
   FILE *fp;
   bool found;

   found = FALSE;
   snprintf( filename, 256, "%s", TIME_FILE );

   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {

      found = TRUE;
      for( ;; )
      {
         char letter = '\0';
         char *word = NULL;

         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }

         if( letter != '#' )
         {
            bug( "%s", "Load_timedata: # not found." );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "TIME" ) )
         {
            fread_timedata( fp );
            break;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "Load_timedata: bad section - %s.", word );
            break;
         }
      }
      fclose( fp );
   }

   return found;
}

/* Saves the current game world time to disk - Samson 1-21-99 */
void save_timedata( void )
{
   FILE *fp;
   char filename[256];

   snprintf( filename, 256, "%s", TIME_FILE );

   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "%s", "pcdata->save_timedata: fopen" );
      perror( filename );
   }
   else
   {
      fprintf( fp, "%s", "#TIME\n" );
      fprintf( fp, "Mhour	%d\n", time_info.hour );
      fprintf( fp, "Mday	%d\n", time_info.day );
      fprintf( fp, "Mmonth	%d\n", time_info.month );
      fprintf( fp, "Myear	%d\n", time_info.year );
      // Uncomment if you have Samson's Pfile Cleanup Snippet installed.
	  //fprintf( fp, "Purgetime %ld\n", ( long int )new_pfile_time_t );
      fprintf( fp, "%s", "End\n\n" );
      fprintf( fp, "%s", "#END\n" );
   }
   fclose( fp );
   return;
}

void
do_time( CHAR_DATA *ch, char *argument)
{
#ifdef ENABLE_HOLIDAYS
   HOLIDAY_DATA *holiday;
#endif
   extern char str_boot_time[];
   // Uncomment if you have Samson's Pfile Cleanup Snippet installed.
   //char buf[MAX_STRING_LENGTH];
   const char *suf;
   short day;

   day = time_info.day + 1;

   if( day > 4 && day < 20 )
      suf = _("th");
   else if( day % 10 == 1 )
      suf = _("st");
   else if( day % 10 == 2 )
      suf = _("nd");
   else if( day % 10 == 3 )
      suf = _("rd");
   else
      suf = _("th");

   ch_printf( ch, _("&wIt is &W%d&w o'clock &W%s&w, Day of &W%s&w,&W %d%s&w day in the Month of &W%s&w.\n"
              "&wIt is the season of %s&w, in the year &W%d&w.\n"
              "&wThe mud started up at :&W %s\n"
              "&wThe system time       :&W %s\n"),
              ( time_info.hour % sysdata.hournoon == 0 ) ? sysdata.hournoon : time_info.hour % sysdata.hournoon,
              time_info.hour >= sysdata.hournoon ? "pm" : "am", _(day_name[(time_info.day) % sysdata.daysperweek]), day, suf,
              _(month_name[time_info.month]), _(season_name[time_info.season]), time_info.year, str_boot_time,
              c_time( current_time, -1 ) );

   ch_printf( ch, _("&wYour local time       :&W %s&D\n"), c_time( current_time, ch->pcdata->timezone ) );

#ifdef ENABLE_HOLIDAYS
   holiday = get_holiday( time_info.month, day - 1 );
   if( holiday != NULL )
      ch_printf( ch, _("&wIt's a holiday today:&W %s\n"), holiday->name );
#endif

   if( !IS_NPC( ch ) )
   {
      if( day == ch->pcdata->day + 1 && time_info.month == ch->pcdata->month )
         send_to_char( _("&WToday is your &Pb&pi&Yr&Oth&Yd&pa&Py&R!&D\n"), ch );
   }

  /* Uncomment if you have Samson's Pfile Cleanup Snippet installed.
   if( IS_IMMORTAL( ch ) && sysdata.CLEANPFILES == TRUE )
   {
      long ptime, curtime;

      ptime = ( long int )( new_pfile_time_t );
      curtime = ( long int )( current_time );

      buf[0] = '\0';
      sec_to_hms( ptime - curtime, buf );
      ch_printf( ch, "&wThe next pfile cleanup is in&W %s&w.&D\r\n", buf );
   } */
   return;
}

void start_winter( void )
{
   ROOM_INDEX_DATA *room;
   int iHash;

   echo_to_all( AT_CYAN, _("The air takes on a chilling cold as winter sets in."), ECHOTAR_ALL );
   echo_to_all( AT_CYAN, _("Freshwater bodies everywhere have frozen over.\n"), ECHOTAR_ALL );

   winter_freeze = TRUE;

   for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
   {
      for( room = room_index_hash[iHash]; room; room = room->next )
      {
         switch ( room->sector_type )
         {
            case SECT_WATER_SWIM:
            case SECT_WATER_NOSWIM:
               room->winter_sector = room->sector_type;
               room->sector_type = SECT_ICE;
               break;
         }
      }
   }
   return;
}

void start_spring( void )
{
   ROOM_INDEX_DATA *room;
   int iHash;

   echo_to_all( AT_DGREEN, _("The chill recedes from the air as spring begins to take hold."), ECHOTAR_ALL );
   echo_to_all( AT_BLUE, _("Freshwater bodies everywhere have thawed out.\n"), ECHOTAR_ALL );

   winter_freeze = FALSE;

   for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
   {
      for( room = room_index_hash[iHash]; room; room = room->next )
      {
         if( room->sector_type == SECT_ICE && room->winter_sector != -1 )
         {
            room->sector_type = room->winter_sector;
            room->winter_sector = -1;
         }
      }
   }
   return;
}

void start_summer( void )
{
   echo_to_all( AT_YELLOW, _("The days grow longer and hotter as summer grips the world.\n"), ECHOTAR_ALL );
   return;
}

void start_fall( void )
{
   echo_to_all( AT_ORANGE, _("The leaves begin changing colors signaling the start of fall.\n"), ECHOTAR_ALL );
   return;
}

void season_update( void )
{
#ifdef ENABLE_HOLIDAYS
   HOLIDAY_DATA *day;

   day = get_holiday( time_info.month, time_info.day );

   if( day != NULL )
   {
      if( time_info.day + 1 == day->day && time_info.hour == 0 )
      {
         echo_to_all( AT_IMMORT, day->announce, ECHOTAR_ALL );
      }
   }
#endif

   if( time_info.season == SEASON_WINTER && winter_freeze == FALSE )
   {
      ROOM_INDEX_DATA *room;
      int iHash;

      winter_freeze = TRUE;

      for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
      {
         for( room = room_index_hash[iHash]; room; room = room->next )
         {
            switch ( room->sector_type )
            {
               case SECT_WATER_SWIM:
               case SECT_WATER_NOSWIM:
                  room->winter_sector = room->sector_type;
                  room->sector_type = SECT_ICE;
                  break;
            }
         }
      }
   }
   return;
}

/* PaB: Which season are we in? 
 * Notes: Simply figures out which season the current month falls into
 * and returns a proper value.
 */
void calc_season( void )
{
   /*
    * How far along in the year are we, measured in days? 
    */
   /*
    * PaB: Am doing this in days to minimize roundoff impact 
    */
   int day = time_info.month * sysdata.dayspermonth + time_info.day;

   if( day < ( sysdata.daysperyear / 4 ) )
   {
      time_info.season = SEASON_SPRING;
      if( time_info.hour == 0 && day == 0 )
         start_spring(  );
   }
   else if( day < ( sysdata.daysperyear / 4 ) * 2 )
   {
      time_info.season = SEASON_SUMMER;
      if( time_info.hour == 0 && day == ( sysdata.daysperyear / 4 ) )
         start_summer(  );
   }
   else if( day < ( sysdata.daysperyear / 4 ) * 3 )
   {
      time_info.season = SEASON_FALL;
      if( time_info.hour == 0 && day == ( ( sysdata.daysperyear / 4 ) * 2 ) )
         start_fall(  );
   }
   else if( day < sysdata.daysperyear )
   {
      time_info.season = SEASON_WINTER;
      if( time_info.hour == 0 && day == ( ( sysdata.daysperyear / 4 ) * 3 ) )
         start_winter(  );
   }
   else
   {
      time_info.season = SEASON_SPRING;
   }

   season_update(  );   /* Maintain the season in case of reboot, check for holidays */

   return;
}
