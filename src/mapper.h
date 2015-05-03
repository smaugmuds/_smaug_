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
     |                      -*- Room Mapper Module -*-                     |
     |_____________________________________________________________________|
     |                                                                     |
     |   Converted for AFKMud 1.64 by Zarius (jeff@mindcloud.com)          |
     |   Downloaded from http://www.mindcloud.com                          |
     |   If you like the snippet let me know                               |
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
   [|                                                                       |]
   [|  In order to use any part of this Merc Diku Mud, you must comply with |]
   [|  both the original Diku license in 'license.doc' as well the Merc     |]
   [|  license in 'license.txt'.  In particular, you may not remove either  |]
   [|  of these copyright notices.                                          |]
   [|                                                                       |]
   [|  Dystopia Mud improvements copyright (C) 2000-2001 by Brian Graversen |]
   [|                                                                       |]
   [|  Much time and thought has gone into this software and you are        |]
   [|  benefitting.  We hope that you share your changes too.  What goes    |]
   [|  around, comes around.                                                |]
    \\_____________________________________________________________________//
*/


/* Defines for ASCII Automapper */
#define MAPX     10
#define MAPY      8

/* You can change MAXDEPTH to 1 if the diagonal directions are confusing */
#define MAXDEPTH  2

#define BOUNDARY(x, y) ( ( (x) < 0) || ((y) < 0 ) || ( (x) > MAPX ) || ( (y) > MAPY ) )

/* Structure for the map itself */
typedef struct map_type MAP_TYPE;
struct map_type
{
//   EXT_BV info;
   char tegn;  /* Character to print at this map coord */
   int vnum;   /* Room this coord represents */
   int depth;  /* Recursive depth this coord was found at */
   int sector; /* Sector type of the map space */
   bool can_see;
};

void draw_room_map( CHAR_DATA * ch, const char *desc );
