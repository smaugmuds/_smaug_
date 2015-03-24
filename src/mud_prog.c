/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops, Fireblade, Edmond, Conran                         |             *
 ****************************************************************************
 *  The MUDprograms are heavily based on the original MOBprogram code that  *
 *  was written by N'Atas-ha.						    *
 *  Much has been added, including the capability to put a "program" on     *
 *  rooms and objects, not to mention many more triggers and ifchecks, as   *
 *  well as "script" support.						    *
 *                                                                          *
 *  Error reporting has been changed to specify whether the offending       *
 *  program is on a mob, a room or and object, along with the vnum.         *
 *                                                                          *
 *  Mudprog parsing has been rewritten (in mprog_driver). Mprog_process_if  *
 *  and mprog_process_cmnd have been removed, mprog_do_command is new.      *
 *  Full support for nested ifs is in.                                      *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "mud.h"


/*
 * Recursive function used by the carryingvnum ifcheck.
 * It loops thru all objects belonging to a char (in nested containers)
 * and returns TRUE if it finds a matching vnum.
 * I declared it static to limit its scope to this file.  --Gorog
 *
 * This recursive function works by using the following method for
 * traversing the nodes in a binary tree:
 *
 *    Start at the root node
 *    if there is a child then visit the child
 *       if there is a sibling then visit the sibling
 *    else
 *    if there is a sibling then visit the sibling
 */
static bool carryingvnum_visit( CHAR_DATA * ch, OBJ_DATA *obj, int vnum )
{
/*
   pager_printf(ch, "***obj=%s vnum=%d\n\r", obj->name, obj->pIndexData->vnum );
*/
   if ( obj->wear_loc == -1 && obj->pIndexData->vnum == vnum )
        return TRUE;
   if ( obj->first_content )  /* node has a child? */
   {
      if ( carryingvnum_visit( ch, obj->first_content, vnum ) )
         return TRUE;
      if ( obj->next_content ) /* node has a sibling? */
         if ( carryingvnum_visit( ch, obj->next_content, vnum ) )
            return TRUE;
   }
   else 
   if ( obj->next_content )  /* node has a sibling? */
      if ( carryingvnum_visit( ch, obj->next_content, vnum ) )
         return TRUE;
   return FALSE;
}

/*  Defines by Narn for new mudprog parsing, used as 
   return values from mprog_do_command. */
#define COMMANDOK    1
#define IFTRUE       2
#define IFFALSE      3
#define ORTRUE       4
#define ORFALSE      5
#define FOUNDELSE    6
#define FOUNDENDIF   7
#define IFIGNORED    8
#define ORIGNORED    9

/* Ifstate defines, used to create and access ifstate array
   in mprog_driver. */
/* Moved these to mud.h as I needed two of them for mpsleep -rkb */ 

int mprog_do_command( char *cmnd, CHAR_DATA *mob, CHAR_DATA *actor, 
                      OBJ_DATA *obj, CHAR_DATA *victim, OBJ_DATA *target, CHAR_DATA *rndm, 
                      bool ignore, bool ignore_ors );

/*
 *  Mudprogram additions
 */
CHAR_DATA *supermob;
struct act_prog_data *room_act_list;
struct act_prog_data *obj_act_list;
struct act_prog_data *mob_act_list;

/* 
 * Global variables to handle sleeping mud progs. 
 */ 
MPSLEEP_DATA *first_mpsleep = NULL; 
MPSLEEP_DATA *last_mpsleep = NULL; 
MPSLEEP_DATA *current_mpsleep = NULL; 

/*
 * Local function prototypes
 */

char *	mprog_next_command	args( ( char* clist ) );
bool	mprog_seval		args( ( char* lhs, char* opr, char* rhs,
                                        CHAR_DATA *mob ) );
bool	mprog_veval		args( ( int lhs, char* opr, int rhs,
                                        CHAR_DATA *mob ) );
int	mprog_do_ifcheck	args( ( char* ifcheck, CHAR_DATA* mob,
				       CHAR_DATA* actor, OBJ_DATA* obj,
				       CHAR_DATA *victim, OBJ_DATA *target, CHAR_DATA* rndm ) );
void	mprog_translate		args( ( char ch, char* t, CHAR_DATA* mob,
				       CHAR_DATA* actor, OBJ_DATA* obj,
				       CHAR_DATA *vict, OBJ_DATA *v_obj, CHAR_DATA* rndm ) );
void	mprog_driver		args( ( char* com_list, CHAR_DATA* mob,
				       CHAR_DATA* actor, OBJ_DATA* obj,
				       CHAR_DATA *victim, OBJ_DATA *target, bool single_step ) );

bool mprog_keyword_check	args( ( const char *argu, const char *argl ) );


bool oprog_wordlist_check( char *arg, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, CHAR_DATA *victim, OBJ_DATA *target, int type, OBJ_DATA *iobj );
void set_supermob(OBJ_DATA *obj);
bool oprog_percent_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, CHAR_DATA *victim, OBJ_DATA *target, int type);
void rprog_percent_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, CHAR_DATA *victim, OBJ_DATA *target, int type);
bool rprog_wordlist_check( char *arg, CHAR_DATA *mob, CHAR_DATA *actor,
			  OBJ_DATA *obj, CHAR_DATA *victim, OBJ_DATA *target, int type, ROOM_INDEX_DATA *room );

/***************************************************************************
 * Local function code and brief comments.
 */

/* if you dont have these functions, you damn well should... */

#ifdef DUNNO_STRSTR
char * strstr(s1,s2) const char *s1; const char *s2;
{
  char *cp;
  int i,j=strlen(s1)-strlen(s2),k=strlen(s2);
  if(j<0)
    return NULL;
  for(i=0; i<=j && strncmp(s1++,s2, k)!=0; i++);
  return (i>j) ? NULL : (s1-1);
}
#endif

#define RID ROOM_INDEX_DATA

void init_supermob()
{
   RID *office;

   supermob = create_mobile(get_mob_index( 3 ));
   office = get_room_index ( 3 );
   char_to_room( supermob, office );

#ifdef NOTDEFD
   CREATE( supermob, CHAR_DATA, 1 );
   clear_char( supermob );

   xSET_BIT(supermob->act,ACT_IS_NPC);
   supermob->name 		= STRALLOC("supermob");
   supermob->short_descr 	= STRALLOC("supermob");
   supermob->long_descr 	= STRALLOC("supermob is here");

   CREATE( supermob_index, MOB_INDEX_DATA, 1 )
#endif
}


#undef RID


/* Used to get sequential lines of a multi line string (separated by "\n\r")
 * Thus its like one_argument(), but a trifle different. It is destructive
 * to the multi line string argument, and thus clist must not be shared.
 */
char *mprog_next_command( char *clist )
{

  char *pointer = clist;

  while ( *pointer != '\n' && *pointer != '\0' )
    pointer++;
  if ( *pointer == '\n' )
    *pointer++ = '\0';
  if ( *pointer == '\r' )
    *pointer++ = '\0';

  return ( pointer );

}

/* These two functions do the basic evaluation of ifcheck operators.
 *  It is important to note that the string operations are not what
 *  you probably expect.  Equality is exact and division is substring.
 *  remember that lhs has been stripped of leading space, but can
 *  still have trailing spaces so be careful when editing since:
 *  "guard" and "guard " are not equal.
 */
bool mprog_seval( char *lhs, char *opr, char *rhs, CHAR_DATA *mob )
{

  if ( !str_cmp( opr, "==" ) )
    return ( bool )( !str_cmp( lhs, rhs ) );
  if ( !str_cmp( opr, "!=" ) )
    return ( bool )( str_cmp( lhs, rhs ) );
  if ( !str_cmp( opr, "/" ) )
    return ( bool )( !str_infix( rhs, lhs ) );
  if ( !str_cmp( opr, "!/" ) )
    return ( bool )( str_infix( rhs, lhs ) );

  sprintf( log_buf, "Improper MOBprog operator '%s'", opr );
  progbug( log_buf, mob );
  return 0;

}

bool mprog_veval( int lhs, char *opr, int rhs, CHAR_DATA *mob )
{

  if ( !str_cmp( opr, "==" ) )
    return ( lhs == rhs );
  if ( !str_cmp( opr, "!=" ) )
    return ( lhs != rhs );
  if ( !str_cmp( opr, ">" ) )
    return ( lhs > rhs );
  if ( !str_cmp( opr, "<" ) )
    return ( lhs < rhs );
  if ( !str_cmp( opr, "<=" ) )
    return ( lhs <= rhs );
  if ( !str_cmp( opr, ">=" ) )
    return ( lhs >= rhs );
  if ( !str_cmp( opr, "&" ) )
    return ( lhs & rhs );
  if ( !str_cmp( opr, "|" ) )
    return ( lhs | rhs );

  sprintf( log_buf, "Improper MOBprog operator '%s'", opr );
  progbug( log_buf, mob );

  return 0;

}

#define isoperator(c) ((c)=='='||(c)=='<'||(c)=='>'||(c)=='!'||(c)=='&'||(c)=='|')
#define MAX_IF_ARGS 6
/*
 * This function performs the evaluation of the if checks.  It is
 * here that you can add any ifchecks which you so desire. Hopefully
 * it is clear from what follows how one would go about adding your
 * own. The syntax for an if check is: ifcheck ( arg ) [opr val]
 * where the parenthesis are required and the opr and val fields are
 * optional but if one is there then both must be. The spaces are all
 * optional. The evaluation of the opr expressions is farmed out
 * to reduce the redundancy of the mammoth if statement list.
 * If there are errors, then return BERR otherwise return boolean 1,0
 * Redone by Altrag.. kill all that big copy-code that performs the
 * same action on each variable..
 */
