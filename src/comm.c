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
 *			 Low-level communication module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include "mud.h"
#ifdef USE_IMC
#include "imc.h"
#include "icec.h"
#endif


/*
 * Socket and TCP/IP stuff.
 */
#ifdef WIN32
  #include <io.h>
  #undef EINTR
  #undef EMFILE
  #define EINTR WSAEINTR
  #define EMFILE WSAEMFILE
  #define EWOULDBLOCK WSAEWOULDBLOCK
  #define MAXHOSTNAMELEN 32

  #define  TELOPT_ECHO        '\x01'
  #define  GA                 '\xF9'
  #define  SB                 '\xFA'
  #define  WILL               '\xFB'
  #define  WONT               '\xFC'
  #define  DO                 '\xFD'
  #define  DONT               '\xFE'
  #define  IAC                '\xFF'
  void bailout(void);
  void shutdown_checkpoint (void);
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netinet/in_systm.h>
  #include <netinet/ip.h>
  #include <arpa/inet.h>
  #include <arpa/telnet.h>
  #include <netdb.h>
  #define closesocket close
#endif

#ifdef sun
int gethostname ( char *name, int namelen );
#endif

const	char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const	char	echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const	char 	go_ahead_str	[] = { IAC, GA, '\0' };


void	auth_maxdesc	args( ( int *md, fd_set *ins, fd_set *outs,
				fd_set *excs ) );
void	auth_check	args( ( fd_set *ins, fd_set *outs, fd_set *excs ) );
void	set_auth	args( ( DESCRIPTOR_DATA *d ) );
void	kill_auth	args( ( DESCRIPTOR_DATA *d ) );

void    add_member              args( ( CHAR_DATA *ch, char* clanname ) );

void    save_sysdata args( ( SYSTEM_DATA sys ) );


/*
 * Global variables.
 */
IMMORTAL_HOST * immortal_host_start;    /* Start of Immortal legal domains */
IMMORTAL_HOST * immortal_host_end;    /* End of Immortal legal domains */
DESCRIPTOR_DATA *   first_descriptor;	/* First descriptor		*/
DESCRIPTOR_DATA *   last_descriptor;	/* Last descriptor		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
int		    num_descriptors;
FILE *		    fpReserve;		/* Reserved file handle		*/
bool		    mud_down;		/* Shutdown			*/
bool		    service_shut_down;  /* Shutdown by operator closing down service */
bool		    wizlock;		/* Game is wizlocked		*/
time_t              boot_time;
HOUR_MIN_SEC  	    set_boot_time_struct;
HOUR_MIN_SEC *      set_boot_time;
struct tm *         new_boot_time;
struct tm           new_boot_struct;
char		    str_boot_time[MAX_INPUT_LENGTH];
char		    lastplayercmd[MAX_INPUT_LENGTH*2];
time_t		    current_time;	/* Time of this pulse		*/
int		    control;		/* Controlling descriptor	*/
int		    control2;		/* Controlling descriptor #2	*/
int		    conclient;		/* MUDClient controlling desc	*/
int		    conjava;		/* JavaMUD controlling desc	*/
int		    newdesc;		/* New descriptor		*/
fd_set		    in_set;		/* Set of desc's for reading	*/
fd_set		    out_set;		/* Set of desc's for writing	*/
fd_set		    exc_set;		/* Set of desc's with errors	*/
int 		    maxdesc;
char *		    alarm_section = "(unknown)";

/*
 * OS-dependent local functions.
 */
void	game_loop		args( ( ) );
int	init_socket		args( ( int port ) );
void	new_descriptor		args( ( int new_desc ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );


/*
 * Other local functions (OS-independent).
 */
bool	check_parse_name	args( ( char *name, bool newchar ) );
bool	check_reconnect		args( ( DESCRIPTOR_DATA *d, char *name,
				    bool fConn ) );
bool	check_playing		args( ( DESCRIPTOR_DATA *d, char *name, bool kick ) );
int	main			args( ( int argc, char **argv ) );
void	nanny			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	flush_buffer		args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void	read_from_buffer	args( ( DESCRIPTOR_DATA *d ) );
void	stop_idling		args( ( CHAR_DATA *ch ) );
void	free_desc		args( ( DESCRIPTOR_DATA *d ) );
void	display_prompt		args( ( DESCRIPTOR_DATA *d ) );
int	make_color_sequence	args( ( const char *col, char *buf,
					DESCRIPTOR_DATA *d ) );
void	set_pager_input		args( ( DESCRIPTOR_DATA *d,
					char *argument ) );
bool	pager_output		args( ( DESCRIPTOR_DATA *d ) );

void	mail_count		args( ( CHAR_DATA *ch ) );

void    tax_player		args( ( CHAR_DATA *ch ) );

int port;

#ifdef WIN32
  int mainthread( int argc, char **argv )
#else
  int main( int argc, char **argv )
#endif
{
    struct timeval now_time;
    char hostn[128];

    /*
     * Memory debugging if needed.
     */
#if defined(MALLOC_DEBUG)
    malloc_debug( 2 );
#endif

    DONT_UPPER			= FALSE;
    num_descriptors		= 0;
    first_descriptor		= NULL;
    last_descriptor		= NULL;
    sysdata.NO_NAME_RESOLVING	= TRUE;
    sysdata.WAIT_FOR_AUTH	= TRUE;

    /*
     * Init time.
     */
    gettimeofday( &now_time, NULL );
    current_time = (time_t) now_time.tv_sec;
/*  gettimeofday( &boot_time, NULL);   okay, so it's kludgy, sue me :) */
    boot_time = time(0);         /*  <-- I think this is what you wanted */
    strcpy( str_boot_time, ctime( &current_time ) );

    /*
     * Init boot time.
     */
    set_boot_time = &set_boot_time_struct;
    set_boot_time->manual = 0;
    
    new_boot_time = update_time(localtime(&current_time));
    /* Copies *new_boot_time to new_boot_struct, and then points
       new_boot_time to new_boot_struct again. -- Alty */
    new_boot_struct = *new_boot_time;
    new_boot_time = &new_boot_struct;
    new_boot_time->tm_mday += 1;
    if(new_boot_time->tm_hour > 12)
    	new_boot_time->tm_mday += 1;
    new_boot_time->tm_sec = 0;
    new_boot_time->tm_min = 0;
    new_boot_time->tm_hour = 6;

    /* Update new_boot_time (due to day increment) */
    new_boot_time = update_time(new_boot_time);
    new_boot_struct = *new_boot_time;
    new_boot_time = &new_boot_struct;
    /* Bug fix submitted by Gabe Yoder */
    new_boot_time_t = mktime(new_boot_time);
    reboot_check(mktime(new_boot_time));
    /* Set reboot time string for do_time */
    get_reboot_string();

    /*
     * Reserve two channels for our use.
     */
    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
	perror( NULL_FILE );
	exit( 1 );
    }
    if ( ( fpLOG = fopen( NULL_FILE, "r" ) ) == NULL )
    {
	perror( NULL_FILE );
	exit( 1 );
    }

    /*
     * Get the port number.
     */
    port = 4000;
    if ( argc > 1 )
    {
	if ( !is_number( argv[1] ) )
	{
	    fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
	    exit( 1 );
	}
	else if ( ( port = atoi( argv[1] ) ) <= 1024 )
	{
	    fprintf( stderr, "Port number must be above 1024.\n" );
	    exit( 1 );
	}
    }

    /*
     * Run the game.
     */
#ifdef WIN32
    {
	/* Initialise Windows sockets library */

	unsigned short wVersionRequested = MAKEWORD(1, 1);
	WSADATA wsadata;
	int err;

	/* Need to include library: wsock32.lib for Windows Sockets */
	err = WSAStartup(wVersionRequested, &wsadata);
	if (err)
	{
	    fprintf(stderr, "Error %i on WSAStartup\n", err);
	    exit(1);
	}

	/* standard termination signals */
	signal(SIGINT, (void *) bailout);
	signal(SIGTERM, (void *) bailout);
  }
#endif /* WIN32 */

    log_string("Booting Database");
    boot_db( );
    log_string("Initializing socket");
    control  = init_socket( port   );
    control2 = init_socket( port+1 );
    conclient= init_socket( port+10);
    conjava  = init_socket( port+20);
    
#ifdef OLD_IMC
    /* Be sure to change RoD to your mud's name! */
    if(port == 4000)
	imc_startup ("RoD", port+5, "imc/");
    else
	imc_startup("RoDBLD", port+5, "imc/");
#else
#ifdef USE_IMC
    imc_startup ("imc/");
    icec_init();
#endif
#endif

    /* I don't know how well this will work on an unnamed machine as I don't
       have one handy, and the man pages are ever-so-helpful.. -- Alty */
    if (gethostname(hostn, sizeof(hostn)) < 0)
    {
      perror("main: gethostname");
      strcpy(hostn, "unresolved");
    }
    sprintf( log_buf, "%s ready at address %s on port %d.",
		sysdata.mud_name, hostn, port );
/*
    sprintf( log_buf, "Realms of Despair ready at address %s on port %d.",
	hostn, port );
*/
    log_string( log_buf );

    game_loop( );
    
#ifdef USE_IMC
    imc_shutdown(); /* shut down IMC */
#endif
    
    closesocket( control  );
    closesocket( control2 );
    closesocket( conclient);
    closesocket( conjava  );

#ifdef WIN32
    if (service_shut_down)
    {
	CHAR_DATA *vch;

	/* Save all characters before booting. */
	for ( vch = first_char; vch; vch = vch->next )
	    if ( !IS_NPC( vch ) )
	    {
		shutdown_checkpoint ();
		save_char_obj( vch );
	    }
	}
    /* Shut down Windows sockets */
    
    WSACleanup();                 /* clean up */
    kill_timer();                 /* stop timer thread */
#endif


    /*
     * That's all, folks.
     */
    log_string( "Normal termination of game." );
    exit( 0 );
    return 0;
}


int init_socket( int port )
{
    char hostname[64];
    struct sockaddr_in	 sa;
    int x = 1;
    int fd;

    gethostname(hostname, sizeof(hostname));
    

    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
	perror( "Init_socket: socket" );
	exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
		    (void *) &x, sizeof(x) ) < 0 )
    {
	perror( "Init_socket: SO_REUSEADDR" );
	closesocket( fd );
	exit( 1 );
    }

#if defined(SO_DONTLINGER) && !defined(SYSV)
    {
	struct	linger	ld;

	ld.l_onoff  = 1;
	ld.l_linger = 1000;

	if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
			(void *) &ld, sizeof(ld) ) < 0 )
	{
	    perror( "Init_socket: SO_DONTLINGER" );
	    closesocket( fd );
	    exit( 1 );
	}
    }
#endif

    memset(&sa, '\0', sizeof(sa));
    sa.sin_family   = AF_INET;
    sa.sin_port	    = htons( port );

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) == -1 )
    {
	perror( "Init_socket: bind" );
	closesocket( fd );
	exit( 1 );
    }

    if ( listen( fd, 50 ) < 0 )
    {
	perror( "Init_socket: listen" );
	closesocket( fd );
	exit( 1 );
    }

    return fd;
}

/*
static void SegVio()
{
  CHAR_DATA *ch;
  char buf[MAX_STRING_LENGTH];

  log_string( "SEGMENTATION VIOLATION" );
  log_string( lastplayercmd );
  for ( ch = first_char; ch; ch = ch->next )
  {
    sprintf( buf, "%cPC: %-20s room: %d", IS_NPC(ch) ? 'N' : ' ',
    		ch->name, ch->in_room->vnum );
    log_string( buf );  
  }
  exit(0);
}
*/

/*
 * LAG alarm!							-Thoric
 */
void caught_alarm()
{
    char buf[MAX_STRING_LENGTH];

    // Fix from Igor Romanenko
    sprintf( buf, "ALARM CLOCK!  In section %s", alarm_section );
    bug( buf );
    strcpy( buf, "Alas, the hideous malevalent entity known only as 'Lag' rises once more!\n\r" );
    echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
    if ( newdesc )
    {
	FD_CLR( newdesc, &in_set );
	FD_CLR( newdesc, &out_set );
	FD_CLR( newdesc, &exc_set );
	log_string( "clearing newdesc" );
    }
}

bool check_bad_desc( int desc )
{
    if ( FD_ISSET( desc, &exc_set ) )
    {
	FD_CLR( desc, &in_set );
	FD_CLR( desc, &out_set );
	log_string( "Bad FD caught and disposed." );
	return TRUE;
    }
    return FALSE;
}

/*
 * Determine whether this player is to be watched  --Gorog
 */
bool chk_watch(sh_int player_level, char *player_name, char *player_site)
{
    WATCH_DATA *pw;
/*
    char buf[MAX_INPUT_LENGTH];
    sprintf( buf, "che_watch entry: plev=%d pname=%s psite=%s",
                  player_level, player_name, player_site);
    log_string(buf);
*/
    if ( !first_watch ) return FALSE;

    for ( pw = first_watch; pw; pw = pw->next )
    {
        if ( pw->target_name )
        {
           if ( !str_cmp(pw->target_name, player_name)
           &&   player_level < pw->imm_level )
                 return TRUE;
        }
        else 
        if ( pw->player_site )
        {
           if ( !str_prefix(pw->player_site, player_site)
           &&   player_level < pw->imm_level )
                 return TRUE;
        }
    }
    return FALSE; 
}


void accept_new( int ctrl )
{
	static struct timeval null_time;
	DESCRIPTOR_DATA *d;
	/* int maxdesc; Moved up for use with id.c as extern */

#if defined(MALLOC_DEBUG)
	if ( malloc_verify( ) != 1 )
	    abort( );
#endif

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( ctrl, &in_set );
	maxdesc	= ctrl;
	newdesc = 0;
	for ( d = first_descriptor; d; d = d->next )
	{
	    maxdesc = UMAX( maxdesc, d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
	    if ( d == last_descriptor )
	      break;
	}
	auth_maxdesc(&maxdesc, &in_set, &out_set, &exc_set);
	
#ifdef USE_IMC
	maxdesc=imc_fill_fdsets(maxdesc, &in_set, &out_set, &exc_set);
#endif

	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	    perror( "accept_new: select: poll" );
	    exit( 1 );
	}

	if ( FD_ISSET( ctrl, &exc_set ) )
	{
	    bug( "Exception raise on controlling descriptor %d", ctrl );
	    FD_CLR( ctrl, &in_set );
	    FD_CLR( ctrl, &out_set );
	}
	else
	if ( FD_ISSET( ctrl, &in_set ) )
	{
	    newdesc = ctrl;
	    new_descriptor( newdesc );
	}
}

