/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops, Fireblade, Edmond, Conran                         |             *
 * ------------------------------------------------------------------------ *
 * 			Gorog's Revenge on Unruly Bastards		    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];

/* truncate a char string if it's length exceeds a given value. */
void truncs(char *s, int len)
{
   if ( strlen(s) > len )
      s[len] = '\0';
}

/* New owhere by Gorog, 98/08/21 */
void do_owhere( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH], field[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj, *outer_obj;
    bool found = FALSE;
    int icnt=0, vnum=0;
    char heading[] =
"    Vnum  Short Desc        Vnum  Room/Char          Vnum  Container\n\r";

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        pager_printf( ch, "Owhere what?\n\r" );
        return;
    }
    if ( is_number(arg) )
       vnum=atoi(arg);

    for ( obj = first_object; obj; obj = obj->next )
    {
        if ( vnum )
        {  
           if ( vnum!=obj->pIndexData->vnum)
              continue;
        }   
        else if ( !nifty_is_name( arg, obj->name ) )
              continue;

        if ( !found )
           send_to_pager( heading, ch );     /* print report heading */
        found = TRUE;
        
        outer_obj = obj;
        while ( outer_obj->in_obj )
              outer_obj = outer_obj->in_obj;

        sprintf(field, "%-18s", obj_short(obj));
        truncs(field, 18);
        sprintf(buf, "%3d %5d %-18s ", ++icnt, obj->pIndexData->vnum, field);
        if ( outer_obj->carried_by )
        {
            sprintf(field, "%-18s", PERS(outer_obj->carried_by, ch));
            truncs(field, 18);
            sprintf(buf+strlen(buf), "%5d %-18s ",
               (IS_NPC(outer_obj->carried_by) ?
                outer_obj->carried_by->pIndexData->vnum : 0), field);
            if ( outer_obj!=obj )
            {
               sprintf(field, "%-18s", obj->in_obj->name);
               truncs(field, 18);
               sprintf(buf+strlen(buf), "%5d %-18s ",
               obj->in_obj->pIndexData->vnum, field);
            }
            sprintf(buf+strlen(buf), "\n\r");
            send_to_pager(buf, ch);
        }
        else if ( outer_obj->in_room )
        {
           sprintf(field, "%-18s", outer_obj->in_room->name);
           truncs(field, 18);
           sprintf(buf+strlen(buf), "%5d %-18s ",
           outer_obj->in_room->vnum, field);
           if ( outer_obj!=obj )
           {
              sprintf(field, "%-18s", obj->in_obj->name);
              truncs(field, 18);
              sprintf(buf+strlen(buf), "%5d %-18s ",
              obj->in_obj->pIndexData->vnum, field);
           }
           sprintf(buf+strlen(buf), "\n\r");
           send_to_pager(buf, ch);
        }
    }
    if ( !found )
      pager_printf(ch, "None found.\n\r");
}
	
/*
 * Find the position of a target substring in a source string.
 * Returns pointer to the first occurrence of the string pointed to 
 * bstr in the string pointed to by astr. It returns a null pointer
 * if no match is found.  --  Gorog (with help from Thoric)
 *
 * Note I made a change when modifying str_infix. If the target string is
 * null, I return NULL (meaning no match was found). str_infix returns
 * FALSE (meaning a match was found).  *grumble*
 */
char *str_str( char *astr, char *bstr )
{
    int sstr1, sstr2, ichar;
    char c0;

    if ( ( c0 = LOWER(bstr[0]) ) == '\0' )
        return NULL;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);

    for ( ichar = 0; ichar <= sstr1 - sstr2; ichar++ )
        if ( c0 == LOWER(astr[ichar]) && !str_prefix(bstr, astr+ichar) )
            return (astr+ichar);
    return NULL;
}

/*
 * Counts the number of times a target string occurs in a source string.
 * case insensitive -- Gorog
 */
int str_count(char *psource, char *ptarget)
{
   char *ptemp=psource;
   int count=0;
   
   while ( (ptemp=str_str(ptemp, ptarget)) )
   {
      ptemp++;
      count++;
   }
   return count;
}

/*
 * Displays the help screen for the "opfind" command
 */
void opfind_help (CHAR_DATA *ch)
{
   send_to_char( "Syntax:\n\r", ch);
   send_to_char( "opfind n lo_vnum hi_vnum text \n\r"
      "   Search obj vnums between lo_vnum and hi_vnum \n\r"
      "   for obj progs that contain an occurrence of text. \n\r"
      "   Display a maxiumu of n lines.\n\r\n\r", ch );
   send_to_char( "opfind n mud text \n\r"
      "   Search all the objs in the mud for\n\r"
      "   obj progs that contain an occurrence of text. \n\r"
      "   Display a maxiumu of n lines.\n\r\n\r", ch );
   
   send_to_char( "Example:\n\r", ch);
   send_to_char( "opfind 20 901 969 if isnpc \n\r"
      "   Search all obj progs in Olympus (vnums 901 thru 969)\n\r"
      "   and display all objects that contain the text \"if isnpc\".\n\r"
      "   Display a maximum of 20 lines.\n\r\n\r", ch );
   send_to_char( "Example:\n\r", ch);
   send_to_char( "opfind 100 mud mpslay \n\r"
      "   Search all obj progs in the entire mud\n\r"
      "   and display all objects that contain the text \"mpslay\".\n\r"
      "   Display a maximum of 100 lines.\n\r\n\r", ch );
}

/*
 * Search objects for obj progs containing a specified text string.
 */
void do_opfind( CHAR_DATA *ch, char *argument )   /* Gorog */
{
   OBJ_INDEX_DATA  *   pObj;
   MPROG_DATA      *   pProg;
   char                arg1 [MAX_INPUT_LENGTH];
   char                arg2 [MAX_INPUT_LENGTH];
   char                arg3 [MAX_INPUT_LENGTH];
   int                 lo_vnum=1, hi_vnum=MAX_VNUM;
   int                 tot_vnum, tot_hits=0;
   int                 hash, disp_cou=0, disp_limit;
   
   argument = one_argument( argument, arg1 );   /* display_limit */
   argument = one_argument( argument, arg2 );

   if ( arg1[0]=='\0' || arg2[0]=='\0' || !is_number(arg1) )
   {
      opfind_help(ch);
      return;
   }

   disp_limit = atoi (arg1);
   disp_limit = UMAX(0, disp_limit);

   if ( str_cmp(arg2, "mud") )
   {
      argument = one_argument( argument, arg3 );
      if ( arg3[0]=='\0' || argument[0]=='\0' 
      ||   !is_number(arg2) || !is_number(arg3) )
      {
         opfind_help(ch);
         return;
      }
      else
      {
         lo_vnum = URANGE(1, atoi(arg2), MAX_VNUM);
         hi_vnum = URANGE(1, atoi(arg3), MAX_VNUM);
         if ( lo_vnum > hi_vnum )
         {
            opfind_help(ch);
            return;
         }
      }
   }   
   if ( argument[0] == '\0' )
   {
      opfind_help(ch);
      return;
   }

   for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
    for ( pObj = obj_index_hash[hash];
          pObj;
          pObj = pObj->next )
   {
      if ( pObj->vnum < lo_vnum ||
           pObj->vnum > hi_vnum )
        continue;

      if (pObj && (pProg=pObj->mudprogs) )
      {
         tot_vnum = 0;
         for ( ; pProg; pProg=pProg->next)
             tot_vnum += str_count(pProg->comlist, argument);
         tot_hits += tot_vnum;
         if ( tot_vnum && ++disp_cou <= disp_limit)
            pager_printf( ch, "%5d %5d %5d\n\r", disp_cou, pObj->vnum, tot_vnum);
      }
   }
   pager_printf( ch, "Total: %10d\n\r", tot_hits);
}
		
/*
 * Displays the help screen for the "mpfind" command
 */
void mpfind_help (CHAR_DATA *ch)
{
   send_to_char( "Syntax:\n\r", ch);
   send_to_char( "mpfind n lo_vnum hi_vnum text \n\r"
      "   Search mob vnums between lo_vnum and hi_vnum \n\r"
      "   for mob progs that contain an occurrence of text. \n\r"
      "   Display a maxiumu of n lines.\n\r\n\r", ch );
   send_to_char( "mpfind n mud text \n\r"
      "   Search all the mobs in the mud for\n\r"
      "   mob progs that contain an occurrence of text. \n\r"
      "   Display a maxiumu of n lines.\n\r\n\r", ch );
   
   send_to_char( "Example:\n\r", ch);
   send_to_char( "mpfind 20 901 969 if isnpc \n\r"
      "   Search all mob progs in Olympus (vnums 901 thru 969)\n\r"
      "   and display all mobs that contain the text \"if isnpc\".\n\r"
      "   Display a maximum of 20 lines.\n\r\n\r", ch );
   send_to_char( "Example:\n\r", ch);
   send_to_char( "mpfind 100 mud mpslay \n\r"
      "   Search all mob progs in the entire mud\n\r"
      "   and display all mobs that contain the text \"mpslay\".\n\r"
      "   Display a maximum of 100 lines.\n\r\n\r", ch );
}

/*
 * Search mobs for mob progs containing a specified text string.
 */

void do_mpfind( CHAR_DATA *ch, char *argument )   /* Gorog */
{
   MOB_INDEX_DATA  *   pMob;
   MPROG_DATA      *   pProg;
   char                arg1 [MAX_INPUT_LENGTH];
   char                arg2 [MAX_INPUT_LENGTH];
   char                arg3 [MAX_INPUT_LENGTH];
   int                 lo_vnum=1, hi_vnum=MAX_VNUM;
   int                 tot_vnum, tot_hits=0;
   int                 disp_cou=0, disp_limit;
   int			hash;   
   argument = one_argument( argument, arg1 );   /* display_limit */
   argument = one_argument( argument, arg2 );

   if ( arg1[0]=='\0' || arg2[0]=='\0' || !is_number(arg1) )
   {
      mpfind_help(ch);
      return;
   }

   disp_limit = atoi (arg1);
   disp_limit = UMAX(0, disp_limit);

   if ( str_cmp(arg2, "mud") )
   {
      argument = one_argument( argument, arg3 );
      if ( arg3[0]=='\0' || !is_number(arg2) || !is_number(arg3) )
      {
         mpfind_help(ch);
         return;
      }
      else
      {
         lo_vnum = URANGE(1, atoi(arg2), MAX_VNUM);
         hi_vnum = URANGE(1, atoi(arg3), MAX_VNUM);
         if ( lo_vnum > hi_vnum )
         {
            mpfind_help(ch);
            return;
         }
      }
   }   
   if ( argument[0] == '\0' )
   {
      mpfind_help(ch);
      return;
   }

   for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
    for ( pMob = mob_index_hash[hash];
          pMob;
          pMob = pMob->next )
   {
      if ( pMob->vnum < lo_vnum ||
           pMob->vnum > hi_vnum )
        continue;

      if (pMob && (pProg=pMob->mudprogs) )
      {
         tot_vnum = 0;
         for ( ; pProg; pProg=pProg->next)
             tot_vnum += str_count(pProg->comlist, argument);
         tot_hits += tot_vnum;
         if ( tot_vnum && ++disp_cou <= disp_limit)
            pager_printf( ch, "%5d %5d %5d\n\r", disp_cou, pMob->vnum, tot_vnum);
      }
   }
   pager_printf( ch, "Total: %10d\n\r", tot_hits);
}
		
/*
 * Displays the help screen for the "rpfind" command
 */
