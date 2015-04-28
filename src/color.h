/****************************************************************************
 *                   ^     +----- |  / ^     ^ |     | +-\                  *
 *                  / \    |      | /  |\   /| |     | |  \                 *
 *                 /   \   +---   |<   | \ / | |     | |  |                 *
 *                /-----\  |      | \  |  v  | |     | |  /                 *
 *               /       \ |      |  \ |     | +-----+ +-/                  *
 ****************************************************************************
 * AFKMud Copyright 1997-2005 by Roger Libiez (Samson),                     *
 * Levi Beckerson (Whir), Michael Ward (Tarl), Erik Wolfe (Dwip),           *
 * Cameron Carroll (Cam), Cyberfox, Karangi, Rathian, Raine, and Adjani.    *
 * All Rights Reserved.                                                     *
 * Registered with the United States Copyright Office. TX 5-877-286         *
 *                                                                          *
 * External contributions from Xorith, Quixadhal, Zarius, and many others.  *
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
 *               Color Module -- Allow user customizable Colors.            *
 *                                   --Matthew                              *
 *                      Enhanced ANSI parser by Samson                      *
 ****************************************************************************/

void reset_colors( CHAR_DATA * );
void set_char_color( short AType, CHAR_DATA * ch );
void set_pager_color( short AType, CHAR_DATA * ch );
char *color_str( short AType, CHAR_DATA * ch );
char *color_align( const char *argument, int size, int align );
int color_strlen( const char *src );
char *colorize( const char *txt, DESCRIPTOR_DATA * d );

DECLARE_DO_FUN( do_color );

/*
 * Color Alignment Parameters
 */
#define ALIGN_LEFT	1
#define ALIGN_CENTER	2
#define ALIGN_RIGHT	3

/* These are the ANSI codes for foreground text colors */
#define ANSI_BLACK    	"\033[0;30m"
#define ANSI_DRED    	"\033[0;31m"
#define ANSI_DGREEN     "\033[0;32m"
#define ANSI_ORANGE    	"\033[0;33m"
#define ANSI_DBLUE    	"\033[0;34m"
#define ANSI_PURPLE    	"\033[0;35m"
#define ANSI_CYAN	  	"\033[0;36m"
#define ANSI_GREY		"\033[0;37m"
#define ANSI_DGREY	"\033[1;30m"
#define ANSI_RED		"\033[1;31m"
#define ANSI_GREEN	"\033[1;32m"
#define ANSI_YELLOW   	"\033[1;33m"
#define ANSI_BLUE		"\033[1;34m"
#define ANSI_PINK		"\033[1;35m"
#define ANSI_LBLUE   	"\033[1;36m"
#define ANSI_WHITE   	"\033[1;37m"

/* These are the ANSI codes for blinking foreground text colors */
#define BLINK_BLACK		"\033[0;5;30m"
#define BLINK_DRED		"\033[0;5;31m"
#define BLINK_DGREEN		"\033[0;5;32m"
#define BLINK_ORANGE		"\033[0;5;33m"
#define BLINK_DBLUE		"\033[0;5;34m"
#define BLINK_PURPLE		"\033[0;5;35m"
#define BLINK_CYAN		"\033[0;5;36m"
#define BLINK_GREY		"\033[0;5;37m"
#define BLINK_DGREY		"\033[1;5;30m"
#define BLINK_RED			"\033[1;5;31m"
#define BLINK_GREEN		"\033[1;5;32m"
#define BLINK_YELLOW		"\033[1;5;33m"
#define BLINK_BLUE		"\033[1;5;34m"
#define BLINK_PINK		"\033[1;5;35m"
#define BLINK_LBLUE		"\033[1;5;36m"
#define BLINK_WHITE		"\033[1;5;37m"

/* These are the ANSI codes for background colors */
#define BACK_BLACK 	"\033[40m"
#define BACK_DRED  	"\033[41m"
#define BACK_DGREEN	"\033[42m"
#define BACK_ORANGE     "\033[43m"
#define BACK_DBLUE      "\033[44m"
#define BACK_PURPLE     "\033[45m"
#define BACK_CYAN       "\033[46m"
#define BACK_GREY       "\033[47m"
#define BACK_DGREY    	"\033[50m"
#define BACK_RED       	"\033[51m"
#define BACK_GREEN    	"\033[52m"
#define BACK_YELLOW    	"\033[53m"
#define BACK_BLUE    	"\033[54m"
#define BACK_PINK   	"\033[55m"
#define BACK_LBLUE    	"\033[56m"
#define BACK_WHITE    	"\033[57m"

/* Other miscelaneous ANSI tags that can be used */
#define ANSI_RESET	"\033[0m"   /* Reset to terminal default */
#define ANSI_BOLD		"\033[1m"   /* For bright color stuff */
#define ANSI_ITALIC	"\033[3m"   /* Italic text */
#define ANSI_UNDERLINE  "\033[4m"   /* Underline text */
#define ANSI_BLINK	"\033[5m"   /* Blinking text */
#define ANSI_REVERSE    "\033[7m"   /* Reverse colors */
#define ANSI_STRIKEOUT  "\033[9m"   /* Overstrike line */

#define AT_BLACK         0
#define AT_BLOOD         1
#define AT_DGREEN        2
#define AT_ORANGE        3
#define AT_DBLUE         4
#define AT_PURPLE        5
#define AT_CYAN          6
#define AT_GREY          7
#define AT_DGREY         8
#define AT_RED           9
#define AT_GREEN        10
#define AT_YELLOW       11
#define AT_BLUE         12
#define AT_PINK         13
#define AT_LBLUE        14
#define AT_WHITE        15
#define AT_BLINK        16

