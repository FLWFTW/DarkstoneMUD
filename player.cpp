/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider             |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops  |~'~.VxvxV.~'~*
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvements copyright (C) 1992, 1993 by Michael       *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 * 		Commands for personal player settings/statictics	    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "connection.h"
#include "connection_manager.h"


PlayerData::PlayerData()
{
	clan = NULL;
	council = NULL;
	area = NULL;
	deity = NULL;
	homepage = NULL;
	email = NULL;
	pwd = NULL;

	for ( int i = 0; i < MAX_KILLTRACK; i++ )
	{
		killed[i].vnum = 0;
		killed[i].count = 0;
	}

	titleLocked = false;
	flags = pkills = pdeaths = mkills = mdeaths = 0;
	illegal_pk = 0;
	secOutcastTime = secRestoreTime = 0;

	r_range_lo = r_range_hi = m_range_lo = m_range_hi = o_range_lo = o_range_hi = 0;
	wizinvis = min_snoop = 0;

	for ( int i = 0; i < MAX_CONDS; i++ )
		condition[i] = 0;
	for ( int i = 0; i < MAX_SKILL; i++ )
		learned[i] = 0;

	quest_number = quest_curr = quest_deci = quest_accum = 0;
	favor = 0;
	auth_state = 0;
	secReleaseDate = 0;
	bio = NULL;

	for ( int i = 0; i < 5; i++ )
		special_skills[i] = NULL;

	subprompt = NULL;
	pagerlen = 24;
	openedtourney = false;

	for ( int i = 0; i < MAX_PCCOLORS; i++ )
		pc_colors[i] = 0;
	last_color = 0;

	recall_room = bank_gold = 0;

	rating = 1000;
	ethos = ETHOS_HERO;

	for ( int i = 0; i < MAX_CITIES; i++ )
		citythief[i] = CityWanted[i] = 0;

	Vault = NULL;
	secVaultLastAccessed = 0;
}


/*
 *  Locals
 */
const char *tiny_affect_loc_name(int location);

void do_gold(CHAR_DATA * ch, const char* argument)
{
   set_char_color( AT_GOLD, ch );
   ch_printf( ch,  "You have %d gold pieces.\r\n", ch->gold );
   return;
}

/* New score command by Warp 26.06.00
 * Basically just reformatted the whole thing and expressed
 * number information with words where possible
 */
void do_oldscore(CHAR_DATA* ch, const char* argument);

void do_score(CHAR_DATA* ch, const char* argument)
{
	char          buf[MAX_STRING_LENGTH];
	AFFECT_DATA*  paf;
	int 	         iLang, linelen;

	if(IS_NPC(ch)) {
		do_oldscore(ch, argument);
		return;
	}

	set_char_color(AT_SCORE, ch);

	/* There seems to be a bug in capitalize(), if you call
	 * capitalize(get_race(ch)), capitalize(get_class(ch)) in the same
	 * param list, it returns the race two times! So let's kludge this.
	 *
	 * KSILYAN: actually, this isn't a bug. It's because of the static
	 * char *.
	 */
	strcpy(buf, capitalize(get_class(ch)));

        ch_printf(ch, "\r\n-- You are %s, level %d", ch->getName().c_str(), ch->level);
        if(get_trust(ch) != ch->level) ch_printf(ch, "[%d]", get_trust(ch));
        ch_printf(ch, " %s %s. You are %d years old. --\n\r\n",	capitalize(get_race(ch)), buf, get_age(ch));
	ch_printf(ch, "Str: %-12s You are %s and others see you as %s.\r\n",
			StatStr(ch, "STR", ch->getStr(), str_names), positionstr(ch), alignstr(ch));
	ch_printf(ch, "Int: %-12s Your armor is %s, and you have\r\n",
			StatStr(ch, "INT", ch->getInt(), int_names), armorstr(ch));
	ch_printf(ch, "Wis: %-12s %s hitroll and %s damage roll bonuses.\r\n",
			StatStr(ch, "WIS", ch->getWis(), wis_names), hitrollstr(ch), damrollstr(ch));
	ch_printf(ch, "Dex: %-12s You have %d experience points, %d practice\r\n",
			StatStr(ch, "DEX", ch->getDex(), dex_names), ch->exp, ch->practice);
	ch_printf(ch, "Con: %-12s sessions and %d.%d glory points left. Your purse has\r\n",
			StatStr(ch, "CON", ch->getCon(), con_names), ch->pcdata->quest_curr, ch->pcdata->quest_deci);
	ch_printf(ch, "Cha: %-12s %d gold coins in it, and you will wimp out of a\r\n",
			StatStr(ch, "CHA", ch->getCha(), cha_names), ch->gold);
	ch_printf(ch, "Lck: %-12s battle at %d or less hit points.\r\n",
			StatStr(ch, "LCK", ch->getLck(), lck_names), ch->wimpy);

	send_to_char("\r\n", ch);

	ch_printf(ch, "HP: %d/%d  Move: %d/%d", ch->hit, ch->max_hit, ch->move, ch->max_move);

	if(ch->Class != CLASS_WARRIOR)
		ch_printf(ch, "  Mana: %d/%d", ch->mana, ch->max_mana);

	if(IS_VAMPIRE(ch))
		ch_printf(ch, "  Blood: %d/%d", ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level);

	ch_printf(ch, "  Weight: %d/%d", ch->carry_weight, can_carry_w(ch));

	send_to_char("\r\n\r\n", ch);
	print_states(ch);   /* Print hungry, thirsty, drunkenes and mental state -- Warp */
	send_to_char("\r\n", ch);

	ch_printf(ch, "You have killed %d monsters and %d player characters, and you have\r\n",
			ch->pcdata->mkills, ch->pcdata->pkills);
	ch_printf(ch, "been killed %d times by monsters and %d times by player characters.\r\n",
			ch->pcdata->mdeaths, ch->pcdata->pdeaths);

	send_to_char("\r\nLanguages: ", ch);
	linelen = 11;

	for (iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++)
		if(knows_language(ch, lang_array[iLang], ch) || (IS_NPC(ch) && ch->speaks == 0)) {
			if(linelen >= 65) { /* wrap and align with prev line */
				send_to_char("\r\n", ch);
				linelen = 11;
				send_to_char("           ", ch);
			}

			if(lang_array[iLang] & ch->speaking || (IS_NPC(ch) && !ch->speaking))
				set_char_color(AT_RED, ch);

			send_to_char(lang_names[iLang], ch);
			send_to_char(" ", ch);
			linelen += strlen(lang_names[iLang]) + 1;
			set_char_color(AT_SCORE, ch);
		}

	send_to_char("\r\n", ch);

	if(ch->pcdata->clan) {
		if(ch->pcdata->clan->clan_type == CLAN_ORDER) sprintf(buf, "order");
		else if(ch->pcdata->clan->clan_type == CLAN_GUILD) sprintf(buf, "guild");
		else sprintf(buf, "clan");

		ch_printf(ch, "You belong to the %s %s. The members have killed %d\r\n",
				ch->pcdata->clan->name_.c_str(), buf, ch->pcdata->clan->pkills);
		ch_printf(ch, "players and %d members have been killed by others.\r\n",
				ch->pcdata->clan->pdeaths);
	}

	if(ch->pcdata->deity) {
		ch_printf(ch, "You worship %s, and have %d Favor points.\r\n",
				ch->pcdata->deity->name_.c_str(), ch->pcdata->favor);
	}

	if(ch->pcdata->bestowments_.length() > 0 )
		ch_printf(ch, "You are bestowed with the command(s): %s.\r\n",
				ch->pcdata->bestowments_.c_str());

	if(IS_IMMORTAL(ch) || ch->pcdata->area)
		send_to_char("----------------------------------------------------------------------------\r\n", ch);
	if(IS_IMMORTAL(ch))
	{
		ch_printf(ch, "IMMORTAL DATA:  Wizinvis [%s]  Wizlevel (%d)\r\n",
				IS_SET(ch->act, PLR_WIZINVIS) ? "X" : " ", ch->pcdata->wizinvis);
		ch_printf(ch, "Bamfin:  %s\r\n", ch->getBamfIn().c_str() );
		ch_printf(ch, "Bamfout: %s\r\n", ch->getBamfOut().c_str() );
	}
	/* Area Loaded info - Scryn 8/11, cleaned up Testaur */
	if(ch->pcdata->area)
	{
		ch_printf(ch, "Area Loaded [%s]  ",
				(IS_SET(ch->pcdata->area->status, AREA_LOADED)) ? "yes" : "no");

		ch_printf(ch, "Vnums: Room %11s",vnum_to_dotted(ch->pcdata->area->low_r_vnum));
		ch_printf(ch, " -%11s\r\n",vnum_to_dotted(ch->pcdata->area->hi_r_vnum));

		ch_printf(ch, "Mob %11s",vnum_to_dotted(ch->pcdata->area->low_r_vnum));
		ch_printf(ch, " -%11s",vnum_to_dotted(ch->pcdata->area->hi_r_vnum));

		ch_printf(ch, " Obj %11s",vnum_to_dotted(ch->pcdata->area->low_r_vnum));
		ch_printf(ch, " -%11s\r\n",vnum_to_dotted(ch->pcdata->area->hi_o_vnum));

	}

	if(ch->first_affect) {
		int i = 0;
		SkillType *sktmp;

		send_to_char("You have the following effects upon you:\r\n", ch);

		for(paf = ch->first_affect; paf; paf = paf->next) {
			if((sktmp = get_skilltype(paf->type)) == NULL) continue;

			ch_printf(ch, "%-34.34s   ", sktmp->name_.c_str());

			if((++i % 2) == 0) send_to_char("\r\n", ch);
		}
	}

	send_to_char("\r\n", ch);

	return;
}