void rpfind_help (CHAR_DATA *ch)
{
   send_to_char( "Syntax:\n\r", ch);
   send_to_char( "rpfind n lo_vnum hi_vnum text \n\r"
      "   Search room vnums between lo_vnum and hi_vnum \n\r"
      "   for room progs that contain an occurrence of text. \n\r"
      "   Display a maxiumu of n lines.\n\r\n\r", ch );
   send_to_char( "rpfind n mud text \n\r"
      "   Search all the rooms in the mud for\n\r"
      "   room progs that contain an occurrence of text. \n\r"
      "   Display a maxiumu of n lines.\n\r\n\r", ch );
   
   send_to_char( "Example:\n\r", ch);
   send_to_char( "rpfind 20 901 969 if isnpc \n\r"
      "   Search all room progs in Olympus (vnums 901 thru 969)\n\r"
      "   and display all vnums that contain the text \"if isnpc\".\n\r"
      "   Display a maximum of 20 lines.\n\r\n\r", ch );
   send_to_char( "Example:\n\r", ch);
   send_to_char( "rpfind 100 mud mpslay \n\r"
      "   Search all room progs in the entire mud\n\r"
      "   and display all vnums that contain the text \"mpslay\".\n\r"
      "   Display a maximum of 100 lines.\n\r\n\r", ch );
}

/*
 * Search rooms for room progs containing a specified text string.
 */
void do_rpfind( CHAR_DATA *ch, char *argument )   /* Gorog */
{
   ROOM_INDEX_DATA *   pRoom;
   MPROG_DATA      *   pProg;
   char                arg1 [MAX_INPUT_LENGTH];
   char                arg2 [MAX_INPUT_LENGTH];
   char                arg3 [MAX_INPUT_LENGTH];
   int                 lo_vnum=1, hi_vnum=MAX_VNUM;
   int                 tot_vnum, tot_hits=0;
   int                 hash, disp_cou=0, disp_limit;
   
   argument = one_argument( argument, arg1 );   /* display_limit */
   argument = one_argument( argument, arg2 );

   if ( arg1[0]=='\0' || arg2[0]=='\0' || !is_number(arg1) )
   {
      rpfind_help(ch);
      return;
   }

   disp_limit = atoi (arg1);
   disp_limit = UMAX(0, disp_limit);

   if ( str_cmp(arg2, "mud") )
   {
      argument = one_argument( argument, arg3 );
      if ( arg3[0]=='\0' || argument[0]=='\0' 
      ||   !is_number(arg2) || !is_number(arg3) )
      {
         rpfind_help(ch);
         return;
      }
      else
      {
         lo_vnum = URANGE(1, atoi(arg2), MAX_VNUM);
         hi_vnum = URANGE(1, atoi(arg3), MAX_VNUM);
         if ( lo_vnum > hi_vnum )
         {
            rpfind_help(ch);
            return;
         }
      }
   }   
   if ( argument[0] == '\0' )
   {
      rpfind_help(ch);
      return;
   }
/*
   pager_printf( ch, "display:%d lo:%d hi:%d test=\"%s\"\n\r", 
                 disp_limit, lo_vnum, hi_vnum, argument);
*/
   for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
    for ( pRoom = room_index_hash[hash];
          pRoom;
          pRoom = pRoom->next )
   {
      if ( pRoom->vnum < lo_vnum ||
           pRoom->vnum > hi_vnum )
		continue;

      if (pRoom && (pProg=pRoom->mudprogs) )
      {
         tot_vnum = 0;
         for ( ; pProg; pProg=pProg->next)
             tot_vnum += str_count(pProg->comlist, argument);
         tot_hits += tot_vnum;
         if ( tot_vnum && ++disp_cou <= disp_limit)
            pager_printf( ch, "%5d %5d %5d\n\r", disp_cou, pRoom->vnum, tot_vnum);
      }
   }
   pager_printf( ch, "Total: %10d\n\r", tot_hits);
}
		
#define  MAX_DISPLAY_LINES  14000      /* Size of Sort Array             */
#define  MAX_NAME_LENGTH       13
#define  MAX_SITE_LENGTH       16
#define  MAX_FIELD_LENGTH      20
#define  MAX_NUM_OPS           32
#define  GR_NUM_FIELDS         12
#define  GO_NUM_FIELDS         24
#define  UMIN(a, b)            ((a) < (b) ? (a) : (b))

typedef  struct gr_struct       GR_STRUCT;
typedef  struct go_struct       GO_STRUCT;

int get_otype( char *type );    /* fun prototype for fun in build.c */

struct field_struct         /* field table - info re each field          */
{
   char    nam [MAX_FIELD_LENGTH];
   bool    num;             /* is field numeric or char string?          */
}  gr_fd [GR_NUM_FIELDS], go_fd [GO_NUM_FIELDS];

struct                      /* operand table - info about each operand   */
{
   int             field;
   int             op;
   long            nval;        /* value for numeric operands            */
   char            sval [MAX_FIELD_LENGTH];
   bool            num;		/* is field numeric or char string?      */
}  gr_op [MAX_NUM_OPS];         /* the above field is stored here as     */
				/* well as in "fields" for readability   */
struct                          /* operand table - info about each op    */
{
   int             field;
   int             op;
   short           nval;        /* value for numeric operands            */
   char            sval [MAX_FIELD_LENGTH];
   bool            num;		/* is field numeric or char string?      */
}  go_op [MAX_NUM_OPS];

enum gr_field_type          /* enumerates the fields in the input record */
   {name, sex, class, race, level, room, gold, clan, council,
    site, last, pkill};

struct  gr_struct               /* input record containing pfile info    */
{
   char    name [MAX_NAME_LENGTH];
   char    sex;
   char    class;
   char    race;
   char    level;
   short   room;
   long    gold;
   char    clan;
   char    council;
   char    site [MAX_SITE_LENGTH];
   long    last;
   char    pkill;
};

struct  go_struct                /* input record containing object data  */
{
short  n[22];
char  *s[2];
};

/*
 * Sort function used by rgrub to sort integers
 */

int rgrub_int_comp(const void *i, const void *j)
{
return *(int*)i - *(int*)j;
}

void rgrub_help (CHAR_DATA *ch)
{
send_to_char( "Syntax:\n\r", ch);
send_to_char( "rgrub <type> n lo hi -  type search.\n\r"
   "   list room vnums between lo and hi that match n on <type> of search.\n\r", ch );
send_to_char( "   e.g. rgrub st 6 901 969 - list all rooms in Olympus\n\r"
   "      that are sectortype 6.\n\r", ch );
send_to_char( "   e.g. rgrub st 2 - list all rooms sectortype 2.\n\r", ch );
send_to_char( "   e.g. rgrub f nomob 901 969 - list all rooms in Olympus\n\r"
   "      that are flagged nomon.\n\r", ch );
send_to_char( "   e.g. rgrub f nomob - list all rooms flagged nomob.\n\r", ch );
}

void do_rgrub (CHAR_DATA *ch, char *argument)
{
/* modified by Edmond to support room flag searches 000820 */

#define RGRUB_MAX_SIZE 5000
ROOM_INDEX_DATA *pRoom;
int x, lo, hi, hit_cou, cou, vnum[RGRUB_MAX_SIZE];

extern    ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
char arg1[MAX_STRING_LENGTH];
char arg2[MAX_STRING_LENGTH];
char arg3[MAX_STRING_LENGTH];
char arg4[MAX_STRING_LENGTH];

argument = one_argument (argument, arg1);
argument = one_argument (argument, arg2);
argument = one_argument (argument, arg3);
argument = one_argument (argument, arg4);

if (!arg2)                                   /* empty arg gets help scrn */
{
rgrub_help(ch);
return;
}

hit_cou = 0;
lo = (*arg3) ? atoi (arg3) : 0;
hi = (*arg4) ? atoi (arg4) : MAX_VNUM;


if (!str_cmp(arg1, "st"))
{
int match;

match = atoi (arg2); 

ch_printf (ch, "\n\rRoom Vnums\n\r");
for (cou = 0; cou < MAX_KEY_HASH; cou++)
{
if ( room_index_hash[cou] )
for (pRoom = room_index_hash[cou]; pRoom; pRoom = pRoom->next)
{
if (pRoom->vnum >= lo && pRoom->vnum <= hi)
{
if ( match == pRoom->sector_type && hit_cou < RGRUB_MAX_SIZE)
vnum[hit_cou++] = pRoom->vnum;
} } }
qsort(vnum, hit_cou, sizeof(int), rgrub_int_comp);      /* sort vnums    */
for (cou=0; cou<hit_cou; cou++)
ch_printf (ch, "%6d\n\r", vnum[cou]);   /* display vnums */
return;
}

if ( !str_cmp(arg1, "f") )
{

ch_printf (ch, "\n\rRoom Vnums\n\r");
for (cou = 0; cou < MAX_KEY_HASH; cou++)
{
if ( room_index_hash[cou] )
for (pRoom = room_index_hash[cou]; pRoom; pRoom = pRoom->next)
{
if (pRoom->vnum >= lo && pRoom->vnum <= hi)
{
for ( x = 0; x < MAX_ROOM_FLAG ; x++ )
if ( xIS_SET( pRoom->room_flags, x ) )
{
if ( !str_cmp( arg2, r_flags[x] ) && hit_cou < RGRUB_MAX_SIZE )
vnum[hit_cou++] = pRoom->vnum;
} } } }
qsort(vnum, hit_cou, sizeof(int), rgrub_int_comp);      /* sort vnums    */
for (cou=0; cou<hit_cou; cou++)
ch_printf (ch, "%6d\n\r", vnum[cou]);   /* display vnums */
return;
}

rgrub_help(ch);
return;

}


short go_wear_ext (long arg)    /* extract bit set in arg ignoring pos 1 */
{
  short cou;
  if ( arg <= 1 ) return arg;
  for (cou=1; cou<=31; cou++)
     if ( arg & ( (unsigned long) 1 << cou ) ) return cou + 1;
  return -1;
}

int go_strcmp( const char *astr, const char *bstr )
{
    int i;
    for ( ; *astr || *bstr; astr++, bstr++ )
    {
        i=LOWER(*astr)-LOWER(*bstr);
        if ( i ) return i;
    }
    return 0;
}

void go_init (void)
{
  int cou;

  for (cou=0; cou<GO_NUM_FIELDS; cou++)
      go_fd[cou].num=TRUE;
  go_fd[22].num=FALSE;
  go_fd[23].num=FALSE;

  strcpy(go_fd[ 0].nam, "count");
  strcpy(go_fd[ 1].nam, "vnum" );
  strcpy(go_fd[ 2].nam, "type" );
  strcpy(go_fd[ 3].nam, "level");
  strcpy(go_fd[ 4].nam, "wear" );
  strcpy(go_fd[ 5].nam, "avg"  );
  strcpy(go_fd[ 6].nam, "hr"   );
  strcpy(go_fd[ 7].nam, "dr"   );
  strcpy(go_fd[ 8].nam, "hp"   );
  strcpy(go_fd[ 9].nam, "mp"   );
  strcpy(go_fd[10].nam, "ac"   );
  strcpy(go_fd[11].nam, "str"  );
  strcpy(go_fd[12].nam, "dex"  );
  strcpy(go_fd[13].nam, "con"  );
  strcpy(go_fd[14].nam, "wis"  );
  strcpy(go_fd[15].nam, "int"  );
  strcpy(go_fd[16].nam, "luck" );
  strcpy(go_fd[17].nam, "sav0" );
  strcpy(go_fd[18].nam, "sav1" );
  strcpy(go_fd[19].nam, "sav2" );
  strcpy(go_fd[20].nam, "sav3" );
  strcpy(go_fd[21].nam, "sav4" );
  strcpy(go_fd[22].nam, "cname" );
  strcpy(go_fd[23].nam, "name" );
}

