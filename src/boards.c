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
 *			     Special boards module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "mud.h"
#ifdef USE_IMC
#include "imc-mercbase.h"
#endif


/* Defines for voting on notes. -- Edmond - update for ballot */
#define VOTE_NONE 0
#define VOTE_CLOSED 1
#define VOTE_BCLOSED 2
#define VOTE_OPEN 3
#define VOTE_BALLOT 4

BOARD_DATA *		first_board;
BOARD_DATA *		last_board;

bool	is_note_to	args( ( CHAR_DATA *ch, NOTE_DATA *pnote ) );
void	note_attach	args( ( CHAR_DATA *ch ) );
void	note_remove	args( ( CHAR_DATA *ch, BOARD_DATA *board,
				NOTE_DATA *pnote ) );
void  	do_note		args( ( CHAR_DATA *ch, char *arg_passed, bool IS_MAIL) );



bool can_remove( CHAR_DATA *ch, BOARD_DATA *board )
{
  /* If your trust is high enough, you can remove it. */
  if ( get_trust( ch ) >= board->min_remove_level )
    return TRUE;
  
  if ( board->extra_removers[0] != '\0' )
  {
    if ( is_name( ch->name, board->extra_removers ) )
      return TRUE;
  }
  return FALSE;
}
 
bool can_read( CHAR_DATA *ch, BOARD_DATA *board )
{
  /* If your trust is high enough, you can read it. */
  if ( get_trust( ch ) >= board->min_read_level )
    return TRUE;

  /* Your trust wasn't high enough, so check if a read_group or extra 
     readers have been set up. */
  if ( board->read_group[0] != '\0' )
  {
    if ( ch->pcdata->clan && !str_cmp( ch->pcdata->clan->name, board->read_group ) ) 
      return TRUE; 
    if ( ch->pcdata->council && !str_cmp( ch->pcdata->council->name, board->read_group ) )
      return TRUE; 
  }
  if ( board->extra_readers[0] != '\0' )
  {
    if ( is_name( ch->name, board->extra_readers ) )
      return TRUE;
  } 
  return FALSE;
}

bool can_post( CHAR_DATA *ch, BOARD_DATA *board )
{
  /* If your trust is high enough, you can post. */
  if ( get_trust( ch ) >= board->min_post_level )
    return TRUE;

  /* Your trust wasn't high enough, so check if a post_group has been set up. */
  if ( board->post_group[0] != '\0' )
  {
    if ( ch->pcdata->clan && !str_cmp( ch->pcdata->clan->name, board->post_group ) ) 
      return TRUE; 
    if ( ch->pcdata->council && !str_cmp( ch->pcdata->council->name, board->post_group ) )
      return TRUE; 
  }
  return FALSE;
}


/*
 * board commands.
 */
void write_boards_txt( )
{
    BOARD_DATA *tboard;
    FILE *fpout;
    char filename[256];

    sprintf( filename, "%s%s", BOARD_DIR, BOARD_FILE );
    fpout = fopen( filename, "w" );
    if ( !fpout )
    {
	bug( "FATAL: cannot open board.txt for writing!\n\r", 0 );
 	return;
    }	  
    for ( tboard = first_board; tboard; tboard = tboard->next )
    {
	fprintf( fpout, "Filename          %s~\n", tboard->note_file	    );
	fprintf( fpout, "Vnum              %d\n",  tboard->board_obj	    );
	fprintf( fpout, "Min_read_level    %d\n",  tboard->min_read_level   );
	fprintf( fpout, "Min_post_level    %d\n",  tboard->min_post_level   );
	fprintf( fpout, "Min_remove_level  %d\n",  tboard->min_remove_level );
	fprintf( fpout, "Max_posts         %d\n",  tboard->max_posts	    );
       	fprintf( fpout, "Type 	           %d\n",  tboard->type		    ); 
	fprintf( fpout, "Read_group        %s~\n", tboard->read_group       );
	fprintf( fpout, "Post_group        %s~\n", tboard->post_group       );
	fprintf( fpout, "Extra_readers     %s~\n", tboard->extra_readers    );
        fprintf( fpout, "Extra_removers    %s~\n", tboard->extra_removers   );
        fprintf( fpout, "Extra_ballots     %s~\n", tboard->extra_ballots    );
        fprintf( fpout, "Ballot_level      %d\n", tboard->min_ballot_level  );
	if ( tboard->ocopymessg )
          fprintf( fpout, "OCopymessg	   %s~\n", tboard->ocopymessg       );
	if ( tboard->olistmessg )
          fprintf( fpout, "OListmessg	   %s~\n", tboard->olistmessg       );
	if ( tboard->opostmessg )
          fprintf( fpout, "OPostmessg	   %s~\n", tboard->opostmessg       );
	if ( tboard->oreadmessg )
	  fprintf( fpout, "OReadmessg	   %s~\n", tboard->oreadmessg       );
	if ( tboard->oremovemessg )
          fprintf( fpout, "ORemovemessg      %s~\n", tboard->oremovemessg     );
	if ( tboard->otakemessg )
	  fprintf( fpout, "OTakemessg	   %s~\n", tboard->otakemessg       );
	if ( tboard->postmessg )
	  fprintf( fpout, "Postmessg	   %s~\n", tboard->postmessg        );
	fprintf( fpout, "End\n" );
    }
    fclose( fpout );
}

BOARD_DATA *get_board( OBJ_DATA *obj )
{
    BOARD_DATA *board;
    
    for ( board = first_board; board; board = board->next )
       if ( board->board_obj == obj->pIndexData->vnum )
         return board;
    return NULL;	
}

BOARD_DATA *find_board( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    BOARD_DATA  *board;

    for ( obj = ch->in_room->first_content;
	  obj;
	  obj = obj->next_content )
    {
	if ( (board = get_board(obj)) != NULL )
	    return board;
    }

    return NULL;
}


bool is_note_to( CHAR_DATA *ch, NOTE_DATA *pnote )
{
    if ( !str_cmp( ch->name, pnote->sender ) )
	return TRUE;

/*  PROJECT 57
    if ( is_name( "all", pnote->to_list ) )
	return TRUE;
*/
    if ( IS_HERO(ch) && is_name( "immortal", pnote->to_list ) )
	return TRUE;

    if ( is_name( ch->name, pnote->to_list ) )
	return TRUE;

    return FALSE;
}



void note_attach( CHAR_DATA *ch )
{
    NOTE_DATA *pnote;

    if ( ch->pnote )
	return;

    CREATE( pnote, NOTE_DATA, 1 );
    pnote->next		= NULL;
    pnote->prev		= NULL;
    pnote->sender	= QUICKLINK( ch->name );
    pnote->date		= STRALLOC( "" );
    pnote->to_list	= STRALLOC( "" );
    pnote->subject	= STRALLOC( "" );
    pnote->text		= STRALLOC( "" );
    ch->pnote		= pnote;
    return;
}

void write_board( BOARD_DATA *board )
{
    FILE *fp;
    char filename[256];
    NOTE_DATA *pnote;

    /*
     * Rewrite entire list.
     */
    fclose( fpReserve );
    sprintf( filename, "%s%s", BOARD_DIR, board->note_file );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
	perror( filename );
    }
    else
    {
	for ( pnote = board->first_note; pnote; pnote = pnote->next )
	{
	    fprintf( fp, "Sender  %s~\nDate    %s~\nTo      %s~\nSubject %s~\nVoting %d\nYesvotes %s~\nNovotes %s~\nAbstentions %s~\nYestally %d\nNotally %d\nAbstaintally %d\nNoremove %d\nText\n%s~\n\n",
		pnote->sender,
		pnote->date,
		pnote->to_list,
		pnote->subject,
                pnote->voting,
                pnote->yesvotes,
                pnote->novotes,
                pnote->abstentions,
                pnote->yestally,    
                pnote->notally,    
                pnote->abstaintally,                                                
		pnote->no_remove,
		pnote->text
		);
	}
	fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}


void free_note( NOTE_DATA *pnote )
{
    STRFREE( pnote->text    );
    STRFREE( pnote->subject );
    STRFREE( pnote->to_list );
    STRFREE( pnote->date    );
    STRFREE( pnote->sender  );
    if ( pnote->yesvotes )
	DISPOSE( pnote->yesvotes );
    if ( pnote->novotes )
	DISPOSE( pnote->novotes );
    if ( pnote->abstentions )
	DISPOSE( pnote->abstentions );
    DISPOSE( pnote );
}

void note_remove( CHAR_DATA *ch, BOARD_DATA *board, NOTE_DATA *pnote )
{

    if ( !board )
    {
      bug( "note remove: null board", 0 );
      return;
    }

    if ( !pnote )
    {
      bug( "note remove: null pnote", 0 );
      return;
    }
    
    /*
     * Remove note from linked list.
     */
    UNLINK( pnote, board->first_note, board->last_note, next, prev );

    --board->num_posts;
    free_note( pnote );
    write_board( board );
}


OBJ_DATA *find_quill( CHAR_DATA *ch )
{
    OBJ_DATA *quill;

    for ( quill = ch->last_carrying; quill; quill = quill->prev_content )
 	if ( quill->item_type == ITEM_PEN
        &&   can_see_obj( ch, quill ) )
	  return quill;
    return NULL;
}

