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
 *                          MSSP Plaintext Module                           *
 ****************************************************************************/

/******************************************************************
* Program writen by:                                              *
*  Greg (Keberus Maou'San) Mosley                                 *
*  Co-Owner/Coder SW: TGA                                         *
*  www.t-n-k-games.com                                            *
*                                                                 *
* Description:                                                    *
*  This program will allow admin to view and set thier MSSP       *
*  variables in game, and allows thier game to respond to a MSSP  *
*  Server with the MSSP-Plaintext protocol                        *
*******************************************************************
* What it does:                                                   *
*  Allows admin to set/view MSSP variables and transmits the MSSP *
*  information to anyone who does an MSSP-REQUEST at the login    *
*  screen                                                         *
*******************************************************************
* Special Thanks:                                                 *
*  A special thanks to Scandum for coming up with the MSSP        *
*  protocol, Cratylus for the MSSP-Plaintext idea, and Elanthis   *
*  for the GNUC_FORMAT idea ( which I like to use now ).          *
******************************************************************/

/*TERMS OF USE
         I only really have 2 terms...
 1. Give credit where it is due, keep the above header in your code 
    (you don't have to give me credit in mud) and if someone asks 
	don't lie and say you did it.
 2. If you have any comments or questions feel free to email me
    at keberus@gmail.com

  Thats All....
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "mud.h"
#include "mssp.h"

struct mssp_info *mssp_info;
void fread_mssp_info( FILE * fp );
bool write_to_descriptor( DESCRIPTOR_DATA * d, const char *txt, int length );
void write_to_descriptor_printf( DESCRIPTOR_DATA * desc, const char *fmt, ... ) GNUC_FORMAT( 2, 3 ); 

void free_mssp_info( void )
{
   DISPOSE( mssp_info->hostname );
   DISPOSE( mssp_info->contact );
   DISPOSE( mssp_info->icon );
   DISPOSE( mssp_info->language );
   DISPOSE( mssp_info->location );
   DISPOSE( mssp_info->website );
   DISPOSE( mssp_info->family );
   DISPOSE( mssp_info->genre );
   DISPOSE( mssp_info->gamePlay );
   DISPOSE( mssp_info->gameSystem );
   DISPOSE( mssp_info->intermud );
   DISPOSE( mssp_info->status );
   DISPOSE( mssp_info->subgenre );
   DISPOSE( mssp_info->equipmentSystem );
   DISPOSE( mssp_info->multiplaying );
   DISPOSE( mssp_info->playerKilling );
   DISPOSE( mssp_info->questSystem );
   DISPOSE( mssp_info->roleplaying );
   DISPOSE( mssp_info->trainingSystem );
   DISPOSE( mssp_info->worldOriginality );
   DISPOSE( mssp_info );
}

void save_mssp_info( void )
{
   FILE *fp;
   char filename[256];

   snprintf( filename, 256, "%s", MSSP_FILE );

   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "%s: can't open file", __func__ );
      perror( filename );
   }
   else
   {
      fprintf( fp, "%s", "#MSSP_INFO\n" );
      fprintf( fp, "Hostname          %s~\n", mssp_info->hostname );
      fprintf( fp, "Contact           %s~\n", mssp_info->contact );
      fprintf( fp, "Icon              %s~\n", mssp_info->icon );
      fprintf( fp, "Language          %s~\n", mssp_info->language );
      fprintf( fp, "Location          %s~\n", mssp_info->location );
      fprintf( fp, "Website           %s~\n", mssp_info->website );
      fprintf( fp, "Family            %s~\n", mssp_info->family );
      fprintf( fp, "Genre             %s~\n", mssp_info->genre );
      fprintf( fp, "GamePlay          %s~\n", mssp_info->gamePlay );
      fprintf( fp, "GameSystem        %s~\n", mssp_info->gameSystem );
      fprintf( fp, "Intermud          %s~\n", mssp_info->intermud );
      fprintf( fp, "Status            %s~\n", mssp_info->status );
      fprintf( fp, "SubGenre          %s~\n", mssp_info->subgenre );
      fprintf( fp, "Created           %d\n", mssp_info->created );
      fprintf( fp, "MinAge            %d\n", mssp_info->minAge );
      fprintf( fp, "Worlds            %d\n", mssp_info->worlds );
      fprintf( fp, "Ansi              %d\n", mssp_info->ansi );
      fprintf( fp, "MCCP              %d\n", mssp_info->mccp );
      fprintf( fp, "MCP               %d\n", mssp_info->mcp );
      fprintf( fp, "MSP               %d\n", mssp_info->msp );
      fprintf( fp, "SSL               %d\n", mssp_info->ssl );
      fprintf( fp, "MXP               %d\n", mssp_info->mxp );
      fprintf( fp, "Pueblo            %d\n", mssp_info->pueblo );
      fprintf( fp, "Vt100             %d\n", mssp_info->vt100 );
      fprintf( fp, "Xterm256          %d\n", mssp_info->xterm256 );
      fprintf( fp, "Pay2Play          %d\n", mssp_info->pay2play );
      fprintf( fp, "Pay4Perks         %d\n", mssp_info->pay4perks );
      fprintf( fp, "HiringBuilders    %d\n", mssp_info->hiringBuilders );
      fprintf( fp, "HiringCoders      %d\n", mssp_info->hiringCoders );
      fprintf( fp, "AdultMaterial     %d\n", mssp_info->adultMaterial );
      fprintf( fp, "Multiclassing     %d\n", mssp_info->multiclassing );
      fprintf( fp, "NewbieFriendly    %d\n", mssp_info->newbieFriendly );
      fprintf( fp, "PlayerCities      %d\n", mssp_info->playerCities );
      fprintf( fp, "PlayerClans       %d\n", mssp_info->playerClans );
      fprintf( fp, "PlayerCrafting    %d\n", mssp_info->playerCrafting );
      fprintf( fp, "PlayerGuilds      %d\n", mssp_info->playerGuilds );
      fprintf( fp, "EquipmentSystem   %s~\n", mssp_info->equipmentSystem );
      fprintf( fp, "Multiplaying      %s~\n", mssp_info->multiplaying );
      fprintf( fp, "PlayerKilling     %s~\n", mssp_info->playerKilling );
      fprintf( fp, "QuestSystem       %s~\n", mssp_info->questSystem );
      fprintf( fp, "RolePlaying       %s~\n", mssp_info->roleplaying );
      fprintf( fp, "TrainingSystem    %s~\n", mssp_info->trainingSystem );
      fprintf( fp, "WorldOriginality  %s~\n", mssp_info->worldOriginality );
      fprintf( fp, "%s", "End\n\n" );
      fprintf( fp, "%s", "#END\n" );

      fclose( fp );
      fp = NULL;
   }
}

/*
 * Load the MSSP data file
 */
