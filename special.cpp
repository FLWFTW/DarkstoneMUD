/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider             |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops  |~'~.VxvxV.~'~*
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *			   "Special procedure" module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

#include "commands.h"


/*
 * The following special functions are available for mobiles.
 */
DECLARE_SPEC_FUN(	spec_breath_any		);
DECLARE_SPEC_FUN(	spec_breath_acid	);
DECLARE_SPEC_FUN(	spec_breath_fire	);
DECLARE_SPEC_FUN(	spec_breath_frost	);
DECLARE_SPEC_FUN(	spec_breath_gas		);
DECLARE_SPEC_FUN(	spec_breath_lightning	);
DECLARE_SPEC_FUN(	spec_cast_adept		);
DECLARE_SPEC_FUN(	spec_cast_cleric	);
DECLARE_SPEC_FUN(	spec_cast_mage		);
DECLARE_SPEC_FUN(	spec_cast_undead	);
DECLARE_SPEC_FUN(	spec_executioner	);
DECLARE_SPEC_FUN(	spec_fido		);
DECLARE_SPEC_FUN(	spec_guard		);
DECLARE_SPEC_FUN(	spec_janitor		);
DECLARE_SPEC_FUN(	spec_mayor		);
DECLARE_SPEC_FUN(	spec_poison		);
DECLARE_SPEC_FUN(	spec_thief		);
DECLARE_SPEC_FUN(   spec_wander_gm  );
DECLARE_SPEC_FUN(   spec_butcher );



/*
 * Given a name, return the appropriate spec fun.
 */
SPEC_FUN *spec_lookup( const char *name )
{
    if ( !str_cmp( name, "spec_breath_any"	  ) ) return spec_breath_any;
    if ( !str_cmp( name, "spec_breath_acid"	  ) ) return spec_breath_acid;
    if ( !str_cmp( name, "spec_breath_fire"	  ) ) return spec_breath_fire;
    if ( !str_cmp( name, "spec_breath_frost"	  ) ) return spec_breath_frost;
    if ( !str_cmp( name, "spec_breath_gas"	  ) ) return spec_breath_gas;
    if ( !str_cmp( name, "spec_breath_lightning"  ) ) return
							spec_breath_lightning;
    if ( !str_cmp( name, "spec_cast_adept"	  ) ) return spec_cast_adept;
    if ( !str_cmp( name, "spec_cast_cleric"	  ) ) return spec_cast_cleric;
    if ( !str_cmp( name, "spec_cast_mage"	  ) ) return spec_cast_mage;
    if ( !str_cmp( name, "spec_cast_undead"	  ) ) return spec_cast_undead;
    if ( !str_cmp( name, "spec_executioner"	  ) ) return spec_executioner;
    if ( !str_cmp( name, "spec_fido"		  ) ) return spec_fido;
    if ( !str_cmp( name, "spec_guard"		  ) ) return spec_guard;
    if ( !str_cmp( name, "spec_janitor"		  ) ) return spec_janitor;
    if ( !str_cmp( name, "spec_mayor"		  ) ) return spec_mayor;
    if ( !str_cmp( name, "spec_poison"		  ) ) return spec_poison;
    if ( !str_cmp( name, "spec_thief"		  ) ) return spec_thief;
    if ( !str_cmp( name, "spec_wander_gm"     ) ) return spec_wander_gm;
    if ( !str_cmp( name, "spec_butcher"       ) ) return spec_butcher;
    return 0;
}

/*
 * Given a pointer, return the appropriate spec fun text.
 */
const char *lookup_spec( SPEC_FUN *special )
{
    if ( special == spec_breath_any	)	return "spec_breath_any";
    
    if ( special == spec_breath_acid	)	return "spec_breath_acid";
    if ( special == spec_breath_fire	) 	return "spec_breath_fire";
    if ( special == spec_breath_frost	) 	return "spec_breath_frost";
    if ( special == spec_breath_gas	) 	return "spec_breath_gas";
    if ( special == spec_breath_lightning )	return "spec_breath_lightning";
    if ( special == spec_cast_adept	)	return "spec_cast_adept";
    if ( special == spec_cast_cleric	)	return "spec_cast_cleric";
    if ( special == spec_cast_mage	)	return "spec_cast_mage";
    if ( special == spec_cast_undead	)	return "spec_cast_undead";
    if ( special == spec_executioner	)	return "spec_executioner";
    if ( special == spec_fido		)	return "spec_fido";
    if ( special == spec_guard		)	return "spec_guard";
    if ( special == spec_janitor	)	return "spec_janitor";
    if ( special == spec_mayor		)	return "spec_mayor";
    if ( special == spec_poison		)	return "spec_poison";
    if ( special == spec_thief		)	return "spec_thief";
    if ( special == spec_wander_gm )    return "spec_wander_gm";
    if ( special == spec_butcher    )   return "spec_butcher";
    return "";
}