void do_noteroom( CHAR_DATA *ch, char *argument )
{
    BOARD_DATA *board;
    char arg[MAX_STRING_LENGTH];
    char arg_passed[MAX_STRING_LENGTH];

    strcpy(arg_passed, argument);

    switch( ch->substate )
    {
	case SUB_WRITING_NOTE:
	do_note(ch, arg_passed, FALSE);
 	break;

	default:

    argument = one_argument(argument, arg);  
    smash_tilde( argument );
    if (!str_cmp(arg, "write") || !str_cmp(arg, "to") 
    ||  !str_cmp(arg, "subject") || !str_cmp(arg, "show"))        
    {
        do_note(ch, arg_passed, FALSE);
        return;  
    }
 	    
    board = find_board( ch );
    if ( !board )
    {
        send_to_char( "There is no bulletin board here to look at.\n\r", ch );
        return;
    }

    if (board->type != BOARD_NOTE)
    {
      send_to_char("You can only use note commands on a note board.\n\r", ch);
      return;
    }
    else
    {
      do_note(ch, arg_passed, FALSE);
      return;
    }
  }
}

void do_mailroom(CHAR_DATA *ch, char *argument)
{
    BOARD_DATA *board;
    char arg[MAX_STRING_LENGTH];
    char arg_passed[MAX_STRING_LENGTH];

    strcpy(arg_passed, argument);

    switch( ch->substate )
    {
	case SUB_WRITING_NOTE:
	do_note(ch, arg_passed, TRUE);
 	break;

	default:

    argument = one_argument(argument, arg);
    smash_tilde( argument );
    if (!str_cmp(arg, "write") || !str_cmp(arg, "to") 
    ||  !str_cmp(arg, "subject") || !str_cmp(arg, "show"))        
    {
        do_note(ch, arg_passed, TRUE);
	return;  
    }

    board = find_board( ch );
    if ( !board )
    {
        send_to_char( "There is no mail facility here.\n\r", ch );
        return;
    }

    if (board->type != BOARD_MAIL)
    {
	send_to_char("You can only use mail commands in a post office.\n\r", ch);
	return;
    }
    else
    {
	do_note(ch, arg_passed, TRUE);
	return;
    }
  }
}

void do_note( CHAR_DATA *ch, char *arg_passed, bool IS_MAIL )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    NOTE_DATA  *pnote;
    BOARD_DATA *board;
    int vnum;
    int anum;
    int first_list;
    OBJ_DATA *quill = NULL, *paper = NULL, *tmpobj = NULL;
    EXTRA_DESCR_DATA *ed = NULL;
    char notebuf[MAX_STRING_LENGTH];  
    char short_desc_buf[MAX_STRING_LENGTH];
    char long_desc_buf[MAX_STRING_LENGTH];
    char keyword_buf[MAX_STRING_LENGTH];
    bool mfound = FALSE;
	bool name_list = FALSE;
/*
	bool noarg = TRUE;
*/

    if ( IS_NPC(ch) )
	return;

    if ( !ch->desc )
    {
	bug( "do_note: no descriptor", 0 );
	return;
    }

    switch( ch->substate )
    {
	default:
	  break;
	case SUB_WRITING_NOTE:
	  if ( ( paper = get_eq_char(ch, WEAR_HOLD) ) == NULL
	  ||     paper->item_type != ITEM_PAPER )
	  {
	     bug("do_note: player not holding paper", 0);
	     stop_editing( ch );
	     return;
          }
	  ed = ch->dest_buf;
	  STRFREE( ed->description );
	  ed->description = copy_buffer( ch );
	  stop_editing( ch );	   
	  return;
    }

    set_char_color( AT_NOTE, ch );
    arg_passed = one_argument( arg_passed, arg );
    smash_tilde( arg_passed );

    /* Reusing alot of code but this will have to do till I can think
     * of a better way to do it. --Shaddai
     */

#ifdef O
    if ( !str_cmp( arg, "date" ) )
    {
	board = find_board( ch );
	if ( !board )
	{
	    send_to_char( "There is no board here to look at.\n\r", ch );
	    return;
	}
	if ( !can_read( ch, board ) )
	{
	    send_to_char( "You cannot make any sense of the cryptic scrawl on this board...\n\r", ch );
	    return;
	}

        first_list = atoi(arg_passed);
        if (first_list)
        {
	    if (IS_MAIL)
            {
		send_to_char( "You cannot use a list number (at this time) with mail.\n\r", ch);
		return;
 	    }
	  
	    if (first_list < 1)
	    {
		send_to_char( "You can't read a note before 1!\n\r", ch);
		return;
	    }
	}
        

        if (!IS_MAIL)
        {
	    set_pager_color( AT_NOTE, ch );
	    vnum = 0;
	    for ( pnote = board->first_note; pnote; pnote = pnote->next )
            {
		vnum++;
		if ( (first_list && vnum >= first_list) || !first_list )
		   pager_printf( ch, "%2d%c %-12s%c %-24s : %-35s\n\r",
			vnum,
			is_note_to( ch, pnote ) ? ')' : '}',
			pnote->sender,
                        (pnote->voting != VOTE_NONE) ?
                                ( (pnote->voting > VOTE_BCLOSED ) ?
                                        ((pnote->voting == VOTE_OPEN) ? 'V' : 'B')
                                        : 'C' )
                                 :  ':',                                                                                                        
			pnote->date,
			pnote->subject );
	    }
	    if ( board->olistmessg )
	        act( AT_ACTION, board->olistmessg, ch, NULL, NULL, TO_CANSEE ); 
	    else
	        act( AT_ACTION, "$n glances over the notes.", ch, NULL, NULL, TO_CANSEE );
	    return;
	}
	else
	{
      	    vnum = 0;


            if (IS_MAIL) /* SB Mail check for Brit */
             {
              for ( pnote = board->first_note; pnote; pnote = pnote->next )
                if (is_note_to( ch, pnote )) mfound = TRUE;

              if ( !mfound && get_trust(ch) < sysdata.read_all_mail ) 
               {
                ch_printf( ch, "You have no mail.\n\r");
                return;
               }
             }

            for ( pnote = board->first_note; pnote; pnote = pnote->next )
		if (is_note_to( ch, pnote ) || get_trust(ch) >= sysdata.read_all_mail)
          	    ch_printf( ch, "%2d%c %-12s: %-13s : %s\n\r",
			++vnum,
			is_note_to( ch, pnote ) ? '-' : '}',
			pnote->sender,
			pnote->subject );
	    return;
	}
    }
