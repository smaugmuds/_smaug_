/**********************************************************************
 *	Wilderness Set snipplet (C) 2004 by Zachery Crosswhite.        *
 *     Snipplet writen for SoulblightMUD.                             *
 *     telnet://soulblight.slayn.net:5500 (5501)                      *
 *     http://soulblight.slayn.net                                    *
 *     Email: cyhawk@comcast.net                                      *
 *     ICQ: 5335308                                                   *
 *     AIM: CalibanL                                                  *
 *                                                                    *
 *     Also check out 'The Void' a website devoted to area exchange   *
 *     and building. Located at http://void.slayn.net                 *
 *                                                                    *
 *     Orginal Idea by Drey of Underworld dreams.                     *
 *     http://dreams.daestroke.com                                    *
 *                                                                    *
 *     Credit also goes to mlkesl@stthomas.edu for creating           *
 *     the Ascii Automapper utility for which I based this code       *
 *     on, and to be used with.                                       *
 *                                                                    *
 *     The command do_set_wilderness is a re-work of mlk's            *
 *     do_set_wild function in his Ascii Automapper utility.          *
 *                                                                    *
 *     In order to use any part of this snipplet, you must            *
 *     Comply with the Orginal Diku/Merc and Rom lience agreements    *
 *     in 'licence.doc' and 'licence.txt'. You may NOT remove this    *
 *     copyright notice from this file.                               *
 *                                                                    *
 *     In order to use this code, you must also create a helpfile     *
 *     with the following information. (Just copy/paste it)           *
 *                                                                    *
 *     -1 SET_WILD~                                                   *
 *     .                                                              *
 *        Random Descriptions for wilderness area code, created by    *
 *        Thri of Soulblight Mud. (http://soulblight.slayn.net).      *
 *     ~                                                              *
 *                                                                    *
 *                                                                    *
 *     Several hours have gone into this code you are benfiting from. *
 *     I encourage you to share your work with others as well.        *
 *     Remember, what comes around, goes around.                      *
 *                                                                    *
 **********************************************************************/

#define FOREST_NAME		4
#define FOREST_MOOD		4
#define FOREST_TERRAIN	4
#define FOREST_WIND		4

#define SWAMP_NAME		5
#define SWAMP_TERRAIN	4
#define SWAMP_WIND		5
#define SWAMP_MOOD		5

#define PLAINS_NAME		3
#define PLAINS_TERRAIN	4
#define PLAINS_WIND		4
#define PLAINS_MOOD		5

#define MOUNTIANS_NAME	5
#define MOUNTIANS_TERRAIN	4
#define MOUNTIANS_WIND	3
#define MOUNTIANS_MOOD	4

#define DESERT_NAME		6
#define DESERT_TERRAIN	5
#define DESERT_WIND		6
#define DESERT_MOOD		8

#define SHALLOW_NAME		3
#define SHALLOW_TERRAIN	3
#define SHALLOW_WIND		3
#define SHALLOW_MOOD		3

#define DEEP_OCEAN_NAME	3
#define DEEP_OCEAN_TERRAIN	3
#define DEEP_OCEAN_WIND	3
#define DEEP_OCEAN_MOOD	3

char * sector_by_number_name 	args ((int sector));
char * sector_by_number_wind 	args ((int sector));
char * sector_by_number_mood 	args ((int sector));
char * sector_by_number_terrain 	args ((int sector));


