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
 * ------------------------------------------------------------------------ *
 *			 Command interpretation module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#ifdef USE_IMC
#include "icec-mercbase.h"
#endif


/*
 * Externals
 */
void refresh_page( CHAR_DATA *ch );
void subtract_times( struct timeval *etime, struct timeval *stime );

bool	check_social	args( ( CHAR_DATA *ch, char *command,
			    char *argument ) );
char *check_cmd_flags args( ( CHAR_DATA *ch, CMDTYPE *cmd ) );



/*
 * Log-all switch.
 */
bool				fLogAll		= FALSE;


CMDTYPE	   *command_hash[126];	/* hash table for cmd_table */
SOCIALTYPE *social_index[27];   /* hash table for socials   */

/*
 * Character not in position for command?
 */
bool check_pos( CHAR_DATA *ch, sh_int position )
{

    if ( IS_NPC( ch ) && ch->position > 3 ) /*Band-aid alert?  -- Blod*/
      return TRUE;

    if ( ch->position < position )
    {
	switch( ch->position )
	{
	case POS_DEAD:
	    send_to_char( "A little difficult to do when you are DEAD...\n\r", ch );
	    break;

	case POS_MORTAL:
	case POS_INCAP:
	    send_to_char( "You are hurt far too bad for that.\n\r", ch );
	    break;

	case POS_STUNNED:
	    send_to_char( "You are too stunned to do that.\n\r", ch );
	    break;

	case POS_SLEEPING:
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    break;

	case POS_RESTING:
	    send_to_char( "Nah... You feel too relaxed...\n\r", ch);
	    break;

	case POS_SITTING:
	    send_to_char( "You can't do that sitting down.\n\r", ch);
	    break;

	case POS_FIGHTING:
            if(position<=POS_EVASIVE){
	      send_to_char( "This fighting style is too demanding for that!\n\r", ch);
            } else {
	      send_to_char( "No way!  You are still fighting!\n\r", ch);
             }
	    break;
    	case POS_DEFENSIVE:
            if(position<=POS_EVASIVE){
	      send_to_char( "This fighting style is too demanding for that!\n\r", ch);
            } else {
	    send_to_char( "No way!  You are still fighting!\n\r", ch);
            }
	    break;
    	case POS_AGGRESSIVE:
            if(position<=POS_EVASIVE){
	      send_to_char( "This fighting style is too demanding for that!\n\r", ch);
            } else {
	    send_to_char( "No way!  You are still fighting!\n\r", ch);
            }
	    break;
    	case POS_BERSERK:
            if(position<=POS_EVASIVE){
	      send_to_char( "This fighting style is too demanding for that!\n\r", ch);
            } else {
	       send_to_char( "No way!  You are still fighting!\n\r", ch);
            }
	    break;
    	case POS_EVASIVE:
	    send_to_char( "No way!  You are still fighting!\n\r", ch);
	    break;

	}
	return FALSE;
    }
    return TRUE;
}

extern char lastplayercmd[MAX_INPUT_LENGTH*2];


/*
 * Determine if this input line is eligible for writing to a watch file.
 * We don't want to write movement commands like (n, s, e, w, etc.)
 */
bool valid_watch( char *logline )
{
int  len = strlen(logline);
char c   = logline[0];

if ( len==1 && (c=='l' || c=='n' || c=='s' || c=='e' || c=='w' || c=='u' || c=='d') )
   return FALSE;
if ( len==2 && c=='n' && (logline[1]=='e' || logline[1]=='w') )
   return FALSE;
if ( len==2 && c=='s' && (logline[1]=='e' || logline[1]=='w') )
   return FALSE;

return TRUE;
}


/*
 * Write input line to watch files if applicable
 */
