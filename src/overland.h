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

#define OVERLANDCODE /* Interaction with other snippets */

/* Uncomment this if you have the Dragonflight Module installed */
// #include "dragonflight.h" /* Data for the Overland Dragonflight Module */

#define OVERLAND_MAP1 30000
#define OVERLAND_MAP2 30100
#define OVERLAND_MAP3 30200

/* Adjust these to fit the size of the maps you want for your world */
#define MAX_X 1000
#define MAX_Y 1000

/* Change these filenames to match yours */
#define MAP_DIR "../maps/"
#define ENTRANCE_FILE "entrances.dat"
#define LANDMARK_FILE "landmarks.dat"
#define MAPRESET_FILE "mapresets.dat"

/* Adjust these to suit your world */
typedef enum
{
   MAP_C1, MAP_C2, MAP_C3, MAP_MAX
} map_types;

typedef enum
{
   TYPE_OBJECT, TYPE_MOBILE
} mapreset_types;

/* 3 maps, each of 1000x1000 rooms, starting from 0,0 as the NW corner */
extern unsigned char map_sector[MAP_MAX][MAX_X][MAX_Y];  /* Initializes the sector array */
extern char *const map_names[];
extern char *const map_name[];
extern char *const continents[];
extern char *const sect_types[];

typedef struct entrance_data ENTRANCE_DATA;
typedef struct landmark_data LANDMARK_DATA;
typedef struct mapreset_data MAPRESET_DATA;

extern ENTRANCE_DATA *first_entrance;
extern ENTRANCE_DATA *last_entrance;
extern LANDMARK_DATA *first_landmark;
extern LANDMARK_DATA *last_landmark;
extern MAPRESET_DATA *first_mapreset;
extern MAPRESET_DATA *last_mapreset;

struct mapreset_data
{
   MAPRESET_DATA *next;
   MAPRESET_DATA *prev;
   int type;   /* Reset a mob or object */
   int vnum;   /* Vnum of object OR mobile */
   short map;  /* Map for the reset */
   short x; /* X coordinate of reset */
   short y; /* Y coordinate of reset */
};

struct landmark_data
{
   LANDMARK_DATA *next;
   LANDMARK_DATA *prev;
   char *description;   /* Description of the landmark */
   int distance;  /* Distance the landmark is visible from */
   short map;  /* Map the landmark is on */
   short x; /* X coordinate of landmark */
   short y; /* Y coordinate of landmark */
   bool Isdesc;   /* If true is room desc. If not is landmark */
};

struct entrance_data
{
   ENTRANCE_DATA *next;
   ENTRANCE_DATA *prev;
   char *area; /* Area name */
   int vnum;   /* Target vnum if it goes to a regular zone */
   short herex;   /* Coordinates the entrance is at */
   short herey;
   short therex;  /* Coordinates the entrance goes to, if any */
   short therey;
   short tomap;   /* Map it goes to, if any */
   short onmap;   /* Which map it's on */
   short prevsector; /* Previous sector type to restore with when an exit is deleted */
};

struct sect_color_type
{
   short sector;  /* Terrain sector */
   char *color;   /* Color to display as */
   char *symbol;  /* Symbol you see for the sector */
   char *desc; /* Description of sector type */
   bool canpass;  /* Impassable terrain */
   int move;   /* Movement loss */
   short graph1;  /* Color numbers for graphic conversion */
   short graph2;
   short graph3;
};

extern const struct sect_color_type sect_show[];

DECLARE_DO_FUN( do_mapedit ); /* Map OLC function */
DECLARE_DO_FUN( do_setexit ); /* Map OLC support to handle exits */
DECLARE_DO_FUN( do_setmark ); /* Map OLC support to handle landmarks */
DECLARE_DO_FUN( do_mreset );  /* Map OLC support to handle resets */
DECLARE_DO_FUN( do_coords );  /* To jump to different coordinates on the map */
DECLARE_DO_FUN( do_survey );  /* The overland survey command */
DECLARE_DO_FUN( do_landmarks );  /* Command to list landmarks */
DECLARE_DO_FUN( do_mapresets );  /* Command to list map resets */

void enter_map( CHAR_DATA * ch, int x, int y, int continent );
void leave_map( CHAR_DATA * ch, CHAR_DATA * victim, ROOM_INDEX_DATA * target );
void display_map( CHAR_DATA * ch );
ch_ret process_exit( CHAR_DATA * ch, short map, short x, short y, int dir );
void collect_followers( CHAR_DATA * ch, ROOM_INDEX_DATA * from, ROOM_INDEX_DATA * to );
bool is_same_map( CHAR_DATA * ch, CHAR_DATA * victim );
void fix_maps( CHAR_DATA * ch, CHAR_DATA * victim );
short get_terrain( short map, short x, short y );

/* Everything from here to the end is for compatibility with AFKMud modifications to stock code */
typedef enum
{
   ACON_C1, ACON_C2, ACON_C3, ACON_MAX
} acon_types;

/* Sunrise and sunset times according to a standard 24hr clock - adjust as needed */
#define HOUR_SUNRISE 6
#define HOUR_NOON 12
#define HOUR_SUNSET 18
#define HOUR_MIDNIGHT 24

/* Simplified indoor sector check */
#define INDOOR_SECTOR(sect) (  sect == SECT_INSIDE || 	           \
				       sect == SECT_UNDERWATER ||               \
                               sect == SECT_OCEANFLOOR ||               \
                               sect == SECT_UNDERGROUND )

/* Flag macros taken from DOTD codebase */
#define IS_PLR_FLAG(var, bit)          ( !IS_NPC(var) && xIS_SET((var)->act, (bit)) )
#define SET_PLR_FLAG(var, bit)         xSET_BIT((var)->act, (bit))
#define REMOVE_PLR_FLAG(var, bit)      xREMOVE_BIT((var)->act, (bit))
#define IS_ACT_FLAG(var, bit)          ( IS_NPC(var) && xIS_SET((var)->act, (bit)) )
#define SET_ACT_FLAG(var, bit)         xSET_BIT((var)->act, (bit))
#define REMOVE_ACT_FLAG(var, bit)      xREMOVE_BIT((var)->act, (bit))
#define SET_OBJ_STAT(obj,stat)     	   (xSET_BIT((obj)->extra_flags, (stat)))
#define REMOVE_OBJ_STAT(obj,stat)      (xREMOVE_BIT((obj)->extra_flags, (stat)))
#define IS_ROOM_FLAG(var, bit)         IS_SET((var)->room_flags, (bit))
#define IS_EXIT_FLAG(var, bit)         IS_SET((var)->exit_info, (bit))
#define SET_EXIT_FLAG(var, bit)        SET_BIT((var)->exit_info, (bit))
#define REMOVE_EXIT_FLAG(var, bit)     REMOVE_BIT((var)->exit_flags, (bit))
#define GET_MOVE(ch)            ((ch)->move)

/* Function protos for needed support functions */
char *bamf_print( char *fmt, CHAR_DATA * ch );
int get_sectypes( char *sector );
void atmob( CHAR_DATA * ch, CHAR_DATA * wch, char *argument );
void atobj( CHAR_DATA * ch, OBJ_DATA * obj, char *argument );
int get_continent( char *continent );
