/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.0 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
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
 * 			Database management module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#ifndef WIN32
  #include <dirent.h>
#else
  #define strcasecmp strcmp
#endif
#include "mud.h"


extern	int	_filbuf		args( (FILE *) );

#if defined(KEY)
#undef KEY
#endif

void init_supermob();

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}


/*
 * Globals.
 */

WIZENT *	first_wiz;
WIZENT *	last_wiz;

time_t                  last_restore_all_time = 0;

HELP_DATA *		first_help;
HELP_DATA *		last_help;

LMSG_DATA	*		first_lmsg;
LMSG_DATA	*		last_lmsg;

#ifdef MYSTARIC
CASINO_DATA *           first_casino;
CASINO_DATA *           last_casino;
#endif

SHOP_DATA *		first_shop;
SHOP_DATA *		last_shop;

REPAIR_DATA *		first_repair;
REPAIR_DATA *		last_repair;

TELEPORT_DATA *		first_teleport;
TELEPORT_DATA *		last_teleport;

PROJECT_DATA *		first_project;
PROJECT_DATA *		last_project;

OBJ_DATA *		extracted_obj_queue;
EXTRACT_CHAR_DATA *	extracted_char_queue;

char			bug_buf		[2*MAX_INPUT_LENGTH];
CHAR_DATA *		first_char;
CHAR_DATA *		last_char;
char *			help_greeting;
char			log_buf		[2*MAX_INPUT_LENGTH];

OBJ_DATA *		first_object;
OBJ_DATA *		last_object;
TIME_INFO_DATA		time_info;
WEATHER_DATA		weather_info;

int			weath_unit;	/* global weather param */
int			rand_factor;
int			climate_factor;
int			neigh_factor;
int			max_vector;

int			cur_qobjs;
int			cur_qchars;
int			nummobsloaded;
int			numobjsloaded;
int			physicalobjects;
int 			last_pkroom;

MAP_INDEX_DATA  *       first_map;	/* maps */

AUCTION_DATA    * 	auction;	/* auctions */

FILE		*	fpLOG;

/* weaponry */
sh_int			gsn_pugilism;
sh_int			gsn_long_blades;
sh_int			gsn_short_blades;
sh_int			gsn_flexible_arms;
sh_int			gsn_talonous_arms;
sh_int			gsn_bludgeons;
sh_int			gsn_missile_weapons;

/* thief */
sh_int			gsn_detrap;
sh_int          	gsn_backstab;
sh_int			gsn_circle;
sh_int			gsn_dodge;
sh_int			gsn_hide;
sh_int			gsn_peek;
sh_int			gsn_pick_lock;
sh_int			gsn_sneak;
sh_int			gsn_steal;
sh_int			gsn_gouge;
sh_int			gsn_poison_weapon;

/* thief & warrior */
sh_int          	gsn_disarm;
sh_int			gsn_enhanced_damage;
sh_int			gsn_kick;
sh_int			gsn_parry;
sh_int			gsn_rescue;
sh_int			gsn_second_attack;
sh_int			gsn_third_attack;
sh_int          	gsn_fourth_attack;
sh_int         		gsn_fifth_attack;
sh_int			gsn_dual_wield;
sh_int			gsn_punch;
sh_int			gsn_bash;
sh_int			gsn_stun;
sh_int                  gsn_bashdoor;
sh_int			gsn_grip; 
sh_int			gsn_berserk;
sh_int			gsn_hitall;
sh_int			gsn_tumble;

/* vampire */
sh_int          	gsn_feed;
sh_int			gsn_bloodlet;
sh_int			gsn_broach;
sh_int			gsn_mistwalk;
sh_int			gsn_pounce;

/* other   */
sh_int			gsn_aid;
sh_int			gsn_track;
sh_int			gsn_search;
sh_int			gsn_dig;
sh_int			gsn_mount;
sh_int			gsn_bite;
sh_int			gsn_claw;
sh_int			gsn_sting;
sh_int			gsn_tail;
sh_int			gsn_scribe;
sh_int			gsn_brew;
sh_int			gsn_climb;
sh_int			gsn_cook;
sh_int                  gsn_scan;
sh_int			gsn_slice;
sh_int			gsn_grapple;
sh_int			gsn_cleave;
sh_int			gsn_meditate;
sh_int			gsn_trance;
/* spells */
sh_int			gsn_aqua_breath;
sh_int          	gsn_blindness;
sh_int			gsn_charm_person;
sh_int			gsn_curse;
sh_int			gsn_invis;
sh_int			gsn_mass_invis;
sh_int			gsn_poison;
sh_int			gsn_sleep;
sh_int			gsn_possess;
sh_int			gsn_fireball;
sh_int			gsn_chill_touch;
sh_int			gsn_lightning_bolt;

/* languages */
sh_int			gsn_common;
sh_int			gsn_elven;
sh_int			gsn_dwarven;
sh_int			gsn_pixie;
sh_int			gsn_ogre;
sh_int			gsn_orcish;
sh_int			gsn_trollish;
sh_int			gsn_goblin;
sh_int			gsn_halfling;
sh_int			gsn_gnomish;

/* for searching */
sh_int			gsn_first_spell;
sh_int			gsn_first_skill;
sh_int			gsn_first_weapon;
sh_int			gsn_first_tongue;
sh_int			gsn_top_sn;

/* For styles?  Trying to rebuild from some kind of accident here - Blod */
sh_int			gsn_style_evasive;
sh_int			gsn_style_defensive;
sh_int			gsn_style_standard;
sh_int			gsn_style_aggressive;
sh_int			gsn_style_berserk;

/*
 * Locals.
 */
MOB_INDEX_DATA *	mob_index_hash		[MAX_KEY_HASH];
OBJ_INDEX_DATA *	obj_index_hash		[MAX_KEY_HASH];
ROOM_INDEX_DATA *	room_index_hash		[MAX_KEY_HASH];

AREA_DATA *		first_area;
AREA_DATA *		last_area;
AREA_DATA *             first_area_name;        /*Used for alphanum. sort*/
AREA_DATA *             last_area_name;
AREA_DATA *		first_build;
AREA_DATA *		last_build;
AREA_DATA *		first_asort;
AREA_DATA *		last_asort;
AREA_DATA *		first_bsort;
AREA_DATA *		last_bsort;

SYSTEM_DATA		sysdata;

int			top_affect;
int			top_area;
int			top_ed;
int			top_exit;
int			top_help;
int			top_mob_index;
int			top_obj_index;
int			top_reset;
int			top_room;
int			top_shop;
int			top_repair;
int			top_vroom;

/*
 * Semi-locals.
 */
bool			fBootDb;
FILE *			fpArea;
char			strArea[MAX_INPUT_LENGTH];



/*
 * Local booting procedures.
 */
void	init_mm		args( ( void ) );

void	boot_log	args( ( const char *str, ... ) );
void	load_area	args( ( FILE *fp ) );
void    load_author     args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_credits	args( ( AREA_DATA *tarea, FILE *fp ) );
void    load_economy    args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_resetmsg	args( ( AREA_DATA *tarea, FILE *fp ) ); /* Rennard */
void    load_flags      args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_helps	args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_mobiles	args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_objects	args( ( AREA_DATA *tarea, FILE *fp ) );
void 	load_projects   args( ( void ) );
void	load_resets	args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_rooms	args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_shops	args( ( AREA_DATA *tarea, FILE *fp ) );
void 	load_repairs	args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_specials	args( ( AREA_DATA *tarea, FILE *fp ) );
void    load_ranges	args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_climate	args( ( AREA_DATA *tarea, FILE *fp ) ); /* FB */
void	load_neighbor	args( ( AREA_DATA *tarea, FILE *fp ) );
void	load_buildlist	args( ( void ) );
bool	load_systemdata	args( ( SYSTEM_DATA *sys ) );
void    load_banlist    args( ( void ) );
void    load_version     args( ( AREA_DATA *tarea, FILE *fp ) );
void    load_watchlist  args( ( void ) );
void    load_reserved   args( ( void ) );
void	load_noauctions	args( ( void ) );
void 	sort_noauctions args( ( NOAUCTION_DATA *pNoauc ) );
void	load_loginmsg	args( ( void ) );
void	initialize_economy args( ( void ) );
void	fix_exits	args( ( void ) );
void    sort_reserved   args( ( RESERVE_DATA *pRes ) );
void	init_area_weather args( ( void ) );
void	load_weatherdata args( ( void ) );
PROJECT_DATA *read_project args( ( char *filename, FILE *fp ) );
NOTE_DATA *read_log  args( ( FILE *fp ) );
/*
 * External booting function
 */
void	load_corpses	args( ( void ) );
void	renumber_put_resets	args( ( AREA_DATA *pArea ) );
void	load_colors	args( ( void ) );

/*
 * MUDprogram locals
 */

int 		mprog_name_to_type	args ( ( char* name ) );
MPROG_DATA *	mprog_file_read 	args ( ( char* f, MPROG_DATA* mprg,
						MOB_INDEX_DATA *pMobIndex ) );
/* int 		oprog_name_to_type	args ( ( char* name ) ); */
MPROG_DATA *	oprog_file_read 	args ( ( char* f, MPROG_DATA* mprg,
						OBJ_INDEX_DATA *pObjIndex ) );
/* int 		rprog_name_to_type	args ( ( char* name ) ); */
MPROG_DATA *	rprog_file_read 	args ( ( char* f, MPROG_DATA* mprg,
						ROOM_INDEX_DATA *pRoomIndex ) );
void		load_mudprogs           args ( ( AREA_DATA *tarea, FILE* fp ) );
void		load_objprogs           args ( ( AREA_DATA *tarea, FILE* fp ) );
void		load_roomprogs          args ( ( AREA_DATA *tarea, FILE* fp ) );
void   		mprog_read_programs     args ( ( FILE* fp,
						MOB_INDEX_DATA *pMobIndex) );
void   		oprog_read_programs     args ( ( FILE* fp,
						OBJ_INDEX_DATA *pObjIndex) );
void   		rprog_read_programs     args ( ( FILE* fp,
						ROOM_INDEX_DATA *pRoomIndex) );


void shutdown_mud( char *reason )
{
    FILE *fp;

    if ( (fp = fopen( SHUTDOWN_FILE, "a" )) != NULL )
    {
	fprintf( fp, "%s\n", reason );
	fclose( fp );
    }
}


/*
 * Big mama top level function.
 */
void boot_db( void )
{
    sh_int wear, x;

    fpArea = NULL;
    show_hash( 32 );
    unlink( BOOTLOG_FILE );
    boot_log( "---------------------[ Boot Log ]--------------------" );

    log_string( "Loading commands" );
    load_commands();

    log_string( "Loading sysdata configuration..." );

    /* default values */
    sysdata.read_all_mail		= LEVEL_DEMI;
    sysdata.read_mail_free 		= LEVEL_IMMORTAL;
    sysdata.write_mail_free 		= LEVEL_IMMORTAL;
    sysdata.take_others_mail		= LEVEL_DEMI;
    sysdata.imc_mail_vnum		= 0;
    sysdata.muse_level			= LEVEL_DEMI;
    sysdata.think_level			= LEVEL_HIGOD;
    sysdata.build_level			= LEVEL_DEMI;
    sysdata.log_level			= LEVEL_LOG;
    sysdata.level_modify_proto		= LEVEL_LESSER;
    sysdata.level_override_private	= LEVEL_GREATER;
    sysdata.level_mset_player		= LEVEL_LESSER;
    sysdata.stun_plr_vs_plr		= 65;
    sysdata.stun_regular		= 15;
    sysdata.gouge_nontank               = 0;
    sysdata.gouge_plr_vs_plr            = 0;
    sysdata.bash_nontank                = 0;
    sysdata.bash_plr_vs_plr             = 0;
    sysdata.dodge_mod                   = 2;
    sysdata.parry_mod                   = 2;
    sysdata.tumble_mod                  = 4;
    sysdata.tumble_pk			= 5;
    sysdata.dam_plr_vs_plr		= 100;
    sysdata.dam_plr_vs_mob		= 100;
    sysdata.dam_mob_vs_plr		= 100;
    sysdata.dam_mob_vs_mob		= 100;
    sysdata.dam_nonav_vs_mob		= 100;
    sysdata.dam_mob_vs_nonav		= 100;
    sysdata.peaceful_exp_mod		= 100;
    sysdata.deadly_exp_mod		= 100;
    sysdata.level_getobjnotake 		= LEVEL_GREATER;
    sysdata.save_frequency		= 20;	/* minutes */
    sysdata.bestow_dif			= 5;
    sysdata.check_imm_host 		= 1;
    sysdata.morph_opt 			= 1;
    sysdata.save_pets			= 0;
    sysdata.pk_loot			= 1;
    sysdata.max_html_news		= -1;
    sysdata.news_html_path		= STRALLOC("");
    sysdata.pk_channels			= 1;
    sysdata.pk_silence			= 0;
    sysdata.wizlock             = FALSE;
	sysdata.magichell			= FALSE;
    sysdata.save_flags			= SV_DEATH | SV_PASSCHG | SV_AUTO
    					| SV_PUT | SV_DROP | SV_GIVE
    					| SV_AUCTION | SV_ZAPDROP | SV_IDLE;
    if ( !load_systemdata(&sysdata) )
    {
	log_string( "Not found.  Creating new configuration." );
	sysdata.alltimemax = 0;
	sysdata.mud_name = str_dup("(Name not set)");
	sysdata.port_name = str_dup("mud");
    }

    log_string("Loading socials");
    load_socials();

    log_string("Loading skill table");
    load_skill_table();
    sort_skill_table();
    remap_slot_numbers();	/* must be after the sort */

    gsn_first_spell  = 0;
    gsn_first_skill  = 0;
    gsn_first_weapon = 0;
    gsn_first_tongue = 0;
    gsn_top_sn	     = top_sn;

    for ( x = 0; x < top_sn; x++ )
	if ( !gsn_first_spell && skill_table[x]->type == SKILL_SPELL )
	    gsn_first_spell = x;
	else
	if ( !gsn_first_skill && skill_table[x]->type == SKILL_SKILL )
	    gsn_first_skill = x;
	else
	if ( !gsn_first_weapon && skill_table[x]->type == SKILL_WEAPON )
	    gsn_first_weapon = x;
	else
	if ( !gsn_first_tongue && skill_table[x]->type == SKILL_TONGUE )
	    gsn_first_tongue = x;


    log_string("Loading classes");
    load_classes();

    log_string("Loading races");
    load_races();

    /* Extended News - 12/15/01 - Nopey */
    log_string("Loading extended news data");
    load_news();

    log_string ("Loading stance data");
    load_stances ();

    log_string("Loading herb table");
    load_herb_table();

    log_string("Loading tongues");
    load_tongues();

    log_string("Making wizlist");
    make_wizlist();

/*    log_string("Making adminlist");
    make_adminlist();
*/
    log_string("Making retiredlist");
    make_retiredlist();

    log_string("Initializing request pipe");
    init_request_pipe();

    fBootDb		= TRUE;

    nummobsloaded	= 0;
    numobjsloaded	= 0;
    physicalobjects	= 0;
    sysdata.maxplayers	= 0;
    first_object	= NULL;
    last_object		= NULL;
    first_char		= NULL;
    last_char		= NULL;
    first_area		= NULL;
    first_area_name     = NULL;         /*Used for alphanum. sort*/
    last_area_name      = NULL;
    last_area		= NULL;
    first_build		= NULL;
    last_area		= NULL;
    first_shop		= NULL;
    last_shop		= NULL;
    first_repair	= NULL;
    last_repair		= NULL;
    first_teleport	= NULL;
    last_teleport	= NULL;
    first_asort		= NULL;
    last_asort		= NULL;
    extracted_obj_queue	= NULL;
    extracted_char_queue= NULL;
    cur_qobjs		= 0;
    cur_qchars		= 0;
    cur_char		= NULL;
    cur_obj		= 0;
    cur_obj_serial	= 0;
    cur_char_died	= FALSE;
    cur_obj_extracted	= FALSE;
    cur_room		= NULL;
    quitting_char	= NULL;
    loading_char	= NULL;
    saving_char		= NULL;
    last_pkroom		= 1;
    immortal_host_start = NULL;
    immortal_host_end = NULL;
    first_ban_class = NULL;
    last_ban_class = NULL;
    first_ban_race = NULL;
    last_ban_race = NULL;
    first_ban = NULL;
    last_ban = NULL;
    
    CREATE( auction, AUCTION_DATA, 1);
    auction->item 	= NULL;
    auction->hist_timer = 0;
    for(x = 0; x < AUCTION_MEM; x++)
    	auction->history[x] = NULL;
    
    weath_unit		= 10;
    rand_factor		= 2;
    climate_factor	= 1;
    neigh_factor	= 3;
    max_vector		= weath_unit*3;
    
    for ( wear = 0; wear < MAX_WEAR; wear++ )
	for ( x = 0; x < MAX_LAYERS; x++ )
	    save_equipment[wear][x] = NULL;



    /*
     * Init random number generator.
     */
    log_string("Initializing random number generator");
    init_mm( );

    /*
     * Set time and weather.
     */
    {
	long lhour, lday, lmonth;

	log_string("Setting time and weather");

	lhour		= (current_time - 650336715)
			/ (PULSE_TICK / PULSE_PER_SECOND);
	time_info.hour	= lhour  % 24;
	lday		= lhour  / 24;
	time_info.day	= lday   % 35;
	lmonth		= lday   / 35;
	time_info.month	= lmonth % 17;
	time_info.year	= lmonth / 17;

	     if ( time_info.hour <  5 ) time_info.sunlight = SUN_DARK;
	else if ( time_info.hour <  6 ) time_info.sunlight = SUN_RISE;
	else if ( time_info.hour < 19 ) time_info.sunlight = SUN_LIGHT;
	else if ( time_info.hour < 20 ) time_info.sunlight = SUN_SET;
	else                            time_info.sunlight = SUN_DARK;

	/*
	weather_info.change	= 0;
	weather_info.mmhg	= 960;
	if ( time_info.month >= 7 && time_info.month <=12 )
	    weather_info.mmhg += number_range( 1, 50 );
	else
	    weather_info.mmhg += number_range( 1, 80 );

	     if ( weather_info.mmhg <=  980 ) weather_info.sky = SKY_LIGHTNING;
	else if ( weather_info.mmhg <= 1000 ) weather_info.sky = SKY_RAINING;
	else if ( weather_info.mmhg <= 1020 ) weather_info.sky = SKY_CLOUDY;
	else                                  weather_info.sky = SKY_CLOUDLESS;
	*/
    }

    /*
     * Assign gsn's for skills which need them.
     */
    {
	log_string("Assigning gsn's");
	ASSIGN_GSN( gsn_style_evasive,		"evasive style" );
	ASSIGN_GSN( gsn_style_defensive,	"defensive style" );
	ASSIGN_GSN( gsn_style_standard,		"standard style" );
	ASSIGN_GSN( gsn_style_aggressive,	"aggressive style" );
	ASSIGN_GSN( gsn_style_berserk,		"berserk style" );

	ASSIGN_GSN( gsn_pugilism,	"pugilism" );
	ASSIGN_GSN( gsn_long_blades,	"long blades" );
	ASSIGN_GSN( gsn_short_blades,	"short blades" );
	ASSIGN_GSN( gsn_flexible_arms,	"flexible arms" );
	ASSIGN_GSN( gsn_talonous_arms,	"talonous arms" );
	ASSIGN_GSN( gsn_bludgeons,	"bludgeons" );
	ASSIGN_GSN( gsn_missile_weapons,"missile weapons" ); 
	ASSIGN_GSN( gsn_detrap,		"detrap" );
	ASSIGN_GSN( gsn_backstab,	"backstab" );
	ASSIGN_GSN( gsn_circle,		"circle" );
	ASSIGN_GSN( gsn_tumble,		"tumble" );
	ASSIGN_GSN( gsn_dodge,		"dodge" );
	ASSIGN_GSN( gsn_hide,		"hide" );
	ASSIGN_GSN( gsn_peek,		"peek" );
	ASSIGN_GSN( gsn_pick_lock,	"pick lock" );
	ASSIGN_GSN( gsn_sneak,		"sneak" );
	ASSIGN_GSN( gsn_steal,		"steal" );
	ASSIGN_GSN( gsn_gouge,		"gouge" );
	ASSIGN_GSN( gsn_poison_weapon, 	"poison weapon" );
	ASSIGN_GSN( gsn_disarm,		"disarm" );
	ASSIGN_GSN( gsn_enhanced_damage, "enhanced damage" );
	ASSIGN_GSN( gsn_kick,		"kick" );
	ASSIGN_GSN( gsn_parry,		"parry" );
	ASSIGN_GSN( gsn_rescue,		"rescue" );
	ASSIGN_GSN( gsn_second_attack, 	"second attack" );
	ASSIGN_GSN( gsn_third_attack, 	"third attack" );
	ASSIGN_GSN( gsn_fourth_attack, 	"fourth attack" );
	ASSIGN_GSN( gsn_fifth_attack, 	"fifth attack" );
	ASSIGN_GSN( gsn_dual_wield,	"dual wield" );
	ASSIGN_GSN( gsn_punch,		"punch" );
	ASSIGN_GSN( gsn_bash,		"bash" );
	ASSIGN_GSN( gsn_stun,		"stun" );
	ASSIGN_GSN( gsn_bashdoor,	"doorbash" );
	ASSIGN_GSN( gsn_grip,		"grip" ); 
	ASSIGN_GSN( gsn_berserk,	"berserk" );
	ASSIGN_GSN( gsn_hitall,		"hitall" );
	ASSIGN_GSN( gsn_feed,		"feed" );
	ASSIGN_GSN( gsn_bloodlet,       "bloodlet" );
	ASSIGN_GSN( gsn_cleave,		"cleave" );
	ASSIGN_GSN( gsn_pounce,		"pounce" );
	ASSIGN_GSN( gsn_grapple,	"grapple" );
	ASSIGN_GSN( gsn_broach,         "broach" );
	ASSIGN_GSN( gsn_mistwalk,	"mistwalk" );
	ASSIGN_GSN( gsn_aid,		"aid" );
	ASSIGN_GSN( gsn_track,		"track" );
	ASSIGN_GSN( gsn_meditate,	"meditate" );
	ASSIGN_GSN( gsn_trance,		"trance"	);
	ASSIGN_GSN( gsn_search,		"search" );
	ASSIGN_GSN( gsn_dig,		"dig" );
	ASSIGN_GSN( gsn_mount,		"mount" );
	ASSIGN_GSN( gsn_bite,		"bite" );
	ASSIGN_GSN( gsn_claw,		"claw" );
	ASSIGN_GSN( gsn_sting,		"sting" );
	ASSIGN_GSN( gsn_tail,		"tail" );
	ASSIGN_GSN( gsn_scribe,		"scribe" );
	ASSIGN_GSN( gsn_brew,		"brew" );
	ASSIGN_GSN( gsn_climb,		"climb" );
	ASSIGN_GSN( gsn_cook,		"cook" );
	ASSIGN_GSN( gsn_scan,		"scan" );
	ASSIGN_GSN( gsn_slice,		"slice" );
	ASSIGN_GSN( gsn_fireball,	"fireball" );
	ASSIGN_GSN( gsn_chill_touch,	"chill touch" );
	ASSIGN_GSN( gsn_lightning_bolt,	"lightning bolt" );
	ASSIGN_GSN( gsn_aqua_breath,	"aqua breath" );
	ASSIGN_GSN( gsn_blindness,	"blindness" );
	ASSIGN_GSN( gsn_charm_person, 	"charm person" );
	ASSIGN_GSN( gsn_curse,		"curse" );
	ASSIGN_GSN( gsn_invis,		"invis" );
	ASSIGN_GSN( gsn_mass_invis,	"mass invis" );
	ASSIGN_GSN( gsn_poison,		"poison" );
	ASSIGN_GSN( gsn_sleep,		"sleep" );
	ASSIGN_GSN( gsn_possess,	"possess" );
	ASSIGN_GSN( gsn_common,		"common" );
	ASSIGN_GSN( gsn_elven,		"elven" );
	ASSIGN_GSN( gsn_dwarven,	"dwarven" );
	ASSIGN_GSN( gsn_pixie,		"pixie" );
	ASSIGN_GSN( gsn_ogre,		"ogre" );
	ASSIGN_GSN( gsn_orcish,		"orcish" );
	ASSIGN_GSN( gsn_trollish,	"trollese" );
	ASSIGN_GSN( gsn_goblin,		"goblin" );
	ASSIGN_GSN( gsn_halfling,	"halfling" );
	ASSIGN_GSN( gsn_gnomish,	"gnomish" );
    }
    
#ifdef PLANES
    log_string("Reading in plane file...");
    load_planes();
#endif

    /*
     * Read in all the area files.
     */
    {
	FILE *fpList;

	log_string("Reading in area files...");
	if ( ( fpList = fopen( AREA_LIST, "r" ) ) == NULL )
	{
	    perror( AREA_LIST );
	    shutdown_mud( "Unable to open area list" );
	    exit( 1 );
	}

	for ( ; ; )
	{
	    strcpy( strArea, fread_word( fpList ) );
	    if ( strArea[0] == '$' )
		break;

	    load_area_file( last_area, strArea );
	    
	}
	fclose( fpList );
    }
    
#ifdef PLANES
    log_string("Making sure rooms are planed...");
    check_planes(NULL);
#endif

   /*
    *   initialize supermob.
    *    must be done before reset_area!
    *
    */
    init_supermob();
    /*
     * Has some bad memory bugs in it
     */
#ifdef PROFANITY_CHECK
    init_profanity_checker();    /* as good a place as any */
#endif

    /*
     * Fix up exits.
     * Declare db booting over.
     * Reset all areas once.
     * Load up the notes file.
     */
    {
	log_string( "Fixing exits" );
	fix_exits( );
	fBootDb	= FALSE;
	log_string( "Initializing economy" );
	initialize_economy( );
	log_string ("Randomizing stance data");
	randomize_stances ();
	log_string( "Resetting areas" );
	area_update( );
	log_string( "Loading buildlist" );
	load_buildlist( );
	log_string( "Loading boards" );
	load_boards( );
	log_string( "Loading vault list" );
	load_vaults( );
	log_string( "Loading clans" );
	load_clans( );
	log_string( "Loading member lists" );
	load_member_lists( );
	log_string( "Loading councils" );
	load_deity( );
	log_string( "Loading deities" );
	load_councils( );
        log_string( "Loading watches" );
        load_watchlist( );
        log_string( "Loading bans" );
        load_banlist( );
        log_string( "Loading reserved names" );
        load_reserved( );
	log_string( "Loading noauction vnums" );
	load_noauctions( );
        log_string( "Loading corpses" );
        load_corpses( );
        log_string ("Loading Immortal Hosts");
    	load_imm_host();
	log_string ("Loading Hints");
	load_hint();
	log_string ("Loading Projects");
	load_projects( );
/* Morphs MUST be loaded after class and race tables are set up --Shaddai */
        log_string ("Loading Morphs");
        load_morphs( );
        log_string("Loading Housing System, Home Accessories Data,"
                       " and Home Auctioning System");
        load_homedata();
        load_accessories();
        load_homebuy();
        log_string("Loading login messages");
        load_loginmsg();
        log_string ("Loading Colors");
        load_colors( );
        MOBtrigger = TRUE;
    }

    /* Initialize area weather data */
    load_weatherdata();
    init_area_weather();

    /* init_maps ( ); */

    return;
}



/*
 * Load an 'area' header line.
 */
void load_area( FILE *fp )
{
    AREA_DATA *pArea;

    CREATE( pArea, AREA_DATA, 1 );
    pArea->first_reset	= NULL;
    pArea->last_reset	= NULL;
    pArea->name		= fread_string_nohash( fp );
    pArea->author       = STRALLOC( "unknown" );
	pArea->credits		= STRALLOC( "" );
    pArea->filename	= str_dup( strArea );
    pArea->age		= 15;
    pArea->nplayer	= 0;
    pArea->low_r_vnum	= 0;
    pArea->low_o_vnum	= 0;
    pArea->low_m_vnum	= 0;
    pArea->hi_r_vnum	= 0;
    pArea->hi_o_vnum	= 0;
    pArea->hi_m_vnum	= 0;
    pArea->low_soft_range = 0;
    pArea->hi_soft_range  = MAX_LEVEL;
    pArea->low_hard_range = 0;
    pArea->hi_hard_range  = MAX_LEVEL;
    pArea->spelllimit	= 0;

    /* initialize weather data - FB */
    CREATE(pArea->weather, WEATHER_DATA, 1);
    pArea->weather->temp = 0;
    pArea->weather->precip = 0;
    pArea->weather->wind = 0;
    pArea->weather->temp_vector = 0;
    pArea->weather->precip_vector = 0;
    pArea->weather->wind_vector = 0;
    pArea->weather->climate_temp = 2;
    pArea->weather->climate_precip = 2;
    pArea->weather->climate_wind = 2;
    pArea->weather->first_neighbor = NULL;
    pArea->weather->last_neighbor = NULL;
    pArea->weather->echo = NULL;
    pArea->weather->echo_color = AT_GREY;
    area_version = 0;
    LINK( pArea, first_area, last_area, next, prev );
    top_area++;
    return;
}