void game_loop( )
{
    struct timeval	  last_time;
    char cmdline[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
/*  time_t	last_check = 0;  */

#ifndef WIN32
    signal( SIGPIPE, SIG_IGN );
    signal( SIGALRM, caught_alarm );
#endif

    /* signal( SIGSEGV, SegVio ); */
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /* Main loop */
    while ( !mud_down )
    {
	accept_new( control  );
	accept_new( control2 );
	accept_new( conclient);
	accept_new( conjava  );
	
	auth_check(&in_set, &out_set, &exc_set);
	
	/*
	 * Kick out descriptors with raised exceptions
	 * or have been idle, then check for input.
	 */
	for ( d = first_descriptor; d; d = d_next )
	{
	    if ( d == d->next )
	    {
	      bug( "descriptor_loop: loop found & fixed" );
	      d->next = NULL;
	    }
 	    d_next = d->next;   

	    d->idle++;	/* make it so a descriptor can idle out */
	    if ( FD_ISSET( d->descriptor, &exc_set ) )
	    {
		FD_CLR( d->descriptor, &in_set  );
		FD_CLR( d->descriptor, &out_set );
		if ( d->character
		&& ( d->connected == CON_PLAYING
		||   d->connected == CON_EDITING ) )
		    save_char_obj( d->character );
		d->outtop	= 0;
		close_socket( d, TRUE );
		continue;
	    }
	    else 
	    if ( (!d->character && d->idle > 360)		  /* 2 mins */
            ||   ( d->connected != CON_PLAYING && d->idle > 1200) /* 5 mins */
	    ||     d->idle > 28800 )				  /* 2 hrs  */
	    {
		write_to_descriptor( d->descriptor,
		 "Idle timeout... disconnecting.\n\r", 0 );
		d->outtop	= 0;
		close_socket( d, TRUE );
		continue;
	    }
	    else
	    {
		d->fcommand	= FALSE;

		if ( FD_ISSET( d->descriptor, &in_set ) )
		{
			d->idle = 0;
			if ( d->character )
			  d->character->timer = 0;
			if ( !read_from_descriptor( d ) )
			{
			    FD_CLR( d->descriptor, &out_set );
			    if ( d->character
			    && ( d->connected == CON_PLAYING
			    ||   d->connected == CON_EDITING ) )
				save_char_obj( d->character );
			    d->outtop	= 0;
			    close_socket( d, FALSE );
			    continue;
			}
		}

		if ( d->character && d->character->wait > 0 )
		{
			--d->character->wait;
			continue;
		}

		read_from_buffer( d );
		if ( d->incomm[0] != '\0' )
		{
			d->fcommand	= TRUE;
			stop_idling( d->character );

			strcpy( cmdline, d->incomm );
			d->incomm[0] = '\0';
			
			if ( d->character )
			  set_cur_char( d->character );

			if ( d->pagepoint )
			  set_pager_input(d, cmdline);
			else
			  switch( d->connected )
			  {
			   default:
 				nanny( d, cmdline );
				break;
			   case CON_PLAYING:
				interpret( d->character, cmdline );
				break;
			   case CON_EDITING:
				edit_buffer( d->character, cmdline );
				break;
			  }
		}
	    }
	    if ( d == last_descriptor )
	      break;
	}
	
#ifdef USE_IMC
	/* kick IMC */
	imc_idle_select(&in_set, &out_set, &exc_set, current_time);
#endif

	/*
	 * Autonomous game motion.
	 */
	update_handler( );

	/*
	 * Check REQUESTS pipe
	 */
        check_requests( );

	/*
	 * Output.
	 */
	for ( d = first_descriptor; d; d = d_next )
	{
	    d_next = d->next;   

	    if ( ( d->fcommand || d->outtop > 0 )
	    &&   FD_ISSET(d->descriptor, &out_set) )
	    {
	        if ( d->pagepoint )
	        {
	          if ( !pager_output(d) )
	          {
	            if ( d->character
	            && ( d->connected == CON_PLAYING
	            ||   d->connected == CON_EDITING ) )
	                save_char_obj( d->character );
	            d->outtop = 0;
	            close_socket(d, FALSE);
	          }
	        }
		else if ( !flush_buffer( d, TRUE ) )
		{
		    if ( d->character
		    && ( d->connected == CON_PLAYING
		    ||   d->connected == CON_EDITING ) )
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d, FALSE );
		}
	    }
	    if ( d == last_descriptor )
	      break;
	}
	
	/*
	 * Synchronize to a clock.
	 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	 * Careful here of signed versus unsigned arithmetic.
	 */
	{
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;

	    gettimeofday( &now_time, NULL );
	    usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SECOND;
	    secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
	    while ( usecDelta < 0 )
	    {
		usecDelta += 1000000;
		secDelta  -= 1;
	    }

	    while ( usecDelta >= 1000000 )
	    {
		usecDelta -= 1000000;
		secDelta  += 1;
	    }

	    if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
	    {
		struct timeval stall_time;

		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;
#ifdef WIN32
		Sleep( (stall_time.tv_sec * 1000L) + (stall_time.tv_usec / 1000L) );
#else
		if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 && errno != EINTR )
		{
		    perror( "game_loop: select: stall" );
		    exit( 1 );
		}
#endif
	    }
	}

	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;

        /* Check every 5 seconds...  (don't need it right now)
	if ( last_check+5 < current_time )
	{
	  CHECK_LINKS(first_descriptor, last_descriptor, next, prev,
	      DESCRIPTOR_DATA);
	  last_check = current_time;
	}
	*/
    }
    /*  Save morphs so can sort later. --Shaddai */
    if ( sysdata.morph_opt )
	save_morphs( );

    fflush(stderr);	/* make sure strerr is flushed */
    return;
}


void new_descriptor( int new_desc )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew;
    struct sockaddr_in sock;
    struct hostent *from;
    int desc;
    int size;
    char bugbuf[MAX_STRING_LENGTH];
#ifdef WIN32
    unsigned long arg = 1;
#endif

    size = sizeof(sock);
    if ( check_bad_desc( new_desc ) )
    {
      set_alarm( 0 );
      return;
    }
    set_alarm( 20 );
    alarm_section = "new_descriptor::accept";
    if ( ( desc = accept( new_desc, (struct sockaddr *) &sock, &size) ) < 0 )
    {
	perror( "New_descriptor: accept" );
	sprintf(bugbuf, "[*****] BUG: New_descriptor: accept");
	log_string_plus( bugbuf, LOG_COMM, sysdata.log_level );
	set_alarm( 0 );
	return;
    }
    if ( check_bad_desc( new_desc ) )
    {
      set_alarm( 0 );
      return;
    }
#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    set_alarm( 20 );
    alarm_section = "new_descriptor: after accept";

#ifdef WIN32
    if ( ioctlsocket(desc, FIONBIO, &arg) == -1 )
#else
    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
#endif
    {
	perror( "New_descriptor: fcntl: FNDELAY" );
	set_alarm( 0 );
	return;
    }
    if ( check_bad_desc( new_desc ) )
      return;

    CREATE( dnew, DESCRIPTOR_DATA, 1 );
    dnew->next		= NULL;
    dnew->descriptor	= desc;
    dnew->connected	= CON_GET_NAME;
    dnew->outsize	= 2000;
    dnew->idle		= 0;
    dnew->lines		= 0;
    dnew->scrlen	= 24;
    dnew->port		= ntohs( sock.sin_port );
    dnew->user 		= STRALLOC("(unknown)");
    dnew->newstate	= 0;
    dnew->prevcolor	= 0x07;

    CREATE( dnew->outbuf, char, dnew->outsize );

    strcpy( buf, inet_ntoa( sock.sin_addr ) );
    sprintf( log_buf, "Sock.sinaddr:  %s, port %hd.",
		buf, dnew->port );
    log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
    if ( sysdata.NO_NAME_RESOLVING )
      dnew->host = STRALLOC( buf );
    else
    {
       from = gethostbyaddr( (char *) &sock.sin_addr,
	  	sizeof(sock.sin_addr), AF_INET );
       dnew->host = STRALLOC( (char *)( from ? from->h_name : buf) );
    }

    if ( check_total_bans( dnew ) )
    {
          write_to_descriptor (desc,
                         "Your site has been banned from this Mud.\n\r", 0);
          free_desc (dnew);
          set_alarm (0);
          return;
     }

    /*
     * Init descriptor data.
     */

    if ( !last_descriptor && first_descriptor )
    {
	DESCRIPTOR_DATA *d;

	bug( "New_descriptor: last_desc is NULL, but first_desc is not! ...fixing" );
	for ( d = first_descriptor; d; d = d->next )
	   if ( !d->next )
		last_descriptor = d;
    }

    LINK( dnew, first_descriptor, last_descriptor, next, prev );

    /*
     * Send the greeting.
     */
    {
	extern char * help_greeting;
/*
	write_to_buffer( dnew, "\x1b[1;32m", 0 );
*/

	if ( help_greeting[0] == '.' )
	    write_to_buffer( dnew, help_greeting+1, 0 );
	else
	    write_to_buffer( dnew, help_greeting  , 0 );
    }

    alarm_section = "new_descriptor: set_auth";
    set_auth(dnew);
    alarm_section = "new_descriptor: after set_auth";

    if ( ++num_descriptors > sysdata.maxplayers )
	sysdata.maxplayers = num_descriptors;
    if ( sysdata.maxplayers > sysdata.alltimemax )
    {
	if ( sysdata.time_of_max )
	  DISPOSE(sysdata.time_of_max);
	sprintf(buf, "%24.24s", ctime(&current_time));
	sysdata.time_of_max = str_dup(buf);
	sysdata.alltimemax = sysdata.maxplayers;
	sprintf( log_buf, "Broke all-time maximum player record: %d", sysdata.alltimemax );
	log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
	to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
	save_sysdata( sysdata );
    }
    set_alarm(0);
    return;
}

void free_desc( DESCRIPTOR_DATA *d )
{
    kill_auth(d);
    closesocket( d->descriptor );
    STRFREE( d->host );
    DISPOSE( d->outbuf );
    STRFREE( d->user );    /* identd */
    if ( d->pagebuf )
	DISPOSE( d->pagebuf );
    DISPOSE( d );
/*    --num_descriptors;  This is called from more than close_socket -- Alty */
    return;
}

void close_socket( DESCRIPTOR_DATA *dclose, bool force )
{
    CHAR_DATA *ch;
    DESCRIPTOR_DATA *d;
    bool DoNotUnlink = FALSE;

    /* flush outbuf */
    if ( !force && dclose->outtop > 0 )
	flush_buffer( dclose, FALSE );

    /* say bye to whoever's snooping this descriptor */
    if ( dclose->snoop_by )
	write_to_buffer( dclose->snoop_by,
	    "Your victim has left the game.\n\r", 0 );

    /* stop snooping everyone else */
    for ( d = first_descriptor; d; d = d->next )
	if ( d->snoop_by == dclose )
	  d->snoop_by = NULL;

    /* Check for switched people who go link-dead. -- Altrag */
    if ( dclose->original )
    {
	if ( ( ch = dclose->character ) != NULL )
	  do_return(ch, "");
	else
	{
	  bug( "Close_socket: dclose->original without character %s",
		(dclose->original->name ? dclose->original->name : "unknown") );
	  dclose->character = dclose->original;
	  dclose->original = NULL;
	}
    }
    
    ch = dclose->character;

    /* sanity check :( */
    if ( !dclose->prev && dclose != first_descriptor )
    {
	DESCRIPTOR_DATA *dp, *dn;
	bug( "Close_socket: %s desc:%p != first_desc:%p and desc->prev = NULL!",
		ch ? ch->name : d->host, dclose, first_descriptor );
	dp = NULL;
	for ( d = first_descriptor; d; d = dn )
	{
	   dn = d->next;
	   if ( d == dclose )
	   {
		bug( "Close_socket: %s desc:%p found, prev should be:%p, fixing.",
		    ch ? ch->name : d->host, dclose, dp );
		dclose->prev = dp;
		break;
	   }
	   dp = d;
	}
	if ( !dclose->prev )
	{
	    bug( "Close_socket: %s desc:%p could not be found!.",
		    ch ? ch->name : dclose->host, dclose );
	    DoNotUnlink = TRUE;
	}
    }
    if ( !dclose->next && dclose != last_descriptor )
    {
	DESCRIPTOR_DATA *dp, *dn;
	bug( "Close_socket: %s desc:%p != last_desc:%p and desc->next = NULL!",
		ch ? ch->name : d->host, dclose, last_descriptor );
	dn = NULL;
	for ( d = last_descriptor; d; d = dp )
	{
	   dp = d->prev;
	   if ( d == dclose )
	   {
		bug( "Close_socket: %s desc:%p found, next should be:%p, fixing.",
		    ch ? ch->name : d->host, dclose, dn );
		dclose->next = dn;
		break;
	   }
	   dn = d;
	}
	if ( !dclose->next )
	{
	    bug( "Close_socket: %s desc:%p could not be found!.",
		    ch ? ch->name : dclose->host, dclose );
	    DoNotUnlink = TRUE;
	}
    }

    if ( dclose->character )
    {
        sprintf( log_buf, "Closing link to %s. (INRoom %d)",
          ch->pcdata->filename,
          ( ch->in_room ? ch->in_room->vnum : -1 ) );
	log_string_plus( log_buf, LOG_COMM, UMAX( sysdata.log_level, ch->level ) );
	if ( ch && ch->fighting )
	{
	  struct   tm *t = localtime(&current_time);

	  sprintf( log_buf, "%d/%d, %d:%2d, %s, %s, %d/%d, %d",
		t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min,
		ch->pcdata->filename,
		ch->fighting->who ? ch->fighting->who->name : "",
		ch->fighting->who ? ch->fighting->who->hit : -1,
		ch->fighting->who ? ch->fighting->who->max_hit : -1,
		ch->in_room       ? ch->in_room->vnum : -1 );
 	  append_to_file( CUTLINK_FILE, log_buf );
	}

/*
	if ( ch->level < LEVEL_DEMI )
	  to_channel( log_buf, CHANNEL_MONITOR, "Monitor", ch->level );
*/
        if ( dclose->connected == CON_EDITING ) {
          if ( ch->last_cmd )
             ch->last_cmd( ch, "" );
          else
             stop_editing( ch );
          dclose->connected = CON_PLAYING;
        }

	if ( dclose->connected == CON_PLAYING
	||   dclose->connected == CON_EDITING )
	{
	    act( AT_ACTION, "$n has lost $s link.", ch, NULL, NULL, TO_CANSEE );
	    ch->desc = NULL;
	}
	else
	{
	    /* clear descriptor pointer to get rid of bug message in log */
	    dclose->character->desc = NULL;
	    free_char( dclose->character );
	}
    }


    if ( !DoNotUnlink )
    {
	/* make sure loop doesn't get messed up */
	if ( d_next == dclose )
	  d_next = d_next->next;
	UNLINK( dclose, first_descriptor, last_descriptor, next, prev );
    }

    if ( dclose->descriptor == maxdesc )
      --maxdesc;

    free_desc( dclose );
    --num_descriptors;
    return;
}


bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    int iStart, iErr;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;

    /* Check for overflow. */
    iStart = strlen(d->inbuf);
    if ( iStart >= sizeof(d->inbuf) - 10 )
    {
	sprintf( log_buf, "%s input overflow!", d->host );
	log_string( log_buf );
	write_to_descriptor( d->descriptor,
	    "\n\r*** PUT A LID ON IT!!! ***\n\rYou cannot enter the same command more than 20 consecutive times!\n\r", 0 );
	return FALSE;
    }

    for ( ; ; )
    {
	int nRead;

	nRead = recv( d->descriptor, d->inbuf + iStart,
	    sizeof(d->inbuf) - 10 - iStart, 0 );
#ifdef WIN32
	iErr = WSAGetLastError ();
#else
	iErr = errno;
#endif
	if ( nRead > 0 )
	{
	    iStart += nRead;
	    if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
		break;
	}
	else if ( nRead == 0 )
	{
	    log_string_plus( "EOF encountered on read.", LOG_COMM, sysdata.log_level );
	    return FALSE;
	}
	else if ( iErr == EWOULDBLOCK )
	    break;
	else
	{
	    perror( "Read_from_descriptor" );
	    return FALSE;
	}
    }

    d->inbuf[iStart] = '\0';
    return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
    int i, j, k;

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )
	return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r' && i<MAX_INBUF_SIZE;
	  i++ )
    {
	if ( d->inbuf[i] == '\0' )
	    return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( k >= 254 )
	{
	    write_to_descriptor( d->descriptor, "Line too long.\n\r", 0 );

	    /* skip the rest of the line */
	    /*
	    for ( ; d->inbuf[i] != '\0' || i>= MAX_INBUF_SIZE ; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    */
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */
    if ( k > 1 || d->incomm[0] == '!' )
    {
	if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
	{
	    d->repeat = 0;
	}
	else
	{
	    if ( ++d->repeat >= 20 )
	    {
/*		sprintf( log_buf, "%s input spamming!", d->host ); */
/*		log_string( log_buf ); */
		write_to_descriptor( d->descriptor,
		    "\n\r*** PUT A LID ON IT!!! ***\n\rYou cannot enter the same command more than 20 consecutive times!\n\r", 0 );
		strcpy( d->incomm, "quit" );
	    }
	}
    }

    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
	strcpy( d->incomm, d->inlast );
    else
	strcpy( d->inlast, d->incomm );

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}


/*
 * Low level output function.
 */
bool flush_buffer( DESCRIPTOR_DATA *d, bool fPrompt )
{
    char buf[MAX_INPUT_LENGTH];
    extern bool mud_down;

    /*
     * If buffer has more than 4K inside, spit out .5K at a time   -Thoric
     */
    if ( !mud_down && d->outtop > 4096 )
    {
	memcpy( buf, d->outbuf, 512 );
	d->outtop -= 512;
	memmove( d->outbuf, d->outbuf + 512, d->outtop );
	if ( d->snoop_by )
	{
	    char snoopbuf[MAX_INPUT_LENGTH];

	    buf[512] = '\0';
	    if ( d->character && d->character->name )
	    {
		if (d->original && d->original->name)
		    sprintf( snoopbuf, "%s (%s)", d->character->name, d->original->name );
		else          
		    sprintf( snoopbuf, "%s", d->character->name);
		write_to_buffer( d->snoop_by, snoopbuf, 0);
	    }
	    write_to_buffer( d->snoop_by, "% ", 2 );
	    write_to_buffer( d->snoop_by, buf, 0 );
	}
        if ( !write_to_descriptor( d->descriptor, buf, 512 ) )
        {
	    d->outtop = 0;
	    return FALSE;
        }
        return TRUE;
    }
                                                                                        

    /*
     * Bust a prompt.
     */
    if ( fPrompt && !mud_down && d->connected == CON_PLAYING )
    {
	CHAR_DATA *ch;

	ch = d->original ? d->original : d->character;
	if ( xIS_SET(ch->act, PLR_BLANK) )
	    write_to_buffer( d, "\n\r", 2 );

	    
   /* Let's add a color code if -prompt. It's a dynamic world
      and this will nearly eradicate color bleeding - Luc 09/2000 */

   if ( xIS_SET(ch->act, PLR_PROMPT) )
      display_prompt(d);
   else
      set_char_color( AT_ACTION, ch );


	if ( xIS_SET(ch->act, PLR_TELNET_GA) )
	    write_to_buffer( d, go_ahead_str, 0 );
    }

    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
	return TRUE;

    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by )
    {
        /* without check, 'force mortal quit' while snooped caused crash, -h */
	if ( d->character && d->character->name )
	{
	    /* Show original snooped names. -- Altrag */
	    if ( d->original && d->original->name )
		sprintf( buf, "%s (%s)", d->character->name, d->original->name );
	    else
		sprintf( buf, "%s", d->character->name);
	    write_to_buffer( d->snoop_by, buf, 0);
	}
	write_to_buffer( d->snoop_by, "% ", 2 );
	write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }

    /*
     * OS-dependent output.
     */
    if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop ) )
    {
	d->outtop = 0;
	return FALSE;
    }
    else
    {
	d->outtop = 0;
	return TRUE;
    }
}



/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
    if ( !d )
    {
	bug( "Write_to_buffer: NULL descriptor" );
	return;
    }

    if ( MPSilent )
	return;

    /*
     * Normally a bug... but can happen if loadup is used.
     */
    if ( !d->outbuf )
    	return;

    /*
     * Find length in case caller didn't.
     */
    if ( length <= 0 )
	length = strlen(txt);

/* Uncomment if debugging or something
    if ( length != strlen(txt) )
    {
	bug( "Write_to_buffer: length(%d) != strlen(txt)!", length );
	length = strlen(txt);
    }
*/

    /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !d->fcommand )
    {
	d->outbuf[0]	= '\n';
	d->outbuf[1]	= '\r';
	d->outtop	= 2;
    }

    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )
    {
        if (d->outsize > 32000) /* pagerfile size */
	{
	    /* empty buffer */
	    d->outtop = 0;
	    close_socket(d, TRUE);
	    bug("Buffer overflow. Closing (%s).", d->character ? d->character->name : "???" );
	    return;
 	}
	d->outsize *= 2;
	RECREATE( d->outbuf, char, d->outsize );
    }

    /*
     * Copy.
     */
    strncpy( d->outbuf + d->outtop, txt, length );
    d->outtop += length;
    d->outbuf[d->outtop] = '\0';
    return;
}


/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor( int desc, char *txt, int length )
{
    int iStart;
    int nWrite;
    int nBlock;

    if ( length <= 0 )
	length = strlen(txt);

    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
	nBlock = UMIN( length - iStart, 4096 );
	if ( ( nWrite = send( desc, txt + iStart, nBlock, 0 ) ) < 0 )
	    { perror( "Write_to_descriptor" ); return FALSE; }
    }

    return TRUE;
}

void show_title( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *ch;

    ch = d->character;

    if ( !IS_SET( ch->pcdata->flags, PCFLAG_NOINTRO ) )
    {
	if (xIS_SET(ch->act, PLR_RIP))
	  send_rip_title(ch);
	else
	if (xIS_SET(ch->act, PLR_ANSI))
	  send_ansi_title(ch);
	else
	  send_ascii_title(ch);
    }
    else
    {
      write_to_buffer( d, "Press enter...\n\r", 0 );
    }
    d->connected = CON_PRESS_ENTER;
}

void show_classes_to_nanny( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *ch;
    sh_int     cnt = 0;
    int        iClass;

    ch = d->character;

    ch_printf_color( ch, "\n\r\n\r&GThe following classes are available (or type &W'help [class]'&G)\n\r" );
    for ( iClass = 0; iClass < MAX_PC_CLASS-3; iClass++ )
    {	
	if ( class_table[iClass]->who_name
	&&   class_table[iClass]->who_name[0] != '\0' )
      {
	    if ( iClass > 0 )
	    cnt++;
	}
	ch_printf_color( ch, "&W %-15.15s ", class_table[iClass]->who_name );
	if ( cnt == 3 )
	{
	    send_to_char( "\n\r", ch );
	    cnt = 0;
	}
    }
    ch_printf_color( ch, "\n\r&GPlease select\n\r&W: " );
    return;
}