char *go_otype_to_disp (int arg)
{
  if ( arg==ITEM_LIGHT     ) return "lt";
  if ( arg==ITEM_SCROLL    ) return "sc";
  if ( arg==ITEM_WAND      ) return "wa";
  if ( arg==ITEM_STAFF     ) return "st";
  if ( arg==ITEM_WEAPON    ) return "wp";
  if ( arg==ITEM_ARMOR     ) return "ar";
  if ( arg==ITEM_POTION    ) return "po";
  if ( arg==ITEM_CONTAINER ) return "cn";
  if ( arg==ITEM_POTION    ) return "po";
  if ( arg==ITEM_NOTE      ) return "no";
  if ( arg==ITEM_KEY       ) return "ky";
  if ( arg==ITEM_FOOD      ) return "fo";
  if ( arg==ITEM_COOK	   ) return "co";
  if ( arg==ITEM_CORPSE_PC ) return "pc";
  if ( arg==ITEM_CORPSE_NPC) return "mc";
  if ( arg==ITEM_PILL      ) return "pi";
  if ( arg==ITEM_BOOK      ) return "bk";
  return NULL;
}

char *owear_to_disp (short arg)
{
  static char owear_disp[20][3] =
     { "??", "ta", "fi", "ne", "bo", "he", "le", "fe", "ha", "ar",
       "sh", "ab", "wa", "wr", "wi", "ho", "du", "ea", "ey", "mi" };

  arg = ( arg<0 || arg>20 ) ? 0 : arg;
  return owear_disp[ arg ];
}

int owear_to_num (char *arg)
{
  if ( !strcmp( arg, "take"    ) ) return  1;
  if ( !strcmp( arg, "finger"  ) ) return  2;
  if ( !strcmp( arg, "neck"    ) ) return  3;
  if ( !strcmp( arg, "body"    ) ) return  4;
  if ( !strcmp( arg, "head"    ) ) return  5;
  if ( !strcmp( arg, "legs"    ) ) return  6;
  if ( !strcmp( arg, "feet"    ) ) return  7;
  if ( !strcmp( arg, "hands"   ) ) return  8;
  if ( !strcmp( arg, "arms"    ) ) return  9;
  if ( !strcmp( arg, "shield"  ) ) return 10;
  if ( !strcmp( arg, "about"   ) ) return 11;
  if ( !strcmp( arg, "waist"   ) ) return 12;
  if ( !strcmp( arg, "wrist"   ) ) return 13;
  if ( !strcmp( arg, "wield"   ) ) return 14;
  if ( !strcmp( arg, "hold"    ) ) return 15;
  if ( !strcmp( arg, "dual"    ) ) return 16;
  if ( !strcmp( arg, "ears"    ) ) return 17;
  if ( !strcmp( arg, "eyes"    ) ) return 18;
  if ( !strcmp( arg, "missile" ) ) return 19;
  return 0;
}
	
int go_fnam_to_num ( char *arg )
{
  int cou;
  
  for (cou=0; cou<GO_NUM_FIELDS; cou++)
      if ( !strcmp(arg, go_fd[cou].nam) )
         return cou;
  return -1;
}

/*
 * Generalized sort function.
 * Sorts either ascending or descending.
 * Sorts an array containing either numbers or strings.
 * 1st parm is a pointer to an array of structures.
 * 2nd parm indicates the starting position within the record
 * 3rd parm indicates the first record in the sort range
 * 4th parm indicates the last  record in the sort range
 *     e.g. the array may contain 100 records but we may wish to sort
 *     only the first fifty.
 * 5th parm is n_s - number/string - TRUE is number - FALSE is string
 * 6th parm is direction - TRUE is ascending - FALSE is descending
 */
void go_sort( CHAR_DATA *ch, GO_STRUCT **p,
        int ind, int left, int right, bool n_s, bool sor_dir )
{
  GO_STRUCT *swap;
  int i=left, j=right, testn = 0;
  static char tests[ MAX_STRING_LENGTH ];

  if ( left < 0 || left >= right ) return;
  right = UMIN(right, MAX_DISPLAY_LINES - 1);

  if ( n_s )
     testn = p[left]->n[ind];
  else
     strcpy( tests, p[left]->s[ind] );

  do
  {
     if ( n_s )
     {
        if ( sor_dir )
           while (p[i]->n[ind] < testn) i++;
        else
           while (p[i]->n[ind] > testn) i++;
        if ( sor_dir )
           while (testn < p[j]->n[ind]) j--;
        else
           while (testn > p[j]->n[ind]) j--;
     }
     else
     {
        if ( sor_dir )
           while ( strcmp( p[i]->s[ind], tests) < 0 ) i++;
        else
           while ( strcmp( p[i]->s[ind], tests) > 0 ) i++;
        if ( sor_dir )
           while ( strcmp( tests, p[j]->s[ind]) < 0 ) j--;
        else
           while ( strcmp( tests, p[j]->s[ind]) > 0 ) j--;
     }

     if (i <= j) { swap=p[i]; p[i] = p[j]; p[j] = swap; i++; j--; }
  } while (i <= j);

if (left < j)  go_sort (ch, p, ind, left,  j, n_s, sor_dir );
if (i < right) go_sort (ch, p, ind, i, right, n_s, sor_dir );
}

void go_accum_aff (GO_STRUCT *r, int loc, int mod)
{
  enum {OCOUNT, OVNUM, OTYPE, OLEVEL, OWEAR, OAVG, OHR, ODR, OHP, OMP, OAC,
        OSTR, ODEX, OCON, OWIS, OINT, OLUCK,
        OSAV0, OSAV1, OSAV2, OSAV3, OSAV4};

switch (loc)
  {
     case APPLY_HITROLL       : {r->n[OHR]  += mod; break;}
     case APPLY_DAMROLL       : {r->n[ODR]  += mod; break;}
     case APPLY_HIT           : {r->n[OHP]  += mod; break;}
     case APPLY_MANA          : {r->n[OMP]  += mod; break;}
     case APPLY_AC            : {r->n[OAC]  += mod; break;}
     case APPLY_STR           : {r->n[OSTR] += mod; break;}
     case APPLY_DEX           : {r->n[ODEX] += mod; break;}
     case APPLY_CON           : {r->n[OCON] += mod; break;}
     case APPLY_WIS           : {r->n[OWIS] += mod; break;}
     case APPLY_INT           : {r->n[OINT] += mod; break;}
     case APPLY_LCK           : {r->n[OLUCK]+= mod; break;}
     case APPLY_SAVING_POISON : {r->n[OSAV0]+= mod; break;}
     case APPLY_SAVING_ROD    : {r->n[OSAV1]+= mod; break;}
     case APPLY_SAVING_PARA   : {r->n[OSAV2]+= mod; break;}
     case APPLY_SAVING_BREATH : {r->n[OSAV3]+= mod; break;}
     case APPLY_SAVING_SPELL  : {r->n[OSAV4]+= mod; break;}
  }
}

void display_operand_table (CHAR_DATA *ch, int op_num)
{
  int cou;
  char opn[7][3] = {"eq", "ne", "su", "ge", "gt", "le", "lt"};

  pager_printf (ch, "OPERAND TABLE\n\r");
  for(cou=0; cou < op_num; cou++)
     if ( go_op[cou].num)
        pager_printf (ch,
        "%2d %-7s %2s %10ld\n\r", cou+1, go_fd[go_op[cou].field].nam,
           opn[go_op[cou].op], go_op[cou].nval);
     else
        pager_printf (ch, "%2d %-7s %2s %s\n\r",
        cou+1, go_fd[go_op[cou].field].nam,
           opn[go_op[cou].op], go_op[cou].sval);
}

/*
 *  Store operand's operator and value in operand table.
 */
bool go_parse_operator (CHAR_DATA *ch, char *pch, int *op_num)
{
  enum op_type {EQ, NE, SU, GE, GT, LE, LT};
  enum {OCOUNT, OVNUM, OTYPE, OLEVEL, OWEAR, OAVG, OHR, ODR, OHP, OMP, OAC,
        OSTR, ODEX, OCON, OWIS, OINT, OLUCK,
        OSAV0, OSAV1, OSAV2, OSAV3, OSAV4};  
  int  cou;
  char opstr [7][3] = { "=", "!=", "<>", ">=", ">", "<=", "<" };

  go_op[*op_num].op = -1;
  for (cou=0; cou<7; cou++)
      if ( !str_prefix(opstr[cou], pch) )
         {
         go_op[*op_num].op = cou;
         break;
         }
  if ( go_op[*op_num].op < 0 )
     {pager_printf(ch, "Invalid operator: %s\n\r", pch); return FALSE;}
  if ( go_op[*op_num].op==EQ || go_op[*op_num].op==GT
  ||   go_op[*op_num].op==LT )
     pch++;
  else pch+=2;                              /* advance to operand value */
  if ( *pch=='\0' )
     {pager_printf(ch, "Value is missing from operand.\n\r"); return FALSE;}

  if ( go_fd[ go_op[ *op_num ].field ].num )
  {
     go_op[*op_num].num  = TRUE;
     if ( isdigit(*pch) )                        /* user entered number */
        go_op[*op_num].nval = atoi ( pch );
     else
     if ( go_op[*op_num].field == OTYPE )
          go_op[*op_num].nval = get_otype( pch ); /* user entered token */
     else
     if ( go_op[*op_num].field == OWEAR )
          go_op[*op_num].nval = owear_to_num( pch ); /* user entered token */
  }
  else
  {
     go_op[*op_num].num  = FALSE;

     if ( strlen(pch) > MAX_FIELD_LENGTH )
     {
        pager_printf(ch, "Char string is too long:%s\n\r", pch);
        return FALSE;
     }
     strcpy ( go_op[*op_num].sval, pch );      /* store str value in table */
  }
  (*op_num)++;                            /* operand now stored in table */
  return TRUE;
}

/*
 * Store operand's field name in the operand table.
 */
bool go_parse_operand (CHAR_DATA *ch, char *arg, int *op_num, int *sor_ind,
        bool *sor_dir, bool *or_sw, bool *np_sw, bool *nm_sw, bool *ng_sw,
        bool *do_sw, bool *d2_sw)
{
  int  cou;
  char *pch;

  if ( !strcmp(arg, "or"    ) ) return *or_sw    = TRUE;
  if ( !strcmp(arg, "np"    ) ) return *np_sw    = TRUE;
  if ( !strcmp(arg, "nm"    ) ) return *nm_sw    = TRUE;
  if ( !strcmp(arg, "ng"    ) ) return *ng_sw    = TRUE;
  if ( !strcmp(arg, "do"    ) ) return *do_sw    = TRUE;
  if ( !strcmp(arg, "d2"    ) ) return *d2_sw = TRUE;

  if ( arg[0]=='+' || arg[0]=='-')
  {
     *sor_dir = (arg[0]=='+') ? TRUE : FALSE;
     pch = arg + 1;
     if ( pch[0] == '\0')
        {
        pager_printf(ch, "Sorry. Missing sort field: %s\n\r", arg);
        return FALSE;
        }

     if ( (*sor_ind = go_fnam_to_num(pch)) == -1 )
        {
        pager_printf(ch, "Sorry. Invalid sort field: %s\n\r", arg);
        return FALSE;
        }
     return TRUE;
  }
                                                 
  for (cou=0; cou<GO_NUM_FIELDS; cou++)           /* check field name    */
      if ( !str_prefix( go_fd[cou].nam, arg ) )
      {
         arg += strlen( go_fd[cou].nam );         /* advance to operator */
         go_op[ *op_num ].field = cou;
						 /* store field enum */
         if ( !go_parse_operator (ch, arg, op_num) )
            return FALSE;
         return TRUE;
      }
  pager_printf(ch, "Sorry. Invalid field name: %s\n\r", arg);
  return FALSE;
}