/*
 * Return ascii name of an affect location.
 */
const char           *
tiny_affect_loc_name(int location)
{
   switch (location) {
    case APPLY_NONE:		return "NIL";
    case APPLY_STR:			return " STR  ";
    case APPLY_DEX:			return " DEX  ";
    case APPLY_INT:			return " INT  ";
    case APPLY_WIS:			return " WIS  ";
    case APPLY_CON:			return " CON  ";
    case APPLY_CHA:			return " CHA  ";
    case APPLY_LCK:			return " LCK  ";
    case APPLY_SEX:			return " SEX  ";
    case APPLY_CLASS:		return " CLASS";
    case APPLY_LEVEL:		return " LVL  ";
    case APPLY_AGE:			return " AGE  ";
    case APPLY_MANA:		return " MANA ";
    case APPLY_HIT:			return " HV   ";
    case APPLY_MOVE:		return " MOVE ";
    case APPLY_GOLD:		return " GOLD ";
    case APPLY_EXP:			return " EXP  ";
    case APPLY_AC:			return " AC   ";
    case APPLY_HITROLL:		return " HITRL";
    case APPLY_DAMROLL:		return " DAMRL";
    case APPLY_SAVING_POISON:	return "SV POI";
    case APPLY_SAVING_ROD:		return "SV ROD";
    case APPLY_SAVING_PARA:		return "SV PARA";
    case APPLY_SAVING_BREATH:	return "SV BRTH";
    case APPLY_SAVING_SPELL:	return "SV SPLL";
    case APPLY_HEIGHT:		return "HEIGHT";
    case APPLY_WEIGHT:		return "WEIGHT";
    case APPLY_AFFECT:		return "AFF BY";
    case APPLY_RESISTANT:		return "RESIST";
    case APPLY_IMMUNE:		return "IMMUNE";
    case APPLY_SUSCEPTIBLE:		return "SUSCEPT";
    case APPLY_WEAPONSPELL:		return " WEAPON";
    case APPLY_BACKSTAB:		return "BACKSTB";
    case APPLY_PICK:		return " PICK  ";
    case APPLY_TRACK:		return " TRACK ";
    case APPLY_STEAL:		return " STEAL ";
    case APPLY_SNEAK:		return " SNEAK ";
    case APPLY_HIDE:		return " HIDE  ";
    case APPLY_PALM:		return " PALM  ";
    case APPLY_DETRAP:		return " DETRAP";
    case APPLY_DODGE:		return " DODGE ";
    case APPLY_PEEK:		return " PEEK  ";
    case APPLY_SCAN:		return " SCAN  ";
    case APPLY_GOUGE:		return " GOUGE ";
    case APPLY_SEARCH:		return " SEARCH";
    case APPLY_MOUNT:		return " MOUNT ";
    case APPLY_DISARM:		return " DISARM";
    case APPLY_KICK:		return " KICK  ";
    case APPLY_PARRY:		return " PARRY ";
    case APPLY_BASH:		return " BASH  ";
    case APPLY_STUN:		return " STUN  ";
    case APPLY_PUNCH:		return " PUNCH ";
    case APPLY_CLIMB:		return " CLIMB ";
    case APPLY_GRIP:		return " GRIP  ";
    case APPLY_SCRIBE:		return " SCRIBE";
    case APPLY_BREW:		return " BREW  ";
    case APPLY_WEARSPELL:		return " WEAR  ";
    case APPLY_REMOVESPELL:		return " REMOVE";
    case APPLY_EMOTION:		return "EMOTION";
    case APPLY_MENTALSTATE:		return " MENTAL";
    case APPLY_STRIPSN:		return " DISPEL";
    case APPLY_REMOVE:		return " REMOVE";
    case APPLY_DIG:			return " DIG   ";
    case APPLY_FULL:		return " HUNGER";
    case APPLY_THIRST:		return " THIRST";
    case APPLY_DRUNK:		return " DRUNK ";
    case APPLY_BLOOD:		return " BLOOD ";
    case APPLY_CONTAGION:       return " CONTAG ";
   }

   bug("tiny_affect_loc_name: unknown location %d.", location);
   return "(\?\?\?)";
}

