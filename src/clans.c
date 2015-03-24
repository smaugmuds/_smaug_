/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops, Fireblade, Edmond, Conran                         |             *
 * ------------------------------------------------------------------------ *
 *			     Special clan module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* #include <stdlib.h> */
#include <time.h>
#include "mud.h"



CLAN_DATA * first_clan;
CLAN_DATA * last_clan;
COUNCIL_DATA * first_council;
COUNCIL_DATA * last_council;
MEMBER_LIST * first_member_list;
MEMBER_LIST * last_member_list;


/* local routines */
void	fread_clan	args( ( CLAN_DATA *clan, FILE *fp ) );
bool	load_clan_file	args( ( char *clanfile ) );
void	write_clan_list	args( ( void ) );

void	fread_council	args( ( COUNCIL_DATA *council, FILE *fp ) );
bool	load_council_file	args( ( char *councilfile ) );
void	write_council_list	args( ( void ) );

void	save_member_lists	args( ( void ) );
void	show_members		args( ( CHAR_DATA *ch, char *argument ) );
void	remove_member		args( ( char *clanname, char *membername ) );

void	add_member		args( ( CHAR_DATA *ch, char* clanname ) );
void	save_member_lists	args( ( void ) );
void	fread_member_list	args( ( FILE *fp ) );


/*
 * Get pointer to clan structure from clan name.
 */
CLAN_DATA *get_clan( char *name )
{
    CLAN_DATA *clan;

    for ( clan = first_clan; clan; clan = clan->next )
       if ( !str_cmp( name, clan->name )
		|| ( ( clan->abbrev!='\0' ) && !str_cmp( name, clan->abbrev ) ) )
         return clan;
    return NULL;
}

/*
CLAN_DATA *get_clan( char *name )
{
    CLAN_DATA *clan;
    
    for ( clan = first_clan; clan; clan = clan->next )
       if ( !str_cmp( name, clan->name ) )
         return clan;
    return NULL;
}
*/


COUNCIL_DATA *get_council( char *name )
{
    COUNCIL_DATA *council;

    for ( council = first_council; council; council = council->next )
       if ( !str_cmp( name, council->name )
		/* council_short */
		 || ( ( council->abbrev!='\0' ) && !str_cmp( name, council->abbrev ) ) )
          return council;
    return NULL;
}


/*
COUNCIL_DATA *get_council( char *name )
{
    COUNCIL_DATA *council;
    
    for ( council = first_council; council; council = council->next )
       if ( !str_cmp( name, council->name ) )
         return council;
    return NULL;
}
*/


void write_clan_list( )
{
    CLAN_DATA *tclan;
    FILE *fpout;
    char filename[256];

    sprintf( filename, "%s%s", CLAN_DIR, CLAN_LIST );
    fpout = fopen( filename, "w" );
    if ( !fpout )
    {
	bug( "FATAL: cannot open clan.lst for writing!\n\r", 0 );
 	return;
    }	  
    for ( tclan = first_clan; tclan; tclan = tclan->next )
	fprintf( fpout, "%s\n", tclan->filename );
    fprintf( fpout, "$\n" );
    fclose( fpout );
}

void write_council_list( )
{
    COUNCIL_DATA *tcouncil;
    FILE *fpout;
    char filename[256];

    sprintf( filename, "%s%s", COUNCIL_DIR, COUNCIL_LIST );
    fpout = fopen( filename, "w" );
    if ( !fpout )
    {
	bug( "FATAL: cannot open council.lst for writing!\n\r", 0 );
 	return;
    }	  
    for ( tcouncil = first_council; tcouncil; tcouncil = tcouncil->next )
	fprintf( fpout, "%s\n", tcouncil->filename );
    fprintf( fpout, "$\n" );
    fclose( fpout );
}

/*
 * Save a clan's data to its data file
 */
void save_clan( CLAN_DATA *clan )
{
    FILE *fp;
    char filename[256];
    char buf[MAX_STRING_LENGTH];

    if ( !clan )
    {
	bug( "save_clan: null clan pointer!", 0 );
	return;
    }
        
    if ( !clan->filename || clan->filename[0] == '\0' )
    {
	sprintf( buf, "save_clan: %s has no filename", clan->name );
	bug( buf, 0 );
	return;
    }
    
    sprintf( filename, "%s%s", CLAN_DIR, clan->filename );
    
    fclose( fpReserve );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
    	bug( "save_clan: fopen", 0 );
    	perror( filename );
    }
    else
    {
	fprintf( fp, "#CLAN\n" );
	fprintf( fp, "Name         %s~\n",	clan->name		);
	fprintf( fp, "Abbrev       %s~\n",	clan->abbrev		);
	fprintf( fp, "Filename     %s~\n",	clan->filename		);
	fprintf( fp, "Motto        %s~\n",	clan->motto		);
	fprintf( fp, "Description  %s~\n",	clan->description	);
	fprintf( fp, "Deity        %s~\n",	clan->deity		);
	fprintf( fp, "Leader       %s~\n",	clan->leader		);
	fprintf( fp, "NumberOne    %s~\n",	clan->number1		);
	fprintf( fp, "NumberTwo    %s~\n",	clan->number2		);
	fprintf( fp, "Badge        %s~\n",	clan->badge		);
	fprintf( fp, "Leadrank     %s~\n",	clan->leadrank		);
	fprintf( fp, "Onerank      %s~\n",	clan->onerank		);
	fprintf( fp, "Tworank      %s~\n",	clan->tworank		);
	fprintf( fp, "PKillRangeNew   %d %d %d %d %d %d %d\n",
		clan->pkills[0], clan->pkills[1], clan->pkills[2],
		clan->pkills[3], clan->pkills[4], clan->pkills[5],
		clan->pkills[6]);
	fprintf( fp, "PDeathRangeNew  %d %d %d %d %d %d %d\n",	
		clan->pdeaths[0], clan->pdeaths[1], clan->pdeaths[2],
		clan->pdeaths[3], clan->pdeaths[4], clan->pdeaths[5],
		clan->pdeaths[6]);
	fprintf( fp, "MKills       %d\n",	clan->mkills		);
	fprintf( fp, "MDeaths      %d\n",	clan->mdeaths		);
	fprintf( fp, "IllegalPK    %d\n",	clan->illegal_pk	);
	fprintf( fp, "Score        %d\n",	clan->score		);
	fprintf( fp, "Type         %d\n",	clan->clan_type		);
	fprintf( fp, "Class        %d\n",	clan->class		);
	fprintf( fp, "Favour       %d\n",	clan->favour		);
	fprintf( fp, "Strikes      %d\n",	clan->strikes		);
	fprintf( fp, "Members      %d\n",	clan->members		);
	fprintf( fp, "MemLimit     %d\n",	clan->mem_limit		);
	fprintf( fp, "Alignment    %d\n",	clan->alignment		);
	fprintf( fp, "Board        %d\n",	clan->board		);
	fprintf( fp, "ClanObjOne   %d\n",	clan->clanobj1		);
	fprintf( fp, "ClanObjTwo   %d\n",	clan->clanobj2		);
	fprintf( fp, "ClanObjThree %d\n",	clan->clanobj3		);
        fprintf( fp, "ClanObjFour  %d\n",	clan->clanobj4		);
	fprintf( fp, "ClanObjFive  %d\n", 	clan->clanobj5		);
	fprintf( fp, "Recall       %d\n",	clan->recall		);
	fprintf( fp, "Storeroom    %d\n",	clan->storeroom		);
	fprintf( fp, "GuardOne     %d\n",	clan->guard1		);
	fprintf( fp, "GuardTwo     %d\n",	clan->guard2		);
	fprintf( fp, "End\n\n"						);
	fprintf( fp, "#END\n"						);
    }
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

/*
 * Save a council's data to its data file
 */
void save_council( COUNCIL_DATA *council )
{
    FILE *fp;
    char filename[256];
    char buf[MAX_STRING_LENGTH];

    if ( !council )
    {
	bug( "save_council: null council pointer!", 0 );
	return;
    }
        
    if ( !council->filename || council->filename[0] == '\0' )
    {
	sprintf( buf, "save_council: %s has no filename", council->name );
	bug( buf, 0 );
	return;
    }
 
    sprintf( filename, "%s%s", COUNCIL_DIR, council->filename );
    
    fclose( fpReserve );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
    	bug( "save_council: fopen", 0 );
    	perror( filename );
    }
    else
    {
	fprintf( fp, "#COUNCIL\n" );
	fprintf( fp, "Name         %s~\n",	council->name		);
	fprintf( fp, "Filename     %s~\n",	council->filename	);
	fprintf( fp, "Description  %s~\n",	council->description	);
	fprintf( fp, "Head         %s~\n",	council->head		);
      	if ( council->head2 != NULL)
        	fprintf (fp, "Head2        %s~\n", council->head2);
	fprintf( fp, "Members      %d\n",	council->members	);
	fprintf( fp, "Board        %d\n",	council->board		);
	fprintf( fp, "Meeting      %d\n",	council->meeting	);
        fprintf( fp, "Storeroom    %d\n",	council->storeroom	);
	fprintf( fp, "Powers       %s~\n",	council->powers		);
	fprintf( fp, "Abbrev       %s~\n",	council->abbrev		);
	fprintf( fp, "End\n\n"						);
	fprintf( fp, "#END\n"						);
    }
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}


/*
 * Read in actual clan data.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

/*
 * Reads in PKill and PDeath still for backward compatibility but now it
 * should be written to PKillRange and PDeathRange for multiple level pkill
 * tracking support. --Shaddai
 * Added a hardcoded limit memlimit to the amount of members a clan can 
 * have set using setclan.  --Shaddai
 */

