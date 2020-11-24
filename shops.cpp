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
 *			 Shop and repair shop module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

// Forward declaration
void do_say(Character * ch, const char* argument);

/*
 * Local functions
 */

#define	CD	CHAR_DATA
CD *	find_keeper	 ( CHAR_DATA *ch ) ;
CD *	find_fixer	 ( CHAR_DATA *ch ) ;
int	get_cost	 ( CHAR_DATA *ch, CHAR_DATA *keeper,
				OBJ_DATA *obj, bool fBuy );
int 	get_repaircost   ( CHAR_DATA *keeper, OBJ_DATA *obj ) ;
#undef CD

/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper( CHAR_DATA *ch )
{
    CHAR_DATA *keeper;
    SHOP_DATA *pShop;
    char buf[MAX_STRING_LENGTH];

    pShop = NULL;
    for ( keeper = ch->GetInRoom()->first_person;
	  keeper;
	  keeper = keeper->next_in_room )
	if ( IS_NPC(keeper) && (pShop = keeper->pIndexData->pShop) != NULL )
	    break;

    if ( !pShop )
    {
	return NULL;
    }

   	/*
     * Checks to see if the shop is open, tells you when to come back - Zoie
     * Supports closing times after midnight
     */
   	if( pShop->open_hour > pShop->close_hour )
   	{
      	if( time_info.hour < pShop->open_hour && time_info.hour > pShop->close_hour )
      	{
         	if( pShop->open_hour == 12 )
		    	do_say( keeper, "Sorry, come back at noon." );
         	else if( pShop->open_hour > 12 )
         	{
		    	sprintf( buf, "Sorry, come back at %d pm.", ( pShop->open_hour - 12 ) );
            	do_say( keeper, buf );
         	}
         	else
         	{
		    	sprintf( buf, "Sorry, come back at %d am.", pShop->open_hour );
            	do_say( keeper, buf );
         	}
         	return NULL;
      	}
   	}
   	else
   	{
      	if( time_info.hour < pShop->open_hour )
      	{
         	if( pShop->open_hour == 12 )
		    	do_say( keeper, "Sorry, come back at noon." );
         	else if( pShop->open_hour > 12 )
         	{
		    	sprintf( buf, "Sorry, come back at %d pm.", ( pShop->open_hour - 12 ) );
            	do_say( keeper, buf );
         	}
         	else
         	{
		    	sprintf( buf, "Sorry, come back at %d am.", pShop->open_hour );
            	do_say( keeper, buf );
         	}
         	return NULL;
      	}
      	if( time_info.hour > pShop->close_hour )
      	{
         	if( pShop->open_hour == 0 )
		    	do_say( keeper, "Sorry, come back at midnight." );
         	else if( pShop->open_hour > 12 )
         	{
		    	sprintf( buf, "Sorry, come back tomorrow at %d pm.", ( pShop->open_hour - 12 ) );
            	do_say( keeper, buf );
         	}
         	else
         	{
		    	sprintf( buf, "Sorry, come back tomorrow at %d am.", pShop->open_hour );
            	do_say( keeper, buf );
         	}
         	return NULL;
      	}
   	}

    /*
     * Invisible or hidden people.
     */
    if ( !can_see( keeper, ch ) )
    {
	do_say( keeper, "I don't trade with folks I can't see." );
	return NULL;
    }

    if ( !knows_language( keeper, ch->speaking, ch ) )
    {
	do_say( keeper, "I can't understand you." );
	return NULL;
    }

    return keeper;
}

/*
 * repair commands.
 */
CHAR_DATA *find_fixer( CHAR_DATA *ch )
{
     CHAR_DATA *keeper;
    REPAIR_DATA *rShop;

    rShop = NULL;
    for ( keeper = ch->GetInRoom()->first_person;
	  keeper;
	  keeper = keeper->next_in_room )
	if ( IS_NPC(keeper) && (rShop = keeper->pIndexData->rShop) != NULL )
	    break;

    if ( !rShop )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return NULL;
    }

    /*
     * Shop hours.
     */
    if ( time_info.hour < rShop->open_hour )
    {
	do_say( keeper, "Sorry, come back later." );
	return NULL;
    }

    if ( time_info.hour > rShop->close_hour )
    {
	do_say( keeper, "Sorry, come back tomorrow." );
	return NULL;
    }

    /*
     * Invisible or hidden people.
     */
    if ( !can_see( keeper, ch ) )
    {
	do_say( keeper, "I don't trade with folks I can't see." );
	return NULL;
    }

    if ( !knows_language( keeper, ch->speaking, ch ) )
    {
	do_say( keeper, "I can't understand you." );
	return NULL;
    }

    return keeper;
}



int get_cost( CHAR_DATA *ch, CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy )
{
    SHOP_DATA *pShop;
    int cost;
    bool richcustomer;
    int profitmod;

    if ( !obj || ( pShop = keeper->pIndexData->pShop ) == NULL )
	return 0;

    if ( ch->gold > (ch->level * ch->level * 100) )
	richcustomer = TRUE;
    else
	richcustomer = FALSE;

    if ( fBuy )
    {
	cost = (int) (cost * (80 + UMIN(ch->level, LEVEL_HERO_MAX))) / 100;

	profitmod = 13 - ch->getCha() + (richcustomer ? 15 : 0)
		  + ((URANGE(5,ch->level,LEVEL_HERO_MAX)-20)/2);
	cost = (int) (obj->cost
	     * UMAX( (pShop->profit_sell+1), pShop->profit_buy+profitmod ) )
	     / 100;
    }
    else
    {
	OBJ_DATA *obj2;
	int itype;

	profitmod = ch->getCha() - 13 - (richcustomer ? 15 : 0);
	cost = 0;
	for ( itype = 0; itype < MAX_TRADE; itype++ )
	{
	    if ( obj->item_type == pShop->buy_type[itype] )
	    {
		cost = (int) (obj->cost
		     * UMIN( (pShop->profit_buy-1),
			      pShop->profit_sell+profitmod) ) / 100;
		break;
	    }
	}

	for ( obj2 = keeper->first_carrying; obj2; obj2 = obj2->next_content )
	{
	    if ( obj->pIndexData == obj2->pIndexData )
	    {
		cost = 0;
		break;
	    }
	}
    }

    if ( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND )
	cost = (int) (cost * obj->value[2] / obj->value[1]);

    return cost;
}

