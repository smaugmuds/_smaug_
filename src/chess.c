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

#include <stdio.h>
#include <string.h>

#include "mud.h"

#define SC_VERSION "5.65"
#define SC_INFO "MUD Chess"

#ifndef GET_NAME
#define GET_NAME(ch) (ch)->name
#endif
#define CHESS_NAME(ch,board) (board->type==TYPE_IMC?ch:GET_NAME(ch))

#define STOCK_COLOR	/* Uncomment */
#define send_to_char send_to_char_color

#define WHITE_BACKGROUND ""
#define BLACK_BACKGROUND ""
#define WHITE_FOREGROUND ""
#define BLACK_FOREGROUND ""

#ifdef USE_IMC
#include "imc.h"
#include "imc-mercbase.h"
void setdata(imc_packet *p, const imc_char_data *d);
const imc_char_data *getdata(CHAR_DATA *ch);
const char *getname(CHAR_DATA *ch, const imc_char_data *vict);
void imc_recv_chess(const imc_char_data *from, const char *to, const char *argument);
void imc_send_chess(CHAR_DATA *ch, const char *to, const char *argument);
#endif

const char *big_pieces[MAX_PIECES][2] = {
    {	"%s       ",
    "%s       " },
    {	"%s  (-)  ",
    "%s  -|-  " },
    {	"%s  ###  ",
    "%s  { }  " },
    {	"%s  /-*- ",
    "%s / /   " },
    {	"%s  () + ",
    "%s  {}-| " },
    {	"%s   @   ",
    "%s  /+\\  " },
    {	"%s  ^^^^^^  ",
    "%s  {@}  " },
    {	"%s  [-]  ",
    "%s  -|-  " },
    {	"%s  ###  ",
    "%s  [ ]  " },
    {	"%s  /-*- ",
    "%s / /   " },
    {	"%s  [] + ",
    "%s  {}-| " },
    {	"%s   #   ",
    "%s  /+\\  " },
    {	"%s  ^^^^^^  ",
    "%s  [#]  " }
};

const char small_pieces[MAX_PIECES+1] = " prnbqkPRNBQK";

static char *print_big_board(CHAR_DATA *ch, GAME_BOARD_DATA *board)
{
    static char retbuf[MAX_STRING_LENGTH*2];
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    char s1[16], s2[16];
    int x,y;
    
    sprintf(s1,"&Y&W");
    sprintf(s2,"&z&z");
    
    sprintf(retbuf,WHITE_FOREGROUND "\n\r&g     1      2      3      4      5      6      7      8\n\r");
    
    for (x=0;x<8;x++)
    {
        strcat(retbuf,"  ");
        for (y=0;y<8;y++)
        {
            sprintf(buf,"%s%s",
                    x%2==0 ? (y%2==0 ? BLACK_BACKGROUND : WHITE_BACKGROUND) : \
                    (y%2==0 ? WHITE_BACKGROUND : BLACK_BACKGROUND),
                    big_pieces[board->board[x][y]][0]);
            sprintf(buf2,buf,IS_WHITE(board->board[x][y]) ? s1 : s2);
            strcat(retbuf,buf2);
        }
        strcat(retbuf, BLACK_BACKGROUND "\n\r");
        
        sprintf(buf, WHITE_FOREGROUND "&g%c ", 'A'+x);
        strcat(retbuf,buf);
        for (y=0;y<8;y++)
        {
            sprintf(buf,"%s%s",
                    x%2==0 ? (y%2==0 ? BLACK_BACKGROUND : WHITE_BACKGROUND) : \
                    (y%2==0 ? WHITE_BACKGROUND : BLACK_BACKGROUND),
                    big_pieces[board->board[x][y]][1]);
            sprintf(buf2,buf,IS_WHITE(board->board[x][y]) ? s1 : s2);
            strcat(retbuf,buf2);
        }
        strcat(retbuf, BLACK_BACKGROUND "\n\r");
    }
    
    return(retbuf);
}


static void init_board(GAME_BOARD_DATA *board)
{
    int x,y;
    for (x=0;x<8;x++)
        for (y=0;y<8;y++)
            board->board[x][y] = 0;
    board->board[0][0] = WHITE_ROOK;
    board->board[0][1] = WHITE_KNIGHT;
    board->board[0][2] = WHITE_BISHOP;
    board->board[0][3] = WHITE_QUEEN;
    board->board[0][4] = WHITE_KING;
    board->board[0][5] = WHITE_BISHOP;
    board->board[0][6] = WHITE_KNIGHT;
    board->board[0][7] = WHITE_ROOK;
    for (x=0;x<8;x++)
        board->board[1][x] = WHITE_PAWN;
    for (x=0;x<8;x++)
        board->board[6][x] = BLACK_PAWN;
    board->board[7][0] = BLACK_ROOK;
    board->board[7][1] = BLACK_KNIGHT;
    board->board[7][2] = BLACK_BISHOP;
    board->board[7][3] = BLACK_QUEEN;
    board->board[7][4] = BLACK_KING;
    board->board[7][5] = BLACK_BISHOP;
    board->board[7][6] = BLACK_KNIGHT;
    board->board[7][7] = BLACK_ROOK;
    board->player1 = NULL;
    board->player2 = NULL;
    board->turn = 0;
    board->type = TYPE_LOCAL;
}

