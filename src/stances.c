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
 * Win32 port by Nick Gammon                                                *
 * ------------------------------------------------------------------------ *
 *       Stances Module written by Shaddai aka Nivek                        * 
 ****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <time.h>
#include "mud.h"


#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                                    \
                                if ( !str_cmp( word, literal ) )        \
                                {                                       \
                                    field  = value;                     \
                                    fMatch = TRUE;                      \
                                    break;                              \
                                }

int get_risflag args ((char *flag));

/* Main function behind stances to drop you into one.  SHADDAI */

void
do_stance (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  int new_stance;

  one_argument (argument, arg);	/* This will have to be changed if we ever
				 * Have stances with more that one word.
				 */


  if (ch->mount)
    {
      send_to_char ("While you are mounted?\n\r", ch);
      return;
    }

/* Do we need this now? SHADDAI
 * if ( IS_AFFECTED( ch, AFF_POLYMORPH ) && !IS_VAMPAFF( ch, VAM_DISGUISED ) )
 * {
 * send_to_char("Not while polymorphed.\n\r",ch);
 * return;
 * }
 */
  if (arg[0] == '\0')
    {
      if (ch->stance == STANCE_NONE)
	{
	  ch->stance = STANCE_NORMAL;
	  send_stance_message (ch, TRUE);
	  update_stances (ch, TRUE);
	  WAIT_STATE (ch, stance_index[STANCE_NONE].wait);
	}
      else
	{
	  update_stances (ch, FALSE);
	  ch->stance = STANCE_NONE;
	  send_stance_message (ch, TRUE);
	  WAIT_STATE (ch, stance_index[STANCE_NORMAL].wait);
	}
      return;
    }
  if (ch->stance > STANCE_NONE)
    {
      send_to_char ("You cannot change stances until you come up from the one you are currently in.\n\r", ch);
      return;
    }
  switch (new_stance = get_stance_number (arg))
    {
    default:
      send_stance_message (ch, FALSE);
      return;
    case STANCE_VIPER:
    case STANCE_CRANE:
    case STANCE_CRAB:
    case STANCE_MONGOOSE:
    case STANCE_BULL:
    case STANCE_MANTIS:
    case STANCE_DRAGON:
    case STANCE_TIGER:
    case STANCE_MONKEY:
    case STANCE_SWALLOW:
      if (can_use_stance (ch, new_stance))
	ch->stance = new_stance;
      else
	{
	  send_stance_message (ch, FALSE);
	  return;
	}
      break;
    }
  send_stance_message (ch, TRUE);
  WAIT_STATE (ch, stance_index[new_stance].wait);
  update_stances (ch, TRUE);
  return;
}

/* This function updates resitances, immunities, and susceptibilites */
void
update_stances (CHAR_DATA * ch, bool flag)
{
  if (flag)
    {
      SET_BIT (ch->stance_resistant, stance_index[ch->stance].resist);
      SET_BIT (ch->stance_immune, stance_index[ch->stance].immune);
      SET_BIT (ch->stance_susceptible, stance_index[ch->stance].suscept);
    }
  else
    {
      REMOVE_BIT (ch->stance_resistant, stance_index[ch->stance].resist);
      REMOVE_BIT (ch->stance_immune, stance_index[ch->stance].immune);
      REMOVE_BIT (ch->stance_susceptible, stance_index[ch->stance].suscept);
    }
}

/*
 * A function to get current stance number. SHADDAI
 */
int
get_stance_mastery ( CHAR_DATA *ch )
{
    if ( ch->stance == STANCE_NONE )
	return 0;
    if ( IS_NPC(ch) )
	return ch->pIndexData->stances[ch->stance];
    else
	return ch->pcdata->stances[ch->stance];
}
 
/* This function returns the name of the stance SHADDAI */
char *
get_stance_name (int stance_num)
{
  switch (stance_num)
    {
    case STANCE_TIGER:
      return "Tiger";
    case STANCE_SWALLOW:
      return "Swallow";
    case STANCE_DRAGON:
      return "Dragon";
    case STANCE_MONKEY:
      return "Monkey";
    case STANCE_MANTIS:
      return "Mantis";
    case STANCE_VIPER:
      return "Viper";
    case STANCE_CRANE:
      return "Crane";
    case STANCE_CRAB:
      return "Crab";
    case STANCE_MONGOOSE:
      return "Mongoose";
    case STANCE_BULL:
      return "Bull";
    case STANCE_NORMAL:
      return "Normal";
    case STANCE_NONE:
      return "None";
    default:
      return NULL;
    }
}
/* This function returns the enum value of a stance name  SHADDAI */