/* Load the version number of the area file if none exists, then it
 * is set to version 0 when #AREA is read in which is why we check for
 * the #AREA here.  --Shaddai
 */

void load_version( AREA_DATA *tarea, FILE *fp )
{
    if ( !tarea )
    {
	bug( "Load_author: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }

    area_version   = fread_number( fp );
    return;
}

/*
 * Load an author section. Scryn 2/1/96
 */
void load_author( AREA_DATA *tarea, FILE *fp )
{
    if ( !tarea )
    {
	bug( "Load_author: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }

    if ( tarea->author )
      STRFREE( tarea->author );
    tarea->author   = fread_string( fp );
    return;
}

/*
 * Load a credits section. Edmond
 */
void load_credits( AREA_DATA *tarea, FILE *fp )
{
    if ( !tarea )
    {
      bug( "Load_credits: no #AREA seen yet." );
      if ( fBootDb )
      {
        shutdown_mud( "No #AREA" );
        exit( 1 );
      }
      else
        return;
    }

    if ( tarea->credits )
      STRFREE( tarea->credits );

    tarea->credits   = fread_string( fp );
    return;
}


/*
 * Load an economy section. Thoric
 */
void load_economy( AREA_DATA *tarea, FILE *fp )
{
    if ( !tarea )
    {
	bug( "Load_economy: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }

    tarea->high_economy	= fread_number( fp );
    tarea->low_economy	= fread_number( fp );
    return;
}

/* Reset Message Load, Rennard */
void load_resetmsg( AREA_DATA *tarea, FILE *fp )
{
    if ( !tarea )
    {
	bug( "Load_resetmsg: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }
    
    if ( tarea->resetmsg )
	DISPOSE( tarea->resetmsg );
    tarea->resetmsg = fread_string_nohash( fp );
    return;
}

/*
 * Load area flags. Narn, Mar/96 
 */
void load_flags( AREA_DATA *tarea, FILE *fp )
{
    char *ln;
    int x1, x2;

    if ( !tarea )
    {
	bug( "Load_flags: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }
    ln = fread_line( fp );
    x1=x2=0;
    sscanf( ln, "%d %d",
	&x1, &x2 );
    tarea->flags = x1;
    tarea->reset_frequency = x2;
    if ( x2 )
	tarea->age = x2;
    return;
}

/*
 * Adds a help page to the list if it is not a duplicate of an existing page.
 * Page is insert-sorted by keyword.			-Thoric
 * (The reason for sorting is to keep do_hlist looking nice)
 */
void add_help( HELP_DATA *pHelp )
{
    HELP_DATA *tHelp;
    int match;

    for ( tHelp = first_help; tHelp; tHelp = tHelp->next )
	if ( pHelp->level == tHelp->level
	&&   strcmp(pHelp->keyword, tHelp->keyword) == 0 )
	{
	    bug( "add_help: duplicate: %s.  Deleting.", pHelp->keyword );
	    STRFREE( pHelp->text );
	    STRFREE( pHelp->keyword );
	    DISPOSE( pHelp );
	    return;
	}
	else
	if ( (match=strcmp(pHelp->keyword[0]=='\'' ? pHelp->keyword+1 : pHelp->keyword,
			   tHelp->keyword[0]=='\'' ? tHelp->keyword+1 : tHelp->keyword)) < 0
	||   (match == 0 && pHelp->level > tHelp->level) )
	{
	    if ( !tHelp->prev )
		first_help	  = pHelp;
	    else
		tHelp->prev->next = pHelp;
	    pHelp->prev		  = tHelp->prev;
	    pHelp->next		  = tHelp;
	    tHelp->prev		  = pHelp;
	    break;
	}

    if ( !tHelp )
	LINK( pHelp, first_help, last_help, next, prev );

    top_help++;
}

/*
 * Load a help section.
 */
void load_helps( AREA_DATA *tarea, FILE *fp )
{
    HELP_DATA *pHelp;

    for ( ; ; )
    {
	CREATE( pHelp, HELP_DATA, 1 );
	pHelp->level	= fread_number( fp );
	pHelp->keyword	= fread_string( fp );
	if ( pHelp->keyword[0] == '$' )
	{
	    STRFREE( pHelp->keyword );
	    DISPOSE( pHelp );
	    break;
	}
	pHelp->text	= fread_string( fp );
	if ( pHelp->keyword[0] == '\0' )
	{
	    STRFREE( pHelp->text );
	    STRFREE( pHelp->keyword );
	    DISPOSE( pHelp );
	    continue;
	}

	if ( !str_cmp( pHelp->keyword, "greeting" ) )
	    help_greeting = pHelp->text;
	add_help( pHelp );
    }
    return;
}


/*
 * Add a character to the list of all characters		-Thoric
 */
void add_char( CHAR_DATA *ch )
{
    LINK( ch, first_char, last_char, next, prev );
}


/*
 * Load a mob section.
 */
void load_mobiles( AREA_DATA *tarea, FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex;
    char *ln;
    int x1, x2, x3, x4, x5, x6, x7, x8;

    if ( !tarea )
    {
	bug( "Load_mobiles: no #AREA seen yet." );
	if ( fBootDb )
	{
	    shutdown_mud( "No #AREA" );
	    exit( 1 );
	}
	else
	    return;
    }

    for ( ; ; )
    {
	char buf[MAX_STRING_LENGTH];
	int vnum;
	char letter;
	int iHash, i;
	bool oldmob;
	bool tmpBootDb;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_mobiles: # not found." );
	    if ( fBootDb )
	    {
		shutdown_mud( "# not found" );
		exit( 1 );
	    }
	    else
		return;
	}

	vnum				= fread_number( fp );
	if ( vnum == 0 )
	    break;

	tmpBootDb = fBootDb;
	fBootDb = FALSE;
	if ( get_mob_index( vnum ) )
	{
	    if ( tmpBootDb )
	    {
		bug( "Load_mobiles: vnum %d duplicated.", vnum );
		shutdown_mud( "duplicate vnum" );
		exit( 1 );
	    }
	    else
	    {
		pMobIndex = get_mob_index( vnum );
		sprintf( buf, "Cleaning mobile: %d", vnum );
		log_string_plus( buf, LOG_BUILD, sysdata.log_level );
		clean_mob( pMobIndex );
		oldmob = TRUE;
	    }
	}
	else
	{
	  oldmob = FALSE;
	  CREATE( pMobIndex, MOB_INDEX_DATA, 1 );
	}
	fBootDb = tmpBootDb;

	pMobIndex->vnum			= vnum;
	if ( fBootDb )
	{
	    if ( !tarea->low_m_vnum )
		tarea->low_m_vnum	= vnum;
	    if ( vnum > tarea->hi_m_vnum )
		tarea->hi_m_vnum	= vnum;
	}
        for (i = 0; i < MAX_STANCE; i++)  /* Make sure the are reset -Shaddai*/
          pMobIndex->stances[i] = 0;

	pMobIndex->player_name		= fread_string( fp );
	pMobIndex->short_descr		= fread_string( fp );
	pMobIndex->long_descr		= fread_string( fp );
	pMobIndex->description		= fread_string( fp );

	pMobIndex->long_descr[0]	= UPPER(pMobIndex->long_descr[0]);
	pMobIndex->description[0]	= UPPER(pMobIndex->description[0]);

	pMobIndex->act			= fread_bitvector( fp );
	xSET_BIT(pMobIndex->act, ACT_IS_NPC);
	pMobIndex->affected_by		= fread_bitvector( fp );
	pMobIndex->pShop		= NULL;
	pMobIndex->rShop		= NULL;
	pMobIndex->alignment		= fread_number( fp );
	letter				= fread_letter( fp );
	pMobIndex->level		= fread_number( fp );

	pMobIndex->mobthac0		= fread_number( fp );
	pMobIndex->ac			= fread_number( fp );
	pMobIndex->hitnodice		= fread_number( fp );
	/* 'd'		*/		  fread_letter( fp );
	pMobIndex->hitsizedice		= fread_number( fp );
	/* '+'		*/		  fread_letter( fp );
	pMobIndex->hitplus		= fread_number( fp );
	pMobIndex->damnodice		= fread_number( fp );
	/* 'd'		*/		  fread_letter( fp );
	pMobIndex->damsizedice		= fread_number( fp );
	/* '+'		*/		  fread_letter( fp );
	pMobIndex->damplus		= fread_number( fp );
	pMobIndex->gold			= fread_number( fp );
	pMobIndex->exp			= fread_number( fp );

	/* pMobIndex->position		= fread_number( fp ); */
         pMobIndex->position          = fread_number( fp );
         if(pMobIndex->position<100){
            switch(pMobIndex->position){
                default: 
                case 0: 
                case 1: 
                case 2: 
                case 3: 
                case 4: break;
                case 5: pMobIndex->position=6; break;
                case 6: pMobIndex->position=8; break;
                case 7: pMobIndex->position=9; break;
                case 8: pMobIndex->position=12; break;
                case 9: pMobIndex->position=13; break;
                case 10: pMobIndex->position=14; break;
                case 11: pMobIndex->position=15; break;
            }
         } else {
            pMobIndex->position-=100;
         }

	/* pMobIndex->defposition		= fread_number( fp ); */
         pMobIndex->defposition          = fread_number( fp );
         if(pMobIndex->defposition<100){
            switch(pMobIndex->defposition){
                default: 
                case 0: 
                case 1: 
                case 2: 
                case 3: 
                case 4: break;
                case 5: pMobIndex->defposition=6; break;
                case 6: pMobIndex->defposition=8; break;
                case 7: pMobIndex->defposition=9; break;
                case 8: pMobIndex->defposition=12; break;
                case 9: pMobIndex->defposition=13; break;
                case 10: pMobIndex->defposition=14; break;
                case 11: pMobIndex->defposition=15; break;
            }
         } else {
            pMobIndex->defposition-=100;
         }


	/*
	 * Back to meaningful values.
	 */
	pMobIndex->sex			= fread_number( fp );

	if ( letter != 'S' && letter != 'C' && letter != 'V')
	{
	    bug( "Load_mobiles: vnum %d: letter '%c' not S or C.", vnum,
	        letter );
	    shutdown_mud( "bad mob data" );
	    exit( 1 );
	}
	if ( letter == 'C' || letter == 'V') /* Realms complex mob -Thoric */
	{
	    pMobIndex->perm_str			= fread_number( fp );
	    pMobIndex->perm_int			= fread_number( fp );
	    pMobIndex->perm_wis			= fread_number( fp );
	    pMobIndex->perm_dex			= fread_number( fp );
	    pMobIndex->perm_con			= fread_number( fp );
	    pMobIndex->perm_cha			= fread_number( fp );
	    pMobIndex->perm_lck			= fread_number( fp );
 	    pMobIndex->saving_poison_death	= fread_number( fp );
	    pMobIndex->saving_wand		= fread_number( fp );
	    pMobIndex->saving_para_petri	= fread_number( fp );
	    pMobIndex->saving_breath		= fread_number( fp );
	    pMobIndex->saving_spell_staff	= fread_number( fp );
	    ln = fread_line( fp );
	    x1=x2=x3=x4=x5=x6=x7=x8=0;
	    sscanf( ln, "%d %d %d %d %d %d %d",
		&x1, &x2, &x3, &x4, &x5, &x6, &x7 );
	    pMobIndex->race		= x1;
	    pMobIndex->class		= x2;
	    pMobIndex->height		= x3;
	    pMobIndex->weight		= x4;
	    pMobIndex->speaks		= x5;
	    pMobIndex->speaking		= x6;
	    pMobIndex->numattacks	= x7;
	    /*  Thanks to Nick Gammon for noticing this.
	    if ( !pMobIndex->speaks )
		pMobIndex->speaks = race_table[pMobIndex->race]->language | LANG_COMMON;
	    if ( !pMobIndex->speaking )
		pMobIndex->speaking = race_table[pMobIndex->race]->language;
	    */
	    if ( !pMobIndex->speaks )
		pMobIndex->speaks = LANG_COMMON;
	    if ( !pMobIndex->speaking )
		pMobIndex->speaking = LANG_COMMON;

#ifndef XBI
	    ln = fread_line( fp );
	    x1=x2=x3=x4=x5=x6=x7=x8=0;
	    sscanf( ln, "%d %d %d %d %d %d %d %d",
		&x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8 );
	    pMobIndex->hitroll		= x1;
	    pMobIndex->damroll		= x2;
	    pMobIndex->xflags		= x3;
	    pMobIndex->resistant	= x4;
	    pMobIndex->immune		= x5;
	    pMobIndex->susceptible	= x6;
	    pMobIndex->attacks		= x7;
	    pMobIndex->defenses		= x8;
#else
	    pMobIndex->hitroll		= fread_number(fp);
	    pMobIndex->damroll		= fread_number(fp);
	    pMobIndex->xflags		= fread_number(fp);
	    pMobIndex->resistant	= fread_number(fp);
	    pMobIndex->immune		= fread_number(fp);
	    pMobIndex->susceptible	= fread_number(fp);
	    pMobIndex->attacks		= fread_bitvector(fp);
	    pMobIndex->defenses		= fread_bitvector(fp);
#endif
	}
	else
	{
	    pMobIndex->perm_str		= 13;
	    pMobIndex->perm_dex		= 13;
	    pMobIndex->perm_int		= 13;
	    pMobIndex->perm_wis		= 13;
	    pMobIndex->perm_cha		= 13;
	    pMobIndex->perm_con		= 13;
	    pMobIndex->perm_lck		= 13;
	    pMobIndex->race		= 0;
	    pMobIndex->class		= 3;
	    pMobIndex->xflags		= 0;
	    pMobIndex->resistant	= 0;
	    pMobIndex->immune		= 0;
	    pMobIndex->susceptible	= 0;
	    pMobIndex->numattacks	= 0;
#ifdef XBI
	    xCLEAR_BITS(pMobIndex->attacks);
	    xCLEAR_BITS(pMobIndex->defenses);
#else
	    pMobIndex->attacks		= 0;
	    pMobIndex->defenses		= 0;
#endif
	}

      if (letter == 'V')        /* Very complex mob (stances) SHADDAI */
        {
          int temp;
          for (temp = 0; temp < MAX_STANCE; temp++)
            pMobIndex->stances[temp] = fread_number (fp);
        }
      else
        /* Set the BASIC stances if no stances are set */
        {
          int temp;

	  if (pMobIndex->level < 10)
            {
              for (temp = 0; temp < BASIC_STANCE; temp++)
                pMobIndex->stances[temp] = 25;
            }
          else if (pMobIndex->level < 20)
            {
              for (temp = 0; temp < BASIC_STANCE; temp++)
                pMobIndex->stances[temp] = 50;
            }
          else if (pMobIndex->level < 30)
            {
              for (temp = 0; temp < BASIC_STANCE; temp++)
                pMobIndex->stances[temp] = 100;
            }
          else if (pMobIndex->level < 40)
            {
              for (temp = 0; temp < BASIC_STANCE; temp++)
                pMobIndex->stances[temp] = 150;
            }
          else if (pMobIndex->level < 50)
            {
              for (temp = 0; temp < BASIC_STANCE; temp++)
                pMobIndex->stances[temp] = 200;
            }
          else
            {
              for (temp = 0; temp < BASIC_STANCE; temp++)
                pMobIndex->stances[temp] = 240;
            }
          pMobIndex->stances[0] = number_range (0, (BASIC_STANCE - 1));
        }
        letter = fread_letter( fp );
	if ( letter == '>' )
	{
	    ungetc( letter, fp );
	    mprog_read_programs( fp, pMobIndex );
	}
	else ungetc( letter,fp );

	if ( !oldmob )
	{
	    iHash			= vnum % MAX_KEY_HASH;
	    pMobIndex->next		= mob_index_hash[iHash];
	    mob_index_hash[iHash]	= pMobIndex;
	    top_mob_index++;
	}
    }

    return;
}



/*
 * Load an obj section.
 */
void load_objects( AREA_DATA *tarea, FILE *fp )
{
    OBJ_INDEX_DATA *pObjIndex;
    char letter;
    char *ln;
    int x1, x2, x3, x4, x5, x6;

    if ( !tarea )
    {
	bug( "Load_objects: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }

    for ( ; ; )
    {
	char buf[MAX_STRING_LENGTH];
	int vnum;
	int iHash;
	bool tmpBootDb;
	bool oldobj;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_objects: # not found." );
	    if ( fBootDb )
	    {
		shutdown_mud( "# not found" );
		exit( 1 );
	    }
	    else
		return;
	}

	vnum				= fread_number( fp );
	if ( vnum == 0 )
	    break;

	tmpBootDb = fBootDb;
	fBootDb = FALSE;
	if ( get_obj_index( vnum ) )
	{
	    if ( tmpBootDb )
	    {
		bug( "Load_objects: vnum %d duplicated.", vnum );
		shutdown_mud( "duplicate vnum" );
		exit( 1 );
	    }
	    else
	    {
		pObjIndex = get_obj_index( vnum );
		sprintf( buf, "Cleaning object: %d", vnum );
		log_string_plus( buf, LOG_BUILD, sysdata.log_level );
		clean_obj( pObjIndex );
		oldobj = TRUE;
	    }
	}
	else
	{
	  oldobj = FALSE;
	  CREATE( pObjIndex, OBJ_INDEX_DATA, 1 );
	}
	fBootDb = tmpBootDb;

	pObjIndex->vnum			= vnum;
	if ( fBootDb )
	{
	  if ( !tarea->low_o_vnum )
	    tarea->low_o_vnum		= vnum;
	  if ( vnum > tarea->hi_o_vnum )
	    tarea->hi_o_vnum		= vnum;
	}
	pObjIndex->name			= fread_string( fp );
	pObjIndex->short_descr		= fread_string( fp );
	pObjIndex->description		= fread_string( fp );
	pObjIndex->action_desc		= fread_string( fp );

        /* Commented out by Narn, Apr/96 to allow item short descs like 
           Bonecrusher and Oblivion */
	/*pObjIndex->short_descr[0]	= LOWER(pObjIndex->short_descr[0]);*/
	pObjIndex->description[0]	= UPPER(pObjIndex->description[0]);

	pObjIndex->item_type		= fread_number(fp);
	pObjIndex->extra_flags		= fread_bitvector(fp);
	ln = fread_line( fp );
    x1=x2=x3=0;
       if ( area_version > 1 )
       {
       sscanf( ln, "%d %d %d",
               &x1, &x2, &x3 );
       pObjIndex->wear_flags           = x1;
		if ( area_version == 2 )
       	{
		if ( x2 == 127 )
			pObjIndex->layers               = 0;
		else
			pObjIndex->layers			= x2;
		}
		else pObjIndex->layers			=x2;			

       pObjIndex->level                = x3;
       }
       else
       {
       sscanf( ln, "%d %d",
              &x1, &x2);
       pObjIndex->wear_flags           = x1;
       pObjIndex->layers               = x2;
       }

	ln = fread_line( fp );
	x1=x2=x3=x4=x5=x6=0;
	sscanf( ln, "%d %d %d %d %d %d",
		&x1, &x2, &x3, &x4, &x5, &x6 );
	pObjIndex->value[0]		= x1;
	pObjIndex->value[1]		= x2;
	pObjIndex->value[2]		= x3;
	pObjIndex->value[3]		= x4;
	pObjIndex->value[4]		= x5;
	pObjIndex->value[5]		= x6;
	pObjIndex->weight		= fread_number( fp );
	pObjIndex->weight = UMAX( 1, pObjIndex->weight );
	pObjIndex->cost			= fread_number( fp );
	pObjIndex->rent		  	= fread_number( fp ); /* unused */
        if ( area_version > 0 )
	{
	  switch ( pObjIndex->item_type )
	  {
	  case ITEM_PILL:
	  case ITEM_POTION:
	  case ITEM_SCROLL:
	      pObjIndex->value[1] = skill_lookup ( fread_word( fp )) ;
	      pObjIndex->value[2] = skill_lookup ( fread_word( fp )) ;
	      pObjIndex->value[3] = skill_lookup ( fread_word( fp )) ;
	    break;
	  case ITEM_STAFF:
	  case ITEM_WAND:
	      pObjIndex->value[3] = skill_lookup ( fread_word( fp )) ;
	    break;
	  case ITEM_SALVE:
	      pObjIndex->value[4] = skill_lookup ( fread_word( fp )) ;
	      pObjIndex->value[5] = skill_lookup ( fread_word( fp )) ;
	    break;
	  }
	}
	for ( ; ; )
	{
	    letter = fread_letter( fp );

	    if ( letter == 'A' )
	    {
		AFFECT_DATA *paf;

		CREATE( paf, AFFECT_DATA, 1 );
		paf->type		= -1;
		paf->duration		= -1;
		paf->location		= fread_number( fp );
		if ( paf->location == APPLY_WEAPONSPELL
		||   paf->location == APPLY_WEARSPELL
		||   paf->location == APPLY_REMOVESPELL
		||   paf->location == APPLY_STRIPSN
		||   paf->location == APPLY_RECURRINGSPELL )
		  paf->modifier		= slot_lookup( fread_number(fp) );
		else
		  paf->modifier		= fread_number( fp );
		xCLEAR_BITS(paf->bitvector);
		LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect,
			   next, prev );
		top_affect++;
	    }

	    else if ( letter == 'E' )
	    {
		EXTRA_DESCR_DATA *ed;

		CREATE( ed, EXTRA_DESCR_DATA, 1 );
		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc,
			  next, prev );
		top_ed++;
	    }
	    else if ( letter == '>' )
	    {
	        ungetc( letter, fp );
	        oprog_read_programs( fp, pObjIndex );
	    }

	    else
	    {
		ungetc( letter, fp );
		break;
	    }
	}

	/*
	 * Translate spell "slot numbers" to internal "skill numbers."
	 */
	if ( area_version == 0 )
	   switch ( pObjIndex->item_type )
	   {
	   case ITEM_PILL:
	   case ITEM_POTION:
	   case ITEM_SCROLL:
	       pObjIndex->value[1] = slot_lookup( pObjIndex->value[1] );
	       pObjIndex->value[2] = slot_lookup( pObjIndex->value[2] );
	       pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
	       break;
   
	   case ITEM_STAFF:
	   case ITEM_WAND:
	       pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
	       break;
	   case ITEM_SALVE:
	       pObjIndex->value[4] = slot_lookup( pObjIndex->value[4] );
	       pObjIndex->value[5] = slot_lookup( pObjIndex->value[5] );
	       break;
	   }

	if ( !oldobj )
	{
	  iHash			= vnum % MAX_KEY_HASH;
	  pObjIndex->next	= obj_index_hash[iHash];
	  obj_index_hash[iHash]	= pObjIndex;
	  top_obj_index++;
	}
    }

    return;
}



/*
 * Load a reset section.
 */
void load_resets( AREA_DATA *tarea, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    bool not01 = FALSE;
    int count = 0;

    if ( !tarea )
    {
	bug( "Load_resets: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }

    if ( tarea->first_reset )
    {
	if ( fBootDb )
	{
	  RESET_DATA *rtmp;

	  bug( "load_resets: WARNING: resets already exist for this area." );
	  for ( rtmp = tarea->first_reset; rtmp; rtmp = rtmp->next )
		++count;
	}
	else
	{
	 /*
	  * Clean out the old resets
	  */
	  sprintf( buf, "Cleaning resets: %s", tarea->name );
	  log_string_plus( buf, LOG_BUILD, sysdata.log_level );
	  clean_resets( tarea );
	}	
    }

    for ( ; ; )
    {
	ROOM_INDEX_DATA *pRoomIndex;
	EXIT_DATA *pexit;
	char letter;
	int extra, arg1, arg2, arg3;

	if ( ( letter = fread_letter( fp ) ) == 'S' )
	    break;

	if ( letter == '*' )
	{
	    fread_to_eol( fp );
	    continue;
	}

	extra	= fread_number( fp );
	arg1	= fread_number( fp );
	arg2	= fread_number( fp );
	arg3	= (letter == 'G' || letter == 'R')
		  ? 0 : fread_number( fp );
		  fread_to_eol( fp );

	++count;

	/*
	 * Validate parameters.
	 * We're calling the index functions for the side effect.
	 */
	switch ( letter )
	{
	default:
	    bug( "Load_resets: bad command '%c'.", letter );
	    if ( fBootDb )
	      boot_log( "Load_resets: %s (%d) bad command '%c'.", tarea->filename, count, letter );
	    return;

	case 'M':
	    if ( get_mob_index( arg1 ) == NULL && fBootDb )
		boot_log( "Load_resets: %s (%d) 'M': mobile %d doesn't exist.",
		    tarea->filename, count, arg1 );
	    if ( get_room_index( arg3 ) == NULL && fBootDb )
		boot_log( "Load_resets: %s (%d) 'M': room %d doesn't exist.",
		    tarea->filename, count, arg3 );
	    break;

	case 'O':
	    if ( get_obj_index(arg1) == NULL && fBootDb )
		boot_log( "Load_resets: %s (%d) '%c': object %d doesn't exist.",
		    tarea->filename, count, letter, arg1 );
	    if ( get_room_index(arg3) == NULL && fBootDb )
		boot_log( "Load_resets: %s (%d) '%c': room %d doesn't exist.",
		    tarea->filename, count, letter, arg3 );
	    break;

	case 'P':
	    if ( get_obj_index(arg1) == NULL && fBootDb )
		boot_log( "Load_resets: %s (%d) '%c': object %d doesn't exist.",
		    tarea->filename, count, letter, arg1 );
	    if ( arg3 > 0 ) {
		if ( get_obj_index(arg3) == NULL && fBootDb )
		    boot_log( "Load_resets: %s (%d) 'P': destination object %d doesn't exist.",
			tarea->filename, count, arg3 ); }
	    else if ( extra > 1 )
	      not01 = TRUE;
	    break;

	case 'G':
	case 'E':
	    if ( get_obj_index(arg1) == NULL && fBootDb )
		boot_log( "Load_resets: %s (%d) '%c': object %d doesn't exist.",
		    tarea->filename, count, letter, arg1 );
	    break;

	case 'T':
	    break;

	case 'H':
	    if ( arg1 > 0 )
		if ( get_obj_index(arg1) == NULL && fBootDb )
		    boot_log( "Load_resets: %s (%d) 'H': object %d doesn't exist.",
			tarea->filename, count, arg1 );
	    break;

	case 'B':
	    switch(arg2 & BIT_RESET_TYPE_MASK)
	    {
	    case BIT_RESET_DOOR:
	      {
	      int door;
	      
	      pRoomIndex = get_room_index( arg1 );
	      if ( !pRoomIndex )
	      {
		bug( "Load_resets: 'B': room %d doesn't exist.", arg1 );
		bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
		    arg3 );
		if ( fBootDb )
		   boot_log( "Load_resets: %s (%d) 'B': room %d doesn't exist.",
			tarea->filename, count, arg1 );
	      }
	    
	      door = (arg2 & BIT_RESET_DOOR_MASK) >> BIT_RESET_DOOR_THRESHOLD;

	      if ( !(pexit = get_exit(pRoomIndex, door)) )
	      {
		bug( "Load_resets: 'B': exit %d not door.", door );
		bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
		    arg3 );
		if ( fBootDb )
		   boot_log( "Load_resets: %s (%d) 'B': exit %d not door.",
			tarea->filename, count, door );
	      }
	      }
	      break;
	    case BIT_RESET_ROOM:
	      if (get_room_index(arg1) == NULL)
	      {
	        bug( "Load_resets: 'B': room %d doesn't exist.", arg1);
	        bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
	            arg3 );
	        if ( fBootDb )
	           boot_log( "Load_resets: %s (%d) 'B': room %d doesn't exist.",
	                tarea->filename, count, arg1 );
	      }
	      break;
	    case BIT_RESET_OBJECT:
	      if (arg1 > 0)
	        if (get_obj_index(arg1) == NULL && fBootDb)
	          boot_log("Load_resets: %s (%d) 'B': object %d doesn't exist.",
	              tarea->filename, count, arg1 );
	      break;
	    case BIT_RESET_MOBILE:
	      if (arg1 > 0)
	        if (get_mob_index(arg1) == NULL && fBootDb)
	          boot_log("Load_resets: %s (%d) 'B': mobile %d doesn't exist.",
	              tarea->filename, count, arg1 );
	      break;
	    default:
	      boot_log( "Load_resets: %s (%d) 'B': bad type flag (%d).",
	           tarea->filename, count, arg2 & BIT_RESET_TYPE_MASK );
	      break;
	    }
	    break;

	case 'D':
	    pRoomIndex = get_room_index( arg1 );
	    if ( !pRoomIndex )
	    {
		bug( "Load_resets: 'D': room %d doesn't exist.", arg1 );
		bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
		    arg3 );
		if ( fBootDb )
		   boot_log( "Load_resets: %s (%d) 'D': room %d doesn't exist.",
			tarea->filename, count, arg1 );
		break;
	    }

	    if ( arg2 < 0
	    ||   arg2 > MAX_DIR+1
	    || ( pexit = get_exit(pRoomIndex, arg2)) == NULL
	    || !IS_SET( pexit->exit_info, EX_ISDOOR ) )
	    {
		bug( "Load_resets: 'D': exit %d not door.", arg2 );
		bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
		    arg3 );
		if ( fBootDb )
		   boot_log( "Load_resets: %s (%d) 'D': exit %d not door.",
			tarea->filename, count, arg2 );
	    }

	    if ( arg3 < 0 || arg3 > 2 )
	    {
		bug( "Load_resets: 'D': bad 'locks': %d.", arg3 );
		if ( fBootDb )
		  boot_log( "Load_resets: %s (%d) 'D': bad 'locks': %d.",
			tarea->filename, count, arg3 );
	    }
	    break;

	case 'R':
	    pRoomIndex = get_room_index( arg1 );
	    if ( !pRoomIndex && fBootDb )
		boot_log( "Load_resets: %s (%d) 'R': room %d doesn't exist.",
		    tarea->filename, count, arg1 );

	    if ( arg2 < 0 || arg2 > 10 )
	    {
		bug( "Load_resets: 'R': bad exit %d.", arg2 );
		if ( fBootDb )
		  boot_log( "Load_resets: %s (%d) 'R': bad exit %d.",
			tarea->filename, count, arg2 );
		break;
	    }

	    break;
	}

	/* finally, add the reset */
	add_reset( tarea, letter, extra, arg1, arg2, arg3 );
    }
    
    if ( !not01 )
      renumber_put_resets(tarea);

    return;
}



/*
 * Load a room section.
 */
void load_rooms( AREA_DATA *tarea, FILE *fp )
{
    ROOM_INDEX_DATA *pRoomIndex;
    char buf[MAX_STRING_LENGTH];
    char *ln;

    if ( !tarea )
    {
	bug( "Load_rooms: no #AREA seen yet." );
	shutdown_mud( "No #AREA" );
	exit( 1 );
    }

    for ( ; ; )
    {
	int vnum;
	char letter;
	int door;
	int iHash;
	bool tmpBootDb;
	bool oldroom;
	int x1, x2, x3, x4, x5, x6;
	int x7;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_rooms: # not found." );
	    if ( fBootDb )
	    {
		shutdown_mud( "# not found" );
		exit( 1 );
	    }
	    else
		return;
	}

	vnum				= fread_number( fp );
	if ( vnum == 0 )
	    break;

	tmpBootDb = fBootDb;
	fBootDb = FALSE;
	if ( get_room_index( vnum ) != NULL )
	{
	    if ( tmpBootDb )
	    {
	      bug( "Load_rooms: vnum %d duplicated.", vnum );
	      shutdown_mud( "duplicate vnum" );
	      exit( 1 );
	    }
	    else
	    {
	      pRoomIndex = get_room_index( vnum );
	      sprintf( buf, "Cleaning room: %d", vnum );
	      log_string_plus( buf, LOG_BUILD, sysdata.log_level );
	      clean_room( pRoomIndex );
	      oldroom = TRUE;
	    }
	}
	else
	{
	  oldroom = FALSE;
	  CREATE( pRoomIndex, ROOM_INDEX_DATA, 1 );
	  pRoomIndex->first_person	= NULL;
	  pRoomIndex->last_person	= NULL;
	  pRoomIndex->first_content	= NULL;
	  pRoomIndex->last_content	= NULL;
	}

	fBootDb = tmpBootDb;
	pRoomIndex->area		= tarea;
	pRoomIndex->vnum		= vnum;
	pRoomIndex->first_extradesc	= NULL;
	pRoomIndex->last_extradesc	= NULL;

	if ( fBootDb )
	{
	  if ( !tarea->low_r_vnum )
	    tarea->low_r_vnum		= vnum;
	  if ( vnum > tarea->hi_r_vnum )
	    tarea->hi_r_vnum		= vnum;
	}
	pRoomIndex->name		= fread_string( fp );
	pRoomIndex->description		= fread_string( fp );

	/* Area number			  fread_number( fp ); */
    x1=x2=x3=x4=x5=x6=x7=0;
    x1 = fread_number( fp );
    pRoomIndex->room_flags = fread_bitvector( fp );
    ln = fread_line( fp );
    sscanf( ln, "%d %d %d %d %d",
         &x3, &x4, &x5, &x6, &x7 );
    pRoomIndex->max_weight  = x7;
	pRoomIndex->sector_type		= x3;
	pRoomIndex->tele_delay		= x4;
	pRoomIndex->tele_vnum		= x5;
	pRoomIndex->tunnel		= x6;

	if (pRoomIndex->sector_type < 0 || pRoomIndex->sector_type == SECT_MAX)
	{
	  bug( "Fread_rooms: vnum %d has bad sector_type %d.", vnum ,
	      pRoomIndex->sector_type);
	  pRoomIndex->sector_type = 1;
	}
	if ( xIS_SET( pRoomIndex->room_flags, ROOM_HOUSE ) )
		pRoomIndex->max_weight = 2000;
	pRoomIndex->light		= 0;
	pRoomIndex->first_exit		= NULL;
	pRoomIndex->last_exit		= NULL;

	for ( ; ; )
	{
	    letter = fread_letter( fp );

	    if ( letter == 'S' )
		break;

	    if ( letter == 'D' )
	    {
		EXIT_DATA *pexit;
		int locks;

		door = fread_number( fp );
		if ( door < 0 || door > 10 )
		{
		    bug( "Fread_rooms: vnum %d has bad door number %d.", vnum,
		        door );
		    if ( fBootDb )
		      exit( 1 );
		}
		else
		{
		  pexit = make_exit( pRoomIndex, NULL, door );
		  pexit->description	= fread_string( fp );
		  pexit->keyword	= fread_string( fp );
		  pexit->exit_info	= 0;
		  ln = fread_line( fp );
		  x1=x2=x3=x4=x5=x6=0;
		  sscanf( ln, "%d %d %d %d %d %d",
		      &x1, &x2, &x3, &x4, &x5, &x6 );

		  locks			= x1;
		  pexit->key		= x2;
		  pexit->vnum		= x3;
		  pexit->vdir		= door;
		  pexit->orig_door	= door;
		  pexit->distance	= x4;
		  pexit->pulltype	= x5;
		  pexit->pull		= x6;

		  switch ( locks )
		  {
		    case 1:  pexit->exit_info = EX_ISDOOR;                break;
		    case 2:  pexit->exit_info = EX_ISDOOR | EX_PICKPROOF; break;
		    default: pexit->exit_info = locks;
		  }
		}
	    }
	    else if ( letter == 'E' )
	    {
		EXTRA_DESCR_DATA *ed;

		CREATE( ed, EXTRA_DESCR_DATA, 1 );
		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		LINK( ed, pRoomIndex->first_extradesc, pRoomIndex->last_extradesc,
			  next, prev );
		top_ed++;
	    }
	    else if ( letter == 'M' )    /* maps */
	    {
		MAP_DATA *map;
		MAP_INDEX_DATA *map_index;
		int i, j;

		CREATE( map, MAP_DATA, 1);
                map->vnum                     = fread_number( fp );
                map->x                        = fread_number( fp ); 
                map->y                        = fread_number( fp );
		map->entry		      = fread_letter( fp );
		
                pRoomIndex->map               = map;
		if(  (map_index = get_map_index(map->vnum)) == NULL  )
		{
                     CREATE( map_index, MAP_INDEX_DATA, 1);
		     map_index->vnum = map->vnum;
		     map_index->next = first_map;
                     first_map       = map_index;
		     for (i = 0; i <  49; i++) {
			     for (j = 0; j <  79; j++) {
			       map_index->map_of_vnums[i][j] = -1;
			       /* map_index->map_of_ptrs[i][j] = NULL; */
                             }
                     }
		}
		if( (map->y <0) || (map->y >48) )
		{
                    bug("Map y coord out of range.  Room %d\n\r", map->y);

		}
		if( (map->x <0) || (map->x >78) )
		{
                    bug("Map x coord out of range.  Room %d\n\r", map->x);

		}
		if(  (map->x >0) 
		   &&(map->x <80) 
		   &&(map->y >0) 
		   &&(map->y <48) )
		   map_index->map_of_vnums[map->y][map->x]=pRoomIndex->vnum;
            }
	    else if ( letter == '>' )
	    {
	      ungetc( letter, fp );
	      rprog_read_programs( fp, pRoomIndex );
            }
	    else
	    {
		bug( "Load_rooms: vnum %d has flag '%c' not 'DES'.", vnum,
		    letter );
		shutdown_mud( "Room flag not DES" );
		exit( 1 );
	    }

	}

	if ( !oldroom )
	{
	  iHash			 = vnum % MAX_KEY_HASH;
	  pRoomIndex->next	 = room_index_hash[iHash];
	  room_index_hash[iHash] = pRoomIndex;
	  top_room++;
	}
    }

    return;
}



/*
 * Load a shop section.
 */
void load_shops( AREA_DATA *tarea, FILE *fp )
{
    SHOP_DATA *pShop;

    for ( ; ; )
    {
	MOB_INDEX_DATA *pMobIndex;
	int iTrade;

	CREATE( pShop, SHOP_DATA, 1 );
	pShop->keeper		= fread_number( fp );
	if ( pShop->keeper == 0 )
	{
	    DISPOSE( pShop );
	    break;
	}
	for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
	    pShop->buy_type[iTrade]	= fread_number( fp );
	pShop->profit_buy	= fread_number( fp );
	pShop->profit_sell	= fread_number( fp );
	pShop->profit_buy	= URANGE( pShop->profit_sell+5, pShop->profit_buy, 1000 );
	pShop->profit_sell	= URANGE( 0, pShop->profit_sell, pShop->profit_buy-5 );
	pShop->open_hour	= fread_number( fp );
	pShop->close_hour	= fread_number( fp );
				  fread_to_eol( fp );
	pMobIndex		= get_mob_index( pShop->keeper );
	pMobIndex->pShop	= pShop;

	if ( !first_shop )
	    first_shop		= pShop;
	else
	    last_shop->next	= pShop;
	pShop->next		= NULL;
	pShop->prev		= last_shop;
	last_shop		= pShop;
	top_shop++;
    }
    return;
}

/*
 * Load a repair shop section.					-Thoric
 */
void load_repairs( AREA_DATA *tarea, FILE *fp )
{
    REPAIR_DATA *rShop;

    for ( ; ; )
    {
	MOB_INDEX_DATA *pMobIndex;
	int iFix;

	CREATE( rShop, REPAIR_DATA, 1 );
	rShop->keeper		= fread_number( fp );
	if ( rShop->keeper == 0 )
	{
	    DISPOSE( rShop );
	    break;
	}
	for ( iFix = 0; iFix < MAX_FIX; iFix++ )
	  rShop->fix_type[iFix] = fread_number( fp );
	rShop->profit_fix	= fread_number( fp );
	rShop->shop_type	= fread_number( fp );
	rShop->open_hour	= fread_number( fp );
	rShop->close_hour	= fread_number( fp );
				  fread_to_eol( fp );
	pMobIndex		= get_mob_index( rShop->keeper );
	pMobIndex->rShop	= rShop;

	if ( !first_repair )
	  first_repair		= rShop;
	else
	  last_repair->next	= rShop;
	rShop->next		= NULL;
	rShop->prev		= last_repair;
	last_repair		= rShop;
	top_repair++;
    }
    return;
}


/*
 * Load spec proc declarations.
 */
void load_specials( AREA_DATA *tarea, FILE *fp )
{
    for ( ; ; )
    {
	MOB_INDEX_DATA *pMobIndex;
	char letter;

	switch ( letter = fread_letter( fp ) )
	{
	default:
	    bug( "Load_specials: letter '%c' not *MS.", letter );
	    exit( 1 );

	case 'S':
	    return;

	case '*':
	    break;

	case 'M':
	    pMobIndex		= get_mob_index	( fread_number ( fp ) );
	    pMobIndex->spec_fun	= spec_lookup	( fread_word   ( fp ) );
	    if ( pMobIndex->spec_fun == 0 )
	    {
		bug( "Load_specials: 'M': vnum %d.", pMobIndex->vnum );
		exit( 1 );
	    }
	    break;
	}

	fread_to_eol( fp );
    }
}


/*
 * Load soft / hard area ranges.
 */
void load_ranges( AREA_DATA *tarea, FILE *fp )
{
    int x1, x2, x3, x4;
    char *ln;

    if ( !tarea )
    {
	bug( "Load_ranges: no #AREA seen yet." );
	shutdown_mud( "No #AREA" );
	exit( 1 );
    }

    for ( ; ; )
    {
	ln = fread_line( fp );

	if (ln[0] == '$')
	  break;

	x1=x2=x3=x4=0;
	sscanf( ln, "%d %d %d %d",
	      &x1, &x2, &x3, &x4 );

	tarea->low_soft_range = x1;
	tarea->hi_soft_range = x2;
	tarea->low_hard_range = x3;
	tarea->hi_hard_range = x4;
    }
    return;

}              

/*
 * Load climate information for the area
 * Last modified: July 13, 1997
 * Fireblade
 */
void load_climate(AREA_DATA *tarea, FILE *fp)
{
	if ( !tarea )
	{
		bug("load_climate: no #AREA seen yet");
		if(fBootDb)
		{
			shutdown_mud("No #AREA");
			exit(1);
		}
		else
			return;
	}
	
	tarea->weather->climate_temp = fread_number(fp);
	tarea->weather->climate_precip = fread_number(fp);
	tarea->weather->climate_wind = fread_number(fp);
	
	return;
}

/*
 * Load data for a neghboring weather system
 * Last modified: July 13, 1997
 * Fireblade
 */
void load_neighbor(AREA_DATA *tarea, FILE *fp)
{
	NEIGHBOR_DATA *new;
	
	if(!tarea)
	{
		bug("load_neighbor: no #AREA seen yet.");
		if(fBootDb)
		{
			shutdown_mud("No #AREA");
			exit(1);
		}
		else
			return;
	}
	
	CREATE(new, NEIGHBOR_DATA, 1);
	new->next = NULL;
	new->prev = NULL;
	new->address = NULL;
	new->name = fread_string(fp);
	LINK(new,
	     tarea->weather->first_neighbor,
	     tarea->weather->last_neighbor,
	     next, prev);
	
	return;
}
	

/*
 * Go through all areas, and set up initial economy based on mob
 * levels and gold
 */
void initialize_economy( void )
{
    AREA_DATA *tarea;
    MOB_INDEX_DATA *mob;
    int idx, gold, rng;

    for ( tarea = first_area; tarea; tarea = tarea->next )
    {
	/* skip area if they already got some gold */
	if ( tarea->high_economy > 0 || tarea->low_economy > 10000 )
	  continue;
	rng = tarea->hi_soft_range - tarea->low_soft_range;
	if ( rng )
	  rng /= 2;
	else
	  rng = 25;
	gold = rng * rng * 50000;
	boost_economy( tarea, gold );
	for ( idx = tarea->low_m_vnum; idx < tarea->hi_m_vnum; idx++ )
	    if ( (mob=get_mob_index(idx)) != NULL )
		boost_economy( tarea, mob->gold * 10 );
    }
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits( void )
{
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pexit, *pexit_next, *rev_exit;
    int iHash;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pRoomIndex  = room_index_hash[iHash];
	      pRoomIndex;
	      pRoomIndex  = pRoomIndex->next )
	{
	    bool fexit;

	    fexit = FALSE;
	    for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit_next )
	    {
		pexit_next = pexit->next;
		pexit->rvnum = pRoomIndex->vnum;
		if ( pexit->vnum <= 0
		||  (pexit->to_room=get_room_index(pexit->vnum)) == NULL )
		{
		    if ( fBootDb )
			boot_log( "Fix_exits: room %d, exit %s leads to bad vnum (%d)",
				pRoomIndex->vnum, dir_name[pexit->vdir], pexit->vnum );
		    
		    bug( "Deleting %s exit in room %d", dir_name[pexit->vdir],
				pRoomIndex->vnum );
		    extract_exit( pRoomIndex, pexit );
		}
		else
		  fexit = TRUE;
	    }
	    if ( !fexit )
	      xSET_BIT( pRoomIndex->room_flags, ROOM_NO_MOB );
	}
    }

    /* Set all the rexit pointers 	-Thoric */
    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for ( pRoomIndex  = room_index_hash[iHash];
	      pRoomIndex;
	      pRoomIndex  = pRoomIndex->next )
	{
	    for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
	    {
		if ( pexit->to_room && !pexit->rexit )
		{
		   rev_exit = get_exit_to( pexit->to_room, rev_dir[pexit->vdir], pRoomIndex->vnum );
		   if ( rev_exit )
		   {
			pexit->rexit	= rev_exit;
			rev_exit->rexit	= pexit;
		   }
		}
	    }
	}
    }

    return;
}


/*
 * Get diku-compatable exit by number				-Thoric
 */
EXIT_DATA *get_exit_number( ROOM_INDEX_DATA *room, int xit )
{
    EXIT_DATA *pexit;
    int count;

    count = 0;
    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
	if ( ++count == xit )
	  return pexit;
    return NULL;
}

/*
 * (prelude...) This is going to be fun... NOT!
 * (conclusion) QSort is f*cked!
 */
int exit_comp( EXIT_DATA **xit1, EXIT_DATA **xit2 )
{
    int d1, d2;

    d1 = (*xit1)->vdir;
    d2 = (*xit2)->vdir;

    if ( d1 < d2 )
      return -1;
    if ( d1 > d2 )
      return 1;
    return 0;
}

void sort_exits( ROOM_INDEX_DATA *room )
{
    EXIT_DATA *pexit; /* *texit */ /* Unused */
    EXIT_DATA *exits[MAX_REXITS];
    int x, nexits;

    nexits = 0;
    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
    {
	exits[nexits++] = pexit;
	if ( nexits > MAX_REXITS )
	{
	    bug( "sort_exits: more than %d exits in room... fatal", nexits );
	    return;
	}
    }
    qsort( &exits[0], nexits, sizeof( EXIT_DATA * ),
		(int(*)(const void *, const void *)) exit_comp );
    for ( x = 0; x < nexits; x++ )
    {
	if ( x > 0 )
	  exits[x]->prev	= exits[x-1];
	else
	{
	  exits[x]->prev	= NULL;
	  room->first_exit	= exits[x];
	}
	if ( x >= (nexits - 1) )
	{
	  exits[x]->next	= NULL;
	  room->last_exit	= exits[x];
	}
	else
	  exits[x]->next	= exits[x+1];
    }
}

void randomize_exits( ROOM_INDEX_DATA *room, sh_int maxdir )
{
    EXIT_DATA *pexit;
    int nexits, /* maxd, */ d0, d1, count, door; /* Maxd unused */
    int vdirs[MAX_REXITS];

    nexits = 0;
    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
       vdirs[nexits++] = pexit->vdir;

    for ( d0 = 0; d0 < nexits; d0++ )
    {
	if ( vdirs[d0] > maxdir )
	  continue;
	count = 0;
	while ( vdirs[(d1 = number_range( d0, nexits - 1 ))] > maxdir
	||      ++count > 5 );
	if ( vdirs[d1] > maxdir )
	  continue;
	door		= vdirs[d0];
	vdirs[d0]	= vdirs[d1];
	vdirs[d1]	= door;
    }
    count = 0;
    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
       pexit->vdir = vdirs[count++];

    sort_exits( room );
}


/*
 * Repopulate areas periodically.
 */
void area_update( void )
{
    AREA_DATA *pArea;

    for ( pArea = first_area; pArea; pArea = pArea->next )
    {
	CHAR_DATA *pch;
	int reset_age = pArea->reset_frequency ? pArea->reset_frequency : 15;

	if ( (reset_age == -1 && pArea->age == -1)
	||    ++pArea->age < (reset_age-1) )
	    continue;

	/*
	 * Check for PC's.
	 */
	if ( pArea->nplayer > 0 && pArea->age == (reset_age-1) )
	{
	    char buf[MAX_STRING_LENGTH];

	    /* Rennard */
	    if ( pArea->resetmsg )
		sprintf( buf, "%s\n\r", pArea->resetmsg );
	    else
		strcpy( buf, "You hear some squeaking sounds...\n\r" );
	    for ( pch = first_char; pch; pch = pch->next )
	    {
		if ( !IS_NPC(pch)
		&&   IS_AWAKE(pch)
		&&   pch->in_room
		&&   pch->in_room->area == pArea )
		{
		    set_char_color( AT_RESET, pch );
		    send_to_char( buf, pch );
		}
	    }
	}

	/*
	 * Check age and reset.
	 * Note: Mud Academy resets every 3 minutes (not 15).
	 */
	if ( pArea->nplayer == 0 || pArea->age >= reset_age )
	{
	    ROOM_INDEX_DATA *pRoomIndex;

	    fprintf( stderr, "Resetting: %s\n", pArea->filename );
	    reset_area( pArea );
	    if ( reset_age == -1 )
		pArea->age = -1;
	    else
		pArea->age = number_range( 0, reset_age / 5 );
	    pRoomIndex = get_room_index( ROOM_VNUM_SCHOOL );
	    if ( pRoomIndex != NULL && pArea == pRoomIndex->area
	    &&   pArea->reset_frequency == 0 )
		pArea->age = 15 - 3;
	}
    }
    return;
}


/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile( MOB_INDEX_DATA *pMobIndex )
{
    CHAR_DATA *mob;

    if ( !pMobIndex )
    {
	bug( "Create_mobile: NULL pMobIndex." );
	exit( 1 );
    }

    CREATE( mob, CHAR_DATA, 1 );
    clear_char( mob );
    mob->pIndexData		= pMobIndex;

    mob->editor			= NULL;
    mob->name			= QUICKLINK( pMobIndex->player_name );
    mob->short_descr		= QUICKLINK( pMobIndex->short_descr );
    mob->long_descr		= QUICKLINK( pMobIndex->long_descr  );
    mob->description		= QUICKLINK( pMobIndex->description );
    mob->spec_fun		= pMobIndex->spec_fun;
    mob->mpscriptpos		= 0;
    mob->level			= number_fuzzy( pMobIndex->level );
    mob->act			= pMobIndex->act;

    if ( xIS_SET( mob->act, ACT_MOBINVIS) )
	mob->mobinvis = mob->level;

    mob->affected_by		= pMobIndex->affected_by;
    mob->alignment		= pMobIndex->alignment;
    mob->sex			= pMobIndex->sex;
    mob->stance 		= 0;

    /*
     * Bug fix from mailing list by stu (sprice@ihug.co.nz)
     * was:  if ( !pMobIndex->ac )
     */
    if ( pMobIndex->ac )
	mob->armor		= pMobIndex->ac;
    else
	mob->armor		= interpolate( mob->level, 100, -100 );

    if ( !pMobIndex->hitnodice )
	mob->max_hit		= mob->level * 70 + number_range(
					mob->level * mob->level / 4,
					mob->level * mob->level );
    else
	mob->max_hit		= pMobIndex->hitnodice * number_range(1, pMobIndex->hitsizedice )
				      + pMobIndex->hitplus;
    mob->hit			= mob->max_hit;
    /* lets put things back the way they used to be! -Thoric */
    mob->gold			= pMobIndex->gold;
    mob->exp			= pMobIndex->exp;
    mob->position		= pMobIndex->position;
    mob->defposition		= pMobIndex->defposition;
    mob->barenumdie		= pMobIndex->damnodice;
    mob->baresizedie		= pMobIndex->damsizedice;
    mob->mobthac0		= pMobIndex->mobthac0;
    mob->hitplus		= pMobIndex->hitplus;
    mob->damplus		= pMobIndex->damplus;

    mob->perm_str		= pMobIndex->perm_str;
    mob->perm_dex		= pMobIndex->perm_dex;
    mob->perm_wis		= pMobIndex->perm_wis;
    mob->perm_int		= pMobIndex->perm_int;
    mob->perm_con		= pMobIndex->perm_con;
    mob->perm_cha		= pMobIndex->perm_cha;
    mob->perm_lck 		= pMobIndex->perm_lck;
    mob->hitroll		= pMobIndex->hitroll;
    mob->damroll		= pMobIndex->damroll;
    mob->race			= pMobIndex->race;
    mob->class			= pMobIndex->class;
    mob->xflags			= pMobIndex->xflags;
    mob->saving_poison_death	= pMobIndex->saving_poison_death;
    mob->saving_wand		= pMobIndex->saving_wand;
    mob->saving_para_petri	= pMobIndex->saving_para_petri;
    mob->saving_breath		= pMobIndex->saving_breath;
    mob->saving_spell_staff	= pMobIndex->saving_spell_staff;
    mob->height			= pMobIndex->height;
    mob->weight			= pMobIndex->weight;
    mob->resistant		= pMobIndex->resistant;
    mob->immune			= pMobIndex->immune;
    mob->susceptible		= pMobIndex->susceptible;
    mob->attacks		= pMobIndex->attacks;
    mob->defenses		= pMobIndex->defenses;
    mob->numattacks		= pMobIndex->numattacks;
    mob->speaks			= pMobIndex->speaks;
    mob->speaking		= pMobIndex->speaking;

  /*
   * Perhaps add this to the index later --Shaddai
   */
    xCLEAR_BITS(mob->no_affected_by);
    mob->no_resistant   	= 0;
    mob->no_immune      	= 0;
    mob->no_susceptible 	= 0;
    /*
     * Insert in list.
     */
    add_char( mob );
    pMobIndex->count++;
    nummobsloaded++;
    return mob;
}



/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object( OBJ_INDEX_DATA *pObjIndex, int level )
{
    OBJ_DATA *obj;

    if ( !pObjIndex )
    {
	bug( "Create_object: NULL pObjIndex." );
	exit( 1 );
    }

    CREATE( obj, OBJ_DATA, 1 );

    obj->pIndexData	= pObjIndex;
    obj->in_room	= NULL;
    obj->level		= level;
    obj->wear_loc	= -1;
    obj->count		= 1;
    cur_obj_serial = UMAX((cur_obj_serial + 1 ) & (BV30-1), 1);
    obj->serial = obj->pIndexData->serial = cur_obj_serial;

    obj->name		= QUICKLINK( pObjIndex->name 	 );
    obj->short_descr	= QUICKLINK( pObjIndex->short_descr );
    obj->description	= QUICKLINK( pObjIndex->description );
    obj->action_desc	= QUICKLINK( pObjIndex->action_desc );
    obj->owner		= STRALLOC ( "" );
    obj->item_type	= pObjIndex->item_type;
    obj->extra_flags	= pObjIndex->extra_flags;
    obj->wear_flags	= pObjIndex->wear_flags;
    obj->value[0]	= pObjIndex->value[0];
    obj->value[1]	= pObjIndex->value[1];
    obj->value[2]	= pObjIndex->value[2];
    obj->value[3]	= pObjIndex->value[3];
    obj->value[4]	= pObjIndex->value[4];
    obj->value[5]	= pObjIndex->value[5];
    obj->weight		= pObjIndex->weight;
    obj->cost		= pObjIndex->cost;
    /*
    obj->cost		= number_fuzzy( 10 )
			* number_fuzzy( level ) * number_fuzzy( level );
     */

    /*
     * Mess with object properties.
     */
    switch ( obj->item_type )
    {
    default:
	bug( "Read_object: vnum %d bad type.", pObjIndex->vnum );
	bug( "------------------------>  %d ", obj->item_type );
	break;

    case ITEM_LIGHT:
    case ITEM_TREASURE:
    case ITEM_FURNITURE:
    case ITEM_TRASH:
    case ITEM_CONTAINER:
    case ITEM_DRINK_CON:
    case ITEM_PUDDLE:
    case ITEM_KEY:
	case ITEM_HOUSEKEY:
    case ITEM_KEYRING:
    case ITEM_ODOR:
    case ITEM_CHANCE:
    case ITEM_PIECE:
	case ITEM_ABACUS:
	break;
    case ITEM_COOK:
    case ITEM_FOOD:
	/*
	 * optional food condition (rotting food)		-Thoric
	 * value1 is the max condition of the food
	 * value4 is the optional initial condition
	 */
	if ( obj->value[4] )
	  obj->timer = obj->value[4];
	else
	  obj->timer = obj->value[1];
	break;
    case ITEM_BOAT:
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
    case ITEM_FOUNTAIN:
    case ITEM_BLOOD:
    case ITEM_BLOODSTAIN:
    case ITEM_SCRAPS:
    case ITEM_PIPE:
    case ITEM_HERB_CON:
    case ITEM_HERB:
    case ITEM_INCENSE:
    case ITEM_FIRE:
    case ITEM_BOOK:
    case ITEM_SWITCH:
    case ITEM_LEVER:
    case ITEM_PULLCHAIN:
    case ITEM_BUTTON:
    case ITEM_DIAL:
    case ITEM_RUNE:
    case ITEM_RUNEPOUCH:
    case ITEM_MATCH:
    case ITEM_TRAP:
    case ITEM_MAP:
    case ITEM_PORTAL:
    case ITEM_PAPER:
    case ITEM_PEN:
    case ITEM_TINDER:
    case ITEM_LOCKPICK:
    case ITEM_SPIKE:
    case ITEM_DISEASE:
    case ITEM_OIL:
    case ITEM_FUEL:
    case ITEM_QUIVER:
    case ITEM_SHOVEL:
    case ITEM_JOURNAL:
	break;

    case ITEM_SALVE:
	obj->value[3]	= number_fuzzy( obj->value[3] );
	break;

    case ITEM_SCROLL:
	obj->value[0]	= number_fuzzy( obj->value[0] );
	break;

    case ITEM_WAND:
    case ITEM_STAFF:
	obj->value[0]	= number_fuzzy( obj->value[0] );
	obj->value[1]	= number_fuzzy( obj->value[1] );
	obj->value[2]	= obj->value[1];
	break;

    case ITEM_WEAPON:
    case ITEM_MISSILE_WEAPON:
    case ITEM_PROJECTILE:
	if ( obj->value[1] && obj->value[2] )
	   obj->value[2] *= obj->value[1];
	else
	{
	   obj->value[1] = number_fuzzy( number_fuzzy( 1 * level / 4 + 2 ) );
	   obj->value[2] = number_fuzzy( number_fuzzy( 3 * level / 4 + 6 ) );
	}
	if (obj->value[0] == 0)
	   obj->value[0] = INIT_WEAPON_CONDITION;
	break;

    case ITEM_ARMOR:
	if ( obj->value[0] == 0 )
	  obj->value[0]	= number_fuzzy( level / 4 + 2 );
	if (obj->value[1] == 0)
	    obj->value[1] = obj->value[0];
	break;

    case ITEM_POTION:
    case ITEM_PILL:
	obj->value[0]	= number_fuzzy( number_fuzzy( obj->value[0] ) );
	break;

    case ITEM_MONEY:
	obj->value[0]	= obj->cost;
	if ( obj->value[0] == 0 )
		obj->value[0] = 1;
	break;
    }

    LINK( obj, first_object, last_object, next, prev );
    ++pObjIndex->count;
    ++numobjsloaded;
    ++physicalobjects;

    return obj;
}


/*
 * Clear a new character.
 */
void clear_char( CHAR_DATA *ch )
{
    ch->editor			= NULL;
    ch->hunting			= NULL;
    ch->fearing			= NULL;
    ch->hating			= NULL;
    ch->name			= NULL;
    ch->short_descr		= NULL;
    ch->long_descr		= NULL;
    ch->description		= NULL;
    ch->next			= NULL;
    ch->prev			= NULL;
    ch->reply			= NULL;
    ch->retell			= NULL;
	ch->variables		= NULL;
    ch->first_carrying		= NULL;
    ch->last_carrying		= NULL;
    ch->next_in_room		= NULL;
    ch->prev_in_room		= NULL;
    ch->fighting		= NULL;
    ch->switched		= NULL;
    ch->first_affect		= NULL;
    ch->last_affect		= NULL;
    ch->prev_cmd		= NULL;    /* maps */
    ch->last_cmd		= NULL;
    ch->dest_buf		= NULL;
    ch->alloc_ptr		= NULL;
    ch->spare_ptr		= NULL;
    ch->mount			= NULL;
    ch->morph    		= NULL;
    xCLEAR_BITS(ch->affected_by);
    ch->logon			= current_time;
    ch->armor			= 100;
    ch->position		= POS_STANDING;
    ch->practice		= 0;
    ch->hit			= 20;
    ch->max_hit			= 20;
    ch->mana			= 100;
    ch->max_mana		= 100;
    ch->move			= 100;
    ch->max_move		= 100;
    ch->height			= 72;
    ch->weight			= 180;
    ch->xflags			= 0;
    ch->race			= 0;
    ch->class			= 3;
    ch->speaking		= LANG_COMMON;
    ch->speaks			= LANG_COMMON;
    ch->barenumdie		= 1;
    ch->baresizedie		= 4;
    ch->substate		= 0;
    ch->tempnum			= 0;
    ch->perm_str		= 13;
    ch->perm_dex		= 13;
    ch->perm_int		= 13;
    ch->perm_wis		= 13;
    ch->perm_cha		= 13;
    ch->perm_con		= 13;
    ch->perm_lck		= 13;
    ch->mod_str			= 0;
    ch->mod_dex			= 0;
    ch->mod_int			= 0;
    ch->mod_wis			= 0;
    ch->mod_cha			= 0;
    ch->mod_con			= 0;
    ch->mod_lck			= 0;
    ch->pagelen                 = 24; 		     /* BUILD INTERFACE */
    ch->inter_page 		= NO_PAGE;           /* BUILD INTERFACE */
    ch->inter_type 		= NO_TYPE;           /* BUILD INTERFACE */
    ch->inter_editing    	= NULL;              /* BUILD INTERFACE */
    ch->inter_editing_vnum	= -1;                /* BUILD INTERFACE */
    ch->inter_substate    	= SUB_NORTH;         /* BUILD INTERFACE */
    return;
}



/*
 * Free a character.
 */
void free_char( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    AFFECT_DATA *paf;
    TIMER *timer;
    MPROG_ACT_LIST *mpact, *mpact_next;
    NOTE_DATA *comments, *comments_next;
    VARIABLE_DATA *vd, *vd_next;

    if ( !ch )
    {
      bug( "Free_char: null ch!" );
      return;
    }

    if ( ch->desc )
      bug( "Free_char: char still has descriptor." );

    if ( ch->morph )
    	free_char_morph( ch->morph );

    while ( (obj = ch->last_carrying) != NULL )
	extract_obj( obj );

    while ( (paf = ch->last_affect) != NULL )
	affect_remove( ch, paf );

    while ( (timer = ch->first_timer) != NULL )
	extract_timer( ch, timer );
	
    if ( ch->editor )
      stop_editing( ch );

    STRFREE( ch->name		);
    STRFREE( ch->short_descr	);
    STRFREE( ch->long_descr	);
    STRFREE( ch->description	);

    if ( ch->inter_editing )
      DISPOSE( ch->inter_editing );

    stop_hunting( ch );
    stop_hating ( ch );
    stop_fearing( ch );
    free_fight  ( ch );

    if ( ch->pnote )
	free_note( ch->pnote );

    if ( ch->pcdata )
    {
    	IGNORE_DATA *temp, *next;
    	
    	/* free up memory allocated to stored ignored names */
    	for(temp = ch->pcdata->first_ignored; temp; temp = next)
    	{
    		next = temp->next;
    		UNLINK(temp, ch->pcdata->first_ignored,
    			ch->pcdata->last_ignored, next, prev);
    		STRFREE(temp->name);
    		DISPOSE(temp);
    	}
    
	STRFREE( ch->pcdata->filename   );
	STRFREE( ch->pcdata->deity_name );
	STRFREE( ch->pcdata->clan_name	);
	STRFREE( ch->pcdata->council_name );
    if ( ch->pcdata->recent_site )
        STRFREE( ch->pcdata->recent_site );
    if ( ch->pcdata->prev_site )
        STRFREE( ch->pcdata->prev_site );
        DISPOSE( ch->pcdata->pwd	);  /* no hash */
	DISPOSE( ch->pcdata->bamfin	);  /* no hash */
	DISPOSE( ch->pcdata->bamfout	);  /* no hash */
	DISPOSE( ch->pcdata->rank	);
	STRFREE( ch->pcdata->title	);
	STRFREE( ch->pcdata->bio	); 
        if ( ch->pcdata->rreply )
                DISPOSE( ch->pcdata->rreply ); /* no hash */
        if ( ch->pcdata->rreply_name )
                DISPOSE( ch->pcdata->rreply_name ); /* no hash */
	DISPOSE( ch->pcdata->bestowments ); /* no hash */
	DISPOSE( ch->pcdata->homepage	);  /* no hash */
	STRFREE( ch->pcdata->authed_by	);
	STRFREE( ch->pcdata->prompt	);
	STRFREE( ch->pcdata->fprompt	);
	if ( ch->pcdata->helled_by )
		STRFREE( ch->pcdata->helled_by );
	if ( ch->pcdata->subprompt )
	   STRFREE( ch->pcdata->subprompt );
	if(ch->pcdata->tell_history)
	{
		int i;
		for(i = 0; i< 26; i++)
		{
			if(ch->pcdata->tell_history[i])
				STRFREE(ch->pcdata->tell_history[i]);
		}
		DISPOSE(ch->pcdata->tell_history);
	}
	DISPOSE(ch->pcdata->ice_listen);
	DISPOSE(ch->pcdata->see_me);
	DISPOSE(ch->pcdata);
     }

    for ( mpact = ch->mpact; mpact; mpact = mpact_next )
    {
	mpact_next = mpact->next;
	DISPOSE( mpact->buf );
	DISPOSE( mpact	    );
    }

    for ( comments = ch->comments; comments; comments = comments_next )
    {
	comments_next = comments->next;
	STRFREE( comments->text    );
	STRFREE( comments->to_list );
	STRFREE( comments->subject );
	STRFREE( comments->sender  );
	STRFREE( comments->date    );
	DISPOSE( comments          );
    }
    for ( vd = ch->variables; vd; vd = vd_next )
    {
    vd_next = vd->next;
    delete_variable(vd);
    }
    DISPOSE( ch );
    return;
}



/*
 * Get an extra description from a list.
 */
char *get_extra_descr( const char *name, EXTRA_DESCR_DATA *ed )
{
    for ( ; ed; ed = ed->next )
	if ( is_name( name, ed->keyword ) )
	    return ed->description;

    return NULL;
}



/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index( int vnum )
{
    MOB_INDEX_DATA *pMobIndex;

    if ( vnum < 0 )
      vnum = 0;

    for ( pMobIndex  = mob_index_hash[vnum % MAX_KEY_HASH];
	  pMobIndex;
	  pMobIndex  = pMobIndex->next )
	if ( pMobIndex->vnum == vnum )
	    return pMobIndex;

    if ( fBootDb )
	bug( "Get_mob_index: bad vnum %d.", vnum );

    return NULL;
}



/*
 * Translates obj virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( int vnum )
{
    OBJ_INDEX_DATA *pObjIndex;

    if ( vnum < 0 )
      vnum = 0;
    
    for ( pObjIndex  = obj_index_hash[vnum % MAX_KEY_HASH];
	  pObjIndex;
	  pObjIndex  = pObjIndex->next )
	if ( pObjIndex->vnum == vnum )
	    return pObjIndex;

    if ( fBootDb )
	bug( "Get_obj_index: bad vnum %d.", vnum );

    return NULL;
}



/*
 * Translates room virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index( int vnum )
{
    ROOM_INDEX_DATA *pRoomIndex;

    if ( vnum < 0 )
      vnum = 0;
    
    for ( pRoomIndex  = room_index_hash[vnum % MAX_KEY_HASH];
	  pRoomIndex;
	  pRoomIndex  = pRoomIndex->next )
	if ( pRoomIndex->vnum == vnum )
	    return pRoomIndex;

    if ( fBootDb )
	bug( "Get_room_index: bad vnum %d.", vnum );

    return NULL;
}



/*
 * Added lots of EOF checks, as most of the file crashes are based on them.
 * If an area file encounters EOF, the fread_* functions will shutdown the
 * MUD, as all area files should be read in in full or bad things will
 * happen during the game.  Any files loaded in without fBootDb which
 * encounter EOF will return what they have read so far.   These files
 * should include player files, and in-progress areas that are not loaded
 * upon bootup.
 * -- Altrag
 */


/*
 * Read a letter from a file.
 */
char fread_letter( FILE *fp )
{
    char c;

    do
    {
        if ( feof(fp) )
        {
          bug("fread_letter: EOF encountered on read.\n\r");
          if ( fBootDb )
            exit(1);
          return '\0';
        }
	c = getc( fp );
    }
    while ( isspace(c) );

    return c;
}



/*
 * Read a number from a file.
 */
int fread_number( FILE *fp )
{
    int number;
    bool sign;
    char c;

    do
    {
        if ( feof(fp) )
        {
          bug("fread_number: EOF encountered on read.\n\r");
          if ( fBootDb )
            exit(1);
          return 0;
        }
	c = getc( fp );
    }
    while ( isspace(c) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
    {
	c = getc( fp );
    }
    else if ( c == '-' )
    {
	sign = TRUE;
	c = getc( fp );
    }

    if ( !isdigit(c) )
    {
	bug( "Fread_number: bad format. (%c)", c );
	if ( fBootDb )
	  exit( 1 );
	return 0;
    }

    while ( isdigit(c) )
    {
        if ( feof(fp) )
        {
          bug("fread_number: EOF encountered on read.\n\r");
          if ( fBootDb )
            exit(1);
          return number;
        }
	number = number * 10 + c - '0';
	c      = getc( fp );
    }

    if ( sign )
	number = 0 - number;

    if ( c == '|' )
	number += fread_number( fp );
    else if ( c != ' ' )
	ungetc( c, fp );

    return number;
}

/*
 * custom str_dup using create					-Thoric
 */
char *str_dup( char const *str )
{
    static char *ret;
    int len;

    if ( !str )
	return NULL;
    
    len = strlen(str)+1;

    CREATE( ret, char, len );
    strcpy( ret, str );
    return ret;
}

/*
 * Read a string from file fp
 */
char *fread_string( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;
    char c;
    int ln;

    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	if ( feof(fp) )
	{
	    bug("fread_string: EOF encountered on read.\n\r");
	    if ( fBootDb )
		exit(1);
	    return STRALLOC("");
	}
	c = getc( fp );
    }
    while ( isspace(c) );

    if ( ( *plast++ = c ) == '~' )
	return STRALLOC( "" );

    for ( ;; )
    {
	if ( ln >= (MAX_STRING_LENGTH - 1) )
	{
	     bug( "fread_string: string too long" );
	     *plast = '\0';
	     return STRALLOC( buf );
	}
	switch ( *plast = getc( fp ) )
	{
	default:
	    plast++; ln++;
	    break;

	case EOF:
	    bug( "Fread_string: EOF" );
	    if ( fBootDb )
	      exit( 1 );
	    *plast = '\0';
	    return STRALLOC(buf);
	    break;

	case '\n':
	    plast++;  ln++;
	    *plast++ = '\r';  ln++;
	    break;

	case '\r':
	    break;

	case '~':
	    *plast = '\0';
	    return STRALLOC( buf );
	}
    }
}

/*
 * Read a string from file fp using str_dup (ie: no string hashing)
 */
char *fread_string_nohash( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;
    char c;
    int ln;

    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	if ( feof(fp) )
	{
	    bug("fread_string_no_hash: EOF encountered on read.\n\r");
	    if ( fBootDb )
		exit(1);
	    return str_dup("");
	}
	c = getc( fp );
    }
    while ( isspace(c) );

    if ( ( *plast++ = c ) == '~' )
	return str_dup( "" );

    for ( ;; )
    {
	if ( ln >= (MAX_STRING_LENGTH - 1) )
	{
	   bug( "fread_string_no_hash: string too long" );
	   *plast = '\0';
	   return str_dup( buf );
	}
	switch ( *plast = getc( fp ) )
	{
	default:
	    plast++; ln++;
	    break;

	case EOF:
	    bug( "Fread_string_no_hash: EOF" );
	    if ( fBootDb )
	      exit( 1 );
	    *plast = '\0';
	    return str_dup(buf);
	    break;

	case '\n':
	    plast++;  ln++;
	    *plast++ = '\r';  ln++;
	    break;

	case '\r':
	    break;

	case '~':
	    *plast = '\0';
	    return str_dup( buf );
	}
    }
}



