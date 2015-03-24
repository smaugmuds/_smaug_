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
 *                           Deity handling module                          *
 ****************************************************************************/

/*Put together by Rennard for Realms of Despair.  Brap on...*/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


DEITY_DATA * first_deity;
DEITY_DATA * last_deity;

/* local routines */

void	fread_deity	 args( ( DEITY_DATA *deity, FILE *fp ) );
bool	load_deity_file	 args( ( char *deityfile ) );
void	write_deity_list args( ( void ) );
int	get_risflag	 args( ( char *flag ) );
int	get_npc_race	 args( ( char *type ) );
int	get_pc_race	 args( ( char *type ) );

/* Get pointer to deity structure from deity name */

DEITY_DATA *get_deity( char *name )
{
     DEITY_DATA *deity;
     for ( deity = first_deity; deity; deity = deity->next )
	if ( !str_cmp( name, deity->name ) )
	  return deity;
	return NULL;
}

void write_deity_list( )
{
     DEITY_DATA *tdeity;
     FILE *fpout;
     char filename[256];

     sprintf( filename, "%s%s", DEITY_DIR, DEITY_LIST );
     fclose(fpReserve);
     fpout = fopen( filename, "w" );
     if ( !fpout )
	bug( "FATAL: cannot open deity.lst for writing!\n\r", 0 );
     else
     {
       for ( tdeity = first_deity; tdeity; tdeity = tdeity->next )
	  fprintf( fpout, "%s\n", tdeity->filename );
       fprintf( fpout, "$\n" );
       fclose( fpout );
     }
     fpReserve = fopen(NULL_FILE, "r");
}

/* Save a deity's data to its data file */

void save_deity( DEITY_DATA *deity )
{
     FILE *fp;
     char filename[256];
     char buf[MAX_STRING_LENGTH];

     if ( !deity )
     {
	bug( "save_deity: null deity pointer!", 0 );
	return;
     }

     if ( !deity->filename || deity->filename[0] == '\0' )
     {
	sprintf( buf, "save_deity: %s has no filename", deity->name );
	bug( buf, 0 );
	return;
     }

     sprintf( filename, "%s%s", DEITY_DIR, deity->filename );

     fclose( fpReserve );
     if ( ( fp = fopen( filename, "w" ) ) == NULL )
     {
	bug( "save_deity: fopen", 0 );
	perror( filename );
     }
     else
     {
	fprintf( fp, "#DEITY\n" );
	fprintf( fp, "Filename		%s~\n",	deity->filename );
	fprintf( fp, "Name		%s~\n",	deity->name	);
	fprintf( fp, "Description	%s~\n",	deity->description );
	fprintf( fp, "Alignment		%d\n",	deity->alignment );
	fprintf( fp, "Worshippers	%d\n",	deity->worshippers );
	fprintf( fp, "Flee		%d\n",	deity->flee );
	fprintf( fp, "Flee_npcrace	%d\n",	deity->flee_npcrace );
	fprintf( fp, "Flee_npcfoe	%d\n",	deity->flee_npcfoe );
	fprintf( fp, "Kill		%d\n",	deity->kill );
	fprintf( fp, "Kill_npcrace	%d\n",	deity->kill_npcrace );
	fprintf( fp, "Kill_npcfoe	%d\n",	deity->kill_npcfoe );
	fprintf( fp, "Kill_magic	%d\n",	deity->kill_magic );
	fprintf( fp, "Sac		%d\n",	deity->sac );
	fprintf( fp, "Bury_corpse	%d\n",	deity->bury_corpse );
	fprintf( fp, "Aid_spell		%d\n",	deity->aid_spell );
	fprintf( fp, "Aid		%d\n",	deity->aid );
	fprintf( fp, "Steal		%d\n",	deity->steal );
	fprintf( fp, "Backstab		%d\n",	deity->backstab );
	fprintf( fp, "Die		%d\n",	deity->die );
	fprintf( fp, "Die_npcrace	%d\n",	deity->die_npcrace );
	fprintf( fp, "Die_npcfoe	%d\n",	deity->die_npcfoe );
	fprintf( fp, "Spell_aid		%d\n",	deity->spell_aid );
	fprintf( fp, "Dig_corpse	%d\n",	deity->dig_corpse );
	fprintf( fp, "Scorpse		%d\n",	deity->scorpse );
	fprintf( fp, "Savatar		%d\n",	deity->savatar );
	fprintf( fp, "Sdeityobj		%d\n",	deity->sdeityobj );
	fprintf( fp, "Srecall		%d\n",	deity->srecall );
	fprintf( fp, "Race		%d\n",	deity->race );
	fprintf( fp, "Class		%d\n",	deity->class );
	fprintf( fp, "Element		%d\n",	deity->element );
	fprintf( fp, "Sex		%d\n",	deity->sex );
	fprintf( fp, "Affected		%s\n",	print_bitvector(&deity->affected) );
	fprintf( fp, "Npcrace		%d\n",	deity->npcrace );
	fprintf( fp, "Npcfoe		%d\n",	deity->npcfoe );

	fprintf( fp, "Suscept		%d\n",	deity->suscept );
	fprintf( fp, "Race2		%d\n",	deity->race2 );
	fprintf( fp, "Susceptnum	%d\n",  deity->susceptnum );
	fprintf( fp, "Elementnum	%d\n",  deity->elementnum );
	fprintf( fp, "Affectednum	%d\n",	deity->affectednum );
	fprintf( fp, "Objstat		%d\n",	deity->objstat	);
	fprintf( fp, "End\n\n" );
	fprintf( fp, "#END\n" );
	fclose(fp);
     }
     fpReserve = fopen ( NULL_FILE, "r" );
     return;
}