int mprog_do_ifcheck( char *ifcheck, CHAR_DATA *mob, CHAR_DATA *actor,
		      OBJ_DATA *obj, CHAR_DATA *victim, OBJ_DATA *target, CHAR_DATA *rndm )
{
    char buf[MAX_STRING_LENGTH];
    char opr[MAX_INPUT_LENGTH];
    char *chck, *cvar;
    char *argv[MAX_IF_ARGS];
    char *rval = "";
    char *q, *p = buf;
    int argc = 0;
    CHAR_DATA *chkchar = NULL;
    OBJ_DATA *chkobj = NULL;
    int lhsvl, rhsvl = 0;
  
    if ( !*ifcheck )
    {
	progbug("Null ifcheck", mob);
	return BERR;
    }

    /*
     * New parsing by Thoric to allow for multiple arguments inside the
     * brackets, ie: if leveldiff($n, $i) > 10
     * It's also smaller, cleaner and probably faster
     */
    strcpy(buf, ifcheck);  opr[0] = '\0';
    while ( isspace(*p) ) ++p;
    argv[argc++] = p;
    while ( isalnum(*p) ) ++p;
    while ( isspace(*p) ) *p++ = '\0';
    if ( *p != '(' )
    {
	progbug("Ifcheck Syntax error (missing left bracket)", mob);
	return BERR;
    }

    *p++ = '\0';
    /* Need to check for spaces or if name( $n ) isn't legal --Shaddai */
    while ( isspace(*p) ) *p++ = '\0';
    for (;;)
    {
	argv[argc++] = p;
	while ( *p == '$' || isalnum(*p) || *p == ':' ) ++p;
	while ( isspace(*p) ) *p++ = '\0';
	switch(*p)
	{
	    case ',':
		*p++ = '\0';
		while ( isspace(*p) ) *p++ = '\0';
		if ( argc >= MAX_IF_ARGS )
		{
		    while ( *p && *p != ')' ) ++p;
		    if ( *p )
			*p++ = '\0';
		    while ( isspace(*p) ) *p++ = '\0';
		    goto doneargs;
		}
		break;
	    case ')':
		*p++ = '\0';
		while ( isspace(*p) ) *p++ = '\0';
	    	goto doneargs;
	    	break;
	    default:
		progbug("Ifcheck Syntax warning (missing right bracket)", mob);
		goto doneargs;
		break;
	}
    }    
doneargs:
    q = p;
    while ( isoperator(*p) ) ++p;
    strncpy(opr, q, p-q);
    opr[p-q] = '\0';
    while ( isspace(*p) ) *p++ = '\0';
    rval = p;
    /*
    while ( *p && !isspace(*p) ) ++p;
    */
    while ( *p ) ++p;
    *p = '\0';

    chck = argv[0] ? argv[0] : "";
    cvar = argv[1] ? argv[1] : "";
  
    /*
     * chck contains check, cvar is the variable in the (), opr is the
     * operator if there is one, and rval is the value if there was an
     * operator.
     */
    if ( cvar[0] == '$' )
    {
	switch(cvar[1])
	{
	    case 'i':	chkchar = mob;			break;
	    case 'n':	chkchar = actor;		break;
	    case 't':	chkchar = victim;		break;
	    case 'r':	chkchar = rndm;			break;
	    case 'o':	chkobj = obj;			break;
	    case 'p':	chkobj = target;		break;
	    default:
		sprintf(rval, "Bad argument '%c' to '%s'", cvar[0], chck);
		progbug(rval, mob);
		return BERR;
	}
	if ( !chkchar && !chkobj )
	    return BERR;
    }
    if ( !str_cmp(chck, "rand") )
    {
	return (number_percent() <= atoi(cvar));
    }
    if ( !str_cmp(chck, "economy") )
    {
	int idx = atoi(cvar);
	ROOM_INDEX_DATA *room;
    
	if ( !idx )
	{
	    if ( !mob->in_room )
	    {
		progbug( "'economy' ifcheck: mob in NULL room with no room vnum "
		    "argument", mob );
		return BERR;
	    }
	    room = mob->in_room;
	}
	else
	    room = get_room_index(idx);
	if ( !room )
	{
	    progbug( "Bad room vnum passed to 'economy'", mob );
	    return BERR;
	}
	return mprog_veval( ((room->area->high_economy > 0) ? 1000000000 : 0)
	    + room->area->low_economy, opr, atoi(rval), mob );
    }
    if (!str_cmp(chck, "mobinarea"))
    {
	int vnum = atoi(cvar);
	int lhsvl;
	int world_count;
	int found_count;
	CHAR_DATA *tmob;
	MOB_INDEX_DATA *m_index;
  	
	if (vnum < 1 || vnum > MAX_VNUM )
	{
	    progbug("Bad vnum to 'mobinarea'", mob);
	    return BERR;
	}
  	
	m_index = get_mob_index(vnum);
  	
	if(!m_index)
	    world_count = 0;
	else
	    world_count = m_index->count;
  	
	lhsvl = 0;
	found_count = 0;
  	
	for(tmob = first_char; tmob && found_count != world_count;
	    tmob = tmob->next)
	{
	    if(IS_NPC(tmob) && tmob->pIndexData->vnum == vnum)
	    {
		found_count++;
  			
		if(tmob->in_room->area == mob->in_room->area)
		    lhsvl++;
	    }
	}
	rhsvl = atoi(rval);

	/* Changed below from 1 to 0 */
	if(rhsvl < 0)
	    rhsvl = 0;
	if(!*opr)
	    strcpy(opr, "==");
  	
	return mprog_veval(lhsvl, opr, rhsvl, mob);
    }

    if ( !str_cmp(chck, "mobinroom") )
    {
	int vnum = atoi(cvar);
	int lhsvl;
	CHAR_DATA *oMob;
    
	if ( vnum < 1 || vnum > MAX_VNUM )
	{
	    progbug( "Bad vnum to 'mobinroom'", mob );
	    return BERR;
	}
	lhsvl = 0;
	for ( oMob = mob->in_room->first_person; oMob;
	    oMob = oMob->next_in_room )
	if ( IS_NPC(oMob) && oMob->pIndexData->vnum == vnum )
	    lhsvl++;
	rhsvl = atoi(rval);
	/* Changed below from 1 to 0 */
	if ( rhsvl < 0 ) rhsvl = 0;
	    if ( !*opr )
		strcpy( opr, "==" );
	return mprog_veval(lhsvl, opr, rhsvl, mob);
    }

    if(!str_cmp(chck, "mobinworld"))
    {
	int vnum = atoi(cvar);
	int lhsvl;
	MOB_INDEX_DATA *m_index;
  	
	if(vnum < 1 || vnum > MAX_VNUM)
	{
	    progbug("Bad vnum to 'mobinworld'", mob);
	    return BERR;
	}
  	
	m_index = get_mob_index(vnum);
	
	if(!m_index)
	    lhsvl = 0;
	else
	    lhsvl = m_index->count;
	
	rhsvl = atoi(rval);
	/* Changed below from 1 to 0 */
	
	if(rhsvl < 0)
	    rhsvl = 0;
	if(!*opr)
	    strcpy(opr, "==");
	
	return mprog_veval(lhsvl, opr, rhsvl, mob);
    }
    if ( !str_cmp(chck, "timeskilled") )
    {
	MOB_INDEX_DATA *pMob;
    
	if ( chkchar )
	    pMob = chkchar->pIndexData;
	else if ( !(pMob = get_mob_index(atoi(cvar))) )
	{
	    progbug("TimesKilled ifcheck: bad vnum", mob);
	    return BERR;
	}
	return mprog_veval(pMob->killed, opr, atoi(rval), mob);
    }

    if(!str_cmp(chck, "objinworld"))
    {
        int vnum = atoi(cvar);
	int lhsvl;
        OBJ_INDEX_DATA *p_index;

        if(vnum < 1 || vnum > MAX_VNUM)
        {
            progbug("Bad vnum to 'objinworld'", mob);
            return BERR;
        }	

        p_index = (get_obj_index(vnum));

        if( !p_index )
            lhsvl = 0;
        else
            lhsvl = p_index->count;

        rhsvl = atoi(rval);

        if(rhsvl < 0)
            rhsvl = 0;
        if(!*opr)
            strcpy(opr, "==");
        return mprog_veval(lhsvl, opr, rhsvl, mob);
    }

    if ( !str_cmp(chck, "ovnumhere") )
    {
	OBJ_DATA *pObj;
	int vnum = atoi(cvar);
    
	if ( vnum < 1 || vnum > MAX_VNUM )
	{
	    progbug("OvnumHere: bad vnum", mob);
	    return BERR;
	}
	lhsvl = 0;

	for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
	    if ( pObj->pIndexData->vnum == vnum )
		lhsvl+=pObj->count;
	for ( pObj = mob->in_room->first_content; pObj; pObj = pObj->next_content )
	    if ( pObj->pIndexData->vnum == vnum )
		lhsvl+=pObj->count;
	rhsvl = is_number(rval) ? atoi(rval) : -1;
	/* Changed from 1 to 0 */
	if ( rhsvl < 0 )
	    rhsvl = 0;
	if ( !*opr )
	    strcpy(opr, "==");
	return mprog_veval(lhsvl, opr, rhsvl, mob);
    }
    if ( !str_cmp(chck, "otypehere") )
    {
	OBJ_DATA *pObj;
	int type;
    
	if ( is_number(cvar) )
	    type = atoi(cvar);
	else
	    type = get_otype(cvar);
	if ( type < 0 || type > MAX_ITEM_TYPE )
	{
	    progbug("OtypeHere: bad type", mob);
	    return BERR;
	}
	lhsvl = 0;

	for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
	    if ( pObj->item_type == type )
		lhsvl+=pObj->count;
	for ( pObj = mob->in_room->first_content; pObj;
	      pObj = pObj->next_content )
	    if ( pObj->item_type == type )
		lhsvl+=pObj->count;
	rhsvl = is_number(rval) ? atoi(rval) : -1;
	/* Change below from 1 to 0 */
	if ( rhsvl < 0 )
	    rhsvl = 0;
	if ( !*opr )
	    strcpy(opr, "==");
	return mprog_veval(lhsvl, opr, rhsvl, mob);
    }
    if ( !str_cmp(chck, "ovnumroom") )
    {
	OBJ_DATA *pObj;
	int vnum = atoi(cvar);
    
	if ( vnum < 1 || vnum > MAX_VNUM )
	{
	    progbug("OvnumRoom: bad vnum", mob);
	    return BERR;
	}
	lhsvl = 0;
	for ( pObj = mob->in_room->first_content; pObj;
	    pObj = pObj->next_content )
	if ( pObj->pIndexData->vnum == vnum )
	    lhsvl+=pObj->count;
	rhsvl = is_number(rval) ? atoi(rval) : -1;
	/* Changed below from 1 to 0 so can check for == no items Shaddai */
	if ( rhsvl < 0 )
	    rhsvl = 0;
	if ( !*opr )
	    strcpy(opr, "==");
	return mprog_veval(lhsvl, opr, rhsvl, mob);
    }
    if ( !str_cmp(chck, "otyperoom") )
    {
	OBJ_DATA *pObj;
	int type;
    
	if ( is_number(cvar) )
	    type = atoi(cvar);
	else
	    type = get_otype(cvar);
	if ( type < 0 || type > MAX_ITEM_TYPE )
	{
	    progbug("OtypeRoom: bad type", mob);
	    return BERR;
	}
	lhsvl = 0;

	for ( pObj = mob->in_room->first_content; pObj;
	    pObj = pObj->next_content )
	if ( pObj->item_type == type )
	    lhsvl+=pObj->count;
	rhsvl = is_number(rval) ? atoi(rval) : -1;
	/* Changed below from 1 to 0 */
	if ( rhsvl < 0 )
	    rhsvl = 0;
	if ( !*opr )
	    strcpy(opr, "==");
	return mprog_veval(lhsvl, opr, rhsvl, mob);
    }
    if ( !str_cmp(chck, "ovnumcarry") )
    {
	OBJ_DATA *pObj;
	int vnum = atoi(cvar);
    
	if ( vnum < 1 || vnum > MAX_VNUM )
	{
	    progbug("OvnumCarry: bad vnum", mob);
	    return BERR;
	}
	lhsvl = 0;

	for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
	    if ( pObj->pIndexData->vnum == vnum )
		lhsvl+=pObj->count;
	rhsvl = is_number(rval) ? atoi(rval) : -1;

	if ( rhsvl < 0 )
	    rhsvl = 0;
	if ( !*opr )
	    strcpy(opr, "==");
	return mprog_veval(lhsvl, opr, rhsvl, mob);
    }
    if ( !str_cmp(chck, "otypecarry") )
    {
	OBJ_DATA *pObj;
	int type;
    
	if ( is_number(cvar) )
	    type = atoi(cvar);
	else
	    type = get_otype(cvar);
	if ( type < 0 || type > MAX_ITEM_TYPE )
	{
	    progbug("OtypeCarry: bad type", mob);
	    return BERR;
	}
	lhsvl = 0;
	for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
	    if ( pObj->item_type == type )
		lhsvl+=pObj->count;
	rhsvl = is_number(rval) ? atoi(rval) : -1;
	/* Changed below from 1 to 0 Shaddai */
	if ( rhsvl < 0 )
	    rhsvl = 0;
	if ( !*opr )
	    strcpy(opr, "==");
	return mprog_veval(lhsvl, opr, rhsvl, mob);
    }
    if ( !str_cmp(chck, "ovnumwear") )
    {
	OBJ_DATA *pObj;
	int vnum = atoi(cvar);
    
	if ( vnum < 1 || vnum > MAX_VNUM )
	{
	    progbug("OvnumWear: bad vnum", mob);
	    return BERR;
	}
	lhsvl = 0;
	for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
	    if ( pObj->wear_loc != WEAR_NONE &&
		pObj->pIndexData->vnum == vnum )
	lhsvl+=pObj->count;
	rhsvl = is_number(rval) ? atoi(rval) : -1;
	/* Changed below from 1 to 0 */
	if ( rhsvl < 0 )
	    rhsvl = 0;
	if ( !*opr )
	    strcpy(opr, "==");
	return mprog_veval(lhsvl, opr, rhsvl, mob);
    }
    if ( !str_cmp(chck, "otypewear") )
    {
	OBJ_DATA *pObj;
	int type;
    
	if ( is_number(cvar) )
	    type = atoi(cvar);
	else
	    type = get_otype(cvar);
	if ( type < 0 || type > MAX_ITEM_TYPE )
	{
	    progbug("OtypeWear: bad type", mob);
	    return BERR;
	}
	lhsvl = 0;
	for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
	    if ( pObj->wear_loc != WEAR_NONE && 
		pObj->item_type == type )
	lhsvl+=pObj->count;
	rhsvl = is_number(rval) ? atoi(rval) : -1;
	/* Changed below from 1 to 0 so can have == 0 Shaddai */
	if ( rhsvl < 0 )
	    rhsvl = 0;
	if ( !*opr )
	    strcpy(opr, "==");
	return mprog_veval(lhsvl, opr, rhsvl, mob);
    }
    if ( !str_cmp(chck, "ovnuminv") )
    {
	OBJ_DATA *pObj;
	int vnum = atoi(cvar);
    
	if ( vnum < 1 || vnum > MAX_VNUM )
	{
	    progbug("OvnumInv: bad vnum", mob);
	    return BERR;
	}
	lhsvl = 0;

	for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
	    if ( pObj->wear_loc == WEAR_NONE
	    &&   pObj->pIndexData->vnum == vnum )
		lhsvl+=pObj->count;
	rhsvl = is_number(rval) ? atoi(rval) : -1;
	/* Changed 1 to 0 so can have == 0 */
	if ( rhsvl < 0 )
	    rhsvl = 0;
	if ( !*opr )
	    strcpy(opr, "==");
	return mprog_veval(lhsvl, opr, rhsvl, mob);
    }
    if ( !str_cmp(chck, "otypeinv") )
    {
	OBJ_DATA *pObj;
	int type;
    
	if ( is_number(cvar) )
	    type = atoi(cvar);
	else
	    type = get_otype(cvar);
	if ( type < 0 || type > MAX_ITEM_TYPE )
	{
	    progbug("OtypeInv: bad type", mob);
	    return BERR;
	}
	lhsvl = 0;
	for ( pObj = mob->first_carrying; pObj; pObj = pObj->next_content )
	    if ( pObj->wear_loc == WEAR_NONE
	    &&   pObj->item_type == type )
		lhsvl+=pObj->count;
	rhsvl = is_number(rval) ? atoi(rval) : -1;
	/* Changed below from 1 to 0 for == 0 Shaddai */
	if ( rhsvl < 0 )
	    rhsvl = 0;
	if ( !*opr )
	    strcpy(opr, "==");
	return mprog_veval(lhsvl, opr, rhsvl, mob);
    }
    if ( chkchar )
    {
	if ( !str_cmp(chck, "ispacifist") )
	{
	    return (IS_NPC(chkchar) && xIS_SET(chkchar->act, ACT_PACIFIST));
	}
	if ( !str_cmp(chck, "stopscript") )
	{
	   return (IS_NPC(chkchar) && xIS_SET(chkchar->act, ACT_STOP_SCRIPT));
	}
	if ( !str_cmp(chck, "ismobinvis") )
	{
	    return (IS_NPC(chkchar) && xIS_SET(chkchar->act, ACT_MOBINVIS));
	}
	if ( !str_cmp(chck, "mobinvislevel") )
	{
	    return (IS_NPC(chkchar) ?
		mprog_veval(chkchar->mobinvis, opr, atoi(rval), mob) : FALSE);
	}
	if ( !str_cmp(chck, "drunk") )
	{
	    return (!IS_NPC(chkchar) ?
		mprog_veval(chkchar->pcdata->condition[COND_DRUNK], opr, atoi(rval), mob) : FALSE);
	}
        if ( !str_cmp(chck, "darrek") ) /* Teehee -- Blod */
        {
            return (!IS_NPC(chkchar) ?
                mprog_veval(chkchar->pcdata->condition[COND_DRUNK], opr, atoi(rval), mob) : FALSE);
        }
	if ( !str_cmp(chck, "ispc") )
	{
	    return IS_NPC(chkchar) ? FALSE : TRUE;
	}
	if ( !str_cmp(chck, "isnpc") )
	{
	    return IS_NPC(chkchar) ? TRUE : FALSE;
	}
	if ( !str_cmp(chck, "cansee") )
	{
	    return can_see( mob , chkchar );
	}
        if ( !str_cmp(chck, "isriding") )
        {
            if ( chkchar->mount == mob )
                return TRUE;
            else
                return FALSE;
        }   
	if ( !str_cmp(chck, "ispassage") )
	{
	    if ( find_door( chkchar, rval , TRUE ) == NULL )
		return FALSE;
	    else
		return TRUE;
	}
	if ( !str_cmp(chck, "isopen" ) )
	{
	    EXIT_DATA *pexit;

	    if ( (pexit = find_door( chkchar, rval, TRUE)) == NULL )
		return FALSE;
	    if (!IS_SET (pexit->exit_info, EX_CLOSED))
		return TRUE;
	    return FALSE;
	}
	if ( !str_cmp(chck, "islocked" ) )
	{
	    EXIT_DATA *pexit;

	    if ( (pexit = find_door( chkchar, rval, TRUE)) == NULL )
		return FALSE;
	    if (IS_SET (pexit->exit_info, EX_LOCKED))
		return TRUE;
	    return FALSE;
	}
	if ( !str_cmp(chck, "ispkill") )
	{
	    return IS_PKILL(chkchar) ? TRUE : FALSE;
	}
	if ( !str_cmp(chck, "isdevoted") )
	{
	    return IS_DEVOTED(chkchar) ? TRUE : FALSE;
	}
	if ( !str_cmp(chck, "canpkill") )
	{
	    return CAN_PKILL(chkchar) ? TRUE : FALSE;
	}
	if ( !str_cmp(chck, "inarena") )
	{
	    return in_arena(chkchar) ? TRUE : FALSE;
	}
	if ( !str_cmp(chck, "ismounted") )
	{
	    return (chkchar->position == POS_MOUNTED);
	}
	if ( !str_cmp(chck, "ismorphed") )
	{
	    return (chkchar->morph != NULL) ? TRUE : FALSE;
	}
	if ( !str_cmp(chck, "isnuisance" ) )
	{
	    return (!IS_NPC(chkchar)? chkchar->pcdata->nuisance? TRUE: FALSE: FALSE);
	}
	if ( !str_cmp(chck, "isgood") )
	{
	    return IS_GOOD(chkchar) ? TRUE : FALSE;
	}
	if ( !str_cmp(chck, "isneutral") )
	{
	    return IS_NEUTRAL(chkchar) ? TRUE : FALSE;
	}
	if ( !str_cmp(chck, "isevil") )
	{
	    return IS_EVIL(chkchar) ? TRUE : FALSE;
	}
	if ( !str_cmp(chck, "isfight") )
	{
	    return who_fighting(chkchar) ? TRUE : FALSE;
	}
	if ( !str_cmp(chck, "isimmort") )
	{
	    return (get_trust(chkchar) >= LEVEL_IMMORTAL);
	}
	if ( !str_cmp(chck, "ischarmed") )
	{
            if ( IS_AFFECTED(chkchar, AFF_CHARM) || 
		 IS_AFFECTED(chkchar, AFF_POSSESS) )
		return TRUE;
            else 
		return FALSE;
	}
        if ( !str_cmp(chck, "ispossesed") )
	{
	    return IS_AFFECTED(chkchar, AFF_POSSESS) ? TRUE : FALSE;
        }
	if ( !str_cmp(chck, "isflying") )
	{
	    return IS_AFFECTED(chkchar, AFF_FLYING) ? TRUE : FALSE;
	}
	if ( !str_cmp(chck, "isthief") )
	{
	    return ( !IS_NPC(chkchar) && xIS_SET(chkchar->act, PLR_THIEF) );
	}
	if ( !str_cmp(chck, "isattacker") )
	{
	    return ( !IS_NPC(chkchar) && xIS_SET(chkchar->act, PLR_ATTACKER) );
	}
	if ( !str_cmp(chck, "iskiller") )
	{
	    return ( !IS_NPC(chkchar) && xIS_SET(chkchar->act, PLR_KILLER) );
	}
	if ( !str_cmp(chck, "isfollow") )
	{
	    return (chkchar->master != NULL
	         && chkchar->master->in_room == chkchar->in_room);
	}
	if ( !str_cmp(chck, "isaffected") )
	{
	    int value = get_aflag(rval);
      
	    if ( value < 0 || value > MAX_BITS )
	    {
		progbug("Unknown affect being checked", mob);
		return BERR;
	    }
	    return IS_AFFECTED(chkchar, value) ? TRUE : FALSE;
	}
	if ( !str_cmp(chck, "numfighting") )
	{
	    return mprog_veval(chkchar->num_fighting-1, opr, atoi(rval), mob );
	}
	if ( !str_cmp(chck, "hitprcnt") )
	{
	    return mprog_veval( ( chkchar->hit * 100 ) / chkchar->max_hit, opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "inroom") )
	{
	    return mprog_veval(chkchar->in_room->vnum, opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "wasinroom") )
	{
	    if ( !chkchar->was_in_room )
		return FALSE;
	    return mprog_veval(chkchar->was_in_room->vnum, opr, atoi(rval), mob);
	}
        if ( !str_cmp(chck, "indoors") )
        {
                return ( ( IS_OUTSIDE( chkchar ) && chkchar->in_room->sector_type != SECT_INSIDE ) ? FALSE : TRUE );
        }
        if ( !str_cmp(chck, "nomagic") )
        {
            return xIS_SET(chkchar->in_room->room_flags, ROOM_NO_MAGIC) ?  TRUE : FALSE;
        }
        if ( !str_cmp(chck, "safe") )
        {
            return xIS_SET(chkchar->in_room->room_flags, ROOM_SAFE) ?  TRUE : FALSE;
        }

        if ( !str_cmp(chck, "nosummon") )
        {
            return xIS_SET(chkchar->in_room->room_flags, ROOM_NO_SUMMON) ?  TRUE : FALSE;
        }
        if ( !str_cmp(chck, "noastral") )
        {
            return xIS_SET(chkchar->in_room->room_flags, ROOM_NO_ASTRAL) ?  TRUE : FALSE;
        }
        if ( !str_cmp(chck, "nosupplicate") )
        {
            return xIS_SET(chkchar->in_room->room_flags, ROOM_NOSUPPLICATE) ?  TRUE : FALSE;
        }
	if ( !str_cmp(chck, "norecall") )
	{
	    return xIS_SET(chkchar->in_room->room_flags, ROOM_NO_RECALL) ? TRUE : FALSE;
	}
	if ( !str_cmp(chck, "sex") )
	{
	    return mprog_veval(chkchar->sex, opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "position") )
	{
	    return mprog_veval(chkchar->position, opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "doingquest") )
	{
	    return IS_NPC(chkchar) ? FALSE
	      : mprog_veval(chkchar->pcdata->quest_number, opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "ishelled") )
	{
	    return IS_NPC(chkchar) ? FALSE
	       : mprog_veval(chkchar->pcdata->release_date, opr, atoi(rval), mob);  
	}

	if ( !str_cmp(chck, "level") )
	{
	    return mprog_veval(get_trust(chkchar), opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "goldamt") )
	{
	    return mprog_veval(chkchar->gold, opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "class") )
	{
	    if ( IS_NPC(chkchar) )
		return mprog_seval(npc_class[chkchar->class], opr, rval, mob);
	    return mprog_seval((char *)class_table[chkchar->class]->who_name,
		opr, rval, mob);
	}
	if ( !str_cmp(chck, "weight" ) )
	{
	    return mprog_veval(chkchar->carry_weight, opr, atoi(rval), mob );
	}
	if ( !str_cmp(chck, "hostdesc") )
	{
	    if ( IS_NPC(chkchar) || !chkchar->desc->host )
		return FALSE;
	    return mprog_seval(chkchar->desc->host, opr, rval, mob);
	}
        if ( !str_cmp(chck, "areamulti") )
        {
            CHAR_DATA *ch;
            int lhsvl = 0;

            for ( ch = first_char; ch; ch = ch->next )
                if ( !IS_NPC( chkchar ) && !IS_NPC( ch )
		&&    ch->in_room
                &&    chkchar->in_room
		&&    ch->in_room->area == chkchar->in_room->area 
                &&    ch->desc
                &&    chkchar->desc
                &&    QUICKMATCH(ch->desc->host, chkchar->desc->host) )
                        lhsvl++;
            rhsvl = atoi(rval);
            if ( rhsvl < 0 ) rhsvl = 0;
            if ( !*opr ) strcpy( opr, "==" );
            return mprog_veval(lhsvl, opr, rhsvl, mob);
        }
	if ( !str_cmp(chck, "multi") )
	{
	    CHAR_DATA *ch;
	    int lhsvl = 0;
 
	    for ( ch = first_char; ch; ch = ch->next )
		if ( !IS_NPC( chkchar ) && !IS_NPC( ch )
		&&    ch->desc
		&&    chkchar->desc
		&&    QUICKMATCH(ch->desc->host, chkchar->desc->host) )
			lhsvl++;
	    rhsvl = atoi(rval);
	    if ( rhsvl < 0 ) rhsvl = 0;
	    if ( !*opr ) strcpy( opr, "==" );
	    return mprog_veval(lhsvl, opr, rhsvl, mob);
	}
	if ( !str_cmp(chck, "race") )
	{
	    if ( IS_NPC(chkchar) )
		return mprog_seval(npc_race[chkchar->race], opr, rval, mob);
	    return mprog_seval((char *)race_table[chkchar->race]->race_name,
		opr, rval, mob);
	}
	if ( !str_cmp(chck, "morph" ) )
	{
	    if ( chkchar->morph == NULL )
		return FALSE;
	    if ( chkchar->morph->morph == NULL )
		return FALSE;
	    return mprog_veval(chkchar->morph->morph->vnum, opr, rhsvl, mob );
	}
	if ( !str_cmp(chck, "nuisance") )
	{
	    if ( IS_NPC( chkchar ) || !chkchar->pcdata->nuisance )
		return FALSE;
	    return mprog_veval(chkchar->pcdata->nuisance->flags, opr, rhsvl, mob );
	}
	if ( !str_cmp(chck, "clan") )
	{
	    if ( IS_NPC(chkchar) || !chkchar->pcdata->clan )
		return FALSE;
	    return mprog_seval(chkchar->pcdata->clan->name, opr, rval, mob);
	}
	/* Check added to see if the person isleader of == clan Shaddai */
	if (!str_cmp (chck, "isleader"))
	{
	    CLAN_DATA *temp;
	    if ( IS_NPC ( chkchar ) )
		return FALSE;
	    if ( (temp = get_clan( rval )) == NULL )
		return FALSE;
	    if ( mprog_seval(chkchar->name, opr, temp->leader, mob)
	    ||	 mprog_seval(chkchar->name, opr, temp->number1, mob)
	    ||   mprog_seval(chkchar->name, opr, temp->number2, mob) )
		return TRUE;
	    else
		return FALSE;
	}

	/* Is char wearing some eq on a specific wear loc?  -- Gorog */
	if (!str_cmp (chck, "wearing"))
	{
	    OBJ_DATA *obj;
	    int i=0;
	    for (obj=chkchar->first_carrying; obj; obj=obj->next_content)
	    {
		i++;
/*
		if ( chkchar==obj->carried_by )
		    pager_printf(chkchar, "count=%d obj name=%s\n\r", i, obj->name);
*/
		if ( chkchar==obj->carried_by
		&&   obj->wear_loc > -1 
		&&  !str_cmp(rval, item_w_flags[obj->wear_loc]) )
		    return TRUE;
	    }
	    return FALSE;
	}
	/* Is char wearing some specific vnum?  -- Gorog */
	if (!str_cmp (chck, "wearingvnum"))
	{
	    OBJ_DATA *obj;

	    if ( !is_number(rval) )
		return FALSE;
	    for (obj=chkchar->first_carrying; obj; obj=obj->next_content)
	    {
		if ( chkchar==obj->carried_by
		&&   obj->wear_loc > -1 
		&&   obj->pIndexData->vnum == atoi(rval) )
		    return TRUE;
	    }
	    return FALSE;
	}
		
	/* Is char carrying a specific piece of eq?  -- Gorog */
	if (!str_cmp (chck, "carryingvnum"))
	{
	    int vnum;

	    if ( !is_number(rval) )
		return FALSE;
	    vnum = atoi(rval);
	    if ( !chkchar->first_carrying )
		return FALSE;
	    return (carryingvnum_visit(chkchar, chkchar->first_carrying, vnum));
	}
		
	/* Check added to see if the person isleader of == clan Gorog */
	if (!str_cmp (chck, "isclanleader"))
	{
	    CLAN_DATA *temp;
	    if ( IS_NPC ( chkchar ) )
		return FALSE;
	    if ( (temp = get_clan( rval )) == NULL )
		return FALSE;
	    if ( mprog_seval(chkchar->name, opr, temp->leader, mob ) )
		return TRUE;
	    else
		return FALSE;
	}
	if (!str_cmp (chck, "isclan1"))
	{
	    CLAN_DATA *temp;
	    if ( IS_NPC ( chkchar ) )
		return FALSE;
	    if ( (temp = get_clan( rval )) == NULL )
		return FALSE;
	    if ( mprog_seval(chkchar->name, opr, temp->number1, mob ) )
		return TRUE;
	    else
		return FALSE;
	}
	if (!str_cmp (chck, "isclan2"))
	{
	    CLAN_DATA *temp;
	    if ( IS_NPC ( chkchar ) )
		return FALSE;
	    if ( (temp = get_clan( rval )) == NULL )
		return FALSE;
	    if ( mprog_seval(chkchar->name, opr, temp->number2, mob ) )
		return TRUE;
	    else
		return FALSE;
	}
	if ( !str_cmp(chck, "council") )
	{
	    if ( IS_NPC(chkchar) || !chkchar->pcdata->council )
		return FALSE;
	    return mprog_seval(chkchar->pcdata->council->name, opr, rval, mob);
	}
	if ( !str_cmp(chck, "deity") )
	{
	    if (IS_NPC(chkchar) || !chkchar->pcdata->deity )
		return FALSE;
	    return mprog_seval(chkchar->pcdata->deity->name, opr, rval, mob);
	}
	if ( !str_cmp(chck, "guild") )
	{
	    if ( IS_NPC(chkchar) || !IS_GUILDED(chkchar) )
		return FALSE;
	    return mprog_seval(chkchar->pcdata->clan->name, opr, rval, mob);
	}
	if ( !str_cmp(chck, "clantype") )
	{
	    if ( IS_NPC(chkchar) || !chkchar->pcdata->clan )
		return FALSE;
	    return mprog_veval(chkchar->pcdata->clan->clan_type, opr, atoi(rval), mob);
	}

    if ( !str_cmp(chck, "isflagged") )
    {
        VARIABLE_DATA *vd;
        int vnum = mob->pIndexData->vnum;
        int flag = 0;
        char *p;

        if ( argc < 3 )
        {
        return BERR;
        }
        if ( argc > 3 )
        flag = atoi(argv[3]);
        if ( (p=strchr(argv[2], ':')) != NULL )
        {
        *p++ = '\0';
        vnum = atoi(p);
        }
        if ( (vd=get_tag(chkchar, argv[2], vnum)) == NULL )
        return FALSE;

        flag = abs(flag) % MAX_BITS;
        switch(vd->type)
        {
        case vtSTR:
        case vtINT:
            return FALSE;
        case vtXBIT:
            return xIS_SET(*(EXT_BV*)vd->data, flag) ? TRUE : FALSE;
        }
        return FALSE;
    }

    if ( !str_cmp(chck, "istagged") )
    {
        VARIABLE_DATA *vd;
        int vnum = mob->pIndexData->vnum;
        char *p;

        if ( argc < 3 )
        {
        return BERR;
        }
        if ( argc > 3 )
        vnum = atoi(argv[3]);
        if ( (p=strchr(argv[2], ':')) != NULL )
        {
        *p++ = '\0';
        vnum = atoi(p);
        }
        if ( (vd=get_tag(chkchar, argv[2], vnum)) == NULL )
        return FALSE;

        if ( !*opr && !*rval )
        return TRUE;
        switch(vd->type)
        {
        case vtSTR:
            return mprog_seval(vd->data, opr, rval, mob);
        case vtINT:
            return mprog_veval((int)vd->data, opr, atoi(rval), mob);
        case vtXBIT:
            return FALSE;   /* for now */
        }
        return FALSE;
    }


	if ( !str_cmp(chck, "waitstate") )
	{
	    if ( IS_NPC( chkchar ) || !chkchar->wait )
		return FALSE;
	    return mprog_veval(chkchar->wait, opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "pkadrenalized") )
	{
	    return mprog_veval( get_timer( chkchar, TIMER_RECENTFIGHT ), opr, atoi(rval), mob );
	}
	if ( !str_cmp(chck, "asupressed") )
	{
	    return mprog_veval( get_timer( chkchar, TIMER_ASUPRESSED ), opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "favor") )
	{
	    if ( IS_NPC(chkchar) || !chkchar->pcdata->favor )
		return FALSE;
	    return mprog_veval(chkchar->pcdata->favor, opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "hps") )
	{
	    return mprog_veval(chkchar->hit, opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "mana") )
	{
	    return mprog_veval(chkchar->mana, opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "str") )
	{
	    return mprog_veval(get_curr_str(chkchar), opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "wis") )
	{
	    return mprog_veval(get_curr_wis(chkchar), opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "int") )
	{
	    return mprog_veval(get_curr_int(chkchar), opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "dex") )
	{
	    return mprog_veval(get_curr_dex(chkchar), opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "con") )
	{
	    return mprog_veval(get_curr_con(chkchar), opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "cha") )
	{
	    return mprog_veval(get_curr_cha(chkchar), opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "lck") )
	{
	    return mprog_veval(get_curr_lck(chkchar), opr, atoi(rval), mob);
	}
    }
    if ( chkobj )
    {
	if ( !str_cmp(chck, "objtype") )
	{
	    return mprog_veval(chkobj->item_type, opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "leverpos") )
	{
	    int isup = FALSE, wantsup=FALSE;
	    if ( chkobj->item_type != ITEM_SWITCH || chkobj->item_type != ITEM_LEVER
	    || chkobj->item_type != ITEM_PULLCHAIN )
		return FALSE;

	    if ( IS_SET( obj->value[0], TRIG_UP ) )
		isup = TRUE;
	    if ( !str_cmp( rval, "up" ) )
		wantsup = TRUE;
	    return mprog_veval( wantsup, opr, isup, mob );
	}
	if ( !str_cmp(chck, "objval0") )
	{
	    return mprog_veval(chkobj->value[0], opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "objval1") )
	{
	    return mprog_veval(chkobj->value[1], opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "objval2") )
	{
	    return mprog_veval(chkobj->value[2], opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "objval3") )
	{
	    return mprog_veval(chkobj->value[3], opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "objval4") )
	{
	    return mprog_veval(chkobj->value[4], opr, atoi(rval), mob);
	}
	if ( !str_cmp(chck, "objval5") )
	{
	    return mprog_veval(chkobj->value[5], opr, atoi(rval), mob);
	}
    }
    /*
     * The following checks depend on the fact that cval[1] can only contain
     * one character, and that NULL checks were made previously.
     */
    if ( !str_cmp(chck, "number") )
    {
	if ( chkchar )
	{
	    if ( !IS_NPC(chkchar) )
		return FALSE;
	    lhsvl = (chkchar == mob) ? chkchar->gold : chkchar->pIndexData->vnum;
	    return mprog_veval(lhsvl, opr, atoi(rval), mob);
	}
	return mprog_veval(chkobj->pIndexData->vnum, opr, atoi(rval), mob);
    }
    if ( !str_cmp(chck, "time") )
    {
	return mprog_veval(time_info.hour, opr, atoi(rval), mob );
    }
    if ( !str_cmp(chck, "name") )
    {
	if ( chkchar )
		return mprog_seval(chkchar->name, opr, rval, mob);
	return mprog_seval(chkobj->name, opr, rval, mob);
    }

    if ( !str_cmp(chck, "rank") ) /* Shaddai */
    {
	if ( chkchar && !IS_NPC( chkchar ) )
	    return mprog_seval(chkchar->pcdata->rank, opr, rval, mob);
	return FALSE;
    }

    if ( !str_cmp(chck, "mortinworld") )   /* -- Gorog */
    {
	DESCRIPTOR_DATA *d; 
	for ( d = first_descriptor; d; d = d->next ) 
	    if   ( d->connected == CON_PLAYING
	    &&     d->character
	    &&     get_trust(d->character) < LEVEL_IMMORTAL
	    &&     nifty_is_name(d->character->name, cvar) )
		return TRUE;
	return FALSE;
    }

    if ( !str_cmp(chck, "mortinroom") )   /* -- Gorog */
    {
	CHAR_DATA *ch;
	for ( ch = mob->in_room->first_person; ch; ch = ch->next_in_room )
	    if ( (!IS_NPC(ch))
	    &&   get_trust(ch) < LEVEL_IMMORTAL
	    &&   nifty_is_name(ch->name, cvar) )
		return TRUE;
	return FALSE;
    }

    if ( !str_cmp( chck, "inarea" ) )
    {
        if ( chkchar )
                return mprog_seval( chkchar->in_room->area->filename,  opr, rval, mob );
        return FALSE;
    }

    if ( !str_cmp(chck, "mortinarea") )   /* -- Gorog */
    {
	CHAR_DATA *ch;
	for ( ch = first_char; ch; ch = ch->next )
	    if ( (!IS_NPC(ch))
	    &&   ch->in_room->area == mob->in_room->area
	    &&   get_trust(ch) < LEVEL_IMMORTAL
	    &&   nifty_is_name(ch->name, cvar) )
		return TRUE;
	return FALSE;
    }


    if ( !str_cmp(chck, "mortcount") )   /* -- Gorog */
    {
	CHAR_DATA *tch;
	ROOM_INDEX_DATA *room;
	int count = 0;
	int rvnum = atoi( cvar );

	room = get_room_index ( rvnum ? rvnum : mob->in_room->vnum );    

	for ( tch = room?room->first_person:NULL; tch; tch = tch->next_in_room )
	    if ( (!IS_NPC(tch))
	    &&   get_trust(tch) < LEVEL_IMMORTAL )
		count++;
	return mprog_veval(count, opr, atoi(rval), mob);
    }


    if ( !str_cmp(chck, "mobcount") )   /* -- Gorog */
    {
	CHAR_DATA *tch;
	ROOM_INDEX_DATA *room;
	int count = -1;
	int rvnum = atoi( cvar );

	room = get_room_index ( rvnum ? rvnum : mob->in_room->vnum );    

	for ( tch = room?room->first_person:NULL; tch; tch = tch->next_in_room )
	    if ( (IS_NPC(tch)) )
		count++;
	return mprog_veval(count, opr, atoi(rval), mob);
    }


    if ( !str_cmp(chck, "charcount") )   /* -- Gorog */
    {
	CHAR_DATA *tch;
	ROOM_INDEX_DATA *room;
	int count = -1;
	int rvnum = atoi( cvar );

	room = get_room_index ( rvnum ? rvnum : mob->in_room->vnum );    

	for ( tch = room?room->first_person:NULL; tch; tch = tch->next_in_room )

	if ( ((!IS_NPC(tch))
	&&      get_trust(tch) < LEVEL_IMMORTAL)
	||      IS_NPC(tch) )		/* mortal or mob */
	    count++;
	return mprog_veval(count, opr, atoi(rval), mob);
    }



    /*
     * Ok... all the ifchecks are done, so if we didnt find ours then something
     * odd happened.  So report the bug and abort the MUDprogram (return error)
     */
    progbug( "Unknown ifcheck", mob );
    return BERR;
}
#undef isoperator
#undef MAX_IF_ARGS

