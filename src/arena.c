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
     |                  -*- Player Informational Module -*-                |
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

/***************************************************************************
 *	ROM 2.4 is copyright 1993-1995 Russ Taylor		                         *
 *	ROM has been brought to you by the ROM consortium	   	                 *
 *	    Russ Taylor (rtaylor@pacinfo.com)			 	                           *
 *	    Gabrielle Taylor (gtaylor@pacinfo.com)		                         *
 *	    Brian Moore (rom@rom.efn.org)			                                 *
 *	By using this code, you have agreed to follow the terms of the	       *
 *	ROM license, in the file Rom24/doc/rom.license  	                     *
 ***************************************************************************/
/***************************************************************************
 *       ROT 1.4 is copyright 1996-1997 by Russ Walsh                      *
 *       By using this code, you have agreed to follow the terms of the    *
 *       ROT license, in the file doc/rot.license                          *
 ***************************************************************************/
/***************************************************************************
 *  Hello and thanks for trying out my arena code. :) All I really have to *
 *  is that I would like a little credit if you dont mind like keeping     *
 *  the statements I make in the file and keeping this header in the file  *
 *  I dont REQUIRE you to give me credit although its always nice to       *
 *  credit the coders for their work(hint hint :p).  Anyways all I really  *
 *  want is for you people to have fun anyways adios.  Any problems,       *
 *  changes, bugs, comments or anything you might wanna see on the next    *
 *  release(if there even is one) mail me at shuvthisupyourass@hotmail.com *
 ******************************[Happy MUDding]******************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "mud.h"

/* Alright you can delete this after you read it I only have here here for a few 
 * ideas and such.  Alright on my mud I have it so you can yourself and fight a 
 * that is cloned after you.  Also I am working on having it so you can challenge
 * groups and guilds and do battle that way which comes in handy for NPK guilds and
 * such.  Anyways if anyone has any ideas or finds some bugs as I didn't test this 
 * all that much just to see if it worked and I could challenge after I challenge 
 * someone.  Alright thats all anyways mail me if you find bugs or have ideas or
 * wanna make a comment about it :) anyways adios have fun 
 */

bool is_challenge;
bool challenge_tme = 0;
bool arena_is_busy = FALSE;

/*
 * do_challenge sends the challenge
 */
void do_challenge(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *room1;
    ROOM_INDEX_DATA *room2;
    char buf[MAX_STRING_LENGTH];

	int i = 0;

    if (argument[0] == '\0')
    {
        send_to_char("You must specify who you want to challenge.\n\r", ch);
        return;
    }

	if (is_challenge)
	{
		send_to_char("Someone has been challenged wait a few moments the try again.",ch);
		return;
    }

	if (arena_is_busy)
    {
		send_to_char("The arena is being used at the moment..Please wait a few minutes.\n\r",ch);
		return;
    }

	if ((victim = get_char_world(ch,argument)) == NULL)
    {

		act (AT_ACTION, "Sorry but $t seems to be gone from the realms at this time.'",
		    ch,argument, NULL, TO_CHAR);
        return; /* simply change the acts to send_to_char if you dont want this. */
    }

    if(xIS_SET(ch->act,ACT_CHALLENGED))
    {
		send_to_char("You have been challenged already.  Accept or decline.\n\r",ch);
		return;
    }

    if(xIS_SET(victim->act,ACT_CHALLENGER))
    {
		act (AT_RED, "$N has already challenged someone else.", ch, NULL, victim, TO_CHAR);
		return;
    }

	if (xIS_SET(victim->act,PLR_AFK))
	{
		act(AT_YELLOW, "Sorry but $N is AFK at the moment.", ch,NULL,victim,TO_CHAR);
		return; /* simply change the acts to send_to_char if you dont want this. */
    }

      /* this check was changed you need to change what it says though :p */
  if (IS_NPC (victim) || IS_NPC (ch))
	// if (IS_NPC(victim) || ch->victim == ch) 
    {
        send_to_char("You can only challenge players or yourself.\n\r",ch);
        return; /* simply change the acts to send_to_char if you dont want this. */
    }

    if (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch))
	{
		act(AT_RED, "Sorry but $E is a higher being.",ch,NULL,victim,TO_CHAR);
		send_to_char("Besides they all would laugh at you!\n\r",ch);
		return; /* simply change the acts to send_to_char if you dont want this. */
 	}

	if (victim->level <= 5)
	{
		act(AT_WHITE, "Sorry but $E is not experienced enough.",ch,NULL,victim,TO_CHAR);
		return; /* simply change the acts to send_to_char if you dont want this. */
	}

	if (victim->fighting != NULL )
    {
        act(AT_ACTION, "Sorry but $N is in combat right now.",ch,NULL,victim,TO_CHAR);
        return;
    }

	if (!str_cmp(argument,"self") || !str_cmp(argument,"mirror match"))
	{              
          return; /* Sorry I dont wanna give away all my stuff right now :p */
      }

	/* this is where the challenge gets sent */
	xSET_BIT(ch->act,ACT_CHALLENGER);
	send_to_char("Your challenge has been sent if they done accept in 3 ticks it will decline",ch);
	sprintf(buf,"\n\rIf they do not accept it will be automatically withdrawn from %s.\n\r",ch->name);
	send_to_char(buf,ch);
	act(AT_RED, "You have been challenged by $n.",ch,NULL,victim,TO_VICT);
	sprintf(buf,"Type accept %s to accept or decline %s to decline.\n\r",ch->name, ch->name);
	send_to_char(buf,victim);
	challenge_tme = 5;
	is_challenge = TRUE;
	return;
}