static bool find_piece(GAME_BOARD_DATA *board, int *x, int *y, int piece)
{
    int a,b;
    
    for (a=0;a<8;a++)
    {
        for (b=0;b<8;b++)
            if (board->board[a][b] == piece)
                break;
        if (board->board[a][b] == piece)
            break;
    }
    *x = a;
    *y = b;
    if (board->board[a][b] == piece)
        return TRUE;
    return FALSE;
}

#define SAME_COLOR(x1,y1,x2,y2)	\
    ((IS_WHITE(board->board[x1][y1]) && IS_WHITE(board->board[x2][y2])) || \
    (IS_BLACK(board->board[x1][y1]) && IS_BLACK(board->board[x2][y2])))

static bool king_in_check(GAME_BOARD_DATA *board, int piece)
{
    int x=0,y=0,l,m;
    
    if ( piece != WHITE_KING && piece != BLACK_KING )
        return FALSE;
    
    if (!find_piece(board,&x,&y,piece))
        return FALSE;
    
    if ( x<0 || y<0 || x>7 || y>7 )
        return FALSE;
    
    /* pawns */
    if ( IS_WHITE(piece) && x < 7 &&
         (( y > 0 && IS_BLACK(board->board[x+1][y-1]) ) ||
          ( y < 7 && IS_BLACK(board->board[x+1][y+1]) )))
        return TRUE;
    else if ( IS_BLACK(piece) && x > 0 &&
              (( y > 0 && IS_WHITE(board->board[x-1][y-1]) ) ||
               ( y < 7 && IS_WHITE(board->board[x-1][y+1]) )))
        return TRUE;
    /* knights */
    if ( x-2 >= 0 && y-1 >= 0 && 
         ( (board->board[x-2][y-1] == BLACK_KNIGHT && IS_WHITE(board->board[x][y])) ||
           (board->board[x-2][y-1] == WHITE_KNIGHT && IS_BLACK(board->board[x][y])) ))
        return TRUE;
    if ( x-2 >= 0 && y+1 < 8 && 
         ( (board->board[x-2][y+1] == BLACK_KNIGHT && IS_WHITE(board->board[x][y])) ||
           (board->board[x-2][y+1] == WHITE_KNIGHT && IS_BLACK(board->board[x][y])) ))
        return TRUE;
    
    if ( x-1 >= 0 && y-2 >= 0 && 
         ( (board->board[x-1][y-2] == BLACK_KNIGHT && IS_WHITE(board->board[x][y])) ||
           (board->board[x-1][y-2] == WHITE_KNIGHT && IS_BLACK(board->board[x][y])) ))
        return TRUE;
    if ( x-1 >= 0 && y+2 < 8 && 
         ( (board->board[x-1][y+2] == BLACK_KNIGHT && IS_WHITE(board->board[x][y])) ||
           (board->board[x-1][y+2] == WHITE_KNIGHT && IS_BLACK(board->board[x][y])) ))
        return TRUE;
    
    if ( x+1 < 8 && y-2 >= 0 && 
         ( (board->board[x+1][y-2] == BLACK_KNIGHT && IS_WHITE(board->board[x][y])) ||
           (board->board[x+1][y-2] == WHITE_KNIGHT && IS_BLACK(board->board[x][y])) ))
        return TRUE;
    if ( x+1 < 8 && y+2 < 8 && 
         ( (board->board[x+1][y+2] == BLACK_KNIGHT && IS_WHITE(board->board[x][y])) ||
           (board->board[x+1][y+2] == WHITE_KNIGHT && IS_BLACK(board->board[x][y])) ))
        return TRUE;
    
    if ( x+2 < 8 && y-1 >= 0 && 
         ( (board->board[x+2][y-1] == BLACK_KNIGHT && IS_WHITE(board->board[x][y])) ||
           (board->board[x+2][y-1] == WHITE_KNIGHT && IS_BLACK(board->board[x][y])) ))
        return TRUE;
    if ( x+2 < 8 && y+1 < 8 && 
         ( (board->board[x+2][y+1] == BLACK_KNIGHT && IS_WHITE(board->board[x][y])) ||
           (board->board[x+2][y+1] == WHITE_KNIGHT && IS_BLACK(board->board[x][y])) ))
        return TRUE;
    
    /* horizontal/vertical long distance */
    for (l=x+1;l<8;l++)
        if ( board->board[l][y] != NO_PIECE )
        {
            if ( SAME_COLOR(x,y,l,y) )
                break;
            if ( board->board[l][y] == BLACK_QUEEN || board->board[l][y] == WHITE_QUEEN ||
                 board->board[l][y] == BLACK_ROOK || board->board[l][y] == WHITE_ROOK )
                return TRUE;
            break;
        }
    for (l=x-1;l>=0;l--)
        if ( board->board[l][y] != NO_PIECE )
        {
            if ( SAME_COLOR(x,y,l,y) )
                break;
            if ( board->board[l][y] == BLACK_QUEEN || board->board[l][y] == WHITE_QUEEN ||
                 board->board[l][y] == BLACK_ROOK || board->board[l][y] == WHITE_ROOK )
                return TRUE;
            break;
        }
    for (m=y+1;m<8;m++)
        if ( board->board[x][m] != NO_PIECE )
        {
            if ( SAME_COLOR(x,y,x,m) )
                break;
            if ( board->board[x][m] == BLACK_QUEEN || board->board[x][m] == WHITE_QUEEN ||
                 board->board[x][m] == BLACK_ROOK || board->board[x][m] == WHITE_ROOK )
                return TRUE;
            break;
        }
    for (m=y-1;m>=0;m--)
        if ( board->board[x][m] != NO_PIECE )
        {
            if ( SAME_COLOR(x,y,x,m) )
                break;
            if ( board->board[x][m] == BLACK_QUEEN || board->board[x][m] == WHITE_QUEEN ||
                 board->board[x][m] == BLACK_ROOK || board->board[x][m] == WHITE_ROOK )
                return TRUE;
            break;
        }
    /* diagonal long distance */
    for (l=x+1,m=y+1;l<8 && m<8;l++,m++)
        if ( board->board[l][m] != NO_PIECE )
        {
            if ( SAME_COLOR(x,y,l,m) )
                break;
            if ( board->board[l][m] == BLACK_QUEEN || board->board[l][m] == WHITE_QUEEN ||
                 board->board[l][m] == BLACK_BISHOP || board->board[l][m] == WHITE_BISHOP )
                return TRUE;
            break;
        }
    for (l=x-1,m=y+1;l>=0 && m<8;l--,m++)
        if ( board->board[l][m] != NO_PIECE )
        {
            if ( SAME_COLOR(x,y,l,m) )
                break;
            if ( board->board[l][m] == BLACK_QUEEN || board->board[l][m] == WHITE_QUEEN ||
                 board->board[l][m] == BLACK_BISHOP || board->board[l][m] == WHITE_BISHOP )
                return TRUE;
            break;
        }
    for (l=x+1,m=y-1;l<8 && m>=0;l++,m--)
        if ( board->board[l][m] != NO_PIECE )
        {
            if ( SAME_COLOR(x,y,l,m) )
                break;
            if ( board->board[l][m] == BLACK_QUEEN || board->board[l][m] == WHITE_QUEEN ||
                 board->board[l][m] == BLACK_BISHOP || board->board[l][m] == WHITE_BISHOP )
                return TRUE;
            break;
        }
    for (l=x-1,m=y-1;l>=0 && m>=0;l--,m--)
        if ( board->board[l][m] != NO_PIECE )
        {
            if ( SAME_COLOR(x,y,l,m) )
                break;
            if ( board->board[l][m] == BLACK_QUEEN || board->board[l][m] == WHITE_QUEEN ||
                 board->board[l][m] == BLACK_BISHOP || board->board[l][m] == WHITE_BISHOP )
                return TRUE;
            break;
        }
    return FALSE;
}

