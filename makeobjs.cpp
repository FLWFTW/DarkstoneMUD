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
 *			Specific object creation module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


/*
 * Make a fire.
 */
void make_fire(ROOM_INDEX_DATA *in_room, sh_int timer)
{
    OBJ_DATA *fire;

    fire = create_object( get_obj_index( OBJ_VNUM_FIRE ), 0 );
    fire->timer = number_fuzzy(timer);
    obj_to_room( fire, in_room );
    return;
}

/*
 * Make a trap.
 */
OBJ_DATA *make_trap(int v0, int v1, int v2, int v3)
{
    OBJ_DATA *trap;

    trap = create_object( get_obj_index( OBJ_VNUM_TRAP ), 0 );
    trap->timer = 0;
    trap->value[0] = v0;
    trap->value[1] = v1;
    trap->value[2] = v2;
    trap->value[3] = v3;
    return trap;
}


/*
 * Turn an object into scraps.		-Thoric
 */
void make_scraps( OBJ_DATA *obj )
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA  *scraps, *tmpobj;
	CHAR_DATA *ch = NULL;

	// Ksilyan:
	if ( IS_SET(obj->extra_flags_2, ITEM_NEVER_BREAKS) )
		return; // don't scrap an unscrappable item!

	separate_obj( obj );
	scraps	= create_object( get_obj_index( OBJ_VNUM_SCRAPS ), 0 );
	scraps->timer = number_range( 5, 15 );

	/* don't make scraps of scraps of scraps of ... */
	if ( obj->pIndexData->vnum == OBJ_VNUM_SCRAPS )
	{
		scraps->shortDesc_ = "some debris";
		scraps->longDesc_ = "Bits of debris lie on the ground here.";
	}
	else
	{
		sprintf( buf, scraps->shortDesc_.c_str(), obj->shortDesc_.c_str() );
		scraps->shortDesc_ = buf;
		sprintf( buf, scraps->longDesc_.c_str(), obj->shortDesc_.c_str() );
		scraps->longDesc_ = buf;
	}

	if ( obj->GetCarriedBy() )
	{
		act( AT_OBJECT, "$p falls to the ground in scraps!",
				obj->GetCarriedBy(), obj, NULL, TO_CHAR );
		if ( obj == get_eq_char( obj->GetCarriedBy(), WEAR_WIELD )
				&&  (tmpobj = get_eq_char( obj->GetCarriedBy(), WEAR_DUAL_WIELD)) != NULL )
			tmpobj->wear_loc = WEAR_WIELD;

		obj_to_room( scraps, obj->GetCarriedBy()->GetInRoom());
	}
	else
		if ( obj->GetInRoom() )
		{
			if ( (ch = obj->GetInRoom()->first_person ) != NULL )
			{
				act( AT_OBJECT, "$p is reduced to little more than scraps.",
						ch, obj, NULL, TO_ROOM );
				act( AT_OBJECT, "$p is reduced to little more than scraps.",
						ch, obj, NULL, TO_CHAR );
			}
			obj_to_room( scraps, obj->GetInRoom());
		}
	if ( (obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_QUIVER
				||   obj->item_type == ITEM_CORPSE_PC) && obj->first_content )
	{
		if ( ch && ch->GetInRoom() )
		{
			act( AT_OBJECT, "The contents of $p fall to the ground.",
					ch, obj, NULL, TO_ROOM );
			act( AT_OBJECT, "The contents of $p fall to the ground.",
					ch, obj, NULL, TO_CHAR );
		}
		if ( obj->GetCarriedBy() )
			empty_obj( obj, NULL, obj->GetCarriedBy()->GetInRoom() );
		else
			if ( obj->GetInRoom() )
				empty_obj( obj, NULL, obj->GetInRoom() );
			else
				if ( obj->GetInObj() )
					empty_obj( obj, obj->GetInObj(), NULL );
	}
	extract_obj( obj, TRUE );
}


