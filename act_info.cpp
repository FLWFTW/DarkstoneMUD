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
 *			     Informational module			    *
 ****************************************************************************/


#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "paths.const.h"

#ifdef unix
	#include <unistd.h>
	#include <crypt.h>
#endif

#include "mud.h"
#include "mxp.h"
#include "connection.h"
#include "character.h"
#include "object.h"
#include "World.h"

// STL includes
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include "utility_objs.hpp"

#include "ScentController.h"
#include "Scent.h"

const char *	const	where_name	[] =
{
    "<used as light>     ",
    "<worn on finger>    ",
    "<worn on finger>    ",
    "<worn around neck>  ",
    "<worn around neck>  ",
    "<worn on chest>     ",
    "<worn on head>      ",
    "<worn on legs>      ",
    "<worn on feet>      ",
    "<worn on hands>     ",
    "<worn on arms>      ",
    "<worn as shield>    ",
    "<worn about body>   ",
    "<worn about waist>  ",
    "<worn around wrist> ",
    "<worn around wrist> ",
    "<wielded>           ",
    "<held>              ",
    "<dual wielded>      ",
    "<worn on ears>      ",
    "<worn as cape>      ",
    "<missile wielded>   ",
    "<worn on shoulders> ",
	"<worn as quiver>    ",
	"<worn as scabbard>  ",
	"<worn as scabbard>  ",
	"<sheathed>          ",
	"<sheathed>          "
};


typedef struct combine_chars COMBINE_CHARS;

/*
 * Local functions.
 */
COMBINE_CHARS*	show_char_to_char_0	( CHAR_DATA *victim, CHAR_DATA *ch );
void	show_char_to_char_1	 ( CHAR_DATA *victim, CHAR_DATA *ch ) ;
bool	check_blind		 ( CHAR_DATA *ch ) ;
void    show_condition           ( CHAR_DATA *ch, CHAR_DATA *victim ) ;


struct combine_chars {
    COMBINE_CHARS * next;
    char* line;
    char* affects;
    int count;
};

const char *skill_prof(int p) {
    if ( p == 0 ) {
        return "";
    } else if ( p < 25 ) {
        return "Poor";
    } else if ( p < 50 ) {
        return "Fair";
    } else if ( p < 75 ) {
        return "Good";
    } else {
        return "Excl";
    }
}

char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
	static char buf[MAX_STRING_LENGTH];

	buf[0] = '\0';
	if ( IS_OBJ_STAT(obj, ITEM_INVIS)     )   strcat( buf, "(Invis) "     );
	if ( IS_AFFECTED(ch, AFF_DETECT_EVIL)
		&& IS_OBJ_STAT(obj, ITEM_EVIL)   )   strcat( buf, "(Red Aura) "  );
	if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC)
		&& IS_OBJ_STAT(obj, ITEM_MAGIC)  )   strcat( buf, "(Magical) "   );
	if ( IS_OBJ_STAT(obj, ITEM_GLOW)      )   strcat( buf, "(Glowing) "   );
	if ( IS_OBJ_STAT(obj, ITEM_DARK) )        strcat( buf, "(Dark) " );
	if ( IS_OBJ_STAT(obj, ITEM_HUM)       )   strcat( buf, "(Humming) "   );
	if ( IS_OBJ_STAT(obj, ITEM_HIDDEN)	  )   strcat( buf, "(Hidden) "	  );
	if ( IS_OBJ_STAT(obj, ITEM_BURIED)    )   strcat( buf, "(Buried) "   );
	if ( IS_IMMORTAL(ch)
		&& IS_OBJ_STAT(obj, ITEM_PROTOTYPE) ) strcat( buf, "(PROTO) "	  );
	if ( IS_AFFECTED(ch, AFF_DETECTTRAPS)
		&& is_trapped(obj)   )   strcat( buf, "(Trap) "  );

	if ( fShort )
	{
		if ( obj->shortDesc_.length() > 0 )
			strcat( buf, obj->shortDesc_.c_str() );
	}
	else
	{
		strcat( buf, obj->longDesc_.c_str() );
	}

	switch (obj->item_type)
	{
		char count[50];
		case ITEM_PROJECTILE:
			sprintf(count, " (%d left)", obj->condition);
			strcat(buf, count);
			break;
		case ITEM_CONTAINER:
			if (obj->wear_flags)
		case ITEM_ARMOR:
		case ITEM_WEAPON:
			if (obj->condition <= 0)
				strcat(buf, " (broken)");
			break;
		default:
			break;
	}

	return buf;
}


/*
 * Some increasingly freaky halucinated objects		-Thoric
 */
const char *halucinated_object( int ms, bool fShort )
{
    int sms = URANGE( 1, (ms+10)/5, 20 );

    if ( fShort )
    switch( number_range( 6-URANGE(1,sms/2,5), sms ) )
    {
	case  1: return "a sword";
	case  2: return "a stick";
	case  3: return "something shiny";
	case  4: return "something";
	case  5: return "something interesting";
	case  6: return "something colorful";
	case  7: return "something that looks cool";
	case  8: return "a nifty thing";
	case  9: return "a cloak of flowing colors";
	case 10: return "a mystical flaming sword";
	case 11: return "a swarm of insects";
	case 12: return "a deathbane";
	case 13: return "a figment of your imagination";
	case 14: return "your gravestone";
	case 15: return "the long lost boots of Ranger Tarin";
	case 16: return "a glowing tome of arcane knowledge";
	case 17: return "a long sought secret";
	case 18: return "the meaning of it all";
	case 19: return "the answer";
	case 20: return "the key to life, the universe and everything";
    }
    switch( number_range( 6-URANGE(1,sms/2,5), sms ) )
    {
	case  1: return "A nice looking sword catches your eye.";
	case  2: return "The ground is covered in small sticks.";
	case  3: return "Something shiny catches your eye.";
	case  4: return "Something catches your attention.";
	case  5: return "Something interesting catches your eye.";
	case  6: return "Something colorful flows by.";
	case  7: return "Something that looks cool calls out to you.";
	case  8: return "A nifty thing of great importance stands here.";
	case  9: return "A cloak of flowing colors asks you to wear it.";
	case 10: return "A mystical flaming sword awaits your grasp.";
	case 11: return "A swarm of insects buzzes in your face!";
	case 12: return "The extremely rare Deathbane lies at your feet.";
	case 13: return "A figment of your imagination is at your command.";
	case 14: return "You notice a gravestone here... upon closer examination, it reads your name.";
	case 15: return "The long lost boots of Ranger Tarin lie off to the side.";
	case 16: return "A glowing tome of arcane knowledge hovers in the air before you.";
	case 17: return "A long sought secret of all mankind is now clear to you.";
	case 18: return "The meaning of it all, so simple, so clear... of course!";
	case 19: return "The answer.  One.  It's always been One.";
	case 20: return "The key to life, the universe and everything awaits your hand.";
    }
    return "Whoa!!!";
}


/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *List, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
	char **prgpstrShow;
	int *prgnShow;
	int *pitShow;
	char *pstrShow;
	OBJ_DATA *obj;
	int nShow;
	int iShow;
	int count, offcount, tmp, ms, cnt;
	bool fCombine;

	// don't bother with all this if there is no connection anyways
	if ( !ch->GetConnection() )
		return;

	/*
	 * if there's no list... then don't do all this crap!  -Thoric
	 */
	if ( !List )
	{
		if ( fShowNothing )
		{
			if ( IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE) )
				send_to_char( "     ", ch );
			send_to_char( "Nothing.\r\n", ch );
		}
		return;
	}
	/*
	 * Alloc space for output lines.
	 */
	count = 0;
	for ( obj = List; obj; obj = obj->next_content )
		count++;

	ms  = (ch->mental_state ? ch->mental_state : 1)
		* (IS_NPC(ch) ? 1 : (ch->pcdata->condition[COND_DRUNK] ? (ch->pcdata->condition[COND_DRUNK]/12) : 1));

	/*
	 * If not mentally stable...
	 */
	if ( abs(ms) > 40 )
	{
		offcount = URANGE( -(count), (count * ms) / 100, count*2 );
		if ( offcount < 0 )
			offcount += number_range(0, abs(offcount));
		else
			if ( offcount > 0 )
				offcount -= number_range(0, offcount);
	}
	else
		offcount = 0;

	if ( count + offcount <= 0 )
	{
		if ( fShowNothing )
		{
			if ( IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE) )
				send_to_char( "     ", ch );
			send_to_char( "Nothing.\r\n", ch );
		}
		return;
	}

	CREATE( prgpstrShow,	char*,	count + ((offcount > 0) ? offcount : 0) );
	CREATE( prgnShow,		int,	count + ((offcount > 0) ? offcount : 0) );
	CREATE( pitShow,		int,	count + ((offcount > 0) ? offcount : 0) );
	nShow	= 0;
	tmp		= (offcount > 0) ? offcount : 0;
	cnt		= 0;

	/*
	 * Format the list of objects.
	 */
	for ( obj = List; obj; obj = obj->next_content )
	{
		if ( offcount < 0 && ++cnt > (count + offcount) )
			break;
		if ( tmp > 0 && number_bits(1) == 0 )
		{
			prgpstrShow [nShow] = str_dup( halucinated_object(ms, fShort) );
			prgnShow	[nShow] = 1;
			pitShow	[nShow] = number_range( ITEM_LIGHT, ITEM_BOOK );
			nShow++;
			--tmp;
		}
		if ( obj->wear_loc == WEAR_NONE
				&& can_see_obj( ch, obj )
				&& !((IS_SET(obj->extra_flags_2, ITEM_INVISIBLE_TO_PLAYERS)) && (ch->level < LEVEL_IMMORTAL))
				&& (obj->item_type != ITEM_TRAP || IS_AFFECTED(ch, AFF_DETECTTRAPS) ) )
		{
			pstrShow = format_obj_to_char( obj, ch, fShort );
			fCombine = FALSE;

			if ( IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE) )
			{
				/*
				 * Look for duplicates, case sensitive.
				 * Matches tend to be near end so run loop backwords.
				 */
				for ( iShow = nShow - 1; iShow >= 0; iShow-- )
				{
					if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
					{
						prgnShow[iShow] += obj->count;
						fCombine = TRUE;
						break;
					}
				}
			}

			pitShow[nShow] = obj->item_type;
			/*
			 * Couldn't combine, or didn't want to.
			 */
			if ( !fCombine )
			{
				prgpstrShow [nShow] = str_dup( pstrShow );
				prgnShow    [nShow] = obj->count;
				nShow++;
			}
		}
	}

	if ( tmp > 0 )
	{
		int x;
		for ( x = 0; x < tmp; x++ )
		{
			prgpstrShow [nShow] = str_dup( halucinated_object(ms, fShort) );
			prgnShow	[nShow] = 1;
			pitShow	[nShow] = number_range( ITEM_LIGHT, ITEM_BOOK );
			nShow++;
		}
	}

	/*
	 * Output the formatted list.		-Color support by Thoric
	 */
	for ( iShow = 0; iShow < nShow; iShow++ )
	{
		switch(pitShow[iShow]) {
			default:
				set_char_color( AT_OBJECT, ch );
				break;
			case ITEM_BLOOD:
				set_char_color( AT_BLOOD, ch );
				break;
			case ITEM_MONEY:
			case ITEM_TREASURE:
				set_char_color( AT_YELLOW, ch );
				break;
			case ITEM_FOOD:
				set_char_color( AT_HUNGRY, ch );
				break;
			case ITEM_DRINK_CON:
			case ITEM_FOUNTAIN:
				set_char_color( AT_THIRSTY, ch );
				break;
			case ITEM_FIRE:
				set_char_color( AT_FIRE, ch );
				break;
			case ITEM_SCROLL:
			case ITEM_WAND:
			case ITEM_STAFF:
				set_char_color( AT_MAGIC, ch );
				break;
		}
		if ( fShowNothing )
			send_to_char( "     ", ch );
		send_to_char( prgpstrShow[iShow], ch );
		/*	if ( IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE) ) */
		{
			if ( prgnShow[iShow] != 1 )
				ch_printf( ch, " (%d)", prgnShow[iShow] );
		}

		send_to_char( "\r\n", ch );
		DISPOSE( prgpstrShow[iShow] );
	}

	if ( fShowNothing && nShow == 0 )
	{
		if ( IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE) )
			send_to_char( "     ", ch );
		send_to_char( "Nothing.\r\n", ch );
	}

	/*
	 * Clean up.
	 */
	DISPOSE( prgpstrShow );
	DISPOSE( prgnShow	 );
	DISPOSE( pitShow	 );
	return;
}


/*
 * Show fancy descriptions for certain spell affects		-Thoric
 */
char* get_visible_affects( CHAR_DATA *victim, CHAR_DATA *ch )
{
	char affects[MAX_STRING_LENGTH*4];
	char buf[MAX_STRING_LENGTH];

	strcpy(affects, "\0");
	strcpy(buf, "\0");

	if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
	{
		if ( IS_GOOD(victim) && can_see(ch, victim) )
		{
			sprintf( buf, "%s%s glows with an aura of divine radiance.\r\n",
				get_char_color(AT_WHITE, ch),
				victim->getShort().c_str() );
			strcat(affects, buf);
		}
		else if ( IS_EVIL(victim) && can_see(ch, victim) )
		{
			sprintf( buf, "%s%s shimmers beneath an aura of dark energy.\r\n",
				get_char_color(AT_WHITE, ch),
				victim->getShort().c_str() );
			strcat(affects, buf);
		}
		else if ( can_see(ch, victim) )
		{
			sprintf( buf, "%s%s is shrouded in flowing shadow and light.\r\n",
				get_char_color(AT_WHITE, ch),
				victim->getShort().c_str() );
			strcat(affects, buf);
		}
	}
	if ( IS_AFFECTED(victim, AFF_FIRESHIELD) )
	{
		if ( can_see(ch, victim) ) {
			sprintf( buf, "%s%s is engulfed within a blaze of mystical flame.\r\n",
				get_char_color(AT_FIRE, ch),
				victim->getShort().c_str() );
			strcat(affects, buf);
		} else {
			sprintf( buf, "%sa blob of mystical flame is burning before you.\r\n",
				get_char_color(AT_FIRE, ch));
			strcat(affects, buf);
		}
	}

	if ( victim->getScent() != NULL )
	{
		if ( can_see(ch, victim) )
		{
			sprintf( buf, "&w%s %s.&w\n\r", victim->getShort().c_str(),
					victim->getScent()->getDescription().c_str() );
			strcat(affects, buf);
		}
	}

	if ( IS_AFFECTED(victim, AFF_SHOCKSHIELD) && can_see(ch, victim) )
	{
		sprintf( buf, "%s%s is surrounded by cascading torrents of energy.\r\n",
			get_char_color(AT_BLUE, ch),
			victim->getShort().c_str() );
		strcat(affects, buf);
	}
	/*Scryn 8/13*/
	if ( IS_AFFECTED(victim, AFF_ICESHIELD) && can_see(ch, victim) )
	{
		sprintf( buf, "%s%s is ensphered by shards of glistening ice.\r\n",
			get_char_color(AT_LBLUE, ch),
			victim->getShort().c_str() );
		strcat(affects, buf);
	}
	if ( IS_AFFECTED(victim, AFF_CHARM)  && can_see(ch, victim) 	)
	{
		/*sprintf( buf, "%s%s wanders in a dazed, zombie-like state.\r\n",
			get_char_color(AT_MAGIC, ch),
			victim->getShort().c_str() );
		strcat(affects, buf);*/
	}
	if ( !IS_NPC(victim) && !victim->GetConnection()
		&&	  victim->SwitchedCharId != 0 && IS_AFFECTED( CharacterMap[victim->SwitchedCharId], AFF_POSSESS)
		&&	  can_see(ch, victim) )
	{
		strcpy( buf, get_char_color(AT_MAGIC, ch));
		strcat( buf, PERS( victim, ch ) );
		strcat( buf, " appears to be in a deep trance...\r\n" );
		strcat(affects, buf);
	}

	return STRALLOC(affects);
}

const char * player_title(const char * str)
{
	const char *p;
	p=str;
	if(p && p[0]!='\0' && p[1]==':')
		p=p+2;
	return(p);
}

COMBINE_CHARS* show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];
	char buf1[MAX_STRING_LENGTH];
	COMBINE_CHARS* item;

	buf[0] = '\0';

	CREATE(item, COMBINE_CHARS, 1);

	strcpy(buf, get_char_color(AT_PERSON, ch));

	if ( victim->IsSnooped() )
	{
		if(ch->GetConnection()
			&& ch->GetConnection()->Account
			&& ch->GetConnection()->Account->flags & ACCOUNT_NOTIFY
			&& !ch->IsSnooped()
			)
		{
			strcat( buf, "~~Snooped~~ " );
		}
	}

	if ( !IS_NPC(victim) && !victim->GetConnection() )
	{
		Character * switched = CharacterMap[victim->SwitchedCharId];

		if ( !switched )
			strcat( buf, "(Link Dead) "  );
		else if ( !IS_AFFECTED( switched, AFF_POSSESS) )
			strcat( buf, "(Switched) " );
	}

	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_AFK) )
		strcat( buf, "[AFK] ");

	if ( (!IS_NPC(victim) && IS_SET(victim->act, PLR_WIZINVIS))
		|| (IS_NPC(victim) && IS_SET(victim->act, ACT_MOBINVIS)) )
	{
		if (!IS_NPC(victim))
			sprintf( buf1,"(Invis %d) ", victim->pcdata->wizinvis );
		else
			sprintf( buf1,"(Mobinvis %d) ", victim->mobinvis);
		strcat( buf, buf1 );
	}
	if ( IS_AFFECTED(victim, AFF_INVISIBLE)   )
		strcat( buf, "(Invis) " 	 );
	if ( IS_AFFECTED(victim, AFF_HIDE)		  )
		strcat( buf, "(Hide) "		 );
	if ( IS_AFFECTED(victim, AFF_PASS_DOOR)   )
		strcat( buf, "(Translucent) ");
	if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) )
		strcat( buf, "(Pink Aura) "  );
	if ( IS_EVIL(victim) && IS_AFFECTED(ch, AFF_DETECT_EVIL) )
		strcat( buf, "(Red Aura) "	 );
	if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_LITTERBUG  ) )
		strcat( buf, "(LITTERBUG) "  );
	if ( IS_NPC(victim) && IS_IMMORTAL(ch) && IS_SET(victim->act, ACT_PROTOTYPE) )
		strcat( buf, "(PROTO) " );
	if ( victim->GetConnection() && victim->GetConnection()->ConnectedState == CON_EDITING )
		strcat( buf, "(Writing) " );

	if ( victim->position == victim->defposition && victim->longDesc_.length() > 0 )
	{
		// Special case for mounts...    -Ksilyan
		if ( ch->GetMount() == victim )
		{
			strcat( buf, "You are mounted upon ");
			strcat( buf, PERS( victim, ch ) );
			strcat( buf, ".");
			item->line	  = STRALLOC(buf);
			item->affects = get_visible_affects( victim, ch);
		}
		else
		{
			strcat( buf, victim->longDesc_.c_str() );
			item->line	  = STRALLOC(buf);
			item->affects = get_visible_affects( victim, ch);
		}
		return item;
	}

	strcat( buf, capitalize(PERS( victim, ch )) );

	if ( !IS_NPC(victim) && !IS_SET(ch->act, PLR_BRIEF) )
	{
		// for immortals, display the title
		if ( IS_IMMORTAL(victim) )
			strcat( buf, player_title(victim->pcdata->title_.c_str()) );
		// else, display the race and class
		else
		{
			string buf2;
			string raceName = race_table[victim->race].race_name;

			buf2 = ", " + string( isavowel(raceName[0]) ? "an " : "a " ) + raceName + ",";
			//buf2 += string(class_table[victim->Class]->who_name) + ")";
			strcat( buf, buf2.c_str() );
		}
	}

	switch ( victim->position )
	{
		case POS_DEAD:	   strcat( buf, " is DEAD!!" ); 		break;
		case POS_MORTAL:   strcat( buf, " is mortally wounded." );		break;
		case POS_INCAP:    strcat( buf, " is incapacitated." ); 	break;
		case POS_STUNNED:  strcat( buf, " is lying here stunned." );	break;
		case POS_SLEEPING:
			if (ch->position == POS_SITTING
				||	ch->position == POS_RESTING )
				strcat( buf, " is sleeping nearby." );
			else
				strcat( buf, " is deep in slumber here." );
			break;
		case POS_RESTING:
			if ( ch->GetSittingOn() && ch->GetSittingOn() == victim->GetSittingOn() )
			{
				Object * sitting_on = victim->GetSittingOn();
				char obj[MAX_STRING_LENGTH];
				sprintf(obj, " is sprawled out on %s beside you.", sitting_on->shortDesc_.c_str() );
				strcat(buf, obj);
			}
			else if ( victim->GetSittingOn() )
			{
				Object * sitting_on = victim->GetSittingOn();

				char obj[MAX_STRING_LENGTH];
				sprintf(obj, " is sprawled out on %s before you.", sitting_on->shortDesc_.c_str());
				strcat(buf, obj);
			}
			else if (ch->position == POS_RESTING && !victim->GetSittingOn() )
			{
				strcat ( buf, " is sprawled out alongside you." );
			}
			else
			{
				if (ch->position == POS_MOUNTED)
				{
					strcat ( buf, " is sprawled out at the foot of your mount." );
				}
				else
				{
					strcat (buf, " is sprawled out here." );
				}
			}
			break;
		case POS_MEDITATING:
			strcat (buf, " is tied up like a knot, contemplating the cosmos.");
			break;
		case POS_SITTING:
			if ( ch->GetSittingOn() && ch->GetSittingOn() == victim->GetSittingOn() )
			{
				Object * sitting_on = victim->GetSittingOn();

				char obj[MAX_STRING_LENGTH];
				sprintf(obj, " sits on %s with you.", sitting_on->shortDesc_.c_str());
				strcat(buf, obj);
			}
			else if ( victim->GetSittingOn() != 0 )
			{
				Object * sitting_on = victim->GetSittingOn();
				char obj[MAX_STRING_LENGTH];
				sprintf(obj, " sits on %s before you.", sitting_on->shortDesc_.c_str());
				strcat(buf, obj);
			}
			else if (ch->position == POS_SITTING && !victim->GetSittingOn())
			{
				strcat( buf, " sits here with you." );
			}
			else if (ch->position == POS_RESTING && !victim->GetSittingOn())
			{
				strcat( buf, " sits nearby as you lie around." );
			}
			else
			{
				strcat( buf, " sits upright here." );
			}
			break;
		case POS_STANDING:
			if ( IS_IMMORTAL(victim) )
				strcat( buf, " is here before you." );
			else if ( ( victim->GetInRoom()->sector_type == SECT_UNDERWATER )
				&& !IS_AFFECTED(victim, AFF_AQUA_BREATH) && !IS_NPC(victim) )
				strcat( buf, " is drowning here." );
			else if ( victim->GetInRoom()->sector_type == SECT_UNDERWATER )
				strcat( buf, " is here in the water." );
			else if ( ( victim->GetInRoom()->sector_type == SECT_OCEANFLOOR )
				&& !IS_AFFECTED(victim, AFF_AQUA_BREATH) && !IS_NPC(victim) )
				strcat( buf, " is drowning here." );
			else if ( victim->GetInRoom()->sector_type == SECT_OCEANFLOOR )
				strcat( buf, " is standing here in the water." );
			else if ( IS_AFFECTED(victim, AFF_FLOATING)
				|| IS_AFFECTED(victim, AFF_FLYING) )
				strcat( buf, " is hovering here." );
			else
				strcat( buf, " is standing here." );
			break;
		case POS_SHOVE:    strcat( buf, " is being shoved around." );	break;
		case POS_DRAG:	   strcat( buf, " is being dragged around." );	break;
		case POS_MOUNTED:
			strcat( buf, " is here, upon " );
			if ( !victim->GetMount() )
				strcat( buf, "thin air???" );
			else
			{
				Character * mount = victim->GetMount();
				if ( mount == ch )
					strcat( buf, "your back." );
				else if ( victim->GetInRoom() == mount->GetInRoom() )
				{
					strcat( buf, PERS( mount, ch ) );
					strcat( buf, "." );
				}
				else
					strcat( buf, "someone who left??" );
				break;
			}
		case POS_FIGHTING:
			strcat( buf, " is here, fighting " );
			if ( !victim->GetVictim() )
				strcat( buf, "thin air???" );
			else if ( victim->GetVictim() == ch )
				strcat( buf, "YOU!" );
			else if ( victim->GetInRoom() == victim->GetVictim()->GetInRoom() )
			{
				strcat( buf, PERS( victim->GetVictim(), ch ) );
				strcat( buf, "." );
			}
			else
				strcat( buf, "someone who left??" );
			break;
	}

	buf[0] = UPPER(buf[0]);

	item->line	  = STRALLOC(buf);
	item->affects = get_visible_affects( victim, ch);
	return item;
}



