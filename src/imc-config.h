/*
 * IMC2 - an inter-mud communications protocol
 *
 * imc-config.h: configuration information
 *
 * Copyright (C) 1996,1997 Oliver Jowett <oliver@jowett.manawatu.planet.co.nz>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, write to the
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef IMC_CONFIG_H
#define IMC_CONFIG_H

/*
 *  This file contains most of the tweakable parameters of the core IMC code
 */


/* NO_OPTIMIZE makes it so your MUD won't try to find a hub with less
direct connections to it. This should probably NOT be defined unless you
have a wacky setup that you don't want switched. 1/5/99 -- Scion */

/* #define NO_OPTIMIZE 42 */


/*  system defines: #define NO_xxxx to 1 if your system doesn't support a
 *  function
 */

/* for memmove() */
#ifndef NO_MEMMOVE
#define NO_MEMMOVE 0
#endif

/* for vsnprintf()/snprintf() (probably most non-linux systems) */
#ifndef NO_VSNPRINTF
#ifndef linux
#define NO_VSNPRINTF 1
#else
#define NO_VSNPRINTF 0
#endif
#endif

/* for strerror() (eg. some versions of SunOS) */
#ifndef NO_STRERROR
#define NO_STRERROR 0
#endif

/* for strtoul() */
#ifndef NO_STRTOUL
#define NO_STRTOUL 0
#endif

/* #define this if you aren't using gcc (or if __attribute__ isn't supported
 * for some other reason)
 */

#ifndef NO_ATTRIBUTE
#define NO_ATTRIBUTE 0
#endif

/* some systems don't have EINTR, #define this if you don't */

#ifndef NO_EINTR
#define NO_EINTR 0
#endif

/* Less tweakable parameters - only change these if you know what they do */

/* number of packets to remember at a time */
#define IMC_MEMORY 256

/* start dropping really old packets based on this figure */
#define IMC_PACKET_LIFETIME 60

/* Maximum reconnect time (seconds) */
#define IMC_MAX_RECONNECT_TIME (90*60)
/* How soon the first reconnect attempt is (seconds) */
#define IMC_MIN_RECONNECT_TIME (2*60)

/* maximum time spent in getting a connection to a remote mud */
#define IMC_LOGIN_TIMEOUT 120

/* time between keepalive broadcasts (seconds) */
#define IMC_KEEPALIVE_TIME 300

/* time before dropping a mud off imclist (seconds) */
#define IMC_KEEPALIVE_TIMEOUT 400
/* time after it drops off imclist before totally forgetting about it (secs) */
#define IMC_DROP_TIMEOUT 600

/* Packet spam thresholds */

/* counter 1
 *  allow > 1/second for 10s
 *  after that, clip to 1/second until 10s of <1/s
 */
#define IMC_SPAM1INTERVAL 1
#define IMC_SPAM1MAX      1
#define IMC_SPAM1TIME     10

/* counter 2
 *  allow > 3/second for 5s
 *  after that, clip to 3/second until 5s of <3/s
 */
#define IMC_SPAM2INTERVAL 1
#define IMC_SPAM2MAX      3
#define IMC_SPAM2TIME     5


/* This is the protocol version */
#define IMC_VERSION 2
/* This is the code version ID (used in keepalives) */
//#define IMC_VERSIONID "imc2-1.03g hub"
//#define IMC_VERSIONID "imc2-1.03g webcgi"
//#define IMC_VERSIONID "imc2-1.03g circle"
//#define IMC_VERSIONID "imc2-1.03g smaug102"
#define IMC_VERSIONID "imc2-1.03g smaug1.4a"
//#define IMC_VERSIONID "imc2-1.03g ack"
//#define IMC_VERSIONID "imc2-1.03g envy"
//#define IMC_VERSIONID "imc2-1.03g merc"
//#define IMC_VERSIONID "imc2-1.03g rom"
//#define IMC_VERSIONID "imc2-1.03g rot"

/* enable paranoia in packet forwarding (generally a Good Thing) */
#define IMC_PARANOIA

/* time with a small buffer before shrinking it by a factor of 2 */
#define IMC_SHRINKTIME    10

/* min input/output buffer size */
#define IMC_MINBUF        256
/* max input/output buffer size */
#define IMC_MAXBUF        16384

/* Changing these impacts the protocol itself - other muds may drop your
 * packets if you get this wrong
 */

/* max length of any packet */
#define IMC_PACKET_LENGTH 16300
/* max length of any mud name */
#define IMC_MNAME_LENGTH  20
/* max length of any player name */
#define IMC_PNAME_LENGTH  40
/* max length of any player@mud name */
#define IMC_NAME_LENGTH   (IMC_MNAME_LENGTH+IMC_PNAME_LENGTH+1)
/* max length of a path */
#define IMC_PATH_LENGTH   200
/* max length of a packet type */
#define IMC_TYPE_LENGTH   20
/* max length of a password */
#define IMC_PW_LENGTH     20
/* max length of a data type (estimate) */
#define IMC_DATA_LENGTH   (IMC_PACKET_LENGTH-2*IMC_NAME_LENGTH-IMC_PATH_LENGTH-IMC_TYPE_LENGTH-20)
/* max number of data keys in a packet */
#define IMC_MAX_KEYS      20
/* notify central server? */
#define IMC_NOTIFY

/****************************************************************************/

/* handle system-specific #defines */

/* our version of memmove (ugly!) */
#if NO_MEMMOVE
#define memmove(dest,src,size)              \
do {                                        \
  char *d=(char *)(dest), *s=(char *)(src); \
  int sz=(size);                            \
  if (d<s)                                  \
    for ( ; sz; --sz)                       \
      *d++=*s++;                            \
  else                                      \
    for (s+=sz,d+=sz; sz; --sz)             \
      *(--d)=*(--s);                        \
} while(0)
#endif

/* try strtol if we don't have strtoul */
#if NO_STRTOUL
#define strtoul(p,e,b) ((unsigned long)strtol((p),(e),(b)))
#endif

/* fake a strerror if we don't have it - ick */
#if NO_STRERROR
#define strerror(e) \
((e)==ECONNRESET   ? "Connection reset by peer" : \
 (e)==ENETUNREACH  ? "Network unreachable"      : \
 (e)==ETIMEDOUT    ? "Connection timed out"     : \
 (e)==ECONNREFUSED ? "Connection refused"       : \
 (e)==EHOSTUNREACH ? "No route to host"         : \
                     "Unknown error")
#endif

/* map vsnprintf to vsprintf if it isn't available (we lose some buffer
 * overflow protection in the logging fns, though)
 */

#if NO_VSNPRINTF
#define vsnprintf(buf, len, fmt, ap) vsprintf(buf, fmt, ap)
#endif

/* nuke __attribute__ if it's not supported */

#if NO_ATTRIBUTE
#define __attribute__(x) /*nothing*/
#endif

#endif