void fread_clan( CLAN_DATA *clan, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;

    clan->mem_limit = 0;  /* Set up defaults */
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
	    KEY( "Abbrev", 		clan->abbrev,   fread_string( fp ) );
	    KEY( "Alignment",	clan->alignment,	fread_number( fp ) );
	    break;

	case 'B':
            KEY( "Badge",       clan->badge,            fread_string( fp ) );
	    KEY( "Board",	clan->board,		fread_number( fp ) );
	    break;

	case 'C':
	    KEY( "ClanObjOne",	clan->clanobj1,		fread_number( fp ) );
	    KEY( "ClanObjTwo",	clan->clanobj2,		fread_number( fp ) );
	    KEY( "ClanObjThree",clan->clanobj3,		fread_number( fp ) );
            KEY( "ClanObjFour", clan->clanobj4,         fread_number( fp ) );
            KEY( "ClanObjFive", clan->clanobj5,         fread_number( fp ) );
	    KEY( "Class",	clan->class,		fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Deity",	clan->deity,		fread_string( fp ) );
	    KEY( "Description",	clan->description,	fread_string( fp ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if (!clan->name)
		  clan->name		= STRALLOC( "" );
		if (!clan->leader)
		  clan->leader		= STRALLOC( "" );
		if (!clan->description)
		  clan->description 	= STRALLOC( "" );
		if (!clan->motto)
		  clan->motto		= STRALLOC( "" );
		if (!clan->number1)
		  clan->number1		= STRALLOC( "" );
		if (!clan->number2)
		  clan->number2		= STRALLOC( "" );
		if (!clan->deity)
		  clan->deity		= STRALLOC( "" );
		if (!clan->badge)
	  	  clan->badge		= STRALLOC( "" );
		if (!clan->leadrank)
		  clan->leadrank	= STRALLOC( "" );
		if (!clan->onerank)
		  clan->onerank		= STRALLOC( "" );
		if (!clan->tworank)
		  clan->tworank		= STRALLOC( "" );
		return;
	    }
	    break;
	    
	case 'F':
	    KEY( "Favour",	clan->favour,		fread_number( fp ) );
	    KEY( "Filename",	clan->filename,		fread_string_nohash( fp ) );

	case 'G':
	    KEY( "GuardOne",	clan->guard1,		fread_number( fp ) );
	    KEY( "GuardTwo",	clan->guard2,		fread_number( fp ) );
	    break;

	case 'I':
	    KEY( "IllegalPK",	clan->illegal_pk,	fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Leader",	clan->leader,		fread_string( fp ) );
	    KEY( "Leadrank",	clan->leadrank,		fread_string( fp ) );
	    break;

	case 'M':
	    KEY( "MDeaths",	clan->mdeaths,		fread_number( fp ) );
	    KEY( "Members",	clan->members,		fread_number( fp ) );
	    KEY( "MemLimit",	clan->mem_limit,	fread_number( fp ) );
	    KEY( "MKills",	clan->mkills,		fread_number( fp ) );
	    KEY( "Motto",	clan->motto,		fread_string( fp ) );
	    break;
 
	case 'N':
	    KEY( "Name",	clan->name,		fread_string( fp ) );
	    KEY( "NumberOne",	clan->number1,		fread_string( fp ) );
	    KEY( "NumberTwo",	clan->number2,		fread_string( fp ) );
	    break;

	case 'O':
	    KEY( "Onerank",	clan->onerank,		fread_string( fp ) );
	    break;

	case 'P':
	    KEY( "PDeaths",	clan->pdeaths[6],	fread_number( fp ) );
	    KEY( "PKills",	clan->pkills[6],	fread_number( fp ) );
	    /* Addition of New Ranges */
	    if ( !str_cmp ( word, "PDeathRange" ) )
	    {
		fMatch = TRUE;
		fread_number( fp );
		fread_number( fp );
		fread_number( fp );
		fread_number( fp );
		fread_number( fp );
		fread_number( fp );
		fread_number( fp );
            }
	    if ( !str_cmp ( word, "PDeathRangeNew" ) )
            {
		fMatch = TRUE;
		clan->pdeaths[0] = fread_number( fp );
		clan->pdeaths[1] = fread_number( fp );
		clan->pdeaths[2] = fread_number( fp );
		clan->pdeaths[3] = fread_number( fp );
		clan->pdeaths[4] = fread_number( fp );
		clan->pdeaths[5] = fread_number( fp );
		clan->pdeaths[6] = fread_number( fp );
	    }
	    if ( !str_cmp ( word, "PKillRangeNew" ) )
            {
		fMatch = TRUE;
		clan->pkills[0] = fread_number( fp );
		clan->pkills[1] = fread_number( fp );
		clan->pkills[2] = fread_number( fp );
		clan->pkills[3] = fread_number( fp );
		clan->pkills[4] = fread_number( fp );
		clan->pkills[5] = fread_number( fp );
		clan->pkills[6] = fread_number( fp );
	    }
	    if ( !str_cmp ( word, "PKillRange" ) )
	    {
		fMatch = TRUE;
		fread_number( fp );
		fread_number( fp );
		fread_number( fp );
		fread_number( fp );
		fread_number( fp );
		fread_number( fp );
		fread_number( fp );
            }
	    break;

	case 'R':
	    KEY( "Recall",	clan->recall,		fread_number( fp ) );
	    break;

	case 'S':
	    KEY( "Score",	clan->score,		fread_number( fp ) );
	    KEY( "Strikes",	clan->strikes,		fread_number( fp ) );
	    KEY( "Storeroom",	clan->storeroom,	fread_number( fp ) );
	    break;

	case 'T':
	    KEY( "Tworank",	clan->tworank,		fread_string( fp ) );
	    KEY( "Type",	clan->clan_type,	fread_number( fp ) );
	    break;
	}
	
	if ( !fMatch )
	{
	    sprintf( buf, "Fread_clan: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}

/*
 * Read in actual council data.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void fread_council( COUNCIL_DATA *council, FILE *fp )
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
		KEY( "Abbrev", council->abbrev,		fread_string( fp ) );
		break;

	case 'B':
	    KEY( "Board",	council->board,		fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Description",	council->description,	fread_string( fp ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if (!council->name)
		  council->name		= STRALLOC( "" );
		if (!council->description)
		  council->description 	= STRALLOC( "" );
		if (!council->powers)
		  council->powers	= STRALLOC( "" );
		return;
	    }
	    break;
	    
	case 'F':
	    KEY( "Filename",	council->filename,	fread_string_nohash( fp ) );
  	    break;

	case 'H':
	    KEY( "Head", 	council->head, 		fread_string( fp ) );
            KEY ("Head2", 	council->head2, 	fread_string( fp ) );
	    break;

	case 'M':
	    KEY( "Members",	council->members,	fread_number( fp ) );
	    KEY( "Meeting",   	council->meeting, 	fread_number( fp ) );
	    break;
 
	case 'N':
	    KEY( "Name",	council->name,		fread_string( fp ) );
	    break;

	case 'P':
	    KEY( "Powers",	council->powers,	fread_string( fp ) );
	    break;
        case 'S':
            KEY( "Storeroom", council->storeroom, fread_number( fp ) );
            break;                                                                                                                              
	}
	
	if ( !fMatch )
	{
	    sprintf( buf, "Fread_council: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}


/*
 * Load a clan file
 */

bool load_clan_file( char *clanfile )
{
    char filename[256];
    CLAN_DATA *clan;
    FILE *fp;
    bool found;

    CREATE( clan, CLAN_DATA, 1 );

    /* Make sure we default these to 0 --Shaddai */
    clan->pkills[0] = 0;
    clan->pkills[1] = 0;
    clan->pkills[2] = 0;
    clan->pkills[3] = 0;
    clan->pkills[4] = 0;
    clan->pkills[5] = 0;
    clan->pkills[6] = 0;
    clan->pdeaths[0]= 0;
    clan->pdeaths[1]= 0;
    clan->pdeaths[2]= 0;
    clan->pdeaths[3]= 0;
    clan->pdeaths[4]= 0;
    clan->pdeaths[5]= 0;
    clan->pdeaths[6]= 0;

    found = FALSE;
    sprintf( filename, "%s%s", CLAN_DIR, clanfile );

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {

	found = TRUE;
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
		bug( "Load_clan_file: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "CLAN"	) )
	    {
	    	fread_clan( clan, fp );
	    	break;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
		char buf[MAX_STRING_LENGTH];

		sprintf( buf, "Load_clan_file: bad section: %s.", word );
		bug( buf, 0 );
		break;
	    }
	}
	fclose( fp );
    }

    if ( found )
    {
        char fname[256];
        VAULT_DATA *vault;                                                                                                                      

        LINK( clan, first_clan, last_clan, next, prev );    

        for ( vault = first_vault; vault; vault = vault->next )
        if (clan->storeroom == vault->vnum)
                return found;                                                                                                                   

        sprintf( fname, "%s%s.vault", CLAN_DIR, clan->filename );
        fread_storage( clan->storeroom, fname );
    }
    else
      DISPOSE( clan );                                                                                                                          

   return found;
}

/*
 * Load a council file
 */

bool load_council_file( char *councilfile )
{
    char filename[256];
    COUNCIL_DATA *council;
    FILE *fp;
    bool found;

    CREATE( council, COUNCIL_DATA, 1 );

    found = FALSE;
    sprintf( filename, "%s%s", COUNCIL_DIR, councilfile );

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {

	found = TRUE;
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
		bug( "Load_council_file: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "COUNCIL"	) )
	    {
	    	fread_council( council, fp );
	    	break;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
		bug( "Load_council_file: bad section.", 0 );
		break;
	    }
	}
	fclose( fp );
    }

    if ( found )
    {
        char fname[256];
        VAULT_DATA *vault;                                                                                                                      

        LINK( council, first_council, last_council, next, prev );                                                                               

        for ( vault = first_vault; vault; vault = vault->next )
        if (council->storeroom == vault->vnum)
                return found;                                                                                                                   

        sprintf( fname, "%s%s.vault", COUNCIL_DIR, council->filename );
        fread_storage( council->storeroom, fname );
        }
    else
      DISPOSE( council );    

    return found;
}

/*
 * Load in all the clan files.
 */
void load_clans( )
{
    FILE *fpList;
    char *filename;
    char clanlist[256];
    char buf[MAX_STRING_LENGTH];
    
    
    first_clan	= NULL;
    last_clan	= NULL;

    log_string( "Loading clans..." );

    sprintf( clanlist, "%s%s", CLAN_DIR, CLAN_LIST );
    fclose( fpReserve );
    if ( ( fpList = fopen( clanlist, "r" ) ) == NULL )
    {
	perror( clanlist );
	exit( 1 );
    }

    for ( ; ; )
    {
	filename = feof( fpList ) ? "$" : fread_word( fpList );
	log_string( filename );
	if ( filename[0] == '$' )
	  break;

	if ( !load_clan_file( filename ) )
	{
	  sprintf( buf, "Cannot load clan file: %s", filename );
	  bug( buf, 0 );
	}
    }
    fclose( fpList );
    log_string(" Done clans " );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

/*
 * Load in all the council files.
 */
void load_councils( )
{
    FILE *fpList;
    char *filename;
    char councillist[256];
    char buf[MAX_STRING_LENGTH];
    
    
    first_council	= NULL;
    last_council	= NULL;

    log_string( "Loading councils..." );

    sprintf( councillist, "%s%s", COUNCIL_DIR, COUNCIL_LIST );
    fclose( fpReserve );
    if ( ( fpList = fopen( councillist, "r" ) ) == NULL )
    {
	perror( councillist );
	exit( 1 );
    }

    for ( ; ; )
    {
	filename = feof( fpList ) ? "$" : fread_word( fpList );
	log_string( filename );
	if ( filename[0] == '$' )
	  break;

	if ( !load_council_file( filename ) )
	{
	  sprintf( buf, "Cannot load council file: %s", filename );
	  bug( buf, 0 );
	}
    }
    fclose( fpList );
    log_string(" Done councils " );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

void do_make( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    CLAN_DATA *clan;
    int level;

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    clan = ch->pcdata->clan;
    
    if ( str_cmp( ch->name, clan->leader )
    &&   str_cmp( ch->name, clan->deity )
    &&  (clan->clan_type != CLAN_GUILD
    ||   str_cmp( ch->name, clan->number1 )) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Make what?\n\r", ch );
	return;
    }

    pObjIndex = get_obj_index( clan->clanobj1 );
    level = 40;

    if ( !pObjIndex || !is_name( arg, pObjIndex->name ) )
    {
      pObjIndex = get_obj_index( clan->clanobj2 );
      level = 45;
    }
    if ( !pObjIndex || !is_name( arg, pObjIndex->name ) )
    {
      pObjIndex = get_obj_index( clan->clanobj3 );
      level = 50;
    }
    if ( !pObjIndex || !is_name( arg, pObjIndex->name ) )
    {
      pObjIndex = get_obj_index( clan->clanobj4 );
      level = 35;
    }
    if ( !pObjIndex || !is_name( arg, pObjIndex->name ) )
    {
      pObjIndex = get_obj_index( clan->clanobj5 );
      level = 1;
    }

    if ( !pObjIndex || !is_name( arg, pObjIndex->name ) )
    {
	send_to_char( "You don't know how to make that.\n\r", ch );
	return;
    }

    obj = create_object( pObjIndex, level );
    xSET_BIT( obj->extra_flags, ITEM_CLANOBJECT );
    if ( CAN_WEAR(obj, ITEM_TAKE) )
      obj = obj_to_char( obj, ch );
    else
      obj = obj_to_room( obj, ch->in_room );
    act( AT_MAGIC, "$n makes $p!", ch, obj, NULL, TO_ROOM );
    act( AT_MAGIC, "You make $p!", ch, obj, NULL, TO_CHAR );
    return;
}

void do_induct( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    clan = ch->pcdata->clan;
    
    if ( (ch->pcdata && ch->pcdata->bestowments
    &&    is_name("induct", ch->pcdata->bestowments))
    ||   !str_cmp( ch->name, clan->deity   )
    ||   !str_cmp( ch->name, clan->leader  )
    ||   !str_cmp( ch->name, clan->number1 )
    ||   !str_cmp( ch->name, clan->number2 ) )
	;
    else
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Induct whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( IS_IMMORTAL(victim) )
    {
	send_to_char( "You can't induct such a godly presence.\n\r", ch );
	return;
    }

    if ( !IS_PKILL( victim ) && clan->clan_type != CLAN_GUILD &&
	 clan->clan_type != CLAN_ORDER && clan->clan_type != CLAN_NOKILL)
    {
        send_to_char( "You cannot induct a peaceful character.\n\r", ch );
        return;
    }

/*
    if ( xIS_SET( victim->act, PLR_NICE ) )
    {
	send_to_char( "This character is too 'nice' to induct.\n\r", ch );
	return;
    }
*/

    if ( clan->clan_type == CLAN_GUILD )
    {
	if ( victim->class != clan->class)
	{
	    send_to_char( "This player's will is not in accordance with your guild.\n\r", ch);
            return;
	}
    }
    else
    {
	if ( victim->level < 10 )
	{
	    send_to_char( "This player is not worthy of joining yet.\n\r", ch );
	    return;
	}

	if ( victim->level > ch->level )
	{
	    send_to_char( "This player is too powerful for you to induct.\n\r", ch );
	    return;
	}
    }

    if ( victim->pcdata->clan )
    {
      if ( victim->pcdata->clan->clan_type == CLAN_ORDER )
      {
	if ( victim->pcdata->clan == clan )
	  send_to_char( "This player already belongs to your order!\n\r", ch );
	else
	  send_to_char( "This player already belongs to an order!\n\r", ch );
	return;
      }
      else
      if ( victim->pcdata->clan->clan_type == CLAN_GUILD )
      {
	if ( victim->pcdata->clan == clan )
	  send_to_char( "This player already belongs to your guild!\n\r", ch );
	else
	  send_to_char( "This player already belongs to an guild!\n\r", ch );
	return;
      }
      else
      {
	if ( victim->pcdata->clan == clan )
	  send_to_char( "This player already belongs to your clan!\n\r", ch );
	else
	  send_to_char( "This player already belongs to a clan!\n\r", ch );
	return;
      }
    }
    if ( clan->mem_limit && clan->members >= clan->mem_limit )
    {
    	send_to_char("Your clan is too big to induct anymore players.\n\r",ch);
	return;
    }
    clan->members++;
    if ( clan->clan_type != CLAN_ORDER && clan->clan_type != CLAN_GUILD )
      SET_BIT(victim->speaks, LANG_CLAN);

    if ( clan->clan_type != CLAN_NOKILL && clan->clan_type != CLAN_ORDER
    &&   clan->clan_type != CLAN_GUILD )
    {
      xREMOVE_BIT( victim->act, PLR_NICE );
      SET_BIT( victim->pcdata->flags, PCFLAG_DEADLY );
    }

    if ( clan->clan_type != CLAN_GUILD && clan->clan_type != CLAN_ORDER
    &&   clan->clan_type != CLAN_NOKILL )
    {
	int sn;

	for ( sn = 0; sn < top_sn; sn++ )
	{
	    if (skill_table[sn]->guild == clan->class &&
		skill_table[sn]->name != NULL )
	    {
		victim->pcdata->learned[sn] = GET_ADEPT(victim, sn);
		ch_printf( victim, "%s instructs you in the ways of %s.\n\r", ch->name, skill_table[sn]->name);
	    }
	}
    }

    victim->pcdata->clan = clan;
    STRFREE(victim->pcdata->clan_name);
    victim->pcdata->clan_name = QUICKLINK( clan->name );
    add_member( victim, victim->pcdata->clan_name );
    save_member_lists( );
    act( AT_MAGIC, "You induct $N into $t", ch, clan->name, victim, TO_CHAR );
    act( AT_MAGIC, "$n inducts $N into $t", ch, clan->name, victim, TO_NOTVICT );
    act( AT_MAGIC, "$n inducts you into $t", ch, clan->name, victim, TO_VICT );
    save_char_obj( victim );
    save_clan( clan );
    return;
}

void do_council_induct( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    COUNCIL_DATA *council;

    if ( IS_NPC( ch ) || !ch->pcdata->council )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    council = ch->pcdata->council;
    
  if ((council->head == NULL || str_cmp (ch->name, council->head))
      && ( council->head2 == NULL || str_cmp ( ch->name, council->head2 ))
      && str_cmp (council->name, "mortal council"))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Induct whom into your council?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

/*    if ( victim->level < LEVEL_IMMORTAL )
    {
	send_to_char( "This player is not worthy of joining any council yet.\n\r", ch );
	return;
    }
*/
    if ( victim->pcdata->council )
    {
	send_to_char( "This player already belongs to a council!\n\r", ch );
	return;
    }

    council->members++;
    victim->pcdata->council = council;
    STRFREE(victim->pcdata->council_name);
    victim->pcdata->council_name = QUICKLINK( council->name );
    act( AT_MAGIC, "You induct $N into $t", ch, council->name, victim, TO_CHAR );
    act( AT_MAGIC, "$n inducts $N into $t", ch, council->name, victim, TO_ROOM );
    act( AT_MAGIC, "$n inducts you into $t", ch, council->name, victim, TO_VICT );
    save_char_obj( victim );
    save_council( council );
    return;
}

void do_outcast( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CLAN_DATA *clan;
    char buf[MAX_STRING_LENGTH];
    int x=0, y=0;

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    clan = ch->pcdata->clan;

    if ( (ch->pcdata && ch->pcdata->bestowments
    &&    is_name("outcast", ch->pcdata->bestowments))
    ||   !str_cmp( ch->name, clan->deity   )
    ||   !str_cmp( ch->name, clan->leader  )
    ||   !str_cmp( ch->name, clan->number1 )
    ||   !str_cmp( ch->name, clan->number2 ) )
	;
    else
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Outcast whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }


    if ( !str_cmp( ch->name, clan->leader ) )
	x = 3;
    if ( !str_cmp( ch->name, clan->number1 ) )
	x = 2;
    if ( !str_cmp( ch->name, clan->number2 ) )
	x = 1;
    if ( !str_cmp( victim->name, clan->leader ) )
	y = 3;
    if ( !str_cmp( victim->name, clan->number1 ) )
	y = 2;
    if ( !str_cmp( victim->name, clan->number2 ) )
	y = 1;

    if ( x <= y && ( get_trust( ch ) <= get_trust( victim ) ) )
    {
	send_to_char( "You are not powerful enough to outcast this character.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	if ( ch->pcdata->clan->clan_type == CLAN_ORDER )
	{
	    send_to_char( "Kick yourself out of your own order?\n\r", ch );
	    return;
	}
	else
	if ( ch->pcdata->clan->clan_type == CLAN_GUILD )
	{
	    send_to_char( "Kick yourself out of your own guild?\n\r", ch );
	    return;
	}
	else
	{
	    send_to_char( "Kick yourself out of your own clan?\n\r", ch );
	    return;
	}
    }
 
    if ( victim->level > ch->level )
    {
	send_to_char( "This player is too powerful for you to outcast.\n\r", ch );
	return;
    }

    if ( victim->pcdata->clan != ch->pcdata->clan )
    {
	if ( ch->pcdata->clan->clan_type == CLAN_ORDER )
	{
	    send_to_char( "This player does not belong to your order!\n\r", ch );
	    return;
	}
	else
	if ( ch->pcdata->clan->clan_type == CLAN_GUILD )
	{
	    send_to_char( "This player does not belong to your guild!\n\r", ch );
	    return;
	}
	else
	{
	    send_to_char( "This player does not belong to your clan!\n\r", ch );
	    return;
	}
    }

    if ( clan->clan_type != CLAN_GUILD && clan->clan_type != CLAN_ORDER
    &&   clan->clan_type != CLAN_NOKILL )
    {
	int sn;

	for ( sn = 0; sn < top_sn; sn++ )
	    if ( skill_table[sn]->guild == victim->pcdata->clan->class
	    &&   skill_table[sn]->name != NULL )
	    {
		victim->pcdata->learned[sn] = 0;
		ch_printf( victim, "You forget the ways of %s.\n\r", skill_table[sn]->name);
	    }
    }

    if ( victim->speaking & LANG_CLAN )
        victim->speaking = LANG_COMMON;
    REMOVE_BIT( victim->speaks, LANG_CLAN );
    --clan->members;
    if ( !str_cmp( victim->name, ch->pcdata->clan->number1 ) )
    {
	STRFREE( ch->pcdata->clan->number1 );
	ch->pcdata->clan->number1 = STRALLOC( "" );
    }
    if ( !str_cmp( victim->name, ch->pcdata->clan->number2 ) )
    {
	STRFREE( ch->pcdata->clan->number2 );
	ch->pcdata->clan->number2 = STRALLOC( "" );
    }
    remove_member( victim->pcdata->clan_name, victim->name ); 
    save_member_lists( );
    victim->pcdata->clan = NULL;
    STRFREE(victim->pcdata->clan_name);
    victim->pcdata->clan_name = STRALLOC( "" );
    act( AT_MAGIC, "You outcast $N from $t", ch, clan->name, victim, TO_CHAR );
    act( AT_MAGIC, "$n outcasts $N from $t", ch, clan->name, victim, TO_ROOM );
    if ( victim->desc && victim->desc->host )
    act( AT_MAGIC, "$n outcasts you from $t", ch, clan->name, victim, TO_VICT );
	else add_loginmsg( victim->name, 6, NULL );

    if ( clan->clan_type != CLAN_GUILD 
    &&   clan->clan_type != CLAN_ORDER )
    {
	sprintf(buf, "%s has been outcast from %s!", victim->name, clan->name);
	echo_to_all( AT_MAGIC, buf, ECHOTAR_PK );
    }

/* Outcast flag setting removed by Narn.  It's useless now that deadlies
   remain deadly even on being cast out of a clan.
*/ 
/*    if ( clan->clan_type != CLAN_GUILD )
	xSET_BIT(victim->act, PLR_OUTCAST);
*/
    save_char_obj( victim );	/* clan gets saved when pfile is saved */
    save_clan( clan );
    return;
}

void do_council_outcast( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    COUNCIL_DATA *council;

    if ( IS_NPC( ch ) || !ch->pcdata->council )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    council = ch->pcdata->council;

  if ((council->head == NULL || str_cmp (ch->name, council->head))
      && ( council->head2 == NULL || str_cmp ( ch->name, council->head2 ))
      && str_cmp (council->name, "mortal council"))
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Outcast whom from your council?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Kick yourself out of your own council?\n\r", ch );
	return;
    }
 
    if ( victim->pcdata->council != ch->pcdata->council )
    {
	send_to_char( "This player does not belong to your council!\n\r", ch );
	return;
    }

    --council->members;
    victim->pcdata->council = NULL;
    STRFREE(victim->pcdata->council_name);
    victim->pcdata->council_name = STRALLOC( "" );
    act( AT_MAGIC, "You outcast $N from $t", ch, council->name, victim, TO_CHAR );
    act( AT_MAGIC, "$n outcasts $N from $t", ch, council->name, victim, TO_ROOM );
    act( AT_MAGIC, "$n outcasts you from $t", ch, council->name, victim, TO_VICT );
    save_char_obj( victim );
    save_council( council );
    return;
}

void do_setclan( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CLAN_DATA *clan;

    set_char_color( AT_PLAIN, ch );
    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Usage: setclan <clan> <field> <deity|leader|number1|number2> <player>\n\r", ch );
	send_to_char( "\n\rField being one of:\n\r", ch );
	send_to_char( " deity leader number1 number2\n\r", ch ); 
	send_to_char( " members board recall storage guard1 guard2\n\r", ch );
	send_to_char( " align (not functional) memlimit\n\r", ch );
	send_to_char( " leadrank onerank tworank\n\r", ch );
	send_to_char( " obj1 obj2 obj3 obj4 obj5\n\r", ch );
	send_to_char( " badge abbrev\n\r", ch );
	if ( get_trust( ch ) >= LEVEL_LESSER )
	{
	  send_to_char( " name filename motto desc\n\r", ch );
	  send_to_char( " favour strikes type class\n\r", ch );
	}
	if ( get_trust( ch ) >= LEVEL_IMPLEMENTOR )
	  send_to_char(" pkill1-7 pdeath1-7\n\r", ch );
	return;
    }

    clan = get_clan( arg1 );
    if ( !clan )
    {
	send_to_char( "No such clan.\n\r", ch );
	return;
    }
    if ( !str_cmp( arg2, "deity" ) )
    {
	STRFREE( clan->deity );
	clan->deity = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "leader" ) )
    {
	STRFREE( clan->leader );
	clan->leader = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "number1" ) )
    {
	STRFREE( clan->number1 );
	clan->number1 = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "number2" ) )
    {
	STRFREE( clan->number2 );
	clan->number2 = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "leadrank" ) )
    {
	STRFREE( clan->leadrank );
	clan->leadrank = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "onerank" ) )
    {
	STRFREE( clan->onerank );
	clan->onerank = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "tworank" ) )
    {
	STRFREE( clan->tworank );
	clan->tworank = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "badge" ) )
    {
	STRFREE( clan->badge );
	clan->badge = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "abbrev" ) )
    {
	STRFREE( clan->abbrev );
	clan->abbrev = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "board" ) )
    {
	clan->board = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "memlimit") )
    {
    	clan->mem_limit = atoi( argument );
	send_to_char( "Done.\n\r", ch  );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "members" ) )
    {
	clan->members = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "recall" ) )
    {
	clan->recall = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "storage" ) )
    {
        VAULT_DATA *vault;
        if (clan->storeroom)
        {
        for (vault = first_vault; vault; vault = vault->next)
      if ( vault->vnum == clan->storeroom )
                UNLINK(vault, first_vault, last_vault, next, prev);
        }
        clan->storeroom = atoi( argument );
        CREATE( vault, VAULT_DATA, 1 );
        vault->vnum = atoi( argument );
        sort_vaults( vault );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }                                                                                                                                           
    if ( !str_cmp( arg2, "obj1" ) )
    {
	clan->clanobj1 = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "obj2" ) )
    {
	clan->clanobj2 = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "obj3" ) )
    {
	clan->clanobj3 = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "obj4" ) )
    {
        clan->clanobj4 = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }
    if ( !str_cmp( arg2, "obj5" ) )
    {
        clan->clanobj5 = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_clan( clan );
        return;
    }
    if ( !str_cmp( arg2, "guard1" ) )
    {
	clan->guard1 = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "guard2" ) )
    {
	clan->guard2 = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( get_trust( ch ) < LEVEL_LESSER )
    {
	do_setclan( ch, "" );
	return;
    }
    if ( !str_cmp( arg2, "align" ) )
    {
	clan->alignment = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "type" ) )
    {
	if ( !str_cmp( argument, "order" ) )
	  clan->clan_type = CLAN_ORDER;
	else
	if ( !str_cmp( argument, "guild" ) )
	  clan->clan_type = CLAN_GUILD;
	else
	  clan->clan_type = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "class" ) )
    {
	clan->class = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "name" ) )
    {
	STRFREE( clan->name );
	clan->name = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "filename" ) )
    {
	DISPOSE( clan->filename );
	clan->filename = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	write_clan_list( );
	return;
    }
    if ( !str_cmp( arg2, "motto" ) )
    {
	STRFREE( clan->motto );
	clan->motto = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( !str_cmp( arg2, "desc" ) )
    {
	STRFREE( clan->description );
	clan->description = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( get_trust( ch ) < LEVEL_IMPLEMENTOR )
    {
        do_setclan( ch, "" );
        return;
    }
    if ( !str_prefix( "pkill", arg2) )
    {
	int temp_value;
	if ( !str_cmp( arg2, "pkill1" ) )
		temp_value = 0;
	else if ( !str_cmp( arg2, "pkill2" ) )
		temp_value = 1;
	else if ( !str_cmp( arg2, "pkill3" ) )
		temp_value = 2;
	else if ( !str_cmp( arg2, "pkill4" ) )
		temp_value = 3;
	else if ( !str_cmp( arg2, "pkill5" ) )
		temp_value = 4;
	else if ( !str_cmp( arg2, "pkill6" ) )
		temp_value = 5;
	else if ( !str_cmp( arg2, "pkill7" ) )
		temp_value = 6;
	else
	{
		do_setclan ( ch, "" );
		return;
	}
	clan->pkills[temp_value] = atoi( argument );
	send_to_char ("Ok.\n\r", ch );
	return;
    }
    if ( !str_prefix( "pdeath", arg2) )
    {
	int temp_value;
	if ( !str_cmp( arg2, "pdeath1" ) )
		temp_value = 0;
	else if ( !str_cmp( arg2, "pdeath2" ) )
		temp_value = 1;
	else if ( !str_cmp( arg2, "pdeath3" ) )
		temp_value = 2;
	else if ( !str_cmp( arg2, "pdeath4" ) )
		temp_value = 3;
	else if ( !str_cmp( arg2, "pdeath5" ) )
		temp_value = 4;
	else if ( !str_cmp( arg2, "pdeath6" ) )
		temp_value = 5;
	else if ( !str_cmp( arg2, "pdeath7" ) )
		temp_value = 6;
	else
	{
		do_setclan ( ch, "" );
		return;
	}
	clan->pdeaths[temp_value] = atoi( argument );
	send_to_char ("Ok.\n\r", ch );
	return;
    }
    do_setclan( ch, "" );
    return;
}

void do_setcouncil( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    COUNCIL_DATA *council;

    set_char_color( AT_PLAIN, ch );

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Usage: setcouncil <council> <field> <deity|leader|number1|number2> <player>\n\r", ch );
	send_to_char( "\n\rField being one of:\n\r", ch );
	send_to_char( " head head2 members board meeting\n\r", ch );
	send_to_char( " abbrev storage\n\r", ch );
	if ( get_trust( ch ) >= LEVEL_GOD )
	  send_to_char( " name filename desc\n\r", ch );
        if ( get_trust( ch ) >= LEVEL_SUB_IMPLEM )
	  send_to_char( " powers\n\r", ch);
	return;
    }

    council = get_council( arg1 );
    if ( !council )
    {
	send_to_char( "No such council.\n\r", ch );
	return;
    }
    if ( !str_cmp( arg2, "head" ) )
    {
	STRFREE( council->head );
	council->head = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }

  if (!str_cmp (arg2, "head2"))
    {
      if ( council->head2 != NULL )
        STRFREE (council->head2);
      if ( !str_cmp ( argument, "none" ) || !str_cmp ( argument, "clear" ) )
        council->head2 = NULL;
      else
        council->head2 = STRALLOC (argument);
      send_to_char ("Done.\n\r", ch);
      save_council (council);
      return;
    }
    if ( !str_cmp( arg2, "board" ) )
    {
	council->board = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }
    if ( !str_cmp( arg2, "members" ) )
    {
	council->members = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }
    if ( !str_cmp( arg2, "meeting" ) )
    {
	council->meeting = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }
    if ( !str_cmp( arg2, "storage" ) )
    {
        VAULT_DATA *vault;
        if (council->storeroom)
        {
        for (vault = first_vault; vault; vault = vault->next)
      if ( vault->vnum == council->storeroom )
                UNLINK(vault, first_vault, last_vault, next, prev);
        }
        council->storeroom = atoi( argument );
        CREATE( vault, VAULT_DATA, 1 );
        vault->vnum = atoi( argument );
        sort_vaults( vault );
        send_to_char( "Done.\n\r", ch );
        save_council( council );
        return;
    }                                                                                                                                           
    if ( get_trust( ch ) < LEVEL_GOD )
    {
	do_setcouncil( ch, "" );
	return;
    }
    if ( !str_cmp( arg2, "name" ) )
    {
	STRFREE( council->name );
	council->name = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }
    if ( !str_cmp( arg2, "filename" ) )
    {
	DISPOSE( council->filename );
	council->filename = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	write_council_list( );
	return;
    }
    if ( !str_cmp( arg2, "abbrev" ) )
    {
	STRFREE( council->abbrev );
	council->abbrev = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }
    if ( !str_cmp( arg2, "desc" ) )
    {
	STRFREE( council->description );
	council->description = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }
    if ( get_trust( ch ) < LEVEL_SUB_IMPLEM )
    {
	do_setcouncil( ch, "" );
	return;
    }
    if ( !str_cmp( arg2, "powers" ) )
    {
	STRFREE( council->powers );
	council->powers = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }
    
    do_setcouncil( ch, "" );
    return;
}