/*
 * Accepts a challenge
 */
void do_accept(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *room1;
    ROOM_INDEX_DATA *room2;
    
    if (IS_NPC(ch))
    {
         send_to_char("Hrmm....You must be a new kinda mobile to accept challenges eh?",ch);
         return;
    }

    if (argument[0] == '\0')
    {
        send_to_char("You must be specify who challenged you I am only a MUD that can do so mud.\n\r", ch);
        return;
    }

	if ((victim = get_char_world(ch,argument)) == NULL)
    {
		act (AT_ACTION, "Sorry but $t seems to be gone from the realms at this time.'",
		    ch,argument, NULL, TO_CHAR);
        return; /* simply change the acts to send_to_char if you dont want this. */
    }

    if (IS_NPC(victim))
    {
        send_to_char("Hrmm, I dont think I can let you die like that.\n\r", ch);
        return;
    }

    if (victim == ch)
    {
        send_to_char("I bet you think your funny eh?\n\r",ch);
        return;
    }

    if (!xIS_SET(victim->act, ACT_CHALLENGER))
    {
        send_to_char("Now have you drank that much ale that you can't remember who challenged you?\n\r",ch);
        return;
    }
    
    /* these will need to get changed for your mud :p */
    room1 = get_room_index(number_range(ROOM_VNUM_ARENA_MIN, ROOM_VNUM_ARENA_MAX));
    room2 = get_room_index(number_range(ROOM_VNUM_ARENA_MIN, ROOM_VNUM_ARENA_MAX));

    sprintf(buf, "%s has accepted your challenge!\n\r", ch->name);
    send_to_char(buf, victim);
    send_to_char("You plop onto the ground.\n\r",ch);
    char_from_room(ch);
    char_to_room( ch, room1 );
    do_look(ch,"auto");
    send_to_char("You plop onto the ground.\n\r",victim);
    char_from_room(victim);
    char_to_room( victim, room2 );
    do_look(ch, "auto");
    xSET_BIT(ch->act,ACT_CHALLENGED);
    xSET_BIT(ch->act,PLR_SILENCE);
//    SET_BIT(ch->act,PLR_NORESTORE);
    xSET_BIT(victim->act,PLR_SILENCE);
//    xSET_BIT(victim->act,PLR_NORESTORE);
    arena_is_busy = TRUE;
    is_challenge = FALSE;
    challenge_tme = 0;
    return;
}

void do_decline(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    if (argument[0] == '\0')
    {
        send_to_char("You must specify who you want to challenge.\n\r", ch);
        return;
    }

	if (!is_challenge)
	{
		send_to_char("Nobody hase even challenged someone wanna try again?",ch);
		return;
      }

    if (arena_is_busy)
    {
		send_to_char("Alright the arena is in use so no one yet again has challenged you.\n\r",ch);
		return;
    }

    if ((victim = get_char_world(ch,argument)) == NULL)
    {

		act(AT_ACTION, "Sorry but $t seems to be gone from the realms at this time.'",
		    ch,argument, NULL, TO_CHAR );
        return; /* simply change the acts to send_to_char if you dont want this. */
    }

    if(!xIS_SET(victim->act,ACT_CHALLENGER))
    {
		act(AT_RED, "$N hasn't challenged you.",ch,NULL,victim,TO_CHAR);
		return;
    }

    if (IS_NPC(victim) || victim == ch)
    {
        send_to_char("How did that happen?\n\r",ch);
        return; /* simply change the acts to send_to_char if you dont want this. */
    }

    is_challenge = FALSE;
    xREMOVE_BIT(ch->act, ACT_CHALLENGER);
    sprintf(buf, "%s decline your invitation to death!\n\r", ch->name);
    send_to_char(buf, victim);
    sprintf(buf, "You have choosen not to die by %s!\n\r", victim->name);
    send_to_char(buf, ch);
    return;
}

void do_withdraw(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
    return;

    if (!xIS_SET(ch->act,ACT_CHALLENGER))
    {
        send_to_char("You haven't challenged anyone!\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, argument)) == NULL)
    {
        send_to_char("Does it look like they are here?!?\n\rNO, I didn't think so!!\n\r", ch);
        return;
    }

    if (arena_is_busy)
    {
        send_to_char("Hrmm, how does that work again?!?\n\r", ch);
        return;
    }

    if (IS_NPC(victim))
    {
        send_to_char("I think you are brain dead how many drugs have you done again?\n\r", ch);
        return;
    }

    is_challenge = FALSE;
    xREMOVE_BIT(ch->act,ACT_CHALLENGER);
    sprintf(buf, "%s has withdrawn their challenge!\n\r", ch->name);
    send_to_char(buf, victim);
    sprintf(buf, "You withdraw your challenge to %s!\n\r", victim->name);
    send_to_char(buf, ch);
    return;
}