/*
 * Evaluate one string criteria
 */
bool go_eval_str (char *lval, int op, char *rval)
{
  enum op_type {EQ, NE, SU, GE, GT, LE, LT};
  switch (op)
  {
     case EQ: if ( !str_cmp(lval, rval) ) return TRUE;
              else return FALSE;
     case NE: if (  str_cmp(lval, rval) ) return TRUE;
              else return FALSE;
     case GT: if (  go_strcmp(lval, rval) >  0 ) return TRUE;
              else return FALSE;
     case GE: if (  go_strcmp(lval, rval) >= 0 ) return TRUE;
              else return FALSE;
     case LT: if (  go_strcmp(lval, rval) <  0 ) return TRUE;
              else return FALSE;
     case LE: if (  go_strcmp(lval, rval) <= 0 ) return TRUE;
              else return FALSE;
     case SU: if ( strstr(lval, rval) ) return TRUE;
              else return FALSE;
  }
  return FALSE;
}

/*
 * Evaluate one numeric criteria
 */
bool go_eval_num (long lval, int op, long rval)
{
  enum op_type {EQ, NE, SU, GE, GT, LE, LT};
  switch (op)
  {
     case EQ: return lval == rval;
     case NE: return lval != rval;
     case GE: return lval >= rval;
     case GT: return lval >  rval;
     case LE: return lval <= rval;
     case LT: return lval <  rval;
     default: return FALSE;
  }
}

/*
 * Evaluate one input record to see if it matches all search criteria
 */
bool go_eval_and (CHAR_DATA *ch, GO_STRUCT *r, int op_num)
{
  enum {OCOUNT, OVNUM, OTYPE, OLEVEL, OWEAR, OAVG, OHR, ODR, OHP, OMP, OAC,
        OSTR, ODEX, OCON, OWIS, OINT, OLUCK,
        OSAV0, OSAV1, OSAV2, OSAV3, OSAV4};  
 int  cou;

  for (cou=0; cou<op_num; cou++)
  {
      if ( go_op[cou].field <= OSAV4 )
      {
         if ( !go_eval_num
               (r->n[go_op[cou].field], go_op[cou].op, go_op[cou].nval) )
            return FALSE;
         else continue;
      }
      else
      {
         if ( !go_eval_str(
                 r->s[go_op[cou].field-OSAV4-1], go_op[cou].op, 
                      go_op[cou].sval) )
           return FALSE;
        else continue;
     }
  }
  return TRUE;
}

/*
 * Evaluate one input record to see if it matches any search criteria
 */
bool go_eval_or (CHAR_DATA *ch, GO_STRUCT *r, int op_num)
{
  enum {OCOUNT, OVNUM, OTYPE, OLEVEL, OWEAR, OAVG, OHR, ODR, OHP, OMP, OAC,
        OSTR, ODEX, OCON, OWIS, OINT, OLUCK,
        OSAV0, OSAV1, OSAV2, OSAV3, OSAV4};  
  int  cou;
  for (cou=0; cou<op_num; cou++)
  {
      if ( go_op[cou].field <= OSAV4 )
      {
         if ( go_eval_num( r->n[ go_op[cou].field ], go_op[cou].op,
              go_op[cou].nval ) )
            return TRUE;
         else continue;
     }
     else
     {
        if ( go_eval_str( r->s[go_op[cou].field-OSAV4-1], go_op[cou].op, 
                          go_op[cou].sval) )
           return TRUE;
        else continue;
     }
  }
  return FALSE;
}

void go_display( CHAR_DATA *ch, int dis_num, int tot_match, bool d2_sw,
                 GO_STRUCT **p)
{
  enum {OCOUNT, OVNUM, OTYPE, OLEVEL, OWEAR, OAVG, OHR, ODR, OHP, OMP, OAC,
        OSTR, ODEX, OCON, OWIS, OINT, OLUCK,
        OSAV0, OSAV1, OSAV2, OSAV3, OSAV4};  
  enum {CNAME, ONAME};

  GO_STRUCT r;
  int cou, lim;
  char pri_cname[MAX_NAME_LENGTH];
  char pri_oname[MAX_NAME_LENGTH];

  if ( tot_match > 0 && dis_num > 0 )          /* print title if app  */
  {
  if ( !d2_sw )
      pager_printf(ch,
         "\n\r%-12s%3s %5s %2s %-12s %2s %2s %2s %2s %2s %3s %3s %3s "
         "%11s\n\r",
         "Character", "Cou", "OVnum", "Lv", "OName", "Ty", "We",
         "Av", "Hr", "Dr", "Hp", "Mp", "AC", "S D C W I L");
  else
      pager_printf(ch,
         "\n\r%-12s%3s %5s %2s %-12s %2s %2s %2s %2s %2s %3s %3s %2s "
         "%2s %2s %2s %2s\n\r",
         "Character", "Cou", "OVnum", "Lv", "OName", "Ty", "We",
         "Av", "Hr", "Dr", "Hp", "Mp", "S0", "S1", "S2", "S3", "S4");
   }
   lim = UMIN(tot_match, dis_num);

   for ( cou=0; cou<lim; cou++)
   {
      r = *p[cou];
      strncpy( pri_cname, r.s[CNAME], MAX_NAME_LENGTH - 1);
      pri_cname[ MAX_NAME_LENGTH - 1] = '\0';
      strncpy( pri_oname, r.s[ONAME], MAX_NAME_LENGTH - 1);
      pri_oname[ MAX_NAME_LENGTH - 1] = '\0';
      if ( !d2_sw )
         pager_printf(ch,
            "%-12s%3d %5d%3d %-12s %2s %2s%3d%3d%3d%4d%4d%4d"
            "%2d%2d%2d%2d%2d%2d\n\r", 
            pri_cname, r.n[OCOUNT], r.n[OVNUM], r.n[OLEVEL],
            pri_oname, go_otype_to_disp( r.n[OTYPE] ),
            owear_to_disp( r.n[OWEAR] ),
            r.n[OAVG], r.n[OHR], r.n[ODR],
            r.n[OHP], r.n[OMP], r.n[OAC], r.n[OSTR], r.n[ODEX],
            r.n[OCON], r.n[OWIS], r.n[OINT], r.n[OLUCK]);
      else
         pager_printf(ch,
            "%-12s%3d %5d%3d %-12s %2s %2s%3d%3d%3d%4d%4d%3d"
            "%3d%3d%3d%3d\n\r", 
            pri_cname, r.n[OCOUNT], r.n[OVNUM], r.n[OLEVEL],
            pri_oname, go_otype_to_disp( r.n[OTYPE] ),
            owear_to_disp( r.n[OWEAR] ),
            r.n[OAVG], r.n[OHR], r.n[ODR],
            r.n[OHP], r.n[OMP], r.n[OSAV0], r.n[OSAV1], r.n[OSAV2],
            r.n[OSAV3], r.n[OSAV4]);
  }
  if (tot_match == 0 )
     pager_printf(ch, "Zero matches were found.\n\r");
  else pager_printf(ch,
    "%5d matches in total.\n\r", tot_match);
}

/*
 * Find the name of the character and object and place in record
 * The name of the object is easy ... but the name of the character
 * proved to be one of the most difficult and frustrating aspects ot
 * this function. F.Y.I - if an object is not "carried_by" someone,
 * it could be on the ground ... but ... growl ... it could also be
 * in a container carried by someone - or in a container on the ground.
 */
bool go_read_names( CHAR_DATA *ch, OBJ_DATA *po, GO_STRUCT *r, bool np_sw, 
                    bool nm_sw, bool ng_sw )
{
  enum {CNAME, ONAME};
  OBJ_DATA *pt;
  char *ground = "(none)";
  char *ack    = "(error in data structure)";

  r->s[ONAME] = ( po->name ) ? po->name : ack;  /* set object name */

  if ( po->carried_by )                  /* it's being carried by a char */
  {
     if ( get_trust(ch) < po->carried_by->level ) return FALSE;
     if ( nm_sw &&  IS_NPC(po->carried_by) ) return FALSE;
     if ( np_sw && !IS_NPC(po->carried_by) ) return FALSE;
     r->s[CNAME] = po->carried_by->name;
  }
  else if ( po->in_obj )                 /* it's in a container          */
  {
     pt = po;
     while( pt->in_obj )
            pt=pt->in_obj;
     if ( pt->carried_by && get_trust(ch) < pt->carried_by->level )
        return FALSE;
     if ( pt->carried_by && nm_sw &&  IS_NPC(pt->carried_by) )
        return FALSE;
     if ( pt->carried_by && np_sw && !IS_NPC(pt->carried_by) )
        return FALSE;
     if ( pt->carried_by ) r->s[CNAME] = pt->carried_by->name;
     else
     {
     if ( ng_sw ) return FALSE;
     r->s[CNAME] = ground;
     }
  }
  else if ( !po->in_obj )                /* it's on the ground           */
  {
     if ( ng_sw ) return FALSE;
     r->s[CNAME] = ground;
  }
  return TRUE;
}

bool go_read( CHAR_DATA *ch, int dis_num, int op_num, int sor_ind,
            bool sor_dir, bool or_sw, bool np_sw, bool nm_sw, bool ng_sw,
            bool d2_sw )
{
  enum {OCOUNT, OVNUM, OTYPE, OLEVEL, OWEAR, OAVG, OHR, ODR, OHP, OMP, OAC,
        OSTR, ODEX, OCON, OWIS, OINT, OLUCK,
        OSAV0, OSAV1, OSAV2, OSAV3, OSAV4};  
  OBJ_INDEX_DATA  *px;
  OBJ_DATA        *po;
  AFFECT_DATA     *pa;
  GO_STRUCT         r;                 /* input (physical record)         */
  GO_STRUCT     a[MAX_DISPLAY_LINES];  /* array of records                */
  GO_STRUCT    *p[MAX_DISPLAY_LINES];  /* array of pointers to records    */
  bool ok_otype [255];                 /* we want to process these otypes */
  int  tot_match = 0;                  /* total records matched           */
  bool res;                            /* result of a boolean exp         */
  int ind;			       /* indicates the sort field        */

  memset(ok_otype, 0, sizeof ok_otype);
  ok_otype[ITEM_LIGHT] = ok_otype[ITEM_WAND] = ok_otype[ITEM_KEY] = 
  ok_otype[ITEM_STAFF] = ok_otype[ITEM_WEAPON] = ok_otype[ITEM_ARMOR] =
  ok_otype[ITEM_CONTAINER] = TRUE; 

  for (po=first_object; po; po=po->next)   /* Loop through all objects   */
  {
      if ( !ok_otype[po->item_type] )      /* don't process useless stuff*/
         continue;
      memset(&r, 0, sizeof r);
      if ( !go_read_names( ch, po, &r, np_sw, nm_sw, ng_sw ) )
         continue;
      px          = po->pIndexData;
      r.n[OCOUNT] = po->count;
      r.n[OVNUM]  = px->vnum;
      r.n[OTYPE]  = po->item_type;
      r.n[OLEVEL] = po->level;
      r.n[OWEAR]  = go_wear_ext( po->wear_flags );
      r.n[OAVG]   = (po->item_type == ITEM_WEAPON) ?
                    (po->value[1] + po->value[2])/2 : 0;
      for (pa=px->first_affect; pa; pa=pa->next)
          go_accum_aff (&r, pa->location, pa->modifier);
      for (pa=po->first_affect; pa; pa=pa->next)
          go_accum_aff (&r, pa->location, pa->modifier);
     res = or_sw ? go_eval_or(ch, &r, op_num) : go_eval_and(ch, &r, op_num);

     if ( res )                             /* record is a match         */
     {
        if ( dis_num > 0 && tot_match < MAX_DISPLAY_LINES )
        {
           a[ tot_match ] = r;
           p[ tot_match ] = &a[ tot_match ];
        }
        tot_match++;
     }
  }
  ind = ( sor_ind<=OSAV4 ) ? sor_ind : sor_ind - OSAV4 - 1;

  if ( tot_match > 1 && dis_num > 0 )
     go_sort( ch, p, ind, 0, UMIN((tot_match - 1), MAX_DISPLAY_LINES - 1),
              ( sor_ind <= OSAV4 ), sor_dir );

  go_display( ch, dis_num, tot_match, d2_sw, p );
  return TRUE;
}