extern    const    struct    sector_swamp_name_type      	sector_swamp_name_table      	[];
extern    const    struct    sector_swamp_mood_type      	sector_swamp_mood_table      	[];
extern    const    struct    sector_swamp_terrain_type      	sector_swamp_terrain_table      	[];
extern    const    struct    sector_swamp_wind_type      	sector_swamp_wind_table      	[];
extern    const    struct    sector_forest_name_type      	sector_forest_name_table      	[];
extern    const    struct    sector_forest_mood_type      	sector_forest_mood_table      	[];
extern    const    struct    sector_forest_terrain_type      	sector_forest_terrain_table      	[];
extern    const    struct    sector_forest_wind_type      	sector_forest_wind_table      	[];
extern    const    struct    sector_plains_name_type      	sector_plains_name_table      	[];
extern    const    struct    sector_plains_mood_type      	sector_plains_mood_table      	[];
extern    const    struct    sector_plains_terrain_type      	sector_plains_terrain_table      	[];
extern    const    struct    sector_plains_wind_type      	sector_plains_wind_table      	[];
extern    const    struct    sector_mountians_name_type      	sector_mountians_name_table      	[];
extern    const    struct    sector_mountians_mood_type      	sector_mountians_mood_table      	[];
extern    const    struct    sector_mountians_terrain_type     sector_mountians_terrain_table     [];
extern    const    struct    sector_mountians_wind_type      	sector_mountians_wind_table      	[];
extern    const    struct    sector_desert_name_type      	sector_desert_name_table      	[];
extern    const    struct    sector_desert_mood_type      	sector_desert_mood_table      	[];
extern    const    struct    sector_desert_terrain_type      	sector_desert_terrain_table      	[];
extern    const    struct    sector_desert_wind_type      	sector_desert_wind_table      	[];
extern    const    struct    sector_shallow_water_name_type    sector_shallow_water_name_table    [];
extern    const    struct    sector_shallow_water_mood_type    sector_shallow_water_mood_table    [];
extern    const    struct    sector_shallow_water_terrain_type sector_shallow_water_terrain_table [];
extern    const    struct    sector_shallow_water_wind_type    sector_shallow_water_wind_table    [];
extern    const    struct    sector_deep_water_name_type      	sector_deep_water_name_table      	[];
extern    const    struct    sector_deep_water_mood_type      	sector_deep_water_mood_table      	[];
extern    const    struct    sector_deep_water_terrain_type    sector_deep_water_terrain_table     [];
extern    const    struct    sector_deep_water_wind_type      	sector_deep_water_wind_table      	[];


struct sector_forest_name_type
{
	char * 	name;
};

struct sector_forest_wind_type
{
	char * 	name;
};

struct sector_forest_mood_type
{
	char * 	name;
};

struct sector_forest_terrain_type
{
	char * 	name;
};

struct sector_swamp_name_type
{
	char * 	name;
};

struct sector_swamp_wind_type
{
	char * 	name;
};

struct sector_swamp_mood_type
{
	char * 	name;
};

struct sector_swamp_terrain_type
{
	char * 	name;
};


struct sector_plains_name_type
{
	char * 	name;
};

struct sector_plains_wind_type
{
	char * 	name;
};

struct sector_plains_mood_type
{
	char * 	name;
};

struct sector_plains_terrain_type
{
	char * 	name;
};


struct sector_mountians_name_type
{
	char * 	name;
};

struct sector_mountians_wind_type
{
	char * 	name;
};

struct sector_mountians_mood_type
{
	char * 	name;
};

struct sector_mountians_terrain_type
{
	char * 	name;
};


struct sector_desert_name_type
{
	char * 	name;
};

struct sector_desert_wind_type
{
	char * 	name;
};

struct sector_desert_mood_type
{
	char * 	name;
};

struct sector_desert_terrain_type
{
	char * 	name;
};

struct sector_shallow_water_name_type
{
	char * 	name;
};

struct sector_shallow_water_wind_type
{
	char * 	name;
};

struct sector_shallow_water_mood_type
{
	char * 	name;
};

struct sector_shallow_water_terrain_type
{
	char * 	name;
};

struct sector_deep_water_name_type
{
	char * 	name;
};

struct sector_deep_water_wind_type
{
	char * 	name;
};

struct sector_deep_water_mood_type
{
	char * 	name;
};

struct sector_deep_water_terrain_type
{
	char * 	name;
};


