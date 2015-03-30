/*****************************************************
**     _________       __			    **
**     \_   ___ \_____|__| _____  ________  ___     **
**	/    \	\/_  __ \ |/	 \/  ___/_ \/	\   **
**	\     \___|  | \/ |  | |  \___ \  / ) |  \  **
**	 \______  /__| |__|__|_|  /____ \__/__|  /  **
**	   ____\/____ _        \/ ___ \/      \/    **
**	   \______   \ |_____  __| _/___	    **
**	    |	 |  _/ |\__  \/ __ | __ \	    **
**	    |	 |   \ |_/ __ \  / | ___/_	    **
**	    |_____  /__/____  /_  /___	/	    **
**		 \/Antipode\/  \/    \/ 	    **
******************************************************
**	   Crimson Blade Codebase (CbC) 	    **
**     (c) 2000-2002 John Bellone (Noplex)	    **
**	     Coders: Noplex, Krowe		    **
**	  http://www.crimsonblade.org		    **
******************************************************
** Based on SMAUG 1.4a, by; Thoric, Altrag, Blodkai **
**  Narn, Haus, Scryn, Rennard, Swordbearer, Gorog  **
**    Grishnakh, Nivek, Tricops, and Fireblade	    **
******************************************************
** Merc 2.1 by; Michael Chastain, Michael Quan, and **
**		    Mitchell Tse		    **
******************************************************
**   DikuMUD by; Sebastian Hammer, Michael Seifert, **
**     Hans Staerfeldt, Tom Madsen and Katja Nyobe  **
*****************************************************/

/*
 * Liquidtable Replacement Headerfile
 * by Noplex (noplex@crimsonblade.org)
 */

#define LIQUIDSYSTEM

#define TOP_MOD_NUM 4
#define MAX_COND_VAL 100

typedef enum
{
   LIQ_NORMAL, LIQ_ALCOHOL, LIQ_POISON, LIQ_UNUSED, LIQ_TOP
} liquids;

extern char *const liquid_types[];

typedef struct		liquid_data		LIQUID_DATA;
typedef struct		liquid_mixture_list	LIQUID_MIXTURE_LIST;

/*
 * Structures for liquids and mixtures
 */
struct liquid_mixture_list
{
	LIQUID_MIXTURE_LIST	*next;
	LIQUID_MIXTURE_LIST	*prev;
	char		*name;
	int		with1; /* obj vnum if there is a obj, liq vnum if liquid */
	int		with2;
	int		into;
	bool		object; /* false = no obj, true = obj */
};

struct liquid_data 
{
	LIQUID_DATA	*next;
	LIQUID_DATA	*prev;
/*	EXT_BV		affected_by; */
	char		*name;
	char		*shortdesc;
	char		*color;
	int		vnum;
	int		type;
	sh_int		mod[4];
};

LIQUID_DATA *first_liquid;
LIQUID_DATA *last_liquid;
LIQUID_MIXTURE_LIST *first_mixture;
LIQUID_MIXTURE_LIST *last_mixture;

/* OLC */
DECLARE_DO_FUN( do_setliquid );
DECLARE_DO_FUN( do_setmixture );
DECLARE_DO_FUN( do_liquids );

/* funcs */
DECLARE_DO_FUN( do_mix );

/* saving/loading */
void load_liquids( void );
void save_liquids( void );
void load_mixtures( void );
void save_mixtures( void );

/* lookup functions */
LIQUID_DATA *get_liq_name( char *argument );
LIQUID_DATA *check_liquid( char *argument );
LIQUID_DATA *get_liq_index( int vnum );
LIQUID_MIXTURE_LIST *get_mix_name( char *argument );
