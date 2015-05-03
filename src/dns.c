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
     |                     -*- DNS Resolver Module -*-                     |
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
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include "mud.h"

DNS_DATA *first_cache;
DNS_DATA *last_cache;

void save_dns( void );

void prune_dns( void )
{
   DNS_DATA *cache, *cache_next;

   for( cache = first_cache; cache; cache = cache_next )
   {
      cache_next = cache->next;

      /*
       * Stay in cache for 14 days 
       */
      if( current_time - cache->time >= 1209600 || !str_cmp( cache->ip, "Unknown??" )
          || !str_cmp( cache->name, "Unknown??" ) )
      {
         STRFREE( cache->ip );
         STRFREE( cache->name );
         UNLINK( cache, first_cache, last_cache, next, prev );
         DISPOSE( cache );
      }
   }
   save_dns(  );
}

void check_dns( void )
{
   if( current_time >= new_boot_time_t )
      prune_dns(  );
}

void add_dns( char *dhost, char *address )
{
   DNS_DATA *cache;

   CREATE( cache, DNS_DATA, 1 );
   cache->ip = STRALLOC( dhost );
   cache->name = STRALLOC( address );
   cache->time = current_time;
   LINK( cache, first_cache, last_cache, next, prev );

   save_dns(  );
}

char *in_dns_cache( char *ip )
{
   DNS_DATA *cache;
   static char dnsbuf[MAX_STRING_LENGTH];

   dnsbuf[0] = '\0';

   for( cache = first_cache; cache; cache = cache->next )
   {
      if( !str_cmp( ip, cache->ip ) )
      {
         strncpy( dnsbuf, cache->name, MAX_STRING_LENGTH );
         break;
      }
   }
   return dnsbuf;
}

void fread_dns( DNS_DATA * cache, FILE * fp )
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

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !cache->ip )
                  cache->ip = STRALLOC( "Unknown??" );
               if( !cache->name )
                  cache->name = STRALLOC( "Unknown??" );
               return;
            }
            break;

         case 'I':
            KEY( "IP", cache->ip, fread_string( fp ) );
            break;

         case 'N':
            KEY( "Name", cache->name, fread_string( fp ) );
            break;

         case 'T':
            KEY( "Time", cache->time, fread_number( fp ) );
            break;
      }

      if( !fMatch )
         bug( "fread_dns: no match: %s", word );
   }
}

void load_dns( void )
{
   char filename[256];
   DNS_DATA *cache;
   FILE *fp;

   first_cache = NULL;
   last_cache = NULL;

   snprintf( filename, 256, "%s", DNS_FILE );

   if( ( fp = fopen( filename, "r" ) ) != NULL )
   {
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
            bug( "%s: # not found.", __FUNCTION__ );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "CACHE" ) )
         {
            CREATE( cache, DNS_DATA, 1 );
            fread_dns( cache, fp );
            LINK( cache, first_cache, last_cache, next, prev );
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "%s: bad section: %s.", __FUNCTION__, word );
            continue;
         }
      }
      fclose( fp );
      fp = NULL;
   }
   prune_dns(  ); /* Clean out entries beyond 14 days */
}

void save_dns( void )
{
   DNS_DATA *cache;
   FILE *fp;
   char filename[256];

   snprintf( filename, 256, "%s", DNS_FILE );

   if( !( fp = fopen( filename, "w" ) ) )
   {
      bug( "%s: fopen", __FUNCTION__ );
      perror( filename );
   }
   else
   {
      for( cache = first_cache; cache; cache = cache->next )
      {
         fprintf( fp, "#CACHE\n" );
         fprintf( fp, "IP		%s~\n", cache->ip );
         fprintf( fp, "Name		%s~\n", cache->name );
         fprintf( fp, "Time		%ld\n", cache->time );
         fprintf( fp, "End\n\n" );
      }
      fprintf( fp, "#END\n" );
      fclose( fp );
      fp = NULL;
   }
}

/* DNS Resolver code by Trax of Forever's End */
/*
 * Almost the same as read_from_buffer...
 */