/* This routine handles the variables for command expansion.
 * If you want to add any go right ahead, it should be fairly
 * clear how it is done and they are quite easy to do, so you
 * can be as creative as you want. The only catch is to check
 * that your variables exist before you use them. At the moment,
 * using $t when the secondary target refers to an object 
 * i.e. >prog_act drops~<nl>if ispc($t)<nl>sigh<nl>endif<nl>~<nl>
 * probably makes the mud crash (vice versa as well) The cure
 * would be to change act() so that vo becomes vict & v_obj.
 * but this would require a lot of small changes all over the code.
 */

/*
 *  There's no reason to make the mud crash when a variable's
 *  fubared.  I added some ifs.  I'm willing to trade some 
 *  performance for stability. -Haus
 *
 *  Added char_died and obj_extracted checks	-Thoric
 */
void mprog_translate( char ch, char *t, CHAR_DATA *mob, CHAR_DATA *actor,
                    OBJ_DATA *obj, CHAR_DATA *vict, OBJ_DATA *v_obj, CHAR_DATA *rndm )
{
 static char *he_she        [] = { "it",  "he",  "she" };
 static char *him_her       [] = { "it",  "him", "her" };
 static char *his_her       [] = { "its", "his", "her" };

// This is all moot now since the void *vo is goooone yay!
// -- Alty
#if 0
/* Fix crash bug :)  SHADDAI
if ( v_obj && v_obj->serial )
  vict = NULL;
else
  v_obj = NULL; */

if ( vict && vict->max_hit )  /* max_hit being a stat that can never be 0 */
  v_obj = NULL;
else
  vict = NULL;
#endif

 *t = '\0';
 switch ( ch ) {
     case 'i':
	 if ( mob && !char_died(mob) )
	 {
	   if (mob->name)
              one_argument( mob->name, t );
         } else
	    strcpy( t, "someone" );
      break;

     case 'I':
	 if ( mob && !char_died(mob) )
	 {
	   if (mob->short_descr)
	   {
              strcpy( t, mob->short_descr );
           } else {
	      strcpy( t, "someone" );
	   }
         } else
	    strcpy( t, "someone" );
      break;

     case 'n':
         if ( actor && !char_died(actor) )
	 {
	   if ( can_see( mob,actor ) )
	     one_argument( actor->name, t );
           if ( !IS_NPC( actor ) )
  	     *t = UPPER( *t );
         }
	 else
	      strcpy( t, "someone" );
         break;

     case 'N':
         if ( actor && !char_died(actor) ) 
	 {
            if ( can_see( mob, actor ) )
	       if ( IS_NPC( actor ) )
		 strcpy( t, actor->short_descr );
	       else
	       {
		   strcpy( t, actor->name );
		   strcat( t, actor->pcdata->title );
	       }
	    else
	      strcpy( t, "someone" );
         } 
	 else
	      strcpy( t, "someone" );
	 break;

     case 't':
         if ( vict && !char_died(vict) )
	 {
	   if ( can_see( mob, vict ) )
	     one_argument( vict->name, t );
           if ( !IS_NPC( vict ) )
	     *t = UPPER( *t );
         } 
	 else 
	      strcpy( t, "someone" );

	 break;

     case 'T':
         if ( vict && !char_died(vict) ) 
	 {
            if ( can_see( mob, vict ) )
	       if ( IS_NPC( vict ) )
		 strcpy( t, vict->short_descr );
	       else
	       {
		 strcpy( t, vict->name );
//		 strcat( t, " " );
		 strcat( t, vict->pcdata->title );
	       }
	    else
	      strcpy( t, "someone" );
         }
	 else 
	      strcpy( t, "someone" );
	 break;
     
     case 'r':             
         if ( rndm && !char_died(rndm) )
	 {
	   if ( can_see( mob, rndm ) )
	   {
	     one_argument( rndm->name, t );
           }
           if ( !IS_NPC( rndm ) )
	   {
	     *t = UPPER( *t );
           }
	 }  
	 else
	   strcpy( t, "someone" );
      break;

     case 'R':
	 if ( rndm && !char_died(rndm) )
	 {
            if ( can_see( mob, rndm ) )
	       if ( IS_NPC( rndm ) )
		 strcpy(t,rndm->short_descr);
	       else
	       {
		 strcpy( t, rndm->name );
//		 strcat( t, " " );
		 strcat( t, rndm->pcdata->title );
	       }
	    else
	      strcpy( t, "someone" );
         }
	 else 
	      strcpy( t, "someone" );
	 break;

     case 'e':
         if ( actor && !char_died(actor) )
	 {
	   can_see( mob, actor ) ? strcpy( t, he_she[ actor->sex ] )
	                         : strcpy( t, "someone" );
         } 
	 else
	      strcpy( t, "it" );
	 break;
  
     case 'm':
         if ( actor && !char_died(actor) )
	 {
	   can_see( mob, actor ) ? strcpy( t, him_her[ actor->sex ] )
                                 : strcpy( t, "someone" );
         }
	 else
	      strcpy( t, "it" );
	 break;
  
     case 's':
         if ( actor && !char_died(actor) )
	 {
	   can_see( mob, actor ) ? strcpy( t, his_her[ actor->sex ] )
	                         : strcpy( t, "someone's" );
         }
	 else
	      strcpy( t, "its'" );
	 break;
     
     case 'E':
         if ( vict && !char_died(vict) )
	 {
	   can_see( mob, vict ) ? strcpy( t, he_she[ vict->sex ] )
                                : strcpy( t, "someone" );
         }
	 else
	      strcpy( t, "it" );
	 break;
  
     case 'M':
         if ( vict && !char_died(vict) )
	 {
	   can_see( mob, vict ) ? strcpy( t, him_her[ vict->sex ] )
                                : strcpy( t, "someone" );
         }
	 else
	      strcpy( t, "it" );
	 break;
  
     case 'S':
         if ( vict && !char_died(vict) )
	 {
	   can_see( mob, vict ) ? strcpy( t, his_her[ vict->sex ] )
                                : strcpy( t, "someone's" ); 
         }
	 else
	      strcpy( t, "its'" );
	 break;

     case 'j':
	 if (mob && !char_died(mob))
	 {
	    strcpy( t, he_she[ mob->sex ] );
         } else {
	    strcpy( t, "it" );
	 }
	 break;
  
     case 'k':
	 if( mob && !char_died(mob) )
	 {
	   strcpy( t, him_her[ mob->sex ] );
         } else {
	    strcpy( t, "it" );
	 }
	 break;
  
     case 'l':
	 if( mob && !char_died(mob) )
	 {
	   strcpy( t, his_her[ mob->sex ] );
         } else {
	    strcpy( t, "it" );
	 }
	 break;

     case 'J':
         if ( rndm && !char_died(rndm) )
	 {
	   can_see( mob, rndm ) ? strcpy( t, he_she[ rndm->sex ] )
	                        : strcpy( t, "someone" );
         }
	 else
	      strcpy( t, "it" );
	 break;
  
     case 'K':
         if ( rndm && !char_died(rndm) )
	 {
	   can_see( mob, rndm ) ? strcpy( t, him_her[ rndm->sex ] )
                                : strcpy( t, "someone's" );
         }
	 else
	      strcpy( t, "its'" );
	 break;
  
     case 'L':
         if ( rndm && !char_died(rndm) )
	 {
	   can_see( mob, rndm ) ? strcpy( t, his_her[ rndm->sex ] )
	                        : strcpy( t, "someone" );
         }
	 else
	      strcpy( t, "its" );
	 break;

     case 'o':
         if ( obj && !obj_extracted(obj) )
         {
           can_see_obj( mob, obj ) ? one_argument( obj->name, t )
                                   : strcpy( t, "something" );
         }
         else
              strcpy( t, "something" );
         break;

     case 'O':
         if ( obj && !obj_extracted(obj) )
	 {
	   can_see_obj( mob, obj ) ? strcpy( t, obj->short_descr )
                                   : strcpy( t, "something" );
         }
	 else
	      strcpy( t, "something" );
	 break;

     case 'p':
         if ( v_obj && !obj_extracted(v_obj) )
	 {
	   can_see_obj( mob, v_obj ) ? one_argument( v_obj->name, t )
                                     : strcpy( t, "something" );
         }
	 else
	      strcpy( t, "something" );
	 break;

     case 'P':
         if ( v_obj && !obj_extracted(v_obj) )
	 {
	   can_see_obj( mob, v_obj ) ? strcpy( t, v_obj->short_descr )
                                     : strcpy( t, "something" );
         }
	 else
	      strcpy( t, "something" );
      break;

     case 'a':
         if ( obj && !obj_extracted(obj) ) 
	 {
	    strcpy( t, aoran(obj->name) );
/*
          switch ( *( obj->name ) )
	  {
	    case 'a': case 'e': case 'i':
            case 'o': case 'u': strcpy( t, "an" );
	      break;
            default: strcpy( t, "a" );
          }
*/
         }
	 else
	      strcpy( t, "a" );
	 break;

     case 'A':
         if ( v_obj && !obj_extracted(v_obj) )
	 {
	      strcpy( t, aoran(v_obj->name) );
         }
	 else
	      strcpy( t, "a" );
	 break;

     case '$':
         strcpy( t, "$" );
	 break;

     default:
         progbug( "Bad $var", mob );
	 break;
       }

 return;

}