bool load_mssp_data( void )
{
   char filename[MAX_INPUT_LENGTH];
   FILE *fp;
   bool found;

   CREATE( mssp_info, struct mssp_info, 1 );

   found = FALSE;
   sprintf( filename, "%s", MSSP_FILE );

   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {
      found = TRUE;

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
            bug( "%s: # not found.", __func__ );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "MSSP_INFO" ) )
         {
            fread_mssp_info( fp );
            break;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "%s: bad section.", __func__ );
            break;
         }
      }
      fclose( fp );
      fp = NULL;
   }
   return found;
}

void fread_mssp_info( FILE * fp )
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
            KEY( "AdultMaterial", mssp_info->adultMaterial, fread_number( fp ) );
            KEY( "Ansi", mssp_info->ansi, fread_number( fp ) );
            break;

         case 'C':
            KEY( "Contact", mssp_info->contact, fread_string_nohash( fp ) );
            KEY( "Created", mssp_info->created, fread_number( fp ) );
            break;

         case 'E':
            KEY( "EquipmentSystem", mssp_info->equipmentSystem, fread_string_nohash( fp ) );
            if( !str_cmp( word, "End" ) )
               return;
            break;

         case 'F':
            KEY( "Family", mssp_info->family, fread_string_nohash( fp ) );
            break;

         case 'G':
            KEY( "Genre", mssp_info->genre, fread_string_nohash( fp ) );
            KEY( "GamePlay", mssp_info->gamePlay, fread_string_nohash( fp ) );
            KEY( "GameSystem", mssp_info->gameSystem, fread_string_nohash( fp ) );
            break;

         case 'H':
            KEY( "Hostname", mssp_info->hostname, fread_string_nohash( fp ) );
            KEY( "HiringBuilders", mssp_info->hiringBuilders, fread_number( fp ) );
            KEY( "HiringCoders", mssp_info->hiringCoders, fread_number( fp ) );
            break;

         case 'I':
            KEY( "Icon", mssp_info->icon, fread_string_nohash( fp ) );
            KEY( "Intermud", mssp_info->intermud, fread_string_nohash( fp ) );
            break;

         case 'L':
            KEY( "Language", mssp_info->language, fread_string_nohash( fp ) );
            KEY( "Location", mssp_info->location, fread_string_nohash( fp ) );
            break;

         case 'M':
            KEY( "MCCP", mssp_info->mccp, fread_number( fp ) );
            KEY( "MCP", mssp_info->mcp, fread_number( fp ) );
            KEY( "MinAge", mssp_info->minAge, fread_number( fp ) );
            KEY( "MSP", mssp_info->msp, fread_number( fp ) );
            KEY( "Multiclassing", mssp_info->multiclassing, fread_number( fp ) );
            KEY( "Multiplaying", mssp_info->multiplaying, fread_string_nohash( fp ) );
            KEY( "MXP", mssp_info->mxp, fread_number( fp ) );
            break;

         case 'N':
            KEY( "NewbieFriendly", mssp_info->newbieFriendly, fread_number( fp ) );
            break;

         case 'P':
            KEY( "Pay2Play", mssp_info->pay2play, fread_number( fp ) );
            KEY( "Pay4Perks", mssp_info->pay4perks, fread_number( fp ) );
            KEY( "PlayerCities", mssp_info->playerCities, fread_number( fp ) );
            KEY( "PlayerClans", mssp_info->playerClans, fread_number( fp ) );
            KEY( "PlayerCrafting", mssp_info->playerCrafting, fread_number( fp ) );
            KEY( "PlayerGuilds", mssp_info->playerGuilds, fread_number( fp ) );
            KEY( "PlayerKilling", mssp_info->playerKilling, fread_string_nohash( fp ) );
            KEY( "Pueblo", mssp_info->pueblo, fread_number( fp ) );
            break;

         case 'Q':
            KEY( "QuestSystem", mssp_info->questSystem, fread_string_nohash( fp ) );
            break;

         case 'R':
            KEY( "RolePlaying", mssp_info->roleplaying, fread_string_nohash( fp ) );
            break;

         case 'S':
            KEY( "SSL", mssp_info->ssl, fread_number( fp ) );
            KEY( "Status", mssp_info->status, fread_string_nohash( fp ) );
            KEY( "SubGenre", mssp_info->subgenre, fread_string_nohash( fp ) );
            break;

         case 'T':
            KEY( "TrainingSystem", mssp_info->trainingSystem, fread_string_nohash( fp ) );
            break;

         case 'V':
            KEY( "Vt100", mssp_info->vt100, fread_number( fp ) );
            break;

         case 'W':
            KEY( "Website", mssp_info->website, fread_string_nohash( fp ) );
            KEY( "WorldOriginality", mssp_info->worldOriginality, fread_string_nohash( fp ) );
            KEY( "Worlds", mssp_info->worlds, fread_number( fp ) );
            break;

         case 'X':
            KEY( "Xterm256", mssp_info->xterm256, fread_number( fp ) );
            break;
      }
      if( !fMatch )
         bug( "%s: no match: %s", __func__, word );
   }
}

