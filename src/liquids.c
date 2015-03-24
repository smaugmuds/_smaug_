/*****************************************************
**     _________       __			    **
**     \_   ___ \_____|__| _____  ________  ___     **
**	/    \	\/_  __ \ |/	 \/  ___/_ \/	\   **
**	\     \___|  | \/ |  | |  \___ \  / ) |  \  **
**	 \______  /__| |__|__|_|  /____ \__/__|  /  **
**	   ____\/____ _        \/ ___ \/      \/    **
**	   \______   \ |_____  __| _/___	    **
**	    |	 |  _/ |\__  \/ __ | __ \	    **
**	    |	 |   \ |_/ __ \  / | ___/_	    **
**	    |_____  /__/____  /_  /___	/	    **
**		 \/Antipode\/  \/    \/ 	    **
******************************************************
**	   Crimson Blade Codebase (CbC) 	    **
**     (c) 2000-2002 John Bellone (Noplex)	    **
**	     Coders: Noplex, Krowe		    **
**	  http://www.crimsonblade.org		    **
******************************************************
** Based on SMAUG 1.4a, by; Thoric, Altrag, Blodkai **
**  Narn, Haus, Scryn, Rennard, Swordbearer, Gorog  **
**    Grishnakh, Nivek, Tricops, and Fireblade	    **
******************************************************
** Merc 2.1 by; Michael Chastain, Michael Quan, and **
**		    Mitchell Tse		    **
******************************************************
**   DikuMUD by; Sebastian Hammer, Michael Seifert, **
**     Hans Staerfeldt, Tom Madsen and Katja Nyobe  **
*****************************************************/

/*
 * Liquidtable Replacement Sourcefile
 * by Noplex (noplex@crimsonblade.org)
 */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/* Globals */
char *	const liquid_types[] =
{
   "Normal", "Alcoholic", "Poisonous", "Blood"
};

int get_liquid_type( char *name )
{
    int x;
    
    for ( x = 0; x < LIQ_TOP; x++ )
	if ( !str_cmp( name, liquid_types[x] ) )
	  return x;
    return -1;
}

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
/*
 * How do you like this hack job? ;)			-Nopey 
 */
#ifdef MOD
#undef MOD
#endif
#define MOD( literal, field, field2, field3, field4, value )		\
				if( !str_cmp( word, literal ) ) 	\
				{					\
				    field	= value;		\
				    field2	= value;		\
				    field3	= value;		\
				    field4	= value;		\
				    fMatch = TRUE;			\
				    break;				\
				}

#ifdef DESTROYOBJ
#undef DESTROYOBJ
#endif
#define DESTROYOBJ( obj )			\
			separate_obj( obj );	\
			obj_from_char( obj );	\
			extract_obj( obj );	\

/*
=====================
Save the liquid data	-Nopey
=====================
*/
void save_liquids()
{
	FILE *fp;
	LIQUID_DATA *liq = NULL;
	char filename[256];
	
	sprintf( filename, "%sliquids.dat", SYSTEM_DIR );
	if( ( fp = fopen( filename, "w" )) == NULL )
	{
		bug( "Cannot open liquids.dat for writing." );
	}
	else
	{
		for( liq = first_liquid; liq; liq = liq->next )
		{
			fprintf( fp, "#LIQUID\n" );
			fprintf( fp, "Name	%s~\n", liq->name );
			fprintf( fp, "Shortdesc %s~\n", liq->shortdesc );
			fprintf( fp, "Color	%s~\n", liq->color );
			fprintf( fp, "Type	%d\n", liq->type );
			fprintf( fp, "Vnum	%d\n", liq->vnum );
//			fprintf( fp, "Affected	%s\n", print_bitvector(&liq->affected_by) );
			fprintf( fp, "Mod	%d %d %d %d\n", liq->mod[0], liq->mod[1], liq->mod[2], liq->mod[3] );
			fprintf( fp, "End\n\n" );
		}
		fprintf( fp, "#END\n" );
	}
	FCLOSE( fp );
	return;
}

/*
========================
Read a liquid from file -Nopey
========================
*/
void fread_liquid( LIQUID_DATA *liq, FILE *fp )
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

	/* add affects to liquids (10/4/01)	-Nopey 
	case 'A':
	    KEY( "Affected", liq->affected_by, fread_bitvector(fp) );
	    break;
	*/ /* Alchemy shit isn't ready */

	case 'C':
	    KEY( "Color", liq->color, fread_string( fp ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {	
		if( !liq->name )
		   liq->name = STRALLOC( "Not Set" );
		if( !liq->shortdesc )
		   liq->shortdesc = STRALLOC( "Not Set" );
		if( !liq->color )
		   liq->color = STRALLOC( "Not Set" );
		return;
	  }
	    break;

	case 'M':
	   MOD( "Mod", liq->mod[COND_DRUNK], liq->mod[COND_FULL], liq->mod[COND_THIRST], liq->mod[COND_BLOODTHIRST], fread_number( fp ) );
	   break;

	case 'N':
	   KEY( "Name", liq->name, fread_string( fp ) );
	   break;
	
	case 'S':
	   KEY( "Shortdesc", liq->shortdesc, fread_string( fp ) );
	   break;

	case 'T':
	   KEY( "Type", liq->type, fread_number( fp ) );
	   break;

	case 'V':
	   KEY( "Vnum", liq->vnum, fread_number( fp ) );
	   break;
	}

	if ( !fMatch )
	    bug("Fread_liquid: no match: %s", word );
    }
}

/*
==============================
Load liquiddata from dat file	-Nopey
==============================
*/
void load_liquids()
{
     char filename[256];
     LIQUID_DATA *liq;
     FILE *fp;

     first_liquid = NULL;
     last_liquid = NULL;

     sprintf( filename, "%sliquids.dat", SYSTEM_DIR );

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
		bug( "Load_liquids: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "LIQUID" ) )
	    {
		CREATE( liq, LIQUID_DATA, 1 );
		fread_liquid( liq, fp );
		LINK( liq, first_liquid, last_liquid, next, prev );
		continue;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
		break;
	    else
	    {
		bug("Load_liquids: bad section: %s.", word );
		continue;
	    }
	}
	FCLOSE( fp );
    }
    return;
}

/*
======================
Save the mixture data	-Nopey
======================
*/
void save_mixtures()
{
	FILE *fp;
	LIQUID_MIXTURE_LIST *mix = NULL;
	char filename[256];

	sprintf( filename, "%smixtures.dat", SYSTEM_DIR );
	if( ( fp = fopen( filename, "w" )) == NULL )
	{
		bug( "Cannot open mixtures.dat for writing." );
	}
	else
	{
		for( mix = first_mixture; mix; mix = mix->next )
		{
			fprintf( fp, "#MIXTURE\n" );
			fprintf( fp, "Name	   %s~\n", mix->name );
			fprintf( fp, "Into	   %d\n", mix->into );
			fprintf( fp, "With	   %d %d\n", mix->with1, mix->with2 );
			fprintf( fp, "Object	   %d\n", mix->object );
			fprintf( fp, "End\n" );
		}
	}
	FCLOSE( fp );
	return;
}

/*
=========================
Read a mixture from file	-Nopey
=========================
*/
void fread_mixture( LIQUID_MIXTURE_LIST *mix, FILE *fp )
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

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if( !mix->name )
		   mix->name = STRALLOC( "Not Set" );
		return;
	  }
	    break;

	case 'I':
	   KEY( "Into", mix->into, fread_number( fp ) );
	   break;

	case 'N':
	   KEY( "Name", mix->name, fread_string( fp ) );
	   break;

	case 'O':
	   KEY( "Object", mix->object, fread_number(fp) );
	   break;

	case 'W':
	   if( !str_cmp( word, "with" ) )
	   {
		mix->with1 = fread_number( fp );
		mix->with2 = fread_number( fp );
		fMatch = TRUE;
		break;
	   }
	   break;
	}

	if(!fMatch)
	    bug("Fread_mixture: no match: %s", word );
    }
}