static bool king_in_checkmate(GAME_BOARD_DATA *board, int piece)
{
    int x=0,y=0,dx,dy,sk=0;
    
    if ( piece != WHITE_KING && piece != BLACK_KING )
        return FALSE;
    
    if (!find_piece(board,&x,&y,piece))
        return FALSE;
    
    if ( x<0 || y<0 || x>7 || y>7 )
        return FALSE;
    
    if (!king_in_check(board,board->board[x][y]))
        return FALSE;
    
    dx = x+1;
    dy = y+1;
    if ( dx < 8 && dy < 8 && board->board[dx][dy] == NO_PIECE )
    {
        sk = board->board[dx][dy] = board->board[x][y];
        board->board[x][y] = NO_PIECE;
        if (!king_in_check(board,sk))
        {
            board->board[x][y] = sk;
            board->board[dx][dy] = NO_PIECE;
            return FALSE;
        }
        board->board[x][y] = sk;
        board->board[dx][dy] = NO_PIECE;
    }
    dx = x-1;
    dy = y+1;
    if ( dx >= 0 && dy < 8 && board->board[dx][dy] == NO_PIECE )
    {
        sk = board->board[dx][dy] = board->board[x][y];
        board->board[x][y] = NO_PIECE;
        if (!king_in_check(board,sk))
        {
            board->board[x][y] = sk;
            board->board[dx][dy] = NO_PIECE;
            return FALSE;
        }
        board->board[x][y] = sk;
        board->board[dx][dy] = NO_PIECE;
    }
    dx = x+1;
    dy = y-1;
    if ( dx < 8 && dy >= 0 && board->board[dx][dy] == NO_PIECE )
    {
        sk = board->board[dx][dy] = board->board[x][y];
        board->board[x][y] = NO_PIECE;
        if (!king_in_check(board,sk))
        {
            board->board[x][y] = sk;
            board->board[dx][dy] = NO_PIECE;
            return FALSE;
        }
        board->board[x][y] = sk;
        board->board[dx][dy] = NO_PIECE;
    }
    dx = x-1;
    dy = y-1;
    if ( dx >= 0 && dy >= 0 && board->board[dx][dy] == NO_PIECE )
    {
        sk = board->board[dx][dy] = board->board[x][y];
        board->board[x][y] = NO_PIECE;
        if (!king_in_check(board,sk))
        {
            board->board[x][y] = sk;
            board->board[dx][dy] = NO_PIECE;
            return FALSE;
        }
        board->board[x][y] = sk;
        board->board[dx][dy] = NO_PIECE;
    }
    dx = x;
    dy = y+1;
    if ( dy < 8 && board->board[dx][dy] == NO_PIECE )
    {
        sk = board->board[dx][dy] = board->board[x][y];
        board->board[x][y] = NO_PIECE;
        if (!king_in_check(board,sk))
        {
            board->board[x][y] = sk;
            board->board[dx][dy] = NO_PIECE;
            return FALSE;
        }
        board->board[x][y] = sk;
        board->board[dx][dy] = NO_PIECE;
    }
    dx = x;
    dy = y-1;
    if ( dy >= 0 && board->board[dx][dy] == NO_PIECE )
    {
        sk = board->board[dx][dy] = board->board[x][y];
        board->board[x][y] = NO_PIECE;
        if (!king_in_check(board,sk))
        {
            board->board[x][y] = sk;
            board->board[dx][dy] = NO_PIECE;
            return FALSE;
        }
        board->board[x][y] = sk;
        board->board[dx][dy] = NO_PIECE;
    }
    dx = x+1;
    dy = y;
    if ( dx < 8 && board->board[dx][dy] == NO_PIECE )
    {
        sk = board->board[dx][dy] = board->board[x][y];
        board->board[x][y] = NO_PIECE;
        if (!king_in_check(board,sk))
        {
            board->board[x][y] = sk;
            board->board[dx][dy] = NO_PIECE;
            return FALSE;
        }
        board->board[x][y] = sk;
        board->board[dx][dy] = NO_PIECE;
    }
    dx = x-1;
    dy = y;
    if ( dx >= 0 && board->board[dx][dy] == NO_PIECE )
    {
        sk = board->board[dx][dy] = board->board[x][y];
        board->board[x][y] = NO_PIECE;
        if (!king_in_check(board,sk))
        {
            board->board[x][y] = sk;
            board->board[dx][dy] = NO_PIECE;
            return FALSE;
        }
        board->board[x][y] = sk;
        board->board[dx][dy] = NO_PIECE;
    }
    return TRUE;
}

