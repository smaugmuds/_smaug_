/**************************************************************************\
 *                                                                        *
 *     OasisOLC II for Smaug 1.40 written by Evan Cortens(Tagith)         *
 *                                                                        *
 *   Based on OasisOLC for CircleMUD3.0bpl9 written by Harvey Gilpin      *
 *                                                                        *
 **************************************************************************
 *                                                                        *
 *                    Object editing module (oedit.c)                     *
 *                                                                        *
\**************************************************************************/

#include <stdio.h>
#include <string.h>
#include "mud.h"
#include "olc.h"

/* External functions */
extern int top_affect;
extern int top_ed;
extern int top_obj_index;
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];

int  get_risflag			args( ( char *flag ) );
void medit_disp_aff_flags		args( ( DESCRIPTOR_DATA *d ) );
void medit_disp_ris			args( ( DESCRIPTOR_DATA *d ) );

/* Internal functions */
DECLARE_DO_FUN( do_oedit_reset );
void oedit_disp_layer_menu		args( ( DESCRIPTOR_DATA *d ) );
void oedit_disp_container_flags_menu	args( ( DESCRIPTOR_DATA *d ) );
void oedit_disp_lever_flags_menu	args( ( DESCRIPTOR_DATA *d ) );
void oedit_disp_extradesc_menu		args( ( DESCRIPTOR_DATA *d ) );
void oedit_disp_weapon_menu		args( ( DESCRIPTOR_DATA *d ) );
void oedit_disp_val1_menu		args( ( DESCRIPTOR_DATA *d ) );
void oedit_disp_val2_menu		args( ( DESCRIPTOR_DATA *d ) );
void oedit_disp_val3_menu		args( ( DESCRIPTOR_DATA *d ) );
void oedit_disp_val4_menu		args( ( DESCRIPTOR_DATA *d ) );
void oedit_disp_val5_menu		args( ( DESCRIPTOR_DATA *d ) );
void oedit_disp_val6_menu		args( ( DESCRIPTOR_DATA *d ) );
void oedit_disp_type_menu		args( ( DESCRIPTOR_DATA *d ) );
void oedit_disp_extra_menu		args( ( DESCRIPTOR_DATA *d ) );
void oedit_disp_wear_menu		args( ( DESCRIPTOR_DATA *d ) );
void oedit_disp_menu			args( ( DESCRIPTOR_DATA *d ) );

void oedit_parse			args( ( DESCRIPTOR_DATA *d, char *arg) );
void oedit_disp_spells_menu		args( ( DESCRIPTOR_DATA *d ) );
void oedit_liquid_type			args( ( DESCRIPTOR_DATA *d ) );
void oedit_setup_new			args( ( DESCRIPTOR_DATA *d ) );
void oedit_setup_existing		args( ( DESCRIPTOR_DATA *d, int real_num) );
void oedit_save_to_disk			args( ( DESCRIPTOR_DATA *d ) ); /* Unused */
void oedit_save_internally		args( ( DESCRIPTOR_DATA *d ) );

void oedit_setup			args( ( DESCRIPTOR_DATA *d, int vnum ) );

/*------------------------------------------------------------------------*/
void cleanup_olc( DESCRIPTOR_DATA *d )
{
    if ( d->olc )
    {
	if ( d->character )
	{
	    d->character->dest_buf = NULL;
	    act( AT_ACTION, "$n stops using OLC.", d->character, NULL, NULL, TO_CANSEE );
	}
	d->connected = CON_PLAYING;
	DISPOSE( d->olc );
    }
    return;
}

/*
 * Starts it all off
 */
void do_ooedit( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    OBJ_DATA *obj;

#ifdef ENABLE_OLC2_EXTRAS
    AREA_DATA              *pArea;
    int                     cvnum,
                            vnum;
    OBJ_INDEX_DATA         *pObjIndex;
#endif

    if ( IS_NPC(ch) )
    {
	send_to_char( "I don't think so...\n\r", ch );
	return;
    }

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
#ifndef ENABLE_OLC2_EXTRAS
    {
	send_to_char( "OEdit what?\n\r", ch );
	return;
    }
#else
    {
        pArea = ch->in_room->area;
        vnum = pArea->low_o_vnum;

        if ( !pArea )
        {
            bug( "object: !pArea", 0 );
            return;
        }

        while ( vnum <= pArea->hi_o_vnum && get_obj_index( vnum ) != NULL )
            vnum++;
        if ( vnum > pArea->hi_o_vnum )
        {
            send_to_char
                ( "&GYou cannot create any more objects as you have used all that your area is alloted.\r\n",
                  ch );
            return;
        }
        pObjIndex = make_object( vnum, cvnum, argument );
        pArea = ch->pcdata->area;

        if ( !pObjIndex )
        {
            send_to_char( "Error.\n\r", ch );
            log_string( "do_ocreate: make_object failed." );
            return;
        }
        obj = create_object( pObjIndex, get_trust( ch ) );
        obj_to_char( obj, ch );
        act( AT_IMMORT, "$n makes arcane gestures, and opens $s hands to reveal $p!", ch,
             obj, NULL, TO_ROOM );
        obj->name = STRALLOC( "object" );
    }

    else
    {
#endif

    if ( (obj = get_obj_world( ch, arg ) ) == NULL )
    {
			send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
			return;
    }

#ifdef ENABLE_OLC2_EXTRAS
	}
#endif

    /* Make sure the object isnt already being edited */
    for ( d = first_descriptor; d; d = d->next )
	if ( d->connected == CON_OEDIT )
	    if ( d->olc && OLC_VNUM(d) == obj->pIndexData->vnum )
	    {
		ch_printf( ch, "That object is currently being edited by %s.\n\r", d->character->name );
		return;
	    }

    if ( !can_omodify( ch, obj ) )
	return;

    d = ch->desc;
    CREATE( d->olc, OLC_DATA, 1 );
    OLC_VNUM(d) = obj->pIndexData->vnum;
    OLC_CHANGE(d) = FALSE;
    OLC_VAL(d) = 0;
    d->character->dest_buf = obj;
    d->connected = CON_OEDIT;
    oedit_disp_menu( d );

    act( AT_ACTION, "$n starts using OLC.", ch, NULL, NULL, TO_CANSEE );
    return;
}

void do_ocopy( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int ovnum, cvnum;
    OBJ_INDEX_DATA *orig;
    OBJ_INDEX_DATA *copy;
    EXTRA_DESCR_DATA *ed, *ced;
    AFFECT_DATA *paf, *cpaf;
    MPROG_DATA *mprog, *cprog;
    int iHash;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( !arg1 || !arg2 )
    {
	send_to_char( "Usage: ocopy <original> <new>\n\r", ch );
        return;
    }
    ovnum = atoi( arg1 );
    cvnum = atoi( arg2 );
    if ( get_trust( ch ) < LEVEL_LESSER )
    {
        AREA_DATA *pArea;

        if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
        {
            send_to_char( "You must have an assigned area to copy objects.\n\r", ch );
            return;
        }
        if ( cvnum < pArea->low_o_vnum
        ||   cvnum > pArea->hi_o_vnum )
	{
            send_to_char( "That number is not in your allocated range.\n\r", ch );
            return;
        }
    }

    if ( get_obj_index( cvnum ) )
    {
        send_to_char( "That object already exists.\n\r", ch );
        return;
    }

    if ( (orig = get_obj_index( ovnum ) ) == NULL )
    {
	send_to_char( "How can you copy something that doesnt exist?\n\r", ch );
	return;
    }
    CREATE( copy, OBJ_INDEX_DATA, 1 );
    copy->vnum			= cvnum;
    copy->name			= QUICKLINK( orig->name );
    copy->first_affect		= NULL;
    copy->last_affect		= NULL;
    copy->first_extradesc 	= NULL;
    copy->last_extradesc	= NULL;
    copy->short_descr		= QUICKLINK( orig->short_descr );
    copy->description		= QUICKLINK( orig->description );
    copy->action_desc		= QUICKLINK( orig->action_desc );
    copy->item_type		= orig->item_type;
    copy->extra_flags		= orig->extra_flags;
    copy->magic_flags		= orig->magic_flags;
    copy->wear_flags		= orig->wear_flags;
    copy->value[0]		= orig->value[0];
    copy->value[1]		= orig->value[1];
    copy->value[2]		= orig->value[2];
    copy->value[3]		= orig->value[3];
    copy->value[4]		= orig->value[4];
    copy->value[5]		= orig->value[5];
    copy->weight		= orig->weight;
    copy->cost			= orig->cost;
    copy->rent			= orig->rent;
    copy->layers		= orig->layers;

    for ( ced = orig->first_extradesc; ced; ced = ced->next )
    {
	CREATE( ed, EXTRA_DESCR_DATA, 1 );
	ed->keyword		= QUICKLINK( ced->keyword );
	ed->description		= QUICKLINK( ced->description );
	LINK( ed, copy->first_extradesc, copy->last_extradesc, next, prev );
	top_ed++;
    }
    for ( cpaf = orig->first_affect; cpaf; cpaf = cpaf->next )
    {
	CREATE( paf, AFFECT_DATA, 1 );
	paf->type               = cpaf->type;
	paf->duration           = cpaf->duration;
	paf->location           = cpaf->location;
	paf->modifier           = cpaf->modifier;
	paf->bitvector          = cpaf->bitvector;
	LINK( paf, copy->first_affect, copy->last_affect, next, prev );
	top_affect++;
    }
    if ( orig->mudprogs )
    {
        CREATE( mprog, MPROG_DATA, 1 );
        copy->mudprogs = mprog;

        for ( cprog = orig->mudprogs; cprog; cprog = cprog->next )
        {
            mprog->type 		= cprog->type;
            xSET_BIT( copy->progtypes, mprog->type );
            mprog->arglist		= QUICKLINK( cprog->arglist );
            mprog->comlist		= QUICKLINK( cprog->comlist );
            if ( cprog->next )
            {
                CREATE( mprog->next, MPROG_DATA, 1 );
                mprog = mprog->next;
            }
            else
                mprog->next = NULL;
        }
    }

    copy->count			= 0;
    iHash			= cvnum % MAX_KEY_HASH;
    copy->next			= obj_index_hash[iHash];
    obj_index_hash[iHash]	= copy;
    top_obj_index++;

    set_char_color( AT_PLAIN, ch );
    send_to_char( "Object copied.\n\r", ch );

    return;
}