void show_races_to_nanny( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *ch;
    sh_int     cnt = 0;
    int        iRace;

    ch = d->character;

    ch_printf_color( ch, "\n\r\n\r&GThe following races are available to your class (or type &W'help [race]'&G)\n\r" );
    for ( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
    {
	if ( iRace != RACE_VAMPIRE
	&&   race_table[iRace]->race_name && race_table[iRace]->race_name[0] != '\0'
	&&  !IS_SET( race_table[iRace]->class_restriction, 1 << ch->class )
	&&   str_cmp( race_table[iRace]->race_name, "unused" ) )
	{
	    if ( iRace >= 0 )
	    {
		cnt++;
		ch_printf_color( ch, " &W%-15.15s ", race_table[iRace]->race_name );
	    }
	    if ( cnt == 3 )
	    {
		send_to_char( "\n\r", ch );
		cnt = 0;
	    }
	}
    }
    ch_printf_color( ch, "\n\r&GPlease select\n\r&W: ");
    return;
}

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
/*	extern int lang_array[];
	extern char *lang_names[];*/
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    CHAR_DATA *ch;
    char *pwdnew;
    char *p;
    int iClass;
    int iRace;
    bool fOld, chk;

    while ( isspace( *argument ) )
	argument++;

    ch = d->character;

    switch ( d->connected )
    {

    default:
	bug( "Nanny: bad d->connected %d.", d->connected );
	close_socket( d, TRUE );
	return;

    case CON_GET_NAME:
	if ( argument[0] == '\0' )
	{
	    close_socket( d, FALSE );
	    return;
	}

	argument[0] = UPPER(argument[0]);

	/* Old players can keep their characters. -- Alty */
	if ( !check_parse_name( argument, (d->newstate != 0) ) )
	{
	    write_to_buffer( d, "That name is reserved, please try another.\n\rName: ", 0 );
	    return;
	}
                                                
        if ( !str_cmp( argument, "New" ) )
	  {
	    if ( d->newstate == 0 )
	    {
              /* New player */
              /* Don't allow new players if DENY_NEW_PLAYERS is true */
      	      if (sysdata.DENY_NEW_PLAYERS == TRUE)
      	      {
       		sprintf( buf, "The mud is currently preparing for a reboot.\n\r" );
      		write_to_buffer( d, buf, 0 );
			sprintf( buf, "New players are not accepted during this time.\n\r" );
			write_to_buffer( d, buf, 0 );
      		sprintf( buf, "Please try again in a few minutes.\n\r" );
      		write_to_buffer( d, buf, 0 );
			close_socket( d, FALSE );
              }
              sprintf( buf, "\n\rChoosing a name is one of the most important parts of this game...\n\r"
              			"Make sure to pick a name appropriate to the character you are going\n\r"
               			"to role play, and be sure that it suits a medieval theme.\n\r"
               			"If the name you select is not acceptable, you will be asked to choose\n\r"
               			"another one.\n\r\n\rPlease choose a name for your character: ");
              write_to_buffer( d, buf, 0 );
	      d->newstate++;
	      d->connected = CON_GET_NAME;
	      return;
	    }
	    else
   	    {
	      write_to_buffer(d, "That name is reserved, please try another.\n\rName: ", 0);
	      return;
	    }
	}

	if ( check_playing( d, argument, FALSE ) == BERR )
	{
	    write_to_buffer( d, "Name: ", 0 );
	    return;
	}

	fOld = load_char_obj( d, argument, TRUE );
	if ( !d->character )
	{
	    sprintf( log_buf, "Bad player file %s@%s.", argument, d->host );
	    log_string( log_buf );
	    write_to_buffer( d, "Your playerfile is corrupt...Please notify derek@idirect.com.\n\r", 0 );
	    close_socket( d, FALSE );
	    return;
	}
	ch   = d->character;
      if ( check_bans( ch, BAN_SITE ) )
      {
              write_to_buffer (d,
                         "Your site has been banned from this Mud.\n\r", 0);
              close_socket (d, FALSE);
              return;
      }

      if ( fOld ) {
      if ( check_bans( ch, BAN_CLASS ) )
      {
              write_to_buffer (d,
                         "Your class has been banned from this Mud.\n\r", 0);
              close_socket (d, FALSE);
              return;
      }
      if ( check_bans( ch, BAN_RACE ) )
      {
              write_to_buffer (d,
                         "Your race has been banned from this Mud.\n\r", 0);
              close_socket (d, FALSE);
              return;
      }
      }

	if ( xIS_SET(ch->act, PLR_DENY) )
	{
	    sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
	    log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
	    if (d->newstate != 0)
	    {
              write_to_buffer( d, "That name is already taken.  Please choose another: ", 0 );
	      d->connected = CON_GET_NAME;
	      d->character->desc = NULL;
	      free_char( d->character ); /* Big Memory Leak before --Shaddai */
	      d->character = NULL;
	      return;
	    }
	    ch_printf_color( ch, "\n\r&RYou are denied access.\n\r" );
	    close_socket( d, FALSE );
	    return;
	}
      /*
       *  Make sure the immortal host is from the correct place.
       *  Shaddai
       */

      if ( IS_IMMORTAL( ch ) && sysdata.check_imm_host
      &&  !check_immortal_domain( ch , d->host) )
      {
        sprintf (log_buf, "%s's char being hacked from %s.", argument, d->host);
        log_string_plus (log_buf, LOG_COMM, sysdata.log_level);
	  ch_printf_color( ch, "\n\r&RThis hacking attempt has been logged.\n\r" );
        close_socket (d, FALSE);
        return;
      }

	chk = check_reconnect( d, argument, FALSE );
	if ( chk == BERR )
	  return;

	if ( chk )
	{
	    fOld = TRUE;
	}
	else
	{
	    if ( wizlock && !IS_IMMORTAL( ch ) )
	    {
		ch_printf_color( ch, "\n\r&RThe game is wizlocked.  Only immortals can connect at this time.\n\r" );
		ch_printf_color( ch, "Please try back later ...\n\r" );
		close_socket( d, FALSE );
		return;
	    }
	}

	if ( fOld )
	{
	    if (d->newstate != 0)
	    {
	      write_to_buffer( d, "That name is already taken.  Please choose another: ", 0 );
	      d->connected = CON_GET_NAME;
	      d->character->desc = NULL;
	      free_char( d->character ); /* Big Memory Leak before --Shaddai */
	      d->character = NULL;
	      return;
	    }
	    /* Old player */
	    write_to_buffer( d, "Password: ", 0 );
	    write_to_buffer( d, echo_off_str, 0 );
	    d->connected = CON_GET_OLD_PASSWORD;
	    return;
	}
	else
	{
	    /*if ( !check_parse_name( argument ) )
	    {
		write_to_buffer( d, "That name is reserved, please try another.\n\rName: ", 0 );
		return;
	    }*/

	    if (d->newstate == 0)
	    {
	      /* No such player */
	      write_to_buffer( d, "\n\rNo such player exists.\n\rPlease check your spelling, or type new to start a new player.\n\r\n\rName: ", 0 );
	      d->connected = CON_GET_NAME;
	      d->character->desc = NULL;
	      free_char( d->character ); /* Big Memory Leak before --Shaddai */
	      d->character = NULL;
	      return;
	    }

            sprintf( buf, "Did I get that right, %s (Y/N)? ", argument );
            write_to_buffer( d, buf, 0 );
            d->connected = CON_CONFIRM_NEW_NAME;
	    return;
	}
	break;

    case CON_GET_OLD_PASSWORD:
	write_to_buffer( d, "\n\r", 2 );

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, "Wrong password.\n\r", 0 );
	    /* clear descriptor pointer to get rid of bug message in log */
	    d->character->desc = NULL;
	    close_socket( d, FALSE );
	    return;
	}

	write_to_buffer( d, echo_on_str, 0 );

	if ( check_playing( d, ch->pcdata->filename, TRUE ) )
	    return;

	chk = check_reconnect( d, ch->pcdata->filename, TRUE );
	if ( chk == BERR )
	{
	    if ( d->character && d->character->desc )
	      d->character->desc = NULL;
	    close_socket( d, FALSE );
	    return;
	}
	if ( chk == TRUE )
	  return;

	sprintf( buf, ch->pcdata->filename );
	d->character->desc = NULL;
	free_char( d->character );
	d->character = NULL;
	fOld = load_char_obj( d, buf, FALSE );
	if ( !d->character )
	{
	    sprintf( log_buf, "Bad player file %s@%s.", argument, d->host );
	    log_string( log_buf );
	    write_to_buffer( d, "Your playerfile is corrupt...Please notify derek@idirect.com\n\r", 0 );
	    close_socket( d, FALSE );
	    return;
	}
	ch = d->character;
	
	if ( ch->position ==  POS_FIGHTING
        ||   ch->position ==  POS_EVASIVE
        ||   ch->position ==  POS_DEFENSIVE
        ||   ch->position ==  POS_AGGRESSIVE
        ||   ch->position ==  POS_BERSERK )
	       ch->position = POS_STANDING;


	sprintf( buf2, "%s , %s",
	  ch->pcdata->filename,
	  d->host );
	append_to_file( CHARCOUNT_FILE, buf2 );
	sprintf( log_buf, "%s@%s(%s) has connected. (INRoom %d)",
          ch->pcdata->filename,
          d->host, d->user,
          ( ch->in_room ? ch->in_room->vnum : -1 ) );

	if ( ch->level < LEVEL_DEMI )
	{
	  /*to_channel( log_buf, CHANNEL_MONITOR, "Monitor", ch->level );*/
	  log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
	}
	else
	  log_string_plus( log_buf, LOG_COMM, ch->level );
	show_title(d);
	break;

     case CON_CONFIRM_NEW_NAME:
	switch ( *argument )
	{
	case 'y': case 'Y':
	    sprintf( buf, "\n\rMake sure to use a password that won't be easily guessed by someone else."
	    		  "\n\rPick a good password for %s: %s",
		ch->name, echo_off_str );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    break;

	case 'n': case 'N':
	    write_to_buffer( d, "Ok, what IS it, then? ", 0 );
	    /* clear descriptor pointer to get rid of bug message in log */
	    d->character->desc = NULL;
	    free_char( d->character );
	    d->character = NULL;
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer( d, "Please type Yes or No. ", 0 );
	    break;
	}
	break;

    case CON_GET_NEW_PASSWORD:
	write_to_buffer( d, "\n\r", 2 );

	if ( strlen(argument) < 5 )
	{
	    write_to_buffer( d,
		"Password must be at least five characters long.\n\rPassword: ",
		0 );
	    return;
	}

	pwdnew = crypt( argument, ch->name );
	for ( p = pwdnew; *p != '\0'; p++ )
	{
	    if ( *p == '~' )
	    {
		write_to_buffer( d,
		    "New password not acceptable, try again.\n\rPassword: ",
		    0 );
		return;
	    }
	}

	DISPOSE( ch->pcdata->pwd );
	ch->pcdata->pwd	= str_dup( pwdnew );
	write_to_buffer( d, "\n\rPlease retype the password to confirm: ", 0 );
	d->connected = CON_CONFIRM_NEW_PASSWORD;
	break;

    case CON_CONFIRM_NEW_PASSWORD:
	write_to_buffer( d, "\n\r", 2 );

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, "Passwords don't match.\n\rRetype password: ",
		0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    return;
	}

	write_to_buffer( d, "\n\rPlease select your color preference\n\r ANSI\n\r RIP\n\r No color support\n\r: ", 0 );
	d->connected = CON_GET_WANT_RIPANSI;
        break;


    case CON_GET_WANT_RIPANSI:
	write_to_buffer( d, echo_on_str, 0 );
	switch ( argument[0] )
	{
	case 'r': case 'R':
	    xSET_BIT(ch->act,PLR_RIP);
	    xSET_BIT(ch->act,PLR_ANSI);
	    break;
	case 'a': case 'A': xSET_BIT(ch->act,PLR_ANSI);  break;
	case 'n': case 'N': break;
	default:
	    write_to_buffer( d, "Invalid selection\n\r ANSI\n\r RIP\n\r No color support\n\r: ", 0 );
	    return;
	}

	ch_printf_color( ch, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r" );
	ch_printf_color( ch, "&G&gRealms of Despair greets you, &G%s ...\n\r", capitalize( ch->name ) );
	ch_printf_color( ch, "&gPlease continue and choose your gender, class and race.\n\r" );
        ch_printf_color( ch, "&G&g================================================================\n\r" );
	ch_printf_color( ch, "\n\r&GWhich gender will your character be?\n\r &WMale\n\r Female\n\r Neutral\n\r&GPlease select\n\r&w: " );
	d->connected = CON_GET_NEW_SEX;
	break;

      case CON_GET_NEW_SEX:
	switch ( argument[0] )
	{
	case 'm': case 'M': ch->sex = SEX_MALE;    break;
	case 'f': case 'F': ch->sex = SEX_FEMALE;  break;
	case 'n': case 'N': ch->sex = SEX_NEUTRAL; break;
	default:
	    ch_printf_color( ch, "\n\r&GWhich gender will your character be?\n\r &WMale\n\r Female\n\r Neutral\n\r&GPlease select\n\r&W: " );
	    return;
	}

	show_classes_to_nanny( d );
	d->connected = CON_GET_NEW_CLASS;
	break;

      case CON_GET_NEW_CLASS:
	argument = one_argument(argument, arg);

      if ( !str_cmp( arg, "help" ) )
      {
	    for ( iClass = 0; iClass < MAX_PC_CLASS; iClass++ )
	    {
		if ( class_table[iClass]->who_name
		&&   class_table[iClass]->who_name[0] != '\0' )
		{
		    if ( toupper( argument[0] )==toupper( class_table[iClass]->who_name[0] )
		    &&  !str_prefix( argument, class_table[iClass]->who_name ) )
		    {
			do_nanny_help( ch, argument );
			ch_printf_color( ch, "&G\n\rPlease select ( press &W[Enter]&G to see classes again )\n\r&W: " );
			return;
		    }
		}
	    }
	    ch_printf_color( ch, "&R* I'm afraid there is no description for that class.\n\r\n\r" );
	    show_classes_to_nanny( d );
	    return;
	}

	for ( iClass = 0; iClass < MAX_PC_CLASS; iClass++ )
	{
	    if ( class_table[iClass]->who_name
	    &&   class_table[iClass]->who_name[0] != '\0' )
	    {
		if ( toupper( arg[0]) == toupper( class_table[iClass]->who_name[0] )
		&&  !str_prefix( arg, class_table[iClass]->who_name ) )
		{
		    ch->class =  iClass;
		    break;
		}
	    }
	}

	if ( iClass == MAX_PC_CLASS
	||  !class_table[iClass]->who_name 
	||   class_table[iClass]->who_name[0] == '\0'
	||  !str_cmp( class_table[iClass]->who_name, "unused" ) )
	{
	    if ( arg[0] != '\0' )
		ch_printf_color( ch, "&R* I'm afraid there is no such class available.\n\r\n\r" );
	    show_classes_to_nanny( d );
	    return;
	}

	if ( check_bans( ch, BAN_CLASS ) )
	{
	    ch_printf_color( ch, "&R *I'm afraid there is no such class available.\n\r" );
	    show_classes_to_nanny( d );
	    return;
	}

	show_races_to_nanny( d );
	d->connected = CON_GET_NEW_RACE;
	break;

	case CON_GET_NEW_RACE:
	argument = one_argument( argument, arg );
    
	if ( !str_cmp( arg, "help" ) )
	{
	    for ( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
	    {
		if ( toupper( argument[0] ) == toupper( race_table[iRace]->race_name[0] )
		&&  !str_prefix( argument, race_table[iRace]->race_name ) )
		{
		    do_nanny_help( ch, argument );
		    ch_printf_color( ch, "&G\n\rPlease select ( press &W[Enter]&G to see races again )\n\r&W: " );
		    return;
		}
	    }
    	    ch_printf_color( ch, "&R* I'm afraid there is no description for that race.\n\r\n\r" );
	    show_races_to_nanny( d );
	    return;
	}

	for ( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
	{
	    if ( toupper( arg[0] ) == toupper(race_table[iRace]->race_name[0] )
	    &&  !str_prefix( arg, race_table[iRace]->race_name ) )
	    {
		ch->race = iRace;
		break;
	    }
	}

	if ( iRace == MAX_PC_RACE
	||  !race_table[iRace]->race_name
	||   race_table[iRace]->race_name[0] == '\0'
	||   iRace == RACE_VAMPIRE
	||   IS_SET( race_table[iRace]->class_restriction, 1 << ch->class )
	||  !str_cmp( race_table[iRace]->race_name, "unused" ) )
	{
	    if ( arg[0] != '\0' )
		ch_printf_color( ch, "&R* I'm afraid there is no such race available.\n\r\n\r" );
	    show_races_to_nanny( d );
	    return;
	}
	
	if ( check_bans( ch, BAN_RACE ) )
	{
	    ch_printf_color( ch, "&R *I'm afraid there is no such race available.\n\r" );
	    show_races_to_nanny( d );
	    return;
	}

	sprintf( log_buf, "%s@%s new %s %s.", ch->name, d->host,
		race_table[ch->race]->race_name,
		class_table[ch->class]->who_name );
	log_string_plus( log_buf, LOG_COMM, sysdata.log_level);
	to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
/*
	ch_printf_color( ch, "&GPress &W[&GEnter&W]&G " );
*/
	show_title(d);
	ch->level = 0;
	ch->position = POS_STANDING;
	d->connected = CON_PRESS_ENTER;
	set_pager_color( AT_PLAIN, ch );
	return;
	break;

    case CON_PRESS_ENTER:
        if ( chk_watch(get_trust(ch), ch->name, d->host) ) /*  --Gorog */
           SET_BIT( ch->pcdata->flags, PCFLAG_WATCH );
        else
           REMOVE_BIT( ch->pcdata->flags, PCFLAG_WATCH );

	set_pager_color( AT_PLAIN, ch );
	if ( xIS_SET(ch->act, PLR_RIP) )
	  send_rip_screen(ch);
	if ( xIS_SET(ch->act, PLR_ANSI) )
	  send_to_pager( "\033[2J", ch );
	else
	  send_to_pager( "\014", ch );
	/* Added DMOTD, replaces AMOTD/MOTD for deadlies - Blod, 8/2000 */
        if ( IS_IMMORTAL(ch) )
          do_help( ch, "imotd" );
        else if ( IS_PKILL( ch ) )
          do_help( ch, "dmotd" );
        else if ( ch->level == 50 )
          do_help( ch, "amotd" );
        else if ( ch->level == 0 )
          do_help( ch, "nmotd" );
        else
          do_help( ch, "motd" );
	send_to_pager( "\n\rPress [ENTER] ", ch );
        d->connected = CON_READ_MOTD;
        break;

    case CON_READ_MOTD:
	{
	  char motdbuf[MAX_STRING_LENGTH];

	  sprintf( motdbuf, "\n\rWelcome to %s...\n\r", sysdata.mud_name);
	  write_to_buffer( d, motdbuf, 0 );
	}
	add_char( ch );
	d->connected	= CON_PLAYING;

        /* ALL of the following can be removed after 90 days (IE all
	 * the players had to have logged in, any thing added between
	 * these comments should be timestamped. -Shaddai
	 */

        if ( !IS_NPC( ch ) && ch->pcdata->clan )
	  add_member( ch, ch->pcdata->clan_name );

  	/* April 16th, but probably won't port till May so update this */
        if ( xIS_SET( ch->deaf, CHANNEL_SHOUT ) )
		xREMOVE_BIT( ch->deaf, CHANNEL_SHOUT );
	if ( !IS_NPC( ch ) && IS_PKILL( ch )
	&&    IS_SET( ch->pcdata->flags, PCFLAG_NOSUMMON ) )
		REMOVE_BIT( ch->pcdata->flags, PCFLAG_NOSUMMON );
		

	/*
  	 * End timestamped stuff -Shaddai
 	 */		

	if ( ch->level == 0 )
	{
	    OBJ_DATA *obj;
	    int iLang;

	    ch->pcdata->clan_name = STRALLOC( "" );
	    ch->pcdata->clan	  = NULL;
	    switch ( class_table[ch->class]->attr_prime )
	    {
	    case APPLY_STR: ch->perm_str = 16; break;
	    case APPLY_INT: ch->perm_int = 16; break;
	    case APPLY_WIS: ch->perm_wis = 16; break;
	    case APPLY_DEX: ch->perm_dex = 16; break;
	    case APPLY_CON: ch->perm_con = 16; break;
	    case APPLY_CHA: ch->perm_cha = 16; break;
	    case APPLY_LCK: ch->perm_lck = 16; break;
	    }
            switch ( class_table[ch->class]->attr_second )
	    {
	    case APPLY_STR: ch->perm_str = 14; break;
	    case APPLY_INT: ch->perm_int = 14; break;
	    case APPLY_WIS: ch->perm_wis = 14; break;
	    case APPLY_DEX: ch->perm_dex = 14; break;
	    case APPLY_CON: ch->perm_con = 14; break;
	    case APPLY_CHA: ch->perm_cha = 14; break;
	    case APPLY_LCK: ch->perm_lck = 14; break;
	    }
            switch ( class_table[ch->class]->attr_deficient )
	    {
	    case APPLY_STR: ch->perm_str = 9; break;
	    case APPLY_INT: ch->perm_int = 9; break;
	    case APPLY_WIS: ch->perm_wis = 9; break;
	    case APPLY_DEX: ch->perm_dex = 9; break;
	    case APPLY_CON: ch->perm_con = 9; break;
	    case APPLY_CHA: ch->perm_cha = 9; break;
	    case APPLY_LCK: ch->perm_lck = 9; break;
	    }

	    ch->perm_str	 += race_table[ch->race]->str_plus;
	    ch->perm_int	 += race_table[ch->race]->int_plus;
	    ch->perm_wis	 += race_table[ch->race]->wis_plus;
	    ch->perm_dex	 += race_table[ch->race]->dex_plus;
	    ch->perm_con	 += race_table[ch->race]->con_plus;
	    ch->perm_cha	 += race_table[ch->race]->cha_plus;
	    ch->affected_by	  = race_table[ch->race]->affected;
	    ch->perm_lck	 += race_table[ch->race]->lck_plus;
	   
            ch->armor		 += race_table[ch->race]->ac_plus;
            ch->alignment	 += race_table[ch->race]->alignment;
            ch->attacks              = race_table[ch->race]->attacks;
            ch->defenses             = race_table[ch->race]->defenses;
	    ch->saving_poison_death  	= race_table[ch->race]->saving_poison_death;
	    ch->saving_wand  		= race_table[ch->race]->saving_wand;
	    ch->saving_para_petri  	= race_table[ch->race]->saving_para_petri;
	    ch->saving_breath  		= race_table[ch->race]->saving_breath;
	    ch->saving_spell_staff	= race_table[ch->race]->saving_spell_staff;

	    ch->height = number_range(race_table[ch->race]->height *.9, race_table[ch->race]->height *1.1);
	    ch->weight = number_range(race_table[ch->race]->weight *.9, race_table[ch->race]->weight *1.1);

	    if ( ch->class == CLASS_PALADIN )
	    	ch->alignment = 1000;

	    if ( (iLang = skill_lookup( "common" )) < 0 )
	    	bug( "Nanny: cannot find common language." );
	    else
	    	ch->pcdata->learned[iLang] = 100;
	    	
	    for ( iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++ )
	    	if ( lang_array[iLang] == race_table[ch->race]->language )
	    		break;
	    if ( lang_array[iLang] == LANG_UNKNOWN )
	    	bug( "Nanny: invalid racial language." );
	    else
	    {
	    	if ( (iLang = skill_lookup( lang_names[iLang] )) < 0 )
	    		bug( "Nanny: cannot find racial language." );
	    	else
	    		ch->pcdata->learned[iLang] = 100;
	    }

	    name_stamp_stats( ch );

	    ch->level	= 1;
	    ch->exp	= 0;
            ch->max_hit    += race_table[ch->race]->hit;
            ch->max_mana   += race_table[ch->race]->mana;
	    ch->hit	= UMAX(1,ch->max_hit);
	    ch->mana	= UMAX(1,ch->max_mana);
	    ch->move	= ch->max_move;
	    ch->gold	= 0;
	    sprintf( buf, "the %s",
		title_table [ch->class] [ch->level]
		[ch->sex == SEX_FEMALE ? 1 : 0] );
	    set_title( ch, buf );

            /* Added by Narn.  Start new characters with autoexit and autgold
               already turned on.  Very few people don't use those. */
	    xSET_BIT( ch->act, PLR_AUTOGOLD ); 
	    xSET_BIT( ch->act, PLR_AUTOEXIT ); 
	     SET_BIT( ch->pcdata->flags, PCFLAG_HINTS );

            /* Added by Brittany, Nov 24/96.  The object is the adventurer's guide
               to the realms of despair, part of Academy.are. */
            {
            OBJ_INDEX_DATA *obj_ind = get_obj_index( 10333 );
            if ( obj_ind != NULL )
            {
              obj = create_object( obj_ind, 0 );
              obj_to_char( obj, ch );
              equip_char( ch, obj, WEAR_HOLD );
            }
            }
	    if (!sysdata.WAIT_FOR_AUTH)
	      char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
	    else
	    {
	      char_to_room( ch, get_room_index( ROOM_AUTH_START ) );
	      ch->pcdata->auth_state = 0;
	      SET_BIT(ch->pcdata->flags, PCFLAG_UNAUTHED);
	    }
	    /* Display_prompt interprets blank as default */
	    ch->pcdata->prompt = STRALLOC("");
	}
	else
	if ( !IS_IMMORTAL(ch) && ch->pcdata->release_date > 0 && 
		ch->pcdata->release_date > current_time )
	{
	    if ( ch->in_room->vnum == 6
	    ||   ch->in_room->vnum == 8
	    ||   ch->in_room->vnum == 1206 )
		char_to_room( ch, ch->in_room );
	    else
	      char_to_room( ch, get_room_index(8) );
	}
	else
	if ( ch->in_room && ( IS_IMMORTAL( ch ) 
             || ( !xIS_SET( ch->in_room->room_flags, ROOM_PROTOTYPE ) 
			&& !xIS_SET( ch->in_room->room_flags, ROOM_NOLOG) ) ) )
	{
	    char_to_room( ch, ch->in_room );
	}
	else
	if ( IS_IMMORTAL(ch) )
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
	}
	else
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	}


    if ( get_timer( ch, TIMER_SHOVEDRAG ) > 0 )
        remove_timer( ch, TIMER_SHOVEDRAG );

    if ( get_timer( ch, TIMER_PKILLED ) > 0 )
	remove_timer( ch, TIMER_PKILLED );

   /* Login trigger by Edmond */
    rprog_login_trigger( ch );
    mprog_login_trigger( ch );

    if ( class_table[ch->class]->login_other )
       act( AT_ACTION, class_table[ch->class]->login_other, ch, NULL, NULL, TO_CANSEE );
    else
       act( AT_ACTION, "$n has entered the game.",  ch, NULL, NULL, TO_CANSEE );

    if ( ch->pcdata->pet )
    {
           act( AT_ACTION, "$n returns to $s master from the Void.",
                      ch->pcdata->pet, NULL, ch, TO_NOTVICT );
           act( AT_ACTION, "$N returns with you to the realms.",
                        ch, NULL, ch->pcdata->pet, TO_CHAR );
    }         
    do_look( ch, "auto" );
    if ( !IS_PKILL( ch ) )
    	xSET_BIT(ch->act, PLR_NICE);
    SET_BIT(ch->pcdata->flags, PCFLAG_NORECALL );
    xSET_BIT( ch->act, PLR_COMBINE );

    { /* FIX up bugged players */
    	if ( ch->played < 0 ) 
		ch->played = 1000;
	if ( ch->max_hit < 0 )
		ch->max_hit = 10;
	if ( ch->hit < 0 )
		ch->hit = 1;
	if ( ch->max_mana < 0 )
		ch->max_mana = 10;
	if ( ch->mana < 0 )
		ch->mana = 1;
	if ( ch->max_move < 0 )
		ch->max_move = 10;
	if ( ch->move < 0 )
		ch->move = 1;
	if ( ch->position < POS_SLEEPING || ch->position == POS_MOUNTED )
		ch->position = POS_STANDING;
	if ( ch->exp < 0 )
		ch->exp = 10;
    }
    tax_player(ch);  /* Here we go, let's tax players to lower the gold
			pool -- TRI */
    mail_count(ch);
    check_loginmsg(ch);

    if ( !ch->was_in_room && ch->in_room == get_room_index( ROOM_VNUM_TEMPLE ))
      	ch->was_in_room = get_room_index( ROOM_VNUM_TEMPLE );
    else if ( ch->was_in_room == get_room_index( ROOM_VNUM_TEMPLE ))
        ch->was_in_room = get_room_index( ROOM_VNUM_TEMPLE );
    else if ( !ch->was_in_room )
    	ch->was_in_room = ch->in_room;
/* Crashy crashy
    if ( ch->first_carrying )
       do_autofinddupe( ch->last_carrying ); Keep an eye out for performance problems -Shaddai
*/
    break;

    }

    return;
}