/*  The main focus of the MOBprograms.  This routine is called 
 *  whenever a trigger is successful.  It is responsible for parsing
 *  the command list and figuring out what to do. However, like all
 *  complex procedures, everything is farmed out to the other guys.
 *
 *  This function rewritten by Narn for Realms of Despair, Dec/95.
 *
 */
void mprog_driver ( char *com_list, CHAR_DATA *mob, CHAR_DATA *actor,
		   OBJ_DATA *obj, CHAR_DATA *victim, OBJ_DATA *target, bool single_step)
{
  char tmpcmndlst[ MAX_STRING_LENGTH ];
  char *command_list;
  char *cmnd;
  CHAR_DATA *rndm  = NULL;
  CHAR_DATA *vch   = NULL;
  int count        = 0;
  int count2	   = 0;
  int ignorelevel  = 0;
  int iflevel, result;
  bool ifstate[MAX_IFS][ DO_ELSE + 1 ];
  static int prog_nest;
  MPSLEEP_DATA *mpsleep = NULL;
  char arg[MAX_INPUT_LENGTH];
  bool oldMPSilent;
  
  if ( IS_AFFECTED( mob, AFF_CHARM ) || IS_AFFECTED(mob, AFF_POSSESS) )
    return;

  /* Next couple of checks stop program looping. -- Altrag */
  if ( mob == actor )
  {
    progbug( "triggering oneself.", mob );
    return;
  }
  
  if ( ++prog_nest > MAX_PROG_NEST )
  {
    progbug( "max_prog_nest exceeded.", mob );
    --prog_nest;
    return;
  }

  /* Make sure all ifstate bools are set to FALSE */
  for ( iflevel = 0; iflevel < MAX_IFS; iflevel++ )
  {
    for ( count = 0; count < DO_ELSE; count++ )
    {
      ifstate[iflevel][count] = FALSE;
    }
  }

  iflevel = 0;

  /*
   * get a random visible player who is in the room with the mob.
   *
   *  If there isn't a random player in the room, rndm stays NULL.
   *  If you do a $r, $R, $j, or $k with rndm = NULL, you'll crash
   *  in mprog_translate.
   *
   *  Adding appropriate error checking in mprog_translate.
   *    -Haus
   *
   * This used to ignore players MAX_LEVEL - 3 and higher (standard
   * Merc has 4 immlevels).  Thought about changing it to ignore all
   * imms, but decided to just take it out.  If the mob can see you, 
   * you may be chosen as the random player. -Narn
   *
   */

  count = 0;
  for ( vch = mob->in_room->first_person; vch; vch = vch->next_in_room )
    if ( !IS_NPC( vch )
       &&  can_see( mob, vch ) )
      {
        if ( number_range( 0, count ) == 0 )
	  rndm = vch;
        count++;
      }
  
  strcpy( tmpcmndlst, com_list );
  command_list = tmpcmndlst;

/* mpsleep - Restore the environment -rkb */ 
  if (current_mpsleep) 
  { 
    ignorelevel = current_mpsleep->ignorelevel; 
    iflevel = current_mpsleep->iflevel; 
 
    if (single_step) 
      mob->mpscriptpos = 0; 

    for (count = 0; count < MAX_IFS; count++) 
    { 
	for (count2 = 0; count2 < DO_ELSE; count2++)
	  ifstate[count][count2] = 
	    current_mpsleep->ifstate[count][count2];
    }

    current_mpsleep = NULL;
  } 

  if ( single_step )
  {
    if ( mob->mpscriptpos > strlen( tmpcmndlst ) )
       mob->mpscriptpos = 0;
    else
       command_list += mob->mpscriptpos;
    if ( *command_list == '\0' )
    {
	command_list = tmpcmndlst;
	mob->mpscriptpos = 0;
    }
  }

  oldMPSilent = MPSilent;
  MPSilent = FALSE;

  /* From here on down, the function is all mine.  The original code
     did not support nested ifs, so it had to be redone.  The max 
     logiclevel (MAX_IFS) is defined at the beginning of this file, 
     use it to increase/decrease max allowed nesting.  -Narn 
  */

  while ( TRUE )
  {
    /* With these two lines, cmnd becomes the current line from the prog,
       and command_list becomes everything after that line. */
    cmnd         = command_list;
    command_list = mprog_next_command( command_list );

    /* Are we at the end? */
    if ( cmnd[0] == '\0' )
    {
      if ( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] )
      {
        progbug( "Missing endif", mob );
      }
      --prog_nest;
      MPSilent = oldMPSilent;
      return;
    }

/* mpsleep - Check if we should sleep -rkb */ 
  if (!str_prefix("mpsleep", cmnd) )
  { 
    CREATE(mpsleep, MPSLEEP_DATA, 1); 
 
    /* State variables */
    mpsleep->ignorelevel = ignorelevel; 
    mpsleep->iflevel = iflevel; 

    for (count = 0; count < MAX_IFS; count++) 
    {
	for (count2 = 0; count2 < DO_ELSE; count2++) 
	{
	  mpsleep->ifstate[count][count2] = 
	    ifstate[count][count2];
	}
    } 
 
    /* Driver arguments */ 
    mpsleep->com_list = STRALLOC(command_list); 
    mpsleep->mob = mob; 
    mpsleep->actor = actor; 
    mpsleep->obj = obj; 
    mpsleep->victim = victim;
    mpsleep->target = target;
    mpsleep->single_step = single_step; 
 
    /* Time to sleep */ 
    cmnd = one_argument(cmnd, arg); 
    cmnd = one_argument(cmnd, arg); 
    if (arg[0] == '\0')
       mpsleep->timer = 4; 
    else
      mpsleep->timer = atoi(arg); 
 
    if (mpsleep->timer < 1) 
    { 
       progbug("mpsleep - bad arg, using default", mob); 
       mpsleep->timer = 4; 
    } 
 
/* Save type of prog, room, object or mob */ 
  if (mpsleep->mob->pIndexData->vnum == 3) 
  { 
    if (!str_prefix("Room", mpsleep->mob->description)) 
    { 
       mpsleep->type = MP_ROOM; 
       mpsleep->room = mpsleep->mob->in_room; 
    } 
    else if (!str_prefix("Object", mpsleep->mob->description)) 
       mpsleep->type = MP_OBJ; 
  } 
  else
    mpsleep->type = MP_MOB;
 
  LINK(mpsleep, first_mpsleep, last_mpsleep, next, prev); 
  --prog_nest;
  MPSilent = oldMPSilent;
  return;
  } 

    /* Evaluate/execute the command, check what happened. */
    result = mprog_do_command( cmnd, mob, actor, obj, victim, target, rndm, 
            ( ifstate[iflevel][IN_IF] && !ifstate[iflevel][DO_IF] )
            || ( ifstate[iflevel][IN_ELSE] && !ifstate[iflevel][DO_ELSE] ),
            ( ignorelevel > 0 ) );

    /* Script prog support  -Thoric */
    if ( single_step )
    {
      mob->mpscriptpos = command_list - tmpcmndlst;
      --prog_nest;
      MPSilent = oldMPSilent;
      return;
    }

    /* This is the complicated part.  Act on the returned value from
       mprog_do_command according to the current logic state. */
    switch ( result )
    {
    case COMMANDOK:
#ifdef DEBUG
log_string( "COMMANDOK" );
#endif
      /* Ok, this one's a no-brainer. */
      continue;
      break;

    case IFTRUE:
#ifdef DEBUG
log_string( "IFTRUE" );
#endif
      /* An if was evaluated and found true.  Note that we are in an
         if section and that we want to execute it. */
      iflevel++;
      if ( iflevel == MAX_IFS )
      {
        progbug( "Maximum nested ifs exceeded", mob );
        --prog_nest;
        MPSilent = oldMPSilent;
        return;
      }

      ifstate[iflevel][IN_IF] = TRUE; 
      ifstate[iflevel][DO_IF] = TRUE;
      break;

    case IFFALSE:
#ifdef DEBUG
log_string( "IFFALSE" );
#endif
      /* An if was evaluated and found false.  Note that we are in an
         if section and that we don't want to execute it unless we find
         an or that evaluates to true. */
      iflevel++;
      if ( iflevel == MAX_IFS )
      {
        progbug( "Maximum nested ifs exceeded", mob );
        --prog_nest;
        MPSilent = oldMPSilent;
        return;
      }
      ifstate[iflevel][IN_IF] = TRUE; 
      ifstate[iflevel][DO_IF] = FALSE;
      break;

    case ORTRUE:
#ifdef DEBUG
log_string( "ORTRUE" );
#endif
      /* An or was evaluated and found true.  We should already be in an
         if section, so note that we want to execute it. */
      if ( !ifstate[iflevel][IN_IF] )
      {
        progbug( "Unmatched or", mob );
        --prog_nest;
        MPSilent = oldMPSilent;
        return;
      }
      ifstate[iflevel][DO_IF] = TRUE;
      break;

    case ORFALSE:
#ifdef DEBUG
log_string( "ORFALSE" );
#endif
      /* An or was evaluated and found false.  We should already be in an
         if section, and we don't need to do much.  If the if was true or
         there were/will be other ors that evaluate(d) to true, they'll set
         do_if to true. */
      if ( !ifstate[iflevel][IN_IF] )
      {
        progbug( "Unmatched or", mob );
        --prog_nest;
        MPSilent = oldMPSilent;
        return;
      }
      continue;
      break;

    case FOUNDELSE:
#ifdef DEBUG
log_string( "FOUNDELSE" );
#endif
      /* Found an else.  Make sure we're in an if section, bug out if not.
         If this else is not one that we wish to ignore, note that we're now 
         in an else section, and look at whether or not we executed the if 
         section to decide whether to execute the else section.  Ca marche 
         bien. */
      if ( ignorelevel > 0 )
        continue;

      if ( ifstate[iflevel][IN_ELSE] )
      {
        progbug( "Found else in an else section", mob );
        --prog_nest;
        MPSilent = oldMPSilent;
        return;
      }
      if ( !ifstate[iflevel][IN_IF] )
      {
        progbug( "Unmatched else", mob );
        --prog_nest;
        MPSilent = oldMPSilent;
        return;
      }

      ifstate[iflevel][IN_ELSE] = TRUE;
      ifstate[iflevel][DO_ELSE] = !ifstate[iflevel][DO_IF];
      ifstate[iflevel][IN_IF]   = FALSE;
      ifstate[iflevel][DO_IF]   = FALSE;
   
      break;

    case FOUNDENDIF:
#ifdef DEBUG
log_string( "FOUNDENDIF" );
#endif
      /* Hmm, let's see... FOUNDENDIF must mean that we found an endif.
         So let's make sure we were expecting one, return if not.  If this
         endif matches the if or else that we're executing, note that we are 
         now no longer executing an if.  If not, keep track of what we're 
         ignoring. */
      if ( !( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] ) )
      {
        progbug( "Unmatched endif", mob );
        --prog_nest;
        MPSilent = oldMPSilent;
        return;
      }

      if ( ignorelevel > 0 )      
      {
        ignorelevel--;
        continue;
      }

      ifstate[iflevel][IN_IF]   = FALSE;
      ifstate[iflevel][DO_IF]   = FALSE;
      ifstate[iflevel][IN_ELSE] = FALSE;
      ifstate[iflevel][DO_ELSE] = FALSE;

      iflevel--;
      break;

    case IFIGNORED:
#ifdef DEBUG
log_string( "IFIGNORED" );
#endif
      if ( !( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] ) )
      {
        progbug( "Parse error, ignoring if while not in if or else", mob );
        --prog_nest;
        MPSilent = oldMPSilent;
        return;
      }
      ignorelevel++;
      break;

    case ORIGNORED:
#ifdef DEBUG
log_string( "ORIGNORED" );
#endif
      if ( !( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] ) )
      {
        progbug( "Unmatched or", mob );
        --prog_nest;
        MPSilent = oldMPSilent;
        return;
      }
      if ( ignorelevel == 0 )
      {
        progbug( "Parse error, mistakenly ignoring or", mob );
        --prog_nest;
        MPSilent = oldMPSilent;
        return;
      }

      break;

    case BERR:
#ifdef DEBUG
log_string( "BERR" );
#endif
      --prog_nest;
      MPSilent = oldMPSilent;
      return;
      break;
    }
  }
  --prog_nest;
  MPSilent = oldMPSilent;
  return;
}

/* This function replaces mprog_process_cmnd.  It is called from 
 * mprog_driver, once for each line in a mud prog.  This function
 * checks what the line is, executes if/or checks and calls interpret
 * to perform the the commands.  Written by Narn, Dec 95.
 */
int mprog_do_command( char *cmnd, CHAR_DATA *mob, CHAR_DATA *actor, 
                      OBJ_DATA *obj, CHAR_DATA *victim, OBJ_DATA *target, CHAR_DATA *rndm, 
                      bool ignore, bool ignore_ors )
{
  char firstword[MAX_INPUT_LENGTH];
  char *ifcheck;
  char buf[ MAX_INPUT_LENGTH ];
  char tmp[ MAX_INPUT_LENGTH ];
  char *point, *str, *i;
  int validif, vnum;

  /* Isolate the first word of the line, it gives us a clue what
     we want to do. */
  ifcheck = one_argument( cmnd, firstword );

  if ( !str_cmp( firstword, "if" ) )
  {
    /* Ok, we found an if.  According to the boolean 'ignore', either
       ignore the ifcheck and report that back to mprog_driver or do
       the ifcheck and report whether it was successful. */
    if ( ignore )
      return IFIGNORED;
    else
      validif = mprog_do_ifcheck( ifcheck, mob, actor, obj, victim, target, rndm );

    if ( validif == 1 )
      return IFTRUE;

    if ( validif == 0 )
      return IFFALSE;

    return BERR;
  }

  if ( !str_cmp( firstword, "or" ) )
  {
    /* Same behavior as with ifs, but use the boolean 'ignore_ors' to
       decide which way to go. */
    if ( ignore_ors )
      return ORIGNORED;
    else
      validif = mprog_do_ifcheck( ifcheck, mob, actor, obj, victim, target, rndm );

    if ( validif == 1 )
      return ORTRUE;

    if ( validif == 0 )
      return ORFALSE;

    return BERR;
  }

  /* For else and endif, just report back what we found.  Mprog_driver
     keeps track of logiclevels. */
  if ( !str_cmp( firstword, "else" ) )
  {
    return FOUNDELSE;
  }

  if ( !str_cmp( firstword, "endif" ) )
  {
    return FOUNDENDIF;
  }

  /* Ok, didn't find an if, an or, an else or an endif.  
     If the command is in an if or else section that is not to be 
     performed, the boolean 'ignore' is set to true and we just 
     return.  If not, we try to execute the command. */

  if ( ignore )
    return COMMANDOK;

  /* If the command is 'break', that's all folks. */
  if ( !str_cmp( firstword, "break" ) )
    return BERR;

  if ( !str_cmp( firstword, "silent" ) )
  {
    MPSilent = TRUE;
    cmnd = one_argument( cmnd, firstword );
  }

  vnum = mob->pIndexData->vnum;
  point   = buf;
  str     = cmnd;

  /* This chunk of code taken from mprog_process_cmnd. */
  while ( *str != '\0' )
  {
    if ( *str != '$' )
    {
      *point++ = *str++;
      continue;
    }
    str++;
    mprog_translate( *str, tmp, mob, actor, obj, victim, target, rndm );
    i = tmp;
    ++str;
    while ( ( *point = *i ) != '\0' )
      ++point, ++i;
  }
  *point = '\0';

  interpret( mob, buf );  

  MPSilent = FALSE;

  /* If the mob is mentally unstable and does things like fireball
     itself, let's make sure it's still alive. */
  if ( char_died( mob ) )
  {
    bug( "Mob died while executing program, vnum %d.", vnum );
    return BERR;
  }

  return COMMANDOK;
}

/***************************************************************************
 * Global function code and brief comments.
 */

 