/**************************************************************************
 Menu functions 
 **************************************************************************/

void oedit_disp_progs( DESCRIPTOR_DATA *d )
{
    OBJ_DATA *obj = d->character->dest_buf;
    MPROG_DATA *mprg;
    int count;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for ( count = 0, mprg = obj->pIndexData->mudprogs; mprg; mprg = mprg->next )
    {
	ch_printf_color( d->character, "&g%2d&w) &c%s\n\r", ++count, mprog_type_to_name( mprg->type ) );
    }

    if ( obj->pIndexData->mudprogs )
	send_to_char( "\n\r", d->character );

    ch_printf_color( d->character, "&gA&w) Add a new program\n\r" );
    ch_printf_color( d->character, "&gR&w) Remove an existing program\n\r" );
    ch_printf_color( d->character, "&gQ&w) Quit\n\r" );
    ch_printf_color( d->character, "\n\rEnter choice: " );

    OLC_MODE(d) = OEDIT_MPROGS;
}

void oedit_disp_prog_types( DESCRIPTOR_DATA *d )
{
    int count;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for ( count = ACT_PROG; count < USE_PROG; count++ )
	ch_printf_color( d->character, "&g%2d&w) %s\n\r", count, mprog_type_to_name(count) );
    send_to_char( "\n\rEnter type: ", d->character );

    return;
}

void oedit_disp_prog_choice( DESCRIPTOR_DATA *d )
{
    MPROG_DATA *mprg = d->character->spare_ptr;
    char buf[MAX_STRING_LENGTH];
    int i = 0;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    if ( mprg->comlist )
    {
	strcat( buf, "\n\r" );
	while ( mprg->comlist[i++] != '\n' )
	    sprintf( buf, "%s%c", buf, mprg->comlist[i] );
    }
    else
	buf[0] = '\0';

    ch_printf_color( d->character, "&gA&w) Type: &c%s\n\r", mprog_type_to_name(mprg->type) );
    ch_printf_color( d->character, "&gB&w) Args: &c%s\n\r", mprg->arglist );
    ch_printf_color( d->character, "&gC&w) Prog: %s\n\r", buf );
    ch_printf_color( d->character, "&gQ&w) Quit\n\r" );

    send_to_char( "\n\rEnter choice: ", d->character );
    return;
}

/* For container flags */
void oedit_disp_container_flags_menu( DESCRIPTOR_DATA *d )
{
    OBJ_DATA *obj = d->character->dest_buf;
    CHAR_DATA *ch = d->character;
    char buf[MAX_STRING_LENGTH];
    int i;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for ( i = 0; i < 5; i++ )
    {
	sprintf( buf, "&g%d&w) %s\n\r", i+1, container_flags[i] );
	send_to_char_color( buf, ch );
    }
    sprintf( buf, "Container flags: &c%s&w\n\r", flag_string( obj->value[1], container_flags ) );
    send_to_char_color( buf, ch );

    send_to_char( "Enter flag, 0 to quit : ", ch );
}

/*
 * Display lever flags menu
 */
void oedit_disp_lever_flags_menu( DESCRIPTOR_DATA *d )
{
    OBJ_DATA *obj = d->character->dest_buf;
    char buf[MAX_STRING_LENGTH];
    int counter;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for ( counter = 0; counter < 29; counter++ )
    {
	sprintf( buf, "&g%2d&w) %s\n\r", counter+1, trig_flags[counter] );
	send_to_char_color( buf, d->character );
    }
    sprintf( buf, "Lever flags: &c%s&w\n\rEnter flag, 0 to quit: ", flag_string( obj->value[0] , trig_flags ) );
    send_to_char_color( buf, d->character );
    return;
}

/*
 * Fancy layering stuff, trying to lessen confusion :)
 */
void oedit_disp_layer_menu( DESCRIPTOR_DATA *d )
{
    OBJ_DATA *obj = d->character->dest_buf;

    OLC_MODE(d) = OEDIT_LAYERS;
    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    send_to_char( "Choose which layer, or combination of layers fits best: \n\r\n\r", d->character );
    ch_printf_color( d->character, "[&c%s&w] &g1&w) Nothing Layers\n\r", (obj->pIndexData->layers == 0) ? "X" : " " );
    ch_printf_color( d->character, "[&c%s&w] &g2&w) Silk Shirt\n\r", IS_SET( obj->pIndexData->layers,   1 ) ? "X" : " " );
    ch_printf_color( d->character, "[&c%s&w] &g3&w) Leather Vest\n\r", IS_SET( obj->pIndexData->layers,   2 ) ? "X" : " " );
    ch_printf_color( d->character, "[&c%s&w] &g4&w) Light Chainmail\n\r", IS_SET( obj->pIndexData->layers,   4 ) ? "X" : " " );
    ch_printf_color( d->character, "[&c%s&w] &g5&w) Leather Jacket\n\r", IS_SET( obj->pIndexData->layers,   8 ) ? "X" : " " );
    ch_printf_color( d->character, "[&c%s&w] &g6&w) Light Cloak\n\r", IS_SET( obj->pIndexData->layers,  16 ) ? "X" : " " );
    ch_printf_color( d->character, "[&c%s&w] &g7&w) Loose Cloak\n\r", IS_SET( obj->pIndexData->layers,  32 ) ? "X" : " " );
    ch_printf_color( d->character, "[&c%s&w] &g8&w) Cape\n\r", IS_SET( obj->pIndexData->layers,  64 ) ? "X" : " " );
    ch_printf_color( d->character, "[&c%s&w] &g9&w) Magical Effects\n\r", IS_SET( obj->pIndexData->layers, 128 ) ? "X" : " " );
    ch_printf_color( d->character, "\n\rLayer or 0 to exit: " );
}