#endif

    if ( !str_cmp( arg, "list" ) || !str_cmp(arg, "mine" ) )
    {
	int mine=0;
	board = find_board( ch );
	if ( !board )
	{
	    send_to_char( "There is no board here to look at.\n\r", ch );
	    return;
	}
	if ( !can_read( ch, board ) )
	{
	    send_to_char( "You cannot make any sense of the cryptic scrawl on this board...\n\r", ch );
	    return;
	}
        if ( !str_cmp( arg, "mine" ) )
                mine = 1;

		/* Check for note list <player> */
		if (!is_number(arg_passed) && arg_passed[0] != '\0' )
			name_list = TRUE;

        first_list = atoi(arg_passed);
		arg_passed = one_argument( arg_passed, arg ); /*For note list <player>*/
        if (first_list)
        {
	    if (IS_MAIL)
            {
		send_to_char( "You cannot use a list number (at this time) with mail.\n\r", ch);
		return;
 	    }
	  
	    if (first_list < 1)
	    {
		send_to_char( "You can't read a note before 1!\n\r", ch);
		return;
	    }
	}
        

        if (!IS_MAIL)
        {
	    set_pager_color( AT_NOTE, ch );
	    vnum = 0;
	    anum = 0;
		for ( pnote = board->first_note; pnote; pnote = pnote->next )
            {
		vnum++;
		if ( ( first_list && vnum >= first_list)
        || ( name_list && !str_cmp( arg, pnote->sender ) )
        || ( !name_list && !first_list ) )
			{
		   pager_printf( ch, "%2d%c %-12s%c %-12.12s : %s\n\r",
			vnum,
			is_note_to( ch, pnote ) ? ')' : '}',
			pnote->sender,
                        (pnote->voting != VOTE_NONE) ?
                                ( (pnote->voting > VOTE_BCLOSED) ?
                                        ( (pnote->voting == VOTE_OPEN) ? 'V' : 'B')
                                : 'C' )
                        :  ':',                                                                                                                 
			pnote->to_list,
			pnote->subject );
		    anum++;
			}
	    }
		if ( anum == 0 && name_list )
			pager_printf( ch, "There are no notes posted by %s.\n\r", capitalize(arg) );
	    if ( board->olistmessg )
		act( AT_ACTION, board->olistmessg, ch, NULL, NULL, TO_CANSEE );
	    else
		act( AT_ACTION, "$n glances over the notes.", ch, NULL, NULL, TO_CANSEE );
	    return;
	}
	else
	{
      	    vnum = 0;


            if (IS_MAIL) /* SB Mail check for Brit */
             {
              for ( pnote = board->first_note; pnote; pnote = pnote->next )
                if (is_note_to( ch, pnote )) mfound = TRUE;

              if ( !mfound && get_trust(ch) < sysdata.read_all_mail ) 
               {
                ch_printf( ch, "You have no mail.\n\r");
                return;
               }
             }

            for ( pnote = board->first_note; pnote; pnote = pnote->next )
                if (is_note_to( ch, pnote ) || (!mine && get_trust(ch) >= sysdata.read_all_mail) )
          	    ch_printf( ch, "%2d%c %s: %s\n\r",
			++vnum,
			is_note_to( ch, pnote ) ? '-' : '}',
			pnote->sender,
			pnote->subject );
                else if ( mine && get_trust(ch) >= sysdata.read_all_mail )
                        vnum++;
	    return;
	}
    }

    if ( !str_cmp( arg, "read" ) )
    {
	bool fAll;

	board = find_board( ch );
	if ( !board )
	{
	    send_to_char( "There is no board here to look at.\n\r", ch );
	    return;
	}
	if ( !can_read( ch, board ) ) 
	{
	    send_to_char( "You cannot make any sense of the cryptic scrawl on this board...\n\r", ch );
	    return;
	}

	if ( !str_cmp( arg_passed, "all" ) )
	{
	    fAll = TRUE;
	    anum = 0;
	}
	else
	if ( is_number( arg_passed ) )
	{
	    fAll = FALSE;
	    anum = atoi( arg_passed );
	}
	else
	{
	    send_to_char( "Note read which number?\n\r", ch );
	    return;
	}

	set_pager_color( AT_NOTE, ch );
	if (!IS_MAIL)
 	{
	    vnum = 0;
	    for ( pnote = board->first_note; pnote; pnote = pnote->next )
            {
		vnum++;
		if ( vnum == anum || fAll )
		{
		    pager_printf( ch, "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r%s",
			vnum,
			pnote->sender,
			pnote->subject,
			pnote->date,
			pnote->to_list,
			pnote->text );

		    if ( pnote->yesvotes[0] != '\0' || pnote->novotes[0] != '\0'
                    || pnote->abstentions[0] != '\0' )
		    {
			send_to_pager( "------------------------------------------------------------\n\r", ch );
                         if ( pnote->voting == VOTE_OPEN ||
                                  pnote->voting == VOTE_CLOSED ||
                                  get_trust(ch) >= board->min_ballot_level ||
                                  ( board->extra_ballots[0] !='\0' && is_name( ch->name, board->extra_ballots ) ) )
                                pager_printf( ch, "Votes:\n\rYes:  %s\n\rNo:  %s\n\rAbstain: %s\n\r"
                                        , pnote->yesvotes , pnote->novotes, pnote->abstentions );
                         else
                                pager_printf( ch, "Votes:\n\rYes:  %d\n\rNo:  %d\n\rAbstain: %d\n\r"
                                        , pnote->yestally, pnote->notally, pnote->abstaintally );                                               
		    }
		    if ( board->oreadmessg )
			act( AT_ACTION, board->oreadmessg, ch, NULL, NULL, TO_CANSEE );
		    else
			act( AT_ACTION, "$n reads a note.", ch, NULL, NULL, TO_CANSEE );
		    return;
		}
	    }
	    send_to_char( "No such note.\n\r", ch );
	    return;
	}
	else
	{
	    vnum = 0;
	    for ( pnote = board->first_note; pnote; pnote = pnote->next )
	    {
		if (is_note_to(ch, pnote) || get_trust(ch) >= sysdata.read_all_mail)
		{
		    vnum++;
		    if ( vnum == anum || fAll )
		    {
			if ( ch->gold < 10
			&&   get_trust(ch) < sysdata.read_mail_free )
			{
			    send_to_char("It costs 10 gold coins to read a message.\n\r", ch);
			    return;
			}
			if (get_trust(ch) < sysdata.read_mail_free)
			   ch->gold -= 10;
			pager_printf( ch, "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r%s",
			    vnum,
			    pnote->sender,
			    pnote->subject,
			    pnote->date,
			    pnote->to_list,
			    pnote->text );
			return;
		    }     
		}
	    }
	    send_to_char( "No such message.\n\r", ch );
	    return;
	}
    }

    if ( !str_cmp( arg, "noremove" ) ) {
        char arg2[MAX_INPUT_LENGTH];
        arg_passed = one_argument( arg_passed, arg2 );

        if ( !IS_IMMORTAL(ch) ) {
                send_to_char("Huh?\n\r",ch);
        }
        board = find_board( ch );
        if ( !board )
        {
            send_to_char( "There is no bulletin board here.\n\r", ch );
            return;
        }
        if ( !can_read( ch, board ) )
        {
            send_to_char( "You cannot read this board.\n\r", ch );
            return;
        }
        if ( is_number( arg2 ) )
            anum = atoi( arg2 );
        else
        {
            send_to_char( "Set noremove on which number?\n\r", ch );
            return;
        }

        vnum = 1;
        for ( pnote = board->first_note; pnote && vnum < anum; pnote = pnote->next )
            vnum++;
        if ( !pnote )
        {
            send_to_char( "No such note.\n\r", ch );
            return;
        }
        if ( pnote->no_remove != 0 && pnote->no_remove > ch->level) {
            send_to_char("That note is already set to noremove.\n\r",ch);
            return;
        }
       pnote->no_remove = ch->level; 
       write_board( board );
       send_to_char("Noremove flag set.\n\r", ch );
       return;
    }


    /* Voting added by Narn, June '96 */
    if ( !str_cmp( arg, "vote" ) )
    {
	char arg2[MAX_INPUT_LENGTH];
	arg_passed = one_argument( arg_passed, arg2 ); 

	board = find_board( ch );
	if ( !board )
	{
	    send_to_char( "There is no bulletin board here.\n\r", ch );
	    return;
	}
	if ( !can_read( ch, board ) ) 
	{
            send_to_char( "You cannot vote on this board.\n\r", ch );
            return;
	}

	if ( is_number( arg2 ) )
	    anum = atoi( arg2 );
	else
	{
            send_to_char( "Note vote which number?\n\r", ch );
            return;
	}

	vnum = 1;
	for ( pnote = board->first_note; pnote && vnum < anum; pnote = pnote->next )
	    vnum++;
	if ( !pnote )
	{
	    send_to_char( "No such note.\n\r", ch );
	    return;
	}

	/* Options: open close yes no abstain */
	/* If you're the author of the note and can read the board you can open 
	   and close voting, if you can read it and voting is open you can vote.
	*/
	if ( !str_cmp( arg_passed, "open" ) )
	{
	    if ( str_cmp( ch->name, pnote->sender ) && get_trust( ch ) < LEVEL_GREATER )
	    {
		send_to_char( "You are not the author of this note.\n\r", ch );
		return;
	    }
                if ( pnote->voting )
                {
                        send_to_char( "This note has already been made a vote.", ch );
                        return;
                }                                                                                                                               
	    pnote->voting = VOTE_OPEN;
	    act( AT_ACTION, "$n opens voting on a note.", ch, NULL, NULL, TO_CANSEE );
            send_to_char( "Voting opened.\n\r", ch );
            write_board( board );
            return;
	}  
        if ( !str_cmp( arg_passed, "ballot" ) )
        {
            if ( str_cmp( ch->name, pnote->sender ) && get_trust( ch ) < LEVEL_GREATER )
            {
                send_to_char( "You are not the author of this note.\n\r", ch );
                return;
            }
                if ( pnote->voting )
                {
                        send_to_char( "This note has already been made a vote.", ch );
                        return;
                }
                pnote->voting = VOTE_BALLOT;
                act( AT_ACTION, "$n opens a ballot on a note.", ch, NULL, NULL, TO_CANSEE );
                send_to_char( "Ballot opened.\n\r", ch );
                write_board( board );
                return;
        }                                                                                                                                       
	if ( !str_cmp( arg_passed, "close" ) )
	{
	    if ( str_cmp( ch->name, pnote->sender ) )
	    {
		send_to_char( "You are not the author of this note.\n\r", ch );
		return;
	    }
            pnote->voting = ( pnote->voting == VOTE_BALLOT ? VOTE_BCLOSED : VOTE_CLOSED );                                                  
	    act( AT_ACTION, "$n closes voting on a note.", ch, NULL, NULL, TO_CANSEE );
	    send_to_char( "Voting closed.\n\r", ch );
	    write_board( board );
	    return;
	}  

	/* Make sure the note is open for voting before going on. */
        if ( (pnote->voting) < VOTE_OPEN )  
	{
	    send_to_char( "Voting is not open on this note.\n\r", ch );
	    return;
	}

	/* Can only vote once on a note. */
	sprintf( buf, "%s %s %s", 
		pnote->yesvotes, pnote->novotes, pnote->abstentions );
	if ( is_name( ch->name, buf ) )
	{
	    send_to_char( "You have already voted on this note.\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg_passed, "yes" ) )
	{
	    sprintf( buf, "%s %s", pnote->yesvotes, ch->name );
	    DISPOSE( pnote->yesvotes );
	    pnote->yesvotes = str_dup( buf );
                pnote->yestally = ( (pnote->yestally) ? pnote->yestally+1 : 1 ); 
	    act( AT_ACTION, "$n votes on a note.", ch, NULL, NULL, TO_CANSEE );
	    ch_printf( ch, "You vote yes on %s.\n\r", pnote->subject );
	    write_board( board );
	    return;
	}  
	if ( !str_cmp( arg_passed, "no" ) )
	{
	    sprintf( buf, "%s %s", pnote->novotes, ch->name );
	    DISPOSE( pnote->novotes );
	    pnote->novotes = str_dup( buf );
                pnote->notally = ( (pnote->notally) ? pnote->notally+1 : 1 );  
	    act( AT_ACTION, "$n votes on a note.", ch, NULL, NULL, TO_CANSEE );
	    ch_printf( ch, "You vote no on %s.\n\r", pnote->subject );
	    write_board( board );
	    return;
	}  
	if ( !str_cmp( arg_passed, "abstain" ) )
	{
	    sprintf( buf, "%s %s", pnote->abstentions, ch->name );
	    DISPOSE( pnote->abstentions );
	    pnote->abstentions = str_dup( buf );
                pnote->abstaintally = ( (pnote->abstaintally) ? pnote->abstaintally+1 : 1 );   
	    act( AT_ACTION, "$n votes on a note.", ch, NULL, NULL, TO_CANSEE );
	    ch_printf( ch, "You abstain on %s.\n\r", pnote->subject );
	    write_board( board );
	    return;
	}  
	do_note( ch, "", FALSE );
    }
    if ( !str_cmp( arg, "write" ) )
    {
	if ( ch->substate == SUB_RESTRICTED )
	{
	    send_to_char( "You cannot write a note from within another command.\n\r", ch );
	    return;
	}
	if (get_trust (ch) < sysdata.write_mail_free)
	{
	    quill = find_quill( ch );
            if (!quill)
	    {
		send_to_char("You need a quill to write a note.\n\r", ch);
		return;
	    }
	    if ( quill->value[0] < 1 )
	    {
		send_to_char("Your quill is dry.\n\r", ch);
		return;
	    }
	}
	if ( ( paper = get_eq_char(ch, WEAR_HOLD) ) == NULL
	||     paper->item_type != ITEM_PAPER )
	{
	    if (get_trust(ch) < sysdata.write_mail_free )
	    {
		send_to_char("You need to be holding a fresh piece of parchment to write a note.\n\r", ch);
		return;
	    }
	    paper = create_object( get_obj_index(OBJ_VNUM_NOTE), 0 );
	    if ((tmpobj = get_eq_char(ch, WEAR_HOLD)) != NULL)
	      unequip_char(ch, tmpobj); 
	    paper = obj_to_char(paper, ch);
	    equip_char(ch, paper, WEAR_HOLD);
	    act(AT_MAGIC, "A piece of parchment magically appears in $n's hands!",
      	        ch, NULL, NULL, TO_CANSEE);
  	    act(AT_MAGIC, "A piece of parchment appears in your hands.",
    	        ch, NULL, NULL, TO_CHAR);
	}
	if (paper->value[0] < 2 )
	{
	    paper->value[0] = 1;
	    ed = SetOExtra(paper, "_text_");
	    ch->substate = SUB_WRITING_NOTE;
	    ch->dest_buf = ed;
	    if ( get_trust(ch) < sysdata.write_mail_free )
		--quill->value[0];
	    start_editing( ch, ed->description );
	    return;
	}
	else
	{
	    send_to_char("You cannot modify this note.\n\r", ch);
	    return;
	}
    }

    if ( !str_cmp( arg, "subject" ) )
    {
	if(get_trust(ch) < sysdata.write_mail_free)
	{
	    quill = find_quill( ch );
	    if ( !quill )
	    {
		send_to_char("You need a quill to write a note.\n\r", ch);
		return;
	    }
	    if ( quill->value[0] < 1 )
	    {
		send_to_char("Your quill is dry.\n\r", ch);
		return;
	    }
	}
	if (!arg_passed || arg_passed[0] == '\0')
	{
	    send_to_char("What do you wish the subject to be?\n\r", ch);
	    return;
	}
	if ( ( paper = get_eq_char(ch, WEAR_HOLD) ) == NULL
	||     paper->item_type != ITEM_PAPER )
	{
	    if(get_trust(ch) < sysdata.write_mail_free )
	    {
		send_to_char("You need to be holding a fresh piece of parchment to write a note.\n\r", ch);
		return;
	    }
	    paper = create_object( get_obj_index(OBJ_VNUM_NOTE), 0 );
    	    if ((tmpobj = get_eq_char(ch, WEAR_HOLD)) != NULL)
		unequip_char(ch, tmpobj); 
	    paper = obj_to_char(paper, ch);
	    equip_char(ch, paper, WEAR_HOLD);
	    act(AT_MAGIC, "A piece of parchment magically appears in $n's hands!",
		ch, NULL, NULL, TO_CANSEE);
	    act(AT_MAGIC, "A piece of parchment appears in your hands.",
		ch, NULL, NULL, TO_CHAR);
	}
	if (paper->value[1] > 1 )
        {
	    send_to_char("You cannot modify this note.\n\r", ch);
	    return;
        }
        else
	{
	    paper->value[1] = 1;
	    ed = SetOExtra(paper, "_subject_");
	    STRFREE( ed->description );
	    ed->description = STRALLOC( arg_passed );
	    send_to_char("Ok.\n\r", ch);
	    return;
	}
    }

    if ( !str_cmp( arg, "to" ) )
    {
	struct stat fst;
/*	char *pn;*/
	char fname[1024];
#ifdef USE_IMC
	bool imc = FALSE;
#endif
	
	if(get_trust(ch) < sysdata.write_mail_free )
	{
	    quill = find_quill( ch );
	    if ( !quill )
	    {
		send_to_char("You need a quill to write a note.\n\r", ch);
		return;
	    }
	    if ( quill->value[0] < 1 )
	    {
		send_to_char("Your quill is dry.\n\r", ch);
		return;
	    }
	}
	if (!arg_passed || arg_passed[0] == '\0')
	{
	    send_to_char("Please specify an addressee.\n\r", ch);
	    return;
	}
	if ( ( paper = get_eq_char(ch, WEAR_HOLD) ) == NULL
	||     paper->item_type != ITEM_PAPER )
	{
	    if(get_trust(ch) < sysdata.write_mail_free )
	    {
		send_to_char("You need to be holding a fresh piece of parchment to write a note.\n\r", ch);
		return;
	    }
	    paper = create_object( get_obj_index(OBJ_VNUM_NOTE), 0 );
    	    if ((tmpobj = get_eq_char(ch, WEAR_HOLD)) != NULL)
		unequip_char(ch, tmpobj);
	    paper = obj_to_char(paper, ch);
	    equip_char(ch, paper, WEAR_HOLD);
	    act(AT_MAGIC, "A piece of parchment magically appears in $n's hands!",
	      ch, NULL, NULL, TO_CANSEE);
	    act(AT_MAGIC, "A piece of parchment appears in your hands.",
      	      ch, NULL, NULL, TO_CHAR);
	}

	if (paper->value[2] > 1)
	{
	    send_to_char("You cannot modify this note.\n\r",ch);
	    return;
	}

        arg_passed[0] = UPPER(arg_passed[0]);

#ifdef USE_IMC
	if (strchr(arg_passed, '@')!=NULL)
	{
	    if (get_trust(ch) < sysdata.imc_mail_level)
	    {
		ch_printf(ch, "You need to be at least level %d to send "
		    "notes to other muds.\n\r", sysdata.imc_mail_level);
		return;
	    }
	    imc = TRUE;
	}
#endif
	
        sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower(arg_passed[0]),
                 capitalize( arg_passed ) );
 
#ifdef USE_IMC
	if ( !IS_MAIL || imc || stat( fname, &fst ) != -1 || !str_cmp(arg_passed, "all") )
#else
	if ( !IS_MAIL || stat( fname, &fst ) != -1 || !str_cmp(arg_passed, "all") )
#endif
	{                                       
	    paper->value[2] = 1;
	    ed = SetOExtra(paper, "_to_");
	    STRFREE( ed->description );
	    ed->description = STRALLOC( arg_passed );
	    send_to_char("Ok.\n\r",ch);
	    return;
        }
	else
	{
	    send_to_char("No player exists by that name.\n\r",ch);
	    return;
	}

    }

    if ( !str_cmp( arg, "show" ) )
    {
	char *subject, *to_list, *text;

	if ( ( paper = get_eq_char(ch, WEAR_HOLD) ) == NULL
	||     paper->item_type != ITEM_PAPER )
	{
	    send_to_char("You are not holding a note.\n\r", ch);
	    return;
	}

	if ( (subject = get_extra_descr( "_subject_", paper->first_extradesc )) == NULL )
	  subject = "(no subject)";
	if ( (to_list = get_extra_descr( "_to_", paper->first_extradesc )) == NULL )
	  to_list = "(nobody)";
	sprintf( buf, "%s: %s\n\rTo: %s\n\r",
	    ch->name,
	    subject,
	    to_list );
	send_to_char( buf, ch );
	if ( (text = get_extra_descr( "_text_", paper->first_extradesc )) == NULL )
	  text = "The note is blank.\n\r";
	send_to_char( text, ch );
	return;
    }

    if ( !str_cmp( arg, "post" ) )
    {
	char *strtime, *to, *subj, *text/*, *np = NULL*/;
#ifdef USE_IMC
	bool imc = FALSE;
#endif

	if ( ( paper = get_eq_char(ch, WEAR_HOLD) ) == NULL
	||     paper->item_type != ITEM_PAPER )
	{
	    send_to_char("You are not holding a note.\n\r", ch);
	    return;
	}
	
	if ( paper->value[0] == 0 )
	{
	    send_to_char("There is nothing written on this note.\n\r", ch);
	    return;
	}

	if ( paper->value[1] == 0 )
	{
	    send_to_char("This note has no subject.\n\r", ch);
	    return;
	}

	if (paper->value[2] == 0)
	{
	    send_to_char("This note is addressed to no one!\n\r", ch);
	    return;
	}

	strtime				= ctime( &current_time );
	strtime[strlen(strtime)-1]	= '\0';
	
	/* handle IMC notes */
	to = get_extra_descr( "_to_", paper->first_extradesc );
	subj = get_extra_descr( "_subject_", paper->first_extradesc );
	text = get_extra_descr( "_text_", paper->first_extradesc );
	
#ifdef USE_IMC
	if (to && strchr(to, '@')!=NULL)
	{
	    if ( !subj || !*subj )
	    {
		send_to_char( "You must specify a subject for IMC mail.\n\r", ch );
		return;
	    }
	    if ( !text || !*text )
	    {
		send_to_char( "You must have text in IMC mail.\n\r", ch );
		return;
	    }
	    
	    imc = TRUE;
	}
#endif
	board = find_board( ch );

	if ( !board )
	{
	    send_to_char( "There is no bulletin board here to post your note on.\n\r", ch );
	    return;
	}
#ifdef USE_IMC
	if ( (imc && board->board_obj != sysdata.imc_mail_vnum) ||
	    (!imc && board->board_obj == sysdata.imc_mail_vnum) )
	{
	    send_to_char( "You can only post IMC mail on the IMC board.\n\r", ch );
	    return;
	}
#endif
	if ( !can_post( ch, board ) ) 
	{
	    send_to_char( "A magical force prevents you from posting your note here...\n\r", ch );
	    return;
	}

	if ( board->num_posts >= board->max_posts )
	{
	    send_to_char( "There is no room on this board to post your note.\n\r", ch );
	    return;
	}

	if ( board->opostmessg )
		act ( AT_ACTION, board->opostmessg, ch, NULL, NULL, TO_CANSEE );
	else
		act( AT_ACTION, "$n posts a note.", ch, NULL, NULL, TO_CANSEE );

	CREATE( pnote, NOTE_DATA, 1 );
	pnote->date			= STRALLOC( strtime );

	pnote->to_list = to ? STRALLOC( to ) : STRALLOC( "all" );
	pnote->text = text ? STRALLOC( text ) : STRALLOC( "" );
	pnote->subject = subj ? STRALLOC( subj ) : STRALLOC( "" );
	pnote->sender  = QUICKLINK( ch->name );
        pnote->voting      = 0;
        pnote->yesvotes    = str_dup( "" );
        pnote->novotes     = str_dup( "" );
        pnote->abstentions = str_dup( "" );
        pnote->no_remove   = 0;

#ifdef USE_IMC
	if ( imc )
	    imc_post_mail(ch, pnote->sender, pnote->to_list, pnote->date,
	        pnote->subject, pnote->text);
#endif

	LINK( pnote, board->first_note, board->last_note, next, prev );
	board->num_posts++;
        write_board( board );
	if ( board->postmessg )
		act( AT_ACTION, board->postmessg, ch, NULL, NULL, TO_CHAR );	
	else
		send_to_char( "You post your note on the board.\n\r", ch );
	extract_obj( paper );
	return;
    }

    if ( !str_cmp( arg, "reply" ) )
    {

        board = find_board( ch );
        if ( !board )
        {
            send_to_char( "There is no bulletin board here.\n\r", ch );
            return;
        }
        if ( !can_read( ch, board ) )
        {
            send_to_char( "You cannot read the notes on this board to reply to.\n\r", ch );
            return;
        }

        if ( is_number( arg_passed ) )
        {
                anum = atoi( arg_passed );
        }
        else
        {
                send_to_char( "Reply to which number?\n\r", ch );
        return;
        }

        if (!IS_MAIL)
        {
        vnum = 1;
        for ( pnote = board->first_note; pnote && vnum < anum; pnote = pnote->next )
                vnum++;
        }
        else
        {
            vnum = 0;
            for ( pnote = board->first_note; pnote && vnum < anum; pnote = pnote->next )
                        if (is_note_to(ch, pnote) || get_trust(ch) >= sysdata.read_all_mail)
                        {
                            vnum++;
                            if ( vnum == anum ) break;
                        }
        }

        if ( !pnote )
        {
            send_to_char( "No such note.\n\r", ch );
            return;
        }

        if ( ch->substate == SUB_RESTRICTED )
        {
            send_to_char( "You cannot write a note from within another command.\n\r", ch );
            return;
        }
        if (get_trust (ch) < sysdata.write_mail_free)
        {
            quill = find_quill( ch );
            if (!quill)
            {
                send_to_char("You need a quill to write a note.\n\r", ch);
                return;
            }
            if ( quill->value[0] < 1 )
            {
                send_to_char("Your quill is dry.\n\r", ch);
                return;
            }
        }
        if ( ( paper = get_eq_char(ch, WEAR_HOLD) ) == NULL
        || paper->item_type != ITEM_PAPER )
        {
            if (get_trust(ch) < sysdata.write_mail_free )
            {
                send_to_char("You need to be holding a fresh piece of parchment to write a note.\n\r", ch);
                return;
            }
            paper = create_object( get_obj_index(OBJ_VNUM_NOTE), 0 );
            if ((tmpobj = get_eq_char(ch, WEAR_HOLD)) != NULL)
              unequip_char(ch, tmpobj);
            paper = obj_to_char(paper, ch);
            equip_char(ch, paper, WEAR_HOLD);
            act(AT_MAGIC, "A piece of parchment magically appears in $n's hands!",
                ch, NULL, NULL, TO_CANSEE);
            act(AT_MAGIC, "A piece of parchment appears in your hands.",
                ch, NULL, NULL, TO_CHAR);
        }
        if (paper->value[0] < 2 )
        {
            paper->value[0] = 1;
                paper->value[1] = 1;
                paper->value[2] = 1;
            ed = SetOExtra(paper, "_text_");
            ch->substate = SUB_WRITING_NOTE;
            ch->dest_buf = ed;
            if ( get_trust(ch) < sysdata.write_mail_free )
                --quill->value[0];
            start_editing( ch, ed->description );
            ed = SetOExtra( paper, "_to_" );
            STRFREE( ed->description );
            ed->description = QUICKLINK( pnote->sender );
                sprintf( notebuf, "Re: %s", pnote->subject );
            ed = SetOExtra(paper, "_subject_");
            STRFREE( ed->description );
            ed->description = STRALLOC( notebuf );
            return;
        }
        else
        {
            send_to_char("You cannot modify this note.\n\r", ch);
            return;
        }
    }


    if ( !str_cmp( arg, "remove" )
    ||   !str_cmp( arg, "take" )
    ||   !str_cmp( arg, "copy" ) )
    {
	char take;

	board = find_board( ch );
	if ( !board )
	{
	    send_to_char( "There is no board here to take a note from!\n\r", ch );
	    return;
	}
	if ( !str_cmp( arg, "take" ) )
	  take = 1;
	else if ( !str_cmp( arg, "copy" ) )
	{
	  if ( !IS_IMMORTAL(ch) )
	  {
	    send_to_char( "Huh?  Type 'help note' for usage.\n\r", ch );
	    return;
	  }
	  take = 2;
	}
	else
	  take = 0;

	if ( !is_number( arg_passed ) )
	{
	    send_to_char( "Note remove which number?\n\r", ch );
	    return;
	}

	if ( !can_read( ch, board ) ) 
	{
	    send_to_char( "You can't make any sense of what's posted here, let alone remove anything!\n\r", ch );
	    return;
	}

	anum = atoi( arg_passed );
	vnum = 0;
	for ( pnote = board->first_note; pnote; pnote = pnote->next )
	{
	    if (IS_MAIL && ((is_note_to(ch, pnote)) 
	    ||  get_trust(ch) >= sysdata.take_others_mail))
	       vnum++;
	    else if (!IS_MAIL)
               vnum++;
	    if ( ( is_note_to( ch, pnote )
	    ||	    can_remove (ch, board)) 
	    &&   ( vnum == anum ) )
	    {
		if ( (is_name("all", pnote->to_list))
		&&   (get_trust( ch ) < sysdata.take_others_mail)
		&&   (take == 1) 
		&&   str_cmp(  ch->name, pnote->sender ) )
		{
		    send_to_char("Notes addressed to 'all' can only be taken by the author.\n\r", ch);
		    return;
		}

	        if ( pnote->no_remove != 0 && (take==1 || take==0) &&
			ch->level < pnote->no_remove ) {
			if ( take == 0 )
				send_to_char("You can't remove this note.\n\r",
					ch);
			else
				send_to_char("You can't take this note.\n\r",
					ch);
			return;
		}
 		if ( take != 0 )
		{
		    if ( ch->gold < 50 && get_trust(ch) < sysdata.read_mail_free )
		    {
			if ( take == 1 )
			  send_to_char("It costs 50 coins to take your mail.\n\r", ch);
			else
			  send_to_char("It costs 50 coins to copy your mail.\n\r", ch);
			return;
		    }
		    if ( get_trust(ch) < sysdata.read_mail_free )
		      ch->gold -= 50;
		    paper = create_object( get_obj_index(OBJ_VNUM_NOTE), 0 );
		    ed = SetOExtra( paper, "_sender_" );
		    STRFREE( ed->description );
		    ed->description = QUICKLINK(pnote->sender);
		    ed = SetOExtra( paper, "_text_" );
		    STRFREE( ed->description );
		    ed->description = QUICKLINK(pnote->text);
		    ed = SetOExtra( paper, "_to_" );
		    STRFREE( ed->description );
		    ed->description = QUICKLINK( pnote->to_list );
		    ed = SetOExtra( paper, "_subject_" );
		    STRFREE( ed->description );
		    ed->description = QUICKLINK( pnote->subject );
		    ed = SetOExtra( paper, "_date_" );
		    STRFREE( ed->description );
		    ed->description = QUICKLINK( pnote->date );
		    ed = SetOExtra( paper, "note" );
		    STRFREE( ed->description );
		    sprintf(notebuf, "From: ");
		    strcat(notebuf, pnote->sender);		 
		    strcat(notebuf, "\n\rTo: ");
		    strcat(notebuf, pnote->to_list);
		    strcat(notebuf, "\n\rSubject: ");
		    strcat(notebuf, pnote->subject);
		    strcat(notebuf, "\n\r\n\r");
		    strcat(notebuf, pnote->text);
		    strcat(notebuf, "\n\r");
		    ed->description = STRALLOC(notebuf);
		    paper->value[0] = 2;
		    paper->value[1] = 2;
	 	    paper->value[2] = 2;
            sprintf(short_desc_buf, "a note from %s to %s: %s",
                pnote->sender, pnote->to_list, pnote->subject);
		    STRFREE(paper->short_descr);
		    paper->short_descr = STRALLOC(short_desc_buf);
		    sprintf(long_desc_buf, "A note from %s to %s lies on the ground.",
			pnote->sender, pnote->to_list);
		    STRFREE(paper->description);
		    paper->description = STRALLOC(long_desc_buf);
		    sprintf(keyword_buf, "note parchment paper %s", 
			pnote->to_list);
		    STRFREE(paper->name);
		    paper->name = STRALLOC(keyword_buf);
		}
		if ( take != 2 )
		    note_remove( ch, board, pnote );
		send_to_char( "Ok.\n\r", ch );
		if ( take == 1 )
		{
		    if ( board->otakemessg )
		    	act( AT_ACTION, board->otakemessg, ch, NULL, NULL, TO_CANSEE );
		    else
		    	act( AT_ACTION, "$n takes a note.", ch, NULL, NULL, TO_CANSEE );
		    obj_to_char(paper, ch);
		}
		else if ( take == 2 )
		{
		    if ( board->ocopymessg )
		       act( AT_ACTION, board->ocopymessg, ch, NULL, NULL, TO_CANSEE );
		    else
		       act( AT_ACTION, "$n copies a note.", ch, NULL, NULL, TO_CANSEE );
		    obj_to_char(paper, ch);
		}
		else
		{
		   if ( board->oremovemessg )
		      act( AT_ACTION, board->oremovemessg, ch, NULL, NULL, TO_CANSEE );
		   else
		      act( AT_ACTION, "$n removes a note.", ch, NULL, NULL, TO_CANSEE );
	 	}
		return;
	    }
	}

	send_to_char( "No such note.\n\r", ch );
	return;
    }

    send_to_char( "Huh?  Type 'help note' for usage.\n\r", ch );
    return;
}