/*
============================
Load mixtures from dat file	-Nopey
============================
*/
void load_mixtures()
{
     char filename[256];
     LIQUID_MIXTURE_LIST *mix;
     FILE *fp;

     first_mixture = NULL;
     last_mixture = NULL;

     sprintf( filename, "%smixtures.dat", SYSTEM_DIR );

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
		bug( "Load_mixtures: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "MIXTURE" ) )
	    {
		CREATE( mix, LIQUID_MIXTURE_LIST, 1 );
		fread_mixture( mix, fp );
		LINK( mix, first_mixture, last_mixture, next, prev );
		continue;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
		break;
	    else
	    {
		bug("Load_mixtures: bad section: %s.", word );
		continue;
	    }
	}
	FCLOSE( fp );
    }
    return;
}

/*
===============================
Create the liquid, part of olc	-Nopey
===============================
*/
void makeliquid( CHAR_DATA *ch, char *argument )
{
	LIQUID_DATA *liq = NULL;
	LIQUID_DATA *tliq = NULL;
	int vnum = 0;
	int x = 0;

	if( argument[0] == '\0' )
	{
		send_to_char( "Syntax: setliquid create <name>\n\r", ch );
		return;
	}

	for( tliq = first_liquid; tliq; tliq = tliq->next )
	{
		if( !str_cmp( argument, tliq->name ) )
		{
			send_to_char( "There is already a liquid with that name.\n\r", ch );
			return;
		}
		vnum++;
	}

	CREATE( liq, LIQUID_DATA, 1 );
	liq->name	= STRALLOC( argument );
	liq->shortdesc	= STRALLOC( "Not Set" );
	liq->color	= STRALLOC( "Not Set" );
	liq->vnum	= vnum;
	liq->type	= LIQ_NORMAL;
/*	xCLEAR_BITS( liq->affected_by ); */
	for( x = 0; x > TOP_MOD_NUM; x++ )
	liq->mod[x]	= 0;

	LINK( liq, first_liquid, last_liquid, next, prev );
	send_to_char( "Liquid created.\n\r", ch );
	return;
}

/*
==================================
Create a mixture, part of the olc	-Nopey
==================================
*/
void makemixture( CHAR_DATA *ch, char *name )
{
	LIQUID_MIXTURE_LIST *mix = NULL;
	LIQUID_MIXTURE_LIST *tmix = NULL;
	LIQUID_DATA *liq = NULL;
	bool found = FALSE;

	for( liq = first_liquid; liq; liq = liq->next )
	{
	   if( !str_cmp( name, liq->name ) )
	   {
		found = TRUE;
		break;
	   }
	}

	if( !found )
	{
	   send_to_char( "A liquid needs to be created before you can define mixtures to make it.\n\r", ch );
	   return;
	}

	if( ( tmix = get_mix_name( name ) ) != NULL )
	{
	 send_to_char( "There is already a mixture with that name.\n\r", ch );
	   return;
	}

	CREATE( mix, LIQUID_MIXTURE_LIST, 1 );
	mix->name	= STRALLOC( name );
	mix->with1	= 0;
	mix->with2	= 0;
	mix->into	= 0;
	mix->object	= FALSE;

	LINK( mix, first_mixture, last_mixture, next, prev );
	send_to_char( "Mixture created.\n\r", ch );
	return;
}

/*
===============================================
Get a vnum from the liquid index (linked list)	-Nopey
slightly modified by Samson, looks for a liquid
matching the specified vnum
===============================================
*/
LIQUID_DATA *get_liq_index( int vnum )
{
   LIQUID_DATA *liq = NULL;

   for( liq = first_liquid; liq; liq = liq->next )
   {
	if( liq->vnum == vnum )
	   return liq;
   }

   return NULL;
}

/*
=========================================
Checks against argument (name of liquid)	-Nopey
and the vnum of liquid
=========================================
*/
LIQUID_DATA *check_liquid( char *argument )
{
	LIQUID_DATA *liq = NULL;
	int vnum = 0;

	if( is_number( argument ) )
	{
		vnum = atoi( argument );

		if( ( liq = get_liq_index( vnum ) ) != NULL )
			return liq;
	}
	else
	{
		if( ( liq = get_liq_name( argument ) ) != NULL )
			return liq;
	}

	return NULL;
}	 


/*
===========================
Grabs the name of a liquid	-Nopey
===========================
*/
LIQUID_DATA *get_liq_name( char *argument )
{
   LIQUID_DATA *liq = NULL;

   for( liq = first_liquid; liq; liq = liq->next )
   {
      if( !str_cmp( argument, liq->name ) )
	   return liq;
   }

   return NULL;
}

/*
===========================
Grabs the name of a mixture	-Nopey
===========================
*/
LIQUID_MIXTURE_LIST *get_mix_name( char *argument )
{
   LIQUID_MIXTURE_LIST *mix = NULL;

   for( mix = first_mixture; mix; mix = mix->next )
   {
	if( !str_cmp( argument, mix->name ) )
	   return mix;
   }

   return NULL;
}

/*
===========================
Mixture OLC command		-Nopey
===========================
*/
void do_setmixture( CHAR_DATA *ch, char *argument )
{
   LIQUID_MIXTURE_LIST *mix = NULL;
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];

   if( IS_NPC( ch ) )
   {
	send_to_char( "NPCs cannot use this command.\n\r", ch );
	return;
   }

   if( argument[0] == '\0' )
   {
	send_to_char( "Mixtures\n\r", ch );
	send_to_char( "Syntax: setmixture save\n\r", ch );
	send_to_char( "Syntax: setmixture create <name>\n\r", ch );
	send_to_char( "Syntax: setmixture delete <name/vnum>\n\r", ch );
	send_to_char( "Syntax: setmixture <name> rename <newname>\n\r", ch );
	send_to_char( "Syntax: setmixture <name> with1 <vnum>\n\r", ch );
	send_to_char( "Syntax: setmixture <name> with2 <vnum>\n\r", ch );
	send_to_char( "Syntax: setmixture <name> into <vnum>\n\r", ch );
	send_to_char( "Syntax: setmixture <name> object <yes/no>\n\r"
		      "\t- Defaults to no\n\r"
		      "\t- With1 must be obj vnum\n\r"
		      "\t- Obj value 2 must be mixture vnum\n\r", ch );
	return;
   }

   if( !str_cmp( argument, "save" ) )
   {
	save_mixtures();
	send_to_char( "Mixtures saved.\n\r", ch );
	return;
   }

   argument = one_argument( argument, arg );

   if( !str_cmp( arg, "create" ) )
   {
	makemixture( ch, argument );
	return; 
   }

   if( !str_cmp( arg, "delete" ) )
   {
	if( ( mix = get_mix_name( argument ) ) == NULL )
	{
		send_to_char( "That is a mixture name.\n\r", ch );
		return;
	}

	UNLINK( mix, first_mixture, last_mixture, next, prev );
	STRFREE( mix->name );
	DISPOSE( mix );
	send_to_char( "Mixture deleted.\n\r", ch );
	return;
   }

   if( ( mix = get_mix_name( arg ) ) == NULL )
   {
	send_to_char( "There is no mixture with than name.\n\r", ch );
	return;
   }

   argument = one_argument( argument, arg2 );

   if( !str_cmp( arg2, "rename" ) )
   {
	STRFREE( mix->name );
	mix->name = STRALLOC( argument );
	send_to_char( "Mixture name changed.\n\r", ch );
	return;
   }

   if( !str_cmp( arg2, "object" ) )
   {
	if( !str_cmp( argument, "yes" ) )
	{
		mix->object	= TRUE;
		send_to_char( "Mixture now mixes with object.\n\r", ch );
		return;
	}
	else if( !str_cmp( argument, "no" ) )
	{
		mix->object	= FALSE;
		send_to_char( "Mixture now doesn't mix with object.\n\r", ch );
		return;
	}
   }

   if( !str_cmp( arg2, "with1" ) )
   {
	mix->with1 = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	if( mix->object == TRUE )
		send_to_char( "With1 must be object vnum.\n\r", ch );

	return;
   }

   if( !str_cmp( arg2, "with2" ) )
   {
	mix->with2 = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	return;
   }

   if( !str_cmp( arg2, "into" ) )
   {
	mix->into = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	return;
   }

   do_setmixture( ch, "" );
   return;
}

