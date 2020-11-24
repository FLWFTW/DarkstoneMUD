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
 *			   Object manipulation module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "bet.h"
#include "connection.h"
#include "object.h"
#include "paths.const.h"

/*
 * External functions
 */

void    show_list_to_char  ( OBJ_DATA *List, CHAR_DATA *ch,
				bool fShort, bool fShowNothing );
/*
 * Local functions.
 */
void	get_obj		( CHAR_DATA *ch, OBJ_DATA *obj,
			    OBJ_DATA *container );
bool	remove_obj	 ( CHAR_DATA *ch, int iWear, bool fReplace ) ;
void	wear_obj	 ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, sh_int wear_bit ) ;


/*
 * how resistant an object is to damage				-Thoric
 */
sh_int get_obj_resistance( OBJ_DATA *obj )
{
    sh_int resist;

    resist = number_fuzzy(MAX_ITEM_IMPACT);

    /* magical items are more resistant */
    if ( IS_OBJ_STAT( obj, ITEM_MAGIC ) )
      resist += number_fuzzy(12);
    /* metal objects are definitely stronger */
    if ( IS_OBJ_STAT( obj, ITEM_METAL ) )
      resist += number_fuzzy(5);
    /* organic objects are most likely weaker */
    if ( IS_OBJ_STAT( obj, ITEM_ORGANIC ) )
      resist -= number_fuzzy(5);
    /* blessed objects should have a little bonus */
    if ( IS_OBJ_STAT( obj, ITEM_BLESS ) )
      resist += number_fuzzy(5);
    /* lets make store inventory pretty tough */
    if ( IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
      resist += 20;
    /* deity objects rule! */
    if ( IS_OBJ_STAT( obj, ITEM_DEITY ) )
        resist += 100;

#ifdef USE_OBJECT_LEVELS
   /* okay... let's add some bonus/penalty for item level... */
    resist += (obj->level / 10) - 2;
#endif

    /* and lastly... take armor or weapon's condition into consideration */
    if (obj->item_type == ITEM_ARMOR || obj->item_type == ITEM_WEAPON)
      /*resist += (obj->value[0] / 2) - 2;  -- Removed by Ksilyan: new condition setting */
	  resist += obj->condition - 2;

    return URANGE(10, resist, 99);
}


void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
	ClanData *clan;
	int weight;

	if ( !CAN_WEAR(obj, ITEM_TAKE)
			&& (ch->level < sysdata.level_getobjnotake )  )
	{
		send_to_char( "You can't take that.\n\r", ch );
		return;
	}

	if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ))
	{

		if (
				can_take_proto( ch )
				|| (
					is_playbuilding(ch)
					&& obj->pIndexData->vnum >= ch->pcdata->area->low_o_vnum
					&& obj->pIndexData->vnum <= ch->pcdata->area->hi_o_vnum
				   )
		   )
			; /* then do so! */
		else
		{
			send_to_char( "A godly force prevents you from getting close to it.\n\r", ch );
			return;
		}
	}

	if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
	{
		act( AT_PLAIN, "$d: you can't carry that many items.",
				ch, NULL, obj->name_.c_str(), TO_CHAR );
		return;
	}

	if ( IS_OBJ_STAT( obj, ITEM_COVERING ) )
		weight = obj->weight;
	else
		weight = get_obj_weight( obj );

	if ( ch->carry_weight + weight > can_carry_w( ch ) )
	{
		act( AT_PLAIN, "$d: you can't carry that much weight.",
				ch, NULL, obj->name_.c_str(), TO_CHAR );
		return;
	}

	if ( container )
	{
		act( AT_ACTION, IS_OBJ_STAT(container, ITEM_COVERING) ?
				"You get $p from beneath $P." : "You get $p from $P.",
				ch, obj, container, TO_CHAR );
		act( AT_ACTION, IS_OBJ_STAT(container, ITEM_COVERING) ?
				"$n gets $p from beneath $P." : "$n gets $p from $P.",
				ch, obj, container, TO_ROOM );
		obj_from_obj( obj );
	}
	else
	{
		act( AT_ACTION, "You get $p.", ch, obj, container, TO_CHAR );
		act( AT_ACTION, "$n gets $p.", ch, obj, container, TO_ROOM );
		obj_from_room( obj );
	}

	/* Clan storeroom checks */
	if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_CLANSTOREROOM)
			&& (!container || container->CarriedById == 0) )
		for ( clan = first_clan; clan; clan = clan->next )
			if ( clan->storeroom == ch->GetInRoom()->vnum )
				save_clan_storeroom(ch, clan);

	/* Regular storeroom check */
	if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_STOREITEMS)
			&& ( !container || container->CarriedById == 0 ) ) {
		save_storeitems_room(ch);
	}

	if ( obj->item_type != ITEM_CONTAINER )
		check_for_trap( ch, obj, TRAP_GET );
	if ( char_died(ch) )
		return;

	if ( obj->item_type == ITEM_MONEY )
	{
		ch->gold += obj->value[0];
		extract_obj( obj, TRUE );
	}
	else
	{
		obj = obj_to_char( obj, ch );
		CheckObjectHolders(obj, ch);
	}

	if ( char_died(ch) || obj_extracted(obj) )
		return;
	oprog_get_trigger(ch, obj);
	mprog_get_trigger(ch, obj);
	return;
}

void do_get(CHAR_DATA *ch, const char* argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	OBJ_DATA *container;
	sh_int number;
	bool found;

	argument = one_argument( argument, arg1 );
	if ( is_number(arg1) )
	{
		number = atoi(arg1);
		if ( number < 1 )
		{
			send_to_char( "That was easy...\n\r", ch );
			return;
		}
		if ( (ch->carry_number + number) > can_carry_n(ch) )
		{
			send_to_char( "You can't carry that many.\n\r", ch );
			return;
		}
		argument = one_argument( argument, arg1 );
	}
	else
		number = 0;
	argument = one_argument( argument, arg2 );
	/* munch optional words */
	if ( !str_cmp( arg2, "from" ) && argument[0] != '\0' )
		argument = one_argument( argument, arg2 );

	/* Get type. */
	if ( arg1[0] == '\0' )
	{
		send_to_char( "Get what?\n\r", ch );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	if ( arg2[0] == '\0' )
	{
		if ( number <= 1 && str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
		{
			/* 'get obj' */
			obj = get_obj_list( ch, arg1, ch->GetInRoom()->first_content );
			if ( !obj )
			{
				act( AT_PLAIN, "I see no $T here.", ch, NULL, arg1, TO_CHAR );
				return;
			}
			separate_obj(obj);
			get_obj( ch, obj, NULL );
			if ( char_died(ch) )
				return;
			if ( IS_SET( sysdata.save_flags, SV_GET ) )
				save_char_obj( ch );
		}
		else
		{
			sh_int cnt = 0;
			bool fAll;
			char *chk;

			if ( IS_SET( ch->GetInRoom()->room_flags, ROOM_DONATION ) )
			{
				send_to_char( "The gods frown upon such a display of greed!\n\r", ch );
				return;
			}
			if ( !str_cmp(arg1, "all") )
				fAll = TRUE;
			else
				fAll = FALSE;
			if ( number > 1 )
				chk = arg1;
			else
				chk = &arg1[4];
			/* 'get all' or 'get all.obj' */
			found = FALSE;
			for ( obj = ch->GetInRoom()->first_content; obj; obj = obj_next )
			{
				obj_next = obj->next_content;
				if ( ( fAll || nifty_is_name( chk, obj->name_.c_str() ) )
					&&	 can_see_obj( ch, obj ) )
				{
					found = TRUE;
					if ( number && (cnt + obj->count) > number )
						split_obj( obj, number - cnt );
					cnt += obj->count;
					get_obj( ch, obj, NULL );
					if ( char_died(ch)
						||	 ch->carry_number >= can_carry_n( ch )
						||	 ch->carry_weight >= can_carry_w( ch )
						||	 (number && cnt >= number) )
					{
						if ( IS_SET(sysdata.save_flags, SV_GET)
							&&	!char_died(ch) )
							save_char_obj(ch);
						return;
					}
				}
			}

			if ( !found )
			{
				if ( fAll )
					send_to_char( "I see nothing here.\n\r", ch );
				else
					act( AT_PLAIN, "I see no $T here.", ch, NULL, chk, TO_CHAR );
			}
			else
				if ( IS_SET( sysdata.save_flags, SV_GET ) )
					save_char_obj( ch );
		}
	}
	else
	{
		/* 'get ... container' */
		if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
		{
			send_to_char( "You can't do that.\n\r", ch );
			return;
		}

		if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
		{
			act( AT_PLAIN, "I see no $T here.", ch, NULL, arg2, TO_CHAR );
			return;
		}

		switch ( container->item_type )
		{
			default:
				if ( !IS_OBJ_STAT( container, ITEM_COVERING ) )
				{
					send_to_char( "That's not a container.\n\r", ch );
					return;
				}
				if ( ch->carry_weight + container->weight > can_carry_w( ch ) )
				{
					send_to_char( "It's too heavy for you to lift.\n\r", ch );
					return;
				}
				break;

			case ITEM_CONTAINER:
			case ITEM_QUIVER:
			case ITEM_CORPSE_NPC:
				break;

			case ITEM_CORPSE_PC:
			{
				char name[MAX_INPUT_LENGTH];
				const char *pd;

				if ( IS_NPC(ch) )
				{
					send_to_char( "You can't do that.\n\r", ch );
					return;
				}

				pd = container->shortDesc_.c_str();
				pd = one_argument( pd, name );
				pd = one_argument( pd, name );
				pd = one_argument( pd, name );
				pd = one_argument( pd, name );
			}
		}

		if ( !IS_OBJ_STAT(container, ITEM_COVERING )
			&&	  IS_SET(container->value[1], CONT_CLOSED) && container->item_type != ITEM_QUIVER )
		{
			act( AT_PLAIN, "The $d is closed.", ch, NULL, container->name_.c_str(), TO_CHAR );
			return;
		}

		if ( number <= 1 && str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
		{
			/* 'get obj container' */
			obj = get_obj_list( ch, arg1, container->first_content );
			if ( !obj )
			{
				act( AT_PLAIN, IS_OBJ_STAT(container, ITEM_COVERING) ?
					"I see nothing like that beneath the $T." :
				"I see nothing like that in the $T.",
					ch, NULL, arg2, TO_CHAR );
				return;
			}
			separate_obj(obj);
			get_obj( ch, obj, container );

			check_for_trap( ch, container, TRAP_GET );
			if ( char_died(ch) )
				return;
			if ( IS_SET( sysdata.save_flags, SV_GET ) )
				save_char_obj( ch );
		}
		else
		{
			int cnt = 0;
			bool fAll;
			char *chk;

			/* 'get all container' or 'get all.obj container' */
			if ( IS_OBJ_STAT( container, ITEM_DONATION ) )
			{
				send_to_char( "The gods frown upon such an act of greed!\n\r", ch );
				return;
			}
			if ( !str_cmp(arg1, "all") )
				fAll = TRUE;
			else
				fAll = FALSE;
			if ( number > 1 )
				chk = arg1;
			else
				chk = &arg1[4];
			found = FALSE;
			for ( obj = container->first_content; obj; obj = obj_next )
			{
				obj_next = obj->next_content;
				if ( ( fAll || nifty_is_name( chk, obj->name_.c_str() ) )
					&&	 can_see_obj( ch, obj ) )
				{
					found = TRUE;
					if ( number && (cnt + obj->count) > number )
						split_obj( obj, number - cnt );
					cnt += obj->count;
					get_obj( ch, obj, container );
					if ( char_died(ch)
						||	 ch->carry_number >= can_carry_n( ch )
						||	 ch->carry_weight >= can_carry_w( ch )
						||	 (number && cnt >= number) )
						return;
				}
			}

			if ( !found )
			{
				if ( fAll )
					act( AT_PLAIN, IS_OBJ_STAT(container, ITEM_COVERING) ?
					"I see nothing beneath the $T." :
				"I see nothing in the $T.",
					ch, NULL, arg2, TO_CHAR );
				else
					act( AT_PLAIN, IS_OBJ_STAT(container, ITEM_COVERING) ?
					"I see nothing like that beneath the $T." :
				"I see nothing like that in the $T.",
					ch, NULL, arg2, TO_CHAR );
			}
			else
				check_for_trap( ch, container, TRAP_GET );
			if ( char_died(ch) )
				return;
			if ( found && IS_SET( sysdata.save_flags, SV_GET ) )
				save_char_obj( ch );
		}
	}
	return;
}



void do_put(CHAR_DATA *ch, const char* argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *container;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	ClanData *clan;
	sh_int	count;
	int		number;
	bool	save_char = FALSE;
	OBJ_DATA * NewArrow = NULL;

	argument = one_argument( argument, arg1 );

	// If the first argument is a number, store it and move on to next argument
	if ( is_number(arg1) )
	{
		number = atoi(arg1);
		if ( number < 1 )
		{
			send_to_char( "That was easy...\n\r", ch );
			return;
		}
		argument = one_argument( argument, arg1 );
	}
	else
		number = 0;

	argument = one_argument( argument, arg2 );

	/* munch optional words */
	if ( (!str_cmp(arg2, "into") || !str_cmp(arg2, "inside") || !str_cmp(arg2, "in"))
			&&   argument[0] != '\0' )
		argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Put what in what?\n\r", ch );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
	{
		send_to_char( "You can't do that.\n\r", ch );
		return;
	}

	if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
	{
		act( AT_PLAIN, "I see no $T here.", ch, NULL, arg2, TO_CHAR );
		return;
	}

	if ( !container->CarriedById && IS_SET( sysdata.save_flags, SV_PUT ) )
		save_char = TRUE;

	if ( IS_OBJ_STAT(container, ITEM_COVERING) )
	{
		if ( ch->carry_weight + container->weight > can_carry_w( ch ) )
		{
			send_to_char( "It's too heavy for you to lift.\n\r", ch );
			return;
		}
	}
	else
	{
		if ( container->item_type != ITEM_CONTAINER && container->item_type != ITEM_QUIVER)
		{
			send_to_char( "That's not a container.\n\r", ch );
			return;
		}

		if ( IS_SET(container->value[1], CONT_CLOSED) && container->item_type != ITEM_QUIVER )
		{
			act( AT_PLAIN, "The $d is closed.", ch, NULL, container->name_.c_str(), TO_CHAR );
			return;
		}
	}

	if ( number <= 1 && str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
	{
		/* 'put obj container' */
		if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
		{
			send_to_char( "You do not have that item.\n\r", ch );
			return;
		}

		if ( obj == container )
		{
			send_to_char( "You can't fold it into itself.\n\r", ch );
			return;
		}

		if ( !can_drop_obj( ch, obj ) )
		{
			send_to_char( "You can't let go of it.\n\r", ch );
			return;
		}

		if ( container->item_type == ITEM_QUIVER )
		{
			if ( obj->item_type != ITEM_PROJECTILE )
			{
	        	send_to_char("That's not a projectile.\r\n", ch);
		        return;
			}
			if (container->value[OBJECT_QUIVER_AMMOTYPE] != obj->value[OBJECT_PROJECTILE_AMMOTYPE])
			{
				ch_printf(ch, "%s isn't the right shape for %s.", obj->shortDesc_.c_str(), container->shortDesc_.c_str());
				return;
			}
		}

		if ( (IS_OBJ_STAT(container, ITEM_COVERING)
				&&   (get_obj_weight( obj ) / obj->count)
				> ((get_obj_weight( container ) / container->count)
				-   container->weight)) )
		{
			send_to_char( "It won't fit under there.\n\r", ch );
			return;
		}

		if ( container->item_type == ITEM_QUIVER )
		{
			if ( GetCountInQuiver(container) >= container->value[OBJECT_CONTAINER_CAPACITY] )
			{
				send_to_char( "It won't fit.\n\r", ch);
				return;
			}
			else if ( GetCountInQuiver(container) + obj->condition > container->value[OBJECT_CONTAINER_CAPACITY] )
			{
				int count = container->value[OBJECT_CONTAINER_CAPACITY] - GetCountInQuiver(container);
				if (count < obj->condition)
				{
					NewArrow = create_arrowstack(obj, count);
					obj->condition -= count;
					obj = NewArrow;
				}
			}
		}
		else
		{
			if ( (get_obj_weight(obj) / obj->count) + (get_obj_weight(container) / container->count)
					>  container->value[OBJECT_CONTAINER_CAPACITY] )
			{
				send_to_char( "It won't fit.\n\r", ch );
				return;
			}
		}

		separate_obj(obj);
		separate_obj(container);

		// If we have a new arrow, then the player isn't actually holding it.
		if (!NewArrow)
			obj_from_char( obj );
		obj = obj_to_obj( obj, container );
		check_for_trap ( ch, container, TRAP_PUT );
		if ( char_died(ch) )
			return;
		count = obj->count;
		obj->count = 1;
		act( AT_ACTION, IS_OBJ_STAT( container, ITEM_COVERING )
				? "$n hides $p beneath $P." : "$n puts $p in $P.",
				ch, obj, container, TO_ROOM );
		act( AT_ACTION, IS_OBJ_STAT( container, ITEM_COVERING )
				? "You hide $p beneath $P." : "You put $p in $P.",
		ch, obj, container, TO_CHAR );
		obj->count = count;

		if ( save_char )
			save_char_obj(ch);
		/* Clan storeroom check */
		if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_CLANSTOREROOM)
				&&   container->CarriedById == 0)
			for ( clan = first_clan; clan; clan = clan->next )
				if ( clan->storeroom == ch->GetInRoom()->vnum )
					save_clan_storeroom(ch, clan);

		/* Regular storeroom check */
		if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_STOREITEMS)
				&& ( !container || container->CarriedById == 0 ) )
			save_storeitems_room(ch);
	}
	else
	{
		bool found = FALSE;
		int cnt = 0;
		bool fAll;
		char *chk;

		if ( !str_prefix(arg1, "coins") )
		{
			/* Ksilyan
			 * Support for putting # coins into a container.
			 */
			if ( number > ch->gold )
			{
				ch_printf(ch, "That's very nice, but you don't have %d coins.\n\r", number);
				return;
			}
			ch->gold -= number;
			obj = create_money(number);
			obj = obj_to_char( obj, ch );
			strcpy(arg1, "coins");
			number = 1;
		}

		if ( !str_cmp(arg1, "all") )
			fAll = TRUE;
		else
			fAll = FALSE;

		if ( number > 1 )
			chk = arg1;
		else
		{
			if ( !str_cmp(arg1, "coins") )
				chk = arg1;
			else
				chk = &arg1[4];
		}

		separate_obj(container);
		/* 'put all container' or 'put all.obj container' */
		for ( obj = ch->first_carrying; obj; obj = obj_next )
		{
			obj_next = obj->next_content;

			if ( ( fAll || nifty_is_name( chk, obj->name_.c_str() ) )
					&&   can_see_obj( ch, obj )
					&&   obj->wear_loc == WEAR_NONE
					&&   obj != container
					&&   can_drop_obj( ch, obj )
					&&   (get_obj_weight( obj ) + get_obj_weight( container ) <= container->value[0]  || obj->item_type == ITEM_MONEY ) )
			{
				if ( container->item_type == ITEM_QUIVER )
				{
					if ( obj->item_type != ITEM_PROJECTILE )
					{
						send_to_char("That's not a projectile.\r\n", ch);
						continue;
					}
					if (container->value[OBJECT_QUIVER_AMMOTYPE] != obj->value[OBJECT_PROJECTILE_AMMOTYPE])
					{
						ch_printf(ch, "%s isn't the right shape for %s.",
								obj->shortDesc_.c_str(), container->shortDesc_.c_str());
						continue;
					}

					if ( GetCountInQuiver(container) >= container->value[OBJECT_CONTAINER_CAPACITY] )
					{
						ch_printf(ch,  "%s is full.\n\r", container->shortDesc_.c_str());
						return;
					}
					else if ( GetCountInQuiver(container) + obj->condition > container->value[OBJECT_CONTAINER_CAPACITY] )
					{
						int count = container->value[OBJECT_CONTAINER_CAPACITY] - GetCountInQuiver(container);
						if (count < obj->condition)
						{
							NewArrow = create_arrowstack(obj, count);
							obj->condition -= count;
							obj = NewArrow;
						}
					}

				}
				if ( number && (cnt + obj->count) > number )
					split_obj( obj, number - cnt );
				cnt += obj->count;
				obj_from_char( obj );
				act( AT_ACTION, "$n puts $p in $P.", ch, obj, container, TO_ROOM );
				act( AT_ACTION, "You put $p in $P.", ch, obj, container, TO_CHAR );
				obj = obj_to_obj( obj, container );
				if ( container->CarriedById != ch->GetId() )
				{
					UpdateObjectHolders(obj, ch);
				}
				found = TRUE;

				check_for_trap( ch, container, TRAP_PUT );
				if ( char_died(ch) )
					return;
				if ( number && cnt >= number )
					break;
			}
		}

		/*
		* Don't bother to save anything if nothing was dropped   -Thoric
		*/
		if ( !found )
		{
			if ( fAll )
				act( AT_PLAIN, "You are not carrying anything.",
						ch, NULL, NULL, TO_CHAR );
			else
				act( AT_PLAIN, "You are not carrying any $T.",
						ch, NULL, chk, TO_CHAR );
			return;
		}

		if ( save_char )
			save_char_obj(ch);
		/* Clan storeroom check */
		if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_CLANSTOREROOM)
				&& container->CarriedById == 0 )
			for ( clan = first_clan; clan; clan = clan->next )
				if ( clan->storeroom == ch->GetInRoom()->vnum )
					save_clan_storeroom(ch, clan);

		/* Regular storeroom check */
		if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_STOREITEMS)
				&& ( !container || container->CarriedById == 0 ) )
			save_storeitems_room(ch);

	}

	return;
}


