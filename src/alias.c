/****************************************************************************
 *                   ^     +----- |  / ^     ^ |     | +-\                  *
 *                  / \    |      | /  |\   /| |     | |  \                 *
 *                 /   \   +---   |<   | \ / | |     | |  |                 *
 *                /-----\  |      | \  |  v  | |     | |  /                 *
 *               /       \ |      |  \ |     | +-----+ +-/                  *
 ****************************************************************************
 * AFKMud Copyright 1997-2003 by Roger Libiez (Samson),                     *
 * Levi Beckerson (Whir), Michael Ward (Tarl), Erik Wolfe (Dwip),           *
 * Cameron Carroll (Cam), Cyberfox, Karangi, Rathian, Raine, and Adjani.    *
 * All Rights Reserved.                                                     *
 *                                                                          *
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag,        *
 * Blodkai, Haus, Narn, Scryn, Swordbearer, Tricops, Gorog, Rennard,        *
 * Grishnakh, Fireblade, and Nivek.                                         *
 *                                                                          *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                      *
 *                                                                          *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen,      *
 * Michael Seifert, and Sebastian Hammer.                                   *
 ****************************************************************************
 *                             Alias module                                 *
 ****************************************************************************/

/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997, 1998  Jesse DeFer and Heath Leach
 http://dotd.mudservices.com  dotd@dotd.mudservices.com 
 ******************************************************/

#include <string.h>
#include "mud.h"

ALIAS_DATA *find_alias( CHAR_DATA * ch, char *argument )
{
   ALIAS_DATA *pal;

   if( !ch || !ch->pcdata )
      return ( NULL );

   for( pal = ch->pcdata->first_alias; pal; pal = pal->next )
      if( !str_prefix( argument, pal->name ) )
         return ( pal );

   return ( NULL );
}

void free_alias( CHAR_DATA * ch, ALIAS_DATA * pal )
{
   STRFREE( pal->name );
   STRFREE( pal->cmd );
   UNLINK( pal, ch->pcdata->first_alias, ch->pcdata->last_alias, next, prev );
   DISPOSE( pal );
   return;
}

void do_alias( CHAR_DATA * ch, char *argument )
{
   ALIAS_DATA *pal = NULL;
   char arg[MAX_INPUT_LENGTH];
   char *p;

   if( IS_NPC( ch ) )
      return;

   for( p = argument; *p != '\0'; p++ )
   {
      if( *p == '~' )
      {
         send_to_char( "Command not acceptable, cannot use the ~ character.\r\n", ch );
         return;
      }
   }

   argument = one_argument( argument, arg );

   if( !arg || arg[0] == '\0' )
   {
      if( !ch->pcdata->first_alias )
      {
         send_to_char( "You have no aliases defined!\r\n", ch );
         return;
      }
      pager_printf( ch, "%-20s What it does\r\n", "Alias" );
      for( pal = ch->pcdata->first_alias; pal; pal = pal->next )
         pager_printf( ch, "%-20s %s\r\n", pal->name, pal->cmd );
      return;
   }

   if( !argument || argument[0] == '\0' )
   {
      if( ( pal = find_alias( ch, arg ) ) != NULL )
      {
         free_alias( ch, pal );
         send_to_char( "Deleted Alias.\r\n", ch );
      }
      else
         send_to_char( "That alias does not exist.\r\n", ch );
      return;
   }

   if( ( pal = find_alias( ch, arg ) ) == NULL )
   {
      CREATE( pal, ALIAS_DATA, 1 );
      pal->name = STRALLOC( arg );
      pal->cmd = STRALLOC( argument );
      LINK( pal, ch->pcdata->first_alias, ch->pcdata->last_alias, next, prev );
      send_to_char( "Created Alias.\r\n", ch );
   }
   else
   {
      STRFREE( pal->cmd );
      pal->cmd = STRALLOC( argument );
      send_to_char( "Modified Alias.\r\n", ch );
   }
}

void free_aliases( CHAR_DATA * ch )
{
   ALIAS_DATA *pal, *next_pal;

   if( !ch || !ch->pcdata )
      return;

   for( pal = ch->pcdata->first_alias; pal; pal = next_pal )
   {
      next_pal = pal->next;
      free_alias( ch, pal );
   }
}

bool check_alias( CHAR_DATA * ch, char *command, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   ALIAS_DATA *alias;

   if( !( alias = find_alias( ch, command ) ) )
      return FALSE;

   if( !alias->cmd || !*alias->cmd )
      return FALSE;

   strncpy( arg, alias->cmd, MAX_INPUT_LENGTH );

   if( ch->cmd_recurse == -1 || ++ch->cmd_recurse > 50 )
   {
      if( ch->cmd_recurse != -1 )
      {
         send_to_char( "Unable to further process command, recurses too much.\r\n", ch );
         ch->cmd_recurse = -1;
      }
      return FALSE;
   }

   if( argument && argument[0] != '\0' )
   {
      mudstrlcat( arg, " ", MAX_INPUT_LENGTH );
      mudstrlcat( arg, argument, MAX_INPUT_LENGTH );
   }
   interpret( ch, arg );
   return TRUE;
}
