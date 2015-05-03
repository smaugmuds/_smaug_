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
     |                        -*- Hotboot Module -*-                       |
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
#include <dirent.h>
#include <unistd.h>
#if !defined(WIN32)
#include <dlfcn.h>
#else
#include <unistd.h>
#include <windows.h>
#define dlopen( libname, flags ) LoadLibrary( (libname) )
#define dlclose( libname ) FreeLibrary( (HINSTANCE) (libname) )
#endif

#include "mud.h"

#ifdef USE_MCCP
#include "mccp.h"
#endif

#define MAX_NEST	100
static OBJ_DATA *rgObjNest[MAX_NEST];

// bool write_to_descriptor( DESCRIPTOR_DATA * d, char *txt, int length );
// bool write_to_descriptor_old( int desc, char *txt, int length );

extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];

/*
 * Save the world's objects and mobs in their current positions -- Scion
 */
void save_mobile( FILE * fp, CHAR_DATA * mob )
{
   AFFECT_DATA *paf;
   SKILLTYPE *skill = NULL;

   if( !IS_NPC( mob ) || !fp )
      return;
   fprintf( fp, "%s", "#MOBILE\n" );
   fprintf( fp, "Vnum	%d\n", mob->pIndexData->vnum );
   fprintf( fp, "Level   %d\n", mob->level );
   fprintf( fp, "Gold	%d\n", mob->gold );
   if( mob->in_room )
   {
      if( xIS_SET( mob->act, ACT_SENTINEL ) )
      {
         /*
          * Sentinel mobs get stamped with a "home room" when they are created
          * by create_mobile(), so we need to save them in their home room regardless
          * of where they are right now, so they will go to their home room when they
          * enter the game from a reboot or copyover -- Scion 
          */
         fprintf( fp, "Room	%d\n", mob->home_vnum );
      }
      else
         fprintf( fp, "Room	%d\n", mob->in_room->vnum );
   }
   else
      fprintf( fp, "Room	%d\n", ROOM_VNUM_LIMBO );
#ifdef OVERLANDCODE
   fprintf( fp, "Coordinates  %d %d %d\n", mob->x, mob->y, mob->map );
#endif
   if( mob->name && mob->pIndexData->player_name && str_cmp( mob->name, mob->pIndexData->player_name ) )
      fprintf( fp, "Name     %s~\n", mob->name );
   if( mob->short_descr && mob->pIndexData->short_descr && str_cmp( mob->short_descr, mob->pIndexData->short_descr ) )
      fprintf( fp, "Short	%s~\n", mob->short_descr );
   if( mob->long_descr && mob->pIndexData->long_descr && str_cmp( mob->long_descr, mob->pIndexData->long_descr ) )
      fprintf( fp, "Long	%s~\n", mob->long_descr );
   if( mob->description && mob->pIndexData->description && str_cmp( mob->description, mob->pIndexData->description ) )
      fprintf( fp, "Description %s~\n", mob->description );
   fprintf( fp, "HpManaMove   %d %d %d %d %d %d\n",
            mob->hit, mob->max_hit, mob->mana, mob->max_mana, mob->move, mob->max_move );
   fprintf( fp, "Position %d\n", mob->position );
   fprintf( fp, "Flags %s\n", print_bitvector( &mob->act ) );
   if( !xIS_EMPTY( mob->affected_by ) )
      fprintf( fp, "AffectedBy   %s\n", print_bitvector( &mob->affected_by ) );

   for( paf = mob->first_affect; paf; paf = paf->next )
   {
      if( paf->type >= 0 && ( skill = get_skilltype( paf->type ) ) == NULL )
         continue;

      if( paf->type >= 0 && paf->type < TYPE_PERSONAL )
         fprintf( fp, "AffectData   '%s' %3d %3d %3d %s\n",
                  skill->name, paf->duration, paf->modifier, paf->location, print_bitvector( &paf->bitvector ) );
      else
         fprintf( fp, "Affect       %3d %3d %3d %3d %s\n",
                  paf->type, paf->duration, paf->modifier, paf->location, print_bitvector( &paf->bitvector ) );
   }

   de_equip_char( mob );

   if( mob->first_carrying )
      fwrite_obj( mob, mob->last_carrying, fp, 0, OS_CARRY, TRUE );

   re_equip_char( mob );

   fprintf( fp, "%s", "EndMobile\n\n" );
   return;
}