int get_repaircost( CHAR_DATA *keeper, OBJ_DATA *obj )
{
	REPAIR_DATA *rShop;
	int basecost;
	int cost;
	int itype;
	bool found;

	if ( !obj || ( rShop = keeper->pIndexData->rShop ) == NULL )
		return 0;

	cost = basecost = 0;
	found = FALSE;
	for ( itype = 0; itype < MAX_FIX; itype++ )
	{
		if ( obj->item_type == rShop->fix_type[itype] )
		{
			basecost = (int) (obj->cost / 75) + rShop->profit_fix;
			if (basecost < 0)
				basecost *= -1;
			found = TRUE;
			break;
		}
	}

	if ( !found )
		basecost = cost = -1;

	if ( basecost == 0 )
		basecost = 1;

	if ( found && basecost > 0 )
	{
		switch (obj->item_type)
		{
			case ITEM_ARMOR:
			case ITEM_WEAPON:
			case ITEM_CONTAINER:
				if (obj->condition >= obj->max_condition)
					cost = -2;
				else
				{
					int tempcondition = obj->condition;
					if (tempcondition < 0)
					{
						cost = basecost * (-tempcondition ) * 3;
						tempcondition = 0;
					}
					cost += basecost * (obj->max_condition - tempcondition );
				}
				break;
			case ITEM_WAND:
			case ITEM_STAFF:
				if (obj->value[OBJECT_WAND_CHARGES] >= obj->value[OBJECT_WAND_MAXCHARGES])
					cost = -2;
				else
					cost = basecost * (obj->value[OBJECT_WAND_MAXCHARGES] - obj->value[OBJECT_WAND_CHARGES]);
		}
	}

	return cost;
}

