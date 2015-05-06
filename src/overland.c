/****************************************************************************
 *                   ^     +----- |  / ^     ^ |     | +-\                  *
 *                  / \    |      | /  |\   /| |     | |  \                 *
 *                 /   \   +---   |<   | \ / | |     | |  |                 *
 *                /-----\  |      | \  |  v  | |     | |  /                 *
 *               /       \ |      |  \ |     | +-----+ +-/                  *
 ****************************************************************************
 * AFKMud Copyright 1997-2003 by Roger Libiez (Samson),                     *
 * Levi Beckerson (Whir), Michael Ward (Tarl), Erik Wolfe (Dwip),           *
 * Cameron Carroll (Cam), Cyberfox, Karangi, Rathian, Raine, and Adjani.    *
 * All Rights Reserved.                                                     *
 *                                                                          *
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag,        *
 * Blodkai, Haus, Narn, Scryn, Swordbearer, Tricops, Gorog, Rennard,        *
 * Grishnakh, Fireblade, and Nivek.                                         *
 *                                                                          *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                      *
 *                                                                          *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen,      *
 * Michael Seifert, and Sebastian Hammer.                                   *
 ****************************************************************************
 *	                     Overland ANSI Map Module                         *
 *			       Created by Samson of Alsherok                      *
 ****************************************************************************/

#include <string.h>
#include <math.h>
#include "mud.h"

/* Gee, I got bored and added a bunch of new comments to things. Interested parties know what to look for. 
 * Reorganized the placement of many things so that they're grouped together better.
 */

ENTRANCE_DATA *first_entrance;
ENTRANCE_DATA *last_entrance;
LANDMARK_DATA *first_landmark;
LANDMARK_DATA *last_landmark;
MAPRESET_DATA *first_mapreset;
MAPRESET_DATA *last_mapreset;

unsigned char map_sector[MAP_MAX][MAX_X][MAX_Y];   /* Initializes the sector array */

extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];

void shutdown_mud( char *reason );

#ifdef DRAGONFLIGHT
void load_landing_sites( void );
LANDING_DATA *check_landing_site( int map, int x, int y );
#endif

char *const map_filenames[] = {
   "map1.raw", "map2.raw", "map3.raw"
};

char *const map_names[] = {
   "Map 1", "Map 2", "Map 3"
};

char *const map_name[] = {
   "map1", "map2", "map3"
};

char *const continents[] = {
   "continent1", "continent2", "continent3"
};

/* The names of varous sector types. Used in the OLC code in addition to
 * the display_map function and probably some other places I've long
 * since forgotten by now.
 */
char *const sect_types[] = {
   "indoors", "city", "field", "forest", "hills", "mountain", "water_swim",
   "water_noswim", "air", "underwater", "desert", "river", "oceanfloor",
   "underground", "jungle", "swamp", "tundra", "ice", "ocean", "lava",
   "shore", "tree", "stone", "quicksand", "wall", "glacier", "exit",
   "trail", "blands", "grassland", "scrub", "barren", "bridge", "road",
#ifdef DRAGONFLIGHT
   "landing",
#endif
   "\n"
};

/* Note - this message array is used to broadcast both the in sector messages,
 *  as well as the messages sent to PCs when they can't move into the sector.
 */
char *const impass_message[SECT_MAX] = {
   "You must locate the proper entrance to go in there.",
   "You are travelling along a smooth stretch of road.",
   "Rich farmland stretches out before you.",
   "Thick forest vegetation covers the ground all around.",
   "Gentle rolling hills stretch out all around.",
   "The rugged terrain of the mountains makes movement slow.",
   "The waters lap at your feet.",
   "The deep waters lap at your feet.",
   "Air",
   "Underwater",
   "The hot, dry desert sands seem to go on forever.",
   "The river churns and burbles beneath you.",
   "Oceanfloor",
   "Underground",
   "The jungle is extremely thick and humid.",
   "The swamps seem to surround everything.",
   "The frozen wastes seem to stretch on forever.",
   "The ice barely provides a stable footing.",
   "The rough seas would rip any boat to pieces!",
   "That's lava! You'd be burnt to a crisp!!!",
   "The soft sand makes for difficult walking.",
   "The forest becomes too thick to pass through that direction.",
   "The mountains are far too steep to keep going that way.",
   "That's quicksand! You'd be dragged under!",
   "The walls are far too high to scale.",
   "The glacier ahead is far too vast to safely cross.",
   "An exit to somewhere new.....",
   "You are walking along a dusty trail.",
   "All around you the land has been scorched to ashes.",
   "Tall grass ripples in the wind.",
   "Scrub land stretches out as far as the eye can see.",
   "The land around you is dry and barren.",
   "A sturdy span of bridge passes over the water.",
   "You are travelling along a smooth stretch of road."
#ifdef DRAGONFLIGHT
      , "Landing"
#endif
};

/* The symbol table. 
 * First entry is the sector type. 
 * Second entry is the color used to display it.
 * Third entry is the symbol used to represent it.
 * Fourth entry is the description of the terrain.
 * Fifth entry determines weather or not the terrain can be walked on.
 * Sixth entry is the amount of movement used up travelling on the terrain.
 * Last 3 entries are the RGB values for each type of terrain used to generate the graphic files.
 */
const struct sect_color_type sect_show[] = {
/*   Sector Type		Color	Symbol Description	Passable?	Move  R  G  B	*/

   {SECT_INSIDE, "&x", " ", "indoors", FALSE, 1, 0, 0, 0},
   {SECT_CITY, "&Y", ":", "city", TRUE, 1, 255, 128, 64},
   {SECT_FIELD, "&G", "+", "field", TRUE, 1, 141, 215, 1},
   {SECT_FOREST, "&g", "+", "forest", TRUE, 2, 0, 108, 47},
   {SECT_HILLS, "&O", "^", "hills", TRUE, 3, 140, 102, 54},
   {SECT_MOUNTAIN, "&w", "^", "mountain", TRUE, 5, 152, 152, 152},
   {SECT_WATER_SWIM, "&C", "~", "shallow water", TRUE, 2, 89, 242, 251},
   {SECT_WATER_NOSWIM, "&B", "~", "deep water", TRUE, 2, 67, 114, 251},
   {SECT_AIR, "&x", "?", "air", FALSE, 1, 0, 0, 0},
   {SECT_UNDERWATER, "&x", "?", "underwater", FALSE, 5, 0, 0, 0},
   {SECT_DESERT, "&Y", "~", "desert", TRUE, 3, 241, 228, 145},
   {SECT_RIVER, "&B", "~", "river", TRUE, 3, 0, 0, 255},
   {SECT_OCEANFLOOR, "&x", "?", "ocean floor", FALSE, 4, 0, 0, 0},
   {SECT_UNDERGROUND, "&x", "?", "underground", FALSE, 3, 0, 0, 0},
   {SECT_JUNGLE, "&g", "*", "jungle", TRUE, 2, 70, 149, 52},
   {SECT_SWAMP, "&g", "~", "swamp", TRUE, 3, 218, 176, 56},
   {SECT_TUNDRA, "&C", "-", "tundra", TRUE, 2, 54, 255, 255},
   {SECT_ICE, "&W", "=", "ice", TRUE, 3, 133, 177, 252},
   {SECT_OCEAN, "&b", "~", "ocean", FALSE, 1, 0, 0, 128},
   {SECT_LAVA, "&R", ":", "lava", FALSE, 2, 245, 37, 29},
   {SECT_SHORE, "&Y", ".", "shoreline", TRUE, 3, 255, 255, 0},
   {SECT_TREE, "&g", "^", "impass forest", FALSE, 10, 0, 64, 0},
   {SECT_STONE, "&W", "^", "impas mountain", FALSE, 10, 128, 128, 128},
   {SECT_QUICKSAND, "&g", "%", "quicksand", FALSE, 10, 128, 128, 0},
   {SECT_WALL, "&P", "I", "wall", FALSE, 10, 255, 0, 255},
   {SECT_GLACIER, "&W", "=", "glacier", FALSE, 10, 141, 207, 244},
   {SECT_EXIT, "&W", "#", "exit", TRUE, 1, 255, 255, 255},
   {SECT_TRAIL, "&O", ":", "trail", TRUE, 1, 128, 64, 0},
   {SECT_BLANDS, "&r", ".", "blasted lands", TRUE, 2, 128, 0, 0},
   {SECT_GRASSLAND, "&G", ".", "grassland", TRUE, 1, 83, 202, 2},
   {SECT_SCRUB, "&g", ".", "scrub", TRUE, 2, 123, 197, 112},
   {SECT_BARREN, "&O", ".", "barren", TRUE, 2, 192, 192, 192},
   {SECT_BRIDGE, "&P", ":", "bridge", TRUE, 1, 255, 0, 128},
#ifdef DRAGONFLIGHT
   {SECT_ROAD, "&Y", ":", "road", TRUE, 1, 215, 107, 0},
   {SECT_LANDING, "&R", "#", "landing", TRUE, 1, 255, 0, 0}
#else
   {SECT_ROAD, "&Y", ":", "road", TRUE, 1, 215, 107, 0}
#endif
};

/* The distance messages for the survey command */
char *landmark_distances[] = {
   "hundreds of miles away in the distance",
   "far off in the skyline",
   "many miles away at great distance",
   "far off many miles away",
   "tens of miles away in the distance",
   "far off in the distance",
   "several miles away",
   "off in the distance",
   "not far from here",
   "in the near vicinity",
   "in the immediate area"
};

/* The array of predefined mobs that the check_random_mobs function uses.
 * Thanks to Geni for supplying this method :)
 */
int const random_mobs[SECT_MAX][25] = {
   /*
    * Mobs for SECT_INSIDE 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_CITY 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    11030, 11031, 11032, 11033, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_FIELD 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    11034, 11035, 11036, 11037, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_FOREST 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    11038, 11039, 11040, 11041, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_HILLS 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    11042, 11043, 11044, 11045, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_MOUNTAIN 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    11046, 11047, 4900, 11048, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_WATER_SWIM 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_WATER_NOSWIM 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_AIR 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_UNDERWATER 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_DESERT 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    11049, 11050, 4654, 11051, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_RIVER 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_OCEANFLOOR 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_UNDERGROUND 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_JUNGLE 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    11054, 11055, 11056, 11057, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_SWAMP 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    11052, 11053, 25111, 5393, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_TUNDRA 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_ICE 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_OCEAN 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_LAVA 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_SHORE 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    11058, 11059, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_TREE 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_STONE 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_QUICKSAND 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_WALL 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_GLACIER 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_EXIT 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_TRAIL 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_BLANDS 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_GRASSLAND 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_SCRUB 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    11060, 11061, 11062, 11063, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_BARREN 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_BRIDGE 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1},
   /*
    * Mobs for SECT_ROAD 
    */
   {-1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1}
};

/* Simply changes the sector type for the specified coordinates */
void putterr( short map, short x, short y, short terr )
{
   map_sector[map][x][y] = terr;
   return;
}

/* Alrighty - this checks where the PC is currently standing to see what kind of terrain the space is.
 * Returns -1 if something is not kosher with where they're standing.
 * Called from several places below, so leave this right here.
 */
short get_terrain( short map, short x, short y )
{
   short terrain;

   if( map == -1 )
      return -1;

   if( x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y )
      return -1;

   terrain = map_sector[map][x][y];

   return terrain;
}

/* Used with the survey command to calculate distance to the landmark.
 * Used by do_scan to see if the target is close enough to justify showing it.
 * Used by the display_map code to create a more "circular" display.
 *
 * Other possible uses: 
 * Summon spell - restrict the distance someone can summon another person from.
 */
double distance( short chX, short chY, short lmX, short lmY )
{
   double xchange, ychange;
   double zdistance;

   xchange = ( chX - lmX );
   xchange *= xchange;
   /*
    * To make the display more circular - Matarael 
    */
   xchange *= ( 5.120000 / 10.780000 );   /* The font ratio. */
   ychange = ( chY - lmY );
   ychange *= ychange;

   zdistance = sqrt( ( xchange + ychange ) );
   return ( zdistance );
}