#define MSSP_YN( value )  ( (value) == 0 ? "No" : "Yes" )

void show_mssp( CHAR_DATA * ch )
{
   if( !ch )
   {
      bug( "%s: NULL ch", __func__ );
      return;
   }

   ch_printf( ch, "&zHostname          &W%s\r\n", mssp_info->hostname );
   ch_printf( ch, "&zContact           &W%s\r\n", mssp_info->contact );
   ch_printf( ch, "&zIcon              &W%s\r\n", mssp_info->icon );
   ch_printf( ch, "&zLanguage          &W%s\r\n", mssp_info->language );
   ch_printf( ch, "&zLocation          &W%s\r\n", mssp_info->location );
   ch_printf( ch, "&zWebsite           &W%s\r\n", mssp_info->website );
   ch_printf( ch, "&zFamily            &W%s\r\n", mssp_info->family );
   ch_printf( ch, "&zGenre             &W%s\r\n", mssp_info->genre );
   ch_printf( ch, "&zGamePlay          &W%s\r\n", mssp_info->gamePlay );
   ch_printf( ch, "&zGameSystem        &W%s\r\n", mssp_info->gameSystem );
   ch_printf( ch, "&zIntermud          &W%s\r\n", mssp_info->intermud );
   ch_printf( ch, "&zStatus            &W%s\r\n", mssp_info->status );
   ch_printf( ch, "&zSubGenre          &W%s\r\n", mssp_info->subgenre );
   ch_printf( ch, "&zCreated           &W%d\r\n", mssp_info->created );
   ch_printf( ch, "&zMinAge            &W%d\r\n", mssp_info->minAge );
   ch_printf( ch, "&zWorlds            &W%d\r\n", mssp_info->worlds );
   ch_printf( ch, "&zAnsi              &W%s\r\n", MSSP_YN( mssp_info->ansi ) );
   ch_printf( ch, "&zMCCP              &W%s\r\n", MSSP_YN( mssp_info->mccp ) );
   ch_printf( ch, "&zMCP               &W%s\r\n", MSSP_YN( mssp_info->mcp ) );
   ch_printf( ch, "&zMSP               &W%s\r\n", MSSP_YN( mssp_info->msp ) );
   ch_printf( ch, "&zSSL               &W%s\r\n", MSSP_YN( mssp_info->ssl ) );
   ch_printf( ch, "&zMXP               &W%s\r\n", MSSP_YN( mssp_info->mxp ) );
   ch_printf( ch, "&zPueblo            &W%s\r\n", MSSP_YN( mssp_info->pueblo ) );
   ch_printf( ch, "&zVt100             &W%s\r\n", MSSP_YN( mssp_info->vt100 ) );
   ch_printf( ch, "&zXterm256          &W%s\r\n", MSSP_YN( mssp_info->xterm256 ) );
   ch_printf( ch, "&zPay2Play          &W%s\r\n", MSSP_YN( mssp_info->pay2play ) );
   ch_printf( ch, "&zPay4Perks         &W%s\r\n", MSSP_YN( mssp_info->pay4perks ) );
   ch_printf( ch, "&zHiringBuilders    &W%s\r\n", MSSP_YN( mssp_info->hiringBuilders ) );
   ch_printf( ch, "&zHiringCoders      &W%s\r\n", MSSP_YN( mssp_info->hiringCoders ) );
   ch_printf( ch, "&zAdultMaterial     &W%s\r\n", MSSP_YN( mssp_info->adultMaterial ));
   ch_printf( ch, "&zMulticlassing     &W%s\r\n", MSSP_YN( mssp_info->multiclassing ));
   ch_printf( ch, "&zNewbieFriendly    &W%s\r\n", MSSP_YN( mssp_info->newbieFriendly ));
   ch_printf( ch, "&zPlayerCities      &W%s\r\n", MSSP_YN( mssp_info->playerCities ));
   ch_printf( ch, "&zPlayerClans       &W%s\r\n", MSSP_YN( mssp_info->playerClans ));
   ch_printf( ch, "&zPlayerCrafting    &W%s\r\n", MSSP_YN( mssp_info->playerCrafting ));
   ch_printf( ch, "&zPlayerGuilds      &W%s\r\n", MSSP_YN( mssp_info->playerGuilds ));
   ch_printf( ch, "&zEquipmentSystem   &W%s\r\n", mssp_info->equipmentSystem );
   ch_printf( ch, "&zMultiplaying      &W%s\r\n", mssp_info->multiplaying );
   ch_printf( ch, "&zPlayerKilling     &W%s\r\n", mssp_info->playerKilling );
   ch_printf( ch, "&zQuestSystem       &W%s\r\n", mssp_info->questSystem );
   ch_printf( ch, "&zRolePlaying       &W%s\r\n", mssp_info->roleplaying );
   ch_printf( ch, "&zTrainingSystem    &W%s\r\n", mssp_info->trainingSystem );
   ch_printf( ch, "&zWorldOriginality  &W%s\r\n", mssp_info->worldOriginality );
}

