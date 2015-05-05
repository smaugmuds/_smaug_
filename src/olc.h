/*
                     R E A L M S    O F    D E S P A I R  !
   ___________________________________________________________________________
  //            /                                                            \\
 [|_____________\   ********   *        *   ********   *        *   *******   |]
 [|   \\._.//   /  **********  **      **  **********  **      **  *********  |]
 [|   (0...0)   \  **********  ***    ***  **********  ***    ***  *********  |]
 [|    ).:.(    /  ***         ****  ****  ***    ***  ***    ***  ***        |]
 [|    {o o}    \  *********   **********  **********  ***    ***  *** ****   |]
 [|   / ' ' \   /   *********  *** ** ***  **********  ***    ***  ***  ****  |]
 [|-'- /   \ -`-\         ***  ***    ***  ***    ***  ***    ***  ***   ***  |]
 [|   .VxvxV.   /   *********  ***    ***  ***    ***  **********  *********  |]
 [|_____________\  **********  **      **  **      **  **********  *********  |]
 [|             /  *********   *        *  *        *   ********    *******   |]
  \\____________\____________________________________________________________//
     |                                                                     |
     |    --{ [S]imulated [M]edieval [A]dventure Multi[U]ser [G]ame }--    |
     |_____________________________________________________________________|
     |                                                                     |
     |                 -*- OLC II Defines, structs, etc -*-                |
     |_____________________________________________________________________|
     |                                                                     |
     |     OasisOLC II for Smaug 1.40 written by Evan Cortens(Tagith)      |
     |                                                                     |
     |   Based on OasisOLC for CircleMUD3.0bpl9 written by Harvey Gilpin   |
     |_____________________________________________________________________|
    //                                                                     \\
   [|  SMAUG 2.0 © 2014-2015 Antonio Cao (@burzumishi)                      |]
   [|                                                                       |]
   [|  AFKMud Copyright 1997-2007 by Roger Libiez (Samson),                 |]
   [|  Levi Beckerson (Whir), Michael Ward (Tarl), Erik Wolfe (Dwip),       |]
   [|  Cameron Carroll (Cam), Cyberfox, Karangi, Rathian, Raine,            |]
   [|  Xorith, and Adjani.                                                  |]
   [|  All Rights Reserved. External contributions from Remcon, Quixadhal,  |]
   [|  Zarius and many others.                                              |]
   [|                                                                       |]
   [|  SMAUG 1.4 © 1994-1998 Thoric/Altrag/Blodkai/Narn/Haus/Scryn/Rennard  |]
   [|  Swordbearer/Gorog/Grishnakh/Nivek/Tricops/Fireblade/Edmond/Conran    |]
   [|                                                                       |]
   [|  Merc 2.1 Diku Mud improvments © 1992-1993 Michael Chastain, Michael  |]
   [|  Quan, and Mitchell Tse. Original Diku Mud © 1990-1991 by Sebastian   |]
   [|  Hammer, Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, Katja    |]
   [|  Nyboe. Win32 port Nick Gammon.                                       |]
    \\_____________________________________________________________________//
*/

#define MAX_RIS_FLAG 21
#define MAX_OLC_ITEMS_LIST 51

/* Global function definitions */
void oedit_parse( DESCRIPTOR_DATA *d, char *arg );
void medit_parse( DESCRIPTOR_DATA *d, char *arg );
void redit_parse( DESCRIPTOR_DATA *d, char *arg );

/*. Utils exported from olc.c .*/
void strip_string( char * );
void cleanup_olc( DESCRIPTOR_DATA *d );
void olc_log( DESCRIPTOR_DATA *d, char *format, ... );


/*. OLC structs .*/

struct olc_data
{
    int mode;
    int zone_num;
    int number;
    int value;
    bool changed;

    MOB_INDEX_DATA *mob;
    ROOM_INDEX_DATA *room;
    OBJ_DATA *obj;
    AREA_DATA *area;
    SHOP_DATA *shop;
    EXTRA_DESCR_DATA *desc;
    AFFECT_DATA *paf;
    EXIT_DATA *xit;
};

struct olc_save_info
{
    int zone;
    char type;
    struct olc_save_info *next;
};

/*. Descriptor access macros .*/
#define OLC_MODE(d) 	((d)->olc->mode) 	/* Parse input mode	*/
#define OLC_NUM(d) 	((d)->olc->number)	/* Room/Obj VNUM 	*/
#define OLC_VNUM(d)	OLC_NUM(d)
#define OLC_VAL(d) 	((d)->olc->value)  	/* Scratch variable	*/
#define OLC_OBJ(d)	(obj)
#define OLC_DESC(d) 	((d)->olc->desc)	/* Extra description	*/
#define OLC_AFF(d)	((d)->olc->paf)		/* Affect data 		*/
#define OLC_CHANGE(d)	((d)->olc->changed)	/* Changed flag		*/
#define OLC_EXIT(d)     ((d)->olc->xit)		/* An Exit		*/

