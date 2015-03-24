/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops, Fireblade, Edmond, Conran                         |             *
 * ------------------------------------------------------------------------ *
 * 			Get Abstract Statistics on Objects		    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


typedef  struct gaso_struct GASO_STRUCT;

struct gaso_struct
{
    sh_int              weight;
    int                 cost;
    int                 value   [6];
    int                 count;
    int                 extra_flags[32];
};

int gaso_level(CHAR_DATA * ch, int level);

void 
do_gaso(CHAR_DATA * ch, char *argument)
{
    char            arg[MAX_STRING_LENGTH];
    int             level,level_lo,level_hi, count; 

    count = 0;

    if (!ch->desc) {
        bug("No desc in do_gas\n\r", 0);
    }


    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        level = -1;
    } else {
       level = atoi( arg );
    }

    if ( (level<0) || (level>MAX_LEVEL)) {
        level_lo = level_hi = level;
    } else {
        level_lo=1;
        level_hi=LEVEL_AVATAR;
    }

    for(level=level_lo;level<=level_hi;level++)
      count += gaso_level(ch,level);   

    return;
}

int gaso_level(CHAR_DATA * ch, int level)
{
    char            buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH]; 
    int             count, type, gcount, i;
    GASO_STRUCT     stats[MAX_ITEM_TYPE+1];   /* duh */
    OBJ_DATA        *obj;
    double dcount;
 
    count = 0;
    gcount = 0;

    for(type=0;type<=MAX_ITEM_TYPE;type++){
         stats[type].weight = stats[type].cost = stats[type].count = 0;
         stats[type].value[0] = 0;
         stats[type].value[1] = 0;
         stats[type].value[2] = 0;
         stats[type].value[3] = 0;
         stats[type].value[4] = 0;
         stats[type].value[5] = 0;
         for(i=0;i<32;i++){
            stats[type].extra_flags[i]=0;
         }
    }

    for ( obj = first_object; obj; obj = obj->next ){
        if(obj->level==level){
           count++;
           stats[obj->item_type].count++;
           stats[obj->item_type].weight+=obj->weight;
           stats[obj->item_type].cost+=obj->cost;
           stats[obj->item_type].value[0]+=obj->value[0];
           stats[obj->item_type].value[1]+=obj->value[1];
           stats[obj->item_type].value[2]+=obj->value[2];
           stats[obj->item_type].value[3]+=obj->value[3];
           stats[obj->item_type].value[4]+=obj->value[4];
           stats[obj->item_type].value[5]+=obj->value[5];
           for(i=0;i<32;i++){
		if(xIS_SET(obj->pIndexData->extra_flags, i))
                   stats[obj->item_type].extra_flags[i]++;
           }

        }
        gcount++;
    }

    if(count==0){
       /* send_to_pager("No objects in this range.\n\r",ch); */
       return(0);
    }

    for(type=0;type<=MAX_ITEM_TYPE;type++){ 
        if(stats[type].count!=0){
#define TODUB(x) (  (double)(1.0 * x) )

           dcount= TODUB(stats[type].count);
           sprintf(buf,"%d,%d,%d,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f,%2.2f",
                level,
		type,
                stats[type].count,
		(double) (100.0 * (dcount/TODUB(count))),   /* %-age of objs at this level */
		(double) (TODUB(stats[type].weight) /dcount),  /* average weight of this item_type for lev*/
		(double) (TODUB(stats[type].cost) /dcount),    /* average cost of this item_type for lev*/
		(double) (TODUB(stats[type].value[0]) /dcount),
		(double) (TODUB(stats[type].value[1]) /dcount),
		(double) (TODUB(stats[type].value[2]) /dcount),
		(double) (TODUB(stats[type].value[3]) /dcount),
		(double) (TODUB(stats[type].value[4]) /dcount),
		(double) (TODUB(stats[type].value[5]) /dcount)
 	   );
           for(i=0;i<32;i++){
              sprintf(buf2,",%2.2f", (double) (TODUB(stats[type].extra_flags[i]) /dcount) );
              strcat(buf,buf2);
           }
           strcat(buf,"\n\r");
           send_to_pager(buf,ch);
        }

    }

#undef TODUB

    return count;
}


/*   STUFF TO DEAL WITH:

    int                 wear_flags; 
    sh_int              wear_loc;
{
    AFFECT_DATA *       next;
    AFFECT_DATA *       prev;
    sh_int              type;
    sh_int              duration;
    sh_int              location;
    int                 modifier;
    int                 bitvector;
};

*/