/* Read in actual deity data */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				      field = value;			\
				      fMatch = TRUE;			\
				      break;				\
				}

void fread_deity( DEITY_DATA *deity, FILE *fp )
{
     char buf[MAX_STRING_LENGTH];
     char *word;
     bool fMatch;

     for ( ; ; )
     {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    KEY( "Affected",	deity->affected,	fread_bitvector( fp ) );
	    KEY( "Affectednum",	deity->affectednum,	fread_number( fp ) );
	    KEY( "Aid",		deity->aid,		fread_number( fp ) );
	    KEY( "Aid_spell",	deity->aid_spell,	fread_number( fp ) );
	    KEY( "Alignment",	deity->alignment,	fread_number( fp ) );
	    break;	

	case 'B':
	    KEY( "Backstab",	deity->backstab,	fread_number( fp ) );
	    KEY( "Bury_corpse",	deity->bury_corpse,	fread_number( fp ) );
	    break;

	case 'C':
	    KEY( "Class",	deity->class,		fread_number( fp ) );	
	    break;

	case 'D':
	    KEY( "Description",	deity->description,	fread_string( fp ) );
	    KEY( "Die",		deity->die,		fread_number( fp ) );
	    KEY( "Die_npcrace",	deity->die_npcrace,	fread_number( fp ) );
	    KEY( "Die_npcfoe",	deity->die_npcfoe,	fread_number( fp ) );
	    KEY( "Dig_corpse",	deity->dig_corpse,	fread_number( fp ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if (!deity->name)
		  deity->name			= STRALLOC( "" );
		if (!deity->description)
		  deity->description		= STRALLOC( "" );
		return;
	    }
	    KEY( "Element",	deity->element,		fread_number( fp ) );
	    KEY( "Elementnum",	deity->elementnum,	fread_number( fp ) );
	    break;

	case 'F':
	    KEY( "Filename",	deity->filename,	fread_string_nohash( fp ) );
	    KEY( "Flee",	deity->flee,		fread_number( fp ) ); 
	    KEY( "Flee_npcrace",deity->flee_npcrace,	fread_number( fp ) );
	    KEY( "Flee_npcfoe",	deity->flee_npcfoe,	fread_number( fp ) );
	    break;

	case 'K':
	    KEY( "Kill",	deity->kill,		fread_number( fp ) );
	    KEY( "Kill_npcrace",deity->kill_npcrace,	fread_number( fp ) );
	    KEY( "Kill_npcfoe",	deity->kill_npcfoe,	fread_number( fp ) );
	    KEY( "Kill_magic",	deity->kill_magic,	fread_number( fp ) );
	    break;

	case 'N':
	    KEY( "Name",	deity->name,		fread_string( fp ) );
	    KEY( "Npcfoe",	deity->npcfoe,		fread_number( fp ) );
	    KEY( "Npcrace",	deity->npcrace,		fread_number( fp ) );
	    break;

	case 'O':
	    KEY( "Objstat",	deity->objstat,		fread_number( fp ) );

	case 'R':
	    KEY( "Race",	deity->race,		fread_number( fp ) );
	    KEY( "Race2",	deity->race2,		fread_number( fp ) );
	    break;

	case 'S':
	    KEY( "Sac",		deity->sac,		fread_number( fp ) );
	    KEY( "Savatar",	deity->savatar,		fread_number( fp ) );
	    KEY( "Scorpse",	deity->scorpse,		fread_number( fp ) );
	    KEY( "Sdeityobj",	deity->sdeityobj,	fread_number( fp ) );
	    KEY( "Srecall",	deity->srecall,		fread_number( fp ) );
	    KEY( "Sex",		deity->sex,		fread_number( fp ) );
	    KEY( "Spell_aid",   deity->spell_aid,	fread_number( fp ) );
	    KEY( "Steal",	deity->steal,		fread_number( fp ) );
	    KEY( "Suscept",	deity->suscept,		fread_number( fp ) );
	    KEY( "Susceptnum",  deity->susceptnum,	fread_number( fp ) );
	    break;

	case 'W':
	    KEY( "Worshippers",	deity->worshippers,	fread_number( fp ) );
	    break;
	}

	if ( !fMatch )
	{
	    sprintf( buf, "Fread_deity: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}

/* Load a deity file */

bool load_deity_file( char *deityfile )
{
     char filename[256];
     DEITY_DATA *deity;
     FILE *fp;
     bool found;

     found = FALSE;
     sprintf( filename, "%s%s", DEITY_DIR, deityfile );

     if ( ( fp = fopen( filename, "r" ) ) != NULL )
     {
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_deity_file: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "DEITY" ) )
	    {
		CREATE( deity, DEITY_DATA, 1 );
		fread_deity( deity, fp );
		LINK(deity, first_deity, last_deity, next, prev);
		found = TRUE;
		break;
	    }
	    else
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf( buf, "Load_deity_file: bad section: %s.", word );
		bug( buf, 0 );
		break;
	    }
	}
	fclose( fp );
    }

    return found;
}

/* Load in all the deity files */

void load_deity( )
{
    FILE *fpList;
    char *filename;
    char deitylist[256];
    char buf[MAX_STRING_LENGTH];

    first_deity = NULL;
    last_deity  = NULL;

    log_string( "Loading deities..." );

    sprintf( deitylist, "%s%s", DEITY_DIR, DEITY_LIST );
    if ( ( fpList = fopen ( deitylist, "r" ) ) == NULL )
    {
	perror( deitylist );
	exit( 1 );
    }

    for ( ; ; )
    {
	filename = feof( fpList ) ? "$" : fread_word( fpList );
	log_string( filename );
	if ( filename[0] == '$' )
	   break;
	if ( !load_deity_file( filename ) )
	{
	  sprintf( buf, "Cannot load deity file: %s", filename );
	  bug( buf, 0 );
	}
    }	
    fclose( fpList );
    log_string( " Done deities " );
    return;
}

