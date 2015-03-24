/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops, Fireblade, Edmond, Conran                         |             *
 *------------------------------------------------------------------------  *
 *                      Shaddai's Polymorph                                 *
 ****************************************************************************/

#include <string.h>
#include <ctype.h>
#include <stdio.h>
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

MORPH_DATA *morph_start = NULL;
MORPH_DATA *morph_end = NULL;
int        morph_vnum = 0;

/*
 * Local functions
 */
void copy_morph args ( ( MORPH_DATA *morph, MORPH_DATA *temp ) );

/*
 * Extern prototypes
 */
int     get_risflag      args( ( char *flag ) );
int     get_npc_race     args( ( char *type ) );
int     get_pc_race     args( ( char *type ) );

/*
 *  Command used to set all the morphing information.
 *  Must use the morphset save command, to write the commands to file.
 *  Blood/Hp/Mana/Move/Hitroll/Damroll can be set using variables such
 *  as 1d2+10.  No boundry checks are in place yet on those, so care must
 *  be taken when using these.  --Shaddai
 */

void
do_morphset ( CHAR_DATA *ch, char *argument )
{
  char 		arg1[MAX_INPUT_LENGTH];
  char 		arg2[MAX_INPUT_LENGTH];
  char 		arg3[MAX_INPUT_LENGTH];
  char		buf[MAX_STRING_LENGTH];
  char          *origarg = argument;
  int		value;
  MORPH_DATA   *morph = NULL;

  set_char_color (AT_PLAIN, ch);

  if (IS_NPC (ch))
  {
      send_to_char ("Mob's can't morphset\n\r", ch);
      return;
  }

  if (!ch->desc)
  {
      send_to_char ("You have no descriptor\n\r", ch);
      return;
  }

  switch( ch->substate )
  {
        default:
          break;
        case SUB_MORPH_DESC:
          if ( !ch->dest_buf )
          {
                send_to_char( "Fatal error: report to Shaddai.\n\r", ch );
                bug( "do_morphset: sub_morph_desc: NULL ch->dest_buf", 0 );
                ch->substate = SUB_NONE;
                return;
          }
          morph = ch->dest_buf;
          STRFREE( morph->description );
          morph->description = copy_buffer( ch );
          stop_editing( ch );
          ch->substate = ch->tempnum;
          if ( ch->substate == SUB_REPEATCMD )
	      ch->dest_buf = morph;
          return;
        case SUB_MORPH_HELP:
          if ( !ch->dest_buf )
          {
                send_to_char( "Fatal error: report to Shaddai.\n\r", ch );
                bug( "do_morphset: sub_morph_help: NULL ch->dest_buf", 0 );
                ch->substate = SUB_NONE;
                return;
          }
          morph = ch->dest_buf;
          STRFREE( morph->help );
          morph->help = copy_buffer( ch );
          stop_editing( ch );
          ch->substate = ch->tempnum;
          if ( ch->substate == SUB_REPEATCMD )
	      ch->dest_buf = morph;
          return;
  }

  morph = NULL; 
  smash_tilde (argument);

  if ( ch->substate == SUB_REPEATCMD )
  {
        morph = ch->dest_buf;
        if ( !morph )
	{
		send_to_char ("Someone deleted your morph!\n\r", ch);
		argument = "done";	
        }
	if (argument[0] == '\0' || !str_cmp (argument, " ") )
	{
		do_morphstat( ch, morph->name );
		return;
	}
        if ( !str_cmp( argument, "stat" ) )
        {
		strcpy ( buf, morph->name );
		strcat ( buf, " help" );
		do_morphstat( ch, buf );
		return;
        }
	if ( !str_cmp (argument, "done") || !str_cmp (argument, "off"))
	{
          send_to_char ("Morphset mode off.\n\r", ch);
          ch->substate = SUB_NONE;
          ch->dest_buf = NULL;
          if (ch->pcdata && ch->pcdata->subprompt) {
            STRFREE (ch->pcdata->subprompt);
            ch->pcdata->subprompt = NULL;
          }
          return;
        }
   }
   if ( morph )
   {
      strcpy (arg1, morph->name);
      argument = one_argument (argument, arg2);
      strcpy (arg3, argument);
   }
   else
   {
      argument = one_argument (argument, arg1);
      argument = one_argument (argument, arg2);
      strcpy (arg3, argument);
   }
  if (!str_cmp (arg1, "on"))
  {
      send_to_char ("Syntax: morphset <morph> on.\n\r", ch);
      return;
  }
  value = is_number( arg3 ) ? atoi( arg3 ) : -1;

  if ( atoi(arg3) < -1 && value == -1 )
      value = atoi(arg3);

  if ( ch->substate != SUB_REPEATCMD && arg1[0] != '\0' &&
       !str_cmp( arg1, "save" ) ) 
  {
	save_morphs( );
	send_to_char ("Morph data saved.\n\r", ch);
	return;
  }
  if (arg1[0] == '\0' || (arg2[0] == '\0' && ch->substate != SUB_REPEATCMD)
      || !str_cmp (arg1, "?"))
    {
      if (ch->substate == SUB_REPEATCMD)
        {
          if (morph)
            send_to_char ("Syntax: <field>  <value>\n\r", ch);
          else
            send_to_char ("Syntax: <morph> <field>  <value>\n\r", ch);
        }
      else
        send_to_char ("Syntax: morphset <morph> <field>  <value>\n\r", ch);
        send_to_char ("Syntax: morphset save\n\r", ch);
      send_to_char ("\n\r", ch);
      send_to_char_color ("&cField being one of:\n\r", ch);
      send_to_char_color ("&c-------------------------------------------------\n\r",ch);
      send_to_char ("  ac, affected, blood, bloodused, cha, class, con, damroll, dayto,\n\r",ch);
      send_to_char ("  dayfrom, deity, description, defpos, dex, dodge,\n\r",ch);
      send_to_char ("  favourused, gloryused, help, hitroll, hp, hpused, immune,\n\r",ch);
      send_to_char ("  int, str, keyword, lck, level, long, mana, manaused,\n\r", ch);
      send_to_char ("  morphother, morphself, move, moveused, name, noaffected,\n\r",ch); 
      send_to_char ("  nocast, noimmune, noresistant, noskill, nosusceptible,\n\r",ch);
      send_to_char ("  obj1, obj2, obj3, objuse1, objuse2, objuse3, parry,\n\r", ch);
      send_to_char ("  pkill, race, resistant, sav1, sav2, sav3, sav4, sav5,\n\r",ch);
      send_to_char ("  sex, short, skills, susceptible, timefrom, timer, timeto,\n\r", ch);
      send_to_char ("  tumble, unmorphother, unmorphself, wis.\n\r", ch );
      send_to_char_color ("&c-------------------------------------------------\n\r",ch);
      return;
    }

    if ( !morph ) {

        if ( !is_number( arg1 ) )
		morph = get_morph( arg1 );
        else
		morph = get_morph_vnum( atoi(arg1) );

	if ( morph == NULL ) {
		send_to_char ("That morph does not exist.\n\r", ch );
		return;
	}
    }
    if (!str_cmp (arg2, "on"))
      {
        CHECK_SUBRESTRICTED (ch);
        ch_printf (ch, "Morphset mode on. (Editing %s).\n\r",
                   morph->name);
        ch->substate = SUB_REPEATCMD;
        ch->dest_buf = morph;
        if (ch->pcdata)
          {
            if (ch->pcdata->subprompt) {
              STRFREE (ch->pcdata->subprompt);
              ch->pcdata->subprompt = NULL;
            }
            sprintf (buf, "<&CMorphset &W%s&w> %%i", morph->name);
            ch->pcdata->subprompt = STRALLOC (buf);
          }
        return;
      }
    if (!str_cmp ( arg2, "str" ) )
    {
	if ( value < -10 || value > 10 )
	{
		send_to_char("Strength must be a value from -10 to 10.\n\r",ch);
		return;
	}
	morph->str = value;
    }
    else if (!str_cmp ( arg2, "int" ) )
    {
	if ( value < -10 || value > 10 )
	{
		send_to_char("Intelligence must be a value from -10 to 10.\n\r",ch);
		return;
	}
	morph->inte = value;
    }
    else if (!str_cmp ( arg2, "wis" ) )
    {
	if ( value < -10 || value > 10 )
	{
		send_to_char("Wisdom must be a value from -10 to 10.\n\r",ch);
		return;
	}
	morph->wis = value;
    }
    else if (!str_cmp ( arg2, "defpos" ) )
    {
      if (value < 0 || value > POS_STANDING)
        {
          ch_printf (ch, "Position range is 0 to %d.\n\r", POS_STANDING);
          return;
        }
      morph->defpos = value;
    }
    else if (!str_cmp ( arg2, "dex" ) )
    {
	if ( value < -10 || value > 10 )
	{
		send_to_char("Dexterity must be a value from -10 to 10.\n\r",ch);
		return;
	}
	morph->dex = value;
    }
    else if (!str_cmp ( arg2, "con" ) )
    {
	if ( value < -10 || value > 10 )
	{
		send_to_char("Constitution must be a value from -10 to 10.\n\r",ch);
		return;
	}
	morph->con = value;
    }
    else if (!str_cmp ( arg2, "cha" ) )
    {
	if ( value < -10 || value > 10 )
	{
		send_to_char("Charisma must be a value from -10 to 10.\n\r",ch);
		return;
	}
	morph->cha = value;
    }
    else if (!str_cmp ( arg2, "lck" ) )
    {
	if ( value < -10 || value > 10 )
	{
		send_to_char("Luck must be a value from -10 to 10.\n\r",ch);
		return;
	}
	morph->lck = value;
    }
    else if (!str_cmp ( arg2, "sex") )
    {
	if ( (value < 0 || value > 2) && value != -1)
	{
		send_to_char("Sex must be a value from 0 to 2.\n\r", ch );
		return;
	}
	morph->sex = value;
    }
    else if ( !str_cmp ( arg2, "pkill") )
    {
	if ( !str_cmp ( arg3, "pkill" ) )
		morph->pkill = ONLY_PKILL;
	else if ( !str_cmp( arg3, "peace" ) )
		morph->pkill = ONLY_PEACEFULL;
	else if ( !str_cmp( arg3, "none" ) )
		morph->pkill = 0;
	else
	{
		send_to_char("Usuage: morphset <morph> pkill [pkill|peace|none]\n\r", ch );
		return;
	}
    }
    else if (!str_cmp ( arg2, "bloodused") )
    {
	if ( value < 0 || value > 60 )
	{
		send_to_char("Blood used is a value from 0 to 60.\n\r", ch );
		return;
	}
	morph->bloodused = value;
    }
    else if (!str_cmp ( arg2, "manaused") )
    {
	if ( value < 0 || value > 2000 )
	{
		send_to_char("Mana used is a value from 0 to 2000.\n\r", ch );
		return;
	}
	morph->manaused = value;
    }
    else if (!str_cmp( arg2, "moveused") )
    {
	if ( value < 0 || value > 2000 )
	{
		send_to_char("Move used is a value from 0 to 2000.\n\r", ch );
		return;
	}
	morph->moveused = value;
    }
    else if (!str_cmp( arg2, "hpused") )
    {
	if ( value < 0 || value > 2000 )
	{
		send_to_char("Hp used is a value from 0 to 2000.\n\r", ch );
		return;
	}
	morph->hpused = value;
    }
    else if ( !str_cmp( arg2, "favourused") )
    {
	if ( value < 0 || value > 2000 )
	{
		send_to_char("Favour used is a value from 0 to 2000.\n\r", ch );
		return;
	}
	morph->favourused = value;
    }
    else if ( !str_cmp( arg2, "gloryused") )
    {
	if ( value < 0 || value > 2000 )
	{
		send_to_char("Glory used is a value from 0 to 2000.\n\r",ch);
		return;
	}
	morph->gloryused = value;
    }
    else if ( !str_cmp( arg2, "timeto") )
    {
	if ( value < 0 || value > 23 )
	{
		send_to_char("Timeto is a value from 0 to 23.\n\r", ch );
		return;
	}
	morph->timeto = value;
    }
    else if ( !str_cmp( arg2, "timefrom") )
    {
	if ( value < 0 || value > 23 )
	{
		send_to_char("Timefrom is a value from 0 to 23.\n\r", ch );
		return;
	}
	morph->timefrom = value;
   }
   else if ( !str_cmp( arg2, "dayto") )
   {
	if ( value < 0 || value > 31 )
	{
		send_to_char("Dayto is a value from 0 to 31.\n\r", ch );
		return;
	}
	morph->dayto = value;
   }
   else if ( !str_cmp( arg2, "dayfrom") )
   {
	if ( value < 0 || value > 31 )
	{
		send_to_char("Dayfrom is a value from 0 to 31.\n\r", ch );
		return;
	}
	morph->dayfrom = value;
    }
    else if (!str_cmp ( arg2, "sav1" ) || !str_cmp( arg2, "savepoison") )
    {
        if (value < -30 || value > 30)
        {
          send_to_char ("Saving throw range is -30 to 30.\n\r", ch);
          return;
        }
	morph->saving_poison_death = value;
    }
    else if (!str_cmp ( arg2, "sav2" ) || !str_cmp( arg2, "savewand") )
    {
        if (value < -30 || value > 30)
        {
          send_to_char ("Saving throw range is -30 to 30.\n\r", ch);
          return;
        }
	morph->saving_wand = value;
    }
    else if (!str_cmp ( arg2, "sav3" ) || !str_cmp( arg2, "savepara") )
    {
        if (value < -30 || value > 30)
        {
          send_to_char ("Saving throw range is -30 to 30.\n\r", ch);
          return;
        }
	morph->saving_para_petri = value;
    }
    else if (!str_cmp ( arg2, "sav4" ) || !str_cmp( arg2, "savebreath") )
    {
        if (value < -30 || value > 30)
        {
          send_to_char ("Saving throw range is -30 to 30.\n\r", ch);
          return;
        }
	morph->saving_breath = value;
    }
    else if (!str_cmp ( arg2, "sav5" ) || !str_cmp( arg2, "savestaff") )
    {
        if (value < -30 || value > 30)
        {
          send_to_char ("Saving throw range is -30 to 30.\n\r", ch);
          return;
        }
	morph->saving_spell_staff = value;
    }
    else if (!str_cmp ( arg2, "timer" ) )
    {
	if ( value < -1 || value == 0 )
	{
	  send_to_char ("Timer must be -1 (None) or greater than 0.\n\r", ch);
	  return;
	}
	morph->timer = value;
    }
    else if (!str_cmp ( arg2, "hp" ) )
    {
	argument = one_argument( argument, arg3 );
	if ( !str_cmp ( arg3, "0"))
		arg3[0] = '\0';
	morph->hit = str_dup( arg3 );
    }
    else if (!str_cmp ( arg2, "mana" ) )
    {
	argument = one_argument( argument, arg3 );
	if ( !str_cmp ( arg3, "0"))
		arg3[0] = '\0';
	morph->mana = str_dup( arg3 );
    }
    else if (!str_cmp ( arg2, "move" ) )
    {
	argument = one_argument( argument, arg3 );
	if ( !str_cmp ( arg3, "0"))
		arg3[0] = '\0';
	morph->move = str_dup( arg3 );
    }
    else if (!str_cmp ( arg2, "blood" ) )
    {
	argument = one_argument( argument, arg3 );
	if ( !str_cmp ( arg3, "0"))
		arg3[0] = '\0';
	morph->blood = str_dup( arg3 );
    }
    else if (!str_cmp ( arg2, "ac" ) )
    {
	if ( value > 500 || value < -500 )
	{
	  send_to_char("Ac range is -500 to 500.\n\r", ch);
	  return;
	}
	morph->ac = value;
    }
    else if ( !str_cmp ( arg2, "hitroll" ) )
    {
	argument = one_argument( argument, arg3 );
	if ( !str_cmp ( arg3, "0"))
		arg3[0] = '\0';
	morph->hitroll = str_dup( arg3 );
    }
    else if ( !str_cmp ( arg2, "damroll" ) )
    {
	argument = one_argument( argument, arg3 );
	if ( !str_cmp ( arg3, "0"))
		arg3[0] = '\0';
	morph->damroll = str_dup( arg3 );
    }
    else if ( !str_cmp ( arg2, "dodge" ) )
    {
	if ( value > 100 || value < -100 )
	{
	  send_to_char ("Dodge range is -100 to 100.\n\r", ch );
	  return;
	}
	morph->dodge = value;
    }
    else if ( !str_prefix( "obj", arg2) )
    {
	int index;
	char temp[MAX_STRING_LENGTH];
	
	if ( arg2[3] == '\0' )
	{
		send_to_char("Obj 1, 2, or 3.\n\r", ch );
		return;
	}
	temp[0] = arg2[3];
	temp[1] = '\0';
	index = atoi(temp);
	if ( index >3 || index < 1 )
	{
		send_to_char("Obj 1, 2, or 3.\n\r", ch );
		return;
	}
        if ( ( get_obj_index( value ) ) == NULL )
	{
	   send_to_char("No such vnum.\n\r", ch );
	   return;
	}
 	morph->obj[index-1] = value; 
    }
    else if ( !str_cmp ( arg2, "parry" ) )
    {
	if ( value > 100 || value < -100 )
	{
	  send_to_char ("Dodge range is -100 to 100.\n\r", ch );
	  return;
	}
	morph->parry = value;
    }
    else if ( !str_cmp ( arg2, "tumble" ) )
    {
	if ( value > 100 || value < -100 )
	{
	  send_to_char ("Dodge range is -100 to 100.\n\r", ch );
	  return;
	}
	morph->tumble = value;
    }
    else if ( !str_cmp ( arg2, "level" ) )
    {
	if ( value < 0 || value > LEVEL_IMMORTAL )
	{
	  ch_printf (ch, "Level range is 0 to %d.\n\r", LEVEL_IMMORTAL);
	  return;
	}
	morph->level = value;
    }
    else if ( !str_prefix ( arg2, "objuse" ) )
    {
	int index;
	char temp[MAX_INPUT_LENGTH];
	
	if ( arg2[6] == '\0' )
	{
		send_to_char("Objuse 1, 2 or 3?\n\r", ch );
		return;
	}
	temp[0] = arg2[6];
	temp[1] = '\0';
	index = atoi( temp );
	if ( index > 3 || index < 1 )
	{
		send_to_char("Objuse 1, 2, or 3?\n\r", ch );
		return;
	}
	if ( value )
		morph->objuse[index-1] = TRUE;
	else
		morph->objuse[index-1] = FALSE;
    }
    else if ( !str_cmp ( arg2, "nocast" ) )
    {
	if ( value )
		morph->no_cast = TRUE;
	else
		morph->no_cast = FALSE;
    }
    else if ( !str_cmp ( arg2, "resistant" ) || !str_cmp( arg2, "r") )
    {
	if ( !argument || argument[0] == '\0' )
        {
           send_to_char( "Usage: morphset <morph> resistant <flag> [flag]...\n\r", ch );
           return;
        }
        while ( argument[0] != '\0' )
        {
           argument = one_argument( argument, arg3 );
           value = get_risflag( arg3 );
           if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
           else
                TOGGLE_BIT( morph->resistant, 1 << value );
        }
   }
   else if ( !str_cmp ( arg2, "susceptible" ) || !str_cmp( arg2, "s" ) )
   {
        if ( !argument || argument[0] == '\0' )
        {
           send_to_char( "Usage: morphset <morph> susceptible <flag> [flag]...\n\r", ch );
           return;
        }
        while ( argument[0] != '\0' )
        {
           argument = one_argument( argument, arg3 );
           value = get_risflag( arg3 );
           if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
           else
                TOGGLE_BIT( morph->suscept, 1 << value );
        }
   }
   else if ( !str_cmp ( arg2, "immune" ) || !str_cmp( arg2, "i" ) )
   {
        if ( !argument || argument[0] == '\0' )
        {
           send_to_char( "Usage: morphset <morph> immune <flag> [flag]...\n\r", ch ); 
           return;
        }
        while ( argument[0] != '\0' )
        {
           argument = one_argument( argument, arg3 );
           value = get_risflag( arg3 );
           if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
           else
                TOGGLE_BIT( morph->immune, 1 << value );
        }
   }
   else if ( !str_cmp ( arg2, "noresistant" ) || !str_cmp( arg2, "nr") )
    {
        if ( !argument || argument[0] == '\0' )
        {
           send_to_char( "Usage: morphset <morph> noresistant <flag> [flag]...\n\r", ch );
           return;
        }
        while ( argument[0] != '\0' )
        {
           argument = one_argument( argument, arg3 );
           value = get_risflag( arg3 );
           if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
           else
                TOGGLE_BIT( morph->no_resistant, 1 << value );
        }
   }
   else if ( !str_cmp ( arg2, "nosusceptible" ) || !str_cmp( arg2, "ns" ) )
   {
        if ( !argument || argument[0] == '\0' )
        {
           send_to_char( "Usage: morphset <morph> nosusceptible <flag> [flag]...\n\r", ch );
           return;
        }
        while ( argument[0] != '\0' )
        {
           argument = one_argument( argument, arg3 );
           value = get_risflag( arg3 );
           if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
           else
                TOGGLE_BIT( morph->no_suscept, 1 << value );
        }
   }
   else if ( !str_cmp ( arg2, "noimmune" ) || !str_cmp( arg2, "ni" ) )
   {
        if ( !argument || argument[0] == '\0' )
        {
           send_to_char( "Usage: morphset <morph> noimmune <flag> [flag]...\n\r", ch );
           return;
        }
        while ( argument[0] != '\0' )
        {
           argument = one_argument( argument, arg3 );
           value = get_risflag( arg3 );
           if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
           else
                TOGGLE_BIT( morph->no_immune, 1 << value );
        }
   }
   else if ( !str_cmp ( arg2, "affected" ) || !str_cmp( arg2, "aff" ) )
   {
        if ( !argument || argument[0] == '\0' )
        {
           send_to_char( "Usage: morphset <morph> affected <flag> [flag]...\n\r", ch );
           return;
        }
        while ( argument[0] != '\0' )
        {
           argument = one_argument( argument, arg3 );
           value = get_aflag( arg3 );
           if ( value < 0 || value > MAX_BITS )
             ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
           else
             xTOGGLE_BIT( morph->affected_by, value );
        }
   }
   else if ( !str_cmp ( arg2, "noaffected" ) || !str_cmp( arg2, "naff" ) )
   {
        if ( !argument || argument[0] == '\0' )
        {
           send_to_char( "Usage: morphset <morph> noaffected <flag> [flag]...\n\r", ch );
           return;
        }
        while ( argument[0] != '\0' )
        {
           argument = one_argument( argument, arg3 );
           value = get_aflag( arg3 );
           if ( value < 0 || value > MAX_BITS )
             ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
           else
             xTOGGLE_BIT( morph->no_affected_by, value );
        }
   }
   else if ( !str_cmp( arg2, "short" ) )
   {
        STRFREE( morph->short_desc );
        morph->short_desc = STRALLOC( arg3 );
   }
   else if ( !str_cmp( arg2, "morphother" ) )
   {
	STRFREE( morph->morph_other );
	morph->morph_other = STRALLOC( arg3 );
   }
   else if ( !str_cmp( arg2, "morphself" ) )
   {
	STRFREE( morph->morph_self );
	morph->morph_self = STRALLOC( arg3 );
   }
   else if ( !str_cmp( arg2, "unmorphother" ) )
   {
	STRFREE( morph->unmorph_other );
	morph->unmorph_other = STRALLOC( arg3 );
   }
   else if ( !str_cmp( arg2, "unmorphself" ) )
   {
	STRFREE( morph->unmorph_self );
	morph->unmorph_self = STRALLOC( arg3 );
   }
   else if ( !str_cmp( arg2, "keyword" ) )
   {
	STRFREE( morph->key_words );
	morph->key_words = STRALLOC( arg3 );
   }
   else if ( !str_cmp( arg2, "long" ) )
   {
        STRFREE( morph->long_desc );
        strcpy( buf, arg3 );
        strcat( buf, "\n\r" );
        morph->long_desc = STRALLOC( buf );
   }
   else if ( !str_cmp( arg2, "description") || !str_cmp( arg2, "desc" ) )
   {
        if ( arg3[0] )
        {
           STRFREE( morph->description );
           morph->description = STRALLOC( arg3 );
        }
        CHECK_SUBRESTRICTED( ch );
        if ( ch->substate == SUB_REPEATCMD )
          ch->tempnum = SUB_REPEATCMD;
        else
          ch->tempnum = SUB_NONE;
        ch->substate = SUB_MORPH_DESC;
        ch->dest_buf = morph;
        start_editing( ch, morph->description );
        return;
   }
   else if ( !str_cmp( arg2, "name")  )
   {
        STRFREE( morph->name );
        morph->name = STRALLOC( arg3 );
   }
   else if ( !str_cmp( arg2, "help" ) )
   {
        if ( arg3[0] )
        {
           STRFREE( morph->help );
           morph->help = STRALLOC( arg3 );
        }
        CHECK_SUBRESTRICTED( ch );
        if ( ch->substate == SUB_REPEATCMD )
          ch->tempnum = SUB_REPEATCMD;
        else
          ch->tempnum = SUB_NONE;
        ch->substate = SUB_MORPH_HELP;
        ch->dest_buf = morph;
        start_editing( ch, morph->help );
        return;
   }
   else if ( !str_cmp ( arg2, "skills" ) )
   {
	if ( !morph->skills )
		morph->skills = str_dup ("");

	if ( arg3[0] == '\0' || !str_cmp( arg3, "none" ) )
        {
		DISPOSE ( morph->skills );
		morph->skills = str_dup ("");
		return;
	}
	sprintf ( buf, "%s %s", morph->skills, arg3 );
	DISPOSE ( morph->skills );
	morph->skills = str_dup ( buf );
   }
   else if ( !str_cmp ( arg2, "noskills" ) )
   {
	if ( !morph->no_skills )
		morph->no_skills = str_dup ("");

	if ( arg3[0] == '\0' || !str_cmp( arg3, "none" ) )
        {
		DISPOSE ( morph->no_skills );
		morph->no_skills = str_dup ("");
		return;
	}
	sprintf ( buf, "%s %s", morph->no_skills, arg3 );
	DISPOSE ( morph->no_skills );
	morph->no_skills = str_dup ( buf );
   }
   else if ( !str_cmp ( arg2, "class" ) )
   {
       if ( !is_number( arg3 ) )
       {
	  int i;
	  for ( i = 0; i < MAX_PC_CLASS; i++ )
	    if ( !str_cmp ( class_table[i]->who_name, arg3 ) )
		break;
	  value = i;
       }
       if (value < 0 || value >= MAX_PC_CLASS)
        {
          ch_printf (ch, "Class range is 0 to %d.\n", MAX_PC_CLASS);
          return;
        }
        TOGGLE_BIT ( morph->class, ( 1 << value) );
   }
   else if ( !str_cmp ( arg2, "race" ) )
   {
        if ( is_number( arg3 ) )
		value = atoi (arg3);
        else
		value = get_pc_race (arg3);
        if ( value < 0 && !is_number( arg3 ) )
	{
	  ch_printf (ch, "Unknown race: %s", arg3 );
	  return;
	}
	else if ( value < 0 || value >= MAX_PC_RACE )
        {
          ch_printf (ch, "Race range is 0 to %d.\n", MAX_PC_RACE - 1);
          return;
        }
        TOGGLE_BIT ( morph->race, ( 1 << value) );
   }
   else if ( ch->substate == SUB_REPEATCMD )
   {
        ch->substate = SUB_RESTRICTED;
        interpret( ch, origarg );
        ch->substate = SUB_REPEATCMD;
        ch->last_cmd = do_morphset;
   }
   else
   {
        do_morphset( ch, "" );
	return;
   }
   send_to_char("Done.\n\r", ch );
   return;
}