/*
 * Added multiple levels on pkills and pdeaths. -- Shaddai
 */

void do_showclan( CHAR_DATA *ch, char *argument )
{   
    CLAN_DATA *clan;

    set_char_color( AT_PLAIN, ch );

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }
    if ( argument[0] == '\0' )
    {
	send_to_char( "Usage: showclan <clan>\n\r", ch );
	return;
    }

    clan = get_clan( argument );
    if ( !clan )
    {
	send_to_char( "No such clan, guild or order.\n\r", ch );
	return;
    }

    ch_printf_color( ch, "\n\r&w%s    : &W%s\t\t&wBadge: %s\n\r&wFilename : &W%s\n\r&wMotto    : &W%s\n\r",
			clan->clan_type == CLAN_ORDER ? "Order" :
		       (clan->clan_type == CLAN_GUILD ? "Guild" : "Clan "),
    			clan->name,
			clan->badge ? clan->badge : "(not set)",
    			clan->filename,
    			clan->motto );
    ch_printf_color( ch, "&wAbbrev   : &W%s\n\r", clan->abbrev ? clan->abbrev : "(not set)" );
    ch_printf_color( ch, "&wDesc     : &W%s\n\r&wDeity    : &W%s\n\r",
    			clan->description,
    			clan->deity );
    ch_printf_color( ch, "&wLeader   : &W%-19.19s\t&wRank: &W%s\n\r",
			clan->leader,
			clan->leadrank );
    ch_printf_color( ch, "&wNumber1  : &W%-19.19s\t&wRank: &W%s\n\r",
			clan->number1,
			clan->onerank );
    ch_printf_color( ch, "&wNumber2  : &W%-19.19s\t&wRank: &W%s\n\r",
			clan->number2,
			clan->tworank );
    ch_printf_color( ch, "&wPKills   : &w1-9:&W%-3d &w10-14:&W%-3d &w15-19:&W%-3d &w20-29:&W%-3d &w30-39:&W%-3d &w40-49:&W%-3d &w50:&W%-3d\n\r",  
    			clan->pkills[0], clan->pkills[1], clan->pkills[2],
    			clan->pkills[3], clan->pkills[4], clan->pkills[5],
			clan->pkills[6]);
    ch_printf_color( ch, "&wPDeaths  : &w1-9:&W%-3d &w10-14:&W%-3d &w15-19:&W%-3d &w20-29:&W%-3d &w30-39:&W%-3d &w40-49:&W%-3d &w50:&W%-3d\n\r",  
    			clan->pdeaths[0], clan->pdeaths[1], clan->pdeaths[2],
    			clan->pdeaths[3], clan->pdeaths[4], clan->pdeaths[5],
			clan->pdeaths[6] );
    ch_printf_color( ch, "&wIllegalPK: &W%-6d\n\r",
			clan->illegal_pk );
    ch_printf_color( ch, "&wMKills   : &W%-6d   &wMDeaths: &W%-6d\n\r",
    			clan->mkills,
    			clan->mdeaths );
    ch_printf_color( ch, "&wScore    : &W%-6d   &wFavor  : &W%-6d   &wStrikes: &W%d\n\r",
    			clan->score,
    			clan->favour,
    			clan->strikes );
    ch_printf_color( ch, "&wMembers  : &W%-6d  &wMemLimit: &W%-6d   &wAlign  : &W%-6d",
    			clan->members,
    			clan->mem_limit,
    			clan->alignment );
    if ( clan->clan_type == CLAN_GUILD )
	ch_printf_color( ch, "   &wClass  : &W%d &w(&W%s&w)",
			clan->class,
    			clan->class<MAX_PC_CLASS?class_table[clan->class]->who_name:
			"unknown" );
    send_to_char( "\n\r", ch );
    ch_printf_color( ch, "&wBoard    : &W%-5d    &wRecall : &W%-5d    &wStorage: &W%-5d\n\r",
			clan->board,
			clan->recall,
			clan->storeroom ); 
    ch_printf_color( ch, "&wGuard1   : &W%-5d    &wGuard2 : &W%-5d\n\r",
 			clan->guard1,
			clan->guard2 );
    ch_printf_color( ch, "&wObj1( &W%d &w)  Obj2( &W%d &w)  Obj3( &W%d &w)  Obj4( &W%d &w)  Obj5( &W%d &w)\n\r",
    			clan->clanobj1,
    			clan->clanobj2,
    			clan->clanobj3,
			clan->clanobj4,
			clan->clanobj5 );
    return;
}