const char* get_class(CHAR_DATA *ch)
{
   if(ch->Class < MAX_NPC_CLASS && ch->Class >= 0)
     return (npc_class[ch->Class]);

   return ("Unknown");
}


const char* get_race(CHAR_DATA *ch)
{
   if(ch->race < MAX_NPC_RACE && ch->race >= 0)
     return (npc_race[ch->race]);
   return ("Unknown");
}

// Utility functions for new score
#define LEVEL_PROVEN 3
static char unproven[] = "Unproven";

const char * StatStr(Character * ch, const char * statName, int stat, const char * const* names)
{
   if( ch->level < LEVEL_PROVEN ) return unproven;

   if(stat >= 1 && stat <= 5) return names[0];
   if(stat >= 6 && stat <= 8) return names[1];
   if(stat >= 9 && stat <= 11) return names[2];
   if(stat >= 12 && stat <= 14) return names[3];
   if(stat >= 15 && stat <= 17) return names[4];
   if(stat >= 18 && stat <= 19) return names[5];
   if(stat >= 20 && stat <= 21) return names[6];
   if(stat >= 22 && stat <= 23) return names[7];
   if(stat >= 24 && stat <= 25) return names[8];

   bug("StatStr: Incorrect %s (%d) for %s", statName, stat, ch->getName().c_str() );
   return ("<err>");

}

const char* armorstr(CHAR_DATA *ch)
{
   int ac = ch->getAC();

   if(ac >= 101)
     return "the rags of a beggar";
   else if(ac >= 80)
     return "improper for adventure";
   else if(ac >= 55)
     return "shabby and threadbare";
   else if(ac >= 40)
     return "of poor quality";
   else if(ac >= 20)
     return "scant protection";
   else if(ac >= 10)
     return "that of a knave";
   else if(ac >= 0)
     return "moderately crafted";
   else if(ac >= -10)
     return "well crafted";
   else if(ac >= -20)
     return "the envy of squires";
   else if(ac >= -40)
     return "excellently crafted";
   else if(ac >= -60)
     return "the envy of knights";
   else if(ac >= -80)
     return "the envy of barons";
   else if(ac >= -100)
     return "the envy of dukes";
   else if(ac >= -200)
     return "the envy of emperors";
   else
     return "that of an avatar";
}

const char* positionstr(CHAR_DATA* ch)
{
    switch(ch->position)
    {
        case POS_DEAD:		 return "slowly decomposing";
        case POS_MORTAL:	 return "mortally wounded";
        case POS_INCAP: 	 return "incapacitated";
        case POS_STUNNED:	 return "stunned";
        case POS_SLEEPING:	 return "sleeping";
        case POS_RESTING:	 return "resting";
        case POS_STANDING:	 return "standing";
        case POS_FIGHTING:	 return "fighting";
        case POS_MOUNTED:	 return "mounted";
        case POS_SITTING:	 return "sitting";
        case POS_MEDITATING: return "meditating";
        default:
                             bug("positionstr(): Invalid position on %s", ch->getName().c_str() );
                             return "<err>";
    }
}

const char* alignstr(CHAR_DATA* ch)
{
   if(ch->alignment > 900)
     return "devout";
   else if(ch->alignment > 700)
     return "noble";
   else if(ch->alignment > 350)
     return "honorable";
   else if(ch->alignment > 100)
     return "worthy";
   else if(ch->alignment > -100)
     return "neutral";
   else if(ch->alignment > -350)
     return "base";
   else if(ch->alignment > -700)
     return "evil";
   else if(ch->alignment > -900)
     return "ignoble";
   else
     return "fiendish";
}

const char* hitrollstr(CHAR_DATA *ch)
{
	int temp_hr = ch->getHitRoll();

	if(temp_hr < -3)	   return "horrible";
	else if(temp_hr == -2) return "terrible";
	else if(temp_hr == -1) return "bad";
	else if(temp_hr == 0)  return "no";
	else if(temp_hr == 1 || temp_hr == 2)	 return "small";
	else if(temp_hr == 3 || temp_hr == 4)	 return "some";
	else if(temp_hr == 5 || temp_hr == 6)	 return "good";
	else if(temp_hr == 7 || temp_hr == 8)	 return "very good";
	else if(temp_hr == 9 || temp_hr == 10)	 return "great";
	else if(temp_hr == 11 || temp_hr == 12)  return "unbelievable";
	else if(temp_hr == 13 || temp_hr == 14)  return "legendary";

	return "godly";
}

const char* damrollstr(CHAR_DATA *ch)
{
   int temp_dr = ch->getDamRoll();

   if(temp_dr < -3)       return "horrible";
   else if(temp_dr == -2) return "terrible";
   else if(temp_dr == -1) return "bad";
   else if(temp_dr == 0)  return "no";
   else if(temp_dr == 1 || temp_dr == 2)    return "small";
   else if(temp_dr == 3 || temp_dr == 4)    return "some";
   else if(temp_dr == 5 || temp_dr == 6)    return "good";
   else if(temp_dr == 7 || temp_dr == 8)    return "very good";
   else if(temp_dr == 9 || temp_dr == 10)   return "great";
   else if(temp_dr == 11 || temp_dr == 12)  return "unbelievable";
   else if(temp_dr == 13 || temp_dr == 14)  return "legendary";

   return "godly";
}

