/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops, Fireblade, Edmond, Conran                         |             *
 * ------------------------------------------------------------------------ *
 *		       Online Building and Editing Module		    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"



#define MOB 0
#define OBJ 1
#define ROOM 2
#define CONTROL 3

#define NO_COMMAND "there_is_to_be_no_command_associated_with_this_element"

/*
 * Externals
 */
int get_npc_race( char *type );

/*
 * Locals
 */

void            refresh_page(CHAR_DATA * ch);
char           *get_ris(MOB_INDEX_DATA * idx, int risFlag);
void            fill_in_mob_page(CHAR_DATA * ch, MOB_INDEX_DATA * idx, char page);
void            fill_in_obj_page(CHAR_DATA * ch, OBJ_INDEX_DATA * idx, char page);
void            fill_in_room_page(CHAR_DATA * ch, ROOM_INDEX_DATA * idx, char page);
void            fill_in_control_page(CHAR_DATA * ch, char page);
void            send_page_to_char(CHAR_DATA * ch, MOB_INDEX_DATA * idx, char page);
void            send_room_page_to_char(CHAR_DATA * ch, ROOM_INDEX_DATA * idx, char page);
void            send_control_page_to_char(CHAR_DATA * ch, char page);
void            send_obj_page_to_char(CHAR_DATA * ch, OBJ_INDEX_DATA * idx, char page);
int             get_affect(OBJ_INDEX_DATA * obj, int type);
int             get_bvl(int bitvector);
void            format_string(char output[], char *str, int length);
char 		*get_item_strings( int item_number, int which_string );

static char    *const space = " ";
static char    *const check = "X";

MENU_DATA       room_help_page_data[] =
{
{ "-", "a", 13, 2, "", NULL, STRING, 0, "rmenu a" },
{ "-", "b", 13, 2, "", NULL, STRING, 0, "rmenu b" },
{ "-", "c", 13, 2, "", NULL, STRING, 0, "rmenu c" },
{ "-", "h", 13, 2, "", NULL, STRING, 0, "rmenu h" },
{ " ", " ", 0, 0, " ", NULL, (int) NULL, 0, NULL }
};

MENU_DATA       mob_help_page_data[] =
{
{ "-", "a", 13, 2, "", NULL, STRING, 1, "mmenu %s a" },
{ "-", "b", 13, 2, "", NULL, STRING, 1, "mmenu %s b" },
{ "-", "c", 13, 2, "", NULL, STRING, 1, "mmenu %s c" },
{ "-", "d", 13, 2, "", NULL, STRING, 1, "mmenu %s d" },
{ "-", "e", 13, 2, "", NULL, STRING, 1, "mmenu %s e" },
{ "-", "f", 13, 2, "", NULL, STRING, 1, "mmenu %s f" },
{ "-", "h", 13, 2, "", NULL, STRING, 1, "mmenu %s h" },
{ " ", " ", 0, 0, " ", NULL, (int) NULL, 0, NULL }
};

MENU_DATA       obj_help_page_data[] =
{
{ "-", "a", 13, 2, "", NULL, STRING, 1, "omenu %s a" },
{ "-", "b", 13, 2, "", NULL, STRING, 1, "omenu %s b" },
{ "-", "c", 13, 2, "", NULL, STRING, 1, "omenu %s c" },
{ "-", "d", 13, 2, "", NULL, STRING, 1, "omenu %s d" },
{ "-", "e", 13, 2, "", NULL, STRING, 1, "omenu %s e" },
{ "-", "h", 13, 2, "", NULL, STRING, 1, "omenu %s h" },
{ " ", " ", 0, 0, " ", NULL, (int) NULL, 0, NULL }
};

MENU_DATA       control_help_page_data[] =
{
{ "-", "a", 13, 2, "", NULL, STRING, 0, "cmenu a" },
{ "-", "h", 13, 2, "", NULL, STRING, 0, "cmenu h" },
{ " ", " ", 0, 0, " ", NULL, (int) NULL, 0, NULL }
};

MENU_DATA       room_page_d_data[] =
{{"1", "a", 2, 24, "%1.1s", NULL, STRING, 0, "redit default_room hut"},
{"1", "b", 2, 42, "%1.1s", NULL, STRING, 0, "redit default_room tent"},
{ "1", "c", 2, 57, "%1.1s", NULL, STRING, 0, "redit default_room hovel" },
{ "1", "d", 2, 72, "%1.1s", NULL, STRING, 0, "redit default_room campsite" },
{ "1", "e", 3, 24, "%1.1s", NULL, STRING, 0, "redit default_room shack" }, 
{ "1", "f", 3, 42, "%1.1s", NULL, STRING, 0, "redit default_room cabin" }, 
{ "1", "g", 3, 57, "%1.1s", NULL, STRING, 0, "redit default_room homestead"},
{ "1", "h", 3, 72, "%1.1s", NULL, STRING, 0, "redit default_room keep" },
{ "1", "i", 4, 24, "%1.1s", NULL, STRING, 0, "redit default_room fortress" },
{ "1", "j", 4, 42, "%1.1s", NULL, STRING, 0, "redit default_room castle" },
{ "1", "k", 4, 57, "%1.1s", NULL, STRING, 0, "redit default_room palace" },
{ "1", "l", 4, 72, "%1.1s", NULL, STRING, 0, "redit default_room temple" },
{ "1", "m", 5, 24, "%1.1s", NULL, STRING, 0, "redit default_room store" },
{ "1", "n", 5, 42, "%1.1s", NULL, STRING, 0, "redit default_room graveyard"},

{
	"1", "q", 6, 24, "%1.1s", NULL, STRING, 0, "redit default_room basement"
},
{
	"1", "r", 6, 42, "%1.1s", NULL, STRING, 0, "redit default_room bedroom"
},
{
	"1", "s", 6, 57, "%1.1s", NULL, STRING, 0, "redit default_room attic"
},
{
	"1", "t", 6, 72, "%1.1s", NULL, STRING, 0, "redit default_room workroom"
},

{
	"1", "u", 7, 24, "%1.1s", NULL, STRING, 0, "redit default_room cellar"
},
{
	"1", "v", 7, 42, "%1.1s", NULL, STRING, 0, "redit default_room banquetroom"
},
{
	"1", "w", 7, 57, "%1.1s", NULL, STRING, 0, "redit default_room tower"
},
{
	"1", "x", 7, 72, "%1.1s", NULL, STRING, 0, "redit default_room workshop"
},

{
	"1", "y", 8, 24, "%1.1s", NULL, STRING, 0, "redit default_room vault"
},
{
	"1", "z", 8, 42, "%1.1s", NULL, STRING, 0, "redit default_room sitting_room"
},
{
	"1", "A", 8, 57, "%1.1s", NULL, STRING, 0, "redit default_room turret"
},
{
	"1", "B", 8, 72, "%1.1s", NULL, STRING, 0, "redit default_room lab"
},

{
	"1", "C", 9, 24, "%1.1s", NULL, STRING, 0, "redit default_room crypt"
},
{
	"1", "D", 9, 42, "%1.1s", NULL, STRING, 0, "redit default_room kitchen"
},
{
	"1", "E", 9, 57, "%1.1s", NULL, STRING, 0, "redit default_room corridor"
},
{
	"1", "F", 9, 72, "%1.1s", NULL, STRING, 0, "redit default_room library"
},

{
	"1", "G", 10, 24, "%1.1s", NULL, STRING, 0, "redit default_room storeroom"
},
{
	"1", "H", 10, 42, "%1.1s", NULL, STRING, 0, "redit default_room office"
},
{
	"1", "I", 10, 57, "%1.1s", NULL, STRING, 0, "redit default_room passage"
},
{
	"1", "J", 10, 72, "%1.1s", NULL, STRING, 0, "redit default_room treasury"
},

{
	"1", "K", 11, 24, "%1.1s", NULL, STRING, 0, "redit default_room closet"
},
{
	"1", "L", 11, 42, "%1.1s", NULL, STRING, 0, "redit default_room drawing_room"
},
{
	"1", "M", 11, 57, "%1.1s", NULL, STRING, 0, "redit default_room hallway"
},
{
	"1", "N", 11, 72, "%1.1s", NULL, STRING, 0, "redit default_room ladder"
},

{
	"1", "O", 12, 24, "%1.1s", NULL, STRING, 0, "redit default_room foyer"
},
{
	"1", "P", 12, 42, "%1.1s", NULL, STRING, 0, "redit default_room arboretum"
},
{
	"1", "Q", 12, 57, "%1.1s", NULL, STRING, 0, "redit default_room stairway"
},
{
	"1", "R", 12, 72, "%1.1s", NULL, STRING, 0, "redit default_room vent"
},

{
	"1", "S", 13, 24, "%1.1s", NULL, STRING, 0, "redit default_room entrance"
},
{
	"1", "T", 13, 42, "%1.1s", NULL, STRING, 0, "redit default_room larder"
},
{
	"1", "U", 13, 57, "%1.1s", NULL, STRING, 0, "redit default_room landing"
},
{
	"1", "V", 13, 72, "%1.1s", NULL, STRING, 0, "redit default_room dumbwaiter"
},

{
	"1", "W", 14, 24, "%1.1s", NULL, STRING, 0, "redit default_room gate"
},
{
	"1", "X", 14, 42, "%1.1s", NULL, STRING, 0, "redit default_room chamber"
},
{
	"1", "Y", 14, 57, "%1.1s", NULL, STRING, 0, "redit default_room balcony"
},
{
	"1", "Z", 14, 72, "%1.1s", NULL, STRING, 0, "redit default_room shaft"
},

{
	"1", "2", 15, 42, "%1.1s", NULL, STRING, 0, "redit default_room conservatory"
},
{
	"1", "4", 15, 72, "%1.1s", NULL, STRING, 0, "redit default_room chimney"
},

{
	"1", "5", 16, 24, "%1.1s", NULL, STRING, 0, "redit default_room well"
},
{ "1", "6", 16, 42, "%1.1s", NULL, STRING, 0, "redit default_room garden" }, { "1", "7", 16, 57, "%1.1s", NULL, STRING, 0, "redit default_room cave" },
{ "1", "8", 16, 72, "%1.1s", NULL, STRING, 0, "redit default_room mine" },
{ "1", "9", 17, 24, "%1.1s", NULL, STRING, 0, "redit default_room grotto" },
/*
{ "1", "0", 17, 42, "%1.1s", NULL, STRING, 0, "redit default_room tunnel" }, { "1", "$", 17, 57, "%1.1s", NULL, STRING, 0, "redit default_room maze" },
*/
{ "-", "a", 13, 2, "", NULL, STRING, 0, "rmenu a" },
{ "-", "b", 13, 2, "", NULL, STRING, 0, "rmenu b" },
{ "-", "c", 13, 2, "", NULL, STRING, 0, "rmenu c" },
{ "-", "h", 13, 2, "", NULL, STRING, 0, "rmenu h" },
{
	" ", " ", 0, 0, " ", NULL, (int) NULL, 0, NULL
}
};

MENU_DATA       room_page_c_data[] =
{{
		"1", "n", 2, 52, "%1.1s", NULL, STRING, 0, "redit substate north"
},
{
	"1", "u", 2, 70, "%1.1s", NULL, STRING, 0, "redit substate up"
},
{
	"1", "w", 3, 43, "%1.1s", NULL, STRING, 0, "redit substate east"
},
{
	"1", "e", 3, 61, "%1.1s", NULL, STRING, 0, "redit substate west"
},
{
	"1", "s", 4, 52, "%1.1s", NULL, STRING, 0, "redit substate south"
},
{
	"1", "d", 4, 70, "%1.1s", NULL, STRING, 0, "redit substate down"
},
{
	"2", "a", 6, 25, "%1.1s", NULL, STRING, 0, "redit ex_flags isdoor"
},
{
	"2", "b", 6, 41, "%1.1s", NULL, STRING, 0, "redit ex_flags secret"
},
{
	"2", "c", 7, 25, "%1.1s", NULL, STRING, 0, "redit ex_flags closed"
},
{
	"2", "d", 7, 41, "%1.1s", NULL, STRING, 0, "redit ex_flags pickproof"
},
{
	"2", "e", 8, 25, "%1.1s", NULL, STRING, 0, "redit ex_flags locked"
},
{
	"2", "f", 8, 41, "%1.1s", NULL, STRING, 0, "redit ex_flags nopassdoor"
},
{
	"2", "i", 10, 14, "%5.5hd", NULL, SH_INT, 1, "redit ex_to_room %s"
},
{
	"2", NO_COMMAND
	,10, 22, "%-19.19s", NULL, STRING, 0, NO_COMMAND
},
{
	"2", "j", 11, 14, "%5.5hd", NULL, SH_INT, 1, "redit ex_key %s"
},
{ "2", NO_COMMAND ,11, 22, "%-19.19s", NULL, STRING, 0, NO_COMMAND
},
{ "2", "k", 13, 2, "%-70.70s", NULL, STRING, 1, "redit ex_exdesc %s" },
{ "-", "a", 13, 2, "", NULL, STRING, 0, "rmenu a" },
{ "-", "b", 13, 2, "", NULL, STRING, 0, "rmenu b" },
{ "-", "c", 13, 2, "", NULL, STRING, 0, "rmenu c" },
{ "-", "h", 13, 2, "", NULL, STRING, 0, "rmenu h" },
{
	" ", " ", 0, 0, " ", NULL, (int) NULL, 0, NULL
}
};


MENU_DATA       room_page_a_data[] =
{{
		"1", "a", 5, 2, "%-73.73s", NULL, STRING, 1, "redit name %s"
},
{
	"1", "b", 7, 2, "%-73.73s", NULL, STRING, 0, "redit desc"
},
{ "-", "a", 13, 2, "", NULL, STRING, 0, "rmenu a" },
{ "-", "b", 13, 2, "", NULL, STRING, 0, "rmenu b" },
{ "-", "c", 13, 2, "", NULL, STRING, 0, "rmenu c" },
{ "-", "h", 13, 2, "", NULL, STRING, 0, "rmenu h" },
{
	" ", " ", 0, 0, " ", NULL, (int) NULL, 0, NULL
}
};

MENU_DATA       room_page_b_data[] =
{{
		"1", "a", 2, 26, "%1.1s", NULL, STRING, 1, "redit flags dark"
},
{
	"1", "b", 2, 42, "%1.1s", NULL, STRING, 1, "redit flags death"
},
{
	"1", "c", 2, 60, "%1.1s", NULL, STRING, 1, "redit flags nomob"
},
{
	"1", "d", 3, 26, "%1.1s", NULL, STRING, 1, "redit flags indoors"
},
{
	"1", "e", 3, 42, "%1.1s", NULL, STRING, 1, "redit flags lawful"
},
{
	"1", "f", 3, 60, "%1.1s", NULL, STRING, 1, "redit flags nomagic"
},
{
	"1", "g", 4, 26, "%1.1s", NULL, STRING, 1, "redit flags petshop"
},
{
	"1", "h", 4, 42, "%1.1s", NULL, STRING, 1, "redit flags neutral"
},
{
	"1", "i", 4, 60, "%1.1s", NULL, STRING, 1, "redit flags nodrop"
},
{
	"1", "j", 5, 26, "%1.1s", NULL, STRING, 1, "redit flags safe"
},
{
	"1", "k", 5, 42, "%1.1s", NULL, STRING, 1, "redit flags chaotic"
},
{
	"1", "l", 5, 60, "%1.1s", NULL, STRING, 1, "redit flags nodropall"
},
{
	"1", "m", 6, 26, "%1.1s", NULL, STRING, 1, "redit flags teleport"
},
{
	"1", "n", 6, 42, "%1.1s", NULL, STRING, 1, "redit flags donation"
},
{
	"1", "o", 6, 60, "%1.1s", NULL, STRING, 1, "redit flags norecall"
},
{
	"1", "p", 7, 26, "%1.1s", NULL, STRING, 1, "redit flags private"
},
{
	"1", "q", 7, 42, "%1.1s", NULL, STRING, 1, "redit flags solitary"
},
{
	"1", "r", 7, 60, "%1.1s", NULL, STRING, 1, "redit flags noastral"
},
/*
{
	"1", "s", 8, 26, "%1.1s", NULL, STRING, 1, "redit flags tunnel"
},
*/
{
	"1", "t", 8, 42, "%1.1s", NULL, STRING, 1, "redit flags silence"
},
{
	"1", "u", 8, 60, "%1.1s", NULL, STRING, 1, "redit flags nosummon"
},
{
	"1", "v", 9, 26, "%1.1s", NULL, STRING, 1, "redit flags clanstorage"
},
{
	"1", "w", 9, 42, "%1.1s", NULL, STRING, 1, "redit flags logspeech"
},
{
	"1", "x", 9, 60, "%1.1s", NULL, STRING, 1, "redit flags nofloor"
},
{
	"2", "a", 11, 25, "%1.1s", NULL, STRING, 0, "redit sector 0"
},				/* inside */
{
	"2", "b", 11, 44, "%1.1s", NULL, STRING, 0, "redit sector 3"
},				/* forest */
{
	"2", "c", 11, 55, "%1.1s", NULL, STRING, 0, "redit sector 1"
},				/* city   */
{
	"2", "d", 11, 68, "%1.1s", NULL, STRING, 0, "redit sector 4"
},				/* hills  */
{
	"2", "e", 12, 25, "%1.1s", NULL, STRING, 0, "redit sector 5"
},				/* mountain */
{
	"2", "f", 12, 44, "%1.1s", NULL, STRING, 0, "redit sector 6"
},				/* water_s */
{
	"2", "g", 12, 55, "%1.1s", NULL, STRING, 0, "redit sector 9"
},				/* air    */
{
	"2", "h", 12, 68, "%1.1s", NULL, STRING, 0, "redit sector 10"
},				/* desert */
{
	"2", "i", 13, 25, "%1.1s", NULL, STRING, 0, "redit sector 11"
},				/* dunno  */
{
	"2", "j", 13, 44, "%1.1s", NULL, STRING, 0, "redit sector 7"
},				/* water_n */
{
	"2", "l", 13, 68, "%1.1s", NULL, STRING, 0, "redit sector 2"
},				/* field  */
{ "-", "a", 13, 2, "", NULL, STRING, 0, "rmenu a" },
{ "-", "b", 13, 2, "", NULL, STRING, 0, "rmenu b" },
{ "-", "c", 13, 2, "", NULL, STRING, 0, "rmenu c" },
{ "-", "h", 13, 2, "", NULL, STRING, 0, "rmenu h" },
{
	" ", " ", 0, 0, " ", NULL, (int)NULL, 0, NULL
}
};


MENU_DATA       obj_page_e_data[] =
{{
		"1", "a", 2, 26, "%1.1s", NULL, STRING, 1, "oset %s flags evil"
},
{
	"1", "b", 2, 44, "%1.1s", NULL, STRING, 1, "oset %s flags antigood"
},
{
	"1", "c", 2, 61, "%1.1s", NULL, STRING, 1, "oset %s flags nodrop"
},
{
	"1", "d", 3, 26, "%1.1s", NULL, STRING, 1, "oset %s flags invis"
},
{
	"1", "e", 3, 44, "%1.1s", NULL, STRING, 1, "oset %s flags antievil"
},
{
	"1", "f", 3, 61, "%1.1s", NULL, STRING, 1, "oset %s flags noremove"
},
{
	"1", "g", 4, 26, "%1.1s", NULL, STRING, 1, "oset %s flags hidden"
},
{
	"1", "h", 4, 44, "%1.1s", NULL, STRING, 1, "oset %s flags antineutral"
},
{
	"1", "i", 4, 61, "%1.1s", NULL, STRING, 1, "oset %s flags inventory"
},
{
	"1", "j", 5, 12, "%1.1s", NULL, STRING, 1, "oset %s flags glow"
},
{
	"1", "k", 5, 26, "%1.1s", NULL, STRING, 1, "oset %s flags bless"
},
{
	"1", "l", 5, 44, "%1.1s", NULL, STRING, 1, "oset %s flags antimage"
},
{
	"1", "m", 5, 61, "%1.1s", NULL, STRING, 1, "oset %s flags donation"
},
{
	"1", "n", 6, 12, "%1.1s", NULL, STRING, 1, "oset %s flags hum"
},
{
	"1", "o", 6, 26, "%1.1s", NULL, STRING, 1, "oset %s flags metal"
},
{
	"1", "p", 6, 44, "%1.1s", NULL, STRING, 1, "oset %s flags antithief"
},
{
	"1", "q", 6, 61, "%1.1s", NULL, STRING, 1, "oset %s flags clanobject"
},
{
	"1", "r", 7, 12, "%1.1s", NULL, STRING, 1, "oset %s flags dark"
},
{
	"1", "s", 7, 26, "%1.1s", NULL, STRING, 1, "oset %s flags organic"
},
{
	"1", "t", 7, 44, "%1.1s", NULL, STRING, 1, "oset %s flags antiwarrior"
},
{
	"1", "u", 7, 61, "%1.1s", NULL, STRING, 1, "oset %s flags clancorpse"
},
{
	"1", "v", 8, 12, "%1.1s", NULL, STRING, 1, "oset %s flags magic"
},
{
	"1", "x", 8, 44, "%1.1s", NULL, STRING, 1, "oset %s flags anticleric"
},
{
	"1", "y", 8, 61, "%1.1s", NULL, STRING, 1, "oset %s flags prototype"
},
{
	"1", "z", 9, 12, "%1.1s", NULL, STRING, 1, "oset %s flags loyal"
},
{
	"1", "B", 9, 44, "%1.1s", NULL, STRING, 1, "oset %s flags antivampire"
},
{
	"1", "C", 9, 61, "%1.1s", NULL, STRING, 1, "oset %s flags deathrot"
},
{
	"1", "F", 10, 44, "%1.1s", NULL, STRING, 1, "oset %s flags antidruid"
},

{
	"2", "a", 12, 24, "%1.1s", NULL, STRING, 1, "oset %s wear take"
},
{
	"2", "b", 12, 39, "%1.1s", NULL, STRING, 1, "oset %s wear finger"
},
{
	"2", "c", 12, 52, "%1.1s", NULL, STRING, 1, "oset %s wear neck"
},
{
	"2", "d", 12, 64, "%1.1s", NULL, STRING, 1, "oset %s wear body"
},
{
	"2", "e", 13, 24, "%1.1s", NULL, STRING, 1, "oset %s wear head"
},
{
	"2", "f", 13, 39, "%1.1s", NULL, STRING, 1, "oset %s wear legs"
},
{
	"2", "g", 13, 52, "%1.1s", NULL, STRING, 1, "oset %s wear feet"
},
{
	"2", "h", 13, 64, "%1.1s", NULL, STRING, 1, "oset %s wear hands"
},
{
	"2", "e", 14, 24, "%1.1s", NULL, STRING, 1, "oset %s wear arms"
},
{
	"2", "f", 14, 39, "%1.1s", NULL, STRING, 1, "oset %s wear shield"
},
{
	"2", "g", 14, 52, "%1.1s", NULL, STRING, 1, "oset %s wear about"
},
{
	"2", "h", 14, 64, "%1.1s", NULL, STRING, 1, "oset %s wear waist"
},
{
	"2", "b", 15, 12, "%1.1s", NULL, STRING, 1, "oset %s wear wrist"
},
{
	"2", "c", 15, 24, "%1.1s", NULL, STRING, 1, "oset %s wear wield"
},
{
	"2", "d", 15, 39, "%1.1s", NULL, STRING, 1, "oset %s wear hold"
},
{
	"2", "d", 15, 52, "%1.1s", NULL, STRING, 1, "oset %s wear _dual_"
},
{
	"2", "d", 15, 64, "%1.1s", NULL, STRING, 1, "oset %s wear ears"
},
{
	"2", "d", 16, 64, "%1.1s", NULL, STRING, 1, "oset %s wear eyes"
},
{ "-", "a", 13, 2, "", NULL, STRING, 1, "omenu %s a" },
{ "-", "b", 13, 2, "", NULL, STRING, 1, "omenu %s b" },
{ "-", "c", 13, 2, "", NULL, STRING, 1, "omenu %s c" },
{ "-", "d", 13, 2, "", NULL, STRING, 1, "omenu %s d" },
{ "-", "e", 13, 2, "", NULL, STRING, 1, "omenu %s e" },
{ "-", "h", 13, 2, "", NULL, STRING, 1, "omenu %s h" },
{
	" ", " ", 0, 0, " ", NULL, (int) NULL, 0, NULL
}
};