#ifdef OLD_CIRCLE_STYLE
# define OLC_ROOM(d)    ((d)->olc->room)        /* Room structure       */
# define OLC_OBJ(d)     ((d)->olc->obj)         /* Object structure     */
# define OLC_MOB(d)     ((d)->olc->mob)         /* Mob structure        */
# define OLC_SHOP(d)    ((d)->olc->shop)        /* Shop structure       */
# define OLC_EXIT(d)	(OLC_ROOM(d)->dir_option[OLC_VAL(d)])	
#endif

/*. Add/Remove save list types	.*/
#define OLC_SAVE_ROOM			0
#define OLC_SAVE_OBJ			1
#define OLC_SAVE_ZONE			2
#define OLC_SAVE_MOB			3
#define OLC_SAVE_SHOP			4

/* Submodes of OEDIT connectedness */
#define OEDIT_MAIN_MENU              	1
#define OEDIT_EDIT_NAMELIST          	2
#define OEDIT_SHORTDESC              	3
#define OEDIT_LONGDESC               	4
#define OEDIT_ACTDESC                	5
#define OEDIT_TYPE                   	6
#define OEDIT_EXTRAS                 	7
#define OEDIT_WEAR                  	8
#define OEDIT_WEIGHT                	9
#define OEDIT_COST                  	10
#define OEDIT_COSTPERDAY            	11
#define OEDIT_TIMER                 	12
#define OEDIT_VALUE_1               	13
#define OEDIT_VALUE_2               	14
#define OEDIT_VALUE_3               	15
#define OEDIT_VALUE_4               	16
#define OEDIT_VALUE_5			17
#define OEDIT_VALUE_6			18
#define OEDIT_EXTRADESC_KEY         	19
#define OEDIT_CONFIRM_SAVEDB        	20
#define OEDIT_CONFIRM_SAVESTRING    	21
#define OEDIT_EXTRADESC_DESCRIPTION 	22
#define OEDIT_EXTRADESC_MENU        	23
#define OEDIT_LEVEL                 	24
#define OEDIT_LAYERS			25
#define OEDIT_AFFECT_MENU		26
#define OEDIT_AFFECT_LOCATION		27
#define OEDIT_AFFECT_MODIFIER		28
#define OEDIT_AFFECT_REMOVE		29
#define OEDIT_AFFECT_RIS		30
#define OEDIT_EXTRADESC_CHOICE		31
#define OEDIT_EXTRADESC_DELETE		32
#define OEDIT_MPROGS			33
#define OEDIT_MPROGS_CHOICE		34
#define OEDIT_MPROGS_DELETE		35
#define OEDIT_MPROGS_TYPE		36
#define OEDIT_MPROGS_ARG		37


/* Submodes of REDIT connectedness */
#define REDIT_MAIN_MENU 		1
#define REDIT_NAME 			2
#define REDIT_DESC 			3
#define REDIT_FLAGS 			4
#define REDIT_SECTOR 			5
#define REDIT_EXIT_MENU 		6
#define REDIT_CONFIRM_SAVEDB 		7
#define REDIT_CONFIRM_SAVESTRING 	8
#define REDIT_EXIT_VNUM 		9
#define REDIT_EXIT_DESC 		10
#define REDIT_EXIT_KEYWORD 		11
#define REDIT_EXIT_KEY 			12
#define REDIT_EXIT_FLAGS 		13
#define REDIT_EXTRADESC_MENU 		14
#define REDIT_EXTRADESC_KEY 		15
#define REDIT_EXTRADESC_DESCRIPTION 	16
#define REDIT_TUNNEL			17
#define REDIT_TELEDELAY			18
#define REDIT_TELEVNUM			19
#define REDIT_EXIT_EDIT			20
#define REDIT_EXIT_ADD			21
#define REDIT_EXIT_DELETE		22
#define REDIT_EXIT_ADD_VNUM		23
#define REDIT_EXTRADESC_DELETE		24
#define REDIT_EXTRADESC_CHOICE		25