/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch, CHAR_DATA *killer )
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *corpse;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	const char *name;
	
	if ( IS_NPC(ch) )
	{
		name		= ch->getShort().c_str();
		corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
		corpse->timer	= 6;
		if ( ch->gold > 0 )
		{
			if ( ch->GetInRoom() )
				ch->GetInRoom()->area->gold_looted += ch->gold;
			obj_to_obj( create_money( ch->gold ), corpse );
			ch->gold = 0;
		}
		
		/* Cannot use these!  They are used.
		corpse->value[0] = (int)ch->pIndexData->vnum;
		corpse->value[1] = (int)ch->max_hit;
		*/
		/*	Using corpse cost to cheat, since corpses not sellable */
		corpse->cost	 = (-(int)ch->pIndexData->vnum);
		corpse->value[2] = corpse->timer; 
		corpse->value[3] = ch->level*3;
	}
	else
	{
		name		= ch->getShort().c_str();
		corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
		corpse->timer	= 40;
		corpse->value[2] = (int)(corpse->timer/8);
	}
	
	/* Added corpse name - make locate easier , other skills */
	sprintf( buf, "corpse %s", name );
	corpse->name_ = buf;
	
	sprintf( buf, corpse->shortDesc_.c_str(), name );
	corpse->shortDesc_ = buf;
	
	sprintf( buf, corpse->longDesc_.c_str(), name );
	corpse->longDesc_ = buf;

	if ( qvar[QVAR_NOPLAYERLOOT].value != 1 && !IS_SET(ch->GetInRoom()->area->flags, AFLAG_NOPLAYERLOOT) )
	{
		/*
		 * This quest knob is for "No player looting"
		 * so, only put items into the corpse if no looting
		 * is NOT set (i.e. != 1)
		 * Also make sure that the area is not flagged for
		 * no player looting.
		 *    -Ksilyan
		 */
		for ( obj = ch->first_carrying; obj; obj = obj_next )
		{
			obj_next = obj->next_content;
			
			obj_from_char( obj );
			if ( IS_OBJ_STAT( obj, ITEM_INVENTORY )
				|| IS_OBJ_STAT( obj, ITEM_DEATHROT )
				|| IS_OBJ_STAT( obj, ITEM_DEITY))
			{
				extract_obj( obj, TRUE );
			}
			else
				obj_to_obj( obj, corpse );
		}
	}
	
	if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_NUKE) ) {
		CHAR_DATA *vic;
		
		for ( vic = ch->GetInRoom()->first_person; vic; vic = vic->next_in_room )
		{
			set_char_color( AT_MAGIC, vic );
			
			if ( vic == ch ) {
				send_to_char("You see your corpse burst into a million tiny pieces.\r\n", ch);
			}
			else
			{
				ch_printf(vic, "As %s's corpse hits the ground it bursts into a million tiny pieces.\r\n", PERS(ch, vic));
			}
		}
		
		extract_obj(corpse, TRUE);
	} else {
		obj_to_room( corpse, ch->GetInRoom() );
	}
	return;
}



void make_blood( CHAR_DATA *ch )
{
	OBJ_DATA *obj;

    if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_NUKE) ) {
        return;
    }
    
	obj		= create_object( get_obj_index( OBJ_VNUM_BLOOD ), 0 );
	obj->timer	= number_range( 2, 4 );
	obj->value[1]   = number_range( 3, UMIN(5, ch->level) );
	obj_to_room( obj, ch->GetInRoom() );
}


void make_bloodstain( CHAR_DATA *ch )
{
	OBJ_DATA *obj;

    if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_NUKE) ) {
        return;
    }

	obj		= create_object( get_obj_index( OBJ_VNUM_BLOODSTAIN ), 0 );
	obj->timer	= number_range( 1, 2 );
	obj_to_room( obj, ch->GetInRoom() );
}


/*
 * make some coinage
 */
OBJ_DATA *create_money( int amount )
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;

	if ( amount <= 0 )
	{
		bug( "Create_money: zero or negative money %d.", amount );
		amount = 1;
	}

	if ( amount == 1 )
	{
		obj = create_object( get_obj_index( OBJ_VNUM_MONEY_ONE ), 0 );
	}
	else
	{
		obj = create_object( get_obj_index( OBJ_VNUM_MONEY_SOME ), 0 );

		sprintf( buf, "%d gold coins", amount );
		obj->shortDesc_ = buf;

		sprintf( buf, "A pile of %d gold coins.", amount);
		obj->longDesc_ = buf;
		
		obj->value[0]	 = amount;
	}

	return obj;
}


/* Ksilyan
 * Create a stack of arrows.
 */
OBJ_DATA * create_arrowstack( OBJ_DATA * arrowtype, int count )
{
	OBJ_DATA * obj;

	obj = clone_object(arrowtype);

	obj->max_condition = obj->condition = count;

	return obj;
}