bool is_reserved_name( char *name )
{
  RESERVE_DATA *res;
  
  for (res = first_reserved; res; res = res->next)
    if ((*res->name == '*' && !str_infix(res->name+1, name)) ||
        !str_cmp(res->name, name))
      return TRUE;
  return FALSE;
}


/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name, bool newchar )
{
 /*
  * Names checking should really only be done on new characters, otherwise
  * we could end up with people who can't access their characters.  Would
  * have also provided for that new area havoc mentioned below, while still
  * disallowing current area mobnames.  I personally think that if we can
  * have more than one mob with the same keyword, then may as well have
  * players too though, so I don't mind that removal.  -- Alty
  */
  
     if ( is_reserved_name(name) && newchar )
	return FALSE;

     /*
      * Outdated stuff -- Alty
      */
/*     if ( is_name( name, "all auto immortal self someone god supreme demigod dog guard cityguard cat cornholio spock hicaine hithoric death ass fuck shit piss crap quit" ) )
       return FALSE;*/
 
    /*
     * Length restrictions.
     */
    if ( strlen(name) <  3 )
	return FALSE;

    if ( strlen(name) > 12 )
	return FALSE;

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha(*pc) )
		return FALSE;
	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;
    }

    /*
     * Code that followed here used to prevent players from naming
     * themselves after mobs... this caused much havoc when new areas
     * would go in...
     */

    return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    CHAR_DATA *ch;

    for ( ch = first_char; ch; ch = ch->next )
    {
	if ( !IS_NPC(ch)
	&& ( !fConn || !ch->desc )
	&&    ch->pcdata->filename
	&&   !str_cmp( name, ch->pcdata->filename ) )
	{
	    if ( fConn && ch->switched )
	    {
	      write_to_buffer( d, "Already playing.\n\rName: ", 0 );
	      d->connected = CON_GET_NAME;
	      if ( d->character )
	      {
		 /* clear descriptor pointer to get rid of bug message in log */
		 d->character->desc = NULL;
		 free_char( d->character );
		 d->character = NULL;
	      }
	      return BERR;
	    }
	    if ( fConn == FALSE )
	    {
		DISPOSE( d->character->pcdata->pwd );
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
	    }
	    else
	    {
		/* clear descriptor pointer to get rid of bug message in log */
		d->character->desc = NULL;
		free_char( d->character );
		d->character = ch;
		ch->desc	 = d;
		ch->timer	 = 0;
        if ( d->host )
            ch->pcdata->recent_site = STRALLOC( d->host );
                if ( class_table[ch->class]->reconnect )
                        ch_printf(ch, "%s\n\r", class_table[ch->class]->reconnect );
                else
                        send_to_char( "Reconnecting.\n\r", ch );
                rprog_login_trigger(ch);
                mprog_login_trigger(ch);
		do_look( ch, "auto" );
		check_loginmsg( ch );
                if ( class_table[ch->class]->reconnect_other )
                   act( AT_ACTION, class_table[ch->class]->reconnect_other, ch, NULL, NULL, TO_CANSEE );
                else
                   act( AT_ACTION, "$n has reconnected.", ch, NULL, NULL, TO_CANSEE );
                sprintf( log_buf, "%s@%s(%s) reconnected. (INRoom %d)",
                  ch->pcdata->filename,
                  d->host,
                  d->user,
                  ( ch->in_room ? ch->in_room->vnum : -1 ) );
		log_string_plus( log_buf, LOG_COMM, UMAX( sysdata.log_level, ch->level ) );
/*
		if ( ch->level < LEVEL_SAVIOR )
		  to_channel( log_buf, CHANNEL_MONITOR, "Monitor", ch->level );
*/
		d->connected = CON_PLAYING;
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name, bool kick )
{
    CHAR_DATA *ch;

    DESCRIPTOR_DATA *dold;
    int	cstate;

    for ( dold = first_descriptor; dold; dold = dold->next )
    {
	if ( dold != d
	&& (  dold->character || dold->original )
	&&   !str_cmp( name, dold->original
		 ? dold->original->pcdata->filename : 
		 dold->character->pcdata->filename ) )
	{
	    cstate = dold->connected;
	    ch = dold->original ? dold->original : dold->character;
	    if ( !ch->name
	    || ( cstate != CON_PLAYING && cstate != CON_EDITING ))
	    {
		write_to_buffer( d, "That character is already connected - try again.\n\r", 0 );
		sprintf( log_buf, "%s already connected.", 
				ch->pcdata->filename );
		log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
		return BERR;
	    }
	    if ( !kick )
	      return TRUE;
	    write_to_buffer( d, "Already playing... Kicking off old connection.\n\r", 0 );
	    write_to_buffer( dold, "Kicking off old connection... bye!\n\r", 0 );
	    close_socket( dold, FALSE );
	    /* clear descriptor pointer to get rid of bug message in log */
	    d->character->desc = NULL;
	    free_char( d->character );
	    d->character = ch;
	    ch->desc	 = d;
	    ch->timer	 = 0;
        if ( d->host )
           ch->pcdata->recent_site  = STRALLOC( d->host );
	    if ( ch->switched )
	      do_return( ch->switched, "" );
	    ch->switched = NULL;
	    send_to_char( "Reconnecting.\n\r", ch );
	    do_look( ch, "auto" );
	    check_loginmsg( ch );
	    act( AT_ACTION, "$n has reconnected, kicking off old link.",
	         ch, NULL, NULL, TO_CANSEE );
            sprintf( log_buf, "%s@%s reconnected, kicking off old link. (INRoom %d)",
                 ch->pcdata->filename,
                 d->host,
                 ( ch->in_room ? ch->in_room->vnum : -1 ) );
	    log_string_plus( log_buf, LOG_COMM, UMAX( sysdata.log_level, ch->level ) );
/*
	    if ( ch->level < LEVEL_SAVIOR )
	      to_channel( log_buf, CHANNEL_MONITOR, "Monitor", ch->level );
*/
	    d->connected = cstate;
	    return TRUE;
	}
    }

    return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;

    /*
    if ( !ch
    ||   !ch->desc
    ||    ch->desc->connected != CON_PLAYING
    ||   !ch->was_in_room
    ||    ch->in_room != get_room_index( ROOM_VNUM_LIMBO ) )
	return;
	*/

    if ( !ch
    ||   !ch->desc 
    ||   ch->desc->connected != CON_PLAYING
    ||   !IS_IDLE(ch) )
    	return;

    /*
    if ( IS_IMMORTAL(ch) )
    	return;
    */

    ch->timer = 0;
    was_in_room = ch->was_in_room;
    char_from_room( ch );
    char_to_room( ch, was_in_room );
    ch->was_in_room = ch->in_room;
    /*
    ch->was_in_room	= NULL;
    */
    REMOVE_BIT(ch->pcdata->flags, PCFLAG_IDLE );
    /* Void triggers by Edmond */
    rprog_void_trigger( ch );
    mprog_void_trigger( ch );
    act( AT_ACTION, "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    return;
}



/*
 * Write to one char.
 */
void send_to_char( const char *txt, CHAR_DATA *ch )
{
    if ( !ch )
    {
      bug( "Send_to_char: NULL *ch" );
      return;
    }
    if ( txt && ch->desc )
	write_to_buffer( ch->desc, txt, strlen(txt) );
    return;
}

/*
 * Same as above, but converts &color codes to ANSI sequences..
 */
void send_to_char_color( const char *txt, CHAR_DATA *ch )
{
  DESCRIPTOR_DATA *d;
  char *colstr;
  const char *prevstr = txt;
  char colbuf[20];
  int ln;
  
  if ( !ch )
  {
    bug( "Send_to_char_color: NULL *ch" );
    return;
  }
  if ( !txt || !ch->desc )
    return;
  d = ch->desc;
  while ( (colstr = strpbrk(prevstr, "&^")) != NULL )
  {
    if (colstr > prevstr)
      write_to_buffer(d, prevstr, (colstr-prevstr));
    ln = make_color_sequence(colstr, colbuf, d);
    if ( ln < 0 )
    {
      prevstr = colstr+1;
      break;
    }
    else if ( ln > 0 )
      write_to_buffer(d, colbuf, ln);
    prevstr = colstr+2;
  }
  if ( *prevstr )
    write_to_buffer(d, prevstr, 0);
  return;
}

void write_to_pager( DESCRIPTOR_DATA *d, const char *txt, int length )
{
  int pageroffset;	/* Pager fix by thoric */

  if ( length <= 0 )
    length = strlen(txt);
  if ( length == 0 )
    return;
  if ( !d->pagebuf )
  {
    d->pagesize = MAX_STRING_LENGTH;
    CREATE( d->pagebuf, char, d->pagesize );
  }
  if ( !d->pagepoint )
  {
    d->pagepoint = d->pagebuf;
    d->pagetop = 0;
    d->pagecmd = '\0';
  }
  if ( d->pagetop == 0 && !d->fcommand )
  {
    d->pagebuf[0] = '\n';
    d->pagebuf[1] = '\r';
    d->pagetop = 2;
  }
  pageroffset = d->pagepoint - d->pagebuf;	/* pager fix (goofup fixed 08/21/97) */
  while ( d->pagetop + length >= d->pagesize )
  {
    if ( d->pagesize >= 24 * MAX_STRING_LENGTH ) /* pagefile size */
    {
      d->pagetop = 0;
      d->pagepoint = NULL;
      DISPOSE(d->pagebuf);
      d->pagesize = MAX_STRING_LENGTH;
      // Move bug call here to avoid infinite loops.  Compliments of Daltorak -- Alty
      bug( "Pager overflow (%s).  Ignoring.\n\r", d->character ? d->character->name : "???" );
      return;
    }
    if ( d->pagesize < 8 * MAX_STRING_LENGTH )
       d->pagesize *= 2;
    else
       d->pagesize = 24 * MAX_STRING_LENGTH;
    RECREATE(d->pagebuf, char, d->pagesize);
  }
  d->pagepoint = d->pagebuf + pageroffset;	/* pager fix (goofup fixed 08/21/97) */
  strncpy(d->pagebuf+d->pagetop, txt, length);
  d->pagetop += length;
  d->pagebuf[d->pagetop] = '\0';
  return;
}

void send_to_pager( const char *txt, CHAR_DATA *ch )
{
  if ( !ch )
  {
    bug( "Send_to_pager: NULL *ch" );
    return;
  }
  if ( txt && ch->desc )
  {
    DESCRIPTOR_DATA *d = ch->desc;
    
    ch = d->original ? d->original : d->character;
    if ( IS_NPC(ch) || !IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) )
    {
	send_to_char(txt, d->character);
	return;
    }
    write_to_pager(d, txt, 0);
  }
  return;
}


void send_to_pager_color( const char *txt, CHAR_DATA *ch )
{
  DESCRIPTOR_DATA *d;
  char *colstr;
  const char *prevstr = txt;
  char colbuf[20];
  int ln;
  
  if ( !ch )
  {
    bug( "Send_to_pager_color: NULL *ch" );
    return;
  }
  if ( !txt || !ch->desc )
    return;
  d = ch->desc;
  ch = d->original ? d->original : d->character;
  if ( IS_NPC(ch) || !IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) )
  {
    send_to_char_color(txt, d->character);
    return;
  }
  while ( (colstr = strpbrk(prevstr, "&^")) != NULL )
  {
    if ( colstr > prevstr )
      write_to_pager(d, prevstr, (colstr-prevstr));
    ln = make_color_sequence(colstr, colbuf, d);
    if ( ln < 0 )
    {
      prevstr = colstr+1;
      break;
    }
    else if ( ln > 0 )
      write_to_pager(d, colbuf, ln);
    prevstr = colstr+2;
  }
  if ( *prevstr )
    write_to_pager(d, prevstr, 0);
  return;
}

sh_int figure_color( sh_int AType, CHAR_DATA *ch )
{
  int at = AType;
  if (at >= AT_COLORBASE && at < AT_TOPCOLOR)
  {
    at -= AT_COLORBASE;
    if (IS_NPC(ch) || ch->pcdata->colorize[at] == -1)
      at = at_color_table[at].def_color;
    else
      at = ch->pcdata->colorize[at];
  }
  if (at < 0 || at > AT_WHITE+AT_BLINK)
  {
    bug("Figure_color: color %d invalid.", at);
    at = AT_GREY;
  }
  return at;
}



/* Modified to fix color bleeding bugs - Luc 09/2000 */
void set_char_color( sh_int AType, CHAR_DATA *ch ) {
   char buf[16];
   CHAR_DATA *och;
   int nc;

   if ( !ch || !ch->desc )
      return;
   och = ( ch->desc->original ? ch->desc->original : ch );
   if ( !IS_NPC( och ) && xIS_SET( och->act, PLR_ANSI ) ) {
      AType = figure_color( AType, och );

      /* Figure_color does return a 5 bits code, where bit 3 is the intensity
         and bit 4 is the blink attribute. d->prevcolor is a whole 8 bits,
         with blink in bit 7, so we need to move it into place :P  Also, we
         will reset the background color to black.  Not sure it's a good thing
         though.  Luc 09/2000 */

      ch->desc->prevcolor = ( AType & 0x0F ) | ( AType & AT_BLINK ? 0x80 : 0 );
      /* The whole sequence should not be longer than 12 chars.  A simple reset
         does not suffice because of different ways it's interpreted by different
         clients. - Luc 09/2000 */
      nc = sprintf(buf, "\033[0;%s%s3%dm",
         AType & 8 ? "1;" : "",
         AType & AT_BLINK ? "5;" : "",
         AType & 7 );
      write_to_buffer( ch->desc, buf, nc );
      }
   }



/* Modified to fix color bleeding bugs - Luc 09/2000 */
void set_pager_color( sh_int AType, CHAR_DATA *ch ) {
   char buf[16];
   CHAR_DATA *och;

   if ( !ch || !ch->desc )
      return;
   och = ( ch->desc->original ? ch->desc->original : ch );
   if ( !IS_NPC( och ) && xIS_SET( och->act, PLR_ANSI ) ) {
      AType = figure_color( AType, och );
      ch->desc->prevcolor = ( AType & 0x0F ) | ( AType & AT_BLINK ? 0x80 : 0 );

      /* The whole sequence should not be longer than 12 chars. */
      sprintf(buf, "\033[0;%s%s3%dm",
         AType & 8 ? "1;" : "",
         AType & AT_BLINK ? "5;" : "",
         AType & 7 );
      send_to_pager( buf, ch );
      }
   }



/* source: EOD, by John Booth (???) */
void ch_printf(CHAR_DATA *ch, char *fmt, ...)
{
    char buf[MAX_STRING_LENGTH*2];	/* better safe than sorry */
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
	
    send_to_char(buf, ch);
}

void pager_printf(CHAR_DATA *ch, char *fmt, ...)
{
    char buf[MAX_STRING_LENGTH*2];
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
	
    send_to_pager(buf, ch);
}


/*
 * Function to strip off the "a" or "an" or "the" or "some" from an object's
 * short description for the purpose of using it in a sentence sent to
 * the owner of the object.  (Ie: an object with the short description
 * "a long dark blade" would return "long dark blade" for use in a sentence
 * like "Your long dark blade".  The object name isn't always appropriate
 * since it contains keywords that may not look proper.		-Thoric
 */
char *myobj( OBJ_DATA *obj )
{
    if ( !str_prefix("a ", obj->short_descr) )
	return obj->short_descr + 2;
    if ( !str_prefix("an ", obj->short_descr) )
	return obj->short_descr + 3;
    if ( !str_prefix("the ", obj->short_descr) )
	return obj->short_descr + 4;
    if ( !str_prefix("some ", obj->short_descr) )
	return obj->short_descr + 5;
    return obj->short_descr;
}


char *obj_short( OBJ_DATA *obj )
{
    static char buf[MAX_STRING_LENGTH];

    if ( obj->count > 1 )
    {
	sprintf( buf, "%s (%d)", obj->short_descr, obj->count );
	return buf;
    }
    return obj->short_descr;
}

/*
 * The primary output interface for formatted output.
 */
/* Major overhaul. -- Alty */

void ch_printf_color(CHAR_DATA *ch, char *fmt, ...)
{
     char buf[MAX_STRING_LENGTH*2];
     va_list args;
 
     va_start(args, fmt);
     vsprintf(buf, fmt, args);
     va_end(args);
 
     send_to_char_color(buf, ch);
}
 
void pager_printf_color(CHAR_DATA *ch, char *fmt, ...)
{
     char buf[MAX_STRING_LENGTH*2];
     va_list args;
 
     va_start(args, fmt);
     vsprintf(buf, fmt, args);
     va_end(args);
 
     send_to_pager_color(buf, ch);
}

#define MORPHNAME(ch)   ((ch->morph&&ch->morph->morph)? \
                         ch->morph->morph->short_desc: \
                         IS_NPC(ch) ? ch->short_descr : ch->name)
