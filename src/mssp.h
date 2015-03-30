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
     |               -*- MSSP Module Common definitions -*-                |
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

#ifndef _MSSP_H_
#define _MSSP_H_

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

#define MSSP_FILE SYSTEM_DIR "mssp.dat"

#define FOPEN( fp, rwx )    fopen( (fp), (rwx) )

#define MSSP_MINAGE  0
#define MSSP_MAXAGE  21

#define MSSP_MINCREATED 1970
#define MSSP_MAXCREATED 2100

#define MSSP_MAXVAL 20000
#define MAX_MSSP_VAR1 4
#define MAX_MSSP_VAR2 3

bool load_mssp_data( void );
void send_mssp_data( DESCRIPTOR_DATA * d );

//GNUC_FORMAT macro was contrived by Elanthis
#ifdef __GNUC__
#  define GNUC_FORMAT(fmt,args) __attribute__ ((format (printf, fmt, args)))
#else
#  define GNUC_FORMAT(fmt,args)
#endif

void mssp_reply( DESCRIPTOR_DATA * d, const char *var, const char *fmt, ... ) GNUC_FORMAT( 3, 4 );

#endif
