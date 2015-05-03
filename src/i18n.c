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
     |                      -*- Gettext Support -*-                        |
     |_____________________________________________________________________|
     |                                                                     |
     |            Read "doc/gettext.txt" & "doc/i18n.txt" files.           |
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

#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "mud.h"

void fread_locale args((LOCALE_DATA * locale, FILE * fp));
bool load_locales_file args((char * localesfile));

/* Get server locale from env LANG */
int
i18n_getlocale (void)
{
	SMAUGlocale = getenv("LANG");
}

/* Set server locale */
int
i18n_setlocale (void)
{
  /* Initialize Gettext support */
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

	i18n_getlocale( );

	if (!SMAUGlocale) {
		  sprintf(log_buf, "Environment variable <LANG> is not set!\n");
		  log_string ( log_buf );
	} else {
		  sprintf(log_buf, "Environment variable <LANG> is set to: %s\n", SMAUGlocale);
		  log_string ( log_buf );
	}
}

/*
 * Load "LOCALES_FILE=SYSTEM_DIR/locales.dat" file
 */

bool
load_locales_file (char *localesfile)
{
  char filename[256];
  LOCALE_DATA *locale;
  FILE *fp;
  bool found;

	sprintf(localesfile, "%s", LOCALES_FILE);
  sprintf (filename, "%s", localesfile);

  found = FALSE;

  if ((fp = fopen (filename, "r")) != NULL)
    {

      found = TRUE;
      for (;;)
	{
	  char letter;
	  char *word;

	  letter = fread_letter (fp);
	  if (letter == '*')
	    {
	      fread_to_eol (fp);
	      continue;
	    }

	  if (letter != '#')
	    {
	      bug ("Load_locales_file: # not found.", 0);
	      break;
	    }

	  word = fread_word (fp);
	  if (!str_cmp (word, "LOCALE"))
	    {
	      fread_locale (locale, fp);
	      break;
	    }
	  else if (!str_cmp (word, "END"))
	    break;
	  else
	    {
	      char buf[MAX_STRING_LENGTH];

	      sprintf (buf, "Load_locale_file: bad section: %s.", word);
	      bug (buf, 0);
	      break;
	    }
	}
      fclose (fp);
    }

  if (found)
	  return found;
}

void
fread_locale (LOCALE_DATA *locale, FILE * fp)
{
  char buf[MAX_STRING_LENGTH];
  char *word;
  bool fMatch;

  locale->mem_limit = 0;		/* Set up defaults */
  for (;;)
    {
      word = feof (fp) ? "End" : fread_word (fp);
      fMatch = FALSE;

      switch (UPPER (word[0]))
	{
	case '*':
	  fMatch = TRUE;
	  fread_to_eol (fp);
	  break;

	case 'E':
	  if (!str_cmp (word, "End"))
	    {
	      if (!locale->name)
		locale->name = STRALLOC ("");
	      if (!locale->lang)
		locale->lang = STRALLOC ("");
	      return;
	    }
	  break;

	case 'N':
	  KEY ("Name", locale->name, fread_string (fp));
	  break;

	case 'L':
	  KEY ("Locale", locale->lang, fread_string (fp));
	  break;
	}

      if (!fMatch)
	{
	  sprintf (buf, "Fread_locale: no match: %s", word);
	  bug (buf, 0);
	}
    }
}