void do_setdeity( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    DEITY_DATA *deity;
    int value;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    switch( ch->substate )
    {
        default:
		break;
 
	case SUB_RESTRICTED:
		send_to_char( "You cannot do this while in another command.\n\r", ch );
		return;

	case SUB_DEITYDESC:  
		deity = ch->dest_buf;
        	STRFREE( deity->description );
        	deity->description = copy_buffer( ch );
        	stop_editing( ch );
		save_deity( deity );
      		ch->substate = ch->tempnum;
        	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Usage: setdeity <deity> <field> <toggle>\n\r", ch );
	send_to_char( "\n\rField being one of:\n\r", ch );
	send_to_char( "filename name description type alignment worshippers npcfoe susceptnum\n\r", ch );
	send_to_char( "race race2 npcrace class element sex affected suscept elementnum affectednum\n\r", ch );
	send_to_char( "\n\rFavor adjustments:\n\r", ch );
	send_to_char( "flee flee_npcrace kill kill_npcrace kill_magic\n\r", ch );
	send_to_char( "die die_npcrace dig_corpse bury_corpse spell_aid\n\r", ch);
	send_to_char( "steal backstab aid aid_spell sac kill_npcfoe\n\r", ch );
	send_to_char( "die_npcfoe flee_npcfoe\n\r", ch );
	send_to_char( "\n\rFavor requirements for supplicate:\n\r", ch );
	send_to_char( "scorpse savatar sdeityobj srecall\n\r\n\r", ch );
	send_to_char( "Objstat - being one of:\n\r", ch );
	send_to_char( "str int wis con dex cha lck\n\r", ch );
	send_to_char( " 0 - 1 - 2 - 3 - 4 - 5 - 6\n\r", ch );
	return;
    }

    deity = get_deity( arg1 );
    if ( !deity )
    {
 	send_to_char( "No such deity.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
	STRFREE( deity->name );
	deity->name = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "filename" ) )
    {
	DISPOSE( deity->filename );
	deity->filename = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	write_deity_list( );
	return;
    }

    if ( !str_cmp( arg2, "description" ) )
    {
	if ( ch->substate == SUB_REPEATCMD )
	  ch->tempnum = SUB_REPEATCMD;
	else
          ch->tempnum = SUB_NONE;
        ch->substate = SUB_DEITYDESC;
        ch->dest_buf = deity;
        start_editing( ch, deity->description );
        return;
    }

    if ( !str_cmp( arg2, "alignment" ) )
    {
	deity->alignment = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "flee" ) )
    {
	deity->flee = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "flee_npcrace" ) )
    {
        deity->flee_npcrace = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "flee_npcfoe" ) )
    {
	deity->flee_npcfoe = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "kill" ) )
    {
        deity->kill = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "kill_npcrace" ) )
    {
        deity->kill_npcrace = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "kill_npcfoe" ) )
    {
	deity->kill_npcfoe = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "kill_magic" ) )
    {
        deity->kill_magic = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "sac" ) )
    {
        deity->sac = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "bury_corpse" ) )
    {
        deity->bury_corpse = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "aid_spell" ) )
    {
        deity->aid_spell = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "aid" ) )
    {
        deity->aid = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "steal" ) )
    {
        deity->steal = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "backstab" ) )
    {
        deity->backstab = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "die" ) )
    {
        deity->die = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "die_npcrace" ) )
    {
        deity->die_npcrace = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "die_npcfoe" ) )
    {
	deity->die_npcfoe = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "spell_aid" ) )
    {
        deity->spell_aid = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "dig_corpse" ) )
    {
        deity->dig_corpse = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "scorpse" ) )
    {
        deity->scorpse = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "savatar" ) )
    {
        deity->savatar = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "sdeityobj" ) )
    {
        deity->sdeityobj = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "objstat" ) )
    {
	deity->objstat = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "srecall" ) )
    {
        deity->srecall = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "worshippers" ) )
    {
	deity->worshippers = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "race" ) )
    {
	value = get_pc_race( argument );
	if ( value < 0 ) value = atoi( argument );
	if ( ( value < 0 ) || ( value >= MAX_PC_RACE ) )
	{
	  deity->race = -1;
	  send_to_char( "No race set.\n\r", ch );
	  return;
	}
	deity->race = value;
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "race2" ) )
    {
        value = get_pc_race( argument );
        if ( value < 0 ) value = atoi( argument );
        if ( ( value < 0 ) || ( value >= MAX_PC_RACE ) )
        {
          deity->race2 = -1;
          send_to_char( "No race set.\n\r", ch );
          return;
        }
        deity->race2 = value;
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }
 

    if ( !str_cmp( arg2, "npcrace" ) )
    {
	value = get_npc_race( argument );
	if ( value < 0 ) value = atoi( argument );
	if ( ( value < 0 ) || ( value >= MAX_NPC_RACE ) )
	{
	  send_to_char( "Invalid npc race.\n\r", ch );
	  return;
	}	
	deity->npcrace = value;
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "npcfoe" ) )
    {
	value = get_npc_race( argument );
	if ( value < 0 ) value = atoi( argument );
	if ( ( value < 0 ) || ( value >= MAX_NPC_RACE ) )
	{
	  send_to_char( "Invalid npc race.\n\r", ch );
	  return;
	}
	deity->npcfoe = value;
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "class" ) )
    {
	deity->class = atoi( argument );
	if ( ( deity->class < 0 ) || ( deity->class >= MAX_PC_CLASS ) ) deity->class = -1;
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "susceptnum" ) )
    {
	deity->susceptnum = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "elementnum" ) )
    {
	deity->elementnum = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "affectednum" ) )
    {
	deity->affectednum = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "suscept" ) )
    {
      bool fMatch = FALSE;
 
      while ( argument[0] != '\0' )
        {
           argument = one_argument( argument, arg3 );
           if ( !str_cmp ( arg3, "none" ) )
            {
            fMatch = TRUE;
            deity->suscept = 0;
            }
           else
            {
             value = get_risflag( arg3 );
             if ( value < 0 || value > 31 )
              ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
             else
             {
              TOGGLE_BIT( deity->suscept, 1 << value );
              fMatch = TRUE;
             }
            }
        }
 
        if ( fMatch ) ch_printf( ch, "Done.\n\r" );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "element" ) )
    {
      bool fMatch = FALSE;

      while ( argument[0] != '\0' )
        {
           argument = one_argument( argument, arg3 );
           if ( !str_cmp ( arg3, "none" ) )
            {
            fMatch = TRUE;
            deity->element = 0;
            }
           else
            {
             value = get_risflag( arg3 );
             if ( value < 0 || value > 31 )
              ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
             else
             {
              TOGGLE_BIT( deity->element, 1 << value );
              fMatch = TRUE;
             }
            }
        }
        if ( fMatch ) ch_printf( ch, "Done.\n\r" );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "sex" ) )
    {
	deity->sex = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "affected" ) )
    {
	bool fMatch = FALSE;

	while ( argument[0] != '\0' )      
	{
	    argument = one_argument( argument, arg3 );
	    if ( !str_cmp ( arg3, "none" ) )
	    {
		fMatch = TRUE;
		xCLEAR_BITS(deity->affected);
            }
	    else
	    {
		value = get_aflag( arg3 );
		if ( value < 0 || value > MAX_BITS )
		    ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
		else
		{
        	    xTOGGLE_BIT( deity->affected, value );
        	    fMatch = TRUE;
		}
	    }
	}

	if ( fMatch ) ch_printf( ch, "Done.\n\r" );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "delete" ) )
      /*
       * Remove the deity from all online players - everything under the IF
       * statement is a copy from do_devote for "none" so we remove all affects
       */
    {
    CHAR_DATA *vch;

    for ( vch = first_char; vch; vch = vch->next )
    {
      set_char_color( AT_RED, vch );
      if ( !IS_NPC( vch ) )
      {
        if ( vch->pcdata->deity == deity )
        {
	   sprintf( buf, "&R\n\rYour deity, %s, has met its demise!\n\r", vch->pcdata->deity_name );
	   if ( !vch->desc )
		add_loginmsg( vch->name, 18, buf );
	   else
		send_to_char_color( buf, vch );
        xREMOVE_BITS( vch->affected_by, vch->pcdata->deity->affected );
        REMOVE_BIT( vch->resistant, vch->pcdata->deity->element );
        REMOVE_BIT( vch->susceptible, vch->pcdata->deity->suscept );
        vch->pcdata->deity = NULL;
        STRFREE( vch->pcdata->deity_name );
        vch->pcdata->deity_name = STRALLOC( "" );
        save_char_obj( vch );
        }
      }
    }
    /* Delete the deity from the list in the deity.lst file and save */
        UNLINK( deity, first_deity, last_deity, next, prev );
        write_deity_list();
        send_to_char( "Deity deleted.\n\r", ch );
    /* Dispose the deity from memory */
        STRFREE( deity->name );
        STRFREE( deity->filename );
        STRFREE( deity->description );
        DISPOSE( deity );
    return;
    }

    do_setdeity( ch, "" );
    return;
}