void do_setmssp( CHAR_DATA *ch, const char* argument )
{
   char arg1[MIL];
   char **strptr = NULL;
   bool *ynptr = NULL;

   argument = one_argument( argument, arg1 );

   if( ( arg1[0] == '\0' ) || !str_cmp( arg1, "show" ) ) //Here you go Conner :)
   {
      show_mssp( ch );
      return;
   }
   if( !argument || ( argument[0] == '\0' ) )
   {
      send_to_char( "Syntax: setmssp <field> [value]\r\n", ch );
      send_to_char( "Field being one of:\r\n", ch );
      send_to_char( "hostname       contact           icon               lanuage          location\r\n", ch );
      send_to_char( "website        family            genre              gameplay         game_system\r\n", ch );
      send_to_char( "intermud       status            subgenre           created          min_age\r\n", ch );
      send_to_char( "worlds         ansi              mccp               mcp              msp\r\n", ch );
      send_to_char( "ssl            mxp               pueblo             vt100            xterm256\r\n", ch );
      send_to_char( "pay2play       pay4perks         hiring_builders    hiring_coders    adult_material\r\n", ch );
      send_to_char( "multiclassing  newbie_friendly   player_cities      player_clans     player_crafting\r\n", ch );
      send_to_char( "player_guilds  equipment_system  multiplaying       player_killing   quest_system\r\n", ch );
      send_to_char( "roleplaying    training_system   world_originality\r\n", ch );    

      return;
   }
   if( !str_cmp( arg1, "hostname" ) )
      strptr = &mssp_info->hostname;
   else if( !str_cmp( arg1, "contact" ) )
      strptr = &mssp_info->contact;
   else if( !str_cmp( arg1, "icon" ) )
      strptr = &mssp_info->icon;
   else if( !str_cmp( arg1, "language" ) )
      strptr = &mssp_info->language;
   else if( !str_cmp( arg1, "location" ) )
      strptr = &mssp_info->location;
   else if( !str_cmp( arg1, "website" ) )
      strptr = &mssp_info->website;
   else if( !str_cmp( arg1, "family" ) )
      strptr = &mssp_info->family;
   else if( !str_cmp( arg1, "genre" ) )
      strptr = &mssp_info->genre;
   else if( !str_cmp( arg1, "gameplay" ) )
      strptr = &mssp_info->gamePlay;
   else if( !str_cmp( arg1, "game_system" ) )
      strptr = &mssp_info->gameSystem;
   else if( !str_cmp( arg1, "intermud" ) )
      strptr = &mssp_info->intermud;
   else if( !str_cmp( arg1, "status" ) )
      strptr = &mssp_info->status;
   else if( !str_cmp( arg1, "subgenre" ) )
      strptr = &mssp_info->subgenre;

   if( strptr != NULL )
   {
      DISPOSE( *strptr );
      *strptr = str_dup( argument );
      ch_printf( ch, "MSSP value, %s has been changed to: %s\r\n", arg1, argument );
      save_mssp_info(  );
      return;
   }
   if( !str_cmp( arg1, "ansi" ) )
      ynptr = &mssp_info->ansi;
   else if( !str_cmp( arg1, "mccp" ) )
      ynptr = &mssp_info->mccp;
   else if( !str_cmp( arg1, "mcp" ) )
      ynptr = &mssp_info->mcp;
   else if( !str_cmp( arg1, "msp" ) )
      ynptr = &mssp_info->msp;
   else if( !str_cmp( arg1, "ssl" ) )
      ynptr = &mssp_info->ssl;
   else if( !str_cmp( arg1, "mxp" ) )
      ynptr = &mssp_info->mxp;
   else if( !str_cmp( arg1, "pueblo" ) )
      ynptr = &mssp_info->pueblo;
   else if( !str_cmp( arg1, "vt100" ) )
      ynptr = &mssp_info->vt100;
   else if( !str_cmp( arg1, "xterm256" ) )
      ynptr = &mssp_info->xterm256;
   else if( !str_cmp( arg1, "pay2play" ) )
      ynptr = &mssp_info->pay2play;
   else if( !str_cmp( arg1, "pay4perks" ) )
      ynptr = &mssp_info->pay4perks;
   else if( !str_cmp( arg1, "hiring_builders" ) )
      ynptr = &mssp_info->hiringBuilders;
   else if( !str_cmp( arg1, "hiring_coders" ) )
      ynptr = &mssp_info->hiringCoders;
   else if( !str_cmp( arg1, "adult_material" ) )
      ynptr = &mssp_info->adultMaterial;
   else if( !str_cmp( arg1, "multiclassing" ) )
      ynptr = &mssp_info->multiclassing;
   else if( !str_cmp( arg1, "newbie_friendly" ) )
      ynptr = &mssp_info->newbieFriendly;
   else if( !str_cmp( arg1, "player_cities" ) )
      ynptr = &mssp_info->playerCities;
   else if( !str_cmp( arg1, "player_clans" ) )
      ynptr = &mssp_info->playerClans;
   else if( !str_cmp( arg1, "player_crafting" ) )
      ynptr = &mssp_info->playerCrafting;
   else if( !str_cmp( arg1, "player_guilds" ) )
      ynptr = &mssp_info->playerGuilds;

   if( ynptr != NULL )
   {
      bool newvalue = FALSE;

      if( str_cmp( argument, "yes" ) && str_cmp( argument, "no" ) )
      {
         ch_printf( ch, "You must specify 'yes' or 'no' for the %s value!\r\n", arg1 );
         return;
      }
      newvalue = !str_cmp( argument, "yes" ) ? TRUE : FALSE;
      *ynptr = newvalue;
      ch_printf( ch, "MSSP value, %s has been changed to: %s\r\n", arg1, argument );
      save_mssp_info(  );
      return;
   }

   if( !str_cmp( arg1, "worlds" ) )
   {
      int value;

      value = atoi( argument );

      if( !is_number( argument ) || ( value < 0 ) || ( value > MSSP_MAXVAL ) )
      {
         ch_printf( ch, "The value for %s must be between 0 and %d\r\n", arg1, MSSP_MAXVAL );
         return;
      }
      mssp_info->worlds = value;

      ch_printf( ch, "MSSP value, %s has been changed to: %s\r\n", arg1, argument );
      save_mssp_info(  );
      return;
   }
   else if( !str_cmp( arg1, "created" ) )
   {
      int value;

      value = atoi( argument );

      if( !is_number( argument ) || ( value < MSSP_MINCREATED ) || ( value > MSSP_MAXCREATED ) )
      {
         ch_printf( ch, "The value for created must be between %d and %d\r\n", MSSP_MINCREATED, MSSP_MAXCREATED );
         return;
      }
      mssp_info->created = value;
      ch_printf( ch, "MSSP value, %s has been changed to: %s\r\n", arg1, argument );
      save_mssp_info(  );
      return;
   }
   else if( !str_cmp( arg1, "multiplaying" ) || !str_cmp( arg1, "player_killing" ) ) 
   {
      if( strcmp( argument, "None" ) && strcmp( argument, "Restricted" ) && str_cmp( argument, "Full" ) ) 
      {
         ch_printf( ch, "Valid choices for %s are: None, Restricted or Full\r\n", arg1 );
         return; 
      }
      if( !str_cmp( arg1, "multiplaying" ) )
      {
         DISPOSE( mssp_info->multiplaying );
         mssp_info->multiplaying = str_dup( argument );
      }
      else
      {
         DISPOSE( mssp_info->playerKilling );
         mssp_info->playerKilling = str_dup( argument );
      }
      ch_printf( ch, "MSSP value, %s has been changed to: %s\r\n", arg1, argument );
      save_mssp_info(  );
      return;
   }
   else if( !str_cmp( arg1, "training_system" ) || !str_cmp( arg1, "equipment_system" ) )
   {
      if( strcmp( argument, "None" ) && strcmp( argument, "Level" ) && str_cmp( argument, "Skill" ) && str_cmp( argument, "Both" ))
      {
         ch_printf( ch, "Valid choices for %s are: None, Level, Skill or Both\r\n", arg1 );
         return;
      }
      if( !str_cmp( arg1, "training_system" ) )
      {
         DISPOSE( mssp_info->trainingSystem );
         mssp_info->trainingSystem = str_dup( argument );
      }
      else
      {
         DISPOSE( mssp_info->equipmentSystem );
         mssp_info->equipmentSystem = str_dup( argument );
      }
      ch_printf( ch, "MSSP value, %s has been changed to: %s\r\n", arg1, argument );
      save_mssp_info(  );
      return;
   }
   else if( !str_cmp( arg1, "quest_system" ) )
   {
      if( strcmp( argument, "None" ) && strcmp( argument, "Immortal Run" ) && str_cmp( argument, "Automated" ) && str_cmp( argument, "Integrated" ))
      {
         ch_printf( ch, "Valid choices for %s are: None, Immortal Run, Automated or Integrated\r\n", arg1 );
         return;
      }
      DISPOSE( mssp_info->questSystem );
      mssp_info->questSystem = str_dup( argument );
      ch_printf( ch, "MSSP value, %s has been changed to: %s\r\n", arg1, argument );
      save_mssp_info(  );
      return;
   }
   else if( !str_cmp( arg1, "roleplaying" ) )
   {
      if( strcmp( argument, "None" ) && strcmp( argument, "Accepted" ) && str_cmp( argument, "Encouraged" ) && str_cmp( argument, "Enforced" ))
      {
         ch_printf( ch, "Valid choices for %s are: None, Accepted, Encouraged or Enforced\r\n", arg1 );
         return;
      }
      DISPOSE( mssp_info->roleplaying );
      mssp_info->roleplaying = str_dup( argument );
      ch_printf( ch, "MSSP value, %s has been changed to: %s\r\n", arg1, argument );
      save_mssp_info(  );
      return;
   }
   else if( !str_cmp( arg1, "world_originality" ) )
   {
      if( strcmp( argument, "All Stock" ) && strcmp( argument, "Mostly Stock" ) && str_cmp( argument, "Mostly Original" ) && str_cmp( argument, "All Original" ))
      {
         ch_printf( ch, "Valid choices for %s are: All Stock, Mostly Stock, Mostly Original or All Original\r\n", arg1 );
         return;
      }
      DISPOSE( mssp_info->worldOriginality );
      mssp_info->worldOriginality = str_dup( argument );
      ch_printf( ch, "MSSP value, %s has been changed to: %s\r\n", arg1, argument );
      save_mssp_info(  );
      return;
   }

   else if( !str_cmp( arg1, "min_age" ) )
   {
      int value;

      value = atoi( argument );

      if( !is_number( argument ) || ( value < MSSP_MINAGE ) || ( value > MSSP_MAXAGE ) )
      {
         ch_printf( ch, "The value for min_age must be between %d and %d\r\n", MSSP_MINAGE, MSSP_MAXAGE );
         return;
      }
      mssp_info->minAge = value;
      ch_printf( ch, "MSSP value, %s has been changed to: %s\r\n", arg1, argument );
      save_mssp_info(  );
      return;
   }
   else
      do_setmssp( ch, "" );
}