/* For extra descriptions */
void oedit_disp_extradesc_menu( DESCRIPTOR_DATA *d )
{
    OBJ_DATA *obj = d->character->dest_buf;
    CHAR_DATA *ch = d->character;
    int count = 0;
  
    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    if ( obj->pIndexData->first_extradesc )
    {
	EXTRA_DESCR_DATA *ed;
	for ( ed = obj->pIndexData->first_extradesc; ed; ed = ed->next )
	{
	    ch_printf_color( ch, "&g%2d&w) Keyword: &O%s\n\r", ++count, ed->keyword );
	}
    }
    if ( obj->first_extradesc )
    {
	EXTRA_DESCR_DATA *ed;
	for ( ed = obj->first_extradesc; ed; ed = ed->next )
	{
	    ch_printf_color( ch, "&g%2d&w) Keyword: &O%s\n\r", ++count, ed->keyword );
        }
    }

    if ( obj->pIndexData->first_extradesc || obj->first_extradesc )
	send_to_char( "\n\r", d->character );

    ch_printf_color( d->character, "&gA&w) Add a new description\n\r" );
    ch_printf_color( d->character, "&gR&w) Remove a description\n\r" );
    ch_printf_color( d->character, "&gQ&w) Quit\n\r" );
    ch_printf_color( d->character, "\n\rEnter choice: " );

    OLC_MODE(d) = OEDIT_EXTRADESC_MENU;
}

void oedit_disp_extra_choice( DESCRIPTOR_DATA *d )
{
    EXTRA_DESCR_DATA *ed = d->character->spare_ptr;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    ch_printf_color( d->character, "&g1&w) Keyword: &O%s\n\r", ed->keyword );
    ch_printf_color( d->character, "&g2&w) Description: \n\r&O%s&w\n\r", ed->description );
    ch_printf_color( d->character, "\n\rChange which option? " );

    OLC_MODE(d) = OEDIT_EXTRADESC_CHOICE;
}

/* Ask for *which* apply to edit and prompt for some other options */
void oedit_disp_prompt_apply_menu( DESCRIPTOR_DATA *d )
{
    OBJ_DATA *obj = d->character->dest_buf;
    CHAR_DATA *ch = d->character;
    AFFECT_DATA *paf;
    int counter = 0;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
    {
	ch_printf_color( d->character, " &g%2d&w) ", counter++ );
	showaffect( ch, paf );
    }

    for ( paf = obj->first_affect; paf; paf = paf->next )
    {
	ch_printf_color( d->character, " &g%2d&w) ", counter++ );
        showaffect( ch, paf );
    }
    send_to_char_color( " \n\r &gA&w) Add an affect\n\r", ch );
    send_to_char_color( " &gR&w) Remove an affect\n\r", ch );
    send_to_char_color( " &gQ&w) Quit\n\r", ch );

    send_to_char( "\n\rEnter option or affect#: ", ch );
    OLC_MODE(d) = OEDIT_AFFECT_MENU;

    return;
}

/*. Ask for liquid type .*/
void oedit_liquid_type( DESCRIPTOR_DATA *d )
{
    char buf[MAX_STRING_LENGTH];
    int counter, col = 0;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for ( counter = 0; counter < LIQ_MAX; counter++ )
    {
	sprintf( buf, " &w%2d&g) &c%-20.20s ", counter, liq_table[counter].liq_name );
	if ( ++col % 3 == 0 )
	    strcat( buf, "\n\r" );
	send_to_char_color( buf, d->character );
    }
    send_to_char_color( "\n\r&wEnter drink type: ", d->character );
    OLC_MODE(d) = OEDIT_VALUE_3;

    return;
}

/*
 * Display the menu of apply types
 */
void oedit_disp_affect_menu( DESCRIPTOR_DATA *d )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *ch = d->character;
    int counter, col = 0;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for ( counter = 0; counter < MAX_APPLY_TYPE; counter++ )
    {
	/* Don't want people choosing these ones */
	if ( counter == 0 || counter == APPLY_EXT_AFFECT )
	    continue;
 
	sprintf( buf, "&g%2d&w) %-20.20s ", counter, a_types[counter] );
	if ( ++col % 3 == 0 )
	    strcat( buf, "\n\r" );
	send_to_char_color( buf, ch );
    }
    send_to_char( "\n\rEnter apply type (0 to quit): ", ch );
    OLC_MODE(d) = OEDIT_AFFECT_LOCATION;

    return;
}


/*
 * Display menu of weapon types
 */
void oedit_disp_weapon_menu( DESCRIPTOR_DATA *d )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *ch = d->character;
    int counter, col = 0;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for ( counter = 0; counter < 18; counter++ )
    {
	sprintf( buf, "&g%2d&w) %-20.20s ", counter, attack_table[counter] );
	if ( ++col % 2 == 0 )
	    strcat( buf, "\n\r" );
	send_to_char_color( buf, ch );
    }
    send_to_char( "\n\rEnter weapon type: ", ch );

    return;
}

/* spell type */
void oedit_disp_spells_menu( DESCRIPTOR_DATA *d )
{
    send_to_char( "Enter the name of the spell: ", d->character );
}

/* object value 0 */
void oedit_disp_val1_menu( DESCRIPTOR_DATA *d )
{
    OBJ_DATA *obj = d->character->dest_buf;
    OLC_MODE(d) = OEDIT_VALUE_1;

    switch (obj->item_type)
    {
    case ITEM_LIGHT:
	/* values 0 and 1 are unused.. jump to 2 */
	oedit_disp_val3_menu(d);
	break;
    case ITEM_SALVE:
    case ITEM_PILL:
    case ITEM_SCROLL:
    case ITEM_WAND:
    case ITEM_STAFF:
    case ITEM_POTION:
	send_to_char("Spell level : ", d->character);
	break;
    case ITEM_MISSILE_WEAPON:
    case ITEM_WEAPON:
	send_to_char( "Condition : ", d->character );
	break;
    case ITEM_ARMOR:
	send_to_char( "Current AC : ", d->character);
	break;
    /* case ITEM_QUIVER:
    case ITEM_KEYRING: */
    case ITEM_PIPE:
    case ITEM_CONTAINER:
    case ITEM_DRINK_CON:
    case ITEM_FOUNTAIN:
	send_to_char("Capacity : ", d->character);
	break;
    case ITEM_FOOD:
 	send_to_char("Hours to fill stomach : ", d->character);
	break;
    case ITEM_MONEY:
	send_to_char("Amount of gold coins : ", d->character);
	break;
    case ITEM_HERB:
	/* Value 0 unused, skip to 1 */
	oedit_disp_val2_menu(d);
	break;
    case ITEM_LEVER:
    case ITEM_SWITCH:
	oedit_disp_lever_flags_menu(d);
	break;
    case ITEM_TRAP:
	send_to_char( "Charges: ", d->character );
	break;

    default:
	oedit_disp_menu(d);
    }
}

/* object value 1 */
void oedit_disp_val2_menu( DESCRIPTOR_DATA *d )
{
    OBJ_DATA *obj = d->character->dest_buf;
    OLC_MODE(d) = OEDIT_VALUE_2;

    switch (obj->item_type)
    {
    case ITEM_PILL:
    case ITEM_SCROLL:
    case ITEM_POTION:
	oedit_disp_spells_menu(d);
	break;

    case ITEM_SALVE:
    case ITEM_HERB:
	send_to_char( "Charges: ", d->character );
	break;

    case ITEM_PIPE:
	send_to_char( "Number of draws: ", d->character );
	break;

    case ITEM_WAND:
    case ITEM_STAFF:
	send_to_char("Max number of charges : ", d->character);
	break;

    case ITEM_WEAPON:
	send_to_char("Number of damage dice : ", d->character);
	break;
    case ITEM_FOOD:
	send_to_char( "Condition: ", d->character );
	break;

    case ITEM_CONTAINER:
	oedit_disp_container_flags_menu(d);
	break;

    case ITEM_DRINK_CON:
    case ITEM_FOUNTAIN:
	send_to_char( "Quantity : ", d->character );
	break;

    case ITEM_ARMOR:
	send_to_char( "Original AC: ", d->character );
	break;

    case ITEM_LEVER:
    case ITEM_SWITCH:
	if ( IS_SET( obj->value[0], TRIG_CAST ) )
	    oedit_disp_spells_menu(d);
	else
	    send_to_char( "Vnum: ", d->character );
	break;

    default:
	oedit_disp_menu(d);
    }
}

