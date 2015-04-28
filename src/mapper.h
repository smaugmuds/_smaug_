/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.8 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops, Fireblade, Edmond, Conran                         |             *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *                           Room Mapper Module                             *
 ****************************************************************************/

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