MENU_DATA       obj_page_d_data[] =
{{
		"1", "a", 3, 29, "%1.1s", NULL, STRING, 1, "oset %s affect affected blind "
},
{
	"1", "b", 3, 47, "%1.1s", NULL, STRING, 1, "oset %s affect affected det_invis"
},
{
	"1", "c", 3, 63, "%1.1s", NULL, STRING, 1, "oset %s affect affected det_evil"
},
{
	"1", "d", 4, 29, "%1.1s", NULL, STRING, 1, "oset %s affect affected det_magic"
},
{
	"1", "e", 4, 47, "%1.1s", NULL, STRING, 1, "oset %s affect affected det_hidden"
},
{
	"1", "f", 4, 63, "%1.1s", NULL, STRING, 1, "oset %s affect affected hold"
},
{
	"1", "g", 5, 12, "%1.1s", NULL, STRING, 1, "oset %s affect affected sanctuary"
},
{
	"1", "h", 5, 29, "%1.1s", NULL, STRING, 1, "oset %s affect affected faerie_fire"
},
{
	"1", "i", 5, 47, "%1.1s", NULL, STRING, 1, "oset %s affect affected infrared"
},
{
	"1", "j", 5, 63, "%1.1s", NULL, STRING, 1, "oset %s affect affected curse"
},
{
	"1", "k", 6, 12, "%1.1s", NULL, STRING, 1, "oset %s affect affected flaming"
},
{
	"1", "l", 6, 29, "%1.1s", NULL, STRING, 1, "oset %s affect affected poison"
},
{
	"1", "m", 6, 47, "%1.1s", NULL, STRING, 1, "oset %s affect affected protect"
},
{
	"1", "n", 6, 63, "%1.1s", NULL, STRING, 1, "oset %s affect affected paralysis"
},
{
	"1", "o", 7, 12, "%1.1s", NULL, STRING, 1, "oset %s affect affected sleep"
},
{
	"1", "p", 7, 29, "%1.1s", NULL, STRING, 1, "oset %s affect affected hide"
},
{
	"1", "q", 7, 47, "%1.1s", NULL, STRING, 1, "oset %s affect affected sneak"
},
{
	"1", "r", 7, 63, "%1.1s", NULL, STRING, 1, "oset %s affect affected charm"
},
{
	"1", "s", 8, 12, "%1.1s", NULL, STRING, 1, "oset %s affect affected flying"
},
{
	"1", "t", 8, 29, "%1.1s", NULL, STRING, 1, "oset %s affect affected passdoor"
},
{
	"1", "u", 8, 47, "%1.1s", NULL, STRING, 1, "oset %s affect affected truesight"
},
{
	"1", "v", 8, 63, "%1.1s", NULL, STRING, 1, "oset %s affect affected det_traps"
},
{
	"1", "w", 9, 12, "%1.1s", NULL, STRING, 1, "oset %s affect affected scrying"
},
{
	"1", "x", 9, 29, "%1.1s", NULL, STRING, 1, "oset %s affect affected fireshield"
},
{
	"1", "y", 9, 47, "%1.1s", NULL, STRING, 1, "oset %s affect affected shockshield"
},
{
	"1", "z", 9, 63, "%1.1s", NULL, STRING, 1, "oset %s affect affected floating"
},
{ "1", "A", 10, 63, "%1.1s", NULL, STRING, 1, "oset %s affect affected invisible" },
{ "-", "a", 13, 2, "", NULL, STRING, 1, "omenu %s a" },
{ "-", "b", 13, 2, "", NULL, STRING, 1, "omenu %s b" },
{ "-", "c", 13, 2, "", NULL, STRING, 1, "omenu %s c" },
{ "-", "d", 13, 2, "", NULL, STRING, 1, "omenu %s d" },
{ "-", "e", 13, 2, "", NULL, STRING, 1, "omenu %s e" },
{ "-", "h", 13, 2, "", NULL, STRING, 1, "omenu %s h" },
{ " ", " ", 0, 0, " ", NULL, (int) NULL, 0, NULL }
};




/* the val#'s weren't dereferencing right as short ints *boggle* */
MENU_DATA       obj_page_a_data[] =
{{
		"1", "a", 2, 20, "%-49.49s", NULL, STRING, 2, "oset %s name %s"
},
{
	"1", "b", 3, 20, "%-49.49s", NULL, STRING, 2, "oset %s short %s"
},
{
	"1", "c", 4, 10, "%-59.69s", NULL, STRING, 2, "oset %s long %s"
},
{
	"1", "d", 5, 15, "%-49.59s", NULL, STRING, 2, "oset %s actiondesc %s"
},
{
	"2", "a", 9, 13, "%5.5d", NULL, INT, 2, "oset %s value0 %s"
},
{
	"2", NO_COMMAND
	,9, 23, "%-19.19s", NULL, STRING, 0, NO_COMMAND
},
{
	"2", "b", 10, 13, "%5.5d", NULL, INT, 2, "oset %s value1 %s"
},
{
	"2", NO_COMMAND
	,10, 23, "%-19.19s", NULL, STRING, 0, NO_COMMAND
},
{
	"2", "c", 11, 13, "%5.5d", NULL, INT, 2, "oset %s value2 %s"
},
{
	"2", NO_COMMAND
	,11, 23, "%-19.19s", NULL, STRING, 0, NO_COMMAND
},
{
	"2", "d", 12, 13, "%5.5d", NULL, INT, 2, "oset %s value3 %s"
},
{
	"2", NO_COMMAND
	,12, 23, "%-19.19s", NULL, STRING, 0, NO_COMMAND
},
{
	"3", "a", 9, 59, "%5.5hd", NULL, SH_INT, 2, "oset %s weight %s"
},
{
	"3", "b", 10, 59, "%5.5hd", NULL, SH_INT, 2, "oset %s cost %s"
},
{ "-", "a", 13, 2, "", NULL, STRING, 1, "omenu %s a" },
{ "-", "b", 13, 2, "", NULL, STRING, 1, "omenu %s b" },
{ "-", "c", 13, 2, "", NULL, STRING, 1, "omenu %s c" },
{ "-", "d", 13, 2, "", NULL, STRING, 1, "omenu %s d" },
{ "-", "e", 13, 2, "", NULL, STRING, 1, "omenu %s e" },
{ "-", "h", 13, 2, "", NULL, STRING, 1, "omenu %s h" },
{
	" ", " ", 0, 0, " ", NULL, (int) NULL, 0, NULL
}
};

MENU_DATA       obj_page_b_data[] =
{{
		"1", "a", 2, 23, "%1.1s", NULL, STRING, 1, "oset %s type none"
},
{
	"1", "b", 2, 42, "%1.1s", NULL, STRING, 1, "oset %s type fountain"
},
{
	"1", "c", 2, 66, "%1.1s", NULL, STRING, 1, "oset %s type furniture"
},
{
	"1", "d", 3, 23, "%1.1s", NULL, STRING, 1, "oset %s type armor"
},
{
	"1", "e", 3, 42, "%1.1s", NULL, STRING, 1, "oset %s type food"
},
{
	"1", "f", 3, 66, "%1.1s", NULL, STRING, 1, "oset %s type money"
},
{
	"1", "g", 4, 23, "%1.1s", NULL, STRING, 1, "oset %s type potion"
},
{
	"1", "h", 4, 42, "%1.1s", NULL, STRING, 1, "oset %s type pullchain"
},
{
	"1", "i", 4, 66, "%1.1s", NULL, STRING, 1, "oset %s type weapon"
},
{
	"1", "j", 5, 9, "%1.1s", NULL, STRING, 1, "oset %s type map"
},
{
	"1", "k", 5, 23, "%1.1s", NULL, STRING, 1, "oset %s type light"
},
{
	"1", "l", 5, 42, "%1.1s", NULL, STRING, 1, "oset %s type button"
},
{
	"1", "m", 5, 66, "%1.1s", NULL, STRING, 1, "oset %s type treasure"
},
{
	"1", "n", 6, 9, "%1.1s", NULL, STRING, 1, "oset %s type herb"
},
{
	"1", "o", 6, 23, "%1.1s", NULL, STRING, 1, "oset %s type trash"
},
{
	"1", "p", 6, 42, "%1.1s", NULL, STRING, 1, "oset %s type switch"
},
{
	"1", "q", 6, 66, "%1.1s", NULL, STRING, 1, "oset %s type container"
},
{
	"1", "r", 7, 9, "%1.1s", NULL, STRING, 1, "oset %s type wand"
},
{
	"1", "s", 7, 23, "%1.1s", NULL, STRING, 1, "oset %s type scroll"
},
{
	"1", "t", 7, 42, "%1.1s", NULL, STRING, 1, "oset %s type staff"
},
{
	"1", "u", 7, 66, "%1.1s", NULL, STRING, 1, "oset %s type bloodstain"
},
{
	"1", "v", 8, 9, "%1.1s", NULL, STRING, 1, "oset %s type pill"
},
{
	"1", "w", 8, 23, "%1.1s", NULL, STRING, 1, "oset %s type blood"
},
{
	"1", "x", 8, 42, "%1.1s", NULL, STRING, 1, "oset %s type dial"
},
{
	"1", "y", 8, 66, "%1.1s", NULL, STRING, 1, "oset %s type portal"
},
{
	"1", "z", 9, 9, "%1.1s", NULL, STRING, 1, "oset %s type key"
},
{
	"1", "A", 9, 23, "%1.1s", NULL, STRING, 1, "oset %s type incense"
},
{
	"1", "B", 9, 42, "%1.1s", NULL, STRING, 1, "oset %s type lever"
},
{
	"1", "C", 9, 66, "%1.1s", NULL, STRING, 1, "oset %s type corpse_pc"
},
{
	"1", "D", 10, 9, "%1.1s", NULL, STRING, 1, "oset %s type pipe"
},
{
	"1", "E", 10, 23, "%1.1s", NULL, STRING, 1, "oset %s type scraps"
},
{
	"1", "F", 10, 42, "%1.1s", NULL, STRING, 1, "oset %s type trap"
},
{
	"1", "G", 10, 66, "%1.1s", NULL, STRING, 1, "oset %s type corpse"
},
{
	"1", "H", 11, 9, "%1.1s", NULL, STRING, 1, "oset %s type book"
},
{
	"1", "I", 11, 23, "%1.1s", NULL, STRING, 1, "oset %s type match"
},
{
	"1", "J", 11, 42, "%1.1s", NULL, STRING, 1, "oset %s type drinkcon"
},
{
	"1", "K", 11, 66, "%1.1s", NULL, STRING, 1, "oset %s type portal"
},
{
	"1", "L", 12, 9, "%1.1s", NULL, STRING, 1, "oset %s type pen"
},
{
	"1", "M", 12, 23, "%1.1s", NULL, STRING, 1, "oset %s type fire"
},
{
	"1", "N", 12, 42, "%1.1s", NULL, STRING, 1, "oset %s type herbcon"
},
{
	"1", "O", 12, 66, "%1.1s", NULL, STRING, 1, "oset %s type _fireweapon"
},
{
	"1", "P", 13, 9, "%1.1s", NULL, STRING, 1, "oset %s type boat"
},
{
	"1", "Q", 13, 23, "%1.1s", NULL, STRING, 1, "oset %s type rune"
},
{
	"1", "R", 13, 42, "%1.1s", NULL, STRING, 1, "oset %s type runepouch"
},
{
	"1", "S", 13, 66, "%1.1s", NULL, STRING, 1, "oset %s type _missile"
},
{ "-", "a", 13, 2, "", NULL, STRING, 1, "omenu %s a" },
{ "-", "b", 13, 2, "", NULL, STRING, 1, "omenu %s b" },
{ "-", "c", 13, 2, "", NULL, STRING, 1, "omenu %s c" },
{ "-", "d", 13, 2, "", NULL, STRING, 1, "omenu %s d" },
{ "-", "e", 13, 2, "", NULL, STRING, 1, "omenu %s e" },
{ "-", "h", 13, 2, "", NULL, STRING, 1, "omenu %s h" },
{
	" ", " ", 0, 0, " ", NULL, (int) NULL, 0, NULL
}
};

MENU_DATA       obj_page_c_data[] =
{{
		"1", "a", 2, 23, "%1.1s", NULL, STRING, 1, "oset %s affect none"
},
{
	"1", "b", 2, 44, "%+-3.3hd", NULL, SH_INT, 2, "oset %s affect save_poison %s"
},
{
	"1", "c", 2, 65, "%+-3.3hd", NULL, SH_INT, 2, "oset %s affect age %s"
},
{
	"1", "d", 3, 23, "%+-2.2hd", NULL, SH_INT, 2, "oset %s affect strength %s"
},
{
	"1", "e", 3, 44, "%+-3.3hd", NULL, SH_INT, 2, "oset %s affect save_rod %s"
},
{
	"1", "f", 3, 65, "%+-3.3hd", NULL, SH_INT, 2, "oset %s affect class %s"
},
{
	"1", "g", 4, 23, "%+-2.2hd", NULL, SH_INT, 2, "oset %s affect intelligence %s"
},
{
	"1", "h", 4, 44, "%+-3.3hd", NULL, SH_INT, 2, "oset %s affect save_para %s"
},
{
	"1", "i", 4, 65, "%+-3.3hd", NULL, SH_INT, 2, "oset %s affect level %s"
},
{
	"1", "j", 5, 23, "%+-2.2hd", NULL, SH_INT, 2, "oset %s affect wisdom %s"
},
{
	"1", "k", 5, 44, "%+-3.3hd", NULL, SH_INT, 2, "oset %s affect save_breath %s"
},
{
	"1", "l", 5, 65, "%+-1.1hd", NULL, SH_INT, 2, "oset %s affect sex %s"
},
{
	"1", "m", 6, 23, "%+-2.2hd", NULL, SH_INT, 2, "oset %s affect dexterity %s"
},
{
	"1", "n", 6, 44, "%+-3.3hd", NULL, SH_INT, 2, "oset %s affect save_spell %s"
},
{
	"1", "o", 6, 65, "%+-3.3hd", NULL, SH_INT, 2, "oset %s affect height %s"
},
{
	"1", "p", 7, 23, "%+-2.2hd", NULL, SH_INT, 2, "oset %s affect constitution %s"
},
{
	"1", "r", 7, 65, "%+-3.3hd", NULL, SH_INT, 2, "oset %s affect weight %s"
},
{
	"1", "s", 8, 23, "%+-2.2hd", NULL, SH_INT, 2, "oset %s affect charisma %s"
},

{
	"1", "v", 9, 12, "%+-4.4hd", NULL, SH_INT, 2, "oset %s affect mana %s"
},
{
	"1", "w", 9, 41, "%+-4.4hd", NULL, SH_INT, 2, "oset %s affect armor %s"
},
{
	"1", "y", 10, 12, "%+-4.4hd", NULL, SH_INT, 2, "oset %s affect hit %s"
},
{
	"1", "z", 10, 41, "%+-4.4hd", NULL, SH_INT, 2, "oset %s affect hitroll %s"
},
{
	"1", "A", 10, 63, "%+-4.4hd", NULL, SH_INT, 2, "oset %s affect gold %s"
},
{
	"1", "B", 11, 12, "%+-4.4hd", NULL, SH_INT, 2, "oset %s affect move %s"
},
{
	"1", "C", 11, 41, "%+-4.4hd", NULL, SH_INT, 2, "oset %s affect damroll %s"
},
{
	"1", "D", 11, 63, "%+-4.4hd", NULL, SH_INT, 2, "oset %s affect experience %s"
},
{
	"2", "a", 13, 22, "%3.3s", NULL, STRING, 2, "oset %s %s fire"
},
{
	"2", "b", 13, 34, "%3.3s", NULL, STRING, 2, "oset %s %s cold"
},
{
	"2", "c", 13, 46, "%3.3s", NULL, STRING, 2, "oset %s %s electricity"
},
{
	"2", "d", 13, 58, "%3.3s", NULL, STRING, 2, "oset %s %s energy"
},
{
	"2", "e", 13, 70, "%3.3s", NULL, STRING, 2, "oset %s %s poison"
},
{
	"2", "f", 14, 22, "%3.3s", NULL, STRING, 2, "oset %s %s blunt"
},
{
	"2", "g", 14, 34, "%3.3s", NULL, STRING, 2, "oset %s %s pierce"
},
{
	"2", "h", 14, 46, "%3.3s", NULL, STRING, 2, "oset %s %s slash"
},
{
	"2", "i", 14, 58, "%3.3s", NULL, STRING, 2, "oset %s %s acid"
},
{
	"2", "j", 14, 70, "%3.3s", NULL, STRING, 2, "oset %s %s nonmagic"
},
{
	"2", "k", 15, 22, "%3.3s", NULL, STRING, 2, "oset %s %s drain"
},
{
	"2", "l", 15, 34, "%3.3s", NULL, STRING, 2, "oset %s %s sleep"
},
{
	"2", "m", 15, 46, "%3.3s", NULL, STRING, 2, "oset %s %s charm"
},
{
	"2", "n", 15, 58, "%3.3s", NULL, STRING, 2, "oset %s %s hold"
},
{
	"2", "o", 15, 70, "%3.3s", NULL, STRING, 2, "oset %s %s magic"
},
{
	"2", "p", 16, 10, "%3.3s", NULL, STRING, 2, "oset %s %s plus1"
},
{
	"2", "q", 16, 22, "%3.3s", NULL, STRING, 2, "oset %s %s plus2"
},
{
	"2", "r", 16, 34, "%3.3s", NULL, STRING, 2, "oset %s %s plus3"
},
{
	"2", "s", 16, 46, "%3.3s", NULL, STRING, 2, "oset %s %s plus4"
},
{
	"2", "t", 16, 58, "%3.3s", NULL, STRING, 2, "oset %s %s plus5"
},
{
	"2", "u", 16, 70, "%3.3s", NULL, STRING, 2, "oset %s %s plus6"
},
{ "-", "a", 13, 2, "", NULL, STRING, 1, "omenu %s a" },
{ "-", "b", 13, 2, "", NULL, STRING, 1, "omenu %s b" },
{ "-", "c", 13, 2, "", NULL, STRING, 1, "omenu %s c" },
{ "-", "d", 13, 2, "", NULL, STRING, 1, "omenu %s d" },
{ "-", "e", 13, 2, "", NULL, STRING, 1, "omenu %s e" },
{ "-", "h", 13, 2, "", NULL, STRING, 1, "omenu %s h" },
{
	" ", " ", 0, 0, " ", NULL, (int) NULL, 0, NULL
}
};



MENU_DATA       mob_page_a_data[] =
{{
		"1", "a", 2, 20, "%-59.59s", NULL, STRING, 2, "mset %s name %s"
},
{
	"1", "b", 3, 20, "%-59.59s", NULL, STRING, 2, "mset %s short %s"
},
{
	"1", "c", 4, 10, "%-69.68s", NULL, STRING, 2, "mset %s long %s"
},
{
	"1", "d", 5, 10, "%-69.68s", NULL, STRING, 2, "mset %s description %s"
},
{
	"2", "a", 9, 11, "%5.5hd", NULL, SH_INT, 2, "mset %s vnum %s"
},
{
	"2", "b", 10, 11, "%3.3hd", NULL, SH_INT, 2, "mset %s level %s"
},
{
	"2", "c", 11, 11, "%-10.10s", NULL, STRING, 2, "mset (obsolete) %s class %s"
},
{
	"2", "d", 12, 11, "%-10.10s", NULL, STRING, 2, "mset %s race %s"
},
{
	"2", "e", 13, 11, "%-6.6s", NULL, STRING, 2, "mset %s sex %s"
},
{
	"2", "f", 14, 11, "%d", NULL, INT, 2, "mset %s gold %s"
},
{
	"2", "g", 15, 11, "%+-4.4hd", NULL, SH_INT, 2, "mset %s align %s"
},
{
	"3", "a", 9, 31, "%2.2hd", NULL, SH_INT, 2, "mset %s str %s"
},
{
	"3", "b", 10, 31, "%2.2hd", NULL, SH_INT, 2, "mset %s int %s"
},
{
	"3", "c", 11, 31, "%2.2hd", NULL, SH_INT, 2, "mset %s wis %s"
},
{
	"3", "d", 12, 31, "%2.2hd", NULL, SH_INT, 2, "mset %s dex %s"
},
{
	"3", "e", 13, 31, "%2.2hd", NULL, SH_INT, 2, "mset %s con %s"
},
{
	"3", "f", 14, 31, "%2.2hd", NULL, SH_INT, 2, "mset %s cha %s"
},
{
	"3", "g", 15, 31, "%4.4hd", NULL, SH_INT, 2, "mset %s armor %s"
},
{
	"4", "a", 9, 49, "%1.1s", NULL, STRING, 1, "mset %s class 0"
},
{
	"4", "b", 9, 68, "%1.1s", NULL, STRING, 1, "mset %s class 1"
},
{
	"4", "c", 10, 49, "%1.1s", NULL, STRING, 1, "mset %s class 2"
},
{
	"4", "d", 10, 68, "%1.1s", NULL, STRING, 1, "mset %s class 3"
},
{
	"4", "e", 11, 49, "%1.1s", NULL, STRING, 1, "mset %s class 4"
},
{
	"4", "f", 11, 68, "%1.1s", NULL, STRING, 1, "mset %s class 5"
},
{
	"4", "g", 12, 49, "%1.1s", NULL, STRING, 1, "mset %s class 6"
},
{
	"4", "h", 12, 68, "%1.1s", NULL, STRING, 1, "mset %s class 20"
},
{
	"4", "i", 13, 49, "%1.1s", NULL, STRING, 1, "mset %s class 21"
},
{
	"4", "j", 13, 68, "%1.1s", NULL, STRING, 1, "mset %s class 22"
},
{
	"4", "k", 14, 49, "%1.1s", NULL, STRING, 1, "mset %s class 23"
},
{
	"4", "l", 14, 68, "%1.1s", NULL, STRING, 1, "mset %s class 24"
},
{
	"4", "m", 15, 49, "%1.1s", NULL, STRING, 1, "mset %s class 25"
},
{ "-", "a", 13, 2, "", NULL, STRING, 1, "mmenu %s a" },
{ "-", "b", 13, 2, "", NULL, STRING, 1, "mmenu %s b" },
{ "-", "c", 13, 2, "", NULL, STRING, 1, "mmenu %s c" },
{ "-", "d", 13, 2, "", NULL, STRING, 1, "mmenu %s d" },
{ "-", "e", 13, 2, "", NULL, STRING, 1, "mmenu %s e" },
{ "-", "f", 13, 2, "", NULL, STRING, 1, "mmenu %s f" },
{ "-", "h", 13, 2, "", NULL, STRING, 1, "mmenu %s h" },
{ " ", " ", 0, 0, " ", NULL, (int) NULL, 1, NULL } };