/*
 *  Shows morph info on a given morph.
 *  To see the description and help file, must use morphstat <morph> help
 *  Shaddai
 */

void
do_morphstat ( CHAR_DATA *ch, char *argument )
{
    MORPH_DATA *morph;
    char arg[MAX_INPUT_LENGTH];
    int count = 1;

    set_pager_color( AT_CYAN, ch );

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_pager( "Morphstat what?\n\r", ch );
	return;
    }
    if (IS_NPC (ch))
    {
      send_to_char ("Mob's can't morphstat\n\r", ch);
      return;
    }

    if ( !str_cmp ( arg, "list") )
    {
        if ( !morph_start )
        {
           send_to_pager( "No morph's currently exist.\n\r", ch );
           return;
        }
	for ( morph = morph_start; morph; morph = morph->next )
	{
	  pager_printf_color ( ch, "&c[&C%2d&c]   Name:  &C%-13s    &cVnum:  &C%4d  &cUsed:  &C%3d\n\r",
		count, morph->name, morph->vnum, morph->used );
	  count++;
	}
	return;
    }
    if ( !is_number( arg ) )
	morph = get_morph( arg );
    else
	morph = get_morph_vnum( atoi(arg) );

    if (  morph == NULL )
    {
	send_to_pager( "No such morph exists.\n\r", ch );
	return;
    }
    if ( argument[0] == '\0' )
    {
	pager_printf_color (ch, "  &cMorph Name: &C%-20s  Vnum: %4d\n\r", morph->name, morph->vnum );
	send_to_pager_color( "&B[----------------------------------------------------------------------------]\n\r", ch );
        send_to_pager_color("                           &BMorph Restrictions\n\r", ch );
	send_to_pager_color( "&B[----------------------------------------------------------------------------]\n\r", ch );
	pager_printf_color (ch, "  &cClasses Allowed   : &w%s\n\r", 
		class_string (morph->class) );
	pager_printf_color (ch, "  &cRaces Not Allowed: &w%s\n\r", 
		race_string (morph->race) );
	pager_printf_color (ch, "  &cSex:  &C%s   &cPkill:   &C%s   &cTime From:   &C%d   &cTime To:    &C%d\n\r", 
        	morph->sex == SEX_MALE    ? "male"   :
        	morph->sex == SEX_FEMALE  ? "female" : "neutral",
		morph->pkill == ONLY_PKILL ? "YES"   :
		morph->pkill == ONLY_PEACEFULL ? "NO" : "n/a",
		morph->timefrom, morph->timeto );
	pager_printf_color (ch, "  &cDay From:  &C%d  &cDay To:  &C%d\n\r",
		morph->dayfrom, morph->dayto );
        pager_printf_color (ch, "  &cLevel:  &C%d       &cCasting Allowed   : &C%s\n\r",
                morph->level, (morph->no_cast)?"NO":"yes" );
	pager_printf_color (ch, "  &cUSAGES:  Mana:  &C%d  &cMove:  &C%d  &cHp:  &C%d  &cFavour:  &C%d  &cGlory:  &C%d\n\r",
		morph->manaused, morph->moveused, morph->hpused,
		morph->favourused, morph->gloryused );
	pager_printf_color (ch, "           &cBlood:  &C%d\n\r",
		morph->bloodused );
	pager_printf_color (ch, "  &cObj1: &C%d  &cObjuse1: &C%s   &cObj2: &C%d  &cObjuse2: &C%s   &cObj3: &C%d  &cObjuse3: &c%s\n\r",
		morph->obj[0], (morph->objuse[0]?"YES":"no"),
		morph->obj[1], (morph->objuse[1]?"YES":"no"),
		morph->obj[2], (morph->objuse[2]?"YES":"no"));
	pager_printf_color (ch, "  &cTimer: &w%d\n\r", morph->timer );
	send_to_pager_color( "&B[----------------------------------------------------------------------------]\n\r", ch );
	send_to_pager_color( "                       &BEnhancements to the Player\n\r", ch );
	send_to_pager_color( "&B[----------------------------------------------------------------------------]\n\r", ch );
	pager_printf_color (ch, "  &cStr: &C%2d&c )( Int: &C%2d&c )( Wis: &C%2d&c )( Dex: &C%2d&c )( Con: &C%2d&c )( Cha: &C%2d&c )( Lck: &C%2d&c )\n\r",
		morph->str, morph->inte, morph->wis, morph->dex, morph->con, 
		morph->cha, morph->lck );
	pager_printf_color (ch, "  &cSave versus: &w%d %d %d %d %d       &cDodge: &w%d  &cParry: &w%d  &cTumble: &w%d\n\r", 
		morph->saving_poison_death, morph->saving_wand,
		morph->saving_para_petri, morph->saving_breath,
		morph->saving_spell_staff, morph->dodge, morph->parry,
		morph->tumble );
	pager_printf_color (ch, "  &cHps     : &w%s    &cBlood  : &w%s    &cMana   : &w%s    &cMove      : &w%s\n\r",
		morph->hit, morph->blood, morph->mana, morph->move );
	pager_printf_color (ch, "  &cDamroll : &w%s    &cHitroll: &w%s    &cAC     : &w%d\n\r", 
		morph->damroll, morph->hitroll, morph->ac );
	send_to_pager_color( "&B[----------------------------------------------------------------------------]\n\r", ch );
	send_to_pager_color( "                          &BAffects to the Player\n\r", ch );
	send_to_pager_color( "&B[----------------------------------------------------------------------------]\n\r", ch );
	pager_printf_color (ch, "  &cAffected by: &C%s\n\r",
			affect_bit_name ( &morph->affected_by) );
	pager_printf_color (ch, "  &cImmune     : &w%s\n\r", 
			flag_string ( morph->immune, ris_flags ) );
	pager_printf_color (ch, "  &cSusceptible: &w%s\n\r", 
			flag_string ( morph->suscept, ris_flags ) );
	pager_printf_color (ch, "  &cResistant  : &w%s\n\r", 
			flag_string ( morph->resistant, ris_flags ) );
	pager_printf_color (ch, "  &cSkills     : &w%s\n\r", morph->skills );
	send_to_pager_color( "&B[----------------------------------------------------------------------------]\n\r", ch );
	send_to_pager_color( "                     &BPrevented affects to the Player\n\r", ch );
	send_to_pager_color( "&B[----------------------------------------------------------------------------]\n\r", ch );
	pager_printf_color (ch, "  &cNot affected by: &C%s\n\r",
			affect_bit_name ( &morph->no_affected_by) );
	pager_printf_color (ch, "  &cNot Immune     : &w%s\n\r", 
			flag_string ( morph->no_immune, ris_flags ) );
	pager_printf_color (ch, "  &cNot Susceptible: &w%s\n\r", 
			flag_string ( morph->no_suscept, ris_flags ) );
	pager_printf_color (ch, "  &cNot Resistant  : &w%s\n\r", 
			flag_string ( morph->no_resistant, ris_flags ) );
        pager_printf_color (ch, "  &cNot Skills     : &w%s\n\r",
			morph->no_skills);
	send_to_pager_color( "&B[----------------------------------------------------------------------------]\n\r", ch );
        send_to_char ("\n\r", ch);
    }
    else if ( !str_cmp ( argument, "help") || !str_cmp ( argument, "desc") )
    { 
	pager_printf_color(ch,"  &cMorph Name  : &C%-20s\n\r", morph->name );
	pager_printf_color(ch,"  &cDefault Pos : &w%d\n\r",  morph->defpos );
	pager_printf_color(ch,"  &cKeywords    : &w%s\n\r",morph->key_words );
	pager_printf_color(ch,"  &cShortdesc   : &w%s\n\r",
	        (morph->short_desc[0] == '\0')?"(none set)":morph->short_desc );
	pager_printf_color(ch,"  &cLongdesc    : &w%s",
		(morph->long_desc[0] == '\0')?"(none set)\n\r":
	        morph->long_desc );
	pager_printf_color(ch,"  &cMorphself   : &w%s\n\r",morph->morph_self );
	pager_printf_color(ch,"  &cMorphother  : &w%s\n\r",morph->morph_other );
	pager_printf_color(ch,"  &cUnMorphself : &w%s\n\r",morph->unmorph_self);
	pager_printf_color(ch,"  &cUnMorphother: &w%s\n\r",morph->unmorph_other);
	send_to_pager_color( "&B[----------------------------------------------------------------------------]\n\r", ch );
	pager_printf_color (ch, "                                  &cHelp:\n\r&w%s\n\r", morph->help );
	send_to_pager_color( "&B[----------------------------------------------------------------------------]\n\r", ch );
	pager_printf_color (ch, "                               &cDescription:\n\r&w%s\n\r", morph->description );
	send_to_pager_color( "&B[----------------------------------------------------------------------------]\n\r", ch );
    }
    else 
    {
	send_to_char ("Syntax: morphstat <morph>\n\r", ch );
	send_to_char ("Syntax: morphstat <morph> <help/desc>\n\r", ch );
    }
    return;
}

