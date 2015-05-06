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

/*
 *     Known Bugs:
 *
 *     The code doesn't complie cleanly ;)
 *     the functions sector_by_number_ warn its sending a local varaible.
 *     Honestly, this does not affect the way this code works. Im sure
 *     Theres a better way to do this.
 *
 *     MAX_STRING in db.c may need to be incressed due to the ammount
 *     of data thats going to be sent to the wilderness. My wilderness
 *     is 100x75 rooms, and i needed to double MAX_STRING to get the mud
 *     to load. Your milage may vary depending on the size of the wilderness.
 *
 *     Sectors that aren't defined here, will show up with no name, and
 *     no description. Just define the Sectors ;)
 *
 *     -Thri
 */

/*
 *     Installation:
 *            This part is easy. First put set_wild.c and set_wild.h
 *     into your /src/ directory, add do_set_wilderness and do_set_wilderness_new
 *     to your interp.h and interp.c files. (make sure level is ML, you honestly
 *     don't want your low level builders mucking up your wilderness code,
 *     do_set_wilderness_new can be set fairly low, due to the fact it just
 *     sets the current room's name/description).
 *            Next, edit do_set_wilderness command to change the vnums
 *     to your wilderness's vnums.
 *            Now, to make this work correctly.. goto your wilderness
 *     (and backup your area directory..) and use the command do_set_wilderness
 *     Now, asave changed before doing ANY other command. Then copyover.
 *            Note: Do this with 0 players on.
 *     Now, after the copyover, check around for any errors.
 */

/*
 *     Expanding the code:
 *            This part is also easy. Just add new fiels to the tables
 *     below, and incresse the maxs in set_wild.h to fit the new descriptions.
 *     Adding a new sector type is a bit more of a pain. First you need 4 new
 *     tables and the defines for the tables (take a look how swamp was done)
 *     You'll need at LEAST 3 names/moods/terrain/wind descriptions, otherwise
 *     it becomes very repeative. Add them in to all the sector_by_number_
 *     functions (both the switch and the range at the top) then recomplie.
 *
 */

/*
 *     How it works:
 *            Basicly, the single room command, finds the sector_type of
 *     the room your currently in, selects a Name, a Mood line, a Terrain line
 *     and a wind line. It then sets the Name to the choosen name, and then
 *     plays with the mood/wind/terrain lines so that they're randomized in
 *     order, then spits out the final Name and description to the room.
 *     After all that, it then sets the area to be saved and formats
 *     the room description. Result: A random description for your wilderness.
 *
 */

/*
 *     To-Do:
 *            Support for more sectors.
 *            Lots of descriptions!
 *            Figure out how drey loaded the info from files.
 *            Re-work do_set_wilderness to support ANY area.
 *                   (ch->in_room->area->lvnum? shrug im lazy ;)
 *            Support to add objects/mobiles based on description.
 *                   (The room says theres a hawk flying around, where is it?)
 *                   (Oo some antlers.. fun ;)
 *            Figure out how to make get_sector commands to not output errors.
 *                   (if anyone knows how, send em to me)
 */

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mud.h"
#include "interp.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "magic.h"
#include "olc.h"
#include "set_wild.h"

void do_set_wilderness (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *room;
  int start, end, current;
  start = 20001;
  end = 28000;

  room = get_room_index (start);
/* initialize room for this check here */
  if (!IS_BUILDER (ch, room->area))
    {
      send_to_char ("do_set_wilderness: Insufficient Security", ch);
      return;
    }

  for (current = start; current <= end; current++)
    {

      room = get_room_index (current);
      char_to_room (ch, room);
      do_set_wilderness_new (ch, "");
      char_from_room (ch);

    }

  char_to_room (ch, room);

  return;
}


/*
 *	This command houses the full wilderness setting
 *	sentax: set_wild inside the area you wish to set descriptions on
 */