MENU_DATA       mob_page_f_data[] =
{{ "1", "a", 2, 27, "%1.1s", NULL, STRING, 1, "mset %s race human" },
 { "1", "b", 2, 42, "%1.1s", NULL, STRING, 1, "mset %s race neanderthal" },
 { "1", "c", 2, 57, "%1.1s", NULL, STRING, 1, "mset %s race avis" },
 { "1", "d", 2, 72, "%1.1s", NULL, STRING, 1, "mset %s race amphibian" },

 { "1", "e", 3, 27, "%1.1s", NULL, STRING, 1, "mset %s race elf" },
 { "1", "f", 3, 42, "%1.1s", NULL, STRING, 1, "mset %s race orc" },
 { "1", "g", 3, 57, "%1.1s", NULL, STRING, 1, "mset %s race bugbear" },
 { "1", "h", 3, 72, "%1.1s", NULL, STRING, 1, "mset %s race bovine" },

 { "1", "i", 4, 27, "%1.1s", NULL, STRING, 1, "mset %s race dwarf" },
 { "1", "j", 4, 42, "%1.1s", NULL, STRING, 1, "mset %s race skeleton" },
 { "1", "k", 4, 57, "%1.1s", NULL, STRING, 1, "mset %s race dragon" },
 { "1", "l", 4, 72, "%1.1s", NULL, STRING, 1, "mset %s race canine" },

 { "1", "m", 5, 12, "%1.1s", NULL, STRING, 1, "mset %s race ant" },
 { "1", "n", 5, 27, "%1.1s", NULL, STRING, 1, "mset %s race halfling" },
 { "1", "o", 5, 42, "%1.1s", NULL, STRING, 1, "mset %s race thoul" },
 { "1", "p", 5, 57, "%1.1s", NULL, STRING, 1, "mset %s race rustmonster" },
 { "1", "q", 5, 72, "%1.1s", NULL, STRING, 1, "mset %s race crustacean" },

 { "1", "r", 6, 12, "%1.1s", NULL, STRING, 1, "mset %s race bee" },
 { "1", "s", 6, 27, "%1.1s", NULL, STRING, 1, "mset %s race pixie" },
 { "1", "t", 6, 42, "%1.1s", NULL, STRING, 1, "mset %s race undead" },
 { "1", "u", 6, 57, "%1.1s", NULL, STRING, 1, "mset %s race shadow" },
 { "1", "v", 6, 72, "%1.1s", NULL, STRING, 1, "mset %s race feline" },

 { "1", "w", 7, 12, "%1.1s", NULL, STRING, 1, "mset %s race beetle" },
 { "1", "x", 7, 27, "%1.1s", NULL, STRING, 1, "mset %s race vampire" },
 { "1", "y", 7, 42, "%1.1s", NULL, STRING, 1, "mset %s race zombie" },
 { "1", "z", 7, 57, "%1.1s", NULL, STRING, 1, "mset %s race shapeshifter" },
 { "1", "A", 7, 72, "%1.1s", NULL, STRING, 1, "mset %s race fish" },

 { "1", "B", 8, 12, "%1.1s", NULL, STRING, 1, "mset %s race fly" },
 { "1", "C", 8, 27, "%1.1s", NULL, STRING, 1, "mset %s race gargoyle" },
 { "1", "D", 8, 42, "%1.1s", NULL, STRING, 1, "mset %s race ape" },
 { "1", "E", 8, 57, "%1.1s", NULL, STRING, 1, "mset %s race shrieker" },
 { "1", "F", 8, 72, "%1.1s", NULL, STRING, 1, "mset %s race insect" },

 { "1", "G", 9, 12, "%1.1s", NULL, STRING, 1, "mset %s race locust" },
 { "1", "H", 9, 27, "%1.1s", NULL, STRING, 1, "mset %s race ghoul" },
 { "1", "I", 9, 42, "%1.1s", NULL, STRING, 1, "mset %s race baboon" },
 { "1", "J", 9, 57, "%1.1s", NULL, STRING, 1, "mset %s race stirge" },
 { "1", "K", 9, 72, "%1.1s", NULL, STRING, 1, "mset %s race mammal" },

 { "1", "L",10, 12, "%1.1s", NULL, STRING, 1, "mset %s race snake" },
 { "1", "M",10, 27, "%1.1s", NULL, STRING, 1, "mset %s race gnoll" },
 { "1", "N",10, 42, "%1.1s", NULL, STRING, 1, "mset %s race bat" },
 { "1", "O",10, 57, "%1.1s", NULL, STRING, 1, "mset %s race wight" },
 { "1", "P",10, 72, "%1.1s", NULL, STRING, 1, "mset %s race porcine" },

 { "1", "Q",11, 12, "%1.1s", NULL, STRING, 1, "mset %s race spider" },
 { "1", "R",11, 27, "%1.1s", NULL, STRING, 1, "mset %s race gnome" },
 { "1", "S",11, 42, "%1.1s", NULL, STRING, 1, "mset %s race bear" },
 { "1", "U",11, 72, "%1.1s", NULL, STRING, 1, "mset %s race reptile" },

 { "1", "V",12, 12, "%1.1s", NULL, STRING, 1, "mset %s race worm" },
 { "1", "W",12, 27, "%1.1s", NULL, STRING, 1, "mset %s race goblin" },
 { "1", "X",12, 42, "%1.1s", NULL, STRING, 1, "mset %s race boar" },
 { "1", "Z",12, 72, "%1.1s", NULL, STRING, 1, "mset %s race rodent" },

 { "1", "1",13, 27, "%1.1s", NULL, STRING, 1, "mset %s race golem" },
 { "1", "2",13, 42, "%1.1s", NULL, STRING, 1, "mset %s race cat" },
 { "1", "4",13, 72, "%1.1s", NULL, STRING, 1, "mset %s race troglodyte" },

 { "1", "5",14, 12, "%1.1s", NULL, STRING, 1, "mset %s race gelatin" },
 { "1", "6",14, 27, "%1.1s", NULL, STRING, 1, "mset %s race gorgon" },
 { "1", "7",14, 42, "%1.1s", NULL, STRING, 1, "mset %s race dog" },
 { "1", "9",14, 72, "%1.1s", NULL, STRING, 1, "mset %s race spirit" },

 { "1", "!",15, 12, "%1.1s", NULL, STRING, 1, "mset %s race mold" },
 { "1", "@",15, 27, "%1.1s", NULL, STRING, 1, "mset %s race harpy" },
 { "1", "#",15, 42, "%1.1s", NULL, STRING, 1, "mset %s race ferret" },
 { "1", "%",15, 72, "%1.1s", NULL, STRING, 1, "mset %s race magical" },

 { "1", "^",16, 12, "%1.1s", NULL, STRING, 1, "mset %s race ooze" },
 { "1", "&",16, 27, "%1.1s", NULL, STRING, 1, "mset %s race hobgoblin" },
 { "1", "*",16, 42, "%1.1s", NULL, STRING, 1, "mset %s race horse" },

 { "1", "-",17, 12, "%1.1s", NULL, STRING, 1, "mset %s race slime" },
 { "1", "_",17, 27, "%1.1s", NULL, STRING, 1, "mset %s race kobold" },
 { "1", "+",17, 42, "%1.1s", NULL, STRING, 1, "mset %s race mule" },

 { "1", "~",18, 27, "%1.1s", NULL, STRING, 1, "mset %s race lizardman" },
 { "1", "`",18, 42, "%1.1s", NULL, STRING, 1, "mset %s race rat" },

 { "1", "]",19, 27, "%1.1s", NULL, STRING, 1, "mset %s race lycanthrope" },
 { "1", ":",19, 42, "%1.1s", NULL, STRING, 1, "mset %s race shrew" },

 { "1", ",",20, 27, "%1.1s", NULL, STRING, 1, "mset %s race minotaur" },
 { "1", ">",20, 42, "%1.1s", NULL, STRING, 1, "mset %s race wolf" },

{ "-", "a", 22, 2, "", NULL, STRING, 1, "mmenu %s a" },
{ "-", "b", 13, 2, "", NULL, STRING, 1, "mmenu %s b" },
{ "-", "c", 13, 2, "", NULL, STRING, 1, "mmenu %s c" },
{ "-", "d", 13, 2, "", NULL, STRING, 1, "mmenu %s d" },
{ "-", "e", 13, 2, "", NULL, STRING, 1, "mmenu %s e" },
{ "-", "f", 13, 2, "", NULL, STRING, 1, "mmenu %s f" },
{ "-", "h", 13, 2, "", NULL, STRING, 1, "mmenu %s h" },
{ " ", " ", 0, 0, " ", NULL, (int) NULL, 1, NULL } };

MENU_DATA       mob_page_b_data[] =
{{
		"1", "a", 2, 29, "%1.1s", NULL, STRING, 1, "mset %s flags sentinel"
},
{
	"1", "b", 2, 47, "%1.1s", NULL, STRING, 1, "mset %s flags scavenger"
},
{
	"1", "c", 2, 63, "%1.1s", NULL, STRING, 1, "mset %s flags aggressive"
},
{
	"1", "d", 3, 29, "%1.1s", NULL, STRING, 1, "mset %s flags stayarea"
},
{
	"1", "e", 3, 47, "%1.1s", NULL, STRING, 1, "mset %s flags wimpy"
},
{
	"1", "f", 3, 63, "%1.1s", NULL, STRING, 1, "mset %s flags pet"
},
{
	"1", "g", 4, 12, "%1.1s", NULL, STRING, 1, "mset %s flags train"
},
{
	"1", "h", 4, 29, "%1.1s", NULL, STRING, 1, "mset %s flags practice"
},
{
	"1", "i", 4, 47, "%1.1s", NULL, STRING, 1, "mset %s flags immortal"
},
{
	"1", "j", 4, 63, "%1.1s", NULL, STRING, 1, "mset %s flags deadly"
},
{
	"1", "k", 5, 12, "%1.1s", NULL, STRING, 1, "mset %s flags polyself"
},
{
	"1", "l", 5, 29, "%1.1s", NULL, STRING, 1, "mset %s flags meta_aggr"
},
{
	"1", "m", 5, 47, "%1.1s", NULL, STRING, 1, "mset %s flags guardian"
},
{
	"1", "n", 5, 63, "%1.1s", NULL, STRING, 1, "mset %s flags running"
},
{
	"1", "o", 6, 12, "%1.1s", NULL, STRING, 1, "mset %s flags nowander"
},
{
	"1", "p", 6, 29, "%1.1s", NULL, STRING, 1, "mset %s flags mountable"
},
{
	"1", "q", 6, 47, "%1.1s", NULL, STRING, 1, "mset %s flags mounted"
},
{
	"2", "a", 8, 29, "%1.1s", NULL, STRING, 1, "mset %s affected blind"
},
{
	"2", "b", 8, 47, "%1.1s", NULL, STRING, 1, "mset %s affected detect_invis"
},
{
	"2", "c", 8, 63, "%1.1s", NULL, STRING, 1, "mset %s affected detect_evil"
},
{
	"2", "d", 9, 29, "%1.1s", NULL, STRING, 1, "mset %s affected detect_magic"
},
{
	"2", "e", 9, 47, "%1.1s", NULL, STRING, 1, "mset %s affected detect_hidden"
},
{
	"2", "f", 9, 63, "%1.1s", NULL, STRING, 1, "mset %s affected _hold"
},
{
	"2", "g", 10, 12, "%1.1s", NULL, STRING, 1, "mset %s affected sanctuary"
},
{
	"2", "h", 10, 29, "%1.1s", NULL, STRING, 1, "mset %s affected faerie_fire"
},
{
	"2", "i", 10, 47, "%1.1s", NULL, STRING, 1, "mset %s affected infrared"
},
{
	"2", "j", 10, 63, "%1.1s", NULL, STRING, 1, "mset %s affected curse"
},
{
	"2", "k", 11, 12, "%1.1s", NULL, STRING, 1, "mset %s affected flaming"
},
{
	"2", "l", 11, 29, "%1.1s", NULL, STRING, 1, "mset %s affected poison"
},
{
	"2", "m", 11, 47, "%1.1s", NULL, STRING, 1, "mset %s affected protect"
},
{
	"2", "n", 11, 63, "%1.1s", NULL, STRING, 1, "mset %s affected _paralysis"
},
{
	"2", "o", 12, 12, "%1.1s", NULL, STRING, 1, "mset %s affected sleep"
},
{
	"2", "p", 12, 29, "%1.1s", NULL, STRING, 1, "mset %s affected hide"
},
{
	"2", "q", 12, 47, "%1.1s", NULL, STRING, 1, "mset %s affected sneak"
},
{
	"2", "r", 12, 63, "%1.1s", NULL, STRING, 1, "mset %s affected charm"
},
{
	"2", "s", 13, 12, "%1.1s", NULL, STRING, 1, "mset %s affected flying"
},
{
	"2", "t", 13, 29, "%1.1s", NULL, STRING, 1, "mset %s affected pass_door"
},
{
	"2", "u", 13, 47, "%1.1s", NULL, STRING, 1, "mset %s affected truesight"
},
{
	"2", "v", 13, 63, "%1.1s", NULL, STRING, 1, "mset %s affected detect_traps"
},
{
	"2", "w", 14, 12, "%1.1s", NULL, STRING, 1, "mset %s affected scrying"
},
{
	"2", "x", 14, 29, "%1.1s", NULL, STRING, 1, "mset %s affected fireshield"
},
{
	"2", "y", 14, 47, "%1.1s", NULL, STRING, 1, "mset %s affected shockshield"
},
{
	"2", "z", 14, 63, "%1.1s", NULL, STRING, 1, "mset %s affected floating"
},
{
	"2", "A", 15, 12, "%1.1s", NULL, STRING, 1, "mset %s affected invisible"
},
{ "-", "a", 13, 2, "", NULL, STRING, 1, "mmenu %s a" },
{ "-", "b", 13, 2, "", NULL, STRING, 1, "mmenu %s b" },
{ "-", "c", 13, 2, "", NULL, STRING, 1, "mmenu %s c" },
{ "-", "d", 13, 2, "", NULL, STRING, 1, "mmenu %s d" },
{ "-", "e", 13, 2, "", NULL, STRING, 1, "mmenu %s e" },
{ "-", "f", 13, 2, "", NULL, STRING, 1, "mmenu %s f" },
{ "-", "h", 13, 2, "", NULL, STRING, 1, "mmenu %s h" },
{
	" ", " ", 0, 0, " ", NULL, (int)NULL, 1, NULL
}
};

MENU_DATA       mob_page_c_data[] =
{{
		"1", "a", 2, 26, "%1.1s", NULL, STRING, 1, "mset %s attack bite"
},
{
	"1", "b", 2, 42, "%1.1s", NULL, STRING, 1, "mset %s attack claws"
},
{
	"1", "c", 2, 68, "%1.1s", NULL, STRING, 1, "mset %s attack firebreath"
},
{
	"1", "d", 3, 26, "%1.1s", NULL, STRING, 1, "mset %s attack bash"
},
{
	"1", "e", 3, 42, "%1.1s", NULL, STRING, 1, "mset %s attack stun"
},
{
	"1", "f", 3, 68, "%1.1s", NULL, STRING, 1, "mset %s attack acidbreath"
},
{
	"1", "g", 4, 11, "%1.1s", NULL, STRING, 1, "mset %s attack drain"
},
{
	"1", "h", 4, 26, "%1.1s", NULL, STRING, 1, "mset %s attack kick"
},
{
	"1", "i", 4, 42, "%1.1s", NULL, STRING, 1, "mset %s attack feed"
},
{
	"1", "j", 4, 68, "%1.1s", NULL, STRING, 1, "mset %s attack gasbreath"
},
{
	"1", "k", 5, 11, "%1.1s", NULL, STRING, 1, "mset %s attack poison"
},
{
	"1", "l", 5, 26, "%1.1s", NULL, STRING, 1, "mset %s attack gaze"
},
{
	"1", "m", 5, 42, "%1.1s", NULL, STRING, 1, "mset %s attack harm"
},
{
	"1", "n", 5, 68, "%1.1s", NULL, STRING, 1, "mset %s attack frostbreath"
},
{
	"1", "o", 6, 11, "%1.1s", NULL, STRING, 1, "mset %s attack curse"
},
{
	"1", "p", 6, 26, "%1.1s", NULL, STRING, 1, "mset %s attack backstab"
},
{
	"1", "q", 6, 42, "%1.1s", NULL, STRING, 1, "mset %s attack punch"
},
{
	"1", "r", 6, 68, "%1.1s", NULL, STRING, 1, "mset %s attack lightnbreath"
},
{
	"1", "s", 7, 11, "%1.1s", NULL, STRING, 1, "mset %s attack weaken"
},
{
	"1", "t", 7, 26, "%1.1s", NULL, STRING, 1, "mset %s attack tail"
},
{
	"1", "u", 7, 42, "%1.1s", NULL, STRING, 1, "mset %s attack sting"
},
{
	"1", "v", 7, 68, "%1.1s", NULL, STRING, 1, "mset %s attack causeserious"
},
{
	"1", "w", 8, 11, "%1.1s", NULL, STRING, 1, "mset %s attack trip"
},
{
	"1", "x", 8, 26, "%1.1s", NULL, STRING, 1, "mset %s attack fireball"
},
{
	"1", "y", 8, 42, "%1.1s", NULL, STRING, 1, "mset %s attack gouge"
},
{
	"1", "z", 8, 68, "%1.1s", NULL, STRING, 1, "mset %s attack causecritical"
},
{
	"1", "B", 9, 29, "%1.1s", NULL, STRING, 1, "mset %s attack flamestrike"
},
{
	"1", "D", 9, 68, "%1.1s", NULL, STRING, 1, "mset %s attack earthquake"
},
{
	"1", "F", 10, 29, "%1.1s", NULL, STRING, 1, "mset %s attack colorspray"
},
{
	"1", "G", 10, 48, "%1.1s", NULL, STRING, 1, "mset %s attack nasty"
},
{
	"1", "H", 10, 68, "%1.1s", NULL, STRING, 1, "mset %s attack blindness"
},
{
	"2", "a", 12, 31, "%1.1s", NULL, STRING, 1, "mset %s defense sanctuary"
},
{
	"2", "b", 12, 51, "%1.1s", NULL, STRING, 1, "mset %s defense curelight"
},
{
	"2", "c", 12, 70, "%1.1s", NULL, STRING, 1, "mset %s defense dispelevil"
},
{
	"2", "d", 13, 31, "%1.1s", NULL, STRING, 1, "mset %s defense shield"
},
{
	"2", "e", 13, 51, "%1.1s", NULL, STRING, 1, "mset %s defense cureserious"
},
{
	"2", "f", 13, 70, "%1.1s", NULL, STRING, 1, "mset %s defense dispelmagic"
},
{
	"2", "g", 14, 11, "%1.1s", NULL, STRING, 1, "mset %s defense dodge"
},
{
	"2", "h", 14, 31, "%1.1s", NULL, STRING, 1, "mset %s defense fireshield"
},
{
	"2", "i", 14, 51, "%1.1s", NULL, STRING, 1, "mset %s defense curecritical"
},
{
	"2", "j", 14, 70, "%1.1s", NULL, STRING, 1, "mset %s defense teleport"
},
{
	"2", "k", 15, 11, "%1.1s", NULL, STRING, 1, "mset %s defense parry"
},
{
	"2", "l", 15, 31, "%1.1s", NULL, STRING, 1, "mset %s defense shockshield"
},
{
	"2", "m", 15, 51, "%1.1s", NULL, STRING, 1, "mset %s defense heal"
},
{
	"2", "n", 15, 70, "%1.1s", NULL, STRING, 1, "mset %s defense bless"
},
{
	"2", "o", 16, 11, "%1.1s", NULL, STRING, 1, "mset %s defense disarm"
},
{
	"2", "p", 16, 31, "%1.1s", NULL, STRING, 1, "mset %s defense stoneskin"
},
{
	"2", "q", 16, 51, "%1.1s", NULL, STRING, 1, "mset %s defense monsum1"
},
{
	"2", "r", 16, 70, "%1.1s", NULL, STRING, 1, "mset %s defense monsum2"
},
{
	"2", "u", 17, 51, "%1.1s", NULL, STRING, 1, "mset %s defense monsum3"
},
{
	"2", "v", 17, 70, "%1.1s", NULL, STRING, 1, "mset %s defense monsum4"
},
{ "-", "a", 13, 2, "", NULL, STRING, 1, "mmenu %s a" },
{ "-", "b", 13, 2, "", NULL, STRING, 1, "mmenu %s b" },
{ "-", "c", 13, 2, "", NULL, STRING, 1, "mmenu %s c" },
{ "-", "d", 13, 2, "", NULL, STRING, 1, "mmenu %s d" },
{ "-", "e", 13, 2, "", NULL, STRING, 1, "mmenu %s e" },
{ "-", "f", 13, 2, "", NULL, STRING, 1, "mmenu %s f" },
{ "-", "h", 13, 2, "", NULL, STRING, 1, "mmenu %s h" },
{
	" ", " ", 0, 0, " ", NULL, (int)NULL, 1, NULL
}
};

MENU_DATA       mob_page_d_data[] =
{{
		"1", "a", 2, 36, "%+-2.2hd", NULL, SH_INT, 2, "mset %s sav1 %s"
},
{
	"1", "b", 2, 63, "%+-2.2d", NULL, SH_INT, 2, "mset %s sav2 %s"
},
{
	"1", "c", 3, 36, "%+-2.2hd", NULL, SH_INT, 2, "mset %s sav3 %s"
},
{
	"1", "d", 3, 63, "%+-2.2hd", NULL, SH_INT, 2, "mset %s sav4 %s"
},
{
	"1", "e", 4, 36, "%+-2.2hd", NULL, SH_INT, 2, "mset %s sav5 %s"
},
{
	"2", "a", 6, 20, "%1.1s", NULL, STRING, 1, "mset %s part fins"
},
{
	"2", "b", 6, 31, "%1.1s", NULL, STRING, 1, "mset %s part ear"
},
{
	"2", "c", 6, 42, "%1.1s", NULL, STRING, 1, "mset %s part eye"
},
{
	"2", "d", 6, 59, "%1.1s", NULL, STRING, 1, "mset %s part long_tongue"
},
{
	"2", "e", 6, 72, "%1.1s", NULL, STRING, 1, "mset %s part scales"
},
{
	"2", "f", 7, 20, "%1.1s", NULL, STRING, 1, "mset %s part tail"
},
{
	"2", "g", 7, 31, "%1.1s", NULL, STRING, 1, "mset %s part arms"
},
{
	"2", "h", 7, 42, "%1.1s", NULL, STRING, 1, "mset %s part legs"
},
{
	"2", "i", 7, 59, "%1.1s", NULL, STRING, 1, "mset %s part tentacles"
},
{
	"2", "j", 7, 72, "%1.1s", NULL, STRING, 1, "mset %s part brains"
},
{
	"2", "k", 8, 20, "%1.1s", NULL, STRING, 1, "mset %s part guts"
},
{
	"2", "l", 8, 31, "%1.1s", NULL, STRING, 1, "mset %s part head"
},
{
	"2", "m", 8, 42, "%1.1s", NULL, STRING, 1, "mset %s part feet"
},
{
	"2", "n", 8, 59, "%1.1s", NULL, STRING, 1, "mset %s part eyestalks"
},
{
	"2", "o", 8, 72, "%1.1s", NULL, STRING, 1, "mset %s part fingers"
},
{
	"2", "p", 9, 21, "%1.1s", NULL, STRING, 1, "mset %s part wings"
},
{
	"2", "q", 9, 32, "%1.1s", NULL, STRING, 1, "mset %s part hands"
},
{
	"2", "r", 9, 43, "%1.1s", NULL, STRING, 1, "mset %s part heart"
},
{
	"2", "u", 10, 21, "%1.1s", NULL, STRING, 1, "mset %s part claws"
},
{
	"2", "v", 10, 32, "%1.1s", NULL, STRING, 1, "mset %s part horns"
},
{
	"2", "w", 10, 43, "%1.1s", NULL, STRING, 1, "mset %s part tusks"
},
{
	"2", "z", 11, 26, "%1.1s", NULL, STRING, 1, "mset %s part tail-attack"
},
{
	"2", "A", 11, 49, "%1.1s", NULL, STRING, 1, "mset %s part sharp-scales"
},
{
	"3", "a", 13, 27, "%3.3hd", NULL, SH_INT, 2, "mset %s numattacks %s"
},
{
	"3", "b", 13, 48, "%s", NULL, STRING, 2, "mset %s hitdie %s"
},
{
	"3", "c", 14, 25, "%5.5hd", NULL, SH_INT, 2, "mset %s hitroll %s"
},
{
	"3", "d", 15, 25, "%5.5hd", NULL, SH_INT, 2, "mset %s damroll %s"
},
{
	"3", "e", 15, 48, "%s", NULL, STRING, 2, "mset %s damdie %s"
},
{ "-", "a", 13, 2, "", NULL, STRING, 1, "mmenu %s a" },
{ "-", "b", 13, 2, "", NULL, STRING, 1, "mmenu %s b" },
{ "-", "c", 13, 2, "", NULL, STRING, 1, "mmenu %s c" },
{ "-", "d", 13, 2, "", NULL, STRING, 1, "mmenu %s d" },
{ "-", "e", 13, 2, "", NULL, STRING, 1, "mmenu %s e" },
{ "-", "f", 13, 2, "", NULL, STRING, 1, "mmenu %s f" },
{ "-", "h", 13, 2, "", NULL, STRING, 1, "mmenu %s h" },
{
	" ", " ", 0, 0, " ", NULL, (int)NULL, 0, NULL
}
};