#define NAME(ch)        (IS_NPC(ch) ? ch->short_descr : ch->name)

char *act_string(const char *format, CHAR_DATA *to, CHAR_DATA *ch,
		 const void *arg1, const void *arg2, int flags)
{
  static char * const he_she  [] = { "it",  "he",  "she" };
  static char * const him_her [] = { "it",  "him", "her" };
  static char * const his_her [] = { "its", "his", "her" };
  static char buf[MAX_STRING_LENGTH];
  char fname[MAX_INPUT_LENGTH];
  char temp[MAX_STRING_LENGTH];
  char *point = buf;
  const char *str = format;
  const char *i;
  CHAR_DATA *vch = (CHAR_DATA *) arg2;
  OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
  OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;

  if ( str[0] == '$' )
  	DONT_UPPER = FALSE;

  while ( *str != '\0' )
  {
    if ( *str != '$' )
    {
      *point++ = *str++;
      continue;
    }
    ++str;
    if ( !arg2 && *str >= 'A' && *str <= 'Z' )
    {
      bug( "Act: missing arg2 for code %c:", *str );
      bug( format );
      i = " <@@@> ";
    }
    else
    {
      switch ( *str )
      {
      default:  bug( "Act: bad code %c.", *str );
		i = " <@@@> ";						break;
      case 't':
        /* bug fix - Edmond  i = (char *) arg1;         break; */
        if ( arg1 )
                i = (char *) arg1;
        else
        {
                bug( "Act: Bad variable $t" );
                i = " <@@@> ";
        }
        break;
      case 'T':
        /* same bug fix as above -  i = (char *) arg2;          break; */
        if ( arg2 )
                i = (char *) arg2;
        else
        {
                bug( "Act: Bad variable $T" );
                i = " <@@@> ";
        }
        break;
      case 'n':
              if ( ch->morph == NULL )
                  i = (to ? PERS ( ch, to ): NAME ( ch ) );
              else if ( !IS_SET( flags, STRING_IMM ) )
                  i = (to ? MORPHPERS (ch, to) : MORPHNAME (ch));
              else
              {
                sprintf(temp, "(MORPH) %s", (to ? PERS(ch,to):NAME(ch))); 
                i = temp;
              }
              break;
      case 'N':
              if ( vch->morph == NULL )
                   i = (to ? PERS ( vch, to ) : NAME( vch ) );
              else if ( !IS_SET( flags, STRING_IMM ) )
                   i = (to ? MORPHPERS (vch, to) : MORPHNAME (vch));
              else
              {
                sprintf(temp, "(MORPH) %s", (to ? PERS(vch,to):NAME(vch)));
                i = temp;
              }
              break;

      case 'e': if (ch->sex > 2 || ch->sex < 0)
		{
		  bug("act_string: player %s has sex set at %d!", ch->name,
		      ch->sex);
		  i = "it";
		}
		else
		  i = he_she [URANGE(0,  ch->sex, 2)];
		break;
      case 'E': if (vch->sex > 2 || vch->sex < 0)
		{
		  bug("act_string: player %s has sex set at %d!", vch->name,
		      vch->sex);
		  i = "it";
		}
		else
		  i = he_she [URANGE(0, vch->sex, 2)];
		break;
      case 'm': if (ch->sex > 2 || ch->sex < 0)
		{
		  bug("act_string: player %s has sex set at %d!", ch->name,
		      ch->sex);
		  i = "it";
		}
		else
		  i = him_her[URANGE(0,  ch->sex, 2)];
		break;
      case 'M': if (vch->sex > 2 || vch->sex < 0)
		{
		  bug("act_string: player %s has sex set at %d!", vch->name,
		      vch->sex);
		  i = "it";
		}
		else
		  i = him_her[URANGE(0, vch->sex, 2)];
		break;
      case 's': if (ch->sex > 2 || ch->sex < 0)
		{
		  bug("act_string: player %s has sex set at %d!", ch->name,
		      ch->sex);
		  i = "its";
		}
		else
		  i = his_her[URANGE(0,  ch->sex, 2)];
		break;
      case 'S' : if (vch->sex > 2 || vch->sex < 0)
                {
                  bug("act_string: player %s has sex set at %d!", vch->name,
                      vch->sex);
                  i = "its";
                }
                else
                  i = his_her[URANGE(0,  vch->sex, 2)];
                break;
      case 'q': i = (to == ch) ? "" : "s";				break;
      case 'Q': i = (to == ch) ? "your" :
		    his_her[URANGE(0,  ch->sex, 2)];			break;
      case 'p': 
	    if ( !to || can_see_obj(to, obj1) )
		{
			/* Prevents act programs from triggering off note shorts */
			if ( ( !to || IS_NPC( to ) ) 
			&& (obj1->item_type == ITEM_PAPER ) )
				i = obj1->pIndexData->short_descr;
			else
				i = obj_short(obj1);
		}
		else i = "something";
		break;
      case 'P': 
	    if ( !to || can_see_obj(to, obj2) )
		{
			/* Prevents act programs from triggering off note shorts */
			if ( ( !to || IS_NPC( to ) ) 
			&& (obj2->item_type == ITEM_PAPER ) )
				i = obj2->pIndexData->short_descr;
			else
				i = obj_short(obj2);
		}
		else i = "something";
		break;
/*      case 'p': i = (!to || can_see_obj(to, obj1)
          ? obj_short(obj1) : "something");         break;
      case 'P': i = (!to || can_see_obj(to, obj2)
          ? obj_short(obj2) : "something");         break;
*/
      case 'd':
        if ( !arg2 || ((char *) arg2)[0] == '\0' )
          i = "door";
        else
        {
          one_argument((char *) arg2, fname);
          i = fname;
        }
        break;
      }
    }
    ++str;
    while ( (*point = *i) != '\0' )
      ++point, ++i;
  
   /*  #0  0x80c6c62 in act_string (
    format=0x81db42e "$n has reconnected, kicking off old link.", to=0x0, 
    ch=0x94fcc20, arg1=0x0, arg2=0x0, flags=2) at comm.c:2901 */
  }
  strcpy(point, "\n\r");
  if ( !DONT_UPPER )
     buf[0] = UPPER(buf[0]);
  return buf;
}
#undef NAME
  