void do_buy(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	int maxgold;
	int sn_haggle;

	sn_haggle = skill_lookup("haggle");

	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Buy what?\n\r", ch );
		return;
	}

	if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_PET_SHOP) )
	{
		char buf[MAX_STRING_LENGTH];
		CHAR_DATA *pet;
		ROOM_INDEX_DATA *pRoomIndexNext;
		ROOM_INDEX_DATA *in_room;

		if ( IS_NPC(ch) )
			return;

		pRoomIndexNext = get_room_index( ch->GetInRoom()->vnum + 1 );
		if ( !pRoomIndexNext )
		{
			bug( "Do_buy: bad pet shop at vnum %s (no pet storage room at vnum+1).", vnum_to_dotted(ch->GetInRoom()->vnum));
			send_to_char( "Sorry, you can't buy that here.\n\r", ch );
			return;
		}

		in_room     = ch->GetInRoom();
		ch->InRoomId = pRoomIndexNext->GetId();
		pet         = get_char_room( ch, arg );
		ch->InRoomId = in_room->GetId();

		if ( pet == NULL || !IS_NPC( pet ) || !IS_SET(pet->act, ACT_PET) )
		{
			send_to_char( "Sorry, you can't buy that here.\n\r", ch );
			return;
		}

		/*	if ( IS_SET(ch->act, PLR_BOUGHT_PET) )
			{
			send_to_char( "You already bought one pet this level.\n\r", ch );
			return;
			}
		 */
		if ( ch->gold < 10 * pet->level * pet->level )
		{
			send_to_char( "You can't afford it.\n\r", ch );
			return;
		}

		if ( ch->level < pet->level )
		{
			send_to_char( "You're not ready for this pet.\n\r", ch );
			return;
		}

		maxgold = 10 * pet->level * pet->level;
		ch->gold	-= maxgold;
		boost_economy( ch->GetInRoom()->area, maxgold );
		pet		= create_mobile( pet->pIndexData );
		SET_BIT(ch->act, PLR_BOUGHT_PET);
		SET_BIT(pet->act, ACT_PET);
		SET_BIT(pet->affected_by, AFF_CHARM);

		argument = one_argument( argument, arg );
		if ( arg[0] != '\0' )
		{
			sprintf( buf, "%s %s", pet->getName().c_str(), arg );
			pet->setName( buf );
		}

		sprintf( buf, "%sA neck tag says 'I belong to %s'.\n\r",
				pet->description_.c_str(), ch->getShort().c_str() );
		pet->description_ = buf;

		char_to_room( pet, ch->GetInRoom() );
		add_follower( pet, ch );
		send_to_char( "Enjoy your pet.\n\r", ch );
		act( AT_ACTION, "$n bought $N as a pet.", ch, NULL, pet, TO_ROOM );
		return;
	}
	else
	{
		CHAR_DATA *keeper;
		OBJ_DATA *obj;
		int cost;
		int noi = 1;		/* Number of items */
		sh_int mnoi = 20;	/* Max number of items to be bought at once */

		if ( ( keeper = find_keeper( ch ) ) == NULL )
		{
			send_to_char("You can't do that here.\n\r", ch);
			return;
		}

		maxgold = keeper->level * keeper->level * 50000;

		if ( is_number( arg ) )
		{
			noi = atoi( arg );
			argument = one_argument( argument, arg );
			if ( noi > mnoi )
			{
				act( AT_TELL, "$n tells you 'I don't sell that many items at"
						" once.'", keeper, NULL, ch, TO_VICT );		return;
			}
		}

		obj  = get_obj_carry( keeper, arg );
		cost = ( get_cost( ch, keeper, obj, TRUE ) * noi );
		if ( cost <= 0 || !can_see_obj( ch, obj ) )
		{
			act( AT_TELL, "$n tells you 'I don't sell that -- try 'list'.'",
					keeper, NULL, ch, TO_VICT );
			return;
		}

		if ( obj->pIndexData->total_count >= obj->pIndexData->rare && obj->pIndexData->rare >= 1 )
		{
			interpret(keeper, "shake");
			interpret(keeper, "say Sorry, I'm all out of stock.");
			return;
		}

		if ( !IS_OBJ_STAT( obj, ITEM_INVENTORY ) && ( noi > 1 ) )
		{
			interpret( keeper, "laugh" );
			act( AT_TELL, "$n tells you 'I don't have enough of those in stock"
					" to sell more than one at a time.'", keeper, NULL, ch, TO_VICT );
			return;
		}

		if ( sn_haggle >= 0 && ch->level >= skill_table[sn_haggle]->skill_level[ch->Class] ) {
			int change;

			change = (cost/100) * (ch->pcdata->learned[sn_haggle]/10);

			if ( number_percent() < ch->pcdata->learned[sn_haggle] ) {
				learn_from_success(ch, sn_haggle);

				set_char_color(AT_ACTION, ch);
				send_to_char("After a bit of haggling, you manage to get the price lowered.\n", ch);
				ch_printf(ch, "The original price was %d, you managed to lower it to %d.\n",
						cost, cost - change);
				cost -= change;
			} else {
				learn_from_failure(ch, sn_haggle);
				set_char_color(AT_ACTION, ch);
				send_to_char("The shopkeeper, angered by your attempt to manipulate him, raised the price.\n", ch);
				ch_printf(ch, "The original price was %d, but the shopkeeper raised it to %d.\n",
						cost, cost + change);
				cost += change;
			}
		}

		if ( ch->gold < cost )
		{
			act( AT_TELL, "$n tells you 'You can't afford to buy $p.'",
					keeper, obj, ch, TO_VICT );
			return;
		}

#ifdef USE_OBJECT_LEVELS
		if ( obj->level > ch->level )
		{
			act( AT_TELL, "$n tells you 'You can't use $p yet.'",
					keeper, obj, ch, TO_VICT );
			return;
		}
#endif

		if ( IS_SET(obj->extra_flags, ITEM_PROTOTYPE)
				&& get_trust( ch ) < LEVEL_IMMORTAL )
		{
			act( AT_TELL, "$n tells you 'This is a only a prototype!  I can't sell you that...'",
					keeper, NULL, ch, TO_VICT );
			return;
		}

		if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
		{
			send_to_char( "You can't carry that many items.\n\r", ch );
			return;
		}

		if ( ch->carry_weight + ( get_obj_weight( obj ) * noi )
				+ (noi > 1 ? 2 : 0) > can_carry_w( ch ) )
		{
			send_to_char( "You can't carry that much weight.\n\r", ch );
			return;
		}

		if ( noi == 1 )
		{
			separate_obj(obj);
			act( AT_ACTION, "$n buys $p.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "You buy $p.", ch, obj, NULL, TO_CHAR );
		}
		else
		{
			bool plural = obj->shortDesc_.c_str() [ obj->shortDesc_.length() - 1 ] == 's';

			sprintf( arg, "$n buys %d $p%s.", noi,
					( plural ? "" : "s" ) );
			act( AT_ACTION, arg, ch, obj, NULL, TO_ROOM );
			sprintf( arg, "You buy %d $p%s.", noi,
					( plural ? "" : "s" ) );
			act( AT_ACTION, arg, ch, obj, NULL, TO_CHAR );
			act( AT_ACTION, "$N puts them into a bag and hands it to you.",
					ch, NULL, keeper, TO_CHAR );
		}

		ch->gold     -= cost;
		keeper->gold += cost;

		if ( keeper->gold > maxgold )
		{
			boost_economy( keeper->GetInRoom()->area, keeper->gold - maxgold/2 );
			keeper->gold = maxgold/2;
			act( AT_ACTION, "$n puts some gold into a large safe.", keeper, NULL, NULL, TO_ROOM );
		}

		if ( IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
		{
			OBJ_DATA *buy_obj, *bag;

#ifdef USE_OBJECT_LEVELS
			buy_obj = create_object( obj->pIndexData, obj->level );
#else
			buy_obj = create_object(obj->pIndexData, 0);
#endif

			REMOVE_BIT(buy_obj->extra_flags, ITEM_INVENTORY);


			/*
			 * Due to grouped objects and carry limitations in SMAUG
			 * The shopkeeper gives you a bag with multiple-buy,
			 * and also, only one object needs be created with a count
			 * set to the number bought.		-Thoric
			 */
			if ( noi > 1 )
			{
				bag = create_object( get_obj_index( OBJ_VNUM_SHOPPING_BAG ), 1 );
				/* perfect size bag ;) */
				bag->value[0] = bag->weight + (buy_obj->weight * noi);
				buy_obj->count = noi;
				obj->pIndexData->count += (noi - 1);
				obj->pIndexData->total_count += (noi -1);
				numobjsloaded += (noi - 1);
				obj_to_obj( buy_obj, bag );
				obj_to_char( bag, ch );
			}
			else
				obj_to_char( buy_obj, ch );
		}
		else
		{
			obj_from_char( obj );
			obj_to_char( obj, ch );
		}

		return;
	}
}


void do_list(CHAR_DATA *ch, const char* argument)
{
	if ( IS_SET(ch->GetInRoom()->room_flags, ROOM_PET_SHOP) )
	{
		ROOM_INDEX_DATA *pRoomIndexNext;
		CHAR_DATA *pet;
		bool found;

		pRoomIndexNext = get_room_index( ch->GetInRoom()->vnum + 1 );
		if ( !pRoomIndexNext )
		{
			bug( "Do_list: bad pet shop at vnum %s (no pet storage room at vnum+1).", vnum_to_dotted(ch->GetInRoom()->vnum));
			send_to_char( "You can't do that here.\n\r", ch );
			return;
		}

		found = FALSE;
		for ( pet = pRoomIndexNext->first_person; pet; pet = pet->next_in_room )
		{
			if ( IS_SET(pet->act, ACT_PET) && IS_NPC(pet) )
			{
				if ( !found )
				{
					found = TRUE;
					send_to_char( "Pets for sale:\n\r", ch );
				}
				ch_printf( ch, "[%2d] %8d - %s\n\r",
						pet->level,
						10 * pet->level * pet->level,
						pet->getShort().c_str() );
			}
		}
		if ( !found )
			send_to_char( "Sorry, we're out of pets right now.\n\r", ch );
		return;
	}
	else
	{
		char arg[MAX_INPUT_LENGTH];
		CHAR_DATA *keeper;
		OBJ_DATA *obj;
		int cost, liquid;
		bool found;


		one_argument( argument, arg );

		if ( ( keeper = find_keeper( ch ) ) == NULL )
		{
			send_to_char("You can't do that here.\n\r", ch);
			return;
		}

		found = FALSE;
		for ( obj = keeper->first_carrying; obj; obj = obj->next_content )
		{
			if ( obj->wear_loc == WEAR_NONE
					&&   can_see_obj( ch, obj )
					&& ( cost = get_cost( ch, keeper, obj, TRUE ) ) > 0
					&& ( arg[0] == '\0' || nifty_is_name( arg, obj->name_.c_str() ) ) )
			{
				if ( !found )
				{
					found = TRUE;
#ifdef USE_OBJECT_LEVELS
					send_to_char( "[Lv Price] Item\n\r", ch );
#else
					send_to_char("[Price] Item\n\r", ch);
#endif
				}
#ifdef USE_OBJECT_LEVELS
				ch_printf( ch, "[%2d %5d] %s.\n\r",
						obj->level, cost, capitalize( obj->shortDesc_.c_str() ) );
#else
				if( obj->item_type == ITEM_DRINK_CON )
				{
					if( ( liquid = obj->value[2] ) > LIQ_MAX )
						liquid = 0;
					if( obj->value[1] > 0 )
						ch_printf( ch, "[%5d] %s (%s).\n\r", cost, capitalize(obj->shortDesc_.c_str()), liq_table[liquid].liq_name );
					else
						ch_printf( ch, "[%5d] %s (empty).\n\r", cost, capitalize(obj->shortDesc_.c_str()) );

				}
				else
					ch_printf( ch, "[%5d] %s.\n\r", cost, capitalize(obj->shortDesc_.c_str()));
#endif
			}
		}

		if ( !found )
		{
			if ( arg[0] == '\0' )
				send_to_char( "You can't buy anything here.\n\r", ch );
			else
				send_to_char( "You can't buy that here.\n\r", ch );
		}
		return;
	}
}


void do_sell(CHAR_DATA *ch, const char* argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Sell what?\n\r", ch );
	return;
    }


	if ( ( keeper = find_keeper( ch ) ) == NULL )
	{
		for (keeper = ch->GetInRoom()->first_person; keeper; keeper = keeper->next_in_room)
			if (IS_NPC(keeper) && IS_SET(keeper->act, ACT_GEMDEALER) )
				break;
	}

	if ( !keeper )
	{
		send_to_char("There is noone here to sell anything to.\n\r", ch);
		return;
	}

	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
		act( AT_TELL, "$n tells you, 'You don't have that item.'", keeper, NULL, ch, TO_VICT );
		return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
		send_to_char( "You can't let go of it!\n\r", ch );
		return;
	}

	if ( obj->item_type == ITEM_GEM && IS_NPC(keeper) && IS_SET(keeper->act, ACT_GEMDEALER) )
	{
		int worth;

		worth = GetGemWorth(obj);

		separate_obj(obj);
		act( AT_ACTION, "$n sells $p.", ch, obj, NULL, TO_ROOM );
		sprintf( buf, "You sell $p for %d gold piece%s.", worth, worth == 1 ? "" : "s" );
		act( AT_ACTION, buf, ch, obj, NULL, TO_CHAR);
		extract_obj(obj, TRUE);

		ch->gold += worth;

		return;
	}

	if ( IS_SET(keeper->act, ACT_GEMDEALER) || obj->item_type == ITEM_GEM )
	{
		act( AT_PLAIN, "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT);
		return;
	}


    if ( obj->timer > 0 )
    {
	act( AT_TELL, "$n tells you, '$p is depreciating in value too quickly...'", keeper, obj, ch, TO_VICT );
	return;
    }

    if ( ( cost = get_cost( ch, keeper, obj, FALSE ) ) <= 0 )
    {
	act( AT_ACTION, "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }

    if ( cost >= keeper->gold )
    {
	act( AT_TELL, "$n tells you, '$p is worth more than I can afford...'", keeper, obj, ch, TO_VICT );
	return;
    }

    separate_obj( obj );
    act( AT_ACTION, "$n sells $p.", ch, obj, NULL, TO_ROOM );
    sprintf( buf, "You sell $p for %d gold piece%s.",
	cost, cost == 1 ? "" : "s" );
    act( AT_ACTION, buf, ch, obj, NULL, TO_CHAR );
    ch->gold     += cost;
    keeper->gold -= cost;
    if ( keeper->gold < 0 )
	keeper->gold = 0;

    if ( obj->item_type == ITEM_TRASH )
    {
	extract_obj( obj, TRUE );
    }
    else
    {
	obj_from_char( obj );
	obj_to_char( obj, keeper );
    }

    return;
}



void do_value(CHAR_DATA *ch, const char* argument)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Value what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
	{
		send_to_char("You can't do that here.\n\r", ch);
		return;
	}

    if ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
    {
	act( AT_TELL, "$n tells you 'You don't have that item.'",
		keeper, NULL, ch, TO_VICT );
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it!\n\r", ch );
	return;
    }

    if ( ( cost = get_cost( ch, keeper, obj, FALSE ) ) <= 0 )
    {
	act( AT_ACTION, "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
	return;
    }

    sprintf( buf, "$n tells you 'I'll give you %d gold coins for $p.'", cost );
    act( AT_TELL, buf, keeper, obj, ch, TO_VICT );

    return;
}

/*
 * Repair a single object. Used when handling "repair all" - Gorog
 */
void repair_one_obj( CHAR_DATA *ch, CHAR_DATA *keeper, OBJ_DATA *obj,
                 const char *arg, int maxgold, const char *fixstr, const char*fixstr2 )
{
	char buf[MAX_STRING_LENGTH];
	int cost;
	int sn_haggle = skill_lookup("haggle");

	if ( !can_drop_obj( ch, obj ) )
	{
		ch_printf( ch, "You can't let go of %s.\n\r", obj->name_.c_str() );
		return;
	}

	if ( obj->max_condition == 1 )
	{
		act( AT_TELL, "$n tells you, '$p is too weak to be repaired.'", keeper, obj, ch, TO_VICT);
		return;
	}

	if ( ( cost = get_repaircost( keeper, obj ) ) < 0 )
	{
		if (cost != -2)
			act( AT_TELL, "$n tells you, 'Sorry, I can't do anything with $p.'",
					keeper, obj, ch, TO_VICT );
		else
			act( AT_TELL, "$n tells you, '$p looks fine to me!'", keeper, obj, ch, TO_VICT );

		return;
	}

	/* "repair all" gets a 10% surcharge - Gorog */
	if ( !strcmp("all", arg) ) {
		cost = 11*cost/10;
	}

	if ( sn_haggle >= 0 && ch->level >= skill_table[sn_haggle]->skill_level[ch->Class] ) {
		int change;

		change = (cost/100) * (ch->pcdata->learned[sn_haggle]/10);

		if ( number_percent() < ch->pcdata->learned[sn_haggle] ) {
			learn_from_success(ch, sn_haggle);

			set_char_color(AT_ACTION, ch);
			send_to_char("After a bit of haggling, you manage to get the cost lowered.\n", ch);
			ch_printf(ch, "The original cost was %d, you managed to lower it to %d.\n",
					cost, cost - change);
			cost -= change;
		} else {
			learn_from_failure(ch, sn_haggle);
			set_char_color(AT_ACTION, ch);
			send_to_char("The shopkeeper, angered by your attempt to manipulate him, raised the cost.\n", ch);
			ch_printf(ch, "The original cost was %d, but the shopkeeper raised it to %d.\n",
					cost, cost + change);
			cost += change;
		}
	}


	if ( cost > ch->gold )
	{
		sprintf( buf,
				"$N tells you, 'It will cost %d piece%s of gold to %s %s...'", cost,
				cost == 1 ? "" : "s", fixstr, obj->name_.c_str() );
		act( AT_TELL, buf, ch, NULL, keeper, TO_CHAR );
		act( AT_TELL, "$N tells you, 'Which I see you can't afford.'", ch,
				NULL, keeper, TO_CHAR );
		return;
	}

	sprintf( buf, "$n gives $p to $N, who quickly %s it.", fixstr2 );
	act( AT_ACTION, buf, ch, obj, keeper, TO_ROOM );
	sprintf( buf, "$N charges you %d gold piece%s to %s $p.",
			cost, cost == 1 ? "" : "s", fixstr );
	act( AT_ACTION, buf, ch, obj, keeper, TO_CHAR );
	ch->gold     -= cost;
	keeper->gold += cost;

	if ( keeper->gold < 0 )
		keeper->gold = 0;
	else
		if ( keeper->gold > maxgold )
		{
			boost_economy( keeper->GetInRoom()->area, keeper->gold - maxgold/2 );
			keeper->gold = maxgold/2;
			act( AT_ACTION, "$n puts some gold into a large safe.", keeper,
					NULL, NULL, TO_ROOM );
		}

	switch ( obj->item_type )
	{
		default:
			send_to_char( "For some reason, you think you got ripped off...\n\r", ch);
			break;

		case ITEM_ARMOR:
		case ITEM_WEAPON:
		case ITEM_CONTAINER:
			/* KSILYAN: new condition handling */

			// don't reduce max_condition on a never-break item.
			if ( !IS_SET(obj->extra_flags_2, ITEM_NEVER_BREAKS) )
				obj->max_condition--;
			obj->condition = obj->max_condition;
			break;
		case ITEM_WAND:
		case ITEM_STAFF:
			obj->value[OBJECT_WAND_CHARGES] = obj->value[OBJECT_WAND_MAXCHARGES];
			break;
	}

	oprog_repair_trigger( ch, obj );
}

void do_repair(CHAR_DATA *ch, const char* argument)
{
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	const char *fixstr;
	const char *fixstr2;
	int maxgold;

	if ( argument[0] == '\0' )
	{
		send_to_char( "Repair what?\n\r", ch );
		return;
	}

	if ( ( keeper = find_fixer( ch ) ) == NULL )
		return;

	maxgold = keeper->level * keeper->level * 100000;
	switch( keeper->pIndexData->rShop->shop_type )
	{
		default:
		case SHOP_FIX:
			fixstr  = "repair";
			fixstr2 = "repairs";
			break;
		case SHOP_RECHARGE:
			fixstr  = "recharge";
			fixstr2 = "recharges";
			break;
	}

	if ( !strcmp( argument, "all" ) )
	{
		for ( obj = ch->first_carrying; obj ; obj = obj->next_content )
		{
			if ( obj->wear_loc  == WEAR_NONE
					&&   can_see_obj( ch, obj )
					&& ( obj->item_type == ITEM_ARMOR
						||   obj->item_type == ITEM_WEAPON
						||   obj->item_type == ITEM_CONTAINER
						||   obj->item_type == ITEM_WAND
						||   obj->item_type == ITEM_STAFF ) )
				repair_one_obj( ch, keeper, obj, argument, maxgold,
						fixstr, fixstr2);
		}
		return;
	}

	if ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
	{
		act( AT_TELL, "$n tells you 'You don't have that item.'",
				keeper, NULL, ch, TO_VICT );
		return;
	}

	repair_one_obj( ch, keeper, obj, argument, maxgold, fixstr, fixstr2);
}

void appraise_all( CHAR_DATA *ch, CHAR_DATA *keeper, const char *fixstr )
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH], *pbuf=buf;
    int cost, total=0;

    for ( obj = ch->first_carrying; obj != NULL ; obj = obj->next_content )
    {
        if ( obj->wear_loc  == WEAR_NONE
        &&   can_see_obj( ch, obj )
        && ( obj->item_type == ITEM_ARMOR
        ||   obj->item_type == ITEM_WEAPON
        ||   obj->item_type == ITEM_WAND
        ||   obj->item_type == ITEM_STAFF ) )
        {
            if ( !can_drop_obj( ch, obj ) ) {
                ch_printf( ch, "You can't let go of %s.\n\r", obj->name_.c_str() );
            } else if ( ( cost = get_repaircost( keeper, obj ) ) < 0 ) {
               if (cost != -2) {
                    act( AT_TELL,
                        "$n tells you, 'Sorry, I can't do anything with $p.'",
                        keeper, obj, ch, TO_VICT );
               } else {
                    act( AT_TELL, "$n tells you, '$p looks fine to me!'",
                        keeper, obj, ch, TO_VICT );
               }
            } else {
                sprintf( buf,
                        "$N tells you, 'It will cost %d piece%s of gold to %s %s'",
                        cost, cost == 1 ? "" : "s", fixstr, obj->name_.c_str() );
                act( AT_TELL, buf, ch, NULL, keeper, TO_CHAR );
                total += cost;
            }
        }
    }
    if ( total > 0 )
    {
       send_to_char ("\n\r", ch);
       sprintf( buf,
          "$N tells you, 'It will cost %d piece%s of gold in total.'",
          total, cost == 1 ? "" : "s" );
       act( AT_TELL, buf, ch, NULL, keeper, TO_CHAR );
       strcpy( pbuf,
       "$N tells you, 'Remember there is a 10% surcharge for repair all.'");
       act( AT_TELL, buf, ch, NULL, keeper, TO_CHAR );
    }
}