/*
  The following code is intended to replace the static arrays "a" and "p"
  with dynamically allocated ones. But I'm confused as to why. If the
  user asks to display 10 lines, but selects 10,000 ... then the arrays
  must be allocated to hold 10,000 so that they may be sorted. Once they
  are sorted, we display only 10 - but - that don't change the fact that
  we have to sort all 10,000 - sigh - arg! - Gorog

  GO_STRUCT        *a;
  GO_STRUCT       **p;
  a = (GO_STRUCT  *) calloc( UMIN(dis_num, MAX_DISPLAY_LINES), sizeof *a);
  if (!a)
  {
     pager_printf(ch, "Sorry. There is currently insufficient memory avail"
     " to service your request. Try later or speak to a coder.\n\r");
     return FALSE;
  }
  p = (GO_STRUCT **) calloc( UMIN(dis_num, MAX_DISPLAY_LINES), sizeof *p);
  if (!p)
  {
     pager_printf(ch, "Sorry. There is currently insufficient memory avail"
     " to service your request. Try later or speak to a coder.\n\r");
     return FALSE;
  }
  free(p); free(a);
*/

void do_ogrub (CHAR_DATA *ch, char *argument)
{
  enum {OCOUNT, OVNUM, OTYPE, OLEVEL, OWEAR, OAVG, OHR, ODR, OHP, OMP, OAC,
        OSTR, ODEX, OCON, OWIS, OINT, OLUCK,
        OSAV0, OSAV1, OSAV2, OSAV3, OSAV4};  
  char arg1[MAX_STRING_LENGTH];
  int  dis_num;                            /* display lines requested     */
  int  op_num = 0;                         /* num of operands on cmd line */
  int  sor_ind  = OVNUM;                   /* sort indicator              */
  bool or_sw    = FALSE;                   /* or search criteria          */
  bool sor_dir  = 1;                       /* sort indicator              */
  bool np_sw    = FALSE;                   /* no players                  */
  bool nm_sw    = FALSE;                   /* no mobs                     */
  bool ng_sw    = FALSE;                   /* no ground objs              */
  bool do_sw    = FALSE;                   /* display operand table       */
  bool d2_sw    = FALSE;                   /* alternate display format    */

  go_init();                              /* initialize data structures  */
  argument = one_argument (argument, arg1);
  if ( !*arg1 )
  {
     pager_printf(ch, "Please type HELP OGRUB for info on OGRUB.\n\r");
     return;
  }
  if ( isdigit(*arg1) )        /* first arg is number of display lines   */
     dis_num = atoi(arg1);
  else
  {
     pager_printf(ch, "You did not specify the number of display lines.\n\r");
     return;
  }
  if ( dis_num > MAX_DISPLAY_LINES )
  {
     pager_printf(ch, "Sorry. You have requested more than %d display " 
                      "lines.\n\r", MAX_DISPLAY_LINES);
     return;
  }

  argument = one_argument (argument, arg1);
  while ( *arg1 )                      /* build the operand table        */
  {
     if ( op_num >= MAX_NUM_OPS )
     {
        pager_printf(ch, "Sorry. You have entered more than %d operands.\n\r",
           MAX_NUM_OPS, MAX_NUM_OPS );
        return;
     }
     if ( !go_parse_operand (ch, arg1, &op_num, &sor_ind, &sor_dir,
        &or_sw, &np_sw, &nm_sw, &ng_sw, &do_sw, &d2_sw ) )
        return;
     argument = one_argument (argument, arg1);
  }
  if (op_num <= 0)
  {
     pager_printf(ch, "Sorry. You did not include any valid operands.\n\r");
     return;
  }
  if ( do_sw ) 
    display_operand_table (ch, op_num);
  if ( !go_read(ch, dis_num, op_num, sor_ind,          /* future expansion*/
        sor_dir, or_sw, np_sw, nm_sw, ng_sw, d2_sw) )
     return;
}

char *gr_strc (char c)           /* convert a char to a str */
{
  static char s[2] = "s";
  s[0]=c;
  return s;
}

/*
 * Evaluate one input record to see if it matches all search criteria
 */
bool gr_eval_and (GR_STRUCT r, int op_num)
{
  int  cou;
  for (cou=0; cou<op_num; cou++)
  {
     switch (gr_op[cou].field)
     {
     case name:
        if ( !go_eval_str (r.name, gr_op[cou].op, gr_op[cou].sval) )
           return FALSE;
        else break;
     case sex:
        if ( !go_eval_num (r.sex, gr_op[cou].op, gr_op[cou].nval) )
           return FALSE;
        else break;
     case class:
        if ( !go_eval_num (r.class, gr_op[cou].op, gr_op[cou].nval) )
           return FALSE;
        else break;
     case race:
        if ( !go_eval_num (r.race, gr_op[cou].op, gr_op[cou].nval) )
           return FALSE;
        else break;
     case level:
        if ( !go_eval_num (r.level, gr_op[cou].op, gr_op[cou].nval) )
           return FALSE;
        else break;
     case room:
        if ( !go_eval_num (r.room, gr_op[cou].op, gr_op[cou].nval) )
           return FALSE;
        else break;
     case gold:
        if ( !go_eval_num (r.gold, gr_op[cou].op, gr_op[cou].nval) )
           return FALSE;
        else break;
     case clan:
        if ( !go_eval_num (r.clan, gr_op[cou].op, gr_op[cou].nval) )
           return FALSE;
        else break;
     case council:
        if ( !go_eval_num (r.council, gr_op[cou].op, gr_op[cou].nval) )
           return FALSE;
        else break;
     case site:
        if ( !go_eval_str (r.site, gr_op[cou].op, gr_op[cou].sval) )
           return FALSE;
        else break;
     case last:
        if ( !go_eval_num (r.last, gr_op[cou].op, gr_op[cou].nval) )
           return FALSE;
        else break;
     case pkill:
        if ( !go_eval_str (gr_strc(r.pkill), gr_op[cou].op, gr_op[cou].sval) )
           return FALSE;
        else break;
     }
  }
  return TRUE;
}

/*
 * Evaluate one input record to see if it matches any search criteria
 */
bool gr_eval_or (GR_STRUCT r, int op_num)
{
  int cou;
  for (cou=0; cou<op_num; cou++)
  {
     switch (gr_op[cou].field)
     {
     case name:
        if ( go_eval_str (r.name, gr_op[cou].op, gr_op[cou].sval) )
           return TRUE;
        else break;
     case sex:
        if ( go_eval_num (r.sex, gr_op[cou].op, gr_op[cou].nval) )
           return TRUE;
        else break;
     case class:
        if ( go_eval_num (r.class, gr_op[cou].op, gr_op[cou].nval) )
           return TRUE;
        else break;
     case race:
        if ( go_eval_num (r.race, gr_op[cou].op, gr_op[cou].nval) )
           return TRUE;
        else break;
     case level:
        if ( go_eval_num (r.level, gr_op[cou].op, gr_op[cou].nval) )
           return TRUE;
        else break;
     case room:
        if ( go_eval_num (r.room, gr_op[cou].op, gr_op[cou].nval) )
           return TRUE;
        else break;
     case gold:
        if ( go_eval_num (r.gold, gr_op[cou].op, gr_op[cou].nval) )
           return TRUE;
        else break;
     case clan:
        if ( go_eval_num (r.clan, gr_op[cou].op, gr_op[cou].nval) )
           return TRUE;
        else break;
     case council:
        if ( go_eval_num (r.council, gr_op[cou].op, gr_op[cou].nval) )
           return TRUE;
        else break;
     case site:
        if ( go_eval_str (r.site, gr_op[cou].op, gr_op[cou].sval) )
           return TRUE;
        else break;
     case last:
        if ( go_eval_num (r.last, gr_op[cou].op, gr_op[cou].nval) )
           return TRUE;
        else break;
     case pkill:
        if ( go_eval_str (gr_strc(r.pkill), gr_op[cou].op, gr_op[cou].sval) )
           return TRUE;
        else break;

     }
  }
  return FALSE;
}

void gr_init (void)
{
  strcpy(gr_fd[ 0].nam, "name"   ); gr_fd[0].num=FALSE;
  strcpy(gr_fd[ 1].nam, "sex"    ); gr_fd[ 1].num=TRUE;
  strcpy(gr_fd[ 2].nam, "class"  ); gr_fd[ 2].num=TRUE;
  strcpy(gr_fd[ 3].nam, "race"   ); gr_fd[ 3].num=TRUE;
  strcpy(gr_fd[ 4].nam, "level"  ); gr_fd[ 4].num=TRUE;
  strcpy(gr_fd[ 5].nam, "room"   ); gr_fd[ 5].num=TRUE;
  strcpy(gr_fd[ 6].nam, "gold"   ); gr_fd[ 6].num=TRUE;
  strcpy(gr_fd[ 7].nam, "clan"   ); gr_fd[ 7].num=TRUE;
  strcpy(gr_fd[ 8].nam, "council"); gr_fd[ 8].num=TRUE;
  strcpy(gr_fd[ 9].nam, "site"   ); gr_fd[ 9].num=FALSE;
  strcpy(gr_fd[10].nam, "last"   ); gr_fd[10].num=TRUE;
  strcpy(gr_fd[11].nam, "pkill"  ); gr_fd[11].num=FALSE;
}

/*
 *  Store operand's operator and value in operand table.
 */