static int is_valid_move(CHAR_DATA *ch, GAME_BOARD_DATA *board, int x, int y, int dx, int dy)
{
    if ( dx<0 || dy<0 || dx>7 || dy>7 )
        return MOVE_OFFBOARD;
    
    if ( board->board[x][y] == NO_PIECE )
        return MOVE_INVALID;
    
    if ( x == dx && y == dy )
        return MOVE_INVALID;
    
    if ( IS_WHITE(board->board[x][y]) && board->player1 == ch )
        return MOVE_WRONGCOLOR;
    if ( IS_BLACK(board->board[x][y]) && (board->player2 == ch || !ch) )
        return MOVE_WRONGCOLOR;
    
    switch (board->board[x][y])
    {
    case WHITE_PAWN:	case BLACK_PAWN:
        if ( IS_WHITE(board->board[x][y]) &&
             dx == x+2 && x == 1 && dy == y &&
             board->board[dx][dy] == NO_PIECE &&
             board->board[x+1][dy] == NO_PIECE )
            return MOVE_OK;
        else if ( IS_BLACK(board->board[x][y]) &&
                  dx == x-2 && x == 6 && dy == y &&
                  board->board[dx][dy] == NO_PIECE &&
                  board->board[x-1][dy] == NO_PIECE )
            return MOVE_OK;
        if ( IS_WHITE(board->board[x][y]) && dx != x+1 )
            return MOVE_INVALID;
        else if ( IS_BLACK(board->board[x][y]) && dx != x-1 )
            return MOVE_INVALID;
        if ( dy != y && dy != y-1 && dy != y+1 )
            return MOVE_INVALID;
        if ( dy == y )
        {
            if ( board->board[dx][dy] == NO_PIECE)
                return MOVE_OK;
            else if ( SAME_COLOR(x,y,dx,dy) )
                return MOVE_SAMECOLOR;
            else
                return MOVE_BLOCKED;
        }	
        else
        {
            if ( board->board[dx][dy] == NO_PIECE )
                return MOVE_INVALID;
            else if ( SAME_COLOR(x,y,dx,dy) )
                return MOVE_SAMECOLOR;
            else if ( board->board[dx][dy] != BLACK_KING &&
                      board->board[dx][dy] != WHITE_KING )
                return MOVE_TAKEN;
            else
                return MOVE_INVALID;
        }
        break;
    case WHITE_ROOK:	case BLACK_ROOK:
        {
            int cnt;
            
            if ( dx != x && dy != y )
                return MOVE_INVALID;
            
            if ( dx == x)
            {
                for (cnt = y; cnt != dy; )
                { 
                    if ( cnt != y && board->board[x][cnt] != NO_PIECE )
                        return MOVE_BLOCKED;
                    if ( dy > y ) 
                        cnt++;
                    else
                        cnt--;
                }
            }
            else if ( dy == y)
            {
                for (cnt = x; cnt != dx; )
                { 
                    if ( cnt !=x && board->board[cnt][y] != NO_PIECE )
                        return MOVE_BLOCKED;
                    if ( dx > x ) 
                        cnt++;
                    else
                        cnt--;
                }
            }
            
            if ( board->board[dx][dy] == NO_PIECE )
                return MOVE_OK;
            
            if ( !SAME_COLOR(x,y,dx,dy) )
                return MOVE_TAKEN;
            
            return MOVE_SAMECOLOR;
        }
        break;
    case WHITE_KNIGHT:	case BLACK_KNIGHT:
        if ( (dx == x-2 && dy == y-1) ||
             (dx == x-2 && dy == y+1) ||
             (dx == x-1 && dy == y-2) ||
             (dx == x-1 && dy == y+2) ||
             (dx == x+1 && dy == y-2) ||
             (dx == x+1 && dy == y+2) ||
             (dx == x+2 && dy == y-1) ||
             (dx == x+2 && dy == y+1) )
        {
            if ( board->board[dx][dy] == NO_PIECE )
                return MOVE_OK;
            if ( SAME_COLOR(x,y,dx,dy) )
                return MOVE_SAMECOLOR;
            return MOVE_TAKEN;
        }
        return MOVE_INVALID;
        break;
    case WHITE_BISHOP:	case BLACK_BISHOP:
        {
            int l, m, blocked = FALSE;
            
            if ( dx == x || dy == y )
                return MOVE_INVALID;
            
            l = x;
            m = y;
            
            while ( 1 )
            {
                if ( dx > x )
                    l++;
                else
                    l--;
                if ( dy > y )
                    m++;
                else
                    m--;
                if ( l > 7 || m > 7 || l < 0 || m < 0 )
                    return MOVE_INVALID;
                if ( l == dx && m == dy )
                    break;
                if ( board->board[l][m] != NO_PIECE )
                    blocked = TRUE;
            }
            if ( l != dx || m != dy )
                return MOVE_INVALID;
            
            if ( blocked )
                return MOVE_BLOCKED;
            
            if ( board->board[dx][dy] == NO_PIECE )
                return MOVE_OK;
            
            if ( !SAME_COLOR(x,y,dx,dy) )
                return MOVE_TAKEN;
            
            return MOVE_SAMECOLOR;
        }
        break;
    case WHITE_QUEEN:	case BLACK_QUEEN:
        {
            int l, m, blocked = FALSE;
            
            l = x;
            m = y;
            
            while ( 1 )
            {
                if ( dx > x )
                    l++;
                else if ( dx < x )
                    l--;
                if ( dy > y )
                    m++;
                else if ( dy < y )
                    m--;
                if ( l > 7 || m > 7 || l < 0 || m < 0 )
                    return MOVE_INVALID;
                if ( l == dx && m == dy )
                    break;
                if ( board->board[l][m] != NO_PIECE )
                    blocked = TRUE;
            }
            if ( l != dx || m != dy )
                return MOVE_INVALID;
            
            if ( blocked )
                return MOVE_BLOCKED;
            
            if ( board->board[dx][dy] == NO_PIECE )
                return MOVE_OK;
            
            if ( !SAME_COLOR(x,y,dx,dy) )
                return MOVE_TAKEN;
            
            return MOVE_SAMECOLOR;
        }
        break;
    case WHITE_KING:	case BLACK_KING:
        {
            int sp,sk;
            if ( dx > x+1 || dx < x-1 || dy > y+1 || dy < y-1 )
                return MOVE_INVALID;
            sk = board->board[x][y];
            sp = board->board[dx][dy];
            board->board[x][y] = sp;
            board->board[dx][dy] = sk;
            if (king_in_check(board,sk))
            {
                board->board[x][y] = sk;
                board->board[dx][dy] = sp;
                return MOVE_CHECK;
            }
            board->board[x][y] = sk;
            board->board[dx][dy] = sp;
            if ( board->board[dx][dy] == NO_PIECE )
                return MOVE_OK;
            if ( SAME_COLOR(x,y,dx,dy) )
                return MOVE_SAMECOLOR;
            return MOVE_TAKEN;
        }
        break;
    default: bug("Invaild piece: %d", board->board[x][y]);
    return MOVE_INVALID;
    }
    
    if ((IS_WHITE(board->board[x][y]) && IS_WHITE(board->board[dx][dy])) ||
        (IS_BLACK(board->board[x][y]) && IS_BLACK(board->board[dx][dy])))
        return MOVE_SAMECOLOR;
    
    return MOVE_OK;
}