char *
class_string ( int bitvector )
{
  static char buf[MAX_STRING_LENGTH];
  int x;

  buf[0] = '\0';
  for (x = 0; x < MAX_PC_CLASS; x++)
    if (IS_SET (bitvector, 1 << x))
      {
        strcat (buf, class_table[x]->who_name);
        strcat (buf, " ");
      }
  if ((x = strlen (buf)) > 0)
    buf[--x] = '\0';

  return buf;
}

char *
race_string ( int bitvector )
{
  static char buf[MAX_STRING_LENGTH];
  int x;

  buf[0] = '\0';
  for (x = 0; x < MAX_PC_RACE; x++)
    if (IS_SET (bitvector, 1 << x))
      {
        strcat (buf, race_table[x]->race_name);
        strcat (buf, " ");
      }
  if ((x = strlen (buf)) > 0)
    buf[--x] = '\0';

  return buf;
}

/*
 * Given the Morph's name, returns the pointer to the morph structure.
 * --Shaddai
 */

MORPH_DATA *
get_morph (  char *arg )
{
  MORPH_DATA *morph;

  if ( arg[0] == '\0' )
	return NULL;
  for ( morph = morph_start; morph; morph = morph->next )
	if ( !str_cmp ( morph->name, arg ) )
		break;
  return morph;
}