bool gr_parse_operator (CHAR_DATA *ch, char *pch, int *op_num)
{
  enum op_type {EQ, NE, SU, GE, GT, LE, LT};
  int  cou;
  char opstr [7][3] = { "=", "!=", "<>", ">=", ">", "<=", "<" };

  gr_op[*op_num].op = -1;
  for (cou=0; cou<7; cou++)
      if ( !str_prefix(opstr[cou], pch) )
         {
         gr_op[*op_num].op = cou;
         break;
         }

  if ( gr_op[*op_num].op < 0 )
   {ch_printf(ch, "Invalid operator: %s\n\r", pch); return FALSE;}

  if ( gr_op[*op_num].op==EQ || gr_op[*op_num].op==LT
  || gr_op[*op_num].op==GT )
     pch++;
  else pch+=2;                               /* advance to operand value */

  if ( *pch=='\0' )
     {ch_printf(ch, "Value is missing from operand.\n\r"); return FALSE;}

  if ( gr_fd[gr_op[*op_num].field].num )
  {
     gr_op[*op_num].num  = TRUE;
     gr_op[*op_num].nval = atol (pch);   /* store num operand value in table */
  }
  else
  {
     if ( strlen(pch) > MAX_FIELD_LENGTH )
        {ch_printf(ch, "Char string is too long:%s\n\r", pch); return FALSE;}
     gr_op[*op_num].num  = FALSE;
     strcpy (gr_op[*op_num].sval, pch);  /* store str operand value in table */
  }
  (*op_num)++;                         /* operand now stored in table      */
  return TRUE;
}

/*
 * Store operand's field name in the operand table.
 */
bool gr_parse_operand (CHAR_DATA *ch, char *arg, bool *or_sw, int *op_num)
{
  int  cou;

  if ( !strcmp(arg, "or") )
     return *or_sw = TRUE;
                                                 
  for (cou=0; cou<GR_NUM_FIELDS; cou++)          /* check field name    */
      if ( !str_prefix( gr_fd[cou].nam, arg ) )
      {
         arg += strlen( gr_fd[ cou ].nam );     /* advance to operator */
         gr_op[ *op_num ].field = cou;          /* store field name    */
         if ( !gr_parse_operator (ch, arg, op_num) )
            return FALSE;
         return TRUE;
      }
  ch_printf(ch, "Sorry. Invalid field name: %s\n\r", arg);
  return FALSE;
}

/*
 * Read the input file to select records matching the search criteria
 */
void gr_read (
     CHAR_DATA *ch, int op_num, bool or_sw, int dis_num)
{
  FILE *fp;
  bool res;                                 /* result of a boolean exp   */
  bool title_sw = FALSE;                    /* only print title once     */
  int  tot_match = 0;                       /* total records matched     */
  GR_STRUCT r;                              /* input (physical record)   */
  char sex[]   = "NMF";                     /* convert sex to text       */
  char class[] = "MCTWVDRAPN";              /* convert class to text     */
  char race[][3] =                          /* convert race to text      */
  {"Hu", "El", "Dw", "Ha", "Px", "Va", "Og", "HO", "HT", "HE", "Gi",
   "Dr", "SE", "Li", "Gn"};
  char clan[][4] = {
  "   ", "Gui", "DS ", "MS ", "RB ", "AR ", "Sco", "Sur", "Nom", "Oph",
  "Ven", "Inc", "Baa", "Rol", "Asc", "Dae", "Pho", "Ill", "Mer", "Ana" };
  char council[][4] =
  {"   ", "CoE", "MC ", "NC ", "VC ", "PK ", "QC ", "Neo", "Cod", "CoB",
   "Sym", "VC "};

#ifdef WIN32
  if ( ( fp = fopen( "\\smaug\\grub.dat", "rb") ) == NULL )
#else
  if ( ( fp = fopen( "/home/mud/grub.new", "r" ) ) == NULL )
#endif
     return;
  fread( &r, sizeof(r), 1, fp);
  while ( !feof(fp) )                       /* read each input record    */
  {
     if ( or_sw )                           /* is this an "or" search?   */
        res = gr_eval_or( r, op_num );
     else res = gr_eval_and( r, op_num );
     if ( res )                             /* record is a match         */
     {
        tot_match++;
        if ( !title_sw && dis_num > 0 )     /* print title if applicable */
        {
           ch_printf(ch,
           "\n\r%-12s %-2s %1s %-2s %1s %3s %3s %5s %11s %-15s %-6s %s\n\r",
           "Name", "Lv", "S", "R", "C", "Cln", "Cou", "Room", "Gold",
           "Site", "Last", "Pk");
           title_sw = TRUE;
        }
        if ( tot_match <= dis_num )         /* print record if applicable */
           ch_printf(ch,
  "%-12s %2hd %c %2s %c %3s %3s %5hd %11ld %-15s %6lu %c\n\r", 
              r.name, r.level, sex[(unsigned char) r.sex],
              race[(unsigned char) r.race], class[(unsigned char) r.class],
              clan[(unsigned char) r.clan],
              council[(unsigned char) r.council],
              r.room, r.gold, r.site, r.last, r.pkill);
     }
     fread( &r, sizeof(r), 1, fp);
  }
  fclose (fp);
  if (tot_match == 0 )
     ch_printf(ch, "Zero matches were found.\n\r");
  else ch_printf(ch, "%5d matches in total\n\r", tot_match);
}

/*
 * GRUB (Gorog's Revenge on Unruly Bastards)
 *
 * This command is used by mud administrators to obtain info from the
 * player files - often to deal with unruly problem players.
 *
 * It works in two phases. The first phase processes the command line
 * entered by the user and stores each operand in an operand table.
 * Each entry in this table contains the field to be compared, the
 * the operation to be performed and the value to be compared to the
 * record's value.
 *
 * The second phase reads each input record - one record for each player.
 * It then compares the appropriate values from the input record to
 * each operand in the operand table.
 */
void do_grub (CHAR_DATA *ch, char *argument)
{
  char arg1[MAX_STRING_LENGTH];
  bool or_sw = FALSE;                       /* or search criteria           */
  int  dis_num;                             /* display lines requested      */
  int  op_num = 0;                          /* num of operands on cmd line  */

  gr_init();				    /* initialize data structures   */
  argument = one_argument (argument, arg1);
  if ( !*arg1 )
  {
     ch_printf(ch, "Please type HELP GRUB for info on how to use GRUB.\n\r");
     return;
  }
  if ( isdigit(*arg1) )        /* first argument is number of display lines */
     dis_num = atoi( arg1 );
  else
  {
     ch_printf(ch, "You did not specify the number of display lines.\n\r");
     return;
  }

  argument = one_argument (argument, arg1);
  while ( *arg1 )
  {					        /* build the operand table */
     if ( !gr_parse_operand (ch, arg1, &or_sw, &op_num) )
        return;
     argument = one_argument (argument, arg1);
  }
  /*display_operand_table (op_num);*/
  gr_read( ch, op_num, or_sw, dis_num );      /* read the input file     */
}


/*
 * The "showlayers" command is used to list all layerable eq in the
 * mud so that we can keep track of it. It lists one line for each
 * piece of unique eq. If there are 1,000 shrouds in the game, it
 * doesn't list 1,000 lines for each shroud - just one line for the shroud.
 *
 * Redone by Edmond, September of 2000 -- Blodkai
 */

void do_showlayers( CHAR_DATA *ch, char *argument )
{
extern    OBJ_INDEX_DATA  *obj_index_hash[MAX_KEY_HASH];
OBJ_INDEX_DATA *pObj;
char arg1[MAX_STRING_LENGTH];
char arg2[MAX_STRING_LENGTH];
char buf[MAX_STRING_LENGTH];

int hash;                                           /* hash counter */
int cou = 0;                                        /* display counter */
int display_limit;                                  /* display limit */

argument = one_argument (argument, arg1);
argument = one_argument (argument, arg2);

if ( !*arg1 || ( !is_number(arg1) && str_cmp(arg1, "layer") ) )
{
	set_char_color( AT_IMMORT, ch );
	send_to_char( "Syntax:\n\r", ch);
	send_to_char( "showlayers n <w> -  display maximum of n lines <with w wear location.>\n\r", ch);
	send_to_char( "showlayers layer ## - Show all of the sublayers a particular layer number uses.  \n\r", ch);
 	send_to_char( " Example: showlayers 30 - Show 30 layerable items.\n\r", ch);
 	send_to_char( "          showlayers 30 arms  - Show 30 items layerable on arms wearloc\n\r", ch);
 	send_to_char( "          showlayers layer 126  - Displays each of the sublayers to 126\n\r", ch);
	return;
}

if (!str_cmp(arg1, "layer") )
{
	int match;
	int i=1;
	int argnum;

	argnum = atoi(arg2);
	if ( argnum < 1 || argnum > 128 )
	{
		send_to_char("Invalid Layer Number\n\r", ch);
		return;
	}

	ch_printf_color(ch, " &GThe layer number %d uses the following sub layers:\n\r", argnum );
   	ch_printf_color(ch, " &Y--------------------------------------\n\r");
	for ( match = 0; match < 8 ; match++ )
	{
		i = ( match == 0 ? 1 : i*2);

        if IS_SET( (1 << match), argnum )
			ch_printf_color(ch, " &Y%4d", i );
	}
	ch_printf_color(ch, "\n\r");
	return;
}

display_limit = atoi(arg1);

send_to_pager_color("   &B#  &cVnum      &WWear &YLayer   &CDescription\n\r", ch);
for (hash = 0; hash < MAX_KEY_HASH; hash++) /* loop thru obj_index_hash */
{
  if ( obj_index_hash[hash] )
  {
for (pObj=obj_index_hash[hash]; pObj; pObj=pObj->next)
{
if (pObj->layers > 0)
{
      if ( arg2[0]=='\0' || IS_SET( pObj->wear_flags, 1 << get_wflag(arg2) )
)
       if (++cou <= display_limit)
{
sprintf( buf, "&B%4d &c%5d &W%9d &Y%5d   &C%s\n\r",
               cou,
               pObj->vnum,
               pObj->wear_flags,
               pObj->layers,
               pObj->short_descr);
         send_to_pager_color( buf, ch );
}
    }
}
  }
}
if (!cou)
send_to_pager_color("   &RNo Matches\n\r", ch);
return;
}


/*
 * Sorts the arrays "vnums" and "count" based on the order in "count"
 */
void zero_sort( int *vnums, int *count, int left, int right )
{
int i=left, j=right, swap, test;
test = count[(left + right) / 2];
do {
   while (count[i] > test) i++;
   while (test > count[j]) j--;
   if (i <= j) {
      swap=count[i]; count[i] = count[j]; count[j] = swap;
      swap=vnums[i]; vnums[i] = vnums[j]; vnums[j] = swap;
      i++; j--;
      }
   }
while (i <= j);
if (left < j)  zero_sort (vnums, count, left, j);
if (i < right) zero_sort (vnums, count, i, right);
}

void diag_visit_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
   pager_printf(ch, "***obj=%s\n\r", obj->name );
   if ( obj->first_content )
   {
      diag_visit_obj( ch, obj->first_content );
      if ( obj->next_content )
         diag_visit_obj( ch, obj->next_content );
   }
   else
   if ( obj->next_content )
      diag_visit_obj( ch, obj->next_content );
   else
   return;
}

void diag_nivek_obj (CHAR_DATA *ch, OBJ_DATA *obj)
{
   pager_printf(ch, "***obj=%s\n\r", obj->name );
   if ( obj->prev_content )
      diag_nivek_obj( ch, obj->prev_content );
   if ( obj->first_content )
      diag_nivek_obj( ch, obj->last_content );
}

/*
 * Sort function used by diagnose "rf" to sort integers
 */

int diag_int_comp(const void *i, const void *j)
{
return *(int*)i - *(int*)j;
}

/*
 * Displays the help screen for the "diagnose" command
 */