void save_world( void )
{
   FILE *mobfp;
   FILE *objfp;
   int mobfile = 0;
   char filename[256];
   CHAR_DATA *rch;
   ROOM_INDEX_DATA *pRoomIndex;
   int iHash;

   log_string( "Preserving world state...." );

   snprintf( filename, 256, "%s%s", SYSTEM_DIR, MOB_FILE );
   if( !( mobfp = fopen( filename, "w" ) ) )
   {
      bug( "%s", "save_world: fopen mob file" );
      perror( filename );
   }
   else
      mobfile++;

   for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
   {
      for( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
      {
         if( pRoomIndex )
         {
            if( !pRoomIndex->first_content   /* Skip room if nothing in it */
                || xIS_SET( pRoomIndex->room_flags, ROOM_CLANSTOREROOM ) /* These rooms save on their own */
                )
               continue;

            snprintf( filename, 256, "%s%d", HOTBOOT_DIR, pRoomIndex->vnum );
            if( ( objfp = fopen( filename, "w" ) ) == NULL )
            {
               bug( "save_world: fopen %d", pRoomIndex->vnum );
               perror( filename );
               continue;
            }
            fwrite_obj( NULL, pRoomIndex->last_content, objfp, 0, OS_CARRY, TRUE );
            fprintf( objfp, "%s", "#END\n" );
            fclose( objfp );
            objfp = NULL;
         }
      }
   }

   if( mobfile )
   {
      for( rch = first_char; rch; rch = rch->next )
      {
         if( !IS_NPC( rch ) || rch == supermob || xIS_SET( rch->act, ACT_PROTOTYPE ) || xIS_SET( rch->act, ACT_PET ) )
            continue;
         else
            save_mobile( mobfp, rch );
      }
      fprintf( mobfp, "%s", "#END\n" );
      fclose( mobfp );
      mobfp = NULL;
   }
   return;
}

CHAR_DATA *load_mobile( FILE * fp )
{
   CHAR_DATA *mob = NULL;
   const char *word;
   bool fMatch;
   int inroom = 0;
   ROOM_INDEX_DATA *pRoomIndex = NULL;

   word = feof( fp ) ? "EndMobile" : fread_word( fp );
   if( !str_cmp( word, "Vnum" ) )
   {
      int vnum;

      vnum = fread_number( fp );
      if( get_mob_index( vnum ) == NULL )
      {
         bug( "%s: No index data for vnum %d", __FUNCTION__, vnum );
         return NULL;
      }
      mob = create_mobile( get_mob_index( vnum ) );
      if( !mob )
      {
         for( ;; )
         {
            word = feof( fp ) ? "EndMobile" : fread_word( fp );
            /*
             * So we don't get so many bug messages when something messes up
             * * --Shaddai 
             */
            if( !str_cmp( word, "EndMobile" ) )
               break;
         }
         bug( "%s: Unable to create mobile for vnum %d", __FUNCTION__, vnum );
         return NULL;
      }
   }
   else
   {
      for( ;; )
      {
         word = feof( fp ) ? "EndMobile" : fread_word( fp );
         /*
          * So we don't get so many bug messages when something messes up
          * * --Shaddai 
          */
         if( !str_cmp( word, "EndMobile" ) )
            break;
      }
      extract_char( mob, TRUE );
      bug( "%s: Vnum not found", __FUNCTION__ );
      return NULL;
   }

   for( ;; )
   {
      word = feof( fp ) ? "EndMobile" : fread_word( fp );
      fMatch = FALSE;
      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

         case '#':
            if( !str_cmp( word, "#OBJECT" ) )
            {
               mob->tempnum = -9999;   /* Hackish, yes. Works though doesn't it? */
               fread_obj( mob, fp, OS_CARRY );
            }
            break;

         case 'A':
            if( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
            {
               AFFECT_DATA *paf;

               CREATE( paf, AFFECT_DATA, 1 );
               if( !str_cmp( word, "Affect" ) )
               {
                  paf->type = fread_number( fp );
               }
               else
               {
                  int sn;
                  char *sname = fread_word( fp );

                  if( ( sn = skill_lookup( sname ) ) < 0 )
                  {
                     if( ( sn = herb_lookup( sname ) ) < 0 )
                        bug( "%s: unknown skill.", __FUNCTION__ );
                     else
                        sn += TYPE_HERB;
                  }
                  paf->type = sn;
               }

               paf->duration = fread_number( fp );
               paf->modifier = fread_number( fp );
               paf->location = fread_number( fp );
               if( paf->location == APPLY_WEAPONSPELL
                   || paf->location == APPLY_WEARSPELL
                   || paf->location == APPLY_REMOVESPELL
                   || paf->location == APPLY_STRIPSN || paf->location == APPLY_RECURRINGSPELL )
                  paf->modifier = slot_lookup( paf->modifier );
               paf->bitvector = fread_bitvector( fp );
               LINK( paf, mob->first_affect, mob->last_affect, next, prev );
               fMatch = TRUE;
               break;
            }
            KEY( "AffectedBy", mob->affected_by, fread_bitvector( fp ) );
            break;

#ifdef OVERLANDCODE
         case 'C':
            if( !str_cmp( word, "Coordinates" ) )
            {
               mob->x = fread_number( fp );
               mob->y = fread_number( fp );
               mob->map = fread_number( fp );

               fMatch = TRUE;
               break;
            }
            break;
#endif

         case 'D':
            if( !str_cmp( word, "Description" ) )
            {
               STRFREE( mob->description );
               mob->description = fread_string( fp );
               fMatch = TRUE;
               break;
            }
            break;

         case 'E':
            if( !str_cmp( word, "EndMobile" ) )
            {
               if( inroom == 0 )
                  inroom = ROOM_VNUM_LIMBO;
               pRoomIndex = get_room_index( inroom );
               if( !pRoomIndex )
                  pRoomIndex = get_room_index( ROOM_VNUM_LIMBO );
               char_to_room( mob, pRoomIndex );
               mob->tempnum = -9998;   /* Yet another hackish fix! */
               return mob;
            }
            if( !str_cmp( word, "End" ) ) /* End of object, need to ignore this. sometimes they creep in there somehow -- Scion */
               fMatch = TRUE; /* Trick the system into thinking it matched something */
            break;

         case 'F':
            KEY( "Flags", mob->act, fread_bitvector( fp ) );
            break;

         case 'G':
            KEY( "Gold", mob->gold, fread_number( fp ) );
            break;

         case 'H':
            if( !str_cmp( word, "HpManaMove" ) )
            {
               mob->hit = fread_number( fp );
               mob->max_hit = fread_number( fp );
               mob->mana = fread_number( fp );
               mob->max_mana = fread_number( fp );
               mob->move = fread_number( fp );
               mob->max_move = fread_number( fp );

               if( mob->max_move <= 0 )
                  mob->max_move = 150;

               fMatch = TRUE;
               break;
            }
            break;

         case 'L':
            if( !str_cmp( word, "Long" ) )
            {
               STRFREE( mob->long_descr );
               mob->long_descr = fread_string( fp );
               fMatch = TRUE;
               break;
            }
            KEY( "Level", mob->level, fread_number( fp ) );
            break;

         case 'N':
            if( !str_cmp( word, "Name" ) )
            {
               STRFREE( mob->name );
               mob->name = fread_string( fp );
               fMatch = TRUE;
               break;
            }
            break;

         case 'P':
            KEY( "Position", mob->position, fread_number( fp ) );
            break;

         case 'R':
            KEY( "Room", inroom, fread_number( fp ) );
            break;

         case 'S':
            if( !str_cmp( word, "Short" ) )
            {
               STRFREE( mob->short_descr );
               mob->short_descr = fread_string( fp );
               fMatch = TRUE;
               break;
            }
            break;
      }

      if( !fMatch && str_cmp( word, "End" ) )
      {
         bug( "%s: no match: %s", __FUNCTION__, word );
         fread_to_eol( fp );
      }
   }
}

void read_obj_file( char *dirname, char *filename )
{
   ROOM_INDEX_DATA *room;
   FILE *fp;
   char fname[256];
   int vnum;

   vnum = atoi( filename );
   snprintf( fname, 256, "%s%s", dirname, filename );

   if( ( room = get_room_index( vnum ) ) == NULL )
   {
      bug( "read_obj_file: ARGH! Missing room index for %d!", vnum );
      unlink( fname );
      return;
   }

   if( ( fp = fopen( fname, "r" ) ) != NULL )
   {
      short iNest;
      bool found;
      OBJ_DATA *tobj, *tobj_next;

      rset_supermob( room );
      for( iNest = 0; iNest < MAX_NEST; iNest++ )
         rgObjNest[iNest] = NULL;

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
            bug( "%s", "read_obj_file: # not found." );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "OBJECT" ) ) /* Objects  */
            fread_obj( supermob, fp, OS_CARRY );
         else if( !str_cmp( word, "END" ) )  /* Done     */
            break;
         else
         {
            bug( "read_obj_file: bad section: %s", word );
            break;
         }
      }
      fclose( fp );
      fp = NULL;
      unlink( fname );
      for( tobj = supermob->first_carrying; tobj; tobj = tobj_next )
      {
         tobj_next = tobj->next_content;
#ifdef OVERLANDCODE
         if( IS_OBJ_STAT( tobj, ITEM_ONMAP ) )
         {
            SET_ACT_FLAG( supermob, ACT_ONMAP );
            supermob->map = tobj->map;
            supermob->x = tobj->x;
            supermob->y = tobj->y;
         }
#endif
         obj_from_char( tobj );
#ifndef OVERLANDCODE
         obj_to_room( tobj, room );
#else
         obj_to_room( tobj, room, supermob );
         REMOVE_ACT_FLAG( supermob, ACT_ONMAP );
         supermob->map = -1;
         supermob->x = -1;
         supermob->y = -1;
#endif
      }
      release_supermob(  );
   }
   else
      log_string( "Cannot open obj file" );

   return;
}