int
get_stance_number (char *argument)
{
  char arg[MAX_INPUT_LENGTH];

  one_argument (argument, arg);	/*If have more than one word name need to
				 *change this.
				 */
  if (!str_cmp (arg, "Tiger"))
    return STANCE_TIGER;
  if (!str_cmp (arg, "Swallow"))
    return STANCE_SWALLOW;
  if (!str_cmp (arg, "Dragon"))
    return STANCE_DRAGON;
  if (!str_cmp (arg, "Monkey"))
    return STANCE_MONKEY;
  if (!str_cmp (arg, "Mantis"))
    return STANCE_MANTIS;
  if (!str_cmp (arg, "Viper"))
    return STANCE_VIPER;
  if (!str_cmp (arg, "Crane"))
    return STANCE_CRANE;
  if (!str_cmp (arg, "Crab"))
    return STANCE_CRAB;
  if (!str_cmp (arg, "Mongoose"))
    return STANCE_MONGOOSE;
  if (!str_cmp (arg, "Bull"))
    return STANCE_BULL;
  if (!str_cmp (arg, "None"))
    return STANCE_NONE;
  if (!str_cmp (arg, "Normal"))
    return STANCE_NORMAL;
  return -1;
}

/* This function determines if a pc or mob can use the stance */

bool
can_use_stance (CHAR_DATA * ch, int new_stance)
{
  bool ability = FALSE;
  int temp_stance = -1;

  /* Have to deal with them in seperate ways
   * Because the data is in different places.
   * SHADDAI */

  if (IS_NPC (ch))
    {
      if (ch->pIndexData && ch->pIndexData->stances[new_stance] > 0)
	ability = TRUE;
      else
	ability = FALSE;
    }
  else
    /* Is a Player */
    {
      if ((temp_stance = stance_index[new_stance].stance[0]) <= 0)
	ability = TRUE;
      else if (ch->pcdata->stances[temp_stance] >= STANCE_GRAND_MASTER)
	{
	  if ((temp_stance = stance_index[new_stance].stance[1]) <= 0)
	    ability = TRUE;
	  else if (ch->pcdata->stances[temp_stance] >= STANCE_GRAND_MASTER)
	    ability = TRUE;
	  else
	    ability = FALSE;
	}
      else
	ability = FALSE;
    }
  if ((stance_index[new_stance].max_weight > 0) &&
      (ch->carry_weight > stance_index[new_stance].max_weight))
    ability = FALSE;
  if (stance_index[new_stance].dual_wield && get_eq_char (ch, WEAR_DUAL_WIELD))
    ability = FALSE;
  if (IS_SET (stance_index[new_stance].class_restrictions, ch->class))
    ability = FALSE;
  if (IS_SET (stance_index[new_stance].race_restrictions, ch->race))
    ability = FALSE;
  return ability;
}

/*  This function sends out the messages of the stances  SHADDAI */

void
send_stance_message (CHAR_DATA * ch, bool flag)
{
  /* Should we check for null pointers?  SHADDAI */
  /* I think it would be better just to dup a default stance to ones that
   * don't have a message listed.   SHADDAI */
  if (flag)
    {
      act (AT_STANCE, stance_index[ch->stance].others, ch, NULL, NULL, TO_ROOM);
      act (AT_STANCE, stance_index[ch->stance].self, ch, NULL, NULL, TO_CHAR);
    }
  else
    {
      send_to_char ("Syntax is: stance <style>.\n\r", ch);
      send_to_char ("Stance being one of: Viper, Crane, Crab, Mongoose, Bull.\n\r", ch);
    }
  return;
}

/* 
 *   This function loads up all the stances        SHADDAI
 */