void diagnose_help (CHAR_DATA *ch)
{
send_to_char( "Syntax:\n\r", ch);
send_to_char( "diagnose of n  -  object frequency top n objects\n\r", ch );
send_to_char( "diagnose zero  -  count objects with zero weight\n\r", ch );
send_to_char( "diagnose zero n - list n objects with zero weight\n\r", ch );
send_to_char( "diagnose rf n lo hi - room flag search.\n\r"
   "   list room vnums between lo and hi that match n.\n\r", ch );
send_to_char( "   e.g. diagnose rf 6 901 969 - list all rooms in Olympus\n\r"
   "      that are nomob and deathtraps.\n\r", ch );
send_to_char( "   e.g. diagnose rf 2 - list all deathtraps.\n\r", ch );
send_to_char( "diagnose mrc num race class vnum1 vnum2 - mobs/race/class\n\r"
   "   display all mobs of a particular race/class combo.\n\r"
   "   e.g. diagnose mrc 50 0 3 7500 7534 - show 50 human warriors "
   " in Edo.\n\r", ch);

}

/*
 * Takes an object vnum and the count of the number of times
 * that object occurs and decides whether or not to include it in the
 * frequency table which contains the "top n" frequently occurring objects.
 */

void diag_ins (OBJ_INDEX_DATA *p, int siz, OBJ_INDEX_DATA **f, CHAR_DATA *ch)
{
int  cou =  0;                             /* temporary counter */
int  ins = -1;                             /* insert pos in dynamic f array */

if (!f[siz-1] || p->count>f[siz-1]->count) /* don't bother looping thru f */
   while ( cou<siz && ins<0 )              /* should this vnum be insertted? */
      if ( !f[cou++] || p->count > f[cou-1]->count )
         ins = cou-1;                      /* needs to go into pos "cou" */

if ( ins>=0 )                              /* if vnum occurs more frequently */
   {
   for (cou = siz-1; cou > ins; cou--)     /* open a slot in the table */
       f[cou] = f[cou-1];
   f[ins] = p;                             /* insert pointer in empty slot */
   }
}

/*
 * The "diagnose" command is designed to be expandable and take different
 * parameters to handle different diagnostic routines.
 */