#undef SAME_COLOR

void free_game( GAME_BOARD_DATA *board )
{
    if ( !board )
        return;
#ifdef USE_IMC
    if ( board->type == TYPE_IMC )
    {
        imc_send_chess((CHAR_DATA *)board->player1?:NULL, (char *)board->player2, "stop");
        if (board->player2)
            DISPOSE(board->player2);
    }
#endif
    if ( board->player1 )
    {
        CHAR_DATA *ch = (CHAR_DATA *)board->player1;
        ch_printf(ch, "The game has been stopped at %d total moves.\n\r", board->turn);
        ch->pcdata->game_board = NULL;
    }
    if ( board->player2 )
    {
        CHAR_DATA *ch = (CHAR_DATA *)board->player2;
        ch_printf(ch, "The game has been stopped at %d total moves.\n\r", board->turn);
        ch->pcdata->game_board = NULL;
    }
    board->player1 = NULL;
    board->player2 = NULL;
    DISPOSE(board);
}

#ifdef USE_IMC
void imc_send_chess(CHAR_DATA *ch, const char *to, const char *argument)
{
    imc_packet out;
    
    if ( !ch || imc_active<IA_UP )
        return;
    
    setdata(&out, getdata(ch));
    
    imc_sncpy(out.to, to, IMC_NAME_LENGTH);
    strcpy(out.type, "chess");
    imc_addkey(&out.data, "text", argument);
    
    imc_send(&out);
    imc_freedata(&out.data);
}