void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
	OBJ_DATA *obj;
	int iWear;
	bool found;
	const char *pdescr;

	if ( victim == ch )
	{
		act( AT_ACTION, "You look at yourself.", ch, NULL, victim, TO_VICT	 );
		act( AT_ACTION, "$n looks at $mself.",	ch, NULL, victim, TO_NOTVICT );
	}
	else if ( can_see( victim, ch ) )
	{
		act( AT_ACTION, "$n looks at you.", ch, NULL, victim, TO_VICT	 );
		act( AT_ACTION, "$n looks at $N.",	ch, NULL, victim, TO_NOTVICT );
	}

	if ( victim->QuestNoteId != 0 && (pdescr=get_extra_descr("_text_",
		ObjectMap[victim->QuestNoteId]->first_extradesc
		)
		)!=NULL
		)
	{
		send_to_char( pdescr, ch );
	}
	else if ( victim->description_.length() > 0 )
	{
		send_to_char( victim->description_.c_str(), ch );
	}
	else
	{
		act( AT_PLAIN, "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
	}

	show_condition( ch, victim );

	found = FALSE;
	for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
	{
		if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
			&&	 can_see_obj( ch, obj ) )
		{
		/* Ksilyan:
		* Don't show sheathed weapons if they're not yours.
			*/
			if (( obj->wear_loc == WEAR_SHEATHED_1 || obj->wear_loc == WEAR_SHEATHED_2) && (ch != victim) )
				continue;
			if ( !found )
			{
				send_to_char( "\r\n", ch );
				act( AT_PLAIN, "$N is using:", ch, NULL, victim, TO_CHAR );
				found = TRUE;
			}
			send_to_char( where_name[iWear], ch );
			send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
			send_to_char( "\r\n", ch );
		}
	}

	/*
	* Crash fix here by Thoric
	*/
	if ( IS_NPC(ch) || victim == ch )
		return;

	if ( ch->ChanceRollSkill(gsn_peek) )
	{
		send_to_char( "\r\nYou peek at the inventory:\r\n", ch );
		show_list_to_char( victim->first_carrying, ch, TRUE, TRUE );
		learn_from_success( ch, gsn_peek );
	}
	else if ( ch->pcdata->learned[gsn_peek] )
		learn_from_failure( ch, gsn_peek );

	return;
}

void show_char_to_char(CHAR_DATA *List, CHAR_DATA* ch)
{
    CHAR_DATA *rch;
    COMBINE_CHARS *head;
    COMBINE_CHARS *tmp;
    COMBINE_CHARS *cur = tmp = head = NULL;
    int red_forms = 0;

    for ( rch = List; rch; rch = rch->next_in_room )
    {
	    if ( rch == ch )
	        continue;

	    if ( can_see( ch, rch ) )
	    {
	        tmp = show_char_to_char_0( rch, ch );

            for ( cur = head; cur; cur = cur->next ) {
                if ( !strcmp(cur->line, tmp->line)
                  && !strcmp(cur->affects, tmp->affects)
                  && !IS_NPC(ch)
                  && IS_SET(ch->act, PLR_COMBINE))
                {
                    cur->count++;
                    STRFREE(tmp->line);
                    STRFREE(tmp->affects);
                    DISPOSE(tmp);
                    break;
                }
            }

            if ( !cur ) {
                tmp->next = head;
                head = tmp;
            }
	    }
	    else if ( room_is_dark( ch->GetInRoom() )
	    &&        IS_AFFECTED(rch, AFF_INFRARED ) )
	    {
            if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_COMBINE) ) {
                red_forms++;
            } else {
	            set_char_color( AT_BLOOD, ch );
	            send_to_char( "The red form of a living creature is here.\r\n", ch );
            }
	    }
    }

    for ( cur = head; cur; ) {
        char buf[MAX_STRING_LENGTH];
        char* ptr = buf;
        strcpy(buf, cur->line);

        while(*ptr) ptr++;
        ptr--;
        while(*ptr=='\r' || *ptr == '\n') { *ptr = '\0'; ptr--; }

        if ( cur->count != 0 ) {
            ch_printf(ch, "%s (%d)\r\n%s",
                    buf, cur->count+1, cur->affects);
        } else {
            ch_printf(ch, "%s\r\n%s", buf, cur->affects);
        }
        tmp = cur->next;
        STRFREE(cur->line);
        STRFREE(cur->affects);
        DISPOSE(cur);
        cur = tmp;
    }

    return;
}



bool check_blind( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) )
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_TRUESIGHT) )
      return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    {
	send_to_char( "You can't see a thing!\r\n", ch );
	return FALSE;
    }

    return TRUE;
}

/*
 * Returns classical DIKU door direction based on text in arg	-Thoric
 */
int get_door( const char *arg )
{
    int door;

	 if ( !str_cmp( arg, "n"  ) || !str_cmp( arg, "north"	  ) ) door = 0;
    else if ( !str_cmp( arg, "e"  ) || !str_cmp( arg, "east"	  ) ) door = 1;
    else if ( !str_cmp( arg, "s"  ) || !str_cmp( arg, "south"	  ) ) door = 2;
    else if ( !str_cmp( arg, "w"  ) || !str_cmp( arg, "west"	  ) ) door = 3;
    else if ( !str_cmp( arg, "u"  ) || !str_cmp( arg, "up"	  ) ) door = 4;
    else if ( !str_cmp( arg, "d"  ) || !str_cmp( arg, "down"	  ) ) door = 5;
    else if ( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast" ) ) door = 6;
    else if ( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest" ) ) door = 7;
    else if ( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast" ) ) door = 8;
    else if ( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest" ) ) door = 9;
    else door = -1;
    return door;
}

void do_exits(CHAR_DATA *ch, const char* argument)
{
	char buf[MAX_STRING_LENGTH];
	ExitData *pexit;
	bool found;
	bool fAuto;

	set_char_color( AT_EXITS, ch );
	buf[0] = '\0';
	fAuto  = !str_cmp( argument, "auto" );

	if ( !check_blind( ch ) )
		return;

	strcpy( buf, fAuto ? "Exits:" : "Obvious exits:\r\n" );

	found = FALSE;
	for ( pexit = ch->GetInRoom()->first_exit; pexit; pexit = pexit->next )
	{
		if ( pexit->to_room
				&& (!IS_SET(pexit->exit_info, EX_WINDOW)
				||   IS_SET(pexit->exit_info, EX_ISDOOR))
				&&  !IS_SET(pexit->exit_info, EX_HIDDEN)
				&&  !IS_SET(pexit->exit_info, EX_SECRET))
		{
			found = TRUE;
			if ( fAuto )
			{
				strcat( buf, " " );
				if (IS_SET(pexit->exit_info, EX_ISDOOR))
					strcat(buf, IS_SET(pexit->exit_info, EX_CLOSED) ? "[" : "]");

				if (IS_SET(pexit->exit_info, EX_ISDOOR))
					strcat( buf, MXPTAG("ExDoor"));
				else
					strcat( buf, MXPTAG("Ex"));
				strcat( buf, dir_name[pexit->vdir] );

				if (IS_SET(pexit->exit_info, EX_ISDOOR))
					strcat( buf, MXPTAG("/ExDoor"));
				else
					strcat( buf, MXPTAG("/Ex"));

				if (IS_SET(pexit->exit_info, EX_ISDOOR))
					strcat(buf, IS_SET(pexit->exit_info, EX_CLOSED) ? "]" : "[");
			}
			else
			{
				sprintf( buf + strlen(buf), "%-10s -%s%s\r\n",
						capitalize( dir_name[pexit->vdir] ),
						IS_SET(pexit->exit_info, EX_ISDOOR)
							? IS_SET(pexit->exit_info, EX_CLOSED) ? " (closed) " : " (open) "
							: " ",
						room_is_dark( pexit->to_room )
							?  "Too dark to tell"
							: pexit->to_room->name_.c_str()
						);
			}
		}
	}

	if ( !found )
		strcat( buf, fAuto ? " none.\r\n" : "None.\r\n" );
	else if ( fAuto )
		strcat( buf, ".\r\n" );
	send_to_char( buf, ch );
	return;
}

/* AIYAN - SCENT SYSTEM - 30/03/05 */

void do_smell(CHAR_DATA *ch, const char* argument)
{
	CHAR_DATA *victim;
	Scent *aScent;

	if ( argument[0] == '\0' )
	{
		send_to_char("You sniff the air...\n\r", ch);

		if (( aScent = ScentController::instance()->findScent(ch->GetInRoom()->scentID)) == NULL )
		{
			send_to_char("The air smells of nothing in particular.\n\r", ch);
			return;
		}

		ch_printf(ch, "The air %s.\n\r", aScent->getDescription().c_str());
		return;
	}

	if (( victim = get_char_room( ch, argument ) ) == NULL )
	{
		send_to_char("There is no-one here by that name.\n\r", ch);
		return;
	}

	if ( victim == ch )
	{
		act( AT_ACTION, "You sniff yourself.", ch, NULL, victim, TO_VICT );
		act( AT_ACTION, "$n sniffs $mself.", ch, NULL, victim, TO_NOTVICT);
	}
	else
	{
		act( AT_ACTION, "$n sniffs you.", ch, NULL, victim, TO_VICT );
		act( AT_ACTION, "$n sniffs $N.", ch, NULL, victim, TO_NOTVICT);
	}

	if ( victim->getScent() == NULL )
	{
		ch_printf(ch, "%s smells of nothing in particular.\n\r", victim->getShort().c_str() );
		return;
	}

	ch_printf(ch, "%s %s.\n\r", victim->getShort().c_str(), victim->getScent()->getDescription().c_str());

	return;
}

void do_look(CHAR_DATA *ch, const char* argument)
{
	char arg  [MAX_INPUT_LENGTH];
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	ExitData *pexit;
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	ROOM_INDEX_DATA *original;
	const char *pdesc;
	bool doexaprog;
	sh_int door;
	int number, cnt;

	if ( !ch->GetConnection() )
		return;

	if ( ch->position < POS_SLEEPING )
	{
		send_to_char( "You can't see anything but stars!\r\n", ch );
		return;
	}

	if ( ch->position == POS_SLEEPING )
	{
		send_to_char( "You can't see anything, you're sleeping!\r\n", ch );
		return;
	}

	if ( !check_blind( ch ) )
		return;

	if ( !IS_NPC(ch)
			&&	 !IS_SET(ch->act, PLR_HOLYLIGHT)
			&&	 !IS_AFFECTED(ch, AFF_TRUESIGHT)
			&&	 room_is_dark( ch->GetInRoom() ) )
	{
		set_char_color( AT_DGREY, ch );
		send_to_char( "It is pitch black ... \r\n", ch );
		show_char_to_char( ch->GetInRoom()->first_person, ch );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );

	doexaprog = str_cmp( "noprog", arg2 ) && str_cmp( "noprog", arg3 );

	if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
	{
		/* Ibuild commented by Ksilyan
		   switch ( ch->inter_page )	 // rmenu
		   {
		   case ROOM_PAGE_A : do_rmenu(ch,"a");
		   break;
		   case ROOM_PAGE_B : do_rmenu(ch,"b");
		   break;
		   case ROOM_PAGE_C : do_rmenu(ch,"c");
		   break;
		   }*/
		/* 'look' or 'look auto' */
		set_char_color( AT_RMNAME, ch );
		send_to_char( ch->GetInRoom()->name_.c_str(), ch );
		send_to_char( "\r\n", ch );
		if ( weather_info.sunlight == SUN_DARK )
		{
			set_char_color( AT_NIGHT, ch );
		}
		else
		{
			set_char_color( AT_RMDESC, ch );
		}

		if ( arg1[0] == '\0'
				|| ( !IS_NPC(ch) && !IS_SET(ch->act, PLR_BRIEF) ) )
		{
			if ( ObjectMap[ch->GetInRoom()->QuestNoteId]
					&& (pdesc=get_extra_descr("_text_",
							ObjectMap[ch->GetInRoom()->QuestNoteId]->first_extradesc
							)
					   )!=NULL
			   )
			{
				send_to_char(pdesc,ch);
			}
			else
			{
				if ( (pdesc = get_extra_descr("desc", ch->GetInRoom()->first_extradesc)) )
				{
					send_to_char(pdesc, ch);
				}
				else if ( ch->GetInRoom()->random_room_type > 0 )
				{
					int roomtype;
					int descnumber;
					AREA_DATA * area;

					area = ch->GetInRoom()->area;

					descnumber = ch->GetInRoom()->random_description;
					roomtype = ch->GetInRoom()->random_room_type;

					if ( weather_info.sunlight == SUN_DARK
							&& area->random_night_descriptions[roomtype][descnumber] != NULL )
					{
						send_to_char(area->random_night_descriptions[roomtype][descnumber], ch);
					}
					else
					{
						if ( area->random_descriptions[roomtype][descnumber] == NULL )
						{
							ostringstream os;

							os <<"Area " << area->name << " doesn't have a random desc "
								<< "for type #" << roomtype << ", desc #" << descnumber << "!";
							gTheWorld->LogBugString(os.str());
							ch->sendText("(error)");
						}
						else
							send_to_char(area->random_descriptions[roomtype][descnumber], ch);
					}
				}
				else
				{
					send_to_char( ch->GetInRoom()->description_.c_str(), ch );
				}
			}
		}

		if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOMAP) )	 /* maps */
		{
			if(ch->GetInRoom()->map != NULL)
			{
				do_lookmap(ch, NULL);
			}
		}

		if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT) )
			do_exits( ch, "auto" );


		show_list_to_char( ch->GetInRoom()->first_content, ch, FALSE, FALSE );
		show_char_to_char( ch->GetInRoom()->first_person,  ch );
		return;
	}

	if ( !str_cmp( arg1, "under" ) )
	{
		int count;

		/* 'look under' */
		if ( arg2[0] == '\0' )
		{
			send_to_char( "Look beneath what?\r\n", ch );
			return;
		}

		if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
		{
			send_to_char( "You do not see that here.\r\n", ch );
			return;
		}
		if ( ch->carry_weight + obj->weight > can_carry_w( ch ) )
		{
			send_to_char( "It's too heavy for you to look under.\r\n", ch );
			return;
		}
		count = obj->count;
		obj->count = 1;
		act( AT_PLAIN, "You lift $p and look beneath it:", ch, obj, NULL, TO_CHAR );
		act( AT_PLAIN, "$n lifts $p and looks beneath it.", ch, obj, NULL, TO_ROOM );
		obj->count = count;
		if ( IS_OBJ_STAT( obj, ITEM_COVERING ) )
			show_list_to_char( obj->first_content, ch, TRUE, TRUE );
		else
			send_to_char( "Nothing.\r\n", ch );
		if ( doexaprog ) oprog_examine_trigger( ch, obj );
		return;
	}

	if ( !str_cmp( arg1, "i" ) || !str_cmp( arg1, "in" ) )
	{
		int count;

		/* 'look in' */
		if ( arg2[0] == '\0' )
		{
			send_to_char( "Look in what?\r\n", ch );
			return;
		}

		if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
		{
			send_to_char( "You do not see that here.\r\n", ch );
			return;
		}

		switch ( obj->item_type )
		{
			default:
				send_to_char( "That is not a container.\r\n", ch );
				break;

			case ITEM_DRINK_CON:
				if ( obj->value[1] <= 0 )
				{
					send_to_char( "It is empty.\r\n", ch );
					if ( doexaprog ) oprog_examine_trigger( ch, obj );
					break;
				}

				ch_printf( ch, "It's %s full of a %s liquid.\r\n",
						obj->value[1] < 	obj->value[0] / 4
						? "less than" :
						obj->value[1] < 3 * obj->value[0] / 4
						? "about"	  : "more than",
						liq_table[obj->value[2]].liq_color
						);

				if ( doexaprog ) oprog_examine_trigger( ch, obj );
				break;

			case ITEM_PORTAL:
				for ( pexit = ch->GetInRoom()->first_exit; pexit; pexit = pexit->next )
				{
					if ( pexit->vdir == DIR_PORTAL
							&&	 IS_SET(pexit->exit_info, EX_PORTAL) )
					{
						if ( room_is_private( pexit->to_room )
								&&	 get_trust(ch) < sysdata.level_override_private )
						{
							set_char_color( AT_WHITE, ch );
							send_to_char( "That room is private buster!\r\n", ch );
							return;
						}
						original = ch->GetInRoom();
						char_from_room( ch );
						char_to_room( ch, pexit->to_room );
						do_look( ch, "auto" );
						char_from_room( ch );
						char_to_room( ch, original );
						return;
					}
				}
				send_to_char( "You see a swirling chaos...\r\n", ch );
				break;
			case ITEM_CONTAINER:
			case ITEM_CORPSE_NPC:
			case ITEM_CORPSE_PC:
				if ( IS_SET(obj->value[1], CONT_CLOSED) )
				{
					send_to_char( "It is closed.\r\n", ch );
					break;
				}
			case ITEM_SCABBARD:
			case ITEM_QUIVER:

				count = obj->count;
				obj->count = 1;
				act( AT_PLAIN, "$p contains:", ch, obj, NULL, TO_CHAR );
				obj->count = count;
				show_list_to_char( obj->first_content, ch, TRUE, TRUE );
				if ( doexaprog ) oprog_examine_trigger( ch, obj );
				break;
			case ITEM_MEAT:
				{
					switch ( obj->value[4] ) {
						case COOKED_DRIPPING:
							send_to_char("It's dripping bright red blood.\r\n", ch);
							break;
						case COOKED_RAW:
							send_to_char("It's soaking wet with blood.\r\n", ch);
							break;
						case COOKED_RARE:
							send_to_char("It's a little underdone, but edible.\r\n", ch);
							break;
						case COOKED_MEDIUM:
							send_to_char("It's cooked perfectly.\r\n", ch);
							break;
						case COOKED_CHARED:
							send_to_char("It's burned too much to be edible.\r\n", ch);
							break;
						default:
							send_to_char("You're not sure about it.\r\n", ch);
							break;
					}
				}
		}
		return;
	}

	if ( (pdesc=get_extra_descr(arg1, ch->GetInRoom()->first_extradesc)) != NULL )
	{
		send_to_char( pdesc, ch );
		return;
	}

	door = get_door( arg1 );
	if ( ( pexit = find_door( ch, arg1, TRUE ) ) != NULL )
	{
		if ( pexit->keyword_.length() > 0 && pexit->keyword_.c_str()[0] != ' ' )
		{
			if ( IS_SET(pexit->exit_info, EX_CLOSED)
					&&	!IS_SET(pexit->exit_info, EX_WINDOW) )
			{
				if ( IS_SET(pexit->exit_info, EX_SECRET)
						&&	 door != -1 )
					send_to_char( "Nothing special there.\r\n", ch );
				else
					act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword_.c_str(), TO_CHAR );
				return;
			}
			if ( IS_SET( pexit->exit_info, EX_BASHED ) )
				act(AT_RED, "The $d has been bashed from its hinges!",ch, NULL, pexit->keyword_.c_str(), TO_CHAR);
		}

		if ( pexit->description_.length() > 0 )
			send_to_char( pexit->description_.c_str(), ch );
		else
			send_to_char( "Nothing special there.\r\n", ch );

		/*
		 * Ability to look into the next room			-Thoric
		 */
		if ( pexit->to_room
				&& ( IS_AFFECTED( ch, AFF_SCRYING )
					||	 IS_SET( pexit->exit_info, EX_xLOOK )
					||	 get_trust(ch) >= LEVEL_IMMORTAL ) )
		{
			if ( !IS_SET( pexit->exit_info, EX_xLOOK )
					&&	  get_trust( ch ) < LEVEL_IMMORTAL )
			{
				set_char_color( AT_MAGIC, ch );
				send_to_char( "You attempt to scry...\r\n", ch );
				/* Change by Narn, Sept 96 to allow characters who don't have the
				   scry spell to benefit from objects that are affected by scry.
				 */
				if (!IS_NPC(ch) )
				{
					int percent = ch->pcdata->learned[ skill_lookup("scry") ];
					if ( !percent )
						percent = 55;		/* 95 was too good -Thoric */

					if(  number_percent( ) > percent )
					{
						send_to_char( "You fail.\r\n", ch );
						return;
					}
				}
			}
			if ( room_is_private( pexit->to_room )
					&&	 get_trust(ch) < sysdata.level_override_private )
			{
				set_char_color( AT_WHITE, ch );
				send_to_char( "That room is private buster!\r\n", ch );
				return;
			}
			original = ch->GetInRoom();
			char_from_room( ch );
			char_to_room( ch, pexit->to_room );
			do_look( ch, "auto" );
			char_from_room( ch );
			char_to_room( ch, original );
		}
		return;
	}
	else if ( door != -1 )
	{
		send_to_char( "Nothing special there.\r\n", ch );
		return;
	}

	if ( ( victim = get_char_room( ch, arg1 ) ) != NULL )
	{
		show_char_to_char_1( victim, ch );
		mprog_look_trigger(ch, victim);

		return;
	}


	/* finally fixed the annoying look 2.obj desc bug	-Thoric */
	number = number_argument( arg1, arg );
	for ( cnt = 0, obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if ( can_see_obj( ch, obj ) )
		{
			if ( (pdesc=get_extra_descr(arg, obj->first_extradesc)) != NULL )
			{
				if ( (cnt += obj->count) < number )
					continue;
				send_to_char( pdesc, ch );
				if ( doexaprog ) oprog_examine_trigger( ch, obj );
				return;
			}

			if ( (pdesc=get_extra_descr(arg, obj->pIndexData->first_extradesc)) != NULL )
			{
				if ( (cnt += obj->count) < number )
					continue;
				send_to_char( pdesc, ch );
				if ( doexaprog ) oprog_examine_trigger( ch, obj );
				return;
			}
			if ( nifty_is_name_prefix( arg, obj->name_.c_str() ) )
			{
				if ( (cnt += obj->count) < number )
					continue;

				/* KSILYAN
				   added in television room looking
				 */

				if ( !obj->TelevisionRoomId )
				{
					if (obj->item_type == ITEM_GEM)
					{
						set_char_color(AT_PLAIN, ch);
						ch_printf(ch, "%s is %s and %s.\r\n", capitalize(obj->shortDesc_.c_str()),
								gem_quality_number_to_name(obj->value[OBJECT_GEM_QUALITY]),
								gem_size_number_to_name(obj->value[OBJECT_GEM_SIZE]));
					}

					pdesc = get_extra_descr( obj->name_.c_str(), obj->pIndexData->first_extradesc );
					if ( !pdesc )
						pdesc = get_extra_descr( obj->name_.c_str(), obj->first_extradesc );
					if ( !pdesc )
					{
						if (!IS_SET(obj->extra_flags_2, ITEM_INVISIBLE_TO_PLAYERS))
						{
							/* KSILYAN
							   We don't want to show the "nothing special" to the player if it's invisble...
							   Also don't want to show for gems.
							 */
							if (obj->item_type != ITEM_GEM)
								send_to_char( "You see nothing special.\r\n", ch );
						}
					}
					else
						send_to_char( pdesc, ch );
				}
				if ( doexaprog ) oprog_examine_trigger( ch, obj );
				return;
			}
		}
	}

	for ( obj = ch->GetInRoom()->last_content; obj; obj = obj->prev_content )
	{
		if ( can_see_obj( ch, obj ) )
		{
			if ( (pdesc=get_extra_descr(arg, obj->first_extradesc)) != NULL )
			{
				if ( (cnt += obj->count) < number )
					continue;
				send_to_char( pdesc, ch );
				if ( doexaprog ) oprog_examine_trigger( ch, obj );
				return;
			}

			if ( (pdesc=get_extra_descr(arg, obj->pIndexData->first_extradesc)) != NULL )
			{
				if ( (cnt += obj->count) < number )
					continue;
				send_to_char( pdesc, ch );
				if ( doexaprog ) oprog_examine_trigger( ch, obj );
				return;
			}
			if ( nifty_is_name_prefix( arg, obj->name_.c_str() ) )
			{
				if ( (cnt += obj->count) < number )
					continue;

				/* KSILYAN
				   Television room stuff
				 */

				if ( !obj->TelevisionRoomId )
				{
					if (obj->item_type == ITEM_GEM)
					{
						set_char_color(AT_PLAIN, ch);
						ch_printf(ch, "%s is %s and %s.\r\n", capitalize(obj->shortDesc_.c_str()),
								gem_quality_number_to_name(obj->value[OBJECT_GEM_QUALITY]),
								gem_size_number_to_name(obj->value[OBJECT_GEM_SIZE]));
					}

					pdesc = get_extra_descr( obj->name_.c_str(), obj->pIndexData->first_extradesc );
					if ( !pdesc )
						pdesc = get_extra_descr( obj->name_.c_str(), obj->first_extradesc );
					if ( !pdesc )
					{
						if (!IS_SET(obj->extra_flags_2, ITEM_INVISIBLE_TO_PLAYERS))
						{
							/*
							 * We don't want to show the "nothing special" if we're invis.
							 * Also don't want to show for gems.
							 *    -Ksilyan
							 */
							if (obj->item_type != ITEM_GEM)
								send_to_char( "You see nothing special.\r\n", ch );
						}
					}
					else
						send_to_char( pdesc, ch );
					if ( doexaprog ) oprog_examine_trigger( ch, obj );
					return;
				}
			}
		}
	}

	send_to_char( "You do not see that here.\r\n", ch );
	return;
}