void print_states(CHAR_DATA *ch)
{
	string buf;

	bool isThirsty = ch->pcdata->condition[COND_THIRST] == 0;
	bool isHungry  = ch->pcdata->condition[COND_FULL] == 0;
	bool isDrunk   = ch->pcdata->condition[COND_DRUNK] > 10;

	if(IS_NPC(ch))
		send_to_char("Bad mobile! Bad!\r\n", ch);

	buf = "";

	if(isThirsty || isHungry || isDrunk) {
		buf += "you are ";
		if(isHungry)  buf += "hungry, ";
		if(isThirsty) buf += "thirsty, ";
		if(isDrunk)   buf += "drunk,";

		buf += " and ";
	}

	if(ch->position != POS_SLEEPING)
		switch(ch->mental_state / 10) {
	  default:	 buf += "your mind is completely messed up."; break;
	  case -10:  buf += "you're barely conscious."; break;
	  case	-9:  buf += "you can barely keep your eyes open."; break;
	  case	-8:  buf += "you're extremely drowsy."; break;
	  case	-7:  buf += "you feel very unmotivated."; break;
	  case	-6:  buf += "you feel sedated."; break;
	  case	-5:  buf += "you feel sleepy."; break;
	  case	-4:  buf += "you feel tired."; break;
	  case	-3:  buf += "you could use a rest."; break;
	  case	-2:  buf += "you feel a little under the weather."; break;
	  case	-1:  buf += "you feel fine."; break;
	  case	 0:  buf += "you feel great."; break;
	  case	 1:  buf += "you feel energetic."; break;
	  case	 2:  buf += "your mind is racing."; break;
	  case	 3:  buf += "you can't think straight."; break;
	  case	 4:  buf += "your mind is going 100 miles an hour."; break;
	  case	 5:  buf += "you're high as a kite."; break;
	  case	 6:  buf += "your mind and body are slipping apart."; break;
	  case	 7:
	  case	 8:  buf += "you feel reality slipping away."; break;
	  case	 9:  buf += "you feel immortal."; break;
	  case	10:  buf += "your mind has no limits."; break;
	}
	else if(ch->mental_state >45)
		buf += "you dream of strange things.";
	else if(ch->mental_state >25)
		buf += "your sleep is uneasy.";
	else if(ch->mental_state <-35)
		buf += "you are deep in a much needed sleep.";
	else if(ch->mental_state <-25)
		buf += "you are in deep slumber.";

	ch_printf(ch, "%s\r\n", capitalize(buf.c_str()));

	return;
}

