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
 * 			Variable Handling Module (Thoric)		    *
 ****************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#ifdef USE_IMC
#include "imc-mercbase.h"
#include "icec-mercbase.h"
#endif


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


VARIABLE_DATA *make_variable(char type, int vnum, char *tag)
{
    VARIABLE_DATA *var;

    CREATE(var, VARIABLE_DATA, 1);
    var->next = NULL;
    var->type = type;
    var->flags = 0;
    var->vnum = vnum;
    var->tag = str_dup(tag);
    var->c_time = current_time;
    var->m_time = current_time;
    var->r_time = 0;
    var->timer = 0;
    switch(type)
    {
	case vtINT:
	    var->data = NULL;
	    break;
	case vtXBIT:
	    CREATE(var->data, EXT_BV, 1);
	    break;
	case vtSTR:
	    var->data = NULL;
    }
    return var;
}

void delete_vdata(VARIABLE_DATA *var)
{
    switch(var->type)
    {
	case vtXBIT:
	case vtSTR:
	    if ( var->data )
		DISPOSE(var->data);
	    break;
    }
}

void delete_variable(VARIABLE_DATA *var)
{
    delete_vdata(var);
    DISPOSE(var->tag);
    DISPOSE(var);
}

/*
 * Return the specified tag from a character
 */
VARIABLE_DATA *get_tag(CHAR_DATA *ch, char *tag, int vnum)
{
    VARIABLE_DATA *vd;

    for ( vd = ch->variables; vd; vd = vd->next )
	if ( (!vnum || vnum == vd->vnum) && !str_cmp(tag, vd->tag) )
	    return vd;
    return NULL;
}

/*
 * Remove the specified tag from a character
 */
bool remove_tag(CHAR_DATA *ch, char *tag, int vnum)
{
    VARIABLE_DATA *vd_next, *vd = ch->variables;

    if ( !vd )
	return FALSE;

    if ( (!vnum || vnum == vd->vnum) && !str_cmp(tag, vd->tag) )
    {
	ch->variables = vd->next;
	delete_variable(vd);
	return TRUE;
    }

    for ( ; vd && vd->next; vd = vd_next )
    {
	vd_next = vd->next;
	if ( (!vnum || vnum == vd_next->vnum) && !str_cmp(tag, vd_next->tag) )
	{
	    vd->next = vd_next->next;
	    delete_variable(vd_next);
	    return TRUE;
	}
    }
    return FALSE;
}

/*
 * Tag a variable onto a character  Will replace if specified to do so,
 * otherwise if already exists, fail
 */
int tag_char(CHAR_DATA *ch, VARIABLE_DATA *var, int replace)
{
    VARIABLE_DATA *vd, *pvd;
    bool found = FALSE;

    pvd = vd = ch->variables;
    for ( ; vd; vd = vd->next )
    {
	if ( vd == var )	/* same variable -- leave it be */
	{
	    var->m_time = current_time;
	    return 0;
	}
	if ( vd->vnum == var->vnum && !str_cmp(vd->tag, var->tag) )
	{
	    if ( !replace )
		return -1;
	    found = TRUE;
	    break;
	}
	pvd = vd;
    }

    if ( found )
    {
	var->m_time = current_time;
	var->c_time = vd->c_time;
	var->r_time = vd->r_time;
	var->next = vd->next;
	if ( vd == ch->variables )
	    ch->variables = var;
	else
	    pvd->next = var;
	delete_variable(vd);
	return 0;
    }
    var->next = ch->variables;
    ch->variables = var;

    return 0;
}

bool is_valid_tag(const char *tagname)
{
  if (!isalpha(*tagname))
    return FALSE;
  for (++tagname; *tagname; ++tagname)
     if (!isalnum(*tagname) && *tagname != '_')
       return FALSE;
   return TRUE;
}