// show_condition function updated by Ksilyan
void show_condition( CHAR_DATA *ch, CHAR_DATA *victim )
{
	string buf;

	int hp_percent;
	int mv_percent;

	if ( victim->max_hit > 0 )
		hp_percent = ( 100 * victim->hit ) / victim->max_hit;
	else
		hp_percent = -1;

	if (victim->max_move > 0 )
		mv_percent = ( 100 * victim->move ) / victim->max_move;
	else
		mv_percent = -1;

	buf += PERS(victim, ch);

	// Added inanimate damage messages    -Ksilyan
	if ( IS_NPC(victim) && IS_SET(victim->act, ACT_INANIMATE) )
	{
		if ( hp_percent >= 100 )      buf += " is in perfect condition.\r\n";
		else if ( hp_percent >=  90 ) buf += " is slightly scratched.\r\n";
		else if ( hp_percent >=  80 ) buf += " has a few dents.\r\n";
		else if ( hp_percent >=  70 ) buf += " is quite dented.\r\n";
		else if ( hp_percent >=  60 ) buf += " is damaged.\r\n";
		else if ( hp_percent >=  50 ) buf += " is very damaged.\r\n";
		else if ( hp_percent >=  40 ) buf += " is holed in places.\r\n";
		else if ( hp_percent >=  30 ) buf += " is beginning to break.\r\n";
		else if ( hp_percent >=  20 ) buf += " is starting to give way.\r\n";
		else if ( hp_percent >=  10 ) buf += " is almost in pieces.\r\n";
		else                          buf += " is BREAKING.\r\n";
	}
	else
	{
		if ( hp_percent >= 100 )      buf += " is in perfect health.\r\n";
		else if ( hp_percent >=  90 ) buf += " is slightly scratched.\r\n";
		else if ( hp_percent >=  80 ) buf += " has a few bruises.\r\n";
		else if ( hp_percent >=  70 ) buf += " has some cuts.\r\n";
		else if ( hp_percent >=  60 ) buf += " has several wounds.\r\n";
		else if ( hp_percent >=  50 ) buf += " has many nasty wounds.\r\n";
		else if ( hp_percent >=  40 ) buf += " is bleeding freely.\r\n";
		else if ( hp_percent >=  30 ) buf += " is covered in blood.\r\n";
		else if ( hp_percent >=  20 ) buf += " is leaking guts.\r\n";
		else if ( hp_percent >=  10 ) buf += " is almost dead.\r\n";
		else                          buf += " is DYING.\r\n";

		buf += PERS(victim, ch);
		if ( mv_percent >= 100 )     buf += " seems to be full of energy.\r\n";
		else if ( mv_percent >= 90 ) buf += " appears well-rested.\r\n";
		else if ( mv_percent >= 80 ) buf += " is a little out of breath.\r\n";
		else if ( mv_percent >= 70 ) buf += " appears a little tired.\r\n";
		else if ( mv_percent >= 60 ) buf += " looks weary.\r\n";
		else if ( mv_percent >= 50 ) buf += " looks fairly worn out.\r\n";
		else if ( mv_percent >= 40 ) buf += " is running out of energy.\r\n";
		else if ( mv_percent >= 30 ) buf += " could really use a rest.\r\n";
		else if ( mv_percent >= 20 ) buf += " is breathing very heavily.\r\n";
		else if ( mv_percent >= 10 ) buf += " is having trouble moving.\r\n";
		else                         buf += " looks completely exhausted.\r\n";
	}

	buf[0] = UPPER(buf[0]);
	send_to_char( buf.c_str(), ch );
	return;
}

/* A much simpler version of look, this function will show you only
the condition of a mob or pc, or if used without an argument, the
same you would see if you enter the room and have config +brief.
-- Narn, winter '96
*/
void do_glance(CHAR_DATA *ch, const char* argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int save_act;

	if ( !ch->GetConnection() )
		return;

	if ( ch->position < POS_SLEEPING )
	{
		send_to_char( "You can't see anything but stars!\r\n", ch );
		return;
	}

	if ( ch->position == POS_SLEEPING )
	{
		send_to_char( "You can't see anything, you're sleeping!\r\n", ch );
		return;
	}

	if ( !check_blind( ch ) )
		return;

	argument = one_argument( argument, arg1 );

	if ( arg1[0] == '\0' )
	{
		save_act = ch->act;
		SET_BIT( ch->act, PLR_BRIEF );
		do_look( ch, "auto" );
		ch->act = save_act;
		return;
	}

	if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
		send_to_char( "They're not here.", ch );
		return;
	}
	else
	{
		if ( victim == ch )
		{
			// We don't really need this...
			//act( AT_ACTION, "You look over yourself.", ch, NULL, victim, TO_CHAR );
			act( AT_ACTION, "$n looks over $mself.", ch, NULL, victim, TO_NOTVICT );
		}
		else if ( can_see( victim, ch ) )
		{
			act( AT_ACTION, "$n glances at you.", ch, NULL, victim, TO_VICT    );
			act( AT_ACTION, "$n glances at $N.",  ch, NULL, victim, TO_NOTVICT );
		}


		show_condition( ch, victim );
		return;
	}

	return;
}


void do_examine(CHAR_DATA *ch, const char* argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	BOARD_DATA *board;
	sh_int dam;

	if ( !argument )
	{
		bug( "do_examine: null argument.", 0);
		return;
	}

	if ( !ch )
	{
		bug( "do_examine: null ch.", 0);
		return;
	}

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Examine what?\r\n", ch );
		return;
	}

	sprintf( buf, "%s noprog", arg );
	do_look( ch, buf );

	/*
	 * Support for looking at boards, checking equipment conditions,
	 * and support for trigger positions by Thoric
	 */
	if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
	{
		if ( (board = get_board( obj )) != NULL )
		{
			if ( board->num_posts )
				ch_printf( ch, "There are about %d notes posted here.  Type 'note list' to list them.\r\n", board->num_posts );
			else
				send_to_char( "There aren't any notes posted here.\r\n", ch );
		}

		switch ( obj->item_type )
		{
			default:
				break;

			case ITEM_ARMOR:
			case ITEM_WEAPON:
				if ( obj->max_condition == 0 )
					obj->max_condition = obj->condition;
				if ( obj->max_condition == 0 )
					obj->max_condition = 1;
				dam = (sh_int) ((obj->condition * 10) / obj->max_condition);
				strcpy( buf, "As you look more closely, you notice that it is ");
				if (dam >= 10) strcat( buf, "in superb condition.");
				else if (dam ==  9) strcat( buf, "in very good condition.");
				else if (dam ==  8) strcat( buf, "in good shape.");
				else if (dam ==  7) strcat( buf, "showing a bit of wear.");
				else if (dam ==  6) strcat( buf, "a little run down.");
				else if (dam ==  5) strcat( buf, "in need of repair.");
				else if (dam ==  4) strcat( buf, "in great need of repair.");
				else if (dam ==  3) strcat( buf, "in dire need of repair.");
				else if (dam ==  2) strcat( buf, "very badly worn.");
				else if (dam ==  1) strcat( buf, "practically worthless.");
				else if (dam <=  0) strcat( buf, "broken.");
				strcat( buf, "\r\n" );
				send_to_char( buf, ch );
				break;

			case ITEM_PAPER:
			{
				// Added by Ksilyan - exa note lets you look at the note

				ostringstream os;
				const char * subject, * to_list, * text;

				if ( (subject = get_extra_descr( "_subject_", obj->first_extradesc )) == NULL )
					subject = "(no subject)";

				if ( (to_list = get_extra_descr( "_to_", obj->first_extradesc )) == NULL )
					to_list = "(nobody)";

				os << "Title: " << subject << endl << "To: " << to_list << endl;

				ch->sendText( os.str() );

				if ( (text = get_extra_descr( "_text_", obj->first_extradesc )) == NULL )
					text = "The note is blank.\n\r";
				ch->sendText( text );
				break;
			}


			case ITEM_FOOD:
				if ( obj->timer > 0 && obj->value[1] > 0 )
					dam = (obj->timer * 10) / obj->value[1];
				else
					dam = 10;
				strcpy( buf, "As you examine it carefully you notice that it " );
				if (dam >= 10) strcat( buf, "is fresh.");
				else if (dam ==  9) strcat( buf, "is nearly fresh.");
				else if (dam ==  8) strcat( buf, "is perfectly fine.");
				else if (dam ==  7) strcat( buf, "looks good.");
				else if (dam ==  6) strcat( buf, "looks ok.");
				else if (dam ==  5) strcat( buf, "is a little stale.");
				else if (dam ==  4) strcat( buf, "is a bit stale.");
				else if (dam ==  3) strcat( buf, "smells slightly off.");
				else if (dam ==  2) strcat( buf, "smells quite rank.");
				else if (dam ==  1) strcat( buf, "smells revolting.");
				else if (dam <=  0) strcat( buf, "is crawling with maggots.");
				strcat( buf, "\r\n" );
				send_to_char( buf, ch );
				break;

			case ITEM_SWITCH:
			case ITEM_LEVER:
			case ITEM_PULLCHAIN:
				if ( IS_SET( obj->value[0], TRIG_UP ) )
					send_to_char( "You notice that it is in the up position.\r\n", ch );
				else
					send_to_char( "You notice that it is in the down position.\r\n", ch );
				break;
			case ITEM_BUTTON:
				if ( IS_SET( obj->value[0], TRIG_UP ) )
					send_to_char( "You notice that it is depressed.\r\n", ch );
				else
					send_to_char( "You notice that it is not depressed.\r\n", ch );
				break;

				/* Not needed due to check in do_look already
				   case ITEM_PORTAL:
				   sprintf( buf, "in %s noprog", arg );
				   do_look( ch, buf );
				   break;
				 */

			case ITEM_CORPSE_PC:
			case ITEM_CORPSE_NPC:
				{
					sh_int timerfrac = obj->timer;
					if ( obj->item_type == ITEM_CORPSE_PC )
						timerfrac = (int)obj->timer / 8 + 1;

					switch (timerfrac)
					{
						default:
							send_to_char( "This corpse has recently been slain.\r\n", ch );
							break;
						case 4:
							send_to_char( "This corpse was slain a little while ago.\r\n", ch );
							break;
						case 3:
							send_to_char( "A foul smell rises from the corpse, and it is covered in flies.\r\n", ch );
							break;
						case 2:
							send_to_char( "A writhing mass of maggots and decay, you can barely go near this corpse.\r\n", ch );
							break;
						case 1:
						case 0:
							send_to_char( "Little more than bones, there isn't much left of this corpse.\r\n", ch );
							break;
					}
				}
			case ITEM_CONTAINER:
				if ( IS_OBJ_STAT( obj, ITEM_COVERING ) )
					break;

			case ITEM_DRINK_CON:
			case ITEM_SCABBARD:
			case ITEM_QUIVER:
				send_to_char( "When you look inside, you see:\r\n", ch );
				sprintf( buf, "in %s noprog", arg );
				do_look( ch, buf );
		}
		if ( IS_OBJ_STAT( obj, ITEM_COVERING ) )
		{
			sprintf( buf, "under %s noprog", arg );
			do_look( ch, buf );
		}
		oprog_examine_trigger( ch, obj );
		if( char_died(ch) || obj_extracted(obj) )
			return;

		check_for_trap( ch, obj, TRAP_EXAMINE );
	}
	return;
}