int
load_stances (void)
{
  char buf[MAX_INPUT_LENGTH];
  char *word;
  FILE *fp;
  bool my_continue = TRUE;
  bool fMatch = FALSE;
  int i;


  /*  Setup default values */

  for (i = 0; i < MAX_STANCE; i++)
    {
      stance_index[i].dual_wield = FALSE;
      stance_index[i].others = NULL;
      stance_index[i].self = NULL;
      stance_index[i].class_restrictions = 0;
      stance_index[i].dam_taken = 0;
      stance_index[i].dam_done = 0;
      stance_index[i].dodge = 0;
      stance_index[i].immune = 0;
      stance_index[i].parry = 0;
      stance_index[i].race_restrictions = 0;
      stance_index[i].resist = 0;
      stance_index[i].special_move = 0;
      stance_index[i].suscept = 0;
      stance_index[i].max_weight = 0;
      stance_index[i].num_attacks = 0;
      stance_index[i].special_percent = 0;
      stance_index[i].stance[0] = 0;
      stance_index[i].stance[1] = 0;
      stance_index[i].wait = 0;
    }
  if ((fp = fopen (STANCE_FILE, "r")) == NULL)
    {
      bug ("Fread_stance: fopen", 0);
      perror (STANCE_FILE);
      return rERROR;
    }

  while (my_continue)
    {
      word = feof (fp) ? "End" : fread_word (fp);
      fMatch = FALSE;

      switch (UPPER (word[0]))
	{
	case 'E':
	  if (!str_cmp (word, "End"))
	    {
	      fclose (fp);
	      fMatch = TRUE;
	      my_continue = FALSE;
	      break;
	    }
	case 'S':
	  if (!str_cmp (word, "StartStance"))
	    {
	      fread_stance (fp);
	      fMatch = TRUE;
	    }
	}
      if (!fMatch)
	{
	  sprintf (buf, "Fread_stance: no match: %s", word);
	  bug (buf, 0);
	}
    }
  log_string("Done.");
  return rNONE;
}

/*
 *  Put randomizing stance data in its own function, in case we want
 *  to put a command or have it randomize every x hours later.  SHADDAI
 */

int
randomize_stances (void)
{
  int i;


  for (i = STANCE_NORMAL; i < MAX_STANCE; i++)
    {
      int change_by = 0;
      if (stance_index[i].dam_taken != 0)
	{
	  change_by = number_range (0, 5);
	  if (number_bits (1) == 0)
	    stance_index[i].dam_taken -= change_by;
	  else
	    stance_index[i].dam_taken += change_by;
	}
      if (stance_index[i].dam_done != 0)
	{
	  change_by = number_range (0, 5);
	  if (number_bits (1) == 0)
	    stance_index[i].dam_done -= change_by;
	  else
	    stance_index[i].dam_done += change_by;
	}
      if (stance_index[i].dodge != 0)
	{
	  change_by = number_range (0, 5);
	  if (number_bits (1) == 0)
	    stance_index[i].dodge -= change_by;
	  else
	    stance_index[i].dodge += change_by;
	}
      if (stance_index[i].parry != 0)
	{
	  change_by = number_range (0, 5);
	  if (number_bits (1) == 0)
	    stance_index[i].parry -= change_by;
	  else
	    stance_index[i].parry += change_by;
	}
      if (stance_index[i].max_weight != 0)
	{
	  change_by = number_range (0, 25);
	  if (number_bits (1) == 0)
	    stance_index[i].max_weight -= change_by;
	  else
	    stance_index[i].max_weight += change_by;
	}
      if (stance_index[i].num_attacks != 0)
	{
	  change_by = number_range (0, 1);
	  if (number_bits (1) == 0)
	    stance_index[i].num_attacks -= change_by;
	  else
	    stance_index[i].num_attacks += change_by;
	}
      if (number_percent () > 96)
	{
	  int value;
	  change_by = number_bits (1);
	  value = number_range (0, 15);
	  if (value == 14)
	    value = 20;
	  else if (value == 15)
	    value = 21;
	  if (change_by)
	    TOGGLE_BIT (stance_index[i].resist, 1 << value);
	  else
	    TOGGLE_BIT (stance_index[i].suscept, 1 << value);
	}
    }
  return rNONE;
}


/*
 *   This function reads in one stance's data.     SHADDAI
 */