/*
===========================
Liquid OLC command		-Nopey
===========================
*/
void do_setliquid( CHAR_DATA *ch, char *argument )
{
	LIQUID_DATA *liq = NULL;
	LIQUID_DATA *tliq = NULL;
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int vnum = 0, value = -1;
	
	if( !IS_IMMORTAL(ch) || IS_NPC(ch) )
	{
		send_to_char( "Huh\n\r", ch );
		return;
	}

	if( argument[0] == '\0' )
	{
		send_to_char( "Syntax: setliquid save\n\r", ch );
		send_to_char( "Syntax: setliquid create <name>\n\r", ch );
		send_to_char( "Syntax: setliquid delete <name/vnum>\n\r", ch );
		send_to_char( "Syntax: setliquid <name/vnum> <field> <value>\n\r", ch );
		send_to_char( "Fields being one of the following:\n\r", ch );
		send_to_char( " name color shortdesc affect type\n\r", ch );
		send_to_char( " thirst drunk blood full\n\r", ch );
		return;
	}

	if( !str_cmp( argument, "save" ) )
	{
		save_liquids();
		send_to_char( "Done.\n\r", ch );
		return;
	}

	argument = one_argument( argument, arg );
	vnum = atoi( arg );

	if( arg[0] == '\0' )
	{
		send_to_char( "Syntax: setliquid create <name>\n\r", ch );
		send_to_char( "Syntax: setliquid delete <name/vnum>\n\r", ch );
		send_to_char( "Syntax: setliquid <name/vnum> <field> <value>\n\r", ch );
		send_to_char( "Fields being one of the following:\n\r", ch );
		send_to_char( " name color shortdesc affect type", ch );
		send_to_char( " thirst drunk blood", ch );
		return;
	}

	if( !str_cmp( arg, "create" ) )
	{
		makeliquid( ch, argument );
		return;
	}

	/* Guess I forgot the re-add this when I merg'ed Sam's
	 * copy.	-Nopey */
	if( !str_cmp( arg, "delete" ) )
	{
		LIQUID_MIXTURE_LIST *mix = NULL;
		bool found = FALSE;

		if( ( liq = check_liquid( argument ) ) == NULL )
		{
			send_to_char( "That is not a liquidname or liquidvnum.\n\r", ch );
			return;
		}

		/* FIX: Should also delete mixture if there is one with the same name -Nopey */
		if( ( mix = get_mix_name( argument ) ) != NULL )
		{
			UNLINK( mix, first_mixture, last_mixture, next, prev );
			STRFREE( mix->name );
			DISPOSE( mix );
		}
		UNLINK( liq, first_liquid, last_liquid, next, prev );
		STRFREE( liq->name );
		STRFREE( liq->color );
		STRFREE( liq->shortdesc );
		DISPOSE( liq );
		send_to_char( "Liquid deleted.\n\r", ch );
		if( found )
			send_to_char( "Mixture deleted.\n\r", ch );

		return;
	}

	if( ( liq = check_liquid( arg ) ) == NULL )
	{
		send_to_char( "That is not a liquid name or vnum!\n\r", ch );
		return;
	}

	argument = one_argument( argument, arg2 );

	if( arg2[0] == '\0' )
	{
		send_to_char( "Syntax: setliquid <name/vnum> <field> <value>\n\r", ch );
		send_to_char( "Fields being one of the following:\n\r", ch );
		send_to_char( " name color shortdesc affect type", ch );
		send_to_char( " thirst drunk blood", ch );
		return;
	}		

	/* name of the liquid */
	if( !str_cmp( arg2, "name" ) )
	{
		if( argument[0] == '\0' )
		{
			send_to_char( "Syntax: setliquid <name> name <desc>\n\r", ch );
			return;
		}

		if( is_number(argument) )
		{
			send_to_char( "No numbers inside the name.\n\r", ch );
			return;
		}
		STRFREE( liq->name );  
		liq->name	= STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		return;
	}

	/* used in do_drink act functions (and other functions) */
	if( !str_cmp( arg2, "shortdesc" ) )
	{
		if( argument[0] == '\0' )
		{
			send_to_char( "Syntax: setliquid <name> shortdesc <desc>\n\r", ch );
			return;
		}

		if( is_number(argument) )
		{
			send_to_char( "No numbers inside the shortdesc.\n\r", ch );
			return;
		}
		STRFREE( liq->shortdesc );  
		liq->shortdesc	= STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		return;
	}

	/* fixed types... just like the original Smaug. Easy to add more. */
	if( !str_cmp( arg2, "type" ) )
	{
		value = get_liquid_type( argument );

		if( value < 0 || value >= LIQ_TOP )
		{
		   send_to_char( "That is not a valid liquid type.\n\r", ch );
		   return;
		}

		liq->type = value;
		ch_printf( ch, "Liquid type set to %s.", argument );
		return;
	}

	/* affect olc command
	 * jacked from build.c =P */
	/*
	if( !str_cmp( arg2, "affect" ) )
	{
		if( argument[0] == '\0' )
		{
			send_to_char( "Syntax: setliquid <name> affect <flag>\n\r", ch );
			return;
		}
		while( argument[0] != '\0' )
		{
			char arg3[MAX_INPUT_LENGTH];
			int value = 0;

			argument = one_argument( argument, arg3 );
			value = get_aflag( arg3 );
			if ( value < 0 || value > MAX_BITS )
				ch_printf( ch, "Unknown affect: %s\n\r", arg3 );
			else
			{
				xTOGGLE_BIT( liq->affected_by, value );
				ch_printf( ch, "Liquid affects added/removed: %s\n\r", arg3 );
			}
		}
		return;
	}
	*/

	/* 8/13/01 - Color support for some of current Smaug. "exam" */
	/* 11/13/01 - Fixed it up a little bit */
	if( !str_cmp( arg2, "color" ) )
	{
		if( argument[0] == '\0' )
		{
			send_to_char( "Syntax: setliquid <name> color <color>\n\r", ch );
			return;
		}
		STRFREE( liq->color );
		liq->color	= STRALLOC( argument );
		send_to_char( "Done.\n\r", ch );
		return;
	}

	/* this should be RARELY used. */
	if( !str_cmp( arg2, "vnum" ) )
	{
		if( argument[0] == '\0' )
		{
			send_to_char( "Syntax: setliquid <name> vnum <number>\n\r", ch );
			return;
		}
		value = atoi( argument );

		if( ( tliq = get_liq_index( value ) ) != NULL )
		{
			send_to_char( "That's already a vnum.\n\r", ch );
			return;
		}

		liq->vnum	= atoi( argument );
		send_to_char( "Done\n\r", ch );
		return;
	}

	/* character modifiers */
	if( !str_cmp( arg2, "drunk" ) )
	{
		if( is_number(argument) )
		{
			liq->mod[COND_DRUNK] = atoi( argument );
			send_to_char( "Done.\n\r", ch );
			return;
		}
		else
		{
			send_to_char( "Drunk modifier must be a number.\n\r", ch );
			return;
		}
	}

	if( !str_cmp( arg2, "full" ) )
	{
		if( is_number(argument) )
		{
			liq->mod[COND_FULL] = atoi( argument );
			send_to_char( "Done.\n\r", ch );
			return;
		}
		else
		{
			send_to_char( "Full modifier must be a number.\n\r", ch );
			return;
		}
	}

	if( !str_cmp( arg2, "thirst" ) )
	{
		if( is_number(argument) )
		{
			liq->mod[COND_THIRST] = atoi( argument );
			send_to_char( "Done.\n\r", ch );
			return;
		}
		else
		{
			send_to_char( "Thirst modifier must be a number.\n\r", ch );
			return;
		}
	}

      if( !str_cmp( arg2, "blood" ) ) /* Blood in stock code */
	{
		if( is_number(argument) )
		{
			liq->mod[COND_BLOODTHIRST] = atoi( argument );
			send_to_char( "Done.\n\r", ch );
			return;
		}
		else
		{
			send_to_char( "Blood modifier must be a number.\n\r", ch );
			return;
		}
	}

	do_setliquid( ch, "" );
	return;
}