void do_showcouncil( CHAR_DATA *ch, char *argument )
{
    COUNCIL_DATA *council;

    set_char_color( AT_PLAIN, ch );

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }
    if ( argument[0] == '\0' )
    {
	send_to_char( "Usage: showcouncil <council>\n\r", ch );
	return;
    }

    council = get_council( argument );
    if ( !council )
    {
	send_to_char( "No such council.\n\r", ch );
	return;
    }

    ch_printf_color( ch, "\n\r&wCouncil :  &W%s\n\r&wFilename:  &W%s\n\r",
    			council->name,
    			council->filename );
    ch_printf_color( ch, "&wAbbreviation :  &W%s\n\r", council->abbrev );
    ch_printf_color (ch, "&wHead:      &W%s\n\r", council->head );
    ch_printf_color (ch, "&wHead2:     &W%s\n\r", council->head2 );
    ch_printf_color (ch, "&wMembers:   &W%-d\n\r", council->members );
    ch_printf_color( ch, "&wBoard:     &W%-5d\n\r&wMeeting:   &W%-5d\n\r&wPowers:    &W%s\n\r",
    			council->board,
    			council->meeting,
			council->powers );
    ch_printf_color( ch, "&wStoreroom: &W%-5d\n\r", council->storeroom );  
    ch_printf_color( ch, "&wDescription:\n\r&W%s\n\r", council->description );
    return;
}

