/***************************************************************************
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
 ***************************************************************************/

/*
 * mccp.c - support functions for mccp (the Mud Client Compression Protocol)
 *
 * see http://homepages.ihug.co.nz/~icecube/compress/ and README.Rom24-mccp
 *
 * Copyright (c) 1999, Oliver Jowett <icecube@ihug.co.nz>.
 *
 * This code may be freely distributed and used if this copyright notice is
 * retained intact.
 *
 * Modified for Merc 2.2 by Celestian (celestian1@gmail.com)
 */

#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <arpa/telnet.h>
#include <zlib.h>
#include <malloc.h>

#include "mud.h"
#include "mccp.h"
  
char    compress_start  [] = { IAC, SB, TELOPT_COMPRESS, WILL, SE, '\0' };

bool processCompressed(DESCRIPTOR_DATA *desc);

int  mccp_mem_usage = 0;
int  mccp_mem_freed = 0;

/*
 * Memory management - zlib uses these hooks to allocate and free memory
 * it needs
 */

void *zlib_alloc(void *opaque, unsigned int items, unsigned int size)
{
  /* memory tracking */
  mccp_mem_usage += size;

  /* return memory pointer */
  return calloc(items, size);
}

void zlib_free(void *opaque, void *address)
{
  /* memory tracking */
  mccp_mem_freed += sizeof(*address);

  /* free the cell */
  free(address);
}

/*
 * Begin compressing data on `desc'
 */
bool compressStart(DESCRIPTOR_DATA *desc)
{
    z_stream *s;

    if (desc->out_compress) /* already compressing */
        return TRUE;

    /* allocate and init stream, buffer */
    s = (z_stream *)malloc(sizeof(*s));
    desc->out_compress_buf = (unsigned char *)malloc(COMPRESS_BUF_SIZE);

    s->next_in = NULL;
    s->avail_in = 0;

    s->next_out = desc->out_compress_buf;
    s->avail_out = COMPRESS_BUF_SIZE;
 
    s->zalloc = zlib_alloc;
    s->zfree  = zlib_free;
    s->opaque = NULL;

    if (deflateInit(s, 9) != Z_OK) {
        /* problems with zlib, try to clean up */
        free(desc->out_compress_buf);
        free(s);
        return FALSE;
    }
    
    write_to_descriptor(desc, compress_start, strlen(compress_start));

    /* now we're compressing */
    desc->out_compress = s;
    return TRUE;
}

/* Cleanly shut down compression on `desc' */
bool compressEnd(DESCRIPTOR_DATA *desc)
{
    unsigned char dummy[1];

    if (!desc->out_compress)
        return TRUE;

    desc->out_compress->avail_in = 0;
    desc->out_compress->next_in = dummy;
    
    /* No terminating signature is needed - receiver will get Z_STREAM_END */

    if (deflate(desc->out_compress, Z_FINISH) != Z_STREAM_END)
        return FALSE;
    
    if (!processCompressed(desc)) /* try to send any residual data */
        return FALSE;

    deflateEnd(desc->out_compress); 
    free(desc->out_compress_buf);
    free(desc->out_compress);
    desc->out_compress = NULL;
    desc->out_compress_buf = NULL;

    return TRUE;
}

bool compressEnd2(DESCRIPTOR_DATA *desc)
{
    unsigned char dummy[1];

    if (!desc->out_compress)
        return TRUE;
  
    desc->out_compress->avail_in = 0;
    desc->out_compress->next_in = dummy;
    
    /* No terminating signature is needed - receiver will get Z_STREAM_END */
    
    if (deflate(desc->out_compress, Z_FINISH) != Z_STREAM_END)
        return FALSE;

    if (!processCompressed(desc)) /* try to send any residual data */
        return FALSE;
    
    deflateEnd(desc->out_compress);
    desc->out_compress = NULL;
    desc->out_compress_buf = NULL;   

    return TRUE;
}


/* Try to send any pending compressed-but-not-sent data in `desc' */
bool processCompressed(DESCRIPTOR_DATA *desc)
{
    int iStart, nBlock, nWrite, len;
 
    if (!desc->out_compress)
        return TRUE;
 
    /* Try to write out some data.. */
    len = desc->out_compress->next_out - desc->out_compress_buf;
    if (len > 0) {
        /* we have some data to write */

        /* logging the amount of data sent - mudinfo.c snippet */
//        increase_total_output(len);

        for (iStart = 0; iStart < len; iStart += nWrite)
        {
            nBlock = UMIN (len - iStart, 4096);
            if ((nWrite = write (desc->descriptor, desc->out_compress_buf + iStart, nBlock)) < 0)
            {
                if (errno == EAGAIN ||
                    errno == ENOSR)
                    break;
    
                return FALSE; /* write error */
            }
    
            if (nWrite <= 0)
                break;
        }
    
        if (iStart) {
            /* We wrote "iStart" bytes */
            if (iStart < len)
                memmove(desc->out_compress_buf, desc->out_compress_buf+iStart, len - iStart);

            desc->out_compress->next_out = desc->out_compress_buf + len - iStart;
        }
    }
 
    return TRUE;
}
 
/* write_to_descriptor, the compressed case */
bool writeCompressed(DESCRIPTOR_DATA *desc, char *txt, int length)
{
    z_stream *s = desc->out_compress;   

    s->next_in = (unsigned char *)txt;
    s->avail_in = length;
    
    while (s->avail_in) {
        s->avail_out = COMPRESS_BUF_SIZE - (s->next_out - desc->out_compress_buf);
                
        if (s->avail_out) {
            int status = deflate(s, Z_SYNC_FLUSH);
    
            if (status != Z_OK) {
                /* Boom */
                return FALSE;
            }
        }
         
        /* Try to write out some data.. */
        if (!processCompressed(desc))
            return FALSE;
            
        /* loop */
    }
            
    /* Done. */
    return TRUE;
}
    
void do_compres( CHAR_DATA *ch, char *argument )
{
  send_to_char("If you want to use compress, spell it out.\n\r",ch);
  return;
}

/* User-level compression toggle */
void do_compress( CHAR_DATA *ch, char *argument )
{
    if (!ch->desc) {
        send_to_char("What descriptor?!\n", ch);
        return;
    }

    if (!ch->desc->out_compress) {
        if (!compressStart(ch->desc)) {
            send_to_char("Failed.\n", ch);
            return;
        }       
 
      send_to_char("Ok, compression enabled.\n", ch);
   } 
   else {
       if (!compressEnd(ch->desc)) {
           send_to_char("Failed.\n", ch);
            return;
        }
         
        send_to_char("Ok, compression disabled.\n", ch);
    }
}

void do_showcompress( CHAR_DATA *ch, char *argument )
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *gch;
  char buf[MAX_STRING_LENGTH];
  int count1 = 0;
  int count2 = 0;

  if (IS_NPC(ch)) return;

  for (d = gch->desc; d != NULL; d = d->next)
  {
    if (d->connected != CON_PLAYING) continue;
    if (d->character != NULL) gch = d->character;
    else continue;
    if (gch->level > 6) continue;
    if (gch->desc->out_compress)
    {
      sprintf(buf, "%-15s uses mccp\n\r", gch->name);
      count1++;
    }
    else
    {
      sprintf(buf,"%-15s Does not use mccp.\n\r", gch->name);
      count2++;
    }
    send_to_char(buf, ch);
  }
  sprintf(buf,"\n\r%d out of %d players uses mccp\n\r",
    count1, count2 + count1);
  send_to_char(buf, ch);
  return;               
}