void act( sh_int AType, const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type )
{
    char *txt;
    const char *str;
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *)arg2;
#define ACTF_NONE 0
#define ACTF_TXT  BV00
#define ACTF_CH   BV01
#define ACTF_OBJ  BV02
    
    OBJ_DATA *obj1 = (OBJ_DATA *)arg1;
    OBJ_DATA *obj2 = (OBJ_DATA *)arg2;
    int flags1 = ACTF_NONE, flags2 = ACTF_NONE;

    /*
     * Discard null and zero-length messages.
     */
    if ( !format || format[0] == '\0' )
	return;

    if ( !ch )
    {
	bug( "Act: null ch. (%s)", format );
	return;
    }

    // Do some proper type checking here..  Sort of.  We base it on the $* params.
    // This is kinda lame really, but I suppose in some weird sense it beats having
    // to pass like 8 different NULL parameters every time we need to call act()..
    for (str = format; *str; ++str)
    {
      if (*str == '$')
      {
        if (!*++str)
          break;
        switch(*str)
        {
        default:
          bug( "Act: bad code %c for format %s.", *str, format );
          break;
        
        case 't':
          flags1 |= ACTF_TXT;
          obj1 = NULL;
          break;
        
        case 'T':
        case 'd':
          flags2 |= ACTF_TXT;
          vch = NULL;
          obj2 = NULL;
          break;
        
        case 'n': case 'e': case 'm': case 's': case 'q':
          break;
        
        case 'N': case 'E': case 'M': case 'S': case 'Q':
          flags2 |= ACTF_CH;
          obj2 = NULL;
          break;
        
        case 'p':
          flags1 |= ACTF_OBJ;
          break;
        
        case 'P':
          flags2 |= ACTF_OBJ;
          vch = NULL;
          break;
        }
      }
    }
    
    
    if (flags1 != ACTF_NONE && flags1 != ACTF_TXT && flags1 != ACTF_CH && flags1 != ACTF_OBJ)
    {
      bug("Act: arg1 has more than one type in format %s.  Setting all NULL.", format);
      obj1 = NULL;
    }

    if (flags2 != ACTF_NONE && flags2 != ACTF_TXT && flags2 != ACTF_CH && flags2 != ACTF_OBJ)
    {
      bug("Act: arg2 has more than one type in format %s.  Setting all NULL.", format);
      vch = NULL;
      obj2 = NULL;
    }

    if ( !ch->in_room )
      to = NULL;
    else if ( type == TO_CHAR )
      to = ch;
    else
      to = ch->in_room->first_person;

    /*
     * ACT_SECRETIVE handling
     */
    if ( IS_NPC(ch) && xIS_SET(ch->act, ACT_SECRETIVE) && type != TO_CHAR )
	return;

    if ( type == TO_VICT )
    {
	if ( !vch )
	{
	    bug( "Act: null vch with TO_VICT." );
	    bug( "%s (%s)", ch->name, format );
	    return;
	}
	if ( !vch->in_room )
	{
	    bug( "Act: vch in NULL room!" );
	    bug( "%s -> %s (%s)", ch->name, vch->name, format );
	    return;
	}
	to = vch;
/*	to = vch->in_room->first_person;*/
    }

    if ( MOBtrigger && type != TO_CHAR && type != TO_VICT && to )
    {
      OBJ_DATA *to_obj;
      
      txt = act_string(format, NULL, ch, arg1, arg2, STRING_IMM);
      if ( HAS_PROG(to->in_room, ACT_PROG) )
        rprog_act_trigger(txt, to->in_room, ch, obj1, vch, obj2);
      for ( to_obj = to->in_room->first_content; to_obj;
            to_obj = to_obj->next_content )
        if ( HAS_PROG(to_obj->pIndexData, ACT_PROG) )
          oprog_act_trigger(txt, to_obj, ch, obj1, vch, obj2);
    }

    /* Anyone feel like telling me the point of looping through the whole
       room when we're only sending to one char anyways..? -- Alty */
    for ( ; to; to = (type == TO_CHAR || type == TO_VICT)
                     ? NULL : to->next_in_room )
    {
	if ((!to->desc 
	&& (  IS_NPC(to) && !HAS_PROG(to->pIndexData, ACT_PROG) ))
	||   !IS_AWAKE(to) )
	    continue;

	if ( type == TO_CHAR && to != ch )
	    continue;
	if ( type == TO_VICT && ( to != vch || to == ch ) )
	    continue;
	if ( type == TO_ROOM && to == ch )
	    continue;
	if ( type == TO_NOTVICT && (to == ch || to == vch) )
	    continue;
	if ( type == TO_CANSEE && ( to == ch || 
	    (!IS_NPC(ch) && (xIS_SET(ch->act, PLR_WIZINVIS) 
	    && (get_trust(to) < (ch->pcdata ? ch->pcdata->wizinvis : 0) ) ) ) ) )
	    continue;

        if ( IS_IMMORTAL(to) )
            txt = act_string (format, to, ch, arg1, arg2, STRING_IMM);
        else
       	    txt = act_string (format, to, ch, arg1, arg2, STRING_NONE);

	if (to->desc)
	{
	  if ( AType == AT_COLORIZE )
	  {
	     if ( txt[0] == '&' )
	     	send_to_char_color( txt, to );
	     else
	     {
	        set_char_color(AT_MAGIC, to );
		write_to_buffer( to->desc, txt, strlen(txt) );
	     }
	  }
	  else {
	     set_char_color(AType, to);
	     write_to_buffer( to->desc, txt, strlen(txt) );
	  }
	}
	if (MOBtrigger)
        {
          /* Note: use original string, not string with ANSI. -- Alty */
	  mprog_act_trigger( txt, to, ch, obj1, vch, obj2 );
        }
    }
    MOBtrigger = TRUE;
    return;
}

void do_name( CHAR_DATA *ch, char *argument )
{
  char fname[1024];
  struct stat fst;
  CHAR_DATA *tmp;

  if ( !NOT_AUTHED(ch) || ch->pcdata->auth_state != 2)
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  argument[0] = UPPER(argument[0]);

  if (!check_parse_name(argument, TRUE))
  {
    send_to_char("That name is reserved, please try another.\n\r", ch);
    return;
  }

  if ( check_playing( ch->desc, argument, FALSE ) == BERR )
         return;

  if (!str_cmp(ch->name, argument))
  {
    send_to_char("That's already your name!\n\r", ch);
    return;
  }

  for ( tmp = first_char; tmp; tmp = tmp->next )
  {
    if (!str_cmp(argument, tmp->name))
    break;
  }

  if ( tmp )
  {
    send_to_char("That name is already taken.  Please choose another.\n\r", ch);
    return;
  }

  sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower(argument[0]),
                        capitalize( argument ) );
  if ( stat( fname, &fst ) != -1 )
  {
    send_to_char("That name is already taken.  Please choose another.\n\r", ch);
    return;
  }

  STRFREE( ch->name );
  ch->name = STRALLOC( argument );
  STRFREE( ch->pcdata->filename );
  ch->pcdata->filename = STRALLOC( argument ); 
  send_to_char("Your name has been changed.  Please apply again.\n\r", ch);
  ch->pcdata->auth_state = 0;
  return;
}
  
char *default_fprompt( CHAR_DATA *ch )
{
  static char buf[60];

  strcpy(buf, "&w<&Y%hhp ");
  if ( IS_VAMPIRE(ch) )
    strcat(buf, "&R%bbp");
  else
    strcat(buf, "&C%mm");
  strcat(buf, " &G%vmv&w> ");
  if ( IS_NPC(ch) || IS_IMMORTAL(ch) )
    strcat(buf, "%i%R");
  return buf;
}

char *default_prompt( CHAR_DATA *ch )
{
  static char buf[60];

  strcpy(buf, "&w<&Y%hhp ");
  if ( IS_VAMPIRE(ch) )
    strcat(buf, "&R%bbp");
  else
    strcat(buf, "&C%mm");
  strcat(buf, " &G%vmv&w> ");
  if ( IS_NPC(ch) || IS_IMMORTAL(ch) )
    strcat(buf, "%i%R");
  return buf;
}

int getcolor(char clr)
{
  static const char colors[16] = "xrgObpcwzRGYBPCW";
  int r;
  
  for ( r = 0; r < 16; r++ )
    if ( clr == colors[r] )
      return r;
  return -1;
}

/* This is meant to be used for "%*.*s" type formatters in the *printf family.
   Pass in the color-coded string and a goal length, and it will return the
   number of color-coded characters from the string are required to make the
   goal length visible when drawn to someone's screen.
   If the number of non-color characters in the string is less than the goal,
   then the remainder is added to the total.
   This should mean that code like:
   
   int len = colorlen(somestring, 50);
   ch_printf_color(ch, "%*.*s", len, len, somestring);   
   will _always_ display a 50-character field, regardless of what's contained
   in the string somestring.
 */
int colorlen(const char *s, int goal)
{
  const char *p = s;
  
  while (*s && goal > 0)
  {
    switch(*s)
    {
    case '&':
      if (s[1] == '-')
      {
        --goal;
        s += 2;
        break;
      }
    case '^':
      ++s;
      if (*s == s[-1])
      {
        --goal;
        ++s;
      }
      else if (getcolor(*s) >= 0)
      {
        ++s;
      }
      break;
    default:
      --goal;
      ++s;
      break;
    }
  }
  return (s-p)+goal;
}

/* This is sort of counterpoint to the above function.  It takes a color-coded
   string and simply returns the display length.  Basically a strlen() but
   ignores color codes.
 */
int c_strlen(const char *s)
{
  int len = 0;
  
  while (*s)
  {
    switch(*s)
    {
    case '&':
      if (s[1] == '-')
      {
        ++len;
        s += 2;
        break;
      }
    case '^':
      ++s;
      if (*s == s[-1])
      {
        ++len;
        ++s;
      }
      else if (getcolor(*s) >= 0)
      {
        ++s;
      }
      break;
    default:
      ++len;
      ++s;
      break;
    }
  }
  return len;
}