/* See if there's any mud programs waiting to be continued -rkb */ 
void mpsleep_update() 
{ 
 MPSLEEP_DATA *mpsleep; 
 MPSLEEP_DATA *tmpMpsleep; 
 bool delete_it; 
 
 mpsleep = first_mpsleep; 
 while (mpsleep) 
 { 
 delete_it = FALSE; 
 
 if (mpsleep->mob) 
 delete_it = char_died(mpsleep->mob); 
 
 if (mpsleep->actor && !delete_it) 
 delete_it = char_died(mpsleep->actor); 
 
 if (mpsleep->obj && !delete_it) 
 delete_it = obj_extracted(mpsleep->obj); 
 
 if (delete_it) 
 { 
 log_string("mpsleep_update - Deleting expired prog."); 
 
 tmpMpsleep = mpsleep; 
 mpsleep = mpsleep->next; 
 STRFREE(tmpMpsleep->com_list); 
 UNLINK(tmpMpsleep, first_mpsleep, last_mpsleep, next, prev); 
 DISPOSE(tmpMpsleep); 
 
 continue; 
 } 
 
 mpsleep = mpsleep->next; 
 } 
 
 mpsleep = first_mpsleep; 
 while (mpsleep) /* Find progs to continue */ 
 { 
 if (--mpsleep->timer <= 0) 
 { 
 current_mpsleep = mpsleep; 
 
 if (mpsleep->type == MP_ROOM) 
 rset_supermob(mpsleep->room); 
 else if (mpsleep->type == MP_OBJ) 
 set_supermob(mpsleep->obj); 
 
 mprog_driver(mpsleep->com_list, mpsleep->mob, mpsleep->actor, 
 mpsleep->obj, mpsleep->victim, mpsleep->target, mpsleep->single_step); 
 
 release_supermob(); 
 
 tmpMpsleep = mpsleep; 
 mpsleep = mpsleep->next; 
 STRFREE(tmpMpsleep->com_list); 
 UNLINK(tmpMpsleep, first_mpsleep, last_mpsleep, next, prev); 
 DISPOSE(tmpMpsleep); 
 
 continue; 
 } 
 
 mpsleep = mpsleep->next; 
 } 
} 
 

bool mprog_keyword_check( const char *argu, const char *argl )
{
    char word[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int i;
    char *arg, *arglist;
    char *start, *end;

    strcpy( arg1, strlower( argu ) );
    arg = arg1;
    strcpy( arg2, strlower( argl ) );
    arglist = arg2;

    for ( i = 0; i < strlen( arglist ); i++ )
	arglist[i] = LOWER( arglist[i] );
    for ( i = 0; i < strlen( arg ); i++ )
	arg[i] = LOWER( arg[i] );
    if ( ( arglist[0] == 'p' ) && ( arglist[1] == ' ' ) )
    {
	arglist += 2;
	while ( ( start = strstr( arg, arglist ) ) )
	    if ( (start == arg || *(start-1) == ' ' )
    	    && ( *(end = start + strlen( arglist ) ) == ' '
    	    ||   *end == '\n'
    	    ||   *end == '\r'
    	    ||   *end == '\0' ) )
		return TRUE;
	    else
		arg = start+1;
    }
    else
    {
	arglist = one_argument( arglist, word );
	for ( ; word[0] != '\0'; arglist = one_argument( arglist, word ) )
	    while ( ( start = strstr( arg, word ) ) )
		if ( ( start == arg || *(start-1) == ' ' )
	    	&& ( *(end = start + strlen( word ) ) == ' '
	    	||   *end == '\n'
	    	||   *end == '\r'
	    	||   *end == '\0' ) )
		    return TRUE;
		else
		    arg = start +1;
    }
/*    bug( "don't match" ); */
    return FALSE;
}


/* The next two routines are the basic trigger types. Either trigger
 *  on a certain percent, or trigger on a keyword or word phrase.
 *  To see how this works, look at the various trigger routines..
 */
bool mprog_wordlist_check( char *arg, CHAR_DATA *mob, CHAR_DATA *actor,
			  OBJ_DATA *obj, CHAR_DATA *victim, OBJ_DATA *target, int type )
{

  char	      temp1[ MAX_STRING_LENGTH ];
  char	      temp2[ MAX_INPUT_LENGTH ];
  char	      word[ MAX_INPUT_LENGTH ];
  MPROG_DATA *mprg;
  char       *list;
  char       *start;
  char       *dupl;
  char       *end;
  int	      i;
  bool	      executed=FALSE;


  for ( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
    if ( mprg->type == type )
      {
	strcpy( temp1, mprg->arglist );
	list = temp1;
	for ( i = 0; i < strlen( list ); i++ )
	  list[i] = LOWER( list[i] );
	strcpy( temp2, arg );
	dupl = temp2;
	for ( i = 0; i < strlen( dupl ); i++ )
	  dupl[i] = LOWER( dupl[i] );
	if ( ( list[0] == 'p' ) && ( list[1] == ' ' ) )
	  {
	    list += 2;
	    while ( ( start = strstr( dupl, list ) ) )
	      if ( (start == dupl || *(start-1) == ' ' )
		  && ( *(end = start + strlen( list ) ) == ' '
		      || *end == '\n'
		      || *end == '\r'
		      || *end == '\0' ) )
		{
		  mprog_driver( mprg->comlist, mob, actor, obj, victim, target, FALSE );
		  executed=TRUE;
		  break;
		}
	      else
		dupl = start+1;
	  }
	else
	  {
	    list = one_argument( list, word );
	    for( ; word[0] != '\0'; list = one_argument( list, word ) )
	      while ( ( start = strstr( dupl, word ) ) )
		if ( ( start == dupl || *(start-1) == ' ' )
		    && ( *(end = start + strlen( word ) ) == ' '
			|| *end == '\n'
			|| *end == '\r'
			|| *end == '\0' ) )
		  {
		    mprog_driver( mprg->comlist, mob, actor, obj, victim, target, FALSE );
		    executed=TRUE;
		    break;
		  }
		else
		  dupl = start+1;
	  }
      }

  return executed;

}


void mprog_percent_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj,
			  CHAR_DATA *victim, OBJ_DATA *target, int type)
{
 MPROG_DATA * mprg;

 for ( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
   if ( ( mprg->type == type )
       && ( number_percent( ) <= atoi( mprg->arglist ) ) )
     {
       mprog_driver( mprg->comlist, mob, actor, obj, victim, target, FALSE );
       if ( type != GREET_PROG && type != ALL_GREET_PROG
       &&   type != LOGIN_PROG && type != VOID_PROG
       &&   type != GREET_IN_FIGHT_PROG ) /* added this line for login/void
					   triggers */
         break;
     }
 return;
}

void mprog_time_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj,
                         CHAR_DATA *victim, OBJ_DATA *target, int type)
{
 MPROG_DATA * mprg;
 bool       trigger_time;

 for ( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
   {
     trigger_time = ( time_info.hour == atoi( mprg->arglist ) );     

     if ( !trigger_time )
     {
       if ( mprg->triggered )
         mprg->triggered = FALSE;
       continue;
     }

     if ( ( mprg->type == type )
       && ( ( !mprg->triggered ) || ( mprg->type == HOUR_PROG ) ) )
     {
       mprg->triggered = TRUE;
       mprog_driver( mprg->comlist, mob, actor, obj, victim, target, FALSE );
     }
   }
 return;
}


void mob_act_add( CHAR_DATA *mob )
{
    struct act_prog_data *runner;
  
    for ( runner = mob_act_list; runner; runner = runner->next )
	if ( runner->vo == mob )
	   return;
    CREATE(runner, struct act_prog_data, 1);
    runner->vo = mob;
    runner->next = mob_act_list;
    mob_act_list = runner;
}


/* The triggers.. These are really basic, and since most appear only
 * once in the code (hmm. i think they all do) it would be more efficient
 * to substitute the code in and make the mprog_xxx_check routines global.
 * However, they are all here in one nice place at the moment to make it
 * easier to see what they look like. If you do substitute them back in,
 * make sure you remember to modify the variable names to the ones in the
 * trigger calls.
 */
void mprog_act_trigger( char *buf, CHAR_DATA *mob, CHAR_DATA *ch,
		       OBJ_DATA *obj, CHAR_DATA *victim, OBJ_DATA *target)
{
    MPROG_ACT_LIST * tmp_act;
    MPROG_DATA *mprg;
    bool found = FALSE;

    if ( IS_NPC( mob )
    &&   HAS_PROG( mob->pIndexData, ACT_PROG ) )
    {
	/* Don't let a mob trigger itself, nor one instance of a mob
	  trigger another instance. */
	if ( IS_NPC( ch ) && ch->pIndexData == mob->pIndexData )
	  return;

	/* make sure this is a matching trigger */
	for ( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
	    if ( mprg->type == ACT_PROG
	    &&   mprog_keyword_check( buf, mprg->arglist ) )
	    {
		found = TRUE;
		break;
	    }
	if ( !found )
	    return;

	CREATE( tmp_act, MPROG_ACT_LIST, 1 );
	if ( mob->mpactnum > 0 )
	  tmp_act->next = mob->mpact;
	else
	  tmp_act->next = NULL;

	mob->mpact      = tmp_act;
	mob->mpact->buf = str_dup( buf );  
	mob->mpact->ch  = ch;
	mob->mpact->obj = obj;
	mob->mpact->victim = victim; 
	mob->mpact->target = target;
	mob->mpactnum++;
	mob_act_add( mob );
    }
    return;
}

void mprog_bribe_trigger( CHAR_DATA *mob, CHAR_DATA *ch, int amount )
{

  char        buf[ MAX_STRING_LENGTH ];
  MPROG_DATA *mprg, *tprg = NULL;
  OBJ_DATA   *obj;

  if ( IS_NPC( mob )
      && can_see( mob, ch )
      && HAS_PROG( mob->pIndexData, BRIBE_PROG ) )
    {
      /* Don't let a mob trigger itself, nor one instance of a mob
         trigger another instance. */
      if ( IS_NPC( ch ) && ch->pIndexData == mob->pIndexData )
        return;

      obj = create_object( get_obj_index( OBJ_VNUM_MONEY_SOME ), 0 );
      sprintf( buf, obj->short_descr, amount );
      STRFREE( obj->short_descr );
      obj->short_descr = STRALLOC( buf );
      obj->value[0]    = amount;
      obj = obj_to_char( obj, mob );
      mob->gold -= amount;

      for ( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
	if ( ( mprg->type == BRIBE_PROG )
	&& ( amount >= atoi( mprg->arglist ) ) )
	{
      if ( tprg )
	   {	
          if ( atoi( tprg->arglist ) < atoi( mprg->arglist ) )
  	          tprg = mprg; 
       }
	  else
		tprg = mprg;
    }

	if ( tprg )
	    mprog_driver( tprg->comlist, mob, ch, obj, NULL, NULL, FALSE );

    }
  
  return;

}

void mprog_death_trigger( CHAR_DATA *killer, CHAR_DATA *mob )
{
    if ( IS_NPC(mob) && killer != mob
    &&   HAS_PROG(mob->pIndexData, DEATH_PROG) )
    {
	mob->position = POS_STANDING;
	mprog_percent_check( mob, killer, NULL, NULL, NULL, DEATH_PROG );
	mob->position = POS_DEAD;
    }
    death_cry( mob );
    return;
}
/* login and void mob triggers by Edmond */
void mprog_login_trigger( CHAR_DATA *ch )
{
 CHAR_DATA *vmob, *vmob_next;

#ifdef DEBUG
 char buf[MAX_STRING_LENGTH];
 sprintf( buf, "mprog_login_trigger -> %s", ch->name );
 log_string( buf );
#endif

 for ( vmob = ch->in_room->first_person; vmob; vmob = vmob_next )
 {
   vmob_next = vmob->next_in_room;
     if ( !IS_NPC( vmob )
        || !can_see( vmob, ch )
        || vmob->fighting
        || !IS_AWAKE( vmob ) )
     continue;

  if ( IS_NPC( ch ) && ch->pIndexData == vmob->pIndexData )
    continue;

   if ( HAS_PROG(vmob->pIndexData, LOGIN_PROG) )
     mprog_percent_check( vmob, ch, NULL, NULL, NULL, LOGIN_PROG );
 }
 return;

}

void mprog_void_trigger( CHAR_DATA *ch )
{
 CHAR_DATA *vmob, *vmob_next;

#ifdef DEBUG
 char buf[MAX_STRING_LENGTH];
 sprintf( buf, "mprog_void_trigger -> %s", ch->name );
 log_string( buf );
#endif

 for ( vmob = ch->in_room->first_person; vmob; vmob = vmob_next )
 {
   vmob_next = vmob->next_in_room;
     if ( !IS_NPC( vmob )
        || !can_see( vmob, ch )
        || vmob->fighting
        || !IS_AWAKE( vmob ) )
     continue;

  if ( IS_NPC( ch ) && ch->pIndexData == vmob->pIndexData )
    continue;

   if ( HAS_PROG(vmob->pIndexData, VOID_PROG) )
     mprog_percent_check( vmob, ch, NULL, NULL, NULL, VOID_PROG );
 }
 return;
}



void mprog_entry_trigger( CHAR_DATA *mob )
{
    if ( IS_NPC(mob)
    &&   HAS_PROG(mob->pIndexData, ENTRY_PROG) )
	mprog_percent_check( mob, NULL, NULL, NULL, NULL, ENTRY_PROG );

    return;
}

void mprog_fight_trigger( CHAR_DATA *mob, CHAR_DATA *ch )
{
    if ( IS_NPC( mob )
    &&   HAS_PROG( mob->pIndexData, FIGHT_PROG ) )
	mprog_percent_check( mob, ch, NULL, NULL, NULL, FIGHT_PROG );

    return;
}

void mprog_give_trigger( CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj )
{
    char        buf[MAX_INPUT_LENGTH];
    MPROG_DATA *mprg;

    if ( IS_NPC( mob )
    &&   can_see( mob, ch )
    &&   HAS_PROG( mob->pIndexData, GIVE_PROG ) )
    {
	/* Don't let a mob trigger itself, nor one instance of a mob
	trigger another instance. */
	if ( IS_NPC( ch ) && ch->pIndexData == mob->pIndexData )
	    return; 

	for ( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
	{
	    one_argument( mprg->arglist, buf );

	    if ( mprg->type == GIVE_PROG
	    &&  (!str_cmp(obj->name, mprg->arglist)
	       || !str_cmp("all", buf)) )
	    {
		mprog_driver( mprg->comlist, mob, ch, obj, NULL, NULL, FALSE );
		break;
	    }
	}
    }
    return;
}

void mprog_sell_trigger( CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj )
{
	char buf[MAX_STRING_LENGTH];
	int s_vnum;
	MPROG_DATA *mprg;

    if ( IS_NPC( mob )
    &&   can_see( mob, ch )
    &&   HAS_PROG( mob->pIndexData, SELL_PROG ) )
    {

		if ( IS_NPC( ch ) && ch->pIndexData == mob->pIndexData )
		    return;

		for ( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
		{
		    one_argument( mprg->arglist, buf );

			if ( !is_number( buf ) )
				continue;

			s_vnum = atoi( buf );

		    if ( mprg->type == SELL_PROG
		    && ( ( s_vnum == obj->pIndexData->vnum ) || ( s_vnum == 0 ) ) )
		    {
				mprog_driver( mprg->comlist, mob, ch, obj, NULL, NULL, FALSE );
				break;
		    }
		}
    }
    return;
}


/* Greet_progs holding an mpat command are executed twice.  The real problem
   is the script engine ( an hack of another hack etc. itself ) does not
   tolerate the script's owner moving to the end of the room's mob list.
   The script engine would really use a rewrite from scratch.  Will be
   scheduled for, say, autumn 2010. In the meantime this fix will suffice,
   a room with more than 1024 scripted mobs should not be common :P - Luc 09/2000 */

#define MAX_MOB_ROOM 1024

void mprog_greet_trigger( CHAR_DATA *ch ) {
   CHAR_DATA * vmob;
   int cmob, mnum = 0;
   EXTRACT_CHAR_DATA * deadmark, * epnt;
   CHAR_DATA * script[MAX_MOB_ROOM];

#ifdef DEBUG
 char buf[MAX_STRING_LENGTH];
 sprintf( buf, "mprog_greet_trigger -> %s", ch->name );
 log_string( buf );
#endif

   for ( vmob = ch->in_room->first_person ; vmob && mnum < MAX_MOB_ROOM ; vmob = vmob->next_in_room ) {
      if ( !IS_NPC( vmob ) || !can_see( vmob, ch )
      || (vmob->fighting && !HAS_PROG(vmob->pIndexData, GREET_IN_FIGHT_PROG)) 
      || !IS_AWAKE( vmob ) )
         continue;
      /* Don't let a mob trigger itself, nor one instance of a mob
         trigger another instance. */
      if ( IS_NPC( ch ) && ch->pIndexData == vmob->pIndexData )
         continue;
      if ( HAS_PROG( vmob->pIndexData, GREET_PROG )
      ||   HAS_PROG( vmob->pIndexData, ALL_GREET_PROG ) 
      ||   HAS_PROG( vmob->pIndexData, GREET_IN_FIGHT_PROG ) )
         script[mnum++] = vmob;
      }

   if ( vmob && mnum == MAX_MOB_ROOM ) {
      char msg[100];
      sprintf( msg, "Greet_prog: too many mobs in room %d.", ch->in_room->vnum );
      log_string( msg );
      }

   deadmark = extracted_char_queue;
   for ( cmob = 0 ; cmob < mnum ; cmob++ ) {
      vmob = script[cmob];
      for ( epnt = extracted_char_queue ; epnt != deadmark ; epnt = epnt->next )
         if ( epnt->ch == vmob )
            break;
      if ( epnt == deadmark ){
         if ( vmob->fighting )
            mprog_percent_check( vmob, ch, NULL, NULL, NULL, GREET_IN_FIGHT_PROG );
	 else
	    mprog_percent_check( vmob, ch, NULL, NULL, NULL,
	      HAS_PROG( vmob->pIndexData, GREET_PROG ) ? GREET_PROG : ALL_GREET_PROG );
      }
   }
}


void mprog_hitprcnt_trigger( CHAR_DATA *mob, CHAR_DATA *ch)
{
    MPROG_DATA *mprg;

    if ( IS_NPC( mob )
    &&   HAS_PROG(mob->pIndexData, HITPRCNT_PROG) )
    {
	for ( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
	    if ( mprg->type == HITPRCNT_PROG
	    &&  (100*mob->hit / mob->max_hit) < atoi(mprg->arglist) )
	{
	    mprog_driver( mprg->comlist, mob, ch, NULL, NULL, NULL, FALSE );
	    break;
	}
    }
    return;
}

void mprog_random_trigger( CHAR_DATA *mob )
{
    if ( HAS_PROG(mob->pIndexData, RAND_PROG) )
	mprog_percent_check(mob,NULL,NULL,NULL,NULL,RAND_PROG);
}

void mprog_time_trigger( CHAR_DATA *mob )
{
    if ( HAS_PROG(mob->pIndexData, TIME_PROG) )
	mprog_time_check(mob,NULL,NULL,NULL,NULL,TIME_PROG);
}

void mprog_hour_trigger( CHAR_DATA *mob )
{
    if ( HAS_PROG(mob->pIndexData, HOUR_PROG) )
	mprog_time_check(mob,NULL,NULL,NULL,NULL,HOUR_PROG); 
}

void mprog_speech_trigger( char *txt, CHAR_DATA *actor )
{
    CHAR_DATA *vmob;

    for ( vmob = actor->in_room->first_person; vmob; vmob = vmob->next_in_room )
    {
	if ( IS_NPC(vmob) && HAS_PROG(vmob->pIndexData, SPEECH_PROG) )
	{
	    if ( IS_NPC(actor) && actor->pIndexData == vmob->pIndexData )
		continue;
	    mprog_wordlist_check( txt, vmob, actor, NULL, NULL, NULL, SPEECH_PROG );
	}
    }
}

void mprog_tell_trigger( char *txt, CHAR_DATA *actor )
{
    CHAR_DATA *vmob;

    for ( vmob = actor->in_room->first_person; vmob; vmob = vmob->next_in_room )
    {
	if ( IS_NPC(vmob) && HAS_PROG(vmob->pIndexData, TELL_PROG) )
	{
	    if ( IS_NPC(actor) && actor->pIndexData == vmob->pIndexData )
		continue;
	    mprog_wordlist_check( txt, vmob, actor, NULL, NULL, NULL, TELL_PROG );
	}
    }
}

bool mprog_command_trigger( CHAR_DATA *actor, char *txt )
{
    CHAR_DATA *vmob;

    for ( vmob = actor->in_room->first_person; vmob; vmob = vmob->next_in_room )
    {
	if ( IS_NPC(vmob) && HAS_PROG(vmob->pIndexData, CMD_PROG) )
	{
	    if ( IS_NPC(actor) && actor->pIndexData == vmob->pIndexData )
		continue;
	    if ( mprog_wordlist_check( txt, vmob, actor, NULL, NULL, NULL, CMD_PROG ) )
	    return TRUE;
	}
    }
    return FALSE;
}

void mprog_script_trigger( CHAR_DATA *mob )
{
    MPROG_DATA * mprg;

    if ( HAS_PROG(mob->pIndexData, SCRIPT_PROG) )
	for ( mprg = mob->pIndexData->mudprogs; mprg; mprg = mprg->next )
	    if ( mprg->type == SCRIPT_PROG
	    &&  (mprg->arglist[0] == '\0'
	      || mob->mpscriptpos != 0
	      || atoi(mprg->arglist) == time_info.hour) )
		mprog_driver( mprg->comlist, mob, NULL, NULL, NULL, NULL, TRUE );
}

void oprog_script_trigger( OBJ_DATA *obj )
{
    MPROG_DATA * mprg;

    if ( HAS_PROG(obj->pIndexData, SCRIPT_PROG) )
      for ( mprg = obj->pIndexData->mudprogs; mprg; mprg = mprg->next )
	if ( mprg->type == SCRIPT_PROG )
	{
	  if ( mprg->arglist[0] == '\0'
	  ||   obj->mpscriptpos != 0
	  ||   atoi( mprg->arglist ) == time_info.hour )
	  {
	     set_supermob( obj );
	     mprog_driver( mprg->comlist, supermob, NULL, NULL, NULL, NULL, TRUE );
	     obj->mpscriptpos = supermob->mpscriptpos;
	     release_supermob();
	  }
	}
    return;
}

void rprog_script_trigger( ROOM_INDEX_DATA *room )
{
    MPROG_DATA * mprg;

    if ( HAS_PROG(room, SCRIPT_PROG) )
      for ( mprg = room->mudprogs; mprg; mprg = mprg->next )
	if ( mprg->type == SCRIPT_PROG )
	{
	  if ( mprg->arglist[0] == '\0'
	  ||   room->mpscriptpos != 0
	  ||   atoi( mprg->arglist ) == time_info.hour )
	  {
	     rset_supermob( room );
	     mprog_driver( mprg->comlist, supermob, NULL, NULL, NULL, NULL, TRUE );
	     room->mpscriptpos = supermob->mpscriptpos;
	     release_supermob();
	  }
	}
    return;
}


/*
 *  Mudprogram additions begin here
 */
void set_supermob( OBJ_DATA *obj)
{
  ROOM_INDEX_DATA *room;
  OBJ_DATA *in_obj;
  CHAR_DATA *mob;
  char buf[200];

  if ( !supermob )
    supermob = create_mobile(get_mob_index( 3 ));

  mob = supermob;   /* debugging */

  if(!obj)
     return;

  for ( in_obj = obj; in_obj->in_obj; in_obj = in_obj->in_obj )
    ;

  if ( in_obj->carried_by )
  {   
      room = in_obj->carried_by->in_room;
  }
  else
  {
      room = obj->in_room;
  }

  if(!room)
     return;

  if (supermob->short_descr)
     STRFREE(supermob->short_descr);

  supermob->short_descr = QUICKLINK(obj->short_descr);
  supermob->mpscriptpos = obj->mpscriptpos;

  /* Added by Jenny to allow bug messages to show the vnum
     of the object, and not just supermob's vnum */
  sprintf( buf, "Object #%d", obj->pIndexData->vnum );
  STRFREE( supermob->description );
  supermob->description = STRALLOC( buf );

  if(room != NULL)
  {
    char_from_room (supermob );
    char_to_room( supermob, room); 
  }
}

void release_supermob( )
{
  char_from_room( supermob );
  char_to_room( supermob, get_room_index( 3 ) );
}


bool oprog_percent_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj,
			  CHAR_DATA *victim, OBJ_DATA *target, int type)
{
    MPROG_DATA * mprg;
    bool executed = FALSE;

    for ( mprg = obj->pIndexData->mudprogs; mprg; mprg = mprg->next )
	if ( mprg->type == type
	&& ( number_percent( ) <= atoi( mprg->arglist ) ) )
	{
	    executed = TRUE;
	    mprog_driver( mprg->comlist, mob, actor, obj, victim, target, FALSE );
	    if ( type != GREET_PROG )
		break;
	}

    return executed;
}

/*
 * Triggers follow
 */


/*
 *  Hold on this
 *
void oprog_act_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    set_supermob( obj );
    if ( HAS_PROG(obj->pIndexData, ACT_PROG) )
	oprog_percent_check( supermob, ch, obj, NULL, NULL, ACT_PROG );
    release_supermob();

    return;
}
 *
 *
 */

void oprog_greet_trigger( CHAR_DATA *ch )
{
    OBJ_DATA *vobj;

    for ( vobj=ch->in_room->first_content; vobj; vobj = vobj->next_content )
	if ( HAS_PROG(vobj->pIndexData, GREET_PROG) )
	{
	    set_supermob( vobj );  /* not very efficient to do here */
	    oprog_percent_check( supermob, ch, vobj, NULL, NULL, GREET_PROG );
	    release_supermob();
	}
}

void oprog_speech_trigger( char *txt, CHAR_DATA *ch )
{
    OBJ_DATA *vobj;

    /* supermob is set and released in oprog_wordlist_check */
    for ( vobj=ch->in_room->first_content; vobj; vobj = vobj->next_content )
	if ( HAS_PROG(vobj->pIndexData, SPEECH_PROG) )
	    oprog_wordlist_check(txt, supermob, ch, vobj, NULL, NULL, SPEECH_PROG, vobj);

 return;
}

bool oprog_command_trigger( CHAR_DATA *ch, char *txt )
{
    OBJ_DATA *vobj;

    /* supermob is set and released in oprog_wordlist_check */
    for ( vobj=ch->in_room->first_content; vobj; vobj = vobj->next_content )
	if ( HAS_PROG(vobj->pIndexData, CMD_PROG) )
	    if ( oprog_wordlist_check(txt, supermob, ch, vobj, NULL, NULL, CMD_PROG, vobj) )
			return TRUE;
	
 return FALSE;
}


/*
 * Called at top of obj_update
 * make sure to put an if(!obj) continue
 * after it
 */
void oprog_random_trigger( OBJ_DATA *obj )
{
    if ( HAS_PROG(obj->pIndexData, RAND_PROG) )
    {
	set_supermob( obj );
	oprog_percent_check(supermob,NULL,obj,NULL,NULL,RAND_PROG);
	release_supermob();
    }
}

/*
 * in wear_obj, between each successful equip_char 
 * the subsequent return
 */
void oprog_wear_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_PROG(obj->pIndexData, WEAR_PROG) )
    {
	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, NULL, WEAR_PROG );
	release_supermob();
    }
}

bool oprog_use_trigger( CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *vict,
                        OBJ_DATA *targ, void *vo )
{
    bool executed = FALSE;

    if ( HAS_PROG(obj->pIndexData, USE_PROG) )
    {
	set_supermob( obj );
	if ( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND
	||   obj->item_type == ITEM_SCROLL )
	{
	    executed = oprog_percent_check( supermob, ch, obj, vict, targ, USE_PROG );
	}
	else
	    executed = oprog_percent_check( supermob, ch, obj, NULL, NULL, USE_PROG );
	release_supermob();
    }
    return executed;
}

/*
 * call in remove_obj, right after unequip_char   
 * do a if(!ch) return right after, and return TRUE (?)
 * if !ch
 */
void oprog_remove_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_PROG(obj->pIndexData, REMOVE_PROG) )
    {
	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, NULL, REMOVE_PROG );
	release_supermob();
    }
}