void do_makeclan( CHAR_DATA *ch, char *argument )
{
    char filename[256];
    CLAN_DATA *clan;
    bool found;

    set_char_color( AT_IMMORT, ch );

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "Usage: makeclan <clan name>\n\r", ch );
	return;
    }

    found = FALSE;
    sprintf( filename, "%s%s", CLAN_DIR, strlower(argument) );

    CREATE( clan, CLAN_DATA, 1 );
    LINK( clan, first_clan, last_clan, next, prev );

    clan->name		= STRALLOC( argument );
    clan->abbrev	= STRALLOC( "" );
    clan->filename	= STRALLOC( "" ); /*Bug fix by baria@mud.tander.com*/
    clan->motto		= STRALLOC( "" );
    clan->description	= STRALLOC( "" );
    clan->deity		= STRALLOC( "" );
    clan->leader	= STRALLOC( "" );
    clan->number1	= STRALLOC( "" );
    clan->number2	= STRALLOC( "" );
    clan->leadrank	= STRALLOC( "" );
    clan->onerank	= STRALLOC( "" );
    clan->tworank	= STRALLOC( "" );
    clan->badge		= STRALLOC( "" );
}

void do_makecouncil( CHAR_DATA *ch, char *argument )
{
    char filename[256];
    COUNCIL_DATA *council;
    bool found;

    set_char_color( AT_IMMORT, ch );

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "Usage: makecouncil <council name>\n\r", ch );
	return;
    }

    found = FALSE;
    sprintf( filename, "%s%s", COUNCIL_DIR, strlower(argument) );

    CREATE( council, COUNCIL_DATA, 1 );
    LINK( council, first_council, last_council, next, prev );
    council->name		= STRALLOC( argument );
    council->head		= STRALLOC( "" );
    council->head2 		= NULL;
    council->powers		= STRALLOC( "" );
    council->abbrev		= STRALLOC( "" );

}

/*
 * Added multiple level pkill and pdeath support. --Shaddai
 */