void mssp_reply( DESCRIPTOR_DATA * d, const char *var, const char *fmt, ... )
{
   char buf[MAX_STRING_LENGTH];
   va_list args;

   if( !d )
   {
      bug( "%s: NULL d", __func__ );
      return;
   }
   if( !var || var[0] == '\0' )
   {
      bug( "%s: NULL var", __func__ );
      return;
   }

   va_start( args, fmt );
   vsprintf( buf, fmt, args );
   va_end( args );

   write_to_descriptor_printf( d, "%s\t%s\r\n", var, buf );
}

extern time_t mud_start_time;
extern int top_area;
extern int top_help;
extern int top_room;
extern int top_reset;
extern int top_prog;
extern int top_mob_index;
extern int top_obj_index;
extern short num_skills;
extern int top_prog;
#define codebase "SmaugFUSS 1.9"

short player_count( void )
{
   DESCRIPTOR_DATA *d;
   short count = 0;

   for( d = first_descriptor; d; d = d->next )
   {
      if( d->connected >= CON_PLAYING )
         ++count;
   }
   return count;
}

void send_mssp_data( DESCRIPTOR_DATA * d )
{
   if( !d )
   {
      bug( "%s: NULL d", __func__ );
      return;
   }

   write_to_descriptor( d, "\r\nMSSP-REPLY-START\r\n", 0 );

   mssp_reply( d, "HOSTNAME", "%s", mssp_info->hostname );
   mssp_reply( d, "PORT", "%d", port );
   mssp_reply( d, "UPTIME", "%d", (int)mud_start_time );
   mssp_reply( d, "PLAYERS", "%d", player_count( ) );
   mssp_reply( d, "CODEBASE", "%s", codebase );
   mssp_reply( d, "CONTACT", "%s", mssp_info->contact );
   mssp_reply( d, "CREATED", "%d", mssp_info->created );
   mssp_reply( d, "ICON", "%s", mssp_info->icon );
   mssp_reply( d, "LANGUAGE", "%s", mssp_info->language );
   mssp_reply( d, "LOCATION", "%s", mssp_info->location );
   mssp_reply( d, "MINIMUM AGE", "%d", mssp_info->minAge );
   mssp_reply( d, "NAME", "%s", sysdata.mud_name );
   mssp_reply( d, "WEBSITE", "%s", mssp_info->website );
   mssp_reply( d, "FAMILY", "%s", mssp_info->family );
   mssp_reply( d, "GENRE", "%s", mssp_info->genre );
   mssp_reply( d, "GAMEPLAY", "%s", mssp_info->gamePlay );
   mssp_reply( d, "GAMESYSTEM", "%s", mssp_info->gameSystem );
   mssp_reply( d, "INTERMUD", "%s", mssp_info->intermud );
   mssp_reply( d, "STATUS", "%s", mssp_info->status );
   mssp_reply( d, "SUBGENRE", "%s", mssp_info->subgenre );
   mssp_reply( d, "AREAS", "%d", top_area );
   mssp_reply( d, "HELPFILES", "%d", top_help );
   mssp_reply( d, "MOBILES", "%d", top_mob_index );
   mssp_reply( d, "OBJECTS", "%d", top_obj_index );
   mssp_reply( d, "ROOMS", "%d", top_room );
   mssp_reply( d, "RESETS", "%d", top_reset );
//   mssp_reply( d, "MUDPROGS", "%d", top_prog );
   mssp_reply( d, "CLASSES", "%d", MAX_CLASS );
   mssp_reply( d, "LEVELS", "%d", MAX_LEVEL );
   mssp_reply( d, "RACES", "%d", MAX_RACE );
   mssp_reply( d, "SKILLS", "%d", num_skills );
   mssp_reply( d, "WORLDS", "%d", mssp_info->worlds );
   mssp_reply( d, "ANSI", "%d", mssp_info->ansi );
   mssp_reply( d, "MCCP", "%d", mssp_info->mccp );
   mssp_reply( d, "MCP", "%d", mssp_info->mcp );
   mssp_reply( d, "MSP", "%d", mssp_info->msp );
   mssp_reply( d, "SSL", "%d", mssp_info->ssl );
   mssp_reply( d, "MXP", "%d", mssp_info->mxp );
   mssp_reply( d, "PUEBLO", "%d", mssp_info->pueblo );
   mssp_reply( d, "VT100", "%d", mssp_info->vt100 );
   mssp_reply( d, "XTERM 256 COLORS", "%d", mssp_info->xterm256 );
   mssp_reply( d, "PAY TO PLAY", "%d", mssp_info->pay2play );
   mssp_reply( d, "PAY FOR PERKS", "%d", mssp_info->pay4perks );
   mssp_reply( d, "HIRING BUILDERS", "%d", mssp_info->hiringBuilders );
   mssp_reply( d, "HIRING CODERS", "%d", mssp_info->hiringCoders );
   mssp_reply( d, "ADULT MATERIAL", "%d", mssp_info->adultMaterial );
   mssp_reply( d, "MULTICLASSING", "%d", mssp_info->multiclassing );
   mssp_reply( d, "NEWBIE FRIENDLY", "%d", mssp_info->newbieFriendly );
   mssp_reply( d, "PLAYER CITIES", "%d", mssp_info->playerCities );
   mssp_reply( d, "PLAYER CLANSS", "%d", mssp_info->playerClans );
   mssp_reply( d, "PLAYER CRAFTING", "%d", mssp_info->playerCrafting );
   mssp_reply( d, "PLAYER GUILDS", "%d", mssp_info->playerGuilds );
   mssp_reply( d, "EQUIPMENT SYSTEM", "%s", mssp_info->equipmentSystem );
   mssp_reply( d, "MULTIPLAYING", "%s", mssp_info->multiplaying );
   mssp_reply( d, "PLAYERKILLING", "%s", mssp_info->playerKilling );
   mssp_reply( d, "QUEST SYSTEM", "%s", mssp_info->questSystem );
   mssp_reply( d, "ROLEPLAYING", "%s", mssp_info->roleplaying );
   mssp_reply( d, "TRAINING SYSTEM", "%s", mssp_info->trainingSystem );
   mssp_reply( d, "WORLD ORIGINALITY", "%s", mssp_info->worldOriginality );
   write_to_descriptor( d, "MSSP-REPLY-END\r\n", 0 );
}

void write_to_descriptor_printf( DESCRIPTOR_DATA * desc, const char *fmt, ... )
{
    char buf[MAX_STRING_LENGTH * 2];

    va_list args;

    va_start( args, fmt );
    vsprintf( buf, fmt, args );
    va_end( args );

    write_to_descriptor( desc, buf, strlen( buf ) );
}