void do_showdeity( CHAR_DATA *ch, char *argument )
{
    DEITY_DATA *deity;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Usage: showdeity <deity>\n\r", ch );
	return;
    }

    deity = get_deity( argument );
    if ( !deity )
    {
	send_to_char( "No such deity.\n\r", ch );
	return;
    }

    ch_printf( ch, "Deity: %s\n\rFilename: %s\n\rDescription:\n\r%s\n\r", deity->name, deity->filename, deity->description );   
    ch_printf( ch, "Alignment: %-7dNpcrace: %-9sNpcfoe: %s\n\r", 
	deity->alignment, 
	( deity->npcrace < 0 || deity->npcrace > MAX_NPC_RACE ) ? "none" : npc_race[deity->npcrace],
	( deity->npcfoe < 0 || deity->npcfoe > MAX_NPC_RACE ) ? "none" : npc_race[deity->npcfoe] );
    ch_printf( ch, "Race: %-12sClass: %-11sSex: %-13sRace2: %s\n\r", 
	( deity->race < 0 || deity->race > MAX_PC_RACE ) ? "none" : race_table[deity->race]->race_name,
	( deity->class < 0 || deity->class > MAX_PC_CLASS ) ? "none" : class_table[deity->class]->who_name,
	deity->sex == -1	  ? "none"   :
        deity->sex == SEX_MALE    ? "male"   :
        deity->sex == SEX_FEMALE  ? "female" : "neutral",
	( deity->race2 < 0 || deity->race2 > MAX_PC_RACE ) ? "none" : npc_race[deity->race2] );
    ch_printf( ch, "Objstat: %-9dWorshippers: %d\n\r", 
	deity->objstat, deity->worshippers );
    ch_printf( ch, "Affectednum: %-5dElementnum: %-6dSusceptnum: %d\n\r", deity->affectednum, deity->elementnum, deity->susceptnum );
    ch_printf( ch, "\n\rAffected: %s\n\r", affect_bit_name(&deity->affected) );
    ch_printf( ch, "Suscept: %s\n\r", flag_string( deity->suscept, ris_flags ));
    ch_printf( ch, "Element: %s\n\r", flag_string( deity->element, ris_flags ));
    ch_printf( ch, "\n\rFlee: %-12dFlee_npcrace: %-4dKill_npcrace: "
	"%-4dKill: %d\n\r", deity->flee, deity->flee_npcrace,
	deity->kill_npcrace, deity->kill );
    ch_printf( ch, "Kill_magic: %-6dSac: %-13dBury_corpse: %-5dAid_spell: "
 	"%d\n\r", deity->kill_magic, deity->sac, deity->bury_corpse, 
	deity->aid_spell );
    ch_printf( ch, "Aid: %-13dSteal: %-11dBackstab: %-8dDie: %d\n\r", 
	deity->aid, deity->steal, deity->backstab, deity->die );
    ch_printf( ch, "Die_npcrace: %-5dDig_corpse: %-6dSpell_aid: %-7dKill_npcfoe: %d\n\r",
	deity->die_npcrace, deity->dig_corpse, deity->spell_aid, deity->kill_npcfoe );
    ch_printf( ch, "Die_npcfoe: %-6dFlee_npcfoe: %d\n\r", 
	deity->die_npcfoe, deity->flee_npcfoe );
    ch_printf( ch, "\n\rScorpse: %-9dSavatar: %-9dSdeityobj: %-7d"
	"Srecall: %d\n\r", deity->scorpse, deity->savatar,
	deity->sdeityobj, deity->srecall );
    return;
}

