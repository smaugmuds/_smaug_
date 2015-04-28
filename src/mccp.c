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
 * ------------------------------------------------------------------------ *
 *                         Client Compression Module                        *
 ****************************************************************************/

/*
 * mccp.c - support functions for mccp (the Mud Client Compression Protocol)
 *
 * see http://mccp.afkmud.com
 *
 * Copyright (c) 1999, Oliver Jowett <oliver@randomly.org>.
 *
 * This code may be freely distributed and used if this copyright notice is
 * retained intact.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#ifndef WIN32
#include <arpa/telnet.h>
#else
#define  TELOPT_ECHO        '\x01'
#define  GA                 '\xF9'
#define  SB                 '\xFA'
#define  SE                 '\xF0'
#define  WILL               '\xFB'
#define  WONT               '\xFC'
#define  DO                 '\xFD'
#define  DONT               '\xFE'
#define  IAC                '\xFF'
#define ENOSR 63
#endif
#include "mud.h"
#include "mccp.h"

#if defined(__OpenBSD__) || defined(__FreeBSD__)
#define ENOSR 63
#endif

char will_compress2_str[] = { IAC, WILL, TELOPT_COMPRESS2, '\0' };
char start_compress2_str[] = { IAC, SB, TELOPT_COMPRESS2, IAC, SE, '\0' };

bool write_to_descriptor( DESCRIPTOR_DATA * d, char *txt, int length );

bool process_compressed( DESCRIPTOR_DATA * d )
{
   int iStart = 0, nBlock, nWrite, len;

   if( !d->mccp->out_compress )
      return TRUE;

   /*
    * Try to write out some data.. 
    */
   len = d->mccp->out_compress->next_out - d->mccp->out_compress_buf;

   if( len > 0 )
   {
      /*
       * we have some data to write 
       */
      for( iStart = 0; iStart < len; iStart += nWrite )
      {
         nBlock = UMIN( len - iStart, 4096 );
         if( ( nWrite = write( d->descriptor, d->mccp->out_compress_buf + iStart, nBlock ) ) < 0 )
         {
            if( errno == EAGAIN || errno == ENOSR )
               break;

            return FALSE;
         }

         if( !nWrite )
            break;
      }

      if( iStart )
      {
         /*
          * We wrote "iStart" bytes 
          */
         if( iStart < len )
            memmove( d->mccp->out_compress_buf, d->mccp->out_compress_buf + iStart, len - iStart );

         d->mccp->out_compress->next_out = d->mccp->out_compress_buf + len - iStart;
      }
   }
   return TRUE;
}

bool compressStart( DESCRIPTOR_DATA * d )
{
   z_stream *s;

   if( d->mccp->out_compress )
      return TRUE;

   CREATE( s, z_stream, 1 );
   CREATE( d->mccp->out_compress_buf, unsigned char, COMPRESS_BUF_SIZE );

   s->next_in = NULL;
   s->avail_in = 0;

   s->next_out = d->mccp->out_compress_buf;
   s->avail_out = COMPRESS_BUF_SIZE;

   s->zalloc = Z_NULL;
   s->zfree = Z_NULL;
   s->opaque = NULL;

   if( deflateInit( s, 9 ) != Z_OK )
   {
      DISPOSE( d->mccp->out_compress_buf );
      DISPOSE( s );
      return FALSE;
   }

   write_to_descriptor( d, start_compress2_str, 0 );

   d->can_compress = TRUE;
   d->mccp->out_compress = s;

   return TRUE;
}

bool compressEnd( DESCRIPTOR_DATA * d )
{
   unsigned char dummy[1];

   if( !d->mccp->out_compress )
      return TRUE;

   d->mccp->out_compress->avail_in = 0;
   d->mccp->out_compress->next_in = dummy;

   if( deflate( d->mccp->out_compress, Z_FINISH ) != Z_STREAM_END )
      return FALSE;

   if( !process_compressed( d ) )   /* try to send any residual data */
      return FALSE;

   deflateEnd( d->mccp->out_compress );
   DISPOSE( d->mccp->out_compress_buf );
   DISPOSE( d->mccp->out_compress );

   return TRUE;
}

void do_compress( CHAR_DATA * ch, char *argument )
{
   if( !ch->desc )
   {
      send_to_char( "What descriptor?!\n", ch );
      return;
   }

   if( !ch->desc->mccp->out_compress )
   {
      if( !compressStart( ch->desc ) )
         send_to_char( "&RCompression failed to start.\r\n", ch );
      else
         send_to_char( "&GOk, compression enabled.\n", ch );
   }
   else
   {
      compressEnd( ch->desc );
      ch->desc->can_compress = FALSE;
      send_to_char( "&ROk, compression disabled.\n", ch );
   }
}
