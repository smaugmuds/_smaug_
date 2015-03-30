/****************************************************************************
 *                      Lands of Altanos by Jeff Maxx                       *
 ***************************************************************************/
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
 ***************************************************************************/

typedef struct	game_board_data	GAME_BOARD_DATA;

struct game_board_data {
  int board[8][8];
  int turn;
  int type;
  void *player1;
  void *player2;
};

void free_game( GAME_BOARD_DATA *board );

#define NO_PIECE	0

#define WHITE_PAWN	1
#define WHITE_ROOK	2
#define WHITE_KNIGHT	3
#define WHITE_BISHOP	4
#define WHITE_QUEEN	5
#define WHITE_KING	6

#define BLACK_PAWN	7
#define BLACK_ROOK	8
#define BLACK_KNIGHT	9
#define BLACK_BISHOP	10
#define BLACK_QUEEN	11
#define BLACK_KING	12

#define MAX_PIECES	13

#define IS_WHITE(x) ((x)>=WHITE_PAWN && (x)<=WHITE_KING)
#define IS_BLACK(x) ((x)>=BLACK_PAWN && (x)<=BLACK_KING)

#define MOVE_OK		0
#define MOVE_INVALID	1
#define MOVE_BLOCKED	2
#define MOVE_TAKEN	3
#define MOVE_CHECKMATE	4
#define MOVE_OFFBOARD	5
#define MOVE_SAMECOLOR	6
#define MOVE_CHECK	8
#define MOVE_WRONGCOLOR	9
#define MOVE_INCHECK	10

#define TYPE_LOCAL	1
#define TYPE_IMC	2