/*
=====================================
List the liquidtable and mixturetable	-Nopey
=====================================
*/
void do_liquids( CHAR_DATA *ch, char *argument )
{
	LIQUID_DATA *liq;
	LIQUID_MIXTURE_LIST *mix = NULL;
	bool found = FALSE;

	/* list liquid stuff, minimum crap. Vnum and other stuff */
	if( argument[0] == '\0' )
	{
		pager_printf_color( ch, "&GLiquid Table\n\r" );
		pager_printf_color( ch, "&G[&gVnum&G]&g--&G[&gType&G]&g--&G[&gThr&G/&gFul&G/&gDru&G/&gBlo&G]&g------&G[&gName&G]&g-------------------&G[&gShortdesc&G]&g\n\r" );	
	
		for( liq = first_liquid; liq; liq = liq->next )
		{
			if( liq->name )
			{
				found = TRUE;
				pager_printf_color( ch, "&g%-8d %-7d %-3d %-3d %-3d %-3d %-24s %-10s\n\r", liq->vnum, liq->type, liq->mod[COND_THIRST], liq->mod[COND_FULL], liq->mod[COND_DRUNK], liq->mod[3], liq->name, liq->shortdesc );
			}
		}
	
		/* this would be REALLY bad... hopefully always keep a backup of original liquids */
		if( !found )
			pager_printf_color( ch, "&RThere are currently no liquids loaded.&g\n\r" );
		else
		{
		pager_printf_color( ch, "&GType 'liquids <name/vnum>' to gain more information.\n\r" );
			pager_printf_color( ch, "&GType 'liquids mixtures' to list the mixture table.\n\r" );
		}
		
		return;
	}

	/* list the mixture table 9/6/01 */
	if( !str_cmp( argument, "mixtures" ) || !str_cmp( argument, "mixes" ) || !str_cmp( argument, "mixturetable" ) )
	{
		pager_printf_color( ch, "&GMixture Table\n\r" );
		pager_printf_color( ch, "&G[&gWith&G]&g and &G[&gWith&G]&g---&G[&gInto&G]&g---&G[&gName&G]&g\n\r" );
		
		found = FALSE;
		for( mix = first_mixture; mix; mix = mix->next )
		{
			if( mix->name )
			{
				found = TRUE;
				pager_printf_color( ch, " &G%-9d %-8d %-8d %-10s\n\r", mix->with1, mix->with2, mix->into, mix->name );
			}
		}

		if( !found )
			pager_printf_color( ch, "&RThere are currently no mixtures loaded.\n\r" );
		else
		{
			pager_printf_color( ch, "&GType 'liquids <name/vnum>' to gain more information.\n\r" );
			pager_printf_color( ch, "&GType 'liquids' to list the liquid table.\n\r" );
		}
		return;
	}

	/* check the liquid against vnum and name */
	if( ( liq = check_liquid( argument ) ) == NULL )
	{
		send_to_char( "That is not a liquid name. Type 'liquids' to get a list.\n\r", ch );
		return;
	}

	/* list that beeatch */
	pager_printf_color( ch, "&GLiquid Information\n\r"
				"&GLiquid Name:&g\t  %s\n\r"
				"&GLiquid Shortdesc:&g %s\n\r"
				"&GLiquid Color:&g\t  %s\n\r"
			"&GLiquid Type:&g\t  %s\n\r", liq->name, liq->shortdesc, liq->color, liquid_types[liq->type] );

	pager_printf_color( ch, "&GLiquid Vnum:&g\t  %d\n\r"
				"&GLiquid Modifiers\n\r"
					"&G\tThirst:&g\t  %d\n\r"
					"&G\tDrunk:&g\t  %d\n\r"
					"&G\tBlood:&g\t  %d\n\r"
					"&G\tFull:&g\t	%d\n\r", liq->vnum, liq->mod[COND_THIRST], liq->mod[COND_DRUNK], liq->mod[3], liq->mod[COND_FULL] );
	/* mixtures  - 8/21/01 */
	pager_printf_color( ch, "&GMixtures Information\n\r" );
	for( mix = first_mixture; mix; mix = mix->next )
	{
		if( mix->with1 == liq->vnum || mix->with2 == liq->vnum )
		{
			found = TRUE;
			pager_printf_color( ch, "&G\tMixture name:&g\t	%s\n\r", mix->name );
			pager_printf_color( ch, "&G\t%d mixes with %d to form %d.\n\r", mix->with1, mix->with2, mix->into );
		}
	}

	if( !found )
		pager_printf_color( ch, "&R%s currently has no mixtures.&g\n\r", liq->name );

	return;
}
/*
========================
Check objects		-Nopey
Might be a function for
this already, but I'm
too lazy to look for it
========================
*/
OBJ_DATA *get_objdata_vnum( int vnum )
{
	OBJ_DATA *obj = NULL;

	for( obj = first_object; obj; obj = obj->next )
	{
		if( obj->pIndexData->vnum == vnum )
			return obj;
	}
	return NULL;
}

/*
=====================================
Checks to make sure two liquids can
mix, then returns the final product	-Nopey
(ack, this one had to be overhauled!)
=====================================
*/
LIQUID_DATA *liq_can_mix( int vnum, LIQUID_DATA *liq2, bool is_liquid )
{
	LIQUID_MIXTURE_LIST *mix = NULL;
	OBJ_DATA *obj = NULL;
	LIQUID_DATA *liq = NULL;
	LIQUID_DATA *liq3 = NULL;
	bool match = FALSE;
	int mix_into = 0;

	if( is_liquid == TRUE )
	{
		if( (liq = get_liq_index( vnum ) ) == NULL )
		{
			bug( "liq_can_mix: NULL liquid vnum (%d).", vnum );
			return NULL;
		}
	}
	else if( is_liquid == FALSE )
	{
		if( (obj = get_objdata_vnum( vnum ) ) == NULL )
		{
			bug( "liq_can_mix: NULL obj vnum (%d).", vnum );
			return NULL;
		}
	}	

	for( mix = first_mixture; mix; mix = mix->next )
	{
		if( mix->object == TRUE && is_liquid == FALSE )
		{
			/* object can only be with1 */
			if( mix->with1 == obj->pIndexData->vnum && liq2->vnum == mix->with2 )
			{
				mix_into = mix->into;
				match = TRUE;
				break;
			}
		}
		else if( is_liquid == TRUE && mix->object == FALSE )
		{
			if( ( mix->with1 == liq->vnum && mix->with2 == liq2->vnum )
			 || ( mix->with1 == liq2->vnum && mix->with2 == liq->vnum ) )
			{
				mix_into = mix->into;
				match = TRUE;
				break;	
			}
		}
	}

	/* can mix two of the same liquids */
	if( is_liquid == TRUE && (liq->vnum == liq2->vnum))
	{
		match = TRUE;
		mix_into = liq->vnum;
	}
	
	if( !match )
		return NULL;
	else
	{
		if( ( liq3 = get_liq_index( mix_into ) ) == NULL )
			return NULL;
		else
			return liq3;
	}
}
	
