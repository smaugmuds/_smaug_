/*
                     R E A L M S    O F    D E S P A I R  !
   ___________________________________________________________________________
  //            /                                                            \\
 [|_____________\   ********   *        *   ********   *        *   *******   |]
 [|   \\._.//   /  **********  **      **  **********  **      **  *********  |]
 [|   (0...0)   \  **********  ***    ***  **********  ***    ***  *********  |]
 [|    ).:.(    /  ***         ****  ****  ***    ***  ***    ***  ***        |]
 [|    {o o}    \  *********   **********  **********  ***    ***  *** ****   |]
 [|   / ' ' \   /   *********  *** ** ***  **********  ***    ***  ***  ****  |]
 [|-'- /   \ -`-\         ***  ***    ***  ***    ***  ***    ***  ***   ***  |]
 [|   .VxvxV.   /   *********  ***    ***  ***    ***  **********  *********  |]
 [|_____________\  **********  **      **  **      **  **********  *********  |]
 [|             /  *********   *        *  *        *   ********    *******   |]
  \\____________\____________________________________________________________//
     |                                                                     |
     |    --{ [S]imulated [M]edieval [A]dventure Multi[U]ser [G]ame }--    |
     |_____________________________________________________________________|
     |                                                                     |
     |                -*- Mobile/Player editing Module -*-                 |
     |_____________________________________________________________________|
     |                                                                     |
     |     OasisOLC II for Smaug 1.40 written by Evan Cortens(Tagith)      |
     |                                                                     |
     |   Based on OasisOLC for CircleMUD3.0bpl9 written by Harvey Gilpin   |
     |_____________________________________________________________________|
    //                                                                     \\
   [|  SMAUG 2.0 © 2014-2015 Antonio Cao (@burzumishi)                      |]
   [|                                                                       |]
   [|  AFKMud Copyright 1997-2007 by Roger Libiez (Samson),                 |]
   [|  Levi Beckerson (Whir), Michael Ward (Tarl), Erik Wolfe (Dwip),       |]
   [|  Cameron Carroll (Cam), Cyberfox, Karangi, Rathian, Raine,            |]
   [|  Xorith, and Adjani.                                                  |]
   [|  All Rights Reserved. External contributions from Remcon, Quixadhal,  |]
   [|  Zarius and many others.                                              |]
   [|                                                                       |]
   [|  SMAUG 1.4 © 1994-1998 Thoric/Altrag/Blodkai/Narn/Haus/Scryn/Rennard  |]
   [|  Swordbearer/Gorog/Grishnakh/Nivek/Tricops/Fireblade/Edmond/Conran    |]
   [|                                                                       |]
   [|  Merc 2.1 Diku Mud improvments © 1992-1993 Michael Chastain, Michael  |]
   [|  Quan, and Mitchell Tse. Original Diku Mud © 1990-1991 by Sebastian   |]
   [|  Hammer, Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, Katja    |]
   [|  Nyboe. Win32 port Nick Gammon.                                       |]
    \\_____________________________________________________________________//
*/

#include <stdio.h>
#include <string.h>
#include "mud.h"
#include "olc.h"

/*-------------------------------------------------------------------*/
/* Externals */
extern int	top_mob_index;
MOB_INDEX_DATA *   mob_index_hash          [MAX_KEY_HASH];

/* Global Variables */
extern char * const act_flags [];
char 		*olc_clan_list[50];
char 		*olc_deity_list[50];
char 		*olc_council_list[50];
int		olc_top_clan;
int		olc_top_guild;
int		olc_top_order;
int             olc_top_council;
int             olc_top_deity;

char *	const	position_names [] =
{ 
"dead", "mortallywounded", "incapacitated", "stunned", "sleeping",
"berserk", "resting", "aggressive", "sitting", "fighting", "defensive",
"evasive", "standing", "mounted", "shoved", "dragged"
};

/*-------------------------------------------------------------------*/
/*. Function prototypes .*/

int get_actflag			args( ( char *flag ) );
int get_risflag			args( ( char *flag ) );
int get_partflag		args( ( char *flag ) );
int get_attackflag		args( ( char *flag ) );
int get_defenseflag		args( ( char *flag ) );

DECLARE_DO_FUN( do_medit_reset );
void medit_parse		args( ( DESCRIPTOR_DATA *d, char *arg ) );
void medit_disp_menu		args( ( DESCRIPTOR_DATA *d ) );
void medit_disp_npc_menu	args( ( DESCRIPTOR_DATA *d ) );
void medit_disp_pc_menu		args( ( DESCRIPTOR_DATA *d ) );
void medit_disp_positions	args( ( DESCRIPTOR_DATA *d ) );
void medit_disp_mob_flags	args( ( DESCRIPTOR_DATA *d ) );
void medit_disp_plr_flags	args( ( DESCRIPTOR_DATA *d ) );
void medit_disp_pcdata_flags	args( ( DESCRIPTOR_DATA *d ) );
void medit_disp_aff_flags	args( ( DESCRIPTOR_DATA *d ) );
void medit_disp_attack_menu 	args( ( DESCRIPTOR_DATA *d ) );
void medit_disp_defense_menu	args( ( DESCRIPTOR_DATA *d ) );
void medit_disp_ris		args( ( DESCRIPTOR_DATA *d ) );
void medit_disp_spec		args( ( DESCRIPTOR_DATA *d ) );
void medit_disp_clans		args( ( DESCRIPTOR_DATA *d ) );
void medit_disp_deities		args( ( DESCRIPTOR_DATA *d ) );
void medit_disp_councils	args( ( DESCRIPTOR_DATA *d ) );
void medit_disp_parts		args( ( DESCRIPTOR_DATA *d ) );
void medit_disp_classes		args( ( DESCRIPTOR_DATA *d ) );
void medit_disp_races		args( ( DESCRIPTOR_DATA *d ) );
void medit_disp_saving_menu	args( ( DESCRIPTOR_DATA *d ) );
void medit_setup_arrays         args( ( void ) );

/*-------------------------------------------------------------------*\
  initialization functions
\*-------------------------------------------------------------------*/
void do_omedit( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

#ifdef ENABLE_OLC2_EXTRAS
    AREA_DATA              *pArea;
    int                     cvnum,
                            vnum;
    MOB_INDEX_DATA         *pMobIndex;
    char                    buf[MAX_STRING_LENGTH];
    CHAR_DATA              *mob;
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
        vnum = pArea->low_m_vnum;

        if ( !pArea )
        {
            bug( "mob: !pArea", 0 );
            return;
        }

        while ( vnum <= pArea->hi_m_vnum && get_mob_index( vnum ) != NULL )
            vnum++;
        if ( vnum > pArea->hi_m_vnum )
        {
            send_to_char
                ( "&GYou cannot create any more mobs as you have used all that your area is alloted.\r\n",
                  ch );
            return;
        }
        argument = STRALLOC( "mob" );
        pMobIndex = make_mobile( vnum, cvnum, argument );
        pArea = ch->pcdata->area;

        if ( !pMobIndex )
        {
            send_to_char( "Error.\n\r", ch );
            log_string( "do_mcreate: make_mobile failed." );
            return;
        }
        mob = create_mobile( pMobIndex );

        char_to_room( mob, ch->in_room );
        mudstrlcpy( buf, "mob", MSL );
        STRFREE( mob->name );
        mob->name = STRALLOC( buf );
        victim = mob;
    }
    else
    {
#endif

    if ( (victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	return;
    }

#ifdef ENABLE_OLC2_EXTRAS
	}
#endif

    /* Temporary, since this isnt done
    if ( !IS_NPC( victim ) && str_cmp( ch->name, "Tagith" ) )
    {
        send_to_char( "PC editing isnt complete yet, sorry.\n\r", ch );
        return;
    } */
    if ( !IS_NPC( victim ) && get_trust( ch ) < sysdata.level_modify_proto )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    /* Make sure the object isnt already being edited */
    for ( d = first_descriptor; d; d = d->next )
	if ( d->connected == CON_MEDIT )
	    if ( d->olc && OLC_VNUM(d) == victim->pIndexData->vnum )
	    {
                ch_printf( ch, "That %s is currently being edited by %s.\n\r",
                           IS_NPC(victim) ? "mobile" : "character", d->character->name );
		return;
	    }

    if ( !can_mmodify( ch, victim ) )
	return;

    d = ch->desc;
    CREATE( d->olc, OLC_DATA, 1 );
    if ( IS_NPC( victim ) )
    {
        OLC_VNUM(d) = victim->pIndexData->vnum;
        /* medit_setup( d, OLC_VNUM(d) ); */
    }
    else
	medit_setup_arrays();
   
    d->character->dest_buf = victim;
    d->connected = CON_MEDIT;
    OLC_CHANGE(d) = FALSE;
    medit_disp_menu( d );

    act( AT_ACTION, "$n starts using OLC.", ch, NULL, NULL, TO_ROOM );
    return;
}
  