/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE *fp )
{
    char c;

    do
    {
	if ( feof(fp) )
	{
	    bug("fread_to_eol: EOF encountered on read.\n\r");
	    if ( fBootDb )
		exit(1);
	    return;
	}
	c = getc( fp );
    }
    while ( c != '\n' && c != '\r' );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return;
}

/*
 * Read to end of line into static buffer			-Thoric
 */
char *fread_line( FILE *fp )
{
    static char line[MAX_STRING_LENGTH];
    char *pline;
    char c;
    int ln;

    pline = line;
    line[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	if ( feof(fp) )
	{
	    bug("fread_line: EOF encountered on read.\n\r");
	    if ( fBootDb )
		exit(1);
	    strcpy(line, "");
	    return line;
	}
	c = getc( fp );
    }
    while ( isspace(c) );

    ungetc( c, fp );
    do
    {
	if ( feof(fp) )
	{
	    bug("fread_line: EOF encountered on read.\n\r");
	    if ( fBootDb )
		exit(1);
	    *pline = '\0';
	    return line;
	}
	c = getc( fp );
	*pline++ = c; ln++;
	if ( ln >= (MAX_STRING_LENGTH - 1) )
	{
	    bug( "fread_line: line too long" );
	    break;
	}
    }
    while ( c != '\n' && c != '\r' );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    *pline = '\0';
    return line;
}