/* Used by the survey command to determine the directional message to send */
double calc_angle( short chX, short chY, short lmX, short lmY, double *ipDistan )
{
   int iNx1 = 0, iNy1 = 0, iNx2, iNy2, iNx3, iNy3;
   double dDist1, dDist2;
   double dTandeg, dDeg, iFinal;
   iNx2 = lmX - chX;
   iNy2 = lmY - chY;
   iNx3 = 0;
   iNy3 = iNy2;

   *ipDistan = ( int )distance( iNx1, iNy1, iNx2, iNy2 );

   if( iNx2 == 0 && iNy2 == 0 )
      return ( -1 );
   if( iNx2 == 0 && iNy2 > 0 )
      return ( 180 );
   if( iNx2 == 0 && iNy2 < 0 )
      return ( 0 );
   if( iNy2 == 0 && iNx2 > 0 )
      return ( 90 );
   if( iNy2 == 0 && iNx2 < 0 )
      return ( 270 );

   /*
    * ADJACENT 
    */
   dDist1 = distance( iNx1, iNy1, iNx3, iNy3 );

   /*
    * OPPOSSITE 
    */
   dDist2 = distance( iNx3, iNy3, iNx2, iNy2 );

   dTandeg = dDist2 / dDist1;
   dDeg = atan( dTandeg );

   iFinal = ( dDeg * 180 ) / 3.14159265358979323846;  /* Pi for the math impared :P */

   if( iNx2 > 0 && iNy2 > 0 )
      return ( ( 90 + ( 90 - iFinal ) ) );

   if( iNx2 > 0 && iNy2 < 0 )
      return ( iFinal );

   if( iNx2 < 0 && iNy2 > 0 )
      return ( ( 180 + iFinal ) );

   if( iNx2 < 0 && iNy2 < 0 )
      return ( ( 270 + ( 90 - iFinal ) ) );

   return ( -1 );

}

/* Will return true or false if ch and victim are in the same map room
 * Used in a LOT of places for checks. Does not need to verify onmap flags.
 */
bool is_same_map( CHAR_DATA * ch, CHAR_DATA * victim )
{
   if( victim->map != ch->map || victim->x != ch->x || victim->y != ch->y )
      return FALSE;

   return TRUE;
}

/* Will set the victims map the same as the characters map
 * I got tired of coding the same stuff... over.. and over...
 *
 * Used in summon, gate, goto
 *
 * Fraktyl
 */
/* Sets victim to whatever conditions ch is under */
void fix_maps( CHAR_DATA * ch, CHAR_DATA * victim )
{
   /*
    * Null ch is an acceptable condition, don't do anything. 
    */
   if( !ch )
      return;

   /*
    * This would be bad though, bug out. 
    */
   if( !victim )
   {
      bug( "%s", "fix_maps: NULL victim!" );
      return;
   }

   /*
    * Fix Act/Plr flags first 
    */
   if( IS_PLR_FLAG( ch, PLR_ONMAP ) || IS_ACT_FLAG( ch, ACT_ONMAP ) )
   {
      if( IS_NPC( victim ) )
         SET_ACT_FLAG( victim, ACT_ONMAP );
      else
         SET_PLR_FLAG( victim, PLR_ONMAP );
   }
   else
   {
      if( IS_NPC( victim ) )
         REMOVE_ACT_FLAG( victim, ACT_ONMAP );
      else
         REMOVE_ACT_FLAG( victim, PLR_ONMAP );
   }

   /*
    * Either way, the map will be the same 
    */
   victim->map = ch->map;
   victim->x = ch->x;
   victim->y = ch->y;

   return;
}

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				      field = value;			\
				      fMatch = TRUE;			\
				      break;				\
				}

/* Overland landmark stuff starts here */
void fread_landmark( LANDMARK_DATA * landmark, FILE * fp )
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
               return;
            break;

         case 'C':
            if( !str_cmp( word, "Coordinates" ) )
            {
               landmark->map = fread_number( fp );
               landmark->x = fread_number( fp );
               landmark->y = fread_number( fp );
               landmark->distance = fread_number( fp );
               fMatch = TRUE;
               break;
            }
            break;

         case 'D':
            KEY( "Description", landmark->description, fread_string( fp ) );
            break;

         case 'I':
            KEY( "Isdesc", landmark->Isdesc, fread_number( fp ) );
            break;

            if( !fMatch )
               bug( "Fread_landmark: no match: %s", word );
      }
   }
}

void load_landmarks( void )
{
   char filename[256];
   LANDMARK_DATA *landmark;
   FILE *fp;

   first_landmark = NULL;
   last_landmark = NULL;

   snprintf( filename, 256, "%s%s", MAP_DIR, LANDMARK_FILE );

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
            bug( "%s", "Load_landmarks: # not found." );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "LANDMARK" ) )
         {
            CREATE( landmark, LANDMARK_DATA, 1 );
            fread_landmark( landmark, fp );
            LINK( landmark, first_landmark, last_landmark, next, prev );
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "Load_landmarks: bad section: %s.", word );
            continue;
         }
      }
      fclose( fp );
      fp = NULL;
   }

   return;
}

void save_landmarks( void )
{
   LANDMARK_DATA *landmark;
   FILE *fp;
   char filename[256];

   snprintf( filename, 256, "%s%s", MAP_DIR, LANDMARK_FILE );

   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "%s", "save_landmarks: fopen" );
      perror( filename );
   }
   else
   {
      for( landmark = first_landmark; landmark; landmark = landmark->next )
      {
         fprintf( fp, "%s", "#LANDMARK\n" );
         fprintf( fp, "Coordinates	%d %d %d %d\n", landmark->map, landmark->x, landmark->y, landmark->distance );
         fprintf( fp, "Description	%s~\n", landmark->description );
         fprintf( fp, "Isdesc		%d\n", landmark->Isdesc );
         fprintf( fp, "%s", "End\n\n" );
      }
      fprintf( fp, "%s", "#END\n" );
      fclose( fp );
      fp = NULL;
   }
   return;
}

LANDMARK_DATA *check_landmark( short map, short x, short y )
{
   LANDMARK_DATA *landmark;

   for( landmark = first_landmark; landmark; landmark = landmark->next )
   {
      if( landmark->map == map )
      {
         if( landmark->x == x && landmark->y == y )
            return landmark;
      }
   }
   return NULL;
}

void add_landmark( short map, short x, short y )
{
   LANDMARK_DATA *landmark;

   CREATE( landmark, LANDMARK_DATA, 1 );
   LINK( landmark, first_landmark, last_landmark, next, prev );
   landmark->map = map;
   landmark->x = x;
   landmark->y = y;
   landmark->distance = 0;
   landmark->description = STRALLOC( "" );

   save_landmarks(  );

   return;
}

void delete_landmark( LANDMARK_DATA * landmark )
{
   if( !landmark )
   {
      bug( "%s", "delete_landmark: Trying to delete NULL landmark!" );
      return;
   }

   UNLINK( landmark, first_landmark, last_landmark, next, prev );
   STRFREE( landmark->description );
   DISPOSE( landmark );

   save_landmarks(  );

   return;
}

/* Landmark survey module - idea snarfed from Medievia and adapted to Smaug by Samson - 8-19-00 */
void do_survey( CHAR_DATA * ch, char *argument )
{
   LANDMARK_DATA *landmark;
   double dist, angle;
   int dir = -1, iMes = 0;
   bool found = FALSE;

   if( !ch )
      return;

   for( landmark = first_landmark; landmark; landmark = landmark->next )
   {
      /*
       * No point in bothering if its not even on this map 
       */
      if( ch->map != landmark->map )
         continue;

      if( landmark->Isdesc )
         continue;

      dist = distance( ch->x, ch->y, landmark->x, landmark->y );

      /*
       * Save the math if it's too far away anyway 
       */
      if( dist <= landmark->distance )
      {
         found = TRUE;

         angle = calc_angle( ch->x, ch->y, landmark->x, landmark->y, &dist );

         if( angle == -1 )
            dir = -1;
         else if( angle >= 360 )
            dir = DIR_NORTH;
         else if( angle >= 315 )
            dir = DIR_NORTHWEST;
         else if( angle >= 270 )
            dir = DIR_WEST;
         else if( angle >= 225 )
            dir = DIR_SOUTHWEST;
         else if( angle >= 180 )
            dir = DIR_SOUTH;
         else if( angle >= 135 )
            dir = DIR_SOUTHEAST;
         else if( angle >= 90 )
            dir = DIR_EAST;
         else if( angle >= 45 )
            dir = DIR_NORTHEAST;
         else if( angle >= 0 )
            dir = DIR_NORTH;

         if( dist > 200 )
            iMes = 0;
         else if( dist > 150 )
            iMes = 1;
         else if( dist > 100 )
            iMes = 2;
         else if( dist > 75 )
            iMes = 3;
         else if( dist > 50 )
            iMes = 4;
         else if( dist > 25 )
            iMes = 5;
         else if( dist > 15 )
            iMes = 6;
         else if( dist > 10 )
            iMes = 7;
         else if( dist > 5 )
            iMes = 8;
         else if( dist > 1 )
            iMes = 9;
         else
            iMes = 10;

         if( dir == -1 )
            ch_printf( ch, "Right here nearby, %s.\r\n",
                       landmark->description ? landmark->description : "BUG! Please report!" );
         else
            ch_printf( ch, "To the %s, %s, %s.\r\n", dir_name[dir], landmark_distances[iMes],
                       landmark->description ? landmark->description : "<BUG! Inform the Immortals>" );

         if( IS_IMMORTAL( ch ) )
         {
            ch_printf( ch, "Distance to landmark: %d\r\n", dist );
            ch_printf( ch, "Landmark coordinates: %dX %dY\r\n", landmark->x, landmark->y );
         }
      }
   }
   if( !found )
      send_to_char( "Your survey of the area yields nothing special.\r\n", ch );

   return;
}

/* Support command to list all landmarks currently loaded */
void do_landmarks( CHAR_DATA * ch, char *argument )
{
   LANDMARK_DATA *landmark;

   if( !first_landmark )
   {
      send_to_char( "No landmarks defined.\r\n", ch );
      return;
   }

   send_to_pager( "Continent | Coordinates | Distance | Description\r\n", ch );
   send_to_pager( "-----------------------------------------------------------\r\n", ch );

   for( landmark = first_landmark; landmark; landmark = landmark->next )
   {
      pager_printf( ch, "%-10s  %-4dX %-4dY   %-4d       %s\r\n",
                    map_names[landmark->map], landmark->x, landmark->y, landmark->distance, landmark->description );
   }
   return;
}