int
fread_stance (FILE * fp)
{
  char buf[MAX_INPUT_LENGTH];
  char *word;
  bool fMatch;
  int count = -1;

  word = feof (fp) ? "End" : fread_word (fp);
  count = get_stance_number (word);
  if (count < 0)
    {
      sprintf (buf, "Fread_stance: Bad stance name (%s).", word);
      bug (buf, 0);
    }

  for (;;)
    {
      word = feof (fp) ? "End" : fread_word (fp);
      fMatch = FALSE;

      switch (UPPER (word[0]))
	{
	case 'A':
	  if (count < 0 || count >= MAX_STANCE)
	    break;
	  KEY ("Attacks", stance_index[count].num_attacks, fread_number (fp));
	  break;
	case 'C':
	  if (count < 0 || count >= MAX_STANCE)
	    break;
	  KEY ("Class", stance_index[count].class_restrictions, fread_number (fp));
	  break;
	case 'D':
	  if (count < 0 || count >= MAX_STANCE)
	    break;
	  KEY ("DamDone", stance_index[count].dam_done, fread_number (fp));
	  KEY ("DamTaken", stance_index[count].dam_taken, fread_number (fp));
	  KEY ("Dodge", stance_index[count].dodge, fread_number (fp));
	  KEY ("Dual", stance_index[count].dual_wield, fread_number (fp));
	  break;
	case 'E':
	  if (!str_cmp (word, "End"))
	    {
	      sprintf (buf, "Fread_stance: End found here instead of load_stances");
	      bug (buf, 0);
	      return rERROR;
	    }
	  if (!str_cmp (word, "EndStance"))
	    count = -1;
	  fMatch = TRUE;
	  return rNONE;
	case 'I':
	  if (count < 0 || count >= MAX_STANCE)
	    break;
	  KEY ("Immune", stance_index[count].immune, fread_number (fp));
	  break;
	case 'O':
	  if (count < 0 || count >= MAX_STANCE)
	    break;
	  KEY ("Other", stance_index[count].others, fread_string_nohash (fp));
	  break;
	case 'P':
	  if (count < 0 || count >= MAX_STANCE)
	    break;
	  KEY ("Parry", stance_index[count].parry, fread_number (fp));
	  KEY ("Percent", stance_index[count].special_percent, fread_number (fp));
	  break;
	case 'R':
	  if (count < 0 || count >= MAX_STANCE)
	    break;
	  KEY ("Race", stance_index[count].race_restrictions, fread_number (fp));
	  KEY ("Resist", stance_index[count].resist, fread_number (fp));
	  break;
	case 'S':
	  if (count < 0 || count >= MAX_STANCE)
	    break;
	  KEY ("Self", stance_index[count].self, fread_string_nohash (fp));
	  if (!str_cmp (word, "Special"))
	    {
	      stance_index[count].special_move = (int) get_special_number (fread_word (fp));
	      fMatch = TRUE;
	    }
	  if (!str_cmp (word, "Stance"))
	    {
	      int temp = -1;
	      temp = get_stance_number (fread_word (fp));
	      if (temp <= 0)
		{
		  sprintf (buf, "Fread_stance: Bad stance name (%s).", word);
		  bug (buf, 0);
		}
	      else
		stance_index[count].stance[0] = temp;
	      temp = get_stance_number (fread_word (fp));
	      if (temp <= 0)
		{
		  sprintf (buf, "Fread_stance: Bad stance name (%s).", word);
		  bug (buf, 0);
		}
	      else
		stance_index[count].stance[1] = temp;
	      fMatch = TRUE;
	    }
	  KEY ("Suscept", stance_index[count].suscept, fread_number (fp));
	  break;
	case 'W':
	  if (count < 0 || count >= MAX_STANCE)
	    break;
	  KEY ("Wait", stance_index[count].wait, fread_number (fp));
	  KEY ("Weight", stance_index[count].max_weight, fread_number (fp));
	  break;
	}
      if (!fMatch)
	{
	  sprintf (buf, "Fread_stance: no match: %s", word);
	  bug (buf, 0);
	}
    }
}

/*  
 * This function writes out the stance data to the dat file. SHADDAI
 */

