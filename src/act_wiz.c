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
     |                   -*- Wizard/God Command Module -*-                 |
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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "mud.h"
#ifdef WIN32
#include <io.h>
#define F_OK 0
#endif


#define RESTORE_INTERVAL 21600


char *const save_flag[] =
  { "death", "kill", "passwd", "drop", "put", "give", "auto", "zap",
  "auction", "get", "receive", "idle", "backup", "quitbackup", "fill",
  "empty", "tmpsave", "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24",
  "r25", "r26", "r27", "r28", "r29", "r30", "r31"
};


/* from reset.c */
int generate_itemlevel args ((AREA_DATA * pArea, OBJ_INDEX_DATA * pObjIndex));

/* from comm.c */
bool write_to_descriptor args ((int desc, char *txt, int length));
bool check_parse_name args ((char *name, bool newchar));

/* from boards.c */
void note_attach (CHAR_DATA * ch);

/* from build.c */
int get_risflag (char *flag);
int get_defenseflag (char *flag);
int get_attackflag (char *flag);

/* from tables.c */
void write_race_file (int ra);

/* from clans.c */
void remove_member args ((char *clanname, char *membername));


/*
 * Local functions.
 */
ROOM_INDEX_DATA *find_location args ((CHAR_DATA * ch, char *arg));
void save_watchlist args ((void));
void save_banlist args ((void));
void close_area args ((AREA_DATA * pArea));

int get_color (char *argument);	/* function proto */

void sort_reserved args ((RESERVE_DATA * pRes));
void sort_noauctions args ((NOAUCTION_DATA * pNoauc));


PROJECT_DATA *get_project_by_number args ((int pnum));
NOTE_DATA *get_log_by_number args ((PROJECT_DATA * pproject, int pnum));

/*
 * Global variables.
 */

char reboot_time[50];
time_t new_boot_time_t;
extern struct tm new_boot_struct;
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];

int
get_saveflag (char *name)
{
  int x;

  for (x = 0; x < sizeof (save_flag) / sizeof (save_flag[0]); x++)
    if (!str_cmp (name, save_flag[x]))
      return x;
  return -1;
}

/*
 * Toggle "Do Not Disturb" flag. Used to prevent lower level imms from
 * using commands like "trans" and "goto" on higher level imms.
 */
void
do_dnd (CHAR_DATA * ch, char *argument)
{
  if (!IS_NPC (ch) && ch->pcdata)
    if (IS_SET (ch->pcdata->flags, PCFLAG_DND))
      {
	REMOVE_BIT (ch->pcdata->flags, PCFLAG_DND);
	send_to_char ("Your 'do not disturb' flag is now off.\n\r", ch);
      }
    else
      {
	SET_BIT (ch->pcdata->flags, PCFLAG_DND);
	send_to_char ("Your 'do not disturb' flag is now on.\n\r", ch);
      }
  else
    send_to_char ("huh?\n\r", ch);
}

/*
 * The "watch" facility allows imms to specify the name of a player or
 * the name of a site to be watched. It is like "logging" a player except
 * the results are written to a file in the "watch" directory named with
 * the same name as the imm. The idea is to allow lower level imms to 
 * watch players or sites without having to have access to the log files.
 */
void
do_watch (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  WATCH_DATA *pw;

  if (IS_NPC (ch))
    return;

  argument = one_argument (argument, arg);
  set_pager_color (AT_IMMORT, ch);

  if (arg[0] == '\0' || !str_cmp (arg, "help"))
    {
      send_to_pager ("Syntax Examples:\n\r", ch);
      /*
       * Only IMP+ can see all the watches. The rest can just see their own.
       */
      if (get_trust (ch) >= LEVEL_IMPLEMENTOR)
	send_to_pager ("   watch show all          show all watches\n\r", ch);
      send_to_pager ("   watch show              show all my watches\n\r"
		     "   watch size              show the size of my watch file\n\r"
		     "   watch player joe        add a new player watch\n\r"
		     "   watch site 2.3.123      add a new site watch\n\r"
		     "   watch command make      add a new command watch\n\r"
		     "   watch site 2.3.12       matches 2.3.12x\n\r"
		     "   watch site 2.3.12.      matches 2.3.12.x\n\r"
		     "   watch delete n          delete my nth watch\n\r"
		     "   watch print 500         print watch file starting at line 500\n\r"
		     "   watch print 500 1000    print 1000 lines starting at line 500\n\r"
		     "   watch clear             clear my watch file\n\r",
		     ch);
      return;
    }

  set_pager_color (AT_PLAIN, ch);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);

  /*
   * Clear watch file
   */
  if (!str_cmp (arg, "clear"))
    {
      char fname[MAX_INPUT_LENGTH];

      sprintf (fname, "%s%s", WATCH_DIR, strlower (ch->name));
      if (!remove (fname))
	{
	  send_to_pager ("Ok. Your watch file has been cleared.\n\r", ch);
	  return;
	}
      send_to_pager ("You have no valid watch file to clear.\n\r", ch);
      return;
    }

  /*
   * Display size of watch file
   */
  if (!str_cmp (arg, "size"))
    {
      FILE *fp;
      char fname[MAX_INPUT_LENGTH], s[MAX_STRING_LENGTH];
      int rec_count = 0;

      sprintf (fname, "%s%s", WATCH_DIR, strlower (ch->name));
      if (!(fp = fopen (fname, "r")))
	{
	  send_to_pager
	    ("You have no watch file. Perhaps you cleared it?\n\r", ch);
	  return;
	}

      fgets (s, MAX_STRING_LENGTH, fp);
      while (!feof (fp))
	{
	  rec_count++;
	  fgets (s, MAX_STRING_LENGTH, fp);
	}
      pager_printf (ch, "You have %d lines in your watch file.\n\r",
		    rec_count);
      fclose (fp);
      return;
    }

  /*
   * Print watch file
   */
  if (!str_cmp (arg, "print"))
    {
      FILE *fp;
      char fname[MAX_INPUT_LENGTH], s[MAX_STRING_LENGTH];
      const int MAX_DISPLAY_LINES = 1000;
      int start, limit, disp_count = 0, rec_count = 0;

      if (arg2[0] == '\0')
	{
	  send_to_pager
	    ("Sorry. You must specify a starting line number.\n\r", ch);
	  return;
	}

      start = atoi (arg2);
      limit = (arg3[0] == '\0') ? MAX_DISPLAY_LINES : atoi (arg3);
      limit = UMIN (limit, MAX_DISPLAY_LINES);

      sprintf (fname, "%s%s", WATCH_DIR, strlower (ch->name));
      if (!(fp = fopen (fname, "r")))
	return;
      fgets (s, MAX_STRING_LENGTH, fp);

      while ((disp_count < limit) && (!feof (fp)))
	{
	  if (++rec_count >= start)
	    {
	      send_to_pager (s, ch);
	      disp_count++;
	    }
	  fgets (s, MAX_STRING_LENGTH, fp);
	}
      send_to_pager ("\n\r", ch);
      if (disp_count >= MAX_DISPLAY_LINES)
	send_to_pager
	  ("Maximum display lines exceeded. List is terminated.\n\r"
	   "Type 'help watch' to see how to print the rest of the list.\n\r"
	   "\n\r"
	   "Your watch file is large. Perhaps you should clear it?\n\r", ch);

      fclose (fp);
      return;
    }

  /*
   * Display all watches
   * Only IMP+ can see all the watches. The rest can just see their own.
   */
  if (get_trust (ch) >= LEVEL_IMPLEMENTOR
      && !str_cmp (arg, "show") && !str_cmp (arg2, "all"))
    {
      pager_printf (ch, "%-12s %-14s %-15s\n\r",
		    "Imm Name", "Player/Command", "Player Site");
      if (first_watch)
	for (pw = first_watch; pw; pw = pw->next)
	  if (get_trust (ch) >= pw->imm_level)
	    pager_printf (ch, "%-14s %-12s %-15s\n\r",
			  pw->imm_name,
			  pw->target_name ? pw->target_name : " ",
			  pw->player_site ? pw->player_site : " ");
      return;
    }

  /*
   * Display only those watches belonging to the requesting imm 
   */
  if (!str_cmp (arg, "show") && arg2[0] == '\0')
    {
      int cou = 0;
      pager_printf (ch, "%-3s %-12s %-14s %-15s\n\r",
		    " ", "Imm Name", "Player/Command", "Player Site");
      if (first_watch)
	for (pw = first_watch; pw; pw = pw->next)
	  if (!str_cmp (ch->name, pw->imm_name))
	    pager_printf (ch, "%3d %-12s %-14s %-15s\n\r",
			  ++cou, pw->imm_name,
			  pw->target_name ? pw->target_name : " ",
			  pw->player_site ? pw->player_site : " ");
      return;
    }

  /*
   * Delete a watch belonging to the requesting imm
   */
  if (!str_cmp (arg, "delete") && isdigit (*arg2))
    {
      int cou = 0;
      int num;

      num = atoi (arg2);
      if (first_watch)
	for (pw = first_watch; pw; pw = pw->next)
	  if (!str_cmp (ch->name, pw->imm_name))
	    if (num == ++cou)
	      {
		/* Oops someone forgot to clear up the memory --Shaddai */
		if (pw->imm_name)
		  DISPOSE (pw->imm_name);
		if (pw->player_site)
		  DISPOSE (pw->player_site);
		if (pw->target_name)
		  DISPOSE (pw->target_name);
		/* Now we can unlink and then clear up that final
		 * pointer -- Shaddai 
		 */
		UNLINK (pw, first_watch, last_watch, next, prev);
		DISPOSE (pw);
		save_watchlist ();
		send_to_pager ("Deleted.\n\r", ch);
		return;
	      }
      send_to_pager ("Sorry. I found nothing to delete.\n\r", ch);
      return;
    }

  /*
   * Watch a specific player
   */
  if (!str_cmp (arg, "player") && *arg2)
    {
      WATCH_DATA *pinsert;
      CHAR_DATA *vic;
      char buf[MAX_INPUT_LENGTH];

      if (first_watch)		/* check for dups */
	for (pw = first_watch; pw; pw = pw->next)
	  if (!str_cmp (ch->name, pw->imm_name)
	      && pw->target_name && !str_cmp (arg2, pw->target_name))
	    {
	      send_to_pager ("You are already watching that player.\n\r", ch);
	      return;
	    }

      CREATE (pinsert, WATCH_DATA, 1);	/* create new watch */
      pinsert->imm_level = get_trust (ch);
      pinsert->imm_name = str_dup (strlower (ch->name));
      pinsert->target_name = str_dup (strlower (arg2));
      pinsert->player_site = NULL;

      /* stupid get_char_world returns ptr to "samantha" when given "sam" */
      /* so I do a str_cmp to make sure it finds the right player --Gorog */

      sprintf (buf, "0.%s", arg2);
      if ((vic = get_char_world (ch, buf)))	/* if vic is in game now */
	if ((!IS_NPC (vic)) && !str_cmp (arg2, vic->name))
	  SET_BIT (vic->pcdata->flags, PCFLAG_WATCH);

      if (first_watch)		/* ins new watch if app */
	for (pw = first_watch; pw; pw = pw->next)
	  if (strcmp (pinsert->imm_name, pw->imm_name) < 0)
	    {
	      INSERT (pinsert, pw, first_watch, next, prev);
	      save_watchlist ();
	      send_to_pager ("Ok. That player will be watched.\n\r", ch);
	      return;
	    }

      LINK (pinsert, first_watch, last_watch, next, prev);	/* link new watch */
      save_watchlist ();
      send_to_pager ("Ok. That player will be watched.\n\r", ch);
      return;
    }

  /*
   * Watch a specific site
   */
  if (!str_cmp (arg, "site") && *arg2)
    {
      WATCH_DATA *pinsert;
      CHAR_DATA *vic;

      if (first_watch)		/* check for dups */
	for (pw = first_watch; pw; pw = pw->next)
	  if (!str_cmp (ch->name, pw->imm_name)
	      && pw->player_site && !str_cmp (arg2, pw->player_site))
	    {
	      send_to_pager ("You are already watching that site.\n\r", ch);
	      return;
	    }

      CREATE (pinsert, WATCH_DATA, 1);	/* create new watch */
      pinsert->imm_level = get_trust (ch);
      pinsert->imm_name = str_dup (strlower (ch->name));
      pinsert->player_site = str_dup (strlower (arg2));
      pinsert->target_name = NULL;

      for (vic = first_char; vic; vic = vic->next)
	if (!IS_NPC (vic) && vic->desc && *pinsert->player_site
	    && !str_prefix (pinsert->player_site, vic->desc->host)
	    && get_trust (vic) < pinsert->imm_level)
	  SET_BIT (vic->pcdata->flags, PCFLAG_WATCH);

      if (first_watch)		/* ins new watch if app */
	for (pw = first_watch; pw; pw = pw->next)
	  if (strcmp (pinsert->imm_name, pw->imm_name) < 0)
	    {
	      INSERT (pinsert, pw, first_watch, next, prev);
	      save_watchlist ();
	      send_to_pager ("Ok. That site will be watched.\n\r", ch);
	      return;
	    }

      LINK (pinsert, first_watch, last_watch, next, prev);
      save_watchlist ();
      send_to_pager ("Ok. That site will be watched.\n\r", ch);
      return;
    }

  /*
   * Watch a specific command - FB
   */
  if (!str_cmp (arg, "command") && *arg2)
    {
      WATCH_DATA *pinsert;
      CMDTYPE *cmd;
      bool found = FALSE;

      for (pw = first_watch; pw; pw = pw->next)
	{
	  if (!str_cmp (ch->name, pw->imm_name)
	      && pw->target_name && !str_cmp (arg2, pw->target_name))
	    {
	      send_to_pager ("You are already watching that command.\n\r",
			     ch);
	      return;
	    }
	}

      for (cmd = command_hash[LOWER (arg2[0]) % 126]; cmd; cmd = cmd->next)
	{
	  if (!strcmp (arg2, cmd->name))
	    {
	      found = TRUE;
	      break;
	    }
	}

      if (!found)
	{
	  send_to_pager ("No such command exists.\n\r", ch);
	  return;
	}
      else
	{
	  SET_BIT (cmd->flags, CMD_WATCH);
	}

      CREATE (pinsert, WATCH_DATA, 1);
      pinsert->imm_level = get_trust (ch);
      pinsert->imm_name = str_dup (strlower (ch->name));
      pinsert->player_site = NULL;
      pinsert->target_name = str_dup (arg2);

      for (pw = first_watch; pw; pw = pw->next)
	{
	  if (strcmp (pinsert->imm_name, pw->imm_name) < 0)
	    {
	      INSERT (pinsert, pw, first_watch, next, prev);
	      save_watchlist ();
	      send_to_pager ("Ok, That command will be watched.\n\r", ch);
	      return;
	    }
	}

      LINK (pinsert, first_watch, last_watch, next, prev);
      save_watchlist ();
      send_to_pager ("Ok. That site will be watched.\n\r", ch);
      return;
    }

  send_to_pager ("Sorry. I can't do anything with that. "
		 "Please read the help file.\n\r", ch);
  return;
}

void
do_setvault (CHAR_DATA * ch, char *argument)
{
  VAULT_DATA *vault;
  int rnum;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (!str_cmp (arg1, "show"))
    {
      ROOM_INDEX_DATA *room;

      pager_printf_color (ch, "&W%6s | %-40.40s | Area name\n\r", "VNUM",
			  "Room Name");
      for (vault = first_vault; vault; vault = vault->next)
	{
	  if ((room = get_room_index (vault->vnum)) == NULL)
	    continue;
	  pager_printf_color (ch, "&c%6d | &C%-40.40s | %s\n\r", vault->vnum,
			      room->name, room->area->filename);
	}
      return;
    }

  if (!str_cmp (arg1, "save") && get_trust (ch) > LEVEL_GREATER)
    {
      save_vault_list ();
      send_to_char ("Done.\n\r", ch);
      return;
    }

  if (is_number (arg1) && (arg2[0] != '\0'))
    {
      rnum = atoi (arg1);

      if (get_room_index (rnum) != NULL)
	{
	  if (!str_cmp (arg2, "create"))
	    {
	      CREATE (vault, VAULT_DATA, 1);
	      vault->vnum = rnum;
	      sort_vaults (vault);
	      ch_printf (ch, "Donation room created.\n\r");
	      return;
	    }
	  if (!str_cmp (arg2, "delete"))
	    {
	      for (vault = first_vault; vault; vault = vault->next)
		if (vault->vnum == rnum)
		  {
		    UNLINK (vault, first_vault, last_vault, next, prev);
		    DISPOSE (vault);
		    send_to_char ("Deleting that vnum...\n\r", ch);
		    return;
		  }
	    }
	  else
	    send_to_char ("Not currently a donation vnum.\n\r", ch);
	}
      else
	{
	  send_to_char ("Invalid vnum argument", ch);
	  return;
	}
    }

  set_char_color (AT_IMMORT, ch);
  ch_printf (ch, "Syntax:\n\r");
  ch_printf (ch,
	     "  setvault show - lists the rooms currently set to save donations\n\r");
  ch_printf (ch,
	     "  setvault <vnum> create - adds a vnum to the list of rooms to save\n\r");
  ch_printf (ch,
	     "  setvault <vnum> delete - removes a vnum from the list of rooms to save\n\r\n\r");
  if (get_trust (ch) > LEVEL_GREATER)
    ch_printf (ch, "  setvault save - saves the vault list\n\r");
  ch_printf (ch,
	     "    Remember, rooms set as storage on clans or councils will need to be\n\r");
  ch_printf (ch,
	     "    removed in both clan/council file and the vault list.\n\r");
  return;
}


/* Old Wizhelp
void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    CMDTYPE * cmd;
    int col, hash;

    col = 0;
    set_pager_color( AT_PLAIN, ch );
    for ( hash = 0; hash < 126; hash++ )
	for ( cmd = command_hash[hash]; cmd; cmd = cmd->next )
	    if ( cmd->level >= LEVEL_HERO
	    &&   cmd->level <= get_trust( ch ) )
	    {
		pager_printf( ch, "%-12s", cmd->name );
		if ( ++col % 6 == 0 )
		    send_to_pager( "\n\r", ch );
	    }

    if ( col % 6 != 0 )
	send_to_pager( "\n\r", ch );
    return;
}
*/

void
do_wizhelp (CHAR_DATA * ch, char *argument)
{
  CMDTYPE *cmd;
  int col, hash;
  int curr_lvl;
  col = 0;

  for (curr_lvl = LEVEL_AVATAR; curr_lvl <= get_trust (ch); curr_lvl++)
    {
      set_pager_color (AT_WHITE, ch);
      send_to_pager ("\n\r\n\r", ch);
      col = 1;
      pager_printf (ch, "[LEVEL %-2d]  \n\r", curr_lvl);
      set_pager_color (AT_GREEN, ch);
      for (hash = 0; hash < 126; hash++)
	for (cmd = command_hash[hash]; cmd; cmd = cmd->next)
	  if ((cmd->level == curr_lvl) && cmd->level <= get_trust (ch))
	    {
	      pager_printf (ch, "%-12s", cmd->name);
	      if (++col % 6 == 0)
		send_to_pager ("\n\r", ch);
	    }
    }
  if (col % 6 != 0)
    send_to_pager ("\n\r", ch);
  set_pager_color (AT_PLAIN, ch);
  return;
}


void
do_restrict (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  sh_int level, hash;
  CMDTYPE *cmd;
  bool found;

  found = FALSE;
  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Restrict which command?\n\r", ch);
      return;
    }

  argument = one_argument (argument, arg2);
  if (arg2[0] == '\0')
    level = get_trust (ch);
  else
    level = atoi (arg2);

  level = UMAX (UMIN (get_trust (ch), level), 0);

  hash = arg[0] % 126;
  for (cmd = command_hash[hash]; cmd; cmd = cmd->next)
    {
      if (!str_prefix (arg, cmd->name) && cmd->level <= get_trust (ch))
	{
	  found = TRUE;
	  break;
	}
    }

  if (found)
    {
      if (!str_prefix (arg2, "show"))
	{
	  sprintf (buf, "%s show", cmd->name);
	  do_cedit (ch, buf);
/*    		ch_printf( ch, "%s is at level %d.\n\r", cmd->name, cmd->level );*/
	  return;
	}
      cmd->level = level;
      ch_printf (ch, "You restrict %s to level %d\n\r", cmd->name, level);
      sprintf (buf, "%s restricting %s to level %d",
	       ch->name, cmd->name, level);
      log_string (buf);
    }
  else
    send_to_char ("You may not restrict that command.\n\r", ch);

  return;
}

/* 
 * Check if the name prefix uniquely identifies a char descriptor
 */
CHAR_DATA *
get_waiting_desc (CHAR_DATA * ch, char *name)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *ret_char = NULL;
  static unsigned int number_of_hits;

  number_of_hits = 0;
  for (d = first_descriptor; d; d = d->next)
    {
      if (d->character && (!str_prefix (name, d->character->name)) &&
	  IS_WAITING_FOR_AUTH (d->character))
	{
	  if (++number_of_hits > 1)
	    {
	      ch_printf (ch, "%s does not uniquely identify a char.\n\r",
			 name);
	      return NULL;
	    }
	  ret_char = d->character;	/* return current char on exit */
	}
    }
  if (number_of_hits == 1)
    return ret_char;
  else
    {
      send_to_char ("No one like that waiting for authorization.\n\r", ch);
      return NULL;
    }
}

/* 02-07-99  New auth messages --Mystaric */
void
do_authorize (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  DESCRIPTOR_DATA *d;

  set_char_color (AT_LOG, ch);

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (arg1[0] == '\0')
    {
      send_to_char
	("Usage:  authorize <player> <yes|name|immsim|mobsim|swear|plain|unpronu|no/deny>\n\r",
	 ch);
      send_to_char ("Pending authorizations:\n\r", ch);
      send_to_char (" Chosen Character Name\n\r", ch);
      send_to_char ("---------------------------------------------\n\r", ch);
      for (d = first_descriptor; d; d = d->next)
	if ((victim = d->character) != NULL && IS_WAITING_FOR_AUTH (victim))
	  ch_printf (ch, " %s@%s new %s %s (%s)...\n\r",
		     victim->name,
		     victim->desc->host,
		     race_table[victim->race]->race_name,
		     class_table[victim->class]->who_name,
		     IS_PKILL (victim) ? "Deadly" : "Peaceful");
      return;
    }

  victim = get_waiting_desc (ch, arg1);
  if (victim == NULL)
    return;

  set_char_color (AT_IMMORT, victim);
  if (arg2[0] == '\0' || !str_cmp (arg2, "accept") || !str_cmp (arg2, "yes"))
    {
      victim->pcdata->auth_state = 3;
      if (victim->pcdata->authed_by)
	STRFREE (victim->pcdata->authed_by);
      victim->pcdata->authed_by = QUICKLINK (ch->name);
      sprintf (buf, "%s: authorized", victim->name);
      to_channel (buf, CHANNEL_AUTH, "Auth", LEVEL_NEOPHYTE);

      ch_printf (ch, "You have authorized %s.\n\r", victim->name);

      /* Below sends a message to player when name is accepted - Brittany */
      ch_printf_color (victim,
		       "\n\r&GThe MUD Administrators have accepted the name %s.\n\r"
		       "You are authorized to enter the Realms at the end of "
		       "this area.\n\r", victim->name);
      return;
    }

  else if (!str_cmp (arg2, "immsim") || !str_cmp (arg2, "i"))
    {
      victim->pcdata->auth_state = 2;
      sprintf (buf, "%s: name denied - similar to Imm name", victim->name);
      to_channel (buf, CHANNEL_AUTH, "Auth", LEVEL_NEOPHYTE);

      send_to_char_color
	("&RThe name you have chosen is too similar to that of an Immortal.\n\r"
	 "We ask you to please choose another name using the 'name' command.\n\r"
	 " (Type HELP NAME or HELP NAMING)\n\r", victim);
      ch_printf (ch, "You requested %s change names.\n\r", victim->name);
      return;
    }

  else if (!str_cmp (arg2, "mobsim") || !str_cmp (arg2, "m"))
    {
      victim->pcdata->auth_state = 2;
      sprintf (buf, "%s: name denied - similar to mob name", victim->name);
      to_channel (buf, CHANNEL_AUTH, "Auth", LEVEL_NEOPHYTE);

      send_to_char_color
	("&RThe name you have chosen is similar to that of a particular\n\r"
	 "creature in the game, in the long run this could cause problems\n\r"
	 "and therefore we are unable to authorize it.  Please choose\n\r"
	 "another name using the 'name' command.\n\r"
	 " (Type HELP NAME or HELP NAMING)\n\r", victim);
      ch_printf (ch, "You requested %s change names.\n\r", victim->name);
      return;
    }

  else if (!str_cmp (arg2, "swear") || !str_cmp (arg2, "s"))
    {
      victim->pcdata->auth_state = 2;
      sprintf (buf, "%s: name denied - swear word", victim->name);
      to_channel (buf, CHANNEL_AUTH, "Auth", LEVEL_NEOPHYTE);

      send_to_char_color
	("&RWe will not authorize names containing profanity, in any language.\n\r"
	 "Please choose another name using the 'name' command.\n\r"
	 " (Type HELP NAME or HELP NAMING)\n\r", victim);
      ch_printf (ch, "You requested %s change names.\n\r", victim->name);
      return;
    }

  else if (!str_cmp (arg2, "plain") || !str_cmp (arg2, "p"))
    {
      victim->pcdata->auth_state = 2;
      sprintf (buf, "%s: name denied", victim->name);
      to_channel (buf, CHANNEL_AUTH, "Auth", LEVEL_NEOPHYTE);

      send_to_char_color
	("&RWe would ask you to please attempt to choose a name that is more\n\r"
	 "medieval in nature.  Please choose another name using the 'name'\n\r"
	 "command.\n\r" " (Type HELP NAME or HELP NAMING)\n\r", victim);
      ch_printf (ch, "You requested %s change names.\n\r", victim->name);
      return;
    }

  else if (!str_cmp (arg2, "unprou") || !str_cmp (arg2, "u"))
    {
      victim->pcdata->auth_state = 2;
      sprintf (buf, "%s: name denied - unpronouncable", victim->name);
      to_channel (buf, CHANNEL_AUTH, "Auth", LEVEL_NEOPHYTE);

      send_to_char_color ("&RThe name you have chosen is unpronouncable.\n\r"
			  "Please choose another name using the 'name'\n\r"
			  "command.\n\r"
			  " (Type HELP NAME or HELP NAMING)", victim);
      ch_printf (ch, "You requested %s change names.\n\r", victim->name);
      return;
    }

  else if (!str_cmp (arg2, "no") || !str_cmp (arg2, "deny"))
    {
      send_to_char_color
	("&RThe name you have chosen and/or the actions you have taken have\n\r"
	 "been deemed grossly unacceptable to the administration of this MUD.\n\r"
	 "We ask you to discontinue such behaviour, or suffer possible banishment\n\r"
	 "from the game.\n\r", victim);
      sprintf (buf, "%s: denied authorization", victim->name);
      to_channel (buf, CHANNEL_AUTH, "Auth", LEVEL_NEOPHYTE);
      ch_printf (ch, "You have denied %s.\n\r", victim->name);
      if (victim->fighting)
	stop_fighting (victim, TRUE);
      do_quit (victim, "");
    }

  else if (!str_cmp (arg2, "name") || !str_cmp (arg2, "n"))
    {
      victim->pcdata->auth_state = 2;
      sprintf (buf, "%s: name denied", victim->name);
      to_channel (buf, CHANNEL_AUTH, "Auth", LEVEL_NEOPHYTE);
      ch_printf_color (victim,
		       "&R\n\rThe MUD Administrators have found the name %s "
		       "to be unacceptable.\n\r"
		       "The name you choose must be medieval and original.\n\r"
		       " (Type HELP NAME or HELP NAMING)\n\r", victim->name);
      ch_printf (ch, "You requested %s change names.\n\r", victim->name);
      return;
    }

  else
    {
      send_to_char ("Invalid argument.\n\r", ch);
      return;
    }
}

void
do_bamfin (CHAR_DATA * ch, char *argument)
{
  if (!IS_NPC (ch))
    {
      smash_tilde (argument);
      DISPOSE (ch->pcdata->bamfin);
      ch->pcdata->bamfin = str_dup (argument);
      send_to_char_color ("&YBamfin set.\n\r", ch);
    }
  return;
}

void
do_bamfout (CHAR_DATA * ch, char *argument)
{
  if (!IS_NPC (ch))
    {
      smash_tilde (argument);
      DISPOSE (ch->pcdata->bamfout);
      ch->pcdata->bamfout = str_dup (argument);
      send_to_char_color ("&YBamfout set.\n\r", ch);
    }
  return;
}

void
do_rank (CHAR_DATA * ch, char *argument)
{

  set_char_color (AT_IMMORT, ch);

  if (IS_NPC (ch))
    return;
  if (!argument || argument[0] == '\0')
    {
      send_to_char ("Usage:  rank <string>.\n\r", ch);
      send_to_char ("   or:  rank none.\n\r", ch);
      return;
    }
  smash_tilde (argument);
  DISPOSE (ch->pcdata->rank);
  if (!str_cmp (argument, "none"))
    ch->pcdata->rank = str_dup ("");
  else
    ch->pcdata->rank = str_dup (argument);
  send_to_char ("Ok.\n\r", ch);
  return;
}

void
do_retire (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Retire whom?\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }
  if (get_trust (victim) >= get_trust (ch))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  if (!IS_RETIRED (victim) && victim->level < LEVEL_SAVIOR)
    {
      send_to_char ("The minimum level for retirement is savior.\n\r", ch);
      return;
    }
  if (IS_RETIRED (victim))
    {
      REMOVE_BIT (victim->pcdata->flags, PCFLAG_RETIRED);
      ch_printf (ch, "%s returns from retirement.\n\r", victim->name);
      ch_printf (victim, "%s brings you back from retirement.\n\r", ch->name);
    }
  else
    {
      do_mortalize (ch, victim->name);
      SET_BIT (victim->pcdata->flags, PCFLAG_RETIRED);
      ch_printf (ch, "%s is now a retired immortal.\n\r", victim->name);
      ch_printf (victim,
		 "Courtesy of %s, you are now a retired immortal.\n\r",
		 ch->name);
    }
  return;
}

void
do_delay (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int delay;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg);
  if (!*arg)
    {
      send_to_char ("Syntax:  delay <victim> <# of rounds>\n\r", ch);
      return;
    }
  if (!(victim = get_char_world (ch, arg)))
    {
      send_to_char ("No such character online.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Mobiles are unaffected by lag.\n\r", ch);
      return;
    }
  if (!IS_NPC (victim) && get_trust (victim) >= get_trust (ch))
    {
      send_to_char ("You haven't the power to succeed against them.\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg);
  if (!*arg)
    {
      send_to_char ("For how long do you wish to delay them?\n\r", ch);
      return;
    }
  if (!str_cmp (arg, "none"))
    {
      send_to_char ("All character delay removed.\n\r", ch);
      victim->wait = 0;
      return;
    }
  delay = atoi (arg);
  if (delay < 1)
    {
      send_to_char ("Pointless.  Try a positive number.\n\r", ch);
      return;
    }
  if (delay > 999)
    {
      send_to_char ("You cruel bastard.  Just kill them.\n\r", ch);
      return;
    }
  WAIT_STATE (victim, delay * PULSE_VIOLENCE);
  ch_printf (ch, "You've delayed %s for %d rounds.\n\r", victim->name, delay);
  return;
}

void
do_deny (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Deny whom?\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }
  if (get_trust (victim) >= get_trust (ch))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  xSET_BIT (victim->act, PLR_DENY);
  set_char_color (AT_IMMORT, victim);
  send_to_char ("You are denied access!\n\r", victim);
  ch_printf (ch, "You have denied access to %s.\n\r", victim->name);
  if (victim->fighting)
    stop_fighting (victim, TRUE);	/* Blodkai, 97 */
  do_quit (victim, "");
  act (AT_BLOOD, "$n denies access to $N.", ch, NULL, victim, TO_ROOM);
  return;
}

void
do_disconnect (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Disconnect whom?\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (victim->desc == NULL)
    {
      act (AT_PLAIN, "$N doesn't have a descriptor.", ch, NULL, victim,
	   TO_CHAR);
      return;
    }
  if (get_trust (ch) <= get_trust (victim))
    {
      send_to_char ("They might not like that...\n\r", ch);
      return;
    }

  for (d = first_descriptor; d; d = d->next)
    {
      if (d == victim->desc)
	{
	  close_socket (d, FALSE);
	  send_to_char ("Ok.\n\r", ch);
	  return;
	}
    }
  bug ("Do_disconnect: *** desc not found ***.", 0);
  send_to_char ("Descriptor not found!\n\r", ch);
  return;
}


/*
 * Force a level one player to quit.             Gorog
 */
void
do_fquit (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char arg1[MAX_INPUT_LENGTH];

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg1);
  if (arg1[0] == '\0')
    {
      send_to_char ("Force whom to quit?\n\r", ch);
      return;
    }
  if (!(victim = get_char_world (ch, arg1)))
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (victim->level != 1)
    {
      send_to_char ("They are not level one!\n\r", ch);
      return;
    }
  set_char_color (AT_IMMORT, victim);
  send_to_char ("The MUD administrators force you to quit...\n\r", victim);
  if (victim->fighting)
    stop_fighting (victim, TRUE);
  do_quit (victim, "");
  ch_printf (ch, "You have forced %s to quit.\n\r", victim->name);
  return;
}

void
do_forceclose (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d;
  int desc;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Usage: forceclose <descriptor#>\n\r", ch);
      return;
    }

  desc = atoi (arg);
  for (d = first_descriptor; d; d = d->next)
    {
      if (d->descriptor == desc)
	{
	  if (d->character && get_trust (d->character) >= get_trust (ch))
	    {
	      send_to_char ("They might not like that...\n\r", ch);
	      return;
	    }
	  close_socket (d, FALSE);
	  send_to_char ("Ok.\n\r", ch);
	  return;
	}
    }
  send_to_char ("Not found!\n\r", ch);
  return;
}

void
do_pardon (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (arg1[0] == '\0' || arg2[0] == '\0')
    {
      send_to_char ("Syntax: pardon <character> <killer|thief|attacker>.\n\r",
		    ch);
      return;
    }
  if ((victim = get_char_world (ch, arg1)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }

  if (!str_cmp (arg2, "attacker"))
    {
      if (xIS_SET (victim->act, PLR_ATTACKER))
	{
	  xREMOVE_BIT (victim->act, PLR_ATTACKER);
	  ch_printf (ch, "Attacker flag removed from %s.\n\r", victim->name);
	  set_char_color (AT_IMMORT, victim);
	  send_to_char ("You are no longer an ATTACKER.\n\r", victim);
	}
      return;
    }
  if (!str_cmp (arg2, "killer"))
    {
      if (xIS_SET (victim->act, PLR_KILLER))
	{
	  xREMOVE_BIT (victim->act, PLR_KILLER);
	  ch_printf (ch, "Killer flag removed from %s.\n\r", victim->name);
	  set_char_color (AT_IMMORT, victim);
	  send_to_char ("You are no longer a KILLER.\n\r", victim);
	}
      return;
    }
  if (!str_cmp (arg2, "thief"))
    {
      if (xIS_SET (victim->act, PLR_THIEF))
	{
	  xREMOVE_BIT (victim->act, PLR_THIEF);
	  ch_printf (ch, "Thief flag removed from %s.\n\r", victim->name);
	  set_char_color (AT_IMMORT, victim);
	  send_to_char ("You are no longer a THIEF.\n\r", victim);
	}
      return;
    }
  send_to_char ("Syntax: pardon <character> <killer|thief>.\n\r", ch);
  return;
}

void
echo_to_all (sh_int AT_COLOR, char *argument, sh_int tar)
{
  DESCRIPTOR_DATA *d;

  if (!argument || argument[0] == '\0')
    return;

  for (d = first_descriptor; d; d = d->next)
    {
      /* Added showing echoes to players who are editing, so they won't
         miss out on important info like upcoming reboots. --Narn */
      if (d->connected == CON_PLAYING || d->connected == CON_EDITING)
	{
	  /* This one is kinda useless except for switched.. */
	  if (tar == ECHOTAR_PC && IS_NPC (d->character))
	    continue;
	  else if (tar == ECHOTAR_IMM && !IS_IMMORTAL (d->character))
	    continue;
	  else if (tar == ECHOTAR_PK && !IS_PKILL (d->character))
	    continue;
	  set_char_color (AT_COLOR, d->character);
	  send_to_char (argument, d->character);
	  send_to_char ("\n\r", d->character);
	}
    }
  return;
}

void
do_ech (CHAR_DATA * ch, char *argument)
{
  send_to_char_color ("&YIf you want to echo something, use 'echo'.\n\r", ch);
  return;
}

void
do_aecho (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  sh_int color;

  if ((color = get_color (argument)))
    argument = one_argument (argument, arg);
  if (argument[0] == '\0')
    {
      send_to_char ("Aecho what?\n\r", ch);
      return;
    }
  for (vch = first_char; vch; vch = vch_next)
    {
      vch_next = vch->next;
      if (vch->in_room->area == ch->in_room->area)
	{
	  if (color)
	    set_char_color (color, vch),
	      send_to_char (argument, vch), send_to_char ("\n\r", vch);
	  else
	    set_char_color (AT_IMMORT, vch),
	      send_to_char (argument, vch), send_to_char ("\n\r", vch);
	}
    }
}

void
do_echo (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  sh_int color;
  int target;
  char *parg;

  set_char_color (AT_IMMORT, ch);

  if (xIS_SET (ch->act, PLR_NO_EMOTE))
    {
      send_to_char ("You can't do that right now.\n\r", ch);
      return;
    }
  if (argument[0] == '\0')
    {
      send_to_char ("Echo what?\n\r", ch);
      return;
    }

  if ((color = get_color (argument)))
    argument = one_argument (argument, arg);
  parg = argument;
  argument = one_argument (argument, arg);
  if (!str_cmp (arg, "PC") || !str_cmp (arg, "player"))
    target = ECHOTAR_PC;
  else if (!str_cmp (arg, "imm"))
    target = ECHOTAR_IMM;
  else
    {
      target = ECHOTAR_ALL;
      argument = parg;
    }
  if (!color && (color = get_color (argument)))
    argument = one_argument (argument, arg);
  if (!color)
    color = AT_IMMORT;
  one_argument (argument, arg);
  if (!str_cmp (arg, "Thoric")
      || !str_cmp (arg, "Blodkai")
      || !str_cmp (arg, "Kali") || !str_cmp (arg, "Nivek"))
    {
      ch_printf (ch, "I don't think %s would like that!\n\r", arg);
      return;
    }
  echo_to_all (color, argument, target);
}

void
echo_to_room (sh_int AT_COLOR, ROOM_INDEX_DATA * room, char *argument)
{
  CHAR_DATA *vic;

  for (vic = room->first_person; vic; vic = vic->next_in_room)
    {
      set_char_color (AT_COLOR, vic);
      send_to_char (argument, vic);
      send_to_char ("\n\r", vic);
    }
}

void
do_recho (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  sh_int color;

  set_char_color (AT_IMMORT, ch);

  if (xIS_SET (ch->act, PLR_NO_EMOTE))
    {
      send_to_char ("You can't do that right now.\n\r", ch);
      return;
    }
  if (argument[0] == '\0')
    {
      send_to_char ("Recho what?\n\r", ch);
      return;
    }

  one_argument (argument, arg);
  if (!str_cmp (arg, "Thoric")
      || !str_cmp (arg, "Circe")
      || !str_cmp (arg, "Haus")
      || !str_cmp (arg, "Scryn") || !str_cmp (arg, "Blodkai"))
    {
      ch_printf (ch, "I don't think %s would like that!\n\r", arg);
      return;
    }
  if ((color = get_color (argument)))
    {
      argument = one_argument (argument, arg);
      echo_to_room (color, ch->in_room, argument);
    }
  else
    echo_to_room (AT_IMMORT, ch->in_room, argument);
}

ROOM_INDEX_DATA *
find_location (CHAR_DATA * ch, char *arg)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;

  if (is_number (arg))
    return get_room_index (atoi (arg));

  if (!str_cmp (arg, "pk"))	/* "Goto pk", "at pk", etc */
    return get_room_index (last_pkroom);

  if ((victim = get_char_world (ch, arg)) != NULL)
    return victim->in_room;

  if ((obj = get_obj_world (ch, arg)) != NULL)
    return obj->in_room;

  return NULL;
}

void
do_transfer (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (arg1[0] == '\0')
    {
      send_to_char ("Transfer whom (and where)?\n\r", ch);
      return;
    }
  if (!str_cmp (arg1, "all") && get_trust (ch) >= LEVEL_GREATER)
    {
      for (d = first_descriptor; d; d = d->next)
	{
	  if (d->connected == CON_PLAYING
	      && d->character != ch
	      && d->character->in_room
	      && d->newstate != 2 && can_see (ch, d->character))
	    {
	      char buf[MAX_STRING_LENGTH];
	      sprintf (buf, "%s %s", d->character->name, arg2);
	      do_transfer (ch, buf);
	    }
	}
      return;
    }

  /*
   * Thanks to Grodyn for the optional location parameter.
   */

  if (arg2[0] == '\0')
    {
      location = ch->in_room;
    }
  else
    {
      if ((location = find_location (ch, arg2)) == NULL)
	{
	  send_to_char ("No such location.\n\r", ch);
	  return;
	}
      if (room_is_private (location)
	  && get_trust (ch) < sysdata.level_override_private)
	{
	  send_to_char ("That room is private right now.\n\r", ch);
	  return;
	}
    }
  if ((victim = get_char_world (ch, arg1)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (NOT_AUTHED (victim))
    {
      send_to_char ("They are not authorized yet!\n\r", ch);
      return;
    }
  if (!victim->in_room)
    {
      send_to_char ("They have no physical location!\n\r", ch);
      return;
    }
  /* modification to prevent a low level imm from transferring a */
  /* higher level imm with the DND flag on.  - Gorog             */
  if (!IS_NPC (victim) && get_trust (ch) < get_trust (victim)
      && victim->desc
      && (victim->desc->connected == CON_PLAYING
	  || victim->desc->connected == CON_EDITING)
      && IS_SET (victim->pcdata->flags, PCFLAG_DND))
    {
      ch_printf (ch,
		 "Sorry. %s does not wish to be disturbed.\n\r",
		 victim->name);
      ch_printf (victim,
		 "Your DND flag just foiled %s's transfer command.\n\r",
		 ch->name);
      return;
    }
  /* end of modification                                         */

  if (victim->fighting)
    stop_fighting (victim, TRUE);
  act (AT_MAGIC, "$n disappears in a cloud of swirling colors.", victim, NULL,
       NULL, TO_ROOM);
  victim->retran = victim->in_room->vnum;
  char_from_room (victim);
  char_to_room (victim, location);
  act (AT_MAGIC, "$n arrives from a puff of smoke.", victim, NULL, NULL,
       TO_ROOM);
  if (ch != victim)
    act (AT_IMMORT, "$n has transferred you.", ch, NULL, victim, TO_VICT);
  do_look (victim, "auto");
  if (!IS_IMMORTAL (victim) && !IS_NPC (victim)
      && !in_hard_range (victim, location->area))
    act (AT_DANGER,
	 "Warning:  this player's level is not within the area's level range.",
	 ch, NULL, NULL, TO_CHAR);
}

void
do_retran (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Retransfer whom?\n\r", ch);
      return;
    }
  if (!(victim = get_char_world (ch, arg)))
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  sprintf (buf, "'%s' %d", victim->name, victim->retran);
  do_transfer (ch, buf);
  return;
}

void
do_regoto (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];

  sprintf (buf, "%d", ch->regoto);
  do_goto (ch, buf);
  return;
}

/*  Added do_at and do_atobj to reduce lag associated with at
 *  --Shaddai
 */
void
do_at (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location = NULL;
  ROOM_INDEX_DATA *original;
  CHAR_DATA *wch = NULL, *victim;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg);
  if (arg[0] == '\0' || argument[0] == '\0')
    {
      send_to_char ("At where what?\n\r", ch);
      return;
    }
  if (is_number (arg))
    location = get_room_index (atoi (arg));
  else if (!str_cmp (arg, "pk"))
    location = get_room_index (last_pkroom);
  else if ((wch = get_char_world (ch, arg)) == NULL || wch->in_room == NULL)
    {
      send_to_char ("No such mobile or player in existance.\n\r", ch);
      return;
    }
  if (!location && wch)
    location = wch->in_room;

  if (!location)
    {
      send_to_char ("No such location exists.\n\r", ch);
      return;
    }

  /* The following mod is used to prevent players from using the */
  /* at command on a higher level immortal who has a DND flag    */
  if (wch && !IS_NPC (wch)
      && IS_SET (wch->pcdata->flags, PCFLAG_DND)
      && get_trust (ch) < get_trust (wch))
    {
      ch_printf (ch,
		 "Sorry. %s does not wish to be disturbed.\n\r", wch->name);
      ch_printf (wch,
		 "Your DND flag just foiled %s's at command.\n\r", ch->name);
      return;
    }
  /* End of modification  -- Gorog */


  if (room_is_private (location))
    {
      if (get_trust (ch) < LEVEL_GREATER)
	{
	  send_to_char ("That room is private right now.\n\r", ch);
	  return;
	}
      else
	send_to_char ("Overriding private flag!\n\r", ch);
    }

  if ((victim = room_is_dnd (ch, location)))
    {
      ch_printf (ch, "That room is \"do not disturb\" right now.\n\r");
      ch_printf (victim,
		 "Your DND flag just foiled %s's atmob command\n\r",
		 ch->name);
      return;
    }

  set_char_color (AT_PLAIN, ch);
  original = ch->in_room;
  char_from_room (ch);
  char_to_room (ch, location);
  interpret (ch, argument);

  if (!char_died (ch))
    {
      char_from_room (ch);
      char_to_room (ch, original);
    }
  return;
}

void
do_atobj (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  ROOM_INDEX_DATA *original;
  OBJ_DATA *obj;
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg);
  if (arg[0] == '\0' || argument[0] == '\0')
    {
      send_to_char ("At where what?\n\r", ch);
      return;
    }

  if ((obj = get_obj_world (ch, arg)) == NULL || !obj->in_room)
    {
      send_to_char ("No such object in existance.\n\r", ch);
      return;
    }
  location = obj->in_room;
  if (room_is_private (location))
    {
      if (get_trust (ch) < LEVEL_GREATER)
	{
	  send_to_char ("That room is private right now.\n\r", ch);
	  return;
	}
      else
	send_to_char ("Overriding private flag!\n\r", ch);
    }

  if ((victim = room_is_dnd (ch, location)))
    {
      ch_printf (ch, "That room is \"do not disturb\" right now.\n\r");
      ch_printf (victim,
		 "Your DND flag just foiled %s's atobj command\n\r",
		 ch->name);
      return;
    }

  set_char_color (AT_PLAIN, ch);
  original = ch->in_room;
  char_from_room (ch);
  char_to_room (ch, location);
  interpret (ch, argument);

  if (!char_died (ch))
    {
      char_from_room (ch);
      char_to_room (ch, original);
    }
  return;
}

void
do_rat (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  ROOM_INDEX_DATA *original;
  int Start, End, vnum;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0')
    {
      send_to_char ("Syntax: rat <start> <end> <command>\n\r", ch);
      return;
    }

  Start = atoi (arg1);
  End = atoi (arg2);
  if (Start < 1 || End < Start || Start > End || Start == End
      || End > MAX_VNUM)
    {
      send_to_char ("Invalid range.\n\r", ch);
      return;
    }
  if (!str_cmp (argument, "quit"))
    {
      send_to_char ("I don't think so!\n\r", ch);
      return;
    }

  original = ch->in_room;
  for (vnum = Start; vnum <= End; vnum++)
    {
      if ((location = get_room_index (vnum)) == NULL)
	continue;
      char_from_room (ch);
      char_to_room (ch, location);
      interpret (ch, argument);
    }

  char_from_room (ch);
  char_to_room (ch, original);
  send_to_char ("Done.\n\r", ch);
  return;
}

void
do_rstat (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char *sect;
  ROOM_INDEX_DATA *location;
  OBJ_DATA *obj;
  CHAR_DATA *rch;
  EXIT_DATA *pexit;
  AFFECT_DATA *paf;
  int cnt;
  static char *dir_text[] =
    { "n", "e", "s", "w", "u", "d", "ne", "nw", "se", "sw", "?" };

  one_argument (argument, arg);
  if (!str_cmp (arg, "ex") || !str_cmp (arg, "exits"))
    {
      location = ch->in_room;

      ch_printf_color (ch, "&cExits for room '&W%s&c'  Vnum &W%d\n\r",
		       location->name, location->vnum);
      for (cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next)
	ch_printf_color (ch,
			 "&W%2d) &w%2s to %-5d  &cKey: &w%d  &cFlags: &w%d  &cKeywords: '&w%s&c'\n\r     Exdesc: &w%s     &cBack link: &w%d  &cVnum: &w%d  &cDistance: &w%d  &cPulltype: &w%s  &cPull: &w%d\n\r",
			 ++cnt,
			 dir_text[pexit->vdir],
			 pexit->to_room ? pexit->to_room->vnum : 0,
			 pexit->key,
			 pexit->exit_info,
			 pexit->keyword,
			 pexit->description[0] != '\0'
			 ? pexit->description : "(none).\n\r",
			 pexit->rexit ? pexit->rexit->vnum : 0,
			 pexit->rvnum,
			 pexit->distance,
			 pull_type_name (pexit->pulltype), pexit->pull);
      return;
    }
  location = (arg[0] == '\0') ? ch->in_room : find_location (ch, arg);
  if (!location)
    {
      send_to_char ("No such location.\n\r", ch);
      return;
    }

  if (ch->in_room != location && room_is_private (location))
    {
      if (get_trust (ch) < LEVEL_GREATER)
	{
	  send_to_char ("That room is private right now.\n\r", ch);
	  return;
	}
      else
	send_to_char ("Overriding private flag!\n\r", ch);
    }

  ch_printf_color (ch, "&cName: &w%s\n\r&cArea: &w%s  &cFilename: &w%s\n\r",
		   location->name,
		   location->area ? location->area->name : "None????",
		   location->area ? location->area->filename : "None????");

  switch (ch->in_room->sector_type)
    {
    default:
      sect = "?!?!";
      break;
    case SECT_INSIDE:
      sect = "Inside";
      break;
    case SECT_CITY:
      sect = "City";
      break;
    case SECT_FIELD:
      sect = "Field";
      break;
    case SECT_FOREST:
      sect = "Forest";
      break;
    case SECT_HILLS:
      sect = "Hills";
      break;
    case SECT_MOUNTAIN:
      sect = "Mountains";
      break;
    case SECT_WATER_SWIM:
      sect = "Swim";
      break;
    case SECT_WATER_NOSWIM:
      sect = "Noswim";
      break;
    case SECT_UNDERWATER:
      sect = "Underwater";
      break;
    case SECT_AIR:
      sect = "Air";
      break;
    case SECT_DESERT:
      sect = "Desert";
      break;
    case SECT_OCEANFLOOR:
      sect = "Oceanfloor";
      break;
    case SECT_UNDERGROUND:
      sect = "Underground";
      break;
    case SECT_LAVA:
      sect = "Lava";
      break;
    case SECT_SWAMP:
      sect = "Swamp";
      break;
    }

  ch_printf_color (ch, "&cVnum: &w%d   &cSector: &w%d (%s)   &cLight: &w%d",
		   location->vnum,
		   location->sector_type, sect, location->light);
  if (location->tunnel > 0)
    ch_printf_color (ch, "   &cTunnel: &W%d", location->tunnel);
  send_to_char ("\n\r", ch);
  ch_printf_color (ch, "&cRoom Weight: &W%d   &cRoom Max Weight: &R%d\n\r",
		   location->weight, location->max_weight);
  if (location->tele_delay > 0 || location->tele_vnum > 0)
    ch_printf_color (ch, "&cTeleDelay: &R%d   &cTeleVnum: &R%d\n\r",
		     location->tele_delay, location->tele_vnum);
  ch_printf_color (ch, "&cRoom flags: &w%s\n\r",
		   ext_flag_string (&location->room_flags, r_flags));
  ch_printf_color (ch, "&cDescription:\n\r&w%s", location->description);
  if (location->first_extradesc)
    {
      EXTRA_DESCR_DATA *ed;

      send_to_char_color ("&cExtra description keywords: &w'", ch);
      for (ed = location->first_extradesc; ed; ed = ed->next)
	{
	  send_to_char (ed->keyword, ch);
	  if (ed->next)
	    send_to_char (", ", ch);
	}
      send_to_char ("'\n\r", ch);
    }
  for (paf = location->first_affect; paf; paf = paf->next)
    ch_printf_color (ch, "&cAffect: &w%s &cby &w%d.\n\r",
		     affect_loc_name (paf->location), paf->modifier);

  send_to_char_color ("&cCharacters: &w", ch);
  for (rch = location->first_person; rch; rch = rch->next_in_room)
    {
      if (can_see (ch, rch))
	{
	  send_to_char (" ", ch);
	  one_argument (rch->name, buf);
	  send_to_char (buf, ch);
	}
    }

  send_to_char_color ("\n\r&cObjects:    &w", ch);
  for (obj = location->first_content; obj; obj = obj->next_content)
    {
      send_to_char (" ", ch);
      one_argument (obj->name, buf);
      send_to_char (buf, ch);
    }
  send_to_char ("\n\r", ch);

  if (location->first_exit)
    send_to_char_color
      ("&c------------------- &wEXITS &c-------------------\n\r", ch);
  for (cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next)
    ch_printf (ch,
	       "%2d) %-2s to %-5d.  Key: %d  Flags: %d  Keywords: %s.\n\r",
	       ++cnt,
	       dir_text[pexit->vdir],
	       pexit->to_room ? pexit->to_room->vnum : 0,
	       pexit->key,
	       pexit->exit_info,
	       pexit->keyword[0] != '\0' ? pexit->keyword : "(none)");
  return;
}

/* Face-lift by Demora */
void
do_ostat (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  AFFECT_DATA *paf;
  OBJ_DATA *obj;

  set_char_color (AT_CYAN, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Ostat what?\n\r", ch);
      return;
    }
  if (arg[0] != '\'' && arg[0] != '"' && strlen (argument) > strlen (arg))
    strcpy (arg, argument);

  if ((obj = get_obj_world (ch, arg)) == NULL)
    {
      send_to_char ("Nothing like that in hell, earth, or heaven.\n\r", ch);
      return;
    }
  ch_printf_color (ch, "&cName: &C%s\n\r", obj->name);
  ch_printf_color (ch, "&cVnum: &w%d  ", obj->pIndexData->vnum);
  ch_printf_color (ch, "&cType: &w%s  ", item_type_name (obj));
  ch_printf_color (ch, "&cCount:  &w%d  ", obj->pIndexData->count);
  ch_printf_color (ch, "&cGcount: &w%d\n\r", obj->count);
  ch_printf_color (ch, "&cSerial#: &w%d  ", obj->serial);
  ch_printf_color (ch, "&cTopIdxSerial#: &w%d  ", obj->pIndexData->serial);
  ch_printf_color (ch, "&cTopSerial#: &w%d\n\r", cur_obj_serial);
  ch_printf_color (ch, "&cShort description: &C%s\n\r", obj->short_descr);
  ch_printf_color (ch, "&cLong description : &C%s\n\r", obj->description);
  if (obj->owner[0] != '\0')
    ch_printf_color (ch, "&cOwner: &Y%s\n\r", obj->owner);
  if (obj->action_desc[0] != '\0')
    ch_printf_color (ch, "&cAction description: &w%s\n\r", obj->action_desc);
  ch_printf_color (ch, "&cWear flags : &w%s\n\r",
		   flag_string (obj->wear_flags, w_flags));
  ch_printf_color (ch, "&cExtra flags: &w%s\n\r",
		   ext_flag_string (&obj->extra_flags, o_flags));
  ch_printf_color (ch, "&cMagic flags: &w%s\n\r",
		   magic_bit_name (obj->magic_flags));
  ch_printf_color (ch, "&cNumber: &w%d/%d   ", 1, get_obj_number (obj));
  ch_printf_color (ch, "&cWeight: &w%d/%d   ", obj->weight,
		   get_obj_weight (obj));
  ch_printf_color (ch, "&cLayers: &w%d   ", obj->pIndexData->layers);
  ch_printf_color (ch, "&cWear_loc: &w%d\n\r", obj->wear_loc);
  ch_printf_color (ch, "&cCost: &Y%d  ", obj->cost);
  ch_printf_color (ch, "&cRent: &w%d  ", obj->pIndexData->rent);
  send_to_char_color ("&cTimer: ", ch);
  if (obj->timer > 0)
    ch_printf_color (ch, "&R%d  ", obj->timer);
  else
    ch_printf_color (ch, "&w%d  ", obj->timer);
  ch_printf_color (ch, "&cLevel: &P%d    ", obj->level);
  ch_printf_color (ch, "&cIndex level: &P%d\n\r", obj->pIndexData->level);
  ch_printf_color (ch, "&cIn room: &w%d  ",
		   obj->in_room == NULL ? 0 : obj->in_room->vnum);
  ch_printf_color (ch, "&cIn object: &w%s  ",
		   obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr);
  ch_printf_color (ch, "&cCarried by: &C%s\n\r",
		   obj->carried_by ==
		   NULL ? "(none)" : obj->carried_by->name);
  ch_printf_color (ch, "&cIndex Values : &w%d %d %d %d %d %d.\n\r",
		   obj->pIndexData->value[0], obj->pIndexData->value[1],
		   obj->pIndexData->value[2], obj->pIndexData->value[3],
		   obj->pIndexData->value[4], obj->pIndexData->value[5]);
  ch_printf_color (ch, "&cObject Values: &w%d %d %d %d %d %d.\n\r",
		   obj->value[0], obj->value[1], obj->value[2], obj->value[3],
		   obj->value[4], obj->value[5]);
  if (obj->pIndexData->first_extradesc)
    {
      EXTRA_DESCR_DATA *ed;
      send_to_char ("Primary description keywords:   '", ch);
      for (ed = obj->pIndexData->first_extradesc; ed; ed = ed->next)
	{
	  send_to_char (ed->keyword, ch);
	  if (ed->next)
	    send_to_char (", ", ch);
	}
      send_to_char ("'\n\r", ch);
    }
  if (obj->first_extradesc)
    {
      EXTRA_DESCR_DATA *ed;
      send_to_char ("Secondary description keywords: '", ch);
      for (ed = obj->first_extradesc; ed; ed = ed->next)
	{
	  send_to_char (ed->keyword, ch);
	  if (ed->next)
	    send_to_char (", ", ch);
	}
      send_to_char ("'\n\r", ch);
    }
  for (paf = obj->first_affect; paf; paf = paf->next)
    ch_printf_color (ch, "&cAffects &w%s &cby &w%d. (extra)\n\r",
		     affect_loc_name (paf->location), paf->modifier);
  for (paf = obj->pIndexData->first_affect; paf; paf = paf->next)
    ch_printf_color (ch, "&cAffects &w%s &cby &w%d.\n\r",
		     affect_loc_name (paf->location), paf->modifier);
  return;
}

void
do_vstat (CHAR_DATA * ch, char *argument)
{
  VARIABLE_DATA *vd;
  CHAR_DATA *victim;

  if (argument[0] == '\0')
    {
      send_to_pager ("Vstat whom?\n\r", ch);
      return;
    }

  if ((victim = get_char_world (ch, argument)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (get_trust (ch) < get_trust (victim))
    {
      send_to_char
	("Their godly glow prevents you from getting a good look.\n\r", ch);
      return;
    }

  if (!victim->variables)
    {
      send_to_char ("They have no variables currently assigned to them.\n\r",
		    ch);
      return;
    }

  pager_printf_color (ch, "\n\r&cName: &C%-20s &cRoom : &w%-10d",
		      victim->name,
		      victim->in_room == NULL ? 0 : victim->in_room->vnum);
  pager_printf_color (ch, "\n\r&cVariables:\n\r");

/*
Variables:
Vnum:           Tag:                 Type:     Timer:
   Flags:
   Data:
*/
  for (vd = victim->variables; vd; vd = vd->next)
    {
      pager_printf_color (ch,
			  "  &cVnum: &W%-10d &cTag: &W%-15s &cTimer: &W%d\n\r",
			  vd->vnum, vd->tag, vd->timer);
      pager_printf_color (ch, "  &cType: ");
      if (vd->data)
	switch (vd->type)
	  {
	  case vtSTR:
	    if (vd->data)
	      pager_printf_color (ch, "&CString     &cData: &W%s", vd->data);
	    break;
	  case vtINT:
	    if (vd->data)
	      pager_printf_color (ch, "&CInteger    &cData: &W%d",
				  (int) vd->data);
	    break;
	  case vtXBIT:
	    if (vd->data)
	      {
		char buf[MAX_STRING_LENGTH];
		int started = 0;
		int x;

		buf[0] = '\0';
		for (x = MAX_BITS; x > 0; --x)
		  {
		    if (!started && xIS_SET (*(EXT_BV *) vd->data, x))
		      started = x;
		  }
		for (x = 1; x <= started; x++)
		  strcat (buf,
			  xIS_SET (*(EXT_BV *) vd->data, x) ? "1 " : "0 ");

		if (buf[0] != '\0')
		  buf[strlen (buf) - 1] = '\0';
		pager_printf_color (ch, "&CXBIT       &cData: &w[&W%s&w]",
				    buf);
	      }
	    break;
	  }
      else
	pager_printf_color (ch, "&CNo Data");

      send_to_pager ("\n\r\n\r", ch);
    }
  return;
}


void
do_mstat (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char hpbuf[MAX_STRING_LENGTH];
  char mnbuf[MAX_STRING_LENGTH];
  char mvbuf[MAX_STRING_LENGTH];
  char bdbuf[MAX_STRING_LENGTH];
  AFFECT_DATA *paf;
  VARIABLE_DATA *vd;
  CHAR_DATA *victim;
  SKILLTYPE *skill;
  int x;


  set_pager_color (AT_CYAN, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_pager ("Mstat whom?\n\r", ch);
      return;
    }
  if (arg[0] != '\'' && arg[0] != '"' && strlen (argument) > strlen (arg))
    strcpy (arg, argument);

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_pager ("They aren't here.\n\r", ch);
      return;
    }
  if (get_trust (ch) < get_trust (victim) && !IS_NPC (victim))
    {
      set_pager_color (AT_IMMORT, ch);
      send_to_pager
	("Their godly glow prevents you from getting a good look.\n\r", ch);
      return;
    }
  if (IS_NPC (victim) && get_trust (ch) < LEVEL_GREATER
      && xIS_SET (victim->act, ACT_STATSHIELD))
    {
      set_pager_color (AT_IMMORT, ch);
      send_to_pager
	("Their godly glow prevents you from getting a good look.\n\r", ch);
      return;
    }

  pager_printf_color (ch, "\n\r&c%s: &C%-20s",
		      IS_NPC (victim) ? "Mobile name" : "Name", victim->name);
  if (!IS_NPC (victim))
    pager_printf_color (ch, "&cStatus : &w%-10s",
			CAN_PKILL (victim) ? "Deadly" : IS_PKILL (victim) ?
			"Pre-Deadly" : "Non-Deadly");
  if (!IS_NPC (victim) && victim->pcdata->clan)
    pager_printf_color (ch, "   &c%s: &w%s",
			victim->pcdata->clan->clan_type ==
			CLAN_ORDER ? "Order" : victim->pcdata->
			clan->clan_type == CLAN_GUILD ? "Guild" : "Clan",
			victim->pcdata->clan->name);
  send_to_pager ("\n\r", ch);
  if (get_trust (ch) >= LEVEL_GOD && !IS_NPC (victim) && victim->desc)
    pager_printf_color (ch,
			"&cUser: &w%-19s Descriptor: %d  &cTrust: &w%d  &cAuthBy: &w%s\n\r",
			victim->desc->user, victim->desc->descriptor,
			victim->trust,
			victim->pcdata->authed_by[0] !=
			'\0' ? victim->pcdata->authed_by : "(unknown)");
  if (!IS_NPC (victim))
    {
      pager_printf_color (ch, "&cRecent IP: &w%-15s",
			  victim->pcdata->recent_site ? victim->
			  pcdata->recent_site : "Unknown");
      pager_printf_color (ch, "&cPrevious IP: &w%-15s",
			  victim->pcdata->prev_site ? victim->
			  pcdata->prev_site : "Unknown");
      pager_printf_color (ch, "&cRank: &w%s\n\r",
			  str_cmp (victim->pcdata->rank,
				   "") ? victim->pcdata->rank : "(default)");
    }
  if (!IS_NPC (victim) && victim->pcdata->release_date != 0)
    pager_printf_color (ch, "&cHelled until %24.24s by %s.\n\r",
			ctime (&victim->pcdata->release_date),
			victim->pcdata->helled_by);
  pager_printf_color (ch,
		      "&cVnum: &w%-5d    &cSex: &w%-6s    &cRoom: &w%-5d    &cCount: &w%d   &cKilled: &w%d\n\r",
		      IS_NPC (victim) ? victim->pIndexData->vnum : 0,
		      victim->sex == SEX_MALE ? "male" : victim->sex ==
		      SEX_FEMALE ? "female" : "neutral",
		      victim->in_room == NULL ? 0 : victim->in_room->vnum,
		      IS_NPC (victim) ? victim->pIndexData->count : 1,
		      IS_NPC (victim) ? victim->pIndexData->
		      killed : victim->pcdata->mdeaths +
		      victim->pcdata->pdeaths);
  pager_printf_color (ch,
		      "&cStr: &C%2d&c )( Int: &C%2d&c )( Wis: &C%2d&c )( Dex: &C%2d&c )( Con: &C%2d&c )( Cha: &C%2d&c )( Lck: &C%2d&c\n\r",
		      get_curr_str (victim), get_curr_int (victim),
		      get_curr_wis (victim), get_curr_dex (victim),
		      get_curr_con (victim), get_curr_cha (victim),
		      get_curr_lck (victim));
  pager_printf_color (ch, "&cLevel   : &P%-2d ", victim->level);
  if (IS_NPC (victim))
    pager_printf_color (ch, "&c(&w%-2.2d&c)         ",
			victim->pIndexData->level);
  else
    pager_printf_color (ch, "             ");
  pager_printf_color (ch,
		      "&cClass  : &w%-2.2d/%-10s   &cRace      : &w%-2.2d/%-10s\n\r",
		      victim->class,
		      IS_NPC (victim) ? victim->class < MAX_NPC_CLASS
		      && victim->class >=
		      0 ? npc_class[victim->
				    class] : "unknown" : victim->class <
		      MAX_PC_CLASS && class_table[victim->class]->who_name
		      && class_table[victim->class]->who_name[0] !=
		      '\0' ? class_table[victim->class]->who_name : "unknown",
		      victim->race,
		      IS_NPC (victim) ? victim->race < MAX_NPC_RACE
		      && victim->race >=
		      0 ? npc_race[victim->race] : "unknown" : victim->race <
		      MAX_PC_RACE && race_table[victim->race]->race_name
		      && race_table[victim->race]->race_name[0] !=
		      '\0' ? race_table[victim->race]->race_name : "unknown",
		      npc_race[victim->race], victim->position);
  sprintf (hpbuf, "%d/%d", victim->hit, victim->max_hit);
  sprintf (mnbuf, "%d/%d", victim->mana, victim->max_mana);
  sprintf (mvbuf, "%d/%d", victim->move, victim->max_move);
  if (IS_VAMPIRE (victim) && !IS_NPC (victim))
    {
      sprintf (bdbuf, "%d/%d", victim->pcdata->condition[COND_BLOODTHIRST],
	       10 + victim->level);
      pager_printf_color (ch,
			  "&cHps     : &w%-12s    &cBlood  : &w%-12s    &cMove      : &w%-12s\n\r",
			  hpbuf, bdbuf, mvbuf);
    }
  else
    pager_printf_color (ch,
			"&cHps     : &w%-12s    &cMana   : &w%-12s    &cMove      : &w%-12s\n\r",
			hpbuf, mnbuf, mvbuf);
  pager_printf_color (ch,
		      "&cHitroll : &C%-5d           &cAlign  : &w%-5d           &cArmorClass: &w%d\n\r",
		      GET_HITROLL (victim), victim->alignment,
		      GET_AC (victim));
  pager_printf_color (ch,
		      "&cDamroll : &C%-5d           &cWimpy  : &w%-5d           &cPosition  : &w%d\n\r",
		      GET_DAMROLL (victim), victim->wimpy, victim->position);
  pager_printf_color (ch,
		      "&cFighting: &w%-13s   &cMaster : &w%-13s   &cLeader    : &w%s\n\r",
		      victim->fighting ? victim->fighting->
		      who->name : "(none)",
		      victim->master ? victim->master->name : "(none)",
		      victim->leader ? victim->leader->name : "(none)");
  if (IS_NPC (victim))
    pager_printf_color (ch,
			"&cHating  : &w%-13s   &cHunting: &w%-13s   &cFearing   : &w%s\n\r",
			victim->hating ? victim->hating->name : "(none)",
			victim->hunting ? victim->hunting->name : "(none)",
			victim->fearing ? victim->fearing->name : "(none)");
  else
    pager_printf_color (ch,
			"&cDeity   : &w%-13s&w   &cFavor  : &w%-5d &cGlory : &w%-3d (%-3d)     &cHonour : &w%-3d\n\r",
			victim->pcdata->deity ? victim->pcdata->
			deity->name : "(none)", victim->pcdata->favor,
			victim->pcdata->quest_curr,
			victim->pcdata->quest_accum, victim->pcdata->honour);
  if (IS_NPC (victim))
    pager_printf_color (ch,
			"&cMob hitdie : &C%dd%d+%d    &cMob damdie : &C%dd%d+%d    &cNumAttacks : &C%d\n\r",
			victim->pIndexData->hitnodice,
			victim->pIndexData->hitsizedice,
			victim->pIndexData->hitplus,
			victim->pIndexData->damnodice,
			victim->pIndexData->damsizedice,
			victim->pIndexData->damplus, victim->numattacks);
  pager_printf_color (ch,
		      "&cMentalState: &w%-3d   &cEmotionalState: &w%-3d   ",
		      victim->mental_state, victim->emotional_state);
  if (!IS_NPC (victim))
    pager_printf_color (ch,
			"&cThirst: &w%d   &cFull: &w%d   &cDrunk: &w%d\n\r",
			victim->pcdata->condition[COND_THIRST],
			victim->pcdata->condition[COND_FULL],
			victim->pcdata->condition[COND_DRUNK]);
  else
    send_to_pager ("\n\r", ch);
  pager_printf_color (ch,
		      "&cSave versus: &w%d %d %d %d %d       &cItems: &w(%d/%d)  &cWeight &w(%d/%d)\n\r",
		      victim->saving_poison_death, victim->saving_wand,
		      victim->saving_para_petri, victim->saving_breath,
		      victim->saving_spell_staff, victim->carry_number,
		      can_carry_n (victim), victim->carry_weight,
		      can_carry_w (victim));
  pager_printf_color (ch,
		      "&cYear: &w%-5d  &cSecs: &w%d  &cTimer: &w%d  &cGold: &Y%d\n\r",
		      get_age (victim), (int) victim->played, victim->timer,
		      victim->gold);
  if (get_timer (victim, TIMER_PKILLED))
    pager_printf_color (ch, "&cTimerPkilled:  &R%d\n\r",
			get_timer (victim, TIMER_PKILLED));
  if (get_timer (victim, TIMER_RECENTFIGHT))
    pager_printf_color (ch, "&cTimerRecentfight:  &R%d\n\r",
			get_timer (victim, TIMER_RECENTFIGHT));
  if (get_timer (victim, TIMER_ASUPRESSED))
    pager_printf_color (ch, "&cTimerAsupressed:  &R%d\n\r",
			get_timer (victim, TIMER_ASUPRESSED));
  if (IS_NPC (victim))
    pager_printf_color (ch, "&cAct Flags  : &w%s\n\r",
			ext_flag_string (&victim->act, act_flags));
  else
    {
      pager_printf_color (ch, "&cPlayerFlags: &w%s\n\r",
			  ext_flag_string (&victim->act, plr_flags));
      pager_printf_color (ch, "&cPcflags    : &w%s\n\r",
			  flag_string (victim->pcdata->flags, pc_flags));
      if (victim->pcdata->nuisance)
	{
	  pager_printf_color (ch,
			      "&RNuisance   &cStage: (&R%d&c/%d)  Power:  &w%d  &cTime:  &w%s.\n\r",
			      victim->pcdata->nuisance->flags,
			      MAX_NUISANCE_STAGE,
			      victim->pcdata->nuisance->power,
			      ctime (&victim->pcdata->nuisance->time));
	}
    }
  if (victim->morph)
    {
      if (victim->morph->morph)
	pager_printf_color (ch,
			    "&cMorphed as : (&C%d&c) &C%s    &cTimer: &C%d\n\r",
			    victim->morph->morph->vnum,
			    victim->morph->morph->short_desc,
			    victim->morph->timer);
      else
	pager_printf_color (ch, "&cMorphed as: Morph was deleted.\n\r");
    }
  pager_printf_color (ch, "&cAffected by: &C%s\n\r",
		      affect_bit_name (&victim->affected_by));
/*
    pager_printf_color( ch, "&cSpeaks: &w%d   &cSpeaking: &w%d   &cExperience: &w%d",
    	victim->speaks, victim->speaking, victim->exp );
*/
  pager_printf_color (ch,
		      "&cSpeaks: &w%d   &cSpeaking: &w%d   &cExperience: &w%d",
		      victim->speaks, victim->speaking,
		      IS_NPC (victim) ? get_exp_worth (victim) <
		      MAX_EXP_WORTH ? get_exp_worth (victim) : MAX_EXP_WORTH :
		      victim->exp);


  if (!IS_NPC (victim) && victim->wait)
    pager_printf_color (ch, "   &cWaitState: &R%d\n\r", victim->wait / 12);
  else
    send_to_pager ("\n\r", ch);
  send_to_pager_color ("&cLanguages  : &w", ch);
  for (x = 0; lang_array[x] != LANG_UNKNOWN; x++)
    if (knows_language (victim, lang_array[x], victim)
	|| (IS_NPC (victim) && victim->speaks == 0))
      {
	if (IS_SET (lang_array[x], victim->speaking)
	    || (IS_NPC (victim) && !victim->speaking))
	  set_pager_color (AT_RED, ch);
	send_to_pager (lang_names[x], ch);
	send_to_pager (" ", ch);
	set_pager_color (AT_PLAIN, ch);
      }
    else
      if (IS_SET (lang_array[x], victim->speaking)
	  || (IS_NPC (victim) && !victim->speaking))
      {
	set_pager_color (AT_PINK, ch);
	send_to_pager (lang_names[x], ch);
	send_to_pager (" ", ch);
	set_pager_color (AT_PLAIN, ch);
      }
  send_to_pager ("\n\r", ch);
  if (victim->pcdata && victim->pcdata->bestowments
      && victim->pcdata->bestowments[0] != '\0')
    pager_printf_color (ch, "&cBestowments: &w%s\n\r",
			victim->pcdata->bestowments);
  if (IS_NPC (victim))
    pager_printf_color (ch, "&cShortdesc  : &w%s\n\r&cLongdesc   : &w%s",
			victim->short_descr[0] !=
			'\0' ? victim->short_descr : "(none set)",
			victim->long_descr[0] !=
			'\0' ? victim->long_descr : "(none set)\n\r");
  else
    {
      if (victim->short_descr[0] != '\0')
	pager_printf_color (ch, "&cShortdesc  : &w%s\n\r",
			    victim->short_descr);
      if (victim->long_descr[0] != '\0')
	pager_printf_color (ch, "&cLongdesc   : &w%s\n\r",
			    victim->long_descr);
    }
  if (IS_NPC (victim) && victim->spec_fun)
    pager_printf_color (ch, "&cMobile has spec fun: &w%s\n\r",
			lookup_spec (victim->spec_fun));
  if (IS_NPC (victim))
    pager_printf_color (ch, "&cBody Parts : &w%s\n\r",
			flag_string (victim->xflags, part_flags));
  if (victim->resistant > 0)
    pager_printf_color (ch, "&cResistant  : &w%s\n\r",
			flag_string (victim->resistant, ris_flags));
  if (victim->immune > 0)
    pager_printf_color (ch, "&cImmune     : &w%s\n\r",
			flag_string (victim->immune, ris_flags));
  if (victim->susceptible > 0)
    pager_printf_color (ch, "&cSusceptible: &w%s\n\r",
			flag_string (victim->susceptible, ris_flags));
  if (IS_NPC (victim))
    {
      pager_printf_color (ch, "&cAttacks    : &w%s\n\r",
			  ext_flag_string (&victim->attacks, attack_flags));
      pager_printf_color (ch, "&cDefenses   : &w%s\n\r",
			  ext_flag_string (&victim->defenses, defense_flags));
#ifdef SHADDAI
      if (xIS_SET (victim->act, ACT_PROTOTYPE))
	pager_printf_color (ch, "&cDefault Stance: &w%s\n\r",
			    get_stance_name (victim->pIndexData->stances[0]));
#endif
    }
#ifdef SHADDAI
  pager_printf_color (ch, "&cStances   : ");
  {
    int i, j = 1;
    for (i = (STANCE_NORMAL + 1); i < MAX_STANCE; i++, j++)
      {
	if (IS_NPC (victim))
	  pager_printf_color (ch, "&w%10s: %3d ", get_stance_name (i),
			      victim->pIndexData->stances[i]);
	else
	  pager_printf_color (ch, "&w%10s: %3d ", get_stance_name (i),
			      victim->pcdata->stances[i]);
	if (j == 4)
	  {
	    j = 0;
	    send_to_char ("\n\r            ", ch);
	  }
      }
  }
#endif
  send_to_char ("\n\r", ch);

  for (paf = victim->first_affect; paf; paf = paf->next)
    {
      if ((skill = get_skilltype (paf->type)) != NULL)
	pager_printf_color (ch,
			    "&c%s: &w'%s' mods %s by %d for %d rnds with bits %s.",
			    skill_tname[skill->type],
			    skill->name,
			    affect_loc_name (paf->location),
			    paf->modifier,
			    paf->duration, affect_bit_name (&paf->bitvector));
      send_to_pager ("\n\r", ch);
    }
/* pre variables update
    if ( ch->variables )
    {
	pager_printf_color( ch, "&cVariables  : &w" );
	for ( vd = victim->variables; vd; vd = vd->next )
	{
	    pager_printf_color( ch, "%s:%d", vd->tag, vd->vnum );
	    switch(vd->type)
	    {
		case vtSTR:
		    if ( vd->data )
			pager_printf_color( ch, "=%s", vd->data );
		    break;
		case vtINT:
		    if ( vd->data )
			pager_printf_color( ch, "=%d", (int)vd->data );
		    break;
		case vtXBIT:
	    }
pre variables update end */
/* post variables update */
  if (victim->variables)
    {
      pager_printf_color (ch, "&cVariables  : &w");
      for (vd = victim->variables; vd; vd = vd->next)
	{
	  pager_printf_color (ch, "%s:%d", vd->tag, vd->vnum);
	  switch (vd->type)
	    {
	    case vtSTR:
	      if (vd->data)
		pager_printf_color (ch, "=%s", vd->data);
	      break;
	    case vtINT:
	      if (vd->data)
		pager_printf_color (ch, "=%d", (int) vd->data);
	      break;
	    case vtXBIT:
	      if (vd->data)
		{
		  char buf[MAX_STRING_LENGTH];
		  int started = 0;

		  buf[0] = '\0';
		  for (x = MAX_BITS; x > 0; --x)
		    {
		      if (!started && xIS_SET (*(EXT_BV *) vd->data, x))
			started = x;
		    }
		  for (x = 1; x <= started; x++)
		    strcat (buf,
			    xIS_SET (*(EXT_BV *) vd->data, x) ? "1 " : "0 ");

		  if (buf[0] != '\0')
		    buf[strlen (buf) - 1] = '\0';
		  pager_printf_color (ch, "=[%s]", buf);
		}
	    }
/* post variables update end */
	  if (vd->next)
	    send_to_pager ("  ", ch);
	}
      send_to_pager ("\n\r", ch);
    }
  return;
}

void
do_mfind (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  MOB_INDEX_DATA *pMobIndex;
  int hash;
  int nMatch;
  bool fAll;

  set_pager_color (AT_PLAIN, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Mfind whom?\n\r", ch);
      return;
    }

  fAll = !str_cmp (arg, "all");
  nMatch = 0;

  /*
   * This goes through all the hash entry points (1024), and is therefore
   * much faster, though you won't get your vnums in order... oh well. :)
   *
   * Tests show that Furey's method will usually loop 32,000 times, calling
   * get_mob_index()... which loops itself, an average of 1-2 times...
   * So theoretically, the above routine may loop well over 40,000 times,
   * and my routine bellow will loop for as many index_mobiles are on
   * your mud... likely under 3000 times.
   * -Thoric
   */
  for (hash = 0; hash < MAX_KEY_HASH; hash++)
    for (pMobIndex = mob_index_hash[hash];
	 pMobIndex; pMobIndex = pMobIndex->next)
      if (fAll || nifty_is_name (arg, pMobIndex->player_name))
	{
	  nMatch++;
	  pager_printf (ch, "[%5d] %s\n\r",
			pMobIndex->vnum, capitalize (pMobIndex->short_descr));
	}

  if (nMatch)
    pager_printf (ch, "Number of matches: %d\n", nMatch);
  else
    send_to_char ("Nothing like that in hell, earth, or heaven.\n\r", ch);
  return;
}

void
do_ofind (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_INDEX_DATA *pObjIndex;
  int hash;
  int nMatch;
  bool fAll;

  set_pager_color (AT_PLAIN, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Ofind what?\n\r", ch);
      return;
    }

  fAll = !str_cmp (arg, "all");
  nMatch = 0;

  /*
   * This goes through all the hash entry points (1024), and is therefore
   * much faster, though you won't get your vnums in order... oh well. :)
   *
   * Tests show that Furey's method will usually loop 32,000 times, calling
   * get_obj_index()... which loops itself, an average of 2-3 times...
   * So theoretically, the above routine may loop well over 50,000 times,
   * and my routine bellow will loop for as many index_objects are on
   * your mud... likely under 3000 times.
   * -Thoric
   */
  for (hash = 0; hash < MAX_KEY_HASH; hash++)
    for (pObjIndex = obj_index_hash[hash];
	 pObjIndex; pObjIndex = pObjIndex->next)
      if (fAll || nifty_is_name (arg, pObjIndex->name))
	{
	  nMatch++;
	  pager_printf (ch, "[%5d] %s\n\r",
			pObjIndex->vnum, capitalize (pObjIndex->short_descr));
	}

  if (nMatch)
    pager_printf (ch, "Number of matches: %d\n", nMatch);
  else
    send_to_char ("Nothing like that in hell, earth, or heaven.\n\r", ch);
  return;
}

void
do_mwhere (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  bool found;

  set_pager_color (AT_PLAIN, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Mwhere whom?\n\r", ch);
      return;
    }

  found = FALSE;
  for (victim = first_char; victim; victim = victim->next)
    {
      if (IS_NPC (victim)
	  && victim->in_room
	  && (nifty_is_name (arg, victim->name)
	      || victim->pIndexData->vnum ==
	      (is_number (arg) ? atoi (arg) : 0)))
	{
	  found = TRUE;
	  pager_printf_color (ch, "[%5d] %-28s [%5d] %s\n\r",
			      victim->pIndexData->vnum,
			      victim->short_descr,
			      victim->in_room->vnum, victim->in_room->name);
	}
    }

  if (!found)
    act (AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR);
  return;
}

void
do_gwhere (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d;
  bool found = FALSE, pmobs = FALSE;
  int low = 1, high = 65, count = 0;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (arg1[0] != '\0')
    {
      if (arg1[0] == '\0' || arg2[0] == '\0')
	{
	  send_to_pager_color
	    ("\n\r&wSyntax:  gwhere | gwhere <low> <high> | gwhere <low> <high> mobs\n\r",
	     ch);
	  return;
	}
      low = atoi (arg1);
      high = atoi (arg2);
    }
  if (low < 1 || high < low || low > high || high > 65)
    {
      send_to_pager_color ("&wInvalid level range.\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg3);
  if (!str_cmp (arg3, "mobs"))
    pmobs = TRUE;

  pager_printf_color (ch, "\n\r&cGlobal %s locations:&w\n\r",
		      pmobs ? "mob" : "player");
  if (!pmobs)
    {
      for (d = first_descriptor; d; d = d->next)
	if ((d->connected == CON_PLAYING || d->connected == CON_EDITING)
	    && (victim = d->character) != NULL && !IS_NPC (victim)
	    && victim->in_room && can_see (ch, victim) && victim->level >= low
	    && victim->level <= high)
	  {
	    found = TRUE;
	    pager_printf_color (ch,
				"&c(&C%2d&c) &w%-12.12s   [%-5d - %-19.19s]   &c%-25.25s\n\r",
				victim->level, victim->name,
				victim->in_room->vnum,
				victim->in_room->area->name,
				victim->in_room->name);
	    count++;
	  }
    }
  else
    {
      for (victim = first_char; victim; victim = victim->next)
	if (IS_NPC (victim)
	    && victim->in_room && can_see (ch, victim)
	    && victim->level >= low && victim->level <= high)
	  {
	    found = TRUE;
	    pager_printf_color (ch,
				"&c(&C%2d&c) &w%-12.12s   [%-5d - %-19.19s]   &c%-25.25s\n\r",
				victim->level, victim->name,
				victim->in_room->vnum,
				victim->in_room->area->name,
				victim->in_room->name);
	    count++;
	  }
    }
  pager_printf_color (ch, "&c%d %s found.\n\r", count,
		      pmobs ? "mobs" : "characters");
  return;
}

void
do_gfighting (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  DESCRIPTOR_DATA *d;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  bool found = FALSE, pmobs = FALSE, phating = FALSE, phunting = FALSE;
  int low = 1, high = 65, count = 0;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (arg1[0] != '\0')
    {
      if (arg1[0] == '\0' || arg2[0] == '\0')
	{
	  send_to_pager_color
	    ("\n\r&wSyntax:  gfighting | gfighting <low> <high> | gfighting <low> <high> mobs\n\r",
	     ch);
	  return;
	}
      low = atoi (arg1);
      high = atoi (arg2);
    }
  if (low < 1 || high < low || low > high || high > 65)
    {
      send_to_pager_color ("&wInvalid level range.\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg3);
  if (!str_cmp (arg3, "mobs"))
    pmobs = TRUE;
  else if (!str_cmp (arg3, "hating"))
    phating = TRUE;
  else if (!str_cmp (arg3, "hunting"))
    phunting = TRUE;

  pager_printf_color (ch, "\n\r&cGlobal %s conflict:\n\r",
		      pmobs ? "mob" : "character");
  if (!pmobs && !phating && !phunting)
    {
      for (d = first_descriptor; d; d = d->next)
	if ((d->connected == CON_PLAYING || d->connected == CON_EDITING)
	    && (victim = d->character) != NULL && !IS_NPC (victim)
	    && victim->in_room && can_see (ch, victim) && victim->fighting
	    && victim->level >= low && victim->level <= high)
	  {
	    found = TRUE;
	    pager_printf_color (ch,
				"&w%-12.12s &C|%2d &wvs &C%2d| &w%-16.16s [%5d]  &c%-20.20s [%5d]\n\r",
				victim->name, victim->level,
				victim->fighting->who->level,
				IS_NPC (victim->fighting->
					who) ? victim->fighting->who->
				short_descr : victim->fighting->who->name,
				IS_NPC (victim->fighting->
					who) ? victim->fighting->who->
				pIndexData->vnum : 0,
				victim->in_room->area->name,
				victim->in_room ==
				NULL ? 0 : victim->in_room->vnum);
	    count++;
	  }
    }
  else if (!phating && !phunting)
    {
      for (victim = first_char; victim; victim = victim->next)
	if (IS_NPC (victim)
	    && victim->in_room && can_see (ch, victim)
	    && victim->fighting && victim->level >= low
	    && victim->level <= high)
	  {
	    found = TRUE;
	    pager_printf_color (ch,
				"&w%-12.12s &C|%2d &wvs &C%2d| &w%-16.16s [%5d]  &c%-20.20s [%5d]\n\r",
				victim->name, victim->level,
				victim->fighting->who->level,
				IS_NPC (victim->fighting->
					who) ? victim->fighting->who->
				short_descr : victim->fighting->who->name,
				IS_NPC (victim->fighting->
					who) ? victim->fighting->who->
				pIndexData->vnum : 0,
				victim->in_room->area->name,
				victim->in_room ==
				NULL ? 0 : victim->in_room->vnum);
	    count++;
	  }
    }
  else if (!phunting && phating)
    {
      for (victim = first_char; victim; victim = victim->next)
	if (IS_NPC (victim)
	    && victim->in_room && can_see (ch, victim)
	    && victim->hating && victim->level >= low
	    && victim->level <= high)
	  {
	    found = TRUE;
	    pager_printf_color (ch,
				"&w%-12.12s &C|%2d &wvs &C%2d| &w%-16.16s [%5d]  &c%-20.20s [%5d]\n\r",
				victim->name, victim->level,
				victim->hating->who->level,
				IS_NPC (victim->hating->
					who) ? victim->hating->who->
				short_descr : victim->hating->who->name,
				IS_NPC (victim->hating->
					who) ? victim->hating->who->
				pIndexData->vnum : 0,
				victim->in_room->area->name,
				victim->in_room ==
				NULL ? 0 : victim->in_room->vnum);
	    count++;
	  }
    }
  else if (phunting)
    {
      for (victim = first_char; victim; victim = victim->next)
	if (IS_NPC (victim)
	    && victim->in_room && can_see (ch, victim)
	    && victim->hunting && victim->level >= low
	    && victim->level <= high)
	  {
	    found = TRUE;
	    pager_printf_color (ch,
				"&w%-12.12s &C|%2d &wvs &C%2d| &w%-16.16s [%5d]  &c%-20.20s [%5d]\n\r",
				victim->name, victim->level,
				victim->hunting->who->level,
				IS_NPC (victim->hunting->
					who) ? victim->hunting->who->
				short_descr : victim->hunting->who->name,
				IS_NPC (victim->hunting->
					who) ? victim->hunting->who->
				pIndexData->vnum : 0,
				victim->in_room->area->name,
				victim->in_room ==
				NULL ? 0 : victim->in_room->vnum);
	    count++;
	  }
    }
  pager_printf_color (ch, "&c%d %s conflicts located.\n\r", count,
		      pmobs ? "mob" : "character");
  return;
}

/* Added 'show' argument for lowbie imms without ostat -- Blodkai */
/* Made show the default action :) Shaddai */
/* Trimmed size, added vict info, put lipstick on the pig -- Blod */
void
do_bodybag (CHAR_DATA * ch, char *argument)
{
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *owner;
  OBJ_DATA *obj;
  bool found = FALSE, bag = FALSE;

  argument = one_argument (argument, arg1);
  if (arg1[0] == '\0')
    {
      send_to_char_color
	("&PSyntax:  bodybag <character> | bodybag <character> yes/bag/now\n\r",
	 ch);
      return;
    }

  sprintf (buf3, " ");
  sprintf (buf2, "the corpse of %s", arg1);
  argument = one_argument (argument, arg2);

  if (arg2[0] != '\0' && (str_cmp (arg2, "yes")
			  && str_cmp (arg2, "bag") && str_cmp (arg2, "now")))
    {
      send_to_char_color
	("\n\r&PSyntax:  bodybag <character> | bodybag <character> yes/bag/now\n\r",
	 ch);
      return;
    }
  if (!str_cmp (arg2, "yes") || !str_cmp (arg2, "bag")
      || !str_cmp (arg2, "now"))
    bag = TRUE;

  pager_printf_color (ch, "\n\r&P%s remains of %s ... ",
		      bag ? "Retrieving" : "Searching for",
		      capitalize (arg1));
  for (obj = first_object; obj; obj = obj->next)
    {
      if (obj->in_room
	  && !str_cmp (buf2, obj->short_descr)
	  && (obj->pIndexData->vnum == 11))
	{
	  send_to_pager ("\n\r", ch);
	  found = TRUE;
	  pager_printf_color (ch,
			      "&P%s:  %s%-12.12s %s  &PIn:  &w%-22.22s  &P[&w%5d&P]   &PTimer:  %s%2d",
			      bag ? "Bagging" : "Corpse", bag ? "&Y" : "&w",
			      capitalize (arg1), IS_OBJ_STAT (obj,
							      ITEM_CLANCORPSE)
			      ? "&RPK" : "&R  ", obj->in_room->area->name,
			      obj->in_room->vnum,
			      obj->timer < 1 ? "&w" : obj->timer <
			      5 ? "&R" : obj->timer < 10 ? "&Y" : "&w",
			      obj->timer);
	  if (bag)
	    {
	      obj_from_room (obj);
	      obj = obj_to_char (obj, ch);
	      obj->timer = -1;
	      save_char_obj (ch);
	    }
	}
    }
  if (!found)
    {
      send_to_pager_color ("&Pno corpse was found.\n\r", ch);
      return;
    }
  send_to_pager ("\n\r", ch);
  for (owner = first_char; owner; owner = owner->next)
    {
      if (IS_NPC (owner))
	continue;
      if (can_see (ch, owner) && !str_cmp (arg1, owner->name))
	break;
    }
  if (owner == NULL)
    {
      pager_printf_color (ch, "&P%s is not currently online.\n\r",
			  capitalize (arg1));
      return;
    }
  if (owner->pcdata->deity)
    pager_printf_color (ch,
			"&P%s (%d) has %d favor with %s (needed to supplicate: %d)\n\r",
			owner->name, owner->level, owner->pcdata->favor,
			owner->pcdata->deity->name,
			owner->pcdata->deity->scorpse);
  else
    pager_printf_color (ch, "&P%s (%d) has no deity.\n\r",
			owner->name, owner->level);
  return;
}

#if 0
/* New owhere by Altrag, 03/14/96 */
void
do_owhere (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  bool found;
  int icnt = 0;

  set_pager_color (AT_PLAIN, ch);

  if (IS_NPC (ch))
    return;

  argument = one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Owhere what?\n\r", ch);
      return;
    }

  if (IS_SET (ch->pcdata->flags, PCFLAG_PAGERON))
    {
      send_to_char
	("The 'owhere' command temporarily  not be used in conjunction with the pager.\n\r",
	 ch);
      return;
    }

  argument = one_argument (argument, arg1);
  if (arg1[0] != '\0' && !str_prefix (arg1, "nesthunt"))
    {
      if (!(obj = get_obj_world (ch, arg)))
	{
	  send_to_char ("Nesthunt for what object?\n\r", ch);
	  return;
	}
      for (; obj->in_obj; obj = obj->in_obj)
	{
	  pager_printf (ch, "[%5d] %-28s in object [%5d] %s\n\r",
			obj->pIndexData->vnum, obj_short (obj),
			obj->in_obj->pIndexData->vnum,
			obj->in_obj->short_descr);
	  ++icnt;
	}
      sprintf (buf, "[%5d] %-28s in ", obj->pIndexData->vnum,
	       obj_short (obj));
      if (obj->carried_by)
	sprintf (buf + strlen (buf), "invent [%5d] %s\n\r",
		 (IS_NPC (obj->carried_by) ? obj->carried_by->pIndexData->vnum
		  : 0), PERS (obj->carried_by, ch));
      else if (obj->in_room)
	sprintf (buf + strlen (buf), "room   [%5d] %s\n\r",
		 obj->in_room->vnum, obj->in_room->name);
      else if (obj->in_obj)
	{
	  bug ("do_owhere: obj->in_obj after NULL!", 0);
	  strcat (buf, "object??\n\r");
	}
      else
	{
	  bug ("do_owhere: object doesnt have location!", 0);
	  strcat (buf, "nowhere??\n\r");
	}
      send_to_pager (buf, ch);
      ++icnt;
      pager_printf (ch, "Nested %d levels deep.\n\r", icnt);
      return;
    }

  found = FALSE;
  for (obj = first_object; obj; obj = obj->next)
    {
      if (!nifty_is_name (arg, obj->name))
	continue;
      found = TRUE;

      sprintf (buf, "(%3d) [%5d] %-28s in ", ++icnt, obj->pIndexData->vnum,
	       obj_short (obj));
      if (obj->carried_by)
	sprintf (buf + strlen (buf), "invent [%5d] %s\n\r",
		 (IS_NPC (obj->carried_by) ? obj->carried_by->pIndexData->vnum
		  : 0), PERS (obj->carried_by, ch));
      else if (obj->in_room)
	sprintf (buf + strlen (buf), "room   [%5d] %s\n\r",
		 obj->in_room->vnum, obj->in_room->name);
      else if (obj->in_obj)
	sprintf (buf + strlen (buf), "object [%5d] %s\n\r",
		 obj->in_obj->pIndexData->vnum, obj_short (obj->in_obj));
      else
	{
	  bug ("do_owhere: object doesnt have location!", 0);
	  strcat (buf, "nowhere??\n\r");
	}
      send_to_char (buf, ch);
    }

  if (!found)
    act (AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR);

  else
    pager_printf (ch, "%d matches.\n\r", icnt);
  return;
}
#endif


/*
 * "Claim" an object.  Will allow an immortal to "grab" an object no matter
 * where it is hiding.  ie: from a player's inventory, from deep inside
 * a container, from a mobile, from anywhere.			-Thoric
 */
void
do_oclaim (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  char arg3[MAX_INPUT_LENGTH];
  char *who = NULL;
  CHAR_DATA *vch = NULL;
  OBJ_DATA *obj;
  bool silently = FALSE, found = FALSE;
  int number, count, vnum;

  number = number_argument (argument, arg);
  argument = arg;
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);

  if (arg1[0] == '\0')
    {
      send_to_char ("Syntax: oclaim <object> [from who] [+silent]\r\n", ch);
      return;
    }
  if (arg3[0] == '\0')
    {
      if (arg2[0] != '\0')
	{
	  if (!str_cmp (arg2, "+silent"))
	    silently = TRUE;
	  else
	    who = arg2;
	}
    }
  else
    {
      who = arg2;
      if (!str_cmp (arg3, "+silent"))
	silently = TRUE;
    }

  if (who)
    {
      if ((vch = get_char_world (ch, who)) == NULL)
	{
	  send_to_pager ("They aren't here.\n\r", ch);
	  return;
	}
      if (get_trust (ch) < get_trust (vch) && !IS_NPC (vch))
	{
	  act (AT_TELL, "$n tells you, 'Keep your hands to yourself!'", vch,
	       NULL, ch, TO_VICT);
	  return;
	}
    }

  if (is_number (arg1))
    vnum = atoi (arg1);
  else
    vnum = -1;

  count = 0;
  for (obj = first_object; obj; obj = obj->next)
    {
      if (can_see_obj (ch, obj)
	  && (obj->pIndexData->vnum == vnum
	      || nifty_is_name (arg1, obj->name)) && (!vch
						      || vch ==
						      carried_by (obj)))
	if ((count += obj->count) >= number)
	  {
	    found = TRUE;
	    break;
	  }
    }
  if (!found && vnum != -1)
    {
      send_to_char ("You can't find that.\r\n", ch);
      return;
    }

  count = 0;
  for (obj = first_object; obj; obj = obj->next)
    {
      if (can_see_obj (ch, obj)
	  && (obj->pIndexData->vnum == vnum
	      || nifty_is_name_prefix (arg1, obj->name)) && (!vch
							     || vch ==
							     carried_by
							     (obj)))
	if ((count += obj->count) >= number)
	  {
	    found = TRUE;
	    break;
	  }
    }

  if (!found)
    {
      send_to_char ("You can't find that.\r\n", ch);
      return;
    }

  if (!vch && (vch = carried_by (obj)) != NULL)
    {
      if (get_trust (ch) < get_trust (vch) && !IS_NPC (vch))
	{
	  act (AT_TELL, "$n tells you, 'Keep your hands off $p!  It's mine.'",
	       vch, obj, ch, TO_VICT);
	  act (AT_IMMORT, "$n tried to lay claim to $p from your possession!",
	       vch, obj, ch, TO_CHAR);
	  return;
	}
    }

  separate_obj (obj);
  if (obj->item_type == ITEM_PORTAL)
    remove_portal (obj);

  if (obj->carried_by)
    obj_from_char (obj);
  else if (obj->in_room)
    obj_from_room (obj);
  else if (obj->in_obj)
    obj_from_obj (obj);

  obj_to_char (obj, ch);
  if (vch)
    {
      if (!silently)
	{
	  act (AT_IMMORT, "$n claims $p from you!", ch, obj, vch, TO_VICT);
	  act (AT_IMMORT, "$n claims $p from $N!", ch, obj, vch, TO_NOTVICT);
	  act (AT_IMMORT, "You claim $p from $N!", ch, obj, vch, TO_CHAR);
	}
      else
	act (AT_IMMORT, "You silently claim $p from $N.", ch, obj, vch,
	     TO_CHAR);
    }
  else
    {
      if (!silently)
	{
	  /* notify people in the room... (not done yet) */
	  act (AT_IMMORT, "You claim $p!", ch, obj, NULL, TO_CHAR);
	}
      else
	act (AT_IMMORT, "You silently claim $p.", ch, obj, NULL, TO_CHAR);
    }
}

void
do_reboo (CHAR_DATA * ch, char *argument)
{
  send_to_char_color ("&YIf you want to REBOOT, spell it out.\n\r", ch);
  return;
}

void
do_reboot (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  extern bool mud_down;
  CHAR_DATA *vch;

  set_char_color (AT_IMMORT, ch);

  sprintf (buf, "%s now", sysdata.port_name);
  sprintf (buf2, "%s nosave", sysdata.port_name);

  if (str_cmp (argument, buf)
      && str_cmp (argument, buf2)
      && str_cmp (argument, "and sort skill table"))
    {
      send_to_char
	("Syntax:  'reboot <port_name> now' or 'reboot <port_name> nosave'\n\r",
	 ch);
      return;
    }

  if (auction->item)
    do_auction (ch, "stop");

  sprintf (buf, "Reboot by %s.", ch->name);
  do_echo (ch, buf);

  if (!str_cmp (argument, "and sort skill table"))
    {
      sort_skill_table ();
      save_skill_table ();
    }

  /* Save all characters before booting. */
  if (str_cmp (argument, "nosave"))
    for (vch = first_char; vch; vch = vch->next)
      if (!IS_NPC (vch))
	save_char_obj (vch);

  mud_down = TRUE;
  return;
}

void
do_shutdow (CHAR_DATA * ch, char *argument)
{
  send_to_char_color ("&YIf you want to SHUTDOWN, spell it out.\n\r", ch);
  return;
}

void
do_shutdown (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  extern bool mud_down;
  CHAR_DATA *vch;

  set_char_color (AT_IMMORT, ch);

  sprintf (buf, "%s now", sysdata.port_name);
  sprintf (buf2, "%s nosave", sysdata.port_name);

  if (str_cmp (argument, buf) && str_cmp (argument, buf2))
    {
      send_to_char
	("Syntax:  'shutdown <port_name> now' or 'shutdown <port_name> nosave'\n\r",
	 ch);
      return;
    }

  if (auction->item)
    do_auction (ch, "stop");
  sprintf (buf, "Shutdown by %s.", ch->name);
  append_file (ch, SHUTDOWN_FILE, buf);
  strcat (buf, "\n\r");
  do_echo (ch, buf);

  /* Save all characters before booting. */
  if (str_cmp (argument, "nosave"))
    for (vch = first_char; vch; vch = vch->next)
      if (!IS_NPC (vch))
	save_char_obj (vch);
  mud_down = TRUE;
  return;
}

void
do_snoop (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Snoop whom?\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (!victim->desc)
    {
      send_to_char ("No descriptor to snoop.\n\r", ch);
      return;
    }
  if (victim == ch)
    {
      send_to_char ("Cancelling all snoops.\n\r", ch);
      for (d = first_descriptor; d; d = d->next)
	if (d->snoop_by == ch->desc)
	  d->snoop_by = NULL;
      return;
    }
  if (victim->desc->snoop_by)
    {
      send_to_char ("Busy already.\n\r", ch);
      return;
    }

  /*
   * Minimum snoop level... a secret mset value
   * makes the snooper think that the victim is already being snooped
   */
  if (get_trust (victim) >= get_trust (ch)
      || (victim->pcdata && victim->pcdata->min_snoop > get_trust (ch)))
    {
      send_to_char ("Busy already.\n\r", ch);
      return;
    }

  if (ch->desc)
    {
      for (d = ch->desc->snoop_by; d; d = d->snoop_by)
	if (d->character == victim || d->original == victim)
	  {
	    send_to_char ("No snoop loops.\n\r", ch);
	    return;
	  }
    }

/*  Snoop notification for higher imms, if desired, uncomment this */
#ifdef TOOSNOOPY
  if (get_trust (victim) > LEVEL_GOD && get_trust (ch) < LEVEL_SUPREME)
    write_to_descriptor (victim->desc->descriptor,
			 "\n\rYou feel like someone is watching your every move...\n\r",
			 0);
#endif
  victim->desc->snoop_by = ch->desc;
  send_to_char ("Ok.\n\r", ch);
  return;
}

void
do_statshield (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (IS_NPC (ch) || get_trust (ch) < LEVEL_GREATER)
    {
      send_to_char ("Huh?\n\r", ch);
      return;
    }
  if (arg[0] == '\0')
    {
      send_to_char ("Statshield which mobile?\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("No such mobile.\n\r", ch);
      return;
    }
  if (!IS_NPC (victim))
    {
      send_to_char ("You can only statshield mobiles.\n\r", ch);
      return;
    }
  if (xIS_SET (victim->act, ACT_STATSHIELD))
    {
      xREMOVE_BIT (victim->act, ACT_STATSHIELD);
      ch_printf (ch, "You have lifted the statshield on %s.\n\r",
		 victim->short_descr);
    }
  else
    {
      xSET_BIT (victim->act, ACT_STATSHIELD);
      ch_printf (ch, "You have applied a statshield to %s.\n\r",
		 victim->short_descr);
    }
  return;
}


void
do_switch (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Switch into whom?\n\r", ch);
      return;
    }
  if (!ch->desc)
    return;
  if (ch->desc->original)
    {
      send_to_char ("You are already switched.\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (victim == ch)
    {
      send_to_char ("Ok.\n\r", ch);
      return;
    }
  if (IS_NPC (victim) && xIS_SET (victim->act, ACT_STATSHIELD)
      && get_trust (ch) < LEVEL_GREATER)
    {
      set_pager_color (AT_IMMORT, ch);
      send_to_pager
	("Their godly glow prevents you from getting close enough.\n\r", ch);
      return;
    }
  if (victim->desc)
    {
      send_to_char ("Character in use.\n\r", ch);
      return;
    }
  if (!IS_NPC (victim) && ch->level < LEVEL_GREATER)
    {
      send_to_char ("You cannot switch into a player!\n\r", ch);
      return;
    }
  if (victim->switched)
    {
      send_to_char ("You can't switch into a player that is switched!\n\r",
		    ch);
      return;
    }
  if (!IS_NPC (victim) && xIS_SET (victim->act, PLR_FREEZE))
    {
      send_to_char ("You shouldn't switch into a player that is frozen!\n\r",
		    ch);
      return;
    }
  ch->desc->character = victim;
  ch->desc->original = ch;
  victim->desc = ch->desc;
  ch->desc = NULL;
  ch->switched = victim;
  send_to_char ("Ok.\n\r", victim);
  return;
}

void
do_return (CHAR_DATA * ch, char *argument)
{

  if (!IS_NPC (ch) && get_trust (ch) < LEVEL_IMMORTAL)
    {
      send_to_char ("Huh?\n\r", ch);
      return;
    }

  if (!ch->desc)
    return;
  if (!ch->desc->original)
    {
      set_char_color (AT_IMMORT, ch);
      send_to_char ("You aren't switched.\n\r", ch);
      return;
    }


  if (IS_NPC (ch) && IS_AFFECTED (ch, AFF_POSSESS))
    {
      affect_strip (ch, gsn_possess);
      xREMOVE_BIT (ch->affected_by, AFF_POSSESS);
    }

  ch->desc->character = ch->desc->original;
  ch->desc->original = NULL;
  ch->desc->character->desc = ch->desc;
  ch->desc->character->switched = NULL;
  ch->desc = NULL;
  set_char_color (AT_IMMORT, ch);
  send_to_char ("You return to your original body.\n\r", ch);
  return;
}

void
do_minvoke (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  MOB_INDEX_DATA *pMobIndex;
  CHAR_DATA *victim;
  int vnum;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Syntax:  minvoke <vnum>\n\r", ch);
      return;
    }
  if (!is_number (arg))
    {
      char arg2[MAX_INPUT_LENGTH];
      int hash, cnt;
      int count = number_argument (arg, arg2);

      vnum = -1;
      for (hash = cnt = 0; hash < MAX_KEY_HASH; hash++)
	for (pMobIndex = mob_index_hash[hash];
	     pMobIndex; pMobIndex = pMobIndex->next)
	  if (nifty_is_name (arg2, pMobIndex->player_name) && ++cnt == count)
	    {
	      vnum = pMobIndex->vnum;
	      break;
	    }
      if (vnum == -1)
	{
	  send_to_char ("No such mobile exists.\n\r", ch);
	  return;
	}
    }
  else
    vnum = atoi (arg);

  if (get_trust (ch) < LEVEL_DEMI)
    {
      AREA_DATA *pArea;

      if (IS_NPC (ch))
	{
	  send_to_char ("Huh?\n\r", ch);
	  return;
	}
      if (!ch->pcdata || !(pArea = ch->pcdata->area))
	{
	  send_to_char
	    ("You must have an assigned area to invoke this mobile.\n\r", ch);
	  return;
	}
      if (vnum < pArea->low_m_vnum && vnum > pArea->hi_m_vnum)
	{
	  send_to_char ("That number is not in your allocated range.\n\r",
			ch);
	  return;
	}
    }
  if ((pMobIndex = get_mob_index (vnum)) == NULL)
    {
      send_to_char ("No mobile has that vnum.\n\r", ch);
      return;
    }

  victim = create_mobile (pMobIndex);
  char_to_room (victim, ch->in_room);
  act (AT_IMMORT, "$n invokes $N!", ch, NULL, victim, TO_ROOM);
  /*How about seeing what we're invoking for a change. -Blodkai */
  ch_printf_color (ch, "&YYou invoke %s (&W#%d &Y- &W%s &Y- &Wlvl %d&Y)\n\r",
		   pMobIndex->short_descr, pMobIndex->vnum,
		   pMobIndex->player_name, victim->level);
  return;
}

void
do_oinvoke (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];	/* Vnum/keyword */
  char arg2[MAX_INPUT_LENGTH];	/* Level */
  char arg3[MAX_INPUT_LENGTH];	/* Quantity */

  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *obj;
  int vnum;
  int level;
  int quantity = 1;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);

  if (arg1[0] == '\0')
    {
      send_to_char ("Syntax: oinvoke <vnum> <level> <quantity>.\n\r", ch);
      return;
    }
  if (arg2[0] == '\0')
    {
      level = get_trust (ch);
    }
  else
    {
      if (!is_number (arg2))
	{
	  send_to_char ("Syntax:  oinvoke <vnum> <level>\n\r", ch);
	  return;
	}
      level = atoi (arg2);
      if (level < 0 || level > get_trust (ch))
	{
	  send_to_char ("Limited to your trust level.\n\r", ch);
	  return;
	}
    }
  if (arg3[0] != '\0')
    {
      if (!is_number (arg3))
	{
	  send_to_char ("Syntax:  oinvoke <vnum> <level> <quantity>\n\r", ch);
	  return;
	}

      quantity = atoi (arg3);

      if (quantity < 1 || quantity > MAX_OINVOKE_QUANTITY)
	{
	  ch_printf (ch, "You must oinvoke between 1 and %d items.\n\r",
		     MAX_OINVOKE_QUANTITY);
	  return;
	}
    }

  if (!is_number (arg1))
    {
      char arg[MAX_INPUT_LENGTH];
      int hash, cnt;
      int count = number_argument (arg1, arg);

      vnum = -1;
      for (hash = cnt = 0; hash < MAX_KEY_HASH; hash++)
	for (pObjIndex = obj_index_hash[hash];
	     pObjIndex; pObjIndex = pObjIndex->next)
	  if (nifty_is_name (arg, pObjIndex->name) && ++cnt == count)
	    {
	      vnum = pObjIndex->vnum;
	      break;
	    }
      if (vnum == -1)
	{
	  send_to_char ("No such object exists.\n\r", ch);
	  return;
	}
    }
  else
    vnum = atoi (arg1);

  if (get_trust (ch) < LEVEL_DEMI)
    {
      AREA_DATA *pArea;

      if (IS_NPC (ch))
	{
	  send_to_char ("Huh?\n\r", ch);
	  return;
	}
      if (!ch->pcdata || !(pArea = ch->pcdata->area))
	{
	  send_to_char
	    ("You must have an assigned area to invoke this object.\n\r", ch);
	  return;
	}
      if (vnum < pArea->low_o_vnum && vnum > pArea->hi_o_vnum)
	{
	  send_to_char ("That number is not in your allocated range.\n\r",
			ch);
	  return;
	}
    }
  if ((pObjIndex = get_obj_index (vnum)) == NULL)
    {
      send_to_char ("No object has that vnum.\n\r", ch);
      return;
    }

  if (level == 0)
    {
      AREA_DATA *temp_area;

      if ((temp_area = get_area_obj (pObjIndex)) == NULL)
	level = ch->level;
      else
	{
	  level = generate_itemlevel (temp_area, pObjIndex);
	  level = URANGE (0, level, LEVEL_AVATAR);
	}
    }

  obj = create_object (pObjIndex, level);

  if (quantity > 1 && (!xIS_SET (obj->extra_flags, ITEM_MULTI_INVOKE)))
    {
      send_to_char
	("This item can not be invoked in quantities greater than 1.\n\r",
	 ch);
      return;
    }
  else
    obj->count = quantity;

  if (CAN_WEAR (obj, ITEM_TAKE))
    {
      obj = obj_to_char (obj, ch);
    }
  else
    {
      obj = obj_to_room (obj, ch->in_room);
      act (AT_IMMORT, "$n fashions $p from ether!", ch, obj, NULL, TO_ROOM);
    }

  /* I invoked what? --Blodkai */
  ch_printf_color (ch,
		   "&YYou invoke %s (&W#%d &Y- &W%s &Y- &Wlvl %d &Y- &Wqty %d&Y)\n\r",
		   pObjIndex->short_descr, pObjIndex->vnum, pObjIndex->name,
		   obj->level, quantity);
  sprintf (log_buf, "%s invoked %s (vnum %d - %s - level %d - quantity %d)",
	   ch->name, pObjIndex->short_descr, pObjIndex->vnum, pObjIndex->name,
	   obj->level, quantity);
  log_string_plus (log_buf, LOG_ALL, 62);
  sprintf (log_buf, "%s (v%d / %s / l%d / q%d)",
	   pObjIndex->short_descr, pObjIndex->vnum, pObjIndex->name,
	   obj->level, quantity);
  append_file (ch, OINVOKE_FILE, log_buf);
  return;
}

void
do_purge (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      /* 'purge' */
      CHAR_DATA *vnext;
      OBJ_DATA *obj_next;

      for (victim = ch->in_room->first_person; victim; victim = vnext)
	{
	  vnext = victim->next_in_room;
	  if (IS_NPC (victim) && victim != ch)
	    extract_char (victim, TRUE);
	}

      for (obj = ch->in_room->first_content; obj; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  extract_obj (obj);
	}

      act (AT_IMMORT, "$n purges the room!", ch, NULL, NULL, TO_ROOM);
      act (AT_IMMORT, "You have purged the room!", ch, NULL, NULL, TO_CHAR);
      save_house_by_vnum (ch->in_room->vnum);	/* Prevent House Object Duplication */
      /* Clan storeroom check */
      if (xIS_SET (ch->in_room->room_flags, ROOM_CLANSTOREROOM))
	{
	  VAULT_DATA *vault;
	  char buf[20];

	  for (vault = first_vault; vault; vault = vault->next)
	    if (vault->vnum == ch->in_room->vnum)
	      {
		sprintf (buf, "%d", vault->vnum);
		save_storeroom (ch, buf);
	      }

	}
      return;
    }
  victim = NULL;
  obj = NULL;

  /* fixed to get things in room first -- i.e., purge portal (obj),
   * no more purging mobs with that keyword in another room first
   * -- Tri */
  if ((victim = get_char_room (ch, arg)) == NULL
      && (obj = get_obj_here (ch, arg)) == NULL)
    {
      if ((victim = get_char_world (ch, arg)) == NULL && (obj = get_obj_world (ch, arg)) == NULL)	/* no get_obj_room */
	{
	  send_to_char ("They aren't here.\n\r", ch);
	  return;
	}
    }

/* Single object purge in room for high level purge - Scryn 8/12*/
  if (obj)
    {
      int vnum = (obj->in_room ? obj->in_room->vnum : ROOM_VNUM_LIMBO);
      separate_obj (obj);
      act (AT_IMMORT, "$n purges $p.", ch, obj, NULL, TO_ROOM);
      act (AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj,
	   NULL, TO_CHAR);
      extract_obj (obj);
      save_house_by_vnum (vnum);	/* Prevent House Object Duplication */
      if (xIS_SET (ch->in_room->room_flags, ROOM_CLANSTOREROOM))
	{
	  VAULT_DATA *vault;
	  char buf[20];

	  for (vault = first_vault; vault; vault = vault->next)
	    if (vault->vnum == ch->in_room->vnum)
	      {
		sprintf (buf, "%d", vault->vnum);
		save_storeroom (ch, buf);
	      }

	}
      return;
    }

  if (!IS_NPC (victim))
    {
      send_to_char ("Not on PC's.\n\r", ch);
      return;
    }

  if (victim == ch)
    {
      send_to_char ("You cannot purge yourself!\n\r", ch);
      return;
    }

  act (AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT);
  act (AT_IMMORT, "You make $N disappear in a puff of smoke!", ch, NULL,
       victim, TO_CHAR);
  extract_char (victim, TRUE);
  return;
}


/* Sunangel
   LOOP <command> [start] [end] [params] */
void
do_loop (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int startvnum, endvnum, i;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);

  if (arg1[0] == '\0')
    {
      send_to_char ("Syntax: loop <command> <start#> <end#> <params>\n\r",
		    ch);
      send_to_char ("  Where <command> is a valid command to execute,\n\r",
		    ch);
      send_to_char ("  <start#> and <end#> are numbers/vnums,\n\r", ch);
      send_to_char ("  and <params> is a parameter list for <command>.\n\r",
		    ch);
      send_to_char ("EXAMPLE: LOOP MSET 22000 22100 FLAGS PROTOTYPE&C&w\n\r",
		    ch);
      return;
    }

  if (arg2[0] == '\0')
    {
      send_to_char ("You must specify a start number/vnum.\n\r", ch);
      return;
    }

  if (arg3[0] == '\0')
    {
      send_to_char ("You must specify an end number/vnum.\n\r", ch);
      return;
    }

  startvnum = (is_number (arg2) ? atoi (arg2) : 1);
  endvnum = (is_number (arg3) ? atoi (arg3) : 1);

  if (endvnum < 0)
    endvnum = 1;

  if (startvnum < 0)
    startvnum = 1;

  if (startvnum > endvnum)
    {
      i = endvnum;
      endvnum = startvnum;
      startvnum = i;
    }

  sprintf (buf, "Beginning loop for %s command, vnums %d to %d (%s).\n\r",
	   arg1, startvnum, endvnum, argument);

  send_to_char (buf, ch);

  for (i = startvnum; i <= endvnum; i++)
    {
      sprintf (buf, "%s %d %s", arg1, i,
	       (!str_cmp (arg1, "mstat") || !str_cmp (arg1, "ostat")) ?
	       "\b" : argument);
      interpret (ch, buf);
    }

  send_to_char ("Done.\n\r", ch);

  return;
}


void
do_low_purge (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Purge what?\n\r", ch);
      return;
    }

  victim = NULL;
  obj = NULL;
  if ((victim = get_char_room (ch, arg)) == NULL
      && (obj = get_obj_here (ch, arg)) == NULL)
    {
      send_to_char ("You can't find that here.\n\r", ch);
      return;
    }

  if (obj)
    {
      separate_obj (obj);
      act (AT_IMMORT, "$n purges $p!", ch, obj, NULL, TO_ROOM);
      act (AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj,
	   NULL, TO_CHAR);
      extract_obj (obj);
      save_house_by_vnum (ch->in_room->vnum);	/* Prevent House Object Duplication */
      if (xIS_SET (ch->in_room->room_flags, ROOM_CLANSTOREROOM))
	{
	  VAULT_DATA *vault;
	  char buf[20];

	  for (vault = first_vault; vault; vault = vault->next)
	    if (vault->vnum == ch->in_room->vnum)
	      {
		sprintf (buf, "%d", vault->vnum);
		save_storeroom (ch, buf);
	      }

	}
      return;
    }

  if (!IS_NPC (victim))
    {
      send_to_char ("Not on PC's.\n\r", ch);
      return;
    }

  if (victim == ch)
    {
      send_to_char ("You cannot purge yourself!\n\r", ch);
      return;
    }

  act (AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT);
  act (AT_IMMORT, "You make $N disappear in a puff of smoke!", ch, NULL,
       victim, TO_CHAR);
  extract_char (victim, TRUE);
  return;
}


void
do_balzhur (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char *name;
  CHAR_DATA *victim;
  AREA_DATA *pArea;
  int sn;

  set_char_color (AT_BLOOD, ch);

  argument = one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Who is deserving of such a fate?\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't currently playing.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("This will do little good on mobiles.\n\r", ch);
      return;
    }
  if (victim->level >= get_trust (ch))
    {
      send_to_char ("I wouldn't even think of that if I were you...\n\r", ch);
      return;
    }

  set_char_color (AT_WHITE, ch);
  send_to_char ("You summon the demon Balzhur to wreak your wrath!\n\r", ch);
  send_to_char
    ("Balzhur sneers at you evilly, then vanishes in a puff of smoke.\n\r",
     ch);
  set_char_color (AT_IMMORT, victim);
  send_to_char
    ("You hear an ungodly sound in the distance that makes your blood run cold!\n\r",
     victim);
  sprintf (buf, "Balzhur screams, 'You are MINE %s!!!'", victim->name);
  echo_to_all (AT_IMMORT, buf, ECHOTAR_ALL);
  victim->level = 2;
  victim->trust = 0;
  victim->exp = 2000;
  victim->max_hit = 10;
  victim->max_mana = 100;
  victim->max_move = 100;
  for (sn = 0; sn < top_sn; sn++)
    victim->pcdata->learned[sn] = 0;
  victim->practice = 0;
  victim->hit = victim->max_hit;
  victim->mana = victim->max_mana;
  victim->move = victim->max_move;
  name = capitalize (victim->name);
  sprintf (buf, "%s%s", GOD_DIR, name);

  set_char_color (AT_RED, ch);
  if (!remove (buf))
    send_to_char ("Player's immortal data destroyed.\n\r", ch);
  else if (errno != ENOENT)
    {
      ch_printf (ch,
		 "Unknown error #%d - %s (immortal data).  Report to Thoric\n\r",
		 errno, strerror (errno));
      sprintf (buf2, "%s balzhuring %s", ch->name, buf);
      perror (buf2);
    }
  sprintf (buf2, "%s.are", name);
  for (pArea = first_build; pArea; pArea = pArea->next)
    if (!str_cmp (pArea->filename, buf2))
      {
	sprintf (buf, "%s%s", BUILD_DIR, buf2);
	if (IS_SET (pArea->status, AREA_LOADED))
	  fold_area (pArea, buf, FALSE);
	close_area (pArea);
	sprintf (buf2, "%s.bak", buf);
	set_char_color (AT_RED, ch);	/* Log message changes colors */
	if (!rename (buf, buf2))
	  send_to_char
	    ("Player's area data destroyed.  Area saved as backup.\n\r", ch);
	else if (errno != ENOENT)
	  {
	    ch_printf (ch,
		       "Unknown error #%d - %s (area data).  Report to  Thoric.\n\r",
		       errno, strerror (errno));
	    sprintf (buf2, "%s destroying %s", ch->name, buf);
	    perror (buf2);
	  }
	break;
      }

  make_wizlist ();
  advance_level (victim);
  do_help (victim, "M_BALZHUR_");
  set_char_color (AT_WHITE, victim);
  send_to_char ("You awake after a long period of time...\n\r", victim);
  while (victim->first_carrying)
    extract_obj (victim->first_carrying);
  return;
}

void
do_advance (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int level;
  int iLevel;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number (arg2))
    {
      send_to_char ("Syntax:  advance <character> <level>\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg1)) == NULL)
    {
      send_to_char ("That character is not in the room.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("You cannot advance a mobile.\n\r", ch);
      return;
    }
  /*You can demote yourself but not someone else at your own trust.-- Narn */
  if (get_trust (ch) <= get_trust (victim) && ch != victim)
    {
      send_to_char ("You can't do that.\n\r", ch);
      return;
    }
  if ((level = atoi (arg2)) < 1 || level > MAX_LEVEL)
    {
      send_to_char ("Level range is 1 to 65.\n\r", ch);
      return;
    }
  if (level > get_trust (ch))
    {
      send_to_char ("Level limited to your trust level.\n\r", ch);
      return;
    }
  /* Lower level:
   *   Reset to level 1.
   *   Then raise again.
   *   Currently, an imp can lower another imp.
   *   -- Swiftest
   *   Can't lower imms >= your trust (other than self) per Narn's change.
   *   Few minor text changes as well.  -- Blod
   */
  if (level <= victim->level)
    {
      int sn;

      set_char_color (AT_IMMORT, victim);
      if (level < victim->level)
	{
	  ch_printf (ch, "Demoting %s from level %d to level %d!\n\r",
		     victim->name, victim->level, level);
	  send_to_char
	    ("Cursed and forsaken!  The gods have lowered your level...\n\r",
	     victim);
	}
      else
	{
	  ch_printf (ch, "%s is already level %d.  Re-advancing...\n\r",
		     victim->name, level);
	  send_to_char
	    ("Deja vu!  Your mind reels as you re-live your past levels!\n\r",
	     victim);
	}
      victim->level = 1;
      victim->exp = exp_level (victim, 1);
      victim->max_hit = 20;
      victim->max_mana = 100;
      victim->max_move = 100;
      for (sn = 0; sn < top_sn; sn++)
	victim->pcdata->learned[sn] = 0;
      victim->practice = 0;
      victim->hit = victim->max_hit;
      victim->mana = victim->max_mana;
      victim->move = victim->max_move;
      advance_level (victim);
      /* Rank fix added by Narn. */
      DISPOSE (victim->pcdata->rank);
      victim->pcdata->rank = str_dup ("");
      /* Stuff added to make sure character's wizinvis level doesn't stay
         higher than actual level, take wizinvis away from advance < 50 */
      if (xIS_SET (victim->act, PLR_WIZINVIS))
	victim->pcdata->wizinvis = victim->trust;
      if (xIS_SET (victim->act, PLR_WIZINVIS)
	  && (victim->level <= LEVEL_AVATAR))
	{
	  xREMOVE_BIT (victim->act, PLR_WIZINVIS);
	  victim->pcdata->wizinvis = victim->trust;
	}
    }
  else
    {
      ch_printf (ch, "Raising %s from level %d to level %d!\n\r",
		 victim->name, victim->level, level);
      if (victim->level >= LEVEL_AVATAR)
	{
	  set_char_color (AT_IMMORT, victim);
	  act (AT_IMMORT,
	       "$n makes some arcane gestures with $s hands, then points $s finger at you!",
	       ch, NULL, victim, TO_VICT);
	  act (AT_IMMORT,
	       "$n makes some arcane gestures with $s hands, then points $s finger at $N!",
	       ch, NULL, victim, TO_NOTVICT);
	  set_char_color (AT_WHITE, victim);
	  send_to_char ("You suddenly feel very strange...\n\r\n\r", victim);
	  set_char_color (AT_LBLUE, victim);
	}
      switch (level)
	{
	default:
	  send_to_char ("The gods feel fit to raise your level!\n\r", victim);
	  break;
	case LEVEL_IMMORTAL:
	  do_help (victim, "M_GODLVL1_");
	  set_char_color (AT_WHITE, victim);
	  send_to_char ("You awake... all your possessions are gone.\n\r",
			victim);
	  while (victim->first_carrying)
	    extract_obj (victim->first_carrying);
	  break;
	case LEVEL_ACOLYTE:
	  do_help (victim, "M_GODLVL2_");
	  break;
	case LEVEL_CREATOR:
	  do_help (victim, "M_GODLVL3_");
	  break;
	case LEVEL_SAVIOR:
	  do_help (victim, "M_GODLVL4_");
	  break;
	case LEVEL_DEMI:
	  do_help (victim, "M_GODLVL5_");
	  break;
	case LEVEL_TRUEIMM:
	  do_help (victim, "M_GODLVL6_");
	  break;
	case LEVEL_LESSER:
	  do_help (victim, "M_GODLVL7_");
	  break;
	case LEVEL_GOD:
	  do_help (victim, "M_GODLVL8_");
	  break;
	case LEVEL_GREATER:
	  do_help (victim, "M_GODLVL9_");
	  break;
	case LEVEL_ASCENDANT:
	  do_help (victim, "M_GODLVL10_");
	  break;
	case LEVEL_SUB_IMPLEM:
	  do_help (victim, "M_GODLVL11_");
	  break;
	case LEVEL_IMPLEMENTOR:
	  do_help (victim, "M_GODLVL12_");
	  break;
	case LEVEL_ETERNAL:
	  do_help (victim, "M_GODLVL13_");
	  break;
	case LEVEL_INFINITE:
	  do_help (victim, "M_GODLVL14_");
	  break;
	case LEVEL_SUPREME:
	  do_help (victim, "M_GODLVL15_");
	}
    }
  for (iLevel = victim->level; iLevel < level; iLevel++)
    {
      if (level < LEVEL_IMMORTAL)
	send_to_char ("You raise a level!!\n\r", victim);
      victim->level += 1;
      advance_level (victim);
    }
  victim->exp = exp_level (victim, victim->level);
  victim->trust = 0;
  return;
}

void
do_elevate (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Syntax: elevate <char>\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("That player is not here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }
  if (victim->level == LEVEL_IMMORTAL)
    {
      send_to_char ("Elevating a player...\n\r", ch);
      set_char_color (AT_IMMORT, victim);
      act (AT_IMMORT,
	   "$n begins to chant softly... then makes some arcane gestures...",
	   ch, NULL, NULL, TO_ROOM);
      set_char_color (AT_WHITE, victim);
      send_to_char ("You suddenly feel very strange...\n\r\n\r", victim);
      set_char_color (AT_LBLUE, victim);
      do_help (victim, "M_GODLVL2_");
      victim->level = LEVEL_ACOLYTE;
      set_char_color (AT_WHITE, victim);
      advance_level (victim);
      victim->exp = exp_level (victim, victim->level);
      victim->trust = 0;
      return;
    }
  if (victim->level == LEVEL_ACOLYTE)
    {
      send_to_char ("Elevating a player...\n\r", ch);
      set_char_color (AT_IMMORT, victim);
      act (AT_IMMORT,
	   "$n begins to chant softly... then makes some arcane gestures...",
	   ch, NULL, NULL, TO_ROOM);
      set_char_color (AT_WHITE, victim);
      send_to_char ("You suddenly feel very strange...\n\r\n\r", victim);
      set_char_color (AT_LBLUE, victim);
      do_help (victim, "M_GODLVL3_");
      victim->level = LEVEL_CREATOR;
      set_char_color (AT_WHITE, victim);
      advance_level (victim);
      victim->exp = exp_level (victim, victim->level);
      victim->trust = 0;
      return;
    }
  else
    send_to_char ("You cannot elevate this character.\n\r", ch);
  return;
}

void
do_imm_news (CHAR_DATA * ch, char *argument)
{

  if (IS_NPC (ch))
    return;

  if (!IS_IMMORTAL (ch))
    {
      send_to_char ("Huh?\n\r", ch);
      return;
    }

  send_to_char_color ("\n\r&G&cImmortal news of the Realms ...\n\r", ch);
  show_file (ch, IMMNEWS_FILE);
  return;
}

void
do_immortalize (CHAR_DATA * ch, char *argument)
{
  int i;
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Syntax:  immortalize <char>\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("That player is not here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }
  if (victim->level != LEVEL_AVATAR)
    {
      send_to_char ("This player is not yet worthy of immortality.\n\r", ch);
      return;
    }

  send_to_char ("Immortalizing a player...\n\r", ch);
  set_char_color (AT_IMMORT, victim);
  act (AT_IMMORT,
       "$n begins to chant softly... then raises $s arms to the sky...", ch,
       NULL, NULL, TO_ROOM);
  set_char_color (AT_WHITE, victim);
  send_to_char ("You suddenly feel very strange...\n\r\n\r", victim);
  set_char_color (AT_LBLUE, victim);
  do_help (victim, "M_GODLVL1_");
  set_char_color (AT_WHITE, victim);
  send_to_char ("You awake... all your possessions are gone.\n\r", victim);
  while (victim->first_carrying)
    extract_obj (victim->first_carrying);
  victim->level = LEVEL_IMMORTAL;
  advance_level (victim);

  /* Remove clan/guild/order and update accordingly */
  if (victim->pcdata->clan)
    {
      if (victim->pcdata->clan->clan_type == CLAN_GUILD)
	{
	  int sn;

	  for (sn = 0; sn < top_sn; sn++)
	    if (skill_table[sn]->guild == victim->pcdata->clan->class
		&& skill_table[sn]->name != NULL)
	      victim->pcdata->learned[sn] = 0;
	}
      if (victim->speaking & LANG_CLAN)
	victim->speaking = LANG_COMMON;
      REMOVE_BIT (victim->speaks, LANG_CLAN);
      --victim->pcdata->clan->members;
      remove_member (victim->pcdata->clan_name, victim->name);
      if (!str_cmp (victim->name, victim->pcdata->clan->leader))
	{
	  STRFREE (victim->pcdata->clan->leader);
	  victim->pcdata->clan->leader = STRALLOC ("");
	}
      if (!str_cmp (victim->name, victim->pcdata->clan->number1))
	{
	  STRFREE (victim->pcdata->clan->number1);
	  victim->pcdata->clan->number1 = STRALLOC ("");
	}
      if (!str_cmp (victim->name, victim->pcdata->clan->number2))
	{
	  STRFREE (victim->pcdata->clan->number2);
	  victim->pcdata->clan->number2 = STRALLOC ("");
	}
      victim->pcdata->clan = NULL;
      STRFREE (victim->pcdata->clan_name);
    }

  /* create immortal only data for tell history */
  CREATE (victim->pcdata->tell_history, char *, 26);
  for (i = 0; i < 26; i++)
    victim->pcdata->tell_history[i] = NULL;
  victim->exp = exp_level (victim, victim->level);
  victim->trust = 0;
  save_char_obj (victim);
  return;
}

void
do_mobinvade (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  AREA_DATA *tarea;
  int count, created;
  bool found = FALSE;
  MOB_INDEX_DATA *pMobIndex;
  ROOM_INDEX_DATA *location;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);
  count = atoi (arg2);
  set_char_color (AT_GREEN, ch);
  if (arg1[0] == '\0' || arg2[0] == '\0')
    {
      send_to_char ("Invade <area> <# of invaders> <mob vnum>\n\r", ch);
      return;
    }
  for (tarea = first_area; tarea; tarea = tarea->next)
    if (!str_cmp (tarea->filename, arg1))
      {
	found = TRUE;
	break;
      }
  if (!found)
    {
      send_to_char ("Area not found.\n\r", ch);
      return;
    }
  if (count > 300)
    {
      send_to_char ("Whoa...Less than 300 please.\n\r", ch);
      return;
    }
  if ((pMobIndex = get_mob_index (atoi (arg3))) == NULL)
    {
      send_to_char ("No mobile has that vnum.\n\r", ch);
      return;
    }

  for (created = 0; created < count; created++)
    {
      if ((location =
	   get_room_index (number_range
			   (tarea->low_r_vnum, tarea->hi_r_vnum))) == NULL)
	{
	  --created;
	  continue;
	}
      if (xIS_SET (location->room_flags, ROOM_SAFE))
	{
	  --created;
	  continue;
	}
      victim = create_mobile (pMobIndex);
      char_to_room (victim, location);
      act (AT_IMMORT, "$N appears as part of an invasion force!", ch, NULL,
	   victim, TO_ROOM);
    }
  send_to_char ("The invasion was successful!\n\r", ch);

  return;
}



void
do_trust (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int level;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number (arg2))
    {
      send_to_char ("Syntax:  trust <char> <level>.\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg1)) == NULL)
    {
      send_to_char ("That player is not here.\n\r", ch);
      return;
    }
  if ((level = atoi (arg2)) < 0 || level > MAX_LEVEL)
    {
      send_to_char ("Level must be 0 (reset) or 1 to 65.\n\r", ch);
      return;
    }
  if (level > get_trust (ch))
    {
      send_to_char ("Limited to your own trust.\n\r", ch);
      return;
    }
  if (get_trust (victim) >= get_trust (ch))
    {
      send_to_char ("You can't do that.\n\r", ch);
      return;
    }

  victim->trust = level;
  send_to_char ("Ok.\n\r", ch);
  return;
}

/* Summer 1997 --Blod */
void
do_scatter (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *pRoomIndex;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Scatter whom?\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (victim == ch)
    {
      send_to_char ("It's called teleport.  Try it.\n\r", ch);
      return;
    }
  if (!IS_NPC (victim) && get_trust (victim) >= get_trust (ch))
    {
      send_to_char ("You haven't the power to succeed against them.\n\r", ch);
      return;
    }
  for (;;)
    {
      pRoomIndex = get_room_index (number_range (0, MAX_VNUM));
      if (pRoomIndex)
	if (!xIS_SET (pRoomIndex->room_flags, ROOM_PRIVATE)
	    && !xIS_SET (pRoomIndex->room_flags, ROOM_SOLITARY)
	    && !xIS_SET (pRoomIndex->room_flags, ROOM_NO_ASTRAL)
	    && !xIS_SET (pRoomIndex->room_flags, ROOM_PROTOTYPE))
	  break;
    }
  if (victim->fighting)
    stop_fighting (victim, TRUE);
  act (AT_MAGIC, "With the sweep of an arm, $n flings $N to the winds.", ch,
       NULL, victim, TO_NOTVICT);
  act (AT_MAGIC,
       "With the sweep of an arm, $n flings you to the astral winds.", ch,
       NULL, victim, TO_VICT);
  act (AT_MAGIC,
       "With the sweep of an arm, you fling $N to the astral winds.", ch,
       NULL, victim, TO_CHAR);
  char_from_room (victim);
  char_to_room (victim, pRoomIndex);
  victim->position = POS_RESTING;
  act (AT_MAGIC,
       "$n staggers forth from a sudden gust of wind, and collapses.", victim,
       NULL, NULL, TO_ROOM);
  do_look (victim, "auto");
  return;
}

void
do_strew (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj_next;
  OBJ_DATA *obj_lose;
  ROOM_INDEX_DATA *pRoomIndex;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (arg1[0] == '\0' || arg2[0] == '\0')
    {
      send_to_char ("Strew who, what?\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg1)) == NULL)
    {
      send_to_char ("It would work better if they were here.\n\r", ch);
      return;
    }
  if (victim == ch)
    {
      send_to_char ("Try taking it out on someone else first.\n\r", ch);
      return;
    }
  if (!IS_NPC (victim) && get_trust (victim) >= get_trust (ch))
    {
      send_to_char ("You haven't the power to succeed against them.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "coins"))
    {
      if (victim->gold < 1)
	{
	  send_to_char ("Drat, this one's got no gold to start with.\n\r",
			ch);
	  return;
	}
      victim->gold = 0;
      act (AT_MAGIC,
	   "$n gestures and an unearthly gale sends $N's coins flying!", ch,
	   NULL, victim, TO_NOTVICT);
      act (AT_MAGIC,
	   "You gesture and an unearthly gale sends $N's coins flying!", ch,
	   NULL, victim, TO_CHAR);
      act (AT_MAGIC,
	   "As $n gestures, an unearthly gale sends your currency flying!",
	   ch, NULL, victim, TO_VICT);
      return;
    }
  for (;;)
    {
      pRoomIndex = get_room_index (number_range (0, MAX_VNUM));
      if (pRoomIndex)
	if (!xIS_SET (pRoomIndex->room_flags, ROOM_PRIVATE)
	    && !xIS_SET (pRoomIndex->room_flags, ROOM_SOLITARY)
	    && !xIS_SET (pRoomIndex->room_flags, ROOM_NO_ASTRAL)
	    && !xIS_SET (pRoomIndex->room_flags, ROOM_PROTOTYPE))
	  break;
    }
  if (!str_cmp (arg2, "inventory"))
    {
      act (AT_MAGIC,
	   "$n speaks a single word, sending $N's possessions flying!", ch,
	   NULL, victim, TO_NOTVICT);
      act (AT_MAGIC,
	   "You speak a single word, sending $N's possessions flying!", ch,
	   NULL, victim, TO_CHAR);
      act (AT_MAGIC,
	   "$n speaks a single word, sending your possessions flying!", ch,
	   NULL, victim, TO_VICT);
      for (obj_lose = victim->first_carrying; obj_lose; obj_lose = obj_next)
	{
	  obj_next = obj_lose->next_content;
	  obj_from_char (obj_lose);
	  obj_to_room (obj_lose, pRoomIndex);
	  pager_printf_color (ch, "\t&w%s sent to %d\n\r",
			      capitalize (obj_lose->short_descr),
			      pRoomIndex->vnum);
	}
      return;
    }
  send_to_char ("Strew their coins or inventory?\n\r", ch);
  return;
}

void
do_strip (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj_next;
  OBJ_DATA *obj_lose;
  int count = 0;

  set_char_color (AT_OBJECT, ch);
  if (!argument)
    {
      send_to_char ("Strip who?\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, argument)) == NULL)
    {
      send_to_char ("They're not here.\n\r", ch);
      return;
    }
  if (victim == ch)
    {
      send_to_char ("Kinky.\n\r", ch);
      return;
    }
  if (!IS_NPC (victim) && get_trust (victim) >= get_trust (ch))
    {
      send_to_char ("You haven't the power to succeed against them.\n\r", ch);
      return;
    }
  act (AT_OBJECT, "Searching $N ...", ch, NULL, victim, TO_CHAR);
  for (obj_lose = victim->first_carrying; obj_lose; obj_lose = obj_next)
    {
      obj_next = obj_lose->next_content;
      obj_from_char (obj_lose);
      obj_to_char (obj_lose, ch);
      pager_printf_color (ch, "  &G... %s (&g%s) &Gtaken.\n\r",
			  capitalize (obj_lose->short_descr), obj_lose->name);
      count++;
    }
  if (!count)
    pager_printf_color (ch, "&GNothing found to take.\n\r", ch);
  return;
}

void
do_restore (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  bool boost = FALSE;
  bool supah_boost = FALSE;
  DEITY_DATA *deity = NULL;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Restore whom?\n\r", ch);
      return;
    }
  // Restore-by-deity. -- Alty
  if (!str_cmp (arg, "deity"))
    {
      argument = one_argument (argument, arg);
      if (!(deity = get_deity (arg)))
	{
	  send_to_char ("No such deity holds weight on this world.\n\r", ch);
	  return;
	}
    }
  argument = one_argument (argument, arg2);
  if (!str_cmp (arg2, "boost") && get_trust (ch) >= LEVEL_SUB_IMPLEM)
    {
      if (!str_cmp (ch->name, "Blodkai"))
	supah_boost = TRUE;
      else
	boost = TRUE;
      send_to_char ("Boosting!\n\r", ch);
    }
  if (deity || !str_cmp (arg, "all"))
    {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      if (!ch->pcdata)
	return;

      if (get_trust (ch) < LEVEL_SUB_IMPLEM)
	{
	  if (IS_NPC (ch))
	    {
	      send_to_char ("You can't do that.\n\r", ch);
	      return;
	    }
	  else
	    {
	      /* Check if the player did a restore all within the last 18 hours. */
	      if (current_time - last_restore_all_time < RESTORE_INTERVAL)
		{
		  send_to_char ("Sorry, you can't do a restore all yet.\n\r",
				ch);
		  do_restoretime (ch, "");
		  return;
		}
	    }
	}
      last_restore_all_time = current_time;
      ch->pcdata->restore_time = current_time;
      save_char_obj (ch);
      send_to_char ("Beginning 'restore all' ...\n\r", ch);
      for (vch = first_char; vch; vch = vch_next)
	{
	  vch_next = vch->next;

	  if (!IS_NPC (vch) && !IS_IMMORTAL (vch)
	      && !CAN_PKILL (vch) && !in_arena (vch))
	    {
	      if (deity && ch->pcdata->deity != deity)
		continue;
	      if (supah_boost)
		vch->hit = vch->max_hit * 2.2;
	      else if (boost)
		vch->hit = vch->max_hit * 1.5;
	      else
		vch->hit = vch->max_hit;
	      if (supah_boost)
		vch->mana = vch->max_mana * 2.2;
	      else
		vch->mana = vch->max_mana;
	      vch->move = vch->max_move;
	      vch->pcdata->condition[COND_BLOODTHIRST] = (10 + vch->level);
	      update_pos (vch);
	      act (AT_IMMORT, "$n has restored you.", ch, NULL, vch, TO_VICT);
	    }
	}
      send_to_char ("Restored.\n\r", ch);
    }
  else
    {

      CHAR_DATA *victim;

      if ((victim = get_char_world (ch, arg)) == NULL)
	{
	  send_to_char ("They aren't here.\n\r", ch);
	  return;
	}

      if (get_trust (ch) < LEVEL_LESSER
	  && victim != ch
	  && !(IS_NPC (victim) && xIS_SET (victim->act, ACT_PROTOTYPE)))
	{
	  send_to_char ("You can't do that.\n\r", ch);
	  return;
	}

      if (boost)
	victim->hit = victim->max_hit * 1.5;
      else
	victim->hit = victim->max_hit;
      victim->mana = victim->max_mana;
      victim->move = victim->max_move;
      if (victim->pcdata)
	victim->pcdata->condition[COND_BLOODTHIRST] = (10 + victim->level);
      update_pos (victim);
      if (ch != victim)
	act (AT_IMMORT, "$n has restored you.", ch, NULL, victim, TO_VICT);
      send_to_char ("Restored.\n\r", ch);
      return;
    }
}

void
do_restoretime (CHAR_DATA * ch, char *argument)
{
  long int time_passed;
  int hour, minute;

  set_char_color (AT_IMMORT, ch);

  if (!last_restore_all_time)
    ch_printf (ch, "There has been no restore all since reboot.\n\r");
  else
    {
      time_passed = current_time - last_restore_all_time;
      hour = (int) (time_passed / 3600);
      minute = (int) ((time_passed - (hour * 3600)) / 60);
      ch_printf (ch,
		 "The  last restore all was %d hours and %d minutes ago.\n\r",
		 hour, minute);
    }

  if (!ch->pcdata)
    return;

  if (!ch->pcdata->restore_time)
    {
      send_to_char ("You have never done a restore all.\n\r", ch);
      return;
    }

  time_passed = current_time - ch->pcdata->restore_time;
  hour = (int) (time_passed / 3600);
  minute = (int) ((time_passed - (hour * 3600)) / 60);
  ch_printf (ch, "Your last restore all was %d hours and %d minutes ago.\n\r",
	     hour, minute);
  return;
}

void
do_nohomepage (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Nohomepage whom?\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }
  if (get_trust (victim) >= get_trust (ch))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  set_char_color (AT_IMMORT, victim);
  if (xIS_SET (victim->act, PLR_NOHOMEPAGE))
    {
      xREMOVE_BIT (victim->act, PLR_NOHOMEPAGE);
      send_to_char ("You can set your own homepage again.\n\r", victim);
      ch_printf (ch, "NOHOMEPAGE removed from %s.\n\r", victim->name);
    }
  else
    {
      xSET_BIT (victim->act, PLR_NOHOMEPAGE);
      STRFREE (victim->pcdata->homepage);
      victim->pcdata->homepage = STRALLOC ("");
      if (!victim->desc)
	add_loginmsg (victim->name, 12, NULL);
      else
	send_to_char ("You can't set your own homepage!\n\r", victim);
      ch_printf (ch, "NOHOMEPAGE set on %s.\n\r", victim->name);
    }
  return;
}

/*
 * Prevent a player from creating a desc and clear offending desc -Shaddai
 */
void
do_nodesc (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Nodesc whom?\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }
  if (get_trust (victim) >= get_trust (ch))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  set_char_color (AT_IMMORT, victim);
  if (IS_SET (victim->pcdata->flags, PCFLAG_NODESC))
    {
      REMOVE_BIT (victim->pcdata->flags, PCFLAG_NODESC);
      send_to_char ("You can set your own description again.\n\r", victim);
      ch_printf (ch, "NODESC removed from %s.\n\r", victim->name);
    }
  else
    {
      SET_BIT (victim->pcdata->flags, PCFLAG_NODESC);
      STRFREE (victim->description);
      victim->description = STRALLOC ("");
      if (!victim->desc)
	add_loginmsg (victim->name, 11, NULL);
      else
	send_to_char ("You can't set your own description!\n\r", victim);
      ch_printf (ch, "NODESC set on %s.\n\r", victim->name);
    }
  return;
}

void
do_nohttp (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Nohttp whom?\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }
  if (get_trust (victim) >= get_trust (ch))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  set_char_color (AT_IMMORT, victim);
  if (IS_SET (victim->pcdata->flags, PCFLAG_NOHTTP))
    {
      REMOVE_BIT (victim->pcdata->flags, PCFLAG_NOHTTP);
      send_to_char ("You can set your own home page again.\n\r", victim);
      ch_printf (ch, "NOHTTP removed from %s.\n\r", victim->name);
    }
  else
    {
      SET_BIT (victim->pcdata->flags, PCFLAG_NOHTTP);
      STRFREE (victim->pcdata->homepage);
      victim->pcdata->homepage = STRALLOC ("");
      if (!victim->desc)
	add_loginmsg (victim->name, 12, NULL);
      else
	send_to_char ("You can't set your own home page!\n\r", victim);
      ch_printf (ch, "NOHTTP set on %s.\n\r", victim->name);
    }
  return;
}


/*
 * Prevent a player from creating a bio and clear offending bio -Shaddai
 */
void
do_nobio (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Nobio whom?\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }
  if (get_trust (victim) >= get_trust (ch))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  set_char_color (AT_IMMORT, victim);
  if (IS_SET (victim->pcdata->flags, PCFLAG_NOBIO))
    {
      REMOVE_BIT (victim->pcdata->flags, PCFLAG_NOBIO);
      send_to_char ("You can set your own bio again.\n\r", victim);
      ch_printf (ch, "NOBIO removed from %s.\n\r", victim->name);
    }
  else
    {
      SET_BIT (victim->pcdata->flags, PCFLAG_NOBIO);
      STRFREE (victim->pcdata->bio);
      victim->pcdata->bio = STRALLOC ("");
      if (!victim->desc)
	add_loginmsg (victim->name, 9, NULL);
      else
	send_to_char ("You can't set your own bio!\n\r", victim);
      ch_printf (ch, "NOBIO set on %s.\n\r", victim->name);
    }
  return;
}


/* Guess I should have added this to begin with, huh? - Blod, 12/2000 */
void
do_nobeckon (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Use nobeckon on who?\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }
  set_char_color (AT_IMMORT, victim);
  if (get_trust (victim) >= get_trust (ch))
    {
      send_to_char
	("Their godly glow prevents you from getting close enough.\n\r", ch);
      return;
    }
  if (IS_SET (victim->pcdata->flags, PCFLAG_NOBECKON))
    {
      REMOVE_BIT (victim->pcdata->flags, PCFLAG_NOBECKON);
      send_to_char ("The gods return your ability to beckon.\n\r", victim);
      ch_printf (ch, "%s can again beckon.\n\r", victim->name);
    }
  else
    {
      SET_BIT (victim->pcdata->flags, PCFLAG_NOBECKON);
      if (!victim->desc)
	add_loginmsg (victim->name, 13, NULL);
      else
	send_to_char ("The gods have removed your ability to beckon.\n\r",
		      victim);
      ch_printf (ch, "You have removed beckon from %s.\n\r", victim->name);
    }
  save_char_obj (victim);
  return;
}

void
do_freeze (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_LBLUE, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Freeze whom?\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }
  set_char_color (AT_LBLUE, victim);
  if (get_trust (victim) >= get_trust (ch))
    {
      send_to_char ("You failed, and they saw...\n\r", ch);
      ch_printf (victim, "%s is attempting to freeze you.\n\r", ch->name);
      return;
    }
  if (xIS_SET (victim->act, PLR_FREEZE))
    {
      xREMOVE_BIT (victim->act, PLR_FREEZE);
      send_to_char ("Your frozen form suddenly thaws.\n\r", victim);
      ch_printf (ch, "%s is now unfrozen.\n\r", victim->name);
    }
  else
    {
      xSET_BIT (victim->act, PLR_FREEZE);
      if (!victim->desc)
	add_loginmsg (victim->name, 15, NULL);
      else
	send_to_char ("A godly force turns your body to ice!\n\r", victim);
      ch_printf (ch, "You have frozen %s.\n\r", victim->name);
    }
  save_char_obj (victim);
  return;
}

void
do_log (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Log whom?\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "all"))
    {
      if (fLogAll)
	{
	  fLogAll = FALSE;
	  send_to_char ("Log ALL off.\n\r", ch);
	}
      else
	{
	  fLogAll = TRUE;
	  send_to_char ("Log ALL on.\n\r", ch);
	}
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }

  /*
   * No level check, gods can log anyone.
   */

  if (xIS_SET (victim->act, PLR_LOG))
    {
      xREMOVE_BIT (victim->act, PLR_LOG);
      ch_printf (ch, "LOG removed from %s.\n\r", victim->name);
    }
  else
    {
      xSET_BIT (victim->act, PLR_LOG);
      ch_printf (ch, "LOG applied to %s.\n\r", victim->name);
    }
  return;
}

void
do_litterbug (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Set litterbug flag on whom?\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }
  if (get_trust (victim) >= get_trust (ch))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  set_char_color (AT_IMMORT, victim);
  if (xIS_SET (victim->act, PLR_LITTERBUG))
    {
      xREMOVE_BIT (victim->act, PLR_LITTERBUG);
      send_to_char ("You can drop items again.\n\r", victim);
      ch_printf (ch, "LITTERBUG removed from %s.\n\r", victim->name);
    }
  else
    {
      xSET_BIT (victim->act, PLR_LITTERBUG);
      send_to_char
	("A strange force prevents you from dropping any more items!\n\r",
	 victim);
      ch_printf (ch, "LITTERBUG set on %s.\n\r", victim->name);
    }
  return;
}

void
do_noemote (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Noemote whom?\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }
  if (get_trust (victim) >= get_trust (ch))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  set_char_color (AT_IMMORT, victim);
  if (xIS_SET (victim->act, PLR_NO_EMOTE))
    {
      xREMOVE_BIT (victim->act, PLR_NO_EMOTE);
      send_to_char ("You can emote again.\n\r", victim);
      ch_printf (ch, "NOEMOTE removed from %s.\n\r", victim->name);
    }
  else
    {
      xSET_BIT (victim->act, PLR_NO_EMOTE);
      if (!victim->desc)
	add_loginmsg (victim->name, 16, NULL);
      else
	send_to_char ("You can't emote!\n\r", victim);
      ch_printf (ch, "NOEMOTE applied to %s.\n\r", victim->name);
    }
  return;
}

void
do_notell (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Notell whom?\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }
  if (get_trust (victim) >= get_trust (ch))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  set_char_color (AT_IMMORT, victim);
  if (xIS_SET (victim->act, PLR_NO_TELL))
    {
      xREMOVE_BIT (victim->act, PLR_NO_TELL);
      send_to_char ("You can use tells again.\n\r", victim);
      ch_printf (ch, "NOTELL removed from %s.\n\r", victim->name);
    }
  else
    {
      xSET_BIT (victim->act, PLR_NO_TELL);
      if (!victim->desc)
	add_loginmsg (victim->name, 14, NULL);
      else
	send_to_char ("You can't use tells!\n\r", victim);
      ch_printf (ch, "NOTELL applied to %s.\n\r", victim->name);
    }
  return;
}

void
do_notitle (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Notitle whom?\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }
  if (get_trust (victim) >= get_trust (ch))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  set_char_color (AT_IMMORT, victim);
  if (IS_SET (victim->pcdata->flags, PCFLAG_NOTITLE))
    {
      REMOVE_BIT (victim->pcdata->flags, PCFLAG_NOTITLE);
      send_to_char ("You can set your own title again.\n\r", victim);
      ch_printf (ch, "NOTITLE removed from %s.\n\r", victim->name);
    }
  else
    {
      SET_BIT (victim->pcdata->flags, PCFLAG_NOTITLE);
      sprintf (buf, "the %s",
	       title_table[victim->class][victim->level]
	       [victim->sex == SEX_FEMALE ? 1 : 0]);
      set_title (victim, buf);
      if (!victim->desc)
	add_loginmsg (victim->name, 8, NULL);
      else
	send_to_char ("You can't set your own title!\n\r", victim);
      ch_printf (ch, "NOTITLE set on %s.\n\r", victim->name);
    }
  return;
}

void
do_silence (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Silence whom?", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }
  if (get_trust (victim) >= get_trust (ch))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  set_char_color (AT_IMMORT, victim);
  if (xIS_SET (victim->act, PLR_SILENCE))
    {
      send_to_char ("Player already silenced, use unsilence to remove.\n\r",
		    ch);
    }
  else
    {
      xSET_BIT (victim->act, PLR_SILENCE);
      if (!victim->desc)
	add_loginmsg (victim->name, 7, NULL);
      else
	send_to_char ("You can't use channels!\n\r", victim);
      ch_printf (ch, "You SILENCE %s.\n\r", victim->name);
    }
  return;
}

/* Much better than toggling this with do_silence, yech --Blodkai */
void
do_unsilence (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Unsilence whom?\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }
  if (get_trust (victim) >= get_trust (ch))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  set_char_color (AT_IMMORT, victim);
  if (xIS_SET (victim->act, PLR_SILENCE))
    {
      xREMOVE_BIT (victim->act, PLR_SILENCE);
      send_to_char ("You can use channels again.\n\r", victim);
      ch_printf (ch, "SILENCE removed from %s.\n\r", victim->name);
    }
  else
    {
      send_to_char ("That player is not silenced.\n\r", ch);
    }
  return;
}

void
do_peace (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *rch;

  act (AT_IMMORT, "$n booms, 'PEACE!'", ch, NULL, NULL, TO_ROOM);
  act (AT_IMMORT, "You boom, 'PEACE!'", ch, NULL, NULL, TO_CHAR);
  for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room)
    {
      if (rch->fighting)
	{
	  stop_fighting (rch, TRUE);
	  do_sit (rch, "");
	}

      /* Added by Narn, Nov 28/95 */
      stop_hating (rch);
      stop_hunting (rch);
      stop_fearing (rch);
    }

  send_to_char_color ("&YOk.\n\r", ch);
  return;
}

WATCH_DATA *first_watch;
WATCH_DATA *last_watch;

void
save_watchlist (void)
{
  WATCH_DATA *pwatch;
  FILE *fp;

  fclose (fpReserve);
  if (!(fp = fopen (SYSTEM_DIR WATCH_LIST, "w")))
    {
      bug ("Save_watchlist: Cannot open " WATCH_LIST, 0);
      perror (WATCH_LIST);
      fpReserve = fopen (NULL_FILE, "r");
      return;
    }

  for (pwatch = first_watch; pwatch; pwatch = pwatch->next)
    fprintf (fp, "%d %s~%s~%s~\n", pwatch->imm_level, pwatch->imm_name,
	     pwatch->target_name ? pwatch->target_name : " ",
	     pwatch->player_site ? pwatch->player_site : " ");
  fprintf (fp, "-1\n");
  fclose (fp);
  fpReserve = fopen (NULL_FILE, "r");
  return;
}

void
do_wizlock (CHAR_DATA * ch, char *argument)
{
  extern bool wizlock;
  wizlock = !wizlock;

  set_char_color (AT_DANGER, ch);

  if (wizlock)
    send_to_char ("Game wizlocked.\n\r", ch);
  else
    send_to_char ("Game un-wizlocked.\n\r", ch);
  return;
}

void
do_noresolve (CHAR_DATA * ch, char *argument)
{
  sysdata.NO_NAME_RESOLVING = !sysdata.NO_NAME_RESOLVING;

  if (sysdata.NO_NAME_RESOLVING)
    send_to_char_color ("&YName resolving disabled.\n\r", ch);
  else
    send_to_char_color ("&YName resolving enabled.\n\r", ch);
  return;
}

void
do_users (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  int count;
  char arg[MAX_INPUT_LENGTH];

  set_pager_color (AT_PLAIN, ch);

  one_argument (argument, arg);
  count = 0;
  buf[0] = '\0';

  sprintf (buf, "\n\rDesc|Con|Idle| Port | Player      @HostIP           ");
  if (get_trust (ch) >= LEVEL_GOD)
    strcat (buf, "| Username");
  strcat (buf, "\n\r");
  strcat (buf, "----+---+----+------+-------------------------------");
  if (get_trust (ch) >= LEVEL_GOD)
    strcat (buf, "+---------");
  strcat (buf, "\n\r");
  send_to_pager (buf, ch);

  for (d = first_descriptor; d; d = d->next)
    {
      if (arg[0] == '\0')
	{
	  if (get_trust (ch) >= LEVEL_SUPREME
	      || (d->character && can_see (ch, d->character)))
	    {
	      count++;
	      sprintf (buf,
		       " %3d| %2d|%4d|%6d| %-12s@%-16s ",
		       d->descriptor,
		       d->connected,
		       d->idle / 4,
		       d->port,
		       d->original ? d->original->name :
		       d->character ? d->character->name : "(none)", d->host);
	      if (get_trust (ch) >= LEVEL_GOD)
		sprintf (buf + strlen (buf), "| %s", d->user);
	      strcat (buf, "\n\r");
	      send_to_pager (buf, ch);
	    }
	}
      else
	{
	  if ((get_trust (ch) >= LEVEL_SUPREME
	       || (d->character && can_see (ch, d->character)))
	      && (!str_prefix (arg, d->host)
		  || (d->character && !str_prefix (arg, d->character->name))))
	    {
	      count++;
	      pager_printf (ch,
			    " %3d| %2d|%4d|%6d| %-12s@%-16s ",
			    d->descriptor,
			    d->connected,
			    d->idle / 4,
			    d->port,
			    d->original ? d->original->name :
			    d->character ? d->character->name : "(none)",
			    d->host);
	      buf[0] = '\0';
	      if (get_trust (ch) >= LEVEL_GOD)
		sprintf (buf, "| %s", d->user);
	      strcat (buf, "\n\r");
	      send_to_pager (buf, ch);
	    }
	}
    }
  pager_printf (ch, "%d user%s.\n\r", count, count == 1 ? "" : "s");
  return;
}

/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void
do_force (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  bool mobsonly;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg);
  if (arg[0] == '\0' || argument[0] == '\0')
    {
      send_to_char ("Force whom to do what?\n\r", ch);
      return;
    }

  mobsonly = get_trust (ch) < sysdata.level_forcepc;

  if (!str_cmp (arg, "all"))
    {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      if (mobsonly)
	{
	  send_to_char ("Force whom to do what?\n\r", ch);
	  return;
	}

      for (vch = first_char; vch; vch = vch_next)
	{
	  vch_next = vch->next;

	  if (!IS_NPC (vch) && get_trust (vch) < get_trust (ch))
	    {
	      act (AT_IMMORT, "$n forces you to '$t'.", ch, argument, vch,
		   TO_VICT);
	      interpret (vch, argument);
	    }
	}
    }
  else
    {
      CHAR_DATA *victim;

      if ((victim = get_char_world (ch, arg)) == NULL)
	{
	  send_to_char ("They aren't here.\n\r", ch);
	  return;
	}

      if (victim == ch)
	{
	  send_to_char ("Aye aye, right away!\n\r", ch);
	  return;
	}

      if ((get_trust (victim) >= get_trust (ch))
	  || (mobsonly && !IS_NPC (victim)))
	{
	  send_to_char ("Do it yourself!\n\r", ch);
	  return;
	}

      if (get_trust (ch) < LEVEL_GOD && IS_NPC (victim)
	  && !str_prefix ("mp", argument))
	{
	  send_to_char ("You can't force a mob to do that!\n\r", ch);
	  return;
	}
      act (AT_IMMORT, "$n forces you to '$t'.", ch, argument, victim,
	   TO_VICT);
      interpret (victim, argument);
    }

  send_to_char ("Ok.\n\r", ch);
  return;
}

void
do_invis (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  sh_int level;

  set_char_color (AT_IMMORT, ch);

  /* if ( IS_NPC(ch)) return; */

  argument = one_argument (argument, arg);
  if (arg && arg[0] != '\0')
    {
      if (!is_number (arg))
	{
	  send_to_char ("Usage: invis | invis <level>\n\r", ch);
	  return;
	}
      level = atoi (arg);
      if (level < 2 || level > get_trust (ch))
	{
	  send_to_char ("Invalid level.\n\r", ch);
	  return;
	}

      if (!IS_NPC (ch))
	{
	  ch->pcdata->wizinvis = level;
	  ch_printf (ch, "Wizinvis level set to %d.\n\r", level);
	}

      if (IS_NPC (ch))
	{
	  ch->mobinvis = level;
	  ch_printf (ch, "Mobinvis level set to %d.\n\r", level);
	}
      return;
    }

  if (!IS_NPC (ch))
    {
      if (ch->pcdata->wizinvis < 2)
	ch->pcdata->wizinvis = ch->level;
    }
  if (IS_NPC (ch))
    {
      if (ch->mobinvis < 2)
	ch->mobinvis = ch->level;
    }
  if (xIS_SET (ch->act, PLR_WIZINVIS))
    {
      xREMOVE_BIT (ch->act, PLR_WIZINVIS);
      act (AT_IMMORT, "$n slowly fades into existence.", ch, NULL, NULL,
	   TO_ROOM);
      send_to_char ("You slowly fade back into existence.\n\r", ch);
    }
  else
    {
      act (AT_IMMORT, "$n slowly fades into thin air.", ch, NULL, NULL,
	   TO_ROOM);
      send_to_char ("You slowly vanish into thin air.\n\r", ch);
      xSET_BIT (ch->act, PLR_WIZINVIS);
    }
  return;
}


void
do_holylight (CHAR_DATA * ch, char *argument)
{

  set_char_color (AT_IMMORT, ch);

  if (IS_NPC (ch))
    return;

  if (xIS_SET (ch->act, PLR_HOLYLIGHT))
    {
      xREMOVE_BIT (ch->act, PLR_HOLYLIGHT);
      send_to_char ("Holy light mode off.\n\r", ch);
    }
  else
    {
      xSET_BIT (ch->act, PLR_HOLYLIGHT);
      send_to_char ("Holy light mode on.\n\r", ch);
    }
  return;
}

/*
 * Make yourself visible to people in the same room as you	-Thoric
 */
void
do_roomvis (CHAR_DATA * ch, char *argument)
{
  set_char_color (AT_IMMORT, ch);

  if (IS_NPC (ch))
    return;

  if (xIS_SET (ch->act, PLR_ROOMVIS))
    {
      xREMOVE_BIT (ch->act, PLR_ROOMVIS);
      send_to_char
	("You are no longer visible to people in the same room as you.\n\r",
	 ch);
    }
  else
    {
      xSET_BIT (ch->act, PLR_ROOMVIS);
      send_to_char
	("You are now visible to people who are in the same room as you.\n\r",
	 ch);
    }
  return;
}

void
do_rassign (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  int r_lo, r_hi;
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);
  r_lo = atoi (arg2);
  r_hi = atoi (arg3);

  if (arg1[0] == '\0' || r_lo < 0 || r_hi < 0)
    {
      send_to_char ("Syntax: rassign <who> <low> <high>\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg1)) == NULL)
    {
      send_to_char ("They don't seem to be around.\n\r", ch);
      return;
    }
  if (IS_NPC (victim) || get_trust (victim) < LEVEL_CREATOR)
    {
      send_to_char ("They wouldn't know what to do with a room range.\n\r",
		    ch);
      return;
    }
  if (r_lo > r_hi)
    {
      send_to_char ("Unacceptable room range.\n\r", ch);
      return;
    }
  if (r_lo == 0)
    r_hi = 0;
  victim->pcdata->r_range_lo = r_lo;
  victim->pcdata->r_range_hi = r_hi;
  assign_area (victim);
  send_to_char ("Done.\n\r", ch);
  set_char_color (AT_IMMORT, victim);
  ch_printf (victim, "%s has assigned you the room vnum range %d - %d.\n\r",
	     ch->name, r_lo, r_hi);
  assign_area (victim);		/* Put back by Thoric on 02/07/96 */
  if (!victim->pcdata->area)
    {
      bug ("rassign: assign_area failed", 0);
      return;
    }

  if (r_lo == 0)		/* Scryn 8/12/95 */
    {
      REMOVE_BIT (victim->pcdata->area->status, AREA_LOADED);
      SET_BIT (victim->pcdata->area->status, AREA_DELETED);
    }
  else
    {
      SET_BIT (victim->pcdata->area->status, AREA_LOADED);
      REMOVE_BIT (victim->pcdata->area->status, AREA_DELETED);
    }
  return;
}

void
do_oassign (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  int o_lo, o_hi;
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);
  o_lo = atoi (arg2);
  o_hi = atoi (arg3);

  if (arg1[0] == '\0' || o_lo < 0 || o_hi < 0)
    {
      send_to_char ("Syntax: oassign <who> <low> <high>\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg1)) == NULL)
    {
      send_to_char ("They don't seem to be around.\n\r", ch);
      return;
    }
  if (IS_NPC (victim) || get_trust (victim) < LEVEL_SAVIOR)
    {
      send_to_char ("They wouldn't know what to do with an object range.\n\r",
		    ch);
      return;
    }
  if (o_lo > o_hi)
    {
      send_to_char ("Unacceptable object range.\n\r", ch);
      return;
    }
  victim->pcdata->o_range_lo = o_lo;
  victim->pcdata->o_range_hi = o_hi;
  assign_area (victim);
  send_to_char ("Done.\n\r", ch);
  set_char_color (AT_IMMORT, victim);
  ch_printf (victim, "%s has assigned you the object vnum range %d - %d.\n\r",
	     ch->name, o_lo, o_hi);
  return;
}

void
do_massign (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  int m_lo, m_hi;
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);
  m_lo = atoi (arg2);
  m_hi = atoi (arg3);

  if (arg1[0] == '\0' || m_lo < 0 || m_hi < 0)
    {
      send_to_char ("Syntax: massign <who> <low> <high>\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg1)) == NULL)
    {
      send_to_char ("They don't seem to be around.\n\r", ch);
      return;
    }
  if (IS_NPC (victim) || get_trust (victim) < LEVEL_SAVIOR)
    {
      send_to_char ("They wouldn't know what to do with a monster range.\n\r",
		    ch);
      return;
    }
  if (m_lo > m_hi)
    {
      send_to_char ("Unacceptable monster range.\n\r", ch);
      return;
    }
  victim->pcdata->m_range_lo = m_lo;
  victim->pcdata->m_range_hi = m_hi;
  assign_area (victim);
  send_to_char ("Done.\n\r", ch);
  set_char_color (AT_IMMORT, victim);
  ch_printf (victim,
	     "%s has assigned you the monster vnum range %d - %d.\n\r",
	     ch->name, m_lo, m_hi);
  return;
}

void
do_cmdtable (CHAR_DATA * ch, char *argument)
{
  int hash, cnt;
  CMDTYPE *cmd;
  char arg[MAX_INPUT_LENGTH];

  one_argument (argument, arg);

  if (strcmp (arg, "lag"))	/* display normal command table */
    {
      set_pager_color (AT_IMMORT, ch);
      send_to_pager ("Commands and Number of Uses This Run\n\r", ch);
      set_pager_color (AT_PLAIN, ch);
      for (cnt = hash = 0; hash < 126; hash++)
	for (cmd = command_hash[hash]; cmd; cmd = cmd->next)
	  {
	    if ((++cnt) % 4)
	      pager_printf (ch, "%-6.6s %4d\t", cmd->name,
			    cmd->userec.num_uses);
	    else
	      pager_printf (ch, "%-6.6s %4d\n\r", cmd->name,
			    cmd->userec.num_uses);
	  }
      send_to_char ("\n\r", ch);
    }
  else				/* display commands causing lag */
    {
      set_pager_color (AT_IMMORT, ch);
      send_to_pager ("Commands that have caused lag this run\n\r", ch);
      set_pager_color (AT_PLAIN, ch);
      for (cnt = hash = 0; hash < 126; hash++)
	for (cmd = command_hash[hash]; cmd; cmd = cmd->next)
	  {
	    if (!cmd->lag_count)
	      continue;
	    else if ((++cnt) % 4)
	      pager_printf (ch, "%-6.6s %4d\t", cmd->name, cmd->lag_count);
	    else
	      pager_printf (ch, "%-6.6s %4d\n\r", cmd->name, cmd->lag_count);
	  }
      send_to_char ("\n\r", ch);
    }

  return;
}

void
do_mortalize (CHAR_DATA * ch, char *argument)
{
  char fname[1024];
  char name[256];
  struct stat fst;
  bool loaded = FALSE;
  DESCRIPTOR_DATA *d;
  int old_room_vnum, test = 0;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  CHAR_DATA *victim = NULL;
  AREA_DATA *pArea;
  int sn;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, name);
  if (name[0] == '\0')
    {
      send_to_char ("Usage: mortalize <playername>\n\r", ch);
      return;
    }

  for (victim = first_char; victim; victim = victim->next)
    {
      if (IS_NPC (victim))
	continue;
      if (!str_cmp (name, victim->name))
	break;
    }

  if (!victim)
    {
      name[0] = UPPER (name[0]);
      sprintf (fname, "%s%c/%s", PLAYER_DIR, tolower (name[0]),
	       capitalize (name));
      if ((test = stat (fname, &fst)))
	{
	  CREATE (d, DESCRIPTOR_DATA, 1);
	  d->next = NULL;
	  d->prev = NULL;
	  d->connected = CON_GET_NAME;
	  d->outsize = 2000;
	  CREATE (d->outbuf, char, d->outsize);

	  loaded = load_char_obj (d, name, FALSE);
	  add_char (d->character);
	  old_room_vnum = d->character->in_room->vnum;
	  char_to_room (d->character, ch->in_room);
	  if (get_trust (d->character) >= get_trust (ch))
	    {
	      do_say (d->character, "Do *NOT* disturb me again!");
	      send_to_char
		("I think you'd better leave that player alone!\n\r", ch);
	      d->character->desc = NULL;
	      if (loaded)
		do_quit (d->character, "");
	      return;
	    }
	  d->character->desc = NULL;
	  victim = d->character;
	  d->character = NULL;
	  DISPOSE (d->outbuf);
	  DISPOSE (d);
	}
    }
  if (test != -1 && victim)
    {
      if (get_trust (victim) >= get_trust (ch))
	{
	  send_to_char ("You failed!\n\r", ch);
	  if (loaded)
	    do_quit (victim, "");
	  return;
	}
      victim->level = 50;
      victim->exp = exp_level (victim, 50);
      victim->max_hit = 800;
      victim->max_mana = 800;
      victim->max_move = 800;
      for (sn = 0; sn < top_sn; sn++)
	victim->pcdata->learned[sn] = 0;
      victim->practice = 0;
      victim->hit = victim->max_hit;
      victim->mana = victim->max_mana;
      victim->move = victim->max_move;
      advance_level (victim);
      DISPOSE (victim->pcdata->rank);
      victim->pcdata->rank = str_dup ("");
      if (xIS_SET (victim->act, PLR_WIZINVIS))
	victim->pcdata->wizinvis = victim->trust;
      if (xIS_SET (victim->act, PLR_WIZINVIS)
	  && (victim->level <= LEVEL_AVATAR))
	{
	  xREMOVE_BIT (victim->act, PLR_WIZINVIS);
	  victim->pcdata->wizinvis = victim->trust;
	}
      sprintf (buf, "%s%s", GOD_DIR, capitalize (victim->name));

      if (!remove (buf))
	send_to_char ("Player's immortal data destroyed.\n\r", ch);
      else if (errno != ENOENT)
	{
	  ch_printf (ch,
		     "Unknown error #%d - %s (immortal data).  Report to Thoric\n\r",
		     errno, strerror (errno));
	  sprintf (buf2, "%s mortalizing %s", ch->name, buf);
	  perror (buf2);
	}
      sprintf (buf2, "%s.are", capitalize (argument));
      for (pArea = first_build; pArea; pArea = pArea->next)
	if (!strcmp (pArea->filename, buf2))
	  {
	    sprintf (buf, "%s%s", BUILD_DIR, buf2);
	    if (IS_SET (pArea->status, AREA_LOADED))
	      fold_area (pArea, buf, FALSE);
	    close_area (pArea);
	    sprintf (buf2, "%s.bak", buf);
	    set_char_color (AT_RED, ch);
	    if (!rename (buf, buf2))
	      send_to_char
		("Player's area data destroyed.  Area saved as backup.\n\r",
		 ch);
	    else if (errno != ENOENT)
	      {
		ch_printf (ch,
			   "Unknown error #%d - %s (area data).  Report to Thoric.\n\r",
			   errno, strerror (errno));
		sprintf (buf2, "%s mortalizing %s", ch->name, buf);
		perror (buf2);
	      }
	  }
      while (victim->first_carrying)
	extract_obj (victim->first_carrying);
      if (loaded)
	do_quit (victim, "");
      else
	save_char_obj (victim);
      make_wizlist ();
      make_adminlist ();
      make_retiredlist ();
      return;
    }
  send_to_char ("No such player.\n\r", ch);
  return;
}

/*
 * Load up a player file
 */
void
do_loadup (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *temp;
  char fname[1024];
  char name[256];
  struct stat fst;
  bool loaded = FALSE;
  DESCRIPTOR_DATA *d;
  int old_room_vnum;
  char buf[MAX_STRING_LENGTH];

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, name);
  if (name[0] == '\0')
    {
      send_to_char ("Usage: loadup <playername>\n\r", ch);
      return;
    }
  for (temp = first_char; temp; temp = temp->next)
    {
      if (IS_NPC (temp))
	continue;
      if (can_see (ch, temp) && !str_cmp (name, temp->name))
	break;
    }
  if (temp != NULL)
    {
      send_to_char ("They are already playing.\n\r", ch);
      return;
    }
  name[0] = UPPER (name[0]);
  sprintf (fname, "%s%c/%s", PLAYER_DIR, tolower (name[0]),
	   capitalize (name));

  if (stat (fname, &fst) != -1)
    {
      CREATE (d, DESCRIPTOR_DATA, 1);
      d->next = NULL;
      d->prev = NULL;
      d->connected = CON_GET_NAME;
      d->outsize = 2000;
      CREATE (d->outbuf, char, d->outsize);

      loaded = load_char_obj (d, name, FALSE);
      add_char (d->character);
      old_room_vnum = d->character->in_room->vnum;
      char_to_room (d->character, ch->in_room);
      if (get_trust (d->character) >= get_trust (ch))
	{
	  do_say (d->character, "Do *NOT* disturb me again!");
	  send_to_char ("I think you'd better leave that player alone!\n\r",
			ch);
	  d->character->desc = NULL;
	  if (loaded)
	    do_quit (d->character, "");
	  return;
	}
      d->character->desc = NULL;
      d->character->retran = old_room_vnum;
      d->character = NULL;
      DISPOSE (d->outbuf);
      DISPOSE (d);
      ch_printf (ch, "Player %s loaded from room %d.\n\r", capitalize (name),
		 old_room_vnum);
      sprintf (buf, "%s appears from nowhere, eyes glazed over.\n\r",
	       capitalize (name));
      act (AT_IMMORT, buf, ch, NULL, NULL, TO_ROOM);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  /* else no player file */
  send_to_char ("No such player.\n\r", ch);
  return;
}

void
do_fixchar (CHAR_DATA * ch, char *argument)
{
  char name[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, name);
  if (name[0] == '\0')
    {
      send_to_char ("Usage: fixchar <playername>\n\r", ch);
      return;
    }

  victim = get_char_room (ch, name);
  if (!victim)
    {
      send_to_char ("They're not here.\n\r", ch);
      return;
    }
  fix_char (victim);
/*  victim->armor	= 100;
    victim->mod_str	= 0;
    victim->mod_dex	= 0;
    victim->mod_wis	= 0;
    victim->mod_int	= 0;
    victim->mod_con	= 0;
    victim->mod_cha	= 0;
    victim->mod_lck	= 0;
    victim->damroll	= 0;
    victim->hitroll	= 0;
    victim->alignment	= URANGE( -1000, victim->alignment, 1000 );
    victim->saving_spell_staff = 0; */
  send_to_char ("Done.\n\r", ch);
}

void
do_newbieset (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (arg1[0] == '\0')
    {
      send_to_char ("Syntax: newbieset <char>.\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg1)) == NULL)
    {
      send_to_char ("That player is not here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }
  if ((victim->level < 1) || (victim->level > 5))
    {
      send_to_char ("Level of victim must be between 1 and 5.\n\r", ch);
      return;
    }

  obj = create_object (get_obj_index (OBJ_VNUM_SCHOOL_VEST), 1);
  obj_to_char (obj, victim);
  obj = create_object (get_obj_index (OBJ_VNUM_SCHOOL_SHIELD), 1);
  obj_to_char (obj, victim);
  obj = create_object (get_obj_index (OBJ_VNUM_SCHOOL_BANNER), 1);
  obj_to_char (obj, victim);

  if ((victim->class == CLASS_MAGE) || (victim->class == CLASS_THIEF)
      || (victim->class == CLASS_VAMPIRE) || (victim->class == CLASS_AUGURER))
    {
      obj = create_object (get_obj_index (OBJ_VNUM_SCHOOL_DAGGER), 1);
      obj_to_char (obj, victim);
    }
  else if ((victim->class == CLASS_CLERIC) || (victim->class == CLASS_DRUID))
    {
      obj = create_object (get_obj_index (OBJ_VNUM_SCHOOL_MACE), 1);
      obj_to_char (obj, victim);
    }
  else if ((victim->class == CLASS_WARRIOR) ||
	   (victim->class == CLASS_RANGER) ||
	   (victim->class == CLASS_PALADIN))
    {
      obj = create_object (get_obj_index (OBJ_VNUM_SCHOOL_SWORD), 1);
      obj_to_char (obj, victim);
    }

  /* Added by Brittany, on Nov. 24, 1996. The object is the adventurer's 
     guide to the realms of despair, part of academy.are. */
  {
    OBJ_INDEX_DATA *obj_ind = get_obj_index (10333);
    if (obj_ind != NULL)
      {
	obj = create_object (obj_ind, 1);
	obj_to_char (obj, victim);
      }
  }

/* Added the burlap sack to the newbieset.  The sack is part of sgate.are
   called Spectral Gate.  Brittany */

  {

    OBJ_INDEX_DATA *obj_ind = get_obj_index (123);
    if (obj_ind != NULL)
      {
	obj = create_object (obj_ind, 1);
	obj_to_char (obj, victim);
      }
  }

  act (AT_IMMORT, "$n has equipped you with a newbieset.", ch, NULL, victim,
       TO_VICT);
  ch_printf (ch, "You have re-equipped %s.\n\r", victim->name);
  return;
}

/*
 * Extract area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "joe.are susan.are"
 * - Gorog
 */
void
extract_area_names (char *inp, char *out)
{
  char buf[MAX_INPUT_LENGTH], *pbuf = buf;
  int len;

  *out = '\0';
  while (inp && *inp)
    {
      inp = one_argument (inp, buf);
      if ((len = strlen (buf)) >= 5 && !strcmp (".are", pbuf + len - 4))
	{
	  if (*out)
	    strcat (out, " ");
	  strcat (out, buf);
	}
    }
}

/*
 * Remove area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "aset sedit cset"
 * - Gorog
 */
void
remove_area_names (char *inp, char *out)
{
  char buf[MAX_INPUT_LENGTH], *pbuf = buf;
  int len;

  *out = '\0';
  while (inp && *inp)
    {
      inp = one_argument (inp, buf);
      if ((len = strlen (buf)) < 5 || strcmp (".are", pbuf + len - 4))
	{
	  if (*out)
	    strcat (out, " ");
	  strcat (out, buf);
	}
    }
}

/*
 * Allows members of the Area Council to add Area names to the bestow field.
 * Area names mus end with ".are" so that no commands can be bestowed.
 */
void
do_bestowarea (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int arg_len;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg);

  if (!*arg)
    {
      send_to_char ("Syntax:\n\r"
		    "bestowarea <victim> <filename>.are\n\r"
		    "bestowarea <victim> none             removes bestowed areas\n\r"
		    "bestowarea <victim> list             lists bestowed areas\n\r"
		    "bestowarea <victim>                  lists bestowed areas\n\r",
		    ch);
      return;
    }
  if (!(victim = get_char_world (ch, arg)))
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("You can't give special abilities to a mob!\n\r", ch);
      return;
    }
  if (get_trust (victim) < LEVEL_IMMORTAL)
    {
      send_to_char ("They aren't an immortal.\n\r", ch);
      return;
    }

  if (!victim->pcdata->bestowments)
    victim->pcdata->bestowments = str_dup ("");

  if (!*argument || !str_cmp (argument, "list"))
    {
      extract_area_names (victim->pcdata->bestowments, buf);
      ch_printf (ch, "Bestowed areas: %s\n\r", buf);
      return;
    }
  if (!str_cmp (argument, "none"))
    {
      remove_area_names (victim->pcdata->bestowments, buf);
      DISPOSE (victim->pcdata->bestowments);
      victim->pcdata->bestowments = str_dup (buf);
      send_to_char ("Done.\n\r", ch);
      return;
    }

  arg_len = strlen (argument);
  if (arg_len < 5
      || argument[arg_len - 4] != '.' || argument[arg_len - 3] != 'a'
      || argument[arg_len - 2] != 'r' || argument[arg_len - 1] != 'e')
    {
      send_to_char ("You can only bestow an area name\n\r", ch);
      send_to_char ("E.G. bestow joe sam.are\n\r", ch);
      return;
    }

  sprintf (buf, "%s %s", victim->pcdata->bestowments, argument);
  DISPOSE (victim->pcdata->bestowments);
  victim->pcdata->bestowments = str_dup (buf);
  set_char_color (AT_IMMORT, victim);
  ch_printf (victim, "%s has bestowed on you the area: %s\n\r",
	     ch->name, argument);
  send_to_char ("Done.\n\r", ch);
}

void
do_bestow (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Bestow whom with what?\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("You can't give special abilities to a mob!\n\r", ch);
      return;
    }
  if (get_trust (victim) > get_trust (ch))
    {
      send_to_char ("You aren't powerful enough...\n\r", ch);
      return;
    }

  if (!victim->pcdata->bestowments)
    victim->pcdata->bestowments = str_dup ("");

  if (argument[0] == '\0' || !str_cmp (argument, "list"))
    {
      ch_printf (ch, "Current bestowed commands on %s: %s.\n\r",
		 victim->name, victim->pcdata->bestowments);
      return;
    }
  if (!str_cmp (argument, "none"))
    {
      DISPOSE (victim->pcdata->bestowments);
      victim->pcdata->bestowments = str_dup ("");
      ch_printf (ch, "Bestowments removed from %s.\n\r", victim->name);
      ch_printf (victim, "%s has removed your bestowed commands.\n\r",
		 ch->name);
      return;
    }

  sprintf (buf, "%s %s", victim->pcdata->bestowments, argument);
  DISPOSE (victim->pcdata->bestowments);
  victim->pcdata->bestowments = str_dup (buf);
  set_char_color (AT_IMMORT, victim);
  ch_printf (victim, "%s has bestowed on you the command(s): %s\n\r",
	     ch->name, argument);
  send_to_char ("Done.\n\r", ch);
}

struct tm *
update_time (struct tm *old_time)
{
  time_t time;

  time = mktime (old_time);
  return localtime (&time);
}

void
do_set_boot_time (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  bool check;

  check = FALSE;
  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char
	("Syntax: setboot time {hour minute <day> <month> <year>}\n\r", ch);
      send_to_char ("        setboot manual {0/1}\n\r", ch);
      send_to_char ("        setboot default\n\r", ch);
      ch_printf (ch,
		 "Boot time is currently set to %s, manual bit is set to %d\n\r",
		 reboot_time, set_boot_time->manual);
      return;
    }

  if (!str_cmp (arg, "time"))
    {
      struct tm *now_time;

      argument = one_argument (argument, arg);
      argument = one_argument (argument, arg1);
      if (!*arg || !*arg1 || !is_number (arg) || !is_number (arg1))
	{
	  send_to_char ("You must input a value for hour and minute.\n\r",
			ch);
	  return;
	}

      now_time = localtime (&current_time);
      if ((now_time->tm_hour = atoi (arg)) < 0 || now_time->tm_hour > 23)
	{
	  send_to_char ("Valid range for hour is 0 to 23.\n\r", ch);
	  return;
	}
      if ((now_time->tm_min = atoi (arg1)) < 0 || now_time->tm_min > 59)
	{
	  send_to_char ("Valid range for minute is 0 to 59.\n\r", ch);
	  return;
	}

      argument = one_argument (argument, arg);
      if (*arg != '\0' && is_number (arg))
	{
	  if ((now_time->tm_mday = atoi (arg)) < 1 || now_time->tm_mday > 31)
	    {
	      send_to_char ("Valid range for day is 1 to 31.\n\r", ch);
	      return;
	    }
	  argument = one_argument (argument, arg);
	  if (*arg != '\0' && is_number (arg))
	    {
	      if ((now_time->tm_mon = atoi (arg)) < 1
		  || now_time->tm_mon > 12)
		{
		  send_to_char ("Valid range for month is 1 to 12.\n\r", ch);
		  return;
		}
	      now_time->tm_mon--;
	      argument = one_argument (argument, arg);
	      if ((now_time->tm_year = atoi (arg) - 1900) < 0 ||
		  now_time->tm_year > 199)
		{
		  send_to_char ("Valid range for year is 1900 to 2099.\n\r",
				ch);
		  return;
		}
	    }
	}

      now_time->tm_sec = 0;
      if (mktime (now_time) < current_time)
	{
	  send_to_char ("You can't set a time previous to today!\n\r", ch);
	  return;
	}
      if (set_boot_time->manual == 0)
	set_boot_time->manual = 1;
      new_boot_time = update_time (now_time);
      new_boot_struct = *new_boot_time;
      new_boot_time = &new_boot_struct;
      reboot_check (mktime (new_boot_time));
      get_reboot_string ();

      ch_printf (ch, "Boot time set to %s\n\r", reboot_time);
      check = TRUE;
    }
  else if (!str_cmp (arg, "manual"))
    {
      argument = one_argument (argument, arg1);
      if (arg1[0] == '\0')
	{
	  send_to_char ("Please enter a value for manual boot on/off\n\r",
			ch);
	  return;
	}
      if (!is_number (arg1))
	{
	  send_to_char ("Value for manual must be 0 (off) or 1 (on)\n\r", ch);
	  return;
	}
      if (atoi (arg1) < 0 || atoi (arg1) > 1)
	{
	  send_to_char ("Value for manual must be 0 (off) or 1 (on)\n\r", ch);
	  return;
	}

      set_boot_time->manual = atoi (arg1);
      ch_printf (ch, "Manual bit set to %s\n\r", arg1);
      check = TRUE;
      get_reboot_string ();
      return;
    }

  else if (!str_cmp (arg, "default"))
    {
      set_boot_time->manual = 0;
      /* Reinitialize new_boot_time */
      new_boot_time = localtime (&current_time);
      new_boot_time->tm_mday += 1;
      if (new_boot_time->tm_hour > 12)
	new_boot_time->tm_mday += 1;
      new_boot_time->tm_hour = 6;
      new_boot_time->tm_min = 0;
      new_boot_time->tm_sec = 0;
      new_boot_time = update_time (new_boot_time);

      sysdata.DENY_NEW_PLAYERS = FALSE;

      send_to_char ("Reboot time set back to normal.\n\r", ch);
      check = TRUE;
    }

  if (!check)
    {
      send_to_char ("Invalid argument for setboot.\n\r", ch);
      return;
    }
  else
    {
      get_reboot_string ();
      new_boot_time_t = mktime (new_boot_time);
    }
}

/* Online high level immortal command for displaying what the encryption
 * of a name/password would be, taking in 2 arguments - the name and the
 * password - can still only change the password if you have access to 
 * pfiles and the correct password
 */
void
do_form_password (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_STRING_LENGTH];

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg);
  ch_printf (ch, "Those two arguments encrypted result in:  %s\n\r",
	     sha256_crypt (arg, argument));
  return;
}

/*
 * Purge a player file.  No more player.  -- Altrag
 */
void
do_destro (CHAR_DATA * ch, char *argument)
{
  set_char_color (AT_RED, ch);
  send_to_char ("If you want to destroy a character, spell it out!\n\r", ch);
  return;
}

/*
 * This could have other applications too.. move if needed. -- Altrag
 */
void
close_area (AREA_DATA * pArea)
{
  extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
  extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
  extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
  CHAR_DATA *ech;
  CHAR_DATA *ech_next;
  OBJ_DATA *eobj;
  OBJ_DATA *eobj_next;
  int icnt;
  ROOM_INDEX_DATA *rid;
  ROOM_INDEX_DATA *rid_next;
  OBJ_INDEX_DATA *oid;
  OBJ_INDEX_DATA *oid_next;
  MOB_INDEX_DATA *mid;
  MOB_INDEX_DATA *mid_next;
  RESET_DATA *ereset;
  RESET_DATA *ereset_next;
  EXTRA_DESCR_DATA *eed;
  EXTRA_DESCR_DATA *eed_next;
  EXIT_DATA *exit;
  EXIT_DATA *exit_next;
  MPROG_ACT_LIST *mpact;
  MPROG_ACT_LIST *mpact_next;
  MPROG_DATA *mprog;
  MPROG_DATA *mprog_next;
  AFFECT_DATA *paf;
  AFFECT_DATA *paf_next;

  for (ech = first_char; ech; ech = ech_next)
    {
      ech_next = ech->next;

      if (ech->fighting)
	stop_fighting (ech, TRUE);
      if (IS_NPC (ech))
	{
	  /* if mob is in area, or part of area. */
	  if (URANGE (pArea->low_m_vnum, ech->pIndexData->vnum,
		      pArea->hi_m_vnum) == ech->pIndexData->vnum ||
	      (ech->in_room && ech->in_room->area == pArea))
	    extract_char (ech, TRUE);
	  continue;
	}
      if (ech->in_room && ech->in_room->area == pArea)
	do_recall (ech, "");
    }
  for (eobj = first_object; eobj; eobj = eobj_next)
    {
      eobj_next = eobj->next;
      /* if obj is in area, or part of area. */
      if (URANGE (pArea->low_o_vnum, eobj->pIndexData->vnum,
		  pArea->hi_o_vnum) == eobj->pIndexData->vnum ||
	  (eobj->in_room && eobj->in_room->area == pArea))
	extract_obj (eobj);
    }
  for (icnt = 0; icnt < MAX_KEY_HASH; icnt++)
    {
      for (rid = room_index_hash[icnt]; rid; rid = rid_next)
	{
	  rid_next = rid->next;

	  for (exit = rid->first_exit; exit; exit = exit_next)
	    {
	      exit_next = exit->next;
	      if (rid->area == pArea || exit->to_room->area == pArea)
		{
		  STRFREE (exit->keyword);
		  STRFREE (exit->description);
		  UNLINK (exit, rid->first_exit, rid->last_exit, next, prev);
		  DISPOSE (exit);
		  /* Crash bug fix.  I know it could go from the start several times
		   * But you CAN NOT iterate over a link-list and DELETE from it or
		   * Nasty things can and will happen. --Shaddai 
		   */
		  exit = rid->first_exit;
		}
	    }
	  if (rid->area != pArea)
	    continue;
	  STRFREE (rid->name);
	  STRFREE (rid->description);
	  if (rid->first_person)
	    {
	      bug ("close_area: room with people #%d", rid->vnum);
	      for (ech = rid->first_person; ech; ech = ech_next)
		{
		  ech_next = ech->next_in_room;
		  if (ech->fighting)
		    stop_fighting (ech, TRUE);
		  if (IS_NPC (ech))
		    extract_char (ech, TRUE);
		  else
		    do_recall (ech, "");
		}
	    }
	  if (rid->first_content)
	    {
	      bug ("close_area: room with contents #%d", rid->vnum);
	      for (eobj = rid->first_content; eobj; eobj = eobj_next)
		{
		  eobj_next = eobj->next_content;
		  extract_obj (eobj);
		}
	    }
	  for (eed = rid->first_extradesc; eed; eed = eed_next)
	    {
	      eed_next = eed->next;
	      STRFREE (eed->keyword);
	      STRFREE (eed->description);
	      DISPOSE (eed);
	    }
	  for (mpact = rid->mpact; mpact; mpact = mpact_next)
	    {
	      mpact_next = mpact->next;
	      STRFREE (mpact->buf);
	      DISPOSE (mpact);
	    }
	  for (mprog = rid->mudprogs; mprog; mprog = mprog_next)
	    {
	      mprog_next = mprog->next;
	      STRFREE (mprog->arglist);
	      STRFREE (mprog->comlist);
	      DISPOSE (mprog);
	    }
	  if (rid == room_index_hash[icnt])
	    room_index_hash[icnt] = rid->next;
	  else
	    {
	      ROOM_INDEX_DATA *trid;

	      for (trid = room_index_hash[icnt]; trid; trid = trid->next)
		if (trid->next == rid)
		  break;
	      if (!trid)
		bug ("Close_area: rid not in hash list %d", rid->vnum);
	      else
		trid->next = rid->next;
	    }
	  DISPOSE (rid);
	}

      for (mid = mob_index_hash[icnt]; mid; mid = mid_next)
	{
	  mid_next = mid->next;

	  if (mid->vnum < pArea->low_m_vnum || mid->vnum > pArea->hi_m_vnum)
	    continue;

	  STRFREE (mid->player_name);
	  STRFREE (mid->short_descr);
	  STRFREE (mid->long_descr);
	  STRFREE (mid->description);
	  if (mid->pShop)
	    {
	      UNLINK (mid->pShop, first_shop, last_shop, next, prev);
	      DISPOSE (mid->pShop);
	    }
	  if (mid->rShop)
	    {
	      UNLINK (mid->rShop, first_repair, last_repair, next, prev);
	      DISPOSE (mid->rShop);
	    }
	  for (mprog = mid->mudprogs; mprog; mprog = mprog_next)
	    {
	      mprog_next = mprog->next;
	      STRFREE (mprog->arglist);
	      STRFREE (mprog->comlist);
	      DISPOSE (mprog);
	    }
	  if (mid == mob_index_hash[icnt])
	    mob_index_hash[icnt] = mid->next;
	  else
	    {
	      MOB_INDEX_DATA *tmid;

	      for (tmid = mob_index_hash[icnt]; tmid; tmid = tmid->next)
		if (tmid->next == mid)
		  break;
	      if (!tmid)
		bug ("Close_area: mid not in hash list %d", mid->vnum);
	      else
		tmid->next = mid->next;
	    }
	  DISPOSE (mid);
	}

      for (oid = obj_index_hash[icnt]; oid; oid = oid_next)
	{
	  oid_next = oid->next;

	  if (oid->vnum < pArea->low_o_vnum || oid->vnum > pArea->hi_o_vnum)
	    continue;

	  STRFREE (oid->name);
	  STRFREE (oid->short_descr);
	  STRFREE (oid->description);
	  STRFREE (oid->action_desc);

	  for (eed = oid->first_extradesc; eed; eed = eed_next)
	    {
	      eed_next = eed->next;
	      STRFREE (eed->keyword);
	      STRFREE (eed->description);
	      DISPOSE (eed);
	    }
	  for (paf = oid->first_affect; paf; paf = paf_next)
	    {
	      paf_next = paf->next;
	      DISPOSE (paf);
	    }
	  for (mprog = oid->mudprogs; mprog; mprog = mprog_next)
	    {
	      mprog_next = mprog->next;
	      STRFREE (mprog->arglist);
	      STRFREE (mprog->comlist);
	      DISPOSE (mprog);
	    }
	  if (oid == obj_index_hash[icnt])
	    obj_index_hash[icnt] = oid->next;
	  else
	    {
	      OBJ_INDEX_DATA *toid;

	      for (toid = obj_index_hash[icnt]; toid; toid = toid->next)
		if (toid->next == oid)
		  break;
	      if (!toid)
		bug ("Close_area: oid not in hash list %d", oid->vnum);
	      else
		toid->next = oid->next;
	    }
	  DISPOSE (oid);
	}
    }
  for (ereset = pArea->first_reset; ereset; ereset = ereset_next)
    {
      ereset_next = ereset->next;
      DISPOSE (ereset);
    }
  DISPOSE (pArea->name);
  DISPOSE (pArea->filename);
  STRFREE (pArea->author);
  UNLINK (pArea, first_build, last_build, next, prev);
  UNLINK (pArea, first_asort, last_asort, next_sort, prev_sort);
  DISPOSE (pArea);
}

void
do_destroy (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char *name;

  set_char_color (AT_RED, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Destroy what player file?\n\r", ch);
      return;
    }

  for (victim = first_char; victim; victim = victim->next)
    if (!IS_NPC (victim) && !str_cmp (victim->name, arg))
      break;

  if (!victim)
    {
      DESCRIPTOR_DATA *d;

      /* Make sure they aren't halfway logged in. */
      for (d = first_descriptor; d; d = d->next)
	if ((victim = d->character) && !IS_NPC (victim) &&
	    !str_cmp (victim->name, arg))
	  break;
      if (d)
	close_socket (d, TRUE);
    }
  else
    {
      int x, y;

      quitting_char = victim;
      save_char_obj (victim);
      saving_char = NULL;
      extract_char (victim, TRUE);
      for (x = 0; x < MAX_WEAR; x++)
	for (y = 0; y < MAX_LAYERS; y++)
	  save_equipment[x][y] = NULL;
    }

  name = str_dup (capitalize (arg));
  sprintf (buf, "%s%c/%s", PLAYER_DIR, tolower (arg[0]), name);
  sprintf (buf2, "%s%c/%s", BACKUP_DIR, tolower (arg[0]), name);
  if (!rename (buf, buf2))
    {
      AREA_DATA *pArea;

      set_char_color (AT_RED, ch);
      ch_printf (ch,
		 "Player %s destroyed.  Pfile saved in backup directory.\n\r",
		 name);
      sprintf (buf, "%s%s", HOUSE_DIR, name);
      if (!remove (buf))
	send_to_char ("Player's housing data destroyed.\n\r", ch);
      else if (errno != ENOENT)
	{
	  ch_printf (ch, "Unknown error #%d - %s (housing data)."
		     "  Report to Coder.\n\r", errno, strerror (errno));
	  sprintf (buf2, "%s destroying %s", ch->name, buf);
	  perror (buf2);
	}
      sprintf (buf, "%s%s", GOD_DIR, name);
      if (!remove (buf))
	send_to_char ("Player's immortal data destroyed.\n\r", ch);
      else if (errno != ENOENT)
	{
	  ch_printf (ch,
		     "Unknown error #%d - %s (immortal data).  Report to Thoric.\n\r",
		     errno, strerror (errno));
	  sprintf (buf2, "%s destroying %s", ch->name, buf);
	  perror (buf2);
	}

      sprintf (buf2, "%s.are", name);
      for (pArea = first_build; pArea; pArea = pArea->next)
	if (!str_cmp (pArea->filename, buf2))
	  {
	    sprintf (buf, "%s%s", BUILD_DIR, buf2);
	    if (IS_SET (pArea->status, AREA_LOADED))
	      fold_area (pArea, buf, FALSE);
	    close_area (pArea);
	    sprintf (buf2, "%s.bak", buf);
	    set_char_color (AT_RED, ch);	/* Log message changes colors */
	    if (!rename (buf, buf2))
	      send_to_char
		("Player's area data destroyed.  Area saved as backup.\n\r",
		 ch);
	    else if (errno != ENOENT)
	      {
		ch_printf (ch,
			   "Unknown error #%d - %s (area data).  Report to Thoric.\n\r",
			   errno, strerror (errno));
		sprintf (buf2, "%s destroying %s", ch->name, buf);
		perror (buf2);
	      }
	    break;
	  }
    }
  else if (errno == ENOENT)
    {
      set_char_color (AT_PLAIN, ch);
      send_to_char ("Player does not exist.\n\r", ch);
    }
  else
    {
      set_char_color (AT_WHITE, ch);
      ch_printf (ch, "Unknown error #%d - %s.  Report to Thoric.\n\r",
		 errno, strerror (errno));
      sprintf (buf, "%s destroying %s", ch->name, arg);
      perror (buf);
    }
  DISPOSE (name);
  return;
}

extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];	/* db.c */

/* Super-AT command:
FOR ALL <action>
FOR MORTALS <action>
FOR GODS <action>
FOR MOBS <action>
FOR EVERYWHERE <action>

Executes action several times, either on ALL players (not including yourself),
MORTALS (including trusted characters), GODS (characters with level higher than
L_HERO), MOBS (Not recommended) or every room (not recommended either!)

If you insert a # in the action, it will be replaced by the name of the target.

If # is a part of the action, the action will be executed for every target
in game. If there is no #, the action will be executed for every room containg
at least one target, but only once per room. # cannot be used with FOR EVERY-
WHERE. # can be anywhere in the action.

Example: 

FOR ALL SMILE -> you will only smile once in a room with 2 players.
FOR ALL TWIDDLE # -> In a room with A and B, you will twiddle A then B.

Destroying the characters this command acts upon MAY cause it to fail. Try to
avoid something like FOR MOBS PURGE (although it actually works at my MUD).

FOR MOBS TRANS 3054 (transfer ALL the mobs to Midgaard temple) does NOT work
though :)

The command works by transporting the character to each of the rooms with 
target in them. Private rooms are not violated.

*/

/* Expand the name of a character into a string that identifies THAT
   character within a room. E.g. the second 'guard' -> 2. guard
*/
const char *
name_expand (CHAR_DATA * ch)
{
  int count = 1;
  CHAR_DATA *rch;
  char name[MAX_INPUT_LENGTH];	/*  HOPEFULLY no mob has a name longer than THAT */

  static char outbuf[MAX_INPUT_LENGTH];

  if (!IS_NPC (ch))
    return ch->name;

  one_argument (ch->name, name);	/* copy the first word into name */

  if (!name[0])			/* weird mob .. no keywords */
    {
      strcpy (outbuf, "");	/* Do not return NULL, just an empty buffer */
      return outbuf;
    }

  /* ->people changed to ->first_person -- TRI */
  for (rch = ch->in_room->first_person; rch && (rch != ch); rch =
       rch->next_in_room)
    if (is_name (name, rch->name))
      count++;


  sprintf (outbuf, "%d.%s", count, name);
  return outbuf;
}

void
do_for (CHAR_DATA * ch, char *argument)
{
  char range[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  bool fGods = FALSE, fMortals = FALSE, fMobs = FALSE, fEverywhere =
    FALSE, found;
  ROOM_INDEX_DATA *room, *old_room;
  CHAR_DATA *p, *p_prev;	/* p_next to p_prev -- TRI */
  int i;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, range);
  if (!range[0] || !argument[0])	/* invalid usage? */
    {
      do_help (ch, "for");
      return;
    }

  if (!str_prefix ("quit", argument))
    {
      send_to_char ("Are you trying to crash the MUD or something?\n\r", ch);
      return;
    }


  if (!str_cmp (range, "all"))
    {
      fMortals = TRUE;
      fGods = TRUE;
    }
  else if (!str_cmp (range, "gods"))
    fGods = TRUE;
  else if (!str_cmp (range, "mortals"))
    fMortals = TRUE;
  else if (!str_cmp (range, "mobs"))
    fMobs = TRUE;
  else if (!str_cmp (range, "everywhere"))
    fEverywhere = TRUE;
  else
    do_help (ch, "for");	/* show syntax */

  /* do not allow # to make it easier */
  if (fEverywhere && strchr (argument, '#'))
    {
      send_to_char ("Cannot use FOR EVERYWHERE with the # thingie.\n\r", ch);
      return;
    }

  set_char_color (AT_PLAIN, ch);
  if (strchr (argument, '#'))	/* replace # ? */
    {
      /* char_list - last_char, p_next - gch_prev -- TRI */
      for (p = last_char; p; p = p_prev)
	{
	  p_prev = p->prev;	/* TRI */
	  /*      p_next = p->next; *//* In case someone DOES try to AT MOBS SLAY # */
	  found = FALSE;

	  if (!(p->in_room) || room_is_private (p->in_room) || (p == ch))
	    continue;

	  if (IS_NPC (p) && fMobs)
	    found = TRUE;
	  else if (!IS_NPC (p) && p->level >= LEVEL_IMMORTAL && fGods)
	    found = TRUE;
	  else if (!IS_NPC (p) && p->level < LEVEL_IMMORTAL && fMortals)
	    found = TRUE;

	  /* It looks ugly to me.. but it works :) */
	  if (found)		/* p is 'appropriate' */
	    {
	      char *pSource = argument;	/* head of buffer to be parsed */
	      char *pDest = buf;	/* parse into this */

	      while (*pSource)
		{
		  if (*pSource == '#')	/* Replace # with name of target */
		    {
		      const char *namebuf = name_expand (p);

		      if (namebuf)	/* in case there is no mob name ?? */
			while (*namebuf)	/* copy name over */
			  *(pDest++) = *(namebuf++);

		      pSource++;
		    }
		  else
		    *(pDest++) = *(pSource++);
		}		/* while */
	      *pDest = '\0';	/* Terminate */

	      /* Execute */
	      old_room = ch->in_room;
	      char_from_room (ch);
	      char_to_room (ch, p->in_room);
	      interpret (ch, buf);
	      char_from_room (ch);
	      char_to_room (ch, old_room);

	    }			/* if found */
	}			/* for every char */
    }
  else				/* just for every room with the appropriate people in it */
    {
      for (i = 0; i < MAX_KEY_HASH; i++)	/* run through all the buckets */
	for (room = room_index_hash[i]; room; room = room->next)
	  {
	    found = FALSE;

	    /* Anyone in here at all? */
	    if (fEverywhere)	/* Everywhere executes always */
	      found = TRUE;
	    else if (!room->first_person)	/* Skip it if room is empty */
	      continue;
	    /* ->people changed to first_person -- TRI */

	    /* Check if there is anyone here of the requried type */
	    /* Stop as soon as a match is found or there are no more ppl in room */
	    /* ->people to ->first_person -- TRI */
	    for (p = room->first_person; p && !found; p = p->next_in_room)
	      {

		if (p == ch)	/* do not execute on oneself */
		  continue;

		if (IS_NPC (p) && fMobs)
		  found = TRUE;
		else if (!IS_NPC (p) && (p->level >= LEVEL_IMMORTAL) && fGods)
		  found = TRUE;
		else if (!IS_NPC (p) && (p->level <= LEVEL_IMMORTAL)
			 && fMortals)
		  found = TRUE;
	      }			/* for everyone inside the room */

	    if (found && !room_is_private (room))	/* Any of the required type here AND room not private? */
	      {
		/* This may be ineffective. Consider moving character out of old_room
		   once at beginning of command then moving back at the end.
		   This however, is more safe?
		 */

		old_room = ch->in_room;
		char_from_room (ch);
		char_to_room (ch, room);
		interpret (ch, argument);
		char_from_room (ch);
		char_to_room (ch, old_room);
	      }			/* if found */
	  }			/* for every room in a bucket */
    }				/* if strchr */
}				/* do_for */

void save_sysdata args ((SYSTEM_DATA sys));

void
do_cset (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_STRING_LENGTH];
  sh_int level;

  set_pager_color (AT_PLAIN, ch);

  if (argument[0] == '\0')
    {
      pager_printf_color (ch, "\n\r&WMud_name: %s", sysdata.mud_name);
      pager_printf_color (ch, "\n\r&WPort_name: %s", sysdata.port_name);
      pager_printf_color (ch,
			  "\n\r&WMail:\n\r  &wRead all mail: &W%d  &wRead mail for free: &W%d  &wWrite mail for free: &W%d\n\r",
			  sysdata.read_all_mail, sysdata.read_mail_free,
			  sysdata.write_mail_free);
      pager_printf_color (ch,
			  "&WChannels:\n\r  &wMuse: &W%d   &wThink: &W%d   &wLog: &W%d   &wBuild: &W%d\n\r",
			  sysdata.muse_level, sysdata.think_level,
			  sysdata.log_level, sysdata.build_level);
      pager_printf_color (ch,
			  "&WBuilding:\n\r  &wPrototype modification: &W%d  &wPlayer msetting: &W%d  &wArea Save version: &W%d\n\r",
			  sysdata.level_modify_proto,
			  sysdata.level_mset_player, sysdata.save_version);
      pager_printf_color (ch,
			  "&WGuilds:\n\r  &wOverseer: &W%s   &wAdvisor: &W%s\n\r",
			  sysdata.guild_overseer, sysdata.guild_advisor);
      pager_printf_color (ch,
			  "&WBan Data:\n\r  &wBan Site Level: &W%d   &wBan Class Level: &W%d   ",
			  sysdata.ban_site_level, sysdata.ban_class_level);
      pager_printf_color (ch, "&wBan Race Level: &W%d\n\r",
			  sysdata.ban_race_level);
      pager_printf_color (ch,
			  "&WDefenses:\n\r  &wDodge_mod: &W%d    &wParry_mod: &W%d    &wTumble_mod: &W%d    &wTumble_pk:  &W%d\n\r",
			  sysdata.dodge_mod, sysdata.parry_mod,
			  sysdata.tumble_mod, sysdata.tumble_pk);
      pager_printf_color (ch,
			  "&WOther:\n\r  &wForce on players:             &W%-2d     ",
			  sysdata.level_forcepc);
      pager_printf_color (ch, "&wPrivate room override:         &W%-2d\n\r",
			  sysdata.level_override_private);
      pager_printf_color (ch, "  &wPenalty to bash plr vs. plr:  &W%-7d",
			  sysdata.bash_plr_vs_plr);
      pager_printf_color (ch, "&wPenalty to non-tank bash:      &W%-3d\n\r",
			  sysdata.bash_nontank);
      pager_printf_color (ch, "  &wPenalty to gouge plr vs. plr: &W%-7d",
			  sysdata.gouge_plr_vs_plr);
      pager_printf_color (ch, "&wPenalty to non-tank gouge:     &W%-3d\n\r",
			  sysdata.gouge_nontank);
      pager_printf_color (ch, "  &wPenalty regular stun chance:  &W%-7d",
			  sysdata.stun_regular);
      pager_printf_color (ch, "&wPenalty to stun plr vs. plr:   &W%-3d\n\r",
			  sysdata.stun_plr_vs_plr);
      pager_printf_color (ch, "  &wPercent damage plr vs. plr:   &W%-7d",
			  sysdata.dam_plr_vs_plr);
      pager_printf_color (ch, "&wPercent damage plr vs. mob:    &W%-3d \n\r",
			  sysdata.dam_plr_vs_mob);
      pager_printf_color (ch, "  &wPercent damage mob vs. plr:   &W%-7d",
			  sysdata.dam_mob_vs_plr);
      pager_printf_color (ch, "&wPercent damage mob vs. mob:    &W%-3d\n\r",
			  sysdata.dam_mob_vs_mob);
      pager_printf_color (ch, "  &wPercent damage nonav vs. mob: &W%-7d",
			  sysdata.dam_nonav_vs_mob);
      pager_printf_color (ch, "&wPercent damage mob vs. nonav:  &W%-3d\n\r",
			  sysdata.dam_mob_vs_nonav);
      pager_printf_color (ch, "  &wGet object without take flag: &W%-7d",
			  sysdata.level_getobjnotake);
      pager_printf_color (ch, "&wAutosave frequency (minutes):  &W%d\n\r",
			  sysdata.save_frequency);
      pager_printf_color (ch, "  &wMax level difference bestow:  &W%-7d",
			  sysdata.bestow_dif);
      pager_printf_color (ch, "&wChecking Imm_host is:          &W%s\n\r",
			  (sysdata.check_imm_host) ? "ON" : "off");
      pager_printf_color (ch, "  &wMorph Optimization is:        &W%-7s",
			  (sysdata.morph_opt) ? "ON" : "off");
      pager_printf_color (ch, "&wSaving Pets is:                &W%s\n\r",
			  (sysdata.save_pets) ? "ON" : "off");
      pager_printf_color (ch, "  &wPkill looting is:             &W%-7s",
			  (sysdata.pk_loot) ? "ON" : "off");
      pager_printf_color (ch, "&wPkill_channels are:            &W%s\n\r",
			  (sysdata.pk_channels) ? "ON" : "off");
      pager_printf_color (ch, "  &wPeaceful exp mod:             &W%-7d",
			  sysdata.peaceful_exp_mod);
      pager_printf_color (ch, "&wPkill_silence is:              &W%s\n\r",
			  (sysdata.pk_silence) ? "ON" : "off");
      pager_printf_color (ch, "  &wDeadly exp mod:               &W%-7d",
			  sysdata.deadly_exp_mod);
      pager_printf_color (ch, "&wWizlock after reboot:          &W%s\n\r",
			  (sysdata.wizlock) ? "ON" : "off");
      pager_printf_color (ch, "  &wSave flags: &W%s\n\r",
			  flag_string (sysdata.save_flags, save_flag));
      pager_printf_color (ch, "  &wIdents retries: &W%d\n\r",
			  sysdata.ident_retries);
      pager_printf_color (ch,
			  "  &wMax HTML news: %d&W\n\r  &wHTML news path: %s&W\n\r",
			  sysdata.max_html_news, sysdata.news_html_path);
      return;
    }

  argument = one_argument (argument, arg);
  smash_tilde (argument);

  if (!str_cmp (arg, "help"))
    {
      do_help (ch, "controls");
      return;
    }

  if (!str_cmp (arg, "save"))
    {
      save_sysdata (sysdata);
      send_to_char ("Cset functions saved.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "saveversion"))
    {
      sysdata.save_version =
	UMAX (MIN_SAVE_VERSION, ((sh_int) atoi (argument)));
      send_to_char ("Save version set.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "wizlock"))
    {
      set_char_color (AT_DANGER, ch);

      sysdata.wizlock = !sysdata.wizlock;
      ch_printf (ch, "Wizlocked at reboot is %s.\n\r",
		 sysdata.wizlock ? "ON" : "OFF");
      return;
    }

  /* 3/29/02 - for extended news - Nopey */
  if (!str_cmp (arg, "newspath"))
    {
      if (argument[0] == '\0')
	{
	  pager_printf_color (ch,
			      "News path now set to nothing.  HTML writing disabled");
	  sysdata.news_html_path[0] = '\0';
	  return;
	}
      sysdata.news_html_path = STRALLOC (argument);
      pager_printf_color (ch, "News path set to: %snews.txt\n\r", argument);
      return;
    }

  if (!str_cmp (arg, "maxnews"))
    {
      sysdata.max_html_news = atoi (argument);
      pager_printf_color (ch, "Max HTML news now set to: %d\n\r",
			  sysdata.max_html_news);
      return;
    }

  if (!str_cmp (arg, "mudname"))
    {
      if (sysdata.mud_name)
	DISPOSE (sysdata.mud_name);
      sysdata.mud_name = str_dup (argument);
      send_to_char ("Mud name set.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "portname"))
    {
      if (sysdata.port_name)
	DISPOSE (sysdata.port_name);
      sysdata.port_name = str_dup (argument);
      send_to_char ("Port name set.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "saveflag"))
    {
      int x = get_saveflag (argument);

      if (x == -1)
	send_to_char ("Not a save flag.\n\r", ch);
      else
	{
	  TOGGLE_BIT (sysdata.save_flags, 1 << x);
	  send_to_char ("Ok.\n\r", ch);
	}
      return;
    }

  if (!str_prefix (arg, "magichell"))
    {
      sysdata.magichell = !sysdata.magichell;
      ch_printf_color (ch, "&BMagic hell is %s.\n\r",
		       sysdata.magichell ? "ON" : "OFF");
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  if (!str_prefix (arg, "guild_overseer"))
    {
      STRFREE (sysdata.guild_overseer);
      sysdata.guild_overseer = STRALLOC (argument);
      send_to_char ("Ok.\n\r", ch);
      return;
    }
  if (!str_prefix (arg, "guild_advisor"))
    {
      STRFREE (sysdata.guild_advisor);
      sysdata.guild_advisor = STRALLOC (argument);
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  level = (sh_int) atoi (argument);

  if (!str_prefix (arg, "savefrequency"))
    {
      sysdata.save_frequency = level;
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  if (!str_prefix (arg, "checkimmhost"))
    {
      if (level != 0 && level != 1)
	{
	  send_to_char ("Use 1 to turn it on, 0 to turn in off.\n\r", ch);
	  return;
	}
      sysdata.check_imm_host = level;
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "bash_pvp"))
    {
      sysdata.bash_plr_vs_plr = level;
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "bash_nontank"))
    {
      sysdata.bash_nontank = level;
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "gouge_pvp"))
    {
      sysdata.gouge_plr_vs_plr = level;
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "gouge_nontank"))
    {
      sysdata.gouge_nontank = level;
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "dodge_mod"))
    {
      sysdata.dodge_mod = level > 0 ? level : 1;
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "parry_mod"))
    {
      sysdata.parry_mod = level > 0 ? level : 1;
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "tumble_mod"))
    {
      sysdata.tumble_mod = level > 0 ? level : 1;
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "tumble_pk"))
    {
      sysdata.tumble_pk = level > 0 ? level : 1;
      send_to_char ("Tumble_pk mod set.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "stun"))
    {
      sysdata.stun_regular = level;
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "stun_pvp"))
    {
      sysdata.stun_plr_vs_plr = level;
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "deadly_exp"))
    {
      sysdata.deadly_exp_mod = level;
      send_to_char ("Deadly experience multiplier modified.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "peaceful_exp"))
    {
      sysdata.peaceful_exp_mod = level;
      send_to_char ("Peaceful experience multiplier modified.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "dam_pvp"))
    {
      sysdata.dam_plr_vs_plr = level;
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "get_notake"))
    {
      sysdata.level_getobjnotake = level;
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "dam_navm"))
    {
      sysdata.dam_nonav_vs_mob = level;
      send_to_char ("Non-avatar damage vs mobile set.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "dam_mvna"))
    {
      sysdata.dam_mob_vs_nonav = level;
      send_to_char ("Mobile damage vs non-avatar set.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "dam_pvm"))
    {
      sysdata.dam_plr_vs_mob = level;
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "dam_mvp"))
    {
      sysdata.dam_mob_vs_plr = level;
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "dam_mvm"))
    {
      sysdata.dam_mob_vs_mob = level;
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "ident_retries") || !str_cmp (arg, "ident"))
    {
      sysdata.ident_retries = level;
      if (level > 20)
	send_to_char ("Caution:  This setting may cause the game to lag.\n\r",
		      ch);
      else if (level <= 0)
	send_to_char ("Ident lookups turned off.\n\r", ch);
      else
	send_to_char ("Ok.\n\r", ch);
      return;
    }

  if (level < 0 || level > MAX_LEVEL)
    {
      send_to_char ("Invalid value for new control.\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "read_all"))
    sysdata.read_all_mail = level;
  else if (!str_cmp (arg, "read_free"))
    sysdata.read_mail_free = level;
  else if (!str_cmp (arg, "write_free"))
    sysdata.write_mail_free = level;
  else if (!str_cmp (arg, "take_all"))
    sysdata.take_others_mail = level;
  else if (!str_cmp (arg, "muse"))
    sysdata.muse_level = level;
  else if (!str_cmp (arg, "think"))
    sysdata.think_level = level;
  else if (!str_cmp (arg, "log"))
    sysdata.log_level = level;
  else if (!str_cmp (arg, "build"))
    sysdata.build_level = level;
  else if (!str_cmp (arg, "proto_modify"))
    sysdata.level_modify_proto = level;
  else if (!str_cmp (arg, "override_private"))
    sysdata.level_override_private = level;
  else if (!str_cmp (arg, "bestow_dif"))
    sysdata.bestow_dif = level > 0 ? level : 1;
  else if (!str_cmp (arg, "forcepc"))
    sysdata.level_forcepc = level;
  else if (!str_cmp (arg, "ban_site_level"))
    sysdata.ban_site_level = level;
  else if (!str_cmp (arg, "ban_race_level"))
    sysdata.ban_race_level = level;
  else if (!str_cmp (arg, "ban_class_level"))
    sysdata.ban_class_level = level;
  else if (!str_cmp (arg, "petsave"))
    {
      if (level)
	sysdata.save_pets = TRUE;
      else
	sysdata.save_pets = FALSE;
    }
  else if (!str_cmp (arg, "pk_silence"))
    {
      if (level)
	{
	  send_to_char ("Pkill silence is enabled.\n\r", ch);
	  sysdata.pk_silence = TRUE;
	}
      else
	{
	  send_to_char
	    ("Pkill silence is disabled.  (use cset pk_silence 1 to enable.)\n\r",
	     ch);
	  sysdata.pk_silence = FALSE;
	}
    }
  else if (!str_cmp (arg, "pk_channels"))
    {
      if (level)
	{
	  send_to_char ("Pkill channels are enabled.\n\r", ch);
	  sysdata.pk_channels = TRUE;
	}
      else
	{
	  send_to_char
	    ("Pkill channels are disabled.  (use cset pk_channels 1 to enable)\n\r",
	     ch);
	  sysdata.pk_channels = FALSE;
	}
    }
  else if (!str_cmp (arg, "pk_loot"))
    {
      if (level)
	{
	  send_to_char ("Pkill looting is enabled.\n\r", ch);
	  sysdata.pk_loot = TRUE;
	}
      else
	{
	  send_to_char
	    ("Pkill looting is disabled.  (use cset pkloot 1 to enable)\n\r",
	     ch);
	  sysdata.pk_loot = FALSE;
	}
    }
  else if (!str_cmp (arg, "morph_opt"))
    {
      if (level)
	sysdata.morph_opt = TRUE;
      else
	sysdata.morph_opt = FALSE;
    }
  else if (!str_cmp (arg, "mset_player"))
    sysdata.level_mset_player = level;
  else
    {
      send_to_char ("Invalid argument.\n\r", ch);
      return;
    }
  send_to_char ("Ok.\n\r", ch);
  return;
}

void
get_reboot_string (void)
{
  sprintf (reboot_time, "%s", asctime (new_boot_time));
}

void
do_orange (CHAR_DATA * ch, char *argument)
{
  send_to_char_color ("&YFunction under construction.\n\r", ch);
  return;
}

void
do_mrange (CHAR_DATA * ch, char *argument)
{
  send_to_char_color ("&YFunction under construction.\n\r", ch);
  return;
}

void
do_hell (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  sh_int time;
  bool h_d = FALSE;
  struct tm *tms;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg);
  if (!*arg)
    {
      send_to_char ("Hell who, and for how long?\n\r", ch);
      return;
    }
  if (!(victim = get_char_world (ch, arg)) || IS_NPC (victim))
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_IMMORTAL (victim))
    {
      send_to_char ("There is no point in helling an immortal.\n\r", ch);
      return;
    }
  if (victim->pcdata->release_date != 0)
    {
      ch_printf (ch, "They are already in hell until %24.24s, by %s.\n\r",
		 ctime (&victim->pcdata->release_date),
		 victim->pcdata->helled_by);
      return;
    }

  argument = one_argument (argument, arg);
  if (!*arg || !is_number (arg))
    {
      send_to_char ("Hell them for how long?\n\r", ch);
      return;
    }

  time = atoi (arg);
  if (time <= 0)
    {
      send_to_char ("You cannot hell for zero or negative time.\n\r", ch);
      return;
    }

  argument = one_argument (argument, arg);
  if (!*arg || !str_cmp (arg, "hours"))
    h_d = TRUE;
  else if (str_cmp (arg, "days"))
    {
      send_to_char ("Is that value in hours or days?\n\r", ch);
      return;
    }
  else if (time > 30)
    {
      send_to_char
	("You may not hell a person for more than 30 days at a time.\n\r",
	 ch);
      return;
    }
  tms = localtime (&current_time);

  if (h_d)
    tms->tm_hour += time;
  else
    tms->tm_mday += time;
  victim->pcdata->release_date = mktime (tms);
  victim->pcdata->helled_by = STRALLOC (ch->name);
  ch_printf (ch, "%s will be released from hell at %24.24s.\n\r",
	     victim->name, ctime (&victim->pcdata->release_date));
  act (AT_MAGIC, "$n disappears in a cloud of hellish light.", victim, NULL,
       ch, TO_NOTVICT);
  char_from_room (victim);
  char_to_room (victim, get_room_index (8));
  act (AT_MAGIC, "$n appears in a could of hellish light.", victim, NULL, ch,
       TO_NOTVICT);
  do_look (victim, "auto");
  if (!victim->desc)
    add_loginmsg (victim->name, 10, NULL);
  else
    ch_printf (victim, "The immortals are not pleased with your actions.\n\r"
	       "You shall remain in hell for %d %s%s.\n\r", time,
	       (h_d ? "hour" : "day"), (time == 1 ? "" : "s"));
  save_char_obj (victim);	/* used to save ch, fixed by Thoric 09/17/96 */
  return;
}

void
do_unhell (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg);
  if (!*arg)
    {
      send_to_char ("Unhell whom..?\n\r", ch);
      return;
    }
  location = ch->in_room;
/*ch->in_room = get_room_index(8);*/
  victim = get_char_world (ch, arg);
/*ch->in_room = location;          The case of unhell self, etc.*/
  if (!victim || IS_NPC (victim))
    {
      send_to_char ("No such player character present.\n\r", ch);
      return;
    }
  if (victim->in_room->vnum != 8
      && victim->in_room->vnum != 1206 && victim->in_room->vnum != 6)
    {
      send_to_char ("No one like that is in hell.\n\r", ch);
      return;
    }

  if (victim->pcdata->clan)
    location = get_room_index (victim->pcdata->clan->recall);
  else
    location = get_room_index (ROOM_VNUM_TEMPLE);
  if (!location)
    location = ch->in_room;
  MOBtrigger = FALSE;
  act (AT_MAGIC, "$n disappears in a cloud of godly light.", victim, NULL, ch,
       TO_NOTVICT);
  char_from_room (victim);
  char_to_room (victim, location);
  send_to_char
    ("The gods have smiled on you and released you from hell early!\n\r",
     victim);
  do_look (victim, "auto");
  if (victim != ch)
    send_to_char ("They have been released.\n\r", ch);
  if (victim->pcdata->helled_by)
    {
      if (str_cmp (ch->name, victim->pcdata->helled_by))
	ch_printf (ch,
		   "(You should probably write a note to %s, explaining the early release.)\n\r",
		   victim->pcdata->helled_by);
      STRFREE (victim->pcdata->helled_by);
      victim->pcdata->helled_by = NULL;
    }

  MOBtrigger = FALSE;
  act (AT_MAGIC, "$n appears in a cloud of godly light.", victim, NULL, ch,
       TO_NOTVICT);
  victim->pcdata->release_date = 0;
  save_char_obj (victim);
  return;
}

/* Consolidated *assign function. 
 * Assigns room/obj/mob ranges and initializes new zone - Samson 2-12-99 
 */
/* Bugfix: Vnum range would not be saved properly without placeholders at
both ends - Samson 1-6-00 */
void
do_vassign (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  int lo, hi;
  CHAR_DATA *victim, *mob;
  ROOM_INDEX_DATA *room;
  MOB_INDEX_DATA *pMobIndex;
  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *obj;
  AREA_DATA *tarea;
  char filename[256];

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);
  lo = atoi (arg2);
  hi = atoi (arg3);

  if (arg1[0] == '\0' || lo < 0 || hi < 0)
    {
      send_to_char ("Syntax: vassign <who> <low> <high>\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg1)) == NULL)
    {
      send_to_char ("They don't seem to be around.\n\r", ch);
      return;
    }
  if (IS_NPC (victim) || get_trust (victim) < LEVEL_CREATOR)
    {
      send_to_char ("They wouldn't know what to do with a vnum range.\n\r",
		    ch);
      return;
    }
  if (victim->pcdata->area && lo != 0)
    {
      send_to_char
	("You cannot assign them a range, they already have one!\n\r", ch);
      return;
    }
  if (lo > hi)
    {
      send_to_char ("Unacceptable vnum range.\n\r", ch);
      return;
    }
  if (lo == 0)
    hi = 0;
  victim->pcdata->r_range_lo = lo;
  victim->pcdata->r_range_hi = hi;
  victim->pcdata->o_range_lo = lo;
  victim->pcdata->o_range_hi = hi;
  victim->pcdata->m_range_lo = lo;
  victim->pcdata->m_range_hi = hi;
  assign_area (victim);
  send_to_char ("Done.\n\r", ch);
  ch_printf (victim, "%s has assigned you the vnum range %d - %d.\n\r",
	     ch->name, lo, hi);
  assign_area (victim);		/* Put back by Thoric on 02/07/96 */

  if (!victim->pcdata->area)
    {
      bug ("vassign: assign_area failed", 0);
      return;
    }

  tarea = victim->pcdata->area;

  if (lo == 0)			/* Scryn 8/12/95 */
    {
      REMOVE_BIT (tarea->status, AREA_LOADED);
      SET_BIT (tarea->status, AREA_DELETED);
    }
  else
    {
      SET_BIT (tarea->status, AREA_LOADED);
      REMOVE_BIT (tarea->status, AREA_DELETED);
    }

  /* Initialize first and last rooms in range */
  room = make_room (lo);
  if (!room)
    {
      bug ("do_vassign: make_room failed to initialize first room.", 0);
      return;
    }
  room->area = tarea;

  room = make_room (hi);
  if (!room)
    {
      bug ("do_vassign: make_room failed to initialize last room.", 0);
      return;
    }
  room->area = tarea;

  /* Initialize first mob in range */
  pMobIndex = make_mobile (lo, 0, "first mob");
  if (!pMobIndex)
    {
      log_string ("do_vassign: make_mobile failed to initialize first mob.");
      return;
    }
  mob = create_mobile (pMobIndex);
  char_to_room (mob, room);

  /* Initialize last mob in range */
  pMobIndex = make_mobile (hi, 0, "last mob");
  if (!pMobIndex)
    {
      log_string ("do_vassign: make_mobile failed to initialize last mob.");
      return;
    }
  mob = create_mobile (pMobIndex);
  char_to_room (mob, room);

  /* Initialize first obj in range */
  pObjIndex = make_object (lo, 0, "first obj");
  if (!pObjIndex)
    {
      log_string ("do_vassign: make_object failed to initialize first obj.");
      return;
    }
  obj = create_object (pObjIndex, 0);
  obj_to_room (obj, room);

  /* Initialize last obj in range */
  pObjIndex = make_object (hi, 0, "last obj");
  if (!pObjIndex)
    {
      log_string ("do_vassign: make_object failed to initialize last obj.");
      return;
    }
  obj = create_object (pObjIndex, 0);
  obj_to_room (obj, room);

  /* Save character and newly created zone */
  save_char_obj (victim);

  if (!IS_SET (tarea->status, AREA_DELETED))
    {
      sprintf (filename, "%s%s", BUILD_DIR, tarea->filename);
      fold_area (tarea, filename, FALSE);
    }

  set_char_color (AT_IMMORT, ch);
  ch_printf (ch, "Vnum range set for %s and initialized.\n\r", victim->name);

  return;
}




/* Vnum search command by Swordbearer */
void
do_vsearch (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  bool found = FALSE;
  OBJ_DATA *obj;
  OBJ_DATA *in_obj;
  int obj_counter = 1;
  int argi;

  set_pager_color (AT_PLAIN, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Syntax:  vsearch <vnum>.\n\r", ch);
      return;
    }

  argi = atoi (arg);
  if (argi < 0 && argi > 20000)
    {
      send_to_char ("Vnum out of range.\n\r", ch);
      return;
    }
  for (obj = first_object; obj != NULL; obj = obj->next)
    {
      if (!can_see_obj (ch, obj) || !(argi == obj->pIndexData->vnum))
	continue;

      found = TRUE;
      for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj);

      if (in_obj->carried_by != NULL)
	pager_printf (ch, "[%2d] Level %d %s carried by %s.\n\r",
		      obj_counter,
		      obj->level, obj_short (obj),
		      PERS (in_obj->carried_by, ch));
      else
	pager_printf (ch, "[%2d] [%-5d] %s in %s.\n\r", obj_counter,
		      ((in_obj->in_room) ? in_obj->in_room->vnum : 0),
		      obj_short (obj), (in_obj->in_room == NULL) ?
		      "somewhere" : in_obj->in_room->name);

      obj_counter++;
    }

  if (!found)
    send_to_char ("Nothing like that in hell, earth, or heaven.\n\r", ch);
  return;
}

/* 
 * Simple function to let any imm make any player instantly sober.
 * Saw no need for level restrictions on this.
 * Written by Narn, Apr/96 
 */
void
do_sober (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char arg1[MAX_INPUT_LENGTH];

  set_char_color (AT_IMMORT, ch);

  smash_tilde (argument);
  argument = one_argument (argument, arg1);
  if ((victim = get_char_room (ch, arg1)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Not on mobs.\n\r", ch);
      return;
    }

  if (victim->pcdata)
    victim->pcdata->condition[COND_DRUNK] = 0;
  send_to_char ("Ok.\n\r", ch);
  set_char_color (AT_IMMORT, victim);
  send_to_char ("You feel sober again.\n\r", victim);
  return;
}

/*
 * Free a social structure					-Thoric
 */
void
free_social (SOCIALTYPE * social)
{
  if (social->name)
    DISPOSE (social->name);
  if (social->char_no_arg)
    DISPOSE (social->char_no_arg);
  if (social->others_no_arg)
    DISPOSE (social->others_no_arg);
  if (social->char_found)
    DISPOSE (social->char_found);
  if (social->others_found)
    DISPOSE (social->others_found);
  if (social->vict_found)
    DISPOSE (social->vict_found);
  if (social->char_auto)
    DISPOSE (social->char_auto);
  if (social->others_auto)
    DISPOSE (social->others_auto);
  DISPOSE (social);
}

/*
 * Remove a social from it's hash index				-Thoric
 */
void
unlink_social (SOCIALTYPE * social)
{
  SOCIALTYPE *tmp, *tmp_next;
  int hash;

  if (!social)
    {
      bug ("Unlink_social: NULL social", 0);
      return;
    }

  if (social->name[0] < 'a' || social->name[0] > 'z')
    hash = 0;
  else
    hash = (social->name[0] - 'a') + 1;

  if (social == (tmp = social_index[hash]))
    {
      social_index[hash] = tmp->next;
      return;
    }
  for (; tmp; tmp = tmp_next)
    {
      tmp_next = tmp->next;
      if (social == tmp_next)
	{
	  tmp->next = tmp_next->next;
	  return;
	}
    }
}

/*
 * Add a social to the social index table			-Thoric
 * Hashed and insert sorted
 */
void
add_social (SOCIALTYPE * social)
{
  int hash, x;
  SOCIALTYPE *tmp, *prev;

  if (!social)
    {
      bug ("Add_social: NULL social", 0);
      return;
    }

  if (!social->name)
    {
      bug ("Add_social: NULL social->name", 0);
      return;
    }

  if (!social->char_no_arg)
    {
      bug ("Add_social: NULL social->char_no_arg", 0);
      return;
    }

  /* make sure the name is all lowercase */
  for (x = 0; social->name[x] != '\0'; x++)
    social->name[x] = LOWER (social->name[x]);

  if (social->name[0] < 'a' || social->name[0] > 'z')
    hash = 0;
  else
    hash = (social->name[0] - 'a') + 1;

  if ((prev = tmp = social_index[hash]) == NULL)
    {
      social->next = social_index[hash];
      social_index[hash] = social;
      return;
    }

  for (; tmp; tmp = tmp->next)
    {
      if ((x = strcmp (social->name, tmp->name)) == 0)
	{
	  bug ("Add_social: trying to add duplicate name to bucket %d", hash);
	  free_social (social);
	  return;
	}
      else if (x < 0)
	{
	  if (tmp == social_index[hash])
	    {
	      social->next = social_index[hash];
	      social_index[hash] = social;
	      return;
	    }
	  prev->next = social;
	  social->next = tmp;
	  return;
	}
      prev = tmp;
    }

  /* add to end */
  prev->next = social;
  social->next = NULL;
  return;
}

/*
 * Social editor/displayer/save/delete				-Thoric
 */
void
do_sedit (CHAR_DATA * ch, char *argument)
{
  SOCIALTYPE *social;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  set_char_color (AT_SOCIAL, ch);

  smash_tilde (argument);
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (arg1[0] == '\0')
    {
      send_to_char ("Syntax: sedit <social> [field]\n\r", ch);
      send_to_char ("Syntax: sedit <social> create\n\r", ch);
      if (get_trust (ch) > LEVEL_GOD)
	send_to_char ("Syntax: sedit <social> delete\n\r", ch);
      if (get_trust (ch) > LEVEL_LESSER)
	send_to_char ("Syntax: sedit <save>\n\r", ch);
      send_to_char ("\n\rField being one of:\n\r", ch);
      send_to_char ("  cnoarg onoarg cfound ofound vfound cauto oauto\n\r",
		    ch);
      return;
    }

  if (get_trust (ch) > LEVEL_LESSER && !str_cmp (arg1, "save"))
    {
      save_socials ();
      send_to_char ("Saved.\n\r", ch);
      return;
    }

  social = find_social (arg1);
  if (!str_cmp (arg2, "create"))
    {
      if (social)
	{
	  send_to_char ("That social already exists!\n\r", ch);
	  return;
	}
      CREATE (social, SOCIALTYPE, 1);
      social->name = str_dup (arg1);
      sprintf (arg2, "You %s.", arg1);
      social->char_no_arg = str_dup (arg2);
      add_social (social);
      send_to_char ("Social added.\n\r", ch);
      return;
    }

  if (!social)
    {
      send_to_char ("Social not found.\n\r", ch);
      return;
    }

  if (arg2[0] == '\0' || !str_cmp (arg2, "show"))
    {
      ch_printf (ch, "Social: %s\n\r\n\rCNoArg: %s\n\r",
		 social->name, social->char_no_arg);
      ch_printf (ch, "ONoArg: %s\n\rCFound: %s\n\rOFound: %s\n\r",
		 social->others_no_arg ? social->others_no_arg : "(not set)",
		 social->char_found ? social->char_found : "(not set)",
		 social->others_found ? social->others_found : "(not set)");
      ch_printf (ch, "VFound: %s\n\rCAuto : %s\n\rOAuto : %s\n\r",
		 social->vict_found ? social->vict_found : "(not set)",
		 social->char_auto ? social->char_auto : "(not set)",
		 social->others_auto ? social->others_auto : "(not set)");
      return;
    }
  if (get_trust (ch) > LEVEL_GOD && !str_cmp (arg2, "delete"))
    {
      unlink_social (social);
      free_social (social);
      send_to_char ("Deleted.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "cnoarg"))
    {
      if (argument[0] == '\0' || !str_cmp (argument, "clear"))
	{
	  send_to_char
	    ("You cannot clear this field.  It must have a message.\n\r", ch);
	  return;
	}
      if (social->char_no_arg)
	DISPOSE (social->char_no_arg);
      social->char_no_arg = str_dup (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "onoarg"))
    {
      if (social->others_no_arg)
	DISPOSE (social->others_no_arg);
      if (argument[0] != '\0' && str_cmp (argument, "clear"))
	social->others_no_arg = str_dup (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "cfound"))
    {
      if (social->char_found)
	DISPOSE (social->char_found);
      if (argument[0] != '\0' && str_cmp (argument, "clear"))
	social->char_found = str_dup (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "ofound"))
    {
      if (social->others_found)
	DISPOSE (social->others_found);
      if (argument[0] != '\0' && str_cmp (argument, "clear"))
	social->others_found = str_dup (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "vfound"))
    {
      if (social->vict_found)
	DISPOSE (social->vict_found);
      if (argument[0] != '\0' && str_cmp (argument, "clear"))
	social->vict_found = str_dup (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "cauto"))
    {
      if (social->char_auto)
	DISPOSE (social->char_auto);
      if (argument[0] != '\0' && str_cmp (argument, "clear"))
	social->char_auto = str_dup (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "oauto"))
    {
      if (social->others_auto)
	DISPOSE (social->others_auto);
      if (argument[0] != '\0' && str_cmp (argument, "clear"))
	social->others_auto = str_dup (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (get_trust (ch) > LEVEL_GREATER && !str_cmp (arg2, "name"))
    {
      bool relocate;

      one_argument (argument, arg1);
      if (arg1[0] == '\0')
	{
	  send_to_char ("Cannot clear name field!\n\r", ch);
	  return;
	}
      if (arg1[0] != social->name[0])
	{
	  unlink_social (social);
	  relocate = TRUE;
	}
      else
	relocate = FALSE;
      if (social->name)
	DISPOSE (social->name);
      social->name = str_dup (arg1);
      if (relocate)
	add_social (social);
      send_to_char ("Done.\n\r", ch);
      return;
    }

  /* display usage message */
  do_sedit (ch, "");
}

/*
 * Free a command structure					-Thoric
 */
void
free_command (CMDTYPE * command)
{
  if (command->name)
    DISPOSE (command->name);
  DISPOSE (command);
}

/*
 * Remove a command from it's hash index			-Thoric
 */
void
unlink_command (CMDTYPE * command)
{
  CMDTYPE *tmp, *tmp_next;
  int hash;

  if (!command)
    {
      bug ("Unlink_command NULL command", 0);
      return;
    }

  hash = command->name[0] % 126;

  if (command == (tmp = command_hash[hash]))
    {
      command_hash[hash] = tmp->next;
      return;
    }
  for (; tmp; tmp = tmp_next)
    {
      tmp_next = tmp->next;
      if (command == tmp_next)
	{
	  tmp->next = tmp_next->next;
	  return;
	}
    }
}

/*
 * Add a command to the command hash table			-Thoric
 */
void
add_command (CMDTYPE * command)
{
  int hash, x;
  CMDTYPE *tmp, *prev;

  if (!command)
    {
      bug ("Add_command: NULL command", 0);
      return;
    }

  if (!command->name)
    {
      bug ("Add_command: NULL command->name", 0);
      return;
    }

  if (!command->do_fun)
    {
      bug ("Add_command: NULL command->do_fun", 0);
      return;
    }

  /* make sure the name is all lowercase */
  for (x = 0; command->name[x] != '\0'; x++)
    command->name[x] = LOWER (command->name[x]);

  hash = command->name[0] % 126;

  if ((prev = tmp = command_hash[hash]) == NULL)
    {
      command->next = command_hash[hash];
      command_hash[hash] = command;
      return;
    }

  /* add to the END of the list */
  for (; tmp; tmp = tmp->next)
    if (!tmp->next)
      {
	tmp->next = command;
	command->next = NULL;
      }
  return;
}

/*
 * Command editor/displayer/save/delete				-Thoric
 * Added support for interpret flags                            -Shaddai
 */
void
do_cedit (CHAR_DATA * ch, char *argument)
{
  CMDTYPE *command;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  set_char_color (AT_IMMORT, ch);

  smash_tilde (argument);
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (arg1[0] == '\0')
    {
      send_to_char ("Syntax: cedit save cmdtable\n\r", ch);
      if (get_trust (ch) > LEVEL_SUB_IMPLEM || !str_cmp (ch->name, "Nivek"))
	{
	  send_to_char ("Syntax: cedit <command> create [code]\n\r", ch);
	  send_to_char ("Syntax: cedit <command> delete\n\r", ch);
	  send_to_char ("Syntax: cedit <command> show\n\r", ch);
	  send_to_char ("Syntax: cedit <command> raise\n\r", ch);
	  send_to_char ("Syntax: cedit <command> lower\n\r", ch);
	  send_to_char ("Syntax: cedit <command> list\n\r", ch);
	  send_to_char ("Syntax: cedit <command> [field]\n\r", ch);
	  send_to_char ("\n\rField being one of:\n\r", ch);
	  send_to_char ("  level position log code flags\n\r", ch);
	}
      return;
    }

  if ((get_trust (ch) > LEVEL_GREATER || !str_cmp (ch->name, "Edmond"))
      && !str_cmp (arg1, "save") && !str_cmp (arg2, "cmdtable"))
    {
      save_commands ();
      send_to_char ("Saved.\n\r", ch);
      return;
    }

  command = find_command (arg1);
  if ((!str_cmp (ch->name, "Nivek") || get_trust (ch) > LEVEL_SUB_IMPLEM)
      && !str_cmp (arg2, "create"))
    {
      if (command && !str_cmp (command->name, arg1))
	{
	  send_to_char ("That command already exists!\n\r", ch);
	  return;
	}
      CREATE (command, CMDTYPE, 1);
      command->lag_count = 0;	/* FB */
      command->name = str_dup (arg1);
      command->level = get_trust (ch);
      if (*argument)
	one_argument (argument, arg2);
      else
	sprintf (arg2, "do_%s", arg1);
      command->do_fun = skill_function (arg2);
      add_command (command);
      send_to_char ("Command added.\n\r", ch);
      if (command->do_fun == skill_notfound)
	ch_printf (ch, "Code %s not found.  Set to no code.\n\r", arg2);
      return;
    }

  if (!command)
    {
      send_to_char ("Command not found.\n\r", ch);
      return;
    }
  else if (command->level > get_trust (ch) && str_cmp (ch->name, "Nivek"))
    {
      send_to_char ("You cannot touch this command.\n\r", ch);
      return;
    }

  if (arg2[0] == '\0' || !str_cmp (arg2, "show"))
    {
      ch_printf (ch,
		 "Command:  %s\n\rLevel:    %d\n\rPosition: %d\n\rLog:      %d\n\rCode:     %s\n\rFlags:  %s\n\r",
		 command->name, command->level, command->position,
		 command->log, skill_name (command->do_fun),
		 flag_string (command->flags, cmd_flags));
      if (command->userec.num_uses)
	send_timer (&command->userec, ch);
      return;
    }

  if (get_trust (ch) <= LEVEL_SUB_IMPLEM && str_cmp (ch->name, "Nivek"))
    {
      do_cedit (ch, "");
      return;
    }

  if (!str_cmp (arg2, "raise"))
    {
      CMDTYPE *tmp, *tmp_next;
      int hash = command->name[0] % 126;

      if ((tmp = command_hash[hash]) == command)
	{
	  send_to_char ("That command is already at the top.\n\r", ch);
	  return;
	}
      if (tmp->next == command)
	{
	  command_hash[hash] = command;
	  tmp_next = tmp->next;
	  tmp->next = command->next;
	  command->next = tmp;
	  ch_printf (ch, "Moved %s above %s.\n\r", command->name,
		     command->next->name);
	  return;
	}
      for (; tmp; tmp = tmp->next)
	{
	  tmp_next = tmp->next;
	  if (tmp_next->next == command)
	    {
	      tmp->next = command;
	      tmp_next->next = command->next;
	      command->next = tmp_next;
	      ch_printf (ch, "Moved %s above %s.\n\r", command->name,
			 command->next->name);
	      return;
	    }
	}
      send_to_char ("ERROR -- Not Found!\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "lower"))
    {
      CMDTYPE *tmp, *tmp_next;
      int hash = command->name[0] % 126;

      if (command->next == NULL)
	{
	  send_to_char ("That command is already at the bottom.\n\r", ch);
	  return;
	}
      tmp = command_hash[hash];
      if (tmp == command)
	{
	  tmp_next = tmp->next;
	  command_hash[hash] = command->next;
	  command->next = tmp_next->next;
	  tmp_next->next = command;

	  ch_printf (ch, "Moved %s below %s.\n\r", command->name,
		     tmp_next->name);
	  return;
	}
      for (; tmp; tmp = tmp->next)
	{
	  if (tmp->next == command)
	    {
	      tmp_next = command->next;
	      tmp->next = tmp_next;
	      command->next = tmp_next->next;
	      tmp_next->next = command;

	      ch_printf (ch, "Moved %s below %s.\n\r", command->name,
			 tmp_next->name);
	      return;
	    }
	}
      send_to_char ("ERROR -- Not Found!\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "list"))
    {
      CMDTYPE *tmp;
      int hash = command->name[0] % 126;

      pager_printf (ch, "Priority placement for [%s]:\n\r", command->name);
      for (tmp = command_hash[hash]; tmp; tmp = tmp->next)
	{
	  if (tmp == command)
	    set_pager_color (AT_GREEN, ch);
	  else
	    set_pager_color (AT_PLAIN, ch);
	  pager_printf (ch, "  %s\n\r", tmp->name);
	}
      return;
    }
  if (!str_cmp (arg2, "delete"))
    {
      unlink_command (command);
      free_command (command);
      send_to_char ("Deleted.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "code"))
    {
      DO_FUN *fun = skill_function (argument);

      if (fun == skill_notfound)
	{
	  send_to_char ("Code not found.\n\r", ch);
	  return;
	}
      command->do_fun = fun;
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "level"))
    {
      int level = atoi (argument);

      if ((level < 0 || level > get_trust (ch))
	  && str_cmp (ch->name, "Nivek"))
	{
	  send_to_char ("Level out of range.\n\r", ch);
	  return;
	}
      command->level = level;
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "log"))
    {
      int log = atoi (argument);

      if (log < 0 || log > LOG_COMM)
	{
	  send_to_char ("Log out of range.\n\r", ch);
	  return;
	}
      command->log = log;
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "position"))
    {
      int position = atoi (argument);

      if (position < 0 || position > POS_DRAG)
	{
	  send_to_char ("Position out of range.\n\r", ch);
	  return;
	}
      command->position = position;
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "flags"))
    {
      int flag;
      if (is_number (argument))
	flag = atoi (argument);
      else
	flag = get_cmdflag (argument);
      if (flag < 0 || flag >= 32)
	{
	  if (is_number (argument))
	    ch_printf (ch, "Invalid flag: range is from 0 to 31.\n");
	  else
	    ch_printf (ch, "Unknown flag %s.\n", argument);
	  return;
	}

      TOGGLE_BIT (command->flags, 1 << flag);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "name"))
    {
      bool relocate;

      one_argument (argument, arg1);
      if (arg1[0] == '\0')
	{
	  send_to_char ("Cannot clear name field!\n\r", ch);
	  return;
	}
      if (arg1[0] != command->name[0])
	{
	  unlink_command (command);
	  relocate = TRUE;
	}
      else
	relocate = FALSE;
      if (command->name)
	DISPOSE (command->name);
      command->name = str_dup (arg1);
      if (relocate)
	add_command (command);
      send_to_char ("Done.\n\r", ch);
      return;
    }

  /* display usage message */
  do_cedit (ch, "");
}

/*
 * Display class information					-Thoric
 */
void
do_showclass (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  struct class_type *class;
  int cl, low, hi;

  set_pager_color (AT_PLAIN, ch);

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (arg1[0] == '\0')
    {
      send_to_char ("Syntax: showclass <class> [level range]\n\r", ch);
      return;
    }
  if (is_number (arg1) && (cl = atoi (arg1)) >= 0 && cl < MAX_CLASS)
    class = class_table[cl];
  else
    {
      class = NULL;
      for (cl = 0; cl < MAX_CLASS && class_table[cl]; cl++)
	if (!str_cmp (class_table[cl]->who_name, arg1))
	  {
	    class = class_table[cl];
	    break;
	  }
    }
  if (!class)
    {
      send_to_char ("No such class.\n\r", ch);
      return;
    }
  pager_printf_color (ch,
		      "&wCLASS: &W%s\n\r&wPrime Attribute: &W%-14s  &wWeapon: &W%-5d      &wGuild: &W%-5d\n\r",
		      class->who_name, affect_loc_name (class->attr_prime),
		      class->weapon, class->guild);
  pager_printf_color (ch,
		      "&wSecond Attribute:  &W%-14s  &wDeficient Attribute:  &W%-14s\n\r",
		      affect_loc_name (class->attr_second),
		      affect_loc_name (class->attr_deficient));
  pager_printf_color (ch,
		      "&wMax Skill Adept: &W%-3d             &wThac0 : &W%-5d     &wThac32: &W%d\n\r",
		      class->skill_adept, class->thac0_00, class->thac0_32);
  pager_printf_color (ch,
		      "&wHp Min/Hp Max  : &W%-2d/%-2d           &wMana  : &W%-3s      &wExpBase: &W%d\n\r",
		      class->hp_min, class->hp_max,
		      class->fMana ? "yes" : "no ", class->exp_base);
  pager_printf_color (ch, "&wAffected by:  &W%s\n\r",
		      affect_bit_name (&class->affected));
  pager_printf_color (ch, "&wResistant to: &W%s\n\r",
		      flag_string (class->resist, ris_flags));
  pager_printf_color (ch, "&wSusceptible to: &W%s\n\r",
		      flag_string (class->suscept, ris_flags));
  pager_printf_color (ch, "&wLogin: &W%s\n\r",
		      class->login ? class->login : "Default");
  pager_printf_color (ch, "&wLoginother: &W%s\n\r",
		      class->login_other ? class->login_other : "Default");
  pager_printf_color (ch, "&wLogout: &W%s\n\r",
		      class->logout ? class->logout : "Default");
  pager_printf_color (ch, "&wLogoutother: &W%s\n\r",
		      class->logout_other ? class->logout_other : "Default");
  pager_printf_color (ch, "&wReconnect: &W%s\n\r",
		      class->reconnect ? class->reconnect : "Default");
  pager_printf_color (ch, "&wReconnectother: &W%s\n\r",
		      class->
		      reconnect_other ? class->reconnect_other : "Default");

  if (arg2[0] != '\0')
    {
      int x, y, cnt;

      low = UMAX (0, atoi (arg2));
      hi = URANGE (low, atoi (argument), MAX_LEVEL);
      for (x = low; x <= hi; x++)
	{
	  set_pager_color (AT_LBLUE, ch);
	  pager_printf (ch, "Male: %-30s Female: %s\n\r",
			title_table[cl][x][0], title_table[cl][x][1]);
	  cnt = 0;
	  set_pager_color (AT_BLUE, ch);
	  for (y = gsn_first_spell; y < gsn_top_sn; y++)
	    if (skill_table[y]->skill_level[cl] == x)
	      {
		pager_printf (ch, "  %-7s %-19s%3d     ",
			      skill_tname[skill_table[y]->type],
			      skill_table[y]->name,
			      skill_table[y]->skill_adept[cl]);
		if (++cnt % 2 == 0)
		  send_to_pager ("\n\r", ch);
	      }
	  if (cnt % 2 != 0)
	    send_to_pager ("\n\r", ch);
	  send_to_pager ("\n\r", ch);
	}
    }
}

/*
 * Create a new class online.				    	-Shaddai
 */
bool
create_new_class (int index, char *argument)
{
  int i;

  if (index >= MAX_CLASS || class_table[index] == NULL)
    return FALSE;
  if (class_table[index]->who_name)
    STRFREE (class_table[index]->who_name);
  if (argument[0] != '\0')
    argument[0] = UPPER (argument[0]);
  class_table[index]->who_name = STRALLOC (argument);
  xCLEAR_BITS (class_table[index]->affected);
  class_table[index]->attr_prime = 0;
  class_table[index]->attr_second = 0;
  class_table[index]->attr_deficient = 0;
  class_table[index]->login = NULL;
  class_table[index]->logout = NULL;
  class_table[index]->login_other = NULL;
  class_table[index]->logout_other = NULL;
  class_table[index]->reconnect = NULL;
  class_table[index]->reconnect_other = NULL;
  class_table[index]->resist = 0;
  class_table[index]->suscept = 0;
  class_table[index]->weapon = 0;
  class_table[index]->guild = 0;
  class_table[index]->skill_adept = 0;
  class_table[index]->thac0_00 = 0;
  class_table[index]->thac0_32 = 0;
  class_table[index]->hp_min = 0;
  class_table[index]->hp_max = 0;
  class_table[index]->fMana = FALSE;
  class_table[index]->exp_base = 0;
  for (i = 0; i < MAX_LEVEL; i++)
    {
      title_table[index][i][0] = str_dup ("Not set.");
      title_table[index][i][1] = str_dup ("Not set.");
    }
  return TRUE;
}

/*
 * Edit class information					-Thoric
 * Fixed bug that Charisma can't be set as an attribute.
 * Reported by ospiosis@interlog.com -- Shaddai
 */
void
do_setclass (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  FILE *fpList;
  char classlist[256];
  struct class_type *class;
  int cl, value, i;

  set_char_color (AT_PLAIN, ch);

  smash_tilde (argument);
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (arg1[0] == '\0')
    {
      send_to_char ("Syntax: setclass <class> <field> <value>\n\r", ch);
      send_to_char ("Syntax: setclass <class> create\n\r", ch);
      send_to_char ("\n\rField being one of:\n\r", ch);
      send_to_char ("  name prime weapon guild thac0 thac32\n\r", ch);
      send_to_char ("  hpmin hpmax mana expbase mtitle ftitle\n\r", ch);
      send_to_char ("  second, deficient affected resist suscept\n\r", ch);
      send_to_char ("  login, loginother, logout, logoutother\n\r", ch);
      send_to_char ("  reconnect, reconnectother\n\r", ch);
      return;
    }
  if (is_number (arg1) && (cl = atoi (arg1)) >= 0 && cl < MAX_CLASS)
    class = class_table[cl];
  else
    {
      class = NULL;
      for (cl = 0; cl < MAX_CLASS && class_table[cl]; cl++)
	{
	  if (!class_table[cl]->who_name)
	    continue;
	  if (!str_cmp (class_table[cl]->who_name, arg1))
	    {
	      class = class_table[cl];
	      break;
	    }
	}
    }
  if (!str_cmp (arg2, "create") && class)
    {
      send_to_char ("That class already exists!\n\r", ch);
      return;
    }

  if (!class && str_cmp (arg2, "create"))
    {
      send_to_char ("No such class.\n\r", ch);
      return;
    }

  if (!str_cmp (arg2, "save"))
    {
      write_class_file (cl);
      send_to_char ("Saved.\n\r", ch);
      return;
    }


  if (!str_cmp (arg2, "create"))
    {
      if (MAX_PC_CLASS >= MAX_CLASS)
	{
	  send_to_char ("You need to up MAX_CLASS in mud and make clean.\n\r",
			ch);
	  return;
	}
      if ((create_new_class (MAX_PC_CLASS, arg1)) == FALSE)
	{
	  send_to_char ("Couldn't create a new class.\n\r", ch);
	  return;
	}
      write_class_file (MAX_PC_CLASS);
      MAX_PC_CLASS++;
      sprintf (classlist, "%s%s", CLASS_DIR, CLASS_LIST);

      if ((fpList = fopen (classlist, "w")) == NULL)
	{
	  bug ("Can't open Class list for writing.", 0);
	  return;
	}

      for (i = 0; i < MAX_PC_CLASS; i++)
	fprintf (fpList, "%s%s.class\n", CLASS_DIR, class_table[i]->who_name);

      fprintf (fpList, "$\n");
      fclose (fpList);
      send_to_char ("Done.\n\r", ch);
      return;
    }

  if (!argument)
    {
      send_to_char ("You must specify an argument.\n\r", ch);
      return;
    }

  if (!str_cmp (arg2, "name"))
    {
      STRFREE (class->who_name);
      class->who_name = STRALLOC (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "login"))
    {
      if (class->login)
	DISPOSE (class->login);
      if (argument)
	class->login = str_dup (argument);
      else
	class->login = NULL;
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "loginother"))
    {
      if (class->login_other)
	DISPOSE (class->login_other);
      if (argument)
	class->login_other = str_dup (argument);
      else
	class->login_other = NULL;
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "logout"))
    {
      if (class->logout)
	DISPOSE (class->logout);
      if (argument)
	class->logout = str_dup (argument);
      else
	class->logout = NULL;
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "logoutother"))
    {
      if (class->logout_other)
	DISPOSE (class->logout_other);
      if (argument)
	class->logout_other = str_dup (argument);
      else
	class->logout_other = NULL;
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "reconnect"))
    {
      if (class->reconnect)
	DISPOSE (class->reconnect);
      if (argument)
	class->reconnect = str_dup (argument);
      else
	class->reconnect = NULL;
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "reconnectother"))
    {
      if (class->reconnect_other)
	DISPOSE (class->reconnect_other);
      if (argument)
	class->reconnect_other = str_dup (argument);
      else
	class->reconnect_other = NULL;
      send_to_char ("Done.\n\r", ch);
      return;
    }

  if (!str_cmp (arg2, "second"))
    {
      int x = get_atype (argument);

      if (x < APPLY_STR
	  || (x > APPLY_CON && x != APPLY_LCK && x != APPLY_CHA))
	send_to_char ("Invalid second attribute!\n\r", ch);
      else
	{
	  class->attr_second = x;
	  send_to_char ("Done.\n\r", ch);
	}
      return;
    }

  if (!str_cmp (arg2, "affected"))
    {
      if (!argument || argument[0] == '\0')
	{
	  send_to_char
	    ("Usage: setclass <class> affected <flag> [flag]...\n\r", ch);
	  return;
	}
      while (argument[0] != '\0')
	{
	  argument = one_argument (argument, arg2);
	  value = get_aflag (arg2);
	  if (value < 0 || value > MAX_BITS)
	    ch_printf (ch, "Unknown flag: %s\n\r", arg2);
	  else
	    xTOGGLE_BIT (class->affected, value);
	}
      send_to_char ("Done.\n\r", ch);
      return;
    }

  if (!str_cmp (arg2, "resist"))
    {
      if (!argument || argument[0] == '\0')
	{
	  send_to_char ("Usage: setclass <class> resist <flag> [flag]...\n\r",
			ch);
	  return;
	}
      while (argument[0] != '\0')
	{
	  argument = one_argument (argument, arg2);
	  value = get_risflag (arg2);
	  if (value < 0 || value > 31)
	    ch_printf (ch, "Unknown flag: %s\n\r", arg2);
	  else
	    TOGGLE_BIT (class->resist, 1 << value);
	}
      send_to_char ("Done.\n\r", ch);
      return;
    }

  if (!str_cmp (arg2, "suscept"))
    {
      if (!argument || argument[0] == '\0')
	{
	  send_to_char
	    ("Usage: setclass <class> suscept <flag> [flag]...\n\r", ch);
	  return;
	}
      while (argument[0] != '\0')
	{
	  argument = one_argument (argument, arg2);
	  value = get_risflag (arg2);
	  if (value < 0 || value > 31)
	    ch_printf (ch, "Unknown flag: %s\n\r", arg2);
	  else
	    TOGGLE_BIT (class->suscept, 1 << value);
	}
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "deficient"))
    {
      int x = get_atype (argument);

      if (x < APPLY_STR
	  || (x > APPLY_CON && x != APPLY_LCK && x != APPLY_CHA))
	send_to_char ("Invalid deficient attribute!\n\r", ch);
      else
	{
	  class->attr_deficient = x;
	  send_to_char ("Done.\n\r", ch);
	}
      return;
    }
  if (!str_cmp (arg2, "prime"))
    {
      int x = get_atype (argument);

      if (x < APPLY_STR
	  || (x > APPLY_CON && x != APPLY_LCK && x != APPLY_CHA))
	send_to_char ("Invalid prime attribute!\n\r", ch);
      else
	{
	  class->attr_prime = x;
	  send_to_char ("Done.\n\r", ch);
	}
      return;
    }
  if (!str_cmp (arg2, "weapon"))
    {
      class->weapon = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "guild"))
    {
      class->guild = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "thac0"))
    {
      class->thac0_00 = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "thac32"))
    {
      class->thac0_32 = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "hpmin"))
    {
      class->hp_min = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "hpmax"))
    {
      class->hp_max = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "mana"))
    {
      if (UPPER (argument[0]) == 'Y')
	class->fMana = TRUE;
      else
	class->fMana = FALSE;
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "expbase"))
    {
      class->exp_base = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "mtitle"))
    {
      char arg3[MAX_INPUT_LENGTH];
      int x;

      argument = one_argument (argument, arg3);
      if (arg3[0] == '\0' || argument[0] == '\0')
	{
	  send_to_char ("Syntax: setclass <class> mtitle <level> <title>\n\r",
			ch);
	  return;
	}
      if ((x = atoi (arg3)) < 0 || x > MAX_LEVEL)
	{
	  send_to_char ("Invalid level.\n\r", ch);
	  return;
	}
      DISPOSE (title_table[cl][x][0]);
      title_table[cl][x - 1][0] = str_dup (argument);
    }
  if (!str_cmp (arg2, "ftitle"))
    {
      char arg3[MAX_INPUT_LENGTH];
      char arg4[MAX_INPUT_LENGTH];
      int x, sex;

      argument = one_argument (argument, arg3);
      argument = one_argument (argument, arg4);
      if (arg3[0] == '\0' || argument[0] == '\0')
	{
	  send_to_char
	    ("Syntax: setclass <class> ftitle <level> <male/female> <title>\n\r",
	     ch);
	  return;
	}
      if ((x = atoi (arg4)) < 0 || x > MAX_LEVEL)
	{
	  send_to_char ("Invalid level.\n\r", ch);
	  return;
	}
      if (!str_cmp (arg4, "Male"))
	sex = 0;
      else
	sex = 1;
      DISPOSE (title_table[cl][x][sex]);
      /* Bug fix below -Shaddai */
      title_table[cl][x][sex] = str_dup (argument);
    }
  do_setclass (ch, "");
}


/*
 * Create an instance of a new race.			-Shaddai
 */

bool
create_new_race (int index, char *argument)
{
  int i = 0;
  if (index >= MAX_RACE || race_table[index] == NULL)
    return FALSE;
  //for ( i = 0; i <= MAX_WHERE_NAME; i++ )
  for (i = 0; i < MAX_WHERE_NAME; i++)
    race_table[index]->where_name[i] = where_name[i];
  if (argument[0] != '\0')
    argument[0] = UPPER (argument[0]);
  sprintf (race_table[index]->race_name, "%-.16s", argument);
  race_table[index]->class_restriction = 0;
  race_table[index]->str_plus = 0;
  race_table[index]->dex_plus = 0;
  race_table[index]->wis_plus = 0;
  race_table[index]->int_plus = 0;
  race_table[index]->con_plus = 0;
  race_table[index]->cha_plus = 0;
  race_table[index]->lck_plus = 0;
  race_table[index]->hit = 0;
  race_table[index]->mana = 0;
  xCLEAR_BITS (race_table[index]->affected);
  race_table[index]->resist = 0;
  race_table[index]->suscept = 0;
  race_table[index]->language = 0;
  race_table[index]->alignment = 0;
  race_table[index]->minalign = 0;
  race_table[index]->maxalign = 0;
  race_table[index]->ac_plus = 0;
  race_table[index]->exp_multiplier = 0;
  xCLEAR_BITS (race_table[index]->attacks);
  xCLEAR_BITS (race_table[index]->defenses);
  race_table[index]->height = 0;
  race_table[index]->weight = 0;
  race_table[index]->hunger_mod = 0;
  race_table[index]->thirst_mod = 0;
  race_table[index]->mana_regen = 0;
  race_table[index]->hp_regen = 0;
  race_table[index]->race_recall = 0;
  return TRUE;
}

void
do_setrace (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  FILE *fpList;
  char racelist[256];
  int value;

  char buf[MAX_STRING_LENGTH];
  struct race_type *race;
  int ra, i;

  set_char_color (AT_PLAIN, ch);

  smash_tilde (argument);
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  if (arg1[0] == '\0' || arg2[0] == '\0')
    {
      send_to_char ("Syntax: setrace <race> <field> <value>\n\r", ch);
      send_to_char ("Syntax: setrace <race> create	     \n\r", ch);
      send_to_char ("\n\rField being one of:\n\r", ch);
      send_to_char ("  name classes strplus dexplus wisplus\n\r", ch);
      send_to_char ("  intplus conplus chaplus lckplus hit\n\r", ch);
      send_to_char ("  mana affected resist suscept language\n\r", ch);
      send_to_char ("  save attack defense alignment acplus \n\r", ch);
      send_to_char ("  minalign maxalign height weight      \n\r", ch);
      send_to_char ("  hungermod thirstmod expmultiplier    \n\r", ch);
      send_to_char ("  saving_poison_death saving_wand      \n\r", ch);
      send_to_char ("  saving_para_petri saving_breath      \n\r", ch);
      send_to_char ("  saving_spell_staff  race_recall      \n\r", ch);
      send_to_char ("  mana_regen hp_regen                  \n\r", ch);
      return;
    }
  if (is_number (arg1) && (ra = atoi (arg1)) >= 0 && ra < MAX_RACE)
    race = race_table[ra];
  else
    {
      race = NULL;
      for (ra = 0; ra < MAX_RACE && race_table[ra]; ra++)
	{
	  if (!race_table[ra]->race_name)
	    continue;
	  if (!str_cmp (race_table[ra]->race_name, arg1))
	    {
	      race = race_table[ra];
	      break;
	    }
	}
    }
  if (!str_cmp (arg2, "create") && race)
    {
      send_to_char ("That race already exists!\n\r", ch);
      return;
    }
  else if (!race && str_cmp (arg2, "create"))
    {
      send_to_char ("No such race.\n\r", ch);
      return;
    }



  if (!str_cmp (arg2, "save"))
    {
      write_race_file (ra);
      send_to_char ("Saved.\n\r", ch);
      return;
    }

  if (!str_cmp (arg2, "create"))
    {
      if (MAX_PC_RACE >= MAX_RACE)
	{
	  send_to_char
	    ("You need to up MAX_RACE in mud.h and make clean.\n\r", ch);
	  return;
	}
      if ((create_new_race (MAX_PC_RACE, arg1)) == FALSE)
	{
	  send_to_char ("Couldn't create a new race.\n\r", ch);
	  return;
	}
      write_race_file (MAX_PC_RACE);
      MAX_PC_RACE++;
      sprintf (racelist, "%s%s", RACE_DIR, RACE_LIST);
      if ((fpList = fopen (racelist, "w")) == NULL)
	{
	  bug ("Error opening racelist.", 0);
	  return;
	}
      for (i = 0; i < MAX_PC_RACE; i++)
	fprintf (fpList, "%s%s.race\n", RACE_DIR, race_table[i]->race_name);
      fprintf (fpList, "$\n");
      fclose (fpList);
      send_to_char ("Done.\n\r", ch);
      return;
    }

  if (!argument)
    {
      send_to_char ("You must specify an argument.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "name"))
    {
      sprintf (race->race_name, "%-.16s", argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }

  if (!str_cmp (arg2, "strplus"))
    {
      race->str_plus = (sh_int) atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "dexplus"))
    {
      race->dex_plus = (sh_int) atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "wisplus"))
    {
      sprintf (buf, "attempting to set wisplus to %s\n\r", argument);
      send_to_char (buf, ch);
      race->wis_plus = (sh_int) atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "intplus"))
    {
      race->int_plus = (sh_int) atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "conplus"))
    {
      race->con_plus = (sh_int) atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "chaplus"))
    {
      race->cha_plus = (sh_int) atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "lckplus"))
    {
      race->lck_plus = (sh_int) atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "hit"))
    {
      race->hit = (sh_int) atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "mana"))
    {
      race->mana = (sh_int) atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "affected"))
    {
      if (!argument || argument[0] == '\0')
	{
	  send_to_char ("Usage: setrace <race> affected <flag> [flag]...\n\r",
			ch);
	  return;
	}
      while (argument[0] != '\0')
	{
	  argument = one_argument (argument, arg3);
	  value = get_aflag (arg3);
	  if (value < 0 || value > MAX_BITS)
	    ch_printf (ch, "Unknown flag: %s\n\r", arg3);
	  else
	    xTOGGLE_BIT (race->affected, value);
	}
      send_to_char ("Done.\n\r", ch);
      return;
    }

  if (!str_cmp (arg2, "resist"))
    {
      if (!argument || argument[0] == '\0')
	{
	  send_to_char ("Usage: setrace <race> resist <flag> [flag]...\n\r",
			ch);
	  return;
	}
      while (argument[0] != '\0')
	{
	  argument = one_argument (argument, arg3);
	  value = get_risflag (arg3);
	  if (value < 0 || value > 31)
	    ch_printf (ch, "Unknown flag: %s\n\r", arg3);
	  else
	    TOGGLE_BIT (race->resist, 1 << value);
	}
      send_to_char ("Done.\n\r", ch);
      return;
    }

  if (!str_cmp (arg2, "suscept"))
    {
      if (!argument || argument[0] == '\0')
	{
	  send_to_char ("Usage: setrace <race> suscept <flag> [flag]...\n\r",
			ch);
	  return;
	}
      while (argument[0] != '\0')
	{
	  argument = one_argument (argument, arg3);
	  value = get_risflag (arg3);
	  if (value < 0 || value > 31)
	    ch_printf (ch, "Unknown flag: %s\n\r", arg3);
	  else
	    TOGGLE_BIT (race->suscept, 1 << value);
	}
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "language"))
    {
      race->language = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }

  if (!str_cmp (arg2, "classes"))
    {
      for (i = 0; i < MAX_CLASS; i++)
	{
	  if (!str_cmp (argument, class_table[i]->who_name))
	    {
	      TOGGLE_BIT (race->class_restriction, 1 << i);	/* k, that's boggling */
	      send_to_char ("Done.\n\r", ch);
	      return;
	    }
	}
      send_to_char ("No such class.\n\r", ch);
      return;
    }


  if (!str_cmp (arg2, "acplus"))
    {
      race->ac_plus = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }

  if (!str_cmp (arg2, "alignment"))
    {
      race->alignment = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }

  /* not implemented */
  if (!str_cmp (arg2, "defense"))
    {
      if (!argument || argument[0] == '\0')
	{
	  send_to_char ("Usage: setrace <race> defense <flag> [flag]...\n\r",
			ch);
	  return;
	}
      while (argument[0] != '\0')
	{
	  argument = one_argument (argument, arg3);
	  value = get_defenseflag (arg3);
	  if (value < 0 || value > MAX_BITS)
	    ch_printf (ch, "Unknown flag: %s\n\r", arg3);
	  else
	    xTOGGLE_BIT (race->defenses, value);
	}
      return;
    }

  /* not implemented */
  if (!str_cmp (arg2, "attack"))
    {
      if (!argument || argument[0] == '\0')
	{
	  send_to_char ("Usage: setrace <race> attack <flag> [flag]...\n\r",
			ch);
	  return;
	}
      while (argument[0] != '\0')
	{
	  argument = one_argument (argument, arg3);
	  value = get_attackflag (arg3);
	  if (value < 0 || value > MAX_BITS)
	    ch_printf (ch, "Unknown flag: %s\n\r", arg3);
	  else
	    xTOGGLE_BIT (race->attacks, value);
	}
      return;
    }


  if (!str_cmp (arg2, "minalign"))
    {
      race->minalign = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "maxalign"))
    {
      race->maxalign = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "height"))
    {
      race->height = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "weight"))
    {
      race->weight = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "thirstmod"))
    {
      race->thirst_mod = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "hungermod"))
    {
      race->hunger_mod = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "maxalign"))
    {
      race->maxalign = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "expmultiplier"))
    {
      race->exp_multiplier = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "saving_poison_death"))
    {
      race->saving_poison_death = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "saving_wand"))
    {
      race->saving_wand = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "saving_para_petri"))
    {
      race->saving_para_petri = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "saving_breath"))
    {
      race->saving_breath = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "saving_spell_staff"))
    {
      race->saving_spell_staff = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  /* unimplemented stuff follows */
  if (!str_cmp (arg2, "mana_regen"))
    {
      race->mana_regen = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "hp_regen"))
    {
      race->hp_regen = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "race_recall"))
    {
      race->race_recall = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }

#ifdef NEW_RACE_STUFF
  if (!str_cmp (arg2, "carry_weight"))
    {
      race->acplus = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg2, "carry_number"))
    {
      race->acplus = atoi (argument);
      send_to_char ("Done.\n\r", ch);
      return;
    }
#endif
  do_setrace (ch, "");

}


void
do_showrace (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  struct race_type *race;
  int ra, i, ct;

  set_pager_color (AT_PLAIN, ch);

  argument = one_argument (argument, arg1);
  if (arg1[0] == '\0')
    {
      send_to_char ("Syntax: showrace <race> \n\r", ch);
      /* Show the races code addition by Blackmane */
      ct = 0;
      for (i = 0; i < MAX_RACE; i++)
	{
	  ++ct;
	  pager_printf (ch, "%2d> %-11s", i, race_table[i]->race_name);
	  if (ct % 5 == 0)
	    send_to_pager ("\n\r", ch);
	}
      send_to_pager ("\n\r", ch);
      return;
    }
  if (is_number (arg1) && (ra = atoi (arg1)) >= 0 && ra < MAX_RACE)
    race = race_table[ra];
  else
    {
      race = NULL;
      for (ra = 0; ra < MAX_RACE && race_table[ra]; ra++)
	if (!str_cmp (race_table[ra]->race_name, arg1))
	  {
	    race = race_table[ra];
	    break;
	  }
    }
  if (!race)
    {
      send_to_char ("No such race.\n\r", ch);
      return;
    }

  sprintf (buf, "RACE: %s\n\r", race->race_name);
  send_to_char (buf, ch);
  ct = 0;
  sprintf (buf, "Disallowed Classes: ");
  send_to_char (buf, ch);
  for (i = 0; i < MAX_CLASS; i++)
    {
      if (IS_SET (race->class_restriction, 1 << i))
	{
	  ct++;
	  sprintf (buf, "%s ", class_table[i]->who_name);
	  send_to_char (buf, ch);
	  if (ct % 6 == 0)
	    send_to_char ("\n\r", ch);
	}
    }
  if ((ct % 6 != 0) || (ct == 0))
    send_to_char ("\n\r", ch);

  ct = 0;
  sprintf (buf, "Allowed Classes: ");
  send_to_char (buf, ch);
  for (i = 0; i < MAX_CLASS; i++)
    {
      if (!IS_SET (race->class_restriction, 1 << i))
	{
	  ct++;
	  sprintf (buf, "%s ", class_table[i]->who_name);
	  send_to_char (buf, ch);
	  if (ct % 6 == 0)
	    send_to_char ("\n\r", ch);
	}
    }
  if ((ct % 6 != 0) || (ct == 0))
    send_to_char ("\n\r", ch);



  sprintf (buf,
	   "Str Plus: %-3d\tDex Plus: %-3d\tWis Plus: %-3d\tInt Plus: %-3d\t\n\r",
	   race->str_plus, race->dex_plus, race->wis_plus, race->int_plus);
  send_to_char (buf, ch);
  sprintf (buf, "Con Plus: %-3d\tCha Plus: %-3d\tLck Plus: %-3d\n\r",
	   race->con_plus, race->cha_plus, race->lck_plus);
  send_to_char (buf, ch);
  sprintf (buf, "Hit Pts:  %-3d\tMana: %-3d\tAlign: %-4d\tAC: %-d\n\r",
	   race->hit, race->mana, race->alignment, race->ac_plus);
  send_to_char (buf, ch);
  sprintf (buf, "Min Align: %d\tMax Align: %-d\t\tXP Mult: %-d%%\n\r",
	   race->minalign, race->maxalign, race->exp_multiplier);
  send_to_char (buf, ch);
  sprintf (buf,
	   "Height: %3d in.\t\tWeight: %4d lbs.\tHungerMod: %d\tThirstMod: %d\n\r",
	   race->height, race->weight, race->hunger_mod, race->thirst_mod);
  send_to_char (buf, ch);

  send_to_char ("Affected by: ", ch);
  send_to_char (affect_bit_name (&race->affected), ch);
  send_to_char ("\n\r", ch);

  send_to_char ("Resistant to: ", ch);
  send_to_char (flag_string (race->resist, ris_flags), ch);
  send_to_char ("\n\r", ch);

  send_to_char ("Susceptible to: ", ch);
  send_to_char (flag_string (race->suscept, ris_flags), ch);
  send_to_char ("\n\r", ch);

  sprintf (buf, "Saves: (P/D) %d (W) %d (P/P) %d (B) %d (S/S) %d\n\r",
	   race->saving_poison_death,
	   race->saving_wand,
	   race->saving_para_petri,
	   race->saving_breath, race->saving_spell_staff);
  send_to_char (buf, ch);

  send_to_char ("Innate Attacks: ", ch);
  send_to_char (ext_flag_string (&race->attacks, attack_flags), ch);
  send_to_char ("\n\r", ch);

  send_to_char ("Innate Defenses: ", ch);
  send_to_char (ext_flag_string (&race->defenses, defense_flags), ch);
  send_to_char ("\n\r", ch);

}

/*
 * Randomize command.  SHADDAI
 * Right now it just randomizes stances could be used for other stuff
 * Later.
 */

void
do_randomize (CHAR_DATA * ch, char *argument)
{
  set_char_color (AT_IMMORT, ch);

  randomize_stances ();
  send_to_char ("Done.\n\r", ch);
  return;
}

/*
 * quest point set - TRI
 * syntax is: qpset char give/take amount
 */

void
do_qpset (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int amount;
  bool give = TRUE;

  set_char_color (AT_IMMORT, ch);

  if (IS_NPC (ch))
    {
      send_to_char ("Cannot qpset as an NPC.\n\r", ch);
      return;
    }
  if (get_trust (ch) < LEVEL_IMMORTAL)
    {
      send_to_char ("Huh?\n\r", ch);
      return;
    }

  argument = one_argument (argument, arg);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);
  amount = atoi (arg3);
  if (arg[0] == '\0' || arg2[0] == '\0' || amount <= 0)
    {
      send_to_char ("Syntax: qpset <character> <give/take> <amount>\n\r", ch);
      send_to_char ("Amount must be a positive number greater than 0.\n\r",
		    ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("There is no such player currently playing.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Glory cannot be given to or taken from a mob.\n\r", ch);
      return;
    }

  set_char_color (AT_IMMORT, victim);
  if (nifty_is_name_prefix (arg2, "give"))
    {
      give = TRUE;
      if (str_cmp (ch->pcdata->council_name, "Quest Council")
	  && (get_trust (ch) < LEVEL_DEMI))
	{
	  send_to_char
	    ("You must be a member of the Quest Council to give qp to a character.\n\r",
	     ch);
	  return;
	}
    }
  else if (nifty_is_name_prefix (arg2, "take"))
    give = FALSE;
  else
    {
      do_qpset (ch, "");
      return;
    }

  if (give)
    {
      victim->pcdata->quest_curr += amount;
      victim->pcdata->quest_accum += amount;
      ch_printf (victim, "Your glory has been increased by %d.\n\r", amount);
      ch_printf (ch, "You have increased the glory of %s by %d.\n\r",
		 victim->name, amount);
      save_char_obj (victim);
    }
  else
    {
      if (victim->pcdata->quest_curr - amount < 0)
	{
	  ch_printf (ch, "%s does not have %d glory to take.\n\r",
		     victim->name, amount);
	  return;
	}
      else
	{
	  victim->pcdata->quest_curr -= amount;
	  ch_printf (victim, "Your glory has been decreased by %d.\n\r",
		     amount);
	  ch_printf (ch, "You have decreased the glory of %s by %d.\n\r",
		     victim->name, amount);
	  save_char_obj (victim);
	}
    }
  return;
}

/* Easy way to check a player's glory -- Blodkai, June 97 */
void
do_qpstat (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  set_char_color (AT_IMMORT, ch);

  if (IS_NPC (ch))
    return;

  argument = one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Syntax:  qpstat <character>\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("No one by that name currently in the Realms.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("Mobs don't have glory.\n\r", ch);
      return;
    }
  ch_printf (ch, "%s has %d glory, out of a lifetime total of %d.\n\r",
	     victim->name,
	     victim->pcdata->quest_curr, victim->pcdata->quest_accum);
  return;
}

/* The following two commands could be a lot more flexible and neat, but it
would be overkill when this works well -- Blod */
void
do_add_change (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  struct tm *t = localtime (&current_time);

  if (argument[0] == '\0')
    {
      send_to_char_color ("\n\r&G&gUsage:  addchange <text>", ch);
      if (get_trust (ch) >= LEVEL_INFINITE)
	send_to_char_color ("&G&g or 'addchange clear now'.\n\r", ch);
      else
	send_to_char ("\n\r", ch);
      return;
    }

  if (!str_cmp (argument, "clear now") && get_trust (ch) >= LEVEL_INFINITE)
    {
      FILE *fp = fopen (CHANGE_FILE, "w");
      if (fp)
	fclose (fp);
      send_to_char_color ("&G&gChange file cleared.\n\r", ch);
      return;
    }

  sprintf (buf, "&G&g| &w%-2.2d/%-2.2d &g| &w%s",
	   t->tm_mon + 1, t->tm_mday, argument);
  append_to_file (CHANGE_FILE, buf);
  ch_printf_color (ch, "&G&gYou have added the following change:\n\r&w%s\n\r",
		   argument);
  return;
}

void
do_add_imm_news (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  struct tm *t = localtime (&current_time);

  if (argument[0] == '\0')
    {
      send_to_char_color ("\n\r&G&cUsage:  addimmnews <text>", ch);
      if (get_trust (ch) >= LEVEL_ASCENDANT)
	send_to_char_color ("&G&c or 'addimmnews clear now'.\n\r", ch);
      else
	send_to_char ("\n\r", ch);
      return;
    }

  if (!str_cmp (argument, "clear now") && get_trust (ch) >= LEVEL_INFINITE)
    {
      FILE *fp = fopen (IMMNEWS_FILE, "w");
      if (fp)
	fclose (fp);
      send_to_char_color ("&G&cImmnews file cleared.\n\r", ch);
      return;
    }

  sprintf (buf, "&G&c| &w%-2.2d/%-2.2d &c| &w%s",
	   t->tm_mon + 1, t->tm_mday, argument);
  append_to_file (IMMNEWS_FILE, buf);
  ch_printf_color (ch,
		   "&G&cYou have added the following Immortal news item:\n\r&w%s\n\r",
		   argument);
  return;
}

void
do_add_news (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  struct tm *t = localtime (&current_time);

  if (argument[0] == '\0')
    {
      send_to_char_color ("\n\r&G&gUsage:  addnews <text>", ch);
      if (get_trust (ch) >= LEVEL_INFINITE)
	send_to_char_color ("&G&g or 'addnews clear now'.\n\r", ch);
      else
	send_to_char ("\n\r", ch);
      return;
    }

  if (!str_cmp (argument, "clear now") && get_trust (ch) >= LEVEL_INFINITE)
    {
      FILE *fp = fopen (NEWS_FILE, "w");
      if (fp)
	fclose (fp);
      send_to_char_color ("&G&gNews file cleared.\n\r", ch);
      return;
    }

  sprintf (buf, "&G&g| &w%-2.2d/%-2.2d &g| &w%s",
	   t->tm_mon + 1, t->tm_mday, argument);
  append_to_file (NEWS_FILE, buf);
  ch_printf_color (ch,
		   "&G&gYou have added the following news item:\n\r&w%s\n\r",
		   argument);
  return;
}


/* Simple, small way to make keeping track of small mods easier - Blod */
void
do_fixed (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  struct tm *t = localtime (&current_time);

  set_char_color (AT_OBJECT, ch);
  if (argument[0] == '\0')
    {
      send_to_char ("\n\rUsage:  'fixed list' or 'fixed <message>'", ch);
      if (get_trust (ch) >= LEVEL_ASCENDANT)
	send_to_char (" or 'fixed clear now'\n\r", ch);
      else
	send_to_char ("\n\r", ch);
      return;
    }
  if (!str_cmp (argument, "clear now") && get_trust (ch) >= LEVEL_ASCENDANT)
    {
      FILE *fp = fopen (FIXED_FILE, "w");
      if (fp)
	fclose (fp);
      send_to_char ("Fixed file cleared.\n\r", ch);
      return;
    }
  if (!str_cmp (argument, "list"))
    {
      send_to_char_color ("\n\r&g[&GDate  &g|  &GVnum&g]\n\r", ch);
      show_file (ch, FIXED_FILE);
    }
  else
    {
      sprintf (buf, "&g|&G%-2.2d/%-2.2d &g| &G%5d&g|  %s:  &G%s",
	       t->tm_mon + 1, t->tm_mday, ch->in_room ? ch->in_room->vnum : 0,
	       IS_NPC (ch) ? ch->short_descr : ch->name, argument);
      append_to_file (FIXED_FILE, buf);
      send_to_char ("Thanks, your modification has been logged.\n\r", ch);
    }
  return;
}

void
do_fshow (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];

  set_char_color (AT_IMMORT, ch);

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char
	("Syntax:  fshow <moblog | oinvoke | plevel | cutlink>\n\r", ch);
      return;
    }
  if (!str_cmp (arg, "moblog"))
    {
      set_char_color (AT_LOG, ch);
      send_to_char ("\n\r[Date_|_Time]  Current moblog:\n\r", ch);
      show_file (ch, MOBLOG_FILE);
      return;
    }
  if (!str_cmp (arg, "plevel"))
    {
      set_char_color (AT_NOTE, ch);
      show_file (ch, PLEVEL_FILE);
      return;
    }
  if (!str_cmp (arg, "oinvoke"))
    {
      set_char_color (AT_NOTE, ch);
      show_file (ch, OINVOKE_FILE);
      return;
    }
  if (!str_cmp (arg, "cutlink"))
    {
      set_char_color (AT_NOTE, ch);
      show_file (ch, CUTLINK_FILE);
      return;
    }
  send_to_char ("No such file.\n\r", ch);
  return;
}

NOAUCTION_DATA *first_noauc;
NOAUCTION_DATA *last_noauc;
void
save_noauctions (void)
{
  NOAUCTION_DATA *noauc;
  FILE *fp;

  fclose (fpReserve);
  if (!(fp = fopen (SYSTEM_DIR NOAUCTION_LIST, "w")))
    {
      bug ("Save_reserved: cannot open " NOAUCTION_LIST, 0);
      perror (NOAUCTION_LIST);
      fpReserve = fopen (NULL_FILE, "r");
      return;
    }
  for (noauc = first_noauc; noauc; noauc = noauc->next)
    fprintf (fp, "%d\n", noauc->vnum);
  fprintf (fp, "0\n");
  fclose (fp);
  fpReserve = fopen (NULL_FILE, "r");
  return;
}

void
do_noauction (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  int arg1;
  NOAUCTION_DATA *noauc;
  OBJ_INDEX_DATA *aObj;
  char objname[MAX_STRING_LENGTH];

  set_char_color (AT_IMMORT, ch);

  argument = one_argument (argument, arg);
  if (arg[0] == '\0' || !is_number (arg))
    {
      int wid = 0;
      pager_printf (ch, "%5s %-20s    %5s %-20s\n\r", "Vnum", "Short Desc",
		    "Vnum", "Short Desc");
      for (noauc = first_noauc; noauc; noauc = noauc->next)
	{
	  if ((aObj = get_obj_index (noauc->vnum)) == NULL)
	    continue;
	  strncpy (objname, aObj->short_descr, 20);
	  objname[20] = '\0';
	  pager_printf (ch, "%-5d %-20s    ", aObj->vnum, objname);
	  if (++wid % 2 == 0)
	    pager_printf (ch, "\n\r");
	}
      if (wid % 2 != 0)
	pager_printf (ch, "\n\r");
      return;
    }
  arg1 = atoi (arg);
  if (get_obj_index (arg1) == NULL)
    {
      send_to_char ("That vnum does not exist\n\r", ch);
      return;
    }
  for (noauc = first_noauc; noauc; noauc = noauc->next)
    if (noauc->vnum == arg1)
      {
	UNLINK (noauc, first_noauc, last_noauc, next, prev);
	DISPOSE (noauc);
	save_noauctions ();
	send_to_char ("Vnum no longer prevented from being auctioned:\n\r",
		      ch);
	send_to_char ("Make sure the item is auctionable.\n\r", ch);
	return;
      }
  CREATE (noauc, NOAUCTION_DATA, 1);
  noauc->vnum = arg1;
  sort_noauctions (noauc);
  save_noauctions ();
  send_to_char ("Vnum can no longer be auctioned.\n\r", ch);
  return;
}


RESERVE_DATA *first_reserved;
RESERVE_DATA *last_reserved;
void
save_reserved (void)
{
  RESERVE_DATA *res;
  FILE *fp;

  fclose (fpReserve);
  if (!(fp = fopen (SYSTEM_DIR RESERVED_LIST, "w")))
    {
      bug ("Save_reserved: cannot open " RESERVED_LIST, 0);
      perror (RESERVED_LIST);
      fpReserve = fopen (NULL_FILE, "r");
      return;
    }
  for (res = first_reserved; res; res = res->next)
    fprintf (fp, "%s~\n", res->name);
  fprintf (fp, "$~\n");
  fclose (fp);
  fpReserve = fopen (NULL_FILE, "r");
  return;
}

void
do_reserve (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  RESERVE_DATA *res;

  set_char_color (AT_PLAIN, ch);

  argument = one_argument (argument, arg);
  if (!*arg)
    {
      int wid = 0;

      send_to_char ("-- Reserved Names --\n\r", ch);
      for (res = first_reserved; res; res = res->next)
	{
	  ch_printf (ch, "%c%-17s ", (*res->name == '*' ? '*' : ' '),
		     (*res->name == '*' ? res->name + 1 : res->name));
	  if (++wid % 4 == 0)
	    send_to_char ("\n\r", ch);
	}
      if (wid % 4 != 0)
	send_to_char ("\n\r", ch);
      return;
    }
  for (res = first_reserved; res; res = res->next)
    if (!str_cmp (arg, res->name))
      {
	UNLINK (res, first_reserved, last_reserved, next, prev);
	DISPOSE (res->name);
	DISPOSE (res);
	save_reserved ();
	send_to_char ("Name no longer reserved.\n\r", ch);
	return;
      }
  CREATE (res, RESERVE_DATA, 1);
  res->name = str_dup (arg);
  sort_reserved (res);
  save_reserved ();
  send_to_char ("Name reserved.\n\r", ch);
  return;
}


/*
 * Command to display the weather status of all the areas
 * Last Modified: July 21, 1997
 * Fireblade
 */
void
do_showweather (CHAR_DATA * ch, char *argument)
{
  AREA_DATA *pArea;
  char arg[MAX_INPUT_LENGTH];

  if (!ch)
    {
      bug ("do_showweather: NULL char data");
      return;
    }

  argument = one_argument (argument, arg);

  set_char_color (AT_BLUE, ch);
  ch_printf (ch, "%-40s%-8s %-8s %-8s\n\r",
	     "Area Name:", "Temp:", "Precip:", "Wind:");

  for (pArea = first_area; pArea; pArea = pArea->next)
    {
      if (arg[0] == '\0' || nifty_is_name_prefix (arg, pArea->name))
	{
	  set_char_color (AT_BLUE, ch);
	  ch_printf (ch, "%-40s", pArea->name);
	  set_char_color (AT_WHITE, ch);
	  ch_printf (ch, "%3d", pArea->weather->temp);
	  set_char_color (AT_BLUE, ch);
	  ch_printf (ch, "(");
	  set_char_color (AT_LBLUE, ch);
	  ch_printf (ch, "%3d", pArea->weather->temp_vector);
	  set_char_color (AT_BLUE, ch);
	  ch_printf (ch, ") ");
	  set_char_color (AT_WHITE, ch);
	  ch_printf (ch, "%3d", pArea->weather->precip);
	  set_char_color (AT_BLUE, ch);
	  ch_printf (ch, "(");
	  set_char_color (AT_LBLUE, ch);
	  ch_printf (ch, "%3d", pArea->weather->precip_vector);
	  set_char_color (AT_BLUE, ch);
	  ch_printf (ch, ") ");
	  set_char_color (AT_WHITE, ch);
	  ch_printf (ch, "%3d", pArea->weather->wind);
	  set_char_color (AT_BLUE, ch);
	  ch_printf (ch, "(");
	  set_char_color (AT_LBLUE, ch);
	  ch_printf (ch, "%3d", pArea->weather->wind_vector);
	  set_char_color (AT_BLUE, ch);
	  ch_printf (ch, ")\n\r");
	}
    }

  return;
}

/*
 * Command to control global weather variables and to reset weather
 * Last Modified: July 23, 1997
 * Fireblade
 */
void
do_setweather (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];

  set_char_color (AT_BLUE, ch);

  argument = one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      ch_printf (ch, "%-15s%-6s\n\r", "Parameters:", "Value:");
      ch_printf (ch, "%-15s%-6d\n\r", "random", rand_factor);
      ch_printf (ch, "%-15s%-6d\n\r", "climate", climate_factor);
      ch_printf (ch, "%-15s%-6d\n\r", "neighbor", neigh_factor);
      ch_printf (ch, "%-15s%-6d\n\r", "unit", weath_unit);
      ch_printf (ch, "%-15s%-6d\n\r", "maxvector", max_vector);

      ch_printf (ch, "\n\rResulting values:\n\r");
      ch_printf (ch, "Weather variables range from "
		 "%d to %d.\n\r", -3 * weath_unit, 3 * weath_unit);
      ch_printf (ch, "Weather vectors range from "
		 "%d to %d.\n\r", -1 * max_vector, max_vector);
      ch_printf (ch, "The maximum a vector can "
		 "change in one update is %d.\n\r",
		 rand_factor + 2 * climate_factor +
		 (6 * weath_unit / neigh_factor));
    }
  else if (!str_cmp (arg, "random"))
    {
      if (!is_number (argument))
	{
	  ch_printf (ch, "Set maximum random "
		     "change in vectors to what?\n\r");
	}
      else
	{
	  rand_factor = atoi (argument);
	  ch_printf (ch, "Maximum random "
		     "change in vectors now " "equals %d.\n\r", rand_factor);
	  save_weatherdata ();
	}
    }
  else if (!str_cmp (arg, "climate"))
    {
      if (!is_number (argument))
	{
	  ch_printf (ch, "Set climate effect " "coefficient to what?\n\r");
	}
      else
	{
	  climate_factor = atoi (argument);
	  ch_printf (ch, "Climate effect "
		     "coefficient now equals " "%d.\n\r", climate_factor);
	  save_weatherdata ();
	}
    }
  else if (!str_cmp (arg, "neighbor"))
    {
      if (!is_number (argument))
	{
	  ch_printf (ch, "Set neighbor effect " "divisor to what?\n\r");
	}
      else
	{
	  neigh_factor = atoi (argument);

	  if (neigh_factor <= 0)
	    neigh_factor = 1;

	  ch_printf (ch, "Neighbor effect "
		     "coefficient now equals " "1/%d.\n\r", neigh_factor);
	  save_weatherdata ();
	}
    }
  else if (!str_cmp (arg, "unit"))
    {
      if (!is_number (argument))
	{
	  ch_printf (ch, "Set weather unit " "size to what?\n\r");
	}
      else
	{
	  weath_unit = atoi (argument);
	  ch_printf (ch, "Weather unit size "
		     "now equals %d.\n\r", weath_unit);
	  save_weatherdata ();
	}
    }
  else if (!str_cmp (arg, "maxvector"))
    {
      if (!is_number (argument))
	{
	  ch_printf (ch, "Set maximum vector " "size to what?\n\r");
	}
      else
	{
	  max_vector = atoi (argument);
	  ch_printf (ch, "Maximum vector size "
		     "now equals %d.\n\r", max_vector);
	  save_weatherdata ();
	}
    }
  else if (!str_cmp (arg, "reset"))
    {
      init_area_weather ();
      ch_printf (ch, "Weather system reinitialized.\n\r");
    }
  else if (!str_cmp (arg, "update"))
    {
      int i, number;

      number = atoi (argument);

      if (number < 1)
	number = 1;

      for (i = 0; i < number; i++)
	weather_update ();

      ch_printf (ch, "Weather system updated.\n\r");
    }
  else
    {
      ch_printf (ch, "You may only use one of the " "following fields:\n\r");
      ch_printf (ch, "\trandom\n\r\tclimate\n\r"
		 "\tneighbor\n\r\tunit\n\r\tmaxvector\n\r");
      ch_printf (ch, "You may also reset or update "
		 "the system using the fields 'reset' "
		 "and 'update' respectively.\n\r");
    }

  return;
}


void
do_khistory (CHAR_DATA * ch, char *argument)
{
  MOB_INDEX_DATA *tmob;
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *vch;
  int track;

  if (IS_NPC (ch) || !IS_IMMORTAL (ch))
    {
      ch_printf (ch, "Huh?\n\r");
      return;
    }

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      ch_printf (ch, "syntax: khistory <player>\n\r");
      return;
    }

  vch = get_char_world (ch, arg);

  if (!vch || IS_NPC (vch))
    {
      ch_printf (ch, "They are not here.\n\r");
      return;
    }

  set_char_color (AT_BLOOD, ch);
  ch_printf (ch, "Kill history for %s:\n\r", vch->name);

  for (track = 0; track < MAX_KILLTRACK &&
       vch->pcdata->killed[track].vnum; track++)
    {
      tmob = get_mob_index (vch->pcdata->killed[track].vnum);

      if (!tmob)
	{
	  char buf[MAX_STRING_LENGTH];
	  sprintf (buf, "killhistory: unknown mob vnum - %d",
		   vch->pcdata->killed[track].vnum);
	  bug (buf);
	  continue;
	}

      set_char_color (AT_RED, ch);
      ch_printf (ch, "   %-30s", capitalize (tmob->short_descr));
      set_char_color (AT_BLOOD, ch);
      ch_printf (ch, "(");
      set_char_color (AT_RED, ch);
      ch_printf (ch, "%-5d", tmob->vnum);
      set_char_color (AT_BLOOD, ch);
      ch_printf (ch, ")");
      set_char_color (AT_RED, ch);
      ch_printf (ch, "    - killed %d times.\n\r",
		 vch->pcdata->killed[track].count);
    }

  return;
}

void
do_project (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int pcount;
  int pnum;
  PROJECT_DATA *pproject;

  if (IS_NPC (ch))
    return;

  if (!ch->desc)
    {
      bug ("do_project: no descriptor", 0);
      return;
    }

  switch (ch->substate)
    {
    default:
      break;
    case SUB_WRITING_NOTE:
      if (!ch->pnote)
	{
	  bug ("do_project: log got lost?", 0);
	  send_to_char ("Your log was lost!\n\r", ch);
	  stop_editing (ch);
	  return;
	}
      if (ch->dest_buf != ch->pnote)
	bug ("do_project: sub_writing_note: ch->dest_buf != ch->pnote", 0);
      STRFREE (ch->pnote->text);
      ch->pnote->text = copy_buffer (ch);
      stop_editing (ch);
      return;
    case SUB_PROJ_DESC:
      if (!ch->dest_buf)
	{
	  send_to_char ("Your description was lost!", ch);
	  bug ("do_project: sub_project_desc: NULL ch->dest_buf", 0);
	  ch->substate = SUB_NONE;
	  return;
	}
      pproject = ch->dest_buf;
      STRFREE (pproject->description);
      pproject->description = copy_buffer (ch);
      stop_editing (ch);
      ch->substate = ch->tempnum;
      write_projects ();
      return;
    }

  set_char_color (AT_NOTE, ch);
  argument = one_argument (argument, arg);
  smash_tilde (argument);

  if (!str_cmp (arg, "save"))
    {
      write_projects ();
      ch_printf (ch, "Projects saved.\n\r");
      return;
    }

  if (!str_cmp (arg, "code"))
    {
      pcount = 0;
      pager_printf (ch, " # | Owner       | Project              |\n\r");
      pager_printf (ch,
		    "---|-------------|----------------------|--------------------------|-----------\n\r");
      for (pproject = first_project; pproject; pproject = pproject->next)
	{
	  pcount++;
	  if ((pproject->status && str_cmp (pproject->status, "approved"))
	      || pproject->coder != NULL)
	    continue;
	  pager_printf (ch, "%2d | %-11s | %-20s |\n\r",
			pcount,
			pproject->owner ? pproject->owner : "(None)",
			pproject->name);
	}
      return;
    }
  if (!str_cmp (arg, "more") || !str_cmp (arg, "mine"))
    {
      NOTE_DATA *log;
      bool MINE = FALSE;
      int num_logs = 0;
      pcount = 0;

      if (!str_cmp (arg, "mine"))
	MINE = TRUE;

      pager_printf (ch, "\n\r");
      pager_printf (ch,
		    " # | Owner       | Project              | Coder         | Status     | # of Logs\n\r");
      pager_printf (ch,
		    "---|-------------|----------------------|---------------|------------|----------\n\r");
      for (pproject = first_project; pproject; pproject = pproject->next)
	{
	  pcount++;
	  if (MINE
	      && (!pproject->owner || str_cmp (ch->name, pproject->owner))
	      && (!pproject->coder || str_cmp (ch->name, pproject->coder)))
	    continue;
	  else if (!MINE && pproject->status &&
		   !str_cmp ("Done", pproject->status))
	    continue;
	  num_logs = 0;
	  for (log = pproject->first_log; log; log = log->next)
	    num_logs++;
	  pager_printf (ch, "%2d | %-11s | %-20s | %-13s | %-10s | %3d\n\r",
			pcount,
			pproject->owner ? pproject->owner : "(None)",
			pproject->name,
			pproject->coder ? pproject->coder : "(None)",
			pproject->status ? pproject->status : "(None)",
			num_logs);
	}
      return;
    }
  if (arg[0] == '\0' || !str_cmp (arg, "list"))
    {
      bool aflag, projects_available;
      aflag = FALSE;
      projects_available = FALSE;
      if (!str_cmp (argument, "available"))
	aflag = TRUE;

      pager_printf (ch, "\n\r");
      if (!aflag)
	{
	  pager_printf (ch,
			" # | Owner       | Project              | Date                     | Status\n\r");
	  pager_printf (ch,
			"---|-------------|----------------------|--------------------------|-----------\n\r");
	}
      else
	{
	  pager_printf (ch, " # | Project              | Date\n\r");
	  pager_printf (ch,
			"---|----------------------|--------------------------\n\r");
	}
      pcount = 0;
      for (pproject = first_project; pproject; pproject = pproject->next)
	{
	  pcount++;
	  if (pproject->status && !str_cmp ("Done", pproject->status))
	    continue;
	  if (!aflag)
	    pager_printf (ch, "%2d | %-11s | %-20s | %-24s | %-10s\n\r",
			  pcount,
			  pproject->owner ? pproject->owner : "(None)",
			  pproject->name,
			  pproject->date,
			  pproject->status ? pproject->status : "(None)");
	  else if (!pproject->taken)
	    {
	      if (!projects_available)
		projects_available = TRUE;
	      pager_printf (ch, "%2d | %-20s | %s\n\r",
			    pcount, pproject->name, pproject->date);
	    }
	}
      if (pcount == 0)
	pager_printf (ch, "No projects exist.\n\r");
      else if (aflag && !projects_available)
	pager_printf (ch, "No projects available.\n\r");
      return;
    }

  if (!str_cmp (arg, "imm"))
    {
      NOTE_DATA *log;
      int num_logs = 0;
      pcount = 0;

      argument = one_argument (argument, arg1);

      if (arg1[0] == '\0')
	{
	  ch_printf (ch, "Whose projects do you wish to see?\n\r");
	  return;
	}

      arg1[0] = UPPER (arg1[0]);


      if (str_cmp (ch->name, "Kali")
	  && str_cmp (ch->name, "Moonbeam")
	  && str_cmp (ch->pcdata->council_name, "Code Council") &&
	  get_trust (ch) < LEVEL_ASCENDANT && str_cmp (arg1, "none")
	  && str_cmp (arg1, "(none)"))
	{

	  ch_printf (ch,
		     "You are not high enough level to view %s's projects.\n\r",
		     arg1);
	  return;
	}

      pager_printf (ch, "\n\r");
      pager_printf (ch,
		    " # | Owner       | Project              | Coder         | Status     | # of Logs\n\r");
      pager_printf (ch,
		    "---|-------------|----------------------|---------------|------------|----------\n\r");
      for (pproject = first_project; pproject; pproject = pproject->next)
	{
	  pcount++;
	  if (str_cmp (arg1, pproject->owner))
	    continue;
	  num_logs = 0;
	  for (log = pproject->first_log; log; log = log->next)
	    num_logs++;
	  pager_printf (ch, "%2d | %-11s | %-20s | %-13s | %-10s | %3d\n\r",
			pcount,
			pproject->owner,
			pproject->name,
			pproject->coder ? pproject->coder : "(None)",
			pproject->status ? pproject->status : "(None)",
			num_logs);
	}
      return;
    }

  if (!str_cmp (arg, "add"))
    {
      char *strtime;
      PROJECT_DATA *new_project;	/* Just to be safe */

      if (get_trust (ch) < LEVEL_GOD &&
	  str_cmp (ch->name, "Kali") && str_cmp (ch->name, "Moonbeam")
	  && str_cmp (ch->pcdata->council_name, "Code Council"))
	{
	  send_to_char
	    ("You are not powerfull enough to add a new project.\n\r", ch);
	  return;
	}

      CREATE (new_project, PROJECT_DATA, 1);
      LINK (new_project, first_project, last_project, next, prev);
      new_project->name = str_dup (argument);
      new_project->coder = NULL;
      new_project->owner = STRALLOC ("None");
      new_project->taken = FALSE;
      new_project->description = STRALLOC ("");
      strtime = ctime (&current_time);
      strtime[strlen (strtime) - 1] = '\0';
      new_project->date = STRALLOC (strtime);
      write_projects ();
      ch_printf (ch, "Ok.\n\r");
      return;
    }

  if (!is_number (arg))
    {
      ch_printf (ch, "Invalid project.\n\r");
      return;
    }

  pnum = atoi (arg);
  pproject = get_project_by_number (pnum);
  if (!pproject)
    {
      ch_printf (ch, "No such project.\n\r");
      return;
    }

  argument = one_argument (argument, arg1);

  if (!str_cmp (arg1, "description"))
    {
      if (get_trust (ch) < LEVEL_GOD &&
	  str_cmp (ch->name, "Kali")
	  && str_cmp (ch->name, "Moonbeam")
	  && str_cmp (ch->pcdata->council_name, "Code Council"))
	CHECK_SUBRESTRICTED (ch);
      ch->tempnum = SUB_NONE;
      ch->substate = SUB_PROJ_DESC;
      ch->dest_buf = pproject;
      if (pproject->description == NULL)
	pproject->description = STRALLOC ("");
      start_editing (ch, pproject->description);
      return;
    }
  if (!str_cmp (arg1, "delete"))
    {
      NOTE_DATA *log, *tlog;
      if (str_cmp (ch->name, "Kali")
	  && str_cmp (ch->name, "Moonbeam")
	  && str_cmp (ch->pcdata->council_name, "Code Council") &&
	  get_trust (ch) < LEVEL_ASCENDANT)
	{
	  send_to_char
	    ("You are not high enough level to delete a project.\n\r", ch);
	  return;
	}

      log = pproject->last_log;
      while (log)
	{
	  UNLINK (log, pproject->first_log, pproject->last_log, next, prev);
	  tlog = log->prev;
	  free_note (log);
	  log = tlog;
	}
      UNLINK (pproject, first_project, last_project, next, prev);

      DISPOSE (pproject->name);
      if (pproject->coder)
	DISPOSE (pproject->coder);
      if (pproject->owner)
	STRFREE (pproject->owner);
      if (pproject->description)
	STRFREE (pproject->description);
      if (pproject->date)
	STRFREE (pproject->date);
      if (pproject->status)
	STRFREE (pproject->status);

      DISPOSE (pproject);
      write_projects ();
      ch_printf (ch, "Ok.\n\r");
      return;
    }

  if (!str_cmp (arg1, "assign"))
    {
      CHAR_DATA *vch;
      COUNCIL_DATA *cou = get_council ("Visionary Consortium");

      if ((cou && str_cmp (ch->name, cou->head))
	  || get_trust (ch) < LEVEL_GREATER)
	{
	  send_to_char
	    ("Only the Head of the Visionary Consortium may assign projects.\n\r",
	     ch);
	  return;
	}

      if (!argument || argument[0] == '\0')
	{
	  send_to_char ("Who do you wish to assign it to?\n\r", ch);
	  return;
	}

      if (!str_cmp ("none", argument))
	{
	  STRFREE (pproject->owner);
	  pproject->taken = FALSE;
	  pproject->owner = NULL;
	  send_to_char ("Cleared.\n\r", ch);
	  write_projects ();
	  return;
	}

      if ((vch = get_char_world (ch, argument)) == NULL)
	{
	  send_to_char ("They're not logged on!\n\r", ch);
	  return;
	}

      if (pproject->owner)
	STRFREE (pproject->owner);
      pproject->owner = STRALLOC (vch->name);
      pproject->taken = TRUE;
      write_projects ();
      send_to_char ("Ok.\n\r", ch);
      ch_printf (vch, "You have been assigned project %d.\n\r", pnum);

      return;
    }


/*   if( !str_cmp( arg1, "take" ) )
   {
      if( pproject->taken  && pproject->owner &&
		!str_cmp(pproject->owner, ch->name ) )
      {
	pproject->taken = FALSE;
	STRFREE( pproject->owner );
	pproject->owner = NULL;
	send_to_char("You removed yourself as the owner.\n\r", ch );
        write_projects();
	return;
      }
      else if ( pproject->taken )
      {
      	ch_printf( ch, "This project is already taken.\n\r" );
	return;
      }

      if( pproject->owner )
	STRFREE( pproject->owner );
      pproject->owner = STRALLOC( ch->name );
      pproject->taken = TRUE;
      write_projects();
      ch_printf( ch, "Ok.\n\r" );
      return;
   }
*/
  if (!str_cmp (arg1, "coder"))
    {
      if (pproject->coder && !str_cmp (ch->name, pproject->coder))
	{
	  DISPOSE (pproject->coder);
	  pproject->coder = NULL;
	  send_to_char ("You removed yourself as the coder.\n\r", ch);
	  write_projects ();
	  return;
	}
      else if (pproject->coder)
	{
	  ch_printf (ch, "This project already has a coder.\n\r");
	  return;
	}
      pproject->coder = str_dup (ch->name);
      write_projects ();
      ch_printf (ch, "Ok.\n\r");
      return;
    }
  if (!str_cmp (arg1, "status"))
    {
      if (pproject->owner && str_cmp (pproject->owner, ch->name) &&
	  get_trust (ch) < LEVEL_GREATER
	  && pproject->coder && str_cmp (pproject->coder, ch->name) &&
	  str_cmp (ch->name, "Kali")
	  && str_cmp (ch->name, "Moonbeam")
	  && str_cmp (ch->pcdata->council_name, "Code Council"))
	{
	  ch_printf (ch, "This is not your project!\n\r");
	  return;
	}
      if (pproject->status)
	STRFREE (pproject->status);
      pproject->status = STRALLOC (argument);
      write_projects ();
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (arg1, "show"))
    {
      if (pproject->description)
	send_to_char (pproject->description, ch);
      else
	send_to_char ("That project does not have a description.\n\r", ch);
      return;
    }
  if (!str_cmp (arg1, "log"))
    {
      NOTE_DATA *plog;
      if (!str_cmp (argument, "write"))
	{
	  note_attach (ch);
	  ch->substate = SUB_WRITING_NOTE;
	  ch->dest_buf = ch->pnote;
	  start_editing (ch, ch->pnote->text);
	  return;
	}

      argument = one_argument (argument, arg2);

      if (!str_cmp (arg2, "subject"))
	{
	  note_attach (ch);
	  STRFREE (ch->pnote->subject);
	  ch->pnote->subject = STRALLOC (argument);
	  ch_printf (ch, "Ok.\n\r");
	  return;
	}

      if (!str_cmp (arg2, "post"))
	{
	  char *strtime;

	  if (pproject->owner && str_cmp (ch->name, pproject->owner) &&
	      pproject->coder && str_cmp (ch->name, pproject->coder) &&
	      get_trust (ch) < LEVEL_GREATER && str_cmp (ch->name, "Kali") &&
	      str_cmp (ch->name, "Moonbeam") &&
	      str_cmp (ch->pcdata->council_name, "Code Council"))
	    {
	      ch_printf (ch, "This is not your project!\n\r");
	      return;
	    }

	  if (!ch->pnote)
	    {
	      ch_printf (ch, "You have no log in progress.\n\r");
	      return;
	    }

	  if (!ch->pnote->subject)
	    {
	      ch_printf (ch, "Your log has no subject.\n\r");
	      return;
	    }

	  strtime = ctime (&current_time);
	  strtime[strlen (strtime) - 1] = '\0';
	  ch->pnote->date = STRALLOC (strtime);
	  ch->pnote->sender = ch->name;

	  plog = ch->pnote;
	  ch->pnote = NULL;
	  LINK (plog, pproject->first_log, pproject->last_log, next, prev);
	  write_projects ();
	  ch_printf (ch, "Ok.\n\r");
	  return;
	}

      if (!str_cmp (arg2, "list"))
	{
	  if (pproject->owner && pproject->coder &&
	      str_cmp (ch->name, pproject->owner)
	      && get_trust (ch) < LEVEL_SAVIOR
	      && str_cmp (ch->name, pproject->coder)
	      && str_cmp (ch->name, "Kali") && str_cmp (ch->name, "Moonbeam")
	      && str_cmp (ch->pcdata->council_name, "Code Council"))
	    {
	      ch_printf (ch, "This is not your project!\n\r");
	      return;
	    }

	  pcount = 0;
	  pager_printf (ch, "Project: %-12s: %s\n\r",
			pproject->owner ? pproject->owner : "(None)",
			pproject->name);

	  for (plog = pproject->first_log; plog; plog = plog->next)
	    {
	      pcount++;
	      pager_printf (ch, "%2d) %-12s: %s\n\r",
			    pcount, plog->sender, plog->subject);
	    }
	  if (pcount == 0)
	    ch_printf (ch, "No logs available.\n\r");
	  return;
	}

      if (!is_number (arg2))
	{
	  ch_printf (ch, "Invalid log.\n\r");
	  return;
	}

      pnum = atoi (arg2);

      plog = get_log_by_number (pproject, pnum);
      if (!plog)
	{
	  ch_printf (ch, "Invalid log.\n\r");
	  return;
	}


      if (!str_cmp (argument, "delete"))
	{
	  if (pproject->owner && str_cmp (ch->name, pproject->owner) &&
	      get_trust (ch) < LEVEL_ASCENDANT &&
	      pproject->coder && str_cmp (ch->name, pproject->coder) &&
	      str_cmp (ch->name, "Kali")
	      && str_cmp (ch->name, "Moonbeam")
	      && str_cmp (ch->pcdata->council_name, "Code Council"))
	    {
	      ch_printf (ch, "This is not your project!\n\r");
	      return;
	    }

	  UNLINK (plog, pproject->first_log, pproject->last_log, next, prev);
	  free_note (plog);
	  write_projects ();
	  ch_printf (ch, "Ok.\n\r");
	  return;
	}

      if (!str_cmp (argument, "read"))
	{
	  if (pproject->owner && pproject->coder &&
	      str_cmp (ch->name, pproject->owner)
	      && get_trust (ch) < LEVEL_SAVIOR
	      && str_cmp (ch->name, pproject->coder)
	      && str_cmp (ch->name, "Kali") && str_cmp (ch->name, "Moonbeam")
	      && str_cmp (ch->pcdata->council_name, "Code Council"))
	    {
	      ch_printf (ch, "This is not your project!\n\r");
	      return;
	    }

	  pager_printf (ch, "[%3d] %s: %s\n\r%s\n\r%s",
			pnum,
			plog->sender, plog->subject, plog->date, plog->text);
	  return;
	}
    }
  send_to_char ("Unknown syntax see help 'PROJECT'.\n\r", ch);
  return;
}

PROJECT_DATA *
get_project_by_number (int pnum)
{
  int pcount;
  PROJECT_DATA *pproject;
  pcount = 0;
  for (pproject = first_project; pproject; pproject = pproject->next)
    {
      pcount++;
      if (pcount == pnum)
	return pproject;
    }
  return NULL;
}

NOTE_DATA *
get_log_by_number (PROJECT_DATA * pproject, int pnum)
{
  int pcount;
  NOTE_DATA *plog;
  pcount = 0;
  for (plog = pproject->first_log; plog; plog = plog->next)
    {
      pcount++;
      if (pcount == pnum)
	return plog;
    }
  return NULL;
}

/*
 * Command to check for multiple ip addresses in the mud.
 * --Shaddai
 */

 /*
  * Added this new struct to do matching
  * If ya think of a better way do it, easiest way I could think of at
  * 2 in the morning :) --Shaddai
  */

typedef struct ipcompare_data IPCOMPARE_DATA;
struct ipcompare_data
{
  struct ipcompare_data *prev;
  struct ipcompare_data *next;
  char *host;
  char *name;
  char *user;
  int connected;
  int count;
  int descriptor;
  int idle;
  int port;
  bool printed;
};

void
do_ipcompare (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  DESCRIPTOR_DATA *d;
  char arg[MAX_INPUT_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char *addie = NULL;
  bool prefix = FALSE, suffix = FALSE, inarea = FALSE, inroom =
    FALSE, inworld = FALSE;
  int count = 0, times = -1;
  bool fMatch;
  argument = one_argument (argument, arg);
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  set_pager_color (AT_PLAIN, ch);

  if (IS_NPC (ch))
    {
      send_to_char ("Huh?\n\r", ch);
      return;
    }

  if (arg[0] == '\0')
    {
      send_to_char ("ipcompare pkill\n\r", ch);
      send_to_char ("ipcompare total\n\r", ch);
      send_to_char ("ipcompare <person> [room|area|world] [#]\n\r", ch);
      send_to_char ("ipcompare <site>   [room|area|world] [#]\n\r", ch);
      return;
    }
  if (!str_cmp (arg, "total"))
    {
      IPCOMPARE_DATA *first_ip = NULL, *last_ip = NULL, *hmm, *hmm_next;
      for (d = first_descriptor; d; d = d->next)
	{
	  fMatch = FALSE;
	  for (hmm = first_ip; hmm; hmm = hmm->next)
	    if (!str_cmp (hmm->host, d->host))
	      fMatch = TRUE;
	  if (!fMatch)
	    {
	      IPCOMPARE_DATA *temp;
	      CREATE (temp, IPCOMPARE_DATA, 1);
	      temp->host = str_dup (d->host);
	      LINK (temp, first_ip, last_ip, next, prev);
	      count++;
	    }
	}
      for (hmm = first_ip; hmm; hmm = hmm_next)
	{
	  hmm_next = hmm->next;
	  UNLINK (hmm, first_ip, last_ip, next, prev);
	  if (hmm->host)
	    DISPOSE (hmm->host);
	  DISPOSE (hmm);
	}
      ch_printf (ch, "There were %d unique ip addresses found.\n\r", count);
      return;
    }
  else if (!str_cmp (arg, "pkill"))
    {
      IPCOMPARE_DATA *first_ip = NULL, *last_ip = NULL, *hmm, *hmm_next;
      sprintf (buf, "\n\rDesc|Con|Idle| Port | Player      ");
      if (get_trust (ch) >= LEVEL_SAVIOR)
	strcat (buf, "@HostIP           ");
      if (get_trust (ch) >= LEVEL_GOD)
	strcat (buf, "| Username");
      strcat (buf, "\n\r");
      strcat (buf, "----+---+----+------+-------------");
      if (get_trust (ch) >= LEVEL_SAVIOR)
	strcat (buf, "------------------");
      if (get_trust (ch) >= LEVEL_GOD)
	strcat (buf, "+---------");
      strcat (buf, "\n\r");
      send_to_pager (buf, ch);

      for (d = first_descriptor; d; d = d->next)
	{
	  IPCOMPARE_DATA *temp;

	  if ((d->connected != CON_PLAYING && d->connected != CON_EDITING)
	      || d->character == NULL || !CAN_PKILL (d->character)
	      || !can_see (ch, d->character))
	    continue;
	  CREATE (temp, IPCOMPARE_DATA, 1);
	  temp->host = str_dup (d->host);
	  temp->descriptor = d->descriptor;
	  temp->connected = d->connected;
	  temp->idle = d->idle;
	  temp->port = d->port;
	  temp->name = (d->original ? str_dup (d->original->name) :
			d->character ? str_dup (d->character->name) :
			str_dup ("(none)"));
	  temp->user = str_dup (d->user);
	  temp->count = 0;
	  temp->printed = FALSE;
	  LINK (temp, first_ip, last_ip, next, prev);
	}

      for (d = first_descriptor; d; d = d->next)
	{
	  fMatch = FALSE;
	  if ((d->connected != CON_PLAYING && d->connected != CON_EDITING)
	      || d->character == NULL || !can_see (ch, d->character))
	    continue;
	  for (hmm = first_ip; hmm; hmm = hmm->next)
	    {
	      if (!str_cmp (hmm->host, d->host) &&
		  str_cmp (hmm->name, (d->original ? d->original->name :
				       d->character ? d->
				       character->name : "(none)")))
		{
		  fMatch = TRUE;
		  break;
		}
	    }
	  if (fMatch && hmm)
	    {
	      hmm->count++;
	      if (!hmm->printed && hmm->count > 0)
		{
		  sprintf (buf,
			   " %3d| %2d|%4d|%6d| %-12s",
			   hmm->descriptor,
			   hmm->connected,
			   hmm->idle / 4, hmm->port, hmm->name);
		  if (get_trust (ch) >= LEVEL_SAVIOR)
		    sprintf (buf + strlen (buf), "@%-16s ", hmm->host);
		  if (get_trust (ch) >= LEVEL_GOD)
		    sprintf (buf + strlen (buf), "| %s", hmm->user);
		  strcat (buf, "\n\r");
		  send_to_pager (buf, ch);
		  hmm->printed = TRUE;
		}
	      sprintf (buf,
		       " %3d| %2d|%4d|%6d| %-12s",
		       d->descriptor,
		       d->connected,
		       d->idle / 4,
		       d->port,
		       d->original ? d->original->name :
		       d->character ? d->character->name : "(none)");
	      if (get_trust (ch) >= LEVEL_SAVIOR)
		sprintf (buf + strlen (buf), "@%-16s ", d->host);
	      if (get_trust (ch) >= LEVEL_GOD)
		sprintf (buf + strlen (buf), "| %s", d->user);
	      strcat (buf, "\n\r");
	      send_to_pager (buf, ch);
	    }
	}
      for (hmm = first_ip; hmm; hmm = hmm_next)
	{
	  hmm_next = hmm->next;
	  UNLINK (hmm, first_ip, last_ip, next, prev);
	  if (hmm->name)
	    DISPOSE (hmm->name);
	  if (hmm->host)
	    DISPOSE (hmm->host);
	  if (hmm->user)
	    DISPOSE (hmm->user);
	  DISPOSE (hmm);
	}
      return;
    }
  if (arg1[0] != '\0')
    {
      if (is_number (arg1))
	times = atoi (arg1);
      else
	{
	  if (!str_cmp (arg1, "room"))
	    inroom = TRUE;
	  else if (!str_cmp (arg1, "area"))
	    inarea = TRUE;
	  else
	    inworld = TRUE;
	}
      if (arg2[0] != '\0')
	{
	  if (is_number (arg2))
	    times = atoi (arg2);
	  else
	    {
	      send_to_char ("Please see help ipcompare for more info.\n\r",
			    ch);
	      return;
	    }
	}
    }
  if ((victim = get_char_world (ch, arg)) != NULL && victim->desc)
    {
      if (IS_NPC (victim))
	{
	  send_to_char ("Not on NPC's.\n\r", ch);
	  return;
	}
      addie = victim->desc->host;
    }
  else
    {
      addie = arg;
      if (arg[0] == '*')
	{
	  prefix = TRUE;
	  addie++;
	}
      if (addie[strlen (addie) - 1] == '*')
	{
	  suffix = TRUE;
	  addie[strlen (addie) - 1] = '\0';
	}
    }
  sprintf (buf, "\n\rDesc|Con|Idle| Port | Player      ");
  if (get_trust (ch) >= LEVEL_SAVIOR)
    strcat (buf, "@HostIP           ");
  if (get_trust (ch) >= LEVEL_GOD)
    strcat (buf, "| Username");
  strcat (buf, "\n\r");
  strcat (buf, "----+---+----+------+-------------");
  if (get_trust (ch) >= LEVEL_SAVIOR)
    strcat (buf, "------------------");
  if (get_trust (ch) >= LEVEL_GOD)
    strcat (buf, "+---------");
  strcat (buf, "\n\r");
  send_to_pager (buf, ch);
  for (d = first_descriptor; d; d = d->next)
    {
      if (!d->character || (d->connected != CON_PLAYING &&
			    d->connected != CON_EDITING)
	  || !can_see (ch, d->character))
	continue;
      if (inroom && ch->in_room != d->character->in_room)
	continue;
      if (inarea && ch->in_room->area != d->character->in_room->area)
	continue;
      if (times > 0 && count == (times - 1))
	break;
      if (prefix && suffix && strstr (addie, d->host))
	fMatch = TRUE;
      else if (prefix && !str_suffix (addie, d->host))
	fMatch = TRUE;
      else if (suffix && !str_prefix (addie, d->host))
	fMatch = TRUE;
      else if (!str_cmp (d->host, addie))
	fMatch = TRUE;
      else
	fMatch = FALSE;
      if (fMatch)
	{
	  count++;
	  sprintf (buf,
		   " %3d| %2d|%4d|%6d| %-12s",
		   d->descriptor,
		   d->connected,
		   d->idle / 4,
		   d->port,
		   d->original ? d->original->name :
		   d->character ? d->character->name : "(none)");
	  if (get_trust (ch) >= LEVEL_SAVIOR)
	    sprintf (buf + strlen (buf), "@%-16s ", d->host);
	  if (get_trust (ch) >= LEVEL_GOD)
	    sprintf (buf + strlen (buf), "| %s", d->user);
	  strcat (buf, "\n\r");
	  send_to_pager (buf, ch);
	}
    }
  pager_printf (ch, "%d user%s.\n\r", count, count == 1 ? "" : "s");
  return;
}


/*
 * New nuisance flag to annoy people that deserve it :) --Shaddai
 */
void
do_nuisance (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  struct tm *now_time;
  int time = 0, max_time = 0, power = 1;
  bool minute = FALSE, day = FALSE, hour = FALSE;

  if (IS_NPC (ch))
    {
      send_to_char ("Huh?\n\r", ch);
      return;
    }

  argument = one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Syntax: nuisance <victim> [Options]\n\r", ch);
      send_to_char ("Options:\n\r", ch);
      send_to_char ("  power <level 1-10>\n\r", ch);
      send_to_char ("  time  <days>\n\r", ch);
      send_to_char ("  maxtime <#> <minutes/hours/days>\n\r", ch);
      send_to_char
	("Defaults: Time -- forever, power -- 1, maxtime 8 days.\n\r", ch);
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("There is no one on with that name.\n\r", ch);
      return;
    }

  if (IS_NPC (victim))
    {
      send_to_char ("You can't set a nuisance flag on a mob.\n\r", ch);
      return;
    }

  if (get_trust (ch) <= get_trust (victim))
    {
      send_to_char ("I don't think they would like that.\n\r", ch);
      return;
    }

  if (victim->pcdata->nuisance)
    {
      send_to_char ("That flag has already been set.\n\r", ch);
      return;
    }

  argument = one_argument (argument, arg1);

  while (argument[0] != '\0')
    {
      if (!str_cmp (arg1, "power"))
	{
	  argument = one_argument (argument, arg1);
	  if (arg1[0] == '\0' || !is_number (arg1))
	    {
	      send_to_char ("Power option syntax: power <number>\n\r", ch);
	      return;
	    }
	  if ((power = atoi (arg1)) < 1 || power > 10)
	    {
	      send_to_char ("Power must be 1 - 10.\n\r", ch);
	      return;
	    }
	}
      else if (!str_cmp (arg1, "time"))
	{
	  argument = one_argument (argument, arg1);
	  if (arg1[0] == '\0' || !is_number (arg1))
	    {
	      send_to_char ("Time option syntax: time <number> (In days)\n\r",
			    ch);
	      return;
	    }
	  if ((time = atoi (arg1)) < 1)
	    {
	      send_to_char ("Time must be a positive number.\n\r", ch);
	      return;
	    }
	}
      else if (!str_cmp (arg1, "maxtime"))
	{
	  argument = one_argument (argument, arg1);
	  argument = one_argument (argument, arg2);
	  if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number (arg1))
	    {
	      send_to_char
		("Maxtime option syntax: maxtime <number> <minute|day|hour>\n\r",
		 ch);
	      return;
	    }
	  if ((max_time = atoi (arg1)) < 1)
	    {
	      send_to_char ("Maxtime must be a positive number.\n\r", ch);
	      return;
	    }
	  if (!str_cmp (arg2, "minutes"))
	    minute = TRUE;
	  else if (!str_cmp (arg2, "hours"))
	    hour = TRUE;
	  else if (!str_cmp (arg2, "days"))
	    day = TRUE;
	}
      else
	{
	  ch_printf (ch, "Unknown option %s.\n\r", arg1);
	  return;
	}
      argument = one_argument (argument, arg1);
    }

  if (minute && (max_time < 1 || max_time > 59))
    {
      send_to_char ("Minutes must be 1 to 59.\n\r", ch);
      return;
    }
  else if (hour && (max_time < 1 || max_time > 23))
    {
      send_to_char ("Hours must be 1 - 23.\n\r", ch);
      return;
    }
  else if (day && (max_time < 1 || max_time > 999))
    {
      send_to_char ("Days must be 1 - 999.\n\r", ch);
      return;
    }
  else if (!max_time)
    {
      day = TRUE;
      max_time = 7;
    }
  CREATE (victim->pcdata->nuisance, NUISANCE_DATA, 1);
  victim->pcdata->nuisance->time = current_time;
  victim->pcdata->nuisance->flags = 1;
  victim->pcdata->nuisance->power = power;
  now_time = localtime (&current_time);

  if (minute)
    now_time->tm_min += max_time;
  else if (hour)
    now_time->tm_hour += max_time;
  else
    now_time->tm_mday += max_time;

  victim->pcdata->nuisance->max_time = mktime (now_time);
  if (time)
    {
      add_timer (victim, TIMER_NUISANCE, (28800 * time), NULL, 0);
      ch_printf (ch, "Nuisance flag set for %d days.\n\r", time);
    }
  else
    send_to_char ("Nuisance flag set forever\n\r", ch);
  return;
}

void
do_unnuisance (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  TIMER *timer, *timer_next;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC (ch))
    {
      send_to_char ("Huh?\n\r", ch);
      return;
    }
  one_argument (argument, arg);

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("There is no one on with that name.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("You can't remove a nuisance flag from a mob.\n\r", ch);
      return;
    }
  if (get_trust (ch) <= get_trust (victim))
    {
      send_to_char ("You can't do that.\n\r", ch);
      return;
    }
  if (!victim->pcdata->nuisance)
    {
      send_to_char ("They do not have that flag set.\n\r", ch);
      return;
    }
  for (timer = victim->first_timer; timer; timer = timer_next)
    {
      timer_next = timer->next;
      if (timer->type == TIMER_NUISANCE)
	extract_timer (victim, timer);
    }
  DISPOSE (victim->pcdata->nuisance);
  send_to_char ("Nuisance flag removed.\n\r", ch);
  return;
}

void
do_pcrename (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char newname[MAX_STRING_LENGTH];
  char oldname[MAX_STRING_LENGTH];
  char backname[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];

  argument = one_argument (argument, arg1);
  one_argument (argument, arg2);
  smash_tilde (arg2);


  if (IS_NPC (ch))
    return;

  if (arg1[0] == '\0' || arg2[0] == '\0')
    {
      send_to_char ("Syntax: rename <victim> <new name>\n\r", ch);
      return;
    }

  if (!check_parse_name (arg2, 1))
    {
      send_to_char ("Illegal name.\n\r", ch);
      return;
    }
  /* Just a security precaution so you don't rename someone you don't mean 
   * too --Shaddai
   */
  if ((victim = get_char_room (ch, arg1)) == NULL)
    {
      send_to_char ("That person is not in the room.\n\r", ch);
      return;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("You can't rename NPC's.\n\r", ch);
      return;
    }

  if (get_trust (ch) < get_trust (victim))
    {
      send_to_char ("I don't think they would like that!\n\r", ch);
      return;
    }
  sprintf (newname, "%s%c/%s", PLAYER_DIR, tolower (arg2[0]),
	   capitalize (arg2));
  sprintf (oldname, "%s%c/%s", PLAYER_DIR,
	   tolower (victim->pcdata->filename[0]),
	   capitalize (victim->pcdata->filename));
  sprintf (backname, "%s%c/%s", BACKUP_DIR,
	   tolower (victim->pcdata->filename[0]),
	   capitalize (victim->pcdata->filename));
  if (access (newname, F_OK) == 0)
    {
      send_to_char ("That name already exists.\n\r", ch);
      return;
    }

  /* Have to remove the old god entry in the directories */
  if (IS_IMMORTAL (victim))
    {
      char godname[MAX_STRING_LENGTH];
      sprintf (godname, "%s%s", GOD_DIR,
	       capitalize (victim->pcdata->filename));
      remove (godname);
    }

  /* Remember to change the names of the areas */
  if (ch->pcdata->area)
    {
      char filename[MAX_STRING_LENGTH];
      char newfilename[MAX_STRING_LENGTH];

      sprintf (filename, "%s%s.are", BUILD_DIR, victim->name);
      sprintf (newfilename, "%s%s.are", BUILD_DIR, capitalize (arg2));
      rename (filename, newfilename);
      sprintf (filename, "%s%s.are.bak", BUILD_DIR, victim->name);
      sprintf (newfilename, "%s%s.are.bak", BUILD_DIR, capitalize (arg2));
      rename (filename, newfilename);
    }

  STRFREE (victim->name);
  victim->name = STRALLOC (capitalize (arg2));
  STRFREE (victim->pcdata->filename);
  victim->pcdata->filename = STRALLOC (capitalize (arg2));
  remove (backname);
  if (remove (oldname))
    {
      sprintf (buf, "Error: Couldn't delete file %s in do_rename.", oldname);
      send_to_char ("Couldn't delete the old file!\n\r", ch);
      log_string (oldname);
    }
  /* Time to save to force the affects to take place */
  save_char_obj (victim);

  /* Now lets update the wizlist */
  if (IS_IMMORTAL (victim))
    make_wizlist ();
  send_to_char ("Character was renamed.\n\r", ch);
  return;
}


/*
 * oowner will make an item owned by a player so only that player can use it.
 * Shaddai
 */
void
do_oowner (CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *obj;
  CHAR_DATA *victim = NULL;
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];

  if (IS_NPC (ch))
    {
      send_to_char ("Huh?\n\r", ch);
      return;
    }

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (arg1[0] == '\0' || arg2[0] == '\0')
    {
      send_to_char ("Syntax: oowner <object> <player|none>\n\r", ch);
      return;
    }

  if (str_cmp (arg2, "none") && (victim = get_char_room (ch, arg2)) == NULL)
    {
      send_to_char ("No such player is in the room.\n\r", ch);
      return;
    }

  if ((obj = get_obj_here (ch, arg1)) == NULL)
    {
      send_to_char ("No such object exists.\n\r", ch);
      return;
    }

  separate_obj (obj);

  if (!str_cmp ("none", arg2))
    {
      STRFREE (obj->owner);
      obj->owner = STRALLOC ("");
      send_to_char ("Done.\n\r", ch);
      return;
    }

  if (IS_NPC (victim))
    {
      send_to_char ("A mob can't be an owner of an item.\n\r", ch);
      return;
    }
  STRFREE (obj->owner);
  obj->owner = STRALLOC (victim->name);
  send_to_char ("Done.\n\r", ch);
  return;
}

/*
 * Appear before a mortal (or mortals)... only visible to them	-Thoric
 */
void
do_appear (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *vch;

  if (IS_NPC (ch))
    {
      send_to_char ("Huh?\n\r", ch);
      return;
    }

  if (argument[0] == '\0')
    {
      ch_printf (ch, "You are appearing before: %s\n\r",
		 ch->pcdata->see_me[0] !=
		 '\0' ? ch->pcdata->see_me : "(nobody)");
      return;
    }
  if (is_name (argument, "clear none nobody"))
    {
      if (ch->pcdata->see_me)
	DISPOSE (ch->pcdata->see_me);
      ch->pcdata->see_me = str_dup ("");
      for (vch = first_char; vch; vch = vch->next)
	{
	  if (vch->reply == ch)
	    vch->reply = NULL;
	  if (vch->retell == ch)
	    vch->retell = NULL;
	}
      send_to_char ("Cleared.\n\r", ch);
      return;
    }

  // I hate having to redo my entire appear list all the time *grin*
  // -- Alty
  while (isspace (*argument))
    ++argument;
  if (*argument == '+')
    {
      ++argument;
      while (isspace (*argument))
	++argument;
      strcpy (buf, ch->pcdata->see_me);
    }
  else
    buf[0] = '\0';

  while (argument[0] != '\0')
    {
      argument = one_argument (argument, arg);

      if (get_clan (arg) || get_council (arg) || !strcmp (arg, "room"))
	{
	  if (buf[0] != '\0')
	    strcat (buf, " ");
	  strcat (buf, arg);
	  continue;
	}

      if ((vch = get_char_world (ch, arg)) != NULL && !IS_NPC (vch))
	{
	  act (AT_IMMORT, "$n appears before you!", ch, NULL, vch, TO_VICT);
	  act (AT_IMMORT, "You appear before $N.", ch, NULL, vch, TO_CHAR);
	  if (buf[0] != '\0')
	    strcat (buf, " ");
	  strcat (buf, vch->name);
	}
      else
	{
	  ch_printf (ch, "%c%s isn't here.\n\r", UPPER (arg[0]), arg + 1);
	}
    }
  if (ch->pcdata->see_me)
    DISPOSE (ch->pcdata->see_me);
  ch->pcdata->see_me = str_dup (buf);
  send_to_char ("Done.\n\r", ch);
}

/*
 * Disappear
 */
void
do_disappear (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char *p;
  CHAR_DATA *vch;

  if (IS_NPC (ch))
    {
      send_to_char ("Huh?\n\r", ch);
      return;
    }

  p = ch->pcdata->see_me;
  while (p[0] != '\0')
    {
      p = one_argument (p, arg);
      if ((vch = get_char_room (ch, arg)) != NULL)
	act (AT_IMMORT, "$n fades from your view...", ch, NULL, vch, TO_VICT);
    }
  if (ch->pcdata->see_me)
    DISPOSE (ch->pcdata->see_me);
  ch->pcdata->see_me = str_dup ("");
  send_to_char ("You disappear...\n\r", ch);
}


/* Send login messages to characters - big modification to the original */
/* login message stuff from the housing module - Edmond - June 02       */

void
do_message (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char name[MAX_INPUT_LENGTH];
  char checkname[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char *arg;
  sh_int type = 0;
  LMSG_DATA *lmsg;

  if (!argument || argument[0] == '\0')
    {
      send_to_char ("Leave a login message for who?\n\r", ch);
      return;
    }

  argument = one_argument (argument, name);
  arg = one_argument (argument, arg1);
  arg = one_argument (arg, arg2);

  if (!str_cmp (name, "list") && get_trust (ch) >= LEVEL_GREATER)
    {
      for (lmsg = first_lmsg; lmsg; lmsg = lmsg->next)
	{
	  sprintf (buf, "&CName: &c%-20s &CType: &c%d\n\r",
		   capitalize (lmsg->name), lmsg->type);
	  send_to_char_color (buf, ch);
	  if (lmsg->text)
	    {
	      sprintf (buf, "&CText:\n\r  &c%s\n\r", lmsg->text);
	      send_to_char_color (buf, ch);
	    }
	  send_to_char ("\n\r", ch);
	}
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  sprintf (checkname, "%s%c/%s", PLAYER_DIR, tolower (name[0]),
	   capitalize (name));

  if (access (checkname, F_OK) == 0)
    {
      CHAR_DATA *temp;

      for (temp = first_char; temp; temp = temp->next)
	{
	  if (IS_NPC (temp))
	    continue;
	  if (!str_cmp (name, temp->name) && temp->desc)
	    {
	      send_to_char
		("They are online, wouldn't tells be just as easy?\n\r", ch);
	      return;
	    }
	}

      if (!str_cmp (arg1, "type"))
	{
	  if (is_number (arg2))
	    {
	      type = atoi (arg2);
	      if (type > MAX_MSG)
		{
		  send_to_char ("Invalid login message.\n\r", ch);
		  return;
		}
	      argument = NULL;
	    }
	  else
	    {
	      send_to_char ("Which type?\n\r", ch);
	      return;
	    }
	}

      if (!type && (!argument || argument[0] == '\0'))
	{
	  send_to_char ("Send them what message?\n\r", ch);
	  return;
	}

      add_loginmsg (name, type, argument);
      sprintf (buf, "You have sent %s the following message:\n\r",
	       capitalize (name));
      send_to_char (buf, ch);
      if (type == 0)
	send_to_char_color (argument, ch);
      else
	send_to_char_color (login_msg[type], ch);
      send_to_char ("\n\r", ch);
    }
  else
    {
      send_to_char ("That name does not exist.\n\r", ch);
      return;
    }

}