MENU_DATA       mob_page_e_data[] =
{{
		"1", "a", 2, 23, "%1.1s", NULL, STRING, 1, "mset %s spec spec_fido"
},
{
	"1", "b", 2, 42, "%1.1s", NULL, STRING, 1, "mset %s spec spec_cast_adept"
},
{
	"1", "c", 2, 70, "%1.1s", NULL, STRING, 1, "mset %s spec spec_breath_fire"
},
{
	"1", "d", 3, 23, "%1.1s", NULL, STRING, 1, "mset %s spec spec_guard"
},
{
	"1", "e", 3, 42, "%1.1s", NULL, STRING, 1, "mset %s spec spec_cast_cleric"
},
{
	"1", "f", 3, 70, "%1.1s", NULL, STRING, 1, "mset %s spec spec_breath_frost"
},
{
	"1", "g", 4, 23, "%1.1s", NULL, STRING, 1, "mset %s spec spec_janitor"
},
{
	"1", "h", 4, 42, "%1.1s", NULL, STRING, 1, "mset %s spec spec_cast_mage"
},
{
	"1", "i", 4, 70, "%1.1s", NULL, STRING, 1, "mset %s spec spec_breath_acid"
},
{
	"1", "j", 5, 23, "%1.1s", NULL, STRING, 1, "mset %s spec spec_thief"
},
{
	"1", "k", 5, 42, "%1.1s", NULL, STRING, 1, "mset %s spec spec_cast_undead"
},
{
	"1", "l", 5, 70, "%1.1s", NULL, STRING, 1, "mset %s spec spec_breath_gas"
},
{
	"1", "m", 6, 23, "%1.1s", NULL, STRING, 1, "mset %s spec spec_mayor"
},
{
	"1", "o", 6, 70, "%1.1s", NULL, STRING, 1, "mset %s spec spec_breath_lightning"
},
{
	"1", "p", 7, 23, "%1.1s", NULL, STRING, 1, "mset %s spec spec_poison"
},
{
	"1", "q", 7, 42, "%1.1s", NULL, STRING, 1, "mset %s spec spec_executioner"
},
{
	"1", "r", 7, 70, "%1.1s", NULL, STRING, 1, "mset %s spec spec_breath_any"
},



{
	"2", "a", 9, 28, "%1.1s", NULL, STRING, 1, "mset %s defpos 0"
},
{
	"2", "b", 9, 44, "%1.1s", NULL, STRING, 1, "mset %s defpos 1"
},
{
	"2", "c", 9, 66, "%1.1s", NULL, STRING, 1, "mset %s defpos 2"
},
{
	"2", "d", 10, 28, "%1.1s", NULL, STRING, 1, "mset %s defpos 3"
},
{
	"2", "e", 10, 44, "%1.1s", NULL, STRING, 1, "mset %s defpos 4"
},
{
	"2", "f", 10, 66, "%1.1s", NULL, STRING, 1, "mset %s defpos 5"
},
{
	"2", "g", 11, 28, "%1.1s", NULL, STRING, 1, "mset %s defpos 6"
},
{
	"2", "h", 11, 44, "%1.1s", NULL, STRING, 1, "mset %s defpos 7"
},
{
	"2", "i", 11, 66, "%1.1s", NULL, STRING, 1, "mset %s defpos 8"
},
{
	"2", "k", 12, 28, "%1.1s", NULL, STRING, 1, "mset %s defpos not_implemented"
},

{
	"3", "a", 14, 22, "%3.3s", NULL, STRING, 2, "mset %s %s fire"
},
{
	"3", "b", 14, 34, "%3.3s", NULL, STRING, 2, "mset %s %s cold"
},
{
	"3", "c", 14, 46, "%3.3s", NULL, STRING, 2, "mset %s %s electricity"
},
{
	"3", "d", 14, 58, "%3.3s", NULL, STRING, 2, "mset %s %s energy"
},
{
	"3", "e", 14, 70, "%3.3s", NULL, STRING, 2, "mset %s %s poison"
},
{
	"3", "f", 15, 22, "%3.3s", NULL, STRING, 2, "mset %s %s blunt"
},
{
	"3", "g", 15, 34, "%3.3s", NULL, STRING, 2, "mset %s %s pierce"
},
{
	"3", "h", 15, 46, "%3.3s", NULL, STRING, 2, "mset %s %s slash"
},
{
	"3", "i", 15, 58, "%3.3s", NULL, STRING, 2, "mset %s %s acid"
},
{
	"3", "j", 15, 70, "%3.3s", NULL, STRING, 2, "mset %s %s nonmagic"
},
{
	"3", "k", 16, 22, "%3.3s", NULL, STRING, 2, "mset %s %s drain"
},
{
	"3", "l", 16, 34, "%3.3s", NULL, STRING, 2, "mset %s %s sleep"
},
{
	"3", "m", 16, 46, "%3.3s", NULL, STRING, 2, "mset %s %s charm"
},
{
	"3", "n", 16, 58, "%3.3s", NULL, STRING, 2, "mset %s %s hold"
},
{
	"3", "o", 16, 70, "%3.3s", NULL, STRING, 2, "mset %s %s magic"
},
{
	"3", "p", 17, 10, "%3.3s", NULL, STRING, 2, "mset %s %s plus1"
},
{
	"3", "q", 17, 22, "%3.3s", NULL, STRING, 2, "mset %s %s plus2"
},
{
	"3", "r", 17, 34, "%3.3s", NULL, STRING, 2, "mset %s %s plus3"
},
{
	"3", "s", 17, 46, "%3.3s", NULL, STRING, 2, "mset %s %s plus4"
},
{
	"3", "t", 17, 58, "%3.3s", NULL, STRING, 2, "mset %s %s plus5"
},
{
	"3", "u", 17, 70, "%3.3s", NULL, STRING, 2, "mset %s %s plus6"
},
{ "-", "a", 13, 2, "", NULL, STRING, 1, "mmenu %s a" },
{ "-", "b", 13, 2, "", NULL, STRING, 1, "mmenu %s b" },
{ "-", "c", 13, 2, "", NULL, STRING, 1, "mmenu %s c" },
{ "-", "d", 13, 2, "", NULL, STRING, 1, "mmenu %s d" },
{ "-", "e", 13, 2, "", NULL, STRING, 1, "mmenu %s e" },
{ "-", "f", 13, 2, "", NULL, STRING, 1, "mmenu %s f" },
{ "-", "h", 13, 2, "", NULL, STRING, 1, "mmenu %s h" },
{
	" ", " ", 0, 0, " ", NULL, (int)NULL, 0, NULL
}
};

MENU_DATA       control_page_a_data[] =
{
{
	"1", "a", 4, 16, "%3.3hd", NULL, SH_INT, 1, "cset read_free %s"
},
{
	"1", "b", 5, 16, "%3.3hd", NULL, SH_INT, 1, "cset write_free %s"
},
{
	"1", "c", 6, 16, "%3.3hd", NULL, SH_INT, 1, "cset read_all %s"
},
{
	"1", "d", 7, 16, "%3.3hd", NULL, SH_INT, 1, "cset take_all %s"
},
{
	"2", "a", 4, 30, "%3.3hd", NULL, SH_INT, 1, "cset muse_level %s"
},
{
	"2", "b", 5, 30, "%3.3hd", NULL, SH_INT, 1, "cset think_level %s"
},
{
	"2", "c", 6, 30, "%3.3hd", NULL, SH_INT, 1, "cset log_level %s"
},
{
	"2", "d", 7, 30, "%3.3hd", NULL, SH_INT, 1, "cset build_level %s"
},
{ "-", "a", 13, 2, "", NULL, STRING, 0, "cmenu a" },
{ "-", "h", 13, 2, "", NULL, STRING, 0, "cmenu h" },
{
	" ", " ", 0, 0, " ", NULL, (int) NULL, 0, NULL
}
};

/*
 * from MERC2.2, but i use it to define the scrolling region of the screen
 */
void 
do_pagelen(CHAR_DATA * ch, char *argument)
{
	char            buf[MAX_STRING_LENGTH];
	char            arg[MAX_INPUT_LENGTH];
	int             lines;	/* don't wanna muck about with atoi and
				 * sh_int */

	one_argument(argument, arg);

	if (arg[0] == '\0')
		lines = 24;
	else
		lines = atoi(arg);

	if (lines < 1) {
		send_to_char("Negative or Zero values for a page length is not legal.\n\r", ch);
		return;
	}
	ch->pagelen = lines;
	sprintf(buf, "Page length set to %d lines.\n\r", lines);
	send_to_char(buf, ch);
	return;
}


char           *
get_ris(MOB_INDEX_DATA * idx, int risFlag)
{
	int             num = 0;

	if (IS_SET(idx->resistant, risFlag))
		num += 1;
	if (IS_SET(idx->immune, risFlag))
		num += 2;
	if (IS_SET(idx->susceptible, risFlag))
		num += 4;

	return ((char *) ris_strings[num]);	/* see const.c */
}

int 
get_offset(CHAR_DATA * ch)
{
	switch (ch->inter_page) {
		case MOB_PAGE_A:
		return 18;
	case MOB_PAGE_B:
		return 18;
	case MOB_PAGE_C:
		return 20;
	case MOB_PAGE_D:
		return 18;
	case MOB_PAGE_E:
		return 19;
	case MOB_PAGE_F:
		return 22;
	case MOB_HELP_PAGE:
		return 13;
	case ROOM_PAGE_A:
		return 10;
	case ROOM_PAGE_B:
		return 15;
	case ROOM_PAGE_C:
		return 18;
	case ROOM_HELP_PAGE:
		return 13;
	case OBJ_PAGE_A:
		return 16;
	case OBJ_PAGE_B:
		return 18;
	case OBJ_PAGE_C:
		return 20;
	case OBJ_PAGE_D:
		return 18;
	case OBJ_PAGE_E:
		return 19;
	case OBJ_HELP_PAGE:
		return 13;
	case CONTROL_PAGE_A:
		return 9;
	case CONTROL_HELP_PAGE:
		return 13;
	default:
		return 0;
	}
}


/*
 * Send a room editing screen
 */
void 
send_room_page_to_char(CHAR_DATA * ch, ROOM_INDEX_DATA * idx, char page)
{
	int             offset;
	char            menu_picture[60 * 80];
        char            buf[MAX_STRING_LENGTH];

	if (!ch) {
		bug("Send_room_page_to_char: NULL *ch", 0);
		return;
	}
	if (!ch->desc) {
		bug("Send_room_page_to_char: NULL ch->desc", 0);
		return;
	}
	switch (page) {
#ifdef NEW_MAPS
	case 'd':
		format_string(buf,ch->inter_editing,10);
		sprintf(menu_picture, room_page_d, buf);
		ch->inter_page = ROOM_PAGE_D;
		break;
#endif
	case 'h':
		format_string(buf,ch->inter_editing,10);
		sprintf(menu_picture, room_help_page, buf);
		ch->inter_page = ROOM_HELP_PAGE;
		break;
	case 'c':
		format_string(buf,ch->inter_editing,10);
		sprintf(menu_picture, room_page_c, buf);
		ch->inter_page = ROOM_PAGE_C;
		break;
	case 'b':
		format_string(buf,ch->inter_editing,10);
		sprintf(menu_picture, room_page_b, buf);
		ch->inter_page = ROOM_PAGE_B;
		break;
	case 'a':
	default:
		format_string(buf,ch->inter_editing,10);
		sprintf(menu_picture, room_page_a, buf);
		ch->inter_page = ROOM_PAGE_A;
		break;
	}
	offset = get_offset(ch);

	/* clear page & home cursor */
	write_to_buffer(ch->desc, "50\x1B[;H\x1B[2J", strlen("50\x1B[;H\x1B[2J"));

	/* prints the necessary mob_page */
	write_to_buffer(ch->desc, menu_picture, strlen(menu_picture));

	/* fill in fields */
	fill_in_room_page(ch, idx, page);
	return;
}

/*
 * Send a control editing screen
 */
void 
send_control_page_to_char(CHAR_DATA * ch, char page)
{
	int             offset;
	char            menu_picture[60 * 80];
        char            buf[MAX_STRING_LENGTH];

	if (!ch) {
		bug("Send_control_page_to_char: NULL *ch", 0);
		return;
	}
	if (!ch->desc) {
		bug("Send_control_page_to_char: NULL ch->desc", 0);
		return;
	}
	switch (page) {
	case 'h':
		format_string(buf,ch->inter_editing,10);
		sprintf(menu_picture, control_help_page, buf);
		ch->inter_page = CONTROL_HELP_PAGE;
		break;
	case 'a':
	default:
		format_string(buf,ch->inter_editing,10);
		sprintf(menu_picture, control_page_a, buf);
		ch->inter_page = CONTROL_PAGE_A;
		break;
	}
	offset = get_offset(ch);

	/* clear page & home cursor */
	write_to_buffer(ch->desc, "50\x1B[;H\x1B[2J", strlen("50\x1B[;H\x1B[2J"));

	/* prints the necessary control_page */
	write_to_buffer(ch->desc, menu_picture, strlen(menu_picture));

	/* fill in fields */
	fill_in_control_page(ch, page);
	return;
}

void 
do_clear(CHAR_DATA * ch, char *argument)
{
	char            buf[MAX_STRING_LENGTH];
	if (!ch->desc) {
		bug("No desc in do_clear\n\r", 0);
	}
	ch->inter_editing = NO_PAGE;
	sprintf(buf, "\x1B[%d;%dr", 1, ch->pagelen);
	write_to_buffer(ch->desc, buf, strlen(buf));
	write_to_buffer(ch->desc, "50\x1B[;H\x1B[2J", strlen("50\x1B[;H\x1B[2J"));
	return;
}


/*
 * Doesn't allocate any memory!  Rehack of strip_cr
 */
void 
format_string(char output[], char *str, int length)
{
	char            newstr[MAX_STRING_LENGTH];
	int             l, i, j, slen;
	bool            elipsis;

	if ((length < 0) || (length > MAX_STRING_LENGTH)) {
		bug("format_string: bad length %d\n\r", length);
		l = 72;
	} else {
		l = length;
	}

	slen = strlen(str);

	if (slen < 1) {
		sprintf(output, "%-s", "(blank)");
	}
	if (slen > l) {
		l = l - 3;
		elipsis = TRUE;
	} else {
		elipsis = FALSE;
	}

	for (i = j = 0; str[i] != '\0' && i <= l; i++)
		if ((str[i] != '\r') && (str[i] != '\n')) {
			newstr[j++] = str[i];
		}
	if (elipsis) {
		newstr[j++] = '.';
		newstr[j++] = '.';
		newstr[j++] = '.';
	}
	newstr[j] = '\0';
	sprintf(output, "%-s", newstr);
	/* return newstr; */

}

/*
 * Send an obj editing screen
 */
void 
send_obj_page_to_char(CHAR_DATA * ch, OBJ_INDEX_DATA * idx, char page)
{
	int             offset;
	char            menu_picture[60 * 80], buf[MAX_STRING_LENGTH];	/* maximum menu size */

	if (!ch) {
		bug("Send_page_to_char: NULL *ch", 0);
		return;
	}
	if (!ch->desc) {
		bug("Send_page_to_char: NULL ch->desc", 0);
		return;
	}
	switch (page) {
	case 'h':
		format_string(buf,ch->inter_editing,10);
		sprintf(menu_picture, obj_help_page, buf);
		ch->inter_page = OBJ_HELP_PAGE;
		break;
	case 'e':
		format_string(buf,ch->inter_editing,10);
		sprintf(menu_picture, obj_page_e, buf);
		ch->inter_page = OBJ_PAGE_E;
		break;
	case 'd':
		format_string(buf,ch->inter_editing,10);
		sprintf(menu_picture, obj_page_d, buf);
		ch->inter_page = OBJ_PAGE_D;
		break;
	case 'c':
		format_string(buf,ch->inter_editing,10);
		sprintf(menu_picture, obj_page_c, buf);
		ch->inter_page = OBJ_PAGE_C;
		break;
	case 'b':
		format_string(buf,ch->inter_editing,10);
		sprintf(menu_picture, obj_page_b, buf);
		ch->inter_page = OBJ_PAGE_B;
		break;
	case 'a':
	default:
		format_string(buf,ch->inter_editing,10);
		sprintf(menu_picture, obj_page_a, buf);
		ch->inter_page = OBJ_PAGE_A;
		break;
	}
	offset = get_offset(ch);



	/* clear page & home cursor */
	write_to_buffer(ch->desc, "50\x1B[;H\x1B[2J", strlen("50\x1B[;H\x1B[2J"));

	/* prints the necessary mob_page */
	write_to_buffer(ch->desc, menu_picture, strlen(menu_picture));

	/* fill in fields */
	fill_in_obj_page(ch, idx, page);
	return;
}
/*
 * Send a (mob) editing screen
 */
void 
send_page_to_char(CHAR_DATA * ch, MOB_INDEX_DATA * idx, char page)
{
	int             offset;
	char            menu_picture[60 * 80], buf[MAX_STRING_LENGTH];

	if (!ch) {
		bug("Send_page_to_char: NULL *ch", 0);
		return;
	}
	if (!ch->desc) {
		bug("Send_page_to_char: NULL ch->desc", 0);
		return;
	}
	switch (page) {
	case 'h':
		format_string(buf,ch->inter_editing,10);
		sprintf(menu_picture, mob_help_page, buf);
		ch->inter_page = MOB_HELP_PAGE;
		break;
	case 'f':
		format_string(buf,ch->inter_editing,10);
		sprintf(menu_picture, mob_page_f, buf);
		ch->inter_page = MOB_PAGE_F;
		break;
	case 'e':
		format_string(buf,ch->inter_editing,10);
		sprintf(menu_picture, mob_page_e, buf);
		ch->inter_page = MOB_PAGE_E;
		break;
	case 'd':
		format_string(buf,ch->inter_editing,10);
		sprintf(menu_picture, mob_page_d, buf);
		ch->inter_page = MOB_PAGE_D;
		break;
	case 'c':
		format_string(buf,ch->inter_editing,10);
		sprintf(menu_picture, mob_page_c, buf);
		ch->inter_page = MOB_PAGE_C;
		break;
	case 'b':
		format_string(buf,ch->inter_editing,10);
		sprintf(menu_picture, mob_page_b, buf);
		ch->inter_page = MOB_PAGE_B;
		break;
	case 'a':
	default:
		format_string(buf,ch->inter_editing,10);
		sprintf(menu_picture, mob_page_a, buf);
		ch->inter_page = MOB_PAGE_A;
		break;
	}
	offset = get_offset(ch);



	/* clear page & home cursor */
	write_to_buffer(ch->desc, "50\x1B[;H\x1B[2J", strlen("50\x1B[;H\x1B[2J"));

	/* prints the necessary mob_page */
	write_to_buffer(ch->desc, menu_picture, strlen(menu_picture));

	/* fill in fields */
	fill_in_mob_page(ch, idx, page);
	return;
}

void 
do_redraw_page(CHAR_DATA * ch, char *argument)
{
	char            page;
	MOB_INDEX_DATA *idx;
	OBJ_INDEX_DATA *oidx;
        ROOM_INDEX_DATA *ridx;

	switch (ch->inter_page) {
	case MOB_PAGE_A:
		page = 'a';
		break;
	case MOB_PAGE_B:
		page = 'b';
		break;
	case MOB_PAGE_C:
		page = 'c';
		break;
	case MOB_PAGE_D:
		page = 'd';
		break;
	case MOB_PAGE_E:
		page = 'e';
		break;
	case MOB_PAGE_F:
		page = 'f';
		break;
	case MOB_HELP_PAGE:
		page = 'h';
		break;
	case OBJ_PAGE_A:
		page = 'a';
		break;
	case OBJ_PAGE_B:
		page = 'b';
		break;
	case OBJ_PAGE_C:
		page = 'c';
		break;
	case OBJ_PAGE_D:
		page = 'd';
		break;
	case OBJ_PAGE_E:
		page = 'e';
		break;
	case OBJ_HELP_PAGE:
		page = 'h';
		break;
	case ROOM_PAGE_A:
		page = 'a';
		break;
	case ROOM_PAGE_B:
		page = 'b';
		break;
	case ROOM_PAGE_C:
		page = 'c';
		break;
	case CONTROL_PAGE_A:
		page = 'a';
		break;
	case CONTROL_HELP_PAGE:
		page = 'h';
		break;
	case NO_PAGE:
	default:
		return;
	}

	if(ch->inter_type== MOB_TYPE)
	{
	   idx = get_mob_index(ch->inter_editing_vnum);
	   if (idx == NULL) {
		   bug("do_redraw_page: Get_mob_index: bad vnum %d.", ch->inter_editing_vnum);
		   send_to_char(" do_redraw_page: bizarre bug!  that vnum evaporated \n\r", ch);
	   }

	   send_page_to_char(ch, idx, page);
	   return;
        }

	if(ch->inter_type== ROOM_TYPE)
	{
	   ridx = get_room_index(ch->inter_editing_vnum);
	   if (ridx == NULL) {
		   bug("do_redraw_page: Get_room_index: bad vnum %d.", ch->inter_editing_vnum);
		   send_to_char(" do_redraw_page: bizarre bug!  that vnum evaporated \n\r", ch);
	   }

	   send_room_page_to_char(ch, ridx, page);
	   return;
        }


	if(ch->inter_type== OBJ_TYPE)
	{
	   oidx = get_obj_index(ch->inter_editing_vnum);
	   if (oidx == NULL) {
		   bug("do_redraw_page: Get_obj_index: bad vnum %d.", ch->inter_editing_vnum);
		   send_to_char(" do_redraw_page: bizarre bug!  that vnum evaporated \n\r", ch);
	   }

	   send_obj_page_to_char(ch, oidx, page);
	   return;
        }
        
        if(ch->inter_type== CONTROL_TYPE)
        {
           send_control_page_to_char(ch, page);
           return;
        }

        bug("do_redraw_page: Bad ch->inter_type: vnum %d.", ch->inter_editing_vnum);
}

void 
do_refresh_page(CHAR_DATA * ch, char *argument)
{

	refresh_page(ch);

	return;
}


void 
refresh_page(CHAR_DATA * ch)
{
	char            page;
	void           *idx;
	switch (ch->inter_type) {
	case MOB_TYPE:
		switch (ch->inter_page) {
		case MOB_PAGE_A:
			page = 'a';
			break;
		case MOB_PAGE_B:
			page = 'b';
			break;
		case MOB_PAGE_C:
			page = 'c';
			break;
		case MOB_PAGE_D:
			page = 'd';
			break;
		case MOB_PAGE_E:
			page = 'e';
			break;
		case MOB_PAGE_F:
			page = 'f';
			break;
		case MOB_HELP_PAGE:
		case NO_PAGE:
		default:
			return;
		}
		idx = (MOB_INDEX_DATA *) get_mob_index(ch->inter_editing_vnum);
		if (idx == NULL) {
			bug("refresh_page: Get_mob_index: bad vnum %d.", ch->inter_editing_vnum);
			send_to_char(" do_refresh_page: bizarre bug!  that vnum evaporated \n\r", ch);
		}
		fill_in_mob_page(ch, idx, page);
		return;
	case ROOM_TYPE:
		switch (ch->inter_page) {
		case ROOM_PAGE_A:
			page = 'a';
			break;
		case ROOM_PAGE_B:
			page = 'b';
			break;
		case ROOM_PAGE_C:
			page = 'c';
			break;
		case ROOM_HELP_PAGE:
		case NO_PAGE:
		default:
			return;
		}
		idx = (ROOM_INDEX_DATA *) get_room_index(ch->inter_editing_vnum);
		if (idx == NULL) {
			bug("refresh_page: Get_room_index: bad vnum %d.", ch->inter_editing_vnum);
			send_to_char(" do_refresh_page: bizarre bug!  that vnum evaporated \n\r", ch);
		}
		fill_in_room_page(ch, idx, page);
		return;
	case OBJ_TYPE:
		switch (ch->inter_page) {
		case OBJ_PAGE_A:
			page = 'a';
			break;
		case OBJ_PAGE_B:
			page = 'b';
			break;
		case OBJ_PAGE_C:
			page = 'c';
			break;
		case OBJ_PAGE_D:
			page = 'd';
			break;
		case OBJ_PAGE_E:
			page = 'e';
			break;
		case OBJ_HELP_PAGE:
		case NO_PAGE:
		default:
			return;
		}
		idx = (OBJ_INDEX_DATA *) get_obj_index(ch->inter_editing_vnum);
		if (idx == NULL) {
			bug("refresh_page: Get_obj_index: bad vnum %d.", ch->inter_editing_vnum);
			send_to_char(" do_refresh_page: bizarre bug!  that vnum evaporated \n\r", ch);
		}
		fill_in_obj_page(ch, idx, page);
		return;
	case CONTROL_TYPE:
		switch (ch->inter_page) {
		case CONTROL_PAGE_A:
			page = 'a';
			break;
		case CONTROL_HELP_PAGE:
			page = 'h';
			break;
		case NO_PAGE:
		default:
			return;
		}		

	default:
		return;
	}
}