const char *	const	day_name	[] =
{
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

const char *	const	month_name	[] =
{
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void GetDaysHoursMinutes(time_t t, int & days, int & hours, int & minutes )
{
	days = t / (60*60*24);
	t = t - days * (60*60*24);

	hours = t / (60*60);
	t = t - hours * (60*60);

	minutes = t / 60;
}

void do_time(CHAR_DATA *ch, const char* argument)
{
    extern char str_boot_time[];
    const char *suf;
    int day;

    day     = time_info.day + 1;

	 if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";


	int days, hours, minutes;

	// Get the number of days/hours/minutes since boot
	GetDaysHoursMinutes( time(0) - secBootTime, days, hours, minutes );

	ostringstream os;
	os << days << " days, " << hours << " hours, " << minutes << " minutes.";

	set_char_color( AT_YELLOW, ch );
    ch_printf( ch,
	"It is %d o'clock %s, Day of %s, %d%s the Month of %s.\r\n"
        "The mud started at (P.S.T):  %s\r"
		"  The mud has been running:  %s\r\n"
		"  The system time (P.S.T.):  %s\r",

	(time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
	time_info.hour >= 12 ? "pm" : "am",
	day_name[day % 7],
	day, suf,
	month_name[time_info.month],
	str_boot_time,
	os.str().c_str(),
	(char *) ctime( &secCurrentTime)
	);

    return;
}



void do_weather(CHAR_DATA *ch, const char* argument)
{
	static const char * const sky_look[4] =
	{
		"cloudless",
			"cloudy",
			"rainy",
			"lit by flashes of lightning"
	};

	if ( ch->isInside() )
	{
		send_to_char( "You can't see the sky from here.\r\n", ch );
		return;
	}

	if(sun_broken())
	{
		set_char_color(AT_NIGHT, ch);
		ch_printf(ch,"The night sky is cloudless and no wind blows.");
		return;
	}

	set_char_color( AT_BLUE, ch );
	ch_printf( ch, "The sky is %s and %s.\r\n",
		sky_look[weather_info.sky],
		weather_info.change >= 0
		? "a warm southerly breeze blows"
		: "a cold northern gust blows"
		);
	return;
}


/*
 * Moved into a separate function so it can be used for other things
 * ie: online help editing				-Thoric
 */
HELP_DATA *get_help(CHAR_DATA *ch, const char *argument)
{
   char argall[MAX_INPUT_LENGTH];
   char argone[MAX_INPUT_LENGTH];
   char argnew[MAX_INPUT_LENGTH];
   HELP_DATA *pHelp;
   int lev;

   if(argument[0] == '\0')
     argument = "summary";

   if(isdigit(argument[0])) {
      lev = number_argument(argument, argnew);
      argument = argnew;
   }
   else
     lev = -2;

   /*
    * Tricky argument handling so 'help a b' doesn't match a.
    */
   argall[0] = '\0';
   while(argument[0] != '\0') {
      argument = one_argument(argument, argone);
      if(argall[0] != '\0')
	strcat(argall, " ");
      strcat(argall, argone);
   }

   for(pHelp = first_help; pHelp; pHelp = pHelp->next) {
      if(ch && pHelp->level > get_trust(ch)) /* allow for null char */
	continue;
      if(lev != -2 && pHelp->level != lev)
	continue;

      if(!str_prefix(argall, pHelp->keyword))
	return pHelp;
   }

   return NULL;
}

HELP_DATA *get_help2(CHAR_DATA *ch, const char *argument)
{
	char input[MAX_INPUT_LENGTH];
	char argone[MAX_INPUT_LENGTH];
	char argnew[MAX_INPUT_LENGTH];
	char *helpentry, *sTemp, cEnd;
	HELP_DATA *pHelp, *pCandidate = NULL;
	int lev;

	if(argument[0] == '\0')
		argument = "summary";

	if(isdigit(argument[0])) {
		lev = number_argument(argument, argnew);
		argument = argnew;
	}
	else
		lev = -2;

	/*
	 * Tricky argument handling so 'help a b' doesn't match a.
	 */
	input[0] = '\0';
	while(argument[0] != '\0') {
		argument = one_argument(argument, argone);
		if(input[0] != '\0')
			strcat(input, " ");
		strcat(input, argone);
	}

	for(pHelp = first_help; pHelp; pHelp = pHelp->next) {
		if(ch &&
				(pHelp->level > get_trust(ch)
				 && (pHelp->level != 55
					 || (!IS_NPC(ch) && ch->pcdata->area==NULL)
					)
				)
		  ) /* allow for null char */
			continue;
		if(lev != -2 && pHelp->level != lev)
			continue;

		/*
		 * Prefix match checking by Warp 30.06.2000
		 */
		helpentry = pHelp->keyword;

		if(!str_prefix(input, helpentry))
			return pHelp;

		while(*helpentry != '\0') {
			sTemp = input;

			/* Find the terminating character */
			if(*helpentry == '\'')
				cEnd = *helpentry++;
			else
				cEnd = ' ';

			/* Traverse the strings while they match */
			while(*helpentry != '\0' && *sTemp != '\0' &&
					LOWER(*sTemp) == LOWER(*helpentry)) {
				sTemp++;
				helpentry++;
			}

			/* We have perfect match, return it right away */
			if((*helpentry == '\0' || *helpentry == cEnd) && *sTemp == '\0')
				return pHelp;

			/* If the input string has ended, we have a prefix match */
			if(*sTemp == '\0')
				pCandidate = pHelp;

			/* If not, move to the next keyword */
			while(*helpentry != '\0' && *helpentry != cEnd)
				helpentry++;

			if(*helpentry == cEnd)
				helpentry++;

			while(isspace(*helpentry))
				helpentry++;
		}
	}

	return pCandidate;
}

/*
 * Now this is cleaner
 */
void do_help(CHAR_DATA *ch, const char* argument)
{
	HELP_DATA *pHelp;

	if((pHelp = get_help2(ch, argument)) == NULL) {
		send_to_char("No help on that word.\r\n", ch);
		return;
	}

	if(pHelp->level > 0 && str_cmp(argument, "imotd")) {
		if(IS_IMMORTAL(ch)) {
			pager_printf(ch, "[%2d] %s\r\n", pHelp->level, pHelp->keyword);
		} else {
			send_to_pager(pHelp->keyword, ch);
		}
		send_to_pager("\r\n", ch);
	}

	/*
	 * Strip leading '.' to allow initial blanks.
	 */
	if(pHelp->text[0] == '.')
		send_to_pager_color(pHelp->text+1, ch);
	else
		send_to_pager_color(pHelp->text, ch);
	return;
}

/*
 * Help editor							-Thoric
 */
void do_hedit(CHAR_DATA *ch, const char* argument)
{
    HELP_DATA *pHelp;

    if ( !ch->GetConnection() )
    {
	send_to_char( "You have no descriptor.\r\n", ch );
	return;
    }

    switch( ch->substate )
    {
	default:
	  break;
	case SUB_HELP_EDIT:
	  if ( (pHelp = (HELP_DATA * ) ch->dest_buf) == NULL )
	  {
		bug( "hedit: sub_help_edit: NULL ch->dest_buf", 0 );
		stop_editing( ch );
		return;
	  }
	  STRFREE( pHelp->text );
	  pHelp->text = copy_buffer( ch );
	  stop_editing( ch );

      if ( !str_prefix("greeting", pHelp->keyword) ) {
          recount_greetings();
      }
	  return;
    }
    if ( (pHelp = get_help2( ch, argument )) == NULL )	/* new help */
    {
	char argnew[MAX_INPUT_LENGTH];
	int lev;

	if ( isdigit(argument[0]) )
	{
	    lev = number_argument( argument, argnew );
	    argument = argnew;
	}
	else
	    lev = get_trust(ch);
	CREATE( pHelp, HELP_DATA, 1 );
	pHelp->keyword = STRALLOC( strupper(argument) );
	pHelp->text    = STRALLOC( "" );
	pHelp->level   = lev;
	add_help( pHelp );
    }
    ch->substate = SUB_HELP_EDIT;
    ch->dest_buf = pHelp;
    start_editing( ch, pHelp->text );
}

/*
 * Stupid leading space muncher fix				-Thoric
 */
std::string help_fix( const char *text )
{
    std::string fixed;

    if ( !text )
      return "";
    fixed = strip_cr(text);
    if ( fixed[0] == ' ' )
      fixed[0] = '.';
    return fixed;
}

void do_hset(CHAR_DATA *ch, const char* argument)
{
    HELP_DATA *pHelp;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = smash_tilde_static( argument );
    argument = one_argument( argument, arg1 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: hset <field> [value] [help page]\r\n",	ch );
	send_to_char( "\r\n",						ch );
	send_to_char( "Field being one of:\r\n",			ch );
	send_to_char( "  level keyword remove save\r\n",		ch );
	return;
    }

    if ( !str_cmp( arg1, "save" ) )
    {
	FILE *fpout;

	log_string_plus( "Saving help.are...", LOG_NORMAL, sysdata.log_level );

	rename( "help.are", "help.are.bak" );
	fclose( fpReserve );
	if ( ( fpout = fopen( "help.are", "w" ) ) == NULL )
	{
	   bug( "hset save: fopen", 0 );
	   perror( "help.are" );
	   fpReserve = fopen( NULL_FILE, "r" );
	   return;
	}

	fprintf( fpout, "#HELPS\n\n" );
	for ( pHelp = first_help; pHelp; pHelp = pHelp->next )
	    fprintf( fpout, "%d %s~\n%s~\n\n",
			pHelp->level, pHelp->keyword, help_fix(pHelp->text).c_str() );

	fprintf( fpout, "0 $~\n\n\n#$\n" );
	fclose( fpout );
	fpReserve = fopen( NULL_FILE, "r" );
	send_to_char( "Saved.\r\n", ch );
	return;
    }
    if ( str_cmp( arg1, "remove" ) )
	argument = one_argument( argument, arg2 );

    if ( (pHelp = get_help2( ch, argument )) == NULL )
    {
	send_to_char( "Cannot find help on that subject.\r\n", ch );
	return;
    }
    if ( !str_cmp( arg1, "remove" ) )
    {
	UNLINK( pHelp, first_help, last_help, next, prev );
	STRFREE( pHelp->text );
	STRFREE( pHelp->keyword );
	DISPOSE( pHelp );

    if ( !str_prefix("greeting", argument) )
        recount_greetings();

    send_to_char( "Removed.\r\n", ch );
	return;
    }
    if ( !str_cmp( arg1, "level" ) )
    {
	pHelp->level = atoi( arg2 );
	send_to_char( "Done.\r\n", ch );
	return;
    }
    if ( !str_cmp( arg1, "keyword" ) )
    {
        if ( !str_prefix("greeting", pHelp->keyword ) ) {
                recount_greetings();
        }
        STRFREE( pHelp->keyword );
        pHelp->keyword = STRALLOC( strupper(arg2) );
        send_to_char( "Done.\r\n", ch );

        if ( !str_prefix("greeting", pHelp->keyword) )
        {
            recount_greetings();
        }
	return;
    }

    do_hset( ch, "" );
}

/*
 * Show help topics in a level range				-Thoric
 * Idea suggested by Gorog
 */
void do_hlist(CHAR_DATA *ch, const char* argument)
{
    int min, max, minlimit, maxlimit, cnt;
    char arg[MAX_INPUT_LENGTH];
    HELP_DATA *help;

    maxlimit = get_trust(ch);
    minlimit = maxlimit >= LEVEL_STONE_INITIATE ? -1 : 0;
    argument = one_argument( argument, arg );

    if ( arg[0] != '\0' && is_number(arg) )
    {
        min = URANGE( minlimit, atoi(arg), maxlimit );

        argument = one_argument(argument, arg);

        if ( arg[0] != '\0' && is_number(arg) )
        {
            max = URANGE( min, atoi(arg), maxlimit );
            argument = one_argument(argument, arg);
        }
        else
            max = maxlimit;
    }
    else
    {
        min = minlimit;
        max = maxlimit;
    }

    set_pager_color( AT_GREEN, ch );

    if ( arg[0] != '\0' ) {
        pager_printf( ch, "Help Topics in level range %d to %d containing %s:\r\n\r\n", min, max, arg );
    } else {
        pager_printf( ch, "Help Topics in level range %d to %d:\r\n\r\n", min, max );
    }

    for ( cnt = 0, help = first_help; help; help = help->next )
    {
        if ( help->level >= min && help->level <= max )
        {
            if ( arg[0] != '\0' && !is_name_prefix(arg, help->keyword) )
                continue;

            pager_printf( ch, "  %3d %s\r\n", help->level, help->keyword );
            ++cnt;
        }
    }

    if ( cnt )
        pager_printf( ch, "\r\n%d pages found.\r\n", cnt );
    else
        send_to_char( "None found.\r\n", ch );
}


/*
 * New do_who with WHO REQUEST, clan, race and homepage support.  -Thoric
 *
 * Latest version of do_who eliminates redundant code by using linked lists.
 * Shows imms separately, indicates guest and retired immortals.
 * Narn, Oct/96
 */
void do_wwho(CHAR_DATA *ch, const char* argument)
{
    char buf[MAX_STRING_LENGTH];
    char clan_name[MAX_INPUT_LENGTH];
    char council_name[MAX_INPUT_LENGTH];
    char invis_str[MAX_INPUT_LENGTH];
    char char_name[MAX_INPUT_LENGTH];
    const char *extra_title;
    char class_text[MAX_INPUT_LENGTH];

    int iClass, iRace;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    bool rgfClass[MAX_CLASS];
    bool rgfRace[MAX_RACE];
    bool fClassRestrict;
    bool fRaceRestrict;
    bool fImmortalOnly;

    bool fShowHomepage;
    bool fClanMatch; /* SB who clan (order),who guild, and who council */
    bool fCouncilMatch;
    bool fDeityMatch;
    ClanData * pClan;
    CouncilData * pCouncil;
    DeityData * pDeity;
    FILE *whoout;

    /*
    #define WT_IMM    0;
    #define WT_MORTAL 1;
    #define WT_DEADLY 2;
    */

    WHO_DATA *cur_who = NULL;
    WHO_DATA *next_who = NULL;
    WHO_DATA *first_mortal = NULL;
    WHO_DATA *first_avatar  = NULL;
    WHO_DATA *first_imm = NULL;

    /*
     * Set default arguments.
     */
    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL;
    fClassRestrict = FALSE;
    fRaceRestrict  = FALSE;
    fImmortalOnly  = FALSE;
    fShowHomepage  = FALSE;
    fClanMatch	   = FALSE; /* SB who clan (order), who guild, who council */
    fCouncilMatch  = FALSE;
    fDeityMatch    = FALSE;
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	rgfClass[iClass] = FALSE;
    for ( iRace = 0; iRace < MAX_RACE; iRace++ )
	rgfRace[iRace] = FALSE;

    /*
     * Parse arguments.
     */
    nNumber = 0;
    for ( ;; )
    {
	char arg[MAX_STRING_LENGTH];

	argument = one_argument( argument, arg );
	if ( arg[0] == '\0' )
	    break;

	if ( is_number( arg ) )
	{
	    switch ( ++nNumber )
	    {
	    case 1: iLevelLower = atoi( arg ); break;
	    case 2: iLevelUpper = atoi( arg ); break;
	    default:
		send_to_char( "Only two level numbers allowed.\r\n", ch );
		return;
	    }
	}
	else
	{
	    if ( strlen(arg) < 3 )
	    {
		send_to_char( "Classes must be longer than that.\r\n", ch );
		return;
	    }

	    /*
	     * Look for classes to turn on.
	     */
	    if ( !str_cmp( arg, "imm" ) || !str_cmp( arg, "gods" ) )
		fImmortalOnly = TRUE;
	    else
	    if ( !str_cmp( arg, "www" ) )
		fShowHomepage = TRUE;
            else		 /* SB who clan (order), guild, council */
             if  ( ( pClan = get_clan (arg) ) )
	   	fClanMatch = TRUE;
            else
             if ( ( pCouncil = get_council (arg) ) )
                fCouncilMatch = TRUE;
	    else
	     if ( ( pDeity = get_deity (arg) ) )
		fDeityMatch = TRUE;
	    else
	    {
		for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
		{
		    if ( !str_cmp( arg, class_table[iClass]->whoName_.c_str() ) )
		    {
			rgfClass[iClass] = TRUE;
			break;
		    }
		}
		if ( iClass != MAX_CLASS )
		  fClassRestrict = TRUE;

		for ( iRace = 0; iRace < MAX_RACE; iRace++ )
		{
		    if ( !str_cmp( arg, race_table[iRace].race_name ) )
		    {
			rgfRace[iRace] = TRUE;
			break;
		    }
		}
		if ( iRace != MAX_RACE )
		  fRaceRestrict = TRUE;

		if ( iClass == MAX_CLASS && iRace == MAX_RACE
 		 && fClanMatch == FALSE
                 && fCouncilMatch == FALSE
		 && fDeityMatch == FALSE )
		{
		 send_to_char( "That's not a class, race, order, guild,"
			" council or deity.\r\n", ch );
		    return;
		}
	    }
	}
    }

    /*
     * Now find matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    if ( ch )
	set_pager_color( AT_GREEN, ch );
    else
    {
	if ( fShowHomepage )
	  whoout = fopen( WEBWHO_FILE, "w" );
	else
	  whoout = fopen( WHO_FILE, "w" );
    }


/* start from last to first to get it in the proper order */
	ritorSocketId itor;
   for ( itor = gTheWorld->GetBeginReverseConnection(); itor != gTheWorld->GetEndReverseConnection(); itor++ )
   {
	   // we know that the world's player connection list only holds player connections IDs,
	   // so we can safely cast it to PlayerConnection*
	   PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];
	   CHAR_DATA *wch;
	   const char *Class;

	   if ( (d->ConnectedState != CON_PLAYING && d->ConnectedState != CON_EDITING)
			   ||   !can_see( ch, d->GetOriginalCharacter() ) )
		   continue;
	   wch   = d->GetOriginalCharacter();
	   if ( wch->level < iLevelLower
			   ||   wch->level > iLevelUpper
			   || ( fImmortalOnly  && wch->level < LEVEL_IMMORTAL )
			   || ( fClassRestrict && !rgfClass[wch->Class] )
			   || ( fRaceRestrict && !rgfRace[wch->race] )
			   || ( fClanMatch && ( pClan != wch->pcdata->clan ))  /* SB */
			   || ( fCouncilMatch && ( pCouncil != wch->pcdata->council )) /* SB */
			   || ( fDeityMatch && ( pDeity != wch->pcdata->deity )) )
		   continue;

	   nMatch++;

	   if ( fShowHomepage
			   &&   wch->pcdata->homepage
			   &&   wch->pcdata->homepage[0] != '\0' )
		   sprintf( char_name, "<A HREF=\"%s\">%s</A>",
				   show_tilde( wch->pcdata->homepage ), wch->getName().c_str() );
	   else
		   strcpy( char_name, wch->getName().c_str() );

#if 1
	   class_text[0] = '\0';
#else
	   sprintf( class_text, "%s%2d %s", NOT_AUTHED(wch) ? "N" : " ", wch->level, class_table[wch->Class]->who_name );
#endif
	   Class = class_text;
	   switch ( wch->level )
	   {
		   default: break;
		   case MAX_LEVEL -  0: Class = "Lord of the Darkstone";	break;
		   case MAX_LEVEL -  1: Class = "Stone Master";	break;
		   case MAX_LEVEL -  2: Class = "Stone Adept";		break;
		   case MAX_LEVEL -  3: Class = "Stone Initiate";		break;
		   case MAX_LEVEL -  4: Class = "Stone Seeker";	break;
		   case MAX_LEVEL -  5: Class = "Artificer";	break;
		   case MAX_LEVEL -  6: Class = "Tinkerer";	break;
		   case MAX_LEVEL -  7: Class = "Creator";		break;
		   case MAX_LEVEL -  8: Class = "Engineer";	break;
		   case MAX_LEVEL -  9: Class = "Wanderer";	break;
		   case MAX_LEVEL - 10: Class = "Hero";	break;
		   case MAX_LEVEL - 11: Class = "Hero";		break;
		   case MAX_LEVEL - 12: Class = "Hero";		break;
		   case MAX_LEVEL - 13: Class = "Hero";		break;
		   case MAX_LEVEL - 14: Class = "Hero";	break;
								/*	   case MAX_LEVEL - 15: Class = "Hero";		break;*/
	   }

	   if ( !str_cmp( wch->getName().c_str(), sysdata.guild_overseer ) )
		   extra_title = " [Overseer of Guilds]";
	   else if ( !str_cmp( wch->getName().c_str(), sysdata.guild_advisor ) )
		   extra_title = " [Advisor to Guilds]";
	   else
		   extra_title = "";

	   if ( IS_RETIRED( wch ) )
		   Class = "Mythic";
	   else if ( IS_GUEST( wch ) )
		   Class = "Emissary";
	   else if ( wch->pcdata->rank_.length() > 0 )
		   Class = wch->pcdata->rank_.c_str();

	   if ( wch->pcdata->clan )
	   {
		   ClanData *pclan = wch->pcdata->clan;
		   if ( pclan->clan_type == CLAN_GUILD )
			   strcpy( clan_name, " <" );
		   else
			   strcpy( clan_name, " (" );

		   if ( pclan->clan_type == CLAN_ORDER )
		   {
			   if ( wch->getName() == pclan->deity_ )
				   strcat( clan_name, "Deity, Order of " );
			   else if ( wch->getName() == pclan->leader_ )
				   strcat( clan_name, "Leader, Order of " );
			   else if ( wch->getName() == pclan->number1_ )
				   strcat( clan_name, "Number One, Order of " );
			   else if ( wch->getName() == pclan->number2_ )
				   strcat( clan_name, "Number Two, Order of " );
			   else
				   strcat( clan_name, "Order of " );
		   }
		   else
			   if ( pclan->clan_type == CLAN_GUILD )
			   {
				   if ( wch->getName() == pclan->leader_ )
					   strcat( clan_name, "Leader, " );
				   if ( wch->getName() == pclan->number1_ )
					   strcat( clan_name, "First, " );
				   if ( wch->getName() == pclan->number2_ )
					   strcat( clan_name, "Second, " );
			   }
			   else
			   {
				   if ( wch->getName() == pclan->deity_ )
					   strcat( clan_name, "Deity of " );
				   else if ( wch->getName() == pclan->leader_ )
					   strcat( clan_name, "Leader of " );
				   else if ( wch->getName() == pclan->number1_ )
					   strcat( clan_name, "Number One " );
				   else if ( wch->getName() == pclan->number2_ )
					   strcat( clan_name, "Number Two " );
			   }
		   strcat( clan_name, pclan->name_.c_str() );
		   if ( pclan->clan_type == CLAN_GUILD )
			   strcat( clan_name, ">" );
		   else
			   strcat( clan_name, ")" );
	   }
	   else
		   clan_name[0] = '\0';

	   if ( wch->pcdata->council )
	   {
		   strcpy( council_name, " [" );
		   if ( wch->getName().ciEqual(wch->pcdata->council->head_) )
			   strcat( council_name, "Head of " );
		   strcat( council_name, wch->pcdata->councilName_.c_str() );
		   strcat( council_name, "]" );
	   }
	   else
		   council_name[0] = '\0';

	   if ( IS_SET(wch->act, PLR_WIZINVIS) )
		   sprintf( invis_str, "(%d) ", wch->pcdata->wizinvis );
	   else
		   invis_str[0] = '\0';

	   if ( Class[0] == '\0' )
		   sprintf( buf, "%s%s%s%s%s%s%s%s\r\n",
				   invis_str,
				   IS_SET(wch->act, PLR_AFK) ? "[AFK] " : "",
				   (wch->GetConnection() && wch->GetConnection()->ConnectedState == CON_EDITING) ?
				   "(writing) " : "",
				   char_name,
				   player_title(wch->pcdata->title_.c_str()),
				   extra_title,
				   clan_name,
				   council_name );
	   else
	   {
		   const char *ptr = Class;
		   sh_int len = 0;
		   sh_int count = 0;
		   char cbuf[MAX_STRING_LENGTH];
		   sh_int width = 16;

		   while ( *ptr ) {
			   if ( *ptr != '&' && *ptr != '^' ) {
				   len++;
			   } else if ( *ptr == '&' && *ptr+1 == '&' ) {
				   len++;
				   ptr++;
			   } else if ( *ptr == '^' && *ptr+1 == '^' ) {
				   len++;
				   ptr++;
			   } else {
				   ptr++;
			   }
			   ptr++;
		   }

		   width -= len;
		   strcpy(cbuf, Class);
		   for ( count = 0; count < width; count++ )
			   strcat(cbuf, " ");

		   for ( count = (width*2)+len; count < 16; count++ )
			   strcat(cbuf, " ");

		   sprintf( buf, "%s %s%s%s%s%s%s%s%s\r\n",
				   cbuf,
				   invis_str,
				   IS_SET(wch->act, PLR_AFK) ? "[AFK] " : "",
				   (wch->GetConnection() && wch->GetConnection()->ConnectedState == CON_EDITING) ?
				   "(writing) " : "",
				   char_name,
				   player_title(wch->pcdata->title_.c_str()),
				   extra_title,
				   clan_name,
				   council_name );
	   }
	   /*
		* This is where the old code would display the found player to the ch.
		* What we do instead is put the found data into a linked list
		*/

	   /* First make the structure. */
	   CREATE( cur_who, WHO_DATA, 1 );
	   cur_who->text = str_dup( buf );
	   if ( IS_AVATAR(wch) )
		   cur_who->type = WT_AVATAR;
	   else if ( IS_IMMORTAL( wch ) )
		   cur_who->type = WT_IMM;
	   else
		   cur_who->type = WT_MORTAL;

	   /* Then put it into the appropriate list. */
	   switch ( cur_who->type )
	   {
		   case WT_MORTAL:
			   cur_who->next = first_mortal;
			   first_mortal = cur_who;
			   break;
		   case WT_AVATAR:
			   cur_who->next = first_avatar;
			   first_avatar = cur_who;
			   break;
		   case WT_IMM:
			   cur_who->next = first_imm;
			   first_imm = cur_who;
			   break;
	   }

   }


   /* Ok, now we have three separate linked lists and what remains is to
    * display the information and clean up.
    */

    if ( !ch )
    {
	  fprintf( whoout, "\n\r\n===============================[  MORTALS  ]===============================\r\n\r\n" );
    }
    else
    {
        set_char_color(AT_LIST, ch);
    	send_to_pager( "\r\n==============================[  MORTALS  ]==============================\r\n\r\n", ch );
    }

   for ( cur_who = first_mortal; cur_who; cur_who = next_who )
    {
        if ( !ch )
           fprintf( whoout, "%s", cur_who->text );
        else
        {
            set_char_color(AT_LIST, ch);
            send_to_pager( cur_who->text, ch );
        }
       next_who = cur_who->next;
       DISPOSE( cur_who->text );
       DISPOSE( cur_who );
    }

    if ( first_avatar ) {
        if ( !ch )
    	    fprintf( whoout, "\r\n------------------------------[  AVATARS  ]-------------------------------\r\n\r\n" );
    	else
        {
            set_char_color(AT_LIST, ch);
    	    send_to_pager( "\r\n------------------------------[  AVATARS  ]------------------------------\r\n\r\n", ch );
        }

        for ( cur_who = first_avatar; cur_who; cur_who = next_who ) {
            if ( !ch ) {
                fprintf(whoout, "%s", cur_who->text);
            }
            else
            {
                set_char_color(AT_LIST, ch);
                send_to_pager(cur_who->text, ch);
                next_who = cur_who->next;
                DISPOSE(cur_who->text);
                DISPOSE(cur_who);
            }
        }
    }

   if ( first_imm )
     {
	if ( !ch )
	  fprintf( whoout, "\r\n------------------------------[ IMMORTALS ]-------------------------------\r\n\r\n" );
	else
    {
        set_char_color(AT_LIST, ch);
	  send_to_pager( "\r\n------------------------------[ IMMORTALS ]------------------------------\r\n\r\n", ch );
     }
     }

   for ( cur_who = first_imm; cur_who; cur_who = next_who )
     {
	if ( !ch )
	  fprintf( whoout, "%s", cur_who->text );
      else
      {
          set_char_color(AT_LIST, ch);
    	  send_to_pager( cur_who->text, ch );
      }
	next_who = cur_who->next;
	DISPOSE( cur_who->text );
	DISPOSE( cur_who );
    }

    if  ( !ch )
    {
	fprintf( whoout, "\nTotal Player%s: %d\r\n", nMatch == 1 ? "" : "s", nMatch );
	fclose( whoout );
	return;
    }

   set_char_color( AT_YELLOW, ch );
   ch_printf( ch, "\nTotal Player%s: %d\r\n", nMatch == 1 ? "" : "s", nMatch );
   return;
}


void do_compare(CHAR_DATA *ch, const char* argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj1;
	OBJ_DATA *obj2;
	int value1;
	int value2;
	const char *msg;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	if ( arg1[0] == '\0' )
	{
		send_to_char( "Compare what to what?\r\n", ch );
		return;
	}

	if ( ( obj1 = get_obj_carry( ch, arg1 ) ) == NULL )
	{
		send_to_char( "You do not have that item.\r\n", ch );
		return;
	}

	if ( arg2[0] == '\0' )
	{
		for ( obj2 = ch->first_carrying; obj2; obj2 = obj2->next_content )
		{
			if ( obj2->wear_loc != WEAR_NONE
					&&   can_see_obj( ch, obj2 )
					&&   obj1->item_type == obj2->item_type
					&& ( obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
				break;
		}

		if ( !obj2 )
		{
			send_to_char( "You aren't wearing anything comparable.\r\n", ch );
			return;
		}
	}
	else
	{
		if ( ( obj2 = get_obj_carry( ch, arg2 ) ) == NULL )
		{
			send_to_char( "You do not have that item.\r\n", ch );
			return;
		}
	}

	msg		= NULL;
	value1	= 0;
	value2	= 0;

	if ( obj1 == obj2 )
	{
		msg = "You compare $p to itself.  It looks about the same.";
	}
	else if ( obj1->item_type != obj2->item_type )
	{
		msg = "You can't compare $p and $P.";
	}
	else
	{
		switch ( obj1->item_type )
		{
			default:
				msg = "You can't compare $p and $P.";
				break;

			case ITEM_ARMOR:
				value1 = obj1->value[OBJECT_ARMOR_AC];
				value2 = obj2->value[OBJECT_ARMOR_AC];
				break;

			case ITEM_WEAPON:

				// Compare works differently for melee and ranged weapons...
				//   -ksilyan 2005-aug-22

				if ( IS_RANGED_WEAPON_OBJ(obj1) && IS_RANGED_WEAPON_OBJ(obj2) )
				{
					// Compare weapon power
					value1 = obj1->value[OBJECT_WEAPON_POWER];
					value2 = obj2->value[OBJECT_WEAPON_POWER];
				}
				else if ( IS_RANGED_WEAPON_OBJ(obj1) != IS_RANGED_WEAPON_OBJ(obj2) )
				{
					msg = "You can't compare ranged and hand-to-hand weapons.";
				}
				else
				{
					value1 = obj1->value[OBJECT_WEAPON_MINDAMAGE] + obj1->value[OBJECT_WEAPON_MAXDAMAGE];
					value2 = obj2->value[OBJECT_WEAPON_MINDAMAGE] + obj2->value[OBJECT_WEAPON_MAXDAMAGE];
				}

				break;

			case ITEM_PROJECTILE:
				// Allow comparison of projectiles
				//   -ksilyan 2005-aug-22

				value1 = obj1->value[OBJECT_PROJECTILE_MINDAMAGE] + obj1->value[OBJECT_PROJECTILE_MAXDAMAGE];
				value2 = obj2->value[OBJECT_PROJECTILE_MINDAMAGE] + obj2->value[OBJECT_PROJECTILE_MAXDAMAGE];

				break;
		}
	}

	if ( !msg )
	{
		if ( value1 == value2 ) msg = "$p and $P look about the same.";
		else if ( value1  > value2 ) msg = "$p looks better than $P.";
		else                         msg = "$p looks worse than $P.";
	}

	act( AT_PLAIN, msg, ch, obj1, obj2, TO_CHAR );
	return;
}



void do_where(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	bool found;

	one_argument( argument, arg );

	set_pager_color( AT_PERSON, ch );
	if ( arg[0] == '\0' )
	{
		pager_printf( ch, "Players near you in %s:\r\n", ch->GetInRoom()->area->name );
		found = FALSE;
		itorSocketId itor;
		for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
		{
			// we know that the world's player connection list only holds player connections IDs,
			// so we can safely cast it to PlayerConnection*
			PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

			if ( (d->ConnectedState == CON_PLAYING || d->ConnectedState == CON_EDITING )
				&& ( victim = d->GetCharacter() ) != NULL
				&&	 !IS_NPC(victim)
				&&	 victim->GetInRoom()
				&&	 victim->GetInRoom()->area == ch->GetInRoom()->area
				&&	 can_see( ch, victim ) )
			{
				found = TRUE;
				pager_printf( ch, "%-28s %s\r\n",
					victim->getShort().c_str(), in_arena(victim) ? "" : victim->GetInRoom()->name_.c_str() );
			}
		}
		if ( !found )
			send_to_char( "None\r\n", ch );
	}
	else
	{
		found = FALSE;
		for ( victim = first_char; victim; victim = victim->next )
			if ( victim->GetInRoom()
				&&	 victim->GetInRoom()->area == ch->GetInRoom()->area
				&&	 !IS_AFFECTED(victim, AFF_HIDE)
				&&	 !IS_AFFECTED(victim, AFF_SNEAK)
				&&	 can_see( ch, victim )
				&&	 is_name( arg, victim->getName().c_str() ) )
			{
				found = TRUE;
				pager_printf( ch, "%-28s %s\r\n",
					PERS(victim, ch), victim->GetInRoom()->name_.c_str() );
				break;
			}
			if ( !found )
				act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
	}

	return;
}




void do_consider(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    const char *msg;
    int diff;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Consider killing whom?\r\n", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They're not here.\r\n", ch );
	return;
    }

    if ( ch != victim ) {
        diff = victim->level - ch->level;


	 if ( diff <= -10 ) msg = "You are far more experienced than $N.";
    else if ( diff <=  -5 ) msg = "$N is not nearly as experienced as you.";
    else if ( diff <=  -2 ) msg = "You are more experienced than $N.";
    else if ( diff <=   1 ) msg = "You are just about as experienced as $N.";
    else if ( diff <=   4 ) msg = "You are not nearly as experienced as $N.";
    else if ( diff <=   9 ) msg = "$N is far more experienced than you!";
    else                    msg = "$N would make a great teacher for you!";
    	act( AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR );
    }

    if ( ch == victim )
        diff = number_range(0, 400) - 200;
    else
        diff = (int) (victim->max_hit - ch->max_hit) / 6;

	 if ( diff <= -200) msg = "$N looks like a feather!";
    else if ( diff <= -150) msg = "You could kill $N with your hands tied!";
    else if ( diff <= -100) msg = "Hey! Where'd $N go?";
    else if ( diff <=  -50) msg = "$N is a wimp.";
    else if ( diff <=    0) msg = "$N looks weaker than you.";
    else if ( diff <=   50) msg = "$N looks about as strong as you.";
    else if ( diff <=  100) msg = "It would take a bit of luck...";
    else if ( diff <=  150) msg = "It would take a lot of luck, and equipment!";
    else if ( diff <=  200) msg = "Why don't you dig a grave for yourself first?";
    else                    msg = "$N is built like a TANK!";
    act( AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR );

    if(!IS_NPC(victim) && !IS_NPC(ch))
    {
       msg="Attacking would not bring dishonor.";
       if(ch->level - victim->level > 5)
         msg="Attacking would be unfair and beneath your dignity.";
       else if (ch->level - victim->level < -5)
         msg="Attacking would be foolish but you would enjoy a hero's funeral!";
       act( AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR );

    }
    return;
}


/*
 * Ksilyan: spells command
 * Lists all spells the player knows, with their mana cost,
 * and at some point components (if any)
 */

void do_spells(Character * ch, const char* argument)
{
	int sn;

	if ( IS_NPC(ch) )
		return;

	int 	col;
	sh_int	lasttype, cnt;

	col = cnt = 0;
	lasttype = SKILL_SPELL;
	set_pager_color( AT_LIST, ch );

	ostringstream result;

	for ( sn = 0; sn < top_sn; sn++ )
	{
		if ( skill_table[sn]->name_ == "" )
			break;

		if ( skill_table[sn]->type != SKILL_SPELL )
			continue; // only show spells

		if (!IS_IMMORTAL(ch)
			&& ( skill_table[sn]->guild != CLASS_NONE
			&& ( !IS_GUILDED(ch)
			|| (ch->pcdata->clan->Class != skill_table[sn]->guild) ) ) )
		{
			continue;
		}

		if ( ch->level < skill_table[sn]->skill_level[ch->Class]
			|| (!IS_IMMORTAL(ch) && skill_table[sn]->skill_level[ch->Class] == 0) )
		{
			continue;
		}

		if ( ch->pcdata->learned[sn] == 0 )
			continue; // only show spells we know

		++cnt;

		result << setw(18) << left << skill_table[sn]->name_.str();

		int manaCost = UMAX(skill_table[sn]->min_mana,
				100 / ( 2 + ch->level - skill_table[sn]->skill_level[ch->Class] ) );

		// vampire mana cost handling
		if ( IS_VAMPIRE(ch) )
			manaCost = UMAX( 1, (manaCost+4)/8 );

		result << " (" << skill_prof(ch->pcdata->learned[sn]) << "), ";
		result << (IS_VAMPIRE(ch) ? "blood: " : "mana: ");
		result << setw(3) << right << manaCost;
		result << "  ";

		if ( ++col % 2 == 0 )
			result << endl;
	}

	if ( col % 2 != 0 )
		result << endl;

	ch->sendText( result.str() );

	return;
}

/*
 * Place any skill types you don't want them to be able to practice
 * normally in this list.  Separate each with a space.
 * (Uses an is_name check). -- Altrag
 */
#define CANT_PRAC "Tongue"

void do_practice(CHAR_DATA *ch, const char* argument)
{
	CHAR_DATA *mob = NULL;
	char buf[MAX_STRING_LENGTH];
	TRAIN_DATA * train;
	TRAIN_LIST_DATA * list;
	int sn;
	bool FirstSkill;
	bool sMatch;
	int train_max = 0;
	int train_charge = 0;

	if ( IS_NPC(ch) )
		return;

	for ( mob = ch->GetInRoom()->first_person; mob; mob = mob->next_in_room )
	{
		if ( IS_NPC(mob) && mob->pIndexData->train )
		{
			break;
		}
	}


	if ( mob )
	{
		if(mob->pIndexData->train)
		{
			train = mob->pIndexData->train;
			if ((train->_class != ch->Class) && (train->_class != -1))
			{
				act(AT_TELL, "$n tells you 'I'll have nothing to do with your kind!", mob, NULL, ch, TO_VICT);
				return;
			}
			else if ((train->max_level < ch->level) && (train->max_level != -1))
			{
				act(AT_TELL, "$n tells you 'You have already learned more than I.'", mob, NULL, ch, TO_VICT);
				return;
			}
			else if ((train->min_level > ch->level))
			{
				act(AT_TELL, "$n tells you 'You are too young for me to bother with.'", mob, NULL, ch, TO_VICT);
				return;
			}
			else if ((train->base_cost > ch->gold))
			{
				act(AT_TELL, "$n tells you 'I will not train beggars!'", mob, NULL, ch, TO_VICT);
				return;
			}
			else if ((
				((IS_EVIL(ch)) && (train->alignment != 0))
				||
				((IS_GOOD(ch)) && (train->alignment != 2))
				||
				((IS_NEUTRAL(ch)) && (train->alignment != 1)))
				&&
				(train->alignment != -1)
				)
			{
				act( AT_TELL, "$n tells you 'I will not train someone opposed to my alignment.'",
					mob, NULL, ch, TO_VICT );
				return;
			}
		}
	}

	if ( argument[0] == '\0' )
	{
		int 	col;
		sh_int	lasttype, cnt;

		col = cnt = 0;
		lasttype = SKILL_SPELL;
		set_pager_color( AT_LIST, ch );
		FirstSkill = TRUE;
		for ( sn = 0; sn < top_sn; sn++ )
		{
			if ( skill_table[sn]->name_ == "" )
			{
				break;
			}

			if ( skill_table[sn]->type != lasttype )
			{
			/*
			* Mark the header bool so it will print
			* if the player can practice instead
			* of an empty header
				*/
				FirstSkill = TRUE;
			}

			lasttype = skill_table[sn]->type;

			if (!IS_IMMORTAL(ch)
				&& ( skill_table[sn]->guild != CLASS_NONE
				&& ( !IS_GUILDED(ch)
				|| (ch->pcdata->clan->Class != skill_table[sn]->guild) ) ) )
			{
				continue;
			}

			if ( ch->level < skill_table[sn]->skill_level[ch->Class]
				|| (!IS_IMMORTAL(ch) && skill_table[sn]->skill_level[ch->Class] == 0) )
			{
				continue;
			}

			if ( ch->pcdata->learned[sn] == 0
				&&	 SPELL_FLAG(skill_table[sn], SF_SECRETSKILL)
				&& !mob)
			{
				continue;
			}
			if ( mob )
			{
				if(mob->pIndexData->train)
				{
					train = mob->pIndexData->train;

					if ( train->first_in_list )
					{
						sMatch = FALSE;

						for ( list = train->first_in_list; list; list = list->next )
						{
							if (sn == list->sn)
							{
								sMatch = TRUE;
								train_charge = list->cost;
							}
						}

						if (!sMatch)
						{
							continue;
						}
						else if  ((train->max_level < skill_table[sn]->skill_level[ch->Class]) && train->max_level > 0)
						{
							continue;
						}
					}
					else
					{
						if ( is_name( skill_tname[skill_table[sn]->type], CANT_PRAC ) )
						{
							continue;
						}

						if ( ch->pcdata->learned[sn] == 0
							&&	 SPELL_FLAG(skill_table[sn], SF_SECRETSKILL) )
						{
							continue;
						}
						else if ( SPELL_FLAG(skill_table[sn], SF_SPECIALTRAINERS) ) {
							continue;
						}
						else if ((ch->level < skill_table[sn]->skill_level[ch->Class]))
						{
							continue;
						}
						else if ( ch->level < skill_table[sn]->skill_level[ch->Class] )
						{
							continue;
						}
					}
				} else {	/* Must have trainerdata! */
					continue;
				}
			}

			++cnt;

			if (FirstSkill == TRUE)
			{
				FirstSkill = FALSE;

				send_to_pager( "\r\n", ch );

				pager_printf( ch,"--------------------------------%ss---------------------------------\r\n",
					skill_tname[skill_table[sn]->type]);
				col = cnt = 0;
			}

			/* Golias 11/6 lets change percentage listing to (POOR) (FAIR) (GOOD) (EXCELLENT)
			pager_printf( ch, "%18s %3d%%  ",
			skill_table[sn]->name, ch->pcdata->learned[sn] );*/
			/* Matthew -- move to a seperate function */

			pager_printf( ch, "%15s (%4s)  ",
				skill_table[sn]->name_.c_str(), skill_prof(ch->pcdata->learned[sn]) );

			if ( ++col % 3 == 0 )
				send_to_pager( "\r\n", ch );
		}

		if ( col % 3 != 0 )
			send_to_pager( "\r\n", ch );

		pager_printf( ch, "You have %d practice sessions left.\r\n",
			ch->practice );

		return;
	}
	else
	{
		/* The actual Practicing */
		bool can_prac = TRUE;
		int adept;
		int totalcost;
		int sklvl;
		int intmod;
		int wismod;
		int randmod;
		int prct;
		int diffmod; /* intial difficulty level of skill		*/
		int levdif;  /* level difference of level - skill level */


		if ( !IS_AWAKE(ch) )
		{
			send_to_char( "In your dreams, or what?\r\n", ch );
			return;
		}

		if ( !mob )
		{
			send_to_char( "You can't do that here.\r\n", ch );
			return;
		}

		if ( !str_prefix("'", argument) ) {
			send_to_char("Note, the quotes (') aren't needed.  If having problems, try without them.\n", ch);
		}


		if ( ch->practice <= 0 )
		{
			act( AT_TELL, "$n tells you 'You must earn some more practice sessions.'",
				mob, NULL, ch, TO_VICT );
			return;
		}

		sn = skill_lookup( argument );




		/* Check if special trainer, in otherwords, if it isn't set, old style training */
		if(!mob->pIndexData->train)
		{
			send_to_char("There is no trainer here.\r\n", ch);
			return;
		}
		else
		{
			train = mob->pIndexData->train;

			/* Check alignment*/
			if ((
				((IS_EVIL(ch)) && (train->alignment != 0))
				||
				((IS_GOOD(ch)) && (train->alignment != 2))
				||
				((IS_NEUTRAL(ch)) && (train->alignment != 1)))
				&&
				(train->alignment != -1)
				)
			{
				act( AT_TELL, "$n tells you 'I will not train someone opposed to my alignment.'",
					mob, NULL, ch, TO_VICT );
				return;
			}

			if (train->_class != ch->Class && train->_class != -1)
			{
				act( AT_TELL, "$n tells you 'I can't train $t's!'",
					mob,capitalize(class_table[ch->Class]->whoName_.c_str()), ch, TO_VICT );
				return;
			}

			if (train->max_level < ch->level && train->max_level != -1)
			{
				act( AT_TELL, "$n tells you 'You already know more than I do.'\r\n$n tells you 'You will have to seek out someone else to teach you.'",
					mob, NULL, ch, TO_VICT );
				return;
			}
			if ((train->min_level > ch->level) && (train->min_level != -1))
			{
				act( AT_TELL, "$n tells you 'I cannot waste time on you.'\r\n$n tells you 'Go find someone else to teach you'",
					mob, NULL, ch, TO_VICT );
				return;
			}

			/* Check Gold for payment if required*/
			if (train->base_cost > ch->gold && train->base_cost != -1)
			{
				act( AT_YELL, "$n screams 'I will not train beggars!  You must PAY for my services!'",
					mob, NULL, ch, TO_VICT );
				return;
			}

			sn = skill_lookup( argument );
			sMatch = FALSE;

			if (!train->first_in_list) {
				if ( sn >= top_sn || sn < 0 || is_name(skill_tname[skill_table[sn]->type], CANT_PRAC))
				{
					act(AT_TELL, "$n tells you, \'You\'ll have to seek another to teach you that skill...\'", mob, NULL, ch, TO_VICT);
					return;
				}

				if (sn != -1) {
					sMatch = TRUE;
				}
			}
			else
			{
				for ( list = train->first_in_list; list; list = list->next )
				{
					if (sn == list->sn)
					{
						sMatch = TRUE;
						train_max = list->max;
						train_charge = list->cost;
						if (train_charge > 0)
						{
							totalcost = train_charge;
							if (train->base_cost != -1)
							{
								totalcost += train->base_cost;
							}
							if (totalcost > ch->gold)
							{
								act( AT_TELL, "$n tells you 'You do not have enough gold to pay for the training.'",
									mob, NULL, ch, TO_VICT );
								return;
							}
						}
						break;
					}
				}
			}
			if (!sMatch)
			{
				act( AT_TELL, "$n tells you 'I do not know how to teach that.'",
					mob, NULL, ch, TO_VICT );
				return;
			}
		}

		if ( can_prac && ( ( sn == -1 )
			|| ( !IS_NPC(ch)
			&&	 ch->level < skill_table[sn]->skill_level[ch->Class] ) ) )
		{
			act( AT_TELL, "$n tells you 'You're not ready to learn that yet...'",
				mob, NULL, ch, TO_VICT );
			return;
		}

		if ( !IS_NPC(ch) && skill_table[sn]->Prerequisites && skill_table[sn]->Prerequisites[0] != '\0' )
		{
			char Arguments[MAX_STRING_LENGTH];
			char Argument[MAX_STRING_LENGTH];
			strcpy(Arguments, skill_table[sn]->Prerequisites);

			while (strlen(Arguments) > 0)
			{
				strcpy(Arguments, one_argument(Arguments, Argument));
				if ( ch_slookup(ch, Argument) == -1 )
				{
					act( AT_TELL, "$n tells you 'You don't know the prequisites for that.'", mob, NULL, ch, TO_VICT);
					return;
				}
			}
		}

		if ( !IS_NPC(ch) && skill_table[sn]->guild != CLASS_NONE)
		{
			act( AT_TELL, "$n tells you 'That is only for members of guilds...'",
				mob, NULL, ch, TO_VICT );
			return;
		}

		adept = (int) UMAX(train_max, (class_table[ch->Class]->skill_adept * 0.2));

		if (ch->pcdata->learned[sn] >= adept)
		{
			sprintf( buf, "$n tells you, 'I've taught you everything I can about %s.'",
				skill_table[sn]->name_.c_str() );
			act( AT_TELL, buf, mob, NULL, ch, TO_VICT );
			act( AT_TELL, "$n tells you, 'You'll have to practice it on your own now...'",
				mob, NULL, ch, TO_VICT );

			return;
		}

		if ((train->first_in_list) && (train_max > 0 ))
		{
			if (ch->pcdata->learned[sn] >= train_max)
			{
				act( AT_TELL, "$n tells you 'You must seek one greater than I'",
					mob, NULL, ch, TO_VICT );
				return;
			}
		}

		sklvl = skill_table[sn]->skill_level[ch->Class];
		intmod = (UMIN(24, ch->getInt()))/6;
		wismod = (UMIN(21, ch->getWis()))/7;
		randmod = number_range( 1, 9);
		prct = 20 + intmod + wismod + randmod;
		// perfect base would be 32%, or three practices roughly

		// hardest difficulty level would only be a 20, 	  if (skill_table[sn]->difficulty  != '\0')
		if (skill_table[sn]->difficulty  != '\0')
		{
			diffmod = (int) skill_table[sn]->difficulty;

			// subtract level difference from diff mod, i.e. no prob is 20 levels higher than spell
			if (ch->level > sklvl)
			{
				levdif = UMIN(20, ch->level - sklvl);

				diffmod = UMAX(0, diffmod - levdif);
			}
			prct -= diffmod;
		}


		if (train_max < 1) {
			train_max = adept;
		}

		ch->pcdata->learned[sn] += prct;
		ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn], 100);
		ch->practice--;

		act(AT_ACTION, "You practice $T.", ch, NULL, skill_table[sn]->name_.c_str(), TO_CHAR );
		act(AT_ACTION, "$n practices $T.", ch, NULL, skill_table[sn]->name_.c_str(), TO_ROOM );

		if ( !str_cmp(skill_tname[skill_table[sn]->type], "Tongue") ) {
			int lang;

			for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ ) {
				if ( !str_cmp(lang_names[lang], skill_table[sn]->name_.c_str()) )
					break;
			}
			if ( lang_array[lang] != LANG_UNKNOWN ) {
				SET_BIT(ch->speaks, lang_array[lang]);
			}

			if ( ch->pcdata->learned[sn] < 30 ) {
				ch_printf(ch, "You have begun your lessons in %s.  You are not, however, adept enough to speak it.\r\n", skill_table[sn]->name_.c_str());
			} else if ( ch->pcdata->learned[sn] < 60 ) {
				ch_printf(ch, "You have learned a good deal of %s, but you still have far to go before you will be ready to speak it.\r\n", skill_table[sn]->name_.c_str());
			} else if ( ch->pcdata->learned[sn] < 90 ) {
				ch_printf(ch, "You feel you can start communicating in %s.\r\n",
					skill_table[sn]->name_.c_str());
			} else {
				ch_printf(ch, "You are now fluent in %s.\r\n",
					skill_table[sn]->name_.c_str());
			}
		}


		if ((train->first_in_list) && (train_max > 0 ))
		{
			if (ch->pcdata->learned[sn] >= train_max)
			{
				act( AT_TELL, "$n tells you 'I can teach you no further, You must seek one greater than I'",
					mob, NULL, ch, TO_VICT );
				return;
			}
		}
		else
		{
			act(AT_TELL,   "$n tells you 'You'll have to practice it on your own now...'",
				mob, NULL, ch, TO_VICT);
		}

		if (
			(
			(mob->pIndexData->train)
			&&
			(train->base_cost > 0)
			)
			||
			(
			(train->first_in_list)
			&&
			(train_charge > 0)
			)
			)
		{
			if ((train->first_in_list) && (train_charge > 0 ))
			{
				ch->gold -= train_charge;
			}
			if((mob->pIndexData->train) && (train->base_cost > 0))
			{
				ch->gold -= train->base_cost;
			}
			act( AT_ACTION, "$n extracts some gold from your pocket.'",
				mob, NULL, ch, TO_VICT );
		}
	}

	return;
}