/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp )
{
    static char word[MAX_INPUT_LENGTH];
    char *pword;
    char cEnd;

    do
    {
	if ( feof(fp) )
	{
	    bug("fread_word: EOF encountered on read.\n\r");
	    if ( fBootDb )
		exit(1);
	    word[0] = '\0';
	    return word;
	}
	cEnd = getc( fp );
    }
    while ( isspace( cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
    {
	pword   = word;
    }
    else
    {
	word[0] = cEnd;
	pword   = word+1;
	cEnd    = ' ';
    }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
	if ( feof(fp) )
	{
	    bug("fread_word: EOF encountered on read.\n\r");
	    if ( fBootDb )
		exit(1);
	    *pword = '\0';
	    return word;
	}
	*pword = getc( fp );
	if ( cEnd == ' ' ? isspace(*pword) : *pword == cEnd )
	{
	    if ( cEnd == ' ' )
		ungetc( *pword, fp );
	    *pword = '\0';
	    return word;
	}
    }

    bug( "Fread_word: word too long" );

    if ( !loading_char )
	exit(1);
    return NULL;
}

void do_memory( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int hash;
 
    set_char_color( AT_PLAIN, ch );
    argument = one_argument( argument, arg );
    send_to_char_color( "\n\r&wSystem Memory [arguments - hash, check, showhigh]\n\r", ch );
    ch_printf_color( ch, "&wAffects: &W%5d\t\t\t&wAreas:   &W%5d\n\r",
        top_affect, top_area   );
    ch_printf_color( ch, "&wExtDes:  &W%5d\t\t\t&wExits:   &W%5d\n\r",
        top_ed,     top_exit   );
    ch_printf_color( ch, "&wHelps:   &W%5d\t\t\t&wResets:  &W%5d\n\r",
        top_help,   top_reset  );
    ch_printf_color( ch, "&wIdxMobs: &W%5d\t\t\t&wMobiles: &W%5d\n\r",
        top_mob_index, nummobsloaded );
    ch_printf_color( ch, "&wIdxObjs: &W%5d\t\t\t&wObjs:    &W%5d(%d)\n\r",
        top_obj_index, numobjsloaded, physicalobjects );
    ch_printf_color( ch, "&wRooms:   &W%5d\t\t\t&wVRooms:  &W%5d\n\r",
        top_room,   top_vroom   );
    ch_printf_color( ch, "&wShops:   &W%5d\t\t\t&wRepShps: &W%5d\n\r",
        top_shop,   top_repair );
    ch_printf_color( ch, "&wCurOq's: &W%5d\t\t\t&wCurCq's: &W%5d\n\r",
        cur_qobjs,  cur_qchars );
    ch_printf_color( ch, "&wPlayers: &W%5d\t\t\t&wMaxplrs: &W%5d\n\r",
        num_descriptors, sysdata.maxplayers );
    ch_printf_color( ch, "&wMaxEver: &W%5d\t\t\t&wTopsn:   &W%5d(%d)\n\r",
 	sysdata.alltimemax, top_sn, MAX_SKILL );
    ch_printf_color( ch, "&wMaxEver was recorded on:  &W%s\n\r\n\r",
	sysdata.time_of_max );
    ch_printf_color( ch, "&wPotion Val:  &W%-16d   &wScribe/Brew: &W%d/%d\n\r",
	sysdata.upotion_val, sysdata.scribed_used, sysdata.brewed_used );
    ch_printf_color( ch, "&wPill Val:    &W%-16d   &wGlobal loot: &W%d\n\r",
        sysdata.upill_val, sysdata.global_looted );

 
    if ( !str_cmp( arg, "check" ) )
    {
#ifdef HASHSTR
        send_to_char( check_hash(argument), ch );
#else
        send_to_char( "Hash strings not enabled.\n\r", ch );
#endif
        return;
    }
    if ( !str_cmp( arg, "showhigh" ) )
    {
#ifdef HASHSTR
        show_high_hash( atoi(argument) );
#else
        send_to_char( "Hash strings not enabled.\n\r", ch );
#endif
        return;
    }
    if ( argument[0] != '\0' )
      hash = atoi(argument);
    else
      hash = -1;
    if ( !str_cmp( arg, "hash" ) )
    {
#ifdef HASHSTR
        ch_printf( ch, "Hash statistics:\n\r%s", hash_stats() );
        if ( hash != -1 )
          hash_dump( hash );
#else
        send_to_char( "Hash strings not enabled.\n\r", ch );
#endif
    }
    return;
}

/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy( int number )
{
    switch ( number_bits( 2 ) )
    {
	case 0:  number -= 1; break;
	case 3:  number += 1; break;
    }

    return UMAX( 1, number );
}



/*
 * Generate a random number.
 * Ooops was (number_mm() % to) + from which doesn't work -Shaddai
 */
int number_range( int from, int to )
{
    if ( (to-from) < 1 )
            return from;
    return ((number_mm() % (to-from+1)) + from);
}



/*
 * Generate a percentile roll.
 * number_mm() % 100 only does 0-99, changed to do 1-100 -Shaddai
 */
int number_percent( void )
{
    return (number_mm() % 100)+1;
}




/*
 * Generate a random door.
 */
int number_door( void )
{
    int door;

    while ( ( door = number_mm( ) & (16-1) ) > 9 )
	;

    return door;
/*    return number_mm() & 10; */
}



int number_bits( int width )
{
    return number_mm( ) & ( ( 1 << width ) - 1 );
}



/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */
static	int	rgiState[2+55];

void init_mm( )
{
    int *piState;
    int iState;

    piState	= &rgiState[2];

    piState[-2]	= 55 - 55;
    piState[-1]	= 55 - 24;

    piState[0]	= ((int) current_time) & ((1 << 30) - 1);
    piState[1]	= 1;
    for ( iState = 2; iState < 55; iState++ )
    {
	piState[iState] = (piState[iState-1] + piState[iState-2])
			& ((1 << 30) - 1);
    }
    return;
}



int number_mm( void )
{
    int *piState;
    int iState1;
    int iState2;
    int iRand;

    piState		= &rgiState[2];
    iState1	 	= piState[-2];
    iState2	 	= piState[-1];
    iRand	 	= (piState[iState1] + piState[iState2])
			& ((1 << 30) - 1);
    piState[iState1]	= iRand;
    if ( ++iState1 == 55 )
	iState1 = 0;
    if ( ++iState2 == 55 )
	iState2 = 0;
    piState[-2]		= iState1;
    piState[-1]		= iState2;
    return iRand >> 6;
}



/*
 * Roll some dice.						-Thoric
 */
int dice( int number, int size )
{
    int idice;
    int sum;

    switch ( size )
    {
      case 0: return 0;
      case 1: return number;
    }

    for ( idice = 0, sum = 0; idice < number; idice++ )
	sum += number_range( 1, size );

    return sum;
}



/*
 * Simple linear interpolation.
 */
int interpolate( int level, int value_00, int value_32 )
{
    return value_00 + level * (value_32 - value_00) / 32;
}


char *smash_color( char *str )
{
    static char buf[MAX_STRING_LENGTH];
    char *bufptr;
    bool found;

    bufptr = buf; found = FALSE;

    for ( ; *str != '\0'; str++ )
    {
	if ( found )
	{
	   found = FALSE; continue;
	}
	if ( *str == '&' || *str == '^' )
	{
	   found = TRUE; continue;
	}
	  else *bufptr = *str;
	bufptr++;
    }

    *bufptr = '\0';
    return buf;
}

void smash_color_token( char *str )
{
    for ( ; *str != '\0'; str++ )
    {
	if ( *str == '^' )
	    *str = '-';
	if ( *str == '&' )
	    *str = '+';
    }
    return;
}

/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde( char *str )
{
    for ( ; *str != '\0'; str++ )
	if ( *str == '~' )
	    *str = '-';

    return;
}

/*
 * Encodes the tildes in a string.				-Thoric
 * Used for player-entered strings that go into disk files.
 */
void hide_tilde( char *str )
{
    for ( ; *str != '\0'; str++ )
	if ( *str == '~' )
	    *str = HIDDEN_TILDE;

    return;
}

char *show_tilde( char *str )
{
    static char buf[MAX_STRING_LENGTH];
    char *bufptr;

    bufptr = buf;
    for ( ; *str != '\0'; str++, bufptr++ )
    {
	if ( *str == HIDDEN_TILDE )
	    *bufptr = '~';
	else
	    *bufptr = *str;
    }
    *bufptr = '\0';

    return buf;
}



/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr )
{
    if ( !astr )
    {
	bug( "Str_cmp: null astr." );
	if ( bstr )
	  fprintf( stderr, "str_cmp: astr: (null)  bstr: %s\n", bstr );
	return TRUE;
    }

    if ( !bstr )
    {
	bug( "Str_cmp: null bstr." );
	if ( astr )
	  fprintf( stderr, "str_cmp: astr: %s  bstr: (null)\n", astr );
	return TRUE;
    }

    for ( ; *astr || *bstr; astr++, bstr++ )
    {
	if ( LOWER(*astr) != LOWER(*bstr) )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
    if ( !astr )
    {
	bug( "Strn_cmp: null astr." );
	return TRUE;
    }

    if ( !bstr )
    {
	bug( "Strn_cmp: null bstr." );
	return TRUE;
    }

    for ( ; *astr; astr++, bstr++ )
    {
	if ( LOWER(*astr) != LOWER(*bstr) )
	    return TRUE;
    }

    return FALSE;
}



/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;
    int ichar;
    char c0;

    if ( ( c0 = LOWER(astr[0]) ) == '\0' )
	return FALSE;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);

    for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
	if ( c0 == LOWER(bstr[ichar]) && !str_prefix( astr, bstr + ichar ) )
	    return FALSE;

    return TRUE;
}



/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);
    if ( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
	return FALSE;
    else
	return TRUE;
}



/*
 * Returns an initial-capped string.
 */