/* These should be 17 - 31 normaly */
#define AT_BLACK_BLINK  AT_BLACK + AT_BLINK
#define AT_BLOOD_BLINK  AT_BLOOD + AT_BLINK
#define AT_DGREEN_BLINK AT_DGREEN + AT_BLINK
#define AT_ORANGE_BLINK AT_ORANGE + AT_BLINK
#define AT_DBLUE_BLINK  AT_DBLUE + AT_BLINK
#define AT_PURPLE_BLINK AT_PURPLE + AT_BLINK
#define AT_CYAN_BLINK   AT_CYAN + AT_BLINK
#define AT_GREY_BLINK   AT_GREY + AT_BLINK
#define AT_DGREY_BLINK  AT_DGREY + AT_BLINK
#define AT_RED_BLINK    AT_RED + AT_BLINK
#define AT_GREEN_BLINK  AT_GREEN + AT_BLINK
#define AT_YELLOW_BLINK AT_YELLOW + AT_BLINK
#define AT_BLUE_BLINK   AT_BLUE + AT_BLINK
#define AT_PINK_BLINK   AT_PINK + AT_BLINK
#define AT_LBLUE_BLINK  AT_LBLUE + AT_BLINK
#define AT_WHITE_BLINK  AT_WHITE + AT_BLINK

#define AT_PLAIN        32
#define AT_ACTION       33
#define AT_SAY        34
#define AT_GOSSIP       35
#define AT_YELL        36
#define AT_TELL        37
#define AT_HIT        38
#define AT_HITME        39
#define AT_IMMORT       40
#define AT_HURT         41
#define AT_FALLING      42
#define AT_DANGER       43
#define AT_MAGIC        44
#define AT_CONSIDER     45
#define AT_REPORT       46
#define AT_POISON       47
#define AT_SOCIAL       48
#define AT_DYING        49
#define AT_DEAD         50
#define AT_SKILL        51
#define AT_CARNAGE      52
#define AT_DAMAGE       53
#define AT_FLEE         54
#define AT_RMNAME       55
#define AT_RMDESC       56
#define AT_OBJECT       57
#define AT_PERSON       58
#define AT_LIST         59
#define AT_BYE          60
#define AT_GOLD         61
#define AT_GTELL        62
#define AT_NOTE         63
#define AT_HUNGRY       64
#define AT_THIRSTY      65
#define AT_FIRE         66
#define AT_SOBER        67
#define AT_WEAROFF      68
#define AT_EXITS        69
#define AT_SCORE        70
#define AT_RESET        71
#define AT_LOG          72
#define AT_DIEMSG       73
#define AT_WARTALK      74
#define AT_ARENA        75
#define AT_MUSE         76
#define AT_THINK        77
#define AT_AFLAGS          78 /* Added by Samson 9-29-98 for area flag display line */
#define AT_WHO            79 /* Added by Samson 9-29-98 for wholist */
#define AT_RACETALK       80 /* Added by Samson 9-29-98 for version 1.4 code */
#define AT_IGNORE         81 /* Added by Samson 9-29-98 for version 1.4 code */
#define AT_WHISPER        82 /* Added by Samson 9-29-98 for version 1.4 code */
#define AT_DIVIDER        83 /* Added by Samson 9-29-98 for version 1.4 code */
#define AT_MORPH          84 /* Added by Samson 9-29-98 for version 1.4 code */
#define AT_SHOUT        85 /* Added by Samson 9-29-98 for shout channel */
#define AT_RFLAGS       86 /* Added by Samson 12-20-98 for room flag display line */
#define AT_STYPE        87 /* Added by Samson 12-20-98 for sector display line */
#define AT_ANAME        88 /* Added by Samson 12-20-98 for filename display line */
#define AT_AUCTION      89 /* Added by Samson 12-25-98 for auction channel */
#define AT_SCORE2       90 /* Added by Samson 2-3-99 for DOTD code */
#define AT_SCORE3       91 /* Added by Samson 2-3-99 for DOTD code */
#define AT_SCORE4       92 /* Added by Samson 2-3-99 for DOTD code */
#define AT_WHO2         93 /* Added by Samson 2-3-99 for DOTD code */
#define AT_WHO3         94 /* Added by Samson 2-3-99 for DOTD code */
#define AT_WHO4         95 /* Added by Samson 2-3-99 for DOTD code */
#define AT_INTERMUD     96 /* Added by Samson 1-15-01 for Intermud3 Channels */
#define AT_HELP         97 /* Added by Samson 1-15-01 for helpfiles */
#define AT_WHO5         98 /* Added by Samson 2-7-01 for guild names on who */
#define AT_SCORE5       99 /* Added by Samson 1-14-02 */
#define AT_WHO6        100 /* Added by Samson 1-14-02 */
#define AT_WHO7        101 /* Added by Samson 1-14-02 */
#define AT_PRAC        102 /* Added by Samson 1-21-02 */
#define AT_PRAC2       103 /* Added by Samson 1-21-02 */
#define AT_PRAC3       104 /* Added by Samson 1-21-02 */
#define AT_PRAC4       105 /* Added by Samson 1-21-02 */
#define AT_MXPPROMPT   106 /* Added by Samson 2-27-02 */
#define AT_GUILDTALK   107 /* Added by Tarl 28 Nov 02 */
#define AT_BOARD       108 /* Samson 10-14-03 */
#define AT_BOARD2      109 /* Samson 10-14-03 */
#define AT_BOARD3      110 /* Samson 10-14-03 */

/* Should ALWAYS be one more than the last numerical value in the list */
#define MAX_COLORS    111

extern const short default_set[MAX_COLORS];