BOARD_DATA *read_board( char *boardfile, FILE *fp )
{
    BOARD_DATA *board;
    char *word;
    char  buf[MAX_STRING_LENGTH];
    bool fMatch;
    char letter;

	do
	{
	    letter = getc( fp );
	    if ( feof(fp) )
	    {
		fclose( fp );
		return NULL;
	    }
	}
	while ( isspace(letter) );
	ungetc( letter, fp );

	CREATE( board, BOARD_DATA, 1 );
	/* Setup pointers --Shaddai */
	board->otakemessg = NULL;
	board->opostmessg = NULL;
	board->oremovemessg = NULL;
	board->olistmessg = NULL;
	board->ocopymessg = NULL;
	board->oreadmessg = NULL;
	board->postmessg = NULL;

#ifdef KEY
#undef KEY
#endif
#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}


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
        case 'B':
                KEY( "Ballot_level",    board->min_ballot_level,fread_number( fp ) );                                                           
	case 'E':
	    KEY( "Extra_readers",	board->extra_readers,	fread_string_nohash( fp ) );
            KEY( "Extra_removers",       board->extra_removers,   fread_string_nohash( fp ) );
        KEY( "Extra_ballots",       board->extra_ballots,   fread_string_nohash( fp ) ); 
            if ( !str_cmp( word, "End" ) )  
            {
 	      board->num_posts	= 0;
	      board->first_note	= NULL;
	      board->last_note	= NULL;
	      board->next	= NULL;
	      board->prev	= NULL;
              if ( !board->read_group )
                board->read_group    = str_dup( "" );
              if ( !board->post_group )
                board->post_group    = str_dup( "" );
              if ( !board->extra_readers )
                board->extra_readers = str_dup( "" );
              if ( !board->extra_removers )
                board->extra_removers = str_dup( "" );
              if ( !board->extra_ballots )
                board->extra_ballots = str_dup( "" );                                                                                           
              return board;
            }
	case 'F':
	    KEY( "Filename",	board->note_file,	fread_string_nohash( fp ) );
	case 'M':
	    KEY( "Min_read_level",	board->min_read_level,	fread_number( fp ) );
	    KEY( "Min_post_level",	board->min_post_level,	fread_number( fp ) );
	    KEY( "Min_remove_level",	board->min_remove_level,fread_number( fp ) );
	    KEY( "Max_posts",		board->max_posts,	fread_number( fp ) );
	case 'O':
	    KEY( "OTakemessg",  board->otakemessg,	fread_string_nohash(fp));
	    KEY( "OCopymessg",  board->ocopymessg,	fread_string_nohash(fp));
	    KEY( "OReadmessg",  board->oreadmessg,	fread_string_nohash(fp));
	    KEY( "ORemovemessg",  board->oremovemessg,	fread_string_nohash(fp));
	    KEY( "OListmessg",  board->olistmessg,	fread_string_nohash(fp));
	    KEY( "OPostmessg",  board->opostmessg,	fread_string_nohash(fp));
	case 'P':
	    KEY( "Post_group",	board->post_group,	fread_string_nohash( fp ) );
	    KEY( "Postmessg",  board->postmessg,	fread_string_nohash(fp));
	case 'R':
	    KEY( "Read_group",	board->read_group,	fread_string_nohash( fp ) );
	case 'T':
	    KEY( "Type",	board->type,		fread_number( fp ) );
	case 'V':
	    KEY( "Vnum",	board->board_obj,	fread_number( fp ) );
        }
	if ( !fMatch )
	{
	    sprintf( buf, "read_board: no match: %s", word );
	    bug( buf, 0 );
	}
    }

  return board;
}