/* OLC command to add/delete/edit landmark information */
void do_setmark( CHAR_DATA * ch, char *argument )
{
   LANDMARK_DATA *landmark = NULL;
   char arg[MAX_INPUT_LENGTH];

   if( IS_NPC( ch ) )
   {
      send_to_char( "Sorry, NPCs can't edit the overland maps.\r\n", ch );
      return;
   }

   if( !ch->desc )
   {
      send_to_char( "You have no descriptor.\r\n", ch );
      return;
   }

   switch ( ch->substate )
   {
      default:
         break;

      case SUB_RESTRICTED:
         send_to_char( "You cannot do this while in another command.\r\n", ch );
         return;

      case SUB_OVERLAND_DESC:
         landmark = ( LANDMARK_DATA * ) ch->dest_buf;
         if( !landmark )
            bug( "%s", "setmark desc: sub_overland_desc: NULL ch->pcdata->dest_buf" );
         if( landmark->description )
            STRFREE( landmark->description );
         landmark->description = copy_buffer( ch );
         stop_editing( ch );
         ch->substate = ch->tempnum;
         save_landmarks(  );
         send_to_char( "Description set.\r\n", ch );
         return;
   }

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' || !str_cmp( arg, "help" ) )
   {
      send_to_char( "Usage: setmark add\r\n", ch );
      send_to_char( "Usage: setmark delete\r\n", ch );
      send_to_char( "Usage: setmark distance <value>\r\n", ch );
      send_to_char( "Usage: setmark desc\r\n", ch );
      send_to_char( "Usage: setmark isdesc\r\n", ch );
      return;
   }

   landmark = check_landmark( ch->map, ch->x, ch->y );

   if( !str_cmp( arg, "add" ) )
   {
      if( landmark )
      {
         send_to_char( "There's already a landmark at this location.\r\n", ch );
         return;
      }
      add_landmark( ch->map, ch->x, ch->y );
      send_to_char( "Landmark added.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "delete" ) )
   {
      if( !landmark )
      {
         send_to_char( "There is no landmark here.\r\n", ch );
         return;
      }
      delete_landmark( landmark );
      send_to_char( "Landmark deleted.\r\n", ch );
      return;
   }

   if( !landmark )
   {
      send_to_char( "There is no landmark here.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "isdesc" ) )
   {
      landmark->Isdesc = !landmark->Isdesc;
      save_landmarks(  );

      if( landmark->Isdesc )
         send_to_char( "Landmark is now a room description.\r\n", ch );
      else
         send_to_char( "Room description is now a landmark.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "distance" ) )
   {
      int value;

      if( !is_number( argument ) )
      {
         send_to_char( "Distance must be a numeric amount.\r\n", ch );
         return;
      }

      value = atoi( argument );

      if( value < 1 )
      {
         send_to_char( "Distance must be at least 1.\r\n", ch );
         return;
      }
      landmark->distance = value;
      save_landmarks(  );
      send_to_char( "Visibility distance set.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "desc" ) || !str_cmp( arg, "description" ) )
   {
      if( ch->substate == SUB_REPEATCMD )
         ch->tempnum = SUB_REPEATCMD;
      else
         ch->tempnum = SUB_NONE;
      ch->substate = SUB_OVERLAND_DESC;
      ch->dest_buf = landmark;
      start_editing( ch, landmark->description );
      return;
   }

   do_setmark( ch, "" );
   return;
}

/* Overland landmark stuff ends here */

/* Overland exit stuff starts here */
void fread_entrance( ENTRANCE_DATA * enter, FILE * fp )
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

         case 'A':
            KEY( "Area", enter->area, fread_string( fp ) );
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               return;
            }
            break;

         case 'H':
            if( !str_cmp( word, "Here" ) )
            {
               enter->herex = fread_number( fp );
               enter->herey = fread_number( fp );
               fMatch = TRUE;
               break;
            }
            break;

         case 'O':
            KEY( "OnMap", enter->onmap, fread_number( fp ) );
            break;

         case 'P':
            KEY( "Prevsector", enter->prevsector, fread_number( fp ) );
            break;

         case 'T':
            if( !str_cmp( word, "There" ) )
            {
               enter->therex = fread_number( fp );
               enter->therey = fread_number( fp );
               fMatch = TRUE;
               break;
            }
            KEY( "ToMap", enter->tomap, fread_number( fp ) );
            break;

         case 'V':
            KEY( "Vnum", enter->vnum, fread_number( fp ) );
            break;
      }

      if( !fMatch )
         bug( "Fread_entrance: no match: %s", word );
   }
}

void load_entrances( void )
{
   char filename[256];
   ENTRANCE_DATA *enter;
   FILE *fp;

   first_entrance = NULL;
   last_entrance = NULL;

   snprintf( filename, 256, "%s%s", MAP_DIR, ENTRANCE_FILE );

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
            bug( "%s", "Load_entrances: # not found." );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "ENTRANCE" ) )
         {
            CREATE( enter, ENTRANCE_DATA, 1 );
            enter->prevsector = SECT_OCEAN;  /* Default for legacy exits */
            fread_entrance( enter, fp );
            LINK( enter, first_entrance, last_entrance, next, prev );
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "Load_entrances: bad section: %s.", word );
            continue;
         }
      }
      fclose( fp );
      fp = NULL;
   }
   return;
}

void save_entrances( void )
{
   ENTRANCE_DATA *enter;
   FILE *fp;
   char filename[256];

   snprintf( filename, 256, "%s%s", MAP_DIR, ENTRANCE_FILE );

   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "%s", "save_entrances: fopen" );
      perror( filename );
   }
   else
   {
      for( enter = first_entrance; enter; enter = enter->next )
      {
         fprintf( fp, "%s", "#ENTRANCE\n" );
         fprintf( fp, "ToMap	%d\n", enter->tomap );
         fprintf( fp, "OnMap	%d\n", enter->onmap );
         fprintf( fp, "Here	%d %d\n", enter->herex, enter->herey );
         fprintf( fp, "There	%d %d\n", enter->therex, enter->therey );
         fprintf( fp, "Vnum	%d\n", enter->vnum );
         fprintf( fp, "Prevsector %d\n", enter->prevsector );
         fprintf( fp, "Area	%s~\n", enter->area );
         fprintf( fp, "%s", "End\n\n" );
      }
      fprintf( fp, "%s", "#END\n" );
      fclose( fp );
      fp = NULL;
   }
   return;
}

ENTRANCE_DATA *check_entrance( short map, short x, short y )
{
   ENTRANCE_DATA *enter;

   for( enter = first_entrance; enter; enter = enter->next )
   {
      if( enter->onmap == map )
      {
         if( enter->herex == x && enter->herey == y )
            return enter;
      }
   }
   return NULL;
}

void modify_entrance( ENTRANCE_DATA * enter, short tomap, short onmap, short hereX, short hereY, short thereX, short thereY,
                      int vnum, char *area )
{
   if( !enter )
   {
      bug( "%s", "modify_entrance: NULL exit being modified!" );
      return;
   }

   enter->tomap = tomap;
   enter->onmap = onmap;
   enter->herex = hereX;
   enter->herey = hereY;
   enter->therex = thereX;
   enter->therey = thereY;
   enter->vnum = vnum;
   if( area )
   {
      STRFREE( enter->area );
      enter->area = STRALLOC( area );
   }

   save_entrances(  );

   return;
}

void add_entrance( short tomap, short onmap, short hereX, short hereY, short thereX, short thereY, int vnum )
{
   ENTRANCE_DATA *enter;

   CREATE( enter, ENTRANCE_DATA, 1 );
   LINK( enter, first_entrance, last_entrance, next, prev );
   enter->tomap = tomap;
   enter->onmap = onmap;
   enter->herex = hereX;
   enter->herey = hereY;
   enter->therex = thereX;
   enter->therey = thereY;
   enter->vnum = vnum;
   enter->prevsector = get_terrain( onmap, hereX, hereY );
   enter->area = STRALLOC( "" );

   save_entrances(  );

   return;
}

void delete_entrance( ENTRANCE_DATA * enter )
{
   if( !enter )
   {
      bug( "%s", "delete_entrance: Trying to delete NULL exit!" );
      return;
   }

   STRFREE( enter->area );
   UNLINK( enter, first_entrance, last_entrance, next, prev );
   DISPOSE( enter );

   save_entrances(  );

   return;
}

/* OLC command to add/delete/edit overland exit information */
void do_setexit( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   ENTRANCE_DATA *enter = NULL;
   int vnum;

   if( IS_NPC( ch ) )
   {
      send_to_char( "Sorry, NPCs can't edit the overland maps.\r\n", ch );
      return;
   }

   if( !IS_PLR_FLAG( ch, PLR_ONMAP ) )
   {
      send_to_char( "This command can only be used from an overland map.\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' || !str_cmp( arg, "help" ) )
   {
      send_to_char( "Usage: setexit create\r\n", ch );
      send_to_char( "Usage: setexit delete\r\n", ch );
      send_to_char( "Usage: setexit <vnum>\r\n", ch );
      send_to_char( "Usage: setexit <area>\r\n", ch );
      send_to_char( "Usage: setexit map <mapname> <X-coord> <Y-coord>\r\n", ch );
      return;
   }

   enter = check_entrance( ch->map, ch->x, ch->y );

   if( !str_cmp( arg, "create" ) )
   {
      if( enter )
      {
         send_to_char( "An exit already exists at these coordinates.\r\n", ch );
         return;
      }

      add_entrance( ch->map, ch->map, ch->x, ch->y, ch->x, ch->y, -1 );
      putterr( ch->map, ch->x, ch->y, SECT_EXIT );
      send_to_char( "New exit created.\r\n", ch );
      return;
   }

   if( !enter )
   {
      send_to_char( "No exit exists at these coordinates.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "delete" ) )
   {
      putterr( ch->map, ch->x, ch->y, enter->prevsector );
      delete_entrance( enter );
      send_to_char( "Exit deleted.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "map" ) )
   {
      char arg2[MAX_INPUT_LENGTH];
      char arg3[MAX_INPUT_LENGTH];
      short x, y, map = -1;

      if( ch->map == -1 )
      {
         bug( "do_setexit: %s is not on a valid map!", ch->name );
         send_to_char( "Can't do that - your on an invalid map.\r\n", ch );
         return;
      }

      argument = one_argument( argument, arg2 );

      if( arg2[0] == '\0' )
      {
         do_setexit( ch, "" );
         return;
      }

      argument = one_argument( argument, arg3 );

      if( arg2[0] == '\0' )
      {
         send_to_char( "Make an exit to what map??\r\n", ch );
         return;
      }

      if( !str_cmp( arg2, "map1" ) )
         map = ACON_C1;

      if( !str_cmp( arg2, "map2" ) )
         map = ACON_C2;

      if( !str_cmp( arg2, "map3" ) )
         map = ACON_C3;

      if( map == -1 )
      {
         ch_printf( ch, "There isn't a map for '%s'.\r\n", arg2 );
         return;
      }

      x = atoi( arg3 );
      y = atoi( argument );

      if( x < 0 || x >= MAX_X )
      {
         ch_printf( ch, "Valid x coordinates are 0 to %d.\r\n", MAX_X - 1 );
         return;
      }

      if( y < 0 || y >= MAX_Y )
      {
         ch_printf( ch, "Valid y coordinates are 0 to %d.\r\n", MAX_Y - 1 );
         return;
      }

      modify_entrance( enter, map, ch->map, ch->x, ch->y, x, y, -1, NULL );
      putterr( ch->map, ch->x, ch->y, SECT_EXIT );
      ch_printf( ch, "Exit set to map of %s, at %dX, %dY.\r\n", arg2, x, y );
      return;
   }

   if( !str_cmp( arg, "area" ) )
   {
      if( argument[0] == '\0' )
      {
         do_setexit( ch, "" );
         return;
      }

      modify_entrance( enter, enter->onmap, ch->map, ch->x, ch->y, enter->therex, enter->therey, enter->vnum, argument );
      ch_printf( ch, "Exit identified for area: %s\r\n", argument );
      return;
   }

   vnum = atoi( arg );

   if( ( location = get_room_index( vnum ) ) == NULL )
   {
      send_to_char( "No such room exists.\r\n", ch );
      return;
   }

   modify_entrance( enter, -1, ch->map, ch->x, ch->y, -1, -1, vnum, NULL );
   putterr( ch->map, ch->x, ch->y, SECT_EXIT );
   ch_printf( ch, "Exit set to room %d.\r\n", vnum );
   return;
}

/* Overland exit stuff ends here */

/* Overland reset stuff starts here */
void mapreset( void )
{
   MAPRESET_DATA *reset;
   ROOM_INDEX_DATA *toroom;
   OBJ_INDEX_DATA *objindex;
   MOB_INDEX_DATA *mobindex;
   OBJ_DATA *obj;
   CHAR_DATA *mob;

   if( !first_mapreset )
      return;

   for( reset = first_mapreset; reset; reset = reset->next )
   {
      toroom = NULL;
      objindex = NULL;
      mobindex = NULL;

      switch ( reset->map )
      {
         case MAP_C1:
            toroom = get_room_index( OVERLAND_MAP1 );
            break;
         case MAP_C2:
            toroom = get_room_index( OVERLAND_MAP2 );
            break;
         case MAP_C3:
            toroom = get_room_index( OVERLAND_MAP3 );
         default:
            bug( "mapreset: Bad overland map in resets: %d", reset->map );
            continue;
      }

      if( !toroom )
      {
         bug( "%s", "mapreset: Room index not found!" );
         continue;
      }

      if( reset->type == TYPE_OBJECT )
         objindex = get_obj_index( reset->vnum );

      if( reset->type == TYPE_MOBILE )
         mobindex = get_mob_index( reset->vnum );

      if( !objindex && !mobindex )
      {
         bug( "%s", "mapreset: Bad map reset!" );
         continue;
      }

      if( objindex )
      {
#ifdef SAMSONRENT
         if( objindex->count >= objindex->limit )
            continue;
#endif
         obj = create_object( objindex, 1 );
         obj = obj_to_room( obj, toroom, NULL );
         SET_OBJ_STAT( obj, ITEM_ONMAP );
         obj->map = reset->map;
         obj->x = reset->x;
         obj->y = reset->y;
      }

      if( mobindex )
      {
         mob = create_mobile( mobindex );
         char_to_room( mob, toroom );
         SET_ACT_FLAG( mob, ACT_ONMAP );
         mob->map = reset->map;
         mob->x = reset->x;
         mob->y = reset->y;
      }
   }
}

void fread_mapreset( MAPRESET_DATA * reset, FILE * fp )
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

         case 'C':
            if( !str_cmp( word, "Coordinates" ) )
            {
               reset->map = fread_number( fp );
               reset->x = fread_number( fp );
               reset->y = fread_number( fp );
               fMatch = TRUE;
               break;
            }
            break;

         case 'E':
            if( !str_cmp( word, "End" ) )
               return;
            break;

         case 'T':
            KEY( "Type", reset->type, fread_number( fp ) );
            break;

         case 'V':
            KEY( "Vnum", reset->vnum, fread_number( fp ) );
            break;
      }

      if( !fMatch )
         bug( "Fread_mapreset: no match: %s", word );
   }
}

void load_mapresets( void )
{
   char filename[256];
   MAPRESET_DATA *reset;
   FILE *fp;

   first_mapreset = NULL;
   last_mapreset = NULL;

   snprintf( filename, 256, "%s%s", MAP_DIR, MAPRESET_FILE );

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
            bug( "%s", "Load_mapresets: # not found." );
            break;
         }

         word = fread_word( fp );
         if( !str_cmp( word, "RESET" ) )
         {
            CREATE( reset, MAPRESET_DATA, 1 );
            fread_mapreset( reset, fp );
            LINK( reset, first_mapreset, last_mapreset, next, prev );
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "Load_mapreset: bad section: %s.", word );
            continue;
         }
      }
      fclose( fp );
      fp = NULL;
   }

   /*
    * Execute the resets at bootup 
    */
   mapreset(  );

   return;
}