void 
fill_in_control_page(CHAR_DATA * ch, char page)
{
	MENU_DATA      *m_data;
	char            buf[MAX_STRING_LENGTH];
	int             i, offset;

	if ((ch->inter_substate < 0) || (ch->inter_substate > 5)) {
		send_to_char("Bad interface substate. Can't fill in page.\n\r", ch);
		return;
	}
	switch (page) {
	case 'h':  return;
	default:
	case 'a':
		m_data = control_page_a_data;
		control_page_a_data[0].data = &(sysdata.read_mail_free);
		control_page_a_data[1].data = &(sysdata.write_mail_free);
		control_page_a_data[2].data = &(sysdata.read_all_mail);
		control_page_a_data[3].data = &(sysdata.take_others_mail);
		control_page_a_data[4].data = &(sysdata.muse_level);
		control_page_a_data[5].data = &(sysdata.think_level);
		control_page_a_data[6].data = &(sysdata.log_level);
		control_page_a_data[7].data = &(sysdata.build_level);


	}

	for (i = 0; m_data[i].ptrType != (int)NULL; i++) {
		sprintf(buf, "\x1B[%d;%dH", m_data[i].x, m_data[i].y);
		write_to_buffer(ch->desc, buf, strlen(buf));
		switch(m_data[i].ptrType)
		{
                    case INT:    sprintf(buf, m_data[i].outFormat,*((int *)(m_data[i].data)));
			         break;
                    case SH_INT: sprintf(buf, m_data[i].outFormat,*((sh_int *)(m_data[i].data)));
			         break;
                    default    : sprintf(buf, m_data[i].outFormat,  m_data[i].data);
                }
#ifdef WRONG_WAY
		if (m_data[i].ptrType == INT) {
			sprintf(buf, m_data[i].outFormat, *((int *) (m_data[i].data)));
		}
		if (m_data[i].ptrType == SH_INT) {
			sprintf(buf, m_data[i].outFormat, *((sh_int *) (m_data[i].data)));
		} else {
			sprintf(buf, m_data[i].outFormat, m_data[i].data);
		}
#endif
		write_to_buffer(ch->desc, buf, strlen(buf));
	}

	offset = get_offset(ch);

	/* restrict scrolling area of screen */
	sprintf(buf, "\x1B[%d;%dr", offset, ch->pagelen);
	write_to_buffer(ch->desc, buf, strlen(buf));

	/* goes to pglen,1 */
	sprintf(buf, "\x1B[%d;1H", (ch->pagelen));
	write_to_buffer(ch->desc, buf, strlen(buf));

	send_to_char("\n\r", ch);
}

void 
fill_in_room_page(CHAR_DATA * ch, ROOM_INDEX_DATA * idx, char page)
{
	MENU_DATA      *m_data;
	char            buf[MAX_STRING_LENGTH];
	OBJ_INDEX_DATA *pObjIndex;
	EXIT_DATA      *xit;
	char            tmp0[MAX_STRING_LENGTH];
	char            tmp1[MAX_STRING_LENGTH];
	int             i, offset;

	if ((ch->inter_substate < 0) || (ch->inter_substate > 5)) {
		send_to_char("Bad interface substate. Can't fill in page.\n\r", ch);
		return;
	}
	switch (page) {
	case 'h':  return;
	case 'c':
		m_data = room_page_c_data;
		room_page_c_data[0].data = (ch->inter_substate == SUB_NORTH) ? check : space;
		room_page_c_data[1].data = (ch->inter_substate == SUB_UP) ? check : space;
		room_page_c_data[2].data = (ch->inter_substate == SUB_EAST) ? check : space;
		room_page_c_data[3].data = (ch->inter_substate == SUB_WEST) ? check : space;
		room_page_c_data[4].data = (ch->inter_substate == SUB_SOUTH) ? check : space;
		room_page_c_data[5].data = (ch->inter_substate == SUB_DOWN) ? check : space;
		xit = get_exit(idx, ch->inter_substate);
		if (!xit) {
			room_page_c_data[6].data = (char *) space;
			room_page_c_data[7].data = (char *) space;
			room_page_c_data[8].data = (char *) space;
			room_page_c_data[9].data = (char *) space;
			room_page_c_data[10].data = (char *) space;
			room_page_c_data[11].data = (char *) space;
			room_page_c_data[12].data = (char *) space;
			room_page_c_data[13].data = (char *) "No Exit This Way";
			room_page_c_data[14].data = (char *) space;
			room_page_c_data[15].data = (char *) space;
			room_page_c_data[16].data = (char *) space;
		} else {
			room_page_c_data[6].data = IS_SET(xit->exit_info, EX_ISDOOR) ? check : space;
			/* SIC.!!! */
			room_page_c_data[7].data = IS_SET(xit->exit_info, EX_SECRET) ? check : space;
			room_page_c_data[8].data = IS_SET(xit->exit_info, EX_CLOSED) ? check : space;
			room_page_c_data[9].data = IS_SET(xit->exit_info, EX_PICKPROOF) ? check : space;
			room_page_c_data[10].data = IS_SET(xit->exit_info, EX_LOCKED) ? check : space;
			room_page_c_data[11].data = IS_SET(xit->exit_info, EX_NOPASSDOOR) ? check : space;
			room_page_c_data[12].data = &(xit->vnum);
			room_page_c_data[13].data = xit->to_room->name;
			room_page_c_data[14].data = &(xit->key);
			if ((pObjIndex = get_obj_index(xit->key)) == NULL) {
				room_page_c_data[15].data = "No (or Unknown) Key";
			} else {
				room_page_c_data[15].data = pObjIndex->name;
			}
			room_page_c_data[16].data = xit->description;
		}
		break;
	case 'b':
		m_data = room_page_b_data;
		room_page_b_data[0].data = xIS_SET(idx->room_flags, ROOM_DARK) ? check : space;
		room_page_b_data[1].data = xIS_SET(idx->room_flags, ROOM_DEATH) ? check : space;
		room_page_b_data[2].data = xIS_SET(idx->room_flags, ROOM_NO_MOB) ? check : space;
		room_page_b_data[3].data = xIS_SET(idx->room_flags, ROOM_INDOORS) ? check : space;
		room_page_b_data[4].data = xIS_SET(idx->room_flags, ROOM_HOUSE) ? check : space;
		room_page_b_data[5].data = xIS_SET(idx->room_flags, ROOM_NO_MAGIC) ? check : space;
		room_page_b_data[6].data = xIS_SET(idx->room_flags, ROOM_PET_SHOP) ? check : space;
		room_page_b_data[7].data = xIS_SET(idx->room_flags, ROOM_NEUTRAL) ? check : space;
		room_page_b_data[8].data = xIS_SET(idx->room_flags, ROOM_NODROP) ? check : space;
		room_page_b_data[9].data = xIS_SET(idx->room_flags, ROOM_SAFE) ? check : space;
		room_page_b_data[10].data = xIS_SET(idx->room_flags, ROOM_CHAOTIC) ? check : space;
		room_page_b_data[11].data = xIS_SET(idx->room_flags, ROOM_NODROPALL) ? check : space;
		room_page_b_data[12].data = xIS_SET(idx->room_flags, ROOM_TELEPORT) ? check : space;
		room_page_b_data[13].data = xIS_SET(idx->room_flags, ROOM_DONATION) ? check : space;
		room_page_b_data[14].data = xIS_SET(idx->room_flags, ROOM_NO_RECALL) ? check : space;
		room_page_b_data[15].data = xIS_SET(idx->room_flags, ROOM_PRIVATE) ? check : space;
		room_page_b_data[16].data = xIS_SET(idx->room_flags, ROOM_SOLITARY) ? check : space;
		room_page_b_data[17].data = xIS_SET(idx->room_flags, ROOM_NO_ASTRAL) ? check : space;
/*
		room_page_b_data[18].data = xIS_SET(idx->room_flags, ROOM_TUNNEL) ? check : space;
*/
		room_page_b_data[19].data = xIS_SET(idx->room_flags, ROOM_SILENCE) ? check : space;
		room_page_b_data[20].data = xIS_SET(idx->room_flags, ROOM_NO_SUMMON) ? check : space;
		room_page_b_data[21].data = xIS_SET(idx->room_flags, ROOM_CLANSTOREROOM) ? check : space;
		room_page_b_data[22].data = xIS_SET(idx->room_flags, ROOM_LOGSPEECH) ? check : space;
		room_page_b_data[23].data = xIS_SET(idx->room_flags, ROOM_NOFLOOR) ? check : space;
		room_page_b_data[24].data = (idx->sector_type == SECT_INSIDE) ? check : space;
		room_page_b_data[25].data = (idx->sector_type == SECT_FOREST) ? check : space;
		room_page_b_data[26].data = (idx->sector_type == SECT_CITY) ? check : space;
		room_page_b_data[27].data = (idx->sector_type == SECT_HILLS) ? check : space;
		room_page_b_data[28].data = (idx->sector_type == SECT_MOUNTAIN) ? check : space;
		room_page_b_data[29].data = (idx->sector_type == SECT_WATER_SWIM) ? check : space;
		room_page_b_data[30].data = (idx->sector_type == SECT_AIR) ? check : space;
		room_page_b_data[31].data = (idx->sector_type == SECT_DESERT) ? check : space;
		room_page_b_data[32].data = (idx->sector_type == SECT_DUNNO) ? check : space;
		room_page_b_data[33].data = (idx->sector_type == SECT_WATER_NOSWIM) ? check : space;
		room_page_b_data[34].data = (idx->sector_type == SECT_FIELD) ? check : space;
		break;

	default:
	case 'a':
		m_data = room_page_a_data;
		/*
		 * room_page_a_data[0].data = idx->name;
		 * room_page_a_data[1].data = idx->description;
		 */
		format_string(tmp0, idx->name, 72);
		format_string(tmp1, idx->description, 72);
		room_page_a_data[0].data = tmp0;
		room_page_a_data[1].data = tmp1;

	}

	for (i = 0; m_data[i].ptrType != (int)NULL; i++) {
		sprintf(buf, "\x1B[%d;%dH", m_data[i].x, m_data[i].y);
		write_to_buffer(ch->desc, buf, strlen(buf));
		switch(m_data[i].ptrType)
		{
                    case INT:    sprintf(buf, m_data[i].outFormat,*((int *)(m_data[i].data)));
			         break;
                    case SH_INT: sprintf(buf, m_data[i].outFormat,*((sh_int *)(m_data[i].data)));
			         break;
                    default    : sprintf(buf, m_data[i].outFormat,  m_data[i].data);
                }
#ifdef WRONG_WAY
		if (m_data[i].ptrType == INT) {
			sprintf(buf, m_data[i].outFormat, *((int *) (m_data[i].data)));
		}
		if (m_data[i].ptrType == SH_INT) {
			sprintf(buf, m_data[i].outFormat, *((sh_int *) (m_data[i].data)));
		} else {
			sprintf(buf, m_data[i].outFormat, m_data[i].data);
		}
#endif
		write_to_buffer(ch->desc, buf, strlen(buf));
	}

	offset = get_offset(ch);

	/* restrict scrolling area of screen */
	sprintf(buf, "\x1B[%d;%dr", offset, ch->pagelen);
	write_to_buffer(ch->desc, buf, strlen(buf));

	/* goes to pglen,1 */
	sprintf(buf, "\x1B[%d;1H", (ch->pagelen));
	write_to_buffer(ch->desc, buf, strlen(buf));

	send_to_char("\n\r", ch);
}


/*****

Comments & notes:
-----------------
To check a normal apply type:
  total_modstr = get_affect( obj, APPLY_STR );

To check if it affects by flying:
  get_affect( obj, GA_AFFECT | get_bvl(AFF_FLYING) );

To get the RIS for DRAIN:

  get_affect( obj, GA_RIS | get_bvl(RIS_DRAIN) );

******/

void 
fill_in_obj_page(CHAR_DATA * ch, OBJ_INDEX_DATA * idx, char page)
{
	MENU_DATA      *m_data;
	char            buf[MAX_STRING_LENGTH];
	int             i, offset;
	char            tmp0[MAX_STRING_LENGTH];
	char            tmp1[MAX_STRING_LENGTH];
	char            tmp2[MAX_STRING_LENGTH];
	char            tmp3[MAX_STRING_LENGTH];
	sh_int          tmp[27];

	switch (page) {
	case 'h':  return;
	case 'e':
		m_data = obj_page_e_data;
		obj_page_e_data[0].data = IS_OBJ_STAT(idx, ITEM_EVIL) ? check : space;
		obj_page_e_data[1].data = IS_OBJ_STAT(idx, ITEM_ANTI_GOOD) ? check : space;
		obj_page_e_data[2].data = IS_OBJ_STAT(idx, ITEM_NODROP) ? check : space;
		obj_page_e_data[3].data = IS_OBJ_STAT(idx, ITEM_INVIS) ? check : space;
		obj_page_e_data[4].data = IS_OBJ_STAT(idx, ITEM_ANTI_EVIL) ? check : space;
		obj_page_e_data[5].data = IS_OBJ_STAT(idx, ITEM_NOREMOVE) ? check : space;
		obj_page_e_data[6].data = IS_OBJ_STAT(idx, ITEM_HIDDEN) ? check : space;
		obj_page_e_data[7].data = IS_OBJ_STAT(idx, ITEM_ANTI_NEUTRAL) ? check : space;
		obj_page_e_data[8].data = IS_OBJ_STAT(idx, ITEM_INVENTORY) ? check : space;
		obj_page_e_data[9].data = IS_OBJ_STAT(idx, ITEM_GLOW) ? check : space;
		obj_page_e_data[10].data = IS_OBJ_STAT(idx, ITEM_BLESS) ? check : space;
		obj_page_e_data[11].data = IS_OBJ_STAT(idx, ITEM_ANTI_MAGE) ? check : space;
		obj_page_e_data[12].data = IS_OBJ_STAT(idx, ITEM_DONATION) ? check : space;
		obj_page_e_data[13].data = IS_OBJ_STAT(idx, ITEM_HUM) ? check : space;
		obj_page_e_data[14].data = IS_OBJ_STAT(idx, ITEM_METAL) ? check : space;
		obj_page_e_data[15].data = IS_OBJ_STAT(idx, ITEM_ANTI_THIEF) ? check : space;
		obj_page_e_data[16].data = IS_OBJ_STAT(idx, ITEM_CLANOBJECT) ? check : space;
		obj_page_e_data[17].data = IS_OBJ_STAT(idx, ITEM_DARK) ? check : space;
		obj_page_e_data[18].data = IS_OBJ_STAT(idx, ITEM_ORGANIC) ? check : space;
		obj_page_e_data[19].data = IS_OBJ_STAT(idx, ITEM_ANTI_WARRIOR) ? check : space;
		obj_page_e_data[20].data = IS_OBJ_STAT(idx, ITEM_CLANCORPSE) ? check : space;
		obj_page_e_data[21].data = IS_OBJ_STAT(idx, ITEM_MAGIC) ? check : space;
		obj_page_e_data[22].data = IS_OBJ_STAT(idx, ITEM_ANTI_CLERIC) ? check : space;
		obj_page_e_data[23].data = IS_OBJ_STAT(idx, ITEM_PROTOTYPE) ? check : space;
		obj_page_e_data[24].data = IS_OBJ_STAT(idx, ITEM_LOYAL) ? check : space;
		obj_page_e_data[25].data = IS_OBJ_STAT(idx, ITEM_ANTI_VAMPIRE) ? check : space;
		obj_page_e_data[26].data = IS_OBJ_STAT(idx, ITEM_DEATHROT) ? check : space;
		obj_page_e_data[27].data = IS_OBJ_STAT(idx, ITEM_ANTI_DRUID) ? check : space;
		obj_page_e_data[28].data = IS_SET(idx->wear_flags, ITEM_TAKE) ? check : space;
		obj_page_e_data[29].data = IS_SET(idx->wear_flags, ITEM_WEAR_FINGER) ? check : space;
		obj_page_e_data[30].data = IS_SET(idx->wear_flags, ITEM_WEAR_NECK) ? check : space;
		obj_page_e_data[31].data = IS_SET(idx->wear_flags, ITEM_WEAR_BODY) ? check : space;
		obj_page_e_data[32].data = IS_SET(idx->wear_flags, ITEM_WEAR_HEAD) ? check : space;
		obj_page_e_data[33].data = IS_SET(idx->wear_flags, ITEM_WEAR_LEGS) ? check : space;
		obj_page_e_data[34].data = IS_SET(idx->wear_flags, ITEM_WEAR_FEET) ? check : space;
		obj_page_e_data[35].data = IS_SET(idx->wear_flags, ITEM_WEAR_HANDS) ? check : space;
		obj_page_e_data[36].data = IS_SET(idx->wear_flags, ITEM_WEAR_ARMS) ? check : space;
		obj_page_e_data[37].data = IS_SET(idx->wear_flags, ITEM_WEAR_SHIELD) ? check : space;
		obj_page_e_data[38].data = IS_SET(idx->wear_flags, ITEM_WEAR_ABOUT) ? check : space;
		obj_page_e_data[39].data = IS_SET(idx->wear_flags, ITEM_WEAR_WAIST) ? check : space;
		obj_page_e_data[40].data = IS_SET(idx->wear_flags, ITEM_WEAR_WRIST) ? check : space;
		obj_page_e_data[41].data = IS_SET(idx->wear_flags, ITEM_WIELD) ? check : space;
		obj_page_e_data[42].data = IS_SET(idx->wear_flags, ITEM_HOLD) ? check : space;
		obj_page_e_data[43].data = IS_SET(idx->wear_flags, ITEM_DUAL_WIELD) ? check : space;
		obj_page_e_data[44].data = IS_SET(idx->wear_flags, ITEM_WEAR_EARS) ? check : space;
		obj_page_e_data[45].data = IS_SET(idx->wear_flags, ITEM_WEAR_EYES) ? check : space;
		break;
	case 'd':
		m_data = obj_page_d_data;
		obj_page_d_data[0].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_BLIND)) ? check : space;
		obj_page_d_data[1].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_DETECT_INVIS)) ? check : space;
		obj_page_d_data[2].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_DETECT_EVIL)) ? check : space;
		obj_page_d_data[3].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_DETECT_MAGIC)) ? check : space;
		obj_page_d_data[4].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_DETECT_HIDDEN)) ? check : space;
		obj_page_d_data[5].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_HOLD)) ? check : space;
		obj_page_d_data[6].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_SANCTUARY)) ? check : space;
		obj_page_d_data[7].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_FAERIE_FIRE)) ? check : space;
		obj_page_d_data[8].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_INFRARED)) ? check : space;
		obj_page_d_data[9].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_CURSE)) ? check : space;
		obj_page_d_data[10].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_FLAMING)) ? check : space;
		obj_page_d_data[11].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_POISON)) ? check : space;
		obj_page_d_data[12].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_PROTECT)) ? check : space;
		obj_page_d_data[13].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_PARALYSIS)) ? check : space;
		obj_page_d_data[14].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_SLEEP)) ? check : space;
		obj_page_d_data[15].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_HIDE)) ? check : space;
		obj_page_d_data[16].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_SNEAK)) ? check : space;
		obj_page_d_data[17].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_CHARM)) ? check : space;
		obj_page_d_data[18].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_FLYING)) ? check : space;
		obj_page_d_data[19].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_PASS_DOOR)) ? check : space;
		obj_page_d_data[20].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_TRUESIGHT)) ? check : space;
		obj_page_d_data[21].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_DETECTTRAPS)) ? check : space;
		obj_page_d_data[22].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_SCRYING)) ? check : space;
		obj_page_d_data[23].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_FIRESHIELD)) ? check : space;
		obj_page_d_data[24].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_SHOCKSHIELD)) ? check : space;
		obj_page_d_data[25].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_FLOATING)) ? check : space;
		obj_page_d_data[26].data = get_affect(idx, GA_AFFECTED | get_bvl(AFF_INVISIBLE)) ? check : space;
		break;
	case 'c':
		m_data = obj_page_c_data;
		obj_page_c_data[0].data = "\\";
		tmp[1] = get_affect(idx, APPLY_SAVING_POISON);
		tmp[2] = get_affect(idx, APPLY_AGE);
		tmp[3] = get_affect(idx, APPLY_STR);
		tmp[4] = get_affect(idx, APPLY_SAVING_ROD);
		tmp[5] = get_affect(idx, APPLY_CLASS);
		tmp[6] = get_affect(idx, APPLY_INT);
		tmp[7] = get_affect(idx, APPLY_SAVING_PARA);
		tmp[8] = get_affect(idx, APPLY_LEVEL);
		tmp[9] = get_affect(idx, APPLY_WIS);
		tmp[10] = get_affect(idx, APPLY_SAVING_BREATH);
		tmp[11] = get_affect(idx, APPLY_SEX);
		tmp[12] = get_affect(idx, APPLY_DEX);
		tmp[13] = get_affect(idx, APPLY_SAVING_SPELL);
		tmp[14] = get_affect(idx, APPLY_HEIGHT);
		tmp[15] = get_affect(idx, APPLY_CON);
		tmp[16] = get_affect(idx, APPLY_WEIGHT);
		tmp[17] = get_affect(idx, APPLY_CHA);
		tmp[18] = get_affect(idx, APPLY_MANA);
		tmp[19] = get_affect(idx, APPLY_AC);
		tmp[20] = get_affect(idx, APPLY_HIT);
		tmp[21] = get_affect(idx, APPLY_HITROLL);
		tmp[22] = get_affect(idx, APPLY_GOLD);
		tmp[23] = get_affect(idx, APPLY_MOVE);
		tmp[24] = get_affect(idx, APPLY_DAMROLL);
		tmp[25] = get_affect(idx, APPLY_EXP);

		obj_page_c_data[1].data = &(tmp[1]);
		obj_page_c_data[2].data = &(tmp[2]);
		obj_page_c_data[3].data = &(tmp[3]);
		obj_page_c_data[4].data = &(tmp[4]);
		obj_page_c_data[5].data = &(tmp[5]);
		obj_page_c_data[6].data = &(tmp[6]);
		obj_page_c_data[7].data = &(tmp[7]);
		obj_page_c_data[8].data = &(tmp[8]);
		obj_page_c_data[9].data = &(tmp[9]);
		obj_page_c_data[10].data = &(tmp[10]);
		obj_page_c_data[11].data = &(tmp[11]);
		obj_page_c_data[12].data = &(tmp[12]);
		obj_page_c_data[13].data = &(tmp[13]);
		obj_page_c_data[14].data = &(tmp[14]);
		obj_page_c_data[15].data = &(tmp[15]);
		obj_page_c_data[16].data = &(tmp[16]);
		obj_page_c_data[17].data = &(tmp[17]);
		obj_page_c_data[18].data = &(tmp[18]);
		obj_page_c_data[19].data = &(tmp[19]);
		obj_page_c_data[20].data = &(tmp[20]);
		obj_page_c_data[21].data = &(tmp[21]);
		obj_page_c_data[22].data = &(tmp[22]);
		obj_page_c_data[23].data = &(tmp[23]);
		obj_page_c_data[24].data = &(tmp[24]);
		obj_page_c_data[25].data = &(tmp[25]);

		obj_page_c_data[26].data = (char *) ris_strings[get_affect(idx, GA_RIS | get_bvl(RIS_FIRE))];
		obj_page_c_data[27].data = (char *) ris_strings[get_affect(idx, GA_RIS | get_bvl(RIS_COLD))];
		obj_page_c_data[28].data = (char *) ris_strings[get_affect(idx, GA_RIS | get_bvl(RIS_ELECTRICITY))];
		obj_page_c_data[29].data = (char *) ris_strings[get_affect(idx, GA_RIS | get_bvl(RIS_ENERGY))];
		obj_page_c_data[30].data = (char *) ris_strings[get_affect(idx, GA_RIS | get_bvl(RIS_POISON))];

		obj_page_c_data[31].data = (char *) ris_strings[get_affect(idx, GA_RIS | get_bvl(RIS_BLUNT))];
		obj_page_c_data[32].data = (char *) ris_strings[get_affect(idx, GA_RIS | get_bvl(RIS_PIERCE))];
		obj_page_c_data[33].data = (char *) ris_strings[get_affect(idx, GA_RIS | get_bvl(RIS_SLASH))];
		obj_page_c_data[34].data = (char *) ris_strings[get_affect(idx, GA_RIS | get_bvl(RIS_ACID))];
		obj_page_c_data[35].data = (char *) ris_strings[get_affect(idx, GA_RIS | get_bvl(RIS_NONMAGIC))];

		obj_page_c_data[36].data = (char *) ris_strings[get_affect(idx, GA_RIS | get_bvl(RIS_DRAIN))];
		obj_page_c_data[37].data = (char *) ris_strings[get_affect(idx, GA_RIS | get_bvl(RIS_SLEEP))];
		obj_page_c_data[38].data = (char *) ris_strings[get_affect(idx, GA_RIS | get_bvl(RIS_CHARM))];
		obj_page_c_data[39].data = (char *) ris_strings[get_affect(idx, GA_RIS | get_bvl(RIS_HOLD))];
		obj_page_c_data[40].data = (char *) ris_strings[get_affect(idx, GA_RIS | get_bvl(RIS_MAGIC))];

		obj_page_c_data[41].data = (char *) ris_strings[get_affect(idx, GA_RIS | get_bvl(RIS_PLUS1))];
		obj_page_c_data[42].data = (char *) ris_strings[get_affect(idx, GA_RIS | get_bvl(RIS_PLUS2))];
		obj_page_c_data[43].data = (char *) ris_strings[get_affect(idx, GA_RIS | get_bvl(RIS_PLUS3))];
		obj_page_c_data[44].data = (char *) ris_strings[get_affect(idx, GA_RIS | get_bvl(RIS_PLUS4))];
		obj_page_c_data[45].data = (char *) ris_strings[get_affect(idx, GA_RIS | get_bvl(RIS_PLUS5))];
		obj_page_c_data[46].data = (char *) ris_strings[get_affect(idx, GA_RIS | get_bvl(RIS_PLUS6))];

		break;
	case 'b':
		m_data = obj_page_b_data;
		obj_page_b_data[0].data = ((idx->item_type == 0) ? check : space);
		obj_page_b_data[1].data = ((idx->item_type == ITEM_FOUNTAIN) ? check : space);
		obj_page_b_data[2].data = ((idx->item_type == ITEM_FURNITURE) ? check : space);
		obj_page_b_data[3].data = ((idx->item_type == ITEM_ARMOR) ? check : space);
		obj_page_b_data[4].data = ((idx->item_type == ITEM_FOOD) ? check : space);
		obj_page_b_data[5].data = ((idx->item_type == ITEM_MONEY) ? check : space);
		obj_page_b_data[6].data = ((idx->item_type == ITEM_POTION) ? check : space);
		obj_page_b_data[7].data = ((idx->item_type == ITEM_PULLCHAIN) ? check : space);
		obj_page_b_data[8].data = ((idx->item_type == ITEM_WEAPON) ? check : space);
		obj_page_b_data[9].data = ((idx->item_type == ITEM_MAP) ? check : space);
		obj_page_b_data[10].data = ((idx->item_type == ITEM_LIGHT) ? check : space);
		obj_page_b_data[11].data = ((idx->item_type == ITEM_BUTTON) ? check : space);
		obj_page_b_data[12].data = ((idx->item_type == ITEM_TREASURE) ? check : space);
		obj_page_b_data[13].data = ((idx->item_type == ITEM_HERB) ? check : space);
		obj_page_b_data[14].data = ((idx->item_type == ITEM_TRASH) ? check : space);
		obj_page_b_data[15].data = ((idx->item_type == ITEM_SWITCH) ? check : space);
		obj_page_b_data[16].data = ((idx->item_type == ITEM_CONTAINER) ? check : space);
		obj_page_b_data[17].data = ((idx->item_type == ITEM_WAND) ? check : space);
		obj_page_b_data[18].data = ((idx->item_type == ITEM_SCROLL) ? check : space);
		obj_page_b_data[19].data = ((idx->item_type == ITEM_STAFF) ? check : space);
		obj_page_b_data[20].data = ((idx->item_type == ITEM_BLOODSTAIN) ? check : space);
		obj_page_b_data[21].data = ((idx->item_type == ITEM_PILL) ? check : space);
		obj_page_b_data[22].data = ((idx->item_type == ITEM_BLOOD) ? check : space);
		obj_page_b_data[23].data = ((idx->item_type == ITEM_DIAL) ? check : space);
		obj_page_b_data[24].data = ((idx->item_type == ITEM_TRAP) ? check : space);
		obj_page_b_data[25].data = ((idx->item_type == ITEM_KEY) ? check : space);
		obj_page_b_data[26].data = ((idx->item_type == ITEM_INCENSE) ? check : space);
		obj_page_b_data[27].data = ((idx->item_type == ITEM_LEVER) ? check : space);
		obj_page_b_data[28].data = ((idx->item_type == ITEM_CORPSE_PC) ? check : space);
		obj_page_b_data[29].data = ((idx->item_type == ITEM_PIPE) ? check : space);
		obj_page_b_data[30].data = ((idx->item_type == ITEM_SCRAPS) ? check : space);
		obj_page_b_data[31].data = ((idx->item_type == ITEM_TRAP) ? check : space);
		obj_page_b_data[32].data = ((idx->item_type == ITEM_CORPSE_NPC) ? check : space);
		obj_page_b_data[33].data = ((idx->item_type == ITEM_BOOK) ? check : space);
		obj_page_b_data[34].data = ((idx->item_type == ITEM_MATCH) ? check : space);
		obj_page_b_data[35].data = ((idx->item_type == ITEM_DRINK_CON) ? check : space);
		obj_page_b_data[36].data = (char *) "\\";
		obj_page_b_data[37].data = (char *) "\\";
		obj_page_b_data[38].data = ((idx->item_type == ITEM_FIRE) ? check : space);
		obj_page_b_data[39].data = ((idx->item_type == ITEM_HERB_CON) ? check : space);
		obj_page_b_data[40].data = (char *) "\\";
		obj_page_b_data[41].data = ((idx->item_type == ITEM_BOAT) ? check : space);
		obj_page_b_data[42].data = ((idx->item_type == ITEM_RUNE) ? check : space);
		obj_page_b_data[43].data = ((idx->item_type == ITEM_RUNEPOUCH) ? check : space);
		obj_page_b_data[44].data = (char *) "\\";

		break;

	default:
	case 'a':
		m_data = obj_page_a_data;
		format_string(tmp0, idx->name, 53);
		format_string(tmp1, idx->short_descr, 53);
		format_string(tmp2, idx->description, 63);
		format_string(tmp3, idx->action_desc, 51);
		obj_page_a_data[0].data = tmp0;
		obj_page_a_data[1].data = tmp1;
		obj_page_a_data[2].data = tmp2;
		obj_page_a_data[3].data = tmp3;
		obj_page_a_data[4].data = &(idx->value[0]);
		obj_page_a_data[5].data = get_item_strings(idx->item_type, 1);

		obj_page_a_data[6].data = &(idx->value[1]);
		obj_page_a_data[7].data = get_item_strings(idx->item_type, 2);

		obj_page_a_data[8].data = &(idx->value[2]);
		obj_page_a_data[9].data = get_item_strings(idx->item_type, 3);

		obj_page_a_data[10].data = &(idx->value[3]);
		obj_page_a_data[11].data = get_item_strings(idx->item_type,4);

		obj_page_a_data[12].data = &(idx->weight);
		obj_page_a_data[13].data = &(idx->cost);


	}

	for (i = 0; m_data[i].ptrType != (int) NULL; i++) {
		sprintf(buf, "\x1B[%d;%dH", m_data[i].x, m_data[i].y);
		write_to_buffer(ch->desc, buf, strlen(buf));
		switch(m_data[i].ptrType)
		{
                    case INT:    sprintf(buf, m_data[i].outFormat,*((int *)(m_data[i].data)));
			         break;
                    case SH_INT: sprintf(buf, m_data[i].outFormat,*((sh_int *)(m_data[i].data)));
			         break;
                    default    : sprintf(buf, m_data[i].outFormat,  m_data[i].data);
		}
		write_to_buffer(ch->desc, buf, strlen(buf));
  	}

	offset = get_offset(ch);

	/* restrict scrolling area of screen */
	sprintf(buf, "\x1B[%d;%dr", offset, ch->pagelen);
	write_to_buffer(ch->desc, buf, strlen(buf));

	/* goes to pglen,1 */
	sprintf(buf, "\x1B[%d;1H", (ch->pagelen));
	write_to_buffer(ch->desc, buf, strlen(buf));

	send_to_char("\n\r", ch);
}