NOTE_DATA *read_note( char *notefile, FILE *fp )
{
    NOTE_DATA *pnote;
    char *word;

    for ( ; ; )
    {
	char letter;

	do
	{
	    letter = getc( fp );
	    if ( feof(fp) )
	    {
		fclose( fp );
		return NULL;
	    }
	}
	while ( isspace(letter) );
	ungetc( letter, fp );

	CREATE( pnote, NOTE_DATA, 1 );
	pnote->no_remove = 0;

	if ( str_cmp( fread_word( fp ), "sender" ) )
	    break;
	pnote->sender	= fread_string( fp );

	if ( str_cmp( fread_word( fp ), "date" ) )
	    break;
	pnote->date	= fread_string( fp );

	if ( str_cmp( fread_word( fp ), "to" ) )
	    break;
	pnote->to_list	= fread_string( fp );

	if ( str_cmp( fread_word( fp ), "subject" ) )
	    break;
	pnote->subject	= fread_string( fp );

        word = fread_word( fp );
        if ( !str_cmp( word, "voting" ) )
        {
          pnote->voting = fread_number( fp );

	  if ( str_cmp( fread_word( fp ), "yesvotes" ) )
	    break;
	  pnote->yesvotes	= fread_string_nohash( fp );

	  if ( str_cmp( fread_word( fp ), "novotes" ) )
	    break;
	  pnote->novotes	= fread_string_nohash( fp );

	  if ( str_cmp( fread_word( fp ), "abstentions" ) )
	    break;
	  pnote->abstentions	= fread_string_nohash( fp );

          word = fread_word( fp );

                if ( !str_cmp( word, "yestally" ) )
                {
                pnote->yestally = fread_number( fp );
 
                if ( str_cmp( fread_word( fp ), "notally" ) )
                        break;
                pnote->notally          = fread_number( fp );                                                                                   

                if ( str_cmp( fread_word( fp ), "abstaintally" ) )
                        break;
                pnote->abstaintally     = fread_number( fp );                                                                                   

                word = fread_word( fp );
                }                                                                                                                               

        }
        if ( !str_cmp( word, "Noremove") )
        {
              pnote->no_remove = fread_number( fp );
              word = fread_word( fp );
        }

 	if ( str_cmp( word, "text" ) )
           break;
        pnote->text	= fread_string( fp );

        if ( !pnote->yesvotes )    pnote->yesvotes	= str_dup( "" );
        if ( !pnote->novotes )     pnote->novotes	= str_dup( "" );
        if ( !pnote->abstentions ) pnote->abstentions	= str_dup( "" );
	pnote->next		= NULL;
	pnote->prev		= NULL;
	return pnote;
    }
    bug ( "read_note: bad key word." );
    exit( 1 );
}