void do_drop(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	bool found;
	ClanData *clan;
	int number;

	argument = one_argument( argument, arg );
	if ( is_number(arg) )
	{
		number = atoi(arg);
		if ( number < 1 )
		{
			send_to_char( "That was easy...\n\r", ch );
			return;
		}
		argument = one_argument( argument, arg );
	}
	else
		number = 0;

	if ( arg[0] == '\0' )
	{
		send_to_char( "Drop what?\n\r", ch );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	if ( IS_SET( ch->GetInRoom()->room_flags, ROOM_NODROP )
			||   (!IS_NPC(ch) && IS_SET( ch->act, PLR_LITTERBUG )) )
	{
		set_char_color( AT_MAGIC, ch );
		send_to_char( "A magical force stops you!\n\r", ch );
		set_char_color( AT_TELL, ch );
		send_to_char( "Someone tells you, 'No littering here!'\n\r", ch );
		return;
	}

	if ( number > 0 )
	{
		/* 'drop NNNN coins' */

		if ( !str_cmp( arg, "coins" ) || !str_cmp( arg, "coin" ) )
		{
			if ( ch->gold < number )
			{
				send_to_char( "You haven't got that many coins.\n\r", ch );
				return;
			}

			ch->gold -= number;

			for ( obj = ch->GetInRoom()->first_content; obj; obj = obj_next )
			{
				obj_next = obj->next_content;

				switch ( obj->pIndexData->vnum )
				{
					case OBJ_VNUM_MONEY_ONE:
						number += 1;
						extract_obj( obj, TRUE);
						break;

					case OBJ_VNUM_MONEY_SOME:
						number += obj->value[0];
						extract_obj( obj, TRUE );
						break;
				}
			}

			act( AT_ACTION, "$n drops some gold.", ch, NULL, NULL, TO_ROOM );

			if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_NUKE) ) {
				char buf[MAX_STRING_LENGTH];

				sprintf(buf, "As the gold hits the ground it bursts into a million tiny pieces!");

				echo_to_room(AT_MAGIC, ch->GetInRoom(), buf);
			} else {
				obj_to_room( create_money( number ), ch->GetInRoom() );
			}
			send_to_char( "OK.\n\r", ch );
			if ( IS_SET( sysdata.save_flags, SV_DROP ) )
				save_char_obj( ch );

			/* Clan storeroom saving */
			if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_CLANSTOREROOM) )
				for ( clan = first_clan; clan; clan = clan->next )
					if ( clan->storeroom == ch->GetInRoom()->vnum )
						save_clan_storeroom(ch, clan);

			/* Regular storeroom check */
			if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_STOREITEMS) ){
				save_storeitems_room(ch);
			}

			return;
		}
	}

	// It's not dropping coins

	if ( number <= 1 && str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
	{
		/* 'drop obj' */
		if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
		{
			send_to_char( "You do not have that item.\n\r", ch );
			return;
		}

		if ( !can_drop_obj( ch, obj ) )
		{
			send_to_char( "You can't let go of it.\n\r", ch );
			return;
		}

		separate_obj( obj );
		act( AT_ACTION, "$n drops $p.", ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "You drop $p.", ch, obj, NULL, TO_CHAR );

		obj_from_char( obj );
		UpdateObjectHolders(obj, ch);
		obj = obj_to_room( obj, ch->GetInRoom() );


		oprog_drop_trigger ( ch, obj );   /* mudprogs */

		if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_NUKE) ) {
			char buf[MAX_STRING_LENGTH];

			sprintf(buf, "As %s hits the ground it bursts into a million tiny pieces!", obj->shortDesc_.c_str());

			echo_to_room(AT_MAGIC, ch->GetInRoom(), buf);

			extract_obj(obj, TRUE);
			return;
		}

		if( char_died(ch) || obj_extracted(obj) )
			return;

		/* Clan storeroom saving */
		if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_CLANSTOREROOM) )
			for ( clan = first_clan; clan; clan = clan->next )
				if ( clan->storeroom == ch->GetInRoom()->vnum )
					save_clan_storeroom(ch, clan);

		/* Regular storeroom check */
		if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_STOREITEMS) ){
			save_storeitems_room(ch);
		}

	}
	else
	{
		int cnt = 0;
		char *chk;
		bool fAll;

		if ( !str_cmp(arg, "all") )
			fAll = TRUE;
		else
			fAll = FALSE;
		if ( number > 1 )
			chk = arg;
		else
			chk = &arg[4];
		/* 'drop all' or 'drop all.obj' */
		if ( IS_SET( ch->GetInRoom()->room_flags, ROOM_NODROPALL )
				||   IS_SET( ch->GetInRoom()->room_flags, ROOM_CLANSTOREROOM ) )
		{
			send_to_char( "You can't seem to do that here...\n\r", ch );
			return;
		}
		found = FALSE;
		for ( obj = ch->first_carrying; obj; obj = obj_next )
		{
			obj_next = obj->next_content;

			if ( (fAll || nifty_is_name( chk, obj->name_.c_str() ) )
					&&   can_see_obj( ch, obj )
					&&   obj->wear_loc == WEAR_NONE
					&&   can_drop_obj( ch, obj ) )
			{
				found = TRUE;
				if ( obj->pIndexData->progtypes & DROP_PROG && obj->count > 1 )
				{
					++cnt;
					separate_obj( obj );
					obj_from_char( obj );
					if ( !obj_next )
						obj_next = ch->first_carrying;
				}
				else
				{
					if ( number && (cnt + obj->count) > number )
						split_obj( obj, number - cnt );
					cnt += obj->count;
					obj_from_char( obj );
				}
				act( AT_ACTION, "$n drops $p.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "You drop $p.", ch, obj, NULL, TO_CHAR );
				obj = obj_to_room( obj, ch->GetInRoom() );

				if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_NUKE) ) {
					char buf[MAX_STRING_LENGTH];

					sprintf(buf, "As %s hits the ground it bursts into a million tiny pieces!", obj->shortDesc_.c_str());

					echo_to_room(AT_MAGIC, ch->GetInRoom(), buf);

					extract_obj(obj, TRUE);
					continue;
				}

				oprog_drop_trigger( ch, obj );		/* mudprogs */
				if ( char_died(ch) )
					return;
				if ( number && cnt >= number )
					break;
			}
		}

		if ( !found )
		{
			if ( fAll )
				act( AT_PLAIN, "You are not carrying anything.",
						ch, NULL, NULL, TO_CHAR );
			else
				act( AT_PLAIN, "You are not carrying any $T.",
						ch, NULL, chk, TO_CHAR );
		}

		/* Clan storeroom saving */
		if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_CLANSTOREROOM) )
			for ( clan = first_clan; clan; clan = clan->next )
				if ( clan->storeroom == ch->GetInRoom()->vnum )
					save_clan_storeroom(ch, clan);

		/* Regular storeroom check */
		if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_STOREITEMS) ){
			save_storeitems_room(ch);
		}


	}
	if ( IS_SET( sysdata.save_flags, SV_DROP ) )
		save_char_obj( ch );	/* duping protector */
	return;
}