void do_appraise(CHAR_DATA *ch, const char* argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;
    const char *fixstr;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Appraise what?\n\r", ch );
	return;
    }

    if ( ( keeper = find_fixer( ch ) ) == NULL )
	return;

    switch( keeper->pIndexData->rShop->shop_type )
    {
	default:
	case SHOP_FIX:
	  fixstr  = "repair";
	  break;
	case SHOP_RECHARGE:
	  fixstr  = "recharge";
	  break;
    }

    if ( !strcmp( arg, "all") )
    {
    appraise_all( ch, keeper, fixstr );
    return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
	act( AT_TELL, "$n tells you 'You don't have that item.'",
		keeper, NULL, ch, TO_VICT );
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "You can't let go of it.\n\r", ch );
	return;
    }

    if ( ( cost = get_repaircost( keeper, obj ) ) < 0 )
    {
      if (cost != -2)
	act( AT_TELL, "$n tells you, 'Sorry, I can't do anything with $p.'", keeper, obj, ch, TO_VICT );
      else
	act( AT_TELL, "$n tells you, '$p looks fine to me!'", keeper, obj, ch, TO_VICT );
      return;
    }

    sprintf( buf,
       "$N tells you, 'It will cost %d piece%s of gold to %s that...'", cost,
       cost == 1 ? "" : "s", fixstr );
    act( AT_TELL, buf, ch, NULL, keeper, TO_CHAR );
    if ( cost > ch->gold )
      act( AT_TELL, "$N tells you, 'Which I see you can't afford.'", ch,
	 NULL, keeper, TO_CHAR );

    return;
}