void do_oldscore(CHAR_DATA *ch, const char* argument)
{
   AFFECT_DATA *paf;
   SkillType   *skill;

   if(IS_AFFECTED(ch, AFF_POSSESS))
     {
	send_to_char("You can't do that in your current state of mind!\r\n", ch);
	return;
     }

   set_char_color(AT_SCORE, ch);
   ch_printf(ch,
	     "You are %s%s, level %d, %d years old (%d hours).\r\n",
	     ch->getName().c_str(),
	     IS_NPC(ch) ? "" : player_title(ch->pcdata->title_.c_str()),
	     ch->level,
	     get_age(ch),
	     (get_age(ch) - 17) * 2);

   if(get_trust(ch) != ch->level)
     ch_printf(ch, "You are trusted at level %d.\r\n",
	       get_trust(ch));

   if(IS_SET(ch->act, ACT_MOBINVIS))
     ch_printf(ch, "You are mobinvis at level %d.\r\n",
	       ch->mobinvis);

   if(IS_VAMPIRE(ch))
     ch_printf(ch,
	       "You have %d/%d hit, %d/%d blood level, %d/%d movement, %d practices.\r\n",
	       ch->hit,  ch->max_hit,
	       ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level,
	       ch->move, ch->max_move,
	       ch->practice);
   else
     ch_printf(ch,
	       "You have %d/%d hit, %d/%d mana, %d/%d movement, %d practices.\r\n",
	       ch->hit,  ch->max_hit,
	       ch->mana, ch->max_mana,
	       ch->move, ch->max_move,
	       ch->practice);

   ch_printf(ch,
	     "You are carrying %d/%d items with weight %d/%d kg.\r\n",
	     ch->carry_number, can_carry_n(ch),
	     ch->carry_weight, can_carry_w(ch));

   /*  if(ch->level >= 5) */
   ch_printf(ch,
	     "Str: %d  Int: %d  Wis: %d  Dex: %d  Con: %d  Cha: %d  Lck: %d.\r\n",
	     ch->getStr(),
	     ch->getInt(),
	     ch->getWis(),
	     ch->getDex(),
	     ch->getCon(),
	     ch->getCha(),
	     ch->getLck());

   ch_printf(ch,
	     "You have scored %d exp, and have %d gold coins.\r\n",
	     ch->exp,  ch->gold);

   if(!IS_NPC(ch))
     ch_printf(ch,
	       "You have achieved %d.%d glory during your life, and currently have %d.\r\n",
	       ch->pcdata->quest_accum, ch->pcdata->quest_deci, ch->pcdata->quest_curr);

   ch_printf(ch,
	     "Autoexit: %s   Autoloot: %s   Autosac: %s   Autogold: %s\r\n",
	     (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT)) ? "yes" : "no",
	     (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOLOOT)) ? "yes" : "no",
	     (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOSAC)) ? "yes" : "no",
	     (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOGOLD)) ? "yes" : "no");

   ch_printf(ch, "Wimpy set to %d hit points.\r\n", ch->wimpy);

   if(!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]   > 10)
     send_to_char("You are drunk.\r\n",   ch);
   if(!IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] ==  0)
     send_to_char("You are thirsty.\r\n", ch);
   if(!IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   ==  0)
     send_to_char("You are hungry.\r\n",  ch);

   switch(ch->mental_state / 10)
     {
      default:   send_to_char("You're completely messed up!\r\n", ch); break;
      case -10:  send_to_char("You're barely conscious.\r\n", ch); break;
      case  -9:  send_to_char("You can barely keep your eyes open.\r\n", ch); break;
      case  -8:  send_to_char("You're extremely drowsy.\r\n", ch); break;
      case  -7:  send_to_char("You feel very unmotivated.\r\n", ch); break;
      case  -6:  send_to_char("You feel sedated.\r\n", ch); break;
      case  -5:  send_to_char("You feel sleepy.\r\n", ch); break;
      case  -4:  send_to_char("You feel tired.\r\n", ch); break;
      case  -3:  send_to_char("You could use a rest.\r\n", ch); break;
      case  -2:  send_to_char("You feel a little under the weather.\r\n", ch); break;
      case  -1:  send_to_char("You feel fine.\r\n", ch); break;
      case   0:  send_to_char("You feel great.\r\n", ch); break;
      case   1:  send_to_char("You feel energetic.\r\n", ch); break;
      case   2:  send_to_char("Your mind is racing.\r\n", ch); break;
      case   3:  send_to_char("You can't think straight.\r\n", ch); break;
      case   4:  send_to_char("Your mind is going 100 miles an hour.\r\n", ch); break;
      case   5:  send_to_char("You're high as a kite.\r\n", ch); break;
      case   6:  send_to_char("Your mind and body are slipping appart.\r\n", ch); break;
      case   7:  send_to_char("Reality is slipping away.\r\n", ch); break;
      case   8:  send_to_char("You have no idea what is real, and what is not.\r\n", ch); break;
      case   9:  send_to_char("You feel immortal.\r\n", ch); break;
      case  10:  send_to_char("You are a Supreme Entity.\r\n", ch); break;
     }

   switch (ch->position)
     {
      case POS_DEAD:
	send_to_char("You are DEAD!!\r\n",		ch);
	break;
      case POS_MORTAL:
	send_to_char("You are mortally wounded.\r\n",	ch);
	break;
      case POS_INCAP:
	send_to_char("You are incapacitated.\r\n",	ch);
	break;
      case POS_STUNNED:
	send_to_char("You are stunned.\r\n",		ch);
	break;
      case POS_SLEEPING:
	send_to_char("You are sleeping.\r\n",		ch);
	break;
      case POS_RESTING:
	send_to_char("You are resting.\r\n",		ch);
	break;
      case POS_STANDING:
	send_to_char("You are standing.\r\n",		ch);
	break;
      case POS_FIGHTING:
	send_to_char("You are fighting.\r\n",		ch);
	break;
      case POS_MOUNTED:
	send_to_char("Mounted.\r\n",			ch);
	break;
      case POS_SHOVE:
	send_to_char("Being shoved.\r\n",		ch);
	break;
      case POS_DRAG:
	send_to_char("Being dragged.\r\n",		ch);
	break;
     }

   if(ch->level >= 25)
     ch_printf(ch, "AC: %d.  ", ch->getAC());

   send_to_char("You are ", ch);
   if(ch->getAC() >=  101) send_to_char("WORSE than naked!\r\n", ch);
   else if(ch->getAC() >=   80) send_to_char("naked.\r\n",            ch);
   else if(ch->getAC() >=   60) send_to_char("wearing clothes.\r\n",  ch);
   else if(ch->getAC() >=   40) send_to_char("slightly armored.\r\n", ch);
   else if(ch->getAC() >=   20) send_to_char("somewhat armored.\r\n", ch);
   else if(ch->getAC() >=    0) send_to_char("armored.\r\n",          ch);
   else if(ch->getAC() >= - 20) send_to_char("well armored.\r\n",     ch);
   else if(ch->getAC() >= - 40) send_to_char("strongly armored.\r\n", ch);
   else if(ch->getAC() >= - 60) send_to_char("heavily armored.\r\n",  ch);
   else if(ch->getAC() >= - 80) send_to_char("superbly armored.\r\n", ch);
   else if(ch->getAC() >= -100) send_to_char("divinely armored.\r\n", ch);
   else                           send_to_char("invincible!\r\n",       ch);

   if(ch->level >= 15)
     ch_printf(ch, "Hitroll: %d  Damroll: %d.\r\n",
	       ch->getHitRoll(), ch->getDamRoll());

   if(ch->level >= 10)
     ch_printf(ch, "Alignment: %d.  ", ch->alignment);

   send_to_char("You are ", ch);
   if(ch->alignment >  900) send_to_char("angelic.\r\n", ch);
   else if(ch->alignment >  700) send_to_char("saintly.\r\n", ch);
   else if(ch->alignment >  350) send_to_char("good.\r\n",    ch);
   else if(ch->alignment >  100) send_to_char("kind.\r\n",    ch);
   else if(ch->alignment > -100) send_to_char("neutral.\r\n", ch);
   else if(ch->alignment > -350) send_to_char("mean.\r\n",    ch);
   else if(ch->alignment > -700) send_to_char("evil.\r\n",    ch);
   else if(ch->alignment > -900) send_to_char("demonic.\r\n", ch);
   else                             send_to_char("satanic.\r\n", ch);

   if(ch->first_affect)
     {
	send_to_char("You are affected by:\r\n", ch);
	for (paf = ch->first_affect; paf; paf = paf->next)
	  if((skill=get_skilltype(paf->type)) != NULL)
	  {
	     ch_printf(ch, "Spell: '%s'", skill->name_.c_str());

	     if(ch->level >= 20 && paf->location != APPLY_CONTAGION)
	       ch_printf(ch,
			 " modifies %s by %d for %d rounds",
			 affect_loc_name(paf->location),
			 paf->modifier,
			 paf->duration);

	     send_to_char(".\r\n", ch);
	  }
     }

   if(!IS_NPC(ch) && IS_IMMORTAL(ch))
     {
	ch_printf(ch, "WizInvis level: %d   WizInvis is %s\r\n",
		  ch->pcdata->wizinvis,
		  IS_SET(ch->act, PLR_WIZINVIS) ? "ON" : "OFF");
	if(ch->pcdata->r_range_lo && ch->pcdata->r_range_hi)
	  ch_printf(ch, "Room Range: %d - %d\r\n", ch->pcdata->r_range_lo,
		    ch->pcdata->r_range_hi	);
	if(ch->pcdata->o_range_lo && ch->pcdata->o_range_hi)
	  ch_printf(ch, "Obj Range : %d - %d\r\n", ch->pcdata->o_range_lo,
		    ch->pcdata->o_range_hi	);
	if(ch->pcdata->m_range_lo && ch->pcdata->m_range_hi)
	  ch_printf(ch, "Mob Range : %d - %d\r\n", ch->pcdata->m_range_lo,
		    ch->pcdata->m_range_hi	);
     }

   return;
}

/*								-Thoric
 * Display your current exp, level, and surrounding level exp requirements
 */
void do_level(CHAR_DATA *ch, const char* argument)
{
   char buf [MAX_STRING_LENGTH];
   int x, lowlvl, hilvl;

   if(ch->level == 1)
     lowlvl = 1;
   else
     lowlvl = UMAX(2, ch->level - 5);
   hilvl = URANGE(ch->level, ch->level + 5, MAX_LEVEL);
   set_char_color(AT_SCORE, ch);
   ch_printf(ch, "Experience required levels %d to %d:\r\n", lowlvl, hilvl);
   sprintf(buf, " exp (You have %d)", ch->exp);
   for (x = lowlvl; x <= hilvl; x++)
     ch_printf(ch, " %3d) %11d%s\r\n", x, exp_level(ch, x),
	       (x == ch->level) ? buf : " exp");
}


