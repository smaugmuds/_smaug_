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
     |                         -*- Bank Module -*-                         |
     |_____________________________________________________________________|
     |                                                                     |
     |        Coded by Minas Ravenblood for The Apocalypse Theatre         |
     |                    (email: krisco7@hotmail.com)                     |
     |_____________________________________________________________________|
    //                                                                     \\
   [|  SMAUG 1.4 © 1994-1998 Thoric/Altrag/Blodkai/Narn/Haus/Scryn/Rennard  |]
   [|  Swordbearer/Gorog/Grishnakh/Nivek/Tricops/Fireblade/Edmond/Conran    |]
   [|                                                                       |]
   [|  Merc 2.1 Diku Mud improvments © 1992-1993 Michael Chastain, Michael  |]
   [|  Quan, and Mitchell Tse. Original Diku Mud © 1990-1991 by Sebastian   |]
   [|  Hammer, Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, Katja    |]
   [|  Nyboe. Win32 port Nick Gammon.                                       |]
   [|                                                                       |]
   [|  SMAUG 2.0 © 2014-2015 Antonio Cao (@burzumishi)                      |]
    \\_____________________________________________________________________//
*/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "mud.h"

#include "bank.h"

#ifdef BANK_INSTALLED

void
do_bank (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *banker;
  char arg1[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int amount;

  if (!(banker = find_banker (ch)))
    {
      send_to_char ("You can't seem to find a banker.\n\r", ch);
      return;
    }

  if (IS_NPC (ch))
    {
      sprintf (buf, "Sorry, %s, we don't do business with mobs.",
	       ch->short_descr);
      do_say (banker, buf);
      return;
    }

  if (argument[0] == '\0')
    {
      do_say (banker, "If you need help, see HELP BANK.");
      return;
    }

  argument = one_argument (argument, arg1);

  if (!str_cmp (arg1, "balance"))
    {
      int total = ch->pcdata->balance + ch->gold;

      set_char_color (AT_GREEN, ch);
      sprintf (buf, "You are carrying %d gold coin%s.\n\r", ch->gold,
	       (ch->gold == 1) ? "" : "s");
      send_to_char (buf, ch);
      sprintf (buf, "You also have %d gold coin%s in the bank.\n\r",
	       ch->pcdata->balance, (ch->pcdata->balance == 1) ? "" : "s");
      send_to_char (buf, ch);
      sprintf (buf, "Making a total of %d gold coin%s.\n\r",
	       total, (total == 1) ? "" : "s");
      send_to_char (buf, ch);
      return;
    }

  if (!str_cmp (arg1, "deposit"))
    {
      char arg2[MAX_INPUT_LENGTH];

      argument = one_argument (argument, arg2);

      if (arg2 == '\0')
	{
	  sprintf (buf, "%s How much gold do you wish to deposit?", ch->name);
	  do_tell (banker, buf);
	  return;
	}

      if (str_cmp (arg2, "all") && !is_number (arg2))
	{
	  sprintf (buf, "%s How much gold do you wish to deposit?", ch->name);
	  do_tell (banker, buf);
	  return;
	}

      if (!str_cmp (arg2, "all"))
	amount = ch->gold;
      else
	amount = atoi (arg2);

      if (amount > ch->gold)
	{
	  sprintf (buf,
		   "%s Sorry, but you don't have that much gold to deposit.",
		   ch->name);
	  do_tell (banker, buf);
	  return;
	}

      if (amount <= 0)
	{
	  sprintf (buf,
		   "%s Oh, I see.. I didn't know i was doing business with a comedian.",
		   ch->name);
	  do_tell (banker, buf);
	  return;
	}

      ch->gold -= amount;
      ch->pcdata->balance += amount;
      sprintf (buf, "You deposit %d gold coin%s.\n\r", amount,
	       (amount != 1) ? "s" : "");
      set_char_color (AT_PLAIN, ch);
      send_to_char (buf, ch);
      sprintf (buf, "$n deposits %d gold coin%s.\n\r", amount,
	       (amount != 1) ? "s" : "");
      act (AT_PLAIN, buf, ch, NULL, NULL, TO_ROOM);
      return;
    }

  if (!str_cmp (arg1, "withdraw"))
    {
      char arg2[MAX_INPUT_LENGTH];

      argument = one_argument (argument, arg2);

      if (arg2 == '\0')
	{
	  sprintf (buf, "%s How much gold do you wish to withdraw?",
		   ch->name);
	  do_tell (banker, buf);
	  return;
	}
      if (str_cmp (arg2, "all") && !is_number (arg2))
	{
	  sprintf (buf, "%s How much gold do you wish to withdraw?",
		   ch->name);
	  do_tell (banker, buf);
	  return;
	}

      if (!str_cmp (arg2, "all"))
	amount = ch->pcdata->balance;
      else
	amount = atoi (arg2);

      if (amount > ch->pcdata->balance)
	{
	  sprintf (buf,
		   "%s But you do not have that much gold in your account!",
		   ch->name);
	  do_tell (banker, buf);
	  return;
	}

      if (amount <= 0)
	{
	  sprintf (buf,
		   "%s Oh I see.. I didn't know i was doing business with a comedian.",
		   ch->name);
	  do_tell (banker, buf);
	  return;
	}

      ch->pcdata->balance -= amount;
      ch->gold += amount;
      sprintf (buf, "You withdraw %d gold coin%s.\n\r", amount,
	       (amount != 1) ? "s" : "");
      set_char_color (AT_PLAIN, ch);
      send_to_char (buf, ch);
      sprintf (buf, "$n withdraws %d gold coin%s.\n\r", amount,
	       (amount != 1) ? "s" : "");
      act (AT_PLAIN, buf, ch, NULL, NULL, TO_ROOM);
      return;
    }

  if (!str_cmp (arg1, "transfer"))
    {
      CHAR_DATA *victim;
      char arg2[MAX_INPUT_LENGTH];
      char arg3[MAX_INPUT_LENGTH];

      argument = one_argument (argument, arg2);
      argument = one_argument (argument, arg3);

      if (arg2 == '\0' || arg3 == '\0')
	{
	  sprintf (buf, "%s How much gold do you wish to send to who?",
		   ch->name);
	  do_tell (banker, buf);
	  return;
	}
      if (str_cmp (arg2, "all") && !is_number (arg2))
	{
	  sprintf (buf, "%s How much gold do you wish to send to who?",
		   ch->name);
	  do_tell (banker, buf);
	  return;
	}

      if (!(victim = get_char_world (ch, arg3)))
	{
	  sprintf (buf, "%s %s could not be located.", ch->name,
		   capitalize (arg3));
	  do_tell (banker, buf);
	  return;
	}

      if (IS_NPC (victim))
	{
	  sprintf (buf, "%s We do not do business with mobiles...", ch->name);
	  do_tell (banker, buf);
	  return;
	}

      if (!str_cmp (arg2, "all"))
	amount = ch->pcdata->balance;
      else
	amount = atoi (arg2);

      if (amount > ch->pcdata->balance)
	{
	  sprintf (buf,
		   "%s You are very generous, but you don't have that much gold!",
		   ch->name);
	  do_tell (banker, buf);
	  return;
	}

      if (amount <= 0)
	{
	  sprintf (buf,
		   "%s Oh I see.. I didn't know I was doing business with a comedian.",
		   ch->name);
	  do_tell (banker, buf);
	  return;
	}

      ch->pcdata->balance -= amount;
      victim->pcdata->balance += amount;
      sprintf (buf, "You transfer %d gold coin%s to %s's bank account.\n\r",
	       amount, (amount != 1) ? "s" : "", victim->name);
      set_char_color (AT_GREEN, ch);
      send_to_char (buf, ch);
      sprintf (buf,
	       "%s just transferred %d gold coin%s to your bank account.\n\r",
	       ch->name, amount, (amount != 1) ? "s" : "");
      set_char_color (AT_GREEN, victim);
      send_to_char (buf, victim);
      return;
    }

  if (!str_cmp (arg1, "help"))
    {
      do_help (ch, "bank");
      return;
    }
  return;
}

#endif