void do_clans( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *clan;
    int count = 0;

    if ( argument[0] == '\0' )
    {
        set_char_color( AT_BLOOD, ch );
        send_to_char( "\n\rClan          Deity         Leader           Pkills:    Avatar      Other\n\r_________________________________________________________________________\n\r\n\r", ch );
        for ( clan = first_clan; clan; clan = clan->next )
        {
            if ( clan->clan_type == CLAN_ORDER || clan->clan_type == CLAN_GUILD )
              continue;
            set_char_color( AT_GREY, ch);
            ch_printf( ch, "%-13s %-13s %-13s", clan->name, clan->deity, clan->leader );
            set_char_color( AT_BLOOD, ch );
            ch_printf( ch, "                %5d      %5d\n\r", 
		clan->pkills[6],
		(clan->pkills[2]+clan->pkills[3]+
		 clan->pkills[4]+clan->pkills[5]) );
            count++;
        }
        set_char_color( AT_BLOOD, ch );
        if ( !count )
          send_to_char( "There are no Clans currently formed.\n\r", ch );
        else
          send_to_char( "_________________________________________________________________________\n\r\n\rUse 'clans <clan>' for detailed information and a breakdown of victories.\n\r", ch );
        return;
    }

    clan = get_clan( argument );
    if ( !clan || clan->clan_type == CLAN_GUILD || clan->clan_type == CLAN_ORDER )
    {
        set_char_color( AT_BLOOD, ch );
        send_to_char( "No such clan.\n\r", ch );
        return;
    }
    set_char_color( AT_BLOOD, ch );
    ch_printf( ch, "\n\r%s, '%s'\n\r\n\r", clan->name, clan->motto );
    set_char_color( AT_GREY, ch );
    send_to_char_color( "Victories:&w\n\r", ch );
   ch_printf_color( ch, "    &w15-19...  &r%-4d\n\r    &w20-29...  &r%-4d\n\r    &w30-39...  &r%-4d\n\r    &w40-49...  &r%-4d\n\r",
	clan->pkills[2],
	clan->pkills[3],
	clan->pkills[4],
	clan->pkills[5] );
    ch_printf_color( ch, "   &wAvatar...  &r%-4d\n\r", 
	clan->pkills[6] );
    set_char_color( AT_GREY, ch );
    ch_printf( ch, "Clan Leader:  %s\n\rNumber One :  %s\n\rNumber Two :  %s\n\rClan Deity :  %s\n\r",
                        clan->leader,
                        clan->number1,
                        clan->number2,
			clan->deity );
    if ( !str_cmp( ch->name, clan->deity   )
    ||   !str_cmp( ch->name, clan->leader  )
    ||   !str_cmp( ch->name, clan->number1 )
    ||   !str_cmp( ch->name, clan->number2 )
    ||   get_trust( ch ) >= LEVEL_GREATER  )
	ch_printf( ch, "Members    :  %d\n\r",
			clan->members );
    ch_printf( ch, "Abbrev     :  %s\n\r",
        clan->abbrev ? clan->abbrev : "" );
    set_char_color( AT_BLOOD, ch );
    ch_printf( ch, "\n\rDescription:  %s\n\r", clan->description );
    return;
}

void do_orders( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *order;
    int count = 0;

    if ( argument[0] == '\0' )
    {
        set_char_color( AT_DGREEN, ch );
        send_to_char( "\n\rOrder            Deity          Leader           Mkills      Mdeaths\n\r____________________________________________________________________\n\r\n\r", ch );
	set_char_color( AT_GREEN, ch );
        for ( order = first_clan; order; order = order->next )
        if ( order->clan_type == CLAN_ORDER )
        {
            ch_printf( ch, "%-16s %-14s %-14s   %-7d       %5d\n\r",
	      order->name, order->deity, order->leader, order->mkills, order->mdeaths );
            count++;
	}
        set_char_color( AT_DGREEN, ch );
	if ( !count )
	  send_to_char( "There are no Orders currently formed.\n\r", ch );
	else
	  send_to_char( "____________________________________________________________________\n\r\n\rUse 'orders <order>' for more detailed information.\n\r", ch );
	return;
    }

    order = get_clan( argument );
    if ( !order || order->clan_type != CLAN_ORDER )
    {
        set_char_color( AT_DGREEN, ch );
        send_to_char( "No such Order.\n\r", ch );
        return;
    }
 
    set_char_color( AT_DGREEN, ch );
    ch_printf( ch, "\n\rOrder of %s\n\r'%s'\n\r\n\r", order->name, order->motto );
    set_char_color( AT_GREEN, ch );
    ch_printf( ch, "Deity      :  %s\n\rLeader     :  %s\n\rNumber One :  %s\n\rNumber Two :  %s\n\r",
                        order->deity,
                        order->leader,
                        order->number1,
                        order->number2 );
    if ( !str_cmp( ch->name, order->deity   )
    ||   !str_cmp( ch->name, order->leader  )
    ||   !str_cmp( ch->name, order->number1 )
    ||   !str_cmp( ch->name, order->number2 )
    ||    get_trust( ch ) >= LEVEL_GREATER  )
        ch_printf( ch, "Members    :  %d\n\r",
			order->members );
    ch_printf( ch, "Abbrev     :  %s\n\r",
        order->abbrev ? order->abbrev : "" );
    set_char_color( AT_DGREEN, ch );
    ch_printf( ch, "\n\rDescription:\n\r%s\n\r", order->description );
    return;
}

void do_councils( CHAR_DATA *ch, char *argument)
{
    COUNCIL_DATA *council;

    set_char_color( AT_GREEN, ch );
    if ( !first_council )
    {
	send_to_char( "There are no councils currently formed.\n\r", ch );
	return;
    }
    if ( argument[0] == '\0' )
    {
        send_to_char_color ("\n\r&GTitle                    Head\n\r", ch);
      for (council = first_council; council; council = council->next)
      {
        if ( council->head2 != NULL )
           ch_printf_color (ch, "&w%-24s %s and %s\n\r",  council->name,
                council->head, council->head2 );
        else
           ch_printf_color (ch, "&w%-24s %-14s\n\r", council->name, council->head);
      }
      send_to_char_color( "&GUse 'councils <name of council>' for more detailed information.\n\r", ch );
      return;
    }        
    council = get_council( argument );
    if ( !council )
    {
      send_to_char_color( "&GNo such council exists...\n\r", ch );
      return;
    }
    ch_printf_color( ch, "&G\n\r%s\n\r", council->name );
  if ( council->head2 == NULL )
        ch_printf_color (ch, "&GHead:     &w%s\n\r&GMembers:  &w%d\n\r",
        council->head, council->members );
  else
        ch_printf_color (ch, "&GCo-Heads:     &w%s &Gand &w%s\n\r&GMembers:  &w%d\n\r",
           council->head, council->head2, council->members );
    ch_printf_color( ch, "&GAbbrev:   &w%s\n\r",
	   council->abbrev ? council->abbrev : "" );
    ch_printf_color( ch, "&GDescription:\n\r&w%s\n\r",
        council->description );
    return;
} 

void do_guilds( CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];  
    CLAN_DATA *guild;
    int count = 0;

    if ( argument[0] == '\0' )
    {
        set_char_color( AT_HUNGRY, ch );
        send_to_char( "\n\rGuild                  Leader             Mkills      Mdeaths\n\r_____________________________________________________________\n\r\n\r", ch );
	set_char_color( AT_YELLOW, ch );
        for ( guild = first_clan; guild; guild = guild->next )
        if ( guild->clan_type == CLAN_GUILD )
	{
	    ++count;
	    ch_printf( ch, "%-20s   %-14s     %-6d       %6d\n\r", guild->name, guild->leader, guild->mkills, guild->mdeaths );
	}
        set_char_color( AT_HUNGRY, ch );
        if ( !count )
          send_to_char( "There are no Guilds currently formed.\n\r", ch );
        else
          send_to_char( "_____________________________________________________________\n\r\n\rUse guilds <class>' for specifics. (ex:  guilds thieves)\n\r", ch );
	return;
    }

    sprintf( buf, "guild of %s", argument );
    guild = get_clan( buf );
	if ( !guild || guild->clan_type != CLAN_GUILD )
		guild = get_clan( argument );
    if ( !guild || guild->clan_type != CLAN_GUILD )
    {
        set_char_color( AT_HUNGRY, ch );
        send_to_char( "No such Guild.\n\r", ch );
        return;
    }
    set_char_color( AT_HUNGRY, ch );
    ch_printf( ch, "\n\r%s\n\r", guild->name );
    set_char_color( AT_YELLOW, ch );
    ch_printf( ch, "Leader:    %s\n\rNumber 1:  %s\n\rNumber 2:  %s\n\rMotto:     %s\n\r",
                        guild->leader,
                        guild->number1,
                        guild->number2,
			guild->motto );
    if ( !str_cmp( ch->name, guild->deity   )
    ||   !str_cmp( ch->name, guild->leader  )
    ||   !str_cmp( ch->name, guild->number1 )
    ||   !str_cmp( ch->name, guild->number2 )
    ||    get_trust( ch ) >= LEVEL_GREATER )
        ch_printf( ch, "Members:   %d\n\r",
			guild->members );
    ch_printf( ch, "Abbrev:    %s\n\r",
	guild->abbrev ? guild->abbrev : "" );
    set_char_color( AT_HUNGRY, ch );
    ch_printf( ch, "Guild Description:\n\r%s\n\r", guild->description );
    return;
}                                                                           