/*
===========================
Mix two liquids together	-Nopey
===========================
*/
void do_mix( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *src = NULL;
	OBJ_DATA *obj = NULL;
	LIQUID_DATA *liq = NULL;
	LIQUID_DATA *liq2 = NULL;
	LIQUID_DATA *liq3 = NULL;
	bool is_liquid = FALSE;
	char arg[MAX_INPUT_LENGTH];
	int amount = 0;
	int tvnum = 0;
	
	argument = one_argument( argument, arg );	

	if( arg[0] == '\0' || argument[0] == '\0' )
	{
		send_to_char( "Mix what into what?\n\r", ch );
		return;
	}
	
	if( (src = get_obj_carry( ch, arg ) ) == NULL || (obj = get_obj_carry( ch, argument ) ) == NULL )
	{
		send_to_char( "You aren't carrying that item.\n\r", ch );
		return;
	}
	
	/* C-O-M-P-R-E-S-S ;) */
	if( (src->item_type != ITEM_DRINK_CON && src->item_type != ITEM_DRINK_MIX) || obj->item_type != ITEM_DRINK_CON )
	{
		send_to_char( "That's not a drink container!\n\r", ch );
		return;
	}
	
	/* mix an obj into a liquid/drink */
	if( src->item_type == ITEM_DRINK_MIX && obj->item_type != ITEM_DRINK_CON )
	{
		send_to_char( "But what will you mix it in?\n\r", ch );
		return;
	}

	/* can I get a refill here? */
	if( (src->value[1] == 0 && src->item_type != ITEM_DRINK_MIX) || obj->value[1] == 0 )
	{
		send_to_char( "But it's empty.\n\r", ch );
		return;
	}

	/* Ick, this is gonna be a hard one! */
	if( src->item_type != ITEM_DRINK_MIX )
	{
		if( ( liq = get_liq_index( src->value[2] ) ) == NULL )
		{
			bug( "Liquid: NULL liquid type on %s.", ch->name );
			tvnum = 0;
			is_liquid = TRUE; /* _is_ a liquid */
		}
		else
		{
			tvnum = liq->vnum;
			is_liquid = TRUE;
		}
	}
	else
	{
		tvnum = src->pIndexData->vnum; /* snatch up the mixture number */
		is_liquid = FALSE; /* not a liquid */
	}
	
	/* can only be a liquid */
	if( ( liq2 = get_liq_index( obj->value[2] ) ) == NULL )
	{
		bug( "Liquid: NULL liquid type on %s.", ch->name );
		liq2 = get_liq_index( 0 );
	}

	/* nerfed up because of the addition of being able
	 * to use objects as a 'mix' to mix liquids. */
	if( ( liq3 = liq_can_mix( tvnum, liq2, is_liquid ) ) == NULL )
	{
		send_to_char( "They don't mix together too well.\n\r", ch );
		return;
	}		

	/* put the new liquid in the second container
	 * mix the amounts, empty the first container
	 * - Only two liquids, might make something for
	 *   obj mix sizes later on */
	if( is_liquid == TRUE )
	{
		amount = (src->value[1] + obj->value[1]);
		src->value[1] = 0;
		obj->value[1] = amount;
		obj->value[2] = liq3->vnum;
	}
	else
	{
		obj->value[1] += src->value[1];
		obj->value[2] = liq3->vnum;
		DESTROYOBJ( src );
	}
	send_to_char( "You mix them together.\n\r", ch );
	return;
}

