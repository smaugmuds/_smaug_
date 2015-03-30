#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "mud.h"

#define NUM_DAYS 35
/* Match this to the number of days per month; this is the moon cycle */
#define NUM_MONTHS 17
/* Match this to the number of months defined in month_name[].  */
#define MAP_WIDTH 72
#define MAP_HEIGHT 8
/* Should be the string length and number of the constants below.*/


const char * star_map[] =
{
"                                               C. C.                  g*",
"    O:       R*        G*    G.  W* W. W.          C. C.    Y* Y. Y.    ", 
"  O*.                c.          W.W.     W.            C.       Y..Y.  ",   
"O.O. O.              c.  G..G.           W:      B*                   Y.",     
"     O.    c.     c.                     W. W.                  r*    Y.",
"     O.c.     c.      G.             P..     W.        p.      Y.   Y:  ",
"        c.                    G*    P.  P.           p.  p:     Y.   Y. ",
"                 b*             P.: P*                 p.p:             "
};

/****************** CONSTELLATIONS and STARS *****************************
  Cygnus     Mars        Orion      Dragon       Cassiopeia          Venus
           Ursa Ninor                           Mercurius     Pluto    
               Uranus              Leo                Crown       Raptor
*************************************************************************/	
const char * sun_map[] =
{
"\\`|'/",
"- O -",
"/.|.\\"
};
const char * moon_map[] =
{
" @@@ ",
"@@@@@",
" @@@ "
};

void look_sky ( CHAR_DATA * ch )
{
    static char buf[MAX_STRING_LENGTH];
    static char buf2[4];
    int starpos, sunpos, moonpos, moonphase, i, linenum, precip;

   pager_printf_color(ch,"You gaze up towards the heavens and see:\n\r");

	precip = (ch->in_room->area->weather->precip + 3*weath_unit - 1)/
		weath_unit;
   if (precip > 1)                 
{
  send_to_char("There are some clouds in the sky so you cannot see anything else.\n\r", ch);
   return;
}
   sunpos  = (MAP_WIDTH * (24 - time_info.hour) / 24);
   moonpos = (sunpos + time_info.day * MAP_WIDTH / NUM_DAYS) % MAP_WIDTH;
   if ((moonphase = ((((MAP_WIDTH + moonpos - sunpos ) % MAP_WIDTH ) +
                      (MAP_WIDTH/16)) * 8 ) / MAP_WIDTH)
		         > 4) moonphase -= 8;
   starpos = (sunpos + MAP_WIDTH * time_info.month / NUM_MONTHS) % MAP_WIDTH;
   /* The left end of the star_map will be straight overhead at midnight during 
      month 0 */

   for ( linenum = 0; linenum < MAP_HEIGHT; linenum++ )
   {
     if ((time_info.hour >= 6 && time_info.hour <= 18) &&
         (linenum < 3 || linenum >= 6))
       continue;
     sprintf(buf," ");
   

     /* for ( i = MAP_WIDTH/4; i <= 3*MAP_WIDTH/4; i++)*/
     for ( i = 1; i <= MAP_WIDTH; i++)
     {
       /* plot moon on top of anything else...unless new moon & no eclipse */
       if ((time_info.hour >= 6 && time_info.hour <= 18)  /* daytime? */
        && (moonpos >= MAP_WIDTH/4 - 2) && (moonpos <= 3*MAP_WIDTH/4 + 2) /* in sky? */
        && ( i >= moonpos - 2 ) && (i <= moonpos + 2) /* is this pixel near moon? */
        && ((sunpos == moonpos && time_info.hour == 12) || moonphase != 0  ) /*no eclipse*/
        && (moon_map[linenum-3][i+2-moonpos] == '@'))
       {
         if ((moonphase < 0 && i - 2 - moonpos >= moonphase) ||
             (moonphase > 0 && i + 2 - moonpos <= moonphase))
           strcat(buf,"&W@");
         else
           strcat(buf," ");
       }
       else
       if ((linenum >= 3) && (linenum < 6) && /* nighttime */
           (moonpos >= MAP_WIDTH/4 - 2) && (moonpos <= 3*MAP_WIDTH/4 + 2) /* in sky? */
        && ( i >= moonpos - 2 ) && (i <= moonpos + 2) /* is this pixel near moon? */
        && (moon_map[linenum-3][i+2-moonpos] == '@'))
       {
         if ((moonphase < 0 && i - 2 - moonpos >= moonphase) ||
             (moonphase > 0 && i + 2 - moonpos <= moonphase))
           strcat(buf,"&W@");
         else
           strcat(buf," ");
       }
       else /* plot sun or stars */
       {
         if (time_info.hour>=6 && time_info.hour<=18) /* daytime */
         {
           if ( i >= sunpos - 2 && i <= sunpos + 2 )
           {
             sprintf(buf2,"&Y%c",sun_map[linenum-3][i+2-sunpos]);
             strcat(buf,buf2);
           }
           else
             strcat(buf," ");
         }
         else
         {
           switch (star_map[linenum][(MAP_WIDTH + i - starpos)%MAP_WIDTH])
           {
             default     : strcat(buf," ");    break;
             case ':'    : strcat(buf,":");    break;
             case '.'    : strcat(buf,".");    break;
             case '*'    : strcat(buf,"*");    break;
             case 'G'    : strcat(buf,"&G ");  break;
             case 'g'    : strcat(buf,"&g ");  break;
             case 'R'    : strcat(buf,"&R ");  break;
             case 'r'    : strcat(buf,"&r ");  break;
             case 'C'    : strcat(buf,"&C ");  break;
             case 'O'    : strcat(buf,"&O ");  break;
             case 'B'    : strcat(buf,"&B ");  break;
             case 'P'    : strcat(buf,"&P ");  break;
             case 'W'    : strcat(buf,"&W ");  break;
             case 'b'    : strcat(buf,"&b ");  break;
             case 'p'    : strcat(buf,"&p ");  break;
             case 'Y'    : strcat(buf,"&Y ");  break;
             case 'c'    : strcat(buf,"&c ");  break;
           }
         }
       }
     }
     strcat(buf,"\n\r");
     pager_printf_color(ch,buf);
  }

}