/*
 * Find a morph you can use -- Shaddai
 */
MORPH_DATA *find_morph ( CHAR_DATA *ch, char *target, bool is_cast)
{
   MORPH_DATA *morph;
   if ( target[0] == '\0' )
   	return NULL;
   for ( morph = morph_start; morph; morph = morph->next )
   {
   	if ( str_cmp( morph->name, target) )
		continue;
	if ( can_morph( ch, morph, is_cast ) )
		break;
   }
   return morph;
}


/*
 * Given the Morph's vnum, returns the pointer to the morph structure.
 * --Shaddai
 */

MORPH_DATA *
get_morph_vnum ( int vnum )
{
  MORPH_DATA *morph;

  if ( vnum < 1  )
	return NULL;
  for ( morph = morph_start; morph; morph = morph->next )
	if ( morph->vnum == vnum )
		break;
  return morph;
}

/*
 * These functions wrap the sending of morph stuff, with morphing the code.
 * In most cases this is what should be called in the code when using spells,
 * obj morphing, etc... --Shaddai
 */
int
do_morph_char ( CHAR_DATA *ch, MORPH_DATA *morph )
{
  bool canmorph = TRUE;
  OBJ_DATA *obj;
  OBJ_DATA *tmpobj;

  if ( ch->morph )
	canmorph = FALSE;
  if ( morph->obj[0] )
  {
     if ((obj =get_obj_vnum(ch, morph->obj[0])) == NULL) 
     	canmorph = FALSE;
     else if ( morph->objuse[0] )
     {
    		act( AT_OBJECT, "$p disappears in a whisp of smoke!",
                  		obj->carried_by, obj, NULL, TO_CHAR ); 
    		if ( obj == get_eq_char( obj->carried_by, WEAR_WIELD )
    		&& (tmpobj=get_eq_char(obj->carried_by,WEAR_DUAL_WIELD))!=NULL)
       			tmpobj->wear_loc = WEAR_WIELD;
        	separate_obj( obj ); 
		extract_obj( obj );
      }
  }
  if ( morph->obj[1] )
  {
     if ((obj =get_obj_vnum(ch, morph->obj[1])) == NULL) 
     	canmorph = FALSE;
     else if ( morph->objuse[1] )
     {
    		act( AT_OBJECT, "$p disappears in a whisp of smoke!",
                  		obj->carried_by, obj, NULL, TO_CHAR ); 
    		if ( obj == get_eq_char( obj->carried_by, WEAR_WIELD )
    		&& (tmpobj=get_eq_char(obj->carried_by,WEAR_DUAL_WIELD))!=NULL)
       			tmpobj->wear_loc = WEAR_WIELD;
        	separate_obj( obj ); 
		extract_obj( obj );
      }
  }
  if ( morph->obj[2] )
  {
     if ((obj =get_obj_vnum(ch, morph->obj[2])) == NULL) 
     	canmorph = FALSE;
     else if ( morph->objuse[2] )
     {
    		act( AT_OBJECT, "$p disappears in a whisp of smoke!",
                  		obj->carried_by, obj, NULL, TO_CHAR ); 
    		if ( obj == get_eq_char( obj->carried_by, WEAR_WIELD )
    		&& (tmpobj=get_eq_char(obj->carried_by,WEAR_DUAL_WIELD))!=NULL)
       			tmpobj->wear_loc = WEAR_WIELD;
        	separate_obj( obj ); 
		extract_obj( obj );
      }
  }

  if ( morph->hpused )
  {
  	if ( ch->hit < morph->hpused )
  		canmorph = FALSE;
  	else
  		ch->hit -= morph->hpused;
  }

  if ( morph->moveused )
  {
  	if ( ch->move < morph->moveused )
  		canmorph = FALSE;
  	else
  		ch->move -= morph->moveused;
  }

  if ( morph->manaused && !IS_VAMPIRE(ch) )
  {
  	if ( ch->mana < morph->manaused )
  		canmorph = FALSE;
  	else
  		ch->mana -= morph->manaused;
  }

  if ( morph->bloodused && IS_VAMPIRE(ch) ) 
  {
       if ( ch->pcdata->condition[COND_BLOODTHIRST] < morph->bloodused )
        canmorph = FALSE;
       else
      	ch->pcdata->condition[COND_BLOODTHIRST]-= morph->bloodused;
  }

  if ( morph->favourused )
  {
  	if ( IS_NPC(ch) || !ch->pcdata->deity || 
		ch->pcdata->favor < morph->favourused )
       		canmorph = FALSE;
	else {
		ch->pcdata->favor -= morph->favourused;
		if ( ch->pcdata->favor < ch->pcdata->deity->susceptnum )
		{
	    	  SET_BIT( ch->susceptible, ch->pcdata->deity->suscept );
		}
	}
  }

  if ( morph->gloryused )
  {
  	if ( !IS_NPC(ch) && ch->pcdata->quest_curr < morph->gloryused )
       		canmorph = FALSE;
  	else
  		ch->pcdata->quest_curr -= morph->gloryused;
  }

  if ( !canmorph )
  {
  	send_to_char ( "You begin to transform, but something goes wrong.\n\r",
		ch );
  	return FALSE;	
  }
 
  send_morph_message ( ch, morph, TRUE );
  do_morph ( ch, morph );
  return TRUE;
}

void
do_unmorph_char ( CHAR_DATA *ch )
{
  MORPH_DATA *temp;
  if ( !ch->morph )
	return;
  temp = ch->morph->morph;
  do_unmorph ( ch );
  send_morph_message ( ch, temp, FALSE );
  return;
}

/*
 * This function sends the morph/unmorph message to the people in the room.
 * -- Shaddai
 */
void
send_morph_message ( CHAR_DATA *ch, MORPH_DATA *morph, bool is_morph )
{
  if ( morph == NULL )
	return;
  if ( is_morph )
  {
	act ( AT_MORPH, morph->morph_other, ch, NULL, NULL, TO_ROOM );
	act ( AT_MORPH, morph->morph_self, ch, NULL, NULL, TO_CHAR );
  }
  else 
  {
	act ( AT_MORPH, morph->unmorph_other, ch, NULL, NULL, TO_ROOM );
	act ( AT_MORPH, morph->unmorph_self, ch, NULL, NULL, TO_CHAR );
  }
  return;
}

/*
 * Checks to see if the char meets all the requirements to morph into
 * the provided morph.  Doesn't Look at NPC's class or race as they
 * are different from pc's, but still checks level and if they can
 * cast it or not.  --Shaddai
 */

bool
can_morph ( CHAR_DATA *ch, MORPH_DATA *morph, bool is_cast )
{
   if ( morph == NULL)
	return FALSE;
   if ( IS_IMMORTAL ( ch ) || IS_NPC(ch) ) /* Let immortals morph to anything
					      Also NPC can do any morph  */
	return TRUE;
   if ( morph->no_cast && is_cast )
        return FALSE;
   if ( ch->level < morph->level )
        return FALSE;
   if ( morph->pkill == ONLY_PKILL && !IS_PKILL(ch) )
	return FALSE;
   if ( morph->pkill == ONLY_PEACEFULL && IS_PKILL(ch) )
	return FALSE;
   if ( morph->sex != -1 && morph->sex != ch->sex )
	return FALSE;
   if ( morph->class != 0 && !IS_SET( morph->class, (1 << ch->class) ) )
        return FALSE;
   if ( morph->race != 0 && IS_SET( morph->race, (1 << ch->race ) ) )
        return FALSE;
   if ( morph->deity && ( !ch->pcdata->deity || !get_deity ( morph->deity ) ) )
        return FALSE;
   if ( morph->timeto != -1 && morph->timefrom != -1 && 
      ( morph->timeto < time_info.hour || morph->timefrom > time_info.hour ) )
	return FALSE;
   if ( morph->dayfrom != -1 && morph->dayto != -1 &&
      ( morph->dayto < (time_info.day+1) || morph->dayfrom > (time_info.day+1)))
	return FALSE;
   return TRUE;
}

/*
 * Workhorse of the polymorph code, this turns the player into the proper
 * morph.  Doesn't check if you can morph or not so must use can_morph before
 * this is called.  That is so we can let people morph into things without
 * checking. Also, if anything is changed here, make sure it gets changed
 * in do_unmorph otherwise you will be giving your player stats for free.
 * This also does not send the message to people when you morph good to
 * use in save functions.
 * --Shaddai
 */	