void do_affected(CHAR_DATA *ch, const char* argument)
{
   char arg [MAX_INPUT_LENGTH];
   AFFECT_DATA *paf;
   SkillType *skill;

   if(IS_NPC(ch))
     return;

   argument = one_argument(argument, arg);

   if(!str_cmp(arg, "by"))
     {
        set_char_color(AT_BLUE, ch);
        send_to_char("\r\nImbued with:\r\n", ch);
	set_char_color(AT_SCORE, ch);
	ch_printf(ch, "%s\r\n", affect_bit_name(ch->affected_by));
        if(ch->level >= 20)
	  {
	     send_to_char("\r\n", ch);
	     if(ch->resistant > 0)
	       {
		  set_char_color (AT_BLUE, ch);
		  send_to_char("Resistances:  ", ch);
		  set_char_color(AT_SCORE, ch);
		  ch_printf(ch, "%s\r\n", flag_string(ch->resistant, ris_flags));
	       }
	     if(ch->immune > 0)
	       {
		  set_char_color(AT_BLUE, ch);
		  send_to_char("Immunities:   ", ch);
		  set_char_color(AT_SCORE, ch);
		  ch_printf(ch, "%s\r\n", flag_string(ch->immune, ris_flags));
	       }
	     if(ch->susceptible > 0)
	       {
		  set_char_color(AT_BLUE, ch);
		  send_to_char("Suscepts:     ", ch);
		  set_char_color(AT_SCORE, ch);
		  ch_printf(ch, "%s\r\n", flag_string(ch->susceptible, ris_flags));
	       }
	  }
	return;
     }

     {
     int displayed=0;

       send_to_char("\r\n", ch);
       for (paf = ch->first_affect; paf; paf = paf->next)
       {
         if((skill=get_skilltype(paf->type)) != NULL)
	 {
	   set_char_color(AT_BLUE, ch);
	   send_to_char("Affected:  ", ch);
	   set_char_color(AT_SCORE, ch);
	   if(ch->level >= 20)
	   {
             if(paf->duration < 25) set_char_color(AT_WHITE, ch);
             if(paf->duration < 6 ) set_char_color(AT_WHITE + AT_BLINK, ch);
               ch_printf(ch, "(%5d)   ", paf->duration);
	   }
	   ch_printf(ch, "%-18s\r\n", skill->name_.c_str());
           displayed++;
	 }
       }
       if(displayed==0)
       {
         set_char_color(AT_SCORE, ch);
         send_to_char("No cantrip or skill affects you.\r\n", ch);
       }
     }
   return;
}

void do_inventory(CHAR_DATA *ch, const char* argument)
{
   set_char_color(AT_RED, ch);
   send_to_char("You are carrying:\r\n", ch);
   show_list_to_char(ch->first_carrying, ch, TRUE, TRUE);
   return;
}