/* object value 2 */
void oedit_disp_val3_menu( DESCRIPTOR_DATA *d )
{
    OBJ_DATA *obj = d->character->dest_buf;
    OLC_MODE(d) = OEDIT_VALUE_3;

    switch ( obj->item_type ) 
    {
    case ITEM_LIGHT:
 	send_to_char("Number of hours (0 = burnt, -1 is infinite) : ", d->character);
 	break;
    case ITEM_PILL:
    case ITEM_SCROLL:
    case ITEM_POTION:
 	oedit_disp_spells_menu(d);
 	break;
    case ITEM_WAND:
    case ITEM_STAFF:
 	send_to_char("Number of charges remaining : ", d->character);
 	break;
    case ITEM_WEAPON:
	send_to_char("Size of damage dice : ", d->character);
	break;
    case ITEM_CONTAINER:
	send_to_char("Vnum of key to open container (-1 for no key) : ", d->character);
	break;
    case ITEM_DRINK_CON:
    case ITEM_FOUNTAIN:
	oedit_liquid_type(d);
	break;
    default:
	oedit_disp_menu(d);
    }
}

/* object value 3 */
void oedit_disp_val4_menu( DESCRIPTOR_DATA *d )
{
    OBJ_DATA *obj = d->character->dest_buf;
    OLC_MODE(d) = OEDIT_VALUE_4;

    switch ( obj->item_type ) 
    {
    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_WAND:
    case ITEM_STAFF:
	oedit_disp_spells_menu(d);
	break;
    case ITEM_WEAPON:
  	oedit_disp_weapon_menu(d);
	break;
    case ITEM_DRINK_CON:
    case ITEM_FOUNTAIN:
    case ITEM_FOOD:
	send_to_char("Poisoned (0 = not poisoned) : ", d->character);
	break;
    default:
	oedit_disp_menu(d);
    }
}

/* object value 4 */
void oedit_disp_val5_menu( DESCRIPTOR_DATA *d )
{
    OBJ_DATA *obj = d->character->dest_buf;
    OLC_MODE(d) = OEDIT_VALUE_5;

    switch ( obj->item_type )
    {
    case ITEM_SALVE:
	oedit_disp_spells_menu(d);
	break;
    case ITEM_FOOD:
	send_to_char( "Food value: ", d->character );
	break;
    case ITEM_MISSILE_WEAPON:
	send_to_char( "Range: ", d->character );
	break;
    default:
	oedit_disp_menu(d);
    }
}

/* object value 5 */
void oedit_disp_val6_menu( DESCRIPTOR_DATA *d )
{
    OBJ_DATA *obj = d->character->dest_buf;
    OLC_MODE(d) = OEDIT_VALUE_6;

    switch ( obj->item_type )
    {
    case ITEM_SALVE:
	oedit_disp_spells_menu(d);
	break;
    default:
	oedit_disp_menu(d);
    }
}

/* object type */
void oedit_disp_type_menu( DESCRIPTOR_DATA *d )
{
    char buf[MAX_STRING_LENGTH];
    int counter, col = 0;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for ( counter = 0; counter < MAX_ITEM_TYPE; counter++ )
    {
	sprintf( buf, "&g%2d&w) %-20.20s ", counter, o_types[counter] );
	if ( ++col % 3 == 0 )
	    strcat( buf, "\n\r" );
	send_to_char_color( buf, d->character );
    }
    send_to_char( "\n\rEnter object type: ", d->character );

    return;
}

/* object extra flags */
void oedit_disp_extra_menu( DESCRIPTOR_DATA *d )
{
    OBJ_DATA *obj = d->character->dest_buf;
    char buf[MAX_STRING_LENGTH];
    int counter, col = 0;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for ( counter = 0; counter < MAX_ITEM_FLAG; counter++ )
    {
	sprintf( buf, "&g%2d&w) %-20.20s ", counter + 1, capitalize(o_flags[counter]) );
	if ( ++col % 2 == 0 )
	    strcat( buf, "\n\r" );
	send_to_char_color( buf, d->character );
    }
    sprintf( buf, "\n\rObject flags: &c%s&w\n\rEnter object extra flag (0 to quit): ",
	ext_flag_string( &obj->extra_flags, o_flags ) );
    send_to_char_color( buf, d->character );

    return;
}

/*
 * Display wear flags menu
 */
void oedit_disp_wear_menu( DESCRIPTOR_DATA *d )
{
    OBJ_DATA *obj = d->character->dest_buf;
    char buf[MAX_STRING_LENGTH];
    int counter, col = 0;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for ( counter = 0; counter <= ITEM_WEAR_MAX; counter++ )
    {
	if ( 1 << counter == ITEM_DUAL_WIELD )
	    continue;

	sprintf( buf, "&g%2d&w) %-20.20s ", counter+1, capitalize(w_flags[counter]) );
	if ( ++col % 2 == 0 )
	    strcat( buf, "\n\r" );
	send_to_char_color( buf, d->character );
    }
    sprintf( buf, "\n\rWear flags: &c%s&w\n\rEnter wear flag, 0 to quit:  ",
	flag_string( obj->wear_flags, w_flags ) );
    send_to_char_color( buf, d->character );

    return;
}


/* display main menu */
void oedit_disp_menu( DESCRIPTOR_DATA *d )
{
    OBJ_DATA *obj = d->character->dest_buf;
    char buf[MAX_STRING_LENGTH];

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    /*. Build first half of menu .*/
    set_char_color( AT_PLAIN, d->character );
    sprintf(buf,
            "-- Item number : [&c%d&w]\r\n"
            "&g1&w) Name     : &O%s\r\n"
            "&g2&w) S-Desc   : &O%s\r\n"
            "&g3&w) L-Desc   :-\r\n&O%s\r\n"
            "&g4&w) A-Desc   :-\r\n&O%s\n\r"
            "&g5&w) Type        : &c%s\r\n"
            "&g6&w) Extra flags : &c%s\r\n",

            obj->pIndexData->vnum,
            obj->name,
            obj->short_descr,
            obj->description,
            obj->action_desc ?  obj->action_desc : "<not set>\r\n",
            capitalize( item_type_name( obj ) ),
            ext_flag_string( &obj->extra_flags, o_flags )
           );
    send_to_char_color(buf, d->character);

    /* Build second half of the menu */
    sprintf(buf,
            "&g7&w) Wear flags  : &c%s\n\r"
            "&g8&w) Weight      : &c%d\n\r"
            "&g9&w) Cost        : &c%d\n\r"
            "&gA&w) Rent(unused): &c%d\n\r"
            "&gB&w) Timer       : &c%d\n\r"
            "&gC&w) Level       : &c%d\n\r" /* -- Object level .*/
            "&gD&w) Layers      : &c%d\n\r"
            "&gE&w) Values      : &c%d %d %d %d %d %d\n\r"
            "&gF&w) Affect menu\n\r"
            "&gG&w) Extra descriptions menu\n\r"
            "&gQ&w) Quit\n\r"
            "Enter choice : ",

            flag_string( obj->wear_flags, w_flags ),
            obj->weight,
            obj->cost,
            obj->pIndexData->rent,
            obj->timer,
            obj->level,
            obj->pIndexData->layers,
            obj->value[0],
            obj->value[1],
            obj->value[2],
            obj->value[3],
            obj->value[4],
            obj->value[5]
           );
    send_to_char_color( buf, d->character );
    OLC_MODE(d) = OEDIT_MAIN_MENU;

    return;
}
/***************************************************************************
 Object affect editing/removing functions
 ***************************************************************************/
void edit_object_affect( DESCRIPTOR_DATA *d, int number )
{
    OBJ_DATA *obj = d->character->dest_buf;
    int count = 0;
    AFFECT_DATA *paf;

    for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
    {
	if ( count == number )
	{
	    d->character->spare_ptr = paf;
	    OLC_VAL(d) = TRUE;
	    oedit_disp_affect_menu(d);
	    return;
	}
	count++;
    }
    for ( paf = obj->first_affect; paf; paf = paf->next )
    {
	if ( count == number )
 	{
	    d->character->spare_ptr = paf;
	    OLC_VAL(d) = TRUE;
	    oedit_disp_affect_menu(d);
	    return;
	}
	count++;
    }
    send_to_char( "Affect not found.\n\r", d->character );
    return;
}

void remove_affect_from_obj( OBJ_DATA *obj, int number )
{
    int count = 0;
    AFFECT_DATA *paf;

    if ( obj->pIndexData->first_affect )
    {
        for ( paf = obj->pIndexData->first_affect; 
	     paf; 
	     paf = paf->next )
        {
            if ( count == number )
            {
                UNLINK( paf, obj->pIndexData->first_affect, obj->pIndexData->last_affect, next, prev );
                DISPOSE( paf );
                --top_affect;
		return;
            }
            count++;
        }
    }

    if ( obj->first_affect )
    {
        for ( paf = obj->first_affect; 
	     paf; 
	     paf = paf->next )
        {
            if ( count == number )
            {
                UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
                DISPOSE( paf );
                --top_affect;
		return;
            }
            count++;
        }
    }
    return;
}