void load_obj_files( void )
{
   DIR *dp;
   struct dirent *dentry;
   char directory_name[100];

   log_string( "World state: loading objs" );
   snprintf( directory_name, 100, "%s", HOTBOOT_DIR );
   dp = opendir( directory_name );
   dentry = readdir( dp );
   while( dentry )
   {
      /*
       * Added by Tarl 3 Dec 02 because we are now using CVS 
       */
      if( !str_cmp( dentry->d_name, "CVS" ) )
      {
         dentry = readdir( dp );
         continue;
      }
      if( dentry->d_name[0] != '.' )
         read_obj_file( directory_name, dentry->d_name );
      dentry = readdir( dp );
   }
   closedir( dp );
   return;
}

void load_world( void )
{
   FILE *mobfp;
   char file1[256];
   char *word;
   int done = 0;
   bool mobfile = FALSE;

   snprintf( file1, 256, "%s%s", SYSTEM_DIR, MOB_FILE );
   if( ( mobfp = fopen( file1, "r" ) ) == NULL )
   {
      bug( "%s", "load_world: fopen mob file" );
      perror( file1 );
   }
   else
      mobfile = TRUE;

   if( mobfile )
   {
      log_string( "World state: loading mobs" );
      while( done == 0 )
      {
         if( feof( mobfp ) )
            done++;
         else
         {
            word = fread_word( mobfp );
            if( str_cmp( word, "#END" ) )
               load_mobile( mobfp );
            else
               done++;
         }
      }
      fclose( mobfp );
      mobfp = NULL;
   }

   load_obj_files(  );

   /*
    * Once loaded, the data needs to be purged in the event it causes a crash so that it won't try to reload 
    */
   unlink( file1 );
   return;
}