void do_defeats( CHAR_DATA *ch, char *argument )
{
    char filename[256];

    if ( IS_NPC( ch ) || !ch->pcdata->clan ) {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    if ( ch->pcdata->clan->clan_type != CLAN_ORDER
    &&   ch->pcdata->clan->clan_type != CLAN_GUILD ) {
        sprintf( filename, "%s%s.defeats",
          CLAN_DIR, ch->pcdata->clan->name );
        set_pager_color( AT_PURPLE, ch );
        if ( !str_cmp( ch->name, ch->pcdata->clan->leader )
        &&   !str_cmp( argument, "clean" ) ) {
          FILE *fp = fopen( filename, "w" );
          if ( fp )
            fclose( fp );
          send_to_pager( "\n\rDefeats ledger has been cleared.\n\r", ch );
          return;
        } else {
          send_to_pager( "\n\rLVL  Character                LVL  Character\n\r", ch );
          show_file( ch, filename );
          return;
        }
    }
    else
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
}

void do_victories( CHAR_DATA *ch, char *argument )
{
    char filename[256]; 

    if ( IS_NPC( ch ) || !ch->pcdata->clan ) {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    if ( ch->pcdata->clan->clan_type != CLAN_ORDER
    &&   ch->pcdata->clan->clan_type != CLAN_GUILD ) {
        sprintf( filename, "%s%s.record",
          CLAN_DIR, ch->pcdata->clan->name );
	set_pager_color( AT_PURPLE, ch );
	if ( !str_cmp( ch->name, ch->pcdata->clan->leader )
	&&   !str_cmp( argument, "clean" ) ) {
	  FILE *fp = fopen( filename, "w" );
	  if ( fp )
	    fclose( fp );
	  send_to_pager( "\n\rVictories ledger has been cleared.\n\r", ch );
	  return;
	} else {
	  send_to_pager( "\n\rLVL  Character       LVL  Character\n\r", ch );
	  show_file( ch, filename );
	  return;
	}
    }
    else
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
}


void do_shove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int exit_dir;
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    bool nogo;
    ROOM_INDEX_DATA *to_room;    
    int chance = 0;
    int race_bonus = 0;
    sh_int temp;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( IS_NPC(ch)
    || !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
    {
	send_to_char("Only deadly characters can shove.\n\r", ch);
	return;
    }

    if  ( get_timer(ch, TIMER_PKILLED) > 0)
    {
	send_to_char("You can't shove a player right now.\n\r", ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Shove whom?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch);
	return;
    }

    if (victim == ch)
    {
	send_to_char("You shove yourself around, to no avail.\n\r", ch);
	return;
    }
    if ( IS_NPC(victim)
    || !IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ) )
    {
	send_to_char("You can only shove deadly characters.\n\r", ch);
	return;
    }
    
    if ( ch->level - victim->level > 5 
    ||   victim->level - ch->level > 5 )
    {
	send_to_char("There is too great an experience difference for you to even bother.\n\r", ch);
	return;
    }

    if  ( get_timer(victim, TIMER_PKILLED) > 0)
    {
	send_to_char("You can't shove that player right now.\n\r", ch);
	return;
    }

    if ( (victim->position) != POS_STANDING )
    {
	act( AT_PLAIN, "$N isn't standing up.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	send_to_char( "Shove them in which direction?\n\r", ch);
	return;
    }

    exit_dir = get_dir( arg2 );
    if ( xIS_SET(victim->in_room->room_flags, ROOM_SAFE)
    &&  get_timer(victim, TIMER_SHOVEDRAG) <= 0)
    {
	send_to_char("That character cannot be shoved right now.\n\r", ch);
	return;
    }
/*    victim->position = POS_SHOVE; -- August, 2000 */
    nogo = FALSE;
    if ((pexit = get_exit(ch->in_room, exit_dir)) == NULL )
      nogo = TRUE;
    else
    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    && (!IS_AFFECTED(victim, AFF_PASS_DOOR)
    ||   IS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
      nogo = TRUE;
    if ( nogo )
    {
	send_to_char( "There's no exit in that direction.\n\r", ch );
/*        victim->position = POS_STANDING; --August, 2000 */
	return;
    }
    to_room = pexit->to_room;
    if (xIS_SET(to_room->room_flags, ROOM_DEATH))
    {
      send_to_char("You cannot shove someone into a death trap.\n\r", ch);
/*    victim->position = POS_STANDING; --August, 2000 */
      return;
    }
    
    if (ch->in_room->area != to_room->area
    &&  !in_hard_range( victim, to_room->area ) )
    {
      send_to_char("That character cannot enter that area.\n\r", ch);
/*
      victim->position = POS_STANDING; -- August, 2000 
*/
      return;
    }

/* Check for class, assign percentage based on that. */
if (ch->class == CLASS_WARRIOR)
  chance = 70;
if (ch->class == CLASS_VAMPIRE)
  chance = 65;
if (ch->class == CLASS_RANGER)
  chance = 60;
if (ch->class == CLASS_DRUID)
  chance = 45;
if (ch->class == CLASS_CLERIC)
  chance = 35;
if (ch->class == CLASS_THIEF)
  chance = 30;
if (ch->class == CLASS_MAGE)
  chance = 15;

/* Add 3 points to chance for every str point above 15, subtract for 
below 15 */

chance += ((get_curr_str(ch) - 15) * 3);

chance += (ch->level - victim->level);

if (ch->race == 1)
race_bonus = -3;

if (ch->race == 2)
race_bonus = 3;

if (ch->race == 3)
race_bonus = -5;

if (ch->race == 4)
race_bonus = -7;
 
if (ch->race == 6)
race_bonus = 5;
 
if (ch->race == 7)
race_bonus = 7;
 
if (ch->race == 8)
race_bonus = 10;
 
if (ch->race == 9)
race_bonus = -2;
 
chance += race_bonus;
 
/* Debugging purposes - show percentage for testing */

/* sprintf(buf, "Shove percentage of %s = %d", ch->name, chance);
act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
*/

if (chance < number_percent( ))
{
  send_to_char("You failed.\n\r", ch);
/*  victim->position = POS_STANDING; -- August, 2000 */
  return;
}
    temp = victim->position;
    victim->position = POS_SHOVE;
    act( AT_ACTION, "You shove $M.", ch, NULL, victim, TO_CHAR );
    act( AT_ACTION, "$n shoves you.", ch, NULL, victim, TO_VICT );
    move_char( victim, get_exit(ch->in_room,exit_dir), 0);
    if ( !char_died(victim) )
      victim->position = temp;
    WAIT_STATE(ch, 12);
    /* Remove protection from shove/drag if char shoves -- Blodkai */
    if ( xIS_SET(ch->in_room->room_flags, ROOM_SAFE)   
    &&   get_timer(ch, TIMER_SHOVEDRAG) <= 0 )
      add_timer( ch, TIMER_SHOVEDRAG, 10, NULL, 0 );
}

void do_drag( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int exit_dir;
    CHAR_DATA *victim;
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *to_room;
    bool nogo;
    int chance = 0;
    int race_bonus = 0;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( IS_NPC(ch) )
    /* || !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )  */
    {
	send_to_char("Only characters can drag.\n\r", ch);
	return;
    }

    if  ( get_timer(ch, TIMER_PKILLED) > 0)
    {
	send_to_char("You can't drag a player right now.\n\r", ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Drag whom?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch);
	return;
    }

    if ( victim == ch )
    {
	send_to_char("You take yourself by the scruff of your neck, but go nowhere.\n\r", ch);
	return; 
    }

    if ( IS_NPC(victim)  )
         /* || !IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ) ) */
    {
	send_to_char("You can only drag characters.\n\r", ch);
	return;
    }

    if ( !xIS_SET( victim->act, PLR_SHOVEDRAG)
    &&   !IS_SET( victim->pcdata->flags, PCFLAG_DEADLY) )
    {
	send_to_char("That character doesn't seem to appreciate your attentions.\n\r", ch);
	return;
    }

    if  ( get_timer(victim, TIMER_PKILLED) > 0)
    {
	send_to_char("You can't drag that player right now.\n\r", ch);
	return;
    }

    if ( victim->fighting )
    {
        send_to_char( "You try, but can't get close enough.\n\r", ch);
        return;
    }
          
    if( !IS_SET(ch->pcdata->flags, PCFLAG_DEADLY ) && IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ) ){
	send_to_char("You cannot drag a deadly character.\n\r", ch);
	return;
    }

    if ( !IS_SET(victim->pcdata->flags, PCFLAG_DEADLY) && victim->position > 3 )
    {
	send_to_char("They don't seem to need your assistance.\n\r", ch );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	send_to_char( "Drag them in which direction?\n\r", ch);
	return;
    }

    if ( ch->level - victim->level > 5
    ||   victim->level - ch->level > 5 )
    {
	if ( IS_SET(victim->pcdata->flags, PCFLAG_DEADLY)
	&&   IS_SET(ch->pcdata->flags, PCFLAG_DEADLY) )
	{
          send_to_char("There is too great an experience difference for you to even bother.\n\r", ch);
          return;
	}
    }

    exit_dir = get_dir( arg2 );

    if ( xIS_SET(victim->in_room->room_flags, ROOM_SAFE)
    &&   get_timer( victim, TIMER_SHOVEDRAG ) <= 0)
    {
	send_to_char("That character cannot be dragged right now.\n\r", ch);
	return;
    }

    nogo = FALSE;
    if ((pexit = get_exit(ch->in_room, exit_dir)) == NULL )
      nogo = TRUE;
    else
    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    && (!IS_AFFECTED(victim, AFF_PASS_DOOR)
    ||   IS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
      nogo = TRUE;
    if ( nogo )
    {
	send_to_char( "There's no exit in that direction.\n\r", ch );
	return;
    }

    to_room = pexit->to_room;
    if (xIS_SET(to_room->room_flags, ROOM_DEATH))
    {
      send_to_char("You cannot drag someone into a death trap.\n\r", ch);
      return;
    }

    if (ch->in_room->area != to_room->area
    && !in_hard_range( victim, to_room->area ) )
    {
      send_to_char("That character cannot enter that area.\n\r", ch);
/*      victim->position = POS_STANDING; --August, 2000 */
      return;
    }
    
/* Check for class, assign percentage based on that. */
if (ch->class == CLASS_WARRIOR)
  chance = 70;
if (ch->class == CLASS_VAMPIRE)
  chance = 65;
if (ch->class == CLASS_RANGER)
  chance = 60;
if (ch->class == CLASS_DRUID)
  chance = 45;
if (ch->class == CLASS_CLERIC)
  chance = 35;
if (ch->class == CLASS_THIEF)
  chance = 30;
if (ch->class == CLASS_MAGE)
  chance = 15;

/* Add 3 points to chance for every str point above 15, subtract for 
below 15 */

chance += ((get_curr_str(ch) - 15) * 3);

chance += (ch->level - victim->level);

if (ch->race == 1)
race_bonus = -3;

if (ch->race == 2)
race_bonus = 3;

if (ch->race == 3)
race_bonus = -5;

if (ch->race == 4)
race_bonus = -7;

if (ch->race == 6)
race_bonus = 5;

if (ch->race == 7)
race_bonus = 7;

if (ch->race == 8)
race_bonus = 10;

if (ch->race == 9)
race_bonus = -2;

chance += race_bonus;
/*
sprintf(buf, "Drag percentage of %s = %d", ch->name, chance);
act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
*/
if (chance < number_percent( ))
{
  send_to_char("You failed.\n\r", ch);
/*  victim->position = POS_STANDING; -- August, 2000 */
  return;
}
    if ( victim->position < POS_STANDING )
    {
	sh_int temp;

	temp = victim->position;
	victim->position = POS_DRAG;
	act( AT_ACTION, "You drag $M into the next room.", ch, NULL, victim, TO_CHAR ); 
	act( AT_ACTION, "$n grabs your hair and drags you.", ch, NULL, victim, TO_VICT ); 
	move_char( victim, get_exit(ch->in_room,exit_dir), 0);
	if ( !char_died(victim) )
	  victim->position = temp;
/* Move ch to the room too.. they are doing dragging - Scryn */
	move_char( ch, get_exit(ch->in_room,exit_dir), 0);
	WAIT_STATE(ch, 12);
	return;
    }
    send_to_char("You cannot do that to someone who is standing.\n\r", ch);
    return;
}


void show_members( CHAR_DATA *ch, char *argument )
{
   MEMBER_LIST	*members_list;
   MEMBER_DATA	*member;
   CLAN_DATA	*clan;
   int count = 0;


   for( members_list = first_member_list; members_list; members_list = members_list->next )
   {
      if( !str_cmp( members_list->name, argument ) )
         break;
   }

   if( !members_list )
	return;

   clan = get_clan( argument );

   if ( !clan  )
      return;

   pager_printf_color( ch, "\n\r&G%s membership roster\n\r", clan->name );
   pager_printf_color( ch, "&GLeader    :  &w%s\n\r", clan->leader );
   pager_printf_color( ch, "&GNumber One:  &w%s\n\r", clan->number1 );
   pager_printf_color( ch, "&GNumber Two:  &w%s\n\r", clan->number2 );
   pager_printf_color( ch, "&G::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n\r" );
   pager_printf_color( ch, "&GLvl  Name          Class       Kills   Deaths        Entered\n\r\n\r" );
   for( member = members_list->first_member; member; member = member->next )
   {
      if( str_cmp( member->name, clan->leader ) && str_cmp( member->name, clan->number1 )
      && str_cmp( member->name, clan->number2 ) )
         pager_printf_color( ch, "&G%3d  &w%-12s  &G%-10s %6d  %7d     &g%10s\n\r",
		member->level,
		capitalize(member->name),
		class_table[member->class]->who_name,
		member->kills, member->deaths, member->since );
      count++;
   }
   pager_printf_color( ch, "\n\r%d members on the roll sheet.\n\r", count );
}

void remove_member( char *clanname, char *membername )
{
   MEMBER_LIST	*members_list;
   MEMBER_DATA	*member;

   for( members_list = first_member_list; members_list; members_list = members_list->next )
   {
      if( !str_cmp( members_list->name, clanname ) )
         break;
   }

   if( !members_list )
	return;

   for( member = members_list->first_member; member; member = member->next )
   {
      if( !str_cmp( member->name, membername ) )
      {
         UNLINK( member, members_list->first_member, members_list->last_member, next, prev );
         STRFREE( member->name );
         STRFREE( member->since );
         DISPOSE( member );
         break;
      }
   }
}

void add_member( CHAR_DATA *ch, char* clanname )
{
   MEMBER_LIST *members_list;
   MEMBER_DATA *nmember, *member;
   CLAN_DATA *clan;
   struct tm *t = localtime(&current_time);
   char buf[MAX_STRING_LENGTH];

   if ( IS_IMMORTAL( ch ) && str_cmp( ch->name, ch->pcdata->clan->deity  ) )
	return;

   for( members_list = first_member_list; members_list; members_list = members_list->next )
   {
      if( !str_cmp( members_list->name, clanname ) )
         break;
   }
   
   if( ( clan = get_clan( clanname ) ) == NULL )
	return;

   if( !members_list )
	return;

   for( member = members_list->first_member; member; member = member->next )
   {
	if ( !str_cmp( member->name, ch->name ) )
	  return;
   }
	
   CREATE( nmember, MEMBER_DATA, 1 );
   nmember->name = STRALLOC( ch->name );
   nmember->level = ch->level;
   nmember->class = ch->class;
   sprintf( buf, "%02d/%02d/%04d", t->tm_mon+1, t->tm_mday, t->tm_year+1900 );
   nmember->since = STRALLOC( buf );
   if( clan->clan_type == CLAN_ORDER || clan->clan_type == CLAN_GUILD )
   {
      nmember->kills = ch->pcdata->mkills;
      nmember->deaths = ch->pcdata->mdeaths;
   }
   else
   {
      nmember->kills = ch->pcdata->pkills;
      nmember->deaths = ch->pcdata->pdeaths;
   }
   LINK( nmember, members_list->first_member, members_list->last_member, next, prev );

}

void do_roster( CHAR_DATA *ch, char *argument )
{

   if ( IS_NPC( ch ) || !ch->pcdata->clan )
   {
	send_to_char( "Huh?\n\r", ch );
	return;
   }

/*
   if ( IS_SET( ch->pcdata->flags, PCFLAG_PAGERON ) && !IS_IMMORTAL( ch ) )
   {
	send_to_char( "The 'roster' command may not be used in conjunction with the pager until further notification.\n\r", ch );
	return;
   }
*/

   if ( IS_IMMORTAL( ch )
   ||   !str_cmp( ch->name, ch->pcdata->clan->leader  )
   ||   !str_cmp( ch->name, ch->pcdata->clan->number1 )
   ||   !str_cmp( ch->name, ch->pcdata->clan->number2 ) )
   {
	show_members( ch, ch->pcdata->clan_name );
	return;
   }
   send_to_char( "Huh?\n\r", ch );
   return;
}


void do_memberlist( CHAR_DATA *ch, char *argument )
{

   if ( !IS_IMMORTAL(ch) )
   {
      if( IS_NPC( ch ) || !ch->pcdata->clan 
          || str_cmp(ch->name, ch->pcdata->clan->leader ) )
      {
         send_to_char( "Huh?\n\r", ch );
         return;
      }

      show_members( ch, ch->pcdata->clan_name );
      return;
   }
   else
   {
      char arg1[MAX_STRING_LENGTH];
      argument = one_argument( argument, arg1 );

      if( argument[0] == '\0' || arg1[0] == '\0' )
      {
         send_to_char( "Arguments are:  show, create or delete.\n\r", ch );
         return;
      }
      
      if( !str_cmp( arg1, "show" ) )
      {
         if( !str_cmp( argument, "all" ) )
         {
            MEMBER_LIST *members_list;
            for( members_list = first_member_list; members_list; members_list = members_list->next )
               show_members( ch, members_list->name );
            return;
         }
         
         show_members( ch, argument );
         return;
      }
      
      if( !str_cmp( arg1, "create" ) )
      {
         MEMBER_LIST *members_list;
         
         CREATE( members_list, MEMBER_LIST, 1 );
         members_list->name = STRALLOC( argument );
         LINK( members_list, first_member_list, last_member_list, next, prev );
         save_member_lists( );
         ch_printf( ch, "Member lists \"%s\" created.\n\r", argument );
         return;
      }
      
      if( !str_cmp( arg1, "delete" ) )
      {
         MEMBER_LIST *members_list;
         MEMBER_DATA *member;

         for( members_list = first_member_list; members_list; members_list = members_list->next )
            if( !str_cmp( argument, members_list->name ) )
            {
               while( members_list->first_member )
               {
                  member = members_list->first_member;
                  STRFREE( member->name );
                  STRFREE( member->since );
                  UNLINK( member, members_list->first_member, members_list->last_member, next, prev );
                  DISPOSE( member );
               }
               
               STRFREE( members_list->name );
               UNLINK( members_list, first_member_list, last_member_list, next, prev );
               DISPOSE( members_list );
               ch_printf( ch, "Member list \"%s\" destroyed.\n\r", argument );
               save_member_lists( );
               return;
            }
         send_to_char( "No such list.\n\r", ch );
         return;
      }
   }
}

void save_member_lists( )
{
   MEMBER_LIST	*members_list;
   MEMBER_DATA	*member;
   FILE		*fp;
   
   if ( ( fp = fopen( MEMBERS_FILE, "w" ) ) == NULL )
   {
      bug( "Cannot open members.dat for writing", 0 );
      perror( SOCIAL_FILE );
      return;
   }

   for( members_list = first_member_list; members_list; members_list = members_list->next )
   {
      fprintf( fp, "#Members_list\n"	);
      fprintf( fp, "Name          %s~\n", members_list->name );
      for( member = members_list->first_member; member; member = member->next )
         fprintf( fp, "Member        %s %s %d %d %d %d\n", member->name, member->since,
                  member->kills, member->deaths, member->level, member->class );
      fprintf( fp, "End\n\n" );
   }
   
   fprintf( fp, "#END\n\n" );
   fclose( fp );
}

void load_member_lists( )
{
   FILE *fp;
   
   if( ( fp = fopen( MEMBERS_FILE, "r" ) ) == NULL )
   {
      bug( "Cannot open members.dat for reading", 0 );
      return;
   }
   
   for( ; ; )
   {
      char letter;
      char *word;
      
      letter = fread_letter( fp );
      
      if( letter != '#' )
      {
         bug( "load_member_lists: # not found", 0 );
         return;
      }
      
      word = fread_word( fp );
      
      if( !str_cmp( word, "Members_list" ) )
         fread_member_list( fp );
      else if ( !str_cmp( word, "END" ) )
         break;
      else
      {
         bug( "load_member_lists: bad section", 0 );
         break;
      }
   }

   fclose( fp );
   save_member_lists();

}


void fread_member_list( FILE *fp )
{
   MEMBER_LIST *members_list;
   MEMBER_DATA *member;
   
   CREATE( members_list, MEMBER_LIST, 1 );
   
   for( ; ; )
   {
      char *word;
      
      word = fread_word( fp );
      
      if( !str_cmp( word, "Name" ) )
      {
         members_list->name = fread_string( fp );
         continue;
      }
#define PURGEROSTER         
      if( !str_cmp( word, "Member" ) )
      {
#ifdef PURGEROSTER
		FILE *tfp;
		char pfile[MAX_INPUT_LENGTH];
#endif

         CREATE( member, MEMBER_DATA, 1 );
         member->name = STRALLOC( fread_word( fp ) );
#ifdef PURGEROSTER
	 sprintf( pfile, "%s%c/%s", PLAYER_DIR, tolower(member->name[0]),
            capitalize( member->name ) );
	 if ( (tfp = fopen( pfile, "r" ) ) == NULL )
	 {
		bug( "Deleting %s from roster - no pfile", member->name ); 
		STRFREE( member->name );
		DISPOSE( member );
		continue;
	 }
	 fclose(tfp);
#endif
         member->since = STRALLOC( fread_word( fp ) );
         member->kills = fread_number( fp );
         member->deaths = fread_number( fp );
         member->level = fread_number( fp );
         member->class = fread_number( fp );
         LINK( member, members_list->first_member, members_list->last_member, next, prev );
         continue;
      }
      
      if( !str_cmp( word, "End" ) )
      {
         LINK( members_list, first_member_list, last_member_list, next, prev );
         return;
      }
  }
   
}

void update_member( CHAR_DATA *ch )
{
   MEMBER_LIST *members_list;
   MEMBER_DATA *member;
   struct tm *t = localtime(&current_time);
   char buf[MAX_STRING_LENGTH];

   if( IS_NPC( ch ) || !ch->pcdata->clan )
      return;

   for( members_list = first_member_list; members_list; members_list = members_list->next )
   {
      if( !str_cmp( members_list->name, ch->pcdata->clan_name ) )
      {
         for( member = members_list->first_member; member; member = member->next )
	 {
            if ( !str_cmp( member->name, ch->name ) )
	    {
               if( ch->pcdata->clan->clan_type == CLAN_PLAIN )
               {
                  member->kills = ch->pcdata->pkills;
                  member->deaths = ch->pcdata->pdeaths;
               }
               else
               {
                  member->kills = ch->pcdata->mkills;
                  member->deaths = ch->pcdata->mdeaths;
               }
	       STRFREE( member->since );
	       sprintf( buf, "%02d/%02d/%04d", t->tm_mon+1, t->tm_mday,
		t->tm_year+1900 );
	       member->since = STRALLOC( buf );
	       member->level = ch->level;
	       if ( quitting_char == ch )
		 save_member_lists( );
	    }
	  }
      }
   }
}