void 
fill_in_mob_page(CHAR_DATA * ch, MOB_INDEX_DATA * idx, char page)
{
	MENU_DATA      *m_data;
	char            buf[MAX_STRING_LENGTH];
	char            buf1[MAX_STRING_LENGTH];
	char            buf2[MAX_STRING_LENGTH];
	char            tmp0[MAX_STRING_LENGTH];
	char            tmp1[MAX_STRING_LENGTH];
	char            tmp2[MAX_STRING_LENGTH];
	char            tmp3[MAX_STRING_LENGTH];
	char           *spec;
	int             i, offset;

	switch (page) {
	case 'h': return;
	case 'e':
		m_data = mob_page_e_data;
		spec = lookup_spec(idx->spec_fun);
		/*
		 * getting a boolean for a special is a royal pain in the
		 * butt.... would be easier if specs were global, then could
		 * cmp by addr instead of string
		 */
		mob_page_e_data[0].data = (!str_cmp(spec, "spec_fido")) ? check : space;
		mob_page_e_data[1].data = (!str_cmp(spec, "spec_cast_adept")) ? check : space;
		mob_page_e_data[2].data = (!str_cmp(spec, "spec_breath_fire")) ? check : space;
		mob_page_e_data[3].data = (!str_cmp(spec, "spec_guard")) ? check : space;
		mob_page_e_data[4].data = (!str_cmp(spec, "spec_cast_cleric")) ? check : space;
		mob_page_e_data[5].data = (!str_cmp(spec, "spec_breath_frost")) ? check : space;
		mob_page_e_data[6].data = (!str_cmp(spec, "spec_janitor")) ? check : space;
		mob_page_e_data[7].data = (!str_cmp(spec, "spec_cast_mage")) ? check : space;
		mob_page_e_data[8].data = (!str_cmp(spec, "spec_breath_acid")) ? check : space;
		mob_page_e_data[9].data = (!str_cmp(spec, "spec_thief")) ? check : space;
		mob_page_e_data[10].data = (!str_cmp(spec, "spec_cast_undead")) ? check : space;
		mob_page_e_data[11].data = (!str_cmp(spec, "spec_breath_gas")) ? check : space;
		mob_page_e_data[12].data = (!str_cmp(spec, "spec_mayor")) ? check : space;
		mob_page_e_data[13].data = (!str_cmp(spec, "spec_breath_lightning")) ? check : space;
		mob_page_e_data[14].data = (!str_cmp(spec, "spec_poison")) ? check : space;
		mob_page_e_data[15].data = (!str_cmp(spec, "spec_executioner")) ? check : space;
		mob_page_e_data[16].data = (!str_cmp(spec, "spec_breath_any")) ? check : space;


		mob_page_e_data[17].data = (idx->defposition == POS_DEAD) ? check : space;
		mob_page_e_data[18].data = (idx->defposition == POS_MORTAL) ? check : space;
		mob_page_e_data[19].data = (idx->defposition == POS_INCAP) ? check : space;
		mob_page_e_data[20].data = (idx->defposition == POS_STUNNED) ? check : space;
		mob_page_e_data[21].data = (idx->defposition == POS_SLEEPING) ? check : space;
		mob_page_e_data[22].data = (idx->defposition == POS_RESTING) ? check : space;
		mob_page_e_data[23].data = (idx->defposition == POS_SITTING) ? check : space;
		mob_page_e_data[24].data = (idx->defposition == POS_FIGHTING) ? check : space;
		mob_page_e_data[25].data = (idx->defposition == POS_STANDING) ? check : space;
		mob_page_e_data[26].data = (idx->defposition == POS_MOUNTED) ? check : space;

		mob_page_e_data[27].data = (char *) get_ris(idx, RIS_FIRE);
		mob_page_e_data[28].data = (char *) get_ris(idx, RIS_COLD);
		mob_page_e_data[29].data = (char *) get_ris(idx, RIS_ELECTRICITY);
		mob_page_e_data[30].data = (char *) get_ris(idx, RIS_ENERGY);
		mob_page_e_data[31].data = (char *) get_ris(idx, RIS_POISON);
		mob_page_e_data[32].data = (char *) get_ris(idx, RIS_BLUNT);
		mob_page_e_data[33].data = (char *) get_ris(idx, RIS_PIERCE);
		mob_page_e_data[34].data = (char *) get_ris(idx, RIS_SLASH);
		mob_page_e_data[35].data = (char *) get_ris(idx, RIS_ACID);
		mob_page_e_data[36].data = (char *) get_ris(idx, RIS_NONMAGIC);
		mob_page_e_data[37].data = (char *) get_ris(idx, RIS_DRAIN);
		mob_page_e_data[38].data = (char *) get_ris(idx, RIS_SLEEP);
		mob_page_e_data[39].data = (char *) get_ris(idx, RIS_CHARM);
		mob_page_e_data[40].data = (char *) get_ris(idx, RIS_HOLD);
		mob_page_e_data[41].data = (char *) get_ris(idx, RIS_MAGIC);
		mob_page_e_data[42].data = (char *) get_ris(idx, RIS_PLUS1);
		mob_page_e_data[43].data = (char *) get_ris(idx, RIS_PLUS2);
		mob_page_e_data[44].data = (char *) get_ris(idx, RIS_PLUS3);
		mob_page_e_data[45].data = (char *) get_ris(idx, RIS_PLUS4);
		mob_page_e_data[46].data = (char *) get_ris(idx, RIS_PLUS5);
		mob_page_e_data[47].data = (char *) get_ris(idx, RIS_PLUS6);
		break;

	case 'd':
		m_data = mob_page_d_data;
		mob_page_d_data[0].data = &(idx->saving_poison_death);
		mob_page_d_data[1].data = &(idx->saving_wand);
		mob_page_d_data[2].data = &(idx->saving_para_petri);
		mob_page_d_data[3].data = &(idx->saving_breath);
		mob_page_d_data[4].data = &(idx->saving_spell_staff);
		mob_page_d_data[5].data = IS_SET(idx->xflags, PART_FINS) ? check : space;
		mob_page_d_data[6].data = IS_SET(idx->xflags, PART_EAR) ? check : space;
		mob_page_d_data[7].data = IS_SET(idx->xflags, PART_EYE) ? check : space;
		mob_page_d_data[8].data = IS_SET(idx->xflags, PART_LONG_TONGUE) ? check : space;
		mob_page_d_data[9].data = IS_SET(idx->xflags, PART_SCALES) ? check : space;
		mob_page_d_data[10].data = IS_SET(idx->xflags, PART_TAIL) ? check : space;
		mob_page_d_data[11].data = IS_SET(idx->xflags, PART_ARMS) ? check : space;
		mob_page_d_data[12].data = IS_SET(idx->xflags, PART_LEGS) ? check : space;
		mob_page_d_data[13].data = IS_SET(idx->xflags, PART_TENTACLES) ? check : space;
		mob_page_d_data[14].data = IS_SET(idx->xflags, PART_BRAINS) ? check : space;
		mob_page_d_data[15].data = IS_SET(idx->xflags, PART_GUTS) ? check : space;
		mob_page_d_data[16].data = IS_SET(idx->xflags, PART_HEAD) ? check : space;
		mob_page_d_data[17].data = IS_SET(idx->xflags, PART_FEET) ? check : space;
		mob_page_d_data[18].data = IS_SET(idx->xflags, PART_EYESTALKS) ? check : space;
		mob_page_d_data[19].data = IS_SET(idx->xflags, PART_FINGERS) ? check : space;
		mob_page_d_data[20].data = IS_SET(idx->xflags, PART_WINGS) ? check : space;
		mob_page_d_data[21].data = IS_SET(idx->xflags, PART_HANDS) ? check : space;
		mob_page_d_data[22].data = IS_SET(idx->xflags, PART_HEART) ? check : space;
		mob_page_d_data[23].data = IS_SET(idx->xflags, PART_CLAWS) ? check : space;
		mob_page_d_data[24].data = IS_SET(idx->xflags, PART_HORNS) ? check : space;
		mob_page_d_data[25].data = IS_SET(idx->xflags, PART_TUSKS) ? check : space;
		mob_page_d_data[26].data = IS_SET(idx->xflags, PART_TAILATTACK) ? check : space;
		mob_page_d_data[27].data = IS_SET(idx->xflags, PART_SHARPSCALES) ? check : space;
		mob_page_d_data[28].data = &(idx->numattacks);
		sprintf(buf1, "[%5.5hd]D[%5.5hd]+[%5.5hd]", idx->hitnodice,
			idx->hitsizedice,
			idx->hitplus);
		if (mob_page_d_data[29].data != NULL)
			DISPOSE(mob_page_d_data[29].data);
		mob_page_d_data[29].data = str_dup(buf1);

		mob_page_d_data[30].data = &(idx->hitroll);
		mob_page_d_data[31].data = &(idx->damroll);
		sprintf(buf2, "[%5.5hd]D[%5.5hd]+[%5.5hd]", idx->damnodice,
			idx->damsizedice,
			idx->damplus);
		if (mob_page_d_data[32].data != NULL)
			DISPOSE(mob_page_d_data[32].data);
		mob_page_d_data[32].data = str_dup(buf2);
		break;
	case 'c':
		m_data = mob_page_c_data;
		mob_page_c_data[0].data = xIS_SET(idx->attacks, ATCK_BITE) ? check : space;
		mob_page_c_data[1].data = xIS_SET(idx->attacks, ATCK_CLAWS) ? check : space;
		mob_page_c_data[2].data = xIS_SET(idx->attacks, ATCK_FIREBREATH) ? check : space;
		mob_page_c_data[3].data = xIS_SET(idx->attacks, ATCK_BASH) ? check : space;
		mob_page_c_data[4].data = xIS_SET(idx->attacks, ATCK_STUN) ? check : space;
		mob_page_c_data[5].data = xIS_SET(idx->attacks, ATCK_ACIDBREATH) ? check : space;
		mob_page_c_data[6].data = xIS_SET(idx->attacks, ATCK_DRAIN) ? check : space;
		mob_page_c_data[7].data = xIS_SET(idx->attacks, ATCK_KICK) ? check : space;
		mob_page_c_data[8].data = xIS_SET(idx->attacks, ATCK_FEED) ? check : space;
		mob_page_c_data[9].data = xIS_SET(idx->attacks, ATCK_GASBREATH) ? check : space;
		mob_page_c_data[10].data = xIS_SET(idx->attacks, ATCK_POISON) ? check : space;
		mob_page_c_data[11].data = xIS_SET(idx->attacks, ATCK_GAZE) ? check : space;
		mob_page_c_data[12].data = xIS_SET(idx->attacks, ATCK_HARM) ? check : space;
		mob_page_c_data[13].data = xIS_SET(idx->attacks, ATCK_FROSTBREATH) ? check : space;
		mob_page_c_data[14].data = xIS_SET(idx->attacks, ATCK_CURSE) ? check : space;
		mob_page_c_data[15].data = xIS_SET(idx->attacks, ATCK_BACKSTAB) ? check : space;
		mob_page_c_data[16].data = xIS_SET(idx->attacks, ATCK_PUNCH) ? check : space;
		mob_page_c_data[17].data = xIS_SET(idx->attacks, ATCK_LIGHTNBREATH) ? check : space;
		mob_page_c_data[18].data = xIS_SET(idx->attacks, ATCK_WEAKEN) ? check : space;
		mob_page_c_data[19].data = xIS_SET(idx->attacks, ATCK_TAIL) ? check : space;
		mob_page_c_data[20].data = xIS_SET(idx->attacks, ATCK_STING) ? check : space;
		mob_page_c_data[21].data = xIS_SET(idx->attacks, ATCK_CAUSESERIOUS) ? check : space;
		mob_page_c_data[22].data = xIS_SET(idx->attacks, ATCK_TRIP) ? check : space;
		mob_page_c_data[23].data = xIS_SET(idx->attacks, ATCK_FIREBALL) ? check : space;
		mob_page_c_data[24].data = xIS_SET(idx->attacks, ATCK_GOUGE) ? check : space;
		mob_page_c_data[25].data = xIS_SET(idx->attacks, ATCK_CAUSECRITICAL) ? check : space;
		mob_page_c_data[26].data = xIS_SET(idx->attacks, ATCK_FLAMESTRIKE) ? check : space;
		mob_page_c_data[27].data = xIS_SET(idx->attacks, ATCK_EARTHQUAKE) ? check : space;
		mob_page_c_data[28].data = xIS_SET(idx->attacks, ATCK_COLORSPRAY) ? check : space;
		mob_page_c_data[29].data = xIS_SET(idx->attacks, ATCK_NASTYPOISON) ? check : space;
		mob_page_c_data[30].data = xIS_SET(idx->attacks, ATCK_BLINDNESS) ? check : space;
		mob_page_c_data[31].data = xIS_SET(idx->defenses, DFND_SANCTUARY) ? check : space;
		mob_page_c_data[32].data = xIS_SET(idx->defenses, DFND_CURELIGHT) ? check : space;
		mob_page_c_data[33].data = xIS_SET(idx->defenses, DFND_DISPELEVIL) ? check : space;
		mob_page_c_data[34].data = xIS_SET(idx->defenses, DFND_SHIELD) ? check : space;
		mob_page_c_data[35].data = xIS_SET(idx->defenses, DFND_CURESERIOUS) ? check : space;
		mob_page_c_data[36].data = xIS_SET(idx->defenses, DFND_DISPELMAGIC) ? check : space;
		mob_page_c_data[37].data = xIS_SET(idx->defenses, DFND_DODGE) ? check : space;
		mob_page_c_data[38].data = xIS_SET(idx->defenses, DFND_FIRESHIELD) ? check : space;
		mob_page_c_data[39].data = xIS_SET(idx->defenses, DFND_CURECRITICAL) ? check : space;
		mob_page_c_data[40].data = xIS_SET(idx->defenses, DFND_TELEPORT) ? check : space;
		mob_page_c_data[41].data = xIS_SET(idx->defenses, DFND_PARRY) ? check : space;
		mob_page_c_data[42].data = xIS_SET(idx->defenses, DFND_SHOCKSHIELD) ? check : space;
		mob_page_c_data[43].data = xIS_SET(idx->defenses, DFND_HEAL) ? check : space;
		mob_page_c_data[44].data = xIS_SET(idx->defenses, DFND_BLESS) ? check : space;
		mob_page_c_data[45].data = xIS_SET(idx->defenses, DFND_DISARM) ? check : space;
		mob_page_c_data[46].data = xIS_SET(idx->defenses, DFND_STONESKIN) ? check : space;
		mob_page_c_data[47].data = xIS_SET(idx->defenses, DFND_MONSUM1) ? check : space;
		mob_page_c_data[48].data = xIS_SET(idx->defenses, DFND_MONSUM2) ? check : space;
		mob_page_c_data[49].data = xIS_SET(idx->defenses, DFND_MONSUM3) ? check : space;
		mob_page_c_data[50].data = xIS_SET(idx->defenses, DFND_MONSUM4) ? check : space;
		break;
	case 'b':
		m_data = mob_page_b_data;
		mob_page_b_data[0].data = xIS_SET(idx->act, ACT_SENTINEL) ? check : space;
		mob_page_b_data[1].data = xIS_SET(idx->act, ACT_SCAVENGER) ? check : space;
		mob_page_b_data[2].data = xIS_SET(idx->act, ACT_AGGRESSIVE) ? check : space;
		mob_page_b_data[3].data = xIS_SET(idx->act, ACT_STAY_AREA) ? check : space;
		mob_page_b_data[4].data = xIS_SET(idx->act, ACT_WIMPY) ? check : space;
		mob_page_b_data[5].data = xIS_SET(idx->act, ACT_PET) ? check : space;
		mob_page_b_data[6].data = xIS_SET(idx->act, ACT_TRAIN) ? check : space;
		mob_page_b_data[7].data = xIS_SET(idx->act, ACT_PRACTICE) ? check : space;
		mob_page_b_data[8].data = xIS_SET(idx->act, ACT_IMMORTAL) ? check : space;
		mob_page_b_data[9].data = xIS_SET(idx->act, ACT_DEADLY) ? check : space;
		mob_page_b_data[10].data = xIS_SET(idx->act, ACT_POLYSELF) ? check : space;
		mob_page_b_data[11].data = xIS_SET(idx->act, ACT_META_AGGR) ? check : space;
		mob_page_b_data[12].data = xIS_SET(idx->act, ACT_GUARDIAN) ? check : space;
		mob_page_b_data[13].data = xIS_SET(idx->act, ACT_RUNNING) ? check : space;
		mob_page_b_data[14].data = xIS_SET(idx->act, ACT_NOWANDER) ? check : space;
		mob_page_b_data[15].data = xIS_SET(idx->act, ACT_MOUNTABLE) ? check : space;
		mob_page_b_data[16].data = xIS_SET(idx->act, ACT_MOUNTED) ? check : space;
		mob_page_b_data[17].data = IS_AFFECTED(idx, AFF_BLIND) ? check : space;
		mob_page_b_data[18].data = IS_AFFECTED(idx, AFF_DETECT_INVIS) ? check : space;
		mob_page_b_data[19].data = IS_AFFECTED(idx, AFF_DETECT_EVIL) ? check : space;
		mob_page_b_data[20].data = IS_AFFECTED(idx, AFF_DETECT_MAGIC) ? check : space;
		mob_page_b_data[21].data = IS_AFFECTED(idx, AFF_DETECT_HIDDEN) ? check : space;
		mob_page_b_data[22].data = IS_AFFECTED(idx, AFF_HOLD) ? check : space;
		mob_page_b_data[23].data = IS_AFFECTED(idx, AFF_SANCTUARY) ? check : space;
		mob_page_b_data[24].data = IS_AFFECTED(idx, AFF_FAERIE_FIRE) ? check : space;
		mob_page_b_data[25].data = IS_AFFECTED(idx, AFF_INFRARED) ? check : space;
		mob_page_b_data[26].data = IS_AFFECTED(idx, AFF_CURSE) ? check : space;
		mob_page_b_data[27].data = IS_AFFECTED(idx, AFF_FLAMING) ? check : space;
		mob_page_b_data[28].data = IS_AFFECTED(idx, AFF_POISON) ? check : space;
		mob_page_b_data[29].data = IS_AFFECTED(idx, AFF_PROTECT) ? check : space;
		mob_page_b_data[30].data = IS_AFFECTED(idx, AFF_PARALYSIS) ? check : space;
		mob_page_b_data[31].data = IS_AFFECTED(idx, AFF_SLEEP) ? check : space;
		mob_page_b_data[32].data = IS_AFFECTED(idx, AFF_HIDE) ? check : space;
		mob_page_b_data[33].data = IS_AFFECTED(idx, AFF_SNEAK) ? check : space;
		mob_page_b_data[34].data = IS_AFFECTED(idx, AFF_CHARM) ? check : space;
		mob_page_b_data[35].data = IS_AFFECTED(idx, AFF_PARALYSIS) ? check : space;
		mob_page_b_data[36].data = IS_AFFECTED(idx, AFF_FLYING) ? check : space;
		mob_page_b_data[37].data = IS_AFFECTED(idx, AFF_PASS_DOOR) ? check : space;
		mob_page_b_data[38].data = IS_AFFECTED(idx, AFF_TRUESIGHT) ? check : space;
		mob_page_b_data[39].data = IS_AFFECTED(idx, AFF_DETECTTRAPS) ? check : space;
		mob_page_b_data[40].data = IS_AFFECTED(idx, AFF_SCRYING) ? check : space;
		mob_page_b_data[41].data = IS_AFFECTED(idx, AFF_FIRESHIELD) ? check : space;
		mob_page_b_data[42].data = IS_AFFECTED(idx, AFF_SHOCKSHIELD) ? check : space;
		mob_page_b_data[43].data = IS_AFFECTED(idx, AFF_FLOATING) ? check : space;
		mob_page_b_data[44].data = IS_AFFECTED(idx, AFF_INVISIBLE) ? check : space;
		break;
	case 'f':
		m_data = mob_page_f_data;
		mob_page_f_data[0].data = (idx->race == get_npc_race("human")) ? check : space;
		mob_page_f_data[1].data = (idx->race ==get_npc_race("neanderthal")) ? check : space;
		mob_page_f_data[2].data = (idx->race ==get_npc_race("avis")) ? check : space;
		mob_page_f_data[3].data = (idx->race ==get_npc_race("amphibian")) ? check : space;
		mob_page_f_data[4].data = (idx->race ==get_npc_race("elf")) ? check : space;
		mob_page_f_data[5].data = (idx->race ==get_npc_race("orc")) ? check : space;
		mob_page_f_data[6].data = (idx->race ==get_npc_race("bugbear")) ? check : space;
		mob_page_f_data[7].data = (idx->race ==get_npc_race("bovine")) ? check : space;

		mob_page_f_data[8].data = (idx->race ==get_npc_race("dwarf")) ? check : space;
		mob_page_f_data[9].data = (idx->race ==get_npc_race("skeleton")) ? check : space;
		mob_page_f_data[10].data = (idx->race ==get_npc_race("dragon")) ? check : space;
		mob_page_f_data[11].data = (idx->race ==get_npc_race("canine")) ? check : space;

		mob_page_f_data[12].data = (idx->race ==get_npc_race("ant")) ? check : space;
		mob_page_f_data[13].data = (idx->race ==get_npc_race("halfling")) ? check : space;
		mob_page_f_data[14].data = (idx->race ==get_npc_race("thoul")) ? check : space;
		mob_page_f_data[15].data = (idx->race ==get_npc_race("rustmonster")) ? check : space;
		mob_page_f_data[16].data = (idx->race ==get_npc_race("crustacean")) ? check : space;
		mob_page_f_data[17].data = (idx->race ==get_npc_race("bee")) ? check : space;
		mob_page_f_data[18].data = (idx->race ==get_npc_race("pixie")) ? check : space;
		mob_page_f_data[19].data = (idx->race ==get_npc_race("undead")) ? check : space;
		mob_page_f_data[20].data = (idx->race ==get_npc_race("shadow")) ? check : space;
		mob_page_f_data[21].data = (idx->race ==get_npc_race("feline")) ? check : space;
		mob_page_f_data[22].data = (idx->race ==get_npc_race("beetle")) ? check : space;
		mob_page_f_data[23].data = (idx->race ==get_npc_race("vampire")) ? check : space;
		mob_page_f_data[24].data = (idx->race ==get_npc_race("zombie")) ? check : space;
		mob_page_f_data[25].data = (idx->race == get_npc_race("shapeshifter")) ? check : space;
		mob_page_f_data[26].data = (idx->race == get_npc_race("fish")) ? check : space;
		mob_page_f_data[27].data = (idx->race == get_npc_race("fly")) ? check : space;
		mob_page_f_data[28].data = (idx->race == get_npc_race("gargoyle")) ? check : space;
		/*mob_page_f_data[29].data = (idx->race == get_npc_race("gargoyle")) ? check : space;*/
		mob_page_f_data[29].data = (idx->race == get_npc_race("ape")) ? check : space;
		mob_page_f_data[30].data = (idx->race == get_npc_race("shrieker")) ? check : space;
		mob_page_f_data[31].data = (idx->race == get_npc_race("insect")) ? check : space;
		mob_page_f_data[32].data = (idx->race == get_npc_race("locust")) ? check : space;
		mob_page_f_data[33].data = (idx->race == get_npc_race("ghoul")) ? check : space;
		mob_page_f_data[34].data = (idx->race == get_npc_race("baboon")) ? check : space;
		mob_page_f_data[35].data = (idx->race == get_npc_race("stirge")) ? check : space;
		mob_page_f_data[36].data = (idx->race == get_npc_race("mammal")) ? check : space;
		mob_page_f_data[37].data = (idx->race == get_npc_race("snake")) ? check : space;
		mob_page_f_data[38].data = (idx->race == get_npc_race("gnoll")) ? check : space;
		mob_page_f_data[39].data = (idx->race == get_npc_race("bat")) ? check : space;
		mob_page_f_data[40].data = (idx->race == get_npc_race("wight")) ? check : space;
		mob_page_f_data[41].data = (idx->race == get_npc_race("porcine")) ? check : space;
		mob_page_f_data[42].data = (idx->race == get_npc_race("spider")) ? check : space;
		mob_page_f_data[43].data = (idx->race == get_npc_race("gnome")) ? check : space;
		mob_page_f_data[44].data = (idx->race == get_npc_race("bear")) ? check : space;
		mob_page_f_data[45].data = (idx->race == get_npc_race("reptile")) ? check : space;
		mob_page_f_data[46].data = (idx->race == get_npc_race("worm")) ? check : space;
		mob_page_f_data[47].data = (idx->race == get_npc_race("goblin")) ? check : space;
		mob_page_f_data[48].data = (idx->race == get_npc_race("boar")) ? check : space;
		mob_page_f_data[49].data = (idx->race == get_npc_race("rodent")) ? check : space;
		mob_page_f_data[50].data = (idx->race == get_npc_race("golem")) ? check : space;
		mob_page_f_data[51].data = (idx->race == get_npc_race("cat")) ? check : space;
		mob_page_f_data[52].data = (idx->race == get_npc_race("troglodyte")) ? check : space;
		mob_page_f_data[53].data = (idx->race == get_npc_race("gelatin")) ? check : space;
		mob_page_f_data[54].data = (idx->race == get_npc_race("gorgon")) ? check : space;
		mob_page_f_data[55].data = (idx->race == get_npc_race("dog")) ? check : space;
		mob_page_f_data[56].data = (idx->race == get_npc_race("spirit")) ? check : space;
		mob_page_f_data[57].data = (idx->race == get_npc_race("mold")) ? check : space;
		mob_page_f_data[58].data = (idx->race == get_npc_race("harpy")) ? check : space;
		mob_page_f_data[59].data = (idx->race == get_npc_race("ferret")) ? check : space;
		mob_page_f_data[60].data = (idx->race == get_npc_race("magical")) ? check : space;
		mob_page_f_data[61].data = (idx->race == get_npc_race("ooze")) ? check : space;
		mob_page_f_data[62].data = (idx->race == get_npc_race("hobgoblin")) ? check : space;
		mob_page_f_data[63].data = (idx->race == get_npc_race("horse")) ? check : space;
		mob_page_f_data[64].data = (idx->race == get_npc_race("slime")) ? check : space;
		mob_page_f_data[65].data = (idx->race == get_npc_race("kobold")) ? check : space;
		mob_page_f_data[66].data = (idx->race == get_npc_race("mule")) ? check : space;
		mob_page_f_data[67].data = (idx->race == get_npc_race("lizardman")) ? check : space;
		mob_page_f_data[68].data = (idx->race == get_npc_race("rat")) ? check : space;
		mob_page_f_data[69].data = (idx->race == get_npc_race("lycanthrope")) ? check : space;
		mob_page_f_data[79].data = (idx->race == get_npc_race("shrew")) ? check : space;
		mob_page_f_data[71].data = (idx->race == get_npc_race("minotaur")) ? check : space;
		mob_page_f_data[72].data = (idx->race == get_npc_race("wolf")) ? check : space;
		break;
	case 'a':
	default:
		m_data = mob_page_a_data;
		/*
		 * mob_page_a_data[0].data    =
		 * format_string(idx->player_name,70);
		 * mob_page_a_data[1].data    =
		 * format_string(idx->short_descr,60);
		 * mob_page_a_data[2].data    =
		 * format_string(idx->long_descr,60); mob_page_a_data[3].data
		 * =  format_string(idx->description,60);
		 */
		format_string(tmp0, idx->player_name, 59);
		format_string(tmp1, idx->short_descr, 59);
		format_string(tmp2, idx->long_descr, 69);
		format_string(tmp3, idx->description, 69);
		mob_page_a_data[0].data = tmp0;
		mob_page_a_data[1].data = tmp1;
		mob_page_a_data[2].data = tmp2;
		mob_page_a_data[3].data = tmp3;

		mob_page_a_data[4].data = &(idx->vnum);
		mob_page_a_data[5].data = &(idx->level);
		mob_page_a_data[6].data = (char *) "see sect. 4";
		mob_page_a_data[7].data = npc_race[idx->race];
		mob_page_a_data[8].data = (char *) npc_sex[idx->sex];
		mob_page_a_data[9].data = (int *) &(idx->gold);
		mob_page_a_data[10].data = &(idx->alignment);
		mob_page_a_data[11].data = &(idx->perm_str);
		mob_page_a_data[12].data = &(idx->perm_int);
		mob_page_a_data[13].data = &(idx->perm_wis);
		mob_page_a_data[14].data = &(idx->perm_dex);
		mob_page_a_data[15].data = &(idx->perm_con);
		mob_page_a_data[16].data = &(idx->perm_cha);
		mob_page_a_data[17].data = &(idx->ac);

		mob_page_a_data[18].data = (idx->class == 0) ? check : space;
		mob_page_a_data[19].data = (idx->class == 1) ? check : space;
		mob_page_a_data[20].data = (idx->class == 2) ? check : space;
		mob_page_a_data[21].data = (idx->class == 3) ? check : space;
		mob_page_a_data[22].data = (idx->class == 4) ? check : space;
		mob_page_a_data[23].data = (idx->class == 5) ? check : space;
		mob_page_a_data[24].data = (idx->class == 6) ? check : space;
		mob_page_a_data[25].data = (idx->class == 20) ? check : space;
		mob_page_a_data[26].data = (idx->class == 21) ? check : space;
		mob_page_a_data[27].data = (idx->class == 22) ? check : space;
		mob_page_a_data[28].data = (idx->class == 23) ? check : space;
		mob_page_a_data[29].data = (idx->class == 24) ? check : space;
		mob_page_a_data[30].data = (idx->class == 25) ? check : space;
		break;
	}


	for (i = 0; m_data[i].ptrType != (int) NULL; i++) {
		sprintf(buf, "\x1B[%d;%dH", m_data[i].x, m_data[i].y);
		write_to_buffer(ch->desc, buf, strlen(buf));
		switch(m_data[i].ptrType)
		{
                    case INT:    sprintf(buf, m_data[i].outFormat,*((int *)(m_data[i].data)));
			         break;
                    case SH_INT: sprintf(buf, m_data[i].outFormat,*((sh_int *)(m_data[i].data)));
			         break;
                    default    : sprintf(buf, m_data[i].outFormat,  m_data[i].data);
		}

#ifdef WRONG_WAY
		if (m_data[i].ptrType == INT) {
			sprintf(buf, m_data[i].outFormat, *((int *) (m_data[i].data)));
		}
		if (m_data[i].ptrType == SH_INT) {
			sprintf(buf, m_data[i].outFormat, *((sh_int *) (m_data[i].data)));
		} else {
			sprintf(buf, m_data[i].outFormat, m_data[i].data);
		}
#endif
		write_to_buffer(ch->desc, buf, strlen(buf));
	}

	offset = get_offset(ch);

	/* restrict scrolling area of screen */
	sprintf(buf, "\x1B[%d;%dr", offset, ch->pagelen);
	write_to_buffer(ch->desc, buf, strlen(buf));

	/* goes to pglen,1 */
	sprintf(buf, "\x1B[%d;1H", (ch->pagelen));
	write_to_buffer(ch->desc, buf, strlen(buf));

	send_to_char("\n\r", ch);
}