/* ------------------ Shop Building and Editing Section ----------------- */


void do_makeshop(CHAR_DATA *ch, const char* argument)
{
	SHOP_DATA *shop;
	int vnum;
	MOB_INDEX_DATA *mob;

	if ( !argument || argument[0] == '\0' )
	{
		send_to_char( "Usage: makeshop <mobvnum>\n\r", ch );
		return;
	}

	vnum = dotted_to_vnum(ch->GetInRoom()->vnum, argument);

	if ( (mob = get_mob_index(vnum)) == NULL )
	{
		send_to_char( "Mobile not found.\n\r", ch );
		return;
	}

	if ( !can_medit(ch, mob) )
		return;

	if ( mob->pShop )
	{
		send_to_char( "This mobile already has a shop.\n\r", ch );
		return;
	}

	CREATE( shop, SHOP_DATA, 1 );

	LINK( shop, first_shop, last_shop, next, prev );
	shop->keeper	= vnum;
	shop->profit_buy	= 120;
	shop->profit_sell	= 90;
	shop->open_hour = 0;
	shop->close_hour	= 23;
	mob->pShop		= shop;
	send_to_char( "Done.\n\r", ch );
	return;
}


void do_shopset(CHAR_DATA *ch, const char* argument)
{
	SHOP_DATA *shop;
	MOB_INDEX_DATA *mob, *mob2;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int vnum;
	int value;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Usage: shopset <mob vnum> <field> value\n\r", ch );
		send_to_char( "\n\rField being one of:\n\r", ch );
		send_to_char( "  buy0 buy1 buy2 buy3 buy4 buy sell open close keeper\n\r", ch );
		return;
	}

	vnum = dotted_to_vnum(ch->GetInRoom()->vnum, arg1);

	if ( (mob = get_mob_index(vnum)) == NULL )
	{
		send_to_char( "Mobile not found.\n\r", ch );
		return;
	}

	if ( !can_medit(ch, mob) )
		return;

	if ( !mob->pShop )
	{
		send_to_char( "This mobile doesn't keep a shop.\n\r", ch );
		return;
	}
	shop = mob->pShop;
	value = atoi( argument );

	if ( !str_cmp( arg2, "buy0" ) )
	{
		if ( !is_number(argument) )
			value = itemtype_name_to_number(argument);
		if ( value < 0 || value > MAX_ITEM_TYPE )
		{
			send_to_char( "Invalid item type!\n\r", ch );
			return;
		}
		shop->buy_type[0] = value;
		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "buy1" ) )
	{
		if ( !is_number(argument) )
			value = itemtype_name_to_number(argument);
		if ( value < 0 || value > MAX_ITEM_TYPE )
		{
			send_to_char( "Invalid item type!\n\r", ch );
			return;
		}
		shop->buy_type[1] = value;
		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "buy2" ) )
	{
		if ( !is_number(argument) )
			value = itemtype_name_to_number(argument);
		if ( value < 0 || value > MAX_ITEM_TYPE )
		{
			send_to_char( "Invalid item type!\n\r", ch );
			return;
		}
		shop->buy_type[2] = value;
		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "buy3" ) )
	{
		if ( !is_number(argument) )
			value = itemtype_name_to_number(argument);
		if ( value < 0 || value > MAX_ITEM_TYPE )
		{
			send_to_char( "Invalid item type!\n\r", ch );
			return;
		}
		shop->buy_type[3] = value;
		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "buy4" ) )
	{
		if ( !is_number(argument) )
			value = itemtype_name_to_number(argument);
		if ( value < 0 || value > MAX_ITEM_TYPE )
		{
			send_to_char( "Invalid item type!\n\r", ch );
			return;
		}
		shop->buy_type[4] = value;
		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "buy" ) )
	{
		if ( value <= (shop->profit_sell+5) || value > 1000 )
		{
			send_to_char( "Out of range.\n\r", ch );
			return;
		}
		shop->profit_buy = value;
		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "sell" ) )
	{
		if ( value < 0 || value >= (shop->profit_buy-5) )
		{
			send_to_char( "Out of range.\n\r", ch );
			return;
		}
		shop->profit_sell = value;
		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "open" ) )
	{
		if ( value < 0 || value > 23 )
		{
			send_to_char( "Out of range.\n\r", ch );
			return;
		}
		shop->open_hour = value;
		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "close" ) )
	{
		if ( value < 0 || value > 23 )
		{
			send_to_char( "Out of range.\n\r", ch );
			return;
		}
		shop->close_hour = value;
		send_to_char( "Done.\n\r", ch );
		return;
	}

	if ( !str_cmp( arg2, "keeper" ) )
	{
		if ( (mob2 = get_mob_index(vnum)) == NULL )
		{
			send_to_char( "Mobile not found.\n\r", ch );
			return;
		}
		if ( !can_medit(ch, mob) )
			return;
		if ( mob2->pShop )
		{
			send_to_char( "That mobile already has a shop.\n\r", ch );
			return;
		}
		mob->pShop	= NULL;
		mob2->pShop = shop;
		shop->keeper = value;
		send_to_char( "Done.\n\r", ch );
		return;
	}

	do_shopset( ch, "" );
	return;
}