void write_watch_files( CHAR_DATA *ch, CMDTYPE *cmd, char *logline )
{
WATCH_DATA *pw;
FILE *fp;
char fname[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
struct tm *t = localtime(&current_time);

if (!first_watch)         /* no active watches */
   return;

/* if we're watching a command we need to do some special stuff */
/* to avoid duplicating log lines - relies upon watch list being */
/* sorted by imm name */
if(cmd)
{
	char *cur_imm;
	bool found;
	
	pw = first_watch;
	while(pw)
	{
		found = FALSE;
		
		for(cur_imm = pw->imm_name;
			pw && !strcmp(pw->imm_name, cur_imm); pw = pw->next)
		{

                        if(!found && ch->desc && get_trust(ch) < pw->imm_level 
                        &&((pw->target_name&&!strcmp(cmd->name,pw->target_name))
                        || (pw->player_site && 
                        !str_prefix(pw->player_site, ch->desc->host))))
			{
				sprintf( fname, "%s%s", WATCH_DIR, strlower( pw->imm_name ) );
				if ( !(fp = fopen(fname, "a+")) )
   				{
       	   				sprintf( buf, "%s%s", "Write_watch_files: Cannot open ", fname);
          				bug( buf, 0 );
          				perror(fname);
          				return;
       				}
       				sprintf(buf, "%.2d/%.2d %.2d:%.2d %s: %s\n\r",
               				t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min,
               				ch->name, logline );
       				fputs(buf, fp);
       				fclose(fp);
				found = TRUE;
			}
		}
	}
}
else
{
	for ( pw = first_watch; pw; pw = pw->next )
                if (((pw->target_name && !str_cmp   (pw->target_name, ch->name))
                ||    (pw->player_site 
                && !str_prefix(pw->player_site, ch->desc->host)) )
                &&    get_trust(ch) < pw->imm_level
                &&    ch->desc )
    		{
			sprintf( fname, "%s%s", WATCH_DIR, strlower( pw->imm_name ) );
			if ( !(fp = fopen(fname, "a+")) )
   			{
       	   			sprintf( buf, "%s%s", "Write_watch_files: Cannot open ", fname);
          			bug( buf, 0 );
          			perror(fname);
          			return;
       			}
       			sprintf(buf, "%.2d/%.2d %.2d:%.2d %s: %s\n\r",
               			t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min,
               			ch->name, logline );
       			fputs(buf, fp);
       			fclose(fp);
    		}
}

return;
}


/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char logline[MAX_INPUT_LENGTH];
    char logname[MAX_INPUT_LENGTH];
    char newcommand[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *buf;
	char *origarg = argument;
    TIMER *timer = NULL;
    CMDTYPE *cmd = NULL;
    int trust;
    int loglvl;
    bool found;
    struct timeval time_used;
    long tmptime;


    if ( !ch )
    {
	bug( "interpret: null ch!", 0 );
	return;
    }

    if ( !ch->in_room )
    {
    	bug( "interpret: null in_room!", 0 );
	return;
    }
    found = FALSE;
    if ( ch->substate == SUB_REPEATCMD )
    {
	DO_FUN *fun;

	if ( (fun=ch->last_cmd) == NULL )
	{
	    ch->substate = SUB_NONE;
	    bug( "interpret: SUB_REPEATCMD with NULL last_cmd", 0 );
	    return;
	}
	else
	{
	    int x;

	    /*
	     * yes... we lose out on the hashing speediness here...
	     * but the only REPEATCMDS are wizcommands (currently)
	     */
	    for ( x = 0; x < 126; x++ )
	    {
		for ( cmd = command_hash[x]; cmd; cmd = cmd->next )
		   if ( cmd->do_fun == fun )
		   {
			found = TRUE;
			break;
		   }
		if ( found )
		   break;
	    }
	    if ( !found )
	    {
		cmd = NULL;
		bug( "interpret: SUB_REPEATCMD: last_cmd invalid", 0 );
		return;
	    }
	    sprintf( logline, "(%s) %s", cmd->name, argument );
	}
    }

    if ( !cmd )
    {
	/* Changed the order of these ifchecks to prevent crashing. */
	if ( !argument || !strcmp(argument,"") ) 
	{
	    bug( "interpret: null argument!", 0 );
	    return;
	}

	/*
	 * Strip leading spaces.
	 */
	while ( isspace(*argument) )
	    argument++;
	if ( argument[0] == '\0' )
	    return;

	/* xREMOVE_BIT( ch->affected_by, AFF_HIDE ); */

	/*
	 * Implement freeze command.
	 */
	if ( !IS_NPC(ch) && xIS_SET(ch->act, PLR_FREEZE) )
	{
	    send_to_char( "You're totally frozen!\n\r", ch );
	    return;
	}

	/*
	 * Grab the command word.
	 * Special parsing so ' can be a command,
	 *   also no spaces needed after punctuation.
	 */
	strcpy( logline, argument );
	if ( !isalpha(argument[0]) && !isdigit(argument[0]) )
	{
	    command[0] = argument[0];
	    command[1] = '\0';
	    argument++;
	    while ( isspace(*argument) )
		argument++;
	}
	else
	    argument = one_argument( argument, command );

	/*
	 * Look for command in command table.
	 * Check for council powers and/or bestowments
	 */
	trust = get_trust( ch );
	for ( cmd = command_hash[LOWER(command[0])%126]; cmd; cmd = cmd->next )
	    if ( !str_prefix( command, cmd->name )
	    &&   (cmd->level <= trust
	    ||  (!IS_NPC(ch) && ch->pcdata->council
	    &&    is_name( cmd->name, ch->pcdata->council->powers )
	    &&    cmd->level <= (trust+MAX_CPD))
	    ||  ( !IS_NPC(ch) && IS_SET(ch->pcdata->flags, PCFLAG_RETIRED)
	    && IS_SET(cmd->flags, CMD_FLAG_RETIRED))
	    ||  (!IS_NPC(ch) && ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0'
	    &&    is_name( cmd->name, ch->pcdata->bestowments )
	    &&    cmd->level <= (trust+sysdata.bestow_dif)) ) )
	    {
		found = TRUE;
		break;
	    }

	/*
	 * Turn off afk bit when any command performed.
	 */
	if ( !IS_NPC( ch ) && xIS_SET ( ch->act, PLR_AFK)  && (str_cmp(command, "AFK")))
	{
	    xREMOVE_BIT( ch->act, PLR_AFK );
/*
     	    act( AT_GREY, "$n is no longer afk.", ch, NULL, NULL, TO_ROOM );
*/
     	    act( AT_GREY, "$n is no longer afk.", ch, NULL, NULL, TO_CANSEE );
	}
    }

    /*
     * Log and snoop.
     */
/*
    sprintf( lastplayercmd, "** %s: %s", ch->name, logline );
*/
    sprintf( lastplayercmd, "%s used %s", ch->name, logline );


    if ( found && cmd->log == LOG_NEVER )
	strcpy( logline, "XXXXXXXX XXXXXXXX XXXXXXXX" );

    loglvl = found ? cmd->log : LOG_NORMAL;

/* Cause people said it was spammy, even though they asked for it in the
first place.  Whaddya gonna do? */
/* Uncommented and checked for the same things as valid_watch to reduce
   the spammage caused from moovement and looking */
    if ( IS_NPC(ch) && IS_AFFECTED(ch, AFF_POSSESS) 
		&& valid_watch( logline ) ) 
		{
        char log_buf[MAX_STRING_LENGTH];
        if ( ch->desc && ch->desc->original && ch->short_descr ) {
           sprintf( log_buf, "%s possessed %s:  %s.", ch->desc->original->name,
		ch->short_descr, logline );
           log_string_plus( log_buf, LOG_NORMAL, 58 );
        }
   }


    /*
     * Write input line to watch files if applicable
     */
    if ( !IS_NPC(ch) && ch->desc
    && valid_watch(logline) )
    {
    	if(found && IS_SET(cmd->flags, CMD_WATCH))
    		write_watch_files(ch, cmd, logline);
        /* temporarily commented to test change */
    	/*else if(IS_SET(ch->pcdata->flags, PCFLAG_WATCH))*/
    	if(IS_SET(ch->pcdata->flags, PCFLAG_WATCH))
    		write_watch_files(ch, NULL, logline);
    }


    if ( ( !IS_NPC(ch) && xIS_SET(ch->act, PLR_LOG) )
    ||   fLogAll
    ||	 loglvl == LOG_BUILD
    ||   loglvl == LOG_HIGH
    ||   loglvl == LOG_ALWAYS )
    {
        /* Added by Narn to show who is switched into a mob that executes
           a logged command.  Check for descriptor in case force is used. */
        if ( ch->desc && ch->desc->original ) 
          sprintf( log_buf, "Log %s (%s): %s", ch->name,
                   ch->desc->original->name, logline );
        else
          sprintf( log_buf, "Log %s: %s", ch->name, logline );

	/*
	 * Make it so a 'log all' will send most output to the log
	 * file only, and not spam the log channel to death	-Thoric
	 */
	if ( fLogAll && loglvl == LOG_NORMAL
	&&  (IS_NPC(ch) || !xIS_SET(ch->act, PLR_LOG)) )
	  loglvl = LOG_ALL;

	/* This is handled in get_trust already */
/*	if ( ch->desc && ch->desc->original )
	  log_string_plus( log_buf, loglvl,
		ch->desc->original->level );
	else*/
	  log_string_plus( log_buf, loglvl, get_trust(ch) );
    }

    if ( ch->desc && ch->desc->snoop_by )
    {
  	sprintf( logname, "%s", ch->name);
	write_to_buffer( ch->desc->snoop_by, logname, 0 );
	write_to_buffer( ch->desc->snoop_by, "% ",    2 );
	write_to_buffer( ch->desc->snoop_by, logline, 0 );
	write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
    }

    /* BUILD INTERFACE (start)*/
    if( ch->inter_type == OBJ_TYPE )
    {
	MENU_DATA *m_data = NULL;
	int i;
        switch (ch->inter_page)
	{
          case OBJ_PAGE_A:  m_data=obj_page_a_data;
			    break;
          case OBJ_PAGE_B:  m_data=obj_page_b_data;
			    break;
          case OBJ_PAGE_C:  m_data=obj_page_c_data;
			    break;
          case OBJ_PAGE_D:  m_data=obj_page_d_data;
			    break;
          case OBJ_PAGE_E:  m_data=obj_page_e_data;
			    break;
          case OBJ_HELP_PAGE:  m_data=obj_help_page_data;
			    break;
        }
	if( m_data )
	{
	  for(i=0;m_data[i].ptrType!= (int) NULL;i++)
	  {
             /*IF 1st char matches && 2nd CHAR MATCHES...*/
	     if(! strcmp(m_data[i].sectionNum,command))
	     {
	       if(! strncmp(m_data[i].charChoice,argument,1) )   /* just check the 1st char */
	       {
	          /* ...then MAKE NEW_COMMAND, and */
	          switch (m_data[i].cmdArgs)
	          {
                    case 1: 
		       sprintf(newcommand,m_data[i].cmdString,ch->inter_editing);
		       break;
                    case 2: 
		       argument = one_argument( argument, arg2 );
		       sprintf(newcommand,m_data[i].cmdString,ch->inter_editing, argument);
		       break;
                    case 0: 
	            default:
		       sprintf(newcommand,m_data[i].cmdString);
	          }
		  send_to_char( newcommand, ch );
		  send_to_char( "\n\r", ch );
	          /* ... interpret NEW_COMMAND */
	          interpret( ch, newcommand );
		  refresh_page( ch );
	          return;
	       }
             }
          }
          if(!strcmp("+",command) && get_obj_world(ch, argument))
          {
             sprintf(newcommand,"omenu %s %c",argument,ch->inter_page);
		  send_to_char( newcommand, ch );
		  send_to_char( "\n\r", ch );
	     interpret( ch, newcommand );
	     refresh_page( ch );
	     return;
          }
        }
    }
    if( ch->inter_type == MOB_TYPE )
    {
	MENU_DATA *m_data = NULL;
	int i;
        switch (ch->inter_page)
	{
          case MOB_PAGE_A:  m_data=mob_page_a_data;
			    break;
          case MOB_PAGE_B:  m_data=mob_page_b_data;
			    break;
          case MOB_PAGE_C:  m_data=mob_page_c_data;
			    break;
          case MOB_PAGE_D:  m_data=mob_page_d_data;
			    break;
          case MOB_PAGE_E:  m_data=mob_page_e_data;
			    break;
          case MOB_PAGE_F:  m_data=mob_page_f_data;   /* 7/19 */
			    break;
          case MOB_HELP_PAGE:  m_data=mob_help_page_data;
			    break;
        }
	if( m_data )
	{
	  for(i=0;m_data[i].ptrType!=(int)NULL;i++)
	  {
             /*IF 1st char matches && 2nd CHAR MATCHES...*/
	     if(! strcmp(m_data[i].sectionNum,command))
	     {
	       if(! strncmp(m_data[i].charChoice,argument,1) )   /* just check the 1st char */
	       {
	          /* ...then MAKE NEW_COMMAND, and */
	          switch (m_data[i].cmdArgs)
	          {
                    case 1: 
		       sprintf(newcommand,m_data[i].cmdString,ch->inter_editing);
		       break;
                    case 2: 
		       argument = one_argument( argument, arg2 );
		       sprintf(newcommand,m_data[i].cmdString,ch->inter_editing, argument);
		       break;
                    case 0: 
	            default:
		       sprintf(newcommand,m_data[i].cmdString);
	          }
		  send_to_char( newcommand, ch );
		  send_to_char( "\n\r", ch );
	          /* ... interpret NEW_COMMAND */
	          interpret( ch, newcommand );
		  refresh_page( ch );
	          return;
	       }
             }
          }
          if(!strcmp("+",command) && get_char_world(ch, argument))
          {
             sprintf(newcommand,"mmenu %s %c",argument,ch->inter_page);
		  send_to_char( newcommand, ch );
		  send_to_char( "\n\r", ch );
	     interpret( ch, newcommand );
	     refresh_page( ch );
	     return;
          }
        }
    }
    if( ch->inter_type == ROOM_TYPE )
    {
	MENU_DATA *m_data = NULL;
	int i;
        switch (ch->inter_page)
	{
          case ROOM_PAGE_A:  m_data=room_page_a_data;
			    break;
          case ROOM_PAGE_B:  m_data=room_page_b_data;
			    break;
          case ROOM_PAGE_C:  m_data=room_page_c_data;
			    break;
          case ROOM_HELP_PAGE:  m_data=room_help_page_data;
			    break;
        }
	if( m_data )
	{
	  for(i=0;m_data[i].ptrType!=(int)NULL;i++)
	  {
             /*IF 1st char matches && 2nd CHAR MATCHES...*/
	     if(! strcmp(m_data[i].sectionNum,command))
	     {
	       if(! strncmp(m_data[i].charChoice,argument,1) )   /* just check the 1st char */
	       {
	          /* ...then MAKE NEW_COMMAND, and */
	          switch (m_data[i].cmdArgs)
	          {
                    case 1: 
		       argument = one_argument( argument, arg2 ); /* new */
		       argument = one_argument( argument, arg2 ); /* new */
		       sprintf(newcommand,m_data[i].cmdString,
					  argument);  /* different than mobs */
		       break;                         /* on purpose */
                    case 2: 
		       argument = one_argument( argument, arg2 );
		       sprintf(newcommand,m_data[i].cmdString,
					  ch->inter_editing, 
					  argument);
		       break;
                    case 0: 
	            default:
		       sprintf(newcommand,m_data[i].cmdString);
	          }
		  send_to_char( newcommand, ch );
		  send_to_char( "\n\r", ch );
	          /* ... interpret NEW_COMMAND */
	          interpret( ch, newcommand );
		  refresh_page( ch );
	          return;
	       }
             }
          }
        }
    }  
    /* BUILD INTERFACE ( end )*/

    /* check for a timer delayed command (search, dig, detrap, etc) */
    if ( ( (timer=get_timerptr(ch, TIMER_DO_FUN)) != NULL )
		&& ( !found || !IS_SET( cmd->flags, CMD_FLAG_NO_ABORT ) ) )
    {
	int tempsub;

	tempsub = ch->substate;
	ch->substate = SUB_TIMER_DO_ABORT;
	(timer->do_fun)(ch,"");
	if ( char_died(ch) )
	    return;
	if ( ch->substate != SUB_TIMER_CANT_ABORT )
	{
	    ch->substate = tempsub;
	    extract_timer( ch, timer );
	}
	else
	{
	    ch->substate = tempsub;
	    return;
	}
    }

    /*
     * Look for command in skill and socials table.
     */
    if ( !found )
    {
	if ( !check_skill( ch, command, argument )
	&&   !rprog_command_trigger( ch, origarg )
	&&   !mprog_command_trigger( ch, origarg )
	&&   !oprog_command_trigger( ch, origarg )
	&&   !check_social( ch, command, argument )
        &&   !news_cmd_hook(ch, command, argument)

#ifdef USE_IMC
	&&   !icec_command_hook( ch, command, argument ) )
#else
	)
#endif
	{
	    EXIT_DATA *pexit;

	    /* check for an auto-matic exit command */
	    if ( (pexit = find_door( ch, command, TRUE )) != NULL
	    &&   IS_SET( pexit->exit_info, EX_xAUTO ))
	    {
		if ( IS_SET(pexit->exit_info, EX_CLOSED)
		&& (!IS_AFFECTED(ch, AFF_PASS_DOOR)
		||   IS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
		{
		  if ( !IS_SET( pexit->exit_info, EX_SECRET ) )
		    act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
		  else
		    send_to_char( "You cannot do that here.\n\r", ch );
		  return;
		}
		move_char( ch, pexit, 0 );
		return;
	    }
	    send_to_char( "Huh?\n\r", ch );
	}
	return;
    }

    /*
     * Character not in position for command?
     */
    if ( !check_pos( ch, cmd->position ) )
	return;
    
    /* Berserk check for flee.. maybe add drunk to this?.. but too much
       hardcoding is annoying.. -- Altrag
       This wasn't catching wimpy --- Blod
    if ( !str_cmp(cmd->name, "flee") &&
          IS_AFFECTED(ch, AFF_BERSERK) )
    {
	send_to_char( "You aren't thinking very clearly..\n\r", ch);
	return;
    } */

    /*  So we can check commands for things like Posses and Polymorph
     *  But still keep the online editing ability.  -- Shaddai
     *  Send back the message to print out, so we have the option
     *  this function might be usefull elsewhere.  Also using the
     *  send_to_char_color so we can colorize the strings if need be. --Shaddai
     */

    buf = check_cmd_flags ( ch, cmd );

    if ( buf[0] != '\0'  ) {
        send_to_char_color( buf, ch );
        return;
    }

    /*
     * Nuisance stuff -- Shaddai
     */
 
    if ( !IS_NPC(ch) && ch->pcdata->nuisance && ch->pcdata->nuisance->flags > 9 
         && number_percent() < ((ch->pcdata->nuisance->flags-9)*10
         *ch->pcdata->nuisance->power))
    {
	send_to_char("You can't seem to do that just now.\n\r", ch );
	return;	
    }
    	
    /*
     * Dispatch the command.
     */
    ch->prev_cmd = ch->last_cmd;    /* haus, for automapping */
    ch->last_cmd = cmd->do_fun;
    start_timer(&time_used);
    (*cmd->do_fun) ( ch, argument );
    end_timer(&time_used);
    /*
     * Update the record of how many times this command has been used (haus)
     */
    update_userec(&time_used, &cmd->userec);
    tmptime = UMIN(time_used.tv_sec,19) * 1000000 + time_used.tv_usec;

    /* laggy command notice: command took longer than 1.5 seconds */
    if ( tmptime > 1500000 )
    {
#ifdef sun
        sprintf(log_buf, "[*****] LAG: %s: %s %s (R:%d S:%ld.%06ld)", ch->name,
                cmd->name, (cmd->log == LOG_NEVER ? "XXX" : argument),
		ch->in_room ? ch->in_room->vnum : 0,
		time_used.tv_sec, time_used.tv_usec );
#else
        sprintf(log_buf, "[*****] LAG: %s: %s %s (R:%d S:%ld.%06ld)", ch->name,
                cmd->name, (cmd->log == LOG_NEVER ? "XXX" : argument),
		ch->in_room ? ch->in_room->vnum : 0,
		(long)time_used.tv_sec, (long)time_used.tv_usec );
#endif
	log_string_plus(log_buf, LOG_NORMAL, get_trust(ch));
	cmd->lag_count++;	/* count the lag flags */
    }

    tail_chain( );
}

CMDTYPE *find_command( char *command )
{
    CMDTYPE *cmd;
    int hash;

    hash = LOWER(command[0]) % 126;

    for ( cmd = command_hash[hash]; cmd; cmd = cmd->next )
	if ( !str_prefix( command, cmd->name ) )
	    return cmd;

    return NULL;
}

SOCIALTYPE *find_social( char *command )
{
    SOCIALTYPE *social;
    int hash;

    if ( command[0] < 'a' || command[0] > 'z' )
	hash = 0;
    else
	hash = (command[0] - 'a') + 1;

    for ( social = social_index[hash]; social; social = social->next )
	if ( !str_prefix( command, social->name ) )
	    return social;

    return NULL;
}

bool check_social( CHAR_DATA *ch, char *command, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim, *victim_next;
    SOCIALTYPE *social;
    CHAR_DATA *remfirst, *remlast, *remtemp; /* for ignore cmnd */

    if ( (social=find_social(command)) == NULL )
	return FALSE;

    if ( !IS_NPC(ch) && xIS_SET(ch->act, PLR_NO_EMOTE) )
    {
	send_to_char( "You are anti-social!\n\r", ch );
	return TRUE;
    }
   
    switch ( ch->position )
    {
    case POS_DEAD:
	send_to_char( "Lie still; you are DEAD.\n\r", ch );
	return TRUE;

    case POS_INCAP:
    case POS_MORTAL:
	send_to_char( "You are hurt far too bad for that.\n\r", ch );
	return TRUE;

    case POS_STUNNED:
	send_to_char( "You are too stunned to do that.\n\r", ch );
	return TRUE;

    case POS_SLEEPING:
	/*
	 * I just know this is the path to a 12" 'if' statement.  :(
	 * But two players asked for it already!  -- Furey
	 */
	if ( !str_cmp( social->name, "snore" ) )
	    break;
	send_to_char( "In your dreams, or what?\n\r", ch );
	return TRUE;

    }

    remfirst = NULL;
    remlast = NULL;
    remtemp = NULL;
    
    /* Search room for chars ignoring social sender and */
    /* remove them from the room until social has been  */
    /* completed					*/
	/* Bug fix for ignore from Valcados 1/02 */
    for(victim = ch->in_room->first_person;victim;victim = victim_next)
    {
		victim_next = victim->next_in_room;
    	if(is_ignoring(victim, ch))
    	{
    		if(!IS_IMMORTAL(ch) || get_trust(victim) > get_trust(ch))
    		{
    			char_from_room(victim);
    			LINK(victim, remfirst, remlast, next_in_room,
    				prev_in_room);
    		}
    		else
    		{
    			set_char_color(AT_IGNORE, victim);
    			ch_printf(victim, "You attempt to ignore %s,"
    				" but are unable to do so.\n\r", !can_see(victim, ch) ? "Someone" : ch->name);
    		}
    	}
    }

    one_argument( argument, arg );
    victim = NULL;
    if ( arg[0] == '\0' )
    {
	act( AT_SOCIAL, social->others_no_arg, ch, NULL, victim, TO_ROOM    );
	act( AT_SOCIAL, social->char_no_arg,   ch, NULL, victim, TO_CHAR    );
    }
    else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
    	/* If they aren't in the room, they may be in the list of */
    	/* people ignoring...					  */
    	for(victim = remfirst; victim; victim = victim->next_in_room)
    	{
    		if(nifty_is_name(victim->name,arg) ||
    			nifty_is_name_prefix(arg,victim->name))
    		{
    			set_char_color(AT_IGNORE, ch);
    			ch_printf(ch,"%s is ignoring you.\n\r",
    				victim->name);
    			break;
    		}
    	}
    	
    	if(!victim)
		send_to_char( "They aren't here.\n\r", ch );
    }
    else if ( victim == ch )
    {
	act( AT_SOCIAL, social->others_auto,   ch, NULL, victim, TO_ROOM    );
	act( AT_SOCIAL, social->char_auto,     ch, NULL, victim, TO_CHAR    );
    }
    else
    {
	act( AT_SOCIAL, social->others_found,  ch, NULL, victim, TO_NOTVICT );
	act( AT_SOCIAL, social->char_found,    ch, NULL, victim, TO_CHAR    );
	act( AT_SOCIAL, social->vict_found,    ch, NULL, victim, TO_VICT    );

	if ( !IS_NPC(ch) && IS_NPC(victim)
	&&   !IS_AFFECTED(victim, AFF_CHARM)
	&&   IS_AWAKE(victim) 
	&&   !victim->desc	// This was just really annoying.. lemme do my own socials! -- Alty
	&&   !HAS_PROG(victim->pIndexData, ACT_PROG) )
	{
	    switch ( number_bits( 4 ) )
	    {
	    case 0:
		if (IS_EVIL(ch) && !is_safe(victim, ch, TRUE)) /* was IS_EVIL(ch) ||.... didn't make sense to me - FB */
		  multi_hit( victim, ch, TYPE_UNDEFINED );
		else
		if ( IS_NEUTRAL(ch) )
		{
		    act( AT_ACTION, "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
		    act( AT_ACTION, "You slap $N.",  victim, NULL, ch, TO_CHAR    );
		    act( AT_ACTION, "$n slaps you.", victim, NULL, ch, TO_VICT    );
		}
		else
		{
		    act( AT_ACTION, "$n acts like $N doesn't even exist.",  victim, NULL, ch, TO_NOTVICT );
		    act( AT_ACTION, "You just ignore $N.",  victim, NULL, ch, TO_CHAR    );
		    act( AT_ACTION, "$n appears to be ignoring you.", victim, NULL, ch, TO_VICT    );
		}
		break;

	    case 1: case 2: case 3: case 4:
	    case 5: case 6: case 7: case 8:
		act( AT_SOCIAL, social->others_found,
		    victim, NULL, ch, TO_NOTVICT );
		act( AT_SOCIAL, social->char_found,
		    victim, NULL, ch, TO_CHAR    );
		act( AT_SOCIAL, social->vict_found,
		    victim, NULL, ch, TO_VICT    );
		break;

	    case 9: case 10: case 11: case 12:
		act( AT_ACTION, "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
		act( AT_ACTION, "You slap $N.",  victim, NULL, ch, TO_CHAR    );
		act( AT_ACTION, "$n slaps you.", victim, NULL, ch, TO_VICT    );
		break;
	    }
	}
    }
    
    /* Replace the chars in the ignoring list to the room */
    /* note that the ordering of the players in the room  */
    /* might change					  */
    for(victim = remfirst; victim; victim = remtemp)
    {
    	remtemp = victim->next_in_room;
    	char_to_room(victim, ch->in_room);
    }

    return TRUE;
}



/*
 * Return true if an argument is completely numeric.
 */
bool is_number( char *arg )
{
    bool first = TRUE;
    if ( *arg == '\0' )
	return FALSE;

    for ( ; *arg != '\0'; arg++ )
    {
        if ( first && *arg == '-')	
	{
		first = FALSE;
		continue; 
	}
	if ( !isdigit(*arg) )
	    return FALSE;
	first = FALSE;
    }

    return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
    char *pdot;
    int number;

    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
	if ( *pdot == '.' )
	{
	    *pdot = '\0';
	    number = atoi( argument );
	    *pdot = '.';
	    strcpy( arg, pdot+1 );
	    return number;
	}
    }

    strcpy( arg, argument );
    return 1;
}



/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;
    sh_int count;

    count = 0;

    if ( !argument || argument[0] == '\0' )
    {
    	arg_first[0] = '\0';
	return argument;
    }

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' || ++count >= 255 )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = LOWER(*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 * Doesn't smash case.  For use with things like password command. -- Alty
 */
char *case_argument( char *argument, char *arg_first )
{
    char cEnd;
    sh_int count;

    count = 0;

    if ( !argument || argument[0] == '\0' )
    {
    	arg_first[0] = '\0';
	return argument;
    }

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' || ++count >= 255 )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = *argument;
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.  Delimiters = { ' ', '-' }
 */
/* Changed to fix a bug with arguments like "two-handed" - Luc 08/2000 */
char *one_argument2( register char *argument, char *arg_first ) {
   int count = 0;
   register char ch;
   register char quote = 0;

   if ( !argument || !argument[0] ) {
      arg_first[0] = '\0';
      return argument;
      }
   ch = *argument;
   while ( isspace( ch ) || ch == '-' )
      ch = *++argument;
   if ( ch == '\'' || ch == '"' ) {
      quote = ch;
      ch = *++argument;
      }
   while ( ch || ++count >= 255 ) {
      if ( quote ? ch == quote : isspace( ch ) || ch == '-' ) {
         argument++;
         break;
         }
      *(arg_first++) = LOWER( ch );
      ch = *++argument;
      }
   *arg_first = '\0';
   ch = *argument;
   while ( isspace( ch ) || ch == '-' )
      ch = *++argument;
   return argument;
   }


void do_timecmd( CHAR_DATA *ch, char *argument )
{
  struct timeval stime;
  struct timeval etime;
  static bool timing;
  extern CHAR_DATA *timechar;
  char arg[MAX_INPUT_LENGTH];
  
  send_to_char("Timing\n\r",ch);
  if ( timing )
    return;
  one_argument(argument, arg);
  if ( !*arg )
  {
    send_to_char( "No command to time.\n\r", ch );
    return;
  }
  if ( !str_cmp(arg, "update") )
  {
    if ( timechar )
      send_to_char( "Another person is already timing updates.\n\r", ch );
    else
    {
      timechar = ch;
      send_to_char( "Setting up to record next update loop.\n\r", ch );
    }
    return;
  }
  set_char_color(AT_PLAIN, ch);
  send_to_char( "Starting timer.\n\r", ch );
  timing = TRUE;
  gettimeofday(&stime, NULL);
  interpret(ch, argument);
  gettimeofday(&etime, NULL);
  timing = FALSE;
  set_char_color(AT_PLAIN, ch);
  send_to_char( "Timing complete.\n\r", ch );
  subtract_times(&etime, &stime);
  ch_printf( ch, "Timing took %d.%06d seconds.\n\r",
      etime.tv_sec, etime.tv_usec );
  return;
}

void start_timer(struct timeval *stime)
{
  if ( !stime )
  {
    bug( "Start_timer: NULL stime.", 0 );
    return;
  }
  gettimeofday(stime, NULL);
  return;
}

time_t end_timer(struct timeval *stime)
{
  struct timeval etime;
  
  /* Mark etime before checking stime, so that we get a better reading.. */
  gettimeofday(&etime, NULL);
  if ( !stime || (!stime->tv_sec && !stime->tv_usec) )
  {
    bug( "End_timer: bad stime.", 0 );
    return 0;
  }
  subtract_times(&etime, stime);
  /* stime becomes time used */
  *stime = etime;
  return (etime.tv_sec*1000000)+etime.tv_usec;
}

void send_timer(struct timerset *vtime, CHAR_DATA *ch)
{
  struct timeval ntime;
  int carry;
  
  if ( vtime->num_uses == 0 )
    return;
  ntime.tv_sec  = vtime->total_time.tv_sec / vtime->num_uses;
  carry = (vtime->total_time.tv_sec % vtime->num_uses) * 1000000;
  ntime.tv_usec = (vtime->total_time.tv_usec + carry) / vtime->num_uses;
  ch_printf(ch, "Has been used %d times this boot.\n\r", vtime->num_uses);
  ch_printf(ch, "Time (in secs): min %d.%0.6d; avg: %d.%0.6d; max %d.%0.6d"
      "\n\r", vtime->min_time.tv_sec, vtime->min_time.tv_usec, ntime.tv_sec,
      ntime.tv_usec, vtime->max_time.tv_sec, vtime->max_time.tv_usec);
  return;
}

void update_userec(struct timeval *time_used, struct timerset *userec)
{
  userec->num_uses++;
  if ( !timerisset(&userec->min_time)
  ||    timercmp(time_used, &userec->min_time, <) )
  {
    userec->min_time.tv_sec  = time_used->tv_sec;
    userec->min_time.tv_usec = time_used->tv_usec;
  }
  if ( !timerisset(&userec->max_time)
  ||    timercmp(time_used, &userec->max_time, >) )
  {
    userec->max_time.tv_sec  = time_used->tv_sec;
    userec->max_time.tv_usec = time_used->tv_usec;
  }
  userec->total_time.tv_sec  += time_used->tv_sec;
  userec->total_time.tv_usec += time_used->tv_usec;
  while ( userec->total_time.tv_usec >= 1000000 )
  {
    userec->total_time.tv_sec++;
    userec->total_time.tv_usec -= 1000000;
  }
  return;
}

/*
 *  This function checks the command against the command flags to make
 *  sure they can use the command online.  This allows the commands to be
 *  edited online to allow or disallow certain situations.  May be an idea
 *  to rework this so we can edit the message sent back online, as well as
 *  maybe a crude parsing language so we can add in new checks online without
 *  haveing to hard-code them in.     -- Shaddai   August 25, 1997
 */

/* Needed a global here */
char cmd_flag_buf[MAX_STRING_LENGTH];

char *
check_cmd_flags ( CHAR_DATA *ch, CMDTYPE *cmd )
{

  if ( IS_AFFECTED (ch, AFF_POSSESS) && IS_SET( cmd->flags, CMD_FLAG_POSSESS )) 
        sprintf ( cmd_flag_buf, "You can't %s while you are possessing someone!\n\r",
                cmd->name );
  else if ( ch->morph != NULL
            && IS_SET( cmd->flags, CMD_FLAG_POLYMORPHED ) )
        sprintf ( cmd_flag_buf, "You can't %s while you are polymorphed!\n\r",
                cmd->name );
  else
        cmd_flag_buf[0] = '\0';

  return cmd_flag_buf;
}