void do_wimpy(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    int wimpy;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	wimpy = (int) ch->max_hit / 5;
    else
	wimpy = atoi( arg );

    if ( wimpy < 0 )
    {
	send_to_char( "Your courage exceeds your wisdom.\r\n", ch );
	return;
    }

    if ( wimpy > ch->max_hit )
    {
	send_to_char( "Such cowardice ill becomes you.\r\n", ch );
	return;
    }

    ch->wimpy	= wimpy;
    ch_printf( ch, "Wimpy set to %d hit points.\r\n", wimpy );
    return;
}



void do_forum_password(CHAR_DATA *ch, const char* argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char *pArg;
	char *pwdnew;
	char *p;
	char cEnd;

	if ( IS_NPC(ch) )
		return;

	if ( !ch->GetConnection() )
	{
		ch->sendText("You don't have a connection.\r\n");
		return;
	}
	if ( !ch->GetConnection()->Account )
	{
		ch->sendText("You don't have an account.\r\n");
		return;
	}

	/*
	 * Can't use one_argument here because it smashes case.
	 * So we just steal all its code.  Bleagh.
	 */
	pArg = arg1;
	while ( isspace(*argument) )
		argument++;

	cEnd = ' ';
	if ( *argument == '\'' || *argument == '"' )
		cEnd = *argument++;

	while ( *argument != '\0' )
	{
		if ( *argument == cEnd )
		{
			argument++;
			break;
		}
		*pArg++ = *argument++;
	}
	*pArg = '\0';

	if ( arg1[0] == '\0' )
	{
		send_to_char( "Syntax: forumpassword <password>. (without the brackets)\r\n", ch );
		return;
	}

	if ( strlen(arg1) < 5 )
	{
		send_to_char("New password must be at least five characters long.\r\n", ch );
		return;
	}

	/*
	* No tilde allowed because of player file format.
	*/
	pwdnew = crypt( arg1, ch->getName().c_str() );
	for ( p = pwdnew; *p != '\0'; p++ )
	{
		if ( *p == '~' )
		{
			send_to_char(
				"New password not acceptable, try again.\r\n", ch );
			return;
		}
	}

	ch->GetConnection()->Account->forumPassword_ = pwdnew;
	write_account_data(ch->GetConnection()->Account);

	send_to_char( "Password set.\r\n", ch );
	return;
}


void do_password(CHAR_DATA *ch, const char* argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( IS_NPC(ch) )
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: password <old> <new>.\r\n", ch );
	return;
    }

    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
	ch->AddWait(40);
	send_to_char( "Wrong password.  Wait 10 seconds.\r\n", ch );
	return;
    }

    if ( strlen(arg2) < 5 )
    {
	send_to_char(
	    "New password must be at least five characters long.\r\n", ch );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, ch->getName().c_str() );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
	    send_to_char(
		"New password not acceptable, try again.\r\n", ch );
	    return;
	}
    }

    DISPOSE( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    if ( IS_SET(sysdata.save_flags, SV_PASSCHG) )
	save_char_obj( ch );
    send_to_char( "Ok.\r\n", ch );
    return;
}

void do_commands(CHAR_DATA *ch, const char* argument)
{
    int col;
    bool found;
    int hash;
    CMDTYPE *command;

    col = 0;
    set_pager_color( AT_PLAIN, ch );
    if ( argument[0] == '\0' )
    {
	for ( hash = 0; hash < 126; hash++ )
	    for ( command = command_hash[hash]; command; command = command->next )
		if ( command->level <  LEVEL_HERO_MIN
		&&   command->level <= get_trust( ch )
		&&  (command->name[0] != 'm'
		&&   command->name[1] != 'p') )
		{
		    pager_printf( ch, "%-12s", command->name );
		    if ( ++col % 6 == 0 )
			send_to_pager( "\r\n", ch );
		}
	if ( col % 6 != 0 )
	    send_to_pager( "\r\n", ch );
    }
    else
    {
	found = FALSE;
	for ( hash = 0; hash < 126; hash++ )
	    for ( command = command_hash[hash]; command; command = command->next )
		if ( command->level <  LEVEL_HERO_MIN
		&&   command->level <= get_trust( ch )
		&&  !str_prefix(argument, command->name)
		&&  (command->name[0] != 'm'
		&&   command->name[1] != 'p') )
		{
		    pager_printf( ch, "%-12s", command->name );
		    found = TRUE;
		    if ( ++col % 6 == 0 )
			send_to_pager( "\r\n", ch );
		}

	if ( col % 6 != 0 )
	    send_to_pager( "\r\n", ch );
	if ( !found )
	    ch_printf( ch, "No command found under %s.\r\n", argument);
    }
    return;
}

void do_channels(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE) )
        {
            send_to_char( "You are silenced.\r\n", ch );
            return;
        }

        send_to_char( "Channels:", ch );

        send_to_char(IS_SET(ch->deaf, CHANNEL_AUCTION ) ? " -auction"  : " +AUCTION",  ch);
        send_to_char(IS_SET(ch->deaf, CHANNEL_CHAT    ) ? " -chat"     : " +CHAT",     ch);
        send_to_char(IS_SET(ch->deaf, CHANNEL_CLAN    ) ? " -clan"     : " +CLAN",     ch);
        send_to_char(IS_SET(ch->deaf, CHANNEL_COUNCIL ) ? " -council"  : " +COUNCIL",  ch);
        send_to_char(IS_SET(ch->deaf, CHANNEL_GUILD   ) ? " -guild"    : " +GUILD",    ch);
        send_to_char(IS_SET(ch->deaf, CHANNEL_QUEST   ) ? " -quest"    : " +QUEST",    ch);
        send_to_char(IS_SET(ch->deaf, CHANNEL_TELLS   ) ? " -tells"    : " +TELLS",    ch);
        send_to_char(IS_SET(ch->deaf, CHANNEL_NEWBIE  ) ? " -newbie"   : " +NEWBIE",   ch);
        send_to_char(IS_SET(ch->deaf, CHANNEL_MUSIC   ) ? " -music"    : " +MUSIC",    ch);
        send_to_char(IS_SET(ch->deaf, CHANNEL_ASK     ) ? " -ask"      : " +ASK",      ch);
        send_to_char(IS_SET(ch->deaf, CHANNEL_YELL    ) ? " -yell"     : " +YELL",     ch);
        send_to_char(IS_SET(ch->deaf, CHANNEL_ORDER   ) ? " -order"    : " +ORDER",    ch);
        send_to_char(IS_SET(ch->deaf, CHANNEL_OOC     ) ? " -ooc"      : " +OOC",      ch);
		send_to_char(IS_SET(ch->deaf, CHANNEL_OSAY    ) ? " -osay"     : " +OSAY",     ch);
        send_to_char(IS_SET(ch->deaf, CHANNEL_ARENA   ) ? " -arena"    : " +ARENA",    ch);


        if ( get_trust(ch) >= LEVEL_HERO_MIN ) {
          /* the 3 avchannels */
            if(
                 get_trust(ch) >= LEVEL_IMMORTAL
              || ch->pcdata->ethos==ETHOS_HERO
              )
              send_to_char(
               IS_SET(ch->deaf,CHANNEL_AVTALK) ? " -avtalk" : " +AVTALK"
               , ch);
            if(
                 get_trust(ch) >= LEVEL_IMMORTAL
              || ch->pcdata->ethos==ETHOS_VILLAIN
              )
              send_to_char(
               IS_SET(ch->deaf,CHANNEL_EVTALK) ? " -evtalk" : " +EVTALK"
               , ch);
            if(
                 get_trust(ch) >= LEVEL_IMMORTAL
              || ch->pcdata->ethos==ETHOS_SAGE
              )
              send_to_char(
               IS_SET(ch->deaf,CHANNEL_SGTALK) ? " -sgtalk" : " +SGTALK"
               , ch);

        }

        if ( IS_IMMORTAL(ch) ) {
            send_to_char(IS_SET(ch->deaf, CHANNEL_IMMTALK ) ? " -immtalk"  : " +IMMTALK",  ch);
            send_to_char(IS_SET(ch->deaf, CHANNEL_LOG     ) ? " -log"      : " +LOG",      ch);
            send_to_char(IS_SET(ch->deaf, CHANNEL_BUILD   ) ? " -build"    : " +BUILD",    ch);
            send_to_char(IS_SET(ch->deaf, CHANNEL_CODETALK) ? " -codetalk" : " +CODETALK", ch);
            send_to_char(IS_SET(ch->deaf, CHANNEL_MONITOR ) ? " -monitor"  : " +MONITOR",  ch);
            send_to_char(IS_SET(ch->deaf, CHANNEL_HIGHGOD ) ? " -muse"     : " +HIGHGOD",  ch);
            send_to_char(IS_SET(ch->deaf, CHANNEL_SHOUT   ) ? " -shout"    : " +SHOUT",    ch);
            send_to_char(IS_SET(ch->deaf, CHANNEL_COMM    ) ? " -comm"     : " +COMM",     ch);
        }

        send_to_char( ".\r\n", ch );
    }
    else
    {
        bool fClear;
        bool ClearAll;
        int bit;

        bit=0;
        ClearAll = FALSE;

        if ( arg[0] == '+' )
        {
            fClear = TRUE;
        }
        else if ( arg[0] == '-' )
        {
            fClear = FALSE;
        }
        else
        {
            send_to_char( "Channels -channel or +channel?\r\n", ch );
            return;
        }

        if      ( !str_cmp( arg+1, "auction"  ) ) bit = CHANNEL_AUCTION;
        else if ( !str_cmp( arg+1, "arena"    ) ) bit = CHANNEL_ARENA;
        else if ( !str_cmp( arg+1, "chat"     ) ) bit = CHANNEL_CHAT;
        else if ( !str_cmp( arg+1, "clan"     ) ) bit = CHANNEL_CLAN;
        else if ( !str_cmp( arg+1, "council"  ) ) bit = CHANNEL_COUNCIL;
        else if ( !str_cmp( arg+1, "guild"    ) ) bit = CHANNEL_GUILD;
        else if ( !str_cmp( arg+1, "quest"    ) ) bit = CHANNEL_QUEST;
        else if ( !str_cmp( arg+1, "tells"    ) ) bit = CHANNEL_TELLS;
        else if ( !str_cmp( arg+1, "immtalk"  ) ) bit = CHANNEL_IMMTALK;
        else if ( !str_cmp( arg+1, "log"      ) ) bit = CHANNEL_LOG;
        else if ( !str_cmp( arg+1, "build"    ) ) bit = CHANNEL_BUILD;
        else if ( !str_cmp( arg+1, "avtalk"   ) ) bit = CHANNEL_AVTALK;
        else if ( !str_cmp( arg+1, "evtalk"   ) ) bit = CHANNEL_EVTALK;
        else if ( !str_cmp( arg+1, "sgtalk"   ) ) bit = CHANNEL_SGTALK;
        else if ( !str_cmp( arg+1, "monitor"  ) ) bit = CHANNEL_MONITOR;
        else if ( !str_cmp( arg+1, "newbie"   ) ) bit = CHANNEL_NEWBIE;
        else if ( !str_cmp( arg+1, "music"    ) ) bit = CHANNEL_MUSIC;
        else if ( !str_cmp( arg+1, "muse"     ) ) bit = CHANNEL_HIGHGOD;
        else if ( !str_cmp( arg+1, "ask"      ) ) bit = CHANNEL_ASK;
        else if ( !str_cmp( arg+1, "shout"    ) && IS_IMMORTAL(ch) ) bit = CHANNEL_SHOUT;
        else if ( !str_cmp( arg+1, "yell"     ) ) bit = CHANNEL_YELL;
        else if ( !str_cmp( arg+1, "comm"     ) ) bit = CHANNEL_COMM;
        else if ( !str_cmp( arg+1, "order"    ) ) bit = CHANNEL_ORDER;
        else if ( !str_cmp( arg+1, "codetalk" ) ) bit = CHANNEL_CODETALK;
        else if ( !str_cmp( arg+1, "ooc"      ) ) bit = CHANNEL_OOC;
		else if ( !str_cmp( arg+1, "osay"     ) ) bit = CHANNEL_OSAY;
        else if ( !str_cmp( arg+1, "all"      ) ) ClearAll = TRUE;
        else
        {
            send_to_char( "Set or clear which channel?\r\n", ch );
            return;
        }

        if (( fClear ) && ( ClearAll ))
        {
            REMOVE_BIT(ch->deaf, CHANNEL_AUCTION);
            REMOVE_BIT(ch->deaf, CHANNEL_CHAT);
            REMOVE_BIT(ch->deaf, CHANNEL_CLAN);
            REMOVE_BIT(ch->deaf, CHANNEL_COUNCIL);
            REMOVE_BIT(ch->deaf, CHANNEL_GUILD);
            REMOVE_BIT(ch->deaf, CHANNEL_QUEST);
            REMOVE_BIT(ch->deaf, CHANNEL_TELLS);
            REMOVE_BIT(ch->deaf, CHANNEL_IMMTALK);
            REMOVE_BIT(ch->deaf, CHANNEL_LOG);
            REMOVE_BIT(ch->deaf, CHANNEL_BUILD);
            REMOVE_BIT(ch->deaf, CHANNEL_AVTALK);
            REMOVE_BIT(ch->deaf, CHANNEL_EVTALK);
            REMOVE_BIT(ch->deaf, CHANNEL_SGTALK);
            REMOVE_BIT(ch->deaf, CHANNEL_NEWBIE);
            REMOVE_BIT(ch->deaf, CHANNEL_MUSIC);
            REMOVE_BIT(ch->deaf, CHANNEL_HIGHGOD);
            REMOVE_BIT(ch->deaf, CHANNEL_ASK);
            REMOVE_BIT(ch->deaf, CHANNEL_SHOUT);
            REMOVE_BIT(ch->deaf, CHANNEL_YELL);
            REMOVE_BIT(ch->deaf, CHANNEL_COMM);
            REMOVE_BIT(ch->deaf, CHANNEL_ORDER);
            REMOVE_BIT(ch->deaf, CHANNEL_CODETALK);
            REMOVE_BIT(ch->deaf, CHANNEL_OOC);
			REMOVE_BIT(ch->deaf, CHANNEL_OSAY);
        }
        else if ((!fClear) && (ClearAll))
        {
               SET_BIT(ch->deaf, CHANNEL_AUCTION);
               SET_BIT(ch->deaf, CHANNEL_CHAT);
               SET_BIT(ch->deaf, CHANNEL_CLAN);
               SET_BIT(ch->deaf, CHANNEL_COUNCIL);
               SET_BIT(ch->deaf, CHANNEL_GUILD);
               SET_BIT(ch->deaf, CHANNEL_QUEST);
               SET_BIT(ch->deaf, CHANNEL_TELLS);
               SET_BIT(ch->deaf, CHANNEL_IMMTALK);
               SET_BIT(ch->deaf, CHANNEL_LOG);
               SET_BIT(ch->deaf, CHANNEL_BUILD);
               SET_BIT(ch->deaf, CHANNEL_AVTALK);
               SET_BIT(ch->deaf, CHANNEL_EVTALK);
               SET_BIT(ch->deaf, CHANNEL_SGTALK);
               SET_BIT(ch->deaf, CHANNEL_NEWBIE);
               SET_BIT(ch->deaf, CHANNEL_MUSIC);
               SET_BIT(ch->deaf, CHANNEL_HIGHGOD);
               SET_BIT(ch->deaf, CHANNEL_ASK);
               SET_BIT(ch->deaf, CHANNEL_SHOUT);
               SET_BIT(ch->deaf, CHANNEL_YELL);
               SET_BIT(ch->deaf, CHANNEL_COMM);
               SET_BIT(ch->deaf, CHANNEL_ORDER);
               SET_BIT(ch->deaf, CHANNEL_CODETALK);
               SET_BIT(ch->deaf, CHANNEL_OOC);
			   SET_BIT(ch->deaf, CHANNEL_OSAY);
        }
        else if (fClear)
        {
            REMOVE_BIT (ch->deaf, bit);
        }
        else
        {
            SET_BIT(ch->deaf, bit);
        }

        send_to_char( "Ok.\r\n", ch );
    }

    return;
}