EXTRA_DESCR_DATA *oedit_find_extradesc( OBJ_DATA *obj, int number )
{
    int count = 0;
    EXTRA_DESCR_DATA *ed;

    for ( ed = obj->pIndexData->first_extradesc; ed; ed = ed->next )
    {
        if ( ++count == number )
	    return ed;
    }

    for ( ed = obj->first_extradesc; ed; ed = ed->next )
    {
        if ( ++count == number )
            return ed;
    }

    return NULL;
}

MPROG_DATA *oedit_find_prog( OBJ_DATA *obj, int number )
{
    MPROG_DATA *mprg;
    int count;

    for ( count = 0, mprg = obj->pIndexData->mudprogs; mprg; mprg = mprg->next )
    {
	if ( ++count == number )
	    return mprg;
    }
 
    return NULL;
}

/*
 * Bogus command for resetting stuff
 */
void do_oedit_reset( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj = ch->dest_buf;
    EXTRA_DESCR_DATA *ed = ch->spare_ptr;
    MPROG_DATA *mprg = ch->spare_ptr;
    int mode = OLC_MODE(ch->desc);

    switch ( ch->substate )
    {
    default:
        return;

    case SUB_OBJ_EXTRA:
        if ( !ch->dest_buf )
        {
            send_to_char( "Fatal error, report to Tagith.\n\r", ch );
            bug( "do_oedit_reset: sub_obj_extra: NULL ch->dest_buf", 0 );
            ch->substate = SUB_NONE;
            return;
        }
        /* OLC_DESC(ch->desc) = ch->spare_ptr; */
        STRFREE( ed->description );
        ed->description = copy_buffer( ch );
        stop_editing( ch );
        ch->dest_buf = obj;
	ch->spare_ptr = ed;
        ch->substate = SUB_NONE;
        ch->desc->connected = CON_OEDIT;
        OLC_MODE(ch->desc) = OEDIT_EXTRADESC_CHOICE;
        oedit_disp_extra_choice( ch->desc );
        return;

    case SUB_OBJ_LONG:
        if ( !ch->dest_buf )
        {
            send_to_char( "Fatal error, report to Tagith.\n\r", ch );
            bug( "do_oedit_reset: sub_obj_long: NULL ch->dest_buf", 0 );
            ch->substate = SUB_NONE;
            return;
        }
        STRFREE( obj->description );
        obj->description = copy_buffer( ch );
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
        {
            STRFREE( obj->pIndexData->description );
            obj->pIndexData->description = QUICKLINK( obj->description );
        }
        stop_editing( ch );
        ch->dest_buf = obj;
        ch->desc->connected = CON_OEDIT;
        ch->substate = SUB_NONE;
        OLC_MODE( ch->desc ) = OEDIT_MAIN_MENU;
        oedit_disp_menu( ch->desc );
        return;

    case SUB_MPROG_EDIT:
	if ( mprg->comlist )
	    STRFREE( mprg->comlist );
	mprg->comlist = copy_buffer( ch );
	stop_editing( ch );
	ch->dest_buf = obj;
	ch->desc->connected = ch->tempnum;
	ch->substate = SUB_NONE;
	OLC_MODE( ch->desc ) = mode;
	oedit_disp_prog_choice( ch->desc );
	return;
    }
}

/*
 * This function interprets the arguments that the character passed
 * to it based on which OLC mode you are in at the time
 */