/*
 *  "tag" is a text identifier to refer to the variable and can
 *  be suffixed with a colon and a mob vnum  ie:  questobj:1101
 *  vnum 0 is used to denote a global tag (local to the victim)
 *  otherwise tags are separated by vnum
 *
 *  mptag	<victim> <tag> [value]
 *  mprmtag	<victim> <tag>
 *  mpflag	<victim> <tag> <flag>
 *  mprmflag    <victim> <tag> <flag>
 *
 *  if istagged($n,tag) [== value]
 *  if isflagged($n,tag[,bit])
 */


/*
 * mptag <victim> <tag> [value]
 */
void do_mptag( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA	  *victim;
    VARIABLE_DATA *vd;
    char	  *p;
    char	   arg1[MAX_INPUT_LENGTH];
    char	   arg2[MAX_INPUT_LENGTH];
    int		   vnum = 0, exp = 0;
    bool	   error = FALSE;

    if ( (!IS_NPC(ch) && get_trust(ch) < LEVEL_GREATER)
    ||     IS_CHARMED(ch) )
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg1 );
    if ( !str_cmp(arg1, "noexpire") )
    {
	exp = 0;
	argument = one_argument( argument, arg1 );
    }
    else if ( !str_cmp(arg1, "timer") )
    {
	argument = one_argument( argument, arg1 );
	exp = atoi(arg1);
	argument = one_argument( argument, arg1 );
    }
    else
	exp = ch->level * get_curr_int(ch);
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "MPtag whom with what?\n\r", ch );
	return;
    }

    if ( (victim=get_char_room(ch, arg1)) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( (p=strchr(arg2, ':')) != NULL )
    {
	*p++ = '\0';
	vnum = atoi(p);
    }
    else
    {
	vnum = ch->pIndexData ? ch->pIndexData->vnum : 0;
    }

    if ( !is_valid_tag(arg2) )
    {
	progbug( "Mptag:  invalid characters in tag", ch );
	return;
    }
    error = FALSE;
    for ( p = argument; *p; p++ )
    {
	if ( !isdigit(*p) && !isspace(*p) )
	{
	    error = TRUE;
	    break;
	}
    }
    if ( error )
    {
	vd = make_variable(vtSTR, vnum, arg2);
	vd->data = str_dup(argument);
    }
    else
    {
	vd = make_variable(vtINT, vnum, arg2);
	vd->data = (void *)atoi(argument);
    }
    vd->timer = exp;
    tag_char(victim, vd, 1);
}

/*
 * mprmtag <victim> <tag>
 */
void do_mprmtag( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA	  *victim;
    char	  *p;
    char	   arg1[MAX_INPUT_LENGTH];
    char	   arg2[MAX_INPUT_LENGTH];
    int		   vnum = 0;

    if ( (!IS_NPC(ch) && get_trust(ch) < LEVEL_GREATER)
    ||     IS_CHARMED(ch) )
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "MPtag whom with what?\n\r", ch );
	return;
    }

    if ( (victim=get_char_room(ch, arg1)) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( (p=strchr(arg2, ':')) != NULL )
    {
	*p++ = '\0';
	vnum = atoi(p);
    }
    else
    {
	vnum = ch->pIndexData ? ch->pIndexData->vnum : 0;
    }

	if ( !is_valid_tag( arg2 ) )
    {
	progbug( "Mptag:  invalid characters in tag", ch );
	return;
    }

    if ( !remove_tag(victim, arg2, vnum) )
    {
	progbug( "Mptag:  could not find tag", ch );
    }
}

/*
 * mpflag <victim> <tag> <flag>
 */