void save_mapresets( void )
{
   MAPRESET_DATA *reset;
   FILE *fp;
   char filename[256];

   snprintf( filename, 256, "%s%s", MAP_DIR, MAPRESET_FILE );

   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "%s", "save_mapresets: fopen" );
      perror( filename );
   }
   else
   {
      for( reset = first_mapreset; reset; reset = reset->next )
      {
         fprintf( fp, "%s", "#RESET\n" );
         fprintf( fp, "Type	%d\n", reset->type );
         fprintf( fp, "Vnum	%d\n", reset->vnum );
         fprintf( fp, "Coordinates	%d %d %d\n", reset->map, reset->x, reset->y );
         fprintf( fp, "%s", "End\n\n" );
      }
      fprintf( fp, "%s", "#END\n" );
      fclose( fp );
      fp = NULL;
   }
   return;
}

void add_mapreset( CHAR_DATA * ch, OBJ_INDEX_DATA * objindex, MOB_INDEX_DATA * mobindex )
{
   MAPRESET_DATA *reset;

   CREATE( reset, MAPRESET_DATA, 1 );
   LINK( reset, first_mapreset, last_mapreset, next, prev );
   reset->map = ch->map;
   reset->x = ch->x;
   reset->y = ch->y;

   if( objindex )
   {
      reset->vnum = objindex->vnum;
      reset->type = TYPE_OBJECT;
   }

   if( mobindex )
   {
      reset->vnum = mobindex->vnum;
      reset->type = TYPE_MOBILE;
   }

   save_mapresets(  );

   return;
}

void delete_mapreset( MAPRESET_DATA * reset )
{
   if( !reset )
   {
      bug( "%s", "delete_mapreset: Trying to delete NULL reset!" );
      return;
   }

   UNLINK( reset, first_mapreset, last_mapreset, next, prev );
   DISPOSE( reset );

   save_mapresets(  );

   return;
}

MAPRESET_DATA *check_mapreset( CHAR_DATA * ch )
{
   MAPRESET_DATA *reset;

   for( reset = first_mapreset; reset; reset = reset->next )
   {
      if( reset->map == ch->map && reset->x == ch->x && reset->y == ch->y )
         return reset;
   }

   return NULL;
}

/* Support command to list all overland resets currently loaded */
void do_mapresets( CHAR_DATA * ch, char *argument )
{
   MAPRESET_DATA *reset;
   MOB_INDEX_DATA *mobindex;
   OBJ_INDEX_DATA *objindex;

   send_to_pager( "&gType     Vnum   Name                      Map         Coordinates\r\n", ch );
   send_to_pager( "-----------------------------------------------------------------\r\n", ch );

   for( reset = first_mapreset; reset; reset = reset->next )
   {
      if( reset->type == TYPE_OBJECT )
      {
         objindex = get_obj_index( reset->vnum );
         snprintf( log_buf, MAX_STRING_LENGTH, "%s", objindex->short_descr );
      }

      if( reset->type == TYPE_MOBILE )
      {
         mobindex = get_mob_index( reset->vnum );
         snprintf( log_buf, MAX_STRING_LENGTH, "%s", mobindex->short_descr );
      }

      pager_printf( ch, "%-7.7s %5d   %-25.25s %-10.10s %4dX %4dY\r\n", reset->type == TYPE_OBJECT ? "Object" : "Mobile",
                    reset->vnum, log_buf, map_names[reset->map], reset->x, reset->y );
   }
   return;
}

