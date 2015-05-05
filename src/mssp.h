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
     |                     -*- MSSP Plaintext Module -*-                   |
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

/* TERMS OF USE
         I only really have 2 terms...
 1. Give credit where it is due, keep the above header in your code 
    (you don't have to give me credit in mud) and if someone asks 
	don't lie and say you did it.
 2. If you have any comments or questions feel free to email me
    at keberus@gmail.com

  Thats All....
 */

struct mssp_info
{
   char *hostname;
   char *contact;
   char *icon;
   char *language;
   char *location;
   char *website;
   char *family;
   char *genre;
   char *gamePlay;
   char *gameSystem;
   char *intermud;
   char *status;
   char *subgenre;
   char *equipmentSystem;
   char *multiplaying;
   char *playerKilling;
   char *questSystem;
   char *roleplaying;
   char *trainingSystem;
   char *worldOriginality;
   short created;
   short minAge;
   short worlds;
   bool ansi;
   bool mccp;
   bool mcp;
   bool msp;
   bool ssl;
   bool mxp;
   bool pueblo;
   bool vt100;
   bool xterm256;
   bool pay2play;
   bool pay4perks;
   bool hiringBuilders;
   bool hiringCoders;
   bool adultMaterial;
   bool multiclassing;
   bool newbieFriendly;
   bool playerCities;
   bool playerClans;
   bool playerCrafting;
   bool playerGuilds;
};    

#define MSSP_MINAGE  0
#define MSSP_MAXAGE  21

#define MSSP_MINCREATED 1970
#define MSSP_MAXCREATED 2100

#define MSSP_MAXVAL 20000
#define MAX_MSSP_VAR1 4
#define MAX_MSSP_VAR2 3

bool load_mssp_data( void );
void send_mssp_data( DESCRIPTOR_DATA * d );
void free_mssp_info( void );

//GNUC_FORMAT macro was contrived by Elanthis
#ifdef __GNUC__
#  define GNUC_FORMAT(fmt,args) __attribute__ ((format (printf, fmt, args)))
#else
#  define GNUC_FORMAT(fmt,args)
#endif

void mssp_reply( DESCRIPTOR_DATA * d, const char *var, const char *fmt, ... ) GNUC_FORMAT( 3, 4 );