void do_set_wilderness_new	(CHAR_DATA * ch, char *argument)
{
	char name[MSL];	// Holds the room name
	char mood[MSL];	// Holds the mood description line
	char wind[MSL];	// Holds the wind description line
	char terrain[MSL];	// Holds the terrain description line
	char desc[MSL];	// Holds the final description to be used

	ROOM_INDEX_DATA *room;
	int vnum;

	vnum = (ch->in_room->vnum);
	room = get_room_index (vnum);

	// Find a name and description lines
	sprintf(name,    "%s", sector_by_number_name   (ch->in_room->sector_type));
	sprintf(wind,    "%s", sector_by_number_wind   (ch->in_room->sector_type));
	sprintf(mood,    "%s", sector_by_number_mood   (ch->in_room->sector_type));
	sprintf(terrain, "%s", sector_by_number_terrain(ch->in_room->sector_type));

	// Randomize description. Theres a better way to do this, i know it..
	switch(number_range(1, 6))
	{	default:
		case 1:
		sprintf(desc, "%s %s %s", wind, mood, terrain);
		break;

		case 2:
		sprintf(desc, "%s %s %s", wind, terrain, mood);
		break;

		case 3:
		sprintf(desc, "%s %s %s", mood, terrain, wind);
		break;

		case 4:
		sprintf(desc, "%s %s %s", mood, wind, terrain);
		break;

		case 5:
		sprintf(desc, "%s %s %s", terrain, mood, wind);
		break;

		case 6:
		sprintf(desc, "%s %s %s", terrain, wind, mood);
		break;
	}


	// Now that we have the Name, and final description, set the damn things
	free_string (room->name);
	room->name = str_dup (name);
	free_string (room->description);
	room->description = str_dup (desc);

	// Now lets format the description and set the area to be saved later
	format_string (room->description);
	SET_BIT (ch->in_room->area->area_flags, AREA_CHANGED);

	return;
}

/*
 *	Gives a name based on the Room sector type
 */

char * sector_by_number_name (int sector)
{
	char buf[MSL];
	int forest_name          = number_range(0, FOREST_NAME     -1);
	int swamp_name           = number_range(0, SWAMP_NAME      -1);
	int plains_name          = number_range(0, PLAINS_NAME     -1);
	int mountians_name       = number_range(0, MOUNTIANS_NAME  -1);
	int desert_name          = number_range(0, DESERT_NAME     -1);
	int shallow_water_name   = number_range(0, SHALLOW_NAME    -1);
	int deep_water_name      = number_range(0, DEEP_OCEAN_NAME -1);

	switch(sector)
	{	default:
		sprintf(buf, "Error");
		break;

		case SECT_FOREST:
		sprintf(buf, "%s", sector_forest_name_table[forest_name].name);
		break;

		case SECT_SWAMP:
		sprintf(buf, "%s", sector_swamp_name_table[swamp_name].name);
		break;

		case SECT_FIELD:
		sprintf(buf, "%s", sector_plains_name_table[plains_name].name);
		break;

		case SECT_MOUNTAIN:
		sprintf(buf, "%s", sector_mountians_name_table[mountians_name].name);
		break;

		case SECT_DESERT:
		sprintf(buf, "%s", sector_desert_name_table[desert_name].name);
		break;

		case SECT_WATER_SWIM:
		sprintf(buf, "%s", sector_shallow_water_name_table[shallow_water_name].name);
		break;

		case SECT_WATER_NOSWIM:
		sprintf(buf, "%s", sector_deep_water_name_table[deep_water_name].name);
		break;

	}

	return (buf);
}


/*
 *	Gives a Wind description line based on sector_type
 */

char * sector_by_number_wind (int sector)
{
	char buf[MSL];
	int forest_wind          = number_range(0, FOREST_WIND     -1);
	int swamp_wind           = number_range(0, SWAMP_WIND      -1);
	int plains_wind          = number_range(0, PLAINS_WIND     -1);
	int mountians_wind       = number_range(0, MOUNTIANS_WIND  -1);
	int desert_wind          = number_range(0, DESERT_WIND     -1);
	int shallow_water_wind   = number_range(0, SHALLOW_WIND    -1);
	int deep_water_wind      = number_range(0, DEEP_OCEAN_WIND -1);

	switch(sector)
	{	default:
		break;

		case SECT_FOREST:
		sprintf(buf, "%s", sector_forest_wind_table[forest_wind].name);
		break;

		case SECT_SWAMP:
		sprintf(buf, "%s", sector_swamp_wind_table[swamp_wind].name);
		break;

		case SECT_FIELD:
		sprintf(buf, "%s", sector_plains_wind_table[plains_wind].name);
		break;

		case SECT_MOUNTAIN:
		sprintf(buf, "%s", sector_mountians_wind_table[mountians_wind].name);
		break;

		case SECT_DESERT:
		sprintf(buf, "%s", sector_desert_wind_table[desert_wind].name);
		break;

		case SECT_WATER_SWIM:
		sprintf(buf, "%s", sector_shallow_water_wind_table[shallow_water_wind].name);
		break;

		case SECT_WATER_NOSWIM:
		sprintf(buf, "%s", sector_deep_water_wind_table[deep_water_wind].name);
		break;

	}

	return (buf);
}