/*
* display WIZLIST file						-Thoric
*/
void do_wizlist(CHAR_DATA *ch, const char* argument)
{
	//set_pager_color( AT_IMMORT, ch );
	//show_file( ch, WIZLIST_FILE );
	// Ksilyan: show help wizlist.
	do_help(ch, "wizlist");
}

/*
 * Contributed by Grodyn.
 */
void do_config(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];

    if ( IS_NPC(ch) )
	return;

    one_argument( argument, arg );

    set_char_color( AT_WHITE, ch );
    if ( arg[0] == '\0' )
    {
	send_to_char( "[ Keyword  ] Option\r\n", ch );

      {
	send_to_char(  IS_SET(ch->act, PLR_NICE)
	    ? "[+NICE     ] You are nice to other players.\r\n"
	    : "[-nice     ] You are not nice to other players.\r\n"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_FLEE)
	    ? "[+FLEE     ] You flee from NPCs if you get attacked.\r\n"
	    : "[-flee     ] You fight back if you get attacked.\r\n"
	    , ch );
      }

	send_to_char(  IS_SET(ch->pcdata->flags, PCFLAG_NORECALL)
	    ? "[+NORECALL ] You fight to the death, link-dead or not.\r\n"
	    : "[-norecall ] You try to recall if fighting link-dead.\r\n"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_AUTOEXIT)
	    ? "[+AUTOEXIT ] You automatically see exits.\r\n"
	    : "[-autoexit ] You don't automatically see exits.\r\n"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_AUTOLOOT)
	    ? "[+AUTOLOOT ] You automatically loot corpses.\r\n"
	    : "[-autoloot ] You don't automatically loot corpses.\r\n"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_AUTOSAC)
	    ? "[+AUTOSAC  ] You automatically sacrifice corpses.\r\n"
	    : "[-autosac  ] You don't automatically sacrifice corpses.\r\n"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_AUTOGOLD)
	    ? "[+AUTOGOLD ] You automatically split gold from kills in groups.\r\n"
	    : "[-autogold ] You don't automatically split gold from kills in groups.\r\n"
	    , ch );

        send_to_char(  IS_SET(ch->pcdata->flags, PCFLAG_GAG)
            ? "[+GAG      ] You see only necessary battle text.\r\n"
            : "[-gag      ] You see full battle text.\r\n"
            , ch );

        send_to_char(  IS_SET(ch->pcdata->flags, PCFLAG_PAGERON)
            ? "[+PAGER    ] Long output is page-paused.\r\n"
            : "[-pager    ] Long output scrolls to the end.\r\n"
            , ch );

	send_to_char(  IS_SET(ch->act, PLR_BLANK)
	    ? "[+BLANK    ] You have a blank line before your prompt.\r\n"
	    : "[-blank    ] You have no blank line before your prompt.\r\n"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_BRIEF)
	    ? "[+BRIEF    ] You see brief descriptions.\r\n"
	    : "[-brief    ] You see long descriptions.\r\n"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_COMBINE)
	    ? "[+COMBINE  ] You see object lists in combined format.\r\n"
	    : "[-combine  ] You see object lists in single format.\r\n"
	    , ch );

	send_to_char(  IS_SET(ch->pcdata->flags, PCFLAG_NOINTRO)
	    ? "[+NOINTRO  ] You don't see the ascii intro screen on login.\r\n"
	    : "[-nointro  ] You see the ascii intro screen on login.\r\n"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_PROMPT)
	    ? "[+PROMPT   ] You have a prompt.\r\n"
	    : "[-prompt   ] You don't have a prompt.\r\n"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_TELNET_GA)
	    ? "[+TELNETGA ] You receive a telnet GA sequence.\r\n"
	    : "[-telnetga ] You don't receive a telnet GA sequence.\r\n"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_ANSI)
	    ? "[+ANSI     ] You receive ANSI color sequences.\r\n"
	    : "[-ansi     ] You don't receive receive ANSI colors.\r\n"
	    , ch );

	send_to_char(  IS_SET(ch->act, PLR_RIP)
	    ? "[+RIP      ] You receive RIP graphic sequences.\r\n"
	    : "[-rip      ] You don't receive RIP graphics.\r\n"
	    , ch );

	  send_to_char(  IS_SET(ch->act, PLR_SHOVEDRAG)
	      ? "[+SHOVEDRAG] You allow yourself to be shoved and dragged around.\r\n"
	      : "[-shovedrag] You'd rather not be shoved or dragged around.\r\n"
	      , ch );

	send_to_char(  IS_SET( ch->pcdata->flags, PCFLAG_NOSUMMON )
	      ? "[+NOSUMMON ] You do not allow other players to summon you.\r\n"
	      : "[-nosummon ] You allow other players to summon you.\r\n"
	      , ch );

	if ( IS_IMMORTAL( ch ) )
	  send_to_char(  IS_SET(ch->act, PLR_ROOMVNUM)
	      ? "[+VNUM     ] You can see the VNUM of a room.\r\n"
	      : "[-vnum     ] You do not see the VNUM of a room.\r\n"
	      , ch );

	if ( IS_IMMORTAL( ch ) )
	  send_to_char(  IS_SET(ch->act, PLR_AUTOMAP)    /* maps */
	      ? "[+MAP      ] You can see the MAP of a room.\r\n"
	      : "[-map      ] You do not see the MAP of a room.\r\n"
	      , ch );

	send_to_char(  IS_SET(ch->act, PLR_SILENCE)
	    ? "[+SILENCE  ] You are silenced.\r\n"
	    : ""
	    , ch );

    send_to_char(  IS_SET(ch->act, PLR_TICKMSG)
        ? "[+TICKMSG  ] You can see area reset messages.\r\n"
        : "[-tickmsg  ] You do not see area reset messages.\r\n"
        , ch );


	send_to_char( !IS_SET(ch->act, PLR_NO_EMOTE)
	    ? ""
	    : "[-emote    ] You can't emote.\r\n"
	    , ch );

	send_to_char( !IS_SET(ch->act, PLR_NO_TELL)
	    ? ""
	    : "[-tell     ] You can't use 'tell'.\r\n"
	    , ch );
	send_to_char( !IS_SET(ch->act, PLR_LITTERBUG)
	    ? ""
	    : "[-litter  ] A convicted litterbug. You cannot drop anything.\r\n"
	    , ch );
    }
    else
    {
	bool fSet;
	int bit = 0;

	     if ( arg[0] == '+' ) fSet = TRUE;
	else if ( arg[0] == '-' ) fSet = FALSE;
	else
	{
	    send_to_char( "Config -option or +option?\r\n", ch );
	    return;
	}

	     if ( !str_prefix( arg+1, "autoexit" ) ) bit = PLR_AUTOEXIT;
	else if ( !str_prefix( arg+1, "autoloot" ) ) bit = PLR_AUTOLOOT;
	else if ( !str_prefix( arg+1, "autosac"  ) ) bit = PLR_AUTOSAC;
	else if ( !str_prefix( arg+1, "autogold" ) ) bit = PLR_AUTOGOLD;
	else if ( !str_prefix( arg+1, "blank"    ) ) bit = PLR_BLANK;
	else if ( !str_prefix( arg+1, "brief"    ) ) bit = PLR_BRIEF;
	else if ( !str_prefix( arg+1, "combine"  ) ) bit = PLR_COMBINE;
	else if ( !str_prefix( arg+1, "prompt"   ) ) bit = PLR_PROMPT;
	else if ( !str_prefix( arg+1, "telnetga" ) ) bit = PLR_TELNET_GA;
	else if ( !str_prefix( arg+1, "ansi"     ) ) bit = PLR_ANSI;
	else if ( !str_prefix( arg+1, "rip"      ) ) bit = PLR_RIP;
    else if ( !str_prefix( arg+1, "tickmsg"  ) ) bit = PLR_TICKMSG;
	else if ( !str_prefix( arg+1, "flee"     ) ) bit = PLR_FLEE;
	else if ( !str_prefix( arg+1, "nice"     ) ) bit = PLR_NICE;
	else if ( !str_prefix( arg+1, "shovedrag") ) bit = PLR_SHOVEDRAG;
	else if ( IS_IMMORTAL( ch )
	     &&   !str_prefix( arg+1, "vnum"     ) ) bit = PLR_ROOMVNUM;
	else if ( IS_IMMORTAL( ch )
	     &&   !str_prefix( arg+1, "map"      ) ) bit = PLR_AUTOMAP;     /* maps */

	if (bit)
        {
	  if ( fSet )
	    SET_BIT    (ch->act, bit);
	  else
	    REMOVE_BIT (ch->act, bit);
	  send_to_char( "Ok.\r\n", ch );
          return;
        }
        else
        {
	       if ( !str_prefix( arg+1, "norecall" ) ) bit = PCFLAG_NORECALL;
	  else if ( !str_prefix( arg+1, "nointro"  ) ) bit = PCFLAG_NOINTRO;
	  else if ( !str_prefix( arg+1, "nosummon" ) ) bit = PCFLAG_NOSUMMON;
          else if ( !str_prefix( arg+1, "gag"      ) ) bit = PCFLAG_GAG;
          else if ( !str_prefix( arg+1, "pager"    ) ) bit = PCFLAG_PAGERON;
          else
	  {
	    send_to_char( "Config which option?\r\n", ch );
	    return;
    	  }

          if ( fSet )
	    SET_BIT    (ch->pcdata->flags, bit);
	  else
	    REMOVE_BIT (ch->pcdata->flags, bit);

	  send_to_char( "Ok.\r\n", ch );
          return;
        }
    }

    return;
}


void do_credits(CHAR_DATA *ch, const char* argument)
{
  do_help( ch, "credits" );
}


extern int top_area;

/*
void do_areas(CHAR_DATA *ch, const char* argument)
{
    AREA_DATA *pArea1;
    AREA_DATA *pArea2;
    int iArea;
    int iAreaHalf;

    iAreaHalf = (top_area + 1) / 2;
    pArea1    = first_area;
    pArea2    = first_area;
    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
	pArea2 = pArea2->next;

    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
    {
	ch_printf( ch, "%-39s%-39s\r\n",
	    pArea1->name, pArea2 ? pArea2->name : "" );
	pArea1 = pArea1->next;
	if ( pArea2 )
	    pArea2 = pArea2->next;
    }

    return;
}
*/

/*
 * New do_areas with soft/hard level ranges
 */

void do_areas(CHAR_DATA *ch, const char* argument)
{
	int count = 0;
	int tcount = 0;
	float percentage = 0.0;
	AREA_DATA *pArea;
	int eLevel = -1;
	int rLevel = -1;
	bool bAuth = FALSE;
	char sAuth[MAX_INPUT_LENGTH];
	bool bName = FALSE;
	char sName[MAX_INPUT_LENGTH];
	bool bAll  = FALSE;
	char arg[MAX_INPUT_LENGTH];

	if ( !argument || argument[0] == '\0' )
	{
		send_to_char("Usage: areas <options>\n\n", ch);
		send_to_char("Where options is any combination of:\n\n", ch);
		send_to_char("    author <authorname>   -- Areas by <authorname>\n", ch);
		send_to_char("    rlevel <level>        -- Areas whose recomended level range includes <level>\n", ch);
		send_to_char("    elevel <level>        -- Areas whose enforced level range includes <level>\n", ch);
		send_to_char("    name <text>           -- Searches for <text> in the area name\n", ch);
		send_to_char("    all                   -- Lists all areas\n", ch);
		send_to_char("    here                  -- your current area\n",ch);
		return;
	}


	while ( argument && argument[0] != '\0' )
	{
		argument = one_argument(argument, arg);

		if ( !str_prefix(arg, "here") )
		{
			pArea=ch->GetInRoom()->area;

			set_pager_color( AT_PLAIN, ch );
			send_to_pager("\r\n   Author    |             Area                     | Recommended |  Enforced\r\n", ch);

			send_to_pager("-------------+--------------------------------------+--------------+-----------\r\n", ch);


			pager_printf(ch, "%-12s | %-36s | %4d - %-4d | %3d - %-3d \r\n",
			             pArea->author, pArea->name, pArea->low_soft_range,
			             pArea->hi_soft_range, pArea->low_hard_range,
			             pArea->hi_hard_range);

			return;
		}

		if ( !str_cmp(arg, "all") )
		{
			bAll = TRUE;
		}
		else if ( !str_prefix(arg, "author") )
		{
			if ( !argument || argument[0] == '\0' )
			{
				send_to_char("Argument required for author argument.\r\n", ch);
				return;
			}

			argument = one_argument(argument, arg);
			strcpy(sAuth, arg);
			bAuth = TRUE;
		}
		else if ( !str_prefix(arg, "name") )
		{
			if ( !argument || argument[0] == '\0' )
			{
				send_to_char("Argument required for name argument.\r\n", ch);
				return;
			}

			argument = one_argument(argument, arg);
			strcpy(sName, arg);
			bName = TRUE;
		}
		else if ( !str_prefix(arg, "elevel") )
		{
			if ( !argument || argument[0] == '\0' )
			{
				send_to_char("Argument required for elevel argument.\r\n", ch);
				return;
			}

			argument = one_argument(argument, arg);

			if ( !is_number(arg) )
			{
				send_to_char("The level for elevel must be a number!\r\n", ch);
				return;
			}

			eLevel = atoi(arg);
		}
		else if ( !str_prefix(arg, "rlevel") )
		{
			if ( !argument || argument[0] == '\0' )
			{
				send_to_char("Argument required for rlevel argument.\r\n", ch);
				return;
			}

			argument = one_argument(argument, arg);

			if ( !is_number(arg) )
			{
				send_to_char("The level for rlevel must be a number!\r\n", ch);
				return;
			}

			rLevel = atoi(arg);
		}
	}

	set_pager_color( AT_PLAIN, ch );
	send_to_pager("\r\n   Author    |             Area                     | Recommended |  Enforced\r\n", ch);
	send_to_pager("-------------+--------------------------------------+-------------+-----------\r\n", ch);

	for ( pArea = first_area; pArea; pArea = pArea->next )
	{
		if ( IS_SET(pArea->flags, AFLAG_NOTINLIST) && get_trust(ch) < LEVEL_ENGINEER )
			continue;
		tcount++;
		if ( !bAll )
		{
			if ( bAuth )
			{
				char buf1[MAX_STRING_LENGTH];
				char buf2[MAX_STRING_LENGTH];
				strcpy(buf1, strupper(pArea->author));
				strcpy(buf2, strupper(sAuth));
				if (!strstr(buf1, buf2))
					continue;
			}
			if ( bName )
			{
				char buf1[MAX_INPUT_LENGTH];
				char buf2[MAX_INPUT_LENGTH];
				strcpy(buf1, strlower(pArea->name));
				strcpy(buf2, strlower(sName));
				if ( strstr(buf1, buf2) == NULL )
					continue;
			}
			if ( eLevel != -1 && !(eLevel >= pArea->low_hard_range && eLevel <= pArea->hi_hard_range ) )
				continue;
			if ( rLevel != -1 && !(rLevel >= pArea->low_soft_range && rLevel <= pArea->hi_soft_range ) )
				continue;
		}

		count++;

		pager_printf(ch, "%-12s | %-36s | %4d - %-4d | %3d - %-3d \r\n",
		             pArea->author, pArea->name, pArea->low_soft_range,
		             pArea->hi_soft_range, pArea->low_hard_range,
		             pArea->hi_hard_range);
	}

	percentage = (float)count / (float)tcount;
	percentage = percentage * 100;

	pager_printf(ch, "Your search returned %d out of %d areas. (%.0f%%)\r\n",
	             count, tcount, percentage);
	return;
}

void do_afk(CHAR_DATA *ch, const char* argument)
{
     if ( IS_NPC(ch) )
     return;

    if ( ch->IsFighting() )
	{
        send_to_char("It wouldn't be nice to go AFK in the middle of a fight.\r\n", ch);
        return;
    }

     if IS_SET(ch->act, PLR_AFK)
     {
    	REMOVE_BIT(ch->act, PLR_AFK);
	send_to_char( "You are no longer afk.\r\n", ch );
	act(AT_GREY,"$n is no longer afk.", ch, NULL, NULL, TO_ROOM);
     }
     else
     {
	SET_BIT(ch->act, PLR_AFK);
	send_to_char( "You are now afk.\r\n", ch );
	act(AT_GREY,"$n is now afk.", ch, NULL, NULL, TO_ROOM);
	return;
     }

}