int 
get_bvl(int bitvector)
{
	int             x;

	for (x = 0; x < 31; x++)
		if (1 << x == bitvector)
			return x;
	return 0;
}


int 
get_affect(OBJ_INDEX_DATA * obj, int type)
{
	AFFECT_DATA    *aff;
	bool            bitsearch, ris_search;
	int             apply = 0, bit = 0, total;

	bitsearch = TRUE;
	total = 0;
	ris_search = FALSE;
	if (type < 1000) {
		apply = type;
		bitsearch = FALSE;
	} else if (IS_SET(type, GA_AFFECTED)) {
		REMOVE_BIT(type, GA_AFFECTED);
		bit = 1 << type;
		apply = APPLY_AFFECT;
	} else if (IS_SET(type, GA_RIS)) {
		REMOVE_BIT(type, GA_RIS);
		bit = 1 << type;
		apply = -1;
	}
	for (aff = obj->first_affect; aff; aff = aff->next) {
		if (aff->location == apply
		    || (bitsearch
			&& aff->location >= APPLY_AFFECT
			&& aff->location <= APPLY_SUSCEPTIBLE)) {
			if (bitsearch) {
				if (IS_SET(aff->modifier, bit))
					switch (aff->location) {
					case APPLY_AFFECT:
						total = 1;
						break;
					case APPLY_RESISTANT:
						SET_BIT(total, RIS_R00);
						break;
					case APPLY_IMMUNE:
						SET_BIT(total, RIS_0I0);
						break;
					case APPLY_SUSCEPTIBLE:
						SET_BIT(total, RIS_00S);
						break;
					}
			} else
				total += aff->modifier;
		}
	}
	return total;
}




/*
 * Constants follow.
 */

const char     *npc_sex[3] = {
	"Neuter", "Male", "Female"
};
const char     *ris_strings[] = {
	"...",			/* 0 */
	"R..",			/* 1 */
	".I.",			/* 2 */
	"RI.",			/* 3 */
	"..S",			/* 4 */
	"R.S",			/* 5 */
	".IS",			/* 6 */
	"RIS"
};				/* 7 */

const char mob_page_a[] = ".----------[mob page a]-----[%10.10s]-------------------------------------.\r\n"
"|[1]TEXT|  a]Name :                                                          |\r\n"
"|-------'  b]Short:                                                          |\r\n"
"|c]Long:                                                                     |\r\n"
"|d]Desc:                                                                     |\r\n"
":---------------------+------------------------------------------------------'\r\n"
"|[2]ATTRIBUTES |      |[3]STATS |   |[4]CLASS |                        |\r\n"
"|--------------'      |---------'   |---------'                        |\r\n"
"| a]Vnum :            | a]Str:      | a]Mage   ( )    b]Cleric    ( )  |\r\n"
"| b]Level:            | b]Int:      | c]Thief  ( )    d]Warrior   ( )  |\r\n"
"| c]Class:            | c]Wis:      | e]Vampire( )    f]Druid     ( )  |\r\n"
"| d]Race :            | d]Dex:      | g]Ranger ( )    h]Baker     ( )  |\r\n"
"| e]Sex  :            | e]Con:      | i]Butcher( )    j]Blacksmith( )  |\r\n"
"| f]Gold :            | f]Cha:      | k]Mayor  ( )    l]King      ( )  |\r\n"
"| g]Align:            | g]AC :      | m]Queen  ( )                     |\r\n"
":---------------------+-------------+----------------------------------'\r\n";

const char mob_page_b[] = ".----------[mob page b]-----[%10.10s]--------------------------.\r\n"
"|[1]ACT FLAGS| a]Sentinel  ( )  b]Scavenger  ( )  c]Aggressiv( )  |\r\n"
"|------------' d]StayArea  ( )  e]Wimpy      ( )  f]Pet      ( )  |\r\n"
"|g]Train  ( )  h]Practice  ( )  i]Immortal   ( )  j]Deadly   ( )  |\r\n"
"|k]PolySlf( )  l]MetaAggres( )  m]Guardian   ( )  n]Running  ( )  |\r\n"
"|o]NoWandr( )  p]Mountable ( )  q]Mounted    ( )                  |\r\n"
":-----------------------------------------------------------------:\r\n"
"|[2]AFF FLAGS| a]Blind     ( )  b]DetInvis   ( )  c]DetEvil  ( )  |\r\n"
"|------------' d]DetMag    ( )  e]DetHiddn   ( )  f]Hold     ( )  |\r\n"
"|g]Sanct  ( )  h]Faerie    ( )  i]InfraRed   ( )  j]Cursed   ( )  |\r\n"
"|k]Flaming( )  l]Poison    ( )  m]Protect    ( )  n]Paralysis( )  |\r\n"
"|o]Sleep  ( )  p]Hide      ( )  q]Sneak      ( )  r]Charm    ( )  |\r\n"
"|s]Flying ( )  t]PassDoor  ( )  u]TrueSight  ( )  v]DetTraps ( )  |\r\n"
"|w]Scrying( )  x]FireShield( )  y]ShockShield( )  z]Floating ( )  |\r\n"
"|A]Invisib( )                                                     |\r\n"
"`-----------------------------------------------------------------'\r\n";

const char mob_page_c[] = ".----------.----------[mob page c]-----[%10.10s]-------------------.\r\n"
"|[1]ATTACKS|  a]bite    ( )      b]claws( )         c]firebreath  ( ) |\r\n"
"|----------'  d]bash    ( )      e]stun ( )         f]acidBreath  ( ) |\r\n"
"|g]drain ( )  h]kick    ( )      i]feed ( )         j]gasBreath   ( ) |\r\n"
"|k]poison( )  l]gaze    ( )      m]harm ( )         n]frostBreath ( ) |\r\n"
"|o]curse ( )  p]backstab( )      q]punch( )         r]lightBreath ( ) |\r\n"
"|s]weaken( )  t]tail    ( )      u]sting( )         v]causeSerious( ) |\r\n"
"|w]trip  ( )  x]fireBall( )      y]gouge( )         z]causeCritica( ) |\r\n"
"|             B]flameStrike( )                      D]earthQuake  ( ) |\r\n"
"|             F]colorSpray ( )   G]nastyPoison( )   H]blindness   ( ) |\r\n"
":-----------.---------------------------------------------------------+-.\r\n"
"|[2]DEFENSES|   a]sanctuary  ( )   b]cureLight   ( )   c]dispelEvil ( ) |\r\n"
"|-----------'   d]shield     ( )   e]cureSerious ( )   f]dispelMagic( ) |\r\n"
"|g]dodge ( )    h]fireShield ( )   i]cureCritical( )   j]teleport   ( ) |\r\n"
"|k]parry ( )    l]shockShield( )   m]heal        ( )   n]bless      ( ) |\r\n"
"|o]disarm( )    p]stoneSkin  ( )   q]mobSummon1  ( )   r]mobSummon2 ( ) |\r\n"
"|                                  u]mobSummon3  ( )   v]mobSummon4 ( ) |\r\n"
":-----------------------------------------------------------------------'\r\n";


const char mob_page_d[] = ".----------------.----------[mob page d]-----[%10.10s]----------.\r\n"
"|[1]SAVING THROWS| a]Poison/Death     %%   c]Paralysis/Petrify    %% |\r\n"
"|----------------' c]Wand             %%   d]Breath               %% |\r\n"
"|                  e]Spell/Staff      %%                            |\r\n"
":------------------------------------------------------------------^-----.\r\n"
"|[2]BODY  | a]Fins( )  b]Ear ( )  c]Eye ( )  d]LongTongue( ) e]Scales ( )|\r\n"
"|   PARTS | f]Tail( )  g]Arms( )  h]Legs( )  i]Tentacles ( ) j]Brains ( )|\r\n"
"|---------' k]Guts( )  l]Head( )  m]Feet( )  n]EyeStalks ( ) o]Fingers( )|\r\n"
"|           p]Wings( ) q]Hands( ) r]Heart( )                             |\r\n"
"|ATTACKING: u]Claws( ) v]Horns( ) w]Tusks( )                             |\r\n"
"|           z]TailAttack( )       A]SharpScales( )                       |\r\n"
":---------.--------------------------------------------------------------:\r\n"
"|[3]FIGHT |  a]Attacks:  [   ]       b]HitDice:[     ]D[     ]+[     ]   |\r\n"
"|   STATS |  c]HitRoll:[     ]                                           |\r\n"
"|---------'  d]DamRoll:[     ]       e]DamDice:[     ]D[     ]+[     ]   |\r\n"
"`------------------------------------------------------------------------'\r\n";