/*. Submodes of ZEDIT connectedness 	.*/
#define ZEDIT_MAIN_MENU              	0
#define ZEDIT_DELETE_ENTRY		1
#define ZEDIT_NEW_ENTRY			2
#define ZEDIT_CHANGE_ENTRY		3
#define ZEDIT_COMMAND_TYPE		4
#define ZEDIT_IF_FLAG			5
#define ZEDIT_ARG1			6
#define ZEDIT_ARG2			7
#define ZEDIT_ARG3			8
#define ZEDIT_ZONE_NAME			9
#define ZEDIT_ZONE_LIFE			10
#define ZEDIT_ZONE_TOP			11
#define ZEDIT_ZONE_RESET		12
#define ZEDIT_CONFIRM_SAVESTRING	13


/*. Submodes of MEDIT connectedness 	.*/
#define MEDIT_NPC_MAIN_MENU		0
#define MEDIT_PC_MAIN_MENU		1
#define MEDIT_NAME 			2
#define MEDIT_S_DESC			3
#define MEDIT_L_DESC			4
#define MEDIT_D_DESC			5
#define MEDIT_NPC_FLAGS			6
#define MEDIT_PC_FLAGS			7
#define MEDIT_AFF_FLAGS			8
#define MEDIT_CONFIRM_SAVESTRING	9
#define MEDIT_SEX			10
#define MEDIT_HITROLL			11
#define MEDIT_DAMROLL			12
#define MEDIT_DAMNUMDIE			13
#define MEDIT_DAMSIZEDIE		14
#define MEDIT_DAMPLUS			15
#define MEDIT_HITNUMDIE			16
#define MEDIT_HITSIZEDIE		17
#define MEDIT_HITPLUS			18
#define MEDIT_AC			19
#define MEDIT_GOLD			20
#define MEDIT_POS			21
#define MEDIT_DEFAULT_POS		22
#define MEDIT_ATTACK			23
#define MEDIT_DEFENSE			24
#define MEDIT_LEVEL			25
#define MEDIT_ALIGNMENT			26
#define MEDIT_STRENGTH			27
#define MEDIT_INTELLIGENCE		28
#define MEDIT_WISDOM			29
#define MEDIT_DEXTERITY			30
#define MEDIT_CONSTITUTION		31
#define MEDIT_CHARISMA			32
#define MEDIT_LUCK			33
#define MEDIT_CLAN			34
#define MEDIT_DEITY			35
#define MEDIT_COUNCIL			36
#define MEDIT_SPEC			37
#define MEDIT_RESISTANT			38
#define MEDIT_IMMUNE			39
#define MEDIT_SUSCEPTIBLE		40
#define MEDIT_PCDATA_FLAGS		41
#define MEDIT_MENTALSTATE		42
#define MEDIT_EMOTIONAL			43
#define MEDIT_THIRST			44
#define MEDIT_FULL			45
#define MEDIT_DRUNK			46
#define MEDIT_PARTS			47
#define MEDIT_FAVOR			48
#define MEDIT_HITPOINT			49
#define MEDIT_MANA			50
#define MEDIT_MOVE			51
#define MEDIT_PRACTICE			52
#define MEDIT_PASSWORD			53
#define MEDIT_SAVE_MENU			54
#define MEDIT_SAV1			55
#define MEDIT_SAV2			56
#define MEDIT_SAV3			57
#define MEDIT_SAV4			58
#define MEDIT_SAV5			59
#define MEDIT_CLASS			60
#define MEDIT_RACE			61

/*. Submodes of SEDIT connectedness 	.*/
#define SEDIT_MAIN_MENU              	0
#define SEDIT_CONFIRM_SAVESTRING	1
#define SEDIT_NOITEM1			2
#define SEDIT_NOITEM2			3
#define SEDIT_NOCASH1			4
#define SEDIT_NOCASH2			5
#define SEDIT_NOBUY			6
#define SEDIT_BUY			7
#define SEDIT_SELL			8
#define SEDIT_PRODUCTS_MENU		11
#define SEDIT_ROOMS_MENU		12
#define SEDIT_NAMELIST_MENU		13
#define SEDIT_NAMELIST			14
/*. Numerical responses .*/
#define SEDIT_NUMERICAL_RESPONSE	20
#define SEDIT_OPEN1			21
#define SEDIT_OPEN2			22
#define SEDIT_CLOSE1			23
#define SEDIT_CLOSE2			24
#define SEDIT_KEEPER			25
#define SEDIT_BUY_PROFIT		26
#define SEDIT_SELL_PROFIT		27
#define SEDIT_TYPE_MENU			29
#define SEDIT_DELETE_TYPE		30
#define SEDIT_DELETE_PRODUCT		31
#define SEDIT_NEW_PRODUCT		32
#define SEDIT_DELETE_ROOM		33
#define SEDIT_NEW_ROOM			34
#define SEDIT_SHOP_FLAGS		35
#define SEDIT_NOTRADE			36