void do_shopstat(CHAR_DATA *ch, const char* argument)
{
    SHOP_DATA *shop;
    MOB_INDEX_DATA *mob;
    int vnum;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Usage: shopstat <keeper vnum>\n\r", ch );
	return;
    }

    vnum = dotted_to_vnum(ch->GetInRoom()->vnum, argument);

    if ( (mob = get_mob_index(vnum)) == NULL )
    {
	send_to_char( "Mobile not found.\n\r", ch );
	return;
    }

    if ( !mob->pShop )
    {
	send_to_char( "This mobile doesn't keep a shop.\n\r", ch );
	return;
    }
    shop = mob->pShop;

    ch_printf( ch, "Keeper: %d  %s\n\r", shop->keeper, mob->shortDesc_.c_str() );
    ch_printf( ch, "buy0 [%s]  buy1 [%s]  buy2 [%s]  buy3 [%s]  buy4 [%s]\n\r",
		itemtype_number_to_name (shop->buy_type[0]),
		itemtype_number_to_name (shop->buy_type[1]),
		itemtype_number_to_name (shop->buy_type[2]),
		itemtype_number_to_name (shop->buy_type[3]),
		itemtype_number_to_name (shop->buy_type[4]) );
    ch_printf( ch, "Profit:  buy %3d%%  sell %3d%%\n\r",
			shop->profit_buy,
			shop->profit_sell );
    ch_printf( ch, "Hours:   open %2d  close %2d\n\r",
			shop->open_hour,
			shop->close_hour );
    return;
}