void imc_recv_chess(const imc_char_data *from, const char *to, const char *argument)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim, *vch;
    char buf[MAX_INPUT_LENGTH];
    
    if ( !strcmp(to, "*") )
        return;
    
    victim=NULL;
    for ( d=descriptor_list; d; d=d->next )
    {
        if ( d->connected==CON_PLAYING &&
             (vch=d->original ? d->original : d->character)!=NULL &&
             !IS_NPC(vch) )
        {
            if ( !str_cmp((char *)to, GET_NAME(vch)) )
            {
                victim=vch;
                break;
            }
            if ( is_name((char *)to, GET_NAME(vch)) )
                victim=vch;
        }
    }
    
    if ( !victim )
    {
        if ( !str_cmp(argument, "stop") )
            return;
        sprintf(buf, "%s is not here.", to);
        imc_send_tell(NULL, from->name, buf, 1);
        return;
    }
    
    if ( !victim->pcdata->game_board )
    {
        if ( !str_cmp(argument, "stop") )
            return;
        sprintf(buf, "%s is not ready to be joined in a game.", to);
        imc_send_tell(NULL, from->name, buf, 1);
        imc_send_chess((CHAR_DATA *)victim->pcdata->game_board->player1?:NULL,(char *)from->name,"stop");
        return;
    }
    
    if ( !str_cmp(argument, "start") )
    {
        if ( victim->pcdata->game_board->player2 != NULL )
        {
            sprintf(buf, "%s is already playing a game.", to);
            imc_send_tell(NULL, from->name, buf, 1);
            imc_send_chess((CHAR_DATA *)victim->pcdata->game_board->player1?:NULL,(char *)from->name,"stop");
            return;
        }
        victim->pcdata->game_board->player2 = str_dup(from->name);
        victim->pcdata->game_board->turn = 0;
        victim->pcdata->game_board->type = TYPE_IMC;
        ch_printf(victim, "%s has joined your game.\n\r", from->name);
        imc_send_chess(victim, from->name, "accepted");
        return;
    }
    if ( !str_cmp(argument, "accepted") )
    {
        if ( !victim->pcdata->game_board ||
             victim->pcdata->game_board->player2 == NULL ||
             victim->pcdata->game_board->type != TYPE_IMC ||
             str_cmp((char *)victim->pcdata->game_board->player2,from->name) )
        {
            imc_send_chess((CHAR_DATA *)victim->pcdata->game_board->player1?:NULL,(char *)from->name, "stop");
            return;
        }
        ch_printf(victim,"You have joined %s in a game.\n\r", from->name);
        if (victim->pcdata->game_board->player2)
            DISPOSE(victim->pcdata->game_board->player2);
        victim->pcdata->game_board->player2 = str_dup(from->name);
        victim->pcdata->game_board->turn = 1;
        return;
    }
    if ( !str_cmp(argument, "stop") )
    {
        ch_printf(victim, "%s has stopped the game.\n\r", from->name);
        free_game(victim->pcdata->game_board);
        return;
    }
    if ( !str_cmp(argument, "invalidmove") )
    {
        send_to_char("You have issued an invalid move according to the other mud.\n\r", victim);
        do_game_board(victim,"stop",0);
        return;
    }
    if ( !str_cmp(argument, "moveok") )
    {
        send_to_char("The other mud has accepted your move.\n\r", victim);
        return;
    }
    
    if ( !str_prefix("move", argument) )
    {
        char a,b;
        int x,y,dx,dy,ret;
        a=b=' ';
        x=y=dx=dy=-1;
        if (sscanf(argument, "move %c%d %c%d",&a,&y,&b,&dy) != 4 ||
            a<'0' || a>'7' || b<'0' || b>'7' || y<0 || y>7 || dy<0 || dy>7)
        {
            imc_send_chess((CHAR_DATA *)victim->pcdata->game_board->player1?:NULL,(char *)from->name, "invalidmove");
            return;
        }
        x = a - '0';
        dx = b - '0';
        x = (7-x);
        y = (7-y);
        dx = (7-dx);
        dy = (7-dy);
        log_printf("%d, %d -> %d, %d", x,y,dx,dy);
        ret = is_valid_move(NULL,victim->pcdata->game_board,x,y,dx,dy);
        if (ret == MOVE_OK || ret == MOVE_TAKEN)
        {
            GAME_BOARD_DATA *board;
            int piece, destpiece;
            board = victim->pcdata->game_board;
            piece = board->board[x][y];
            destpiece = board->board[dx][dy];
            board->board[dx][dy] = piece;
            board->board[x][y] = NO_PIECE;
            if ( king_in_check(board,IS_WHITE(board->board[dx][dy])?WHITE_KING:BLACK_KING) &&
                 ( board->board[dx][dy]!=WHITE_KING && board->board[dx][dy]!=BLACK_KING ))
            {
                board->board[dx][dy] = destpiece;
                board->board[x][y] = piece;
            }
            else
            {
                board->turn++;
                imc_send_chess((CHAR_DATA *)board->player1?:NULL,(char *)from->name, "moveok");
                return;
            }
        }
        imc_send_chess((CHAR_DATA *)victim->pcdata->game_board->player1?:NULL,(char *)from->name, "invalidmove");
        return;
    }
    
    sprintf(log_buf, "Unknown chess command from: %s, %s", from->name, argument);
    log_string(log_buf);
}
#endif