/*
=====================================
Modified Smaug functions		-Nopey
NOTE: Any functions that you've
changed after the ifcheck you
will need to modify your func,
or the on in this file. It's probably
best to leave all of the liquid funcs
in this file.
======================================
*/
#ifdef LIQUIDSYSTEM
void do_drink( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_DATA *obj;
   LIQUID_DATA *liq = NULL;
   AFFECT_DATA af;
   int amount;

   argument = one_argument( argument, arg );
   /* munch optional words */
   if ( !str_cmp( arg, "from" ) && argument[0] != '\0' )
	argument = one_argument( argument, arg );

   if ( arg[0] == '\0' )
   {
	for ( obj = ch->in_room->first_content; obj; obj = obj->next_content )
	   if( obj->item_type == ITEM_FOUNTAIN )
		break;

	if ( !obj )
	{
	   send_to_char( "Drink what?\n\r", ch );
	   return;
	}
   }

   else
   {
	if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
	{
	   send_to_char( "You can't find it.\n\r", ch );
	   return;
	}
   }

   if ( obj->count > 1 && obj->item_type != ITEM_FOUNTAIN )
	separate_obj( obj );

   if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > MAX_COND_VAL - 8 )
   {
	send_to_char( "You fail to reach your mouth.  *Hic*\n\r", ch );
	return;
   }

   switch ( obj->item_type )
   {
	default:
	   if ( obj->carried_by == ch )
	   {
	      act( AT_ACTION, "$n lifts $p up to $s mouth and tries to drink from it...", ch, obj, NULL, TO_ROOM );
	      act( AT_ACTION, "You bring $p up to your mouth and try to drink from it...", ch, obj, NULL, TO_CHAR );
	   }
	   else
	   {
	      act( AT_ACTION, "$n gets down and tries to drink from $p... (Is $e feeling ok?)", ch, obj, NULL, TO_ROOM );
	      act( AT_ACTION, "You get down on the ground and try to drink from $p...", ch, obj, NULL, TO_CHAR );
	   }
	   break;

    case ITEM_BLOOD:
	if ( IS_VAMPIRE(ch) && !IS_NPC(ch) )
	{
	    if ( obj->timer > 0 	/* if timer, must be spilled blood */
	    &&	 ch->level > 5
	    &&	 ch->pcdata->condition[COND_BLOODTHIRST] > (5+ch->level/10) )
	    {
		send_to_char( "It is beneath you to stoop to drinking blood from the ground!\n\r", ch );
		send_to_char( "Unless in dire need, you'd much rather have blood from a victim's neck!\n\r", ch );
		return;
	    }
	    if ( ch->pcdata->condition[COND_BLOODTHIRST] < (10 + ch->level) )
	    {
		if ( ch->pcdata->condition[COND_FULL] >= 48
		||   ch->pcdata->condition[COND_THIRST] >= 48 )
		{
		    send_to_char( "You are too full to drink any blood.\n\r", ch );
		    return;
		}

		if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
		   act( AT_BLOOD, "$n drinks from the spilled blood.", ch, NULL, NULL, TO_ROOM );
		   set_char_color( AT_BLOOD, ch );
		   send_to_char( "You relish in the replenishment of this vital fluid...\n\r", ch );
		   if (obj->value[1] <=1) 
		   {
			set_char_color( AT_BLOOD, ch );
			send_to_char( "You drink the last drop of blood from the spill.\n\r", ch);
			act( AT_BLOOD, "$n drinks the last drop of blood from the spill.", ch, NULL, NULL, TO_ROOM );
		   }
		}

		gain_condition(ch, COND_BLOODTHIRST, 1);
		gain_condition(ch, COND_FULL, 1);
		gain_condition(ch, COND_THIRST, 1);
		if (--obj->value[1] <=0)
		{
		   if ( obj->serial == cur_obj )
		     global_objcode = rOBJ_DRUNK;
		   extract_obj( obj );
		   make_bloodstain( ch );
		}
	    }
	    else
	      send_to_char( "Alas... you cannot consume any more blood.\n\r", ch );
	}
	else
	  send_to_char( "It is not in your nature to do such things.\n\r", ch );
	break;

      case ITEM_POTION:
	   if ( obj->carried_by == ch )
	      do_quaff( ch, obj->name );
	   else
	      send_to_char( "You're not carrying that.\n\r", ch );
	   break;

      case ITEM_FOUNTAIN:
	   if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
	   {
	      act( AT_ACTION, "$n drinks from the fountain.", ch, NULL, NULL, TO_ROOM );
	      send_to_char( "You take a long thirst-quenching drink.\n\r", ch );
	   }

	   if ( !IS_NPC(ch) )
	      ch->pcdata->condition[COND_THIRST] = MAX_COND_VAL;
	   break;

      case ITEM_DRINK_CON:
	   if ( obj->value[1] <= 0 )
	   {
	      send_to_char( "It is already empty.\n\r", ch );
	      return;
	   }

	   if( ch->pcdata->condition[COND_THIRST] == MAX_COND_VAL || ch->pcdata->condition[COND_FULL] == MAX_COND_VAL )
	   {
		send_to_char( "Your stomach is too full to drink anymore!\n\r", ch );
		return;
	   }
 
	   if ( ( liq = get_liq_index( obj->value[2] ) ) == NULL )
	   {
	      bug( "Do_drink: bad liquid number %d.", obj->value[2] );
	      liq = get_liq_index( 0 );
	   }

	   if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
	   {
	      act( AT_ACTION, "$n drinks $T from $p.", ch, obj, liq->shortdesc, TO_ROOM );
	      act( AT_ACTION, "You drink $T from $p.", ch, obj, liq->shortdesc, TO_CHAR );
	   }

	   amount = 1; /* UMIN(amount, obj->value[1]); */
	   /* what was this? concentrated drinks?  concentrated water too I suppose... sheesh! */

	   gain_condition( ch, COND_DRUNK,  liq->mod[COND_DRUNK] );
	   gain_condition( ch, COND_FULL,   liq->mod[COND_FULL] );
	   gain_condition( ch, COND_THIRST, liq->mod[COND_THIRST] );
	   if( IS_VAMPIRE(ch) )
		gain_condition( ch, COND_BLOODTHIRST, liq->mod[COND_BLOODTHIRST] );

	   if( liq->type == LIQ_POISON )
	 {
	      act( AT_POISON, "$n sputters and gags.", ch, NULL, NULL, TO_ROOM );
	      act( AT_POISON, "You sputter and gag.", ch, NULL, NULL, TO_CHAR );
	      ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );
	      af.type	   = gsn_poison;
	      af.duration  = obj->value[3];
	      af.location  = APPLY_NONE;
	      af.modifier  = 0;
	      af.bitvector = meb(AFF_POISON);
	      affect_join( ch, &af );
	 }

	 if( !IS_NPC(ch) )
	 {
	      if( ch->pcdata->condition[COND_DRUNK] > ( MAX_COND_VAL / 2 ) && ch->pcdata->condition[COND_DRUNK] < ( MAX_COND_VAL * .4 ) )
		   send_to_char( "You feel quite sloshed.\n\r", ch );
	      else if( ch->pcdata->condition[COND_DRUNK] >= ( MAX_COND_VAL * .4 ) && ch->pcdata->condition[COND_DRUNK] < ( MAX_COND_VAL * .6 ) )
		   send_to_char( "You start to feel a little drunk.\n\r", ch );
	      else if( ch->pcdata->condition[COND_DRUNK] >= ( MAX_COND_VAL * .6 ) && ch->pcdata->condition[COND_DRUNK] < ( MAX_COND_VAL * .9 ) )
		   send_to_char( "Your vision starts to get blurry.\n\r", ch );
	      else if( ch->pcdata->condition[COND_DRUNK] >= ( MAX_COND_VAL * .9 ) && ch->pcdata->condition[COND_DRUNK] < MAX_COND_VAL )
		   send_to_char( "You feel very drunk.\n\r", ch );
	      else if( ch->pcdata->condition[COND_DRUNK] == MAX_COND_VAL )
		   send_to_char( "You feel like your going to pass out.\n\r", ch );

	      if( ch->pcdata->condition[COND_THIRST] > ( MAX_COND_VAL / 2 ) && ch->pcdata->condition[COND_THIRST] < ( MAX_COND_VAL * .4 ) )
		   send_to_char( "Your stomach begins to slosh around.\n\r", ch );
	      else if( ch->pcdata->condition[COND_THIRST] >= ( MAX_COND_VAL * .4 ) && ch->pcdata->condition[COND_THIRST] < ( MAX_COND_VAL * .6 ) )
		   send_to_char( "You start to feel bloated.\n\r", ch );
	      else if( ch->pcdata->condition[COND_THIRST] >= ( MAX_COND_VAL * .6 ) && ch->pcdata->condition[COND_THIRST] < ( MAX_COND_VAL * .9 ) )
		   send_to_char( "You feel bloated.\n\r", ch );
	      else if( ch->pcdata->condition[COND_THIRST] >= ( MAX_COND_VAL * .9 ) && ch->pcdata->condition[COND_THIRST] < MAX_COND_VAL )
		   send_to_char( "You stomach is almost filled to it's brim!\n\r", ch );
	      else if( ch->pcdata->condition[COND_THIRST] == MAX_COND_VAL )
		   send_to_char( "Your stomach is full, you can't manage to get anymore down.\n\r", ch );

	   /* Hopefully this is the reason why that crap was happening. =0P */
	   if( IS_VAMPIRE(ch) )
	   {
	      if( ch->pcdata->condition[COND_BLOODTHIRST] > ( MAX_COND_VAL / 2 ) && ch->pcdata->condition[COND_BLOODTHIRST] < ( MAX_COND_VAL * .4 ) )
		send_to_char_color( "&rYou replenish your body with the vidal fluid.\n\r", ch );
	      else if( ch->pcdata->condition[COND_BLOODTHIRST] >= ( MAX_COND_VAL * .4 ) && ch->pcdata->condition[COND_BLOODTHIRST] < ( MAX_COND_VAL * .6 ) )
		send_to_char_color( "&rYour thirst for blood begins to decrease.\n\r", ch );
	      else if( ch->pcdata->condition[COND_BLOODTHIRST] >= ( MAX_COND_VAL *.6 ) && ch->pcdata->condition[COND_BLOODTHIRST] < ( MAX_COND_VAL * .9 ) )
		send_to_char_color( "&rThe thirst for blood begins to leave you...\n\r", ch );
	      else if( ch->pcdata->condition[COND_BLOODTHIRST] >= ( MAX_COND_VAL * .9 ) && ch->pcdata->condition[COND_BLOODTHIRST] < MAX_COND_VAL )
		send_to_char( "&rYou drink the last drop of the fluid, the thirst for more leaves your body.\n\r", ch );
	   }
	   else if( !IS_VAMPIRE(ch) && ch->pcdata->condition[COND_BLOODTHIRST] >= MAX_COND_VAL )
	   {
		ch->pcdata->condition[COND_BLOODTHIRST ] = 0;
	   }
	 }

	   obj->value[1] -= amount;
	   if ( obj->value[1] <= 0 ) /* Come now, what good is a drink container that vanishes?? */
	   {
	      obj->value[1] = 0; /* Prevents negative values - Samson */
	      send_to_char( "You drink the last drop from your container.\n\r", ch );
	      if ( cur_obj == obj->serial )
		 global_objcode = rOBJ_DRUNK;
	      /* extract_obj( obj );	  Modified 4-21-98 - Samson */
	   }
	   break;
   }

   if ( who_fighting( ch ) && IS_PKILL( ch ) )
      WAIT_STATE( ch, PULSE_PER_SECOND/3 );
   else
      WAIT_STATE( ch, PULSE_PER_SECOND );
   return;
}

/*
 * Fill a container
 * Many enhancements added by Thoric (ie: filling non-drink containers)
 * Edited for new liquidsystem		-Nopey
 */