bool read_from_dns( int fd, char *buffer )
{
   static char inbuf[MAX_STRING_LENGTH * 2];
   unsigned int iStart;
   int i, j, k;

   /*
    * Check for overflow. 
    */
   iStart = strlen( inbuf );
   if( iStart >= sizeof( inbuf ) - 10 )
   {
      bug( "%s: DNS input overflow!!!", __FUNCTION__ );
      return FALSE;
   }

   /*
    * Snarf input. 
    */
   for( ;; )
   {
      int nRead;

      nRead = read( fd, inbuf + iStart, sizeof( inbuf ) - 10 - iStart );
      if( nRead > 0 )
      {
         iStart += nRead;
         if( inbuf[iStart - 2] == '\n' || inbuf[iStart - 2] == '\r' )
            break;
      }
      else if( nRead == 0 )
      {
         return FALSE;
      }
      else if( errno == EWOULDBLOCK )
         break;
      else
      {
         perror( "Read_from_dns" );
         return FALSE;
      }
   }

   inbuf[iStart] = '\0';

   /*
    * Look for at least one new line.
    */
   for( i = 0; inbuf[i] != '\n' && inbuf[i] != '\r'; i++ )
   {
      if( inbuf[i] == '\0' )
         return FALSE;
   }

   /*
    * Canonical input processing.
    */
   for( i = 0, k = 0; inbuf[i] != '\n' && inbuf[i] != '\r'; i++ )
   {
      if( inbuf[i] == '\b' && k > 0 )
         --k;
      else if( isascii( inbuf[i] ) && isprint( inbuf[i] ) )
         buffer[k++] = inbuf[i];
   }

   /*
    * Finish off the line.
    */
   if( k == 0 )
      buffer[k++] = ' ';
   buffer[k] = '\0';

   /*
    * Shift the input buffer.
    */
   while( inbuf[i] == '\n' || inbuf[i] == '\r' )
      i++;
   for( j = 0; ( inbuf[j] = inbuf[i + j] ) != '\0'; j++ )
      ;

   return TRUE;
}

/* DNS Resolver code by Trax of Forever's End */
/*
 * Process input that we got from resolve_dns.
 */
void process_dns( DESCRIPTOR_DATA * d )
{
   char address[MAX_INPUT_LENGTH];

   address[0] = '\0';

   if( !read_from_dns( d->ifd, address ) || address[0] == '\0' )
      return;

   if( address[0] != '\0' )
   {
      add_dns( d->host, address );  /* Add entry to DNS cache */
      STRFREE( d->host );
      d->host = STRALLOC( address );
   }

   /*
    * close descriptor and kill dns process 
    */
   if( d->ifd != -1 )
   {
      close( d->ifd );
      d->ifd = -1;
   }
}

/* DNS Resolver hook. Code written by Trax of Forever's End */
void resolve_dns( DESCRIPTOR_DATA * d, long ip )
{
   int fds[2];
   pid_t pid;

   /*
    * create pipe first 
    */
   if( pipe( fds ) != 0 )
   {
      perror( "resolve_dns: pipe: " );
      return;
   }

   if( dup2( fds[1], STDOUT_FILENO ) != STDOUT_FILENO )
   {
      perror( "resolve_dns: dup2(stdout): " );
      return;
   }

   if( ( pid = fork(  ) ) > 0 )
   {
      /*
       * parent process 
       */
      d->ifd = fds[0];
      d->ipid = pid;
      close( fds[1] );
   }
   else if( pid == 0 )
   {
      /*
       * child process 
       */
      char str_ip[64];
      int i;

      d->ifd = fds[0];
      d->ipid = pid;

      for( i = 2; i < 255; ++i )
         close( i );

      snprintf( str_ip, 64, "%ld", ip );
      execl( "./smaug-resolv", "DNS Resolver", str_ip, NULL );
      /*
       * Still here --> hmm. An error. 
       */
      bug( "%s: Exec failed; Closing child.", __FUNCTION__ );
      d->ifd = -1;
      d->ipid = -1;
      exit( 0 );
   }
   else
   {
      /*
       * error 
       */
      perror( "resolve_dns: failed fork" );
      close( fds[0] );
      close( fds[1] );
   }
}

void do_cache( CHAR_DATA * ch, char *argument )
{
   DNS_DATA *cache;
   int ip = 0;

   send_to_pager( "&YCached DNS Information\r\n", ch );
   send_to_pager( "IP               | Address\r\n", ch );
   send_to_pager( "------------------------------------------------------------------------------\r\n", ch );
   for( cache = first_cache; cache; cache = cache->next )
   {
      pager_printf( ch, "&W%16.16s  &Y%s\r\n", cache->ip, cache->name );
      ip++;
   }
   pager_printf( ch, "\r\n&W%d IPs in the cache.\r\n", ip );
}
