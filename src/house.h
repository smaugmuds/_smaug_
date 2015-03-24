/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
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
 *                     Housing Module Header File                           *
 ****************************************************************************
 * Author : Senir                                                           *
 * E-Mail : oldgaijin@yahoo.com                                             *
 ****************************************************************************/

/* So other snippets know this is installed */
#define HOUSING_CODE

/* Comment the line if you don't want support for house mobs */
#define HOUSE_MOBS

/* Uncomment the line below if you've installed a banking snippet */
//#define BANK_INSTALLED

/* Uncomment the line below if you've changed your room flags into extended bitvectors*/
//#define EXTENDED_ROOMS

/* Uncomment the line below and change the area if you want to search another area for free
 * rooms to turn into additional housing rooms, instead of having it search the area where
 * the original house room is.
 */
#define ADDED_ROOM_HOUSING_AREA "houses.are"

/* Location of housing directory */
#define HOUSE_DIR             "../houses/"
/* Location of housing list for loadup of houses */
#define HOUSE_LIST            "house.lst" 
/* Location of automated housing auction file */
#define HOMEBUY_FILE          HOUSE_DIR "homebuy.dat"
/* Location of house accessories file */
#define ACCESSORIES_FILE      HOUSE_DIR "homeaccessories.dat"

/* Minimum amount a house may go up on auction for */
#define MIN_HOUSE_BID 5000000
/* Minimum amount an apartment may go up on auction for */
#define MIN_APARTMENT_BID 2000000
/* Default percentage of current bid to be used for the bid increment */
#define DEFAULT_BID_INCREMENT_PERCENTAGE 3
/* Percentage of bid that will be taken from either the bidder or seller for a penalty */
#define PENALTY_PERCENTAGE 20

/* Cost of a player creating another room for their house using 'home addroom' */
#define ADDITIONAL_ROOM_COST 100000000
/* Max number of rooms allowed for a player house, this includes the first room given the player */
#define MAX_HOUSE_ROOMS 5

#ifdef HOUSE_MOBS
/* Default price for a mob on accessories. Used by the house mobs. */
  #define DEFAULT_MOB_PRICE  50000
#endif

/*For automated housing auction*/
typedef struct  homebuy_data   HOMEBUY_DATA; 
struct homebuy_data
{
    HOMEBUY_DATA *prev;
    HOMEBUY_DATA *next;
    char *bidder;
    char *seller;
    int vnum;
	sh_int incpercent; 
    int bid;    
    int endtime;
    bool apartment;  
}; 

HOMEBUY_DATA *first_homebuy;
HOMEBUY_DATA *last_homebuy;

/* For basic housing */
typedef struct  home_data   HOME_DATA; 
struct home_data
{
    HOME_DATA   *prev;
    HOME_DATA   *next;
	char        *name;
	bool        apartment;
    int         vnum[MAX_HOUSE_ROOMS];
}; 

HOME_DATA *first_home;
HOME_DATA *last_home;


/* For the accessories auction */
typedef struct  accessories_data   ACCESSORIES_DATA; 
struct accessories_data
{
    ACCESSORIES_DATA *prev;
    ACCESSORIES_DATA *next;
    int vnum;
    int price;
	bool mob;
}; 

ACCESSORIES_DATA *first_accessory;
ACCESSORIES_DATA *last_accessory;


/* Commands added for housing system. */
DECLARE_DO_FUN( do_accessories); // Only command for housing accessories.
DECLARE_DO_FUN( do_gohome);      // Command for a player to be instantly transported to their residence.
DECLARE_DO_FUN( do_homebuy);     // Main housing auction command.
DECLARE_DO_FUN( do_house);       // House personalization command and also immortal command to manually 
                                 // edit residences.
DECLARE_DO_FUN( do_residence);   // Command to list current houses and their stats.
DECLARE_DO_FUN( do_sellhouse);   // Player command to sell their residence and place it on auction.

/* Functions for basic housing functionality and functionality of the house command. */
void save_residence(ROOM_INDEX_DATA *location);
bool set_house( CHAR_DATA *ch, int vnum, bool apartment);
bool remove_house( CHAR_DATA *ch);
bool add_room( HOME_DATA *homedata, ROOM_INDEX_DATA *location, char *argument);
bool give_key( CHAR_DATA *ch, int vnum, char *name, bool apt );
void fwrite_house(HOME_DATA *homedata);
void load_homedata();
bool load_house_file( char *name );
void update_house_list( );
int fread_house( FILE *fp );
void save_house_by_vnum(int vnum);
bool in_same_house( CHAR_DATA *ch, CHAR_DATA *vch );

/* Functions for accessories functionality */
void save_accessories();
void load_accessories();
void fread_accessories( FILE *fp );

/* Functions for housing auction functionality */
bool add_homebuy( CHAR_DATA *seller, int vnum, bool apartment, int price);
bool remove_homebuy(HOMEBUY_DATA *tmphome);
void save_homebuy();
void load_homebuy();
void fread_homebuy( FILE *fp );
void homebuy_update();
bool check_parse_name( char *name, bool newchar ); /* Used from comm.c to prevent /dev/null bug */
CHAR_DATA *load_player( char *name );
void logoff( CHAR_DATA *ch );

/*Function declarations from reset.c used for house mobs resets and removal of resets in general.*/
void delete_reset   ( AREA_DATA *pArea, RESET_DATA *pReset );   
bool is_room_reset  ( RESET_DATA *pReset, ROOM_INDEX_DATA *aRoom, AREA_DATA *pArea );    