void do_mcopy( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int ovnum, cvnum;
    MOB_INDEX_DATA *orig;
    MOB_INDEX_DATA *copy;
    MPROG_DATA *mprog, *cprog;
    int iHash;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( !arg1 || !arg2 )
    {
	send_to_char( "Usage: mcopy <original> <new>\n\r", ch );
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

    if ( get_mob_index( cvnum ) )
    {
        send_to_char( "That object already exists.\n\r", ch );
        return;
    }

    if ( (orig = get_mob_index( ovnum ) ) == NULL )
    {
	send_to_char( "How can you copy something that doesnt exist?\n\r", ch );
	return;
    }

    CREATE( copy, MOB_INDEX_DATA, 1 );
    copy->vnum                  = cvnum;
    copy->player_name           = QUICKLINK( orig->player_name );
    copy->short_descr	        = QUICKLINK( orig->short_descr );
    copy->long_descr	        = QUICKLINK( orig->long_descr  );
    copy->description	        = QUICKLINK( orig->description );
    copy->act		        = orig->act;
    copy->affected_by	        = orig->affected_by;
    copy->pShop		        = NULL;
    copy->rShop		        = NULL;
    copy->spec_fun		= orig->spec_fun;
    copy->mudprogs		= NULL;
    xCLEAR_BITS(copy->progtypes);
    copy->alignment	        = orig->alignment;
    copy->level		        = orig->level;
    copy->mobthac0		= orig->mobthac0;
    copy->ac		        = orig->ac;
    copy->hitnodice	        = orig->hitnodice;
    copy->hitsizedice	        = orig->hitsizedice;
    copy->hitplus		= orig->hitplus;
    copy->damnodice	        = orig->damnodice;
    copy->damsizedice	        = orig->damsizedice;
    copy->damplus		= orig->damplus;
    copy->gold		        = orig->gold;
#ifdef ENABLE_GOLD_SILVER_COPPER
    copy->silver		      = orig->silver;
    copy->copper		      = orig->copper;
#endif
    copy->exp		        = orig->exp;
    copy->position		= orig->position;
    copy->defposition	        = orig->defposition;
    copy->sex		        = orig->sex;
    copy->perm_str		= orig->perm_str;
    copy->perm_dex		= orig->perm_dex;
    copy->perm_int		= orig->perm_int;
    copy->perm_wis		= orig->perm_wis;
    copy->perm_cha		= orig->perm_cha;
    copy->perm_con		= orig->perm_con;
    copy->perm_lck		= orig->perm_lck;
    copy->race		        = orig->race;
    copy->class		        = orig->class;
    copy->xflags	        = orig->xflags;
    copy->resistant	        = orig->resistant;
    copy->immune	        = orig->immune;
    copy->susceptible	        = orig->susceptible;
    copy->numattacks	        = orig->numattacks;
    copy->attacks	        = orig->attacks;
    copy->defenses	        = orig->defenses;
    copy->height		= orig->height;
    copy->weight		= orig->weight;
    copy->saving_poison_death	= orig->saving_poison_death;
    copy->saving_wand         	= orig->saving_wand;
    copy->saving_para_petri	= orig->saving_para_petri;
    copy->saving_breath  	= orig->saving_breath;
    copy->saving_spell_staff 	= orig->saving_spell_staff;

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
    copy->next			= mob_index_hash[iHash];
    mob_index_hash[iHash]	= copy;
    top_mob_index++;

    set_char_color( AT_PLAIN, ch );
    send_to_char( "Mobile copied.\n\r", ch );
    
    return;
}

/*
 * Quite the hack here :P
 */
void medit_setup_arrays( void )
{
    CLAN_DATA *clan;
    DEITY_DATA *deity;
    COUNCIL_DATA *council;
    int count;

    count = 0;
    for ( clan = first_clan; clan; clan = clan->next )
    {
	if ( clan->clan_type != CLAN_ORDER && clan->clan_type != CLAN_GUILD )
	{
	    olc_clan_list[count] = clan->name;
	    count++;
	}
    }
    olc_top_clan = count-1;
   
    for ( clan = first_clan; clan; clan = clan->next )
    {
	if ( clan->clan_type == CLAN_GUILD )
	{
	    olc_clan_list[count] = clan->name;
	    count++;
	}
    }
    olc_top_guild = count-1;
   
    for ( clan = first_clan; clan; clan = clan->next )
    {
	if ( clan->clan_type == CLAN_ORDER )
	{
	    olc_clan_list[count] = clan->name;
	    count++;
	}
    }
    olc_top_order = count-1;

    count = 0;
    for ( deity = first_deity; deity; deity = deity->next )
    {
	olc_deity_list[count] = QUICKLINK( deity->name );
	count++;
    }
    olc_top_deity = count;

    count = 0;
    for ( council = first_council; council; council = council->next )
    {
	olc_council_list[count] = council->name;
	count++;
    }
    olc_top_council = count;
}

/**************************************************************************
 Menu Displaying Functions
 **************************************************************************/

/*
 * Display poistions (sitting, standing etc), same for pos and defpos
 */
void medit_disp_positions( DESCRIPTOR_DATA *d )
{ 
    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    send_to_char_color( " &g0&w) Dead\n\r", d->character );
    send_to_char_color( " &g1&w) Mortally Wounded\n\r", d->character );
    send_to_char_color( " &g2&w) Incapacitated\n\r", d->character );
    send_to_char_color( " &g3&w) Stunned\n\r", d->character );
    send_to_char_color( " &g4&w) Sleeping\n\r", d->character );
    send_to_char_color( " &g5&w) Berserk\n\r", d->character );
    send_to_char_color( " &g6&w) Resting\n\r", d->character );
    send_to_char_color( " &g7&w) Aggressive\n\r", d->character );
    send_to_char_color( " &g8&w) Sitting\n\r", d->character );
    send_to_char_color( " &g9&w) Fighting\n\r", d->character );
    send_to_char_color( "&g10&w) Defensive\n\r", d->character );
    send_to_char_color( "&g11&w) Evasive\n\r", d->character );
    send_to_char_color( "&g12&w) Standing\n\r", d->character );
    /* for (i = 0; i < POS_MOUNTED; i++)
    {
	sprintf(buf, "&g%2d&w) %s\r\n", i, capitalize(position_names[i]));
	send_to_char_color(buf, d->character);
    } */
    send_to_char("Enter position number : ", d->character);
}

/*
 * Display mobile sexes, this is hard coded cause it just works that way :)
 */
void medit_disp_sex( DESCRIPTOR_DATA *d )
{
    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

    send_to_char_color( " &g0&w) Neutral\n\r", d->character );
    send_to_char_color( " &g1&w) Male\n\r", d->character );
    send_to_char_color( " &g2&w) Female\n\r", d->character );
    send_to_char("\n\rEnter gender number : ", d->character);
}

void medit_disp_spec( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *ch = d->character;
  
    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 ); 
    ch_printf_color( ch, " &g 0&w) No Special\n\r" );
    ch_printf_color( ch, " &g 1&w) Spec_breath_any\n\r" );
    ch_printf_color( ch, " &g 2&w) Spec_breath_acid\n\r" );
    ch_printf_color( ch, " &g 3&w) Spec_breath_fire\n\r" );
    ch_printf_color( ch, " &g 4&w) Spec_breath_frost\n\r" );
    ch_printf_color( ch, " &g 5&w) Spec_breath_gas\n\r" );
    ch_printf_color( ch, " &g 6&w) Spec_breath_lightning\n\r" );
    ch_printf_color( ch, " &g 7&w) Spec_cast_adept\n\r" );
    ch_printf_color( ch, " &g 8&w) Spec_cast_cleric\n\r" );
    ch_printf_color( ch, " &g 9&w) Spec_cast_mage\n\r" );
    ch_printf_color( ch, " &g10&w) Spec_cast_undead\n\r" );
    ch_printf_color( ch, " &g11&w) Spec_executioner\n\r" );
    ch_printf_color( ch, " &g12&w) Spec_fido\n\r" );
    ch_printf_color( ch, " &g13&w) Spec_guard\n\r" );
    ch_printf_color( ch, " &g14&w) Spec_janitor\n\r" );
    ch_printf_color( ch, " &g15&w) Spec_poison\n\r" );
    ch_printf_color( ch, " &g16&w) Spec_thief\n\r" );

    send_to_char( "Enter number of special: ", ch );
}

/*
 * Used for both mob affected_by and object affect bitvectors
 */
void medit_disp_ris( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *victim = d->character->dest_buf;
    int counter;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );

    for ( counter = 0; counter < 21; counter++ )
    {
	ch_printf_color( d->character, "&g%2d&w) %-20.20s\n\r",
	    counter+1, ris_flags[counter] );
    }
    switch( OLC_MODE(d) )
    {
    case OEDIT_AFFECT_MODIFIER:
	ch_printf_color( d->character, "\n\rCurrent flags: &c%s&w\n\r", flag_string( d->character->tempnum, ris_flags ) );
	break;
    case MEDIT_RESISTANT:
	ch_printf_color( d->character, "\n\rCurrent flags: &c%s&w\n\r", flag_string( victim->resistant, ris_flags ) );
	break;
    case MEDIT_IMMUNE:
	ch_printf_color( d->character, "\n\rCurrent flags: &c%s&w\n\r", flag_string( victim->immune, ris_flags ) );
	break;
    case MEDIT_SUSCEPTIBLE:
	ch_printf_color( d->character, "\n\rCurrent flags: &c%s&w\n\r", flag_string( victim->susceptible, ris_flags ) );
	break;
    }
    send_to_char( "Enter flag (0 to quit): ", d->character );
}

/*
 * Mobile attacks
 */
void medit_disp_attack_menu( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *victim = d->character->dest_buf;
    char buf[MAX_STRING_LENGTH];
    int i;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for ( i = 0; i < MAX_ATTACK_TYPE; i++ )
    {
	sprintf( buf, "&g%2d&w) %-20.20s\n\r", i+1, attack_flags[i] );
	send_to_char_color( buf, d->character );
    }
    sprintf( buf, "Current flags: &c%s&w\n\rEnter attack flag (0 to exit): ",
             ext_flag_string( &victim->attacks, attack_flags ) );
    send_to_char_color( buf, d->character );
}

/*
 * Display menu of NPC defense flags
 */
void medit_disp_defense_menu( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *victim = d->character->dest_buf;
    char buf[MAX_STRING_LENGTH];
    int i;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for ( i = 0; i < MAX_DEFENSE_TYPE; i++ )
    {
	sprintf( buf, "&g%2d&w) %-20.20s\n\r", i+1, defense_flags[i] );
	send_to_char_color( buf, d->character );
    }
    sprintf( buf, "Current flags: &c%s&w\n\rEnter defense flag (0 to exit): ",
             ext_flag_string( &victim->defenses, defense_flags ) );
    send_to_char_color( buf, d->character );
}

/*-------------------------------------------------------------------*/
/*. Display mob-flags menu .*/

void medit_disp_mob_flags( DESCRIPTOR_DATA *d )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim = d->character->dest_buf;
    int i, columns = 0;
  
    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for (i = 0; i < MAX_ACT_FLAGS; i++)
    {
	sprintf(buf, "&g%2d&w) %-20.20s  ",
	    i+1, act_flags[i]
	);
  	if(!(++columns % 2))
  	    strcat(buf, "\n\r");
  	send_to_char_color( buf, d->character );
    }
    sprintf(buf, "\r\n"
	"Current flags : &c%s&w\r\n"
	"Enter mob flags (0 to quit) : ",
	ext_flag_string( &victim->act, act_flags )
    );
    send_to_char_color( buf, d->character );
}

/*
 * Special handing for PC only flags
 */
void medit_disp_plr_flags( DESCRIPTOR_DATA *d )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim = d->character->dest_buf;
    int i, columns = 0;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for ( i = 0; i < PLR_INVISPROMPT; i++ )
    {
	ch_printf_color( d->character, "&g%2d&w) %-20.20s   ", i+1, plr_flags[i] );
	if ( ++columns % 2 == 0 )
	    send_to_char( "\n\r", d->character );
    }
    sprintf( buf, "\n\rCurrent flags: &c%s&w\n\rEnter flags (0 to quit): ",
	ext_flag_string( &victim->act, plr_flags ) );
    send_to_char_color( buf, d->character );
}