char *capitalize( const char *str )
{
    static char strcap[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
	strcap[i] = LOWER(str[i]);
    strcap[i] = '\0';
    strcap[0] = UPPER(strcap[0]);
    return strcap;
}


/*
 * Returns a lowercase string.
 */
char *strlower( const char *str )
{
    static char strlow[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
	strlow[i] = LOWER(str[i]);
    strlow[i] = '\0';
    return strlow;
}

/*
 * Returns an uppercase string.
 */
char *strupper( const char *str )
{
    static char strup[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
	strup[i] = UPPER(str[i]);
    strup[i] = '\0';
    return strup;
}

/*
 * Returns TRUE or FALSE if a letter is a vowel			-Thoric
 */
bool isavowel( char letter )
{
    char c;

    c = LOWER( letter );
    if ( c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' )
      return TRUE;
    else
      return FALSE;
}

/*
 * Shove either "a " or "an " onto the beginning of a string	-Thoric
 */
char *aoran( const char *str )
{
    static char temp[MAX_STRING_LENGTH];

    if ( !str )
    {
	bug( "Aoran(): NULL str" );
	return "";
    }

    if ( isavowel(str[0])
    || ( strlen(str) > 1 && LOWER(str[0]) == 'y' && !isavowel(str[1])) )
      strcpy( temp, "an " );
    else
      strcpy( temp, "a " );
    strcat( temp, str );
    return temp;
}


/*
 * Append a string to a file.
 */
void append_file( CHAR_DATA *ch, char *file, char *str )
{
    FILE *fp;

    if ( IS_NPC(ch) || str[0] == '\0' )
	return;

    fclose( fpLOG );
    if ( ( fp = fopen( file, "a" ) ) == NULL )
    {
	perror( file );
	send_to_char( "Could not open the file!\n\r", ch );
    }
    else
    {
	fprintf( fp, "[%5d] %s: %s\n",
	    ch->in_room ? ch->in_room->vnum : 0, ch->name, str );
	fclose( fp );
    }

    fpLOG = fopen( NULL_FILE, "r" );
    return;
}

/*
 * Append a string to a file.
 */
void append_to_file( char *file, char *str )
{
    FILE *fp;

    if ( ( fp = fopen( file, "a" ) ) == NULL )
      perror( file );
    else
    {
	fprintf( fp, "%s\n", str );
	fclose( fp );
    }

    return;
}


/*
 * Reports a bug.
 */
void bug( const char *str, ... )
{
    char buf[MAX_STRING_LENGTH];
    FILE *fp;
    struct stat fst;

    if ( fpArea != NULL )
    {
	int iLine;
	int iChar;

	if ( fpArea == stdin )
	{
	    iLine = 0;
	}
	else
	{
	    iChar = ftell( fpArea );
	    fseek( fpArea, 0, 0 );
	    for ( iLine = 0; ftell( fpArea ) < iChar; iLine++ )
	    {
		while ( getc( fpArea ) != '\n' )
		    ;
	    }
	    fseek( fpArea, iChar, 0 );
	}

	sprintf( buf, "[*****] FILE: %s LINE: %d", strArea, iLine );
	log_string_plus( buf, LOG_BUG, sysdata.log_level );
/*	log_string( buf ); */

	if ( stat( SHUTDOWN_FILE, &fst ) != -1 )	/* file exists */
	{
	    if ( ( fp = fopen( SHUTDOWN_FILE, "a" ) ) != NULL )
	    {
		fprintf( fp, "[*****] %s\n", buf );
		fclose( fp );
	    }
	}
    }

    strcpy( buf, "[*****] BUG: " );
    {
	va_list param;
    
	va_start(param, str);
	vsprintf( buf + strlen(buf), str, param );
	va_end(param);
    }
    log_string_plus( buf, LOG_BUG, sysdata.log_level );
    /*    log_string( buf ); */

    fclose( fpLOG );
    if ( ( fp = fopen( BUG_FILE, "a" ) ) != NULL )
    {
	fprintf( fp, "%s\n", buf );
	fclose( fp );
    }
    fpLOG = fopen( NULL_FILE, "r" );

    return;
}

/*
 * Add a string to the boot-up log				-Thoric
 */
void boot_log( const char *str, ... )
{
    char buf[MAX_STRING_LENGTH];
    FILE *fp;
    va_list param;

    strcpy( buf, "[*****] BOOT: " );
    va_start(param, str);
    vsprintf( buf+strlen(buf), str, param );
    va_end(param);
    log_string( buf );

    fclose( fpLOG );
    if ( ( fp = fopen( BOOTLOG_FILE, "a" ) ) != NULL )
    {
	fprintf( fp, "%s\n", buf );
 	fclose( fp );
    }
    fpLOG = fopen( NULL_FILE, "r" );

    return;
}

/*
 * Dump a text file to a player, a line at a time		-Thoric
 */
void show_file( CHAR_DATA *ch, char *filename )
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    int c;
    int num = 0;

    fclose( fpReserve );
    if ( (fp = fopen( filename, "r" )) != NULL )
    {
      while ( !feof(fp) )
      {
	while ((buf[num]=fgetc(fp)) != EOF
	&&      buf[num] != '\n'
	&&      buf[num] != '\r'
	&&      num < (MAX_STRING_LENGTH-2))
	  num++;
	c = fgetc(fp);
	if ( (c != '\n' && c != '\r') || c == buf[num] )
	  ungetc(c, fp);
	buf[num++] = '\n';
	buf[num++] = '\r';
	buf[num  ] = '\0';
	send_to_pager_color( buf, ch );
	num = 0;
      }
      /* Thanks to stu <sprice@ihug.co.nz> from the mailing list in pointing
       *  This out. */
      fclose(fp);
    }
    fpReserve = fopen( NULL_FILE, "r" );
}

/*
 * Dump a text file to a player, a line at a time		-Thoric
 * This version picks off the room vnum at each line to be used
 * against a range check.					-- Alty
 */
void show_file_vnum( CHAR_DATA *ch, char *filename, int lo, int hi )
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    int c;
    int num = 0;

    fclose( fpReserve );
    if ( (fp = fopen( filename, "r" )) != NULL )
    {
      while ( !feof(fp) )
      {
	while ((buf[num]=fgetc(fp)) != EOF
	&&      buf[num] != '\n'
	&&      buf[num] != '\r'
	&&      num < (MAX_STRING_LENGTH-2))
	  num++;
	c = fgetc(fp);
	if ( (c != '\n' && c != '\r') || c == buf[num] )
	  ungetc(c, fp);
	buf[num++] = '\n';
	buf[num++] = '\r';
	buf[num  ] = '\0';
	c = atoi(buf+1);
	if ((lo < 0 || c >= lo) && (hi < 0 || c <= hi))
	  send_to_pager_color( buf, ch );
	num = 0;
      }
      /* Thanks to stu <sprice@ihug.co.nz> from the mailing list in pointing
       *  This out. */
      fclose(fp);
    }
    fpReserve = fopen( NULL_FILE, "r" );
}

/*
 * Show the boot log file					-Thoric
 */
void do_dmesg( CHAR_DATA *ch, char *argument )
{
    set_pager_color( AT_LOG, ch );
    show_file( ch, BOOTLOG_FILE );
}

/*
 * Writes a string to the log, extended version			-Thoric
 */
void log_string_plus( const char *str, sh_int log_type, sh_int level )
{
    char *strtime;
    int offset;

    strtime                    = ctime( &current_time );
    strtime[strlen(strtime)-1] = '\0';
    fprintf( stderr, "%s :: %s\n", strtime, str );
    if ( strncmp( str, "Log ", 4 ) == 0 )
      offset = 4;
    else
      offset = 0;
    switch( log_type )
    {
	default:
	  to_channel( str + offset, CHANNEL_LOG, "Log", level );
	  break;
	case LOG_BUILD:
	  to_channel( str + offset, CHANNEL_BUILD, "Build", level );
	  break;
	case LOG_COMM:
	  to_channel( str + offset, CHANNEL_COMM, "Comm", level );
	  break;
        case LOG_WARN:
          to_channel (str + offset, CHANNEL_WARN, "Warn", level );
          break;
	case LOG_BUG:
	  to_channel (str + offset, CHANNEL_BUG, "Bug", level );
	  break;
	case LOG_ALL:
	  break;
    }
    return;
}

/*
 * wizlist builder!						-Thoric
 */

void toretiredfile( const char *line )
{
  int filler, xx;
  char outline[MAX_STRING_LENGTH];
  FILE *wfp;

  outline[0] = '\0';

  if ( line && line[0] != '\0' )
  {
   filler = ( 78-strlen( line ) );
   if ( filler < 1 )
     filler = 1;
   filler /= 2;
   for ( xx = 0; xx < filler; xx++ )
      strcat( outline, " " );
    strcat( outline, line );
  }
  strcat( outline, "\n\r" );
  wfp = fopen( RETIREDLIST_FILE, "a" );
  if ( wfp )
  {
    fputs( outline, wfp );
    fclose( wfp );
  }
}

void towizfile( const char *line )
{
  int filler, xx;
  char outline[MAX_STRING_LENGTH];
  FILE *wfp;

  outline[0] = '\0';

  if ( line && line[0] != '\0' )
  {
   filler = ( 78-strlen( line ) );
   if ( filler < 1 )
     filler = 1;
   filler /= 2;
   for ( xx = 0; xx < filler; xx++ )
      strcat( outline, " " );
    strcat( outline, line );
  }
  strcat( outline, "\n\r" );
  wfp = fopen( WIZLIST_FILE, "a" );
  if ( wfp )
  {
    fputs( outline, wfp );
    fclose( wfp );
  }
}

void add_to_wizlist( char *name, int level )
{
  WIZENT *wiz, *tmp;

#ifdef DEBUG
  log_string( "Adding to wizlist..." );
#endif

  CREATE( wiz, WIZENT, 1 );
  wiz->name	= str_dup( name );
  wiz->level	= level;

  if ( !first_wiz )
  {
    wiz->last	= NULL;
    wiz->next	= NULL;
    first_wiz	= wiz;
    last_wiz	= wiz;
    return;
  }

  /* insert sort, of sorts */
  for ( tmp = first_wiz; tmp; tmp = tmp->next )
    if ( level > tmp->level )
    {
      if ( !tmp->last )
	first_wiz	= wiz;
      else
	tmp->last->next = wiz;
      wiz->last = tmp->last;
      wiz->next = tmp;
      tmp->last = wiz;
      return;
    }

  wiz->last		= last_wiz;
  wiz->next		= NULL;
  last_wiz->next	= wiz;
  last_wiz		= wiz;
  return;
}

void make_retiredlist( )
{
  DIR *dp;
  struct dirent *dentry;
  FILE *gfp;
  char *word=NULL;
  int ilevel, iflags;
  WIZENT *wiz, *wiznext;
  char buf[MAX_STRING_LENGTH];

  first_wiz = NULL;
  last_wiz  = NULL;

  dp = opendir( GOD_DIR );

  ilevel = 0;
  dentry = readdir( dp );
  while ( dentry )
  {
      if ( dentry->d_name[0] != '.' )
      {
	sprintf( buf, "%s%s", GOD_DIR, dentry->d_name );
	gfp = fopen( buf, "r" );
	if ( gfp )
	{
	  word = feof( gfp ) ? "End" : fread_word( gfp );
	  ilevel = fread_number( gfp );
          fread_to_eol( gfp );
	  word = feof( gfp ) ? "End" : fread_word( gfp );
          if ( !str_cmp( word, "Pcflags" ) )
	    iflags = fread_number( gfp );
          else
	    iflags = 0;
	  fclose( gfp );
          if ( IS_SET( iflags, PCFLAG_RETIRED ) )
	     add_to_wizlist( dentry->d_name, 50 );
	}
      }
      dentry = readdir( dp );
  }
  closedir( dp );

  unlink( RETIREDLIST_FILE );
  sprintf(buf, " Retired Immortals of the %s!", sysdata.mud_name);
  toretiredfile(buf);
  buf[0] = '\0';
  ilevel = 65535;
  for ( wiz = first_wiz; wiz; wiz = wiz->next )
  {
    if ( buf[0] )
    {
    	toretiredfile( buf );
	buf[0] = '\0';
    }
    toretiredfile( "" );
    if ( strlen( buf ) + strlen( wiz->name ) > 76 )
    {
	toretiredfile( buf );
	buf[0] = '\0';
    }
    strcat( buf, " " );
    strcat( buf, wiz->name );
    if ( strlen( buf ) > 70 )
    {
      toretiredfile( buf );
      buf[0] = '\0';
    }
  }

  if ( buf[0] )
    toretiredfile( buf );

  for ( wiz = first_wiz; wiz; wiz = wiznext )
  {
    wiznext = wiz->next;
    DISPOSE(wiz->name);
    DISPOSE(wiz);
  }
  first_wiz = NULL;
  last_wiz = NULL;
}

void do_makeretiredlist( CHAR_DATA *ch, char *argument )
{
  make_retiredlist();
}

/*
 * Wizlist builder						-Thoric
 */
void make_wizlist( )
{
  DIR *dp;
  struct dirent *dentry;
  FILE *gfp;
  char *word;
  int ilevel, iflags;
  WIZENT *wiz, *wiznext;
  char buf[MAX_STRING_LENGTH];

  first_wiz = NULL;
  last_wiz  = NULL;

  dp = opendir( GOD_DIR );

  ilevel = 0;
  dentry = readdir( dp );
  while ( dentry )
  {
      if ( dentry->d_name[0] != '.' )
      {
	sprintf( buf, "%s%s", GOD_DIR, dentry->d_name );
	gfp = fopen( buf, "r" );
	if ( gfp )
	{
	  word = feof( gfp ) ? "End" : fread_word( gfp );
	  ilevel = fread_number( gfp );
          fread_to_eol( gfp );
	  word = feof( gfp ) ? "End" : fread_word( gfp );
          if ( !str_cmp( word, "Pcflags" ) )
	    iflags = fread_number( gfp );
          else
	    iflags = 0;
	  fclose( gfp );
          if ( IS_SET( iflags, PCFLAG_GUEST ) )
            ilevel = MAX_LEVEL - 16;
	  if ( !IS_SET( iflags, PCFLAG_RETIRED) )
	    add_to_wizlist( dentry->d_name, ilevel );
	}
      }
      dentry = readdir( dp );
  }
  closedir( dp );

  unlink( WIZLIST_FILE );
  sprintf(buf, "   &GAdministrators of the %s!", sysdata.mud_name);
  towizfile(buf);
/*  towizfile( " Masters of the Realms of Despair!" );*/
  buf[0] = '\0';
  ilevel = 65535;
  for ( wiz = first_wiz; wiz; wiz = wiz->next )
  {
    if ( wiz->level < ilevel )
    {
      if ( buf[0] )
      {
	towizfile( buf );
	buf[0] = '\0';
      }
      towizfile( "" );
      ilevel = wiz->level;
      switch(ilevel)
      {
	case MAX_LEVEL -  0: towizfile( "     &GSupreme Entity&W" );	break;
        case MAX_LEVEL -  1: towizfile( "     &GInfinite&W" );     	break;
        case MAX_LEVEL -  2: towizfile( "     &GEternal&W" );      	break;
        case MAX_LEVEL -  3: towizfile( "     &GAncient&W" );      	break;
        case MAX_LEVEL -  4: towizfile( "     &GExalted Gods&W" ); 	break;
        case MAX_LEVEL -  5: towizfile( "     &GAscendant Gods&W" );	break;
        case MAX_LEVEL -  6: towizfile( "     &GGreater Gods&W" );	break;
        case MAX_LEVEL -  7: towizfile( "     &GGods&W" );		break;
        case MAX_LEVEL -  8: towizfile( "     &GLesser Gods&W" );	break;
        case MAX_LEVEL -  9: towizfile( "     &GImmortals&W" );		break;
        case MAX_LEVEL - 10: towizfile( "     &GDemi Gods&W" );		break;
        case MAX_LEVEL - 11: towizfile( "     &GSaviors&W" );		break;
        case MAX_LEVEL - 12: towizfile( "     &GCreators&W" );     	break;
        case MAX_LEVEL - 13: towizfile( "     &GAcolytes&W" );     	break;
        case MAX_LEVEL - 14: towizfile( "     &GNeophytes&W" );    	break;
        case MAX_LEVEL - 16: towizfile( "     &GGuests&W" );            break;
        default:             towizfile( "     &GServants&W" );          break;
      }
    }
    if ( strlen( buf ) + strlen( wiz->name ) > 76 )
    {
	towizfile( buf );
	buf[0] = '\0';
    }
    strcat( buf, " " );
    strcat( buf, wiz->name );
    if ( strlen( buf ) > 70 )
    {
      towizfile( buf );
      buf[0] = '\0';
    }
  }

  if ( buf[0] )
    towizfile( buf );

  for ( wiz = first_wiz; wiz; wiz = wiznext )
  {
    wiznext = wiz->next;
    DISPOSE(wiz->name);
    DISPOSE(wiz);
  }
  first_wiz = NULL;
  last_wiz = NULL;
}


void do_makewizlist( CHAR_DATA *ch, char *argument )
{
  make_wizlist();
}


/* mud prog functions */

/* This routine reads in scripts of MUDprograms from a file */

int mprog_name_to_type ( char *name )
{
   if ( !str_cmp( name, "in_file_prog"   ) )	return IN_FILE_PROG;
   if ( !str_cmp( name, "act_prog"       ) )    return ACT_PROG;
   if ( !str_cmp( name, "speech_prog"    ) )	return SPEECH_PROG;
   if ( !str_cmp( name, "rand_prog"      ) ) 	return RAND_PROG;
   if ( !str_cmp( name, "fight_prog"     ) )	return FIGHT_PROG;
   if ( !str_cmp( name, "hitprcnt_prog"  ) )	return HITPRCNT_PROG;
   if ( !str_cmp( name, "death_prog"     ) )	return DEATH_PROG;
   if ( !str_cmp( name, "entry_prog"     ) )	return ENTRY_PROG;
   if ( !str_cmp( name, "greet_prog"     ) )	return GREET_PROG;
   if ( !str_cmp( name, "all_greet_prog" ) )	return ALL_GREET_PROG;
   if ( !str_cmp( name, "give_prog"      ) ) 	return GIVE_PROG;
   if ( !str_cmp( name, "bribe_prog"     ) )	return BRIBE_PROG;
   if ( !str_cmp( name, "time_prog"     ) )	return TIME_PROG;
   if ( !str_cmp( name, "hour_prog"     ) )	return HOUR_PROG;
   if ( !str_cmp( name, "wear_prog"     ) )	return WEAR_PROG;
   if ( !str_cmp( name, "remove_prog"   ) )	return REMOVE_PROG;
   if ( !str_cmp( name, "sac_prog"      ) )	return SAC_PROG;
   if ( !str_cmp( name, "look_prog"     ) )	return LOOK_PROG;
   if ( !str_cmp( name, "exa_prog"      ) )	return EXA_PROG;
   if ( !str_cmp( name, "zap_prog"      ) )	return ZAP_PROG;
   if ( !str_cmp( name, "get_prog"      ) ) 	return GET_PROG;
   if ( !str_cmp( name, "drop_prog"     ) )	return DROP_PROG;
   if ( !str_cmp( name, "damage_prog"   ) )	return DAMAGE_PROG;
   if ( !str_cmp( name, "repair_prog"   ) )	return REPAIR_PROG;
   if ( !str_cmp( name, "greet_prog"    ) )	return GREET_PROG;
   if ( !str_cmp( name, "randiw_prog"   ) )	return RANDIW_PROG;
   if ( !str_cmp( name, "speechiw_prog" ) )	return SPEECHIW_PROG;
   if ( !str_cmp( name, "pull_prog"	) )     return PULL_PROG;
   if ( !str_cmp( name, "push_prog"	) )     return PUSH_PROG;
   if ( !str_cmp( name, "sleep_prog"    ) )	return SLEEP_PROG;
   if ( !str_cmp( name, "rest_prog"	) )	return REST_PROG;
   if ( !str_cmp( name, "rfight_prog"   ) )	return FIGHT_PROG;
   if ( !str_cmp( name, "enter_prog"    ) )	return ENTRY_PROG;
   if ( !str_cmp( name, "login_prog"    ) )     return LOGIN_PROG;
   if ( !str_cmp( name, "void_prog"    ) )      return VOID_PROG;
   if ( !str_cmp( name, "leave_prog"    ) )	return LEAVE_PROG;
   if ( !str_cmp( name, "rdeath_prog"	) )	return DEATH_PROG;
   if ( !str_cmp( name, "script_prog"	) )	return SCRIPT_PROG;
   if ( !str_cmp( name, "use_prog"	) )	return USE_PROG;
   if ( !str_cmp( name, "load_prog"	) )	return LOAD_PROG;
   if ( !str_cmp( name, "imminfo_prog" ) )	return IMMINFO_PROG;
   if ( !str_cmp( name, "cmd_prog" ) )		return CMD_PROG;
   if ( !str_cmp( name, "sell_prog" ) )		return SELL_PROG;
   if ( !str_cmp( name, "tell_prog" ) )		return TELL_PROG;
   if ( !str_cmp( name, "greet_in_fight_prog" ) ) return GREET_IN_FIGHT_PROG;
   return( ERROR_PROG );
}

MPROG_DATA *mprog_file_read( char *f, MPROG_DATA *mprg,
			    MOB_INDEX_DATA *pMobIndex )
{

  char        MUDProgfile[ MAX_INPUT_LENGTH ];
  FILE       *progfile;
  char        letter;
  MPROG_DATA *mprg_next, *mprg2;
  bool        done = FALSE;

  sprintf( MUDProgfile, "%s%s", PROG_DIR, f );

  progfile = fopen( MUDProgfile, "r" );
  if ( !progfile )
  {
     bug( "Mob: %d couldn't open mudprog file", pMobIndex->vnum );
     exit( 1 );
  }

  mprg2 = mprg;
  switch ( letter = fread_letter( progfile ) )
  {
    case '>':
     break;
    case '|':
       bug( "empty mudprog file." );
       exit( 1 );
     break;
    default:
       bug( "in mudprog file syntax error." );
       exit( 1 );
     break;
  }

  while ( !done )
  {
    mprg2->type = mprog_name_to_type( fread_word( progfile ) );
    switch ( mprg2->type )
    {
     case ERROR_PROG:
	bug( "mudprog file type error" );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	bug( "mprog file contains a call to file." );
	exit( 1 );
      break;
     default:
	xSET_BIT(pMobIndex->progtypes, mprg2->type);
	mprg2->arglist       = fread_string( progfile );
	mprg2->comlist       = fread_string( progfile );
	switch ( letter = fread_letter( progfile ) )
	{
	  case '>':
	     CREATE( mprg_next, MPROG_DATA, 1 );
	     mprg_next->next = mprg2;
	     mprg2 = mprg_next;
	   break;
	  case '|':
	     done = TRUE;
	   break;
	  default:
	     bug( "in mudprog file syntax error." );
	     exit( 1 );
	   break;
	}
      break;
    }
  }
  fclose( progfile );
  return mprg2;
}

/* Load a MUDprogram section from the area file.
 */
void load_mudprogs( AREA_DATA *tarea, FILE *fp )
{
  MOB_INDEX_DATA *iMob;
  MPROG_DATA     *original;
  MPROG_DATA     *working;
  char            letter;
  int             value;

  for ( ; ; )
    switch ( letter = fread_letter( fp ) )
    {
    default:
      bug( "Load_mudprogs: bad command '%c'.",letter);
      exit(1);
      break;
    case 'S':
    case 's':
      fread_to_eol( fp );
      return;
    case '*':
      fread_to_eol( fp );
      break;
    case 'M':
    case 'm':
      value = fread_number( fp );
      if ( ( iMob = get_mob_index( value ) ) == NULL )
      {
	bug( "Load_mudprogs: vnum %d doesnt exist", value );
	exit( 1 );
      }

      /* Go to the end of the prog command list if other commands
	 exist */

      if ( (original = iMob->mudprogs) != NULL )
	for ( ; original->next; original = original->next );

      CREATE( working, MPROG_DATA, 1 );
      if ( original )
	original->next = working;
      else
	iMob->mudprogs = working;
      working = mprog_file_read( fread_word( fp ), working, iMob );
      working->next = NULL;
      fread_to_eol( fp );
      break;
    }

  return;

}

/* This procedure is responsible for reading any in_file MUDprograms.
 */

void mprog_read_programs( FILE *fp, MOB_INDEX_DATA *pMobIndex)
{
  MPROG_DATA *mprg;
  char        letter;
  bool        done = FALSE;

  if ( ( letter = fread_letter( fp ) ) != '>' )
  {
      bug( "Load_mobiles: vnum %d MUDPROG char", pMobIndex->vnum );
      exit( 1 );
  }
  CREATE( mprg, MPROG_DATA, 1 );
  pMobIndex->mudprogs = mprg;

  while ( !done )
  {
    mprg->type = mprog_name_to_type( fread_word( fp ) );
    switch ( mprg->type )
    {
     case ERROR_PROG:
	bug( "Load_mobiles: vnum %d MUDPROG type.", pMobIndex->vnum );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	mprg = mprog_file_read( fread_string( fp ), mprg,pMobIndex );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_mobiles: vnum %d bad MUDPROG.", pMobIndex->vnum );
	     exit( 1 );
	   break;
	}
      break;
     default:
	xSET_BIT(pMobIndex->progtypes, mprg->type);
	mprg->arglist        = fread_string( fp );
	fread_to_eol( fp );
	mprg->comlist        = fread_string( fp );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_mobiles: vnum %d bad MUDPROG.", pMobIndex->vnum );
	     exit( 1 );
	   break;
	}
      break;
    }
  }

  return;

}



/*************************************************************/
/* obj prog functions */
/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

/* This routine reads in scripts of OBJprograms from a file */


MPROG_DATA *oprog_file_read( char *f, MPROG_DATA *mprg,
			    OBJ_INDEX_DATA *pObjIndex )
{

  char        MUDProgfile[ MAX_INPUT_LENGTH ];
  FILE       *progfile;
  char        letter;
  MPROG_DATA *mprg_next, *mprg2;
  bool        done = FALSE;

  sprintf( MUDProgfile, "%s%s", PROG_DIR, f );

  progfile = fopen( MUDProgfile, "r" );
  if ( !progfile )
  {
     bug( "Obj: %d couldnt open mudprog file", pObjIndex->vnum );
     exit( 1 );
  }

  mprg2 = mprg;
  switch ( letter = fread_letter( progfile ) )
  {
    case '>':
     break;
    case '|':
       bug( "empty objprog file." );
       exit( 1 );
     break;
    default:
       bug( "in objprog file syntax error." );
       exit( 1 );
     break;
  }

  while ( !done )
  {
    mprg2->type = mprog_name_to_type( fread_word( progfile ) );
    switch ( mprg2->type )
    {
     case ERROR_PROG:
	bug( "objprog file type error" );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	bug( "objprog file contains a call to file." );
	exit( 1 );
      break;
     default:
	xSET_BIT(pObjIndex->progtypes, mprg2->type);
	mprg2->arglist       = fread_string( progfile );
	mprg2->comlist       = fread_string( progfile );
	switch ( letter = fread_letter( progfile ) )
	{
	  case '>':
	     CREATE( mprg_next, MPROG_DATA, 1 );
	     mprg_next->next = mprg2;
	     mprg2 = mprg_next;
	   break;
	  case '|':
	     done = TRUE;
	   break;
	  default:
	     bug( "in objprog file syntax error." );
	     exit( 1 );
	   break;
	}
      break;
    }
  }
  fclose( progfile );
  return mprg2;
}

/* Load a MUDprogram section from the area file.
 */
void load_objprogs( AREA_DATA *tarea, FILE *fp )
{
  OBJ_INDEX_DATA *iObj;
  MPROG_DATA     *original;
  MPROG_DATA     *working;
  char            letter;
  int             value;

  for ( ; ; )
    switch ( letter = fread_letter( fp ) )
    {
    default:
      bug( "Load_objprogs: bad command '%c'.",letter);
      exit(1);
      break;
    case 'S':
    case 's':
      fread_to_eol( fp );
      return;
    case '*':
      fread_to_eol( fp );
      break;
    case 'M':
    case 'm':
      value = fread_number( fp );
      if ( ( iObj = get_obj_index( value ) ) == NULL )
      {
	bug( "Load_objprogs: vnum %d doesnt exist", value );
	exit( 1 );
      }

      /* Go to the end of the prog command list if other commands
	 exist */

      if ( (original = iObj->mudprogs) != NULL )
	for ( ; original->next; original = original->next );

      CREATE( working, MPROG_DATA, 1 );
      if ( original )
	original->next = working;
      else
	iObj->mudprogs = working;
      working = oprog_file_read( fread_word( fp ), working, iObj );
      working->next = NULL;
      fread_to_eol( fp );
      break;
    }

  return;

}

/* This procedure is responsible for reading any in_file OBJprograms.
 */

void oprog_read_programs( FILE *fp, OBJ_INDEX_DATA *pObjIndex)
{
  MPROG_DATA *mprg;
  char        letter;
  bool        done = FALSE;

  if ( ( letter = fread_letter( fp ) ) != '>' )
  {
      bug( "Load_objects: vnum %d OBJPROG char", pObjIndex->vnum );
      exit( 1 );
  }
  CREATE( mprg, MPROG_DATA, 1 );
  pObjIndex->mudprogs = mprg;

  while ( !done )
  {
    mprg->type = mprog_name_to_type( fread_word( fp ) );
    switch ( mprg->type )
    {
     case ERROR_PROG:
	bug( "Load_objects: vnum %d OBJPROG type.", pObjIndex->vnum );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	mprg = oprog_file_read( fread_string( fp ), mprg,pObjIndex );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_objects: vnum %d bad OBJPROG.", pObjIndex->vnum );
	     exit( 1 );
	   break;
	}
      break;
     default:
	xSET_BIT(pObjIndex->progtypes, mprg->type);
	mprg->arglist        = fread_string( fp );
	fread_to_eol( fp );
	mprg->comlist        = fread_string( fp );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_objects: vnum %d bad OBJPROG.", pObjIndex->vnum );
	     exit( 1 );
	   break;
	}
      break;
    }
  }

  return;

}


/*************************************************************/
/* room prog functions */
/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

/* This routine reads in scripts of OBJprograms from a file */
MPROG_DATA *rprog_file_read( char *f, MPROG_DATA *mprg,
			    ROOM_INDEX_DATA *RoomIndex )
{

  char        MUDProgfile[ MAX_INPUT_LENGTH ];
  FILE       *progfile;
  char        letter;
  MPROG_DATA *mprg_next, *mprg2;
  bool        done = FALSE;

  sprintf( MUDProgfile, "%s%s", PROG_DIR, f );

  progfile = fopen( MUDProgfile, "r" );
  if ( !progfile )
  {
     bug( "Room: %d couldnt open roomprog file", RoomIndex->vnum );
     exit( 1 );
  }

  mprg2 = mprg;
  switch ( letter = fread_letter( progfile ) )
  {
    case '>':
     break;
    case '|':
       bug( "empty roomprog file." );
       exit( 1 );
     break;
    default:
       bug( "in roomprog file syntax error." );
       exit( 1 );
     break;
  }

  while ( !done )
  {
    mprg2->type = mprog_name_to_type( fread_word( progfile ) );
    switch ( mprg2->type )
    {
     case ERROR_PROG:
	bug( "roomprog file type error" );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	bug( "roomprog file contains a call to file." );
	exit( 1 );
      break;
     default:
	xSET_BIT(RoomIndex->progtypes, mprg2->type);
	mprg2->arglist       = fread_string( progfile );
	mprg2->comlist       = fread_string( progfile );
	switch ( letter = fread_letter( progfile ) )
	{
	  case '>':
	     CREATE( mprg_next, MPROG_DATA, 1 );
	     mprg_next->next = mprg2;
	     mprg2 = mprg_next;
	   break;
	  case '|':
	     done = TRUE;
	   break;
	  default:
	     bug( "in roomprog file syntax error." );
	     exit( 1 );
	   break;
	}
      break;
    }
  }
  fclose( progfile );
  return mprg2;
}