void do_slist(CHAR_DATA *ch, const char* argument)
{
	int sn, i, lFound;
	char skn[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int lowlev, hilev;
	sh_int lasttype = SKILL_SPELL;

	if  ( IS_NPC(ch) )
		return;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	lowlev=1;
	hilev=50;

	if  (arg1[0]!='\0')
		lowlev=atoi(arg1);

	if  ((lowlev<1) || (lowlev>LEVEL_IMMORTAL))
		lowlev=1;

	if  (arg2[0]!='\0')
		hilev=atoi(arg2);

	if  ((hilev<0) || (hilev>=LEVEL_IMMORTAL))
		hilev=LEVEL_HERO_MAX;

	if  (lowlev>hilev)
		lowlev=hilev;

	set_pager_color( AT_MAGIC, ch );
	send_to_pager("SPELL & SKILL LIST\r\n",ch);
	send_to_pager("------------------\r\n",ch);

	for (i=lowlev; i <= hilev; i++)
	{
		lFound= 0;
		sprintf(skn,"Spell");
		for ( sn = 0; sn < top_sn; sn++ )
		{
			if ( skill_table[sn]->name_ == "" )
				break;

			if ( skill_table[sn]->type != lasttype )
			{
				lasttype = skill_table[sn]->type;
				strcpy( skn, skill_tname[lasttype] );
			}

			if ( ch->pcdata->learned[sn] == 0
					&&   SPELL_FLAG(skill_table[sn], SF_SECRETSKILL) )
				continue;

			if(i==skill_table[sn]->skill_level[ch->Class]  )
			{
				if( !lFound )
				{
					lFound=1;
					pager_printf( ch, "Level %d\r\n", i );
				}

				int learned = ch->pcdata->learned[sn];
				int maxLearn = skill_table[sn]->skill_adept[ch->Class];

				if ( learned >= maxLearn )
				{
					pager_printf(ch, "%7s: %20.20s \t Current: adept Max: %s\r\n",
							skn, skill_table[sn]->name_.c_str(), skill_prof(maxLearn) );
				}
				else
				{
					pager_printf(ch, "%7s: %20.20s \t Current: %s Max: %s\r\n",
						skn, skill_table[sn]->name_.c_str(),
						skill_prof(learned),
						skill_prof(maxLearn) );
				}
			}
		}
	}
	return;
}

// forward declaration
void do_comment(CHAR_DATA *ch, const char* argument);

void do_whois(CHAR_DATA *ch, const char* argument)
{
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  buf[0] = '\0';

  if(IS_NPC(ch))
    return;

  if(argument[0] == '\0')
  {
    send_to_char("You must input the name of a player online.\r\n", ch);
    return;
  }

  if ( !CanGlobalCommunicate(ch) )
  {
  	send_to_char(MESSAGE_NO_COMM_DEVICE, ch);
	return;
  }

  strcat(buf, "0.");
  strcat(buf, argument);
  if( ( ( victim = get_char_world(ch, buf) ) == NULL ))
  {
    send_to_char("No such player online.\r\n", ch);
    return;
  }

  if(IS_NPC(victim))
  {
    send_to_char("That's not a player!\r\n", ch);
    return;
  }

  ch_printf(ch, "%s is %s",
	victim->getName().c_str(),
	victim->sex == SEX_MALE ? "male" :
	victim->sex == SEX_FEMALE ? "female" : "neutral");
  if (IS_IMMORTAL(ch))
    ch_printf(ch, " in room %s.\r\n",
	vnum_to_dotted(victim->GetInRoom()->vnum));
  else
    ch_printf(ch, ".\r\n");

  if ( victim->pcdata->clan )
  {
     if ( victim->pcdata->clan->clan_type == CLAN_ORDER )
       ch_printf(ch, "%s belongs to the Order ", victim->getName().c_str() );
     else
	if ( victim->pcdata->clan->clan_type == CLAN_GUILD )
	 ch_printf(ch, "%s belongs to the ", victim->getName().c_str());
	else
       ch_printf(ch, "%s belongs to Clan ", victim->getName().c_str());

	send_to_char( victim->pcdata->clan->name_.c_str(), ch );
     send_to_char(".\r\n", ch);
  }

  if(victim->pcdata->council)
    ch_printf(ch, "%s belongs to the %s.\r\n",
	victim->getName().c_str(),
	victim->pcdata->council->name_.c_str());

  if(victim->pcdata->deity)
    ch_printf(ch, "%s has found succor in the deity %s.\r\n",
	victim->getName().c_str(),
	victim->pcdata->deity->name_.c_str());

  if(victim->pcdata->homepage && victim->pcdata->homepage[0] != '\0')
    ch_printf(ch, "%s's homepage can be found at %s.\r\n",
	victim->getName().c_str(),
	victim->pcdata->homepage);

  if(victim->pcdata->bio && victim->pcdata->bio[0] != '\0')
    ch_printf(ch, "%s's personal bio:\r\n%s",
	victim->getName().c_str(),
	victim->pcdata->bio);

  ch_printf(ch, "%s's arena rating is: %d\r\n",
        victim->getName().c_str(),
        victim->pcdata->rating);

  if(IS_IMMORTAL(ch))
  {
    send_to_char("----------------------------------------------------\r\n", ch);

    send_to_char("Info for immortals:\r\n", ch);

     ch_printf(ch, "%s is a %s level %d %s %s",
	                      victim->getName().c_str(),
	       victim->sex == SEX_MALE ? "male" :
	       victim->sex == SEX_FEMALE ? "female" : "neutral",
	                      victim->level,
	                      npc_race[victim->race],
	                      npc_class[victim->Class]);
          if (IS_IMMORTAL(ch))
                ch_printf(ch, " in room %s.\r\n",
                                 vnum_to_dotted(victim->GetInRoom()->vnum));
          else
                ch_printf(ch, ".\r\n");


     if ( victim->pcdata->authedBy_.length() > 0 )
	ch_printf(ch, "%s was authorized by %s.\r\n",
		victim->getName().c_str(), victim->pcdata->authedBy_.c_str());

    ch_printf(ch, "%s has killed %d times, and been killed by a mobile %d times.\r\n",
		victim->getName().c_str(), victim->pcdata->mkills, victim->pcdata->mdeaths );
    if ( victim->pcdata->pkills || victim->pcdata->pdeaths )
	ch_printf(ch, "%s has killed %d players, and been killed by a player %d times.\r\n",
		victim->getName().c_str(), victim->pcdata->pkills, victim->pcdata->pdeaths );
    if ( victim->pcdata->illegal_pk )
	ch_printf(ch, "%s has committed %d player kills.\r\n",
		victim->getName().c_str(), victim->pcdata->illegal_pk );

    ch_printf(ch, "%s is %shelled at the moment.\r\n",
	victim->getName().c_str(),
	(victim->pcdata->secReleaseDate == 0) ? "not " : "");

    if(victim->pcdata->secReleaseDate != 0)
      ch_printf(ch, "%s was helled by %s, and will be released on %24.24s.\r\n",
	victim->sex == SEX_MALE ? "He" :
	victim->sex == SEX_FEMALE ? "She" : "It",
        victim->pcdata->helledBy_.c_str(),
	ctime(&victim->pcdata->secReleaseDate));

    if(get_trust(victim) < get_trust(ch))
    {
      sprintf(buf2, "list %s", buf);
      do_comment(ch, buf2);
    }

    if(IS_SET(victim->act, PLR_SILENCE) || IS_SET(victim->act, PLR_NO_EMOTE)
       || IS_SET(victim->act, PLR_NO_TELL)
       )
    {
      sprintf(buf2, "This player has the following flags set:");
      if(IS_SET(victim->act, PLR_SILENCE))
        strcat(buf2, " silence");
      if(IS_SET(victim->act, PLR_NO_EMOTE))
        strcat(buf2, " noemote");
      if(IS_SET(victim->act, PLR_NO_TELL) )
        strcat(buf2, " notell");
      strcat(buf2, ".\r\n");
      send_to_char(buf2, ch);
    }
     if ( victim->GetConnection() && victim->GetConnection()->GetHostString().length() != 0 )   /* added by Gorog */
     {
         sprintf (buf2, "%s's IP info: %s ", victim->getName().c_str(), victim->GetConnection()->GetHost() );
         if (get_trust(ch) >= LEVEL_STONE_INITIATE)
            strcat (buf2, victim->GetConnection()->User.c_str());
         strcat (buf2, "\r\n");
         send_to_char(buf2, ch);
     }
     if (victim->pcdata) {
        sprintf (buf2, "%s's account: %s", victim->getName().c_str(), victim->pcdata->email);
        send_to_char(buf2, ch);
     }

  }
  if (victim->IsSnooped() && !ch->IsSnooped()
     && ch->GetConnection()->Account && ch->GetConnection()->Account->flags & ACCOUNT_NOTIFY)
  {
     sprintf(buf2, "~~ %s is being snooped by %s ~~\r\n",
             victim->getName().c_str(), victim->SnoopedByNames().c_str()
            );
     ch->sendText(buf2, false);
  }
}

void do_pager(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];

	if ( IS_NPC(ch) )
		return;
	argument = one_argument(argument, arg);
	if ( !*arg )
	{
		/*if ( IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) )
			do_config(ch, "-pager");
		else
			do_config(ch, "+pager");
		return;*/
		ch_printf(ch, "Your pager is currently set to %d lines.\r\n", ch->pcdata->pagerlen);
		return;
	}
	if ( !is_number(arg) )
	{
		send_to_char( "Set page pausing to how many lines?\r\n", ch );
		return;
	}
	ch->pcdata->pagerlen = atoi(arg);
	if ( ch->pcdata->pagerlen < 5 )
		ch->pcdata->pagerlen = 5;
	ch_printf( ch, "Page pausing set to %d lines.\r\n", ch->pcdata->pagerlen );
	return;
}

/* Ksilyan
 * do_newwho, & associated structures/functions
 * This is the newest who function that while simple
 * does exactly what we want it to do.
 * Most notably, it incorporates the new communication
 * rules.
 */

bool IsOnNetwork(CHAR_DATA * ch)
{
	OBJ_DATA * obj;
	if (!ch)
	{
		bug("IsOnNetwork: Trying to check a null CH.");
		return FALSE;
	}

	// Newbies and Av+ are always on the network
	if (get_trust(ch) <= 10 || get_trust(ch) >= LEVEL_HERO_MIN )
		return TRUE;

	// returns true if ch is connected to the network
	// i.e. has a communication device

	for (obj = ch->first_carrying; obj; obj=obj->next_content)
	{
		if ( (IS_SET(obj->extra_flags_2, ITEM_COMMUNICATION_DEVICE)) && (obj->wear_loc != WEAR_NONE) )
		{
			return TRUE;
		}
	}

	return FALSE;
}


class WhoData
{
public:
	// Members
	int RaceColor;
	shared_str Race;

	int RankColor;
	shared_str Rank;

	shared_str Name;
	shared_str Title;

	int ClanColor;
	shared_str Clan;

	int CouncilColor;
	shared_str Council;

	string ExtraData;

	bool OnNetwork;

	//////////////////
	// Constructors //
	//////////////////

	// Default - empty
	WhoData()
	{
		RaceColor = RankColor = ClanColor = CouncilColor = AT_PLAIN;
		OnNetwork = false;
	}

	// Create from character
	WhoData(Character * ch)
	{
		WhoData();

		if (!ch)
			return;

		switch (ch->race)
		{
			default:
				this->RaceColor = AT_WHITE;
				break;
			case RACE_ELF:
				this->RaceColor = AT_GREEN;
				break;
			case RACE_DWARF: case RACE_GNOME:
				this->RaceColor = AT_ORANGE;
				break;
			case RACE_PIXIE:
				this->RaceColor = AT_CYAN;
				break;
			case RACE_HALFLING:
				this->RaceColor = AT_BLUE;
				break;
			case RACE_HALF_ELF:
				this->RaceColor = AT_LBLUE;
				break;
			case RACE_KATRIN:
				this->RaceColor = AT_DBLUE;
				break;
			case RACE_GITH:
				this->RaceColor = AT_YELLOW;
			case RACE_HALF_OGRE: case RACE_HALF_TROLL:
				this->RaceColor = AT_GREY;
				break;
			case RACE_MINOTAUR:
				this->RaceColor = AT_RED;
				break;
			case RACE_HALF_ORC:
				this->RaceColor = AT_DGREEN;
				break;
		}

		this->Race = CenterString(race_table[ch->race].race_name, 16);

		if (ch->pcdata && ch->pcdata->rank_.length() > 0)
			this->Rank = ch->pcdata->rank_;
		else
		{
			char buf[100];
			sprintf(buf, "%s %d", class_table[ch->Class]->whoName_.c_str(), ch->level);
			this->Rank = buf;
		}

		this->Rank = CenterString(this->Rank.c_str(), 16);

		this->Name = ch->getName();

		if (ch->pcdata && ch->pcdata->title_.length() > 0 )
			this->Title = player_title(ch->pcdata->title_.c_str());
		else
		{
			this->Title = string(" the ") + class_table[ch->Class]->whoName_.str();
		}

		if (ch->pcdata && ch->pcdata->clan)
		{
			ClanData * pclan = ch->pcdata->clan;

			if ( pclan->clan_type == CLAN_ORDER )
			{
				this->Clan += "<";

				if ( ch->getName().ciEqual(pclan->deity_) )
					this->Clan += ("Deity, Order of ");
				else if ( ch->getName().ciEqual( pclan->leader_ ) )
					this->Clan += ("Leader, Order of ");
				else if ( ch->getName().ciEqual( pclan->number1_ ) )
					this->Clan += ("Number One, Order of ");
				else if ( ch->getName().ciEqual( pclan->number2_ ) )
					this->Clan += ("Number Two, Order of ");
				else
					this->Clan += ("Order of ");

				this->Clan += (pclan->name_);
				this->Clan += (">");
			}
			else if ( pclan->clan_type == CLAN_GUILD )
			{
				this->Clan += ("<");
				if ( ch->getName().ciEqual( pclan->leader_ ) )
					this->Clan += ("Leader, ");
				else if ( ch->getName().ciEqual( pclan->number1_ ) )
					this->Clan += ("First, ");
				else if ( ch->getName().ciEqual( pclan->number2_ ) )
					this->Clan += ("Second, ");
				this->Clan += (pclan->name_);
				this->Clan += (">");
			}
			else
			{
				this->Clan += ("(");
				if ( ch->getName().ciEqual( pclan->deity_ ) )
					this->Clan += ("Deity of ");
				else if ( ch->getName().ciEqual( pclan->leader_ ) )
					this->Clan += ("Leader of ");
				else if ( ch->getName().ciEqual( pclan->number1_ ) )
					this->Clan += ("Number One ");
				else if ( ch->getName().ciEqual( pclan->number2_ ) )
					this->Clan += ("Number Two ");
				this->Clan += (pclan->name_);
				this->Clan += (")");
			}
			this->ClanColor = ch->pcdata->clan->color;
		}

		if (ch->pcdata && ch->pcdata->council)
		{
			this->Council = "[";
			if ( ch->getName().ciEqual( ch->pcdata->council->head_ ) )
				this->Council += ("Head of ");
			this->Council += (ch->pcdata->council->name_.c_str());
			this->Council += ("]");
			this->CouncilColor = AT_LBLUE;
		}

		if ( ch->GetConnection() && ch->GetConnection()->ConnectedState == CON_EDITING )
			this->ExtraData += (" (writing)");

		// Don't need to check for trust since won't display if can't see.
		if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_WIZINVIS) )
		{
			char buf[100];
			sprintf(buf, " (I:%d)", ch->pcdata->wizinvis);
			this->ExtraData += (buf);
		}

		if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AFK) )
			this->ExtraData += (" [Absent]");

		this->OnNetwork = IsOnNetwork(ch);
	}
};

struct fo_WhoDataCompare : greater <WhoData*>
{
public:
	bool operator() (const WhoData * firstWho, const WhoData * secondWho) const
    {
		if (firstWho->Name.length() == 0)
			return false;
		if (secondWho->Name.length() == 0)
			return true;

		fo_StringCompare_i compare;
		return compare( firstWho->Name.str(), secondWho->Name.str() );
	};
};

// Update a string by centering it in the given width.
void CenterString(char * String, unsigned short Width)
{
	unsigned int Length;
	unsigned short i;
	char LocalBuf[MAX_INPUT_LENGTH];

	// If there is no string, or if it's greater than
	// width anyways, then there is nothing to do.
	if (!String || strlen(String) == 0 || strlen(String) >= Width)
		return;

	Length = 0;

	for (i = 0; i < strlen(String) - 1; i++)
	{
		if (String[i] == '&' && String[i+1] != '&')
			i++;
		else if (String[i] == '^' && String[i+1] != '^')
			i++;
		else
			Length++;
	}

	strcpy(LocalBuf, "");

	for (i = 0; i < Width/2 - Length/2 - 1; i++)
		strcat(LocalBuf, " ");
	strcat(LocalBuf, String);
	for (i = Width/2 + Length/2; i < Width - 1; i++)
		strcat(LocalBuf, " ");
	if (Length % 2 == 0)
		strcat(LocalBuf, " "); // odd length so add one

	strcpy(String, LocalBuf);
}

void do_newwho(CHAR_DATA * ch, const char* argument)
{
	list<WhoData*> Immortals;
	list<WhoData*> Avatars;
	list<WhoData*> Mortals;
	list<WhoData*>::iterator whoListItor;

	bool Connected = FALSE;

	Connected = IsOnNetwork(ch);

	/*
	 * Build the lists.
	 */

	itorSocketId itor;
	for ( itor = gTheWorld->GetBeginConnection(); itor != gTheWorld->GetEndConnection(); itor++ )
	{
		// we know that the world's player connection list only holds player connections IDs,
		// so we can safely cast it to PlayerConnection*
		PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];

		CHAR_DATA * vch;
		vch = d->GetOriginalCharacter();

		if (!vch)
			continue;

		// Don't show players still creating
		if ( d->ConnectedState < 0 )
			continue;

		// If vch is wizinvis then don't show them.
		if ( IS_SET(vch->act, PLR_WIZINVIS) && vch->pcdata->wizinvis > get_trust(ch) )
			continue;

		if ( vch->level <= 50 )
			Mortals.push_back(new WhoData(vch));
		else if ( vch->level >= 51 && vch->level <= 55)
			Avatars.push_back(new WhoData(vch));
		else
			Immortals.push_back(new WhoData(vch));
	}


	Immortals.sort( fo_WhoDataCompare() );
	Avatars.sort( fo_WhoDataCompare() );
	Mortals.sort( fo_WhoDataCompare() );

	/*
	 * And now... finally... print out the who list.
	 */

	set_char_color(AT_PLAIN, ch);
	ch_printf(ch, "\r\n");
	ch_printf(ch, "+----------------+\r\n");

	// Print out Immortal list
	for (whoListItor = Immortals.begin(); whoListItor != Immortals.end(); whoListItor++)
	{
		WhoData * whoData = (*whoListItor);

		set_char_color(AT_PLAIN, ch);
		send_to_char("|", ch);
		set_char_color(AT_WHITE, ch);
		ch->sendText(whoData->Rank.str());

		set_char_color(AT_PLAIN, ch);
		ch_printf(ch, "| %s%s", whoData->Name.c_str(), whoData->Title.c_str());
		if ( whoData->Clan.length() > 0)
		{
			set_char_color(whoData->ClanColor, ch);
			ch_printf(ch, " %s", whoData->Clan.c_str() );
			set_char_color(AT_PLAIN, ch);
		}
		if ( whoData->Council.length() > 0)
		{
			set_char_color(whoData->CouncilColor, ch);
			ch_printf(ch, " %s", whoData->Council.c_str() );
			set_char_color(AT_PLAIN, ch);
		}
		if ( whoData->ExtraData.length() > 0)
		{
			set_char_color(AT_PLAIN, ch);
			ch_printf(ch, " %s", whoData->ExtraData.c_str() );
			set_char_color(AT_PLAIN, ch);
		}
		send_to_char("\r\n", ch);
	}
	if ( Immortals.empty() == false )
	{
		set_char_color(AT_PLAIN, ch);
		ch_printf(ch, "+----------------+\r\n");
	}

	// Print out Avatar list
	for (whoListItor = Avatars.begin(); whoListItor != Avatars.end(); whoListItor++)
	{
		WhoData * whoData = (*whoListItor);
		set_char_color(AT_PLAIN, ch);
		ch->sendText("|", false);
		ch->sendText(whoData->Rank.str());

		set_char_color(AT_PLAIN, ch);
		ch_printf(ch, "| %s%s", whoData->Name.c_str(), whoData->Title.c_str());
		if ( whoData->Clan.length() > 0)
		{
			set_char_color(whoData->ClanColor, ch);
			ch_printf(ch, " %s", whoData->Clan.c_str() );
			set_char_color(AT_PLAIN, ch);
		}
		if ( whoData->Council.length() > 0)
		{
			set_char_color(whoData->CouncilColor, ch);
			ch_printf(ch, " %s", whoData->Council.c_str() );
			set_char_color(AT_PLAIN, ch);
		}
		if ( whoData->ExtraData.length() > 0)
		{
			set_char_color(AT_PLAIN, ch);
			ch_printf(ch, " %s", whoData->ExtraData.c_str() );
			set_char_color(AT_PLAIN, ch);
		}

		ch->sendText("\r\n");
	}
	if (Avatars.empty() == false)
	{
		set_char_color(AT_PLAIN, ch);
		ch_printf(ch, "+----------------+\r\n");
	}

	for (whoListItor = Mortals.begin(); whoListItor != Mortals.end(); whoListItor++)
	{
		WhoData * whoData = (*whoListItor);

		set_char_color(AT_PLAIN, ch);
		send_to_char("|", ch);
		set_char_color(whoData->RaceColor, ch);
		ch->sendText(whoData->Race.str());
		set_char_color(AT_PLAIN, ch);

		if ( Connected || !whoData->Name.str().compare(ch->getName().str()) )
		{
			if (whoData->OnNetwork || !whoData->Name.str().compare(ch->getName().str()) )
			{
				ch_printf(ch, "| %s%s", whoData->Name.c_str(), whoData->Title.c_str());
				if (whoData->Clan.length() > 0)
				{
					set_char_color(whoData->ClanColor, ch);
					ch_printf(ch, " %s", whoData->Clan.c_str());
				}
				set_char_color(AT_PLAIN, ch);
				if (whoData->Council.length() > 0)
				{
					set_char_color(whoData->CouncilColor, ch);
					ch_printf(ch, " %s", whoData->Council.c_str());
				}
				if ( whoData->ExtraData.length() > 0)
				{
					set_char_color(AT_PLAIN, ch);
					ch_printf(ch, " %s", whoData->ExtraData.c_str());
				}
				set_char_color(AT_PLAIN, ch);
			}
			else
			{
				ch_printf(ch, "| - - -");
			}
		}
		else
		{
			set_char_color(AT_PLAIN, ch);
			send_to_char("|", ch);
		}
		send_to_char("\r\n", ch);
	}
	if (Mortals.empty() == false)
		ch_printf(ch, "+----------------+\r\n");

	// Now, free all the memory we allocated.
	for_each(Immortals.begin(), Immortals.end(), fo_DeleteObject());
	for_each(Avatars.begin(), Avatars.end(), fo_DeleteObject());
	for_each(Mortals.begin(), Mortals.end(), fo_DeleteObject());
}