/* OLC command to add/delete/edit overland reset information */
void do_mreset( CHAR_DATA * ch, char *argument )
{
   MAPRESET_DATA *reset;
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int vnum;

   if( IS_NPC( ch ) )
   {
      send_to_char( "Mobs cannot use this command.\r\n", ch );
      return;
   }

   if( !IS_PLR_FLAG( ch, PLR_ONMAP ) )
   {
      send_to_char( "This command can only be used from the overland.\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' || ( str_cmp( arg, "add" ) && str_cmp( arg, "delete" ) ) )
   {
      send_to_char( "Usage: mreset add <object|mobile> vnum or name\r\n", ch );
      send_to_char( "Usage: mreset delete <object|mobile> vnum\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg2 );

   if( arg2[0] == '\0' || ( str_cmp( arg2, "object" ) && str_cmp( arg2, "mobile" ) ) )
   {
      do_mreset( ch, "" );
      return;
   }

   if( !str_cmp( arg2, "object" ) )
   {
      OBJ_INDEX_DATA *objindex = NULL;

      if( is_number( argument ) )
         vnum = atoi( argument );
      else
      {
         char arg1[MAX_INPUT_LENGTH];
         int hash, cnt;
         int count = number_argument( argument, arg1 );

         vnum = -1;
         for( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
            for( objindex = obj_index_hash[hash]; objindex; objindex = objindex->next )
               if( nifty_is_name( arg1, objindex->name ) && ++cnt == count )
               {
                  vnum = objindex->vnum;
                  break;
               }
         if( vnum == -1 )
         {
            send_to_char( "No such object exists.\r\n", ch );
            return;
         }
      }

      objindex = get_obj_index( vnum );

      if( !objindex )
      {
         ch_printf( ch, "Object %d does not exist.\r\n", vnum );
         return;
      }

      if( !str_cmp( arg, "add" ) )
      {
         add_mapreset( ch, objindex, NULL );
         ch_printf( ch, "Object reset for vnum %d added.\r\n", vnum );
         return;
      }

      if( !str_cmp( arg, "delete" ) )
      {
         if( ( reset = check_mapreset( ch ) ) == NULL )
         {
            send_to_char( "There are no resets here to delete.\r\n", ch );
            return;
         }

         for( reset = first_mapreset; reset; reset = reset->next )
         {
            if( reset->map == ch->map
                && reset->x == ch->x && reset->y == ch->y && reset->vnum == vnum && reset->type == TYPE_OBJECT )
            {
               delete_mapreset( reset );
               ch_printf( ch, "Object %d overland reset deleted.\r\n", vnum );
               return;
            }
         }
         ch_printf( ch, "No object reset found for vnum %d here.\r\n", vnum );
         return;
      }
   }

   if( !str_cmp( arg2, "mobile" ) )
   {
      MOB_INDEX_DATA *mobindex = NULL;

      if( is_number( argument ) )
         vnum = atoi( argument );
      else
      {
         char arg1[MAX_INPUT_LENGTH];
         int hash, cnt;
         int count = number_argument( argument, arg1 );

         vnum = -1;
         for( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
            for( mobindex = mob_index_hash[hash]; mobindex; mobindex = mobindex->next )
               if( nifty_is_name( arg1, mobindex->player_name ) && ++cnt == count )
               {
                  vnum = mobindex->vnum;
                  break;
               }
         if( vnum == -1 )
         {
            send_to_char( "No such mobile exists.\r\n", ch );
            return;
         }
      }

      mobindex = get_mob_index( vnum );

      if( !mobindex )
      {
         ch_printf( ch, "Mobile %d does not exist.\r\n", vnum );
         return;
      }

      if( !str_cmp( arg, "add" ) )
      {
         add_mapreset( ch, NULL, mobindex );
         ch_printf( ch, "Mobile reset for vnum %d added.\r\n", vnum );
         return;
      }

      if( !str_cmp( arg, "delete" ) )
      {
         if( ( reset = check_mapreset( ch ) ) == NULL )
         {
            send_to_char( "There are no resets here to delete.\r\n", ch );
            return;
         }

         for( reset = first_mapreset; reset; reset = reset->next )
         {
            if( reset->map == ch->map
                && reset->x == ch->x && reset->y == ch->y && reset->vnum == vnum && reset->type == TYPE_MOBILE )
            {
               delete_mapreset( reset );
               ch_printf( ch, "Mobile %d overland reset deleted.\r\n", vnum );
               return;
            }
         }
         ch_printf( ch, "No mobile reset found for vnum %d here.\r\n", vnum );
         return;
      }
   }

   do_mreset( ch, "" );
   return;
}

/* Overland reset stuff ends here */

/* As it implies, loads the map from the graphic file */
void load_mapfile( char *mapfile, short mapnumber )
{
   FILE *fp;
   char filename[256];
   short graph1, graph2, graph3, x, y, z;
   short terr = SECT_OCEAN;

   snprintf( log_buf, MAX_STRING_LENGTH, "Loading continent of %s.....", map_names[mapnumber] );
   log_string( log_buf );

   snprintf( filename, 256, "%s%s", MAP_DIR, mapfile );

   if( !( fp = fopen( filename, "r" ) ) )
   {
      bug( "load_continent: Missing graphical map file %s for continent!", mapfile );
      shutdown_mud( "Missing map file" );
      exit( 1 );
   }

   for( y = 0; y < MAX_Y; y++ )
   {
      for( x = 0; x < MAX_X; x++ )
      {
         graph1 = getc( fp );
         graph2 = getc( fp );
         graph3 = getc( fp );

         for( z = 0; z < SECT_MAX; z++ )
         {
            if( sect_show[z].graph1 == graph1 && sect_show[z].graph2 == graph2 && sect_show[z].graph3 == graph3 )
            {
               terr = z;
               break;
            }
            terr = SECT_OCEAN;
         }
         putterr( mapnumber, x, y, terr );
      }
   }
   fclose( fp );
   fp = NULL;
   return;
}

/* Called from db.c - loads up the map files at bootup */
void load_maps( void )
{
   short map, x, y;

   log_string( "Initializing map grid array...." );
   for( map = 0; map < MAP_MAX; map++ )
   {
      for( x = 0; x < MAX_X; x++ )
      {
         for( y = 0; y < MAX_Y; y++ )
         {
            putterr( map, x, y, SECT_OCEAN );
         }
      }
   }

   /*
    * My my Samson, aren't you getting slick.... 
    */
   for( x = 0; x < MAP_MAX; x++ )
      load_mapfile( map_filenames[x], x );

   log_string( "Loading overland map exits...." );
   load_entrances(  );

   log_string( "Loading overland landmarks...." );
   load_landmarks(  );

#ifdef DRAGONFLIGHT
   log_string( "Loading landing sites...." );
   load_landing_sites(  );
#endif

   /*
    * Do not put the mapreset loading command here! It won't work at this stage! 
    */

   return;
}

/* The guts of the map display code. Streamlined to only change color codes when it needs to.
 * If you are also using my newly updated Custom Color code you'll get even better performance
 * out of it since that code replaced the stock Smaug color tag convertor, which shaved a few
 * microseconds off the time it takes to display a full immortal map :)
 * Don't believe me? Check this out:
 * 
 * 3 immortals in full immortal view spamming 15 movement commands:
 * Unoptimized code: Timing took 0.123937 seconds.
 * Optimized code  : Timing took 0.031564 seconds.
 *
 * 3 mortals with wizardeye spell on spamming 15 movement commands:
 * Unoptimized code: Timing took 0.064086 seconds.
 * Optimized code  : Timing took 0.009459 seconds.
 *
 * Results: The code is at the very least 4x faster than it was before being optimized.
 *
 * Problem? It crashes the damn game with MAX_STRING_LENGTH set to 4096, so I raised it to 8192.
 */
void new_map_to_char( CHAR_DATA * ch, short startx, short starty, short endx, short endy, int radius )
{
   CHAR_DATA *rch;
   OBJ_DATA *obj;
   char secbuf[MAX_STRING_LENGTH];
   short x, y, sector, lastsector;
   bool other, object, group, npc, aship;

   if( startx < 0 )
      startx = 0;

   if( starty < 0 )
      starty = 0;

   if( endx >= MAX_X )
      endx = MAX_X - 1;

   if( endy >= MAX_Y )
      endy = MAX_Y - 1;

   lastsector = -1;
   secbuf[0] = '\0';

   for( y = starty; y < endy + 1; y++ )
   {
      for( x = startx; x < endx + 1; x++ )
      {
         if( distance( ch->x, ch->y, x, y ) > radius )
         {
            if( !IS_PLR_FLAG( ch, PLR_HOLYLIGHT ) )
            {
               strncat( secbuf, " ", MAX_STRING_LENGTH );
               lastsector = -1;
               continue;
            }
         }

         sector = get_terrain( ch->map, x, y );
         other = FALSE;
         npc = FALSE;
         object = FALSE;
         group = FALSE;
         aship = FALSE;
         for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
         {
            if( x == rch->x && y == rch->y && rch != ch && ( rch->x != ch->x || rch->y != ch->y ) )
            {
               if( IS_PLR_FLAG( rch, PLR_WIZINVIS ) && rch->pcdata->wizinvis > ch->level )
                  other = FALSE;
               else
               {
                  other = TRUE;
                  if( IS_NPC( rch ) )
                     npc = TRUE;
                  lastsector = -1;
               }
            }

            if( is_same_group( ch, rch ) && ch != rch )
            {
               if( x == ch->x && y == ch->y && ( rch->x == ch->x && rch->y == ch->y ) )
               {
                  group = TRUE;
                  lastsector = -1;
               }
            }
         }

         for( obj = ch->in_room->first_content; obj; obj = obj->next_content )
         {
            if( x == obj->x && y == obj->y && ( obj->x != ch->x || obj->y != ch->y ) )
            {
               object = TRUE;
               lastsector = -1;
            }
         }

         if( object && !other && !aship )
            strncat( secbuf, "&Y$", MAX_STRING_LENGTH );

         if( other && !aship )
         {
            if( npc )
               strncat( secbuf, "&B@", MAX_STRING_LENGTH );
            else
               strncat( secbuf, "&P@", MAX_STRING_LENGTH );
         }

         if( aship )
            send_to_char( "&R4", ch );

         if( x == ch->x && y == ch->y && !aship )
         {
            if( group )
               strncat( secbuf, "&Y@", MAX_STRING_LENGTH );
            else
               strncat( secbuf, "&R@", MAX_STRING_LENGTH );
            other = TRUE;
            lastsector = -1;
         }

         if( !other && !object && !aship )
         {
            if( lastsector == sector )
               strncat( secbuf, sect_show[sector].symbol, MAX_STRING_LENGTH );
            else
            {
               lastsector = sector;
               strncat( secbuf, sect_show[sector].color, MAX_STRING_LENGTH );
               strncat( secbuf, sect_show[sector].symbol, MAX_STRING_LENGTH );
            }
         }
      }
      strncat( secbuf, "\r\n", MAX_STRING_LENGTH );
   }

   /*
    * Clears the screen, then homes the cursor before displaying the map buffer 
    */
   /*
    * Thanks for the tip on ANSI codes for this Orion :) 
    */
/*
    send_to_char( "\e[2J", ch );
    send_to_char( "\e[0;0f", ch );
*/
   send_to_char( secbuf, ch );
   return;
}

/* This function determines the size of the display to show to a character.
 * For immortals, it also shows any details present in the sector, like resets and landmarks.
 */
void display_map( CHAR_DATA * ch )
{
   LANDMARK_DATA *landmark = NULL;
   MAPRESET_DATA *reset = NULL;
#ifdef DRAGONFLIGHT
   LANDING_DATA *landing = NULL;
#endif
   short startx, starty, endx, endy, sector;

   /*
    * Adjust this to change the size of the map display radius 
    */
   int mod = 7;

   if( ch->map == -1 )
   {
      bug( "display_map: Player %s on invalid map! Moving them to %s.", ch->name, map_names[MAP_C1] );
      ch_printf( ch, "&RYou were found on an invalid map and have been moved to %s.\r\n", map_names[MAP_C1] );
      enter_map( ch, 499, 500, ACON_C1 );
      return;
   }

   sector = get_terrain( ch->map, ch->x, ch->y );

   if( IS_PLR_FLAG( ch, PLR_HOLYLIGHT ) )
   {
      startx = ch->x - 37;
      endx = ch->x + 37;
      starty = ch->y - 14;
      endy = ch->y + 14;
   }
   else
   {
      OBJ_DATA *light;

      light = get_eq_char( ch, WEAR_LIGHT );

      if( time_info.hour == HOUR_SUNRISE || time_info.hour == HOUR_SUNSET )
         mod = 4;

      if( ( ch->in_room->area->weather->precip + 3 * weath_unit - 1 ) / weath_unit > 1 && mod != 1 )
         mod -= 1;

      if( time_info.hour > HOUR_SUNSET || time_info.hour < HOUR_SUNRISE )
         mod = 2;

      if( light != NULL )
      {
         if( light->item_type == ITEM_LIGHT && ( time_info.hour > HOUR_SUNSET || time_info.hour < HOUR_SUNRISE ) )
            mod += 1;
      }

      startx = ( short )( UMAX( ch->x - ( mod * 1.5 ), ch->x - 37 ) );
      starty = ( short )( UMAX( ch->y - mod, ch->y - 14 ) );
      endx = ( short )( UMIN( ch->x + ( mod * 1.5 ), ch->x + 37 ) );
      endy = ( short )( UMIN( ch->y + mod, ch->y + 14 ) );
   }

   if( IS_PLR_FLAG( ch, PLR_MAPEDIT ) && sector != SECT_EXIT )
   {
      putterr( ch->map, ch->x, ch->y, ch->pcdata->secedit );
      sector = ch->pcdata->secedit;
   }

   new_map_to_char( ch, startx, starty, endx, endy, mod );

#ifdef DRAGONFLIGHT
   if( !ch->dragonflight )
#endif
   {
      ch_printf( ch, "&GTravelling on the continent of %s.\r\n", map_names[ch->map] );
      landmark = check_landmark( ch->map, ch->x, ch->y );

      if( landmark && landmark->Isdesc )
         ch_printf( ch, "&G%s\r\n", landmark->description ? landmark->description : "" );
      else
         ch_printf( ch, "&G%s\r\n", impass_message[sector] );
   }
#ifdef DRAGONFLIGHT
   else
      ch_printf( ch, "&GRiding a dragon over the continent of %s.\r\n", map_names[ch->map] );
#endif

   if( IS_IMMORTAL( ch ) )
   {
      ch_printf( ch, "&GSector type: %s. Coordinates: %dX, %dY\r\n", sect_types[sector], ch->x, ch->y );

      reset = check_mapreset( ch );
#ifdef DRAGONFLIGHT
      landing = check_landing_site( ch->map, ch->x, ch->y );

      if( landing )
         ch_printf( ch, "&CLanding site for %s.\r\n", landing->area ? landing->area : "<NOT SET>" );
#endif

      if( landmark && !landmark->Isdesc )
      {
         ch_printf( ch, "&BLandmark present: %s\r\n", landmark->description ? landmark->description : "<NO DESCRIPTION>" );
         ch_printf( ch, "&BVisibility distance: %d.\r\n", landmark->distance );
      }

      /*
       * If you have one, check for more since more than one of these can be in the same sector 
       */
      if( reset )
      {
         for( reset = first_mapreset; reset; reset = reset->next )
         {
            if( reset->map == ch->map && reset->x == ch->x && reset->y == ch->y )
               ch_printf( ch, "&P%s reset present: Vnum %d\r\n", reset->type == TYPE_OBJECT ? "Object" : "Mobile",
                          reset->vnum );
         }
      }

      if( IS_PLR_FLAG( ch, PLR_MAPEDIT ) )
         ch_printf( ch, "&YYou are currently creating %s sectors.&z\r\n", sect_types[ch->pcdata->secedit] );
   }
   return;
}

/* Called in update.c modification for wandering mobiles - Samson 7-29-00
 * For mobs entering overland from normal zones:
 * If the sector of the origin room matches the terrain type on the map,
 *    it becomes the mob's native terrain.
 * If the origin room sector does NOT match, then the mob will assign itself the
 *    first terrain type it can move onto that is different from what it entered on.
 * This allows you to load mobs in a loading room, and have them stay on roads or
 *    trails to act as patrols. And also compensates for when a map road or trail
 *    enters a zone and becomes forest 
 */
bool map_wander( CHAR_DATA * ch, int map, int x, int y, int sector )
{
   /*
    * Obviously sentinel mobs have no need to move :P 
    */
   if( IS_ACT_FLAG( ch, ACT_SENTINEL ) )
      return FALSE;

   /*
    * Allows the mob to move onto it's native terrain as well as roads or trails - 
    * * EG: a mob loads in a forest, but a road slices it in half, mob can cross the
    * * road to reach more forest on the other side. Won't cross SECT_ROAD though,
    * * we use this to keep SECT_CITY and SECT_TRAIL mobs from leaving the roads
    * * near their origin sites - Samson 7-29-00
    */
   if( get_terrain( map, x, y ) == ch->sector
       || get_terrain( map, x, y ) == SECT_CITY || get_terrain( map, x, y ) == SECT_TRAIL )
      return TRUE;

   /*
    * Sector -2 is used to tell the code this mob came to the overland from an internal
    * * zone, but the terrain didn't match the originating room's sector type. It'll assign 
    * * the first differing terrain upon moving, provided it isn't a SECT_ROAD. From then on 
    * * it will only wander in that type of terrain - Samson 7-29-00
    */
   if( ch->sector == -2
       && get_terrain( map, x, y ) != sector
       && get_terrain( map, x, y ) != SECT_ROAD && sect_show[get_terrain( map, x, y )].canpass )
   {
      ch->sector = get_terrain( map, x, y );
      return TRUE;
   }

   return FALSE;
}

/* This function does a random check, currently set to 6%, to see if it should load a mobile
 * at the sector the PC just walked into. I have no doubt a cleaner, more efficient way can
 * be found to accomplish this, but until such time as divine inspiration hits me ( or some kind
 * soul shows me the light ) this is how its done. [WOO! Yes Geni, I think that's how its done :P]
 * 
 * Rewritten by Geni to use tables and way too many mobs.
 */
void check_random_mobs( CHAR_DATA * ch )
{
   MOB_INDEX_DATA *imob = NULL;
   CHAR_DATA *mob = NULL;
   int vnum = -1;
   int terrain = get_terrain( ch->map, ch->x, ch->y );

   /*
    * This could ( and did ) get VERY messy
    * * VERY quickly if wandering mobs trigger more of themselves
    */
   if( IS_NPC( ch ) )
      return;

   if( number_percent(  ) < 6 )
      vnum = random_mobs[terrain][UMIN( 25, number_range( 0, ch->level / 2 ) )];

   if( vnum == -1 )
      return;
   imob = get_mob_index( vnum );
   if( !imob )
   {
      snprintf( log_buf, MAX_STRING_LENGTH, "check_random_mobs: Missing mob for vnum %d", vnum );
      log_string( log_buf );
      return;
   }

   mob = create_mobile( imob );
   char_to_room( mob, ch->in_room );
   SET_ACT_FLAG( mob, ACT_ONMAP );
   mob->sector = terrain;
   mob->map = ch->map;
   mob->x = ch->x;
   mob->y = ch->y;
   mob->timer = 400;
   /*
    * This should be long enough. If not, increase. Mob will be extracted when it expires.
    * * And trust me, this is a necessary measure unless you LIKE having your memory flooded
    * * by random overland mobs.
    */
   return;
}

/* An overland hack of the scan command - the OLD scan command, not the one in stock Smaug
 * where you have to specify a direction to scan in. This is only called from within do_scan
 * and probably won't be of much use to you unless you want to modify your scan command to
 * do like ours. And hey, if you do, I'd be happy to send you our do_scan - separately.
 */
void map_scan( CHAR_DATA * ch )
{
   CHAR_DATA *gch;
   OBJ_DATA *light;
   double dist, angle;
   int mod = 7;
   int dir = -1;
   int iMes = 0;
   bool found = FALSE;

   if( !ch )
      return;

   light = get_eq_char( ch, WEAR_LIGHT );

   if( time_info.hour == HOUR_SUNRISE || time_info.hour == HOUR_SUNSET )
      mod = 4;

   if( ( ch->in_room->area->weather->precip + 3 * weath_unit - 1 ) / weath_unit > 1 && mod != 1 )
      mod -= 1;

   if( time_info.hour > HOUR_SUNSET || time_info.hour < HOUR_SUNRISE )
      mod = 1;

   if( light != NULL )
   {
      if( light->item_type == ITEM_LIGHT && ( time_info.hour > HOUR_SUNSET || time_info.hour < HOUR_SUNRISE ) )
         mod += 1;
   }

   /*
    * Freshen the map with up to the nanosecond display :) 
    */
   do_look( ch, "auto" );

   for( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
   {
      /*
       * No need in scanning for yourself. 
       */
      if( ch == gch )
         continue;

      /*
       * Don't reveal invisible imms 
       */
      if( IS_PLR_FLAG( gch, PLR_WIZINVIS ) && gch->pcdata->wizinvis > ch->level )
         continue;

      dist = distance( ch->x, ch->y, gch->x, gch->y );

      /*
       * Save the math if they're too far away anyway 
       */
      if( dist <= mod )
      {
         found = TRUE;

         angle = calc_angle( ch->x, ch->y, gch->x, gch->y, &dist );

         if( angle == -1 )
            dir = -1;
         else if( angle >= 360 )
            dir = DIR_NORTH;
         else if( angle >= 315 )
            dir = DIR_NORTHWEST;
         else if( angle >= 270 )
            dir = DIR_WEST;
         else if( angle >= 225 )
            dir = DIR_SOUTHWEST;
         else if( angle >= 180 )
            dir = DIR_SOUTH;
         else if( angle >= 135 )
            dir = DIR_SOUTHEAST;
         else if( angle >= 90 )
            dir = DIR_EAST;
         else if( angle >= 45 )
            dir = DIR_NORTHEAST;
         else if( angle >= 0 )
            dir = DIR_NORTH;

         if( dist > 200 )
            iMes = 0;
         else if( dist > 150 )
            iMes = 1;
         else if( dist > 100 )
            iMes = 2;
         else if( dist > 75 )
            iMes = 3;
         else if( dist > 50 )
            iMes = 4;
         else if( dist > 25 )
            iMes = 5;
         else if( dist > 15 )
            iMes = 6;
         else if( dist > 10 )
            iMes = 7;
         else if( dist > 5 )
            iMes = 8;
         else if( dist > 1 )
            iMes = 9;
         else
            iMes = 10;

         set_char_color( AT_SKILL, ch );
         if( dir == -1 )
            ch_printf( ch, "Here with you: %s.\r\n", IS_NPC( gch ) ? gch->short_descr : gch->name );
         else
            ch_printf( ch, "To the %s, %s, %s.\r\n", dir_name[dir], landmark_distances[iMes],
                       IS_NPC( gch ) ? gch->short_descr : gch->name );
      }
   }
   if( !found )
      send_to_char( "Your survey of the area turns up nobody.\r\n", ch );

   return;
}

/* Note: For various reasons, this isn't designed to pull PC followers along with you */
void collect_followers( CHAR_DATA * ch, ROOM_INDEX_DATA * from, ROOM_INDEX_DATA * to )
{
   CHAR_DATA *fch;
   CHAR_DATA *nextinroom;
   int chars = 0, count = 0;

   if( !ch )
   {
      bug( "%s", "collect_followers: NULL master!" );
      return;
   }

   if( !from )
   {
      bug( "collect_followers: %s NULL source room!", ch->name );
      return;
   }

   if( !to )
   {
      bug( "collect_followers: %s NULL target room!", ch->name );
      return;
   }

   for( fch = from->first_person; fch; fch = fch->next_in_room )
      chars++;

   for( fch = from->first_person; fch && ( count < chars ); fch = nextinroom )
   {
      nextinroom = fch->next_in_room;
      count++;

      if( fch != ch && fch->master == ch && ( fch->position == POS_STANDING || fch->position == POS_MOUNTED ) )
      {
         if( !IS_NPC( fch ) )
            continue;

         char_from_room( fch );
         char_to_room( fch, to );
         fix_maps( ch, fch );
      }
   }
   return;
}

/* The guts of movement on the overland. Checks all sorts of nice things. Makes DAMN
 * sure you can actually go where you just tried to. Breaks easily if messed with wrong too.
 */
ch_ret process_exit( CHAR_DATA * ch, short map, short x, short y, int dir )
{
   char buf[MAX_STRING_LENGTH];
   short sector = get_terrain( map, x, y );
   char *txt;
   char *dtxt;
   bool drunk = FALSE;
   CHAR_DATA *fch;
   CHAR_DATA *nextinroom;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *from_room = ch->in_room;
   ch_ret retcode = rNONE;
   int move, chars = 0, count = 0;
   short fx = ch->x, fy = ch->y, fmap = ch->map;
   bool boat = FALSE;

   if( !IS_NPC( ch ) )
   {
      if( IS_DRUNK( ch, 2 ) && ( ch->position != POS_SHOVE ) && ( ch->position != POS_DRAG ) )
         drunk = TRUE;
   }

   for( obj = ch->first_carrying; obj; obj = obj->next_content )
   {
      if( obj->item_type == ITEM_BOAT )
      {
         boat = TRUE;
         break;
      }
   }

   if( IS_AFFECTED( ch, AFF_FLYING ) || IS_AFFECTED( ch, AFF_FLOATING ) )
      boat = TRUE;   /* Cheap hack, but I think it'll work for this purpose */

   if( IS_IMMORTAL( ch ) )
      boat = TRUE;   /* Cheap hack, but hey, imms need a break on this one :P */

   if( sector == SECT_EXIT )
   {
      ENTRANCE_DATA *enter;
      ROOM_INDEX_DATA *toroom = NULL;

      enter = check_entrance( map, x, y );

      if( enter != NULL && !IS_PLR_FLAG( ch, PLR_MAPEDIT ) )
      {
         if( enter->tomap != -1 )   /* Means exit goes to another map */
         {
            enter_map( ch, enter->therex, enter->therey, enter->tomap );
            if( ch->mount )
               enter_map( ch->mount, enter->therex, enter->therey, enter->tomap );

            for( fch = from_room->first_person; fch; fch = fch->next_in_room )
               chars++;

            for( fch = from_room->first_person; fch && ( count < chars ); fch = nextinroom )
            {
               nextinroom = fch->next_in_room;
               count++;
               if( fch != ch  /* loop room bug fix here by Thoric */
                   && fch->master == ch
                   && ( fch->position == POS_STANDING || fch->position == POS_MOUNTED )
                   && fch->x == fx && fch->y == fy && fch->map == fmap )
               {
                  if( !IS_NPC( fch ) )
                  {
                     act( AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR );
                     process_exit( fch, fch->map, x, y, dir );
                  }
                  else
                     enter_map( fch, enter->therex, enter->therey, enter->tomap );
               }
            }
            return rNONE;
         }

         if( ( toroom = get_room_index( enter->vnum ) ) == NULL )
         {
            bug( "Target vnum %d for map exit does not exist!", enter->vnum );
            send_to_char( "Ooops. Something bad happened. Contact the immortals ASAP.\r\n", ch );
            return rNONE;
         }

         if( IS_NPC( ch ) )
         {
            EXIT_DATA *pexit;
            bool found = FALSE;

            for( pexit = toroom->first_exit; pexit; pexit = pexit->next )
            {
               if( IS_EXIT_FLAG( pexit, EX_OVERLAND ) )
               {
                  found = TRUE;
                  break;
               }
            }

            if( found )
            {
               if( IS_EXIT_FLAG( pexit, EX_NOMOB ) )
               {
                  send_to_char( "Mobs cannot go there.\r\n", ch );
                  return rNONE;
               }
            }
         }

         if( ( toroom->sector_type == SECT_WATER_NOSWIM || toroom->sector_type == SECT_OCEAN ) && !boat )
         {
            send_to_char( "The water is too deep to cross without a boat or spell!\r\n", ch );
            return rNONE;
         }

         if( toroom->sector_type == SECT_RIVER && !boat )
         {
            send_to_char( "The river is too swift to cross without a boat or spell!\r\n", ch );
            return rNONE;
         }

         if( toroom->sector_type == SECT_AIR && !IS_AFFECTED( ch, AFF_FLYING ) )
         {
            send_to_char( "You'd need to be able to fly to go there!\r\n", ch );
            return rNONE;
         }

         leave_map( ch, NULL, toroom );

         for( fch = from_room->first_person; fch; fch = fch->next_in_room )
            chars++;

         for( fch = from_room->first_person; fch && ( count < chars ); fch = nextinroom )
         {
            nextinroom = fch->next_in_room;
            count++;
            if( fch != ch  /* loop room bug fix here by Thoric */
                && fch->master == ch
                && ( fch->position == POS_STANDING || fch->position == POS_MOUNTED )
                && fch->x == fx && fch->y == fy && fch->map == fmap )
            {
               if( !IS_NPC( fch ) )
               {
                  act( AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR );
                  process_exit( fch, fch->map, x, y, dir );
               }
               else
                  leave_map( fch, ch, toroom );
            }
         }
         return rNONE;
      }

      if( enter != NULL && IS_PLR_FLAG( ch, PLR_MAPEDIT ) )
      {
         delete_entrance( enter );
         putterr( ch->map, x, y, ch->pcdata->secedit );
         send_to_char( "&RMap exit deleted.\r\n", ch );
      }
   }

   if( !sect_show[sector].canpass && !IS_IMMORTAL( ch ) )
   {
      ch_printf( ch, "%s\r\n", impass_message[sector] );
      return rNONE;
   }

   if( sector == SECT_RIVER && !boat )
   {
      send_to_char( "The river is too swift to cross without a boat or spell!\r\n", ch );
      return rNONE;
   }

   if( sector == SECT_WATER_NOSWIM && !boat )
   {
      send_to_char( "The water is too deep to navigate without a boat or spell!\r\n", ch );
      return rNONE;
   }

   switch ( dir )
   {
      case DIR_NORTH:
         if( y == -1 )
         {
            send_to_char( "You cannot go any further north!\r\n", ch );
            return rNONE;
         }
         break;

      case DIR_EAST:
         if( x == MAX_X )
         {
            send_to_char( "You cannot go any further east!\r\n", ch );
            return rNONE;
         }
         break;

      case DIR_SOUTH:
         if( y == MAX_Y )
         {
            send_to_char( "You cannot go any further south!\r\n", ch );
            return rNONE;
         }
         break;

      case DIR_WEST:
         if( x == -1 )
         {
            send_to_char( "You cannot go any further west!\r\n", ch );
            return rNONE;
         }
         break;

      case DIR_NORTHEAST:
         if( x == MAX_X || y == -1 )
         {
            send_to_char( "You cannot go any further northeast!\r\n", ch );
            return rNONE;
         }
         break;

      case DIR_NORTHWEST:
         if( x == -1 || y == -1 )
         {
            send_to_char( "You cannot go any further northwest!\r\n", ch );
            return rNONE;
         }
         break;

      case DIR_SOUTHEAST:
         if( x == MAX_X || y == MAX_Y )
         {
            send_to_char( "You cannot go any further southeast!\r\n", ch );
            return rNONE;
         }
         break;

      case DIR_SOUTHWEST:
         if( x == -1 || y == MAX_Y )
         {
            send_to_char( "You cannot go any further southwest!\r\n", ch );
            return rNONE;
         }
         break;
   }

   if( ch->mount && !IS_FLOATING( ch->mount ) )
      move = sect_show[sector].move;
   else if( !IS_FLOATING( ch ) )
      move = sect_show[sector].move;
   else
      move = 1;

   if( ch->mount && ch->mount->move < move )
   {
      send_to_char( "Your mount is too exhausted.\r\n", ch );
      return rNONE;
   }

   if( ch->move < move )
   {
      send_to_char( "You are too exhausted.\r\n", ch );
      return rNONE;
   }

   if( ch->mount )
   {
      if( IS_AFFECTED( ch->mount, AFF_FLOATING ) )
         txt = "floats";
      else if( IS_AFFECTED( ch->mount, AFF_FLYING ) )
         txt = "flies";
      else
         txt = "rides";
   }
   else if( IS_AFFECTED( ch, AFF_FLOATING ) )
   {
      if( drunk )
         txt = "floats unsteadily";
      else
         txt = "floats";
   }
   else if( IS_AFFECTED( ch, AFF_FLYING ) )
   {
      if( drunk )
         txt = "flies shakily";
      else
         txt = "flies";
   }
   else if( ch->position == POS_SHOVE )
      txt = "is shoved";
   else if( ch->position == POS_DRAG )
      txt = "is dragged";
   else
   {
      if( drunk )
         txt = "stumbles drunkenly";
      else
         txt = "leaves";
   }

   if( ch->mount )
   {
      snprintf( buf, MAX_STRING_LENGTH, "$n %s %s upon $N.", txt, dir_name[dir] );
      act( AT_ACTION, buf, ch, NULL, ch->mount, TO_NOTVICT );
   }
   else
   {
      snprintf( buf, MAX_STRING_LENGTH, "$n %s $T.", txt );
      act( AT_ACTION, buf, ch, NULL, dir_name[dir], TO_ROOM );
   }

   if( !IS_IMMORTAL( ch ) )   /* Imms don't get charged movement */
   {
      if( ch->mount )
      {
         ch->mount->move -= move;
         ch->mount->x = x;
         ch->mount->y = y;
      }
      else
         ch->move -= move;
   }

   ch->x = x;
   ch->y = y;

   /*
    * Don't make your imms suffer - editing with movement delays is a bitch 
    */
   if( !IS_IMMORTAL( ch ) )
      WAIT_STATE( ch, move );

   if( ch->mount )
   {
      if( IS_AFFECTED( ch->mount, AFF_FLOATING ) )
         txt = "floats in";
      else if( IS_AFFECTED( ch->mount, AFF_FLYING ) )
         txt = "flies in";
      else
         txt = "rides in";
   }
   else
   {
      if( IS_AFFECTED( ch, AFF_FLOATING ) )
      {
         if( drunk )
            txt = "floats in unsteadily";
         else
            txt = "floats in";
      }
      else if( IS_AFFECTED( ch, AFF_FLYING ) )
      {
         if( drunk )
            txt = "flies in shakily";
         else
            txt = "flies in";
      }
      else if( ch->position == POS_SHOVE )
         txt = "is shoved in";
      else if( ch->position == POS_DRAG )
         txt = "is dragged in";
      else
      {
         if( drunk )
            txt = "stumbles drunkenly in";
         else
            txt = "arrives";
      }
   }
   dtxt = rev_exit( dir );

   if( ch->mount )
   {
      snprintf( buf, MAX_STRING_LENGTH, "$n %s from %s upon $N.", txt, dtxt );
      act( AT_ACTION, buf, ch, NULL, ch->mount, TO_ROOM );
   }
   else
   {
      snprintf( buf, MAX_STRING_LENGTH, "$n %s from %s.", txt, dtxt );
      act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
   }

   for( fch = from_room->first_person; fch; fch = fch->next_in_room )
      chars++;

   for( fch = from_room->first_person; fch && ( count < chars ); fch = nextinroom )
   {
      nextinroom = fch->next_in_room;
      count++;
      if( fch != ch  /* loop room bug fix here by Thoric */
          && fch->master == ch
          && ( fch->position == POS_STANDING || fch->position == POS_MOUNTED ) && fch->x == fx && fch->y == fy )
      {
         if( !IS_NPC( fch ) )
         {
            act( AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR );
            process_exit( fch, fch->map, x, y, dir );
         }
         else
         {
            fch->x = x;
            fch->y = y;
         }
      }
   }

   check_random_mobs( ch );

   do_look( ch, "auto" );

   mprog_entry_trigger( ch );
   if( char_died( ch ) )
      return retcode;

   rprog_enter_trigger( ch );
   if( char_died( ch ) )
      return retcode;

   mprog_greet_trigger( ch );
   if( char_died( ch ) )
      return retcode;

   oprog_greet_trigger( ch );
   if( char_died( ch ) )
      return retcode;

   return retcode;
}

/* Fairly self-explanitory. It finds out what continent an area is on and drops the PC on the appropriate map for it */
ROOM_INDEX_DATA *find_continent( CHAR_DATA * ch, ROOM_INDEX_DATA * maproom )
{
   ROOM_INDEX_DATA *location = NULL;

   if( maproom->area->continent == ACON_C1 )
   {
      location = get_room_index( OVERLAND_MAP1 );
      ch->map = MAP_C1;
   }

   if( maproom->area->continent == ACON_C2 )
   {
      location = get_room_index( OVERLAND_MAP2 );
      ch->map = MAP_C2;
   }

   if( maproom->area->continent == ACON_C3 )
   {
      location = get_room_index( OVERLAND_MAP3 );
      ch->map = MAP_C3;
   }

   return location;
}

/* How one gets from a normal zone onto the overland, via an exit flagged as EX_OVERLAND */
void enter_map( CHAR_DATA * ch, int x, int y, int continent )
{
   ROOM_INDEX_DATA *maproom = NULL, *original;

   if( continent < 0 )  /* -1 means you came in from a regular area exit */
      maproom = find_continent( ch, ch->in_room );

   else  /* Means you are either an immortal using the goto command, or a mortal who teleported */
   {
      switch ( continent )
      {
         case ACON_C1:
            maproom = get_room_index( OVERLAND_MAP1 );
            ch->map = MAP_C1;
            break;
         case ACON_C2:
            maproom = get_room_index( OVERLAND_MAP2 );
            ch->map = MAP_C2;
            break;
         case ACON_C3:
            maproom = get_room_index( OVERLAND_MAP3 );
            ch->map = MAP_C3;
            break;
         default:
            bug( "Invalid target map specified: %d", continent );
            return;
      }
   }

   if( !maproom )
   {
      bug( "%s", "enter_map: Overland map room is missing!" );
      send_to_char( "Woops. Something is majorly wrong here - inform the immortals.\r\n", ch );
      return;
   }

   if( !IS_NPC( ch ) )
      SET_PLR_FLAG( ch, PLR_ONMAP );
   else
   {
      SET_ACT_FLAG( ch, ACT_ONMAP );
      if( ch->sector == -1 && get_terrain( ch->map, x, y ) == ch->in_room->sector_type )
         ch->sector = get_terrain( ch->map, x, y );
      else
         ch->sector = -2;
   }

   ch->x = x;
   ch->y = y;

   original = ch->in_room;
   char_from_room( ch );
   char_to_room( ch, maproom );
   collect_followers( ch, original, ch->in_room );
   do_look( ch, "auto" );

   return;
}

 /*
  * How one gets off the overland into a regular zone via those nifty white # symbols :) 
  * * Of course, it's also how you leave the overland by any means, but hey. 
  * * This can also be used to simply move a person from a normal room to another normal room.
  */
void leave_map( CHAR_DATA * ch, CHAR_DATA * victim, ROOM_INDEX_DATA * target )
{
   if( !IS_NPC( ch ) )
   {
      REMOVE_PLR_FLAG( ch, PLR_ONMAP );
      REMOVE_PLR_FLAG( ch, PLR_MAPEDIT ); /* Just in case they were editing */
   }
   else
      REMOVE_ACT_FLAG( ch, ACT_ONMAP );

   ch->x = -1;
   ch->y = -1;
   ch->map = -1;

   if( target != NULL )
   {
      ROOM_INDEX_DATA *from = ch->in_room;
      char_from_room( ch );
      char_to_room( ch, target );
      fix_maps( victim, ch );
      collect_followers( ch, from, target );
      do_look( ch, "auto" );
   }

   return;
}

/* Imm command to jump to a different set of coordinates on the same map */
void do_coords( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   int x, y;

   if( IS_NPC( ch ) )
   {
      send_to_char( "NPCs cannot use this command.\r\n", ch );
      return;
   }

   if( !IS_PLR_FLAG( ch, PLR_ONMAP ) )
   {
      send_to_char( "This command can only be used from the overland maps.\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg );

   if( arg[0] == '\0' || argument[0] == '\0' )
   {
      send_to_char( "Usage: coords <x> <y>\r\n", ch );
      return;
   }

   x = atoi( arg );
   y = atoi( argument );

   if( x < 0 || x >= MAX_X )
   {
      ch_printf( ch, "Valid x coordinates are 0 to %d.\r\n", MAX_X - 1 );
      return;
   }

   if( y < 0 || y >= MAX_Y )
   {
      ch_printf( ch, "Valid y coordinates are 0 to %d.\r\n", MAX_Y - 1 );
      return;
   }

   ch->x = x;
   ch->y = y;

   if( ch->mount )
   {
      ch->mount->x = x;
      ch->mount->y = y;
   }

   do_look( ch, "auto" );
   return;
}

/* Online OLC map editing stuff starts here */

/* Stuff for the floodfill and undo functions */
struct undotype
{
   short map;
   short xcoord;
   short ycoord;
   short prevterr;
   struct undotype *next;
};

struct undotype *undohead = 0x0; /* for undo buffer */
struct undotype *undocurr = 0x0; /* most recent undo data */

/* This baby floodfills an entire region of sectors, starting from where the PC is standing,
 * and continuing until it hits something other than what it was told to fill with. 
 * Aborts if you attempt to fill with the same sector your standing on.
 * Floodfill code courtesy of Vor - don't ask for his address, I'm not allowed to give it to you!
 */
int floodfill( short map, short xcoord, short ycoord, short fill, char terr )
{
   struct undotype *undonew;

   /*
    * Abort if trying to flood same terr type 
    */
   if( terr == fill )
      return ( 1 );

   undonew = ( struct undotype * )malloc( sizeof( struct undotype ) );

   undonew->map = map;
   undonew->xcoord = xcoord;
   undonew->ycoord = ycoord;
   undonew->prevterr = terr;
   undonew->next = 0x0;

   if( undohead == 0x0 )
   {
      undohead = undocurr = undonew;
   }
   else
   {
      undocurr->next = undonew;
      undocurr = undonew;
   };

   putterr( map, xcoord, ycoord, fill );

   if( get_terrain( map, xcoord + 1, ycoord ) == terr )
      floodfill( map, xcoord + 1, ycoord, fill, terr );
   if( get_terrain( map, xcoord, ycoord + 1 ) == terr )
      floodfill( map, xcoord, ycoord + 1, fill, terr );
   if( get_terrain( map, xcoord - 1, ycoord ) == terr )
      floodfill( map, xcoord - 1, ycoord, fill, terr );
   if( get_terrain( map, xcoord, ycoord - 1 ) == terr )
      floodfill( map, xcoord, ycoord - 1, fill, terr );

   return ( 0 );
}

/* call this to undo any floodfills buffered, changes the undo buffer 
to hold the undone terr type, so doing an undo twice actually 
redoes the floodfill :) */
int unfloodfill( void )
{
   char terr;  /* terr to undo */

   if( undohead == 0x0 )
      return ( 0 );

   undocurr = undohead;
   do
   {
      terr = get_terrain( undocurr->map, undocurr->xcoord, undocurr->ycoord );
      putterr( undocurr->map, undocurr->xcoord, undocurr->ycoord, undocurr->prevterr );
      undocurr->prevterr = terr;
      undocurr = undocurr->next;
   }
   while( undocurr->next != 0x0 );

   /*
    * you'll prolly want to add some error checking here.. :P 
    */
   return ( 0 );
}

/* call this any time you want to clear the undo buffer, such as 
between successful floodfills (otherwise, an undo will undo ALL floodfills done this session :P) */
int purgeundo( void )
{
   while( undohead != 0x0 )
   {
      undocurr = undohead;
      undohead = undocurr->next;
      free( undocurr );
   }

   /*
    * you'll prolly want to add some error checking here.. :P 
    */
   return ( 0 );
}

/* Used to reload a graphic file for the map you are currently standing on.
 * Take great care not to hose your file, results would be unpredictable at best.
 */
void reload_map( CHAR_DATA * ch )
{
   short x, y;

   if( ch->map < 0 || ch->map >= MAP_MAX )
   {
      bug( "%s", "reload_map: Trying to reload invalid map!" );
      return;
   }

   pager_printf( ch, "&GReinitializing map grid for %s....\r\n", map_names[ch->map] );

   for( x = 0; x < MAX_X; x++ )
   {
      for( y = 0; y < MAX_Y; y++ )
         putterr( ch->map, x, y, SECT_OCEAN );
   }
   load_mapfile( map_filenames[ch->map], ch->map );
   return;
}

/* As it implies, this saves the map you are currently standing on to disk.
 * Output is in graphic format, making it easily edited with most paint programs.
 * Could also be used as a method for filtering bad color data out of your source
 * image if it had any at loadup. This code should only be called from the mapedit
 * command. Using it in any other way could break something.
 */
void save_map( char *name, short map )
{
   FILE *gfp;
   char graphicname[256];
   short x, y, terr;

   name = strlower( name );   /* Forces filename into lowercase */

   snprintf( graphicname, 256, "%s%s.raw", MAP_DIR, name );

   if( ( gfp = fopen( graphicname, "w" ) ) == NULL )
   {
      bug( "%s", "save_map: fopen" );
      perror( graphicname );
   }
   else
   {
      for( y = 0; y < MAX_Y; y++ )
      {
         for( x = 0; x < MAX_X; x++ )
         {
            terr = get_terrain( map, x, y );
            if( terr >= SECT_MAX || terr < 0 )
            {
               bug( "save_map: Terrain data out of bounds!!! Value found: %hd - replacing with Ocean.", terr );
               terr = SECT_OCEAN;
            }
            fputc( sect_show[terr].graph1, gfp );
            fputc( sect_show[terr].graph2, gfp );
            fputc( sect_show[terr].graph3, gfp );
         }
      }
      fclose( gfp );
      gfp = NULL;
   }
   return;
}

/* And here we have the OLC command itself. Fairly simplistic really. And more or less useless
 * for anything but really small edits now ( graphic file editing is vastly superior ).
 * Used to be the only means for editing maps before the 12-6-00 release of this code.
 * Caution is warranted if using the floodfill option - trying to floodfill a section that
 * is too large will overflow the memory in short order and cause a rather uncool crash.
 */
void do_mapedit( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   int value;

   if( IS_NPC( ch ) )
   {
      send_to_char( "Sorry, NPCs can't edit the overland maps.\r\n", ch );
      return;
   }

   if( !IS_PLR_FLAG( ch, PLR_ONMAP ) )
   {
      send_to_char( "This command can only be used from an overland map.\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg1 );

   if( arg1[0] == '\0' )
   {
      if( IS_PLR_FLAG( ch, PLR_MAPEDIT ) )
      {
         REMOVE_PLR_FLAG( ch, PLR_MAPEDIT );
         send_to_char( "&GMap editing mode is now OFF.\r\n", ch );
         return;
      }

      SET_PLR_FLAG( ch, PLR_MAPEDIT );
      send_to_char( "&RMap editing mode is now ON.\r\n", ch );
      ch_printf( ch, "&YYou are currently creating %s sectors.&z\r\n", sect_types[ch->pcdata->secedit] );
      return;
   }

   if( !str_cmp( arg1, "help" ) )
   {
      send_to_char( "Usage: mapedit sector <sectortype>\r\n", ch );
      send_to_char( "Usage: mapedit save <mapname>\r\n", ch );
      send_to_char( "Usage: mapedit fill <sectortype>\r\n", ch );
      send_to_char( "Usage: mapedit undo\r\n", ch );
      send_to_char( "Usage: mapedit reload\r\n\r\n", ch );
      return;
   }

   if( !str_cmp( arg1, "reload" ) )
   {
      if( str_cmp( argument, "confirm" ) )
      {
         send_to_char( "This is a dangerous command if used improperly.\r\n", ch );
         send_to_char( "Are you sure about this? Confirm by typing: mapedit reload confirm\r\n", ch );
         return;
      }
      reload_map( ch );
      return;
   }

   if( !str_cmp( arg1, "fill" ) )
   {
      int flood, fill;
      short standingon = get_terrain( ch->map, ch->x, ch->y );

      if( argument[0] == '\0' )
      {
         send_to_char( "Floodfill with what???\r\n", ch );
         return;
      }

      if( standingon == -1 )
      {
         send_to_char( "Unable to process floodfill. Your coordinates are invalid.\r\n", ch );
         return;
      }

      flood = get_sectypes( argument );
      if( flood < 0 || flood >= SECT_MAX )
      {
         send_to_char( "Invalid sector type.\r\n", ch );
         return;
      }

      purgeundo(  );
      fill = floodfill( ch->map, ch->x, ch->y, flood, standingon );

      if( fill == 0 )
      {
         display_map( ch );
         ch_printf( ch, "&RFooodfill with %s sectors successful.\r\n", argument );
         return;
      }

      if( fill == 1 )
      {
         send_to_char( "Cannot floodfill identical terrain type!\r\n", ch );
         return;
      }

      send_to_char( "Unknown error during floodfill.\r\n", ch );
      return;
   }

   if( !str_cmp( arg1, "undo" ) )
   {
      int undo = unfloodfill(  );

      if( undo == 0 )
      {
         display_map( ch );
         send_to_char( "&RUndo successful.\r\n", ch );
         return;
      }

      send_to_char( "Unknown error during undo.\r\n", ch );
      return;
   }

   if( !str_cmp( arg1, "save" ) )
   {
      int map = -1;

      if( argument[0] == '\0' )
      {
         char *mapname;

         if( ch->map == -1 )
         {
            bug( "do_mapedit: %s is not on a valid map!", ch->name );
            send_to_char( "Can't do that - your on an invalid map.\r\n", ch );
            return;
         }
         mapname = map_name[ch->map];

         ch_printf( ch, "Saving map of %s....\r\n", mapname );
         save_map( mapname, ch->map );
         return;
      }

      if( !str_cmp( argument, "map1" ) )
         map = MAP_C1;

      if( !str_cmp( argument, "map2" ) )
         map = MAP_C2;

      if( !str_cmp( argument, "map3" ) )
         map = MAP_C3;

      if( map == -1 )
      {
         ch_printf( ch, "There isn't a map for '%s'.\r\n", arg1 );
         return;
      }

      ch_printf( ch, "Saving map of %s....", argument );
      save_map( argument, map );
      return;
   }

   if( !str_cmp( arg1, "sector" ) )
   {
      char arg2[MAX_INPUT_LENGTH];

      argument = one_argument( argument, arg2 );

      value = get_sectypes( arg2 );
      if( value < 0 || value > SECT_MAX )
      {
         send_to_char( "Invalid sector type.\r\n", ch );
         return;
      }

      if( !str_cmp( arg2, "exit" ) )
      {
         send_to_char( "You cannot place exits this way.\r\n", ch );
         send_to_char( "Please use the setexit command for this.\r\n", ch );
         return;
      }

      ch->pcdata->secedit = value;
      ch_printf( ch, "&YYou are now creating %s sectors.\r\n", arg2 );
      return;
   }

   send_to_char( "Usage: mapedit sector <sectortype>\r\n", ch );
   send_to_char( "Usage: mapedit save <mapname>\r\n", ch );
   send_to_char( "Usage: mapedit fill <sectortype>\r\n", ch );
   send_to_char( "Usage: mapedit undo\r\n", ch );
   send_to_char( "Usage: mapedit reload\r\n\r\n", ch );
   return;
}

/* Online OLC map editing stuff ends here */

/* Everything from here to the end is for compatibility with AFKMud modifications to stock code.
 * You might want to check through this stuff and see if you may already have some of it defined
 * in your code. Cause if you do and your compiler sees this, it may barf on you - and that isn't fun.
 */

/* Added by Samson 12-10-97. Takes sector name being input and returns the
   numerical value back. Called by do_redit when editing sector types.
   Also used by the Overland Map code.
 */
int get_sectypes( char *sector )
{
   int x;

   for( x = 0; x < SECT_MAX; x++ )
      if( !str_cmp( sector, sect_types[x] ) )
         return x;
   return -1;
}

/* Added by Samson 8-8-98. Takes area continent and returns numerical value for do_aset command */
int get_continent( char *continent )
{
   int x;

   for( x = 0; x < ACON_MAX; x++ )
      if( !str_cmp( continent, continents[x] ) )
         return x;
   return -1;
}

/* Bamfin parsing code by Altrag, installed by Samson 12-10-97
   Allows use of $n where the player's name would be */
char *bamf_print( char *fmt, CHAR_DATA * ch )
{
   static char buf[MAX_STRING_LENGTH];
   char *fp, *bp = buf;

   for( fp = strstr( fmt, "$n" ); fp; fmt = fp + 2, fp = strstr( fmt, "$n" ) )
      bp += snprintf( bp, MAX_STRING_LENGTH - ( bp - buf ), "%.*s%s", ( fp - fmt ), fmt, ch->name );
   strncpy( bp, fmt, MAX_STRING_LENGTH - ( bp - buf ) );
   return buf;
}

/*  Added atmob and atobj to reduce lag associated with at
 *  --Shaddai
 *  Modified for use with Overland maps by Samson
 */
void atmob( CHAR_DATA * ch, CHAR_DATA * wch, char *argument )
{
   ROOM_INDEX_DATA *location;
   ROOM_INDEX_DATA *original;
   int origmap, origx, origy;

   set_char_color( AT_IMMORT, ch );
   location = wch->in_room;

   if( is_ignoring( wch, ch ) )
   {
      send_to_char( "No such location.\r\n", ch );
      return;
   }

   if( room_is_private( location ) )
   {
      if( get_trust( ch ) < LEVEL_GREATER )
      {
         send_to_char( "That room is private right now.\r\n", ch );
         return;
      }
      else
      {
         send_to_char( "Overriding private flag!\r\n", ch );
      }
   }

   origmap = ch->map;
   origx = ch->x;
   origy = ch->y;

   /*
    * Bunch of checks to make sure the "ator" is temporarily on the same grid as
    * * the "atee" - Samson
    */

   if( IS_ROOM_FLAG( location, ROOM_MAP ) && !IS_PLR_FLAG( ch, PLR_ONMAP ) )
   {
      SET_PLR_FLAG( ch, PLR_ONMAP );
      ch->map = wch->map;
      ch->x = wch->x;
      ch->y = wch->y;
   }
   else if( IS_ROOM_FLAG( location, ROOM_MAP ) && IS_PLR_FLAG( ch, PLR_ONMAP ) )
   {
      ch->map = wch->map;
      ch->x = wch->x;
      ch->y = wch->y;
   }
   else if( !IS_ROOM_FLAG( location, ROOM_MAP ) && IS_PLR_FLAG( ch, PLR_ONMAP ) )
   {
      REMOVE_PLR_FLAG( ch, PLR_ONMAP );
      ch->map = -1;
      ch->x = -1;
      ch->y = -1;
   }

   set_char_color( AT_PLAIN, ch );
   original = ch->in_room;
   char_from_room( ch );
   char_to_room( ch, location );
   interpret( ch, argument );

   if( IS_PLR_FLAG( ch, PLR_ONMAP ) && !IS_ROOM_FLAG( original, ROOM_MAP ) )
      REMOVE_PLR_FLAG( ch, PLR_ONMAP );
   else if( !IS_PLR_FLAG( ch, PLR_ONMAP ) && IS_ROOM_FLAG( original, ROOM_MAP ) )
      SET_PLR_FLAG( ch, PLR_ONMAP );

   ch->map = origmap;
   ch->x = origx;
   ch->y = origy;

   if( !char_died( ch ) )
   {
      char_from_room( ch );
      char_to_room( ch, original );
   }
   return;
}

void atobj( CHAR_DATA * ch, OBJ_DATA * obj, char *argument )
{
   ROOM_INDEX_DATA *location;
   ROOM_INDEX_DATA *original;
   int origmap, origx, origy;

   set_char_color( AT_IMMORT, ch );
   location = obj->in_room;
   if( room_is_private( location ) )
   {
      if( get_trust( ch ) < LEVEL_GREATER )
      {
         send_to_char( "That room is private right now.\r\n", ch );
         return;
      }
      else
      {
         send_to_char( "Overriding private flag!\r\n", ch );
      }
   }

   origmap = ch->map;
   origx = ch->x;
   origy = ch->y;

   /*
    * Bunch of checks to make sure the imm is on the same grid as the object - Samson 
    */
   if( IS_ROOM_FLAG( location, ROOM_MAP ) && !IS_PLR_FLAG( ch, PLR_ONMAP ) )
   {
      SET_PLR_FLAG( ch, PLR_ONMAP );
      ch->map = obj->map;
      ch->x = obj->x;
      ch->y = obj->y;
   }
   else if( IS_ROOM_FLAG( location, ROOM_MAP ) && IS_PLR_FLAG( ch, PLR_ONMAP ) )
   {
      ch->map = obj->map;
      ch->x = obj->x;
      ch->y = obj->y;
   }
   else if( !IS_ROOM_FLAG( location, ROOM_MAP ) && IS_PLR_FLAG( ch, PLR_ONMAP ) )
   {
      REMOVE_PLR_FLAG( ch, PLR_ONMAP );
      ch->map = -1;
      ch->x = -1;
      ch->y = -1;
   }

   set_char_color( AT_PLAIN, ch );
   original = ch->in_room;
   char_from_room( ch );
   char_to_room( ch, location );
   interpret( ch, argument );

   if( IS_PLR_FLAG( ch, PLR_ONMAP ) && !IS_ROOM_FLAG( original, ROOM_MAP ) )
      REMOVE_PLR_FLAG( ch, PLR_ONMAP );
   else if( !IS_PLR_FLAG( ch, PLR_ONMAP ) && IS_ROOM_FLAG( original, ROOM_MAP ) )
      SET_PLR_FLAG( ch, PLR_ONMAP );

   ch->map = origmap;
   ch->x = origx;
   ch->y = origy;

   if( !char_died( ch ) )
   {
      char_from_room( ch );
      char_to_room( ch, original );
   }
   return;
}