/* Load a ROOMprogram section from the area file.
 */
void load_roomprogs( AREA_DATA *tarea, FILE *fp )
{
  ROOM_INDEX_DATA *iRoom;
  MPROG_DATA     *original;
  MPROG_DATA     *working;
  char            letter;
  int             value;

  for ( ; ; )
    switch ( letter = fread_letter( fp ) )
    {
    default:
      bug( "Load_objprogs: bad command '%c'.",letter);
      exit(1);
      break;
    case 'S':
    case 's':
      fread_to_eol( fp );
      return;
    case '*':
      fread_to_eol( fp );
      break;
    case 'M':
    case 'm':
      value = fread_number( fp );
      if ( ( iRoom = get_room_index( value ) ) == NULL )
      {
	bug( "Load_roomprogs: vnum %d doesnt exist", value );
	exit( 1 );
      }

      /* Go to the end of the prog command list if other commands
	 exist */

      if ( (original = iRoom->mudprogs) != NULL )
	for ( ; original->next; original = original->next );

      CREATE( working, MPROG_DATA, 1 );
      if ( original )
	original->next = working;
      else
	iRoom->mudprogs = working;
      working = rprog_file_read( fread_word( fp ), working, iRoom );
      working->next = NULL;
      fread_to_eol( fp );
      break;
    }

  return;

}

/* This procedure is responsible for reading any in_file ROOMprograms.
 */

void rprog_read_programs( FILE *fp, ROOM_INDEX_DATA *pRoomIndex)
{
  MPROG_DATA *mprg;
  char        letter;
  bool        done = FALSE;

  if ( ( letter = fread_letter( fp ) ) != '>' )
  {
      bug( "Load_rooms: vnum %d ROOMPROG char", pRoomIndex->vnum );
      exit( 1 );
  }
  CREATE( mprg, MPROG_DATA, 1 );
  pRoomIndex->mudprogs = mprg;

  while ( !done )
  {
    mprg->type = mprog_name_to_type( fread_word( fp ) );
    switch ( mprg->type )
    {
     case ERROR_PROG:
	bug( "Load_rooms: vnum %d ROOMPROG type.", pRoomIndex->vnum );
	exit( 1 );
      break;
     case IN_FILE_PROG:
	mprg = rprog_file_read( fread_string( fp ), mprg,pRoomIndex );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_rooms: vnum %d bad ROOMPROG.", pRoomIndex->vnum );
	     exit( 1 );
	   break;
	}
      break;
     default:
	xSET_BIT(pRoomIndex->progtypes, mprg->type);
	mprg->arglist        = fread_string( fp );
	fread_to_eol( fp );
	mprg->comlist        = fread_string( fp );
	fread_to_eol( fp );
	switch ( letter = fread_letter( fp ) )
	{
	  case '>':
	     CREATE( mprg->next, MPROG_DATA, 1 );
	     mprg = mprg->next;
	   break;
	  case '|':
	     mprg->next = NULL;
	     fread_to_eol( fp );
	     done = TRUE;
	   break;
	  default:
	     bug( "Load_rooms: vnum %d bad ROOMPROG.", pRoomIndex->vnum );
	     exit( 1 );
	   break;
	}
      break;
    }
  }

  return;

}


/*************************************************************/
/* Function to delete a room index.  Called from do_rdelete in build.c
   Narn, May/96
   Don't ask me why they return bool.. :).. oh well.. -- Alty
*/
bool delete_room( ROOM_INDEX_DATA *room )
{
  int hash;
  ROOM_INDEX_DATA *prev, *limbo = get_room_index(ROOM_VNUM_LIMBO);
  OBJ_DATA *o;
  CHAR_DATA *ch;
  EXTRA_DESCR_DATA *ed;
  EXIT_DATA *ex;
  MPROG_ACT_LIST *mpact;
  MPROG_DATA *mp;
  
  while ((ch = room->first_person) != NULL)
  {
    if (!IS_NPC(ch))
    {
      char_from_room(ch);
      char_to_room(ch, limbo);
    }
    else
      extract_char(ch, TRUE);
  }
  while ((o = room->first_content) != NULL)
    extract_obj(o);
  while ((ed = room->first_extradesc) != NULL)
  {
    room->first_extradesc = ed->next;
    STRFREE(ed->keyword);
    STRFREE(ed->description);
    DISPOSE(ed);
    --top_ed;
  }
  while ((ex = room->first_exit) != NULL)
    extract_exit(room, ex);
  while ((mpact = room->mpact) != NULL) 
  {
    room->mpact = mpact->next;
    DISPOSE(mpact->buf);
    DISPOSE(mpact);
  }
  while ((mp = room->mudprogs) != NULL)
  {
    room->mudprogs = mp->next;
    STRFREE(mp->arglist);
    STRFREE(mp->comlist);
    DISPOSE(mp);
  }
  if (room->map)
  {
    MAP_INDEX_DATA *mapi;
    
    if ((mapi = get_map_index(room->map->vnum)) != NULL)
      if (room->map->x > 0 && room->map->x < 80 &&
          room->map->y > 0 && room->map->y < 48)
        mapi->map_of_vnums[room->map->y][room->map->x] = -1;
    DISPOSE(room->map);
  }
  STRFREE(room->name);
  STRFREE(room->description);
  
  hash = room->vnum%MAX_KEY_HASH;
  if (room == room_index_hash[hash])
    room_index_hash[hash] = room->next;
  else
  {
    for (prev = room_index_hash[hash]; prev; prev = prev->next)
      if (prev->next == room)
        break;
    if (prev)
      prev->next = room->next;
    else
      bug("delete_room: room %d not in hash bucket %d.", room->vnum, hash);
  }
  DISPOSE(room);
  --top_room;
  return TRUE;
}

/* See comment on delete_room. */
bool delete_obj( OBJ_INDEX_DATA *obj )
{
  int hash;
  OBJ_INDEX_DATA *prev;
  OBJ_DATA *o, *o_next;
  EXTRA_DESCR_DATA *ed;
  AFFECT_DATA *af;
  MPROG_DATA *mp;
  int auc;
  
  /* Remove references to object index */
  for (o = first_object; o; o = o_next)
  {
    o_next = o->next;
    if (o->pIndexData == obj)
      extract_obj(o);
  }
  while ((ed = obj->first_extradesc) != NULL)
  {
    obj->first_extradesc = ed->next;
    STRFREE(ed->keyword);
    STRFREE(ed->description);
    DISPOSE(ed);
    --top_ed;
  }
  while ((af = obj->first_affect) != NULL)
  {
    obj->first_affect = af->next;
    DISPOSE(af);
    --top_affect;
  }
  while ((mp = obj->mudprogs) != NULL)
  {
    obj->mudprogs = mp->next;
    STRFREE(mp->arglist);
    STRFREE(mp->comlist);
    DISPOSE(mp);
  }
  STRFREE(obj->name);
  STRFREE(obj->short_descr);
  STRFREE(obj->description);
  STRFREE(obj->action_desc);
  
  for (auc = 0; auc < AUCTION_MEM; ++auc)
    if (auction->history[auc] == obj)
    {
      if (auc < AUCTION_MEM-1)
        memmove(&auction->history[auc], &auction->history[auc+1],
            (AUCTION_MEM-auc-1)*sizeof(OBJ_INDEX_DATA *));
      auction->history[AUCTION_MEM-1] = NULL;
      --auc;
    }
  
  hash = obj->vnum%MAX_KEY_HASH;
  if (obj == obj_index_hash[hash])
    obj_index_hash[hash] = obj->next;
  else
  {
    for (prev = obj_index_hash[hash]; prev; prev = prev->next)
      if (prev->next == obj)
        break;
    if (prev)
      prev->next = obj->next;
    else
      bug("delete_obj: object %d not in hash bucket %d.", obj->vnum, hash);
  }
  DISPOSE(obj);
  --top_obj_index;
  return TRUE;
}

/* See comment on delete_room. */
bool delete_mob( MOB_INDEX_DATA *mob )
{
  int hash;
  MOB_INDEX_DATA *prev;
  CHAR_DATA *ch, *ch_next;
  MPROG_DATA *mp;
  
  for (ch = first_char; ch; ch = ch_next)
  {
    ch_next = ch->next;
    if (ch->pIndexData == mob)
      extract_char(ch, TRUE);
  }
  while ((mp = mob->mudprogs) != NULL)
  {
    mob->mudprogs = mp->next;
    STRFREE(mp->arglist);
    STRFREE(mp->comlist);
    DISPOSE(mp);
  }
  
  if (mob->pShop)
  {
    UNLINK(mob->pShop, first_shop, last_shop, next, prev);
    DISPOSE(mob->pShop);
    --top_shop;
  }
  
  if (mob->rShop)
  {
    UNLINK(mob->rShop, first_repair, last_repair, next, prev);
    DISPOSE(mob->rShop);
    --top_repair;
  }
  
  STRFREE(mob->player_name);
  STRFREE(mob->short_descr);
  STRFREE(mob->long_descr);
  STRFREE(mob->description);
  
  hash = mob->vnum%MAX_KEY_HASH;
  if (mob == mob_index_hash[hash])
    mob_index_hash[hash] = mob->next;
  else
  {
    for (prev = mob_index_hash[hash]; prev; prev = prev->next)
      if (prev->next == mob)
        break;
    if (prev)
      prev->next = mob->next;
    else
      bug("delete_mob: mobile %d not in hash bucket %d.", mob->vnum, hash);
  }
  DISPOSE(mob);
  --top_mob_index;
  return TRUE;
}

/*
 * Creat a new room (for online building)			-Thoric
 */
ROOM_INDEX_DATA *make_room( int vnum )
{
	ROOM_INDEX_DATA *pRoomIndex;
	int	iHash;

	CREATE( pRoomIndex, ROOM_INDEX_DATA, 1 );
	pRoomIndex->first_person	= NULL;
	pRoomIndex->last_person		= NULL;
	pRoomIndex->first_content	= NULL;
	pRoomIndex->last_content	= NULL;
	pRoomIndex->first_extradesc	= NULL;
	pRoomIndex->last_extradesc	= NULL;
	pRoomIndex->area		= NULL;
	pRoomIndex->vnum		= vnum;
	pRoomIndex->name		= STRALLOC("Floating in a void");
	pRoomIndex->description		= STRALLOC("");
	xSET_BIT( pRoomIndex->room_flags, ROOM_PROTOTYPE);
	pRoomIndex->sector_type		= 1;
	pRoomIndex->light		= 0;
	pRoomIndex->first_exit		= NULL;
	pRoomIndex->last_exit		= NULL;

	iHash			= vnum % MAX_KEY_HASH;
	pRoomIndex->next	= room_index_hash[iHash];
	room_index_hash[iHash]	= pRoomIndex;
	top_room++;

	return pRoomIndex;
}

/*
 * Create a new INDEX object (for online building)		-Thoric
 * Option to clone an existing index object.
 */
OBJ_INDEX_DATA *make_object( int vnum, int cvnum, char *name )
{
	OBJ_INDEX_DATA *pObjIndex, *cObjIndex;
	char buf[MAX_STRING_LENGTH];
	int	iHash;

	if ( cvnum > 0 )
	  cObjIndex = get_obj_index( cvnum );
	else
	  cObjIndex = NULL;
	CREATE( pObjIndex, OBJ_INDEX_DATA, 1 );
	pObjIndex->vnum			= vnum;
	pObjIndex->name			= STRALLOC( name );
	pObjIndex->first_affect		= NULL;
	pObjIndex->last_affect		= NULL;
	pObjIndex->first_extradesc	= NULL;
	pObjIndex->last_extradesc	= NULL;
	if ( !cObjIndex )
	{
	  sprintf( buf, "A newly created %s", name );
	  pObjIndex->short_descr	= STRALLOC( buf  );
	  sprintf( buf, "Some god dropped a newly created %s here.", name );
	  pObjIndex->description	= STRALLOC( buf );
	  pObjIndex->action_desc	= STRALLOC( "" );
	  pObjIndex->short_descr[0]	= LOWER(pObjIndex->short_descr[0]);
	  pObjIndex->description[0]	= UPPER(pObjIndex->description[0]);
	  pObjIndex->item_type		= ITEM_TRASH;
	  xCLEAR_BITS(pObjIndex->extra_flags);
	  xSET_BIT(pObjIndex->extra_flags, ITEM_PROTOTYPE);
	  pObjIndex->wear_flags		= 0;
	  pObjIndex->value[0]		= 0;
	  pObjIndex->value[1]		= 0;
	  pObjIndex->value[2]		= 0;
	  pObjIndex->value[3]		= 0;
	  pObjIndex->weight		= 1;
	  pObjIndex->cost		= 0;
	  pObjIndex->level		= 0;
	}
	else
	{
	  EXTRA_DESCR_DATA *ed,  *ced;
	  AFFECT_DATA	   *paf, *cpaf;

	  pObjIndex->short_descr	= QUICKLINK( cObjIndex->short_descr );
	  pObjIndex->description	= QUICKLINK( cObjIndex->description );
	  pObjIndex->action_desc	= QUICKLINK( cObjIndex->action_desc );
	  pObjIndex->item_type		= cObjIndex->item_type;
	  pObjIndex->extra_flags	= cObjIndex->extra_flags;
	  xSET_BIT(pObjIndex->extra_flags, ITEM_PROTOTYPE);
	  pObjIndex->wear_flags		= cObjIndex->wear_flags;
	  pObjIndex->value[0]		= cObjIndex->value[0];
	  pObjIndex->value[1]		= cObjIndex->value[1];
	  pObjIndex->value[2]		= cObjIndex->value[2];
	  pObjIndex->value[3]		= cObjIndex->value[3];
	  pObjIndex->weight		= cObjIndex->weight;
	  pObjIndex->cost		= cObjIndex->cost;
	  pObjIndex->level		= cObjIndex->level;
	  for ( ced = cObjIndex->first_extradesc; ced; ced = ced->next )
	  {
		CREATE( ed, EXTRA_DESCR_DATA, 1 );
		ed->keyword		= QUICKLINK( ced->keyword );
		ed->description		= QUICKLINK( ced->description );
		LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc,
			  next, prev );
		top_ed++;
	  }
	  for ( cpaf = cObjIndex->first_affect; cpaf; cpaf = cpaf->next )
	  {
		CREATE( paf, AFFECT_DATA, 1 );
		paf->type		= cpaf->type;
		paf->duration		= cpaf->duration;
		paf->location		= cpaf->location;
		paf->modifier		= cpaf->modifier;
		paf->bitvector		= cpaf->bitvector;
		LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect,
			   next, prev );
		top_affect++;
	  }
	}
	pObjIndex->count		= 0;
	iHash				= vnum % MAX_KEY_HASH;
	pObjIndex->next			= obj_index_hash[iHash];
	obj_index_hash[iHash]		= pObjIndex;
	top_obj_index++;

	return pObjIndex;
}

/*
 * Create a new INDEX mobile (for online building)		-Thoric
 * Option to clone an existing index mobile.
 */
MOB_INDEX_DATA *make_mobile( int vnum, int cvnum, char *name )
{
	MOB_INDEX_DATA *pMobIndex, *cMobIndex;
	char buf[MAX_STRING_LENGTH];
	int	iHash;

	if ( cvnum > 0 )
	  cMobIndex = get_mob_index( cvnum );
	else
	  cMobIndex = NULL;
	CREATE( pMobIndex, MOB_INDEX_DATA, 1 );
	pMobIndex->vnum			= vnum;
	pMobIndex->count		= 0;
	pMobIndex->killed		= 0;
	pMobIndex->player_name		= STRALLOC( name );
	if ( !cMobIndex )
	{
	  sprintf( buf, "A newly created %s", name );
	  pMobIndex->short_descr	= STRALLOC( buf  );
	  sprintf( buf, "Some god abandoned a newly created %s here.\n\r", name );
	  pMobIndex->long_descr		= STRALLOC( buf );
	  pMobIndex->description	= STRALLOC( "" );
	  pMobIndex->short_descr[0]	= LOWER(pMobIndex->short_descr[0]);
	  pMobIndex->long_descr[0]	= UPPER(pMobIndex->long_descr[0]);
	  pMobIndex->description[0]	= UPPER(pMobIndex->description[0]);
	  xCLEAR_BITS(pMobIndex->act);
	  xSET_BIT(pMobIndex->act, ACT_IS_NPC);
	  xSET_BIT(pMobIndex->act, ACT_PROTOTYPE);
	  xCLEAR_BITS(pMobIndex->affected_by);
	  pMobIndex->pShop		= NULL;
	  pMobIndex->rShop		= NULL;
	  pMobIndex->spec_fun		= NULL;
	  pMobIndex->mudprogs		= NULL;
	  xCLEAR_BITS(pMobIndex->progtypes);
	  pMobIndex->alignment		= 0;
	  pMobIndex->level		= 1;
	  pMobIndex->mobthac0		= 0;
	  pMobIndex->ac			= 0;
	  pMobIndex->hitnodice		= 0;
	  pMobIndex->hitsizedice	= 0;
	  pMobIndex->hitplus		= 0;
	  pMobIndex->damnodice		= 0;
	  pMobIndex->damsizedice	= 0;
	  pMobIndex->damplus		= 0;
	  pMobIndex->gold		= 0;
	  pMobIndex->exp		= 0;
	  /*
	   * Bug noticed by Sevoreria Dragonlight
	   * -- So we set it back to constants.. :P.. changed to POS_STANDING -- Alty
	   */
	  pMobIndex->position		= POS_STANDING;
	  pMobIndex->defposition	= POS_STANDING;
	  pMobIndex->sex		= 0;
	  pMobIndex->perm_str		= 13;
	  pMobIndex->perm_dex		= 13;
	  pMobIndex->perm_int		= 13;
	  pMobIndex->perm_wis		= 13;
	  pMobIndex->perm_cha		= 13;
	  pMobIndex->perm_con		= 13;
	  pMobIndex->perm_lck		= 13;
	  pMobIndex->race		= 0;
	  pMobIndex->class		= 3;
	  pMobIndex->xflags		= 0;
	  pMobIndex->resistant		= 0;
	  pMobIndex->immune		= 0;
	  pMobIndex->susceptible	= 0;
	  pMobIndex->numattacks		= 0;
	  xCLEAR_BITS(pMobIndex->attacks);
	  xCLEAR_BITS(pMobIndex->defenses);
	}
	else
	{
	  pMobIndex->short_descr	= QUICKLINK( cMobIndex->short_descr );
	  pMobIndex->long_descr		= QUICKLINK( cMobIndex->long_descr  );
	  pMobIndex->description	= QUICKLINK( cMobIndex->description );
	  pMobIndex->act		= cMobIndex->act;
	  xSET_BIT(pMobIndex->act, ACT_PROTOTYPE);
	  pMobIndex->affected_by	= cMobIndex->affected_by;
	  pMobIndex->pShop		= NULL;
	  pMobIndex->rShop		= NULL;
	  pMobIndex->spec_fun		= cMobIndex->spec_fun;
	  pMobIndex->mudprogs		= NULL;
	  xCLEAR_BITS(pMobIndex->progtypes);
	  pMobIndex->alignment		= cMobIndex->alignment;
	  pMobIndex->level		= cMobIndex->level;
	  pMobIndex->mobthac0		= cMobIndex->mobthac0;
	  pMobIndex->ac			= cMobIndex->ac;
	  pMobIndex->hitnodice		= cMobIndex->hitnodice;
	  pMobIndex->hitsizedice	= cMobIndex->hitsizedice;
	  pMobIndex->hitplus		= cMobIndex->hitplus;
	  pMobIndex->damnodice		= cMobIndex->damnodice;
	  pMobIndex->damsizedice	= cMobIndex->damsizedice;
	  pMobIndex->damplus		= cMobIndex->damplus;
	  pMobIndex->gold		= cMobIndex->gold;
	  pMobIndex->exp		= cMobIndex->exp;
	  pMobIndex->position		= cMobIndex->position;
	  pMobIndex->defposition	= cMobIndex->defposition;
	  pMobIndex->sex		= cMobIndex->sex;
	  pMobIndex->perm_str		= cMobIndex->perm_str;
	  pMobIndex->perm_dex		= cMobIndex->perm_dex;
	  pMobIndex->perm_int		= cMobIndex->perm_int;
	  pMobIndex->perm_wis		= cMobIndex->perm_wis;
	  pMobIndex->perm_cha		= cMobIndex->perm_cha;
	  pMobIndex->perm_con		= cMobIndex->perm_con;
	  pMobIndex->perm_lck		= cMobIndex->perm_lck;
	  pMobIndex->race		= cMobIndex->race;
	  pMobIndex->class		= cMobIndex->class;
	  pMobIndex->xflags		= cMobIndex->xflags;
	  pMobIndex->resistant		= cMobIndex->resistant;
	  pMobIndex->immune		= cMobIndex->immune;
	  pMobIndex->susceptible	= cMobIndex->susceptible;
	  pMobIndex->numattacks		= cMobIndex->numattacks;
	  pMobIndex->attacks		= cMobIndex->attacks;
	  pMobIndex->defenses		= cMobIndex->defenses;
	}
	iHash				= vnum % MAX_KEY_HASH;
	pMobIndex->next			= mob_index_hash[iHash];
	mob_index_hash[iHash]		= pMobIndex;
	top_mob_index++;

	return pMobIndex;
}

/*
 * Creates a simple exit with no fields filled but rvnum and optionally
 * to_room and vnum.						-Thoric
 * Exits are inserted into the linked list based on vdir.
 */
EXIT_DATA *make_exit( ROOM_INDEX_DATA *pRoomIndex, ROOM_INDEX_DATA *to_room, sh_int door )
{
	EXIT_DATA *pexit, *texit;
	bool broke;

	CREATE( pexit, EXIT_DATA, 1 );
	pexit->vdir		= door;
    pexit->orig_door	= door;
	pexit->rvnum		= pRoomIndex->vnum;
	pexit->to_room		= to_room;
	pexit->distance		= 1;
	if ( to_room )
	{
	    pexit->vnum = to_room->vnum;
	    texit = get_exit_to( to_room, rev_dir[door], pRoomIndex->vnum );
	    if ( texit )	/* assign reverse exit pointers */
	    {
		texit->rexit = pexit;
		pexit->rexit = texit;
	    }
	}
	broke = FALSE;
	for ( texit = pRoomIndex->first_exit; texit; texit = texit->next )
	   if ( door < texit->vdir )
	   {
	     broke = TRUE;
	     break;
	   }
	if ( !pRoomIndex->first_exit )
	  pRoomIndex->first_exit	= pexit;
	else
	{
	  /* keep exits in incremental order - insert exit into list */
	  if ( broke && texit )
	  {
	    if ( !texit->prev )
	      pRoomIndex->first_exit	= pexit;
	    else
	      texit->prev->next		= pexit;
	    pexit->prev			= texit->prev;
	    pexit->next			= texit;
	    texit->prev			= pexit;
	    top_exit++;
	    return pexit;
	  }
	  pRoomIndex->last_exit->next	= pexit;
	}
	pexit->next			= NULL;
	pexit->prev			= pRoomIndex->last_exit;
	pRoomIndex->last_exit		= pexit;
	top_exit++;
	return pexit;
}

void fix_area_exits( AREA_DATA *tarea )
{
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pexit, *rev_exit;
    int rnum;
    bool fexit;

    for ( rnum = tarea->low_r_vnum; rnum <= tarea->hi_r_vnum; rnum++ )
    {
	if ( (pRoomIndex = get_room_index( rnum )) == NULL )
	  continue;

	fexit = FALSE;
	for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
	{
		fexit = TRUE;
		pexit->rvnum = pRoomIndex->vnum;
		if ( pexit->vnum <= 0 )
	       	  pexit->to_room = NULL;
		else
		  pexit->to_room = get_room_index( pexit->vnum );
	}
	if ( !fexit )
	  xSET_BIT( pRoomIndex->room_flags, ROOM_NO_MOB );
    }


    for ( rnum = tarea->low_r_vnum; rnum <= tarea->hi_r_vnum; rnum++ )
    {
	if ( (pRoomIndex = get_room_index( rnum )) == NULL )
	  continue;

	for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
	{
		if ( pexit->to_room && !pexit->rexit )
		{
		   rev_exit = get_exit_to( pexit->to_room, rev_dir[pexit->vdir], pRoomIndex->vnum );
		   if ( rev_exit )
		   {
			pexit->rexit	= rev_exit;
			rev_exit->rexit	= pexit;
		   }
		}
	}
    }
}

void load_area_file( AREA_DATA *tarea, char *filename )
{

    if ( fBootDb )
      tarea = last_area;
    if ( !fBootDb && !tarea )
    {
	bug( "Load_area: null area!" );
	return;
    }

    if ( ( fpArea = fopen( filename, "r" ) ) == NULL )
    {
	perror( filename );
	bug( "load_area: error loading file (can't open)" );
	bug( filename );
	return;
    }
    area_version = 0;
    for ( ; ; )
    {
	char *word;

	if ( fread_letter( fpArea ) != '#' )
	{
	    bug( tarea->filename );
	    bug( "load_area: # not found." );
	    exit( 1 );
	}

	word = fread_word( fpArea );

	     if ( word[0] == '$'               )                 break;
	else if ( !str_cmp( word, "AREA"     ) )
	{
		if ( fBootDb )
		{
		  load_area    (fpArea);
		  tarea = last_area;
		}
		else
		{
		  DISPOSE( tarea->name );
		  tarea->name = fread_string_nohash( fpArea );
		}
	}
	else if ( !str_cmp( word, "AUTHOR"   ) ) load_author  (tarea, fpArea);
    else if ( !str_cmp( word, "CREDITS"  ) ) load_credits (tarea, fpArea);
	else if ( !str_cmp( word, "FLAGS"    ) ) load_flags   (tarea, fpArea);
	else if ( !str_cmp( word, "RANGES"   ) ) load_ranges  (tarea, fpArea);
	else if ( !str_cmp( word, "ECONOMY"  ) ) load_economy (tarea, fpArea);
	else if ( !str_cmp( word, "RESETMSG" ) ) load_resetmsg(tarea, fpArea); 
	/* Rennard */
	else if ( !str_cmp( word, "HELPS"    ) ) load_helps   (tarea, fpArea);
	else if ( !str_cmp( word, "MOBILES"  ) ) load_mobiles (tarea, fpArea);
	else if ( !str_cmp( word, "MUDPROGS" ) ) load_mudprogs(tarea, fpArea);
	else if ( !str_cmp( word, "OBJECTS"  ) ) load_objects (tarea, fpArea);
	else if ( !str_cmp( word, "OBJPROGS" ) ) load_objprogs(tarea, fpArea);
	else if ( !str_cmp( word, "RESETS"   ) ) load_resets  (tarea, fpArea);
	else if ( !str_cmp( word, "ROOMS"    ) ) load_rooms   (tarea, fpArea);
	else if ( !str_cmp( word, "SHOPS"    ) ) load_shops   (tarea, fpArea);
	else if ( !str_cmp( word, "REPAIRS"  ) ) load_repairs (tarea, fpArea);
	else if ( !str_cmp( word, "SPECIALS" ) ) load_specials(tarea, fpArea);
	else if ( !str_cmp( word, "CLIMATE"  ) ) load_climate (tarea, fpArea);
	else if ( !str_cmp( word, "NEIGHBOR" ) ) load_neighbor(tarea, fpArea);
	else if ( !str_cmp( word, "VERSION"  ) ) load_version (tarea, fpArea);
	else if ( !str_cmp( word, "SPELLLIMIT")) {
		tarea->spelllimit = fread_number( fpArea );
	}
	else
	{
	    bug( tarea->filename );
	    bug( "load_area: bad section name." );
	    if ( fBootDb )
	      exit( 1 );
	    else
	    {
	      fclose( fpArea );
	      fpArea = NULL;
	      return;
	    }
	}
    }
    fclose( fpArea );
    fpArea = NULL;
    if ( tarea )
    {
	if ( fBootDb )
	{
	  sort_area_by_name(tarea);     /* 4/27/97 */
	  sort_area( tarea, FALSE );
	}
	fprintf( stderr, "%-14s: Rooms: %5d - %-5d Objs: %5d - %-5d Mobs: %5d - %d\n",
		 tarea->filename,
		 tarea->low_r_vnum, tarea->hi_r_vnum,
		 tarea->low_o_vnum, tarea->hi_o_vnum,
		 tarea->low_m_vnum, tarea->hi_m_vnum );
	if ( !tarea->author )
	  tarea->author = STRALLOC( "" );
	SET_BIT( tarea->status, AREA_LOADED );
    }
    else
      fprintf( stderr, "(%s)\n", filename );
}