void UpdateObjectHolders ( OBJ_DATA * obj, CHAR_DATA * ch)
{
	if (IS_NPC(ch))
		return;

	if (!ch->GetConnection())
		return;
	if (!ch->GetConnection()->Account)
		return;

	if ( ch->getName() == obj->Holders[1]->Name )
	{
		// If the name is the same, only update the time,
		// and don't shift the rest of the information.

		char buf[30];
		sprintf(buf, "%s", ctime(&secCurrentTime));
		buf[24] = '\0'; // Remove trailing newline  - index 24 (see reference)
		obj->Holders[1]->Time.assign(buf);
		return;
	}

	obj->Holders[0]->Host = obj->Holders[1]->Host;
	obj->Holders[0]->Name = obj->Holders[1]->Name;
	obj->Holders[0]->Time = obj->Holders[1]->Time;
	obj->Holders[0]->Account = obj->Holders[1]->Account;

	obj->Holders[1]->Host = ch->GetConnection()->GetHostString();
	obj->Holders[1]->Name = ch->getName().str();
	obj->Holders[1]->Account = ch->GetConnection()->Account->email;

	char buf[30];
	sprintf(buf, "%s", ctime(&secCurrentTime));
	buf[24] = '\0'; // Remove trailing newline  - index 24 (see reference)
	obj->Holders[1]->Time.assign(buf);
}

void CheckObjectHolders ( OBJ_DATA * obj, CHAR_DATA * ch)
{
	int i;
	bool found;
	int HolderNumber;

	if (!obj)
		return;

	if (!ch->GetConnection())
		return;
	if (!ch->GetConnection()->Account)
		return;
	if (!ch->GetInRoom())
		return;

	if (IS_NPC(ch))
		return;

	if ( IS_SET(ch->GetConnection()->Account->flags, ACCOUNT_CANMPLAY) )
		return;

	found = FALSE;

	for (i = 0; i <= 1; i++)
	{
		if ( obj->Holders[i]->Name.compare("") == 0)
			continue; // empty holder, don't consider

		if ( ( obj->Holders[i]->Host.compare(ch->GetConnection()->GetHost()) == 0 )
			&& ( ch->getName() != obj->Holders[i]->Name )
			) // host is same, but names are different
		{
			if ( obj->Holders[i]->Account.compare(ch->GetConnection()->Account->email) )
			{
				// accounts different --> this means multiplaying!
				found = TRUE;
				HolderNumber = i;
			}
		}
		if ( obj->Holders[i]->Account.compare(ch->GetConnection()->Account->email) == 0
			&& ch->getName() != obj->Holders[i]->Name
			) // account is same, but name is different
		{
			found = TRUE;
			HolderNumber = i;
		}
	}

	if (found)
	{
		char buf[MAX_STRING_LENGTH];


		sprintf(buf, "MULTIPLAYING: Object %s (%s) last held by %s (%s) on %s",
				obj->shortDesc_.c_str(), vnum_to_dotted(obj->pIndexData->vnum), obj->Holders[HolderNumber]->Name.c_str(),
				obj->Holders[HolderNumber]->Account.c_str(), obj->Holders[HolderNumber]->Time.c_str() );
		log_string(buf);
		log_multiplaying(buf);
		sprintf(buf, "              acquired by %s (%s) in room %s (%s).",
					ch->getName().c_str(), ch->GetConnection()->Account->email,
					ch->GetInRoom()->name_.c_str(), vnum_to_dotted(ch->GetInRoom()->vnum));
		log_string(buf);
		log_multiplaying(buf);
	}
}