/*
 *	Gives a Mood description line based on sector_type
 */

char * sector_by_number_mood (int sector)
{
	char buf[MSL];
	int forest_mood          = number_range(0, FOREST_MOOD     -1);
	int swamp_mood           = number_range(0, SWAMP_MOOD      -1);
	int plains_mood          = number_range(0, PLAINS_MOOD     -1);
	int mountians_mood       = number_range(0, MOUNTIANS_MOOD  -1);
	int desert_mood          = number_range(0, DESERT_MOOD     -1);
	int shallow_water_mood   = number_range(0, SHALLOW_MOOD    -1);
	int deep_water_mood      = number_range(0, DEEP_OCEAN_MOOD -1);

	switch(sector)
	{	default:
		break;

		case SECT_FOREST:
		sprintf(buf, "%s", sector_forest_mood_table[forest_mood].name);
		break;

		case SECT_SWAMP:
		sprintf(buf, "%s", sector_swamp_mood_table[swamp_mood].name);
		break;

		case SECT_FIELD:
		sprintf(buf, "%s", sector_plains_mood_table[plains_mood].name);
		break;

		case SECT_MOUNTAIN:
		sprintf(buf, "%s", sector_mountians_mood_table[mountians_mood].name);
		break;

		case SECT_DESERT:
		sprintf(buf, "%s", sector_desert_mood_table[desert_mood].name);
		break;

		case SECT_WATER_SWIM:
		sprintf(buf, "%s", sector_shallow_water_mood_table[shallow_water_mood].name);
		break;

		case SECT_WATER_NOSWIM:
		sprintf(buf, "%s", sector_deep_water_mood_table[deep_water_mood].name);
		break;

	}

	return (buf);
}


/*
 *	Gives a Terrain description line based on sector_type
 */

char * sector_by_number_terrain (int sector)
{
	char buf[MSL];
	int forest_terrain          = number_range(0, FOREST_TERRAIN     -1);
	int swamp_terrain           = number_range(0, SWAMP_TERRAIN      -1);
	int plains_terrain          = number_range(0, PLAINS_TERRAIN     -1);
	int mountians_terrain       = number_range(0, MOUNTIANS_TERRAIN  -1);
	int desert_terrain          = number_range(0, DESERT_TERRAIN     -1);
	int shallow_water_terrain   = number_range(0, SHALLOW_TERRAIN    -1);
	int deep_water_terrain      = number_range(0, DEEP_OCEAN_TERRAIN -1);

	switch(sector)
	{	default:
		break;

		case SECT_FOREST:
		sprintf(buf, "%s", sector_forest_terrain_table[forest_terrain].name);
		break;

		case SECT_SWAMP:
		sprintf(buf, "%s", sector_swamp_terrain_table[swamp_terrain].name);
		break;

		case SECT_FIELD:
		sprintf(buf, "%s", sector_plains_terrain_table[plains_terrain].name);
		break;

		case SECT_MOUNTAIN:
		sprintf(buf, "%s", sector_mountians_terrain_table[mountians_terrain].name);
		break;

		case SECT_DESERT:
		sprintf(buf, "%s", sector_desert_terrain_table[desert_terrain].name);
		break;

		case SECT_WATER_SWIM:
		sprintf(buf, "%s", sector_shallow_water_terrain_table[shallow_water_terrain].name);
		break;

		case SECT_WATER_NOSWIM:
		sprintf(buf, "%s", sector_deep_water_terrain_table[deep_water_terrain].name);
		break;

	}

	return (buf);
}

/* The following three tables house Forest Name/Terrain/Wind/Mood descriptions */
const struct sector_forest_name_type 	sector_forest_name_table[] =
{
	{"Pine Forest"},
	{"Forest Clearing"},
	{"Evergreen Grove"},
	{"Old Game Trail"},
};

const struct sector_forest_terrain_type 	sector_forest_terrain_table[] =
{
	{"The evergreens surround you on all sides, with only slight parts between some of them to allow a trail."},
	{"You crunch through pine needles fallen from the trees that surround you and hide the horizon from view."},
	{"The evergreens form an endless and monotonous landscape that seem to stretch out forever."},
	{"The trees of this pristine wilderness rise around and above you on all sides in a variety of greens and styles of needles."},
};

const struct sector_forest_wind_type 	sector_forest_wind_table[] =
{
	{"The wind carries the fragrant scent of the evergreens around you."},
	{"The pine needles on the forest floor stir slightly from the wind."},
	{"The wind here is a gentle breeze, which is due largely to the surround forest"},
	{"A sudden rise in the wind dips down from above and picks up some of the needles in a small whirlwind on the ground."},
};


