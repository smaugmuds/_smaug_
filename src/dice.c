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
     |                       -*- Dice Game Module -*-                      |
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

/*
 *  Programmer: Cameron Taylor, a.k.a. Shanyr
 *
 *  Creation Date: 3 May 2005
 *  Last Modified Date: 15 July 2006
 *
 *  Notes:   Put do_dice in where you think it fits; don't forget the DO_FUN
 *         entry; you may need to add an #include or two for is_number() and
 *         atoi().
 *
 *           'dice' will roll two six-sided dice.  'dice' followed by a number
 *         will restrict the number to 1-10 and roll that number of six-sided
 *         dice.  'dice' followed by two numbers will restrict the first number
 *         to 1-10 and the second number to 2-20 and roll (first number) dice
 *         of (second number) sides.  The values of all dice rolled are listed,
 *         and the total.
 *
 *           The length of the strings stored in buf1 and buf2 will not exceed
 *         102 bytes (or 103, counting the '\0'), unless MAX_DICE or MAX_SIDES
 *         is changed.
 *
 *           If you have a 'snippets' help file, I would appreciate (but I do
 *         not require) an entry similar to "Dice code by Cameron Taylor" or
 *         "Dice code by Shanyr".  Again, this is requested, not required.
 *
 *  Legal: Copyright 2005 Cameron Taylor
 *
 *         Freely licensed for use by anyone, provided that the following
 *         disclaimer is accepted and this legal stuff remains in the source
 *         code.  I do not require compiled code to mention me to the end user.
 *
 *         THIS CODE IS PROVIDED "AS IS" WITH NO WARRANTIES OF ANY KIND.  UNDER
 *         NO CIRCUMSTANCES SHALL THE COPYRIGHT HOLDER BE HELD LIABLE FOR ANY
 *         DAMAGES, WHETHER DIRECT, INDIRECT, OR OTHERWISE.  USE AT YOUR OWN
 *         RISK.
 */

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include "mud.h"

/*
 *  I use this macro to check for data content in a string
 */
#define STR_HASDATA(str) (str != NULL && str[0] != '\0')

/*
 *  This function allows a character to roll 1 or more dice.
 *  Programmer: Cameron Taylor (Shanyr)
 */
void do_dice( CHAR_DATA *ch, char *argument )
{
    const int MAX_DICE = 10;
    const int MAX_SIDES = 20;
    int number=2, sides=6;
    int total = 0;
    int dice[10];
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    int i;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if(STR_HASDATA(arg1)) /* Check for the number of dice */
    {
        if(is_number(arg1))
            number = atoi(arg1);

        /* Limit number to the range [1, MAX_DICE] */
        number = number > MAX_DICE ? MAX_DICE : (number < 1 ? 1 : number);

        if(STR_HASDATA(arg2)) /* Check for the number of sides */
        {
            if(is_number(arg2))
                sides = atoi(arg2);

            /* Limit sides to the range [2, MAX_SIDES] */
            sides = sides > MAX_SIDES ? MAX_SIDES : (sides < 2 ? 2 : sides);
        }
    }

    /* Roll the dice and find the total */
    for(i=0; i<number; i++)
        total += (dice[i]=number_range(1, sides));

    /* Build the string describing the die roll */
    if(number == 1)
        sprintf(buf1, "%s up a %d-sided die and %s %d.", "%s", sides, "%s", dice[0]);
    else
    {
        sprintf(buf1, "%s up %d %d-sided dice and %s %d%s", "%s", number, sides, "%s", dice[0], "%s");
        for(i=1; i<number-1; i++)
        {
            sprintf(buf2, buf1, "%s", "%s", ", %d%s");
            sprintf(buf1, buf2, "%s", "%s", dice[i], "%s");
        }
        sprintf(buf2, buf1, "%s", "%s", " and %d, for a total of %d.");
        sprintf(buf1, buf2, "%s", "%s", dice[number-1], total);
    }

    /* Polish off the string and send it to the players */
    sprintf(buf2, buf1, "You pick", "roll");
    act(AT_ACTION, buf2, ch, NULL, NULL, TO_CHAR);
    sprintf(buf2, buf1, "$n picks", "rolls");
    act(AT_ACTION, buf2, ch, NULL, NULL, TO_ROOM);
}