int
fwrite_stance (void)
{
  char strsave[MAX_INPUT_LENGTH];
  FILE *fp;
  int i;

  fclose (fpReserve);
  sprintf (strsave, "%s%s", SYSTEM_DIR, STANCE_FILE);
  if ((fp = fopen (strsave, "w")) == NULL)
    {
      bug ("Fwrite_stance: fopen", 0);
      perror (strsave);
      fclose (fp);
      fpReserve = fopen (NULL_FILE, "r");
      return rERROR;
    }


  /*  
   *   Setup this function to only write out information that matters. SHADDAI
   */

  for (i = 0; i < MAX_STANCE; i++)
    {
      fprintf (fp, "StartStance\t%s\n", get_stance_name (i));
      if (stance_index[i].num_attacks != 0)
	fprintf (fp, "Attacks\t%d\n", stance_index[i].num_attacks);
      if (stance_index[i].class_restrictions > 0)
	fprintf (fp, "Class\t%d\n", stance_index[i].class_restrictions);
      if (stance_index[i].dam_done > 0)
	fprintf (fp, "DamDone\t%d\n", stance_index[i].dam_done);
      if (stance_index[i].dam_taken > 0)
	fprintf (fp, "DamTaken\t%d\n", stance_index[i].dam_taken);
      if (stance_index[i].dodge != 0)
	fprintf (fp, "Dodge\t%d\n", stance_index[i].dodge);
      if (stance_index[i].dual_wield)
	fprintf (fp, "Dual\t%d\n", stance_index[i].dual_wield);
      if (stance_index[i].immune > 0)
	fprintf (fp, "Immune\t%d\n", stance_index[i].immune);
      if (stance_index[i].others)
	fprintf (fp, "Other\t%s~\n", stance_index[i].others);
      if (stance_index[i].parry != 0)
	fprintf (fp, "Parry\t%d\n", stance_index[i].parry);
      if (stance_index[i].special_percent > 0)
	fprintf (fp, "Percent\t%d\n", stance_index[i].special_percent);
      if (stance_index[i].race_restrictions > 0)
	fprintf (fp, "Race\t%d\n", stance_index[i].race_restrictions);
      if (stance_index[i].resist > 0)
	fprintf (fp, "Resist\t%d\n", stance_index[i].resist);
      if (stance_index[i].self)
	fprintf (fp, "Self\t%s~\n", stance_index[i].self);
      if (stance_index[i].special_move > 0)
	fprintf (fp, "Special\t%s\n",
		 (char *) get_special_name (stance_index[i].special_move));
      if (stance_index[i].stance[0] > 0)
	fprintf (fp, "Stance\t%s %s\n",
		 get_stance_name (stance_index[i].stance[0]),
		 get_stance_name (stance_index[i].stance[1]));
      if (stance_index[i].suscept > 0)
	fprintf (fp, "Suscept\t%d\n", stance_index[i].suscept);
      if (stance_index[i].wait > 0)
	fprintf (fp, "Wait\t%d\n", stance_index[i].wait);
      if (stance_index[i].max_weight > 0)
	fprintf (fp, "Weight\t%d\n", stance_index[i].max_weight);
      fprintf (fp, "EndStance\n\n");
    }
  fprintf (fp, "End\n" );
  fclose (fp);
  fpReserve = fopen (NULL_FILE, "r");
  return rNONE;
}