const char mob_page_e[] = ".---------.----------[mob page e]-----[%10.10s]----------------------.\r\n"
"|[1]SPEC'L| a]fido   ( )    b]castAdept ( )        c]breathFire     ( ) |\r\n"
"|   FUNCTS| d]guard  ( )    e]castCleric( )        f]breathFrost    ( ) |\r\n"
"|---------' g]janitor( )    h]castMage  ( )        i]breathAcid     ( ) |\r\n"
"|           j]thief  ( )    k]castUndead( )        l]breathGas      ( ) |\r\n"
"|           m]mayor  ( )                           o]breathLightning( ) |\r\n"
"|           p]poison ( )    q]executionr( )        r]breathAny      ( ) |\r\n"
":-----------.-------------------------------------------------------.---+\r\n"
"|[2]DEFAULT |    a]Dead   ( )   b]Mortally( )    c]Incapacitated( ) |\r\n"
"|   POSITION|    d]Stunned( )   e]Sleeping( )    f]Resting      ( ) |\r\n"
"|-----------'    g]Sitting( )   h]Fighting( )    i]Standing     ( ) |\r\n"
"|                k]Mounted( )                                       |\r\n"
":--------.----------------------------------------------------------^----.\r\n"
"|[3]RIS  |   a]fire  ... b]cold  ... c]elect ... d]energ ... e]poison ...|\r\n"
"|PROFILE |   f]blunt ... g]pierce... h]slash ... i]acid  ... j]nonMag ...|\r\n"
"|--------'   k]drain ... l]sleep ... m]charm ... n]hold  ... o]magic  ...|\r\n"
"|p]plus1 ... q]plus2 ... r]plus3 ... s]plus4 ... t]plus5 ... u]plus6  ...|\r\n"
":------------------------------------------------------------------------'\r\n";





const char room_page_a[] = ".---------.--[rmenu page a]-------[%10.10s]-----------------------------.\r\n"
"||[1]TEXT|                                                                  |\r\n"
"|--------'                                                                  |\r\n"
"|a]Name:                                                                    |\r\n"
"|                                                                           |\r\n"
"|b]Description:                                                             |\r\n"
"|                                                                           |\r\n"
"|                                                                           |\r\n"
":---------------------+-----------------------------------------------------'\r\n";



const char room_page_b[] = ".---------.--[rmenu page b]-------[%10.10s]----------------.\r\n"
"|[1]ROOM  | a]Dark      ( )  b]Death    ( )    c]NoMobs   ( ) |\r\n"
"|   FLAGS | d]Indoors   ( )  e]Lawful   ( )    f]NoMagic  ( ) |\r\n"
"|---------' g]PetShop   ( )  h]Neutral  ( )    i]NoDrop   ( ) |\r\n"
"|           j]Safe      ( )  k]Chaotic  ( )    l]NoDropAll( ) |\r\n"
"|           m]Teleport  ( )  n]Donation ( )    o]NoRecall ( ) |\r\n"
"|           p]Private   ( )  q]Solitary ( )    r]NoAstral ( ) |\r\n"
"|           s]Tunnel    ( )  t]Silence  ( )    u]NoSummon ( ) |\r\n"
"|           v]ClanStorge( )  w]LogSpeech( )    x]NoFloor  ( ) |\r\n"
":-------------------------------------------------------------+--------.\r\n"
"|[2]SECTOR | a]Inside  ( )  b]Forest      ( )  c]City( )  d]Hills ( )  |\r\n"
"|   TYPE   | e]Mountain( )  f]Water_Swim  ( )  g]Air ( )  h]Desert( )  |\r\n"
"|----------' i]Dunno   ( )  j]Water_NoSwim( )             l]Field ( )  |\r\n"
"`----------------------------------------------------------------------'\r\n";


const char room_page_c[] = ".----------.-[rmenu page c]-------[%10.10s]--------------------------.\r\n"
"|[1] EXIT  | Section [2] now    :           n]orth( )            u]p( ) |\r\n"
"|DIRECTION | shows info about   :   w]est( )          e]ast( )          |\r\n"
"| SUBMENU  |   the exit to      :           s]outh( )          d]own( ) |\r\n"
":-----------------------------------------------------------------------:\r\n"
"|[2] EXIT  | a]IsDoor  ( )  b]Secret   ( )  KeyWords:                   |\r\n"
"|PROPERTIES| c]Closed  ( )  d]PickProof( )  m:[#######################] |\r\n"
"|----------' e]Locked  ( )  f]PassProof( )  n:[#######################] |\r\n"
"|                                           o:[#######################] |\r\n"
"|i]To Room: [     ] [                   ]   p:[#######################] |\r\n"
"|j]    Key: [     ] [                   ]   q:[#######################] |\r\n"
"|k]:ExitDescr:                                                          |\r\n"
"|                                                                       |\r\n"
"`-----------------------------------------------------------------------'\r\n";


const char room_page_d[] = ".---------.--[rmenu page d]-------[%10.10s]---------------------------.\r\n"
"|[1] STD  |   a]Hut      ( )  b]Tent    ( )  c]Hovel   ( )  d]Campsite( ) |\r\n"
"|   ROOMS |   e]Shack    ( )  f]Cabin   ( )  g]Homested( )  h]Keep    ( ) |\r\n"
"|---------'   i]Fortress ( )  j]Castle  ( )  k]GuardHse( )  l]Temple  ( ) |\r\n"
"|m]Store  ( ) n]Graveyard( )  o]Monastry( )  p]Stable  ( )  q]Tavern  ( ) |\r\n"
"|r]Basemnt( ) s]Bedroom  ( )  t]BnquetRm( )  u]Corridor( )  v]Attic   ( ) |\r\n"
"|w]Vault  ( ) x]SittingRm( )  y]Study   ( )  z]Passage ( )  A]Tower   ( ) |\r\n"
"|B]Crypt  ( ) C]WorkRoom ( )  D]Lab     ( )  E]Hallway ( )  F]Turret  ( ) |\r\n"
"|G]StorRm ( ) H]Kitchen  ( )  I]Larder  ( )  J]Stairway( )  K]Rooftop ( ) |\r\n"
"|L]Closet ( ) M]Office   ( )  N]Treasury( )  O]Landing ( )  P]Balcony ( ) |\r\n"
"|Q]Foyer  ( ) R]DrawingRm( )  S]Den     ( )  T]Ladder  ( )  U]Catwalk ( ) |\r\n"
"|V]Entrnce( ) W]Arboretum( )  X]Library ( )  Y]Vent    ( )  Z]Shaft   ( ) |\r\n"
"|0]Gate   ( ) 1]AudiencRm( )  2]Consrvty( )  3]DumbWatr( )  4]Chimney ( ) |\r\n"
"|5]Porch  ( ) 6]ClassRoom( )  7]CloakRm ( )  8]Lawn    ( )  9]Garden  ( ) |\r\n"
"|!]Lake   ( ) @]Forest  ( )   #]Swamp   ( )  $]Well    ( )  %%]Street  ( ) |\r\n"
"|^]River  ( ) &]Canyon  ( )   *]Beach   ( )  (]Mine    ( )  )]Road    ( ) |\r\n"
"|-]Stream ( ) _]Clearing( )   +]SnakePit( )  =]Tunnel  ( )  |]Path    ( ) |\r\n"
"|\\]Rapids ( ) ~]Desert  ( )   `]SandStrm( )  {]Rope    ( )  []Cliff   ( ) |\r\n"
"|}]CaveRiv( ) ]]Jungle  ( )   :]Sandbar ( )  \"]RopeBrdg( )  ']Bridge  ( ) |\r\n"
"|<]CaveLak( ) ,]Cave    ( )   >]None    ( )  .]RopeLadr( )  ?]NatlBrdg( ) |\r\n"
"`-------------------------------------------------------------------------'\r\n";



const char mob_page_f[] = ".---------.--[mmenu page f]-------[%10.10s]---------------------------.\r\n"
"|[1] STD  |   a]Human    ( )  b]Neandrtl( )  c]Avis    ( )  d]Amphib  ( ) |\r\n"
"|   ROOMS |   e]Elf      ( )  f]Orc     ( )  g]Bugbear ( )  h]Bovine  ( ) |\r\n"
"|---------'   i]Dwarf    ( )  j]Skeleton( )  k]Dragon  ( )  l]Canine  ( ) |\r\n"
"|m]Ant    ( ) n]Halfling ( )  o]Thoul   ( )  p]RustMnst( )  q]Crustacn( ) |\r\n"
"|r]Bee    ( ) s]Pixie    ( )  t]Undead  ( )  u]Shadow  ( )  v]Feline  ( ) |\r\n"
"|w]Beetle ( ) x]Vampire  ( )  y]Zombie  ( )  z]ShapShft( )  A]Fish    ( ) |\r\n"
"|B]Fly    ( ) C]Gargoyle ( )  D]Ape     ( )  E]Shrieker( )  F]Insect  ( ) |\r\n"
"|G]Locust ( ) H]Ghoul    ( )  I]Baboon  ( )  J]Stirge  ( )  K]Mammal  ( ) |\r\n"
"|L]Snake  ( ) M]Gnoll    ( )  N]Bat     ( )  O]Wight   ( )  P]Porcine ( ) |\r\n"
"|Q]Spider ( ) R]Gnome    ( )  S]Bear    ( )                 U]Reptile ( ) |\r\n"
"|V]Worm   ( ) W]Goblin   ( )  X]Boar    ( )                 Z]Rodent  ( ) |\r\n"
"|             1]Golem    ( )  2]Cat     ( )                 4]Trogldte( ) |\r\n"
"|5]Gelatin( ) 6]Gorgon   ( )  7]Dog     ( )                 9]Spirit  ( ) |\r\n"
"|!]Mold   ( ) @]Harpy    ( )  #]Ferret  ( )                 %%]Magical ( ) |\r\n"
"|^]Ooze   ( ) &]Hobgoblin( )  *]Horse   ( )                               |\r\n"
"|-]Slime  ( ) _]Kobold   ( )  +]Mule    ( )                               |\r\n"
"|             ~]Lizardman( )  `]Rat     ( )                               |\r\n"
"|             ]]Lycanthrp( )  :]Shrew   ( )                               |\r\n"
"|             ,]Minotaur ( )  >]Wolf    ( )                               |\r\n"
"`-------------------------------------------------------------------------'\r\n";



const char obj_page_a[] = ".-------.----[omenu page a]-------[%10.10s]----------------------------.\r\n"
"|[1]TEXT|  a]Name :                                                       |\r\n"
"|-------'  b]Short:                                                       |\r\n"
"|c]Long:                                                                  |\r\n"
"|d]ActionDesc:                                                            | \r\n"
":---------------------------------------------+---------------------------'\r\n"
"|[2]VALUES|         Value Meaning             |[3]STATS |             |\r\n"
"|---------'         ---------------           |---------'             |\r\n"
"| a]Value0:[      ] (                      )  | a]Weight:[      ]     |\r\n"
"| b]Value1:[      ] (                      )  | b]Cost:  [      ]     |\r\n"
"| c]Value2:[      ] (                      )  |                       |\r\n"
"| d]Value3:[      ] (                      )  |                       |\r\n"
"`---------------------------------------------+-----------------------'\r\n";


const char obj_page_b[] = ".---------.--[omenu page b]-------[%10.10s]----------------------.\r\n"
"|[1]OBJECT| a]none   ( )    b]fountain  ( )    c]furniture      ( ) |\r\n"
"|   TYPES | d]armor  ( )    e]food      ( )    f]money          ( ) |\r\n"
"|---------' g]potion ( )    h]pullchain ( )    i]weapon         ( ) |\r\n"
"|j]map ( )  k]light  ( )    l]button    ( )    m]treasure       ( ) |\r\n"
"|n]herb( )  o]trash  ( )    p]switch    ( )    q]container      ( ) |\r\n"
"|r]wand( )  s]scroll ( )    t]staff     ( )    u]bloodstain     ( ) |\r\n"
"|v]pill( )  w]blood  ( )    x]dial      ( )    y]_oldtrap       ( ) |\r\n"
"|z]key ( )  A]incense( )    B]lever     ( )    C]corpse_pc      ( ) |\r\n"
"|D]pipe( )  E]scraps ( )    F]trap      ( )    G]corpse         ( ) |\r\n"
"|H]book( )  I]match  ( )    J]drinkcon  ( )    K]portal         ( ) |\r\n"
"|L]_pen( )  M]fire   ( )    N]herbcon   ( )    O]_fireweapon    ( ) |\r\n"
"|P]boat( )  Q]rune   ( )    R]runepouch ( )    S]_missile       ( ) |\r\n"
"`-------------------------------------------------------------------'\r\n";


const char obj_page_c[] = ".----------.-[omenu page c]-------[%10.10s]--------------------------.\r\n"
"|[1] BASIC |  a]none:( )    b]save_poi   :[    ]%%     c]age   :[    ]   |\r\n"
"|  AFFECTS |  d]STR :[   ]  e]save_rod   :[    ]%%     f]class :[    ]   |\r\n"
"|----------'  g]INT :[   ]  h]save_para  :[    ]%%     i]level :[    ]   |\r\n"
"|             j]WIS :[   ]  k]save_breath:[    ]%%     l]sex   :[    ]   | \r\n"
"|             m]DEX :[   ]  n]save_spell :[    ]%%     o]height:[    ]   |\r\n"
"|             p]CON :[   ]                            r]weight:[    ]   |\r\n"
"|             s]CHA :[   ]                                              |\r\n"
"|  v]Mana:[     ]            w]AC     :[     ]                          |\r\n"
"|  y]HP  :[     ]            z]HitRoll:[     ]        A]Gold:[     ]    |\r\n"
"|  B]Move:[     ]            C]DamRoll:[     ]        D]Exp :[     ]    |\r\n"
":--------.---------------------------------------------------------------.\r\n"
"|[2] RIS |   a]fire  ... b]cold  ... c]elect ... d]energ ... e]poison ...|\r\n"
"|AFFECTS |   f]blunt ... g]pierce... h]slash ... i]acid  ... j]nonMag ...|\r\n"
"|--------'   k]drain ... l]sleep ... m]charm ... n]hold  ... o]magic  ...|\r\n"
"|p]plus1 ... q]plus2 ... r]plus3 ... s]plus4 ... t]plus5 ... u]plus6  ...|\r\n"
"`------------------------------------------------------------------------'\r\n";


const char obj_page_d[] = ".-----------.[omenu page d]-------[%10.10s]--------------------.\r\n"
":[1]ADVANCED|                                                     |\r\n"
"|    AFFECTS|  a]Blind     ( )  b]DetInvis   ( )  c]DetEvil  ( )  |\r\n"
"|-----------'  d]DetMag    ( )  e]DetHiddn   ( )  f]Hold     ( )  |\r\n"
"|g]Sanct  ( )  h]Faerie    ( )  i]InfraRed   ( )  j]Cursed   ( )  |\r\n"
"|k]Flaming( )  l]Poison    ( )  m]Protection ( )  n]Paralysis( )  |\r\n"
"|o]Sleep  ( )  p]Hide      ( )  q]Sneak      ( )  r]Charm    ( )  |\r\n"
"|s]Flying ( )  t]PassDoor  ( )  u]TrueSight  ( )  v]DetTraps ( )  |\r\n"
"|w]Scrying( )  x]FireShield( )  y]ShockShield( )  z]Floating ( )  |\r\n"
"|A]Invisib( )                                                     |\r\n"
"`-----------------------------------------------------------------'\r\n";


const char obj_page_e[] = ".----------.-[omenu page e]-------[%10.10s]--------------------.\r\n"
"|[1] OBJECT|   a]Evil   ( )  b]AntiGood   ( )  c]NoDrop    ( )    |\r\n"
"|    FLAGS |   d]Invis  ( )  e]AntiEvil   ( )  f]NoRemove  ( )    |\r\n"
"|----------'   g]Hidden ( )  h]AntiNeut   ( )  i]Inventory ( )    |\r\n"
"|j]Glow   ( )  k]Bless  ( )  l]AntiMage   ( )  m]Donation  ( )    |\r\n"
"|n]Hum    ( )  o]Metal  ( )  p]AntiThief  ( )  q]ClanObject( )    |\r\n"
"|r]Dark   ( )  s]Organic( )  t]AntiWarrior( )  u]ClanCorpse( )    |\r\n"
"|v]Magic  ( )                x]AntiCleric ( )  y]Prototype ( )    |\r\n"
"|z]Loyal  ( )                B]AntiVampire( )  C]Deathrot  ( )    |\r\n"
"|                            F]AntiDruid  ( )                     |\r\n"
":-----------------------------------------------------------------.\r\n"
"|[2] WEAR  |   a]Take ( )  b]Finger  ( )  c]Neck  ( )  d]Body ( ) |\r\n"
"|    FLAGS |   e]Head ( )  f]Legs    ( )  g]Feet  ( )  h]Hands( ) |\r\n"
"|----------'   i]Arms ( )  j]Shield  ( )  k]About ( )  l]Waist( ) |\r\n"
"|m]Wrist  ( )  n]Wield( )  o]Hold    ( )  p]_dual_( )  q]Ears ( ) |\r\n"
"|r]Eyes   ( )                                                     |\r\n"
"`-----------------------------------------------------------------'\r\n";


const char mob_help_page[] = "\r\n"
".------------[mmenu help pg]------[%10.10s]--------------------.\r\n"
"|    Page contents                         |                      |\r\n"
"|                                          | MISC COMMANDS.       |\r\n"
"| PAGE     CONTENTS                        | --------------       |\r\n"
"|------|-----------------------------------| clear:  clear screen |\r\n"
"|  a   | strings, attrs, stats, class      | redraw: redraw menu  |\r\n"
"|  b   | act & affected                    | refresh:redraw data  |\r\n"
"|  c   | attacks & defenses                | pagelen:set page     |\r\n"
"|  d   | saves, bodyparts, and fight stats |           length     |\r\n"
"|  e   | specials, position, and RIS       |                      |\r\n"
"|  f   | race                              |                      |\r\n"
"`-----------------------------------------------------------------'\r\n";


const char room_help_page[] = "\r\n"
".------------[rmenu help pg]------[%10.10s]--------------------.\r\n"
"|    Page contents                         |                      |\r\n"
"|                                          | MISC COMMANDS.       |\r\n"
"| PAGE     CONTENTS                        | --------------       |\r\n"
"|------|-----------------------------------| clear:  clear screen |\r\n"
"|  a   | name & description                | redraw: redraw menu  |\r\n"
"|  b   | room flags & sector type          | refresh:redraw data  |\r\n"
"|  c   | exit stuff                        | pagelen:set page     |\r\n"
"| (d)  | (map codes)                       |           length     |\r\n"
"|      |                                   |                      |\r\n"
"|      |                                   |                      |\r\n"
"`-----------------------------------------------------------------'\r\n";


const char obj_help_page[] = "\r\n"
".------------[omenu help pg]------[%10.10s]--------------------.\r\n"
"|    Page contents                         |                      |\r\n"
"|                                          | MISC COMMANDS.       |\r\n"
"| PAGE     CONTENTS                        | --------------       |\r\n"
"|------|-----------------------------------| clear:  clear screen |\r\n"
"|  a   | strings, values, weight/cost      | redraw: redraw menu  |\r\n"
"|  b   | object type                       | refresh:redraw data  |\r\n"
"|  c   | affects & affect RIS              | pagelen:set page     |\r\n"
"|  d   | affect affecteds                  |           length     |\r\n"
"|  e   | oflags & wear flags               |                      |\r\n"
"|      |                                   |                      |\r\n"
"`-----------------------------------------------------------------'\r\n";


const char control_page_a[] = ".--------.---[cmenu page a]-----.[%10.10s]-------------------------------.\r\n"
"||[1]MAIL|         ||[2]CHANNELS|                                           |\r\n"
"|--------'         |------------'            	                            |\r\n"
"|a]Read free:      |a]Muse:                                                 |\r\n"
"|b]Write free:     |b]Think:                                                |\r\n"
"|c]Read all:       |c]Log:                                                  |\r\n"
"|d]Take all:       |d]Build:                                                |\r\n"
"`---------------------------------------------------------------------------'\r\n";


const char control_help_page[] = "\r\n"
".------------[cmenu help pg]------[%10.10s]--------------------.\r\n"
"|    Page contents                         |                      |\r\n"
"|                                          | MISC COMMANDS.       |\r\n"
"| PAGE     CONTENTS                        | --------------       |\r\n"
"|------|-----------------------------------| clear:  clear screen |\r\n"
"|  a   | main control panel                | redraw: redraw menu  |\r\n"
"|      |                                   | refresh:redraw data  |\r\n"
"|      |                                   | pagelen:set page     |\r\n"
"|      |                                   |           length     |\r\n"
"|      |                                   |                      |\r\n"
"|      |                                   |                      |\r\n"
"`-----------------------------------------------------------------'\r\n";


const char * item_lookup [46] [5] =
{ {"bad type",	"bad type",	"bad type",	"bad type",		"bad type"},  	/* 0 */
  {"light",	"unused",	"unused",	"unused",		"unused"},  	/* 1 */
  {"scroll",	"level",	"sn1",		"sn2",			"sn3"},  	/* 2 */
  {"wand",	"level",	"max charges",	"current charges",	"sn"},  	/* 3 */
  {"staff",	"level",	"max charges",	"current charges",	"sn"},  	/* 4 */
  {"weapon",	"unused",       "unused",       "unused",               "hit string"},  /* 5 */
  {"undefined",	"unused",       "unused",       "unused",               "undefined"},  	/* 6 */
  {"undefined",	"unused",       "unused",       "unused",               "undefined"},  	/* 7 */
  {"treasure",	"type",         "condition",    "unused",               "unused"},	/*  8*/
  {"armor",	"condition",    "original ac",  "unused",               "unused"},	/*  9*/
  {"potion",	"level",	"sn1",		"sn2",			"sn3"},		/* 10*/
  {"undefined",	"unused",       "unused",       "unused",               "undefined"},	/* 11*/
  {"furniture",	"unused",	"unused",	"unused",		"unused"},	/* 12*/
  {"trash",	"unused",	"unused",	"unused",		"unused"},	/* 13*/
  {"undefined",	"unused",       "unused",       "unused",               "undefined"},	/* 14*/
  {"container",	"max weight",	"flags",	"key vnum",		"condition"},	/* 15*/
  {"undefined",	"unused",       "unused",       "unused",               "undefined"},	/* 16*/
  {"drink con",	"capacity",	"how full",	"liquid number",	"is poisoned?"},/* 17*/
  {"key",	"unused",	"unused",	"unused",		"unused"},	/* 18*/
  {"food",	"food_hours",	"condition",	"unused",		"is poisoned?"},/* 19*/
  {"money",	"amt gold",	"unused",	"unused",		"unused"},	/* 20*/
  {"undefined",	"unused",       "unused",       "unused",               "undefined"},	/* 21*/
  {"boat",	"unused",	"unused",	"unused",		"unused"},	/* 22*/
  {"npc corpse","unused",	"unused",	"unused",		"unused"},	/* 23*/
  {"pc corpse",	"unused",	"unused",	"unused",		"unused"},	/* 24*/
  {"fountain",	"unused",	"unused",	"unused",		"unused"},	/* 25*/
  {"pill",	"level",	"sn1",		"sn2",			"sn3"},	/* 26*/
  {"blood",	"?",		"?",		"?",			"?"},	/* 26*/
  {"bloodstain","?",		"?",		"?",			"?"},	/* 26*/
  {"scraps",	"?",		"?",		"?",			"?"},	/* 26*/
  {"pipe",	"?",		"?",		"?",			"?"},	/* 30*/
  {"herb con",	"?",		"?",		"?",			"?"},	/* 31*/
  {"herb",	"?",		"?",		"?",			"?"},	/* 32*/
  {"incense",	"?",		"?",		"?",			"?"},	/* 33*/
  {"fire",	"?",		"?",		"?",			"?"},	/* 34*/
  {"book",	"?",		"?",		"?",			"?"},	/* 35*/
  {"switch",	"?",		"?",		"?",			"?"},	/* 36*/
  {"lever",	"?",		"?",		"?",			"?"},	/* 37*/
  {"pullchain",	"?",		"?",		"?",			"?"},	/* 38*/
  {"button",	"?",		"?",		"?",			"?"},	/* 39*/
  {"dial",	"?",		"?",		"?",			"?"},	/* 40*/
  {"rune",	"?",		"?",		"?",			"?"},	/* 41*/
  {"runepouch",	"?",		"?",		"?",			"?"},	/* 42*/
  {"match",	"?",		"?",		"?",			"?"},	/* 43*/
  {"trap",	"charges",	"type",		"level",		"flags"},/*44*/
  {"map",	"?",		"?",		"?",			"?"}	/* 45*/
};

char * get_item_strings( int item_number, int which_string )
{

  if(  (which_string<0) || (which_string>4))
    return("get_item_strings bug");

  if(  (which_string<0) || (which_string>45))
    return("get_item_strings bug");

  return ( char *) item_lookup[item_number][which_string];
}