void
do_morph ( CHAR_DATA *ch, MORPH_DATA *morph )
{
  CHAR_MORPH *ch_morph;

  if ( !morph )
	return;

  if ( ch->morph )
	do_unmorph_char ( ch );

  ch_morph 		   = make_char_morph ( morph );
  ch->armor               += morph->ac;
  ch->mod_str             += morph->str;
  ch->mod_int             += morph->inte;
  ch->mod_wis             += morph->wis;
  ch->mod_dex             += morph->dex;
  ch->mod_cha             += morph->cha;
  ch->mod_lck             += morph->lck;
  ch->saving_breath       += morph->saving_breath;
  ch->saving_para_petri   += morph->saving_para_petri;
  ch->saving_poison_death += morph->saving_poison_death;
  ch->saving_spell_staff  += morph->saving_spell_staff;
  ch->saving_wand         += morph->saving_wand;
  ch_morph->hitroll	   = dice_parse( ch, morph->level, morph->hitroll );
  ch->hitroll		  += ch_morph->hitroll;
  ch_morph->damroll	   = dice_parse( ch, morph->level, morph->damroll );
  ch->damroll		  += ch_morph->damroll;
  ch_morph->hit		   = dice_parse( ch, morph->level, morph->hit );
  if ( (ch->hit + ch_morph->hit ) > 32700 )
	ch_morph->hit = (32700 - ch->hit);
  ch->hit		  += ch_morph->hit;
  ch_morph->move	   = dice_parse( ch, morph->level, morph->move );
  if ( (ch->move + ch_morph->move ) > 32700 )
	ch_morph->move = (32700 - ch->move);
  ch->move		  += ch_morph->move;
  if ( !IS_NPC(ch) && IS_VAMPIRE ( ch ) )
  {
  	ch_morph->blood = dice_parse(ch,morph->level,morph->blood);
	ch->pcdata->condition[COND_BLOODTHIRST] += ch_morph->blood;
  }
  else
  {
  	ch_morph->mana	   = dice_parse( ch, morph->level, morph->mana );
  	if ( (ch->mana + ch_morph->mana ) > 32700 )
		ch_morph->mana = (32700 - ch->mana);
 	ch->mana	  += ch_morph->mana;
  }
  xSET_BITS ( ch->affected_by, morph->affected_by );
  SET_BIT ( ch->immune, morph->immune );
  SET_BIT ( ch->resistant, morph->resistant );
  SET_BIT ( ch->susceptible, morph->suscept );
  xREMOVE_BITS ( ch->affected_by, morph->no_affected_by );
  REMOVE_BIT ( ch->immune, morph->no_immune );
  REMOVE_BIT ( ch->resistant, morph->no_resistant );
  REMOVE_BIT ( ch->susceptible, morph->no_suscept );
  ch->morph = ch_morph;
  morph->used++;
  return;
}

/*
 * This makes sure to take all the affects given to the player by the morph
 * away.  Several things to keep in mind.  If you add something here make
 * sure you add it to do_morph as well (Unless you want to charge your players
 * for morphing ;) ).  Also make sure that their pfile saves with the morph
 * affects off, as the morph may not exist when they log back in.  This
 * function also does not send the message to people when you morph so it is
 * good to use in save functions and other places you don't want people to
 * see the stuff.
 * --Shaddai
 */
void 
do_unmorph ( CHAR_DATA *ch )
{
  CHAR_MORPH *morph;

  if ( (morph = ch->morph) == NULL )
	return;
  ch->armor               -= morph->ac;
  ch->mod_str             -= morph->str;
  ch->mod_int             -= morph->inte;
  ch->mod_wis             -= morph->wis;
  ch->mod_dex             -= morph->dex;
  ch->mod_cha             -= morph->cha;
  ch->mod_lck             -= morph->lck;
  ch->saving_breath       -= morph->saving_breath;
  ch->saving_para_petri   -= morph->saving_para_petri;
  ch->saving_poison_death -= morph->saving_poison_death;
  ch->saving_spell_staff  -= morph->saving_spell_staff;
  ch->saving_wand         -= morph->saving_wand;
  ch->hitroll             -= morph->hitroll;
  ch->damroll             -= morph->damroll;
  ch->hit                 -= morph->hit;
  ch->move                -= morph->move;
  if ( !IS_NPC(ch) && IS_VAMPIRE ( ch ) )
  	ch->pcdata->condition[COND_BLOODTHIRST]-=morph->blood;
  else
  	ch->mana          -= morph->mana;
  xREMOVE_BITS( ch->affected_by, morph->affected_by );
  REMOVE_BIT ( ch->immune, morph->immune );
  REMOVE_BIT ( ch->resistant, morph->resistant );
  REMOVE_BIT ( ch->susceptible, morph->suscept );
  free_char_morph ( ch->morph );
  ch->morph = NULL;
  update_aris ( ch );
  return;
}

/*
 * This function saves the morph data.  Should be put in on reboot or shutdown
 * to make use of the sort algorithim. --Shaddai
 */
void
save_morphs ( )
{
  MORPH_DATA *morph;
  FILE *fp;

  fclose( fpReserve );
  if ((fp = fopen (SYSTEM_DIR MORPH_FILE, "w")) == NULL)
    {
      bug ("Save_morph: fopen", 0);
      perror (SYSTEM_DIR MORPH_FILE);
      fpReserve = fopen( NULL_FILE, "r" );
      return;
    }
  for ( morph = morph_start; morph; morph = morph->next )
 	fwrite_morph ( fp, morph );
  fprintf ( fp, "#END\n" );
  fclose ( fp );
  fpReserve = fopen( NULL_FILE, "r" );
  return;
}

/* 
 * Write one morph structure to a file. It doesn't print the variable to file
 * if it hasn't been set why waste disk-space :)  --Shaddai
 */

void
fwrite_morph ( FILE *fp, MORPH_DATA *morph )
{
  if ( !morph )
	return;
  fprintf (fp, "Morph           	%s\n",  morph->name );
  if ( morph->obj[0] != 0 || morph->obj[1] != 0 || morph->obj[2] != 0 )
        fprintf (fp, "Objs  			%d %d %d\n",
		morph->obj[0], morph->obj[1], morph->obj[2] );
  if ( morph->objuse[0] != 0 || morph->objuse[1] != 0 || morph->objuse[2] != 0 )
        fprintf ( fp, "Objuse			%d %d %d\n",
		morph->objuse[0], morph->objuse[1], morph->objuse[2] );
  if ( morph->vnum != 0  )
	fprintf (fp, "Vnum              %d\n",  morph->vnum );
  if ( morph->blood[0] != '\0' )
  	fprintf (fp, "Blood    	        %s~\n", morph->blood );
  if ( morph->damroll[0] != '\0' )
  	fprintf (fp, "Damroll         	%s~\n", morph->damroll );
  if ( morph->defpos != POS_STANDING )
	fprintf (fp, "Defpos            %d\n",  morph->defpos );
  if ( morph->description[0] != '\0' )
  	fprintf (fp, "Description     	%s~\n", morph->description ); 
  if ( morph->help[0] != '\0' )
  	fprintf (fp, "Help            	%s~\n", morph->help );
  if ( morph->hit[0] != '\0' )
  	fprintf (fp, "Hit             	%s~\n", morph->hit );
  if ( morph->hitroll[0] != '\0' )
  	fprintf (fp, "Hitroll         	%s~\n", morph->hitroll );
  if ( morph->key_words[0] != '\0' )
  	fprintf (fp, "Keywords        	%s~\n", morph->key_words );
  if ( morph->long_desc[0] != '\0' )
  	fprintf (fp, "Longdesc        	%s~\n", morph->long_desc );
  if ( morph->mana[0] != '\0' )
  	fprintf (fp, "Mana            	%s~\n", morph->mana );
  if ( morph->morph_other[0] != '\0' )
  	fprintf (fp, "MorphOther      	%s~\n", morph->morph_other );
  if ( morph->morph_self[0] != '\0' )
  	fprintf (fp, "MorphSelf       	%s~\n", morph->morph_self );
  if ( morph->move[0] != '\0' )
  	fprintf (fp, "Move            	%s~\n", morph->move );
  if ( morph->no_skills[0] != '\0' )
	fprintf ( fp, "NoSkills         %s~\n", morph->no_skills );
  if ( morph->short_desc[0] != '\0' )
  	fprintf (fp, "ShortDesc       	%s~\n", morph->short_desc );
  if ( morph->skills[0] != '\0' )
  	fprintf (fp, "Skills          	%s~\n", morph->skills );
  if ( morph->unmorph_other[0] != '\0' )
  	fprintf (fp, "UnmorphOther    	%s~\n", morph->unmorph_other );
  if ( morph->unmorph_self[0] != '\0' )
  	fprintf (fp, "UnmorphSelf     	%s~\n", morph->unmorph_self );
  if ( !xIS_EMPTY(morph->affected_by) ) 
  	fprintf (fp, "Affected        	%s\n",  print_bitvector(&morph->affected_by) );
  if ( morph->class != 0 ) 
  	fprintf (fp, "Class           	%s~\n",  class_string(morph->class) );
  if ( morph->immune != 0 ) 
  	fprintf (fp, "Immune          	%d\n",  morph->immune );
  if ( !xIS_EMPTY(morph->no_affected_by) ) 
  	fprintf (fp, "NoAffected      	%s\n",  print_bitvector(&morph->no_affected_by) );
  if ( morph->no_immune != 0 ) 
  	fprintf (fp, "NoImmune        	%d\n",  morph->no_immune );
  if ( morph->no_resistant != 0 ) 
  	fprintf (fp, "NoResistant     	%d\n",  morph->no_resistant );
  if ( morph->no_suscept != 0 ) 
  	fprintf (fp, "NoSuscept       	%d\n",  morph->no_suscept );
  if ( morph->race != 0 ) 
  	fprintf (fp, "Race        	%s~\n",  race_string(morph->race) );
  if ( morph->resistant != 0 ) 
  	fprintf (fp, "Resistant       	%d\n",  morph->resistant );
  if ( morph->suscept != 0 ) 
  	fprintf (fp, "Suscept        	%d\n",  morph->suscept );
  if ( morph->used != 0 ) 
  	fprintf (fp, "Used       	%d\n",  morph->used );
  if ( morph->sex != -1 )
	fprintf ( fp, "Sex		%d\n",  morph->sex );
  if ( morph->pkill != 0 )
	fprintf ( fp, "Pkill		%d\n",  morph->pkill );
  if ( morph->timefrom != -1 )
	fprintf ( fp, "TimeFrom		%d\n",  morph->timefrom );
  if ( morph->timeto != -1 )
	fprintf ( fp, "TimeTo		%d\n",  morph->timeto );
  if ( morph->dayfrom != -1 )
	fprintf ( fp, "DayFrom		%d\n",  morph->dayfrom );
  if ( morph->dayto != -1 )
	fprintf ( fp, "DayTo		%d\n",  morph->dayto );
  if ( morph->bloodused != 0 )
  	fprintf ( fp, "BloodUsed	%d\n",  morph->bloodused );
  if ( morph->manaused != 0 )
	fprintf ( fp, "ManaUsed		%d\n",  morph->manaused );
  if ( morph->moveused != 0 )
	fprintf ( fp, "MoveUsed		%d\n",  morph->moveused );
  if ( morph->hpused != 0 )
	fprintf ( fp, "HpUsed		%d\n",  morph->hpused );
  if ( morph->favourused != 0 )
	fprintf ( fp, "FavourUsed	%d\n",  morph->favourused );
  if ( morph->gloryused != 0 )
	fprintf ( fp, "GloryUsed	%d\n",  morph->gloryused );
  if ( morph->ac != 0 ) 
  	fprintf (fp, "Armor        	%d\n",  morph->ac );
  if ( morph->cha != 0 ) 
  	fprintf (fp, "Charisma        	%d\n",  morph->cha );
  if ( morph->con != 0 ) 
  	fprintf (fp, "Constitution    	%d\n",  morph->con );
  if ( morph->dex != 0 ) 
  	fprintf (fp, "Dexterity       	%d\n",  morph->dex );
  if ( morph->dodge != 0 ) 
  	fprintf (fp, "Dodge        	%d\n",  morph->dodge );
  if ( morph->inte != 0 ) 
  	fprintf (fp, "Intelligence    	%d\n",  morph->inte );
  if ( morph->lck != 0 ) 
  	fprintf (fp, "Luck        	%d\n",  morph->lck );
  if ( morph->level != 0 ) 
  	fprintf (fp, "Level        	%d\n",  morph->level );
  if ( morph->parry != 0 ) 
  	fprintf (fp, "Parry        	%d\n",  morph->parry );
  if ( morph->saving_breath != 0 ) 
  	fprintf (fp, "SaveBreath      	%d\n",  morph->saving_breath );
  if ( morph->saving_para_petri != 0 ) 
  	fprintf (fp, "SavePara        	%d\n",  morph->saving_para_petri );
  if ( morph->saving_poison_death != 0 ) 
  	fprintf (fp, "SavePoison      	%d\n",  morph->saving_poison_death );
  if ( morph->saving_spell_staff != 0 ) 
  	fprintf (fp, "SaveSpell       	%d\n",  morph->saving_spell_staff );
  if ( morph->saving_wand != 0 ) 
  	fprintf (fp, "SaveWand        	%d\n",  morph->saving_wand );
  if ( morph->str != 0 ) 
  	fprintf (fp, "Strength        	%d\n",  morph->str );
  if ( morph->tumble != 0 ) 
  	fprintf (fp, "Tumble          	%d\n",  morph->tumble );
  if ( morph->wis != 0 ) 
  	fprintf (fp, "Wisdom          	%d\n",  morph->wis );
  if ( morph->no_cast ) 
  	fprintf (fp, "NoCast          	%d\n",  morph->no_cast );
  fprintf (fp, "End\n\n" );
  return;
}

