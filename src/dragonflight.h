
/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * Win32 port by Nick Gammon                                                *
 * ------------------------------------------------------------------------ *
 *         Dragonflight Module for use with Samson's Overland Module        *
 *                       Created by Ymris of Terahoun                       *
 *			         Dragonflight header file		          	    *
 ****************************************************************************/

#define DRAGONFLIGHT /* Module define since stock Overland won't have this */

#define LANDING_SITE_FILE "landing_sites.dat"

#define PULSE_DFLIGHT PULSE_MOBILE

/*
 * Do Functions
 */
DECLARE_DO_FUN( do_call );
DECLARE_DO_FUN( do_release );
DECLARE_DO_FUN( do_land );
DECLARE_DO_FUN( do_fly );
DECLARE_DO_FUN( do_landing_sites );
DECLARE_DO_FUN( do_setlanding );

/*
 * Prototypes for dragonflight functions
 */
void dflight_update( void );

typedef struct landing_data LANDING_DATA;

extern	LANDING_DATA	*	first_landing;
extern	LANDING_DATA	*	last_landing;

struct landing_data
{
  LANDING_DATA * next;
  LANDING_DATA * prev;
  char *area;
  int	map; 
  int	x;
  int	y;
};