void medit_disp_pcdata_flags( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *victim = d->character->dest_buf;
    int i;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for ( i = 0; i < 15; i++ )
    {
	ch_printf_color( d->character, "&g%2d&w) %-20.20s   \n\r", i+1, pc_flags[i] );
    }
    ch_printf_color( d->character, "\n\rCurrent flags: &c%s&w\n\rEnter flags (0 to quit): ",
	flag_string( victim->pcdata->flags, pc_flags ) );
}

/*-------------------------------------------------------------------*/
/*. Display aff-flags menu .*/

void medit_disp_aff_flags( DESCRIPTOR_DATA *d )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim = d->character->dest_buf;
    int i, columns = 0;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );  
    for (i = 0; i < MAX_AFFECTED_BY; i++)
    {
	sprintf(buf, "&g%2d&w) %-20.20s  ", i+1, a_flags[i] );
	if( !(++columns % 2) )
	    strcat(buf, "\r\n");
	send_to_char_color( buf, d->character );
    }
    /* sprintf(buf, "\r\n"
	"Current flags   : &c%s&w\r\n"
	"Enter affected flags (0 to quit) : ",
	affect_bit_name( &victim->affected_by )
    ); */
    if ( OLC_MODE(d) == OEDIT_AFFECT_MODIFIER )
    {
	buf[0] = '\0';

	for ( i = 0; i < 32; i++ )
	    if ( IS_SET( d->character->tempnum, 1 << i ) )
	    {
		strcat( buf, " " );
		strcat( buf, a_flags[i] );
	    }
	ch_printf_color( d->character, "\n\rCurrent flags   : &c%s&w\n\r", buf );
    }
    else
	ch_printf_color( d->character, "\n\rCurrent flags   : &c%s&w\n\r", affect_bit_name( &victim->affected_by ) );
    send_to_char_color( "Enter affected flags (0 to quit) : ", d->character );
}

void medit_disp_clans( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *ch = d->character;
    int count;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    /* ch_printf( ch, "%d %d %d\n\r", olc_top_clan, olc_top_guild, olc_top_order ); */
    send_to_char( "Clans:\n\r", d->character );
    for ( count = 0; count <= olc_top_clan; count++ )
    {
	ch_printf_color( ch, "&g%2d&w) %-20.20s\n\r", count+1, olc_clan_list[count] );
    }

    send_to_char( "\n\rGuilds:\n\r", d->character );	
    for ( count = olc_top_clan+1; count <= olc_top_guild; count++ )
    {
	ch_printf_color( ch, "&g%2d&w) %-20.20s\n\r", count+1, olc_clan_list[count] );
    }

    send_to_char( "\n\rOrders:\n\r", d->character );
    for ( count = olc_top_guild+1; count <= olc_top_order; count++ )
    {
	ch_printf_color( ch, "&g%2d&w) %-20.20s\n\r", count+1, olc_clan_list[count] );
    }
    send_to_char( "Enter choice (0 for none): ", d->character );
}

void medit_disp_deities( DESCRIPTOR_DATA *d )
{
    int count;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    ch_printf( d->character, "%d\n\r", olc_top_deity );
    ch_printf_color( d->character, "&g%2d&w) %-20.20s\n\r", 0, "None" );
    for ( count = 0; count < olc_top_deity; count++ )
    {
	ch_printf_color( d->character, "&g%2d&w) %-20.20s\n\r", count+1, olc_deity_list[count] );
    }
    send_to_char( "Enter choice: ", d->character );
}

void medit_disp_councils( DESCRIPTOR_DATA *d )
{
    int count;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    ch_printf( d->character, "%d\n\r", olc_top_council );
    ch_printf_color( d->character, "&g%2d&w) %-20.20s\n\r", 0, "None" );
    for ( count = 0; count < olc_top_council; count++ )
    {
	ch_printf_color( d->character, "&g%2d&w) %-20.20s\n\r", count+1, olc_council_list[count] );
    }
    send_to_char( "Enter choice: ", d->character );
}

void medit_disp_parts( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *victim = d->character->dest_buf;
    int count, columns = 0;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    for ( count = 0; count < 29; count++ )
    {
	ch_printf_color( d->character, "&g%2d&w) %-20.20s    ", count+1, part_flags[count] );

	if ( ++columns % 2 == 0 )
	    send_to_char( "\n\r", d->character );
    }
    ch_printf_color( d->character, "\n\rCurrent flags: %s\n\rEnter flag or 0 to exit: ",
	flag_string( victim->xflags, part_flags ) ); 
}

void medit_disp_classes( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *victim = d->character->dest_buf;
    int iClass, columns = 0;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    if ( IS_NPC(victim) )
    {
	for ( iClass = 0; iClass < MAX_NPC_CLASS; iClass++ )
	{
	    ch_printf_color( d->character, "&g%2d&w) %-20.20s     ", iClass, npc_class[iClass] );
	    if ( ++columns % 2 == 0 )
		send_to_char( "\n\r", d->character );
	}
    }
    else
    {
	for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	{
	    ch_printf_color( d->character, "&g%2d&w) %-20.20s     \n\r", iClass, class_table[iClass]->who_name );
	    /* if ( ++columns % 2 == 0 )
		send_to_char( "\n\r", d->character ); */
	}
    }
    send_to_char( "\n\rEnter class: ", d->character );
}

void medit_disp_races( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *victim = d->character->dest_buf;
    int iRace, columns = 0;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    if ( IS_NPC(victim) )
    {
	for ( iRace = 0; iRace < MAX_NPC_RACE; iRace++ )
	{
	    ch_printf_color( d->character, "&g%2d&w) %-20.20s  ", iRace, npc_race[iRace] );
	    if ( ++columns % 3 == 0 )
		send_to_char( "\n\r", d->character );
	}
    }
    else
    {
	for ( iRace = 0; iRace < MAX_RACE; iRace++ )
	{
	    ch_printf_color( d->character, "&g%2d&w) %-20.20s     \n\r", iRace, race_table[iRace]->race_name );
	    /* if ( ++columns % 2 == 0 )
		send_to_char( "\n\r", d->character ); */
	}
    }
    send_to_char( "\n\rEnter race: ", d->character );
}

void medit_disp_saving_menu( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *victim = d->character->dest_buf;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    ch_printf_color( d->character, "&g1&w) %-30.30s: %2d\n\r", "Saving vs. poison", victim->saving_poison_death );
    ch_printf_color( d->character, "&g2&w) %-30.30s: %2d\n\r", "Saving vs. wands", victim->saving_wand );
    ch_printf_color( d->character, "&g3&w) %-30.30s: %2d\n\r", "Saving vs. paralysis", victim->saving_para_petri );
    ch_printf_color( d->character, "&g4&w) %-30.30s: %2d\n\r", "Saving vs. breath", victim->saving_breath );
    ch_printf_color( d->character, "&g5&w) %-30.30s: %2d\n\r", "Saving vs. spells", victim->saving_spell_staff );
    send_to_char( "\n\rModify which saving throw: ", d->character );

    OLC_MODE(d) = MEDIT_SAVE_MENU;
}

void medit_disp_menu( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *victim = d->character->dest_buf;

    if ( !IS_NPC( victim ) )
	medit_disp_pc_menu(d);
    else
	medit_disp_npc_menu(d);
}
  
/*
 * Display main menu for NPCs
 */
void medit_disp_npc_menu( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *ch = d->character;
    CHAR_DATA *mob = d->character->dest_buf;
    int hitestimate, damestimate;

    if ( !mob->pIndexData->hitnodice )
	hitestimate = mob->level * 8 + number_range( mob->level * mob->level / 4, 
	    mob->level * mob->level );
    else
	hitestimate = mob->pIndexData->hitnodice * number_range(1, 
	    mob->pIndexData->hitsizedice ) + mob->pIndexData->hitplus;

    damestimate = number_range( mob->pIndexData->damnodice, 
	mob->pIndexData->damsizedice * mob->pIndexData->damnodice );
    damestimate += GET_DAMROLL(mob);

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    set_char_color( AT_PLAIN, d->character );
    ch_printf_color( ch, "-- Mob Number:  [&c%d&w]\n\r", mob->pIndexData->vnum );
    ch_printf_color( ch, "&g1&w) Sex: &O%-7.7s          &g2&w) Name: &O%s\n\r", mob->sex == SEX_MALE ? "male" : mob->sex == SEX_FEMALE  ? "female" : "neutral", mob->name );
    ch_printf_color( ch, "&g3&w) Shortdesc: &O%s\n\r", mob->short_descr[0] == '\0' ? "(none set)" : mob->short_descr );
    ch_printf_color( ch, "&g4&w) Longdesc:-\n\r&O%s\n\r", mob->long_descr[0] == '\0' ? "(none set)" : mob->long_descr );
    ch_printf_color( ch, "&g5&w) Description:-\n\r&O%-74.74s\n\r", mob->description );
    ch_printf_color( ch, "&g6&w) Class: [&c%-11.11s&w], &g7&w) Race:   [&c%-11.11s&w]\n\r",
	npc_class[mob->class], npc_race[mob->race] );
    ch_printf_color( ch, "&g8&w) Level:       [&c%5d&w], &g9&w) Alignment:    [&c%5d&w], &gA&w) Strength: [&c%5d&w]\n\r", 
	mob->level, mob->alignment, get_curr_str(mob) );
    ch_printf_color( ch, "&gB&w) Intelligence:[&c%5d&w], &gC&w) Widsom:       [&c%5d&w], &gD&w) Dexterity:[&c%5d&w]\n\r",
	get_curr_int(mob), get_curr_wis(mob), get_curr_dex(mob) );
    ch_printf_color( ch, "&gE&w) Constitution:[&c%5d&w], &gF&w) Charisma:     [&c%5d&w], &gG&w) Luck:     [&c%5d&w]\n\r",
	get_curr_con(mob), get_curr_cha(mob), get_curr_lck(mob) );
    ch_printf_color( ch, "&gH&w) DamNumDice:  [&c%5d&w], &gI&w) DamSizeDice:  [&c%5d&w], &gJ&w) DamPlus:  [&c%5d&w]=[&c%5d&w]\n\r",
        mob->pIndexData->damnodice, mob->pIndexData->damsizedice, mob->pIndexData->damplus, damestimate );
    ch_printf_color( ch, "&gK&w) HitNumDice:  [&c%5d&w], &gL&w) HitSizeDice:  [&c%5d&w], &gM&w) HitPlus:  [&c%5d&w]=[&c%5d&w]\r\n",
        mob->pIndexData->hitnodice, mob->pIndexData->hitsizedice, mob->pIndexData->hitplus, hitestimate );
#ifdef ENABLE_GOLD_SILVER_COPPER
    ch_printf_color( ch, "&gN&w) G/S/C: [&c%8dg&w||&c%8ds&w||&c%8dc&w], &gO&w) Spec: &O%-22.22s\n\r",
        mob->gold, mob->silver, mob->copper, lookup_spec( mob->spec_fun ) );
#else
    ch_printf_color( ch, "&gN&w) Gold:     [&c%8d&w], &gO&w) Spec: &O%-22.22s\n\r",
        mob->gold, lookup_spec( mob->spec_fun ) );
#endif
    ch_printf_color( ch, "&gP&w) Saving Throws\n\r" );
    ch_printf_color( ch, "&gR&w) Resistant   : &O%s\n\r", flag_string( mob->resistant, ris_flags ) );
    ch_printf_color( ch, "&gS&w) Immune      : &O%s\n\r", flag_string( mob->immune, ris_flags ) );
    ch_printf_color( ch, "&gT&w) Susceptible : &O%s\n\r", flag_string( mob->susceptible, ris_flags ) );
    ch_printf_color( ch, "&gU&w) Position    : &O%s\n\r", position_names[ (int) mob->position ] );
    ch_printf_color( ch, "&gV&w) Attacks     : &c%s\n\r", ext_flag_string( &mob->attacks, attack_flags ) );
    ch_printf_color( ch, "&gW&w) Defenses    : &c%s\n\r", ext_flag_string( &mob->defenses, defense_flags ) );
    ch_printf_color( ch, "&gX&w) Body Parts  : &c%s\n\r", flag_string(mob->xflags, part_flags) );
    ch_printf_color( ch, "&gY&w) Act Flags   : &c%s\n\r", ext_flag_string( &mob->act, act_flags ) );
    ch_printf_color( ch, "&gZ&w) Affected    : &c%s\n\r", affect_bit_name( &mob->affected_by ) );
    ch_printf_color( ch, "&gQ&w) Quit\n\r" );
    ch_printf_color( ch, "Enter choice : " );


    OLC_MODE(d) = MEDIT_NPC_MAIN_MENU;
}