/*  Warm reboot stuff, gotta make sure to thank Erwin for this :) */
void do_hotboot( CHAR_DATA * ch, char *argument )
{
   FILE *fp;
   CHAR_DATA *victim = NULL;
   DESCRIPTOR_DATA *d, *de_next;
   char buf[100], buf2[100], buf3[100];
   char log_buf[MAX_STRING_LENGTH];
   extern int control;
	 int count = 0;
   bool found = FALSE;

   for( d = first_descriptor; d; d = d->next )
   {
      if( ( d->connected == CON_PLAYING || d->connected == CON_EDITING )
          && ( victim = d->character ) != NULL && !IS_NPC( victim ) && victim->in_room
          && victim->fighting && victim->level >= 1 && victim->level <= MAX_LEVEL )
      {
         found = TRUE;
         count++;
      }
   }

   if( found )
   {
      ch_printf( ch, "Cannot hotboot at this time. There are %d combats in progress.\r\n", count );
      return;
   }

   found = FALSE;
   for( d = first_descriptor; d; d = d->next )
   {
      if( d->connected == CON_EDITING && d->character )
      {
         found = TRUE;
         break;
      }
   }

   if( found )
   {
      send_to_char( "Cannot hotboot at this time. Someone is using the line editor.\r\n", ch );
      return;
   }

   snprintf( log_buf, MAX_STRING_LENGTH, "Hotboot initiated by %s.", ch->name );
   log_string( log_buf );

   fp = fopen( HOTBOOT_FILE, "w" );

   if( !fp )
   {
      send_to_char( "Hotboot file not writeable, aborted.\r\n", ch );
      bug( "Could not write to hotboot file: %s. Hotboot aborted.", HOTBOOT_FILE );
      perror( "do_copyover:fopen" );
      return;
   }

   /*
    * And this one here will save the status of all objects and mobs in the game.
    * * This really should ONLY ever be used here. The less we do stuff like this the better.
    */
   save_world(  );

   log_string( "Saving player files and connection states...." );
   if( ch && ch->desc )
      write_to_descriptor( ch->desc, "\033[0m", 0 );
   snprintf( buf, 100, "\r\nThe flow of time is halted momentarily as the world is reshaped!\r\n" );
   /*
    * For each playing descriptor, save its state 
    */
   for( d = first_descriptor; d; d = de_next )
   {
      CHAR_DATA *och = CH( d );
      de_next = d->next;   /* We delete from the list , so need to save this */
      if( !d->character || d->connected < CON_PLAYING )  /* drop those logging on */
      {
         write_to_descriptor( ch->desc, "\r\nSorry, we are rebooting. Come back in a few minutes.\r\n", 0 );
         close_socket( d, FALSE );  /* throw'em out */
      }
      else
      {
#ifdef USE_MCCP
         fprintf( fp, "%d %d %d %d %d %s %s\n", d->descriptor,
                  d->can_compress, och->in_room->vnum, d->port, d->idle, och->name, d->host );
#else
         fprintf( fp, "%d %d %d %d %s %s\n", d->descriptor,
                  och->in_room->vnum, d->port, d->idle, och->name, d->host );
#endif

         /*
          * One of two places this gets changed 
          */
         och->pcdata->hotboot = TRUE;
         save_char_obj( och );
         write_to_descriptor( ch->desc, buf, 0 );
#ifdef USE_MCCP
         compressEnd( d );
#endif
      }
   }

   fprintf( fp, "0 0 0 0 %d maxp maxp\n", sysdata.maxplayers );
   fprintf( fp, "%s", "-1\n" );
   fclose( fp );
   fp = NULL;

   /*
    * added this in case there's a need to debug the contents of the various files 
    */
   if( argument && !str_cmp( argument, "debug" ) )
   {
      log_string( "Hotboot debug - Aborting before execl" );
      return;
   }

   log_string( "Executing hotboot...." );

   /*
    * exec - descriptors are inherited 
    */
   snprintf( buf, 100, "%d", port );
   snprintf( buf2, 100, "%d", control );
   strncpy( buf3, "-1", 100 );

   set_alarm( 0 );
   dlclose( sysdata.dlHandle );
   execl( EXE_FILE, PACKAGE, buf, "hotboot", buf2, buf3, ( char * )NULL );

   /*
    * Failed - sucessful exec will not return 
    */
   perror( "do_hotboot: execl" );

   sysdata.dlHandle = dlopen( NULL, RTLD_LAZY );
   if( !sysdata.dlHandle )
   {
      bug( "%s", "FATAL ERROR: Unable to reopen system executable handle!" );
      exit( 1 );
   }

   bug( "%s", "Hotboot execution failed!!" );
   send_to_char( "Hotboot FAILED!\r\n", ch );
}