/*
 *  This function loads in the morph data.  Note that this function MUST be
 *  used AFTER the race and class tables have been read in and setup.
 *  --Shaddai
 */

void
load_morphs ( )
{
  MORPH_DATA *morph;
  char buf[MAX_INPUT_LENGTH];
  char *word;
  FILE *fp;
  bool my_continue = TRUE;
  bool fMatch = FALSE;

  if ((fp = fopen (SYSTEM_DIR MORPH_FILE, "r")) == NULL)
    {
      bug ("Load_morph: fopen", 0);
      perror (SYSTEM_DIR MORPH_FILE);
      return;
    }

  while (my_continue)
    {
      morph = NULL;
      word = feof (fp) ? "#END" : fread_word (fp);
      fMatch = FALSE;

      switch (UPPER (word[0]))
        {
        case '#':
	  if (!str_cmp (word, "#END" ) )
	  {
	     fclose (fp);
	     fMatch = TRUE;
              my_continue = FALSE;
              break;
	  }
        break;
        case 'M':
          if (!str_cmp (word, "Morph"))
            {
              morph = fread_morph (fp);
              fMatch = TRUE;
            }
          break;
        }
      if (!fMatch)
      {
          sprintf (buf, "Fread_morph: no match: %s", word);
          bug (buf, 0);
      }
      if ( morph )
        LINK (morph, morph_start, morph_end, next, prev);
    }
  if ( sysdata.morph_opt )
  {
  	log_string("Optimizing Morphs.");
  	sort_morphs ( );
  }
  setup_morph_vnum ( );
  log_string("Done.");
  return;
}

/*
 * Read in one morph structure
 */

MORPH_DATA * 
fread_morph ( FILE *fp )
{
  MORPH_DATA *morph;
  char buf[MAX_INPUT_LENGTH];
  char *arg;
  char temp[MAX_STRING_LENGTH];
  char *word;
  int i;
  bool fMatch;

  word = feof (fp) ? "End" : fread_word (fp);

  if ( !str_cmp ( word, "End" ) )
	return NULL;

  CREATE ( morph, MORPH_DATA, 1);
  morph_defaults ( morph );
  morph->name = str_dup ( word );

  for  (;;)
  {
      word = feof (fp) ? "End" : fread_word (fp);
      fMatch = FALSE;
      switch (UPPER (word[0]))
      {
	case 'A':
		KEY ( "Armor", morph->ac, fread_number ( fp ) );
		KEY ( "Affected", morph->affected_by, fread_bitvector( fp ) );
		break;
	case 'B':
		KEY ( "Blood", morph->blood, fread_string_nohash ( fp ) );
		KEY ( "BloodUsed", morph->bloodused, fread_number ( fp ) );
		break;
	case 'C':
		KEY( "Charisma", morph->cha, fread_number ( fp ) );
		if ( !str_cmp ( word, "Class" ) ) 
		{
		  fMatch = TRUE;
		  arg = fread_string_nohash ( fp );
		  arg = one_argument( arg, temp );
		  while ( temp[0] != '\0' )
		  {
		  	for ( i=0; i < MAX_PC_CLASS; i++ )
			  if ( !str_cmp( temp, class_table[i]->who_name ) )
		          {
				SET_BIT( morph->class, (1 << i) );
				break;
			  }
		        arg = one_argument( arg, temp );
		  }
		} 
		KEY( "Constitution", morph->con, fread_number ( fp ) );
		break;
	case 'D':
		KEY( "Damroll", morph->damroll, fread_string_nohash(fp));
		KEY( "DayFrom", morph->dayfrom, fread_number( fp ) );
		KEY( "DayTo",   morph->dayto,   fread_number( fp ) );
		KEY( "Defpos",  morph->defpos,  fread_number( fp ) );
		KEY( "Description",morph->description,fread_string_nohash(fp));
		KEY( "Dexterity", morph->dex, fread_number ( fp ) );
		KEY( "Dodge", morph->dodge, fread_number( fp ) );
		break;
	case 'E':
          	if (!str_cmp (word, "End"))
          		return morph;
		break;
	case 'F':
		KEY( "FavourUsed", morph->favourused, fread_number( fp ) );
		break;
	case 'G':
		KEY( "GloryUsed", morph->gloryused, fread_number( fp ) );
		break;
	case 'H':
		KEY( "Help", morph->help, fread_string_nohash(fp));
		KEY( "Hit", morph->hit, fread_string_nohash(fp));
		KEY( "Hitroll", morph->hitroll, fread_string_nohash(fp));
		KEY( "HpUsed", morph->hpused, fread_number( fp ) );
		break;
	case 'I':
		KEY( "Intelligence", morph->inte, fread_number( fp ) );
		KEY( "Immune", morph->immune, fread_number( fp ) );
		break;
	case 'K':
	  	KEY( "Keywords", morph->key_words, fread_string_nohash(fp));
		break;
	case 'L':
		KEY( "Level", morph->level, fread_number ( fp ) );
		KEY( "Longdesc", morph->long_desc, fread_string_nohash(fp));
		KEY( "Luck", morph->lck, fread_number( fp ) );
		break;
	case 'M':
		KEY( "Mana", morph->mana, fread_string_nohash(fp));
		KEY( "ManaUsed", morph->manaused, fread_number( fp ) );
		KEY( "MorphOther", morph->morph_other,fread_string_nohash(fp));
		KEY( "MorphSelf", morph->morph_self, fread_string_nohash(fp));
		KEY( "Move", morph->morph_self, fread_string_nohash(fp));
		KEY( "MoveUsed", morph->moveused, fread_number( fp ) );
		break;
	case 'N':
		KEY( "NoAffected", morph->no_affected_by, fread_bitvector(fp));
		KEY( "NoImmune", morph->no_immune, fread_number( fp ) );
		KEY( "NoResistant", morph->no_resistant, fread_number(fp));
                KEY( "NoSkills", morph->no_skills, fread_string_nohash(fp));
		KEY( "NoSuscept", morph->no_suscept, fread_number(fp));
		if ( !str_cmp ( word, "NoCast" ) )
		{
		  fMatch = TRUE;
		  morph->no_cast = fread_number( fp );
		}
		break;
	case 'O':
		if ( !str_cmp( word, "Objs" ) )
		{
		   fMatch = TRUE;
		   morph->obj[0] = fread_number( fp );
		   morph->obj[1] = fread_number( fp );
		   morph->obj[2] = fread_number( fp );
		}
		if ( !str_cmp( word, "Objuse" ) )
		{
		   fMatch = TRUE;
		   morph->objuse[0] = fread_number( fp );
		   morph->objuse[1] = fread_number( fp );
		   morph->objuse[2] = fread_number( fp );
		}
		break;
	case 'P':
		KEY( "Parry", morph->parry, fread_number( fp ) );
		KEY( "Pkill", morph->pkill, fread_number( fp ) );
		break;
	case 'R':
                if ( !str_cmp ( word, "Race" ) )
                {
                  fMatch = TRUE;
                  arg = fread_string_nohash ( fp );
                  arg = one_argument( arg, temp );
                  while ( temp[0] != '\0' )
                  {
                        for ( i=0; i < MAX_PC_RACE; i++ )
                          if ( !str_cmp( temp, race_table[i]->race_name ) )
			  {
                                SET_BIT( morph->race, (1 << i) );
				break;
			  }
                        arg = one_argument( arg, temp );
                  }
                } 
		KEY( "Resistant", morph->resistant, fread_number( fp ) );
		break;
	case 'S':
		KEY("SaveBreath", morph->saving_breath, fread_number( fp ) );
		KEY("SavePara", morph->saving_para_petri, fread_number( fp ) );
		KEY("SavePoison", morph->saving_poison_death,fread_number(fp));
		KEY("SaveSpell", morph->saving_spell_staff, fread_number(fp));
		KEY("SaveWand", morph->saving_wand, fread_number(fp));
		KEY("Sex", morph->sex, fread_number( fp ) );
		KEY("ShortDesc",morph->short_desc, fread_string_nohash(fp));
		KEY("Skills", morph->skills, fread_string_nohash(fp));
		KEY("Strength", morph->str, fread_number( fp ) );
		KEY("Suscept", morph->suscept, fread_number( fp ) );
		break;
	case 'T':
		KEY("TimeFrom", morph->timefrom, fread_number( fp ) );
		KEY("TimeTo", morph->timeto, fread_number( fp ) );
		KEY("Tumble", morph->tumble, fread_number( fp ) );
		break;
	case 'U':
		KEY("UnmorphOther",morph->unmorph_other,fread_string_nohash(fp));
		KEY("UnmorphSelf",morph->unmorph_self,fread_string_nohash(fp));
		KEY("Used", morph->used, fread_number( fp ) );
		break;
        case 'V':
		KEY("Vnum", morph->vnum, fread_number( fp ) );
		break;
	case 'W':
		KEY("Wisdom", morph->wis, fread_number( fp ) );
		break;
      }
      if (!fMatch)
      {
          sprintf (buf, "Fread_morph: no match: %s", word);
          bug (buf, 0);
	  /* Bailing out on this morph as something may be messed up 
	   * this is going to have lots of bugs from the load_morphs this
           * way, but it is better than possibly having the memory messed
           * up! --Shaddai
           */
	  free_morph( morph );
	  return NULL;
      }
  }
}

/* 
 * Function that releases all the memory for a morph struct.  Carefull not
 * to use the memory afterwards as it doesn't exist.
 * --Shaddai 
 */
void
free_morph ( MORPH_DATA *morph )
{
   if ( !morph )
	return;
   if ( morph->blood )
	DISPOSE ( morph->blood );
   if ( morph->damroll )
	DISPOSE ( morph->damroll );
   if ( morph->description )
	DISPOSE ( morph->description );
   if ( morph->help )
	DISPOSE ( morph->help );
   if ( morph->hit )
	DISPOSE ( morph->hit );
   if ( morph->hitroll )
	DISPOSE ( morph->hitroll );
   if ( morph->key_words )
	DISPOSE ( morph->key_words );
   if ( morph->long_desc )
	DISPOSE ( morph->long_desc );
   if ( morph->mana )
	DISPOSE ( morph->mana );
   if ( morph->morph_other )
	DISPOSE ( morph->morph_other );
   if ( morph->morph_self )
	DISPOSE ( morph->morph_self );
   if ( morph->move )
	DISPOSE ( morph->move );
   if ( morph->name )
	DISPOSE ( morph->name );
   if ( morph->short_desc )
	DISPOSE ( morph->short_desc );
   if ( morph->skills )
	DISPOSE ( morph->skills );
   if ( morph->no_skills )
	DISPOSE ( morph->no_skills );
   if ( morph->unmorph_other )
	DISPOSE ( morph->unmorph_other );
   if ( morph->unmorph_self )
	DISPOSE ( morph->unmorph_self );
   DISPOSE ( morph );
   return;
}

/*
 * This function set's up all the default values for a morph
 */