const struct sector_forest_mood_type 	sector_forest_mood_table[] =
{
	{"Small little birds hop in and out of the trees here, gathering branches for their nests."},
	{"Little forest birds are gathering juniper berries here for their young."},
	{"The stillness is broken by the sound of the occassional pine cone falling to the floor."},
	{"Distant crunching noises reveal to you that other parties, probably animals, are also moving through this forest."},
};




const struct sector_swamp_name_type 	sector_swamp_name_table[] =
{
	{"Dank Swamp"},
	{"Swampy Ground"},
	{"Marshy Forest"},
	{"Muddy Path"},
	{"Inside the Bog"},
};

const struct sector_swamp_terrain_type 	sector_swamp_terrain_table[] =
{
	{"Maneuvering here is slightly difficult, due to the mud and water you're forced to slog through."},
	{"Each step you take is a difficult one, as the mud is reluctant to let go of your feet."},
	{"Grey and dismal, the swamp surrounds you in a forgettable haze of reeds, cattails, and standing pools of water, all wrapped up in thick, gooey mud."},
	{"The ground here is soft and oozing. You feel slimy water seeping into your boots."},
};

const struct sector_swamp_wind_type 	sector_swamp_wind_table[] =
{
	{"The air here is barely moving and does nothing to relieve you."},
	{"The occasional bit of wind sliughtly rustles the reeds and thrushes around you."},
	{"Dark clouds whip around you and the wind raises its voice and assails you."},
	{"A creaking beat fills the air, accompanied by groans and soughing of the air."},
	{"Snarls can be heard echoing as if from a great distance."},
};


const struct sector_swamp_mood_type 	sector_swamp_mood_table[] =
{
	{"Off to your right, a staggered but regular stream of bubbles appear on the surface of the water and then pop noisily, letting off an unpleasant smell in the process."},
	{"You can't decided which is worse here; the stench of the swamp and the relentless attacks of the mosquitos and black flies."},
	{"Occasionally you see a whole humanoid form floating face down in the water, but mostly you only find pieces."},
	{"Several large winged beasts flap over you and send chilling glares your way. To the East you see a large mountian range."},
	{"A creaking beat pulses through the air, leaving you with a cold sweat and fear in your heart."},
};



const struct sector_plains_name_type 	sector_plains_name_table[] =
{
	{"On some plains."},
	{"On the grassy wasteland."},
	{"Deep within the grassy northland"},
};

const struct sector_plains_terrain_type 	sector_plains_terrain_table[] =
{
	{"The grass here on the plains has grown up to your waist in height and impedes your progress somewhat."},
	{"The grassland extends further than the eye can see. The grass is yellowed from lack of water."},
	{"Small mana bushes grow here, sticking out of the sea of grass like a sore thumb."},
	{"Patchs of dirt can be seen. The grass here is more brown than yellow here."},
};

const struct sector_plains_wind_type 	sector_plains_wind_table[] =
{
	{"The winds sweep across the plain and move the long grass in endless waves."},
	{"The winds whistle as it passes through the dying grass."},
	{"The wind stops briefly, only to start agian, stronger than before."},
	{"A few tree stumps are the only landmarks here."},
};


const struct sector_plains_mood_type 	sector_plains_mood_table[] =
{
	{"Small grasshoppers are jumping in swarms through the grass."},
	{"A small rodent pops its head out from a hole in the ground and quickly darts back in."},
	{"To the far west you can see what appears to be greyish mountians."},
	{"Some red flowers grow here."},
	{"A log is on the ground here, providing shelter for the small creatues that live here."},
};


const struct sector_mountians_name_type 	sector_mountians_name_table[] =
{
	{"On a rocky mountian range."},
	{"A mountian range."},
	{"Along a path in the mountians."},
	{"Deep within grey mountians."},
	{"A rocky path within the mountians"},
};

const struct sector_mountians_terrain_type 	sector_mountians_terrain_table[] =
{
	{"A baren waste. Surrounded by giant boulders, the mountians are empty compared to the valleys below."},
	{"You kick up some dust as your feet hit a small pile of gravel."},
	{"A massive grey boulder sits here."},
	{"A dried river once flowed through here."},
};

const struct sector_mountians_wind_type 	sector_mountians_wind_table[] =
{
	{"The sun shines brightly here, causing waves of heat to rise from the boulders."},
	{"A small gust of wind blows past you, but it is not enough to cool you from the scorching heat."},
	{"A strong burst of wind rocks a large grey boulder side to side."},
};