void display_prompt( DESCRIPTOR_DATA *d )
{
  CHAR_DATA *ch = d->character;
  CHAR_DATA *och = (d->original ? d->original : d->character);
  CHAR_DATA *victim;
  bool ansi = (!IS_NPC(och) && xIS_SET(och->act, PLR_ANSI));
  const char *prompt;
  const char *helpstart = "<Type HELP START>";
  char buf[MAX_STRING_LENGTH];
  char *pbuf = buf;
  int stat, percent;

  if ( !ch )
  {
    bug( "display_prompt: NULL ch" );
    return;
  }

  if ( !IS_NPC(ch) && !IS_SET(ch->pcdata->flags, PCFLAG_HELPSTART ) )
	prompt = helpstart;
  else if ( !IS_NPC(ch) && ch->substate != SUB_NONE && ch->pcdata->subprompt
  &&   ch->pcdata->subprompt[0] != '\0' )
    prompt = ch->pcdata->subprompt;
  else if (IS_NPC (ch) || (!ch->fighting && (!ch->pcdata->prompt
                || !*ch->pcdata->prompt) ) )
    prompt = default_prompt (ch);
  else if ( ch->fighting )
  {
        if ( !ch->pcdata->fprompt || !*ch->pcdata->fprompt )
                prompt = default_fprompt ( ch );
        else
                prompt = ch->pcdata->fprompt;
  }
  else
    prompt = ch->pcdata->prompt;
  if ( ansi )
  {
    /* Most clients, especially mud clients, don't revert to gray but
       to a 'default color' after a reset sequence.  If we set to gray
       the prevcolor field we may experience color bleeding since 
       make_color_sequence will try to curb on redundant codes based
       on prevcolor, hence ignoring explicit requests to set a Gray
       foreground - Luc  09/2000 */
    strcpy(pbuf, "\033[0;37m");
    d->prevcolor = 0x07;
    pbuf += 7;
  }
  for ( ; *prompt; prompt++ )
  {
    /*
     * '&' = foreground color/intensity bit
     * '^' = background color/blink bit
     * '%' = prompt commands
     * Note: foreground changes will revert background to 0 (black)
     */
    if ( *prompt != '&' && *prompt != '^' && *prompt != '%' )
    {
      *(pbuf++) = *prompt;
      continue;
    }
    ++prompt;
    if ( !*prompt )
      break;
    if ( *prompt == *(prompt-1) )
    {
      *(pbuf++) = *prompt;
      continue;
    }
    switch(*(prompt-1))
    {
    default:
      bug( "Display_prompt: bad command char '%c'.", *(prompt-1) );
      break;
    case '&':
    case '^':
      stat = make_color_sequence(&prompt[-1], pbuf, d);
      if ( stat < 0 )
        --prompt;
      else if ( stat > 0 )
        pbuf += stat;
      break;
    case '%':
      *pbuf = '\0';
      stat = 0x80000000;
      switch(*prompt)
      {
      case '%':
	*pbuf++ = '%';
	*pbuf = '\0';
	break;
      case 'a':
	if ( ch->level >= 10 )
	  stat = ch->alignment;
	else if ( IS_GOOD(ch) )
	  strcpy(pbuf, "good");
	else if ( IS_EVIL(ch) )
	  strcpy(pbuf, "evil");
	else
	  strcpy(pbuf, "neutral");
	break;
      case 'A':
	sprintf( pbuf, "%s%s%s", IS_AFFECTED( ch, AFF_INVISIBLE ) ? "I" : "",
				 IS_AFFECTED( ch, AFF_HIDE )      ? "H" : "",
				 IS_AFFECTED( ch, AFF_SNEAK )     ? "S" : "" );
        break;
        case 'C':  /* Tank */
	  if ( !IS_IMMORTAL( ch ) ) break;
          if ( !ch->fighting || ( victim = ch->fighting->who ) == NULL )
             strcpy( pbuf, "N/A" );
          else if(!victim->fighting||(victim = victim->fighting->who)==NULL)
             strcpy( pbuf, "N/A" );
          else {
              if ( victim->max_hit > 0 )
                    percent = (100 * victim->hit) / victim->max_hit;
              else
                    percent = -1;
                   if (percent >= 100)
                       strcpy (pbuf, "perfect health");
                   else if (percent >= 90)
                       strcpy (pbuf, "slightly scratched");
                   else if (percent >= 80)
                       strcpy (pbuf, "few bruises");
                    else if (percent >= 70)
                       strcpy (pbuf, "some cuts");
                    else if (percent >= 60)
                       strcpy (pbuf, "several wounds");
                    else if (percent >= 50)
                       strcpy (pbuf, "nasty wounds");
                    else if (percent >= 40)
                       strcpy (pbuf, "bleeding freely");
                    else if (percent >= 30)
                       strcpy (pbuf, "covered in blood");
                    else if (percent >= 20)
                       strcpy (pbuf, "leaking guts");
                    else if (percent >= 10)
                       strcpy (pbuf, "almost dead");
                    else
                       strcpy (pbuf, "DYING");
             }
          break;
        case 'c':
	  if ( !IS_IMMORTAL( ch ) && IS_PKILL( ch ) ) break;
          if ( !ch->fighting || ( victim = ch->fighting->who ) == NULL )
             strcpy( pbuf, "N/A" );
          else {
              if ( victim->max_hit > 0 )
                    percent = (100 * victim->hit) / victim->max_hit;
              else
                    percent = -1;
                if (percent >= 100)
                       strcpy (pbuf, "perfect health");
                    else if (percent >= 90)
                       strcpy (pbuf, "slightly scratched");
                    else if (percent >= 80)
                       strcpy (pbuf, "few bruises");
                    else if (percent >= 70)
                       strcpy (pbuf, "some cuts");
                    else if (percent >= 60)
                       strcpy (pbuf, "several wounds");
                    else if (percent >= 50)
                       strcpy (pbuf, "nasty wounds");
                    else if (percent >= 40)
                       strcpy (pbuf, "bleeding freely");
                    else if (percent >= 30)
                       strcpy (pbuf, "covered in blood");
                    else if (percent >= 20)
                       strcpy (pbuf, "leaking guts");
                    else if (percent >= 10)
                       strcpy (pbuf, "almost dead");
                    else
                       strcpy (pbuf, "DYING");
           }
          break;
      case 'h':
	stat = ch->hit;
	break;
      case 'H':
	stat = ch->max_hit;
	break;
      case 'm':
	if ( IS_VAMPIRE(ch) )
	  stat = 0;
	else
	  stat = ch->mana;
	break;
      case 'M':
	if ( IS_VAMPIRE(ch) )
	  stat = 0;
	else
	  stat = ch->max_mana;
	break;
        case 'N': /* Tank */
	  if ( !IS_IMMORTAL(ch) ) break;
          if ( !ch->fighting || ( victim = ch->fighting->who ) == NULL )
            strcpy( pbuf, "N/A" );
          else if(!victim->fighting||(victim=victim->fighting->who)==NULL)
            strcpy( pbuf, "N/A" );
          else {
            if ( ch == victim )
                    strcpy ( pbuf, "You" );
            else if ( IS_NPC(victim) )
                    strcpy ( pbuf, victim->short_descr );
            else
                    strcpy ( pbuf, victim->name );
                pbuf[0] = UPPER( pbuf[0] );
          }
          break;
        case 'n':
	  if ( !IS_IMMORTAL(ch) && IS_PKILL( ch ) ) break;
          if (!ch->fighting || (victim = ch->fighting->who) == NULL )
            strcpy( pbuf, "N/A" );
          else {
            if ( ch == victim )
                    strcpy ( pbuf, "You" );
            else if ( IS_NPC(victim) )
                    strcpy ( pbuf, victim->short_descr );
            else
                    strcpy ( pbuf, victim->name );
            pbuf[0] = UPPER( pbuf[0] );
          }
          break;
      case 'T':
        if      ( time_info.hour <  5 ) strcpy( pbuf, "night" );
        else if ( time_info.hour <  6 ) strcpy( pbuf, "dawn" );
        else if ( time_info.hour < 19 ) strcpy( pbuf, "day" );
        else if ( time_info.hour < 21 ) strcpy( pbuf, "dusk" );
        else                            strcpy( pbuf, "night" );
        break;
      case 'b':
	if ( IS_VAMPIRE(ch) )
	  stat = ch->pcdata->condition[COND_BLOODTHIRST];
	else
	  stat = 0;
	break;
      case 'B':
	if ( IS_VAMPIRE(ch) )
	  stat = ch->level + 10;
	else
	  stat = 0;
	break;
      case 'u':
	stat = num_descriptors;
	break;
      case 'U':
	stat = sysdata.maxplayers;
	break;
      case 'L': /* Heck, why not -- Blodkai, 1/5/2001 */
        if ( ch->wait <= 0 )
          sprintf( pbuf, " " );
        else
        if ( ch->wait <= 6 )
          sprintf( pbuf, "|" );
        else
        if ( ch->wait <= 12 )
          sprintf( pbuf, "||" );
        else
        if ( ch->wait <= 18 )
          sprintf( pbuf, "|||" );
        else
        if ( ch->wait <= 24 )
          sprintf( pbuf, "||||" );
        else
          sprintf( pbuf, "!!!!!" );
        break;
      case 'v':
	stat = ch->move;
	break;
      case 'V':
	stat = ch->max_move;
	break;
      case 'g':
	stat = ch->gold;
	break;
      case 'r':
	if ( IS_IMMORTAL(och) )
	  stat = ch->in_room->vnum;
	break;
      case 'F':
	if ( IS_IMMORTAL( och ) )
	  sprintf( pbuf, "%s", ext_flag_string( &ch->in_room->room_flags, r_flags) );
	break;
    case 'D': /*display DND status*/
      if ( IS_IMMORTAL(ch) )
      {
         if ( IS_SET(ch->pcdata->flags, PCFLAG_DND) )
            strcpy (pbuf, "DND");
      }
      break;
    case 'p': /* display who is on appear list. */
      if ( IS_IMMORTAL( ch ) )
		sprintf( pbuf, "(%s)", ch->pcdata->see_me[0] ? ch->pcdata->see_me : "" );
      break;
      case 'R':
	if ( xIS_SET(och->act, PLR_ROOMVNUM) )
	  sprintf(pbuf, "<#%d> ", ch->in_room->vnum);
	break;
      case 'x':
	stat = ch->exp;
	break;
      case 'X':
	stat = exp_level(ch, ch->level+1) - ch->exp;
	break;
      case 'w':
         stat = ch->carry_weight;
         break;
      case 'W':
         stat = can_carry_w(ch);
         break;
      case 'o':         /* display name of object on auction */
        if ( auction->item )
           strcpy( pbuf, auction->item->name );
        break;
      case 'S':
        if      ( ch->style == STYLE_BERSERK )    strcpy( pbuf, "B" );
        else if ( ch->style == STYLE_AGGRESSIVE ) strcpy( pbuf, "A" );
        else if ( ch->style == STYLE_DEFENSIVE )  strcpy( pbuf, "D" );
        else if ( ch->style == STYLE_EVASIVE )    strcpy( pbuf, "E" );
        else                                      strcpy( pbuf, "S" );
	break;
      case 'i':
	if ( (!IS_NPC(ch) && xIS_SET(ch->act, PLR_WIZINVIS)) ||
	      (IS_NPC(ch) && xIS_SET(ch->act, ACT_MOBINVIS)) )
	  sprintf(pbuf, "(Invis %d) ", (IS_NPC(ch) ? ch->mobinvis : ch->pcdata->wizinvis));
	else
	if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
	  sprintf(pbuf, "(Invis) " );
	break;
      case 'I':
	stat = (IS_NPC(ch) ? (xIS_SET(ch->act, ACT_MOBINVIS) ? ch->mobinvis : 0)
	     : (xIS_SET(ch->act, PLR_WIZINVIS) ? ch->pcdata->wizinvis : 0));
	break;
      }
      if ( stat != 0x80000000 )
	sprintf(pbuf, "%d", stat);
      pbuf += strlen(pbuf);
      break;
    }
  }
  *pbuf = '\0';
  write_to_buffer(d, buf, (pbuf-buf));
  return;
}


/* Huge modifications due to a slew of old bugs causing
   color bleeding. See also set_char_color() - Luc 09/2000 */
int make_color_sequence( const char *col, char *buf, DESCRIPTOR_DATA *d ) {
   int ln = 0;
   const char *ctype = col;
   unsigned char cl;
   CHAR_DATA *och;
   bool ansi;

   och = (d->original ? d->original : d->character);
   ansi = (!IS_NPC(och) && xIS_SET(och->act, PLR_ANSI));
   col++;
   if ( !*col )
      ln = -1;
   else if ( *ctype != '&' && *ctype != '^' ) {
      bug("Make_color_sequence: command '%c' not '&' or '^'.", *ctype);
      ln = -1;
      }
   else if ( *col == *ctype ) {
      buf[0] = *col;
      buf[1] = '\0';
      ln = 1;
      }
   else if ( !ansi )
      ln = 0;
   else {
      cl = d->prevcolor;
      switch( *ctype ) {
         default:
            bug( "Make_color_sequence: bad command char '%c'.", *ctype );
            ln = -1;
            break;
         case '&':
            if ( *col == '-' ) {
               buf[0] = '~';
               buf[1] = '\0';
               ln = 1;
               break;
               }
         case '^': {
               /* Getcolor() returns color and intensity infos, 4 bits */
               int newcol = getcolor( *col );

               if ( newcol < 0 ) {
                  ln = 0;
                  break;
                  }
               else if ( *ctype == '&' )
                  cl = (cl & 0xF0) | newcol;
               else
                  cl = ( cl & 0x0F ) | ( newcol << 4 );
               }
            if ( cl == d->prevcolor ) {
               ln = 0;
               break;
               }

            /* Any color-enabled client understands a color-change sequence,
               but it seems most of them don't understand intensity or blink
               reset codes, only the 'set' ones or the general reset. Other
               clients instead do reset to weird foreground colors instead of
               the standard grey, so to obtain the same behavior through the
               spectrum we will resend the whole shabang if a reset occur to
               either intensity or blink :P              
               Luc - 09/2000 */

            if ( ( ( d->prevcolor ^ cl ) & 0x88 ) & ( cl ^ 0x88 ) ) {
               /* One or both the attribs have been reset.  Let's
                  resend the whole color status.  Luc 09/2000 */
               ln = sprintf( buf, "\033[0;%s%s3%d;",
                  ( cl & 0x08 ) ? "1;" : "",
                  ( cl & 0x80 ) ? "5;" : "",
                  cl & 0x07 );
               if ( ( cl & 0x70 ) != 0 )
                  ln += sprintf( buf + ln, "4%d;", ( cl & 0x70 ) >> 4 );
               buf[ln - 1] = 'm';
               }
            else {
               /* No attribs cleared, so let's send only what's needed,
                  plus a changed attribute can only mean it has been set,
                  not reset, otherwise we would not be here. Luc 09/2000 */
               unsigned char changes = cl ^ d->prevcolor;

               ln = sprintf( buf, "\033[" );
               if ( changes & 0x08 ) /* High Brightness */
                  ln += sprintf ( buf + ln, "1;" );
               if ( changes & 0x80 ) /* Blink */
                  ln += sprintf ( buf + ln, "5;" );
               if ( changes & 0x07 ) /* Foreground */
                  ln += sprintf( buf + ln, "3%d;", cl & 0x07 );
               if ( changes & 0x70 ) /* Background */
                  ln += sprintf( buf + ln, "4%d;", ( cl & 0x70 ) >> 4 );
               buf[ln - 1] = 'm';
               }
            d->prevcolor = cl;
            }
         }
      if ( ln <= 0 )
         *buf = '\0';
      return ln;
   }



void set_pager_input( DESCRIPTOR_DATA *d, char *argument )
{
  while ( isspace(*argument) )
    argument++;
  d->pagecmd = *argument;
  return;
}

/* Well, Daltorak figured this could do with some commentting, so here goes -- Alty */
bool pager_output( DESCRIPTOR_DATA *d )
{
  register char *last;
  CHAR_DATA *ch;
  int pclines;
  register int lines;
  bool ret;

  if ( !d || !d->pagepoint || d->pagecmd == -1 )
    return TRUE;
  ch = d->original ? d->original : d->character;
  /* Calculate # lines on player's screen */
  pclines = UMAX(ch->pcdata->pagerlen, 5) - 1;
  switch(LOWER(d->pagecmd))
  {
  /* Next page */
  default:
    lines = 0;
    break;
  /* Previous page */
  case 'b':
    lines = -1-(pclines*2);
    break;
  /* Redraw */
  case 'r':
    lines = -1-pclines;
    break;
  /* Non-stop. */
  case 'n':
    lines = 0;
    pclines = 0x7FFFFFFF;	/* As many lines as possible */
    break;
  /* Quit */
  case 'q':
    d->pagetop = 0;
    d->pagepoint = NULL;
    flush_buffer(d, TRUE);
    DISPOSE(d->pagebuf);
    d->pagesize = MAX_STRING_LENGTH;
    return TRUE;
  }
  /* Search back if we need to. */
  while ( lines < 0 && --d->pagepoint >= d->pagebuf )
    if ( *d->pagepoint == '\n' )
      ++lines;
  /* pagepoint could go out of range in the above loop */
  if ( d->pagepoint < d->pagebuf )
    d->pagepoint = d->pagebuf;
  /* Skip a blank line if there is one.  Could make this skip all blanks, but
     that was annoying (screwed up some places where blanks were wanted) */
  if ( *d->pagepoint == '\n' && *(++d->pagepoint) == '\r' )
      ++d->pagepoint;
  /* Find the end of the page */
  for ( lines = 0, last = d->pagepoint; lines < pclines; ++last )
    if ( !*last )
      break;
    else if ( *last == '\n' )
      ++lines;
  if ( *last == '\r' )
    ++last;
  /* If there's anything to show, show it */
  if ( last != d->pagepoint )
  {
    if ( !write_to_descriptor(d->descriptor, d->pagepoint,
          (last-d->pagepoint)) )
      return FALSE;
    d->pagepoint = last;
  }
  /* If only whitespace remaining, terminate paging */
  while ( isspace(*last) )
    ++last;
  if ( !*last )
  {
    d->pagetop = 0;
    d->pagepoint = NULL;
    flush_buffer(d, TRUE);
    DISPOSE(d->pagebuf);
    d->pagesize = MAX_STRING_LENGTH;
    return TRUE;
  }
  /* Display the pager prompt */
  d->pagecmd = -1;
  if ( xIS_SET( ch->act, PLR_ANSI ) )
      if ( write_to_descriptor(d->descriptor, "\033[1;36m", 7) == FALSE )
	return FALSE;
  if ( (ret=write_to_descriptor(d->descriptor,
	"(C)ontinue, (N)on-stop, (R)efresh, (B)ack, (Q)uit: [C] ", 0)) == FALSE )
	return FALSE;
  /* Restore previous color */
  if ( xIS_SET( ch->act, PLR_ANSI ) )
  {
      char buf[32];

      if ( d->prevcolor == 7 )
	strcpy( buf, "\033[m" );
      else
	sprintf( buf, "\033[0;%s%s3%dm",
           d->prevcolor & 8 ? "1;" : "",
           d->prevcolor > 15 ? "5;" : "",
           d->prevcolor & 7 );
      ret = write_to_descriptor( d->descriptor, buf, 0 );
  }
  return ret;
}


#ifdef WIN32

void shutdown_mud( char *reason );

void bailout(void)
{
    echo_to_all( AT_IMMORT, "MUD shutting down by system operator NOW!!", ECHOTAR_ALL );
    shutdown_mud( "MUD shutdown by system operator" );
    log_string ("MUD shutdown by system operator");
    Sleep (5000);		/* give "echo_to_all" time to display */
    mud_down = TRUE;		/* This will cause game_loop to exit */
    service_shut_down = TRUE;	/* This will cause characters to be saved */
    fflush(stderr);
    return;
}

#endif