void do_fill( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *source;
    sh_int    dest_item, src_item1, src_item2, src_item3;
    int       diff = 0;
    bool      all = FALSE;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    /* munch optional words */
    if ( (!str_cmp( arg2, "from" ) || !str_cmp( arg2, "with" ))
    &&	  argument[0] != '\0' )
	argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Fill what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }
    else
	dest_item = obj->item_type;

    src_item1 = src_item2 = src_item3 = -1;
    switch( dest_item )
    {
	default:
	  act( AT_ACTION, "$n tries to fill $p... (Don't ask me how)", ch, obj, NULL, TO_ROOM );
	  send_to_char( "You cannot fill that.\n\r", ch );
	  return;
	/* place all fillable item types here */
	case ITEM_DRINK_CON:
	  src_item1 = ITEM_FOUNTAIN;	src_item2 = ITEM_BLOOD; 	break;
	case ITEM_HERB_CON:
	  src_item1 = ITEM_HERB;	src_item2 = ITEM_HERB_CON;	break;
	case ITEM_PIPE:
	  src_item1 = ITEM_HERB;	src_item2 = ITEM_HERB_CON;	break;
	case ITEM_CONTAINER:
	  src_item1 = ITEM_CONTAINER;	src_item2 = ITEM_CORPSE_NPC;
	  src_item3 = ITEM_CORPSE_PC;	break;
    }

    if ( dest_item == ITEM_CONTAINER )
    {
	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	{
	    act( AT_PLAIN, "The $d is closed.", ch, NULL, obj->name, TO_CHAR );
	    return;
	}
	if ( get_real_obj_weight( obj ) / obj->count
	>=   obj->value[0] )
	{
	   send_to_char( "It's already full as it can be.\n\r", ch );
	   return;
	}
    }
    else
    {
	diff = MAX_COND_VAL;
	if ( diff < 1 || obj->value[1] >= obj->value[0] )
	{
	   send_to_char( "It's already full as it can be.\n\r", ch );
	   return;
	}
    }

    if ( dest_item == ITEM_PIPE
    &&	 IS_SET( obj->value[3], PIPE_FULLOFASH ) )
    {
	send_to_char( "It's full of ashes, and needs to be emptied first.\n\r", ch );
	return;
    }

    if ( arg2[0] != '\0' )
    {
      if ( dest_item == ITEM_CONTAINER
      && (!str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 )) )
      {
	all = TRUE;
	source = NULL;
      }
      else
      /* This used to let you fill a pipe from an object on the ground.  Seems
	 to me you should be holding whatever you want to fill a pipe with.
	 It's nitpicking, but I needed to change it to get a mobprog to work
	 right.  Check out Lord Fitzgibbon if you're curious.  -Narn */
      if ( dest_item == ITEM_PIPE )
      {
	if ( ( source = get_obj_carry( ch, arg2 ) ) == NULL )
	{
	   send_to_char( "You don't have that item.\n\r", ch );
	   return;
	}
	if ( source->item_type != src_item1 && source->item_type != src_item2
	&&   source->item_type != src_item3 )
	{
	   act( AT_PLAIN, "You cannot fill $p with $P!", ch, obj, source, TO_CHAR );
	   return;
	}
      }
      else
      {
	if ( ( source =  get_obj_here( ch, arg2 ) ) == NULL )
	{
	   send_to_char( "You cannot find that item.\n\r", ch );
	   return;
	}
      }
    }
    else
	source = NULL;

    if ( !source && dest_item == ITEM_PIPE )
    {
	send_to_char( "Fill it with what?\n\r", ch );
	return;
    }

    if ( !source )
    {
	bool	  found = FALSE;
	OBJ_DATA *src_next;

	found = FALSE;
	separate_obj( obj );
	for ( source = ch->in_room->first_content;
	      source;
	      source = src_next )
	{
	    src_next = source->next_content;
	    if (dest_item == ITEM_CONTAINER)
	    {
		if ( !CAN_WEAR(source, ITEM_TAKE)
		||    IS_OBJ_STAT( source, ITEM_BURIED )
		||   (IS_OBJ_STAT( source, ITEM_PROTOTYPE) && !can_take_proto(ch))
		||    ch->carry_weight + get_obj_weight(source) > can_carry_w(ch)
		||   (get_real_obj_weight(source) + get_real_obj_weight(obj)/obj->count)
		    > obj->value[0] )
		  continue;
		if ( all && arg2[3] == '.'
		&&  !nifty_is_name( &arg2[4], source->name ) )
		   continue;
		obj_from_room(source);
		if ( source->item_type == ITEM_MONEY )
		{
		   ch->gold += source->value[0];
		   extract_obj( source );
		}
		else
		   obj_to_obj(source, obj);
		found = TRUE;
	    }
	    else
	    if (source->item_type == src_item1
	    ||	source->item_type == src_item2
	    ||	source->item_type == src_item3)
	    {
		found = TRUE;
		break;
	    }
	}
	if ( !found )
	{
	    switch( src_item1 )
	    {
		default:
		  send_to_char( "There is nothing appropriate here!\n\r", ch );
		  return;
		case ITEM_FOUNTAIN:
		  send_to_char( "There is no fountain or pool here!\n\r", ch );
		  return;
		case ITEM_BLOOD:
		  send_to_char( "There is no blood pool here!\n\r", ch );
		  return;
		case ITEM_HERB_CON:
		  send_to_char( "There are no herbs here!\n\r", ch );
		  return;
		case ITEM_HERB:
		  send_to_char( "You cannot find any smoking herbs.\n\r", ch );
		  return;
	    }
	}
	if (dest_item == ITEM_CONTAINER)
	{
	  act( AT_ACTION, "You fill $p.", ch, obj, NULL, TO_CHAR );
	  act( AT_ACTION, "$n fills $p.", ch, obj, NULL, TO_ROOM );
	  return;
	}
    }

    if (dest_item == ITEM_CONTAINER)
    {
	OBJ_DATA *otmp, *otmp_next;
	char name[MAX_INPUT_LENGTH];
	CHAR_DATA *gch;
	char *pd;
	bool found = FALSE;

	if ( source == obj )
	{
	    send_to_char( "You can't fill something with itself!\n\r", ch );
	    return;
	}

	switch( source->item_type )
	{
	    default:	/* put something in container */
		if ( !source->in_room	/* disallow inventory items */
		||   !CAN_WEAR(source, ITEM_TAKE)
		||   (IS_OBJ_STAT( source, ITEM_PROTOTYPE) && !can_take_proto(ch))
		||    ch->carry_weight + get_obj_weight(source) > can_carry_w(ch)
		||   (get_real_obj_weight(source) + get_real_obj_weight(obj)/obj->count)
		    > obj->value[0] )
		{
		    send_to_char( "You can't do that.\n\r", ch );
		    return;
		}
		separate_obj( obj );
		act( AT_ACTION, "You take $P and put it inside $p.", ch, obj, source, TO_CHAR );
		act( AT_ACTION, "$n takes $P and puts it inside $p.", ch, obj, source, TO_ROOM );
		obj_from_room(source);
		obj_to_obj(source, obj);
		break;
	    case ITEM_MONEY:
		send_to_char( "You can't do that... yet.\n\r", ch );
		break;
	    case ITEM_CORPSE_PC:
		if ( IS_NPC(ch) )
		{
		    send_to_char( "You can't do that.\n\r", ch );
		    return;
		}
		if ( IS_OBJ_STAT( source, ITEM_CLANCORPSE )
		&&  !IS_IMMORTAL( ch ) )
		{
		    send_to_char( "Your hands fumble.  Maybe you better loot a different way.\n\r", ch );
		    return;
		}
		if ( !IS_OBJ_STAT( source, ITEM_CLANCORPSE )
		||   !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
		{
		    pd = source->short_descr;
		    pd = one_argument( pd, name );
		    pd = one_argument( pd, name );
		    pd = one_argument( pd, name );
		    pd = one_argument( pd, name );

		    if ( str_cmp( name, ch->name ) && !IS_IMMORTAL(ch) )
		    {
			bool fGroup;

			fGroup = FALSE;
			for ( gch = first_char; gch; gch = gch->next )
			{
			    if ( !IS_NPC(gch)
			    &&	 is_same_group( ch, gch )
			    &&	 !str_cmp( name, gch->name ) )
			    {
				fGroup = TRUE;
				break;
			    }
			}
			if ( !fGroup )
			{
			    send_to_char( "That's someone else's corpse.\n\r", ch );
			    return;
			}
		    }
		}
	    case ITEM_CONTAINER:
		if ( source->item_type == ITEM_CONTAINER  /* don't remove */
		&&   IS_SET(source->value[1], CONT_CLOSED) )
		{
		    act( AT_PLAIN, "The $d is closed.", ch, NULL, source->name, TO_CHAR );
		    return;
		}
	    case ITEM_CORPSE_NPC:
		if ( (otmp=source->first_content) == NULL )
		{
		    send_to_char( "It's empty.\n\r", ch );
		    return;
		}
		separate_obj( obj );
		for ( ; otmp; otmp = otmp_next )
		{
		    otmp_next = otmp->next_content;

		    if ( !CAN_WEAR(otmp, ITEM_TAKE)
		    ||	 (IS_OBJ_STAT( otmp, ITEM_PROTOTYPE) && !can_take_proto(ch))
		    ||	  ch->carry_number + otmp->count > can_carry_n(ch)
		    ||	  ch->carry_weight + get_obj_weight(otmp) > can_carry_w(ch)
		    ||	 (get_real_obj_weight(source) + get_real_obj_weight(obj)/obj->count)
			> obj->value[0] )
			continue;
		    obj_from_obj(otmp);
		    obj_to_obj(otmp, obj);
		    found = TRUE;
		}
		if ( found )
		{
		   act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
		   act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
		}
		else
		   send_to_char( "There is nothing appropriate in there.\n\r", ch );
		break;
	}
	return;
    }

    if ( source->value[1] < 1 )
    {
	send_to_char( "There's none left!\n\r", ch );
	return;
    }
    if ( source->count > 1 && source->item_type != ITEM_FOUNTAIN )
      separate_obj( source );
    separate_obj( obj );

    switch( source->item_type )
    {
	default:
	  bug( "do_fill: got bad item type: %d", source->item_type );
	  send_to_char( "Something went wrong...\n\r", ch );
	  return;
	case ITEM_FOUNTAIN:
	  if ( obj->value[1] != 0 && obj->value[2] != 0 )
	  {
	     send_to_char( "There is already another liquid in it.\n\r", ch );
	     return;
	  }
	  obj->value[2] = 0;
	  obj->value[1] = obj->value[0];
	  act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
	  act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
	  return;
	case ITEM_BLOOD:
	  if ( obj->value[1] != 0 && obj->value[2] != 13 )
	  {
	     send_to_char( "There is already another liquid in it.\n\r", ch );
	     return;
	  }
	  obj->value[2] = 13;
	  if ( source->value[1] < diff )
	    diff = source->value[1];
	  obj->value[1] += diff;
	  act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
	  act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
	  if ( (source->value[1] -= diff) < 1 )
	  {
	     extract_obj( source );
	     make_bloodstain( ch );
	  }
	  return;
	case ITEM_HERB:
	  if ( obj->value[1] != 0 && obj->value[2] != source->value[2] )
	  {
	     send_to_char( "There is already another type of herb in it.\n\r", ch );
	     return;
	  }
	  obj->value[2] = source->value[2];
	  if ( source->value[1] < diff )
	    diff = source->value[1];
	  obj->value[1] += diff;
	  act( AT_ACTION, "You fill $p with $P.", ch, obj, source, TO_CHAR );
	  act( AT_ACTION, "$n fills $p with $P.", ch, obj, source, TO_ROOM );
	  if ( (source->value[1] -= diff) < 1 )
	     extract_obj( source );
	  return;
	case ITEM_HERB_CON:
	  if ( obj->value[1] != 0 && obj->value[2] != source->value[2] )
	  {
	     send_to_char( "There is already another type of herb in it.\n\r", ch );
	     return;
	  }
	  obj->value[2] = source->value[2];
	  if ( source->value[1] < diff )
	    diff = source->value[1];
	  obj->value[1] += diff;
	  source->value[1] -= diff;
	  act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
	  act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
	  return;
	case ITEM_DRINK_CON:
	  if ( obj->value[1] != 0 && obj->value[2] != source->value[2] )
	  {
	     send_to_char( "There is already another liquid in it.\n\r", ch );
	     return;
	  }
	  obj->value[2] = source->value[2];
	  if ( source->value[1] < diff )
	    diff = source->value[1];
	  obj->value[1] += diff;
	  source->value[1] -= diff;
	  act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
	  act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
	  return;
    }
}