/*
 * call in do_sac, right before extract_obj
 */
void oprog_sac_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_PROG(obj->pIndexData, SAC_PROG) )
    {
	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, NULL, SAC_PROG );
	release_supermob();
    }
}

/*
 * call in do_get, right before check_for_trap
 * do a if(!ch) return right after
 */
void oprog_get_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_PROG(obj->pIndexData, GET_PROG) )
    {
	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, NULL, GET_PROG );
	release_supermob();
    }
}

/*
 * called in damage_obj in act_obj.c
 */
void oprog_damage_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_PROG(obj->pIndexData, DAMAGE_PROG) )
    {
	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, NULL, DAMAGE_PROG );
	release_supermob();
    }
}

/*
 * called in do_repair in shops.c
 */
void oprog_repair_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_PROG(obj->pIndexData, REPAIR_PROG) )
    {
	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, NULL, REPAIR_PROG );
	release_supermob();
    }
}

/*
 * call twice in do_drop, right after the act( AT_ACTION,...)
 * do a if(!ch) return right after
 */
void oprog_drop_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_PROG(obj->pIndexData, DROP_PROG) )
    {
	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, NULL, DROP_PROG );
	release_supermob();
    }
}

/*
 * call towards end of do_examine, right before check_for_trap
 */
void oprog_examine_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_PROG(obj->pIndexData, EXA_PROG) )
    {
	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, NULL, EXA_PROG );
	release_supermob();
    }
}


/*
 * call in fight.c, group_gain, after (?) the obj_to_room
 */
void oprog_zap_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_PROG(obj->pIndexData, ZAP_PROG) )
    {
	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, NULL, ZAP_PROG );
	release_supermob();
    }
}

/*
 * call in levers.c, towards top of do_push_or_pull
 *  see note there 
 */
void oprog_pull_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_PROG(obj->pIndexData, PULL_PROG) )
    {
	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, NULL, PULL_PROG );
	release_supermob();
    }
}

/*
 * call in levers.c, towards top of do_push_or_pull
 *  see note there 
 */
void oprog_push_trigger( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( HAS_PROG(obj->pIndexData, PUSH_PROG) )
    {
	set_supermob( obj );
	oprog_percent_check( supermob, ch, obj, NULL, NULL, PUSH_PROG );
	release_supermob();
    }
}

void obj_act_add( OBJ_DATA *obj );
void oprog_act_trigger( char *buf, OBJ_DATA *mobj, CHAR_DATA *ch,
			OBJ_DATA *obj, CHAR_DATA *victim, OBJ_DATA *target )
{
    if ( HAS_PROG(mobj->pIndexData, ACT_PROG) )
    {
	MPROG_ACT_LIST *tmp_act;
      
	CREATE(tmp_act, MPROG_ACT_LIST, 1);
	if ( mobj->mpactnum > 0 )
	    tmp_act->next = mobj->mpact;
	else
	    tmp_act->next = NULL;
      
	mobj->mpact = tmp_act;
	mobj->mpact->buf = str_dup(buf);
	mobj->mpact->ch = ch;
	mobj->mpact->obj = obj;
	mobj->mpact->victim = victim;
	mobj->mpact->target = target;
	mobj->mpactnum++;
	obj_act_add(mobj);
     }
}