void do_mpflag( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA	  *victim;
    VARIABLE_DATA *vd;
    char	  *p;
    char	   arg1[MAX_INPUT_LENGTH];
    char	   arg2[MAX_INPUT_LENGTH];
    char	   arg3[MAX_INPUT_LENGTH];
    int		   vnum = 0, exp = 0, def = 0, flag = 0;
    bool	   error = FALSE;

    if ( (!IS_NPC(ch) && get_trust(ch) < LEVEL_GREATER)
    ||     IS_CHARMED(ch) )
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg1 );
    if ( !str_cmp(arg1, "noexpire") )
    {
	exp = 0;
	argument = one_argument( argument, arg1 );
    }
    else if ( !str_cmp(arg1, "timer") )
    {
	argument = one_argument( argument, arg1 );
	exp = atoi(arg1);
	argument = one_argument( argument, arg1 );
    }
    else
    {
	exp = ch->level * get_curr_int(ch);
	def = 1;
    }
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "MPflag whom with what?\n\r", ch );
	return;
    }

    if ( (victim=get_char_room(ch, arg1)) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( (p=strchr(arg2, ':')) != NULL )
    {
	*p++ = '\0';
	vnum = atoi(p);
    }
    else
    {
	vnum = ch->pIndexData ? ch->pIndexData->vnum : 0;
    }

    if ( !is_valid_tag(arg2) )
    {
	progbug( "Mpflag:  invalid characters in tag", ch );
	return;
    }
    error = FALSE;
    for ( p = arg3; *p; p++ )
    {
	if ( !isdigit(*p) && !isspace(*p) )
	{
	    error = TRUE;
	    break;
	}
    }
    flag = atoi(arg3);
    if ( error || flag < 0 || flag >= MAX_BITS )
    {
	progbug( "Mpflag:  invalid flag value", ch );
	return;
    }
    if ( (vd=get_tag(victim, arg2, vnum)) != NULL )
    {
	if ( vd->type != vtXBIT )
	{
	    progbug( "Mpflag:  type mismatch", ch );
	    return;
	}
	if ( !def )
	    vd->timer = exp;
    }
    else
    {
	vd = make_variable(vtXBIT, vnum, arg2);
	vd->timer = exp;
    }
    xSET_BIT(*(EXT_BV*)vd->data, flag);
    tag_char(victim, vd, 1);
}


/*
 * mprmflag <victim> <tag> <flag>
 */
void do_mprmflag( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA	  *victim;
    VARIABLE_DATA *vd;
    char	  *p;
    char	   arg1[MAX_INPUT_LENGTH];
    char	   arg2[MAX_INPUT_LENGTH];
    char	   arg3[MAX_INPUT_LENGTH];
    int		   vnum = 0;
    bool	   error = FALSE;

    if ( (!IS_NPC(ch) && get_trust(ch) < LEVEL_GREATER)
    ||     IS_CHARMED(ch) )
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "MPrmflag whom with what?\n\r", ch );
	return;
    }

    if ( (victim=get_char_room(ch, arg1)) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( (p=strchr(arg2, ':')) != NULL )
    {
	*p++ = '\0';
	vnum = atoi(p);
    }
    else
    {
	vnum = ch->pIndexData ? ch->pIndexData->vnum : 0;
    }

    if ( !is_valid_tag( arg2 ) )
    {
	progbug( "Mprmflag:  invalid characters in tag", ch );
	return;
    }
    error = FALSE;
    for ( p = arg3; *p; p++ )
    {
	if ( !isdigit(*p) && !isspace(*p) )
	{
	    error = TRUE;
	    break;
	}
    }
    if ( error )
    {
	progbug( "Mprmflag:  invalid flag value", ch );
	return;
    }
    /*
     * Only bother doing anything if the tag exists
     */
    if ( (vd=get_tag(victim, arg2, vnum)) != NULL )
    {
	if ( vd->type != vtXBIT )
	{
	    progbug( "Mprmflag:  type mismatch", ch );
	    return;
	}
	if ( !vd->data )
	{
	    progbug( "Mprmflag:  missing data???", ch );
	    return;
	}
	xREMOVE_BIT(*(EXT_BV*)vd->data, atoi(arg3));
	tag_char(victim, vd, 1);
    }
}