/* if a spell casting mob is hating someone... try and summon them */
void summon_if_hating( CHAR_DATA *ch )
{
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];
	char name[MAX_INPUT_LENGTH];
	bool found = FALSE;

	if ( ch->IsFighting() || ch->fearing
			||  !ch->hating || IS_SET( ch->GetInRoom()->room_flags, ROOM_SAFE ) )
		return;

	/* if player is close enough to hunt... don't summon */
	if ( ch->hunting )
		return;

	one_argument( ch->hating->name_.c_str(), name );

	/* make sure the char exists - works even if player quits */
	for (victim = first_char;
			victim;
			victim = victim->next)
	{
		if ( ch->hating->name_.ciEqual( victim->getName() ) )
		{
			found = TRUE;
			break;
		}
	}

	if ( !found )
		return;
	if ( ch->GetInRoom() == victim->GetInRoom() )
		return;
	if ( !IS_NPC( victim ) )
		sprintf( buf, "summon 0.%s", name );
	else
		sprintf( buf, "summon %s", name );
	do_cast( ch, buf );
	return;
}

/*
 * Core procedure for dragons.
 */
bool dragon( CHAR_DATA *ch, const char *spell_name )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    int sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    for ( victim = ch->GetInRoom()->first_person; victim; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->GetVictim() == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( !victim )
	return FALSE;

    if ( ( sn = skill_lookup( spell_name ) ) < 0 )
	return FALSE;
    (*skill_table[sn]->spell_fun) ( sn, ch->level, ch, victim );
    return TRUE;
}



/*
 * Special procedures for mobiles.
 */
bool spec_breath_any( CHAR_DATA *ch )
{
    if ( ch->position != POS_FIGHTING )
	return FALSE;

    switch ( number_bits( 3 ) )
    {
    case 0: return spec_breath_fire		( ch );
    case 1:
    case 2: return spec_breath_lightning	( ch );
    case 3: return spec_breath_gas		( ch );
    case 4: return spec_breath_acid		( ch );
    case 5:
    case 6:
    case 7: return spec_breath_frost		( ch );
    }

    return FALSE;
}



bool spec_breath_acid( CHAR_DATA *ch )
{
    return dragon( ch, "acid breath" );
}



bool spec_breath_fire( CHAR_DATA *ch )
{
    return dragon( ch, "fire breath" );
}



bool spec_breath_frost( CHAR_DATA *ch )
{
    return dragon( ch, "frost breath" );
}



bool spec_breath_gas( CHAR_DATA *ch )
{
    int sn;

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    if ( ( sn = skill_lookup( "gas breath" ) ) < 0 )
	return FALSE;
    (*skill_table[sn]->spell_fun) ( sn, ch->level, ch, NULL );
    return TRUE;
}



bool spec_breath_lightning( CHAR_DATA *ch )
{
    return dragon( ch, "lightning breath" );
}



bool spec_cast_adept( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( victim = ch->GetInRoom()->first_person; victim; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim != ch && can_see( ch, victim ) && number_bits( 1 ) == 0 )
	    break;
    }

    if ( !victim )
	return FALSE;

    switch ( number_bits( 3 ) )
    {
    case 0:
    act( AT_MAGIC, "$n utters the word 'ciroht'.", ch, NULL, NULL, TO_ROOM );
	spell_smaug( skill_lookup( "armor" ), ch->level, ch, victim );
	return TRUE;

    case 1:
    act( AT_MAGIC, "$n utters the word 'sunimod'.", ch, NULL, NULL, TO_ROOM );
	spell_smaug( skill_lookup( "bless" ), ch->level, ch, victim );
	return TRUE;

    case 2:
    act( AT_MAGIC, "$n utters the word 'suah'.", ch, NULL, NULL, TO_ROOM );
	spell_cure_blindness( skill_lookup( "cure blindness" ),
	    ch->level, ch, victim );
	return TRUE;

    case 3:
    act( AT_MAGIC, "$n utters the word 'nran'.", ch, NULL, NULL, TO_ROOM );
	spell_smaug( skill_lookup( "cure light" ),
	    ch->level, ch, victim );
	return TRUE;

    case 4:
    act( AT_MAGIC, "$n utters the word 'nyrcs'.", ch, NULL, NULL, TO_ROOM );
	spell_cure_poison( skill_lookup( "cure poison" ),
	    ch->level, ch, victim );
	return TRUE;

    case 5:
    act( AT_MAGIC, "$n utters the word 'gartla'.", ch, NULL, NULL, TO_ROOM );
	spell_smaug( skill_lookup( "refresh" ), ch->level, ch, victim );
	return TRUE;

    }

    return FALSE;
}