void do_empty( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( !str_cmp( arg2, "into" ) && argument[0] != '\0' )
	argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Empty what?\n\r", ch );
	return;
    }
    if ( ms_find_obj(ch) )
	return;

    if ( (obj = get_obj_carry( ch, arg1 )) == NULL )
    {
	send_to_char( "You aren't carrying that.\n\r", ch );
	return;
    }
    if ( obj->count > 1 )
      separate_obj(obj);

    switch( obj->item_type )
    {
	default:
	  act( AT_ACTION, "You shake $p in an attempt to empty it...", ch, obj, NULL, TO_CHAR );
	  act( AT_ACTION, "$n begins to shake $p in an attempt to empty it...", ch, obj, NULL, TO_ROOM );
	  return;
	case ITEM_PIPE:
	  act( AT_ACTION, "You gently tap $p and empty it out.", ch, obj, NULL, TO_CHAR );
	  act( AT_ACTION, "$n gently taps $p and empties it out.", ch, obj, NULL, TO_ROOM );
	  REMOVE_BIT( obj->value[3], PIPE_FULLOFASH );
	  REMOVE_BIT( obj->value[3], PIPE_LIT );
	  obj->value[1] = 0;
	  return;
	case ITEM_DRINK_CON:
	  if ( obj->value[1] < 1 )
	  {
		send_to_char( "It's already empty.\n\r", ch );
		return;
	  }
	  act( AT_ACTION, "You empty $p.", ch, obj, NULL, TO_CHAR );
	  act( AT_ACTION, "$n empties $p.", ch, obj, NULL, TO_ROOM );
	  obj->value[1] = 0;
	  return;
	case ITEM_CONTAINER:
	case ITEM_QUIVER:
	  if ( IS_SET(obj->value[1], CONT_CLOSED) )
	  {
		act( AT_PLAIN, "The $d is closed.", ch, NULL, obj->name, TO_CHAR );
		return;
	  }
	case ITEM_KEYRING:
	  if ( !obj->first_content )
	  {
		send_to_char( "It's already empty.\n\r", ch );
		return;
	  }
	  if ( arg2[0] == '\0' )
	  {
		if ( xIS_SET( ch->in_room->room_flags, ROOM_NODROP )
		||  xIS_SET( ch->act, PLR_LITTERBUG ) )
		{
		       set_char_color( AT_MAGIC, ch );
		       send_to_char( "A magical force stops you!\n\r", ch );
		       set_char_color( AT_TELL, ch );
		       send_to_char( "Someone tells you, 'No littering here!'\n\r", ch );
		       return;
		}
		if ( xIS_SET( ch->in_room->room_flags, ROOM_NODROPALL ) 
		||   xIS_SET( ch->in_room->room_flags, ROOM_CLANSTOREROOM ) )
		{
		   send_to_char( "You can't seem to do that here...\n\r", ch );
		   return;
		}
		if ( empty_obj( obj, NULL, ch->in_room ) )
		{
		    act( AT_ACTION, "You empty $p.", ch, obj, NULL, TO_CHAR );
		    act( AT_ACTION, "$n empties $p.", ch, obj, NULL, TO_ROOM );
		    if ( IS_SET( sysdata.save_flags, SV_EMPTY ) )
			save_char_obj( ch );
		}
		else
		    send_to_char( "Hmmm... didn't work.\n\r", ch );
	  }
	  else
	  {
		OBJ_DATA *dest = get_obj_here( ch, arg2 );

		if ( !dest )
		{
		    send_to_char( "You can't find it.\n\r", ch );
		    return;
		}
		if ( dest == obj )
		{
		    send_to_char( "You can't empty something into itself!\n\r", ch );
		    return;
		}
		if ( dest->item_type != ITEM_CONTAINER && dest->item_type != ITEM_KEYRING
		&&   dest->item_type != ITEM_QUIVER )
		{
		    send_to_char( "That's not a container!\n\r", ch );
		    return;
		}
		if ( IS_SET(dest->value[1], CONT_CLOSED) )
		{
		    act( AT_PLAIN, "The $d is closed.", ch, NULL, dest->name, TO_CHAR );
		    return;
		}
		separate_obj( dest );
		if ( empty_obj( obj, dest, NULL ) )
		{
		    act( AT_ACTION, "You empty $p into $P.", ch, obj, dest, TO_CHAR );
		    act( AT_ACTION, "$n empties $p into $P.", ch, obj, dest, TO_ROOM );
		    if ( !dest->carried_by
		    &&	  IS_SET( sysdata.save_flags, SV_EMPTY ) )
			save_char_obj( ch );
		}
		else
		    act( AT_ACTION, "$P is too full.", ch, obj, dest, TO_CHAR );
	  }
	  return;
    }
}
#endif