void oedit_parse( DESCRIPTOR_DATA *d, char *arg )
{
    OBJ_DATA *obj = d->character->dest_buf;
    AFFECT_DATA *paf = d->character->spare_ptr;
    AFFECT_DATA *npaf;
    EXTRA_DESCR_DATA *ed = d->character->spare_ptr;
    MPROG_DATA *mprg = d->character->spare_ptr;
    MPROG_DATA *mprog = obj->pIndexData->mudprogs;
    char arg1[MAX_INPUT_LENGTH];
    int number = 0, max_val, min_val, value;
    /* bool found; */

    switch (OLC_MODE(d))
    {
    case OEDIT_CONFIRM_SAVESTRING:
        switch (*arg)
        {
        case 'y':
        case 'Y':
            send_to_char("Saving object to memory.\r\n", d->character);
            /* oedit_save_internally(d); */
        case 'n':
        case 'N':
            cleanup_olc( d );
            return;
        default:
            send_to_char("Invalid choice!\r\n", d->character);
            send_to_char("Do you wish to save this object internally?\r\n", d->character);
            return;
        }

    case OEDIT_MAIN_MENU:
        /* switch to whichever mode the user selected, display prompt or menu */
        switch ( UPPER(arg[0]) )
	{
        case 'Q':
            /* send_to_char( "Do you wish to save this object internally?: ", d->character );
             OLC_MODE(d) = OEDIT_CONFIRM_SAVESTRING; */
            cleanup_olc( d );
            return;
        case '1':
            send_to_char("Enter namelist : ", d->character);
            OLC_MODE(d) = OEDIT_EDIT_NAMELIST;
            break;
        case '2':
            send_to_char("Enter short desc : ", d->character);
            OLC_MODE(d) = OEDIT_SHORTDESC;
            break;
        case '3':
            send_to_char("Enter long desc :-\r\n| ", d->character);
            OLC_MODE(d) = OEDIT_LONGDESC;
            break;
        case '4':
            /* lets not */
            send_to_char("Enter action desc :-\r\n", d->character);
            OLC_MODE(d) = OEDIT_ACTDESC;
            break;
        case '5':
            oedit_disp_type_menu(d);
            OLC_MODE(d) = OEDIT_TYPE;
            break;
        case '6':
            oedit_disp_extra_menu(d);
            OLC_MODE(d) = OEDIT_EXTRAS;
            break;
        case '7':
            oedit_disp_wear_menu(d);
            OLC_MODE(d) = OEDIT_WEAR;
            break;
        case '8':
            send_to_char("Enter weight : ", d->character);
            OLC_MODE(d) = OEDIT_WEIGHT;
            break;
        case '9':
            send_to_char("Enter cost : ", d->character);
            OLC_MODE(d) = OEDIT_COST;
            break;
        case 'A':
            send_to_char("Enter cost per day : ", d->character);
            OLC_MODE(d) = OEDIT_COSTPERDAY;
            break;
        case 'B':
            send_to_char("Enter timer : ", d->character);
            OLC_MODE(d) = OEDIT_TIMER;
            break;
        case 'C':
            send_to_char("Enter level : ", d->character);
            OLC_MODE(d) = OEDIT_LEVEL;
            break;
        case 'D':
	    if ( IS_SET( obj->wear_flags, ITEM_WEAR_BODY )
	    ||   IS_SET( obj->wear_flags, ITEM_WEAR_ABOUT )
	    ||   IS_SET( obj->wear_flags, ITEM_WEAR_ARMS )
	    ||   IS_SET( obj->wear_flags, ITEM_WEAR_FEET )
	    ||   IS_SET( obj->wear_flags, ITEM_WEAR_HANDS )
	    ||   IS_SET( obj->wear_flags, ITEM_WEAR_LEGS )
	    ||   IS_SET( obj->wear_flags, ITEM_WEAR_WAIST ) )
	    {
        	oedit_disp_layer_menu(d);
        	OLC_MODE(d) = OEDIT_LAYERS;
	    }
	    else
		send_to_char( "The wear location of this object is not layerable.\n\r", d->character );
            break;
        case 'E':
            oedit_disp_val1_menu(d);
            break;
        case 'F':
            oedit_disp_prompt_apply_menu(d);
            break;
        case 'G':
            oedit_disp_extradesc_menu(d);
            break;
	case 'H':
	    oedit_disp_progs(d);
	    break;
        default:
            oedit_disp_menu(d);
            break;
        }
        return;			/* end of OEDIT_MAIN_MENU */

    case OEDIT_EDIT_NAMELIST:
        STRFREE(obj->name);
        obj->name = STRALLOC(arg);
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE) )
        {
            STRFREE( obj->pIndexData->name );
            obj->pIndexData->name = QUICKLINK( obj->name );
        }
	olc_log( d, "Changed name to %s", obj->name );
        break;

    case OEDIT_SHORTDESC:
        STRFREE(obj->short_descr);
        obj->short_descr = STRALLOC(arg);
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE) )
        {
            STRFREE( obj->pIndexData->short_descr );
            obj->pIndexData->short_descr = QUICKLINK( obj->short_descr );
        }
	olc_log( d, "Changed short to %s", obj->short_descr );
        break;

    case OEDIT_LONGDESC:
        STRFREE(obj->description);
        obj->description = STRALLOC(arg);
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE) )
        {
            STRFREE( obj->pIndexData->description );
            obj->pIndexData->description = QUICKLINK( obj->description );
        }
	olc_log( d, "Changed longdesc to %s", obj->description );
        break;

    case OEDIT_ACTDESC:
        STRFREE( obj->action_desc );
        obj->action_desc = STRALLOC(arg);
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE) )
        {
            STRFREE( obj->pIndexData->action_desc );
            obj->pIndexData->action_desc = QUICKLINK( obj->action_desc );
        }
	olc_log( d, "Changed actiondesc to %s", obj->action_desc );
        break;

    case OEDIT_TYPE:
	if ( is_number( arg ) )
            number = atoi(arg);
	else
	    number = get_otype( arg );

        if ((number < 1) || (number >= MAX_ITEM_TYPE))
        {
            send_to_char("Invalid choice, try again : ", d->character);
            return;
        }
        else
        {
            obj->item_type = (sh_int) number;
            if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
                obj->pIndexData->item_type = obj->item_type;
        }
	olc_log( d, "Changed object type to %s", o_types[number] );
        break;

    case OEDIT_EXTRAS:
	while ( arg[0] != '\0' )
	{
	    arg = one_argument( arg, arg1 );
	    if ( is_number( arg1 ) )
	    {
		number = atoi( arg1 );

		if ( number == 0 )
		{
		    oedit_disp_menu(d);
		    return;
		}

		number -= 1; /* Offset for 0 */
		if ( number < 0 || number > MAX_ITEM_FLAG )
		{
		    oedit_disp_extra_menu(d);
		    return;
		}
	    }
	    else
	    {
		number = get_oflag( arg1 );
		if ( number < 0 || number > MAX_BITS )
		{
		    oedit_disp_extra_menu(d);
		    return;
		}
	    }

	    if ( number == ITEM_PROTOTYPE
	    &&   get_trust( d->character ) < LEVEL_GREATER
	    &&   !is_name( "protoflag", d->character->pcdata->bestowments ) )
		send_to_char( "You cannot change the prototype flag.\n\r", d->character );
	    else
	    {
		xTOGGLE_BIT( obj->extra_flags, number );
		olc_log( d, "%s the flag %s",
		    xIS_SET( obj->extra_flags, number ) ? "Added" : "Removed",
		    o_flags[number] );
	    }

	    /* If you used a number, you can only do one flag at a time */
	    if ( is_number( arg ) )
		break;
	}
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	    obj->pIndexData->extra_flags = obj->extra_flags;
        oedit_disp_extra_menu(d);
        return;

    case OEDIT_WEAR:
	if ( is_number( arg ) )
	{
            number = atoi(arg);
	    if ( number == 0 )
		break;
	    else if ( number < 0 || number > ITEM_WEAR_MAX+1 )
	    {
		send_to_char( "Invalid flag, try again: ", d->character );
		return;
	    }
	    else
	    {
		number -= 1; /* Offset to accomodate 0 */
		TOGGLE_BIT( obj->wear_flags, 1 << number );
		olc_log( d, "%s the wearloc %s",
		    IS_SET( obj->wear_flags, 1 << number ) ? "Added" : "Removed",
		    w_flags[number] );
	    }
	}
	else
	{
	    while ( arg[0] != '\0' )
	    {
		arg = one_argument( arg, arg1 );
		number = get_wflag( arg1 );
		if ( number != -1 )
		{
		    TOGGLE_BIT( obj->wear_flags, 1 << number );
		    olc_log( d, "%s the wearloc %s",
			IS_SET( obj->wear_flags, 1 << number ) ? "Added" : "Removed",
			w_flags[number] );
		}
	    }
	}
	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	    obj->pIndexData->wear_flags = obj->wear_flags;
	oedit_disp_wear_menu(d);
	return;

    case OEDIT_WEIGHT:
        number = atoi(arg);
        obj->weight = number;
	olc_log( d, "Changed weight to %d", obj->weight );
        if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
            obj->pIndexData->weight = obj->weight;
        break;

    case OEDIT_COST:
        number = atoi(arg);
        obj->cost = number;
	olc_log( d, "Changed cost to %d", obj->cost );
        if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
            obj->pIndexData->cost = obj->cost;
        break;

    case OEDIT_COSTPERDAY:
        number = atoi(arg);
        obj->pIndexData->rent = number;
	olc_log( d, "Changed rent to %d", obj->pIndexData->rent );
        break;

    case OEDIT_TIMER:
        number = atoi(arg);
        obj->timer = number;
	olc_log( d, "Changed timer to %d", obj->timer );
        break;

    case OEDIT_LEVEL:
        number = atoi(arg);
        obj->level = URANGE( 0, number, MAX_LEVEL );
	olc_log( d, "Changed object level to %d", obj->level );
        break;

    case OEDIT_LAYERS:
        /* Like they say, easy on the user, hard on the programmer :) */
        /* Or did I just make that up.... */
        number = atoi( arg );
        switch ( number )
        {
        case 0:
            oedit_disp_menu(d);
	    return;
        case 1:
            obj->pIndexData->layers = 0;
            break;
        case 2:
            TOGGLE_BIT( obj->pIndexData->layers, 1 );
            break;
        case 3:
            TOGGLE_BIT( obj->pIndexData->layers, 2 );
            break;
        case 4:
            TOGGLE_BIT( obj->pIndexData->layers, 4 );
            break;
        case 5:
            TOGGLE_BIT( obj->pIndexData->layers, 8 );
            break;
        case 6:
            TOGGLE_BIT( obj->pIndexData->layers, 16 );
            break;
        case 7:
            TOGGLE_BIT( obj->pIndexData->layers, 32 );
            break;
        case 8:
            TOGGLE_BIT( obj->pIndexData->layers, 64 );
            break;
        case 9:
            TOGGLE_BIT( obj->pIndexData->layers, 128 );
            break;
        default:
            send_to_char( "Invalid selection, try again: ", d->character );
            return;
        }
	olc_log( d, "Changed layers to %d", obj->pIndexData->layers );
	oedit_disp_layer_menu(d);
        return;

    case OEDIT_VALUE_1:
        number = atoi(arg);
        switch ( obj->item_type )
        {
        case ITEM_LEVER:
        case ITEM_SWITCH:
            if ( number < 0 || number > 29 )
                oedit_disp_lever_flags_menu(d);
            else
            {
                if ( number != 0 )
                {
                    TOGGLE_BIT( obj->value[0], 1 << (number - 1) );
                    if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE) )
                        TOGGLE_BIT( obj->pIndexData->value[0], 1 << (number - 1) );
                    oedit_disp_val1_menu(d);
                }
                else
                    oedit_disp_val2_menu(d);
            }
            break;

        default:
            obj->value[0] = number;
            if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE) )
                obj->pIndexData->value[0] = number;
            oedit_disp_val2_menu(d);
        }
        olc_log( d, "Changed v0 to %d", obj->value[0] );
        return;

    case OEDIT_VALUE_2:
        number = atoi(arg);
        switch (obj->item_type)
        {
        case ITEM_PILL:
        case ITEM_SCROLL:
        case ITEM_POTION:
            if ( !is_number(arg) )
                number = skill_lookup( arg );
            obj->value[1] = number;
            if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE) )
                obj->pIndexData->value[1] = number;
            oedit_disp_val3_menu(d);
            break;
        case ITEM_LEVER:
        case ITEM_SWITCH:
            if ( IS_SET( obj->value[0], TRIG_CAST ) )
                number = skill_lookup( arg );
            obj->value[1] = number;
            if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE) )
                obj->pIndexData->value[1] = number;
            oedit_disp_val3_menu(d);
            break;

        case ITEM_CONTAINER:
            number = atoi(arg);
            if ( number < 0 || number > 31 )
                oedit_disp_container_flags_menu(d);
            else
            {
                /* if 0, quit */
                if (number != 0)
                { number = 1 << (number - 1);
                    TOGGLE_BIT( obj->value[1], number );
                if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE) )
                    TOGGLE_BIT( obj->pIndexData->value[1], number );
                oedit_disp_val2_menu(d);
                }
                else
                    oedit_disp_val3_menu(d);
            }
            break;
        default:
            obj->value[1] = number;
            if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE) )
                obj->pIndexData->value[1] = number;
            oedit_disp_val3_menu(d);
	    break;
        }
	olc_log( d, "Changed v1 to %d", obj->value[1] );
        return;

    case OEDIT_VALUE_3:
        number = atoi(arg);
        /* Some error checking done here */
        switch ( obj->item_type )
        {
	case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
            min_val = -1;
            max_val = top_sn-1;
            if ( !is_number( arg ) )
                number = skill_lookup( arg );
            break;
        case ITEM_WEAPON:
            min_val = 0;
            max_val = 100;
	    break;
        case ITEM_DRINK_CON:
        case ITEM_FOUNTAIN:
            min_val = 0;
            max_val = LIQ_MAX;
            break;
        default:
            /* Would require modifying if you have bvnum */
            min_val = -32000;
            max_val = 32000;
	    break;
        }
        obj->value[2] = URANGE( min_val, number, max_val );
        olc_log( d, "Changed v2 to %d", obj->value[2] );
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE) )
            obj->pIndexData->value[2] = obj->value[2];
        oedit_disp_val4_menu(d);
        return;

    case OEDIT_VALUE_4:
        number = atoi(arg);
        switch ( obj->item_type )
        {
        case ITEM_PILL:
        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_WAND:
        case ITEM_STAFF:
            min_val = -1;
            max_val = top_sn - 1;
            if ( !is_number( arg ) )
                number = skill_lookup( arg );
            break;
        case ITEM_WEAPON:
            min_val = 0;
            max_val = MAX_ATTACK_TYPE -1;
            if ( number < min_val || number > max_val )
            {
                oedit_disp_val4_menu(d);
                return;
            }
            break;
        default:
            min_val = -32000;
            max_val = 32000;
            break;
        }
        obj->value[3] = URANGE( min_val, number, max_val );
        olc_log( d, "Changed v3 to %d", obj->value[3] );
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE) )
            obj->pIndexData->value[3] = obj->value[3];
        oedit_disp_val5_menu(d);
        break;

    case OEDIT_VALUE_5:
        number = atoi(arg);
        switch( obj->item_type )
        {
        case ITEM_SALVE:
            if ( !is_number( arg ) )
                number = skill_lookup(arg);
            min_val = -1;
            max_val = top_sn-1;
            break;
        case ITEM_FOOD:
            min_val = 0;
            max_val = 32000;
            break;
        default:
            min_val = -32000;
            max_val = 32000;
            break;
        }
        obj->value[4] = URANGE( min_val, number, max_val );
        olc_log( d, "Changed v4 to %d", obj->value[4] );
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE) )
            obj->pIndexData->value[4] = obj->value[4];
        oedit_disp_val6_menu(d);
        break;

    case OEDIT_VALUE_6:
        number = atoi(arg);
        switch( obj->item_type )
        {
        case ITEM_SALVE:
            if ( !is_number( arg ) )
                number = skill_lookup(arg);
            min_val = -1;
            max_val = top_sn-1;
            break;
        default:
            min_val = -32000;
            max_val = 32000;
            break;
        }
        obj->value[5] = URANGE( min_val, number, max_val );
        olc_log( d, "Changed v5 to %d", obj->value[5] );
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE) )
            obj->pIndexData->value[5] = obj->value[5];
        break;

    case OEDIT_AFFECT_MENU:
        number = atoi( arg );

        switch ( arg[0] )
        {
        default: /* if its a number, then its prolly for editing an affect */
            if ( is_number( arg ) )
                edit_object_affect( d, number );
            else
                oedit_disp_prompt_apply_menu(d);
            return;

        case 'r':
        case 'R':
	    /* Chop off the 'R', if theres a number following use it, otherwise
               prompt for input */
            arg = one_argument( arg, arg1 );
	    if ( arg && arg[0] != '\0' )
	    {
		number = atoi( arg );
		remove_affect_from_obj( obj, number );
		oedit_disp_prompt_apply_menu(d);
	    }
	    else
	    {
                send_to_char( "Remove which affect? ", d->character );
                OLC_MODE(d) = OEDIT_AFFECT_REMOVE;
            }
            return;

        case 'a':
        case 'A':
            CREATE( paf, AFFECT_DATA, 1 );
	    d->character->spare_ptr = paf;
            oedit_disp_affect_menu(d);
            return;

        case 'q':
        case 'Q':
	    d->character->spare_ptr = NULL;
            break;
        }
        break; /* If we reach here, we're done */

    case OEDIT_AFFECT_LOCATION:
	if ( is_number( arg ) )
	{
            number = atoi( arg );
	    if ( number == 0 )
	    {
		/* Junk the affect */
		DISPOSE( paf );
		d->character->spare_ptr = NULL;
		oedit_disp_prompt_apply_menu( d );
		return;
	    }
	}
	else
	    number = get_atype( arg );

        if ( number < 0 || number >= MAX_APPLY_TYPE || number == APPLY_EXT_AFFECT )
        {
            send_to_char( "Invalid location, try again: ", d->character );
            return;
        }
	    
        paf->location = number;
	OLC_MODE(d) = OEDIT_AFFECT_MODIFIER;
        /* Insert all special affect handling here ie: non numerical stuff */
        /* And add the apropriate case statement below */
	if ( number == APPLY_AFFECT )
	{
	    d->character->tempnum = 0;
	    medit_disp_aff_flags( d );
	}
	else if ( number == APPLY_RESISTANT || number == APPLY_IMMUNE || number == APPLY_SUSCEPTIBLE )
	{
	    d->character->tempnum = 0;
	    medit_disp_ris( d );
	}
	else if ( number == APPLY_WEAPONSPELL || number == APPLY_WEARSPELL || number == APPLY_REMOVESPELL )
	    oedit_disp_spells_menu( d );
	else
            send_to_char( "\n\rModifier: ", d->character );
        return;

    case OEDIT_AFFECT_MODIFIER:
	switch ( paf->location )
	{
	case APPLY_AFFECT:
	case APPLY_RESISTANT:
	case APPLY_IMMUNE:
	case APPLY_SUSCEPTIBLE:
	    if ( is_number( arg ) )
	    {
		number = atoi( arg );
		if ( number == 0 )
		{
		    value = d->character->tempnum;
		    break;
		}
		TOGGLE_BIT( d->character->tempnum, 1 << (number - 1) );
	    }
	    else
	    {
		while ( arg[0] != '\0' )
		{
		    arg = one_argument( arg, arg1 );
		    if ( paf->location == APPLY_AFFECT )
			number = get_aflag( arg1 );
		    else
			number = get_risflag( arg1 );
		    if ( number < 0 )
			ch_printf( d->character, "Invalid flag: %s\n\r", arg1 );
		    else
			TOGGLE_BIT( d->character->tempnum, 1 << number );
		}
	    }
	    if ( paf->location == APPLY_AFFECT )
		medit_disp_aff_flags(d);
	    else
		medit_disp_ris(d);
	    return;

	case APPLY_WEAPONSPELL:
	case APPLY_WEARSPELL:
	case APPLY_REMOVESPELL:
	    if ( is_number( arg ) )
	    {
		number = atoi( arg );
		if ( IS_VALID_SN( number ) )
		    value = number;
		else
		{
		    send_to_char( "Invalid sn, try again: ", d->character );
		    return;
		}
	    }
	    else
	    {
		value = bsearch_skill_exact( arg, gsn_first_spell, gsn_first_skill - 1 );
		if ( value < 0 )
		{
		    ch_printf( d->character, "Invalid spell %s, try again: ", arg );
		    return;
		}
	    }
	    break;
	default:
	    value = atoi( arg );
	    break;
	}
        /* Link it in */
	if ( !value || OLC_VAL(d) == TRUE )
	{
            paf->modifier = value;
            olc_log( d, "Modified affect to: %s by %d", a_types[paf->location], value );
	    OLC_VAL(d) = FALSE;
	    oedit_disp_prompt_apply_menu(d);
	    return;
	}
	CREATE( npaf, AFFECT_DATA, 1 );
        npaf->type = -1;
        npaf->duration = -1;
        npaf->location = URANGE( 0, paf->location, MAX_APPLY_TYPE );
	npaf->modifier = value;
	xCLEAR_BITS( npaf->bitvector );
        npaf->next = NULL;

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            LINK( npaf, obj->pIndexData->first_affect, obj->pIndexData->last_affect, next, prev );
        else
            LINK( npaf, obj->first_affect, obj->last_affect, next, prev );
        ++top_affect;
        olc_log( d, "Added new affect: %s by %d", a_types[npaf->location], npaf->modifier );

	DISPOSE( paf );
	d->character->spare_ptr = NULL;
        oedit_disp_prompt_apply_menu(d);
        return;

    case OEDIT_AFFECT_RIS:
	/* Unnecessary atm */
        number = atoi( arg );
        if ( number < 0 || number > 32 )
        {
            send_to_char( "Unknown flag, try again: ", d->character );
            return;
        }
        return;

    case OEDIT_AFFECT_REMOVE:
        number = atoi( arg );
        remove_affect_from_obj( obj, number );
        olc_log( d, "Removed affect #%d", number );
        oedit_disp_prompt_apply_menu(d);
        return;

    case OEDIT_EXTRADESC_KEY:
	/* if ( SetOExtra( obj, arg ) || SetOExtraProto( obj->pIndexData, arg ) )
	{
	    send_to_char( "A extradesc with that keyword already exists.\n\r", d->character );
	    oedit_disp_extradesc_menu(d);
	    return;
	} */
        olc_log( d, "Changed exdesc %s to %s", ed->keyword, arg );
        STRFREE( ed->keyword );
        ed->keyword = STRALLOC( arg );
        oedit_disp_extra_choice(d);
        return;

    case OEDIT_EXTRADESC_DESCRIPTION:
        /* Should never reach this */
        break;

    case OEDIT_EXTRADESC_CHOICE:
        number = atoi( arg );
        switch ( number )
        {
        case 0:
            OLC_MODE(d) = OEDIT_EXTRADESC_MENU;
            oedit_disp_extradesc_menu(d);
            return;
        case 1:
            OLC_MODE(d) = OEDIT_EXTRADESC_KEY;
            send_to_char( "Enter keywords, speperated by spaces: ", d->character );
            return;
        case 2:
            OLC_MODE(d) = OEDIT_EXTRADESC_DESCRIPTION;
            d->character->substate = SUB_OBJ_EXTRA;
	    d->character->last_cmd = do_oedit_reset;

	    send_to_char( "Enter new extra description - :\n\r", d->character );
	    if ( !ed->description )
		ed->description = STRALLOC( "" );
	    start_editing( d->character, ed->description );
	    return;
	}
	break; 

    case OEDIT_EXTRADESC_DELETE:
        ed = oedit_find_extradesc( obj, atoi(arg) );
        if ( !ed )
        {
            send_to_char( "Extra description not found, try again: ", d->character );
            return;
        }
        olc_log( d, "Deleted exdesc %s", ed->keyword );
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            UNLINK( ed, obj->pIndexData->first_extradesc, obj->pIndexData->last_extradesc, next, prev );
        else
            UNLINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
        STRFREE( ed->keyword );
        STRFREE( ed->description );
        DISPOSE( ed );
        top_ed--;
        oedit_disp_extradesc_menu(d);
        return;

    case OEDIT_EXTRADESC_MENU:
        switch ( UPPER( arg[0] ) )
        {
        case 'Q':
            break;

        case 'A':
	    CREATE( ed, EXTRA_DESCR_DATA, 1 );
	    if ( IS_OBJ_STAT(obj, ITEM_PROTOTYPE) )
		LINK( ed, obj->pIndexData->first_extradesc, obj->pIndexData->last_extradesc, next, prev );
	    else
		LINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
	    ed->keyword		= STRALLOC( "" );
	    ed->description	= STRALLOC( "" );
	    top_ed++;
            d->character->spare_ptr = ed;
            olc_log( d, "Added new exdesc" );
	    
            oedit_disp_extra_choice(d);
            return;

        case 'R':
	    OLC_MODE(d) = OEDIT_EXTRADESC_DELETE;
	    send_to_char( "Delete which extra description? ", d->character );
            return;

        default:
            if ( is_number(arg) )
            {
		ed = oedit_find_extradesc( obj, atoi(arg) );
		if ( !ed )
		{
		    send_to_char( "Not found, try again: ", d->character );
		    return;
		}
		d->character->spare_ptr = ed;
		oedit_disp_extra_choice(d);
            }
            else
                oedit_disp_extradesc_menu(d);
            return;
        }
        break;
    case OEDIT_MPROGS:
	switch ( UPPER( arg[0] ) )
	{
	case 'A':
	    if ( obj->pIndexData->mudprogs != NULL )
		for ( ; mprog->next; mprog = mprog->next );
	    CREATE( mprg, MPROG_DATA, 1 );
	    if ( obj->pIndexData->mudprogs )
		mprog->next		= mprg;
	    else
		obj->pIndexData->mudprogs		= mprg;
	    mprg->next = NULL;

	    d->character->spare_ptr = mprg;
	    oedit_disp_prog_choice(d);
	    return;
	case 'R':
	    send_to_char( "Delete which prog? ", d->character );
	    OLC_MODE(d) = OEDIT_MPROGS_DELETE;
	    return;
	case 'Q':
	    break;
	default:
	    if ( is_number(arg) )
	    {
		mprg = oedit_find_prog( obj, atoi(arg) );
		if ( !mprg )
		{
		    send_to_char( "Not found, try again: ", d->character );
		    return;
		}
		d->character->spare_ptr = mprg;
		oedit_disp_prog_choice(d);
	    }
	    else
		oedit_disp_progs(d);
	    return;
	}
	break;

    case OEDIT_MPROGS_CHOICE:
	switch ( UPPER( arg[0] ) )
	{
	case 'A':
	    OLC_MODE(d) = OEDIT_MPROGS_TYPE;
	    oedit_disp_prog_types(d);
	    return;
	case 'B':
	    OLC_MODE(d) = OEDIT_MPROGS_ARG;
	    send_to_char( "What are the arguments to the program? ", d->character );
	    return;
	case 'C':
	    if ( mprg->type < 1 )
	    {
		send_to_char( "The program must have a type before you can edit it.", d->character );
		return;
	    }
	    d->character->substate = SUB_MPROG_EDIT;
	    d->character->last_cmd = do_oedit_reset;
	    d->character->tempnum  = CON_OEDIT;
	    if ( !mprg->comlist )
		mprg->comlist = STRALLOC( "" );
	    start_editing( d->character, mprg->comlist );
	    return;

	case 'Q':
	    oedit_disp_progs(d);
	    return;
	}
	break;

    case OEDIT_MPROGS_TYPE:
	d->character->tempnum = mprg->type;
	if ( !is_number( arg ) )
	{
	    number = get_mpflag( arg );
	    if ( number < 0 )
	    {
		send_to_char( "Invalid program type, try again: ", d->character );
		return;
	    }
	}
	else
	{
	    number = atoi( arg );
	    if ( number < 0 || number > USE_PROG )
	    {
		send_to_char( "Invalid program type, try again: ", d->character );
		return;
	    }
	}
	mprg->type = number;

	olc_log( d, "Changed prog %s to %s",
	    d->character->tempnum > 0 ? mprog_type_to_name(d->character->tempnum) : "(none)", mprog_type_to_name(number) );

	oedit_disp_prog_choice(d);
	return;

    case OEDIT_MPROGS_DELETE:
	mprg = oedit_find_prog( obj, atoi(arg) );
	if ( !mprg )
	{
	    send_to_char( "Not found, try again: ", d->character );
	    return;
	}

	STRFREE( mprg->arglist );
	STRFREE( mprg->comlist );
	DISPOSE( mprg );
	mprg = mprg->next;
	
	oedit_disp_progs(d);
	return;

    default:
        bug( "Oedit_parse: Reached default case!", 0 );
        break;
    }

    /*. If we get here, we have changed something .*/
    OLC_CHANGE(d) = TRUE; /*. Has changed flag .*/
    oedit_disp_menu(d);
}