void fwrite_variables(CHAR_DATA *ch, FILE *fp)
{
    VARIABLE_DATA *vd;

    for ( vd = ch->variables; vd; vd = vd->next )
    {
	fprintf(fp, "#VARIABLE\n");
	fprintf(fp, "Type    %d\n",	vd->type);
	fprintf(fp, "Flags   %d\n", vd->flags);
	fprintf(fp, "Vnum    %d\n", vd->vnum);
	fprintf(fp, "Ctime   %ld\n", vd->c_time);
	fprintf(fp, "Mtime   %ld\n", vd->m_time);
	fprintf(fp, "Rtime   %ld\n", vd->r_time);
	fprintf(fp, "Timer   %d\n", vd->timer);
	fprintf(fp, "Tag     %s~\n", vd->tag);
	switch(vd->type)
	{
	    case vtSTR:
		fprintf(fp, "Str     %s~\n", (char *)vd->data);
		break;
	    case vtXBIT:
		fprintf(fp, "Xbit    %s\n", print_bitvector((EXT_BV*)vd->data));
		break;
	    case vtINT:
		fprintf(fp, "Int     %d\n", (int)vd->data);
		break;
	}
	fprintf(fp, "End\n\n");
    }
}

void fread_variable(CHAR_DATA *ch, FILE *fp)
{
    VARIABLE_DATA *pvd;
    char *word;
    bool fMatch;

    CREATE(pvd, VARIABLE_DATA, 1);
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

	    case 'C':
		KEY( "Ctime",	pvd->c_time,		fread_number( fp ) );
		break;

	    case 'E':
		if ( !str_cmp( word, "End" ) )
		{
			switch(pvd->type)
			{
		    default:
		    {
			bug("Fread_variable: invalid/incomplete variable %s", pvd->tag);
			if ( pvd->tag )
			    DISPOSE(pvd->tag);
				DISPOSE(pvd);
		    break;
			}
			case vtSTR:
		    case vtXBIT:
			if ( !pvd->data )
			{
				bug("Fread_variable: invalid/incomplete variable %s", pvd->tag);
				if ( pvd->tag )
			   	 DISPOSE(pvd->tag);
				 DISPOSE(pvd);
				break;
			}
		    case vtINT:
				tag_char(ch, pvd, 1);
				break;
		    }
		    return;
		}
		break;

	    case 'F':
		KEY( "Flags",	pvd->flags,		fread_number( fp ) );
		break;

	    case 'I':
		if ( !str_cmp(word, "Int") )
		{
		    if ( pvd->type != vtINT )
			bug("Fread_variable: Type mismatch -- type(%d) != vtInt", pvd->type);
		    else
		    {
			pvd->data = (void *)fread_number(fp);
			fMatch = TRUE;
		    }
		    break;
		}
		break;

	    case 'M':
		KEY( "Mtime",	pvd->m_time,		fread_number( fp ) );
		break;

	    case 'R':
		KEY( "Rtime",	pvd->r_time,		fread_number( fp ) );
		break;

	    case 'S':
		if ( !str_cmp(word, "Str") )
		{
		    if ( pvd->type != vtSTR )
			bug("Fread_variable: Type mismatch -- type(%d) != vtSTR", pvd->type);
		    else
		    {
			pvd->data = (void *)fread_string_nohash(fp);
			fMatch = TRUE;
		    }
		    break;
		}
		break;
	
	    case 'T':
		KEY( "Tag",	pvd->tag,		fread_string_nohash( fp ) );
		KEY( "Timer",	pvd->timer,		fread_number( fp ) );
		KEY( "Type",	pvd->type,		fread_number( fp ) );
		break;

	    case 'V':
		KEY( "Vnum",	pvd->vnum,		fread_number( fp ) );
		break;

	    case 'X':
		if ( !str_cmp(word, "Xbit") )
		{
		    if ( pvd->type != vtXBIT )
			bug("Fread_variable: Type mismatch -- type(%d) != vtXBIT", pvd->type);
		    else
		    {
			CREATE(pvd->data, EXT_BV, 1);
			*(EXT_BV *)pvd->data = fread_bitvector(fp);
			fMatch = TRUE;
		    }
		    break;
		}
		break;
	}
	
	if ( !fMatch )
	{
	    bug( "Fread_variable: no match: %s", word );
	}
    }
}