void
do_ststat (CHAR_DATA * ch, char *argument)
{
  int index_num = -1;
  char arg[MAX_INPUT_LENGTH];

  set_char_color (AT_PLAIN, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
  {
       send_to_char("STANCE list:\n\r", ch );
       for ( index_num = 0; index_num < MAX_STANCE; index_num++ ) {
         ch_printf( ch, "%s ", get_stance_name( index_num ) );
	 if ( index_num != 0 && (!index_num%4) )
	     send_to_char("\n\r", ch);
      }
      return;
  }
  if (arg[0] != '\'' && arg[0] != '"' && strlen (argument) > strlen (arg))
    strcpy (arg, argument);
  if ((index_num = get_stance_number (argument)) < 0 ||
      index_num >= MAX_STANCE)
    {
      send_to_char ("That stance does not exist.\n\r", ch);
      do_help (ch, "STANCEFLAGS");
      return;
    }
  ch_printf (ch, "Name :   %s\n\r", get_stance_name (index_num));
  ch_printf (ch, "Self :   %s\n\r", stance_index[index_num].self);
  ch_printf (ch, "Other:   %s\n\r", stance_index[index_num].others);
  ch_printf (ch, "Required Stances:   %s   ",
	     get_stance_name (stance_index[index_num].stance[0]));
  if (stance_index[index_num].stance[1] > 0)
    ch_printf (ch, "%s\n\r", get_stance_name (stance_index[index_num].stance[1]));
  else
    send_to_char ("\n\r", ch);
  ch_printf (ch, "Dual Wield allowed:  %s", stance_index[index_num].dual_wield
	     ? "No\n\r" : "Yes\n\r");
  ch_printf (ch, "Damage Done:   %3d   Damage Taken:   %3d\n\r",
       stance_index[index_num].dam_done, stance_index[index_num].dam_taken);
  ch_printf (ch, "Dodge      :   %3d   Parry       :   %3d\n\r",
	     stance_index[index_num].dodge, stance_index[index_num].parry);
  ch_printf (ch, "Attacks    :   %3d   Max Weight  :   %3d\n\r",
   stance_index[index_num].num_attacks, stance_index[index_num].max_weight);
  ch_printf (ch, "Wait	     :   %3d\n\r",
	     stance_index[index_num].wait);
  ch_printf (ch, "Resistance :   %s\n\r",
	     flag_string (stance_index[index_num].resist, ris_flags));
  ch_printf (ch, "Susceptible:   %s\n\r",
	     flag_string (stance_index[index_num].suscept, ris_flags));
  ch_printf (ch, "Immune     :   %s\n\r",
	     flag_string (stance_index[index_num].immune, ris_flags));
  ch_printf (ch, "Special Move:  %s    Chance:   %d\n\r",
	     get_special_name (stance_index[index_num].special_move),
	     stance_index[index_num].special_percent);
  ch_printf ( ch, "Class Restrictions :   %s\n\r", 
     	     class_string(stance_index[index_num].class_restrictions));	
  ch_printf ( ch, "Race Restrictions :    %s\n\r",
  	     race_string(stance_index[index_num].race_restrictions));
}

/*
 *  The main function to edit all the stances online.
 *  SHADDAI 
 */

void
do_stset (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  int index = -1;
  int value = -1;
  bool found = FALSE;

  if (IS_NPC (ch))
    {
      send_to_char ("Mob's can't mset\n\r", ch);
      return;
    }

  if (!ch->desc)
    {
      send_to_char ("You have no descriptor\n\r", ch);
      return;
    }

  set_char_color (AT_PLAIN, ch);
  smash_tilde (argument);

  argument = one_argument (argument, arg1);	/* Stances name */
  argument = one_argument (argument, arg2);	/* Field name to change */
  strcpy (arg3, argument);	/* Value */

  if (!str_cmp ("save", arg1))
    {
      fwrite_stance ();
      send_to_char ("Done.\n\r", ch);
      return;
    }

  if (arg1[0] == '\0' || arg2[0] == '\0' || !str_cmp (arg1, "?"))
    {
      send_to_char ("Syntax: stset <stance> <field>  <value>\n\r", ch);
      send_to_char ("Syntax: stset save\n\r", ch);
      send_to_char ("\n\r", ch);
      send_to_char ("Field being one of:\n\r", ch);
      send_to_char ("  attacks class damage dodge dual immune others\n\r", ch);
      send_to_char ("  parry percent protection race resist self\n\r", ch);
      send_to_char ("  special susceptible stance1 stance2 wait\n\r", ch);
      send_to_char ("  weight\n\r", ch);
      return;
    }

  if ((index = get_stance_number (arg1)) < 0)
    {
      send_to_char ("No such stance name.\n\r", ch);
      do_help (ch, "STANCEFLAGS");
      return;
    }

  if (is_number (arg3))
    value = atoi (arg3);
  else
    value = -100;

  switch (UPPER (arg2[0]))
    {
    default:
      do_stset (ch, "?");
      return;
    case 'A':
      if (!str_cmp ("attacks", arg2))
	{
	  found = TRUE;
	  if (value < -5 || value > 5)
	    {
	      send_to_char ("Attacks to be added are between -5 and 5.\n\r", ch);
	      return;
	    }
	  stance_index[index].num_attacks = value;
	}
      break;
    case 'C':
      if (!str_cmp ("class", arg2))
	{
	  found = TRUE;
	}
      break;
    case 'D':
      if (!str_cmp ("damage", arg2))
	{
	  found = TRUE;
	  if (value < 0 || value > 200)
	    {
	      send_to_char ("Damage value is between 0 and 200.\n\r", ch);
	      return;
	    }
	  stance_index[index].dam_done = value;
	}
      else if (!str_cmp ("dodge", arg2))
	{
	  found = TRUE;
	  if (value < -50 || value > 50)
	    {
	      send_to_char ("Dodge value is between -50 and 50.\n\r", ch);
	      return;
	    }
	  stance_index[index].dodge = value;
	}
      else if (!str_cmp ("dual", arg2))
	{
	  found = TRUE;
	  if (value < 0 || value > 1)
	    {
	      send_to_char ("Specify 1 for can dual wield and 0 for can't.\n\r", ch);
	      return;
	    }
	  if (value)
	    stance_index[index].dual_wield = 0;
	  else
	    stance_index[index].dual_wield = 1;
	}
      break;
    case 'I':
      if (!str_cmp ("immune", arg2))
	{
	  found = TRUE;
	  value = get_risflag (arg3);
	  if (value < 0 || value > 31)
	    {
	      ch_printf (ch, "Unkown flag: %s\n\r", arg3);
	      return;
	    }
	  TOGGLE_BIT (stance_index[index].immune, 1 << value);
	}
      break;
    case 'O':
      if (!str_cmp ("others", arg2))
	{
	  found = TRUE;
	  DISPOSE (stance_index[index].others);
	  stance_index[index].others = str_dup (arg3);
	}
      break;
    case 'P':
      if (!str_cmp ("parry", arg2))
	{
	  found = TRUE;
	  if (value < -50 || value > 50)
	    {
	      send_to_char ("Parry value is between -50 and 50.\n\r", ch);
	      return;
	    }
	  stance_index[index].parry = value;
	}
      else if (!str_cmp ("percent", arg2))
	{
	  found = TRUE;
	  if (value < 0 || value > 100)
	    {
	      send_to_char ("Percentage is from 0 to 100.\n\r", ch);
	      return;
	    }
	  stance_index[index].special_percent = value;
	}
      else if (!str_cmp ("protection", arg2))
	{
	  found = TRUE;
	  if (value < 0 || value > 200)
	    {
	      send_to_char ("Protection value is from 0 to 200.\n\r", ch);
	      return;
	    }
	  stance_index[index].dam_taken = value;
	}
      break;
    case 'R':
      if (!str_cmp ("race", arg2))
	{
	  found = TRUE;
	}
      else if (!str_cmp ("resist", arg2))
	{
	  found = TRUE;
	  value = get_risflag (arg3);
	  if (value < 0 || value > 31)
	    {
	      ch_printf (ch, "Unkown flag: %s\n\r", arg3);
	      return;
	    }
	  TOGGLE_BIT (stance_index[index].resist, 1 << value);
	}
      break;
    case 'S':
      if (!str_cmp ("self", arg2))
	{
	  found = TRUE;
	  DISPOSE (stance_index[index].self);
	  stance_index[index].self = str_dup (arg3);
	}
      else if (!str_cmp ("special", arg2))
	{
	  found = TRUE;
	  if ((value = get_special_number (arg3)) < 0)
	    {
	      send_to_char ("Unknown special routine.\n\r", ch);
	      return;
	    }
	  stance_index[index].special_move = value;
	}
      else if (!str_cmp ("susceptible", arg2))
	{
	  found = TRUE;
	  value = get_risflag (arg3);
	  if (value < 0 || value > 31)
	    {
	      ch_printf (ch, "Unkown flag: %s\n\r", arg3);
	      return;
	    }
	  TOGGLE_BIT (stance_index[index].suscept, 1 << value);

	}
      else if (!str_cmp ("stance1", arg2) || !str_cmp ("stance2", arg2))
	{
	  found = TRUE;
	  value = get_stance_number (arg3);
	  if (value)
	    if (!str_cmp ("stance1", arg2))
	      stance_index[index].stance[0] = value;
	    else
	      stance_index[index].stance[1] = value;
	  else
	    {
	      send_to_char ("Invalid stance name.\n\r", ch);
	      do_help (ch, "STANCEFLAGS");
	      return;
	    }
	}
      break;
    case 'W':
      if (!str_cmp ("wait", arg2))
	{
	  found = TRUE;
	  if (value < 0 || value > 25)
	    {
	      send_to_char ("Wait is from 0 to 20.\n\r", ch);
	      return;
	    }
	  stance_index[index].wait = value;
	}
      else if (!str_cmp ("weight", arg2))
	{
	  found = TRUE;
	  if (value < 0 || value > 999)
	    {
	      send_to_char ("Weight is from 0 to 999.\n\r", ch);
	      return;
	    }
	  stance_index[index].max_weight = value;
	}
      break;
    }
  if (!found)
    {
      do_stset (ch, "?");
      return;
    }
  send_to_char ("Done.\n\r", ch);
  return;
}

int
get_special_number (char *name)
{
  return 0;
}
char *
get_special_name (int num)
{
  return "None";
}
