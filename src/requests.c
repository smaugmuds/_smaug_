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
     |                    -*- Special Requests Module -*-                  |
     |_____________________________________________________________________|
     |                                                                     |
     | Only handles who requests currently, but will hopefully support     |
     | much more in the future. Including: reboot/shutdown etc.		         |
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

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mud.h"

int REQ;

void init_request_pipe( )
{
#ifdef REQUESTS
  if ( (REQ = open( REQUEST_PIPE, O_RDONLY | O_NONBLOCK )) == -1 )
  {
    bug ( "REQUEST pipe not found", 0 );
    exit(1);
  }
#endif
}

void check_requests( )
{
#ifdef REQUESTS
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int  c;
  if ( read( REQ, buf, sizeof( buf ) ) > 0 )
  {
     close( REQ );
     init_request_pipe();
     for ( c = 0; c < MAX_STRING_LENGTH; c++ )
	if ( buf[c] == '\n' || buf[c] == '\r' )
	{
	   buf[c] = '\0';
	   break;
	}
     sprintf( buf2, "REQUEST: %s", buf );
     log_string( buf2 );
     if ( strcmp( buf, "who" ) == 0 )
	do_who( NULL, "" );
     else
     if ( strcmp( buf, "webwho" ) == 0 )
	do_who( NULL, "www" );
  }
#endif
}