void do_diagnose( CHAR_DATA *ch, char *argument )
{
#define   DIAG_MAX_SIZE  1000
extern    OBJ_INDEX_DATA  *obj_index_hash[MAX_KEY_HASH];
extern    ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
extern    MOB_INDEX_DATA  *mob_index_hash[MAX_KEY_HASH];
OBJ_INDEX_DATA *pObj;
OBJ_INDEX_DATA **freq;                        /* dynamic array of pointers */
char arg1 [MAX_INPUT_LENGTH];
char arg2 [MAX_INPUT_LENGTH];
char arg3 [MAX_INPUT_LENGTH];
char arg4 [MAX_INPUT_LENGTH];
char arg5 [MAX_INPUT_LENGTH];
char arg6 [MAX_INPUT_LENGTH];
int   num = 20;                               /* display lines requested */
int   cou;
int   ctimes;

argument = one_argument( argument, arg1 );
argument = one_argument( argument, arg2 );
argument = one_argument( argument, arg3 );
argument = one_argument( argument, arg4 );
argument = one_argument( argument, arg5 );
argument = one_argument( argument, arg6 );

if (!*arg1) {                                 /* empty arg gets help screen */
   diagnose_help(ch);
   return;
   }

if (!str_cmp( arg1, "compass" ))
{
	CHAR_DATA *victim;
	DESCRIPTOR_DATA *d;
	int compcount = 0;

        for ( d = first_descriptor; d; d = d->next )
	{
	    if ( ( d->connected == CON_PLAYING || d->connected == CON_EDITING )
	    && (   victim = d->character ) != NULL && !IS_NPC( victim ) )
	    {
		if ( xIS_SET( victim->act, PLR_COMPASS ) )
		  compcount++;
	    }
	}
	ch_printf( ch, "%d compass users.\n\r", compcount );
	return;
}	


if (!str_cmp( arg1, "clones" ))
{
	DESCRIPTOR_DATA *dsrc, *ddst, *dsrc_next, *ddst_next;
	DESCRIPTOR_DATA *dlistf, *dlistl;
	sh_int clone_count;

	set_pager_color( AT_PLAIN, ch );
	pager_printf( ch, " %-12.12s | %-12.12s | %-s\n\r", "characters", "user", "host" );
	pager_printf( ch, "--------------+--------------+---------------------------------------------\n\r" );

	dlistf = dlistl = NULL;

	for( dsrc = first_descriptor ; dsrc ; dsrc = dsrc_next )
	{
		if( (dsrc->character && !can_see(ch, dsrc->character)) 
		|| !dsrc->user || !dsrc->host )
		{
			dsrc_next = dsrc->next;
			continue;
		}

		pager_printf( ch, " %-12.12s |",
			dsrc->original ? dsrc->original->name : 
			(dsrc->character ? dsrc->character->name : "(No name)") );
		clone_count = 1;

		for( ddst = first_descriptor ; ddst ; ddst = ddst_next )
		{
			ddst_next = ddst->next;

			if( dsrc == ddst )
				continue;
			if( (ddst->character && !can_see(ch, dsrc->character)) 
			|| !ddst->user || !ddst->host )
				continue;

			if( !str_cmp( dsrc->user, ddst->user ) 
			&& !str_cmp( dsrc->host, ddst->host ) )
			{
				UNLINK(ddst, first_descriptor, last_descriptor, next, prev );
				LINK( ddst, dlistf, dlistl, next, prev );
				pager_printf( ch, "              |\n\r %-12.12s |",
					ddst->original ? ddst->original->name : 
					(ddst->character ? ddst->character->name : "(No name)") );
				clone_count++;
			}
		}

		pager_printf( ch, " %-12.12s | %s (%d clone%s)\n\r", 
			dsrc->user, dsrc->host,
			clone_count,
			clone_count > 1 ? "s" : "" );

		dsrc_next = dsrc->next;

		UNLINK(dsrc, first_descriptor, last_descriptor, next, prev );
		LINK( dsrc, dlistf, dlistl, next, prev );
	}


	for( dsrc = dlistf ; dsrc ; dsrc = dsrc_next )
	{
		dsrc_next = dsrc->next;
		UNLINK( dsrc, dlistf, dlistl, next, prev );
		LINK(dsrc, first_descriptor, last_descriptor, next, prev );
	}
	return;
}

if (!str_cmp( arg1, "mcount" ) )
{
    MOB_INDEX_DATA  *  pMob;
    int mcount;
    int hash;
    int totcount = 0;

    if ( arg2[0] == '\0' )
    {
  	send_to_pager_color( "\n\rSyntax:  mcount <minimum amount>\n\r", ch );
	return;
    }

    mcount = atoi( arg2 );

    pager_printf( ch, "\n\rMobile count:\n\r" );
    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
	for ( pMob=mob_index_hash[hash]; pMob; pMob = pMob->next )
	{
		if ( pMob->count < mcount )
		  continue;
		else if ( pMob->count >= mcount )
		{
		  ch_printf( ch, "%5d\t\t%d\t\t%s\n\r",
			pMob->count,
			pMob->vnum,
			pMob->player_name );
		  totcount++;
		}
	}
    pager_printf( ch, "%d found.\n\r", totcount );
    return;
}
 
if (!str_cmp( arg1, "dice" ) )
{
	ch_printf( ch, "%d  %d\n\r",
	  dice(level, 1),
	  dice(level, 6) );
	return;
}
	
if (!str_cmp( arg1, "color1" ) )
{
	send_to_char( "Send_to_char_color with codes\n\r", ch );
	send_to_char_color( "&B!&B!&B!&B!&B!&B!&B!&B!&B!&B!&B!&B!&B!&B!&B!&B\n\r", ch );
	return;
}

if (!str_cmp(arg1, "color2"))
{
	send_to_char( "Send_to_char with codes\n\r", ch );
	send_to_char( "&B!&B!&B!&B!&B!&B!&B!&B!&B!&B!&B!&B!&B!&B!&B!&B\n\r", ch );
	return;
}
if (!str_cmp(arg1, "color3"))
{
	send_to_char( "Send_to_char with no codes\n\r", ch );
	send_to_char( "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\r", ch );
	return;
}

if (!str_cmp(arg1, "color4"))
{
	send_to_char( "Send_to_char_color with no codes\n\r", ch);
	send_to_char_color("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\r", ch );
	return;
}

if (!str_cmp(arg1, "color5"))
{
	send_to_char( "Set_char_color and send_to_char_color in for\n\r", ch );
	for ( ctimes = 0; ctimes < 15; ctimes++ )
	{
	  set_char_color( AT_BLUE, ch );
	  send_to_char( "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\r", ch );
	}
	return;
}

if (!str_cmp(arg1, "color6"))
{
	send_to_char( "Set_char_color one time\n\r", ch );
	set_char_color( AT_BLUE, ch );
	set_char_color( AT_WHITE, ch );
	send_to_char("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\r", ch );
	return;
}

if (!str_cmp( arg1, "roll" ))
{
	ch_printf( ch, "%d\n\r", number_range( 66*.9, 66*1.2 ) );
	return;
}

if (!str_cmp(arg1, "bits"))
{
    int match;
    int argnum;
    int i=1;

    argnum = atoi(arg2);
    ch_printf_color(ch, " &Y| BIT | MATCHED |     INT    |\n\r");
    ch_printf_color(ch, "  &Y----------------------------\n\r");

    for ( match = 0; match < 31 ; match++ )
    {
        i = ( match == 0 ? 1 : i*2);

        if IS_SET( (1 << match), argnum )
            ch_printf_color(ch, " &Y| &c%3d &Y| &W%5d   &Y| &G%10d &Y|\n\r",
                 match, match, i );
        else
            ch_printf_color(ch, " &Y| &c%3d &Y|         |            |\n\r", match);
    }
    ch_printf_color(ch, "  &Y----------------------------\n\r");
    return;
}

if ( !str_cmp(arg1, "time") )
{
struct tm *t = localtime(&current_time);

pager_printf( ch, "mon=%d day=%d hh=%d mm=%d\n\r", 
   t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min);
return;
}

if ( !str_cmp(arg1, "clones") )
{
	DESCRIPTOR_DATA *dsrc, *ddst, *dsrc_next, *ddst_next;
	DESCRIPTOR_DATA *dlistf, *dlistl;
	sh_int clone_count;

	set_pager_color( AT_PLAIN, ch );
	pager_printf( ch, " %-12.12s | %-12.12s | %-s\n\r", "characters", "user", "host" );
	pager_printf( ch, "--------------+--------------+---------------------------------------------\n\r" );

	dlistf = dlistl = NULL;

	for( dsrc = first_descriptor ; dsrc ; dsrc = dsrc_next )
	{
		if( (dsrc->character && !can_see(ch, dsrc->character)) 
		|| !dsrc->user || !dsrc->host )
		{
			dsrc_next = dsrc->next;
			continue;
		}

		pager_printf( ch, " %-12.12s |",
			dsrc->original ? dsrc->original->name : 
			(dsrc->character ? dsrc->character->name : "(No name)") );
		clone_count = 1;

		for( ddst = first_descriptor ; ddst ; ddst = ddst_next )
		{
			ddst_next = ddst->next;

			if( dsrc == ddst )
				continue;
			if( (ddst->character && !can_see(ch, dsrc->character)) 
			|| !ddst->user || !ddst->host )
				continue;

			if( !str_cmp( dsrc->user, ddst->user ) 
			&& !str_cmp( dsrc->host, ddst->host ) )
			{
				UNLINK(ddst, first_descriptor, last_descriptor, next, prev );
				LINK( ddst, dlistf, dlistl, next, prev );
				pager_printf( ch, "              |\n\r %-12.12s |",
					ddst->original ? ddst->original->name : 
					  (ddst->character ? ddst->character->name : "(No name)") );
				clone_count++;
			}
		}

		pager_printf( ch, " %-12.12s | %s (%d clone%s)\n\r", 
			dsrc->user, dsrc->host,
			clone_count,
			clone_count > 1 ? "s" : "" );

		dsrc_next = dsrc->next;

		UNLINK(dsrc, first_descriptor, last_descriptor, next, prev );
		LINK( dsrc, dlistf, dlistl, next, prev );
	}

	for( dsrc = dlistf ; dsrc ; dsrc = dsrc_next )
	{
		dsrc_next = dsrc->next;
		UNLINK( dsrc, dlistf, dlistl, next, prev );
		LINK(dsrc, first_descriptor, last_descriptor, next, prev );
	}
	return;
}

 
if (!str_cmp(arg1, "rf"))
{
   #define DIAG_RF_MAX_SIZE 5000
   ROOM_INDEX_DATA *pRoom;
   int match, lo, hi, hit_cou, cou, vnum[DIAG_RF_MAX_SIZE];

   if (!*arg2)                                   /* empty arg gets help scrn */
   {
      diagnose_help(ch);
      return;
   }
   else match = atoi (arg2);

   hit_cou = 0;                                 /* number of vnums found */
   lo = (*arg3) ? atoi (arg3) : 0;
   hi = (*arg4) ? atoi (arg4) : MAX_VNUM;

   ch_printf (ch, "\n\rRoom Vnums\n\r");
   for (cou = 0; cou < MAX_KEY_HASH; cou++)
   {
      if ( room_index_hash[cou] )
         for (pRoom = room_index_hash[cou]; pRoom; pRoom = pRoom->next)
         {
            if (pRoom->vnum >= lo && pRoom->vnum <= hi)
            {
            if ( xIS_SET( pRoom->room_flags, match )
            && hit_cou < DIAG_RF_MAX_SIZE)
	       vnum[hit_cou++] = pRoom->vnum;
            }
         }
   }
   qsort(vnum, hit_cou, sizeof(int), diag_int_comp);      /* sort vnums    */
   for (cou=0; cou<hit_cou; cou++)
       ch_printf (ch, "%5d %6d\n\r", cou+1, vnum[cou]);   /* display vnums */
   return;
}

if (!str_cmp(arg1, "of")) {
   if (*arg2)                                    /* empty arg gets dft number */
      num = atoi (arg2);
   if (num > DIAG_MAX_SIZE  || num < 1) {        /* display num out of bounds */
      diagnose_help(ch);
      return;
      }
   CREATE(freq, OBJ_INDEX_DATA *, num);           /* dynamic freq array */
   for (cou = 0; cou < num; cou++)                /* initialize freq array */
       freq[cou] = NULL;                          /* to NULL pointers */
   for (cou = 0; cou < MAX_KEY_HASH; cou++) {     /* loop thru obj_index_hash */
       if ( obj_index_hash[cou] )                 /* safety check */
          for (pObj=obj_index_hash[cou];          /* loop thru all pObjInd */
               pObj; pObj=pObj->next)
               diag_ins (pObj, num, freq, ch);    /* insert pointer into list */
       }
   ch_printf (ch, "\n\rObject Frequencies\n\r");  /* send results to char */
   for (cou = 0; cou < num && freq[cou]; cou++)
       ch_printf(ch, "%3d%8d%8d\n\r", cou+1,freq[cou]->vnum,freq[cou]->count);
   DISPOSE(freq);
   return;
   }

if (!str_cmp(arg1, "mm")) {
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;

   if ( !*arg2 )
      return;

   if ( get_trust(ch) < LEVEL_SUB_IMPLEM )
      return;

   if ( ( victim = get_char_world( ch, arg2 ) ) == NULL )
      {
      send_to_char( "Not here.\n\r", ch );
      return;
      }

   if ( !victim->desc )
      {
      send_to_char( "No descriptor.\n\r", ch );
      return;
      }

   if ( victim == ch )
      {
      send_to_char( "Cancelling.\n\r", ch );
      for ( d = first_descriptor; d; d = d->next )
          if ( d->snoop_by == ch->desc )
             d->snoop_by = NULL;
      return;
      }

   if ( victim->desc->snoop_by )
      {
      send_to_char( "Busy.\n\r", ch );
      return;
      }

   if ( get_trust( victim ) >= get_trust( ch ) )
      {
       send_to_char( "Busy.\n\r", ch );
       return;
       }

   victim->desc->snoop_by = ch->desc;
   send_to_char( "Ok.\n\r", ch );
   return;
   }

if (!str_cmp(arg1, "zero"))
{
   #define ZERO_MAX   1500
   int vnums[ZERO_MAX];
   int count[ZERO_MAX];
   int zero_obj_ind = 0;                        /* num of obj_ind's with 0 wt */
   int zero_obj     = 0;                        /* num of objs with 0 wt */
   int zero_num     = -1;                       /* num of lines requested */

   if (*arg2)
      zero_num = atoi (arg2);
   for (cou = 0; cou < MAX_KEY_HASH; cou++)     /* loop thru obj_index_hash */
       if ( obj_index_hash[cou] )
          for (pObj=obj_index_hash[cou]; pObj; pObj=pObj->next)
              if (pObj->weight == 0) {
                 zero_obj_ind++;
                 zero_obj += pObj->count;
                 if (zero_obj_ind <= ZERO_MAX) {
                    vnums[zero_obj_ind - 1] = pObj->vnum;
                    count[zero_obj_ind - 1] = pObj->count;
                    }
                 }
   if (zero_num > 0) {
      zero_sort (vnums, count, 0, zero_obj_ind - 1);
      zero_num = UMIN (zero_num, ZERO_MAX);
      zero_num = UMIN (zero_num, zero_obj_ind);
      for (cou=0; cou<zero_num; cou++)
          ch_printf (ch, "%6d %6d %6d\n\r",
                     cou+1, vnums[cou], count[cou]);
      }
   ch_printf (ch, "%6d %6d\n\r", zero_obj_ind, zero_obj);
   return;
}

if (!str_cmp(arg1, "visit"))
{
diag_visit_obj( ch, ch->first_carrying);
return;
}

if (!str_cmp(arg1, "nivek"))
{
diag_nivek_obj( ch, ch->first_carrying);
return;
}

if (!str_cmp(arg1, "xxxobxxx"))
{
OBJ_INDEX_DATA *px;
OBJ_DATA       *po, *pt = NULL;
AFFECT_DATA    *pa;
int            i=0;
char           buf[MAX_STRING_LENGTH];

pa=NULL;
ch_printf(ch, "CHAR name=%s \n\r", ch->name);
strcpy(buf, ch->first_carrying ? ch->first_carrying->name : "NULL");
ch_printf(ch, "   first_carry=%s\n\r", buf);
strcpy(buf, ch->last_carrying ? ch->last_carrying->name : "NULL");
ch_printf(ch, "   last_carry=%s\n\r", buf);

/*
for (pa=ch->first_affect; pa; pa=pa->next)
   ch_printf(ch,
   "   type=%d duration=%d location=%d modifier=%d bitvector=%d\n\r",
   pa->type, pa->duration, pa->location, pa->modifier, pa->bitvector);
 */

for (po=first_object; po; po=po->next)
{
    i++;
    pt=NULL;
    if ( !po->carried_by && !po->in_obj ) continue;
    if ( !po->carried_by )
    {
       pt = po;
       while( pt->in_obj )           /* could be in a container on ground */
              pt=pt->in_obj;
    }
    if ( ch==po->carried_by || (pt && ch==pt->carried_by) )
    {
       px = po->pIndexData;
       ch_printf(ch, "\n\r%d OBJ name=%s \n\r", i, po->name);
       strcpy(buf, po->next_content ? po->next_content->name : "NULL");
       ch_printf(ch, "   next_content=%s\n\r", buf);
       strcpy(buf, po->prev_content ? po->prev_content->name : "NULL");
       ch_printf(ch, "   prev_content=%s\n\r", buf);
       strcpy(buf, po->first_content ? po->first_content->name : "NULL");
       ch_printf(ch, "   first_content=%s\n\r", buf);
       strcpy(buf, po->last_content ? po->last_content->name : "NULL");
       ch_printf(ch, "   last_content=%s\n\r", buf);

/*  
     ch_printf(ch, 
       "\n\rINDEX_DATA vnum=%d name=%s level=%d extra_flags=%d\n\r",
       px->vnum, px->name, px->level, px->extra_flags);

       ch_printf(ch,
       "v0=%d v1=%d v2=%d v3=%d v4=%d v5=%d item_type=%d\n\r",
       px->value[0], px->value[1], px->value[2], px->value[3],
       px->value[4], px->value[5], px->item_type);
*/
/*
       for (pa=px->first_affect; pa; pa=pa->next)
           ch_printf(ch,
           "   type=%d duration=%d location=%d modifier=%d bitvector=%d\n\r",
           pa->type, pa->duration, pa->location, pa->modifier, pa->bitvector);
*/
/*
      ch_printf(ch,
      "\n\rOBJECT_DATA %d name=%s level=%d wear_flags=%d wear_loc=%d\n\r",
      i, po->name, po->level, po->wear_flags, po->wear_loc);
*/
/*
      ch_printf(ch,
      "v0=%d v1=%d v2=%d v3=%d v4=%d v5=%d item_type=%d\n\r",
       po->value[0], po->value[1], po->value[2], po->value[3],
       po->value[4], po->value[5], po->item_type);
*/
/*
       for (pa=po->first_affect; pa; pa=pa->next)
           ch_printf(ch,
           "   type=%d duration=%d location=%d modifier=%d bitvector=%d\n\r",
           pa->type, pa->duration, pa->location, pa->modifier, pa->bitvector);
*/

    }
}
return;
}

if (!str_cmp(arg1, "mrc"))
{
   MOB_INDEX_DATA *pm;
   sh_int cou, race, class, dis_num, vnum1, vnum2, dis_cou = 0;

   if ( !*arg2 || !*arg3 || !*arg4 || !*arg5  || !*arg6
   ||  !isdigit(*arg2) || !isdigit(*arg3) || !isdigit(*arg4)
   ||  !isdigit(*arg5) || !isdigit(*arg6) )
   {
      send_to_char( "Sorry. Invalid format.\n\r\n\r", ch);
      diagnose_help(ch);
      return;
   }
   dis_num  = UMIN(atoi (arg2), DIAG_MAX_SIZE);
   race     = atoi (arg3);
   class    = atoi (arg4);
   vnum1    = atoi (arg5);
   vnum2    = atoi (arg6);
/*
   ch_printf(ch, "dis_num=%d race=%d class=%d vnum1=%d vnum2=%d\n\r",
       dis_num, race, class, vnum1, vnum2);
*/
   send_to_char("\n\r", ch);

   for (cou = 0; cou < MAX_KEY_HASH; cou++)
   {
      if ( mob_index_hash[cou] )
         for (pm = mob_index_hash[cou]; pm; pm = pm->next)
         {
            if ( pm->vnum >= vnum1 && pm->vnum <= vnum2
            &&   pm->race==race && pm->class==class && dis_cou++ < dis_num )
                pager_printf( ch, "%5d %s\n\r", pm->vnum, pm->player_name );
         }
   }
   return;
}

diagnose_help( ch );
}