void load_reserved( void )
{
  RESERVE_DATA *res;
  FILE *fp;
  
  if ( !(fp = fopen( SYSTEM_DIR RESERVED_LIST, "r" )) )
    return;
  
  for ( ; ; )
  {
    if ( feof( fp ) )
    {
      bug( "Load_reserved: no $ found." );
      fclose(fp);
      return;
    }
    CREATE(res, RESERVE_DATA, 1);
    res->name = fread_string_nohash(fp);
    if (*res->name == '$')
      break;
    sort_reserved(res);
  }
  DISPOSE(res->name);
  DISPOSE(res);
  fclose(fp);
  return;
}

void load_noauctions( void )
{
  NOAUCTION_DATA *noauc;
  FILE *fp;

  if ( !(fp = fopen( SYSTEM_DIR NOAUCTION_LIST, "r" )) )
    return;

  for ( ; ; )
  {
    if ( feof( fp ) )
    {
      bug( "Load_noauctions: no 0 found." );
      fclose(fp);
      return;
    }

    CREATE(noauc, NOAUCTION_DATA, 1);
    noauc->vnum = fread_number(fp);
    if (noauc->vnum == 0)
      break;
    if ( get_obj_index(noauc->vnum) == NULL )
    {
      UNLINK(noauc, first_noauc, last_noauc, next, prev);
      DISPOSE(noauc);
    }
    sort_noauctions(noauc);
    save_noauctions();
  }
  DISPOSE(noauc);
  fclose(fp);
  return;
}

void sort_noauctions( NOAUCTION_DATA *pNoauc )
{
	NOAUCTION_DATA *noauc = NULL;

	if ( !pNoauc )
	{
		bug( "Sort_Noauction: NULL pNoauc");
		return;
	}

	pNoauc->next = NULL;
	pNoauc->prev = NULL;

	for ( noauc = first_noauc; noauc; noauc = noauc->next )
	{
		if ( noauc->vnum > pNoauc->vnum )
		{
			INSERT(pNoauc, noauc, first_noauc, next, prev);
			break;
		}
	}

	if ( !noauc )
		LINK(pNoauc, first_noauc, last_noauc, next, prev);
    return;
}


/* Build list of in_progress areas.  Do not load areas.
 * define AREA_READ if you want it to build area names rather than reading
 * them out of the area files. -- Altrag */
void load_buildlist( void )
{
	DIR *dp;
	struct dirent *dentry;
	FILE *fp;
	char buf[MAX_STRING_LENGTH];
	AREA_DATA *pArea;
	char line[81];
	char word[81];
	int low, hi;
	int mlow, mhi, olow, ohi, rlow, rhi;
	bool badfile = FALSE;
	char temp;
	
	dp = opendir( GOD_DIR );
	dentry = readdir( dp );
	while ( dentry )
	{
		if ( dentry->d_name[0] != '.' )
		{
			sprintf( buf, "%s%s", GOD_DIR, dentry->d_name );
			if ( !(fp = fopen( buf, "r" )) )
			{
				bug( "Load_buildlist: invalid file" );
				perror( buf );
				dentry = readdir(dp);
				continue;
			}
			log_string( buf );
			badfile = FALSE;
			rlow=rhi=olow=ohi=mlow=mhi=0;
			while ( !feof(fp) && !ferror(fp) )
			{
				low = 0; hi = 0; word[0] = 0; line[0] = 0;
				if ( (temp = fgetc(fp)) != EOF )
					ungetc( temp, fp );
				else
					break;
				
				fgets(line, 80, fp);
				sscanf( line, "%s %d %d", word, &low, &hi );
				if ( !strcmp( word, "Level" ) )
				{
					if ( low < LEVEL_IMMORTAL )
					{
						sprintf( buf, "%s: God file with level %d < %d",
							dentry->d_name, low, LEVEL_IMMORTAL );
						badfile = TRUE;
					}
				}
				if ( !strcmp( word, "RoomRange" ) )
					rlow = low, rhi = hi;
				else if ( !strcmp( word, "MobRange" ) )
					mlow = low, mhi = hi;
				else if ( !strcmp( word, "ObjRange" ) )
					olow = low, ohi = hi;
			}
			fclose( fp );
			if ( rlow && rhi && !badfile )
			{
				sprintf( buf, "%s%s.are", BUILD_DIR, dentry->d_name );
				if ( !(fp = fopen( buf, "r" )) )
				{
					bug( "Load_buildlist: cannot open area file for read" );
					perror( buf );
					dentry = readdir(dp);
					continue;
				}
#if !defined(READ_AREA)  /* Dont always want to read stuff.. dunno.. shrug */
				strcpy( word, fread_word( fp ) );
				if ( word[0] != '#' || strcmp( &word[1], "AREA" ) )
				{
					sprintf( buf, "Make_buildlist: %s.are: no #AREA found.",
						dentry->d_name );
					fclose( fp );
					dentry = readdir(dp);
					continue;
				}
#endif
				CREATE( pArea, AREA_DATA, 1 );
				sprintf( buf, "%s.are", dentry->d_name );
				pArea->author = STRALLOC( dentry->d_name );
				pArea->filename = str_dup( buf );
#if !defined(READ_AREA)
				pArea->name = fread_string_nohash( fp );
#else
				sprintf( buf, "{PROTO} %s's area in progress", dentry->d_name );
				pArea->name = str_dup( buf );
#endif
				fclose( fp );
				pArea->low_r_vnum = rlow; pArea->hi_r_vnum = rhi;
				pArea->low_m_vnum = mlow; pArea->hi_m_vnum = mhi;
				pArea->low_o_vnum = olow; pArea->hi_o_vnum = ohi;
				pArea->low_soft_range = -1; pArea->hi_soft_range = -1;
				pArea->low_hard_range = -1; pArea->hi_hard_range = -1;
				
				CREATE(pArea->weather, WEATHER_DATA, 1); /* FB */
				pArea->weather->temp = 0;
				pArea->weather->precip = 0;
				pArea->weather->wind = 0;
				pArea->weather->temp_vector = 0;
				pArea->weather->precip_vector = 0;
				pArea->weather->wind_vector = 0;
				pArea->weather->climate_temp = 2;
				pArea->weather->climate_precip = 2;
				pArea->weather->climate_wind = 2;
				pArea->weather->first_neighbor = NULL;
				pArea->weather->last_neighbor = NULL;
				pArea->weather->echo = NULL;
				pArea->weather->echo_color = AT_GREY;
				
				pArea->first_reset = NULL; pArea->last_reset = NULL;
				LINK( pArea, first_build, last_build, next, prev );
				fprintf( stderr, "%-14s: Rooms: %5d - %-5d Objs: %5d - %-5d "
								 "Mobs: %5d - %-5d\n",
					pArea->filename,
					pArea->low_r_vnum, pArea->hi_r_vnum,
					pArea->low_o_vnum, pArea->hi_o_vnum,
					pArea->low_m_vnum, pArea->hi_m_vnum );
				sort_area( pArea, TRUE );
			}
		}
		dentry = readdir(dp);
	}
	closedir(dp);
}

/* Rebuilt from broken copy, but bugged - commented out for now - Blod */
void sort_reserved( RESERVE_DATA *pRes )
{
    RESERVE_DATA *res = NULL;
 
    if ( !pRes )
    {
        bug( "Sort_reserved: NULL pRes" );
        return;
    }
 
    pRes->next = NULL;
    pRes->prev = NULL;
 
    for ( res = first_reserved; res; res = res->next )
    {
        if ( strcasecmp(pRes->name, res->name) > 0 )
        {
            INSERT(pRes, res, first_reserved, next, prev);
            break;
        }
    }
 
    if ( !res )
    {
	LINK(pRes, first_reserved, last_reserved,
		next, prev);
    }
    
    return;
}


/*
 * Sort areas by name alphanumercially
 *      - 4/27/97, Fireblade
 */
void sort_area_by_name(AREA_DATA *pArea)
{
        AREA_DATA *temp_area;
 
        if(!pArea)
        {
                bug("Sort_area_by_name: NULL pArea");
                return;
        }
        for(temp_area = first_area_name; temp_area;
                        temp_area = temp_area->next_sort_name)
        {
                if(strcmp(pArea->name, temp_area->name) < 0)
                {
                        INSERT(pArea, temp_area, first_area_name,
                                next_sort_name, prev_sort_name);
                        break;
                }
        }
        if(!temp_area)
        {
                LINK(pArea, first_area_name, last_area_name,
                        next_sort_name, prev_sort_name);
        }
        return;
}

/*
 * Sort by room vnums					-Altrag & Thoric
 */
void sort_area( AREA_DATA *pArea, bool proto )
{
    AREA_DATA *area = NULL;
    AREA_DATA *first_sort, *last_sort;
    bool found;

    if ( !pArea )
    {
	bug( "Sort_area: NULL pArea" );
	return;
    }

    if ( proto )
    {
	first_sort = first_bsort;
	last_sort  = last_bsort;
    }
    else
    {
	first_sort = first_asort;
	last_sort  = last_asort;
    }
	
    found = FALSE;
    pArea->next_sort = NULL;
    pArea->prev_sort = NULL;

    if ( !first_sort )
    {
	pArea->prev_sort = NULL;
	pArea->next_sort = NULL;
	first_sort	 = pArea;
	last_sort	 = pArea;
	found = TRUE;
    }
    else
    for ( area = first_sort; area; area = area->next_sort )
	if ( pArea->low_r_vnum < area->low_r_vnum )
	{
	    if ( !area->prev_sort )
	      first_sort	= pArea;
	    else
	      area->prev_sort->next_sort = pArea;
	    pArea->prev_sort = area->prev_sort;
	    pArea->next_sort = area;
	    area->prev_sort  = pArea;
	    found = TRUE;
	    break;
        }

    if ( !found )
    {
	pArea->prev_sort     = last_sort;
	pArea->next_sort     = NULL;
	last_sort->next_sort = pArea;
	last_sort	     = pArea;
    }

    if ( proto )
    {
	first_bsort = first_sort;
	last_bsort  = last_sort;
    }
    else
    {
	first_asort = first_sort;
	last_asort  = last_sort;
    }
}


/*
 * Display vnums currently assigned to areas		-Altrag & Thoric
 * Sorted, and flagged if loaded.
 */
void show_vnums( CHAR_DATA *ch, int low, int high, bool proto, bool shownl,
		 char *loadst, char *notloadst )
{
    AREA_DATA *pArea, *first_sort;
    int count, loaded;

    count = 0;	loaded = 0;
    set_pager_color( AT_PLAIN, ch );
    if ( proto )
      first_sort = first_bsort;
    else
      first_sort = first_asort;
    for ( pArea = first_sort; pArea; pArea = pArea->next_sort )
    {
	if ( IS_SET( pArea->status, AREA_DELETED ) )
	   continue;
	if ( pArea->low_r_vnum < low )
	   continue;
	if ( pArea->hi_r_vnum > high )
	   break;
	if ( IS_SET(pArea->status, AREA_LOADED) )
	   loaded++;
	else
	if ( !shownl )
	   continue;
	pager_printf(ch, "%-15s| Rooms: %5d - %-5d"
		     " Objs: %5d - %-5d Mobs: %5d - %-5d%s\n\r",
		(pArea->filename ? pArea->filename : "(invalid)"),
		pArea->low_r_vnum, pArea->hi_r_vnum,
		pArea->low_o_vnum, pArea->hi_o_vnum,
		pArea->low_m_vnum, pArea->hi_m_vnum,
		IS_SET(pArea->status, AREA_LOADED) ? loadst : notloadst );
	count++;
    }
    pager_printf( ch, "Areas listed: %d  Loaded: %d\n\r", count, loaded );
    return;
}

/*
 * Shows prototype vnums ranges, and if loaded
 */
void do_vnums( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int low, high;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    low = 0;	high = 1073741824;
    if ( arg1[0] != '\0' )
    {
	low = atoi(arg1);
	if ( arg2[0] != '\0' )
	  high = atoi(arg2);
    }
    show_vnums( ch, low, high, TRUE, TRUE, " *", "" );
}

/*
 * Shows installed areas, sorted.  Mark unloaded areas with an X
 */
void do_zones( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int low, high;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    low = 0;	high = 1073741824;
    if ( arg1[0] != '\0' )
    {
	low = atoi(arg1);
	if ( arg2[0] != '\0' )
	  high = atoi(arg2);
    }
    show_vnums( ch, low, high, FALSE, TRUE, "", " X" );
}

/*
 * Show prototype areas, sorted.  Only show loaded areas
 */
void do_newzones( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int low, high;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    low = 0;	high = 1073741824;
    if ( arg1[0] != '\0' )
    {
	low = atoi(arg1);
	if ( arg2[0] != '\0' )
	  high = atoi(arg2);
    }
    show_vnums( ch, low, high, TRUE, FALSE, "", " X" );
}

/*
 * Save system info to data file
 */
void save_sysdata( SYSTEM_DATA sys )
{
    FILE *fp;
    char filename[MAX_INPUT_LENGTH];

    sprintf( filename, "%ssysdata.dat", SYSTEM_DIR );
    
    fclose( fpReserve );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
    	bug( "save_sysdata: fopen" );
    	perror( filename );
    }
    else
    {
	fprintf( fp, "#SYSTEM\n" );
	fprintf( fp, "MudName	     %s~\n", sys.mud_name		);
	fprintf( fp, "PortName	     %s~\n", sys.port_name		);
	fprintf( fp, "Highplayers    %d\n", sys.alltimemax		);
	fprintf( fp, "Highplayertime %s~\n", sys.time_of_max		);
	fprintf( fp, "NewsPath	     %s~\n", sys.news_html_path		);
        fprintf (fp, "CheckImmHost   %d\n", sys.check_imm_host );
	fprintf( fp, "Nameresolving  %d\n", sys.NO_NAME_RESOLVING	);
	fprintf( fp, "Waitforauth    %d\n", sys.WAIT_FOR_AUTH		);
	fprintf( fp, "Readallmail    %d\n", sys.read_all_mail		);
	fprintf( fp, "Readmailfree   %d\n", sys.read_mail_free		);
	fprintf( fp, "Writemailfree  %d\n", sys.write_mail_free		);
	fprintf( fp, "Takeothersmail %d\n", sys.take_others_mail	);
	fprintf( fp, "IMCMailVnum    %d\n", sys.imc_mail_vnum		);
	fprintf( fp, "Muse           %d\n", sys.muse_level		);
	fprintf( fp, "Think          %d\n", sys.think_level		);
	fprintf( fp, "Build          %d\n", sys.build_level		);
	fprintf( fp, "Log            %d\n", sys.log_level		);
	fprintf( fp, "Protoflag      %d\n", sys.level_modify_proto	);
	fprintf( fp, "Overridepriv   %d\n", sys.level_override_private	);
	fprintf( fp, "Msetplayer     %d\n", sys.level_mset_player	);
	fprintf( fp, "Stunplrvsplr   %d\n", sys.stun_plr_vs_plr		);
	fprintf( fp, "Stunregular    %d\n", sys.stun_regular		);
	fprintf( fp, "Gougepvp       %d\n", sys.gouge_plr_vs_plr        );
        fprintf( fp, "Gougenontank   %d\n", sys.gouge_nontank           );
        fprintf( fp, "Bashpvp        %d\n", sys.bash_plr_vs_plr         );
        fprintf( fp, "Bashnontank    %d\n", sys.bash_nontank            );
        fprintf( fp, "Dodgemod       %d\n", sys.dodge_mod               );
        fprintf( fp, "Parrymod       %d\n", sys.parry_mod               );
        fprintf( fp, "Tumblemod      %d\n", sys.tumble_mod              );
	fprintf( fp, "Tumblepk	     %d\n", sys.tumble_pk		);
	fprintf( fp, "Damplrvsplr    %d\n", sys.dam_plr_vs_plr		);
	fprintf( fp, "Damplrvsmob    %d\n", sys.dam_plr_vs_mob		);
	fprintf( fp, "Dammobvsplr    %d\n", sys.dam_mob_vs_plr		);
	fprintf( fp, "Dammobvsmob    %d\n", sys.dam_mob_vs_mob		);
	fprintf( fp, "Damnonavvsmob  %d\n", sys.dam_nonav_vs_mob	);
	fprintf( fp, "Dammobvsnonav  %d\n", sys.dam_mob_vs_nonav	);
	fprintf( fp, "Peaceexpmod    %d\n", sys.peaceful_exp_mod	);
	fprintf( fp, "Deadlyexpmod   %d\n", sys.deadly_exp_mod		);
	fprintf( fp, "Forcepc        %d\n", sys.level_forcepc		);
	fprintf( fp, "Guildoverseer  %s~\n", sys.guild_overseer		);
	fprintf( fp, "Guildadvisor   %s~\n", sys.guild_advisor		);
	fprintf( fp, "Saveflags      %d\n", sys.save_flags		);
	fprintf( fp, "Savefreq       %d\n", sys.save_frequency		);
	fprintf( fp, "Bestowdif      %d\n", sys.bestow_dif		);
        fprintf (fp, "BanSiteLevel   %d\n", sys.ban_site_level          );
        fprintf (fp, "BanRaceLevel   %d\n", sys.ban_race_level          );
        fprintf (fp, "BanClassLevel  %d\n", sys.ban_class_level         );
        fprintf (fp, "MorphOpt       %d\n", sys.morph_opt 		);
	fprintf (fp, "PetSave	     %d\n", sys.save_pets		);
        fprintf( fp, "IdentTries     %d\n", sys.ident_retries		);
	fprintf( fp, "Pkloot	     %d\n", sys.pk_loot			);
	fprintf( fp, "Pkchannels     %d\n", sys.pk_channels		);
	fprintf( fp, "Pksilence	     %d\n", sys.pk_silence		);
        fprintf( fp, "SaveVersion    %d\n", sys.save_version            );
    	fprintf( fp, "Wizlock        %d\n", sys.wizlock );
		fprintf( fp, "Magichell      %d\n", sys.magichell );
	fprintf( fp, "MaxNews	     %d\n", sys.max_html_news		);
	fprintf( fp, "End\n\n"						);
	fprintf( fp, "#END\n"						);
    }
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}


void fread_sysdata( SYSTEM_DATA *sys, FILE *fp )
{
    char *word;
    bool fMatch;

    sys->time_of_max = NULL;
    sys->mud_name = NULL;
    sys->port_name = NULL;
    sys->save_version = 0; /* default value */
    
    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;
	case 'B':
            KEY( "Bashpvp",     sys->bash_plr_vs_plr, fread_number( fp ) );
            KEY( "Bashnontank", sys->bash_nontank,    fread_number( fp ) );
	    KEY( "Bestowdif",   sys->bestow_dif,      fread_number( fp ) );
	    KEY( "Build",	sys->build_level,     fread_number( fp ) );
            KEY ("BanSiteLevel", sys->ban_site_level, fread_number (fp));
            KEY ("BanClassLevel", sys->ban_class_level, fread_number (fp));
            KEY ("BanRaceLevel", sys->ban_race_level, fread_number (fp));
	    break;

        case 'C':
            KEY ("CheckImmHost", sys->check_imm_host, fread_number (fp));
            break;

	case 'D':
	    KEY( "Damplrvsplr",	   sys->dam_plr_vs_plr,	  fread_number( fp ) );
	    KEY( "Damplrvsmob",	   sys->dam_plr_vs_mob,	  fread_number( fp ) );
	    KEY( "Dammobvsplr",	   sys->dam_mob_vs_plr,	  fread_number( fp ) );
	    KEY( "Dammobvsmob",	   sys->dam_mob_vs_mob,	  fread_number( fp ) );
	    KEY( "Dammobvsnonav",  sys->dam_mob_vs_nonav, fread_number( fp ) );
	    KEY( "Damnonavvsmob",  sys->dam_nonav_vs_mob, fread_number( fp ) );
	    KEY( "Deadlyexpmod",   sys->deadly_exp_mod,   fread_number( fp ) );
            KEY( "Dodgemod",   sys->dodge_mod, fread_number( fp ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		extern bool wizlock;

		if ( !sys->time_of_max )
		    sys->time_of_max = str_dup("(not recorded)");
		if ( !sys->mud_name )
		    sys->mud_name = str_dup("(Name Not Set)");
		if ( !sys->port_name )
		    sys->port_name = str_dup("mud");
	       if ( sys->wizlock )
                   wizlock = TRUE;
		return;
	    }
	    break;

	case 'F':
	    KEY( "Forcepc",	   sys->level_forcepc,	  fread_number( fp ) );
	    break;
	    
	case 'G':
            KEY( "Gougepvp",     sys->gouge_plr_vs_plr, fread_number( fp ) );
            KEY( "Gougenontank", sys->gouge_nontank,   fread_number( fp ) );
	    KEY( "Guildoverseer",  sys->guild_overseer,  fread_string( fp ) );
	    KEY( "Guildadvisor",   sys->guild_advisor,   fread_string( fp ) );
	    break;

	case 'H':
	    KEY( "Highplayers",	   sys->alltimemax,	  fread_number( fp ) );
	    KEY( "Highplayertime", sys->time_of_max,      fread_string_nohash( fp ) );
	    break;

	case 'I':
	    KEY( "IdentTries",	   sys->ident_retries,	  fread_number( fp ) );
	    KEY( "IMCMailVnum",	   sys->imc_mail_vnum,	  fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Log",		   sys->log_level,	  fread_number( fp ) );
	    break;

	case 'M':
		KEY( "Magichell", sys->magichell, fread_number(fp));
	    KEY( "MaxNews", sys->max_html_news, fread_number(fp));
            KEY( "MorphOpt", sys->morph_opt, fread_number (fp));
	    KEY( "Msetplayer",	   sys->level_mset_player, fread_number( fp ) );
	    KEY( "MudName", sys->mud_name, fread_string_nohash(fp));
	    KEY( "Muse",	   sys->muse_level,	   fread_number( fp ) );
	    break;

	case 'N':
            KEY( "Nameresolving",  sys->NO_NAME_RESOLVING, fread_number( fp ) );
	    KEY( "NewsPath", sys->news_html_path, fread_string(fp));
	    break;

	case 'O':
	    KEY( "Overridepriv",   sys->level_override_private, fread_number( fp ) );
	    break;

	case 'P':
	    KEY( "Parrymod",    sys->parry_mod,   fread_number( fp ) );
	    KEY( "Peaceexpmod", sys->peaceful_exp_mod, fread_number( fp ) );
	    KEY( "PetSave", 	sys->save_pets,	  fread_number( fp ) );
	    KEY( "Pkchannels",	sys->pk_channels, fread_number( fp ) );
	    KEY( "Pkloot",	sys->pk_loot,	  fread_number( fp ) );
	    KEY( "Pksilence",	sys->pk_silence,  fread_number( fp ) );
            KEY( "PortName",    sys->port_name, fread_string_nohash(fp));
	    KEY( "Protoflag",	sys->level_modify_proto, fread_number( fp ) );
	    break;

	case 'R':
	    KEY( "Readallmail",	   sys->read_all_mail,	fread_number( fp ) );
	    KEY( "Readmailfree",   sys->read_mail_free,	fread_number( fp ) );
	    break;

	case 'S':
	    KEY( "Stunplrvsplr",   sys->stun_plr_vs_plr, fread_number( fp ) );
	    KEY( "Stunregular",    sys->stun_regular,	fread_number( fp ) );
	    KEY( "Saveflags",	   sys->save_flags,	fread_number( fp ) );
	    KEY( "Savefreq",	   sys->save_frequency,	fread_number( fp ) );
            KEY( "SaveVersion",    sys->save_version,   fread_number( fp ) );
	    break;

	case 'T':
	    KEY( "Takeothersmail", sys->take_others_mail, fread_number( fp ) );
	    KEY( "Think",	   sys->think_level,	fread_number( fp ) );
	    KEY( "Tumblemod",   sys->tumble_mod, fread_number( fp ) );
	    KEY( "Tumblepk",	sys->tumble_pk, fread_number( fp ) );
	    break;


	case 'W':
	    KEY( "Waitforauth",	   sys->WAIT_FOR_AUTH,	  fread_number( fp ) );
	    KEY( "Wizlock",        sys->wizlock,          fread_number( fp ) );
            KEY( "Writemailfree",  sys->write_mail_free,  fread_number( fp ) );
	    break;
	}
	

	if ( !fMatch )
	{
            bug( "Fread_sysdata: no match: %s", word );
	}
    }
}



/*
 * Load the sysdata file
 */
bool load_systemdata( SYSTEM_DATA *sys )
{
    char filename[MAX_INPUT_LENGTH];
    FILE *fp;
    bool found;

    found = FALSE;
    sprintf( filename, "%ssysdata.dat", SYSTEM_DIR );

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {

	found = TRUE;
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_sysdata_file: # not found." );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "SYSTEM" ) )
	    {
	    	fread_sysdata( sys, fp );
	    	break;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
		bug( "Load_sysdata_file: bad section." );
		break;
	    }
	}
	fclose( fp );
    }

    if ( !sysdata.guild_overseer ) sysdata.guild_overseer = STRALLOC( "" );
    if ( !sysdata.guild_advisor  ) sysdata.guild_advisor  = STRALLOC( "" );
    return found;
}

void load_watchlist( void )
{
  WATCH_DATA *pwatch;
  FILE *fp;
  int number;
  CMDTYPE *cmd;
  
  if ( !(fp = fopen( SYSTEM_DIR WATCH_LIST, "r" )) )
    return;
    
  for ( ; ; )
  {
    if ( feof( fp ) )
    {
      bug( "Load_watchlist: no -1 found." );
      fclose( fp );
      return;
    }
    number = fread_number( fp );
    if ( number == -1 )
    {
      fclose( fp );
      return;
    }

    CREATE( pwatch, WATCH_DATA, 1 );
    pwatch->imm_level   = number;
    pwatch->imm_name    = fread_string_nohash(fp);
    pwatch->target_name = fread_string_nohash(fp);
    if ( strlen( pwatch->target_name ) < 2 )
    	DISPOSE( pwatch->target_name );
    pwatch->player_site = fread_string_nohash(fp);
    if ( strlen ( pwatch->player_site ) < 2 )
    	DISPOSE( pwatch->player_site );

    /* Check for command watches */
    if ( pwatch->target_name )
    for(cmd = command_hash[(int)pwatch->target_name[0]]; cmd; cmd = cmd->next)
    {
    	if(!str_cmp(pwatch->target_name, cmd->name))
    	{
    		SET_BIT(cmd->flags, CMD_WATCH);
    		break;
    	}
    }

    LINK( pwatch, first_watch, last_watch, next, prev );
  }
}


/* Check to make sure range of vnums is free - Scryn 2/27/96 */