void do_give(CHAR_DATA *ch, const char* argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char buf  [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA  *obj;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	if ( !str_cmp( arg2, "to" ) && argument[0] != '\0' )
		argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Give what to whom?\n\r", ch );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	if ( is_number( arg1 ) )
	{
		/* 'give NNNN coins victim' */
		int amount;

		amount	 = atoi(arg1);
		if ( amount <= 0
			|| ( str_cmp( arg2, "coins" ) && str_cmp( arg2, "coin" ) ) )
		{
			send_to_char( "Sorry, you can't do that.\n\r", ch );
			return;
		}

		argument = one_argument( argument, arg2 );
		if ( !str_cmp( arg2, "to" ) && argument[0] != '\0' )
			argument = one_argument( argument, arg2 );
		if ( arg2[0] == '\0' )
		{
			send_to_char( "Give what to whom?\n\r", ch );
			return;
		}

		if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
		{
			send_to_char( "They aren't here.\n\r", ch );
			return;
		}

		if ( ch->gold < amount )
		{
			send_to_char( "Very generous of you, but you haven't got that much gold.\n\r", ch );
			return;
		}

		ch->gold	 -= amount;
		victim->gold += amount;
		strcpy(buf, "$n gives you ");
		strcat(buf, arg1);
		strcat(buf, (amount > 1) ? " coins." : " coin.");

		act( AT_ACTION, buf, ch, NULL, victim, TO_VICT	  );
		act( AT_ACTION, "$n gives $N some gold.",  ch, NULL, victim, TO_NOTVICT );
		act( AT_ACTION, "You give $N some gold.",  ch, NULL, victim, TO_CHAR	);
		send_to_char( "OK.\n\r", ch );
		mprog_bribe_trigger( victim, ch, amount );
		if ( IS_SET( sysdata.save_flags, SV_GIVE ) && !char_died(ch) )
			save_char_obj(ch);
		if ( IS_SET( sysdata.save_flags, SV_RECEIVE ) && !char_died(victim) )
			save_char_obj(victim);
		return;
	}

	if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
	{
		send_to_char( "You do not have that item.\n\r", ch );
		return;
	}

	if ( obj->wear_loc != WEAR_NONE )
	{
		send_to_char( "You must remove it first.\n\r", ch );
		return;
	}

	if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if ( !can_drop_obj( ch, obj ) && !IS_IMMORTAL(victim) )
	{
		send_to_char( "You can't let go of it.\n\r", ch );
		return;
	}

	if ( victim->carry_number + (get_obj_number(obj)/obj->count) > can_carry_n( victim ) && !IS_IMMORTAL(ch) )
	{
		act( AT_PLAIN, "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if ( victim->carry_weight + (get_obj_weight(obj)/obj->count) > can_carry_w( victim ) )
	{
		act( AT_PLAIN, "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if ( !can_see_obj( victim, obj ) && !IS_IMMORTAL(ch) )
	{
		act( AT_PLAIN, "$N can't see it.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if (IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) && !can_take_proto( victim ) )
	{
		act( AT_PLAIN, "You cannot give that to $N!", ch, NULL, victim, TO_CHAR );
		return;
	}

	/* matthew */
	if ( victim->pIndexData && victim->pIndexData->pStable && obj->pObjNPC ) {
		stable_give(ch, obj, victim);
		return;
	}

	separate_obj( obj );
	obj_from_char( obj );
	UpdateObjectHolders(obj, ch);
	act( AT_ACTION, "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT );
	act( AT_ACTION, "$n gives you $p.",   ch, obj, victim, TO_VICT	  );
	act( AT_ACTION, "You give $p to $N.", ch, obj, victim, TO_CHAR	  );
	obj = obj_to_char( obj, victim );
	CheckObjectHolders(obj, victim);
	mprog_give_trigger( victim, ch, obj );
	if ( IS_SET( sysdata.save_flags, SV_GIVE ) && !char_died(ch) )
		save_char_obj(ch);
	if ( IS_SET( sysdata.save_flags, SV_RECEIVE ) && !char_died(victim) )
		save_char_obj(victim);
	return;
}

/* Ksilyan:
	Allow for sheathing of a weapon.
*/

void do_sheathe(CHAR_DATA * ch, const char* argument)
{
	OBJ_DATA * scabbard = NULL;
	OBJ_DATA * weapon = NULL;
	char arg1[MAX_STRING_LENGTH];
	bool autosheathe;
	bool noauto;
	bool found;
	sh_int weapontype;

	one_argument(argument, arg1);

	autosheathe = noauto = found = FALSE;

	if ( !strcmp(arg1, "auto") )
		autosheathe = TRUE;

	if ( strlen(arg1) > 0 && !autosheathe )
	{
		OBJ_DATA * wield;

		wield = get_eq_char( ch, WEAR_WIELD );
		if (!wield)
		{
			send_to_char("You're not wielding a weapon.\n\r", ch);
			return;
		}

		if (nifty_is_name_prefix(arg1, wield->name_.c_str()) )
		{
			weapon = wield;
		}
		else
		{
			wield = get_eq_char(ch, WEAR_DUAL_WIELD);
			if (wield && nifty_is_name_prefix(arg1, wield->name_.c_str()) )
				weapon = wield;
		}
		if (!weapon)
		{
			send_to_char( "You don't have that weapon.\n\r", ch );
			return;
		}
		noauto = TRUE;
	}

	if ( !weapon )
		if ( ( weapon = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
		{
			if ( !autosheathe )
				send_to_char( "You're not wielding a weapon to sheathe...\n\r", ch );
			return;
		}

	weapontype = weapon->value[OBJECT_WEAPON_WEAPONTYPE];

	if ( ( scabbard = get_eq_char( ch, WEAR_SCABBARD_1 ) ) != NULL )
	{
		if ( scabbard->value[OBJECT_SCABBARD_WEAPONTYPE] == weapontype
//				&& get_eq_char(ch, WEAR_SHEATHED_1) == NULL )
				&& scabbard->first_content == NULL )
		{
			// Put the weapon into sheath 1.
			if ( remove_obj(ch, weapon->wear_loc, TRUE) )
			{
				act( AT_ACTION, "$n sheathes $p.", ch, weapon, NULL, TO_ROOM );
				act( AT_ACTION, "You sheathe $p.", ch, weapon, NULL, TO_CHAR );

				obj_from_char(weapon);
				obj_to_obj(weapon, scabbard);
				//weapon->wear_loc = WEAR_SHEATHED_1;

				if ( get_eq_char( ch, WEAR_WIELD ) != NULL && !noauto )
					do_sheathe( ch, "auto");
			}

			return;
		}
		else
		{
			if ( (scabbard = get_eq_char( ch, WEAR_SCABBARD_2 ) ) != NULL )
			{
				if ( scabbard->value[OBJECT_SCABBARD_WEAPONTYPE] == weapontype
//						&& get_eq_char(ch, WEAR_SHEATHED_2) == NULL )
						&& scabbard->first_content == NULL )
				{
					// Put the weapon into sheath 2.
					if ( remove_obj(ch, weapon->wear_loc, TRUE) )
					{
						act( AT_ACTION, "$n sheathes $p.", ch, weapon, NULL, TO_ROOM );
						act( AT_ACTION, "You sheathe $p.", ch, weapon, NULL, TO_CHAR );

						obj_from_char(weapon);
						obj_to_obj(weapon, scabbard);
						//weapon->wear_loc = WEAR_SHEATHED_2;

						if ( get_eq_char( ch, WEAR_WIELD ) != NULL && !noauto )
							do_sheathe( ch, "auto");
					}

					return;
				}
			}
		}
	}

	ch_printf( ch, "You have nowhere to put %s.\n\r", weapon->shortDesc_.c_str() );
	return;
}

/*
 * Ksilyan
 * Draw weapons from sheathes and wield them.
 */

void do_draw(CHAR_DATA * ch, const char* argument)
{
	OBJ_DATA * weapon;
	char arg1[MAX_STRING_LENGTH];

	argument = one_argument(argument, arg1);

	// temporary pointers . . . baaaaad
	OBJ_DATA * scabbard1 = NULL;
	OBJ_DATA * scabbard2 = NULL;
	scabbard1 = get_eq_char(ch, WEAR_SCABBARD_1);
	scabbard2 = get_eq_char(ch, WEAR_SCABBARD_2);
	if ( !scabbard1 && !scabbard2 )
	{
		send_to_char( "You're not wearing a scabbard.\n\r", ch);
		return;
	}

	//if ( get_eq_char(ch, WEAR_SHEATHED_1) == NULL && get_eq_char(ch, WEAR_SHEATHED_2) == NULL )
	if ( (scabbard1 && scabbard1->first_content == NULL) && (scabbard2 && scabbard2->first_content == NULL ) )
	{
		send_to_char( "You don't have anything to draw.\n\r", ch);
		return;
	}

	if (strlen(arg1) > 0)
	{
		OBJ_DATA * sheathed;

		/*
		sheathed = get_eq_char(ch, WEAR_SHEATHED_1);
		if (sheathed && nifty_is_name_prefix(arg1, sheathed->name))
		{
			wear_obj(ch, sheathed, TRUE, -1 );
			return;
		}
		sheathed = get_eq_char(ch, WEAR_SHEATHED_2);
		if (sheathed && nifty_is_name_prefix(arg1, sheathed->name))
		{
			wear_obj(ch, sheathed, TRUE, -1 );
			return;
		}
		*/
		if (scabbard1)
		{
			sheathed = scabbard1->first_content;
			if (sheathed && nifty_is_name_prefix(arg1, sheathed->name_.c_str()))
			{
				obj_from_obj(sheathed); // remove weapon from scabbard
				sheathed = obj_to_char(sheathed, ch); // move weapon to ch's inventory
				separate_obj(sheathed);
				sheathed->wear_loc = WEAR_SHEATHED_1;
				wear_obj(ch, sheathed, TRUE, -1 ); // wear the weapon
				if (sheathed->wear_loc == WEAR_SHEATHED_1)
				{
					// If this is still the wear_loc, then it wasn't actually worn.
					// So put it back into the scabbard.
					sheathed->wear_loc = -1;
					obj_from_char(sheathed);
					obj_to_obj(sheathed, scabbard1);
				}
				return;
			}
		}
		if (scabbard2)
		{
			sheathed = scabbard2->first_content;
			if (sheathed && nifty_is_name_prefix(arg1, sheathed->name_.c_str()))
			{
				obj_from_obj(sheathed); // remove weapon from scabbard
				sheathed = obj_to_char(sheathed, ch); // move weapon to ch's inventory
				separate_obj(sheathed);
				sheathed->wear_loc = WEAR_SHEATHED_1; // this is temporary to get the draw message
				wear_obj(ch, sheathed, TRUE, -1 ); // wear the weapon

				if (sheathed->wear_loc == WEAR_SHEATHED_1)
				{
					// If this is still the wear_loc, then it wasn't actually worn.
					// So put it back into the scabbard.
					sheathed->wear_loc = -1;
					obj_from_char(sheathed);
					obj_to_obj(sheathed, scabbard2);
				}
				return;
			}
		}
		send_to_char( "You don't have that sheathed.\n\r", ch);
		return;
	}

//	if ( (scabbard = get_eq_char(ch, WEAR_SCABBARD_1) ) != NULL )
	if ( scabbard1 )
	{
//		if ( (weapon = get_eq_char(ch, WEAR_SHEATHED_1) ) != NULL )
		if ( (weapon = scabbard1->first_content ) != NULL )
		{
			obj_from_obj(weapon); // remove weapon from scabbard
			weapon = obj_to_char(weapon, ch); // move weapon to ch's inventory
			separate_obj(weapon);
			weapon->wear_loc = WEAR_SHEATHED_1; // this is temporary to get the draw message
			wear_obj(ch, weapon, TRUE, -1 ); // wear the weapon
			if (weapon->wear_loc == WEAR_SHEATHED_1)
			{
				// If this is still the wear_loc, then it wasn't actually worn.
				// So put it back into the scabbard.
				weapon->wear_loc = -1;
				obj_from_char(weapon);
				obj_to_obj(weapon, scabbard1);
			}
		}
	}

//	if ( (scabbard = get_eq_char(ch, WEAR_SCABBARD_2) ) != NULL )
	if ( scabbard2 )
	{
//		if ( ( weapon = get_eq_char(ch, WEAR_SHEATHED_2) ) != NULL )
		if ( ( weapon = scabbard2->first_content ) != NULL )
		{
			obj_from_obj(weapon); // remove weapon from scabbard
			weapon = obj_to_char(weapon, ch); // move weapon to ch's inventory
			separate_obj(weapon);
			weapon->wear_loc = WEAR_SHEATHED_1; // this is temporary to get the draw message
			wear_obj(ch, weapon, TRUE, -1 ); // wear the weapon
			if (weapon->wear_loc == WEAR_SHEATHED_1)
			{
				// If this is still the wear_loc, then it wasn't actually worn.
				// So put it back into the scabbard.
				weapon->wear_loc = -1;
				obj_from_char(weapon);
				obj_to_obj(weapon, scabbard2);
			}
		}
	}

	return;
}

/*
 * Ksilyan: 'use' an object
 */
void do_use(Character * ch, const char* argument)
{
	char arg1[MAX_INPUT_LENGTH];

	// Get the first argument (the object to use)
	one_argument(argument, arg1);

	// Mental state check
	if ( ms_find_obj(ch) )
		return;

	Object * obj;

	// Find the object to use
	if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
	{
		send_to_char( "You do not have that item.\n\r", ch );
		return;
	}

	// Send the object a 'use' message
	ArgumentMobile chArg(ch);
	list<Argument*> list;
	list.push_back(&chArg);
	obj->skryptSendEvent("use", list);
	list.clear();
}

/*
 * Damage an object.						-Thoric
 * Affect player's AC if necessary.
 * Make object into scraps if necessary.
 * Send message about damaged object.
 *
 * Ksilyan: handling for (max)condition.
 *
 */
obj_ret damage_obj( Object *obj )
{
	CHAR_DATA *ch;
	obj_ret objcode;

	ch = obj->GetCarriedBy();
	objcode = rNONE;

	if ( in_arena(ch) )
	{
		return rNONE;
	}

	if ( IS_SET(obj->extra_flags, ITEM_DEITY) )
	{
		return rNONE;
	}

	separate_obj( obj );
	if ( ch )
		act( AT_OBJECT, "($p gets damaged)", ch, obj, NULL, TO_CHAR );
	else if ( obj->GetInRoom() && ( ch = obj->GetInRoom()->first_person ) != NULL )
	{
		act( AT_OBJECT, "($p gets damaged)", ch, obj, NULL, TO_ROOM );
		act( AT_OBJECT, "($p gets damaged)", ch, obj, NULL, TO_CHAR );
		ch = NULL;
	}

	oprog_damage_trigger(ch, obj);
	if ( obj_extracted(obj) )
		return global_objcode;

	switch( obj->item_type )
	{
		default:
			make_scraps( obj );
			objcode = rOBJ_SCRAPPED;
			break;

		case ITEM_CONTAINER:
			if (--obj->condition <= 0)
			{
				if (obj->max_condition <= 1 || obj->condition <= -obj->max_condition )
				{
					unequip_char(ch, obj);
					make_scraps( obj );
					objcode = rOBJ_SCRAPPED;
				}
				else if (obj->condition == 0)
				{
					if (ch)
					{
						act( AT_OBJECT, "($p is broken and needs to be repaired)", ch, obj, NULL, TO_CHAR );
						act( AT_OBJECT, "($p breaks)", ch, obj, NULL, TO_ROOM );
						if (obj->wear_loc != WEAR_NONE)
							unequip_char(ch, obj);
					}
					else if ( obj->GetInRoom() && (ch = obj->GetInRoom()->first_person ) != NULL )
					{
						act( AT_OBJECT, "($p breaks)", ch, obj, NULL, TO_ROOM );
					}
				}
			}
			break;

		case ITEM_ARMOR:
			if (--obj->condition <= 0)
			{
				if (obj->max_condition == 1 || obj->condition <= -obj->max_condition )
				{
					unequip_char(ch, obj);
					make_scraps( obj );
					objcode = rOBJ_SCRAPPED;
				}
				else if (obj->condition == 0)
				{
					if (ch)
					{
						act( AT_OBJECT, "($p is broken and needs to be repaired)", ch, obj, NULL, TO_CHAR );
						act( AT_OBJECT, "($p breaks)", ch, obj, NULL, TO_ROOM );
						if (obj->wear_loc != WEAR_NONE)
							unequip_char(ch, obj);
					}
					else if ( obj->GetInRoom() && (ch = obj->GetInRoom()->first_person ) != NULL )
					{
						act( AT_OBJECT, "($p breaks)", ch, obj, NULL, TO_ROOM );
					}
				}
			}

			break;
		case ITEM_WEAPON:
			if (--obj->condition <= 0)
			{
				if (obj->max_condition == 1 || obj->condition <= -obj->max_condition )
				{
					unequip_char(ch, obj);
					make_scraps( obj );
					objcode = rOBJ_SCRAPPED;
				}
				else if (obj->condition >= 0)
				{
					if (ch)
					{
						act( AT_OBJECT, "($p is broken and needs to be repaired)", ch, obj, NULL, TO_CHAR );
						act( AT_OBJECT, "($p breaks)", ch, obj, NULL, TO_ROOM );
						if (obj->wear_loc != WEAR_NONE)
							unequip_char(ch, obj);
					}
					else if ( obj->GetInRoom() && (ch = obj->GetInRoom()->first_person ) != NULL )
					{
						act( AT_OBJECT, "($p breaks)", ch, obj, NULL, TO_ROOM );
					}
				}
			}
			break;
	}
	return objcode;
}


/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace )
{
    OBJ_DATA *obj, *tmpobj;

    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	return TRUE;

    if ( !fReplace
    &&   ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	act( AT_PLAIN, "$d: you can't carry that many items.",
	    ch, NULL, obj->name_.c_str(), TO_CHAR );
	return FALSE;
    }

    if ( !fReplace )
	return FALSE;

    if ( IS_OBJ_STAT(obj, ITEM_NOREMOVE) )
    {
	act( AT_PLAIN, "You can't remove $p.", ch, obj, NULL, TO_CHAR );
	return FALSE;
    }

	if ( obj == get_eq_char( ch, WEAR_SCABBARD_1 )
		&& ( tmpobj = get_eq_char( ch, WEAR_SHEATHED_1 ) ) != NULL )
	{
		act( AT_PLAIN, "You can't remove $p until you empty it.", ch, obj, NULL, TO_CHAR );
		return FALSE;
	}

    if ( obj == get_eq_char( ch, WEAR_WIELD )
    && ( tmpobj = get_eq_char( ch, WEAR_DUAL_WIELD)) != NULL )
       tmpobj->wear_loc = WEAR_WIELD;

	if ( obj == get_eq_char( ch, WEAR_SCABBARD_1 )
	&& ( tmpobj = get_eq_char( ch, WEAR_SCABBARD_2)) != NULL )
	{
		tmpobj->wear_loc = WEAR_SCABBARD_1;
		if ( (tmpobj = get_eq_char( ch, WEAR_SHEATHED_2 )) != NULL )
			tmpobj->wear_loc = WEAR_SHEATHED_1;
	}

    unequip_char( ch, obj );

    act( AT_ACTION, "$n stops using $p.", ch, obj, NULL, TO_ROOM );
    act( AT_ACTION, "You stop using $p.", ch, obj, NULL, TO_CHAR );
    oprog_remove_trigger( ch, obj );
    return TRUE;
}

/*
 * See if char could be capable of dual-wielding		-Thoric
 */
bool could_dual( CHAR_DATA *ch )
{
  if ( IS_NPC(ch) )
    return TRUE;
  if ( ch->pcdata->learned[gsn_dual_wield] )
    return TRUE;

  return FALSE;
}

/*
 * See if char can dual wield at this time			-Thoric
 */
bool can_dual( CHAR_DATA *ch )
{
   if ( !could_dual(ch) )
     return FALSE;

   if ( get_eq_char( ch, WEAR_DUAL_WIELD ) )
   {
	send_to_char( "You are already wielding two weapons!\n\r", ch );
	return FALSE;
   }
   if ( get_eq_char( ch, WEAR_SHIELD ) )
   {
	send_to_char( "You cannot dual wield while holding a shield!\n\r", ch );
	return FALSE;
   }
   if ( get_eq_char( ch, WEAR_HOLD ) )
   {
	send_to_char( "You cannot dual wield while holding something!\n\r", ch );
	return FALSE;
   }
   return TRUE;
}


/*
 * Check to see if there is room to wear another object on this location
 * (Layered clothing support)
 */
bool can_layer( CHAR_DATA *ch, OBJ_DATA *obj, sh_int wear_loc )
{
    OBJ_DATA   *otmp;
    sh_int	bitlayers = 0;
    sh_int	objlayers = obj->pIndexData->layers;

    for ( otmp = ch->first_carrying; otmp; otmp = otmp->next_content )
	if ( otmp->wear_loc == wear_loc )
    {
	    if ( !otmp->pIndexData->layers )
        {
            return FALSE;
        }
	    else
        {
            bitlayers |= otmp->pIndexData->layers;
        }
    }

    if ( (bitlayers && !objlayers) || bitlayers > objlayers ) {
	    return FALSE;
    }
    if ( !bitlayers || ((bitlayers & ~objlayers) == bitlayers) ) {
	    return TRUE;
    }
    return FALSE;
}

/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 * Restructured a bit to allow for specifying body location	-Thoric
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, sh_int wear_bit )
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *tmpobj;
	sh_int bit, tmp;

	separate_obj( obj );

#ifdef USE_OBJECT_LEVELS
	if ( get_trust( ch ) < obj->level )
	{
		sprintf( buf, "You must be level %d to use this object.\n\r",
				obj->level );
		send_to_char( buf, ch );
		act( AT_ACTION, "$n tries to use $p, but is too inexperienced.",
				ch, obj, NULL, TO_ROOM );
		return;
	}
#endif

	if ( !IS_IMMORTAL(ch))
	{
		/* KSILYAN
		   Added in item classifications, for armor.
		 */
		bool canwear;
		canwear = TRUE;
		if (obj->item_type == ITEM_ARMOR)
		{
			int classification;
			int Class;
			classification = obj->value[OBJECT_ARMOR_CLASSIFICATION];
			Class = ch->Class;
			if ((classification == 4) && ((Class != CLASS_WARRIOR) || (Class != CLASS_KNIGHT)) )
			{
				canwear = FALSE;
			}
			if ((classification == 3) && ((Class != CLASS_WARRIOR) || (Class != CLASS_KNIGHT) || (Class != CLASS_DRUID) ||
						(Class != CLASS_RANGER) || (Class != CLASS_THIEF)) )
			{
				canwear = FALSE;
			}
			if ((classification == 2) && ((Class == CLASS_CLERIC) || (Class == CLASS_MAGE)) )
			{
				/* here, everyone but cleric and mage can wear class two items. */
				canwear = FALSE;
			}
		}

		if ( (( IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR)
						&& ch->Class == CLASS_WARRIOR					)
					||   ( IS_OBJ_STAT(obj, ITEM_ANTI_MAGE)
						&& ch->Class == CLASS_MAGE					)
					||   ( IS_OBJ_STAT(obj, ITEM_ANTI_THIEF)
						&& ch->Class == CLASS_THIEF					)
					||   ( IS_OBJ_STAT(obj, ITEM_ANTI_VAMPIRE)
						&& ch->Class == CLASS_VAMPIRE					)
					||   ( IS_OBJ_STAT(obj, ITEM_ANTI_DRUID)
						&& ch->Class == CLASS_DRUID					)
					||   ( IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR)
						&& ch->Class == CLASS_RANGER					)
					||	 ( IS_OBJ_STAT(obj, ITEM_ANTI_MAGE)
						&& ch->Class == CLASS_AUGURER					)
					||   ( IS_OBJ_STAT(obj, ITEM_ANTI_CLERIC)
						&& ch->Class == CLASS_CLERIC					)) )
		{
			canwear = FALSE;
		}

		if (canwear == FALSE)
		{
			act( AT_MAGIC, "You are forbidden to use that item.", ch, NULL, NULL, TO_CHAR );
			act( AT_ACTION, "$n tries to use $p, but is forbidden to do so.", ch, obj, NULL, TO_ROOM );
			return;
		}
	}

	/* Ksilyan: condition handling */
	if ( obj->condition <= 0 )
	{
		act( AT_OBJECT, "You must repair $p before you can use it.", ch, obj, NULL, TO_CHAR );
		return;
	}

	if ( wear_bit > -1 )
	{
		bit = wear_bit;
		if ( !CAN_WEAR(obj, 1 << bit) )
		{
			if ( fReplace )
			{
				switch( 1 << bit )
				{
					case ITEM_HOLD:
						send_to_char( "You cannot hold that.\n\r", ch );
						break;
					case ITEM_WIELD:
						send_to_char( "You cannot wield that.\n\r", ch );
						break;
					default:
						sprintf( buf, "You cannot wear that on your %s.\n\r",
								w_flags[bit] );
						send_to_char( buf, ch );
				}
			}
			return;
		}
	}
	else
	{
		for ( bit = -1, tmp = 1; tmp < 31; tmp++ )
		{
			if ( CAN_WEAR(obj, 1 << tmp) )
			{
				bit = tmp;
				break;
			}
		}
	}

	/* currently cannot have a light in non-light position */
	if ( obj->item_type == ITEM_LIGHT )
	{
		if ( !remove_obj( ch, WEAR_LIGHT, fReplace ) )
			return;
		if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
			act( AT_ACTION, "$n holds $p as a light.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "You hold $p as your light.",  ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEAR_LIGHT );
		oprog_wear_trigger( ch, obj );
		return;
	}

	if ( bit == -1 )
	{
		if ( fReplace )
			send_to_char( "You can't wear, wield, or hold that.\n\r", ch );
		return;
	}

	switch ( 1 << bit )
	{
		default:
			bug( "wear_obj: uknown/unused item_wear bit %d", bit );
			if ( fReplace )
				send_to_char( "You can't wear, wield, or hold that.\n\r", ch );
			return;

		case ITEM_WEAR_FINGER:
			if ( get_eq_char( ch, WEAR_FINGER_L )
					&&   get_eq_char( ch, WEAR_FINGER_R )
					&&   !remove_obj( ch, WEAR_FINGER_L, fReplace )
					&&   !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
				return;

			if ( !get_eq_char( ch, WEAR_FINGER_L ) )
			{
				if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
				{
					act( AT_ACTION, "$n slips $s left finger into $p.",    ch, obj, NULL, TO_ROOM );
					act( AT_ACTION, "You slip your left finger into $p.",  ch, obj, NULL, TO_CHAR );
				}
				equip_char( ch, obj, WEAR_FINGER_L );
				oprog_wear_trigger( ch, obj );
				return;
			}

			if ( !get_eq_char( ch, WEAR_FINGER_R ) )
			{
				if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
				{
					act( AT_ACTION, "$n slips $s right finger into $p.",   ch, obj, NULL, TO_ROOM );
					act( AT_ACTION, "You slip your right finger into $p.", ch, obj, NULL, TO_CHAR );
				}
				equip_char( ch, obj, WEAR_FINGER_R );
				oprog_wear_trigger( ch, obj );
				return;
			}

			bug( "Wear_obj: no free finger.", 0 );
			send_to_char( "You already wear something on both fingers.\n\r", ch );
			return;

		case ITEM_WEAR_NECK:
			if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
					&&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
					&&   !remove_obj( ch, WEAR_NECK_1, fReplace )
					&&   !remove_obj( ch, WEAR_NECK_2, fReplace ) )
				return;

			if ( !get_eq_char( ch, WEAR_NECK_1 ) )
			{
				if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
				{
					act( AT_ACTION, "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
					act( AT_ACTION, "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
				}
				equip_char( ch, obj, WEAR_NECK_1 );
				oprog_wear_trigger( ch, obj );
				return;
			}

			if ( !get_eq_char( ch, WEAR_NECK_2 ) )
			{
				if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
				{
					act( AT_ACTION, "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
					act( AT_ACTION, "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
				}
				equip_char( ch, obj, WEAR_NECK_2 );
				oprog_wear_trigger( ch, obj );
				return;
			}

			bug( "Wear_obj: no free neck.", 0 );
			send_to_char( "You already wear two neck items.\n\r", ch );
			return;

		case ITEM_WEAR_BODY:
			/*
			   if ( !remove_obj( ch, WEAR_BODY, fReplace ) )
			   return;
			 */
			if ( !can_layer( ch, obj, WEAR_BODY ) )
			{
				send_to_char( "It won't fit overtop of what you're already wearing.\n\r", ch );
				return;
			}
			if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				act( AT_ACTION, "$n fits $p on $s chest.",   ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "You fit $p on your chest.", ch, obj, NULL, TO_CHAR );
			}
			equip_char( ch, obj, WEAR_BODY );
			oprog_wear_trigger( ch, obj );
			return;

		case ITEM_WEAR_HEAD:
			if ( !remove_obj( ch, WEAR_HEAD, fReplace ) )
				return;
			if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				act( AT_ACTION, "$n dons $p upon $s head.",   ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "You don $p upon your head.", ch, obj, NULL, TO_CHAR );
			}
			equip_char( ch, obj, WEAR_HEAD );
			oprog_wear_trigger( ch, obj );
			return;

		case ITEM_WEAR_CAPE:
			if ( !remove_obj( ch, WEAR_CAPE, fReplace ) )
				return;
			if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				act( AT_ACTION, "$n dons $p.",   ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "You don $p.", ch, obj, NULL, TO_CHAR );
			}
			equip_char( ch, obj, WEAR_CAPE);
			oprog_wear_trigger( ch, obj );
			return;

		case ITEM_WEAR_QUIVER: /* KSILYAN */
			if ( !remove_obj( ch, WEAR_QUIVER, fReplace ) )
				return;
			if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				act( AT_ACTION, "$n slings $p over $s shoulder.",   ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "You sling $p over your shoulder.", ch, obj, NULL, TO_CHAR );
			}
			equip_char( ch, obj, WEAR_QUIVER);
			oprog_wear_trigger( ch, obj );
			return;


		case ITEM_WEAR_SCABBARD:
			if ( get_eq_char( ch, WEAR_SCABBARD_1 ) != NULL
					&&   get_eq_char( ch, WEAR_SCABBARD_2 ) != NULL
					&&   !remove_obj( ch, WEAR_SCABBARD_1, fReplace )
					&&   !remove_obj( ch, WEAR_SCABBARD_2, fReplace ) )
				return;

			if ( !get_eq_char( ch, WEAR_SCABBARD_1 ) )
			{
				if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
				{
					act( AT_ACTION, "$n straps on $p.",   ch, obj, NULL, TO_ROOM );
					act( AT_ACTION, "You strap on $p.", ch, obj, NULL, TO_CHAR );
				}
				equip_char( ch, obj, WEAR_SCABBARD_1 );
				oprog_wear_trigger( ch, obj );
				return;
			}

			if ( !get_eq_char( ch, WEAR_SCABBARD_2 ) )
			{
				if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
				{
					act( AT_ACTION, "$n straps on $p.",   ch, obj, NULL, TO_ROOM );
					act( AT_ACTION, "You strap on $p.", ch, obj, NULL, TO_CHAR );
				}
				equip_char( ch, obj, WEAR_SCABBARD_2 );
				oprog_wear_trigger( ch, obj );
				return;
			}

			bug( "Wear_obj: no free scabbard.", 0 );
			send_to_char( "You already wear two scabbards.\n\r", ch );
			return;

		case ITEM_WEAR_EARS:
			if ( !remove_obj( ch, WEAR_EARS, fReplace ) )
				return;
			if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				act( AT_ACTION, "$n wears $p on $s ears.",   ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "You wear $p on your ears.", ch, obj, NULL, TO_CHAR );
			}
			equip_char( ch, obj, WEAR_EARS );
			oprog_wear_trigger( ch, obj );
			return;

		case ITEM_WEAR_LEGS:
			if ( !can_layer( ch, obj, WEAR_LEGS ) )
			{
				send_to_char( "It won't fit overtop of what you're already wearing.\n\r", ch );
				return;
			}
			if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				act( AT_ACTION, "$n slips into $p.",   ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "You slip into $p.", ch, obj, NULL, TO_CHAR );
			}
			equip_char( ch, obj, WEAR_LEGS );
			oprog_wear_trigger( ch, obj );
			return;

		case ITEM_WEAR_FEET:
			if ( !can_layer( ch, obj, WEAR_FEET ) )
			{
				send_to_char( "It won't fit overtop of what you're already wearing.\n\r", ch );
				return;
			}
			if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				act( AT_ACTION, "$n wears $p on $s feet.",   ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "You wear $p on your feet.", ch, obj, NULL, TO_CHAR );
			}
			equip_char( ch, obj, WEAR_FEET );
			oprog_wear_trigger( ch, obj );
			return;

		case ITEM_WEAR_HANDS:
			if ( !can_layer( ch, obj, WEAR_HANDS ) )
			{
				send_to_char( "It won't fit overtop of what you're already wearing.\n\r", ch );
				return;
			}
			if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				act( AT_ACTION, "$n wears $p on $s hands.",   ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "You wear $p on your hands.", ch, obj, NULL, TO_CHAR );
			}
			equip_char( ch, obj, WEAR_HANDS );
			oprog_wear_trigger( ch, obj );
			return;

		case ITEM_WEAR_ARMS:
			if ( !can_layer( ch, obj, WEAR_ARMS ) )
			{
				send_to_char( "It won't fit overtop of what you're already wearing.\n\r", ch );
				return;
			}
			if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				act( AT_ACTION, "$n wears $p on $s arms.",   ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "You wear $p on your arms.", ch, obj, NULL, TO_CHAR );
			}
			equip_char( ch, obj, WEAR_ARMS );
			oprog_wear_trigger( ch, obj );
			return;

		case ITEM_WEAR_SHOULDERS:
			if ( !can_layer(ch, obj, WEAR_SHOULDERS) ) {
				send_to_char("It won't fit overtop of what you're already wearing.\r\n", ch);
				return;
			}

			if ( !oprog_use_trigger(ch, obj, NULL, NULL, NULL)) {
				act(AT_ACTION, "$n wears $p on $s shoulders.", ch, obj, NULL, TO_ROOM);
				act(AT_ACTION, "You wear $p on your shoulders.", ch, obj, NULL, TO_CHAR);
			}

			equip_char(ch, obj, WEAR_SHOULDERS);
			oprog_wear_trigger(ch, obj);
			return;

		case ITEM_WEAR_ABOUT:
			if ( !can_layer( ch, obj, WEAR_ABOUT ) )
			{
				send_to_char( "It won't fit overtop of what you're already wearing.\n\r", ch );
				return;
			}
			if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				act( AT_ACTION, "$n wears $p about $s body.",   ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "You wear $p about your body.", ch, obj, NULL, TO_CHAR );
			}
			equip_char( ch, obj, WEAR_ABOUT );
			oprog_wear_trigger( ch, obj );
			return;

		case ITEM_WEAR_WAIST:
			if ( !can_layer( ch, obj, WEAR_WAIST ) )
			{
				send_to_char( "It won't fit overtop of what you're already wearing.\n\r", ch );
				return;
			}
			if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				act( AT_ACTION, "$n wears $p about $s waist.",   ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "You wear $p about your waist.", ch, obj, NULL, TO_CHAR );
			}
			equip_char( ch, obj, WEAR_WAIST );
			oprog_wear_trigger( ch, obj );
			return;

		case ITEM_WEAR_WRIST:
			if ( get_eq_char( ch, WEAR_WRIST_L )
					&&   get_eq_char( ch, WEAR_WRIST_R )
					&&   !remove_obj( ch, WEAR_WRIST_L, fReplace )
					&&   !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
				return;

			if ( !get_eq_char( ch, WEAR_WRIST_L ) )
			{
				if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
				{
					act( AT_ACTION, "$n fits $p around $s left wrist.",
							ch, obj, NULL, TO_ROOM );
					act( AT_ACTION, "You fit $p around your left wrist.",
							ch, obj, NULL, TO_CHAR );
				}
				equip_char( ch, obj, WEAR_WRIST_L );
				oprog_wear_trigger( ch, obj );
				return;
			}

			if ( !get_eq_char( ch, WEAR_WRIST_R ) )
			{
				if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
				{
					act( AT_ACTION, "$n fits $p around $s right wrist.",
							ch, obj, NULL, TO_ROOM );
					act( AT_ACTION, "You fit $p around your right wrist.",
							ch, obj, NULL, TO_CHAR );
				}
				equip_char( ch, obj, WEAR_WRIST_R );
				oprog_wear_trigger( ch, obj );
				return;
			}

			bug( "Wear_obj: no free wrist.", 0 );
			send_to_char( "You already wear two wrist items.\n\r", ch );
			return;

		case ITEM_WEAR_SHIELD:
			if ( get_eq_char( ch, WEAR_DUAL_WIELD ) )
			{
				send_to_char( "You can't use a shield AND two weapons!\n\r", ch );
				return;
			}
			if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
				return;
			if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				act( AT_ACTION, "$n uses $p as a shield.", ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "You use $p as a shield.", ch, obj, NULL, TO_CHAR );
			}
			equip_char( ch, obj, WEAR_SHIELD );
			oprog_wear_trigger( ch, obj );
			return;

		case ITEM_WIELD:
			if ( (tmpobj  = get_eq_char( ch, WEAR_WIELD )) != NULL
					&&   !could_dual(ch) )
			{
				send_to_char( "You're already wielding something.\n\r", ch );
				return;
			}

			if ( tmpobj )
			{
				if ( can_dual(ch) )
				{
					if ( get_obj_weight( obj ) + get_obj_weight( tmpobj ) > str_app[ch->getStr()].wield )
					{
						send_to_char( "It is too heavy for you to wield.\n\r", ch );
						return;
					}
					if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
					{
						if ( obj->wear_loc == WEAR_SHEATHED_1 || obj->wear_loc == WEAR_SHEATHED_2 )
						{
							act( AT_ACTION, "$n draws and dual-wields $p!", ch, obj, NULL, TO_ROOM );
							act( AT_ACTION, "You draw and dual-wield $p!", ch, obj, NULL, TO_CHAR );
						}
						else
						{
							act( AT_ACTION, "$n dual-wields $p.", ch, obj, NULL, TO_ROOM );
							act( AT_ACTION, "You dual-wield $p.", ch, obj, NULL, TO_CHAR );
						}
					}
					equip_char( ch, obj, WEAR_DUAL_WIELD );
					oprog_wear_trigger( ch, obj );
				}
				return;
			}

			if ( get_obj_weight( obj ) > str_app[ch->getStr()].wield )
			{
				send_to_char( "It is too heavy for you to wield.\n\r", ch );
				return;
			}

			if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				if ( obj->wear_loc == WEAR_SHEATHED_1 || obj->wear_loc == WEAR_SHEATHED_2 )
				{
					act( AT_ACTION, "$n draws and wields $p!", ch, obj, NULL, TO_ROOM );
					act( AT_ACTION, "You draw and wield $p!", ch, obj, NULL, TO_CHAR );
				}
				else
				{
					act( AT_ACTION, "$n wields $p.", ch, obj, NULL, TO_ROOM );
					act( AT_ACTION, "You wield $p.", ch, obj, NULL, TO_CHAR );
				}
			}
			equip_char( ch, obj, WEAR_WIELD );
			oprog_wear_trigger( ch, obj );
			return;

		case ITEM_HOLD:
			if ( get_eq_char( ch, WEAR_DUAL_WIELD ) )
			{
				send_to_char( "You cannot hold something AND two weapons!\n\r", ch );
				return;
			}
			if ( !remove_obj( ch, WEAR_HOLD, fReplace ) )
				return;
			if ( obj->item_type == ITEM_WAND
					|| obj->item_type == ITEM_STAFF
					|| obj->item_type == ITEM_FOOD
					|| obj->item_type == ITEM_PILL
					|| obj->item_type == ITEM_POTION
					|| obj->item_type == ITEM_SCROLL
					|| obj->item_type == ITEM_DRINK_CON
					|| obj->item_type == ITEM_BLOOD
					|| obj->item_type == ITEM_PIPE
					|| obj->item_type == ITEM_HERB
					|| obj->item_type == ITEM_KEY
					|| !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
			{
				act( AT_ACTION, "$n holds $p in $s hands.",   ch, obj, NULL, TO_ROOM );
				act( AT_ACTION, "You hold $p in your hands.", ch, obj, NULL, TO_CHAR );
			}
			equip_char( ch, obj, WEAR_HOLD );
			oprog_wear_trigger( ch, obj );
			return;
	}
}