void do_makedeity( CHAR_DATA *ch, char *argument )
{
    char filename[256];
    DEITY_DATA *deity;
    bool found;

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "Usage: makedeity <deity name>\n\r", ch );
	return;
    }

    found = FALSE;
    sprintf( filename, "%s%s", DEITY_DIR, strlower(argument) );
    CREATE( deity, DEITY_DATA, 1 );
    LINK( deity, first_deity, last_deity, next, prev );
    deity->name		= STRALLOC( argument );
    deity->description  = STRALLOC( "" );
    deity->filename	= str_dup( filename );
    write_deity_list();
    save_deity(deity);

}

void do_devote( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DEITY_DATA *deity;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( ch->level < 5 )
    {
	send_to_char( "You are not yet prepared for such devotion.\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Devote yourself to which deity?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "none" ) )
    {
        AFFECT_DATA af;
        if ( !ch->pcdata->deity )
        {
           send_to_char( "You have already chosen to worship no deities.\n\r", ch );
           return;
        }
        --ch->pcdata->deity->worshippers;
        ch->pcdata->favor = -2500;
	ch->mental_state = -80;
        send_to_char( "A terrible curse afflicts you as you forsake a deity!\n\r", ch );
	xREMOVE_BITS( ch->affected_by, ch->pcdata->deity->affected );
	REMOVE_BIT( ch->resistant, ch->pcdata->deity->element );
	REMOVE_BIT( ch->susceptible, ch->pcdata->deity->suscept );
	affect_strip( ch, gsn_blindness );
        af.type         = gsn_blindness;
        af.location     = APPLY_HITROLL;
        af.modifier     = -4;
        af.duration     = 50 * DUR_CONV;
        af.bitvector    = meb(AFF_BLIND);
        affect_to_char( ch, &af );
        save_deity(ch->pcdata->deity);
        send_to_char( "You cease to worship any deity.\n\r", ch );
        ch->pcdata->deity = NULL;
        STRFREE(ch->pcdata->deity_name);
        ch->pcdata->deity_name = STRALLOC( "" );
        save_char_obj( ch );
        return;
    }

    deity = get_deity( arg );
    if ( !deity )
    {
	send_to_char( "No such deity holds weight on this world.\n\r", ch );
	return;
    }

    if ( ch->pcdata->deity )
    {
	send_to_char( "You are already devoted to a deity.\n\r", ch );
	return;
    }

    if ( ( deity->class != -1 )
    && ( deity->class != ch->class ) )
    {
	send_to_char( "That deity will not accept your worship due to your class.\n\r", ch );
	return;
    }

    if ( ( deity->sex != -1 )
    && ( deity->sex != ch->sex ) )
    {
	send_to_char( "That deity will not accept worshippers of your sex.\n\r", ch );
	return;
    }

    if ( ( deity->race == -1 ) && ( deity->race2 == -1 ) );    
    else
    {
    if ( ( deity->race != ch->race ) && ( deity->race2 != ch->race ) )
    {
	send_to_char( "That deity will not accept worshippers of your race.\n\r", ch );
	return;
    }
    }
/*
    if ( ch->pcdata->deity )
    {
	AFFECT_DATA af;
	--ch->pcdata->deity->worshippers;
	ch->pcdata->favor = -2500;
	send_to_char( "A terrible curse afflicts you as you forsake a deity!\n\r", ch );
	xREMOVE_BITs( ch->affected_by, ch->pcdata->deity->affected );
	REMOVE_BIT( ch->resistant, ch->pcdata->deity->element );

	affect_strip( ch, gsn_blindness );
	af.type		= gsn_blindness;
	af.location	= APPLY_HITROLL;
	af.modifier	= -4;
	af.duration	= 50 * DUR_CONV;
	af.bitvector	= AFF_BLIND;
	affect_to_char( ch, &af );
	save_deity(ch->pcdata->deity);
    }	
*/

    STRFREE(ch->pcdata->deity_name);
    ch->pcdata->deity_name = QUICKLINK( deity->name );
    ch->pcdata->deity = deity;
    if ( ch->pcdata->favor > deity->affectednum )
    {
	 xSET_BITS( ch->affected_by, ch->pcdata->deity->affected );
    }
    if ( ch->pcdata->favor > deity->elementnum )
    {
	SET_BIT( ch->resistant, ch->pcdata->deity->element );
    }
    if ( ch->pcdata->favor < deity->susceptnum )
    {
	SET_BIT( ch->susceptible, ch->pcdata->deity->suscept );
    }
    act( AT_MAGIC, "Body and soul, you devote yourself to $t!", ch, ch->pcdata->deity_name, NULL, TO_CHAR );
    ++ch->pcdata->deity->worshippers;
    save_deity(ch->pcdata->deity);
    save_char_obj( ch );
    return;
}


void do_deities( CHAR_DATA *ch, char *argument )
{
    DEITY_DATA *deity;
    int count = 0;

    if ( argument[0] == '\0' )
    {
	send_to_pager_color( "&gFor detailed information on a deity, try 'deities <deity>' or 'help deities'\n\r", ch );
	send_to_pager_color( "Deity			Worshippers\n\r", ch );
	for ( deity = first_deity; deity; deity = deity->next )
	{
	   pager_printf_color( ch, "&G%-14s	&g%19d\n\r", deity->name, deity->worshippers );
	   count++;
	}
	if ( !count )
	{
	    send_to_pager_color( "&gThere are no deities on this world.\n\r", ch );
	    return;
	}
        return;
     }

    deity = get_deity( argument );
    if ( !deity )
    {
	send_to_pager_color( "&gThat deity does not exist.\n\r", ch );
	return;
    }

    pager_printf_color( ch, "&gDeity:        &G%s\n\r", deity->name );
    pager_printf_color( ch, "&gDescription:\n\r&G%s", deity->description );
    return;
}

void do_supplicate( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int oldfavor;
    
    one_argument( argument, arg );
    if ( IS_NPC(ch) || !ch->pcdata->deity )
    {
	send_to_char( "You have no deity to supplicate to.\n\r", ch );
	return;
    }

    oldfavor = ch->pcdata->favor;

    if ( arg[0] == '\0' )
    {
	send_to_char( "Supplicate for what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "corpse" ) )
    {
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	bool found;
        bool retr = FALSE;
        
	if ( ch->pcdata->favor < ch->pcdata->deity->scorpse )
	{
	   send_to_char( "You are not favored enough for a corpse retrieval.\n\r", ch );
	   return;
	}

	if ( xIS_SET( ch->in_room->room_flags, ROOM_CLANSTOREROOM ) )
	{
	   send_to_char( "You cannot supplicate in a storage room.\n\r", ch );
	   return;
	}

	found = FALSE;
	sprintf(buf3, " ");
	sprintf(buf2,"the corpse of %s", ch->name);
	for ( obj = first_object; obj; obj = obj->next )
	{
	   if ( obj->in_room
	   && !str_cmp( buf2, obj->short_descr )
	   && (obj->pIndexData->vnum == 11 ) )
	   {
		found = TRUE;
                if ( IS_PKILL( ch ) && obj->timer > 19 )
                {
                   if ( retr )
                     ch->pcdata->favor -= ch->pcdata->deity->scorpse;
                   send_to_char( "So soon?  Have patience...\n\r", ch );
                   return;
                }
                                                                                                    
		if ( xIS_SET( obj->in_room->room_flags, ROOM_NOSUPPLICATE ) )
		{
		  act( AT_MAGIC, "The image of your corpse appears, but suddenly wavers away.", ch, NULL, NULL, TO_CHAR );
		  return;
		}
		act( AT_MAGIC, "Your corpse appears suddenly, surrounded by a divine presence...", ch, NULL, NULL, TO_CHAR );		
		act( AT_MAGIC, "$n's corpse appears suddenly, surrounded by a divine force...", ch, NULL, NULL, TO_ROOM );
		obj_from_room(obj);
		/* what kinda deity doesn't unbury your corpse? :P  Eddy */
		if ( IS_OBJ_STAT( obj, ITEM_BURIED ) )
		   xREMOVE_BIT( obj->extra_flags, ITEM_BURIED );
		obj = obj_to_room(obj, ch->in_room);
                retr = TRUE;
	   }
	}

	if ( !found )
	{
	   send_to_char( "No corpse of yours litters the world...\n\r", ch );
	   return;
	}
	ch->pcdata->favor -= ch->pcdata->deity->scorpse;

        if(ch->pcdata->favor < ch->pcdata->deity->susceptnum)
                SET_BIT(ch->susceptible, ch->pcdata->deity->suscept);

        if((oldfavor > ch->pcdata->deity->affectednum &&
            ch->pcdata->favor <= ch->pcdata->deity->affectednum) ||
           (oldfavor > ch->pcdata->deity->elementnum &&
            ch->pcdata->favor <= ch->pcdata->deity->elementnum) ||
           (oldfavor < ch->pcdata->deity->susceptnum &&
            ch->pcdata->favor >= ch->pcdata->deity->susceptnum))
        {
                update_aris(ch);
        }
	
	return;
   }

   if ( !str_cmp( arg, "avatar" ) )
   {
	MOB_INDEX_DATA *pMobIndex;
	CHAR_DATA *victim;

        if ( ch->pcdata->favor < ch->pcdata->deity->savatar )
        {
           send_to_char( "You are not favored enough for that.\n\r", ch );
           return;
        }
 
        pMobIndex = get_mob_index( MOB_VNUM_DEITY );
        victim = create_mobile( pMobIndex );
        char_to_room( victim, ch->in_room );

        sprintf( buf, victim->short_descr, ch->pcdata->deity->name );
        STRFREE( victim->short_descr );
        victim->short_descr = STRALLOC( buf );
 
	sprintf( buf, victim->long_descr, ch->pcdata->deity->name );
	STRFREE( victim->long_descr );
	victim->long_descr = STRALLOC( buf );

        sprintf( buf, victim->description, ch->pcdata->deity->name );
        STRFREE( victim->description );
        victim->description = STRALLOC( buf );	
        

        act( AT_MAGIC, "$n summons a powerful avatar!", ch, NULL, NULL, TO_ROOM );
        act( AT_MAGIC, "You summon a powerful avatar!", ch, NULL, NULL, TO_CHAR );
	add_follower( victim, ch );
	xSET_BIT( victim->affected_by, AFF_CHARM );
	victim->level = 10;
	victim->hit = ch->hit*6 + ch->pcdata->favor;
	victim->alignment = ch->pcdata->deity->alignment;
	victim->max_hit = ch->hit*6 + ch->pcdata->favor;

	ch->pcdata->favor -= ch->pcdata->deity->savatar;
        
        if(ch->pcdata->favor < ch->pcdata->deity->susceptnum)
        	SET_BIT(ch->susceptible, ch->pcdata->deity->suscept);
        
	if((oldfavor > ch->pcdata->deity->affectednum &&
            ch->pcdata->favor <= ch->pcdata->deity->affectednum) ||
           (oldfavor > ch->pcdata->deity->elementnum &&
            ch->pcdata->favor <= ch->pcdata->deity->elementnum) ||
           (oldfavor < ch->pcdata->deity->susceptnum &&
            ch->pcdata->favor >= ch->pcdata->deity->susceptnum))        
        {
        	update_aris(ch);
        }
        
        return;
   } 

   if ( !str_cmp( arg, "object" ) )
   {
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *pObjIndex;
	AFFECT_DATA *paf;

	if ( ch->pcdata->favor < ch->pcdata->deity->sdeityobj )
	{
	   send_to_char( "You are not favored enough for that.\n\r", ch );
	   return;
	}

	pObjIndex = get_obj_index( OBJ_VNUM_DEITY );
		
	obj = create_object( pObjIndex, ch->level );
	if ( CAN_WEAR(obj, ITEM_TAKE) )
	   obj = obj_to_char( obj, ch );
	else
	   obj = obj_to_room( obj, ch->in_room );

	sprintf( buf, "sigil %s", ch->pcdata->deity->name );
        STRFREE( obj->name );
        obj->name = STRALLOC( buf );
 
        sprintf( buf, obj->short_descr, ch->pcdata->deity->name );
        STRFREE( obj->short_descr );
        obj->short_descr = STRALLOC( buf );
 
        sprintf( buf, obj->description, ch->pcdata->deity->name );
        STRFREE( obj->description );
        obj->description = STRALLOC( buf );

	act( AT_MAGIC, "$n weaves $p from divine matter!", ch, obj, NULL, TO_ROOM );
	act( AT_MAGIC, "You weave $p from divine matter!", ch, obj, NULL, TO_CHAR );
  	ch->pcdata->favor -= ch->pcdata->deity->sdeityobj;

	if(ch->pcdata->favor < ch->pcdata->deity->susceptnum)
		SET_BIT(ch->susceptible, ch->pcdata->deity->suscept);

	if((oldfavor > ch->pcdata->deity->affectednum &&
            ch->pcdata->favor <= ch->pcdata->deity->affectednum) ||
           (oldfavor > ch->pcdata->deity->elementnum &&
            ch->pcdata->favor <= ch->pcdata->deity->elementnum) ||
           (oldfavor < ch->pcdata->deity->susceptnum &&
            ch->pcdata->favor >= ch->pcdata->deity->susceptnum))
	{
		update_aris(ch);
	}
	
        CREATE( paf, AFFECT_DATA, 1 );
        paf->type           = -1;
        paf->duration       = -1;
	switch( ch->pcdata->deity->objstat )
	{
	case 0: paf->location = APPLY_STR; break;
	case 1: paf->location = APPLY_INT; break;
	case 2: paf->location = APPLY_WIS; break;
	case 3: paf->location = APPLY_CON; break;
	case 4: paf->location = APPLY_DEX; break;
	case 5: paf->location = APPLY_CHA; break;
	case 6: paf->location = APPLY_LCK; break;
	}
        paf->modifier       = 1;
 	xCLEAR_BITS(paf->bitvector);
        LINK( paf, obj->first_affect, obj->last_affect, next, prev );

	return;
   }

   if ( !str_cmp( arg, "recall" ) )
   {
	ROOM_INDEX_DATA *location;

	if ( ch->pcdata->favor < ch->pcdata->deity->srecall )
	{
	   send_to_char( "Your favor is inadequate for such a supplication.\n\r", ch );
 	   return;
	}

        if ( xIS_SET( ch->in_room->room_flags, ROOM_NOSUPPLICATE ) || xIS_SET( ch->in_room->room_flags, ROOM_NS_RECALL ) )
        {
           send_to_char( "You have been forsaken!\n\r", ch );
           return;
        }

	if ( get_timer(ch, TIMER_RECENTFIGHT) > 0 && !IS_IMMORTAL(ch) )
        {
          send_to_char( "You cannot supplicate recall under adrenaline!\n\r", ch );
          return;
        }

	location = NULL;
 
	if ( !IS_NPC(ch) && ch->pcdata->clan )
      	  location = get_room_index( ch->pcdata->clan->recall );
 
	if ( !IS_NPC( ch ) && !location && ch->level >= 5
    	&&   IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
       	  location = get_room_index( 3009 );

	if ( !location )
       	  location = get_room_index( ROOM_VNUM_TEMPLE );
 
    	if ( !location )
    	{
          send_to_char( "You are completely lost.\n\r", ch );
          return;
	}

    	act( AT_MAGIC, "$n disappears in a column of divine power.", ch, NULL, NULL, TO_ROOM );
    	char_from_room( ch );
    	char_to_room( ch, location );
    	if ( ch->mount )
    	{
            char_from_room( ch->mount );
            char_to_room( ch->mount, location );
    	}
    	act( AT_MAGIC, "$n appears in the room from a column of divine mist.", ch, NULL, NULL, TO_ROOM );
    	do_look( ch, "auto" );
    	ch->pcdata->favor -= ch->pcdata->deity->srecall; 

	if(ch->pcdata->favor < ch->pcdata->deity->susceptnum)
		SET_BIT(ch->susceptible, ch->pcdata->deity->suscept);

	if((oldfavor > ch->pcdata->deity->affectednum &&
            ch->pcdata->favor <= ch->pcdata->deity->affectednum) ||
           (oldfavor > ch->pcdata->deity->elementnum &&
            ch->pcdata->favor <= ch->pcdata->deity->elementnum) ||
           (oldfavor < ch->pcdata->deity->susceptnum &&
            ch->pcdata->favor >= ch->pcdata->deity->susceptnum))
	{
		update_aris(ch);
	}
	
    	return;
    }

    send_to_char( "You cannot supplicate for that.\n\r", ch );
    return;
}

/*
Internal function to adjust favor.
Fields are:
0 = flee		5 = sac			10 = backstab	
1 = flee_npcrace	6 = bury_corpse		11 = die
2 = kill		7 = aid_spell		12 = die_npcrace
3 = kill_npcrace	8 = aid			13 = spell_aid
4 = kill_magic		9 = steal		14 = dig_corpse
15 = die_npcfoe	       16 = flee_npcfoe         17 = kill_npcfoe
*/
void  adjust_favor( CHAR_DATA *ch, int field, int mod )
{
   int oldfavor;
   
   if ( IS_NPC( ch )
   ||   !ch->pcdata->deity )   
	return;

   oldfavor = ch->pcdata->favor;

   if ( (ch->alignment - ch->pcdata->deity->alignment > 650
   ||    ch->alignment - ch->pcdata->deity->alignment < -650)
   &&	 ch->pcdata->deity->alignment != 0 )
     {	
	ch->pcdata->favor -= 2;
	ch->pcdata->favor = URANGE( -2500, ch->pcdata->favor, 2500 );
	
	if(ch->pcdata->favor > ch->pcdata->deity->affectednum)
		xSET_BITS(ch->affected_by, ch->pcdata->deity->affected);
	if(ch->pcdata->favor > ch->pcdata->deity->elementnum)
		SET_BIT(ch->resistant, ch->pcdata->deity->element);
	if(ch->pcdata->favor < ch->pcdata->deity->susceptnum)
		SET_BIT(ch->susceptible, ch->pcdata->deity->suscept);
	
	/* If favor crosses over the line then strip the affect */
	if((oldfavor > ch->pcdata->deity->affectednum &&
	    ch->pcdata->favor <= ch->pcdata->deity->affectednum) ||
	   (oldfavor > ch->pcdata->deity->elementnum &&
	    ch->pcdata->favor <= ch->pcdata->deity->elementnum) ||
	   (oldfavor < ch->pcdata->deity->susceptnum &&
	    ch->pcdata->favor >= ch->pcdata->deity->susceptnum))
	{
		update_aris(ch);
	}

	return;
     }

   if ( mod < 1 ) mod = 1;
   switch ( field )
   {
	case 0:  ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->flee / mod);	break;
	case 1:  ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->flee_npcrace / mod);	 break;
	case 2:  ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->kill / mod);	break;
	case 3:  ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->kill_npcrace / mod);	 break;
	case 4:  ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->kill_magic / mod);	 break;
	case 5:  ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->sac / mod);	break;
	case 6:  ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->bury_corpse / mod);	 break;
	case 7:  ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->aid_spell / mod);	 break;
	case 8:  ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->aid / mod);	break;	
	case 9:  ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->steal / mod);	break;
	case 10: ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->backstab / mod); break;
	case 11: ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->die / mod);	break;
	case 12: ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->die_npcrace / mod);	 break;
	case 13: ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->spell_aid / mod);	 break;
	case 14: ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->dig_corpse / mod);	 break;
	case 15: ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->die_npcfoe / mod);	 break;
	case 16: ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->flee_npcfoe / mod);	 break;
	case 17: ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->kill_npcfoe / mod);	 break;
   }
   ch->pcdata->favor = URANGE( -2500, ch->pcdata->favor, 2500 );

   if(ch->pcdata->favor > ch->pcdata->deity->affectednum)
   	xSET_BITS(ch->affected_by, ch->pcdata->deity->affected);
   if(ch->pcdata->favor > ch->pcdata->deity->elementnum)
   	SET_BIT(ch->resistant, ch->pcdata->deity->element);
   if(ch->pcdata->favor < ch->pcdata->deity->susceptnum)
   	SET_BIT(ch->susceptible, ch->pcdata->deity->suscept);
   
   /* If favor crosses over line then strip the affect */
   if((oldfavor > ch->pcdata->deity->affectednum &&
       ch->pcdata->favor <= ch->pcdata->deity->affectednum) ||
      (oldfavor > ch->pcdata->deity->elementnum &&
       ch->pcdata->favor <= ch->pcdata->deity->elementnum) ||
      (oldfavor < ch->pcdata->deity->susceptnum &&
       ch->pcdata->favor >= ch->pcdata->deity->susceptnum))
   {
   	update_aris(ch);
   }
   
   return;
}