void do_who(CHAR_DATA*ch, const char* argument)
{
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   char buf3[MAX_STRING_LENGTH];
   char buf4[MAX_STRING_LENGTH];
   char buf5[MAX_STRING_LENGTH];
   char buf6[MAX_STRING_LENGTH];
   char clan_name[MAX_INPUT_LENGTH];
   char check_name[MAX_INPUT_LENGTH];
   char council_name[MAX_INPUT_LENGTH];
   char invis_str[MAX_INPUT_LENGTH];
   char char_name[MAX_INPUT_LENGTH];
   char extra_title[MAX_INPUT_LENGTH];
   char class_text[MAX_INPUT_LENGTH];
   char www_file[MAX_INPUT_LENGTH];

   int iClass, iRace;
   int iLevelLower;
   int iLevelUpper;
   int nNumber;
   int nMatch;
   int tMatch;
   int class_color;
   int name_color;
   int clan_color;
   int loop;

   bool argok;
   bool rgfClass[MAX_CLASS];
   bool rgfRace[MAX_RACE];
   bool fClassRestrict;
   bool fRaceRestrict;
   bool fImmortalOnly;
   bool fPkill;
   bool fShowHomepage;
   bool fClanMatch; /* SB who clan (order),who guild, and who council */
   bool fCouncilMatch;
   bool fDeityMatch;
   bool fNameMatch;

   ClanData *pClan;
   CouncilData *pCouncil;
   DeityData *pDeity;
   FILE *whoout;

   WHO_DATA *cur_who = NULL;
   WHO_DATA *next_who = NULL;
   WHO_DATA *first_mortal = NULL;
   WHO_DATA *first_imm = NULL;
   WHO_DATA *first_av = NULL;
// WHO_DATA *first_deadly  = NULL;

/* This was from Strings...
   char * godranks[] =
      {
         "Grander Poohbah","Grand Poohbah","Poohbah","Little Poohbah","Infinite","Immense",
         "Greater God","God","Lesser God","Artificer","Tinkerer","Creator", "Engineer",
         "Wanderer","Maggot","Avatar"
      };
*/
   const char * godranks[] = {
       "Lord of the Darkstone", "Stone Master", "Stone Adept", "Stone Initiate",
       "Stone Seeker", "Artificer", "Tinkerer", "Creator", "Engineer",
       "Wanderer","Maggot"
   };

   const char *avranks[4][5] = {
    {""}, {"Avatar", "Legend", "Titan", "Champion", "Hero"}  /* ETHOS_HERO */
   ,{"Nemesis", "Diabolical","Fiend","Scoundrel","Villain"} /* ETHOS_VILLAIN */
   ,{"Infallible","Judicious","Savant","Wise","Sage"} /* ETHOS_SAGE */
   };


   // White,green, red,cyan,yellow,grey
   const char * html_color[] = {"FFFFFF","00FF00","FF0000","00FFFF","FFFF00","C6C6C6"};

   /*
    * Set default arguments.
    */
   iLevelLower    = 0;
   iLevelUpper    = MAX_LEVEL;
   fClassRestrict = FALSE;
   fRaceRestrict  = FALSE;
   fImmortalOnly  = FALSE;
   fPkill         = FALSE;
   fShowHomepage  = FALSE;
   fClanMatch   = FALSE; /* SB who clan (order), who guild, who council */
   fCouncilMatch  = FALSE;
   fDeityMatch    = FALSE;
   fNameMatch     = FALSE;

   for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
     rgfClass[iClass] = FALSE;

   for ( iRace = 0; iRace < MAX_RACE; iRace++ )
     rgfRace[iRace] = FALSE;

   /*
    * Parse arguments.
    */
   nNumber = 0;
   for ( ;; ) {
      char arg[MAX_STRING_LENGTH];

      argument = one_argument( argument, arg );
      if ( arg[0] == '\0' )
        break;

      if ( is_number( arg ) ) {
         switch ( ++nNumber ) {
          case 1: iLevelLower = atoi( arg ); break;
          case 2: iLevelUpper = atoi( arg ); break;
          default:
            send_to_char( "Only two level numbers allowed.\r\n", ch );
            return;
         }
      }
      else if ( !str_prefix( arg, "name" )) {
         fNameMatch = TRUE;
         argument = one_argument( argument, check_name );

      }
      else {
         if ( strlen(arg) < 3 ) {
            send_to_char( "Classes must be longer than that.\r\n", ch );
            return;
         }

         /*
          * Look for classes to turn on.
          */
         if ( !str_prefix( arg, "deadly" ) || !str_prefix( arg, "pkill" ) )
           fPkill = TRUE;
         else if ( !str_prefix( arg, "imm" ) || !str_prefix( arg, "gods" ) )
           fImmortalOnly = TRUE;
         else if ( !str_cmp( arg, "www" ) && (!ch) )
           {
         argument = one_argument(argument, www_file);
         fShowHomepage = TRUE;
           }
         else if  ( ( pClan = get_clan (arg) ) )
           fClanMatch = TRUE;
         else if ( ( pCouncil = get_council (arg) ) )
           fCouncilMatch = TRUE;
         else if ( ( pDeity = get_deity (arg) ) )
           fDeityMatch = TRUE;
         else {
            for ( iClass = 0; iClass < MAX_CLASS; iClass++ ) {
               if ( !str_cmp( arg, class_table[iClass]->whoName_.c_str() ) ) {
                  rgfClass[iClass] = TRUE;
                  break;
               }
            }
            if ( iClass != MAX_CLASS )
              fClassRestrict = TRUE;

            for ( iRace = 0; iRace < MAX_RACE; iRace++ ) {
               if ( !str_cmp( arg, race_table[iRace].race_name ) ) {
                  rgfRace[iRace] = TRUE;
                  break;
               }
            }
            if ( iRace != MAX_RACE )
              fRaceRestrict = TRUE;

            if ( iClass == MAX_CLASS && iRace == MAX_RACE
                && fClanMatch == FALSE
                && fCouncilMatch == FALSE
                && fDeityMatch == FALSE ) {
               send_to_char( "That's not a class, race, order, guild,"
                            " council or deity.\r\n", ch );
               return;
            }
         }
      }
   }

   /*
    * Now find matching chars.
    */
   nMatch = 0;
   tMatch = 0;
   buf[0] = '\0';
   if ( ch )
     set_pager_color( AT_GREY, ch );
   else {
      if ( fShowHomepage ) {
         whoout = fopen( www_file, "w" );
      }
      else
        whoout = fopen( WHO_FILE, "w" );
   }

   /* start from last to first to get it in the proper order */
   ritorSocketId itor;
   for ( itor = gTheWorld->GetBeginReverseConnection(); itor != gTheWorld->GetEndReverseConnection(); itor++ )
   {
	   // we know that the world's player connection list only holds player connections IDs,
	   // so we can safely cast it to PlayerConnection*
	   PlayerConnection * d = (PlayerConnection *) SocketMap[*itor];
	   CHAR_DATA *wch;
	   const char *_class;

	   tMatch++;

	   if ( (d->ConnectedState != CON_PLAYING && d->ConnectedState != CON_EDITING)
			   ||   !can_who( ch, d->GetOriginalCharacter() ) )
		   continue;

	   wch   = d->GetOriginalCharacter();

	   if ( IS_IMMORTAL(ch) ) {
		   if ( wch->level < iLevelLower
				   ||   wch->level > iLevelUpper
				   || ( fClassRestrict && !rgfClass[wch->Class] )
				   || ( fRaceRestrict && !rgfRace[wch->race] )
				   || ( fClanMatch && ( pClan != wch->pcdata->clan ))  /* SB */
				   || ( fCouncilMatch && ( pCouncil != wch->pcdata->council )) /* SB */
				   || ( fDeityMatch && ( pDeity != wch->pcdata->deity )) ) {
			   continue;
		   }
	   }

	   if ( ( fImmortalOnly  && wch->level < LEVEL_IMMORTAL )
			   || (  fNameMatch &&  ( !nifty_is_name_prefix( check_name, wch->getName().c_str() ) ) ) )
		   continue;

	   nMatch++;

	   if ( fShowHomepage
			   &&   wch->pcdata->homepage
			   &&   wch->pcdata->homepage[0] != '\0' ) {
		   sprintf( char_name, "<A HREF=\"%s\">%s</A>",
				   show_tilde( wch->pcdata->homepage ), wch->getName().c_str() );
	   }
	   else
		   strcpy( char_name, wch->getName().c_str() );

	   class_color = AT_GREY;

	   if(IS_IMMORTAL(ch)) {
		   sprintf(class_text, "%s %d",
				   capitalize(npc_class[wch->Class]),
				   wch->level);
	   }
	   else
		   strcpy(class_text, "(mortal)");

	   _class = class_text;

	   sprintf(buf2, "%s", (wch->GetConnection() && wch->GetConnection()->ConnectedState == CON_EDITING)
			   ? " (Writing)" : "");

	   name_color = AT_PLAIN;

	   switch ( wch->level ) {
		   default: break;
		   case MAX_LEVEL -  0: _class = godranks[0]; class_color = AT_WHITE;break;
		   case MAX_LEVEL -  1: _class = godranks[1]; class_color = AT_BLUE; break;
		   case MAX_LEVEL -  2: _class = godranks[2]; class_color = AT_YELLOW;break;
		   case MAX_LEVEL -  3: _class = godranks[3]; class_color = AT_YELLOW;break;
		   case MAX_LEVEL -  4: _class = godranks[4]; class_color = AT_GREEN;break;
		   case MAX_LEVEL -  5: _class = godranks[5]; class_color = AT_GREEN;break;
		   case MAX_LEVEL -  6: _class = godranks[6]; class_color = AT_GREEN;break;
		   case MAX_LEVEL -  7: _class = godranks[7]; class_color = AT_GREEN;break;
		   case MAX_LEVEL -  8: _class = godranks[8]; class_color = AT_LBLUE;break;
		   case MAX_LEVEL -  9: _class = godranks[9]; class_color = AT_LBLUE;break;

		   case MAX_LEVEL - 10: _class = avranks[wch->pcdata->ethos][0];
								class_color = AT_PURPLE;
								break;
		   case MAX_LEVEL - 11: _class = avranks[wch->pcdata->ethos][1];
								class_color = AT_WHITE;
								break;
		   case MAX_LEVEL - 12: _class = avranks[wch->pcdata->ethos][2];
								class_color = AT_LBLUE;
								break;
		   case MAX_LEVEL - 13: _class = avranks[wch->pcdata->ethos][3];
								class_color = AT_PURPLE;
								break;
		   case MAX_LEVEL - 14: _class = avranks[wch->pcdata->ethos][4];
								class_color = AT_PINK;
								break;
	   }

	   if ( wch->getName().ciEqual( sysdata.guild_overseer ) )
		   strcpy( extra_title, "[Overseer of Guilds] ");
	   else if ( wch->getName().ciEqual( sysdata.guild_advisor ) )
		   strcpy( extra_title, "[Advisor to Guilds] ");
	   else
		   extra_title[0] = '\0';

	   if ( IS_RETIRED( wch ) )
		   _class = "Retired";
	   else if ( IS_GUEST( wch ) )
		   _class = "Guest";
	   else if ( wch->pcdata->rank_.length() > 0 ) {
		   /* need to check rank class against god ranks so as not to fool the players */
		   argok = TRUE;
		   for ( loop = 0; loop < 10; loop++ ) {
			   if ( wch->pcdata->rank_.ciEqual(godranks[loop]) && MAX_LEVEL - wch->level > loop)
				   argok = FALSE;
		   }
		   if (argok)
			   _class = wch->pcdata->rank_.c_str();
		   else
			   _class = "Maggot";
	   }


	   if ( wch->pcdata->clan ) {
		   ClanData *pclan = wch->pcdata->clan;
		   if ( pclan->clan_type == CLAN_GUILD ) {
			   if(!ch)
				   strcpy( clan_name, "\r\n<BR>&lt;" );
			   else
				   strcpy( clan_name, " <" );
		   }
		   else {
			   if(!ch)
				   strcpy( clan_name, "\r\n<BR>(" );
			   else
				   strcpy( clan_name, " (" );
		   }

		   if ( pclan->clan_type == CLAN_ORDER ) {
			   if ( wch->getName().ciEqual( pclan->deity_ ) )
				   strcat( clan_name, "Deity, Order of " );
			   else
				   if ( wch->getName().ciEqual( pclan->leader_ ) )
					   strcat( clan_name, "Leader, Order of " );
				   else
					   if ( wch->getName().ciEqual( pclan->number1_ ) )
						   strcat( clan_name, "Number One, Order of " );
					   else
						   if ( wch->getName().ciEqual( pclan->number2_ ) )
							   strcat( clan_name, "Number Two, Order of " );
						   else
							   strcat( clan_name, "Order of " );
		   }
		   else
			   if ( pclan->clan_type == CLAN_GUILD ) {
				   if ( wch->getName().ciEqual( pclan->leader_ ) )
					   strcat( clan_name, "Leader, " );
				   if ( wch->getName().ciEqual( pclan->number1_ ) )
					   strcat( clan_name, "First, " );
				   if ( wch->getName().ciEqual( pclan->number2_ ) )
					   strcat( clan_name, "Second, " );
			   }
			   else {
				   if ( wch->getName().ciEqual( pclan->deity_ ) )
					   strcat( clan_name, "Deity of " );
				   else
					   if ( wch->getName().ciEqual( pclan->leader_ ) )
						   strcat( clan_name, "Leader of " );
					   else
						   if ( wch->getName().ciEqual( pclan->number1_ ) )
							   strcat( clan_name, "Number One " );
						   else
							   if ( wch->getName().ciEqual( pclan->number2_ ) )
								   strcat( clan_name, "Number Two " );
			   }
		   strcat( clan_name, pclan->name_.c_str() );
		   clan_color = pclan->color;
		   if ( pclan->clan_type == CLAN_GUILD ) {
			   if (!ch)
				   strcat( clan_name, "&gt;" );
			   else
				   strcat( clan_name, ">" );
		   }
		   else
			   strcat( clan_name, ")" );

		   if (!wch->pcdata->council)
			   strcat( clan_name, "" );
	   }
	   else
		   clan_name[0] = '\0';

	   if ( wch->pcdata->council ) {

		   if ( !wch->pcdata->clan ) {
			   if (!ch)
				   strcpy( council_name, "\r\n<BR>[" );
			   else
				   strcpy( council_name, " [" );
		   }
		   else
			   strcpy( council_name, " [" );

		   if ( wch->getName().ciEqual( wch->pcdata->council->head_ ) )
			   strcat( council_name, "Head of " );
		   strcat( council_name, wch->pcdata->councilName_.c_str() );
		   strcat( council_name, "]" );
	   }
	   else
		   council_name[0] = '\0';

	   if ( IS_SET(wch->act, PLR_WIZINVIS) )
		   sprintf( invis_str, "(I:%d)", wch->pcdata->wizinvis );
	   else
		   invis_str[0] = '\0';

	   if (!ch)
		   sprintf( buf, "%s", _class);
	   else
	   {
		   int count;
		   int width = 16;
		   int len = 0;
		   const char *ptr = _class;

		   while ( *ptr ) {
			   if ( *ptr != '&' && *ptr != '^' ) {
				   len++;
			   } else if ( *ptr == '&' && *ptr+1 == '&' ) {
				   len++;
				   ptr++;
			   } else if ( *ptr == '^' && *ptr+1 == '^' ) {
				   len++;
				   ptr++;
			   } else {
				   ptr++;
			   }
			   ptr++;
		   }

		   width -= len;
		   width /= 2;
		   strcpy(buf, "\0");
		   for ( count = 0; count < width; count++ )
			   strcat(buf, " ");
		   strcat(buf, _class);
		   for ( count = 0; count < width; count++ )
			   strcat(buf, " ");

		   for ( count = (width*2)+len; count < 16; count++ )
			   strcat(buf, " ");


	   }

	   sprintf( buf3, "%s%s ",char_name, player_title(wch->pcdata->title_.c_str()));

	   sprintf( buf4, "%s%s\r\n", IS_SET(wch->act, PLR_AFK) ? "[AFK] " : "",invis_str);

	   /* buf5 added by Warp to allow extra coloring for stuff after title */
	   sprintf( buf5, "%s%s", extra_title, clan_name);
	   sprintf( buf6, "%s", council_name );

	   /*
		* This is where the old code would display the found player to the ch.
		* What we do instead is put the found data into a linked list
		*/

	   /* First make the structure. */
	   CREATE( cur_who, WHO_DATA, 1 );
	   cur_who->text = str_dup( buf );
	   cur_who->text2 = str_dup( buf2 );
	   cur_who->text3 = str_dup( buf3 );
	   cur_who->text4 = str_dup( buf4 );
	   cur_who->text5 = str_dup( buf5 );
	   cur_who->clan_color = clan_color;
	   cur_who->color = class_color;
	   cur_who->color2 = name_color;
	   cur_who->council_color = AT_LBLUE;
	   cur_who->text6 = str_dup( buf6 );

	   /* Then put it into the appropriate list. */
	   if ( IS_AVATAR(wch) ) {
		   cur_who->next = first_av;
		   first_av = cur_who;
	   } else if ( IS_IMMORTAL( wch ) ) {
		   cur_who->next = first_imm;
		   first_imm = cur_who;
	   }
	   else {
		   cur_who->next = first_mortal;
		   first_mortal = cur_who;
	   }
   }


   /* Ok, now we have three separate linked lists and what remains is to
    * display the information and clean up.
    */

   {
	   if ( !ch ) {
#ifdef WHO_MAKE_HTML
		   morefile = FALSE;
		   keepreading = TRUE;
		   num = 0;
		   while (keepreading) {
			   while ((htmlbuf[num]=fgetc(whohead)) != EOF) {
				   num++;
				   if ( (num + 1) > MAX_STRING_LENGTH) {
					   if ( htmlbuf[num - 1] != EOF )
						   morefile = TRUE;
					   break;
				   }
			   }
			   if (!morefile) {
				   fclose(whohead);
				   keepreading = FALSE;
			   }

			   htmlbuf[num] = 0;

			   fprintf( whoout, htmlbuf);

			   if (morefile) {
				   num = 0;
				   morefile = FALSE;
			   }
		   }
#endif
		   fprintf( whoout,"<TR><TD COLSPAN=3><CENTER>This page last updated at C.S.T.: %s</CENTER><P><P></TD></TR>\r\n" , (char *) ctime( &secCurrentTime));

		   //fprintf( whoout, "\r\n\r\n--------------------------------[ PLAYERS ONLINE ]---------------------------\r\n\r\n" );
		   fprintf( whoout, "\r\n\r\n\r\n\r\n" );
		   fprintf( whoout, "\r\n\r\n<TR><TD COLSPAN=3 BGCOLOR=\"#A2F9A8\"><FONT COLOR=\"#000000\"><CENTER>[ PLAYERS ONLINE ]</CENTER><FONT COLOR=\"#FFFFFF\"></TD><TR>\r\n\r\n" );
		   fprintf( whoout, "\r\n\r\n" );
		   if (tMatch < 1)
			   fprintf( whoout, "<TR><TD><CENTER><H2>There are no players currently online</TD></TR><P><P>\r\n\r\n" );
	   }
	   else
	   {
		   set_pager_color(AT_WHITE, ch);
		   send_to_pager( "----[ RANK ]----+-[ NAME ]--------[ TITLE ]------------------------------\r\n", ch);
		   //send_to_pager( "\r\n\r\n--------------------------------[ PLAYERS ONLINE ]----------------------------\r\n\r\n", ch );
	   }
   }

   for ( cur_who = first_imm; cur_who; cur_who = next_who ) {
      if ( !ch ) {
         fprintf( whoout, "<TR><TD><FONT COLOR=\"#");
         switch (cur_who->color) {
          default:
          case AT_WHITE:
            fprintf( whoout, "%s\">" ,html_color[0]);
            break;
          case AT_GREEN:
            fprintf( whoout, "%s\">" ,html_color[1]);
            break;
          case AT_RED:
            fprintf( whoout, "%s\">" ,html_color[2]);
            break;
          case AT_LBLUE:
            fprintf( whoout, "%s\">" ,html_color[3]);
            break;
          case AT_YELLOW:
            fprintf( whoout, "%s\">" ,html_color[4]);
            break;
         }
         fprintf( whoout, "%s", cur_who->text );
         fprintf( whoout, "%s", "<FONT COLOR=\"#FFFFFF\"></TD><TD><FONT COLOR=\"#FF0000\">" );
         fprintf( whoout, "<FONT COLOR=\"#FFFFFF\"></TD><TD><FONT COLOR=\"#%s\">",(cur_who->color2 == AT_RED)  ? html_color[2]  : html_color[0]);
         fprintf( whoout, "%s", cur_who->text3 );
         fprintf( whoout, "%s", "<FONT COLOR=\"#C6C6C6\">");
         fprintf( whoout, "%s", cur_who->text4 );
         fprintf( whoout, "%s", "<FONT COLOR=\"#FFFFFF\"></TD></TR>\r\n" );
      }
      else {
         set_pager_color( cur_who->color, ch );
         send_to_pager( cur_who->text, ch );
         set_pager_color( AT_WHITE, ch );
         send_to_pager( "| ", ch );
         set_pager_color( cur_who->color2, ch );
         send_to_pager( cur_who->text3, ch );
         set_pager_color( AT_PLAIN, ch);
         send_to_pager( cur_who->text2, ch );
         /*
		 Commented out by Ksilyan to account for clan
		 colors and stuff.
		 set_pager_color( AT_BLUE, ch );
         */
		 set_pager_color( cur_who->clan_color, ch);
		 send_to_pager( cur_who->text5, ch );
		 set_pager_color( cur_who->council_color, ch);
		 send_to_pager( cur_who->text6, ch);
         set_pager_color( AT_PLAIN, ch );
         send_to_pager( cur_who->text4, ch );
      }
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      DISPOSE( cur_who->text3 );
      DISPOSE( cur_who->text4 );
      DISPOSE( cur_who->text5 );
      DISPOSE( cur_who->text6 );
      DISPOSE( cur_who->text2 );
      DISPOSE( cur_who );
   }

   /* An empty line between imms, avs and morts, just for Saruman
    * and Tarin :)   -- Warp
    */

   if(first_imm && (first_av || first_mortal))
     ch_printf( ch, "\r\n");

   for ( cur_who = first_av; cur_who; cur_who = next_who ) {
      if ( !ch ) {
         fprintf( whoout, "<TR><TD><FONT COLOR=\"#");
         switch (cur_who->color) {
          default:
          case AT_WHITE:
            fprintf( whoout, "%s\">" ,html_color[0]);
            break;
          case AT_GREEN:
            fprintf( whoout, "%s\">" ,html_color[1]);
            break;
          case AT_RED:
            fprintf( whoout, "%s\">" ,html_color[2]);
            break;
          case AT_LBLUE:
            fprintf( whoout, "%s\">" ,html_color[3]);
            break;
          case AT_YELLOW:
            fprintf( whoout, "%s\">" ,html_color[4]);
            break;
         }
         fprintf( whoout, "%s", cur_who->text );
         fprintf( whoout, "%s", "<FONT COLOR=\"#FFFFFF\"></TD><TD><FONT COLOR=\"#FF0000\">" );
         fprintf( whoout, "<FONT COLOR=\"#FFFFFF\"></TD><TD><FONT COLOR=\"#%s\">",(cur_who->color2 == AT_RED)  ? html_color[2]  : html_color[0]);
         fprintf( whoout, "%s", cur_who->text3 );
         fprintf( whoout, "%s", "<FONT COLOR=\"#C6C6C6\">");
         fprintf( whoout, "%s", cur_who->text4 );
         fprintf( whoout, "%s", "<FONT COLOR=\"#FFFFFF\"></TD></TR>\r\n" );
      }
      else {
         set_pager_color( cur_who->color, ch );
         send_to_pager( cur_who->text, ch );
         set_pager_color( AT_WHITE, ch );
         send_to_pager( "| ", ch );
         set_pager_color( cur_who->color2, ch );
         send_to_pager( cur_who->text3, ch );
         set_pager_color( AT_PLAIN, ch);
         send_to_pager( cur_who->text2, ch );
         //set_pager_color( AT_BLUE, ch );
		 set_pager_color( cur_who->clan_color, ch);
         send_to_pager( cur_who->text5, ch );
		 set_pager_color( cur_who->council_color, ch);
		 send_to_pager( cur_who->text6, ch);
         set_pager_color( AT_PLAIN, ch );
         send_to_pager( cur_who->text4, ch );
      }
      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      DISPOSE( cur_who->text3 );
      DISPOSE( cur_who->text4 );
      DISPOSE( cur_who->text5 );
      DISPOSE( cur_who->text6 );
      DISPOSE( cur_who->text2 );
      DISPOSE( cur_who );
   }

   /* An empty line between imms, avs and morts, just for Saruman
    * and Tarin :)   -- Warp
    */

   if(first_av && first_mortal)
     ch_printf( ch, "\r\n");

   for ( cur_who = first_mortal; cur_who; cur_who = next_who ) {
      if ( !ch ) {
         fprintf( whoout, "<TR><TD><FONT COLOR=\"#%s\">", (cur_who->color == AT_RED)  ? html_color[2]  : html_color[1]);
         fprintf( whoout, "%s", cur_who->text );
         fprintf( whoout, "%s", "<FONT COLOR=\"#FFFFFF\"></TD><TD><FONT COLOR=\"#FF0000\">");
         fprintf( whoout, "%s", cur_who->text2 );
         fprintf( whoout, "<FONT COLOR=\"#FFFFFF\"></TD><TD><FONT COLOR=\"#%s\">",(cur_who->color2 == AT_RED)  ? html_color[2]  : html_color[1]);
         fprintf( whoout, "%s", cur_who->text3 );
         fprintf( whoout, "%s", "<FONT COLOR=\"#C6C6C6\">");
         fprintf( whoout, "%s", cur_who->text4 ); /* The AFK text */
         fprintf( whoout, "%s", "<FONT COLOR=\"#FFFFFF\"></TD></TR>\r\n" );
      }
      else {
         set_pager_color( cur_who->color, ch );
         send_to_pager( cur_who->text, ch );
         set_pager_color( AT_WHITE, ch );
         send_to_pager( "| ", ch );
         set_pager_color( cur_who->color2, ch );
         send_to_pager( cur_who->text3, ch );
         set_pager_color( AT_PLAIN, ch );
         send_to_pager( cur_who->text2, ch );
         //set_pager_color( AT_BLUE, ch );
		 set_pager_color( cur_who->clan_color, ch);
         send_to_pager( cur_who->text5, ch );
		 set_pager_color( cur_who->council_color, ch);
		 send_to_pager( cur_who->text6, ch);
         set_pager_color( AT_PLAIN, ch );
         send_to_pager( cur_who->text4, ch );
      }

      next_who = cur_who->next;
      DISPOSE( cur_who->text );
      DISPOSE( cur_who->text3 );
      DISPOSE( cur_who->text4 );
      DISPOSE( cur_who->text5 );
      DISPOSE( cur_who->text6 );
      DISPOSE( cur_who->text2 );
      DISPOSE( cur_who );
   }

   if ( !ch ) {
      //if (nMatch < tMatch)
      //fprintf( whoout, "<TR><TD>%d Visible Player%s.</TD></TR>\r\n", nMatch, nMatch == 1 ? "" : "s" );
      //if (tMatch)
      //fprintf( whoout, "<TR><TD>%d Total player%s.</TS></TR>\r\n", tMatch, tMatch == 1 ? "" : "s" );
      fprintf( whoout, "<TR><TD>%d Total Player%s.</TD></TR>\r\n", nMatch, nMatch == 1 ? "" : "s" );
#ifdef WHO_MAKE_HTML
      morefile = FALSE;
      keepreading = TRUE;
      num = 0;
      while (keepreading) {
         while ((htmlbuf[num]=fgetc(whofoot)) != EOF) {
            num++;
            if ( (num + 1) > MAX_STRING_LENGTH) {
               if ( htmlbuf[num - 1] != EOF )
                 morefile = TRUE;
               break;
            }
         }
         if (!morefile) {
            fclose(whofoot);
            keepreading = FALSE;
         }

         htmlbuf[num] = 0;

         fprintf( whoout, htmlbuf);

         if (morefile) {
            num = 0;
            morefile = FALSE;
         }
      }
#endif

      fclose( whoout );
      return;
   }

   set_char_color( AT_YELLOW, ch );

    ch_printf( ch, "\r\n%d Total Player%s.\r\n", nMatch, nMatch == 1 ? "" : "s" );

   set_char_color( AT_PLAIN, ch );

   return;
}


int can_who(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if( can_see(ch, victim) )
		return true;
	else
		return false;
}

/* Find non-existant help files. -Sadiq */
void do_nohelps(CHAR_DATA *ch, const char *argument)
{
  CMDTYPE *command;
  AREA_DATA *tArea;
  char arg[MAX_STRING_LENGTH];
  int hash, col=0, sn=0;

   argument = one_argument( argument, arg );

   if(!IS_IMMORTAL(ch) || IS_NPC(ch) )
    {
	send_to_char("Huh?\n\r", ch);
	return;
    }

   if ( arg[0] == '\0' || !str_cmp(arg, "all") )
   {
      do_nohelps(ch, "commands");
      do_nohelps(ch, "skills");
      do_nohelps(ch, "areas");
      return;
   }

  if(!str_cmp(arg, "commands") )
  {
    send_to_char_color("\r\n&C&YCommands for which there are no help files:\n\r\n\r", ch);

    for ( hash = 0; hash < 126; hash++ )
     {
       for ( command = command_hash[hash]; command; command = command->next )
	{
	      	if(!get_help2(ch, command->name) )
	      	{
	      set_char_color( AT_WHITE, ch );
	   	  ch_printf(ch, "%-15s", command->name);
	   	  if ( ++col % 5 == 0 )
	   	      {
			send_to_char( "\n\r", ch );
		      }
	      	}
	}
      }

	send_to_char("\n\r", ch);
	return;
   }

   if(!str_cmp(arg, "skills") || !str_cmp(arg, "spells") )
   {
     send_to_char_color("\r\n&CSkills/Spells for which there are no help files:\n\r\n\r", ch);

     for ( sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name_.c_str(); sn++ )
     {
   	if(!get_help2(ch, skill_table[sn]->name_.c_str()))
   	{
	   set_char_color( AT_WHITE, ch );
   	   ch_printf(ch, "%-20s", skill_table[sn]->name_.c_str());
   	   if ( ++col % 4 == 0 )
   	     {
   	     	send_to_char("\n\r", ch);
   	     }
   	}
     }

	send_to_char("\n\r", ch);
	return;
   }

   if(!str_cmp(arg, "areas") )
   {
        send_to_char_color("\r\n&GAreas for which there are no help files:\n\r\n\r", ch);

        for (tArea = first_area; tArea;tArea = tArea->next)
        {
    	  if(!get_help2(ch, tArea->name) )
    	   {
	   		  set_char_color( AT_WHITE, ch );
    	      ch_printf(ch, "%-35s", tArea->name);
    	      if ( ++col % 2 == 0 )
    	       {
    	     	send_to_char("\n\r", ch);
    	       }
    	   }
        }

	   send_to_char( "\n\r", ch);
	   return;
    }

	send_to_char("\r\nSyntax:  nohelps <all|areas|commands|skills>\n\r", ch);
	return;
}