void do_wear(CHAR_DATA *ch, const char* argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	sh_int wear_bit;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	if ( (!str_cmp(arg2, "on")  || !str_cmp(arg2, "upon") || !str_cmp(arg2, "around"))
	     &&   argument[0] != '\0' )
		argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' )
	{
		send_to_char( "Wear, wield, or hold what?\n\r", ch );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	if ( !str_cmp( arg1, "all" ) )
	{
		OBJ_DATA *obj_next;

		for ( obj = ch->first_carrying; obj; obj = obj_next )
		{
			obj_next = obj->next_content;
			if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
				wear_obj( ch, obj, FALSE, -1 );
		}
		return;
	}
	else
	{
		if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
		{
			send_to_char( "You do not have that item.\n\r", ch );
			return;
		}
		if ( arg2[0] != '\0' )
			wear_bit = get_wflag(arg2);
		else
			wear_bit = -1;
		wear_obj( ch, obj, TRUE, wear_bit );
	}

	return;
}



void do_remove(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj, *obj_next;


    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Remove what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

   if ( !str_cmp( arg, "all" ) )  /* SB Remove all */
    {
      for ( obj = ch->first_carrying; obj != NULL ; obj = obj_next )
      {
        obj_next = obj->next_content;
        if ( obj->wear_loc != WEAR_NONE && can_see_obj ( ch, obj ) )
          remove_obj ( ch, obj->wear_loc, TRUE );
      }
      return;
    }

    if ( ( obj = get_obj_wear( ch, arg ) ) == NULL )
    {
	send_to_char( "You are not using that item.\n\r", ch );
	return;
    }
    if ( (obj_next=get_eq_char(ch, obj->wear_loc)) != obj && obj_next )
    {
	act( AT_PLAIN, "You must remove $p first.", ch, obj_next, NULL, TO_CHAR );
	return;
    }

	if ( obj->wear_loc == WEAR_SHEATHED_1 || obj->wear_loc == WEAR_SHEATHED_2 )
	{
		send_to_char( "You must draw your weapon before removing it.\n\r", ch);
		return;
	}

    remove_obj( ch, obj->wear_loc, TRUE );
    return;
}


void do_bury(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    bool shovel;
    sh_int move;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "What do you wish to bury?\n\r", ch );
        return;
    }

    if ( ms_find_obj(ch) )
        return;

    shovel = FALSE;
    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
      if ( obj->item_type == ITEM_SHOVEL )
      {
	  shovel = TRUE;
	  break;
      }

    obj = get_obj_list_rev( ch, arg, ch->GetInRoom()->last_content );
    if ( !obj )
    {
        send_to_char( "You can't find it.\n\r", ch );
        return;
    }

    separate_obj(obj);
    if ( !CAN_WEAR(obj, ITEM_TAKE) )
    {
	if ( !IS_OBJ_STAT( obj, ITEM_CLANCORPSE )
	|| IS_NPC(ch))
	   {
		act( AT_PLAIN, "You cannot bury $p.", ch, obj, 0, TO_CHAR );
        	return;
           }
    }

    switch( ch->GetInRoom()->sector_type )
    {
	case SECT_CITY:
	case SECT_INSIDE:
	    send_to_char( "The floor is too hard to dig through.\n\r", ch );
	    return;
	case SECT_WATER_SWIM:
	case SECT_WATER_NOSWIM:
	case SECT_UNDERWATER:
	    send_to_char( "You cannot bury something here.\n\r", ch );
	    return;
	case SECT_AIR:
	    send_to_char( "What?  In the air?!\n\r", ch );
	    return;
    }

    if ( obj->weight > (UMAX(5, (can_carry_w(ch) / 10)))
    &&  !shovel )
    {
	send_to_char( "You'd need a shovel to bury something that big.\n\r", ch );
	return;
    }

    move = (obj->weight * 50 * (shovel ? 1 : 5)) / UMAX(1, can_carry_w(ch));
    move = URANGE( 2, move, 1000 );
    if ( move > ch->move )
    {
	send_to_char( "You don't have the energy to bury something of that size.\n\r", ch );
	return;
    }
    ch->move -= move;
    if ( obj->item_type == ITEM_CORPSE_NPC
    ||   obj->item_type == ITEM_CORPSE_PC )
	adjust_favor( ch, 6, 1 );

    act( AT_ACTION, "You solemnly bury $p...", ch, obj, NULL, TO_CHAR );
    act( AT_ACTION, "$n solemnly buries $p...", ch, obj, NULL, TO_ROOM );
    SET_BIT( obj->extra_flags, ITEM_BURIED );
    save_buried_items(ch);
    ch->AddWait( URANGE( 10, move / 2, 100 ) );
    return;
}