void do_shops(CHAR_DATA *ch, const char* argument)
{
    SHOP_DATA *shop;

    if ( !first_shop )
    {
	send_to_char( "There are no shops.\n\r", ch );
	return;
    }

    set_char_color( AT_NOTE, ch );
    for ( shop = first_shop; shop; shop = shop->next )
	ch_printf( ch, "Keeper: %9s Buy: %3d Sell: %3d Open: %2d Close: %2d Buy: %2d %2d %2d %2d %2d\n\r",
		vnum_to_dotted(shop->keeper),	   shop->profit_buy, shop->profit_sell,
		shop->open_hour,   shop->close_hour,
		shop->buy_type[0], shop->buy_type[1],
		shop->buy_type[2], shop->buy_type[3], shop->buy_type[4] );
    return;
}


/* -------------- Repair Shop Building and Editing Section -------------- */


void do_makerepair(CHAR_DATA *ch, const char* argument)
{
    REPAIR_DATA *repair;
    int vnum;
    MOB_INDEX_DATA *mob;

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "Usage: makerepair <mobvnum>\n\r", ch );
	return;
    }

    vnum = dotted_to_vnum(ch->GetInRoom()->vnum, argument);

    if ( (mob = get_mob_index(vnum)) == NULL )
    {
	send_to_char( "Mobile not found.\n\r", ch );
	return;
    }

    if ( !can_medit(ch, mob) )
      return;

    if ( mob->rShop )
    {
	send_to_char( "This mobile already has a repair shop.\n\r", ch );
	return;
    }

    CREATE( repair, REPAIR_DATA, 1 );

    LINK( repair, first_repair, last_repair, next, prev );
    repair->keeper	= vnum;
    repair->profit_fix	= 100;
    repair->shop_type	= SHOP_FIX;
    repair->open_hour	= 0;
    repair->close_hour	= 23;
    mob->rShop		= repair;
    send_to_char( "Done.\n\r", ch );
    return;
}