bool spec_cast_cleric( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    const char *spell;
    int sn;

    summon_if_hating( ch );

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    for ( victim = ch->GetInRoom()->first_person; victim; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->GetVictim() == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( !victim || victim == ch )
	return FALSE;

    for ( ;; )
    {
	int min_level;

	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "blindness";      break;
	case  1: min_level =  3; spell = "cause serious";  break;
	case  2: min_level =  7; spell = "earthquake";     break;
	case  3: min_level =  9; spell = "cause critical"; break;
	case  4: min_level = 10; spell = "dispel evil";    break;
	case  5: min_level = 12; spell = "curse";          break;
	case  7: min_level = 13; spell = "flamestrike";    break;
	case  8:
	case  9:
	case 10: min_level = 15; spell = "harm";           break;
	default: min_level = 16; spell = "dispel magic";   break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn]->spell_fun) ( sn, ch->level, ch, victim );
    return TRUE;
}



bool spec_cast_mage( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    const char *spell;
    int sn;

    summon_if_hating( ch );

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    for ( victim = ch->GetInRoom()->first_person; victim; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->GetVictim() && number_bits( 2 ) == 0 )
	    break;
    }

    if ( !victim || victim == ch )
	return FALSE;

    for ( ;; )
    {
	int min_level;

	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "blindness";      break;
	case  1: min_level =  3; spell = "chill touch";    break;
	case  2: min_level =  7; spell = "weaken";         break;
/* Commented out by Narn, Nov 28/95 at the request of Selic
	case  3: min_level =  8; spell = "teleport";       break; */
	case  4: min_level = 11; spell = "colour spray";   break;
	case  6: min_level = 13; spell = "energy drain";   break;
	case  7:
	case  8:
	case  9: min_level = 15; spell = "fireball";       break;
	default: min_level = 20; spell = "acid blast";     break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn]->spell_fun) ( sn, ch->level, ch, victim );
    return TRUE;
}



bool spec_cast_undead( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    const char *spell;
    int sn;

    summon_if_hating( ch );

    if ( ch->position != POS_FIGHTING )
	return FALSE;

    for ( victim = ch->GetInRoom()->first_person; victim; victim = v_next )
    {
	v_next = victim->next_in_room;
	if ( victim->GetVictim() == ch && number_bits( 2 ) == 0 )
	    break;
    }

    if ( !victim || victim == ch )
	return FALSE;

    for ( ;; )
    {
	int min_level;

	switch ( number_bits( 4 ) )
	{
	case  0: min_level =  0; spell = "curse";          break;
	case  1: min_level =  3; spell = "weaken";         break;
	case  2: min_level =  6; spell = "chill touch";    break;
	case  3: min_level =  9; spell = "blindness";      break;
	case  4: min_level = 12; spell = "poison";         break;
	case  5: min_level = 15; spell = "energy drain";   break;
	case  6: min_level = 18; spell = "harm";           break;
/* Commented out by Narn, Nov 28/95
	case  7: min_level = 21; spell = "teleport";       break; */
	default: min_level = 35; spell = "gate";           break;
	}

	if ( ch->level >= min_level )
	    break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
	return FALSE;
    (*skill_table[sn]->spell_fun) ( sn, ch->level, ch, victim );
    return TRUE;
}



bool spec_executioner( CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];
	MOB_INDEX_DATA *cityguard;
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	char crime[] = "COWARD";

	if ( !IS_AWAKE(ch) || ch->IsFighting() )
		return FALSE;

	for ( victim = ch->GetInRoom()->first_person; victim; victim = v_next )
	{
		v_next = victim->next_in_room;
	}

	if ( !victim )
		return FALSE;

	if ( IS_SET( ch->GetInRoom()->room_flags, ROOM_SAFE ) )
	{
		sprintf( buf, "%s is a %s!",
				victim->getShort().c_str(), crime );
		do_yell( ch, buf );
		return TRUE;
	}

	sprintf( buf, "%s is a %s!  PROTECT THE INNOCENT!  MORE BLOOOOD!!!",
			victim->getName().c_str(), crime );
	do_shout( ch, buf );
	multi_hit( ch, victim, TYPE_UNDEFINED );
	if ( char_died(ch) )
		return TRUE;

	/* Added log in case of missing cityguard -- Tri */

	cityguard = get_mob_index( MOB_VNUM_CITYGUARD );

	if ( !cityguard )
	{
		sprintf( buf, "Missing Cityguard - Vnum:[%s]", vnum_to_dotted(MOB_VNUM_CITYGUARD) );
		bug( buf, 0 );
		return TRUE;
	}

	char_to_room( create_mobile( cityguard ), ch->GetInRoom() );
	char_to_room( create_mobile( cityguard ), ch->GetInRoom() );
	return TRUE;
}