void do_check_vnums( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    AREA_DATA *pArea;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    bool room, mob, obj, all, area_conflict;
    int low_range, high_range;

    room = FALSE;
    mob  = FALSE;
    obj  = FALSE;
    all  = FALSE;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (arg1[0] == '\0')
    {
      send_to_char("Please specify room, mob, object, or all as your first argument.\n\r", ch);
      return;
    }

    if(!str_cmp(arg1, "room"))
      room = TRUE;

    else if(!str_cmp(arg1, "mob"))
      mob = TRUE;

    else if(!str_cmp(arg1, "object"))
      obj = TRUE;
   
    else if(!str_cmp(arg1, "all"))
      all = TRUE;
    else 
    {
      send_to_char("Please specify room, mob, or object as your first argument.\n\r", ch);
      return;
    }

    if(arg2[0] == '\0')
    {
      send_to_char("Please specify the low end of the range to be searched.\n\r", ch);
      return;
    }

    if(argument[0] == '\0')
    {
      send_to_char("Please specify the high end of the range to be searched.\n\r", ch);
      return;
    }

    low_range = atoi(arg2);
    high_range = atoi(argument);

    if (low_range < 1 || low_range > MAX_VNUM )
    {
      send_to_char("Invalid argument for bottom of range.\n\r", ch);
      return;
    }

    if (high_range < 1 || high_range > MAX_VNUM )
    {
      send_to_char("Invalid argument for top of range.\n\r", ch);
      return;
    }
      
    if (high_range < low_range)
    {
      send_to_char("Bottom of range must be below top of range.\n\r", ch);
      return;
    }
    
    if (all)
    {
      sprintf(buf, "room %d %d", low_range, high_range);
      do_check_vnums(ch, buf);
      sprintf(buf, "mob %d %d", low_range, high_range);
      do_check_vnums(ch, buf);
      sprintf(buf, "object %d %d", low_range, high_range);
      do_check_vnums(ch, buf);
      return;
    }
    set_char_color( AT_PLAIN, ch );

    for ( pArea = first_asort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;
	if ( IS_SET( pArea->status, AREA_DELETED ) )
	   continue;
	else
	if (room)
	{
	  if ( low_range < pArea->low_r_vnum && pArea->low_r_vnum < high_range )
	    area_conflict = TRUE;

	  if ( low_range < pArea->hi_r_vnum && pArea->hi_r_vnum < high_range )
	    area_conflict = TRUE;

	  if ( ( low_range >= pArea->low_r_vnum )
	  && ( low_range <= pArea->hi_r_vnum ) )
	    area_conflict = TRUE;

	  if ( ( high_range <= pArea->hi_r_vnum )
	  && ( high_range >= pArea->low_r_vnum ) ) 
	    area_conflict = TRUE;
	}

	if (mob)
	{
	  if ( low_range < pArea->low_m_vnum && pArea->low_m_vnum < high_range )
	    area_conflict = TRUE;

	  if ( low_range < pArea->hi_m_vnum && pArea->hi_m_vnum < high_range )
	    area_conflict = TRUE;
	  if ( ( low_range >= pArea->low_m_vnum )
	  && ( low_range <= pArea->hi_m_vnum ) )
	    area_conflict = TRUE;

	  if ( ( high_range <= pArea->hi_m_vnum )
	  && ( high_range >= pArea->low_m_vnum ) )
	    area_conflict = TRUE;
	}

	if (obj)
	{
	  if ( low_range < pArea->low_o_vnum && pArea->low_o_vnum < high_range )
	    area_conflict = TRUE;

	  if ( low_range < pArea->hi_o_vnum && pArea->hi_o_vnum < high_range )
	    area_conflict = TRUE;

	  if ( ( low_range >= pArea->low_o_vnum )
	  && ( low_range <= pArea->hi_o_vnum ) )
	    area_conflict = TRUE;

	  if ( ( high_range <= pArea->hi_o_vnum )
	  && ( high_range >= pArea->low_o_vnum ) )
	    area_conflict = TRUE;
	}

	if (area_conflict)
	{
	sprintf(buf, "Conflict:%-15s| ",
		(pArea->filename ? pArea->filename : "(invalid)"));
        if(room)
          sprintf( buf2, "Rooms: %5d - %-5d\n\r", pArea->low_r_vnum, 
          pArea->hi_r_vnum);
        if(mob)
          sprintf( buf2, "Mobs: %5d - %-5d\n\r", pArea->low_m_vnum, 
          pArea->hi_m_vnum);
        if(obj)
          sprintf( buf2, "Objects: %5d - %-5d\n\r", pArea->low_o_vnum, 
          pArea->hi_o_vnum);
        
        strcat( buf, buf2 );
	send_to_char(buf, ch);
    	}
    }    
    for ( pArea = first_bsort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;
	if ( IS_SET( pArea->status, AREA_DELETED ) )
	   continue;
	else
	if (room)
	{
	  if ( low_range < pArea->low_r_vnum && pArea->low_r_vnum < high_range )
	    area_conflict = TRUE;

	  if ( low_range < pArea->hi_r_vnum && pArea->hi_r_vnum < high_range )
	    area_conflict = TRUE;

	  if ( ( low_range >= pArea->low_r_vnum )
	  && ( low_range <= pArea->hi_r_vnum ) )
	    area_conflict = TRUE;

	  if ( ( high_range <= pArea->hi_r_vnum )
	  && ( high_range >= pArea->low_r_vnum ) ) 
	    area_conflict = TRUE;
	}

	if (mob)
	{
	  if ( low_range < pArea->low_m_vnum && pArea->low_m_vnum < high_range )
	    area_conflict = TRUE;

	  if ( low_range < pArea->hi_m_vnum && pArea->hi_m_vnum < high_range )
	    area_conflict = TRUE;
	  if ( ( low_range >= pArea->low_m_vnum )
	  && ( low_range <= pArea->hi_m_vnum ) )
	    area_conflict = TRUE;

	  if ( ( high_range <= pArea->hi_m_vnum )
	  && ( high_range >= pArea->low_m_vnum ) )
	    area_conflict = TRUE;
	}

	if (obj)
	{
	  if ( low_range < pArea->low_o_vnum && pArea->low_o_vnum < high_range )
	    area_conflict = TRUE;

	  if ( low_range < pArea->hi_o_vnum && pArea->hi_o_vnum < high_range )
	    area_conflict = TRUE;

	  if ( ( low_range >= pArea->low_o_vnum )
	  && ( low_range <= pArea->hi_o_vnum ) )
	    area_conflict = TRUE;

	  if ( ( high_range <= pArea->hi_o_vnum )
	  && ( high_range >= pArea->low_o_vnum ) )
	    area_conflict = TRUE;
	}

	if (area_conflict)
	{
	sprintf(buf, "Conflict:%-15s| ",
		(pArea->filename ? pArea->filename : "(invalid)"));
        if(room)
          sprintf( buf2, "Rooms: %5d - %-5d\n\r", pArea->low_r_vnum, 
          pArea->hi_r_vnum);
        if(mob)
          sprintf( buf2, "Mobs: %5d - %-5d\n\r", pArea->low_m_vnum, 
          pArea->hi_m_vnum);
        if(obj)
          sprintf( buf2, "Objects: %5d - %-5d\n\r", pArea->low_o_vnum, 
          pArea->hi_o_vnum);
        
        strcat( buf, buf2 );
	send_to_char(buf, ch);
    	}
    }    

/*
    for ( pArea = first_asort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;
	if ( IS_SET( pArea->status, AREA_DELETED ) )
	   continue;
	else
	if (room)
	  if((pArea->low_r_vnum >= low_range) 
	  && (pArea->hi_r_vnum <= high_range))
	    area_conflict = TRUE;

	if (mob)
	  if((pArea->low_m_vnum >= low_range) 
	  && (pArea->hi_m_vnum <= high_range))
	    area_conflict = TRUE;

	if (obj)
	  if((pArea->low_o_vnum >= low_range) 
	  && (pArea->hi_o_vnum <= high_range))
	    area_conflict = TRUE;

	if (area_conflict)
	  ch_printf(ch, "Conflict:%-15s| Rooms: %5d - %-5d"
		     " Objs: %5d - %-5d Mobs: %5d - %-5d\n\r",
		(pArea->filename ? pArea->filename : "(invalid)"),
		pArea->low_r_vnum, pArea->hi_r_vnum,
		pArea->low_o_vnum, pArea->hi_o_vnum,
		pArea->low_m_vnum, pArea->hi_m_vnum );
    }

    for ( pArea = first_bsort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;
	if ( IS_SET( pArea->status, AREA_DELETED ) )
	   continue;
	else
	if (room)
	  if((pArea->low_r_vnum >= low_range) 
	  && (pArea->hi_r_vnum <= high_range))
	    area_conflict = TRUE;

	if (mob)
	  if((pArea->low_m_vnum >= low_range) 
	  && (pArea->hi_m_vnum <= high_range))
	    area_conflict = TRUE;

	if (obj)
	  if((pArea->low_o_vnum >= low_range) 
	  && (pArea->hi_o_vnum <= high_range))
	    area_conflict = TRUE;

	if (area_conflict)
	  sprintf(ch, "Conflict:%-15s| Rooms: %5d - %-5d"
		     " Objs: %5d - %-5d Mobs: %5d - %-5d\n\r",
		(pArea->filename ? pArea->filename : "(invalid)"),
		pArea->low_r_vnum, pArea->hi_r_vnum,
		pArea->low_o_vnum, pArea->hi_o_vnum,
		pArea->low_m_vnum, pArea->hi_m_vnum );
    }
*/
    return;
}

/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain( void )
{
    return;
}


/*
 * Initialize the weather for all the areas
 * Last Modified: July 21, 1997
 * Fireblade
 */
void init_area_weather()
{
	AREA_DATA *pArea;
	NEIGHBOR_DATA *neigh;
	NEIGHBOR_DATA *next_neigh;
	
	for(pArea = first_area; pArea; pArea = pArea->next)
	{
		int cf;
		
		/* init temp and temp vector */
		cf = pArea->weather->climate_temp - 2;
		pArea->weather->temp =
			number_range(-weath_unit, weath_unit) +
			cf * number_range(0, weath_unit);
		pArea->weather->temp_vector =
			cf + number_range(-rand_factor, rand_factor);
		
		/* init precip and precip vector */
		cf = pArea->weather->climate_precip - 2;
		pArea->weather->precip = 
			number_range(-weath_unit, weath_unit) +
			cf * number_range(0, weath_unit);
		pArea->weather->precip_vector =
			cf + number_range(-rand_factor, rand_factor);
			
		/* init wind and wind vector */
		cf = pArea->weather->climate_wind - 2;
		pArea->weather->wind = 
			number_range(-weath_unit, weath_unit) +
			cf * number_range(0, weath_unit);
		pArea->weather->wind_vector =
			cf + number_range(-rand_factor, rand_factor);

		/* check connections between neighbors */
		for(neigh = pArea->weather->first_neighbor; neigh;
				neigh = next_neigh)
		{
			AREA_DATA *tarea;
			NEIGHBOR_DATA *tneigh;
			
			/* get the address if needed */
			if(!neigh->address)
				neigh->address = get_area(neigh->name);

			/* area does not exist */
			if(!neigh->address
			|| str_cmp(neigh->name, neigh->address->name) )
			{
				tneigh = neigh;
				next_neigh = tneigh->next;
				UNLINK(tneigh,
					pArea->weather->first_neighbor,
					pArea->weather->last_neighbor,
					next,
					prev);
				STRFREE(tneigh->name);
				DISPOSE(tneigh);
				fold_area(pArea, pArea->filename, FALSE);
				continue;
			}
			
			/* make sure neighbors both point to each other */
			tarea = neigh->address;
			for(tneigh = tarea->weather->first_neighbor; tneigh;
					tneigh = tneigh->next)
			{
				if(!strcmp(pArea->name, tneigh->name))
					break;
			}
			
			if(!tneigh)
			{
				CREATE(tneigh, NEIGHBOR_DATA, 1);
				tneigh->name = STRALLOC(pArea->name);
				LINK(tneigh,
					tarea->weather->first_neighbor,
					tarea->weather->last_neighbor,
					next,
					prev);
				fold_area(tarea, tarea->filename, FALSE);
			}
			
			tneigh->address = pArea;
			
			next_neigh = neigh->next;
		}
	}
	
	return;
}

/*
 * Load weather data from appropriate file in system dir
 * Last Modified: July 24, 1997
 * Fireblade
 */
void load_weatherdata()
{
	char filename[MAX_INPUT_LENGTH];
	FILE *fp;
	
	sprintf(filename, "%sweather.dat", SYSTEM_DIR);
	
	if((fp = fopen(filename, "r")) != NULL)
	{
		for(;;)
		{
			char letter;
			char *word;
			
			letter = fread_letter(fp);
			
			if(letter != '#')
			{
				bug("load_weatherdata: # not found");
				return;
			}
			
			word = fread_word(fp);
			
			if(!str_cmp(word, "RANDOM"))
				rand_factor = fread_number(fp);
			else if(!str_cmp(word, "CLIMATE"))
				climate_factor = fread_number(fp);
			else if(!str_cmp(word, "NEIGHBOR"))
				neigh_factor = fread_number(fp);
			else if(!str_cmp(word, "UNIT"))
				weath_unit = fread_number(fp);
			else if(!str_cmp(word, "MAXVECTOR"))
				max_vector = fread_number(fp);
			else if(!str_cmp(word, "END"))
			{
				fclose(fp);
				break;
			}
			else
			{
				bug("load_weatherdata: unknown field");
				fclose(fp);
				break;
			}
		}
	}
	
	return;
}

/*
 * Write data for global weather parameters
 * Last Modified: July 24, 1997
 * Fireblade
 */
void save_weatherdata()
{
	char filename[MAX_INPUT_LENGTH];
	FILE *fp;
	
	sprintf(filename, "%sweather.dat", SYSTEM_DIR);
	
	if((fp = fopen(filename, "w")) != NULL)
	{
		fprintf(fp, "#RANDOM %d\n", rand_factor);
		fprintf(fp, "#CLIMATE %d\n", climate_factor);
		fprintf(fp, "#NEIGHBOR %d\n", neigh_factor);
		fprintf(fp, "#UNIT %d\n", weath_unit);
		fprintf(fp, "#MAXVECTOR %d\n", max_vector);
		fprintf(fp, "#END\n");
		fclose(fp);
	}
	else
	{
		bug("save_weatherdata: could not open file");
	}
	
	return;
}

void load_projects( void )  /* Copied load_boards structure for simplicity */
{
   char filename[MAX_INPUT_LENGTH];
   FILE *fp;
   PROJECT_DATA *project;

   first_project = NULL;
   last_project = NULL;
   sprintf( filename, "%s", PROJECTS_FILE );
   if( ( fp = fopen( filename, "r" ) ) == NULL )
      return;
   
   while( ( project = read_project( filename, fp ) ) != NULL )
      LINK( project, first_project, last_project, next, prev );
 
   return;
}

PROJECT_DATA *read_project( char *filename, FILE *fp )
{
   PROJECT_DATA *project;
   NOTE_DATA *log, *tlog;
   char *word;
   char buf[MAX_STRING_LENGTH];
   bool fMatch;
   char letter;
  
   do
   {
      letter = getc( fp );
      if( feof(fp) )
      {
	 fclose( fp );
	 return NULL;
      }
   }
   while( isspace(letter) );
   ungetc( letter, fp );

   CREATE( project, PROJECT_DATA, 1);

#ifdef KEY
#undef KEY
#endif
#define KEY( literal, field, value )                                    \
                                if ( !str_cmp( word, literal ) )        \
                                {                                       \
                                    field  = value;                     \
                                    fMatch = TRUE;                      \
                                    break;                              \
                                }
   project->first_log = NULL;
   project->last_log  = NULL;
   project->next       = NULL;
   project->prev       = NULL;
   project->coder = NULL;
   project->description = STRALLOC("");
   project->name = STRALLOC( "" );
   project->owner = STRALLOC( "None" );
   project->date = STRALLOC( "Not Set?!");
   project->status = STRALLOC( "No update." );

    for ( ; ; )
    {
        word   = feof( fp ) ? "End" : fread_word( fp );
        fMatch = FALSE;

        switch ( UPPER(word[0]) )
        {
        case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;
	case 'C':
		KEY("Coder",	project->coder,	fread_string_nohash(fp));
		break;
	case 'D':
		if (!str_cmp(word, "Date") )
			STRFREE(project->date);
		else if ( !str_cmp(word, "Description") )
			STRFREE(project->description);
		KEY("Date",		project->date,		fread_string( fp ));		
	   	KEY("Description",project->description,fread_string(fp));
		break;
        case 'E':
            if ( !str_cmp( word, "End" ) )
            {   
	      if ( !project->description )
	      	project->description = STRALLOC( "" );
	      if ( !project->name )
	        project->name = STRALLOC( "" );
	      if ( !project->owner )
	        project->owner = STRALLOC( "" );
	      if ( !project->date )
	        project->date = STRALLOC( "Not Set?!" );
	      if ( !project->status )
	        project->status = STRALLOC( "No update." );
	      if ( str_cmp(project->owner, "None") )
		project->taken = TRUE;
              return project;
            }
		break;
	case 'L':
	    if( !str_cmp( word, "Log" ) )
	    {
		fread_to_eol( fp );
		log = read_log( fp );
		if( !log )
		{
		   sprintf( buf, "read_project: couldn't read log, aborting");
		   bug( buf, 0 );
		   exit( 1 );
	        }
		if(!log->sender)
		  log->sender = STRALLOC( "" );
		if(!log->date)
		  log->date   = STRALLOC( "" );
		if(!log->subject)
		  log->subject = STRALLOC( "None" );
		  log->to_list = STRALLOC( "" );
		LINK( log, project->first_log, project->last_log, next, prev );
		fMatch = TRUE;
		break;
	    }
		break;
	case 'N':
	    if ( !str_cmp(word, "Name") )
	    	STRFREE(project->name );
	    KEY( "Name", 		project->name, 		fread_string_nohash( fp ) );
		break;
	case 'O':
	    if ( !str_cmp(word, "Owner") )
	    	STRFREE(project->owner);
	    KEY( "Owner", 		project->owner, 	fread_string( fp ) );
		break;
	case 'S':
	    if ( !str_cmp(word, "Status") )
	    	STRFREE(project->status);
	    KEY( "Status", 		project->status,	fread_string( fp ) );
		break;
        }
        if ( !fMatch ) 
        {
            sprintf( buf, "read_project: no match: %s", word );
            bug( buf, 0 );
        }
    }
  log = project->last_log;
  while ( log )
  {
    UNLINK( log, project->first_log, project->last_log, next, prev );
    tlog = log->prev;
    free_note( log );
    log = tlog;
  }
  if ( project->coder )
  	DISPOSE( project->coder );
  if ( project->description )
  	STRFREE( project->description );
  if ( project->name )
  	STRFREE( project->name );
  if ( project->owner )
  	STRFREE( project->owner );
  if ( project->date )
  	STRFREE( project->date );
  if ( project->status )
  	STRFREE( project->status );
  DISPOSE( project );
  return project;   
}

NOTE_DATA *read_log( FILE *fp )
{
    NOTE_DATA *log;
    char *word;
    CREATE( log, NOTE_DATA, 1 );
    
    for( ; ; )
    {
    word = fread_word( fp );

    if ( !str_cmp( word, "Sender" ) )
      log->sender   = fread_string( fp );
    else if ( !str_cmp( word, "Date" ) )
      log->date     = fread_string( fp );
    else if ( !str_cmp( word, "Subject" ) )
        log->subject  = fread_string( fp );
    else if ( !str_cmp( word, "Text" ) )
        log->text     = fread_string( fp );
    else if ( !str_cmp( word, "Endlog" ) )
    {
	fread_to_eol( fp );
        log->next             = NULL;
        log->prev             = NULL;
        return log;
    }	
    else
    {
        DISPOSE( log );
        bug( "read_log: bad key word.", 0 );
        return NULL;
    }
    }
}


void write_projects()
{
    PROJECT_DATA *project;
    NOTE_DATA *log;
    FILE *fpout;
    char filename[MAX_INPUT_LENGTH];
  
    sprintf( filename, "%s", PROJECTS_FILE );
    fpout = fopen( filename, "w" );
    if ( !fpout )
    {
        bug( "FATAL: cannot open projects.txt for writing!\n\r", 0 );
        return;
    }
    for ( project = first_project; project; project = project->next )
    {
	fprintf( fpout, "Name		   %s~\n",  project->name	    );
       	fprintf( fpout, "Owner		   %s~\n",  (project->owner) ? project->owner : "None" );
	if ( project->coder )
	 fprintf( fpout, "Coder		    %s~\n", project->coder );
	fprintf( fpout, "Status		   %s~\n",  (project->status) ? project->status : "No update." );
 	fprintf( fpout, "Date		   %s~\n",  (project->date) ? project->date : "Not Set?!?" );
	if ( project->description )
	  fprintf( fpout, "Description         %s~\n", project->description );
	for( log = project->first_log; log; log = log->next )
	  fprintf( fpout, "Log\nSender %s~\nDate %s~\nSubject %s~\nText %s~\nEndlog\n",
                log->sender,
                log->date,
                log->subject,
                log->text
                );
    
        fprintf( fpout, "End\n" );
    }
    fclose( fpout );
}

/* load_loginmsg, check_loginmsg, fread_loginmsg, etc.. all support the do_message */
/* command - hugely modified from the orginal housing module by Edmond June 02     */ 
void load_loginmsg( )
{
	FILE *fp;
	char filename[MAX_INPUT_LENGTH];
 
	first_lmsg = NULL;
	last_lmsg = NULL;
 
	sprintf( filename, "%s%s", SYSTEM_DIR, LOGIN_MSG );
	if ( ( fp = fopen( filename, "r" ) ) == NULL )
	{
		boot_log( "Load_loginmsg: Cannot open login message file." );
		return;
	}
	
	for ( ;; )
	{
		char letter;
		char *word;
		
		letter = fread_letter( fp );
		
		if ( letter == '*' )
		{
			fread_to_eol(fp);
			continue;
		}
		
		if ( letter != '#' )
		{
			bug ( "load_loginmsg: # not found. ", 0);
			break;
		}
		
		word = fread_word( fp );
		
		if ( !str_cmp( word, "LOGINMSG" ) )
		{
			fread_loginmsg( fp );
			continue;
		}
		else
		if ( !str_cmp( word, "END" ) )
			break;
		else
		{
			boot_log( "Load_loginmsg: bad section." );
			continue;
		}
	}
	
	fclose( fp );
	fp = NULL;
}
 
void fread_loginmsg( FILE *fp )
{
	LMSG_DATA *lmsg = NULL;
	char buf[MAX_STRING_LENGTH];
 
		CREATE( lmsg, LMSG_DATA, 1 );
 
#if defined(KEY)
#undef KEY
#endif
 
#define KEY( literal, field, value )					\
	if ( !str_cmp( word, literal ) )					\
	{													\
		field = value;									\
		fMatch = TRUE;									\
		break;											\
	}													\
	
	for ( ;; )
	{
		char *word;
		bool fMatch;
		
		word = fread_word( fp );
		fMatch = FALSE;
		
		switch ( UPPER( word[0] ) )
		{
			case '*':
			fMatch = TRUE;
			fread_to_eol( fp );
			break;
			
			case 'E':
			if ( !str_cmp( word, "End" ))
			{
				char checkname[MAX_STRING_LENGTH];
			

				if ( !lmsg->name || lmsg->name[0] == '\0' )
				{
					bug( "load_loginmsg: Login message with no name", 0 );
					STRFREE( lmsg->text );
					DISPOSE( lmsg );
					return;
				}
				else
				{
				    sprintf( checkname, "%s%c/%s", PLAYER_DIR, tolower(lmsg->name[0]),
                                 capitalize( lmsg->name ) );

    				if ( access( checkname, F_OK ) != 0 )
					{
						bug( "load_loginmsg: Login message expired - %s no longer exists", lmsg->name );
						STRFREE( lmsg->name );
						STRFREE( lmsg->text );
						DISPOSE( lmsg );
						return;
					}
				}	
			
				LINK( lmsg, first_lmsg, last_lmsg, next, prev );
				return;
			}
			break;
						
			case 'N':
			KEY( "Name", 	lmsg->name, 		fread_string( fp ) );
			break;
			
			case 'T':
			KEY( "Type",	lmsg->type, 		fread_number ( fp ) );
			KEY( "Text", 	lmsg->text,			fread_string ( fp ) );
			break;
			
		}
		
		if ( !fMatch )
		{
			sprintf( buf, "Load_loginmsg: no match: %s", word );
			bug ( buf, 0 );
		}
	}
						
}
 
void save_loginmsg( )
{
	FILE *fp;
	char filename[MAX_INPUT_LENGTH];
	LMSG_DATA *lmsg;
	
	sprintf( filename, "%s%s", SYSTEM_DIR, LOGIN_MSG );
	if ( ( fp = fopen( filename, "w" ) ) == NULL )
	{
		bug( "Save_loginmsg: Cannot open login message file." );
		return;
	}
 
	for ( lmsg = first_lmsg; lmsg; lmsg = lmsg->next )
	{
		fprintf( fp, "#LOGINMSG\n" );
		fprintf( fp, "Name           %s~\n", lmsg->name );
		if ( lmsg->text )
			fprintf( fp, "Text           %s~\n", lmsg->text );
		fprintf( fp, "Type           %d\n", lmsg->type );
		fprintf( fp, "End\n" );
	}
 
	fprintf( fp, "#END\n" );
	fclose( fp );
	fp = NULL;
}
 
void add_loginmsg( char *name, sh_int type, char *argument )
{
	LMSG_DATA *lmsg;
	
        if ( type < 0 || !name || name[0] == '\0' )
       {
               bug ( "add_loginmsg: bad name or type", 0 );
               return;
       }
	
	CREATE( lmsg, LMSG_DATA, 1 );
	
	lmsg->type = type;
	lmsg->name = STRALLOC( name );
	if ( argument && argument[0] != '\0')
		lmsg->text = STRALLOC( argument );
 
	LINK( lmsg, first_lmsg, last_lmsg, next, prev );
	save_loginmsg();
 
	return;
}

char *	const	login_msg[] =
{
/*0*/	"",
/*1*/	"\n\r&GYou did not have enough money for the residence you bid on.\n\r"
            "It has been readded to the auction and you've been penalized.\n\r",
/*2*/	"\n\r&GThere was an error in looking up the seller for the residence\n\r"
            "you had bid on. Residence removed and no interaction has taken place.\n\r",
/*3*/	"\n\r&GThere was no bidder on your residence. Your residence has been\n\r"
            "removed from auction and you have been penalized.\n\r",
/*4*/	"\n\r&GYou have successfully received your new residence.\n\r",
/*5*/	"\n\r&GYou have successfully sold your residence.\n\r",
/*6*/	"\n\r&RYou have been outcast from your clan/order/guild.  Contact a leader\n\r"
             "of that organization if you have any questions.\n\r",
/*7*/	"\n\r&RYou have been silenced.  Contact an immortal if you wish to discuss\n\r"
	     "your sentence.\n\r",
/*8*/	"\n\r&RYou have lost your ability to set your title.  Contact an immortal if you\n\r"
	     "wish to discuss your sentence.\n\r",
/*9*/	"\n\r&RYou have lost your ability to set your biography.  Contact an immortal if\n\r"
	     "you wish to discuss your sentence.\n\r",
/*10*/	"\n\r&RYou have been sent to hell.  You will be automatically released when your\n\r"
	     "sentence is up.  Contact an immortal if you wish to discuss your sentence.\n\r",
/*11*/	"\n\r&RYou have lost your ability to set your own description.  Contact an\n\r"
	     "immortal if you wish to discuss your sentence.\n\r",
/*12*/	"\n\r&RYou have lost your ability to set your homepage address.  Contact an\n\r"
	     "immortal if you wish to discuss your sentence.\n\r",
/*13*/	"\n\r&RYou have lost your ability to \"beckon\" other players.  Contact an\n\r"
             "immortal if you wish to discuss your sentence.\n\r",
/*14*/	"\n\r&RYou have lost your ability to send tells.  Contact an immortal if\n\r"
 	     "you wish to discuss your sentence.\n\r",
/*15*/	"\n\r&CYour character has been frozen.  Contact an immortal if you wish\n\r"
	     "to discuss your sentence.\n\r",
/*16*/	"\n\r&RYou have lost your ability to emote.  Contact an immortal if\n\r"
 	     "you wish to discuss your sentence.\n\r",
/*17*/  "RESERVED FOR LINKDEAD DEATH MESSAGES",
/*18*/	"RESERVED FOR CODE-SENT MESSAGES"
};	
/* MAX_MSG = 18 - IF ADDING MESSAGE TYPES, ENSURE YOU BUMP THIS VALUE IN MUD.H */

void check_loginmsg( CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];
	LMSG_DATA *lmsg, *lmsg_next;
		
	if ( !ch || IS_NPC( ch ))
	return;
	
	for( lmsg = first_lmsg; lmsg; lmsg = lmsg_next )
	{
		lmsg_next = lmsg->next;
		
		if (!str_cmp( lmsg->name, ch->name ))
		{
		
			if ( lmsg->type > MAX_MSG )
			{
	                sprintf( buf, "Error: Unknown login msg: %d for %s.", lmsg->type, ch->name);
                		bug( buf, 0 );
			}

			switch (lmsg->type)
			{
			case 0:/* Imm sent message */
			{			
				if ( !lmsg->text || lmsg->text[0] == '\0' )
				{
					bug( "check_loginmsg: NULL loginmsg text for type 0", 0 );
					STRFREE( lmsg->name );
					UNLINK( lmsg, first_lmsg, last_lmsg, next, prev );
					DISPOSE( lmsg );
					continue;
				}
				sprintf( buf, "\n\r&YThe game administrators have left you the following message:\n\r%s\n\r", lmsg->text );			
				send_to_char_color( buf, ch );
				break;
			}
			case 17: /* Death message */
			{
				if ( !lmsg->text || lmsg->text[0] == '\0' )
				{
					bug( "check_loginmsg: NULL loginmsg text for type 17", 0 );
					STRFREE( lmsg->name );
					UNLINK( lmsg, first_lmsg, last_lmsg, next, prev );
					DISPOSE( lmsg );
					continue;
				}
				sprintf( buf, "\n\r&RYou were killed by %s while your character was link-dead.\n\r", lmsg->text );
				send_to_char_color( buf, ch );
				sprintf( buf, "You should look for your corpse immediately.\n\r" );
				send_to_char_color( buf, ch );
				break;
			}
                        case 18:/* Code-sent message for 'World change' notice */
                        {
                                if ( !lmsg->text || lmsg->text[0] == '\0' )
				{
                                        bug( "check_loginmsg: NULL loginmsg text for type 18", 0 );
                                        STRFREE( lmsg->name );
                                        UNLINK( lmsg, first_lmsg, last_lmsg, next, prev );
                                        DISPOSE( lmsg );
                                        continue;
                                }
                                sprintf( buf, "\n\r&GA change in the Realms has affected you personally:\n\r%s\n\r", lmsg->text );
                                send_to_char_color( buf, ch );
                                break;
                        }
			default:
			send_to_char_color( login_msg[lmsg->type], ch );
			break;
			}
 
			STRFREE( lmsg->name );
			if ( lmsg->text )
			STRFREE( lmsg->text );
			UNLINK( lmsg, first_lmsg, last_lmsg, next, prev );
			DISPOSE( lmsg );
			save_loginmsg();
		}
	}
 
	return;
}