void
morph_defaults ( MORPH_DATA *morph )
{
	morph->blood 			= str_dup ( "" );
	morph->damroll 			= str_dup ( "" );
	morph->description 		= str_dup ( "" );
	morph->help 			= str_dup ( "" );
	morph->hit 			= str_dup ( "" );
	morph->hitroll 			= str_dup ( "" );
	morph->key_words 		= str_dup ( "" );
	morph->long_desc 		= str_dup ( "" );
	morph->mana 			= str_dup ( "" );
	morph->morph_other 		= str_dup ( "" );
	morph->morph_self 		= str_dup ( "" );
	morph->move 			= str_dup ( "" );
	morph->name 			= str_dup ( "" );
	morph->short_desc    		= str_dup ( "" );
	morph->skills       		= str_dup ( "" );
        morph->no_skills                = str_dup ( "" );
	morph->unmorph_other 		= str_dup ( "" );
	morph->unmorph_self  		= str_dup ( "" );
	xCLEAR_BITS(morph->affected_by);
	morph->class    		= 0;
	morph->sex			= -1;
	morph->timefrom			= -1;
	morph->timeto			= -1;
	morph->dayfrom			= -1;
	morph->dayto			= -1;
	morph->pkill			= 0;
	morph->bloodused		= 0;
	morph->manaused			= 0;
	morph->moveused			= 0;
	morph->hpused			= 0;
	morph->favourused		= 0;
	morph->gloryused		= 0;
	morph->immune    		= 0;
	xCLEAR_BITS(morph->no_affected_by);
	morph->no_immune    		= 0;
	morph->no_resistant    		= 0;
	morph->no_suscept    		= 0;
	morph->obj[0]    		= 0;
	morph->obj[1]    		= 0;
	morph->obj[2]    		= 0;
	morph->objuse[0]		= FALSE;
	morph->objuse[1]		= FALSE;
	morph->objuse[2]		= FALSE;
	morph->race    			= 0;
	morph->resistant	    	= 0;
	morph->suscept 		   	= 0;
	morph->used    		        = 0;
	morph->ac    			= 0;
        morph->defpos                   = POS_STANDING;
	morph->dex    			= 0;
	morph->dodge   		 	= 0;
	morph->cha    			= 0;
	morph->con    			= 0;
	morph->inte    			= 0;
	morph->lck   		 	= 0;
	morph->level    		= 0;
	morph->parry    		= 0;
	morph->saving_breath    	= 0;
	morph->saving_para_petri    	= 0;
	morph->saving_poison_death  	= 0;
	morph->saving_spell_staff    	= 0;
	morph->saving_wand    		= 0;
	morph->str    			= 0;
	morph->tumble    		= 0;
	morph->wis    			= 0;
	morph->no_cast    		= FALSE;
	morph->timer			= -1;
        morph->vnum                     = 0;
	return;
}

/*
 * This function copys one morph structure to another
 */
void
copy_morph ( MORPH_DATA *morph, MORPH_DATA *temp )
{
	morph->blood 			= str_dup ( temp->blood );
	morph->damroll 			= str_dup ( temp->damroll );
	morph->description 		= str_dup ( temp->description );
	morph->help 			= str_dup ( temp->help );
	morph->hit 			= str_dup ( temp->hit );
	morph->hitroll 			= str_dup ( temp->hitroll );
	morph->key_words 		= str_dup ( temp->key_words );
	morph->long_desc 		= str_dup ( temp->long_desc );
	morph->mana 			= str_dup ( temp->mana );
	morph->morph_other 		= str_dup ( temp->morph_other );
	morph->morph_self 		= str_dup ( temp->morph_self );
	morph->move 			= str_dup ( temp->move );
	morph->name 			= str_dup ( temp->name );
	morph->short_desc    		= str_dup ( temp->short_desc );
	morph->skills       		= str_dup ( temp->skills );
        morph->no_skills                = str_dup ( temp->no_skills );
	morph->unmorph_other 		= str_dup ( temp->unmorph_other );
	morph->unmorph_self  		= str_dup ( temp->unmorph_self );
	morph->affected_by    		= temp->affected_by;
	morph->class    		= temp->class;
	morph->sex			= temp->sex;
	morph->timefrom			= temp->timefrom;
	morph->timeto			= temp->timeto;
	morph->timefrom			= temp->timefrom;
	morph->dayfrom			= temp->dayfrom;
	morph->dayto			= temp->dayto;
	morph->pkill			= temp->pkill;
	morph->manaused			= temp->manaused;
	morph->bloodused		= temp->bloodused;
	morph->moveused			= temp->moveused;
	morph->hpused			= temp->hpused;
	morph->favourused		= temp->favourused;
	morph->gloryused		= temp->gloryused;
	morph->immune    		= temp->immune;
	morph->no_affected_by  		= temp->no_affected_by;
	morph->no_immune    		= temp->no_immune;
	morph->no_resistant    		= temp->no_resistant;
	morph->no_suscept    		= temp->no_suscept;
	morph->obj[0]    		= temp->obj[0];
	morph->obj[1]    		= temp->obj[1];
	morph->obj[2]    		= temp->obj[2];
	morph->objuse[0]		= temp->objuse[0];
	morph->objuse[1]		= temp->objuse[1];
	morph->objuse[2]		= temp->objuse[2];
	morph->race    			= temp->race;
	morph->resistant	    	= temp->resistant;
	morph->suscept 		   	= temp->suscept;
	morph->ac    			= temp->ac;
        morph->defpos                   = temp->defpos;
	morph->dex    			= temp->dex;
	morph->dodge   		 	= temp->dodge;
	morph->cha    			= temp->cha;
	morph->con    			= temp->con;
	morph->inte    			= temp->inte;
	morph->lck   		 	= temp->lck;
	morph->level    		= temp->level;
	morph->parry    		= temp->parry;
	morph->saving_breath    	= temp->saving_breath;
	morph->saving_para_petri    	= temp->saving_para_petri;
	morph->saving_poison_death  	= temp->saving_poison_death;
	morph->saving_spell_staff    	= temp->saving_spell_staff;
	morph->saving_wand    		= temp->saving_wand;
	morph->str    			= temp->str;
	morph->tumble    		= temp->tumble;
	morph->wis    			= temp->wis;
	morph->no_cast    		= temp->no_cast;
	morph->timer			= temp->timer;
	return;
}

/*
 * This is to move the more used morphs to the beginning hopefully saving
 * that .000000001 second ;) --Shaddai
 */

void 
sort_morphs ( )
{
  MORPH_DATA *morph, *temp;
  int fMatch;

  /* This is slow and the wrong way to code it but I am tired and have more
   * Important things to-do :)  Also it is only done at boot-up so who cares
   * if it is slow or not. --Shaddai
   */

  do {
        fMatch = FALSE;
        if ( morph_start == NULL )
		return;
	for ( morph = morph_start, temp = morph->next;morph && temp; 
			morph = morph->next, temp = temp->next)
        {
	  if ( morph->used < temp->used )
          {
		if ( morph == morph_start )
			morph_start = temp;
		if ( temp == morph_end )
			morph_end = morph;
		if ( morph->prev )
			morph->prev->next = temp;
		if ( temp->next )
			temp->next->prev = morph;
		temp->prev = morph->prev;
		morph->next = temp->next;
		temp->next = morph;
		morph->prev = temp;
		fMatch = TRUE;
		break;
	  }
        }
  } while (fMatch);
  return;
}

/*
 * Create new player morph, a scailed down version of original morph
 * so if morph gets changed stats don't get messed up.
 */
CHAR_MORPH *
make_char_morph ( MORPH_DATA *morph )
{
  CHAR_MORPH *ch_morph;

  if ( morph == NULL )
	return NULL;

  CREATE ( ch_morph, CHAR_MORPH, 1);
  ch_morph->morph		=       morph;
  ch_morph->ac 			=	morph->ac;
  ch_morph->str			=	morph->str;
  ch_morph->inte		=	morph->inte;
  ch_morph->wis			=	morph->wis;
  ch_morph->dex			=	morph->dex;
  ch_morph->cha			=	morph->cha;
  ch_morph->lck			=	morph->lck;
  ch_morph->saving_breath	=	morph->saving_breath;
  ch_morph->saving_para_petri	=	morph->saving_para_petri;
  ch_morph->saving_poison_death	=	morph->saving_poison_death;
  ch_morph->saving_spell_staff	=	morph->saving_spell_staff;
  ch_morph->saving_wand		=	morph->saving_wand;
  ch_morph->timer               =       morph->timer;
  ch_morph->hitroll		=	0;
  ch_morph->damroll		=	0;
  ch_morph->hit			=	0;
  ch_morph->mana		=	0;
  ch_morph->move		=	0;
  ch_morph->blood		=	0;
  ch_morph->affected_by		=	morph->affected_by;
  ch_morph->immune   		=	morph->immune;
  ch_morph->resistant		=	morph->resistant;
  ch_morph->suscept		=	morph->suscept;
  ch_morph->no_affected_by	=	morph->no_affected_by;
  ch_morph->no_immune   	=	morph->no_immune;
  ch_morph->no_resistant	=	morph->no_resistant;
  ch_morph->no_suscept		=	morph->no_suscept;
  return ch_morph;
}

/*
 * Player command to create a new morph
 */
void
do_morphcreate ( CHAR_DATA *ch, char *argument )
{
  MORPH_DATA *morph;
  MORPH_DATA *temp = NULL;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  
  if ( arg1[0] == '\0' )
  {
	send_to_char( "Usage: morphcreate <name>\n\r", ch);
        send_to_char( "Usage: morphcreate <name/vnum> copy\n\r", ch );
	return;
  }

  if ( arg2[0] != '\0' && !str_cmp( arg2, "copy" ) )
  {
	if ( is_number( arg1 ) )
	{
	  if ( (temp = get_morph_vnum( atoi( arg1 ) ) ) == NULL )
	  {
	    ch_printf(ch, "No such morph vnum %d exists.\n\r", atoi(arg1) );
	    return;
	  }	
	}
	else if ( (temp = get_morph( arg1 ) ) == NULL )
	{
	  ch_printf(ch, "No such morph %s exists.\n\r", arg1 );
	  return;
	}
  }
  smash_tilde( arg1 );
  CREATE( morph, MORPH_DATA, 1);
  morph_defaults ( morph );
  if ( morph->name )
	DISPOSE( morph->name );
  if ( arg2[0] != '\0' && !str_cmp( arg2, "copy" ) && temp )
	copy_morph ( morph, temp );		
  else
  	morph->name = str_dup( arg1 );
  morph->vnum = morph_vnum;
  morph_vnum++;
  LINK (morph, morph_start, morph_end, next, prev);
  ch_printf (ch, "Morph %s created with vnum %d.\n\r",morph->name,morph->vnum);
  return;
}

/*
 * Player function to delete a morph. --Shaddai
 * NOTE Need to check all players and force them to unmorph first
 */

void
do_morphdestroy ( CHAR_DATA *ch, char *argument )
{
  MORPH_DATA *morph;
  char arg[MAX_INPUT_LENGTH];

  one_argument( argument, arg );

  if ( is_number( arg ) )
  	morph = get_morph_vnum ( atoi(arg) );
  else 
  	morph = get_morph ( arg );

  if ( !morph )
  {
	ch_printf(ch, "Unkown morph %s.\n\r", arg );
	return;
  }
  unmorph_all ( morph );
  UNLINK( morph, morph_start, morph_end, next, prev );
  free_morph( morph );
  send_to_char( "Morph deleted.\n\r", ch );
  return;
}

void 
free_char_morph ( CHAR_MORPH *morph )
{
  /* Boy this is easy but better to keep it a seperate function for later
   * possible complications.  --Shaddai
   */
  DISPOSE ( morph );
  return;
}

