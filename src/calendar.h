/****************************************************************************
 *                   ^     +----- |  / ^     ^ |     | +-\                  *
 *                  / \    |      | /  |\   /| |     | |  \                 *
 *                 /   \   +---   |<   | \ / | |     | |  |                 *
 *                /-----\  |      | \  |  v  | |     | |  /                 *
 *               /       \ |      |  \ |     | +-----+ +-/                  *
 ****************************************************************************
 * AFKMud Copyright 1997-2007 by Roger Libiez (Samson),                     *
 * Levi Beckerson (Whir), Michael Ward (Tarl), Erik Wolfe (Dwip),           *
 * Cameron Carroll (Cam), Cyberfox, Karangi, Rathian, Raine,                *
 * Xorith, and Adjani.                                                      *
 * All Rights Reserved.                                                     *
 *                                                                          *
 * External contributions from Remcon, Quixadhal, Zarius, and many others.  *
 *                                                                          *
 * Original SMAUG 1.8b written by Thoric (Derek Snider) with Altrag,        *
 * Blodkai, Haus, Narn, Scryn, Swordbearer, Tricops, Gorog, Rennard,        *
 * Grishnakh, Fireblade, Edmond, Conran, and Nivek.                         *
 *                                                                          *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                      *
 *                                                                          *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen,      *
 * Michael Seifert, and Sebastian Hammer.                                   *
 ****************************************************************************
 *                      Calendar Handler/Seasonal Updates                   *
 ****************************************************************************/

/* Well, ok, so it didn't turn out the way I wanted, but that's life - Samson */
/* Ever write a comment like the one above this one and completely forget what it means? */
/* Portions of this data courtesy of Yrth mud */

/* PaB: Seasons */
/* Notes: Each season will be arbitrarily set at 1/4 of the year.
 */
#define SEASON_SPRING		0
#define SEASON_SUMMER		1
#define SEASON_FALL		    2
#define SEASON_WINTER		3
#define SEASON_MAX         	4

/* Hunger/Thirst modifiers */
#define WINTER_HUNGER 1
#define SUMMER_THIRST 1
#define SUMMER_THIRST_DESERT 2

typedef struct holiday_data HOLIDAY_DATA;

extern HOLIDAY_DATA *first_holiday;
extern HOLIDAY_DATA *last_holiday;

extern const char *const day_name[];
extern const char *const month_name[];
extern const char *const season_name[];
extern bool winter_freeze;

struct holiday_data
{
   HOLIDAY_DATA *next;
   HOLIDAY_DATA *prev;
   short month;  /* Month the holiday falls in */
   short day; /* Day the holiday falls on */
   char *name; /* Name of the holiday */
   char *announce;   /* Message to announce the holiday with */
};


void calc_season( void );
char *mini_c_time( time_t curtime, int tz );
char *c_time( time_t curtime, int tz );
bool load_timedata( void );
void load_holidays( void );
void save_timedata( void );
void update_timers( void );
void update_calendar( void );

DECLARE_DO_FUN( do_timezone );
DECLARE_DO_FUN( do_holidays );
DECLARE_DO_FUN( do_saveholiday );
DECLARE_DO_FUN( do_setholiday );