const struct sector_mountians_mood_type 	sector_mountians_mood_table[] =
{
	{"A Hawk soars high above."},
	{"The mountians seem to go on forever in every direction."},
	{"The pitter patter of little feet running across the scorching rock can be heard in the distance."},
	{"The sound of falling rocks can be heard."},
};


const struct sector_desert_name_type 	sector_desert_name_table[] =
{
	{"On a sand dune."},
	{"Surrounded by sand dunes."},
	{"Deep within the endless sand dunes"},
	{"Inside a scorching hot desert."},
	{"In the endless sand desert."},
	{"Drifting Sands."},
};

const struct sector_desert_terrain_type 	sector_desert_terrain_table[] =
{
	{"The sand here is very hot to the touch. The scorching sun makes you sweat."},
	{"The sand dunes here are huge, some are even unclimbable. "},
	{"Specks of white and black sand stand out aginst the sea of yellow sand that surrounds you."},
	{"Small blackened rocks are scattered around this part of the desert."},
	{"A few jagged rock outcropings can be seen in the distance."},
};

const struct sector_desert_wind_type 	sector_desert_wind_table[] =
{
	{"A gust of wind picks up some sand and tosses it all around you."},
	{"The scorching heat from the sun bares down on the desert."},
	{"The still wind tortures you as you traverse the sand dunes."},
	{"A cool breeze can be felt, but only for a moment."},
	{"The winds that race across this sandy desert do little to cool you since the air itself is so hot."},
	{"For a moment the sweat gathers on your brow, then the winds pick up again and leave you dry but not cooled."},
};


const struct sector_desert_mood_type 	sector_desert_mood_table[] =
{
	{"A small red scorpian scurrys along the sand dunes, looking for a rock to hide under."},
	{"The sun-bleached bones of some traveller less fortunate than yourself are buried here under the shifting sands."},
	{"A few small stones, bleached white by the sun, peek out of the sand here."},
	{"The shifting sands have uncovered a low stone wall, left over from some long forgotten home."},
	{"A few pebbles, bleached white by the sun, peek out of the sand here."},
	{"The leg of a an old stone statue, far too large or heavy to move, stands buried in the sands."},
	{"A small rock, bleached white by the sun, peek out of the sand here."},
	{"An old wooden wagon wheel juts out of the sand here."},
};

const struct sector_shallow_water_name_type 	sector_shallow_water_name_table[] =
{
	{"In some shallow water."},
	{"Shallow water."},
	{"Light blue waters."},
};

const struct sector_shallow_water_terrain_type 	sector_shallow_water_terrain_table[] =
{
	{"The water here is very shallow, only three to four feet deep."},
	{"The beautiful light blue water shimmers aginst the setting sun."},
	{"A few small waves brush up aginst you."},
};

const struct sector_shallow_water_wind_type 	sector_shallow_water_wind_table[] =
{
	{"The cool ocean breeze feels good aginst your face."},
	{"The breeze creates tiny little waves in the water here."},
	{"A strong burst of wind is followed by a large wave, which almost overtakes you."},
};


const struct sector_shallow_water_mood_type 	sector_shallow_water_mood_table[] =
{
	{"A small fish jumps out of the water just out of arms reach."},
	{"Thousands of colourful fish can be seen swimming around the coral reef here."},
	{"The water here is filled with ocean life."},
};


const struct sector_deep_water_name_type 	sector_deep_water_name_table[] =
{
	{"On a sea"},
	{"In the ocean"},
	{"Surrounded by the blue ocean."},
};

const struct sector_deep_water_terrain_type 	sector_deep_water_terrain_table[] =
{
	{"The waves here are much larger than those near the shore."},
	{"The ocean stretches out for miles in every direction."},
	{"As the sun sets, the deep blue ocean turns a dark orange."},
	// Note: Yes i know the sun only turns orange because of
	// Polution, BUT.. Its cool anyhow. -Thri
};

const struct sector_deep_water_wind_type 	sector_deep_water_wind_table[] =
{
	{"The winds here are extremely strong, and create some very impressive waves."},
	{"The wind here is but a weak summer breeze."},
	{"The cool tropical winds here feel good aginst your skin."},
};


const struct sector_deep_water_mood_type 	sector_deep_water_mood_table[] =
{
	{"In the distance, you can see a whale touching the surface."},
	{"A few dolphins playfuly jump around here."},
	{"Dark clouds can be seen in the distance."},
};