void do_sacrifice(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char name[50];
	OBJ_DATA *obj;

	one_argument( argument, arg );

	if ( arg[0] == '\0' || ch->getName().ciEqual(arg) )
	{
		act( AT_ACTION, "$n offers $mself to $s deity, who graciously declines.",
				ch, NULL, NULL, TO_ROOM );
		send_to_char( "Your deity appreciates your offer and may accept it later.", ch );
		return;
	}

	if ( ms_find_obj(ch) )
		return;

	obj = get_obj_list_rev( ch, arg, ch->GetInRoom()->last_content );
	if ( !obj )
	{
		send_to_char( "You can't find it.\n\r", ch );
		return;
	}

	separate_obj(obj);
	if ( !CAN_WEAR(obj, ITEM_TAKE) ||
			(obj->item_type != ITEM_CORPSE_PC && obj->item_type != ITEM_CORPSE_NPC))
	{
		act( AT_PLAIN, "$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR );
		return;
	}

	if ( IS_SET(obj->extra_flags, ITEM_DONATION) ) {
		act(AT_PLAIN, "$p was donated by some kind soul, and you want to destroy it?", ch, obj, 0, TO_CHAR);
		return;
	}

	if ( !IS_NPC( ch ) && ch->pcdata->deity && ch->pcdata->deity->name_.length() > 0 )
	{
		strcpy( name, ch->pcdata->deity->name_.c_str() );
	}
	else if ( !IS_NPC( ch ) && IS_GUILDED(ch) && sysdata.guild_overseer[0] != '\0' )
	{
		strcpy( name, sysdata.guild_overseer );
	}
	else if ( !IS_NPC( ch ) && ch->pcdata->clan && ch->pcdata->clan->deity_.length() > 0 )
	{
		strcpy( name, ch->pcdata->clan->deity_.c_str() );
	}
	else
	{
		strcpy( name, "Tarin" );
	}
	ch->gold += 1;
	if ( obj->item_type == ITEM_CORPSE_NPC
			||   obj->item_type == ITEM_CORPSE_PC )
		adjust_favor( ch, 5, 1 );
	sprintf( buf, "%s gives you one gold coin for your sacrifice.\n\r", name );
	send_to_char( buf, ch );
	sprintf( buf, "$n sacrifices $p to %s.", name );
	act( AT_ACTION, buf, ch, obj, NULL, TO_ROOM );
	oprog_sac_trigger( ch, obj );
	if ( obj_extracted(obj) )
		return;
	if ( cur_obj == obj->serial )
		global_objcode = rOBJ_SACCED;
	extract_obj( obj, TRUE);
	return;
}

void do_brandish(CHAR_DATA *ch, const char* argument)
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *staff;
    ch_ret retcode;
    int sn;

    if ( ( staff = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( staff->item_type != ITEM_STAFF )
    {
	send_to_char( "You can brandish only with a staff.\n\r", ch );
	return;
    }

    if ( ( sn = staff->value[3] ) < 0
    ||   sn >= top_sn
    ||   skill_table[sn]->spell_fun == NULL )
    {
	bug( "Do_brandish: bad sn %d.", sn );
	return;
    }

    ch->AddWait( 2 * PULSE_VIOLENCE );

    if ( staff->value[2] > 0 )
    {
      if ( !oprog_use_trigger( ch, staff, NULL, NULL, NULL ) )
      {
        act( AT_MAGIC, "$n brandishes $p.", ch, staff, NULL, TO_ROOM );
        act( AT_MAGIC, "You brandish $p.",  ch, staff, NULL, TO_CHAR );
      }
	for ( vch = ch->GetInRoom()->first_person; vch; vch = vch_next )
	{
	    vch_next	= vch->next_in_room;
            if ( !IS_NPC( vch ) && IS_SET( vch->act, PLR_WIZINVIS )
                  && vch->pcdata->wizinvis >= LEVEL_IMMORTAL )
                continue;
            else
	    switch ( skill_table[sn]->target )
	    {
	    default:
		bug( "Do_brandish: bad target for sn %d.", sn );
		return;

	    case TAR_IGNORE:
		if ( vch != ch )
		    continue;
		break;

	    case TAR_CHAR_OFFENSIVE:
		if ( IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch) )
		    continue;
		break;

	    case TAR_CHAR_DEFENSIVE:
		if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
		    continue;
		break;

	    case TAR_CHAR_SELF:
		if ( vch != ch )
		    continue;
		break;
	    }

	    retcode = obj_cast_spell( staff->value[3], staff->value[0], ch, vch, NULL );
	    if ( retcode == rCHAR_DIED || retcode == rBOTH_DIED )
	    {
		bug( "do_brandish: char died", 0 );
		return;
	    }
	}
    }

    if ( --staff->value[2] <= 0 )
    {
	act( AT_MAGIC, "$p blazes bright and vanishes from $n's hands!", ch, staff, NULL, TO_ROOM );
	act( AT_MAGIC, "$p blazes bright and is gone!", ch, staff, NULL, TO_CHAR );
	if ( staff->serial == cur_obj )
	  global_objcode = rOBJ_USED;
	extract_obj( staff, TRUE);
    }

    return;
}

/* copied from wiggy by matthew */
void do_behead(CHAR_DATA *ch, const char* argument)
{

	char arg1[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char name[MAX_STRING_LENGTH];
	char *bufptr;
	MOB_INDEX_DATA *pMobIndex;
	OBJ_DATA *obj;
	OBJ_DATA *head;
	OBJ_DATA *weapon;
	bool keep_going;
	int vnum;


	keep_going = FALSE;


	argument = one_argument( argument, arg1 );

	if ( arg1[0] == '\0' )
    {
		send_to_char( "behead what?\n\r", ch );
		return;
    }

	 /* 'get obj' */
	 obj = get_obj_list( ch, arg1, ch->GetInRoom()->first_content );

	if ( !obj )
	{
		act( AT_PLAIN, "I see no $T here.", ch, NULL, arg1, TO_CHAR );
		return;
	}

	/* check for corpse in the room */
	if ((!( obj->item_type == ITEM_CORPSE_PC)) && (!(obj->item_type == ITEM_CORPSE_NPC )))
	{

		send_to_char( "That isn't a corpse!\n\r", ch );
		return;
	}
	/* check for beheaded status by looking for "beheaded" in long desc */


	bufptr = one_argument( obj->shortDesc_.c_str(), name );
	bufptr = one_argument( bufptr, name );

	/* lets check 2nd  word of short desc to see if corpse has already been beheaded */

	if ( is_name("beheaded", obj->name_.c_str()))
	{
		send_to_char( "That corpse is already headless.\n\r", ch );
		return;
	}

/*check to see if head is one of the body parts available, since only mobs have body parts,
	only need to check against mobs
	*/

	if (obj->item_type == ITEM_CORPSE_NPC)
	{

		if ( (pMobIndex = get_mob_index((sh_int) abs(obj->cost) )) != NULL )
		{
			if ( !(HAS_BODYPART(pMobIndex, PART_HEAD)) && (obj->item_type == ITEM_CORPSE_NPC))
			{
				send_to_char( "That doesn't appear to have a head you can hack off.\n\r", ch );
				return;
			}
		}
		else
		{
			 act( AT_PLAIN, "You can't find a way to behead that\n\r", ch, NULL, NULL, TO_CHAR );
			return;

		}


	}

	separate_obj(obj);


    keep_going = FALSE;

    /* check to see if pc has a weapon to do the slicing*/

	for ( weapon = ch->first_carrying; weapon ; weapon = weapon->next_content )
	{
           if ( can_see_obj( ch, weapon )
           && ( weapon->item_type == ITEM_WEAPON)
           &&
           (
           (weapon->wear_loc == WEAR_WIELD)
           ||
           (weapon->wear_loc == WEAR_HOLD)
           ||
           (weapon->wear_loc == WEAR_DUAL_WIELD)
           )
              )
             {
                keep_going = TRUE;
               switch ( weapon->value[OBJECT_WEAPON_DAMAGEMESSAGE] )
               {
                case DAMAGE_MSG_STAB:/*   | stab     | dagger */
                    act( AT_ACTION, "$n slowly saws off the head of $p.", ch, obj, NULL, TO_ROOM );
		            act( AT_ACTION, "You stab away at the neck and finally behead $p.", ch, obj, NULL, TO_CHAR );

		            break;

                case DAMAGE_MSG_SWING:/*   | slash    | sword */
                   act( AT_ACTION, "$n slashes off the head of $p.", ch, obj, NULL, TO_ROOM );
		            act( AT_ACTION, "You behead $p with a mighty slash.", ch, obj, NULL, TO_CHAR );

		            break;

                case DAMAGE_MSG_WHIP:/*   | whip     | whip */
                    act( AT_ACTION, "$n whips at the neck of $p, yanks, and pops of it's head.", ch, obj, NULL, TO_ROOM );
		            act( AT_ACTION, "You yank off the head of $p.", ch, obj, NULL, TO_CHAR );

		            break;
                case DAMAGE_MSG_CLAW:/*   | claw     | claw */
                    act( AT_ACTION, "$n claws the head off of $p.", ch, obj, NULL, TO_ROOM );
		            act( AT_ACTION, "You claw off the head of $p.", ch, obj, NULL, TO_CHAR );

		            break;
                case DAMAGE_MSG_POUND:/*   | pound    | club/hammer */
                    act( AT_ACTION, "$n smashes the head off of $p.", ch, obj, NULL, TO_ROOM );
		            act( AT_ACTION, "You smash off the head of $p.", ch, obj, NULL, TO_CHAR );
		            break;
                case DAMAGE_MSG_CRUSH:/*   | crush    | club/hammer */
                    act( AT_ACTION, "$n crushes the neck of $p, severing the head.", ch, obj, NULL, TO_ROOM );
		            act( AT_ACTION, "You behead of $p.", ch, obj, NULL, TO_CHAR );
		            break;
                 case DAMAGE_MSG_BITE:/*   | bite     |  */
                    act( AT_ACTION, "$n bites off the head of $p.", ch, obj, NULL, TO_ROOM );
		            act( AT_ACTION, "You bite off the head of $p.", ch, obj, NULL, TO_CHAR );
		            break;

                default:
					act( AT_ACTION, "$n beheads the corpse of $p", ch, obj, NULL, TO_ROOM );
		            act( AT_ACTION, "You behead $p", ch, obj, NULL, TO_CHAR );
		            break;
               }
               break;
             }
	}

    if (!keep_going)
    {
    	act( AT_PLAIN, "You are not wielding the proper weapon for that.", ch, NULL, NULL, TO_CHAR );
    	return;
    }

	sprintf(buf, "beheaded %s", obj->name_.c_str());
	obj->name_ = buf;

	sprintf( buf, "the beheaded corpse of %s", capitalize(strip_the(obj->shortDesc_.c_str())));
	obj->shortDesc_ = buf;


	sprintf( buf,"The beheaded corpse of %s lies here, bleeding profusely from the neck.",capitalize(strip_the(obj->shortDesc_.c_str())));
	obj->longDesc_ = buf;

	/* Drain the corpse of blood */

	obj->value[1] = 0;


	/* fish heads fish heads, rolly polly fish heads, let's make one */

	vnum = OBJ_VNUM_SEVERED_HEAD;

	head = create_object( get_obj_index( vnum ), 0 );

	/* Leave heads laying around for head quests
	obj->timer	= 60;
	*/

	/* rename the head */

	sprintf( buf, "head %s", strip_a(obj->name_.c_str()) );
	head->name_ = buf;

	sprintf( buf, "the head of %s", capitalize( strip_the(obj->shortDesc_.c_str()) ) );
	head->shortDesc_ = buf;

	sprintf( buf, "The head of %s lies here, staring at you", capitalize( strip_the(obj->shortDesc_.c_str()) ) );
	head->longDesc_ = buf;

	head->weight = (int) (obj->weight * .1);
	obj->weight = (int) (obj->weight * .9);

	head = obj_to_room( head, ch->GetInRoom() );

	return;
}


void do_zap(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *wand;
    OBJ_DATA *obj;
    ch_ret retcode;

    one_argument( argument, arg );
    if ( arg[0] == '\0' && !ch->GetVictim() )
    {
	send_to_char( "Zap whom or what?\n\r", ch );
	return;
    }

    if ( ( wand = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "You hold nothing in your hand.\n\r", ch );
	return;
    }

    if ( wand->item_type != ITEM_WAND )
    {
	send_to_char( "You can zap only with a wand.\n\r", ch );
	return;
    }

    obj = NULL;
    if ( arg[0] == '\0' )
    {
	if ( ch->GetVictim() )
	{
	    victim = ch->GetVictim();
	}
	else
	{
	    send_to_char( "Zap whom or what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    ch->AddWait( 1 * PULSE_VIOLENCE );

    if ( wand->value[2] > 0 )
    {
	if ( victim )
	{
          if ( !oprog_use_trigger( ch, wand, victim, NULL, NULL ) )
          {
	    act( AT_MAGIC, "$n aims $p at $N.", ch, wand, victim, TO_ROOM );
	    act( AT_MAGIC, "You aim $p at $N.", ch, wand, victim, TO_CHAR );
          }
	}
	else
	{
          if ( !oprog_use_trigger( ch, wand, NULL, obj, NULL ) )
          {
	    act( AT_MAGIC, "$n aims $p at $P.", ch, wand, obj, TO_ROOM );
	    act( AT_MAGIC, "You aim $p at $P.", ch, wand, obj, TO_CHAR );
          }
	}

	retcode = obj_cast_spell( wand->value[3], wand->value[0], ch, victim, obj );
	if ( retcode == rCHAR_DIED || retcode == rBOTH_DIED )
	{
	   bug( "do_zap: char died", 0 );
	   return;
	}
    }

    if ( --wand->value[2] <= 0 )
    {
      act( AT_MAGIC, "$p explodes into fragments.", ch, wand, NULL, TO_ROOM );
      act( AT_MAGIC, "$p explodes into fragments.", ch, wand, NULL, TO_CHAR );
      if ( wand->serial == cur_obj )
        global_objcode = rOBJ_USED;
      extract_obj( wand, TRUE );
    }

    return;
}


/*
 * Save items in a store items room
 * Copied almost exactly from save_clan_storeroom :)
 */
void save_storeitems_room( CHAR_DATA *ch )
{
    FILE *fp;
    char filename[256];
    sh_int templvl;
    OBJ_DATA *contents;

    if ( !ch )
    {
	bug ("save_storeitems_room: Null ch pointer!", 0);
	return;
    }

    sprintf( filename, "%s%d.vault", STOREITEMS_DIR, ch->GetInRoom()->vnum );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
	bug( "save_storeitems_room: fopen", 0 );
	perror( filename );
    }
    else
    {
		fprintf(fp, "#VNUM %d\n\n", ch->GetInRoom()->vnum);

	templvl = ch->level;
	ch->level = LEVEL_HERO_MAX;		/* make sure EQ doesn't get lost */
	contents = ch->GetInRoom()->last_content;

	if (contents)
		  fwrite_obj(ch, contents, fp, 0, OS_CARRY );
		fprintf( fp, "#END\n" );
		ch->level = templvl;
		fclose( fp );
		return;
    }

    save_buried_items(ch);

    return;
}

/* save buried items -- almost exactly like save_storeitems_room */
void save_buried_items( CHAR_DATA *ch )
{
	FILE *fp;
	char filename[256];
	sh_int templvl = ch->level;
	OBJ_DATA *contents;

	if ( !ch )
	{
		bug ("save_buried_items: Null ch pointer!", 0);
		return;
	}

	sprintf( filename, "%s%d.vault", BURIEDITEMS_DIR, ch->GetInRoom()->vnum );
	if ( ( fp = fopen( filename, "w" ) ) == NULL )
	{
		bug( "save_buried_items: fopen", 0 );
		perror( filename );
	}
	else
	{
		fprintf(fp, "#VNUM %d\n\n", ch->GetInRoom()->vnum);

		templvl = ch->level;
		ch->level = LEVEL_HERO_MAX; 	/* make sure EQ doesn't get lost */
		contents = ch->GetInRoom()->last_content;

		if (contents)
			fwrite_obj(ch, contents, fp, 0, OS_BURIED );
		fprintf( fp, "#END\n" );
		ch->level = templvl;
		fclose( fp );
		return;
	}
	ch->level = templvl;
	return;
}


/*
 * Save items in a clan storage room			-Scryn & Thoric
 */
void save_clan_storeroom( CHAR_DATA *ch, ClanData *clan )
{
	FILE *fp;
	char filename[256];
	sh_int templvl;
	OBJ_DATA *contents;

	if ( !clan )
	{
		bug( "save_clan_storeroom: Null clan pointer!", 0 );
		return;
	}

	if ( !ch )
	{
		bug ("save_clan_storeroom: Null ch pointer!", 0);
		return;
	}

	sprintf( filename, "%s%s.vault", CLAN_DIR, clan->filename_.c_str() );
	if ( ( fp = fopen( filename, "w" ) ) == NULL )
	{
		bug( "save_clan_storeroom: fopen", 0 );
		perror( filename );
	}
	else
	{
		templvl = ch->level;
		ch->level = LEVEL_HERO_MAX;		/* make sure EQ doesn't get lost */
		contents = ch->GetInRoom()->last_content;
		if (contents)
			fwrite_obj(ch, contents, fp, 0, OS_CARRY );
		fprintf( fp, "#END\n" );
		ch->level = templvl;
		fclose( fp );
		return;
	}
	return;
}

/* put an item on auction, or see the stats on the current item or bet
 *
 * You have to be in an auction room (rflag ROOM_AUCTIONROOM) to
 * participate in the auction. One can see the stats of an auction
 * from anywhere, though. An immortal doesn't have to be in an
 * auctionroom to stop the ongoing auction.
 *
 * 08.08.2000 -- Warp
 */
void do_auction(CHAR_DATA *ch, const char* argument)
{
	OBJ_DATA *obj;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];

	argument = one_argument(argument, arg1);

	if(IS_NPC(ch)) /* NPC can be extracted at any time and thus can't auction! */
		return;

	if(arg1[0] == '\0')
	{
		if(auction->item != NULL)
		{
			obj = auction->item;

			set_char_color(AT_LBLUE, ch);
			ch_printf(ch, "Object '%s' is %s.\r\n", obj->name_.c_str(),
					aoran(itemtype_number_to_name(obj->item_type)));

			if((obj->item_type == ITEM_CONTAINER) &&(obj->first_content))
			{
				set_char_color(AT_OBJECT, ch);
				send_to_char("Contents:\r\n", ch);
				show_list_to_char(obj->first_content, ch, TRUE, FALSE);
			}

			if(auction->bet > 0)
				sprintf(buf, "Current bid on this item is %d gold coins.\n\r",auction->bet);
			else
				sprintf(buf, "No bids on this item have been received.\n\r");
			set_char_color(AT_BLUE, ch);
			send_to_char(buf,ch);

			if(IS_IMMORTAL(ch))
			{
				sprintf(buf, "Seller: %s.  Bidder: %s.  Round: %d.\n\r",
				             auction->seller->getShort().c_str(), auction->buyer->getShort().c_str(),
				             (auction->going + 1));
				send_to_char(buf, ch);
				sprintf(buf, "Time left in round: %d.\n\r", auction->pulse);
				send_to_char(buf, ch);
			}
			return;
		}
		else
		{
			set_char_color(AT_LBLUE, ch);
			send_to_char("\n\rThere is nothing being auctioned right now. What would you like to auction?\n\r", ch);
			return;
		}
	}

	if(IS_IMMORTAL(ch) && !str_cmp(arg1,"stop"))
	{
		if(auction->item == NULL)
		{
			send_to_char("There is no auction to stop.\n\r",ch);
			return;
		}
		else
		{
			/* stop the auction */
			set_char_color(AT_LBLUE, ch);
			sprintf(buf,"Sale of %s has been stopped by an Immortal.\n\r",
			            auction->item->shortDesc_.c_str());
			talk_auction(buf);
			obj_to_char(auction->item, auction->seller);
			if(IS_SET(sysdata.save_flags, SV_AUCTION))
				save_char_obj(auction->seller);
			auction->item = NULL;
			if(auction->buyer != NULL && auction->buyer != auction->seller)
			{
				/* return money to the buyer */
				auction->buyer->gold += auction->bet;
				send_to_char("Your bid has been returned because an Immortal stopped the auction.\n\r",auction->buyer);
			}
			return;
		}
	}

	if(!str_cmp(arg1,"bid"))
	{
		/* One has to be in an auctionroom to participate */
		if(!char_in_auctionroom(ch))
		{
			send_to_char("This is not a suitable site for an auction.\n\r", ch);
			return;
		}

		if(auction->item != NULL)
		{
			int newbet;

			if(ch == auction->seller)
			{
				send_to_char("You can't bid on your own item!\n\r", ch);
				return;
			}

			/* make - perhaps - a bet now */
			if(argument[0] == '\0')
			{
				send_to_char("Bid how much?\n\r",ch);
				return;
			}

			newbet = parsebet(auction->bet, argument);

			if(newbet < auction->starting)
			{
				send_to_char("You must place a bid that is higher than the starting bet.\n\r", ch);
				return;
			}

			/* to avoid slow auction, use a bigger amount than 100 if the bet
			 * is higher up - changed to 10000 for our high economy
			 */

			if(newbet < (auction->bet + AUCTION_MINIMUM))
			{
				send_to_char("You must at least bid 10 coins over the current bid.\n\r",ch);
				return;
			}

			if(newbet > ch->gold)
			{
				send_to_char("You don't have that much money!\n\r",ch);
				return;
			}

			if(newbet > 2000000000)
			{
				send_to_char("You can't bid over 2 billion coins.\n\r", ch);
				return;
			}

			/* the actual bet is OK! */

			/* return the gold to the last buyer, if one exists */
			if (auction->buyer != NULL && auction->buyer != auction->seller)
				auction->buyer->gold += auction->bet;

			ch->gold -= newbet; /* substract the gold - important :) */
			if (IS_SET(sysdata.save_flags, SV_AUCTION))
			{
				save_char_obj(ch);
			}
			auction->buyer = ch;
			auction->bet   = newbet;
			auction->going = 0;
			auction->pulse = PULSE_AUCTION; /* start the auction over again */

			sprintf(buf, "A bid of %d gold has been received on %s.\n\r", newbet,auction->item->shortDesc_.c_str());
			talk_auction(buf);
			return;
		}
		else
		{
			send_to_char("There isn't anything being auctioned right now.\n\r",ch);
			return;
		}
	}

	/* finally... */
	if(ms_find_obj(ch))
		return;

	obj = get_obj_carry(ch, arg1); /* does char have the item ? */

	if(obj == NULL)
	{
		send_to_char("You aren't carrying that.\n\r",ch);
		return;
	}

	if(IS_SET(obj->extra_flags, ITEM_DONATION))
	{
		send_to_char("Someone donated that item out of the goodness of their heart, and you want to make money off of it?", ch);
		return;
	}

	if(obj->timer > 0)
	{
		send_to_char("You can't auction objects that are decaying.\n\r", ch);
		return;
	}

	if(IS_SET(obj->extra_flags, ITEM_DEITY))
	{
		send_to_char("Your deity gave you that...\r\n", ch);
		return;
	}

	argument = one_argument(argument, arg2);

	if(arg2[0] == '\0')
	{
		auction->starting = 0;
		strcpy(arg2, "0");
	}

	if(!is_number(arg2))
	{
		send_to_char("You must input a number at which to start the auction.\n\r", ch);
		return;
	}

	if(atoi(arg2) < 0)
	{
		send_to_char("You can't auction something for less than 0 gold!\n\r", ch);
		return;
	}

	if(auction->item == NULL)
	{
		if(char_in_auctionroom(ch))
		{
			switch(obj->item_type)
			{
				default:
					act(AT_TELL, "You cannot auction $Ts.",ch, NULL, itemtype_number_to_name(obj->item_type), TO_CHAR);
					return;

				/* insert any more item types here...
				 * items with a timer MAY NOT BE AUCTIONED!
				 */
				case ITEM_LIGHT:
				case ITEM_TREASURE:
				case ITEM_POTION:
				case ITEM_CONTAINER:
				case ITEM_DRINK_CON:
				case ITEM_FOOD:
				case ITEM_PEN:
				case ITEM_BOAT:
				case ITEM_PILL:
				case ITEM_PIPE:
				case ITEM_HERB_CON:
				case ITEM_INCENSE:
				case ITEM_FIRE:
				case ITEM_MAP:
				case ITEM_BOOK:
				case ITEM_MATCH:
				case ITEM_HERB:
				case ITEM_WEAPON:
				case ITEM_ARMOR:
				case ITEM_STAFF:
				case ITEM_WAND:
				case ITEM_SCROLL:
					separate_obj(obj);
					obj_from_char(obj);
					if(IS_SET(sysdata.save_flags, SV_AUCTION))
						save_char_obj(ch);
					auction->item = obj;
					auction->bet = 0;
					auction->buyer = ch;
					auction->seller = ch;
					auction->pulse = PULSE_AUCTION;
					auction->going = 0;
					auction->starting = atoi(arg2);

					if(auction->starting > 0)
						auction->bet = auction->starting;

					sprintf(buf, "A new item is being auctioned: %s at %d gold.\n\r", obj->shortDesc_.c_str(), auction->starting);
					talk_auction(buf);

					return;
			}
		}
		else
		{
			send_to_char("This is not a suitable site to conduct an auction.\n\r", ch);
			return;
		}
	}
	else
	{
		act(AT_TELL, "Try again later - $p is being auctioned right now!",ch,auction->item,NULL,TO_CHAR);
		ch->AddWait( (int) (1.5 * PULSE_VIOLENCE));
		return;
	}
}


/* Make objects in rooms that are nofloor fall - Scryn 1/23/96 */

void obj_fall( OBJ_DATA *obj, bool through )
{
    ExitData *pexit;
    ROOM_INDEX_DATA *to_room;
    static int fall_count;
    char buf[MAX_STRING_LENGTH];
    static bool is_falling; /* Stop loops from the call to obj_to_room()  -- Altrag */

    if ( !obj->GetInRoom() || is_falling )
    	return;

    if (fall_count > 30)
    {
    	bug( "object falling in loop more than 30 times", 0 );
	extract_obj(obj, TRUE);
    	fall_count = 0;
	return;
     }

     if ( IS_SET( obj->GetInRoom()->room_flags, ROOM_NOFLOOR )
     &&   CAN_GO( obj, DIR_DOWN )
     &&   !IS_OBJ_STAT( obj, ITEM_MAGIC ) )
     {

	pexit = get_exit( obj->GetInRoom(), DIR_DOWN );
    	to_room = pexit->to_room;

    	if (through)
	  fall_count++;
	else
	  fall_count = 0;

	if (obj->GetInRoom() == to_room)
	{
	    sprintf(buf, "Object falling into same room, room %s",
		vnum_to_dotted(to_room->vnum));
	    bug( buf, 0 );
	    extract_obj( obj, TRUE );
            return;
	}

	if (obj->GetInRoom()->first_person)
	{
	  	act( AT_PLAIN, "$p falls far below...",
			obj->GetInRoom()->first_person, obj, NULL, TO_ROOM );
		act( AT_PLAIN, "$p falls far below...",
			obj->GetInRoom()->first_person, obj, NULL, TO_CHAR );
	}
	obj_from_room( obj );
	is_falling = TRUE;
	obj = obj_to_room( obj, to_room );
	is_falling = FALSE;

    if ( !obj ) {
        return;
    }

	if (obj->GetInRoom()->first_person)
	{
	  	act( AT_PLAIN, "$p falls from above...",
			obj->GetInRoom()->first_person, obj, NULL, TO_ROOM );
		act( AT_PLAIN, "$p falls from above...",
			obj->GetInRoom()->first_person, obj, NULL, TO_CHAR );
	}

 	if (!IS_SET( obj->GetInRoom()->room_flags, ROOM_NOFLOOR ) && through )
	{
/*		int dam = (int)9.81*sqrt(fall_count*2/9.81)*obj->weight/2;
*/		int dam = fall_count*obj->weight/2;
		/* Damage players */
		if ( obj->GetInRoom()->first_person && number_percent() > 15 )
		{
			CHAR_DATA *rch;
			CHAR_DATA *vch = NULL;
			int chcnt = 0;

			for ( rch = obj->GetInRoom()->first_person; rch;
				rch = rch->next_in_room, chcnt++ )
				if ( number_range( 0, chcnt ) == 0 )
					vch = rch;
			act( AT_WHITE, "$p falls on $n!", vch, obj, NULL, TO_ROOM );
			act( AT_WHITE, "$p falls on you!", vch, obj, NULL, TO_CHAR );
			damage( vch, vch, dam*vch->level, TYPE_UNDEFINED ,0);
		}
    	/* Damage objects */
	    switch( obj->item_type )
     	    {
	     	case ITEM_WEAPON:
		case ITEM_ARMOR:
		    if ( (obj->value[0] - dam) <= 0 )
 		    {
   			if (obj->GetInRoom()->first_person)
			{
			act( AT_PLAIN, "$p is destroyed by the fall!",
				obj->GetInRoom()->first_person, obj, NULL, TO_ROOM );
			act( AT_PLAIN, "$p is destroyed by the fall!",
				obj->GetInRoom()->first_person, obj, NULL, TO_CHAR );
			}
			make_scraps(obj);
	 	    }
		    else
	           	obj->value[0] -= dam;
		    break;
		default:
		    if ( (dam*15) > get_obj_resistance(obj) )
		    {
	              if (obj->GetInRoom()->first_person)
		      {
 			    act( AT_PLAIN, "$p is destroyed by the fall!",
			    	obj->GetInRoom()->first_person, obj, NULL, TO_ROOM );
			    act( AT_PLAIN, "$p is destroyed by the fall!",
		    		obj->GetInRoom()->first_person, obj, NULL, TO_CHAR );
		      }
		      make_scraps(obj);
		    }
		    break;
	    }
     	}
     	obj_fall( obj, TRUE );
    }
    return;
}

void do_donate(CHAR_DATA *ch, const char* argument) {
	char arg[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	int vnum;
	ROOM_INDEX_DATA *pNew;
	ROOM_INDEX_DATA *pOld;

	argument = one_argument(argument, arg);

	if ( arg[0] == '\0' ) {
		send_to_char("Donate what?\r\n", ch);
		return;
	}

	if ( ms_find_obj(ch) ) {
		return;
	}

	if ( (obj = get_obj_carry(ch, arg)) == NULL ) {
		send_to_char("That's very generous of you, but you don't have it...\r\n", ch);
		return;
	}

	if (is_playbuilding(ch))
	{
		ch_printf(ch,"You can't do that!\n");
		return;
	}

	if ( !can_drop_obj(ch, obj) )
	{
		send_to_char("You can't even let go of it.\r\n", ch);
		return;
	}

	switch ( obj->item_type ) {
	case ITEM_WEAPON:
	case ITEM_SPIKE:
	case ITEM_PROJECTILE:
	case ITEM_QUIVER:
		vnum = ROOM_VNUM_DONATION_WEAPONS;
		break;
	case ITEM_ARMOR:
		vnum = ROOM_VNUM_DONATION_ARMOR;
		break;
	case ITEM_SCROLL:
	case ITEM_WAND:
	case ITEM_STAFF:
	case ITEM_POTION:
	case ITEM_SALVE:
		vnum = ROOM_VNUM_DONATION_MAGICAL;
		break;
	case ITEM_FOOD:
	case ITEM_DRINK_CON:
	case ITEM_MEAT:
		vnum = ROOM_VNUM_DONATION_FOOD;
		break;
	case ITEM_TRASH:
		vnum = ROOM_VNUM_DONATION_TRASH;
		break;
	default:
		vnum = ROOM_VNUM_DONATION_MISC;
		break;
	}

	if ( !(pNew = get_room_index(vnum)) ) {
		send_to_char("You can't donate that; for some reason the donation room disappeard!\r\n", ch);
		return;
	}

	separate_obj(obj);

	if ( IS_IMMORTAL(ch) )
	{
		sprintf(arg, "%s donate %s", ch->getName().c_str(), obj->name_.c_str());
		log_string_plus(arg, LOG_NORMAL, ch->level);
	}

	// moved this to up here to provide a message
	// even if the item is trashed.

	act(AT_ACTION, "$n grabs $p, shoves $s hand into the air, and pulls it back empty.",
		ch, obj, NULL, TO_ROOM);
	act(AT_ACTION, "You grab $p, shove your hand into the air, and pull it back empty.",
		ch, obj, NULL, TO_CHAR);

	// Ksilyan: levels 16-50 donate items and get rid of them.
	if (vnum == ROOM_VNUM_DONATION_TRASH || (ch->level > 15 && ch->level <= 50) )
	{
		extract_obj(obj, TRUE);
		return;
	}
	else
	{
		obj_from_char(obj);
		UpdateObjectHolders(obj, ch);
		obj_to_room(obj, pNew);
	}

	pOld = ch->GetInRoom();
	char_from_room(ch);
	char_to_room(ch, pNew);
	act(AT_ACTION, "A hand appears in front of you, drops $p, and disappears again.",
		ch, obj, NULL, TO_ROOM);
	char_from_room(ch);
	char_to_room(ch, pOld);

}