void medit_disp_pc_menu( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *ch = d->character;
    CHAR_DATA *victim = d->character->dest_buf;

    write_to_buffer( d, "50\x1B[;H\x1B[2J", 0 );
    ch_printf_color( ch, "&g1&w) Sex: &O%-7.7s           &g2&w) Name: &O%s\n\r",
	victim->sex == SEX_MALE ? "male" : victim->sex == SEX_FEMALE  ? "female" : "neutral", victim->name );
    ch_printf_color( ch, "&g3&w) Description:-\n\r&O%-74.74s\n\r", victim->description );
    ch_printf_color( ch, "&g4&w) Class: [&c%-11.11s&w],  &g5&w) Race:   [&c%-11.11s&w]\n\r",
	class_table[victim->class]->who_name, race_table[victim->race]->race_name );
    ch_printf_color( ch, "&g6&w) Level:       [&c%5d&w],  &g7&w) Alignment:    [&c%5d&w],  &g8&w) Strength:  [&c%5d&w]\n\r", 
	victim->level, victim->alignment, get_curr_str(victim) );
    ch_printf_color( ch, "&g9&w) Intelligence:[&c%5d&w],  &gA&w) Widsom:       [&c%5d&w],  &gB&w) Dexterity: [&c%5d&w]\n\r",
	get_curr_int(victim), get_curr_wis(victim), get_curr_dex(victim) );
    ch_printf_color( ch, "&gC&w) Constitution:[&c%5d&w],  &gD&w) Charisma:     [&c%5d&w],  &gE&w) Luck:      [&c%5d&w]\n\r",
	get_curr_con(victim), get_curr_cha(victim), get_curr_lck(victim) );
    ch_printf_color( ch, "&gF&w) Hps:   [&c%5d&w/&c%5d&w],  &gG&w) Mana:   [&c%5d&w/&c%5d&w],  &gH&w) Move:[&c%5d&w/&c%-5d&w]\n\r",
	victim->hit, victim->max_hit, victim->mana, victim->max_mana, victim->move, victim->max_move );
#ifdef ENABLE_GOLD_SILVER_COPPER
    ch_printf_color( ch, "&gI&w) G/S/C: [&c%11d&w||&c%11d&w||&c%11d&w],  &gJ&w) Mentalstate:  [&c%5d&w],  &gK&w) Emotional: [&c%5d&w]\n\r",
        victim->gold, victim->silver, victim->copper, victim->mental_state, victim->emotional_state );
#else
    ch_printf_color( ch, "&gI&w) Gold:  [&c%11d&w],  &gJ&w) Mentalstate:  [&c%5d&w],  &gK&w) Emotional: [&c%5d&w]\n\r",
        victim->gold, victim->mental_state, victim->emotional_state );
#endif
    ch_printf_color( ch, "&gL&w) Thirst:      [&c%5d&w],  &gM&w) Full:         [&c%5d&w],  &gN&w) Drunk:     [&c%5d&w]\n\r",
	victim->pcdata->condition[COND_THIRST], victim->pcdata->condition[COND_FULL], victim->pcdata->condition[COND_DRUNK] );
    ch_printf_color( ch, "&gO&w) Favor:       [&c%5d&w]\n\r", victim->pcdata->favor );
    ch_printf_color( ch, "&gP&w) Saving Throws\n\r" );
    ch_printf_color( ch, "&gR&w) Resistant   : &O%s\n\r", flag_string( victim->resistant, ris_flags ) );
    ch_printf_color( ch, "&gS&w) Immune      : &O%s\n\r", flag_string( victim->immune, ris_flags ) );
    ch_printf_color( ch, "&gT&w) Susceptible : &O%s\n\r", flag_string( victim->susceptible, ris_flags ) );
    ch_printf_color( ch, "&gU&w) Position    : &O%s\n\r", position_names[ (int) victim->position ] );
    ch_printf_color( ch, "&gV&w) Act Flags   : &c%s\n\r", ext_flag_string( &victim->act, plr_flags ) );
    ch_printf_color( ch, "&gW&w) PC Flags    : &c%s\n\r", flag_string( victim->pcdata->flags, pc_flags ) );
    ch_printf_color( ch, "&gX&w) Affected    : &c%s\n\r", affect_bit_name( &victim->affected_by ) );
    ch_printf_color( ch, "&gY&w) Deity       : &O%s\n\r",
	victim->pcdata->deity ? victim->pcdata->deity->name : "None" );

  if ( get_trust(ch) >= LEVEL_GOD && victim->pcdata->clan )
    ch_printf_color( ch, "&gZ&w) %-12.12s: &O%s\n\r",
	victim->pcdata->clan->clan_type == CLAN_ORDER ? "Order" :
	victim->pcdata->clan->clan_type == CLAN_GUILD ? "Guild" : "Clan",
	victim->pcdata->clan->name );
  else if ( get_trust(ch) >= LEVEL_GOD && !victim->pcdata->clan )
    ch_printf_color( ch, "&gZ&w) Clan        : &ONone\n\r" );

  if ( get_trust(ch) >= LEVEL_SUB_IMPLEM )
    ch_printf_color( ch, "&g=&w) Council     : &O%s\n\r",
	victim->pcdata->council ? victim->pcdata->council->name : "None" );
    ch_printf_color( ch, "&gQ&w) Quit\n\r" );
    ch_printf_color( ch, "Enter choice : " );

    OLC_MODE(d) = MEDIT_PC_MAIN_MENU;
}

/*
 * Bogus command for resetting stuff
 */
void do_medit_reset( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim = ch->dest_buf;

    switch ( ch->substate )
    {
	default:
	    return;

	case SUB_MOB_DESC:
	    if ( !ch->dest_buf )
	    {
		send_to_char( "Fatal error, report to Tagith.\n\r", ch );
		bug( "do_medit_reset: sub_mob_desc: NULL ch->dest_buf", 0 );
		cleanup_olc( ch->desc );
		ch->substate = SUB_NONE;
		return;
	    }
	    STRFREE( victim->description );
	    victim->description = copy_buffer( ch );
	    if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
	    {
		STRFREE( victim->pIndexData->description );
		victim->pIndexData->description = QUICKLINK( victim->description );
	    }
	    stop_editing( ch );
	    ch->dest_buf = victim;
	    ch->substate = SUB_NONE;
	    ch->desc->connected = CON_MEDIT;
	    medit_disp_menu( ch->desc );
	    return;
    }
}

/**************************************************************************
  The GARGANTAUN event handler
 **************************************************************************/