void do_game_board(CHAR_DATA *ch, char *argument, int cmdnumber)
{
    char arg[MAX_INPUT_LENGTH];
    
    argument = one_argument(argument, arg);
    
    if ( IS_NPC(ch) )
    {
        send_to_char("NPC's can't be in games.\n\r", ch);
        return;
    }
    
    if ( !str_cmp(arg, "begin") )
    {
        GAME_BOARD_DATA *board;
        if ( ch->pcdata->game_board )
        {
            send_to_char("You are already in a chess match.\n\r", ch);
            return;
        }
        CREATE(board, GAME_BOARD_DATA, 1);
        init_board(board);
        ch->pcdata->game_board = board;
        ch->pcdata->game_board->player1 = ch;
        send_to_char("You have started a game of chess.\n\r", ch);
        return;
    }
    
    if ( !str_cmp(arg, "join") )
    {
        GAME_BOARD_DATA *board=NULL;
        CHAR_DATA *vch;
        char arg2[MAX_INPUT_LENGTH];
        if ( ch->pcdata->game_board )
        {
            send_to_char("You are already in a game of chess.\n\r", ch);
            return;
        }
        argument = one_argument(argument,arg2);
        if ( arg[0] == '\0' )
        {
            send_to_char("Join whom in a chess match?\n\r", ch);
            return;
        }
#ifdef USE_IMC
        if ( strstr( arg2, "@" ) )
        {
            if (!str_cmp(imc_mudof(arg2), imc_name))
            {
                send_to_char("That is this mud, don't use @.\n\r",ch);
                return;
            }
            send_to_char("Attempting to initiate IMC game...\n\r", ch);
            if (!str_cmp(imc_mudof(arg2), "*"))
            {
                send_to_char("* is not a valid mud name.\n\r",ch);
                return;
            }
            CREATE(board, GAME_BOARD_DATA, 1);
            init_board(board);
            board->type = TYPE_IMC;
            board->player1 = (void *)ch;
            board->player2 = (void *)str_dup(arg2);
            board->turn = -1;
            ch->pcdata->game_board = board;
            imc_send_chess(ch,arg2,"start");
            return;
        }
#endif
        if ( !( vch = get_char_world(ch,arg2) ) )
        {
            send_to_char("Cannot find that player.\n\r", ch);
            return;
        }
        if ( IS_NPC(vch) )
        {
            send_to_char("That player is an NPC, and cannot play games.\n\r", ch);
            return;
        }
        board = vch->pcdata->game_board;
        if ( !board )
        {
            send_to_char("That player is not playing a game.\n\r", ch);
            return;
        }
        if ( board->player2 )
        {
            send_to_char("That game already has two players.\n\r", ch);
            return;
        }
        board->player2 = (void *)ch;
        ch->pcdata->game_board = board;
        send_to_char("You have joined a game of chess.\n\r", ch);
        ch_printf((CHAR_DATA *)board->player1, "%s has joined your game.\n\r", GET_NAME(ch));
        return;
    }
    
    if ( !ch->pcdata->game_board )
    {
        send_to_char("Usage: chess <begin|cease|status|board|move|join>\n\r",ch);
        return;
    }
    
    if ( !str_cmp(arg, "cease") )
    {
        free_game(ch->pcdata->game_board);
        return;
    }
    
    if ( !str_cmp(arg, "status") )
    {
        GAME_BOARD_DATA *board = ch->pcdata->game_board;
        if ( !board->player1 )
            send_to_char("There is no black player.\n\r", ch);
        else if ( board->player1 == ch )
            send_to_char("You are black.\n\r", ch);
        else
            ch_printf(ch, "%s is black.\n\r",
                      GET_NAME((CHAR_DATA *)board->player1));
        if (king_in_checkmate(board,BLACK_KING))
            send_to_char("The black king is in check.\n\r", ch);
        else if (king_in_check(board,BLACK_KING))
            send_to_char("The black king is in check.\n\r", ch);
        if ( !board->player2 )
            send_to_char("There is no white player.\n\r", ch);
        else if ( board->player2 == ch )
            send_to_char("You are white.\n\r", ch);
        else
            ch_printf(ch, "%s is white.\n\r",
                      board->type == TYPE_LOCAL ?
                      GET_NAME((CHAR_DATA *)board->player2) :
                      (char *)board->player2);
        if (king_in_checkmate(board,WHITE_KING))
            send_to_char("The white king is in check.\n\r", ch);
        else if (king_in_check(board,WHITE_KING))
            send_to_char("The white king is in check.\n\r", ch);
        if ( !board->player2 ||
             !board->player1 )
            return;
        ch_printf(ch, "%d turns.\n\r", board->turn);
        if ( board->turn % 2 == 1 && board->player1 == ch )
        {
            ch_printf(ch, "It is %s's turn.\n\r", GET_NAME((CHAR_DATA *)board->player2));
            return;
        }
        else if ( board->turn % 2 == 0 && board->player2 == ch )
        {
            ch_printf(ch, "It is %s's turn.\n\r",
                      board->type == TYPE_LOCAL ?
                      GET_NAME((CHAR_DATA *)board->player1) :
                      (char *)board->player1);
            return;
        }
        else
        {
            send_to_char("It is your turn.\n\r", ch);
            return;
        }
        return;
    }
    
    if ( !str_prefix(arg, "board") )
    {
        static char *b1;
        b1 = print_big_board(ch, ch->pcdata->game_board);
        send_to_char(b1,ch);
        return;
    }
    
    if ( !str_prefix(arg, "move") )
    {
        CHAR_DATA *opp;
        char a,b;
        int x,y,dx,dy,ret;
        
        if ( !ch->pcdata->game_board->player1 || 
             !ch->pcdata->game_board->player2 )
        {
            send_to_char("There is only 1 player.\n\r", ch);
            return;
        }
        if ( ch->pcdata->game_board->turn < 0 )
        {
            send_to_char("The game hasn't started yet.\n\r", ch);
            return;
        }
        
        if ( king_in_checkmate(ch->pcdata->game_board, BLACK_KING) )
        {
            send_to_char("The black king has been checkmated, the game is over.\n\r", ch);
            return;
        }
        if ( king_in_checkmate(ch->pcdata->game_board, WHITE_KING) )
        {
            send_to_char("The white king has been checkmated, the game is over.\n\r", ch);
            return;
        }
        
        if ( !*argument )
        {
            send_to_char("Usage: chess move [piece to move] [where to move]\n\r",ch);
            return;
        }
        
        if ( ch->pcdata->game_board->turn % 2 == 1 &&
             ch->pcdata->game_board->player1 == ch )
        {
            send_to_char("It is not your turn.\n\r", ch);
            return;
        }
        if ( ch->pcdata->game_board->turn % 2 == 0 &&
             ch->pcdata->game_board->player2 == ch )
        {
            send_to_char("It is not your turn.\n\r", ch);
            return;
        }
        
        if (sscanf(argument,"%c%d %c%d",&a,&y,&b,&dy)!=4)
        {
            send_to_char("Usage: chess move [dest] [source]\n\r",ch);
            return;
        }
        
        if ( a < 'a' || a > 'h' || b < 'a' || b > 'h' || y < 1 || y > 8 || dy < 1 || dy > 8 )
        {
            send_to_char("Invalid move, use a-h, 1-8.\n\r", ch);
            return;
        }
        
        x = a - 'a';
        dx = b - 'a';
        y--;
        dy--;
        
        ret = is_valid_move(ch,ch->pcdata->game_board,x,y,dx,dy);
        if (ret == MOVE_OK || ret == MOVE_TAKEN)
        {
            GAME_BOARD_DATA *board;
            int piece, destpiece;
            board = ch->pcdata->game_board;
            piece = board->board[x][y];
            destpiece = board->board[dx][dy];
            board->board[dx][dy] = piece;
            board->board[x][y] = NO_PIECE;
            if ( king_in_check(board,IS_WHITE(board->board[dx][dy])?WHITE_KING:BLACK_KING) &&
                 ( board->board[dx][dy]!=WHITE_KING && board->board[dx][dy]!=BLACK_KING ))
            {
                board->board[dx][dy] = destpiece;
                board->board[x][y] = piece;
                ret = MOVE_INCHECK;
            }
            else
            {
                board->turn++;
#ifdef USE_IMC
                if ( ch->pcdata->game_board->type == TYPE_IMC)
                {
                    sprintf(arg, "move %d%d %d%d", x, y, dx, dy);
                    imc_send_chess((CHAR_DATA *)ch->pcdata->game_board->player1, (char *)ch->pcdata->game_board->player2, arg);
                }
#endif
            }
        }
        if ( ch == ch->pcdata->game_board->player1 )
            opp = (CHAR_DATA *)ch->pcdata->game_board->player2;
        else
            opp = (CHAR_DATA *)ch->pcdata->game_board->player1;
#ifdef USE_IMC
#define SEND_TO_OPP(arg,opp) \
    if (opp) \
    { \
    if (ch->pcdata->game_board->type==TYPE_LOCAL) \
    send_to_char((arg),(opp)); \
    if (ch->pcdata->game_board->type==TYPE_IMC) \
    imc_send_tell(NULL, (char *)(opp), (arg), 1); \
    }
#else
#define SEND_TO_OPP(arg,opp) \
    if (opp) \
    { \
    if (ch->pcdata->game_board->type==TYPE_LOCAL) \
    send_to_char((arg),(opp)); \
    }
#endif
        switch (ret)
        {
        case MOVE_OK:
            send_to_char("Ok.\n\r", ch);
            sprintf(arg, "%s has moved.\n\r", GET_NAME(ch));
            SEND_TO_OPP(arg, opp);
            break;
        case MOVE_INVALID:
            send_to_char("Invalid move.\n\r", ch);
            break;
        case MOVE_BLOCKED:
            send_to_char("You are blocked in that direction.\n\r", ch);
            break;
        case MOVE_TAKEN:
            send_to_char("You take the enemy's piece.\n\r", ch);
            sprintf(arg, "%s has taken one of your pieces!", GET_NAME(ch));
            SEND_TO_OPP(arg, opp);
            break;
        case MOVE_CHECKMATE:
            send_to_char("That move would result in a checkmate.\n\r", ch);
            sprintf(arg, "%s has attempted a move that would result in checkmate.", GET_NAME(ch));
            SEND_TO_OPP(arg, opp);
            break;
        case MOVE_OFFBOARD:
            send_to_char("That move would be off the board.\n\r", ch);
            break;
        case MOVE_SAMECOLOR:
            send_to_char("Your own piece blocks the way.\n\r", ch);
            break;
        case MOVE_CHECK:
            send_to_char("That move would result in a check.\n\r", ch);
            sprintf(arg, "%s has made a move that would result in a check.", GET_NAME(ch));
            SEND_TO_OPP(arg, opp);
            break;
        case MOVE_WRONGCOLOR:
            send_to_char("That is not your piece.\n\r", ch);
            break;
        case MOVE_INCHECK:
            send_to_char("You are in check, you must save your king.\n\r", ch);
            break;
        default:
            bug("Unknown return value from is_valid_move():games.c");
            break;
        }
#undef SEND_TO_OPP
        return;
    }
    
    send_to_char("Usage: chess <begin|cease|status|board|move|join>\n\r",ch);
}