void do_new_inv( CHAR_DATA *ch, const char* argument )
{
   OBJ_DATA *obj;
   OBJ_DATA *contained;

	set_char_color( AT_RED, ch );
	send_to_char( "You are carrying:\r\n", ch );

   /*Is there anything being worn in scabbard 1?*/
   if( ( obj = get_eq_char( ch, WEAR_SCABBARD_1 ) ) )
   {
      set_char_color( AT_OBJECT, ch );
      send_to_char( "   ", ch );
      send_to_char( obj->shortDesc_.c_str(), ch );
      send_to_char( "\r\n", ch );
      /*If there is, show us what it is. Else print Nothing.*/
      if( ( contained =  obj->first_content  ) )
      {
         send_to_char( "      ", ch );
         set_char_color( AT_OBJECT, ch );
         send_to_char( contained->shortDesc_.c_str(), ch );
         send_to_char( "\r\n", ch );
      }
      else
      {
         set_char_color( AT_OBJECT, ch );
         send_to_char( "      Nothing.\r\n", ch );
      }
   }
   /*Same as above but for scabbard 2.*/
   if( ( obj = get_eq_char( ch, WEAR_SCABBARD_2 ) ) )
   {
      set_char_color( AT_OBJECT, ch );
      send_to_char( "   ", ch );
      send_to_char( obj->shortDesc_.c_str(), ch );
      send_to_char( "\r\n", ch );

      if( ( contained = obj->first_content ) )
      {
         set_char_color( AT_OBJECT, ch );
         send_to_char( "      ", ch );
         send_to_char( contained->shortDesc_.c_str(), ch );
         send_to_char( "\r\n", ch );
      }
      else
      {
         set_char_color( AT_OBJECT, ch );
         send_to_char( "      Nothing.\r\n", ch );
      }
   }

	for( obj = ch->first_carrying; obj != NULL; obj=obj->next_content )
	{
		if( can_see_obj( ch, obj ) )
		{
			if( obj->item_type == ITEM_CONTAINER )
			{
				set_char_color( AT_OBJECT, ch );
				send_to_char( "   In ", ch );
				send_to_char( obj->shortDesc_.c_str(), ch );
				send_to_char( "\r\n", ch );

            if( !obj->first_content )
            {
               send_to_char( "      Nothing.\r\n", ch );
               continue;
            }

				for( contained = obj->first_content; contained != obj->last_content;
				     contained = contained->next_content )
				{
					switch( contained->item_type )
					{
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
					
					send_to_char( "      ", ch );
					send_to_char( contained->shortDesc_.c_str(), ch );
					send_to_char( "\r\n", ch );
				}
			}
			else
				continue;
		}
	}
	return;
}


			

void do_equipment(CHAR_DATA *ch, const char* argument)
{
   OBJ_DATA *obj;
   int iWear;
   bool found;

   set_char_color(AT_RED, ch);
   send_to_char("You are using:\r\n", ch);
   found = FALSE;
   set_char_color(AT_OBJECT, ch);
   for (iWear = 0; iWear < MAX_WEAR; iWear++)
     {
	for (obj = ch->first_carrying; obj; obj = obj->next_content)
	  if(obj->wear_loc == iWear)
	  {
	     send_to_char(where_name[iWear], ch);
	     if(can_see_obj(ch, obj))
	       {
               int limit;

		  send_to_char(format_obj_to_char(obj, ch, TRUE), ch);

                  limit = obj->max_condition;
                  switch(obj->item_type)
                  {
                    case ITEM_ARMOR:
                    case ITEM_WEAPON:
					case ITEM_CONTAINER:
                      if(obj->condition < limit && obj->condition > 0)
                        send_to_char(" (dmg)",ch);
                      break;
                  }
		  send_to_char("\r\n", ch);
	       }
	     else
	       send_to_char("something.\r\n", ch);
	     found = TRUE;
	  }
     }

   if(!found)
     send_to_char("Nothing.\r\n", ch);

   return;
}



void set_title(CHAR_DATA *ch, const std::string& title)
{
    std::string buf;
	char sbuf[MAX_STRING_LENGTH];

    if(IS_NPC(ch))
    {
        bug("Set_title: NPC.", 0);
        return;
    }

    buf = title;
    ch->pcdata->title_ = buf;

	if(( isalpha(ch->pcdata->title_.c_str()[0]) || isdigit(ch->pcdata->title_.c_str()[0])) && ch->pcdata->title_.c_str()[1]!=':' )
	{
		sprintf( sbuf, " %s", ch->pcdata->title_.c_str() );
		ch->pcdata->title_ = sbuf;
	}

}


void do_titlelock(CHAR_DATA * ch, const char* argument)
{
	Character * vch;

	if ( ( vch = get_char_world(ch, argument) ) == NULL )
	{
		ch_printf(ch,"You can't find them.\r\n");
		return;
	}

	if ( IS_NPC(vch) )
	{
		ch->sendText("Not on mobs!\r\n");
		return;
	}
	if ( get_trust(ch) < get_trust(vch) )
	{
		ch->sendText("You're not worthy enough to lock " + vch->getName().str() + "'s name.\r\n");
		return;
	}

	if ( !vch->pcdata )
	{
		ch->sendText("Critical: That player has no playerdata structure! Aborting, please report.\r\n");
		return;
	}

	vch->pcdata->titleLocked = !vch->pcdata->titleLocked;

	if ( vch->pcdata->titleLocked )
		ch->sendText("Title locked.\r\n");
	else
		ch->sendText("Title unlocked.\r\n");
}

void do_titlechange(CHAR_DATA *ch, const char* argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *vch;

	argument = one_argument( argument, arg );

	if ( (vch=get_char_world(ch,arg))==NULL )
	{
		ch_printf(ch,"You can't find them.\r\n");
		return;
	}
	if ( IS_NPC(ch) )
	{
		ch_printf(ch,"Not on mobs!\r\n");
		return;
	}
	if (get_trust(ch) < get_trust(vch))
	{
		ch_printf(ch,"You are not worthy enough to change their title!\r\n");
		return;
	}

	set_title(vch,argument);
}

void do_title(CHAR_DATA *ch, const char* argument)
{
    if(IS_NPC(ch))
        return;

    std::string title = argument;

    if(ch->level < 5)
    {
        send_to_char("Sorry... you must be at least level 5 to do that.\r\n", ch);
        return;
    }
    if(IS_SET(ch->pcdata->flags, PCFLAG_NOTITLE))
    {
        send_to_char("The Gods prohibit you from changing your title.\r\n", ch);
        return;
    }


    if(title.length() == 0)
    {
        send_to_char("Change your title to what?\r\n", ch);
        return;
    }

    if(title[1]==':')
        title[1]=' '; /* oh no you don't - Testaur */

    if(title.length() > 50)
        title = title.substr(0, 50);

    title = SmashTilde(title);
    set_title(ch, title);
    send_to_char("Ok.\r\n", ch);
}


void do_homepage(CHAR_DATA *ch, const char* argument)
{
	char buf[MAX_STRING_LENGTH];

	if(IS_NPC(ch))
		return;

	if(ch->level < 5)
	{
		send_to_char("Sorry... you must be at least level 5 to do that.\r\n", ch);
		return;
	}

	if(argument[0] == '\0')
	{
		if(!ch->pcdata->homepage)
			ch->pcdata->homepage = str_dup("");
		ch_printf(ch, "Your homepage is: %s\r\n",
				show_tilde(ch->pcdata->homepage));
		return;
	}

	if(!str_cmp(argument, "clear"))
	{
		if(ch->pcdata->homepage)
			DISPOSE(ch->pcdata->homepage);
		ch->pcdata->homepage = str_dup("");
		send_to_char("Homepage cleared.\r\n", ch);
		return;
	}

	if(strstr(argument, "://"))
		strcpy(buf, argument);
	else
		sprintf(buf, "http://%s", argument);
	if(strlen(buf) > 70)
		buf[70] = '\0';

	hide_tilde(buf);
	if(ch->pcdata->homepage)
		DISPOSE(ch->pcdata->homepage);
	ch->pcdata->homepage = str_dup(buf);
	send_to_char("Homepage set.\r\n", ch);
}



/*
 * Set your personal description				-Thoric
 */
void do_description(CHAR_DATA *ch, const char* argument)
{
   if(IS_NPC(ch))
     {
	send_to_char("Monsters are too dumb to do that!\r\n", ch);
	return;
     }

   if(!ch->GetConnection())
     {
	bug("do_description: no descriptor", 0);
	return;
     }

   switch(ch->substate)
     {
      default:
	bug("do_description: illegal substate", 0);
	return;

      case SUB_RESTRICTED:
	send_to_char("You cannot use this command from within another command.\r\n", ch);
	return;

      case SUB_NONE:
	ch->substate = SUB_PERSONAL_DESC;
	ch->dest_buf = ch;
	start_editing(ch, ch->description_.c_str());
	return;

      case SUB_PERSONAL_DESC:
	ch->description_ = copy_buffer_string(ch);
	stop_editing(ch);
	return;
     }
}

/* Ripped off do_description for whois bio's -- Scryn*/
void do_bio(CHAR_DATA *ch, const char* argument)
{
   if(IS_NPC(ch))
     {
	send_to_char("Mobs can't set bio's!\r\n", ch);
	return;
     }

   if(!ch->GetConnection())
     {
	bug("do_bio: no descriptor", 0);
	return;
     }

   switch(ch->substate)
     {
      default:
	bug("do_bio: illegal substate", 0);
	return;

      case SUB_RESTRICTED:
	send_to_char("You cannot use this command from within another command.\r\n", ch);
	return;

      case SUB_NONE:
	ch->substate = SUB_PERSONAL_BIO;
	ch->dest_buf = ch;
	start_editing(ch, ch->pcdata->bio);
	return;

      case SUB_PERSONAL_BIO:
	STRFREE(ch->pcdata->bio);
	ch->pcdata->bio = copy_buffer(ch);
	if ( strlen(ch->pcdata->bio) >= MAX_STRING_LENGTH - 1)
	{
		send_to_char("Bio too long. Cancelled...\r\n", ch);
		STRFREE(ch->pcdata->bio);
	}
	stop_editing(ch);
	return;
     }
}



void do_report(CHAR_DATA *ch, const char* argument)
{
   char buf[MAX_INPUT_LENGTH];

   if(IS_AFFECTED(ch, AFF_POSSESS))
     {
	send_to_char("You can't do that in your current state of mind!\r\n", ch);
	return;
     }

   if(IS_VAMPIRE(ch))
     ch_printf(ch,
	       "You report: %d/%d hp %d/%d blood %d/%d mv %d xp.\r\n",
	       ch->hit,  ch->max_hit,
	       ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level,
	       ch->move, ch->max_move,
	       ch->exp  );
   else
     ch_printf(ch,
	       "You report: %d/%d hp %d/%d mana %d/%d mv %d xp.\r\n",
	       ch->hit,  ch->max_hit,
	       ch->mana, ch->max_mana,
	       ch->move, ch->max_move,
	       ch->exp  );

   if(IS_VAMPIRE(ch))
     sprintf(buf, "$n reports: %d/%d hp %d/%d blood %d/%d mv %d xp.\r\n",
	     ch->hit,  ch->max_hit,
	     ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level,
	     ch->move, ch->max_move,
	     ch->exp  );
   else
     sprintf(buf, "$n reports: %d/%d hp %d/%d mana %d/%d mv %d xp.",
	     ch->hit,  ch->max_hit,
	     ch->mana, ch->max_mana,
	     ch->move, ch->max_move,
	     ch->exp  );

   act(AT_REPORT, buf, ch, NULL, NULL, TO_ROOM);

   return;
}

void do_prompt(CHAR_DATA *ch, const char* argument)
{
    char arg[MAX_INPUT_LENGTH];
    std::string prompt;

    if(IS_NPC(ch))
    {
        send_to_char("NPC's can't change their prompt..\r\n", ch);
        return;
    }
    prompt = SmashTilde(argument);
    one_argument(prompt.c_str(), arg);

    if(arg[0] == '\0')
    {
        ch_printf_nocolor(ch, "Current prompts set to:\r\n"
                          "    Current: \t%s\r\n"
                          "  Alternate: \t%s\r\n"
                          , ch->pcdata->prompt_.c_str(), ch->pcdata->prompt2_.c_str());
        return;
    }

    if( prompt.length() > 128 )
        prompt = prompt.substr(0, 128);

    if ( !str_cmp(arg, "switch") )
    {
        shared_str prompt2 = ch->pcdata->prompt2_;

        ch->pcdata->prompt2_ = ch->pcdata->prompt_;
        ch->pcdata->prompt_ = prompt2;
    }
    else
    {
        if( !str_cmp(arg, "default") )
            ch->pcdata->prompt_ = "";
        else
            ch->pcdata->prompt_ = prompt;
    }

    send_to_char("Ok.\r\n", ch);
    return;
}

void do_gain(CHAR_DATA* ch, const char* argument) {
   CHAR_DATA *mob;
   TRAIN_DATA *train;

   if(IS_NPC(ch)) {
      return;
   }

   if(!IS_AWAKE(ch)) {
      send_to_char("In your dreams, or what?\r\n", ch);
      return;
   }

   for (mob = ch->GetInRoom()->first_person; mob; mob = mob->next_in_room) {
      if(IS_NPC(mob) && mob->pIndexData->train) {
	 break;
      }
   }

   if(!mob) {
      send_to_char("There is no one here who can teach you.\r\n", ch);
      return;
   }

   train = mob->pIndexData->train;

   if((train->_class != ch->Class) && (train->_class != -1))
     {
        act(AT_TELL, "$n tells you 'I'll have nothing to do with your kind!", mob, NULL, ch, TO_VICT);
        return;
     }
   else if(ch->level+1 > train->max_level && (train->max_level != -1))
     {
        act(AT_TELL, "$n tells you 'You have already learned more than I.'", mob, NULL, ch, TO_VICT);
        return;
     }
   else if((train->min_level > ch->level))
     {
        act(AT_TELL, "$n tells you 'You are too young for me to bother with.'", mob, NULL, ch, TO_VICT);
        return;
     }
   else if((train->gain_cost > ch->gold))
     {
        act(AT_TELL, "$n tells you 'I will not train beggars!'", mob, NULL, ch, TO_VICT);
        return;
     }
   else if((
            ((IS_EVIL(ch)) && (train->alignment != 0))
            ||
            ((IS_GOOD(ch)) && (train->alignment != 2))
            ||
            ((IS_NEUTRAL(ch)) && (train->alignment != 1)))
	   &&
	   (train->alignment != -1)
	   )
     {
        act(AT_TELL, "$n tells you 'I will not train someone opposed to my alignment.'",
	    mob, NULL, ch, TO_VICT);
        return;
     }

   if(ch->level+1 == LEVEL_HERO_MIN) {
      send_to_char("You must seek out the Avatar Challenge now.\r\n", ch);
      return;
   }

   if(ch->level+1 < LEVEL_HERO_MIN && ch->exp >= exp_level(ch, ch->level+1))
     {
        char buf[MAX_STRING_LENGTH];

        if(train->gain_cost > 0) {
	   ch->gold -= train->gain_cost;
	   act(AT_ACTION, "$n pockets some of your money.", mob, NULL, ch, TO_VICT);
        }

        set_char_color(AT_WHITE + AT_BLINK, ch);
        ch_printf(ch, "You have now obtained experience level %d!\r\n", ++ch->level);
        advance_level(ch);

        sprintf(buf, "%s gained 1 level.", ch->getShort().c_str());
        to_channel(buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL);

        do_gain(ch, "");
     }

}

extern const unsigned char echo_off_str[];

void do_delete(CHAR_DATA *ch, const char* argument) {
   if(str_cmp(argument, "myself forever")) {
      send_to_char("Usage: delete myself forever\r\n", ch);
      return;
   }

   send_to_char("This command allows you to delete your character.\r\n", ch);
   send_to_char("At the prompt, enter your password.\r\n", ch);
   send_to_char("If you change your mind, enter anything other than your password.\r\r\n\n", ch);
   send_to_char("Enter your password ==>  ", ch);
   send_to_char((const char*) echo_off_str, ch);
   ch->GetConnection()->ConnectedState = CON_DELETE_PROMPT;
   ch->GetConnection()->SetInputReceiver(gConnectionManager);
}