void do_repairset(CHAR_DATA *ch, const char* argument)
{
    REPAIR_DATA *repair;
    MOB_INDEX_DATA *mob, *mob2;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int vnum;
    int value;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Usage: repairset <mob vnum> <field> value\n\r", ch );
	send_to_char( "\n\rField being one of:\n\r", ch );
	send_to_char( "  fix0 fix1 fix2 profit type open close keeper\n\r", ch );
	return;
    }

    vnum = dotted_to_vnum(ch->GetInRoom()->vnum, arg1);

    if ( (mob = get_mob_index(vnum)) == NULL )
    {
	send_to_char( "Mobile not found.\n\r", ch );
	return;
    }

    if ( !can_medit(ch, mob) )
      return;

    if ( !mob->rShop )
    {
	send_to_char( "This mobile doesn't keep a repair shop.\n\r", ch );
	return;
    }
    repair = mob->rShop;
    value = atoi( argument );

    if ( !str_cmp( arg2, "fix0" ) )
    {
	if ( !is_number(argument) )
	  value = itemtype_name_to_number(argument);
	if ( value < 0 || value > MAX_ITEM_TYPE )
	{
	    send_to_char( "Invalid item type!\n\r", ch );
	    return;
	}
	repair->fix_type[0] = value;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "fix1" ) )
    {
	if ( !is_number(argument) )
	  value = itemtype_name_to_number(argument);
	if ( value < 0 || value > MAX_ITEM_TYPE )
	{
	    send_to_char( "Invalid item type!\n\r", ch );
	    return;
	}
	repair->fix_type[1] = value;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "fix2" ) )
    {
	if ( !is_number(argument) )
	  value = itemtype_name_to_number(argument);
	if ( value < 0 || value > MAX_ITEM_TYPE )
	{
	    send_to_char( "Invalid item type!\n\r", ch );
	    return;
	}
	repair->fix_type[2] = value;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "profit" ) )
    {
	if ( value < 1 || value > 1000 )
	{
	    send_to_char( "Out of range.\n\r", ch );
	    return;
	}
	repair->profit_fix = value;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "type" ) )
    {
	if ( value < 1 || value > 2 )
	{
	    send_to_char( "Out of range.\n\r", ch );
	    return;
	}
	repair->shop_type = value;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "open" ) )
    {
	if ( value < 0 || value > 23 )
	{
	    send_to_char( "Out of range.\n\r", ch );
	    return;
	}
	repair->open_hour = value;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "close" ) )
    {
	if ( value < 0 || value > 23 )
	{
	    send_to_char( "Out of range.\n\r", ch );
	    return;
	}
	repair->close_hour = value;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "keeper" ) )
    {
	if ( (mob2 = get_mob_index(vnum)) == NULL )
	{
	  send_to_char( "Mobile not found.\n\r", ch );
	  return;
	}
	if ( !can_medit(ch, mob) )
	  return;
	if ( mob2->rShop )
	{
	  send_to_char( "That mobile already has a repair shop.\n\r", ch );
	  return;
	}
	mob->rShop  = NULL;
	mob2->rShop = repair;
	repair->keeper = value;
	send_to_char( "Done.\n\r", ch );
	return;
    }

    do_repairset( ch, "" );
    return;
}


void do_repairstat(CHAR_DATA *ch, const char* argument)
{
    REPAIR_DATA *repair;
    MOB_INDEX_DATA *mob;
    int vnum;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Usage: repairstat <keeper vnum>\n\r", ch );
	return;
    }

    vnum = dotted_to_vnum(ch->GetInRoom()->vnum, argument);

    if ( (mob = get_mob_index(vnum)) == NULL )
    {
	send_to_char( "Mobile not found.\n\r", ch );
	return;
    }

    if ( !mob->rShop )
    {
	send_to_char( "This mobile doesn't keep a repair shop.\n\r", ch );
	return;
    }
    repair = mob->rShop;

    ch_printf( ch, "Keeper: %d  %s\n\r", repair->keeper, mob->shortDesc_.c_str() );
    ch_printf( ch, "fix0 [%s]  fix1 [%s]  fix2 [%s]\n\r",
			itemtype_number_to_name (repair->fix_type[0]),
			itemtype_number_to_name (repair->fix_type[1]),
			itemtype_number_to_name (repair->fix_type[2]) );
    ch_printf( ch, "Profit: %3d%%  Type: %d\n\r",
			repair->profit_fix,
			repair->shop_type );
    ch_printf( ch, "Hours:   open %2d  close %2d\n\r",
			repair->open_hour,
			repair->close_hour );
    return;
}


void do_repairshops(CHAR_DATA *ch, const char* argument)
{
    REPAIR_DATA *repair;

    if ( !first_repair )
    {
	send_to_char( "There are no repair shops.\n\r", ch );
	return;
    }

    set_char_color( AT_NOTE, ch );
    for ( repair = first_repair; repair; repair = repair->next )
	ch_printf( ch, "Keeper: %5d Profit: %3d Type: %d Open: %2d Close: %2d Fix: %2d %2d %2d\n\r",
		repair->keeper,	     repair->profit_fix, repair->shop_type,
		repair->open_hour,   repair->close_hour,
		repair->fix_type[0], repair->fix_type[1], repair->fix_type[2] );
    return;
}