void medit_parse( DESCRIPTOR_DATA *d, char *arg )
{
    CHAR_DATA *victim = d->character->dest_buf;
    int number=0, minattr, maxattr;
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CLAN_DATA *clan;
    DEITY_DATA *deity;
    COUNCIL_DATA *council;
    AREA_DATA *tarea;
    bool found = FALSE;
    char *pwdnew, *p;

    if ( IS_NPC( victim ) )
    {
        minattr = 1;
        maxattr = 25;
    }
    else
    {
        minattr = 3;
        maxattr = 18;
    }

    switch ( OLC_MODE(d) ) 
    {
    case MEDIT_CONFIRM_SAVESTRING:
	switch ( UPPER(*arg) )
	{
	case 'Y':
	    /* If its a mob, fold_area otherwise save_char_obj */
	    send_to_char("Saving...\n\r", d->character);
	    if ( IS_NPC(victim) )
	    {
		sprintf( log_buf, "OLC: %s edits mob %d(%s)", d->character->name, victim->pIndexData->vnum, victim->name );
		for ( tarea = first_asort; tarea; tarea = tarea->next )
		{
		    if ( OLC_VNUM(d) >= tarea->low_m_vnum
		    &&   OLC_VNUM(d) <= tarea->hi_m_vnum )
		    {
			if ( get_trust(d->character) >= LEVEL_SUB_IMPLEM )
			    fold_area( tarea, tarea->filename, FALSE );
			found = TRUE;
		    }
		}
		/* I'm assuming that if it isn't an installed area, its the char's */
		if ( !found 
		&&   (tarea = d->character->pcdata->area) != NULL
		&&   get_trust(d->character) >= LEVEL_CREATOR 
		&&   IS_SET(tarea->status, AREA_LOADED ) )
		{
		    tarea = d->character->pcdata->area;
		    sprintf( buf, "%s%s", BUILD_DIR, tarea->filename );
		    fold_area( tarea, buf, FALSE );
		}
	    }
            else
            {
                sprintf( log_buf, "OLC: %s edits %s", d->character->name, victim->name );
                save_char_obj( victim );
            }
	    log_string_plus( log_buf, LOG_BUILD, d->character->level );
	    cleanup_olc( d );
	    return;
	case 'N':
	    cleanup_olc( d );
	    return;
	default:
	    send_to_char("Invalid choice!\n\r", d->character);
	    send_to_char("Do you wish to save to disk? : ", d->character);
	    return;
	}
	break;

  case MEDIT_NPC_MAIN_MENU:
	switch (UPPER(*arg)) 
	{
	case 'Q':
	    cleanup_olc( d );
	    return;
	case '1':
	    OLC_MODE(d) = MEDIT_SEX;
	    medit_disp_sex(d);
	    return;
	case '2':
	    OLC_MODE(d) = MEDIT_NAME;
	    send_to_char( "\n\rEnter name: ", d->character );
	    return;
	case '3':
	    OLC_MODE(d) = MEDIT_S_DESC;
	    send_to_char( "\n\rEnter short description: ", d->character );
	    return;
	case '4':
	    OLC_MODE(d) = MEDIT_L_DESC;
	    send_to_char( "\n\rEnter long description: ", d->character );
	    return;
	case '5':
	    OLC_MODE(d) = MEDIT_D_DESC;
	    d->character->substate = SUB_MOB_DESC;
	    d->character->last_cmd = do_medit_reset;
	
	    send_to_char("Enter new mob description:\r\n", d->character);
	    if ( !victim->description )
		victim->description = STRALLOC( "" );
	    start_editing( d->character, victim->description );
	    return;
	case '6':
	    OLC_MODE(d) = MEDIT_CLASS;
	    medit_disp_classes(d);
	    return;
	case '7':
	    OLC_MODE(d) = MEDIT_RACE;
	    medit_disp_races(d);
	   return;
	case '8':
	    OLC_MODE(d) = MEDIT_LEVEL;
	    send_to_char( "\n\rEnter level: ", d->character );
	    return;
	case '9':
	    OLC_MODE(d) = MEDIT_ALIGNMENT;
	    send_to_char( "\n\rEnter alignment: ", d->character );
            return;
	case 'A':
	    OLC_MODE(d) = MEDIT_STRENGTH;
	    send_to_char( "\n\rEnter strength: ", d->character );
	    return;
	case 'B':
	    OLC_MODE(d) = MEDIT_INTELLIGENCE;
	    send_to_char( "\n\rEnter intelligence: ", d->character );
	    return;
	case 'C':
	    OLC_MODE(d) = MEDIT_WISDOM;
	    send_to_char( "\n\rEnter wisdom: ", d->character );
	    return;
	case 'D':
	    OLC_MODE(d) = MEDIT_DEXTERITY;
	    send_to_char( "\n\rEnter dexterity: ", d->character );
	    return;
	case 'E':
	    OLC_MODE(d) = MEDIT_CONSTITUTION;
	    send_to_char( "\n\rEnter constitution: ", d->character );
	    return;
	case 'F':
	    OLC_MODE(d) = MEDIT_CHARISMA;
	    send_to_char( "\n\rEnter charisma: ", d->character );
	    return;
	case 'G':
	    OLC_MODE(d) = MEDIT_LUCK;
	    send_to_char( "\n\rEnter luck: ", d->character );
	    return;
	case 'H':
	    OLC_MODE(d) = MEDIT_DAMNUMDIE;
	    send_to_char( "\n\rEnter number of damage dice: ", d->character );
	    return;
	case 'I':
	    OLC_MODE(d) = MEDIT_DAMSIZEDIE;
	    send_to_char( "\n\rEnter size of damage dice: ", d->character );
	    return;
	case 'J':
	    OLC_MODE(d) = MEDIT_DAMPLUS;
	    send_to_char( "\n\rEnter amount to add to damage: ", d->character );
	    return;
	case 'K':
	    OLC_MODE(d) = MEDIT_HITNUMDIE;
	    send_to_char( "\n\rEnter number of hitpoint dice: ", d->character );
	    return;
	case 'L':
	    OLC_MODE(d) = MEDIT_HITSIZEDIE;
	    send_to_char( "\n\rEnter size of hitpoint dice: ", d->character );
	    return;
	case 'M':
	    OLC_MODE(d) = MEDIT_HITPLUS;
	    send_to_char( "\n\rEnter amount to add to hitpoints: ", d->character );
	    return;
	case 'N':
#ifdef ENABLE_GOLD_SILVER_COPPER
	    OLC_MODE(d) = MEDIT_COPPER;
	    send_to_char( "\n\rEnter amount of copper mobile carries: ", d->character );
#else
	    OLC_MODE(d) = MEDIT_GOLD;
	    send_to_char( "\n\rEnter amount of gold mobile carries: ", d->character );
#endif
	    return;
	case 'O':
	    OLC_MODE(d) = MEDIT_SPEC;
	    medit_disp_spec(d);
	    return;
	case 'P':
	    OLC_MODE(d) = MEDIT_SAVE_MENU;
	    medit_disp_saving_menu(d);
	    return;
	case 'R':
	    OLC_MODE(d) = MEDIT_RESISTANT;
	    medit_disp_ris(d);
	    return;
	case 'S':
	    OLC_MODE(d) = MEDIT_IMMUNE;
	    medit_disp_ris(d);
	    return;
	case 'T':
	    OLC_MODE(d) = MEDIT_SUSCEPTIBLE;
	    medit_disp_ris(d);
	    return;
	case 'U':
	    OLC_MODE(d) = MEDIT_POS;
	    medit_disp_positions(d);
	    return;
	case 'V':
	    OLC_MODE(d) = MEDIT_ATTACK;
	    medit_disp_attack_menu(d);
	    return;
	case 'W':
	    OLC_MODE(d) = MEDIT_DEFENSE;
	    medit_disp_defense_menu(d);
	    return;
	case 'X':
	    OLC_MODE(d) = MEDIT_PARTS;
	    medit_disp_parts(d);
	    return;
	case 'Y':
	    OLC_MODE(d) = MEDIT_NPC_FLAGS;
	    medit_disp_mob_flags(d);
	    return;
	case 'Z':
	    OLC_MODE(d) = MEDIT_AFF_FLAGS;
	    medit_disp_aff_flags(d);
	    return;
	default:
	    medit_disp_npc_menu(d);
	    return;
	}
	break;

  case MEDIT_PC_MAIN_MENU:
	switch (UPPER(*arg)) 
	{
	case 'Q':
	    if ( OLC_CHANGE(d) )
	    {
		send_to_char( "Do you wish to save changes to disk? (y/n): ", d->character );
		OLC_MODE(d) = MEDIT_CONFIRM_SAVESTRING;
	    }
	    else
		cleanup_olc( d );
	    return;
	case '1':
	    OLC_MODE(d) = MEDIT_SEX;
	    medit_disp_sex(d);
	    return;
	case '2':
	    OLC_MODE(d) = MEDIT_NAME;
	    return;
	case '3':
	    OLC_MODE(d) = MEDIT_D_DESC;
	    d->character->substate = SUB_MOB_DESC;
	    d->character->last_cmd = do_medit_reset;
	
	    send_to_char("Enter new player description:\r\n", d->character);
	    if ( !victim->description )
		victim->description = STRALLOC( "" );
	    start_editing( d->character, victim->description );
	    return;
	case '4':
	    OLC_MODE(d) = MEDIT_CLASS;
	    medit_disp_classes(d);
	    return;
	case '5':
	    OLC_MODE(d) = MEDIT_RACE;
	    medit_disp_races(d);
	    return;
	case '6':
	    send_to_char( "\n\rNPC Only!!", d->character );
	    break;
	case '7':
	    OLC_MODE(d) = MEDIT_ALIGNMENT;
	    send_to_char( "\n\rEnter alignment: ", d->character );
            return;
	case '8':
	    OLC_MODE(d) = MEDIT_STRENGTH;
	    send_to_char( "\n\rEnter strength: ", d->character );
	    return;
	case '9':
	    OLC_MODE(d) = MEDIT_INTELLIGENCE;
	    send_to_char( "\n\rEnter intelligence: ", d->character );
	    return;
	case 'A':
	    OLC_MODE(d) = MEDIT_WISDOM;
	    send_to_char( "\n\rEnter wisdom: ", d->character );
	    return;
	case 'B':
	    OLC_MODE(d) = MEDIT_DEXTERITY;
	    send_to_char( "\n\rEnter dexterity: ", d->character );
	    return;
	case 'C':
	    OLC_MODE(d) = MEDIT_CONSTITUTION;
	    send_to_char( "\n\rEnter constitution: ", d->character );
	    return;
	case 'D':
	    OLC_MODE(d) = MEDIT_CHARISMA;
	    send_to_char( "\n\rEnter charisma: ", d->character );
	    return;
	case 'E':
	    OLC_MODE(d) = MEDIT_LUCK;
	    send_to_char( "\n\rEnter luck: ", d->character );
	    return;
	case 'F':
	    OLC_MODE(d) = MEDIT_HITPOINT;
	    send_to_char( "\n\rEnter hitpoints: ", d->character );
	    return;
	case 'G':
	    OLC_MODE(d) = MEDIT_MANA;
	    send_to_char( "\n\rEnter mana: ", d->character );
	    return;
	case 'H':
	    OLC_MODE(d) = MEDIT_MOVE;
	    send_to_char( "\n\rEnter moves: ", d->character );
	    return;
	case 'I':
#ifdef ENABLE_GOLD_SILVER_COPPER
	    OLC_MODE(d) = MEDIT_COPPER;
	    send_to_char( "\n\rEnter amount of copper player carries: ", d->character );
#else
	    OLC_MODE(d) = MEDIT_GOLD;
	    send_to_char( "\n\rEnter amount of gold player carries: ", d->character );
#endif
	    return;
	case 'J':
	    OLC_MODE(d) = MEDIT_MENTALSTATE;
	    send_to_char( "\n\rEnter players mentalstate: ", d->character );
	    return;
	case 'K':
	    OLC_MODE(d) = MEDIT_EMOTIONAL;
	    send_to_char( "\n\rEnter players emotional state: ", d->character );
	    return;
	case 'L':
	    OLC_MODE(d) = MEDIT_THIRST;
 	    send_to_char( "\n\rEnter player's thirst (0 = dehydrated): ", d->character );
	    return;
	case 'M':
	    OLC_MODE(d) = MEDIT_FULL;
	    send_to_char( "\n\rEnter player's fullness (0 = starving): ", d->character );
	    return;
	case 'N':
	    OLC_MODE(d) = MEDIT_DRUNK;
	    send_to_char( "\n\rEnter player's drunkeness (0 = sober): ", d->character );
	    return;
	case 'O':
	    OLC_MODE(d) = MEDIT_FAVOR;
	    send_to_char( "\n\rEnter player's favor (-2500 to 2500): ", d->character );
	    return;
	case 'P':
	    OLC_MODE(d) = MEDIT_SAVE_MENU;
	    medit_disp_saving_menu(d);
	    return;
	case 'R':
	    OLC_MODE(d) = MEDIT_RESISTANT;
	    medit_disp_ris(d);
	    return;
	case 'S':
	    OLC_MODE(d) = MEDIT_IMMUNE;
	    medit_disp_ris(d);
	    return;
	case 'T':
	    OLC_MODE(d) = MEDIT_SUSCEPTIBLE;
	    medit_disp_ris(d);
	    return;
	case 'U':
	    send_to_char( "NPCs Only!!\n\r", d->character );
	    break;
	case 'V':
	    OLC_MODE(d) = MEDIT_PC_FLAGS;
	    medit_disp_plr_flags(d);
	    return;
	case 'W':
	    OLC_MODE(d) = MEDIT_PCDATA_FLAGS;
	    medit_disp_pcdata_flags(d);
	    return;
	case 'X':
	    OLC_MODE(d) = MEDIT_AFF_FLAGS;
	    medit_disp_aff_flags(d);
	    return;
	case 'Y':
	    OLC_MODE(d) = MEDIT_DEITY;
	    medit_disp_deities(d);
	    return;
	case 'Z':
	    if ( get_trust(d->character) < LEVEL_GOD )
		break;
	    OLC_MODE(d) = MEDIT_CLAN;
	    medit_disp_clans(d);
	    return;
	case '=':
	    if ( get_trust(d->character) < LEVEL_SUB_IMPLEM )
		break;
	    OLC_MODE(d) = MEDIT_COUNCIL;
	    medit_disp_councils(d);
	    return;
	default:
	    medit_disp_npc_menu(d);
	    return;
	}
	break;

        
    case MEDIT_NAME:
	if ( !IS_NPC(victim) && get_trust( d->character ) > LEVEL_SUB_IMPLEM-1 )
	{
	    sprintf( buf, "%s %s", victim->name, arg );
	    do_pcrename( d->character, buf );
	    olc_log( d, "Changes name to %s", arg );
	    return;
	}
	STRFREE( victim->name );
	victim->name = STRALLOC( arg );
	if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE) )
	{
	    STRFREE( victim->pIndexData->player_name );
	    victim->pIndexData->player_name = QUICKLINK( victim->name );
	}
	olc_log( d, "Changed name to %s", arg );
	break;

    case MEDIT_S_DESC:
	STRFREE( victim->short_descr );
	victim->short_descr = STRALLOC( arg );
	if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE) )
	{
	    STRFREE( victim->pIndexData->short_descr );
	    victim->pIndexData->short_descr = QUICKLINK( victim->short_descr );
	}
	olc_log( d, "Changed short desc to %s", arg );
        break;

    case MEDIT_L_DESC:
	STRFREE( victim->long_descr );
	strcpy( buf, arg );
	strcat( buf, "\n\r" );
	victim->long_descr = STRALLOC( buf ); 
	if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE) )
	{
	    STRFREE( victim->pIndexData->long_descr );
	    victim->pIndexData->long_descr = QUICKLINK( victim->long_descr );
	}
	olc_log( d, "Changed long desc to %s", arg );
	break;

    case MEDIT_D_DESC:
	/*. We should never get here .*/
	cleanup_olc( d );
	bug( "OLC: medit_parse(): Reached D_DESC case!", 0 );
	break;

    case MEDIT_NPC_FLAGS:
        /* REDONE, again, then again */
	if ( is_number( arg ) )
	    if ( atoi(arg) == 0 )
		break;

        while ( arg[0] != '\0' )
        {
            arg = one_argument( arg, arg1 );
            
            if ( is_number( arg1 ) )
            {
                number = atoi( arg1 );
                number -= 1;

                if ( number < 0 || number > MAX_ACT_FLAGS )
                {
                    send_to_char( "Invalid flag, try again: ", d->character );
                    return;
                }
            }
            else
            {
                number = get_actflag( arg1 );
                if ( number < 0 )
                {
                    send_to_char( "Invalid flag, try again: ", d->character );
                    return;
                }
            }
            if ( IS_NPC(victim)
            &&   number == ACT_PROTOTYPE
            &&   get_trust( d->character ) < LEVEL_GREATER
            &&   !is_name( "protoflag", d->character->pcdata->bestowments ) )
                send_to_char( "You don't have permission to change the prototype flag.\n\r", d->character );
            else if( IS_NPC(victim) && number == ACT_IS_NPC )
                send_to_char( "It isn't possible to change that flag.\n\r", d->character );
            else
            {
                xTOGGLE_BIT( victim->act, number );
            }
            if ( IS_NPC(victim) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
                victim->pIndexData->act = victim->act;
        }
	medit_disp_mob_flags(d);
	return;

    case MEDIT_PC_FLAGS:
        if ( is_number( arg ) )
        {
            number = atoi(arg);
            if ( number == 0 )
                break;
            if ( (number > 0) || (number < MAX_OLC_ITEMS_LIST) )
            {
		number -= 1; /* offset :P */
                xTOGGLE_BIT( victim->act, number );
		olc_log( d, "%s the flag %s", xIS_SET( victim->act, number ) ? "Added" : "Removed",
		    plr_flags[number] );
            }
        }
        else
        {
            while ( arg[0] != '\0' )
            {
                arg = one_argument( arg, arg1 );
                number = get_actflag( arg1 );
		if ( number > 0 )
		{
                    xTOGGLE_BIT( victim->act, number );
		    olc_log( d, "%s the flag %s", xIS_SET( victim->act, number ) ? "Added" : "Removed",
			plr_flags[number] );
		}
            }
        }
        medit_disp_plr_flags(d);
        return;

    case MEDIT_PCDATA_FLAGS:
        if ( is_number( arg ) )
        {
            number = atoi(arg);
            if ( number == 0 )
                break;
            if ( (number > 0) || (number < MAX_OLC_ITEMS_LIST) )
            {
		number -= 1;
                TOGGLE_BIT( victim->pcdata->flags, 1 << number );
		olc_log( d, "%s the pcflag %s", IS_SET( victim->pcdata->flags, 1 << number ) ? "Added" : "Removed",
		    pc_flags[number] );
            }
        }
        else
        {
            while ( arg[0] != '\0' )
            {
                arg = one_argument( arg, arg1 );
                number = get_actflag( arg1 );
		if ( number > 0 )
		{
                    TOGGLE_BIT( victim->pcdata->flags, 1 << number );
		    olc_log( d, "%s the pcflag %s", IS_SET( victim->pcdata->flags, 1 << number ) ? "Added" : "Removed",
			pc_flags[number] );
		}
            }
        }
	medit_disp_pcdata_flags(d);
	return;

    case MEDIT_AFF_FLAGS:
        if ( is_number( arg ) )
        {
            number = atoi( arg );
            if ( number == 0 )
                break;
            if ( (number > 0) || (number < MAX_OLC_ITEMS_LIST) )
            {
		number -= 1;
                xTOGGLE_BIT( victim->affected_by, number );
		olc_log( d, "%s the affect %s", xIS_SET( victim->affected_by, number ) ? "Added" : "Removed",
		    a_flags[number] );
            }
        }
        else
        {
            while ( arg[0] != '\0' )
            {
                arg = one_argument( arg, arg1 );
                number = get_actflag( arg1 );
		if ( number > 0 )
		{
                    xTOGGLE_BIT( victim->affected_by, number );
		    olc_log( d, "%s the affect %s", xIS_SET( victim->affected_by, number ) ? "Added" : "Removed",
			a_flags[number] );
		}
            }
        }
	if ( IS_NPC(victim) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
	    victim->pIndexData->affected_by = victim->affected_by;
	medit_disp_aff_flags(d);
	return;
    
/*-------------------------------------------------------------------*/
/*. Numerical responses .*/
    case MEDIT_HITPOINT:
	victim->max_hit = URANGE( 1, atoi(arg), 32700 );
	olc_log( d, "Changed hitpoints to %d", victim->max_hit );
	break;

    case MEDIT_MANA:
	victim->max_mana = URANGE( 1, atoi(arg), 30000 );
	olc_log( d, "Changed mana to %d", victim->max_mana );
	break;

    case MEDIT_MOVE:
	victim->max_move = URANGE( 1, atoi(arg), 30000 );
	olc_log( d, "Changed moves to %d", victim->max_move );
	break;

    case MEDIT_PRACTICE:
	victim->practice = URANGE( 1, atoi(arg), 300 );
	olc_log( d, "Changed practives to %d", victim->practice );
	break;

    case MEDIT_PASSWORD:
	if ( get_trust(d->character) < LEVEL_SUB_IMPLEM )
	    break;
	if ( strlen(arg) < 5 )
	{
	    send_to_char( "Password too short, try again: ", d->character );
	    return;
	}
	pwdnew = sha256_crypt( arg, victim->name );
	for ( p = pwdnew; *p != '\0'; p++ )
	{
	    if ( *p == '~' )
	    {
		send_to_char( "Unacceptable choice, try again: ", d->character );
		return;
	    }
	}
	DISPOSE( victim->pcdata->pwd );
	victim->pcdata->pwd = str_dup( pwdnew );
	if ( IS_SET(sysdata.save_flags, SV_PASSCHG) )
	    save_char_obj( victim );
	olc_log( d, "Modified password" );
	break;

    case MEDIT_SAV1:
	victim->saving_poison_death = URANGE( -30, atoi(arg), 30 );
	if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE) )
	    victim->pIndexData->saving_poison_death = victim->saving_poison_death;
	medit_disp_saving_menu(d);
	olc_log( d, "Changed save_poison_death to %d", victim->saving_poison_death );
	return;

    case MEDIT_SAV2:
	victim->saving_wand = URANGE( -30, atoi(arg), 30 );
	if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE) )
	    victim->pIndexData->saving_wand = victim->saving_wand;
	medit_disp_saving_menu(d);
	olc_log( d, "Changed save_wand to %d", victim->saving_wand );
	return;

    case MEDIT_SAV3:
	victim->saving_para_petri = URANGE( -30, atoi(arg), 30 );
	if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE) )
	    victim->pIndexData->saving_para_petri = victim->saving_para_petri;
	medit_disp_saving_menu(d);
	olc_log( d, "Changed save_paralysis_petrification to %d", victim->saving_para_petri );
	return;

    case MEDIT_SAV4:
	victim->saving_breath = URANGE( -30, atoi(arg), 30 );
	if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE) )
	    victim->pIndexData->saving_breath = victim->saving_breath;
	medit_disp_saving_menu(d);
	olc_log( d, "Changed save_breath to %d", victim->saving_breath );
	return;

    case MEDIT_SAV5:
	victim->saving_spell_staff = URANGE( -30, atoi(arg), 30 );
	if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE) )
	    victim->pIndexData->saving_spell_staff = victim->saving_spell_staff;
	medit_disp_saving_menu(d);
	olc_log( d, "Changed save_spell_staff to %d", victim->saving_spell_staff );
	return;

    case MEDIT_STRENGTH:
        victim->perm_str = URANGE( minattr, atoi( arg ), maxattr );
	if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE) )
	    victim->pIndexData->perm_str = victim->perm_str;
	olc_log( d, "Changed strength to %d", victim->perm_str );
	break;

    case MEDIT_INTELLIGENCE:
        victim->perm_int = URANGE( minattr, atoi( arg ), maxattr );
	if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE) )
	    victim->pIndexData->perm_int = victim->perm_int;
	olc_log( d, "Changed intelligence to %d", victim->perm_int );
        break;

    case MEDIT_WISDOM:
        victim->perm_wis = URANGE( minattr, atoi( arg ), maxattr );
	if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE) )
	    victim->pIndexData->perm_wis = victim->perm_wis;
	olc_log( d, "Changed victim wisdom to %d", victim->perm_wis );
        break;

    case MEDIT_DEXTERITY:
        victim->perm_dex = URANGE( minattr, atoi( arg ), maxattr );
	if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE) )
	    victim->pIndexData->perm_dex = victim->perm_dex;
	olc_log( d, "Changed dexterity to %d", victim->perm_dex );
        break;

    case MEDIT_CONSTITUTION:
        victim->perm_con = URANGE( minattr, atoi( arg ), maxattr );
	if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE) )
	    victim->pIndexData->perm_con = victim->perm_con;
	olc_log( d, "Changed constitution to %d", victim->perm_con );
        break;

    case MEDIT_CHARISMA:
        victim->perm_cha = URANGE( minattr, atoi( arg ), maxattr );
	if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE) )
	    victim->pIndexData->perm_cha = victim->perm_cha;
	olc_log( d, "Changed charisma to %d", victim->perm_cha );
        break;

    case MEDIT_LUCK:
        victim->perm_lck = URANGE( minattr, atoi( arg ), maxattr );
	if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE) )
	    victim->pIndexData->perm_lck = victim->perm_lck;
	olc_log( d, "Changed luck to %d", victim->perm_lck );
        break;

    case MEDIT_SEX:
	victim->sex = URANGE( 0, atoi( arg ), 2 );
	if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE) )
	    victim->pIndexData->sex = victim->sex;
	olc_log( d, "Changed sex to %s", victim->sex == 1 ? "Male" : victim->sex == 2 ? "Female" : "Neutral" );
	break;

    case MEDIT_HITROLL:
	victim->hitroll = URANGE( 0, atoi(arg), 85);
	if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE) )
	    victim->pIndexData->hitroll = victim->hitroll;
	olc_log( d, "Changed hitroll to %d", victim->hitroll );
	break;

    case MEDIT_DAMROLL:
	victim->damroll = URANGE( 0, atoi(arg), 65 );
	if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE) )
	    victim->pIndexData->damroll = victim->damroll;
	olc_log( d, "Changed damroll to %d", victim->damroll );
	break;

    case MEDIT_DAMNUMDIE:
        if ( IS_NPC(victim) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
	    victim->pIndexData->damnodice = URANGE( 0, atoi(arg), 100 );
	olc_log( d, "Changed damnumdie to %d", victim->pIndexData->damnodice );
	break;

    case MEDIT_DAMSIZEDIE:
        if ( IS_NPC(victim) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
	    victim->pIndexData->damsizedice = URANGE( 0, atoi(arg), 100 );
	olc_log( d, "Changed damsizedie to %d", victim->pIndexData->damsizedice );
	break;

    case MEDIT_DAMPLUS:
        if ( IS_NPC(victim) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
	    victim->pIndexData->damplus = URANGE( 0, atoi(arg), 1000 );
	olc_log( d, "Changed damplus to %d", victim->pIndexData->damplus );
	break;

    case MEDIT_HITNUMDIE:
	if ( IS_NPC(victim) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
	    victim->pIndexData->hitnodice = URANGE( 0, atoi(arg), 32767 );
	olc_log( d, "Changed hitnumdie to %d", victim->pIndexData->hitnodice );
	break;

    case MEDIT_HITSIZEDIE:
	if ( IS_NPC(victim) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
	    victim->pIndexData->hitsizedice = URANGE( 0, atoi(arg), 30000 );
	olc_log( d, "Changed hitsizedie to %d", victim->pIndexData->hitsizedice );
	break;

    case MEDIT_HITPLUS:
	if ( IS_NPC(victim) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
	    victim->pIndexData->hitplus = URANGE( 0, atoi(arg), 30000 );
	olc_log( d, "Changed hitplus to %d", victim->pIndexData->hitplus );
	break;

    case MEDIT_AC:
	victim->armor = URANGE( -300, atoi(arg), 300 );
	olc_log( d, "Changed armor to %d", victim->armor );
	break;

    case MEDIT_GOLD:
	victim->gold = UMAX( 0, atoi(arg) );
	olc_log( d, "Changed gold to %d", victim->gold );
	break;

#ifdef ENABLE_GOLD_SILVER_COPPER
    case MEDIT_SILVER:
	victim->silver = UMAX( 0, atoi(arg) );
	olc_log( d, "Changed silver to %d", victim->silver );
	break;

    case MEDIT_COPPER:
	victim->copper = UMAX( 0, atoi(arg) );
	olc_log( d, "Changed copper to %d", victim->copper );
	break;
#endif

    case MEDIT_POS:
	victim->position = URANGE( 0, atoi(arg), POS_STANDING );
	olc_log( d, "Changed position to %d", victim->position );
	break;

    case MEDIT_DEFAULT_POS:
	victim->defposition = URANGE( 0, atoi(arg), POS_STANDING );
	olc_log( d, "Changed default position to %d", victim->defposition );
	break;

    case MEDIT_MENTALSTATE:
	victim->mental_state = URANGE( -100, atoi(arg), 100 );
	olc_log( d, "Changed mental state to %d", victim->mental_state );
	break;

    case MEDIT_EMOTIONAL:
	victim->emotional_state = URANGE( -100, atoi(arg), 100 );
	olc_log( d, "Changed emotional state to %d", victim->emotional_state );
	break;

    case MEDIT_THIRST:
	victim->pcdata->condition[COND_THIRST] = URANGE( 0, atoi(arg), 100 );
	olc_log( d, "Changed thirst to %d", victim->pcdata->condition[COND_THIRST] );
	break;

    case MEDIT_FULL:
	victim->pcdata->condition[COND_FULL] = URANGE( 0, atoi(arg), 100 );
	olc_log( d, "Changed hunger to %d", victim->pcdata->condition[COND_FULL] );
	break;

    case MEDIT_DRUNK:
	victim->pcdata->condition[COND_DRUNK] = URANGE( 0, atoi(arg), 100 );
	olc_log( d, "Changed drunkness to %d", victim->pcdata->condition[COND_DRUNK] );
	break;

    case MEDIT_FAVOR:
	victim->pcdata->favor = URANGE( -2500, atoi(arg), 2500 );
	olc_log( d, "Changed favor to %d", victim->pcdata->favor );
	break;

    case MEDIT_SAVE_MENU:
	number = atoi( arg );
	switch( number )
	{
	default:
	    send_to_char( "Invalid saving throw, try again: ", d->character );
	    return;
	case 0:
	    break;
	case 1:
	    OLC_MODE(d) = MEDIT_SAV1;
	    send_to_char( "\n\rEnter throw (-30 to 30): ", d->character );
	    return;
	case 2:
	    OLC_MODE(d) = MEDIT_SAV2;
	    send_to_char( "\n\rEnter throw (-30 to 30): ", d->character );
	    return;
	case 3:
	    OLC_MODE(d) = MEDIT_SAV3;
	    send_to_char( "\n\rEnter throw (-30 to 30): ", d->character );
	    return;
	case 4:
	    OLC_MODE(d) = MEDIT_SAV4;
	    send_to_char( "\n\rEnter throw (-30 to 30): ", d->character );
	    return;
	case 5:
	    OLC_MODE(d) = MEDIT_SAV5;
	    send_to_char( "\n\rEnter throw (-30 to 30): ", d->character );
	    return;
	}
	/* If we reach here, we are going back to the main menu */
	break;

    case MEDIT_CLASS:
	number = atoi( arg );
	if ( IS_NPC(victim) )
	{
	    victim->class = URANGE( 0, number, MAX_NPC_CLASS-1 );
	    if ( xIS_SET(victim->act, ACT_PROTOTYPE) )
		victim->pIndexData->class = victim->class;
	    break;
	}
	victim->class = URANGE( 0, number, MAX_CLASS );
	olc_log( d, "Changed class to %s", npc_class[victim->class] );
	break;

    case MEDIT_RACE:
	number = atoi( arg );
	if ( IS_NPC(victim ) )
	{
	    victim->race = URANGE( 0, number, MAX_NPC_RACE-1 );
	    if ( xIS_SET(victim->act, ACT_PROTOTYPE) )
		victim->pIndexData->race = victim->race;
	    break;
	}
	victim->race = URANGE( 0, number, MAX_RACE-1 );
	olc_log( d, "Changed race to %s", npc_race[victim->race] );
	break;

    case MEDIT_PARTS:
	number = atoi( arg );
	if ( number < 0 || number > 29 )
	{
	    send_to_char( "Invalid part, try again: ", d->character );
	    return;
	}
	else
	{
	    if ( number == 0 )
		break;
	    else
	    {
		number -= 1;
		TOGGLE_BIT( victim->xflags, 1 << number );
	    }
	    if ( IS_NPC( victim ) && xIS_SET(victim->act, ACT_PROTOTYPE) )
		victim->pIndexData->xflags = victim->xflags;
	}
	olc_log( d, "%s the body part %s", IS_SET( victim->xflags, 1 << (number - 1) ) ? "Added" : "Removed",
	    part_flags[number] );
	medit_disp_parts(d);
	return;

    case MEDIT_ATTACK:
	if ( is_number( arg ) )
	{
	    number = atoi( arg );
	    if ( number == 0 )
		break;

	    number -= 1; /* offset */
	    if ( number < 0 || number > MAX_ATTACK_TYPE+1 )
	    {
		send_to_char( "Invalid flag, try again: ", d->character );
		return;
	    }
	    else
		xTOGGLE_BIT( victim->attacks, number );
	}
	else
	{
	    while ( arg[0] != '\0' )
	    {
		arg = one_argument( arg, arg1 );
		number = get_attackflag( arg1 );
		if ( number < 0 )
		{
		    send_to_char( "Invalid flag, try again: ", d->character );
		    return;
		}
		xTOGGLE_BIT( victim->attacks, number );
	    }
	}
	if ( IS_NPC(victim) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
	    victim->pIndexData->attacks = victim->attacks;
	medit_disp_attack_menu(d);
	olc_log( d, "%s the attack %s", xIS_SET(victim->attacks,number)? "Added" : "Removed", attack_flags[number] );
 	return;

    case MEDIT_DEFENSE:
	if ( is_number( arg ) )
	{
	    number = atoi( arg );
	    if ( number == 0 )
		break;

	    number -= 1; /* offset */
	    if ( number < 0 || number > MAX_DEFENSE_TYPE+1 )
	    {
		send_to_char( "Invalid flag, try again: ", d->character );
		return;
	    }
	    else
		xTOGGLE_BIT( victim->defenses, number );
	}
	else
	{
	    while ( arg[0] != '\0' )
	    {
		arg = one_argument( arg, arg1 );
		number = get_defenseflag( arg1 );
		if ( number < 0 )
		{
		    send_to_char( "Invalid flag, try again: ", d->character );
		    return;
		}
		xTOGGLE_BIT( victim->defenses, number );
	    }
	}
	if ( IS_NPC(victim) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
	    victim->pIndexData->defenses = victim->defenses;
	medit_disp_defense_menu(d);
	olc_log( d, "%s the attack %s", xIS_SET(victim->defenses,number) ? "Added" : "Removed", defense_flags[number] );
	return;

    case MEDIT_LEVEL:
	victim->level = URANGE( 1, atoi(arg), MAX_LEVEL-1 );
	olc_log( d, "Changed level to %d", victim->level );
	break;

    case MEDIT_ALIGNMENT:
	victim->alignment = URANGE( -1000, atoi(arg), 1000 );
	olc_log( d, "Changed alignment to %d", victim->alignment );
	break;

    case MEDIT_RESISTANT:
        if ( is_number( arg ) )
        {
            number = atoi( arg );
            if ( number == 0 )
                break;

	    number -= 1; /* offset */
            if ( number < 0 || number > MAX_RIS_FLAG+1 )
            {
                send_to_char( "Invalid flag, try again: ", d->character );
                return;
            }
            TOGGLE_BIT( victim->resistant, 1 << number );
        }
        else
        {
            while ( arg[0] != '\0' )
            {
                arg = one_argument( arg, arg1 );
                number = get_risflag( arg1 );
                if ( number < 0 )
		{
		    send_to_char( "Invalid flag, try again: ", d->character );
		    return;
		}
                TOGGLE_BIT( victim->resistant, 1 << number );
            }
        }
        if ( IS_NPC(victim) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->resistant = victim->resistant;
	medit_disp_ris(d);
	olc_log( d, "%s the resistant %s", IS_SET( victim->resistant, 1 << number ) ? "Added" : "Removed", ris_flags[number] );
	return;

    case MEDIT_IMMUNE:
        if ( is_number( arg ) )
        {
            number = atoi( arg );
            if ( number == 0 )
                break;

	    number -= 1;
            if ( number < 0 || number > MAX_RIS_FLAG+1 )
            {
                send_to_char( "Invalid flag, try again: ", d->character );
                return;
            }
            TOGGLE_BIT( victim->immune, 1 << number );
        }
        else
        {
            while ( arg[0] != '\0' )
            {
                arg = one_argument( arg, arg1 );
                number = get_risflag( arg1 );
                if ( number < 0 )
		{
		    send_to_char( "Invalid flag, try again: ", d->character );
		    return;
		}
                TOGGLE_BIT( victim->immune, 1 << number );
            }
        }
        if ( IS_NPC(victim) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->immune = victim->immune;

	medit_disp_ris(d);
	olc_log( d, "%s the immune %s", IS_SET( victim->immune, 1 << number ) ? "Added" : "Removed", ris_flags[number] );
	return;

    case MEDIT_SUSCEPTIBLE:
        if ( is_number( arg ) )
        {
            number = atoi( arg );
            if ( number == 0 )
                break;

	    number -= 1;
            if ( number < 0 || number > MAX_RIS_FLAG+1 )
            {
                send_to_char( "Invalid flag, try again: ", d->character );
                return;
            }
            TOGGLE_BIT( victim->susceptible, 1 << number );
        }
        else
        {
            while ( arg[0] != '\0' )
            {
                arg = one_argument( arg, arg1 );
                number = get_risflag( arg1 );
		if ( number < 0 )
		{
		    send_to_char( "Invalid flag, try again: ", d->character );
		    return;
		}
                TOGGLE_BIT( victim->susceptible, 1 << number );
            }
        }
        if ( IS_NPC(victim) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->susceptible = victim->susceptible;
	medit_disp_ris(d);
	olc_log( d, "%s the suscept %s", IS_SET( victim->susceptible, 1 << number ) ? "Added" : "Removed", ris_flags[number] );
	return;

    case MEDIT_SPEC:
	number = atoi( arg );
 	switch( number )
	{
	case 0:
	    victim->spec_fun = NULL;
	    break;
	case 1:
	    victim->spec_fun = spec_lookup( "spec_breath_any" );
	    break;
	case 2:
	    victim->spec_fun = spec_lookup( "spec_breath_acid" );
	    break;
	case 3:
	    victim->spec_fun = spec_lookup( "spec_breath_fire" );
	    break;
	case 4:
	    victim->spec_fun = spec_lookup( "spec_breath_frost" );
	    break;
	case 5:
	    victim->spec_fun = spec_lookup( "spec_breath_gas" );
	    break;
	case 6:
	    victim->spec_fun = spec_lookup( "spec_breath_lightning" );
	    break;
	case 7:
	    victim->spec_fun = spec_lookup( "spec_cast_adept" );
	    break;
	case 8:
	    victim->spec_fun = spec_lookup( "spec_cast_cleric" );
	    break;
	case 9:
	    victim->spec_fun = spec_lookup( "spec_cast_mage" );
	    break;
	case 10:
	    victim->spec_fun = spec_lookup( "spec_cast_undead" );
	    break;
	case 11:
	    victim->spec_fun = spec_lookup( "spec_executioner" );
	    break;
	case 12:
	    victim->spec_fun = spec_lookup( "spec_fido" );
	    break;
	case 13:
	    victim->spec_fun = spec_lookup( "spec_guard" );
	    break;
	case 14:
	    victim->spec_fun = spec_lookup( "spec_janitor" );
	    break;
	case 15:
	    victim->spec_fun = spec_lookup( "spec_poison" );
	    break;
	case 16:
	    victim->spec_fun = spec_lookup( "spec_thief" );
	    break;
	}
	if ( IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE) )
	    victim->pIndexData->spec_fun = victim->spec_fun;
	olc_log( d, "Changes spec_func to %s", lookup_spec( victim->spec_fun ) );
	break;

    case MEDIT_DEITY:
	number = atoi(arg);
	if ( number < 0 || number > olc_top_deity+1 )
	{
	    send_to_char( "Invalid deity, try again: ", d->character );
	    return;
	}
	if ( number == 0 )
	{
	    STRFREE( victim->pcdata->deity_name );
	    victim->pcdata->deity_name	= STRALLOC( "" );
	    victim->pcdata->deity	= NULL;
	    break;
	}
	number -= 1;
	deity = get_deity( olc_deity_list[number] );
	if ( !deity )
	{
	    bug( "Unknown deity linked into olc_deity_list.", 0 );
	    break;
	}
	STRFREE( victim->pcdata->deity_name );
	victim->pcdata->deity_name = QUICKLINK( deity->name );
	victim->pcdata->deity = deity;
	olc_log( d, "Deity changed to %s", deity->name );
	break;

    case MEDIT_CLAN:
	if ( get_trust(d->character) < LEVEL_GOD )
	    break;
	number = atoi(arg);
	if ( number < 0 || number > olc_top_order+1 )
	{
	    send_to_char( "Invalid choice, try again: ", d->character );
	    return;
	}
	if ( number == 0 )
	{
	    if ( !IS_IMMORTAL(victim) )
	    {
		--victim->pcdata->clan->members;
		save_clan( victim->pcdata->clan );
	    }
	    STRFREE( victim->pcdata->clan_name );
	    victim->pcdata->clan_name		= STRALLOC( "" );
	    victim->pcdata->clan		= NULL;
	    break;
	}
	clan = get_clan( olc_clan_list[number-1] );
	if ( !clan )
	{
	    bug( "Non-existant clan linked into olc_clan_list.", 0 );
	    break;
	}
	if ( victim->pcdata->clan != NULL && !IS_IMMORTAL(victim) )
	{
	    --victim->pcdata->clan->members;
	    save_clan( victim->pcdata->clan );
	}
	STRFREE( victim->pcdata->clan_name );
	victim->pcdata->clan_name = QUICKLINK( clan->name );
	victim->pcdata->clan = clan;
	if ( !IS_IMMORTAL( victim ) )
	{
	    ++victim->pcdata->clan->members;
	    save_clan( victim->pcdata->clan );
	}
	olc_log( d, "Clan changed to %s", clan->name );
	break;

    case MEDIT_COUNCIL:
	if ( get_trust(d->character) < LEVEL_SUB_IMPLEM )
	    break;
	number = atoi(arg);
	if ( number < 0 || number > olc_top_council )
	{
	    send_to_char( "Invalid council, try again: ", d->character );
	    return;
	}
	if ( number == 0 )
	{
	    STRFREE( victim->pcdata->council_name );
	    victim->pcdata->council_name	= STRALLOC( "" );
	    victim->pcdata->council		= NULL;
	    break;
	}
	number -= 1; /* Offset cause 0 is first element but 0 is None, soo */
	council = get_council( olc_council_list[number] );
	if ( !council )
	{
	    bug( "Unknown council linked into olc_council_list.", 0 );
	    break;
	}
	STRFREE( victim->pcdata->council_name );
	victim->pcdata->council_name = QUICKLINK( council->name );
	victim->pcdata->council = council;
	olc_log( d, "Council changed to %s", council->name );
	break;

/*-------------------------------------------------------------------*/
    default:
	/*. We should never get here .*/
	bug( "OLC: medit_parse(): Reached default case!", 0 );
	cleanup_olc(d);
	return;;
    }
/*-------------------------------------------------------------------*/
/*. END OF CASE 
    If we get here, we have probably changed something, and now want to
    return to main menu.  Use OLC_CHANGE as a 'has changed' flag .*/

    OLC_CHANGE(d) = TRUE;
    medit_disp_menu(d);
}
/*. End of medit_parse() .*/