/*
 * Load boards file.
 */
void load_boards( void )
{
    FILE	*board_fp;
    FILE	*note_fp;
    BOARD_DATA	*board;
    NOTE_DATA	*pnote;
    char	boardfile[256];
    char	notefile[256];

    first_board	= NULL;
    last_board	= NULL;

    sprintf( boardfile, "%s%s", BOARD_DIR, BOARD_FILE );
    if ( ( board_fp = fopen( boardfile, "r" ) ) == NULL )
	return;

    while ( (board = read_board( boardfile, board_fp )) != NULL )
    {
	LINK( board, first_board, last_board, next, prev );
	sprintf( notefile, "%s%s", BOARD_DIR, board->note_file );
	log_string( notefile );
	if ( ( note_fp = fopen( notefile, "r" ) ) != NULL )
	{
	    while ( (pnote = read_note( notefile, note_fp )) != NULL )
	    {
		LINK( pnote, board->first_note, board->last_note, next, prev );
		board->num_posts++;
	    }
	}
    }
    return;
}


void do_makeboard( CHAR_DATA *ch, char *argument )
{
    BOARD_DATA *board;

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "Usage: makeboard <filename>\n\r", ch );
	return;
    }

    smash_tilde( argument );

    CREATE( board, BOARD_DATA, 1 );

    LINK( board, first_board, last_board, next, prev );
    board->note_file	   = str_dup( strlower( argument ) );
    board->read_group      = str_dup( "" );
    board->post_group      = str_dup( "" );
    board->extra_readers   = str_dup( "" );
    board->extra_removers  = str_dup( "" );
    board->extra_ballots  = str_dup( "" );
    board->min_ballot_level = 51; 
}