void
fwrite_morph_data ( CHAR_DATA *ch, FILE *fp )
{
  CHAR_MORPH *morph;
  if ( ch->morph == NULL )
	return;
  morph = ch->morph;
  fprintf ( fp, "#MorphData\n" );
  /* Only Print Out what is necessary */
  if ( morph->morph != NULL )
  {
	fprintf ( fp, "Vnum %d\n", morph->morph->vnum );
	fprintf ( fp, "Name %s~\n", morph->morph->name );
  }
  if ( !xIS_EMPTY(morph->affected_by) )
        fprintf ( fp, "Affect          %s\n", print_bitvector(&morph->affected_by) );
  if ( morph->immune != 0 )
        fprintf ( fp, "Immune          %d\n", morph->immune );
  if ( morph->resistant != 0 )
        fprintf ( fp, "Resistant       %d\n", morph->resistant );
  if ( morph->suscept != 0 )
        fprintf ( fp, "Suscept         %d\n", morph->suscept );
  if ( !xIS_EMPTY(morph->no_affected_by) )
        fprintf ( fp, "NoAffect        %s\n", print_bitvector(&morph->no_affected_by) );
  if ( morph->no_immune != 0 )
        fprintf ( fp, "NoImmune        %d\n", morph->no_immune );
  if ( morph->no_resistant != 0 )
        fprintf ( fp, "NoResistant     %d\n", morph->no_resistant );
  if ( morph->no_suscept != 0 )
        fprintf ( fp, "NoSuscept       %d\n", morph->no_suscept );
  if ( morph->ac != 0 )
        fprintf ( fp, "Armor           %d\n", morph->ac );
  if ( morph->blood != 0 )
        fprintf ( fp, "Blood           %d\n", morph->blood );
  if ( morph->cha != 0 )
        fprintf ( fp, "Charisma        %d\n", morph->cha );
  if ( morph->con != 0 )
        fprintf ( fp, "Constitution    %d\n", morph->con );
  if ( morph->damroll != 0 )
        fprintf ( fp, "Damroll	 %d\n", morph->damroll );
  if ( morph->dex != 0 )
        fprintf ( fp, "Dexterity       %d\n", morph->dex );
  if ( morph->dodge != 0 )
        fprintf ( fp, "Dodge           %d\n", morph->dodge );
  if ( morph->hit != 0 )
        fprintf ( fp, "Hit             %d\n", morph->hit );
  if ( morph->hitroll != 0 )
        fprintf ( fp, "Hitroll         %d\n", morph->hitroll );
  if ( morph->inte != 0 )
        fprintf ( fp, "Intelligence    %d\n", morph->inte );
  if ( morph->lck != 0 )
        fprintf ( fp, "Luck            %d\n", morph->lck );
  if ( morph->mana != 0 )
        fprintf ( fp, "Mana            %d\n", morph->mana );
  if ( morph->move != 0 )
	fprintf ( fp, "Move            %d\n", morph->move );
  if ( morph->parry != 0 )
        fprintf ( fp, "Parry           %d\n", morph->parry );
  if ( morph->saving_breath != 0 )
        fprintf ( fp, "Save1           %d\n", morph->saving_breath );
  if ( morph->saving_para_petri != 0 )
        fprintf ( fp, "Save2           %d\n", morph->saving_para_petri );
  if ( morph->saving_poison_death != 0 )
        fprintf ( fp, "Save3           %d\n", morph->saving_poison_death );
  if ( morph->saving_spell_staff != 0 )
        fprintf ( fp, "Save4           %d\n", morph->saving_spell_staff );
  if ( morph->saving_wand != 0 )
        fprintf ( fp, "Save5           %d\n", morph->saving_wand );
  if ( morph->str != 0 )
        fprintf ( fp, "Strength        %d\n", morph->str );
  if ( morph->timer != -1 )
	fprintf ( fp, "Timer	       %d\n", morph->timer );
  if ( morph->tumble != 0 )
        fprintf ( fp, "Tumble          %d\n", morph->tumble );
  if ( morph->wis != 0 )
        fprintf ( fp, "Wisdom          %d\n", morph->wis );
  fprintf ( fp, "End\n" );
  return;
}

void
fread_morph_data ( CHAR_DATA *ch, FILE *fp )
{
  CHAR_MORPH *morph;
  bool fMatch;
  char *word;
  char buf[MAX_STRING_LENGTH];

  CREATE ( morph, CHAR_MORPH, 1);
  clear_char_morph ( morph );
  ch->morph = morph;
  for ( ; ; )
  {
    word = feof ( fp ) ? "End" : fread_word (fp);
    fMatch = FALSE;

    switch ( UPPER ( word[0] ) )
    {
	case 'A':
		KEY ( "Affect", morph->affected_by, fread_bitvector( fp ) );
		KEY ( "Armor", morph->ac, fread_number( fp ) );
		break;
	case 'B':
		KEY ( "Blood", morph->blood, fread_number( fp ) );
		break;
	case 'C':
		KEY ( "Charisma", morph->cha, fread_number( fp ) );
		KEY ( "Constitution", morph->con, fread_number( fp ) );
		break;
	case 'D':
		KEY ( "Damroll", morph->damroll, fread_number( fp ) );
		KEY ( "Dexterity", morph->dex, fread_number( fp ) );
		KEY ( "Dodge", morph->dodge, fread_number( fp ) );
		break;
	case 'E':
		if ( !str_cmp ( "End", word ) )
		  return;
		break;
	case 'H':
		KEY ( "Hit", morph->hit, fread_number( fp ) );
		KEY ( "Hitroll", morph->hitroll, fread_number( fp ) );
		break;
	case 'I':
		KEY ( "Immune", morph->immune, fread_number( fp ) );
		KEY ( "Intelligence", morph->inte, fread_number( fp ) );
		break;
	case 'L':
		KEY ( "Luck", morph->lck, fread_number( fp ) );
		break;
	case 'M':
		KEY ( "Mana", morph->mana, fread_number( fp ) );
		KEY ( "Move", morph->move, fread_number( fp ) );
		break;
	case 'N':
		if ( !str_cmp ( "Name", word ) )
		{
		   fMatch = TRUE;
		   if ( morph->morph )
			if ( str_cmp ( morph->morph->name, fread_string( fp )))
				bug ( "Morph Name doesn't match vnum %d.",
					morph->morph->vnum );
		}
		KEY ( "NoAffect", morph->no_affected_by, fread_bitvector( fp ) );
		KEY ( "NoImmune", morph->no_immune, fread_number( fp ) );
		KEY ( "NoResistant", morph->no_resistant, fread_number( fp ) );
		KEY ( "NoSuscept", morph->no_suscept, fread_number( fp ) );
		break;
	case 'P':
		KEY ( "Parry", morph->parry, fread_number( fp ) );
		break;
	case 'R':
		KEY ( "Resistant", morph->resistant, fread_number( fp ) );
		break;
	case 'S':
		KEY ( "Save1", morph->saving_breath, fread_number( fp ) );
		KEY ( "Save2", morph->saving_para_petri, fread_number( fp ) );
		KEY ( "Save3", morph->saving_poison_death, fread_number( fp ) );
		KEY ( "Save4", morph->saving_spell_staff, fread_number( fp ) );
		KEY ( "Save5", morph->saving_wand, fread_number( fp ) );
		KEY ( "Strength", morph->str, fread_number( fp ) );
		KEY ( "Suscept", morph->suscept, fread_number( fp ) );
		break;
	case 'T':
		KEY ( "Timer", morph->timer, fread_number( fp ) );
		KEY ( "Tumble", morph->tumble, fread_number( fp ) );
		break;
        case 'V':
		if ( !str_cmp ( "Vnum", word ) )
		{
		   fMatch = TRUE;
		   morph->morph = get_morph_vnum ( fread_number( fp ) ); 
		}
		break;
	case 'W':
		KEY ( "Wisdom", morph->wis, fread_number( fp ) );
		break;
    }
      if (!fMatch)
      {
          sprintf (buf, "Fread_morph_data: no match: %s", word);
          bug (buf, 0);
      }
  }
  return;
}

void
clear_char_morph ( CHAR_MORPH *morph )
{
  morph->timer			=	-1;
  xCLEAR_BITS(morph->affected_by);
  xCLEAR_BITS(morph->no_affected_by);
  morph->immune			= 	0;
  morph->no_immune		= 	0;
  morph->no_resistant		= 	0;
  morph->no_suscept		= 	0;
  morph->resistant		= 	0;
  morph->suscept		= 	0;
  morph->ac			= 	0;
  morph->blood			= 	0;
  morph->cha			= 	0;
  morph->con			= 	0;
  morph->damroll		= 	0;
  morph->dex			= 	0;
  morph->dodge			= 	0;
  morph->hit			= 	0;
  morph->hitroll		= 	0;
  morph->inte			= 	0;
  morph->lck			= 	0;
  morph->mana			= 	0;
  morph->parry			= 	0;
  morph->saving_breath		= 	0;
  morph->saving_para_petri	= 	0;
  morph->saving_poison_death	= 	0;
  morph->saving_spell_staff	= 	0;
  morph->saving_wand		= 	0;
  morph->str			= 	0;
  morph->tumble			= 	0;
  morph->wis			= 	0;
  morph->morph			=	NULL;
  return;
}

void
setup_morph_vnum( void )
{
  MORPH_DATA *morph;
  int        vnum;
  vnum = morph_vnum;

  for ( morph = morph_start; morph; morph = morph->next ) 
	if ( morph->vnum > vnum )
		vnum = morph->vnum;
  if ( vnum < 1000 )
	vnum = 1000;
  else
  	vnum++;

  for ( morph = morph_start; morph; morph = morph->next ) 
	if ( morph->vnum == 0 )
	{
		morph->vnum = vnum;
		vnum++;
	}
  morph_vnum = vnum;
  return;
}

void
unmorph_all ( MORPH_DATA *morph )
{
  CHAR_DATA *vch;

  for (vch = first_char; vch; vch = vch->next )
  {
	if ( IS_NPC(vch) )
		continue;
	if ( vch->morph == NULL || vch->morph->morph == NULL || 
	     vch->morph->morph != morph )
		continue;
	do_unmorph_char ( vch );
  }
  return;
}

/* 
 * Following functions are for immortal testing purposes.
 */

void 
do_imm_morph ( CHAR_DATA *ch, char *argument )
{
   MORPH_DATA *morph;
   CHAR_DATA  *victim = NULL;
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int	vnum;
   
   if ( IS_NPC(ch) )
   {
      send_to_char("Only player characters can use this command.\n\r", ch );
      return;
   }

   argument = one_argument( argument, arg );
   one_argument( argument, arg2 );

   if ( !is_number(arg) )
   {
      send_to_char("Syntax: morph <vnum>\n\r", ch );
      return;
   }
   vnum = atoi( arg );
   morph = get_morph_vnum ( vnum );

   if ( morph == NULL )
   {
      ch_printf(ch, "No such morph %d exists.\n\r", vnum );
      return;
   }
   if ( arg2[0] == '\0' )
   	do_morph_char ( ch, morph );
   else if ( ( victim = get_char_world ( ch, arg2 ) ) == NULL )
   {
          send_to_char ("No one like that in all the realms.\n\r", ch);
          return;
   }
   if ( victim!= NULL && get_trust (ch) < get_trust (victim) 
		&& !IS_NPC (victim))
   {
      send_to_char ("You can't do that!\n\r", ch);
      return;
   }
   else if ( victim != NULL )
	do_morph_char ( victim, morph );
   send_to_char("Done.\n\r", ch );
   return;
}

/*
 * This is just a wrapper.  --Shaddai
 */

void 
do_imm_unmorph( CHAR_DATA *ch , char *argument)
{
   CHAR_DATA *victim = NULL;
   char arg[MAX_INPUT_LENGTH];
 
   one_argument( argument, arg );
   if ( arg[0] == '\0' )
   	do_unmorph_char ( ch );
   else if ( ( victim = get_char_world ( ch, arg ) ) == NULL )
   {
          send_to_char ("No one like that in all the realms.\n\r", ch);
          return;
   }
   if ( victim!= NULL && get_trust (ch) < get_trust (victim) 
                && !IS_NPC (victim))
   {
      send_to_char ("You can't do that!\n\r", ch);
      return;
   }
   else if ( victim != NULL )
        do_unmorph_char ( victim );
   send_to_char("Done.\n\r", ch );
   return;
}