bool spec_fido( CHAR_DATA *ch )
{
    OBJ_DATA *corpse;
    OBJ_DATA *c_next;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( corpse = ch->GetInRoom()->first_content; corpse; corpse = c_next )
    {
	c_next = corpse->next_content;
	if ( corpse->item_type != ITEM_CORPSE_NPC )
	    continue;

    act( AT_ACTION, "$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM );
	for ( obj = corpse->first_content; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    obj_from_obj( obj );
	    obj_to_room( obj, ch->GetInRoom() );
	}
	extract_obj( corpse, TRUE );
	return TRUE;
    }

    return FALSE;
}



bool spec_guard( CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	CHAR_DATA *ech;
	const char *crime;
	int max_evil;

	if ( !IS_AWAKE(ch) || ch->IsFighting() )
		return FALSE;

	max_evil = 300;
	ech      = NULL;
	crime    = "COWARD";

	for ( victim = ch->GetInRoom()->first_person; victim; victim = v_next )
	{
		v_next = victim->next_in_room;

		if ( victim->IsFighting()
				&&   victim->GetVictim() != ch
				&&   victim->alignment < max_evil )
		{
			max_evil = victim->alignment;
			ech      = victim;
		}
	}

	if ( victim && IS_SET( ch->GetInRoom()->room_flags, ROOM_SAFE ) )
	{
		sprintf( buf, "%s is a %s!",
				victim->getName().c_str(), crime );
		do_yell( ch, buf );
		return TRUE;
	}

	if ( victim )
	{
		sprintf( buf, "%s is a %s!  PROTECT THE INNOCENT!!  BANZAI!!",
				victim->getName().c_str(), crime );
		do_shout( ch, buf );
		multi_hit( ch, victim, TYPE_UNDEFINED );
		return TRUE;
	}

	if ( ech )
	{
		act( AT_YELL, "$n screams 'PROTECT THE INNOCENT!!  BANZAI!!",
				ch, NULL, NULL, TO_ROOM );
		multi_hit( ch, ech, TYPE_UNDEFINED );
		return TRUE;
	}

	return FALSE;
}



bool spec_janitor( CHAR_DATA *ch )
{
    OBJ_DATA *trash;
    OBJ_DATA *trash_next;

    if ( !IS_AWAKE(ch) )
	return FALSE;

    for ( trash = ch->GetInRoom()->first_content; trash; trash = trash_next )
    {
	trash_next = trash->next_content;
	if ( !IS_SET( trash->wear_flags, ITEM_TAKE )
        ||    IS_OBJ_STAT( trash, ITEM_BURIED ) )
	    continue;
	if ( trash->item_type == ITEM_DRINK_CON
	||   trash->item_type == ITEM_TRASH
	||   trash->cost < 10
	||  (trash->pIndexData->vnum == OBJ_VNUM_SHOPPING_BAG
	&&  !trash->first_content) )
	{
	    act( AT_ACTION, "$n picks up some trash.", ch, NULL, NULL, TO_ROOM );
	    obj_from_room( trash );
	    obj_to_char( trash, ch );
	    return TRUE;
	}
    }

    return FALSE;
}



bool spec_mayor( CHAR_DATA *ch )
{
	static const char open_path[] =
		"W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

	static const char close_path[] =
		"W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

	static const char *path;
	static int pos;
	static bool move;

	if ( !move )
	{
		if ( time_info.hour ==  6 )
		{
			path = open_path;
			move = TRUE;
			pos  = 0;
		}

		if ( time_info.hour == 20 )
		{
			path = close_path;
			move = TRUE;
			pos  = 0;
		}
	}

	if ( ch->IsFighting() )
		return spec_cast_cleric( ch );
	if ( !move || ch->position < POS_SLEEPING )
		return FALSE;

	switch ( path[pos] )
	{
		case '0':
		case '1':
		case '2':
		case '3':
			move_char( ch, get_exit( ch->GetInRoom(), path[pos] - '0' ), 0 );
			break;

		case 'W':
			ch->position = POS_STANDING;
			act( AT_ACTION, "$n awakens and groans loudly.", ch, NULL, NULL, TO_ROOM );
			break;

		case 'S':
			ch->position = POS_SLEEPING;
			act( AT_ACTION, "$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM );
			break;

		case 'a':
			act( AT_SAY, "$n says 'Hello Honey!'", ch, NULL, NULL, TO_ROOM );
			break;

		case 'b':
			act( AT_SAY, "$n says 'What a view!  I must do something about that dump!'",
					ch, NULL, NULL, TO_ROOM );
			break;

		case 'c':
			act( AT_SAY, "$n says 'Vandals!  Youngsters have no respect for anything!'",
					ch, NULL, NULL, TO_ROOM );
			break;

		case 'd':
			act( AT_SAY, "$n says 'Good day, citizens!'", ch, NULL, NULL, TO_ROOM );
			break;

		case 'e':
			act( AT_SAY, "$n says 'I hereby declare the town of Darkhaven open!'",
					ch, NULL, NULL, TO_ROOM );
			break;

		case 'E':
			act( AT_SAY, "$n says 'I hereby declare the town of Darkhaven closed!'",
					ch, NULL, NULL, TO_ROOM );
			break;

		case 'O':
			do_unlock( ch, "gate" );
			do_open( ch, "gate" );
			break;

		case 'C':
			do_close( ch, "gate" );
			do_lock( ch, "gate" );
			break;

		case '.' :
			move = FALSE;
			break;
	}

	pos++;
	return FALSE;
}



bool spec_poison( CHAR_DATA *ch )
{
    CHAR_DATA *victim;

    if ( ch->position != POS_FIGHTING
    || ( victim = ch->GetVictim() ) == NULL
    ||   number_percent( ) > 2 * ch->level )
	return FALSE;

    act( AT_HIT, "You bite $N!",  ch, NULL, victim, TO_CHAR    );
    act( AT_ACTION, "$n bites $N!",  ch, NULL, victim, TO_NOTVICT );
    act( AT_POISON, "$n bites you!", ch, NULL, victim, TO_VICT    );
    spell_poison( gsn_poison, ch->level, ch, victim );
    return TRUE;
}



bool spec_thief( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    int gold, maxgold;

    if ( ch->position != POS_STANDING )
	return FALSE;

    for ( victim = ch->GetInRoom()->first_person; victim; victim = v_next )
    {
	v_next = victim->next_in_room;

	if ( IS_NPC(victim)
	||   victim->level >= LEVEL_IMMORTAL
	||   number_bits( 2 ) != 0
	||   !can_see( ch, victim ) )	/* Thx Glop */
	    continue;

	if ( IS_AWAKE(victim) && number_range( 0, ch->level ) == 0 )
	{
	    act( AT_ACTION, "You discover $n's hands in your sack of gold!",
		ch, NULL, victim, TO_VICT );
	    act( AT_ACTION, "$N discovers $n's hands in $S sack of gold!",
		ch, NULL, victim, TO_NOTVICT );
	    return TRUE;
	}
	else
	{
	    maxgold = ch->level * ch->level * 1000;
	    gold = victim->gold
	    	 * number_range( 1, URANGE(2, ch->level/4, 10) ) / 100;
	    ch->gold     += 9 * gold / 10;
	    victim->gold -= gold;
	    if ( ch->gold > maxgold )
	    {
		boost_economy( ch->GetInRoom()->area, ch->gold - maxgold/2 );
		ch->gold = maxgold/2;
	    }
	    return TRUE;
	}
    }

    return FALSE;
}

bool spec_wander_gm(CHAR_DATA *ch)
{
    CHAR_DATA *vch;
    TRAIN_DATA *train = NULL;
    bool bwander = TRUE;
    bool bpc = FALSE;
    
    if ( ch->position != POS_STANDING && ch->position != POS_MOUNTED )
        return FALSE;

    if ( IS_NPC(ch) && ch->pIndexData->train ) {
        train = ch->pIndexData->train;
    }
    
    if ( IS_SET(ch->act, ACT_PROTOTYPE) ) {
        return FALSE;
    }
    
    for ( vch = ch->GetInRoom()->first_person; vch; vch = vch->next_in_room ) 
    {
        if ( !IS_NPC(vch) )
        {
            bpc = TRUE;

            if (train && (train->_class == -1 || ch->Class == train->_class)) {
                bwander = FALSE;
                break;
            }
        }
    }
    
    if ( !bwander )
        return TRUE;

    if ( !ch->GetInRoom()->first_exit )
    {
        ROOM_INDEX_DATA *pRoomIndex;
        AREA_DATA *pArea;

SPEC_WANDER_GM_GOTO:
NEXTROOM:
        for ( ; ; )
        {
            int vnum = number_range(0, 1048575999);
            pRoomIndex = get_room_index(vnum);

            if ( !pRoomIndex ) {
                continue;
            }
    
            for ( pArea = first_build; pArea; pArea = pArea->next ) {
                if ( !str_cmp(pRoomIndex->area->filename, pArea->filename) ) {
                    goto NEXTROOM;
                }
                if ( !str_cmp(pRoomIndex->area->filename, "limbo.are")
                  || !str_cmp(pRoomIndex->area->filename, "gods.are")
                  || !str_cmp(pRoomIndex->area->filename, "circus.are")
                  || !str_cmp(pRoomIndex->area->filename, "void.are")
                  || !str_cmp(pRoomIndex->area->filename, "houses.are")
                  ) {
                    goto NEXTROOM;
                }
            }

            break;
        }

        act(AT_MAGIC, "$n slowly fades out of view.", ch, NULL, NULL, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, pRoomIndex);
        act(AT_MAGIC, "$n slowly fades into view.", ch, NULL, NULL, TO_ROOM);
        do_look(ch, "auto");

        return TRUE;
    } else {
        ExitData *pexit;
        int door;
        int count;
        int done[10] = {0,0,0,0,0,0,0,0,0,0};
       
        while ( count < 8 ) {
            door = number_bits(5);

            if ( door > 9 ) {
                continue;
            }

            if ( done[door]++ ) {
                continue;
            }

            ++count;
            
            if ( !(pexit = get_exit(ch->GetInRoom(), door) ) ) {
                continue;
            }
            
            if ( !pexit->to_room ) {
                continue;
            }
          
            if ( IS_SET(pexit->exit_info, EX_CLOSED) ) {
                continue;        
            }
           
            if ( IS_SET(pexit->exit_info, EX_WINDOW) ) {
                continue;
            }
            
            move_char(ch, pexit, 0);

            return TRUE;
        }
        
        if ( !bpc ) {
            /* If we get this far, just teleport.  I use goto, it's easier */
            goto SPEC_WANDER_GM_GOTO;
        }
    }
    
    return TRUE;
}

bool spec_butcher( CHAR_DATA *ch )
{
    OBJ_DATA *corpse;
    OBJ_DATA *c_next;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *slice;
    float a, b, c = 0.0;
 
    MOB_INDEX_DATA *pMobIndex;
    char name[MAX_STRING_LENGTH]; 
	char objstr[MAX_STRING_LENGTH];
#ifdef DEBUG_SPECIAL
	char buf[MAX_STRING_LENGTH];
#endif 

	bool clean_case;
	bool mark_down;
   
	int  meattype;
	int  spamcount = 0; /* Just so that heavy stuff doesn't make shitload of meat */
	
	const char * name_descs[] =
   { 
     "meat ground %s burger", 
     "meat %s sausage",
     "meat %s roast",
     "meat %s pie",
     "meat %s filet"
   };
	
	const char * short_descs[] =
   { 
     "1 lb. of ground %s", 
     "a few links of %s sausage",
     "a %s rump roast",
     "a %s pie",
     "a tender filet of %s"
   };
	
	const char * long_descs[] =
   { 
     "A package of ground %s burger lies here.", 
     "Some sausage links stuffed with %s lies here.",
     "A %s rump roast lies here.",
     "A frozen meat pie made from %s lies here.",
     "A tender filet cut from %s lies here, waiting to be grilled."
   };
   
   const char * eat_descs[] =
   { 
     "$n pop$q a big wad of ground %s burger in $s mouth and swallows it down RAW!.", 
     "$n shove$q a %s sausage link in $s mouth and gobble$q it up!.",
     "$n take$q a big bite out of roasted %s!.",
     "$n gobble$q down a %s meat pie!.",
     "$n slide$q a filet of %s down $s throat!."
   };
		
#ifdef DEBUG_SPECIAL
		    sprintf( buf, "special(1)  %s", ch->name);
     		log_string( buf );
#endif 

    if ( !IS_AWAKE(ch) )
		return FALSE;
	
	if ( get_obj_index(OBJ_VNUM_SLICE) == NULL )
	{
#ifdef DEBUG_SPECIAL
		    sprintf( buf, "special(2)  %s", ch->name);
     		log_string( buf );
#endif 

		bug("Vnum 24 not found for spec_butcher!", 0);
	    return FALSE;
	}
	
	/* check through inventory and look for items 1/2 decayed.  Change short description
	to desc [REDUCED FOR QUICK SALE] 
	just use obj to save on memory....*/
	
	clean_case = FALSE;
	mark_down = FALSE;
	
	for ( obj = ch->first_carrying; obj; obj = obj_next )
   {
		obj_next = obj->next_content;
		
		one_argument( obj->name_.c_str(), name );   
#ifdef DEBUG_SPECIAL
		    sprintf( buf, "special(3)  %s", ch->name);
     		log_string( buf );
#endif 
		a = (float) obj->timer;
		b = (float) obj->value[1];
		if (obj->timer)
			c = ((a * 10) / b);
			
		if (
		    ( (obj->timer > 0) && (obj->value[1] > 0) )
		    &&
		    ( c < 6)
		    &&
		    (strcmp( name, "old"))
		    //&&
		    //( !IS_OBJ_STAT( obj, ITEM_INVENTORY ))
		    
		   )
		{
#ifdef DEBUG_SPECIAL
		    sprintf( buf, "special(4)  %s", ch->name);
     		log_string( buf );
#endif 


		    sprintf( objstr, "%s [REDUCED FOR QUICK SALE]", capitalize(obj->shortDesc_.c_str()));
			obj->shortDesc_ = objstr;
			
			sprintf (objstr, "old %s",obj->name_.c_str());
			obj->name_ = objstr;
			mark_down = TRUE;
			
			/* Chop price in 1/2*/
			
			a = obj->cost;
			obj->cost = (int) a/2;
			
		}
		
#ifdef DEBUG_SPECIAL
		    sprintf( buf, "special(4.5)  %s", ch->name);
     		log_string( buf );
#endif 
		
		
		/* food  5 and 4 are still somewhat ok to eat....
		when it gets down to 2, lets just remove it.....
	
		*/
		a = (float) obj->timer;
		b = (float) obj->value[1];
		if (obj->timer)
			c= ((a * 10)/ b);
		
		if (
		(c < 4) 
		&& 
		(obj->timer > 0) 
		)
		{
#ifdef DEBUG_SPECIAL
		    sprintf( buf, "special(5)  %s", ch->name);
     		log_string( buf );
#endif 

			clean_case = TRUE;			
			 extract_obj( obj, TRUE );
		}
		
		
		
	}
	
	if (mark_down)
	{
#ifdef DEBUG_SPECIAL
		    sprintf( buf, "special(6)  %s", ch->name);
     		log_string( buf );
#endif 

			act( AT_ACTION, "$n marks down some of the meat for a special sale.", ch, NULL, NULL, TO_ROOM );
			act( AT_ACTION, "You mark down some of the meat for a special sale.", ch, NULL, NULL, TO_CHAR );

	}
	
	if (clean_case)
	{
#ifdef DEBUG_SPECIAL
		    sprintf( buf, "special(7)  %s", ch->name);
     		log_string( buf );
#endif 

		act( AT_ACTION, "$n removes some old meat from the display case.", ch, NULL, NULL, TO_ROOM );
		act( AT_ACTION, "You remove some meat from the case.", ch, NULL, NULL, TO_CHAR );

	}

	

    for ( corpse = ch->GetInRoom()->first_content; corpse; corpse = c_next )
    {
		c_next = corpse->next_content;
		if (
		(corpse->item_type != ITEM_CORPSE_NPC) 
		&& 
		(corpse->item_type != ITEM_CORPSE_PC) 
		)
		{
		#ifdef DEBUG_SPECIAL
		    sprintf( buf, "special(8)  %s", ch->name);
     		log_string( buf );
#endif 

			continue;
		}
			
		if (
					((corpse->timer <= 12)
					&&
					(corpse->item_type == ITEM_CORPSE_PC))
				||
					((corpse->timer <= 2)
					&&
					(corpse->item_type == ITEM_CORPSE_NPC))
				||
					((corpse->item_type == ITEM_CORPSE_NPC 
					&& 
					corpse->cost == -5))
			)
	   {
			
#ifdef DEBUG_SPECIAL
		    sprintf( buf, "special(9)  %s", ch->name);
     		log_string( buf );
#endif 
			
			act( AT_ACTION, "$n mumbles something about the corpse of $T being rotten.",
					ch, NULL, strip_the(corpse->shortDesc_.c_str()), TO_ROOM );
			act( AT_ACTION, "You mumble something about the corpse of $T being rotten.",
					ch, NULL, strip_the(corpse->shortDesc_.c_str()), TO_CHAR );
			
			for ( obj = corpse->first_content; obj; obj = obj_next )
			{
				obj_next = obj->next_content;
				obj_from_obj( obj );
		    	obj_to_room( obj, ch->GetInRoom() );
#ifdef DEBUG_SPECIAL
		    sprintf( buf, "special(10)  %s", ch->name);
     		log_string( buf );
#endif 
		    	
		    	
			}
#ifdef DEBUG_SPECIAL
		    sprintf( buf, "special(11)  %s", ch->name);
     		log_string( buf );
#endif 

			 extract_obj( corpse, TRUE );
			 return TRUE;
		
		}
			
		if (corpse->timer < 1)
			continue;
			
		if (
		( corpse->weight > (int) (get_obj_weight(corpse)/3) ) 
		&& 
		(corpse->weight > 1)
		)
		{
#ifdef DEBUG_SPECIAL
		    sprintf( buf, "special(12)  %s", ch->name);
     		log_string( buf );
#endif 

	    	act( AT_ACTION, "$n butchers $P.", ch, NULL, corpse, TO_ROOM );
	    	act( AT_ACTION, "You butcher $P into itty bitty pieces.", ch, NULL, corpse, TO_CHAR );

	    	    	

	    }
	    	
	    	    
	    while (
	    ( corpse->weight > (int) (get_obj_weight(corpse)/3) ) 
	    && 
	    (corpse->weight > 1)
	    )
	    {

#ifdef DEBUG_SPECIAL
		    sprintf( buf, "special(13)  %s", ch->name);
     		log_string( buf );
#endif 

	    	slice = create_object( get_obj_index(OBJ_VNUM_SLICE), 0 );		    		    	
	    	
	    	spamcount++;
	    	
	  
			/*Work with only the name on the corpse" 
			since all corpses start with corpse, strip that off and leave the name */
  			
  			meattype = number_range(0,4);
  
  			sprintf( objstr, name_descs[ meattype ], 
                          capitalize( strip_a(strip_the(corpse->name_.c_str())) ) );
  
  			slice->name_ = objstr;
  		
			sprintf( objstr, short_descs[ meattype ], 
                          capitalize( strip_a(corpse->shortDesc_.c_str()) ) );
                          
			slice->shortDesc_ = objstr;
		
			sprintf( objstr, long_descs[ meattype ], 
                          capitalize( strip_a(corpse->shortDesc_.c_str()) ) );
                          
			slice->longDesc_ = objstr;
			
			sprintf( objstr, eat_descs[ meattype ], 
                           capitalize( strip_a(corpse->shortDesc_.c_str()) ) );
                          
			slice->actionDesc_ = objstr;
			
			if (corpse->weight > 1)
  				corpse->weight -= 10;	
  				
  			slice->weight = 1;
  			
  			if (spamcount > 20)
  				corpse->weight = 1;
  				
  			slice->cost = 20; /* Base cost for slices made from PC's */
  				
  			if (corpse->item_type == ITEM_CORPSE_NPC)
  			{
  				slice->timer = (int) ( corpse->timer * 40)/6;
  				if ( (pMobIndex = get_mob_index((sh_int) abs(corpse->cost) )) != NULL )
  					slice->cost = pMobIndex->level;
  				
  			}
  			else
  				slice->timer = corpse->timer;
  			
  			slice->value[1] = 40;
  			
  			
  			/*obj_to_room(slice, ch->GetInRoom());  */
  			obj_to_char(slice, ch);  	
	    	
	    }
		   
		act( AT_ACTION, "$n places some meat in the display case.", ch, name, NULL, TO_ROOM );
		act( AT_ACTION, "You place some meat in the display case.", ch, name, NULL, TO_CHAR );
	
		
		for ( obj = corpse->first_content; obj; obj = obj_next )
		{
			obj_next = obj->next_content;
			obj_from_obj( obj );
	    	obj_to_room( obj, ch->GetInRoom() );
	    	
	    	
		}
		
		 act( AT_ACTION, "$n discards a stripped carcass of $t.",
				 ch, strip_the(corpse->shortDesc_.c_str()), NULL, TO_ROOM );
		 act( AT_ACTION, "You discard a stripped carcass of $t.",
				 ch, strip_the(corpse->shortDesc_.c_str()), NULL, TO_CHAR );
		 extract_obj( corpse, TRUE );
		 return TRUE;	
	    
	}
	return FALSE;
	
}