void do_bset( CHAR_DATA *ch, char *argument )
{
    BOARD_DATA *board;
    bool found;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int value;
    
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    set_char_color( AT_NOTE, ch );
    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Usage: bset <board filename> <field> value\n\r", ch );
	send_to_char( "\n\rField being one of:\n\r", ch );
	send_to_char( "  ovnum read post remove maxpost filename type\n\r", ch );
	send_to_char( "  read_group post_group extra_readers extra_removers extra_ballots\n\r", ch );
	send_to_char ("The following will affect how an action is sent:\n\r", ch );
	send_to_char ("  oremove otake olist oread ocopy opost postmessg\n\r", ch );
	return;
    }

    value = atoi( argument );
    found = FALSE;
    for ( board = first_board; board; board = board->next )
	if ( !str_cmp( arg1, board->note_file ) )
	{
	   found = TRUE;
	   break;
	}
    if ( !found )
    {
	send_to_char( "Board not found.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "ovnum" ) )
    {
	if ( !get_obj_index(value) )
	{
	    send_to_char( "No such object.\n\r", ch );
	    return;
	}
	board->board_obj = value;
	write_boards_txt( );
	send_to_char( "Done.  (board's object vnum set)\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "read" ) )
    {
	if ( value < 0 || value > MAX_LEVEL )
	{
	    send_to_char( "Value outside valid character level range.\n\r", ch );
	    return;
	}
	board->min_read_level = value;
	write_boards_txt( );
	send_to_char( "Done.  (minimum reading level set)\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "read_group" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	    send_to_char( "No reading group specified.\n\r", ch );
	    return;
	}
	DISPOSE( board->read_group );
        if ( !str_cmp( argument, "none" ) )
	  board->read_group = str_dup( "" );
        else
	  board->read_group = str_dup( argument );
	write_boards_txt( );
	send_to_char( "Done.  (reading group set)\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "post_group" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	    send_to_char( "No posting group specified.\n\r", ch );
	    return;
	}
	DISPOSE( board->post_group );
        if ( !str_cmp( argument, "none" ) )
	  board->post_group = str_dup( "" );
        else
	  board->post_group = str_dup( argument );
	write_boards_txt( );
	send_to_char( "Done.  (posting group set)\n\r", ch );
	return;
    }
   if ( !str_cmp( arg2, "postmessg" ) )
   {
        if ( !argument || argument[0] == '\0' )
        {   
            send_to_char( "No message specified.\n\r", ch );
            return;
        }
	if ( board->postmessg )
          	DISPOSE( board->postmessg );
        if ( !str_cmp( argument, "none" ) )
		board->postmessg = NULL;
        else
	{
            sprintf( buf, "%s", argument );
            board->postmessg = str_dup( buf ); 
	}
        write_boards_txt( );
	send_to_char("Done.\n\r", ch );
	return;
   }
   if ( !str_cmp( arg2, "opost" ) )
   {
        if ( !argument || argument[0] == '\0' )
        {   
            send_to_char( "No message specified.\n\r", ch );
            return;
        }
	if ( board->opostmessg )
           DISPOSE( board->opostmessg );
        if ( !str_cmp( argument, "none" ) )
		board->opostmessg = NULL;
        else
	{
            sprintf( buf, "%s", argument );
            board->opostmessg = str_dup( buf ); 
	}
        write_boards_txt( );
	send_to_char("Done.\n\r", ch );
	return;
   }
   if ( !str_cmp( arg2, "oremove" ) )
   {
        if ( !argument || argument[0] == '\0' )
        {   
            send_to_char( "No message specified.\n\r", ch );
            return;
        }
	if ( board->oremovemessg )
           DISPOSE( board->oremovemessg );
        if ( !str_cmp( argument, "none" ) )
		board->oremovemessg = NULL;
        else
	{
            sprintf( buf, "%s", argument );
            board->oremovemessg = str_dup( buf ); 
	}
        write_boards_txt( );
	send_to_char("Done.\n\r", ch );
	return;
   }
   if ( !str_cmp( arg2, "otake" ) )
   {
        if ( !argument || argument[0] == '\0' )
        {   
            send_to_char( "No message specified.\n\r", ch );
            return;
        }
	if ( board->otakemessg )
           DISPOSE( board->otakemessg );
        if ( !str_cmp( argument, "none" ) )
		board->otakemessg = NULL;
        else
	{
            sprintf( buf, "%s", argument );
            board->otakemessg = str_dup( buf ); 
	}
        write_boards_txt( );
	send_to_char("Done.\n\r", ch );
	return;
   }
   if ( !str_cmp( arg2, "ocopy" ) )
   {
        if ( !argument || argument[0] == '\0' )
        {   
            send_to_char( "No message specified.\n\r", ch );
            return;
        }
	if ( board->ocopymessg )
           DISPOSE( board->ocopymessg );
        if ( !str_cmp( argument, "none" ) )
		board->ocopymessg = NULL;
        else
	{
            sprintf( buf, "%s", argument );
            board->ocopymessg = str_dup( buf ); 
	}
        write_boards_txt( );
	send_to_char("Done.\n\r", ch );
	return;
   }
   if ( !str_cmp( arg2, "oread" ) )
   {
	if ( !argument || argument[0] == '\0' )
	{
	    send_to_char( "No message sepcified.\n\r", ch );
	    return;
	}
	if ( board->oreadmessg )
	   DISPOSE( board->oreadmessg );
        if ( !str_cmp( argument, "none" ) )
                board->oreadmessg = NULL;
        else
        {
            sprintf( buf, "%s", argument );
            board->oreadmessg = str_dup( buf );
        }
        write_boards_txt( );
        send_to_char("Done.\n\r", ch );
        return;
   }
   if ( !str_cmp( arg2, "olist" ) )
   {
        if ( !argument || argument[0] == '\0' )
        {   
            send_to_char( "No message specified.\n\r", ch );
            return;
        }
	if ( board->olistmessg )
           DISPOSE( board->olistmessg );
        if ( !str_cmp( argument, "none" ) )
		board->olistmessg = NULL;
        else
	{
            sprintf( buf, "%s", argument );
            board->olistmessg = str_dup( buf ); 
	}
        write_boards_txt( );
	send_to_char("Done.\n\r", ch );
	return;
   }
   if ( !str_cmp( arg2, "extra_removers" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {   
            send_to_char( "No names specified.\n\r", ch );
            return;
        }
        if ( !str_cmp( argument, "none" ) )
            buf[0] = '\0';
        else
            sprintf( buf, "%s %s", board->extra_removers, argument );
        DISPOSE( board->extra_removers );
        board->extra_removers = str_dup( buf ); 
        write_boards_txt( );
        send_to_char( "Done.  (extra removers set)\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "extra_readers" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	    send_to_char( "No names specified.\n\r", ch );
	    return;
	}
	if ( !str_cmp( argument, "none" ) )
	    buf[0] = '\0';
	else
	    sprintf( buf, "%s %s", board->extra_readers, argument );        
	DISPOSE( board->extra_readers );
	board->extra_readers = str_dup( buf );
	write_boards_txt( );
	send_to_char( "Done.  (extra readers set)\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "filename" ) )
    {
	if ( !argument || argument[0] == '\0' )
	{
	    send_to_char( "No filename specified.\n\r", ch );
	    return;
	}
	DISPOSE( board->note_file );
	board->note_file = str_dup( argument );
	write_boards_txt( );
	send_to_char( "Done.  (board's filename set)\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "post" ) )
    {
	if ( value < 0 || value > MAX_LEVEL )
	{
	  send_to_char( "Value outside valid character level range.\n\r", ch );
	  return;
	}
	board->min_post_level = value;
	write_boards_txt( );
	send_to_char( "Done.  (minimum posting level set)\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "remove" ) )
    {
	if ( value < 0 || value > MAX_LEVEL )
	{
	  send_to_char( "Value outside valid character level range.\n\r", ch );
	  return;
	}
	board->min_remove_level = value;
	write_boards_txt( );
	send_to_char( "Done.  (minimum remove level set)\n\r", ch );
	return;
    }

     if ( !str_cmp( arg2, "ballot" ) )
     {
        if ( value < 0 || value > MAX_LEVEL )
        {
          send_to_char( "Value outside valid character level range.\n\r", ch );
          return;
        }
        board->min_ballot_level = value;
        write_boards_txt( );
        send_to_char( "Done.  (minimum ballot read level set)\n\r", ch );
        return;
     }                                                                                                                                          

   if ( !str_cmp( arg2, "extra_ballots" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "No names specified.\n\r", ch );
            return;
        }
        if ( !str_cmp( argument, "none" ) )
            buf[0] = '\0';
        else
            sprintf( buf, "%s %s", board->extra_ballots, argument );
        DISPOSE( board->extra_ballots );
        board->extra_ballots = str_dup( buf );
        write_boards_txt( );
        send_to_char( "Done.  (extra ballot readers set)\n\r", ch );
        return;
    }                                                                                                                                           

    if ( !str_cmp( arg2, "maxpost" ) )
    {
	if ( value < 1 || value > 999 )
	{
	  send_to_char( "Value out of range.\n\r", ch );
	  return;
	}
	board->max_posts = value;
	write_boards_txt( );
	send_to_char( "Done.  (maximum number of posts set)\n\r", ch );
	return;
    }
    if ( !str_cmp( arg2, "type" ) )
    {
	if ( value < 0 || value > 1 )
	{
	  send_to_char( "Value out of range.\n\r", ch );
	  return;
	}
	board->type = value;
	write_boards_txt( );
	send_to_char( "Done.  (board's type set)\n\r", ch );
	return;
    }

    do_bset( ch, "" );
    return;
}


void do_bstat( CHAR_DATA *ch, char *argument )
{
    BOARD_DATA *board;
    bool found;
    char arg[MAX_INPUT_LENGTH];
    
    argument = one_argument( argument, arg );
    found = FALSE;
    for ( board = first_board; board; board = board->next )
      if ( !str_cmp( arg, board->note_file ) )
      {
	found = TRUE;
	break;
      }

    if ( !found )
    {
      if ( argument && argument[0] != '\0' )
      {
        send_to_char_color( "&GBoard not found.  Usage: bstat <board filename>\n\r", ch );
        return;
      }
      else
      {
        board = find_board( ch );
	if ( !board )
	{
	  send_to_char_color( "&GNo board present.  Usage: bstat <board filename>\n\r", ch );
	  return;
	}
      }
    }

    ch_printf_color( ch, "\n\r&GFilename: &W%-15.15s &GOVnum: &W%-5d &GRead: &W%-2d &GPost: &W%-2d &GRemove: &W%-2d\n\r&GMaxpost: &W%d &GBallot: &W%-2d   &GType: &W%d\n\r&GPosts:    %d\n\r", 
                board->note_file, board->board_obj,
                board->min_read_level, board->min_post_level,
                board->min_remove_level, board->max_posts,
                board->min_ballot_level,
                board->type,             board->num_posts );                                                                                    

    ch_printf_color( ch, "&GRead_group:  &W%s\n\r&GPost_group:  &W%s\n\r&GExtra_readers:  &W%s\n\r&GExtra_ballots: &W%s\n\r&GExtra_removers: &W%s\n\r",    
                board->read_group, board->post_group, board->extra_readers, board->extra_ballots, board->extra_removers );                      

    ch_printf_color( ch, "&GPost Message:    %s\n\r", board->postmessg?board->postmessg:"Default Message" );
    ch_printf_color( ch, "&GOPost Message:   %s\n\r", board->opostmessg?board->opostmessg:"Default Message" );
    ch_printf_color( ch, "&GORead Message:   %s\n\r", board->oreadmessg?board->oreadmessg:"Default Message" );
    ch_printf_color( ch, "&GORemove Message: %s\n\r", board->oremovemessg?board->oremovemessg:"Default Message" );
    ch_printf_color( ch, "&GOTake Message:   %s\n\r", board->otakemessg?board->otakemessg:"Default Message" );
    ch_printf_color( ch, "&GOList Message:   %s\n\r", board->olistmessg?board->olistmessg:"Default Message" );
    ch_printf_color( ch, "&GOCopy Message:   %s\n\r", board->ocopymessg?board->ocopymessg:"Default Message" );
    return;
}


void do_boards( CHAR_DATA *ch, char *argument )
{
    BOARD_DATA *board;

    if ( !first_board )
    {
      send_to_char_color( "T&Ghere are no boards yet.\n\r", ch );
      return;
    }
    for ( board = first_board; board; board = board->next )
      pager_printf_color( ch, "&G%-15.15s #: %5d Read: %2d Post: %2d Rmv: %2d Max: %3d Posts: &g%3d &GType: %d\n\r",
	board->note_file,	 board->board_obj,
	board->min_read_level,	 board->min_post_level,
	board->min_remove_level, board->max_posts, board->num_posts, 
	board->type);
}

void mail_count(CHAR_DATA *ch)
{
  BOARD_DATA *board;
  NOTE_DATA *note;
  int cnt_to = 0, cnt_from=0;
  
  for ( board = first_board; board; board = board->next )
    if ( board->type == BOARD_MAIL && can_read(ch, board) )
      for ( note = board->first_note; note; note = note->next )
      {
        if ( is_name(ch->name, note->to_list) )
          ++cnt_to;
	else if ( !str_cmp(ch->name, note->sender ) ) 
	  ++cnt_from;
      }
  if ( cnt_to )
    ch_printf(ch, "You have %d mail message%swaiting for you.\n\r", cnt_to, 
	(cnt_to > 1) ? "s " : " ");

  if ( cnt_from )
    ch_printf(ch, "You have %d mail message%swritten by you.\n\r", cnt_from, 
	(cnt_from > 1) ? "s " : " ");
  return;
}

/*
 * Journal command. Allows users to write notes to an object of type "journal".
 * Options are Write, Read and Size. Write and Read options require a numerical
 * argument. Option Size retrives v0 or value0 from the object, which is indicitive
 * of how many pages are in the journal.
 *
 * Forced a maximum limit of 50 pages to all journals, just incase someone slipped
 * with a value command and we ended up with an object that could store 500 pages.
 * This is added in journal write and journal size. Leart.
 */

void do_journal( CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf [MAX_STRING_LENGTH];
    EXTRA_DESCR_DATA *ed;


    OBJ_DATA *quill = NULL, *journal = NULL;

    int pages;
    int anum=0;
    char *text;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( IS_NPC(ch) )
	return;

    if ( !ch->desc )
    {
	bug( "do_journal: no descriptor", 0 );
	return;
    }
    switch( ch->substate )
    {
	default:
	  break;
	case SUB_JOURNAL_WRITE:
	  if ( ( journal = get_eq_char(ch, WEAR_HOLD) ) == NULL
	  ||     journal->item_type != ITEM_JOURNAL )
	  {
	     bug("do_journal: Player not holding journal. (Player: %s)", ch->name );
	     stop_editing( ch );
	     return;
          }
	  ed = ch->dest_buf;
	  STRFREE( ed->description );
	  ed->description = copy_buffer( ch );
	  stop_editing( ch );	   
	  return;
    }
    if ( arg1[0] == '\0' )
	    {
	    send_to_char( "Syntax: Journal <command>\n\r", ch );
		send_to_char( "\n\r",						   ch );
	    send_to_char( "Where command is one of:\n\r", ch );
	    send_to_char( "write read size\n\r", ch );
	    return;
	    }

/* 
 * Write option. Allows user to enter the buffer, adding an extra_desc to the
 * journal object called "PageX" where X is the argument associated with the write command
 */

    if ( !str_cmp( arg1, "write" ) )
    {
        if ( ( journal = get_eq_char(ch, WEAR_HOLD) ) == NULL
	  ||     journal->item_type != ITEM_JOURNAL )
	  {
	     send_to_char( "You must be holding a journal in order to write in it.\n\r", ch );
	     return;
	  }
         if ( arg2[0] == '\0' || !is_number( arg2 ) )
	  {
             send_to_char( "Syntax: Journal write <number>\n\r", ch );
	     return;
          }		
        if (get_trust (ch) < sysdata.write_mail_free)
	{
	    quill = find_quill( ch );
            if (!quill)
	    {
		send_to_char("You need a quill to write in your journal.\n\r", ch);
		return;
	    }
	    if ( quill->value[0] < 1 )
	    {
		send_to_char("Your quill is dry.\n\r", ch);
		return;
	    }
	}
 	if (journal->value[0] < 1)
	   {
		send_to_char("There are no pages in this journal. Seek an immortal for assistance.\n\r",ch);
		return;
	   }
/* Force a max value of 50 */
	if (journal->value[0] > 50)
	   {
		journal->value[0] = 50;
		bug( "do_journal: Journal size greater than 50 pages! Resetting to 50 pages. (Player: %s)", ch->name );
	   }
	set_char_color(AT_GREY, ch);
        pages = journal->value[0];
        if ( is_number( arg2 ) )
        {
        anum = atoi(arg2);
        }
	if (pages < anum)
	   {
		send_to_char("That page does not exist in this journal.\n\r", ch);
		return;
	   }
/* Making the edits turn out to be "page1" etc - just so people can't/don't type "look 1" */
	strcpy ( buf , "page");
	strcat ( buf , arg2);

        ed = SetOExtra( journal, buf );
	ch->substate = SUB_JOURNAL_WRITE;
	ch->dest_buf = ed;
	if ( get_trust(ch) < sysdata.write_mail_free )
	   --quill->value[0];
	start_editing( ch, ed->description );
	journal->value[1]++;
	return;
    }

/* Size option, returns how many pages are in the journal */

    if ( !str_cmp( arg1, "size" ) )
    {
    if ( ( journal = get_eq_char(ch, WEAR_HOLD) ) == NULL
	  ||     journal->item_type != ITEM_JOURNAL )
	  {
	     send_to_char( "You must be holding a journal in order to check it's size.\n\r", ch );
	     return;
	  }
 	if (journal->value[0] < 1)
	  {
		send_to_char("There are no pages in this journal. Seek an immortal for assistance.\n\r",ch);
	  }
	else
	  {
		if (journal->value[0] > 50)
	        {
		  	journal->value[0] = 50;
			bug( "do_journal: Journal size greater than 50 pages! Resetting to 50 pages. (Player: %s)", ch->name );
	        }
		set_char_color(AT_GREY, ch);
		pager_printf(ch, "There are %d pages in this journal.\n\r", journal->value[0]);
		return;
	   }
    }
/* Read option. Players can read the desc on the journal by typing "look page1", but I thought about putting
 * in this option anyway.
 */
    if ( !str_cmp( arg1, "read" ) )
    {
     if ( !arg2 || arg2[0] == '\0' )
     {
         send_to_char( "Syntax: Journal read <number>\n\r", ch );
         return;
     }
     if ( !is_number( arg2 ) )
     {
         send_to_char( "Syntax: Journal read <number>\n\r", ch );
         return;
     }
     if ( is_number( arg2 ) )
     {
        anum = atoi(arg2);
     }
     strcpy( buf, "page");
     strcat( buf, arg2);

        if ( ( journal = get_eq_char(ch, WEAR_HOLD) ) == NULL
        ||     journal->item_type != ITEM_JOURNAL )
        {
          send_to_char( "You must be holding a journal in order to read it.\n\r", ch );
          return;
        }
	if (journal->value[0] > 50)
	   {
		journal->value[0] = 50;
		bug( "do_journal: Journal size greater than 50 pages! Resetting to 50 pages. (Player: %s)", ch->name );
	   }
	set_char_color(AT_GREY, ch);
        pages = journal->value[0];
     if (pages < anum)
        {
          send_to_char("That page does not exist in this journal.\n\r", ch);
          return;
        }
     if ( (text = get_extra_descr( buf, journal->first_extradesc )) == NULL )
          send_to_char_color( "That journal page is blank.\n\r", ch);
     else
          send_to_char_color( text, ch);
     return;
    }

	send_to_char( "Syntax: Journal <command>\n\r", ch );
	send_to_char( "\n\r",						   ch );
	send_to_char( "Where command is one of:\n\r", ch );
	send_to_char( "write read size\n\r", ch );
	return;

}
