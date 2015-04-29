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
     |                  -*- External DNS Resolver Module -*-               |
     |_____________________________________________________________________|
     |                                                                     |
     |                          SMC version 0.9.7b3                        |
     |           Additions to Rom2.3 (C) 1995, 1996 by Tom Adriaenssen     |
     |_____________________________________________________________________|
     |                                                                     |
     |  Share and enjoy! But please give the original authors some credit. |
     |_____________________________________________________________________|
     |                                                                     |
     |  Ideas, tips, or comments can be send to:                           |
     |           tadriaen@zorro.ruca.ua.ac.be                              |
     |           shadow@www.dma.be                                         |
     |_____________________________________________________________________|
     |                                                                     |
     |  This had an identd routine as well, but it's been removed since Smaug
     |  handles that in another file.
     |_____________________________________________________________________|
    //                                                                     \\
   [|  SMAUG 1.4 © 1994-1998 Thoric/Altrag/Blodkai/Narn/Haus/Scryn/Rennard  |]
   [|  Swordbearer/Gorog/Grishnakh/Nivek/Tricops/Fireblade/Edmond/Conran    |]
   [|                                                                       |]
   [|  Merc 2.1 Diku Mud improvments © 1992-1993 Michael Chastain, Michael  |]
   [|  Quan, and Mitchell Tse. Original Diku Mud © 1990-1991 by Sebastian   |]
   [|  Hammer, Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, Katja    |]
   [|  Nyboe. Win32 port Nick Gammon.                                       |]
   [|                                                                       |]
   [|  SMAUG 2.0 © 2014-2015 Antonio Cao (@burzumishi)                      |]
    \\_____________________________________________________________________//
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

#include "mud.h"

char *resolve_address( int address )
{
   static char addr_str[256];
   struct hostent *from;
   int addr;
    
   if( ( from = gethostbyaddr( (char*)&address, sizeof(address), AF_INET ) ) != NULL )
   {
      strcpy( addr_str, 
    	strcmp( from->h_name, "localhost" ) ? from->h_name : "local-host" );
   }
   else
   {
    	addr = ntohl( address );
    	snprintf( addr_str, 256, "%d.%d.%d.%d",
	    ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
	    ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF );
   } 
   return addr_str;
}

int main( int argc, char *argv[] )
{
   int ip;
   char *address;

		setlocale (LC_ALL, "");
		bindtextdomain (PACKAGE, LOCALEDIR);
		textdomain (PACKAGE);  
    
   if( argc != 2 )
   {
      printf( _("Unknown Hostname!\n") );
    	exit( 1 );
   }
    
   ip = atoi( argv[1] );
    
   address = resolve_address( ip );
    
   printf( "%s\r\n", address );
   exit( 0 );
}