bool oprog_wordlist_check( char *arg, CHAR_DATA *mob, CHAR_DATA *actor,
			  OBJ_DATA *obj, CHAR_DATA *victim, OBJ_DATA *target, int type, OBJ_DATA *iobj )
{
  char        temp1[ MAX_STRING_LENGTH ];
  char        temp2[ MAX_INPUT_LENGTH ];
  char        word[ MAX_INPUT_LENGTH ];
  MPROG_DATA *mprg;
  char       *list;
  char       *start;
  char       *dupl;
  char       *end;
  int         i;
  bool	      executed = FALSE;


  for ( mprg = iobj->pIndexData->mudprogs; mprg; mprg = mprg->next )
    if ( mprg->type == type )
      {
	strcpy( temp1, mprg->arglist );
	list = temp1;
	for ( i = 0; i < strlen( list ); i++ )
	  list[i] = LOWER( list[i] );
	strcpy( temp2, arg );
	dupl = temp2;
	for ( i = 0; i < strlen( dupl ); i++ )
	  dupl[i] = LOWER( dupl[i] );
	if ( ( list[0] == 'p' ) && ( list[1] == ' ' ) )
	  {
	    list += 2;
	    while ( ( start = strstr( dupl, list ) ) )
	      if ( (start == dupl || *(start-1) == ' ' )
		  && ( *(end = start + strlen( list ) ) == ' '
		      || *end == '\n'
		      || *end == '\r'
		      || *end == '\0' ) )
		{
		  set_supermob( iobj );
		  mprog_driver( mprg->comlist, mob, actor, obj, victim, target, FALSE );
		  executed = TRUE;
		  release_supermob() ;
		  break;
		}
	      else
		dupl = start+1;
	  }
	else
	  {
	    list = one_argument( list, word );
	    for( ; word[0] != '\0'; list = one_argument( list, word ) )
	      while ( ( start = strstr( dupl, word ) ) )
		if ( ( start == dupl || *(start-1) == ' ' )
		    && ( *(end = start + strlen( word ) ) == ' '
			|| *end == '\n'
			|| *end == '\r'
			|| *end == '\0' ) )
		  {
		    set_supermob( iobj );
		    mprog_driver( mprg->comlist, mob, actor, obj, victim, target, FALSE );
		    executed = TRUE;
		    release_supermob();
		    break;
		  }
		else
		  dupl = start+1;
	  }
      }

  return executed;
}



/*
 *  room_prog support starts here
 *
 *
 */

void rset_supermob( ROOM_INDEX_DATA *room)
{
  char buf[200];

  if (room)
  {
    STRFREE(supermob->short_descr);
    supermob->short_descr = QUICKLINK(room->name);
    STRFREE(supermob->name);
    supermob->name        = QUICKLINK(room->name);
    supermob->mpscriptpos = room->mpscriptpos;

    /* Added by Jenny to allow bug messages to show the vnum
       of the room, and not just supermob's vnum */
    sprintf( buf, "Room #%d", room->vnum );
    STRFREE( supermob->description );
    supermob->description = STRALLOC( buf );

    char_from_room (supermob );
    char_to_room( supermob, room); 
  }
}


void rprog_percent_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj,
			  CHAR_DATA *victim, OBJ_DATA *target, int type)
{
    MPROG_DATA * mprg;

    if(!mob->in_room)
	return;

    for ( mprg = mob->in_room->mudprogs; mprg; mprg = mprg->next )
	if ( mprg->type == type
	&&   number_percent() <= atoi(mprg->arglist) )
	{
	    mprog_driver( mprg->comlist, mob, actor, obj, victim, target, FALSE );
	    if( type!=ENTER_PROG )
		break;
	}
}

/*
 * Triggers follow
 */


/*
 *  Hold on this
 * Unhold. -- Alty
 */
void room_act_add( ROOM_INDEX_DATA *room );
void rprog_act_trigger( char *buf, ROOM_INDEX_DATA *room, CHAR_DATA *ch,
			OBJ_DATA *obj, CHAR_DATA *victim, OBJ_DATA *target )
{
    if ( HAS_PROG(room, ACT_PROG) )
    {
	MPROG_ACT_LIST *tmp_act;
      
	CREATE(tmp_act, MPROG_ACT_LIST, 1);
	if ( room->mpactnum > 0 )
	    tmp_act->next = room->mpact;
	else
	    tmp_act->next = NULL;
      
	room->mpact = tmp_act;
	room->mpact->buf = str_dup(buf);
	room->mpact->ch = ch;
	room->mpact->obj = obj;
	room->mpact->victim = victim;
	room->mpact->target = target;
	room->mpactnum++;
	room_act_add(room);
    }
}
/*
 *
 */


void rprog_leave_trigger( CHAR_DATA *ch )
{
    if ( HAS_PROG(ch->in_room, LEAVE_PROG) )
    {
	rset_supermob( ch->in_room );
	rprog_percent_check( supermob, ch, NULL, NULL, NULL, LEAVE_PROG );
	release_supermob();
    }
}

/* login and void room triggers by Edmond */
void rprog_login_trigger( CHAR_DATA *ch )
{
    if ( HAS_PROG(ch->in_room, LOGIN_PROG) )
    {
        rset_supermob( ch->in_room );
        rprog_percent_check( supermob, ch, NULL, NULL, NULL, LOGIN_PROG );
        release_supermob();
    }
}

void rprog_void_trigger( CHAR_DATA *ch )
{
    if ( HAS_PROG(ch->in_room, VOID_PROG) )
    {
        rset_supermob( ch->in_room );
        rprog_percent_check( supermob, ch, NULL, NULL, NULL, VOID_PROG );
        release_supermob();
    }
}
void rprog_enter_trigger( CHAR_DATA *ch )
{
    if ( HAS_PROG(ch->in_room, ENTER_PROG) )
    {
	rset_supermob( ch->in_room );
	rprog_percent_check( supermob, ch, NULL, NULL, NULL, ENTER_PROG );
	release_supermob();
    }
}

void rprog_imminfo_trigger( CHAR_DATA *ch )
{
    if ( HAS_PROG(ch->in_room, IMMINFO_PROG) )
    {
        rset_supermob( ch->in_room );
        rprog_percent_check( supermob, ch, NULL, NULL, NULL, IMMINFO_PROG );
        release_supermob();
    }
}

void rprog_sleep_trigger( CHAR_DATA *ch )
{
    if ( HAS_PROG(ch->in_room, SLEEP_PROG) )
    {
	rset_supermob( ch->in_room );
	rprog_percent_check( supermob, ch, NULL, NULL, NULL, SLEEP_PROG );
	release_supermob();
    }
}

void rprog_rest_trigger( CHAR_DATA *ch )
{
   if( HAS_PROG(ch->in_room, REST_PROG) )
   {
	rset_supermob( ch->in_room );
	rprog_percent_check( supermob, ch, NULL, NULL, NULL, REST_PROG );
	release_supermob();
   }
}

void rprog_rfight_trigger( CHAR_DATA *ch )
{
   if( HAS_PROG(ch->in_room, RFIGHT_PROG) )
   {
	rset_supermob( ch->in_room );
	rprog_percent_check( supermob, ch, NULL, NULL, NULL, RFIGHT_PROG );
	release_supermob();
   }
}

void rprog_death_trigger( CHAR_DATA *killer, CHAR_DATA *ch )
{
   if( HAS_PROG(ch->in_room, RDEATH_PROG) )
   {
	rset_supermob( ch->in_room );
	rprog_percent_check( supermob, ch, NULL, NULL, NULL, RDEATH_PROG );
	release_supermob();
   }
}

void rprog_speech_trigger( char *txt, CHAR_DATA *ch )
{
   if( HAS_PROG(ch->in_room, SPEECH_PROG) )
   {
	/* supermob is set and released in rprog_wordlist_check */
	rprog_wordlist_check( txt, supermob, ch, NULL, NULL, NULL, SPEECH_PROG, ch->in_room );
   }
}

bool rprog_command_trigger( CHAR_DATA *ch, char *txt )
{
   if( HAS_PROG(ch->in_room, CMD_PROG) )
   {
	/* supermob is set and released in rprog_wordlist_check */
	if ( rprog_wordlist_check( txt, supermob, ch, NULL, NULL, NULL, CMD_PROG, ch->in_room ) )
		return TRUE;
   }
	return FALSE;
}

void rprog_random_trigger( CHAR_DATA *ch )
{
   if ( HAS_PROG(ch->in_room, RAND_PROG) )
   {
	rset_supermob( ch->in_room );
	rprog_percent_check(supermob,ch,NULL,NULL,NULL,RAND_PROG);
	release_supermob();
   }
}

bool rprog_wordlist_check( char *arg, CHAR_DATA *mob, CHAR_DATA *actor,
			  OBJ_DATA *obj, CHAR_DATA *victim, OBJ_DATA *target, int type, ROOM_INDEX_DATA *room )
{

  char        temp1[ MAX_STRING_LENGTH ];
  char        temp2[ MAX_INPUT_LENGTH ];
  char        word[ MAX_INPUT_LENGTH ];
  MPROG_DATA *mprg;
  char       *list;
  char       *start;
  char       *dupl;
  char       *end;
  int         i;
  bool	      executed = FALSE;

  if ( actor && !char_died(actor) && actor->in_room )
    room = actor->in_room;

  for ( mprg = room->mudprogs; mprg; mprg = mprg->next )
    if ( mprg->type == type )
      {
	strcpy( temp1, mprg->arglist );
	list = temp1;
	for ( i = 0; i < strlen( list ); i++ )
	  list[i] = LOWER( list[i] );
	strcpy( temp2, arg );
	dupl = temp2;
	for ( i = 0; i < strlen( dupl ); i++ )
	  dupl[i] = LOWER( dupl[i] );
	if ( ( list[0] == 'p' ) && ( list[1] == ' ' ) )
	  {
	    list += 2;
	    while ( ( start = strstr( dupl, list ) ) )
	      if ( (start == dupl || *(start-1) == ' ' )
		  && ( *(end = start + strlen( list ) ) == ' '
		      || *end == '\n'
		      || *end == '\r'
		      || *end == '\0' ) )
		{
		  rset_supermob( room );
		  mprog_driver( mprg->comlist, mob, actor, obj, victim, target, FALSE );
		  executed = TRUE;
		  release_supermob() ;
		  break;
		}
	      else
		dupl = start+1;
	  }
	else
	  {
	    list = one_argument( list, word );
	    for( ; word[0] != '\0'; list = one_argument( list, word ) )
	      while ( ( start = strstr( dupl, word ) ) )
		if ( ( start == dupl || *(start-1) == ' ' )
		    && ( *(end = start + strlen( word ) ) == ' '
			|| *end == '\n'
			|| *end == '\r'
			|| *end == '\0' ) )
		  {
		    rset_supermob( room );
		    mprog_driver( mprg->comlist, mob, actor, obj, victim, target, FALSE );
  		    executed = TRUE;
		    release_supermob();
		    break;
		  }
		else
		  dupl = start+1;
	  }
      }
      return executed;
}

void rprog_time_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj,
			ROOM_INDEX_DATA *room, int type )
{
  MPROG_DATA * mprg;
  bool 	       trigger_time;

  for ( mprg = room->mudprogs; mprg; mprg = mprg->next )
  {
    trigger_time = ( time_info.hour == atoi( mprg->arglist ) );

    if ( !trigger_time )
    {
      if ( mprg->triggered )
        mprg->triggered = FALSE;
      continue;
    }

    if ( mprg->type == type
	&& ( ( !mprg->triggered ) || ( mprg->type == HOUR_PROG ) ) )
    {
      mprg->triggered = TRUE;
      mprog_driver( mprg->comlist, mob, actor, obj, NULL, NULL, FALSE );
    }
  }
  return;
}

void rprog_time_trigger( CHAR_DATA *ch )
{
   if ( HAS_PROG(ch->in_room, TIME_PROG))
   {
	rset_supermob( ch->in_room );
	rprog_time_check( supermob, NULL, NULL, ch->in_room, TIME_PROG );
	release_supermob();
   }
}

void rprog_hour_trigger( CHAR_DATA *ch )
{
   if ( HAS_PROG(ch->in_room, HOUR_PROG))
   {
	rset_supermob( ch->in_room );
	rprog_time_check( supermob, NULL, NULL, ch->in_room, HOUR_PROG );
	release_supermob();
   }
}
 
/* Written by Jenny, Nov 29/95 */
void progbug( char *str, CHAR_DATA *mob )
{
    char buf[MAX_STRING_LENGTH];
    int vnum = mob->pIndexData ? mob->pIndexData->vnum : 0;

    /*
     * Check if we're dealing with supermob, which means the bug occurred
     * in a room or obj prog.
     */
    if ( vnum == 3 )
    {
	/*
	 * It's supermob.  In set_supermob and rset_supermob, the description
	 * was set to indicate the object or room, so we just need to show
	 * the description in the bug message.
	 */
	sprintf( buf, "%s, %s.", str, mob->description == NULL ? "(unknown)" : mob->description );
    }
    else
    {
	sprintf( buf, "%s, Mob #%d.", str, vnum );
    }

    bug( buf, 0 );
    return;
}


/* Room act prog updates.  Use a separate list cuz we dont really wanna go
   thru 5-10000 rooms every pulse.. can we say lag? -- Alty */

void room_act_add( ROOM_INDEX_DATA *room )
{
    struct act_prog_data *runner;
  
    for ( runner = room_act_list; runner; runner = runner->next )
	if ( runner->vo == room )
	   return;
    CREATE(runner, struct act_prog_data, 1);
    runner->vo = room;
    runner->next = room_act_list;
    room_act_list = runner;
}


void room_act_update( void )
{
  struct act_prog_data *runner;
  MPROG_ACT_LIST *mpact;
  
  while ( (runner = room_act_list) != NULL )
  {
    ROOM_INDEX_DATA *room = runner->vo;
    
    while ( (mpact = room->mpact) != NULL )
    {
      if ( mpact->ch->in_room == room )
        rprog_wordlist_check(mpact->buf, supermob, mpact->ch, mpact->obj,
                             mpact->victim, mpact->target, ACT_PROG, room);
      room->mpact = mpact->next;
      DISPOSE(mpact->buf);
      DISPOSE(mpact);
    }
    room->mpact = NULL;
    room->mpactnum = 0;
    room_act_list = runner->next;
    DISPOSE(runner);
  }
  return;
}

void obj_act_add( OBJ_DATA *obj )
{
  struct act_prog_data *runner;
  
  for ( runner = obj_act_list; runner; runner = runner->next )
    if ( runner->vo == obj )
      return;
  CREATE(runner, struct act_prog_data, 1);
  runner->vo = obj;
  runner->next = obj_act_list;
  obj_act_list = runner;
}
void obj_act_update( void )
{
  struct act_prog_data *runner;
  MPROG_ACT_LIST *mpact;
  
  while ( (runner = obj_act_list) != NULL )
  {
    OBJ_DATA *obj = runner->vo;
    
    while ( (mpact = obj->mpact) != NULL )
    {
      oprog_wordlist_check(mpact->buf, supermob, mpact->ch, mpact->obj,
                           mpact->victim, mpact->target, ACT_PROG, obj);
      obj->mpact = mpact->next;
      DISPOSE(mpact->buf);
      DISPOSE(mpact);
    }
    obj->mpact = NULL;
    obj->mpactnum = 0;
    obj_act_list = runner->next;
    DISPOSE(runner);
  }
  return;
}