/* Recover from a hotboot - load players */
void hotboot_recover( void )
{
   DESCRIPTOR_DATA *d = NULL;
   FILE *fp;
   char name[100];
   char host[MAX_STRING_LENGTH];
   int desc, dcompress, room, dport, idle, maxp = 0;
   bool fOld;

   fp = fopen( HOTBOOT_FILE, "r" );

   if( !fp )   /* there are some descriptors open which will hang forever then ? */
   {
      perror( "hotboot_recover: fopen" );
      bug( "%s", "Hotboot file not found. Exitting." );
      exit( 1 );
   }

   unlink( HOTBOOT_FILE ); /* In case something crashes - doesn't prevent reading */
   for( ;; )
   {
      d = NULL;

      fscanf( fp, "%d %d %d %d %d %s %s\n", &desc, &dcompress, &room, &dport, &idle, name, host );

      if( desc == -1 || feof( fp ) )
         break;

      if( !str_cmp( name, "maxp" ) || !str_cmp( host, "maxp" ) )
      {
         maxp = idle;
         continue;
      }

      /*
       * Write something, and check if it goes error-free 
       */
      if( !dcompress && !write_to_descriptor( desc, "\r\nThe ether swirls in chaos.\r\n", 0 ) )
      {
         close( desc ); /* nope */
         continue;
      }

      CREATE( d, DESCRIPTOR_DATA, 1 );

      d->next = NULL;
      d->descriptor = desc;
      d->connected = CON_GET_NAME;
      d->outsize = 2000;
      d->idle = 0;
      d->lines = 0;
      d->scrlen = 24;
      d->newstate = 0;
      d->prevcolor = 0x08;
      d->ifd = -1;
      d->ipid = -1;

      CREATE( d->outbuf, char, d->outsize );

      d->host = STRALLOC( host );
      d->port = dport;
      d->idle = idle;
#ifdef USE_MCCP
      CREATE( d->mccp, MCCP, 1 );
      d->can_compress = dcompress;
      if( d->can_compress )
         compressStart( d );
#endif
      LINK( d, first_descriptor, last_descriptor, next, prev );
      d->connected = CON_COPYOVER_RECOVER;   /* negative so close_socket will cut them off */

      /*
       * Now, find the pfile 
       */
      fOld = load_char_obj( d, name, FALSE );

      if( !fOld ) /* Player file not found?! */
      {
         write_to_descriptor( desc, "\r\nSomehow, your character was lost during hotboot. Contact the immortals ASAP.\r\n", 0 );
         close_socket( d, FALSE );
      }
      else  /* ok! */
      {
         write_to_descriptor( desc, "\r\nTime resumes its normal flow.\r\n", 0 );
         d->character->in_room = get_room_index( room );
         if( !d->character->in_room )
            d->character->in_room = get_room_index( ROOM_VNUM_TEMPLE );

         /*
          * Insert in the char_list 
          */
         LINK( d->character, first_char, last_char, next, prev );

         char_to_room( d->character, d->character->in_room );
         act( AT_MAGIC, "A puff of ethereal smoke dissipates around you!", d->character, NULL, NULL, TO_CHAR );
         act( AT_MAGIC, "$n appears in a puff of ethereal smoke!", d->character, NULL, NULL, TO_ROOM );
         d->connected = CON_PLAYING;
         if( ++num_descriptors > sysdata.maxplayers )
            sysdata.maxplayers = num_descriptors;
#ifdef AUTO_AUTH
         check_auth_state( d->character );   /* new auth */
#endif
      }
   }
   fclose( fp );
   fp = NULL;
   if( maxp > sysdata.maxplayers )
      sysdata.maxplayers = maxp;
   log_string( "Hotboot recovery complete." );
   return;
}
